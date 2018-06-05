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
* Purpose : ASIC-level driver implementation for IEEE 802.1x
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/

#include <rtk_types.h>
#include <rtk_api_ext.h>
#include <asicdrv/rtl8316d_types.h> 
#include <rtl8316d_reg_struct.h>
#include <rtl8316d_asicDrv.h>
#include <rtl8316d_table_struct.h> /*specify the file in SDK ASIC driver directory*/
#include <rtl8316d_asicdrv_tbl.h>
#include <rtl8316d_asicdrv_dot1x.h>


/* Function Name:
 *      rtk_dot1x_tagunauthPacketOper_set
 * Description:
 *      Set 802.1x unauth action configuration 
 * Input:
 *      port                 - Port id
 *      unauth_action   - 802.1X unauth action
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_DOT1X_PROC
 * Note:
 *      This API can set 802.1x unauth action configuration 
 *      The unauth action is as following:
 *      DOT1X_ACTION_DROP
 *      DOT1X_ACTION_TRAP2CPU
 *      DOT1X_ACTION_GUESTVLAN 
 */
rtk_api_ret_t rtk_dot1x_unauthPacketOper_set(rtk_port_t port, rtk_dot1x_unauth_action_t unauth_action)
{
//    rtk_api_ret_t retVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(unauth_action >= DOT1X_ACTION_END)
        return RT_ERR_DOT1X_PROC;

    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, GUEST_VLAN_GLOBAL_CONTROL, INUNTAGDOT1XUNAUTHBH, unauth_action));
    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, GUEST_VLAN_GLOBAL_CONTROL, OUTUNTAGDOT1XUNAUTHBH, unauth_action));
    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, GUEST_VLAN_GLOBAL_CONTROL, INTAGDOT1XUNAUTHBH, unauth_action));
    CHK_FUN_RETVAL(reg_field_write(RTL8316D_UNIT, GUEST_VLAN_GLOBAL_CONTROL, OUTTAGDOT1XUNAUTHBH, unauth_action));
    

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_unauthPacketOper_get
 * Description:
 *      get 802.1x unauth action configuration 
 * Input:
 *      port                  - Port id
 * Output:
 *      pUnauth_action   - 802.1X unauth action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_NULL_POINTER
 * Note:
 *      This API can set 802.1x unauth action configuration 
 *      The unauth action is as following:
 *      DOT1X_ACTION_DROP
 *      DOT1X_ACTION_TRAP2CPU
 *      DOT1X_ACTION_GUESTVLAN 
 */
rtk_api_ret_t rtk_dot1x_unauthPacketOper_get(rtk_port_t port, rtk_dot1x_unauth_action_t *pUnauth_action)
{
//    rtk_api_ret_t retVal;

    if ( NULL == pUnauth_action)
        return RT_ERR_NULL_POINTER;
    
    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, GUEST_VLAN_GLOBAL_CONTROL, INTAGDOT1XUNAUTHBH, pUnauth_action));

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_eapolFrame2CpuEnable_set
 * Description:
 *      Set 802.1x EAPOL packet trap to CPU configuration 
 * Input:
 *      enable  - the status of 802.1x EAPOL packet
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_ENABLE
 * Note:
 *   To support 802.1x authentication functionality, EAPOL frame (DMAC = 0x01-80-C2-00-00-03) has to
 *    be trapped to CPU.The status of EAPOL frame trap to CPU is as following:
 *    DISABLED
 *    ENABLED
 */
rtk_api_ret_t rtk_dot1x_eapolFrame2CpuEnable_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    retVal = reg_field_write(RTL8316D_UNIT, RMA_CONTROL0, ACT3, enable);

    if (retVal != SUCCESS)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_eapolFrame2CpuEnable_get
 * Description:
 *      Get 802.1x EAPOL packet trap to CPU configuration
 * Input:
 *      none
 * Output:
 *      pEnable  - the status of 802.1x EAPOL packet 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER
 * Note:
 *   To support 802.1x authentication functionality, EAPOL frame (DMAC = 0x01-80-C2-00-00-03) has to
 *    be trapped to CPU.The status of EAPOL frame trap to CPU is as following:
 *    DISABLED
 *    ENABLED
 */
rtk_api_ret_t rtk_dot1x_eapolFrame2CpuEnable_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    if (NULL == pEnable)
         return RT_ERR_NULL_POINTER;

    retVal = reg_field_read(RTL8316D_UNIT, RMA_CONTROL0, ACT3, pEnable);
    if (retVal != SUCCESS)
        return retVal;
    
    if (*pEnable != 1)
    {
        *pEnable = DISABLED;
    } else 
    {
        *pEnable = ENABLED;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_dot1x_portBasedEnable_set
 * Description:
 *      Set 802.1x port-based enable configuration
 * Input:
 *      port      - Port id
 *      enable   - The status of 802.1x port
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_ENABLE
 *      RT_ERR_DOT1X_PORTBASEDPNEN
 * Note:
 *      The API can update the port-based port enable register content. If a port is 802.1x 
 *       port based network access control "enabled", it should be authenticated so packets 
 *       from that port won't be dropped or trapped to CPU. The status of 802.1x port-based 
 *       network access control is as following:
 *       DISABLED
 *       ENABLED
 */
rtk_api_ret_t rtk_dot1x_portBasedEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if((retVal = reg_field_write(RTL8316D_UNIT, PORT0_BASED_DOT1X_CONTROL+port, DOT1XPORTEN, enable)) != SUCCESS)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedEnable_get
 * Description:
 *      Get 802.1x port-based enable configuration
 * Input:
 *      port        - Port id
 * Output:
 *      pEnable   - The status of 802.1x port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_ENABLE
 * Note:
 *     The API can get the 802.1x port-based port status.
 */
rtk_api_ret_t rtk_dot1x_portBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (pEnable == NULL)
        return RT_ERR_NULL_POINTER;

    if((retVal = reg_field_read(RTL8316D_UNIT, PORT0_BASED_DOT1X_CONTROL+port, DOT1XPORTEN, pEnable)) != SUCCESS)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_set
 * Description:
 *      Set 802.1x port-based auth. port configuration
 * Input:
 *      port          - Port id
 *      port_auth  - The status of 802.1x port
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_DOT1X_PORTBASEDAUTH
 * Note:
 *     The authenticated status of 802.1x port-based network access control is as following:
 *     UNAUTH
 *     AUTH
 */
rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_set(rtk_port_t port, rtk_dot1x_auth_status_t port_auth)
{
    rtk_api_ret_t retVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(port_auth >= AUTH_STATUS_END)
        return RT_ERR_DOT1X_PORTBASEDAUTH;

    if((retVal = reg_field_write(RTL8316D_UNIT, PORT0_BASED_DOT1X_CONTROL+port, DOT1XPORTAUTH, port_auth)) != SUCCESS)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_get
 * Description:
 *      get 802.1x port-based auth. port configuration
 * Input:
 *      port           - Port id
 * Output:
 *      pPort_auth  - The status of 802.1x port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 * Note:
 *      The API can get 802.1x port-based port auth.information.
 */
rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_get(rtk_port_t port, rtk_dot1x_auth_status_t *pPort_auth)
{
    rtk_api_ret_t retVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (NULL == pPort_auth)
        return RT_ERR_NULL_POINTER;

    if((retVal = reg_field_read(RTL8316D_UNIT, PORT0_BASED_DOT1X_CONTROL+port, DOT1XPORTAUTH, pPort_auth)) != SUCCESS)
        return retVal;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedDirection_set
 * Description:
 *      Set 802.1x port-based operational direction configuration
 * Input:
 *      port                 - Port id
 *      port_direction    - Operation direction
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 *      RT_ERR_DOT1X_PORTBASEDOPDIR
 * Note:
 *      The operate controlled direction of 802.1x port-based network access control is as following:
 *      BOTH
 *      IN
 */
rtk_api_ret_t rtk_dot1x_portBasedDirection_set(rtk_port_t port, rtk_dot1x_direction_t port_direction)
{
    rtk_api_ret_t retVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(port_direction >= DIRECTION_END)
        return RT_ERR_DOT1X_PORTBASEDOPDIR;

    if((retVal = reg_field_write(RTL8316D_UNIT, PORT0_EGRESS_DOT1X_CONTROL+port, DOT1XOPDIR, port_direction))!= SUCCESS)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_portBasedDirection_get
 * Description:
 *      get 802.1X port-based operational direction configuration
 * Input:
 *      port                   - Port id
 * Output:
 *      pPort_direction    - Operation direction
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 * Note:
 *      The API can get 802.1x port-based operational direction information.
 */
rtk_api_ret_t rtk_dot1x_portBasedDirection_get(rtk_port_t port, rtk_dot1x_direction_t *pPort_direction)
{
    rtk_api_ret_t retVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if ( NULL == pPort_direction)
        return RT_ERR_NULL_POINTER;

    if((retVal = reg_field_read(RTL8316D_UNIT, PORT0_EGRESS_DOT1X_CONTROL+port, DOT1XOPDIR, pPort_direction))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_macBasedEnable_set
 * Description:
 *      Set 802.1x mac-based port enable configuration
 * Input:
 *      port                   - Port id
 *      enable                - the status of 802.1x port 
 * Output:
 *      pPort_direction    - Operation direction
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_ENABLE
 *      RT_ERR_PORT_ID
 *      RT_ERR_DOT1X_MACBASEDPNEN
 * Note:
 *     If a port is 802.1x MAC based network access control "enabled", the incoming packets should 
 *     be authenticated so packets from that port won't be dropped or trapped to CPU.
 *     The status of 802.1x MAC-based network access control is as following:
 *     DISABLED
 *     ENABLED
 */
rtk_api_ret_t rtk_dot1x_macBasedEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if((retVal = reg_field_write(RTL8316D_UNIT, PORT0_MAC_BASED_DOT1X_CONTROL+port, DOT1XMACEN, enable)) != SUCCESS)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_macBasedEnable_get
 * Description:
 *      get 802.1x mac-based port enable configuration
 * Input:
 *      port                   - Port id
 * Output:
 *      pEnable              - the status of 802.1x port 
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_ENABLE
 *      RT_ERR_PORT_ID
 *      RT_ERR_INPUT
 * Note:
 *   If a port is 802.1x MAC based network access control "enabled", the incoming packets should 
 *   be authenticated so packets from that port wont be dropped or trapped to CPU.
 *   The status of 802.1x MAC-based network access control is as following:
 *   DISABLED
 *   ENABLED
 */
rtk_api_ret_t rtk_dot1x_macBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if((retVal = reg_field_read(RTL8316D_UNIT, PORT0_MAC_BASED_DOT1X_CONTROL+port, DOT1XMACEN, pEnable)) != SUCCESS)
        return retVal;
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_add
 * Description:
 *      Add an authenticated MAC to ASIC
 * Input:
 *      port                   - Port id
 *      pAuth_mac          - The authenticated MAC
 *      fid                      -  filtering database id  
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_MAC
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 * Note:
 *     The API can add a 802.1x authenticated MAC address to port. If the MAC does not exist in LUT, 
 *     user can't add this MAC to auth status.
 */
rtk_api_ret_t rtk_dot1x_macBasedAuthMac_add(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid)
{
    rtk_api_ret_t retVal;
    rtl8316d_tblasic_lutParam_t l2Table;
    rtk_mac_t macEntry;
    rtk_l2_ucastAddr_t l2_data;

    /* must be unicast address */
    if((NULL == pAuth_mac) || (pAuth_mac->octet[0] & 0x1))
        return RT_ERR_MAC;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;    

    if(fid > RTL8316D_VIDMAX)
        return RT_ERR_L2_FID;    

    memset(&l2Table,0,sizeof(l2Table));

    /* fill key (MAC,FID) to get L2 entry */
    _rtk_copy(l2Table.lutEther.mac.octet, pAuth_mac->octet, ETHER_ADDR_LEN);
    l2Table.lutEther.fid = fid;
    retVal = rtk_l2_addr_get(&macEntry, fid, &l2_data);
    if (RT_ERR_OK == retVal)
    {
        if (l2_data.port != port)
            return RT_ERR_DOT1X_MAC_PORT_MISMATCH;

        l2_data.auth = 1;
        retVal= rtk_l2_addr_add(&macEntry, &l2_data);
        return retVal;        
    }    
    else 
        return retVal;            

}

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_del
 * Description:
 *      Delete an authenticated MAC to ASIC
 * Input:
 *      port                 -  Port id
 *      fid                   -  filtering database id  
 * Output:
 *      pAuth_mac        - The authenticated MAC
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_MAC
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 * Note:
 *     The API can delete a 802.1x authenticated MAC address to port. It only change the auth status of
 *     the MAC and won't delete it from LUT.
 */
rtk_api_ret_t rtk_dot1x_macBasedAuthMac_del(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid)
{
    rtk_api_ret_t retVal;
    rtl8316d_tblasic_lutParam_t l2Table;
    rtk_mac_t macEntry;
    rtk_l2_ucastAddr_t l2_data;

    /* must be unicast address */
    if((NULL == pAuth_mac) || (pAuth_mac->octet[0] & 0x1))
        return RT_ERR_MAC;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;    

    if(fid > RTL8316D_VIDMAX)
        return RT_ERR_L2_FID;    

    memset(&l2Table,0,sizeof(l2Table));

    /* fill key (MAC,FID) to get L2 entry */
    _rtk_copy(l2Table.lutEther.mac.octet, pAuth_mac->octet, ETHER_ADDR_LEN);
    l2Table.lutEther.fid = fid;
    retVal = rtk_l2_addr_get(&macEntry, fid, &l2_data);
    if (retVal == RT_ERR_OK)
    {
        if (l2_data.port != port)
            return RT_ERR_DOT1X_MAC_PORT_MISMATCH;

        l2_data.auth = 0;
        retVal= rtk_l2_addr_add(&macEntry, &l2_data);
        return retVal;        
    }    
    else 
        return retVal;            

}

/* Function Name:
 *      rtk_dot1x_macBasedDirection_set
 * Description:
 *      Set 802.1x mac-based operational direction configuration
 * Input:
 *      mac_direction    -  Operation direction
 * Output:
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 *      RT_ERR_DOT1X_MACBASEDOPDIR
 * Note:
 *     The operate controlled direction of 802.1x mac-based network access control is as following:
 *     BOTH
 *     IN
 */
rtk_api_ret_t rtk_dot1x_macBasedDirection_set(rtk_dot1x_direction_t mac_direction)
{
    rtk_api_ret_t retVal;

    if(mac_direction >= DIRECTION_END)
        return RT_ERR_DOT1X_MACBASEDOPDIR;

    if((retVal= reg_field_write(RTL8316D_UNIT, DOT1X_NETWORK_ACCESS_RELATED_CONTROL, DOT1XMACOPDIR, mac_direction))!= SUCCESS)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_dot1x_macBasedDirection_get
 * Description:
 *      get 802.1x mac-based operational direction configuration
 * Input:
 *      none  
 * Output:
 *      pMac_direction - Operation direction
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_INPUT
 * Note:
 *      The API can get 802.1x mac-based operational direction information.
 */
rtk_api_ret_t rtk_dot1x_macBasedDirection_get(rtk_dot1x_direction_t *pMac_direction)
{
    rtk_api_ret_t retVal;

    if (NULL ==  pMac_direction)
    {
        return RT_ERR_NULL_POINTER;
    }
    
    if((retVal=reg_field_read(RTL8316D_UNIT, DOT1X_NETWORK_ACCESS_RELATED_CONTROL, DOT1XMACOPDIR, pMac_direction)) != SUCCESS)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_guestVlan_set
 * Description:
 *      Set 802.1x guest VLAN configuration
 * Input:
 *      vid  - guest VLAN ID
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 *      The API can set 802.1x guest VLAN ID.
 */
rtk_api_ret_t rtk_dot1x_guestVlan_set(rtk_vlan_t vid)
{
    rtk_api_ret_t retVal;

    if (vid > RTL8316D_VIDMAX)
        return RT_ERR_VLAN_VID;
    
    if((retVal = reg_field_write(RTL8316D_UNIT, GUEST_VLAN_GLOBAL_CONTROL, GVLANID, vid)) != RT_ERR_OK)
        return retVal; 

    return RT_ERR_OK;    
}


/* Function Name:
 *      rtk_dot1x_guestVlan_get
 * Description:
 *      Get 802.1x guest VLAN configuration
 * Input:
 *      none  
 * Output:
 *      pVid - guest VLAN ID
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER
 * Note:
 *      The API can get 802.1x guest VLAN ID.
 */
rtk_api_ret_t rtk_dot1x_guestVlan_get(rtk_vlan_t *pVid)
{
    rtk_api_ret_t retVal;

    if (NULL == pVid)
        return RT_ERR_NULL_POINTER;
    
    if((retVal = reg_field_read(RTL8316D_UNIT, GUEST_VLAN_GLOBAL_CONTROL, GVLANID, pVid)) != RT_ERR_OK)
        return retVal; 

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_guestVlan2Auth_set
 * Description:
 *      Set 802.1x guest VLAN to auth host configuration
 * Input:
 *      enable  - The status of guest VLAN to auth host 
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER
 * Note:
 *      The operational direction of 802.1x guest VLAN to auth host control is as following:
 *      DISABLED
 *      ENABLED
 */
rtk_api_ret_t rtk_dot1x_guestVlan2Auth_set(rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if((retVal= reg_field_write(RTL8316D_UNIT, DOT1X_NETWORK_ACCESS_RELATED_CONTROL, GVOPDIR, enable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_dot1x_guestVlan2Auth_get
 * Description:
 *      Get 802.1x guest VLAN to auth host configuration
 * Input:
 *      none
 * Output:
 *      pEnable  - The status of guest VLAN to auth host 
 * Return:
 *      RT_ERR_OK                      
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER
 * Note:
 *      The operational direction of 802.1x guest VLAN to auth host control is as following:
 *      DISABLED
 *      ENABLED
 */
rtk_api_ret_t rtk_dot1x_guestVlan2Auth_get(rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    if (NULL == pEnable)
        return RT_ERR_NULL_POINTER;
    
    if((retVal = reg_field_read(RTL8316D_UNIT, DOT1X_NETWORK_ACCESS_RELATED_CONTROL, GVOPDIR, pEnable)) !=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}




