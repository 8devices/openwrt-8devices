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
* Purpose :  QoS ASIC driver related Macro, structure, enumeration, 
*                variable types declaration and definition.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/
#ifndef __RTL8316D_ASICDRV_QOS_H__
#define __RTL8316D_ASICDRV_QOS_H__

#define RTL8316D_STORM_RATE_INPUT_MAX         0xFFFFF
#define RTL8316D_STORM_RATE_INPUT_MIN         1

#define RTL8316D_QOS_RATE_INPUT_MAX     (0xFFFFUL * 16)      /*kbps*/
#define RTL8316D_QOS_RATE_INPUT_MIN     (16)                    /*kbps*/

// storm control
#define STRM_CTRL_BLOCK         0               
#define STRM_CTRL_OFF           (0x1FFFF * 8)   

#define RTL8316D_PRI_MAX        7
#define RTL8316D_QIDMAX         4

/*to be implemented*/
extern int32 rtk_port_phyTestMode_set(rtk_port_t port, rtk_port_phy_test_mode_t mode);
extern int32 rtk_port_phyTestMode_get(rtk_port_t port, rtk_port_phy_test_mode_t *pMode);
extern int32 rtk_port_phy1000BaseTMasterSlave_set(rtk_port_t port, rtk_enable_t enabled, rtk_enable_t masterslave);
extern int32 rtk_port_macForceLink_set(rtk_port_t port, rtk_port_mac_ability_t *pPortability);
extern int32 rtk_port_macForceLink_get(rtk_port_t port, rtk_port_mac_ability_t *pPortability);
extern int32 rtk_port_macStatus_get(rtk_port_t port, rtk_port_mac_ability_t *pPortstatus);
extern int32 rtk_port_phyReg_set(rtk_port_t port, rtk_port_phy_reg_t reg, rtk_port_phy_data_t data);
extern int32 rtk_port_phyReg_get(rtk_port_t port, rtk_port_phy_reg_t reg, rtk_port_phy_data_t *pData); 
extern int32 rtk_port_backpressureEnable_get(rtk_port_t port, rtk_enable_t *pEnable);
extern int32 rtk_port_adminEnable_get(rtk_port_t port, rtk_enable_t *pEnable);//ask Aaron
extern int32 rtk_port_isolation_set(rtk_port_t port, rtk_portmask_t portmask);
extern int32 rtk_port_isolation_get(rtk_port_t port, rtk_portmask_t *pPortmask);

#endif/*__RTL8316D_ASICDRV_QOS_H__*/

