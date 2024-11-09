/**
 * @file ehip_netdev_tool.c
 * @brief 网络设备控制工具
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-05
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 

 */

#include <eh_error.h>
#include "ehip_netdev_tool.h"

int ehip_netdev_tool_up(ehip_netdev_t *netdev){
    if(netdev && netdev->param && netdev->param->ops && netdev->param->ops->ndo_up)
        return netdev->param->ops->ndo_up(netdev);
    return EH_RET_INVALID_PARAM;
}

int ehip_netdev_tool_down(ehip_netdev_t *netdev){
    if(netdev && netdev->param && netdev->param->ops && netdev->param->ops->ndo_down)
        return netdev->param->ops->ndo_down(netdev);
    return EH_RET_INVALID_PARAM;
}

int ehip_netdev_tool_ctrl(ehip_netdev_t *netdev, uint32_t ctrl, void *arg){
    if(netdev && netdev->param && netdev->param->ops && netdev->param->ops->ndo_ctrl)
        return netdev->param->ops->ndo_ctrl(netdev, ctrl, arg);
    return EH_RET_INVALID_PARAM;
}



