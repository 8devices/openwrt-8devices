/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision$
 * $Date$
 *
 * Purpose : Definition function prototype of RTK API.
 *
 * Feature : Function prototype definition
 *
 */

#ifndef __RTK_API_H__
#define __RTK_API_H__

/*
 * Include Files
 */
#include <rtk_types.h>
#include <rtk_error.h>

#include <port.h>
#include <qos.h>
#include <acl.h>
#include <storm.h>
#include <cpu.h>
#include <dot1x.h>
#include <eee.h>
#include <igmp.h>
#include <interrupt.h>
#include <l2.h>
#include <leaky.h>
#include <led.h>
#include <mirror.h>
#include <oam.h>
#include <ptp.h>
#include <rate.h>
#include <rldp.h>
#include <stat.h>
#include <trap.h>
#include <trunk.h>
#include <vlan.h>
#include <oam.h>
/*
 * Data Type Declaration
 */
#define ENABLE                                      1
#define DISABLE                                     0

#define PHY_CONTROL_REG                             0
#define PHY_STATUS_REG                              1
#define PHY_AN_ADVERTISEMENT_REG                    4
#define PHY_AN_LINKPARTNER_REG                      5
#define PHY_1000_BASET_CONTROL_REG                  9
#define PHY_1000_BASET_STATUS_REG                   10
//#define PHY_RESOLVED_REG                            17

#ifndef CONFIG_RTL_CUSTOM_PASSTHRU
#define CONFIG_RTL_CUSTOM_PASSTHRU 1
#endif

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
#define PASSTHRU_VLAN_ID 100
#endif

typedef enum rtk_switch_maxPktLen_e
{
    MAXPKTLEN_1522B = 0,
    MAXPKTLEN_1536B,
    MAXPKTLEN_1552B,
    MAXPKTLEN_16000B,
    MAXPKTLEN_END
} rtk_switch_maxPktLen_t;

#define rtl83xx_setAsicPHYReg(phyNo, phyAddr, phyData)	rtl8367c_setAsicPHYReg(phyNo, phyAddr, phyData)
#define rtl83xx_setAsicLutFlushMode(mode)	rtl8367c_setAsicLutFlushMode(mode)
#define rtl83xx_setAsicLutForceFlush(portmask)	rtl8367c_setAsicLutForceFlush(portmask)
#define rtl83xx_setAsicLutFlushVid(vid)	rtl8367c_setAsicLutFlushVid(vid)
#define rtl83xx_setAsicReg(reg, value)	rtl8367c_setAsicReg(reg, value)
#define rtl83xx_setAsicRegBit(reg, bit, value)	rtl8367c_setAsicRegBit(reg, bit, value)
#define rtl83xx_setAsicPriorityDecision(index, prisrc, decisionPri)	rtl8367c_setAsicPriorityDecision(index, prisrc, decisionPri)
#define rtl83xx_setAsicRegBits(reg, bits, value)	rtl8367c_setAsicRegBits(reg, bits, value)	
#define rtl83xx_setAsicLutLearnLimitNo(port, number)	rtl8367c_setAsicLutLearnLimitNo(port, number)
#define rtl83xx_getAsicPHYReg(phyNo, phyAddr, pRegData )	rtl8367c_getAsicPHYReg(phyNo, phyAddr, pRegData )

#define RTL83XX_REG_CHIP_RESET	RTL8367C_REG_CHIP_RESET
#define RTL83XX_CHIP_RST_OFFSET	RTL8367C_CHIP_RST_OFFSET
#define RTL83XX_REG_LED_CONFIGURATION	RTL8367C_REG_LED_CONFIGURATION
#define RTL83XX_REG_UNDA_FLOODING_PMSK	RTL8367C_REG_UNDA_FLOODING_PMSK
#define RTL83XX_REG_UNMCAST_FLOADING_PMSK	RTL8367C_REG_UNMCAST_FLOADING_PMSK
#define RTL83XX_REG_BCAST_FLOADING_PMSK	RTL8367C_REG_BCAST_FLOADING_PMSK
#define RTL83XX_VIDMAX	RTL8367C_VIDMAX
#define RTL83XX_PORT4_ENABLE_OFFSET	RTL8367C_PORT4_ENABLE_OFFSET
#define RTL83XX_REG_MAX_LENGTH_LIMINT_IPG	RTL8367C_REG_MAX_LENGTH_LIMINT_IPG
#define RTL83XX_MAX_LENTH_CTRL_MASK	RTL8367C_MAX_LENTH_CTRL_MASK
#define RTL83XX_QOS_RATE_INPUT_MAX	RTL8367C_QOS_RATE_INPUT_MAX
#define RTL83XX_REG_CPU_PORT_MASK	RTL8367C_REG_CPU_PORT_MASK
#define RTL83XX_REG_CPU_CTRL	RTL8367C_REG_CPU_CTRL
#define RTL83XX_CPU_TRAP_PORT_OFFSET	RTL8367C_CPU_TRAP_PORT_OFFSET
#define RTL83XX_METERNO	RTL8367C_METERNO

#endif /* __RTK_API_H__ */

