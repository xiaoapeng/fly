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

static ehip_netdev_t *utun_netdev = NULL;
static ehip_netdev_t *loopback_netdev = NULL;
static eh_flags_t utun_last_flags_status = 0;

EH_DEFINE_SIGNAL( sig_utun_ip_add );
EH_DEFINE_SIGNAL( sig_utun_ip_del );

static struct route_info utun_route_info_tab[] = {
    {
        /* 局域网路由 */
        .dst_addr = ipv4_make_addr(10,10,0,0),
        .gateway = ipv4_make_addr(0,0,0,0),
        .metric = 200,
        .mask_len = 16,
        .src_addr = ipv4_make_addr(0,0,0,0),
        .netdev = NULL,
    },{
        /* 外网路由 */
        .dst_addr = ipv4_make_addr(0,0,0,0),
        .gateway = ipv4_make_addr(10,10,0,1),
        .metric = 200,
        .mask_len = 0,
        .src_addr = ipv4_make_addr(0,0,0,0),
        .netdev = NULL,
    }
};

struct ip_setting utun_main_ip[] = {
    {
        .ip = ipv4_make_addr(10,10,0,88),
        .mask_len = 16
    }
};

#define utun_IP_SETTING_NUM  EH_ARRAY_SIZE(utun_main_ip)

struct ip_setting utun_sub_ip = {
    .ip = ipv4_make_addr(10,10,0,89),
    .mask_len = 16
};

#define utun_ROUTE_TAB_SIZE  EH_ARRAY_SIZE(utun_route_info_tab)

static uint32_t ip_addr_setting_inx = 0;

void slot_functhion_utun_link_status_changed(eh_event_t *e, void *slot_param){
    (void)e;
    (void)slot_param;
    int ret;
    eh_flags_t utun_flags_status = ehip_netdev_flags_get(utun_netdev);
    if(utun_flags_status ==  utun_last_flags_status)
        return ;

    do{    
        if((utun_flags_status & EHIP_NETDEV_STATUS_LINK) != (utun_last_flags_status & EHIP_NETDEV_STATUS_LINK)){
            const char *link_status;
            struct ipv4_netdev* utun_ipv4_netdev;
            utun_ipv4_netdev = ehip_netdev_trait_ipv4_dev(utun_netdev);
            if(utun_ipv4_netdev == NULL)
                break;
            if(utun_flags_status & EHIP_NETDEV_STATUS_LINK){
                /* link */
                link_status = "link";
                struct ip_setting *setting = &utun_main_ip[ip_addr_setting_inx%utun_IP_SETTING_NUM];
                eh_minfofl( STATIC_IP_SETTING, "new main ip:" IPV4_FORMATIO" mark: "IPV4_FORMATIO, ipv4_formatio(setting->ip), ipv4_formatio(0xffffffff >>  setting->mask_len) );
                ipv4_netdev_set_main_addr(utun_ipv4_netdev, setting->ip, setting->mask_len);
                ipv4_netdev_set_sub_addr(utun_ipv4_netdev, utun_sub_ip.ip, utun_sub_ip.mask_len);
                for(size_t i=0; i<utun_ROUTE_TAB_SIZE; i++){
                    utun_route_info_tab[i].netdev = utun_netdev;
                    ret = ipv4_route_add(utun_route_info_tab + i);
                    if(ret < 0)
                        eh_mwarnfl( STATIC_IP_SETTING, "ipv4_route_add error ret = %d", ret);
                }
                eh_signal_notify(&sig_utun_ip_add);
            }else{
                link_status = "unlink";
                for(size_t i=0; i<utun_ROUTE_TAB_SIZE; i++){
                    utun_route_info_tab[i].netdev = utun_netdev;
                    ipv4_route_del(utun_route_info_tab + i);
                }
                ipv4_netdev_del_addr(utun_ipv4_netdev, utun_main_ip[ip_addr_setting_inx%utun_IP_SETTING_NUM].ip);
                ip_addr_setting_inx++;
                ipv4_netdev_del_addr(utun_ipv4_netdev, utun_sub_ip.ip);
                eh_signal_notify(&sig_utun_ip_del);
            }
            eh_infofl("utun link status changed: %s", link_status);
        }

    }while(0);
    utun_last_flags_status = utun_flags_status;
}


EH_DEFINE_SLOT(slot_utun_link_status_changed, slot_functhion_utun_link_status_changed, NULL);

static int __init static_ip_init(void){
    int ret;
    loopback_netdev = ehip_netdev_tool_find("lo");
    ehip_netdev_tool_up(loopback_netdev);
    utun_netdev = ehip_netdev_tool_find("mac-utun");
    if(utun_netdev == NULL)
        return EH_RET_INVALID_STATE;
    ret = ehip_netdev_tool_up(utun_netdev);
    if(ret < 0)
        return ret;

    ret = eh_signal_slot_connect(ehip_netdev_signal_flags(utun_netdev), &slot_utun_link_status_changed);
    if(ret < 0){
        return ret;
    }
    slot_functhion_utun_link_status_changed(&ehip_netdev_signal_flags(utun_netdev)->event, NULL);
    return 0;
}

static void __exit static_ip_exit(void){
    eh_signal_slot_disconnect(ehip_netdev_signal_flags(utun_netdev), &slot_utun_link_status_changed);
    ehip_netdev_tool_down(utun_netdev);
    ehip_netdev_tool_down(loopback_netdev);
}

eh_module_level0_export(static_ip_init, static_ip_exit);