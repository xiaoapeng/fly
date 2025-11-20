/**
 * @file linux_tap.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-11-05
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */

/*
 *  sudo ip tuntap add dev tap0 mode tap user $(whoami)
 *  sudo ip addr add 10.10.0.1/16 dev tap0
 *  sudo ip link set tap0 up

 *  # NAT 转发
    sudo sysctl -w net.ipv4.ip_forward=1
    sudo iptables -t nat -A POSTROUTING -s 10.10.0.0/16 -o eth0 -j MASQUERADE
    sudo iptables -A FORWARD -i tap0 -o eth0 -j ACCEPT
    sudo iptables -A FORWARD -i eth0 -o tap0 -m state --state RELATED,ESTABLISHED -j ACCEPT
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <linux/if_tun.h>
#include <linux/if_ether.h>
#include <linux/if.h>



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

#include <ehip_core.h>
#include <ehip_module.h>
#include <ehip_buffer_type.h>
#include <ehip_buffer.h>
#include <ehip_netdev.h>
#include <ehip_netdev_type.h>
#include <ehip_netdev_trait.h>
#include <ehip-mac/ethernet.h>
#include <ehip-netdev-class/ethernet_dev.h>


#include <eh_platform.h>
#include <epoll_hub.h>

#ifndef EH_DBG_MODEULE_LEVEL_NETDEV_LINUX_TAP
#define EH_DBG_MODEULE_LEVEL_NETDEV_LINUX_TAP EH_DBG_WARNING
#endif

static const struct ethernet_trait * s_ethernet_trait;
static ehip_netdev_t * s_netdev = NULL;
static int tap_fd;
#define TAP_NAME "tap0"

static void tap_rx_process(ehip_netdev_t *netdev){
    ehip_buffer_t *ehip_buf;
    uint8_t *buffer;
    ssize_t len;
    do{
        ehip_buf = ehip_buffer_new(netdev->attr.buffer_type, 0);
        if(eh_ptr_to_error(ehip_buf) < 0){
            eh_mwarnfl(NETDEV_LINUX_TAP, "ehip_buffer_new failed %d", eh_ptr_to_error(ehip_buf));
            /* 理论上应该过一会来继续读取，比如触发一个100us定时器 */
            break;
        }
        buffer = ehip_buffer_get_buffer_ptr(ehip_buf);
        len = read(tap_fd, buffer, ehip_buffer_get_buffer_size(ehip_buf));
        if(len <= 0){
            if(errno != EAGAIN){
                eh_mwarnfl(NETDEV_LINUX_TAP, "read tap_fd failed %s", strerror(errno));
            }
            ehip_buffer_free(ehip_buf);
            break;
        }
        ehip_buffer_payload_append(ehip_buf, (ehip_buffer_size_t)len);
        ehip_buf->netdev = netdev;
        ehip_buf->protocol = eth_hdr_ptype_get((struct eth_hdr *)ehip_buffer_get_payload_ptr(ehip_buf));
        eh_mdebugfl(NETDEV_LINUX_TAP, "eth rx:%d@|%.*hhq|", len, len, buffer);
        ehip_rx(ehip_buf);
    }while(1);

}

static void tap_epool_callback(uint32_t events, void *arg){
    (void)arg;
    if(events & EPOLLOUT){
        ehip_queue_tx_wakeup(s_netdev);
    }
    if(events & EPOLLIN){
        tap_rx_process(s_netdev);
    }
}

static struct epoll_fd_action action = {
    .callback = tap_epool_callback,
    .arg = NULL,
};




static void set_tap_mtu(const char *ifname, int mtu) {
    struct ifreq ifr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        eh_merrfl(NETDEV_LINUX_TAP, "socket failed %s", strerror(errno));
        return ;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ifr.ifr_mtu = mtu;

    if (ioctl(sock, SIOCSIFMTU, &ifr) < 0) {
        eh_merrfl(NETDEV_LINUX_TAP, "ioctl SIOCSIFMTU failed %s", strerror(errno));
        close(sock);
        return ;
    }
    
    close(sock);
}

static int tap_up(ehip_netdev_t *netdev){
    int ret;
    struct ifreq ifr;
    int flags;
    tap_fd = open("/dev/net/tun", O_RDWR);
    if(tap_fd < 0){
        eh_merrfl(NETDEV_LINUX_TAP, "open /dev/net/tun failed %s", strerror(errno));
        return -1;
    }
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, TAP_NAME);
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if(ioctl(tap_fd, TUNSETIFF, (void *)&ifr) < 0){
        eh_merrfl(NETDEV_LINUX_TAP, "ioctl TUNSETIFF failed %s", strerror(errno));
        close(tap_fd);
        return -1;
    }
    flags = fcntl(tap_fd, F_GETFL, 0);
    fcntl(tap_fd, F_SETFL, flags | O_NONBLOCK);

    set_tap_mtu(TAP_NAME, EHIP_ETH_FRAME_MAX_LEN - EHIP_ETH_FRAME_CRC_LEN - sizeof(struct eth_hdr));

    ret = epoll_hub_add_fd(tap_fd, EPOLLIN | EPOLLOUT | EPOLLET, &action);
    if(ret < 0){
        eh_merrfl(NETDEV_LINUX_TAP, "epoll_hub_add_fd failed %s", strerror(errno));
        close(tap_fd);
        return -1;
    }
    ehip_netdev_set_link_status(netdev, true);

    return 0;
}

static void tap_down(ehip_netdev_t *netdev){
    ehip_netdev_set_link_status(netdev, false);
    epoll_hub_del_fd(tap_fd);
    close(tap_fd);  
}

static int tap_ctrl(ehip_netdev_t *netdev, uint32_t cmd, void *arg){
    (void)netdev;
    (void)cmd;
    (void)arg;
    return 0;
}

static int tap_start_xmit(ehip_netdev_t *netdev, ehip_buffer_t *buf){
    (void)netdev;
    ssize_t len;
    int retv = 0;
    eh_mdebugfl(NETDEV_LINUX_TAP, "eth tx:%d@|%.*hhq|", ehip_buffer_get_payload_size(buf), ehip_buffer_get_payload_size(buf), ehip_buffer_get_payload_ptr(buf));
    len = write(tap_fd, ehip_buffer_get_payload_ptr(buf), ehip_buffer_get_payload_size(buf));
    if(len < 0){
        if(errno == EAGAIN || errno == EWOULDBLOCK){
            retv = EH_RET_BUSY;
        }else{
            eh_merrfl(NETDEV_LINUX_TAP, "write tap_fd failed %s len: %d", strerror(errno), len);
            retv = EH_RET_INVALID_STATE;
        }
    }
    ehip_buffer_free(buf);
    return retv;
}

static void tap_tx_timeout(ehip_netdev_t *netdev){
    (void)netdev;
}



static struct ehip_netdev_ops  netdev_tap_ops = {
    .ndo_up = tap_up,
    .ndo_down = tap_down,
    .ndo_ctrl = tap_ctrl,
    .ndo_start_xmit = tap_start_xmit,
    .ndo_tx_timeout = tap_tx_timeout,
};

static const struct ehip_netdev_param netdev_tap_param = {
    .name = TAP_NAME "-eth0",
    .net_max_frame_size = EHIP_ETH_FRAME_MAX_LEN,
    .ops = &netdev_tap_ops,
    .userdata = NULL
};



static int  __init linux_tap_netdev_init(void){
    ehip_netdev_t *netdev;
    static const ehip_eth_addr_t mac_addr = {{0x00, 0x04, 0x9F, 0x08, 0xB6, 0xB6}};
    int ret;
    netdev = ehip_netdev_register(EHIP_NETDEV_TYPE_ETHERNET, &netdev_tap_param);
    ret = eh_ptr_to_error(netdev);
    if(ret < 0)
        return ret;
    s_netdev = netdev;
    ehip_netdev_trait_change(
        netdev,
        ehip_netdev_trait_hw_addr(netdev),
        &mac_addr
    );
    s_ethernet_trait = ehip_netdev_to_trait(netdev);

    return ret;
}

static void __exit linux_tap_netdev_exit(void){
    ehip_netdev_unregister(s_netdev);
}

ehip_netdev_module_export(linux_tap_netdev_init, linux_tap_netdev_exit);