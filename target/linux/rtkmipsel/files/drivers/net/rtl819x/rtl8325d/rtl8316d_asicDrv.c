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
* Purpose :This header defines VERY general macro, structure, enumeration and variable types that
*              could be used both by applications, APIs and ASIC driver.
*              Be ware that this file could be released to customer whenever you add new contents.
*
*  Feature :
*
*/

#include <asicdrv/rtl8316d_types.h>
#include <rtk_api.h>
#include <rtl8316d_general_reg.h>
#include <rtl8316d_table_struct.h>
#include <rtl8316d_asicDrv.h>
#include <rtl8316d_asicdrv_nic.h>
//#include <rtl8316d_asicdrv_tbl.h>
//#include <stdio.h>
#include <rtl8316d_reg_struct.h>
#include <rtl8316d_asicdrv_trunk.h>
#include <rtl8316d_debug.h>

void rtk_switch_init(void)
{
    uint16 t;

    reg_write(RTL8316D_UNIT, PHY_AUTO_ACCESS_MASK1, 0x1ffffff);

    for(t=0;t<16;t++)
        phy_reg_write(t,18,1,0x9270);
    for(t=16;t<24;t++)
        phy_reg_write(t,18,1,0x9230);

    reg_write(RTL8316D_UNIT, GLOBAL_MAC_INTERFACE_INTERNAL3, 0x10);
    reg_write(RTL8316D_UNIT, INTERFACE_DMY_REGSITER, 0x100);

    reg_write(RTL8316D_UNIT, PORT0_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT1_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT2_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT3_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT4_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT5_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT6_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT7_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT8_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT9_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT10_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT11_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT12_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT13_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT14_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT15_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT16_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT17_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT18_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT19_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT20_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT21_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT22_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT23_L2_MISC0, 0x4f7);
    reg_write(RTL8316D_UNIT, PORT24_L2_MISC0, 0x4f7);
}

void rtl8316d_sys_reboot(void)
{
    reg_write(RTL8316D_UNIT, RESET_GLOBAL_CONTROL1, 0x0);
}

/* Function Name:
 *      rtl8316d_swInfo_MAC_get
 * Description:
 *      Get switch mac address
 * Input:
 *      none
 * Output:
 *      mac - pointer to switch mac address
 * Return:
 *
 */

void rtl8316d_swInfo_MAC_get(uint8* mac)
{
    uint32 mac_high;
    uint32 mac_low;

    reg_read(RTL8316D_UNIT, SWITCH_OWN_MAC_CONTROL0, &mac_high);
    mac[0] = (mac_high >> 24) & 0xff;
    mac[1] = (mac_high >> 16) & 0xff;
    mac[2] = (mac_high >> 8) & 0xff;
    mac[3] = (mac_high >> 0) & 0xff;

    reg_read(RTL8316D_UNIT, SWITCH_OWN_MAC_CONTROL1, &mac_low);
    mac[4] = (mac_low >> 8) & 0xff;
    mac[5] = (mac_low >> 0) & 0xff;

    rtlglue_printf("The Mac Address is: %lx:%lx\n", mac_high,mac_low);
}

uint8 get_port_cnt(uint32 mask)
{
    uint8 port_cnt;

    port_cnt = 0;
    while (mask != 0)
    {
        if ((mask & 0x01) == 0x01)
            port_cnt++;

        mask = mask >> 1;
    }

    return port_cnt;
}

/* Function Name:
 *      rtk_trunk_port_set
 * Description:
 *      Set trunking group available port mask
 * Input:
 *      trk_gid - trunk group id
 *      pTrunk_member_portmask  - Logic trunking member port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_LA_TRUNK_ID - error trunk ID
 *      RT_ERR_PORT_MASK - error port mask
 *      RT_ERR_LA_PORTNUM_NORMAL - every port could support no more than 8 ports.
 * Note:
 *      The API can set 4 port trunking group enabled port mask. Each port trunking group has max 8 ports.
 *      If enabled port mask has less than 2 ports available setting, then this trunking group function is disabled.
 */
rtk_api_ret_t rtk_trunk_port_set(rtk_trunk_group_t trk_gid, rtk_portmask_t trunk_member_portmask)
{
    uint32 mask;
    rtk_api_ret_t retVal;

    if (trk_gid>=TRUNK_GROUP_END)
        return RT_ERR_LA_TRUNK_ID;

    if ((trunk_member_portmask.bits[0] & RTK_TRUNK_MAX_PORT_MASK) != trunk_member_portmask.bits[0])
        return RT_ERR_PORT_MASK;

    mask = trunk_member_portmask.bits[0];

    if (get_port_cnt(mask) > RTK_TRUNK_PORT_NUM_PER_GRP)
        return RT_ERR_LA_PORTNUM_NORMAL;

    retVal = reg_write(RTL8316D_UNIT, LINK_AGGREGATION_MEMBER_CONTROL0+trk_gid, mask);
    return retVal;
}


/* Function Name:
 *      rtk_trunk_port_get
 * Description:
 *      Get trunking group available port mask
 * Input:
 *      trk_gid - trunk group id
 * Output:
 *      pTrunk_member_portmask  - Logic trunking member port mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER - NULL pointer input
 *      RT_ERR_LA_TRUNK_ID - error trunk ID
 * Note:
 *      RTL8316D has four groups of trunk.
 */
rtk_api_ret_t rtk_trunk_port_get(rtk_trunk_group_t trk_gid, rtk_portmask_t *pTrunk_member_portmask)
{
    uint32 regVal;
    rtk_api_ret_t retVal;

    if (trk_gid >= TRUNK_GROUP_END)
        return RT_ERR_LA_TRUNK_ID;

    if (pTrunk_member_portmask == NULL)
        return RT_ERR_NULL_POINTER;

    memset(pTrunk_member_portmask, 0, sizeof(rtk_portmask_t));

    retVal = reg_read(RTL8316D_UNIT, LINK_AGGREGATION_MEMBER_CONTROL0+trk_gid, &regVal);
    pTrunk_member_portmask->bits[0] = regVal;

    return retVal;
}

/* Function Name:
 *      rtk_trunk_qeueuEmptyStatus_get
 * Description:
 *      get current output queue if empty status
 * Input:
 *     None
 * Output:
 *      pPortmask  - queue empty port mask, 1 for empty and 0 for not empty
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER - NULL pointer input
 * Note:
 *      The API can get port mask whose queues are empty.
 */
rtk_api_ret_t rtk_trunk_qeueuEmptyStatus_get(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;

    if (pPortmask == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    if((retVal=reg_field_read(RTL8316D_UNIT, LINK_AGGREGATION_EMPTY_PORT_CONTROL, EMPTYDPM, &pPortmask->bits[0]))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


rtk_api_ret_t rtl8316d_setAsicPortIngressBandwidthEnable(uint32 port,uint32 enable)
{
    uint32 regFieldValue;
    uint32 retVal;

    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(enable>=RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    regFieldValue = (enable>0)?1:0;

    if((retVal=reg_field_write(RTL8316D_UNIT, PORT0_INGRESS_BANDWIDTH__RATE_CONTROL0+port, INBW0_EN, regFieldValue))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_getAsicPortIngressBandwidthEnable(uint32 port,uint32 *enable)
{
    uint32 retVal;

    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(NULL != enable)
    {
        if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_INGRESS_BANDWIDTH__RATE_CONTROL0+port, INBW0_EN, enable))!=RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/*8316D has two leaky bucket for every ingress port, here set the rate to both two leaky bucket.*/
rtk_api_ret_t rtl8316d_setAsicPortIngressBandwidth(uint32 port,uint32 rate, uint32 ifg_include)
{
    uint32 regFieldValue;
    uint32 retVal;

    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if((rate > 0xFFFF)||(rate < 1))
        return RT_ERR_INBW_RATE;

    if(ifg_include >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    regFieldValue = (ifg_include>0)?1:0;

    if((retVal=reg_field_write(RTL8316D_UNIT,INGRESS_BANDWIDTH_CONTROL_DROP_THRESHOLD_GLOBAL_CONTROL,INBW_INC_IFG,regFieldValue))!=RT_ERR_OK)
        return retVal;

    if((retVal=reg_field_write(RTL8316D_UNIT, PORT0_INGRESS_BANDWIDTH__RATE_CONTROL0+port, INBW0_RATE, rate))!=RT_ERR_OK)
        return retVal;

    if((retVal=reg_field_write(RTL8316D_UNIT, PORT0_INGRESS_BANDWIDTH__RATE_CONTROL1+port, INBW0_RATE, rate))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_getAsicPortIngressBandwidth(uint32 port,uint32 *rate, uint32 *ifg_include)
{
    uint32 retVal;

    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(NULL != rate)
    {
        if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_INGRESS_BANDWIDTH__RATE_CONTROL0+port, INBW0_RATE, rate))!=RT_ERR_OK)
            return retVal;
    }

    if(NULL != ifg_include)
    {
        if((retVal=reg_field_read(RTL8316D_UNIT,INGRESS_BANDWIDTH_CONTROL_DROP_THRESHOLD_GLOBAL_CONTROL,INBW_INC_IFG,ifg_include))!=RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_setAsicPortEgressBandwidthEnable(uint32 port,uint32 enable)
{
    uint32 regFieldValue;
    uint32 retVal;

    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(enable>=RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    regFieldValue = (enable>0)?1:0;

    if((retVal=reg_field_write(RTL8316D_UNIT,PORT0_QUEUE_SCHEDULE_CONTROL+port, MINLB_EN,regFieldValue))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_getAsicPortEgressBandwidthEnable(uint32 port,uint32 *enable)
{
    uint32 retVal;

    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(NULL != enable)
    {
        if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_QUEUE_SCHEDULE_CONTROL+port, MINLB_EN, enable))!=RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_setAsicPortEgressBandwidth(uint32 port,uint32 rate, uint32 ifg_include)
{
    uint32 regFieldValue;
    uint32 retVal;

    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if((rate > 0xFFFF)||(rate<1))
        return RT_ERR_INBW_RATE;

    if(ifg_include>=RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    regFieldValue = (ifg_include>0)?1:0;

    if((retVal=reg_field_write(RTL8316D_UNIT,PORT_LEAKY_BUKET_CONTROL, PKTLENWITHPIFG,regFieldValue))!=RT_ERR_OK)
        return retVal;

    if((retVal=reg_field_write(RTL8316D_UNIT, PORT0_BANDWITH_CONTROL+port, BWRATE, rate))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_getAsicPortEgressBandwidth(uint32 port,uint32 *rate, uint32 *ifg_include)
{
    uint32 retVal;



    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(NULL != rate)
    {
        if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_BANDWITH_CONTROL+port, BWRATE, rate))!=RT_ERR_OK)
            return retVal;
    }

    if(NULL != ifg_include)
    {
        if((retVal=reg_field_read(RTL8316D_UNIT,PORT_LEAKY_BUKET_CONTROL, PKTLENWITHPIFG, ifg_include))!=RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}


/*
@func ret_t | rtl8316d_vlan_portPvid_set | Set port based VID which is indexed to 32 VLAN member configurations.
@parm uint32 | port | Physical port number (0~15).
@parm uint32 | index | Index to VLAN member configuration (0~31).
@parm uint32 | pri | 1Q Port based VLAN priority (0~7).
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_VLAN_PRIORITY | Invalid priority
@rvalue RT_ERR_VLAN_ENTRY_NOT_FOUND | Invalid VLAN member configuration index (0~31).
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    This API can Set Port-Based VLAN setting
    In port based VLAN, untagged packets recieved by port N are forwarded to a VLAN according to the setting VID of port N.
    Usage of VLAN 4k table is enabled and there are only VID and 802.1q priority retrieved from 32 member configurations .
    Member set, untag set and FID of port based VLAN are be retrieved from 4K mapped VLAN entry.

*/


/* Function Name:
 *      rtl8316d_vlan_portPvid_set
 * Description:
 *      Set VLAN port-based VID and priority
 * Input:
 *      port                          - Port id
 *      Vid                           -  Port-based vid
 *      Pri                           -   Port-base vlan priority
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
rtk_api_ret_t rtl8316d_vlan_portPvid_set(uint32 port, uint32 vid, uint32 pri)
{
    uint32 regVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (vid > RTL8316D_VIDMAX)
        return RT_ERR_VLAN_VID;

    if(pri > RTL8316D_PRIMAX)
        return RT_ERR_VLAN_PRIORITY;

    CHK_FUN_RETVAL(reg_read(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, &regVal));
    reg_field_set(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, PIPRI, pri, &regVal);
    reg_field_set(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, PIVID, vid, &regVal);
    CHK_FUN_RETVAL(reg_write(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, regVal));

    return RT_ERR_OK;
}


/* Function Name:
 *      rtl8316d_vlan_portPvid_get
 * Description:
 *      Get VLAN port-based VID and priority
 * Input:
 *      port                           - Port id
 * Output:
 *      pVid                          - the pointer of port-based vid
 *      pPri                           - the pointer of port-base vlan priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_SMI
 *
 * Note:
 *
 */
rtk_api_ret_t rtl8316d_vlan_portPvid_get(uint32 port, uint32 *pVid, uint32 *pPri)
{
    uint32 regVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(pVid == NULL)
        return RT_ERR_INPUT;

    if(pPri == NULL)
        return RT_ERR_INPUT;

    CHK_FUN_RETVAL(reg_read(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, &regVal));
    reg_field_get(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, PIVID, pVid, regVal);
    reg_field_get(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, PIPRI, pPri, regVal);

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_get
 * Description:
 *      Set VLAN support frame type
 * Input:
 *      port                           - Port id
 *      accept_frame_type      - accept frame type
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_INPUT
 *      RT_ERR_SMI
 *
 * Note:
 *     The API can Get the VLAN ingress filter.
 *     The accept frame type as following:
 *         ACCEPT_FRAME_TYPE_ALL
 *         ACCEPT_FRAME_TYPE_TAG_ONLY
 *         ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
rtk_api_ret_t rtl8316d_vlan_portAcceptFrameType_set(uint32 port, rtk_vlan_acceptFrameType_t accept_frame_type)
{
    uint32 regVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    CHK_FUN_RETVAL(reg_read(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, &regVal));
    switch(accept_frame_type)
    {
        case ACCEPT_FRAME_TYPE_ALL:
            reg_field_set(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTITAG, 1, &regVal);
            reg_field_set(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTIUTAG, 1, &regVal);
            break;
        case ACCEPT_FRAME_TYPE_TAG_ONLY:
            reg_field_set(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTITAG, 1, &regVal);
            reg_field_set(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTIUTAG, 0, &regVal);
            break;
        case ACCEPT_FRAME_TYPE_UNTAG_ONLY:
            reg_field_set(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTITAG, 0, &regVal);
            reg_field_set(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTIUTAG, 1, &regVal);
            break;
        default:
            return RT_ERR_INPUT;
    }
    CHK_FUN_RETVAL(reg_write(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, regVal));

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN Ingress Filter
 * Input:
 *      port                           - Port id
 * Output:
 *      pAccept_frame_type     - accept frame type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_SMI
 *
 * Note:
 *     The API can Get the VLAN ingress filter.
 *     The accept frame type as following:
 *         ACCEPT_FRAME_TYPE_ALL
 *         ACCEPT_FRAME_TYPE_TAG_ONLY
 *         ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
rtk_api_ret_t rtl8316d_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type)
{
    uint32 regVal, acctag, accutag;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    CHK_FUN_RETVAL(reg_read(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, &regVal));
    reg_field_get(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTITAG, &acctag, regVal);
    reg_field_get(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTIUTAG, &accutag, regVal);
    *pAccept_frame_type = acctag ? 0:0x2;
    *pAccept_frame_type +=  accutag ? 0: 0x1;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_vlan_tagMode_set
 * Description:
 *      Set CVLAN egress tag mode
 * Input:
 *      tag_rxc     - the pkts whose souce port is customer port tx tag status
 *      tag_rxs     - the pkts whose souce port is service port tx tag status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_INPUT
 *      RT_ERR_SMI
 *
 * Note:
 *    The API is used for setting CVLAN egress tag mode
 *    The accept frame type as following:
 *    RTL8316D_VLAN_TAG_ORIGINAL
 *    RTL8316D_VLAN_TAG_KEEP
 *    RTL8316D_VLAN_TAG_REAL_KEEP
 */
rtk_api_ret_t rtl8316d_vlan_tagMode_set(rtk_port_t port, rtl8316d_vlan_tagMode_s tag_rxc, rtl8316d_vlan_tagMode_s tag_rxs )
{

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if ((tag_rxc >= RTL8316D_VLAN_TAG_END) || (tag_rxs >= RTL8316D_VLAN_TAG_END))
        return RT_ERR_INPUT;

    if (port < PN_PORT23)
    {

        CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PORT0_TX_TAG_CONTROL + port, RXCTX_ITAG_KEEP, tag_rxc));
        CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PORT0_TX_TAG_CONTROL + port, RXSTX_ITAG_KEEP, tag_rxs));
    }
    else
    {
        CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PORT23_TX_TAG_CONTROL + (port - PN_PORT23), RXCTX_ITAG_KEEP, tag_rxc));
        CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PORT23_TX_TAG_CONTROL + (port - PN_PORT23), RXSTX_ITAG_KEEP, tag_rxs));
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      rtl8316d_vlan_tagMode_get
 * Description:
 *      Get CVLAN egress tag mode
 * Input:
 *      pTag_rxc     - the pkts whose souce port is customer port tx tag status
 *      pTag_rxs     - the pkts whose souce port is service port tx tag status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_SMI
 *
 * Note:
 *    The API is used for setting CVLAN egress tag mode
 *    The accept frame type as following:
 *    RTL8316D_VLAN_TAG_ORIGINAL
 *    RTL8316D_VLAN_TAG_KEEP
 *    RTL8316D_VLAN_TAG_REAL_KEEP
 */
rtk_api_ret_t rtl8316d_vlan_tagMode_get(rtk_port_t port, rtl8316d_vlan_tagMode_s *pTag_rxc, rtl8316d_vlan_tagMode_s *pTag_rxs)
{
    uint32 field_val;

    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if ((NULL == pTag_rxc ) || ( NULL == pTag_rxs))
        return RT_ERR_NULL_POINTER;

    if (port < PN_PORT23)
    {
        CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PORT0_TX_TAG_CONTROL + port, RXCTX_ITAG_KEEP, &field_val));
        *pTag_rxc = (rtl8316d_vlan_tagMode_s)field_val;
        CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PORT0_TX_TAG_CONTROL + port, RXSTX_ITAG_KEEP, &field_val));
        *pTag_rxs = (rtl8316d_vlan_tagMode_s)field_val;
    }
    else
    {
        CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PORT23_TX_TAG_CONTROL + (port - PN_PORT23), RXCTX_ITAG_KEEP, &field_val));
        *pTag_rxc = (rtl8316d_vlan_tagMode_s)field_val;
        CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PORT23_TX_TAG_CONTROL + (port - PN_PORT23), RXSTX_ITAG_KEEP, &field_val));
        *pTag_rxs = (rtl8316d_vlan_tagMode_s)field_val;
    }

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_vlan_tagAware_set(rtk_port_t port, uint32 enabled)
{
    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PORT0_IGNORE_VLAN_TAG_CONTROL, IGNOREITAGVID, enabled ? 0 : 1));

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_vlan_tagAware_get(rtk_port_t port, uint32 *pEnabled)
{
    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (NULL == pEnabled)
        return RT_ERR_NULL_POINTER;

    CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PORT0_IGNORE_VLAN_TAG_CONTROL, IGNOREITAGVID, pEnabled));
    *pEnabled = (*pEnabled) ? 0 : 1;

    return RT_ERR_OK;

}

rtk_api_ret_t rtl8316d_vlan_ingFilterEnable_set(rtk_port_t port, uint32 enabled)
{
    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PORT0_VLAN_INGRESS_FILTER_CONTROL + port, IGFILTER,  enabled));

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_vlan_ingFilterEnable_get(rtk_port_t port, uint32 *pEnabled)
{
    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (NULL == pEnabled)
        return RT_ERR_NULL_POINTER;

    CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PORT0_VLAN_INGRESS_FILTER_CONTROL + port, IGFILTER, pEnabled));

    return RT_ERR_OK;
}


rtk_api_ret_t rtl8316d_vlan_portRole_set(rtk_port_t port, rtl8316d_vlan_portRole_s portrole)
{
    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (port >= RTL8316D_PORTROLE_END)
        return RT_ERR_INPUT;

    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port,  PORTROLE, (uint32)portrole));

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_vlan_portRole_get(rtk_port_t port, rtl8316d_vlan_portRole_s *pPortrole)
{
    uint32 fieldval;

    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (NULL == pPortrole)
        return RT_ERR_NULL_POINTER;

    CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port,  PORTROLE, &fieldval));
    *pPortrole = fieldval ? RTL8316D_PORTROLE_S : RTL8316D_PORTROLE_C;

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_vlan_pidEntry_set(uint32 entry, uint32 pid)
{

    switch(entry)
    {
        case 0:
            CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PACKET_PARSER_PID_CONTROL_REGIST_0, PID0, pid & 0xffff));
            break;
        case 1:
            CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PACKET_PARSER_PID_CONTROL_REGIST_0, PID1, pid & 0xffff));
            break;
        case 2:
            CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PACKET_PARSER_PID_CONTROL_REGIST_1, PID2, pid & 0xffff));
            break;
        case 3:
            CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PACKET_PARSER_PID_CONTROL_REGIST_1, PID3, pid & 0xffff));
            break;
        default:
            return RT_ERR_INPUT;
    }

    return RT_ERR_OK;
}


rtk_api_ret_t rtl8316d_vlan_pidEntry_get(uint32 entry, uint32* pPid)
{

    switch(entry)
    {
        case 0:
            CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PACKET_PARSER_PID_CONTROL_REGIST_0, PID0, pPid));
            break;
        case 1:
            CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PACKET_PARSER_PID_CONTROL_REGIST_0, PID1, pPid));
            break;
        case 2:
            CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PACKET_PARSER_PID_CONTROL_REGIST_1, PID2, pPid));
            break;
        case 3:
            CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PACKET_PARSER_PID_CONTROL_REGIST_1, PID3, pPid));
            break;
        default:
            return RT_ERR_INPUT;
    }
    *pPid &= 0xffff;

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_vlan_portTpid_set(rtk_port_t port, uint32 tpidmask)
{
    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (port < PN_PORT23)
    {
        CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, port + PORT0_PID_MASK_CONTROL, TPIDMASK, tpidmask & 0x3));
    }
    else
    {
        CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, (port - PN_PORT23) + PORT23_PID_MASK_CONTROL , TPIDMASK, tpidmask & 0xF));
    }

    return RT_ERR_OK;
}


rtk_api_ret_t rtl8316d_vlan_portTpid_get(rtk_port_t port, uint32* pTpidmask)
{
    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (NULL == pTpidmask)
        return RT_ERR_NULL_POINTER;


    if (port < PN_PORT23)
    {
        CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, port + PORT0_PID_MASK_CONTROL, TPIDMASK, pTpidmask));
    }
    else
    {
        CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, (port - PN_PORT23) + PORT23_PID_MASK_CONTROL , TPIDMASK, pTpidmask));
    }

    return RT_ERR_OK;
}


rtk_api_ret_t rtl8316d_svlan_fwdBaseOvid_set(rtk_port_t port, uint32 enabled)
{
    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, port + PORT0_FORWARDING_TAG_SELECT_CONTROL, FWDBASE_OVID, enabled));

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_svlan_fwdBaseOvid_get(rtk_port_t port, uint32 *pEnabled)
{
    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (NULL ==  pEnabled)
        return RT_ERR_NULL_POINTER;

    CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, port + PORT0_FORWARDING_TAG_SELECT_CONTROL, FWDBASE_OVID, pEnabled));

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_svlan_portSpid_set(rtk_port_t port, uint32 spidmask)
{
    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (port < PN_PORT23)
    {
        CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, port + PORT0_PID_MASK_CONTROL, SPIDMASK, spidmask & 0x3));
    }
    else
    {
        CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, (port - PN_PORT23) + PORT23_PID_MASK_CONTROL , SPIDMASK, spidmask & 0xF));
    }

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_svlan_portSpid_get(rtk_port_t port, uint32* pSpidmask)
{
    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (NULL == pSpidmask)
        return RT_ERR_NULL_POINTER;

    if (port < PN_PORT23)
    {
        CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, port + PORT0_PID_MASK_CONTROL, SPIDMASK, pSpidmask));
    }
    else
    {
        CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, (port - PN_PORT23) + PORT23_PID_MASK_CONTROL , SPIDMASK, pSpidmask));
    }

    return RT_ERR_OK;
}



rtk_api_ret_t rtl8316d_svlan_portPvid_set(uint32 port, uint32 svid, uint32 pri, uint32 dei)
{
    uint32 regVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (svid > RTL8316D_VIDMAX)
        return RT_ERR_SVLAN_VID;

    if(pri > RTL8316D_PRIMAX)
        return RT_ERR_VLAN_PRIORITY;

    if (dei > 1)
        return RT_ERR_INPUT;

    CHK_FUN_RETVAL(reg_read(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, &regVal));
    reg_field_set(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, POVID, svid, &regVal);
    reg_field_set(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, POPRI, pri, &regVal);
    reg_field_set(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, PODEI, dei, &regVal);
    CHK_FUN_RETVAL(reg_write(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, regVal));

    return RT_ERR_OK;
}


rtk_api_ret_t rtl8316d_svlan_portPvid_get(uint32 port, uint32 *pSvid, uint32 *pPri, uint32 *pDei)
{
    uint32 regVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if ((NULL == pSvid) || (NULL == pPri) || (NULL == pDei))
        return RT_ERR_NULL_POINTER;

    CHK_FUN_RETVAL(reg_read(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, &regVal));
    reg_field_get(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, POVID, pSvid, regVal);
    reg_field_get(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, POPRI, pPri, regVal);
    reg_field_get(RTL8316D_UNIT, PORT0_VLAN_CONTROL + port, PODEI, pDei, regVal);

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_svlan_portAcceptFrameType_set(uint32 port, rtk_vlan_acceptFrameType_t accept_frame_type)
{
    uint32 regVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    CHK_FUN_RETVAL(reg_read(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, &regVal));
    switch(accept_frame_type)
    {
        case ACCEPT_FRAME_TYPE_ALL:
            reg_field_set(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTOTAG, 1, &regVal);
            reg_field_set(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTOUTAG, 1, &regVal);
            break;
        case ACCEPT_FRAME_TYPE_TAG_ONLY:
            reg_field_set(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTOTAG, 1, &regVal);
            reg_field_set(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTOUTAG, 0, &regVal);
            break;
        case ACCEPT_FRAME_TYPE_UNTAG_ONLY:
            reg_field_set(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTOTAG, 0, &regVal);
            reg_field_set(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTOUTAG, 1, &regVal);
            break;
        default:
            return RT_ERR_INPUT;
    }
    CHK_FUN_RETVAL(reg_write(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, regVal));

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_svlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type)
{
    uint32 regVal, acctag, accutag;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    CHK_FUN_RETVAL(reg_read(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, &regVal));
    reg_field_get(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTOTAG, &acctag, regVal);
    reg_field_get(RTL8316D_UNIT, PORT0_ACCEPT_FRAME_TYPE_CONTROL + port, ACPTOUTAG, &accutag, regVal);
    *pAccept_frame_type = acctag ? 0:0x2;
    *pAccept_frame_type +=  accutag ? 0: 0x1;

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_svlan_tagMode_set(rtk_port_t port, rtl8316d_vlan_tagMode_s tag_rxc, rtl8316d_vlan_tagMode_s tag_rxs )
{

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if ((tag_rxc > RTL8316D_VLAN_TAG_END) || (tag_rxs > RTL8316D_VLAN_TAG_END))
        return RT_ERR_INPUT;

    if (port < PN_PORT23)
    {
        CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PORT0_TX_TAG_CONTROL + port, RXCTX_OTAG_KEEP, tag_rxc));
        CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PORT0_TX_TAG_CONTROL + port, RXSTX_OTAG_KEEP, tag_rxs));
    }
    else
    {
        CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PORT23_TX_TAG_CONTROL + (port - PN_PORT23), RXCTX_OTAG_KEEP, tag_rxc));
        CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PORT23_TX_TAG_CONTROL + (port - PN_PORT23), RXSTX_OTAG_KEEP, tag_rxs));
    }

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_svlan_tagMode_get(rtk_port_t port, rtl8316d_vlan_tagMode_s *pTag_rxc, rtl8316d_vlan_tagMode_s *pTag_rxs)
{
    uint32 field_val;

    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if ((NULL == pTag_rxc ) || ( NULL == pTag_rxs))
        return RT_ERR_NULL_POINTER;

    if (port < PN_PORT23)
    {
        CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PORT0_TX_TAG_CONTROL + port, RXCTX_OTAG_KEEP, &field_val));
        *pTag_rxc = (rtl8316d_vlan_tagMode_s)field_val;
        CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PORT0_TX_TAG_CONTROL + port, RXSTX_OTAG_KEEP, &field_val));
        *pTag_rxs = (rtl8316d_vlan_tagMode_s)field_val;
    }
    else
    {
        CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PORT23_TX_TAG_CONTROL + (port - PN_PORT23), RXCTX_OTAG_KEEP, &field_val));
        *pTag_rxc = (rtl8316d_vlan_tagMode_s)field_val;
        CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PORT23_TX_TAG_CONTROL + (port - PN_PORT23), RXSTX_OTAG_KEEP, &field_val));
        *pTag_rxs = (rtl8316d_vlan_tagMode_s)field_val;
    }

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_svlan_tagAware_set(rtk_port_t port, uint32 enabled)
{
    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, PORT0_IGNORE_VLAN_TAG_CONTROL, IGNOREOTAGVID, enabled ? 0 : 1));

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_svlan_tagAware_get(rtk_port_t port, uint32 *pEnabled)
{
    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (NULL == pEnabled)
        return RT_ERR_NULL_POINTER;

    CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PORT0_IGNORE_VLAN_TAG_CONTROL, IGNOREOTAGVID, pEnabled));
    *pEnabled = (*pEnabled) ? 0 : 1;

    return RT_ERR_OK;
}


/*
@func ret_t | rtl8316d_jumbo_size_set | Max receiving packet length.
@parm uint32 | maxLength
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_JUMBO_FRAME_SIZE | Invalid input length parameter.
@comm
    If the API can be used to set accepted max packet length.
*/
rtk_api_ret_t rtl8316d_jumbo_size_set(uint32 maxLength)
{
    rtk_api_ret_t retVal;

    if (maxLength > RTL8316D_JUMBO_MAX_SIZE)
        return RT_ERR_JUMBO_FRAME_SIZE;

    if((retVal = reg_field_write(RTL8316D_UNIT, PORT_MAXLEN_CONTROL, MAXLEN, maxLength)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
/*
@func ret_t | rtl8316d_jumbo_size_get | Max receiving packet length.
@parm uint32* | maxLength
@rvalue RT_ERR_OK | Success.
@comm
    If the API can be used to get accepted max packet length.
*/
rtk_api_ret_t rtl8316d_jumbo_size_get(uint32* maxLength)
{
    rtk_api_ret_t retVal;
    if((retVal = reg_field_read(RTL8316D_UNIT, PORT_MAXLEN_CONTROL, MAXLEN, maxLength)) != RT_ERR_OK)
        return retVal;

    rtlglue_printf("The maxLength is :%lx\n", maxLength);
    return RT_ERR_OK;
}


rtk_api_ret_t rtl8316d_iso_mask_get(uint32 *mask)
{
    uint32 retVal;

    if (mask == NULL)
        return RT_ERR_NULL_POINTER;

    if((retVal=reg_field_read(RTL8316D_UNIT, PORT_ISOLATION_CONTROL, TISO, mask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_iso_mask_set(uint32 mask)
{
    uint32 retVal;

    if (mask >= RTL8316D_MAX_PORTMASK)
        return RT_ERR_PORT_MASK;

    if((retVal=reg_field_write(RTL8316D_UNIT, PORT_ISOLATION_CONTROL, TISO, mask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_mirror_portBased_set(rtk_port_t mirroring_port, rtk_portmask_t *pMirrored_rx_mask, rtk_portmask_t *pMirrored_tx_mask)
{
//    uint32 retVal;
    uint32 reg_val;

    if ( (NULL == pMirrored_rx_mask) || (NULL == pMirrored_tx_mask))
        return RT_ERR_NULL_POINTER;

    if (mirroring_port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (pMirrored_rx_mask->bits[0] >= RTL8316D_MAX_PORTMASK || pMirrored_tx_mask->bits[0] >= RTL8316D_MAX_PORTMASK)
        return RT_ERR_PORT_MASK;

    /*mirror port != source port*/
    if ((pMirrored_rx_mask->bits[0] & ((uint32)1 << mirroring_port)) ||
        (pMirrored_tx_mask->bits[0] & ((uint32)1 << mirroring_port)) )
        return RT_ERR_PORT_MASK;

    reg_val = mirroring_port;

    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, TRAFFIC_MIRROR_CONTROL1, DP, reg_val));


    reg_val = pMirrored_rx_mask->bits[0];
    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, TRAFFIC_MIRROR_CONTROL0, SPM, reg_val));


    reg_val = pMirrored_tx_mask->bits[0];
    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, TRAFFIC_MIRROR_CONTROL1, DPM, reg_val));

    reg_val = 0;
    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, TRAFFIC_MIRROR_CONTROL0, SPMDPMOP, reg_val));

    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, TRAFFIC_MIRROR_CONTROL1, MORG, 1));
    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, TRAFFIC_MIRROR_CONTROL0, CROSSVLAN, 1));
    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, TRAFFIC_MIRROR_CONTROL0, MUA, 1));
    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, TRAFFIC_MIRROR_CONTROL0, MMA, 1));
    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, TRAFFIC_MIRROR_CONTROL0, MBA, 1));
    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, TRAFFIC_MIRROR_CONTROL0, MBPKT, 1));
    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, TRAFFIC_MIRROR_CONTROL0, MGPKT, 1));
    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, TRAFFIC_MIRROR_CONTROL2, MPTKPP, 1));

    return RT_ERR_OK;

}


rtk_api_ret_t rtl8316d_mirror_portBased_get(rtk_port_t* pMirroring_port, rtk_portmask_t *pMirrored_rx_mask, rtk_portmask_t *pMirrored_tx_mask)
{
//    uint32 retVal;
    uint32 reg_val;

    if (pMirroring_port == NULL || pMirrored_rx_mask == NULL || pMirrored_tx_mask == NULL)
        return RT_ERR_NULL_POINTER;

    *pMirroring_port = 0;
    memset(pMirrored_rx_mask, 0, sizeof(rtk_portmask_t));
    memset(pMirrored_tx_mask, 0, sizeof(rtk_portmask_t));

    CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, TRAFFIC_MIRROR_CONTROL1, DP, &reg_val));
    *pMirroring_port = reg_val;

    CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, TRAFFIC_MIRROR_CONTROL0, SPM, &reg_val));
    pMirrored_rx_mask->bits[0] = reg_val;

    CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, TRAFFIC_MIRROR_CONTROL1, DPM, &reg_val));
    pMirrored_tx_mask->bits[0] = reg_val;

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_mirror_portIso_set(uint32 enabled)
{

   enabled = enabled ? 1:0;
   CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, MIRROR_PORT_EGRESS_FILTER_CONTROL, MIREGFILTER, enabled));

    return RT_ERR_OK;
}


rtk_api_ret_t rtl8316d_mirror_portIso_get(uint32 *pEnabled)
{
   if ( NULL == pEnabled )
       return RT_ERR_NULL_POINTER;

   CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, MIRROR_PORT_EGRESS_FILTER_CONTROL, MIREGFILTER, pEnabled));

   return RT_ERR_OK;

}




int32 rtl8316d_portStat_phyAbility_get(rtk_port_t port, rtk_port_phy_ability_t* pAbility)
{
    uint16 reg0_val;
    uint16 reg4_val;
    uint16 reg9_val;

    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    phy_reg_read((uint32)port, 0, 0, &reg0_val);
    phy_reg_read((uint32)port, 4, 0, &reg4_val);
    phy_reg_read((uint32)port, 9, 0, &reg9_val);

    pAbility->AutoNegotiation = (reg0_val >> 12) & 0x01;
    pAbility->Half_10   =   (reg4_val >> 5) & 0x01;
    pAbility->Full_10   =   (reg4_val >> 6) & 0x01;
    pAbility->Half_100  =   (reg4_val >> 7) & 0x01;
    pAbility->Full_100  =   (reg4_val >> 8) & 0x01;
    pAbility->Full_1000 =   (reg9_val >> 9) & 0x01;
    pAbility->FC        =   (reg4_val >> 10) & 0x01;
    pAbility->AsyFC     =   (reg4_val >> 11) & 0x01;

    return RT_ERR_OK;
}

int32 rtl8316d_portStat_phyAbility_auto_set(rtk_port_t port, rtk_port_phy_ability_t* ability)
{
    uint16 reg0_val;
    uint16 reg4_val;
    uint16 reg9_val;

    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    // modify reg 4
    phy_reg_read((uint32)port, 4, 0, &reg4_val);

    reg4_val &= ~((uint32)0xDE0);
    reg4_val |= (uint32)ability->Half_10    << 5;
    reg4_val |= (uint32)ability->Full_10    << 6;
    reg4_val |= (uint32)ability->Half_100   << 7;
    reg4_val |= (uint32)ability->Full_100   << 8;
    reg4_val |= (uint32)ability->FC         << 10;
    reg4_val |= (uint32)ability->AsyFC      << 11;

    phy_reg_write(port, 4, 0, reg4_val);

    // modify reg 9
    phy_reg_read((uint32)port, 9, 0, &reg9_val);

    reg9_val &= ~((uint32)1 << 9);
    reg9_val |= (uint32)ability->Full_1000 << 9;

    phy_reg_write(port, 9, 0, reg9_val);

    // modify reg 0
    phy_reg_read((uint32)port, 0, 0, &reg0_val);

    reg0_val &= ~((uint32)1 << 12);
    reg0_val |= (uint32)ability->AutoNegotiation << 12;

    phy_reg_write(port, 0, 0, reg0_val);

    return RT_ERR_OK;
}

int32 rtl8316d_portStat_phyAbility_force_set(rtk_port_t port, rtk_port_phy_ability_t* ability)
{
    return RT_ERR_OK;
}

int32 rtl8316d_portStat_phyAbility_set(rtk_port_t port, rtk_port_phy_ability_t* ability)
{
    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (ability->AutoNegotiation == 1)
        return rtl8316d_portStat_phyAbility_auto_set(port, ability);
    else
        return rtl8316d_portStat_phyAbility_force_set(port, ability);

    return RT_ERR_OK;
}

int32 rtl8316d_portStat_LinkStat_get(rtk_port_t port, rtl8316d_portStat_info_t* pStat)
{
    uint32 reg_val;
    uint16 phy_reg_val;
    uint32 retVal;

    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_LINK_STATUS + port, DUP_STA_P0, &reg_val))!=RT_ERR_OK)
        return retVal;

    pStat->duplex = reg_val;

    if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_LINK_STATUS + port, SPD_STA_P0, &reg_val))!=RT_ERR_OK)
        return retVal;

    pStat->spd = reg_val;

    if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_LINK_STATUS + port, LINK_STA_P0, &reg_val))!=RT_ERR_OK)
        return retVal;

    pStat->Link = reg_val;

    if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_LINK_STATUS + port, TX_PAUSE_STA_P0, &reg_val))!=RT_ERR_OK)
        return retVal;

    pStat->FC = reg_val;

    phy_reg_read(port, 0, 0, &phy_reg_val);

    pStat->AutoNego = (phy_reg_val >> 12) & 0x01;

    return RT_ERR_OK;
}

int32 rtl8316d_portStat_Enable_get(rtk_port_t port, rtk_enable_t* pEnable)
{
    uint32 reg_val;
    uint32 retVal;

    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_LINK_STATUS + port, PHY_PWR_DOWN_P0, &reg_val))!=RT_ERR_OK)
        return retVal;

    if (reg_val == 0)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    return RT_ERR_OK;
}

rtk_api_ret_t rtl8316d_portStat_Enable_set(rtk_port_t port, rtk_enable_t enable)
{
    uint32 reg_val; 
    uint32 retVal;
    uint32 pollcnt, finished;
    
    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (enable == ENABLED)
        reg_val = 0;
    else
        reg_val = 1;

    if (port < 22) /* the difference of PHY_PWR_DOWN_Pn between port 0~21 and port 22~24 is different*/
    {
        if((retVal=reg_field_write(RTL8316D_UNIT, PORT0_PROPERTY_CONFIGURE + port, PHY_PWR_DOWN_P0 + port * (PHY_PWR_DOWN_P1 - PHY_PWR_DOWN_P0), reg_val))!=RT_ERR_OK)
                return retVal;
    }
    else 
    {
        if((retVal=reg_field_write(RTL8316D_UNIT, PORT0_PROPERTY_CONFIGURE + port, PHY_PWR_DOWN_P22 + (port - 22) * (PHY_PWR_DOWN_P23 - PHY_PWR_DOWN_P22), reg_val))!=RT_ERR_OK)
            return retVal;
    }

    /*triger, write configure to phy*/
    reg_val = (1UL << 25) | ( 1UL << port);
    CHK_FUN_RETVAL(reg_write(RTL8316D_UNIT, PHY_AUTO_ACCESS_MASK0, reg_val));
    
    for (pollcnt = 0; pollcnt < RTL8316D_MAX_POLLCNT; pollcnt ++)    
    {
        CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PHY_AUTO_ACCESS_MASK0, PHYCFG_CMD, &finished));
        if(!finished)
            break;
    }

    if(RTL8316D_MAX_POLLCNT == pollcnt)
        return RT_ERR_BUSYWAIT_TIMEOUT;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_int_globalEn_set
 * Description:
 *      Enable/disable the global interrupt mask.
 * Input:
 *      enable -  Interrupt mask, should be ENABLED or DISABLED.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      The API can enable/disable the global interrupt mask. Any interrupt could be signled to
 *      CPU/external interrupt output pin only after enabling this global interrupt mask.
 */
rtk_api_ret_t rtk_int_globalEn_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    retVal = reg_field_write(RTL8316D_UNIT, SWITCH_INTERRUPT_GLOBAL_CONTROL, SWITCH_IE, enable);
    return retVal;
}

/* Function Name:
 *      rtk_int_globalEn_get
 * Description:
 *      Get the global interrupt mask .
 * Input:
 *      None
 * Output:
 *      pEnable -  Interrupt mask, should be ENABLED or DISABLED.
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can retrieve whether the global interrupt mask is enabled or not. Any interrupt
 *      could be signled to CPU/external interrupt output pin only after enabling this global interrupt mask.
 */
rtk_api_ret_t rtk_int_globalEn_get(rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;

    if (pEnable == NULL)
        return RT_ERR_NULL_POINTER;

    retVal = reg_field_read(RTL8316D_UNIT, SWITCH_INTERRUPT_GLOBAL_CONTROL, SWITCH_IE, pEnable);
    return retVal;
}



/* Function Name:
 *      rtk_int_modMask_set
 * Description:
 *      Set modules' interrupt mask configuration.
 * Input:
 *      type - Interrupt type.
 *      enable - Interrupt mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_INPUT - Error interrupt type.
 *      RT_ERR_ENABLE - enable should ENABLED or DISABLED.
 * Note:
 *      The API can set modules' global interrupt mask configuration.
 *      The modules' interrupt mask is shown in the following:
 *      -INT_GLOBAL_ACLHIT0
 *      -INT_GLOBAL_ACLHIT1
 *      -INT_GLOBAL_ACLHIT2
 *      -INT_GLOBAL_ACLHIT3
 *      -INT_GLOBAL_SAMOV
 *      -INT_GLOBAL_MIBOVERFLOW
 *      -INT_GLOBAL_UDLD
 *      -INT_GLOBAL_LINKCHANGE
 *      -INT_GLOBAL_PKTDROP
 *      -INT_GLOBAL_SALRNLIMIT
 *      -INT_GLOBAL_LOOPDETECT
 *      -INT_GLOBAL_SERDES
 *      -INT_GLOBAL_DYINGGSP
 *      -INT_GLOBAL_LOOPRESOLVED
 */
rtk_api_ret_t rtk_int_modMask_set(rtk_int_type_t type, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    rtk_reg_list_t regAddr;
    uint32 mask;

    if (type >= INT_GLOBAL_END)
        return RT_ERR_INPUT;

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if (type >= INT_GLOBAL_DYINGGSP)
    {
        regAddr = MISC_INTERRUPT_CONTROL;
        type = type - INT_GLOBAL_DYINGGSP;
    } else {
        regAddr = SWITCH_INTERRUPT_GLOBAL_CONTROL;
    }

    if ((retVal=reg_read(RTL8316D_UNIT, regAddr, &mask)) != SUCCESS)
    {
        return retVal;
    }

    if (enable == ENABLED)
        mask = mask | (1<<type);
    else if (enable==DISABLED)
        mask = mask & ~(1<<type);
    else
        return RT_ERR_ENABLE;

    retVal = reg_write(RTL8316D_UNIT, regAddr, mask);

    return retVal;
}

/* Function Name:
 *      rtk_int_modMask_get
 * Description:
 *      Get modules' interrupt mask configuration.
 * Input:
 *      type - Interrupt type.
 * Output:
 *      pEnable - Interrupt mask value.
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_INPUT - Error interrupt type.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get modules' global interrupt mask configuration.
 *      The modules' interrupt mask is shown in the following:
 *      -INT_GLOBAL_ACLHIT0
 *      -INT_GLOBAL_ACLHIT1
 *      -INT_GLOBAL_ACLHIT2
 *      -INT_GLOBAL_ACLHIT3
 *      -INT_GLOBAL_SAMOV
 *      -INT_GLOBAL_MIBOVERFLOW
 *      -INT_GLOBAL_UDLD
 *      -INT_GLOBAL_LINKCHANGE
 *      -INT_GLOBAL_PKTDROP
 *      -INT_GLOBAL_SALRNLIMIT
 *      -INT_GLOBAL_LOOPDETECT
 *      -INT_GLOBAL_SERDES
 *      -INT_GLOBAL_DYINGGSP
 *      -INT_GLOBAL_LOOPRESOLVED
 */
rtk_api_ret_t rtk_int_modMask_get(rtk_int_type_t type, rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;
    uint32 mask;

    if (type >= INT_GLOBAL_END)
        return RT_ERR_INPUT;

    if (pEnable == NULL)
        return RT_ERR_NULL_POINTER;

    if (type >= INT_GLOBAL_DYINGGSP)
    {
        retVal = reg_read(RTL8316D_UNIT, MISC_INTERRUPT_CONTROL, &mask);
        type = type - INT_GLOBAL_DYINGGSP;
    } else {
        retVal = reg_read(RTL8316D_UNIT, SWITCH_INTERRUPT_GLOBAL_CONTROL, &mask);
    }

    *pEnable = (mask >> type) & 0x1;

    return retVal;
}

/* Function Name:
 *      rtk_int_modStatus_get
 * Description:
 *      Get modules' interrupt pending flags.
 * Input:
 *      None
 * Output:
 *      pStatusMask - Interrupt pending flag mask.
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_INPUT - Error interrupt type.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get modules' global interrupt pending flags.
 *      The bit mapping of modules' pending flags is shown in the following:
 *      bit0: INT_GLOBAL_RESERVED0 = 0,
 *      bit1: INT_GLOBAL_SERDES,
 *      bit2: INT_GLOBAL_LOOPDETECT,
 *      bit3: INT_GLOBAL_SALRNLIMIT,
 *      bit4: INT_GLOBAL_PKTDROP,
 *      bit5: INT_GLOBAL_LINKCHANGE,
 *      bit6: INT_GLOBAL_RESERVED6 = 6,
 *      bit7: INT_GLOBAL_RESERVED7 = 7,
 *      bit8: INT_GLOBAL_UDLD,
 *      bit9: INT_GLOBAL_MIBOVERFLOW,
 *      bit10:INT_GLOBAL_SAMOV,
 *      bit11:INT_GLOBAL_ACLHIT0,
 *      bit12:INT_GLOBAL_ACLHIT1,
 *      bit13:INT_GLOBAL_ACLHIT2,
 *      bit14:INT_GLOBAL_ACLHIT3,
 *      bit15:INT_GLOBAL_DYINGGSP,
 *      bit16:INT_GLOBAL_LOOPRESOLVED,
 *      These flags reflect the corresponding modules' interrupt status, and will not be cleared unless the
 *      particular modules' (which have interrupt signaled) interrupt status relevent APIs are called.
 */
rtk_api_ret_t rtk_int_modStatus_get(rtk_int_status_t* pStatusMask)
{
    rtk_api_ret_t retVal;
    uint32 mask;

    if (NULL == pStatusMask)
    {
        return RT_ERR_NULL_POINTER;
    }

    if ((retVal = reg_read(RTL8316D_UNIT, MISC_INTERRUPT_CONTROL, &mask)) != SUCCESS)
    {
       return retVal;
    }

    if ((retVal = reg_read(RTL8316D_UNIT, SWITCH_INTERRUPT_GLOBAL_CONTROL, (uint32*)pStatusMask)) != SUCCESS)
    {
        return retVal;
    }

    /*get INT_GLOBAL_DYINGGSP and INT_GLOBAL_LOOPRESOLVED*/
    *pStatusMask |= (mask & 0x3) << 15;

    /*DYING gasp and loop resolved interrupt pending flags cleared here*/
    retVal = reg_write(RTL8316D_UNIT, MISC_INTERRUPT_CONTROL, 0xFFFFFFFF);

    return retVal;
}


/* Function Name:
 *      rtk_int_linkChangeMsk_set
 * Description:
 *      Set ports' link status change interrupt masks.
 * Input:
 *      pPortMask - Port mask.
 *      enable -
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_ENABLE -  Invalid enable input, shuold be ENABLED or DISABLED.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can enable/disable ports to signal an interrupt when link status change. It will only affect
 *      those ports that are specificed in pPortMask.
 */
rtk_api_ret_t rtk_int_linkChangeMsk_set(rtk_portmask_t* pPortMask, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    uint32 mask;

    if (pPortMask == NULL)
        return RT_ERR_NULL_POINTER;

    if(pPortMask->bits[0] > RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_ID;

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    if ((retVal = reg_read(RTL8316D_UNIT, PER_PORT_LINK_CHANGE_INTERRUPT_CONTROL, &mask)) != SUCCESS)
    {
        return retVal;
    }

    if (enable == ENABLED)
    {
        mask |= pPortMask->bits[0];
    } else {
        mask &= ~pPortMask->bits[0];
    }

    retVal = reg_write(RTL8316D_UNIT, PER_PORT_LINK_CHANGE_INTERRUPT_CONTROL, mask);

    return retVal;
}

/* Function Name:
 *      rtk_int_linkChangeMsk_get
 * Description:
 *      Get port's link status change  interrupt mask.
 * Input:
 *      port - Port ID.
 * Output:
 *      pEnable
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_PORT_ID -  Invalid port ID.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get port's link status change interrupt mask.
 */
rtk_api_ret_t rtk_int_linkChangeMsk_get(rtk_port_t port, rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;
    uint32 mask;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (NULL == pEnable)
    {
        return RT_ERR_NULL_POINTER;
    }

    if ((retVal = reg_read(RTL8316D_UNIT, PER_PORT_LINK_CHANGE_INTERRUPT_CONTROL, &mask)) != SUCCESS)
    {
        return retVal;
    }

    *pEnable = (mask >> port) & 0x1;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_int_linkChangeStatus_get
 * Description:
 *      Get port's link status change interrupt pending flags.
 * Input:
 *      None
 * Output:
 *      pStatusMask -   Interrupt pending flags returned.
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get port's link status change interrupt pending flags. Pending flags will be cleared after
  *     being retrieved.
 */
rtk_api_ret_t rtk_int_linkChangeStatus_get(rtk_int_status_t* pStatusMask)
{
    rtk_api_ret_t retVal;

    if (NULL == pStatusMask)
    {
        return RT_ERR_NULL_POINTER;
    }

    if ((retVal = reg_read(RTL8316D_UNIT, PER_PORT_LINK_CHANGE_INTERRUPT_STATUS, (uint32*)pStatusMask)) != SUCCESS)
    {
        return retVal;
    }

    retVal = reg_write(RTL8316D_UNIT, PER_PORT_LINK_CHANGE_INTERRUPT_STATUS, 0xFFFFFFFF);

    return retVal;
}

/* Function Name:
 *      rtk_int_smacLimitMsk_set
 * Description:
 *      Set ports' source MAC limit interrupt masks.
 * Input:
 *      pPortMask - Port mask.
 *      enable - ENABLED or DISABLED
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_ENABLE -  enable should be ENABLED or DISABLED.
 *      RT_ERR_PORT_ID -  Invalid port ID.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can enable/disable ports to signal an interrupt when the number of source MAC auto-learned
 *      reaches the threshold allowed. It will only affect those ports that are specificed in pPortMask.
 */
rtk_api_ret_t rtk_int_smacLimitMsk_set(rtk_portmask_t* pPortMask, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    uint32 mask;

    if (pPortMask == NULL)
        return RT_ERR_NULL_POINTER;

    if(pPortMask->bits[0] > RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_ID;

    if (enable >= RTK_ENABLE_END)
    {
        return RT_ERR_ENABLE;
    }

    if ((retVal = reg_read(RTL8316D_UNIT, PER_PORT_SA_LEARNING_CONSTRAIN_INTERRUPT_CONTROL, &mask)) != SUCCESS)
    {
        return retVal;
    }

    if (enable == ENABLED)
    {
        mask |= pPortMask->bits[0];
    } else {
        mask &= ~pPortMask->bits[0];
    }

    retVal = reg_write(RTL8316D_UNIT, PER_PORT_SA_LEARNING_CONSTRAIN_INTERRUPT_CONTROL, mask);

    return retVal;
}


/* Function Name:
 *      rtk_int_smacLimitMsk_get
 * Description:
 *      Get port's source MAC limit interrupt mask.
 * Input:
 *      port - Port ID.
 * Output:
 *      pEnable - ENABLED or DISABLED
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_PORT_ID -  Invalid port ID.
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get port's interrupt mask of source MAC auto-learned limitation .
 */
rtk_api_ret_t rtk_int_smacLimitMsk_get(rtk_port_t port, rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;
    uint32 mask;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (NULL == pEnable)
    {
        return RT_ERR_NULL_POINTER;
    }

    if ((retVal = reg_read(RTL8316D_UNIT, PER_PORT_SA_LEARNING_CONSTRAIN_INTERRUPT_CONTROL, &mask)) != SUCCESS)
    {
        return retVal;
    }

    *pEnable = (mask >> port) & 0x1;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_int_smacLimitStatus_get
 * Description:
 *      Get port's source MAC limit interrupt pending flags.
 * Input:
 *      None
 * Output:
 *      pStatusMask - Interrupt pending flags returned.
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get ports' interrupt pending flags of source MAC auto-learned limitation .The interrupt
 *      pending flags will be cleared after retrieved.
 */
rtk_api_ret_t rtk_int_smacLimitStatus_get(rtk_int_status_t* pStatusMask)
{
    rtk_api_ret_t retVal;

    if (NULL == pStatusMask)
    {
        return RT_ERR_NULL_POINTER;
    }

    if ((retVal = reg_read(RTL8316D_UNIT, PER_PORT_SA_LEARNING_CONSTRAIN_INTERRUPT_CONTROL, (uint32*)pStatusMask)) != SUCCESS)
    {
        return retVal;
    }

    retVal = reg_write(RTL8316D_UNIT, PER_PORT_SA_LEARNING_CONSTRAIN_INTERRUPT_CONTROL, 0xFFFFFFFF);

    return retVal;
}

/* Function Name:
 *      rtk_int_saMovStatus_get
 * Description:
 *      Get Source MAC moving interrupt pending flags.
 * Input:
 *      None
 * Output:
 *      pStatusMask - Interrupt pending flags returned.
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get source MAC moving interrupt pending flags. Pending flags will be cleared after
 *      being retrieved.
 */
rtk_api_ret_t rtk_int_saMovStatus_get(rtk_int_status_t* pStatusMask)
{
    rtk_api_ret_t retVal;

    if (NULL == pStatusMask)
    {
        return RT_ERR_NULL_POINTER;
    }

    if ((retVal = reg_read(RTL8316D_UNIT, SOURCE_MAC_MOVING_PORT_MASK, (uint32*)pStatusMask)) != SUCCESS)
    {
        return retVal;
    }

    retVal = reg_write(RTL8316D_UNIT, SOURCE_MAC_MOVING_PORT_MASK, 0xFFFFFFFF);

    return retVal;
}


/* Function Name:
 *      rtk_int_loopDetecStatus_get
 * Description:
 *      Get loop detected interrupt pending flags.
 * Input:
 *      None
 * Output:
 *      pStatusMask - Interrupt pending flags returned.
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get loop detected interrupt pending flags. Pending flags will be cleared after
 *      being retrieved.
 */
rtk_api_ret_t rtk_int_loopDetecStatus_get(rtk_int_status_t* pStatusMask)
{
    rtk_api_ret_t retVal;

    if (NULL == pStatusMask)
    {
        return RT_ERR_NULL_POINTER;
    }

    if ((retVal = reg_read(RTL8316D_UNIT, RLDP_INTERRUPT_PENDING_MASK_CONTROL, (uint32*)pStatusMask)) != SUCCESS)
    {
        return retVal;
    }

    retVal = reg_write(RTL8316D_UNIT, RLDP_INTERRUPT_PENDING_MASK_CONTROL, 0xFFFFFFFF);

    return retVal;
}


/* Function Name:
 *      rtk_int_udldStatus_get
 * Description:
 *      Get UniDirection Link Detect interrupt pending flags.
 * Input:
 *      None
 * Output:
 *      pStatusMask - Interrupt pending flags returned.
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_NULL_POINTER - NULL pointer input.
 * Note:
 *      The API can get UDLD interrupt pending flags. Pending flags will be cleared after
 *       being retrieved.
 */
rtk_api_ret_t rtk_int_udldStatus_get(rtk_int_status_t* pStatusMask)
{
    rtk_api_ret_t retVal;

    if (NULL == pStatusMask)
    {
        return RT_ERR_NULL_POINTER;
    }

    if ((retVal = reg_read(RTL8316D_UNIT, UDL_INTERRUPT_PENDING_MASK_CONTROL, (uint32*)pStatusMask)) != SUCCESS)
    {
        return retVal;
    }

    retVal = reg_write(RTL8316D_UNIT, UDL_INTERRUPT_PENDING_MASK_CONTROL, 0xFFFFFFFF);

    return retVal;
}


/* Function Name:
 *      rtk_int_aclHitMsk_set
 * Description:
 *      Set ACL rule hit interrupt masks.
 * Input:
 *      blockId - PIE module ID.
 *      ruleId - ACL rule ID.
 *      enable - Interrupt mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_INPUT - Error block ID or rule ID.
 *      RT_ERR_ENABLE - enable should be ENABLED or DISABLED.
 * Note:
 *      The API can enable/disable specified ACL rule to signal an interrupt when the incoming packet hit the rule.
 */
rtk_api_ret_t rtk_int_aclHitMsk_set(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    uint32 mask;

    if (blockId >= RTK_ACL_BLOCK_MAX)
        return RT_ERR_INPUT;

    if (ruleId >= RTK_ACL_BLOCKRULE_MAX)
        return RT_ERR_INPUT;

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = reg_read(RTL8316D_UNIT, ACL_RULE_HIT_INTERRUPT_CONTROL0+blockId, &mask)) != SUCCESS)
    {
        return retVal;
    }

    if (enable == ENABLED)
    {
        mask |= 1 << ruleId;
    } else {
        mask &= ~(1 << ruleId);
    }

    retVal = reg_write(RTL8316D_UNIT, ACL_RULE_HIT_INTERRUPT_CONTROL0+blockId, mask);

    return retVal;
}


/* Function Name:
 *      rtk_int_aclHitMsk_get
 * Description:
 *      Get ACL rule hit interrupt masks.
 * Input:
 *      blockId - PIE module ID.
 *      ruleId - ACL rule ID.
 * Output:
 *      pEnable - Interrupt mask returned.
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_INPUT - Error block ID or rule ID.
 *      RT_ERR_NULL_POINTER - NULL pointer returned.
 * Note:
 *      The API can get ACL rule hit interrupt mask.
 */
rtk_api_ret_t rtk_int_aclHitMsk_get(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_enable_t* pEnable)
{
    rtk_api_ret_t retVal;
    uint32 mask;

    if (blockId >= RTK_ACL_BLOCK_MAX)
        return RT_ERR_INPUT;

    if (ruleId >= RTK_ACL_BLOCKRULE_MAX)
        return RT_ERR_INPUT;

    if (NULL == pEnable)
    {
        return RT_ERR_NULL_POINTER;
    }

    if ((retVal = reg_read(RTL8316D_UNIT, ACL_RULE_HIT_INTERRUPT_CONTROL0+blockId, &mask)) != SUCCESS)
    {
        return retVal;
    }

    *pEnable = (mask >> ruleId) & 0x1;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_int_aclHitStatus_get
 * Description:
 *      Get ACL rule hit interrupt pending flags.
 * Input:
 *      blockId - PIE module ID.
 * Output:
 *      pStatusMask - Interrupt pending flags returned.
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_INPUT - Error block ID.
 *      RT_ERR_NULL_POINTER - NULL pointer returned.
 * Note:
 *      The API can get ACL rule hit interrupt pending flags. The interrupt pending flags will be cleared
 *      after retrieved.
 */
rtk_api_ret_t rtk_int_aclHitStatus_get(rtk_aclBlock_id_t blockId, rtk_int_status_t* pStatusMask)
{
    rtk_api_ret_t retVal;

    if (blockId >= RTK_ACL_BLOCK_MAX)
        return RT_ERR_INPUT;

    if (NULL == pStatusMask)
    {
        return RT_ERR_NULL_POINTER;
    }

    if ((retVal = reg_read(RTL8316D_UNIT, ACL_RULE_HIT_INTERRUPT_CONTROL0+blockId, (uint32*)pStatusMask)) != SUCCESS)
    {
        return retVal;
    }

    retVal = reg_write(RTL8316D_UNIT, ACL_RULE_HIT_INTERRUPT_CONTROL0+blockId, 0xFFFFFFFF);

    return retVal;
}

/* Function Name:
 *      rtl8316d_setAsicPHYReg
 * Description:
 *      Set PHY registers .
 * Input:
 *      pPhyAddr - PHY index.
 *      pRegAddr - register address.
 *      data - data to be written.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_PORT_ID - Invalid PHY index (port ID).
 *      RT_ERR_PHY_REG_ID - Invalid register address.
 *      RT_ERR_NULL_POINTER - NULL pointer returned.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *
 */
rtk_api_ret_t rtl8316d_setAsicPHYReg(rtl8316d_phyAddr_t* pPhyAddr, rtl8316d_phyRegAddr_t* pRegAddr, uint32 data)
{
//    rtk_api_ret_t retVal;
    uint32 regData;
    uint32 busyFlag, delay;

    if (pPhyAddr == NULL || pRegAddr == NULL)
        return RT_ERR_NULL_POINTER;

    reg_write(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL2_FOR_CPU, data);

    regData = 0;
    /*CPU occupies SMI*/
    reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_OCCUPY_PHY, 1, &regData);
    /*write*/
    reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_RWOP, 1, &regData);

    if (pPhyAddr->phyAddrType == 0)
    {
        if (pPhyAddr->un.address > RTL8316D_MAX_PORT)
        {
            return RT_ERR_PORT_ID;
        }
        reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_PHYADDR_4_0, pPhyAddr->un.address, &regData);
        reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_WRITEPHYMETHOD, 0, &regData);
    } else {
        /*write to multiple phys*/
        if (pPhyAddr->un.phyMask > 0x1FFFFFF)
        {
            return RT_ERR_PORT_ID;
        }
        reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_WRITEPHYMETHOD, 1, &regData);
        reg_write(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL1_FOR_CPU, pPhyAddr->un.phyMask);
    }

    if (pRegAddr->phyRegType == 0)
    {   /*clause 22*/
        if (pRegAddr->un.clause22.page > 0x1F || pRegAddr->un.clause22.addr > 0x1F)
        {
            return RT_ERR_PHY_REG_ID;
        }
        reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_PAGE_4_0, pRegAddr->un.clause22.page, &regData);
        reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_REG_4_0, pRegAddr->un.clause22.addr, &regData);
        reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_PHY_REG_TYP, 0, &regData);
    } else {
        if (pRegAddr->un.mmdAddr > 0x1F){
            return RT_ERR_PHY_REG_ID;
        }
        /*clause 45*/
        reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_REG_4_0, pRegAddr->un.mmdAddr, &regData);
        reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_PHY_REG_TYP, 1, &regData);
    }

    reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_CMD, 1, &regData);

    reg_write(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, regData);

    delay = 0;
    do{
        reg_field_read(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_CMD, &busyFlag);

        if (busyFlag == 0)
            break;
        else {
            delay++;
            if (delay >= 1000)
                return RT_ERR_BUSYWAIT_TIMEOUT;
        }
    }while(1);

    return SUCCESS;
}


/* Function Name:
 *      rtl8316d_getAsicPHYReg
 * Description:
 *      Get PHY registers .
 * Input:
 *      phyAddr - PHY index.
 *      pRegAddr - register address.
 * Output:
 *      pData - register data returned.
 * Return:
 *      RT_ERR_OK  -
 *      RT_ERR_FAILED   -
 *      RT_ERR_NULL_POINTER - NULL pointer input
 *      RT_ERR_PORT_ID - Invalid PHY index (port ID).
 *      RT_ERR_PHY_REG_ID - Invalid register address.
 *      RT_ERR_NULL_POINTER - NULL pointer returned.
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *     The API can get internal PHY register 0~31. There are 8 internal PHYs in switch and each PHY can be
 *     accessed by software.
 */
rtk_api_ret_t rtl8316d_getAsicPHYReg(uint32 phyAddr, rtl8316d_phyRegAddr_t* pRegAddr, uint32 *pData)
{
//    rtk_api_ret_t retVal;
    uint32 regData;
    uint32 busyFlag;
    uint32 delay;

    if(phyAddr > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (pRegAddr == NULL || pData == NULL)
        return RT_ERR_NULL_POINTER;

    regData = 0;
    if (pRegAddr->phyRegType == 0)
    {   /*clause 22*/
        if (pRegAddr->un.clause22.page > 0x1F || pRegAddr->un.clause22.addr > 0x1F)
        {
            return RT_ERR_PHY_REG_ID;
        }
        reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_PAGE_4_0, pRegAddr->un.clause22.page, &regData);
        reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_REG_4_0, pRegAddr->un.clause22.addr, &regData);
        reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_PHY_REG_TYP, 0, &regData);
    } else {
        if (pRegAddr->un.mmdAddr > 0x1F){
            return RT_ERR_PHY_REG_ID;
        }
        /*clause 45*/
        reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_REG_4_0, pRegAddr->un.mmdAddr, &regData);
        reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_PHY_REG_TYP, 1, &regData);
    }

    reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_PHYADDR_4_0, phyAddr, &regData);
    reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_WRITEPHYMETHOD, 0, &regData);
    reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_RWOP, 3, &regData);
    reg_field_set(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_CMD, 1, &regData);

    delay = 0;
    do {
        reg_field_read(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL0_FOR_CPU, CPU_CMD, &busyFlag);

        if (busyFlag == 0)
            break;
        else {
            delay++;
            if (delay >= 1000)
                return RT_ERR_BUSYWAIT_TIMEOUT;
        }
    } while(1);

    reg_read(RTL8316D_UNIT, PHY_REG_ACCESS_CONTROL2_FOR_CPU, pData);

    return RT_ERR_OK;

}

#define DELAY                        10000
#define CLK_DURATION(clk)            { int i; for(i=0; i<clk; i++); }
	
/* clear global MIB counters */
rtk_api_ret_t rtk_stat_global_reset(void)
{
	int i;

	for(i=0; i<16; i++)
		rtk_stat_port_reset(i);

	rtk_stat_port_reset(24);
	
    return RT_ERR_OK;
}

int RTL8325D_init(void)
{
	uint32 value;

	/* Initial Chip */
	rtk_switch_init();

	/* clear global MIB counters */
	rtk_stat_global_reset2();

	// occupied by CPU
	rtlglue_reg32_read_data(0xbb640000, &value);
	rtlglue_reg32_write(0xbb640000, (value | 0x80000000));
	CLK_DURATION(DELAY);
	
	// stop and start counting for System standard Counter.
	rtlglue_reg32_write(0xBB640008, 0);
	CLK_DURATION(DELAY);
	rtlglue_reg32_write(0xBB640008, 0xfffffff);
	CLK_DURATION(DELAY);
	rtlglue_reg32_write(0xbb640000, (value & ~0x80000000));

	/* clear global MIB counters */
	rtk_stat_global_reset();

	// set RTL8325D RGMII port to Force linkup, set reg PORT24_PROPERTY_CONFIGURE bit[2:1] to 0b11, bit8=0.
	// disable EEE
	if (rtlglue_reg32_read_data(0xbb040088, &value) == RT_ERR_OK)
		//rtlglue_reg32_write(0xbb040088, ((value & ~0x100) | 0x6));
		rtlglue_reg32_write(0xbb040088, ((value & ~0x60170) | 0x6));

	// disable the polling mask for port16~port23. set reg PHY_AUTO_ACCESS_MASK1 to 0xFFFF
	rtlglue_reg32_write(0xbb040024, 0xFFFF);

	return RT_ERR_OK; 
}

int32 rtl8325d_portStat_dump(char *page)
{
	int		len, port;
    uint32 reg, value;
	uint16 value2;
		
	len = sprintf(page, "Dump 8325D Port Status:\n");

	for(port=0;port<16;port++)
	{
		reg = 0xbb04008c + (port * 4);
		if ((rtlglue_reg32_read_data(reg, &value)) == RT_ERR_OK) {
			
			len += sprintf(page+len, "Port%d ", port);
			value2 = 0;
		    phy_reg_read((uint32)port, 1, 0, &value2);

			if ((value2 & 0x4) == 0) {
				len += sprintf(page+len, "LinkDown\n");
				continue;
			}
			else {
				len += sprintf(page+len, "LinkUp | ");
			}
			len += sprintf(page+len, "%s duplex | ", (value & 0x10) ? "full":"half");
			len += sprintf(page+len, "Speed %s\n", (value & 0x4)? "100M":"10M");
			len += sprintf(page+len, "  pause ability %s | ", (value & 0x20) ?"enabled":"disabled");
			len += sprintf(page+len, "asymmetric pause ability %s\n", (value & 0x40) ?"enabled":"disabled");					
		}			
	}
	return len;
}

int32 rtl8325d_mibCounter_dump(int port)
{
	rtk_stat_counter_t cntr;
	
	if ( port == 24 )
		rtlglue_printf("\n<CPU port>\n");
	else
		rtlglue_printf("\n<Port: %d>\n", port);

	rtlglue_printf("Rx counters:\n");

	cntr=0;
	rtk_stat_port_get(port, IfInOctets, &cntr);
	rtlglue_printf("   Rcv %llu bytes,", cntr);
	cntr=0;
	rtk_stat_port_get(port, IfInUcastPkts, &cntr);
	rtlglue_printf(" Unicast %llu pkts,", cntr);
	cntr=0;
	rtk_stat_port_get(port, EtherStatsMulticastPkts, &cntr);
	rtlglue_printf(" Multicast %llu pkts,", cntr);
	cntr=0;
	rtk_stat_port_get(port, EtherStatsBroadcastPkts, &cntr);
	rtlglue_printf(" Broadcast %llu pkts\n", cntr);

	cntr=0;
	rtk_stat_port_get(port, EtherStatsPkts64Octets, &cntr);
	rtlglue_printf("   64: %llu pkts,", cntr);
	cntr=0;
	rtk_stat_port_get(port, EtherStatsPkts65to127Octets, &cntr);
	rtlglue_printf(" 65 -127: %llu pkts,", cntr);
	cntr=0;
	rtk_stat_port_get(port, EtherStatsPkts128to255Octets, &cntr);
	rtlglue_printf(" 128 -255: %llu pkts\n", cntr);
	cntr=0;
	rtk_stat_port_get(port, EtherStatsPkts256to511Octets, &cntr);
	rtlglue_printf("   256 - 511: %llu pkts,", cntr);
	cntr=0;
	rtk_stat_port_get(port, EtherStatsPkts512to1023Octets, &cntr);
	rtlglue_printf(" 512 - 1023: %llu pkts,", cntr);
	cntr=0;
	rtk_stat_port_get(port, EtherStatsPkts1024to1518Octets, &cntr);
	rtlglue_printf(" 1024 - max: %llu pkts\n", cntr);

	cntr=0;
	rtk_stat_port_get(port, EtherStatsDropEvents, &cntr);
	rtlglue_printf("   Drop: %llu,", cntr);
	cntr=0;
	rtk_stat_port_get(port, Dot3StatsFCSErrors, &cntr);
	rtlglue_printf(" FCSError: %llu,", cntr);
	cntr=0;
	rtk_stat_port_get(port, EtherStatsFragments, &cntr);
	rtlglue_printf(" Fragment: %llu,", cntr);
	cntr=0;
	rtk_stat_port_get(port, EtherStatsJabbers, &cntr);
	rtlglue_printf(" JabberErr: %llu\n", cntr);

	cntr=0;
	rtk_stat_port_get(port, EtherOversizeStats, &cntr);
	rtlglue_printf("   OverSize: %llu,", cntr);
	cntr=0;
	rtk_stat_port_get(port, EtherStatsUnderSizePkts, &cntr);
	rtlglue_printf(" UnderSize: %llu,", cntr);
	cntr=0;
	rtk_stat_port_get(port, Dot3ControlInUnknownOpcodes, &cntr);
	rtlglue_printf(" Unknown: %llu", cntr);
	cntr=0;
	rtk_stat_port_get(port, InOampduPkts, &cntr);
	rtlglue_printf(" inOampdu: %llu\n", cntr);
    
	cntr=0;
	rtk_stat_port_get(port, Dot1dTpPortInDiscards, &cntr);
	rtlglue_printf("   PortInDiscard: %llu,", cntr);
	cntr=0;
	rtk_stat_port_get(port, Dot3StatsSymbolErrors, &cntr);
	rtlglue_printf(" SymbolErr: %llu,", cntr);
	cntr=0;
	rtk_stat_port_get(port, Dot3InPauseFrames, &cntr);
	rtlglue_printf(" Pause: %llu\n", cntr);
	cntr=0;
	rtk_stat_port_get(port, EtherStatsOctets, &cntr);
	rtlglue_printf("   etherStatsOctets  %llu\n", cntr);
    
	rtlglue_printf("\nOutput counters:\n");
	
	cntr=0;
	rtk_stat_port_get(port, IfOutOctets, &cntr);
	rtlglue_printf("   Snd %llu bytes,", cntr);
	cntr=0;
	rtk_stat_port_get(port, IfOutUcastPkts, &cntr);
	rtlglue_printf(" Unicast %llu pkts,", cntr);
	cntr=0;
	rtk_stat_port_get(port, IfOutMulticastPkts, &cntr);
	rtlglue_printf(" Multicast %llu pkts,", cntr);
	cntr=0;
	rtk_stat_port_get(port, IfOutBroadcastPkts, &cntr);
	rtlglue_printf(" Broadcast %llu pkts\n", cntr);
    
	cntr=0;
	rtk_stat_port_get(port, EtherStatsCollisions, &cntr);
	rtlglue_printf("   Collision: %llu,", cntr);
	cntr=0;
	rtk_stat_port_get(port, Dot3StatsSingleCollisionFrames, &cntr);
	rtlglue_printf(" SingleCol: %llu,", cntr);
	cntr=0;
	rtk_stat_port_get(port, Dot3StatMultipleCollisionFrames, &cntr);
	rtlglue_printf(" MultipleCol: %llu,", cntr);
	cntr=0;
	rtk_stat_port_get(port, Dot3StatsLateCollisions, &cntr);
	rtlglue_printf(" LateCol: %llu\n", cntr);
	cntr=0;
	rtk_stat_port_get(port, Dot3StatsExcessiveCollisions, &cntr);
	rtlglue_printf("   ExcessiveCol: %llu,", cntr);
	cntr=0;
	rtk_stat_port_get(port, Dot3sDeferredTransmissions, &cntr);
	rtlglue_printf(" DeferredTx: %llu,", cntr);
	cntr=0;
	rtk_stat_port_get(port, Dot1dBasePortDelayExceededDiscards, &cntr);
	rtlglue_printf(" DelayExceededDiscard: %llu\n", cntr);
	cntr=0;
	rtk_stat_port_get(port, OutOampduPkts, &cntr);
	rtlglue_printf("   outOampdu: %llu,", cntr);
	cntr=0;
	rtk_stat_port_get(port, PktgenPkts, &cntr);
	rtlglue_printf(" pktgen: %llu,", cntr);
	cntr=0;
	rtk_stat_port_get(port, Dot3OutPauseFrames, &cntr);
	rtlglue_printf(" Pause: %llu\n", cntr);
    
	return 0;
}

