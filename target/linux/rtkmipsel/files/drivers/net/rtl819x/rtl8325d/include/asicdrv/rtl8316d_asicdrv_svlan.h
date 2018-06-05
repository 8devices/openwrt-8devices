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
* Purpose : ASIC-level driver header for SVLAN.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/

#ifndef __RTL8316D_ASICDRV_VLAN_H__
#define __RTL8316D_ASICDRV_VLAH_H__

/* Function Name:
 *      rtk_svlan_init
 * Description:
 *      Initialize SVLAN Configuration
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_INPUT
 * Note:
 *    Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200 for Q-in-Q SLAN design. 
 *    User can set mathced ether type as service provider supported protocol. After call this API, 
 *    all ports are set as CVLAN port. you can use rtk_svlan_servicePort_add to add SVLAN port. 
 *    
 */
extern rtk_api_ret_t rtk_svlan_init(void);

/* Function Name:
 *      rtk_svlan_servicePort_add
 * Description:
 *      Enable one service port in the specified device
 * Input:
 *      port     -  Port id
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 *    This API is setting which port is connected to provider switch. All frames receiving from this port
 *    will recognize Service Vlan Tag.
 *    
 */
extern rtk_api_ret_t rtk_svlan_servicePort_add(rtk_port_t port);

/* Function Name:
 *      rtk_svlan_servicePort_del
 * Description:
 *      Disable one service port in the specified device
 * Input:
 *      none
 * Output:
 *      pSvlan_portmask  - svlan ports mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type
 *      0x9100 and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_servicePort_del(rtk_port_t port);

/* Function Name:
 *      rtk_svlan_servicePort_get
 * Description:
 *      Disable one service port in the specified device
 * Input:
 *      none
 * Output:
 *      pSvlan_portmask  - svlan ports mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_servicePort_get(rtk_portmask_t *pSvlan_portmask);

/* Function Name:
 *      rtk_svlan_tpidEntry_set
 * Description:
 *      Configure accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8
 * Input:
 *      svlan_tag_id  - Ether type of S-tag frame parsing in uplink ports
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_tpidEntry_set(rtk_svlan_tpid_t svlan_tag_id);

/* Function Name:
 *      rtk_svlan_tpidEntry_get
 * Description:
 *      Get accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8
 * Input:
 *      pSvlan_tag_id       - Ether type of S-tag frame parsing in uplink ports
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_tpidEntry_get(rtk_svlan_tpid_t *pSvlan_tag_id);

/* Function Name:
 *      rtk_svlan_tpidEntry_set
 * Description:
 *      Configure accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8
 * Input:
 *      svlan_tag_id  - Ether type of S-tag frame parsing in uplink ports
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority, rtk_dei_t dei);

/* Function Name:
 *      rtk_svlan_tpidEntry_get
 * Description:
 *      Get accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8
 * Input:
 *      pSvlan_tag_id       - Ether type of S-tag frame parsing in uplink ports
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t* pPriority, rtk_dei_t *pDei);

/* Function Name:
 *      rtk_svlan_portPvid_set
 * Description:
 *      Set port to specified VLAN ID(PVID) for Service Provider Port
 * Input:
 *      port             - Port id
 *      pvid             - Specified Service VLAN ID
 *      priority         - 802.1p priority for the PVID
 *      dei               - Service VLAN tag DEI bit
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_VLAN_VID
 *      RT_ERR_SMI 
 *      RT_ERR_VLAN_PRIORITY 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *    The API is used for Port-based VLAN. The untagged frame received from the
 *    port will be classified to the specified VLAN and assigned to the specified priority.
 */
extern rtk_api_ret_t rtk_svlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t accept_frame_type);

/* Function Name:
 *      rtk_svlan_portPvid_get
 * Description:
 *      Get Service VLAN ID(PVID) on specified port
 * Input:
 *      port             - Port id
 *      pPvid            - Specified VLAN ID
 *      pPriority        - 802.1p priority for the PVID
 *      pDei             - DEI bit
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_NULL_POINTER 
 * Note:
 *    The API is used for Port-based VLAN. The untagged frame received from the
 *    port will be classified to the specified VLAN and assigned to the specified priority.
 */
extern rtk_api_ret_t rtk_svlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type);

/* Function Name:
 *      rtk_vlan_stagMode_set
 * Description:
 *      Set SVLAN egress tag mode
 * Input:
 *      port                - Port id
 *      tag_mode        - Egress tag mode
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI 
 *      RT_ERR_INPUT
 * Note:
 *    The API is used for setting port vlan egress tag mode
 *    The tag mode  as following:
 *    VLAN_TAG_MODE_ORIGINAL
 *    VLAN_TAG_MODE_KEEP_FORMAT
 *    VLAN_TAG_MODE_REAL_KEEP_FORMAT
 */
extern rtk_api_ret_t rtk_svlan_tagMode_set(rtk_port_t port, rtk_vlan_tagMode_t tag_mode);

/* Function Name:
 *      rtk_svlan_tagMode_get
 * Description:
 *      Get SVLAN egress tag mode
 * Input:
 *      port                - Port id
 *      pTag_mode      - Egress tag mode
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI 
 *      RT_ERR_INPUT
 * Note:
 *    The API is used for setting port vlan egress tag mode
 *    The tag mode  as following:
 *    VLAN_TAG_MODE_ORIGINAL
 *    VLAN_TAG_MODE_KEEP_FORMAT
 *    VLAN_TAG_MODE_REAL_KEEP_FORMAT
 */
extern rtk_api_ret_t rtk_svlan_tagMode_get(rtk_port_t port, rtk_vlan_tagMode_t *pTag_mode);

/*following APIs are implemented in RTL8370S, while not implemented in RTL8316D*/
extern int32 rtk_svlan_priorityRef_set(rtk_svlan_pri_ref_t ref);
extern int32 rtk_svlan_priorityRef_get(rtk_svlan_pri_ref_t *pRef);
extern int32 rtk_svlan_memberPortEntry_set(uint32 svid_idx, rtk_svlan_memberCfg_t *psvlan_cfg);
extern int32 rtk_svlan_memberPortEntry_get(uint32 svid_idx, rtk_svlan_memberCfg_t *pSvlan_cfg);
extern int32 rtk_svlan_defaultSvlan_set(rtk_vlan_t svid);
extern int32 rtk_svlan_defaultSvlan_get(rtk_vlan_t *pSvid);
extern int32 rtk_svlan_c2s_add(rtk_vlan_t vid, rtk_vlan_t svid, rtk_portmask_t c2s_portmask);
extern int32 rtk_svlan_c2s_del(rtk_vlan_t vid, rtk_vlan_t svid);
extern int32 rtk_svlan_c2s_get(rtk_vlan_t vid, rtk_vlan_t svid, rtk_portmask_t *pC2s_portmask);
extern int32 rtk_svlan_ipmc2s_add(ipaddr_t ipmc, rtk_vlan_t svid);
extern int32 rtk_svlan_ipmc2s_del(ipaddr_t ipmc);
extern int32 rtk_svlan_ipmc2s_get(ipaddr_t ipmc, rtk_vlan_t *pSvid);
extern int32 rtk_svlan_l2mc2s_add(rtk_vlan_t svid, rtk_mac_t mac);
extern int32 rtk_svlan_l2mc2s_del(rtk_mac_t mac);
extern int32 rtk_svlan_l2mc2s_get(rtk_mac_t mac, rtk_vlan_t *pSvid);
extern int32 rtk_svlan_sp2c_add(rtk_vlan_t svid, rtk_port_t dst_port, rtk_vlan_t cvid);
extern int32 rtk_svlan_sp2c_get(rtk_vlan_t svid, rtk_port_t dst_port, rtk_vlan_t *pCvid); 
extern int32 rtk_svlan_sp2c_del(rtk_vlan_t svid, rtk_port_t dst_port);   

#endif


