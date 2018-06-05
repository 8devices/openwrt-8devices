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
* Purpose : ASIC-level driver implementation for VLAN.
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
#include <rtl8316d_asicdrv_tbl.h>
#include <rtl8316d_asicdrv_vlan.h>

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
rtk_api_ret_t rtk_vlan_init(void)
{       
    rtl8316d_tbl_vlanParam_t vlanEntry;
    rtk_api_ret_t retVal;
    uint16 index;
    uint16 port;

    /*set default vlan vid = 1*/
    vlanEntry.vid = 1;
    vlanEntry.member = RTL8316D_MAX_PORTMASK;
    vlanEntry.untag = RTL8316D_MAX_PORTMASK;
    vlanEntry.valid = 1;
    vlanEntry.fid = 0;
    vlanEntry.ucslkfid = 1;
    vlanEntry.vbfwd = 0;
    retVal = rtl8316d_vlan_setAsicEntry(4, &vlanEntry);
    if (RT_ERR_OK != retVal)
        return retVal;                       

    memset(&vlanEntry, 0, sizeof(vlanEntry));   
    for (index = 5; index < 8; index ++ )
    {
        retVal = rtl8316d_vlan_setAsicEntry(index, &vlanEntry);
        if (RT_ERR_OK != retVal)
            return retVal;                                        
    }

    /*enable all port tag aware*/
    for (port =0 ; port < RTL8316D_MAX_PORT; port ++)
    {
        retVal =  rtl8316d_vlan_tagAware_set(port, TRUE);
        if (RT_ERR_OK != retVal)
            return retVal;
    }
       
    return RT_ERR_OK;
}


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

rtk_api_ret_t rtk_vlan_set(rtk_vlan_t vid, rtk_portmask_t mbrmsk, rtk_portmask_t untagmsk, rtk_fid_t fid)
{
    rtl8316d_tbl_vlanParam_t vlanEntry;
    uint16 index, hit_index;
    uint16 hit_flag;                                /*0:not hit  1: hit vlan table, 2: hit vlan cam table */
    uint16 invalid_index;
    uint16 cam_invalid_index;   
    rtk_api_ret_t retVal;
    rtk_fid_t fid1;

    //printf("VLAN Entry Set:\n");
    //printf("vid: %ld, mbrmsk: %08lx, untagmask: %08lx, fid: %ld\n", vid, mbrmsk, untagmsk, fid);

    fid1 = fid;
    
    /* vid must be 0~4095 */
    if (vid > RTL8316D_VIDMAX)
        return RT_ERR_VLAN_VID;

    if (mbrmsk.bits[0] > RTL8316D_MAX_PORTMASK)
        return RT_ERR_VLAN_PORT_MBR_EXIST;

    if (untagmsk.bits[0] > RTL8316D_MAX_PORTMASK)
        return RT_ERR_VLAN_PORT_MBR_EXIST;

    /* fid must be 0~31 */
    if (fid > RTL8316D_FIDMAX)
        return RT_ERR_L2_FID;

    /*search 512-entry vlan table and 16-entry vlan cam table to find the vid*/
    hit_flag = 0;
    hit_index = 0xffff;
    invalid_index = 0xffff;
    cam_invalid_index = 0xffff;  
    memset(&vlanEntry, 0, sizeof(vlanEntry));
    for (index = (vid & 0x7F) << 2; index < ((vid & 0x7F) << 2) + 4; index ++)
    {
        retVal = rtl8316d_vlan_getAsicEntry(index, &vlanEntry);

        //printf("Index: %d valid: %bd vid: %d\n", index, vlanEntry.valid, vlanEntry.vid);
        if (RT_ERR_OK != retVal)
            return retVal;

        if (!vlanEntry.valid)
            invalid_index = index;
        
        if (vlanEntry.valid && (vlanEntry.vid == vid))
        {
            hit_flag = 1;
            hit_index = index;
            break;
        }
    }
    
    if (!hit_flag)
    {
        for (index = 0; index < RTL8316D_VLANCAMMAX; index ++)
        {
            retVal = rtl8316d_vlan_getCamEntry(index, &vlanEntry);
            if (RT_ERR_OK != retVal)
                return retVal; 
            
            if (!vlanEntry.valid)
                cam_invalid_index = index;
            if (vlanEntry.valid && (vlanEntry.vid == vid))
            {
                hit_flag = 2;
                hit_index = index;
                break;
            }                                                           
        }        
    }

    memset(&vlanEntry, 0, sizeof(vlanEntry));
    vlanEntry.vid = (uint16)vid;
    vlanEntry.member = mbrmsk.bits[0];
    vlanEntry.untag = untagmsk.bits[0];
    vlanEntry.valid = 1;
    vlanEntry.fid = (uint16)fid1;
    vlanEntry.ucslkfid = 1;
    vlanEntry.vbfwd = 0;

    if (hit_flag == 1)
    {
        //printf("hit Index: %d\n", hit_index);
        retVal = rtl8316d_vlan_setAsicEntry(hit_index, &vlanEntry);
        if (RT_ERR_OK != retVal)
            return retVal;                 
    }
    else if (hit_flag == 2)
    {
        retVal = rtl8316d_vlan_setCamEntry(hit_index, &vlanEntry);
        if (RT_ERR_OK != retVal)
            return retVal;                 
    }
    else if (invalid_index != 0xffff)
    {
        //printf("hit Index: %d\n", invalid_index);
        retVal = rtl8316d_vlan_setAsicEntry(invalid_index, &vlanEntry);
        if (RT_ERR_OK != retVal)
            return retVal;                 
    }
    else if (cam_invalid_index != 0xffff)
    {
        //printf("cam Index: %d\n", cam_invalid_index);
        retVal = rtl8316d_vlan_setCamEntry(cam_invalid_index, &vlanEntry);
        if (RT_ERR_OK != retVal)
            return retVal;                         
    }
    else 
    {
        return RT_ERR_TBL_FULL;
    }
        
    return RT_ERR_OK;
}

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
 *     Because SVLAN and CVLAN share the same vlan table, so SVLAN also use this API to set vlan information.
 */
rtk_api_ret_t rtk_vlan_get(rtk_vlan_t vid, rtk_portmask_t *pMbrmsk, rtk_portmask_t *pUntagmsk, rtk_fid_t *pFid)
{
    rtl8316d_tbl_vlanParam_t vlanEntry;
    uint16 index, hit_index;
    uint16 hit_flag;        /*0:not hit  1: hit vlan table, 2: hit vlan cam table */
    rtk_api_ret_t retVal;
        
    /* vid must be 0~4095 */
    if(vid > RTL8316D_VIDMAX)
        return RT_ERR_VLAN_VID;

    if ((NULL == pMbrmsk) || (NULL == pUntagmsk) || (NULL == pFid))
        return RT_ERR_NULL_POINTER;

    /*search 512-entry vlan table and 16-entry vlan cam table to find the vid*/
    hit_flag = 0;
    hit_index = 0xffff;
    memset(&vlanEntry, 0, sizeof(vlanEntry));
    for (index = (vid & 0x7F) << 2; index < ((vid & 0x7F) << 2) + 4; index ++)
    {
        retVal = rtl8316d_vlan_getAsicEntry(index, &vlanEntry);
        if (RT_ERR_OK != retVal)
            return retVal;
        
        if (vlanEntry.valid && (vlanEntry.vid == vid))
        {
            hit_flag = 1;
            hit_index = index;            
            pMbrmsk->bits[0] = vlanEntry.member;
            pUntagmsk->bits[0] = vlanEntry.untag;
            *pFid = (rtk_fid_t)vlanEntry.fid;
            break;
        }
    }
    
    if (!hit_flag)
    {
        for (index = 0; index < RTL8316D_VLANCAMMAX; index ++)
        {
            retVal = rtl8316d_vlan_getCamEntry(index, &vlanEntry);
            if (RT_ERR_OK != retVal)
                return retVal; 
            
            if (vlanEntry.valid && (vlanEntry.vid == vid))
            {
                hit_flag = 2;
                hit_index = index;
                pMbrmsk->bits[0] = vlanEntry.member;
                pUntagmsk->bits[0] = vlanEntry.untag;
                *pFid = (rtk_fid_t)vlanEntry.fid;                
                break;
            }                                                           
        }        
    }

    if (!hit_flag)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;
            
    return RT_ERR_OK;

}

/* Function Name:
 *      rtk_vlan_destroy
 * Description:
 *      remove a VLAN entry
 * Input:
 *      vid             - VLAN ID to configure
 * Output:
 *      None
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
rtk_api_ret_t rtk_vlan_destroy(rtk_vlan_t vid)
{
    rtl8316d_tbl_vlanParam_t vlanEntry;
    uint16 index, hit_index;
    uint16 hit_flag;        /*0:not hit  1: hit vlan table, 2: hit vlan cam table */
    rtk_api_ret_t retVal;
        
    /* vid must be 0~4095 */
    if(vid > RTL8316D_VIDMAX)
        return RT_ERR_VLAN_VID;

    /*search 512-entry vlan table and 16-entry vlan cam table to find the vid*/
    hit_flag = 0;
    hit_index = 0xffff;
    memset(&vlanEntry, 0, sizeof(vlanEntry));
    for (index = (vid & 0x7F) << 2; index < ((vid & 0x7F) << 2) + 4; index ++)
    {
        retVal = rtl8316d_vlan_getAsicEntry(index, &vlanEntry);
        if (RT_ERR_OK != retVal)
            return retVal;
        
        if (vlanEntry.valid && (vlanEntry.vid == vid))
        {
            hit_flag = 1;
            hit_index = index;            
            memset(&vlanEntry, 0, sizeof(vlanEntry));            
            retVal = rtl8316d_vlan_setAsicEntry(hit_index, &vlanEntry);
            if (RT_ERR_OK != retVal)
                return retVal;                                        
            break;
        }
    }
    
    if (!hit_flag)
    {
        for (index = 0; index < RTL8316D_VLANCAMMAX; index ++)
        {
            retVal = rtl8316d_vlan_getCamEntry(index, &vlanEntry);
            if (RT_ERR_OK != retVal)
                return retVal; 
            
            if (vlanEntry.valid && (vlanEntry.vid == vid))
            {
                hit_flag = 2;
                hit_index = index;
                memset(&vlanEntry, 0, sizeof(vlanEntry)); 
                retVal = rtl8316d_vlan_setCamEntry(hit_index, &vlanEntry);
                if (RT_ERR_OK != retVal)
                    return retVal;                 
                break;
            }                                                           
        }        
    }

    if (!hit_flag)
        return RT_ERR_VLAN_ENTRY_NOT_FOUND;


    return RT_ERR_OK;
}

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
rtk_api_ret_t rtk_vlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority)
{
    rtk_api_ret_t retVal;

    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;
    
    /* vid must be 0~4095 */
    if (pvid > RTL8316D_VIDMAX)
        return RT_ERR_VLAN_VID;

    /* priority must be 0~7 */
    if (priority > RTL8316D_PRIMAX)
        return RT_ERR_VLAN_PRIORITY;

    retVal = rtl8316d_vlan_portPvid_set(port, pvid, priority);
    
    
    return retVal;
}

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
rtk_api_ret_t rtk_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t *pPriority)
{
    rtk_api_ret_t retVal;
    
    if (port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (pPvid == NULL)
        return RT_ERR_NULL_POINTER;

    if (pPriority == NULL) 
        return RT_ERR_NULL_POINTER;
    
    retVal = rtl8316d_vlan_portPvid_get(port, pPvid, pPriority);
    return retVal;
}

#if 0
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
rtk_api_ret_t rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igr_filter)
{
    rtk_api_ret_t retVal;

    if (igr_filter>=RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    retVal = rtl8316d_vlan_ingFilterEnable_set(port, (uint32)igr_filter);

    return retVal;
    
}

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
rtk_api_ret_t rtk_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgr_filter)
{
    rtk_api_ret_t retVal;
    uint32 enabled;
    
    if (NULL == pIgr_filter )
        return RT_ERR_NULL_POINTER;
    
    retVal = rtl8316d_vlan_ingFilterEnable_get(port, &enabled);
    *pIgr_filter = (rtk_enable_t) enabled;

    return retVal;
}
#endif    

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
rtk_api_ret_t rtk_vlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t accept_frame_type)
{
    rtk_api_ret_t retVal;

    if ( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    retVal = rtl8316d_vlan_portAcceptFrameType_set(port, accept_frame_type);

    return retVal;
}


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
rtk_api_ret_t rtk_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type)
{
    uint32 retVal;

    if ( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;

    if (pAccept_frame_type == NULL)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8316d_vlan_portAcceptFrameType_get(port, pAccept_frame_type);

    return retVal;
}    


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
rtk_api_ret_t rtk_vlan_tagMode_set(rtk_port_t port, rtk_vlan_tagMode_t tag_mode)
{
    rtk_api_ret_t retVal;
    rtl8316d_vlan_tagMode_s mode;
    
    if ( port >= RTL8316D_MAX_PORT)
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
    
    retVal = rtl8316d_vlan_tagMode_set(port, mode, mode);

    return retVal;
}


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
rtk_api_ret_t rtk_vlan_tagMode_get(rtk_port_t port, rtk_vlan_tagMode_t *pTag_mode)
{
    rtk_api_ret_t retVal;
    rtl8316d_vlan_tagMode_s rxc_mode, rxs_mode;
    
    if ( port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;
    
    if ( NULL == pTag_mode )
        return RT_ERR_NULL_POINTER;

    retVal = rtl8316d_vlan_tagMode_get(port, &rxc_mode, &rxs_mode);
    
    switch (rxc_mode)
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
 *      RT_ERR_INPUT
 *      RT_ERR_SMI
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *     The API can set spanning tree group instance of the vlan to the specified device.
 *     The spanning tree group number is from 0 to 31.  
 */
rtk_api_ret_t rtk_vlan_stg_set(rtk_vlan_t vid, rtk_stg_t stg)
{
    rtk_api_ret_t retVal;
    rtk_portmask_t mbrmsk;
    rtk_portmask_t untagmsk;
    rtk_fid_t fid;

    /* vid must be 0~4095 */
    if( vid > RTL8316D_VIDMAX)
        return RT_ERR_VLAN_VID;

    /* stg must be 0~31 */
    if( stg > RTL8316D_FIDMAX)
        return RT_ERR_MSTI;
    
    if ((retVal = rtk_vlan_get(vid, &mbrmsk, &untagmsk, &fid)) != RT_ERR_OK )
        return retVal;

    fid = (rtk_fid_t)stg;

    retVal = rtk_vlan_set(vid, mbrmsk, untagmsk, fid);
       
    return retVal;    
}

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
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *     The API can set spanning tree group instance of the vlan to the specified device.
 *     The spanning tree group number is from 0 to 31.  
 */
rtk_api_ret_t rtk_vlan_stg_get(rtk_vlan_t vid, rtk_stg_t *pStg)
{
    rtk_api_ret_t retVal;
    rtk_portmask_t mbrmsk;
    rtk_portmask_t untagmsk;
    rtk_fid_t fid;

    /* vid must be 0~4095 */
    if( vid > RTL8316D_VIDMAX)
        return RT_ERR_VLAN_VID;
    
    if ( NULL == pStg )
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtk_vlan_get(vid, &mbrmsk, &untagmsk, &fid)) != RT_ERR_OK )
        return retVal;
    
    *pStg = (rtk_stg_t) (fid);

    return RT_ERR_OK;
    
}



/* Function Name:
 *      rtk_leaky_vlan_set
 * Description:
 *      Set VLAN leaky
 * Input:
 *      type             - Packet type for VLAN leaky
 *      enable          - Leaky status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI  
 *      RT_ERR_INPUT
 *      RT_ERR_ENABLE 
 * Note:
 *    This API can set VLAN leaky.
 *    The leaky frame types are as following:
 *    LEAKY_STATIC_LUT.
 *    LEAKY_RRCP,
 *    LEAKY_RLDP.
 */
rtk_api_ret_t rtk_leaky_vlan_set(rtk_leaky_type_t type, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (type >= LEAKY_END)
        return RT_ERR_INPUT;

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;        
    
    switch (type)
    {
        case LEAKY_STATIC_LUT:
            retVal = reg_field_write(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_CONTROL, STATICVLKY, enable);
            break;
        case LEAKY_RRCP:
            retVal = reg_field_write(RTL8316D_UNIT, RRCP_GLOBAL_CONTROL, RRCPVLEAKY, enable);
            break;
        case LEAKY_RLDP:
            retVal = reg_field_write(RTL8316D_UNIT, RLDP_GLOBAL_CONTROL, RLDP_VLEAKY, enable);
            break;
        default:
            return RT_ERR_INPUT;
    }

    return retVal;
}


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
 rtk_api_ret_t rtk_leaky_vlan_get(rtk_leaky_type_t type, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    if (pEnable == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }
    
    if (type >= LEAKY_END)
        return RT_ERR_INPUT;    

    switch (type)
    {
        case LEAKY_STATIC_LUT:
            retVal = reg_field_read(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_CONTROL, STATICVLKY, pEnable);
            break;
        case LEAKY_RRCP:
            retVal = reg_field_read(RTL8316D_UNIT, RRCP_GLOBAL_CONTROL, RRCPVLEAKY, pEnable);
            break;
        case LEAKY_RLDP:
            retVal = reg_field_read(RTL8316D_UNIT, RLDP_GLOBAL_CONTROL, RLDP_VLEAKY, pEnable);
            break;
        default:
            return RT_ERR_INPUT;
    }
        
    return retVal;
}


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
rtk_api_ret_t rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igr_filter)
{
    rtk_api_ret_t retVal;

    if(port >= RTK_MAX_NUM_OF_PORT)
        return RT_ERR_PORT_ID;

    if(igr_filter>=RTK_ENABLE_END)
        return RT_ERR_ENABLE; 

    if((retVal = reg_field_write(RTL8316D_UNIT, PORT0_VLAN_INGRESS_FILTER_CONTROL+port, IGFILTER, igr_filter))!=RT_ERR_OK)
        return retVal; 


    return RT_ERR_OK;
}

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

rtk_api_ret_t rtk_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgr_filter)
{
    rtk_api_ret_t retVal;
    
    if(port >= RTK_MAX_NUM_OF_PORT)
        return RT_ERR_PORT_ID;

    if (pIgr_filter == NULL)
        return RT_ERR_NULL_POINTER;

    if((retVal=reg_field_read(RTL8316D_UNIT, PORT0_VLAN_INGRESS_FILTER_CONTROL+port, IGFILTER, pIgr_filter))!=RT_ERR_OK)
        return retVal; 

    return RT_ERR_OK;
}





