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
* $Revision: 8134 $
* $Date: 2010-01-12 14:32:39 +0800 (Tue, 12 Jan 2010) $
*
* Purpose : ASIC-level driver implementation for Reserved Multicast Address.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/

#include <rtk_types.h>
#include <rtk_api_ext.h>
#include <asicdrv/rtl8316d_types.h> /*specify the file in SDK ASIC driver directory*/
#include <rtl8316d_reg_struct.h>
#include <rtl8316d_asicdrv_qos.h>


/* Function Name:
 *      rtk_trap_rmaAction_set
 * Description:
 *      Set reserved multicast address frame action.
 * Input:
 *      pRma_frame - Reserved multicast address.
 *      rma_action - RMA action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - Access Acl table error or parameter error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_RMA_ADDR - Invalid RMA address. 
 *      RT_ERR_RMA_ACTION - Invalid RMA action.
 * Note:
 *      The RMA action is as following:
 *          RMA_ACTION_FORWARD
 *          RMA_ACTION_DROP
 *          RMA_ACTION_TRAP2CPU
 *          RMA_ACTION_FORWARD_COPY2_CPU
 */
rtk_api_ret_t rtk_trap_rmaAction_set(rtk_mac_t *pRma_frame, rtk_trap_rma_action_t rma_action)
{
    rtk_api_ret_t retVal;

    if (pRma_frame == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    if (pRma_frame->octet[0]!=0x01&&pRma_frame->octet[1]!=0x80&&pRma_frame->octet[2]!=0xC2
        &&pRma_frame->octet[3]!=0&&pRma_frame->octet[4]!=0&&pRma_frame->octet[5]>0x3F)
        return RT_ERR_RMA_ADDR;

    if (rma_action >= RMA_ACTION_END)
        return RT_ERR_RMA_ACTION;

    if (pRma_frame->octet[5] == 0)
    {
        retVal = reg_field_write(RTL8316D_UNIT, RMA_CONTROL0, ACT0, rma_action);
    } else if (pRma_frame->octet[5] == 1) {
        retVal = reg_field_write(RTL8316D_UNIT, RMA_CONTROL0, ACT1, rma_action);
    } else if (pRma_frame->octet[5] == 2) {
        retVal = reg_field_write(RTL8316D_UNIT, RMA_CONTROL0, ACT2, rma_action);
    } else if (pRma_frame->octet[5] == 3) {
        retVal = reg_field_write(RTL8316D_UNIT, RMA_CONTROL0, ACT3, rma_action);
    } else if ((pRma_frame->octet[5] >= 4 && pRma_frame->octet[5] <= 0xD) 
                                         || pRma_frame->octet[5] == 0xF) {
        retVal = reg_field_write(RTL8316D_UNIT, RMA_CONTROL0, ACT4, rma_action);
    } else if (pRma_frame->octet[5] == 0xE) {
        retVal = reg_field_write(RTL8316D_UNIT, RMA_CONTROL1, ACT5, rma_action);
    } else if (pRma_frame->octet[5] == 0x10) {
        retVal = reg_field_write(RTL8316D_UNIT, RMA_CONTROL1, ACT6, rma_action);
    } else if (pRma_frame->octet[5] == 0x20) {
        retVal = reg_field_write(RTL8316D_UNIT, RMA_CONTROL1, ACT7, rma_action);
    } else if (pRma_frame->octet[5] == 0x21) {
        retVal = reg_field_write(RTL8316D_UNIT, RMA_CONTROL1, ACT8, rma_action);
    } else if (pRma_frame->octet[5] >= 0x22 && pRma_frame->octet[5] <= 0x2F) {
        retVal = reg_field_write(RTL8316D_UNIT, RMA_CONTROL1, ACT9, rma_action);
    } else if (pRma_frame->octet[5] >= 0x31 && pRma_frame->octet[5] <= 0x3F) {
        retVal = reg_field_write(RTL8316D_UNIT, RMA_CONTROL2, ACT10, rma_action);
    } else  {
        return RT_ERR_RMA_ADDR;
    } 

    if (retVal != SUCCESS)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_rmaAction_get
 * Description:
 *      Retrieve RMA frame action.
 * Input:
 *      pRma_frame - Reserved multicast address.
 * Output:
 *      pRma_action - RMA action.
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - Access Acl table error or parameter error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_RMA_ADDR - Invalid RMA address. 
 *      RT_ERR_NULL_POINTER - Invalid RMA address. 
 *      RT_ERR_RMA_ACTION - Invalid RMA action.
 * Note:
 *      The RMA action is as following:
 *          -RMA_ACTION_FORWARD
 *          -RMA_ACTION_DROP
 *          -RMA_ACTION_TRAP2CPU
 *          -RMA_ACTION_FORWARD_COPY2_CPU
 */
rtk_api_ret_t rtk_trap_rmaAction_get(rtk_mac_t *pRma_frame, rtk_trap_rma_action_t *pRma_action)
{
    rtk_api_ret_t retVal;

    if (pRma_frame == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    if (pRma_frame->octet[0]!=0x01&&pRma_frame->octet[1]!=0x80&&pRma_frame->octet[2]!=0xC2
        &&pRma_frame->octet[3]!=0&&pRma_frame->octet[4]!=0&&pRma_frame->octet[5]>0x2F)
        return RT_ERR_RMA_ADDR;

    if (pRma_action == NULL)
        return RT_ERR_NULL_POINTER;

    if (pRma_frame->octet[5] == 0)
    {
        retVal = reg_field_read(RTL8316D_UNIT, RMA_CONTROL0, ACT0, pRma_action);
    } else if (pRma_frame->octet[5] == 1) {
        retVal = reg_field_read(RTL8316D_UNIT, RMA_CONTROL0, ACT1, pRma_action);
    } else if (pRma_frame->octet[5] == 2) {
        retVal = reg_field_read(RTL8316D_UNIT, RMA_CONTROL0, ACT2, pRma_action);
    } else if (pRma_frame->octet[5] == 3) {
        retVal = reg_field_read(RTL8316D_UNIT, RMA_CONTROL0, ACT3, pRma_action);
    } else if ((pRma_frame->octet[5] >= 4 && pRma_frame->octet[5] <= 0xD) 
                                         || pRma_frame->octet[5] == 0xF) {
        retVal = reg_field_read(RTL8316D_UNIT, RMA_CONTROL0, ACT4, pRma_action);
    } else if (pRma_frame->octet[5] == 0xE) {
        retVal = reg_field_read(RTL8316D_UNIT, RMA_CONTROL1, ACT5, pRma_action);
    } else if (pRma_frame->octet[5] == 0x10) {
        retVal = reg_field_read(RTL8316D_UNIT, RMA_CONTROL1, ACT6, pRma_action);
    } else if (pRma_frame->octet[5] == 0x20) {
        retVal = reg_field_read(RTL8316D_UNIT, RMA_CONTROL1, ACT7, pRma_action);
    } else if (pRma_frame->octet[5] == 0x21) {
        retVal = reg_field_read(RTL8316D_UNIT, RMA_CONTROL1, ACT8, pRma_action);
    } else if (pRma_frame->octet[5] >= 0x22 && pRma_frame->octet[5] <= 0x2F) {
        retVal = reg_field_read(RTL8316D_UNIT, RMA_CONTROL1, ACT9, pRma_action);
    } else if (pRma_frame->octet[5] >= 0x31 && pRma_frame->octet[5] <= 0x3F) {
        retVal = reg_field_read(RTL8316D_UNIT, RMA_CONTROL2, ACT10, pRma_action);
    } else  {
        return RT_ERR_RMA_ADDR;
    } 

    if (retVal != SUCCESS)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_trap_rmaToCPUPriority_get
 * Description:
 *      Get priority assigned to packets when trapped to CPU.
 * Input:
 *      pRma_frame - Reserved multicast address.
 * Output:
 *      pPriority - priority assigned when packets with specific RMA trapped to CPU. 
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - Access Acl table error or parameter error
 *      RT_ERR_RMA_ADDR - Invalid RMA address. 
 *      RT_ERR_NULL_POINTER - Invalid RMA address. 
 * Note:
 *      Packets with DMAC which is RMA could be assigned priority when trapped to CPU. This API could be used to
 *     get the priority for each RMA classes.
 */
int32 rtk_trap_rmaToCPUPriority_get(rtk_mac_t *pRma_frame, rtk_pri_t *pPriority)
{
//    rtk_api_ret_t retVal;
    uint32 regVal;

    if (pRma_frame == NULL || pPriority == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    if (pRma_frame->octet[0]!=0x01&&pRma_frame->octet[1]!=0x80&&pRma_frame->octet[2]!=0xC2
        &&pRma_frame->octet[3]!=0&&pRma_frame->octet[4]!=0&&pRma_frame->octet[5]>0x3F)
        return RT_ERR_RMA_ADDR;

    if (pRma_frame->octet[5] == 0)
    {
        reg_read(RTL8316D_UNIT, RMA_CONTROL0, &regVal);
        *pPriority = regVal & 0x3;
    } else if (pRma_frame->octet[5] == 1) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL0, &regVal);
        *pPriority = (regVal >> 6) & 0x3;
    } else if (pRma_frame->octet[5] == 2) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL0, &regVal);
        *pPriority = (regVal >> 12) & 0x3;
    } else if (pRma_frame->octet[5] == 3) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL0, &regVal);
        *pPriority = (regVal >> 18) & 0x3;
    } else if ((pRma_frame->octet[5] >= 4 && pRma_frame->octet[5] <= 0xD) 
                                         || pRma_frame->octet[5] == 0xF) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL0, &regVal);
        *pPriority = (regVal >> 24) & 0x3;
    } else if (pRma_frame->octet[5] == 0xE) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL1, &regVal);
        *pPriority = regVal& 0x3;
    } else if (pRma_frame->octet[5] == 0x10) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL1, &regVal);
        *pPriority = (regVal >> 6) & 0x3;
    } else if (pRma_frame->octet[5] == 0x20) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL1, &regVal);
        *pPriority = (regVal >> 12) & 0x3;
    } else if (pRma_frame->octet[5] == 0x21) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL1, &regVal);
        *pPriority = (regVal >> 18) & 0x3;
    } else if (pRma_frame->octet[5] >= 0x22 && pRma_frame->octet[5] <= 0x2F) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL1, &regVal);
        *pPriority = (regVal >> 24) & 0x3;
    } else if (pRma_frame->octet[5] >= 0x31 && pRma_frame->octet[5] <= 0x3F) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL2, &regVal);
        *pPriority = regVal & 0x3;
    } else  {
        return RT_ERR_RMA_ADDR;
    } 

    return RT_ERR_OK;

}

/* Function Name:
 *      rtk_trap_rmaToCPUPriority_set
 * Description:
 *      Set priority assigned to packets when trapped to CPU.
 * Input:
 *      pRma_frame - Reserved multicast address.
 *      priority - priority assigned when packets with specific RMA trapped to CPU. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_RMA_ADDR - Invalid RMA address. 
 *      RT_ERR_NULL_POINTER - Invalid RMA address. 
 * Note:
 *      Packets with DMAC which is RMA could be assigned priority when trapped to CPU. This API could be used to
 *     get the priority for each RMA classes.
 */
int32 rtk_trap_rmaToCPUPriority_set(rtk_mac_t *pRma_frame, rtk_pri_t priority)
{
//    rtk_api_ret_t retVal;
    uint32 regVal;

    if (pRma_frame == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    if (pRma_frame->octet[0]!=0x01&&pRma_frame->octet[1]!=0x80&&pRma_frame->octet[2]!=0xC2
        &&pRma_frame->octet[3]!=0&&pRma_frame->octet[4]!=0&&pRma_frame->octet[5]>0x3F)
        return RT_ERR_RMA_ADDR;

    if (priority > RTL8316D_PRI_MAX)
        return RT_ERR_INPUT;

    if (pRma_frame->octet[5] == 0)
    {
        reg_read(RTL8316D_UNIT, RMA_CONTROL0, &regVal);
        regVal |= 1 << 3;
        regVal &= ~0x3;
        regVal |= priority;
        reg_write(RTL8316D_UNIT, RMA_CONTROL0, regVal);
    } else if (pRma_frame->octet[5] == 1) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL0, &regVal);
        regVal |= 1 << 9;
        regVal &= ~(0x3 << 6);
        regVal |= priority << 6;
        reg_write(RTL8316D_UNIT, RMA_CONTROL0, regVal);
    } else if (pRma_frame->octet[5] == 2) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL0, &regVal);
        regVal |= 1 << 15;
        regVal &= ~(0x3 << 12);
        regVal |= priority << 12;
        reg_write(RTL8316D_UNIT, RMA_CONTROL0, regVal);
    } else if (pRma_frame->octet[5] == 3) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL0, &regVal);
        regVal |= 1 << 21;
        regVal &= ~(0x3 << 18);
        regVal |= priority << 18;
        reg_write(RTL8316D_UNIT, RMA_CONTROL0, regVal);
    } else if ((pRma_frame->octet[5] >= 4 && pRma_frame->octet[5] <= 0xD) 
                                         || pRma_frame->octet[5] == 0xF) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL0, &regVal);
        regVal |= 1 << 27;
        regVal &= ~(0x3 << 24);
        regVal |= priority << 24;
        reg_write(RTL8316D_UNIT, RMA_CONTROL0, regVal);
    } else if (pRma_frame->octet[5] == 0xE) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL1, &regVal);
        regVal |= 1 << 3;
        regVal &= ~0x3;
        regVal |= priority;
        reg_write(RTL8316D_UNIT, RMA_CONTROL1, regVal);
    } else if (pRma_frame->octet[5] == 0x10) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL1, &regVal);
        regVal |= 1 << 9;
        regVal &= ~(0x3 << 6);
        regVal |= priority << 6;
        reg_write(RTL8316D_UNIT, RMA_CONTROL1, regVal);
    } else if (pRma_frame->octet[5] == 0x20) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL1, &regVal);
        regVal |= 1 << 15;
        regVal &= ~(0x3 << 12);
        regVal |= priority << 12;
        reg_write(RTL8316D_UNIT, RMA_CONTROL1, regVal);
    } else if (pRma_frame->octet[5] == 0x21) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL1, &regVal);
        regVal |= 1 << 21;
        regVal &= ~(0x3 << 18);
        regVal |= priority << 18;
        reg_write(RTL8316D_UNIT, RMA_CONTROL1, regVal);
    } else if (pRma_frame->octet[5] >= 0x22 && pRma_frame->octet[5] <= 0x2F) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL1, &regVal);
        regVal |= 1 << 27;
        regVal &= ~(0x3 << 24);
        regVal |= priority << 24;
        reg_write(RTL8316D_UNIT, RMA_CONTROL1, regVal);
    } else if (pRma_frame->octet[5] >= 0x31 && pRma_frame->octet[5] <= 0x3F) {
        reg_read(RTL8316D_UNIT, RMA_CONTROL2, &regVal);
        regVal |= 1 << 3;
        regVal &= ~0x3;
        regVal |= priority;
        reg_write(RTL8316D_UNIT, RMA_CONTROL2, regVal);
    } else  {
        return RT_ERR_RMA_ADDR;
    } 

    return RT_ERR_OK;
}


