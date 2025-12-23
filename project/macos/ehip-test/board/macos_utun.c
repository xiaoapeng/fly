/**
 * @file macos_utun.c
 * @brief macos utun device test
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-12-21
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/kern_control.h>
#include <sys/sys_domain.h>
#include <net/if_dl.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/if_utun.h>
#include <net/route.h>
#include <netinet/in.h>


#include <eh.h>
#include <eh_event.h>
#include <eh_event_flags.h>
#include <eh_error.h>
#include <eh_mem.h>
#include <eh_module.h>
#include <eh_debug.h>
#include <eh_sleep.h>
#include <eh_signal.h>
#include <eh_mem_pool.h>
#include <eh_types.h>
#include <eh_swab.h>

#include <ehip_core.h>
#include <ehip_module.h>
#include <ehip_buffer_type.h>
#include <ehip_buffer.h>
#include <ehip_netdev.h>
#include <ehip_netdev_type.h>
#include <ehip_netdev_trait.h>
#include <ehip-netdev-class/tun_dev.h>
#include <kqueue_hub.h>


/**
 * @brief utun device test
 *      sudo ifconfig utun10 create
 *      sudo ifconfig utun10 up
 *      sudo ifconfig utun10 inet 10.10.0.1 netmask 255.255.255.0
 */

#ifndef EH_DBG_MODEULE_LEVEL_MACOS_UTUN 
#define EH_DBG_MODEULE_LEVEL_MACOS_UTUN EH_DBG_MODEULE_LEVEL_INFO
#endif

#define UTUN_UNIT 10

static ehip_netdev_t * s_netdev = NULL;
static int s_utun_fd = -1;

int utun_add_lan_route_pfroute(const char *ifname,
                               const char *network,
                               int prefix_len) {
    int sock = -1;
    int ret = -1;
    struct rt_msghdr *rtm;
    struct sockaddr_in *sin;
    char buf[1024];
    int ifindex = 0;
    in_addr_t network_addr, netmask;
    
    // 参数验证
    if (!ifname || !network || prefix_len < 0 || prefix_len > 32) {
        errno = EINVAL;
        return -1;
    }
    
    printf("Adding route via PF_ROUTE: %s/%d -> %s\n", 
           network, prefix_len, ifname);
    
    // 获取接口索引
    ifindex = (int)if_nametoindex(ifname);
    if (ifindex == 0) {
        fprintf(stderr, "Interface %s not found\n", ifname);
        errno = ENXIO;
        return -1;
    }
    
    // 转换网络地址
    if (inet_pton(AF_INET, network, &network_addr) != 1) {
        fprintf(stderr, "Invalid network address: %s\n", network);
        errno = EINVAL;
        return -1;
    }
    
    // 计算子网掩码
    if (prefix_len == 0) {
        netmask = 0;
    } else if (prefix_len == 32) {
        netmask = 0xFFFFFFFF;
    } else {
        netmask = htonl(0xFFFFFFFF << (32 - prefix_len));
    }
    
    // 创建路由socket
    sock = socket(PF_ROUTE, SOCK_RAW, 0);
    if (sock < 0) {
        perror("socket(PF_ROUTE)");
        return -1;
    }
    
    // 清除缓冲区
    memset(buf, 0, sizeof(buf));
    rtm = (struct rt_msghdr *)buf;
    
    // 设置路由消息头
    rtm->rtm_msglen = sizeof(struct rt_msghdr) + 
                      sizeof(struct sockaddr_in) +  // 目标网络
                      sizeof(struct sockaddr_in) +  // 网关
                      sizeof(struct sockaddr_in) +  // 子网掩码
                      sizeof(struct sockaddr_dl);   // 接口
    rtm->rtm_version = RTM_VERSION;
    rtm->rtm_type = RTM_ADD;           // 添加路由
    rtm->rtm_flags = RTF_UP | RTF_STATIC;
    rtm->rtm_addrs = RTA_DST | RTA_GATEWAY | RTA_NETMASK | RTA_IFP;
    rtm->rtm_pid = getpid();
    rtm->rtm_seq = 1;
    rtm->rtm_errno = 0;
    rtm->rtm_index = (u_short)ifindex;          // 接口索引
    
    char *cp = buf + sizeof(struct rt_msghdr);
    
    // 1. 目标网络地址
    sin = (struct sockaddr_in *)cp;
    sin->sin_len = sizeof(struct sockaddr_in);
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = network_addr;
    cp += sizeof(struct sockaddr_in);
    
    // 2. 网关地址（设置为0.0.0.0，表示直接连接）
    sin = (struct sockaddr_in *)cp;
    sin->sin_len = sizeof(struct sockaddr_in);
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = 0;  // 0.0.0.0
    cp += sizeof(struct sockaddr_in);
    
    // 3. 子网掩码
    sin = (struct sockaddr_in *)cp;
    sin->sin_len = sizeof(struct sockaddr_in);
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = netmask;
    cp += sizeof(struct sockaddr_in);
    
    // 4. 接口（sockaddr_dl）
    struct sockaddr_dl *sdl = (struct sockaddr_dl *)cp;
    sdl->sdl_len = sizeof(struct sockaddr_dl);
    sdl->sdl_family = AF_LINK;
    sdl->sdl_index = (u_short)ifindex;
    sdl->sdl_type = IFT_OTHER;
    sdl->sdl_nlen = 0;
    sdl->sdl_alen = 0;
    sdl->sdl_slen = 0;
    
    // 发送路由消息
    ssize_t n = write(sock, buf, rtm->rtm_msglen);
    if (n < 0) {
        perror("write route message");
        fprintf(stderr, "Failed to add route: %s\n", strerror(errno));
        
        // 如果错误是路由已存在，可以认为是成功的
        if (errno == EEXIST) {
            printf("Route already exists\n");
            ret = 0;
        }
    } else {
        printf("Route added successfully via PF_ROUTE\n");
        ret = 0;
    }
    
    close(sock);
    return ret;
}

static int utun_config_ipv4(const char *ifname,
                            const char *ip,
                            const char *mask)
{
    int fd;
    struct ifreq ifr;
    struct sockaddr_in *sin;

    if (!ifname || !ip || !mask) {
        errno = EINVAL;
        return -1;
    }

    /* 1. control socket */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        return -1;
    }

    /* 2. set IPv4 address */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    sin = (struct sockaddr_in *)&ifr.ifr_addr;
    sin->sin_family = AF_INET;

    if (inet_pton(AF_INET, ip, &sin->sin_addr) != 1) {
        close(fd);
        errno = EINVAL;
        return -1;
    }

    if (ioctl(fd, SIOCSIFADDR, &ifr) < 0) {
        close(fd);
        return -1;
    }

    /* 3. set netmask */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    sin = (struct sockaddr_in *)&ifr.ifr_addr;
    sin->sin_family = AF_INET;

    if (inet_pton(AF_INET, mask, &sin->sin_addr) != 1) {
        close(fd);
        errno = EINVAL;
        return -1;
    }

    if (ioctl(fd, SIOCSIFNETMASK, &ifr) < 0) {
        close(fd);
        return -1;
    }

    /* 4. set interface UP | RUNNING */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        close(fd);
        return -1;
    }

    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);

    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

static void utun_rx_process(ehip_netdev_t *netdev){
    ehip_buffer_t *ehip_buf;
    uint8_t *buffer;
    ssize_t len;
    uint32_t *type;
    ehip_buf = ehip_buffer_new(netdev->attr.buffer_type, 0);
    if(eh_ptr_to_error(ehip_buf) < 0){
        eh_mwarnfl(MACOS_UTUN, "ehip_buffer_new failed %d", eh_ptr_to_error(ehip_buf));
        /* 理论上应该过一会来继续读取，比如触发一个100us定时器 */
        return ;
    }
    buffer = ehip_buffer_get_buffer_ptr(ehip_buf);
    len = read(s_utun_fd, buffer, ehip_buffer_get_buffer_size(ehip_buf));
    if(len <= 0){
        if(errno != EAGAIN){
            eh_mwarnfl(MACOS_UTUN, "read utun_fd failed %s", strerror(errno));
        }
        ehip_buffer_free(ehip_buf);
        return ;
    }
    ehip_buffer_payload_append(ehip_buf, (ehip_buffer_size_t)len);
    type = (uint32_t *)ehip_buffer_head_reduce(ehip_buf, 4);
    if((*type) != eh_hton32(AF_INET)){
        ehip_buffer_free(ehip_buf);
        return ;
    }
    ehip_buf->netdev = netdev;
    ehip_buf->protocol = EHIP_PTYPE_ETHERNET_IP;
    eh_mdebugfl(MACOS_UTUN, "ip rx:%d@|%.*hhq|", len, len, buffer);
    ehip_rx(ehip_buf);
}
static void tap_epool_callback(int fd, int16_t filter, void *arg){
    (void) fd;
    (void) arg;
    if(filter == EVFILT_READ){
        utun_rx_process(s_netdev);
    }else if(filter == EVFILT_WRITE){
        ehip_queue_tx_wakeup(s_netdev);
    }
}
struct kqueue_event_action utun_event_callback_action = {
    .callback = tap_epool_callback,
    .arg = NULL,
};

static int utun_up(ehip_netdev_t *netdev){
    int fd = -1;
    struct ctl_info info;
    struct sockaddr_ctl addr;
    char ifname[20];
    int flags;
    socklen_t ifname_len = sizeof(ifname);
    fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
    if(fd < 0){
        eh_merrfl(MACOS_UTUN, "socket failed, errno: %d %s", errno, strerror(errno));
        return -1;
    }
    memset(&info, 0, sizeof(info));
    strlcpy(info.ctl_name, UTUN_CONTROL_NAME, sizeof(info.ctl_name));
    if (ioctl(fd, CTLIOCGINFO, &info) == -1) {
        eh_merrfl(MACOS_UTUN, "ioctl CTLIOCGINFO failed, errno: %d %s", errno, strerror(errno));
        close(fd);
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sc_len = sizeof(addr);
    addr.sc_family = AF_SYSTEM;
    addr.ss_sysaddr = AF_SYS_CONTROL;
    addr.sc_id = info.ctl_id;
    addr.sc_unit = UTUN_UNIT + 1;

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        eh_merrfl(MACOS_UTUN, "connect failed, errno: %d %s", errno, strerror(errno));
        close(fd);
        return -1;
    }
    if(getsockopt(fd, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, ifname, &ifname_len) < 0){
        eh_merrfl(MACOS_UTUN, "getsockopt UTUN_OPT_IFNAME failed, errno: %d %s", errno, strerror(errno));
        close(fd);
        return -1;
    }
    eh_minfofl(MACOS_UTUN, "utun ifname: %s", ifname);

    /* 设置utun为up状态 设置ip为10.10.0.1 netmask 255.0.0.0 */
    utun_config_ipv4(ifname, "10.10.0.1", "255.0.0.0");
    /* 10                 utun10             USc                utun10  */
    utun_add_lan_route_pfroute(ifname, "10.0.0.0", 8);
    flags = fcntl(fd, F_GETFL, 0);
    if(flags < 0){
        eh_merrfl(MACOS_UTUN, "fcntl F_GETFL failed, errno: %d %s", errno, strerror(errno));
        close(fd);
        return -1;
    }else{
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }
    kqueue_hub_add_fd(fd, EVFILT_READ, EV_ENABLE, &utun_event_callback_action);
    kqueue_hub_add_fd(fd, EVFILT_WRITE, EV_ENABLE|EV_CLEAR, &utun_event_callback_action);
    s_utun_fd = fd;
    ehip_netdev_set_link_status(netdev, true);
    return 0;
}

static void utun_down(ehip_netdev_t *netdev){
    ehip_netdev_set_link_status(netdev, false);
    kqueue_hub_del_fd(s_utun_fd, EVFILT_READ);
    kqueue_hub_del_fd(s_utun_fd, EVFILT_WRITE);
    close(s_utun_fd);
    s_utun_fd = -1;
}

static int utun_ctrl(ehip_netdev_t *netdev, uint32_t cmd, void *arg){
    (void) netdev;
    (void) cmd;
    (void) arg;
    return 0;
}

static int utun_start_xmit(ehip_netdev_t *netdev, ehip_buffer_t *buf){
    (void)netdev;
    ssize_t len;
    int retv = 0;
    uint32_t *type;
    eh_mdebugfl(MACOS_UTUN, "utun tx:%d@|%.*hhq|", ehip_buffer_get_payload_size(buf), ehip_buffer_get_payload_size(buf), ehip_buffer_get_payload_ptr(buf));
    type = (uint32_t *)ehip_buffer_head_append(buf, 4);
    if(type == NULL){
        eh_mwarnfl(MACOS_UTUN, "ehip_buffer_head_append failed");
        ehip_buffer_free(buf);
        return EH_RET_OK;
    }
    (*type) = eh_hton32(AF_INET);
    len = write(s_utun_fd, ehip_buffer_get_payload_ptr(buf), ehip_buffer_get_payload_size(buf));
    if(len < 0){
        if(errno == EAGAIN || errno == EWOULDBLOCK){
            retv = EH_RET_BUSY;
        }else{
            eh_merrfl(MACOS_UTUN, "write utun_fd failed %s len: %d", strerror(errno), len);
            retv = EH_RET_INVALID_STATE;
        }
    }
    ehip_buffer_free(buf);
    return retv;
}

static void utun_tx_timeout(ehip_netdev_t *netdev){
    (void) netdev;
}

static struct ehip_netdev_ops  netdev_utun_ops = {
    .ndo_up = utun_up,
    .ndo_down = utun_down,
    .ndo_ctrl = utun_ctrl,
    .ndo_start_xmit = utun_start_xmit,
    .ndo_tx_timeout = utun_tx_timeout,
};

static struct tun_trait_param tun_trait_param = {
    .hw_head_size = 4,
    .hw_tail_size = 0,
};

static const struct ehip_netdev_param netdev_tap_param = {
    .name = "mac-utun",
    .net_max_frame_size = 1500 + 4,
    .ops = &netdev_utun_ops,
    .trait_param = (void *)&tun_trait_param,
    .userdata = NULL
};

static int __init macos_utun_init(void){
    ehip_netdev_t *netdev;
    int ret;
    netdev = ehip_netdev_register(EHIP_NETDEV_TYPE_TUN, &netdev_tap_param);
    ret = eh_ptr_to_error(netdev);
    if(ret < 0){
        eh_merrfl(MACOS_UTUN, "ehip_netdev_register failed %d", ret);
        return ret;
    }
    s_netdev = netdev;
    return ret;
}

static void __exit macos_utun_exit(void){
    ehip_netdev_unregister(s_netdev);
}


ehip_netdev_module_export(macos_utun_init, macos_utun_exit);