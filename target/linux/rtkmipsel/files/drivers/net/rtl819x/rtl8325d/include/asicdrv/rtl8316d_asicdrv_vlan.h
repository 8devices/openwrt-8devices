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
* Purpose : ASIC-level driver header for VLAN.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/

#ifndef __RTL8316D_ASICDRV_VLAN_H__
#define __RTL8316D_ASICDRV_VLAH_H__


/* CVLAN */

/* Function Name:
 *      rtk_vlan_init
 * Description:
 *      Initialize VLAN
 * Input:
 *      void
 * Output:
 *      void
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 * Note:
 *     VLAN is disabled by default. User has to call this API to enable VLAN before
 *     using it. And It will set a default VLAN(vid 1) including all ports and set 
 *     all ports PVID to the default VLAN.
 */
extern rtk_api_ret_t rtk_vlan_init(void);

/* Function Name:
 *      rtk_vlan_set
 * Description:
 *      Set a VLAN entry
 * Input:
 *      vid          - VLAN ID to configure
 *      mbrmsk    - VLAN member set portmask
 *      untagmsk  - VLAN untag set portmask
 *      fid           -  filtering database id
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_VLAN_VID
 *      RT_ERR_VLAN_PORT_MBR_EXIST
 *      RT_ERR_L2_FID
 *      RT_ERR_TBL_FULL
 * Note:
 *     There are 4K VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 *     FID is for SVL/IVL usage, and the range is 0~4095.
 *     Because SVLAN and CVLAN share the same vlan table, so SVLAN also use this API to set vlan information. 
 */
extern rtk_api_ret_t rtk_vlan_set(rtk_vlan_t vid, rtk_portmask_t mbrmsk, rtk_portmask_t untagmsk, rtk_fid_t fid);

/* Function Name:
 *      rtk_vlan_get
 * Description:
 *      Get a VLAN entry
 * Input:
 *      vid             - VLAN ID to configure
 * Output:
 *      pMbrmsk     - VLAN member set portmask
 *      pUntagmsk  - VLAN untag set portmask
 *      pFid           -  filtering database id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_VLAN_VID
 *      RT_ERR_NULL_POINTER 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *     There are 4K VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 *     FID is for SVL/IVL usage, and the range is 0~4095.
 *     Because SVLAN and CVLAN share the same vlan table, so SVLAN also use this API to get vlan infomation
 */
extern rtk_api_ret_t rtk_vlan_get(rtk_vlan_t vid, rtk_portmask_t *pMbrmsk, rtk_portmask_t *pUntagmsk, rtk_fid_t *pFid);

/* Function Name:
 *      rtk_vlan_get
 * Description:
 *      Get a VLAN entry
 * Input:
 *      vid             - VLAN ID to configure
 * Output:
 *      pMbrmsk     - VLAN member set portmask
 *      pUntagmsk  - VLAN untag set portmask
 *      pFid           -  filtering database id
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_VLAN_VID
 *      RT_ERR_NULL_POINTER 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *     There are 4K VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 *     FID is for SVL/IVL usage, and the range is 0~4095.
 */
extern rtk_api_ret_t rtk_vlan_destroy(rtk_vlan_t vid);

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_set
 * Description:
 *      Set VLAN ingress for each port
 * Input:
 *      port             - Port id
 *      igr_filter        - VLAN ingress function enable status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_INPUT
 *      RT_ERR_ENABLE 
 * Note:
 *   While VLAN function is enabled, ASIC will decide VLAN ID for each received frame and get belonged member
 *   ports from VLAN table. If received port is not belonged to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
 */
extern rtk_api_ret_t rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igr_filter);

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_get
 * Description:
 *      Get VLAN ingress for each port
 * Input:
 *      port               - Port id
 * Output:
 *      pIgr_filter        - VLAN ingress function enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI 
 *      RT_ERR_NULL_POINTER
 * Note:
 *   While VLAN function is enabled, ASIC will decide VLAN ID for each received frame and get belonged member
 *   ports from VLAN table. If received port is not belonged to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
 */
extern rtk_api_ret_t rtk_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgr_filter);

/* Function Name:
 *      rtk_vlan_portPvid_set
 * Description:
 *      Set port to specified VLAN ID(PVID)
 * Input:
 *      port             - Port id
 *      pvid             - Specified VLAN ID
 *      priority         - 802.1p priority for the PVID
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_VLAN_VID
 *      RT_ERR_VLAN_PRIORITY 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *    The API is used for Port-based VLAN. The untagged frame received from the
 *    port will be classified to the specified VLAN and assigned to the specified priority.
 */
extern rtk_api_ret_t rtk_vlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority);

/* Function Name:
 *      rtk_vlan_portPvid_get
 * Description:
 *      Get VLAN ID(PVID) on specified port
 * Input:
 *      port             - Port id
 *      pPvid            - Specified VLAN ID
 *      pPriority        - 802.1p priority for the PVID
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
extern rtk_api_ret_t rtk_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t *pPriority);

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_set
 * Description:
 *      Set VLAN ingress for each port
 * Input:
 *      port             - Port id
 *      igr_filter        - VLAN ingress function enable status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_INPUT
 *      RT_ERR_ENABLE 
 * Note:
 *   While VLAN function is enabled, ASIC will decide VLAN ID for each received frame and get belonged member
 *   ports from VLAN table. If received port is not belonged to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
 */
extern rtk_api_ret_t rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igr_filter);

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_get
 * Description:
 *      Get VLAN ingress for each port
 * Input:
 *      port             - Port id
 * Output:
 *      pIgr_filter        - VLAN ingress function enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_NULL_POINTER
 * Note:
 *   While VLAN function is enabled, ASIC will decide VLAN ID for each received frame and get belonged member
 *   ports from VLAN table. If received port is not belonged to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
 */
extern rtk_api_ret_t rtk_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgr_filter);

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_set
 * Description:
 *      Set VLAN support frame type
 * Input:
 *      port                                 - Port id
 *      accept_frame_type             - accept frame type
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_PORT_ID
 * Note:
 *    The API is used for checking 802.1Q tagged frames.
 *    The accept frame type as following:
 *    ACCEPT_FRAME_TYPE_ALL
 *    ACCEPT_FRAME_TYPE_TAG_ONLY
 *    ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
extern rtk_api_ret_t rtk_vlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t accept_frame_type);

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN support frame type
 * Input:
 *      port                                 - Port id
 *      accept_frame_type             - accept frame type
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_PORT_ID
 * Note:
 *    The API is used for checking 802.1Q tagged frames.
 *    The accept frame type as following:
 *    ACCEPT_FRAME_TYPE_ALL
 *    ACCEPT_FRAME_TYPE_TAG_ONLY
 *    ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
extern rtk_api_ret_t rtk_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type);

/* Function Name:
 *      rtk_vlan_tagMode_set
 * Description:
 *      Set VLAN egress tag mode
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
extern rtk_api_ret_t rtk_vlan_tagMode_set(rtk_port_t port, rtk_vlan_tagMode_t tag_mode);

/* Function Name:
 *      rtk_vlan_tagMode_get
 * Description:
 *      Get VLAN egress tag mode
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
extern rtk_api_ret_t rtk_vlan_tagMode_get(rtk_port_t port, rtk_vlan_tagMode_t *pTag_mode);

/* Function Name:
 *      rtk_vlan_stg_set
 * Description:
 *      Set spanning tree group instance of the vlan to the specified device
 * Input:
 *      vid                -  specified VLAN ID
 *      stg                -  spanning tree group instance
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI 
 *      RT_ERR_INPUT
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *     The API can set spanning tree group instance of the vlan to the specified device.
 *     The spanning tree group number is from 0 to 31.  
 */
extern rtk_api_ret_t rtk_vlan_stg_set(rtk_vlan_t vid, rtk_stg_t stg);

/* Function Name:
 *      rtk_vlan_stg_get
 * Description:
 *      Set spanning tree group instance of the vlan to the specified device
 * Input:
 *      vid                -  specified VLAN ID
 * Output:
 *      pStg             -   spanning tree group instance
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_SMI 
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *     The API can set spanning tree group instance of the vlan to the specified device.
 *     The spanning tree group number is from 0 to 31.  
 */
extern rtk_api_ret_t rtk_vlan_stg_get(rtk_vlan_t vid, rtk_stg_t *pStg);

/* Function Name:
 *      rtk_vlan_tagMode_get
 * Description:
 *      Get VLAN egress tag mode
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
extern rtk_api_ret_t rtk_leaky_vlan_set(rtk_leaky_type_t type, rtk_enable_t enable);

/* Function Name:
 *      rtk_leaky_vlan_get
 * Description:
 *      Get VLAN leaky setting
 * Input:
 *      type                - Packet type for VLAN leaky
 * Output:
 *      rtk_enable_t     - Leaky status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI 
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER 
 * Note:
 *    This API can get VLAN leaky setting
 *    The leaky frame types are as following:
 *    LEAKY_STATIC_LUT.
 *    LEAKY_RRCP,
 *    LEAKY_RLDP.
 */
extern rtk_api_ret_t rtk_leaky_vlan_get(rtk_leaky_type_t type, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_set
 * Description:
 *      Set VLAN ingress for each port
 * Input:
 *      port         - Port id
 *      igr_filter    - VLAN ingress function enable status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 *      RT_ERR_ENABLE
 * Note:
 *    The status of vlan ingress filter is as following:
 *      DISABLED
 *      ENABLED
 *   While VLAN function is enabled, ASIC will decide VLAN ID for each received frame and get belonged member
 *   ports from VLAN table. If received port is not belonged to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
 */
extern rtk_api_ret_t rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igr_filter);

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_get
 * Description:
 *      Get VLAN Ingress Filter
 * Input:
 *      port            - Port id
 * Output:
 *      pIgr_filter    -  the pointer of VLAN ingress function enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID
 *      RT_ERR_NULL_POINTER
 * Note:
 *     The API can Get the VLAN ingress filter status.
 *     The status of vlan ingress filter is as following:
 *     DISABLED
 *     ENABLED   
 */

extern rtk_api_ret_t rtk_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgr_filter);


#endif /*__RTL8316D_ASICDRV_VLAH_H__*/

