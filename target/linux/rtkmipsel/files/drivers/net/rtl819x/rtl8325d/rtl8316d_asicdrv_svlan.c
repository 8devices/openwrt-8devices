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
* Purpose : ASIC-level driver header for SVLAN.
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
#include <rtl8316d_table_struct.h>
//#include <rtl8316d_asicdrv_tbl.h>
#include <rtl8316d_asicdrv_svlan.h>


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
rtk_api_ret_t rtk_svlan_init(void)
{
    rtk_api_ret_t retVal;
    rtk_port_t port;

    /*set default SVLAN SPID 0x88a8*/
    if ((retVal = rtl8316d_vlan_pidEntry_set(1, 0x88a8)) != RT_ERR_OK)
        return retVal;

    /*set all port as cvlan port, forward base cvlan*/
    for ( port = 0 ; port < RTL8316D_MAX_PORT; port ++ )
    {
        if ((retVal = rtl8316d_vlan_portRole_set(port, RTL8316D_PORTROLE_C)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8316d_svlan_portSpid_set(port, 0x0)) != RT_ERR_OK )
            return retVal;

        if ((retVal = rtl8316d_svlan_fwdBaseOvid_set(port, FALSE)) != RT_ERR_OK)
            return retVal;
    }
    
    return RT_ERR_OK;
}


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
rtk_api_ret_t rtk_svlan_servicePort_add(rtk_port_t port)
{

    rtk_api_ret_t retVal;

    /*set port as Service Provider Port, SPID is PID1, forward base SVID, */
    if ((retVal = rtl8316d_vlan_portRole_set(port, RTL8316D_PORTROLE_S)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8316d_svlan_portSpid_set(port, 0x2)) != RT_ERR_OK )
        return retVal;

    if ((retVal = rtl8316d_svlan_fwdBaseOvid_set(port, TRUE)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8316d_vlan_ingFilterEnable_set(port, TRUE)) != RT_ERR_OK)
        return retVal;

    if ((retVal =  rtl8316d_svlan_tagAware_set(port, TRUE) ) != RT_ERR_OK) 
        return retVal;
    
    return RT_ERR_OK;
}


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
rtk_api_ret_t rtk_svlan_servicePort_del(rtk_port_t port)
{
    rtk_api_ret_t retVal;

    /*set port as Custermer Port, forward base SVID, */
    if ((retVal = rtl8316d_vlan_portRole_set(port, RTL8316D_PORTROLE_C)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8316d_svlan_portSpid_set(port, 0x0)) != RT_ERR_OK )
        return retVal;

    if ((retVal = rtl8316d_svlan_fwdBaseOvid_set(port, FALSE)) != RT_ERR_OK)
        return retVal;

    if ((retVal =  rtl8316d_svlan_tagAware_set(port, FALSE) ) != RT_ERR_OK) 
        return retVal;
        
    return RT_ERR_OK;
}

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
rtk_api_ret_t rtk_svlan_servicePort_get(rtk_portmask_t *pSvlan_portmask)
{
    rtk_port_t port;    
    rtk_api_ret_t retVal;
    rtl8316d_vlan_portRole_s port_role;

    if (NULL == pSvlan_portmask)
        return RT_ERR_NULL_POINTER; //retVal;

    for (port = 0 ; port < RTL8316D_MAX_PORT; port ++)
    {
        if ((retVal = rtl8316d_vlan_portRole_get(port, &port_role)) != RT_ERR_OK)
            return retVal;
        if ( RTL8316D_PORTROLE_S == port_role)
            pSvlan_portmask->bits[0] |=   (1 << (uint32)port);        
    }
        
    return RT_ERR_OK;
}

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
rtk_api_ret_t rtk_svlan_tpidEntry_set(rtk_svlan_tpid_t svlan_tag_id)
{
    rtk_api_ret_t retVal;    

    if ((retVal = rtl8316d_vlan_pidEntry_set(1, (uint32)svlan_tag_id)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t rtk_svlan_tpidEntry_get(rtk_svlan_tpid_t *pSvlan_tag_id)
{

    rtk_api_ret_t retVal;    

    if ((retVal = rtl8316d_vlan_pidEntry_get(1, (uint32 *)pSvlan_tag_id)) !=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


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
rtk_api_ret_t rtk_svlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority, rtk_dei_t dei)
{
    rtk_api_ret_t retVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;
    
    /* vid must be 0~4095 */
    if(pvid > RTL8316D_VIDMAX)
        return RT_ERR_VLAN_VID;

    /* priority must be 0~7 */
    if(priority > RTL8316D_PRIMAX)
        return RT_ERR_VLAN_PRIORITY;

    retVal = rtl8316d_svlan_portPvid_set(port, pvid, priority, dei);
        
    return retVal;
}

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
rtk_api_ret_t rtk_svlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t* pPriority, rtk_dei_t *pDei)
{
    rtk_api_ret_t retVal;

    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;
    
    if ((NULL == pPvid) || (NULL == pPriority) || (NULL == pDei))
        return RT_ERR_NULL_POINTER;

    retVal = rtl8316d_svlan_portPvid_get(port, pPvid, pPriority, pDei);
        
    return retVal;
}

/* Function Name:
 *      rtk_svlan_portAcceptFrameType_set
 * Description:
 *      Set Service VLAN support frame type
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
 *    The API is used for checking Service VLAN tagged frames.
 *    The accept frame type as following:
 *    ACCEPT_FRAME_TYPE_ALL
 *    ACCEPT_FRAME_TYPE_TAG_ONLY
 *    ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
rtk_api_ret_t rtk_svlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t accept_frame_type)
{
    rtk_api_ret_t retVal;

    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    retVal = rtl8316d_svlan_portAcceptFrameType_set(port, accept_frame_type);

    return retVal;
}

/* Function Name:
 *      rtk_svlan_portAcceptFrameType_get
 * Description:
 *      Get Service VLAN support frame type
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
 *    The API is used for checking svlan tagged frames.
 *    The accept frame type as following:
 *    ACCEPT_FRAME_TYPE_ALL
 *    ACCEPT_FRAME_TYPE_TAG_ONLY
 *    ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
rtk_api_ret_t rtk_svlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type)
{
    uint32 retVal;

    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (pAccept_frame_type == NULL)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8316d_svlan_portAcceptFrameType_get(port, pAccept_frame_type);

    return retVal;
}    


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
rtk_api_ret_t rtk_svlan_tagMode_set(rtk_port_t port, rtk_vlan_tagMode_t tag_mode)
{
    rtk_api_ret_t retVal;
    rtl8316d_vlan_tagMode_s mode;
    
    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;
    
    if (tag_mode > VLAN_TAG_MODE_END)
        return RT_ERR_INPUT;

    switch(tag_mode)
    {
        case VLAN_TAG_MODE_ORIGINAL:
            mode = RTL8316D_VLAN_TAG_ORIGINAL;
            //printf("Original mode!\n");
            break;
        case VLAN_TAG_MODE_KEEP_FORMAT:
            mode = RTL8316D_VLAN_TAG_KEEP;
            //printf("Keep format mode!\n");
            break;
        case VLAN_TAG_MODE_REAL_KEEP_FORMAT:
            mode = RTL8316D_VLAN_TAG_REAL_KEEP;
            break;
        case VLAN_TAG_MODE_PRI:
            //printf("Unimplemented mode!\n");
            return RT_ERR_INPUT;
            break;
        default:
            return RT_ERR_INPUT;
    }
    
    retVal = rtl8316d_svlan_tagMode_set(port, mode, mode);

    return retVal;
}


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
rtk_api_ret_t rtk_svlan_tagMode_get(rtk_port_t port, rtk_vlan_tagMode_t *pTag_mode)
{
    rtk_api_ret_t retVal;
    rtl8316d_vlan_tagMode_s rxc_mode, rxs_mode;
    
    if( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;
    
    if ( NULL == pTag_mode )
        return RT_ERR_NULL_POINTER;

    retVal = rtl8316d_svlan_tagMode_get(port, &rxc_mode, &rxs_mode);
 
    switch(rxs_mode)
    {
        case RTL8316D_VLAN_TAG_ORIGINAL:
            *pTag_mode = VLAN_TAG_MODE_ORIGINAL;
            //printf("Original mode!\n");
            break;
        case RTL8316D_VLAN_TAG_KEEP:
            *pTag_mode = VLAN_TAG_MODE_KEEP_FORMAT;
            //printf("Keep format mode!\n");
            break;
        case RTL8316D_VLAN_TAG_REAL_KEEP:
            *pTag_mode = VLAN_TAG_MODE_REAL_KEEP_FORMAT;
            break;
        default:
            *pTag_mode = VLAN_TAG_MODE_END;
            //printf("unknown mode!\n");
            return RT_ERR_FAILED;
    }

    return retVal;
}



