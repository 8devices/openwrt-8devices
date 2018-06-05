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
* Purpose : ASIC-level driver implementation for IEEE 802.1x .
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/

#ifndef __RTL8316D_ASICDRV_DOT1X_H__

#define __RTL8316D_ASICDRV_DOT1X_H__

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
extern rtk_api_ret_t rtk_dot1x_unauthPacketOper_set(rtk_port_t port, rtk_dot1x_unauth_action_t unauth_action);

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
extern rtk_api_ret_t rtk_dot1x_unauthPacketOper_get(rtk_port_t port, rtk_dot1x_unauth_action_t *pUnauth_action);

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
extern rtk_api_ret_t rtk_dot1x_eapolFrame2CpuEnable_set(rtk_enable_t enable);

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
extern rtk_api_ret_t rtk_dot1x_eapolFrame2CpuEnable_get(rtk_enable_t *pEnable);

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
extern rtk_api_ret_t rtk_dot1x_portBasedEnable_set(rtk_port_t port, rtk_enable_t enable);

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
extern rtk_api_ret_t rtk_dot1x_portBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

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
extern rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_set(rtk_port_t port, rtk_dot1x_auth_status_t port_auth);


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
extern rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_get(rtk_port_t port, rtk_dot1x_auth_status_t *pPort_auth);

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
extern rtk_api_ret_t rtk_dot1x_portBasedDirection_set(rtk_port_t port, rtk_dot1x_direction_t port_direction);

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
extern rtk_api_ret_t rtk_dot1x_portBasedDirection_get(rtk_port_t port, rtk_dot1x_direction_t *pPort_direction);


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
extern rtk_api_ret_t rtk_dot1x_macBasedEnable_set(rtk_port_t port, rtk_enable_t enable);

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
extern rtk_api_ret_t rtk_dot1x_macBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

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
extern rtk_api_ret_t rtk_dot1x_macBasedAuthMac_add(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid);

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
extern rtk_api_ret_t rtk_dot1x_macBasedAuthMac_del(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid);

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
extern rtk_api_ret_t rtk_dot1x_macBasedDirection_set(rtk_dot1x_direction_t mac_direction);

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
extern rtk_api_ret_t rtk_dot1x_macBasedDirection_get(rtk_dot1x_direction_t *pMac_direction);

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
extern rtk_api_ret_t rtk_dot1x_guestVlan_set(rtk_vlan_t vid);

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
extern rtk_api_ret_t rtk_dot1x_guestVlan_get(rtk_vlan_t *pVid);

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
extern rtk_api_ret_t rtk_dot1x_guestVlan2Auth_set(rtk_enable_t enable);

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
extern rtk_api_ret_t rtk_dot1x_guestVlan2Auth_get(rtk_enable_t *pEnable);

/*to be implemented*/
extern int32 rtk_dot1x_init(void);

/*implemented in RTL8370S*/
extern int32 rtk_dot1x_tagunauthPacketOper_get(rtk_port_t port, rtk_dot1x_unauth_action_t *pUnauth_action);
extern int32 rtk_dot1x_untagunauthPacketOper_get(rtk_port_t port, rtk_dot1x_unauth_action_t *pUnauth_action);
extern int32 rtk_dot1x_tagunauthPacketOper_set(rtk_port_t port, rtk_dot1x_unauth_action_t unauth_action);
extern int32 rtk_dot1x_untagunauthPacketOper_set(rtk_port_t port, rtk_dot1x_unauth_action_t unauth_action);

#endif  /*__RTL8316D_ASICDRV_DOT1X_H__*/


