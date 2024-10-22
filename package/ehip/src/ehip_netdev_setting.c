/**
 * @file ehip_netdev_setting.c
 * @brief 网络设备配置基础结构
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @version 1.0
 * @date 2024-10-20
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 * @par 修改日志:
 */

#include "ehip_netdev_setting.h"
#include "eh_list.h"
#include <stdint.h>


int ehip_netdev_setting_item_add(struct ehip_netdev_setting *setting, struct ehip_netdev_setting_item *item){
    struct ehip_netdev_setting_item *pos_item;
    
    ehip_netdev_setting_for_each(setting, pos_item){
        if(pos_item->type > item->type)
            break;
    }
    eh_list_add_tail(&item->node, &pos_item->node);
    return 0;
}

