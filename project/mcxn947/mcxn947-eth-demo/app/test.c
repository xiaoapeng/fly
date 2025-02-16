/**
 * @file test.c
 * @brief 测试代码
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-10-06
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#include <stdint.h>

#include <eh.h>
#include <eh_swab.h>
#include <eh_module.h>
#include <eh_error.h>
#include <eh_mem_pool.h>
#include <eh_debug.h>
#include <eh_sleep.h>
#include <eh_llist.h>
#include <eh_signal.h>
#include <ehip_netdev_tool.h>
#include <ehip_netdev_trait.h>
#include <ehip-ipv4/ip.h>
#include <ehip-ipv4/route.h>
#include <ehip-ipv4/arp.h>
#include <ehip_chksum.h>

#include "button.h"

void mem_fill(void* mem, size_t size, char start){
    for(size_t i=0; i<size; i++){
        ((char *)mem)[i] = start++;
    }
}

int mem_alloc_test(void **test_ptr, size_t num, eh_mem_pool_t test_pool){
    for(size_t i=0; i<num; i++){
        test_ptr[i] = eh_mem_pool_alloc(test_pool);
        if(test_ptr[i] == NULL) return -1;
    }
    return 0;
}

void mem_free_test(void **test_ptr, int *free_tab, eh_mem_pool_t test_pool){
    int *free_tab_start;
    for(free_tab_start = free_tab; *free_tab_start >= 0; free_tab_start++){
        eh_mem_pool_free(test_pool, test_ptr[*free_tab_start]);
    }
}


void mem_data_fill_test(void **test_ptr, size_t num, size_t size, char *fill_data_start_val){
    char fill_data;
    for(size_t i=0;i<num;i++){
        fill_data = fill_data_start_val[i];
        for(size_t j=0;j<size;j++){
            ((char*)test_ptr[i])[j] = fill_data++;
        }
    }
}

int mem_data_fill_check(void **test_ptr, size_t num, size_t size, char *fill_data_start_val){
    char fill_data;
    for(size_t i=0;i<num;i++){
        fill_data = fill_data_start_val[i];
        for(size_t j=0;j<size;j++){
            if(((char*)test_ptr[i])[j] != fill_data++){
                return -1;
            }
        }
    }
    return 0;
}

void mem_pool_test(void){
    
#define TEST_NUM 16
#define TEST_SIZE 128
    eh_mem_pool_t test_pool;
    void *test_ptr[TEST_NUM];
    eh_infoln("pool alloc test:");

    test_pool = eh_mem_pool_create(32, TEST_SIZE, TEST_NUM);
    EH_DBG_ERROR_EXEC(eh_ptr_to_error(test_pool) < 0, goto error);

    {
        EH_DBG_ERROR_EXEC(mem_alloc_test(test_ptr, TEST_NUM, test_pool) != 0, goto test_error);
        mem_data_fill_test(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef");
        EH_DBG_ERROR_EXEC(mem_data_fill_check(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef") != 0, goto test_error);
        int free_test_tab_test_1[] = {7,0,11,12,13,3,4,9,10,5,6,8,14,15,1,2,-1};
        mem_free_test(test_ptr, free_test_tab_test_1, test_pool);
        eh_infoln("1 Pass..");
    }
    {
        EH_DBG_ERROR_EXEC(mem_alloc_test(test_ptr, TEST_NUM, test_pool) != 0, goto test_error);
        mem_data_fill_test(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef");
        EH_DBG_ERROR_EXEC(mem_data_fill_check(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef") != 0, goto test_error);
        int free_test_tab_test_1[] = {4,9,13,3,8,10,5,6,7,0,11,12,14,15,1,2,-1};
        mem_free_test(test_ptr, free_test_tab_test_1, test_pool);
        eh_infoln("2 Pass..");
    }
    {
        EH_DBG_ERROR_EXEC(mem_alloc_test(test_ptr, TEST_NUM, test_pool) != 0, goto test_error);
        mem_data_fill_test(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef");
        EH_DBG_ERROR_EXEC(mem_data_fill_check(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef") != 0, goto test_error);
        int free_test_tab_test_1[] = {4,9,13,3,8,10,5,6,7,0,11,12,14,15,1,2,-1};
        mem_free_test(test_ptr, free_test_tab_test_1, test_pool);
        eh_infoln("3 Pass..");
    }
    {
        EH_DBG_ERROR_EXEC(mem_alloc_test(test_ptr, TEST_NUM, test_pool) != 0, goto test_error);
        mem_data_fill_test(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef");
        EH_DBG_ERROR_EXEC(mem_data_fill_check(test_ptr, TEST_NUM, TEST_SIZE, "123456789abcdef") != 0, goto test_error);
        int free_test_tab_test_1[] = {4,9,13,3,8,10,5,6,7,0,11,12,14,15,1,2,-1};
        mem_free_test(test_ptr, free_test_tab_test_1, test_pool);
        eh_infoln("4 Pass..");
    }

    
    
    eh_mem_pool_destroy(test_pool);

    eh_infoln("Pass..");
    return ;
test_error:
    eh_mem_pool_destroy(test_pool);
error:
    eh_errln("Fail..");
}

void swab_test(void){
    struct test{
        char test_data[16];
    };
    struct test td0 = {
        .test_data = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'}
    };

    struct test td1 = eh_complex_swab(td0);

    eh_debugln("test eh_swab16 %0#hx", eh_hton((short)0x1234));
    eh_debugln("test eh_swab32 %0#x", eh_hton((int)0x12345678));
    eh_debugln("test eh_swab64 %0#llx", eh_hton(0x1234567887654321ULL));
    eh_debugln("td0 |%.*hhq|", sizeof(td0),&td0);
    eh_debugln("td1 |%.*hhq|", sizeof(td1),&td1);

    int64_t t1 = 0xFFFFFFFFF;
    int32_t t2 = (int32_t)t1;
    eh_debugln("t11 %lld t2 %d", t1, t2);
}

int llist_test(void){
    struct eh_llist_head head = EH_LLIST_HEAD_INIT(head);
    struct eh_llist_node *pos, *n, *prev;
    struct test_node{
        struct eh_llist_node node;
        int data;
    };
    struct test_node test_data[30];
    for(int i=0;i<30;i++){
        test_data[i].data = i;
        eh_llist_add(&test_data[i].node, &head);
    }

    prev = (struct eh_llist_node *)&head;
    eh_llist_for_each_safe(pos, n, head.first){
        eh_llist_del(prev, pos);
    }

    if(eh_llist_empty(&head)){
        eh_infoln("llist empty");
    }

    for(int i=0;i<30;i++){
        test_data[i].data = i;
        eh_llist_push(&test_data[i].node, &head);
    }
    eh_debugfl("debug!");
    while((pos = eh_llist_pop(&head))){
        eh_debugln("stack data %d", eh_llist_entry(pos, struct test_node, node)->data);
    }


    for(int i=0;i<30;i++){
        test_data[i].data = i;
        eh_llist_enqueue(&test_data[i].node, &head);
    }

    while((pos = eh_llist_dequeue(&head))){
        eh_debugln("queue data %d", eh_llist_entry(pos, struct test_node, node)->data);
    }
    
    
    for(int i=0;i<30;i++){
        test_data[i].data = i;
        eh_llist_push(&test_data[i].node, &head);
    }

    while((pos = eh_llist_pop(&head))){
        eh_debugln("stack data %d", eh_llist_entry(pos, struct test_node, node)->data);
    }

    return 0;
}


void  slot_function_arp_table_dump(eh_event_t *e, void *slot_param){
    (void)e;
    (void)slot_param;
    uint32_t sta = button_sw3_sta();
    if(sta == 0 || e == &signal_arp_table_changed.event){
        eh_infofl("arp table changed:");
        arp_table_dump();
    }
}

EH_DEFINE_SLOT(slot_arp_table_changed, slot_function_arp_table_dump, NULL);
EH_DEFINE_SLOT(slot_button_sw3, slot_function_arp_table_dump, NULL);



void arp_test(void){
    // ehip_netdev_t * eth0_netdev;
    // eth0_netdev = ehip_netdev_tool_find("eth0");

    eh_signal_slot_connect(&signal_arp_table_changed, &slot_arp_table_changed);
    eh_signal_slot_connect(&button_sw3_signal, &slot_button_sw3);

    // eh_usleep(1000*1000*2);
    // arp_query(eth0_netdev, ipv4_make_addr(192,168,12,12), -1);
    // arp_query(eth0_netdev, ipv4_make_addr(192,168,12,6), -1);
    // arp_query(eth0_netdev, ipv4_make_addr(192,168,12,1), -1);
    
}

/* 
arp 测试：
sudo apt-get install arping

sudo arping -S 192.168.12.50 192.168.12.88 -C 1
sudo arping -S 192.168.12.51 192.168.12.88 -C 1
sudo arping -S 192.168.12.52 192.168.12.88 -C 1
sudo arping -S 192.168.12.53 192.168.12.88 -C 1
sudo arping -S 192.168.12.54 192.168.12.88 -C 1
sudo arping -S 192.168.12.55 192.168.12.88 -C 1
sudo arping -S 192.168.12.56 192.168.12.88 -C 1
sudo arping -S 192.168.12.57 192.168.12.88 -C 1
sudo arping -S 192.168.12.58 192.168.12.88 -C 1
sudo arping -S 192.168.12.59 192.168.12.88 -C 1
sudo arping -S 192.168.12.60 192.168.12.88 -C 1
sudo arping -S 192.168.12.61 192.168.12.88 -C 1
sudo arping -S 192.168.12.62 192.168.12.88 -C 1
sudo arping -S 192.168.12.63 192.168.12.88 -C 1
sudo arping -S 192.168.12.64 192.168.12.88 -C 1
sudo arping -S 192.168.12.65 192.168.12.88 -C 1


sudo arping -S 192.168.12.72 192.168.12.88 -C 1
sudo arping -S 192.168.12.73 192.168.12.88 -C 1
sudo arping -S 192.168.12.74 192.168.12.88 -C 1
sudo arping -S 192.168.12.75 192.168.12.88 -C 1

*/



void eth_test(void){
    struct route_info route_info;
    ehip_netdev_t * eth0_netdev;
    struct ipv4_netdev* eth0_ipv4_netdev;
    enum route_table_type route_table_type;
    ipv4_addr_t  src_addr;
    eth0_netdev = ehip_netdev_tool_find("eth0");
    eh_infoln("eth0 netdev %p", eth0_netdev);
    EH_DBG_ERROR_EXEC( ehip_netdev_tool_up(eth0_netdev) != 0, return );
    eth0_ipv4_netdev = ehip_netdev_trait_ipv4_dev(eth0_netdev);
    EH_DBG_ERROR_EXEC( eth0_ipv4_netdev == NULL, return );
    ipv4_netdev_set_main_addr(eth0_ipv4_netdev, ipv4_make_addr(192,168,12,88), 24);

    /* 局域网路由 */
    route_info.dst_addr = ipv4_make_addr(192,168,12,0);
    route_info.gateway = ipv4_make_addr(0,0,0,0);
    route_info.mask_len = 24;
    route_info.metric = 200;
    route_info.netdev = eth0_netdev;
    route_info.src_addr = ipv4_make_addr(0,0,0,0);

    EH_DBG_ERROR_EXEC( ipv4_route_add(&route_info) != 0, return );

    /* 外网路由 */
    route_info.dst_addr = ipv4_make_addr(0,0,0,0);
    route_info.gateway = ipv4_make_addr(192,168,12,6);
    route_info.mask_len = 0;
    route_info.metric = 200;
    route_info.netdev = eth0_netdev;
    route_info.src_addr = ipv4_make_addr(0,0,0,0);

    EH_DBG_ERROR_EXEC( ipv4_route_add(&route_info) != 0, return );

    route_table_type = ipv4_route_lookup(ipv4_make_addr(192,168,12,8), &route_info);
    src_addr = ipv4_route_best_src_ip(&route_info);
    eh_infoln("route table type %d", route_table_type);
    eh_infoln("route info dst %d.%d.%d.%d gateway %d.%d.%d.%d mask_len %d metric %d netdev %s src_addr %d.%d.%d.%d", 
        ipv4_addr_to_dec0(route_info.dst_addr), ipv4_addr_to_dec1(route_info.dst_addr),
        ipv4_addr_to_dec2(route_info.dst_addr), ipv4_addr_to_dec3(route_info.dst_addr),
        ipv4_addr_to_dec0(route_info.gateway), ipv4_addr_to_dec1(route_info.gateway), 
        ipv4_addr_to_dec2(route_info.gateway), ipv4_addr_to_dec3(route_info.gateway), 
        route_info.mask_len, 
        route_info.metric, route_info.netdev ? route_info.netdev->param->name : NULL,
        ipv4_addr_to_dec0(src_addr), ipv4_addr_to_dec1(src_addr), 
        ipv4_addr_to_dec2(src_addr), ipv4_addr_to_dec3(src_addr)
    );

    eh_infoln("\n");


    route_table_type = ipv4_route_lookup(ipv4_make_addr(188,16,11,8), &route_info);
    src_addr = ipv4_route_best_src_ip(&route_info);
    eh_infoln("route table type %d", route_table_type);
    eh_infoln("route info dst %d.%d.%d.%d gateway %d.%d.%d.%d mask_len %d metric %d netdev %s src_addr %d.%d.%d.%d", 
        ipv4_addr_to_dec0(route_info.dst_addr), ipv4_addr_to_dec1(route_info.dst_addr),
        ipv4_addr_to_dec2(route_info.dst_addr), ipv4_addr_to_dec3(route_info.dst_addr),
        ipv4_addr_to_dec0(route_info.gateway), ipv4_addr_to_dec1(route_info.gateway), 
        ipv4_addr_to_dec2(route_info.gateway), ipv4_addr_to_dec3(route_info.gateway), 
        route_info.mask_len, 
        route_info.metric, route_info.netdev ? route_info.netdev->param->name : NULL,
        ipv4_addr_to_dec0(src_addr), ipv4_addr_to_dec1(src_addr), 
        ipv4_addr_to_dec2(src_addr), ipv4_addr_to_dec3(src_addr)
    );

    union{
        uint8_t addr;
        struct{
            uint8_t	ihl:4,
		    version:4;
        };
    }rr = {0};
    rr.version = 0x4;
    eh_infoln("rr.addr %02x rr.ihl %02x rr.version %02x", rr.addr, rr.ihl, rr.version);
    arp_test();
    
    char buf0[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15};
    char buf1[17] = {0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15};
    char buf2[18] = {0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15};
    char buf3[19] = {0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15};
    uint16_t sum;
    
    sum = ehip_standard_chksum(0x00, buf0, 16);
    eh_debugln("buf0 sum %04hx", sum);

    sum = ehip_standard_chksum(0x00, buf1+1, 16);
    eh_debugln("buf1 sum %04hx", sum);

    sum = ehip_standard_chksum(0x00, buf2+2, 16);
    eh_debugln("buf2 sum %04hx", sum);

    sum = ehip_standard_chksum(0x00, buf3+3, 16);
    eh_debugln("buf3 sum %04hx", sum);

}


static int task_task(void *arg){
    (void) arg;
    // llist_test();
    eth_test();
    return 0;
}


static int __init test_init(void){
    eh_infoln("test init");
    eh_task_create("test", EH_TASK_FLAGS_DETACH, 4096, NULL, task_task);
    return 0;
}

eh_module_level9_export(test_init, NULL);
