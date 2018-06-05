/*
* Copyright (C) 2009 Realtek Semiconductor Corp.
* All Rights Reserved.
*
* This program is the proprietary software of Realtek Semiconductor
* Corporation and/or its licensors, and only be used, duplicated,
* modified or distributed under the authorized license from Realtek.
*
* ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER 
* THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
* 
* $Revision: 8334 $
* $Date: 2010-02-09 13:22:42 +0800 (Tue, 09 Feb 2010) $
*
* Purpose :  ASIC-level driver implementation for trunk (link aggregation).
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/

#include <asicdrv/rtl8316d_types.h>
#include <rtk_types.h>
#include <rtl8316d_asicdrv_trunk.h>
#include <rtk_error.h>
#include <rtk_api.h>

static rtk_trunk_hashVal2Port_t distri[RTK_TRUNK_GROUP_MAX];
static uint8 trunk_distri_first = TRUE;

rtk_api_ret_t rtk_trunk_distributionAlgorithm_get(rtk_trunk_group_t trk_gid, rtk_trunk_hashVal2Port_t *pAlgo_bitmask)
{
    int8 i;
    
    if (trunk_distri_first == TRUE)
    {
        for (i = 0; i < RTK_TRUNK_GROUP_MAX; i++)
            distri[i].value[0] = 0;
        
        trunk_distri_first = FALSE;
    }

    memcpy(pAlgo_bitmask, &distri[trk_gid], sizeof(rtk_trunk_hashVal2Port_t));
    
    return RT_ERR_OK;
}

/*
    7 bits mask for link aggregation group0 hash parameter selection {DIP, SIP, DMAC, SMAC, SPA}
    <nl><tab>0b0000001: SPA
    <nl><tab>0b0000010: SMAC
    <nl><tab>0b0000100: DMAC
    <nl><tab>0b0001000: SIP
    <nl><tab>0b0010000: DIP
    <nl><tab>0b0100000: TCP/UDP Source Port
    <nl><tab>0b1000000: TCP/UDP Destination Port
    <nl>Example:
    <nl><tab>0b0000011: SMAC & SPA
    <nl>Note that it could be an arbitrary combination or independent set
 */

rtk_api_ret_t rtk_trunk_distributionAlgorithm_set(rtk_trunk_group_t trk_gid, rtk_trunk_hashVal2Port_t algo_bitmask)
{
    int8 i;
    
    if (trunk_distri_first == TRUE)
    {
        for (i = 0; i < RTK_TRUNK_GROUP_MAX; i++)
            distri[i].value[0] = 0;
        
        trunk_distri_first = FALSE;
    }

    memcpy(&distri[trk_gid], &algo_bitmask, sizeof(rtk_trunk_hashVal2Port_t));

    return RT_ERR_OK;
}

int32 rtk_web_trunkRepresentPortNum_get(rtk_portmask_t portmask, rtk_port_t* represent_port)
{
    uint8 ucPortId = 0;

    if (portmask.bits[0] == 0)
        return RT_ERR_LA_TRUNK_NOT_EXIST;
    
    for (ucPortId = 0; ucPortId < RTL8316D_MAX_PHY_PORT; ++ucPortId) 
    {
        if (portmask.bits[ucPortId/32] & (1 << (ucPortId % 32))) 
        {
            *represent_port = ucPortId;
            break;
        }        
    }
    
    return RT_ERR_OK;
}


