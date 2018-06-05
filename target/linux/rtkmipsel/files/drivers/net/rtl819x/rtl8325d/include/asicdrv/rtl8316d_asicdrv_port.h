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
* Purpose :  Port property ASIC driver related Macro, structure, enumeration, 
*                variable types declaration and definition.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/

#ifndef __RTL8316D_ASICDRV_PORT_H__
#define __RTL8316D_ASICDRV_PORT_H__


/* Function Name:
 *      rtk_port_phyStatus_get
 * Description:
 *      Get ethernet PHY linking status
 * Input:
 *      port - Port id.
 * Output:
 *      linkStatus  - PHY link status
 *      speed       - PHY link speed
 *      duplex      - PHY duplex mode
 * Return:
 *      RT_ERR_OK                               OK
 *      RT_ERR_FAILED                         Failed
 *      RT_ERR_SMI                             SMI access error
 *      RT_ERR_PORT_ID                      Invalid port number.
 *      RT_ERR_PHY_REG_ID                 Invalid PHY address
 *      RT_ERR_INPUT                          Invalid input parameters.
 *      RT_ERR_BUSYWAIT_TIMEOUT     PHY access busy
 * Note:
 *      API will return auto negotiation status of phy.
 */
extern rtk_api_ret_t rtk_port_phyStatus_get(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus, rtk_port_speed_t *pSpeed, rtk_port_duplex_t *pDuplex);

/* Function Name:
 *      rtk_port_adminEnable_set
 * Description:
 *      Set port admin configuration of the specific port.
 * Input:
 *      port    - port id.
 *      enable  - Back pressure status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can set port admin configuration of the specific port.
 *      The port admin configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern rtk_api_ret_t rtk_port_adminEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_set
 * Description:
 *      Set ethernet PHY auto-negotiation desired ability.
 * Input:
 *      port  - the physical port where packet is to be transmitted
 * Output:
 *      pAbility - Ability structure
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID - Invalid port number.   
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_INPUT - group should be 0 or 1
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 *      RT_ERR_PHY_REG_ID - Invalid PHY address
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      If Full_1000 bit is set to 1, the AutoNegotiation will be automatic set to 1. While both AutoNegotiation and Full_1000 are set to 0, the PHY speed and duplex selection will
 *      be set as following 100F > 100H > 10F > 10H priority sequence.
 */
extern rtk_api_ret_t rtk_port_phyAutoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_get
 * Description:
 *      Get ethernet PHY auto-negotiation desired ability.
 * Input:
 *      port  - the physical port where packet is to be transmitted
 * Output:
 *      pAbility - Ability structure
 * Return:
 *      RT_ERR_OK - Success.
 *      RT_ERR_FAILED   - 
 *      RT_ERR_PORT_ID - Invalid port number.   
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_INPUT - group should be 0 or 1
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 *      RT_ERR_PHY_REG_ID - Invalid PHY address
 *      RT_ERR_BUSYWAIT_TIMEOUT - PHY access busy
 * Note:
 *      Get the capablity of specified PHY.
 */
extern rtk_api_ret_t rtk_port_phyAutoNegoAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      rtk_port_phyForceModeAbility_set
 * Description:
 *      Set the port PHY force mode, config its speed/duplex mode/pause/asy_pause 
 * Input:
 *      port       -  Port id
 *      pAbility   -  Ability structure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_PHY_REG_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_BUSYWAIT_TIMEOUT
 *     
 * Note:
 *      In force mode,  phy autoNegotiation ability must be disabled.
 *      If Full_1000 bit is set to 1, the AutoNegotiation will be automatic set to 1.
 *      so it could not use this API to force phy mode as Full_1000.
 */
extern rtk_api_ret_t rtk_port_phyForceModeAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      rtk_port_phyForceModeAbility_get
 * Description:
 *      Get the port PHY speed/duplex mode/pause/asy_pause in force mode
 * Input:
 *      port       -  Port id
 * Output:
 *      pAbility   -  Ability structure
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_PHY_REG_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_BUSYWAIT_TIMEOUT
 *     
 * Note:
 *      In force mode, phy autoNegotiation ability must be disabled.
 */
extern rtk_api_ret_t rtk_port_phyForceModeAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      rtk_port_backpressureEnable_set
 * Description:
 *      Set the half duplex backpressure enable status of the specific port.
 * Input:
 *      port    - port id.
 *      enable  - Back pressure status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can set the half duplex backpressure enable status of the specific port.
 *      The half duplex backpressure enable status of the port is as following:
 *      - DISABLE(Defer)
 *      - ENABLE (Backpressure)
 */
extern rtk_api_ret_t rtk_port_backpressureEnable_set(rtk_port_t port, rtk_enable_t enable);

#endif

