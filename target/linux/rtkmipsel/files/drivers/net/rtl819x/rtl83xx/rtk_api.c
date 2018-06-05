/*
 * Copyright (C) 2010 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 8252 $
 * $Date: 2010-01-29 14:04:02 +0800 (?Ÿæ?äº? 29 ä¸€??2010) $
 *
 * Purpose : RTK switch high-level API for RTL83XX
 * Feature : Here is a list of all functions and variables in this module.
 *
 * API source : Realtek_Unmanaged_Switch_API_V1.3.10_20170309
 */

#include <linux/version.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/seq_file.h>

#include <rtl8367c_asicdrv_acl.h>
#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_dot1x.h>
#include <rtl8367c_asicdrv_qos.h>
#include <rtl8367c_asicdrv_scheduling.h>
#include <rtl8367c_asicdrv_fc.h>
#include <rtl8367c_asicdrv_port.h>
#include <rtl8367c_asicdrv_phy.h>
#include <rtl8367c_asicdrv_igmp.h>
#include <rtl8367c_asicdrv_unknownMulticast.h>
#include <rtl8367c_asicdrv_rma.h>
#include <rtl8367c_asicdrv_vlan.h>
#include <rtl8367c_asicdrv_lut.h>
#include <rtl8367c_asicdrv_led.h>
#include <rtl8367c_asicdrv_svlan.h>
#include <rtl8367c_asicdrv_meter.h>
#include <rtl8367c_asicdrv_inbwctrl.h>
#include <rtl8367c_asicdrv_storm.h>
#include <rtl8367c_asicdrv_misc.h>
#include <rtl8367c_asicdrv_portIsolation.h>
#include <rtl8367c_asicdrv_cputag.h>
#include <rtl8367c_asicdrv_trunking.h>
#include <rtl8367c_asicdrv_mirror.h>
#include <rtl8367c_asicdrv_mib.h>
#include <rtl8367c_asicdrv_interrupt.h>
#include <rtl8367c_asicdrv_green.h>
#include <rtl8367c_asicdrv_eee.h>
#include <rtl8367c_asicdrv_eav.h>
#include <rtl8367c_asicdrv_hsb.h>
#include <rtl8367c_asicdrv_oam.h>

#include <rtk_api.h>
#include <rtk_error.h>

#ifdef __KERNEL__
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#define CONFIG_RTL_PROC_NEW	1
#endif
#endif

#ifdef CONFIG_OPENWRT_SDK
#include <linux/kernel.h> 
#include "../common/smi.h"
#endif

#include <rtk_switch.h>

#define DELAY_800MS_FOR_CHIP_STATABLE() {  }

rtk_uint16      (*init_para)[2];
rtk_uint16      init_size;
rtk_api_ret_t rtk_forceFull_init(void);
extern int smi_write(unsigned int mAddrs, unsigned int rData);
#define EEE_OCP_PHY_ADDR    (0xA5D0)

#define REG1301_8367R_VB				0x1010
#define REG1301_8367RB					0x1000

#define CONFIG_LAN_WAN_ISOLATION 1
#define CONFIG_RTK_REFINE_PORT_DUPLEX_MODE 1

#if defined(CONFIG_RTL_8197F)
	#if defined(CONFIG_RTL_8363NB_SUPPORT) || defined(CONFIG_RTL_8365MB_SUPPORT) || defined(CONFIG_RTL_8367SB_SUPPORT)
	#define CONFIG_RTL_ENABLE_EXT_SSC		1 	// for EMI
	#elif defined(CONFIG_RTL_8366SC_SUPPORT) || defined(CONFIG_RTL_8364NB_SUPPORT)
	#else // 8367RB
	#define CONFIG_RTL_8367R_SUPPORT		1
	#define CONFIG_RTL_ENABLE_EXT_SSC		1 	// for EMI	
	#endif
#endif

//#define RTL83XX_USE_ONE_LED_PER_PORT		1

#if defined(CONFIG_RTL_8366SC_SUPPORT) || defined(CONFIG_RTL_8364NB_SUPPORT)
#define EXT_PORT_FOR_HOST			EXT_PORT1
#define LED_GROUP_X					LED_GROUP_1

#elif defined(CONFIG_RTL_8363NB_SUPPORT) || defined(CONFIG_RTL_8365MB_SUPPORT) || defined(CONFIG_RTL_8367SB_SUPPORT)
#define EXT_PORT_FOR_HOST           EXT_PORT0
#define LED_GROUP_X                 LED_GROUP_1

#else // 8367RB, 8367R_VB
#define EXT_PORT_FOR_HOST			EXT_PORT0
#define EXT_PORT_FOR_RGMII2			EXT_PORT1
#define EXT_MAC_FOR_HOST			RTK_EXT_1_MAC
#define LED_GROUP_X					LED_GROUP_1
#endif

#ifndef CONFIG_RTL_8197F
#define ENABLE_8367RB_RGMII2	1
#endif

#if defined(CONFIG_RTL_8366SC_SUPPORT) || defined(CONFIG_RTL_8364NB_SUPPORT)
#define EXT1_RGMII_TX_DELAY		0
#define EXT1_RGMII_RX_DELAY		6
#else
#define EXT0_RGMII_TX_DELAY		0
#define EXT0_RGMII_RX_DELAY		2
#ifdef ENABLE_8367RB_RGMII2
#define EXT1_RGMII_TX_DELAY		1
#define EXT1_RGMII_RX_DELAY		3
#endif
#endif

#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8363NB_SUPPORT) || defined(CONFIG_RTL_8365MB_SUPPORT) || defined(CONFIG_RTL_8367SB_SUPPORT)
#define EXT0_RGMII_TX_DELAY_ENABLE_SSC		0
#define EXT0_RGMII_RX_DELAY_ENABLE_SSC		5
#elif defined(CONFIG_RTL_8366SC_SUPPORT) || defined(CONFIG_RTL_8364NB_SUPPORT)
#endif

rtk_uint32 vportmask;
rtk_uint32 vutpportmask;
unsigned long irq_flags;

/* New API do not have these*/

/* Function Name:
 *      rtk_l2_flushType_set
 * Description:
 *      Flush L2 mac address by type in the specified device.
 * Input:
 *      type - flush type
 *      vid - VLAN id
 *      portOrTid - port id or trunk id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This function trigger flushing of per-port L2 learning.
 *      When flushing operaton completes, the corresponding bit will be clear.
 *      The flush type as following:
 *      - FLUSH_TYPE_BY_PORT        (physical port)
 *      - FLUSH_TYPE_BY_PORT_VID    (physical port + VID)
 */
rtk_api_ret_t rtk_l2_flushType_set(rtk_l2_flushType_t type, rtk_vlan_t vid, rtk_l2_flushItem_t portOrTid)
{
    rtk_api_ret_t retVal;

    if (type>=FLUSH_TYPE_END)
        return RT_ERR_INPUT;

    if (portOrTid > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    switch (type)
    {
        case FLUSH_TYPE_BY_PORT:
            if ((retVal = rtl83xx_setAsicLutFlushMode(FLUSHMDOE_PORT)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl83xx_setAsicLutForceFlush(1<<portOrTid)) != RT_ERR_OK)
                return retVal;

            break;
        case FLUSH_TYPE_BY_PORT_VID:
            if ((retVal = rtl83xx_setAsicLutFlushMode(FLUSHMDOE_VID)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl83xx_setAsicLutFlushVid(vid)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtl83xx_setAsicLutForceFlush(1<<portOrTid)) != RT_ERR_OK)
                return retVal;

            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367b_setAsicMaxLengthInRx
 * Description:
 *      Set Max receiving packet length
 * Input:
 *      maxLength 	- 0: 1522 bytes 1:1536 bytes 2:1552 bytes 3:16000bytes
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 * Note:
 *      None
 */
rtk_api_ret_t rtl83xx_setAsicMaxLengthInRx(rtk_uint32 maxLength)
{	
	return rtl83xx_setAsicRegBits(RTL83XX_REG_MAX_LENGTH_LIMINT_IPG, RTL83XX_MAX_LENTH_CTRL_MASK, maxLength);
}

/* Function Name:
 *      rtk_switch_maxPktLen_set
 * Description:
 *      Set the max packet length of the specific unit
 * Input:
 *      len - max packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can set max packet length of the specific unit to
 *      - MAXPKTLEN_1522B,
 *      - MAXPKTLEN_1536B,
 *      - MAXPKTLEN_1552B,
 *      - MAXPKTLEN_16000B.
 */
rtk_api_ret_t rtk_switch_maxPktLen_set(rtk_switch_maxPktLen_t len)
{
    rtk_api_ret_t retVal;

    if (len>=MAXPKTLEN_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl83xx_setAsicMaxLengthInRx(len)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

rtk_api_ret_t rtk_port_macForceLinkExtSSC_set(rtk_port_t port, rtk_enable_t state)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regValue;
    rtk_uint32 type;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_EXT(port);

    if(state >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if((retVal = rtl8367c_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_getAsicReg(0x1300, &regValue)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x13C2, 0x0000)) != RT_ERR_OK)
        return retVal;

    switch (regValue)
    {
        case 0x0276:
        case 0x0597:
        case 0x6367:
            type = 1;
            break;
        case 0x0801:
        case 0x6511:
            type = 3;
            break;
        default:
            return RT_ERR_FAILED;
    }

    if(type == 1)
    {
        if(port == EXT_PORT0)
        {
            if(state == ENABLED)
            {
                if ((retVal = rtl8367c_setAsicReg(0x1d53, 0x0001)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d55, 0x000f)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d54, 0x05fa)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d52, 0x2473)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d70, 0x0000)) != RT_ERR_OK)
                    return retVal;

            }
            else
            {
                if ((retVal = rtl8367c_setAsicReg(0x1d53, 0x8000)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d55, 0x000f)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d54, 0x0008)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d52, 0x2411)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if(port == EXT_PORT1)
        {
            if(state == ENABLED)
            {
                if ((retVal = rtl8367c_setAsicReg(0x1d5a, 0x0001)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d5c, 0x000f)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d5b, 0x05fa)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d59, 0x2473)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d70, 0x0000)) != RT_ERR_OK)
                    return retVal;

            }
            else
            {
                if ((retVal = rtl8367c_setAsicReg(0x1d5a, 0x8000)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d5c, 0x000f)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d5b, 0x0008)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d59, 0x2411)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else
            return RT_ERR_PORT_ID;
    }
    else if(type == 3)
    {
        if(port == EXT_PORT0)
        {
            if(state == ENABLED)
            {
                if ((retVal = rtl8367c_setAsicReg(0x1d54, 0x05fa)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d53, 0x0001)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d52, 0x2472)) != RT_ERR_OK)
                    return retVal;

            }
            else
            {
                if ((retVal = rtl8367c_setAsicReg(0x1d52, 0x2470)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else if(port == EXT_PORT1)
        {
            if(state == ENABLED)
            {
                if ((retVal = rtl8367c_setAsicReg(0x1d5b, 0x05fa)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d5a, 0x0001)) != RT_ERR_OK)
                    return retVal;

                if ((retVal = rtl8367c_setAsicReg(0x1d59, 0x2472)) != RT_ERR_OK)
                    return retVal;

            }
            else
            {
                if ((retVal = rtl8367c_setAsicReg(0x1d52, 0x2470)) != RT_ERR_OK)
                    return retVal;
            }
        }
        else
            return RT_ERR_PORT_ID;
    }

    return RT_ERR_OK;
}

rtk_api_ret_t rtk_port_macForceLinkExtSSC_get(rtk_port_t port, rtk_enable_t *pState)
{
    rtk_api_ret_t retVal;
    rtk_uint32 data;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_IS_EXT(port);

    if(NULL == pState)
        return RT_ERR_NULL_POINTER;

    if(port == EXT_PORT0)
    {
        if ((retVal = rtl8367c_getAsicReg(0x1d52, &data)) != RT_ERR_OK)
            return retVal;
    }
    else if(port == EXT_PORT1)
    {
        if ((retVal = rtl8367c_getAsicReg(0x1d59, &data)) != RT_ERR_OK)
            return retVal;
    }

    *pState = (data & 0x0002) ? ENABLED : DISABLED;
    return RT_ERR_OK;
}

rtk_api_ret_t rtk_phyPatch_set(rtk_uint32 opt)
{
    rtk_api_ret_t retVal;
    rtk_uint32 port;
    rtk_uint32 data;

    if( (opt != 0) && (opt != 1) )
        return RT_ERR_FAILED;

    for(port = 0; port <= 4; port++)
    {
        if( (retVal = rtl8367c_getAsicPHYOCPReg(port, 0xA42C, &data)) != RT_ERR_OK)
            return retVal;

        data |= 0x0010;
        if( (retVal = rtl8367c_setAsicPHYOCPReg(port, 0xA42C, data)) != RT_ERR_OK)
            return retVal;

        if(opt == 0)
        {
            if( (retVal = rtl8367c_setAsicPHYOCPReg(port, 0xBC02, 0x00D0)) != RT_ERR_OK)
                return retVal;
        }
        else
        {
            if( (retVal = rtl8367c_setAsicPHYOCPReg(port, 0xBC02, 0x00F0)) != RT_ERR_OK)
                return retVal;
        }

        if ((retVal = rtl8367c_getAsicPHYReg(port, PHY_CONTROL_REG, &data)) != RT_ERR_OK)
        return retVal;

        data |= (0x0001 << 9);
        if ((retVal = rtl8367c_setAsicPHYReg(port, PHY_CONTROL_REG, data)) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* END of New API do not have these*/

rtk_uint32 r8367_cpu_port = EXT_PORT_FOR_HOST;

#ifdef ENABLE_8367RB_RGMII2
// for tr181
int rtk_rgmii_set(int enable)
{
    rtk_port_mac_ability_t mac_cfg;

    mac_cfg.forcemode = MAC_FORCE;
    mac_cfg.speed = SPD_1000M;
    mac_cfg.duplex = FULL_DUPLEX;
    if (enable == TRUE)
        mac_cfg.link = PORT_LINKUP;
    else		
        mac_cfg.link = PORT_LINKDOWN;
    mac_cfg.nway = DISABLED;
    mac_cfg.txpause = ENABLED;
    mac_cfg.rxpause = ENABLED;
    rtk_port_macForceLinkExt_set(EXT_PORT_FOR_RGMII2, MODE_EXT_RGMII, &mac_cfg);

    return 0;
}
#endif

int RTL83XX_init(void)
{
    rtk_portmask_t portmask;
    unsigned int ret, i;
    rtk_uint32 regData;

    /* Set external interface 0 to RGMII with Force mode, 1000M, Full-duple, enable TX&RX pause*/
    rtk_port_mac_ability_t mac_cfg;
    rtk_mode_ext_t mode ;

    // since we have set 8367RB GPIO reset pin to 0 and then to 1, so RTL83XX_REG_CHIP_RESET is no need
    //// do the whole chip reset in case the 8367 may be set in boot loader
    //rtl83xx_setAsicReg(RTL83XX_REG_CHIP_RESET, (1<<RTL83XX_CHIP_RST_OFFSET));
    //mdelay(1200);

    /* Initial Chip */
    ret = rtk_switch_init();

	rtk_get_halctrl_member(&vportmask, &vutpportmask);

    /* Enable LED Group 0&1 from P0 to P4 */
    portmask.bits[0] = vutpportmask;
    rtk_led_enable_set(LED_GROUP_0, &portmask);
    rtk_led_enable_set(LED_GROUP_X, &portmask);
 
    mode = MODE_EXT_RGMII ;
    mac_cfg.forcemode = MAC_FORCE;
    mac_cfg.speed = SPD_1000M;
    mac_cfg.duplex = FULL_DUPLEX;
    mac_cfg.link = PORT_LINKUP;
    mac_cfg.nway = DISABLED;
    mac_cfg.txpause = ENABLED;
    mac_cfg.rxpause = ENABLED;
    rtk_port_macForceLinkExt_set(EXT_PORT_FOR_HOST,mode,&mac_cfg);

#ifdef ENABLE_8367RB_RGMII2
    mode = MODE_EXT_RGMII ;
    mac_cfg.forcemode = MAC_FORCE;
    mac_cfg.speed = SPD_1000M;
    mac_cfg.duplex = FULL_DUPLEX;
    mac_cfg.link = PORT_LINKUP;
    mac_cfg.nway = DISABLED;
    mac_cfg.txpause = ENABLED;
    mac_cfg.rxpause = ENABLED;
    rtk_port_macForceLinkExt_set(EXT_PORT_FOR_RGMII2,mode,&mac_cfg);

    // the tx/rx delay is depend on the board
    //rtk_port_rgmiiDelayExt_set(EXT_PORT_FOR_RGMII2, 1, 3);
#endif

    /* Set RGMII Interface 0 TX delay to 2ns and RX to step 4 */
    // set the tx/rx delay in 8197D site
    //rtk_port_rgmiiDelayExt_set(EXT_PORT_1, 1, 4);
    //rtk_port_rgmiiDelayExt_set(EXT_PORT_1, 0, 0);
#if defined(CONFIG_RTL_8366SC_SUPPORT) || defined(CONFIG_RTL_8364NB_SUPPORT)
    rtk_port_rgmiiDelayExt_set(EXT_PORT_FOR_HOST, EXT1_RGMII_TX_DELAY, EXT1_RGMII_RX_DELAY); // change rxDelay to 7 to enhance the compatibility of 8197F and 8366SC
#else
    rtk_port_rgmiiDelayExt_set(EXT_PORT_FOR_HOST, EXT0_RGMII_TX_DELAY, EXT0_RGMII_TX_DELAY); // change rxDelay to 2 to enhance the compatibility of 8197D and 8367RB
#endif

    /* set CPU port */
	rtk_cpu_enable_set(ENABLE);
	
	rtk_cpu_tagPort_set(EXT_PORT_FOR_HOST, CPU_INSERT_TO_NONE);

    // for LED setting
#ifdef RTL83XX_USE_ONE_LED_PER_PORT
        rtl83xx_setAsicReg(RTL83XX_REG_LED_CONFIGURATION, 0x0222);
#else
        /* demo board use 2 LEDs for each port */	
        rtl83xx_setAsicReg(RTL83XX_REG_LED_CONFIGURATION, 0x0936);
#endif

#if 1
    // for 802.11ac logo 4.2.40 test (udp test item)
    rtl83xx_setAsicReg(0x121f, 0x01D6);
    rtl83xx_setAsicReg(0x1220, 0x01B8);
    rtl83xx_setAsicReg(0x1221, 0x01CC);
    rtl83xx_setAsicReg(0x1222, 0x01AE);
    rtl83xx_setAsicReg(0x1223, 0x0302);
    rtl83xx_setAsicReg(0x1224, 0x02E4);
    rtl83xx_setAsicReg(0x1225, 0x02D0);
    rtl83xx_setAsicReg(0x1226, 0x02A8);	
#endif

#if defined(CONFIG_RTL_ENABLE_EXT_SSC)
	if (EXT_PORT_FOR_HOST == EXT_PORT0) {
		// disable RGMII-2
		rtl83xx_setAsicReg(0x13C3, 0x0000);	
		rtl83xx_setAsicReg(0x13C4, 0x0000);
	}
	else {
		// disable RGMII-1
	}

	rtk_port_macForceLinkExtSSC_set(EXT_PORT_FOR_HOST, 1);

	// the RGMII Tx/Rx delay will be effected when enable SSC, need to re-find them.
	rtk_port_rgmiiDelayExt_set(EXT_PORT_FOR_HOST, EXT0_RGMII_TX_DELAY_ENABLE_SSC, EXT0_RGMII_RX_DELAY_ENABLE_SSC);
#endif
 
#if defined(CONFIG_RTK_REFINE_PORT_DUPLEX_MODE)
	rtk_forceFull_init();
#endif

#if defined(CONFIG_LAN_WAN_ISOLATION)
	{
	rtk_portmask_t pm;
    	
	pm.bits[0] = (vutpportmask | BIT(r8367_cpu_port));
	#ifdef ENABLE_8367RB_RGMII2
	pm.bits[0] |= BIT(EXT_PORT_FOR_RGMII2);
	#endif

	for (i=0; i<=4; i++)
		rtk_port_isolation_set(i, &pm);
	}
#endif

	/* Enable auto down speed to 10M */
	for (i=0; i<=4; i++) {
		rtl8367c_getAsicPHYOCPReg(i, 0xA442, &regData);
		rtl8367c_setAsicPHYOCPReg(i, 0xA442, (regData|0x20));
	}

	/* Enhance the anti-jamming capability of the port, 
	   and enhance the compatibility of the mismatch resistance. */
	rtk_phyPatch_set(0);

    return ret; 
}

int RTL83XX_init_switch_mode(void)
{
    rtk_portmask_t portmask;
    unsigned int ret;
 
    /* Set external interface 0 to RGMII with Force mode, 1000M, Full-duple, enable TX&RX pause*/
    rtk_port_mac_ability_t mac_cfg;
    rtk_mode_ext_t mode ;
 
    /* Initial Chip */
    ret = rtk_switch_init();

	rtk_get_halctrl_member(&vportmask, &vutpportmask);
	
    /* Enable LED Group 0&1 from P0 to P4 */
    portmask.bits[0]=vutpportmask;
    rtk_led_enable_set(LED_GROUP_0, &portmask);
    rtk_led_enable_set(LED_GROUP_1, &portmask);
 
    mode = MODE_EXT_RGMII ;
    mac_cfg.forcemode = MAC_FORCE;
    mac_cfg.speed = SPD_1000M;
    mac_cfg.duplex = FULL_DUPLEX;
    mac_cfg.link = PORT_LINKUP;
    mac_cfg.nway = DISABLED;
    mac_cfg.txpause = ENABLED;
    mac_cfg.rxpause = ENABLED;
    rtk_port_macForceLinkExt_set(EXT_PORT_FOR_HOST,mode,&mac_cfg);
 
    /* Set RGMII Interface 0 TX delay to 2ns and RX to step 4 */
    // set the tx/rx delay in 8197D site
    //rtk_port_rgmiiDelayExt_set(EXT_PORT_1, 1, 4);
    rtk_port_rgmiiDelayExt_set(EXT_PORT_FOR_HOST, 0, 2);
 
    /* set port 5 as CPU port */
    rtk_cpu_enable_set(ENABLE);
    rtk_cpu_tagPort_set(EXT_PORT_FOR_HOST, CPU_INSERT_TO_NONE);

    rtl83xx_setAsicReg(RTL83XX_REG_UNDA_FLOODING_PMSK, 0xff);
    rtl83xx_setAsicReg(RTL83XX_REG_UNMCAST_FLOADING_PMSK, 0xff);
    rtl83xx_setAsicReg(RTL83XX_REG_BCAST_FLOADING_PMSK, 0xff);

	rtk_switch_maxPktLen_set(MAXPKTLEN_16000B);
	rtl83xx_setAsicReg(0x1B03, 0x0222);

    return ret; 
}

#define BIT(nr)			(1UL << (nr))
#define WAN_VID			8
#define LAN_VID			9

#if defined(CONFIG_RTL_EXCHANGE_PORTMASK)
    #if defined(CONFIG_RTL_8363NB_SUPPORT) || defined(CONFIG_RTL_8364NB_SUPPORT)
    #define RTL83XX_WAN         1       // WAN port is set to 83XX port 1
    #else
    #define	RTL83XX_WAN			0		// WAN port is set to 83XX port 0
    #endif
#else
    #if defined(CONFIG_RTL_8366SC_SUPPORT) || defined(CONFIG_RTL_8363NB_SUPPORT) || defined(CONFIG_RTL_8365MB_SUPPORT) || defined(CONFIG_RTL_8364NB_SUPPORT)
    #define	RTL83XX_WAN			3		// WAN port is set to 83XX port 3
    #else
    #define	RTL83XX_WAN			4		// WAN port is set to 83XX port 4
    #endif
#endif

#define	RTL_WANPORT_MASK		(0x1 << RTL83XX_WAN)
#define RTL_LANPORT_MASK        (0xFF & (~RTL_WANPORT_MASK)) // may UTP_P0~UTP_P7

#define GATEWAY_MODE				0
#define BRIDGE_MODE				1

static int rtl8197d_op_mode = 0;

// refine RTL8367R_vlan_init() and RTL8367R_vlan_reinit()
#if 1

struct _vlan_conf {
	rtk_vlan_t 		vid;
	rtk_uint32		mbrmsk;
	rtk_uint32		untagmsk;
	rtk_fid_t			fid;
	rtk_pri_t 			priority;
};

#define _VID_END	(RTL83XX_VIDMAX+1)
#define _8367RB_RGMII2		EXT_PORT1
#define BIT(nr)			(1UL << (nr))

// please assign different fid for them
struct _vlan_conf vc_gateway[] = {
#ifdef ENABLE_8367RB_RGMII2
	{ 	LAN_VID,	 	(RTL_LANPORT_MASK | BIT(_8367RB_RGMII2)),   	(RTL_LANPORT_MASK | BIT(_8367RB_RGMII2)),	0, 0 },
#else
	{ 	LAN_VID,	 	RTL_LANPORT_MASK,   	RTL_LANPORT_MASK,	0, 0 },
#endif	
	{	WAN_VID,	RTL_WANPORT_MASK,   RTL_WANPORT_MASK,	1, 0},
	{	PASSTHRU_VLAN_ID,	(RTL_LANPORT_MASK|RTL_WANPORT_MASK),   (RTL_LANPORT_MASK|RTL_WANPORT_MASK), 0, 0},//for IPv6	
	{	_VID_END,	0, 0, 0, 0}
};

struct _vlan_conf vc_bridge_svl[] = {
	{ 	LAN_VID,	 	(RTL_LANPORT_MASK | RTL_WANPORT_MASK),   	(RTL_LANPORT_MASK | RTL_WANPORT_MASK),	2, 0 },
	{	_VID_END,	0, 0, 0, 0}
};


int _vlan_setting(struct _vlan_conf vc[])
{
	int i, j, retval;
	rtk_vlan_cfg_t vlanCfg;

	for(i=0; vc[i].vid <= RTL83XX_VIDMAX; i++)
	{
		memset(&vlanCfg,0x00,sizeof(rtk_vlan_cfg_t));
        vlanCfg.mbr.bits[0] = (vc[i].mbrmsk & vutpportmask) | BIT(r8367_cpu_port);
		vlanCfg.untag.bits[0] = vc[i].untagmsk & vutpportmask;
        vlanCfg.fid_msti = vc[i].fid;
		retval = rtk_vlan_set(vc[i].vid, &vlanCfg);

		if(vc[i].vid == PASSTHRU_VLAN_ID)		
			continue;
		
		/* set pvid*/	
		for(j=0;j<5;j++)
		{
			if  ((1<<j)& (vc[i].mbrmsk))
				rtk_vlan_portPvid_set(j, vc[i].vid, vc[i].priority);			
		}     	  
	}	
	return 0;
}

int RTL83XX_vlan_init(void)
{
	_vlan_setting(vc_gateway);

	rtl8197d_op_mode = GATEWAY_MODE;
	return 0;
}

int RTL83XX_vlan_reinit(int mode)
{
#if defined (CONFIG_RTL_IVL_SUPPORT)
	// when CONFIG_RTL_IVL_SUPPORT is defined, keep vc_gateway setting for gateway and bridge mode both
	
	rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, WAN_VID, r8367_cpu_port);				
#else

	if (mode==rtl8197d_op_mode) // no need tio do the re-initialization
		return 0;

	rtk_vlan_init();

	if (mode==GATEWAY_MODE)
		_vlan_setting(vc_gateway);
	
	else
		_vlan_setting(vc_bridge_svl);
		
#endif

	rtl8197d_op_mode = mode;	
	return 0;
}

#else
int RTL83XX_vlan_init(void)
{
	int i, retval;
	rtk_portmask_t mbrmsk, untagmsk;

	/* for lan */
	mbrmsk.bits[0] = RTL_LANPORT_MASK|BIT(r8367_cpu_port);
	untagmsk.bits[0] = RTL_LANPORT_MASK;
	retval = rtk_vlan_set(LAN_VID, mbrmsk, untagmsk, 0);

	for(i=0;i<5;i++) {
		if  ((1<<i)&RTL_LANPORT_MASK)
		{
			retval = rtk_vlan_portPvid_set(i, LAN_VID, 0);			
		}
	}
		
	/* for wan */
	mbrmsk.bits[0] = RTL_WANPORT_MASK|BIT(r8367_cpu_port);
	untagmsk.bits[0] = RTL_WANPORT_MASK;
	retval = rtk_vlan_set(WAN_VID, mbrmsk, untagmsk, 1);

	for(i=0;i<5;i++) {
		if  ((1<<i)&RTL_WANPORT_MASK)
		{
			retval = rtk_vlan_portPvid_set(i, WAN_VID, 0);
		}
	}

	rtl8197d_op_mode = GATEWAY_MODE;
	return 0;
}

int RTL83XX_vlan_reinit(int mode)
{
	int i, retval;
	rtk_portmask_t mbrmsk, untagmsk;

	if (mode==rtl8197d_op_mode) // no need tio do the re-initialization
		return 0;

	rtk_vlan_init();
	
	if (mode==GATEWAY_MODE)
	{
		/* for lan */
		mbrmsk.bits[0] = RTL_LANPORT_MASK|BIT(r8367_cpu_port);
		untagmsk.bits[0] = RTL_LANPORT_MASK;
		retval = rtk_vlan_set(LAN_VID, mbrmsk, untagmsk, 0);

		for(i=0;i<5;i++) {
			if  ((1<<i)&RTL_LANPORT_MASK)
			{
				retval = rtk_vlan_portPvid_set(i, LAN_VID, 0);			
			}
		}
	
		/* for wan */
		mbrmsk.bits[0] = RTL_WANPORT_MASK|BIT(r8367_cpu_port);
		untagmsk.bits[0] = RTL_WANPORT_MASK;
		retval = rtk_vlan_set(WAN_VID, mbrmsk, untagmsk, 1);

		for(i=0;i<5;i++) {
			if  ((1<<i)&RTL_WANPORT_MASK)
			{
				retval = rtk_vlan_portPvid_set(i, WAN_VID, 0);
			}
		}		
	} 
	else {
		/* for lan */
		mbrmsk.bits[0] = (RTL_LANPORT_MASK | RTL_WANPORT_MASK) |BIT(r8367_cpu_port);
		untagmsk.bits[0] = (RTL_LANPORT_MASK | RTL_WANPORT_MASK);
		retval = rtk_vlan_set(LAN_VID, mbrmsk, untagmsk, 0);

		for(i=0;i<5;i++) {
			if  ((1<<i)&(RTL_LANPORT_MASK | RTL_WANPORT_MASK))
			{
				retval = rtk_vlan_portPvid_set(i, LAN_VID, 0);			
			}
		}		
	}
	rtl8197d_op_mode = mode;
	
	return 0;
}
#endif

#define RTL83XX_WAN_PORT_BITMAP 		(1<<RTL83XX_WAN)
#define RTL83XX_LAN_PORT_BITMAP        (0xFF & (~RTL_WANPORT_MASK)) // may UTP_p0 ~ UTP_P7
#define RTL83XX_LAN_EFID				2
extern int rtl865x_curOpMode;

#if defined(CONFIG_RTL_VLAN_8021Q) && (defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT))
#if 0
int rtl_vlan_RTL8367R_set(unsigned short vid, unsigned int tagmask, unsigned int mask)
{
	rtk_portmask_t mbrmsk, untag;
	rtk_api_ret_t retVal;
	int i;
    unsigned untagmask;

    if(vid==WAN_VID || vid == LAN_VID)
        return 0;

    untagmask = mask&(~tagmask);
    
    mbrmsk.bits[0] = (BIT(r8367_cpu_port)| vutpportmask);

    //panic_printk("untagmsk is 0x%x\n", (untagmask&RTL83XX_LAN_PORT_BITMAP));
    
    if(tagmask&RTL83XX_WAN_PORT_BITMAP)
        untag.bits[0] = 0;
    else
        untag.bits[0] = (BIT(r8367_cpu_port)|RTL83XX_WAN_PORT_BITMAP|(untagmask&(RTL83XX_LAN_PORT_BITMAP & vutpportmask)));

    retVal=rtk_vlan_set(vid, mbrmsk, untag, 0); 
    
	return 0;
}
#else
int rtl_vlan_RTL83XX_set(unsigned short vid, unsigned int tagmask, unsigned int mask, unsigned int fid)
{
	int retval;
	rtk_vlan_cfg_t vlanCfg; 
	
    unsigned untagmask;
    untagmask = mask&(~tagmask);

	memset(&vlanCfg,0x00,sizeof(rtk_vlan_cfg_t));
	
    if (mask == 0 && tagmask == 0)//clear
    {
		vlanCfg.mbr.bits[0] = mask;
		vlanCfg.untag.bits[0] = untagmask;
        vlanCfg.fid_msti = fid;
    }
    else
	{
		//mbrmsk.bits[0] = (mask);
        //panic_printk("%s %d vid = %u mbrmsk.bits[0]=%u untagmsk.bits[0]=%u\n", __FUNCTION__, __LINE__, vid, mbrmsk.bits[0], untagmsk.bits[0]);
		vlanCfg.mbr.bits[0] = (mask) |BIT(r8367_cpu_port);;
		vlanCfg.untag.bits[0] = untagmask;
		vlanCfg.fid_msti = fid;
	}

	retval = rtk_vlan_set(vid, &vlanCfg);
        
	return 0;
}

#endif

int rtl_83XX_vlan_get(unsigned int i, unsigned int *mbrmsk, unsigned int *untagmsk, unsigned int *fid)
{
	rtk_api_ret_t ret = 0;
	rtk_vlan_cfg_t vlan1;

	if (!mbrmsk || !untagmsk || !fid)
		return -1;

	memset(&vlan1, 0x00, sizeof(rtk_vlan_cfg_t));
	ret = rtk_vlan_get(i, &vlan1);
	if (ret == RT_ERR_OK)
	{
		*mbrmsk = vlan1.mbr.bits[0];
		*untagmsk = vlan1.untag.bits[0];
		*fid = vlan1.fid_msti;
		return 0;
	}

	return -1;
}
#endif

int RTL83XX_vlan_set(void)
{
	rtk_portmask_t mbrmsk;
	rtk_vlan_cfg_t vlanCfg;
	rtk_api_ret_t retVal;
	int i;

 	for(i=0;i<4096;i++)
 	{
		memset(&vlanCfg, 0x00, sizeof(rtk_vlan_cfg_t));
		
 		//if (i==WAN_VID ||i==LAN_VID)
 		#ifdef CONFIG_RTL_VLAN_8021Q
 		if (i==WAN_VID || i==LAN_VID ||(i==1)
		#if defined(CONFIG_RTL_CUSTOM_PASSTHRU) 
			||i==PASSTHRU_VLAN_ID
		#endif	
			)
        #else
 		if (i==WAN_VID ||i==LAN_VID || (i==10) || (i==11) || (i==12) || (i==1)	
		#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
			||i==PASSTHRU_VLAN_ID
		#endif	
		) /* RTK VLAN */
        #endif
 		{
			vlanCfg.mbr.bits[0] = (BIT(r8367_cpu_port)| vutpportmask);
			vlanCfg.untag.bits[0] = (BIT(r8367_cpu_port)| vutpportmask);
		}
 		else
 		{
 			vlanCfg.mbr.bits[0] = (BIT(r8367_cpu_port)| vutpportmask);
 			vlanCfg.untag.bits[0] = 0;
 		} 	
			vlanCfg.fid_msti = 0;
			retVal=rtk_vlan_set(i, &vlanCfg); //all vlan's fid is 0 
 	}

	/* set pvid :  wan:8   lan:9  */	
	for(i=0;i<5;i++)
	{
		#if defined (CONFIG_RTL_IVL_SUPPORT)
		if (i == RTL83XX_WAN)
			retVal=rtk_vlan_portPvid_set(i, WAN_VID,0);
		else
			retVal=rtk_vlan_portPvid_set(i, LAN_VID,0);
		#else
		if(rtl865x_curOpMode==GATEWAY_MODE){
			if (i == RTL83XX_WAN)
				retVal=rtk_vlan_portPvid_set(i, WAN_VID,0);
			else
				retVal=rtk_vlan_portPvid_set(i, LAN_VID,0);
		}else{
			retVal=rtk_vlan_portPvid_set(i, LAN_VID,0);
		}
		#endif
	}       

	/* set wan port efid=1, other ports efid=2 */
	for(i=0;i<5;i++)
	{
		#if defined (CONFIG_RTL_IVL_SUPPORT)
		if (i == RTL83XX_WAN)
			retVal = rtk_port_efid_set(i,1);
		else
			retVal = rtk_port_efid_set(i,RTL83XX_LAN_EFID);
		#else
		if(rtl865x_curOpMode==GATEWAY_MODE){
			if (i == RTL83XX_WAN)
				retVal = rtk_port_efid_set(i,1);
			else
				retVal = rtk_port_efid_set(i,RTL83XX_LAN_EFID);
		}else{
			retVal = rtk_port_efid_set(i,RTL83XX_LAN_EFID);
		}
		#endif
	}

	// suggested by HM-Chung
#if defined (CONFIG_RTL_IVL_SUPPORT)	
	for (i=0; i<5; i++)
	{
		if (i == RTL83XX_WAN) 
			mbrmsk.bits[0] = BIT(r8367_cpu_port);    
		else
			mbrmsk.bits[0] = (vutpportmask | BIT(r8367_cpu_port)) & ~BIT(RTL83XX_WAN);

		rtk_port_isolation_set(i, &mbrmsk);
	}    
#else	
	for (i=0; i<5; i++)
	{
		if (rtl865x_curOpMode == GATEWAY_MODE) {
			if (i == RTL83XX_WAN) 
				mbrmsk.bits[0] = BIT(r8367_cpu_port);    
			else
				mbrmsk.bits[0] = (vutpportmask | BIT(r8367_cpu_port)) & ~BIT(RTL83XX_WAN);
		}
		else
			mbrmsk.bits[0] = (vutpportmask | BIT(r8367_cpu_port));
			
		rtk_port_isolation_set(i, &mbrmsk);
	}    
#endif

#if defined (CONFIG_RTL_IVL_SUPPORT)
	//if (rtl865x_curOpMode != GATEWAY_MODE) 
	{	// no matter Gateway or Bridge mode, always disable wan port L2 learning
		rtk_l2_limitLearningCnt_set(RTL83XX_WAN, 0);
		rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, WAN_VID, RTL83XX_WAN);		
	}
#else
	if (rtl865x_curOpMode == GATEWAY_MODE) {
		rtk_l2_limitLearningCnt_set(RTL83XX_WAN, 0);
		rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, WAN_VID, RTL83XX_WAN);		
		rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, LAN_VID, RTL83XX_WAN);		
	}
	else {
		rtk_l2_limitLearningCnt_set(RTL83XX_WAN, 2112);
		rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, WAN_VID, RTL83XX_WAN);		
		rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, LAN_VID, RTL83XX_WAN);		
	}
#endif

	/* disable cpu port's mac addr learning ability */
	rtk_l2_limitLearningCnt_set(r8367_cpu_port,0);

	/* disable unknown unicast/mcast/bcast flooding between LAN ports */
	rtl83xx_setAsicReg(RTL83XX_REG_UNDA_FLOODING_PMSK, BIT(rtk_switch_port_L2P_get(r8367_cpu_port)));
	rtl83xx_setAsicReg(RTL83XX_REG_UNMCAST_FLOADING_PMSK, BIT(rtk_switch_port_L2P_get(r8367_cpu_port)));
	rtl83xx_setAsicReg(RTL83XX_REG_BCAST_FLOADING_PMSK, BIT(rtk_switch_port_L2P_get(r8367_cpu_port)));

	return 0;
}

void RTL83XX_cpu_tag(int enable)
{
	if(enable){
		rtl83xx_setAsicReg(RTL83XX_REG_CPU_PORT_MASK,BIT(rtk_switch_port_L2P_get(r8367_cpu_port))); //set CPU port
		rtl83xx_setAsicReg(RTL83XX_REG_CPU_CTRL,0x281|(rtk_switch_port_L2P_get(r8367_cpu_port))<<RTL83XX_CPU_TRAP_PORT_OFFSET);
	}
	else{
		rtl83xx_setAsicReg(RTL83XX_REG_CPU_CTRL,0x280|(rtk_switch_port_L2P_get(r8367_cpu_port))<<RTL83XX_CPU_TRAP_PORT_OFFSET);
	}
}

void set_83XX_L2(unsigned int *mac, int intf_wan, int is_static)
{
	rtk_mac_t Mac;
	rtk_l2_ucastAddr_t L2_data;
		
	memset(&L2_data, 0, sizeof(rtk_l2_ucastAddr_t));
	
	L2_data.efid= (intf_wan)? 1 : 2;
	L2_data.port=r8367_cpu_port;
	L2_data.is_static=is_static;

	memcpy(&Mac.octet[0], mac, 6);
	rtk_l2_addr_add(&Mac,  &L2_data);
}

void del_83XX_L2(rtk_mac_t *pMac)
{
	rtk_l2_ucastAddr_t L2_data;

	memset(&L2_data, 0, sizeof(rtk_l2_ucastAddr_t));
	L2_data.fid = 0;
	L2_data.efid = RTL83XX_LAN_EFID;
	
	if (rtk_l2_addr_get(pMac, &L2_data) == RT_ERR_OK)
		rtk_l2_addr_del(pMac, &L2_data);

	return;
}

#if 0
void get_all_L2(void)
{
	int i, ret;
	rtk_l2_addr_table_t p;
		
	for (i=1; i<=RTK_MAX_NUM_OF_LEARN_LIMIT;i++)
	{
		p.index = i;
		ret = rtk_l2_entry_get(&p);
		if (ret == RT_ERR_OK)
		{
			printk(" [%d] mac: %02x:%02x:%02x:%02x:%02x:%02x, portmask: 0x%x, age: %d, fid: %d\n", i,
				p.mac.octet[0],p.mac.octet[1],p.mac.octet[2],p.mac.octet[3],p.mac.octet[4],p.mac.octet[5],
				p.portmask, p.age, p.fid);			
		}
	}
	return;
}
#endif

enum 
{
	PORT_DOWN=0,
	HALF_DUPLEX_10M,
	HALF_DUPLEX_100M,
	HALF_DUPLEX_1000M,
	DUPLEX_10M,
	DUPLEX_100M,
	DUPLEX_1000M,
	PORT_AUTO,
	PORT_UP,
	AN_10M,
	AN_100M,
	AN_AUTO
};

rtk_api_ret_t set_83XX_speed_mode(int port, int mode)
{
	rtk_port_phy_ability_t phyAbility;

	memset(&phyAbility, 0, sizeof(rtk_port_phy_ability_t));

	phyAbility.FC = 1;
	phyAbility.AsyFC = 1;
	phyAbility.AutoNegotiation = 1;

	if (mode == HALF_DUPLEX_10M) //10M half
	{
		phyAbility.Half_10 = 1;
	}
	else if (mode == DUPLEX_10M)	//10M full
	{
		phyAbility.Full_10 = 1;
	}
	else if (mode == HALF_DUPLEX_100M) // 100M half
	{
		phyAbility.Half_100 = 1;
	}
	else if (mode == DUPLEX_100M) // 100M full
	{
		phyAbility.Full_100 = 1;
	}
	else if (mode == DUPLEX_1000M) // 1000M
	{
		phyAbility.Full_1000 = 1;
	}
	else if (mode == AN_10M)
	{
		phyAbility.Half_10 = 1;
		phyAbility.Full_10 = 1;
	}
	else if (mode == AN_100M)
	{
		phyAbility.Half_100 = 1;
		phyAbility.Full_100 = 1;
	}
	else
	{
		phyAbility.Half_10 = 1;
		phyAbility.Full_10 = 1;
		phyAbility.Half_100 = 1;
		phyAbility.Full_100 = 1;
		phyAbility.Full_1000 = 1;
	}

	return (rtk_port_phyAutoNegoAbility_set(port, &phyAbility));
}

void rtl83XX_reset(void)
{
	rtl83xx_setAsicReg(RTL83XX_REG_CHIP_RESET, (1<<RTL83XX_CHIP_RST_OFFSET));	
	return;
}

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
int rtl8367_setProtocolBasedVLAN(rtk_vlan_proto_type_t proto_type,rtk_vlan_t cvid, int cmdFlag)
{
	rtk_port_t port;
	int ret;
	rtk_vlan_protoAndPortInfo_t info;
	info.proto_type=proto_type;
   	info.frame_type=FRAME_TYPE_ETHERNET;
    info.cvid=cvid;
    info.cpri=0;
	
	if(cmdFlag==TRUE)
	{
		//printk("ADD[%s]:[%d].\n",__FUNCTION__,__LINE__);
		/*add */
		for(port=0;port<5;port++){
			
			ret=rtk_vlan_protoAndPortBasedVlan_add(port, &info);
			
		}
	}
	else
	{
		//printk("DEL[%s]:[%d].\n",__FUNCTION__,__LINE__);
		/*delete */
		for(port=0;port<5;port++){
			
			ret=rtk_vlan_protoAndPortBasedVlan_del(port,  proto_type, FRAME_TYPE_ETHERNET);
		}
	}
	return ret;
}
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined(CONFIG_RTL_VLAN_8021Q) || defined(CONFIG_OPENWRT_SDK) || defined(CONFIG_RTL_HW_VLAN_SUPPORT)
int rtl865x_enableRtl8367ToCpuAcl(void)
{
	int retVal;
	rtk_filter_field_t	filter_field[2];
	rtk_filter_cfg_t	cfg;
	rtk_filter_action_t	act;
	rtk_filter_number_t	ruleNum = 0;

         memset(filter_field, 0, 2*sizeof(rtk_filter_field_t));
         memset(&cfg, 0, sizeof(rtk_filter_cfg_t));
         memset(&act, 0, sizeof(rtk_filter_action_t));
         filter_field[0].fieldType  = FILTER_FIELD_DMAC;
         if ((retVal = rtk_filter_igrAcl_field_add(&cfg,	&filter_field[0])) != RT_ERR_OK)
         	return retVal;

         /*add all ports to active ports*/
         cfg.activeport.value.bits[0] = vutpportmask;
         cfg.activeport.mask.bits[0] = vportmask;
         cfg.invert = FALSE;
         act.actEnable[FILTER_ENACT_TRAP_CPU] = TRUE;
         if ((retVal = rtk_filter_igrAcl_cfg_add(0, &cfg, &act, &ruleNum)) != RT_ERR_OK)
              	return retVal;
         return RT_ERR_OK;
}

int rtl865x_disableRtl8367ToCpuAcl(void)
{
	return rtk_filter_igrAcl_cfg_del(0);
}
#endif

#if defined(CONFIG_RTK_REFINE_PORT_DUPLEX_MODE)
#define MIB_STATE_FRAG_VAL_UPDATE_BASE		0x13A0
#define MIB_STATE_FRAG_CTL_UPDATE_BASE	0x13A4
int rtk_refinePortDuplexMode(void)
{
	rtk_stat_counter_t stateFragCounters = 0;
	int port;
	
	for(port=0; port<4; port++)
	{
		rtk_stat_port_get(port, STAT_EtherStatsFragments, &stateFragCounters);
		//if(port == 1)
			//printk("stateFragCounters is %d\n", (unsigned short)stateFragCounters);
		rtl83xx_setAsicReg(MIB_STATE_FRAG_VAL_UPDATE_BASE+port, (unsigned short)stateFragCounters);
		rtl83xx_setAsicReg(MIB_STATE_FRAG_CTL_UPDATE_BASE+port, 0x1);
	}
	return RT_ERR_OK;
}


rtk_api_ret_t rtk_forceFull_init(void)
{
    rtk_api_ret_t retVal;
//    rtk_uint32 busyFlag;    
    rtk_uint16 i,length;
    static rtk_uint8 iromCode[] = {
0x02,0x09,0x89,0x02,0x03,0x70,0x7D,0x17,
0x7C,0x20,0x7F,0x53,0x7E,0x02,0x12,0x09,
0x6D,0x7D,0x09,0x7C,0x06,0x7F,0x54,0x7E,
0x02,0x12,0x09,0x6D,0x7D,0x51,0x7C,0x00,
0x7F,0x36,0x7E,0x13,0x12,0x09,0x6D,0xE4,
    0x90,0x00,0x00,0xF0,0xA3,0xF0,0x90,0x00,
0x07,0xF0,0xA3,0xF0,0xA3,0xF0,0xA3,0xF0,
0x90,0x00,0x5C,0xF0,0x90,0x00,0x5C,0xE0,
0xFF,0xC3,0x94,0x05,0x50,0x59,0x74,0x01,
    0x7E,0x00,0xA8,0x07,0x08,0x80,0x05,0xC3,
0x33,0xCE,0x33,0xCE,0xD8,0xF9,0x54,0x1F,
0x60,0x11,0x90,0x00,0x5C,0xE0,0x24,0x12,
    0xFF,0xE4,0x34,0x13,0xFE,0xE4,0xFD,0xFC,
0x12,0x09,0x6D,0x90,0x00,0x5C,0xE0,0xFF,
0x25,0xE0,0x24,0x0B,0xF5,0x82,0xE4,0x34,
    0x00,0xF5,0x83,0xE4,0xF0,0xA3,0xF0,0x74,
    0x02,0x2F,0xF5,0x82,0xE4,0x34,0x00,0xF5,
0x83,0xE4,0xF0,0x74,0x15,0x2F,0xF5,0x82,
    0xE4,0x34,0x00,0xF5,0x83,0xE4,0xF0,0x90,
0x00,0x5C,0xE0,0x04,0xF0,0x80,0x9D,0x12,
0x09,0x9F,0x7D,0x01,0x7C,0x00,0x7F,0x18,
0x7E,0x11,0x12,0x09,0x6D,0x90,0x00,0x00,
    0xE0,0x70,0x02,0xA3,0xE0,0x70,0x03,0x02,
0x02,0xB0,0xE4,0x90,0x00,0x5C,0xF0,0x90,
0x00,0x5C,0xE0,0xFF,0xC3,0x94,0x05,0x40,
    0x03,0x02,0x02,0xB0,0x74,0x01,0x7E,0x00,
    0xA8,0x07,0x08,0x80,0x05,0xC3,0x33,0xCE,
    0x33,0xCE,0xD8,0xF9,0xFF,0x90,0x00,0x00,
    0xE0,0x5E,0xFE,0xA3,0xE0,0x5F,0x4E,0x70,
0x03,0x02,0x02,0xA7,0x90,0x00,0x5C,0xE0,
    0xFF,0x24,0x02,0xF5,0x82,0xE4,0x34,0x00,
    0xF5,0x83,0xE0,0x70,0x67,0xEF,0x24,0xA4,
0xFF,0xE4,0x34,0x13,0xFE,0x12,0x09,0x49,
0x90,0x00,0x61,0xEE,0xF0,0xA3,0xEF,0xF0,
    0x4E,0x70,0x03,0x02,0x02,0xA7,0x90,0x00,
0x5C,0xE0,0xFF,0x24,0x02,0xF5,0x82,0xE4,
    0x34,0x00,0xF5,0x83,0x74,0x01,0xF0,0xEF,
    0x24,0xA0,0xFF,0xE4,0x34,0x13,0xFE,0x12,
0x09,0x49,0x90,0x00,0x5C,0xE0,0xFD,0x25,
0xE0,0x24,0x0B,0xF5,0x82,0xE4,0x34,0x00,
    0xF5,0x83,0xEE,0xF0,0xA3,0xEF,0xF0,0x74,
0x15,0x2D,0xF5,0x82,0xE4,0x34,0x00,0xF5,
    0x83,0xE4,0xF0,0xED,0x24,0xA4,0xFF,0xE4,
0x34,0x13,0xFE,0xE4,0xFD,0xFC,0x12,0x09,
0x6D,0x02,0x02,0xA7,0xE4,0x90,0x00,0x63,
0xF0,0x90,0x00,0x5C,0xE0,0x24,0xA4,0xFF,
0xE4,0x34,0x13,0xFE,0x12,0x09,0x49,0x90,
0x00,0x61,0xEE,0xF0,0xA3,0xEF,0xF0,0x4E,
0x70,0x03,0x02,0x02,0x4C,0x90,0x00,0x5C,
    0xE0,0x24,0xA0,0xFF,0xE4,0x34,0x13,0xFE,
0x12,0x09,0x49,0x90,0x00,0x5D,0xEE,0xF0,
0xA3,0xEF,0xF0,0x90,0x00,0x5C,0xE0,0xFD,
0x25,0xE0,0x24,0x0B,0xF5,0x82,0xE4,0x34,
    0x00,0xF5,0x83,0xE0,0xFA,0xA3,0xE0,0x6F,
    0x70,0x02,0xEA,0x6E,0x60,0x59,0x90,0x00,
0x5C,0xE0,0xFF,0x24,0x15,0xF5,0x82,0xE4,
    0x34,0x00,0xF5,0x83,0xE0,0x04,0xF0,0xEF,
0x24,0x28,0xFF,0xE4,0x34,0x17,0xFE,0x74,
0x15,0x2D,0xF5,0x82,0xE4,0x34,0x00,0xF5,
0x83,0xE0,0xFD,0x7C,0x00,0x12,0x09,0x6D,
0x90,0x00,0x5C,0xE0,0x24,0x24,0xFF,0xE4,
0x34,0x17,0xFE,0xA3,0xE0,0xFC,0xA3,0xE0,
0xFD,0x12,0x09,0x6D,0x90,0x00,0x5C,0xE0,
0x24,0x15,0xF5,0x82,0xE4,0x34,0x00,0xF5,
    0x83,0xE0,0xD3,0x94,0x0A,0x40,0x34,0x90,
0x00,0x63,0x74,0x01,0xF0,0x80,0x2C,0x90,
0x00,0x5C,0xE0,0xFF,0x24,0x15,0xF5,0x82,
    0xE4,0x34,0x00,0xF5,0x83,0xE4,0xF0,0xEF,
0x24,0x28,0xFF,0xE4,0x34,0x17,0xFE,0x90,
0x00,0x5C,0xE0,0x24,0x15,0xF5,0x82,0xE4,
    0x34,0x00,0xF5,0x83,0xE0,0xFD,0x7C,0x00,
0x12,0x09,0x6D,0x90,0x00,0x5C,0xE0,0x24,
    0xA4,0xFF,0xE4,0x34,0x13,0xFE,0xE4,0xFD,
0xFC,0x12,0x09,0x6D,0x90,0x00,0x63,0xE0,
0x60,0x55,0x90,0x00,0x5C,0xE0,0x24,0x52,
0xFF,0xE4,0x34,0x13,0xFE,0x12,0x09,0x49,
    0xEE,0x44,0x10,0xAD,0x07,0xFC,0xED,0x54,
0xFB,0xFD,0x90,0x00,0x5C,0xE0,0x24,0x12,
0xFF,0xE4,0x34,0x13,0xFE,0x12,0x09,0x6D,
0x90,0x00,0x5C,0xE0,0xFF,0x74,0x01,0x7E,
    0x00,0xA8,0x07,0x08,0x80,0x05,0xC3,0x33,
    0xCE,0x33,0xCE,0xD8,0xF9,0xFF,0xF4,0xFD,
    0xEE,0xF4,0xFC,0x90,0x00,0x00,0xE0,0x5C,
0xF0,0xA3,0xE0,0x5D,0xF0,0x90,0x00,0x07,
    0xE0,0x4E,0xF0,0xA3,0xE0,0x4F,0xF0,0x90,
0x00,0x5C,0xE0,0x04,0xF0,0x02,0x00,0xBF,
0x90,0x00,0x09,0xE0,0x70,0x02,0xA3,0xE0,
0x70,0x03,0x02,0x00,0xAD,0xE4,0x90,0x00,
0x5C,0xF0,0x90,0x00,0x5C,0xE0,0xFF,0xC3,
0x94,0x05,0x40,0x03,0x02,0x00,0xAD,0x74,
    0x01,0x7E,0x00,0xA8,0x07,0x08,0x80,0x05,
    0xC3,0x33,0xCE,0x33,0xCE,0xD8,0xF9,0xFF,
0x90,0x00,0x09,0xE0,0x5E,0xFE,0xA3,0xE0,
0x5F,0x4E,0x60,0x7B,0x90,0x00,0x5C,0xE0,
    0x75,0xF0,0x20,0xA4,0x24,0x01,0xFF,0xE5,
0xF0,0x34,0x20,0xFE,0x12,0x09,0x49,0xE4,
0xFC,0xFD,0xEF,0x54,0x04,0xFF,0xE4,0xFE,
0xEF,0x70,0x5C,0x90,0x00,0x5C,0xE0,0x24,
0x12,0xFF,0xEE,0x34,0x13,0xFE,0xE4,0x12,
0x09,0x6D,0x90,0x00,0x5C,0xE0,0xF9,0x74,
0x01,0x7E,0x00,0xA8,0x01,0x08,0x80,0x05,
0xC3,0x33,0xCE,0x33,0xCE,0xD8,0xF9,0xF4,
0xFF,0xEE,0xF4,0xFE,0x90,0x00,0x00,0xE0,
0x5E,0xF0,0xA3,0xE0,0x5F,0xF0,0x90,0x00,
0x09,0xE0,0x5E,0xF0,0xA3,0xE0,0x5F,0xF0,
0x74,0x01,0x7E,0x00,0xA8,0x01,0x08,0x80,
0x05,0xC3,0x33,0xCE,0x33,0xCE,0xD8,0xF9,
0xF4,0xFF,0xEE,0xF4,0xFE,0x90,0x00,0x07,
0xE0,0x5E,0xF0,0xA3,0xE0,0x5F,0xF0,0x90,
0x00,0x5C,0xE0,0x04,0xF0,0x02,0x02,0xC2,
0xC0,0xE0,0xC0,0xF0,0xC0,0x83,0xC0,0x82,
0xC0,0xD0,0x75,0xD0,0x00,0xC0,0x00,0xC0,
0x01,0xC0,0x02,0xC0,0x03,0xC0,0x04,0xC0,
0x05,0xC0,0x06,0xC0,0x07,0xC2,0xAF,0x7F,
0x24,0x7E,0x11,0x12,0x09,0x49,0x90,0x00,
0x5A,0xEE,0xF0,0xA3,0xEF,0xF0,0x7F,0x23,
0x7E,0x11,0x12,0x09,0x49,0x90,0x00,0x58,
0xEE,0xF0,0xA3,0xEF,0xF0,0x78,0x0C,0x76,
0x00,0x78,0x0C,0xE6,0xFF,0xC3,0x94,0x05,
0x40,0x03,0x02,0x04,0xA9,0x74,0x01,0x7E,
0x00,0xA8,0x07,0x08,0x80,0x05,0xC3,0x33,
0xCE,0x33,0xCE,0xD8,0xF9,0xFF,0x90,0x00,
0x5A,0xE0,0x5E,0xFC,0xA3,0xE0,0x5F,0x4C,
0x70,0x03,0x02,0x04,0xA3,0xEF,0x54,0x1F,
0x70,0x03,0x02,0x04,0xA3,0x78,0x0C,0xE6,
0x75,0xF0,0x20,0xA4,0x24,0x05,0xFF,0xE5,
0xF0,0x34,0x20,0xFE,0x12,0x09,0x49,0xEE,
0x30,0xE6,0x03,0x02,0x04,0xA3,0x78,0x0C,
0xE6,0x24,0x52,0xFF,0xE4,0x34,0x13,0xFE,
0x12,0x09,0x49,0xAD,0x07,0xAC,0x06,0xED,
0x30,0xE2,0x03,0x02,0x04,0xA3,0x20,0xE7,
0x03,0x02,0x04,0xA3,0x78,0x0C,0xE6,0xF9,
0x74,0x01,0x7E,0x00,0xA8,0x01,0x08,0x80,
0x05,0xC3,0x33,0xCE,0x33,0xCE,0xD8,0xF9,
0xFF,0x90,0x00,0x07,0xE0,0x5E,0xFE,0xA3,
0xE0,0x5F,0x4E,0x70,0x66,0x04,0x7E,0x00,
0xA8,0x01,0x08,0x80,0x05,0xC3,0x33,0xCE,
0x33,0xCE,0xD8,0xF9,0xFF,0x90,0x00,0x00,
0xE0,0x4E,0xF0,0xA3,0xE0,0x4F,0xF0,0x74,
0x01,0x7E,0x00,0xA8,0x01,0x08,0x80,0x05,
0xC3,0x33,0xCE,0x33,0xCE,0xD8,0xF9,0xFF,
0x90,0x00,0x09,0xE0,0x4E,0xF0,0xA3,0xE0,
0x4F,0xF0,0xE9,0x24,0x12,0xFF,0xE4,0x34,
0x13,0xFE,0xEC,0x44,0x10,0xFC,0xED,0x44,
0x04,0xFD,0x12,0x09,0x6D,0x78,0x0C,0xE6,
0x24,0xA4,0xFF,0xE4,0x34,0x13,0xFE,0xE4,
0xFD,0xFC,0x12,0x09,0x6D,0x78,0x0C,0xE6,
0x24,0x02,0xF5,0x82,0xE4,0x34,0x00,0xF5,
0x83,0xE4,0xF0,0x78,0x0C,0x06,0x02,0x03,
0xB1,0x90,0x00,0x5A,0xE0,0xFC,0xA3,0xE0,
0xFD,0x7F,0x24,0x7E,0x11,0x12,0x09,0x6D,
0x90,0x00,0x58,0xE0,0xFC,0xA3,0xE0,0xFD,
0x7F,0x23,0x7E,0x11,0x12,0x09,0x6D,0x7F,
0x19,0x7E,0x11,0x12,0x09,0x49,0xAC,0x06,
0xAD,0x07,0x7F,0x19,0x7E,0x11,0x12,0x09,
0x6D,0x7F,0x51,0x7E,0x02,0x12,0x09,0x49,
0xEF,0x24,0x01,0xFD,0xE4,0x3E,0xFC,0x7F,
0x51,0x7E,0x02,0x12,0x09,0x6D,0xD2,0xAF,
0xD2,0xAF,0xD2,0xAF,0xD0,0x07,0xD0,0x06,
0xD0,0x05,0xD0,0x04,0xD0,0x03,0xD0,0x02,
0xD0,0x01,0xD0,0x00,0xD0,0xD0,0xD0,0x82,
0xD0,0x83,0xD0,0xF0,0xD0,0xE0,0x32,0xE8,
0x8F,0xF0,0xA4,0xCC,0x8B,0xF0,0xA4,0x2C,
0xFC,0xE9,0x8E,0xF0,0xA4,0x2C,0xFC,0x8A,
0xF0,0xED,0xA4,0x2C,0xFC,0xEA,0x8E,0xF0,
0xA4,0xCD,0xA8,0xF0,0x8B,0xF0,0xA4,0x2D,
0xCC,0x38,0x25,0xF0,0xFD,0xE9,0x8F,0xF0,
0xA4,0x2C,0xCD,0x35,0xF0,0xFC,0xEB,0x8E,
0xF0,0xA4,0xFE,0xA9,0xF0,0xEB,0x8F,0xF0,
0xA4,0xCF,0xC5,0xF0,0x2E,0xCD,0x39,0xFE,
0xE4,0x3C,0xFC,0xEA,0xA4,0x2D,0xCE,0x35,
0xF0,0xFD,0xE4,0x3C,0xFC,0x22,0xEF,0x5B,
0xFF,0xEE,0x5A,0xFE,0xED,0x59,0xFD,0xEC,
0x58,0xFC,0x22,0xEF,0x4B,0xFF,0xEE,0x4A,
0xFE,0xED,0x49,0xFD,0xEC,0x48,0xFC,0x22,
0xCF,0xF4,0xCF,0xCE,0xF4,0xCE,0xCD,0xF4,
0xCD,0xCC,0xF4,0xCC,0x22,0xEB,0x9F,0xF5,
0xF0,0xEA,0x9E,0x42,0xF0,0xE9,0x9D,0x42,
0xF0,0xEC,0x64,0x80,0xC8,0x64,0x80,0x98,
0x45,0xF0,0x22,0xEB,0x9F,0xF5,0xF0,0xEA,
0x9E,0x42,0xF0,0xE9,0x9D,0x42,0xF0,0xE8,
0x9C,0x45,0xF0,0x22,0xE8,0x60,0x0F,0xEC,
0xC3,0x13,0xFC,0xED,0x13,0xFD,0xEE,0x13,
0xFE,0xEF,0x13,0xFF,0xD8,0xF1,0x22,0xE8,
0x60,0x0F,0xEF,0xC3,0x33,0xFF,0xEE,0x33,
0xFE,0xED,0x33,0xFD,0xEC,0x33,0xFC,0xD8,
0xF1,0x22,0xBB,0x01,0x07,0x89,0x82,0x8A,
0x83,0x02,0x06,0x0C,0x50,0x05,0xE9,0xF8,
0x02,0x06,0x42,0xBB,0xFE,0x05,0xE9,0xF8,
0x02,0x06,0x4E,0x89,0x82,0x8A,0x83,0x02,
0x06,0x5A,0xBB,0x01,0x07,0x89,0x82,0x8A,
0x83,0x02,0x06,0x24,0x50,0x05,0xE9,0xF8,
0x02,0x06,0x6A,0xBB,0xFE,0x05,0xE9,0xF8,
0x02,0x06,0x76,0x22,0xE0,0xFC,0xA3,0xE0,
0xFD,0xA3,0xE0,0xFE,0xA3,0xE0,0xFF,0x22,
0xE0,0xF8,0xA3,0xE0,0xF9,0xA3,0xE0,0xFA,
0xA3,0xE0,0xFB,0x22,0xEC,0xF0,0xA3,0xED,
0xF0,0xA3,0xEE,0xF0,0xA3,0xEF,0xF0,0x22,
0xE0,0xFB,0xA3,0xE0,0xFA,0xA3,0xE0,0xF9,
0x22,0xEB,0xF0,0xA3,0xEA,0xF0,0xA3,0xE9,
0xF0,0x22,0xE6,0xFC,0x08,0xE6,0xFD,0x08,
0xE6,0xFE,0x08,0xE6,0xFF,0x22,0xE2,0xFC,
0x08,0xE2,0xFD,0x08,0xE2,0xFE,0x08,0xE2,
0xFF,0x22,0xE4,0x93,0xFC,0x74,0x01,0x93,
0xFD,0x74,0x02,0x93,0xFE,0x74,0x03,0x93,
0xFF,0x22,0xEC,0xF6,0x08,0xED,0xF6,0x08,
0xEE,0xF6,0x08,0xEF,0xF6,0x22,0xEC,0xF2,
0x08,0xED,0xF2,0x08,0xEE,0xF2,0x08,0xEF,
0xF2,0x22,0x90,0x00,0x1A,0x12,0x06,0x24,
0x90,0x00,0x1A,0x12,0x06,0x0C,0xE4,0xFD,
0xFC,0x90,0x00,0x1A,0x12,0x06,0x24,0x90,
0x00,0x1E,0x12,0x06,0x0C,0xE4,0xFD,0xFC,
0x90,0x00,0x1E,0x12,0x06,0x24,0x90,0x00,
0x1A,0x12,0x06,0x0C,0xD3,0xEF,0x94,0xFF,
0xEE,0x94,0xFF,0x40,0x03,0x02,0x07,0x5F,
0x90,0x00,0x1E,0x12,0x06,0x0C,0xE4,0xFC,
0xFD,0xFB,0xFA,0x79,0x01,0xF8,0xD3,0x12,
0x05,0x85,0x50,0x03,0x02,0x07,0x5F,0xE4,
0xFF,0xFE,0xFD,0xFC,0x90,0x00,0x2A,0x12,
0x06,0x24,0x90,0x00,0x2A,0x12,0x06,0x0C,
    0x74,0x01,0x7E,0x00,0xA8,0x07,0x08,0x80,
    0x05,0xC3,0x33,0xCE,0x33,0xCE,0xD8,0xF9,
0xFF,0xEE,0x33,0x95,0xE0,0xFD,0xFC,0x90,
0x00,0x1E,0x12,0x06,0x18,0x12,0x05,0x5E,
0xEC,0x4D,0x4E,0x4F,0x70,0x2D,0x90,0x00,
0x2A,0x12,0x06,0x0C,0xEF,0x24,0x01,0xFF,
0xE4,0x3E,0xFE,0xE4,0x3D,0xFD,0xE4,0x3C,
0xFC,0x90,0x00,0x2A,0x12,0x06,0x24,0xE4,
0x7F,0x10,0xFE,0xFD,0xFC,0x90,0x00,0x2A,
0x12,0x06,0x18,0xC3,0x12,0x05,0x9B,0x40,
0xA9,0x80,0x2C,0x90,0x00,0x2A,0x12,0x06,
0x0C,0xA9,0x07,0x90,0x00,0x22,0x12,0x06,
0x0C,0xA8,0x01,0x12,0x05,0xBF,0x90,0x00,
0x2E,0x12,0x06,0x24,0xE4,0x7F,0xFF,0x7E,
0xFF,0xFD,0xFC,0x90,0x00,0x2E,0x12,0x06,
0x18,0xD3,0x12,0x05,0x9B,0x40,0x05,0x7E,
0x00,0x7F,0x01,0x22,0x90,0x00,0x1A,0x12,
0x06,0x0C,0x12,0x09,0x49,0xE4,0xFC,0xFD,
0x90,0x00,0x26,0x12,0x06,0x24,0x90,0x00,
0x1E,0x12,0x06,0x0C,0x12,0x05,0x78,0x90,
0x00,0x26,0x12,0x06,0x18,0x12,0x05,0x5E,
0x90,0x00,0x26,0x12,0x06,0x24,0x90,0x00,
0x2E,0x12,0x06,0x0C,0x90,0x00,0x1E,0x12,
0x06,0x18,0x12,0x05,0x5E,0x90,0x00,0x26,
0x12,0x06,0x18,0x12,0x05,0x6B,0x90,0x00,
0x26,0x12,0x06,0x24,0x90,0x00,0x1A,0x12,
0x06,0x0C,0xC0,0x06,0xC0,0x07,0x90,0x00,
0x26,0x12,0x06,0x0C,0xAD,0x07,0xAC,0x06,
0xD0,0x07,0xD0,0x06,0x12,0x09,0x6D,0xE4,
0xFE,0xFF,0x22,0x90,0x00,0x32,0x12,0x06,
0x24,0x90,0x00,0x36,0x12,0x06,0x0C,0xE4,
0xFF,0xEE,0x54,0xFC,0xFE,0xE4,0xFD,0xFC,
0x78,0x0A,0x12,0x05,0xAC,0x90,0x00,0x3D,
0x12,0x06,0x24,0xE4,0x7F,0x3F,0xFE,0xFD,
0xFC,0x90,0x00,0x1E,0x12,0x06,0x24,0x90,
0x00,0x3D,0x12,0x06,0x0C,0x90,0x00,0x22,
0x12,0x06,0x24,0xE4,0x7F,0x15,0x7E,0x1D,
0xFD,0xFC,0x12,0x06,0x82,0xEF,0x4E,0x60,
0x01,0x22,0x90,0x00,0x36,0x12,0x06,0x0C,
0x78,0x06,0x12,0x05,0xAC,0xEF,0x54,0x0F,
0xFF,0xE4,0xFE,0xFD,0xFC,0x90,0x00,0x41,
0x12,0x06,0x24,0x90,0x00,0x36,0x12,0x06,
0x0C,0x78,0x01,0x12,0x05,0xAC,0xEF,0x54,
0x1F,0xFF,0xE4,0xFE,0xFD,0xFC,0x90,0x00,
0x45,0x12,0x06,0x24,0x90,0x00,0x41,0x12,
0x06,0x0C,0x78,0x08,0x12,0x05,0xBF,0xEE,
0x44,0x20,0xFE,0xEC,0xC0,0x04,0xC0,0x05,
0xC0,0x06,0xC0,0x07,0x90,0x00,0x32,0x12,
0x06,0x0C,0x78,0x05,0x12,0x05,0xBF,0xD0,
0x03,0xD0,0x02,0xD0,0x01,0xD0,0x00,0x12,
0x05,0x6B,0x90,0x00,0x45,0x12,0x06,0x18,
0x12,0x05,0x6B,0x90,0x00,0x3A,0x12,0x06,
0x30,0x12,0x08,0xEE,0xEF,0x4E,0x60,0x01,
0x22,0xE4,0xFE,0xFF,0x22,0x90,0x00,0x49,
0x12,0x06,0x24,0xE4,0x7F,0x1F,0xFE,0xFD,
0xFC,0x90,0x00,0x4D,0x12,0x06,0x18,0xD3,
0x12,0x05,0x9B,0x40,0x05,0x7E,0x0E,0x7F,
0x01,0x22,0xE4,0x7F,0x02,0xFE,0xFD,0xFC,
0x90,0x00,0x4D,0x12,0x06,0x18,0x12,0x05,
0x0F,0xE4,0x2F,0xFF,0xEE,0x34,0xA4,0xFE,
0xE4,0x3D,0xFD,0xE4,0x3C,0xFC,0x90,0x00,
0x54,0x12,0x06,0x24,0x90,0x00,0x54,0x12,
0x06,0x0C,0x90,0x00,0x36,0x12,0x06,0x24,
0x90,0x00,0x51,0x12,0x06,0x30,0x90,0x00,
0x3A,0x12,0x06,0x39,0x90,0x00,0x49,0x12,
0x06,0x0C,0x12,0x07,0xCB,0x22,0x90,0x00,
0x64,0x12,0x06,0x24,0x90,0x00,0x68,0x12,
0x06,0x39,0x90,0x00,0x64,0x12,0x06,0x0C,
0xE4,0xFD,0xFC,0x90,0x00,0x64,0x12,0x06,
0x24,0xE4,0x7F,0xFF,0x7E,0xFF,0xFD,0xFC,
0x90,0x00,0x64,0x12,0x06,0x18,0xD3,0x12,
0x05,0x9B,0x40,0x05,0x7E,0x00,0x7F,0x01,
0x22,0x90,0x00,0x64,0x12,0x06,0x0C,0x12,
0x09,0x49,0xE4,0xFC,0xFD,0x90,0x00,0x68,
0x12,0x06,0x30,0x12,0x05,0xF2,0x90,0x00,
0x68,0x12,0x06,0x30,0x12,0x05,0xD2,0xE4,
0xFD,0xFC,0x12,0x05,0xF2,0xE4,0xFE,0xFF,
0x22,0xC2,0xAF,0xAD,0x07,0xAC,0x06,0x8C,
0xA2,0x8D,0xA3,0x75,0xA0,0x01,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xAE,0xA1,0xBE,0x00,0xF0,0xAE,0xA6,
0xAF,0xA7,0xD2,0xAF,0x22,0xC2,0xAF,0xAB,
0x07,0xAA,0x06,0x8A,0xA2,0x8B,0xA3,0x8C,
0xA4,0x8D,0xA5,0x75,0xA0,0x03,0x00,0x00,
0x00,0xAA,0xA1,0xBA,0x00,0xF8,0xD2,0xAF,
0x22,0x78,0x7F,0xE4,0xF6,0xD8,0xFD,0x75,
0x81,0x0C,0x02,0x00,0x06,0xE4,0xF5,0xA8,
0xF5,0xE8,0xD2,0xAF,0xD2,0xA8,0x22,0x12,
0x09,0xA5,0x02,0x09,0x95,0xE4,0xF5,0x8E,
0x22,};

    length  = (sizeof(iromCode) / sizeof(rtk_uint8));
	
    if ((retVal = rtl83xx_setAsicRegBit(0x1322, 4, 1))!=RT_ERR_OK)
        return retVal;
    if ((retVal = rtl83xx_setAsicRegBit(0x130c, 5, 1))!=RT_ERR_OK)
        return retVal;
    if ((retVal = rtl83xx_setAsicRegBit(0x1336, 1, 1))!=RT_ERR_OK)
        return retVal;
    if ((retVal = rtl83xx_setAsicRegBit(0x1322, 2, 0))!=RT_ERR_OK)
        return retVal;

    for(i=0; i<length; i++)
    {
        if(i == 0x2000)
        {
            if ((retVal = rtl83xx_setAsicRegBit(0x1336, 2, 1))!=RT_ERR_OK)
                return retVal;
        }
        rtl83xx_setAsicReg((rtk_uint32)(0xE000 + i % 0x2000), (rtk_uint32)iromCode[i]);
    }

    if ((retVal = rtl83xx_setAsicRegBit(0x1336, 2, 0))!=RT_ERR_OK)
        return retVal;
    if ((retVal = rtl83xx_setAsicRegBit(0x1336, 1, 0))!=RT_ERR_OK)
        return retVal;
    if ((retVal = rtl83xx_setAsicRegBit(0x1322, 4, 0))!=RT_ERR_OK)
        return retVal;
        
    return RT_ERR_OK;

}
#endif

#if 1//defined(CONFIG_RTL_HW_VLAN_SUPPORT)
#if defined(CONFIG_RTL_PROC_NEW)
int rtl_8367r_vlan_read(struct seq_file *s, void *v)
{
	int  i = 0, ret = 0;
	unsigned int pvid = 0, priority = 0;
	rtk_vlan_cfg_t vlan1;
    rtk_l2_addr_table_t l2_entry;

	seq_printf(s, "%s\n", "vlan:");
    for (i=1; i <= 1000; i++)
    {
		if(i!=1 && i!=8 && i!=9 && i%100!=0)
			continue;

		memset(&vlan1, 0x00, sizeof(rtk_vlan_cfg_t));

        if ((rtk_vlan_get(i, &vlan1)== 0) && (vlan1.mbr.bits[0] != 0))
            seq_printf(s, "vid %d Mbrmsk 0x%x Untagmsk 0x%x fid %u\n", i, vlan1.mbr.bits[0], vlan1.untag.bits[0], vlan1.fid_msti);           
    }

	seq_printf(s, "\n%s\n", "pvid:");
    for(i=0;i<8;i++)
    {
        pvid = priority = 0;        
        if (rtk_vlan_portPvid_get(i, &pvid, &priority)==0)
            seq_printf(s, "port %d pvid %u pri %u\n", i, pvid, priority);           
    }

    
	seq_printf(s, "\n%s\n", "l2:");
     /*Get All Lookup Table and Print the valid entry*/
 
    for (i=1;i<=2112;i++)
    {
        memset(&l2_entry,0,sizeof(rtk_l2_addr_table_t));
        l2_entry.index = i;
        ret = rtk_l2_entry_get(&l2_entry);
        if (ret==RT_ERR_OK)
        {
            if(l2_entry.is_ipmul)
            {
                
                seq_printf(s, "\r\nIndex SourceIP DestinationIP MemberPort State\n");           
                seq_printf(s, "%4d ", l2_entry.index);
                seq_printf(s,"%0x ",(l2_entry.sip));
                seq_printf(s,"%0x ",(l2_entry.dip));
                seq_printf(s,"%-8x ",l2_entry.portmask.bits[0]);
                seq_printf(s,"%s \n",(l2_entry.is_static? "Static" : "Auto"));
            }
            else if(l2_entry.mac.octet[0]&0x01)
            {
                seq_printf(s,"%4d %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X %-8x %-4d %-4s %-5s %s %d\n",
                l2_entry.index,
                l2_entry.mac.octet[0],l2_entry.mac.octet[1],l2_entry.mac.octet[2],
                l2_entry.mac.octet[3],l2_entry.mac.octet[4],l2_entry.mac.octet[5],
                l2_entry.portmask.bits[0], l2_entry.fid, (l2_entry.auth ? "Auth" : "x"),
                (l2_entry.sa_block? "Block" : "x"), (l2_entry.is_static? "Static" : "Auto"),
                l2_entry.age);
            }
            else if((l2_entry.age!=0)||(l2_entry.is_static==1))
            {
                seq_printf(s,"%4d %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X %-8x %-4d %-4s %-5s %s %d\n",
                l2_entry.index,l2_entry.mac.octet[0],l2_entry.mac.octet[1],l2_entry.mac.octet[2],
                l2_entry.mac.octet[3],l2_entry.mac.octet[4],l2_entry.mac.octet[5],
                l2_entry.portmask.bits[0], l2_entry.fid, (l2_entry.auth ? "Auth" : "x"),
                (l2_entry.sa_block? "Block" : "x"), (l2_entry.is_static? "Static" : "Auto"),
                l2_entry.age);
            }
        }
    }


	return 0;
}

#else
int rtl_8367r_vlan_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len = 0, i = 0, ret = 0;
    unsigned int pvid = 0, priority = 0, fid = 0;
    rtk_portmask_t Mbrmsk = {0}, Untagmsk={0};
    
	len = sprintf(page, "%s\n", "vlan:");
    for (i=0; i < 4096; i++)
    {
        memset(&Mbrmsk, 0x00, sizeof(Mbrmsk));        
        memset(&Untagmsk, 0x00, sizeof(Untagmsk));
        fid =0;
        if ((rtk_vlan_get(i, &Mbrmsk, &Untagmsk, &fid)== 0) && (Mbrmsk.bits[0] != 0))
            len += sprintf(page+len, "vid %d Mbrmsk 0x%x Untagmsk 0x%x fid %u\n", i, Mbrmsk.bits[0], Untagmsk.bits[0], fid);           
    }

	len += sprintf(page+len, "\n%s\n", "pvid:");
    for(i=0;i<8;i++)
    {
        pvid = priority = 0;        
        if (rtk_vlan_portPvid_get(i, &pvid, &priority)==0)
            len += sprintf(page+len, "port %d pvid %u pri %u\n", i, pvid, priority);           
    }

    
	len += sprintf(page+len, "\n%s\n", "l2:");
     /*Get All Lookup Table and Print the valid entry*/
    rtk_l2_addr_table_t l2_entry; 
    for (i=1;i<=2112;i++)
    {
        memset(&l2_entry,0,sizeof(rtk_l2_addr_table_t));
        l2_entry.index = i;
        ret = rtk_l2_entry_get(&l2_entry);
        if (ret==RT_ERR_OK)
        {
            if(l2_entry.is_ipmul)
            {
                
                len += sprintf(page+len, "\r\nIndex SourceIP DestinationIP MemberPort State\n");           
                len += sprintf(page+len, "%4d ", l2_entry.index);
                len += sprintf(page+len,"%0x ",(l2_entry.sip));
                len += sprintf(page+len,"%0x ",(l2_entry.dip));
                len += sprintf(page+len,"%-8x ",l2_entry.portmask);
                len += sprintf(page+len,"%s \n",(l2_entry.is_static? "Static" : "Auto"));
            }
            else if(l2_entry.mac.octet[0]&0x01)
            {
                len += sprintf(page+len,"%4d %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X %-8x %-4d %-4s %-5s %s %d\n",
                l2_entry.index,
                l2_entry.mac.octet[0],l2_entry.mac.octet[1],l2_entry.mac.octet[2],
                l2_entry.mac.octet[3],l2_entry.mac.octet[4],l2_entry.mac.octet[5],
                l2_entry.portmask, l2_entry.fid, (l2_entry.auth ? "Auth" : "x"),
                (l2_entry.sa_block? "Block" : "x"), (l2_entry.is_static? "Static" : "Auto"),
                l2_entry.age);
            }
            else if((l2_entry.age!=0)||(l2_entry.is_static==1))
            {
                len += sprintf(page+len,"%4d %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X %-8x %-4d %-4s %-5s %s %d\n",
                l2_entry.index,l2_entry.mac.octet[0],l2_entry.mac.octet[1],l2_entry.mac.octet[2],
                l2_entry.mac.octet[3],l2_entry.mac.octet[4],l2_entry.mac.octet[5],
                l2_entry.portmask, l2_entry.fid, (l2_entry.auth ? "Auth" : "x"),
                (l2_entry.sa_block? "Block" : "x"), (l2_entry.is_static? "Static" : "Auto"),
                l2_entry.age);
            }
        }
    }

     
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}
#endif
#endif


#if defined(CONFIG_RTL_DNS_TRAP)
int rtl_8367_add_acl_for_dns(unsigned int acl_idx)
{
	int retVal; //, acl_idx=0;
	rtk_filter_field_t	filter_field[2];
	rtk_filter_cfg_t	cfg;
	rtk_filter_action_t	act;
	rtk_filter_number_t	ruleNum = 0;

	/* disable cpu port's mac addr learning ability */
	//rtl8367b_setAsicLutLearnLimitNo(r8367_cpu_port,0);

	/* disable unknown unicast/mcast/bcast flooding between LAN ports */
	//smi_write(RTL8367B_REG_UNDA_FLOODING_PMSK, BIT(r8367_cpu_port));

    memset(&filter_field, 0, 2*sizeof(rtk_filter_field_t));
    memset(&cfg, 0, sizeof(rtk_filter_cfg_t));
    memset(&act, 0, sizeof(rtk_filter_action_t));
    /*Search all MAC (data & mask are all "0")*/
    filter_field[0].fieldType = FILTER_FIELD_UDP_DPORT;                    
    filter_field[0].filter_pattern_union.udpDstPort.dataType = FILTER_FIELD_DATA_MASK;
    filter_field[0].filter_pattern_union.udpDstPort.value = 53;
    filter_field[0].filter_pattern_union.udpDstPort.mask = 0xFFFF;
    filter_field[0].next = NULL;        
    if ((retVal = rtk_filter_igrAcl_field_add(&cfg, &filter_field[0])) != RT_ERR_OK)
       return RT_ERR_FAILED;   

    cfg.careTag.tagType[CARE_TAG_UDP].value = TRUE;
    cfg.careTag.tagType[CARE_TAG_UDP].mask = TRUE;
    /*Add port0~port4 to active ports*/
	cfg.activeport.value.bits[0] = vutpportmask;
	cfg.activeport.mask.bits[0] = vportmask;
	cfg.invert =FALSE;         
    /*Set Action to Trap to CPU*/
    act.actEnable[FILTER_ENACT_TRAP_CPU] = TRUE;        
    if ((retVal = rtk_filter_igrAcl_cfg_add(acl_idx, &cfg, &act, &ruleNum)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    return RT_ERR_OK;			
}
int rtl_8367_remove_acl_for_dns(unsigned int acl_idx)
{
	rtk_filter_igrAcl_cfg_del(acl_idx);
}
#endif
#if defined (IMPROVE_MCAST_PERFORMANCE_WITH_RTL8367)
int rtl_initMcastImprove(void)
{
	return rtk_qos_init(1);
}
int rtl865x_enableRtl8367McastPriorityAcl(int priority)
{
	int retVal;
	rtk_filter_field_t	filter_field[2];
	rtk_filter_cfg_t	cfg;
	rtk_filter_action_t	act;
	rtk_filter_number_t	ruleNum = 0;

	
	memset(filter_field, 0, 2*sizeof(rtk_filter_field_t));
	memset(&cfg, 0, sizeof(rtk_filter_cfg_t));
	memset(&act, 0, sizeof(rtk_filter_action_t));

	filter_field[0].fieldType = FILTER_FIELD_DMAC;
	filter_field[0].filter_pattern_union.dmac.dataType = FILTER_FIELD_DATA_MASK;
	filter_field[0].filter_pattern_union.dmac.value.octet[0] = 0x01;
	filter_field[0].filter_pattern_union.dmac.value.octet[1] = 0x00;
	filter_field[0].filter_pattern_union.dmac.value.octet[2] = 0x5e;
	filter_field[0].filter_pattern_union.dmac.mask.octet[0] = 0xFF;
	filter_field[0].filter_pattern_union.dmac.mask.octet[1] = 0xFF;
	filter_field[0].filter_pattern_union.dmac.mask.octet[1] = 0xFF;
	filter_field[0].next = NULL;
	if ((retVal = rtk_filter_igrAcl_field_add(&cfg, &filter_field[0])) != RT_ERR_OK)
		return retVal;

	cfg.activeport.value.bits[0] = vutpportmask;
	cfg.activeport.mask.bits[0] = vportmask;
	cfg.invert = FALSE;
	act.actEnable[FILTER_ENACT_PRIORITY] = TRUE;
	act.filterPriority = priority;

	if ((retVal = rtk_filter_igrAcl_cfg_add(1, &cfg, &act, &ruleNum)) != RT_ERR_OK)
        return retVal;

	return RT_ERR_OK;

}
int rtl865x_disableRtl8367McastPriorityAcl(void)
{
	return rtk_filter_igrAcl_cfg_del(1);
}

int rtl_enable_mCast_improve(int enable)
{
	if(enable)
	{
		rtk_qos_queueNum_set(r8367_cpu_port, 2);
		rtl865x_enableRtl8367McastPriorityAcl(7);
	}
	else
	{
		rtk_qos_queueNum_set(r8367_cpu_port, 1);
		rtl865x_disableRtl8367McastPriorityAcl();
	}
	return 0;
}
#endif

#if defined CONFIG_RTL_8367_QOS_SUPPORT || defined CONFIG_RTL_8367_QOS_TEST
#define MAX_PHY_PORT_NUM 5
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define MAX_RTL8367_QOS_QUEUE_NUM 8
#else
#define MAX_RTL8367_QOS_QUEUE_NUM 6
#endif
#define	QOS_VALID_MASK	0x2
#define	QOS_TYPE_MASK		0x1
#define	QOS_TYPE_STR		0x0	/*0x0|QOS_VALID_MASK*/
#define	QOS_TYPE_WFQ		0x1	/*0x1|QOS_VALID_MASK*/
#if defined CONFIG_RTL_8367_QOS_SUPPORT
#define CPU_METER_ID1        16
#define CPU_METER_ID2        17
#define UNKNOWN_OWNER_METER  0
#define WAN_METER            1
#define LAN_METER            2
#define CPU_METER            3
int qos_meter_owner[RTL83XX_METERNO];
#endif

#if defined CONFIG_RTL_8367_QOS_SUPPORT || defined CONFIG_RTL_8367_QOS_TEST
#if defined CONFIG_RTL_PROC_NEW
int rtl_8367QosReadProc(struct seq_file *s, void *v)
{
	int port, queueNum, i, queue;	
	rtk_qos_queue_weights_t qWeights;
	rtk_meter_id_t meterId;
	rtk_meter_type_t type;
	rtk_rate_t qRate, pRate;
	rtk_enable_t qIfg_include, pIfg_include, qEnable;
	rtk_uint32 Bucket_size;
    rtk_port_mac_ability_t mac_cfg;
    rtk_mode_ext_t mode ;
	rtk_rate_t inRate;
	rtk_enable_t inIfg_include, inFc_enable;

    rtk_port_macForceLinkExt_get(EXT_PORT_FOR_HOST,&mode,&mac_cfg);

	seq_printf(s, "%s\n", "rtl8367 cpu flow control");
	seq_printf(s, "\ttx pause:%d\n", mac_cfg.txpause);
	seq_printf(s, "\trx pause:%d\n", mac_cfg.rxpause);

	seq_printf(s, "%s\n", "rtl8367 qos related parameters");
	//for(port=0; port<RTK_PORT_ID_MAX; port++)
	for(port=0; port <= EXT_PORT2; port++)
	{
		if(port>UTP_PORT7 && port!=r8367_cpu_port)
			continue;
			
		rtk_qos_queueNum_get(port,&queueNum);
		rtk_rate_egrBandwidthCtrlRate_get(port, &pRate, &pIfg_include); 

		rtk_qos_schedulingQueue_get(port, &qWeights);
		rtk_rate_egrQueueBwCtrlEnable_get(port, RTK_WHOLE_SYSTEM, &qEnable); 
		if (port < RTK_MAX_NUM_OF_PORT){
			seq_printf(s,"<%d> queueNum:%d portRate:%d Ifg include:%d queueBwCtrl:%s\n", 
				port, queueNum, pRate, pIfg_include, (qEnable==ENABLED)?"enabled":"disable");
		} else {
			seq_printf(s,"<EXT_PORT%d> queueNum:%d portRate:%d Ifg include:%d queueBwCtrl:%s\n", 
				(port-16), queueNum, pRate, pIfg_include, (qEnable==ENABLED)?"enabled":"disable");
		}
		rtk_rate_igrBandwidthCtrlRate_get(port, &inRate, &inIfg_include, &inFc_enable);
		seq_printf(s,"\tingressBw:%d Ifg include:%d flowControl:%d\n", 
			inRate, inIfg_include, inFc_enable);
		
		seq_printf(s,"\tQueue Parameters:\n ");
		for(queue=0; queue<RTK_MAX_NUM_OF_QUEUE; queue++)
		{
			rtk_rate_egrQueueBwCtrlRate_get(port, queue, &meterId); 
			 
			seq_printf(s,"\t[%d] type:%s, weight:%d, MeterId:%d\n", 
				queue, (qWeights.weights[queue]==0)?"SP":"WFQ", qWeights.weights[queue], meterId);
		}

		seq_printf(s,"\tMeter Parameters:\n ");
		if(port<4)
		{
			for(i=port*8; i<=port*8+7; i++)
			{			
				rtk_rate_shareMeter_get(i, &type, &qRate, &qIfg_include);
				rtk_rate_shareMeterBucket_get(i,&Bucket_size);
				if(i==port*8)
					seq_printf(s,"\t");
				if(i==port*8+4)
					seq_printf(s,"\n\t");
				seq_printf(s,"[%d]%d,%d,%d ", i, qRate, qIfg_include,Bucket_size);
			}
			seq_printf(s,"\n");
		}
		else
		{
			for(i=(port-4)*8; i<=(port-4)*8+7; i++)
			{	
				if(i==(port-4)*8)
					seq_printf(s,"\t");
				if(i==(port-4)*8+4)
					seq_printf(s,"\n\t");
				rtk_rate_shareMeter_get(i, &type, &qRate, &qIfg_include);
				rtk_rate_shareMeterBucket_get(i,&Bucket_size);
				seq_printf(s,"[%d]%d,%d,%d ", i, qRate, qIfg_include, Bucket_size);
			}
			seq_printf(s,"\n");
		}

	}
	return 0;
}
#else
int rtl_8367QosReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)	
{
	int len;
	
	int port, queueNum, i, queue;	
	rtk_qos_queue_weights_t qWeights;
	rtk_meter_id_t meterId;
	rtk_rate_t qRate, pRate;
	rtk_enable_t qIfg_include, pIfg_include, qEnable;
	rtk_uint32 Bucket_size;
	rtk_port_mac_ability_t mac_cfg;
    rtk_mode_ext_t mode ;
	rtk_rate_t inRate;
	rtk_enable_t inIfg_include, inFc_enable;

    rtk_port_macForceLinkExt_get(EXT_PORT_1,&mode,&mac_cfg);

	len = sprintf(page, "%s\n", "rtl8367 cpu flow control");
	len += sprintf(page+len, "\ttx pause:%d\n", mac_cfg.txpause);
	len += sprintf(page+len, "\trx pause:%d\n", mac_cfg.rxpause);
	len += sprintf(page+len, "%s\n", "rtl8367 qos related parameters");

	for(port=0; port<RTK_PORT_ID_MAX; port++)
	{
		rtk_qos_queueNum_get(port,&queueNum);
		rtk_rate_egrBandwidthCtrlRate_get(port, &pRate, &pIfg_include); 

		rtk_qos_schedulingQueue_get(port, &qWeights);
		rtk_rate_egrQueueBwCtrlEnable_get(port, RTK_WHOLE_SYSTEM, &qEnable); 

		len += sprintf(page+len,"<%d> queueNum:%d portRate:%d Ifg include:%d queueBwCtrl:%s\n", 
			port, queueNum, pRate, pIfg_include, (qEnable==ENABLED)?"enabled":"disable");
		rtk_rate_igrBandwidthCtrlRate_get(port, &inRate, &inIfg_include, &inFc_enable);
		len += sprintf(page+len,"\tingressBw:%d Ifg include:%d flowControl:%d\n", 
			inRate, inIfg_include, inFc_enable);

		len += sprintf(page+len,"\tQueue Parameters:\n ");
		for(queue=0; queue<RTK_MAX_NUM_OF_QUEUE; queue++)
		{
			rtk_rate_egrQueueBwCtrlRate_get(port, queue, &meterId); 
			 
			len += sprintf(page+len,"\t[%d] type:%s, weight:%d, MeterId:%d\n", 
				queue, (qWeights.weights[queue]==0)?"SP":"WFQ", qWeights.weights[queue], meterId);
		}

		len += sprintf(page+len,"\tMeter Parameters:\n ");
		if(port<4)
		{
			for(i=port*8; i<=port*8+7; i++)
			{			
				rtk_rate_shareMeter_get(i, &qRate, &qIfg_include);
				rtk_rate_shareMeterBucket_get(i,&Bucket_size);
				if(i==port*8)
					len += sprintf(page+len,"\t");
				if(i==port*8+4)
					len += sprintf(page+len,"\n\t");
				len += sprintf(page+len,"[%d]%d,%d,%d ", i, qRate, qIfg_include,Bucket_size);
			}
			len += sprintf(page+len,"\n");
		}
		else
		{
			for(i=(port-4)*8; i<=(port-4)*8+7; i++)
			{	
				if(i==(port-4)*8)
					len += sprintf(page+len,"\t");				
				if(i==(port-4)*8+4)
					len += sprintf(page+len,"\n\t");
				rtk_rate_shareMeter_get(i, &qRate, &qIfg_include);
				rtk_rate_shareMeterBucket_get(i,&Bucket_size);
				len += sprintf(page+len,"[%d]%d,%d,%d ", i, qRate, qIfg_include, Bucket_size);
			}
			len += sprintf(page+len,"\n");
		}

	}
				
	if (len <= off+count)
		*eof = 1;
			
	*start = page + off;
		len -= off;
			
	if (len>count)
		len = count;
			
	if (len<0) len = 0;
			
	return len;
}
#endif

int rtl_8367QosWriteProc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
#if 0
	char tmp[256];
		
	char		*strptr, *cmd_addr;
	char		*tokptr;
	int port, qid, page;
	unsigned int queue_page[RTK_MAX_NUM_OF_QUEUE];

	if (count < 2)
		return -EFAULT;
	
	if (buffer && !copy_from_user(tmp, buffer, count)) {
	
		tmp[count] = '\0';
		strptr=tmp;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		
		if (!memcmp(tokptr, "current", 7))
		{				
			printk( "Current Page for Egress Port and Queues\n");
			printk( "PortNo.  Port	 Q0 	Q1	   Q2	  Q3	 Q4 	Q5	   Q6	  Q7\n");
			for(port=0; port<RTK_PORT_ID_MAX; port++)
			{
				/*Egress Port page number*/
				rtl8367b_setAsicReg(RTL8367B_REG_FLOWCTRL_DEBUG_CTRL0,port);
				rtl8367b_getAsicReg(RTL8367B_REG_FLOWCTRL_PORT_PAGE_COUNT,&page);
				
				for(qid=0;qid<=7;qid++) 				   
				{						 
					rtl8367b_getAsicReg(RTL8367B_REG_FLOWCTRL_QUEUE0_PAGE_COUNT+qid,&queue_page[qid]);
				}
		
				printk("%2d		 %4d  %4d	%4d   %4d	%4d   %4d	%4d   %4d	%4d\n", port+1,page,
				queue_page[0],queue_page[1],queue_page[2],queue_page[3],queue_page[4],queue_page[5],queue_page[6],queue_page[7]);
			}
			
		}
		else if(!memcpy(tokptr, "max",3))
		{
			printk( "Maximum Page for Egress Port and Queues\n");
			printk( "PortNo.  Port	 Q0 	Q1	   Q2	  Q3	 Q4 	Q5	   Q6	  Q7\n");
			for(port=0; port<RTK_PORT_ID_MAX; port++)
			{
				/*Egress Port page number*/
				rtl8367b_setAsicReg(RTL8367B_REG_FLOWCTRL_DEBUG_CTRL0,port);
				rtl8367b_getAsicReg(RTL8367B_REG_FLOWCTRL_PORT_MAX_PAGE_COUNT,&page);
				 
				for(qid=0;qid<=7;qid++) 				   
				{						 
					rtl8367b_getAsicReg(RTL8367B_REG_FLOWCTRL_QUEUE0_MAX_PAGE_COUNT+qid,&queue_page[qid]);
				}
		
				printk("%2d		 %4d  %4d	%4d   %4d	%4d   %4d	%4d   %4d	%4d\n", port+1,page,
				queue_page[0],queue_page[1],queue_page[2],queue_page[3],queue_page[4],queue_page[5],queue_page[6],queue_page[7]);
		
			}
		}
		else
		{
errout:
			printk("error input!\n");
		}
	}
#endif
	return count;	
}
int rtl8367_qos_init()
{
    rtk_uint32 priority;
    rtk_api_ret_t retVal;
	rtk_queue_num_t queueNum;
	rtk_uint32 priDec;
	int i;

	//the priority to qid matrix should be equal to 97F/9xd...
	//change this matrix can't work!!!!??
	#if 0
	CONST_T rtk_uint16 g_prioritytToQid[8][8]= {
		{0,0,0,0,0,0,0,0}, 
		{0,0,0,0,5,5,5,5},	
		{0,0,0,0,1,1,5,5},
		{0,0,0,1,2,2,5,5},
		{0,0,0,1,2,3,5,5},
		{0,0,1,2,3,4,5,5},
		{0,0,1,2,3,4,5,6},
		{0,1,2,3,4,5,6,7}
		};
	#endif

	//802.1p priority based should be heighest priority
    CONST_T rtk_uint32 g_priorityDecision[8] = {0x01, 0x80,0x04,0x80,0x20,0x40,0x10,0x08};

	rtk_qos_init(1);
	#if 0
	 /*Set Priority to Qid*/
	for(queueNum = 0; priority <RTK_MAX_NUM_OF_QUEUE; priority++)
	{
	   	for (priority = 0; priority <= RTL8367B_PRIMAX; priority++)
	   	{
	      	if ((retVal = rtl8367b_setAsicPriorityToQIDMappingTable(queueNum, priority, g_prioritytToQid[queueNum][priority])) != RT_ERR_OK)
	         	return retVal;
	  	}

	}
	#endif

	/*Change Priority Decision Order*/
    for (priDec = 0;priDec < PRIDEC_END;priDec++)
    {
		if ((retVal = rtl83xx_setAsicPriorityDecision(PRIDECTBL_IDX0, priDec, g_priorityDecision[priDec])) != RT_ERR_OK)
            return retVal;
        if ((retVal = rtl83xx_setAsicPriorityDecision(PRIDECTBL_IDX1, priDec, g_priorityDecision[priDec])) != RT_ERR_OK)
            return retVal;
    }

	for(i=0; i<RTL83XX_METERNO; i++)
	{
		if(i==CPU_METER_ID1 || i==CPU_METER_ID2)
			qos_meter_owner[i] = CPU_METER;
		else
			qos_meter_owner[i] = UNKNOWN_OWNER_METER;
	}
}
#endif

#if defined CONFIG_RTL_8367_QOS_TEST
extern void rtl865x_qos_set(void);
int rtl8367_qos_test()
{
	int port, qid;
	rtk_priority_select_t PriDec;
	rtk_qos_queue_weights_t qweights;

	//97D 
	rtl865x_qos_set();
	
	//init qos
	rtl8367_qos_init();

	//set wan port queue num=2
	rtk_qos_queueNum_set(RTL83XX_PORT4_ENABLE_OFFSET, 2);
	//rtk_qos_queueNum_set(r8367_cpu_port, 2);

	//set port priority
	#if 0
	rtk_qos_portPri_set(0,0);
	rtk_qos_portPri_set(1,0);
	rtk_qos_portPri_set(2,7);
	rtk_qos_portPri_set(3,7);
	#endif

	//set queue parameter on CPU port
	//strict priority
	for (qid = 0; qid < RTK_MAX_NUM_OF_QUEUE; qid ++)
	{
		if(qid==0 || qid==7)
			qweights.weights[qid] = 0;
		else
			qweights.weights[qid] = 0;
	}
	#if 1
	rtk_qos_schedulingQueue_set(RTL83XX_PORT4_ENABLE_OFFSET,&qweights);	
	//rtk_rate_egrBandwidthCtrlRate_set(RTL8367B_PORT4_ENABLE_OFFSET, 204800, 1);
	
	//set queue 0 rate to 10M and queue 7 rate to 5M
	rtk_rate_shareMeter_set(0, METER_TYPE_KBPS, 15360, ENABLE);
	rtk_rate_shareMeter_set(1, METER_TYPE_KBPS, 10240, ENABLE);
	rtk_rate_shareMeterBucket_set(0,10000);
	rtk_rate_shareMeterBucket_set(1,30000);
	rtk_rate_egrQueueBwCtrlEnable_set(RTL83XX_PORT4_ENABLE_OFFSET,0xFF,ENABLE);
	rtk_rate_egrQueueBwCtrlRate_set(RTL83XX_PORT4_ENABLE_OFFSET,0,0);
	rtk_rate_egrQueueBwCtrlRate_set(RTL83XX_PORT4_ENABLE_OFFSET,7,1);
	#else
	rtk_qos_schedulingQueue_set(r8367_cpu_port,&qweights);	
	rtk_rate_egrBandwidthCtrlRate_set(r8367_cpu_port, 204800, 1);
	
	//set queue 0 rate to 10M and queue 7 rate to 5M
	rtk_rate_shareMeter_set(16, 204800, ENABLE);
	rtk_rate_shareMeter_set(17, 153600, ENABLE);
	rtk_rate_shareMeterBucket_set(16,10000);
	rtk_rate_shareMeterBucket_set(17,30000);
	rtk_rate_egrQueueBwCtrlEnable_set(r8367_cpu_port,0xFF,ENABLE);
	rtk_rate_egrQueueBwCtrlRate_set(r8367_cpu_port,0,16);
	rtk_rate_egrQueueBwCtrlRate_set(r8367_cpu_port,7,17);
	#endif

}
#endif

#if defined CONFIG_RTL_8367_QOS_SUPPORT
int rtl8367_qosSetIngressBandwidth(unsigned int memberPort, unsigned int Kbps)
{
	unsigned int	port;

	//todo: wait output queue empty?
	for(port=0;port<MAX_PHY_PORT_NUM;port++)
	{
		if(((1<<port)&memberPort)==0)
			continue;
		if(Kbps==0)
			rtk_rate_igrBandwidthCtrlRate_set(port, RTL83XX_QOS_RATE_INPUT_MAX, DISABLED, ENABLED);
		else
			rtk_rate_igrBandwidthCtrlRate_set(port, Kbps, ENABLED, ENABLED);		
	}
	return 0;

}

int rtl8367_qosSetBandwidth(unsigned int memberPort, unsigned int Kbps)
{
	unsigned int	port;
	int ret;

	//todo: wait output queue empty?
	for(port=0;port<MAX_PHY_PORT_NUM;port++)
	{
		if(((1<<port)&memberPort)==0)
			continue;
		ret=rtk_rate_egrBandwidthCtrlRate_set(port, Kbps, ENABLED);
	}
	return 0;
}
int rtl8367_qosFlushBandwidth(unsigned int memberPort)
{
	unsigned int port;

	//todo: wait output queue empty?
	for(port=0;port<MAX_PHY_PORT_NUM;port++)
	{
		if(((1<<port)&memberPort)==0)
			continue;
		rtk_rate_egrBandwidthCtrlRate_set(port, RTL83XX_QOS_RATE_INPUT_MAX, ENABLED);
	}
	return 0;
}

int rtl8367_qosGetMeterId(unsigned int port, unsigned int bw, unsigned int wanPortMask)
{
	rtk_meter_id_t idbegin, idend;
	rtk_meter_id_t i;
	rtk_rate_t Rate;
	rtk_data_t Ifg_include;
	
	if(port<0 || port>=MAX_PHY_PORT_NUM)
		return -1;

	if(port<4)
	{
		idbegin = port*8;
		idend = port*8+7;
	}
	else
	{
		idbegin = (port-4)*8;
		idend = (port-4)*8+7;
	}

	for(i=idbegin; i<=idend; i++)
	{
		if(qos_meter_owner[i] == UNKNOWN_OWNER_METER)
			break;
	}
	if(i > idend)
	{
		return -1;
	}

	//printk("get meter of port %d, id:%d, bw:%d, [%s:%d]\n", port, i, bw, __FUNCTION__, __LINE__);
	if(wanPortMask & (1<<port))
	{
		qos_meter_owner[i]=WAN_METER;
	}
	else
	{
		qos_meter_owner[i]=LAN_METER;
	}

	rtk_rate_shareMeter_set(i, METER_TYPE_KBPS, bw, ENABLED);
	//rtk_rate_shareMeterBucket_set(j,QOS_BUCKET_SIZE);

	return i;
		
}
//ok
int rtl8367_qosFlushMeter(int port, unsigned int wanPortMask)
{
	int meterId,idbegin,idend;

	if(port<0 || port>=MAX_PHY_PORT_NUM)
		return -1;
	
	if(port<4)
	{
		idbegin = port*8;
		idend = port*8+7;
	}
	else
	{
		idbegin = (port-4)*8;
		idend = (port-4)*8+7;
	}

	//printk("flush meter of port %d, [%s:%d]\n", port, __FUNCTION__, __LINE__);

	for(meterId=idbegin; meterId<=idend; meterId++)
	{
		if(((wanPortMask & (1<<port)) && (qos_meter_owner[meterId]==WAN_METER)) 
			||(!(wanPortMask & (1<<port)) && (qos_meter_owner[meterId]==LAN_METER)))
		{
			rtk_rate_shareMeter_set(meterId, METER_TYPE_KBPS, RTL83XX_QOS_RATE_INPUT_MAX, DISABLED);
			qos_meter_owner[meterId]=UNKNOWN_OWNER_METER;
			//always use default bucket size
			//rtk_rate_shareMeterBucket_set(meterId,DEFAULT_QOS_BUCKET_SIZE);
		}
	}
	return 0;
}

int rtl8367_qosProcessQueue(unsigned int memberPort, unsigned int queueNum, unsigned int *queueFlag, unsigned int *queueId, unsigned int *queueBw, unsigned int *queueWeight, unsigned int wanPortMask)
{
	unsigned int port, queue, qid, meterId;
	rtk_qos_queue_weights_t qweights;
	unsigned int set_error=0;
	unsigned int all_phyPortMask = vutpportmask;

	memset(&qweights, 0, sizeof(qweights));
	
	for(port = 0; port < MAX_PHY_PORT_NUM; port++)
	{
		if(((1<<port)&memberPort)==0)
			continue;
		rtk_qos_queueNum_set(port, queueNum);
		rtk_rate_egrQueueBwCtrlEnable_set(port,RTK_WHOLE_SYSTEM, ENABLED);
		
		for (queue=0;queue<MAX_RTL8367_QOS_QUEUE_NUM;queue++)
		{
			if((queueFlag[queue]&QOS_VALID_MASK)==0 || queueBw[queue] == 0)
				continue;

			qid = queueId[queue];
			if((queueFlag[queue]&QOS_TYPE_MASK)==QOS_TYPE_STR)
			{
				qweights.weights[qid] = 0;
			}
			else
			{
				qweights.weights[qid] = queueWeight[queue];
			}

			meterId = rtl8367_qosGetMeterId(port, queueBw[queue], wanPortMask);

			if(meterId != -1)
			{
				rtk_rate_egrQueueBwCtrlRate_set(port,qid,meterId);
			}
			else
			{
				set_error = 1;
			}
				
		}
		rtk_qos_schedulingQueue_set(port,&qweights);
	}
	if(set_error)
	{
		printk("sharemeter is not enough!\n");
		rtl8367_qosFlushBandwidth(all_phyPortMask);
		rtl8367_closeQos(all_phyPortMask, wanPortMask);
	}
	return 0;
}
//ok
int rtl8367_closeQos(unsigned int memberPort, unsigned int wanPortMask)
{
	unsigned int port, qid;	
	rtk_qos_queue_weights_t qweights;

	//todo: wait output queue empty?
	for(port=0;port<MAX_PHY_PORT_NUM;port++)
	{
		if(((1<<port)&memberPort)==0)
			continue;

		
		for (qid = 0; qid < RTK_MAX_NUM_OF_QUEUE; qid ++)
		{
			qweights.weights[qid] = 0;
			if(port<4)
				rtk_rate_egrQueueBwCtrlRate_set(port, qid, port*8);
			else
				rtk_rate_egrQueueBwCtrlRate_set(port, qid, (port-4)*8);
		}

		rtk_qos_queueNum_set(port, 1);
		rtk_qos_schedulingQueue_set(port,&qweights);		
		rtk_rate_egrQueueBwCtrlEnable_set(port,RTK_WHOLE_SYSTEM,DISABLED);
		rtl8367_qosFlushMeter(port, wanPortMask);
	}

	//close 8367 cpu port flow control
	

	return 0;
	
}
void rtl8367_setFlowControl(int qosEnable)
{
    /* Set external interface 0 to RGMII with Force mode, 1000M, Full-duple, enable TX&RX pause*/
    rtk_port_mac_ability_t mac_cfg;
    rtk_mode_ext_t mode ;

    rtk_port_macForceLinkExt_get(EXT_PORT_FOR_HOST,&mode,&mac_cfg);
	if(((mac_cfg.txpause==ENABLED) || (mac_cfg.rxpause==ENABLED)) && qosEnable==1)
	{
		mac_cfg.txpause = DISABLED;
		mac_cfg.rxpause = DISABLED;
		rtk_port_macForceLinkExt_set(EXT_PORT_FOR_HOST,mode,&mac_cfg);
	}
	else if(((mac_cfg.txpause==DISABLED) || (mac_cfg.rxpause==DISABLED)) && qosEnable==0)
	{
		mac_cfg.txpause = ENABLED;
		mac_cfg.rxpause = ENABLED;		
		rtk_port_macForceLinkExt_set(EXT_PORT_FOR_HOST,mode,&mac_cfg);
	}
}
#endif
#endif

#if defined(CONFIG_RTL_VLAN_8021Q) || defined(CONFIG_RTL_HW_VLAN_SUPPORT) || defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
int rtl865x_enableRtl8367BCMCToCpu(unsigned int acl_idx)
{
    int retVal;
    rtk_filter_field_t  filter_field[2];
    rtk_filter_cfg_t    cfg;
    rtk_filter_action_t act;
    rtk_filter_number_t ruleNum = 0;

    
    memset(filter_field, 0, 2*sizeof(rtk_filter_field_t));
    memset(&cfg, 0, sizeof(rtk_filter_cfg_t));
    memset(&act, 0, sizeof(rtk_filter_action_t));

    filter_field[0].fieldType = FILTER_FIELD_DMAC;
    filter_field[0].filter_pattern_union.dmac.dataType = FILTER_FIELD_DATA_MASK;
    filter_field[0].filter_pattern_union.dmac.value.octet[0] = 0x01;
    
    filter_field[0].filter_pattern_union.dmac.mask.octet[0] = 0x1;
    filter_field[0].next = NULL;
    if ((retVal = rtk_filter_igrAcl_field_add(&cfg, &filter_field[0])) != RT_ERR_OK)
        return retVal;
    
    
    cfg.activeport.value.bits[0] = vutpportmask;
    cfg.activeport.mask.bits[0] = vportmask;
    cfg.invert = FALSE;

    act.actEnable[FILTER_ENACT_TRAP_CPU] = TRUE;

    if ((retVal = rtk_filter_igrAcl_cfg_add(1, &cfg, &act, &ruleNum)) != RT_ERR_OK)
        return retVal;

    //printk("%s %d BC MC to cpu !\n", __func__, __LINE__);
    return RT_ERR_OK;

}

int rtl865x_disableRtl8367BCMCToCpu(unsigned int acl_idx)
{
    int ret = -1;
    
    ret = rtk_filter_igrAcl_cfg_del(acl_idx);

    return ret;
}
#endif

void rtl_get_83xx_snr(void)
{
	int i, phy_id;
	unsigned long flags;
	rtk_port_mac_ability_t sts;
	unsigned int sum_snr_a = 0, sum_snr_b = 0, sum_snr_c = 0, sum_snr_d = 0, snr_tmp=0;

	local_irq_save(flags);
	for (phy_id=0; phy_id <=RTK_PHY_ID_MAX; phy_id++)
	{
		memset(&sts, 0, sizeof(rtk_port_mac_ability_t));
		if ((rtk_port_macStatus_get(phy_id, &sts)==0) && (sts.link==1))
		{
			for (i=0; i<10; i++)
			{
				if (rtl8367c_getAsicPHYOCPReg(phy_id, 0xA8C0, &snr_tmp) == RT_ERR_OK) {
					sum_snr_a += snr_tmp;
				} else {
					printk("%s[%d], rtl8367b_getAsicPHYOCPReg() Failed\n", __FUNCTION__, __LINE__);
					local_irq_restore(flags);
					return;
				}

				if (rtl8367c_getAsicPHYOCPReg(phy_id, 0xA9C0, &snr_tmp) == RT_ERR_OK) {
					sum_snr_b += snr_tmp;
				} else {
					printk("%s[%d], rtl8367b_getAsicPHYOCPReg() Failed\n", __FUNCTION__, __LINE__);
					local_irq_restore(flags);
					return;
				}

				if (rtl8367c_getAsicPHYOCPReg(phy_id, 0xAAC0, &snr_tmp) == RT_ERR_OK) {
					sum_snr_c += snr_tmp;
				} else {
					printk("%s[%d], rtl8367b_getAsicPHYOCPReg() Failed\n", __FUNCTION__, __LINE__);
					local_irq_restore(flags);
					return;
				}

				if (rtl8367c_getAsicPHYOCPReg(phy_id, 0xABC0, &snr_tmp) == RT_ERR_OK) {
					sum_snr_d += snr_tmp;
				} else {
					printk("%s[%d], rtl8367b_getAsicPHYOCPReg() Failed\n", __FUNCTION__, __LINE__);
					local_irq_restore(flags);
					return;
				}
			}

			sum_snr_a = sum_snr_a/10;
			sum_snr_b = sum_snr_b/10;
			sum_snr_c = sum_snr_c/10;
			sum_snr_d = sum_snr_d/10;

			printk("Port[%d] link speed is %s, CH_A_SNR is %d, CH_B_SNR is %d, CH_C_SNR is %d, CH_D_SNR is %d\n", 
				    phy_id, (sts.speed)==0x1?"100M":((sts.speed)==0x2?"1G": ((sts.speed) ==0x0?"10M":"Unkown")), 
				    sum_snr_a, sum_snr_b, sum_snr_c, sum_snr_d);
		}
	}
	printk("\n## NOTE: snr value translating to dB value is by: -(10 * log10(snr/pow(2,18)))\n");

	local_irq_restore(flags);
	return;
}

