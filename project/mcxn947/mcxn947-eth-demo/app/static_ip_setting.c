/**
 * @file static_ip_setting.c
 * @brief 设置一些静态ip和路由
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-05-15
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */

#include <eh.h>
#include <eh_types.h>
#include <eh_error.h>
#include <eh_module.h>
#include <eh_event.h>
#include <eh_event_flags.h>
#include <eh_signal.h>
#include <eh_debug.h>
#include <ehip_netdev_trait.h>
#include <ehip_netdev_tool.h>
#include <ehip-ipv4/route.h>
#include <ehip-ipv4/ip.h>

#include "global_signal.h"

struct ip_setting{
    ipv4_addr_t ip;
    uint8_t     mask_len;
};

static ehip_netdev_t *eth0_netdev = NULL;
static ehip_netdev_t *loopback_netdev = NULL;
static eh_flags_t eth0_last_flags_status = 0;

EH_DEFINE_SIGNAL( sig_eth0_ip_add );
EH_DEFINE_SIGNAL( sig_eth0_ip_del );

static struct route_info eth0_route_info_tab[] = {
    {
        /* 局域网路由 */
        .dst_addr = ipv4_make_addr(192,168,12,0),
        .gateway = ipv4_make_addr(0,0,0,0),
        .metric = 200,
        .mask_len = 24,
        .src_addr = ipv4_make_addr(0,0,0,0),
        .netdev = NULL,
    },{
        /* 外网路由 */
        .dst_addr = ipv4_make_addr(0,0,0,0),
        .gateway = ipv4_make_addr(192,168,12,6),
        .metric = 200,
        .mask_len = 0,
        .src_addr = ipv4_make_addr(0,0,0,0),
        .netdev = NULL,
    },{
        /* 局域网路由 */
        .dst_addr = ipv4_make_addr(192,168,9,0),
        .gateway = ipv4_make_addr(0,0,0,0),
        .metric = 200,
        .mask_len = 24,
        .src_addr = ipv4_make_addr(0,0,0,0),
        .netdev = NULL,
    }
};

struct ip_setting eth0_main_ip[] = {
    {
        .ip = ipv4_make_addr(192,168,12,88),
        .mask_len = 24
    },{
        .ip = ipv4_make_addr(192,168,9,66),
        .mask_len = 24
    },{
        .ip = ipv4_make_addr(192,168,9,67),
        .mask_len = 24
    }
};

#define ETH0_IP_SETTING_NUM  EH_ARRAY_SIZE(eth0_main_ip)

struct ip_setting eth0_sub_ip = {
    .ip = ipv4_make_addr(192,168,12,89),
    .mask_len = 24
};

#define ETH0_ROUTE_TAB_SIZE  EH_ARRAY_SIZE(eth0_route_info_tab)

static uint32_t ip_addr_setting_inx = 0;

void slot_functhion_eth0_link_status_changed(eh_event_t *e, void *slot_param){
    (void)e;
    (void)slot_param;
    int ret;
    eh_flags_t eth0_flags_status = ehip_netdev_flags_get(eth0_netdev);
    if(eth0_flags_status ==  eth0_last_flags_status)
        return ;

    do{    
        if((eth0_flags_status & EHIP_NETDEV_STATUS_LINK) != (eth0_last_flags_status & EHIP_NETDEV_STATUS_LINK)){
            const char *link_status;
            struct ipv4_netdev* eth0_ipv4_netdev;
            eth0_ipv4_netdev = ehip_netdev_trait_ipv4_dev(eth0_netdev);
            if(eth0_ipv4_netdev == NULL)
                break;
            if(eth0_flags_status & EHIP_NETDEV_STATUS_LINK){
                /* link */
                link_status = "link";
                struct ip_setting *setting = &eth0_main_ip[ip_addr_setting_inx%ETH0_IP_SETTING_NUM];
                eh_minfofl( STATIC_IP_SETTING, "new main ip:" IPV4_FORMATIO" mark: "IPV4_FORMATIO, ipv4_formatio(setting->ip), ipv4_formatio(0xffffffff >>  setting->mask_len) );
                ipv4_netdev_set_main_addr(eth0_ipv4_netdev, setting->ip, setting->mask_len);
                ipv4_netdev_set_sub_addr(eth0_ipv4_netdev, eth0_sub_ip.ip, eth0_sub_ip.mask_len);
                for(size_t i=0; i<ETH0_ROUTE_TAB_SIZE; i++){
                    eth0_route_info_tab[i].netdev = eth0_netdev;
                    ret = ipv4_route_add(eth0_route_info_tab + i);
                    if(ret < 0)
                        eh_mwarnfl( STATIC_IP_SETTING, "ipv4_route_add error ret = %d", ret);
                }
                eh_signal_notify(&sig_eth0_ip_add);
            }else{
                link_status = "unlink";
                for(size_t i=0; i<ETH0_ROUTE_TAB_SIZE; i++){
                    eth0_route_info_tab[i].netdev = eth0_netdev;
                    ipv4_route_del(eth0_route_info_tab + i);
                }
                ipv4_netdev_del_addr(eth0_ipv4_netdev, eth0_main_ip[ip_addr_setting_inx%ETH0_IP_SETTING_NUM].ip);
                ip_addr_setting_inx++;
                ipv4_netdev_del_addr(eth0_ipv4_netdev, eth0_sub_ip.ip);
                eh_signal_notify(&sig_eth0_ip_del);
            }
            eh_infofl("eth0 link status changed: %s", link_status);
        }

    }while(0);
    eth0_last_flags_status = eth0_flags_status;
}


EH_DEFINE_SLOT(slot_eth0_link_status_changed, slot_functhion_eth0_link_status_changed, NULL);

static int __init static_ip_init(void){
    int ret;
    loopback_netdev = ehip_netdev_tool_find("lo");
    ehip_netdev_tool_up(loopback_netdev);
    eth0_netdev = ehip_netdev_tool_find("eth0");
    if(eth0_netdev == NULL)
        return EH_RET_INVALID_STATE;
    ret = ehip_netdev_tool_up(eth0_netdev);
    if(ret < 0)
        return ret;

    eh_signal_slot_connect(ehip_netdev_signal_flags(eth0_netdev), &slot_eth0_link_status_changed);
    slot_functhion_eth0_link_status_changed(&ehip_netdev_signal_flags(eth0_netdev)->event, NULL);
    eh_signal_register(&sig_eth0_ip_add);
    eh_signal_register(&sig_eth0_ip_del);
    return 0; 
}

static void __exit static_ip_exit(void){
    eh_signal_unregister(&sig_eth0_ip_add);
    eh_signal_unregister(&sig_eth0_ip_del);
    eh_signal_slot_disconnect(&slot_eth0_link_status_changed);
    ehip_netdev_tool_down(eth0_netdev);
    ehip_netdev_tool_down(loopback_netdev);
}

eh_module_level0_export(static_ip_init, static_ip_exit);