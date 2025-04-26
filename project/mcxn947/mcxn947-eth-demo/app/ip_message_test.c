/**
 * @file ip_message_test.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-02-08
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */


#include <eh.h>
#include <eh_swab.h>
#include <eh_module.h>
#include <eh_debug.h>
#include <eh_mem_pool.h>
#include <eh_sleep.h>
#include <ehip_buffer.h>
#include <ehip-ipv4/ip.h>
#include <ehip-ipv4/ip_message.h>
#include <ehip_netdev_tool.h>



/* 构造一个多片的IP报文 */
static struct ip_message* ip_message_test_create_multi_part_rx_ip_message(uint32_t ip_message_size, ehip_buffer_size_t mtu_size){
    int ret;
    struct ip_message*  new_ip_message = NULL;
    struct ip_hdr ip_hdr;
    ehip_buffer_t *buffer;
    uint8_t *write;
    ehip_buffer_size_t max_fragment_size = mtu_size - (ehip_buffer_size_t)sizeof(struct ip_hdr);
    ehip_buffer_size_t fragment_size;
    ehip_buffer_size_t offset = 0;
    ehip_buffer_size_t end = (ehip_buffer_size_t)ip_message_size;
    int fragment_i = 0;

    while(end-offset > 0){
        fragment_size = end - offset > max_fragment_size ? max_fragment_size : (ehip_buffer_size_t)(end - offset);
        buffer = ehip_buffer_new(EHIP_BUFFER_TYPE_GENERAL_FRAME, 0);
        if(eh_ptr_to_error(buffer)){
            eh_errfl("ehip_buffer_new fail");
            goto error;
        }
        ip_hdr.version = 4;
        ip_hdr.ihl = sizeof(struct ip_hdr) >> 2;
        ip_hdr.frag_off = 0;
        ip_hdr.tot_len = eh_hton16((ehip_buffer_size_t)(fragment_size + (ehip_buffer_size_t)sizeof(struct ip_hdr)));

        ipv4_hdr_frag_set(&ip_hdr, offset, fragment_size == (end - offset) ? 0 : IP_FRAG_MF);

        write = ehip_buffer_payload_append(buffer, fragment_size);
        for(int i = 0; i < fragment_size; i++,fragment_i++)
            write[i] = (uint8_t)fragment_i;

        if(!new_ip_message){
            new_ip_message = ip_message_rx_new_fragment(NULL, buffer, &ip_hdr);
        }else{
            ret = ip_message_rx_merge_fragment(new_ip_message, buffer, &ip_hdr);
            if(ret < 0){
                eh_errfl("ip_message_rx_merge_fragment fail ret = %d", ret);
                goto error;
            }
        }
        
        offset += fragment_size;
    }
    return new_ip_message;

error :
    if(new_ip_message)
        ip_message_free(new_ip_message);
    return NULL;
}


static int  ip_message_test_task(void *arg){
    (void) arg;
    struct ip_message* ip_message;
    // struct ip_message* ip_message_dup;
    ehip_buffer_t *pos_buffer;
    int int_tmp_i, int_tmp_sort_i;

    {
        int fragment_i = 0;
        ip_message = ip_message_test_create_multi_part_rx_ip_message(512, 64+20);
        if(ip_message == NULL){
            eh_errfl("ip_message_test_create_multi_part_rx_ip_message fail");
            return -1;
        }

        ip_message_rx_fragment_for_each(pos_buffer, int_tmp_i, int_tmp_sort_i, ip_message){
            int fragment_size = ehip_buffer_get_payload_size(pos_buffer);
            for(int i = 0; i < fragment_size; i++,fragment_i++){
                if(ehip_buffer_get_payload_ptr(pos_buffer)[i] != (uint8_t)fragment_i){
                    eh_errfl("fragment %d data error %d", int_tmp_i, i);
                    ip_message_free(ip_message);
                    goto exit;
                }
            }
        }
        eh_minfoln(IP_MESSAGE_TEST, "ip_message_rx_fragment_for_each TEST PASS");
        ip_message_free(ip_message);
    }

    {
        int fragment_i = 0;
        ip_message = ip_message_test_create_multi_part_rx_ip_message(512, 64+20);
        if(ip_message == NULL){
            eh_errfl("ip_message_test_create_multi_part_rx_ip_message fail");
            return -1;
        }

        EH_DBG_ERROR_EXEC(ip_message_rx_data_tail_trim(ip_message, 128) < 0, 
            ip_message_free(ip_message);
            goto exit;
        );

        if(ip_message_rx_data_size(ip_message) != 512 - 128){
            eh_errfl("ip_message_rx_data_size error %d", ip_message_rx_data_size(ip_message));
            ip_message_free(ip_message);
            goto exit;
        }

        ip_message_rx_fragment_for_each(pos_buffer, int_tmp_i, int_tmp_sort_i, ip_message){
            int fragment_size = ehip_buffer_get_payload_size(pos_buffer);
            for(int i = 0; i < fragment_size; i++,fragment_i++){
                if(ehip_buffer_get_payload_ptr(pos_buffer)[i] != (uint8_t)fragment_i){
                    eh_errfl("fragment %d data error %d", int_tmp_i, i);
                    ip_message_free(ip_message);
                    goto exit;
                }
            }
        }
        eh_minfoln(IP_MESSAGE_TEST, "ip_message_rx_data_tail_trim TEST PASS");
        ip_message_free(ip_message);
    }


exit:
    return 0;
}

static int __init ip_message_test_init(void){
    eh_infofl("test init");
    eh_task_create("ip_message_test_task", EH_TASK_FLAGS_DETACH, 4096, NULL, ip_message_test_task);
    return 0;
}

eh_module_level9_export(ip_message_test_init, NULL);