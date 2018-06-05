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
* Purpose : ASIC-level driver implementation for mirror.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/


#ifndef __RTL8316D_ASICDRV_MIR_H__
#define __RTL8316D_ASICDRV_MIR_H__

/* Function Name:
 *      rtk_mirror_portBased_set
 * Description:
 *      Set port mirror function
 * Input:
 *      pMirrored_rx_portmask   - Rx mirror port mask
 *      pMirrored_tx_portmask   - Tx mirror port mask
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_PORT_MASK
 * Note:
 *      The API is to set mirror function of  mirror port and Rx/Tx mirror ports
 *    
 */
extern rtk_api_ret_t rtk_mirror_portBased_set(rtk_port_t mirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask);

/* Function Name:
 *      rtk_mirror_portBased_get
 * Description:
 *      Get port mirror function
 * Input:
 *      none
 * Output:
 *      pMirrored_rx_portmask   - Rx mirror port mask
 *      pMirrored_tx_portmask   - Tx mirror port mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI 
 *      RT_ERR_NULL_POINTER
 * Note:
 *      The API is to get mirror function of  mirror port and Rx/Tx mirror ports
 *    
 */
extern rtk_api_ret_t rtk_mirror_portBased_get(rtk_port_t* pMirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask);

/* Function Name:
 *      rtk_mirror_portIso_set
 * Description:
 *      Set mirror port isolation
 * Input:
 *      enable   - Mirror isolation status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_ENABLE
 * Note:
 *    The API is to set mirror isolation function that prevent normal forwarding packets to miror port.
 *    
 */
extern rtk_api_ret_t rtk_mirror_portIso_set(rtk_enable_t enable);


/* Function Name:
 *      rtk_mirror_portIso_get
 * Description:
 *      Get mirror port isolation
 * Input:
 *      none
 * Output:
 *      pEnable   - Mirror isolation status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_ENABLE
 * Note:
 *    The API is to get mirror isolation status.
 *    
 */
extern rtk_api_ret_t rtk_mirror_portIso_get(rtk_enable_t *pEnable);


#endif /*__RTL8316D_ASICDRV_MIR_H__*/


