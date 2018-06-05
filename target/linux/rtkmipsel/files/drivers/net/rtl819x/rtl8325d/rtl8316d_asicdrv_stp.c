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
* Purpose : ASIC-level driver implementation for Spanning Tree Protocol.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/
#include <rtk_types.h>
#include <rtk_error.h>
#include <rtk_api.h>
#include <rtl8316d_reg_struct.h>
#include <asicdrv/rtl8316d_types.h> /*SDK*/


/* Function Name:
 *      rtk_stp_init
 * Description:
 *      Initialize stp module of the specified device.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  - Successfully add the acl rule
 *      RT_ERR_FAILED   - 
 * Note:
 *      Initialize stp module before calling any vlan APIs
 */
rtk_api_ret_t rtk_stp_init(void)
{

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_stp_mstpState_set
 * Description:
 *      Configure spanning tree state per each port.
 * Input:
 *      port - port ID
 *      msti - Multiple spanning tree instance.
 *      stp_state - Spanning tree state for msti
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  - Successfully add the acl rule
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_MSTI -Invalid msti parameter
 *      RT_ERR_MSTP_STATE - Invalid STP state.
 * Note:
 *    System supports per-port multiple spanning tree state for each msti. 
 *    There are four states supported by ASIC.
 *      -STP_STATE_DISABLED
 *      -STP_STATE_BLOCKING
 *      -STP_STATE_LEARNING
 *      -STP_STATE_FORWARDING
 */
rtk_api_ret_t rtk_stp_mstpState_set(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t stp_state)
{
//    rtk_api_ret_t retVal;
    rtk_reg_list_t regAddr;
    uint32 regVal;
    
    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(msti >= RTK_MAX_NUM_OF_MSTI)
        return RT_ERR_MSTI;

    if(stp_state >= STP_STATE_END)
        return RT_ERR_MSTP_STATE;

    regAddr = SPANNING_TREE_PORT_STATE_CONTROL0A + port/15 + msti*2;
    reg_read(RTL8316D_UNIT, regAddr, &regVal);
    regVal &= ~(0x3 << (port%15) * 2);
    regVal |= stp_state << ((port%15) * 2);
    reg_write(RTL8316D_UNIT, regAddr, regVal);
    
    return RT_ERR_OK;
}    

/* Function Name:
 *      rtk_stp_mstpState_get
 * Description:
 *      Configure spanning tree state per each port.
 * Input:
 *      port - port ID
 *      msti - Multiple spanning tree instance.
 * Output:
 *      pStp_state - Spanning tree state for msti
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_MSTI -Invalid msti parameter
 * Note:
 *    System supports per-port multiple spanning tree state for each msti. 
 *    There are four states supported by ASIC.
 *      -STP_STATE_DISABLED
 *      -STP_STATE_BLOCKING
 *      -STP_STATE_LEARNING
 *      -STP_STATE_FORWARDING
 */
rtk_api_ret_t rtk_stp_mstpState_get(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t *pStp_state)
{
//    rtk_api_ret_t retVal;
    rtk_reg_list_t regAddr;
    uint32 regVal;
    
    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(msti >= RTK_MAX_NUM_OF_MSTI)
        return RT_ERR_MSTI;

    if(pStp_state == NULL)
        return RT_ERR_NULL_POINTER;

    regAddr = SPANNING_TREE_PORT_STATE_CONTROL0A + port/15 + msti*2;
    reg_read(RTL8316D_UNIT, regAddr, &regVal);
    *pStp_state = (regVal >> ((port%15) * 2)) & 0x3;
    
    return RT_ERR_OK;
}    


