/*
 *  Routines to access hardware
 *
 *  $Id: 8192cd_hw.c,v 1.107.2.43 2011/01/17 13:21:01 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_HW_C_
#define _printk printk

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <asm/unistd.h>
#include <linux/gpio.h>
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#endif

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"
#ifdef CONFIG_RTL_88E_SUPPORT
#include "Hal8188EPwrSeq.h"
#endif
#ifdef CONFIG_RTL_8723B_SUPPORT
#include "Hal8723BPwrSeq.h"
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
#include "Hal8812PwrSeq.h"
#endif

#ifdef CONFIG_WLAN_HAL_8814AE
#include "./phydm/rtl8814a/phydm_rtl8814a.h"
#endif

#ifdef CONFIG_WLAN_HAL_8197F
#include "./efuse.h"
#endif

#ifdef __KERNEL__
#ifdef __LINUX_2_6__
#include <linux/syscalls.h>
#else
#include <linux/fs.h>
#endif
#endif

#if defined(USE_RTL8186_SDK) && !defined(__OSK__)
#ifdef CONFIG_RTL8672
	#ifdef USE_RLX_BSP
	#include <bspchip.h>	
	#ifdef __LINUX_3_10__
	#include <bspgpio.h>
	#else
	#include <gpio.h>
	#endif

	#ifdef CONFIG_RTL_8196C
	#undef CONFIG_RTL_8196C
	#endif
	#ifdef CONFIG_RTL8196C_REVISION_B
	#undef CONFIG_RTL8196C_REVISION_B
	#endif

	#else
	#include <platform.h>
	#include "../../../arch/mips/realtek/rtl8672/gpio.h"
	#endif
#else

#if defined(__LINUX_2_6__)
#if (defined(CONFIG_OPENWRT_SDK) && !defined(CONFIG_ARCH_CPU_RLX)) || defined(CONFIG_RTL_8197F)
#include <bspchip.h>
#else
#include <bsp/bspchip.h>
#endif //CONFIG_OPENWRT_SDK
#else
#ifndef __ECOS
#include <asm/rtl865x/platform.h>
#endif
#endif
#endif

#endif // USE_RTL8186_SDK

#if defined(CONFIG_WLAN_HAL_8814AE)
#define MAX_CONFIG_FILE_SIZE (64*1024) // for 8814, added to 64k
#else
#define MAX_CONFIG_FILE_SIZE (20*1024)
#endif

int rtl8192cd_fileopen(const char *filename, int flags, int mode);
void selectMinPowerIdex(struct rtl8192cd_priv *priv);
void PHY_RF6052SetOFDMTxPower(struct rtl8192cd_priv *priv, unsigned int channel);
void PHY_RF6052SetCCKTxPower(struct rtl8192cd_priv *priv, unsigned int channel);
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT) || (defined(CONFIG_RTL_88E_SUPPORT) && defined(__KERNEL__) && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
static void rtl8192cd_ReadFwHdr(struct rtl8192cd_priv * priv);
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
static int Load_92C_Firmware(struct rtl8192cd_priv *priv);
#endif



#define VAR_MAPPING(dst,src) \
unsigned char *data_##dst##_start = &data_##src[0]; \
unsigned char *data_##dst##_end   = &data_##src[sizeof(data_##src)]; \
 
#ifdef CONFIG_RTL_92D_SUPPORT
#include "data_PHY_REG_n.c"
#include "data_AGC_TAB_n.c"
#include "data_AGC_TAB_2G_n.c"
#include "data_AGC_TAB_5G_n.c"
#include "data_radio_a_n.c"
#include "data_radio_b_n.c"
#ifdef RTL8192D_INT_PA
#ifdef USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
#include "data_radio_a_intPA_GM_new.c"
#include "data_radio_b_intPA_GM_new.c"
#elif defined (RTL8192D_INT_PA_GAIN_TABLE_NEW1)
#include "data_radio_a_intPA_GM_new1.c"
#include "data_radio_b_intPA_GM_new1.c"
#else
#include "data_radio_a_intPA_GM.c"
#include "data_radio_b_intPA_GM.c"
#endif

#else // USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
#include "data_radio_a_intPA_new.c"
#include "data_radio_b_intPA_new.c"
#else
#include "data_radio_a_intPA.c"
#include "data_radio_b_intPA.c"
#endif

#endif // USB_POWER_SUPPORT
#endif // RTL8192D_INT_PA
//_TXPWR_REDEFINE
#ifdef HIGH_POWER_EXT_PA
#include "data_AGC_TAB_n_92d_hp.c"
#include "data_PHY_REG_n_92d_hp.c"
#include "data_radio_a_n_92d_hp.c"
#include "data_radio_b_n_92d_hp.c"
#include "data_PHY_REG_PG_92d_hp.c"
#endif
#include "data_PHY_REG_PG.c"
#include "data_PHY_REG_PG_FCC.c"
#include "data_PHY_REG_PG_CE.c"

#ifdef _TRACKING_TABLE_FILE
#include "data_REG_TXPWR_TRK_n_92d.c"
#include "data_REG_TXPWR_TRK_n_92d_hp.c"
#endif
#include "data_PHY_REG_MP_n.c"
#include "data_MACPHY_REG.c"
#include "data_rtl8192dfw_n.c"
VAR_MAPPING(PHY_REG_n, PHY_REG_n);
VAR_MAPPING(AGC_TAB_n, AGC_TAB_n);
VAR_MAPPING(AGC_TAB_2G_n, AGC_TAB_2G_n);
VAR_MAPPING(AGC_TAB_5G_n, AGC_TAB_5G_n);
VAR_MAPPING(radio_a_n, radio_a_n);
VAR_MAPPING(radio_b_n, radio_b_n);
#ifdef RTL8192D_INT_PA
#ifdef USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
VAR_MAPPING(radio_a_intPA_GM_new, radio_a_intPA_GM_new);
VAR_MAPPING(radio_b_intPA_GM_new, radio_b_intPA_GM_new);
#elif defined (RTL8192D_INT_PA_GAIN_TABLE_NEW1)
VAR_MAPPING(radio_a_intPA_GM_new1, radio_a_intPA_GM_new1);
VAR_MAPPING(radio_b_intPA_GM_new1, radio_b_intPA_GM_new1);
#else
VAR_MAPPING(radio_a_intPA_GM, radio_a_intPA_GM);
VAR_MAPPING(radio_b_intPA_GM, radio_b_intPA_GM);
#endif

#else // USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
VAR_MAPPING(radio_a_intPA_new, radio_a_intPA_new);
VAR_MAPPING(radio_b_intPA_new, radio_b_intPA_new);
#else
VAR_MAPPING(radio_a_intPA, radio_a_intPA);
VAR_MAPPING(radio_b_intPA, radio_b_intPA);
#endif

#endif // USB_POWER_SUPPORT
#endif // RTL8192D_INT_PA
//_TXPWR_REDEFINE
#ifdef HIGH_POWER_EXT_PA
VAR_MAPPING(AGC_TAB_n_92d_hp, AGC_TAB_n_92d_hp);
VAR_MAPPING(PHY_REG_n_92d_hp, PHY_REG_n_92d_hp);
VAR_MAPPING(radio_a_n_92d_hp, radio_a_n_92d_hp);
VAR_MAPPING(radio_b_n_92d_hp, radio_b_n_92d_hp);
VAR_MAPPING(PHY_REG_PG_92d_hp, PHY_REG_PG_92d_hp);
#endif
VAR_MAPPING(PHY_REG_PG, PHY_REG_PG);
VAR_MAPPING(PHY_REG_PG_FCC, PHY_REG_PG_FCC);
VAR_MAPPING(PHY_REG_PG_CE, PHY_REG_PG_CE);
#ifdef TXPWR_LMT 
#include "TXPWR_92D.h"
#endif // TXPWR_LMT
#ifdef _TRACKING_TABLE_FILE
VAR_MAPPING(REG_TXPWR_TRK_n_92d, REG_TXPWR_TRK_n_92d);
VAR_MAPPING(REG_TXPWR_TRK_n_92d_hp, REG_TXPWR_TRK_n_92d_hp);
#endif
VAR_MAPPING(PHY_REG_MP_n, PHY_REG_MP_n);
VAR_MAPPING(MACPHY_REG, MACPHY_REG);
VAR_MAPPING(rtl8192dfw_n, rtl8192dfw_n);
#endif // CONFIG_RTL_92D_SUPPORT

#ifdef CONFIG_RTL_92C_SUPPORT

#ifdef TESTCHIP_SUPPORT
#include "data_AGC_TAB.c"
#include "data_PHY_REG_1T.c"
#include "data_PHY_REG_2T.c"
#include "data_radio_a_1T.c"
#include "data_radio_a_2T.c"
#include "data_radio_b_2T.c"
#include "data_rtl8192cfw.c"
#endif

#include "data_AGC_TAB_n_92C.c"
#include "data_PHY_REG_1T_n.c"
#include "data_PHY_REG_2T_n.c"
#include "data_PHY_REG_2T_n_lna.c"
#include "data_radio_a_2T_n.c"
#include "data_radio_b_2T_n.c"
#include "data_radio_a_1T_n.c"
#include "data_rtl8192cfwn.c"
#include "data_rtl8192cfwua.c"


#include "data_PHY_REG_PG_92C.c"
#include "data_MACPHY_REG_92C.c"
#include "data_PHY_REG_MP_n_92C.c"

#include "data_AGC_TAB_n_hp.c"
#include "data_PHY_REG_2T_n_hp.c"
#include "data_radio_a_2T_n_lna.c"
#include "data_radio_b_2T_n_lna.c"
#include "data_PHY_REG_1T_n_hp.c"

#ifdef HIGH_POWER_EXT_PA
#include "data_radio_a_2T_n_hp.c"
#include "data_radio_b_2T_n_hp.c"
#include "data_PHY_REG_PG_hp.c"
#endif

#ifdef _TRACKING_TABLE_FILE
#include "data_REG_TXPWR_TRK.c"
#include "data_REG_TXPWR_TRK_hp.c"
#endif

#ifdef TXPWR_LMT 
#include "TXPWR_92C.h"
#endif // TXPWR_LMT




#define VAR_MAPPING(dst,src) \
	unsigned char *data_##dst##_start = &data_##src[0]; \
	unsigned char *data_##dst##_end   = &data_##src[sizeof(data_##src)]; \
 
#ifdef TESTCHIP_SUPPORT
VAR_MAPPING(AGC_TAB, AGC_TAB);
VAR_MAPPING(PHY_REG_1T, PHY_REG_1T);
VAR_MAPPING(PHY_REG_2T, PHY_REG_2T);
VAR_MAPPING(radio_a_1T, radio_a_1T);
VAR_MAPPING(radio_a_2T, radio_a_2T);
VAR_MAPPING(radio_b_2T, radio_b_2T);
VAR_MAPPING(rtl8192cfw, rtl8192cfw);
#endif

VAR_MAPPING(AGC_TAB_n_92C, AGC_TAB_n_92C);
VAR_MAPPING(PHY_REG_1T_n, PHY_REG_1T_n);
VAR_MAPPING(PHY_REG_2T_n, PHY_REG_2T_n);
VAR_MAPPING(PHY_REG_2T_n_lna, PHY_REG_2T_n_lna);
VAR_MAPPING(radio_a_1T_n, radio_a_1T_n);
VAR_MAPPING(radio_a_2T_n, radio_a_2T_n);
VAR_MAPPING(radio_b_2T_n, radio_b_2T_n);
VAR_MAPPING(rtl8192cfw_n, rtl8192cfwn);
VAR_MAPPING(rtl8192cfw_ua, rtl8192cfwua);

VAR_MAPPING(MACPHY_REG_92C, MACPHY_REG_92C);
VAR_MAPPING(PHY_REG_PG_92C, PHY_REG_PG_92C);
VAR_MAPPING(PHY_REG_MP_n_92C, PHY_REG_MP_n_92C);

VAR_MAPPING(AGC_TAB_n_hp, AGC_TAB_n_hp);
VAR_MAPPING(PHY_REG_2T_n_hp, PHY_REG_2T_n_hp);
VAR_MAPPING(PHY_REG_1T_n_hp, PHY_REG_1T_n_hp);
VAR_MAPPING(radio_a_2T_n_lna, radio_a_2T_n_lna);
VAR_MAPPING(radio_b_2T_n_lna, radio_b_2T_n_lna);

#ifdef HIGH_POWER_EXT_PA
VAR_MAPPING(radio_a_2T_n_hp, radio_a_2T_n_hp);
VAR_MAPPING(radio_b_2T_n_hp, radio_b_2T_n_hp);
VAR_MAPPING(PHY_REG_PG_hp, PHY_REG_PG_hp);
#endif

#ifdef _TRACKING_TABLE_FILE
VAR_MAPPING(REG_TXPWR_TRK, REG_TXPWR_TRK);
VAR_MAPPING(REG_TXPWR_TRK_hp, REG_TXPWR_TRK_hp);
#endif

#endif // CONFIG_RTL_92C_SUPPORT

#ifdef CONFIG_RTL_88E_SUPPORT
#include "data_PHY_REG_PG_88E.c"

VAR_MAPPING(PHY_REG_PG_88E, PHY_REG_PG_88E);

#include "data_PHY_REG_PG_88E_new.c"
VAR_MAPPING(PHY_REG_PG_88E_new, PHY_REG_PG_88E_new);
#ifdef TXPWR_LMT_88E
#include "data_TXPWR_LMT_88E_new.c"
VAR_MAPPING(TXPWR_LMT_88E_new, TXPWR_LMT_88E_new);
#endif

#endif // CONFIG_RTL_88E_SUPPORT

#ifdef CONFIG_RTL_8812_SUPPORT
#include "data_AGC_TAB_8812.c"
#include "data_MAC_REG_8812.c"
#include "data_PHY_REG_8812.c"
#include "data_PHY_REG_MP_8812.c"
#include "data_PHY_REG_PG_8812.c"
#include "data_RadioA_8812.c"
#include "data_RadioB_8812.c"
#include "data_rtl8812fw.c"
#ifdef _TRACKING_TABLE_FILE
#include "data_REG_TXPWR_TRK_8812.c"
#ifdef HIGH_POWER_EXT_PA
#include "data_REG_TXPWR_TRK_8812_hp.c"
#include "data_PHY_REG_PG_8812_hp.c"
VAR_MAPPING(PHY_REG_PG_8812_hp, PHY_REG_PG_8812_hp);
#endif

#include "data_PHY_REG_PG_8812_new.c"
VAR_MAPPING(PHY_REG_PG_8812_new, PHY_REG_PG_8812_new);
#ifdef TXPWR_LMT_8812
#include "data_TXPWR_LMT_8812_new.c"
VAR_MAPPING(TXPWR_LMT_8812_new, TXPWR_LMT_8812_new);
#endif

#endif

VAR_MAPPING(rtl8812fw, rtl8812fw);
VAR_MAPPING(AGC_TAB_8812, AGC_TAB_8812);
VAR_MAPPING(MAC_REG_8812, MAC_REG_8812);
VAR_MAPPING(PHY_REG_8812, PHY_REG_8812);
VAR_MAPPING(PHY_REG_MP_8812, PHY_REG_MP_8812);
VAR_MAPPING(PHY_REG_PG_8812, PHY_REG_PG_8812);
VAR_MAPPING(RadioA_8812, RadioA_8812);
VAR_MAPPING(RadioB_8812, RadioB_8812);
#ifdef _TRACKING_TABLE_FILE
VAR_MAPPING(REG_TXPWR_TRK_8812, REG_TXPWR_TRK_8812);
#ifdef HIGH_POWER_EXT_PA
VAR_MAPPING(REG_TXPWR_TRK_8812_hp, REG_TXPWR_TRK_8812_hp);
#endif
#endif

//FOR_8812_MP_CHIP
#include "data_MAC_REG_8812_n.c"
#include "data_AGC_TAB_8812_n_default.c"
#include "data_PHY_REG_8812_n_default.c"
#include "data_RadioA_8812_n_default.c"
#include "data_RadioB_8812_n_default.c"
#include "data_rtl8812fw_n.c"
#ifdef AC2G_256QAM
#include "data_rtl8812fw_n_2g.c"
VAR_MAPPING(rtl8812fw_n_2g, rtl8812fw_n_2g);
#endif
VAR_MAPPING(MAC_REG_8812_n, MAC_REG_8812_n);
VAR_MAPPING(rtl8812fw_n, rtl8812fw_n);
VAR_MAPPING(AGC_TAB_8812_n_default, AGC_TAB_8812_n_default);
VAR_MAPPING(PHY_REG_8812_n_default, PHY_REG_8812_n_default);
VAR_MAPPING(RadioA_8812_n_default, RadioA_8812_n_default);
VAR_MAPPING(RadioB_8812_n_default, RadioB_8812_n_default);
#include "data_AGC_TAB_8812_n_extlna.c"
#include "data_PHY_REG_8812_n_extlna.c"
#include "data_RadioA_8812_n_extlna.c"
#include "data_RadioB_8812_n_extlna.c"
VAR_MAPPING(AGC_TAB_8812_n_extlna, AGC_TAB_8812_n_extlna);
VAR_MAPPING(PHY_REG_8812_n_extlna, PHY_REG_8812_n_extlna);
VAR_MAPPING(RadioA_8812_n_extlna, RadioA_8812_n_extlna);
VAR_MAPPING(RadioB_8812_n_extlna, RadioB_8812_n_extlna);
#include "data_AGC_TAB_8812_n_extpa.c"
#include "data_PHY_REG_8812_n_extpa.c"
#include "data_RadioA_8812_n_extpa.c"
#include "data_RadioB_8812_n_extpa.c"
VAR_MAPPING(AGC_TAB_8812_n_extpa, AGC_TAB_8812_n_extpa);
VAR_MAPPING(PHY_REG_8812_n_extpa, PHY_REG_8812_n_extpa);
VAR_MAPPING(RadioA_8812_n_extpa, RadioA_8812_n_extpa);
VAR_MAPPING(RadioB_8812_n_extpa, RadioB_8812_n_extpa);

#ifdef HIGH_POWER_EXT_PA //FOR_8812_HP
#include "data_AGC_TAB_8812_hp.c"
#include "data_RadioA_8812_hp.c"
#include "data_RadioB_8812_hp.c"
VAR_MAPPING(AGC_TAB_8812_hp, AGC_TAB_8812_hp);
VAR_MAPPING(RadioA_8812_hp, RadioA_8812_hp);
VAR_MAPPING(RadioB_8812_hp, RadioB_8812_hp);

//FOR_8812_MP_CHIP
#include "data_AGC_TAB_8812_n_hp.c"
#include "data_PHY_REG_8812_n_hp.c"
#include "data_RadioA_8812_n_hp.c"
#include "data_RadioB_8812_n_hp.c"
#include "data_RadioA_8812_n_ultra_hp.c"
#include "data_RadioB_8812_n_ultra_hp.c"

VAR_MAPPING(AGC_TAB_8812_n_hp, AGC_TAB_8812_n_hp);
VAR_MAPPING(PHY_REG_8812_n_hp, PHY_REG_8812_n_hp);
VAR_MAPPING(RadioA_8812_n_hp, RadioA_8812_n_hp);
VAR_MAPPING(RadioB_8812_n_hp, RadioB_8812_n_hp);
VAR_MAPPING(RadioA_8812_n_ultra_hp, RadioA_8812_n_ultra_hp);
VAR_MAPPING(RadioB_8812_n_ultra_hp, RadioB_8812_n_ultra_hp);

#endif
#endif

#ifdef	CONFIG_RTL_8723B_SUPPORT
#include "data_TXPWR_LMT_8723.c"
#include "data_PHY_REG_PG_8723.c"
#include "data_rtl8723bfw.c"
#include "data_TxPowerTrack_AP_8723.c"

VAR_MAPPING(TXPWR_LMT_8723, TXPWR_LMT_8723);
VAR_MAPPING(PHY_REG_PG_8723, PHY_REG_PG_8723); 
VAR_MAPPING(rtl8723bfw, rtl8723bfw);
VAR_MAPPING(TxPowerTrack_AP_8723, TxPowerTrack_AP_8723);
#endif

/*-----------------------------------------------------------------------------
 * Function:	PHYCheckIsLegalRfPath8192cPci()
 *
 * Overview:	Check different RF type to execute legal judgement. If RF Path is illegal
 *			We will return false.
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	11/15/2007	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
int PHYCheckIsLegalRfPath8192cPci(struct rtl8192cd_priv *priv, unsigned int eRFPath)
{
	unsigned int rtValue = TRUE;

	if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
		if ((eRFPath == RF92CD_PATH_A) || (eRFPath == RF92CD_PATH_B))
			rtValue = TRUE;
		else
			rtValue = FALSE;
	} else if (get_rf_mimo_mode(priv) == MIMO_1T1R) {
		if (eRFPath == RF92CD_PATH_A)
			rtValue = TRUE;
		else
			rtValue = FALSE;
	} else {
		rtValue = FALSE;
	}

	return rtValue;
}
#if defined(CONFIG_RTL_8196CS)
void setBaseAddressRegister(void)
{
	int tmp32 = 0, status;
	while (++tmp32 < 100) {
		REG32(0xb8b00004) = 0x00100007;
		REG32(0xb8b10004) = 0x00100007;
		REG32(0xb8b10010) = 0x18c00001;
		REG32(0xb8b10018) = 0x19000004;
		status = (REG32(0xb8b10010) ^ 0x18c00001) | ( REG32(0xb8b10018) ^ 0x19000004);
		if (!status)
			break;
		else {
			printk("set BAR fail,%x\n", status);
			printk("%x %x %x %x \n",
				   REG32(0xb8b00004) , REG32(0xb8b10004) , REG32(0xb8b10010),  REG32(0xb8b10018) );
		}
	} ;
}
#endif
/**
* Function:	phy_CalculateBitShift
*
* OverView:	Get shifted position of the BitMask
*
* Input:
*			u4Byte		BitMask,
*
* Output:	none
* Return:		u4Byte		Return the shift bit bit position of the mask
*/
unsigned int phy_CalculateBitShift(unsigned int BitMask)
{
	unsigned int i;

	for (i = 0; i <= 31; i++) {
		if (((BitMask >> i) & 0x1) == 1)
			break;
	}

	return (i);
}

/**
* Function:	PHY_QueryBBReg
*
* OverView:	Read "sepcific bits" from BB register
*
* Input:
*			PADAPTER		Adapter,
*			u4Byte			RegAddr,		//The target address to be readback
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be readback
* Output:	None
* Return:		u4Byte			Data			//The readback register value
* Note:		This function is equal to "GetRegSetting" in PHY programming guide
*/
unsigned int PHY_QueryBBReg(struct rtl8192cd_priv *priv, unsigned int RegAddr, unsigned int BitMask)
{
	unsigned int ReturnValue = 0, OriginalValue, BitShift;

#ifdef DISABLE_BB_RF
	return 0;
#endif //DISABLE_BB_RF

#ifdef CONFIG_RTL_92D_DMDP
	unsigned char reg;
	reg = MAC_PHY_CTRL_MP;
	if (!(RTL_R8(reg) & BIT(1)) && priv->pshare->wlandev_idx == 1) {
		return DMDP_PHY_QueryBBReg(0, RegAddr, BitMask);
	}
#endif

	OriginalValue = RTL_R32(RegAddr);
	BitShift = phy_CalculateBitShift(BitMask);
	ReturnValue = (OriginalValue & BitMask) >> BitShift;

	return (ReturnValue);
}


/**
* Function:	PHY_SetBBReg
*
* OverView:	Write "Specific bits" to BB register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			u4Byte			RegAddr,		//The target address to be modified
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be modified
*			u4Byte			Data			//The new register value in the target bit position
*										//of the target address
*
* Output:	None
* Return:		None
* Note:		This function is equal to "PutRegSetting" in PHY programming guide
*/
void PHY_SetBBReg(struct rtl8192cd_priv *priv, unsigned int RegAddr, unsigned int BitMask, unsigned int Data)
{
	unsigned int OriginalValue, BitShift, NewValue;

#ifdef DISABLE_BB_RF
	return;
#endif //DISABLE_BB_RF

#ifdef CONFIG_RTL_92D_DMDP
	unsigned char reg;
	reg = MAC_PHY_CTRL_MP;

	if (!(RTL_R8(reg) & BIT(1)) && priv->pshare->wlandev_idx == 1) {
		DMDP_PHY_SetBBReg(0, RegAddr, BitMask, Data);
		return;
	}
#endif

	if (BitMask != bMaskDWord) {
		//if not "double word" write
		//_TXPWR_REDEFINE ?? if have original value, how to count tx power index from PG file ??
		OriginalValue = RTL_R32(RegAddr);
		BitShift = phy_CalculateBitShift(BitMask);
		NewValue = ((OriginalValue & (~BitMask)) | ((Data << BitShift) & BitMask));
		RTL_W32(RegAddr, NewValue);
	} else
		RTL_W32(RegAddr, Data);

	return;
}



/**
* Function:	phy_RFSerialWrite
*
* OverView:	Write data to RF register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			Offset,		//The target address to be read
*			u4Byte			Data			//The new register Data in the target bit position
*										//of the target to be read
*
* Output:	None
* Return:		None
* Note:		Threre are three types of serial operations: (1) Software serial write
*			(2) Hardware LSSI-Low Speed Serial Interface (3) Hardware HSSI-High speed
*			serial write. Driver need to implement (1) and (2).
*			This function is equal to the combination of RF_ReadReg() and  RFLSSIRead()
 *
 * Note: 		  For RF8256 only
 *			 The total count of RTL8256(Zebra4) register is around 36 bit it only employs
 *			 4-bit RF address. RTL8256 uses "register mode control bit" (Reg00[12], Reg00[10])
 *			 to access register address bigger than 0xf. See "Appendix-4 in PHY Configuration
 *			 programming guide" for more details.
 *			 Thus, we define a sub-finction for RTL8526 register address conversion
 *		       ===========================================================
 *			 Register Mode		RegCTL[1]		RegCTL[0]		Note
 *								(Reg00[12])		(Reg00[10])
 *		       ===========================================================
 *			 Reg_Mode0				0				x			Reg 0 ~15(0x0 ~ 0xf)
 *		       ------------------------------------------------------------------
 *			 Reg_Mode1				1				0			Reg 16 ~30(0x1 ~ 0xf)
 *		       ------------------------------------------------------------------
 *			 Reg_Mode2				1				1			Reg 31 ~ 45(0x1 ~ 0xf)
 *		       ------------------------------------------------------------------
*/
void phy_RFSerialWrite(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int Offset, unsigned int Data)
{
	struct rtl8192cd_hw			*phw = GET_HW(priv);
	unsigned int				DataAndAddr = 0;
	BB_REGISTER_DEFINITION_T	*pPhyReg = &phw->PHYRegDef[eRFPath];
	unsigned int				NewOffset;

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if(GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) 
		Offset &= 0xff;
	else
#endif
		Offset &= 0x7f;
	//
	// Switch page for 8256 RF IC
	//
	NewOffset = Offset;

	//
	// Put write addr in [5:0]  and write data in [31:16]
	//
	//DataAndAddr = (Data<<16) | (NewOffset&0x3f);
	DataAndAddr = ((NewOffset << 20) | (Data & 0x000fffff)) & 0x0fffffff;	// T65 RF

	//
	// Write Operation
	//
	PHY_SetBBReg(priv, pPhyReg->rf3wireOffset, bMaskDWord, DataAndAddr);
}


/**
* Function:	phy_RFSerialRead
*
* OverView:	Read regster from RF chips
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			Offset,		//The target address to be read
*			u4Byte			dbg_avoid,	//set bitmask in reg 0 to prevent RF switchs to debug mode
*
* Output:	None
* Return:		u4Byte			reback value
* Note:		Threre are three types of serial operations: (1) Software serial write
*			(2) Hardware LSSI-Low Speed Serial Interface (3) Hardware HSSI-High speed
*			serial write. Driver need to implement (1) and (2).
*			This function is equal to the combination of RF_ReadReg() and  RFLSSIRead()
*/
unsigned int phy_RFSerialRead(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int Offset, unsigned int dbg_avoid)
{
	struct rtl8192cd_hw			*phw = GET_HW(priv);
	unsigned int 				tmplong, tmplong2;
	unsigned int				retValue = 0;
	BB_REGISTER_DEFINITION_T	*pPhyReg = &phw->PHYRegDef[eRFPath];
	unsigned int				NewOffset;

	//
	// Make sure RF register offset is correct
	//
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if(GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) 
		Offset &= 0xff;
	else
#endif
		Offset &= 0x7f;

	//
	// Switch page for 8256 RF IC
	//
	NewOffset = Offset;

#if 0 // 92E use PHY_QueryRFReg & PHY_WriteRFReg callback function
//#ifdef CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E) {
		int RfPiEnable = 0;
		if (eRFPath == RF_PATH_A) {
			tmplong2 = PHY_QueryBBReg(priv, rFPGA0_XA_HSSIParameter2, bMaskDWord);;
			tmplong2 = (tmplong2 & (~bLSSIReadAddress)) | (NewOffset << 23) | bLSSIReadEdge;	//T65 RF
			PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, bMaskDWord, tmplong2 & (~bLSSIReadEdge));
		} else {
			tmplong2 = PHY_QueryBBReg(priv, rFPGA0_XB_HSSIParameter2, bMaskDWord);
			tmplong2 = (tmplong2 & (~bLSSIReadAddress)) | (NewOffset << 23) | bLSSIReadEdge;	//T65 RF
			PHY_SetBBReg(priv, rFPGA0_XB_HSSIParameter2, bMaskDWord, tmplong2 & (~bLSSIReadEdge));
		}

		tmplong2 = PHY_QueryBBReg(priv, rFPGA0_XA_HSSIParameter2, bMaskDWord);
		PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, bMaskDWord, tmplong2 & (~bLSSIReadEdge));
		PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, bMaskDWord, tmplong2 | bLSSIReadEdge);

		delay_us(20);
	} else
#endif
	{

		// For 92S LSSI Read RFLSSIRead
		// For RF A/B write 0x824/82c(does not work in the future)
		// We must use 0x824 for RF A and B to execute read trigger
		if( pPhyReg->rfHSSIPara2==0 )
		{
			printk(KERN_ERR"Warnning in %s %d : rfHSSIPara2 don't be initialized yet\n",__FUNCTION__,__LINE__);
			return 0;
		}
		tmplong = RTL_R32(rFPGA0_XA_HSSIParameter2);
		tmplong2 = RTL_R32(pPhyReg->rfHSSIPara2);
		tmplong2 = (tmplong2 & (~bLSSIReadAddress)) | ((NewOffset << 23) | bLSSIReadEdge);	//T65 RF

		RTL_W32(rFPGA0_XA_HSSIParameter2, tmplong & (~bLSSIReadEdge));
		delay_us(20);
		RTL_W32(pPhyReg->rfHSSIPara2, tmplong2);
		delay_us(20);
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
		if (
#ifdef CONFIG_RTL_92C_SUPPORT
			(GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
			||
#endif
			(GET_CHIP_VER(priv) == VERSION_8192D)
#endif
		) {
			RTL_W32(rFPGA0_XA_HSSIParameter2, tmplong | bLSSIReadEdge);
			delay_us(20);
			RTL_W32(rFPGA0_XA_HSSIParameter2, tmplong & (~bLSSIReadEdge));
		}
#endif

	}
	//Read from BBreg8a0, 12 bits for 8190, 20 bits for T65 RF
	if (((eRFPath == RF92CD_PATH_A) && (RTL_R32(0x820)&BIT(8)))
			|| ((eRFPath == RF92CD_PATH_B) && (RTL_R32(0x828)&BIT(8))))
		retValue = PHY_QueryBBReg(priv, pPhyReg->rfLSSIReadBackPi, bLSSIReadBackData);
	else
		retValue = PHY_QueryBBReg(priv, pPhyReg->rfLSSIReadBack, bLSSIReadBackData);

	return retValue;
}

static void set_rf_path_num(struct rtl8192cd_priv *priv){
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8197F) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8822BE)
	if(GET_CHIP_VER(priv) == VERSION_8192C || GET_CHIP_VER(priv) == VERSION_8192D ||
		GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8197F ||
		GET_CHIP_VER(priv) == VERSION_8812E|| GET_CHIP_VER(priv) == VERSION_8822B){
		priv->pshare->phw->tx_path_start = RF92CD_PATH_A;
		priv->pshare->phw->tx_path_end = RF92CD_PATH_B;
		priv->pshare->phw->rx_path_start = RF92CD_PATH_A;
		priv->pshare->phw->rx_path_end = RF92CD_PATH_B;
	} else
#endif
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	if(GET_CHIP_VER(priv) == VERSION_8188E|| GET_CHIP_VER(priv) == VERSION_8881A){
		priv->pshare->phw->tx_path_start = RF92CD_PATH_A;
		priv->pshare->phw->tx_path_end = RF92CD_PATH_A;
		priv->pshare->phw->rx_path_start = RF92CD_PATH_A;
		priv->pshare->phw->rx_path_end = RF92CD_PATH_A;
	} else
#endif
#if defined(CONFIG_WLAN_HAL_8814AE)
	if(GET_CHIP_VER(priv) == VERSION_8814A){
		priv->pshare->phw->tx_path_start = RF92CD_PATH_B;
		priv->pshare->phw->tx_path_end = RF92CD_PATH_D;
		priv->pshare->phw->rx_path_start = RF92CD_PATH_A;
		priv->pshare->phw->rx_path_end = RF92CD_PATH_D;
	} else
#endif
	{			
		priv->pshare->phw->tx_path_start = RF92CD_PATH_A;
		priv->pshare->phw->tx_path_end = RF92CD_PATH_A;
		priv->pshare->phw->rx_path_start = RF92CD_PATH_A;
		priv->pshare->phw->rx_path_end = RF92CD_PATH_A;
	}
}


#if defined(AC2G_256QAM) || defined(WLAN_HAL_8814AE)
char is_ac2g(struct rtl8192cd_priv * priv)
{

	unsigned char ac2g = 0;

	if((GET_CHIP_VER(priv)== VERSION_8812E) || (GET_CHIP_VER(priv)== VERSION_8881A))
	{
	if(priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC 
		&& ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) == 0)
		&& (priv->pshare->rf_ft_var.ac2g_enable)
		)
		ac2g = 1;

		if((OPMODE&WIFI_STATION_STATE) 
			&& (priv->pmib->dot11BssType.net_work_type == (WIRELESS_11A|WIRELESS_11B|WIRELESS_11G|WIRELESS_11N|WIRELESS_11AC))
			&& ((priv->pmib->dot11RFEntry.dot11channel != 0) && (priv->pmib->dot11RFEntry.dot11channel<36)))
			ac2g = 1;
	}

	if(GET_CHIP_VER(priv)==VERSION_8814A) {
		if(priv->pmib->dot11RFEntry.phyBandSelect ==  PHY_BAND_2G)
			ac2g = 1;
	}

	return ac2g; 

}
#endif

#if defined(CONFIG_RTL_8812_SUPPORT)

unsigned int PHY_QueryRFReg_8812(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath,
								 unsigned int RegAddr, unsigned int BitMask, unsigned int dbg_avoid)
{
	unsigned int Original_Value = 0, Readback_Value, BitShift;
	unsigned int dwTmp;

	PHY_SetBBReg(priv, 0x838, 0xf, 0xc);		//		CCA off

	RTL_W8(0x8b0, RegAddr);

	if (IS_TEST_CHIP(priv)) {	//for_8812_mp_chip
		dwTmp = RTL_R32(0x8b0);
		dwTmp &= ~BIT(8);
		RTL_W32(0x8b0, dwTmp);
		//printk("0x8b0 = 0x%x\n", dwTmp);
		dwTmp |= BIT(8);
		RTL_W32(0x8b0, dwTmp);
		//printk("0x8b0 = 0x%x\n", dwTmp);
	}

	delay_us(50);

	if (eRFPath == RF92CD_PATH_A)
		Original_Value = RTL_R32(0xd04);
	else if (eRFPath == RF92CD_PATH_B)
		Original_Value = RTL_R32(0xd44);

	//printk("_eric_8812 rf_%x = 0x%x \n", RegAddr, Original_Value);

	Original_Value &= 0xfffff;

	BitShift =	phy_CalculateBitShift(BitMask);
	Readback_Value = (Original_Value & BitMask) >> BitShift;

	PHY_SetBBReg(priv, 0x838, 0xf, 0x4);		//		CCA on

	return (Readback_Value);
}

void phy_RFSerialWrite_8812(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int Offset, unsigned int Data)
{
	unsigned int dwTmp = 0;
	unsigned int value = ((Offset << 20) | Data);

	//printk("_eric_8182 phy_RFSerialWrite_8812 +++ \n");

	PHY_SetBBReg(priv, 0x838, 0xf, 0xc);		//		CCA off

	if (eRFPath == RF92CD_PATH_A) {
		dwTmp = RTL_R32(0xc90);
		dwTmp &= 0xf0000000;
		dwTmp |= value;
		//printk("_eric_8812 0xc90 = 0x%x \n", dwTmp);
		RTL_W32(0xc90, dwTmp);
	} else if (eRFPath == RF92CD_PATH_B) {
		dwTmp = RTL_R32(0xe90);
		dwTmp &= 0xf0000000;
		dwTmp |= value;
		//printk("_eric_8812 0xe90 = 0x%x \n", dwTmp);
		RTL_W32(0xe90, dwTmp);
	}

	PHY_SetBBReg(priv, 0x838, 0xf, 0x4);		//		CCA on
}

#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void PHY_SetRFReg_8812(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int RegAddr,
					   unsigned int BitMask, unsigned int Data)
{
	unsigned int Original_Value, BitShift, New_Value;
#ifndef SMP_SYNC
	unsigned long flags;
#endif
#if 0
	if (priv->pshare->No_RF_Write == 1
#ifdef MP_TEST
			&& (!priv->pshare->rf_ft_var.mp_specific)
#endif
	   )
		return;
#endif
	SAVE_INT_AND_CLI(flags);
	if (BitMask != bMask20Bits) {
		Original_Value = PHY_QueryRFReg_8812(priv, eRFPath, RegAddr, bMask20Bits, 0);
		BitShift = phy_CalculateBitShift(BitMask);
		New_Value = ((Original_Value & (~BitMask)) | (Data << BitShift));
		phy_RFSerialWrite_8812(priv, eRFPath, RegAddr, New_Value);
	} else {
		phy_RFSerialWrite_8812(priv, eRFPath, RegAddr, Data);
	}
	RESTORE_INT(flags);	
	delay_us(4);	// At least 500ns delay to avoid RF write fail.
}

#endif


/**
* Function:	PHY_QueryRFReg
*
* OverView:	Query "Specific bits" to RF register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			RegAddr,		//The target address to be read
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be read
*			u4Byte			dbg_avoid	//set bitmask in reg 0 to prevent RF switchs to debug mode
*
* Output:	None
* Return:		u4Byte			Readback value
* Note:		This function is equal to "GetRFRegSetting" in PHY programming guide
*/
unsigned int PHY_QueryRFReg(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath,
							unsigned int RegAddr, unsigned int BitMask, unsigned int dbg_avoid)
{
#ifdef DISABLE_BB_RF
	return 0;
#endif //DISABLE_BB_RF

#if defined(CONFIG_RTL_8812_SUPPORT) 
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		return PHY_QueryRFReg_8812(priv, eRFPath, RegAddr, BitMask, dbg_avoid);
	}
#endif

#if defined(CONFIG_WLAN_HAL)
	if(IS_HAL_CHIP(priv)) {
		return GET_HAL_INTERFACE(priv)->PHYQueryRFRegHandler(priv, eRFPath, RegAddr, BitMask);        
	} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{
		unsigned int	Original_Value, Readback_Value, BitShift;

#ifdef CONFIG_RTL_92D_DMDP
		unsigned char reg;
		reg = MAC_PHY_CTRL_MP;
		if (!(RTL_R8(reg) & BIT(1)) && priv->pshare->wlandev_idx == 1) {
			return DMDP_PHY_QueryRFReg(0, eRFPath, RegAddr, BitMask, dbg_avoid);
		}
#endif

		Original_Value = phy_RFSerialRead(priv, eRFPath, RegAddr, dbg_avoid);
		BitShift =  phy_CalculateBitShift(BitMask);
		Readback_Value = (Original_Value & BitMask) >> BitShift;
		return (Readback_Value);
	}
}


/**
* Function:	PHY_SetRFReg
*
* OverView:	Write "Specific bits" to RF register (page 8~)
*
* Input:
*			PADAPTER		Adapter,
*			RF92CD_RADIO_PATH_E	eRFPath,	//Radio path of A/B/C/D
*			u4Byte			RegAddr,		//The target address to be modified
*			u4Byte			BitMask		//The target bit position in the target address
*										//to be modified
*			u4Byte			Data			//The new register Data in the target bit position
*										//of the target address
*
* Output:	None
* Return:		None
* Note:		This function is equal to "PutRFRegSetting" in PHY programming guide
*/
#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void PHY_SetRFReg(struct rtl8192cd_priv *priv, RF92CD_RADIO_PATH_E eRFPath, unsigned int RegAddr,
				  unsigned int BitMask, unsigned int Data)
{
#ifdef DISABLE_BB_RF
	return;
#endif //DISABLE_BB_RF

#if defined(CONFIG_RTL_8812_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		return PHY_SetRFReg_8812(priv, eRFPath, RegAddr, BitMask, Data);
	}
#endif

#if defined(CONFIG_WLAN_HAL)
	if(IS_HAL_CHIP(priv)) {
		return GET_HAL_INTERFACE(priv)->PHYSSetRFRegHandler(priv, eRFPath, RegAddr, BitMask, Data);                
	} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{
		unsigned int Original_Value, BitShift, New_Value;
#ifndef SMP_SYNC
		unsigned long flags;
#endif

#ifdef CONFIG_RTL_92D_DMDP
		unsigned char reg;
		reg = MAC_PHY_CTRL_MP;

		if (!(RTL_R8(reg) & BIT(1)) && priv->pshare->wlandev_idx == 1) {
			DMDP_PHY_SetRFReg(0, eRFPath, RegAddr, BitMask, Data);
			return;
		}
#endif

		SAVE_INT_AND_CLI(flags);

		if (BitMask != bMask20Bits) {
			Original_Value = phy_RFSerialRead(priv, eRFPath, RegAddr, 1);
			BitShift = phy_CalculateBitShift(BitMask);
			New_Value = ((Original_Value & (~BitMask)) | (Data << BitShift));

			phy_RFSerialWrite(priv, eRFPath, RegAddr, New_Value);
		} else {
			phy_RFSerialWrite(priv, eRFPath, RegAddr, Data);
		}

		RESTORE_INT(flags);
		delay_us(4);    // At least 500ns delay to avoid RF write fail.
	}
}


#ifdef TXPWR_LMT_NEWFILE
static int is_NA(char* s)
{
	if (( s[0] == 'N' || s[0] == 'n') && ( s[1] == 'A' || s[1] == 'a'))
		return 1;
	else
		return 0;
}
#endif

static int is_hex(char s)
{
	if (( s >= '0' && s <= '9') || ( s >= 'a' && s <= 'f') || (s >= 'A' && s <= 'F') || (s == 'x' || s == 'X'))
		return 1;
	else
		return 0;
}


static int is_item(char s)
{
	if (s == 't' || s == 'a' || s == 'b' || s == 'l' || s == 'e'  || s == ':')
		return 1;
	else
		return 0;
}

static unsigned char *get_digit(unsigned char **data)
{
	unsigned char *buf = *data;
	int i = 0;

	while (buf[i] && ((buf[i] == ' ') || (buf[i] == '\t')))
		i++;
	*data = &buf[i];

	while (buf[i]) {
		if ((buf[i] == ' ') || (buf[i] == '\t')) {
			buf[i] = '\0';
			break;
		}
		if (buf[i] >= 'A' && buf[i] <= 'Z')
			buf[i] += 32;

		if (!is_hex(buf[i]) && !is_item(buf[i]))
			return NULL;
		i++;
	}
	if (i == 0)
		return NULL;
	else
		return &buf[i + 1];
}

#ifdef TXPWR_LMT_NEWFILE

static unsigned char *get_digit_dot_8812(unsigned char **data)
{
	unsigned char *buf=*data;
	int i=0;

	while(buf[i] && ((buf[i] == ' ') || (buf[i] == '\t')))
		i++;
	
	*data = &buf[i];

	while(buf[i]) {
		if(buf[i] == '.'){
//			while((buf[i]==' ') ||(buf[i]=='\t') || (buf[i]=='\0') || (buf[i]=='/'))
//				i++;

			i++;
		}
		
		if ((buf[i] == ' ') || (buf[i] == '\t')) {
			buf[i] = '\0';
			break;
		}
		
		if (buf[i]>='A' && buf[i]<='Z')
			buf[i] += 32;
		
		if (!is_hex(buf[i])&&!is_item(buf[i])
			&& !is_NA((char *)&buf[i]))
			return NULL;
		
		i++;
	}
	if (i == 0)
		return NULL;
	else
		{
			return &buf[i+1];
		}
}


#define LMT_NUM 256
static int get_chnl_lmt_dot_new(struct rtl8192cd_priv *priv, unsigned char *line_head, unsigned int *channel, unsigned int *limit, unsigned int *table_idx)
{
	unsigned char *next, *next2, *next3;
	int base, idx;
	int num=0;
	int num_index;
	int val;
	int i; // for loop
	unsigned char *ch;
	unsigned char lmt[LMT_NUM];
	unsigned char *twpwr_lmt_index_array;
	unsigned char lmtIndex;
	extern int _atoi(char *s, int base);
	
	*channel = '\0';
	base = 10;

	//find limit table index first
	if (!memcmp(line_head+2,"Table",5)) {
		*channel = 0;
		//printk("found idx\n");
		goto find_idx;
	}
	else if (!memcmp(line_head,"CH0",3)) {
		//printk("found channel\n");
		ch = line_head+3;
	}
	else if (!memcmp(line_head,"CH",2)) {
		//printk("found channel\n");
		ch = line_head+2;
	}
	else
		return 0;

	*channel = _atoi(ch, base);	
	

	num = 0;
	next = get_digit_dot_8812(&ch);
	
	while(next != NULL)
	{		
		num++;
		if(is_NA(next))
			lmt[num] = 0; //No limit
		else {
			val = _convert_2_pwr_dot(next, base);
			if(val == -1)
				num --;
			else
				lmt[num] = val;
		}
		next = get_digit_dot_8812(&next);
		
	}
	if(num == 0)
		panic_printk("TX Power Porfile format Error\n");
	
	//printk("channel = %d, lmt = %d %d %d\n", *channel, lmt[1], lmt[2], lmt[3]);

	
	if(priv->pmib->dot11StationConfigEntry.txpwr_lmt_index <= 0) {
		if(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK)
			lmtIndex = 3;	
		else if(priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_ETSI)
			lmtIndex = 2;	
		else
			lmtIndex = 1;	
	} else {
		lmtIndex = priv->pmib->dot11StationConfigEntry.txpwr_lmt_index;
	}		
    
	*limit = lmt[lmtIndex];

	return 1;

find_idx:
	
	*channel = 0;
	*limit = 0;
	*table_idx = 0;

	line_head = line_head + 8; 

	*table_idx = _atoi(line_head, base); //In this condition, 'limit' represents "# of table"
	//printk("\n[table_idx = %d]\n", *table_idx);

	return 1;

}
#endif


static unsigned char *get_digit_dot(unsigned char **data)
{
	unsigned char *buf = *data;
	int i = 0;

	while (buf[i] && ((buf[i] == ' ') || (buf[i] == '\t')))
		i++;

	*data = &buf[i];

	while (buf[i]) {
		if (buf[i] == '.') {
//			while ((buf[i] == ' ') || (buf[i] == '\t') || (buf[i] == '\0') || (buf[i] == '/'))
//				i++;

			i++;
		}

		if ((buf[i] == ' ') || (buf[i] == '\t')) {
			buf[i] = '\0';
			break;
		}
		if (buf[i] >= 'A' && buf[i] <= 'Z')
			buf[i] += 32;

		if (!is_hex(buf[i]) && !is_item(buf[i]))
			return NULL;
		i++;
	}
	if (i == 0)
		return NULL;
	else
		{
		return &buf[i + 1];
	}
}


#ifdef TXPWR_LMT
static int get_chnl_lmt_dot(unsigned char *line_head, unsigned int *ch_start, unsigned int *ch_end, unsigned int *limit, unsigned int *target)
{
	unsigned char *next, *next2;
	int base, idx;
	int num = 0, round = 0;
	unsigned char *ch;
	extern int _atoi(char * s, int base);

	*ch_start = *ch_start = '\0';

	// remove comments
	ch = line_head;
	while (1) {
		if ((*ch == '\0') || (*ch == '\n') || (*ch == '\r'))
			break;
		else if (*ch == '/') {
			*ch = '\0';
			break;
		} else {
			ch++;
		}

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	next = get_digit_dot(&line_head);
	if (next == NULL)
		return num;
	num++;

	if (!memcmp(line_head, "table", 5)) {
		*ch_start = 0;
		*ch_end = 0;
	} else {
//		char *s;
		int format = 0;
		int idx2;

		if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2))) {
			base = 16;
			idx = 2;
		} else {
			base = 10;
			idx = 0;
		}
		idx2 = idx;
		while (line_head[idx2] != '\0') {
			//printk("(%c)",line_head[idx2]);
			if (line_head[idx2] == ':') {
				line_head[idx2] = '\0';
				format = 1; // format - start:end
				break;
			}
			idx2++;
		}
		*ch_start = _atoi((char *)&line_head[idx], base);
		if (format == 0) {
			*ch_end = *ch_start;
		} else {
			*ch_end = _atoi((char *)&line_head[idx2 + 1], base);
		}
	}


	*limit = 0;
	if (next) {
		if (!(next2 = get_digit_dot(&next)))
			return num;
		num++;

		base = 10;
		idx = 0;

		if ( (*ch_start == 0) && (*ch_end == 0) )
			*limit = _atoi((char *)&next[idx], base); //In this condition, 'limit' represents "# of table"
		else
			*limit = _convert_2_pwr_dot((char *)&next[idx], base);
	}


	*target = 0;
	if (next2) {
		if (!get_digit_dot(&next2))
			return num;
		num++;

		base = 10;
		idx = 0;

		*target = _convert_2_pwr_dot((char *)&next2[idx], base);
	}


	return num;
}
#endif

//static 
int get_offset_val(unsigned char *line_head, unsigned int *u4bRegOffset, unsigned int *u4bRegValue)
{
	unsigned char *next;
	int base, idx;
	int num = 0, round = 0;
	unsigned char *ch;
	extern int _atoi(char * s, int base);

	*u4bRegOffset = *u4bRegValue = '\0';

	ch = line_head;
	while (1) {
		if ((*ch == '\0') || (*ch == '\n') || (*ch == '\r'))
			break;
		else if (*ch == '/') {
			*ch = '\0';
			break;
		} else {
			ch++;
		}

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	next = get_digit(&line_head);
	if (next == NULL)
		return num;
	num++;
	if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2))) {
		base = 16;
		idx = 2;
	} else {
		base = 10;
		idx = 0;
	}
	*u4bRegOffset = _atoi((char *)&line_head[idx], base);

	if (next) {
		if (!get_digit(&next))
			return num;
		num++;
		if ((!memcmp(next, "0x", 2)) || (!memcmp(next, "0X", 2))) {
			base = 16;
			idx = 2;
		} else {
			base = 10;
			idx = 0;
		}
		*u4bRegValue = _atoi((char *)&next[idx], base);
	} else
		*u4bRegValue = 0;

	return num;
}


#if 0//defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)
void assign_target_value_88e_new(struct rtl8192cd_priv *priv, unsigned int *u4bRegOffset, unsigned char *tmp_byte)
{	
	//CCK-11M
	if(*u4bRegOffset == rTxAGC_A_CCK11_2_B_CCK11)
	if(tmp_byte[3] != 0)
		priv->pshare->tgpwr_CCK_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_A_CCK11_2_B_CCK11)
	if(tmp_byte[0] != 0)
		priv->pshare->tgpwr_CCK_new[RF_PATH_B] = tmp_byte[0];	

	//OFDM-54M
	if(*u4bRegOffset == rTxAGC_A_Rate54_24)
		priv->pshare->tgpwr_OFDM_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_Rate54_24)
		priv->pshare->tgpwr_OFDM_new[RF_PATH_B] = tmp_byte[3];	

	//HT-MCS7
	if(*u4bRegOffset == rTxAGC_A_Mcs07_Mcs04)
		priv->pshare->tgpwr_HT1S_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_Mcs07_Mcs04)
		priv->pshare->tgpwr_HT1S_new[RF_PATH_B] = tmp_byte[3];	

	//HT-MCS15
	if(*u4bRegOffset == rTxAGC_A_Mcs15_Mcs12)
		priv->pshare->tgpwr_HT2S_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_Mcs15_Mcs12)
		priv->pshare->tgpwr_HT2S_new[RF_PATH_B] = tmp_byte[3];	


}
#endif
#if 0//defined( CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
void assign_target_value_8812_new(struct rtl8192cd_priv *priv, unsigned int *u4bRegOffset, unsigned char *tmp_byte)
{	
	//CCK-11M
	if(*u4bRegOffset == rTxAGC_A_CCK11_CCK1_JAguar)
		priv->pshare->tgpwr_CCK_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_CCK11_CCK1_JAguar)
		priv->pshare->tgpwr_CCK_new[RF_PATH_B] = tmp_byte[3];	

	//OFDM-54M
	if(*u4bRegOffset == rTxAGC_A_Ofdm54_Ofdm24_JAguar)
		priv->pshare->tgpwr_OFDM_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_Ofdm54_Ofdm24_JAguar)
		priv->pshare->tgpwr_OFDM_new[RF_PATH_B] = tmp_byte[3];	

	//HT-MCS7
	if(*u4bRegOffset == rTxAGC_A_MCS7_MCS4_JAguar)
		priv->pshare->tgpwr_HT1S_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_MCS7_MCS4_JAguar)
		priv->pshare->tgpwr_HT1S_new[RF_PATH_B] = tmp_byte[3];	

	//HT-MCS15
	if(*u4bRegOffset == rTxAGC_A_MCS15_MCS12_JAguar)
		priv->pshare->tgpwr_HT2S_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_MCS15_MCS12_JAguar)
		priv->pshare->tgpwr_HT2S_new[RF_PATH_B] = tmp_byte[3];	

	//VHT-NSS1-MCS7
	if(*u4bRegOffset == rTxAGC_A_Nss1Index7_Nss1Index4_JAguar)
		priv->pshare->tgpwr_VHT1S_new[RF_PATH_A] = tmp_byte[3];

	if(*u4bRegOffset == rTxAGC_B_Nss1Index7_Nss1Index4_JAguar)
		priv->pshare->tgpwr_VHT1S_new[RF_PATH_B] = tmp_byte[3];	

	//VHT-NSS2-MCS7
	if(*u4bRegOffset == rTxAGC_A_Nss2Index9_Nss2Index6_JAguar)
		priv->pshare->tgpwr_VHT2S_new[RF_PATH_A] = tmp_byte[1];

	if(*u4bRegOffset == rTxAGC_B_Nss2Index9_Nss2Index6_JAguar)
		priv->pshare->tgpwr_VHT2S_new[RF_PATH_B] = tmp_byte[1];	

}
#endif

unsigned int gen_reg_value(unsigned char* tmp_reg)
{
	return (tmp_reg[0] | (tmp_reg[1]<<8) | (tmp_reg[2]<<16) | (tmp_reg[3]<<24)) ;
}

unsigned int gen_reg_value_2(unsigned char target, unsigned char* tmp_byte, unsigned char* tmp_reg)
{
	unsigned char i =0;
	
	for(i=0; i<4; i++)
	{
		if(target < tmp_byte[i])
			tmp_reg[i] = tmp_byte[i] - target;
		else
			tmp_reg[i] = target - tmp_byte[i];
	}
	
	return (tmp_reg[0] | (tmp_reg[1]<<8) | (tmp_reg[2]<<16) | (tmp_reg[3]<<24)) ;
}

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)	|| defined(CONFIG_RTL_8723B_SUPPORT)
unsigned int generate_u4bRegValue_88e_new(struct rtl8192cd_priv *priv, unsigned int *u4bRegOffset, unsigned char *tmp_byte) 
{
	unsigned char tmp_reg[4];
	unsigned int  reg_value = 0;
	unsigned char target = 0;

	if(*u4bRegOffset == rTxAGC_A_CCK1_Mcs32) //{NA NA 1M NA}
	{
		reg_value = RTL_R32(rTxAGC_A_CCK1_Mcs32); 
		
		tmp_reg[0]= (reg_value & (0xff));
		tmp_reg[1]= tmp_byte[1]- priv->pshare->tgpwr_CCK_new[RF_PATH_A];
		tmp_reg[2]= ((reg_value & (0xff0000)) >> 16);
		tmp_reg[3]= ((reg_value & (0xff000000)) >> 24);

		reg_value = gen_reg_value(tmp_reg);
		return reg_value;
	}
	else if(*u4bRegOffset == rTxAGC_B_CCK5_1_Mcs32) // {5.5M 2M 1M NA}
	{
		reg_value = RTL_R32(rTxAGC_B_CCK5_1_Mcs32); 
		tmp_reg[0]= (reg_value & (0xff));
		tmp_reg[1]= tmp_byte[1]- priv->pshare->tgpwr_CCK_new[RF_PATH_B];
		tmp_reg[2]= tmp_byte[2]- priv->pshare->tgpwr_CCK_new[RF_PATH_B];
		tmp_reg[3]= tmp_byte[3]- priv->pshare->tgpwr_CCK_new[RF_PATH_B];

		reg_value = gen_reg_value(tmp_reg);
		return reg_value;
	}
	else if(*u4bRegOffset == rTxAGC_A_CCK11_2_B_CCK11) //{11M 5.5M 2M NA} {NA NA NA NA 11M}
	{
		reg_value = RTL_R32(rTxAGC_A_CCK11_2_B_CCK11); 
		if (tmp_byte[3] > 0)
			tmp_reg[3] = tmp_byte[3] - priv->pshare->tgpwr_CCK_new[RF_PATH_A];
		else			
			tmp_reg[3] = ((reg_value & (0xff000000)) >> 24);

		if (tmp_byte[0] > 0)
			tmp_reg[0] = tmp_byte[0] - priv->pshare->tgpwr_CCK_new[RF_PATH_B];		
		else
			tmp_reg[0] = (reg_value & 0xff);
		
		if(tmp_byte[1] > 0)
		tmp_reg[1]= tmp_byte[1]- priv->pshare->tgpwr_CCK_new[RF_PATH_A];
		else
		tmp_reg[1]=((reg_value & (0xff00)) >> 8);
		
		if(tmp_byte[2] > 0)
		tmp_reg[2]= tmp_byte[2]- priv->pshare->tgpwr_CCK_new[RF_PATH_A];
		else
		tmp_reg[2]=((reg_value & (0xff0000)) >> 16);
		
		reg_value = gen_reg_value(tmp_reg);
		RTL_W32(rTxAGC_A_CCK11_2_B_CCK11, reg_value);
		return reg_value;
	}

	switch (*u4bRegOffset) {
		case rTxAGC_A_Rate18_06:
		case rTxAGC_A_Rate54_24: 
			target = priv->pshare->tgpwr_OFDM_new[RF_PATH_A];
			break;
		case rTxAGC_A_Mcs03_Mcs00: 
		case rTxAGC_A_Mcs07_Mcs04: 
			target = priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
			break;
		case rTxAGC_A_Mcs11_Mcs08:	
		case rTxAGC_A_Mcs15_Mcs12:
			target = priv->pshare->tgpwr_HT2S_new[RF_PATH_A];
			break;
		case rTxAGC_B_Rate18_06:	
		case rTxAGC_B_Rate54_24: 
			target = priv->pshare->tgpwr_OFDM_new[RF_PATH_B];
			break;
		case rTxAGC_B_Mcs03_Mcs00: 
		case rTxAGC_B_Mcs07_Mcs04: 
			target = priv->pshare->tgpwr_HT1S_new[RF_PATH_B];
			break;
		case rTxAGC_B_Mcs11_Mcs08:
		case rTxAGC_B_Mcs15_Mcs12:
			target = priv->pshare->tgpwr_HT2S_new[RF_PATH_B];
			break;
	}

	reg_value = gen_reg_value_2(target, tmp_byte, tmp_reg);
	return reg_value; 

}
#endif

#if defined(RTK_AC_SUPPORT)
unsigned int generate_u4bRegValue_8812_new(struct rtl8192cd_priv *priv, unsigned int *u4bRegOffset, unsigned char *tmp_byte)
{
	unsigned char tmp_reg[4];
	unsigned int  reg_value = 0;
	unsigned char target = 0;
	int i;
	if(*u4bRegOffset == rTxAGC_A_Nss2Index1_Nss1Index8_JAguar)
	{
		tmp_reg[0]= tmp_byte[0]- priv->pshare->tgpwr_VHT1S_new[RF_PATH_A]; // tgpwr_VHT1S_new is ZERO?
		tmp_reg[1]= tmp_byte[1]- priv->pshare->tgpwr_VHT1S_new[RF_PATH_A];
		tmp_reg[2]= tmp_byte[2]- priv->pshare->tgpwr_VHT2S_new[RF_PATH_A];
		tmp_reg[3]= tmp_byte[3]- priv->pshare->tgpwr_VHT2S_new[RF_PATH_A];

		reg_value = gen_reg_value(tmp_reg);
		return reg_value;
	}
	else if(*u4bRegOffset == rTxAGC_B_Nss2Index1_Nss1Index8_JAguar)
	{
		tmp_reg[0]= tmp_byte[0]- priv->pshare->tgpwr_VHT1S_new[RF_PATH_B];
		tmp_reg[1]= tmp_byte[1]- priv->pshare->tgpwr_VHT1S_new[RF_PATH_B];
		tmp_reg[2]= tmp_byte[2]- priv->pshare->tgpwr_VHT2S_new[RF_PATH_B];
		tmp_reg[3]= tmp_byte[3]- priv->pshare->tgpwr_VHT2S_new[RF_PATH_B];

		reg_value = gen_reg_value(tmp_reg);
		return reg_value;
	}

	switch (*u4bRegOffset) {
		case rTxAGC_A_CCK11_CCK1_JAguar:
			target = priv->pshare->tgpwr_CCK_new[RF_PATH_A]; // tgpwr_CCK_new is ZERO?
			break;
		case rTxAGC_A_Ofdm18_Ofdm6_JAguar:
			target = priv->pshare->tgpwr_OFDM_new[RF_PATH_A];
			break;
		case rTxAGC_A_Ofdm54_Ofdm24_JAguar: 
			target = priv->pshare->tgpwr_OFDM_new[RF_PATH_A];
			break;
		case rTxAGC_A_MCS3_MCS0_JAguar: 
			target = priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
			break;
		case rTxAGC_A_MCS7_MCS4_JAguar: 
			target = priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
			break;
		case rTxAGC_A_MCS11_MCS8_JAguar:	
			target = priv->pshare->tgpwr_HT2S_new[RF_PATH_A];
			break;
		case rTxAGC_A_MCS15_MCS12_JAguar:
			target = priv->pshare->tgpwr_HT2S_new[RF_PATH_A];
			break;
		case rTxAGC_A_Nss1Index3_Nss1Index0_JAguar: 
			target = priv->pshare->tgpwr_VHT1S_new[RF_PATH_A];
			break;
		case rTxAGC_A_Nss1Index7_Nss1Index4_JAguar: 
			target = priv->pshare->tgpwr_VHT1S_new[RF_PATH_A];
			break;
		case rTxAGC_A_Nss2Index1_Nss1Index8_JAguar: 
			break;
		case rTxAGC_A_Nss2Index5_Nss2Index2_JAguar: 
			target = priv->pshare->tgpwr_VHT2S_new[RF_PATH_A];
			break;
		case rTxAGC_A_Nss2Index9_Nss2Index6_JAguar: 
			target = priv->pshare->tgpwr_VHT2S_new[RF_PATH_A];
			break;
		case rTxAGC_B_CCK11_CCK1_JAguar:	
			target = priv->pshare->tgpwr_CCK_new[RF_PATH_B];
			break;
		case rTxAGC_B_Ofdm18_Ofdm6_JAguar:	
			target = priv->pshare->tgpwr_OFDM_new[RF_PATH_B];
			break;
		case rTxAGC_B_Ofdm54_Ofdm24_JAguar: 
			target = priv->pshare->tgpwr_OFDM_new[RF_PATH_B];
			break;
		case rTxAGC_B_MCS3_MCS0_JAguar: 
			target = priv->pshare->tgpwr_HT1S_new[RF_PATH_B];
			break;
		case rTxAGC_B_MCS7_MCS4_JAguar: 
			target = priv->pshare->tgpwr_HT1S_new[RF_PATH_B];
			break;
		case rTxAGC_B_MCS11_MCS8_JAguar:
			target = priv->pshare->tgpwr_HT2S_new[RF_PATH_B];
			break;
		case rTxAGC_B_MCS15_MCS12_JAguar:
			target = priv->pshare->tgpwr_HT2S_new[RF_PATH_B];
			break;
		case rTxAGC_B_Nss1Index3_Nss1Index0_JAguar: 
			target = priv->pshare->tgpwr_VHT1S_new[RF_PATH_B];
			break;
		case rTxAGC_B_Nss1Index7_Nss1Index4_JAguar:
			target = priv->pshare->tgpwr_VHT1S_new[RF_PATH_B];
			break;
		case rTxAGC_B_Nss2Index1_Nss1Index8_JAguar: 
			break;
		case rTxAGC_B_Nss2Index5_Nss2Index2_JAguar: 
			target = priv->pshare->tgpwr_VHT2S_new[RF_PATH_B];
			break;
		case rTxAGC_B_Nss2Index9_Nss2Index6_JAguar: 
			target = priv->pshare->tgpwr_VHT2S_new[RF_PATH_B];
			break;
	}

	reg_value = gen_reg_value_2(target, tmp_byte, tmp_reg);
	return reg_value; 

}
#endif

#if defined(CONFIG_WLAN_HAL_8197F)
void set_power_by_rate(struct rtl8192cd_priv *priv, unsigned int *u4bRegOffset, unsigned char *tmp_byte)
{
	unsigned char tmp_reg[4];
	unsigned int  reg_value = 0;
	unsigned char target = 0;
	int i;

	//panic_printk("[%s] u4bRegOffset=%x, tmp_byte=%x\n",__FUNCTION__,*u4bRegOffset, *((unsigned int*)tmp_byte));
	switch (*u4bRegOffset) {
		case rTxAGC_A_CCK1_Mcs32:
				priv->pshare->phw->CCKTxAgc_A[3] = (char)tmp_byte[1];
				//printk("CCK_A[3] (1M)=%x\n",priv->pshare->phw->CCKTxAgc_A[3]);u
				break;
		case rTxAGC_A_CCK11_2_B_CCK11:
				priv->pshare->phw->CCKTxAgc_B[0] = (char)tmp_byte[0]?(char)tmp_byte[0]:priv->pshare->phw->CCKTxAgc_B[0];
				priv->pshare->phw->CCKTxAgc_A[2] = (char)tmp_byte[1]?(char)tmp_byte[1]:priv->pshare->phw->CCKTxAgc_A[2];
				priv->pshare->phw->CCKTxAgc_A[1] = (char)tmp_byte[2]?(char)tmp_byte[2]:priv->pshare->phw->CCKTxAgc_A[1] ;
				priv->pshare->phw->CCKTxAgc_A[0] = (char)tmp_byte[3]?(char)tmp_byte[3]:priv->pshare->phw->CCKTxAgc_A[0] ;
				//for(i=0;i<4;i++)
				//	printk("CCK_A[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_A[i]);
				//printk("CCK_B[0] (11M)=%x\n",priv->pshare->phw->CCKTxAgc_B[0]);
				break;
		case rTxAGC_A_Rate18_06:
				priv->pshare->phw->OFDMTxAgcOffset_A[3] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_A[2] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_A[1] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_A[0] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("OFDM_A[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_A[i]);
				break;
		case rTxAGC_A_Rate54_24: 
				priv->pshare->phw->OFDMTxAgcOffset_A[7] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_A[6] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_A[5] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_A[4] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("OFDM_A[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_A[i]);	
				break;
		case rTxAGC_A_Mcs03_Mcs00: 
				priv->pshare->phw->MCSTxAgcOffset_A[3] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[2] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[1] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[0] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("MCS_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);						
				break;
		case rTxAGC_A_Mcs07_Mcs04: 
				priv->pshare->phw->MCSTxAgcOffset_A[7] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[6] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[5] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[4] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("MCS_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);					
				break;
		case rTxAGC_A_Mcs11_Mcs08:	
				priv->pshare->phw->MCSTxAgcOffset_A[11] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[10] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[9] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[8] = (char)tmp_byte[3];
				//for(i=8;i<12;i++)
				//	printk("MCS_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);					
				break;
		case rTxAGC_A_Mcs15_Mcs12:
				priv->pshare->phw->MCSTxAgcOffset_A[15] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[14] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[13] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[12] = (char)tmp_byte[3];
				//for(i=12;i<16;i++)
				//	printk("MCS_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);						
				break;
		case rTxAGC_B_CCK5_1_Mcs32:	
				priv->pshare->phw->CCKTxAgc_B[3] = (char)tmp_byte[1];
				priv->pshare->phw->CCKTxAgc_B[2] = (char)tmp_byte[2];
				priv->pshare->phw->CCKTxAgc_B[1] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("CCK_B[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_B[i]);
				break;
		case rTxAGC_B_Rate18_06:	
				priv->pshare->phw->OFDMTxAgcOffset_B[3] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_B[2] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_B[1] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_B[0] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("OFDM_B[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_B[i]);
				break;
		case rTxAGC_B_Rate54_24: 
				priv->pshare->phw->OFDMTxAgcOffset_B[7] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_B[6] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_B[5] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_B[4] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("OFDM_B[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_B[i]);
				break;
		case rTxAGC_B_Mcs03_Mcs00: 
				priv->pshare->phw->MCSTxAgcOffset_B[3] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_B[2] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_B[1] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_B[0] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("MCS_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);				
				break;
		case rTxAGC_B_Mcs07_Mcs04: 
				priv->pshare->phw->MCSTxAgcOffset_B[7] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_B[6] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_B[5] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_B[4] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("MCS_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);				
				break;
		case rTxAGC_B_Mcs11_Mcs08:
				priv->pshare->phw->MCSTxAgcOffset_B[11] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_B[10] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_B[9] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_B[8] = (char)tmp_byte[3];
				//for(i=8;i<12;i++)
				//	printk("MCS_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);				
				break;
		case rTxAGC_B_Mcs15_Mcs12:
				priv->pshare->phw->MCSTxAgcOffset_B[15] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_B[14] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_B[13] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_B[12] = (char)tmp_byte[3];
				//for(i=12;i<16;i++)
				//	printk("MCS_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);				
				break;
	}
}
#endif
#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
void set_power_by_rate_8814(struct rtl8192cd_priv *priv, unsigned int *u4bRegOffset, unsigned char *tmp_byte)
{
	unsigned char tmp_reg[4];
	unsigned int  reg_value = 0;
	unsigned char target = 0;
	int i;
	
	switch (*u4bRegOffset) {
		case rTxAGC_A_CCK11_CCK1_JAguar:
				priv->pshare->phw->CCKTxAgc_A[0] = (char)tmp_byte[0];
				priv->pshare->phw->CCKTxAgc_A[1] = (char)tmp_byte[1];
				priv->pshare->phw->CCKTxAgc_A[2] = (char)tmp_byte[2];
				priv->pshare->phw->CCKTxAgc_A[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
					//printk("priv->pshare->phw->CCKTxAgc_A[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_A[i]);
			break;
		case rTxAGC_A_Ofdm18_Ofdm6_JAguar:
				priv->pshare->phw->OFDMTxAgcOffset_A[0] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_A[1] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_A[2] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_A[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
					//printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_A[i]);
			break;
		case rTxAGC_A_Ofdm54_Ofdm24_JAguar: 
				priv->pshare->phw->OFDMTxAgcOffset_A[4] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_A[5] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_A[6] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_A[7] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
					//printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_A[i]);	
			break;
		case rTxAGC_A_MCS3_MCS0_JAguar: 
				priv->pshare->phw->MCSTxAgcOffset_A[0] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[1] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[2] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
					//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);						
			break;
		case rTxAGC_A_MCS7_MCS4_JAguar: 
				priv->pshare->phw->MCSTxAgcOffset_A[4] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[5] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[6] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[7] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
					//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);					
			break;
		case rTxAGC_A_MCS11_MCS8_JAguar:	
				priv->pshare->phw->MCSTxAgcOffset_A[8] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[9] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[10] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[11] = (char)tmp_byte[3];
				//for(i=8;i<12;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);					
			break;
		case rTxAGC_A_MCS15_MCS12_JAguar:
				priv->pshare->phw->MCSTxAgcOffset_A[12] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[13] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[14] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[15] = (char)tmp_byte[3];
				//for(i=12;i<16;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);						
			break;
#if defined(CONFIG_WLAN_HAL_8814AE)		
		case rTxAGC_A_MCS19_MCS16_JAguar:
				priv->pshare->phw->MCSTxAgcOffset_A[16] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[17] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[18] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[19] = (char)tmp_byte[3];
				//for(i=16;i<20;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
			break;
		case rTxAGC_A_MCS23_MCS20_JAguar:
				priv->pshare->phw->MCSTxAgcOffset_A[20] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_A[21] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_A[22] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_A[23] = (char)tmp_byte[3];
				//for(i=20;i<24;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
			break;
#endif
		case rTxAGC_A_Nss1Index3_Nss1Index0_JAguar: 
				priv->pshare->phw->VHTTxAgcOffset_A[0] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_A[1] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_A[2] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_A[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);					
			break;
		case rTxAGC_A_Nss1Index7_Nss1Index4_JAguar: 
				priv->pshare->phw->VHTTxAgcOffset_A[4] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_A[5] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_A[6] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_A[7] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);						
			break;
		case rTxAGC_A_Nss2Index1_Nss1Index8_JAguar: 
				priv->pshare->phw->VHTTxAgcOffset_A[8] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_A[9] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_A[10] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_A[11] = (char)tmp_byte[3];
				//for(i=8;i<12;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);
			break;
		case rTxAGC_A_Nss2Index5_Nss2Index2_JAguar: 
				priv->pshare->phw->VHTTxAgcOffset_A[12] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_A[13] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_A[14] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_A[15] = (char)tmp_byte[3];
				//for(i=12;i<16;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);					
			break;
		case rTxAGC_A_Nss2Index9_Nss2Index6_JAguar: 
				priv->pshare->phw->VHTTxAgcOffset_A[16] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_A[17] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_A[18] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_A[19] = (char)tmp_byte[3];
				//for(i=16;i<20;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);						
			break;
#if defined(CONFIG_WLAN_HAL_8814AE)					
		case rTxAGC_A_Nss3Index3_Nss3Index0_JAguar:
				priv->pshare->phw->VHTTxAgcOffset_A[20] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_A[21] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_A[22] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_A[23] = (char)tmp_byte[3];
				//for(i=20;i<24;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);
			break;
		case rTxAGC_A_Nss3Index7_Nss3Index4_JAguar:
				priv->pshare->phw->VHTTxAgcOffset_A[24] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_A[25] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_A[26] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_A[27] = (char)tmp_byte[3];
				//for(i=24;i<28;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);
			break;
		case rTxAGC_A_Nss3Index9_Nss3Index8_JAguar:
				priv->pshare->phw->VHTTxAgcOffset_A[28] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_A[29] = (char)tmp_byte[1];
				//for(i=28;i<30;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);
			break;
#endif
		case rTxAGC_B_CCK11_CCK1_JAguar:	
				priv->pshare->phw->CCKTxAgc_B[0] = (char)tmp_byte[0];
				priv->pshare->phw->CCKTxAgc_B[1] = (char)tmp_byte[1];
				priv->pshare->phw->CCKTxAgc_B[2] = (char)tmp_byte[2];
				priv->pshare->phw->CCKTxAgc_B[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->CCKTxAgc_B[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_B[i]);
			break;
		case rTxAGC_B_Ofdm18_Ofdm6_JAguar:	
				priv->pshare->phw->OFDMTxAgcOffset_B[0] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_B[1] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_B[2] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_B[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_B[i]);
			break;
		case rTxAGC_B_Ofdm54_Ofdm24_JAguar: 
				priv->pshare->phw->OFDMTxAgcOffset_B[4] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_B[5] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_B[6] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_B[7] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_B[i]);
			break;
		case rTxAGC_B_MCS3_MCS0_JAguar: 
				priv->pshare->phw->MCSTxAgcOffset_B[0] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_B[1] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_B[2] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_B[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);				
			break;
		case rTxAGC_B_MCS7_MCS4_JAguar: 
			priv->pshare->phw->MCSTxAgcOffset_B[4] = (char)tmp_byte[0];
			priv->pshare->phw->MCSTxAgcOffset_B[5] = (char)tmp_byte[1];
			priv->pshare->phw->MCSTxAgcOffset_B[6] = (char)tmp_byte[2];
			priv->pshare->phw->MCSTxAgcOffset_B[7] = (char)tmp_byte[3];
			//for(i=4;i<8;i++)
			//	printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);				
			break;
		case rTxAGC_B_MCS11_MCS8_JAguar:
			priv->pshare->phw->MCSTxAgcOffset_B[8] = (char)tmp_byte[0];
			priv->pshare->phw->MCSTxAgcOffset_B[9] = (char)tmp_byte[1];
			priv->pshare->phw->MCSTxAgcOffset_B[10] = (char)tmp_byte[2];
			priv->pshare->phw->MCSTxAgcOffset_B[11] = (char)tmp_byte[3];
			//for(i=8;i<12;i++)
			//	printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);				
			break;
		case rTxAGC_B_MCS15_MCS12_JAguar:
			priv->pshare->phw->MCSTxAgcOffset_B[12] = (char)tmp_byte[0];
			priv->pshare->phw->MCSTxAgcOffset_B[13] = (char)tmp_byte[1];
			priv->pshare->phw->MCSTxAgcOffset_B[14] = (char)tmp_byte[2];
			priv->pshare->phw->MCSTxAgcOffset_B[15] = (char)tmp_byte[3];
			//for(i=12;i<16;i++)
			//	printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);				
			break;
#if defined(CONFIG_WLAN_HAL_8814AE)	
		case rTxAGC_B_MCS19_MCS16_JAguar: 
			priv->pshare->phw->MCSTxAgcOffset_B[16] = (char)tmp_byte[0];
			priv->pshare->phw->MCSTxAgcOffset_B[17] = (char)tmp_byte[1];
			priv->pshare->phw->MCSTxAgcOffset_B[18] = (char)tmp_byte[2];
			priv->pshare->phw->MCSTxAgcOffset_B[19] = (char)tmp_byte[3];
			//for(i=16;i<20;i++)
			//	printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);				
			break;
		case rTxAGC_B_MCS23_MCS20_JAguar: 
			priv->pshare->phw->MCSTxAgcOffset_B[20] = (char)tmp_byte[0];
			priv->pshare->phw->MCSTxAgcOffset_B[21] = (char)tmp_byte[1];
			priv->pshare->phw->MCSTxAgcOffset_B[22] = (char)tmp_byte[2];
			priv->pshare->phw->MCSTxAgcOffset_B[23] = (char)tmp_byte[3];
			//for(i=20;i<24;i++)
			//	printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);				
			break;
#endif
		case rTxAGC_B_Nss1Index3_Nss1Index0_JAguar: 
			priv->pshare->phw->VHTTxAgcOffset_B[0] = (char)tmp_byte[0];
			priv->pshare->phw->VHTTxAgcOffset_B[1] = (char)tmp_byte[1];
			priv->pshare->phw->VHTTxAgcOffset_B[2] = (char)tmp_byte[2];
			priv->pshare->phw->VHTTxAgcOffset_B[3] = (char)tmp_byte[3];
			//for(i=0;i<4;i++)
			//	printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);						
			break;
		case rTxAGC_B_Nss1Index7_Nss1Index4_JAguar:
			priv->pshare->phw->VHTTxAgcOffset_B[4] = (char)tmp_byte[0];
			priv->pshare->phw->VHTTxAgcOffset_B[5] = (char)tmp_byte[1];
			priv->pshare->phw->VHTTxAgcOffset_B[6] = (char)tmp_byte[2];
			priv->pshare->phw->VHTTxAgcOffset_B[7] = (char)tmp_byte[3];
			//for(i=4;i<8;i++)
			//	printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);

			break;
		case rTxAGC_B_Nss2Index1_Nss1Index8_JAguar: 
			priv->pshare->phw->VHTTxAgcOffset_B[8] = (char)tmp_byte[0];
			priv->pshare->phw->VHTTxAgcOffset_B[9] = (char)tmp_byte[1];
			priv->pshare->phw->VHTTxAgcOffset_B[10] = (char)tmp_byte[2];
			priv->pshare->phw->VHTTxAgcOffset_B[11] = (char)tmp_byte[3];
			//for(i=8;i<12;i++)
			//	printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);	
			break;
		case rTxAGC_B_Nss2Index5_Nss2Index2_JAguar: 
			priv->pshare->phw->VHTTxAgcOffset_B[12] = (char)tmp_byte[0];
			priv->pshare->phw->VHTTxAgcOffset_B[13] = (char)tmp_byte[1];
			priv->pshare->phw->VHTTxAgcOffset_B[14] = (char)tmp_byte[2];
			priv->pshare->phw->VHTTxAgcOffset_B[15] = (char)tmp_byte[3];
			//for(i=12;i<16;i++)
			//	printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);	
			break;
		case rTxAGC_B_Nss2Index9_Nss2Index6_JAguar: 
			priv->pshare->phw->VHTTxAgcOffset_B[16] = (char)tmp_byte[0];
			priv->pshare->phw->VHTTxAgcOffset_B[17] = (char)tmp_byte[1];
			priv->pshare->phw->VHTTxAgcOffset_B[18] = (char)tmp_byte[2];
			priv->pshare->phw->VHTTxAgcOffset_B[19] = (char)tmp_byte[3];
			//for(i=16;i<20;i++)
			//	printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);	
			break;
#if defined(CONFIG_WLAN_HAL_8814AE)				
		case rTxAGC_B_Nss3Index3_Nss3Index0_JAguar: 
			priv->pshare->phw->VHTTxAgcOffset_B[20] = (char)tmp_byte[0];
			priv->pshare->phw->VHTTxAgcOffset_B[21] = (char)tmp_byte[1];
			priv->pshare->phw->VHTTxAgcOffset_B[22] = (char)tmp_byte[2];
			priv->pshare->phw->VHTTxAgcOffset_B[23] = (char)tmp_byte[3];
			//for(i=20;i<24;i++)
			//	printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);	
			break;
		case rTxAGC_B_Nss3Index7_Nss3Index4_JAguar: 
			priv->pshare->phw->VHTTxAgcOffset_B[24] = (char)tmp_byte[0];
			priv->pshare->phw->VHTTxAgcOffset_B[25] = (char)tmp_byte[1];
			priv->pshare->phw->VHTTxAgcOffset_B[26] = (char)tmp_byte[2];
			priv->pshare->phw->VHTTxAgcOffset_B[27] = (char)tmp_byte[3];
			//for(i=24;i<28;i++)
			//	printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);	
			break;
		case rTxAGC_B_Nss3Index9_Nss3Index8_JAguar: 
			priv->pshare->phw->VHTTxAgcOffset_B[28] = (char)tmp_byte[0];
			priv->pshare->phw->VHTTxAgcOffset_B[29] = (char)tmp_byte[1];
			//for(i=28;i<30;i++)
			//	printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);	
			break;
		case rTxAGC_C_CCK11_CCK1_JAguar:
				priv->pshare->phw->CCKTxAgc_C[0] = (char)tmp_byte[0];
				priv->pshare->phw->CCKTxAgc_C[1] = (char)tmp_byte[1];
				priv->pshare->phw->CCKTxAgc_C[2] = (char)tmp_byte[2];
				priv->pshare->phw->CCKTxAgc_C[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->CCKTxAgc_C[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_C[i]);
			break;
		case rTxAGC_C_Ofdm18_Ofdm6_JAguar:
				priv->pshare->phw->OFDMTxAgcOffset_C[0] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_C[1] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_C[2] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_C[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->OFDMTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_C[i]);
			break;
		case rTxAGC_C_Ofdm54_Ofdm24_JAguar: 
				priv->pshare->phw->OFDMTxAgcOffset_C[4] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_C[5] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_C[6] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_C[7] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("priv->pshare->phw->OFDMTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_C[i]);	
			break;
		case rTxAGC_C_MCS3_MCS0_JAguar: 
				priv->pshare->phw->MCSTxAgcOffset_C[0] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_C[1] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_C[2] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_C[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_C[i]);						
			break;
		case rTxAGC_C_MCS7_MCS4_JAguar: 
				priv->pshare->phw->MCSTxAgcOffset_C[4] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_C[5] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_C[6] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_C[7] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
					//printk("priv->pshare->phw->MCSTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_C[i]);					
			break;
		case rTxAGC_C_MCS11_MCS8_JAguar:	
				priv->pshare->phw->MCSTxAgcOffset_C[8] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_C[9] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_C[10] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_C[11] = (char)tmp_byte[3];
				//for(i=8;i<12;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_C[i]);					
			break;
		case rTxAGC_C_MCS15_MCS12_JAguar:
				priv->pshare->phw->MCSTxAgcOffset_C[12] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_C[13] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_C[14] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_C[15] = (char)tmp_byte[3];
				//for(i=12;i<16;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_C[i]);						
			break;
		case rTxAGC_C_MCS19_MCS16_JAguar:
				priv->pshare->phw->MCSTxAgcOffset_C[16] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_C[17] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_C[18] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_C[19] = (char)tmp_byte[3];
				//for(i=16;i<20;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_C[i]);
			break;
		case rTxAGC_C_MCS23_MCS20_JAguar:
				priv->pshare->phw->MCSTxAgcOffset_C[20] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_C[21] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_C[22] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_C[23] = (char)tmp_byte[3];
				//for(i=20;i<24;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_C[i]);
			break;
		case rTxAGC_C_Nss1Index3_Nss1Index0_JAguar: 
				priv->pshare->phw->VHTTxAgcOffset_C[0] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_C[1] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_C[2] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_C[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_C[i]);					
			break;
		case rTxAGC_C_Nss1Index7_Nss1Index4_JAguar: 
				priv->pshare->phw->VHTTxAgcOffset_C[4] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_C[5] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_C[6] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_C[7] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_C[i]);						
			break;
		case rTxAGC_C_Nss2Index1_Nss1Index8_JAguar: 
				priv->pshare->phw->VHTTxAgcOffset_C[8] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_C[9] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_C[10] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_C[11] = (char)tmp_byte[3];
				//for(i=8;i<12;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_C[i]);
			break;
		case rTxAGC_C_Nss2Index5_Nss2Index2_JAguar: 
				priv->pshare->phw->VHTTxAgcOffset_C[12] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_C[13] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_C[14] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_C[15] = (char)tmp_byte[3];
				//for(i=12;i<16;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_C[i]);					
			break;
		case rTxAGC_C_Nss2Index9_Nss2Index6_JAguar: 
				priv->pshare->phw->VHTTxAgcOffset_C[16] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_C[17] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_C[18] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_C[19] = (char)tmp_byte[3];
				//for(i=16;i<20;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_C[i]);						
			break;
		case rTxAGC_C_Nss3Index3_Nss3Index0_JAguar:
				priv->pshare->phw->VHTTxAgcOffset_C[20] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_C[21] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_C[22] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_C[23] = (char)tmp_byte[3];
				//for(i=20;i<24;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_C[i]);
			break;
		case rTxAGC_C_Nss3Index7_Nss3Index4_JAguar:
				priv->pshare->phw->VHTTxAgcOffset_C[24] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_C[25] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_C[26] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_C[27] = (char)tmp_byte[3];
				//for(i=24;i<28;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_C[i]);
			break;
		case rTxAGC_C_Nss3Index9_Nss3Index8_JAguar:
				priv->pshare->phw->VHTTxAgcOffset_C[28] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_C[29] = (char)tmp_byte[1];
				//for(i=28;i<30;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_C[i]);
			break;
		case rTxAGC_D_CCK11_CCK1_JAguar:
				priv->pshare->phw->CCKTxAgc_D[0] = (char)tmp_byte[0];
				priv->pshare->phw->CCKTxAgc_D[1] = (char)tmp_byte[1];
				priv->pshare->phw->CCKTxAgc_D[2] = (char)tmp_byte[2];
				priv->pshare->phw->CCKTxAgc_D[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->CCKTxAgc_D[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_D[i]);
			break;
		case rTxAGC_D_Ofdm18_Ofdm6_JAguar:
				priv->pshare->phw->OFDMTxAgcOffset_D[0] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_D[1] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_D[2] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_D[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->OFDMTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_D[i]);
			break;
		case rTxAGC_D_Ofdm54_Ofdm24_JAguar: 
				priv->pshare->phw->OFDMTxAgcOffset_D[4] = (char)tmp_byte[0];
				priv->pshare->phw->OFDMTxAgcOffset_D[5] = (char)tmp_byte[1];
				priv->pshare->phw->OFDMTxAgcOffset_D[6] = (char)tmp_byte[2];
				priv->pshare->phw->OFDMTxAgcOffset_D[7] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("priv->pshare->phw->OFDMTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_D[i]);	
			break;
		case rTxAGC_D_MCS3_MCS0_JAguar: 
				priv->pshare->phw->MCSTxAgcOffset_D[0] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_D[1] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_D[2] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_D[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_D[i]);						
			break;
		case rTxAGC_D_MCS7_MCS4_JAguar: 
				priv->pshare->phw->MCSTxAgcOffset_D[4] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_D[5] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_D[6] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_D[7] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_D[i]);					
			break;
		case rTxAGC_D_MCS11_MCS8_JAguar:	
				priv->pshare->phw->MCSTxAgcOffset_D[8] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_D[9] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_D[10] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_D[11] = (char)tmp_byte[3];
				//for(i=8;i<12;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_D[i]);					
			break;
		case rTxAGC_D_MCS15_MCS12_JAguar:
				priv->pshare->phw->MCSTxAgcOffset_D[12] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_D[13] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_D[14] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_D[15] = (char)tmp_byte[3];
				//for(i=12;i<16;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_D[i]);						
			break;
		case rTxAGC_D_MCS19_MCS16_JAguar:
				priv->pshare->phw->MCSTxAgcOffset_D[16] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_D[17] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_D[18] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_D[19] = (char)tmp_byte[3];
				//for(i=16;i<20;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_D[i]);
			break;
		case rTxAGC_D_MCS23_MCS20_JAguar:
				priv->pshare->phw->MCSTxAgcOffset_D[20] = (char)tmp_byte[0];
				priv->pshare->phw->MCSTxAgcOffset_D[21] = (char)tmp_byte[1];
				priv->pshare->phw->MCSTxAgcOffset_D[22] = (char)tmp_byte[2];
				priv->pshare->phw->MCSTxAgcOffset_D[23] = (char)tmp_byte[3];
				//for(i=20;i<24;i++)
				//	printk("priv->pshare->phw->MCSTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_D[i]);
			break;
		case rTxAGC_D_Nss1Index3_Nss1Index0_JAguar: 
				priv->pshare->phw->VHTTxAgcOffset_D[0] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_D[1] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_D[2] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_D[3] = (char)tmp_byte[3];
				//for(i=0;i<4;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_D[i]);					
			break;
		case rTxAGC_D_Nss1Index7_Nss1Index4_JAguar: 
				priv->pshare->phw->VHTTxAgcOffset_D[4] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_D[5] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_D[6] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_D[7] = (char)tmp_byte[3];
				//for(i=4;i<8;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_D[i]);						
			break;
		case rTxAGC_D_Nss2Index1_Nss1Index8_JAguar: 
				priv->pshare->phw->VHTTxAgcOffset_D[8] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_D[9] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_D[10] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_D[11] = (char)tmp_byte[3];
				//for(i=8;i<12;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_D[i]);
			break;
		case rTxAGC_D_Nss2Index5_Nss2Index2_JAguar: 
				priv->pshare->phw->VHTTxAgcOffset_D[12] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_D[13] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_D[14] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_D[15] = (char)tmp_byte[3];
				//for(i=12;i<16;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_D[i]);					
			break;
		case rTxAGC_D_Nss2Index9_Nss2Index6_JAguar: 
				priv->pshare->phw->VHTTxAgcOffset_D[16] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_D[17] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_D[18] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_D[19] = (char)tmp_byte[3];
				//for(i=16;i<20;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_D[i]);						
			break;
		case rTxAGC_D_Nss3Index3_Nss3Index0_JAguar:
				priv->pshare->phw->VHTTxAgcOffset_D[20] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_D[21] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_D[22] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_D[23] = (char)tmp_byte[3];
				//for(i=20;i<24;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_D[i]);
			break;
		case rTxAGC_D_Nss3Index7_Nss3Index4_JAguar:
				priv->pshare->phw->VHTTxAgcOffset_D[24] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_D[25] = (char)tmp_byte[1];
				priv->pshare->phw->VHTTxAgcOffset_D[26] = (char)tmp_byte[2];
				priv->pshare->phw->VHTTxAgcOffset_D[27] = (char)tmp_byte[3];
				//for(i=24;i<28;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_D[i]);
			break;
		case rTxAGC_D_Nss3Index9_Nss3Index8_JAguar:
				priv->pshare->phw->VHTTxAgcOffset_D[28] = (char)tmp_byte[0];
				priv->pshare->phw->VHTTxAgcOffset_D[29] = (char)tmp_byte[1];
				//for(i=28;i<30;i++)
				//	printk("priv->pshare->phw->VHTTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_D[i]);
			break;
#endif
	}
}
#endif


static int find_str(char *line, char *str)
{
	int len=0, idx=0;
	char *ch = NULL;

	if (!line ||!str || (strlen(str)>strlen(line)))
		return 0;
	
	ch = line;
	while (1) {
		if (strlen(line) - strlen(str) < (idx+1))
			break;
		
		if (!memcmp(&(line[idx]), str, strlen(str))) {
			return idx;
			//break;
		}
		idx++;
	}

	return 0;
}

#if 0
static int get_target_val_new(struct rtl8192cd_priv *priv, unsigned char *line_head, unsigned int *u4bRegOffset, unsigned int *u4bMask, unsigned int *u4bRegValue)
{
	int base, idx;
	unsigned char *ch, *next, *pMask, *pValue;
	extern int _atoi(char *s, int base);

	unsigned char *b3, *b2, *b1, *b0;
	unsigned char tmp_byte[4];
	unsigned char lable = 0;		//lable = {0: none, 1: 1Tx, 2: 2Tx}


	*u4bRegOffset = *u4bRegValue = *u4bMask = '\0';

	ch = line_head;
	
	while (1) {
		if ((*ch == '\0') || (*ch == '\n') || (*ch == '\r'))
			break;
		else if (*ch == '/') {
			*ch = '\0';
			break;
		} else {
			ch++;
		}
	}

	if ((!memcmp(line_head, "0xff", 4)) || (!memcmp(line_head, "0XFF", 4)))
		return 0;

	if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2)))
	{
		base = 16;
		idx = 2;
	}
	else if (!memcmp(line_head, "[1Tx]", 5))
	{
		lable = 1;
		idx = find_str(line_head, "0x");
	}
	else if (!memcmp(line_head, "[2Tx]", 5))
	{
		lable = 2;
		idx = find_str(line_head, "0x");
	}
	else
	{
		return 0;
	}

	if (lable)
	{
		*u4bRegOffset = _atoi((char *)&line_head[idx+2], base);
	
		next = &(line_head[idx+2]);
		pMask = get_digit(&next);
		if (pMask) {
			//*u4bMask = _atoi((char *)&pMask[2], 16);
				
			if ((b3 = get_digit_dot(&pMask)) == NULL)
				return 0;
			//panic_printk("\nb3:%s\n", b3);				
			if ((b2 = get_digit_dot(&b3)) == NULL)
				return 0;
			//panic_printk("b2:%s\n", b2);
			if ((b1 = get_digit_dot(&b2)) == NULL)
				return 0;
			//panic_printk("b1:%s\n", b1);

			if ((b0 = get_digit_dot(&b1)) == NULL)
				return 0;
			//panic_printk("b0:%s\n", b0);
		}								
	}
	else
	{
		*u4bRegOffset = _atoi((char *)&line_head[idx], base);

		b3 = get_digit_dot(&line_head);			
		if (b3 == NULL)
			return 0;
		//panic_printk("b3:%s\n", b3);	
		b2 = get_digit_dot(&b3);
		if (b2 == NULL)
			return 0;
		//panic_printk("b2:%s\n", b2);
		b1 = get_digit_dot(&b2);
		if (b1 == NULL)
			return 0;
		//panic_printk("b1:%s\n", b1);	
		b0 = get_digit_dot(&b1);
		if (b0 == NULL)
			return 0;
		//panic_printk("b0:%s\n", b0);
	}

	base = 10;
	idx = 0;

#if 1
	tmp_byte[3] = _convert_2_pwr_dot(b3, base);
	tmp_byte[2] = _convert_2_pwr_dot(b2, base);
	tmp_byte[1] = _convert_2_pwr_dot(b1, base);
	tmp_byte[0] = _convert_2_pwr_dot(b0, base);
#else
	tmp_byte[3] = _atoi(b3, base);
	tmp_byte[2] = _atoi(b2, base);
	tmp_byte[1] = _atoi(b1, base);
	tmp_byte[0] = _atoi(b0, base);
#endif

	//printk("[%s] 0x%x: %d %d %d %d\n", __FUNCTION__, *u4bRegOffset, tmp_byte[3], tmp_byte[2], tmp_byte[1], tmp_byte[0]);

#if 0
	panic_printk("[0x%x] tmp_byte = %02d-%02d-%02d-%02d\n", *u4bRegOffset, 
		tmp_byte[3], tmp_byte[2], tmp_byte[1], tmp_byte[0]);
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)//CONFIG_RTL_88E_SUPPORT
	if(GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv)==VERSION_8192E)
		assign_target_value_88e_new(priv, u4bRegOffset, tmp_byte);
#endif
#if defined( CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	if(GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8881A)
	assign_target_value_8812_new(priv, u4bRegOffset, tmp_byte);
#endif
	return 1;
}
#endif

static int get_offset_mask_val_new(struct rtl8192cd_priv *priv, unsigned char *line_head, unsigned int *u4bRegOffset, unsigned int *u4bMask, unsigned int *u4bRegValue)
{
	int base, idx, round = 0;
	unsigned char *ch, *next, *pMask, *pValue;
	extern int _atoi(char *s, int base);

	unsigned char *b3, *b2, *b1, *b0;
	unsigned char tmp_byte[4];
	unsigned char lable = 0;		//lable = {0: none, 1: 1Tx, 2: 2Tx}

	*u4bRegOffset = *u4bRegValue = *u4bMask = '\0';

	ch = line_head;
	
	while (1) {
		if ((*ch == '\0') || (*ch == '\n') || (*ch == '\r'))
			break;
		else if (*ch == '/') {
			*ch = '\0';
			break;
		} else {
			ch++;
		}

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	if ((!memcmp(line_head, "0xffff", 6)) || (!memcmp(line_head, "0XFFFF", 6))){
		*u4bRegOffset=0xffff;
		return 1;
	}
	if ((!memcmp(line_head, "0xff", 4)) || (!memcmp(line_head, "0XFF", 4)))
		return 0;

	if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2)))
	{
		idx = 2;
	}
	else if (!memcmp(line_head, "[1Tx]", 5))
	{
		lable = 1;
		idx = find_str(line_head, "0x");
	}
	else if (!memcmp(line_head, "[2Tx]", 5))
	{
		lable = 2;
		idx = find_str(line_head, "0x");
	}
	else if (!memcmp(line_head, "[3Tx]", 5))
	{
		lable = 3;
		idx = find_str(line_head, "0x");
	}
	else
	{
		return 0;
	}
	base = 16;

	if (lable) /* For 8814 */
	{
		next = &(line_head[idx+2]);
		pMask = get_digit(&next);
		if (NULL == pMask)
			return 0;
		*u4bRegOffset = _atoi((char *)next, base);
		
		next = &pMask[2];
		b3 = get_digit(&next);
		*u4bMask = _atoi((char *)next, 16);

		if (b3 == NULL)
			return 0;
		//panic_printk("b3:%s,", b3);

		b2 = get_digit_dot(&b3);
		if (b2 == NULL)
			return 0;
		//panic_printk("b2:%s,", b2);
		
		b1 = get_digit_dot(&b2);
		if (b1 == NULL)
			return 0;
		//panic_printk("b1:%s,", b1);
		
		b0 = get_digit_dot(&b1);
		if (b0 == NULL)
			return 0;
		//panic_printk("b0:%s\n", b0);
	}
	else
	{
		*u4bRegOffset = _atoi((char *)&line_head[idx], base);
		if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8192E))
		*u4bMask = 0xffffffff;
		else if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8881A)
		*u4bMask = 0x7f7f7f7f;

		b3 = get_digit_dot(&line_head);
		
		if (b3 == NULL)
			return 0;
		
		b2 = get_digit_dot(&b3);

		if (b2 == NULL)
			return 0;
		
		b1 = get_digit_dot(&b2);

		if (b1 == NULL)
			return 0;
		
		b0 = get_digit_dot(&b1);

		if (b0 == NULL)
			return 0;
	}
	
	base = 10;
	idx = 0;

#if 1
	tmp_byte[3] = _convert_2_pwr_dot(b3, base);
	tmp_byte[2] = _convert_2_pwr_dot(b2, base);
	tmp_byte[1] = _convert_2_pwr_dot(b1, base);
	tmp_byte[0] = _convert_2_pwr_dot(b0, base);
#else
	tmp_byte[3] = _atoi(b3, base);
	tmp_byte[2] = _atoi(b2, base);
	tmp_byte[1] = _atoi(b1, base);
	tmp_byte[0] = _atoi(b0, base);
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_RTL_8723B_SUPPORT)
	if(GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8723B)
	*u4bRegValue = generate_u4bRegValue_88e_new(priv, u4bRegOffset, tmp_byte);
#endif
#if defined(CONFIG_WLAN_HAL_8197F)
	if(GET_CHIP_VER(priv) == VERSION_8197F && !priv->pshare->rf_ft_var.disable_pwr_by_rate)
		set_power_by_rate(priv, u4bRegOffset, tmp_byte);
#endif
#if defined( CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	if(GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8881A) 
		*u4bRegValue = generate_u4bRegValue_8812_new(priv, u4bRegOffset, tmp_byte);
#endif
#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
	if((GET_CHIP_VER(priv) == VERSION_8814A && !priv->pshare->rf_ft_var.disable_pwr_by_rate)||
             (GET_CHIP_VER(priv) == VERSION_8822B && !priv->pshare->rf_ft_var.disable_pwr_by_rate))
		set_power_by_rate_8814(priv, u4bRegOffset, tmp_byte);
#endif	
#if 0
	panic_printk("[0x%x] tmp_byte = %02d-%02d-%02d-%02d ", *u4bRegOffset, 
		tmp_byte[3], tmp_byte[2], tmp_byte[1], tmp_byte[0]);

	panic_printk(" [0x%08x]\n", *u4bRegValue);
#endif

	return 1;
}



static int get_offset_mask_val(unsigned char *line_head, unsigned int *u4bRegOffset, unsigned int *u4bMask, unsigned int *u4bRegValue)
{
	unsigned char *next, *n1;
	int base, idx;
	int num = 0, round = 0;
	unsigned char *ch;
	extern int _atoi(char * s, int base);

	*u4bRegOffset = *u4bRegValue = *u4bMask = '\0';

	ch = line_head;
	while (1) {
		if ((*ch == '\0') || (*ch == '\n') || (*ch == '\r'))
			break;
		else if (*ch == '/') {
			*ch = '\0';
			break;
		} else {
			ch++;
		}

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	next = get_digit(&line_head);
	if (next == NULL)
		return num;
	num++;
	if ((!memcmp(line_head, "0x", 2)) || (!memcmp(line_head, "0X", 2))) {
		base = 16;
		idx = 2;
	} else {
		base = 10;
		idx = 0;
	}
	*u4bRegOffset = _atoi((char *)&line_head[idx], base);

	if (next) {
		n1 = get_digit(&next);
		if (n1 == NULL)
			return num;
		num++;
		if ((!memcmp(next, "0x", 2)) || (!memcmp(next, "0X", 2))) {
			base = 16;
			idx = 2;
		} else {
			base = 10;
			idx = 0;
		}
		*u4bMask = _atoi((char *)&next[idx], base);

		if (n1) {
			if (!get_digit(&n1))
				return num;
			num++;
			if ((!memcmp(n1, "0x", 2)) || (!memcmp(n1, "0X", 2))) {
				base = 16;
				idx = 2;
			} else {
				base = 10;
				idx = 0;
			}
			*u4bRegValue = _atoi((char *)&n1[idx], base);
		} else
			*u4bRegValue = 0;
	} else
		*u4bMask = 0;

	return num;
}


unsigned char *get_line(unsigned char **line)
{
	unsigned char *p = *line;

	while (*p && ((*p == '\n') || (*p == '\r')))
		p++;

	if (*p == '\0') {
		*line = NULL;
		return NULL;
	}
	*line = p;

	while (*p && (*p != '\n') && (*p != '\r'))
		p++;

	*p = '\0';
	return p + 1;
}

int ch2idx(int ch)
{
	int val = -1;
	// |1~14|36, 38, 40, ..., 64|100, 102, ..., 140|149, 151, ..., 165|
	if (ch <= 14)
		val = ch - 1;
	else if (ch <= 64)
		val = ((ch - 36) >> 1) + 14;
	else if (ch <= 140)
		val = ((ch - 100) >> 1) + 29;
	else if (ch <= 165)
		val = ((ch - 149) >> 1) + 50;
	else if (ch <= 177) 
		val = ((ch - 169) >> 1) + 59;

	return val;
}

#ifdef TXPWR_LMT
#ifdef TXPWR_LMT_NEWFILE
void find_pwr_limit_new(struct rtl8192cd_priv *priv, int channel, int offset)
{
    int chidx_20 = 0,  chidx_40 = 0, chidx_80 = 0;	
    int working_channel_40m = 0;
    int working_channel_80m = get_center_channel(priv, channel, offset,1);

    working_channel_40m = (priv->pshare->CurrentChannelBW ? ((offset==1) ? (channel-2) : (channel+2)) : channel);

    chidx_20 = ch2idx(channel);
    chidx_40 = ch2idx(working_channel_40m);	
    chidx_80 = ch2idx(working_channel_80m);		

#if 0
    panic_printk("[BW%dM]: central channel %d %d %d ==> index %d %d %d:\n", 
        20*(1 << priv->pshare->CurrentChannelBW),		 
        channel, working_channel_40m, working_channel_80m,
        chidx_20, chidx_40, chidx_80);
#endif

	if ((chidx_20>=0) && (chidx_40>=0) && (chidx_80>=0)) {
			priv->pshare->txpwr_lmt_CCK = priv->pshare->ch_pwr_lmtCCK[chidx_20];
			priv->pshare->txpwr_lmt_OFDM = priv->pshare->ch_pwr_lmtOFDM[chidx_20];
			
		if (priv->pshare->CurrentChannelBW == 2){ //BW=80M
			priv->pshare->txpwr_lmt_HT1S = priv->pshare->ch_pwr_lmtHT40_1S[chidx_40];
			priv->pshare->txpwr_lmt_HT2S = priv->pshare->ch_pwr_lmtHT40_2S[chidx_40];
			priv->pshare->txpwr_lmt_VHT1S = priv->pshare->ch_pwr_lmtVHT80_1S[chidx_80];
			priv->pshare->txpwr_lmt_VHT2S = priv->pshare->ch_pwr_lmtVHT80_2S[chidx_80];
#if defined(CONFIG_WLAN_HAL_8814AE)
			priv->pshare->txpwr_lmt_HT3S = priv->pshare->ch_pwr_lmtHT40_3S[chidx_40];
			priv->pshare->txpwr_lmt_HT4S = priv->pshare->ch_pwr_lmtHT40_4S[chidx_40];
			priv->pshare->txpwr_lmt_VHT3S = priv->pshare->ch_pwr_lmtVHT80_3S[chidx_80];
			priv->pshare->txpwr_lmt_VHT4S = priv->pshare->ch_pwr_lmtVHT80_4S[chidx_80];
#endif		
#ifdef BEAMFORMING_AUTO
			priv->pshare->txpwr_lmt_TXBF_HT1S = priv->pshare->ch_pwr_lmtHT40_TXBF_1S[chidx_40];
			priv->pshare->txpwr_lmt_TXBF_HT2S = priv->pshare->ch_pwr_lmtHT40_TXBF_2S[chidx_40];
			priv->pshare->txpwr_lmt_TXBF_VHT1S = priv->pshare->ch_pwr_lmtVHT80_TXBF_1S[chidx_80];
			priv->pshare->txpwr_lmt_TXBF_VHT2S = priv->pshare->ch_pwr_lmtVHT80_TXBF_2S[chidx_80];
#if defined(CONFIG_WLAN_HAL_8814AE)
			priv->pshare->txpwr_lmt_TXBF_HT3S = priv->pshare->ch_pwr_lmtHT40_TXBF_3S[chidx_40];
			priv->pshare->txpwr_lmt_TXBF_HT4S = priv->pshare->ch_pwr_lmtHT40_TXBF_4S[chidx_40];
			priv->pshare->txpwr_lmt_TXBF_VHT3S = priv->pshare->ch_pwr_lmtVHT80_TXBF_3S[chidx_80];
			priv->pshare->txpwr_lmt_TXBF_VHT4S = priv->pshare->ch_pwr_lmtVHT80_TXBF_4S[chidx_80];
#endif
#endif // BEAMFORMING_AUTO
		} else if (priv->pshare->CurrentChannelBW == 1){ //BW=40M
			priv->pshare->txpwr_lmt_HT1S = priv->pshare->ch_pwr_lmtHT40_1S[chidx_40];
			priv->pshare->txpwr_lmt_HT2S = priv->pshare->ch_pwr_lmtHT40_2S[chidx_40];
			priv->pshare->txpwr_lmt_VHT1S = priv->pshare->ch_pwr_lmtHT40_1S[chidx_40];
			priv->pshare->txpwr_lmt_VHT2S = priv->pshare->ch_pwr_lmtHT40_2S[chidx_40];
#if defined(CONFIG_WLAN_HAL_8814AE)
			priv->pshare->txpwr_lmt_HT3S = priv->pshare->ch_pwr_lmtHT40_3S[chidx_40];
			priv->pshare->txpwr_lmt_HT4S = priv->pshare->ch_pwr_lmtHT40_4S[chidx_40];
			priv->pshare->txpwr_lmt_VHT3S = priv->pshare->ch_pwr_lmtHT40_3S[chidx_40];
			priv->pshare->txpwr_lmt_VHT4S = priv->pshare->ch_pwr_lmtHT40_4S[chidx_40];
#endif			
#ifdef BEAMFORMING_AUTO
			priv->pshare->txpwr_lmt_TXBF_HT1S = priv->pshare->ch_pwr_lmtHT40_TXBF_1S[chidx_40];
			priv->pshare->txpwr_lmt_TXBF_HT2S = priv->pshare->ch_pwr_lmtHT40_TXBF_2S[chidx_40];
			priv->pshare->txpwr_lmt_TXBF_VHT1S = priv->pshare->ch_pwr_lmtHT40_TXBF_1S[chidx_40];
			priv->pshare->txpwr_lmt_TXBF_VHT2S = priv->pshare->ch_pwr_lmtHT40_TXBF_2S[chidx_40];
#if defined(CONFIG_WLAN_HAL_8814AE)
			priv->pshare->txpwr_lmt_TXBF_HT3S = priv->pshare->ch_pwr_lmtHT40_TXBF_3S[chidx_40];
			priv->pshare->txpwr_lmt_TXBF_HT4S = priv->pshare->ch_pwr_lmtHT40_TXBF_4S[chidx_40];
			priv->pshare->txpwr_lmt_TXBF_VHT3S = priv->pshare->ch_pwr_lmtHT40_TXBF_3S[chidx_40];
			priv->pshare->txpwr_lmt_TXBF_VHT4S = priv->pshare->ch_pwr_lmtHT40_TXBF_4S[chidx_40];
#endif
#endif // BEAMFORMING_AUTO
		} else { //BW=20M
			priv->pshare->txpwr_lmt_HT1S = priv->pshare->ch_pwr_lmtHT20_1S[chidx_20];
			priv->pshare->txpwr_lmt_HT2S = priv->pshare->ch_pwr_lmtHT20_2S[chidx_20];
			priv->pshare->txpwr_lmt_VHT1S = priv->pshare->ch_pwr_lmtHT20_1S[chidx_20];
			priv->pshare->txpwr_lmt_VHT2S = priv->pshare->ch_pwr_lmtHT20_2S[chidx_20];
#if defined(CONFIG_WLAN_HAL_8814AE)
			priv->pshare->txpwr_lmt_HT3S = priv->pshare->ch_pwr_lmtHT20_3S[chidx_20];
			priv->pshare->txpwr_lmt_HT4S = priv->pshare->ch_pwr_lmtHT20_4S[chidx_20];
			priv->pshare->txpwr_lmt_VHT3S = priv->pshare->ch_pwr_lmtHT20_3S[chidx_20];
			priv->pshare->txpwr_lmt_VHT4S = priv->pshare->ch_pwr_lmtHT20_4S[chidx_20];
#endif			
#ifdef BEAMFORMING_AUTO
			priv->pshare->txpwr_lmt_TXBF_HT1S = priv->pshare->ch_pwr_lmtHT20_TXBF_1S[chidx_20];
			priv->pshare->txpwr_lmt_TXBF_HT2S = priv->pshare->ch_pwr_lmtHT20_TXBF_2S[chidx_20];
			priv->pshare->txpwr_lmt_TXBF_VHT1S = priv->pshare->ch_pwr_lmtHT20_TXBF_1S[chidx_20];
			priv->pshare->txpwr_lmt_TXBF_VHT2S = priv->pshare->ch_pwr_lmtHT20_TXBF_2S[chidx_20];
#if defined(CONFIG_WLAN_HAL_8814AE)
			priv->pshare->txpwr_lmt_TXBF_HT3S = priv->pshare->ch_pwr_lmtHT20_TXBF_3S[chidx_20];
			priv->pshare->txpwr_lmt_TXBF_HT4S = priv->pshare->ch_pwr_lmtHT20_TXBF_4S[chidx_20];
			priv->pshare->txpwr_lmt_TXBF_VHT3S = priv->pshare->ch_pwr_lmtHT20_TXBF_3S[chidx_20];
			priv->pshare->txpwr_lmt_TXBF_VHT4S = priv->pshare->ch_pwr_lmtHT20_TXBF_4S[chidx_20];
#endif			

#endif // BEAMFORMING_AUTO
		}
#if defined CONFIG_WLAN_HAL_8814AE
		if(priv->pmib->dot11RFEntry.tx3path){
#if 0			
			if (priv->pshare->txpwr_lmt_CCK)
				priv->pshare->txpwr_lmt_CCK -= 9;
			if (priv->pshare->txpwr_lmt_OFDM)
				priv->pshare->txpwr_lmt_OFDM -= 9;
			if (priv->pshare->txpwr_lmt_HT1S)
				priv->pshare->txpwr_lmt_HT1S -= 9;	
			if (priv->pshare->txpwr_lmt_VHT1S)
				priv->pshare->txpwr_lmt_VHT1S -= 9;
			if (priv->pshare->txpwr_lmt_HT2S)
				priv->pshare->txpwr_lmt_HT2S -= 6;	
			if (priv->pshare->txpwr_lmt_VHT2S)
				priv->pshare->txpwr_lmt_VHT2S -= 6;
#endif			
		} else
#endif

#if defined(CONFIG_WLAN_HAL_8197F)
        if(GET_CHIP_VER(priv) != VERSION_8197F) /* 97F doesn't need to do this, already handled by power lmt table*/
#endif
        {
            if (priv->pmib->dot11RFEntry.tx2path && !priv->pshare->rf_ft_var.disable_txpwrlmt2path) {
                if (priv->pshare->txpwr_lmt_CCK)
                    priv->pshare->txpwr_lmt_CCK -= 6;
                if (priv->pshare->txpwr_lmt_OFDM)
                    priv->pshare->txpwr_lmt_OFDM -= 6;
                if (priv->pshare->txpwr_lmt_HT1S)
                    priv->pshare->txpwr_lmt_HT1S -= 6;	
                if (priv->pshare->txpwr_lmt_VHT1S)
                    priv->pshare->txpwr_lmt_VHT1S -= 6;
            }
        }
    } else {
        priv->pshare->txpwr_lmt_CCK = 0;
        priv->pshare->txpwr_lmt_OFDM = 0;
        priv->pshare->txpwr_lmt_HT1S = 0;
        priv->pshare->txpwr_lmt_HT2S = 0;
        priv->pshare->txpwr_lmt_VHT1S = 0;
        priv->pshare->txpwr_lmt_VHT2S = 0;
#if defined(CONFIG_WLAN_HAL_8814AE)
        priv->pshare->txpwr_lmt_HT3S = 0;
        priv->pshare->txpwr_lmt_HT4S = 0;
        priv->pshare->txpwr_lmt_VHT3S = 0;
        priv->pshare->txpwr_lmt_VHT4S = 0;
#endif			
	if(channel <= 165)
        printk("Cannot map current working channel (%d) to find power limit!\n", channel);
    }
	
}

#endif

void find_pwr_limit(struct rtl8192cd_priv *priv, int channel, int offset)
{
	int chidx_20 = 0,  chidx_40 = 0;
	int working_channel_40m = get_center_channel(priv, channel, offset,1);

	chidx_20 = ch2idx(channel);
	chidx_40 = ch2idx(working_channel_40m);	

#if 0
	panic_printk("[BW%dM]: central channel[ %d %d ]==> index[ %d %d %d ]\n", 
			20*(1 << priv->pshare->CurrentChannelBW),		 
			channel, working_channel_40m,
			chidx_20, chidx_40);
#endif

	if ((chidx_20 >= 0) && (chidx_40 >= 0)) {
		priv->pshare->txpwr_lmt_CCK = priv->pshare->ch_pwr_lmtCCK[chidx_20];
		priv->pshare->txpwr_lmt_OFDM = priv->pshare->ch_pwr_lmtOFDM[chidx_20];
		priv->pshare->tgpwr_CCK = priv->pshare->ch_tgpwr_CCK[chidx_20];
		priv->pshare->tgpwr_OFDM = priv->pshare->ch_tgpwr_OFDM[chidx_20];

		if (priv->pshare->CurrentChannelBW) {
			priv->pshare->txpwr_lmt_HT1S = priv->pshare->ch_pwr_lmtHT40_1S[chidx_40];
			priv->pshare->txpwr_lmt_HT2S = priv->pshare->ch_pwr_lmtHT40_2S[chidx_40];
			priv->pshare->tgpwr_HT1S = priv->pshare->ch_tgpwr_HT40_1S[chidx_40];
			priv->pshare->tgpwr_HT2S = priv->pshare->ch_tgpwr_HT40_2S[chidx_40];
		} else { //if 20M bw, tmp == tmp2 ??
			priv->pshare->txpwr_lmt_HT1S = priv->pshare->ch_pwr_lmtHT20_1S[chidx_20];
			priv->pshare->txpwr_lmt_HT2S = priv->pshare->ch_pwr_lmtHT20_2S[chidx_20];
			priv->pshare->tgpwr_HT1S = priv->pshare->ch_tgpwr_HT20_1S[chidx_20];
			priv->pshare->tgpwr_HT2S = priv->pshare->ch_tgpwr_HT20_2S[chidx_20];
		}
	} else {
		priv->pshare->txpwr_lmt_CCK = 0;
		priv->pshare->txpwr_lmt_OFDM = 0;
		priv->pshare->txpwr_lmt_HT1S = 0;
		priv->pshare->txpwr_lmt_HT2S = 0;

		if(channel <= 165)
		printk("Cannot map current working channel (%d) to find power limit!\n", channel);
	}

	//printk("txpwr_lmt_OFDM %d tgpwr_OFDM %d\n", priv->pshare->txpwr_lmt_OFDM, priv->pshare->tgpwr_OFDM);

}

#ifdef TXPWR_LMT_NEWFILE

#define	BAND2G_20M_1T_CCK 		1
#define	BAND2G_20M_1T_OFDM 		2
#define	BAND2G_20M_1T_HT 		3
#define	BAND2G_20M_2T_HT 		4
#define	BAND2G_20M_3T_HT 		5
#define	BAND2G_20M_4T_HT 		6
#define	BAND2G_40M_1T_HT		7
#define	BAND2G_40M_2T_HT		8
#define	BAND2G_40M_3T_HT		9
#define	BAND2G_40M_4T_HT		10

#define	BAND5G_20M_1T_OFDM 		11
#define	BAND5G_20M_1T_HT 		12
#define	BAND5G_20M_2T_HT 		13
#define	BAND5G_20M_3T_HT 		14
#define	BAND5G_20M_4T_HT 		15
#define	BAND5G_40M_1T_HT		16
#define	BAND5G_40M_2T_HT		17
#define	BAND5G_40M_3T_HT		18
#define	BAND5G_40M_4T_HT		19
#define	BAND5G_80M_1T_VHT		20
#define	BAND5G_80M_2T_VHT		21
#define	BAND5G_80M_3T_VHT		22
#define	BAND5G_80M_4T_VHT		23

int PHY_ConfigTXLmtWithParaFile_new(struct rtl8192cd_priv *priv)
{
    int read_bytes, found, num, len=0, round;
    unsigned int  channel, limit, table_idx = 0;
    unsigned char *mem_ptr, *line_head, *next_head;
    const unsigned char * table_index;
    const unsigned char table_index_old[24]={0,1,2,3,4,14,15,5,6,16,17,7,8,9,18,19,10,11,20,21,12,13,22,23};
    const unsigned char table_index_new[24]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
    struct TxPwrLmtTable_new *reg_table;

    unsigned int tmp =0;

    if(GET_CHIP_VER(priv) <= VERSION_8881A || GET_CHIP_VER(priv) == VERSION_8723B){/* for old ic format*/
        table_index = table_index_old;
    }
    else { /*new ic format*/
        table_index = table_index_new;
    }

    priv->pshare->txpwr_lmt_CCK = 0;
    priv->pshare->txpwr_lmt_OFDM = 0;
    priv->pshare->txpwr_lmt_HT1S = 0;
    priv->pshare->txpwr_lmt_HT2S = 0;
    priv->pshare->txpwr_lmt_VHT1S = 0;
    priv->pshare->txpwr_lmt_VHT2S = 0;
#ifdef CONFIG_WLAN_HAL_8814AE
	priv->pshare->txpwr_lmt_HT3S = 0;
	priv->pshare->txpwr_lmt_HT4S = 0;
	priv->pshare->txpwr_lmt_VHT3S = 0;
	priv->pshare->txpwr_lmt_VHT4S = 0;
#endif	

	reg_table = (struct TxPwrLmtTable *)priv->pshare->txpwr_lmt_buf;

	if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
		printk("PHY_ConfigMACWithParaFile(): not enough memory\n");
		return -1;
	}

	DEBUG_INFO("regdomain=%d\n",priv->pmib->dot11StationConfigEntry.dot11RegDomain);

	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory

#ifdef CONFIG_RTL_8812_SUPPORT
	if(GET_CHIP_VER(priv) == VERSION_8812E){
		panic_printk("[TXPWR_LMT_8812_new]\n");
	next_head = data_TXPWR_LMT_8812_new_start;
	read_bytes = (int)(data_TXPWR_LMT_8812_new_end - data_TXPWR_LMT_8812_new_start);
	}
	else 
#endif
#ifdef CONFIG_RTL_8723B_SUPPORT
	if(GET_CHIP_VER(priv) == VERSION_8723B){
		next_head = data_TXPWR_LMT_8723_start;
		read_bytes = (int)(data_TXPWR_LMT_8723_end - data_TXPWR_LMT_8723_start);
		panic_printk("[TXPWR_LMT_8723B_new] read_bytes = %d\n", read_bytes);
	}
	else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
	if(GET_CHIP_VER(priv) == VERSION_8188E){
		panic_printk("[TXPWR_LMT_88E_new]\n");
		next_head = data_TXPWR_LMT_88E_new_start;
		read_bytes = (int)(data_TXPWR_LMT_88E_new_end - data_TXPWR_LMT_88E_new_start);
	}
	else
#endif
#ifdef CONFIG_WLAN_HAL
	if ( IS_HAL_CHIP(priv) ) {
#ifdef PWR_BY_RATE_92E_HP
#ifdef HIGH_POWER_EXT_PA
		if (priv->pshare->rf_ft_var.use_ext_pa && GET_CHIP_VER(priv) == VERSION_8192E) {
			panic_printk("[%s][TXPWR_LMT_92EE_hp]\n", __FUNCTION__);				
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERLIMITFILE_HP_SIZE, (pu1Byte)&read_bytes);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERLIMITFILE_HP_START, (pu1Byte)&next_head);
		} else
#endif	
#endif
		{
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERLIMITFILE_SIZE, (pu1Byte)&read_bytes);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERLIMITFILE_START, (pu1Byte)&next_head);
		}			
	} else
#endif		
	{
		printk("This WiFi IC not support Tx Power limit !!\n");
		kfree(mem_ptr);
		return -1;
	}

	memcpy(mem_ptr, next_head, read_bytes);

	next_head = mem_ptr;
	round = 0;
	while (1) {
		line_head = next_head;
		next_head = get_line(&line_head);

		if (line_head == NULL)
			break;

		if (((line_head[0] == '/') || (line_head[0] == '#')) && (line_head[2] != 'T')) /* get string "//Table 1:" */  //OSK: change "/" to "#"
			continue;

		found = get_chnl_lmt_dot_new(priv, line_head, &channel, &limit, &table_idx);

		if (found > 0) {
			reg_table[len].channel = channel;
			reg_table[len].limit = limit;
			reg_table[len].table_idx = table_idx;

			len++;
			if ((len * sizeof(struct TxPwrLmtTable_new)) > MAC_REG_SIZE)
				break;
		}

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	kfree(mem_ptr);

	if ((len * sizeof(struct TxPwrLmtTable_new)) > MAC_REG_SIZE) {
		printk("TXPWR_LMT table buffer not large enough!\n");
		return -1;
	}
	
	num = 0;

	memset(priv->pshare->ch_pwr_lmtOFDM, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT20_1S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT20_2S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT40_1S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT40_2S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtVHT80_1S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtVHT80_2S, 0, SUPPORT_CH_NUM);

#ifdef CONFIG_WLAN_HAL_8814AE
	memset(priv->pshare->ch_pwr_lmtHT20_3S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT20_4S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT40_3S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT40_4S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtVHT80_3S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtVHT80_4S, 0, SUPPORT_CH_NUM);
#endif
	
	round = 0;
	while (1) {
		channel = reg_table[num].channel;
		limit = reg_table[num].limit;
		table_idx = reg_table[num].table_idx;

		if(channel > 0)
		{
			//printk(">> [%02d]-%03d-%02d\n", table_idx, channel , limit);

			
				int j = ch2idx(channel);

				if (table_idx == table_index[BAND2G_20M_1T_CCK]){
					priv->pshare->ch_pwr_lmtCCK[j] = limit;
				}else if (table_idx == table_index[BAND2G_20M_1T_OFDM]){
					priv->pshare->ch_pwr_lmtOFDM[j] = limit;
				}else if (table_idx == table_index[BAND2G_20M_1T_HT]){
					priv->pshare->ch_pwr_lmtHT20_1S[j] = limit;
				}else if (table_idx == table_index[BAND2G_20M_2T_HT]){
					priv->pshare->ch_pwr_lmtHT20_2S[j] = limit;
				}else if (table_idx == table_index[BAND2G_40M_1T_HT]){
					priv->pshare->ch_pwr_lmtHT40_1S[j] = limit;
				}else if (table_idx == table_index[BAND2G_40M_2T_HT]){
					priv->pshare->ch_pwr_lmtHT40_2S[j] = limit;
				}
#ifdef CONFIG_WLAN_HAL_8814AE				
				else if (table_idx == table_index[BAND2G_20M_3T_HT]){
					priv->pshare->ch_pwr_lmtHT20_3S[j] = limit;
				}else if (table_idx == table_index[BAND2G_20M_4T_HT]){
					priv->pshare->ch_pwr_lmtHT20_4S[j] = limit;
				}else if (table_idx == table_index[BAND2G_40M_3T_HT]){
					priv->pshare->ch_pwr_lmtHT40_3S[j] = limit;
				}else if (table_idx == table_index[BAND2G_40M_4T_HT]){
					priv->pshare->ch_pwr_lmtHT40_4S[j] = limit;
				}
#endif			
				else if (table_idx == table_index[BAND5G_20M_1T_OFDM]){
					priv->pshare->ch_pwr_lmtOFDM[j] = limit;
				}else if (table_idx == table_index[BAND5G_20M_1T_HT]){
					priv->pshare->ch_pwr_lmtHT20_1S[j] = limit;
				}else if (table_idx == table_index[BAND5G_20M_2T_HT]){
					priv->pshare->ch_pwr_lmtHT20_2S[j] = limit;
				}else if (table_idx == table_index[BAND5G_40M_1T_HT]){
					priv->pshare->ch_pwr_lmtHT40_1S[j] = limit;
				}else if (table_idx == table_index[BAND5G_40M_2T_HT]){
					priv->pshare->ch_pwr_lmtHT40_2S[j] = limit;
				}else if (table_idx == table_index[BAND5G_80M_1T_VHT]){
					priv->pshare->ch_pwr_lmtVHT80_1S[j] = limit;
				}else if (table_idx == table_index[BAND5G_80M_2T_VHT]){
					priv->pshare->ch_pwr_lmtVHT80_2S[j] = limit;
				}
#ifdef CONFIG_WLAN_HAL_8814AE
				else if (table_idx == table_index[BAND5G_20M_3T_HT]){
					priv->pshare->ch_pwr_lmtHT20_3S[j] = limit;
				}else if (table_idx == table_index[BAND5G_20M_4T_HT]){
					priv->pshare->ch_pwr_lmtHT20_4S[j] = limit;
				}else if (table_idx == table_index[BAND5G_40M_3T_HT]){
					priv->pshare->ch_pwr_lmtHT40_3S[j] = limit;
				}else if (table_idx == table_index[BAND5G_40M_4T_HT]){
					priv->pshare->ch_pwr_lmtHT40_4S[j] = limit;
				}else if (table_idx == table_index[BAND5G_80M_3T_VHT]){
					priv->pshare->ch_pwr_lmtVHT80_3S[j] = limit;
				}else if (table_idx == table_index[BAND5G_80M_4T_VHT]){
					priv->pshare->ch_pwr_lmtVHT80_4S[j] = limit;
				}
#endif				

		
		}

		num++;

		if(num >= len)
			break;
		
		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	return 0;
}
#ifdef BEAMFORMING_AUTO
void BEAMFORMING_TXPWRLMT_Auto(struct rtl8192cd_priv *priv)
{
	unsigned int maxDelta;
	if(priv->pmib->dot11RFEntry.txbf == 1) {
		if (!priv->pshare->rf_ft_var.disable_txpwrlmt) {
			priv->pshare->txbf2TXbackoff = 0;
			#ifdef CONFIG_WLAN_HAL_8814AE
			priv->pshare->txbf3TXbackoff = 0;
			#endif
			#ifdef RTK_AC_SUPPORT //for 11ac logo
		    if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) {
				// For VHT STA
				maxDelta = (priv->pshare->txpwr_lmt_VHT2S - priv->pshare->txpwr_lmt_TXBF_VHT2S);
				maxDelta = max(maxDelta,(priv->pshare->txpwr_lmt_VHT1S - priv->pshare->txpwr_lmt_TXBF_VHT1S));
				if(maxDelta > 4) 
				{				
					priv->pshare->txbferVHT2TX = 0;
					#ifdef CONFIG_WLAN_HAL_8814AE
					priv->pshare->txbferVHT3TX = 0;
					#endif
				} else 
				{	
					priv->pshare->txbferVHT2TX = 1;
					priv->pshare->txbf2TXbackoff = maxDelta; 
					#ifdef CONFIG_WLAN_HAL_8814AE
					if(get_rf_mimo_mode(priv) == MIMO_3T3R) {
						maxDelta = max(maxDelta,(priv->pshare->txpwr_lmt_VHT3S - priv->pshare->txpwr_lmt_TXBF_VHT3S));
						if(maxDelta > 4)
							priv->pshare->txbferVHT3TX = 0;
						else {
							priv->pshare->txbferVHT3TX= 1;
							priv->pshare->txbf3TXbackoff = maxDelta; 
						}
					}
					#endif				
				}
			}
			#endif
			// For HT STA
			
			maxDelta = (priv->pshare->txpwr_lmt_HT2S - priv->pshare->txpwr_lmt_TXBF_HT2S);
			maxDelta = max(maxDelta,(priv->pshare->txpwr_lmt_HT1S - priv->pshare->txpwr_lmt_TXBF_HT1S));
			if(maxDelta > 4) 
			{				
				priv->pshare->txbferHT2TX= 0;
				#ifdef CONFIG_WLAN_HAL_8814AE
				priv->pshare->txbferHT3TX= 0;
				#endif
			} else 
			{	
				priv->pshare->txbferHT2TX= 1;				
				priv->pshare->txbf2TXbackoff = max(priv->pshare->txbf2TXbackoff,maxDelta);						
				
				#ifdef CONFIG_WLAN_HAL_8814AE				
				if(get_rf_mimo_mode(priv) == MIMO_3T3R) {
					if((priv->pshare->txpwr_lmt_HT3S - priv->pshare->txpwr_lmt_TXBF_HT3S) > 4)
						priv->pshare->txbferHT3TX= 0;
					else {
						priv->pshare->txbferHT3TX= 1;
						priv->pshare->txbf3TXbackoff = max(maxDelta, priv->pshare->txbf3TXbackoff);
					}
				}
				#endif				
			}
			#ifdef CONFIG_WLAN_HAL_8814AE
			if (GET_CHIP_VER(priv)==VERSION_8814A) {
				if(priv->pshare->txbferHT2TX || priv->pshare->txbferVHT2TX) {				
					if(priv->pshare->txbf2TXbackoff == 4 || priv->pshare->txbf2TXbackoff == 3) // -2dB + 1.7dB
						PHY_SetBBReg(priv, 0x950, 0x0003FE00, 0x7B); // r_tx_bf_backoff_2tx Bit(17:9)
					else if(priv->pshare->txbf2TXbackoff == 2 || priv->pshare->txbf2TXbackoff == 1) // -1dB + 1.7dB
						PHY_SetBBReg(priv, 0x950, 0x0003FE00, 0x8A); // r_tx_bf_backoff_2tx Bit(17:9)
		
				}
				if(priv->pshare->txbf2TXbackoff == 0) // 0dB + 1.7dB
						PHY_SetBBReg(priv, 0x950, 0x0003FE00, 0x9B); // r_tx_bf_backoff_2tx Bit(17:9)

				if(get_rf_mimo_mode(priv) == MIMO_3T3R) {		
					if(priv->pshare->txbferHT3TX || priv->pshare->txbferVHT3TX) {				
						if(priv->pshare->txbf3TXbackoff == 4 || priv->pshare->txbf3TXbackoff == 3) // -2dB
							PHY_SetBBReg(priv, 0x950, 0x07FC0000, 0x65); // r_tx_bf_backoff_3tx Bit(17:9)
						else if(priv->pshare->txbf3TXbackoff == 2 || priv->pshare->txbf3TXbackoff == 1) // -1dB
							PHY_SetBBReg(priv, 0x950, 0x07FC0000, 0x72); // r_tx_bf_backoff_3tx Bit(17:9)
			
					}		
					if(priv->pshare->txbf3TXbackoff == 0) // 0dB
							PHY_SetBBReg(priv, 0x950, 0x07FC0000, 0x80); // r_tx_bf_backoff_3tx Bit(17:9)
				}
			}
			#endif		
		}else {			
			#ifdef CONFIG_WLAN_HAL_8814AE
			if (GET_CHIP_VER(priv)==VERSION_8814A) {
				PHY_SetBBReg(priv, 0x950, 0x0003FE00, 0x9B); // r_tx_bf_backoff_2tx Bit(17:9)
			}
			#endif
		}
	}
		
}

int PHY_ConfigTXLmtWithParaFile_new_TXBF(struct rtl8192cd_priv *priv)
{
	int read_bytes, found, num, len=0, round;
	unsigned int  channel, limit, table_idx = 0;
	unsigned char *mem_ptr, *line_head, *next_head;
	int	tbl_idx[13], set_en=0, type=-1;
	struct TxPwrLmtTable_new *reg_table;

	unsigned int tmp =0;
	

	priv->pshare->txpwr_lmt_TXBF_HT1S = 0;
	priv->pshare->txpwr_lmt_TXBF_HT2S = 0;
	priv->pshare->txpwr_lmt_TXBF_VHT1S = 0;
	priv->pshare->txpwr_lmt_TXBF_VHT2S = 0;
#ifdef CONFIG_WLAN_HAL_8814AE
	priv->pshare->txpwr_lmt_TXBF_HT3S = 0;
	priv->pshare->txpwr_lmt_TXBF_HT4S = 0;
	priv->pshare->txpwr_lmt_TXBF_VHT3S = 0;
	priv->pshare->txpwr_lmt_TXBF_VHT4S = 0;
#endif	

	reg_table = (struct TxPwrLmtTable *)priv->pshare->txpwr_lmt_buf;

	if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
		printk("PHY_ConfigMACWithParaFile(): not enough memory\n");
		return -1;
	}

	DEBUG_INFO("regdomain=%d\n",priv->pmib->dot11StationConfigEntry.dot11RegDomain);

	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory

#ifdef CONFIG_WLAN_HAL
	if ( IS_HAL_CHIP(priv) ) {
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERLIMITFILE_TXBF_SIZE, (pu1Byte)&read_bytes);
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERLIMITFILE_TXBF_START, (pu1Byte)&next_head);
	} else
#endif		
	{
		printk("This WiFi IC not support Tx Power limit (TXBF)!!\n");
		kfree(mem_ptr);
		return -1;
	}

	memcpy(mem_ptr, next_head, read_bytes);

	next_head = mem_ptr;
	round = 0;
	while (1) {
		line_head = next_head;
		next_head = get_line(&line_head);

		if (line_head == NULL)
			break;

		if ((line_head[0] == '/') && (line_head[2] != 'T')) /* get string "//Table 1:" */
			continue;

		found = get_chnl_lmt_dot_new(priv, line_head, &channel, &limit, &table_idx);

		if (found > 0) {
			reg_table[len].channel = channel;
			reg_table[len].limit = limit;
			reg_table[len].table_idx = table_idx;

			len++;
			if ((len * sizeof(struct TxPwrLmtTable_new)) > MAC_REG_SIZE)
				break;
		}

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	kfree(mem_ptr);

	if ((len * sizeof(struct TxPwrLmtTable_new)) > MAC_REG_SIZE) {
		printk("TXPWR_LMT (TXBF) table buffer not large enough!\n");
		return -1;
	}
	
	num = 0;

	memset(priv->pshare->ch_pwr_lmtHT20_TXBF_1S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT20_TXBF_2S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT40_TXBF_1S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT40_TXBF_2S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtVHT80_TXBF_1S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtVHT80_TXBF_2S, 0, SUPPORT_CH_NUM);

#ifdef CONFIG_WLAN_HAL_8814AE
	memset(priv->pshare->ch_pwr_lmtHT20_TXBF_3S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT20_TXBF_4S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT40_TXBF_3S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtHT40_TXBF_4S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtVHT80_TXBF_3S, 0, SUPPORT_CH_NUM);
	memset(priv->pshare->ch_pwr_lmtVHT80_TXBF_4S, 0, SUPPORT_CH_NUM);
#endif
	
	round = 0;
	while (1) {
		channel = reg_table[num].channel;
		limit = reg_table[num].limit;
		table_idx = reg_table[num].table_idx;

		if(channel > 0)
		{
			//printk(">> [%02d]-%03d-%02d\n", table_idx, channel , limit);

			if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
			if ((table_idx >= BAND2G_20M_1T_CCK) && (table_idx <= BAND2G_40M_4T_HT)){
				
				int j = ch2idx(channel);

				if (table_idx == BAND2G_20M_1T_HT){
					priv->pshare->ch_pwr_lmtHT20_TXBF_1S[j] = limit;
				}else if (table_idx == BAND2G_20M_2T_HT){
					priv->pshare->ch_pwr_lmtHT20_TXBF_2S[j] = limit;
				}else if (table_idx == BAND2G_40M_1T_HT){
					priv->pshare->ch_pwr_lmtHT40_TXBF_1S[j] = limit;
				}else if (table_idx == BAND2G_40M_2T_HT){
					priv->pshare->ch_pwr_lmtHT40_TXBF_2S[j] = limit;
				}
#ifdef CONFIG_WLAN_HAL_8814AE				
				else if (table_idx == BAND2G_20M_3T_HT){
					priv->pshare->ch_pwr_lmtHT20_TXBF_3S[j] = limit;
				}else if (table_idx == BAND2G_20M_4T_HT){
					priv->pshare->ch_pwr_lmtHT20_TXBF_4S[j] = limit;
				}else if (table_idx == BAND2G_40M_3T_HT){
					priv->pshare->ch_pwr_lmtHT40_TXBF_3S[j] = limit;
				}else if (table_idx == BAND2G_40M_4T_HT){
					priv->pshare->ch_pwr_lmtHT40_TXBF_4S[j] = limit;
				}
#endif
				
			}

			if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
			if ( (table_idx >= BAND5G_20M_1T_OFDM) && (table_idx <= BAND5G_80M_4T_VHT)){
				
				int j = ch2idx(channel);
				
				if (table_idx == BAND5G_20M_1T_HT){
					priv->pshare->ch_pwr_lmtHT20_TXBF_1S[j] = limit;
				}else if (table_idx == BAND5G_20M_2T_HT){
					priv->pshare->ch_pwr_lmtHT20_TXBF_2S[j] = limit;
				}else if (table_idx == BAND5G_40M_1T_HT){
					priv->pshare->ch_pwr_lmtHT40_TXBF_1S[j] = limit;
				}else if (table_idx == BAND5G_40M_2T_HT){
					priv->pshare->ch_pwr_lmtHT40_TXBF_2S[j] = limit;
				}else if (table_idx == BAND5G_80M_1T_VHT){
					priv->pshare->ch_pwr_lmtVHT80_TXBF_1S[j] = limit;
				}else if (table_idx == BAND5G_80M_2T_VHT){
					priv->pshare->ch_pwr_lmtVHT80_TXBF_2S[j] = limit;
				}
#ifdef CONFIG_WLAN_HAL_8814AE
				else if (table_idx == BAND5G_20M_3T_HT){
					priv->pshare->ch_pwr_lmtHT20_TXBF_3S[j] = limit;
				}else if (table_idx == BAND5G_20M_4T_HT){
					priv->pshare->ch_pwr_lmtHT20_TXBF_4S[j] = limit;
				}else if (table_idx == BAND5G_40M_3T_HT){
					priv->pshare->ch_pwr_lmtHT40_TXBF_3S[j] = limit;
				}else if (table_idx == BAND5G_40M_4T_HT){
					priv->pshare->ch_pwr_lmtHT40_TXBF_4S[j] = limit;
				}else if (table_idx == BAND5G_80M_3T_VHT){
					priv->pshare->ch_pwr_lmtVHT80_TXBF_3S[j] = limit;
				}else if (table_idx == BAND5G_80M_4T_VHT){
					priv->pshare->ch_pwr_lmtVHT80_TXBF_4S[j] = limit;
				}
#endif				
			}
		
		}

		num++;

		if(num >= len)
			break;
		
		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	return 0;
}
#endif
#endif

int PHY_ConfigTXLmtWithParaFile(struct rtl8192cd_priv *priv)
{
	int read_bytes, num, len = 0, round;
	unsigned int  ch_start, ch_end, limit, target = 0;
	unsigned char *mem_ptr, *line_head, *next_head;
	int	tbl_idx[6], set_en = 0, type = -1;
	struct TxPwrLmtTable *reg_table;

	priv->pshare->txpwr_lmt_CCK = 0;
	priv->pshare->txpwr_lmt_OFDM = 0;
	priv->pshare->txpwr_lmt_HT1S = 0;
	priv->pshare->txpwr_lmt_HT2S = 0;

	reg_table = (struct TxPwrLmtTable *)priv->pshare->txpwr_lmt_buf;

	if ((GET_CHIP_VER(priv) != VERSION_8192D) && (GET_CHIP_VER(priv) != VERSION_8192C) && (GET_CHIP_VER(priv) != VERSION_8188C)) {
		printk("[%s]NOT support! TXPWR_LMT is for RTL8192D & 92C/88C ONLY!\n", __FUNCTION__);
		return -1;
	}

	if ((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
		printk("PHY_ConfigTXLmtWithParaFile(): not enough memory\n");
		return -1;
	}


	tbl_idx[0] = 1;
	tbl_idx[1] = 2;
	tbl_idx[2] = 3;
	tbl_idx[3] = 4;
	tbl_idx[4] = 5;
	tbl_idx[5] = 6;


	DEBUG_INFO("regdomain=%d tbl_idx=%d,%d\n", priv->pmib->dot11StationConfigEntry.dot11RegDomain, tbl_idx[0], tbl_idx[1]);


	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory

	DEBUG_INFO("[%s][TXPWR_LMT]\n", __FUNCTION__);


#ifdef CONFIG_RTL_92C_SUPPORT
	if((GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C))
	{
		int txpwr_lmt_index;

		if(priv->pmib->dot11StationConfigEntry.txpwr_lmt_index <= 0) {
			// if mib txpwr_lmt_index is default value
			if(priv->pmib->dot11StationConfigEntry.dot11RegDomain==DOMAIN_FCC)
				txpwr_lmt_index = 1;
			else if(priv->pmib->dot11StationConfigEntry.dot11RegDomain==DOMAIN_ETSI)
				txpwr_lmt_index = 2;
			else
				txpwr_lmt_index = TXPWR_LMT_92c_MAX + 1;
		} 
		else {
			txpwr_lmt_index = priv->pmib->dot11StationConfigEntry.txpwr_lmt_index;
		}

		if(txpwr_lmt_index <= TXPWR_LMT_92c_MAX &&
		   data_TXPWR_LMT_92c_array[txpwr_lmt_index]) 
		{
		
			next_head = data_TXPWR_LMT_92c_array[txpwr_lmt_index];
			read_bytes = data_TXPWR_LMT_92c_array_end[txpwr_lmt_index] - data_TXPWR_LMT_92c_array[txpwr_lmt_index];
		} 
		else 
		{		   		
			// When compiling error is "undefine data_TXPWR_LMT_92c"
		    // Please check if the file,rtl8192cd/data/TXPWR_LMT_92c.txt, exits.
			next_head = data_TXPWR_LMT_92c;
			read_bytes = (int)sizeof(data_TXPWR_LMT_92c);
		}		
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if(GET_CHIP_VER(priv)==VERSION_8192D)
	{
		
		int txpwr_lmt_index;

		if(priv->pmib->dot11StationConfigEntry.txpwr_lmt_index <= 0) {
			// if mib txpwr_lmt_index is default value
			if(priv->pmib->dot11StationConfigEntry.dot11RegDomain==DOMAIN_FCC)
				txpwr_lmt_index = 1;
			else if(priv->pmib->dot11StationConfigEntry.dot11RegDomain==DOMAIN_ETSI)
				txpwr_lmt_index = 2;
			else
				txpwr_lmt_index = TXPWR_LMT_92d_MAX + 1;
		} 
		else {
			txpwr_lmt_index = priv->pmib->dot11StationConfigEntry.txpwr_lmt_index;
		}

	    if(txpwr_lmt_index <= TXPWR_LMT_92d_MAX &&
		   data_TXPWR_LMT_92d_array[txpwr_lmt_index]) 
		{
		
			next_head = data_TXPWR_LMT_92d_array[txpwr_lmt_index];
			read_bytes = data_TXPWR_LMT_92d_array_end[txpwr_lmt_index] -
						 data_TXPWR_LMT_92d_array[txpwr_lmt_index];
		} else 
		{		   		
			// Note: default TX Power Limit
			// When compiling error is "undefine data_TXPWR_LMT_92d"
		    // Please check if the file,rtl8192cd/data/TXPWR_LMT_92d.txt, exits.		    
			next_head = data_TXPWR_LMT_92d;
			read_bytes = (int)sizeof(data_TXPWR_LMT_92d);
		}	
	}
#endif

	memcpy(mem_ptr, next_head, read_bytes);

	next_head = mem_ptr;
	round = 0;
	while (1) {
		line_head = next_head;
		next_head = get_line(&line_head);

		if (line_head == NULL)
			break;

		if (line_head[0] == '/')
			continue;

		num = get_chnl_lmt_dot(line_head, &ch_start, &ch_end, &limit, &target);

		if (num > 0) {
			reg_table[len].start = ch_start;
			reg_table[len].end = ch_end;
			reg_table[len].limit = limit;
			reg_table[len].target = target;

			len++;
			if ((len * sizeof(struct TxPwrLmtTable)) > MAC_REG_SIZE)
				break;
		}

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	reg_table[len].start = 0xff;

	kfree(mem_ptr);

	if ((len * sizeof(struct TxPwrLmtTable)) > MAC_REG_SIZE) {
		printk("TXPWR_LMT table buffer not large enough!\n");
		return -1;
	}

	num = 0;
	round = 0;
	while (1) {
		ch_start = reg_table[num].start;
		ch_end = reg_table[num].end;
		limit = reg_table[num].limit;
		target = reg_table[num].target;

		//printk(">> %d-%d-%d-%d\n",ch_start,ch_end,limit,target);
		if (ch_start == 0xff)
			break;

		if (ch_start == 0 && ch_end == 0) {
			if (limit == tbl_idx[0]) {
				set_en = 1;
				type = 0;
				memset(priv->pshare->ch_pwr_lmtCCK, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_CCK, 0, SUPPORT_CH_NUM);
			} else if (limit == tbl_idx[1]) {
				set_en = 1;
				type = 1;
				memset(priv->pshare->ch_pwr_lmtOFDM, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_OFDM, 0, SUPPORT_CH_NUM);
			} else if (limit == tbl_idx[2]) {
				set_en = 1;
				type = 2;
				memset(priv->pshare->ch_pwr_lmtHT20_1S, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_HT20_1S, 0, SUPPORT_CH_NUM);
			} else if (limit == tbl_idx[3]) {
				set_en = 1;
				type = 3;
				memset(priv->pshare->ch_pwr_lmtHT20_2S, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_HT20_2S, 0, SUPPORT_CH_NUM);
			} else if (limit == tbl_idx[4]) {
				set_en = 1;
				type = 4;
				memset(priv->pshare->ch_pwr_lmtHT40_1S, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_HT40_1S, 0, SUPPORT_CH_NUM);
			} else if (limit == tbl_idx[5]) {
				set_en = 1;
				type = 5;
				memset(priv->pshare->ch_pwr_lmtHT40_2S, 0, SUPPORT_CH_NUM);
				memset(priv->pshare->ch_tgpwr_HT40_2S, 0, SUPPORT_CH_NUM);
			} else {
				set_en = 0;
			}
		}

		if (set_en && ch_start) {
			int j;
			for (j = ch2idx(ch_start); j <= ch2idx(ch_end); j++) {
				if (j < 0 || j >= SUPPORT_CH_NUM) {
					panic_printk("channel out of bound!!\n");
					break;
				}

				if (type == 0) {
					priv->pshare->ch_pwr_lmtCCK[j] = limit;
					priv->pshare->ch_tgpwr_CCK[j] = target;
				} else if (type == 1) {
					priv->pshare->ch_pwr_lmtOFDM[j] = limit;
					priv->pshare->ch_tgpwr_OFDM[j] = target;
				} else if (type == 2) {
					priv->pshare->ch_pwr_lmtHT20_1S[j] = limit;
					priv->pshare->ch_tgpwr_HT20_1S[j] = target;
				} else if (type == 3) {
					priv->pshare->ch_pwr_lmtHT20_2S[j] = limit;
					priv->pshare->ch_tgpwr_HT20_2S[j] = target;
				} else if (type == 4) {
					priv->pshare->ch_pwr_lmtHT40_1S[j] = limit;
					priv->pshare->ch_tgpwr_HT40_1S[j] = target;
				} else if (type == 5) {
					priv->pshare->ch_pwr_lmtHT40_2S[j] = limit;
					priv->pshare->ch_tgpwr_HT40_2S[j] = target;
				}
			}
		}

		num++;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	return 0;
}

#endif


#ifdef _TRACKING_TABLE_FILE

static char TXPWR_TRACKING_NAME[][32] = {
	"2GCCKA_P",
	"2GCCKA_N",
	"2GCCKB_P",
	"2GCCKB_N",
	"2GA_P",
	"2GA_N",
	"2GB_P",
	"2GB_N",
	"5GLA_P",
	"5GLA_N",
	"5GLB_P",
	"5GLB_N",
	"5GMA_P",
	"5GMA_N",
	"5GMB_P",
	"5GMB_N",
	"5GHA_P",
	"5GHA_N",
	"5GHB_P",
	"5GHB_N",
};

#if defined(CONFIG_WLAN_HAL_8814AE)
static char TXPWR_TRACKING_NAME_NEW[][32] = {
	"[2G][A][+][CCK]",
	"[2G][A][-][CCK]",
	"[2G][B][+][CCK]",
	"[2G][B][-][CCK]",
	"[2G][C][+][CCK]",
	"[2G][C][-][CCK]",
	"[2G][D][+][CCK]",
	"[2G][D][-][CCK]",
	"[2G][A][+][ALL]",
	"[2G][A][-][ALL]",
	"[2G][B][+][ALL]",
	"[2G][B][-][ALL]",
	"[2G][C][+][ALL]",
	"[2G][C][-][ALL]",
	"[2G][D][+][ALL]",
	"[2G][D][-][ALL]",
	"[5G][A][+][ALL][0]",
	"[5G][A][-][ALL][0]",
	"[5G][B][+][ALL][0]",
	"[5G][B][-][ALL][0]",
	"[5G][C][+][ALL][0]",
	"[5G][C][-][ALL][0]",
	"[5G][D][+][ALL][0]",
	"[5G][D][-][ALL][0]",
	"[5G][A][+][ALL][1]",
	"[5G][A][-][ALL][1]",
	"[5G][B][+][ALL][1]",
	"[5G][B][-][ALL][1]",
	"[5G][C][+][ALL][1]",
	"[5G][C][-][ALL][1]",
	"[5G][D][+][ALL][1]",
	"[5G][D][-][ALL][1]",	
	"[5G][A][+][ALL][2]",
	"[5G][A][-][ALL][2]",
	"[5G][B][+][ALL][2]",
	"[5G][B][-][ALL][2]",	
	"[5G][C][+][ALL][2]",
	"[5G][C][-][ALL][2]",
	"[5G][D][+][ALL][2]",
	"[5G][D][-][ALL][2]",
};
#else
static char TXPWR_TRACKING_NAME_NEW[][32] = {
	"[2G][A][+][CCK]",
	"[2G][A][-][CCK]",
	"[2G][B][+][CCK]",
	"[2G][B][-][CCK]",
	"[2G][A][+][ALL]",
	"[2G][A][-][ALL]",
	"[2G][B][+][ALL]",
	"[2G][B][-][ALL]",
	"[5G][A][+][ALL][0]",
	"[5G][A][-][ALL][0]",
	"[5G][B][+][ALL][0]",
	"[5G][B][-][ALL][0]",
	"[5G][A][+][ALL][1]",
	"[5G][A][-][ALL][1]",
	"[5G][B][+][ALL][1]",
	"[5G][B][-][ALL][1]",
	"[5G][A][+][ALL][2]",
	"[5G][A][-][ALL][2]",
	"[5G][B][+][ALL][2]",
	"[5G][B][-][ALL][2]",
};
#endif

//#ifndef TXPWR_LMT

/* 
     Convert ascii value(*s) to integer
 */
int _convert_2_pwr_tracking(char *s, int base)
{
	int k = 0;

	k = 0;
	if (base == 10) {
		while (*s >= '0' && *s <= '9') {
			k = 10 * k + (*s - '0');
			s++;
		}
	} else
		return 0;

	return k;
}


static unsigned char *get_digit_tracking(unsigned char **data)
{
	unsigned char *buf = *data;
	int i = 0, round;
	BOOLEAN others=FALSE;

	*data = &buf[i];

	round = 0;
	while (buf[i]) {
		if (buf[i] == 0xff || (buf[i] == '\n') || (buf[i] == '\r')) { //4 Enter -> end of a line
			return NULL;
		}

		//if ((buf[i] == ' ') || (buf[i] == '\t'))   //4 Next is supposed to be a number so break... 
		if(others && buf[i+1] >= '0' && buf[i+1] <= '9')  //4 get the next number by passing through "something other than numbers"
			break;

		i++;
		others=TRUE;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}
	
    /* buf[i+1] is a number */
	
	round = 0;
	while (buf[i]) {

		if ((buf[i] == '\n') || (buf[i] == '\r')) {
			return NULL;
		}

		if ((buf[i] >= '0') && (buf[i] <= '9')) {
			//printk("found buf[i] = %c \n", buf[i]);
			return &buf[i];
		}

		i++;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	return NULL;
}

//#endif


void input_tracking_value(struct rtl8192cd_priv *priv, int offset, int num, int value)
{

	//printk("[%d][%d]=%d\n",offset%8, num, value);
	switch (offset) {
		case CCKA_P:
		case CCKA_N:
		case CCKB_P:
		case CCKB_N:
#if defined(CONFIG_WLAN_HAL_8814AE)
		case CCKC_P:
		case CCKC_N:
		case CCKD_P:
		case CCKD_N:
			offset = (offset % 8);		
#else
			offset = (offset % 4);
#endif
			priv->pshare->txpwr_tracking_2G_CCK[offset][num] = value;
			//printk("txpwr_tracking_2G_CCK[%d][%d]=%d\n", offset, num,value);
			break;
		case A_P:
		case A_N:
		case B_P:
		case B_N:
#if defined(CONFIG_WLAN_HAL_8814AE)
		case C_P:
		case C_N:
		case D_P:
		case D_N:
			offset = (offset % 8);		
#else
			offset = (offset % 4);
#endif
			priv->pshare->txpwr_tracking_2G_OFDM[offset][num] = value;
			//printk("txpwr_tracking_2G_OFDM[%d][%d]=%d\n", offset, num,value);
			break;
		case LA_P:
		case LA_N:
		case LB_P:
		case LB_N:
#if defined(CONFIG_WLAN_HAL_8814AE)
		case LC_P:
		case LC_N:
		case LD_P:
		case LD_N:
			offset = (offset % 8);		
#else
			offset = (offset % 4);
#endif
			priv->pshare->txpwr_tracking_5GL[offset][num] = value;
			//printk("txpwr_tracking_5GL[%d][%d]=%d\n", offset, num,value);
			break;
		case MA_P:
		case MA_N:
		case MB_P:
		case MB_N:
#if defined(CONFIG_WLAN_HAL_8814AE)
		case MC_P:
		case MC_N:
		case MD_P:
		case MD_N:
			offset = (offset % 8);		
#else
			offset = (offset % 4);
#endif
			priv->pshare->txpwr_tracking_5GM[offset][num] = value;
			//printk("txpwr_tracking_5GM[%d][%d]=%d\n", offset, num,value);
			break;
		case HA_P:
		case HA_N:
		case HB_P:
		case HB_N:
#if defined(CONFIG_WLAN_HAL_8814AE)
		case HC_P:
		case HC_N:
		case HD_P:
		case HD_N:
			offset = (offset % 8);		
#else
			offset = (offset % 4);
#endif	
			priv->pshare->txpwr_tracking_5GH[offset][num] = value;
			//printk("txpwr_tracking_5GH[%d][%d]=%d\n", offset, num,value);
			break;
		default:
			break;

		}


}

int get_tx_tracking_index(struct rtl8192cd_priv *priv, int channel, int rf_path, int delta, int is_decrease, int is_CCK)
{
	int index = 0;

	if (delta == 0)
		return 0;

	if (delta > index_mapping_NUM_MAX-1)
		delta = index_mapping_NUM_MAX-1;

	//printk("\n_eric_tracking +++ channel = %d, i = %d, delta = %d, is_decrease = %d, is_CCK = %d\n", channel, i, delta, is_decrease, is_CCK);

	if (!priv->pshare->tracking_table_new)
	delta = delta - 1;

	if (channel > 14) {
		if (channel <= 99) {
			index = priv->pshare->txpwr_tracking_5GL[(rf_path * 2) + is_decrease][delta];
		} else if (channel <= 140) {

			index = priv->pshare->txpwr_tracking_5GM[(rf_path * 2) + is_decrease][delta];
		} else {
			index = priv->pshare->txpwr_tracking_5GH[(rf_path * 2) + is_decrease][delta];
		}
	} else {
		if (is_CCK) {
			index = priv->pshare->txpwr_tracking_2G_CCK[(rf_path * 2) + is_decrease][delta];
		} else {
			index = priv->pshare->txpwr_tracking_2G_OFDM[(rf_path * 2) + is_decrease][delta];
		}
	}

	//printk("_eric_tracking +++ offset = %d\n", index);

	return index;

}

static int get_tracking_table(struct rtl8192cd_priv *priv, unsigned char *line_head)
{
	unsigned char *next;
	int base, idx;
	int num = 0;
	int offset = 0;
	unsigned char *swim;
	extern int _atoi(char * s, int base);

	swim = line_head + 1;
	priv->pshare->tracking_table_new = 0;
	
	for (offset = 0; offset < sizeof(TXPWR_TRACKING_NAME)/sizeof(TXPWR_TRACKING_NAME[0]); offset++) { //4 legacy format
		if (!memcmp(line_head, TXPWR_TRACKING_NAME[offset], strlen(TXPWR_TRACKING_NAME[offset]))){
			//printk("Legacy****************offset:%d   For %s\n",offset,TXPWR_TRACKING_NAME[offset]);
			break;
		}
	}
	
	if (offset >= sizeof(TXPWR_TRACKING_NAME)/sizeof(TXPWR_TRACKING_NAME[0])) { //4 new format
		for (offset = 0; offset < sizeof(TXPWR_TRACKING_NAME_NEW)/sizeof(TXPWR_TRACKING_NAME_NEW[0]); offset++) {
			if (!memcmp(line_head, TXPWR_TRACKING_NAME_NEW[offset], strlen(TXPWR_TRACKING_NAME_NEW[offset]))){
				//printk("****************   For %s\n",TXPWR_TRACKING_NAME_NEW[offset]);
				swim = line_head + strlen(TXPWR_TRACKING_NAME_NEW[offset]);
				break;
			}
		}

		if (offset >= sizeof(TXPWR_TRACKING_NAME_NEW)/sizeof(TXPWR_TRACKING_NAME_NEW[0])){
			//printk("offset >= TXPWR_TRACKING_NAME_NUM %d\n",sizeof(TXPWR_TRACKING_NAME)/sizeof(TXPWR_TRACKING_NAME[0]));
			return offset;
		}

		priv->pshare->tracking_table_new = 1;
	}

	if (offset >= sizeof(TXPWR_TRACKING_NAME_NEW)/sizeof(TXPWR_TRACKING_NAME_NEW[0])){
		//printk("ERROR !  offset >= TXPWR_TRACKING_NAME_NUM %d\n",sizeof(TXPWR_TRACKING_NAME)/sizeof(TXPWR_TRACKING_NAME[0]));
		return offset;
	}

	/*
		2GCCKA_P	0 1 1 2 2 2 2 3 3 3 4 4 5 5 6 6 6 7 7 7 8 8 8 9 9 9 9 12
		offset represent label "2GCCKA_P"
	*/

	//printk("_Eric offset = %d \n", offset);
	//printk("_Eric line_head = %s \n", line_head);


	next = get_digit_tracking(&swim); //4 Get the start address of next number
    /*
	     2GCCKA_P	0 1 1 2 2 2 2 3 3 3 4 4 5 5 6 6 6 7 7 7 8 8 8 9 9 9 9 12
	                        next
	*/
	//printk("*next = %c\n",*next);
	while (1) {

		if (next == NULL)
			break;

		if (next) {

			base = 10;
			idx = 0;

			//printk("[%d][%d] = %d \n",offset, num, _convert_2_pwr_tracking((char *)&next[idx], base)); //4 convert to 10-base for next number
            /* Fill in index vaule from table in power track data structure */
			input_tracking_value(priv, offset, num, _convert_2_pwr_tracking((char *)&next[idx], base));

			num++;
		} else
			break;

		if (num >= index_mapping_NUM_MAX){
			break;
		}
		next = get_digit_tracking(&next);
		/*
	 	     2GCCKA_P	0 1 1 2 2 2 2 3 3 3 4 4 5 5 6 6 6 7 7 7 8 8 8 9 9 9 9 12
	                                  next
         	*/
		//if(next != NULL)
		//	printk("   *next  =  %c\n",*next);

	}


	return offset;

}


void check_tracking_table(struct rtl8192cd_priv *priv)
{

	int tmp = 0;
	int tmp2 = 0;

	for (tmp = 0; tmp < 2*TXPWR_TRACKING_PATH_NUM; tmp++) {
		for (tmp2 = 1; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
			//printk("tracking_2G_CCK[%d][%d]=%d\n",tmp,tmp2-1,priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2-1]);
			if (priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2] < priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2 - 1])
				priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2] = priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2 - 1];
		}
	}

	for (tmp = 0; tmp < 2*TXPWR_TRACKING_PATH_NUM; tmp++) {
		for (tmp2 = 1; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
			//printk("txpwr_tracking_2G_OFDM[%d][%d]=%d\n",tmp,tmp2-1,priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2-1]);
			if (priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2] < priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2 - 1])
				priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2] = priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2 - 1];
		}
	}

	for (tmp = 0; tmp < 2*TXPWR_TRACKING_PATH_NUM; tmp++) {
		for (tmp2 = 1; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
			//printk("txpwr_tracking_5GL[%d][%d]=%d\n",tmp,tmp2-1,priv->pshare->txpwr_tracking_5GL[tmp][tmp2-1]);			
			if (priv->pshare->txpwr_tracking_5GL[tmp][tmp2] < priv->pshare->txpwr_tracking_5GL[tmp][tmp2 - 1])
				priv->pshare->txpwr_tracking_5GL[tmp][tmp2] = priv->pshare->txpwr_tracking_5GL[tmp][tmp2 - 1];
		}
	}

	for (tmp = 0; tmp < 2*TXPWR_TRACKING_PATH_NUM; tmp++) {
		for (tmp2 = 1; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
			//printk("txpwr_tracking_5GM[%d][%d]=%d\n",tmp,tmp2-1,priv->pshare->txpwr_tracking_5GM[tmp][tmp2-1]);			
			if (priv->pshare->txpwr_tracking_5GM[tmp][tmp2] < priv->pshare->txpwr_tracking_5GM[tmp][tmp2 - 1])
				priv->pshare->txpwr_tracking_5GM[tmp][tmp2] = priv->pshare->txpwr_tracking_5GM[tmp][tmp2 - 1];
		}
	}

	for (tmp = 0; tmp < 2*TXPWR_TRACKING_PATH_NUM; tmp++) {
		for (tmp2 = 1; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
			//printk("txpwr_tracking_5GH[%d][%d]=%d\n",tmp,tmp2-1,priv->pshare->txpwr_tracking_5GH[tmp][tmp2-1]);			
			if (priv->pshare->txpwr_tracking_5GH[tmp][tmp2] < priv->pshare->txpwr_tracking_5GH[tmp][tmp2 - 1])
				priv->pshare->txpwr_tracking_5GH[tmp][tmp2] = priv->pshare->txpwr_tracking_5GH[tmp][tmp2 - 1];
		}
	}

}

int PHY_ConfigTXPwrTrackingWithParaFile(struct rtl8192cd_priv *priv)
{
	int read_bytes, num, round = 0;
	unsigned char *mem_ptr, *line_head, *next_head;
	printk("[%s %d]\n",__FUNCTION__, __LINE__);
	
	memset(priv->pshare->txpwr_tracking_2G_CCK, 0, (2*TXPWR_TRACKING_PATH_NUM * index_mapping_NUM_MAX)); 
	memset(priv->pshare->txpwr_tracking_2G_OFDM, 0, (2*TXPWR_TRACKING_PATH_NUM * index_mapping_NUM_MAX));
	memset(priv->pshare->txpwr_tracking_5GL, 0, (2*TXPWR_TRACKING_PATH_NUM * index_mapping_NUM_MAX));  
	memset(priv->pshare->txpwr_tracking_5GM, 0, (2*TXPWR_TRACKING_PATH_NUM * index_mapping_NUM_MAX));
	memset(priv->pshare->txpwr_tracking_5GH, 0, (2*TXPWR_TRACKING_PATH_NUM * index_mapping_NUM_MAX));

	if ((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
		printk("PHY_ConfigTXPwrTrackingWithParaFile(): not enough memory\n");
		return -1;
	}

	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory


	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) { //_Eric_?? any other IC types ??
#ifdef CONFIG_RTL_92C_SUPPORT
#ifdef HIGH_POWER_EXT_PA
		printk("[%s][REG_TXPWR_TRK_hp]\n", __FUNCTION__);
		next_head = data_REG_TXPWR_TRK_hp_start;
		read_bytes = (int)(data_REG_TXPWR_TRK_hp_end - data_REG_TXPWR_TRK_hp_start);
#else
		printk("[%s][REG_TXPWR_TRK]\n", __FUNCTION__);
		next_head = data_REG_TXPWR_TRK_start;
		read_bytes = (int)(data_REG_TXPWR_TRK_end - data_REG_TXPWR_TRK_start);
#endif
#endif
	}
#ifdef CONFIG_RTL_92D_SUPPORT
	else if (GET_CHIP_VER(priv) == VERSION_8192D) {
#ifdef HIGH_POWER_EXT_PA
		printk("[%s][REG_TXPWR_TRK_n_92d_hp]\n", __FUNCTION__);
		next_head = data_REG_TXPWR_TRK_n_92d_hp_start;
		read_bytes = (int)(data_REG_TXPWR_TRK_n_92d_hp_end - data_REG_TXPWR_TRK_n_92d_hp_start);
#else
		printk("[%s][REG_TXPWR_TRK_n_92d]\n", __FUNCTION__);
		next_head = data_REG_TXPWR_TRK_n_92d_start;
		read_bytes = (int)(data_REG_TXPWR_TRK_n_92d_end - data_REG_TXPWR_TRK_n_92d_start);
#endif
	}
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
	else if (GET_CHIP_VER(priv) == VERSION_8812E) {
#ifdef HIGH_POWER_EXT_PA
		if (priv->pshare->rf_ft_var.use_ext_pa) {
			panic_printk("[REG_TXPWR_TRK_8812_hp]\n");
			next_head = data_REG_TXPWR_TRK_8812_hp_start;
			read_bytes = (int)(data_REG_TXPWR_TRK_8812_hp_end - data_REG_TXPWR_TRK_8812_hp_start);
		} else
#endif
		{
			panic_printk("[REG_TXPWR_TRK_8812]\n");
			next_head = data_REG_TXPWR_TRK_8812_start;
			read_bytes = (int)(data_REG_TXPWR_TRK_8812_end - data_REG_TXPWR_TRK_8812_start);
		}
	}
#endif

#ifdef CONFIG_RTL_8723B_SUPPORT
	else if (GET_CHIP_VER(priv) == VERSION_8723B) {
			next_head = data_TxPowerTrack_AP_8723_start;
			read_bytes = (int)(data_TxPowerTrack_AP_8723_end - data_TxPowerTrack_AP_8723_start);
			panic_printk("[REG_TXPWR_TRK_8723B], read_bytes = %d\n", read_bytes);
	}
#endif

#ifdef CONFIG_WLAN_HAL
    else if (IS_HAL_CHIP(priv)){
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERTRACKINGFILE_SIZE, (pu1Byte)&read_bytes);
		printk("[%s %d] read_bytes=%d\n",__FUNCTION__, __LINE__, read_bytes);
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERTRACKINGFILE_START, (pu1Byte)&next_head);   
    }
#endif 

#if 0
#ifdef CONFIG_WLAN_HAL_8192EE
    else if (GET_CHIP_VER(priv) == VERSION_8192E) {
            //panic_printk("[REG_TXPWR_TRK_8192E]\n");
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERTRACKINGFILE_SIZE, (pu1Byte)&read_bytes);
       	    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERTRACKINGFILE_START, (pu1Byte)&next_head);            
    } 
#endif
#ifdef CONFIG_WLAN_HAL_8881A
	else if (GET_CHIP_VER(priv) == VERSION_8881A) {
		//panic_printk("[REG_TXPWR_TRK_8881A]\n");
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERTRACKINGFILE_SIZE, (pu1Byte)&read_bytes);
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERTRACKINGFILE_START, (pu1Byte)&next_head);			
	} 
#endif
#ifdef CONFIG_WLAN_HAL_8814AE
	else if (GET_CHIP_VER(priv) == VERSION_8814A) {
		//printk("[%s][REG_TXPWR_TRK_8814A]\n", __FUNCTION__);
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERTRACKINGFILE_SIZE, (pu1Byte)&read_bytes);
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_POWERTRACKINGFILE_START, (pu1Byte)&next_head);			
	} 
#endif
#endif 
	else {
		//printk("[%s][NOT SUPPORT]\n", __FUNCTION__);
		kfree(mem_ptr);
		return -1;
	}

	memcpy(mem_ptr, next_head, read_bytes);
	mem_ptr[read_bytes] = 0xff;  /* add 0xff for recognition for end of table*/
	next_head = mem_ptr;

	while (1) {
		line_head = next_head;
		next_head = get_line(&line_head); /* line_head: start of current line; next_head: start of next line*/

		if (line_head == NULL) /* end of table*/
			break;

		if (line_head[0] == '/'){
			if(find_str(line_head, "Parameter")){ /* Record PHY parameter version */
				char *ch = line_head+2;
				while (1) {
					if (*ch == '\n')
						break;
					else {
						printk("%c", *ch);
						ch++;
					}
				}
				printk("\n");
			} 
			continue;
		}

		num = get_tracking_table(priv, line_head); //4 for each line

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	check_tracking_table(priv);

#if 0

	{
		int tmp = 0;
		int tmp2 = 0;

		for (tmp = 0; tmp < 4; tmp++) {
			printk("txpwr_tracking_2G_CCK #%d = ", tmp);
			for (tmp2 = 0; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
				printk("%d ", priv->pshare->txpwr_tracking_2G_CCK[tmp][tmp2]);
			}
			printk("\n");
		}

		for (tmp = 0; tmp < 4; tmp++) {
			printk("txpwr_tracking_2G_OFDM #%d = ", tmp);
			for (tmp2 = 0; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
				printk("%d ", priv->pshare->txpwr_tracking_2G_OFDM[tmp][tmp2]);
			}
			printk("\n");
		}

		for (tmp = 0; tmp < 4; tmp++) {
			printk("txpwr_tracking_5GL #%d = ", tmp);
			for (tmp2 = 0; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
				printk("%d ", priv->pshare->txpwr_tracking_5GL[tmp][tmp2]);
			}
			printk("\n");
		}

		for (tmp = 0; tmp < 4; tmp++) {
			printk("txpwr_tracking_5GM #%d = ", tmp);
			for (tmp2 = 0; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
				printk("%d ", priv->pshare->txpwr_tracking_5GM[tmp][tmp2]);
			}
			printk("\n");
		}

		for (tmp = 0; tmp < 4; tmp++) {
			printk("txpwr_tracking_5GH #%d = ", tmp);
			for (tmp2 = 0; tmp2 < index_mapping_NUM_MAX; tmp2 ++) {
				printk("%d ", priv->pshare->txpwr_tracking_5GH[tmp][tmp2]);
			}
			printk("\n");
		}

	}

#endif

	kfree(mem_ptr);

	return 0;
}
#endif

#ifdef THERMAL_CONTROL

#define STATE_INIT 0
#define STATE_THERMAL_CONTROL 1
#define THER_DIFF 1    // the diff thermal idx. if current del_ther dosen't drop down by THER_DIFF, thermal control will go more aggressive
#define INITIAL_PA 80  // limit Tx throughput to origin's INITIAL_PA percent for the 1st-time of tx duty cycle mechanism
//#define SECONDARY_PA 92  // limit Tx to current TP's SECONDARY_PA after 1st limitation
#define SECONDARY_PA 94  // limit Tx to current TP's SECONDARY_PA after 1st limitation
#define LOW_TP_PA 96   //  limit Tx to current TP's SECONDARY_PA when TP under 10Mbps
#define LOWEST_LIMIT 500 // in Kbps, Tx duty cycle final lowest bound
//#define LOWER_BOUND_LIMIT 10*1024 // in Kbps, Tx duty first lower bound 
#define LOWER_BOUND_LIMIT 25*1024 // in Kbps, Tx duty first lower bound ,  25Mbps approximately 18Mbps

#define thermal_printk(fmt, args...) \
	if (priv->pshare->rf_ft_var.debug) \
		panic_printk("[%s][%s] "fmt, priv->dev->name, __FUNCTION__, ## args);
	
void fix_path_92e(struct rtl8192cd_priv *priv, unsigned char path)
{
	if(path == 0){ /* fix path A Tx */
		PHY_SetBBReg(priv, 0x90c , bMaskDWord, 0x81121111);
		PHY_SetBBReg(priv, 0xa04, 0xf0000000, 0x8);
		priv->pshare->rf_ft_var.chosen_path = 1;
	}else if(path == 1){ /* fix path B Tx */
		PHY_SetBBReg(priv, 0x90c , bMaskDWord, 0x82221222);
		PHY_SetBBReg(priv, 0xa04, 0xf0000000, 0x8);
		priv->pshare->rf_ft_var.chosen_path = 2;
	}	
}

void set_1t_92e(struct rtl8192cd_priv *priv)
{
	PHY_SetBBReg(priv, 0x90c , bMaskDWord, 0x81121111);
	PHY_SetBBReg(priv, 0xa04, 0xf0000000, 0x8);
	RTL_W8(0xA07, 0x81);	
	RTL_W32(0xC8C, 0xa0e40000);
	
	priv->pshare->rf_ft_var.chosen_path = 1;
	thermal_printk("92E 1T\n");
	//3 To-Do: set default 0x6d8, 0x800, 0xa11, 0xc8c, .etc
}

void set_2t_92e(struct rtl8192cd_priv *priv)
{
	RTL_W8(0x6D8, RTL_R8(0x6D8) | 0x3F);
	RTL_W8(0x800, RTL_R8(0x800) & ~BIT(1));
	RTL_W32(0x80C, RTL_R32(0x80C) & ~BIT(31));				
	RTL_W32(0x90C, 0x83321333);
	RTL_W8(0xA07, 0xC1);
	RTL_W8(0xA11, RTL_R8(0xA11) & ~BIT(5));		
	RTL_W32(0xC8C, 0xa0240000);
	thermal_printk("92E 2T\n");
}

void fix_path_8814(struct rtl8192cd_priv *priv, unsigned char path)
{
	if(path == 1){ /* fix path B Tx */
		PHY_SetBBReg(priv,rTX_PATH_SEL_1, bMaskH12Bits,0x2);	// Tx path B for 1SS  //0x93c
		PHY_SetBBReg(priv,rTXPATH_AC, bMask4to7Bits,0x2);		// Tx path B for CCK //0x80c
		PHY_SetBBReg(priv,rCCK_RX_AC, bMaskH4Bits	,0x4);		// Tx path B for CCK //0xa04
		priv->pshare->rf_ft_var.chosen_path = 2;
		thermal_printk("ANTDIV fix path B\n");
	}else if(path == 2){ /* fix path C Tx */
		PHY_SetBBReg(priv,rTX_PATH_SEL_1, bMaskH12Bits,0x4);	// Tx path B for 1SS
		PHY_SetBBReg(priv,rTXPATH_AC, bMask4to7Bits,0x4);		// Tx path C for CCK
		PHY_SetBBReg(priv,rCCK_RX_AC, bMaskH4Bits	,0x2);		// Tx path C for CCK
		priv->pshare->rf_ft_var.chosen_path = 3;
		thermal_printk("ANTDIV fix path C\n");
	}	
}

void set_1t_8814(struct rtl8192cd_priv *priv)
{
	fix_path_8814(priv,1);
	thermal_printk("8814 1T\n");
}

void set_2t_8814(struct rtl8192cd_priv *priv)
{
	PHY_SetBBReg(priv,rTX_PATH_SEL_1, bMaskH12Bits,0x106);	// Tx path B/C for 1SS
	PHY_SetBBReg(priv,rTXPATH_AC, bMask4to7Bits,0x6);		// Tx path B/C for CCK
	PHY_SetBBReg(priv,rCCK_RX_AC, bMaskH4Bits	,0x6);		// Tx path B/C for CCK
	thermal_printk("8814 2T\n");
}

#ifdef CONFIG_WLAN_HAL_8197F
//1 Not Sure Yet
void set_1t_97f(struct rtl8192cd_priv *priv)
{
	config_phydm_trx_mode_8197f(ODMPTR, (ODM_RF_A), (ODM_RF_A|ODM_RF_B), 0);
	priv->pshare->rf_ft_var.chosen_path = 1;
	thermal_printk("97f 1T\n");
}

void set_2t_97f(struct rtl8192cd_priv *priv)
{
	config_phydm_trx_mode_8197f(ODMPTR, (ODM_RF_A|ODM_RF_B), (ODM_RF_A|ODM_RF_B), 1);
	thermal_printk("97f 2T\n");
}

//1 Not Sure Yet
void fix_path_97f(struct rtl8192cd_priv *priv, unsigned char path)
{
	if(path == 0){ /* fix path A Tx */
		config_phydm_trx_mode_8197f(ODMPTR, ODM_RF_A, (ODM_RF_A|ODM_RF_B), 0);
		priv->pshare->rf_ft_var.chosen_path = 1;
	}else if(path == 1){ /* fix path B Tx */
		config_phydm_trx_mode_8197f(ODMPTR, ODM_RF_B, (ODM_RF_A|ODM_RF_B), 0);
		priv->pshare->rf_ft_var.chosen_path = 2;
	}	
}

#endif			
#ifdef CONFIG_WLAN_HAL_8822BE
//1 Not Sure Yet
void set_1t_8822(struct rtl8192cd_priv *priv)
{
	config_phydm_trx_mode_8822b(ODMPTR, (ODM_RF_A), (ODM_RF_A|ODM_RF_B), 0);
	priv->pshare->rf_ft_var.chosen_path = 1;
	thermal_printk("8812b 1T\n");

}
void set_2t_8822(struct rtl8192cd_priv *priv)
{
	config_phydm_trx_mode_8822b(ODMPTR, (ODM_RF_A|ODM_RF_B), (ODM_RF_A|ODM_RF_B), 1);
	thermal_printk("8812b 2T\n");
}

//1 Not Sure Yet
void fix_path_8822(struct rtl8192cd_priv *priv, unsigned char path)
{
	if(path == 0){ /* fix path A Tx */
		config_phydm_trx_mode_8822b(ODMPTR, ODM_RF_A, (ODM_RF_A|ODM_RF_B), 0);
		priv->pshare->rf_ft_var.chosen_path = 1;
	}else if(path == 1){ /* fix path B Tx */
		config_phydm_trx_mode_8822b(ODMPTR, ODM_RF_B, (ODM_RF_A|ODM_RF_B), 0);
		priv->pshare->rf_ft_var.chosen_path = 2;
	}	
}

#endif	


void set_1t(struct rtl8192cd_priv *priv)
{
	priv->pshare->rf_ft_var.current_path = 1;
	if(GET_CHIP_VER(priv) == VERSION_8192E)
		set_1t_92e(priv);
	else if(GET_CHIP_VER(priv) == VERSION_8814A)
		set_1t_8814(priv);
#ifdef CONFIG_WLAN_HAL_8197F
	else if (GET_CHIP_VER(priv) == VERSION_8197F)
		;//set_1t_97f(priv);
#endif			
#ifdef CONFIG_WLAN_HAL_8822BE
	else if (GET_CHIP_VER(priv) == VERSION_8822B)
		set_1t_8822(priv);
#endif	

}

void set_2t(struct rtl8192cd_priv *priv)
{

	priv->pshare->rf_ft_var.current_path = 2;
	if(GET_CHIP_VER(priv) == VERSION_8192E){	
		
		set_2t_92e(priv);
	}else if(GET_CHIP_VER(priv) == VERSION_8814A)
		set_2t_8814(priv);
#ifdef CONFIG_WLAN_HAL_8197F
	else if (GET_CHIP_VER(priv) == VERSION_8197F)
		;//set_2t_97f(priv);
#endif			
#ifdef CONFIG_WLAN_HAL_8822BE
	else if (GET_CHIP_VER(priv) == VERSION_8822B)
		set_2t_8822(priv);
#endif	
}


/* decides 2T to 1T for 1SS */
void set_path(struct rtl8192cd_priv *priv)
{
	if(priv->pshare->rf_ft_var.path == 0){ /* as default by original tx2path*/
		if(priv->pshare->rf_ft_var.current_path == 1 &&
			priv->pmib->dot11RFEntry.tx2path == 1){
			set_2t(priv);
		}else if(priv->pshare->rf_ft_var.current_path == 2 &&
			priv->pmib->dot11RFEntry.tx2path == 0){
			set_1t(priv);
		}
		
	}else if(priv->pshare->rf_ft_var.path == 1 && 
		priv->pshare->rf_ft_var.current_path == 2){ /* set to 1T */
		/* set to 1T*/
		set_1t(priv);
	}
	else if(priv->pshare->rf_ft_var.path == 2 && 
			priv->pshare->rf_ft_var.current_path == 1){ /* set to 2T */
		/* set to 2T*/
		set_2t(priv);
	}
}

void fix_path(struct rtl8192cd_priv *priv, unsigned char path)
{

	if(GET_CHIP_VER(priv) == VERSION_8192E){
		fix_path_92e(priv, path);
	}else if(GET_CHIP_VER(priv) == VERSION_8814A){
		fix_path_8814(priv, path);
	}
}

struct stat_info* get_sta(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat, *pstat_max=NULL;
	struct list_head	*phead, *plist;
	unsigned int		 max_tp;
	phead = &priv->asoc_list;
	plist = phead;

	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if (pstat){
			if((pstat->current_tx_rate <= _MCS7_RATE_ || (pstat->current_tx_rate >= _NSS1_MCS0_RATE_ && pstat->current_tx_rate <= _NSS1_MCS9_RATE_))
				 &&	pstat->tx_avarage > max_tp) // find STA using 1SS and with highest Tx throughput
				pstat_max = pstat;
		}
	}
	return pstat_max;
}

void ant_div_2t(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	
	pstat = get_sta(priv);
	if (pstat) {
		if(pstat->rf_info.mimorssi[0] >= pstat->rf_info.mimorssi[1] && /* compare rssi */
			priv->pshare->rf_ft_var.chosen_path != 1){  /* currently not using path A*/
			fix_path(priv,0); /* use path A */
		}else if(pstat->rf_info.mimorssi[0] < pstat->rf_info.mimorssi[1] && /* compare rssi */
				priv->pshare->rf_ft_var.chosen_path != 2){	/* currently not using path B*/
			fix_path(priv,1); /* use path B */
		}
	}
}

void ant_div_8814(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;

	pstat = get_sta(priv);

	if(pstat != NULL){		
		if(pstat->rf_info.mimorssi[1] > pstat->rf_info.mimorssi[2] && /* compare rssi */
			  priv->pshare->rf_ft_var.chosen_path != 2){  /* currently not using path B */		  
			fix_path(priv,1); /* use path B */
		}else if(pstat->rf_info.mimorssi[1] < pstat->rf_info.mimorssi[2] && /* compare rssi */
				priv->pshare->rf_ft_var.chosen_path != 3){	/* currently not using path C */
			fix_path(priv,2); /* use path C */
		}
	}
}


/* 
*  decides path A or path B; B or C for 8814
*  path selection by higher rssi 
*/
void ant_div(struct rtl8192cd_priv *priv)
{
	if(GET_CHIP_VER(priv) == VERSION_8192E
#ifdef CONFIG_WLAN_HAL_8197F
	|| GET_CHIP_VER(priv) == VERSION_8197F
#endif
#ifdef CONFIG_WLAN_HAL_8822BE
	|| GET_CHIP_VER(priv) == VERSION_8822B
#endif	
		){
		ant_div_2t(priv);
	}else if(GET_CHIP_VER(priv) == VERSION_8814A){
		ant_div_8814(priv);
	}
}

unsigned char degrade_power(struct rtl8192cd_priv *priv)
{

	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	unsigned char set_power=0;
		
	phead = &priv->asoc_list;
	plist = phead;

	/* degrad power per STA */
//	if((plist = asoc_list_get_next(priv, plist)) != phead)
	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);

		if (pstat){
			if(pstat->power == 3){/* currently in -11dB*/
				if(pstat->rssi < RSSI2-TOLERANCE){
					pstat->power = 2; /* -7 dB*/			
					set_power = 1;
				}	
			}else if(pstat->power == 2){/* currently in -7dB*/
				if(pstat->rssi >= RSSI2+TOLERANCE){
					pstat->power = 3; /* -11 dB*/
					set_power = 1;
				}else if(pstat->rssi < RSSI1-TOLERANCE){
					pstat->power = 1; /* -3 dB*/	
					set_power = 1;
				}
			}else if(pstat->power == 1){/* currently in -3dB*/
				if(pstat->rssi >= RSSI1+TOLERANCE){
					pstat->power = 2; /* -7 dB*/
					set_power = 1;
				}else if((pstat->current_tx_rate <= _MCS7_RATE_) ||
							(pstat->current_tx_rate >= _NSS1_MCS0_RATE_ && pstat->current_tx_rate <= _NSS1_MCS9_RATE_))
				{  /*when using 1SS rate, don't degrade power*/
					pstat->power = 0; /* 0 dB*/	
					set_power = 1;
					thermal_printk("using 1SS Tx rate and cancel power degradation\n");
				}
			}else{ /* currently in 0dB*/
				if(pstat->rssi >= RSSI2){
					pstat->power = 3; /* -11 dB*/
					set_power = 1;
				}else if(pstat->rssi >= RSSI1){
					pstat->power = 2; /* -7 dB*/
					set_power = 1;
				}else if((pstat->current_tx_rate >= _NSS2_MCS0_RATE_) ||
							(pstat->current_tx_rate >= _MCS8_RATE_ && pstat->current_tx_rate <= _MCS15_RATE_))
				{
					pstat->power = 1; /* -3 dB*/	
					set_power = 1;
				}
			}
		}
		if(set_power)
			thermal_printk(" set powr power_desc=%u\n", pstat->power);
	}

	return set_power;
}

unsigned char cancel_degrade_power(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	unsigned char set_power=0;
		
	phead = &priv->asoc_list;
	plist = phead;

	/* degrad power per STA */
	//if((plist = asoc_list_get_next(priv, plist)) != phead)
	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if (pstat){
			if(pstat->power != 0){
				pstat->power = 0; /* 0 dB*/
				set_power = 1;
			}
		}
	}
	if(set_power)
		thermal_printk("[sta%u] cancel power degradation\n", pstat->aid);
	return set_power;
}


void update_del_ther(struct rtl8192cd_priv *priv)
{
	int ther;
	
	// enable power and trigger
	if(priv->up_time % 2 == 1){
		if (GET_CHIP_VER(priv)==VERSION_8814A)
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x42, BIT(17), 0x1);
		else if (GET_CHIP_VER(priv)==VERSION_8192E)
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x42, (BIT(17) | BIT(16)), 0x03);	
#ifdef CONFIG_WLAN_HAL_8197F
		else if (GET_CHIP_VER(priv) == VERSION_8197F)
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x42, (BIT(17) | BIT(16)), 0x03);
#endif			
#ifdef CONFIG_WLAN_HAL_8822BE
		else if (GET_CHIP_VER(priv) == VERSION_8822B)
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x42, BIT(17), 0x1);
#endif			
	}else{
	// delay for 1 mili-second
	//delay_ms(1);

	// query rf reg 0x24[4:0], for thermal meter value
		if (GET_CHIP_VER(priv)==VERSION_8814A)
			ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x42, 0xfc00, 1);
		else if (GET_CHIP_VER(priv)==VERSION_8192E)
			ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x42, 0xfc00, 1);
#ifdef CONFIG_WLAN_HAL_8197F
		else if (GET_CHIP_VER(priv) == VERSION_8197F)
			ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x42, 0xfc00, 1);
#endif			
#ifdef CONFIG_WLAN_HAL_8822BE
		else if (GET_CHIP_VER(priv) == VERSION_8822B)
			ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x42, 0xfc00, 1);
#endif	

		if(priv->pmib->dot11RFEntry.ther > 7)
			priv->pshare->rf_ft_var.del_ther = ther - (int)priv->pmib->dot11RFEntry.ther;
		else
			priv->pshare->rf_ft_var.del_ther = 0;
		thermal_printk("[%d]Delta Ther:%d, current ther: %d, cal ther: %u, hit[hi-%d low-%d]\n", priv->pshare->rf_ft_var.countdown,priv->pshare->rf_ft_var.del_ther, ther, priv->pmib->dot11RFEntry.ther, priv->pshare->rf_ft_var.hitcount_hi, priv->pshare->rf_ft_var.hitcount_low);
	}
}


void limit_tp(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	
	phead = &priv->asoc_list;
	plist = phead;

	/* calculating average Tx throughput per STA */
	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if (pstat){
			pstat->sta_bwcthrd_tx = priv->pshare->rf_ft_var.limit_tp; //tx_tp_base: kbps
			thermal_printk("[sta%u] limit tp %u Kbps, %u Mbps\n", pstat->aid, pstat->sta_bwcthrd_tx, pstat->sta_bwcthrd_tx/1024);
		}
	}
}


/* Calculating average Tx throughput for every station*/
void decide_limit_tp(struct rtl8192cd_priv *priv, unsigned char get_base)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	
	phead = &priv->asoc_list;
	plist = phead;

	/* calculating average Tx throughput per STA */
	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if (pstat){
			if(get_base == 1){			
				pstat->tx_tp_base = pstat->tx_avarage * 8 / 1024; // tx_average: Bytes per second; tx_tp_base: kbps
				pstat->got_limit_tp = 1;
				thermal_printk("[sta%u] got limit tp base %u Kbps, %u Mbps\n", pstat->aid, pstat->tx_tp_base, pstat->tx_tp_base/1024);
			}
			if(pstat->got_limit_tp == 1){
				pstat->tx_tp_limit = pstat->tx_tp_base * priv->pshare->rf_ft_var.pa / 100;
				if(pstat->tx_tp_limit < LOWER_BOUND_LIMIT){
					pstat->tx_tp_limit = LOWER_BOUND_LIMIT; // limitation lower bound = 10Mbps. Unit for tx_tp_limit is Kbps
				}
				pstat->sta_bwcthrd_tx = pstat->tx_tp_limit;
				thermal_printk("[sta%u] limit tp %u Kbps, %u Mbps, (%u %%)\n", pstat->aid, pstat->tx_tp_limit, pstat->tx_tp_limit/1024, priv->pshare->rf_ft_var.pa);
			}
		}
	}
}

/* Calculating average Tx throughput for every station*/
void decide_limit_tp_new(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	
	phead = &priv->asoc_list;
	plist = phead;

	if(priv->pshare->rf_ft_var.limit_90pa == 1){ /* limit to 90%*/
		priv->pshare->rf_ft_var.pa = LOW_TP_PA;
		thermal_printk("Meet 90%% limit conditions\n");
		while ((plist = asoc_list_get_next(priv, plist)) != phead){
			pstat = list_entry(plist, struct stat_info, asoc_list);
			if (pstat){
				if(pstat->tx_avarage * 8 / 1024 < pstat->tx_tp_limit)  // update tx_tp_base only if current TP is lower than previous limit throughput
					pstat->tx_tp_base = pstat->tx_avarage * 8 / 1024; // tx_average: Bytes per second; tx_tp_base: kbps
				else{
					pstat->tx_tp_base = pstat->tx_tp_limit;
					thermal_printk("[sta%u] Current Throughput %u Kbps, %u Mbps higher than previous limit throughput %u Kbps, %u Mbps \n", pstat->aid, pstat->tx_avarage*8/1024, pstat->tx_avarage*8/1024/1024, pstat->tx_tp_limit, pstat->tx_tp_limit/1024);
				}
				pstat->got_limit_tp = 1;
				thermal_printk("[sta%u] got limit tp base %u Kbps, %u Mbps\n", pstat->aid, pstat->tx_tp_base, pstat->tx_tp_base/1024);

				pstat->tx_tp_limit = pstat->tx_tp_base * priv->pshare->rf_ft_var.pa / 100;
				if(pstat->tx_tp_limit <= LOWEST_LIMIT)
					pstat->tx_tp_limit = LOWEST_LIMIT;
				pstat->sta_bwcthrd_tx = pstat->tx_tp_limit;
				thermal_printk("[sta%u] limit tp %u Kbps, %u Mbps, (%u %%)\n", pstat->aid, pstat->tx_tp_limit, pstat->tx_tp_limit/1024, priv->pshare->rf_ft_var.pa);		
			}				
		}
	}else{
		if(priv->pshare->rf_ft_var.txduty_level == 1) /* limit to 55%*/
			priv->pshare->rf_ft_var.pa = INITIAL_PA;
		else
			priv->pshare->rf_ft_var.pa = SECONDARY_PA; /* limit to 80%*/

		//1 tmp off; only one stage
		//priv->pshare->rf_ft_var.limit_90pa = 1;
		
		while ((plist = asoc_list_get_next(priv, plist)) != phead){
			pstat = list_entry(plist, struct stat_info, asoc_list);
			if (pstat){
				if(pstat->current_tx_rate <= _54M_RATE_){
					thermal_printk("[sta%u] don't limit TP since using OFDM or CCK\n", pstat->aid); /*don't limit for CCK or OFDM*/
				}else{
					if(pstat->tx_avarage * 8 / 1024 < pstat->tx_tp_limit || pstat->tx_tp_limit == 0)  // update tx_tp_base only if current TP is lower than previous limit throughput
						pstat->tx_tp_base = pstat->tx_avarage * 8 / 1024; // tx_average: Bytes per second; tx_tp_base: kbps
					else{
						pstat->tx_tp_base = pstat->tx_tp_limit;
						thermal_printk("[sta%u] Current Throughput %u Kbps, %u Mbps higher than previous limit throughput %u Kbps, %u Mbps \n", pstat->aid, pstat->tx_avarage*8/1024, pstat->tx_avarage*8/1024/1024, pstat->tx_tp_limit, pstat->tx_tp_limit/1024);
					}
					pstat->got_limit_tp = 1;
					thermal_printk("[sta%u] Current Throughput %u Kbps, %u Mbps\n", pstat->aid, pstat->tx_tp_base, pstat->tx_tp_base/1024);

					pstat->tx_tp_limit = pstat->tx_tp_base * priv->pshare->rf_ft_var.pa / 100;
					if(pstat->tx_tp_limit < LOWER_BOUND_LIMIT){
						pstat->tx_tp_limit = LOWER_BOUND_LIMIT; // limitation lower bound = 10Mbps. Unit for tx_tp_limit is Kbps
						thermal_printk("[sta%u] Limit Throughput under 10Mbps\n", pstat->aid);
					}else{
						priv->pshare->rf_ft_var.limit_90pa = 0;
					}
					pstat->sta_bwcthrd_tx = pstat->tx_tp_limit;
					thermal_printk("[sta%u] Limit Throughput %u Kbps, %u Mbps, (%u %%)\n", pstat->aid, pstat->tx_tp_limit, pstat->tx_tp_limit/1024, priv->pshare->rf_ft_var.pa);		
				}
			}
		}	
	}
}


/* disable throughput limit */
void cancel_limit_tp(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	
	phead = &priv->asoc_list;
	plist = phead;

	/* calculating average Tx throughput per STA */
	//if((plist = asoc_list_get_next(priv, plist)) != phead)
	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if (pstat){
			if(pstat->got_limit_tp){
				pstat->tx_tp_base = 0;
				pstat->tx_tp_limit = 0;
				pstat->sta_bwcthrd_tx = 0;
				pstat->got_limit_tp = 0;
#ifdef RTK_STA_BWC					
				pstat->sta_bwcdrop_cnt = 0;
#endif
				thermal_printk("[sta%u] Cancel Tx throughput limit\n", pstat->aid);
			}
		}
	}
	priv->pshare->rf_ft_var.limit_90pa = 0;
}


void enter_init_state(struct rtl8192cd_priv *priv)
{
	thermal_printk("Enter Initial State\n");
	priv->pshare->rf_ft_var.state = STATE_INIT;

	thermal_printk("Disable Tx power degradation. low_power = 0\n");
	priv->pshare->rf_ft_var.low_power = 0;

	thermal_printk("Disable 1T. path = 2, path_select = 0\n");
	priv->pshare->rf_ft_var.path = 2;
	priv->pshare->rf_ft_var.path_select = 0;

	thermal_printk("Disable Tx Duty Cycle\n");
	priv->pshare->rf_ft_var.txduty = 0;
	cancel_limit_tp(priv);	
}

void enter_thermal_control_state(struct rtl8192cd_priv *priv)
{
	thermal_printk("Enter Thermal Control State\n");
	priv->pshare->rf_ft_var.state = STATE_THERMAL_CONTROL;

	priv->pshare->rf_ft_var.txduty_level = 1; 
	priv->pshare->rf_ft_var.txduty = 1;
	decide_limit_tp_new(priv);
	thermal_printk("Enable Tx Duty Cycle with Level %d => limit to %d%%  \n", priv->pshare->rf_ft_var.txduty_level, priv->pshare->rf_ft_var.pa);		
	
	thermal_printk("Enable Tx power degradation. low_power = 1\n");
	priv->pshare->rf_ft_var.low_power = 1;

	thermal_printk("Enable 1T. path = 1, path_select = 1\n");
	priv->pshare->rf_ft_var.path = 1;
	priv->pshare->rf_ft_var.path_select = 1;
	
	priv->pshare->rf_ft_var.countdown = priv->pshare->rf_ft_var.monitor_time; // wait monitor_tim then to check
	priv->pshare->rf_ft_var.ther_drop = priv->pshare->rf_ft_var.del_ther - THER_DIFF; //expected ther to drop to during monitor_tim
	thermal_printk("ther_drop = %u\n", priv->pshare->rf_ft_var.ther_drop);
}

void state_init_dm(struct rtl8192cd_priv *priv)
{
	//if(priv->pshare->rf_ft_var.del_ther >= priv->pshare->rf_ft_var.ther_hi)
	if (priv->pshare->rf_ft_var.hitcount_hi >= THER_HIT_COUNT)
	{
		priv->pshare->rf_ft_var.hitcount_hi = 0;
		enter_thermal_control_state(priv);
	}
}

void state_thermal_control_dm(struct rtl8192cd_priv *priv)
{
	if(priv->pshare->rf_ft_var.countdown)
	priv->pshare->rf_ft_var.countdown--;
	/* thermal reduced to expected ther_low, release thermal control */
	//if(priv->pshare->rf_ft_var.del_ther <= priv->pshare->rf_ft_var.ther_low)
	if (priv->pshare->rf_ft_var.hitcount_low >= THER_HIT_COUNT)
	{
		enter_init_state(priv);
		priv->pshare->rf_ft_var.hitcount_low = 0;
	}else if(priv->pshare->rf_ft_var.countdown == 0){/*already monitored for monitor_time */
		priv->pshare->rf_ft_var.countdown = priv->pshare->rf_ft_var.monitor_time;
		if(priv->pshare->rf_ft_var.del_ther > priv->pshare->rf_ft_var.ther_drop){// hasn't drop to expected ther yet during monitor_tim
			priv->pshare->rf_ft_var.txduty_level++;
			thermal_printk("Upgrade Limit Level %d\n", priv->pshare->rf_ft_var.txduty_level);
			decide_limit_tp_new(priv);
		}else{
			priv->pshare->rf_ft_var.ther_drop -= THER_DIFF;
		}
	}
}

void ther_dm_switch(struct rtl8192cd_priv *priv)
{

	if(priv->pshare->rf_ft_var.state == STATE_INIT){
		if (priv->pshare->rf_ft_var.del_ther >= priv->pshare->rf_ft_var.ther_hi)
			priv->pshare->rf_ft_var.hitcount_hi++;
		else if (priv->pshare->rf_ft_var.hitcount_hi)
			priv->pshare->rf_ft_var.hitcount_hi--;		
		state_init_dm(priv);
	}else if(priv->pshare->rf_ft_var.state == STATE_THERMAL_CONTROL){
		if (priv->pshare->rf_ft_var.del_ther <= priv->pshare->rf_ft_var.ther_low)
			priv->pshare->rf_ft_var.hitcount_low++;
		else if (priv->pshare->rf_ft_var.hitcount_low)
			priv->pshare->rf_ft_var.hitcount_low--;
		state_thermal_control_dm(priv);
	}
}

void set_power_desc(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	
	phead = &priv->asoc_list;
	plist = phead;

	/* degrad power per STA */
	//if((plist = asoc_list_get_next(priv, plist)) != phead)
	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if (pstat){
			clear_short_cut_cache(priv, pstat); /*clear short cut cache so that new Tx power desc can take effect*/
			thermal_printk("[sta%u] clear short cut cache\n", pstat->aid);
		}
	}
}

void thermal_control_dm(struct rtl8192cd_priv *priv)
{

	/* get delta thermal  */
	if(!priv->pshare->rf_ft_var.man)
		update_del_ther(priv);

	/* decides mechanism on off*/
	if(priv->pshare->rf_ft_var.ther_dm == 1)
		ther_dm_switch(priv);

	/* 1. Tx Power*/
	if(priv->pshare->rf_ft_var.low_power == 1){
		if(degrade_power(priv)) /* degrade Tx power by descriptor per STA */
			set_power_desc(priv);
	}else if(priv->pshare->rf_ft_var.low_power == 2){ /*  set Tx power from MIB power_desc*/
		if(priv->pshare->rf_ft_var.current_power_desc != (priv->pshare->rf_ft_var.power_desc) ){
			priv->pshare->rf_ft_var.current_power_desc = priv->pshare->rf_ft_var.power_desc;
			set_power_desc(priv);
			thermal_printk("Use %ddB\n", priv->pshare->rf_ft_var.current_power_desc==3?-11:priv->pshare->rf_ft_var.current_power_desc==2?-7:priv->pshare->rf_ft_var.current_power_desc==1?-3:priv->pshare->rf_ft_var.current_power_desc==0?0:100);
		}
	}else if(priv->pshare->rf_ft_var.low_power == 0){
		if(cancel_degrade_power(priv)) /* cancel degrading Tx power by descriptor per STA */
			set_power_desc(priv);
	}

	/* 2. 2T -> 1S */
	set_path(priv); /* decides 2T to 1T for 1SS */

	/* 2.2 Antenna Diverisy */
#if 0	
	if(priv->pshare->rf_ft_var.path == 1 &&  /* if uses 1T */
		priv->pshare->rf_ft_var.path_select == 1 ){ /* enable path selection */
		ant_div(priv);  /* decides path A or path B; B or C for 8814 */
	}
#endif 

#if 1
	/* Manual Tx Duty Cycle*/
	if(priv->pshare->rf_ft_var.man == 1){
		if(priv->pshare->rf_ft_var.txduty == 1){
			limit_tp(priv);
		}
/*		
		if(priv->pshare->rf_ft_var.pa_refresh == 1){
			decide_limit_tp(priv, 0);
			priv->pshare->rf_ft_var.pa_refresh = 0;	
		}else if(priv->pshare->rf_ft_var.txduty == 1){
			decide_limit_tp(priv, 1);
		}else if(priv->pshare->rf_ft_var.txduty == 0){
			cancel_limit_tp(priv);
		}
*/		
	}	
#endif
}

#endif
#ifdef BT_COEXIST
int c2h_bt_cnt =0;
int bt_state;


void bt_coex_dm(struct rtl8192cd_priv *priv)
{
	int bt_disable=0;
	unsigned int reg770, reg774;
	reg770 = PHY_QueryBBReg(priv, 0x770, bMaskHWord);
	reg774 = PHY_QueryBBReg(priv, 0x774, bMaskHWord); /* BT TRx counter*/
	if(c2h_bt_cnt == 0 && reg770 == 0 && reg774 == 0){
		bt_disable = 1;
	} else{
		bt_disable = 0;
		c2h_bt_cnt = 0;
	}
	/* 
	 *	BT enable --> BT disable, turn off TDMA 
	*/
	if(bt_disable && bt_state){ 
		unsigned char H2CCommand[1]={0};
		H2CCommand[0] = 0x8;		
		FillH2CCmd88XX(priv, H2C_88XX_BT_TDMA, 1, H2CCommand);
		if(priv->pshare->rf_ft_var.bt_dump)
			panic_printk("[%s] BT disable\n",__FUNCTION__);

		delay_ms(10);
		PHY_SetBBReg(priv, 0x40, bMaskDWord, 0x200);
		bt_state = 0;
	}
	/* 
	 *	BT disable --> BT enable, turn on TDMA 
	*/
	else if(!bt_disable && !bt_state){ 
		unsigned char H2CCommand[6]={0};
		unsigned int center_ch;
		if(priv->pshare->rf_ft_var.bt_dump)
			panic_printk("[%s] BT alive\n",__FUNCTION__);

		PHY_SetBBReg(priv, 0x40, bMaskDWord, 0x220);
		/*
		*	Parameter 1: e3,12,12,21,10,0
		*	Parameter 2: e3,12,03,31,10,0 
		*/
		H2CCommand[0] = 0x61;
		H2CCommand[1] = 0x12;
		H2CCommand[2] = 0x03; 
		H2CCommand[3] = 0x31; 
		H2CCommand[4] = 0x10;
		H2CCommand[5] = 0x0;
		FillH2CCmd88XX(priv, H2C_88XX_BT_TDMA, 6, H2CCommand);

		center_ch = get_center_channel(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan, 1);
		H2CCommand[0] = 0x01;
		H2CCommand[1] = center_ch;							
		if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40){
			H2CCommand[2] = 0x30;
		} else {				
			H2CCommand[2] = 0x20;
		}
		FillH2CCmd88XX(priv, 0x66, 3, H2CCommand);
		bt_state = 1;
	}
}
#endif

#ifdef CONFIG_RF_DPK_SETTING_SUPPORT
void TX_DPK_Tracking(struct rtl8192cd_priv *priv)
{
	if (priv->up_time % priv->pshare->rf_ft_var.dpk_period == 0) {

#ifdef CONFIG_WLAN_HAL_8197F
	if (GET_CHIP_VER(priv) == VERSION_8197F) {
		phy_dpk_track_8197f(ODMPTR);		
		}
#endif	
	}	
}
#endif


void TXPowerTracking(struct rtl8192cd_priv *priv)
{
	if (priv->up_time % priv->pshare->rf_ft_var.tpt_period == 0) {
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
		if (!IS_OUTSRC_CHIP(priv))
#endif
		{				
#ifdef CONFIG_RTL_92C_SUPPORT			
			if ((GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C))
				tx_power_tracking(priv);
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8192D) 
				tx_power_tracking_92D(priv);			
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8188E)
				odm_TXPowerTrackingCallback_ThermalMeter_8188E(priv);			
#endif	
		}			
#endif 
#if defined(USE_OUT_SRC)
	{
		if (IS_OUTSRC_CHIP(priv)) {
			#ifndef TPT_THREAD
			ODM_TXPowerTrackingCheck(ODMPTR);
			#else
			rtl_atomic_set(&priv->pshare->do_tpt, 1);
			#endif
		}
	}
#endif
	} 
	else if ((priv->up_time % (priv->pshare->rf_ft_var.tpt_period)) == (priv->pshare->rf_ft_var.tpt_period -1)) 
	{
		// It will execute the TX power tracking at next Sec, so Trigger the Thermo Meter update first.
#ifdef CONFIG_RTL_92C_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192C || GET_CHIP_VER(priv) == VERSION_8188C)
			PHY_SetRFReg(priv, RF92CD_PATH_A, RF_T_METER, bMask20Bits, 0x60);
		else
#endif
		// for 8192D, 8188E, 8192E, 8821 serious, the Thermo meter register address is 0x42
		PHY_SetRFReg(priv, RF92CD_PATH_A, RF_T_METER_92D, BIT(16)|BIT(17), 0x3);

		//panic_printk("%s:%d trigger thermalmeter!\n", __FUNCTION__, __LINE__);
		return;
	} 
}

void SetTxPowerLevel(struct rtl8192cd_priv *priv, unsigned char channel)
{
	//unsigned int channel = priv->pmib->dot11RFEntry.dot11channel;	
#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		PHY_SetOFDMTxPower_8812(priv, channel);
		if (priv->pshare->curr_band == BAND_2G)
			PHY_SetCCKTxPower_8812(priv, channel);
			priv->pshare->No_RF_Write = 1;
		
	}
#endif
	
#if defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
	if ((GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)) {
		GET_HAL_INTERFACE(priv)->PHYSetOFDMTxPowerHandler(priv, channel);
		if (priv->pshare->curr_band == BAND_2G) {
			if (RT_STATUS_FAILURE == GET_HAL_INTERFACE(priv)->PHYSetCCKTxPowerHandler(priv, channel)) {
				DEBUG_WARN("PHYSetCCKTxPower Fail !\n");
			}
		}
		priv->pshare->No_RF_Write = 1;			
	}
#endif //#if defined(CONFIG_WLAN_HAL_8881A)
	
#if defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8197F)
	if (GET_CHIP_VER(priv) == VERSION_8192E ||(GET_CHIP_VER(priv) == VERSION_8197F))  {
		GET_HAL_INTERFACE(priv)->PHYSetOFDMTxPowerHandler(priv, channel);
		if (priv->pshare->curr_band == BAND_2G) {
			GET_HAL_INTERFACE(priv)->PHYSetCCKTxPowerHandler(priv, channel);
		}	
	}
#endif //#if defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8197F)

#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv) <= VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
		PHY_RF6052SetOFDMTxPower(priv, channel);
		if (priv->pshare->curr_band == BAND_2G)
			PHY_RF6052SetCCKTxPower(priv, channel);
	}
#endif

	selectMinPowerIdex(priv);

	return;
}

#ifdef _DEBUG_RTL8192CD_

//_TXPWR_REDEFINE
int Read_PG_File(struct rtl8192cd_priv *priv, int reg_file, int table_number,
				  char *MCSTxAgcOffset_A, char *MCSTxAgcOffset_B, char *OFDMTxAgcOffset_A,
				  char *OFDMTxAgcOffset_B, char *CCKTxAgc_A, char *CCKTxAgc_B)
{
	int                read_bytes = 0, num, len = 0, round;
	unsigned int       u4bRegOffset, u4bRegValue, u4bRegMask;
	unsigned char      *mem_ptr, *line_head, *next_head = NULL;
	struct PhyRegTable *phyreg_table = NULL;
	struct MacRegTable *macreg_table = NULL;
	unsigned short     max_len = 0;
	int                file_format = TWO_COLUMN;
#ifdef CONFIG_RTL_92D_SUPPORT
	int				idx = 0, pg_tbl_idx = table_number, write_en = 0;

	int tmp_rTxAGC_A_CCK1_Mcs32 = 0;
	int tmp_rTxAGC_B_CCK5_1_Mcs32 = 0;
	int prev_reg = 0;
#endif

	//printk("PHYREG_PG = %d\n", PHYREG_PG);

	if (reg_file == PHYREG_PG) {
		//printk("[%s][PHY_REG_PG]\n",__FUNCTION__);

#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {

			if (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_FCC) {
				//printk("\nFCC PG!!!\n");
				next_head = data_PHY_REG_PG_FCC_start;
				read_bytes = (int)(data_PHY_REG_PG_FCC_end - data_PHY_REG_PG_FCC_start);
			} else if (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_ETSI) {
				//printk("\nCE PG!!!\n");
				next_head = data_PHY_REG_PG_CE_start;
				read_bytes = (int)(data_PHY_REG_PG_CE_end - data_PHY_REG_PG_CE_start);
			} else {
				//printk("\nOTHER PG!!!\n");
				next_head = data_PHY_REG_PG_start;
				read_bytes = (int)(data_PHY_REG_PG_end - data_PHY_REG_PG_start);
			}

#ifdef HIGH_POWER_EXT_PA
			if ( priv->pshare->rf_ft_var.use_ext_pa) {
				//printk("[%s][data_PHY_REG_PG_92d_hp]\n", __FUNCTION__);
				next_head = data_PHY_REG_PG_92d_hp_start;
				read_bytes = (int)(data_PHY_REG_PG_92d_hp_end - data_PHY_REG_PG_92d_hp_start);
			}
#endif

		}
#endif //CONFIG_RTL_92D_SUPPORT

#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
#ifdef HIGH_POWER_EXT_PA
			if ( priv->pshare->rf_ft_var.use_ext_pa) {
				//printk("[%s][data_PHY_REG_PG_hp]\n", __FUNCTION__);
				next_head = data_PHY_REG_PG_hp_start;
				read_bytes = (int)(data_PHY_REG_PG_hp_end - data_PHY_REG_PG_hp_start);

			} else
#endif
			{
				//printk("[%s][data_PHY_REG_PG_92C]\n", __FUNCTION__);
				next_head = data_PHY_REG_PG_92C_start;
				read_bytes = (int)(data_PHY_REG_PG_92C_end - data_PHY_REG_PG_92C_start);
			}
		}
#endif //CONFIG_RTL_92C_SUPPORT
		macreg_table = (struct MacRegTable *)priv->pshare->phy_reg_pg_buf;
		max_len = PHY_REG_PG_SIZE;
		file_format = THREE_COLUMN;
	}

#ifdef CONFIG_RTL_92D_SUPPORT
	else if (reg_file == PHYREG) {
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
			//printk("[%s][PHY_REG_n]\n",__FUNCTION__);
			next_head = data_PHY_REG_n_start;
			read_bytes = (int)(data_PHY_REG_n_end - data_PHY_REG_n_start);
			max_len = PHY_REG_SIZE;
		}
	}
#endif // CONFIG_RTL_92D_SUPPORT

#ifdef MP_TEST
	else if (reg_file == PHYREG_MP) {
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
			//printk("[%s][PHY_REG_MP_n]\n",__FUNCTION__);
			next_head = data_PHY_REG_MP_n_start;
			read_bytes = (int)(data_PHY_REG_MP_n_end - data_PHY_REG_MP_n_start);
			max_len = PHY_REG_SIZE;
		}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
			next_head = data_PHY_REG_MP_n_92C_start;
			read_bytes = (int)(data_PHY_REG_MP_n_92C_end - data_PHY_REG_MP_n_92C_start);
			max_len = PHY_REG_SIZE;
		}
#endif
	}
#endif

	{
		if ((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigBBWithParaFile(): not enough memory\n");
			return -1;
		}

		memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory
		memcpy(mem_ptr, next_head, read_bytes);

		next_head = mem_ptr;
		round = 0;
		while (1) {
			line_head = next_head;
			next_head = get_line(&line_head);
			if (line_head == NULL)
				break;

			if (line_head[0] == '/')
				continue;

			if (file_format == TWO_COLUMN) {
				num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
				if (num > 0) {
					phyreg_table[len].offset = u4bRegOffset;
					phyreg_table[len].value = u4bRegValue;
					len++;

					if (u4bRegOffset == 0xff)
						break;
					if ((len * sizeof(struct PhyRegTable)) > max_len)
						break;
				}
			} else {
				num = get_offset_mask_val(line_head, &u4bRegOffset, &u4bRegMask , &u4bRegValue);
				if (num > 0) {
					macreg_table[len].offset = u4bRegOffset;
					macreg_table[len].mask = u4bRegMask;
					macreg_table[len].value = u4bRegValue;
					len++;
					if (u4bRegOffset == 0xff)
						break;
					if ((len * sizeof(struct MacRegTable)) > max_len)
						break;
				}
			}

			if (++round > 10000) {
				panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
				break;
			}
		}

		kfree(mem_ptr);

		if ((len * sizeof(struct PhyRegTable)) > max_len) {
			printk("PHY REG PG table buffer not large enough!\n");
			printk("len=%d,sizeof(struct PhyRegTable)=%d,max_len=%u",len,sizeof(struct PhyRegTable),max_len);
			return -1;
		}
	}

	num = 0;
	round = 0;
	while (1) {
		if (file_format == THREE_COLUMN) {
			u4bRegOffset = macreg_table[num].offset;
			u4bRegValue = macreg_table[num].value;
			u4bRegMask = macreg_table[num].mask;
		} else {
			u4bRegOffset = phyreg_table[num].offset;
			u4bRegValue = phyreg_table[num].value;
		}

		if (u4bRegOffset == 0xff)
			break;
		else if (file_format == THREE_COLUMN) {

#ifdef CONFIG_RTL_92D_SUPPORT
			if (reg_file == PHYREG_PG && GET_CHIP_VER(priv) == VERSION_8192D) {
				if (u4bRegOffset == 0xe00) {
					if (idx == pg_tbl_idx)
						write_en = 1;
					idx++;
				}
				if (write_en) {
					//PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
					//printk("3C- 92D %x %x %x \n", u4bRegOffset, u4bRegMask, u4bRegValue);
					if (u4bRegMask != bMaskDWord) {
						int BitShift = phy_CalculateBitShift(u4bRegMask);
						u4bRegValue = (u4bRegValue << BitShift);
					}

					//=== PATH A ===

					if (u4bRegOffset == rTxAGC_A_Mcs03_Mcs00)
						*(unsigned int *)(&MCSTxAgcOffset_A[0]) = cpu_to_be32(u4bRegValue);
					if (u4bRegOffset == rTxAGC_A_Mcs07_Mcs04)
						*(unsigned int *)(&MCSTxAgcOffset_A[4]) = cpu_to_be32(u4bRegValue);
					if (u4bRegOffset == rTxAGC_A_Mcs11_Mcs08)
						*(unsigned int *)(&MCSTxAgcOffset_A[8]) = cpu_to_be32(u4bRegValue);
					if (u4bRegOffset == rTxAGC_A_Mcs15_Mcs12)
						*(unsigned int *)(&MCSTxAgcOffset_A[12]) = cpu_to_be32(u4bRegValue);

					if (u4bRegOffset == rTxAGC_A_Rate18_06)
						*(unsigned int *)(&OFDMTxAgcOffset_A[0]) = cpu_to_be32(u4bRegValue);
					if (u4bRegOffset == rTxAGC_A_Rate54_24)
						*(unsigned int *)(&OFDMTxAgcOffset_A[4]) = cpu_to_be32(u4bRegValue);

					if (u4bRegOffset == rTxAGC_A_CCK1_Mcs32) {
						tmp_rTxAGC_A_CCK1_Mcs32 = ((u4bRegValue & 0xff00) >> phy_CalculateBitShift(0xff00));
						prev_reg = rTxAGC_A_CCK1_Mcs32;
					}

					if (u4bRegOffset == rTxAGC_A_CCK11_2_B_CCK11) {
						if (prev_reg == rTxAGC_A_CCK1_Mcs32) {
							//printk("\n%x %x %x\n", tmp_rTxAGC_A_CCK1_Mcs32, u4bRegValue, cpu_to_be32((u4bRegValue & 0xffffff00) | tmp_rTxAGC_A_CCK1_Mcs32));

							*(unsigned int *)(&CCKTxAgc_A[0]) =
								cpu_to_be32((u4bRegValue & 0xffffff00) | tmp_rTxAGC_A_CCK1_Mcs32);
						}
					}

					//=== PATH B ===

					if (u4bRegOffset == rTxAGC_B_Mcs03_Mcs00)
						*(unsigned int *)(&MCSTxAgcOffset_B[0]) = cpu_to_be32(u4bRegValue);
					if (u4bRegOffset == rTxAGC_B_Mcs07_Mcs04)
						*(unsigned int *)(&MCSTxAgcOffset_B[4]) = cpu_to_be32(u4bRegValue);
					if (u4bRegOffset == rTxAGC_B_Mcs11_Mcs08)
						*(unsigned int *)(&MCSTxAgcOffset_B[8]) = cpu_to_be32(u4bRegValue);
					if (u4bRegOffset == rTxAGC_B_Mcs15_Mcs12)
						*(unsigned int *)(&MCSTxAgcOffset_B[12]) = cpu_to_be32(u4bRegValue);

					if (u4bRegOffset == rTxAGC_B_Rate18_06)
						*(unsigned int *)(&OFDMTxAgcOffset_B[0]) = cpu_to_be32(u4bRegValue);
					if (u4bRegOffset == rTxAGC_B_Rate54_24)
						*(unsigned int *)(&OFDMTxAgcOffset_B[4]) = cpu_to_be32(u4bRegValue);

					if (u4bRegOffset == rTxAGC_B_CCK5_1_Mcs32) {
						tmp_rTxAGC_B_CCK5_1_Mcs32 = u4bRegValue;
						prev_reg = rTxAGC_B_CCK5_1_Mcs32;
					}

					if (u4bRegOffset == rTxAGC_A_CCK11_2_B_CCK11) {
						if (prev_reg == rTxAGC_B_CCK5_1_Mcs32) {
							//printk("\n%x %x %x\n", tmp_rTxAGC_B_CCK5_1_Mcs32, u4bRegValue, cpu_to_be32((u4bRegValue << 24) | (tmp_rTxAGC_B_CCK5_1_Mcs32 >> 8)));
							*(unsigned int *)(&CCKTxAgc_B[0]) = cpu_to_be32((u4bRegValue << 24) | (tmp_rTxAGC_B_CCK5_1_Mcs32 >> 8));
						}
					}

					if (u4bRegOffset == 0x868) {
						write_en = 0;
						break;
					}
				}
			} else
#endif
			{
				//PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
				//printk("3C - 92C %x %x %x \n", u4bRegOffset, u4bRegMask, u4bRegValue);
			}
		} else {
			//printk("Not 3C - %x %x %x \n", u4bRegOffset, bMaskDWord, u4bRegValue);
			//PHY_SetBBReg(priv, u4bRegOffset, bMaskDWord, u4bRegValue);
		}
		num++;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	return 0;
}

#endif

/*-----------------------------------------------------------------------------
 * Function:    PHY_ConfigBBWithParaFile()
 *
 * Overview:    This function read BB parameters from general file format, and do register
 *			  Read/Write
 *
 * Input:      	PADAPTER		Adapter
 *			ps1Byte 			pFileName
 *
 * Output:      NONE
 *
 * Return:      RT_STATUS_SUCCESS: configuration file exist
 *
 *---------------------------------------------------------------------------*/
int PHY_ConfigBBWithParaFile(struct rtl8192cd_priv *priv, int reg_file)
{
	int                read_bytes = 0, num = 0, len = 0, round;
	unsigned int       u4bRegOffset, u4bRegValue, u4bRegMask = 0;
	int 			   file_format = TWO_COLUMN;
	unsigned char      *mem_ptr, *line_head, *next_head = NULL;
#if 1//defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL)//TXPWR_LMT_8812 TXPWR_LMT_88E
	unsigned char      *mem_ptr2, *next_head2=NULL;
#endif
	struct PhyRegTable *phyreg_table = NULL;
	struct MacRegTable *macreg_table = NULL;
	unsigned short     max_len = 0;
	unsigned int 		regstart, regend;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8723B_SUPPORT)
	int				idx = 0, pg_tbl_idx = BGN_2040_ALL, write_en = 0;
#endif

	if (reg_file == AGCTAB) {
		phyreg_table = (struct PhyRegTable *)priv->pshare->agc_tab_buf;
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
#ifdef HIGH_POWER_EXT_PA //_eric_?? DMDP & SMSP ??
			if (priv->pshare->rf_ft_var.use_ext_pa) {
				//printk("[%s][AGC_TAB_n_92d_hp]\n",__FUNCTION__);
				next_head = data_AGC_TAB_n_92d_hp_start;
				read_bytes = (int)(data_AGC_TAB_n_92d_hp_end - data_AGC_TAB_n_92d_hp_start);
			} else {
				//printk("[%s][AGC_TAB_n]\n",__FUNCTION__);
				next_head = data_AGC_TAB_n_start;
				read_bytes = (int)(data_AGC_TAB_n_end - data_AGC_TAB_n_start);
			}
#else //HIGH_POWER_EXT_PA
#ifdef CONFIG_RTL_92D_DMDP
			if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1) {
				if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
					//printk("[%s][AGC_TAB_5G_n]\n",__FUNCTION__);
					next_head = data_AGC_TAB_5G_n_start;
					read_bytes = (int)(data_AGC_TAB_5G_n_end - data_AGC_TAB_5G_n_start);
				} else {
					//printk("[%s][AGC_TAB_2G_n]\n",__FUNCTION__);
					next_head = data_AGC_TAB_2G_n_start;
					read_bytes = (int)(data_AGC_TAB_2G_n_end - data_AGC_TAB_2G_n_start);
				}
			} else
#endif
			{
				//printk("[%s][AGC_TAB_n]\n",__FUNCTION__);
				next_head = data_AGC_TAB_n_start;
				read_bytes = (int)(data_AGC_TAB_n_end - data_AGC_TAB_n_start);
			}
#endif //HIGH_POWER_EXT_PA
		}
#endif //CONFIG_RTL_92D_SUPPORT

#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
#ifdef TESTCHIP_SUPPORT
			if (IS_TEST_CHIP(priv)) {
				next_head = data_AGC_TAB_start;
				read_bytes = (int)(data_AGC_TAB_end - data_AGC_TAB_start);
			} else
#endif

#if defined(HIGH_POWER_EXT_PA) || defined(HIGH_POWER_EXT_LNA)
				if (
#ifdef HIGH_POWER_EXT_PA					
					priv->pshare->rf_ft_var.use_ext_pa
#else
					priv->pshare->rf_ft_var.use_ext_lna
#endif
				) {
					next_head = data_AGC_TAB_n_hp_start;
					read_bytes = (int)(data_AGC_TAB_n_hp_end - data_AGC_TAB_n_hp_start);
				} else 
#endif
				{
					next_head = data_AGC_TAB_n_92C_start;
					read_bytes = (int)(data_AGC_TAB_n_92C_end - data_AGC_TAB_n_92C_start);
				}
		}
#endif



#ifdef CONFIG_RTL_8812_SUPPORT //8812 agc
		if (GET_CHIP_VER(priv) == VERSION_8812E) {
			if (IS_TEST_CHIP(priv)) {
#ifdef HIGH_POWER_EXT_PA 				
				if (priv->pshare->rf_ft_var.use_ext_pa) {
					printk("[AGC_TAB_8812_hp]\n");
					next_head = data_AGC_TAB_8812_hp_start;
					read_bytes = (int)(data_AGC_TAB_8812_hp_end - data_AGC_TAB_8812_hp_start);
				} else 
#endif
				{
					printk("[AGC_TAB_8812]\n");
					next_head = data_AGC_TAB_8812_start;
					read_bytes = (int)(data_AGC_TAB_8812_end - data_AGC_TAB_8812_start);
				}
			} else { //for_8812_mp_chip
#ifdef HIGH_POWER_EXT_PA 
#ifdef HIGH_POWER_EXT_LNA			
				if (priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
					panic_printk("[AGC_TAB_8812_n_hp]\n");
					next_head = data_AGC_TAB_8812_n_hp_start;
					read_bytes = (int)(data_AGC_TAB_8812_n_hp_end - data_AGC_TAB_8812_n_hp_start);
				} else if (priv->pshare->rf_ft_var.use_ext_pa && !priv->pshare->rf_ft_var.use_ext_lna) {
					panic_printk("[AGC_TAB_8812_n_extpa]\n");
					next_head = data_AGC_TAB_8812_n_extpa_start;
					read_bytes = (int)(data_AGC_TAB_8812_n_extpa_end - data_AGC_TAB_8812_n_extpa_start);
				} else if (!priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
					panic_printk("[AGC_TAB_8812_n_extlna]\n");
					next_head = data_AGC_TAB_8812_n_extlna_start;
					read_bytes = (int)(data_AGC_TAB_8812_n_extlna_end - data_AGC_TAB_8812_n_extlna_start);
				} else 
#else
				if (priv->pshare->rf_ft_var.use_ext_pa) {
					panic_printk("[AGC_TAB_8812_n_extpa]\n");
					next_head = data_AGC_TAB_8812_n_extpa_start;
					read_bytes = (int)(data_AGC_TAB_8812_n_extpa_end - data_AGC_TAB_8812_n_extpa_start);
				} else					
#endif
#else	//HIGH_POWER_EXT_PA=n
#ifdef HIGH_POWER_EXT_LNA
				if (priv->pshare->rf_ft_var.use_ext_lna) {
					panic_printk("[AGC_TAB_8812_n_extlna]\n");
					next_head = data_AGC_TAB_8812_n_extlna_start;
					read_bytes = (int)(data_AGC_TAB_8812_n_extlna_end - data_AGC_TAB_8812_n_extlna_start);
				} else				
#endif
#endif
				{
					printk("[AGC_TAB_8812_n_default]\n");
					next_head = data_AGC_TAB_8812_n_default_start;
					read_bytes = (int)(data_AGC_TAB_8812_n_default_end - data_AGC_TAB_8812_n_default_start);
				}
			}
		}
#endif

#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
#ifdef HIGH_POWER_EXT_PA
#ifdef HIGH_POWER_EXT_LNA
			if (priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_HP_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_HP_START, (pu1Byte)&next_head);
			} else if (priv->pshare->rf_ft_var.use_ext_pa && !priv->pshare->rf_ft_var.use_ext_lna) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTPA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTPA_START, (pu1Byte)&next_head);
			} else if (!priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {			
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTLNA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTLNA_START, (pu1Byte)&next_head);
			} else
#else
			if (priv->pshare->rf_ft_var.use_ext_pa) {				
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTPA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTPA_START, (pu1Byte)&next_head);
			} else
#endif
#else
#ifdef HIGH_POWER_EXT_LNA
			if (priv->pshare->rf_ft_var.use_ext_lna) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTLNA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_EXTLNA_START, (pu1Byte)&next_head);
			} else
#endif
#endif
			{
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_AGC_START, (pu1Byte)&next_head);
			}
		}
#endif //CONFIG_WLAN_HAL

		max_len = AGC_TAB_SIZE;
	} else if (reg_file == PHYREG_PG) {
		//printk("[%s][PHY_REG_PG]\n",__FUNCTION__);
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {

			if (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_FCC) {
				//printk("\nFCC PG!!!\n");
				next_head = data_PHY_REG_PG_FCC_start;
				read_bytes = (int)(data_PHY_REG_PG_FCC_end - data_PHY_REG_PG_FCC_start);
			} else if (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_ETSI) {
				//printk("\nCE PG!!!\n");
				next_head = data_PHY_REG_PG_CE_start;
				read_bytes = (int)(data_PHY_REG_PG_CE_end - data_PHY_REG_PG_CE_start);
			} else {
				//printk("\nOTHER PG!!!\n");
				next_head = data_PHY_REG_PG_start;
				read_bytes = (int)(data_PHY_REG_PG_end - data_PHY_REG_PG_start);
			}

#ifdef HIGH_POWER_EXT_PA
			if ( priv->pshare->rf_ft_var.use_ext_pa) {
				//printk("[%s][data_PHY_REG_PG_92d_hp]\n", __FUNCTION__);
				next_head = data_PHY_REG_PG_92d_hp_start;
				read_bytes = (int)(data_PHY_REG_PG_92d_hp_end - data_PHY_REG_PG_92d_hp_start);
			}
#endif

//_TXPWR_REDEFINE ?? Why 5G no need working channel ??
//_TXPWR_REDEFINE in MP Tool, 3 Groups: 36-99 100-148 149-165
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
				if (priv->pshare->is_40m_bw == 0) {
					if (priv->pmib->dot11RFEntry.dot11channel <= 99)
						pg_tbl_idx = AN_20_CH_36_64;
					else if (priv->pmib->dot11RFEntry.dot11channel <= 148)
						pg_tbl_idx = AN_20_CH_100_140;
					else
						pg_tbl_idx = AN_20_CH_149_165;
				} else {
					//_TXPWR_REDEFINE ??
					int val = priv->pmib->dot11RFEntry.dot11channel;

					if (priv->pshare->offset_2nd_chan == 1)
						val -= 2;
					else
						val += 2;

					if (priv->pmib->dot11RFEntry.dot11channel <= 99)
						pg_tbl_idx = AN_40_CH_36_64;
					else if (priv->pmib->dot11RFEntry.dot11channel <= 148)
						pg_tbl_idx = AN_40_CH_100_140;
					else
						pg_tbl_idx = AN_40_CH_149_165;
				}
			} else {
				if (priv->pshare->is_40m_bw == 0) {
					if (priv->pmib->dot11RFEntry.dot11channel <= 3)
						pg_tbl_idx = BGN_20_CH1_3;
					else if (priv->pmib->dot11RFEntry.dot11channel <= 9)
						pg_tbl_idx = BGN_20_CH4_9;
					else
						pg_tbl_idx = BGN_20_CH10_14;
				} else {
					int val = priv->pmib->dot11RFEntry.dot11channel;

					if (priv->pshare->offset_2nd_chan == 1)
						val -= 2;
					else
						val += 2;

					if (val <= 3)
						pg_tbl_idx = BGN_40_CH1_3;
					else if (val <= 9)
						pg_tbl_idx = BGN_40_CH4_9;
					else
						pg_tbl_idx = BGN_40_CH10_14;
				}
			}
#ifdef MP_TEST
			//In Noraml Driver mode, and if mib 'pwr_by_rate' = 0 >> Use default power by rate table
			if ( (priv->pshare->rf_ft_var.mp_specific == 0) && (priv->pshare->rf_ft_var.pwr_by_rate == 0) )
				pg_tbl_idx = BGN_2040_ALL;
#endif
			DEBUG_INFO("channel=%d pg_tbl_idx=%d\n", priv->pmib->dot11RFEntry.dot11channel, pg_tbl_idx);

		}
#endif //CONFIG_RTL_92D_SUPPORT

#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
#ifdef HIGH_POWER_EXT_PA
			if ( priv->pshare->rf_ft_var.use_ext_pa) {
				//printk("[%s][data_PHY_REG_PG_hp]\n", __FUNCTION__);
				next_head = data_PHY_REG_PG_hp_start;
				read_bytes = (int)(data_PHY_REG_PG_hp_end - data_PHY_REG_PG_hp_start);

			} else
#endif
			{
				//printk("[%s][data_PHY_REG_PG_92C]\n", __FUNCTION__);
				next_head = data_PHY_REG_PG_92C_start;
				read_bytes = (int)(data_PHY_REG_PG_92C_end - data_PHY_REG_PG_92C_start);
			}
		}
#endif //CONFIG_RTL_92C_SUPPORT

#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E) {
#ifdef SUPPORT_RTL8188E_TC
			if (IS_TEST_CHIP(priv))
				return 0;
#endif

#ifdef TXPWR_LMT_88E
#ifdef __ECOS
			DEBUG_INFO("[%s][PHY_REG_PG_88E_new]\n", __FUNCTION__);
#else
			printk("[%s][PHY_REG_PG_88E_new]\n", __FUNCTION__);
#endif
			next_head = data_PHY_REG_PG_88E_new_start;
			read_bytes = (int)(data_PHY_REG_PG_88E_new_end - data_PHY_REG_PG_88E_new_start);

			pg_tbl_idx = 0;
#else

			printk("[%s][PHY_REG_PG_88E]\n", __FUNCTION__);
			next_head = data_PHY_REG_PG_88E_start;
			read_bytes = (int)(data_PHY_REG_PG_88E_end - data_PHY_REG_PG_88E_start);

			/* In Noraml Driver mode, and if mib 'pwr_by_rate' = 0 >> Use default power by rate table  */
			if (
#ifdef MP_TEST
				priv->pshare->rf_ft_var.mp_specific ||
#endif
				priv->pshare->rf_ft_var.pwr_by_rate) {
				if (priv->pshare->is_40m_bw == 0) {
					if (priv->pmib->dot11RFEntry.dot11channel <= 3)
						pg_tbl_idx = BGN_20_CH1_3;
					else if (priv->pmib->dot11RFEntry.dot11channel <= 9)
						pg_tbl_idx = BGN_20_CH4_9;
					else
						pg_tbl_idx = BGN_20_CH10_14;
				} else {
					int val = priv->pmib->dot11RFEntry.dot11channel;

					if (priv->pshare->offset_2nd_chan == 1)
						val -= 2;
					else
						val += 2;

					if (val <= 3)
						pg_tbl_idx = BGN_40_CH1_3;
					else if (val <= 9)
						pg_tbl_idx = BGN_40_CH4_9;
					else
						pg_tbl_idx = BGN_40_CH10_14;
				}
			}
#endif

#ifdef MP_TEST
			if (priv->pshare->rf_ft_var.mp_specific)
				pg_tbl_idx = 0;
#endif

			DEBUG_INFO("channel=%d pg_tbl_idx=%d\n", priv->pmib->dot11RFEntry.dot11channel, pg_tbl_idx);
		}
#endif

#ifdef CONFIG_RTL_8723B_SUPPORT 
		if (GET_CHIP_VER(priv) == VERSION_8723B) {
			if (IS_OUTSRC_CHIP(priv)) {
				next_head = data_PHY_REG_PG_8723_start;
				read_bytes = (int)(data_PHY_REG_PG_8723_end - data_PHY_REG_PG_8723_start);
				panic_printk("(%s)line=%d, 8723B PG table, read_bytes = %d\n", __FUNCTION__, __LINE__, read_bytes); 
			}
		}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT //eric_8812 pg
		if (GET_CHIP_VER(priv) == VERSION_8812E) {
			if (priv->pshare->curr_band == BAND_5G)
				pg_tbl_idx = 1;
			else
				pg_tbl_idx = 0;

			priv->pshare->current_pwr_pg_index=pg_tbl_idx;

#ifdef HIGH_POWER_EXT_PA
			if(priv->pmib->dot11RFEntry.pa_type == PA_SKYWORKS_85712_HP){
				panic_printk("[PHY_REG_PG_8812_hp]\n");
				next_head = data_PHY_REG_PG_8812_hp_start;
				read_bytes = (int)(data_PHY_REG_PG_8812_hp_end - data_PHY_REG_PG_8812_hp_start);		
			}else
#endif
			{
#ifdef TXPWR_LMT_8812
				panic_printk("[PHY_REG_PG_8812_new]\n");
				next_head = data_PHY_REG_PG_8812_new_start;
				read_bytes = (int)(data_PHY_REG_PG_8812_new_end - data_PHY_REG_PG_8812_new_start);		
#else
				panic_printk("[PHY_REG_PG_8812]\n");
				next_head = data_PHY_REG_PG_8812_start;
				read_bytes = (int)(data_PHY_REG_PG_8812_end - data_PHY_REG_PG_8812_start);
#endif
			}

			DEBUG_INFO("channel=%d pg_tbl_idx=%d\n", priv->pmib->dot11RFEntry.dot11channel, pg_tbl_idx);
		}
#endif //CONFIG_RTL_8812_SUPPORT

#ifdef CONFIG_WLAN_HAL
		if ( IS_HAL_CHIP(priv) ) {
#ifdef PWR_BY_RATE_92E_HP			
#ifdef HIGH_POWER_EXT_PA
			if (priv->pshare->rf_ft_var.use_ext_pa) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_PG_HP_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_PG_HP_START, (pu1Byte)&next_head);
			} else
#endif
#endif
			{
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_PG_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_PG_START, (pu1Byte)&next_head);
			}
#if defined(CONFIG_WLAN_HAL)
			if(IS_HAL_CHIP(priv)) {
				if (priv->pshare->curr_band == BAND_5G)
					pg_tbl_idx = 1;
				else
					pg_tbl_idx = 0;

#if defined(CONFIG_WLAN_HAL_8881A)
			if (GET_CHIP_VER(priv) == VERSION_8881A)
				priv->pshare->current_pwr_pg_index=pg_tbl_idx;
#endif
			} else			
#endif
			{
			/* In Noraml Driver mode, and if mib 'pwr_by_rate' = 0 >> Use default power by rate table  */
			if (
#ifdef MP_TEST
				priv->pshare->rf_ft_var.mp_specific ||
#endif
				priv->pshare->rf_ft_var.pwr_by_rate) {
				if (priv->pshare->is_40m_bw == 0) {
					if (priv->pmib->dot11RFEntry.dot11channel <= 3)
						pg_tbl_idx = BGN_20_CH1_3;
					else if (priv->pmib->dot11RFEntry.dot11channel <= 9)
						pg_tbl_idx = BGN_20_CH4_9;
					else
						pg_tbl_idx = BGN_20_CH10_14;
				} else {
					int val = priv->pmib->dot11RFEntry.dot11channel;

					if (priv->pshare->offset_2nd_chan == 1)
						val -= 2;
					else
						val += 2;

					if (val <= 3)
						pg_tbl_idx = BGN_40_CH1_3;
					else if (val <= 9)
						pg_tbl_idx = BGN_40_CH4_9;
					else
						pg_tbl_idx = BGN_40_CH10_14;
				}
			}

#ifdef MP_TEST
			if (priv->pshare->rf_ft_var.mp_specific)
				pg_tbl_idx = 0;
#endif
			}
			DEBUG_INFO("channel=%d pg_tbl_idx=%d\n", priv->pmib->dot11RFEntry.dot11channel, pg_tbl_idx);
		}
#endif //CONFIG_WLAN_HAL

		macreg_table = (struct MacRegTable *)priv->pshare->phy_reg_pg_buf;
		max_len = PHY_REG_PG_SIZE;
		file_format = THREE_COLUMN;
		priv->pshare->txpwr_pg_format_abs = 0;

		unsigned char      *mem_ptr_tmp, *next_head_tmp=NULL;
		if ((mem_ptr_tmp= (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigBBWithParaFile(): not enough memory\n");
			return -1;
		}

		memset(mem_ptr_tmp, 0, MAX_CONFIG_FILE_SIZE); // clear memory
		memcpy(mem_ptr_tmp, next_head, read_bytes);
		
		next_head_tmp= mem_ptr_tmp;
		round = 0;
		while(1) {
			line_head = next_head_tmp;
			next_head_tmp= get_line(&line_head);
					
			if (line_head == NULL)
				break;
			if (line_head[0] == '/')
				continue;		

			if (line_head[0] == '#') {				
				int line_idx = 1; //line_len = strlen(line_head);
				
				while (line_idx <= strlen(line_head))
				{
					if (!memcmp(&(line_head[line_idx]), "Exact", 5)) {
						//panic_printk("\n******** PG Exact format !!! ********\n");
						priv->pshare->txpwr_pg_format_abs = 1;
						file_format = FIVE_COLUMN;
						break;
					}
					line_idx++;
				}
			}

			if (!memcmp(line_head, "0x", 2) || !memcmp(line_head, "0X", 2))
				break;					

			if (++round > 10000) {
				panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
				break;
			}
		}
			
		kfree(mem_ptr_tmp);

		//panic_printk("PG file_format : %s\n", ((file_format == THREE_COLUMN) ? "THREE_COLUMN" : "FIVE_COLUMN"));

#ifdef TXPWR_LMT_NEWFILE
		if ((file_format == THREE_COLUMN) && (GET_CHIP_VER(priv) >= VERSION_8188E)) {
			panic_printk("%s() fail !!!:  Wrong PHY_REG_PG format\n", __FUNCTION__);
			return -1;
		}
#endif	
		memset(priv->pshare->tgpwr_CCK_new, 0, 2);
		memset(priv->pshare->tgpwr_OFDM_new, 0, 2);
		memset(priv->pshare->tgpwr_HT1S_new, 0, 2);
		memset(priv->pshare->tgpwr_HT2S_new, 0, 2);
		memset(priv->pshare->tgpwr_VHT1S_new, 0, 2);
		memset(priv->pshare->tgpwr_VHT2S_new, 0, 2);
	}
#if 0
	else if (reg_file == PHYREG_1T2R) {
		macreg_table = (struct MacRegTable *)priv->pshare->phy_reg_2to1;
		max_len = PHY_REG_1T2R;
		file_format = THREE_COLUMN;
		if (priv->pshare->rf_ft_var.pathB_1T == 0) { // PATH A
			next_head = __PHY_to1T2R_start;
			read_bytes = (int)(__PHY_to1T2R_end - __PHY_to1T2R_start);
		} else { // PATH B
			next_head = __PHY_to1T2R_b_start;
			read_bytes = (int)(__PHY_to1T2R_b_end - __PHY_to1T2R_b_start);
		}
	}
#endif
#if defined(CONFIG_RTL_92D_SUPPORT) || defined (CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
	else if (reg_file == PHYREG) {
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
#ifdef HIGH_POWER_EXT_PA
			//printk("[%s][PHY_REG_n_92d_hp]\n",__FUNCTION__);
			next_head = data_PHY_REG_n_92d_hp_start;
			read_bytes = (int)(data_PHY_REG_n_92d_hp_end - data_PHY_REG_n_92d_hp_start);
#else
			//printk("[%s][PHY_REG_n]\n",__FUNCTION__);
			next_head = data_PHY_REG_n_start;
			read_bytes = (int)(data_PHY_REG_n_end - data_PHY_REG_n_start);
#endif
			max_len = PHY_REG_SIZE;
		}
#endif
#ifdef CONFIG_RTL_8812_SUPPORT //8812 phy
		if (GET_CHIP_VER(priv) == VERSION_8812E) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
			if (IS_TEST_CHIP(priv)) { //for_8812_mp_chip
				panic_printk("[%s][PHY_REG_8812]\n",__FUNCTION__);
				next_head = data_PHY_REG_8812_start;
				read_bytes = (int)(data_PHY_REG_8812_end - data_PHY_REG_8812_start);
			} else {
#ifdef HIGH_POWER_EXT_PA
#ifdef HIGH_POWER_EXT_LNA			
				if (priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
					panic_printk("[PHY_REG_8812_n_hp]\n");
					next_head = data_PHY_REG_8812_n_hp_start;
					read_bytes = (int)(data_PHY_REG_8812_n_hp_end - data_PHY_REG_8812_n_hp_start);
				} else if (priv->pshare->rf_ft_var.use_ext_pa && !priv->pshare->rf_ft_var.use_ext_lna) {
					 panic_printk("[PHY_REG_8812_n_extpa]\n");
					next_head = data_PHY_REG_8812_n_extpa_start;
					read_bytes = (int)(data_PHY_REG_8812_n_extpa_end - data_PHY_REG_8812_n_extpa_start);
				} else if (!priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
					 panic_printk("[PHY_REG_8812_n_extlna]\n");
					next_head = data_PHY_REG_8812_n_extlna_start;
					read_bytes = (int)(data_PHY_REG_8812_n_extlna_end - data_PHY_REG_8812_n_extlna_start);
				} else 
#else
				if (priv->pshare->rf_ft_var.use_ext_pa) {				
					 panic_printk("[PHY_REG_8812_n_extpa]\n");
					next_head = data_PHY_REG_8812_n_extpa_start;
					read_bytes = (int)(data_PHY_REG_8812_n_extpa_end - data_PHY_REG_8812_n_extpa_start);
				} else
#endif
#else
#ifdef HIGH_POWER_EXT_LNA				
				if (priv->pshare->rf_ft_var.use_ext_lna) {
					 panic_printk("[PHY_REG_8812_n_extlna]\n");
					next_head = data_PHY_REG_8812_n_extlna_start;
					read_bytes = (int)(data_PHY_REG_8812_n_extlna_end - data_PHY_REG_8812_n_extlna_start);
				} else 
#endif
#endif
				{
					panic_printk("[PHY_REG_8812_n_default]\n");
					next_head = data_PHY_REG_8812_n_default_start;
					read_bytes = (int)(data_PHY_REG_8812_n_default_end - data_PHY_REG_8812_n_default_start);
				}
			}
			max_len = PHY_REG_SIZE;
		}
#endif

#ifdef CONFIG_WLAN_HAL
		if ( IS_HAL_CHIP(priv) ) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
#ifdef HIGH_POWER_EXT_PA
#ifdef HIGH_POWER_EXT_LNA
			if (priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_HP_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_HP_START, (pu1Byte)&next_head);
			} else if (priv->pshare->rf_ft_var.use_ext_pa && !priv->pshare->rf_ft_var.use_ext_lna) {	
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTPA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTPA_START, (pu1Byte)&next_head);
			} else if (!priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {			
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTLNA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTLNA_START, (pu1Byte)&next_head);
			} else
#else
			if (priv->pshare->rf_ft_var.use_ext_pa) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTPA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTPA_START, (pu1Byte)&next_head);
			} else
#endif
#else
#ifdef HIGH_POWER_EXT_LNA
			if ( priv->pshare->rf_ft_var.use_ext_lna) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTLNA_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_EXTLNA_START, (pu1Byte)&next_head);
			} else
#endif
#endif
			{
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_SIZE, (pu1Byte)&read_bytes);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_START, (pu1Byte)&next_head);
			}

			max_len = PHY_REG_SIZE;
		}
#endif //CONFIG_WLAN_HAL        

	}
#endif // CONFIG_RTL_92D_SUPPORT
#ifdef MP_TEST
	else if (reg_file == PHYREG_MP) {
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
			//printk("[%s][PHY_REG_MP_n]\n",__FUNCTION__);
			next_head = data_PHY_REG_MP_n_start;
			read_bytes = (int)(data_PHY_REG_MP_n_end - data_PHY_REG_MP_n_start);
			max_len = PHY_REG_SIZE;
		}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
			next_head = data_PHY_REG_MP_n_92C_start;
			read_bytes = (int)(data_PHY_REG_MP_n_92C_end - data_PHY_REG_MP_n_92C_start);
			max_len = PHY_REG_SIZE;
		}
#endif


#ifdef CONFIG_RTL_8812_SUPPORT //8812 phy mp
		if (GET_CHIP_VER(priv) == VERSION_8812E) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
			printk("[%s][PHY_REG_MP_8812]\n", __FUNCTION__);
			next_head = data_PHY_REG_MP_8812_start;
			read_bytes = (int)(data_PHY_REG_MP_8812_end - data_PHY_REG_MP_8812_start);
			max_len = PHY_REG_SIZE;
		}
#endif

#ifdef CONFIG_WLAN_HAL
		if ( IS_HAL_CHIP(priv) ) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_mp_buf;
			printk("[%s][PHY_REG_MP_HAL]\n", __FUNCTION__);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_MP_SIZE, (pu1Byte)&read_bytes);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_MP_START, (pu1Byte)&next_head);
			max_len = PHY_REG_SIZE;
		}
#endif //CONFIG_WLAN_HAL 

	}
#endif
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL)
	else if (reg_file == PHYREG_1T1R) { // PATH A
		phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
#ifdef TESTCHIP_SUPPORT
			if ( IS_TEST_CHIP(priv) ) {
				next_head = data_PHY_REG_1T_start;
				read_bytes = (int)(data_PHY_REG_1T_end - data_PHY_REG_1T_start);
			} else
#endif
			{
#if defined(HIGH_POWER_EXT_PA) || defined(HIGH_POWER_EXT_LNA)
				if (
#ifdef HIGH_POWER_EXT_PA
					priv->pshare->rf_ft_var.use_ext_pa
#else
					priv->pshare->rf_ft_var.use_ext_lna
#endif
				   ) {
					//printk("[%s][PHY_REG_1T_n_hp]\n", __FUNCTION__);
					next_head = data_PHY_REG_1T_n_hp_start;
					read_bytes = (int)(data_PHY_REG_1T_n_hp_end - data_PHY_REG_1T_n_hp_start);
				} else 
#endif
				{
					//printk("[%s][PHY_REG_1T_n]\n", __FUNCTION__);
					next_head = data_PHY_REG_1T_n_start;
					read_bytes = (int)(data_PHY_REG_1T_n_end - data_PHY_REG_1T_n_start);
				}
			}
		}
#endif


#ifdef CONFIG_WLAN_HAL
		if ( IS_HAL_CHIP(priv) ) {
			printk("[%s][PHY_REG_1T_HAL]\n", __FUNCTION__);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_1T_SIZE, (pu1Byte)&read_bytes);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_1T_START, (pu1Byte)&next_head);
		}
#endif //CONFIG_WLAN_HAL

		max_len = PHY_REG_SIZE;
#if 0
		if (priv->pshare->rf_ft_var.pathB_1T == 0) {
			next_head = __PHY_to1T1R_start;
			read_bytes = (int)(__PHY_to1T1R_end - __PHY_to1T1R_start);
		} else { // PATH B
			next_head = __PHY_to1T1R_b_start;
			read_bytes = (int)(__PHY_to1T1R_b_end - __PHY_to1T1R_b_start);
		}
#endif
	}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
	else if (reg_file == PHYREG_2T2R) {
		printk("[%s][PHY_REG_2T]\n",__FUNCTION__);
#ifdef TESTCHIP_SUPPORT
		if (IS_TEST_CHIP(priv)) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
			next_head = data_PHY_REG_2T_start;
			read_bytes = (int)(data_PHY_REG_2T_end - data_PHY_REG_2T_start);
		} else
#endif
		{
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
#ifdef HIGH_POWER_EXT_PA
			if (priv->pshare->rf_ft_var.use_ext_pa) 
			{
				panic_printk("[%s][PHY_REG_2T_n_hp]\n", __FUNCTION__);
				next_head = data_PHY_REG_2T_n_hp_start;
				read_bytes = (int)(data_PHY_REG_2T_n_hp_end - data_PHY_REG_2T_n_hp_start);
			} else
#endif
#ifdef HIGH_POWER_EXT_LNA
			if (priv->pshare->rf_ft_var.use_ext_lna)
			{
				panic_printk("[%s][PHY_REG_2T_n_lna]\n", __FUNCTION__);
				next_head = data_PHY_REG_2T_n_lna_start;
				read_bytes = (int)(data_PHY_REG_2T_n_lna_end - data_PHY_REG_2T_n_lna_start);
			} else 
#endif
			{
				//printk("[%s][PHY_REG_2T_n]\n", __FUNCTION__);
				next_head = data_PHY_REG_2T_n_start;
				read_bytes = (int)(data_PHY_REG_2T_n_end - data_PHY_REG_2T_n_start);
			}
		}
		max_len = PHY_REG_SIZE;
	}
#endif

	{
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)		
		if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8881A  || GET_CHIP_VER(priv)==VERSION_8814A  || GET_CHIP_VER(priv)==VERSION_8822B) {
			if (pg_tbl_idx == 0) { //4 2G
				regstart = 0xc20;
				if(GET_CHIP_VER(priv) == VERSION_8814A || GET_CHIP_VER(priv)==VERSION_8822B)
					regend = 0x1ae8;
				else
					regend = 0xe38;
			} else {
				regstart = 0xc24;
				if(GET_CHIP_VER(priv) == VERSION_8814A || GET_CHIP_VER(priv)==VERSION_8822B)
					regend = 0x1ae8;
				else
					regend = 0xe4c;
			}
		} else {
			regstart = 0xe00;
			regend = 0x868;
		}		
		//printk("regstart=%x\nregend=%x\n",regstart,regend);
#endif		
#if 0
		if ((file_format == FIVE_COLUMN) || (file_format == SEVEN_COLUMN))
		{

			if ((mem_ptr2 = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
				printk("PHY_ConfigBBWithParaFile(): not enough memory\n");
				return -1;
			}

			memset(mem_ptr2, 0, MAX_CONFIG_FILE_SIZE); // clear memory
			memcpy(mem_ptr2, next_head, read_bytes);

			next_head2 = mem_ptr2;
			while(1) {
				line_head = next_head2;
				next_head2 = get_line(&line_head);
						
				if (line_head == NULL)
					break;
				if (line_head[0] == '/')
					continue;				
				get_target_val_new(priv, line_head, &u4bRegOffset, &u4bRegMask ,&u4bRegValue);
			}
			
			kfree(mem_ptr2);
		}
#endif
		if((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigBBWithParaFile(): not enough memory\n");
			return -1;
		}

		memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory
		memcpy(mem_ptr, next_head, read_bytes);

		next_head = mem_ptr;
		round = 0;
		while (1) {
			line_head = next_head;
			next_head = get_line(&line_head);
			if (line_head == NULL)
				break;
			if (line_head[0] == '/'){
				if(find_str(line_head, "Parameter")||find_str(line_head, "version")){ /* Record PHY parameter version */
					char *ch = line_head+2;
					while (1) {
						if (*ch == '\0')
							break;
						else {
							printk("%c", *ch);
							ch++;
						}
					}
					printk("\n");
				} 
				continue;
			}
			if (line_head[0] == '#')
				continue;
			
			if (file_format == TWO_COLUMN) {
				num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
				if (num > 0) {
					phyreg_table[len].offset = u4bRegOffset;
					phyreg_table[len].value = u4bRegValue;
					len++;

					if ((len & 0x7ff) == 0)
						watchdog_kick();

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)|| defined(CONFIG_WLAN_HAL) //_eric_8812 ?? u4bRegOffset
					if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))) {
						if (u4bRegOffset == 0xffff)
							break;
					} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
					{
						if (u4bRegOffset == 0xff)
							break;
					}
					if ((len * sizeof(struct PhyRegTable)) > max_len)
						break;
				}
			} else {
				if (reg_file == PHYREG_PG) {
					if (file_format == THREE_COLUMN){				
						num = get_offset_mask_val(line_head, &u4bRegOffset, &u4bRegMask , &u4bRegValue);
					}else{				
						num = get_offset_mask_val_new(priv, line_head, &u4bRegOffset, &u4bRegMask , &u4bRegValue);		
						//printk("reg=%x, mask=%x, power=%x\n",u4bRegOffset,u4bRegMask,u4bRegValue);					
						/* 2G parse first parts, 5G parse both parts but only use the last one*/
#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
						if((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B)){
							if (pg_tbl_idx == 0 && u4bRegOffset == regend) {							
								macreg_table[len].offset = u4bRegOffset;
								macreg_table[len].mask = u4bRegMask;
								macreg_table[len].value = u4bRegValue;
								len++;
								macreg_table[len].offset = 0Xffff;	

								break;
							}
						}
#endif						

					}	
				}			
		
				if (num > 0) {
					macreg_table[len].offset = u4bRegOffset;
					macreg_table[len].mask = u4bRegMask;
					macreg_table[len].value = u4bRegValue;
					len++;
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
					if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))) {
						if (u4bRegOffset == 0xffff)
							break;
					} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
					{
						if (u4bRegOffset == 0xff)
							break;
					}
					if ((len * sizeof(struct MacRegTable)) > max_len)
						break;

					if ((len & 0x7ff) == 0)
						watchdog_kick();
				}
			}

			if (++round > 10000) {
				panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
				break;
			}
		}

#if 0//defined(CONFIG_WLAN_HAL_8814AE)
	if(GET_CHIP_VER(priv) == VERSION_8814A){
		int i;
		for(i=0;i<4;i++)
			printk("priv->pshare->phw->CCKTxAgc_A[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_A[i]);
		for(i=0;i<8;i++)
			printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_A[i]);
		for(i=0;i<24;i++)
			printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
		for(i=0;i<30;i++)
			printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);
		for(i=0;i<4;i++)
			printk("priv->pshare->phw->CCKTxAgc_B[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_B[i]);
		for(i=0;i<8;i++)
			printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_B[i]);
		for(i=0;i<24;i++)
			printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);
		for(i=0;i<30;i++)
			printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);
		for(i=0;i<4;i++)
			printk("priv->pshare->phw->CCKTxAgc_C[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_C[i]);
		for(i=0;i<8;i++)
			printk("priv->pshare->phw->OFDMTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_C[i]);
		for(i=0;i<24;i++)
			printk("priv->pshare->phw->MCSTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_C[i]);
		for(i=0;i<30;i++)
			printk("priv->pshare->phw->VHTTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_C[i]);
		for(i=0;i<4;i++)
			printk("priv->pshare->phw->CCKTxAgc_D[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_D[i]);
		for(i=0;i<8;i++)
			printk("priv->pshare->phw->OFDMTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_D[i]);
		for(i=0;i<24;i++)
			printk("priv->pshare->phw->MCSTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_D[i]);
		for(i=0;i<30;i++)
			printk("priv->pshare->phw->VHTTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_D[i]);
	}
#endif
		kfree(mem_ptr);

		if ((len * sizeof(struct PhyRegTable)) > max_len) {
			printk("PHY REG table buffer not large enough!\n");
			printk("len=%d,sizeof(struct PhyRegTable)=%d,max_len=%u",len,sizeof(struct PhyRegTable),max_len);
			return -1;
		}
	}

	num = 0;
	round = 0;
	while (1) {
		if (file_format == THREE_COLUMN || file_format == FIVE_COLUMN || file_format == SEVEN_COLUMN) {
			u4bRegOffset = macreg_table[num].offset;
			u4bRegValue = macreg_table[num].value;
			u4bRegMask = macreg_table[num].mask;
		} else {
			u4bRegOffset = phyreg_table[num].offset;
			u4bRegValue = phyreg_table[num].value;
		}

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
		if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))) {
			if (u4bRegOffset == 0xffff)
				break;
		} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{
			if (u4bRegOffset == 0xff)
				break;
		}


#if defined(CONFIG_WLAN_HAL_8197F)
        if (GET_CHIP_VER(priv) == VERSION_8197F) {
    		if (reg_file == AGCTAB) 
    		{
    			odm_UpdateAgcBigJumpLmt_8197F(ODMPTR, u4bRegOffset, u4bRegValue);
    		}
        }
#endif        
        


		if (file_format == THREE_COLUMN || file_format == FIVE_COLUMN) {
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8723B_SUPPORT)
			if (reg_file == PHYREG_PG && (
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8723B_SUPPORT)
				(GET_CHIP_VER(priv) == VERSION_8192D)||(GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv)) || (GET_CHIP_VER(priv) == VERSION_8723B)
#endif
					)) {

					if (u4bRegOffset == regstart) {
						if (idx == pg_tbl_idx)
							write_en = 1;
						idx++;
					}
					if (write_en
#if defined(CONFIG_WLAN_HAL_8814AE)
						&& (GET_CHIP_VER(priv) != VERSION_8814A) /* 8814 needs not to set Tx AGC registers */
#endif
#if defined(CONFIG_WLAN_HAL_8197F)
						&& (GET_CHIP_VER(priv) != VERSION_8197F) /* 97F doesn't set power by rate to Tx AGC registers */
#endif
#if defined(CONFIG_WLAN_HAL_8822BE)
						&& (GET_CHIP_VER(priv) != VERSION_8822B) /* 8822 needs not to set Tx AGC registers */
#endif

						) { //3 5 column PG set regs
						//panic_printk("0x%05x	0x%08x\n", u4bRegOffset, u4bRegValue);
						PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
						if (u4bRegOffset == regend) {
							write_en = 0;
							break;
						}
					}
				
			} else
#endif
			{
				PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
				//panic_printk("0x%05x	0x%08x\n", u4bRegOffset, u4bRegValue);
			}
		} else
			PHY_SetBBReg(priv, u4bRegOffset, bMaskDWord, u4bRegValue);
#ifdef USE_OUT_SRC
		if (reg_file == AGCTAB &&  priv->pshare->_dmODM.print_agc) 
		{
			panic_printk("Reg %x [31:0]  =  %x \n", u4bRegOffset, u4bRegValue);
		}
#endif
		num++;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	return 0;
}


/*-----------------------------------------------------------------------------
 * Function:    PHY_ConfigRFWithParaFile()
 *
 * Overview:    This function read RF parameters from general file format, and do RF 3-wire
 *
 * Input:      	PADAPTER			Adapter
 *			ps1Byte 				pFileName
 *			RF92CD_RADIO_PATH_E	eRFPath
 *
 * Output:      NONE
 *
 * Return:      RT_STATUS_SUCCESS: configuration file exist
 *
 * Note:		Delay may be required for RF configuration
 *---------------------------------------------------------------------------*/
int PHY_ConfigRFWithParaFile(struct rtl8192cd_priv *priv,
							 unsigned char *start, int read_bytes,
							 RF92CD_RADIO_PATH_E eRFPath)
{
	int           num, round = 0, round1 = 0;
	unsigned int  u4bRegOffset, u4bRegValue;
	unsigned char *mem_ptr, *line_head, *next_head;

	if ((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
		printk("PHY_ConfigRFWithParaFile(): not enough memory\n");
		return -1;
	}

	memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory
	memcpy(mem_ptr, start, read_bytes);

	next_head = mem_ptr;
	while (1) {
		line_head = next_head;
		next_head = get_line(&line_head);
		if (line_head == NULL)
			break;

		if (line_head[0] == '/'){
			if(find_str(line_head, "Parameter") || find_str(line_head, "version")){ /* Record PHY parameter version */
				char *ch = line_head+2;
				while (1) {
					if (*ch == '\n')
						break;
					else {
						printk("%c", *ch);
						ch++;
					}
					if (++round1 > 10000) {
						panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
						break;
					}
				}
				printk("\n");
			} 
			continue;
		}

		num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
		if (num > 0) {
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) //_eric_8812 ?? rf paras ??
			if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))) {
				if (u4bRegOffset == 0xffff) {
					break;
				} else if ((u4bRegOffset == 0xfe) || (u4bRegOffset == 0xffe)) {
					delay_ms(50);	// Delay 50 ms. Only RF configuration require delay
				} else if (num == 2) {
					PHY_SetRFReg(priv, eRFPath, u4bRegOffset, bMask20Bits, u4bRegValue);
					delay_ms(1);
				}
			} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
			{
				if (u4bRegOffset == 0xff) {
					break;
				} else if (u4bRegOffset == 0xfe) {
					delay_ms(50);	// Delay 50 ms. Only RF configuration require delay
				} else if (num == 2) {
					PHY_SetRFReg(priv, eRFPath, u4bRegOffset, bMask20Bits, u4bRegValue);
					delay_ms(1);
				}
			}
		}

		watchdog_kick();

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	kfree(mem_ptr);

	return 0;
}

#if 1
int PHY_ConfigMACWithParaFile(struct rtl8192cd_priv *priv)
{
	int read_bytes, num, len = 0, round = 0, round1 = 0;
	unsigned int  u4bRegOffset, u4bRegValue;
	unsigned char *mem_ptr, *line_head, *next_head;
	struct PhyRegTable *reg_table = (struct PhyRegTable *)priv->pshare->mac_reg_buf;

	{
		if ((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigMACWithParaFile(): not enough memory\n");
			return -1;
		}

		memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory
#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
			u8 *pMACRegStart;
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MACREGFILE_SIZE, (pu1Byte)&read_bytes);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MACREGFILE_START, (pu1Byte)&pMACRegStart);
			memcpy(mem_ptr, pMACRegStart, read_bytes);
		}
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			printk("[%s][MACPHY_REG]\n", __FUNCTION__);
			read_bytes = (int)(data_MACPHY_REG_end - data_MACPHY_REG_start);
			memcpy(mem_ptr, data_MACPHY_REG_start, read_bytes);
		}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
			printk("[%s][MACPHY_REG_92C]\n", __FUNCTION__);
			read_bytes = (int)(data_MACPHY_REG_92C_end - data_MACPHY_REG_92C_start);
			memcpy(mem_ptr, data_MACPHY_REG_92C_start, read_bytes);
		}
#endif


#ifdef CONFIG_RTL_8812_SUPPORT //mac reg
		if (GET_CHIP_VER(priv) == VERSION_8812E) {
			if (IS_TEST_CHIP(priv)) {
				panic_printk("[MAC_REG_8812]\n");
			read_bytes = (int)(data_MAC_REG_8812_end - data_MAC_REG_8812_start);
			memcpy(mem_ptr, data_MAC_REG_8812_start, read_bytes);
			} else {
				panic_printk("[MAC_REG_8812_n]\n");
				read_bytes = (int)(data_MAC_REG_8812_n_end - data_MAC_REG_8812_n_start);
				memcpy(mem_ptr, data_MAC_REG_8812_n_start, read_bytes);
			}
		}
#endif
		next_head = mem_ptr;
		while (1) {
			line_head = next_head;
			next_head = get_line(&line_head);
			if (line_head == NULL)
				break;

			if (line_head[0] == '/'){
				if(find_str(line_head, "Parameter") || find_str(line_head, "version")){ /* Record PHY parameter version */
					char *ch = line_head+2;
					while (1) {
						if (*ch == '\n')
							break;
						else {
							printk("%c", *ch);
							ch++;
						}

						if (++round1 > 10000) {
							panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
							break;
						}
					}
					printk("\n");
				} 
				continue;
			}

			num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
			if (num > 0) {
				reg_table[len].offset = u4bRegOffset;
				reg_table[len].value = u4bRegValue;
				len++;
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
				if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))) {
					if (u4bRegOffset == 0xffff)
						break;
				} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
				{
					if (u4bRegOffset == 0xff)
						break;
				}
				if ((len * sizeof(struct MacRegTable)) > MAC_REG_SIZE)
					break;
			}

			if (++round > 10000) {
				panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
				break;
			}
		}
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
		if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv)))
			reg_table[len].offset = 0xffff;
		else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
			reg_table[len].offset = 0xff;

		kfree(mem_ptr);

		if ((len * sizeof(struct MacRegTable)) > MAC_REG_SIZE) {
			printk("MAC REG table buffer not large enough!\n");
			return -1;
		}
	}

	num = 0;
	round = 0;
	while (1) {
		u4bRegOffset = reg_table[num].offset;
		u4bRegValue = reg_table[num].value;

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
		if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))) {
			if (u4bRegOffset == 0xffff)
				break;
		} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{
			if (u4bRegOffset == 0xff)
				break;
		}
		
		RTL_W8(u4bRegOffset, u4bRegValue);
		num++;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

#if 0 //defined(CONFIG_RTL_8812_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
		if (priv->pmib->dot11nConfigEntry.dot11nSTBC && priv->pmib->dot11nConfigEntry.dot11nLDPC)
			RTL_W16(REG_RESP_SIFS_OFDM_8812, 0x0c0c);
	}
#endif

	return 0;
}
#endif

#ifdef UNIVERSAL_REPEATER
static struct rtl8192cd_priv *get_another_interface_priv(struct rtl8192cd_priv *priv)
{
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
		return GET_VXD_PRIV(priv);
	else if (IS_DRV_OPEN(GET_ROOT(priv)))
		return GET_ROOT(priv);
	else
		return NULL;
}


static int get_shortslot_for_another_interface(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_priv *p_priv;

	p_priv = get_another_interface_priv(priv);
	if (p_priv) {
		if (p_priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE)
			return (p_priv->pmib->dot11ErpInfo.shortSlot);
		else {
			if (p_priv->pmib->dot11OperationEntry.opmode & WIFI_ASOC_STATE)
				return (p_priv->pmib->dot11ErpInfo.shortSlot);
		}
	}
	return -1;
}
#endif // UNIVERSAL_REPEATER


void set_slot_time(struct rtl8192cd_priv *priv, int use_short)
{
#ifdef UNIVERSAL_REPEATER
	int is_short;
	is_short = get_shortslot_for_another_interface(priv);
	if (is_short != -1) { // not abtained
		use_short &= is_short;
	}
#endif

#if defined(CONFIG_PCI_HCI)
	if (use_short)
		RTL_W8(SLOT_TIME, 0x09);
	else
		RTL_W8(SLOT_TIME, 0x14);
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	notify_slot_time_change(priv, use_short);
#endif
}

void SwChnl(struct rtl8192cd_priv *priv, unsigned char channel, int offset)
{
	unsigned int val = channel, eRFPath, curMaxRFPath;
	unsigned int is_need_reload_txpwr=0;
	int org_val, tmp_val;
#ifdef CONFIG_P2P_RTK_SUPPORT/*cfg p2p cfg p2p*/
    if(priv->pshare->rtk_remain_on_channel){
        NDEBUG("deny rtk_remain_on_channel\n");
        return;
    }
    if(priv->pshare->deny_scan_myself){
        NDEBUG("deny deny_scan_myself\n");
        return;
    }
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8822BE)|| defined(CONFIG_WLAN_HAL_8881A)|| defined(CONFIG_WLAN_HAL_8814AE)
		if ((GET_CHIP_VER(priv) == VERSION_8812E)||(GET_CHIP_VER(priv) == VERSION_8822B)||(GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A)) {	
			if ((channel < 36) && (priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G)) {
				GDEBUG("[8822B] Error Channel = %d , Force to Ch149 !!\n", channel);
				priv->pmib->dot11RFEntry.dot11channel = val = channel = 149;			
			}
		}
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	if ((GET_CHIP_VER(priv) == VERSION_8812E)||(GET_CHIP_VER(priv) == VERSION_8881A)) {
			if((channel>14 && priv->pshare->current_pwr_pg_index==BAND_2G)||(channel<=14 && priv->pshare->current_pwr_pg_index==BAND_5G))
				is_need_reload_txpwr = 1;
#ifdef CLIENT_MODE
			if(channel > 14){
				priv->pmib->dot11RFEntry.ther = priv->pshare->ori_ther;
				priv->pmib->dot11RFEntry.xcap = priv->pshare->ori_xcap;		
			}else{
				priv->pmib->dot11RFEntry.ther = priv->pshare->ori_ther2;
				priv->pmib->dot11RFEntry.xcap = priv->pshare->ori_xcap2;
			}

			if (GET_CHIP_VER(priv) == VERSION_8881A) {
				if (priv->pmib->dot11RFEntry.xcap > 0 && priv->pmib->dot11RFEntry.xcap < 0x3F) {
					
					org_val = (RTL_R32(0x2C) >> 12) & 0x3F;
					if (org_val > priv->pmib->dot11RFEntry.xcap) {
						for (tmp_val=org_val; tmp_val>=priv->pmib->dot11RFEntry.xcap; tmp_val--) {
							PHY_SetBBReg(priv, 0x2C, BIT(12) | BIT(13) | BIT(14) | BIT(15) | BIT(16) | BIT(17), tmp_val & 0x3F);
							PHY_SetBBReg(priv, 0x2C, BIT(18) | BIT(19) | BIT(20) | BIT(21) | BIT(22) | BIT(23), tmp_val & 0x3F);
						}
					}else {
						for (tmp_val=org_val; tmp_val<=priv->pmib->dot11RFEntry.xcap; tmp_val++) {
							PHY_SetBBReg(priv, 0x2C, BIT(12) | BIT(13) | BIT(14) | BIT(15) | BIT(16) | BIT(17), tmp_val & 0x3F);
							PHY_SetBBReg(priv, 0x2C, BIT(18) | BIT(19) | BIT(20) | BIT(21) | BIT(22) | BIT(23), tmp_val & 0x3F);
						}
					}
				}
			}

			if (GET_CHIP_VER(priv) == VERSION_8812E) {
				if (priv->pmib->dot11RFEntry.xcap > 0 && priv->pmib->dot11RFEntry.xcap < 0x3F) {
	
					org_val = (RTL_R32(0x2C) >> 19) & 0x3F;
					if (org_val > priv->pmib->dot11RFEntry.xcap) {
						for (tmp_val=org_val; tmp_val>=priv->pmib->dot11RFEntry.xcap; tmp_val--) {
							PHY_SetBBReg(priv, 0x2C, BIT(19) | BIT(20) | BIT(21) | BIT(22) | BIT(23) | BIT(24), tmp_val & 0x3F);
							PHY_SetBBReg(priv, 0x2C, BIT(25) | BIT(26) | BIT(27) | BIT(28) | BIT(29) | BIT(30), tmp_val & 0x3F);
						}
					}else {
						for (tmp_val=org_val; tmp_val<=priv->pmib->dot11RFEntry.xcap; tmp_val++) {
							PHY_SetBBReg(priv, 0x2C, BIT(19) | BIT(20) | BIT(21) | BIT(22) | BIT(23) | BIT(24), tmp_val & 0x3F);
							PHY_SetBBReg(priv, 0x2C, BIT(25) | BIT(26) | BIT(27) | BIT(28) | BIT(29) | BIT(30), tmp_val & 0x3F);
						}
					}
				}

			}
#endif
	}
#endif

		if(priv->pshare->CurrentChannelBW != HT_CHANNEL_WIDTH_20)
	    if(((channel < 5) && (offset == HT_2NDCH_OFFSET_BELOW)) || 
	     ((channel > 9) && (channel <=13) && (offset == HT_2NDCH_OFFSET_ABOVE))){
		panic_printk("Error Channel = %d Offset = %d, Force to 20M BW \n", channel, offset);
		
		offset = HT_2NDCH_OFFSET_DONTCARE;
		priv->pshare->offset_2nd_chan =  HT_2NDCH_OFFSET_DONTCARE;

		SwBWMode(priv, HT_CHANNEL_WIDTH_20, HT_2NDCH_OFFSET_DONTCARE);
	}

#if defined(DFS) && !defined(RTK_NL80211)
#ifdef UNIVERSAL_REPEATER
	unsigned int no_beacon = 0;
	if(under_apmode_repeater(priv))
	{
		if(!priv->pmib->dot11DFSEntry.disable_DFS && is_DFS_channel(channel))
		{		
		    if(!priv->site_survey->hidden_ap_found){
                no_beacon = 1;
            }
		}
	}
#endif
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B){  
		priv->pshare->No_RF_Write = 0;
		UpdateBBRFVal8812(priv, channel);
	}
#endif

// TODO: 8814AE BB/RF
// TODO: after all BB/RF init ready, change (8881A || 8814AE) to (WLAN_HAL && 11AC_SUPPORT) ??
#if defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
	if ((GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)) {
		priv->pshare->No_RF_Write = 0;
		GET_HAL_INTERFACE(priv)->PHYUpdateBBRFValHandler(priv, channel, offset);
	}
#endif

#if defined(AC2G_256QAM)
	if(is_ac2g(priv)) //for 8812 & 8881a & 8194
	{
		RTL_W32(0x8c0, RTL_R32(0x8c0) & (~BIT(17)));	//enable tx vht rates
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
#ifdef MP_TEST
		if ( (priv->pshare->rf_ft_var.mp_specific == 0) || (priv->pshare->rf_ft_var.pwr_by_rate == 1) )
			reload_txpwr_pg(priv);
#endif
	}
#endif


#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		curMaxRFPath = RF92CD_PATH_B;
	else
#endif
		curMaxRFPath = RF92CD_PATH_MAX;

	if (channel > 14)
		priv->pshare->curr_band = BAND_5G;
	else
		priv->pshare->curr_band = BAND_2G;

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	if ((GET_CHIP_VER(priv) == VERSION_8812E)||(GET_CHIP_VER(priv) == VERSION_8881A))
		if(is_need_reload_txpwr)
			reload_txpwr_pg(priv);
#endif

//_TXPWR_REDEFINE ?? Working channel also apply to 5G ?? what if channel = 165 + 2 or 36 -2 ??
#if 0
#ifdef RTK_AC_SUPPORT //todo, find working channel for 80M
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_80) {
#ifdef AC2G_256QAM
		if(is_ac2g(priv))
			val = 7;		//in 2g band, only channel 7 can expand to 80M bw
		else
#endif
		if (channel <= 48)
			val = 42;
		else if (channel <= 64)
			val = 58;
		else if (channel <= 112)
			val = 106;
		else if (channel <= 128)
			val = 122;
		else if (channel <= 144)
			val = 138;
		else if (channel <= 161)
			val = 155;
		else if (channel <= 177)
			val = 171;
	} else
#endif
		if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40) {
			if (offset == 1)
				val -= 2;
			else
				val += 2;
		}
#else		
		val = get_center_channel(priv, channel, offset, 1);
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
	if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)
#endif
		if (priv->pshare->rf_ft_var.use_frq_2_3G)
			val += 14;

	for (eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++)	{

#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			priv->pshare->RegRF18[eRFPath] = RTL_SET_MASK(priv->pshare->RegRF18[eRFPath], 0xff, val, 0);
			//PHY_SetRFReg(priv, eRFPath, rRfChannel, 0xff, val);
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
				/*
				 *	Set Bit18 when channel >= 100, for 5G only
				 */
				if (val >= 100)
					priv->pshare->RegRF18[eRFPath] |= BIT(18);
				//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(18), 1);
				else
					priv->pshare->RegRF18[eRFPath] &= (~(BIT(18)));
				//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(18), 0);

				priv->pshare->RegRF18[eRFPath] |= BIT(16);
				//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(16), 1);
				priv->pshare->RegRF18[eRFPath] |= BIT(8);
				//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(8), 1);
				// CLOAD for RF paht_A/B (MP-chip)
				if (val < 149)
					PHY_SetRFReg(priv, eRFPath, 0xB, BIT(16) | BIT(15) | BIT(14), 0x7);
				else
					PHY_SetRFReg(priv, eRFPath, 0xB, BIT(16) | BIT(15) | BIT(14), 0x2);
			} else {
				priv->pshare->RegRF18[eRFPath] &= (~(BIT(18)));
				//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(18, 0);
				priv->pshare->RegRF18[eRFPath] &= (~(BIT(16)));
				//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(16), 0);
				priv->pshare->RegRF18[eRFPath] &= (~(BIT(8)));
				//PHY_SetRFReg(priv, eRFPath, rRfChannel, BIT(8), 0);
				// CLOAD for RF paht_A/B (MP-chip)
				PHY_SetRFReg(priv, eRFPath, 0xB, BIT(16) | BIT(15) | BIT(14), 0x7);
			}

			PHY_SetRFReg(priv, eRFPath, rRfChannel, bMask20Bits, priv->pshare->RegRF18[eRFPath]);
			//printk("%s(%d) RF 18 = 0x%05x[0x%05x]\n",__FUNCTION__,__LINE__,priv->pshare->RegRF18[eRFPath],
			//PHY_QueryRFReg(priv,eRFPath,rRfChannel,bMask20Bits,1));
#ifdef RX_GAIN_TRACK_92D
			priv->pshare->RegRF3C[eRFPath] = PHY_QueryRFReg(priv, eRFPath, 0x3C, bMask20Bits, 1);
#endif
		} else
#endif
		{
			PHY_SetRFReg(priv, eRFPath, rRfChannel, 0xff, val);
		}

	}

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		SetSYN_para(priv, val);
#ifdef SW_LCK_92D
		phy_ReloadLCKSetting(priv);
#endif
		SetIMR_n(priv, val);

		Update92DRFbyChannel(priv, val);

	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)
#endif
		if (priv->pshare->rf_ft_var.use_frq_2_3G)
			val -= 14;

		/*cfg p2p cfg p2p rm
		priv->pshare->working_channel = val;
		*/

#ifdef TXPWR_LMT
	{
		if (!priv->pshare->rf_ft_var.disable_txpwrlmt) {

			if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188E)) //92c_pwrlmt
			{
				//printk("reload_txpwr_pg\n");
				reload_txpwr_pg(priv);
			}

#ifdef TXPWR_LMT_NEWFILE
			if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8188E) || IS_HAL_CHIP(priv))
				find_pwr_limit_new(priv, channel, offset);
			else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
				find_pwr_limit(priv, channel, offset);

#ifdef BEAMFORMING_AUTO
			if(priv->pshare->rf_ft_var.txbf_pwrlmt == TXBF_TXPWRLMT_AUTO) 
				BEAMFORMING_TXPWRLMT_Auto(priv);
#endif //BEAMFORMING_AUTO
		}
	}
#endif


#if defined(DFS) && !defined(RTK_NL80211)
#ifdef UNIVERSAL_REPEATER
	if(under_apmode_repeater(priv))
	{
		if(no_beacon == 1)
			RTL_W8(TXPAUSE, RTL_R8(TXPAUSE) | STOP_BCN);
		else
			RTL_W8(TXPAUSE, RTL_R8(TXPAUSE) & ~STOP_BCN);
	}
#endif
#endif
/*cfg p2p cfg p2p*/
	priv->pshare->working_channel = val;
	priv->pshare->working_channel2 = channel;
	SetTxPowerLevel(priv, channel);
	
#if defined(CONFIG_WLAN_HAL_8814AE)
	/* eliminate the 5280MHz & 5600MHz & 5760MHzspur of 8814A */
	if(GET_CHIP_VER(priv) == VERSION_8814A && priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
		GET_HAL_INTERFACE(priv)->PHYSpurCalibration(priv);
	}else if(GET_CHIP_VER(priv) == VERSION_8814A && priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G) {
		if (priv->pshare->rf_ft_var.mp_specific == 0)
		phydm_spur_nbi_setting_8814a(ODMPTR);/* eliminate the 2440MHz & 2480MHz spur of 8814A */
	}
#endif	

#ifdef CONFIG_WLAN_HAL_8197F
    if(GET_CHIP_VER(priv) == VERSION_8197F && _GET_HAL_DATA(priv)->cutVersion == ODM_CUT_A) {
		set_lck_cv(priv, val);
    }
	
	if (GET_CHIP_VER(priv) == VERSION_8197F) {
		if (channel == 13) {
			PHY_SetBBReg(priv, 0xd18, BIT(27), 1);
			PHY_SetBBReg(priv, 0xd2C, BIT(28), 1);	
			if (PHY_QueryRFReg(priv,0,0x33,bMask20Bits,1) == 0x6000d)
				PHY_SetBBReg(priv, 0xd40, BIT(26), 0);
			else
				PHY_SetBBReg(priv, 0xd40, BIT(26), 1);
		} else {
			PHY_SetBBReg(priv, 0xd18, BIT(27), 0);
			PHY_SetBBReg(priv, 0xd2C, BIT(28), 0);
			PHY_SetBBReg(priv, 0xd40, BIT(26), 0);
		}
	}
#endif

#ifdef BT_COEXIST	
	if(GET_CHIP_VER(priv) == VERSION_8192E && priv->pshare->rf_ft_var.btc == 1){
		extern int bt_state;
		if(bt_state){ /* BT enable, set channel for BT*/
			unsigned char H2CCommand[3]={0};
			unsigned int center_ch;
			if(priv->pshare->rf_ft_var.bt_dump)
				panic_printk("[%s] BT alive\n",__FUNCTION__);

			H2CCommand[0] = 0x01;
			center_ch = get_center_channel(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan, 1);
			H2CCommand[1] = center_ch;							
			if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40){
				H2CCommand[2] = 0x30;
			} else {				
				H2CCommand[2] = 0x20;
			}
			FillH2CCmd88XX(priv, 0x66, 3, H2CCommand);
		}
	}
#endif

#if 0
		if(!(OPMODE & WIFI_SITE_MONITOR)&& !(priv->ss_req_ongoing)
#ifdef UNIVERSAL_REPEATER			
			 && !(OPMODE_VXD & WIFI_SITE_MONITOR) && !(GET_VXD_PRIV(priv)->ss_req_ongoing)
#endif			
		){
			/* 
panic_printk("     ++++ from SwChnl's ");  */
			PHY_IQCalibrate(priv);
		}
#endif 
		if(!(OPMODE & WIFI_SITE_MONITOR)
#ifdef UNIVERSAL_REPEATER			
			 && !(OPMODE_VXD & WIFI_SITE_MONITOR)
#endif			
		){
			PHY_IQCalibrate(priv);
		}

#if defined(CONFIG_WLAN_HAL_8197F) || defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT) || defined(CONFIG_WLAN_HAL_8814AE)	// CCK shaping filter
		if(GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8197F || GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B || (GET_CHIP_VER(priv) == VERSION_8814A && priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)){
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G
#ifdef HIGH_POWER_EXT_LNA		
			&& !(GET_CHIP_VER(priv) == VERSION_8188E && priv->pshare->rf_ft_var.use_ext_lna)
#endif		
			){			
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)	
					if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
#ifdef HIGH_POWER_EXT_PA
						if (priv->pshare->rf_ft_var.use_ext_pa)
							priv->pshare->mp_cck_swing_idx = 14;
						else
#endif					
							priv->pshare->mp_cck_swing_idx = 20;
					} else
#endif		
		
#if defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8197F) || defined(CONFIG_WLAN_HAL_8814AE)
					if (GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8197F || (GET_CHIP_VER(priv) == VERSION_8814A && priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)) {
#ifdef HIGH_POWER_EXT_PA
						if (priv->pshare->rf_ft_var.use_ext_pa)
							priv->pshare->mp_cck_swing_idx = 30;
						else
#endif					
							priv->pshare->mp_cck_swing_idx = 20;
					} else
#endif		
					{
						priv->pshare->mp_cck_swing_idx = 12;
					}			
				
				//printk("==> mp_ofdm_swing_idx=%d\n", priv->pshare->mp_ofdm_swing_idx);
				//printk("==> mp_cck_swing_idx=%d\n", priv->pshare->mp_cck_swing_idx);
				set_CCK_swing_index(priv, priv->pshare->mp_cck_swing_idx);
			}	
		}
#endif

#if defined(CONFIG_WLAN_HAL_8197F) 
		if((GET_CHIP_VER(priv) == VERSION_8197F) && (!(OPMODE & WIFI_SITE_MONITOR)))		{
			if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40)				{
				if(offset == HT_2NDCH_OFFSET_ABOVE)
					phydm_api_primary_cca(ODMPTR, 1);			
				else if(offset == HT_2NDCH_OFFSET_BELOW)		
					phydm_api_primary_cca(ODMPTR, 2);		
			}			
			else			
			{				
					phydm_api_primary_cca(ODMPTR, 0);			
			}		
		}
#endif

	return;
}

#ifdef MCR_WIRELESS_EXTEND
#ifdef CONFIG_WLAN_HAL_8814AE
int Switch_Antenna_8814(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int rx_ss=0, ant=0;
	unsigned char*val=NULL;	

	if (strlen(data)==0) {
		GDEBUG("Null data !!!\n");
		return 1;
	} else
		GDEBUG("data: %s\n", data);


	val = get_value_by_token((char *)data, "ss=");
	if (val) {
		rx_ss = _atoi(val,10);
		if (rx_ss < 1 || rx_ss > 4) {
			GDEBUG("not support ss=%d !\n", rx_ss);
			return 1;
		}
	} else {
		GDEBUG("no specific ss !\n");
		return 1;
	}

	val = get_value_by_token((char *)data, "path=");
	if (val) {
		if (rx_ss == 1) {
			if (priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_80) {
				PHY_SetBBReg(priv, 0x838, BIT0, 0x1);
				PHY_SetBBReg(priv, 0x82C, 0x00f00000, 0xb);
				PHY_SetBBReg(priv, 0x82C, 0x000f0000, 0xb);
				PHY_SetBBReg(priv, 0x838, 0x0f000000, 0x9);
				PHY_SetBBReg(priv, 0x838, 0x00f00000, 0x9);
				PHY_SetBBReg(priv, 0x838, 0x000f0000, 0x9);
				PHY_SetBBReg(priv, 0x840, 0x0000f000, 0x7);
			} else {
				PHY_SetBBReg(priv, 0x82c, BIT27|BIT26|BIT25|BIT24, 0x5);
				RTL_W8(0x830,0xa);
			}
			if (!strcmp(val,"a"))	
				ant = ANTENNA_A;
			else if (!strcmp(val,"b"))	
				ant = ANTENNA_B;
			else if (!strcmp(val,"c"))	
				ant = ANTENNA_C;
			else if (!strcmp(val,"d"))	
				ant = ANTENNA_D;
		}
		else if (rx_ss == 2) {
			if (priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_80) {
				PHY_SetBBReg(priv, 0x838, BIT0, 0x1);
				PHY_SetBBReg(priv, 0x82C, 0x00f00000, 0xa);
				PHY_SetBBReg(priv, 0x82C, 0x000f0000, 0x9);
				PHY_SetBBReg(priv, 0x838, 0x0f000000, 0x9);
				PHY_SetBBReg(priv, 0x838, 0x00f00000, 0x9);
				PHY_SetBBReg(priv, 0x838, 0x000f0000, 0x9);
				PHY_SetBBReg(priv, 0x840, 0x0000f000, 0x6);
			} else {			
				PHY_SetBBReg(priv, 0x82c, BIT27|BIT26|BIT25|BIT24, 0x5);
				RTL_W8(0x830,0xa);
			}
			if (!strcmp(val,"ab"))	
				ant = 0;	//ANTENNA_AB;
			else if (!strcmp(val,"bc"))	
				ant = ANTENNA_BC;
			else if (!strcmp(val,"cd"))	
				ant = 0;	//ANTENNA_CD;
		}
		else if (rx_ss == 3) {
			if (priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_80) {
				PHY_SetBBReg(priv, 0x838, BIT0, 0x1);
				PHY_SetBBReg(priv, 0x82C, 0x00f00000, 0xa);
				PHY_SetBBReg(priv, 0x82C, 0x000f0000, 0x8);
				PHY_SetBBReg(priv, 0x838, 0x0f000000, 0x7);
				PHY_SetBBReg(priv, 0x838, 0x00f00000, 0x7);
				PHY_SetBBReg(priv, 0x838, 0x000f0000, 0x7);
				PHY_SetBBReg(priv, 0x840, 0x0000f000, 0x6);
			} else {				
				PHY_SetBBReg(priv, 0x82c, BIT27|BIT26|BIT25|BIT24, 0x3);
				RTL_W8(0x830,0x8);
			}
			
			if (!strcmp(val,"abc"))	
				ant = 0;	//ANTENNA_ABC;
			else if (!strcmp(val,"bcd"))	
				ant = ANTENNA_BCD;
		}
		else if (rx_ss == 4) {
			if (priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_80) {
				PHY_SetBBReg(priv, 0x838, BIT0, 0x1);
				PHY_SetBBReg(priv, 0x82C, 0x00f00000, 0xa);
				PHY_SetBBReg(priv, 0x82C, 0x000f0000, 0x8);
				PHY_SetBBReg(priv, 0x838, 0x0f000000, 0x7);
				PHY_SetBBReg(priv, 0x838, 0x00f00000, 0x7);
				PHY_SetBBReg(priv, 0x838, 0x000f0000, 0x7);
				PHY_SetBBReg(priv, 0x840, 0x0000f000, 0x7);
			} else {				
				PHY_SetBBReg(priv, 0x82c, BIT27|BIT26|BIT25|BIT24, 0x3);
				RTL_W8(0x830,0x8);
			}
			if (!strcmp(val,"abcd"))	
				ant = ANTENNA_ABCD;
		}
		
		if (!ant) {
			GDEBUG("no support ant=%s ! (rx_ss=%d)\n", val, rx_ss);
			return 1;
		}
	} else {
		GDEBUG("no specific path !\n");
		return 1;
	}


	switch(ant)
	{
		case ANTENNA_A:
			//Tx
			PHY_SetBBReg(priv, 0x93c, 0xfff00000, 0x001);	// 0x93C[31:20]=12'b0000_0000_0001

			//Rx
			PHY_SetBBReg(priv, 0x808, 0xff, 0x11);
			break;
			
		case ANTENNA_B:
			//Tx
			PHY_SetBBReg(priv, 0x93c, 0xfff00000, 0x002);	// 0x93C[31:20]=12'b0000_0000_0001

			//Rx
			PHY_SetBBReg(priv, 0x808, 0xff, 0x22);
			break;	

		case ANTENNA_C:
			//Tx
			PHY_SetBBReg(priv, 0x93c, 0xfff00000, 0x004);	// 0x93C[31:20]=12'b0000_0000_0001

			//Rx
			PHY_SetBBReg(priv, 0x808, 0xff, 0x44);
			break;	

		case ANTENNA_D:
			//Tx
			PHY_SetBBReg(priv, 0x93c, 0xfff00000, 0x008);	// 0x93C[31:20]=12'b0000_0000_0001

			//Rx
			PHY_SetBBReg(priv, 0x808, 0xff, 0x88);			
			break;		

		case ANTENNA_BC:
			//Tx
			PHY_SetBBReg(priv, 0x93c, 0xfff00000, 0x002);	// 0x93C[31:20]=12'b0000_0000_0001

			//Rx
			PHY_SetBBReg(priv, 0x808, 0xff, 0x66);			
			break;	

		case ANTENNA_BCD:
			//Tx
			PHY_SetBBReg(priv, 0x93c, 0xfff00000, 0x002);	// 0x93C[31:20]=12'b0000_0000_0001
			
			//Rx
			PHY_SetBBReg(priv, 0x808, 0xff, 0xee);			
			break;	

		case ANTENNA_ABCD:
			//Tx
			PHY_SetBBReg(priv, 0x93c, 0xfff00000, 0x002);	// 0x93C[31:20]=12'b0000_0000_0001

			//Rx
			PHY_SetBBReg(priv, 0x808, 0xff, 0xff);			
			break;				
	}

		return 0;
}
#endif
#endif
#ifdef EXPERIMENTAL_WIRELESS_EXTEND
// switch 1 spatial stream path
//antPath: 01 for PathA,10 for PathB, 11for Path AB
void Switch_1SS_Antenna(struct rtl8192cd_priv *priv, unsigned int antPath )
{
	unsigned int dword = 0;
	if (get_rf_mimo_mode(priv) != MIMO_2T2R)
		return;

	if(GET_CHIP_VER(priv) == VERSION_8812E) {
		switch (antPath) {
		case 1:
			dword = RTL_R32(0x80C);
			if ((dword & 0xf000) == 0x1000)
				goto switch_1ss_end;
			dword &= 0xffff0fff;
			dword |= 0x1000; // Path A
			RTL_W32(0x80C, dword);
			break;
		case 2:
			dword = RTL_R32(0x80C);
			if ((dword & 0xf000) == 0x2000)
				goto switch_1ss_end;
			dword &= 0xffff0fff;
			dword |= 0x2000; // Path B
			RTL_W32(0x80C, dword);
			break;
		
		case 3:
			if (priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1) // use one ANT for 1ss
				goto switch_1ss_end;// do nothing
			dword = RTL_R32(0x80C);
			if ((dword & 0xf000) == 0x3000)
				goto switch_1ss_end;
			dword &= 0xffff0fff;
			dword |= 0x3000; // Path A, B
			RTL_W32(0x80C, dword);
			break;
		
		default:// do nothing
			break;
		}

	} else {

		switch (antPath) {
		case 1:
			dword = RTL_R32(0x90C);
			if ((dword & 0x0ff00000) == 0x01100000)
				goto switch_1ss_end;
			dword &= 0xf00fffff;
			dword |= 0x01100000; // Path A
			RTL_W32(0x90C, dword);
			break;
		case 2:
			dword = RTL_R32(0x90C);
			if ((dword & 0x0ff00000) == 0x02200000)
				goto switch_1ss_end;
			dword &= 0xf00fffff;
			dword |= 0x02200000;	// Path B
			RTL_W32(0x90C, dword);
			break;

		case 3:
			if (priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1) // use one ANT for 1ss
				goto switch_1ss_end;// do nothing
			dword = RTL_R32(0x90C);
			if ((dword & 0x0ff00000) == 0x03300000)
				goto switch_1ss_end;
			dword &= 0xf00fffff;
			dword |= 0x03300000; // Path A,B
			RTL_W32(0x90C, dword);
			break;

		default:// do nothing
			break;
		}
	}
switch_1ss_end:
	return;

}

// switch OFDM path
//antPath: 01 for PathA,10 for PathB, 11for Path AB
void Switch_OFDM_Antenna(struct rtl8192cd_priv *priv, unsigned int antPath )
{
	unsigned int dword = 0;
	if (get_rf_mimo_mode(priv) != MIMO_2T2R)
		return;
	
	if(GET_CHIP_VER(priv) == VERSION_8812E) {
		
		switch (antPath) {
			case 1:
				dword = RTL_R32(0x80C);
				if ((dword & 0xf00) == 0x100)
					goto switch_OFDM_end;
				dword &= 0xfffff0ff;
				dword |= 0x100; // Path A
				RTL_W32(0x80C, dword);
				break;
			case 2:
				dword = RTL_R32(0x80C);
				if ((dword & 0xf00) == 0x200)
					goto switch_OFDM_end;
				dword &= 0xfffff0ff;
				dword |= 0x200; // Path B
				RTL_W32(0x80C, dword);
				break;
		
			case 3:
				if (priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1) // use one ANT for 1ss
					goto switch_OFDM_end;// do nothing
				dword = RTL_R32(0x80C);
				if ((dword & 0xf00) == 0x300)
					goto switch_OFDM_end;
				dword &= 0xfffff0ff;
				dword |= 0x300; // Path A, B
				RTL_W32(0x80C, dword);
				break;
		
			default:// do nothing
				break;
			}

	} else {

		switch (antPath) {
			case 1:
				dword = RTL_R32(0x90C);
				if ((dword & 0x000000f0) == 0x00000010)
					goto switch_OFDM_end;
				dword &= 0xffffff0f;
				dword |= 0x00000010; // Path A
				RTL_W32(0x90C, dword);
				break;
			case 2:
				dword = RTL_R32(0x90C);
				if ((dword & 0x000000f0) == 0x00000020)
					goto switch_OFDM_end;
				dword &= 0xffffff0f;
				dword |= 0x00000020;	// Path B
				RTL_W32(0x90C, dword);
				break;

			case 3:
				if (priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1) // use one ANT for 1ss
					goto switch_OFDM_end;// do nothing
				dword = RTL_R32(0x90C);
				if ((dword & 0x000000f0) == 0x00000030)
					goto switch_OFDM_end;
				dword &= 0xffffff0f;
				dword |= 0x00000030; // Path A,B
				RTL_W32(0x90C, dword);
				break;

			default:// do nothing
				break;
		}
	}
switch_OFDM_end:
	return;

}



#endif

void enable_hw_LED(struct rtl8192cd_priv *priv, unsigned int led_type)
{

#if (defined(HW_ANT_SWITCH) || defined(SW_ANT_SWITCH))&&( defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
	int b23 = RTL_R32(LEDCFG) & BIT(23);
#endif

#ifdef  CONFIG_WLAN_HAL
	// TODO: we should check register then set
	if (IS_HAL_CHIP(priv))
		return;
	else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
	switch (led_type) {
	case LEDTYPE_HW_TX_RX:
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D)
			RTL_W32(LEDCFG, LED_TX_RX_EVENT_ON << LED1CM_SHIFT_92D | LED1DIS_92D);
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C))
			RTL_W32(LEDCFG, LED_RX_EVENT_ON << LED1CM_SHIFT | LED_TX_EVENT_ON << LED0CM_SHIFT);
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B)
			RTL_W32(LEDCFG, (RTL_R32(LEDCFG) & 0xffff00ff) | BIT(13) | (LED_TX_RX_EVENT_ON << LED1CM_SHIFT));
#endif
		break;
	case LEDTYPE_HW_LINKACT_INFRA:
		RTL_W32(LEDCFG, LED_TX_RX_EVENT_ON << LED0CM_SHIFT);
		if ((OPMODE & WIFI_AP_STATE) || (OPMODE & WIFI_STATION_STATE))
			RTL_W32(LEDCFG, RTL_R32(LEDCFG) & 0x0ff);
		else
			RTL_W32(LEDCFG, (RTL_R32(LEDCFG) & 0xfffff0ff) | LED1SV);
		break;
	default:
		break;
	}

#if (defined(HW_ANT_SWITCH) || defined(SW_ANT_SWITCH))&&( defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
	RTL_W32(LEDCFG, b23 | RTL_R32(LEDCFG));
#endif
	}
}


/**
* Function:	phy_InitBBRFRegisterDefinition
*
* OverView:	Initialize Register definition offset for Radio Path A/B/C/D
*
* Input:
*			PADAPTER		Adapter,
*
* Output:	None
* Return:		None
* Note:		The initialization value is constant and it should never be changes
*/
void phy_InitBBRFRegisterDefinition(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_hw *phw = GET_HW(priv);

	// RF Interface Sowrtware Control
	phw->PHYRegDef[RF92CD_PATH_A].rfintfs = rFPGA0_XAB_RFInterfaceSW; // 16 LSBs if read 32-bit from 0x870
	phw->PHYRegDef[RF92CD_PATH_B].rfintfs = rFPGA0_XAB_RFInterfaceSW; // 16 MSBs if read 32-bit from 0x870 (16-bit for 0x872)

	// RF Interface Readback Value
	phw->PHYRegDef[RF92CD_PATH_A].rfintfi = rFPGA0_XAB_RFInterfaceRB; // 16 LSBs if read 32-bit from 0x8E0
	phw->PHYRegDef[RF92CD_PATH_B].rfintfi = rFPGA0_XAB_RFInterfaceRB;// 16 MSBs if read 32-bit from 0x8E0 (16-bit for 0x8E2)

	// RF Interface Output (and Enable)
	phw->PHYRegDef[RF92CD_PATH_A].rfintfo = rFPGA0_XA_RFInterfaceOE; // 16 LSBs if read 32-bit from 0x860
	phw->PHYRegDef[RF92CD_PATH_B].rfintfo = rFPGA0_XB_RFInterfaceOE; // 16 LSBs if read 32-bit from 0x864

	// RF Interface (Output and)  Enable
	phw->PHYRegDef[RF92CD_PATH_A].rfintfe = rFPGA0_XA_RFInterfaceOE; // 16 MSBs if read 32-bit from 0x860 (16-bit for 0x862)
	phw->PHYRegDef[RF92CD_PATH_B].rfintfe = rFPGA0_XB_RFInterfaceOE; // 16 MSBs if read 32-bit from 0x864 (16-bit for 0x866)

	//Addr of LSSI. Wirte RF register by driver
	phw->PHYRegDef[RF92CD_PATH_A].rf3wireOffset = rFPGA0_XA_LSSIParameter; //LSSI Parameter
	phw->PHYRegDef[RF92CD_PATH_B].rf3wireOffset = rFPGA0_XB_LSSIParameter;

	// RF parameter
	phw->PHYRegDef[RF92CD_PATH_A].rfLSSI_Select = rFPGA0_XAB_RFParameter;  //BB Band Select
	phw->PHYRegDef[RF92CD_PATH_B].rfLSSI_Select = rFPGA0_XAB_RFParameter;

	// Tx AGC Gain Stage (same for all path. Should we remove this?)
	phw->PHYRegDef[RF92CD_PATH_A].rfTxGainStage = rFPGA0_TxGainStage; //Tx gain stage
	phw->PHYRegDef[RF92CD_PATH_B].rfTxGainStage = rFPGA0_TxGainStage; //Tx gain stage

	// Tranceiver A~D HSSI Parameter-1
	phw->PHYRegDef[RF92CD_PATH_A].rfHSSIPara1 = rFPGA0_XA_HSSIParameter1;  //wire control parameter1
	phw->PHYRegDef[RF92CD_PATH_B].rfHSSIPara1 = rFPGA0_XB_HSSIParameter1;  //wire control parameter1

	// Tranceiver A~D HSSI Parameter-2
	phw->PHYRegDef[RF92CD_PATH_A].rfHSSIPara2 = rFPGA0_XA_HSSIParameter2;  //wire control parameter2
	phw->PHYRegDef[RF92CD_PATH_B].rfHSSIPara2 = rFPGA0_XB_HSSIParameter2;  //wire control parameter2

	// RF switch Control
	phw->PHYRegDef[RF92CD_PATH_A].rfSwitchControl = rFPGA0_XAB_SwitchControl; //TR/Ant switch control
	phw->PHYRegDef[RF92CD_PATH_B].rfSwitchControl = rFPGA0_XAB_SwitchControl;

	// AGC control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfAGCControl1 = rOFDM0_XAAGCCore1;
	phw->PHYRegDef[RF92CD_PATH_B].rfAGCControl1 = rOFDM0_XBAGCCore1;

	// AGC control 2
	phw->PHYRegDef[RF92CD_PATH_A].rfAGCControl2 = rOFDM0_XAAGCCore2;
	phw->PHYRegDef[RF92CD_PATH_B].rfAGCControl2 = rOFDM0_XBAGCCore2;

	// RX AFE control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfRxIQImbalance = rOFDM0_XARxIQImbalance;
	phw->PHYRegDef[RF92CD_PATH_B].rfRxIQImbalance = rOFDM0_XBRxIQImbalance;

	// RX AFE control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfRxAFE = rOFDM0_XARxAFE;
	phw->PHYRegDef[RF92CD_PATH_B].rfRxAFE = rOFDM0_XBRxAFE;

	// Tx AFE control 1
	phw->PHYRegDef[RF92CD_PATH_A].rfTxIQImbalance = rOFDM0_XATxIQImbalance;
	phw->PHYRegDef[RF92CD_PATH_B].rfTxIQImbalance = rOFDM0_XBTxIQImbalance;

	// Tx AFE control 2
	phw->PHYRegDef[RF92CD_PATH_A].rfTxAFE = rOFDM0_XATxAFE;
	phw->PHYRegDef[RF92CD_PATH_B].rfTxAFE = rOFDM0_XBTxAFE;

	// Tranceiver LSSI Readback SI mode
	phw->PHYRegDef[RF92CD_PATH_A].rfLSSIReadBack = rFPGA0_XA_LSSIReadBack;
	phw->PHYRegDef[RF92CD_PATH_B].rfLSSIReadBack = rFPGA0_XB_LSSIReadBack;

	// Tranceiver LSSI Readback PI mode
	phw->PHYRegDef[RF92CD_PATH_A].rfLSSIReadBackPi = TransceiverA_HSPI_Readback;
	phw->PHYRegDef[RF92CD_PATH_B].rfLSSIReadBackPi = TransceiverB_HSPI_Readback;
}


void check_chipID_MIMO(struct rtl8192cd_priv *priv)
{
#ifdef  CONFIG_WLAN_HAL
	if (GET_CHIP_VER(priv) == VERSION_8881A)	{
		priv->pshare->version_id |= (RTL_R16(0xf0)>>8) & 0x0f0;		// save b[15:12]		
		goto exit_func;
	}else if (GET_CHIP_VER(priv) == VERSION_8192E)	{
		priv->pshare->version_id |= (RTL_R16(0xf0)>>8) & 0x0f0;		// 92E c-cut	
		goto exit_func;
	}else if (GET_CHIP_VER(priv) == VERSION_8814A)	{
		u8 ChipCut      = _GET_HAL_DATA(priv)->cutVersion & 0xF;
        u8 bTestChip    = _GET_HAL_DATA(priv)->bTestChip;

        priv->pshare->version_id |= (ChipCut<<4);
        if (bTestChip) {
            priv->pshare->version_id |= 0x100;
        }

        goto exit_func;
	}
#endif
#ifdef CONFIG_RTL_8812_SUPPORT //FOR_8812_MP_CHIP
	if (GET_CHIP_VER(priv) == VERSION_8812E)	{
		unsigned int val32;
		val32 = RTL_R32(SYS_CFG);
		if (val32 & BIT(23)) {
			DEBUG_INFO("8812 test chip !! \n");
			priv->pshare->version_id |= 0x100; //is 8812 test chip
		} else {
			DEBUG_INFO("8812 mp chip !! \n");
			if (((val32 & 0xf000) >> 12) == 1) {	// [15:12] :1 --> C-cut
				priv->pshare->version_id |= 0x10;
			}
		}
		priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
		goto exit_func;
	}
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B)	{
		priv->pshare->phw->MIMO_TR_hw_support = MIMO_1T1R;
		goto exit_func;
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
			priv->pshare->phw->MIMO_TR_hw_support = MIMO_1T1R;
		else
			priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
		goto exit_func;
	}
#endif

#ifdef CONFIG_RTL_92C_SUPPORT
	{
		unsigned int val32;
		val32 = RTL_R32(SYS_CFG);
		if (val32 & BIT(27)) {
			priv->pshare->version_id = VERSION_8192C;
			priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
		} else {
			priv->pshare->version_id = VERSION_8188C;
			priv->pshare->phw->MIMO_TR_hw_support = MIMO_1T1R;

			if ((0x3 & (RTL_R32(0xec) >> 22)) == 0x01)
				priv->pshare->version_id |= 0x200;		// 88RE
		}

		if (val32 & BIT(23)) {
			priv->pshare->version_id |= 0x100;
		}
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
			if (val32 & BIT(19)) {
				priv->pshare->version_id |= 0x400;			// UMC
				priv->pshare->version_id |= (0xf0 & (val32 >> 8));	//	0:  a-cut
			}
			if (((0x0f & (val32 >> 16)) == 0) && ((0x0f & (val32 >> 12)) == 1)) {		//6195B
				priv->pshare->version_id |= 0x400;
				priv->pshare->version_id |= 0x10;					//	0x10:	b-cut
			}
		}
	}

#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8723B_SUPPORT)
exit_func:
#endif
	return;
}

void SelectTXPowerOffset(struct rtl8192cd_priv *priv)
{
	
	unsigned char isMinus11db = 0;

	
#if 0//defined(CONFIG_WLAN_HAL_8814AE)
	if(GET_CHIP_VER(priv) == VERSION_8814A && 
		(priv->pmib->dot11RFEntry.rfe_type == 3 || priv->pmib->dot11RFEntry.rfe_type == 5 )) // hp board
		isMinus11db = 1;
#endif
#if defined(CONFIG_WLAN_HAL_8192EE) && defined(HIGH_POWER_EXT_PA)
	if(GET_CHIP_VER(priv)==VERSION_8192E && priv->pshare->rf_ft_var.use_ext_pa)
		isMinus11db = 1;
#endif

	if (isMinus11db && priv->pshare->rf_ft_var.min_pwr_idex >= 22) {
		priv->pshare->phw->TXPowerOffset = 3; // -11 dB
	} else if(priv->pshare->rf_ft_var.min_pwr_idex >= 14){
		priv->pshare->phw->TXPowerOffset = 2; // -7 dB
	} else if(priv->pshare->rf_ft_var.min_pwr_idex >= 6){
		priv->pshare->phw->TXPowerOffset = 1; // -3 dB
	} else {
		priv->pshare->phw->TXPowerOffset = 0; // 0 dB
	}
}

void selectMinPowerIdex(struct rtl8192cd_priv *priv)
{
	int i = 0, idx, pwr_min = 0xff;
	unsigned int val32;
	unsigned char val;

#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8197F)
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8192C || GET_CHIP_VER(priv) == VERSION_8192D || GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8197F) {
		unsigned int pwr_regA[] = {0xe00, 0xe04, 0xe08, 0x86c, 0xe10, 0xe14, 0xe18, 0xe1c};
		unsigned int pwr_regB[] = {0x830, 0x834, 0x838, 0x86c, 0x83c, 0x848, 0x84c, 0x868}; // For 92C,92D

		for (idx = 0 ; idx < 8 ; idx++) {
			val32 = RTL_R32(pwr_regA[idx]);
			switch (pwr_regA[idx]) {
			case 0xe08:
				pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> 8) & 0xff);
				break;

			case 0x86c:
				for (i = 8 ; i < 32 ; i += 8)
					pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);
				break;

			default:
				for (i = 0 ; i < 32 ; i += 8)
					pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);
				break;
			}
		}

		if(get_rf_mimo_mode(priv) == MIMO_2T2R)
		{
			for (idx = 0 ; idx < 8 ; idx++) {
				val32 = RTL_R32(pwr_regB[idx]);
				switch (pwr_regB[idx]) {
				case 0x86c:
					pwr_min = POWER_MIN_CHECK(pwr_min, val32 & 0xff);
					break;

				case 0x838:
					for (i = 8 ; i < 32 ; i += 8)
						pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);
					break;

				default:
					for (i = 0 ; i < 32 ; i += 8)
						pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);
					break;
				}
			}
		}
	//#endif
	}	
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	if (GET_CHIP_VER(priv) == VERSION_8881A || GET_CHIP_VER(priv) == VERSION_8812E) {
		unsigned int pwr_regA[] = {0xc20,0xc24,0xc28,0xc2c,0xc30,0xc34,0xc38,0xc3c,0xc40,0xc44,0xc48,0xc4c}; //
		unsigned int pwr_regB[] = {0xe20,0xe24,0xe28,0xe2c,0xe30,0xe34,0xe38,0xe3c,0xe40,0xe44,0xe48,0xe4c}; // For 8812,92E

		// CCK
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G){
			val32 = RTL_R32(pwr_regA[0]);
			for (i = 0 ; i < 32 ; i += 8)
					pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);
			
			if(get_rf_mimo_mode(priv) == MIMO_2T2R){
				val32 = RTL_R32(pwr_regB[0]);
				for (i = 0 ; i < 32 ; i += 8)
					pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);
			}
		}

		// OFDM
		for (idx = 1 ; idx < 3 ; idx++) {
			val32 = RTL_R32(pwr_regA[idx]);
			for (i = 0 ; i < 32 ; i += 8)
					pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);
			
			if(get_rf_mimo_mode(priv) == MIMO_2T2R){				
				val32 = RTL_R32(pwr_regB[idx]);
				for (i = 0 ; i < 32 ; i += 8)
					pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);
			}	
		}
		// MCS 1SS rate
		for (idx = 3 ; idx < 5 ; idx++) {
			val32 = RTL_R32(pwr_regA[idx]);
			for (i = 0 ; i < 32 ; i += 8)
					pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);
			if(get_rf_mimo_mode(priv) == MIMO_2T2R) {
				val32 = RTL_R32(pwr_regB[idx]);
				for (i = 0 ; i < 32 ; i += 8)
					pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);		
			}
		}
		// MCS 2SS rate
		if(get_rf_mimo_mode(priv) == MIMO_2T2R){
			for (idx = 5 ; idx < 7 ; idx++) {
				val32 = RTL_R32(pwr_regA[idx]);
				for (i = 0 ; i < 32 ; i += 8)
						pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);
				val32 = RTL_R32(pwr_regB[idx]);
				for (i = 0 ; i < 32 ; i += 8)
					pwr_min = POWER_MIN_CHECK(pwr_min, (val32 >> i) & 0xff);		
			}
			
		}
		// VHT 1SS rate
		for (idx = 0 ; idx < 10 ; idx++) {
			val = RTL_R8(pwr_regA[7]+idx);
			pwr_min = POWER_MIN_CHECK(pwr_min, val);
			if(get_rf_mimo_mode(priv) == MIMO_2T2R) {
				val = RTL_R8(pwr_regB[7]+idx);
				pwr_min = POWER_MIN_CHECK(pwr_min, val);		
			}
		}
		// VHT 2SS rate
		if(get_rf_mimo_mode(priv) == MIMO_2T2R){
			for (idx = 0 ; idx < 10 ; idx++) {
				val = RTL_R8(pwr_regA[9]+2+idx);
				pwr_min = POWER_MIN_CHECK(pwr_min, val);
				
				val = RTL_R8(pwr_regB[9]+2+idx);
				pwr_min = POWER_MIN_CHECK(pwr_min, val);		
				
			}
			
		}
	}	
#endif

#if defined(CONFIG_WLAN_HAL_8822BE)
	if (GET_CHIP_VER(priv) == VERSION_8822B) {
		unsigned int path,i;		
		for(path=0;path<2;path++) {
			if(priv->pshare->phw->CurrentTxAgcCCK[path][3]!=0 && 
				priv->pshare->phw->CurrentTxAgcCCK[path][3] < pwr_min)
				pwr_min = priv->pshare->phw->CurrentTxAgcCCK[path][3];

			if(priv->pshare->phw->CurrentTxAgcOFDM[path][7]!=0 &&
				priv->pshare->phw->CurrentTxAgcOFDM[path][7] < pwr_min)
				pwr_min = priv->pshare->phw->CurrentTxAgcOFDM[path][7];

			if(priv->pshare->phw->CurrentTxAgcMCS[path][15]!=0 &&
				priv->pshare->phw->CurrentTxAgcMCS[path][15] < pwr_min)
				pwr_min = priv->pshare->phw->CurrentTxAgcMCS[path][15];

			if(priv->pshare->phw->CurrentTxAgcVHT[path][19]!=0 && 
				priv->pshare->phw->CurrentTxAgcVHT[path][19] < pwr_min)
				pwr_min = priv->pshare->phw->CurrentTxAgcVHT[path][19];
		}			
	}
#endif


#if defined(CONFIG_WLAN_HAL_8814AE)
	if (GET_CHIP_VER(priv) == VERSION_8814A) {
		unsigned int path,i;		
		for(path=0;path<4;path++) {
			if(priv->pshare->phw->CurrentTxAgcCCK[path][3]!=0 && 
				priv->pshare->phw->CurrentTxAgcCCK[path][3] < pwr_min)
				pwr_min = priv->pshare->phw->CurrentTxAgcCCK[path][3];

			if(priv->pshare->phw->CurrentTxAgcOFDM[path][7]!=0 &&
				priv->pshare->phw->CurrentTxAgcOFDM[path][7] < pwr_min)
				pwr_min = priv->pshare->phw->CurrentTxAgcOFDM[path][7];

			if(priv->pshare->phw->CurrentTxAgcMCS[path][23]!=0 &&
				priv->pshare->phw->CurrentTxAgcMCS[path][23] < pwr_min)
				pwr_min = priv->pshare->phw->CurrentTxAgcMCS[path][23];

			if(priv->pshare->phw->CurrentTxAgcVHT[path][29]!=0 && 
				priv->pshare->phw->CurrentTxAgcVHT[path][29] < pwr_min)
				pwr_min = priv->pshare->phw->CurrentTxAgcVHT[path][29];
		}			
	}
#endif

	priv->pshare->rf_ft_var.min_pwr_idex = pwr_min;
	SelectTXPowerOffset(priv);

}

#ifdef POWER_PERCENT_ADJUSTMENT
s1Byte PwrPercent2PwrLevel(int percentage)
{
#define ARRAYSIZE(x)	(sizeof(x)/sizeof((x)[0]))

	const int percent_threshold[] = {95, 85, 75, 67, 60, 54, 48, 43, 38, 34, 30, 27, 24, 22, 19, 17, 15, 14, 12, 11, 10};
	const s1Byte pwrlevel_diff[9] = { -40, -34, -30, -28, -26, -24, -23, -22, -21};	// for < 10% case
	int i;

	for (i = 0; i < ARRAYSIZE(percent_threshold); ++i) {
		if (percentage >= percent_threshold[i]) {
			return (s1Byte) - i;
		}
	}

	if (percentage < 1) percentage = 1;

	return pwrlevel_diff[percentage - 1];
}
#endif

void PHY_RF6052SetOFDMTxPower(struct rtl8192cd_priv *priv, unsigned int channel)
{
	unsigned int writeVal, defValue = 0x28 ;
	unsigned char  offset;
	unsigned char base, byte0, byte1, byte2, byte3;
	unsigned char pwrlevelHT40_1S_A = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[channel - 1];
	unsigned char pwrlevelHT40_1S_B = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[channel - 1];
	unsigned char pwrdiffHT40_2S = priv->pmib->dot11RFEntry.pwrdiffHT40_2S[channel - 1];
	unsigned char pwrdiffHT20 = priv->pmib->dot11RFEntry.pwrdiffHT20[channel - 1];
	unsigned char pwrdiffOFDM = priv->pmib->dot11RFEntry.pwrdiffOFDM[channel - 1];
#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE
	unsigned char pwrlevelHT40_6dBm_1S_A;
	unsigned char pwrlevelHT40_6dBm_1S_B;
	unsigned char pwrdiffHT40_6dBm_2S;
	unsigned char pwrdiffHT20_6dBm;
	unsigned char pwrdiffOFDM_6dBm;
	unsigned char offset_6dBm;
	s1Byte base_6dBm;
#endif
#ifdef POWER_PERCENT_ADJUSTMENT
	s1Byte pwrdiff_percent = PwrPercent2PwrLevel(priv->pmib->dot11RFEntry.power_percent);
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
	unsigned int ori_channel = channel; //Keep the original channel setting
#endif


	
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	/* for testchip only */
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
		defValue = 0x21;


#if defined(CALIBRATE_BY_ODM)
#ifdef MP_TEST
		if ((priv->pshare->rf_ft_var.mp_specific) && priv->pshare->mp_txpwr_patha)
			defValue = priv->pshare->mp_txpwr_patha;
#endif
		if (ODMPTR->RFCalibrateInfo.ThermalValue > GET_ROOT(priv)->pmib->dot11RFEntry.ther)
			defValue += (ODMPTR->RFCalibrateInfo.BbSwingIdxOfdm[RF_PATH_A] - ODMPTR->RFCalibrateInfo.DefaultOfdmIndex);
#endif
	}
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198B)
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
			defValue = 0x28;
		else
			defValue = 0x2d;
#else
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
			defValue = 0x26;
		else
			defValue = 0x30;
#endif

		//TXPWR_REDEFINE
		//FLASH GROUP [36-99] [100-148] [149-165]
		//Special Cases: [34-2, 34, 34+2,  36-2, 165+2]:No DATA , [149-2]:FLASH DATA OF Channel-146-6dBm
		//Use Flash data of channel 36 & 140 & 165 for these special cases.
		if ((channel > 30) && (channel < 36))
			channel = 36;
		else if (channel == (149 - 2))
			channel = 140;
		else if (channel > 165)
			channel = 165;

		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			pwrlevelHT40_1S_A = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[channel - 1];
			pwrlevelHT40_1S_B = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[channel - 1];
			pwrdiffHT40_2S = priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[channel - 1];
			pwrdiffHT20 = priv->pmib->dot11RFEntry.pwrdiff5GHT20[channel - 1];
			pwrdiffOFDM = priv->pmib->dot11RFEntry.pwrdiff5GOFDM[channel - 1];
		}

#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE
//MCS 8 - 15: No Power By Rate
//Others: Power by Rate (Add Power)
//Remove PWR_5G_DIFF

//?? phyBandSelect will auto swtich or 2G | 5G ??
		{
			int i;

			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
				pwrlevelHT40_6dBm_1S_A = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[channel];
				pwrlevelHT40_6dBm_1S_B = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[channel];
				pwrdiffHT40_6dBm_2S = priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[channel];
				pwrdiffHT20_6dBm = priv->pmib->dot11RFEntry.pwrdiff5GHT20[channel];
				pwrdiffOFDM_6dBm = priv->pmib->dot11RFEntry.pwrdiff5GOFDM[channel];
			} else {
				pwrlevelHT40_6dBm_1S_A = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[channel - 1];
				pwrlevelHT40_6dBm_1S_B = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[channel - 1];
				pwrdiffHT40_6dBm_2S = priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[channel - 1];
				pwrdiffHT20_6dBm = priv->pmib->dot11RFEntry.pwrdiff5GHT20[channel - 1];
				pwrdiffOFDM_6dBm = priv->pmib->dot11RFEntry.pwrdiff5GOFDM[channel - 1];
			}
		}
#endif


#ifdef CONFIG_RTL_92D_DMDP
		if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY &&
				priv->pshare->wlandev_idx == 1) {
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
				pwrlevelHT40_1S_A = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[channel - 1];
			else {
				pwrlevelHT40_1S_A = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[channel - 1];
			}
//_TXPWR_REDEFINE
#ifdef USB_POWER_SUPPORT
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
				pwrlevelHT40_6dBm_1S_A = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[channel];
			} else {
				pwrlevelHT40_6dBm_1S_A = priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[channel - 1];
			}
#endif
		}
#endif

		channel = ori_channel; //_TXPWR_REDEFINE Restore the channel setting

	}
#endif


#ifdef TXPWR_LMT
#ifdef TXPWR_LMT_88E
	if(GET_CHIP_VER(priv) == VERSION_8188E) {
		if(!priv->pshare->rf_ft_var.disable_txpwrlmt){
			int i;
			int max_idx_a, max_idx_b =0;

			if (!priv->pshare->txpwr_lmt_OFDM || !priv->pshare->tgpwr_OFDM_new[RF_PATH_A]) {
				//printk("No limit for OFDM TxPower\n");
				max_idx_a = 255;
				max_idx_b = 255;
			} else {
				// maximum additional power index
				max_idx_a = (priv->pshare->txpwr_lmt_OFDM - priv->pshare->tgpwr_OFDM_new[RF_PATH_A]);
				max_idx_b = (priv->pshare->txpwr_lmt_OFDM - priv->pshare->tgpwr_OFDM_new[RF_PATH_B]);
			}

			for (i = 0; i <= 7; i++) {
				//printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=0x%x max=0x%x\n",i, priv->pshare->phw->OFDMTxAgcOffset_A[i], max_idx_a);
				priv->pshare->phw->OFDMTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->OFDMTxAgcOffset_A[i], max_idx_a);
				priv->pshare->phw->OFDMTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->OFDMTxAgcOffset_B[i], max_idx_b);
				//printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->OFDMTxAgcOffset_B[i]);
			}

			if (!priv->pshare->txpwr_lmt_HT1S || !priv->pshare->tgpwr_HT1S_new[RF_PATH_A]) {
				//printk("No limit for HT1S TxPower\n");
				max_idx_a = 255;
				max_idx_b = 255;
			} else {
				// maximum additional power index
				max_idx_a = (priv->pshare->txpwr_lmt_HT1S - priv->pshare->tgpwr_HT1S_new[RF_PATH_A]);
				max_idx_b = (priv->pshare->txpwr_lmt_HT1S - priv->pshare->tgpwr_HT1S_new[RF_PATH_B]);
			}

			for (i = 0; i <= 7; i++) {
				//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=0x%x max=0x%x\n",i, priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx_a);
				priv->pshare->phw->MCSTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx_a);
				priv->pshare->phw->MCSTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_B[i], max_idx_b);
				//printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_B[i]);
			}

			if (!priv->pshare->txpwr_lmt_HT2S || !priv->pshare->tgpwr_HT2S_new[RF_PATH_A]) {
				//printk("No limit for HT2S TxPower\n");
				max_idx_a = 255;
				max_idx_b = 255;
			} else {
				// maximum additional power index
				max_idx_a = (priv->pshare->txpwr_lmt_HT2S - priv->pshare->tgpwr_HT2S_new[RF_PATH_A]);
				max_idx_b = (priv->pshare->txpwr_lmt_HT2S - priv->pshare->tgpwr_HT2S_new[RF_PATH_B]);
			}

			for (i = 8; i <= 15; i++) {
				//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x max=0x%x\n",i, priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx_a);
				priv->pshare->phw->MCSTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx_a);
				priv->pshare->phw->MCSTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_B[i], max_idx_b);
				//printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_B[i]);
			}
		}
	}
	else
#endif
	if ((GET_CHIP_VER(priv) == VERSION_8192D) || (GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C))
		if (!priv->pshare->rf_ft_var.disable_txpwrlmt) {
			int i;
			int max_idx;

			if (!priv->pshare->txpwr_lmt_OFDM || !priv->pshare->tgpwr_OFDM) {
				//printk("No limit for OFDM TxPower\n");
				max_idx = 255;
			} else {
				// maximum additional power index
				max_idx = (priv->pshare->txpwr_lmt_OFDM - priv->pshare->tgpwr_OFDM);
			}

			for (i = 0; i <= 7; i++) {
				priv->pshare->phw->OFDMTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->OFDMTxAgcOffset_A[i], max_idx);
				priv->pshare->phw->OFDMTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->OFDMTxAgcOffset_B[i], max_idx);
				//printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=%x\n",i, priv->pshare->phw->OFDMTxAgcOffset_A[i]);
				//printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->OFDMTxAgcOffset_B[i]);
			}

			if (!priv->pshare->txpwr_lmt_HT1S || !priv->pshare->tgpwr_HT1S) {
				//printk("No limit for HT1S TxPower\n");
				max_idx = 255;
			} else {
				// maximum additional power index
				max_idx = (priv->pshare->txpwr_lmt_HT1S - priv->pshare->tgpwr_HT1S);
			}

			for (i = 0; i <= 7; i++) {
				priv->pshare->phw->MCSTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx);
				priv->pshare->phw->MCSTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_B[i], max_idx);
				//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_A[i]);
				//printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_B[i]);
			}

			if (!priv->pshare->txpwr_lmt_HT2S || !priv->pshare->tgpwr_HT2S) {
				//printk("No limit for HT2S TxPower\n");
				max_idx = 255;
			} else {
				// maximum additional power index
				max_idx = (priv->pshare->txpwr_lmt_HT2S - priv->pshare->tgpwr_HT2S);
			}

			for (i = 8; i <= 15; i++) {
				priv->pshare->phw->MCSTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx);
				priv->pshare->phw->MCSTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_B[i], max_idx);
				//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_A[i]);
				//printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_B[i]);
			}
		}
#endif


	if ((pwrlevelHT40_1S_A == 0)
#if defined(MP_TEST) && (defined(CONFIG_RTL_88E_SUPPORT)|| defined(CONFIG_RTL_8723B_SUPPORT)) && defined(CALIBRATE_BY_ODM)
			|| ((priv->pshare->rf_ft_var.mp_specific) || (OPMODE & WIFI_MP_STATE))
#endif
	   )

	{
		// use default value

#ifdef HIGH_POWER_EXT_PA
		if(!(priv->pshare->rf_ft_var.mp_specific) && !(OPMODE & WIFI_MP_STATE))
			if (priv->pshare->rf_ft_var.use_ext_pa)
				defValue = HP_OFDM_POWER_DEFAULT ;
#endif
#ifndef ADD_TX_POWER_BY_CMD
		writeVal = (defValue << 24) | (defValue << 16) | (defValue << 8) | (defValue);
		RTL_W32(rTxAGC_A_Rate18_06, writeVal);
		RTL_W32(rTxAGC_A_Rate54_24, writeVal);
		RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);
		RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);
		RTL_W32(rTxAGC_B_Rate18_06, writeVal);
		RTL_W32(rTxAGC_B_Rate54_24, writeVal);
		RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);
		RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);

#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE, pwrlevelHT40_1S_A == 0 >> No 6dBm Power >> default value >> so USB = def - 14
		writeVal = POWER_RANGE_CHECK(defValue - USB_HT_2S_DIFF);
		writeVal |= (writeVal << 24) | (writeVal << 16) | (writeVal << 8);
#endif
		RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);
		RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);
		RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);
		RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);
#else
		base = defValue;
		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_ofdm_18);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_ofdm_12);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_ofdm_9);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_ofdm_6);

		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		RTL_W32(rTxAGC_A_Rate18_06, writeVal);
		RTL_W32(rTxAGC_B_Rate18_06, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_ofdm_54);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_ofdm_48);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_ofdm_36);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_ofdm_24);
		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		RTL_W32(rTxAGC_A_Rate54_24, writeVal);
		RTL_W32(rTxAGC_B_Rate54_24, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_3);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_2);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_1);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_0);
		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);
		RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);

		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_7);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_6);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_5);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_4);
		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);
		RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);

//_TXPWR_REDEFINE
#ifdef USB_POWER_SUPPORT
		byte0 = byte1 = byte2 = byte3 = -USB_HT_2S_DIFF;
#else
		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_11);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_10);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_9);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_8);
#endif

		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);
		RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);

//_TXPWR_REDEFINE
#ifdef USB_POWER_SUPPORT
		byte0 = byte1 = byte2 = byte3 = -USB_HT_2S_DIFF;
#else
		byte0 = byte1 = byte2 = byte3 = 0;
		ASSIGN_TX_POWER_OFFSET(byte0, priv->pshare->rf_ft_var.txPowerPlus_mcs_15);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_mcs_14);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_mcs_13);
		ASSIGN_TX_POWER_OFFSET(byte3, priv->pshare->rf_ft_var.txPowerPlus_mcs_12);
#endif

		byte0 = POWER_RANGE_CHECK(base + byte0);
		byte1 = POWER_RANGE_CHECK(base + byte1);
		byte2 = POWER_RANGE_CHECK(base + byte2);
		byte3 = POWER_RANGE_CHECK(base + byte3);
		writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
		RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);
		RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);

#endif // ADD_TX_POWER_BY_CMD
		return; // use default
	}

	/******************************  PATH A  ******************************/
	base = pwrlevelHT40_1S_A;
	offset = (pwrdiffOFDM & 0x0f);
#if defined(CONFIG_RTL_92D_SUPPORT)&& defined(CONFIG_RTL_92D_DMDP)
//_TXPWR_REDEFINE??
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1) {
		offset = ((pwrdiffOFDM & 0xf0) >> 4);
	}
#endif
	base = COUNT_SIGN_OFFSET(base, offset);
#ifdef POWER_PERCENT_ADJUSTMENT
	base = POWER_RANGE_CHECK(base + pwrdiff_percent);
#endif

#if (defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)) && defined(CALIBRATE_BY_ODM)
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
		if (ODMPTR->RFCalibrateInfo.ThermalValue > GET_ROOT(priv)->pmib->dot11RFEntry.ther)
			base += (ODMPTR->RFCalibrateInfo.BbSwingIdxOfdm[RF_PATH_A] - ODMPTR->RFCalibrateInfo.DefaultOfdmIndex);
	}
#endif

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[3]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_A_Rate18_06, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_A[7]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_A_Rate54_24, writeVal);

	base = pwrlevelHT40_1S_A;
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset = (pwrdiffHT20 & 0x0f);
#if defined(CONFIG_RTL_92D_SUPPORT)&& defined(CONFIG_RTL_92D_DMDP)
//_TXPWR_REDEFINE??
		if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1) {
			offset = ((pwrdiffHT20 & 0xf0) >> 4);
		}
#endif
		base = COUNT_SIGN_OFFSET(base, offset);
	}
#ifdef POWER_PERCENT_ADJUSTMENT
	base = POWER_RANGE_CHECK(base + pwrdiff_percent);
#endif

#if (defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)) && defined(CALIBRATE_BY_ODM)
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
		if (ODMPTR->RFCalibrateInfo.ThermalValue > GET_ROOT(priv)->pmib->dot11RFEntry.ther)
			base += (ODMPTR->RFCalibrateInfo.BbSwingIdxOfdm[RF_PATH_A] - ODMPTR->RFCalibrateInfo.DefaultOfdmIndex);
	}
#endif

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[3]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[7]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);

	offset = (pwrdiffHT40_2S & 0x0f);
#if defined(CONFIG_RTL_92D_SUPPORT)&& defined(CONFIG_RTL_92D_DMDP)
//_TXPWR_REDEFINE??
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1) {
		offset = ((pwrdiffHT40_2S & 0xf0) >> 4);
	}
#endif
	base = COUNT_SIGN_OFFSET(base, offset);

//_TXPWR_REDEFINE
#ifdef USB_POWER_SUPPORT

	base_6dBm = pwrlevelHT40_6dBm_1S_A;

	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset_6dBm = (pwrdiffHT20_6dBm & 0x0f);

#if defined(CONFIG_RTL_92D_SUPPORT)&& defined(CONFIG_RTL_92D_DMDP)
		if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1) {
			offset_6dBm = ((pwrdiffHT20_6dBm & 0xf0) >> 4);
		}
#endif

		base_6dBm = COUNT_SIGN_OFFSET(base_6dBm, offset_6dBm);
	}

	offset_6dBm = (pwrdiffHT40_6dBm_2S & 0x0f);

#if defined(CONFIG_RTL_92D_SUPPORT)&& defined(CONFIG_RTL_92D_DMDP)
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1) {
		offset_6dBm = ((pwrdiffHT40_6dBm_2S & 0xf0) >> 4);
	}
#endif

	base_6dBm = COUNT_SIGN_OFFSET(base_6dBm, offset_6dBm);

	if ((pwrlevelHT40_6dBm_1S_A != 0) && (pwrlevelHT40_6dBm_1S_A != pwrlevelHT40_1S_A))
		byte0 = byte1 = byte2 = byte3 =	base_6dBm;
	else if ((base - USB_HT_2S_DIFF) > 0)
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(base - USB_HT_2S_DIFF);
	else
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(defValue - USB_HT_2S_DIFF);

#else
//_TXPWR_REDEFINE ?? MCS 8 - 11, shall NOT add power by rate even NOT USB power ??
	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[8]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[9]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[10]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[11]);
#endif

	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;

	//DEBUG_INFO("debug e18:%x,%x,[%x,%x,%x,%x],%x\n", offset, base, byte0, byte1, byte2, byte3, writeVal);
	RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);

#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE
	if ((pwrlevelHT40_6dBm_1S_A != 0) && (pwrlevelHT40_6dBm_1S_A != pwrlevelHT40_1S_A))
		byte0 = byte1 = byte2 = byte3 =	base_6dBm;
	else if ((base - USB_HT_2S_DIFF) > 0)
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(base - USB_HT_2S_DIFF);
	else
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(defValue - USB_HT_2S_DIFF);

#else
	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[12]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[13]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[14]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_A[15]);
#endif

	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);

	/******************************  PATH B  ******************************/
	base = pwrlevelHT40_1S_B;
	offset = ((pwrdiffOFDM & 0xf0) >> 4);
	base = COUNT_SIGN_OFFSET(base, offset);
#ifdef POWER_PERCENT_ADJUSTMENT
	base = POWER_RANGE_CHECK(base + pwrdiff_percent);
#endif

#if (defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)) && defined(CALIBRATE_BY_ODM)
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
		if (ODMPTR->RFCalibrateInfo.ThermalValue > GET_ROOT(priv)->pmib->dot11RFEntry.ther)
			base += (ODMPTR->RFCalibrateInfo.BbSwingIdxOfdm[RF_PATH_A] - ODMPTR->RFCalibrateInfo.DefaultOfdmIndex);
	}
#endif

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[3]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_B_Rate18_06, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->OFDMTxAgcOffset_B[7]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_B_Rate54_24, writeVal);

	base = pwrlevelHT40_1S_B;
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset = ((pwrdiffHT20 & 0xf0) >> 4);
		base = COUNT_SIGN_OFFSET(base, offset);
	}
#ifdef POWER_PERCENT_ADJUSTMENT
	base = POWER_RANGE_CHECK(base + pwrdiff_percent);
#endif

#if (defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)) && defined(CALIBRATE_BY_ODM)
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
		if (ODMPTR->RFCalibrateInfo.ThermalValue > GET_ROOT(priv)->pmib->dot11RFEntry.ther)
			base += (ODMPTR->RFCalibrateInfo.BbSwingIdxOfdm[RF_PATH_A] - ODMPTR->RFCalibrateInfo.DefaultOfdmIndex);
	}
#endif

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[0]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[1]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[2]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[3]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);

	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[4]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[5]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[6]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[7]);
	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);

	offset = ((pwrdiffHT40_2S & 0xf0) >> 4);
	base = COUNT_SIGN_OFFSET(base, offset);

#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE ?? 2.4G
	base_6dBm = pwrlevelHT40_6dBm_1S_B;
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		offset_6dBm = ((pwrdiffHT20_6dBm & 0xf0) >> 4);
		base_6dBm = COUNT_SIGN_OFFSET(base_6dBm, offset_6dBm);
	}

	offset_6dBm = ((pwrdiffHT40_6dBm_2S & 0xf0) >> 4);
	base_6dBm = COUNT_SIGN_OFFSET(base_6dBm, offset_6dBm);

	if (( pwrlevelHT40_6dBm_1S_B != 0 ) && (pwrlevelHT40_6dBm_1S_B != pwrlevelHT40_1S_B))
		byte0 = byte1 = byte2 = byte3 = base_6dBm;
	else if ((base - USB_HT_2S_DIFF) > 0)
		byte0 = byte1 = byte2 = byte3 = POWER_RANGE_CHECK(base - USB_HT_2S_DIFF);
	else
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(defValue - USB_HT_2S_DIFF);

#else
	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[8]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[9]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[10]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[11]);
#endif

	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);

#ifdef USB_POWER_SUPPORT
//_TXPWR_REDEFINE ?? 2.4G
	if (( pwrlevelHT40_6dBm_1S_B != 0 ) && (pwrlevelHT40_6dBm_1S_B != pwrlevelHT40_1S_B))
		byte0 = byte1 = byte2 = byte3 = base_6dBm;
	else if ((base - USB_HT_2S_DIFF) > 0)
		byte0 = byte1 = byte2 = byte3 = POWER_RANGE_CHECK(base - USB_HT_2S_DIFF);
	else
		byte0 = byte1 = byte2 = byte3 =	POWER_RANGE_CHECK(defValue - USB_HT_2S_DIFF);

#else
	byte0 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[12]);
	byte1 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[13]);
	byte2 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[14]);
	byte3 = POWER_RANGE_CHECK(base + priv->pshare->phw->MCSTxAgcOffset_B[15]);
#endif

	writeVal = (byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3;
	RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);
}	/* PHY_RF6052SetOFDMTxPower */


void PHY_RF6052SetCCKTxPower(struct rtl8192cd_priv *priv, unsigned int channel)
{
	unsigned int writeVal = 0;
	u1Byte byte, byte1, byte2;
	u1Byte pwrlevelCCK_A = priv->pmib->dot11RFEntry.pwrlevelCCK_A[channel - 1];
	u1Byte pwrlevelCCK_B = priv->pmib->dot11RFEntry.pwrlevelCCK_B[channel - 1];
#ifdef POWER_PERCENT_ADJUSTMENT
	s1Byte pwrdiff_percent = PwrPercent2PwrLevel(priv->pmib->dot11RFEntry.power_percent);
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_92D_DMDP)
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY &&
				priv->pshare->wlandev_idx == 1) {
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)  {
				pwrlevelCCK_A = priv->pmib->dot11RFEntry.pwrlevelCCK_B[channel - 1];
			}
		}
	}
#endif
#ifdef TXPWR_LMT
	if (!priv->pshare->rf_ft_var.disable_txpwrlmt) {
#ifdef TXPWR_LMT_88E
		if(GET_CHIP_VER(priv) == VERSION_8188E) {
			int max_idx_a, max_idx_b, i;
			if (!priv->pshare->txpwr_lmt_CCK || 
				!priv->pshare->tgpwr_CCK_new[RF_PATH_A]) {
				DEBUG_INFO("No limit for CCK TxPower\n");
				max_idx_a = 255;
				max_idx_b = 255;
			} else {
				// maximum additional power index
				max_idx_a = (priv->pshare->txpwr_lmt_CCK - priv->pshare->tgpwr_CCK_new[RF_PATH_A]);
				max_idx_b = (priv->pshare->txpwr_lmt_CCK - priv->pshare->tgpwr_CCK_new[RF_PATH_B]);
			}
			for (i = 0; i <= 3; i++) {
				//printk("priv->pshare->phw->CCKTxAgc_A[%d]=0x%x max=0x%x\n",i, priv->pshare->phw->CCKTxAgc_A[i], max_idx_a);
				//printk("priv->pshare->phw->CCKTxAgc_B[%d]=%x\n",i, priv->pshare->phw->CCKTxAgc_B[i]);
				priv->pshare->phw->CCKTxAgc_A[i] = POWER_MIN_CHECK(priv->pshare->phw->CCKTxAgc_A[i], max_idx_a);
				priv->pshare->phw->CCKTxAgc_B[i] = POWER_MIN_CHECK(priv->pshare->phw->CCKTxAgc_B[i], max_idx_b);
			}
		}
		else
#endif
		{
		int max_idx, i;
		if (!priv->pshare->txpwr_lmt_CCK || !priv->pshare->tgpwr_CCK) {
			DEBUG_INFO("No limit for CCK TxPower\n");
			max_idx = 255;
		} else {
			// maximum additional power index
			max_idx = (priv->pshare->txpwr_lmt_CCK - priv->pshare->tgpwr_CCK);
		}

		for (i = 0; i <= 3; i++) {
			priv->pshare->phw->CCKTxAgc_A[i] = POWER_MIN_CHECK(priv->pshare->phw->CCKTxAgc_A[i], max_idx);
			priv->pshare->phw->CCKTxAgc_B[i] = POWER_MIN_CHECK(priv->pshare->phw->CCKTxAgc_B[i], max_idx);
			//printk("priv->pshare->phw->CCKTxAgc_A[%d]=%x\n",i, priv->pshare->phw->CCKTxAgc_A[i]);
			//printk("priv->pshare->phw->CCKTxAgc_B[%d]=%x\n",i, priv->pshare->phw->CCKTxAgc_B[i]);
		}
	}
	}
#endif

	if (priv->pshare->rf_ft_var.cck_pwr_max) {
		//byte = POWER_RANGE_CHECK(priv->pshare->rf_ft_var.cck_pwr_max);
		byte = (priv->pshare->rf_ft_var.cck_pwr_max > 0x3f) ? 0x3f : priv->pshare->rf_ft_var.cck_pwr_max;
		writeVal = byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);
		writeVal = (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
		writeVal = (byte << 24) | (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
		return;
	}

	if ((pwrlevelCCK_A == 0 && pwrlevelCCK_B == 0)
#if defined(MP_TEST) && defined(CONFIG_RTL_88E_SUPPORT) && defined(CALIBRATE_BY_ODM)
			|| ((priv->pshare->rf_ft_var.mp_specific) || (OPMODE & WIFI_MP_STATE))
#endif
	   ) {
		// use default value
#ifdef HIGH_POWER_EXT_PA
		if (priv->pshare->rf_ft_var.use_ext_pa)
			byte = HP_CCK_POWER_DEFAULT;
		else
#endif
			byte = 0x24;

#ifdef CONFIG_RTL_88E_SUPPORT
		/* for testchip only */
		if (GET_CHIP_VER(priv) == VERSION_8188E) {
			byte = 0x21;

#if defined(CALIBRATE_BY_ODM)
#ifdef MP_TEST
			if ((priv->pshare->rf_ft_var.mp_specific) && priv->pshare->mp_txpwr_patha)
				byte = priv->pshare->mp_txpwr_patha;
#endif
			if (ODMPTR->RFCalibrateInfo.ThermalValue > GET_ROOT(priv)->pmib->dot11RFEntry.ther)
				byte += (ODMPTR->RFCalibrateInfo.BbSwingIdxCck - ODMPTR->RFCalibrateInfo.DefaultCckIndex);
#endif
		}
#endif

#ifndef ADD_TX_POWER_BY_CMD
		writeVal = byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);
		writeVal = (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
		writeVal = (byte << 24) | (byte << 16) | (byte << 8) | byte;
		PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
#else
		pwrlevelCCK_A = pwrlevelCCK_B = byte;
		byte = 0;
		ASSIGN_TX_POWER_OFFSET(byte, priv->pshare->rf_ft_var.txPowerPlus_cck_1);
		writeVal = POWER_RANGE_CHECK(pwrlevelCCK_A + byte);
		PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);

		byte = byte1 = byte2 = 0;
		ASSIGN_TX_POWER_OFFSET(byte, priv->pshare->rf_ft_var.txPowerPlus_cck_1);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_cck_2);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_cck_5);
		byte  = POWER_RANGE_CHECK(pwrlevelCCK_B + byte);
		byte1 = POWER_RANGE_CHECK(pwrlevelCCK_B + byte1);
		byte2 = POWER_RANGE_CHECK(pwrlevelCCK_B + byte2);
		writeVal = ((byte2 << 16) | (byte1 << 8) | byte);
		PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);

		byte = byte1 = byte2 = 0;
		ASSIGN_TX_POWER_OFFSET(byte, priv->pshare->rf_ft_var.txPowerPlus_cck_2);
		ASSIGN_TX_POWER_OFFSET(byte1, priv->pshare->rf_ft_var.txPowerPlus_cck_5);
		ASSIGN_TX_POWER_OFFSET(byte2, priv->pshare->rf_ft_var.txPowerPlus_cck_11);
		byte  = POWER_RANGE_CHECK(pwrlevelCCK_A + byte);
		byte1 = POWER_RANGE_CHECK(pwrlevelCCK_A + byte1);
		byte2 = POWER_RANGE_CHECK(pwrlevelCCK_A + byte2);
		writeVal = ((byte2 << 24) | (byte1 << 16) | (byte << 8) | byte2);
		PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
#endif
		return; // use default
	}

	if ((get_rf_mimo_mode(priv) == MIMO_2T2R) && (pwrlevelCCK_B == 0)) {
		pwrlevelCCK_B = pwrlevelCCK_A +
						priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[channel - 1] - priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[channel - 1];
	}

#ifdef POWER_PERCENT_ADJUSTMENT
	pwrlevelCCK_A = POWER_RANGE_CHECK(pwrlevelCCK_A + pwrdiff_percent);
	pwrlevelCCK_B = POWER_RANGE_CHECK(pwrlevelCCK_B + pwrdiff_percent);
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(CALIBRATE_BY_ODM)
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		if (ODMPTR->RFCalibrateInfo.ThermalValue > GET_ROOT(priv)->pmib->dot11RFEntry.ther)
			pwrlevelCCK_A += (ODMPTR->RFCalibrateInfo.BbSwingIdxCck - ODMPTR->RFCalibrateInfo.DefaultCckIndex);
	}
#endif

	writeVal = POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[3] + priv->pmib->dot11RFEntry.add_cck1M_pwr);
	PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);
	writeVal = (POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[1]) << 16) |
			   (POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[2]) << 8)  |
	            POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[3] + priv->pmib->dot11RFEntry.add_cck1M_pwr);
	PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);
	writeVal = (POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[0]) << 24) |
			   (POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[1]) << 16) |
			   (POWER_RANGE_CHECK(pwrlevelCCK_A + priv->pshare->phw->CCKTxAgc_A[2]) << 8)  |
			   POWER_RANGE_CHECK(pwrlevelCCK_B + priv->pshare->phw->CCKTxAgc_B[0]);
	PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);
}


#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)

int PHY_CheckBBWithParaFile(struct rtl8192cd_priv *priv, int reg_file)
{
	int                read_bytes = 0, num, len = 0, round;
	unsigned int       u4bRegOffset, u4bRegValue, u4bRegMask;
	int 			   file_format = TWO_COLUMN;
	unsigned char      *mem_ptr, *line_head, *next_head = NULL;
	struct PhyRegTable *phyreg_table = NULL;
	struct MacRegTable *macreg_table = NULL;
	unsigned short     max_len = 0;
	unsigned int 		regstart, regend;

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	int				idx = 0, pg_tbl_idx = BGN_2040_ALL, write_en = 0;
#endif


	if (reg_file == PHYREG) {
#ifdef CONFIG_WLAN_HAL_8881A
		if (GET_CHIP_VER(priv) == VERSION_8881A) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_SIZE, (pu1Byte)&read_bytes);
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PHYREGFILE_START, (pu1Byte)&next_head);
			max_len = PHY_REG_SIZE;
		}
#endif //CONFIG_WLAN_HAL

#ifdef CONFIG_RTL_8812_SUPPORT //8812 phy
		if (GET_CHIP_VER(priv) == VERSION_8812E) {
			phyreg_table = (struct PhyRegTable *)priv->pshare->phy_reg_buf;
			max_len = PHY_REG_SIZE;

			if (IS_TEST_CHIP(priv)) { //for_8812_mp_chip
				printk("[%s][PHY_REG_8812]\n", __FUNCTION__);
				next_head = data_PHY_REG_8812_start;
				read_bytes = (int)(data_PHY_REG_8812_end - data_PHY_REG_8812_start);
			} else {
#ifdef HIGH_POWER_EXT_PA
#ifdef HIGH_POWER_EXT_LNA
				if (priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
					printk("[%s][PHY_REG_8812_n_hp]\n", __FUNCTION__);
					next_head = data_PHY_REG_8812_n_hp_start;
					read_bytes = (int)(data_PHY_REG_8812_n_hp_end - data_PHY_REG_8812_n_hp_start);
				} else if (priv->pshare->rf_ft_var.use_ext_pa && !priv->pshare->rf_ft_var.use_ext_lna) {
					printk("[%s][PHY_REG_8812_n_extpa]\n", __FUNCTION__);
					next_head = data_PHY_REG_8812_n_extpa_start;
					read_bytes = (int)(data_PHY_REG_8812_n_extpa_end - data_PHY_REG_8812_n_extpa_start);
				} else if (!priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
					printk("[%s][PHY_REG_8812_n_extlna]\n", __FUNCTION__);
					next_head = data_PHY_REG_8812_n_extlna_start;
					read_bytes = (int)(data_PHY_REG_8812_n_extlna_end - data_PHY_REG_8812_n_extlna_start);
				} else 
#else
				if (priv->pshare->rf_ft_var.use_ext_pa) {					
					printk("[%s][PHY_REG_8812_n_hp]\n", __FUNCTION__);
					next_head = data_PHY_REG_8812_n_hp_start;
					read_bytes = (int)(data_PHY_REG_8812_n_hp_end - data_PHY_REG_8812_n_hp_start);
				} else
#endif
#else
#ifdef HIGH_POWER_EXT_LNA
				if (priv->pshare->rf_ft_var.use_ext_lna) {
					printk("[%s][PHY_REG_8812_n_extlna]\n", __FUNCTION__);
					next_head = data_PHY_REG_8812_n_extlna_start;
					read_bytes = (int)(data_PHY_REG_8812_n_extlna_end - data_PHY_REG_8812_n_extlna_start);
				} else
#endif
#endif
				{
					printk("[%s][PHY_REG_8812_n_default]\n", __FUNCTION__);
					next_head = data_PHY_REG_8812_n_default_start;
					read_bytes = (int)(data_PHY_REG_8812_n_default_end - data_PHY_REG_8812_n_default_start);
				}
			}
		}
#endif
	}

	{
		if ((mem_ptr = (unsigned char *)kmalloc(MAX_CONFIG_FILE_SIZE, GFP_ATOMIC)) == NULL) {
			printk("PHY_ConfigBBWithParaFile(): not enough memory\n");
			return -1;
		}

		memset(mem_ptr, 0, MAX_CONFIG_FILE_SIZE); // clear memory

		memcpy(mem_ptr, next_head, read_bytes);

		next_head = mem_ptr;
		round = 0;
		while (1) {
			line_head = next_head;
			next_head = get_line(&line_head);
			if (line_head == NULL)
				break;

			if (line_head[0] == '/')
				continue;

			if (file_format == TWO_COLUMN) {
				num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
				if (num > 0) {
					phyreg_table[len].offset = u4bRegOffset;
					phyreg_table[len].value = u4bRegValue;
					len++;

					if ((len & 0x7ff) == 0)
						watchdog_kick();

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
					if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))) {
						if (u4bRegOffset == 0xffff)
							break;
					} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
					{
						if (u4bRegOffset == 0xff)
							break;
					}
					if ((len * sizeof(struct PhyRegTable)) > max_len)
						break;
				}
			} else {
				num = get_offset_mask_val(line_head, &u4bRegOffset, &u4bRegMask , &u4bRegValue);
				if (num > 0) {
					macreg_table[len].offset = u4bRegOffset;
					macreg_table[len].mask = u4bRegMask;
					macreg_table[len].value = u4bRegValue;
					len++;
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
					if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))) {
						if (u4bRegOffset == 0xffff)
							break;
					} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
					{
						if (u4bRegOffset == 0xff)
							break;
					}
					if ((len * sizeof(struct MacRegTable)) > max_len)
						break;

					if ((len & 0x7ff) == 0)
						watchdog_kick();
				}
			}

			if (++round > 10000) {
				panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
				break;
			}
		}

		kfree(mem_ptr);

		if ((len * sizeof(struct PhyRegTable)) > max_len) {

			printk("PHY REG table buffer not large enough!\n");

			return -1;
		}
	}

	num = 0;
	round = 0;
	while (1) {
		if (file_format == THREE_COLUMN) {
			u4bRegOffset = macreg_table[num].offset;
			u4bRegValue = macreg_table[num].value;
			u4bRegMask = macreg_table[num].mask;
		} else {
			u4bRegOffset = phyreg_table[num].offset;
			u4bRegValue = phyreg_table[num].value;
		}

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
		if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))) {
			if (u4bRegOffset == 0xffff)
				break;
		} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{
			if (u4bRegOffset == 0xff)
				break;
		}

		if (file_format == THREE_COLUMN) {
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
			if (reg_file == PHYREG_PG && (
#ifdef CONFIG_RTL_92D_SUPPORT
						(GET_CHIP_VER(priv) == VERSION_8192D)
#endif
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
#ifdef CONFIG_RTL_92D_SUPPORT
						||
#endif
						(GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (IS_HAL_CHIP(priv))
#endif
					)) {
				if (GET_CHIP_VER(priv) == VERSION_8812E) {
					if (pg_tbl_idx == 0) {
						regstart = 0xc20;
						regend = 0xe38;
					} else {
						regstart = 0xc24;
						regend = 0xe4c;
					}
					DEBUG_INFO("[%d]pg_tbl_idx=%d\n", __LINE__, pg_tbl_idx);
					if (u4bRegOffset == regstart) {
						if (idx == pg_tbl_idx) {
							write_en = 1;
						}
						idx++;
					}
					if (write_en) {
						PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
						if (u4bRegOffset == regend) {
							write_en = 0;
							break;
						}
					}
				} else {
					regstart = 0xe00;
					regend = 0x868;
					if (u4bRegOffset == regstart) {
						if (idx == pg_tbl_idx)
							write_en = 1;
						idx++;
					}
					if (write_en) {
						PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
						if (u4bRegOffset == regend) {
							write_en = 0;
							break;
						}
					}
				}
			} else
#endif
			{
				PHY_SetBBReg(priv, u4bRegOffset, u4bRegMask, u4bRegValue);
			}
		} else {
			unsigned int tmp = RTL_R32(u4bRegOffset);
			//PHY_SetBBReg(priv, u4bRegOffset, bMaskDWord, u4bRegValue);

			if (tmp != u4bRegValue) {
//				printk("[0x%x] 0x%08x 0x%08x \n", u4bRegOffset, u4bRegValue, tmp);
				PHY_SetBBReg(priv, u4bRegOffset, bMaskDWord, u4bRegValue);
			}
		}
		num++;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}

	return 0;
}

/*
static void phy_BB8192CD_Check_ParaFile(struct rtl8192cd_priv *priv)
{
	int rtStatus=0;
	unsigned short val16;
	unsigned int val32;

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A))
		rtStatus = PHY_CheckBBWithParaFile(priv, PHYREG);
#endif
}
*/
#endif

#ifdef CONFIG_WLAN_HAL
// TODO: Filen, comfirm register setting below
static int phy_BB88XX_Config_ParaFile(struct rtl8192cd_priv *priv)
{
	int rtStatus = 0;
	unsigned short val16;
	unsigned int val32;
	unsigned int i, j;
	
	phy_InitBBRFRegisterDefinition(priv);
	
#if defined(CONFIG_SDIO_HCI) && defined(HIGH_POWER_EXT_LNA)
	if (priv->pshare->rf_ft_var.use_ext_lna) {
		RTL_W8(0x11, 0xeb);	    
		RTL_W8(0x12, 0x07);
		RTL_W8(0x14, 0x75);
	}
#endif

	// Enable BB and RF
	val16 = RTL_R16(REG_SYS_FUNC_EN);
	RTL_W16(REG_SYS_FUNC_EN, val16 | BIT(13) | BIT(0) | BIT(1));


    /* temporary enable BIT_RF1_SDMRSTB/BIT_RF1_RSTB/BIT_RF1_EN of REG_AFE_CTRL4(0x78),
            need to be moved to MAC_REG.txt in the future (suggested by SD1 Eric.Liu)
            temporary enable BIT_POW_REGU_P0(2)/BIT_POW_REGU_P1(10) of REG_AFE_LDO_CTRL(0x20),
            need to be moved to MAC_REG.txt in the future (suggested by Yen-Cheng Hsu)
        */
    #if defined(CONFIG_WLAN_HAL_8197F)
	if (GET_CHIP_VER(priv) == VERSION_8197F)  {
				RTL_W8(REG_RF_CTRL, RTL_R8(REG_RF_CTRL) & ~(RF_EN | RF_RSTB | RF_SDMRSTB));
        RTL_W32(0x78, RTL_R32(0x78) & ~(BIT24 | BIT25 | BIT26));
        RTL_W32(0x78, RTL_R32(0x78) | BIT24 | BIT25 | BIT26);
        RTL_W32(0x20, RTL_R32(0x20) | BIT2 | BIT10);
    }
    #endif

    
	RTL_W8(REG_RF_CTRL, RF_EN | RF_RSTB | RF_SDMRSTB);

	/*----Check chip ID and hw TR MIMO config----*/
//	check_chipID_MIMO(priv);

// 8814 merge issue, why eric mark this flag in branch 3.4
#if 0//def CONFIG_WLAN_HAL
	if ( IS_HAL_CHIP(priv) ) {

#ifdef CONFIG_WLAN_HAL_8192EE
		if (GET_CHIP_VER(priv) == VERSION_8192E) {
#if defined(CONFIG_MACBBRF_BY_ODM)
			printk("%s(%d), HAL PHY_ConfigBBWithParaFile\n", __FUNCTION__, __LINE__);
			rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_PHY_REG);
#else
			printk("%s(%d), HAL PHY_ConfigBBWithParaFile\n", __FUNCTION__, __LINE__);
			rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG);
#endif
		}
#endif //CONFIG_WLAN_HAL_8192EE        

#if defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_WLAN_HAL_8814AE)
		if ((GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A)) {
			printk("%s(%d), HAL PHY_ConfigBBWithParaFile\n", __FUNCTION__, __LINE__);
			rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG);
		}
#endif //CONFIG_WLAN_HAL_8881A || CONFIG_WLAN_HAL_8814AE       
	}
#endif  //CONFIG_WLAN_HAL

	if(ODMPTR->ConfigBBRF)	
		rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_PHY_REG);
	else
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG);

#if defined(CONFIG_WLAN_HAL_8197F)
	if (GET_CHIP_VER(priv) == VERSION_8197F) {
		phy_path_a_dpk_init_8197f(ODMPTR);
		phy_path_b_dpk_init_8197f(ODMPTR);
	}
#endif

#ifdef CONFIG_RF_DPK_SETTING_SUPPORT
	ODMPTR->RFCalibrateInfo.bDPKEnable = 1; 
	
	if (priv->pmib->dot11RFDPKEntry.bDPPathAOK == 1 && priv->pmib->dot11RFDPKEntry.bDPPathBOK == 1) {
		ODMPTR->RFCalibrateInfo.bDPPathAOK = priv->pmib->dot11RFDPKEntry.bDPPathAOK;	
		ODMPTR->RFCalibrateInfo.bDPPathBOK = priv->pmib->dot11RFDPKEntry.bDPPathBOK;
		//printk("bDPPathAOK=%d   bDPPathBOK=%d\n", priv->pmib->dot11RFDPKEntry.bDPPathAOK, priv->pmib->dot11RFDPKEntry.bDPPathBOK);	
		for (i = 0; i < 3; i++) 	{	
			ODMPTR->RFCalibrateInfo.pwsf_2g_a[i] = priv->pmib->dot11RFDPKEntry.pwsf_2g_a[i];
			ODMPTR->RFCalibrateInfo.pwsf_2g_b[i] = priv->pmib->dot11RFDPKEntry.pwsf_2g_b[i];
			//printk("!!! pwsf_2g_a[%d]=0x%x   pwsf_2g_b[%d]=0x%x\n", i, priv->pmib->dot11RFDPKEntry.pwsf_2g_a[i], i, priv->pmib->dot11RFDPKEntry.pwsf_2g_b[i]);	
			for (j = 0; j < 64; j++)			
			{			
				ODMPTR->RFCalibrateInfo.lut_2g_even_a[i][j] = priv->pmib->dot11RFDPKEntry.lut_2g_even_a[i][j];	
				ODMPTR->RFCalibrateInfo.lut_2g_odd_a[i][j] = priv->pmib->dot11RFDPKEntry.lut_2g_odd_a[i][j];		
				ODMPTR->RFCalibrateInfo.lut_2g_even_b[i][j] = priv->pmib->dot11RFDPKEntry.lut_2g_even_b[i][j];	
				ODMPTR->RFCalibrateInfo.lut_2g_odd_b[i][j] = priv->pmib->dot11RFDPKEntry.lut_2g_odd_b[i][j];	
				//printk("!!! lut_2g_even_a[%d][%d]=0x%x   lut_2g_odd_a[%d][%d]=0x%x\n", i, j, priv->pmib->dot11RFDPKEntry.lut_2g_even_a[i][j], i, j, priv->pmib->dot11RFDPKEntry.lut_2g_odd_a[i][j]);		
				//printk("!!! lut_2g_even_b[%d][%d]=0x%x   lut_2g_odd_b[%d][%d]=0x%x\n", i, j, priv->pmib->dot11RFDPKEntry.lut_2g_even_b[i][j], i, j, priv->pmib->dot11RFDPKEntry.lut_2g_odd_b[i][j]);		
			}	
		}	
	}
#endif
#if 0			
	printk("bDPPathAOK=%d	bDPPathBOK=%d\n", priv->pmib->dot11RFDPKEntry.bDPPathAOK, priv->pmib->dot11RFDPKEntry.bDPPathBOK);		
	for (i = 0; i < 3; i++) 
	{		
		printk("!!! pwsf_2g_a[%d]=0x%x	 pwsf_2g_b[%d]=0x%x\n", i, ODMPTR->RFCalibrateInfo.pwsf_2g_a[i], i, ODMPTR->RFCalibrateInfo.pwsf_2g_b[i]);	
	}		
	for (i = 0; i < 3; i++) 	
	{			
		for (j = 0; j < 64; j++)			
		printk("!!! lut_2g_even_a[%2d][%2d]=0x%x\n", i, j, priv->pmib->dot11RFDPKEntry.lut_2g_even_a[i][j]);	
		for (j = 0; j < 64; j++)			
		printk("!!!  lut_2g_odd_a[%2d][%2d]=0x%x\n", i, j, priv->pmib->dot11RFDPKEntry.lut_2g_odd_a[i][j]); 	
		for (j = 0; j < 64; j++)			
		printk("!!! lut_2g_even_b[%2d][%2d]=0x%x\n", i, j, priv->pmib->dot11RFDPKEntry.lut_2g_even_b[i][j]);	
		for (j = 0; j < 64; j++)				
		printk("!!!  lut_2g_odd_b[%2d][%2d]=0x%x\n", i, j, priv->pmib->dot11RFDPKEntry.lut_2g_odd_b[i][j]); 	
	}
#endif

#if 1   //Filen, BB have no release these files
#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific) {
		delay_ms(10);
	if(ODMPTR->ConfigBBRF)
		rtStatus |= ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_PHY_REG_MP);
	else	
		rtStatus |= PHY_ConfigBBWithParaFile(priv, PHYREG_MP);
	}
#endif

	if (rtStatus) {
		printk("phy_BB88XX_Config_ParaFile(): PHYREG_MP Reg Fail!!\n");
		return rtStatus;
	}

	/*----If EEPROM or EFUSE autoload OK, We must config by PHY_REG_PG.txt----*/
	if (0)//(GET_CHIP_VER(priv) == VERSION_8822B) //eric-8822 ?? PG use txt ??
		rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_PHY_REG_PG);
	else
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_PG);
	
	if (rtStatus) {
		printk("phy_BB88XX_Config_ParaFile():BB_PG Reg Fail!!\n");
		return rtStatus;
	}
#endif
	/*----BB AGC table Initialization----*/
	if(ODMPTR->ConfigBBRF)
		rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_AGC_TAB);
	else
		rtStatus = PHY_ConfigBBWithParaFile(priv, AGCTAB);

	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile(): Write BB AGC Table Fail!!\n");
		return rtStatus;
	}

#ifdef CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E) {
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 1);
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 1);
	}
#endif //CONFIG_WLAN_HAL_8192EE     

	if(ODMPTR->ConfigBBRF)
		priv->pshare->PhyVersion = ODM_GetHWImgVersion(ODMPTR);
	DEBUG_INFO("PHY-BB Initialization Success\n");

	return 0;
}
#endif  //#ifdef CONFIG_WLAN_HAL

#if(CONFIG_WLAN_NOT_HAL_EXIST==1)
static int phy_BB8192CD_Config_ParaFile(struct rtl8192cd_priv *priv)
{
	int rtStatus = 0;
	unsigned short val16;
	unsigned int val32;

	phy_InitBBRFRegisterDefinition(priv);

	// Enable BB and RF
	val16 = RTL_R16(REG_SYS_FUNC_EN);
	RTL_W16(REG_SYS_FUNC_EN, val16 | BIT(13) | BIT(0) | BIT(1));

	// 20090923 Joseph: Advised by Steven and Jenyu. Power sequence before init RF.
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	if (
#ifdef CONFIG_RTL_92C_SUPPORT
		(GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		||
#endif
		(GET_CHIP_VER(priv) == VERSION_8192D)
#endif
	) {
		RTL_W8(REG_AFE_PLL_CTRL, 0x83);
		RTL_W8(REG_AFE_PLL_CTRL + 1, 0xdb);
	}
#endif
	RTL_W8(REG_RF_CTRL, RF_EN | RF_RSTB | RF_SDMRSTB);
#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E)
		RTL_W8(0x76, 0x7); //enable RF Path B
#endif

#if defined(CONFIG_PCI_HCI)
	//RTL_W8(REG_SYS_FUNC_EN, FEN_PPLL|FEN_PCIEA|FEN_DIO_PCIE|FEN_USBA|FEN_BB_GLB_RST|FEN_BBRSTB);
	//RTL_W8(REG_LDOHCI12_CTRL, 0x1f);
#elif defined(CONFIG_SDIO_HCI)
	RTL_W8(REG_SYS_FUNC_EN, FEN_PPLL|FEN_PCIEA|FEN_DIO_PCIE|FEN_BB_GLB_RST|FEN_BBRSTB);
#endif

#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	if (
#ifdef CONFIG_RTL_92C_SUPPORT
		(GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		||
#endif
		(GET_CHIP_VER(priv) == VERSION_8192D)
#endif
	) {
		RTL_W8(REG_AFE_XTAL_CTRL + 1, 0x80);

		val32 = RTL_R32(REG_AFE_XTAL_CTRL);
		val32 = (val32 & (~(BIT(11) | BIT(14)))) | (BIT(18) | BIT(19) | BIT(21) | BIT(22));
		RTL_W32(REG_AFE_XTAL_CTRL, val32);
	}

#endif
	/*----Check chip ID and hw TR MIMO config----*/
//	check_chipID_MIMO(priv);
#ifdef USE_OUT_SRC
	if(ODMPTR->ConfigBBRF)
		rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_PHY_REG);
	else
#endif		
	{
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C))
		{
			if (get_rf_mimo_mode(priv) == MIMO_2T2R)
				rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_2T2R);
			else if (get_rf_mimo_mode(priv) == MIMO_1T1R)
				rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_1T1R);
		}
		else
			rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG);
	}

#ifdef MP_TEST
	if ((priv->pshare->rf_ft_var.mp_specific)
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
			&& (
#ifdef CONFIG_RTL_92C_SUPPORT
				!IS_TEST_CHIP(priv)
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
				||
#endif
				(GET_CHIP_VER(priv) == VERSION_8192D)
#endif
			)
#endif
	   ) {
		delay_ms(10);
#ifdef USE_OUT_SRC		
	if(ODMPTR->ConfigBBRF)
		rtStatus |= ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_PHY_REG_MP);
	else
#endif		
		rtStatus |= PHY_ConfigBBWithParaFile(priv, PHYREG_MP);
	}
#endif

	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile(): Write BB Reg Fail!!\n");
		return rtStatus;
	}

	/*----If EEPROM or EFUSE autoload OK, We must config by PHY_REG_PG.txt----*/
#ifdef USE_OUT_SRC
	if (0)
		rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_PHY_REG_PG);
	else
#endif
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_PG);

	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile():BB_PG Reg Fail!!\n");
		return rtStatus;
	}

	/*----BB AGC table Initialization----*/
#ifdef CONFIG_RTL_92D_SUPPORT

	if (GET_CHIP_VER(priv) == VERSION_8192D)
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn | bCCKEn, 0);
#endif
#ifdef USE_OUT_SRC
	if(ODMPTR->ConfigBBRF)
		rtStatus = ODM_ConfigBBWithHeaderFile(ODMPTR, CONFIG_BB_AGC_TAB);
	else
#endif		
		rtStatus = PHY_ConfigBBWithParaFile(priv, AGCTAB);

#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) != VERSION_8812E)
#endif
	{
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 1);
#ifdef CONFIG_RTL_92D_SUPPORT
		if (!(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G))

#endif
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 1);
	}

	if (rtStatus) {
		printk("phy_BB8192CD_Config_ParaFile(): Write BB AGC Table Fail!!\n");
		return rtStatus;
	}
	
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	// Check if the CCK HighPower is turned ON.
	// This is used to calculate PWDB.
	priv->pshare->phw->reg824_bit9 = (unsigned char)PHY_QueryBBReg(priv, rFPGA0_XA_HSSIParameter2, 0x200);
#endif

#if 0
	/*----For 1T2R Config----*/
	if (get_rf_mimo_mode(priv) == MIMO_1T2R) {
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_1T2R);
		if (rtStatus) {
			printk("phy_BB8192CD_Config_ParaFile(): Write BB Reg for 1T2R Fail!!\n");
			return rtStatus;
		}
	} else if (get_rf_mimo_mode(priv) == MIMO_1T1R) {
		delay_ms(100);
		rtStatus = PHY_ConfigBBWithParaFile(priv, PHYREG_1T1R);
		if (rtStatus) {
			printk("phy_BB8192CD_Config_ParaFile(): Write BB Reg for 1T1R Fail!!\n");
			return rtStatus;
		}
	}
#endif

	DEBUG_INFO("PHY-BB Initialization Success\n");
	return 0;
}
#else
static int phy_BB8192CD_Config_ParaFile(struct rtl8192cd_priv *priv)
{
	return 0;
}
#endif//CONFIG_WLAN_NOT_HAL_EXIST

#if 1
//#if !defined(CONFIG_MACBBRF_BY_ODM) || !defined(CONFIG_RTL_88E_SUPPORT)

int phy_RF6052_Config_ParaFile(struct rtl8192cd_priv *priv)
{
	int rtStatus = 0;
	RF92CD_RADIO_PATH_E eRFPath;
	BB_REGISTER_DEFINITION_T *pPhyReg;
	unsigned int  u4RegValue = 0, NumTotalRFPath;
#ifndef SMP_SYNC
	unsigned long x;
#endif

#ifdef CONFIG_WLAN_HAL
#if defined(HIGH_POWER_EXT_PA) && defined(HIGH_POWER_EXT_LNA)
	HW_VARIABLES RF_REG_FILE_RADIO_HP_SIZE[RF92CD_PATH_MAX]  = {
		HW_VAR_RFREGFILE_RADIO_A_HP_SIZE,  HW_VAR_RFREGFILE_RADIO_B_HP_SIZE,
#ifdef CONFIG_WLAN_HAL_8814AE
		HW_VAR_RFREGFILE_RADIO_C_HP_SIZE,  HW_VAR_RFREGFILE_RADIO_D_HP_SIZE
#endif
	};
	HW_VARIABLES RF_REG_FILE_RADIO_HP_START[RF92CD_PATH_MAX] = {
		HW_VAR_RFREGFILE_RADIO_A_HP_START, HW_VAR_RFREGFILE_RADIO_B_HP_START,
#ifdef CONFIG_WLAN_HAL_8814AE
		HW_VAR_RFREGFILE_RADIO_C_HP_START, HW_VAR_RFREGFILE_RADIO_D_HP_START
#endif
	};
#endif // HIGH_POWER_EXT_PA && HIGH_POWER_EXT_LNA

	HW_VARIABLES RF_REG_FILE_RADIO_SIZE[RF92CD_PATH_MAX]     = {
		HW_VAR_RFREGFILE_RADIO_A_SIZE,     HW_VAR_RFREGFILE_RADIO_B_SIZE,
#ifdef CONFIG_WLAN_HAL_8814AE
		HW_VAR_RFREGFILE_RADIO_C_SIZE,     HW_VAR_RFREGFILE_RADIO_D_SIZE
#endif
	};
	HW_VARIABLES RF_REG_FILE_RADIO_START[RF92CD_PATH_MAX]    = {
		HW_VAR_RFREGFILE_RADIO_A_START,    HW_VAR_RFREGFILE_RADIO_B_START,
#ifdef CONFIG_WLAN_HAL_8814AE
		HW_VAR_RFREGFILE_RADIO_C_START,    HW_VAR_RFREGFILE_RADIO_D_START
#endif
	};
	
#ifdef HIGH_POWER_EXT_PA
	HW_VARIABLES RF_REG_FILE_RADIO_EXTPA_SIZE[RF92CD_PATH_MAX]	= {
		HW_VAR_RFREGFILE_RADIO_A_EXTPA_SIZE,  HW_VAR_RFREGFILE_RADIO_B_EXTPA_SIZE,
#ifdef CONFIG_WLAN_HAL_8814AE			
		HW_VAR_RFREGFILE_RADIO_C_EXTPA_SIZE,  HW_VAR_RFREGFILE_RADIO_D_EXTPA_SIZE
#endif		
		};
	HW_VARIABLES RF_REG_FILE_RADIO_EXTPA_START[RF92CD_PATH_MAX] = {
		HW_VAR_RFREGFILE_RADIO_A_EXTPA_START, HW_VAR_RFREGFILE_RADIO_B_EXTPA_START,
#ifdef CONFIG_WLAN_HAL_8814AE				
		HW_VAR_RFREGFILE_RADIO_C_EXTPA_START, HW_VAR_RFREGFILE_RADIO_D_EXTPA_START
#endif		
		};
#endif // HIGH_POWER_EXT_PA

#ifdef HIGH_POWER_EXT_LNA
	HW_VARIABLES RF_REG_FILE_RADIO_EXTLNA_SIZE[RF92CD_PATH_MAX]  = {
		HW_VAR_RFREGFILE_RADIO_A_EXTLNA_SIZE,  HW_VAR_RFREGFILE_RADIO_B_EXTLNA_SIZE,
#ifdef CONFIG_WLAN_HAL_8814AE				
		HW_VAR_RFREGFILE_RADIO_C_EXTLNA_SIZE,  HW_VAR_RFREGFILE_RADIO_D_EXTLNA_SIZE
#endif		
		};
	
	HW_VARIABLES RF_REG_FILE_RADIO_EXTLNA_START[RF92CD_PATH_MAX] = {
		HW_VAR_RFREGFILE_RADIO_A_EXTLNA_START, HW_VAR_RFREGFILE_RADIO_B_EXTLNA_START,
#ifdef CONFIG_WLAN_HAL_8814AE		
		HW_VAR_RFREGFILE_RADIO_C_EXTLNA_START, HW_VAR_RFREGFILE_RADIO_D_EXTLNA_START
#endif			
		};
#endif // HIGH_POWER_EXT_LNA
#endif // CONFIG_WLAN_HAL

//	SAVE_INT_AND_CLI(x);

#ifdef CONFIG_RTL_92D_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8192D) && (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY))
		priv->pshare->phw->NumTotalRFPath = 1;
	else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E)
			priv->pshare->phw->NumTotalRFPath = 1;
		else
#endif
#ifdef CONFIG_WLAN_HAL
		if ( IS_HAL_CHIP(priv)) {
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_NUM_TOTAL_RF_PATH, (pu1Byte)&priv->pshare->phw->NumTotalRFPath);
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif //CONFIG_WLAN_HAL
		priv->pshare->phw->NumTotalRFPath = 2;
	
		NumTotalRFPath = priv->pshare->phw->NumTotalRFPath;	
#if defined(CONFIG_WLAN_HAL_8814AE)
		if(GET_CHIP_VER(priv) == VERSION_8814A)
			NumTotalRFPath++;
#endif
	for (eRFPath = RF92CD_PATH_A; eRFPath < NumTotalRFPath; eRFPath++) {

#ifdef CONFIG_WLAN_HAL_8814AE
        if (GET_CHIP_VER(priv) == VERSION_8814A && !IS_HAL_TEST_CHIP(priv)){
            // Note: In 8814A MP chip, we should read radio.txt in a special sequence. => RadioB, C, D, and A.
            eRFPath = (eRFPath+1)%4;
        }
#endif
		pPhyReg = &priv->pshare->phw->PHYRegDef[eRFPath];

		/*----Store original RFENV control type----*/
		switch (eRFPath) {
		case RF92CD_PATH_A:
			u4RegValue = PHY_QueryBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV);
			break;
		case RF92CD_PATH_B :
			u4RegValue = PHY_QueryBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV << 16);
			break;
#ifdef CONFIG_WLAN_HAL_8814AE
		case RF92CD_PATH_C:
			// TODO: 8814AE BB/RF
			break;
		case RF92CD_PATH_D:
			break;
#endif
		case RF92CD_PATH_MAX:
			break;
		}

#ifndef  CONFIG_WLAN_HAL_8814AE // TODO: temporarily remove in 8814A, need to ask SD8 eric
		/*----Set RF_ENV enable----*/
		PHY_SetBBReg(priv, pPhyReg->rfintfe, bRFSI_RFENV << 16, 0x1);

		/*----Set RF_ENV output high----*/
		PHY_SetBBReg(priv, pPhyReg->rfintfo, bRFSI_RFENV, 0x1);

		/* Set bit number of Address and Data for RF register */
		PHY_SetBBReg(priv, pPhyReg->rfHSSIPara2, b3WireAddressLength, 0x0);
		PHY_SetBBReg(priv, pPhyReg->rfHSSIPara2, b3WireDataLength, 0x0);
#endif
		/*----Initialize RF fom connfiguration file----*/
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			switch (eRFPath) {
			case RF92CD_PATH_A:
#ifdef CONFIG_RTL_92D_DMDP
				if ((priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY) &&
						(priv->pshare->wlandev_idx == 1)) {
#ifdef RTL8192D_INT_PA
					if (priv->pshare->rf_ft_var.use_intpa92d) {
#ifdef USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
						printk("[%s][radio_b_intPA_GM_new]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_GM_new_start,
															(int)(data_radio_b_intPA_GM_new_end - data_radio_b_intPA_GM_new_start), eRFPath);
#elif defined (RTL8192D_INT_PA_GAIN_TABLE_NEW1)
						printk("[%s][radio_b_intPA_GM_new1]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_GM_new1_start,
															(int)(data_radio_b_intPA_GM_new1_end - data_radio_b_intPA_GM_new1_start), eRFPath);
#else
						printk("[%s][radio_b_intPA_GM]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_GM_start,
															(int)(data_radio_b_intPA_GM_end - data_radio_b_intPA_GM_start), eRFPath);
#endif

#else //USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
						printk("[%s][radio_b_intPA_new]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_new_start,
															(int)(data_radio_b_intPA_new_end - data_radio_b_intPA_new_start), eRFPath);
#else
						printk("[%s][radio_b_intPA]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_start,
															(int)(data_radio_b_intPA_end - data_radio_b_intPA_start), eRFPath);
#endif

#endif //USB_POWER_SUPPORT

					} else
#endif
					{
#ifdef HIGH_POWER_EXT_PA
						if (priv->pshare->rf_ft_var.use_ext_pa) {
							printk("[%s][radio_b_n_92d_hp]\n", __FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_n_92d_hp_start,
																(int)(data_radio_b_n_92d_hp_end - data_radio_b_n_92d_hp_start), eRFPath);
						} else
#endif
						{
							printk("[%s] [radio_b_n]\n", __FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_n_start,
																(int)(data_radio_b_n_end - data_radio_b_n_start), eRFPath);
						}
					}
				} else
#endif
				{
#ifdef RTL8192D_INT_PA
					if (priv->pshare->rf_ft_var.use_intpa92d) {
#ifdef USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
						printk("[%s][radio_a_intPA_GM_new]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_intPA_GM_new_start,
															(int)(data_radio_a_intPA_GM_new_end - data_radio_a_intPA_GM_new_start), eRFPath);
#elif defined (RTL8192D_INT_PA_GAIN_TABLE_NEW1)
						printk("[%s][radio_a_intPA_GM_new1]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_intPA_GM_new1_start,
															(int)(data_radio_a_intPA_GM_new1_end - data_radio_a_intPA_GM_new1_start), eRFPath);
#else
						printk("[%s][radio_a_intPA_GM]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_intPA_GM_start,
															(int)(data_radio_a_intPA_GM_end - data_radio_a_intPA_GM_start), eRFPath);
#endif

#else //USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)

						printk("[%s][radio_a_intPA_new]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_intPA_new_start,
															(int)(data_radio_a_intPA_new_end - data_radio_a_intPA_new_start), eRFPath);

#else
						printk("[%s][radio_a_intPA]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_intPA_start,
															(int)(data_radio_a_intPA_end - data_radio_a_intPA_start), eRFPath);
#endif

#endif //USB_POWER_SUPPORT
					} else
#endif
					{
//_TXPWR_REDEFINE
#ifdef HIGH_POWER_EXT_PA
						if (priv->pshare->rf_ft_var.use_ext_pa) {
							printk("[%s][radio_a_n_92d_hp]\n", __FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_n_92d_hp_start,
																(int)(data_radio_a_n_92d_hp_end - data_radio_a_n_92d_hp_start), eRFPath);
						} else
#endif
						{
							printk("[%s][radio_a_n]\n", __FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_n_start,
																(int)(data_radio_a_n_end - data_radio_a_n_start), eRFPath);
						}
					}
				}
				break;
			case RF92CD_PATH_B:
#ifdef RTL8192D_INT_PA
				if (priv->pshare->rf_ft_var.use_intpa92d) {
#ifdef USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
					printk("[%s][radio_b_intPA_GM_new]\n", __FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_GM_new_start,
														(int)(data_radio_b_intPA_GM_new_end - data_radio_b_intPA_GM_new_start), eRFPath);
#elif defined (RTL8192D_INT_PA_GAIN_TABLE_NEW1)
					printk("[%s][radio_b_intPA_GM_new1]\n", __FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_GM_new1_start,
														(int)(data_radio_b_intPA_GM_new1_end - data_radio_b_intPA_GM_new1_start), eRFPath);
#else
					printk("[%s][radio_b_intPA_GM]\n", __FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_GM_start,
														(int)(data_radio_b_intPA_GM_end - data_radio_b_intPA_GM_start), eRFPath);
#endif


#else //USB_POWER_SUPPORT

#if defined (RTL8192D_INT_PA_GAIN_TABLE_NEW)
					printk("[%s][radio_b_intPA_new]\n", __FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_new_start,
														(int)(data_radio_b_intPA_new_end - data_radio_b_intPA_new_start), eRFPath);
#else
					printk("[%s][radio_b_intPA]\n", __FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_intPA_start,
														(int)(data_radio_b_intPA_end - data_radio_b_intPA_start), eRFPath);
#endif

#endif //USB_POWER_SUPPORT
				} else
#endif
				{
#ifdef HIGH_POWER_EXT_PA
					if (priv->pshare->rf_ft_var.use_ext_pa) {
						printk("[%s][radio_b_n_92d_hp]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_n_92d_hp_start,
															(int)(data_radio_b_n_92d_hp_end - data_radio_b_n_92d_hp_start), eRFPath);
					} else
#endif
					{
						printk("[%s][radio_b_n]\n", __FUNCTION__);
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_n_start,
															(int)(data_radio_b_n_end - data_radio_b_n_start), eRFPath);
					}
				}
				break;
			default:
				break;
			}
		}
#endif //!CONFIG_RTL_92D_SUPPORT
#if defined(CONFIG_RTL_92C_SUPPORT)
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
			switch (eRFPath) {
			case RF92CD_PATH_A:
				if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
#ifdef TESTCHIP_SUPPORT
					if (IS_TEST_CHIP(priv))
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_2T_start,
															(int)(data_radio_a_2T_end - data_radio_a_2T_start), eRFPath);
					else
#endif
					{
#ifdef HIGH_POWER_EXT_PA
						if (priv->pshare->rf_ft_var.use_ext_pa) {
							//printk("[%s][data_radio_a_2T_n_hp]\n", __FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_2T_n_hp_start,
																(int)(data_radio_a_2T_n_hp_end - data_radio_a_2T_n_hp_start), eRFPath);
						} else
#endif
#ifdef HIGH_POWER_EXT_LNA
						if (priv->pshare->rf_ft_var.use_ext_lna) {
							//printk("[%s][data_radio_a_2T_n_lna]\n", __FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_2T_n_lna_start,
																(int)(data_radio_a_2T_n_lna_end - data_radio_a_2T_n_lna_start), eRFPath);
						} else
#endif
						{ 						
							//printk("[%s][data_radio_a_2T_n]\n", __FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_2T_n_start,
																(int)(data_radio_a_2T_n_end - data_radio_a_2T_n_start), eRFPath);						
						}
					}
				} else if (get_rf_mimo_mode(priv) == MIMO_1T1R)	{
#ifdef TESTCHIP_SUPPORT
					if (IS_TEST_CHIP(priv))
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_1T_start,
															(int)(data_radio_a_1T_end - data_radio_a_1T_start), eRFPath);
					else
#endif
					{
#ifdef HIGH_POWER_EXT_PA
						if (priv->pshare->rf_ft_var.use_ext_pa) {
							//printk("[%s][data_radio_a_2T_n_hp]\n", __FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_2T_n_hp_start,
																(int)(data_radio_a_2T_n_hp_end - data_radio_a_2T_n_hp_start), eRFPath);
						} else
#endif
						{
							//printk("[%s][data_radio_a_1T_n]\n", __FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_a_1T_n_start,
																(int)(data_radio_a_1T_n_end - data_radio_a_1T_n_start), eRFPath);
						}

					}
				}
				break;
			case RF92CD_PATH_B:
				if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
#ifdef TESTCHIP_SUPPORT
					if (IS_TEST_CHIP(priv))
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_2T_start,
															(int)(data_radio_b_2T_end - data_radio_b_2T_start), eRFPath);
					else
#endif
					{
#ifdef HIGH_POWER_EXT_PA
						if (priv->pshare->rf_ft_var.use_ext_pa) {
							//printk("[%s][data_radio_b_2T_n_hp]\n", __FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_2T_n_hp_start,
																(int)(data_radio_b_2T_n_hp_end - data_radio_b_2T_n_hp_start), eRFPath);
						} else
#endif
#ifdef HIGH_POWER_EXT_LNA
						if (priv->pshare->rf_ft_var.use_ext_lna) {
							//printk("[%s][data_radio_b_2T_n_lna]\n", __FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_2T_n_lna_start,
																(int)(data_radio_b_2T_n_lna_end - data_radio_b_2T_n_lna_start), eRFPath);
						} else 
#endif
						{						
							//printk("[%s][data_radio_b_2T_n]\n", __FUNCTION__);
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_radio_b_2T_n_start,
																(int)(data_radio_b_2T_n_end - data_radio_b_2T_n_start), eRFPath);						
						}
					}
				} else if (get_rf_mimo_mode(priv) == MIMO_1T1R)
					rtStatus = 0;
				break;
			default:
				break;
			}
		}
#endif



#ifdef CONFIG_RTL_8812_SUPPORT //data radio
		if (GET_CHIP_VER(priv) == VERSION_8812E) {
			switch (eRFPath) {
			case RF92CD_PATH_A://for_8812_mp_chip
				if (IS_TEST_CHIP(priv)) {
#ifdef HIGH_POWER_EXT_PA //FOR_8812_HP
					if ( priv->pshare->rf_ft_var.use_ext_pa) {
						printk("[RadioA_8812_hp]\n");
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioA_8812_hp_start,
															(int)(data_RadioA_8812_hp_end - data_RadioA_8812_hp_start), eRFPath);
					} else
#endif
					{
						printk("[RadioA_8812]\n");
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioA_8812_start,
															(int)(data_RadioA_8812_end - data_RadioA_8812_start), eRFPath);
					}
				} else {
#ifdef HIGH_POWER_EXT_PA
#ifdef HIGH_POWER_EXT_LNA
					if (priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
						if (priv->pmib->dot11RFEntry.pa_type == PA_SKYWORKS_5023 || priv->pmib->dot11RFEntry.pa_type == PA_SKYWORKS_85712_HP) {
							panic_printk("[RadioA_8812_n_ultra_hp]\n");
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioA_8812_n_ultra_hp_start,
								(int)(data_RadioA_8812_n_ultra_hp_end - data_RadioA_8812_n_ultra_hp_start), eRFPath);
						} else {
							panic_printk("[RadioA_8812_n_hp]\n");
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioA_8812_n_hp_start,
							(int)(data_RadioA_8812_n_hp_end - data_RadioA_8812_n_hp_start), eRFPath);
						}
					} else if (priv->pshare->rf_ft_var.use_ext_pa && !priv->pshare->rf_ft_var.use_ext_lna) {
						panic_printk("[RadioA_8812_n_extpa]\n");
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioA_8812_n_extpa_start,
															(int)(data_RadioA_8812_n_extpa_end - data_RadioA_8812_n_extpa_start), eRFPath);
					} else if (!priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
						panic_printk("[RadioA_8812_n_extlna]\n");
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioA_8812_n_extlna_start,
															(int)(data_RadioA_8812_n_extlna_end - data_RadioA_8812_n_extlna_start), eRFPath);
					} else 
#else
					if (priv->pshare->rf_ft_var.use_ext_pa) {
					panic_printk("[RadioA_8812_n_extpa]\n");
					rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioA_8812_n_extpa_start,
														(int)(data_RadioA_8812_n_extpa_end - data_RadioA_8812_n_extpa_start), eRFPath);
					} else
#endif
#else
#ifdef HIGH_POWER_EXT_LNA
					if (priv->pshare->rf_ft_var.use_ext_lna) {					   
					   	panic_printk("[RadioA_8812_n_extlna]\n");
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioA_8812_n_extlna_start,
															(int)(data_RadioA_8812_n_extlna_end - data_RadioA_8812_n_extlna_start), eRFPath);

					} else
#endif
#endif
					{
						panic_printk("[RadioA_8812_n_default]\n");
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioA_8812_n_default_start,
															(int)(data_RadioA_8812_n_default_end - data_RadioA_8812_n_default_start), eRFPath);
					}
				}
				break;
			case RF92CD_PATH_B:
				if (IS_TEST_CHIP(priv)) {
#ifdef HIGH_POWER_EXT_PA //FOR_8812_HP
					if ( priv->pshare->rf_ft_var.use_ext_pa) {
						panic_printk("[RadioB_8812_hp]\n");
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioB_8812_hp_start,
															(int)(data_RadioB_8812_hp_end - data_RadioB_8812_hp_start), eRFPath);
					} else
#endif
					{
						panic_printk("[RadioB_8812]\n");
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioB_8812_start,
															(int)(data_RadioB_8812_end - data_RadioB_8812_start), eRFPath);
					}
				} else {
#ifdef HIGH_POWER_EXT_PA
#ifdef HIGH_POWER_EXT_LNA
					if (priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
						if (priv->pmib->dot11RFEntry.pa_type == PA_SKYWORKS_5023 || priv->pmib->dot11RFEntry.pa_type == PA_SKYWORKS_85712_HP) {
							panic_printk("[RadioB_8812_n_ultra_hp]\n");
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioB_8812_n_ultra_hp_start,
								(int)(data_RadioB_8812_n_ultra_hp_end - data_RadioB_8812_n_ultra_hp_start), eRFPath);
						} else {
							panic_printk("[RadioB_8812_n_hp]\n");
							rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioB_8812_n_hp_start,
								(int)(data_RadioB_8812_n_hp_end - data_RadioB_8812_n_hp_start), eRFPath);
						}
					} else if (priv->pshare->rf_ft_var.use_ext_pa && !priv->pshare->rf_ft_var.use_ext_lna) {
						panic_printk("[RadioB_8812_n_extpa]\n");
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioB_8812_n_extpa_start,
															(int)(data_RadioB_8812_n_extpa_end - data_RadioB_8812_n_extpa_start), eRFPath);
					} else if (!priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
						panic_printk("[RadioB_8812_n_extlna]\n");
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioB_8812_n_extlna_start,
															(int)(data_RadioB_8812_n_extlna_end - data_RadioB_8812_n_extlna_start), eRFPath);
					} else 
#else
					if (priv->pshare->rf_ft_var.use_ext_pa) {						
						panic_printk("[RadioB_8812_n_extpa]\n");
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioB_8812_n_extpa_start,
															(int)(data_RadioB_8812_n_extpa_end - data_RadioB_8812_n_extpa_start), eRFPath);
					} else
#endif
#else
#ifdef HIGH_POWER_EXT_LNA
					if (priv->pshare->rf_ft_var.use_ext_lna) {						
						panic_printk("[RadioB_8812_n_extlna]\n");
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioB_8812_n_extlna_start,
															(int)(data_RadioB_8812_n_extlna_end - data_RadioB_8812_n_extlna_start), eRFPath);
					} else
#endif
#endif
					{
						panic_printk("[RadioB_8812_n_default]\n");
						rtStatus = PHY_ConfigRFWithParaFile(priv, data_RadioB_8812_n_default_start,
															(int)(data_RadioB_8812_n_default_end - data_RadioB_8812_n_default_start), eRFPath);
					}
				}
				break;
			default:
				break;
			}
		}
#endif

#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
			pu4Byte FileStartPtr;
			u4Byte  FileLength;
#ifdef HIGH_POWER_EXT_PA
#ifdef HIGH_POWER_EXT_LNA
			if (priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {
				//printk("[%s][Radio_HAL_hp] path%d\n",__FUNCTION__,eRFPath+1);
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_HP_SIZE[eRFPath], (pu1Byte)&FileLength);
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_HP_START[eRFPath], (pu1Byte)&FileStartPtr);
    			rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
			} else if (priv->pshare->rf_ft_var.use_ext_pa && !priv->pshare->rf_ft_var.use_ext_lna) {
				//printk("[%s][Radio_HAL_extpa] path%d\n",__FUNCTION__,eRFPath+1);
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTPA_SIZE[eRFPath], (pu1Byte)&FileLength);
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTPA_START[eRFPath], (pu1Byte)&FileStartPtr);
    			rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
			} else if (!priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) {			
				//printk("[%s][Radio_HAL_extlna] path%d\n",__FUNCTION__,eRFPath+1);	
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTLNA_SIZE[eRFPath], (pu1Byte)&FileLength);
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTLNA_START[eRFPath], (pu1Byte)&FileStartPtr);
    			rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
			} else
#else
			if (priv->pshare->rf_ft_var.use_ext_pa) {	
				//printk("[%s][Radio_HAL_extpa] path%d\n",__FUNCTION__,eRFPath+1);				
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTPA_SIZE[eRFPath], (pu1Byte)&FileLength);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTPA_START[eRFPath], (pu1Byte)&FileStartPtr);
				rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
			}else
#endif  //HIGH_POWER_EXT_LNA
#else
#ifdef HIGH_POWER_EXT_LNA
			if (priv->pshare->rf_ft_var.use_ext_lna) {			
				//printk("[%s][Radio_HAL_extlna] path%d\n",__FUNCTION__,eRFPath+1);	
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTLNA_SIZE[eRFPath], (pu1Byte)&FileLength);
    			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_EXTLNA_START[eRFPath], (pu1Byte)&FileStartPtr);
    			rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
			} else
#endif
#endif  //HIGH_POWER_EXT_PA
			{
				//printk("[%s][Radio_HAL] path%d\n",__FUNCTION__,eRFPath+1);				
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_SIZE[eRFPath], (pu1Byte)&FileLength);
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, RF_REG_FILE_RADIO_START[eRFPath], (pu1Byte)&FileStartPtr);
				rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
			}
#if 0 // TODO: can be removed after 8814A test ok
			switch (eRFPath) {
			case RF92CD_PATH_A:
#ifdef HIGH_POWER_EXT_PA
				if ( priv->pshare->rf_ft_var.use_ext_pa) {
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_HP_SIZE, (pu1Byte)&FileLength);
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_HP_START, (pu1Byte)&FileStartPtr);
					printk("[%s][%s][RadioA_HAL_hp]\n", __FUNCTION__, ((GET_CHIP_VER(priv) == VERSION_8881A) ? "RTL_8881A" : "RTL_8192E"));
					rtStatus = PHY_ConfigRFWithParaFile(priv, FileStartPtr,
														(int)FileLength, eRFPath);
				} else
#endif
#ifdef HIGH_POWER_EXT_LNA
					if ( priv->pshare->rf_ft_var.use_ext_lna) {
						GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_EXTLNA_SIZE, (pu1Byte)&FileLength);
						GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_EXTLNA_START, (pu1Byte)&FileStartPtr);
						printk("[%s][%s][RadioA_HAL_extlna]\n", __FUNCTION__, ((GET_CHIP_VER(priv) == VERSION_8881A) ? "RTL_8881A" : "RTL_8192E"));
						rtStatus = PHY_ConfigRFWithParaFile(priv, FileStartPtr,
															(int)FileLength, eRFPath);
					} else
#endif
				{
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_SIZE, (pu1Byte)&FileLength);
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_A_START, (pu1Byte)&FileStartPtr);
					printk("[%s][RadioA_HAL]\n", __FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
				}
				break;
			case RF92CD_PATH_B:
#ifdef HIGH_POWER_EXT_PA
				if ( priv->pshare->rf_ft_var.use_ext_pa) {
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_HP_SIZE, (pu1Byte)&FileLength);
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_HP_START, (pu1Byte)&FileStartPtr);
					printk("[%s][%s][RadioB_HAL_hp]\n", __FUNCTION__, ((GET_CHIP_VER(priv) == VERSION_8881A) ? "RTL_8881A" : "RTL_8192E"));
					rtStatus = PHY_ConfigRFWithParaFile(priv, FileStartPtr,
														(int)FileLength, eRFPath);
				} else
#endif
#ifdef HIGH_POWER_EXT_LNA
				if ( priv->pshare->rf_ft_var.use_ext_lna) {
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_EXTLNA_SIZE, (pu1Byte)&FileLength);
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_EXTLNA_START, (pu1Byte)&FileStartPtr);
					printk("[%s][%s][RadioB_HAL_extlna]\n", __FUNCTION__, ((GET_CHIP_VER(priv) == VERSION_8881A) ? "RTL_8881A" : "RTL_8192E"));
					rtStatus = PHY_ConfigRFWithParaFile(priv, FileStartPtr,
														(int)FileLength, eRFPath);
				} else
#endif
				{
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_SIZE, (pu1Byte)&FileLength);
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_B_START, (pu1Byte)&FileStartPtr);
					printk("[%s][RadioB_HAL]\n", __FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr,
														(int)FileLength, eRFPath);
				}
				break;
#ifdef CONFIG_WLAN_HAL_8814AE				
// TODO: 8814AE BB/RF
			case RF92CD_PATH_C:
#ifdef HIGH_POWER_EXT_PA
				if ( priv->pshare->rf_ft_var.use_ext_pa) {
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_C_HP_SIZE, (pu1Byte)&FileLength);
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_C_HP_START, (pu1Byte)&FileStartPtr);
					printk("[%s][RadioC_HAL_hp]\n", __FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
				} else
#endif
				{
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_C_SIZE, (pu1Byte)&FileLength);
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_C_START, (pu1Byte)&FileStartPtr);
					printk("[%s][RadioC_HAL]\n", __FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
				}
				break;
			// TODO: 8814AE BB/RF
			case RF92CD_PATH_D:
#ifdef HIGH_POWER_EXT_PA
				if ( priv->pshare->rf_ft_var.use_ext_pa) {
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_D_HP_SIZE, (pu1Byte)&FileLength);
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_D_HP_START, (pu1Byte)&FileStartPtr);
					printk("[%s][RadioD_HAL_hp]\n", __FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
				} else
#endif
				{
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_D_SIZE, (pu1Byte)&FileLength);
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RFREGFILE_RADIO_D_START, (pu1Byte)&FileStartPtr);
					printk("[%s][RadioD_HAL]\n", __FUNCTION__);
					rtStatus = PHY_ConfigRFWithParaFile(priv, (pu1Byte)FileStartPtr, (int)FileLength, eRFPath);
				}
				break;
#endif // CONFIG_WLAN_HAL_8814AE

			default:
				break;
			}
#endif // end if 0

		}
#endif


#ifndef CONFIG_WLAN_HAL_8814AE // TODO: temporarily remove in 8814A, need to ask SD8 eric
		/*----Restore RFENV control type----*/;
		switch (eRFPath) {
		case RF92CD_PATH_A:
			PHY_SetBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV, u4RegValue);
			break;
		case RF92CD_PATH_B :
			PHY_SetBBReg(priv, pPhyReg->rfintfs, bRFSI_RFENV << 16, u4RegValue);
			break;
#ifdef CONFIG_WLAN_HAL_8814AE
		// TODO: 8814AE BB/RF
		case RF92CD_PATH_C:
			break;
		case RF92CD_PATH_D:
			break;
#endif // CONFIG_WLAN_HAL_8814AE
		case RF92CD_PATH_MAX:
			break;
		}
#endif

#ifdef CONFIG_WLAN_HAL_8814AE
        if (GET_CHIP_VER(priv) == VERSION_8814A && !IS_HAL_TEST_CHIP(priv)){
            eRFPath = (eRFPath-1)%4;
        }
#endif
	}

#ifdef CONFIG_WLAN_HAL_8814AE
    /* RCK for pathB/C/D.  Fix 8814 MP-chip RF reg setting bug  */
    if (GET_CHIP_VER(priv) == VERSION_8814A && !IS_HAL_TEST_CHIP(priv)){
        unsigned int rfPathA = PHY_QueryRFReg(priv, RF_PATH_A, 0x1c, bMaskDWord, 1);
        PHY_SetRFReg(priv, RF_PATH_B, 0x1c, bMaskDWord, rfPathA);
        PHY_SetRFReg(priv, RF_PATH_C, 0x1c, bMaskDWord, rfPathA);
        PHY_SetRFReg(priv, RF_PATH_D, 0x1c, bMaskDWord, rfPathA);
    }
#endif

	DEBUG_INFO("PHY-RF Initialization Success\n");

//	RESTORE_INT(x);
	return rtStatus;
}
#endif

//
// Description:
//	Set the MAC offset [0x09] and prevent all I/O for a while (about 20us~200us, suggested from SD4 Scott).
//	If the protection is not performed well or the value is not set complete, the next I/O will cause the system hang.
// Note:
//	This procudure is designed specifically for 8192S and references the platform based variables
//	which violates the stucture of multi-platform.
//	Thus, we shall not extend this procedure to common handler.
// By Bruce, 2009-01-08.
//
unsigned char
HalSetSysClk8192CD(	struct rtl8192cd_priv *priv,	unsigned char Data)
{
	RTL_W8((SYS_CLKR + 1), Data);
	delay_us(200);
	return TRUE;
}


#if defined(CONFIG_RTL_8812_SUPPORT) ||  defined(CONFIG_RTL_8723B_SUPPORT)

#define MAX_RX_DMA_BUFFER_SIZE_8812	0x3E80   //0x3FFF	// RX 16K


static void LLT_table_init_8812(struct rtl8192cd_priv *priv)
{
	unsigned int	i;//, maxPage = 255;
	unsigned int	count = 0;
//	unsigned int	value32;	//High+low page number
	unsigned char   txpktbuf_bndy = 0xfc, bufBd = 0xff;


	DEBUG_INFO("=====>LLT_table_init_8812\n");

	// 12.	TXRKTBUG_PG_BNDY 0x114[31:0] = 0x27FF00F6	//TXRKTBUG_PG_BNDY
	RTL_W8(TRXFF_BNDY, txpktbuf_bndy);

	RTL_W16(TRXFF_BNDY + 2, MAX_RX_DMA_BUFFER_SIZE_8812 - 1);

	// 13.	TDECTRL[15:8] 0x209[7:0] = 0xF6				// Beacon Head for TXDMA
	RTL_W8(TDECTRL + 1, txpktbuf_bndy);

	// 14.	BCNQ_PGBNDY 0x424[7:0] =  0xF6				//BCNQ_PGBNDY
	// 2009/12/03 Why do we set so large boundary. confilct with document V11.
	RTL_W8(TXPKTBUF_BCNQ_BDNY, txpktbuf_bndy);
	RTL_W8(TXPKTBUF_MGQ_BDNY, txpktbuf_bndy);

	// 15.	WMAC_LBK_BF_HD 0x45D[7:0] =  0xF6			//WMAC_LBK_BF_HD
	RTL_W8(TXPKTBUF_WMAC_LBK_BF_HD, txpktbuf_bndy);

	// Set Tx/Rx page size (Tx must be 128 Bytes, Rx can be 64,128,256,512,1024 bytes)
	// 16.	PBP [7:0] = 0x11								// TRX page size
	RTL_W8(PBP, 0x31);

	// 17.	DRV_INFO_SZ = 0x04
	RTL_W8(RX_DRVINFO_SZ, 0x4);

	// 18.	LLT_table_init(Adapter);
	for ( i = 0; i < txpktbuf_bndy - 1; i++) {
		RTL_W32(LLT_INI, ((LLTE_RWM_WR & LLTE_RWM_Mask) << LLTE_RWM_SHIFT) | (i & LLTINI_ADDR_Mask) << LLTINI_ADDR_SHIFT
				| ((i + 1)&LLTINI_HDATA_Mask) << LLTINI_HDATA_SHIFT);

		count = 0;
		do {
			if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD & LLTE_RWM_Mask) << LLTE_RWM_SHIFT)))
				break;
			if (++count >= 100) {
				printk("LLT_init, section 01, i=%d\n", i);
				printk("LLT Polling failed 01 !!!\n");
				return;
			}
		} while (count < 100);
	}

	RTL_W32(LLT_INI, ((LLTE_RWM_WR & LLTE_RWM_Mask) << LLTE_RWM_SHIFT)
			| ((txpktbuf_bndy - 1)&LLTINI_ADDR_Mask) << LLTINI_ADDR_SHIFT | (255 & LLTINI_HDATA_Mask) << LLTINI_HDATA_SHIFT);

	count = 0;
	do {
		if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD & LLTE_RWM_Mask) << LLTE_RWM_SHIFT)))
			break;
		if (++count >= 100) {
			printk("LLT Polling failed 02 !!!\n");
			return;
		}
	} while (count < 100);


	for (i = txpktbuf_bndy; i < bufBd; i++) {
		RTL_W32(LLT_INI, ((LLTE_RWM_WR & LLTE_RWM_Mask) << LLTE_RWM_SHIFT) | (i & LLTINI_ADDR_Mask) << LLTINI_ADDR_SHIFT
				| ((i + 1)&LLTINI_HDATA_Mask) << LLTINI_HDATA_SHIFT);

		do {
			if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD & LLTE_RWM_Mask) << LLTE_RWM_SHIFT)))
				break;
			if (++count >= 100) {
				printk("LLT Polling failed 03 !!!\n");
				return;
			}
		} while (count < 100);
	}

	RTL_W32(LLT_INI, ((LLTE_RWM_WR & LLTE_RWM_Mask) << LLTE_RWM_SHIFT) | (bufBd & LLTINI_ADDR_Mask) << LLTINI_ADDR_SHIFT
			| (txpktbuf_bndy & LLTINI_HDATA_Mask) << LLTINI_HDATA_SHIFT);

	count = 0;
	do {
		if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD & LLTE_RWM_Mask) << LLTE_RWM_SHIFT)))
			break;
		if (++count >= 100) {
			printk("LLT Polling failed 04 !!!\n");
			return;
		}
	} while (count < 100);


	// Set reserved page for each queue
	// 11.	RQPN 0x200[31:0]	= 0x80BD1C1C				// load RQPN

	if (priv->pmib->dot11OperationEntry.wifi_specific != 0) {
#ifdef MULTI_STA_REFINE
		if(priv->pshare->rf_ft_var.txbuf_merge) {
			RTL_W8(RQPN_NPQ, 0x49);
			RTL_W32(RQPN, 0x80a90004);
		} else 
#endif
		{
			RTL_W8(RQPN_NPQ, 0x29);
			RTL_W32(RQPN, 0x80a92004);
		}
	} else {
		RTL_W32(RQPN, 0x80EB0808);//0x80cb1010);//0x80711010);//0x80cb1010);
		RTL_W8(RQPN_NPQ, 0x0);
	}

	DEBUG_INFO("LLT_table_init_8812<=====\n");

	return;

}

#endif //CONFIG_RTL_8812_SUPPORT

static void LLT_table_init(struct rtl8192cd_priv *priv)
{
	unsigned txpktbufSz, bufBd;
	unsigned int i, count = 0;

#ifdef CONFIG_PCI_HCI
#if 1
#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pmib->dot11RFEntry.macPhyMode != SINGLEMAC_SINGLEPHY) {
		txpktbufSz = 120; //0x7C
		bufBd = 127;
	} else
#endif
	{
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E) {
#ifdef DRVMAC_LB
			txpktbufSz = 83; // 0x53
			bufBd = 87;
#else
			txpktbufSz = 171; // 0xAB
			bufBd = 175;
#endif
		} else
#endif
		{
			txpktbufSz = 246; // 0xF6
			bufBd = 255;
		}
	}
#else
	unsigned txpktbufSz = 252; //174(0xAE) 120(0x78) 252(0xFC)
#endif
#endif // CONFIG_PCI_HCI

#if defined(CONFIG_RTL_88E_SUPPORT) && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	if (!priv->pmib->dot11OperationEntry.wifi_specific) {
		txpktbufSz = TX_PAGE_BOUNDARY_88E;
	} else {
		txpktbufSz = WMM_NORMAL_TX_PAGE_BOUNDARY_88E;
	}
	bufBd = LAST_ENTRY_OF_TX_PKT_BUFFER_88E;
#endif

	for ( i = 0; i < txpktbufSz - 1; i++) {
		RTL_W32(LLT_INI, ((LLTE_RWM_WR & LLTE_RWM_Mask) << LLTE_RWM_SHIFT) | (i & LLTINI_ADDR_Mask) << LLTINI_ADDR_SHIFT
				| ((i + 1)&LLTINI_HDATA_Mask) << LLTINI_HDATA_SHIFT);

		count = 0;
		do {
			if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD & LLTE_RWM_Mask) << LLTE_RWM_SHIFT)))
				break;
			if (++count >= 100) {
				printk("LLT_init, section 01, i=%d\n", i);
				printk("LLT Polling failed 01 !!!\n");
				return;
			}
		} while (count < 100);
	}

	RTL_W32(LLT_INI, ((LLTE_RWM_WR & LLTE_RWM_Mask) << LLTE_RWM_SHIFT)
			| ((txpktbufSz - 1)&LLTINI_ADDR_Mask) << LLTINI_ADDR_SHIFT | (255 & LLTINI_HDATA_Mask) << LLTINI_HDATA_SHIFT);

	count = 0;
	do {
		if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD & LLTE_RWM_Mask) << LLTE_RWM_SHIFT)))
			break;
		if (++count >= 100) {
			printk("LLT Polling failed 02 !!!\n");
			return;
		}
	} while (count < 100);


	for (i = txpktbufSz; i < bufBd; i++) {
		RTL_W32(LLT_INI, ((LLTE_RWM_WR & LLTE_RWM_Mask) << LLTE_RWM_SHIFT) | (i & LLTINI_ADDR_Mask) << LLTINI_ADDR_SHIFT
				| ((i + 1)&LLTINI_HDATA_Mask) << LLTINI_HDATA_SHIFT);

		do {
			if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD & LLTE_RWM_Mask) << LLTE_RWM_SHIFT)))
				break;
			if (++count >= 100) {
				printk("LLT Polling failed 03 !!!\n");
				return;
			}
		} while (count < 100);
	}

	RTL_W32(LLT_INI, ((LLTE_RWM_WR & LLTE_RWM_Mask) << LLTE_RWM_SHIFT) | (bufBd & LLTINI_ADDR_Mask) << LLTINI_ADDR_SHIFT
			| (txpktbufSz & LLTINI_HDATA_Mask) << LLTINI_HDATA_SHIFT);

	count = 0;
	do {
		if (!(RTL_R32(LLT_INI) & ((LLTE_RWM_RD & LLTE_RWM_Mask) << LLTE_RWM_SHIFT)))
			break;
		if (++count >= 100) {
			printk("LLT Polling failed 04 !!!\n");
			return;
		}
	} while (count < 100);

// Set reserved page for each queue

#ifdef CONFIG_PCI_HCI
#if 1
	/* normal queue init MUST be previoius of RQPN enable */
	//RTL_W8(RQPN_NPQ, 4);		//RQPN_NPQ
#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pmib->dot11RFEntry.macPhyMode != SINGLEMAC_SINGLEPHY ) {
		if (priv->pmib->dot11OperationEntry.wifi_specific == 1) {
			RTL_W8(RQPN_NPQ, 0x29);
			RTL_W32(RQPN, 0x802f1c04);
		} else {
			//RTL_W32(RQPN, 0x80501010);
			RTL_W8(RQPN_NPQ, 0x10);
			//RTL_W32(RQPN, 0x80630410);
			RTL_W32(RQPN, 0x80600404);
		}
	} else
#endif
	{
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E) {
			//RTL_W8(RQPN_NPQ, 4);
			RTL_W8(RQPN_NPQ, 0x29);
#ifdef DRVMAC_LB
			RTL_W32(RQPN, 0x80460404);
#else
			//RTL_W32(RQPN, 0x805d2029);
			if (priv->pmib->dot11OperationEntry.wifi_specific == 1)
				RTL_W32(RQPN, 0x805d2004);
			else
				RTL_W32(RQPN, 0x807d0004);
#endif
		} else
#endif
		{
			RTL_W8(RQPN_NPQ, 0x29);
			//RTL_W32(RQPN, 0x809f2929);
			//RTL_W32(RQPN, 0x80a82029);
#ifdef DRVMAC_LB
			RTL_W8(RQPN_NPQ + 2, 0x4);
			RTL_W32(RQPN, 0x80380404);
#else
			if (priv->pmib->dot11OperationEntry.wifi_specific == 0) {
				RTL_W32(RQPN, 0x80C50404);
			} else {
				RTL_W32(RQPN, 0x80a92004);
			}
#endif
		}
	}
#else
	if (txpktbufSz == 120)
		RTL_W32(RQPN, 0x80272828);
	else if (txpktbufSz == 252) {
		//RTL_W32(RQPN, 0x80c31c1c);

		// Joseph test
		//RTL_W32(RQPN, 0x80838484);
		RTL_W32(RQPN, 0x80bd1c1c);
	} else
		RTL_W32(RQPN, 0x80393a3a);
#endif
#endif // CONFIG_PCI_HCI

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
#ifdef CONFIG_RTL_88E_SUPPORT
	_InitQueueReservedPage(priv);
#endif
#endif

	//RTL_W32(TDECTRL, RTL_R32(TDECTRL)|(txpktbufSz&BCN_HEAD_Mask)<<BCN_HEAD_SHIFT);
	RTL_W8(TXPKTBUF_BCNQ_BDNY, txpktbufSz);
	RTL_W8(TXPKTBUF_MGQ_BDNY, txpktbufSz);
	RTL_W8(TRXFF_BNDY, txpktbufSz);
	RTL_W8(TDECTRL + 1, txpktbufSz);

	RTL_W8(0x45D, txpktbufSz);
}


#ifdef CONFIG_RTL_8812_SUPPORT
static void MacInit_8812(struct rtl8192cd_priv *priv)
{
	volatile unsigned char bytetmp;
	volatile unsigned int Qbytetmp;
	unsigned short retry;
	DEBUG_INFO("CP: MacInit_8812===>>\n");

	RTL_W8(RSV_CTRL0, 0x00);
#if 0
	if(RTL_R8(RSV_CTRL0) == 0)
		RTL_W8(SPS0_CTRL, (RTL_R8(SPS0_CTRL) & 0xc3) | 0x10);   // b[5:2] = 4
	else
		panic_printk("MAC reg unlock fail\n");
#endif

	//Auto Power Down to CHIP-off State
	bytetmp = RTL_R8(APS_FSMCO + 1);
	bytetmp &= (~BIT(7)); //PlatformEFIORead1Byte(Adapter, REG_APS_FSMCO_8812+1) & (~BIT7)
	RTL_W8(APS_FSMCO + 1, bytetmp);

	//printk("_eric HalPwrSeqCmdParsing +++ \n");
	//HW power on sequence
	if (!HalPwrSeqCmdParsing(priv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK,
							 PWR_INTF_PCI_MSK, rtl8812_card_enable_flow))
		panic_printk("!!!	[%s %d]HalPwrSeqCmdParsing init fail!!!\n", __FUNCTION__, __LINE__);

	//printk("_eric HalPwrSeqCmdParsing --- \n");

#if 0
	RTL_W32(0x500, 0x7f);
	RTL_W32(0x504, 0x7f);
	RTL_W32(0x508, 0x7f);
	RTL_W32(0x50c, 0x7f);

	printk("0x%x, 0x%x, 0x%x, 0x%x \n", RTL_R32(0x500), RTL_R32(0x504), RTL_R32(0x508), RTL_R32(0x50c));
#endif

	// Release MAC IO register reset
	RTL_W32(CR, RTL_R32(CR) | MACRXEN | MACTXEN | SCHEDULE_EN | PROTOCOL_EN
			| RXDMA_EN | TXDMA_EN | HCI_RXDMA_EN | HCI_TXDMA_EN);
	delay_ms(2);

	RTL_W8(HWSEQ_CTRL, 0x7f);
	delay_ms(2);


	// Add for wakeup online
	bytetmp = RTL_R8(SYS_CLKR);
	bytetmp |= BIT(3);
	RTL_W8(SYS_CLKR, bytetmp);

	bytetmp = RTL_R8(GPIO_MUXCFG + 1);
	bytetmp &= (~BIT(4));
	RTL_W8(GPIO_MUXCFG + 1, bytetmp);


	// Release MAC IO register reset
	// 9.	CR 0x100[7:0]	= 0xFF;
	// 10.	CR 0x101[1]	= 0x01; // Enable SEC block
	RTL_W16(CR, 0x2ff);


	//System init
	LLT_table_init_8812(priv);

	//printk("\n\n 0x%.x, 0x%.8x \n\n\n", RTL_R32(0x200),  RTL_R32(0x204));

	// Enable interrupt
	RTL_W32(REG_HISR0_8812, 0xffffffff);
	RTL_W32(REG_HISR1_8812, 0xffffffff);

	Qbytetmp = RTL_R16(REG_TRXDMA_CTRL_8812);
	Qbytetmp &= 0xf;

#ifdef MULTI_STA_REFINE
	if(priv->pshare->rf_ft_var.txbuf_merge)
		Qbytetmp |= 0xaaa3;
	else
#endif
	Qbytetmp |= 0x5663; //0xF5B1;
	RTL_W16(REG_TRXDMA_CTRL_8812, Qbytetmp);


	// Reported Tx status from HW for rate adaptive.
	// 2009/12/03 MH This should be realtive to power on step 14. But in document V11
	// still not contain the description.!!!
	RTL_W8(REG_FWHW_TXQ_CTRL_8812 + 1, 0x0F);

	// disable earlymode
	RTL_W8(0x4d0, 0x0);

#if 0		// 2012-07-06
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		RTL_W32(TRXFF_BNDY, (RTL_R32(TRXFF_BNDY) & 0x0000FFFF) | (0x13ff & RXFF0_BNDY_Mask) << RXFF0_BNDY_SHIFT);
	} else
#endif
	{
		// Set Rx FF0 boundary : 9K/10K
		RTL_W32(TRXFF_BNDY, (RTL_R32(TRXFF_BNDY) & 0x0000FFFF) | (0x27FF & RXFF0_BNDY_Mask) << RXFF0_BNDY_SHIFT);
	}
#endif
//	RTL_W8(TDECTRL, 0x11);	// need to confirm

	// Set Network type: ap mod
	//RTL_W32(CR, RTL_R32(CR) | BIT(8));

	RTL_W32(CR, RTL_R32(CR) | ((NETYPE_AP & NETYPE_Mask) << NETYPE_SHIFT));

	// Set SLOT time
	RTL_W8(SLOT_TIME, 0x09);

	// Set AMPDU min space
	RTL_W8(AMPDU_MIN_SPACE, 0);	//	need to confirm

	// Set Tx/Rx page size (Tx must be 128 Bytes, Rx can be 64,128,256,512,1024 bytes)
	//RTL_W8(PBP, (PBP_256B&PSTX_Mask)<<PSTX_SHIFT|(PBP_256B&PSRX_Mask)<<PSRX_SHIFT);

	// Set RCR register
	RTL_W32(RCR, RCR_APP_FCS | RCR_APP_MIC | RCR_APP_ICV | RCR_APP_PHYSTS | RCR_HTC_LOC_CTRL
			| RCR_AMF | RCR_ADF | RCR_ACRC32 | RCR_AB | RCR_AM | RCR_APM | RCR_AAP);

	// Set Driver info size
	RTL_W8(RX_DRVINFO_SZ, 4);

	// This part is not in WMAC InitMAC()
	// Set SEC register
	RTL_W16(SECCFG, RTL_R16(SECCFG) & ~(RXUSEDK | TXUSEDK));

	// Set TCR register
//	RTL_W32(TCR, RTL_R32(TCR)|CRC|CFE_FORM);
	RTL_W32(TCR, RTL_R32(TCR) | CFE_FORM);

	// Set TCR to avoid deadlock
	RTL_W32(TCR, RTL_R32(TCR) | BIT(15) | BIT(14) | BIT(13) | BIT(12));

	// Set RRSR (response rate set reg)
	//SetResponseRate();
	// Set RRSR (response rate set reg)
	// Set RRSR to all legacy rate and HT rate
	// CCK rate is supported by default.
	// CCK rate will be filtered out only when associated AP does not support it.
	// Only enable ACK rate to OFDM 24M


	{
		/*
		 *	Set RRSR at here before MACPHY_REG.txt is ready
		 */
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			/*
			 *	PHY_BAND_5G
			 */
			RTL_W16(RRSR, 0x150);
		} else {
			/*
			 *	PHY_BAND_2G
			 */
			RTL_W16(RRSR, 0x15F); //Set 0x15F for NDSi Client Connection Issue
		}
		RTL_W8(RRSR + 2, 0);

	}

	// Set Spec SIFS (used in NAV)
	// Joseph test
	RTL_W16(SPEC_SIFS_A, (0x10 & SPEC_SIFS_OFDM_Mask) << SPEC_SIFS_OFDM_SHIFT
			| (0x0A & SPEC_SIFS_CCK_Mask) << SPEC_SIFS_CCK_SHIFT);

	// Set SIFS for CCK
	// Joseph test
	RTL_W16(SIFS_CCK, (0x0E & SIFS_TRX_Mask) << SIFS_TRX_SHIFT | (0x0A & SIFS_CTX_Mask) << SIFS_CTX_SHIFT);

	// Set SIFS for OFDM
	// Joseph test
	RTL_W16(SIFS_OFDM, (0x0E & SIFS_TRX_Mask) << SIFS_TRX_SHIFT | (0x0A & SIFS_CTX_Mask) << SIFS_CTX_SHIFT);

	// Set retry limit
	if (priv->pmib->dot11OperationEntry.dot11LongRetryLimit)
		priv->pshare->RL_setting = priv->pmib->dot11OperationEntry.dot11LongRetryLimit & 0xff;
	else {
#ifdef CLIENT_MODE 
	    if (priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) 
			priv->pshare->RL_setting = 0x30;
		else
#endif 
			priv->pshare->RL_setting = 0x10;
	}
	if (priv->pmib->dot11OperationEntry.dot11ShortRetryLimit)
		priv->pshare->RL_setting |= ((priv->pmib->dot11OperationEntry.dot11ShortRetryLimit & 0xff) << 8);
	else {
#ifdef CLIENT_MODE 
	    if (priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) 
			priv->pshare->RL_setting |= (0x30 << 8);
		else
#endif 
			priv->pshare->RL_setting |= (0x10 << 8);
	}
	RTL_W16(REG_RL_8812, priv->pshare->RL_setting);


#if 0 //eric_8812 ??

	RTL_W16(TRXDMA_CTRL, (0xC660 | RXSHFT_EN | RXDMA_ARBBW_EN));
	//RTL_W8(PBP, (PBP_256B&PSTX_Mask)<<PSTX_SHIFT|(PBP_256B&PSRX_Mask)<<PSRX_SHIFT);

#endif



	//Set Desc Address
#if 0 //def CONFIG_RTL_8812_SUPPORT
	RTL_W32(BCNQ_DESA, priv->pshare->phw->tx_bufringB_addr);
	RTL_W32(REG_92E_MGQ_DESA, priv->pshare->phw->tx_bufring0_addr);
	RTL_W32(REG_92E_VOQ_DESA, priv->pshare->phw->tx_bufring4_addr);
	RTL_W32(REG_92E_VIQ_DESA, priv->pshare->phw->tx_bufring3_addr);
	RTL_W32(REG_92E_BEQ_DESA, priv->pshare->phw->tx_bufring2_addr);
	RTL_W32(REG_92E_BKQ_DESA, priv->pshare->phw->tx_bufring1_addr);
	RTL_W32(REG_92E_HI0Q_DESA, priv->pshare->phw->tx_bufring5_addr);

	RTL_W32(REG_92E_HI1Q_DESA, priv->pshare->phw->tx_bufring6_addr);
	RTL_W32(REG_92E_HI2Q_DESA, priv->pshare->phw->tx_bufring7_addr);
	RTL_W32(REG_92E_HI3Q_DESA, priv->pshare->phw->tx_bufring8_addr);
	RTL_W32(REG_92E_HI4Q_DESA, priv->pshare->phw->tx_bufring9_addr);
	RTL_W32(REG_92E_HI5Q_DESA, priv->pshare->phw->tx_bufring10_addr);
	RTL_W32(REG_92E_HI6Q_DESA, priv->pshare->phw->tx_bufring11_addr);
	RTL_W32(REG_92E_HI7Q_DESA, priv->pshare->phw->tx_bufring12_addr);

	RTL_W32(RX_DESA, priv->pshare->phw->rx_ring_addr);
	printk("0x%x,0x%x,0x%x,0x%x,0x%x\n", priv->pshare->phw->tx_bufring0_addr, priv->pshare->phw->tx_bufring1_addr, priv->pshare->phw->tx_bufring2_addr, priv->pshare->phw->tx_bufring3_addr, priv->pshare->phw->tx_bufring4_addr);
	printk("1clean hw host point,0x%x,0x%x\n", priv->pshare->phw->tx_bufring5_addr, priv->pshare->phw->tx_bufringB_addr);
	printk("2clean hw host point,0x%x,0x%x\n", priv->pshare->phw->tx_bufring6_addr, priv->pshare->phw->tx_bufring7_addr);
	printk("3clean hw host point,0x%x,0x%x\n", priv->pshare->phw->tx_bufring8_addr, priv->pshare->phw->tx_bufring9_addr);
	printk("4clean hw host point,0x%x,0x%x\n", priv->pshare->phw->tx_bufring10_addr, priv->pshare->phw->tx_bufring11_addr);
	printk("5clean hw host point,0x%x,0x%x\n", priv->pshare->phw->tx_bufring12_addr, priv->pshare->phw->tx_bufringB_addr);
	RTL_W32(REG_92E_ACQ_DES_NUM0, (((NUM_TX_DESC & ACQ_92E_VIQ_DESC_NUM_MASK) << ACQ_92E_VIQ_DESC_NUM_SHIFT) | ((NUM_TX_DESC & ACQ_92E_VOQ_DESC_NUM_MASK) << ACQ_92E_VOQ_DESC_NUM_SHIFT)));
	RTL_W32(REG_92E_ACQ_DES_NUM1, (((NUM_TX_DESC & ACQ_92E_BKQ_DESC_NUM_MASK) << ACQ_92E_BKQ_DESC_NUM_SHIFT) | ((NUM_TX_DESC & ACQ_92E_BEQ_DESC_NUM_MASK) << ACQ_92E_BEQ_DESC_NUM_SHIFT)));
	RTL_W32(REG_92E_HQ_DES_NUM0,  (((NUM_TX_DESC & ACQ_92E_H1Q_DESC_NUM_MASK) << ACQ_92E_H1Q_DESC_NUM_SHIFT) | ((NUM_TX_DESC & ACQ_92E_H0Q_DESC_NUM_MASK) << ACQ_92E_H0Q_DESC_NUM_SHIFT)));
	RTL_W32(REG_92E_HQ_DES_NUM1,  (((NUM_TX_DESC & ACQ_92E_H3Q_DESC_NUM_MASK) << ACQ_92E_H3Q_DESC_NUM_SHIFT) | ((NUM_TX_DESC & ACQ_92E_H2Q_DESC_NUM_MASK) << ACQ_92E_H2Q_DESC_NUM_SHIFT)));
	RTL_W32(REG_92E_HQ_DES_NUM2,  (((NUM_TX_DESC & ACQ_92E_H5Q_DESC_NUM_MASK) << ACQ_92E_H5Q_DESC_NUM_SHIFT) | ((NUM_TX_DESC & ACQ_92E_H4Q_DESC_NUM_MASK) << ACQ_92E_H4Q_DESC_NUM_SHIFT)));
	RTL_W32(REG_92E_HQ_DES_NUM3,  (((NUM_TX_DESC & ACQ_92E_H7Q_DESC_NUM_MASK) << ACQ_92E_H7Q_DESC_NUM_SHIFT) | ((NUM_TX_DESC & ACQ_92E_H6Q_DESC_NUM_MASK) << ACQ_92E_H6Q_DESC_NUM_SHIFT)));

	RTL_W32(REG_92E_ACQ_DES_NUM2, (((NUM_RX_DESC & ACQ_92E_RXQ_DESC_NUM_MASK) << ACQ_92E_RXQ_DESC_NUM_SHIFT) | ((NUM_TX_DESC & ACQ_92E_MGQ_DESC_NUM_MASK) << ACQ_92E_MGQ_DESC_NUM_SHIFT)));
	//RTL_W32(0x3be, 64);
	RTL_W32(REG_92E_TSFT_CLRQ, CLRQ_92E_ALL_IDX);
	RTL_W8(REG_92E_UPD_HGQMD, RTL_R8(REG_92E_UPD_HGQMD) | BIT(5) | BIT(4));
	//RTL_W16(0x3be, 64);
	//RTL_W16(0x3b8, 64);
	//RTL_W8(0x229, 0xf6);
	//RTL_W8(0x457, 0xf6);
	//RTL_W8(0x5a7,0xff);
#else
	RTL_W32(BCNQ_DESA, priv->pshare->phw->tx_ringB_addr);
	RTL_W32(MGQ_DESA, priv->pshare->phw->tx_ring0_addr);
	RTL_W32(VOQ_DESA, priv->pshare->phw->tx_ring4_addr);
	RTL_W32(VIQ_DESA, priv->pshare->phw->tx_ring3_addr);
	RTL_W32(BEQ_DESA, priv->pshare->phw->tx_ring2_addr);
	DEBUG_INFO("BEQ_DESA = 0x%08x 0x%08x \n", RTL_R32(BEQ_DESA), (unsigned int)priv->pshare->phw->tx_ring2_addr);
	RTL_W32(BKQ_DESA, priv->pshare->phw->tx_ring1_addr);
	RTL_W32(HQ_DESA, priv->pshare->phw->tx_ring5_addr);
	RTL_W32(RX_DESA, priv->pshare->phw->ring_dma_addr);
#endif

//	RTL_W32(RCDA, priv->pshare->phw->rxcmd_ring_addr);
//	RTL_W32(TCDA, priv->pshare->phw->txcmd_ring_addr);
//	RTL_W32(TCDA, phw->tx_ring5_addr);
	// 2009/03/13 MH Prevent incorrect DMA write after accident reset !!!
//	RTL_W16(CMDR, 0x37FC);


	RTL_W32(PCIE_CTRL_REG, RTL_R32(PCIE_CTRL_REG) | (0x07 & MAX_RXDMA_Mask) << MAX_RXDMA_SHIFT
			| (0x07 & MAX_TXDMA_Mask) << MAX_TXDMA_SHIFT | BCNQSTOP);
	if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_5){
		RTL_W8(0x303,0x80);
	}
	// 20090928 Joseph
	// Reconsider when to do this operation after asking HWSD.
	RTL_W8(APSD_CTRL, RTL_R8(APSD_CTRL) & ~ BIT(6));
	retry = 0;
	do {
		retry++;
		bytetmp = RTL_R8(APSD_CTRL);
	} while ((retry < 200) && (bytetmp & BIT(7))); //polling until BIT7 is 0. by tynli

	if (bytetmp & BIT(7)) {
		DEBUG_ERR("%s ERROR: APSD_CTRL=0x%02X\n", __FUNCTION__, bytetmp);
	}
	// disable BT_enable
	RTL_W8(GPIO_MUXCFG, 0);



	DEBUG_INFO("MacInit_8812 DONE\n");
}
#endif

#ifdef CONFIG_RTL_8723B_SUPPORT
#define _FAIL 		0
#define _SUCCESS	1

static unsigned int _InitPowerOn_8723BE(struct rtl8192cd_priv *priv)
{

	unsigned char	tmpU1b;
	
	RTL_W8(REG_RSV_CTRL, 0x0);

	//Auto Power Down to CHIP-off State
	tmpU1b = (RTL_R8(REG_APS_FSMCO+1) & (~BIT7));
	RTL_W8(REG_APS_FSMCO+1, tmpU1b);

	if(!HalPwrSeqCmdParsing(priv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK, rtl8723B_card_enable_flow)){
		panic_printk("%s %d, HalPwrSeqCmdParsing init fail!!!\n", __FUNCTION__, __LINE__);
		return _FAIL;
	}
	//Enable Power Down Interrupt 
	tmpU1b = (RTL_R8(REG_APS_FSMCO) | BIT4);
	RTL_W8(REG_APS_FSMCO, tmpU1b);

	// Release MAC IO register reset
	RTL_W32(CR, RTL_R32(CR) | MACRXEN | MACTXEN | SCHEDULE_EN | PROTOCOL_EN
			| RXDMA_EN | TXDMA_EN | HCI_RXDMA_EN | HCI_TXDMA_EN);
	delay_ms(2);

	tmpU1b = RTL_R8(REG_HWSEQ_CTRL);
	tmpU1b |= 0x7f;
	RTL_W8(REG_HWSEQ_CTRL, tmpU1b);
	delay_ms(2);

	// Add for wakeup online
	tmpU1b = RTL_R8(REG_SYS_CLKR);
	RTL_W8(REG_SYS_CLKR, (tmpU1b|BIT3));
	tmpU1b = RTL_R8(REG_GPIO_MUXCFG+1);
	RTL_W8(REG_GPIO_MUXCFG+1, (tmpU1b & ~BIT4));

	// Release MAC IO register reset
	// 9.	CR 0x100[7:0]	= 0xFF;
	// 10.	CR 0x101[1]	= 0x01; // Enable SEC block
	RTL_W16(REG_CR, 0x2ff);

	return _SUCCESS;
}

static int InitMAC_8723B(struct rtl8192cd_priv *priv)
{
	unsigned char	tmpU1b, u1bRegCR;
	unsigned short	tmpU2b;
	unsigned int	rtStatus = _SUCCESS;
	unsigned short  retry;

	u1bRegCR = RTL_R8(REG_CR);
	DEBUG_TRACE(" power-on :REG_CR 0x100=0x%02x.\n", u1bRegCR);

	if(u1bRegCR != 0 && u1bRegCR != 0xEA)
		DEBUG_TRACE(" MAC has already power on.\n");

	DEBUG_TRACE("_InitPowerOn_8723BE\n");

	if(_InitPowerOn_8723BE(priv) == _FAIL) {
		panic_printk("_InitPowerOn8723BE fail\n");
		return _FAIL;
	}

	//
	//  MAC Initialize
	//
	panic_printk("=======>InitMAC_8723B()\n");
	
	//System init
	LLT_table_init_8812(priv);
		
	// Enable Host Interrupt
	RTL_W32(REG_HISR0_8723B, 0xffffffff);
	RTL_W32(REG_HISR1_8723B, 0xffffffff);

	tmpU2b = RTL_R16(REG_TRXDMA_CTRL_8723B);
	tmpU2b &= 0xf;
	tmpU2b |= 0x5663; //0xF5B1;
	RTL_W16(REG_TRXDMA_CTRL_8723B, tmpU2b);

	// Reported Tx status from HW for rate adaptive.
	RTL_W8(REG_FWHW_TXQ_CTRL_8723B + 1, 0x1F);

	// disable earlymode
	RTL_W8(0x4d0, 0x0);

	RTL_W32(CR, RTL_R32(CR) | ((NETYPE_AP & NETYPE_Mask) << NETYPE_SHIFT));
	// Set SLOT time
	RTL_W8(SLOT_TIME, 0x09);

	// Set AMPDU min space
	RTL_W8(AMPDU_MIN_SPACE, 0); //	need to confirm

	// Set RCR register
	RTL_W32(RCR, RCR_APP_FCS | RCR_APP_MIC | RCR_APP_ICV | RCR_APP_PHYSTS | RCR_HTC_LOC_CTRL
			| RCR_AMF | RCR_ADF | RCR_ACRC32 | RCR_AB | RCR_AM | RCR_APM | RCR_AAP);

	// Set Driver info size
	RTL_W8(RX_DRVINFO_SZ, 4);

	// Set SEC register
	RTL_W16(SECCFG, RTL_R16(SECCFG) & ~(RXUSEDK | TXUSEDK));

	// Set TCR register
	//	RTL_W32(TCR, RTL_R32(TCR)|CRC|CFE_FORM);
	RTL_W32(TCR, RTL_R32(TCR) | CFE_FORM);

	// Set TCR to avoid deadlock
	RTL_W32(TCR, RTL_R32(TCR) | BIT(15) | BIT(14) | BIT(13) | BIT(12));
	
	//Enable FW Beamformer Interrupt. 
	RTL_W8(REG_FWIMR+3, (RTL_R8(REG_FWIMR+3) | BIT6));

	// Set RCR register
	//RTL_W32(Adapter,REG_RCR, pHalData->ReceiveConfig);
	RTL_W16(REG_RXFLTMAP2, 0xFFFF);
	// Set TCR register
	//RTL_W32(Adapter,REG_TCR, pHalData->TransmitConfig);

	/*
	 *	Set RRSR at here before MACPHY_REG.txt is ready
	 */
	if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
		/*
		 *	PHY_BAND_5G
		 */
		RTL_W16(RRSR, 0x150);
	} else {
		/*
		 *	PHY_BAND_2G
		 */
		RTL_W16(RRSR, 0x15F); //Set 0x15F for NDSi Client Connection Issue
	}
	RTL_W8(RRSR + 2, 0);

	
	
	
		// Set retry limit
		if (priv->pmib->dot11OperationEntry.dot11LongRetryLimit)
			priv->pshare->RL_setting = priv->pmib->dot11OperationEntry.dot11LongRetryLimit & 0xff;
		else {
#ifdef CLIENT_MODE 
			if (priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) 
				priv->pshare->RL_setting = 0x30;
			else
#endif 
				priv->pshare->RL_setting = 0x10;
		}
		if (priv->pmib->dot11OperationEntry.dot11ShortRetryLimit)
			priv->pshare->RL_setting |= ((priv->pmib->dot11OperationEntry.dot11ShortRetryLimit & 0xff) << 8);
		else {
#ifdef CLIENT_MODE 
			if (priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) 
				priv->pshare->RL_setting |= (0x30 << 8);
			else
#endif 
				priv->pshare->RL_setting |= (0x10 << 8);
		}
		RTL_W16(REG_RL_8812, priv->pshare->RL_setting);
	//
	// Set TX/RX descriptor physical address(from OS API).
	//
	//Set Desc Address
	RTL_W32(BCNQ_DESA, priv->pshare->phw->tx_ringB_addr);
	RTL_W32(MGQ_DESA, priv->pshare->phw->tx_ring0_addr);
	RTL_W32(VOQ_DESA, priv->pshare->phw->tx_ring4_addr);
	RTL_W32(VIQ_DESA, priv->pshare->phw->tx_ring3_addr);
	RTL_W32(BEQ_DESA, priv->pshare->phw->tx_ring2_addr);
	DEBUG_INFO("BEQ_DESA = 0x%08x 0x%08x \n", RTL_R32(BEQ_DESA), (unsigned int)priv->pshare->phw->tx_ring2_addr);
	RTL_W32(BKQ_DESA, priv->pshare->phw->tx_ring1_addr);
	RTL_W32(HQ_DESA, priv->pshare->phw->tx_ring5_addr);
	RTL_W32(RX_DESA, priv->pshare->phw->ring_dma_addr);
	

	
	RTL_W32(PCIE_CTRL_REG, RTL_R32(PCIE_CTRL_REG) | (0x07 & MAX_RXDMA_Mask) << MAX_RXDMA_SHIFT
			| (0x07 & MAX_TXDMA_Mask) << MAX_TXDMA_SHIFT | BCNQSTOP);
	if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_5){
		RTL_W8(0x303,0x80);
	}
	// 20090928 Joseph
	// Reconsider when to do this operation after asking HWSD.
	RTL_W8(APSD_CTRL, RTL_R8(APSD_CTRL) & ~ BIT(6));
	retry = 0;
	do {
		retry++;
		tmpU1b = RTL_R8(APSD_CTRL);
	} while ((retry < 200) && (tmpU1b & BIT(7))); //polling until BIT7 is 0. by tynli

	if (tmpU1b & BIT(7)) {
		DEBUG_ERR("%s ERROR: APSD_CTRL=0x%02X\n", __FUNCTION__, tmpU1b);
	}
	// disable BT_enable
	RTL_W8(GPIO_MUXCFG, 0);

	tmpU1b = RTL_R8(REG_PCIE_CTRL_REG+3);
	RTL_W8(REG_PCIE_CTRL_REG+3, (tmpU1b|0x77));

	// 20100318 Joseph: Reset interrupt migration setting when initialization. Suggested by SD1.
	RTL_W32(REG_INT_MIG, 0);	
	//pHalData->bInterruptMigration = _FALSE;

	//2009.10.19. Reset H2C protection register. by tynli.
	RTL_W32(REG_MCUTST_1, 0x0);
	
// ending , Mask until BB secondary cca is ready. 
	RTL_W8(REG_SECONDARY_CCA_CTRL_8723B, 0x3);

	// Release RX DMA
	tmpU1b = RTL_R8(REG_RXDMA_CONTROL_8723B);
	RTL_W8(REG_RXDMA_CONTROL_8723B, tmpU1b&(~BIT2));

	DEBUG_TRACE("InitMAC_8723B() <====\n");
	
	return _SUCCESS; 
}
#endif


#if(CONFIG_WLAN_NOT_HAL_EXIST==1)
static int MacInit(struct rtl8192cd_priv *priv)
{
	volatile unsigned char bytetmp;
	unsigned short retry;
#ifdef CONFIG_RTL_88E_SUPPORT
	int round = 0;
#endif

	DEBUG_INFO("CP: MacInit===>>");

#ifdef CONFIG_PCI_HCI

	RTL_W8(RSV_CTRL0, 0x00);

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
	#if 0
		if(RTL_R8(RSV_CTRL0) == 0)
			RTL_W8(SPS0_CTRL, (RTL_R8(SPS0_CTRL) & 0xc3) | 0x10);   // b[5:2] = 4
		else
			panic_printk("MAC reg unlock fail\n");
	#endif
		if (!HalPwrSeqCmdParsing(priv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, 
				PWR_INTF_PCI_MSK, rtl8188E_card_enable_flow))
			panic_printk("%s %d, HalPwrSeqCmdParsing init fail!!!\n", __FUNCTION__, __LINE__);

#ifdef SUPPORT_RTL8188E_TC
		check_RTL8188E_testChip(priv);
#endif
	}
#endif
#ifdef CONFIG_RTL_8812_SUPPORT //_eric_8812 mac init
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		//set RF register as PI mode
		RTL_W8(0xc00, RTL_R8(0xc00) | BIT(2));
		RTL_W8(0xe00, RTL_R8(0xe00) | BIT(2));

		MacInit_8812(priv);
		return TRUE;
	}
#endif

#ifdef CONFIG_RTL_8723B_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8723B) {
		unsigned char	tmpU1b;
		unsigned int rtStatus = InitMAC_8723B(priv);
		if(rtStatus != _SUCCESS) {
			panic_printk("Init MAC failed\n");
			return rtStatus;
		}
		tmpU1b = 0;
		RTL_W8(0x384, tmpU1b);
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		RTL_W8(SYS_FUNC_EN, FEN_PPLL | FEN_PCIEA | FEN_DIO_PCIE);

		/* Enable PLL Power (LDOA15V) */
		RTL_W8(LDOA15_CTRL, LDA15_OBUF | LDA15_EN);

		/* advise by MAC team */
		RTL_W8(LDOHCI12_CTRL, 0x1f);

#ifndef NOT_RTK_BSP
#if !defined(CONFIG_RTL_8198) && !defined(CONFIG_RTL_819XD) && !defined(CONFIG_RTL_8196E) && !defined(CONFIG_RTL_8198B)
		{
			/* temp modifying, for 96c pocket ap better performance */
			volatile unsigned int Qbytetmp;
			Qbytetmp = REG32(0xb8000048);
			Qbytetmp &= ~(BIT(10) | BIT(8));
			Qbytetmp |= BIT(19);
			REG32(0xb8000048) = Qbytetmp;
		}
#endif
#endif
	}

#endif

	// Power on when re-enter from IPS/Radio off/card disable
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		volatile unsigned int Qbytetmp;
		RTL_W8(AFE_XTAL_CTRL, RTL_R8(AFE_XTAL_CTRL) | BIT(0));

		Qbytetmp = RTL_R16(APS_FSMCO);
		Qbytetmp &= 0xE7ff;
		Qbytetmp |= 0x0800;
		RTL_W16(APS_FSMCO, Qbytetmp);

		while (!((Qbytetmp = RTL_R32(APS_FSMCO)) & 0x00020000)) {
			if (++round > 10000) {
				panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
				break;
			}
		}

		Qbytetmp = RTL_R16(APS_FSMCO);
		Qbytetmp &= 0x7FFF;
		RTL_W16(APS_FSMCO, Qbytetmp);

		Qbytetmp = RTL_R16(APS_FSMCO);
		Qbytetmp &= 0xE7ff;
		Qbytetmp |= 0x0000;
		RTL_W16(APS_FSMCO, Qbytetmp);
	} else
#endif
	{
#ifdef CONFIG_RTL8672
		RTL_W8(AFE_XTAL_CTRL, RTL_R8(AFE_XTAL_CTRL) | BIT(0));	// enable XTAL
#else
		/* just don't change BIT(1),Crystal engine setting refine*/
		//RTL_W8(AFE_XTAL_CTRL, 0x0d);	// enable XTAL		// clk inverted
#endif
		RTL_W8(SPS0_CTRL, 0x2b);		// enable SPS into PWM
	}
	delay_ms(1);

#if 0
	// Enable AFE BANDGAP
	RTL_W8(AFE_MISC, RTL_R8(AFE_MISC) | AFE_BGEN);
	DEBUG_INFO("AFE_MISC = 0x%02x\n", RTL_R8(AFE_MISC));

	// Enable AFE MBIAS
	RTL_W8(AFE_MISC, RTL_R8(AFE_MISC) | AFE_MBEN);
	DEBUG_INFO("AFE_MISC = 0x%02x\n", RTL_R8(AFE_MISC));

	// Enable PLL Power (LDOA15V)
#ifdef CONFIG_RTL_92C_SUPPORT //#ifndef CONFIG_RTL_92D_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C))
		RTL_W8(LDOA15_CTRL, RTL_R8(LDOA15_CTRL) | LDA15_EN);
#endif

	// Enable VDDCORE (LDOD12V)
	RTL_W8(LDOV12D_CTRL, RTL_R8(LDOV12D_CTRL) | LDV12_EN);

	// Release XTAL Gated for AFE PLL
//	RTL_W32(AFE_XTAL_CTRL, RTL_R32(AFE_XTAL_CTRL)|XTAL_GATE_AFE);
	RTL_W32(AFE_XTAL_CTRL, RTL_R32(AFE_XTAL_CTRL) & ~XTAL_GATE_AFE);

	// Enable AFE PLL
	RTL_W32(AFE_PLL_CTRL, RTL_R32(AFE_PLL_CTRL) | APLL_EN);

	// Release Isolation AFE PLL & MD
	RTL_W16(SYS_ISO_CTRL, RTL_R16(SYS_ISO_CTRL) & ~ISO_MD2PP);

	// Enable WMAC Clock
	RTL_W16(SYS_CLKR, RTL_R16(SYS_CLKR) | MAC_CLK_EN | SEC_CLK_EN);

	// Release WMAC reset & register reset
	RTL_W16(SYS_FUNC_EN, RTL_R16(SYS_FUNC_EN) | FEN_MREGEN | FEN_DCORE);

	// Release IMEM Isolation
	RTL_W16(SYS_ISO_CTRL, RTL_R16(SYS_ISO_CTRL) & ~(BIT(10) | ISO_DIOR));	//	need to confirm

	/*	// need double setting???
		// Enable MAC IO registers
		RTL_W16(SYS_FUNC_EN, RTL_R16(SYS_FUNC_EN)|FEN_MREGEN);
	*/

	// Switch HWFW select
	RTL_W16(SYS_CLKR, (RTL_R16(SYS_CLKR) | CLKR_80M_SSC_DIS) & ~BIT(6));	//	need to confirm
#else
	// auto enable WLAN

	// Power On Reset for MAC Block
	bytetmp = RTL_R8(APS_FSMCO + 1) | BIT(0);
	delay_us(2);
	RTL_W8(APS_FSMCO + 1, bytetmp);
	delay_us(2);

	bytetmp = RTL_R8(APS_FSMCO + 1);
	delay_us(2);
	retry = 0;
	while ((bytetmp & BIT(0)) && retry < 1000) {
		retry++;
		delay_us(50);
		bytetmp = RTL_R8(APS_FSMCO + 1);
		delay_us(50);
	}

	if (bytetmp & BIT(0)) {
		DEBUG_ERR("%s ERROR: auto enable WLAN failed!!(0x%02X)\n", __FUNCTION__, bytetmp);
	}

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E)
		RTL_W16(SYS_FUNC_EN, RTL_R16(SYS_FUNC_EN) & ~FEN_CPUEN);
	else		/*Enable Radio off, GPIO, and LED function*/
#endif
		RTL_W16(APS_FSMCO, 0x1012);			// when enable HWPDN

	// release RF digital isolation
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	if (
#ifdef CONFIG_RTL_92C_SUPPORT
		(GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		||
#endif
		(GET_CHIP_VER(priv) == VERSION_8192D)
#endif
	)
		RTL_W8(SYS_ISO_CTRL + 1, 0x82);
#endif

	delay_us(2);
#endif

	// Release MAC IO register reset
	RTL_W32(CR, RTL_R32(CR) | MACRXEN | MACTXEN | SCHEDULE_EN | PROTOCOL_EN
			| RXDMA_EN | TXDMA_EN | HCI_RXDMA_EN | HCI_TXDMA_EN);

#endif // CONFIG_PCI_HCI

#ifdef CONFIG_USB_HCI
#ifdef CONFIG_RTL_92C_SUPPORT
	if (rtl8192cu_InitPowerOn(priv) == FALSE)
		return FALSE;
#elif defined(CONFIG_RTL_88E_SUPPORT)
	if (rtl8188eu_InitPowerOn(priv) == FALSE)
		return FALSE;
#endif
#endif // CONFIG_USB_HCI

#if defined(CONFIG_SDIO_HCI) && defined(CONFIG_RTL_88E_SUPPORT)
	if (rtl8188es_InitPowerOn(priv) == FALSE)
		return FALSE;
#endif

	//System init
	LLT_table_init(priv);

	// Clear interrupt and enable interrupt
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		RTL_W32(REG_88E_HISR, 0xFFFFFFFF);
		RTL_W32(REG_88E_HISRE, 0xFFFFFFFF);
	} else
#endif
	{
		RTL_W32(HISR, 0xFFFFFFFF);
		RTL_W16(HISRE, 0xFFFF);
	}

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {

		switch (priv->pmib->dot11RFEntry.macPhyMode) {
		case SINGLEMAC_SINGLEPHY:
			RTL_W8(MAC_PHY_CTRL_T, 0xfc);
			RTL_W8(MAC_PHY_CTRL_MP, 0xfc); //enable super mac
			RTL_W32(AGGLEN_LMT, 0xb972a841);
			break;
		case DUALMAC_SINGLEPHY:
			RTL_W8(MAC_PHY_CTRL_T, 0xf1);
			RTL_W8(MAC_PHY_CTRL_MP, 0xf1); //enable supermac
			RTL_W32(AGGLEN_LMT, 0x54325521);
			break;
		case DUALMAC_DUALPHY:
			RTL_W8(MAC_PHY_CTRL_T, 0xf3);
			RTL_W8(MAC_PHY_CTRL_MP, 0xf3); //DMDP
			RTL_W32(AGGLEN_LMT, 0x54325521);
			break;
		default:
			DEBUG_ERR("Unknown 92D macPhyMode selection!\n");
		}
		/*
		 *    Set Rx FF0 boundary, half sized for testchip & dual MAC
		 */
#ifdef CONFIG_RTL_92D_DMDP
		if (priv->pmib->dot11RFEntry.macPhyMode != SINGLEMAC_SINGLEPHY)
			RTL_W32(TRXFF_BNDY, (RTL_R32(TRXFF_BNDY) & 0x0000FFFF) | (0x13ff & RXFF0_BNDY_Mask) << RXFF0_BNDY_SHIFT);
		else
#endif
			RTL_W32(TRXFF_BNDY, (RTL_R32(TRXFF_BNDY) & 0x0000FFFF) | (0x27ff & RXFF0_BNDY_Mask) << RXFF0_BNDY_SHIFT);

	} else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E) {
			RTL_W32(TRXFF_BNDY, (RTL_R32(TRXFF_BNDY) & 0x0000FFFF) | (0x25ff & RXFF0_BNDY_Mask) << RXFF0_BNDY_SHIFT);
		} else
#endif
		{
			// Set Rx FF0 boundary : 9K/10K
			RTL_W32(TRXFF_BNDY, (RTL_R32(TRXFF_BNDY) & 0x0000FFFF) | (0x27FF & RXFF0_BNDY_Mask) << RXFF0_BNDY_SHIFT);
		}

#ifdef CONFIG_PCI_HCI
#ifdef CONFIG_RTL_92C_SUPPORT
	if (IS_TEST_CHIP(priv)) {
		// Set High priority queue select : HPQ:BC/H/VO/VI/MG, LPQ:BE/BK
		// [5]:H, [4]:MG, [3]:BK, [2]:BE, [1]:VI, [0]:VO
		RTL_W16(TRXDMA_CTRL, ((HPQ_SEL_HIQ | HPQ_SEL_MGQ | HPQ_SEL_VIQ | HPQ_SEL_VOQ)&HPQ_SEL_Mask) << HPQ_SEL_SHIFT);

		/*
		 * Enable ampdu rx check error, and enable rx byte shift
		 */
		RTL_W8(TRXDMA_CTRL, RTL_R8(TRXDMA_CTRL) | RXSHFT_EN | RXDMA_ARBBW_EN);
	} else
#endif
	{
#if 0//def CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E)
			RTL_W16(TRXDMA_CTRL, (/*0xF5B1*/ 0xB5B1 | RXSHFT_EN | RXDMA_ARBBW_EN));
		else
#endif
			//RTL_W16(TRXDMA_CTRL, (0xB770 | RXSHFT_EN | RXDMA_ARBBW_EN));

#if defined(CONFIG_RTL_ULINKER_BRSC)
			RTL_W16(TRXDMA_CTRL, (0x5660 | RXDMA_ARBBW_EN)); //disable IP(layer3) auto aligne to 4bytes
#else
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8188E && priv->pmib->dot11OperationEntry.wifi_specific != 1)
				RTL_W16(TRXDMA_CTRL, (0x56a0 | RXSHFT_EN | RXDMA_ARBBW_EN));
			else
#endif
				RTL_W16(TRXDMA_CTRL, (0x5660 | RXSHFT_EN | RXDMA_ARBBW_EN));
#endif
	}
#endif // CONFIG_PCI_HCI

#ifdef CONFIG_USB_HCI
	_InitQueuePriority(priv);
	
	// Enable TXDMA to drop incorrect Bulk out packet
	RTL_W32(TXDMA_OFFSET_CHK, RTL_R32(TXDMA_OFFSET_CHK) | DROP_DATA_EN);
	
#ifdef CONFIG_RTL_92C_SUPPORT
	if (BOARD_USB_DONGLE == priv->pshare->BoardType)
		priv->pmib->dot11RFEntry.trswitch = 1;
#endif
#endif // CONFIG_USB_HCI

#ifdef CONFIG_SDIO_HCI
	_InitQueuePriority(priv);
#endif

//	RTL_W8(TDECTRL, 0x11);	// need to confirm

	// Set Network type: ap mode
	RTL_W32(CR, RTL_R32(CR) | ((NETYPE_AP & NETYPE_Mask) << NETYPE_SHIFT));

	// Set SLOT time
	RTL_W8(SLOT_TIME, 0x09);

	// Set AMPDU min space
	RTL_W8(AMPDU_MIN_SPACE, 0);	//	need to confirm

	// Set Tx/Rx page size (Tx must be 128 Bytes, Rx can be 64,128,256,512,1024 bytes)
	RTL_W8(PBP, (PBP_128B & PSTX_Mask) << PSTX_SHIFT | (PBP_128B & PSRX_Mask) << PSRX_SHIFT);

	// Set RCR register
	RTL_W32(RCR, RCR_APP_FCS | RCR_APP_MIC | RCR_APP_ICV | RCR_APP_PHYSTS | RCR_HTC_LOC_CTRL
			| RCR_AMF | RCR_ADF | RCR_ACRC32 | RCR_AB | RCR_AM | RCR_APM | RCR_AAP);

#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
    if (GET_CHIP_VER(priv) == VERSION_8192D || GET_CHIP_VER(priv) == VERSION_8192C || GET_CHIP_VER(priv) == VERSION_8188C) {
        RTL_W32(RCR, RTL_R32(RCR) | RCR_AICV); /*force accept icv error packet in 92D, 92C and 88C*/
    }
#endif

	// Set Driver info size
	RTL_W8(RX_DRVINFO_SZ, 4);

	// This part is not in WMAC InitMAC()
	// Set SEC register
	RTL_W16(SECCFG, RTL_R16(SECCFG) & ~(RXUSEDK | TXUSEDK));

	// Set TCR register
//	RTL_W32(TCR, RTL_R32(TCR)|CRC|CFE_FORM);
	RTL_W32(TCR, RTL_R32(TCR) | CFE_FORM);

	// Set TCR to avoid deadlock
	RTL_W32(TCR, RTL_R32(TCR) | BIT(15) | BIT(14) | BIT(13) | BIT(12));

	// Set RRSR (response rate set reg)
	//SetResponseRate();
	// Set RRSR (response rate set reg)
	// Set RRSR to all legacy rate and HT rate
	// CCK rate is supported by default.
	// CCK rate will be filtered out only when associated AP does not support it.
	// Only enable ACK rate to OFDM 24M
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		/*
		 *	Set RRSR at here before MACPHY_REG.txt is ready
		 */
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			/*
			 *	PHY_BAND_5G
			 */
			RTL_W16(RRSR, 0x150);
		} else {
			/*
			 *	PHY_BAND_2G
			 */
			RTL_W16(RRSR, 0x15F); //Set 0x15F for NDSi Client Connection Issue
		}
		RTL_W8(RRSR + 2, 0);

		RTL_W8(RCR, 0x0e);		//follow 92c MACPHY_REG
		RTL_W8(RCR + 1, 0x2a); 	//follow 92c MACPHY_REG
	} else
#endif
	{
		RTL_W16(RRSR, 0xFFFF);
		RTL_W8(RRSR + 2, 0xFF);
	}

	// Set Spec SIFS (used in NAV)
	// Joseph test
	RTL_W16(SPEC_SIFS_A, (0x0A & SPEC_SIFS_OFDM_Mask) << SPEC_SIFS_OFDM_SHIFT
			| (0x0A & SPEC_SIFS_CCK_Mask) << SPEC_SIFS_CCK_SHIFT);

	// Set SIFS for CCK
	// Joseph test
	RTL_W16(SIFS_CCK, (0x0A & SIFS_TRX_Mask) << SIFS_TRX_SHIFT | (0x0A & SIFS_CTX_Mask) << SIFS_CTX_SHIFT);

	// Set SIFS for OFDM
	// Joseph test
	RTL_W16(SIFS_OFDM, (0x0A & SIFS_TRX_Mask) << SIFS_TRX_SHIFT | (0x0A & SIFS_CTX_Mask) << SIFS_CTX_SHIFT);

	// Set retry limit
	if (priv->pmib->dot11OperationEntry.dot11LongRetryLimit)
		priv->pshare->RL_setting = priv->pmib->dot11OperationEntry.dot11LongRetryLimit & 0xff;
	else {
#ifdef CLIENT_MODE 
	    if (priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) 
			priv->pshare->RL_setting = 0x30;
		else
#endif 
		{
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
				priv->pshare->RL_setting = 0x10;
			else
				priv->pshare->RL_setting = 0x10;
		}
	}
	if (priv->pmib->dot11OperationEntry.dot11ShortRetryLimit)
		priv->pshare->RL_setting |= ((priv->pmib->dot11OperationEntry.dot11ShortRetryLimit & 0xff) << 8);
	else {
#ifdef CLIENT_MODE 
	    if (priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) 
			priv->pshare->RL_setting |= (0x30 << 8);
		else
#endif 
		{
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
				priv->pshare->RL_setting |= (0x10 << 8);
			else
				priv->pshare->RL_setting |= (0x10 << 8);
		}
	}
	RTL_W16(RL, priv->pshare->RL_setting);

#ifdef CONFIG_PCI_HCI
	//Set Desc Address
	RTL_W32(BCNQ_DESA, priv->pshare->phw->tx_ringB_addr);
	RTL_W32(MGQ_DESA, priv->pshare->phw->tx_ring0_addr);
	RTL_W32(VOQ_DESA, priv->pshare->phw->tx_ring4_addr);
	RTL_W32(VIQ_DESA, priv->pshare->phw->tx_ring3_addr);
	RTL_W32(BEQ_DESA, priv->pshare->phw->tx_ring2_addr);
	RTL_W32(BKQ_DESA, priv->pshare->phw->tx_ring1_addr);
	RTL_W32(HQ_DESA, priv->pshare->phw->tx_ring5_addr);
	RTL_W32(RX_DESA, priv->pshare->phw->ring_dma_addr);
//	RTL_W32(RCDA, priv->pshare->phw->rxcmd_ring_addr);
//	RTL_W32(TCDA, priv->pshare->phw->txcmd_ring_addr);
//	RTL_W32(TCDA, phw->tx_ring5_addr);
	// 2009/03/13 MH Prevent incorrect DMA write after accident reset !!!
//	RTL_W16(CMDR, 0x37FC);

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D)
		RTL_W32(PCIE_CTRL_REG, (RTL_R32(PCIE_CTRL_REG) & 0x00ffffff) | (0x03 & MAX_RXDMA_Mask) << MAX_RXDMA_SHIFT
				| (0x03 & MAX_TXDMA_Mask) << MAX_TXDMA_SHIFT | BCNQSTOP);
	else
#endif
		RTL_W32(PCIE_CTRL_REG, RTL_R32(PCIE_CTRL_REG) | (0x07 & MAX_RXDMA_Mask) << MAX_RXDMA_SHIFT
				| (0x07 & MAX_TXDMA_Mask) << MAX_TXDMA_SHIFT | BCNQSTOP);
#endif // CONFIG_PCI_HCI

	// 20090928 Joseph
	// Reconsider when to do this operation after asking HWSD.
	RTL_W8(APSD_CTRL, RTL_R8(APSD_CTRL) & ~ BIT(6));
	retry = 0;
	do {
		retry++;
		bytetmp = RTL_R8(APSD_CTRL);
	} while ((retry < 200) && (bytetmp & BIT(7))); //polling until BIT7 is 0. by tynli

	if (bytetmp & BIT(7)) {
		DEBUG_ERR("%s ERROR: APSD_CTRL=0x%02X\n", __FUNCTION__, bytetmp);
	}
	// disable BT_enable
	RTL_W8(GPIO_MUXCFG, 0);

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		RTL_W16(TCR, RTL_R16(TCR) | WMAC_TCR_ERRSTEN3 | WMAC_TCR_ERRSTEN2
				| WMAC_TCR_ERRSTEN1 | WMAC_TCR_ERRSTEN0);
		/*
		 *	For 92DE,Mac0 and Mac1 power off.
		 *	0x1F	BIT6: 0 mac0 off, 1: mac0 on
		 *		BIT7: 0 mac1 off, 1: mac1 on.
		 */
#ifdef CONFIG_RTL_92D_DMDP
		if (priv->pshare->wlandev_idx == 0)
#endif
		{
			RTL_W8(RSV_MAC0_CTRL, RTL_R8(RSV_MAC0_CTRL) | MAC0_EN);

			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
				RTL_W8(RSV_MAC0_CTRL, RTL_R8(RSV_MAC0_CTRL) & (~BAND_STAT));
			else
				RTL_W8(RSV_MAC0_CTRL, RTL_R8(RSV_MAC0_CTRL) | BAND_STAT);
		}

#ifdef CONFIG_RTL_92D_DMDP
		if (priv->pshare->wlandev_idx == 1) {
			RTL_W8(RSV_MAC1_CTRL, RTL_R8(RSV_MAC1_CTRL) | MAC1_EN);
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
				RTL_W8(RSV_MAC1_CTRL, RTL_R8(RSV_MAC1_CTRL) & (~BAND_STAT));
			else
				RTL_W8(RSV_MAC1_CTRL, RTL_R8(RSV_MAC1_CTRL) | BAND_STAT);
		}
#endif
	}
#endif //CONFIG_RTL_92D_SUPPORT
	
#if defined(CONFIG_USB_HCI)
	InitUsbAggregationSetting(priv);
#elif defined(CONFIG_SDIO_HCI)
	_initSdioAggregationSetting(priv);
#endif

#ifdef CONFIG_EXT_CLK_26M
	_InitClockTo26MHz(priv);
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	// Enable MACTXEN/MACRXEN block
	RTL_W32(CR, RTL_R32(CR) | (MACTXEN | MACRXEN));
#endif

	DEBUG_INFO("DONE\n");

	return TRUE;
}	//	MacInit
#else
static int MacInit(struct rtl8192cd_priv *priv)
{
	return TRUE;
}
#endif//CONFIG_WLAN_NOT_HAL_EXIST

#if(CONFIG_WLAN_NOT_HAL_EXIST==1)
static void MacConfig(struct rtl8192cd_priv *priv)
{
	//RTL_W8(INIRTS_RATE_SEL, 0x8); // 24M
	priv->pshare->phw->RTSInitRate_Candidate = priv->pshare->phw->RTSInitRate = 0x8; // 24M
	RTL_W8(INIRTS_RATE_SEL, priv->pshare->phw->RTSInitRate);

	// 2007/02/07 Mark by Emily becasue we have not verify whether this register works
	//For 92C,which reg?
	//	RTL_W8(BWOPMODE, BW_20M);	//	set if work at 20m

	// Ack timeout.
	if ((priv->pmib->miscEntry.ack_timeout > 0) && (priv->pmib->miscEntry.ack_timeout < 0xff))
		RTL_W8(ACKTO, priv->pmib->miscEntry.ack_timeout);
	else
		RTL_W8(ACKTO, 0x40);

	// clear for mbid beacon tx
	RTL_W8(MULTI_BCNQ_EN, 0);
	RTL_W8(MULTI_BCNQ_OFFSET, 0);

#ifdef CONFIG_RTL_92D_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8192D) && (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)) {
		RTL_W32(ARFR0, 0xFF010);	// 40M mode
		RTL_W32(ARFR1, 0xFF010);	// 20M mode
	} else
#endif
#if defined(CONFIG_RTL_88E_SUPPORT) && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	if (GET_CHIP_VER(priv) == VERSION_8188E)
	{
		RTL_W32(ARFR0, 0xFF00F);	// 40M mode
		RTL_W32(ARFR1, 0xFF00F);	// 20M mode
	} else
#endif
	{
		// set user defining ARFR table for 11n 1T
		RTL_W32(ARFR0, 0xFF015);	// 40M mode
		RTL_W32(ARFR1, 0xFF005);	// 20M mode
	}
	/*
	 * Disable TXOP CFE
	 */
	RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) | DIS_TXOP_CFE);

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		RTL_W8(MAC_SEL, 0);
		RTL_W8(0x526, 0xff);		/* enable all MBID interface beacon */

		RTL_W32(0x024, 0x0011800d);	//syn CLK enable, Xtal_bsel=nand
		RTL_W32(0x028, 0x00ffdb83);	//320MHz CLK enable
		RTL_W32(0x014, 0x088ba955);	//SPS=1.537V
		RTL_W32(0x010, 0x49022b03);

		/*
		 *	Protection mode control for hw RTS
		 */
		RTL_W16(PROT_MODE_CTRL, 0xff0D);
	}
#endif

	/*
	 *	RA try rate aggr limit
	 */
	RTL_W8(RA_TRY_RATE_AGG_LMT, 0);

	/*
	 *	Max mpdu number per aggr
	 */
	RTL_W16(PROT_MODE_CTRL + 2, 0x0909);

	if(!CHIP_VER_92X_SERIES(priv))
		RTL_W8(RESP_SIFS_OFDM+1, 0x0a);		//R2T

#if (defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT)) && defined(TX_EARLY_MODE)
	if (
#ifdef CONFIG_RTL_92D_SUPPORT
		(GET_CHIP_VER(priv) == VERSION_8192D)
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
#ifdef CONFIG_RTL_92D_SUPPORT
		||
#endif
		(GET_CHIP_VER(priv) == VERSION_8188E)
#endif
	)
		disable_em(priv);
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
	if (GET_CHIP_VER(priv) == VERSION_8188E)
		RTL8188E_EnableTxReport(priv);
#endif
	
#if defined(CONFIG_RTL_88E_SUPPORT) && defined(CONFIG_SDIO_HCI)
	//
	// Configure SDIO TxRx Control to enable Rx DMA timer masking.
	// 2010.02.24.
	//
//	value8 = SdioLocalCmd52Read1Byte(priv, SDIO_REG_TX_CTRL);
	SdioLocalCmd52Write4Byte(priv, SDIO_REG_TX_CTRL, 0x02);
	
#ifdef CONFIG_SDIO_TX_INTERRUPT
	// Invalidate All TX Free Page Threshold
	RTL_W32(REG_88E_TXDMA_TH, 0xFFFFFFFF);
	RTL_W16(REG_88E_LQ_TH, 0xFFFF);
#endif
	//
	// Update current Tx FIFO page status.
	//
	HalQueryTxBufferStatus8189ESdio(priv);
	HalQueryTxOQTBufferStatus8189ESdio(priv);
#endif // CONFIG_RTL_88E_SUPPORT && CONFIG_SDIO_HCI
}
#else
static void MacConfig(struct rtl8192cd_priv *priv)
{
	return;
}
#endif//CONFIG_WLAN_NOT_HAL_EXIST


unsigned int get_mean_of_2_close_value(unsigned int *val_array)
{
	unsigned int tmp1, tmp2;

	//printk("v1 %08x v2 %08x v3 %08x\n", val_array[0], val_array[1], val_array[2]);
	if (val_array[0] > val_array[1]) {
		tmp1 = val_array[1];
		val_array[1] = val_array[0];
		val_array[0] = tmp1;
	}
	if (val_array[1] > val_array[2]) {
		tmp1 = val_array[2];
		val_array[2] = val_array[1];
		val_array[1] = tmp1;
	}
	if (val_array[0] > val_array[1]) {
		tmp1 = val_array[1];
		val_array[1] = val_array[0];
		val_array[0] = tmp1;
	}
	//printk("v1 %08x v2 %08x v3 %08x\n", val_array[0], val_array[1], val_array[2]);

	tmp1 = val_array[1] - val_array[0];
	tmp2 = val_array[2] - val_array[1];
	if (tmp1 < tmp2)
		tmp1 = (val_array[0] + val_array[1]) / 2;
	else
		tmp1 = (val_array[1] + val_array[2]) / 2;

	//printk("final %08x\n", tmp1);
	return tmp1;
}

#ifdef CONFIG_RTL_92C_SUPPORT

void PHY_APCalibrate(struct rtl8192cd_priv *priv)
{
#ifdef HIGH_POWER_EXT_PA
	if (!priv->pshare->rf_ft_var.use_ext_pa)
#endif
		if (!IS_TEST_CHIP(priv)) {
			if (GET_CHIP_VER(priv) == VERSION_8192C)
				APK_MAIN(priv, 1);
			else if (GET_CHIP_VER(priv) == VERSION_8188C)
				APK_MAIN(priv, 0);
		}
}

#ifdef CONFIG_RTL_92C_SUPPORT
#ifndef CONFIG_RTL_NEW_IQK
static void IQK_92CD(struct rtl8192cd_priv *priv)
{
	unsigned int cal_num = 0, cal_retry = 0, Oldval = 0, temp_c04 = 0, temp_c08 = 0, temp_874 = 0, temp_eac;
	unsigned int cal_e94, cal_e9c, cal_ea4, cal_eac, cal_eb4, cal_ebc, cal_ec4, cal_ecc, adda_on_reg;
	unsigned int X, Y, val_e94[3], val_e9c[3], val_ea4[3], val_eac[3], val_eb4[3], val_ebc[3], val_ec4[3], val_ecc[3];
#ifdef HIGH_POWER_EXT_PA
	unsigned int temp_870 = 0, temp_860 = 0, temp_864 = 0;
#endif

	// step 1: save ADDA power saving parameters
	unsigned int temp_85c = RTL_R32(0x85c);
	unsigned int temp_e6c = RTL_R32(0xe6c);
	unsigned int temp_e70 = RTL_R32(0xe70);
	unsigned int temp_e74 = RTL_R32(0xe74);
	unsigned int temp_e78 = RTL_R32(0xe78);
	unsigned int temp_e7c = RTL_R32(0xe7c);
	unsigned int temp_e80 = RTL_R32(0xe80);
	unsigned int temp_e84 = RTL_R32(0xe84);
	unsigned int temp_e88 = RTL_R32(0xe88);
	unsigned int temp_e8c = RTL_R32(0xe8c);
	unsigned int temp_ed0 = RTL_R32(0xed0);
	unsigned int temp_ed4 = RTL_R32(0xed4);
	unsigned int temp_ed8 = RTL_R32(0xed8);
	unsigned int temp_edc = RTL_R32(0xedc);
	unsigned int temp_ee0 = RTL_R32(0xee0);
	unsigned int temp_eec = RTL_R32(0xeec);

#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
	{
		if (priv->pshare->iqk_2g_done)
			return;
		priv->pshare->iqk_2g_done = 1;
	}

	printk(">> %s \n", __FUNCTION__);

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		temp_870 = RTL_R32(0x870);
		temp_860 = RTL_R32(0x860);
		temp_864 = RTL_R32(0x864);
	}
#endif

	// step 2: Path-A ADDA all on
	adda_on_reg = 0x04db25a4;

	RTL_W32(0x85c, adda_on_reg);
	RTL_W32(0xe6c, adda_on_reg);
	RTL_W32(0xe70, adda_on_reg);
	RTL_W32(0xe74, adda_on_reg);
	RTL_W32(0xe78, adda_on_reg);
	RTL_W32(0xe7c, adda_on_reg);
	RTL_W32(0xe80, adda_on_reg);
	RTL_W32(0xe84, adda_on_reg);
	RTL_W32(0xe88, adda_on_reg);
	RTL_W32(0xe8c, adda_on_reg);
	RTL_W32(0xed0, adda_on_reg);
	RTL_W32(0xed4, adda_on_reg);
	RTL_W32(0xed8, adda_on_reg);
	RTL_W32(0xedc, adda_on_reg);
	RTL_W32(0xee0, adda_on_reg);
	RTL_W32(0xeec, adda_on_reg);

	// step 3: IQ&LO calibration Setting
	// BB switch to PI mode
	//RTL_W32(0x820, 0x01000100);
	//RTL_W32(0x828, 0x01000100);
	//BB setting
	temp_c04 = RTL_R32(0xc04);
	temp_c08 = RTL_R32(0xc08);
	temp_874 = RTL_R32(0x874);
	RTL_W32(0xc04, 0x03a05600);
	RTL_W32(0xc08, 0x000800e4);
	RTL_W32(0x874, 0x00204000);

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		PHY_SetBBReg(priv, 0x870, BIT(10), 1);
		PHY_SetBBReg(priv, 0x870, BIT(26), 1);
		PHY_SetBBReg(priv, 0x860, BIT(10), 0);
		PHY_SetBBReg(priv, 0x864, BIT(10), 0);
	}
#endif
	RTL_W32(0x840, 0x00010000);
	RTL_W32(0x844, 0x00010000);

	//AP or IQK
	RTL_W32(0xb68 , 0x00080000);
	RTL_W32(0xb6c , 0x00080000);

	// IQK setting
	PHY_SetBBReg(priv, 0xe28, 0xffffff00, 0x808000);
	RTL_W32(0xe40, 0x01007c00);
	RTL_W32(0xe44, 0x01004800);
	// path-A IQK setting
	RTL_W32(0xe30, 0x10008c1f);
	RTL_W32(0xe34, 0x10008c1f);
	RTL_W32(0xe38, 0x82140102);
	RTL_W32(0xe3c, 0x28160202);
	// path-B IQK setting
	RTL_W32(0xe50, 0x10008c22);
	RTL_W32(0xe54, 0x10008c22);
	RTL_W32(0xe58, 0x82140102);
	RTL_W32(0xe5c, 0x28160202);
	// LO calibration setting
	RTL_W32(0xe4c, 0x001028d1);

	// delay to ensure Path-A IQK success
	delay_ms(300);

	// step 4: One shot, path A LOK & IQK
	while (cal_num < 3) {
		// One shot, path A LOK & IQK
		RTL_W32(0xe48, 0xf9000000);
		RTL_W32(0xe48, 0xf8000000);
		// delay 1ms
		delay_ms(10);

		// check fail bit and check abnormal condition, then fill BB IQ matrix
		cal_e94 = (RTL_R32(0xe94) >> 16) & 0x3ff;
		cal_e9c = (RTL_R32(0xe9c) >> 16) & 0x3ff;
		cal_ea4 = (RTL_R32(0xea4) >> 16) & 0x3ff;
		temp_eac = RTL_R32(0xeac);
		cal_eac = (temp_eac >> 16) & 0x3ff;
		if (!(temp_eac & BIT(28)) && !(temp_eac & BIT(27)) &&
				(cal_e94 != 0x142) && (cal_e9c != 0x42) &&
				(cal_ea4 != 0x132) && (cal_eac != 0x36)) {
			val_e94[cal_num] = cal_e94;
			val_e9c[cal_num] = cal_e9c;
			val_ea4[cal_num] = cal_ea4;
			val_eac[cal_num] = cal_eac;
			cal_num++;
		} else {
			if (++cal_retry >= 10) {
				printk("%s Path-A Check\n", __FUNCTION__);
				break;
			}
		}
	}

	if (cal_num == 3) {
		cal_e94 = get_mean_of_2_close_value(val_e94);
		cal_e9c = get_mean_of_2_close_value(val_e9c);
		cal_ea4 = get_mean_of_2_close_value(val_ea4);
		cal_eac = get_mean_of_2_close_value(val_eac);

		priv->pshare->RegE94 = cal_e94;
		priv->pshare->RegE9C = cal_e9c;

		Oldval = (RTL_R32(0xc80) >> 22) & 0x3ff;

		X = cal_e94;
		PHY_SetBBReg(priv, 0xc80, 0x3ff, X * (Oldval / 0x100));
		PHY_SetBBReg(priv, 0xc4c, BIT(24), ((X * Oldval) >> 7) & 0x1);

		Y = cal_e9c;
		PHY_SetBBReg(priv, 0xc94, 0xf0000000, ((Y * (Oldval / 0x100)) >> 6) & 0xf);
		PHY_SetBBReg(priv, 0xc80, 0x003f0000, (Y * (Oldval / 0x100)) & 0x3f);
		PHY_SetBBReg(priv, 0xc4c, BIT(26), ((Y * Oldval) >> 7) & 0x1);

		PHY_SetBBReg(priv, 0xc14, 0x3ff, cal_ea4);

		PHY_SetBBReg(priv, 0xc14, 0xfc00, cal_eac & 0x3f);

		PHY_SetBBReg(priv, 0xca0, 0xf0000000, (cal_eac >> 6) & 0xf);
	} else {
		priv->pshare->RegE94 = 0x100;
		priv->pshare->RegE9C = 0x00;
	}

	// step 5: Path-A standby mode
	PHY_SetBBReg(priv, 0xe28, 0xffffff00, 0);
	RTL_W32(0x840, 0x00010000);
	PHY_SetBBReg(priv, 0xe28, 0xffffff00, 0x808000);

	// step 6: Path-B ADDA all on
	adda_on_reg = 0x0b1b25a4;

	RTL_W32(0x85c, adda_on_reg);
	RTL_W32(0xe6c, adda_on_reg);
	RTL_W32(0xe70, adda_on_reg);
	RTL_W32(0xe74, adda_on_reg);
	RTL_W32(0xe78, adda_on_reg);
	RTL_W32(0xe7c, adda_on_reg);
	RTL_W32(0xe80, adda_on_reg);
	RTL_W32(0xe84, adda_on_reg);
	RTL_W32(0xe88, adda_on_reg);
	RTL_W32(0xe8c, adda_on_reg);
	RTL_W32(0xed0, adda_on_reg);
	RTL_W32(0xed4, adda_on_reg);
	RTL_W32(0xed8, adda_on_reg);
	RTL_W32(0xedc, adda_on_reg);
	RTL_W32(0xee0, adda_on_reg);
	RTL_W32(0xeec, adda_on_reg);

	// step 7: One shot, path B LOK & IQK
	cal_num = 0;
	cal_retry = 0;
	while (cal_num < 3) {
		// One shot, path B LOK & IQK
		RTL_W32(0xe60, 2);
		RTL_W32(0xe60, 0);
		// delay 1ms
		delay_ms(10);

		// check fail bit and check abnormal condition, then fill BB IQ matrix
		cal_eb4 = (RTL_R32(0xeb4) >> 16) & 0x3ff;
		cal_ebc = (RTL_R32(0xebc) >> 16) & 0x3ff;
		cal_ec4 = (RTL_R32(0xec4) >> 16) & 0x3ff;
		cal_ecc = (RTL_R32(0xecc) >> 16) & 0x3ff;
		temp_eac = RTL_R32(0xeac);
		if (!(temp_eac & BIT(31)) && !(temp_eac & BIT(30)) &&
				(cal_eb4 != 0x142) && (cal_ebc != 0x42) &&
				(cal_ec4 != 0x132) && (cal_ecc != 0x36)) {
			val_eb4[cal_num] = cal_eb4;
			val_ebc[cal_num] = cal_ebc;
			val_ec4[cal_num] = cal_ec4;
			val_ecc[cal_num] = cal_ecc;
			cal_num++;
		} else {
			if (++cal_retry >= 10) {
				printk("%s Path-B Check\n", __FUNCTION__);
				break;
			}
		}
	}

	if (cal_num == 3) {
		cal_eb4 = get_mean_of_2_close_value(val_eb4);
		cal_ebc = get_mean_of_2_close_value(val_ebc);
		cal_ec4 = get_mean_of_2_close_value(val_ec4);
		cal_ecc = get_mean_of_2_close_value(val_ecc);

		priv->pshare->RegEB4 = cal_eb4;
		priv->pshare->RegEBC = cal_ebc;

		Oldval = (RTL_R32(0xc88) >> 22) & 0x3ff;

		X = cal_eb4;
		PHY_SetBBReg(priv, 0xc88, 0x3ff, X * (Oldval / 0x100));
		PHY_SetBBReg(priv, 0xc4c, BIT(28), ((X * Oldval) >> 7) & 0x1);

		Y = cal_ebc;
		PHY_SetBBReg(priv, 0xc9c, 0xf0000000, ((Y * (Oldval / 0x100)) >> 6) & 0xf);
		PHY_SetBBReg(priv, 0xc88, 0x003f0000, (Y * (Oldval / 0x100)) & 0x3f);
		PHY_SetBBReg(priv, 0xc4c, BIT(30), ((Y * Oldval) >> 7) & 0x1);

		PHY_SetBBReg(priv, 0xc1c, 0x3ff, cal_ec4);

		PHY_SetBBReg(priv, 0xc1c, 0xfc00, cal_ecc & 0x3f);

		PHY_SetBBReg(priv, 0xc78, 0xf000, (cal_ecc >> 6) & 0xf);
	} else {
		priv->pshare->RegEB4 = 0x100;
		priv->pshare->RegEBC = 0x00;
	}


	// step 8: back to BB mode, load original values
	RTL_W32(0xc04, temp_c04);
	RTL_W32(0x874, temp_874);
	RTL_W32(0xc08, temp_c08);
	PHY_SetBBReg(priv, 0xe28, 0xffffff00, 0);
	RTL_W32(0x840, 0x32ed3);
	RTL_W32(0x844, 0x32ed3);
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		RTL_W32(0x870, temp_870);
		RTL_W32(0x860, temp_860);
		RTL_W32(0x864, temp_864);
	}
#endif
	// return to SI mode
	//RTL_W32(0x820, 0x01000000);
	//RTL_W32(0x828, 0x01000000);

	// step 9: reload ADDA power saving parameters
	RTL_W32(0x85c, temp_85c);
	RTL_W32(0xe6c, temp_e6c);
	RTL_W32(0xe70, temp_e70);
	RTL_W32(0xe74, temp_e74);
	RTL_W32(0xe78, temp_e78);
	RTL_W32(0xe7c, temp_e7c);
	RTL_W32(0xe80, temp_e80);
	RTL_W32(0xe84, temp_e84);
	RTL_W32(0xe88, temp_e88);
	RTL_W32(0xe8c, temp_e8c);
	RTL_W32(0xed0, temp_ed0);
	RTL_W32(0xed4, temp_ed4);
	RTL_W32(0xed8, temp_ed8);
	RTL_W32(0xedc, temp_edc);
	RTL_W32(0xee0, temp_ee0);
	RTL_W32(0xeec, temp_eec);

}


static void IQK_88C(struct rtl8192cd_priv *priv)
{
	unsigned int cal_num = 0, cal_retry = 0;
	unsigned int Oldval_0 = 0, temp_c04 = 0, temp_c08 = 0, temp_874 = 0;
	unsigned int cal_e94, cal_e9c, cal_ea4, cal_eac, temp_eac;
	unsigned int X, Y, val_e94[3], val_e9c[3], val_ea4[3], val_eac[3];

#ifdef HIGH_POWER_EXT_PA
	unsigned int temp_870 = 0, temp_860 = 0, temp_864 = 0;
#endif
	// step 1: save ADDA power saving parameters
	unsigned int temp_85c = RTL_R32(0x85c);
	unsigned int temp_e6c = RTL_R32(0xe6c);
	unsigned int temp_e70 = RTL_R32(0xe70);
	unsigned int temp_e74 = RTL_R32(0xe74);
	unsigned int temp_e78 = RTL_R32(0xe78);
	unsigned int temp_e7c = RTL_R32(0xe7c);
	unsigned int temp_e80 = RTL_R32(0xe80);
	unsigned int temp_e84 = RTL_R32(0xe84);
	unsigned int temp_e88 = RTL_R32(0xe88);
	unsigned int temp_e8c = RTL_R32(0xe8c);
	unsigned int temp_ed0 = RTL_R32(0xed0);
	unsigned int temp_ed4 = RTL_R32(0xed4);
	unsigned int temp_ed8 = RTL_R32(0xed8);
	unsigned int temp_edc = RTL_R32(0xedc);
	unsigned int temp_ee0 = RTL_R32(0xee0);
	unsigned int temp_eec = RTL_R32(0xeec);

#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
	{
		if (priv->pshare->iqk_2g_done)
			return;
		priv->pshare->iqk_2g_done = 1;
	}

#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		temp_870 = RTL_R32(0x870);
		temp_860 = RTL_R32(0x860);
		temp_864 = RTL_R32(0x864);
	}
#endif

	// step 2: ADDA all on
	RTL_W32(0x85c, 0x0b1b25a0);
	RTL_W32(0xe6c, 0x0bdb25a0);
	RTL_W32(0xe70, 0x0bdb25a0);
	RTL_W32(0xe74, 0x0bdb25a0);
	RTL_W32(0xe78, 0x0bdb25a0);
	RTL_W32(0xe7c, 0x0bdb25a0);
	RTL_W32(0xe80, 0x0bdb25a0);
	RTL_W32(0xe84, 0x0bdb25a0);
	RTL_W32(0xe88, 0x0bdb25a0);
	RTL_W32(0xe8c, 0x0bdb25a0);
	RTL_W32(0xed0, 0x0bdb25a0);
	RTL_W32(0xed4, 0x0bdb25a0);
	RTL_W32(0xed8, 0x0bdb25a0);
	RTL_W32(0xedc, 0x0bdb25a0);
	RTL_W32(0xee0, 0x0bdb25a0);
	RTL_W32(0xeec, 0x0bdb25a0);

	// step 3: start IQK
	// BB switch to PI mode
	//RTL_W32(0x820, 0x01000100);
	//RTL_W32(0x828, 0x01000100);
	//BB setting
	temp_c04 = RTL_R32(0xc04);
	temp_c08 = RTL_R32(0xc08);
	temp_874 = RTL_R32(0x874);
	RTL_W32(0xc04, 0x03a05600);
	RTL_W32(0xc08, 0x000800e4);
	RTL_W32(0x874, 0x00204000);
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		PHY_SetBBReg(priv, 0x870, BIT(10), 1);
		PHY_SetBBReg(priv, 0x870, BIT(26), 1);
		PHY_SetBBReg(priv, 0x860, BIT(10), 0);
		PHY_SetBBReg(priv, 0x864, BIT(10), 0);
	}
#endif

	//AP or IQK
//	RTL_W32(0xb68, 0x0f600000);
	RTL_W32(0xb68, 0x00080000);

	// IQK setting
	PHY_SetBBReg(priv, 0xe28, 0xffffff00, 0x808000);
	RTL_W32(0xe40, 0x01007c00);
	RTL_W32(0xe44, 0x01004800);
	// path-A IQK setting
	RTL_W32(0xe30, 0x10008c1f);
	RTL_W32(0xe34, 0x10008c1f);
	RTL_W32(0xe38, 0x82140102);
	//RTL_W32(0xe3c, 0x28160502);
	RTL_W32(0xe3c, 0x28160202);

	// LO calibration setting
	RTL_W32(0xe4c, 0x001028d1);

	while (cal_num < 3) {
		// One shot, path A LOK & IQK
		RTL_W32(0xe48, 0xf9000000);
		RTL_W32(0xe48, 0xf8000000);
		// delay 1ms
		delay_ms(150);

		// step 4: check fail bit and check abnormal condition, then fill BB IQ matrix
		cal_e94 = (RTL_R32(0xe94) >> 16) & 0x3ff;
		cal_e9c = (RTL_R32(0xe9c) >> 16) & 0x3ff;
		cal_ea4 = (RTL_R32(0xea4) >> 16) & 0x3ff;
		temp_eac = RTL_R32(0xeac);
		cal_eac = (temp_eac >> 16) & 0x3ff;
		if (!(temp_eac & BIT(28)) && !(temp_eac & BIT(27)) &&
				(cal_e94 != 0x142) && (cal_e9c != 0x42) &&
				(cal_ea4 != 0x132) && (cal_eac != 0x36)) {
			val_e94[cal_num] = cal_e94;
			val_e9c[cal_num] = cal_e9c;
			val_ea4[cal_num] = cal_ea4;
			val_eac[cal_num] = cal_eac;
			cal_num++;
		} else {
			if (++cal_retry >= 10) {
				printk("IQK Check\n");
				break;
			}
		}
	}

	if (cal_num == 3) {
		cal_e94 = get_mean_of_2_close_value(val_e94);
		cal_e9c = get_mean_of_2_close_value(val_e9c);
		cal_ea4 = get_mean_of_2_close_value(val_ea4);
		cal_eac = get_mean_of_2_close_value(val_eac);

		priv->pshare->RegE94 = cal_e94;
		priv->pshare->RegE9C = cal_e9c;

		Oldval_0 = (RTL_R32(0xc80) >> 22) & 0x3ff;

		X = cal_e94;
		PHY_SetBBReg(priv, 0xc80, 0x3ff, X * (Oldval_0 / 0x100));
		PHY_SetBBReg(priv, 0xc4c, BIT(24), ((X * Oldval_0) >> 7) & 0x1);

		Y = cal_e9c;
		PHY_SetBBReg(priv, 0xc94, 0xf0000000, ((Y * (Oldval_0 / 0x100)) >> 6) & 0xf);
		PHY_SetBBReg(priv, 0xc80, 0x003f0000, (Y * (Oldval_0 / 0x100)) & 0x3f);
		PHY_SetBBReg(priv, 0xc4c, BIT(26), ((Y * Oldval_0) >> 7) & 0x1);

		PHY_SetBBReg(priv, 0xc14, 0x3ff, cal_ea4);

		PHY_SetBBReg(priv, 0xc14, 0xfc00, cal_eac & 0x3f);

		PHY_SetBBReg(priv, 0xca0, 0xf0000000, (cal_eac >> 6) & 0xf);
	} else {
		priv->pshare->RegE94 = 0x100;
		priv->pshare->RegE9C = 0x00;
	}

	// back to BB mode
	RTL_W32(0xc04, temp_c04);
	RTL_W32(0x874, temp_874);
	RTL_W32(0xc08, temp_c08);
	PHY_SetBBReg(priv, 0xe28, 0xffffff00, 0);
	RTL_W32(0x840, 0x00032ed3);
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		RTL_W32(0x870, temp_870);
		RTL_W32(0x860, temp_860);
		RTL_W32(0x864, temp_864);
	}
#endif
	// return to SI mode
	//RTL_W32(0x820, 0x01000000);
	//RTL_W32(0x828, 0x01000000);

	// step 5: reload ADDA power saving parameters
	RTL_W32(0x85c, temp_85c);
	RTL_W32(0xe6c, temp_e6c);
	RTL_W32(0xe70, temp_e70);
	RTL_W32(0xe74, temp_e74);
	RTL_W32(0xe78, temp_e78);
	RTL_W32(0xe7c, temp_e7c);
	RTL_W32(0xe80, temp_e80);
	RTL_W32(0xe84, temp_e84);
	RTL_W32(0xe88, temp_e88);
	RTL_W32(0xe8c, temp_e8c);
	RTL_W32(0xed0, temp_ed0);
	RTL_W32(0xed4, temp_ed4);
	RTL_W32(0xed8, temp_ed8);
	RTL_W32(0xedc, temp_edc);
	RTL_W32(0xee0, temp_ee0);
	RTL_W32(0xeec, temp_eec);
}	// IQK
#endif
#endif

#endif

void PHY_IQCalibrate(struct rtl8192cd_priv *priv)
{
#ifndef SMP_SYNC
	unsigned long x;
#endif

#ifdef RF_MIMO_SWITCH
	if(priv->pshare->rf_status) {
		if (get_rf_mimo_mode(priv) == MIMO_2T2R)
			set_MIMO_Mode(priv, MIMO_2T2R);
		 else if(get_rf_mimo_mode(priv) == MIMO_3T3R)
		 	set_MIMO_Mode(priv, MIMO_3T3R);
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		SAVE_INT_AND_CLI(x);
#ifdef CONFIG_RTL_92D_DMDP
		if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY) {
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
#ifdef DFS
				if ((priv->pshare->rf_ft_var.dfsdelayiqk) &&
						(OPMODE & WIFI_AP_STATE) &&
						!priv->pmib->dot11DFSEntry.disable_DFS &&
						(timer_pending(&priv->ch_avail_chk_timer) ||
						 priv->pmib->dot11DFSEntry.disable_tx)) {
					RESTORE_INT(x);	 
					return;
				}
#endif
				IQK_92D_5G_phy0_n(priv);
			} else
				IQK_92D_2G_phy1(priv);
		} else
#endif
		{
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
#ifdef DFS
				if ((priv->pshare->rf_ft_var.dfsdelayiqk) &&
						(OPMODE & WIFI_AP_STATE) &&
						!priv->pmib->dot11DFSEntry.disable_DFS &&
						(timer_pending(&priv->ch_avail_chk_timer) ||
						 priv->pmib->dot11DFSEntry.disable_tx)) {
					RESTORE_INT(x);
					return;
				}
#endif
				IQK_92D_5G_n(priv);
			} else
				IQK_92D_2G(priv);
		}
		RESTORE_INT(x);
	}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
		SAVE_INT_AND_CLI(x);
#ifdef CONFIG_RTL_NEW_IQK
		PHY_IQCalibrate_92C(priv);
#else
		if ( IS_UMC_A_CUT_88C(priv)
#ifdef HIGH_POWER_EXT_PA
				|| (priv->pshare->rf_ft_var.use_ext_pa)
#endif
		   ) {
			PHY_IQCalibrate_92C(priv);
		} else {
			if (GET_CHIP_VER(priv) == VERSION_8192C)
				IQK_92CD(priv);
			else
				IQK_88C(priv);
		}
#endif
		RESTORE_INT(x);
	}
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E)
#ifdef USE_OUT_SRC
		PHY_IQCalibrate_8188E(ODMPTR, FALSE);
#else
		PHY_IQCalibrate_8188E(priv, FALSE);
#endif
#endif

#ifdef CONFIG_RTL_8723B_SUPPORT 
	if (GET_CHIP_VER(priv) == VERSION_8723B){		
		panic_printk("(%s)line=%d, 8723B IQK \n", __FUNCTION__, __LINE__);
		PHY_IQCalibrate_8723B(ODMPTR, FALSE, FALSE, FALSE, ODM_RF_PATH_A);
	}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT //FOR_8812_IQK
#ifdef USE_OUT_SRC
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
#ifdef DFS
		if (priv->pshare->rf_ft_var.dfs_skip_iqk)
			return;

		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			if ((priv->pshare->rf_ft_var.dfsdelayiqk) &&
					(OPMODE & WIFI_AP_STATE) &&
					!priv->pmib->dot11DFSEntry.disable_DFS &&
					(timer_pending(&priv->ch_avail_chk_timer) ||
					 priv->pmib->dot11DFSEntry.disable_tx))
				return;
		}
#endif
		SAVE_INT_AND_CLI(x);
		priv->pshare->No_RF_Write = 0;
		phy_IQCalibrate_8812A(ODMPTR, priv->pshare->working_channel);
		priv->pshare->No_RF_Write = 1;
		RESTORE_INT(x);
	}
#endif
#endif
/*
*   Disable IQK for 8814 MP chip
*/
#ifdef CONFIG_WLAN_HAL_8814AE //FOR_8814_IQK
#ifdef USE_OUT_SRC
	if (GET_CHIP_VER(priv) == VERSION_8814A) {
		printk("\nIQ Calibrate\n");
		priv->pshare->No_RF_Write = 0;
		PHY_IQCalibrate_8814A(ODMPTR, FALSE);
		priv->pshare->No_RF_Write = 1;
	}
#endif
#endif

#ifdef CONFIG_WLAN_HAL_8822BE //eric-8822 iqk
	if (GET_CHIP_VER(priv) == VERSION_8822B) {
		PHY_IQCalibrate_8822B(ODMPTR, FALSE);
	}
#endif

#ifdef CONFIG_WLAN_HAL_8881A
#ifdef USE_OUT_SRC
	if (GET_CHIP_VER(priv) == VERSION_8881A) {
#ifdef DFS
		if (priv->pshare->rf_ft_var.dfs_skip_iqk)
			return;

		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			if ((priv->pshare->rf_ft_var.dfsdelayiqk) &&
					(OPMODE & WIFI_AP_STATE) &&
					!priv->pmib->dot11DFSEntry.disable_DFS &&
					(timer_pending(&priv->ch_avail_chk_timer) ||
					 priv->pmib->dot11DFSEntry.disable_tx))
				return;
		}
#endif
		SAVE_INT_AND_CLI(x);
		priv->pshare->No_RF_Write = 0;
		phy_IQCalibrate_8821A(ODMPTR);
		priv->pshare->No_RF_Write = 1;
		RESTORE_INT(x);
	}
#endif
#endif

#ifdef CONFIG_WLAN_HAL_8197F
#ifdef USE_OUT_SRC
	if (GET_CHIP_VER(priv) == VERSION_8197F) {
		//PHY_IQCalibrate_8197F(ODMPTR, FALSE);	
		PHY_IQCalibrate_8197F(ODMPTR, FALSE);	

#ifdef CONFIG_RF_DPK_SETTING_SUPPORT
		//if ((ODMPTR->RFCalibrateInfo.bDPPathAOK == 0) && (ODMPTR->RFCalibrateInfo.bDPPathBOK == 0)) {	
			//printk("call %s 97F DPK\n",__FUNCTION__);	
			//u1Byte k;		
			//for (k = 0; k < 3; k++) {		
				//do_dpk_8197f(ODMPTR, TRUE, k);		
				//}		
			//}		

		if ((ODMPTR->RFCalibrateInfo.bDPPathAOK == 1) && (ODMPTR->RFCalibrateInfo.bDPPathBOK == 1))	
			phy_lut_sram_write_8197f(ODMPTR);
#endif		
	}
#endif
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
#ifdef USE_OUT_SRC
	if (GET_CHIP_VER(priv) == VERSION_8192E) {
//		SAVE_INT_AND_CLI(x);
		PHY_IQCalibrate_8192E(ODMPTR, FALSE);
//		RESTORE_INT(x);
	}
#endif
#endif
}


#ifdef ADD_TX_POWER_BY_CMD
static void assign_txpwr_offset(struct rtl8192cd_priv *priv)
{
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[0], priv->pshare->rf_ft_var.txPowerPlus_cck_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[1], priv->pshare->rf_ft_var.txPowerPlus_cck_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[2], priv->pshare->rf_ft_var.txPowerPlus_cck_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_A[3], priv->pshare->rf_ft_var.txPowerPlus_cck_1);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[0], priv->pshare->rf_ft_var.txPowerPlus_cck_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[1], priv->pshare->rf_ft_var.txPowerPlus_cck_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[2], priv->pshare->rf_ft_var.txPowerPlus_cck_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->CCKTxAgc_B[3], priv->pshare->rf_ft_var.txPowerPlus_cck_1);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[0], priv->pshare->rf_ft_var.txPowerPlus_ofdm_18);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[1], priv->pshare->rf_ft_var.txPowerPlus_ofdm_12);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[2], priv->pshare->rf_ft_var.txPowerPlus_ofdm_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[3], priv->pshare->rf_ft_var.txPowerPlus_ofdm_6);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[0], priv->pshare->rf_ft_var.txPowerPlus_ofdm_18);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[1], priv->pshare->rf_ft_var.txPowerPlus_ofdm_12);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[2], priv->pshare->rf_ft_var.txPowerPlus_ofdm_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[3], priv->pshare->rf_ft_var.txPowerPlus_ofdm_6);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[4], priv->pshare->rf_ft_var.txPowerPlus_ofdm_54);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[5], priv->pshare->rf_ft_var.txPowerPlus_ofdm_48);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[6], priv->pshare->rf_ft_var.txPowerPlus_ofdm_36);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_A[7], priv->pshare->rf_ft_var.txPowerPlus_ofdm_24);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[4], priv->pshare->rf_ft_var.txPowerPlus_ofdm_54);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[5], priv->pshare->rf_ft_var.txPowerPlus_ofdm_48);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[6], priv->pshare->rf_ft_var.txPowerPlus_ofdm_36);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->OFDMTxAgcOffset_B[7], priv->pshare->rf_ft_var.txPowerPlus_ofdm_24);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[0], priv->pshare->rf_ft_var.txPowerPlus_mcs_3);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[1], priv->pshare->rf_ft_var.txPowerPlus_mcs_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[2], priv->pshare->rf_ft_var.txPowerPlus_mcs_1);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[3], priv->pshare->rf_ft_var.txPowerPlus_mcs_0);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[0], priv->pshare->rf_ft_var.txPowerPlus_mcs_3);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[1], priv->pshare->rf_ft_var.txPowerPlus_mcs_2);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[2], priv->pshare->rf_ft_var.txPowerPlus_mcs_1);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[3], priv->pshare->rf_ft_var.txPowerPlus_mcs_0);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[4], priv->pshare->rf_ft_var.txPowerPlus_mcs_7);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[5], priv->pshare->rf_ft_var.txPowerPlus_mcs_6);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[6], priv->pshare->rf_ft_var.txPowerPlus_mcs_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[7], priv->pshare->rf_ft_var.txPowerPlus_mcs_4);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[4], priv->pshare->rf_ft_var.txPowerPlus_mcs_7);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[5], priv->pshare->rf_ft_var.txPowerPlus_mcs_6);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[6], priv->pshare->rf_ft_var.txPowerPlus_mcs_5);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[7], priv->pshare->rf_ft_var.txPowerPlus_mcs_4);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[8], priv->pshare->rf_ft_var.txPowerPlus_mcs_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[9], priv->pshare->rf_ft_var.txPowerPlus_mcs_10);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[10], priv->pshare->rf_ft_var.txPowerPlus_mcs_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[11], priv->pshare->rf_ft_var.txPowerPlus_mcs_8);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[8], priv->pshare->rf_ft_var.txPowerPlus_mcs_11);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[9], priv->pshare->rf_ft_var.txPowerPlus_mcs_10);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[10], priv->pshare->rf_ft_var.txPowerPlus_mcs_9);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[11], priv->pshare->rf_ft_var.txPowerPlus_mcs_8);

	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[12], priv->pshare->rf_ft_var.txPowerPlus_mcs_15);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[13], priv->pshare->rf_ft_var.txPowerPlus_mcs_14);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[14], priv->pshare->rf_ft_var.txPowerPlus_mcs_13);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_A[15], priv->pshare->rf_ft_var.txPowerPlus_mcs_12);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[12], priv->pshare->rf_ft_var.txPowerPlus_mcs_15);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[13], priv->pshare->rf_ft_var.txPowerPlus_mcs_14);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[14], priv->pshare->rf_ft_var.txPowerPlus_mcs_13);
	ASSIGN_TX_POWER_OFFSET(priv->pshare->phw->MCSTxAgcOffset_B[15], priv->pshare->rf_ft_var.txPowerPlus_mcs_12);
}
#endif


void reload_txpwr_pg(struct rtl8192cd_priv *priv)
{
	int i;
	
	PHY_ConfigBBWithParaFile(priv, PHYREG_PG);

#if 0 //def HIGH_POWER_EXT_PA
	if (!priv->pshare->rf_ft_var.use_ext_pa)
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8812E) {
			*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_CCK11_CCK1_JAguar));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_Ofdm18_Ofdm6_JAguar));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_A_Ofdm54_Ofdm24_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[0])	= cpu_to_be32(RTL_R32(rTxAGC_A_MCS3_MCS0_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[4])	= cpu_to_be32(RTL_R32(rTxAGC_A_MCS7_MCS4_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[8])	= cpu_to_be32(RTL_R32(rTxAGC_A_MCS11_MCS8_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_A_MCS15_MCS12_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss1Index3_Nss1Index0_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss1Index7_Nss1Index4_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[8]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss2Index5_Nss2Index2_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[16]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss2Index9_Nss2Index6_JAguar));
			*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_B[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_CCK11_CCK1_JAguar));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_Ofdm18_Ofdm6_JAguar));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[4]) = cpu_to_be32(RTL_R32(rTxAGC_B_Ofdm54_Ofdm24_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[0])	= cpu_to_be32(RTL_R32(rTxAGC_B_MCS3_MCS0_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[4])	= cpu_to_be32(RTL_R32(rTxAGC_B_MCS7_MCS4_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[8])	= cpu_to_be32(RTL_R32(rTxAGC_B_MCS11_MCS8_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[12]) = cpu_to_be32(RTL_R32(rTxAGC_B_MCS15_MCS12_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_B[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_Nss1Index3_Nss1Index0_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_B[4]) = cpu_to_be32(RTL_R32(rTxAGC_B_Nss1Index7_Nss1Index4_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_B[8]) = cpu_to_be32(RTL_R32(rTxAGC_B_Nss2Index1_Nss1Index8_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_B[12]) = cpu_to_be32(RTL_R32(rTxAGC_B_Nss2Index5_Nss2Index2_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_B[16]) = cpu_to_be32(RTL_R32(rTxAGC_B_Nss2Index9_Nss2Index6_JAguar));
		} else
#endif
#if defined(CONFIG_WLAN_HAL_8881A)
		if (GET_CHIP_VER(priv)==VERSION_8881A) {
			*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_CCK11_CCK1_JAguar));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_Ofdm18_Ofdm6_JAguar));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_A_Ofdm54_Ofdm24_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[0])	= cpu_to_be32(RTL_R32(rTxAGC_A_MCS3_MCS0_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[4])	= cpu_to_be32(RTL_R32(rTxAGC_A_MCS7_MCS4_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[8])	= cpu_to_be32(RTL_R32(rTxAGC_A_MCS11_MCS8_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_A_MCS15_MCS12_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss1Index3_Nss1Index0_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss1Index7_Nss1Index4_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[8]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss2Index5_Nss2Index2_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[16]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss2Index9_Nss2Index6_JAguar));
		} else
#endif

	{
		// get default Tx AGC offset
		//_TXPWR_REDEFINE ?? CCKTxAgc_A[1] [2] [3] ??
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[0])	= cpu_to_be32(RTL_R32(rTxAGC_A_Mcs03_Mcs00));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[4])	= cpu_to_be32(RTL_R32(rTxAGC_A_Mcs07_Mcs04));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[8])	= cpu_to_be32(RTL_R32(rTxAGC_A_Mcs11_Mcs08));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs15_Mcs12));
		*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_Rate18_06));
		*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_A_Rate54_24));
		*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]) = cpu_to_be32((RTL_R32(rTxAGC_A_CCK11_2_B_CCK11) & 0xffffff00)
				| RTL_R8(rTxAGC_A_CCK1_Mcs32 + 1));

#if defined(CONFIG_RTL_92D_SUPPORT)&& defined(CONFIG_RTL_92D_DMDP)
		if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1) {
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[0])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs03_Mcs00));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[4])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs07_Mcs04));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[8])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs11_Mcs08));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs15_Mcs12));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate18_06));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate54_24));
			*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]) = cpu_to_be32((RTL_R8(rTxAGC_A_CCK11_2_B_CCK11) << 24)
					| (RTL_R32(rTxAGC_B_CCK5_1_Mcs32) >> 8));
		}
#endif

		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[0])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs03_Mcs00));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[4])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs07_Mcs04));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[8])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs11_Mcs08));
		*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[12]) = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs15_Mcs12));
		*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate18_06));
		*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[4]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate54_24));
		*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_B[0]) = cpu_to_be32((RTL_R8(rTxAGC_A_CCK11_2_B_CCK11) << 24)
				| (RTL_R32(rTxAGC_B_CCK5_1_Mcs32) >> 8));

	}

       if (priv->pshare->txpwr_pg_format_abs)
       {
               //CCK
               if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G) {
                       priv->pshare->tgpwr_CCK_new[RF_PATH_A] = priv->pshare->phw->CCKTxAgc_A[0];
                       priv->pshare->tgpwr_CCK_new[RF_PATH_B] = priv->pshare->phw->CCKTxAgc_B[0];
                       for (i=0;i<4;i++) {
                               priv->pshare->phw->CCKTxAgc_A[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_A];                                      
                               priv->pshare->phw->CCKTxAgc_B[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_B];                                      
                       }
               }
               //OFDM
               priv->pshare->tgpwr_OFDM_new[RF_PATH_A] = priv->pshare->phw->OFDMTxAgcOffset_A[4];
               priv->pshare->tgpwr_OFDM_new[RF_PATH_B] = priv->pshare->phw->OFDMTxAgcOffset_B[4];
               for (i=0;i<8;i++) {
                       priv->pshare->phw->OFDMTxAgcOffset_A[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_A];
                       priv->pshare->phw->OFDMTxAgcOffset_B[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_B];
               }

               //HT-1S
               priv->pshare->tgpwr_HT1S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[4];
               priv->pshare->tgpwr_HT1S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[4];
               for (i=0;i<8;i++) {
                       priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
                       priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_B];
               }

               //HT-2S
               priv->pshare->tgpwr_HT2S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[12];
               priv->pshare->tgpwr_HT2S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[12];
               for (i=8;i<16;i++) {
                       priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_A];
                       priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_B];
               }
#ifdef RTK_AC_SUPPORT
               if ((GET_CHIP_VER(priv)==VERSION_8812E) || (GET_CHIP_VER(priv)==VERSION_8881A)) {
                       //VHT-1S
                       priv->pshare->tgpwr_VHT1S_new[RF_PATH_A] = priv->pshare->phw->VHTTxAgcOffset_A[4];
                       priv->pshare->tgpwr_VHT1S_new[RF_PATH_B] = priv->pshare->phw->VHTTxAgcOffset_B[4];
                       //VHT-2S                                
                       priv->pshare->tgpwr_VHT2S_new[RF_PATH_A] = priv->pshare->phw->VHTTxAgcOffset_A[18];
                       priv->pshare->tgpwr_VHT2S_new[RF_PATH_B] = priv->pshare->phw->VHTTxAgcOffset_B[18];

                       for (i=0 ; i<20 ; i++) {
                               if (i<8) {
                                       priv->pshare->phw->VHTTxAgcOffset_A[i] -= priv->pshare->tgpwr_VHT1S_new[RF_PATH_A];
                                       priv->pshare->phw->VHTTxAgcOffset_B[i] -= priv->pshare->tgpwr_VHT1S_new[RF_PATH_B];                       
                               } else if (i==10 || i==11) {
                                       priv->pshare->phw->VHTTxAgcOffset_A[i] = priv->pshare->tgpwr_VHT1S_new[RF_PATH_A] - priv->pshare->phw->VHTTxAgcOffset_A[i];
                                       priv->pshare->phw->VHTTxAgcOffset_B[i] = priv->pshare->tgpwr_VHT1S_new[RF_PATH_B] - priv->pshare->phw->VHTTxAgcOffset_B[i];
                               } else if (i==16 || i==17) {
                                       priv->pshare->phw->VHTTxAgcOffset_A[i] = priv->pshare->tgpwr_VHT2S_new[RF_PATH_A] - priv->pshare->phw->VHTTxAgcOffset_A[i];
                                       priv->pshare->phw->VHTTxAgcOffset_B[i] = priv->pshare->tgpwr_VHT2S_new[RF_PATH_B] - priv->pshare->phw->VHTTxAgcOffset_B[i];
                               } else {
                                       priv->pshare->phw->VHTTxAgcOffset_A[i] -= priv->pshare->tgpwr_VHT2S_new[RF_PATH_A];
                                       priv->pshare->phw->VHTTxAgcOffset_B[i] -= priv->pshare->tgpwr_VHT2S_new[RF_PATH_B];                       
                               }
                       }
               }
#endif
       }

#ifdef TXPWR_LMT
	if (!priv->pshare->rf_ft_var.disable_txpwrlmt) {
		//4 Set Target Power from PG table values
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188E)) //92c_pwrlmt
		{
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G) {
				//CCK
				priv->pshare->tgpwr_CCK_new[RF_PATH_A] = priv->pshare->phw->CCKTxAgc_A[0];				
				priv->pshare->tgpwr_CCK_new[RF_PATH_B] = priv->pshare->phw->CCKTxAgc_B[0];
				//4 Set PG table values to difference
				for (i=0;i<4;i++) { 
					priv->pshare->phw->CCKTxAgc_A[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_A];													
					priv->pshare->phw->CCKTxAgc_B[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_B];								
				}
			}
			//OFDM
			priv->pshare->tgpwr_OFDM_new[RF_PATH_A] = priv->pshare->phw->OFDMTxAgcOffset_A[4];	
			priv->pshare->tgpwr_OFDM_new[RF_PATH_B] = priv->pshare->phw->OFDMTxAgcOffset_B[4];
			for (i=0;i<8;i++) {
				priv->pshare->phw->OFDMTxAgcOffset_A[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_A];				
				priv->pshare->phw->OFDMTxAgcOffset_B[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_B];
			}

			//HT-1S
			priv->pshare->tgpwr_HT1S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[4];				
			priv->pshare->tgpwr_HT1S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[4];
			for (i=0;i<8;i++) {
				priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
				priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_B];
			}

			//HT-2S
			priv->pshare->tgpwr_HT2S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[12];
			priv->pshare->tgpwr_HT2S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[12];
			for (i=8;i<16;i++) {
				priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_A];
				priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_B];
			}
		}
	}
#endif

#ifdef ADD_TX_POWER_BY_CMD
	assign_txpwr_offset(priv);
#endif
}


#ifdef BT_COEXIST		
void bt_coex_init(struct rtl8192cd_priv *priv)
{

	unsigned char H2CCommand[6]={0};
	PHY_SetBBReg(priv, 0x6c0, bMaskDWord, 0x5ddd5ddd);
	PHY_SetBBReg(priv, 0x6c4, bMaskDWord, 0x5fdb5fdb);
	PHY_SetBBReg(priv, 0x6c8, bMaskDWord, 0xffffff);
	PHY_SetBBReg(priv, 0x6cc, bMaskDWord, 0x0);
	PHY_SetBBReg(priv, 0x778, bMaskDWord, 0x1);
	PHY_SetBBReg(priv, 0x790, bMaskDWord, 0x5);
	PHY_SetBBReg(priv, 0x76c, bMaskDWord, 0xc0000);
	PHY_SetBBReg(priv, 0x40, bMaskDWord, 0x200); /* BT reopen issue*/
	PHY_SetBBReg(priv, 0x100, bMaskDWord, 0x314ff);
	PHY_SetBBReg(priv, 0x118, bMaskDWord, 0x107);

	H2CCommand[0] = 0x1;

	FillH2CCmd88XX(priv, 0x71, 6, H2CCommand);

	/* TDMA on 
	H2CCommand[0] = 0xe3;
	H2CCommand[1] = 0x12;
	H2CCommand[2] = 0x12;
	H2CCommand[3] = 0x21;
	H2CCommand[4] = 0x10;
	H2CCommand[5] = 0x0;
	delay_ms(10);
	FillH2CCmd88XX(priv, H2C_88XX_BT_TDMA, 6, H2CCommand);
	*/
}
#endif


#ifdef USE_OUT_SRC

void ODM_software_init(struct rtl8192cd_priv *priv)
{
	unsigned long ability;
	unsigned int	BoardType = ODM_BOARD_DEFAULT;
	priv->pshare->_dmODM.priv = priv;

	//
	// Init Value
	//
	// 1. u1Byte SupportPlatform
	panic_printk("[ODM_software_init] \n");
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_PLATFORM, ODM_AP);
	PHYDM_InitDebugSetting(ODMPTR);

#if 0// defined(CONFIG_RTL_92E_SUPPORT) && defined(CONFIG_PHYDM_ANTENNA_DIVERSITY)	
	ODM_CmnInfoInit(ODMPTR,ODM_CMNINFO_RF_ANTENNA_TYPE,CGCS_RX_HW_ANTDIV);
#endif
	// 2. u4Byte SupportAbility
	ability =	\
			ODM_BB_DIG				|
			ODM_BB_RA_MASK			|
			ODM_BB_FA_CNT			|
			ODM_BB_RATE_ADAPTIVE	|
			ODM_MAC_EDCA_TURBO	|
			ODM_RF_RX_GAIN_TRACK	|
			ODM_RF_CALIBRATION		|
			ODM_BB_DYNAMIC_TXPWR	|
			ODM_RF_TX_PWR_TRACK 	|
			ODM_BB_NHM_CNT		| 
			ODM_BB_PRIMARY_CCA	| 
			0;

#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		ability =	\
			ODM_BB_DIG				|
			ODM_BB_RA_MASK			|
			ODM_BB_FA_CNT			|
			ODM_MAC_EDCA_TURBO	|
			ODM_BB_RSSI_MONITOR 	|
			ODM_BB_DYNAMIC_TXPWR	|
			ODM_RF_TX_PWR_TRACK 	|
			ODM_RF_CALIBRATION		|
			ODM_BB_NHM_CNT		|
			ODM_BB_CCK_PD			|
			0;
	}
#endif

#ifdef CONFIG_RTL_8723B_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8723B) {
		ability =	\
			ODM_BB_DIG				|
			ODM_BB_RA_MASK			|
			ODM_BB_FA_CNT			|
			ODM_MAC_EDCA_TURBO		|
			ODM_BB_RSSI_MONITOR 	|
			ODM_BB_DYNAMIC_TXPWR	|
			ODM_RF_TX_PWR_TRACK 	|
			ODM_RF_CALIBRATION		|
			ODM_BB_NHM_CNT			|
			ODM_BB_CCK_PD			|
			0;
	}
#endif

#ifdef CONFIG_WLAN_HAL_8881A
    	if (GET_CHIP_VER(priv) == VERSION_8881A) {		
        	ability = \
			ODM_BB_DIG          		|
                    ODM_BB_RA_MASK      		|
                    ODM_BB_FA_CNT       		|
                    ODM_MAC_EDCA_TURBO  	|
                    ODM_BB_RSSI_MONITOR 	|
                    ODM_BB_DYNAMIC_TXPWR	|
                    ODM_RF_TX_PWR_TRACK	|
                    ODM_RF_CALIBRATION		|
			ODM_BB_NHM_CNT		|
			ODM_BB_CCK_PD			|
                    0;
	}
#endif

#ifdef CONFIG_WLAN_HAL_8197F
    if (GET_CHIP_VER(priv) == VERSION_8197F) {
        ability =   \
			ODM_BB_DIG				|
			ODM_BB_RA_MASK			|
			ODM_BB_FA_CNT			|
			ODM_BB_RSSI_MONITOR		|
//			ODM_MAC_EDCA_TURBO	|
			ODM_BB_DYNAMIC_TXPWR	|
			ODM_RF_TX_PWR_TRACK 	|
			ODM_RF_CALIBRATION		|
			ODM_BB_CCK_PD			|
			ODM_BB_RATE_ADAPTIVE	|
			ODM_BB_LNA_SAT_CHK		|
			ODM_BB_ADAPTIVE_SOML	|
        0;
    }
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E) {
		ability = \
			ODM_BB_DIG				|
			ODM_BB_RA_MASK		|
			ODM_BB_FA_CNT			|
			ODM_BB_RSSI_MONITOR	|
			ODM_MAC_EDCA_TURBO	|
			ODM_BB_DYNAMIC_TXPWR	|
			ODM_RF_TX_PWR_TRACK	|
			ODM_RF_CALIBRATION		|
			ODM_BB_CCK_PD			|
			ODM_BB_NHM_CNT		|
			0;
	}
#endif

#ifdef CONFIG_WLAN_HAL_8814AE
	if (GET_CHIP_VER(priv) == VERSION_8814A) {
		ability = ODM_BB_DIG			|
			ODM_BB_RA_MASK			|
			ODM_BB_FA_CNT			|
			ODM_BB_RSSI_MONITOR	|
			//ODM_MAC_EDCA_TURBO	|
			ODM_BB_DYNAMIC_TXPWR	|
			ODM_RF_TX_PWR_TRACK		|
//			ODM_RF_CALIBRATION		|
			ODM_BB_CCK_PD			|
			0;
	}
#endif

//eric-8822 ?? odm ability

#ifdef CONFIG_WLAN_HAL_8822BE
		if (GET_CHIP_VER(priv) == VERSION_8822B) {
			ability = \
				ODM_BB_DIG			|
				ODM_BB_RA_MASK			|
				ODM_BB_FA_CNT			|
				ODM_BB_RSSI_MONITOR |
	//			ODM_MAC_EDCA_TURBO	|
				ODM_BB_DYNAMIC_TXPWR	|
				ODM_RF_TX_PWR_TRACK 	|
	//			ODM_RF_CALIBRATION		|
	//			ODM_BB_CCK_PD			|
				ODM_BB_ADAPTIVE_SOML	|
				0;
		}
#endif


	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific) {
		ability = \
			ODM_RF_CALIBRATION 		|
			ODM_RF_TX_PWR_TRACK	|
			ODM_BB_FA_CNT			|
			0;

		if (GET_CHIP_VER(priv) == VERSION_8822B)
			ability |= ODM_BB_DYNAMIC_PSDTOOL;
	}

#if defined(SW_ANT_SWITCH)
	if (priv->pshare->rf_ft_var.antSw_enable)
		ability |= ODM_BB_ANT_DIV;
#endif
#ifdef TX_EARLY_MODE
	ability |= ODM_MAC_EARLY_MODE;
#endif

	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ABILITY, ability);
	ODM_CmnInfoUpdate(ODMPTR, ODM_CMNINFO_ABILITY, ability);

	// 3. u1Byte SupportInterface
#ifdef CONFIG_PCI_HCI
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_INTERFACE, ODM_ITRF_PCIE);
#elif defined(CONFIG_USB_HCI)
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_INTERFACE, ODM_ITRF_USB);
#elif defined(CONFIG_SDIO_HCI)
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_INTERFACE, ODM_ITRF_SDIO);
#endif

	// 4. u4Byte SupportICType
	if (GET_CHIP_VER(priv) == VERSION_8188E)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8188E);
	else if (GET_CHIP_VER(priv) == VERSION_8812E)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8812);
	else if (GET_CHIP_VER(priv) == VERSION_8881A)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8881A);
	else if (GET_CHIP_VER(priv) == VERSION_8192E)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8192E);
	else if (GET_CHIP_VER(priv) == VERSION_8814A)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8814A);
	else if (GET_CHIP_VER(priv) == VERSION_8723B)	
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8723B); 	
	else if (GET_CHIP_VER(priv) == VERSION_8822B) //eric-8822
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8822B);
	else if (GET_CHIP_VER(priv) == VERSION_8197F)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_IC_TYPE, ODM_RTL8197F);

	// 5. u1Byte CutVersion
	if (IS_TEST_CHIP(priv)) {
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_CUT_VER, ODM_CUT_TEST);
	} else {
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_CUT_VER, (RTL_R32(SYS_CFG) >> 12) & 0xF);

		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_MP_TEST_CHIP, 1);
	}	

	// 6. u1Byte FabVersion
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_FAB_VER, ODM_TSMC);

	// 7. u1Byte RFType
	if (get_rf_mimo_mode(priv) == MIMO_1T1R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_1T1R);
	else if (get_rf_mimo_mode(priv) == MIMO_1T2R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_1T2R);
	else if (get_rf_mimo_mode(priv) == MIMO_2T2R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_2T2R);
	//else if (get_rf_mimo_mode(priv) == MIMO_2T2R_GREEN)
		//ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_2T2R_GREEN);
	else if (get_rf_mimo_mode(priv) == MIMO_2T3R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_2T3R);
	else if (get_rf_mimo_mode(priv) == MIMO_2T4R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_2T4R);
	else if (get_rf_mimo_mode(priv) == MIMO_3T3R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_3T3R);
	else if (get_rf_mimo_mode(priv) == MIMO_3T4R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_3T4R);
	else if (get_rf_mimo_mode(priv) == MIMO_4T4R)
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_4T4R);
	else
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RF_TYPE, ODM_XTXR);

	// 8. u1Byte BoardType
#if defined(HIGH_POWER_EXT_PA) && defined(HIGH_POWER_EXT_LNA)	
	if(priv->pshare->rf_ft_var.use_ext_pa && priv->pshare->rf_ft_var.use_ext_lna) { //For 88C/92C only	
		priv->pmib->dot11RFEntry.trswitch = 1;
	}
#endif

#ifdef CONFIG_RTL_8723B_SUPPORT
	if(GET_CHIP_VER(priv) == VERSION_8723B) 
		BoardType |= ODM_BOARD_MINICARD;
#endif

#ifdef HIGH_POWER_EXT_PA
	if(priv->pshare->rf_ft_var.use_ext_pa)
	{
		if(GET_CHIP_VER(priv) == VERSION_8814A) 
			BoardType |= (ODM_BOARD_EXT_LNA|ODM_BOARD_EXT_LNA_5G|ODM_BOARD_EXT_PA|ODM_BOARD_EXT_PA_5G);
		else if((GET_CHIP_VER(priv) == VERSION_8812E)||(GET_CHIP_VER(priv) == VERSION_8881A))
			BoardType |= ODM_BOARD_EXT_PA_5G;
		else //2G Chip: 92C/88E/92E
			BoardType |= (ODM_BOARD_EXT_LNA|ODM_BOARD_EXT_PA);
	}
#endif
#ifdef HIGH_POWER_EXT_LNA
	if(priv->pshare->rf_ft_var.use_ext_lna){
		if((GET_CHIP_VER(priv) == VERSION_8812E)||(GET_CHIP_VER(priv) == VERSION_8881A))
			BoardType |= ODM_BOARD_EXT_LNA_5G;
		else if((GET_CHIP_VER(priv) == VERSION_8188E)||(GET_CHIP_VER(priv) == VERSION_8192E))
			BoardType |= ODM_BOARD_EXT_LNA;
	}
#endif
	if(priv->pmib->dot11RFEntry.trswitch)
		BoardType |= ODM_BOARD_EXT_TRSW;

	if((GET_CHIP_VER(priv) == VERSION_8814A)&&((priv->pmib->dot11RFEntry.rfe_type == 7)||(priv->pmib->dot11RFEntry.rfe_type == 9)))
		BoardType = (ODM_BOARD_EXT_LNA|ODM_BOARD_EXT_LNA_5G); // 8814 new internal
	
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_BOARD_TYPE, BoardType);

#if defined(CONFIG_WLAN_HAL_8197F)
	extern unsigned int rtl819x_bond_option(void);
	if(GET_CHIP_VER(priv) == VERSION_8197F){
		if(rtl819x_bond_option() == BSP_BOND_97FN){
			panic_printk("[97F] Bonding Type 97FN, PKG2\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_PACKAGE_TYPE, 2); /* 97FN */
		}else if(rtl819x_bond_option() == BSP_BOND_97FS){
			panic_printk("[97F] Bonding Type 97FS, PKG1\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_PACKAGE_TYPE, 1); /* 97FS */
		}else if(rtl819x_bond_option() == BSP_BOND_97FB){
			panic_printk("[97F] Bonding Type 97FB, PKG0\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_PACKAGE_TYPE, 0); /* 97FB */
		}else{
			panic_printk("[97F] Bonding Type Unknown, PKG1\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_PACKAGE_TYPE, 1); /* Unknown */
		}
	}
#endif	

	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_RFE_TYPE, priv->pmib->dot11RFEntry.rfe_type);
    
	// ExtLNA & ExtPA Type
	if(GET_CHIP_VER(priv) == VERSION_8812E)
	{
		if(priv->pmib->dot11RFEntry.pa_type == PA_SKYWORKS_5023)
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, 0x05);
		else
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, 0x00);
		
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GPA, 0x00);
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, 0x00);
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, 0x00);	
	}
	else if(GET_CHIP_VER(priv) == VERSION_8814A)
	{		
		if(priv->pmib->dot11RFEntry.rfe_type == 2) {
			panic_printk("PHY paratemters: RFE type 2 APA1+ALNA1+GPA1+GLNA1\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GPA, TYPE_GPA1);  // path (A,B,C,D) = (1,1,1,1)
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, TYPE_APA1);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, TYPE_GLNA1);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, TYPE_ALNA1);	
		} else if(priv->pmib->dot11RFEntry.rfe_type == 3) {
			panic_printk("PHY paratemters: RFE type 3 APA2+ALNA2+GPA2+GLNA2\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GPA, TYPE_GPA2);	// path (A,B,C,D) = (2,2,2,2)
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, TYPE_APA2);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, TYPE_GLNA2);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, TYPE_ALNA2);	
		} else if(priv->pmib->dot11RFEntry.rfe_type == 4) {
			panic_printk("PHY paratemters: RFE type 4 APA1+ALNA3+GPA1+GLNA1\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GPA, TYPE_GPA1); // path (A,B,C,D) = (2,2,2,2)
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, TYPE_APA1);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, TYPE_GLNA1); // path (A,B,C,D) = (3,3,3,3)
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, TYPE_ALNA3);	
		} else if(priv->pmib->dot11RFEntry.rfe_type == 5) {
			panic_printk("PHY paratemters: RFE type 5 APA2+ALNA4+GPA2+GLNA2\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GPA, TYPE_GPA2); 
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, TYPE_APA2);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, TYPE_GLNA2); 
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, TYPE_ALNA4);
		} else if(priv->pmib->dot11RFEntry.rfe_type == 7) {
			panic_printk("PHY paratemters: RFE type 0 ALNA5+GLNA3\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, TYPE_GLNA3);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, TYPE_ALNA5);
		} else if(priv->pmib->dot11RFEntry.rfe_type == 0){
			panic_printk("PHY paratemters: RFE type 0\n");
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GPA, TYPE_GPA0);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, TYPE_APA0);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, TYPE_GLNA0);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, TYPE_ALNA0);	
		}else {
			panic_printk("PHY paratemters: RFE type %d!!\n",priv->pmib->dot11RFEntry.rfe_type);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GPA, TYPE_GPA0);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_APA, TYPE_APA0);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_GLNA, TYPE_GLNA0);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_ALNA, TYPE_ALNA0);	
		}
	}

#ifdef HIGH_POWER_EXT_PA
	//priv->pshare->rf_ft_var.use_ext_lna is useless
	if(priv->pshare->rf_ft_var.use_ext_pa)
	{
		// 9. u1Byte ExtLNA
		// 10. u1Byte ExtPA
		if ((GET_CHIP_VER(priv) == VERSION_8188E)||(GET_CHIP_VER(priv) == VERSION_8192E)){
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_PA, TRUE);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_LNA, TRUE);
		} else {
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_PA, TRUE);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_5G_EXT_PA, TRUE);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_LNA, TRUE);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_5G_EXT_LNA, TRUE);
		}
	}
#endif
#ifdef HIGH_POWER_EXT_LNA
		if(priv->pshare->rf_ft_var.use_ext_lna){
			// 9. u1Byte ExtLNA
			if ((GET_CHIP_VER(priv) == VERSION_8188E)||(GET_CHIP_VER(priv) == VERSION_8192E)){
				ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_LNA, TRUE);
				ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_LNA_GAIN, priv->pshare->rf_ft_var.ext_lna_gain);
			} else{
				ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_LNA, TRUE);
				ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_5G_EXT_LNA, TRUE);
			}
		}
#endif

#ifdef CONFIG_WLAN_HAL_8881A
	if(GET_CHIP_VER(priv) == VERSION_8881A){
		if(get_bonding_type_8881A() == BOND_8881AM){
			if(priv->pshare->rf_ft_var.use_intpa8881A){
				ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_PA, FALSE);
				ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_5G_EXT_PA, FALSE);
			}else{
				ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_PA, TRUE);
				ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_5G_EXT_PA, TRUE);
			}	
		
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_LNA, TRUE);
			ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_5G_EXT_LNA, TRUE);
		}
	}
#endif

	// 11. u1Byte ExtTRSW, ODM_CMNINFO_EXT_TRSW:
	// follows variable "trswitch" which is modified in rtl8192cd_init_hw_PCI().
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_EXT_TRSW, priv->pmib->dot11RFEntry.trswitch);

	// 12. u1Byte PatchID
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_PATCH_ID, 0);

	// 13. BOOLEAN bInHctTest
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_BINHCT_TEST, FALSE);

	// 14. BOOLEAN bWIFITest
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_BWIFI_TEST, (priv->pmib->dot11OperationEntry.wifi_specific > 0));

	// 15. BOOLEAN bDualMacSmartConcurrent
	ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_SMART_CONCURRENT, FALSE);

	// Config BB/RF by ODM
	if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B) || (GET_CHIP_VER(priv) == VERSION_8197F) || (GET_CHIP_VER(priv) == VERSION_8723B)) //eric-8822 ??
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_CONFIG_BB_RF, TRUE);
	else
		ODM_CmnInfoInit(ODMPTR, ODM_CMNINFO_CONFIG_BB_RF, FALSE);

	//
	// Dynamic Value
	//

	// 1. u1Byte *pMacPhyMode
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_MAC_PHY_MODE, &priv->pmib->dot11RFEntry.macPhyMode);

	// 2. u8Byte *pNumTxBytesUnicast
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_TX_UNI, &priv->pshare->NumTxBytesUnicast);

	// 3. u8Byte *pNumRxBytesUnicast
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_RX_UNI, &priv->pshare->NumRxBytesUnicast);

	// 4. u1Byte *pWirelessMode
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_WM_MODE, &priv->pmib->dot11BssType.net_work_type);

	// 5. u1Byte *pBandType
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BAND, &priv->pmib->dot11RFEntry.phyBandSelect);

	// 6. u1Byte *pSecChOffset
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_SEC_CHNL_OFFSET, &priv->pshare->offset_2nd_chan);

	// 7. u1Byte *pSecurity
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_SEC_MODE, &priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm);

	// 8. u1Byte *pBandWidth
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BW, &priv->pshare->CurrentChannelBW);

	// 9. u1Byte *pChannel
//	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_CHNL, &priv->pshare->working_channel);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_CHNL, &priv->pmib->dot11RFEntry.dot11channel);

	// 10. BOOLEAN *pbMasterOfDMSP
//	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_DMSP_IS_MASTER, NULL);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_DMSP_IS_MASTER, &priv->pshare->dummy);


	// 11. BOOLEAN *pbGetValueFromOtherMac
//	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_DMSP_GET_VALUE, NULL);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_DMSP_GET_VALUE, &priv->pshare->dummy);

	// 12. PADAPTER *pBuddyAdapter
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BUDDY_ADAPTOR, NULL);

	// 13. BOOLEAN *pbBTOperation
//	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BT_OPERATION, NULL);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BT_OPERATION, &priv->pshare->dummy);

	// 14. BOOLEAN *pbBTDisableEDCATurbo
//	ODM_CmnInfoHook(pOdm, ODM_CMNINFO_BT_DISABLE_EDCA, NULL);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BT_DISABLE_EDCA, &priv->pshare->dummy);


	// 15. BOOLEAN *pbScanInProcess
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_SCAN, &priv->pshare->bScanInProcess);

	// 16. pU4byte force data rate add by YuChen
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_FORCED_RATE, &priv->pshare->current_tx_rate);

	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_POWER_SAVING, &priv->pshare->dummy);


	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_ONE_PATH_CCA, &priv->pshare->rf_ft_var.one_path_cca);


// dummy

	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_DRV_STOP, &priv->pshare->dummy);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_PNP_IN, &priv->pshare->dummy);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_INIT_ON, &priv->pshare->dummy);
	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_BT_BUSY, &priv->pshare->dummy);
//	ODM_CmnInfoHook(ODMPTR, ODM_CMNINFO_ANT_DIV, &priv->pshare->dummy);

	//For Phy para verison
	ODM_GetHWImgVersion(ODMPTR);


// DM parameters init
//	ODM_DMInit(ODMPTR);

	ODM_InitAllTimers(ODMPTR);

#ifdef TPT_THREAD
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
#ifdef CHECK_HANGUP
	if (!priv->reset_hangup)
#endif
#ifdef SMART_REPEATER_MODE
	if (!priv->pshare->switch_chan_rp)
#endif
		ODM_InitAllThreads(ODMPTR);
#endif
}

#endif

// TODO : TEMP add by Eric , check 8197F need add ?
#ifdef CONFIG_WLAN_HAL_8192EE

unsigned int getval_8b4(struct rtl8192cd_priv *priv, unsigned char rf_path)
{
	unsigned int tmp_8b4 = 0;
	unsigned int delay = priv->pshare->rf_ft_var.delay_8b4;
#if 1
	if (rf_path == RF_PATH_A)
		RTL_W8(0x804, 0x3);
	else if (rf_path == RF_PATH_B)
		RTL_W8(0x804, 0x13);

	RTL_W32(0x808, 0xfccd);
	RTL_W32(0x808, 0x40fccd);
#else
	if(rf_path == RF_PATH_A)
	{
		RTL_W8(0xc50, 0x20);
		PHY_SetRFReg(priv, RF_PATH_A, 0, bRFRegOffsetMask, 0x33e00);
		RTL_W8(0x804, 0x3);
		RTL_W32(0x808, 0xfccd);
		RTL_W32(0x808, 0x40fccd);
	}
	else if(rf_path == RF_PATH_B)
	{
		RTL_W8(0xc58, 0x20);
		PHY_SetRFReg(priv, RF_PATH_B, 0, bRFRegOffsetMask, 0x33e00);
		RTL_W8(0x804, 0x13);
		RTL_W32(0x808, 0xfccd);
		RTL_W32(0x808, 0x40fccd);
	}
#endif
	delay_ms(delay);
	tmp_8b4 =  RTL_R32(0x8b4);
	
	//printk("tmp_8b4[%d] = 0x%x \n", rf_path, tmp_8b4);
	
	return tmp_8b4;
}


void Check_92E_Spur_Valid(struct rtl8192cd_priv * priv, BOOLEAN long_delay)
{
#ifdef CONFIG_PCI_HCI
	unsigned char tmp_loop = 0;
	unsigned int path_a_8b4 = 0, Reg0x804 = 0, Reg0x808 = 0; 
	unsigned int path_b_8b4 = 0; 
//	unsigned long flags;
	unsigned int channel_bak, bw_bak, Offset2nd_bak; 

	watchdog_kick();
//	watchdog_stop(priv);

	Reg0x804 = RTL_R32(0x804);	
	Reg0x808 = RTL_R32(0x808);
	bw_bak = priv->pshare->CurrentChannelBW; 
	Offset2nd_bak =  priv->pshare->offset_2nd_chan;
	channel_bak = priv->pmib->dot11RFEntry.dot11channel;
	//RESTORE_INT(flags);
	
	// Switch to channel 13, bw = 20M	
	SwBWMode(priv, HT_CHANNEL_WIDTH_20, HT_2NDCH_OFFSET_DONTCARE);
	SwChnl(priv, 13, HT_2NDCH_OFFSET_DONTCARE);

	RTL_W8(0xc50, 0x30);
	RTL_W8(0xc58, 0x30);

	//PHY_SetRFReg(priv, RF_PATH_A, 0x0, bMask20Bits, 0x0);
	RTL_W32(0x88c, 0xccf000c0);	//disable 3-wire
	

	path_a_8b4 = getval_8b4(priv, RF92CD_PATH_A);
	path_b_8b4 = getval_8b4(priv, RF92CD_PATH_B);
	
	for(tmp_loop = 0 ; tmp_loop < priv->pshare->rf_ft_var.loop_8b4 ; tmp_loop ++) {
		if (path_a_8b4 > priv->pshare->rf_ft_var.thrd_8b4 || path_b_8b4 > priv->pshare->rf_ft_var.thrd_8b4) {					
			//PLL reset
			if (long_delay)
				delay_ms(10);
			RTL_W8(0x29, 0x7);
			if (long_delay)
				delay_ms(10);
			RTL_W8(0x29, 0x47);

			path_a_8b4 = getval_8b4(priv, RF92CD_PATH_A);
			path_b_8b4 = getval_8b4(priv, RF92CD_PATH_B);			
		} else {		
			break;
		}
#if !defined(CONFIG_OPENWRT_SDK) && !defined(RTK_NL80211)
		watchdog_kick();
#endif
	}

	if (tmp_loop == priv->pshare->rf_ft_var.loop_8b4) {
		priv->pshare->PLL_reset_ok = false;
		panic_printk("\n[Loop#%d] 0x8b4 = 0x%x[A] 0x%x[B], FAIL1 !!\n\n", tmp_loop, path_a_8b4, path_b_8b4);
#if 0
		if (long_delay) {
		for(tmp_loop = 0 ; tmp_loop < priv->pshare->rf_ft_var.loop_8b4 ; tmp_loop ++) {
			if (path_a_8b4 > priv->pshare->rf_ft_var.thrd_8b4 || path_b_8b4 > priv->pshare->rf_ft_var.thrd_8b4) {					
				//PLL reset
				delay_ms(20);
				RTL_W8(0x29, 0x7);
				delay_ms(20);
				RTL_W8(0x29, 0x47);

				path_a_8b4 = getval_8b4(priv, RF92CD_PATH_A);
				path_b_8b4 = getval_8b4(priv, RF92CD_PATH_B);			
			} else {		
				break;
			}		
		}
		
		if (tmp_loop == priv->pshare->rf_ft_var.loop_8b4) {
			priv->pshare->PLL_reset_ok = false;
			printk("\n[Loop#%d] 0x8b4 = 0x%x[A] 0x%x[B], FAIL2 !!\n\n", tmp_loop, path_a_8b4, path_b_8b4);
		} else {		
			priv->pshare->PLL_reset_ok = true;
			printk("\n[Loop#%d] 0x8b4 = 0x%x[A] 0x%x[B], OK2 !!\n\n", tmp_loop+1, path_a_8b4, path_b_8b4);
		}
		}
#endif		
	} else {		
		DEBUG_INFO("\n[Loop#%d] 0x8b4 = 0x%x[A] 0x%x[B], OK !!\n\n", tmp_loop+1, path_a_8b4, path_b_8b4);
		priv->pshare->PLL_reset_ok = true;
	}
	
	RTL_W32(0x88c, 0xcc0000c0);	//enable 3-wire

	RTL_W8(0xc50, 0x20);
	RTL_W8(0xc58, 0x20);
	RTL_W32(0x804, Reg0x804);
	RTL_W32(0x808, Reg0x808);
	
	SwBWMode(priv, bw_bak, Offset2nd_bak);
	SwChnl(priv, channel_bak, Offset2nd_bak);
	//SAVE_INT_AND_CLI(flags);
#endif // CONFIG_PCI_HCI

	if (priv->pmib->dot11RFEntry.dot11channel == 13) {
		PHY_SetBBReg(priv, 0xd18, BIT(27), 1);
		PHY_SetBBReg(priv, 0xd2C, BIT(28), 1);
		PHY_SetBBReg(priv, 0xd40, BIT(26), 1);
	} else {
		PHY_SetBBReg(priv, 0xd18, BIT(27), 0);
		PHY_SetBBReg(priv, 0xd2C, BIT(28), 0);
		PHY_SetBBReg(priv, 0xd40, BIT(26), 0);
	}
	watchdog_kick();

//	watchdog_resume(priv);
}

#endif // CONFIG_WLAN_HAL_8192EE


void set_target_power(struct rtl8192cd_priv *priv)
{	
	int i;
	if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G) {
		//CCK
		priv->pshare->tgpwr_CCK_new[RF_PATH_A] = priv->pshare->phw->CCKTxAgc_A[0];				
		priv->pshare->tgpwr_CCK_new[RF_PATH_B] = priv->pshare->phw->CCKTxAgc_B[0];
		for (i=0;i<4;i++) {
			priv->pshare->phw->CCKTxAgc_A[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_A];													
			priv->pshare->phw->CCKTxAgc_B[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_B];								
		}
	}
	//OFDM
	priv->pshare->tgpwr_OFDM_new[RF_PATH_A] = priv->pshare->phw->OFDMTxAgcOffset_A[4];	
	priv->pshare->tgpwr_OFDM_new[RF_PATH_B] = priv->pshare->phw->OFDMTxAgcOffset_B[4];
	for (i=0;i<8;i++) {
		priv->pshare->phw->OFDMTxAgcOffset_A[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_A];				
		priv->pshare->phw->OFDMTxAgcOffset_B[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_B];
	}

	//HT-1S
	priv->pshare->tgpwr_HT1S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[4];				
	priv->pshare->tgpwr_HT1S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[4];
	for (i=0;i<8;i++) {
		priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
		priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_B];
	}

	//HT-2S
	priv->pshare->tgpwr_HT2S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[12];
	priv->pshare->tgpwr_HT2S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[12];
	for (i=8;i<16;i++) {
		priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_A];
		priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_B];
	}
#ifdef RTK_AC_SUPPORT
	if ((GET_CHIP_VER(priv)==VERSION_8812E) || (GET_CHIP_VER(priv)==VERSION_8881A) ||(GET_CHIP_VER(priv) == VERSION_8814A)) {			
	//VHT-1S
	priv->pshare->tgpwr_VHT1S_new[RF_PATH_A] = priv->pshare->phw->VHTTxAgcOffset_A[4];				
	priv->pshare->tgpwr_VHT1S_new[RF_PATH_B] = priv->pshare->phw->VHTTxAgcOffset_B[4];
	//VHT-2S				
	priv->pshare->tgpwr_VHT2S_new[RF_PATH_A] = priv->pshare->phw->VHTTxAgcOffset_A[18]; 			
	priv->pshare->tgpwr_VHT2S_new[RF_PATH_B] = priv->pshare->phw->VHTTxAgcOffset_B[18];

	for (i=0 ; i<20 ; i++) {
		if (i<8) {
			priv->pshare->phw->VHTTxAgcOffset_A[i] -= priv->pshare->tgpwr_VHT1S_new[RF_PATH_A];
			priv->pshare->phw->VHTTxAgcOffset_B[i] -= priv->pshare->tgpwr_VHT1S_new[RF_PATH_B];					
		} else if (i==10 || i==11) {						
			priv->pshare->phw->VHTTxAgcOffset_A[i] = priv->pshare->tgpwr_VHT1S_new[RF_PATH_A] - priv->pshare->phw->VHTTxAgcOffset_A[i];
			priv->pshare->phw->VHTTxAgcOffset_B[i] = priv->pshare->tgpwr_VHT1S_new[RF_PATH_B] - priv->pshare->phw->VHTTxAgcOffset_B[i];				
		} else if (i==16 || i==17) {						
			priv->pshare->phw->VHTTxAgcOffset_A[i] = priv->pshare->tgpwr_VHT2S_new[RF_PATH_A] - priv->pshare->phw->VHTTxAgcOffset_A[i];
			priv->pshare->phw->VHTTxAgcOffset_B[i] = priv->pshare->tgpwr_VHT2S_new[RF_PATH_B] - priv->pshare->phw->VHTTxAgcOffset_B[i];				
		} else {
			priv->pshare->phw->VHTTxAgcOffset_A[i] -= priv->pshare->tgpwr_VHT2S_new[RF_PATH_A];
			priv->pshare->phw->VHTTxAgcOffset_B[i] -= priv->pshare->tgpwr_VHT2S_new[RF_PATH_B];					
		}
	}
	} 
#endif		
#if 0
	panic_printk("[%s]\n",__FUNCTION__);
	for(i=0;i<4;i++)
		printk("priv->pshare->phw->CCKTxAgc_A[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_A[i]);
	for(i=0;i<8;i++)
		printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_A[i]);
	for(i=0;i<16;i++)
		printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
	for(i=0;i<4;i++)
		printk("priv->pshare->phw->CCKTxAgc_B[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_B[i]);
	for(i=0;i<8;i++)
		printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_B[i]);
	for(i=0;i<16;i++)
		printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);
#endif	

}


#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
void set_target_power_8814(struct rtl8192cd_priv *priv)
{
	int i;
	if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G) {
		priv->pshare->tgpwr_CCK_new[RF_PATH_A] = priv->pshare->phw->CCKTxAgc_A[3]; /* CCK 11M */				
		priv->pshare->tgpwr_CCK_new[RF_PATH_B] = priv->pshare->phw->CCKTxAgc_B[3];
#if defined(CONFIG_WLAN_HAL_8814AE)		
		priv->pshare->tgpwr_CCK_new[RF_PATH_C] = priv->pshare->phw->CCKTxAgc_C[3]; 				
		priv->pshare->tgpwr_CCK_new[RF_PATH_D] = priv->pshare->phw->CCKTxAgc_D[3];		
#endif
		for (i=0;i<4;i++) {
			priv->pshare->phw->CCKTxAgc_A[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_A]; 												
			priv->pshare->phw->CCKTxAgc_B[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_B];
#if defined(CONFIG_WLAN_HAL_8814AE)
			priv->pshare->phw->CCKTxAgc_C[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_C]; 												
			priv->pshare->phw->CCKTxAgc_D[i] -= priv->pshare->tgpwr_CCK_new[RF_PATH_D];
#endif			
		}
	}
	priv->pshare->tgpwr_OFDM_new[RF_PATH_A] = priv->pshare->phw->OFDMTxAgcOffset_A[7]; /* OFDM 54M */
	priv->pshare->tgpwr_OFDM_new[RF_PATH_B] = priv->pshare->phw->OFDMTxAgcOffset_B[7];
#if defined(CONFIG_WLAN_HAL_8814AE)
	priv->pshare->tgpwr_OFDM_new[RF_PATH_C] = priv->pshare->phw->OFDMTxAgcOffset_C[7];
	priv->pshare->tgpwr_OFDM_new[RF_PATH_D] = priv->pshare->phw->OFDMTxAgcOffset_D[7];
#endif
	for (i=0;i<8;i++) {
		priv->pshare->phw->OFDMTxAgcOffset_A[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_A]; 			
		priv->pshare->phw->OFDMTxAgcOffset_B[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_B];
#if defined(CONFIG_WLAN_HAL_8814AE)
		priv->pshare->phw->OFDMTxAgcOffset_C[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_C]; 			
		priv->pshare->phw->OFDMTxAgcOffset_D[i] -= priv->pshare->tgpwr_OFDM_new[RF_PATH_D];		
#endif
	}

	priv->pshare->tgpwr_HT1S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[7]; /* MCS 7 */				
	priv->pshare->tgpwr_HT1S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[7];
#if defined(CONFIG_WLAN_HAL_8814AE)
	priv->pshare->tgpwr_HT1S_new[RF_PATH_C] = priv->pshare->phw->MCSTxAgcOffset_C[7];				
	priv->pshare->tgpwr_HT1S_new[RF_PATH_D] = priv->pshare->phw->MCSTxAgcOffset_D[7];
#endif	
	for (i=0;i<8;i++) {
		priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
		priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_B];
#if defined(CONFIG_WLAN_HAL_8814AE)		
		priv->pshare->phw->MCSTxAgcOffset_C[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_C];
		priv->pshare->phw->MCSTxAgcOffset_D[i] -= priv->pshare->tgpwr_HT1S_new[RF_PATH_D];		
#endif
	}

	priv->pshare->tgpwr_HT2S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[15]; /* MCS 15 */
	priv->pshare->tgpwr_HT2S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[15];
#if defined(CONFIG_WLAN_HAL_8814AE)
	priv->pshare->tgpwr_HT2S_new[RF_PATH_C] = priv->pshare->phw->MCSTxAgcOffset_C[15];
	priv->pshare->tgpwr_HT2S_new[RF_PATH_D] = priv->pshare->phw->MCSTxAgcOffset_D[15];
#endif
	for (i=8;i<16;i++) {
		priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_A];
		priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_B];
#if defined(CONFIG_WLAN_HAL_8814AE)
		priv->pshare->phw->MCSTxAgcOffset_C[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_C];
		priv->pshare->phw->MCSTxAgcOffset_D[i] -= priv->pshare->tgpwr_HT2S_new[RF_PATH_D];		
#endif
	}
#if defined(CONFIG_WLAN_HAL_8814AE)
	priv->pshare->tgpwr_HT3S_new[RF_PATH_A] = priv->pshare->phw->MCSTxAgcOffset_A[23]; /* MCS 23 */
	priv->pshare->tgpwr_HT3S_new[RF_PATH_B] = priv->pshare->phw->MCSTxAgcOffset_B[23];
	priv->pshare->tgpwr_HT3S_new[RF_PATH_C] = priv->pshare->phw->MCSTxAgcOffset_C[23];
	priv->pshare->tgpwr_HT3S_new[RF_PATH_D] = priv->pshare->phw->MCSTxAgcOffset_D[23];
	for (i=16;i<24;i++) {
		priv->pshare->phw->MCSTxAgcOffset_A[i] -= priv->pshare->tgpwr_HT3S_new[RF_PATH_A];
		priv->pshare->phw->MCSTxAgcOffset_B[i] -= priv->pshare->tgpwr_HT3S_new[RF_PATH_B];
		priv->pshare->phw->MCSTxAgcOffset_C[i] -= priv->pshare->tgpwr_HT3S_new[RF_PATH_C];
		priv->pshare->phw->MCSTxAgcOffset_D[i] -= priv->pshare->tgpwr_HT3S_new[RF_PATH_D];		
	}
#endif
	priv->pshare->tgpwr_VHT1S_new[RF_PATH_A] = priv->pshare->phw->VHTTxAgcOffset_A[7]; /* VHT MCS 7 */
	priv->pshare->tgpwr_VHT1S_new[RF_PATH_B] = priv->pshare->phw->VHTTxAgcOffset_B[7];
#if defined(CONFIG_WLAN_HAL_8814AE)
	priv->pshare->tgpwr_VHT1S_new[RF_PATH_C] = priv->pshare->phw->VHTTxAgcOffset_C[7]; 				
	priv->pshare->tgpwr_VHT1S_new[RF_PATH_D] = priv->pshare->phw->VHTTxAgcOffset_D[7];
#endif			
	priv->pshare->tgpwr_VHT2S_new[RF_PATH_A] = priv->pshare->phw->VHTTxAgcOffset_A[17]; /* VHT NSS2 MCS7 */
	priv->pshare->tgpwr_VHT2S_new[RF_PATH_B] = priv->pshare->phw->VHTTxAgcOffset_B[17];
#if defined(CONFIG_WLAN_HAL_8814AE)
	priv->pshare->tgpwr_VHT2S_new[RF_PATH_C] = priv->pshare->phw->VHTTxAgcOffset_C[17];
	priv->pshare->tgpwr_VHT2S_new[RF_PATH_D] = priv->pshare->phw->VHTTxAgcOffset_D[17];
#endif
#if defined(CONFIG_WLAN_HAL_8814AE)
	priv->pshare->tgpwr_VHT3S_new[RF_PATH_A] = priv->pshare->phw->VHTTxAgcOffset_A[27]; /* VHT NSS3 MCS7 */
	priv->pshare->tgpwr_VHT3S_new[RF_PATH_B] = priv->pshare->phw->VHTTxAgcOffset_B[27];
	priv->pshare->tgpwr_VHT3S_new[RF_PATH_C] = priv->pshare->phw->VHTTxAgcOffset_C[27];
	priv->pshare->tgpwr_VHT3S_new[RF_PATH_D] = priv->pshare->phw->VHTTxAgcOffset_D[27];
#endif
	for (i=0 ; i<30 ; i++) {
		if (i < 8) {
			priv->pshare->phw->VHTTxAgcOffset_A[i] -= priv->pshare->tgpwr_VHT1S_new[RF_PATH_A];
			priv->pshare->phw->VHTTxAgcOffset_B[i] -= priv->pshare->tgpwr_VHT1S_new[RF_PATH_B];
#if defined(CONFIG_WLAN_HAL_8814AE)
			priv->pshare->phw->VHTTxAgcOffset_C[i] -= priv->pshare->tgpwr_VHT1S_new[RF_PATH_C];
			priv->pshare->phw->VHTTxAgcOffset_D[i] -= priv->pshare->tgpwr_VHT1S_new[RF_PATH_D];			
#endif
		}else if(i == 8 || i == 9) {	/* VHT NSS1 MCS8,9 */					
			priv->pshare->phw->VHTTxAgcOffset_A[i] = priv->pshare->tgpwr_VHT1S_new[RF_PATH_A] - priv->pshare->phw->VHTTxAgcOffset_A[i];
			priv->pshare->phw->VHTTxAgcOffset_B[i] = priv->pshare->tgpwr_VHT1S_new[RF_PATH_B] - priv->pshare->phw->VHTTxAgcOffset_B[i];				
#if defined(CONFIG_WLAN_HAL_8814AE)
			priv->pshare->phw->VHTTxAgcOffset_C[i] = priv->pshare->tgpwr_VHT1S_new[RF_PATH_C] - priv->pshare->phw->VHTTxAgcOffset_C[i];
			priv->pshare->phw->VHTTxAgcOffset_D[i] = priv->pshare->tgpwr_VHT1S_new[RF_PATH_D] - priv->pshare->phw->VHTTxAgcOffset_D[i];					
#endif
		}else if(i < 18){
			priv->pshare->phw->VHTTxAgcOffset_A[i] -= priv->pshare->tgpwr_VHT2S_new[RF_PATH_A];
			priv->pshare->phw->VHTTxAgcOffset_B[i] -= priv->pshare->tgpwr_VHT2S_new[RF_PATH_B];	
#if defined(CONFIG_WLAN_HAL_8814AE)
			priv->pshare->phw->VHTTxAgcOffset_C[i] -= priv->pshare->tgpwr_VHT2S_new[RF_PATH_C];
			priv->pshare->phw->VHTTxAgcOffset_D[i] -= priv->pshare->tgpwr_VHT2S_new[RF_PATH_D];				
#endif			
		}else if(i == 18 || i == 19) {	 /* VHT NSS2 MCS8,9 */					
			priv->pshare->phw->VHTTxAgcOffset_A[i] = priv->pshare->tgpwr_VHT2S_new[RF_PATH_A] - priv->pshare->phw->VHTTxAgcOffset_A[i];
			priv->pshare->phw->VHTTxAgcOffset_B[i] = priv->pshare->tgpwr_VHT2S_new[RF_PATH_B] - priv->pshare->phw->VHTTxAgcOffset_B[i];				
#if defined(CONFIG_WLAN_HAL_8814AE)
			priv->pshare->phw->VHTTxAgcOffset_C[i] = priv->pshare->tgpwr_VHT2S_new[RF_PATH_C] - priv->pshare->phw->VHTTxAgcOffset_C[i];
			priv->pshare->phw->VHTTxAgcOffset_D[i] = priv->pshare->tgpwr_VHT2S_new[RF_PATH_D] - priv->pshare->phw->VHTTxAgcOffset_D[i];				
#endif
		}
#if defined(CONFIG_WLAN_HAL_8814AE)
		else if(i == 28 || i == 29){   /* VHT NSS3 MCS8,9 */	
			priv->pshare->phw->VHTTxAgcOffset_A[i] = priv->pshare->tgpwr_VHT3S_new[RF_PATH_A] - priv->pshare->phw->VHTTxAgcOffset_A[i];
			priv->pshare->phw->VHTTxAgcOffset_B[i] = priv->pshare->tgpwr_VHT3S_new[RF_PATH_B] - priv->pshare->phw->VHTTxAgcOffset_B[i];				
			priv->pshare->phw->VHTTxAgcOffset_C[i] = priv->pshare->tgpwr_VHT3S_new[RF_PATH_C] - priv->pshare->phw->VHTTxAgcOffset_C[i];
			priv->pshare->phw->VHTTxAgcOffset_D[i] = priv->pshare->tgpwr_VHT3S_new[RF_PATH_D] - priv->pshare->phw->VHTTxAgcOffset_D[i];								
		}else{
			priv->pshare->phw->VHTTxAgcOffset_A[i] -= priv->pshare->tgpwr_VHT3S_new[RF_PATH_A];
			priv->pshare->phw->VHTTxAgcOffset_B[i] -= priv->pshare->tgpwr_VHT3S_new[RF_PATH_B];	
			priv->pshare->phw->VHTTxAgcOffset_C[i] -= priv->pshare->tgpwr_VHT3S_new[RF_PATH_C];
			priv->pshare->phw->VHTTxAgcOffset_D[i] -= priv->pshare->tgpwr_VHT3S_new[RF_PATH_D];				
		}
#endif		
	}
#if 0	
	for(i=0;i<4;i++)
		printk("priv->pshare->phw->CCKTxAgc_A[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_A[i]);
	for(i=0;i<8;i++)
		printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_A[i]);
	for(i=0;i<24;i++)
		printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_A[i]);
	for(i=0;i<30;i++)
		printk("priv->pshare->phw->VHTTxAgcOffset_A[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_A[i]);
	for(i=0;i<4;i++)
		printk("priv->pshare->phw->CCKTxAgc_B[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_B[i]);
	for(i=0;i<8;i++)
		printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_B[i]);
	for(i=0;i<24;i++)
		printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_B[i]);
	for(i=0;i<30;i++)
		printk("priv->pshare->phw->VHTTxAgcOffset_B[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_B[i]);
	for(i=0;i<4;i++)
		printk("priv->pshare->phw->CCKTxAgc_C[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_C[i]);
	for(i=0;i<8;i++)
		printk("priv->pshare->phw->OFDMTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_C[i]);
	for(i=0;i<24;i++)
		printk("priv->pshare->phw->MCSTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_C[i]);
	for(i=0;i<30;i++)
		printk("priv->pshare->phw->VHTTxAgcOffset_C[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_C[i]);
	for(i=0;i<4;i++)
		printk("priv->pshare->phw->CCKTxAgc_D[%d]=%x\n",i,priv->pshare->phw->CCKTxAgc_D[i]);
	for(i=0;i<8;i++)
		printk("priv->pshare->phw->OFDMTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->OFDMTxAgcOffset_D[i]);
	for(i=0;i<24;i++)
		printk("priv->pshare->phw->MCSTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->MCSTxAgcOffset_D[i]);
	for(i=0;i<30;i++)
		printk("priv->pshare->phw->VHTTxAgcOffset_D[%d]=%x\n",i,priv->pshare->phw->VHTTxAgcOffset_D[i]);
#endif	
}
#endif

#ifdef CONFIG_WLAN_HAL_8822BE //eric-8822
void set_8822_trx_regs(struct rtl8192cd_priv *priv)
{
	unsigned tmp32 = 0;
	unsigned tmp8  = 0;
	
	panic_printk("[%s] +++ \n", __FUNCTION__);
	
//0x804[3] = 1
	tmp8 = RTL_R8(0x804);
	tmp8 |= BIT3; 		
	RTL_W8(0x804, tmp8);

//0x73=4
	RTL_W8(0x73, 4);

//0x1704 = 0000 0000
	RTL_W32(0x1704, 0);

//0x1700 = c00f 0038
	RTL_W32(0x1700, 0xc00f0038);

//0x1080[16] = 1
	tmp32 = RTL_R32(0x1080);
	tmp32 |= BIT16; 
	RTL_W32(0x1080, tmp32);

//0x523[7] = 1
	tmp8 = RTL_R8(0x523);
	tmp8 |= BIT7; 
	RTL_W8(0x523, tmp8);
	
	
// ====== special mod by eric-8822 ========

//0x8a4= 7f7f 7f7f (edcca)
	RTL_W32(0x8a4, 0x7f7f7f7f);

//0x524[10] = 1
	tmp32 = RTL_R32(0x524);
	tmp32 |= BIT10;		
	RTL_W32(0x524, tmp32);

//0x55d = 0x10	
	RTL_W8(0x55d, 0x10);

//0x608[28] = 1; Enable PHYST for rssi level
	tmp32 = RTL_R32(0x608);
	tmp32 |= BIT28; 
	RTL_W32(0x608, tmp32);

	RTL_W8(0x60f, 4);

//0x29 = 0xa9 from RF-Brian refine TX EVM
	RTL_W8(0x29, 0xa9);

#if 0 //eric-8822 TX HANG
//0x26= 0xcf Set MAC clock from 40M to 80M
	RTL_W8(0x26, 0xcf);

// Stop MGMT Q
// 0x522 = 0x10
//	RTL_W8(0x522, 0x10);
// 0x300[13] = 1
//	tmp32 = RTL_R32(0x300);
//	tmp32 |= BIT13; 	
//	RTL_W32(0x300, tmp32);
#endif


#if 1 //eric-8822 tp
	RTL_W8(0x420, 0);
	RTL_W16(0x4ca, 0x3b3b);
	RTL_W8(0x455, 0x70);
	RTL_W32(0x4bc, 0);
	#ifdef MBSSID
	if (GET_ROOT(priv)->vap_count == 0)
		RTL_W8(0x5BE,0x08);
	#endif				
	//panic_printk("Disable SOML, Set 0x19a8 form 0x%x to 0x0 \n", RTL_R32(0x19a8));
	//RTL_W32(0x19a8, 0);

	RTL_W8(0x60c, (RTL_R8(0x60c)|0x3f));

	RTL_W8(0x7d4, 0x98);
#endif

#if 1 //eric-mu set REG
	RTL_W8(0x719, 0x82);

	if(priv->pshare->rf_ft_var.mu_retry){
		if((RTL_R8(0x42a) & 0x3f) <= priv->pshare->rf_ft_var.mu_retry)
			RTL_W8(0x42a, (priv->pshare->rf_ft_var.mu_retry+1));

		if((RTL_R8(0x42b) & 0x3f) <= priv->pshare->rf_ft_var.mu_retry)
			RTL_W8(0x42b, (priv->pshare->rf_ft_var.mu_retry+1));
	} else {
		RTL_W8(0x42a, 0xb);
		RTL_W8(0x42b, 0xb);
	}
#endif


	//for MU performance
	//0x528[17] = 1
	tmp32 = RTL_R32(0x528);
	tmp32 |= BIT17;		
	RTL_W32(0x528, tmp32);	
	
#ifdef DEBUG_8822TX
		RTL_W8(0x520, 0x7);
    //panic_printk("\n>>> 0x520=0x%x\n", RTL_R8(0x520));
    RTL_W8(0x515, 0x10);
    //panic_printk("\n>>> 0x515=0x%x\n", RTL_R8(0x515));
    RTL_W8(0x429, 0x20);
    //panic_printk("\n>>> 0x429=0x%x\n", RTL_R8(0x429));
    RTL_W8(NAV_PROT_LEN, 0x40);
    //panic_printk("\n>>> 0x546=0x%x\n\n", RTL_R8(NAV_PROT_LEN));
#endif

#ifdef MP_TEST
	if(priv->pshare->rf_ft_var.mp_specific)
	RTL_W32(0x19a8, 0x00);	// Disable SoftML, SoftML make Rx Sensitivity worse in some cases (from RF-Brian)
#endif

	if (PHY_QueryRFReg(priv, RF92CD_PATH_A, 0xC9, BIT7|BIT6|BIT5|BIT4|BIT3, 1) <= 3) {
		printk("%s:%d 0xC9[7:3]=%d\n", __FUNCTION__, __LINE__, PHY_QueryRFReg(priv, RF92CD_PATH_A, 0xC9, BIT7|BIT6|BIT5|BIT4|BIT3, 1));
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0xCA, BIT19, 0) ;
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0xB2, BIT18|BIT17|BIT16|BIT15|BIT14, 6) ;
	}

	
	// Mod for extFEM MAX input power
	 if((priv->pmib->dot11RFEntry.rfe_type == 1)
	  ||(priv->pmib->dot11RFEntry.rfe_type == 6)
	  ||(priv->pmib->dot11RFEntry.rfe_type == 7)
	  ||(priv->pmib->dot11RFEntry.rfe_type == 9)
	  ||(priv->pmib->dot11RFEntry.rfe_type == 11))
	 RTL_W8(0x847, 0x4d);

//==================================
	
}
#endif

void set_lck_cv(struct rtl8192cd_priv *priv, unsigned char channel)
{
	u1Byte eRFPath, cv=0, ch;
	u1Byte delta[14] = {0,4,4,4,4,4,4,4,4,4,4,4,4,12};
	cv = priv->pshare->rf_ft_var.cv_ch1;
	for(ch=2; ch <= channel; ch++)
		cv += delta[ch-1];
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0xC9, BIT13, 1); /* enable manually setting cv */
	PHY_SetRFReg(priv, RF92CD_PATH_A, 0xB2, 0xff, cv);
	//panic_printk("[%s] center channel=%u,cv=%x\n",__FUNCTION__, channel, cv);
    return;
}

#if defined(CONFIG_WLAN_HAL_8814AE)
void spread_spectrum(struct rtl8192cd_priv *priv)
{
#if defined(CONFIG_WLAN_PCIE_SSC)
#ifdef CONFIG_BAND_2G_ON_WLAN0
	#define WLANIDX	1
#else
	#define WLANIDX	0
#endif
	unsigned char val;
	if(GET_CHIP_VER(priv) == VERSION_8814A){
#ifdef CONFIG_RTL_5G_SLOT_0
		if(priv->pshare->wlandev_idx == (1^WLANIDX)){  //slot1
			printk("Spread Spectrum set in slot1\n");
			REG32(0xb8b21000) = 0x00040101; //PCIe PORT1		
			delay_ms(1);
			val = REG32(0xb8b21000);
			REG32(0xb8b21000) = 0x45990b01; //PCIe PORT1
			delay_ms(1);
			val = REG32(0xb8b21000);
		}else{  //slot 0
			printk("Spread Spectrum set in slot0\n");
			REG32(0xb8b01000) = 0x00040101; //PCIe PORT0
			delay_ms(1);
			val = REG32(0xb8b01000);
			REG32(0xb8b01000) = 0x45990b01; //PCIe PORT0
			delay_ms(1);
			val = REG32(0xb8b01000);
		}
#elif CONFIG_RTL_5G_SLOT_1
		if(priv->pshare->wlandev_idx == (1^WLANIDX)){  //slot 0
			printk("Spread Spectrum set in slot0\n");
			REG32(0xb8b01000) = 0x00040101; //PCIe PORT0
			delay_ms(1);
			val = REG32(0xb8b01000);
			REG32(0xb8b01000) = 0x45990b01; //PCIe PORT0
			delay_ms(1);
			val = REG32(0xb8b01000);
		}else{  //slot1
			printk("Spread Spectrum set in slot1\n");
			REG32(0xb8b21000) = 0x00040101; //PCIe PORT1		
			delay_ms(1);
			val = REG32(0xb8b21000);
			REG32(0xb8b21000) = 0x45990b01; //PCIe PORT1
			delay_ms(1);
			val = REG32(0xb8b21000);
		}
#endif
	}
#endif
}
#endif

// Note: Not all FW version support H2C cmd for disable beacon when edcca on
// 92e -> v28+, 8812 -> v43+, 8881a -> v10+
void set_bcn_dont_ignore_edcca(struct rtl8192cd_priv *priv)
{
	BOOLEAN carrier_sense_enable = FALSE,
			bcn_dont_ignore_edcca = priv->pshare->rf_ft_var.bcn_dont_ignore_edcca,
			adaptivity_enable = priv->pshare->rf_ft_var.adaptivity_enable;

#ifdef USE_OUT_SRC
	if (IS_OUTSRC_CHIP(priv))
		carrier_sense_enable = ODMPTR->Carrier_Sense_enable;
#endif	
	
	if ( carrier_sense_enable || (/*adaptivity_enable &&*/ bcn_dont_ignore_edcca) ) {

		DEBUG_INFO("set bcn care edcca\n");

#if defined(deCONFIG_WLAN_HAL_8881A) || defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_RTL_8197F) || defined(CONFIG_WLAN_HAL_8822BE)
		if ( (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv)== VERSION_8192E) || (GET_CHIP_VER(priv)==VERSION_8814A) || (GET_CHIP_VER(priv)==VERSION_8197F) || (GET_CHIP_VER(priv)==VERSION_8822B)) {
			unsigned char H2CCommand[1]={0};
			GET_HAL_INTERFACE(priv)->FillH2CCmdHandler(priv, H2C_88XX_BCN_IGNORE_EDCCA, 1, H2CCommand);
		}
#endif

#if defined(CONFIG_RTL_8812_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8812E) {
			unsigned char H2CCommand[1]={0};
			FillH2CCmd8812(priv, H2C_8812_BCN_IGNORE_EDCCA, 1, H2CCommand);
		}
#endif

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E) {
		   unsigned int content = 0;
		   unsigned int h2c_cmd_bcn_ignore_edcca = 0xC2;
	
		   content |= h2c_cmd_bcn_ignore_edcca;
		   signin_h2c_cmd(priv, content, 0);
		}
#endif
#endif
	}
}

int rtl8192cd_init_hw_PCI(struct rtl8192cd_priv *priv)
{
	struct wifi_mib *pmib;
	unsigned int opmode;
	unsigned long val32;
	unsigned short val16;
	int i;
#ifdef CONFIG_WLAN_HAL
	unsigned int errorFlag = 0;
#endif
#ifndef SMP_SYNC
	unsigned long x;
#endif
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
	unsigned int fwStatus = 0, dwnRetry = 5;
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	unsigned int	c50_bak, e50_bak;
#endif
#if defined(CONFIG_WLAN_HAL_8814AE)
	unsigned int	back_c50, back_e50, back_1850, back_1a50;
#endif

	SAVE_INT_AND_CLI(x);
	
	pmib = GET_MIB(priv);
	opmode = priv->pmib->dot11OperationEntry.opmode;

	DBFENTER;

//1 For Test, Firmware Downloading

//	MacConfigBeforeFwDownload(priv);

#if 0 	//	==========>> later
	// ===========================================================================================
	// Download Firmware
	// allocate memory for tx cmd packet
	if ((priv->pshare->txcmd_buf = (unsigned char *)kmalloc((LoadPktSize), GFP_ATOMIC)) == NULL) {
		printk("not enough memory for txcmd_buf\n");
		return -1;
	}

	priv->pshare->cmdbuf_phyaddr = get_physical_addr(priv, priv->pshare->txcmd_buf,
								   LoadPktSize, PCI_DMA_TODEVICE);

	if (LoadFirmware(priv) == FALSE) {
//		panic_printk("Load Firmware Fail!\n");
		panic_printk("Load Firmware check!\n");
		return -1;
	} else {
//		delay_ms(20);
		PRINT_INFO("Load firmware successful! \n");
	}
#endif
//	MacConfigAfterFwDownload(priv);

#if 0 // defined(CONFIG_RTL_92D_SUPPORT) && defined(MP_TEST)  // 92D MP DUAL-PHY SETTING
	if (priv->pshare->rf_ft_var.mp_specific && (GET_CHIP_VER(priv) == VERSION_8192D))
		priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_DUALPHY;
	//priv->pmib->dot11RFEntry.macPhyMode = SINGLEMAC_SINGLEPHY;
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		check_chipID_MIMO(priv);
	}
#endif
#if 0 //def CONFIG_RTL_92D_SUPPORT
	if (check_MacPhyMode(priv) < 0) {
		printk("Check macPhyMode Fail!\n");
		return -1;
	}
#endif


#ifdef USE_OUT_SRC
	if (IS_OUTSRC_CHIP(priv))
		ODM_software_init(priv);
#endif
#if defined(CONFIG_WLAN_HAL_8822BE) 
	if(GET_CHIP_VER(priv) == VERSION_8822B){
		if(GET_CHIP_VER_8822(priv) < PHYDM_SUPPORT_8822_CUT_VERSION){
			panic_printk("8822 CHIP VER = (%d %d) use TXT \n", GET_CHIP_VER_8822(priv), PHYDM_SUPPORT_8822_CUT_VERSION);
			ODMPTR->ConfigBBRF = 0;
		}
	}	
#endif


#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {

		unsigned int ClkSel = XTAL_CLK_SEL_40M;	

#if defined(CONFIG_AUTO_PCIE_PHY_SCAN)
		// Get XTAL information from platform
        #if defined(CONFIG_RTL_8881A) || defined(CONFIG_RTL_8197F)
        if(GET_CHIP_VER(priv) == VERSION_8881A || GET_CHIP_VER(priv) == VERSION_8197F) {
    		if ((REG32(0xb8000008) & 0x1000000) != 0x1000000) 
    			 ClkSel = XTAL_CLK_SEL_25M; 
    		else
    			 ClkSel = XTAL_CLK_SEL_40M;
        }        
        #elif defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)		
		if ((REG32(0xb8000008) & 0x2000000) != 0x2000000) 
			 ClkSel = XTAL_CLK_SEL_25M;	
		else
			 ClkSel = XTAL_CLK_SEL_40M;
        #endif
#else //CONFIG_AUTO_PCIE_PHY_SCAN

        #ifdef CONFIG_WLAN_HAL_8197F
    	if(GET_CHIP_VER(priv) == VERSION_8197F) {
            #ifdef CONFIG_PHY_EAT_40MHZ
    		ClkSel = XTAL_CLK_SEL_40M;
            #else	
    		ClkSel = XTAL_CLK_SEL_25M;
            #endif
        }
        else 
        #endif //CONFIG_WLAN_HAL_8197F
        #ifdef CONFIG_WLAN_HAL_8881A
  		if(GET_CHIP_VER(priv) == VERSION_8881A)
  			ClkSel = XTAL_CLK_SEL_40M;
        else
        #endif    
           // independent with platform
        #ifdef CONFIG_PHY_WLAN_EAT_40MHZ	
  		ClkSel = XTAL_CLK_SEL_40M;
        #else
    	ClkSel = XTAL_CLK_SEL_25M;
        #endif //CONFIG_PHY_WLAN_EAT_40MHZ
#endif //defined(CONFIG_AUTO_PCIE_PHY_SCAN)

		if(ClkSel == XTAL_CLK_SEL_25M)
			panic_printk("clock 25MHz\n");
		else if(ClkSel == XTAL_CLK_SEL_40M)
			panic_printk("clock 40MHz\n");
		else
			panic_printk("Unknown Clock Frequency\n");
		if ( RT_STATUS_SUCCESS != GET_HAL_INTERFACE(priv)->InitPONHandler(priv, ClkSel) ) {
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			errorFlag |= DRV_ER_INIT_PON;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			panic_printk("InitPON Failed =>\n");
			#if defined(CONFIG_WLAN_HAL_8814AE)
			errorFlag=0;
			RTL_W8(0x7d, RTL_R8(0x7d) | BIT(6));
			if ( RT_STATUS_SUCCESS != GET_HAL_INTERFACE(priv)->InitPONHandler(priv, ClkSel) ) {
			 	GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
                        	errorFlag |= DRV_ER_INIT_PON;
			}
			#endif
		} else {
		#if defined(CONFIG_WLAN_HAL_8197F) 
				u1Byte	efuse0x3F0;
				EFUSE_DATA efuse_virtual_data;
				load_efuse_data_to_reg();
				if(load_efuse_data(&efuse_virtual_data, 0))
					panic_printk("eFuse read failed!\n");
				else {
						
					if(efuse_virtual_data.special) {
						#if 1
						//for RC check
						if( ( REG32(0xb8b00728) & 0x1f ) != 0x11) {
                                        			extern  int PCIE_reset_procedure_97F(unsigned int PCIeIdx, unsigned int mdioReset);
                                        			if ((PCIE_reset_procedure_97F(0,1))  != 1) {
                                                			
									panic_printk("$$$$\r");
                                                			return -1;
                                        			}
								else {
									if( (REG32(0xb8b10000) >>  16 ) != 0xb822) {
										panic_printk("$$XX\r");
										return -1;
									} 			
								}
						}
						else {
							
							if( (REG32(0xb8b10000) >>  16 ) != 0xb822) {
								panic_printk("$$XX\r");
								return -1;
							} 	


						}
						#endif
					}
				}
		#endif
			
			DEBUG_INFO("InitPON OK\n");
		}

		if ( RT_STATUS_SUCCESS != GET_HAL_INTERFACE(priv)->InitMACHandler(priv) ) {
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			errorFlag |= DRV_ER_INIT_MAC;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			panic_printk("InitMAC Failed\n");
			RESTORE_INT(x);
			return -1;
		} else {
			DEBUG_INFO("InitMAC OK\n");
		}

		//TXPAUSE BCN for 8822B, avoid TX Spur after download FW
		if(priv->pshare->rf_ft_var.lock5d1 
			&& (GET_CHIP_VER(priv) == VERSION_8822B)
			&& (OPMODE & WIFI_AP_STATE)){
			RTL_W8(0x5d1, RTL_R8(0x5d1) | STOP_BCN);
		}

#ifdef CONFIG_PCI_HCI
		if ( RT_STATUS_SUCCESS != GET_HAL_INTERFACE(priv)->InitHCIDMARegHandler(priv) ) {
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			errorFlag |= DRV_ER_INIT_HCIDMA;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			panic_printk("InitHCIDMAReg Failed\n");
		} else {
			DEBUG_INFO("InitHCIDMAReg OK\n");
		}
#endif

#ifdef CONFIG_WLAN_HAL_8881A
		//Filen, according to RLE0538 setting
		if (GET_CHIP_VER(priv) == VERSION_8881A) {
			//set RF register as PI mode
			RTL_W8(0xc00, RTL_R8(0xc00) | BIT(2));
			RTL_W8(0xe00, RTL_R8(0xe00) | BIT(2));
		}
#endif // CONFIG_WLAN_HAL_8881A  
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
#ifdef CONFIG_RTL_8812_SUPPORT
	if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_5){
		unsigned int dwTmp1,dwTmp2,dwTmp3 = 0; 
		dwTmp1 = RTL_R8(0x301);
		dwTmp2 = RTL_R32(0x808);
		dwTmp2 &= ~(BIT(28) | BIT(29));
		RTL_W8(REG_RSV_CTRL_8812, 0x0); // Enable to write page 3 reg
		RTL_W8(0x301,0xff); // Close dma before clock set to 5M				
		RTL_W8(0x303,0x80); 
		RTL_W32 (0x808,dwTmp2);
		dwTmp3 = RTL_R8(0x8);
		dwTmp3 &= ~(BIT(7) | BIT(6));
		dwTmp3 |= BIT(7);	
		RTL_W8(0x8,dwTmp3);
		RTL_W8(0x301,dwTmp1);// rewrite 301 to default
		RTL_W8(REG_RSV_CTRL_8812, 0x0e);
	}
#endif
		if (MacInit(priv) == FALSE) {
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			printk("Mac Init failed!!\n");
			RESTORE_INT(x);
			return -1;
#endif
		}
	}

//	RTL_W32(AGGLEN_LMT, RTL_R32(AGGLEN_LMT)|(0x0F&AGGLMT_MCS15S_Mask)<<AGGLMT_MCS15S_SHIFT
//		|(0x0F&AGGLMT_MCS15_Mask)<<AGGLMT_MCS15_SHIFT);

//	RTL_W8(AGGR_BK_TIME, 0x10);

#if defined(CONFIG_VERIWAVE_CHECK)
	RTL_W8(0x6b7, 0x09);
#endif

	//
	// 2. Initialize MAC/PHY Config by MACPHY_reg.txt
	//
/*
#if defined(CONFIG_MACBBRF_BY_ODM) && defined(CONFIG_RTL_88E_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8188E)	{
		ODM_ConfigMACWithHeaderFile(ODMPTR);
	} else
#endif
*/
	set_rf_path_num(priv);	
#ifdef CONFIG_WLAN_HAL
	if ( IS_HAL_CHIP(priv)) {
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_NUM_TOTAL_RF_PATH, (pu1Byte)&priv->pshare->phw->NumTotalRFPath);
	} 
#endif


#ifdef CONFIG_WLAN_HAL_8822BE //eric-8822
	if(GET_CHIP_VER(priv) == VERSION_8822B) {
		//prepare to init 8822B PHY setting
		priv->pshare->phw->NumTotalRFPath = 2;
		config_phydm_parameter_init(ODMPTR, ODM_PRE_SETTING);

		//disable loop back mode
		if (!priv->pmib->miscEntry.drvmac_lb)
		RTL_W8(0x103, 0); 
	}
#endif


#ifdef CONFIG_WLAN_HAL_8197F
	if(GET_CHIP_VER(priv) == VERSION_8197F) {
		config_phydm_parameter_8197f_init(ODMPTR, ODM_PRE_SETTING);
	}
#endif

#ifdef USE_OUT_SRC
	if (IS_OUTSRC_CHIP(priv) && ODMPTR->ConfigBBRF) //eric-?? will 8822 go here ??
	{
	   ODM_ConfigMACWithHeaderFile(ODMPTR);
	}
	else
#endif
	{
		panic_printk("\n\n************* Initialize MAC/PHY parameter *************\n");
		if (PHY_ConfigMACWithParaFile(priv) < 0) {
			#ifdef  CONFIG_WLAN_HAL
			if (IS_HAL_CHIP(priv)) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
				errorFlag |= DRV_ER_INIT_MACPHYREGFILE;
				GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			}
			#endif
			printk("Initialize MAC/PHY Config failure\n");
			RESTORE_INT(x);
			return -1;
		}
		
	} 
	if(IS_C_CUT_8192E(priv))
	{
		if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_5 
			|| priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_10)
					RTL_W8(0x303,0x80); 
	}

	//
	// 3. Initialize BB After MAC Config PHY_reg.txt, AGC_Tab.txt
	//

#ifdef DRVMAC_LB
	if (!priv->pmib->miscEntry.drvmac_lb)
#endif
	{
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
			RESTORE_INT(x);			
			val32 = phy_BB88XX_Config_ParaFile(priv);
			SAVE_INT_AND_CLI(x);
			if (val32) {
				GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
				errorFlag |= DRV_ER_INIT_BBEGFILE;
				GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
				printk("Initialize phy_BB88XX_Config_ParaFile failure\n");
			}
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		val32 = phy_BB8192CD_Config_ParaFile(priv);
		if (val32) {
			RESTORE_INT(x);
			return -1;
		}
	}

	if(GET_CHIP_VER(priv) == VERSION_8192E){//for 92e only
		if (priv->pmib->dot11nConfigEntry.dot11nLDPC &1)	// Rx LDPC
			RTL_W8(0x913, RTL_R8(0x913)|0x02);
		else
			RTL_W8(0x913, RTL_R8(0x913)&~0x02);
	}

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		if (priv->pmib->dot11RFEntry.xcap != 0xff) {
			PHY_SetBBReg(priv, 0x24, 0xF0, priv->pmib->dot11RFEntry.xcap & 0x0F);
			PHY_SetBBReg(priv, 0x28, 0xF0000000, ((priv->pmib->dot11RFEntry.xcap & 0xF0) >> 4));
		}
	}
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if ((GET_CHIP_VER(priv) == VERSION_8188E)  || (GET_CHIP_VER(priv) == VERSION_8723B)){ 
		if (priv->pmib->dot11RFEntry.xcap > 0 && priv->pmib->dot11RFEntry.xcap < 0x3F) {
			int org_val, tmp_val;
			org_val = (RTL_R32(0x24) >> 11) & 0x3F;
			if (org_val > priv->pmib->dot11RFEntry.xcap) {
				for (tmp_val=org_val; tmp_val>=priv->pmib->dot11RFEntry.xcap; tmp_val--) {
					PHY_SetBBReg(priv, 0x24, BIT(11) | BIT(12) | BIT(13) | BIT(14) | BIT(15) | BIT(16), tmp_val & 0x3F);
					PHY_SetBBReg(priv, 0x24, BIT(17) | BIT(18) | BIT(19) | BIT(20) | BIT(21) | BIT(22), tmp_val & 0x3F);
				}
			}	
			else {
				for (tmp_val=org_val; tmp_val<=priv->pmib->dot11RFEntry.xcap; tmp_val++) {
					PHY_SetBBReg(priv, 0x24, BIT(11) | BIT(12) | BIT(13) | BIT(14) | BIT(15) | BIT(16), tmp_val & 0x3F);
					PHY_SetBBReg(priv, 0x24, BIT(17) | BIT(18) | BIT(19) | BIT(20) | BIT(21) | BIT(22), tmp_val & 0x3F);
				}
			}
		}
	}
#endif

#if defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8881A)
	if ((GET_CHIP_VER(priv)== VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A)) {
		if (priv->pmib->dot11RFEntry.xcap > 0 && priv->pmib->dot11RFEntry.xcap < 0x3F) {
			int org_val, tmp_val;
			org_val = (RTL_R32(0x2C) >> 12) & 0x3F;
			if (org_val > priv->pmib->dot11RFEntry.xcap) {
				for (tmp_val=org_val; tmp_val>=priv->pmib->dot11RFEntry.xcap; tmp_val--) {
					PHY_SetBBReg(priv, 0x2C, BIT(12) | BIT(13) | BIT(14) | BIT(15) | BIT(16) | BIT(17), tmp_val & 0x3F);
					PHY_SetBBReg(priv, 0x2C, BIT(18) | BIT(19) | BIT(20) | BIT(21) | BIT(22) | BIT(23), tmp_val & 0x3F);
				}
			}	
			else {
				for (tmp_val=org_val; tmp_val<=priv->pmib->dot11RFEntry.xcap; tmp_val++) {
					PHY_SetBBReg(priv, 0x2C, BIT(12) | BIT(13) | BIT(14) | BIT(15) | BIT(16) | BIT(17), tmp_val & 0x3F);
					PHY_SetBBReg(priv, 0x2C, BIT(18) | BIT(19) | BIT(20) | BIT(21) | BIT(22) | BIT(23), tmp_val & 0x3F);
				}
			}
		}
	}
#endif


#if defined(CONFIG_WLAN_HAL_8197F)||defined(CONFIG_WLAN_HAL_8822BE)
	if ((GET_CHIP_VER(priv) == VERSION_8197F)||(GET_CHIP_VER(priv) == VERSION_8822B)) {
			int org_val, tmp_val;
			org_val = (RTL_R32(0x24) >> 25) & 0x3F;
		if (GET_CHIP_VER(priv) == VERSION_8197F && priv->pmib->dot11RFEntry.share_xcap){

				for (tmp_val=org_val; tmp_val<=0x3F; tmp_val++) {
					PHY_SetBBReg(priv, 0x24, BIT(30) | BIT(29) | BIT(28) | BIT(27) | BIT(26) | BIT(25), tmp_val);
					PHY_SetBBReg(priv, 0x28, BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1), tmp_val);
				}

		}else{
			if (priv->pmib->dot11RFEntry.xcap > 0 && priv->pmib->dot11RFEntry.xcap < 0x3F) {	
				if (GET_CHIP_VER(priv) == VERSION_8822B)
					PHY_SetBBReg(priv, 0x10, 0x00000040, 0x01); // control by WiFi
					
				if (org_val > priv->pmib->dot11RFEntry.xcap) {
					for (tmp_val=org_val; tmp_val>=priv->pmib->dot11RFEntry.xcap; tmp_val--) {
						PHY_SetBBReg(priv, 0x24, BIT(30) | BIT(29) | BIT(28) | BIT(27) | BIT(26) | BIT(25), tmp_val & 0x3F);
						PHY_SetBBReg(priv, 0x28, BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1), tmp_val & 0x3F);
					}
				}else {
					for (tmp_val=org_val; tmp_val<=priv->pmib->dot11RFEntry.xcap; tmp_val++) {
						PHY_SetBBReg(priv, 0x24, BIT(30) | BIT(29) | BIT(28) | BIT(27) | BIT(26) | BIT(25), tmp_val & 0x3F);
						PHY_SetBBReg(priv, 0x28, BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1), tmp_val & 0x3F);
					}
				}
			}
		}
	}
#endif

#if defined(CONFIG_RTL_8812_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		if (priv->pmib->dot11RFEntry.xcap > 0 && priv->pmib->dot11RFEntry.xcap < 0x3F) {
			int org_val, tmp_val;
			org_val = (RTL_R32(0x2C) >> 19) & 0x3F;
			if (org_val > priv->pmib->dot11RFEntry.xcap) {
				for (tmp_val=org_val; tmp_val>=priv->pmib->dot11RFEntry.xcap; tmp_val--) {
					PHY_SetBBReg(priv, 0x2C, BIT(19) | BIT(20) | BIT(21) | BIT(22) | BIT(23) | BIT(24), tmp_val & 0x3F);
					PHY_SetBBReg(priv, 0x2C, BIT(25) | BIT(26) | BIT(27) | BIT(28) | BIT(29) | BIT(30), tmp_val & 0x3F);
				}
			}	
			else {
				for (tmp_val=org_val; tmp_val<=priv->pmib->dot11RFEntry.xcap; tmp_val++) {
					PHY_SetBBReg(priv, 0x2C, BIT(19) | BIT(20) | BIT(21) | BIT(22) | BIT(23) | BIT(24), tmp_val & 0x3F);
					PHY_SetBBReg(priv, 0x2C, BIT(25) | BIT(26) | BIT(27) | BIT(28) | BIT(29) | BIT(30), tmp_val & 0x3F);
				}
			}
		}

		// chang initial gain to avoid loading wrong AGC table, by Arthur
		c50_bak = RTL_R8(0xc50);
		e50_bak = RTL_R8(0xe50);

		RTL_W8(0xc50, 0x22);
		RTL_W8(0xe50, 0x22);

		RTL_W8(0xc50, c50_bak);
		RTL_W8(0xe50, e50_bak);
	}
#endif
#if defined(CONFIG_WLAN_HAL_8814AE)
	if (GET_CHIP_VER(priv) == VERSION_8814A) {
		if (priv->pmib->dot11RFEntry.xcap > 0 && priv->pmib->dot11RFEntry.xcap < 0x3F) {
			int org_val, tmp_val;
			org_val = (RTL_R32(0x2C) >> 15) & 0x3F;
			if (org_val > priv->pmib->dot11RFEntry.xcap) {
				for (tmp_val=org_val; tmp_val>=priv->pmib->dot11RFEntry.xcap; tmp_val--) {
					PHY_SetBBReg(priv, 0x2C, 0x1f8000, tmp_val & 0x3F); // 0x2c[20:15]
					PHY_SetBBReg(priv, 0x2C, 0x7e00000, tmp_val & 0x3F); // 0x2c[26:21]
				}
			}	
			else {
				for (tmp_val=org_val; tmp_val<=priv->pmib->dot11RFEntry.xcap; tmp_val++) {
					PHY_SetBBReg(priv, 0x2C, 0x1f8000, tmp_val & 0x3F); // 0x2c[20:15]
					PHY_SetBBReg(priv, 0x2C, 0x7e00000, tmp_val & 0x3F); // 0x2c[26:21]
				}
			}
		}

		// chang initial gain to avoid loading wrong AGC table, by Arthur
		back_c50 = RTL_R8(0xc50);
		back_e50 = RTL_R8(0xe50);
		back_1850 = RTL_R8(0x1850);
		back_1a50 = RTL_R8(0x1a50);

		RTL_W8(0xc50, 0x22);
		RTL_W8(0xe50, 0x22);
		RTL_W8(0x1850, 0x22);
		RTL_W8(0x1a50, 0x22);

		RTL_W8(0xc50, back_c50);
		RTL_W8(0xe50, back_e50);
		RTL_W8(0x1850, back_1850);
		RTL_W8(0x1a50, back_1a50);
	}
#endif
	
#if defined(CONFIG_WLAN_HAL_8814AE)
	if (GET_CHIP_VER(priv) == VERSION_8814A){
		if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G){
#if 0 /* Enable 2R CCA for CCK Rx @ path B, 2nd CCA @ path C*/
			PHY_SetBBReg(priv,0xA00,0x8000,0x0); //0xA01[7] = 1'b0  turn off antenna diversity
			PHY_SetBBReg(priv,0xA2C,BIT18,1); //0xA2e[6,2] = 2'b11 enable 2R CCA
			PHY_SetBBReg(priv,0xA2C,BIT22,1);
#else /* Disable 2RCCA for CCK Rx */
			PHY_SetBBReg(priv,0xA2C,BIT18,0); // disable 2R CCA
			PHY_SetBBReg(priv,0xA2C,BIT22,0);
#endif
		}
	}
#endif		

	// support up to MCS7 for 1T1R, modify rx capability here
	/*
	if (get_rf_mimo_mode(priv) == MIMO_1T1R)
		pmib->dot11nConfigEntry.dot11nSupportedMCS &= 0x00ff;

	*/
	/*
		// Set NAV protection length
		// CF-END Threshold
		if (priv->pmib->dot11OperationEntry.wifi_specific) {
			RTL_W16(NAV_PROT_LEN, 0x80);
	//		RTL_W8(CFEND_TH, 0x2);
		}
		else {
			RTL_W16(NAV_PROT_LEN, 0x01C0);
	//		RTL_W8(CFEND_TH, 0xFF);
		}
	*/
	//
	// 4. Initiailze RF RAIO_A.txt RF RAIO_B.txt
	//
	// 2007/11/02 MH Before initalizing RF. We can not use FW to do RF-R/W.
	// close loopback, normal mode

	// For RF test only from Scott's suggestion
//	RTL_W8(0x27, 0xDB);	//	==========>> ???
//	RTL_W8(0x1B, 0x07); // ACUT

	/*
		// set RCR: RX_SHIFT and disable ACF
	//	RTL_W8(0x48, 0x3e);
	//	RTL_W32(0x48, RTL_R32(0x48) & ~ RCR_ACF  & ~RCR_ACRC32);
		RTL_W16(RCR, RCR_AAP | RCR_APM | RCR_ACRC32);
		RTL_W32(RCR, RTL_R32(RCR) & ~ RCR_ACF  & ~RCR_ACRC32);
		// for debug by victoryman, 20081119
	//	RTL_W32(RCR, RTL_R32(RCR) | RCR_APP_PHYST_RXFF);
		RTL_W32(RCR, RTL_R32(RCR) | RCR_APP_PHYSTS);
	*/
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
#if defined(RTL8192D_INT_PA)
		u8 c9 = priv->pmib->dot11RFEntry.trsw_pape_C9;
		u8 cc = priv->pmib->dot11RFEntry.trsw_pape_CC;
		if ((c9 == 0xAA && cc == 0xA0) ||	// Type 2 : 5G TRSW+Int. PA, 2G TR co-matched
				(c9 == 0xAA && cc == 0xAF) || 	// Type 3 : 5G SP3TSW+ Ext.(or Int.)PA, 2G TR co-matched
				(c9 == 0x00 && cc == 0xA0)) {	// Type 4 : 5G TRSW+ Ext( or Int.)PA, 2G TRSW+ Ext.(or Int.)PA

			panic_printk("\n**********************************\n");
			panic_printk("\n** NOTE!! RTL8192D INTERNAL PA! **\n");
			panic_printk("\n**********************************\n");


			priv->pshare->rf_ft_var.use_intpa92d = 1;
			priv->pshare->phw->InternalPA5G[0] = 1;
			priv->pshare->phw->InternalPA5G[1] = 1;
		} else {
			// if  using default setting, set as external PA for safe.
			priv->pshare->rf_ft_var.use_intpa92d = 0;
			priv->pmib->dot11RFEntry.trsw_pape_C9 = 0x00;
			priv->pmib->dot11RFEntry.trsw_pape_CC = 0xFF;
			priv->pshare->phw->InternalPA5G[0] = 0;
			priv->pshare->phw->InternalPA5G[1] = 0;
		}
#else
		// to ignore flash setting for external PA
		priv->pmib->dot11RFEntry.trsw_pape_C9 = 0x00;
		priv->pmib->dot11RFEntry.trsw_pape_CC = 0xFF;
		priv->pshare->phw->InternalPA5G[0] = 0;
		priv->pshare->phw->InternalPA5G[1] = 0;
#endif
	}
#endif

	RESTORE_INT(x);

#ifdef DRVMAC_LB
	if (priv->pmib->miscEntry.drvmac_lb) {
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
			RT_OP_MODE  OP_Mode = RT_OP_MODE_NO_LINK;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_MEDIA_STATUS, (pu1Byte)&OP_Mode);
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{//not HAL
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
		}
		drvmac_loopback(priv);
	} else
#endif
    {

#ifdef  CONFIG_WLAN_HAL
        if (IS_HAL_CHIP(priv)) {
#ifdef CONFIG_WLAN_HAL_8822BE //eric-8822
            if(GET_CHIP_VER(priv) == VERSION_8822B){
                //Power ON 8822 RF
                RTL_W8(0x1f, 0x0);//for B cut
                RTL_W8(0x1f, 0x7); 
                RTL_W8(0xef, 0x0);//for B cut
                RTL_W8(0xef, 0x7);
            }
#endif  
          
#ifdef CONFIG_WLAN_HAL_8197F 
			if(GET_CHIP_VER(priv) == VERSION_8197F){
				RTL_W8(0x7B, 0x0);
				RTL_W8(0x7B, 0x7); 
			}
#endif

			if(ODMPTR->ConfigBBRF)
            {

                val32 = ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_A);
                val32 |= ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_B);	
                if(GET_CHIP_VER(priv) == VERSION_8814A){
                    val32 |= ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_C);			
                    val32 |= ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_D);
                }

            }	
            else
                val32 = phy_RF6052_Config_ParaFile(priv);

            if (val32) {
                GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
                errorFlag |= DRV_ER_INIT_PHYRF;
                GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
            }
            if (val32)
                return -1;

            priv->pshare->No_RF_Write = 1;

            #ifdef CONFIG_WLAN_HAL_8822BE //eric-8822
            //Finish 8822B PHY Setting
            if(GET_CHIP_VER(priv) == VERSION_8822B)
                config_phydm_parameter_init(ODMPTR, ODM_POST_SETTING);
            #endif

            #ifdef CONFIG_WLAN_HAL_8197F
            if(GET_CHIP_VER(priv) == VERSION_8197F) {
                config_phydm_parameter_8197f_init(ODMPTR, ODM_POST_SETTING);
				if (PHY_QueryRFReg(priv, RF92CD_PATH_A, 0xb4, bMask20Bits, 1)==0x140a0){
					delay_ms(1);
					RTL_W8(0x7A,0);
				}
				RTL_W8(0x7d4,0x90);
            }
            #endif


            #if defined(CONFIG_WLAN_HAL_8814AE)
            if (GET_CHIP_VER(priv) == VERSION_8814A)
                priv->pshare->phw->NumTotalRFPath = 4;
            #endif	
            
        }
        else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
        {//not HAL

#ifdef USE_OUT_SRC
            if(ODMPTR->ConfigBBRF)
            {
                val32 = ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_A);
                val32 |= ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_B);
                val32 |= ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_C);
                val32 |= ODM_ConfigRFWithHeaderFile(ODMPTR, CONFIG_RF_RADIO, ODM_RF_PATH_D);
            }
            else
#endif					
                val32 = phy_RF6052_Config_ParaFile(priv);

            if (GET_CHIP_VER(priv) == VERSION_8188E)
                priv->pshare->phw->NumTotalRFPath = 1;
            else
                priv->pshare->phw->NumTotalRFPath = 2; //8812A

            #ifdef CONFIG_RTL_8812_SUPPORT 		 //eric_test
            if (GET_CHIP_VER(priv) == VERSION_8812E)
                priv->pshare->No_RF_Write = 1;
            #endif
            
            if (val32)
                return -1;
				
            SAVE_INT_AND_CLI(x);

#ifdef CONFIG_RTL_92C_SUPPORT
            if (IS_UMC_A_CUT_88C(priv))	{
                PHY_SetRFReg(priv, RF92CD_PATH_A, RF_RX_G1, bMask20Bits, 0x30255);
                PHY_SetRFReg(priv, RF92CD_PATH_A, RF_RX_G2, bMask20Bits, 0x50a00);
            } else if (IS_UMC_B_CUT_88C(priv))	{

                PHY_SetRFReg(priv, RF92CD_PATH_A, 0x1e, bMask20Bits, 0x03 | (PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x1e, bMaskDWord, 1) & 0xff0f0));
                PHY_SetRFReg(priv, RF92CD_PATH_A, 0x1f, bMask20Bits, 0x200 | (PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x1f, bMaskDWord, 1) & 0xff0ff));
            }
#endif
            RESTORE_INT(x);
        }
    }
	

	SAVE_INT_AND_CLI(x);
	
#if defined(CONFIG_WLAN_HAL_8814AE)
	if(GET_CHIP_VER(priv) == VERSION_8814A){
		GET_HAL_INTERFACE(priv)->PHYConfigBB(priv);
	}
#endif

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
		//Do nothing
		//MacConfig() is integrated in code below
		//Don't enable BB here
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			unsigned int eRFPath, curMaxRFPath = ((priv->pmib->dot11RFEntry.macPhyMode == SINGLEMAC_SINGLEPHY) ? RF92CD_PATH_MAX : RF92CD_PATH_B);
			for (eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++) {
				priv->pshare->RegRF18[eRFPath] = PHY_QueryRFReg(priv, eRFPath, 0x18, bMask20Bits, 1);
				priv->pshare->RegRF28[eRFPath] = PHY_QueryRFReg(priv, eRFPath, 0x28, bMask20Bits, 1);
			}
			UpdateBBRFVal8192DE(priv);
		}
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
		if (GET_CHIP_VER(priv) != VERSION_8812E)
#endif
		{
			/*---- Set CCK and OFDM Block "ON"----*/
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 1);
#if defined(CONFIG_RTL_92D_SUPPORT)
			if (!(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G))
#endif
				PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 1);

			MacConfig(priv);
		}

		/*
		 *	Force CCK CCA for High Power products
		 */
#ifdef HIGH_POWER_EXT_LNA
		if (priv->pshare->rf_ft_var.use_ext_lna)
			RTL_W8(0xa0a, 0xcd);
#endif		
	}

//	RTL_W8(BW_OPMODE, BIT(2)); // 40Mhz:0 20Mhz:1
//	RTL_W32(MACIDR,0x0);

	// under loopback mode
//	RTL_W32(MACIDR,0xffffffff);		//	need to confirm
	/*
	#ifdef CONFIG_NET_PCI
		if (IS_PCIBIOS_TYPE)
			pci_unmap_single(priv->pshare->pdev, priv->pshare->cmdbuf_phyaddr,
				(LoadPktSize), PCI_DMA_TODEVICE);
	#endif
	*/
#if	0
//	RTL_W32(0x230, 0x40000000);	//for test
////////////////////////////////////////////////////////////

	printk("init_hw: 1\n");

	RTL_W16(SIFS_OFDM, 0x1010);
	RTL_W8(SLOT_TIME, 0x09);

	RTL_W8(MSR, MSR_AP);

//	RTL_W8(MSR,MSR_INFRA);
	// for test, loopback
//	RTL_W8(MSR, MSR_NOLINK);
//	RTL_W8(LBKMD_SEL, BIT(0)| BIT(1) );
//	RTL_W16(LBDLY, 0xffff);

	//beacon related
	RTL_W16(BCN_INTERVAL, pmib->dot11StationConfigEntry.dot11BeaconPeriod);
	RTL_W16(ATIMWND, 2); //0
	RTL_W16(BCN_DRV_EARLY_INT, 10 << 4); // 2
	RTL_W16(BCN_DMATIME, 256); // 0xf
	RTL_W16(SIFS_OFDM, 0x0e0e);
	RTL_W8(SLOT_TIME, 0x10);

//	CamResetAllEntry(priv);
	RTL_W16(SECR, 0x0000);

// By H.Y. advice
//	RTL_W16(_BCNTCFG_, 0x060a);
//	if (OPMODE & WIFI_AP_STATE)
//		RTL_W16(BCNTCFG, 0x000a);
//	else
// for debug
//	RTL_W16(_BCNTCFG_, 0x060a);
//	RTL_W16(BCNTCFG, 0x0204);

	init_beacon(priv);

	priv->pshare->InterruptMask = (IMR_ROK | IMR_VODOK | IMR_VIDOK | IMR_BEDOK | IMR_BKDOK |		\
								   IMR_HCCADOK | IMR_MGNTDOK | IMR_COMDOK | IMR_HIGHDOK | 					\
								   IMR_BDOK | /*IMR_RXCMDOK | IMR_TIMEOUT0 |*/ IMR_RDU | IMR_RXFOVW	|			\
								   IMR_BcnInt/* | IMR_TXFOVW*/ /*| IMR_TBDOK | IMR_TBDER*/) ;// IMR_ROK | IMR_BcnInt | IMR_RDU | IMR_RXFOVW | IMR_RXCMDOK;

//	priv->pshare->InterruptMask = IMR_ROK| IMR_BDOK | IMR_BcnInt | IMR_MGNTDOK | IMR_TBDOK | IMR_RDU ;
//	priv->pshare->InterruptMask  = 0;
	priv->pshare->InterruptMaskExt = 0;
	RTL_W32(IMR, priv->pshare->InterruptMask);
	RTL_W32(IMR + 4, priv->pshare->InterruptMaskExt);

//////////////////////////////////////////////////////////////
	printk("end of init hw\n");

	return 0;

#endif
// clear TPPoll
//	RTL_W16(TPPoll, 0x0);
// Should 8192SE do this initialize? I don't know yet, 080812. Joshua
	// PJ 1-5-2007 Reset PHY parameter counters
//	RTL_W32(0xD00, RTL_R32(0xD00)|BIT(27));
//	RTL_W32(0xD00, RTL_R32(0xD00)&(~(BIT(27))));
	/*
		// configure timing parameter
		RTL_W8(ACK_TIMEOUT, 0x30);
		RTL_W8(PIFS_TIME,0x13);
	//	RTL_W16(LBDLY, 0x060F);
	//	RTL_W16(SIFS_OFDM, 0x0e0e);
	//	RTL_W8(SLOT_TIME, 0x10);
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
			RTL_W16(SIFS_OFDM, 0x0a0a);
			RTL_W8(SLOT_TIME, 0x09);
		}
		else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
			RTL_W16(SIFS_OFDM, 0x0a0a);
			RTL_W8(SLOT_TIME, 0x09);
		}
		else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
			RTL_W16(SIFS_OFDM, 0x0a0a);
			RTL_W8(SLOT_TIME, 0x09);
		}
		else { // WIRELESS_11B
			RTL_W16(SIFS_OFDM, 0x0a0a);
			RTL_W8(SLOT_TIME, 0x14);
		}
	*/

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
		RT_OP_MODE      OP_Mode;
		MACCONFIG_PARA  MacCfgPara;
		u2Byte          beaconPeriod;

		if (opmode & WIFI_AP_STATE) {
			DEBUG_INFO("AP-mode enabled...\n");
#if defined(CONFIG_RTK_MESH)	//Mesh Mode but mesh not enable
			if (priv->pmib->dot11WdsInfo.wdsPure || priv->pmib->dot1180211sInfo.meshSilence )
#else
			if (priv->pmib->dot11WdsInfo.wdsPure)
#endif
			{
				OP_Mode = RT_OP_MODE_NO_LINK;
			} else {
				OP_Mode = RT_OP_MODE_AP;
			}
		} else if (opmode & WIFI_STATION_STATE) {
			DEBUG_INFO("Station-mode enabled...\n");
			OP_Mode = RT_OP_MODE_INFRASTRUCTURE;
		} else if (opmode & WIFI_ADHOC_STATE) {
			DEBUG_INFO("Adhoc-mode enabled...\n");
			OP_Mode = RT_OP_MODE_IBSS;
		} else {
			printk("Operation mode error!\n");
			RESTORE_INT(x);
			return 2;
		}

		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_MEDIA_STATUS, (pu1Byte)&OP_Mode);

		MacCfgPara.AckTO                = priv->pmib->miscEntry.ack_timeout;
		MacCfgPara.vap_enable           = GET_ROOT(priv)->pmib->miscEntry.vap_enable;
		MacCfgPara.OP_Mode              = OP_Mode;
		MacCfgPara.dot11DTIMPeriod      = priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod;		
		MacCfgPara.WdsPure      		= priv->pmib->dot11WdsInfo.wdsPure;
		beaconPeriod                    = pmib->dot11StationConfigEntry.dot11BeaconPeriod;

		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_BEACON_INTERVAL, (pu1Byte)&beaconPeriod);
		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_MAC_CONFIG, (pu1Byte)&MacCfgPara);
	}
#endif

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
	if (!IS_OUTSRC_CHIP(priv))
#endif
		init_EDCA_para(priv, pmib->dot11BssType.net_work_type);
#endif

	// we don't have EEPROM yet, Mark this for FPGA Platform
//	RTL_W8(_9346CR_, CR9346_CFGRW);

//	92SE Windows driver does not set the PCIF as 0x77, seems HW bug?
	// Set Tx and Rx DMA burst size
//	RTL_W8(PCIF, 0x77);
	// Enable byte shift
//	RTL_W8(_PCIF_+2, 0x01);

	/*
		// Retry Limit
		if (priv->pmib->dot11OperationEntry.dot11LongRetryLimit)
			val16 = priv->pmib->dot11OperationEntry.dot11LongRetryLimit & 0xff;
		else {
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
				val16 = 0x30;
			else
				val16 = 0x06;
		}
		if (priv->pmib->dot11OperationEntry.dot11ShortRetryLimit)
			val16 |= ((priv->pmib->dot11OperationEntry.dot11ShortRetryLimit & 0xff) << 8);
		else {
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
				val16 |= (0x30 << 8);
			else
				val16 |= (0x06 << 8);
		}
		RTL_W16(RETRY_LIMIT, val16);

		This should be done later, but Windows Driver not done yet.
		// Response Rate Set
		// let ACK sent by highest of 24Mbps
		val32 = 0x1ff;
		if (pmib->dot11RFEntry.shortpreamble)
			val32 |= BIT(23);
		RTL_W32(_RRSR_, val32);
	*/




//	panic_printk("0x2c4 = bitmap = 0x%08x\n", (unsigned int)val32);
//	panic_printk("0x2c0 = cmd | macid | band = 0x%08x\n", 0xfd0000a2 | (1<<9 | (sta_band & 0xf))<<8);
//	panic_printk("Add id %d val %08x to ratr\n", 0, (unsigned int)val32);

	/*	for (i = 0; i < 8; i++)
			RTL_W32(ARFR0+i*4, val32 & 0x1f0ff0f0);
	*/

	//settting initial rate for control frame to 24M
//	RTL_W8(INIRTSMCS_SEL, 8);	//	==========>> later

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
		u8  MulticastAddr[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_MULTICAST_REG, (pu1Byte)MulticastAddr);

		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_BSSID, (pu1Byte)pmib->dot11OperationEntry.hwaddr);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
		//setting MAR
		RTL_W32(MAR, 0xffffffff);
		RTL_W32((MAR + 4), 0xffffffff);

		//setting BSSID, not matter AH/AP/station
		memcpy((void *)&val32, (pmib->dot11OperationEntry.hwaddr), 4);
		memcpy((void *)&val16, (pmib->dot11OperationEntry.hwaddr + 4), 2);
		RTL_W32(BSSIDR, cpu_to_le32(val32));
		RTL_W16((BSSIDR + 4), cpu_to_le16(val16));
		//	RTL_W32(BSSIDR, 0x814ce000);
		//	RTL_W16((BSSIDR + 4), 0xee92);
	}

	//setting TCR
	//TCR, use default value

	//setting RCR // set in MacConfigAfterFwDownload
//	RTL_W32(_RCR_, _APWRMGT_ | _AMF_ | _ADF_ | _AICV_ | _ACRC32_ | _AB_ | _AM_ | _APM_);
//	if (pmib->dot11OperationEntry.crc_log)
//		RTL_W32(_RCR_, RTL_R32(_RCR_) | _ACRC32_);

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
		//3 Integrated into SetHwRegHandler(priv, HW_VAR_MAC_CONFIG, &MacCfgPara);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
		// setting network type
		if (opmode & WIFI_AP_STATE) {
			DEBUG_INFO("AP-mode enabled...\n");

#if defined(CONFIG_RTK_MESH)	//Mesh Mode but mesh not enable
			if (priv->pmib->dot11WdsInfo.wdsPure || priv->pmib->dot1180211sInfo.meshSilence )
#else
			if (priv->pmib->dot11WdsInfo.wdsPure)
#endif
			{
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
				if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
					//WDEBUG("pure WDS mode\n");
					RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_INFRA & NETYPE_Mask) << NETYPE_SHIFT));
				} else
#endif
				{
					RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
				}

			} else {
				RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_AP & NETYPE_Mask) << NETYPE_SHIFT));
			}
			// Move init beacon after f/w download
#if 0
			if (priv->auto_channel == 0) {
				DEBUG_INFO("going to init beacon\n");
				init_beacon(priv);
			}
#endif
		}
#ifdef CLIENT_MODE
		else if (opmode & WIFI_STATION_STATE) {
			DEBUG_INFO("Station-mode enabled...\n");
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_INFRA & NETYPE_Mask) << NETYPE_SHIFT));
		} else if (opmode & WIFI_ADHOC_STATE) {
			DEBUG_INFO("Adhoc-mode enabled...\n");
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_ADHOC & NETYPE_Mask) << NETYPE_SHIFT));
		}
#endif
		else {
			printk("Operation mode error!\n");
			RESTORE_INT(x);
			return 2;
		}
	}


	//3 Security Related

	CamResetAllEntry(priv);
#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
		SECURITY_CONFIG_OPERATION  SCO = 0;
		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
		RTL_W16(SECCFG, 0);
	}
	if ((OPMODE & (WIFI_AP_STATE | WIFI_STATION_STATE | WIFI_ADHOC_STATE)) &&
			!priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm &&
			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
			 pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)) {
		pmib->dot11GroupKeysTable.dot11Privacy = pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
		if (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)
			i = 5;
		else
			i = 13;
#ifdef USE_WEP_DEFAULT_KEY
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
			   &priv->pmib->dot11DefaultKeysTable.keytype[pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex].skey[0], i);
		pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen = i;
		pmib->dot11GroupKeysTable.keyid = pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		pmib->dot11GroupKeysTable.keyInCam = 0;
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
			SECURITY_CONFIG_OPERATION  SCO;
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
			SCO |= SCO_NOSKMC;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{//not HAL
			RTL_W16(SECCFG, RTL_R16(SECCFG) | NOSKMC);	// no search multicast
		}
#else
#if defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
			   &priv->pmib->dot11DefaultKeysTable.keytype[pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex].skey[0], i);
#else
		memcpy(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
			   &priv->pmib->dot11DefaultKeysTable.keytype[0].skey[0], i);
#endif
		pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen = i;
		pmib->dot11GroupKeysTable.keyid = pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		pmib->dot11GroupKeysTable.keyInCam = 0;
#endif
	}

// for debug
#if 0
// when hangup reset, re-init TKIP/AES key in ad-hoc mode
#ifdef CLIENT_MODE
	if ((OPMODE & WIFI_ADHOC_STATE) && pmib->dot11OperationEntry.keep_rsnie &&
			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_ ||
			 pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_)) {
		DOT11_SET_KEY Set_Key;
		Set_Key.KeyType = DOT11_KeyType_Group;
		Set_Key.EncType = pmib->dot11GroupKeysTable.dot11Privacy;
		DOT11_Process_Set_Key(priv->dev, NULL, &Set_Key, pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey);
	} else
#endif
//-------------------------------------- david+2006-06-30
		// restore group key if it has been set before open, david
		if (pmib->dot11GroupKeysTable.keyInCam) {
			int retVal;
			retVal = CamAddOneEntry(priv, (unsigned char *)"\xff\xff\xff\xff\xff\xff",
									pmib->dot11GroupKeysTable.keyid,
									pmib->dot11GroupKeysTable.dot11Privacy << 2,
									0, pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey);
			if (retVal)
				priv->pshare->CamEntryOccupied++;
			else {
				DEBUG_ERR("Add group key failed!\n");
			}
		}
#endif
	//here add if legacy WEP
	// if 1x is enabled, do not set default key, david
#ifdef USE_WEP_DEFAULT_KEY
#ifdef MBSSID
	if (!(OPMODE & WIFI_AP_STATE) || !priv->pmib->miscEntry.vap_enable)
#endif
	{
		if (!SWCRYPTO && !IEEE8021X_FUN &&
				(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_ ||
				 pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_))
			init_DefaultKey_Enc(priv, NULL, 0);
	}
#endif


	//3 MAC Beacon Tming Related

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
		//Do Nothing
		// Integrated into code above
		//3      Integrated into SetHwRegHandler(priv, HW_VAR_MAC_CONFIG, &MacCfgPara);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL

		//Setup Beacon Interval/interrupt interval, ATIM-WIND ATIM-Interrupt
		RTL_W32(MBSSID_BCN_SPACE, pmib->dot11StationConfigEntry.dot11BeaconPeriod);
		//Setting BCNITV is done by firmware now
//	set_fw_reg(priv, (0xF1000000 | (pmib->dot11StationConfigEntry.dot11BeaconPeriod << 8)), 0, 0);
		// Set max AMPDU aggregation time
//	int max_aggre_time = 0xc0; // in 4us
//	set_fw_reg(priv, (0xFD0000B1|((max_aggre_time << 8) & 0xff00)), 0 ,0);

//	RTL_W32(0x2A4, 0x00006300);
//	RTL_W32(0x2A0, 0xb026007C);
//	delay_ms(1);
//	while(RTL_R32(0x2A0) != 0){};
		//RTL_W16(TBTT_PROHIBIT, 0xc804);
		
        if((OPMODE & WIFI_AP_STATE ))
        {   // for pure AP mode + repeater mode + APWDS + Mbssid    
    		if ( (priv->pmib->miscEntry.vap_enable) || (priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod <= 40)){
    			RTL_W32(TBTT_PROHIBIT, 0x1df04);
#if defined(CONFIG_RTL_88E_SUPPORT) && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
		if (GET_CHIP_VER(priv) == VERSION_8188E)
			RTL_W16(ATIMWND, 18);
#endif
    		}else{
    			RTL_W32(TBTT_PROHIBIT, 0x40004);
#if defined(CONFIG_RTL_88E_SUPPORT) && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
		if (GET_CHIP_VER(priv) == VERSION_8188E)
			RTL_W16(ATIMWND, 35);
#endif
            }
            // for pure WDS
            if((priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsPure)) // pure wds mode
            {
               RTL_W32(TBTT_PROHIBIT, 0x104);
            }            
       }else{
            // for STA mode only
            RTL_W32(TBTT_PROHIBIT, 0x4004); //tbtt=2ms
       }
	   
	   
		RTL_W8(DRVERLYINT, 10);
		RTL_W8(BCNDMATIM, 1);
		/*
			if (priv->pshare->rf_ft_var.bcast_to_dzq)
				RTL_W16(ATIMWND, 0x0a);
			else
				RTL_W16(ATIMWND, 5);
		*/

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
			RTL_W16(ATIMWND, 0x10);
			RTL_W8(REG_92E_DTIM_COUNT_ROOT, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
			RTL_W32(REG_92E_PKT_LIFETIME_CTRL, RTL_R32(REG_92E_PKT_LIFETIME_CTRL) & ~BIT(19));
		} else
#endif
		{
#if defined(CONFIG_PCI_HCI)
			RTL_W16(ATIMWND, 1);
#endif
		}

		/*
			if (!((OPMODE & WIFI_AP_STATE)
		#if defined(WDS) && defined(CONFIG_RTK_MESH)
				&& ((priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsPure) || priv->pmib->dot1180211sInfo.meshSilence))
		#elif defined(WDS)
				&& priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsPure )
		#elif defined(CONFIG_RTK_MESH)	//Mesh Mode but mesh not enable
				&& priv->pmib->dot1180211sInfo.meshSilence )
		#else
				)
		#endif
			)

				RTL_W16(BCN_DRV_EARLY_INT, RTL_R16(BCN_DRV_EARLY_INT)|BIT(15)); // sw beacon
		*/
#ifdef MBSSID
		if (priv->pmib->miscEntry.vap_enable && RTL8192CD_NUM_VWLAN == 1 &&
				priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod < 30)
			//RTL_W16(BCN_DRV_EARLY_INT, (RTL_R16(BCN_DRV_EARLY_INT)&0xf00f) | ((6<<4)&0xff0));
			RTL_W8(DRVERLYINT, 6);
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B)
			RTL_W8(REG_92E_PRE_DL_BCN_ITV, RTL_R8(DRVERLYINT) + 1);
#endif

#ifdef CONFIG_RTL_92C_SUPPORT
		if (IS_TEST_CHIP(priv) && ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C))) {
			RTL_W8(BCN_CTRL, 0);
			RTL_W8(0x553, 1);
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION);
			//	RTL_W16(BCN_DMATIME, 0x400); // 1ms

			// for debug
#ifdef CLIENT_MODE
			if (OPMODE & WIFI_ADHOC_STATE)
				RTL_W8(BCN_MAX_ERR, 0xff);
#endif
		} else
#endif
		{
			RTL_W8(BCN_CTRL, DIS_TSF_UPDATE_N |  DIS_SUB_STATE_N  );
			RTL_W8(BCN_MAX_ERR, 0xff);
			RTL_W16(0x518, 0);
			RTL_W8(0x553, 3);
			if (OPMODE & WIFI_STATION_STATE)
				RTL_W8(0x422, RTL_R8(0x422) ^ BIT(6));

			if ((priv->pmib->dot11WdsInfo.wdsPure == 0)
#ifdef MP_TEST
					&& (!priv->pshare->rf_ft_var.mp_specific)
#endif
			   ) {
				RTL_W8(BCN_CTRL, RTL_R8(BCN_CTRL) | EN_BCN_FUNCTION | EN_TXBCN_RPT);
			} else {

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
				if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
					//WDEBUG("pure WDS mode\n");
					RTL_W8(BCN_CTRL, 0);
				}
#endif
			}
		}


//--------------

// By H.Y. advice
//	RTL_W16(_BCNTCFG_, 0x060a);
		if (OPMODE & WIFI_AP_STATE)
			RTL_W16(BCNTCFG, 0x320c);	//RTL_W16(BCNTCFG, 0x000a);
		else
// for debug
//	RTL_W16(_BCNTCFG_, 0x060a);
			RTL_W16(BCNTCFG, 0x0204);
	}

	//3 IMR Related
	//3 Download Firmware Related
	// Ack ISR, and then unmask IMR
#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
		if (opmode & WIFI_AP_STATE) {
			GET_HAL_INTERFACE(priv)->InitIMRHandler(priv, RT_OP_MODE_AP);
		} else if (opmode & WIFI_STATION_STATE) {
			GET_HAL_INTERFACE(priv)->InitIMRHandler(priv, RT_OP_MODE_INFRASTRUCTURE);
		} else if (opmode & WIFI_ADHOC_STATE) {
			GET_HAL_INTERFACE(priv)->InitIMRHandler(priv, RT_OP_MODE_IBSS);
		}

	GET_HAL_INTERFACE(priv)->AddInterruptMaskHandler(priv, HAL_INT_TYPE_PSTIMEOUT2);

        #if defined(SW_TX_QUEUE) || defined(RTK_ATM)
        GET_HAL_INTERFACE(priv)->AddInterruptMaskHandler(priv, HAL_INT_TYPE_GTIMER4);
        #endif
		// TODO: Filen, no need to sync !?
		priv->pshare->InterruptMask = _GET_HAL_DATA(priv)->IntMask[0];
		priv->pshare->InterruptMask = _GET_HAL_DATA(priv)->IntMask[1];

		// TODO: Filen, check download 8051 firmware
		//Download Firmware
		
	if(priv->pshare->rf_ft_var.load_fw){
		RESTORE_INT(x);
    if(!IS_HARDWARE_MACHAL_SUPPORT(priv))
    {
		if ( RT_STATUS_SUCCESS != GET_HAL_INTERFACE(priv)->InitFirmwareHandler(priv)) {
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			errorFlag |= DRV_ER_INIT_DLFW;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
			panic_printk("InitDownload FW Fail\n");
		} else {
			DEBUG_INFO("InitDownload FW OK \n");
#ifdef AP_PS_Offlaod
            priv->APPS_phase = 0; //0:phase1, 1:phase2
            GET_HAL_INTERFACE(priv)->SetAPPSOffloadHandler(priv,1, priv->APPS_phase, 4, 1);
#endif
		}
    }
  
#if defined(CONFIG_WLAN_HAL_8192EE)		
		RTL_W8(DRVERLYINT, 10);	// v24 92e fw overwrire DRVERLYINT
		RTL_W8(FWHW_TXQ_CTRL + 1, 0x0F); // v24 92e fw overwrire FWHW_TXQ_CTRL
#endif		
		SAVE_INT_AND_CLI(x);
#if defined(CONFIG_WLAN_HAL_8814AE)
		//To prevent FW hangup
		RTL_W8(0xf0, 0x3f);
#endif
		}else
		panic_printk("\n[%s] Not To Use FW\n",__FUNCTION__);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
#if 1
#ifdef CONFIG_PCI_HCI
		// Ack ISR, and then unmask IMR
#if 0
		RTL_W32(ISR, RTL_R32(ISR));
		RTL_W32(ISR + 4, RTL_R16(ISR + 4));
		RTL_W32(IMR, 0x0);
		RTL_W32(IMR + 4, 0x0);
		priv->pshare->InterruptMask = _ROK_ | _BCNDMAINT_ | _RDU_ | _RXFOVW_ | _RXCMDOK_;
		priv->pshare->InterruptMask = (IMR_ROK | IMR_VODOK | IMR_VIDOK | IMR_BEDOK | IMR_BKDOK |		\
									   IMR_HCCADOK | IMR_MGNTDOK | IMR_COMDOK | IMR_HIGHDOK | 					\
									   IMR_BDOK | IMR_RXCMDOK | /*IMR_TIMEOUT0 |*/ IMR_RDU | IMR_RXFOVW	|			\
									   IMR_BcnInt/* | IMR_TXFOVW*/ /*| IMR_TBDOK | IMR_TBDER*/);// IMR_ROK | IMR_BcnInt | IMR_RDU | IMR_RXFOVW | IMR_RXCMDOK;
#endif
		//priv->pshare->InterruptMask = HIMR_ROK | HIMR_BCNDMA0 | HIMR_RDU | HIMR_RXFOVW;

#ifdef CONFIG_RTL_88E_SUPPORT
        if (GET_CHIP_VER(priv) == VERSION_8188E) {
            priv->pshare->InterruptMask = HIMR_88E_ROK | HIMR_88E_HISR1_IND_INT;
            priv->pshare->InterruptMaskExt = HIMRE_88E_RXFOVW;
        } else
#endif
#if defined(CONFIG_RTL_8812_SUPPORT)  || defined(CONFIG_RTL_8723B_SUPPORT)
        if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
            priv->pshare->InterruptMask = HIMR_92E_ROK | HIMR_92E_HISR1_IND_INT;
            priv->pshare->InterruptMaskExt = HIMRE_92E_RXFOVW;
            #if (BEAMFORMING_SUPPORT == 1) 
            if (priv->pmib->dot11RFEntry.txbf == 1) {
                priv->pshare->InterruptMask |= IMR_GTINT3_8812;
            }
            #endif

            #if defined(SW_TX_QUEUE) || defined(RTK_ATM)
            priv->pshare->InterruptMask |= IMR_GTINT4_8812;
            #endif
        } else
#endif
        {
            priv->pshare->InterruptMask = HIMR_ROK | HIMR_BCNDMA0 | HIMR_RXFOVW;
            priv->pshare->InterruptMaskExt = 0;
        }

#ifdef MP_TEST
		if (priv->pshare->rf_ft_var.mp_specific) {
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8188E)
				priv->pshare->InterruptMask |= HIMR_88E_BEDOK;
			else
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
				if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B)
					priv->pshare->InterruptMask |= IMR_BEDOK_8812;
				else
#endif
#ifdef CONFIG_WLAN_HAL
					//3 Integrated into HAL code
#endif
					priv->pshare->InterruptMask	|= HIMR_BEDOK;
		}
#endif

		if (opmode & WIFI_AP_STATE) {
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8188E)
				priv->pshare->InterruptMask |= HIMR_88E_BcnInt | HIMR_88E_TBDOK | HIMR_88E_TBDER;
			else
#endif
#if defined(CONFIG_RTL_92E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
				if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8723B)) {
					if (priv->pmib->dot11WdsInfo.wdsPure) {
						//WDEBUG("pure-WDS mode don't enable HIMR_92E_BcnInt | HIMR_92E_TBDOK | HIMR_92E_TBDER \n");
					} else {
						priv->pshare->InterruptMask |= HIMR_92E_BcnInt | HIMR_92E_TBDOK | HIMR_92E_TBDER;
					}
				} else
#endif
				{
					priv->pshare->InterruptMask |= HIMR_BCNDOK0 | HIMR_TXBCNERR;
				}
		}
#ifdef CLIENT_MODE
		else if (opmode & WIFI_ADHOC_STATE) {
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8188E)
				priv->pshare->InterruptMask |= HIMR_88E_BcnInt | HIMR_88E_TBDOK | HIMR_88E_TBDER;
			else
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
				if (GET_CHIP_VER(priv) == VERSION_8812E || (GET_CHIP_VER(priv) == VERSION_8723B))
					priv->pshare->InterruptMask |= HIMR_92E_BcnInt | HIMR_92E_TBDOK | HIMR_92E_TBDER;
				else
#endif
					priv->pshare->InterruptMask |= (HIMR_TXBCNERR | HIMR_TXBCNOK);
		}
#endif

#endif // CONFIG_PCI_HCI
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
#ifdef CONFIG_USB_HCI
#ifdef CONFIG_SUPPORT_USB_INT
		priv->pshare->InterruptMask = 0;
		priv->pshare->InterruptMaskExt = HIMRE_88E_TXERR | HIMRE_88E_RXERR | HIMRE_88E_TXFOVW | HIMRE_88E_RXFOVW;
		
#ifdef CONFIG_INTERRUPT_BASED_TXBCN
		if (opmode & WIFI_AP_STATE)
			priv->pshare->InterruptMask |= HIMR_88E_BcnInt | HIMR_88E_TBDOK | HIMR_88E_TBDER;
		else if (opmode & WIFI_ADHOC_STATE)
			priv->pshare->InterruptMask |= HIMR_88E_BcnInt | HIMR_88E_TBDOK | HIMR_88E_TBDER;
#endif
#endif // CONFIG_SUPPORT_USB_INT
#endif // CONFIG_USB_HCI

#ifdef CONFIG_SDIO_HCI
		//
		// Initialize and enable SDIO Host Interrupt.
		//
		InitSdioInterrupt(priv);
#endif
#endif // CONFIG_RTL_88E_SUPPORT

		// FGPA does not have eeprom now
//	RTL_W8(_9346CR_, 0);
		/*
			// ===========================================================================================
			// Download Firmware
			// allocate memory for tx cmd packet
			if((priv->pshare->txcmd_buf = (unsigned char *)kmalloc((LoadPktSize), GFP_ATOMIC)) == NULL) {
				printk("not enough memory for txcmd_buf\n");
				return -1;
			}

			priv->pshare->cmdbuf_phyaddr = get_physical_addr(priv, priv->pshare->txcmd_buf,
					LoadPktSize, PCI_DMA_TODEVICE);
		*/

#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
		if (
#ifdef CONFIG_RTL_92C_SUPPORT
			(GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
			||
#endif
			(GET_CHIP_VER(priv) == VERSION_8192D)
#endif
		) {
			/* currently need not to download fw	*/
			rtl8192cd_ReadFwHdr(priv);

			while (dwnRetry-- && !fwStatus) {
#ifdef CONFIG_RTL_92D_SUPPORT
				if (GET_CHIP_VER(priv) == VERSION_8192D)
					fwStatus = Load_92D_Firmware(priv);
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
				if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C))
					fwStatus = Load_92C_Firmware(priv);
#endif
				delay_ms(20);
			};
			if (fwStatus) {
				DEBUG_INFO("Load firmware successful!\n");
			} else {
				DEBUG_INFO("Load firmware check!\n");
#ifdef PCIE_POWER_SAVING
				priv->pshare->rf_ft_var.power_save &= ~( L1_en | L2_en);
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8192D) {
				if (RTL_R8(0x1c5) == 0xE0) {
					DEBUG_INFO("RTL8192D part number failed!!\n");
					RESTORE_INT(x);
					return -1;
				}
			}
#endif
			}
		}
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
#if defined(__KERNEL__) && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
		if (GET_CHIP_VER(priv) == VERSION_8188E) {
			rtl8192cd_ReadFwHdr(priv);
			
			fwStatus = Load_88E_Firmware(priv);
			if (fwStatus) {
				DEBUG_INFO("Load firmware successful!\n");
			} else {
				DEBUG_INFO("Load firmware check!\n");
			}
		}
#endif
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
#ifdef MP_TEST
		if (!priv->pshare->rf_ft_var.mp_specific)
#endif
		if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
			extern RT_STATUS FirmwareDownload8812(struct rtl8192cd_priv * priv);
			rtl8192cd_ReadFwHdr(priv);
			priv->bFWReady = ! (0 ^ FirmwareDownload8812(priv));
		}
#endif
	}

#ifdef  CONFIG_WLAN_HAL
	if (errorFlag) {
		panic_printk("%s, %d, errorFlag:%08x \n", __FUNCTION__, __LINE__, errorFlag);
		RESTORE_INT(x);
		return -1;
	}
#endif
	/*
		MacConfigAfterFwDownload(priv);
	*/

	// Adaptive Rate Table for Basic Rate
	val32 = 0;
	for (i = 0; i < 32; i++) {
		if (AP_BSSRATE[i]) {
			if (AP_BSSRATE[i] & 0x80)
				val32 |= get_bit_value_from_ieee_value(AP_BSSRATE[i] & 0x7f);
		}
	}
	val32 |= (priv->pmib->dot11nConfigEntry.dot11nBasicMCS << 12);

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
		// Do Nothing
		// Filen: it is not necessary to check !?
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
		unsigned int delay_count = 10;
		do {
			if (!is_h2c_buf_occupy(priv))
				break;
			delay_us(5);
			delay_count--;
		} while (delay_count);
	}

#ifdef P2P_SUPPORT
	/*cfg p2p cfg p2p if (OPMODE & WIFI_P2P_SUPPORT) {*/
	if (rtk_p2p_is_enabled(priv)==PROPERTY_P2P) {
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
		P2P_DEBUG("managment frame G only \n");
		set_RATid_cmd(priv, 0, ARFR_G_ONLY, val32);	// under P2P mode
#endif		
	} else
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
#ifdef USB_POWER_SUPPORT
				val32 &= USB_RA_MASK;
#endif
				set_RATid_cmd(priv, 0, ARFR_Band_A_BMC, val32);
			} else
				set_RATid_cmd(priv, 0, ARFR_BMC, val32);
		}
#ifdef CONFIG_RTL_92C_SUPPORT
		else if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
			set_RATid_cmd(priv, 0, ARFR_BMC, val32);
		}
#endif
		else
#endif
		{
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
			set_RATid_cmd(priv, 0, ARFR_BMC, val32);
#endif
		}

//	kfree(priv->pshare->txcmd_buf);

#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
		if (opmode & WIFI_AP_STATE) {
			if (priv->auto_channel == 0) {
				DEBUG_INFO("going to init beacon\n");
				init_beacon(priv);
			}
		}

	//3 Enable IMR
#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
#if 0   //Filen: defer to open after drv_open
		GET_HAL_INTERFACE(priv)->EnableIMRHandler(priv);
#endif
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
#ifdef CONFIG_PCI_HCI
		//enable interrupt
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E) {
			RTL_W32(REG_88E_HIMR, priv->pshare->InterruptMask);
			RTL_W32(REG_88E_HIMRE, priv->pshare->InterruptMaskExt);
		} else
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
			if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
#ifdef DRVMAC_LB
				priv->pshare->InterruptMask &= ~HIMR_92E_BcnInt;
#endif
#ifdef TXREPORT
				priv->pshare->InterruptMask |= HIMR_92E_C2HCMD;
#endif
				RTL_W32(REG_HIMR0_8812, priv->pshare->InterruptMask);
				RTL_W32(REG_HIMR1_8812, priv->pshare->InterruptMaskExt);
			} else
#endif
			{
				RTL_W32(HIMR, priv->pshare->InterruptMask);
			}
//	RTL_W32(IMR+4, priv->pshare->InterruptMaskExt);
//	RTL_W32(IMR, 0xffffffff);
//	RTL_W8(IMR+4, 0x3f);
#endif // CONFIG_PCI_HCI

#if defined(CONFIG_USB_HCI) && defined(CONFIG_RTL_88E_SUPPORT)
#ifdef CONFIG_SUPPORT_USB_INT
		RTL_W32(REG_88E_HIMR, priv->pshare->InterruptMask);
		RTL_W32(REG_88E_HIMRE, priv->pshare->InterruptMaskExt);

		// REG_USB_SPECIAL_OPTION - BIT(4)
		// 0; Use interrupt endpoint to upload interrupt pkt
		// 1; Use bulk endpoint to upload interrupt pkt,

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
		if(pHalData->RtIntInPipe == 0x05)
			RTL_W8(REG_USB_SPECIAL_OPTION, RTL_R8(REG_USB_SPECIAL_OPTION) & (~INT_BULK_SEL));
		else
#endif
			RTL_W8(REG_USB_SPECIAL_OPTION, RTL_R8(REG_USB_SPECIAL_OPTION) | (INT_BULK_SEL));
#endif // CONFIG_SUPPORT_USB_INT
#endif // CONFIG_USB_HCI && CONFIG_RTL_88E_SUPPORT
	}

	// ===========================================================================================

#ifdef CHECK_HANGUP
	if (priv->reset_hangup)
		priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
	else
#endif
	{
		if (opmode & WIFI_AP_STATE)
			priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
		else
			priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20;
	}

	if (!priv->pshare->rf_ft_var.disable_pwr_by_rate
#ifdef SUPPORT_RTL8188E_TC
		&& (!((GET_CHIP_VER(priv) == VERSION_8188E) && IS_TEST_CHIP(priv)))
#endif
	)
	{
#ifdef CONFIG_RTL_8812_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8812E) {
			*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_CCK11_CCK1_JAguar));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_Ofdm18_Ofdm6_JAguar));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_A_Ofdm54_Ofdm24_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[0])	= cpu_to_be32(RTL_R32(rTxAGC_A_MCS3_MCS0_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[4])	= cpu_to_be32(RTL_R32(rTxAGC_A_MCS7_MCS4_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[8])	= cpu_to_be32(RTL_R32(rTxAGC_A_MCS11_MCS8_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_A_MCS15_MCS12_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss1Index3_Nss1Index0_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss1Index7_Nss1Index4_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[8]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss2Index5_Nss2Index2_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[16]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss2Index9_Nss2Index6_JAguar));
			*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_B[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_CCK11_CCK1_JAguar));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_Ofdm18_Ofdm6_JAguar));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[4]) = cpu_to_be32(RTL_R32(rTxAGC_B_Ofdm54_Ofdm24_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[0])	= cpu_to_be32(RTL_R32(rTxAGC_B_MCS3_MCS0_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[4])	= cpu_to_be32(RTL_R32(rTxAGC_B_MCS7_MCS4_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[8])	= cpu_to_be32(RTL_R32(rTxAGC_B_MCS11_MCS8_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[12]) = cpu_to_be32(RTL_R32(rTxAGC_B_MCS15_MCS12_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_B[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_Nss1Index3_Nss1Index0_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_B[4]) = cpu_to_be32(RTL_R32(rTxAGC_B_Nss1Index7_Nss1Index4_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_B[8]) = cpu_to_be32(RTL_R32(rTxAGC_B_Nss2Index1_Nss1Index8_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_B[12]) = cpu_to_be32(RTL_R32(rTxAGC_B_Nss2Index5_Nss2Index2_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_B[16]) = cpu_to_be32(RTL_R32(rTxAGC_B_Nss2Index9_Nss2Index6_JAguar));
		} else
#endif
#if defined(CONFIG_WLAN_HAL_8881A)
		if (GET_CHIP_VER(priv)==VERSION_8881A) {
			*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_CCK11_CCK1_JAguar));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_Ofdm18_Ofdm6_JAguar));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_A_Ofdm54_Ofdm24_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[0])	= cpu_to_be32(RTL_R32(rTxAGC_A_MCS3_MCS0_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[4])	= cpu_to_be32(RTL_R32(rTxAGC_A_MCS7_MCS4_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[8])	= cpu_to_be32(RTL_R32(rTxAGC_A_MCS11_MCS8_JAguar));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_A_MCS15_MCS12_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss1Index3_Nss1Index0_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss1Index7_Nss1Index4_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[8]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss2Index5_Nss2Index2_JAguar));
			*(unsigned int *)(&priv->pshare->phw->VHTTxAgcOffset_A[16]) = cpu_to_be32(RTL_R32(rTxAGC_A_Nss2Index9_Nss2Index6_JAguar));
		} else
#endif	
		if(1
#if defined(CONFIG_WLAN_HAL_8814AE)
		    && (GET_CHIP_VER(priv) != VERSION_8814A)
#endif
#if defined(CONFIG_WLAN_HAL_8822BE)
		    && (GET_CHIP_VER(priv) != VERSION_8822B)
#endif
#if defined(CONFIG_WLAN_HAL_8197F)
			&& (GET_CHIP_VER(priv) != VERSION_8197F) /* 97F doesn't set power by rate to Tx AGC registers */
#endif
		)
		{
			// get default Tx AGC offset
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[0])  = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs03_Mcs00));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[4])  = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs07_Mcs04));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[8])  = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs11_Mcs08));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_A_Mcs15_Mcs12));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_A_Rate18_06));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_A_Rate54_24));
			*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]) = cpu_to_be32((RTL_R32(rTxAGC_A_CCK11_2_B_CCK11) & 0xffffff00)
					| RTL_R8(rTxAGC_A_CCK1_Mcs32 + 1));

#if defined(CONFIG_RTL_92D_SUPPORT)&& defined(CONFIG_RTL_92D_DMDP)
			if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && priv->pshare->wlandev_idx == 1) {
				*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[0])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs03_Mcs00));
				*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[4])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs07_Mcs04));
				*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[8])	= cpu_to_be32(RTL_R32(rTxAGC_B_Mcs11_Mcs08));
				*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[12]) = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs15_Mcs12));
				*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate18_06));
				*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[4]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate54_24));
				*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]) = cpu_to_be32((RTL_R8(rTxAGC_A_CCK11_2_B_CCK11) << 24)
						| (RTL_R32(rTxAGC_B_CCK5_1_Mcs32) >> 8));
			}
#endif

			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[0])  = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs03_Mcs00));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[4])  = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs07_Mcs04));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[8])  = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs11_Mcs08));
			*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[12]) = cpu_to_be32(RTL_R32(rTxAGC_B_Mcs15_Mcs12));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[0]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate18_06));
			*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[4]) = cpu_to_be32(RTL_R32(rTxAGC_B_Rate54_24));
			*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_B[0]) = cpu_to_be32((RTL_R8(rTxAGC_A_CCK11_2_B_CCK11) << 24)
					| (RTL_R32(rTxAGC_B_CCK5_1_Mcs32) >> 8));
#if 0			
			for(i=0;i<=12;i+=4){
				panic_printk("MCSTxAgcOffset_A[%d]=%x\n",i,*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_A[i]));
			}
			for(i=0;i<=12;i+=4){
				panic_printk("MCSTxAgcOffset_B[%d]=%x\n",i,*(unsigned int *)(&priv->pshare->phw->MCSTxAgcOffset_B[i]));
			}
			for(i=0;i<=4;i+=4){
				panic_printk("OFDMTxAgcOffset_A[%d]=%x\n",i,*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_A[i]));
			}
			for(i=0;i<=4;i+=4){
				panic_printk("OFDMTxAgcOffset_B[%d]=%x\n",i,*(unsigned int *)(&priv->pshare->phw->OFDMTxAgcOffset_B[i]));
			}
			panic_printk("CCKTxAgc_A[0]=%x\n",*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_A[0]));
			panic_printk("CCKTxAgc_B[0]=%x\n",*(unsigned int *)(&priv->pshare->phw->CCKTxAgc_B[0]));
#endif			
		}		

		if (priv->pshare->txpwr_pg_format_abs)        
		{
#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
			if(( GET_CHIP_VER(priv) == VERSION_8814A)||( GET_CHIP_VER(priv) == VERSION_8822B))
				set_target_power_8814(priv);
			else
#endif
			set_target_power(priv);
		}

	}

#ifdef ADD_TX_POWER_BY_CMD
	assign_txpwr_offset(priv);
#endif

#ifdef TXPWR_LMT
	if (!priv->pshare->rf_ft_var.disable_txpwrlmt)
	{
#ifdef TXPWR_LMT_NEWFILE
	if((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8188E) || IS_HAL_CHIP(priv)) {
		PHY_ConfigTXLmtWithParaFile_new(priv);
#ifdef BEAMFORMING_AUTO		
#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE) || defined(CONFIG_WLAN_HAL_8197F)
		if( GET_CHIP_VER(priv) == VERSION_8814A || GET_CHIP_VER(priv) == VERSION_8822B || GET_CHIP_VER(priv) == VERSION_8197F)
			PHY_ConfigTXLmtWithParaFile_new_TXBF(priv);
#endif			
#endif		
	} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		PHY_ConfigTXLmtWithParaFile(priv);
	}
#endif
	RESTORE_INT(x);

#ifdef _TRACKING_TABLE_FILE
	if (priv->pshare->rf_ft_var.pwr_track_file) {
		#ifndef CONFIG_RTL_8723B_SUPPORT
		if((ODMPTR->ConfigBBRF) && (GET_CHIP_VER(priv) != VERSION_8188E))
			ODM_ConfigRFWithTxPwrTrackHeaderFile(ODMPTR);
		else
		#endif
			PHY_ConfigTXPwrTrackingWithParaFile(priv);
	}
#endif


#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192D)) {
		if ((priv->pmib->dot11RFEntry.ther < 0x07) || (priv->pmib->dot11RFEntry.ther > 0x1d)) {
			DEBUG_ERR("TPT: unreasonable target ther %d, disable tpt\n", priv->pmib->dot11RFEntry.ther);
			priv->pmib->dot11RFEntry.ther = 0;
		}
	} else
#endif
	{
		if ((priv->pmib->dot11RFEntry.ther < 0x07) || (priv->pmib->dot11RFEntry.ther > 0x32)) {
			DEBUG_ERR("TPT: unreasonable target ther %d, disable tpt\n", priv->pmib->dot11RFEntry.ther);
			priv->pmib->dot11RFEntry.ther = 0;
		}
	}

	
	/*
		if (opmode & WIFI_AP_STATE)
		{
			if (priv->auto_channel == 0) {
				DEBUG_INFO("going to init beacon\n");
				init_beacon(priv);
			}
		}
	*/

#ifdef BT_COEXIST
	if(GET_CHIP_VER(priv) == VERSION_8192E && priv->pshare->rf_ft_var.btc == 1){
			bt_coex_init(priv);
	}
#endif

#ifdef CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
#if defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8197F) 
		if ((GET_CHIP_VER(priv) == VERSION_8881A)||(GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8197F)){
			//Don't enable BB here
		}
#endif // CONFIG_WLAN_HAL_8881A         

#ifdef CONFIG_WLAN_HAL_8192EE
		if (GET_CHIP_VER(priv) == VERSION_8192E) {
			// TODO: Filen, check 8192E BB/RF control
			//Don't enable BB here
		}
#endif //CONFIG_WLAN_HAL_8192EE
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
#ifdef CONFIG_RTL_8812_SUPPORT
		if (GET_CHIP_VER(priv) != VERSION_8812E)
#endif
		{
			/*---- Set CCK and OFDM Block "ON"----*/
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 1);
#if defined(CONFIG_RTL_92D_SUPPORT)
			if (!(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G))
#endif
				PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 1);
		}
	}

	delay_ms(2);

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific) {
#ifdef CONFIG_WLAN_HAL
		if ( IS_HAL_CHIP(priv) ) {
			u4Byte  MACAddr = 0x87654321;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_ETHER_ADDR, (pu1Byte)&MACAddr);
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif //CONFIG_WLAN_HAL
			RTL_W32(MACID, 0x87654321);

		delay_ms(150);
	}
#endif
//	RESTORE_INT(x);

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8192D) {
		SAVE_INT_AND_CLI(x);		
#ifdef DPK_92D		
		if (((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific) 
			&& priv->pmib->dot11RFEntry.phyBandSelect==PHY_BAND_5G && !priv->pshare->rf_ft_var.dpk_on) {
			panic_printk(">>> DPK ON!!!");
			priv->pshare->rf_ft_var.dpk_on = 1;
		}
#endif		
#ifdef SW_LCK_92D
		PHY_LCCalibrate_92D(priv);
#endif
		SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
		SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);

		if (priv->pmib->dot11RFEntry.macPhyMode == SINGLEMAC_SINGLEPHY)
			clnt_ss_check_band(priv, priv->pmib->dot11RFEntry.dot11channel);
		RESTORE_INT(x);
		/*
		 *	IQK
		 */

#ifdef DPK_92D
		if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G && priv->pshare->rf_ft_var.dpk_on)
			PHY_DPCalibrate(priv);
#endif
	}
#endif // CONFIG_RTL_92D_SUPPORT

#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (
#ifdef CONFIG_RTL_92C_SUPPORT
		(GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		||
#endif
		(GET_CHIP_VER(priv) == VERSION_8188E)
#endif
#ifdef CONFIG_RTL_8723B_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
			||
#endif
			(GET_CHIP_VER(priv) == VERSION_8723B)
#endif
	) {
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
			// switch to channel 7 before doing IQK
			printk("Switch to channel 7 before doing 88E IQK\n");
			SAVE_INT_AND_CLI(x);
			SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
			SwChnl(priv, 7, priv->pshare->offset_2nd_chan);
			RESTORE_INT(x);
		}
#endif

		watchdog_kick();

		//Do NOT perform APK fot RF team's request
		//PHY_APCalibrate(priv);		// APK_92C  APK_88C
#ifdef CALIBRATE_BY_ODM
		if (GET_CHIP_VER(priv) == VERSION_8188E) {
			PHY_LCCalibrate_8188E(ODMPTR);
		} else
#endif
		{
			SAVE_INT_AND_CLI(x);
			PHY_LCCalibrate(priv);
			RESTORE_INT(x);
		}

		SAVE_INT_AND_CLI(x);

		SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
		SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);

		/*
			 *	Set RF & RRSR depends on band in use
			 */
		if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11G | WIRELESS_11N)) {
			if ((priv->pmib->dot11StationConfigEntry.autoRate) || !(priv->pmib->dot11StationConfigEntry.fixedTxRate & 0xf)) {
#ifdef CONFIG_RTL_92C_SUPPORT
				if ( IS_UMC_A_CUT_88C(priv) || GET_CHIP_VER(priv) == VERSION_8192C )
					PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f000);
				else
#endif
					PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f200);

//				RTL_W32(RRSR, RTL_R32(RRSR) & ~(0x0c));
			} else {
				PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x0f400);
			}
		} else {
			PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x0f400);
		}

		RESTORE_INT(x);
	}
#endif // CONFIG_RTL_92C_SUPPORT || CONFIG_RTL_88E_SUPPORT
	/*
		if(priv->pshare->rf_ft_var.ofdm_1ss_oneAnt == 1){// use one PATH for ofdm and 1SS
			Switch_1SS_Antenna(priv, 2);
			Switch_OFDM_Antenna(priv, 2);
		}
	*/

	watchdog_kick();
	delay_ms(100);
	SAVE_INT_AND_CLI(x);

	//RTL_W32(0x100, RTL_R32(0x100) | BIT(14)); //for 8190 fw debug

	// init DIG variables
//	val32 = 0x40020064;	// 0x20010020
	val32 = 0x20010064;	// 0x20010020
//	
	priv->pshare->threshold0 = (unsigned short)(val32 & 0x000000FF);
	priv->pshare->threshold1 = (unsigned short)((val32 & 0x000FFF00) >> 8);
	priv->pshare->threshold2 = (unsigned short)((val32 & 0xFFF00000) >> 20);
	priv->pshare->digDownCount = 0;
	priv->pshare->digDeadPoint = 0;
	priv->pshare->digDeadPointHitCount = 0;

	if (priv->pshare->rf_ft_var.nbi_filter_enable && 
		(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)) {
		NBI_filter_on(priv);
	}

	set_DIG_state(priv, 1);		// DIG on

#if defined(CONFIG_WLAN_HAL_8814AE)
	if (priv->pmib->dot11RFEntry.tx4path && priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G){
		priv->pmib->dot11RFEntry.tx2path = 0;
		priv->pmib->dot11RFEntry.tx3path = 0;
	}else{
		if (priv->pmib->dot11RFEntry.tx3path){
			if(get_rf_mimo_mode(priv) < MIMO_3T3R){
				DEBUG_INFO("Not 3T3R, disable tx3path\n");
				priv->pmib->dot11RFEntry.tx3path = 0;
			} else{
				if(priv->pmib->dot11RFEntry.tx2path)
					priv->pmib->dot11RFEntry.tx2path = 0;
			}
		}
	}
#endif
#if defined(CONFIG_WLAN_HAL_8814AE)
	if(GET_CHIP_VER(priv) == VERSION_8814A){
		if(priv->pmib->dot11RFEntry.tx4path && priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G){
			PHY_SetBBReg(priv,rTX_PATH_SEL_1, bMaskH12Bits,0xe4f);	// Tx path B/C/D for 1SS HT/VHT
			PHY_SetBBReg(priv,rTX_PATH_SEL_2, bMask4to15Bits,0xe4f);// Tx path B/C/D for 2SS HT/VHT
			PHY_SetBBReg(priv,rTXPATH_AC, bMask4to7Bits,0xf);		// Tx path B/C/D for CCK
			PHY_SetBBReg(priv,rCCK_RX_AC, bMaskH4Bits,0xf);			// Tx path B/C/D for CCK
		}else if(priv->pmib->dot11RFEntry.tx3path){
			PHY_SetBBReg(priv,rTX_PATH_SEL_1, bMaskH12Bits,0x90e);	// Tx path B/C/D for 1SS HT/VHT
			PHY_SetBBReg(priv,rTX_PATH_SEL_2, bMask4to15Bits,0x90e);// Tx path B/C/D for 2SS HT/VHT
			PHY_SetBBReg(priv,rTXPATH_AC, bMask4to7Bits,0xe);		// Tx path B/C/D for CCK
			PHY_SetBBReg(priv,rCCK_RX_AC, bMaskH4Bits,0x7);			// Tx path B/C/D for CCK
		}else if(priv->pmib->dot11RFEntry.tx2path){
			PHY_SetBBReg(priv,rTX_PATH_SEL_1, bMaskH12Bits,0x106);	// Tx path B/C for 1SS
			PHY_SetBBReg(priv,rTXPATH_AC, bMask4to7Bits,0x6);		// Tx path B/C for CCK
			PHY_SetBBReg(priv,rCCK_RX_AC, bMaskH4Bits	,0x6);		// Tx path B/C for CCK
		}
		RTL_W8(0x194, RTL_R8(0x194) | BIT(0)); // Enable clock source, for 8814 firmware v23
	}
#endif

#if defined(CONFIG_WLAN_HAL_8197F)
    if(GET_CHIP_VER(priv) == VERSION_8197F) {
        if (priv->pmib->dot11RFEntry.MIMO_TR_mode == MIMO_1T1R) {
            config_phydm_trx_mode_8197f(ODMPTR, ODM_RF_A, ODM_RF_A, 0);
        }
        else {
            config_phydm_trx_mode_8197f(ODMPTR, (ODM_RF_A|ODM_RF_B), (ODM_RF_A|ODM_RF_B), priv->pmib->dot11RFEntry.tx2path);
        }
    }
#endif

#if defined(CONFIG_WLAN_HAL_8822BE)
    if(GET_CHIP_VER(priv) == VERSION_8822B) {
        if (priv->pmib->dot11RFEntry.MIMO_TR_mode == MIMO_1T1R) {
            config_phydm_trx_mode_8822b(ODMPTR, ODM_RF_A, ODM_RF_A, 0);
        }
        else {
            config_phydm_trx_mode_8822b(ODMPTR, (ODM_RF_A|ODM_RF_B), (ODM_RF_A|ODM_RF_B), priv->pmib->dot11RFEntry.tx2path);
        }
    }
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
	if ((GET_CHIP_VER(priv) == VERSION_8812E) || IS_HAL_CHIP(priv)) {
		if (GET_CHIP_VER(priv) == VERSION_8192E) {
			if (priv->pmib->dot11RFEntry.tx2path)
			{
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)		
				RTL_W32(0x90C, 0x83321333);
#else
				RTL_W32(0x90C, 0xc3321333);
#endif
				RTL_W32(0x80C, RTL_R32(0x80C) & ~BIT(31));
//				RTL_W8(0x6D8, RTL_R8(0x6D8) | 0x3F);
				RTL_W8(0x6D8, RTL_R8(0x6D8) | 0xFF);

				RTL_W8(0xA07, 0xC1);
				RTL_W8(0xA11, RTL_R8(0xA11) & ~BIT(5));
				//RTL_W8(0xA20, (RTL_R8(0xA20) & ~BIT(5)) | BIT(4));
				//RTL_W8(0xA2E, RTL_R8(0xA2E) | BIT(3) | BIT(2));
				//RTL_W8(0xA2F, (RTL_R8(0xA2F) & ~BIT(5)) | BIT(4));
				//RTL_W8(0xA75, RTL_R8(0xA75) | BIT(0));
				RTL_W32(0xC8C, 0xa0240000);
				RTL_W8(0x800, RTL_R8(0x800) & ~BIT(1));
			} else {
				PHY_SetBBReg(priv, 0x90C, BIT(30), 0);
			}
			if (priv->pmib->dot11RFEntry.bcn2path){
				RTL_W32(0x80c,RTL_R32(0x80c)|BIT(31));
				RTL_W8(0x6D8, RTL_R8(0x6D8) | 0xCF);
			}
		}
	} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
    {
        if (priv->pshare->rf_ft_var.cck_tx_pathB) {
            RTL_W8(0xa07, 0x40);	// 0x80 -> 0x40    CCK path B Tx
            RTL_W8(0xa0b, 0x84);	// 0x88 -> 0x84    CCK path B Tx
        }

        // CCK path A Tx
        #ifdef CONFIG_POCKET_ROUTER_SUPPORT
        #ifdef CONFIG_RTL_92D_SUPPORT
        if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)
        #endif
        {
            RTL_W8(0xa07, (RTL_R8(0xa07) & 0xbf));
            RTL_W8(0xa0b, (RTL_R8(0xa0b) & 0xfb));
        }
        #endif

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198B)
// 8814 merge issue
#ifdef CONFIG_WLAN_HAL
        if (IS_HAL_CHIP(priv)) {
        } else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif //CONFIG_WLAN_HAL
        {
    		RTL_W8(AGGR_BK_TIME, 0x18);
		RTL_W16(0x4ca, 0x0a0a);
    //	RTL_W32(RESP_SIFS_CCK, 0x0e0e0a0a);
    		//RTL_W32(ACKTO, 0x40001440);
    		RTL_W16(ACKTO, 0x1440);
    		RTL_W16(RXFLTMAP2, 0xffff);
    		//RTL_W16(RCR, RTL_R16(RCR)&(~ BIT(11)));
        }
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
		//CBN debug
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
//			RTL_W32(RD_CTRL, RTL_R32(RD_CTRL)|BIT(13)); // enable force tx beacon
			RTL_W8(BCN_MAX_ERR, 0); // tx beacon error threshold
//			RTL_W16(EIFS, 0x0040);	// eifs < tbtt_prohibit
			if (opmode & WIFI_AP_STATE)
				RTL_W16(rFPGA0_RFTiming1, 0x5388);
		}
#endif

#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif //CONFIG_WLAN_HAL
		{//not HAL
			RTL_W16(EIFS, 0x0040);	// eifs = 40 us

#ifdef CONFIG_PCI_HCI
			RTL_W32(0x350, RTL_R32(0x350) | BIT(26));	// tx status check
#endif
		}

#ifdef HIGH_POWER_EXT_PA
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
			if (priv->pshare->rf_ft_var.use_ext_pa) {
				priv->pmib->dot11RFEntry.trswitch = 1;
				PHY_SetBBReg(priv, 0x870, BIT(10), 0);
				if (GET_CHIP_VER(priv) == VERSION_8192C)
					PHY_SetBBReg(priv, 0x870, BIT(26), 0);
			}
		}
#endif

#if defined(SW_ANT_SWITCH) || defined(HW_ANT_SWITCH)
//	priv->pmib->dot11RFEntry.trswitch = 1;
#endif
		if(GET_CHIP_VER(priv) != VERSION_8188E){
			if (priv->pmib->dot11RFEntry.trswitch)
				RTL_W8(GPIO_MUXCFG, RTL_R8(GPIO_MUXCFG) | TRSW0EN);
			else
				RTL_W8(GPIO_MUXCFG, RTL_R8(GPIO_MUXCFG) & ~TRSW0EN);
		}

#ifdef __OSK__
    	if (/*GET_CHIP_VER(priv) == VERSION_8188C && */IS_88RE(priv)){
       		RTL_W8(0xC30, 0x4A);
	    	RTL_W8(0xC50, 0x20);
    	}
#endif

#ifdef DFS
		if (!priv->pmib->dot11DFSEntry.disable_DFS) {
			RTL_W8(0xc50, 0x24);
			delay_us(10);
			RTL_W8(0xc50, 0x20);
		}
#endif

		if (get_rf_mimo_mode(priv) == MIMO_2T2R) {			
			RTL_W8(0xA20, (RTL_R8(0xA20) & ~BIT(5)) | BIT(4));
			RTL_W8(0xA2E, RTL_R8(0xA2E) | BIT(3) | BIT(2));				
			RTL_W8(0x800, RTL_R8(0x800) & ~BIT(1));
			
			if (priv->pmib->dot11RFEntry.tx2path) {
				RTL_W32(0x90C, 0x83321333);
				RTL_W32(0x80C, RTL_R32(0x80C) & ~BIT(31));
				RTL_W8(0x6D8, RTL_R8(0x6D8) | 0x3F);
				RTL_W8(0xA07, 0xC1);
				RTL_W8(0xA11, RTL_R8(0xA11) & ~BIT(5));
				RTL_W8(0xA2F, (RTL_R8(0xA2F) & ~BIT(5)) | BIT(4));
				RTL_W8(0xA75, RTL_R8(0xA75) | BIT(0));
				RTL_W32(0xC8C, 0xa0240000);
			}
				PHY_SetBBReg(priv, 0x90C, BIT(30), 0);
		} else if (get_rf_mimo_mode(priv) == MIMO_1T1R) {
			if (priv->pmib->dot11RFEntry.tx2path) {
				DEBUG_INFO("Not 2T2R, disable tx2path\n");
				priv->pmib->dot11RFEntry.tx2path = 0;
			}
			if (priv->pmib->dot11RFEntry.txbf) {
				DEBUG_INFO("Not 2T2R, disable txbf\n");
				priv->pmib->dot11RFEntry.txbf = 0;
			}
		}
		
	}


{
	//Set 0x55d=0 except ADHOC Mode for all IC (BCN_MAX_ERR)
	#ifdef CLIENT_MODE
		if (OPMODE & WIFI_ADHOC_STATE)
			RTL_W8(0x55d, 0xff);
		else
	#endif
		if(OPMODE & WIFI_AP_STATE) {
			if((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C))
				RTL_W8(0x55d, 0xff); //Set 92C to MAX value to avoid init fail for some un-healthy demo boards.
			else
				RTL_W8(0x55d, 0x1); //Allow 1 Beacon Error only //Always Tx Beacon within PIFS
		}

//	printk("0x55d = 0x%x\n", RTL_R8(0x55d));
}

#ifdef DRVMAC_LB
	if (!priv->pmib->miscEntry.drvmac_lb)
#endif
	{
#ifdef USE_OUT_SRC
		if (IS_OUTSRC_CHIP(priv)) {
			ODM_DMInit(ODMPTR);			// DM parameters init
			EdcaParaInit(priv);
#if (PHYDM_LA_MODE_SUPPORT == 1)			
			ADCSmp_Init(ODMPTR);
#endif
        }
        else
#endif
        {
            rtl8192cd_AdaptivityInit(priv);
        }

// TODO: the following code need to be integrated into WlanHAL.... for 8814A...
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A) ||  defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
		if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)) { //eric-8822
// ?	did twice ?
//		phy_BB8192CD_Check_ParaFile(priv);
#if defined(CONFIG_WLAN_HAL_8881A)
			if ((GET_CHIP_VER(priv) == VERSION_8881A) &&  _GET_HAL_DATA(priv)->bTestChip ) 
			{
				PHY_CheckBBWithParaFile(priv, PHYREG);
			}
#endif

			priv->pshare->No_RF_Write = 0;

#ifdef CONFIG_WLAN_HAL_8822BE
			//Set some fine-tune TRX related Registers (Avoid PHY parameters error)
			if(GET_CHIP_VER(priv) == VERSION_8822B){

#ifdef DFS
				//WorkAround Solution to Stop 8822 HW Tx (if fw downloaded) 
				if(priv->pmib->dot11DFSEntry.disable_tx)
				RTL_W8(TXPAUSE, 0xff);
#endif


				set_8822_trx_regs(priv);

				//eric-8822 lck
				PHY_LCCalibrate_8822B(ODMPTR);
			}
#endif
	
			SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);		
			SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
			priv->pshare->No_RF_Write = 1;
			if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
				RTL_W32(0x808, RTL_R32(0x808) | BIT(29));
				RTL_W8(0x454, RTL_R8(0x454) | BIT(7));
			} else { //PHY_BAND_2G
				RTL_W32(0x808, RTL_R32(0x808) | BIT(29) | BIT(28));
				RTL_W8(0x454, RTL_R8(0x454) & ~ BIT(7));
			}
#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)			
			if ((GET_CHIP_VER(priv) != VERSION_8814A) && (GET_CHIP_VER(priv) != VERSION_8822B))
#endif
			{
				RTL_W16(0x4ca, 0x1f1f);			
				RTL_W8(REG_RA_TRY_RATE_AGG_LMT_8812, 0x0);				// try rate agg limit
			}	

#if defined(CONFIG_WLAN_HAL_8814AE)			
			if (GET_CHIP_VER(priv) == VERSION_8814A) {
				if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G){
					if((priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)==0)
						RTL_W16(0x4ca, 0x2a2a);
				}
				#ifdef NOT_RTK_BSP
				else
					RTL_W16(0x4ca, 0x3030);
				#endif
				
#ifdef MBSSID
				if (GET_ROOT(priv)->vap_count == 0)
					RTL_W8(0x5BE,0x08);
#endif				
				
				if (priv->pshare->rf_ft_var.disable_pathA)
				{
					RTL_W8(0x1002, 0x2);
					//Rx setting					
					RTL_W8(0x808, 0xee);
					RTL_W8(0x1002, 0x3);
					panic_printk("disable PathA\n");
				}
				
				if (priv->pmib->dot11RFEntry.MIMO_TR_mode == MIMO_2T2R) 
				{			
					if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)
						RTL_W8(0xa07, 0x46);

					RTL_W8(0x1002, 0x2);
								
					//4 Tx setting
					PHY_SetBBReg(priv, 0x93c, 0xfff00000, 0x106);
					PHY_SetBBReg(priv, 0x940, 0x0000fff0, 0x106);

					//Rx setting					
					RTL_W8(0x808, 0x66);
						
					RTL_W8(0x1002, 0x3);

					panic_printk("[%s] switch to 2T2R!\n", priv->dev->name);
				}
			}
#endif


//		RTL_W32(0x2c, 0x28a3e200);
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
			if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
				if (IS_TEST_CHIP(priv))	{
					RTL_W8(0x44b, 0x3e);		// 2SS MCS7~MCS3
				} else {

					RTL_W32(0x448, 0xfffff010);		// 2SS MCS9~MCS3
					RTL_W8(0x452, 0x1f);		// 1SS MCS8

				}
				// RTL_W32(0x2c, 0x4103e200);
				RTL_W8(0x577, RTL_R8(0x577) | 3);	//disable CCA in TXOp
				RTL_W32(0x608, RTL_R32(0x608) | BIT(26));
//				RTL_W8(REG_RESP_SIFS_OFDM_8812+1, 0x0a);		//R2T
#if (BEAMFORMING_SUPPORT == 1)
				if (priv->pmib->dot11RFEntry.txbf == 1) {
					RTL_W8(0x71b, 0x50);							// ndp_rx_standby_timer
					RTL_W16(0x718, RTL_R16(0x718) | 0x2cb);			// Disable SIG-B CRC8 check
					RTL_W32(0x604, RTL_R32(0x604) | BIT(26));			// Disable SIG-B CRC8 check
					RTL_W32(RCR, RTL_R32(RCR) | RCR_ACF);					
				}
#endif
			}
#endif

#if defined(CONFIG_RTL_8812_SUPPORT)


			RTL_W8(REG_RX_PKT_LIMIT_8812, 0x20);		// rx pkt limit = 16k

//	Path A only
			if (GET_CHIP_VER(priv) == VERSION_8812E) {
				if (get_rf_mimo_mode(priv) == MIMO_1T1R) {
					
					if(AC_SIGMA_MODE==AC_SIGMA_NONE){
					RTL_W8(0x808, 0x11);
					RTL_W16(0x80c, 0x1111);
					RTL_W8(0xa07, (RTL_R8(0xa07) & 0xf3));	// [3:2] = 2'b 00
					RTL_W32(0x8bc, (RTL_R32(0x8bc) & 0x3FFFFF9F) | BIT(30) );
					RTL_W8(0xe00, (RTL_R8(0xe00) & 0xf0) | 0x04 );
					RTL_W8(0xe90, 0);
					RTL_W32(0xe60, 0);
					RTL_W32(0xe64, 0);

					}
#ifdef AC2G_256QAM		
					if(1) //if(is_ac2g(priv))
					{
						RTL_W32(0x48c, 0x00000015);
						RTL_W32(0x48c+4, 0x003ff000);
					}
#endif
				} else if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
                	if (!((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific))	{
					       if (priv->pmib->dot11RFEntry.tx2path) {
					              DEBUG_INFO("8812 Enable Tx 2 Path\n");
					       	      RTL_W16(0x80c, 0x3333);
					       } else {
						       DEBUG_INFO("8812 Disable Tx 2 Path\n");
						       RTL_W16(0x80c, 0x1113);
					       }
                    }
				}
#ifdef AC2G_256QAM	
					if(1) //if(is_ac2g(priv))
					{
						RTL_W32(0x48c, 0x00000015);
						RTL_W32(0x48c+4, 0x003ff000);
						RTL_W32(0x494, 0x00000015);
						RTL_W32(0x494+4, 0xffcff000);
					}
#endif
			}
#endif

			// 8814 merge issue

#if 1 //for 8812 IOT issue with 11N NICs
#if defined(CONFIG_WLAN_HAL_8814AE)			
			if (GET_CHIP_VER(priv) == VERSION_8814A && priv->pshare->rf_ft_var.disable_2ndcca)
#endif
#if defined(CONFIG_WLAN_HAL_8822BE)	
			if (GET_CHIP_VER(priv) != VERSION_8822B)
#endif
			{
				DEBUG_INFO("0x838 B(1)= 0, 0x456 = 0x32 \n");
				RTL_W8(0x838, (RTL_R8(0x838)& ~ BIT(0))); //Disbale CCA
			}
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
			if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
				if (IS_TEST_CHIP(priv))
					RTL_W8(0x456, 0x32); //Refine AMPDU MAX duration
				else if(get_rf_mimo_mode(priv) == MIMO_1T1R)
					RTL_W8(0x456, 0xe0);
				else
					RTL_W8(0x456, 0x70); //8812_11n_iot, increase 0x456 for ampdu number

				RTL_W8(0x480, 0x20); //Enable duplicate RTS to whole bandwidth
				
				//RTL_W32(0x458, 0xffff);			// aggregation max length
				RTL_W32(0x458, 0x7fff); 			// agg size 64k->32k

#ifdef CONFIG_RTL8672
				RTL_W8(0x420, 0x00); // Enable Retry + New frames for AMPDU
#endif				
				if (IS_C_CUT_8812(priv)) {
					RTL_W8(0x640, 0x40);
					RTL_W8(0x45b, 0x80);
				} else {
					RTL_W8(0x640, 0x80);
				}

#ifdef EN_EFUSE								
				if (priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE){
					PHY_SetBBReg(priv, 0xc1c, 0xffe00000, OFDMSwingTable_8812[priv->pshare->OFDM_index0[0]]);
					PHY_SetBBReg(priv, 0xe1c, 0xffe00000, OFDMSwingTable_8812[priv->pshare->OFDM_index0[1]]);
				}
#else
				if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G){						
						// 2G OFDM base index should be 0dB
						PHY_SetBBReg(priv, 0xc1c, 0xffe00000, OFDMSwingTable_8812[12]);
						PHY_SetBBReg(priv, 0xe1c, 0xffe00000, OFDMSwingTable_8812[12]);
						priv->pshare->OFDM_index0[0] = 12;
						priv->pshare->OFDM_index0[1] = 12;
				}else{
#ifdef HIGH_POWER_EXT_PA
					if (priv->pshare->rf_ft_var.use_ext_pa) {
						// ext_pa OFDM base index should be -4dB
						PHY_SetBBReg(priv, 0xc1c, 0xffe00000, OFDMSwingTable_8812[20]);
						PHY_SetBBReg(priv, 0xe1c, 0xffe00000, OFDMSwingTable_8812[20]);
						priv->pshare->OFDM_index0[0] = 20;
						priv->pshare->OFDM_index0[1] = 20;
					} else
#endif
					{
						//int_pa OFDM base index should be -3dB
						PHY_SetBBReg(priv, 0xc1c, 0xffe00000, OFDMSwingTable_8812[18]);
						PHY_SetBBReg(priv, 0xe1c, 0xffe00000, OFDMSwingTable_8812[18]);
						priv->pshare->OFDM_index0[0] = 18;
						priv->pshare->OFDM_index0[1] = 18;
					}
			}
#endif
#ifdef MCR_WIRELESS_EXTEND				
				RTL_W32(0x10 , 0x2aab5e0c); //Enhance the Vcore for reviewing the function on
				RTL_W32(0x830,0x2aaa6c88);
#endif
			}
#endif
#if defined(CONFIG_WLAN_HAL_8814AE)
			if (GET_CHIP_VER(priv) == VERSION_8814A) {
#if 0
				if (IS_TEST_CHIP(priv))
					RTL_W8(0x456, 0x32); //Refine AMPDU MAX duration
				else if(get_rf_mimo_mode(priv) == MIMO_1T1R)
					RTL_W8(0x456, 0xe0);
				else
					RTL_W8(0x456, 0x70); //8812_11n_iot, increase 0x456 for ampdu number

				RTL_W8(0x480, 0x20); //Enable duplicate RTS to whole bandwidth
				
				//RTL_W32(0x458, 0xffff);			// aggregation max length
				RTL_W32(0x458, 0x7fff); 			// agg size 64k->32k

				if (IS_C_CUT_8812(priv)) {
					RTL_W8(0x640, 0x40);
					RTL_W8(0x45b, 0x80);
				} else {
					RTL_W8(0x640, 0x80);
				}
#endif
#ifdef HIGH_POWER_EXT_PA
				if (priv->pshare->rf_ft_var.use_ext_pa) {
					//4 ext_pa OFDM base index  -3dB
					PHY_SetBBReg(priv, rA_TxScale_Jaguar, 0xffe00000, TxScalingTable_Jaguar[18]); //4 path A
					PHY_SetBBReg(priv, rB_TxScale_Jaguar, 0xffe00000, TxScalingTable_Jaguar[18]); //4 path B
					PHY_SetBBReg(priv, rC_TxScale_Jaguar2, 0xffe00000, TxScalingTable_Jaguar[18]); //4 path C
					PHY_SetBBReg(priv, rD_TxScale_Jaguar2, 0xffe00000, TxScalingTable_Jaguar[18]); //4 path D
					priv->pshare->OFDM_index0[0] = 18;
					priv->pshare->OFDM_index0[1] = 18;
					priv->pshare->OFDM_index0[2] = 18;
					priv->pshare->OFDM_index0[3] = 18;
				} else
#endif
				{
					//4 0 dB for internal PA
					PHY_SetBBReg(priv, rA_TxScale_Jaguar, 0xffe00000, TxScalingTable_Jaguar[24]);
					PHY_SetBBReg(priv, rB_TxScale_Jaguar, 0xffe00000, TxScalingTable_Jaguar[24]);
					PHY_SetBBReg(priv, rC_TxScale_Jaguar2, 0xffe00000, TxScalingTable_Jaguar[24]);
					PHY_SetBBReg(priv, rD_TxScale_Jaguar2, 0xffe00000, TxScalingTable_Jaguar[24]);
					priv->pshare->OFDM_index0[0] = 24;
					priv->pshare->OFDM_index0[1] = 24;
					priv->pshare->OFDM_index0[2] = 24;
					priv->pshare->OFDM_index0[3] = 24;					
				}

			}

#endif

#endif
		}
#endif
	}

#if  defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8723B) {
		panic_printk("(%s)line=%d, LCK 8723B\n", __FUNCTION__, __LINE__); 
		PHY_LCCalibrate_8723B(ODMPTR);
		panic_printk("(%s)line=%d  set GRAN_BT = 0 \n", __FUNCTION__, __LINE__); 
		RTL_W8(0x765, 0x0);
		priv->pshare->OFDM_index0[0] = 14;
		priv->pshare->CCK_index0 = 20;
		}
#endif

	RESTORE_INT(x);

#ifdef CONFIG_WLAN_HAL_8197F
    if(GET_CHIP_VER(priv) == VERSION_8197F) {
        PHY_LCCalibrate_8197F(ODMPTR);
	unsigned int rf_path=0, count=0, no_iqk=0;
	for(rf_path=0; rf_path<2 ; rf_path++) {
		PHY_SetRFReg(priv, rf_path, RF_WE_LUT, 0x80000,0x1); // RF Mode table write enable
		PHY_SetRFReg(priv, rf_path, RF_WE_LUT, 0x80000,0x1); // RF Mode table write enable
			
		PHY_SetRFReg(priv, rf_path, RF_RCK_OS, 0xfffff,0x20000); // Select IQK Tx mode 0x30=0x18000
		count=0;
		while(PHY_QueryRFReg(priv, rf_path, RF_RCK_OS, 0xfffff, 1)!=0x20000) {
			delay_us(2);
			PHY_SetRFReg(priv, rf_path, RF_RCK_OS, 0xfffff,0x20000); 
			if (++count > 100)
				break;
		}
		if (PHY_QueryRFReg(priv, rf_path, RF_RCK_OS, 0xfffff, 1)!=0x20000) {
			printk("%s:%d Set RF mode table to TX TQK mode fail!!!\n", __FUNCTION__, __LINE__);
			no_iqk = 1;
		} else {
			PHY_SetRFReg(priv, rf_path, RF_TXPA_G1, 0xfffff,0x0005f); // Set Table data
			PHY_SetRFReg(priv, rf_path, RF_TXPA_G2, 0xfffff,0x01ff7); // /*PA off, default: 0x1fff*/
		}
		
		PHY_SetRFReg(priv, rf_path, RF_RCK_OS, 0xfffff,0x30000); // Select IQK Rx mode 0x30=0x30000
		count=0;
		while(PHY_QueryRFReg(priv, rf_path, RF_RCK_OS, 0xfffff, 1)!=0x30000) {
			delay_us(2);
			PHY_SetRFReg(priv, rf_path, RF_RCK_OS, 0xfffff,0x30000); 
			if (++count > 100)
				break;
		}
		if (PHY_QueryRFReg(priv, rf_path, RF_RCK_OS, 0xfffff, 1)!=0x30000) {
			printk("%s:%d Set RF mode table to RX TQK mode-1 fail!!!\n", __FUNCTION__, __LINE__);
			no_iqk = 1;
		} else {
			PHY_SetRFReg(priv, rf_path, RF_TXPA_G1, 0xfffff,0x0005f); // Set Table data
			PHY_SetRFReg(priv, rf_path, RF_TXPA_G2, 0xfffff,0xf1df3); ///*PA off, deafault:0xf1dfb*/
		}
		
		PHY_SetRFReg(priv, rf_path, RF_RCK_OS, 0xfffff,0x38000); // Select IQK Rx mode 0x30=0x38000
		count=0;
		while(PHY_QueryRFReg(priv, rf_path, RF_RCK_OS, 0xfffff, 1)!=0x38000) {
			delay_us(2);
			PHY_SetRFReg(priv, rf_path, RF_RCK_OS, 0xfffff,0x38000); 
			if (++count > 100)
				break;
		}
		if (PHY_QueryRFReg(priv, rf_path, RF_RCK_OS, 0xfffff, 1)!=0x38000) {
			printk("%s:%d Set RF mode table to RX TQK mode-2 fail!!!\n", __FUNCTION__, __LINE__);
			no_iqk = 1;
		} else {
			PHY_SetRFReg(priv, rf_path, RF_TXPA_G1, 0xfffff,0x0005f); // Set Table data
			PHY_SetRFReg(priv, rf_path, RF_TXPA_G2, 0xfffff,0xf1ff2); // /*PA off : default:0xf1ffa*/
		}	
		
		PHY_SetRFReg(priv, rf_path, RF_WE_LUT, 0x80000,0x0); // RF Mode table write disable
		PHY_SetRFReg(priv, rf_path, RF_WE_LUT, 0x80000,0x0); // RF Mode table write disable

		if (no_iqk) {
			printk("%s:%d Dont't do IQK!!!\n", __FUNCTION__, __LINE__);
			ODM_CmnInfoUpdate(ODMPTR, ODM_CMNINFO_ABILITY, ODMPTR->SupportAbility & (~ ODM_RF_CALIBRATION));
		}
	}
		
        if(_GET_HAL_DATA(priv)->cutVersion == ODM_CUT_A){
            /* Get channel 1 CV for LCK */
            PHY_SetRFReg(priv, RF92CD_PATH_A, rRfChannel, 0xff, 1);
            priv->pshare->rf_ft_var.cv_ch1 = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0xB2, 0xff, 1);
            //panic_printk("cv_ch1=%x\n", priv->pshare->rf_ft_var.cv_ch1);
        }
        SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
        SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
        //PHY_QueryBBReg(priv, rOFDM0_XATxIQImbalance, bMaskOFDM_D);
        panic_printk("[97F] Default BB Swing=%u\n",ODMPTR->RFCalibrateInfo.DefaultOfdmIndex);
        /* 97F BB swing default values are set by PHY_REG, currently 0dB for intpa, -5dB for extpa*/
    }
#endif

#if	defined(CONFIG_WLAN_HAL_8192EE)

	if (GET_CHIP_VER(priv) == VERSION_8192E) {
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0xb1, bMask20Bits, 0x55418);		// LCK
//		RESTORE_INT(x);
		PHY_IQCalibrate(priv);
        // TX power tracking init 
#ifdef HIGH_POWER_EXT_PA
		if (priv->pshare->rf_ft_var.use_ext_pa) {
			priv->pshare->OFDM_index0[0] = 30;
			priv->pshare->OFDM_index0[1] = 30;
    			priv->pshare->CCK_index0 = 30;
		} else
#endif
		{
		        priv->pshare->OFDM_index0[0] = 20;
			priv->pshare->OFDM_index0[1] = 20;
		    	priv->pshare->CCK_index0 = 20;
		}
       // printk("Init OFDM_index0 base index = %x \n",priv->pshare->OFDM_index0[0]);
       // printk("Init OFDM_index0 base index = %x \n",priv->pshare->OFDM_index0[1]);
       // printk("priv->pshare->CCK_index0 = %x \n",priv->pshare->CCK_index0);            
//
//		PHY_ConfigBBWithParaFile(priv, PHYREG);

	   //LCCalibrate   #20130301 Anchi
//	   printk("Do 8192E LCK!! RF 0xB6 = 0x%x\n", PHY_QueryRFReg(priv, RF_PATH_A, 0xB6, bMaskDWord, 1));
		for (i=0 ; i<10 ; i++) {
			if (PHY_QueryRFReg(priv, RF_PATH_A, 0xB6, 0xff000, 1) == 0x8)
				break;
	  
			PHY_SetRFReg(priv, RF_PATH_A, 0x18, bMaskDWord, 0x0FC07);	   //LCK
			delay_ms(50);
			PHY_SetRFReg(priv, RF_PATH_A, 0xB6, bMaskDWord, 0x0803E);
			DEBUG_INFO("==> RF 0xB6 = 0x%x\n", PHY_QueryRFReg(priv, RF_PATH_A, 0xB6, bMaskDWord, 1));
		}
		   
		for (i=0 ; i<10 ; i++) {
			if (PHY_QueryRFReg(priv, RF_PATH_A, 0xB2, bMaskDWord, 1) == 0x8CC00)
				break;
	  		PHY_SetRFReg(priv, RF_PATH_A, 0x18, bMaskDWord, 0x0FC07);	   //LCK		   
			delay_ms(50);
			PHY_SetRFReg(priv, RF_PATH_A, 0xB2, bMaskDWord, 0x8CC00);		   
			DEBUG_INFO("==> RF 0xB2 = 0x%x\n", PHY_QueryRFReg(priv, RF_PATH_A, 0xB2, bMaskDWord, 1));
		}
		DEBUG_INFO("8192E LCK done!!\n");
//	   SAVE_INT_AND_CLI(x);
	   
		//Increse 92E rx gain   #20130301 Anchi
		PHY_SetRFReg(priv, RF_PATH_A, 0x0, bMaskDWord, 0x33e74);

		//2013-0717 VCO 9.6G supr #Anchi
	   //PHY_SetRFReg(priv, RF_PATH_A, 0xb2, bMaskDWord, 0xCC00);    
	   //PHY_SetRFReg(priv, RF_PATH_A, 0x18, bMaskDWord, 0xFC07);    

		RTL_W8(0xa2f, 0x10);	//CDD for CCK
		RTL_W8(0xa10, 0x7c);	// for ACPR
#if 0 //disable CCK 2R CCA due to DC tone
		if (_GET_HAL_DATA(priv)->cutVersion == ODM_CUT_D) {
			//CCK-2RCCA+path selection
			RTL_W8(0xa2e, 0xd6);  
			RTL_W8(0xa01, 0x47);
				
			PHY_SetBBReg(priv,0xa04,BIT(24)|BIT(25),1); 
			PHY_SetBBReg(priv,0xa04,BIT(26)|BIT(27),0); 
			PHY_SetBBReg(priv,0xa74,BIT(8),1); 
			PHY_SetBBReg(priv,0xa08,BIT(28),1); 
		}
#endif
		RTL_W8(0x4c7, 0x80); 	// Enable Single-AMPDU

		SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
		SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
	}
#endif


#if 1 // TODO: Filen, tmp setting for tuning TP
#ifdef CONFIG_WLAN_HAL_8881A
	if (GET_CHIP_VER(priv) == VERSION_8881A) {
		unsigned int c50_bak = RTL_R8(0xc50);
		RTL_W8(0xc50, 0x22);
		RTL_W8(0xc50, c50_bak);
#if 0		
		if (get_bonding_type_8881A()==BOND_8881AB) {
			RTL_W32(0x460, 0x03086666);
		} else {
			RTL_W32(0x460, 0x0320ffff);
		}
#endif
		RTL_W32(0x460, 0x03014444);
		RTL_W16(0x4ca, 0x1414);
		RTL_W8(0x456, 0x30);
		priv->pshare->OFDM_index0[0] = 22;		// -5dB
		priv->pshare->AddTxAGC = 0;
		priv->pshare->AddTxAGC_index = 0;
		priv->pshare->rf_ft_var.pwrtrk_TxAGC = 0;

		if (get_bonding_type_8881A() == BOND_8881AN) 
		{
			unsigned int ch_band = PHY_QueryRFReg(priv, RF_PATH_A, 0x18, BIT(17)|BIT(18), 1);
			if (ch_band == 0) 
				PHY_SetRFReg(priv, RF_PATH_A, 0x63, bMaskDWord, 0x114eb);
			else if (ch_band == 1) 
				PHY_SetRFReg(priv, RF_PATH_A, 0x63, bMaskDWord, 0x116e7);
			else if (ch_band = 2) 
				PHY_SetRFReg(priv, RF_PATH_A, 0x63, bMaskDWord, 0x11aeb);			
		}		

		if ((get_bonding_type_8881A() == BOND_8881AM ||get_bonding_type_8881A() == BOND_8881AN) 
			&& priv->pshare->rf_ft_var.use_intpa8881A && (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)) {									
			PHY_SetBBReg(priv, 0xc1c, 0xffe00000, OFDMSwingTable_8812[16]);		//int_pa OFDM base index should be -2dB
			priv->pshare->OFDM_index0[0] = 16;		
		}
		
#ifdef CONFIG_8881A_INT_PA_RTC5634
		if(get_bonding_type_8881A() == BOND_8881AN && priv->pshare->rf_ft_var.use_intpa8881A == 2){
			priv->pshare->OFDM_index0[0] = 18;		
			PHY_SetBBReg(priv, 0xc1c, 0xffe00000, OFDMSwingTable_8812[18]);
			printk("[%s] 8881A internal PA RTC5634\n",__FUNCTION__);
		}
#endif

		
#ifdef AC2G_256QAM		
		if(1) //if(is_ac2g(priv))
		{
			RTL_W32(0x48c, 0x00000015);
			RTL_W32(0x48c+4, 0x003ff000);
		}
#endif

	}
#endif //CONFIG_WLAN_HAL_8881A
#endif

// TODO : TEMP add by Eric , check 8197F need add ?
#ifdef CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E) {
		if (priv->pmib->dot11RFEntry.trswitch
#if defined(HIGH_POWER_EXT_PA)
			|| priv->pshare->rf_ft_var.use_ext_pa 
#endif
#if defined (HIGH_POWER_EXT_LNA)
			|| priv->pshare->rf_ft_var.use_ext_lna
#endif
			) {
#if defined(CONFIG_PCI_HCI)
			PHY_SetBBReg(priv,0x4c,0xfffffff,0x628282);
			PHY_SetBBReg(priv,0x930,0xff000f,0x540000);//930[3:0] =0 for RFE_CTRL_0 and 54 for tr switch
			//if (IS_HAL_TEST_CHIP(priv)) 
				RTL_W32(0x938, 0x540); //938[27:24]=0 for RFE_CTRL_6 and 54 for tr switch
			//else
			//	RTL_W32(0x938, 0x450);
			RTL_W32(0x940, 0x15);
			RTL_W32(0x944, 0xffff);
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			RTL_W32(0x4c, 0x70628282);
			RTL_W32(0x930, 0x5000);
			RTL_W32(0x934, 0x4000);
			RTL_W32(0x93c, 0x0);
			RTL_W32(0x938, 0x540); //938[27:24]=0 for RFE_CTRL_6 and 54 for tr switch
			RTL_W32(0x940, 0x15);
			RTL_W32(0x944, 0x83f);
#endif
			DEBUG_INFO("8192E ext PA or ext LNA !!!\n");
		}
	
		RTL_W8(0x462, 0x08);
#if defined(CONFIG_PCI_HCI)
		RTL_W16(0x4ca, 0x1414);
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		RTL_W16(0x4ca, 0x0a0a);
#ifdef SDIO_AP_OFFLOAD
		RTL_W16(0x454 , RTL_R16(0x454) & ~BIT(6));
		RTL_W16(0x454 , RTL_R16(0x454) & ~BIT(3));
		RTL_W16(0x100 , RTL_R16(0x100) | BIT(8));
#endif
#endif
#if defined(HIGH_POWER_EXT_PA)
		if (priv->pshare->rf_ft_var.use_ext_pa)
			set_CCK_swing_index(priv, 30);	
#endif		
	}
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		RTL_W8(0x462, 0x02);
#ifdef USE_OUT_SRC
		if (IS_OUTSRC_CHIP(priv))
			set_CCK_swing_index(priv, ODMPTR->RFCalibrateInfo.DefaultCckIndex);
#endif
	}
#endif	
    if (get_rf_mimo_mode(priv) == MIMO_1T1R){
#ifdef CONFIG_WLAN_HAL_8881A
        if(GET_CHIP_VER(priv) == VERSION_8881A){    // AC chip & 1T1R
            unsigned int tmp1=0;            
            tmp1 = PHY_QueryBBReg(priv, 0x8b8, bMaskDWord);
            tmp1 &= ~(BIT(19)|BIT(13)); /*bit13 control HT,bit19 controlVHT ,  0:cca no black,1:cca  black*/ 
            PHY_SetBBReg(priv, 0x8b8, 0xffffffff, tmp1);

            tmp1 = PHY_QueryBBReg(priv, 0x8bc, bMaskDWord);
            tmp1 &= ~(BIT(6)|BIT(5));   /*[6:5]=0x0, control by group;group0=mcs0~7*/            
            tmp1 &= ~(BIT(17)|BIT(15)); /*[18:15]=0xa;support mcs<10(0xa)*/
            tmp1 |= (BIT(18) | BIT(16));             
            PHY_SetBBReg(priv, 0x8bc, 0xffffffff, tmp1);          
        }
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
	    if (GET_CHIP_VER(priv)==VERSION_8192D)
	    {
    	   /*  
                when 1T1R let 0xd00[20:19] = 2'b 00  ==> Support < MCS8 
                when 2T2R let 0xd00[20:19] = 2'b 01  ==> Support < MCS16        
                2. 0xd08[3] = 1'b 1           ==> Not support, break*/
	       unsigned int  bbtmp1=0;
	       bbtmp1 = PHY_QueryBBReg(priv, 0xd00, 0xffffffff);
    	   bbtmp1 &= ~(BIT(19)|BIT(20));
	       PHY_SetBBReg(priv, 0xd00, 0xffffffff, bbtmp1);

	       bbtmp1 = PHY_QueryBBReg(priv, 0xd08, 0xffffffff);
    	   bbtmp1 |= BIT(3);
	       PHY_SetBBReg(priv, 0xd08, 0xffffffff, bbtmp1);   
	    }
#endif // CONFIG_RTL_92D_SUPPORT
    }

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
    if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {    // AC chip & 1T1R
        unsigned int delay_count = 100;
        unsigned char val_8b = 0;        
        unsigned char valtmp = 0;                
        do {
            val_8b = RTL_R8(0x8B);
            if ( val_8b & BIT(7)){
                break;
            }
            delay_us(50);
            delay_count--;
            if(delay_count==0){
                panic_printk("check 0x8B expire!!\n");
            }
        } while (delay_count);

        //SDEBUG("net_work_type:%d\n",priv->pmib->dot11BssType.net_work_type);        
        
        if (val_8b & BIT(7)) {
            //SDEBUG("0x8B=[%02x],delay_count=%d\n",val_8b,delay_count);                
            /*------------------------------------------------*/            
            valtmp = (val_8b&(BIT(4)|BIT(5)|BIT(6)))>> 4 ;                               
            if(valtmp==1 && priv->pshare->phw->MIMO_TR_hw_support==MIMO_2T2R){
                panic_printk("Antenna munber not match[%d]\n",valtmp);
//				RESTORE_INT(x);
                return -1;
            }    
            /*---------------------TYPES;AC or N---------------------------*/
            valtmp = (val_8b&(BIT(2)|BIT(3)))>>2  ;               
            //SDEBUG("%s\n",valtmp==3?"AC":(valtmp==2?"11N":"unknow"));                
            if(valtmp==2){  
                if(priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC){
                    //priv->pmib->dot11BssType.net_work_type &= ~(WIRELESS_11AC);
                    panic_printk("Not support AC!!\n");
//					RESTORE_INT(x);
                    return -1;                        
                }                    
                if( priv->pshare->is_40m_bw == 2 ){
                   //priv->pshare->is_40m_bw=1;                        
                    panic_printk("Not support 80M BW!!\n");
//					RESTORE_INT(x);
                    return -1;                       
                }                    
            }
            //SDEBUG("net_work_type:%d\n",priv->pmib->dot11BssType.net_work_type);                                       
            /*------------------------------------------------*/            
            valtmp = val_8b&(BIT(1)|BIT(0))  ;
            //panic_printk("Chip:%s\n",valtmp==3?"AE/AU":(valtmp==2?"AR-VN":"unknow"));                
            /*------------------------------------------------*/
        }
    }
#endif // CONFIG_RTL_8812_SUPPORT
#ifdef WMM_DSCP_C42
	priv->pshare->aggrmax_bak = RTL_R16(PROT_MODE_CTRL + 2);
#endif

#ifdef USE_OUT_SRC
    if (IS_OUTSRC_CHIP(priv)) {        
        if((GET_CHIP_VER(priv) != VERSION_8197F) && (GET_CHIP_VER(priv) != VERSION_8822B)) /* 97F & 8822 need to disable CFE (to prevent tx hang)*/
        {
			if(priv->pmib->dot11BssType.net_work_type != WIRELESS_11B)
            	RTL_W16( RD_CTRL, RTL_R16( RD_CTRL) & ~(DIS_TXOP_CFE));
        }
    }
#endif

	//Tego added power on protection to avoid wlan hang after suddenly wdg reboot, button reviewed
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8881A)|| defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_RTL_8723B_SUPPORT)
	if ((GET_CHIP_VER(priv) == VERSION_8812E)||(GET_CHIP_VER(priv)== VERSION_8192E)||
        (GET_CHIP_VER(priv) == VERSION_8881A)||(GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8723B)){
		RTL_W8(0x1c, RTL_R8(0x1c)|0x02);
	}
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E)
	{
		if(priv->pmib->dot11RFEntry.tx2path)
		{
			PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_WE_LUT, 0x80000,0x1); // RF Mode table write enable
			PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_WE_LUT, 0x80000,0x1); // RF Mode table write enable

			// Paath_A
			PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_RCK_OS, 0xfffff,0x18000); // Select RX mode 0x30=0x18000
			PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_TXPA_G1, 0xfffff,0x0000f); // Set Table data
			PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_TXPA_G2, 0xfffff,0x77fc2); // Enable TXIQGEN in RX mode
			// Path_B
			PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_RCK_OS, 0xfffff,0x18000); // Select RX mode
			PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_TXPA_G1, 0xfffff,0x0000f); // Set Table data
			PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_TXPA_G2, 0xfffff,0x77fc2); // Enable TXIQGEN in RX mode

			PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_WE_LUT, 0x80000,0x0); // RF Mode table write disable
			PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_WE_LUT, 0x80000,0x0); // RF Mode table write disable
			RTL_W8(0xA2F, 0x10);
		}
	}
#endif
#ifdef THERMAL_CONTROL
	priv->pshare->rf_ft_var.current_path = priv->pmib->dot11RFEntry.tx2path+1;
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E)
	{
		if(priv->pmib->dot11RFEntry.tx2path)
		{
			PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_ModeTableAddr, 0x78000,0x3); // Select RX mode
			PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_ModeTableData0, 0xfffff,0x3F7FF); // Set Table data
			PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_ModeTableData1, 0xfffff,0xE26BF); // Enable TXIQGEN in RX mode
			PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_ModeTableAddr, 0x78000, 0x3); // Select RX mode
			PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_ModeTableData0, 0xfffff,0x3F7FF); // Set Table data
			PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_ModeTableData1, 0xfffff,0xE26BF); // Enable TXIQGEN in RX mode
		}
	}
#endif

#ifdef CONFIG_WLAN_HAL_8814AE
    /* RCK for pathB/C/D. Fix 8814 MP-chip RF reg setting bug  */
    if (GET_CHIP_VER(priv) == VERSION_8814A && !IS_HAL_TEST_CHIP(priv)){
        unsigned int rfPathA = PHY_QueryRFReg(priv, RF_PATH_A, 0x1c, bMaskDWord, 1);
        PHY_SetRFReg(priv, RF_PATH_B, 0x1c, bMaskDWord, rfPathA);
        PHY_SetRFReg(priv, RF_PATH_C, 0x1c, bMaskDWord, rfPathA);
        PHY_SetRFReg(priv, RF_PATH_D, 0x1c, bMaskDWord, rfPathA);
    }
#endif


#ifdef CONFIG_1RCCA_RF_POWER_SAVING
#ifdef MP_TEST
	if (!priv->pshare->rf_ft_var.mp_specific)
#endif
	if (priv->pshare->rf_ft_var.one_path_cca_ps) {
		one_path_cca_power_save(priv, 1);
	}
#endif // CONFIG_1RCCA_RF_POWER_SAVING

	set_bcn_dont_ignore_edcca(priv);

#ifdef USB_INTERFERENCE_ISSUE
#ifdef CONFIG_RTL_92C_SUPPORT
 //fixed USB interface interference issue
	if ( GET_CHIP_VER(priv) == VERSION_8188C || GET_CHIP_VER(priv) == VERSION_8192C ) {
		RTL_W8(0xfe40, 0xe0);
		RTL_W8(0xfe41, 0x8d);
		RTL_W8(0xfe42, 0x80);
		RTL_W32(0x20c,0xfd0320);
#if 1
		//2011/01/07 ,suggest by Johnny,for solved the problem that too many protocol error on USB bus
		if(!IS_UMC_A_CUT(priv) )//&& !IS_92C_SERIAL(pHalData->VersionID))// TSMC , 8188
		{		
		    	// 0xE6=0x94
		    	RTL_W8(0xFE40, 0xE6);
			RTL_W8(0xFE41, 0x94);
			RTL_W8(0xFE42, 0x80); 

			// 0xE0=0x19
			RTL_W8(0xFE40, 0xE0);
			RTL_W8(0xFE41, 0x19);
			RTL_W8(0xFE42, 0x80);

			// 0xE5=0x91
			RTL_W8(0xFE40, 0xE5);
			RTL_W8(0xFE41, 0x91);
			RTL_W8(0xFE42, 0x80); 

			// 0xE2=0x81
			RTL_W8(0xFE40, 0xE2);
			RTL_W8(0xFE41, 0x81);
			RTL_W8(0xFE42, 0x80);    
		
		}
	}
#endif
#endif // CONFIG_RTL_92C_SUPPORT
#endif // USB_INTERFERENCE_ISSUE

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)
	if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8192E)) {
		if (priv->pmib->dot11nConfigEntry.dot11nDisable2RCCA) { /* Disable 2RCCA for CCK Rx */
				PHY_SetBBReg(priv,0xA2C,BIT18,0); // disable 2R CCA
				PHY_SetBBReg(priv,0xA2C,BIT22,0);
		}
	}
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) 
    /*8812 default let 0x800 bit(10) be 1, ADC convert mask 
         , when adaptivity_enable EDCCA_enable is false, it will keep 1 for avoid some case tx hangup*/
	if ((GET_CHIP_VER(priv) == VERSION_8812E) ) {
        RTL_W32(0x800,(RTL_R32(0x800)|BIT(10)));            
	}
#endif


#ifdef THERMAL_CONTROL
	if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G){
		priv->pshare->rf_ft_var.ther_hi = THER_HIGH_2G;
		priv->pshare->rf_ft_var.ther_low =	THER_LOW_2G;
	}else{
		priv->pshare->rf_ft_var.ther_hi = THER_HIGH_5G;
		priv->pshare->rf_ft_var.ther_low =	THER_LOW_5G;
	}
#endif

#ifdef BT_COEXIST	
	if(GET_CHIP_VER(priv) == VERSION_8192E){
		extern int bt_state;
		bt_state = 0;
	}
#endif

#ifdef POWER_TRIM
	/* apply kfree if Flash KFREE_ENABLE set or in MP mode*/
	if(GET_CHIP_VER(priv) == VERSION_8197F){
		if (priv->pmib->dot11RFEntry.kfree_enable || priv->pshare->rf_ft_var.mp_specific){
			/* Load power trim (or k free) value from efuse and set RF gain offset */
			power_trim(priv,0);
			priv->pmib->dot11RFEntry.kfree_enable = 1;
		}else{
			/* restore original RF gain offset*/
			power_trim(priv,1);
		}
	}else{
		priv->pmib->dot11RFEntry.kfree_enable = 0;
	}
#endif

#ifdef THER_TRIM
	if(GET_CHIP_VER(priv) == VERSION_8197F){
		ther_trim_efuse(priv);
	}else{
		priv->pshare->rf_ft_var.ther_trim_enable = 0;
	}
#endif

#if defined(CONFIG_WLAN_HAL_8822BE)
	if((GET_CHIP_VER(priv) == VERSION_8822B) && (GET_CHIP_VER_8822(priv) >=3))
		TxACurrentCalibration(priv);
#endif

#if defined(CONFIG_WLAN_HAL_8822BE) || defined(CONFIG_WLAN_HAL_8197F)
		if((GET_CHIP_VER(priv) == VERSION_8822B) || (GET_CHIP_VER(priv) == VERSION_8197F)){
			if(priv->pshare->rf_ft_var.force_bcn){
				PHY_SetBBReg(priv,0x528,BIT18,0);  //enable force beacon
				RTL_W8(0x58c,0x80);//4ms
				RTL_W8(0x55d,0);//force beacon every time
				PHY_SetBBReg(priv,0x574,BIT26,0); 
				RTL_W8(0x58e,0x8);
				PHY_SetBBReg(priv,0x524,BIT12,0x1);
				PHY_SetBBReg(priv,0x524,BIT13,0x1);
			}else{
				PHY_SetBBReg(priv,TBTT_PROHIBIT,0x000FFF00,0x138); //10ms 
			}
		}			
#endif



	// for new IC, don't receive ICV error packet
	if (GET_CHIP_VER(priv) >= VERSION_8192E && GET_CHIP_VER(priv) != VERSION_8822B) {
		RTL_W32(RCR, RTL_R32(RCR) & ~RCR_AICV);	
	}

	RESTORE_INT(x);
	DBFEXIT;

	return 0;

}


#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT) || (defined(CONFIG_RTL_88E_SUPPORT) && defined(__KERNEL__) && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
static void rtl8192cd_ReadFwHdr(struct rtl8192cd_priv *priv)
{
	struct __RTL8192C_FW_HDR__ *pFwHdr = NULL;
	unsigned char *swap_arr;

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific)
		return;
#endif
#if defined(__ECOS) ||  defined(__OSK__)
	unsigned char hdr_buf[RT_8192CD_FIRMWARE_HDR_SIZE];
	swap_arr = hdr_buf;
#endif
#ifdef __KERNEL__
	swap_arr = kmalloc(RT_8192CD_FIRMWARE_HDR_SIZE, GFP_ATOMIC);
	if (swap_arr == NULL)
		return;
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		memcpy(swap_arr, data_rtl8192dfw_n_start, RT_8192CD_FIRMWARE_HDR_SIZE);
	}
#endif

#ifdef CONFIG_RTL_8723B_SUPPORT
if (GET_CHIP_VER(priv) == VERSION_8723B) {
			if (IS_OUTSRC_CHIP(priv)) 
				memcpy(swap_arr, data_rtl8723bfw_start, RT_8192CD_FIRMWARE_HDR_SIZE);
		}
#endif


#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		if (IS_TEST_CHIP(priv))
			memcpy(swap_arr, data_rtl8812fw_start, RT_8192CD_FIRMWARE_HDR_SIZE);
#ifdef AC2G_256QAM
		else if(is_ac2g(priv))
			memcpy(swap_arr, data_rtl8812fw_n_2g_start, RT_8192CD_FIRMWARE_HDR_SIZE);
#endif
		else
			memcpy(swap_arr, data_rtl8812fw_n_start, RT_8192CD_FIRMWARE_HDR_SIZE);
	}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
#ifdef TESTCHIP_SUPPORT
		if (IS_TEST_CHIP(priv))
			memcpy(swap_arr, data_rtl8192cfw_start, RT_8192CD_FIRMWARE_HDR_SIZE);
		else
#endif
		{
			if ( IS_UMC_A_CUT_88C(priv) )
				memcpy(swap_arr, data_rtl8192cfw_ua_start, RT_8192CD_FIRMWARE_HDR_SIZE);
			else
				memcpy(swap_arr, data_rtl8192cfw_n_start, RT_8192CD_FIRMWARE_HDR_SIZE);
		}
	}
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(__KERNEL__) && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		memcpy(swap_arr, Array_8188E_FW_AP, RT_8192CD_FIRMWARE_HDR_SIZE);
	}
#endif

	pFwHdr = (struct __RTL8192C_FW_HDR__ *)swap_arr;
#ifdef _BIG_ENDIAN_
	pFwHdr->signature		= le16_to_cpu(pFwHdr->signature);
	pFwHdr->version		= le16_to_cpu(pFwHdr->version);
	pFwHdr->year		= le16_to_cpu(pFwHdr->year);	// ready on after v33.1
#endif

	priv->pshare->fw_signature	= pFwHdr->signature;
	priv->pshare->fw_category		= pFwHdr->category;
	priv->pshare->fw_function		= pFwHdr->function;
	priv->pshare->fw_version		= pFwHdr->version;
	priv->pshare->fw_sub_version	= pFwHdr->subversion;
	priv->pshare->fw_date_month	= pFwHdr->month;
	priv->pshare->fw_date_day	= pFwHdr->day;
	priv->pshare->fw_date_hour	= pFwHdr->hour;
	priv->pshare->fw_date_minute	= pFwHdr->minute;
#ifdef __KERNEL__
	kfree(swap_arr);
#endif
	/*
		printk("fw_signature: ");
		if (priv->pshare->fw_signature == RTL8192C_TEST_CHIP)
			printk("92C_TEST_CHIP");
		else if (priv->pshare->fw_signature == RTL8188C_TEST_CHIP)
			printk("88C_TEST_CHIP");
		else if (priv->pshare->fw_signature == RTL8192C_MP_CHIP_A)
			printk("92C_MP_CHIP_A");
		else if (priv->pshare->fw_signature == RTL8188C_MP_CHIP_A)
			printk("88C_MP_CHIP_A");
		else if (priv->pshare->fw_signature == RTL8192C_MP_CHIP_B)
			printk("92C_MP_CHIP_B");
		else if (priv->pshare->fw_signature == RTL8188C_MP_CHIP_B)
			printk("88C_MP_CHIP_B");
		printk(", ");

		printk("fw_category: ");
		if (priv->pshare->fw_category == RTL8192C_NIC_PCIE)
			printk("92C_NIC_PCIE");
		else if (priv->pshare->fw_category == RTL8192C_NIC_USB)
			printk("92C_NIC_USB");
		else if (priv->pshare->fw_category == RTL8192C_AP_PCIE)
			printk("92C_AP_PCIE");
		else if (priv->pshare->fw_category == RTL8192C_AP_USB)
			printk("92C_AP_USB");
		printk(", ");

		printk("fw_function: ");
		if (priv->pshare->fw_function == RTL8192C_NIC_NORMAL)
			printk("92C_NIC_NORMAL");
		else if (priv->pshare->fw_function == RTL8192C_NIC_WWLAN)
			printk("92C_NIC_WWLAN");
		else if (priv->pshare->fw_function == RTL8192C_AP_NORMAL)
			printk("92C_AP_NORMAL");
		else if (priv->pshare->fw_function == RTL8192C_AP_SUSPEND)
			printk("92C_AP_SUSPEND");
		printk("\n");

		printk("fw_version: %d.%d, ", priv->pshare->fw_version, priv->pshare->fw_sub_version);
		printk("fw_date: %02x-%02x %02x:%02x\n", priv->pshare->fw_date_month, priv->pshare->fw_date_day,
			priv->pshare->fw_date_hour, priv->pshare->fw_date_minute);
	*/
}
#endif


#ifdef CONFIG_RTL_92C_SUPPORT
static int Load_92C_Firmware(struct rtl8192cd_priv *priv)
{
	int fw_len, wait_cnt = 0;
#ifdef CONFIG_PCI_HCI
	unsigned int CurPtr = 0;
#endif
	unsigned int WriteAddr;
	unsigned int Temp;
	unsigned char *ptmp;

#ifdef CONFIG_RTL8672
	printk("val=%x\n", RTL_R8(0x80));
#endif

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific)
		return TRUE;
#endif

	printk("===> %s\n", __FUNCTION__);

#ifdef TESTCHIP_SUPPORT
	if (IS_TEST_CHIP(priv)) {
		ptmp = data_rtl8192cfw_start + 32;
		fw_len = (int)(data_rtl8192cfw_end - ptmp);

	} else
#endif
	{
		if ( IS_UMC_A_CUT_88C(priv) ) {
			ptmp = data_rtl8192cfw_ua_start + 32;
			fw_len = (int)(data_rtl8192cfw_ua_end - ptmp);

		} else {
			ptmp = data_rtl8192cfw_n_start + 32;
			fw_len = (int)(data_rtl8192cfw_n_end - ptmp);
		}
	}

	// Disable SIC
	RTL_W8(0x41, 0x40);
	delay_ms(1);

	// Enable MCU

	RTL_W8(SYS_FUNC_EN + 1, RTL_R8(SYS_FUNC_EN + 1) | 0x04);
	delay_ms(1);

#ifdef CONFIG_RTL8672
	RTL_W8(0x04, RTL_R8(0x04) | 0x02);
	delay_ms(1);  //czyao
#endif

	// Load SRAM
	WriteAddr = 0x1000;
	RTL_W8(MCUFWDL, RTL_R8(MCUFWDL) | MCUFWDL_EN);
	delay_ms(1);

//	if (IS_TEST_CHIP(priv))
//		RTL_W8(0x82, RTL_R8(0x82) & 0xf7);
//	else
	RTL_W32(MCUFWDL, RTL_R32(MCUFWDL) & 0xfff0ffff);

	delay_ms(1);

#ifdef CONFIG_PCI_HCI
	while (CurPtr < fw_len) {
		if ((CurPtr + 4) > fw_len) {
			// Reach the end of file.
			while (CurPtr < fw_len) {
				Temp = *(ptmp + CurPtr);
				RTL_W8(WriteAddr, (unsigned char)Temp);
				WriteAddr++;
				CurPtr++;
			}
		} else {
			// Write FW content to memory.
			Temp = *((unsigned int *)(ptmp + CurPtr));
			Temp = cpu_to_le32(Temp);
			RTL_W32(WriteAddr, Temp);
			WriteAddr += 4;

			if ((IS_TEST_CHIP(priv) == 0) && (WriteAddr == 0x2000)) {
				unsigned char tmp = RTL_R8(MCUFWDL + 2);
				tmp += 1;
				WriteAddr = 0x1000;
				RTL_W8(MCUFWDL + 2, tmp) ;
				delay_ms(10);
//				printk("\n[CurPtr=%x, 0x82=%x]\n", CurPtr, RTL_R8(0x82));
			}
			CurPtr += 4;
		}
	}
#else
	if (IS_ERR_VALUE(_WriteFW(priv, ptmp, fw_len)))
		printk("WriteFW FAIL !\n");
#endif

	Temp = RTL_R8(0x80);
	Temp &= 0xfe;
	Temp |= 0x02;
	RTL_W8(0x80, (unsigned char)Temp);
	delay_ms(1);
	RTL_W8(0x81, 0x00);

	printk("<=== %s\n", __FUNCTION__);

	// check if firmware is ready
	while (!(RTL_R8(MCUFWDL) & WINTINI_RDY)) {
		if (++wait_cnt > 10) {
			printk("8192c firmware not ready\n");
			return FALSE;
		}
		
		delay_ms(1);
	}
#ifdef CONFIG_RTL8672
	printk("val=%x\n", RTL_R8(MCUFWDL));
#endif

	return TRUE;
}
#endif // CONFIG_RTL_92C_SUPPORT


#define	SET_RTL8192CD_RF_HALT(priv)						\
{ 														\
	unsigned char u1bTmp;								\
														\
	do													\
	{													\
		u1bTmp = RTL_R8(LDOV12D_CTRL);					\
		u1bTmp |= BIT(0); 								\
		RTL_W8(LDOV12D_CTRL, u1bTmp);					\
		RTL_W8(SPS1_CTRL, 0x0);							\
		RTL_W8(TXPAUSE, 0xFF);							\
		RTL_W16(CMDR, 0x57FC);							\
		delay_us(100);									\
		RTL_W16(CMDR, 0x77FC);							\
		RTL_W8(PHY_CCA, 0x0);							\
		delay_us(10);									\
		RTL_W16(CMDR, 0x37FC);							\
		delay_us(10);									\
		RTL_W16(CMDR, 0x77FC);							\
		delay_us(10);									\
		RTL_W16(CMDR, 0x57FC);							\
		RTL_W16(CMDR, 0x0000);							\
		u1bTmp = RTL_R8((SYS_CLKR + 1));				\
		if (u1bTmp & BIT(7))							\
		{												\
			u1bTmp &= ~(BIT(6) | BIT(7));				\
			if (!HalSetSysClk8192CD(priv, u1bTmp))		\
			break;										\
		}												\
		RTL_W8(0x03, 0x71);								\
		RTL_W8(0x09, 0x70);								\
		RTL_W8(0x29, 0x68);								\
		RTL_W8(0x28, 0x00);								\
		RTL_W8(0x20, 0x50);								\
		RTL_W8(0x26, 0x0E);								\
	} while (FALSE);									\
}

void Unlock_MCU(struct rtl8192cd_priv *priv)
{
	// 1. To clear C2H
	RTL_W8(C2H_SYNC_BYTE, 0x0);
	// 2. Unlock Overall MCU while(1)
	RTL_W8(MCU_UNLOCK, 0xFF);
}

void FirmwareSelfReset(struct rtl8192cd_priv *priv)
{
	unsigned char u1bTmp;
	unsigned int  Delay = 1000;
	if (priv->pshare->fw_version > 0x21
#ifdef CONFIG_RTL_92D_SUPPORT
			|| GET_CHIP_VER(priv) == VERSION_8192D
#endif
	   )	{
		RTL_W32(FWIMR, 0x20);
		RTL_W8(REG_HMETFR + 3, 0x20);
		u1bTmp = RTL_R8( REG_SYS_FUNC_EN + 1);
		while (u1bTmp & BIT(2)) {
			Delay--;
			DEBUG_INFO("polling 0x03[2] Delay = %d \n", Delay);
			if (Delay == 0)
				break;
			delay_us(50);
			Unlock_MCU(priv);
			u1bTmp = RTL_R8( REG_SYS_FUNC_EN + 1);
		}
		// restore MCU internal while(1) loop
		RTL_W8(MCU_UNLOCK, 0);
		if (u1bTmp & BIT(2)) {
			DEBUG_ERR("FirmwareSelfReset fail: 0x03=%02x, 0x1EB=0x%02x\n", u1bTmp, RTL_R8(0x1EB));
#ifdef CONFIG_USB_HCI
			RTL_W8(SYS_FUNC_EN+1, 0x50);	//Reset MAC and Enable 8051
			delay_ms(10);
#endif
		} else {
			DEBUG_INFO("FirmwareSelfReset success: 0x03 = %x\n", u1bTmp);
		}
	}
}

//Return Value:
//	1: MAC I/O Registers Enable
//	0: MAC I/O Registers Disable
int check_MAC_IO_Enable(struct rtl8192cd_priv *priv)
{
	//Check PON register to decide
	return ( (RTL_R16(SYS_FUNC_EN) & (FEN_MREGEN | FEN_DCORE)) == (FEN_MREGEN | FEN_DCORE) );
}
#if defined(CONFIG_RTL8672) && !(defined(__OSK__) && defined(CONFIG_RTL_8881A))
extern unsigned char clk_src_40M;
#endif

#ifdef CONFIG_PCI_HCI
int rtl8192cd_stop_hw(struct rtl8192cd_priv *priv)
{
	RTL_W8(0x1c, RTL_R8(0x1c)& ~BIT(1));// unlock reg0x00~0x03 for 8812

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
#ifdef TXREPORT
		RTL8188E_DisableTxReport(priv);
#endif
		RTL_W32(REG_88E_HIMR, 0);
		RTL_W32(REG_88E_HIMRE, 0);
		HalPwrSeqCmdParsing(priv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK,
							PWR_INTF_PCI_MSK, rtl8188E_leave_lps_flow);
	} else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8812E) {
			RTL_W32(REG_HIMR0_8812, 0);
			RTL_W32(REG_HIMR1_8812, 0);
			HalPwrSeqCmdParsing(priv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK,
								PWR_INTF_PCI_MSK, rtl8812_enter_lps_flow);
		} else
#endif
#ifdef CONFIG_RTL_8723B_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8723B) {
			RTL_W32(REG_HIMR_8723B, 0);
			RTL_W32(REG_HIMR1_8723B, 0);
			HalPwrSeqCmdParsing(priv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK,
								PWR_INTF_PCI_MSK, rtl8723B_enter_lps_flow);
		} else
#endif

		{
			RTL_W32(HIMR, 0);
			RTL_W16(HIMRE, 0);
			RTL_W16(HIMRE + 2, 0);
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
		}

	RTL_W8(RCR, 0);
	RTL_W8(TXPAUSE, 0xff);								// Pause MAC TX queue
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	if (
#ifdef CONFIG_RTL_92C_SUPPORT
		(GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
		||
#endif
		(GET_CHIP_VER(priv) == VERSION_8192D)
#endif
	) {
		//	RTL_W8(CR, RTL_R8(CR) & ~(MACTXEN|MACRXEN));
		RTL_W8(CR, 0);
	}
#endif

#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pshare->wlandev_idx == 0)
		RTL_W8(RSV_MAC0_CTRL, RTL_R8(RSV_MAC0_CTRL) & (~MAC0_EN));
	else
		RTL_W8(RSV_MAC1_CTRL, RTL_R8(RSV_MAC1_CTRL) & (~MAC1_EN));

	if ((RTL_R8(RSV_MAC0_CTRL)& MAC0_EN) || (RTL_R8(RSV_MAC1_CTRL)& MAC1_EN)) { // check if another interface exists
		DEBUG_INFO("Another MAC exists, cannot stop hw!!\n");
	} else
#endif
	{
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
		if (
#ifdef CONFIG_RTL_92C_SUPPORT
			(GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_92C_SUPPORT
			||
#endif
			(GET_CHIP_VER(priv) == VERSION_8192D)
#endif
		) {
			//3 2.) ==== RF Off Sequence ====
			phy_InitBBRFRegisterDefinition(priv);		// preparation for read/write RF register

			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x00, bMask20Bits, 0x00);	// disable RF
			RTL_W8(RF_CTRL, 0x00);
			RTL_W8(APSD_CTRL, 0x40);
			RTL_W8(SYS_FUNC_EN, 0xe2);		// reset BB state machine
			RTL_W8(SYS_FUNC_EN, 0xe0);		// reset BB state machine



			//3 3.) ==== Reset digital sequence ====
			if (RTL_R8(MCUFWDL) & BIT(1)) {
				//Make sure that Host Recovery Interrupt is handled by 8051 ASAP.
				RTL_W32(FSIMR, 0);				// clear FSIMR
				RTL_W32(FWIMR, 0x20);			// clear FWIMR except HRCV_INT
				RTL_W32(FTIMR, 0);				// clear FTIMR
				FirmwareSelfReset(priv);

				//Clear FWIMR to guarantee if 8051 runs in ROM, it is impossible to run FWISR Interrupt handler
				RTL_W32(FWIMR, 0x0);			// clear All FWIMR
			} else {
				//Critical Error.
				//the operation that reset 8051 is necessary to be done by 8051
				DEBUG_ERR("%s %d ERROR: (RTL_R8(MCUFWDL) & BIT(1))=0\n", __FUNCTION__, __LINE__);
				DEBUG_ERR("%s %d ERROR: the operation that reset 8051 is necessary to be done by 8051,%d\n", __FUNCTION__, __LINE__, RTL_R8(MCUFWDL));
			}
		}
#endif

		// ==== Reset digital sequence ====

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8723B))
			RTL_W8(SYS_FUNC_EN + 1, RTL_R8(SYS_FUNC_EN + 1) & ~BIT(2));
		else
#endif
			RTL_W8(SYS_FUNC_EN + 1, 0x51);								// reset MCU, MAC register, DCORE
		RTL_W8(MCUFWDL, 0);											// reset MCU ready status

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
			unsigned char u1bTmp = 0;
			// 0x1F[7:0] = 0		// turn off RF
			RTL_W8(REG_RF_CTRL_8812, 0x00);

			// Reset MCU. Suggested by Filen. 2011.01.26. by tynli.
			u1bTmp = RTL_R8(REG_SYS_FUNC_EN_8812 + 1);
			RTL_W8(REG_SYS_FUNC_EN_8812 + 1, (u1bTmp & (~BIT(2))));

			// MCUFWDL 0x80[1:0]=0				// reset MCU ready status
			RTL_W8(REG_MCUFWDL_8812, 0x00);
		}
#endif

#ifdef CONFIG_RTL_8723B_SUPPORT	
		// HW card disable configuration.
		if (GET_CHIP_VER(priv) == VERSION_8723B)
			HalPwrSeqCmdParsing(priv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK, rtl8723B_card_disable_flow);
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E) {
			HalPwrSeqCmdParsing(priv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK, rtl8188E_card_disable_flow);
		} else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8812E) {
				HalPwrSeqCmdParsing(priv, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK, rtl8812_card_disable_flow);
			} else
#endif
			{
				//3 4.) ==== Disable analog sequence ====
				RTL_W8(AFE_PLL_CTRL, 0x80);			// disable PLL

#if defined(CONFIG_RTL_92C_SUPPORT) && defined(CONFIG_RTL_92D_SUPPORT)
				if (GET_CHIP_VER(priv) == VERSION_8192C) {
					RTL_W8(SPS0_CTRL, 0x2b);
				} else
#endif
				{
#ifdef CONFIG_RTL_92C_SUPPORT
					if (IS_UMC_B_CUT_88C(priv))
						RTL_W8(SPS0_CTRL, 0x2b);
					else
#endif
						RTL_W8(SPS0_CTRL, 0x23);
				}
			}
#if defined(CONFIG_RTL8672) && !(defined(__OSK__) && defined(CONFIG_RTL_8881A))
		if (!clk_src_40M)
			RTL_W8(AFE_XTAL_CTRL, RTL_R8(AFE_XTAL_CTRL)&~BIT(0));		// only for ADSL platform because 40M crystal is only used by WiFi chip // disable XTAL, if No BT COEX
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
			unsigned char u1bTmp = 0;

			// Reset MCU IO Wrapper
			u1bTmp = RTL_R8(REG_RSV_CTRL_8812 + 1);
			RTL_W8(REG_RSV_CTRL_8812 + 1, (u1bTmp & (~BIT(0))));
			u1bTmp = RTL_R8(REG_RSV_CTRL_8812 + 1);
			RTL_W8(REG_RSV_CTRL_8812 + 1, u1bTmp | BIT(0));

			// RSV_CTRL 0x1C[7:0] = 0x0E			// lock ISO/CLK/Power control register
			RTL_W8(REG_RSV_CTRL_8812, 0x0e);
		}
#endif
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if ((GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8723B)) {
			// Reset MCU IO Wrapper
			RTL_W8(RSV_CTRL0 + 1, RTL_R8(RSV_CTRL0 + 1) & ~BIT(3));
			RTL_W8(RSV_CTRL0 + 1, RTL_R8(RSV_CTRL0 + 1) | BIT(3));
		} else
#endif
		{
			RTL_W8(APS_FSMCO + 1, 0x10);
		}
		RTL_W8(RSV_CTRL0, 0x0e);				// lock ISO/CLK/Power control register

		//3 5.) ==== interface into suspend ====
//		RTL_W16(APS_FSMCO, (RTL_R16(APS_FSMCO) & 0x00ff) | (0x18 << 8));	// PCIe suspend mode

#ifdef CONFIG_RTL8672
		// 6.) Switch to XTAL_BSEL: NAND
		RTL_W8(AFE_XTAL_CTRL, RTL_R8(AFE_XTAL_CTRL) & ~ BIT(1));
#endif
	}
	return SUCCESS;
}
#endif // CONFIG_PCI_HCI


#ifdef RTK_AC_SUPPORT  //#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_WLAN_HAL_8814AE)

#ifdef AC2G_256QAM
#define MAX_NUM_80M 8

unsigned char available_channel_AC_80m[MAX_NUM_80M][4] = {
	{36, 40, 44, 48},
	{52, 56, 60, 64},
	{100, 104, 108, 112},
	{116, 120, 124, 128},
	{132, 136, 140, 144},
	{149, 153, 157, 161},
	{165, 169, 173, 177},
	{1, 5, 9, 13},
};
#else
#define MAX_NUM_80M 7

unsigned char available_channel_AC_80m[MAX_NUM_80M][4] = {
	{36, 40, 44, 48},
	{52, 56, 60, 64},
	{100, 104, 108, 112},
	{116, 120, 124, 128},
	{132, 136, 140, 144},
	{149, 153, 157, 161},
	{165, 169, 173, 177},
};
#endif


void get_txsc_AC(struct rtl8192cd_priv *priv, unsigned char channel)
{
	unsigned char tmp, i, found = 0;

	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_80) {
		for (tmp = 0; tmp < MAX_NUM_80M; tmp ++) {
			for (i = 0; i < 4; i++) {
				if (channel == available_channel_AC_80m[tmp][i]) {
					found = 1;
					//printk("found channel[%d] at [%d][%d]\n", channel, tmp, i);
					break;
				}
			}

			if (found)
				break;
		}

		if (priv->pshare->rf_ft_var.mp_specific==1){
				if(GET_CHIP_VER(priv) == VERSION_8822B){
				i = 2;			
				}		
		}

		switch (i) {
		case 0:
			//printk("case 0 \n");
			priv->pshare->txsc_20 = _20_B_40_B;
			priv->pshare->txsc_40 = _40_B;
			priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;
			break;
		case 1:
			priv->pshare->txsc_20 = _20_A_40_B;
			priv->pshare->txsc_40 = _40_B;
			priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
			break;
		case 2:
			priv->pshare->txsc_20 = _20_B_40_A;
			priv->pshare->txsc_40 = _40_A;
			priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;
			break;
		case 3:
			priv->pshare->txsc_20 = _20_A_40_A;
			priv->pshare->txsc_40 = _40_A;
			priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
			break;
		default:
			break;
		}
	} else if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40) {
		if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
			priv->pshare->txsc_20 = 1;
		else
			priv->pshare->txsc_20 = 2;

		priv->pshare->txsc_40 = 0;
	} else if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		priv->pshare->txsc_20 = 0;
		priv->pshare->txsc_40 = 0;
	}

	//printk("get_txsc_8812= %d %d \n", priv->pshare->txsc_20, priv->pshare->txsc_40);

}

#endif 

#ifdef CONFIG_RTL_8812_SUPPORT

void SwBWMode_AC(struct rtl8192cd_priv *priv, unsigned int bandwidth, int offset)
{
	unsigned char bTmp = 0, bTmp2 = 0;
	unsigned int dwTmp = 0, dwTmp2 = 0,dwTmp3 = 0, tmp_rf = 0;
	unsigned char primary_channel = 0;
	unsigned int eRFPath, curMaxRFPath;

//		printk("SwBWMode_AC +++ BW = %d txsc_20 = %d txsc_40 = %d\n", bandwidth, priv->pshare->txsc_20, priv->pshare->txsc_40);
	
	curMaxRFPath = RF92CD_PATH_MAX;

	primary_channel = priv->pshare->txsc_20;

//3 ========== <1> Set rf_mode 0x8ac & 0x668 & 0x8
	dwTmp = RTL_R32(0x8ac);
	dwTmp2 = RTL_R32(0x668);
	dwTmp3 = RTL_R32(0x830);

	dwTmp &= ~(BIT(0) | BIT(1) | BIT(6) | BIT(7) | BIT(8) | BIT(9) | BIT(20) | BIT(21));
	dwTmp2 &= ~(BIT(7) | BIT(8));
	dwTmp3 &= ~(BIT(3) | BIT(2) | BIT(1));

	switch (bandwidth) {
	case HT_CHANNEL_WIDTH_AC_5:
		dwTmp |= (BIT(6) | BIT(20));
		dwTmp3 |= (BIT(3) | BIT(1));
		RTL_W32(0x8ac, dwTmp);

		RTL_W32(0x668, dwTmp2);
		RTL_W32(0x830, dwTmp3);
		break;
	case HT_CHANNEL_WIDTH_AC_10:
		dwTmp |= (BIT(7) | BIT(8) | BIT(21));
		RTL_W32(0x8ac, dwTmp);

		RTL_W32(0x668, dwTmp2);
		
		dwTmp3 |= BIT(3);
		RTL_W32(0x830, dwTmp3);
		break;
	case HT_CHANNEL_WIDTH_AC_20:
		dwTmp |= (BIT(9) | BIT(20) | BIT(21));
		RTL_W32(0x8ac, dwTmp);

		RTL_W32(0x668, dwTmp2);
		break;
	case HT_CHANNEL_WIDTH_AC_40:
		dwTmp |= (BIT(0) | BIT(9) | BIT(20) | BIT(21));
		RTL_W32(0x8ac, dwTmp);

		dwTmp2 |= BIT(7);
		RTL_W32(0x668, dwTmp2);
		break;
	case HT_CHANNEL_WIDTH_AC_80:
		dwTmp |= (BIT(1) | BIT(9) | BIT(20) | BIT(21));
		RTL_W32(0x8ac, dwTmp);

		dwTmp2 |= BIT(8);
		RTL_W32(0x668, dwTmp2);
		break;
	}


//3 ========== <2> Set adc buff clk 0x8c4 , rf_mode 0x8
		if (IS_C_CUT_8812(priv)) {
			switch (bandwidth)
			{
			case HT_CHANNEL_WIDTH_AC_5:
				PHY_SetBBReg(priv, 0x8c4, BIT(30), 0);				
				PHY_SetBBReg(priv, 0x8, BIT(7)|BIT(6), 0x2);
				break;
			case HT_CHANNEL_WIDTH_AC_10:
				PHY_SetBBReg(priv, 0x8c4, BIT(30), 0);				
				PHY_SetBBReg(priv, 0x8, BIT(7)|BIT(6), 0x1);
				break;
			case HT_CHANNEL_WIDTH_AC_20:		
				PHY_SetBBReg(priv, 0x8c4, BIT(30), 0);				
				PHY_SetBBReg(priv, 0x8, BIT(7)|BIT(6), 0x0);
				break;
			case HT_CHANNEL_WIDTH_AC_40:
				PHY_SetBBReg(priv, 0x8c4, BIT(30), 0);				
				PHY_SetBBReg(priv, 0x8, BIT(7)|BIT(6), 0x0);
				break;
			case HT_CHANNEL_WIDTH_AC_80:
				PHY_SetBBReg(priv, 0x8c4, BIT(30), 1);	
				PHY_SetBBReg(priv, 0x8, BIT(7)|BIT(6), 0x0);
				break;
			}
		}


//3 ========== <3> Set primary channel 0x8ac & 0xa00, txsc 0x483

		dwTmp = RTL_R32(0x8ac);
		dwTmp &= ~(BIT(2)|BIT(3)|BIT(4)|BIT(5));
		
		bTmp = RTL_R8(0xa00);
		bTmp2= RTL_R8(0x483);

	switch (bandwidth) {
	case HT_CHANNEL_WIDTH_AC_5:
	case HT_CHANNEL_WIDTH_AC_10:
	case HT_CHANNEL_WIDTH_AC_20:
		break;
	case HT_CHANNEL_WIDTH_AC_40:
		dwTmp |= (primary_channel << 2);
		RTL_W32(0x8ac, dwTmp);
		RTL_W32(0x838, (RTL_R32(0x838) & 0x0fffffff) | (primary_channel << 28));

		if (primary_channel == 1)
			bTmp |= BIT(4);
		else
			bTmp &= ~(BIT(4));

		RTL_W8(0xa00, bTmp);

		bTmp2 &= 0xf0;
		bTmp2 |= priv->pshare->txsc_20;
		RTL_W8(0x483, bTmp2);

		break;
	case HT_CHANNEL_WIDTH_AC_80:
		dwTmp |= (primary_channel << 2);
		RTL_W32(0x8ac, dwTmp);
		RTL_W32(0x838, (RTL_R32(0x838) & 0x0fffffff) | (primary_channel << 28));

		bTmp2 = ( priv->pshare->txsc_20 | (priv->pshare->txsc_40 << 4));
		RTL_W8(0x483, bTmp2);

		break;
	}

	SpurCheck8812(priv, 0, 0, bandwidth);

//3 ========== <4> Set RRSR_RSC 0x440

	dwTmp = RTL_R32(0x440);
	dwTmp &= ~(BIT(21) | BIT(22));

	switch (bandwidth) {
	case HT_CHANNEL_WIDTH_AC_5:
	case HT_CHANNEL_WIDTH_AC_10:
	case HT_CHANNEL_WIDTH_AC_20:
		break;
	case HT_CHANNEL_WIDTH_AC_40:
		RTL_W32(0x440, dwTmp);
		break;
	case HT_CHANNEL_WIDTH_AC_80:

		//dwTmp |= (0x3 << 21);
		//dwTmp |= (0x2 << 21);		// duplicate ?
		RTL_W32(0x440, dwTmp);
		break;
	}

//3 ========== <5> 0821, L1_peak_th 0x848
		dwTmp = RTL_R32(0x848);
		dwTmp &= ~(BIT(22)|BIT(23)|BIT(24)|BIT(25));

		switch (bandwidth) {

		case HT_CHANNEL_WIDTH_AC_5:
			if (get_rf_mimo_mode(priv) == MIMO_1T1R)
				dwTmp |= (7<<22);
			else
				dwTmp |= (6<<22);
			break;
			
		case HT_CHANNEL_WIDTH_AC_10:
			if (get_rf_mimo_mode(priv) == MIMO_1T1R)
				dwTmp |= (8<<22);
			else
				dwTmp |= (7<<22);
			break;
			
		case HT_CHANNEL_WIDTH_AC_20:
			if (get_rf_mimo_mode(priv) == MIMO_1T1R)
				dwTmp |= (8<<22);
			else
				dwTmp |= (7<<22);
			break;
		case HT_CHANNEL_WIDTH_AC_40:
			dwTmp |= (6<<22);
			break;
		case HT_CHANNEL_WIDTH_AC_80:
			dwTmp |= (4<<22);	// 5 -> 4 for beacon missing
			break;
		}	
		RTL_W32(0x848, dwTmp);		

//3 ========== <6> Set RF TRX_BW rf_0x18
		switch (bandwidth)
		{
		case HT_CHANNEL_WIDTH_AC_5:
		case HT_CHANNEL_WIDTH_AC_10:
		case HT_CHANNEL_WIDTH_AC_20:
			tmp_rf = 0x03;
			break;
		case HT_CHANNEL_WIDTH_AC_40:
			tmp_rf = 0x01;
			break;
		case HT_CHANNEL_WIDTH_AC_80:
			tmp_rf = 0x00;
			break;
		}

		for(eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++) {
			unsigned int orig_val = PHY_QueryRFReg(priv, eRFPath, rRfChannel, bMask20Bits, 1);
			orig_val &= ~(BIT(10)|BIT(11));
			orig_val |= (tmp_rf << 10); 
			PHY_SetRFReg(priv, eRFPath, rRfChannel, bMask20Bits, orig_val);
		}

}
#endif // #if defined(CONFIG_RTL_8812_SUPPORT) 

#if defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8197F)
void get_txsc_92e(struct rtl8192cd_priv *priv, unsigned char channel)
{
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40) {
		if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
			priv->pshare->txsc_20_92e = 1;
		else
			priv->pshare->txsc_20_92e = 2;
	} else if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
		priv->pshare->txsc_20_92e = 0;
	}
}
#endif

void SwBWMode(struct rtl8192cd_priv *priv, unsigned int bandwidth, int offset)
{
	unsigned char regBwOpMode, regRRSR_RSC, nCur40MhzPrimeSC;
	unsigned int eRFPath, curMaxRFPath, val;
	priv->pshare->CurrentChannelBW = bandwidth; 

#if defined(CONFIG_RTL_8812_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		priv->pshare->No_RF_Write = 0;
		get_txsc_AC(priv, priv->pmib->dot11RFEntry.dot11channel);
		SwBWMode_AC(priv, bandwidth, offset);
		priv->pshare->No_RF_Write = 1;
		return;
	}
#endif //#if defined(CONFIG_RTL_8812_SUPPORT)

//eric-8822
#if defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE) 
	if ((GET_CHIP_VER(priv) == VERSION_8881A) 
		|| (GET_CHIP_VER(priv) == VERSION_8814A)
		|| (GET_CHIP_VER(priv) == VERSION_8822B))
    {
		priv->pshare->No_RF_Write = 0;
		get_txsc_AC(priv, priv->pmib->dot11RFEntry.dot11channel);
		GET_HAL_INTERFACE(priv)->PHYSwBWModeHandler(priv, bandwidth, offset);
		priv->pshare->No_RF_Write = 1;
#if defined(CONFIG_WLAN_HAL_8814AE)
		/* eliminate the 5280MHz & 5600MHz & 5760MHzspur of 8814A */
		if(GET_CHIP_VER(priv) == VERSION_8814A && priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			GET_HAL_INTERFACE(priv)->PHYSpurCalibration(priv);
		}else if(GET_CHIP_VER(priv) == VERSION_8814A && priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G) {
			if (priv->pshare->rf_ft_var.mp_specific == 0)
				phydm_spur_nbi_setting_8814a(ODMPTR);/* eliminate the 2440MHz & 2480MHz spur of 8814A */
		}
#endif	
		return;
	}
#endif //#if defined(CONFIG_WLAN_HAL_8881A)

#if	defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8197F)
	if (GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8197F) {
		get_txsc_92e(priv, priv->pmib->dot11RFEntry.dot11channel);
		GET_HAL_INTERFACE(priv)->PHYSwBWModeHandler(priv, bandwidth, offset);
		return;
	}
#endif

	DEBUG_INFO("SwBWMode(): Switch to %s bandwidth\n", bandwidth ? "40MHz" : "20MHz");

#ifdef CONFIG_RTL_92D_DMDP
	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		curMaxRFPath = RF92CD_PATH_B;
	else
#endif
		curMaxRFPath = RF92CD_PATH_MAX;

	if (offset == 1)
		nCur40MhzPrimeSC = 2;
	else
		nCur40MhzPrimeSC = 1;

	//3 <1> Set MAC register
	regBwOpMode = RTL_R8(BWOPMODE);
	regRRSR_RSC = RTL_R8(RRSR + 2);

	switch (bandwidth) {
	case HT_CHANNEL_WIDTH_20:
		regBwOpMode |= BW_OPMODE_20MHZ;
		RTL_W8(BWOPMODE, regBwOpMode);
		break;
	case HT_CHANNEL_WIDTH_20_40:
		regBwOpMode &= ~BW_OPMODE_20MHZ;
		RTL_W8(BWOPMODE, regBwOpMode);
		regRRSR_RSC = (regRRSR_RSC & 0x90) | (nCur40MhzPrimeSC << 5);
		RTL_W8(RRSR + 2, regRRSR_RSC);

		// Let 812cd_rx, re-assign value
		if (priv->pshare->is_40m_bw) {
			priv->pshare->Reg_RRSR_2 = 0;
			priv->pshare->Reg_81b = 0;
		}
		break;
	default:
		DEBUG_ERR("SwBWMode(): bandwidth mode error!\n");
		return;
		break;
	}

	//3 <2> Set PHY related register
	switch (bandwidth) {
	case HT_CHANNEL_WIDTH_20:
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bRFMOD, 0x0);
		PHY_SetBBReg(priv, rFPGA1_RFMOD, bRFMOD, 0x0);
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, BIT(11) | BIT(10), 3);// SET BIT10 BIT11  for receive cck
		}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, BIT(10), 1);
		}
#endif
		break;
	case HT_CHANNEL_WIDTH_20_40:
		PHY_SetBBReg(priv, rFPGA0_RFMOD, bRFMOD, 0x1);
		PHY_SetBBReg(priv, rFPGA1_RFMOD, bRFMOD, 0x1);
		// Set Control channel to upper or lower. These settings are required only for 40MHz
		PHY_SetBBReg(priv, rCCK0_System, bCCKSideBand, (nCur40MhzPrimeSC >> 1));
		PHY_SetBBReg(priv, rOFDM1_LSTF, 0xC00, nCur40MhzPrimeSC);
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, BIT(11) | BIT(10), 0);// SET BIT10 BIT11  for receive cck
		}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, BIT(10), 0);
		}
#endif
		PHY_SetBBReg(priv, 0x818, (BIT(26) | BIT(27)), (nCur40MhzPrimeSC == 2) ? 1 : 2);
		break;
	default:
		DEBUG_ERR("SwBWMode(): bandwidth mode error! %d\n", __LINE__);
		return;
		break;
	}

	//3<3> Set RF related register
	switch (bandwidth) {
	case HT_CHANNEL_WIDTH_20: {
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B)
			val = 3;
		else
#endif
			val = 1;
	}
	break;
	case HT_CHANNEL_WIDTH_20_40:
		val = 0;
		break;
	default:
		DEBUG_ERR("SwBWMode(): bandwidth mode error! %d\n", __LINE__);
		return;
		break;
	}

	for (eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++)	{
#ifdef CONFIG_RTL_92C_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8188C)) {
			PHY_SetRFReg(priv, eRFPath, rRfChannel, (BIT(11) | BIT(10)), val);
		}
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			priv->pshare->RegRF18[eRFPath] = RTL_SET_MASK(priv->pshare->RegRF18[eRFPath], (BIT(11) | BIT(10)), val, 10);
			PHY_SetRFReg(priv, eRFPath, rRfChannel, bMask20Bits, priv->pshare->RegRF18[eRFPath]);
			//PHY_SetRFReg(priv, eRFPath, rRfChannel, (BIT(11)|BIT(10)), val);
		}
#endif
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
			PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, rRfChannel, (BIT(11) | BIT(10)), val);
		}
#endif
	}

#if 0
	if (priv->pshare->rf_ft_var.use_frq_2_3G)
		PHY_SetRFReg(priv, RF90_PATH_C, 0x2c, 0x60, 0);
#endif
#ifdef TX_EARLY_MODE
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
		if ((bandwidth == HT_CHANNEL_WIDTH_20) && GET_TX_EARLY_MODE) {
			GET_TX_EARLY_MODE = 0;
			//printk("[%s:%d] 88E 20M mpde ===> turn off early mode!\n", __FUNCTION__, __LINE__);
		}
	}
#endif
#endif
}


#if defined(SUPPORT_TX_AMSDU) || defined (P2P_SUPPORT)
void setup_timer2(struct rtl8192cd_priv *priv, unsigned int timeout)
{
	unsigned int current_value;

#ifdef CONFIG_WLAN_HAL
    if ( IS_HAL_CHIP(priv) ) {
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_TSF_TIMER, (pu1Byte)&current_value);
    } else
#endif
    {
        current_value = RTL_R32(TSFTR);
    }

	if (TSF_LESS(timeout, current_value))
		timeout = current_value + 20;

#ifdef CONFIG_WLAN_HAL
    if ( IS_HAL_CHIP(priv) ) {
        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_PS_TIMER, (pu1Byte)&timeout);
    } else 
#endif
    {
    	RTL_W32(TIMER1, timeout);
    }

#ifdef CONFIG_WLAN_HAL
	if ( IS_HAL_CHIP(priv) ) {
        GET_HAL_INTERFACE(priv)->AddInterruptMaskHandler(priv, HAL_INT_TYPE_FS_TIMEOUT0);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif //CONFIG_WLAN_HAL
	{//not HAL
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B)
			RTL_W32(REG_HIMR0_8812,	RTL_R32(REG_HIMR0_8812) | IMR_TIMER2_8812);
		else
#endif
			RTL_W32(HIMR, RTL_R32(HIMR) | HIMR_TIMEOUT2);
	}
}


void cancel_timer2(struct rtl8192cd_priv *priv)
{

#ifdef CONFIG_WLAN_HAL
	if ( IS_HAL_CHIP(priv) ) {
        GET_HAL_INTERFACE(priv)->RemoveInterruptMaskHandler(priv, HAL_INT_TYPE_FS_TIMEOUT0);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif //CONFIG_WLAN_HAL    
	{//not HAL
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B)
			RTL_W32(REG_HIMR0_8812, RTL_R32(REG_HIMR0_8812) & ~IMR_TIMER2_8812);
		else
#endif
			RTL_W32(HIMR, RTL_R32(HIMR) & ~HIMR_TIMEOUT2);
	}
}
#endif

#if defined(CONFIG_VERIWAVE_CHECK)
void setup_timer1(struct rtl8192cd_priv *priv, int timeout)
{
	unsigned int current_value = RTL_R32(TSFTR);

	if (TSF_LESS(timeout, current_value))
		timeout = current_value + 20;

   	RTL_W32(TIMER0, timeout);

#ifdef CONFIG_WLAN_HAL
	if ( IS_HAL_CHIP(priv) ) {
        GET_HAL_INTERFACE(priv)->AddInterruptMaskHandler(priv, HAL_INT_TYPE_PSTIMEOUT1);
	} else
#endif //CONFIG_WLAN_HAL
	{//not HAL
#ifdef CONFIG_RTL_8812_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8812E)
			RTL_W32(REG_HIMR0_8812,	RTL_R32(REG_HIMR0_8812) | IMR_TIMER1_8812);
		else
#endif
			RTL_W32(HIMR, RTL_R32(HIMR) | HIMR_TIMEOUT1);
	}
}

void cancel_timer1(struct rtl8192cd_priv *priv)
{

#ifdef CONFIG_WLAN_HAL
	if ( IS_HAL_CHIP(priv) ) {
        GET_HAL_INTERFACE(priv)->RemoveInterruptMaskHandler(priv, HAL_INT_TYPE_PSTIMEOUT1);
	} else
#endif //CONFIG_WLAN_HAL    
	{//not HAL
#ifdef CONFIG_RTL_8812_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8812E)
			RTL_W32(REG_HIMR0_8812, RTL_R32(REG_HIMR0_8812) & ~IMR_TIMER1_8812);
		else
#endif
			RTL_W32(HIMR, RTL_R32(HIMR) & ~HIMR_TIMEOUT1);
	}
}
#endif



#if 0
void tx_path_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char enable)
{

	if ((get_rf_mimo_mode(priv) != MIMO_2T2R))
		return; // 1T2R, 1T1R; do nothing

	if (pstat == NULL)
		return;

	if (pstat->sta_in_firmware != 1 || getSTABitMap(&priv->pshare->has_2r_sta, REMAP_AID(pstat))) // 2r STA
		return; // do nothing

	// for debug, by victoryman 20090623
	if (pstat->tx_ra_bitmap & 0xff00000) {
		// this should be a 2r station!!!
		return;
	}

	if (pstat->tx_ra_bitmap & 0xffff000) { // 11n 1R client
		if (enable) {
			if (pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_1SS_Antenna(priv, 1);
			else
				Switch_1SS_Antenna(priv, 2);
		} else
			Switch_1SS_Antenna(priv, 3);
	} else if (pstat->tx_ra_bitmap & 0xff0) { // 11bg client
		if (enable) {
			if (pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_OFDM_Antenna(priv, 1);
			else
				Switch_OFDM_Antenna(priv, 2);
		} else
			Switch_OFDM_Antenna(priv, 3);
	}

#if 0  // original  setup
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) { // for 11n 1ss sta
		if (enable) {
			if (pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_1SS_Antenna(priv, 1);
			else
				Switch_1SS_Antenna(priv, 2);
		} else
			Switch_1SS_Antenna(priv, 3);
	} else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) { // for 11g
		if (enable) {
			if (pstat->rf_info.mimorssi[0] > pstat->rf_info.mimorssi[1])
				Switch_OFDM_Antenna(priv, 1);
			else
				Switch_OFDM_Antenna(priv, 2);
		} else
			Switch_OFDM_Antenna(priv, 3);
	}
#endif


}
//#endif


// dynamic Rx path selection by signal strength
void rx_path_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat, int enable)
{
	unsigned char highest_rssi = 0, higher_rssi = 0, under_ss_th_low = 0;
	RF92CD_RADIO_PATH_E eRFPath, eRFPath_highest = 0, eRFPath_higher = 0;
	int ant_on_processing = 0;
#ifdef _DEBUG_RTL8192CD_
	char path_name[] = {'A', 'B'};
#endif

	if (enable == FALSE) {
		if (priv->pshare->phw->ant_off_num) {
			priv->pshare->phw->ant_off_num = 0;
			priv->pshare->phw->ant_off_bitmap = 0;
			RTL_W8(rOFDM0_TRxPathEnable, 0x0f);
			RTL_W8(rOFDM1_TRxPathEnable, 0x0f);
			DEBUG_INFO("More than 1 sta, turn on all path\n");
		}
		return;
	}

	for (eRFPath = RF92CD_PATH_A; eRFPath < priv->pshare->phw->NumTotalRFPath; eRFPath++) {
		if (priv->pshare->phw->ant_off_bitmap & BIT(eRFPath))
			continue;

		if (pstat->rf_info.mimorssi[eRFPath] > highest_rssi) {
			higher_rssi = highest_rssi;
			eRFPath_higher = eRFPath_highest;
			highest_rssi = pstat->rf_info.mimorssi[eRFPath];
			eRFPath_highest = eRFPath;
		}

		else if (pstat->rf_info.mimorssi[eRFPath] > higher_rssi) {
			higher_rssi = pstat->rf_info.mimorssi[eRFPath];
			eRFPath_higher = eRFPath;
		}

		if (pstat->rf_info.mimorssi[eRFPath] < priv->pshare->rf_ft_var.ss_th_low)
			under_ss_th_low = 1;
	}

	// for OFDM
	if (priv->pshare->phw->ant_off_num > 0) {
		for (eRFPath = RF92CD_PATH_A; eRFPath < priv->pshare->phw->NumTotalRFPath; eRFPath++) {
			if (!(priv->pshare->phw->ant_off_bitmap & BIT(eRFPath)))
				continue;

			if (highest_rssi >= priv->pshare->phw->ant_on_criteria[eRFPath]) {
				priv->pshare->phw->ant_off_num--;
				priv->pshare->phw->ant_off_bitmap &= (~BIT(eRFPath));
				RTL_W8(rOFDM0_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
				RTL_W8(rOFDM1_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
				DEBUG_INFO("Path %c is on due to >= %d%%\n",
						   path_name[eRFPath], priv->pshare->phw->ant_on_criteria[eRFPath]);
				ant_on_processing = 1;
			}
		}
	}

	if (!ant_on_processing) {
		if (priv->pshare->phw->ant_off_num < 2) {
			for (eRFPath = RF92CD_PATH_A; eRFPath < priv->pshare->phw->NumTotalRFPath; eRFPath++) {
				if ((eRFPath == eRFPath_highest) || (priv->pshare->phw->ant_off_bitmap & BIT(eRFPath)))
					continue;

				if ((pstat->rf_info.mimorssi[eRFPath] < priv->pshare->rf_ft_var.ss_th_low) &&
						((highest_rssi - pstat->rf_info.mimorssi[eRFPath]) > priv->pshare->rf_ft_var.diff_th)) {
					priv->pshare->phw->ant_off_num++;
					priv->pshare->phw->ant_off_bitmap |= BIT(eRFPath);
					priv->pshare->phw->ant_on_criteria[eRFPath] = highest_rssi + 5;
					RTL_W8(rOFDM0_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
					RTL_W8(rOFDM1_TRxPathEnable, ~(priv->pshare->phw->ant_off_bitmap) & 0x0f);
					DEBUG_INFO("Path %c is off due to under th_low %d%% and diff %d%%, will be on at %d%%\n",
							   path_name[eRFPath], priv->pshare->rf_ft_var.ss_th_low,
							   (highest_rssi - pstat->rf_info.mimorssi[eRFPath]),
							   priv->pshare->phw->ant_on_criteria[eRFPath]);
					break;
				}
			}
		}
	}

	// For CCK
	if (priv->pshare->rf_ft_var.cck_sel_ver == 1) {
		if (under_ss_th_low && (pstat->rx_pkts > 20)) {
			if (priv->pshare->phw->ant_cck_sel != ((eRFPath_highest << 2) | eRFPath_higher)) {
				priv->pshare->phw->ant_cck_sel = ((eRFPath_highest << 2) | eRFPath_higher);
				RTL_W8(0xa07, (RTL_R8(0xa07) & 0xf0) | priv->pshare->phw->ant_cck_sel);
				DEBUG_INFO("CCK select default: path %c, optional: path %c\n",
						   path_name[eRFPath_highest], path_name[eRFPath_higher]);
			}
		}
	}
}


// dynamic Rx path selection by signal strength
void rx_path_by_rssi_cck_v2(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int highest_rssi = -1000, higher_rssi = -1000;
	RF92CD_RADIO_PATH_E eRFPath, eRFPath_highest = 0, eRFPath_higher = 0;
#ifdef _DEBUG_RTL8192CD_
	char path_name[] = {'A', 'B'};
#endif

	for (eRFPath = RF92CD_PATH_A; eRFPath < priv->pshare->phw->NumTotalRFPath; eRFPath++) {
		if (pstat->cck_mimorssi_total[eRFPath] > highest_rssi) {
			higher_rssi = highest_rssi;
			eRFPath_higher = eRFPath_highest;
			highest_rssi = pstat->cck_mimorssi_total[eRFPath];
			eRFPath_highest = eRFPath;
		}

		else if (pstat->cck_mimorssi_total[eRFPath] > higher_rssi) {
			higher_rssi = pstat->cck_mimorssi_total[eRFPath];
			eRFPath_higher = eRFPath;
		}
	}

	if (priv->pshare->phw->ant_cck_sel != ((eRFPath_highest << 2) | eRFPath_higher)) {
		priv->pshare->phw->ant_cck_sel = ((eRFPath_highest << 2) | eRFPath_higher);
		RTL_W8(0xa07, (RTL_R8(0xa07) & 0xf0) | priv->pshare->phw->ant_cck_sel);
		DEBUG_INFO("CCK rssi A:%d B:%d C:%d D:%d accu %d pkts\n", pstat->cck_mimorssi_total[0],
				   pstat->cck_mimorssi_total[1], pstat->cck_mimorssi_total[2], pstat->cck_mimorssi_total[3], pstat->cck_rssi_num);
		DEBUG_INFO("CCK select default: path %c, optional: path %c\n",
				   path_name[eRFPath_highest], path_name[eRFPath_higher]);
	}
}


// Tx power control
void tx_power_control(struct rtl8192cd_priv *priv, struct stat_info *pstat, int enable)
{
	if (enable) {
		if (!priv->pshare->phw->lower_tx_power) {
			// TX High power enable
//			set_fw_reg(priv, 0xfd000009, 0, 0);
			if (!priv->pshare->bcnTxAGC)
				RTL_W8(0x364, RTL_R8(0x364) | FW_REG364_HP);
			priv->pshare->phw->lower_tx_power++;

			if ((!priv->pshare->is_40m_bw || (pstat->tx_bw == HT_CHANNEL_WIDTH_20)) &&
					(!pstat->is_rtl8190_sta && !pstat->is_broadcom_sta && !pstat->is_marvell_sta && !pstat->is_intel_sta))
				set_fw_reg(priv, 0xfd004314, 0, 0);
			else
				set_fw_reg(priv, 0xfd000015, 0, 0);
		}
	} else {
		if (priv->pshare->phw->lower_tx_power) {
			//TX High power disable
//			set_fw_reg(priv, 0xfd000008, 0, 0);
			RTL_W8(0x364, RTL_R8(0x364) & ~FW_REG364_HP);
			priv->pshare->phw->lower_tx_power = 0;
		}
	}
}


void tx_power_tracking(struct rtl8192cd_priv *priv)
{
	if (priv->pmib->dot11RFEntry.ther) {
		DEBUG_INFO("TPT: triggered(every %d seconds)\n", priv->pshare->rf_ft_var.tpt_period);

		// enable rf reg 0x24 power and trigger, to get ther value in 1 second
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x24, bMask20Bits, 0x60);
		mod_timer(&priv->pshare->phw->tpt_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(1000)); // 1000ms
	}
}


void rtl8192cd_tpt_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	unsigned int val32;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (timer_pending(&priv->pshare->phw->tpt_timer))
		del_timer_sync(&priv->pshare->phw->tpt_timer);

	if (priv->pmib->dot11RFEntry.ther) {
		// query rf reg 0x24[4:0], for thermal meter value
		val32 = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x24, bMask20Bits, 1) & 0x01f;

		if (val32) {
			set_fw_reg(priv, 0xfd000019 | (priv->pmib->dot11RFEntry.ther & 0xff) << 8 | val32 << 16, 0, 0);
			DEBUG_INFO("TPT: finished once (ther: current=0x%02x, target=0x%02x)\n",
					   val32, priv->pmib->dot11RFEntry.ther);
		} else {
			DEBUG_WARN("TPT: cannot finish, since wrong current ther value report\n");
		}
	}
}
#endif


/*
 *
 * CAM related functions
 *
 */

/*******************************************************/
/*CAM related utility                                  */
/*CamAddOneEntry                                       */
/*CamDeleteOneEntry                                    */
/*CamResetAllEntry                                     */
/*******************************************************/
#define TOTAL_CAM_ENTRY (priv->pshare->total_cam_entry)

#define CAM_CONTENT_COUNT 8
#define CAM_CONTENT_USABLE_COUNT 6

#define CFG_VALID        BIT(15)
#define CFG_RPT_MD       BIT(8)


static UCHAR CAM_find_usable(struct rtl8192cd_priv *priv)
{
	unsigned long command = 0, content = 0;
	unsigned char index;
	int for_begin = 4;
	int round = 0;
#ifdef CONFIG_WLAN_HAL
	u4Byte retVal;
#endif // CONFIG_WLAN_HAL

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		for_begin = 3;

#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
			for_begin = 0;
#endif
#ifdef UNIVERSAL_REPEATER
		if (IS_VXD_INTERFACE(priv)) {
			for_begin = 0;
		} else {
			if (IS_ROOT_INTERFACE(priv)) {
				if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
					for_begin = 0;
			}
		}
#endif
	}
#endif

	if(GET_CHIP_VER(priv) >= VERSION_8812E) {
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
		for_begin = RTL8192CD_NUM_VWLAN + 1;
#else
		for_begin = 0;
#endif
	}

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
#ifdef USE_WEP_4_KEYS
		for_begin = 0;
#endif
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
		for_begin = RTL8192CD_NUM_VWLAN + 1;
#endif
		retVal = GET_HAL_INTERFACE(priv)->CAMFindUsableHandler(priv, for_begin);
		return (unsigned char)retVal;
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
		for (index = for_begin; index < TOTAL_CAM_ENTRY; index++) {
			// polling bit, and No Write enable, and address
			command = CAM_CONTENT_COUNT * index;
			RTL_W32(CAMCMD, (SECCAM_POLL | command));

			// Check polling bit is clear
			while (1) {
				command = RTL_R32(CAMCMD);
				if (command & SECCAM_POLL)
					continue;
				else
					break;

				if (++round > 10000) {
					panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
					break;
				}
			}
			content = RTL_R32(CAMREAD);

			// check valid bit. if not valid,
			if ((content & CFG_VALID) == 0) {
				return index;
			}
		}
	}
	return TOTAL_CAM_ENTRY;
}


static void CAM_program_entry(struct rtl8192cd_priv *priv, unsigned char index, unsigned char* macad,
							  unsigned char* key128, unsigned short config)
{
	unsigned long target_command = 0, target_content = 0;
	signed char entry_i = 0;
	struct stat_info *pstat;

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
		GET_HAL_INTERFACE(priv)->CAMProgramEntryHandler(
			priv,
			index,
			macad,
			key128,
			config
		);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
		for (entry_i = (CAM_CONTENT_USABLE_COUNT-1); entry_i >= 0; entry_i--) {
			// polling bit, and write enable, and address
			target_command = entry_i + CAM_CONTENT_COUNT * index;
			target_command = target_command | SECCAM_POLL | SECCAM_WE;
			if (entry_i == 0) {
				//first 32-bit is MAC address and CFG field
				target_content = (ULONG)(*(macad + 0)) << 16
								 | (ULONG)(*(macad + 1)) << 24
								 | (ULONG)config;
				target_content = target_content | config;
			} else if (entry_i == 1) {
				//second 32-bit is MAC address
				target_content = (ULONG)(*(macad + 5)) << 24
								 | (ULONG)(*(macad + 4)) << 16
								 | (ULONG)(*(macad + 3)) << 8
								 | (ULONG)(*(macad + 2));
			} else {
				target_content = (ULONG)(*(key128 + (entry_i * 4 - 8) + 3)) << 24
								 | (ULONG)(*(key128 + (entry_i * 4 - 8) + 2)) << 16
								 | (ULONG)(*(key128 + (entry_i * 4 - 8) + 1)) << 8
								 | (ULONG)(*(key128 + (entry_i * 4 - 8) + 0));
			}

			RTL_W32(CAMWRITE, target_content);
			RTL_W32(CAMCMD, target_command);
		}

		target_content = RTL_R32(CR);
		if ((target_content & MAC_SEC_EN) == 0)
			RTL_W32(CR, (target_content | MAC_SEC_EN));
	}

	pstat = get_stainfo(priv, macad);
	if (pstat) {
		pstat->cam_id = index;
	}
// move above
#if 0
	target_content = RTL_R32(CR);
	if ((target_content & MAC_SEC_EN) == 0)
		RTL_W32(CR, (target_content | MAC_SEC_EN));
#endif
}


int CamAddOneEntry(struct rtl8192cd_priv *priv, unsigned char *pucMacAddr, unsigned long keyId,
				   unsigned long encAlg, unsigned long useDK, unsigned char *pKey)
{
	unsigned char retVal = 0, camIndex = 0, wpaContent = 0;
	unsigned short usConfig = 0;
	unsigned int set_dk_margin = 4;

	//use Hardware Polling to check the valid bit.
	//in reality it should be done by software link-list
	if ((!memcmp(pucMacAddr, "\xff\xff\xff\xff\xff\xff", 6)) || (useDK
#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
			&& ((encAlg >> 2) != DOT11_ENC_WAPI)
#endif
		)) {
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
		camIndex = priv->vap_init_seq;
#else
		camIndex = keyId;
#endif
	}
	else
		camIndex = CAM_find_usable(priv);

	if (camIndex == TOTAL_CAM_ENTRY)
		return retVal;

	usConfig = usConfig | CFG_VALID | ((USHORT)(encAlg)) | (UCHAR)keyId;
#ifdef MULTI_MAC_CLONE
	if ((OPMODE & WIFI_STATION_STATE) && priv->pmib->ethBrExtInfo.macclone_enable)
		usConfig |= CFG_RPT_MD;
#endif

#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
	if ((encAlg >> 2) == DOT11_ENC_WAPI) {
		//ulUseDK is used to diff Parwise and Group
		//if sw crypto or enable vap, wapi driver does not place the multiple key to cam
		if((!SWCRYPTO) && 
#ifdef MBSSID
		(!(GET_ROOT(priv)->pmib->miscEntry.vap_enable))  &&
#endif
		camIndex < 2
		)
		//if (camIndex < 2) //is group key
			usConfig |= BIT(6);

		if (useDK == 1) // ==0 sec key; == 1mic key
			usConfig |= BIT(5);

		useDK = 0;
	}
#endif

	CAM_program_entry(priv, camIndex, pucMacAddr, pKey, usConfig);

	if (priv->pshare->CamEntryOccupied == 0) {
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
			SECURITY_CONFIG_OPERATION  SCO;

			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
			if (useDK == 1) {
				SCO |= SCO_RXUSEDK | SCO_TXUSEDK;
			}
			SCO |= SCO_RXDEC | SCO_TXENC | SCO_NOSKMC | SCO_CHK_KEYID;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{//not HAL
			if (useDK == 1)
				wpaContent = RXUSEDK | TXUSEDK;
			RTL_W16(SECCFG, RTL_R16(SECCFG) | RXDEC | TXENC | wpaContent | NOSKMC | CHK_KEYID);
		}
	}

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		set_dk_margin = 3;

#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
			set_dk_margin = 0;
#endif
#ifdef UNIVERSAL_REPEATER
		if (IS_VXD_INTERFACE(priv)) {
			set_dk_margin = 0;
		} else {
			if (IS_ROOT_INTERFACE(priv)) {
				if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
					set_dk_margin = 0;
			}
		}
#endif
	}
#endif

	if(GET_CHIP_VER(priv) >= VERSION_8812E)
		set_dk_margin = 0;

#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
	set_dk_margin = 0;
#endif

//for WAPI, camIndex 0 and 1 are used to store broadcast keys (crypto key and mic key)	
#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
	if ((encAlg >> 2) == DOT11_ENC_WAPI) 
	{
		if((!SWCRYPTO) && 
#ifdef MBSSID
		(!(GET_ROOT(priv)->pmib->miscEntry.vap_enable))  &&
#endif
		GET_CHIP_VER(priv) >= VERSION_8812E
		)
			set_dk_margin = 2;
	}
#endif	

	if (camIndex < set_dk_margin) {
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
			SECURITY_CONFIG_OPERATION  SCO;
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
			SCO = (SCO & ~SCO_NOSKMC) | (SCO_RXBCUSEDK | SCO_TXBCUSEDK);
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{//not HAL
			RTL_W16(SECCFG, (RTL_R16(SECCFG) & ~NOSKMC) | (RXBCUSEDK | TXBCUSEDK));
		}
	}

	return 1;
}

#if(CONFIG_WLAN_NOT_HAL_EXIST==1)
void CAM_read_mac_config(struct rtl8192cd_priv *priv, unsigned char index, unsigned char* pMacad,
						 unsigned short* pTempConfig)
{
	unsigned long command = 0, content = 0;
	int round;

	// polling bit, and No Write enable, and address
	// cam address...
	// first 32-bit
	command = CAM_CONTENT_COUNT * index + 0;
	command = command | SECCAM_POLL;
	RTL_W32(CAMCMD, command);

	//Check polling bit is clear
	round = 0;
	while (1) {
#ifdef CONFIG_SDIO_HCI
		{
			s32 err;
			command = sdio_read32(priv, CAMCMD, &err);
			if (-ENOMEDIUM == err)
				return;
		}
#else
		command = RTL_R32(CAMCMD);
#endif
		if (command & SECCAM_POLL)
			continue;
		else
			break;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}
	content = RTL_R32(CAMREAD);

	//first 32-bit is MAC address and CFG field
	*(pMacad + 0) = (UCHAR)((content >> 16) & 0x000000FF);
	*(pMacad + 1) = (UCHAR)((content >> 24) & 0x000000FF);
	*pTempConfig  = (USHORT)(content & 0x0000FFFF);

	command = CAM_CONTENT_COUNT * index + 1;
	command = command | SECCAM_POLL;
	RTL_W32(CAMCMD, command);

	//Check polling bit is clear
	round = 0;
	while (1) {
#ifdef CONFIG_SDIO_HCI
		{
			s32 err;
			command = sdio_read32(priv, CAMCMD, &err);
			if (-ENOMEDIUM == err)
				return;
		}
#else
		command = RTL_R32(CAMCMD);
#endif
		if (command & SECCAM_POLL)
			continue;
		else
			break;

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}
	content = RTL_R32(CAMREAD);

	*(pMacad + 5) = (UCHAR)((content >> 24) & 0x000000FF);
	*(pMacad + 4) = (UCHAR)((content >> 16) & 0x000000FF);
	*(pMacad + 3) = (UCHAR)((content >> 8) & 0x000000FF);
	*(pMacad + 2) = (UCHAR)((content) & 0x000000FF);
}
#else
void CAM_read_mac_config(struct rtl8192cd_priv *priv, unsigned char index, unsigned char* pMacad,
						 unsigned short* pTempConfig)
{
	return;
}
#endif//CONFIG_WLAN_NOT_HAL_EXIST


#if 0
void CAM_mark_invalid(struct rtl8192cd_priv *priv, UCHAR ucIndex)
{
	ULONG ulCommand = 0;
	ULONG ulContent = 0;

	// polling bit, and No Write enable, and address
	ulCommand = CAM_CONTENT_COUNT * ucIndex;
	ulCommand = ulCommand | _CAM_POLL_ | _CAM_WE_;
	// write content 0 is equall to mark invalid
	RTL_W32(_CAM_W_, ulContent);
	RTL_W32(_CAMCMD_, ulCommand);
}
#endif


static void CAM_empty_entry(struct rtl8192cd_priv *priv, unsigned char index)
{
	unsigned long command = 0, content = 0;
	unsigned int i;

	for (i = 0; i < CAM_CONTENT_COUNT; i++) {
		// polling bit, and No Write enable, and address
		command = CAM_CONTENT_COUNT * index + i;
		command = command | SECCAM_POLL | SECCAM_WE;
		// write content 0 is equal to mark invalid
		RTL_W32(CAMWRITE, content);
		RTL_W32(CAMCMD, command);
	}
}


int CamDeleteOneEntry(struct rtl8192cd_priv *priv, unsigned char *pMacAddr, unsigned long keyId, unsigned int useDK)
{
	unsigned char ucIndex;
	unsigned char ucTempMAC[6];
	unsigned short usTempConfig = 0;
	int for_begin = 4;

#ifdef  CONFIG_WLAN_HAL
	CAM_ENTRY_CFG   CamEntryCfg;
#endif

	// group key processing
	if ((!memcmp(pMacAddr, "\xff\xff\xff\xff\xff\xff", 6)) || (useDK)) {
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
			GET_HAL_INTERFACE(priv)->CAMReadMACConfigHandler(priv, priv->vap_init_seq, ucTempMAC, &CamEntryCfg);
#else
			GET_HAL_INTERFACE(priv)->CAMReadMACConfigHandler(priv, keyId, ucTempMAC, &CamEntryCfg);
#endif
			if ( _TRUE == CamEntryCfg.bValid ) {
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
				GET_HAL_INTERFACE(priv)->CAMEmptyEntryHandler(priv, priv->vap_init_seq);
#else
				GET_HAL_INTERFACE(priv)->CAMEmptyEntryHandler(priv, keyId);
#endif
				if (priv->pshare->CamEntryOccupied == 1) {
					SECURITY_CONFIG_OPERATION  SCO = 0;
					GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
				}

				return 1;
			} else {
				return 0;
			}
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{//not HAL
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
			CAM_read_mac_config(priv, priv->vap_init_seq, ucTempMAC, &usTempConfig);
#else
			CAM_read_mac_config(priv, keyId, ucTempMAC, &usTempConfig);
#endif
			if (usTempConfig & CFG_VALID) {
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
				CAM_empty_entry(priv, priv->vap_init_seq);
#else
				CAM_empty_entry(priv, keyId);
#endif
				if (priv->pshare->CamEntryOccupied == 1)
					RTL_W16(SECCFG, 0);
				return 1;
			} else
				return 0;
		}
	}

#ifdef  CONFIG_WLAN_HAL
	// TODO:    check 8881A desgin below
#ifdef USE_WEP_4_KEYS
	for_begin = 0;
#endif
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
	for_begin = RTL8192CD_NUM_VWLAN + 1;
#endif
#endif  //CONFIG_WLAN_HAL

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		for_begin = 3;

#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
			for_begin = 0;
#endif
#ifdef UNIVERSAL_REPEATER
		if (IS_VXD_INTERFACE(priv)) {
			for_begin = 0;
		} else {
			if (IS_ROOT_INTERFACE(priv)) {
				if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
					for_begin = 0;
			}
		}
#endif
	}
#endif

	if(GET_CHIP_VER(priv) >= VERSION_8812E) {
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
		for_begin = RTL8192CD_NUM_VWLAN + 1;
#else
		for_begin = 0;
#endif
	}

	// unicast key processing
	// key processing for RTL818X(B) series
	for (ucIndex = for_begin; ucIndex < TOTAL_CAM_ENTRY; ucIndex++) {
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
			GET_HAL_INTERFACE(priv)->CAMReadMACConfigHandler(priv, ucIndex, ucTempMAC, &CamEntryCfg);
			if (!memcmp(pMacAddr, ucTempMAC, 6)) {
#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
				if ( (CamEntryCfg.EncAlgo == DOT11_ENC_WAPI) && (CamEntryCfg.KeyID != keyId)) {
					continue;
				}
#endif
#ifdef USE_WEP_4_KEYS
				if (((CamEntryCfg.EncAlgo == DOT11_ENC_WEP40)||(CamEntryCfg.EncAlgo == DOT11_ENC_WEP104) )
					&& (CamEntryCfg.KeyID != keyId)) {
					continue;
				}
#endif				

				GET_HAL_INTERFACE(priv)->CAMEmptyEntryHandler(priv, ucIndex);

				if (priv->pshare->CamEntryOccupied == 1) {
					SECURITY_CONFIG_OPERATION  SCO = 0;
					GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_SECURITY_CONFIG, (pu1Byte)&SCO);
				}
				return 1;
			}
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{//not HAL
			CAM_read_mac_config(priv, ucIndex, ucTempMAC, &usTempConfig);
			if (!memcmp(pMacAddr, ucTempMAC, 6)) {

#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
				if ((((usTempConfig & 0x1c) >> 2) == DOT11_ENC_WAPI) && ((usTempConfig & 0x3) != keyId))
					continue;
#endif
				CAM_empty_entry(priv, ucIndex);	// reset MAC address, david+2007-1-15

				if (priv->pshare->CamEntryOccupied == 1)
					RTL_W16(SECCFG, 0);

				return 1;
			}
		}
	}
	return 0;
}


/*now use empty to fill in the first 4 entries*/
void CamResetAllEntry(struct rtl8192cd_priv *priv)
{
#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_CAM_RESET_ALL_ENTRY, NULL);
	} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{
		unsigned char index;

		RTL_W32(CAMCMD, SECCAM_CLR);

		for (index = 0; index < TOTAL_CAM_ENTRY; index++)
			CAM_empty_entry(priv, index);

		RTL_W32(CR, RTL_R32(CR) & (~MAC_SEC_EN));
	}
	priv->pshare->CamEntryOccupied = 0;
	priv->pmib->dot11GroupKeysTable.keyInCam = 0;

//	RTL_W32(CR, RTL_R32(CR) & (~MAC_SEC_EN));
}


void CAM_read_entry(struct rtl8192cd_priv *priv, unsigned char index, unsigned char* macad,
					unsigned char* key128, unsigned short* config)
{
	unsigned long  target_command = 0, target_content = 0;
	unsigned char entry_i = 0;
	unsigned long status;
	int round = 0;

	for (entry_i = 0; entry_i < CAM_CONTENT_USABLE_COUNT; entry_i++) {
		// polling bit, and No Write enable, and address
		target_command = (unsigned long)(entry_i + CAM_CONTENT_COUNT * index);
		target_command = target_command | SECCAM_POLL;

		RTL_W32(CAMCMD, target_command);
		//Check polling bit is clear
		while (1) {
			status = RTL_R32(CAMCMD);
			if (status & SECCAM_POLL)
				continue;
			else
				break;

			if (++round > 10000) {
				panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
				break;
			}
		}
		target_content = RTL_R32(CAMREAD);

		if (entry_i == 0) {
			//first 32-bit is MAC address and CFG field
			*(config)  = (unsigned short)((target_content) & 0x0000FFFF);
			*(macad + 0) = (unsigned char)((target_content >> 16) & 0x000000FF);
			*(macad + 1) = (unsigned char)((target_content >> 24) & 0x000000FF);
		} else if (entry_i == 1) {
			*(macad + 5) = (unsigned char)((target_content >> 24) & 0x000000FF);
			*(macad + 4) = (unsigned char)((target_content >> 16) & 0x000000FF);
			*(macad + 3) = (unsigned char)((target_content >> 8) & 0x000000FF);
			*(macad + 2) = (unsigned char)((target_content) & 0x000000FF);
		} else {
			*(key128 + (entry_i * 4 - 8) + 3) = (unsigned char)((target_content >> 24) & 0x000000FF);
			*(key128 + (entry_i * 4 - 8) + 2) = (unsigned char)((target_content >> 16) & 0x000000FF);
			*(key128 + (entry_i * 4 - 8) + 1) = (unsigned char)((target_content >> 8) & 0x000000FF);
			*(key128 + (entry_i * 4 - 8) + 0) = (unsigned char)(target_content & 0x000000FF);
		}

		target_content = 0;
	}
}


#ifdef CAM_SWAP

void dump_mac3(struct rtl8192cd_priv *priv, unsigned char *mac)
{
	if(mac)
		printk(" %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
}


int is_hw_encrypt(struct stat_info	*pstat)
{

	if(pstat->dot11KeyMapping.keyInCam)
		return 1;
	else
		return 0;
}

void swap_to_sw_encrypt(struct rtl8192cd_priv *priv, struct stat_info	 *pstat)
{
	int ret = 0;

	//dump_mac3(priv, pstat->hwaddr);

	ret = CamDeleteOneEntry(priv, pstat->hwaddr, pstat->dot11KeyMapping.keyid, 0);

	pstat->dot11KeyMapping.keyInCam = 0;
	priv->pshare->CamEntryOccupied--;
}

void swap_to_hw_encrypt(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int ret = 0;

	//dump_mac3(priv, pstat->hwaddr);

	ret = CamAddOneEntry(priv, pstat->hwaddr, pstat->dot11KeyMapping.keyid, 
		pstat->dot11KeyMapping.dot11Privacy<<2, 0, GET_UNICAST_ENCRYP_KEY);

	pstat->dot11KeyMapping.keyInCam = 1;
	priv->pshare->CamEntryOccupied++;
}

struct sta_list{
	struct stat_info *pstat_array[NUM_STAT];
	unsigned char	num;
};

static struct sta_list sta_idle_hw_cam[NUM_STAT];
static struct sta_list sta_low_hw_cam[NUM_STAT];
static struct sta_list sta_mid_sw_cam[NUM_STAT];
static struct sta_list sta_high_sw_cam[NUM_STAT];

int get_sw_encrypt_sta_num(struct rtl8192cd_priv *priv)
{
	int i;
	struct stat_info *pstat = NULL;
	int num = 0;
			
	for(i=0; i<NUM_STAT; i++) { 
		if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)) {
			
			if (!((GET_MIB(priv->pshare->aidarray[i]->priv))->dot11OperationEntry.opmode & WIFI_AP_STATE))
				continue;
	
			pstat = &(priv->pshare->aidarray[i]->station);
		
			if((pstat->wpa_sta_info->RSNEnabled) && (pstat->dot11KeyMapping.keyInCam==0))
				num ++;
		}
	}

	return num;

}

void get_sta_traffic_level(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int total_cnt = pstat->traffic.delta_tx_bytes + pstat->traffic.delta_rx_bytes;

	//printk("%s +++ \n", __FUNCTION__);
	
	if(total_cnt <= TRAFFIC_THRESHOLD_LOW)
		pstat->traffic.level = STA_TRAFFIC_IDLE;
#if 0
	else if(total_cnt <= TRAFFIC_THRESHOLD_MID)
		pstat->traffic.level = STA_TRAFFIC_LOW;
	else if(total_cnt <= TRAFFIC_THRESHOLD_HIGH)
		pstat->traffic.level = STA_TRAFFIC_MID;
#endif
	else if(total_cnt > TRAFFIC_THRESHOLD_HIGH)
		pstat->traffic.level = STA_TRAFFIC_HIGH;
	else
		pstat->traffic.level = STA_TRAFFIC_DONT_CARE;

	//printk("total_cnt=%d level=%d\n", total_cnt, pstat->traffic.level);

}

void add_sta_traffic_list(struct stat_info *pstat)
{
	//printk("%s +++ \n", __FUNCTION__);
	//printk("pstat = 0x%x is_hw_encrypt=%d level=%d \n",
		//pstat, is_hw_encrypt(pstat), pstat->traffic.level);

	switch(pstat->traffic.level) {
		case STA_TRAFFIC_IDLE:
			if(is_hw_encrypt(pstat)){
				sta_idle_hw_cam->pstat_array[sta_idle_hw_cam->num] = pstat;
				sta_idle_hw_cam->num ++;
			}
			break;
#if 0
		case STA_TRAFFIC_LOW:
			if(is_hw_encrypt(pstat)){
				sta_low_hw_cam->pstat_array[sta_low_hw_cam->num] = pstat;
				sta_low_hw_cam->num ++;
			}
			break;

		case STA_TRAFFIC_MID:
			if(!is_hw_encrypt(pstat)){
				sta_mid_sw_cam->pstat_array[sta_mid_sw_cam->num] = pstat;
				sta_mid_sw_cam->num ++;
			}
			break;
#endif
		case STA_TRAFFIC_HIGH:
			if(!is_hw_encrypt(pstat)){
				sta_high_sw_cam->pstat_array[sta_high_sw_cam->num] = pstat;
				sta_high_sw_cam->num ++;
			}
			break;

	}

	//printk("cam num = %d %d %d %d \n", 
		//sta_idle_hw_cam->num, sta_low_hw_cam->num, 
		//sta_mid_sw_cam->num, sta_high_sw_cam->num);


}


void cal_sta_traffic(struct rtl8192cd_priv *priv, int sta_use_sw_encrypt)
{
	int i;
	struct stat_info *pstat = NULL;

	//printk("%s +++ \n", __FUNCTION__);

	sta_idle_hw_cam->num = 0;
	sta_low_hw_cam->num = 0;
	sta_mid_sw_cam->num = 0;
	sta_high_sw_cam->num = 0;

	for(i=0; i<NUM_STAT; i++) { 
		if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)) {
			
			if (!((GET_MIB(priv->pshare->aidarray[i]->priv))->dot11OperationEntry.opmode & WIFI_AP_STATE))
				continue;
	
			pstat = &(priv->pshare->aidarray[i]->station);
		
			if(pstat->wpa_sta_info->RSNEnabled){
				 pstat->traffic.delta_tx_bytes = pstat->tx_bytes - pstat->traffic.prev_tx_bytes;
				 pstat->traffic.delta_rx_bytes = pstat->rx_bytes - pstat->traffic.prev_rx_bytes;

				 pstat->traffic.prev_tx_bytes = pstat->tx_bytes;
				 pstat->traffic.prev_rx_bytes = pstat->rx_bytes;

				 get_sta_traffic_level(priv, pstat);

				 add_sta_traffic_list(pstat);
			}
		}
	}


}

void rotate_sta_cam(struct rtl8192cd_priv *priv, int sta_use_sw_encrypt)
{
	int i = 0;
	int num_remove_idle = 0;
	int num_remove_low = 0;

	struct stat_info *pstat_remove_hw = NULL;
	struct stat_info *pstat_move2_hw = NULL;

	//printk("%s +++ \n", __FUNCTION__);

	//printk("cam num = %d %d %d %d \n", 
		//sta_idle_hw_cam->num, sta_low_hw_cam->num, 
		//sta_mid_sw_cam->num, sta_high_sw_cam->num);
	
	
	for(i = 0; i<sta_high_sw_cam->num; i++){

		if(num_remove_idle < sta_idle_hw_cam->num){
			
			pstat_remove_hw = sta_idle_hw_cam->pstat_array[i];
			pstat_move2_hw = sta_high_sw_cam->pstat_array[i];

			swap_to_sw_encrypt(priv, pstat_remove_hw);
			swap_to_hw_encrypt(priv, pstat_move2_hw);
			
			num_remove_idle++; 
			continue; 
		}
#if 0
		if(num_remove_low < sta_low_hw_cam->num){

			pstat_remove_hw = sta_low_hw_cam->pstat_array[i];
			pstat_move2_hw = sta_high_sw_cam->pstat_array[i];

			swap_to_sw_encrypt(priv, pstat_remove_hw);
			swap_to_hw_encrypt(priv, pstat_move2_hw);
			
			num_remove_low++; 
		}
#endif

	}

#if 0
	if(num_remove_idle < sta_idle_hw_cam->num)
	for(i = 0; i<sta_mid_sw_cam->num; i++){

 		if(num_remove_idle < sta_idle_hw_cam->num){
			
			pstat_remove_hw = sta_idle_hw_cam->pstat_array[i];
			pstat_move2_hw = sta_mid_sw_cam->pstat_array[i];

			swap_to_sw_encrypt(priv, pstat_remove_hw);
			swap_to_hw_encrypt(priv, pstat_move2_hw);
			
			num_remove_idle++; 
			continue; 
		}

	}
#endif

}

#endif

#if 0
void debug_cam(UCHAR*TempOutputMac, UCHAR*TempOutputKey, USHORT TempOutputCfg)
{
	printk("MAC Address\n");
	printk(" %X %X %X %X %X %X\n", *TempOutputMac
		   , *(TempOutputMac + 1)
		   , *(TempOutputMac + 2)
		   , *(TempOutputMac + 3)
		   , *(TempOutputMac + 4)
		   , *(TempOutputMac + 5));
	printk("Config:\n");
	printk(" %X\n", TempOutputCfg);

	printk("Key:\n");
	printk("%X %X %X %X,%X %X %X %X,\n%X %X %X %X,%X %X %X %X\n"
		   , *TempOutputKey, *(TempOutputKey + 1), *(TempOutputKey + 2)
		   , *(TempOutputKey + 3), *(TempOutputKey + 4), *(TempOutputKey + 5)
		   , *(TempOutputKey + 6), *(TempOutputKey + 7), *(TempOutputKey + 8)
		   , *(TempOutputKey + 9), *(TempOutputKey + 10), *(TempOutputKey + 11)
		   , *(TempOutputKey + 12), *(TempOutputKey + 13), *(TempOutputKey + 14)
		   , *(TempOutputKey + 15));
}


void CamDumpAll(struct rtl8192cd_priv *priv)
{
	UCHAR TempOutputMac[6];
	UCHAR TempOutputKey[16];
	USHORT TempOutputCfg = 0;
	unsigned long flags;
	int i;

	SAVE_INT_AND_CLI(flags);
	for (i = 0; i < TOTAL_CAM_ENTRY; i++) {
		printk("%X-", i);
		CAM_read_entry(priv, i, TempOutputMac, TempOutputKey, &TempOutputCfg);
		debug_cam(TempOutputMac, TempOutputKey, TempOutputCfg);
		printk("\n\n");
	}
	RESTORE_INT(flags);
}


void CamDump4(struct rtl8192cd_priv *priv)
{
	UCHAR TempOutputMac[6];
	UCHAR TempOutputKey[16];
	USHORT TempOutputCfg = 0;
	unsigned long flags;
	int i;

	SAVE_INT_AND_CLI(flags);
	for (i = 0; i < 4; i++) {
		printk("%X", i);
		CAM_read_entry(priv, i, TempOutputMac, TempOutputKey, &TempOutputCfg);
		debug_cam(TempOutputMac, TempOutputKey, TempOutputCfg);
		printk("\n\n");
	}
	RESTORE_INT(flags);
}
#endif

#ifdef SDIO_AP_OFFLOAD
int offloadTestFunction(struct rtl8192cd_priv *priv, unsigned char *data)
{
	int mode;
	mode = _atoi(data, 16);

	if (strlen(data) == 0) {
		printk("Usage:\n");
		printk("offload 6 - Leave offload\n");
		return 0;
	}

	if (mode == 0x6) {
		schedule_work(&GET_ROOT(priv)->ap_cmd_queue);
	}
	
	return 0;
}
#endif // SDIO_AP_OFFLOAD

#ifdef CONFIG_OFFLOAD_FUNCTION
int offloadTestFunction(struct rtl8192cd_priv *priv, unsigned char *data)
{
	int mode = 0, status;
    unsigned long flags;
    u32 tmp32, haddr, saddr, cnt, ctr;
    PHCI_RX_DMA_MANAGER_88XX        prx_dma;
    PHCI_RX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q_tx;
    u4Byte RXBDReg;

	mode = _atoi(data, 16);
    haddr = 0xb8b00000; //pcie host addr
    saddr = 0xb8b10000; //pcie slave addr

	if (strlen(data) == 0) {
		printk("offloadTest 0x1 downlaod Rsvd page\n");
		printk("offloadTest 0x2: AP offload enable \n");

		return 0;
	}
	
	if (mode == 0x0) {		
		u1Byte reg_val;
		 //clear the non-valid bit	
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);
		reg_val = reg_val & ~BIT(6);
		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);
	}
	
	if (mode == 0x1) {
		printk("epdn 6e: download probe rsp\n");
		priv->offload_function_ctrl = 1;
		//RTL_W16(0x100 , RTL_R16(0x100) | BIT(8));		// enable sw beacon
//		tasklet_hi_schedule(&priv->pshare->rx_tasklet);
		//RTL_W8(HWSEQ_CTRL,RTL_R8(HWSEQ_CTRL) | BIT(6)); // HW SEQ

		u4Byte u4_val;
		
		// enable sw beacon
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
		u4_val = u4_val | BIT(8);
		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);

		u1Byte u1_val;

#if (IS_RTL8192E_SERIES | IS_RTL8881A_SERIES)
		 // HW SEQ
		GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HWSEQ_CTRL, (pu1Byte)&u1_val);
		u1_val = u1_val | BIT(6);
		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_HWSEQ_CTRL, (pu1Byte)&u1_val);
#endif
        
	}


	if (mode == 0x2)	 {

		unsigned char      loc_bcn[1];
		unsigned char      loc_probe[1];
        unsigned short     BcnHead;
        u1Byte temp_120count=0;
        u1Byte H2CCommand[2]={0};
        u1Byte reg_val;
        u4Byte u4_val;

#ifdef GPIO_WAKEPIN 
        if(IS_HARDWARE_TYPE_8822B(priv)) //for GPIO8 wakeup, test only
            RTL_W8(REG_LED_CFG+2,0x40);
#endif         
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_BCN_HEAD, (pu2Byte)&BcnHead); 

		loc_bcn[0] = priv->offload_bcn_page;
		loc_probe[0] = priv->offload_proc_page;

		printk("loc_bcn[0]= %x \n", loc_bcn[0]);
		printk("loc_probe[0]= %x \n", loc_probe[0]);
        
#if (IS_RTL8197F_SERIES || IS_RTL8822B_SERIES)
        if ( IS_HARDWARE_TYPE_8197F(priv) || IS_HARDWARE_TYPE_8822B(priv)) { //3081

            loc_bcn[0] = (u1Byte) (priv->offload_bcn_page - BcnHead);
            loc_probe[0] = (u1Byte) (priv->offload_proc_page - BcnHead);

            printk("short loc_bcn[0]= %x \n", loc_bcn[0]);
            printk("short loc_probe[0]= %x \n", loc_probe[0]);
        }
#endif

#ifdef CONFIG_SAPPS

        H2CCommand[0] = BIT0|BIT1;
        H2CCommand[1] = 20;
        FillH2CCmd88XX(priv,0x26,2,H2CCommand);
        printk("enter SAPPS 0x1c4=0x%x\n",RTL_R8(0x1c4));
#endif
		GET_HAL_INTERFACE(priv)->SetAPOffloadHandler(priv, 1, 1, 0, 0, loc_bcn, loc_probe);

        temp_120count=0;
        while( !(RTL_R8(0x284+2)&BIT2)){//tingchu 20161006 only check 0x284 bit18
			if(temp_120count > 100){
				printk("enable apoffload error, 0x120=0x%x, 0x284=0x%x, 0xc0=0x%x\n",RTL_R32(0x120),RTL_R32(0x284),RTL_R32(0xc0));
                printk("enable apoffload error\n");
            
                return FAIL;
				//break;
			}
			printk("wait apoffload enable\n");
            printk("0x1a0=0x%x,0x1ac=0x%x\n",RTL_R32(0x1a0),RTL_R32(0x1ac));
            delay_us(100);
            temp_120count++;

        }

        printk("apoffload enable success 0x1a0=0x%x,0x1ac=0x%x\n",RTL_R32(0x1a0),RTL_R32(0x1ac));
        
#ifdef CONFIG_32K
        RTL_W8(0x3d9,(RTL_R8(0x3d9)|BIT0)^BIT7);
        printk("enable 32k, 3d9h=0x%x, 0x1ac=0x%x\n",RTL_R8(0x3d9),RTL_R8(0x1ac));
        priv->offload_32k_flag = 1;
#endif
		delay_ms(10);
	}

	if (mode == 0x3) {
		GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 1 , 1);
	}

	if (mode == 0x4) {
		GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0 , 1);
	}
    
	if (mode == 0x05){
#ifdef PCIE_PME_WAKEUP_TEST//yllin         
        //setting PCIE
        u32 PME_enable;
        PME_enable = REG32(0xb8b0008c) | 0x8;
        REG32(0xb8b0008c) = PME_enable;
        
        PME_enable = REG32(0xb8b10044) | 0x100;
        REG32(0xb8b10044) = PME_enable;
#endif  	    
    }

    if (mode == 0x05){
#ifdef CONFIG_32K
        priv->offload_32k_flag = 0;
        RTL_W8(0x3d9,(RTL_R8(0x3d9)&~BIT0)^BIT7);
        printk("disable 32k\n");

#endif
    }

    
#ifdef PCIE_POWER_SAVING_TEST    
        if( mode == 0x7)//pcie goto L1
        {
            SAVE_INT_AND_CLI(flags);
            priv->pwr_state = PCIE_SETTING;
            // mac
            RTL_W32(PCIE_CTRL_REG, RTL_R32(PCIE_CTRL_REG)| 0x7FFF ); //stop pcie dma
            
            priv->b0IMR=RTL_R32(REG_HIMR0);
            priv->b8IMR=RTL_R32(REG_HIMR1);
    
            RTL_W32(REG_HIMR0, 0x0);
            RTL_W32(REG_HIMR1, 0x0);
            delay_ms(1000);
            
            // pcie link into L1
            tmp32 = REG32(saddr + 0x44) & ( ~(3)); //D0
            REG32(saddr + 0x44) = tmp32 | (3); //D3
            priv->pwr_state = L1;
            printk("D3 hot -> L1\n");
            RESTORE_INT(flags);
        }
    
    
        if(mode == 0x8)//pcie return to L0
        {   
            SAVE_INT_AND_CLI(flags);
            priv->pwr_state = PCIE_SETTING;
            // pcie link into L0
            cnt = 20;
            tmp32 = REG32(saddr + 0x44) & ( ~(3));  //D0
            REG32(saddr + 0x44) = tmp32 | (0);      //D0
            do {
                status = REG32(haddr + 0x728) & 0x1f;
                if (status == 0x11)
                    break;
                delay_ms(10);
            } while(--cnt);
    
            if (status != 0x11)
            {
                printk("change to L0 fail!!!, status=%x, MAC0\n", status);
                return 0;
            }
            // mac
            RTL_W32(PCIE_CTRL_REG, RTL_R32(PCIE_CTRL_REG)& ~0x7FFF ); //enable pcie dma
            priv->pwr_state = L0;
    
            RTL_W32(REG_HIMR0, priv->b0IMR);
            RTL_W32(REG_HIMR1, priv->b8IMR);
#if 0        
            //3 ------apofflaod disable-----
            u1Byte reg_val;
            u4Byte u4_val;
    
            GET_HAL_INTERFACE(priv)->SetAPOffloadHandler(priv, 0, 1, 0, 0, NULL, NULL);
            //restore setting
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);
            reg_val = reg_val | BIT(6);
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);
    
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
            u4_val = u4_val & ~BIT(8);
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
            
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val); //release RXDMA
            u4_val = u4_val & ~BIT(18);
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val);
            //3 ------apofflaod disable-----
#endif        
            printk("back to -> L0\n");
            RESTORE_INT(flags);
    
        }
        
        if (mode == 0x9) //pcie go to L2
        {  
            rtl8192cd_tx_dsr(priv);
            rtl8192cd_rx_isr(priv);
    
            u16 reg_host_idx,reg_hw_idx;
            SAVE_INT_AND_CLI(flags);
            priv->hwidx_error_flag = 0;
            RTL_W8(REG_RXPKT_NUM+2,  4); //stop Rx dma to host
    
            priv->pwr_state = PCIE_SETTING;
    
            ctr = 3000;
            do {
                delay_us(100);
                if ((RTL_R32(REG_Q0_INFO)& BIT(22)) && (RTL_R32(REG_Q1_INFO) & BIT(22)) && (RTL_R32(REG_Q2_INFO) & BIT(22)) && (RTL_R32(REG_Q3_INFO) & BIT(22)) && (RTL_R32(REG_HIQ_INFO) & BIT(22)) \
                           && (RTL_R32(REG_Q4_INFO)& BIT(22)) && (RTL_R32(REG_Q5_INFO)& BIT(22)) && (RTL_R32(REG_Q6_INFO)& BIT(22)) && (RTL_R32(REG_Q7_INFO)& BIT(22)) && (RTL_R32(REG_MGQ_INFO) & BIT(22))) {
                    break;
                }
            } while (--ctr);
            if (!ctr) {
                printk("Error,Q is not empty\n");
            }
    
            ctr = 5000;
            do {
                delay_us(100);
                if(RTL_R32(REG_RXPKT_NUM) & 0x00020000) //rx dma idle
                {
                    break;
                }
            } while (--ctr);
            if (!ctr) {
                printk("Error 2, 0x284=%x!!!\n",RTL_R32(REG_RXPKT_NUM));
            }
            
           
            RTL_W8(TXPAUSE, 0x2f);
            delay_ms(1);
    
            { //check hw idx
          
                int     tx_head, tx_tail, queueIndex;
                unsigned int tx_hw_reg;
                PTX_DESC_88XX             cur_notdma_txdesc;
                struct tx_desc      *phdesc, *pdesc;
                ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
                
                for (queueIndex = MGNT_QUEUE; queueIndex <= HIGH_QUEUE; queueIndex++) {
                    
                    cur_q_tx       = &(ptx_dma->tx_queue[queueIndex]);
                    
                    tx_hw_reg = RTL_R32(ptx_dma->tx_queue[queueIndex].reg_rwptr_idx);
                    
                    tx_head     = cur_q_tx->host_idx;
                    tx_tail     = cur_q_tx->hw_idx;
                    reg_host_idx = tx_hw_reg & 0xffff;
                    reg_hw_idx = (tx_hw_reg & 0xffff0000)>>16;
                    //printk("sw tx_head=%x, sw tx_tail=%x, reg_host_idx=%x, reg_hw_idx=%x\n",tx_head,tx_tail,reg_host_idx,reg_hw_idx);
                    if(reg_host_idx!=reg_hw_idx)
                        priv->hwidx_error_flag = 1;
                    
                }
                
    
    
                prx_dma         = (PHCI_RX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PRxDMA88XX);
                cur_q           = &(prx_dma->rx_queue[0]);
                //printk("RX cur_host_idx=%x, host_idx=%x, hw_idx=%x, reg=%x\n",cur_q->cur_host_idx,cur_q->host_idx,cur_q->hw_idx,RTL_R32(REG_RXQ_RXBD_IDX));
    
               
            }
       
            if(priv->hwidx_error_flag == 1){
                
                RTL_W8(REG_RXPKT_NUM+2,  RTL_R8(REG_RXPKT_NUM+2) & ~BIT(2));
                
                RTL_W8(TXPAUSE, 0x0);
                priv->pwr_state = L0;
                printk("No enter L2 --> wrong HW_IDX\n");
            }       
            else         
            {
    
            RTL_W16(CR , RTL_R16(CR) | ENSWBCN);        // enable sw beacon
    
            RTL_W8(CR, 0xFC);
            
            RTL_W32(PCIE_CTRL_REG, RTL_R32(PCIE_CTRL_REG)| 0x7FFF ); //stop pcie dma
    
            RTL_W8(PCIE_CTRL_REG + 2, 0x2); // sw L123
            REG32(saddr + 0x44) |= 0x8108;      // clear pme status
            
            priv->b0IMR=RTL_R32(REG_HIMR0);
            priv->b8IMR=RTL_R32(REG_HIMR1);
    
            RTL_W32(REG_HIMR0, 0x0);
            RTL_W32(REG_HIMR1, 0x0);
    
            
             
            RTL_W8(RSV_CTRL0, 0xe0);        // reg lock, dis_prst
            RTL_W8(RSV_CTRL0, 0xe0);
            priv->pwr_state = L2;
    
           
            delay_ms(100);
            REG32(haddr + 0x1008) |= (0x200);
            delay_ms(10);
            RESTORE_INT(flags);
    
            printk("PME turn off -> L2\n");
            
            }
        }
        
        if (mode == 0xa) //pcie return to L0
        {
            if(priv->hwidx_error_flag == 1)
                return;
            
            SAVE_INT_AND_CLI(flags);
    
            PCIE_reset_procedure3_V2(priv);
            setBaseAddressRegister_V2();
    
            priv->pwr_state = PCIE_SETTING;
    
            RTL_W8(CR, 0xFC);
            RTL_W32(PCIE_CTRL_REG, RTL_R32(PCIE_CTRL_REG)| 0x7FFF ); //stop pcie dma
            
    
            RTL_W8(RSV_CTRL0, 0x0);
      
          
            prx_dma         = (PHCI_RX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PRxDMA88XX);
            cur_q           = &(prx_dma->rx_queue[0]);
            cur_q->cur_host_idx = 0;
            cur_q->host_idx = 0;
            cur_q->hw_idx = 0;
    
            RTL_W32(REG_RXQ_RXBD_DESA, _GET_HAL_DATA(priv)->ring_dma_addr);
            RTL_W32(REG_RXQ_RXBD_IDX,0);
            
            int txQ_i=0;
            ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
            
            
            for(txQ_i=0;txQ_i<HCI_TX_DMA_QUEUE_MAX_NUM;txQ_i++){
                
                cur_q_tx       = &(ptx_dma->tx_queue[txQ_i]);
                cur_q_tx->host_idx = 0;
                cur_q_tx->hw_idx = 0;
    
            }
            RTL_W32(REG_BCNQ_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_BCN]);
            RTL_W32(REG_MGQ_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_MGT]);
            RTL_W32(REG_VOQ_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_VO]);
            RTL_W32(REG_VIQ_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_VI]);
            RTL_W32(REG_BEQ_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_BE]);
            RTL_W32(REG_BKQ_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_BK]);
            RTL_W32(REG_HI0Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI0]);
            RTL_W32(REG_HI1Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI1]);
            RTL_W32(REG_HI2Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI2]);
            RTL_W32(REG_HI3Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI3]);
            RTL_W32(REG_HI4Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI4]);
            RTL_W32(REG_HI5Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI5]);
            RTL_W32(REG_HI6Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI6]);
            RTL_W32(REG_HI7Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI7]);
            RTL_W32(REG_BD_RWPTR_CLR,0xffffffff);
            RTL_W32(REG_MGQ_TXBD_IDX,0);
            RTL_W32(REG_VOQ_TXBD_IDX,0);
            RTL_W32(REG_VIQ_TXBD_IDX,0);
            RTL_W32(REG_BEQ_TXBD_IDX,0);
            RTL_W32(REG_BKQ_TXBD_IDX,0);
            RTL_W32(REG_HI0Q_TXBD_IDX,0);
            RTL_W32(REG_HI1Q_TXBD_IDX,0);
            RTL_W32(REG_HI2Q_TXBD_IDX,0);
            RTL_W32(REG_HI3Q_TXBD_IDX,0);
            RTL_W32(REG_HI4Q_TXBD_IDX,0);
            RTL_W32(REG_HI5Q_TXBD_IDX,0);
            RTL_W32(REG_HI6Q_TXBD_IDX,0);
            RTL_W32(REG_HI7Q_TXBD_IDX,0);
    
#ifdef DELAY_REFILL_RX_BUF
            priv->pshare->phw->cur_rx_refill = 0;
#endif
    
    
    
    //3    ===Set TXBD Mode and Number===
            RTL_W16(REG_MGQ_TXBD_NUM, BIT_MGQ_DESC_MODE(TX_MGQ_TXBD_MODE_SEL) |
                        BIT_MGQ_DESC_NUM(TX_MGQ_TXBD_NUM));
            RTL_W16(REG_VOQ_TXBD_NUM, BIT_VOQ_DESC_MODE(TX_VOQ_TXBD_MODE_SEL) |
                        BIT_VOQ_DESC_NUM(TX_VOQ_TXBD_NUM));
            RTL_W16(REG_VIQ_TXBD_NUM, BIT_VIQ_DESC_MODE(TX_VIQ_TXBD_MODE_SEL) |
                        BIT_VIQ_DESC_NUM(TX_VIQ_TXBD_NUM));
            RTL_W16(REG_BEQ_TXBD_NUM, BIT_BEQ_DESC_MODE(TX_BEQ_TXBD_MODE_SEL) |
                        BIT_BEQ_DESC_NUM(TX_BEQ_TXBD_NUM));
            RTL_W16(REG_BKQ_TXBD_NUM, BIT_BKQ_DESC_MODE(TX_BKQ_TXBD_MODE_SEL) |
                        BIT_BKQ_DESC_NUM(TX_BKQ_TXBD_NUM));
        
            RTL_W16(REG_HI0Q_TXBD_NUM, BIT_HI0Q_DESC_MODE(TX_HI0Q_TXBD_MODE_SEL) |
                        BIT_HI0Q_DESC_NUM(TX_HI0Q_TXBD_NUM));
            RTL_W16(REG_HI1Q_TXBD_NUM, BIT_HI1Q_DESC_MODE(TX_HI1Q_TXBD_MODE_SEL) |
                        BIT_HI1Q_DESC_NUM(TX_HI1Q_TXBD_NUM));
            RTL_W16(REG_HI2Q_TXBD_NUM, BIT_HI2Q_DESC_MODE(TX_HI2Q_TXBD_MODE_SEL) |
                        BIT_HI2Q_DESC_NUM(TX_HI2Q_TXBD_NUM));
            RTL_W16(REG_HI3Q_TXBD_NUM, BIT_HI3Q_DESC_MODE(TX_HI3Q_TXBD_MODE_SEL) |
                        BIT_HI3Q_DESC_NUM(TX_HI3Q_TXBD_NUM));
            RTL_W16(REG_HI4Q_TXBD_NUM, BIT_HI4Q_DESC_MODE(TX_HI4Q_TXBD_MODE_SEL) |
                        BIT_HI4Q_DESC_NUM(TX_HI4Q_TXBD_NUM));
            RTL_W16(REG_HI5Q_TXBD_NUM, BIT_HI5Q_DESC_MODE(TX_HI5Q_TXBD_MODE_SEL) |
                        BIT_HI5Q_DESC_NUM(TX_HI5Q_TXBD_NUM));
            RTL_W16(REG_HI6Q_TXBD_NUM, BIT_HI6Q_DESC_MODE(TX_HI6Q_TXBD_MODE_SEL) |
                        BIT_HI6Q_DESC_NUM(TX_HI6Q_TXBD_NUM));
            RTL_W16(REG_HI7Q_TXBD_NUM, BIT_HI7Q_DESC_MODE(TX_HI7Q_TXBD_MODE_SEL) |
                        BIT_HI7Q_DESC_NUM(TX_HI7Q_TXBD_NUM));
        
#if IS_EXIST_RTL8822BE || IS_EXIST_RTL8197FEM
            RTL_W16(REG_H2CQ_TXBD_NUM, BIT_H2CQ_DESC_MODE(TX_H2CQ_TXBD_MODE_SEL) |
                        BIT_H2CQ_DESC_NUM(TX_CMDQ_TXBD_NUM));
#endif //IS_EXIST_RTL8822BE
    
    
    
            //3 ===Set Beacon Mode: 2, 4, or 8 segment each descriptor===
            RXBDReg = RTL_R16(REG_RX_RXBD_NUM);
            RXBDReg = (RXBDReg & ~(BIT_MASK_BCNQ_DESC_MODE << BIT_SHIFT_BCNQ_DESC_MODE)) |
                      BIT_BCNQ_DESC_MODE(TX_BCNQ_TXBD_MODE_SEL);
            RTL_W16(REG_RX_RXBD_NUM, RXBDReg);
    
            //3 ===Set RXBD Number===
            RXBDReg = (RXBDReg & ~BIT_MASK_RXQ_DESC_NUM) | BIT_RXQ_DESC_NUM(RX_Q_RXBD_NUM_IF(priv));
            RTL_W16(REG_RX_RXBD_NUM, RXBDReg);
    
            //3 ===Set 32Bit / 64 Bit System===
            RXBDReg = (RXBDReg & ~BIT_SYS_32_64) | (TXBD_SEG_32_64_SEL << 15);
            RTL_W16(REG_RX_RXBD_NUM, RXBDReg);
           
#if 0
           //3 ------apofflaod disable-----
            u1Byte reg_val;
            u4Byte u4_val;
    
            GET_HAL_INTERFACE(priv)->SetAPOffloadHandler(priv, 0, 1, 0, 0, NULL, NULL);
            //restore setting
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);
            reg_val = reg_val | BIT(6);
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);
    
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
            u4_val = u4_val & ~BIT(8);
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
            //printk("0x284=%x\n",RTL_R32(0x284));
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val); //release RXDMA
            u4_val = u4_val & ~BIT(18);
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val);
            //3 ------apofflaod disable-----
#endif
    
            RTL_W8(REG_RXPKT_NUM+2,  RTL_R8(REG_RXPKT_NUM+2) & ~BIT(2));
    
            // wait until FW stop parsing packet
            ctr = 1000;
            do {
                if (!(RTL_R8(FWIMR) & FWIMR_RXDONE))
                    break;
                delay_us(200);
            } while (--ctr) ;
            if (!ctr)
                printk("stop offload fail\n");
    
    
            RTL_W8(REG_PCIE_CTRL + 2, 0x3); // sw L123
            
            RTL_W8(TXPAUSE, 0x0);
    
            RTL_W32(REG_HIMR0, priv->b0IMR);
            RTL_W32(REG_HIMR1, priv->b8IMR);
            RTL_W32(PCIE_CTRL_REG, RTL_R32(PCIE_CTRL_REG)& ~0x7FFF ); //enable pcie dma
            RTL_W8(CR, 0xFF);
            
            RTL_W16(CR , RTL_R16(CR) & ~ENSWBCN);
            priv->pwr_state = L0;
           
            RESTORE_INT(flags);
            rtl8192cd_tx_dsr(priv);
            rtl8192cd_rx_isr(priv);
    
            printk("back to -> L0\n");
        }
        
            
#endif    
    
	return 0;
}
#endif //#ifdef CONFIG_OFFLOAD_FUNCTION

#ifdef GPIO_WAKEPIN
#ifdef __LINUX_2_6__
irqreturn_t gpio_wakeup_isr_V2(int irq, void *dev_instance);
#else
void gpio_wakeup_isr_V2(int irq, void *dev_instance, struct pt_regs *regs);
#endif
int request_irq_for_wakeup_pin_V2(struct net_device *dev)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	unsigned int saddr;
#if defined(__LINUX_2_6__)
    //test, setting GPIO H1
    gpio_request(BSP_GPIO_PIN_H1, "WLAN Sleep INT");
    gpio_direction_input(BSP_GPIO_PIN_H1);

    int h1irq = gpio_to_irq(BSP_GPIO_PIN_H1);
    printk("GOIP H1 IRQ num=%d\n",h1irq);
    return request_irq(h1irq, gpio_wakeup_isr_V2, IRQF_SHARED |  IRQF_TRIGGER_FALLING, "rtk_gpio", dev);

#else
	return request_irq(2, gpio_wakeup_isr_V2, SA_SHIRQ, "rtl_gpio", dev);
#endif

}
#endif

#ifdef GPIO_WAKEPIN
#ifdef __LINUX_2_6__
irqreturn_t gpio_wakeup_isr_V2(int irq, void *dev_instance)
#else
void gpio_wakeup_isr_V2(int irq, void *dev_instance, struct pt_regs *regs)
#endif
{
	struct net_device *dev = NULL;
	struct rtl8192cd_priv *priv = NULL;
    u1Byte reg_val;
    u4Byte u4_val;

	
    PHCI_RX_DMA_MANAGER_88XX        prx_dma;
    PHCI_RX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q_tx;
    unsigned long flags;
    u4Byte                      RXBDReg;

    u32 tmp32, haddr, saddr, cnt, ctr;
    int status;
    haddr = 0xb8b00000;
    saddr = 0xb8b10000;
	

	dev = (struct net_device *)dev_instance;
	priv = GET_DEV_PRIV(dev);
#ifdef PCIE_POWER_SAVING_TEST
        if(priv->pwr_state == L1){
        // pcie link into L0
        cnt = 20;
        tmp32 = REG32(saddr + 0x44) & ( ~(3));  //D0
        REG32(saddr + 0x44) = tmp32 | (0);      //D0
        do {
            status = REG32(haddr + 0x728) & 0x1f;
            if (status == 0x11)
                break;
            delay_ms(10);
        } while(--cnt);
    
        if (status != 0x11)
        {
            printk("change to L0 fail!!!, status=%x, MAC0\n", status);
            return 0;
        }
        
        priv->pwr_state = L0;
        }else if(priv->pwr_state == L2){
            printk("GPIO wakeup, PCIE=L2\n");
            PCIE_reset_procedure3_V2(priv);
            setBaseAddressRegister_V2();
    
            priv->pwr_state = PCIE_SETTING;
    
            SAVE_INT_AND_CLI(flags);
            
    
            RTL_W8(RSV_CTRL0, 0x0);
    
     
            //priv->pshare->phw->cur_rx = 0;
            prx_dma         = (PHCI_RX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PRxDMA88XX);
            cur_q           = &(prx_dma->rx_queue[0]);
            cur_q->cur_host_idx = 0;
            cur_q->host_idx = 0;
            cur_q->hw_idx = 0;
    
            RTL_W32(REG_RXQ_RXBD_DESA, _GET_HAL_DATA(priv)->ring_dma_addr);
            RTL_W32(REG_RXQ_RXBD_IDX,0);
            
            int txQ_i=0;
            ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
            //HAL_RTL_W32(REG_BD_RWPTR_CLR,0xffffffff);
            
            for(txQ_i=0;txQ_i<HCI_TX_DMA_QUEUE_MAX_NUM;txQ_i++){
                
                cur_q_tx       = &(ptx_dma->tx_queue[txQ_i]);
                cur_q_tx->host_idx = 0;
                cur_q_tx->hw_idx = 0;
    
            }
            RTL_W32(REG_BCNQ_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_BCN]);
            RTL_W32(REG_MGQ_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_MGT]);
            RTL_W32(REG_VOQ_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_VO]);
            RTL_W32(REG_VIQ_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_VI]);
            RTL_W32(REG_BEQ_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_BE]);
            RTL_W32(REG_BKQ_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_BK]);
            RTL_W32(REG_HI0Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI0]);
            RTL_W32(REG_HI1Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI1]);
            RTL_W32(REG_HI2Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI2]);
            RTL_W32(REG_HI3Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI3]);
            RTL_W32(REG_HI4Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI4]);
            RTL_W32(REG_HI5Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI5]);
            RTL_W32(REG_HI6Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI6]);
            RTL_W32(REG_HI7Q_TXBD_DESA, _GET_HAL_DATA(priv)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_HI7]);
    
            RTL_W32(REG_MGQ_TXBD_IDX,0);
            RTL_W32(REG_VOQ_TXBD_IDX,0);
            RTL_W32(REG_VIQ_TXBD_IDX,0);
            RTL_W32(REG_BEQ_TXBD_IDX,0);
            RTL_W32(REG_BKQ_TXBD_IDX,0);
            RTL_W32(REG_HI0Q_TXBD_IDX,0);
            RTL_W32(REG_HI1Q_TXBD_IDX,0);
            RTL_W32(REG_HI2Q_TXBD_IDX,0);
            RTL_W32(REG_HI3Q_TXBD_IDX,0);
            RTL_W32(REG_HI4Q_TXBD_IDX,0);
            RTL_W32(REG_HI5Q_TXBD_IDX,0);
            RTL_W32(REG_HI6Q_TXBD_IDX,0);
            RTL_W32(REG_HI7Q_TXBD_IDX,0);
    
#ifdef DELAY_REFILL_RX_BUF
            priv->pshare->phw->cur_rx_refill = 0;
#endif
    
    
    
    //3    ===Set TXBD Mode and Number===
            RTL_W16(REG_MGQ_TXBD_NUM, BIT_MGQ_DESC_MODE(TX_MGQ_TXBD_MODE_SEL) |
                        BIT_MGQ_DESC_NUM(TX_MGQ_TXBD_NUM));
            RTL_W16(REG_VOQ_TXBD_NUM, BIT_VOQ_DESC_MODE(TX_VOQ_TXBD_MODE_SEL) |
                        BIT_VOQ_DESC_NUM(TX_VOQ_TXBD_NUM));
            RTL_W16(REG_VIQ_TXBD_NUM, BIT_VIQ_DESC_MODE(TX_VIQ_TXBD_MODE_SEL) |
                        BIT_VIQ_DESC_NUM(TX_VIQ_TXBD_NUM));
            RTL_W16(REG_BEQ_TXBD_NUM, BIT_BEQ_DESC_MODE(TX_BEQ_TXBD_MODE_SEL) |
                        BIT_BEQ_DESC_NUM(TX_BEQ_TXBD_NUM));
            RTL_W16(REG_BKQ_TXBD_NUM, BIT_BKQ_DESC_MODE(TX_BKQ_TXBD_MODE_SEL) |
                        BIT_BKQ_DESC_NUM(TX_BKQ_TXBD_NUM));
        
            RTL_W16(REG_HI0Q_TXBD_NUM, BIT_HI0Q_DESC_MODE(TX_HI0Q_TXBD_MODE_SEL) |
                        BIT_HI0Q_DESC_NUM(TX_HI0Q_TXBD_NUM));
            RTL_W16(REG_HI1Q_TXBD_NUM, BIT_HI1Q_DESC_MODE(TX_HI1Q_TXBD_MODE_SEL) |
                        BIT_HI1Q_DESC_NUM(TX_HI1Q_TXBD_NUM));
            RTL_W16(REG_HI2Q_TXBD_NUM, BIT_HI2Q_DESC_MODE(TX_HI2Q_TXBD_MODE_SEL) |
                        BIT_HI2Q_DESC_NUM(TX_HI2Q_TXBD_NUM));
            RTL_W16(REG_HI3Q_TXBD_NUM, BIT_HI3Q_DESC_MODE(TX_HI3Q_TXBD_MODE_SEL) |
                        BIT_HI3Q_DESC_NUM(TX_HI3Q_TXBD_NUM));
            RTL_W16(REG_HI4Q_TXBD_NUM, BIT_HI4Q_DESC_MODE(TX_HI4Q_TXBD_MODE_SEL) |
                        BIT_HI4Q_DESC_NUM(TX_HI4Q_TXBD_NUM));
            RTL_W16(REG_HI5Q_TXBD_NUM, BIT_HI5Q_DESC_MODE(TX_HI5Q_TXBD_MODE_SEL) |
                        BIT_HI5Q_DESC_NUM(TX_HI5Q_TXBD_NUM));
            RTL_W16(REG_HI6Q_TXBD_NUM, BIT_HI6Q_DESC_MODE(TX_HI6Q_TXBD_MODE_SEL) |
                        BIT_HI6Q_DESC_NUM(TX_HI6Q_TXBD_NUM));
            RTL_W16(REG_HI7Q_TXBD_NUM, BIT_HI7Q_DESC_MODE(TX_HI7Q_TXBD_MODE_SEL) |
                        BIT_HI7Q_DESC_NUM(TX_HI7Q_TXBD_NUM));
        
#if IS_EXIST_RTL8822BE || IS_EXIST_RTL8197FEM
            RTL_W16(REG_H2CQ_TXBD_NUM, BIT_H2CQ_DESC_MODE(TX_H2CQ_TXBD_MODE_SEL) |
                        BIT_H2CQ_DESC_NUM(TX_CMDQ_TXBD_NUM));
#endif //IS_EXIST_RTL8822BE
    
    
    
            //3 ===Set Beacon Mode: 2, 4, or 8 segment each descriptor===
            RXBDReg = RTL_R16(REG_RX_RXBD_NUM);
            RXBDReg = (RXBDReg & ~(BIT_MASK_BCNQ_DESC_MODE << BIT_SHIFT_BCNQ_DESC_MODE)) |
                      BIT_BCNQ_DESC_MODE(TX_BCNQ_TXBD_MODE_SEL);
            RTL_W16(REG_RX_RXBD_NUM, RXBDReg);
    
            //3 ===Set RXBD Number===
            RXBDReg = (RXBDReg & ~BIT_MASK_RXQ_DESC_NUM) | BIT_RXQ_DESC_NUM(RX_Q_RXBD_NUM_IF(priv));
            RTL_W16(REG_RX_RXBD_NUM, RXBDReg);
    
            //3 ===Set 32Bit / 64 Bit System===
            RXBDReg = (RXBDReg & ~BIT_SYS_32_64) | (TXBD_SEG_32_64_SEL << 15);
            RTL_W16(REG_RX_RXBD_NUM, RXBDReg);
    
           //3 ------apofflaod disable-----
            u1Byte reg_val;
            u4Byte u4_val;
    
            GET_HAL_INTERFACE(priv)->SetAPOffloadHandler(priv, 0, 1, 0, 0, NULL, NULL);
            //restore setting
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);
            reg_val = reg_val | BIT(6);
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);
    
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
            u4_val = u4_val & ~BIT(8);
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
            //printk("0x284=%x\n",RTL_R32(0x284));
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val); //release RXDMA
            u4_val = u4_val & ~BIT(18);
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val);
            //3 ------apofflaod disable-----
    
    
            // wait until FW stop parsing packet
            ctr = 1000;
            do {
                if (!(RTL_R8(FWIMR) & FWIMR_RXDONE))
                    break;
                delay_us(200);
            } while (--ctr) ;
            if (!ctr)
                printk("stop offload fail\n");
    
    
            RTL_W8(REG_PCIE_CTRL + 2, 0x3); // sw L123
            RTL_W32(PCIE_CTRL_REG, RTL_R32(PCIE_CTRL_REG)& ~0x7FFF ); //enable pcie dma
            RTL_W8(TXPAUSE, 0x0);
            RTL_W16(CR , RTL_R16(CR) & ~ENSWBCN);
    
    
#if defined(RX_TASKLET)
            if (IS_DRV_OPEN(priv)) {
                tasklet_hi_schedule(&priv->pshare->rx_tasklet);
            }
#endif
    
            RESTORE_INT(flags);
    
            RTL_W32(REG_HIMR0, priv->b0IMR);
            RTL_W32(REG_HIMR1, priv->b8IMR);
      
            priv->pwr_state = L0;
            printk("back to -> L0\n");
        }
        
#endif
    
	printk("%s, PEFGH_ISR=%x\t", dev->name, REG32(PEFGH_ISR));

//yllin test
    REG32(PEFGH_ISR) = BIT(25); // clear int status

    //stop apofflaod
    GET_HAL_INTERFACE(priv)->SetAPOffloadHandler(priv, 0, 1, 0, 0, NULL, NULL);
    //restore setting
    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);
	reg_val = reg_val | BIT(6);
	GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);

    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
	u4_val = u4_val & ~BIT(8);
	GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
    //printk("0x284=%x\n",RTL_R32(0x284));
    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val); //release RXDMA
	u4_val = u4_val & ~BIT(18);
	GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val);

#ifdef __LINUX_2_6__
	return IRQ_HANDLED;
#endif
}
#define CLK_MANAGE	0xb8000010

void PCIE_reset_procedure3_V2(struct rtl8192cd_priv *priv)
{
	//PCIE Register
	unsigned int PCIE_PHY0_REG, PCIE_PHY0, linkstatus, haddr;
	int status = 0, counter1 = 0, counter2 = 0;

	haddr = 0xb8b00000;

	PCIE_PHY0_REG  = haddr + 0x1000;
	PCIE_PHY0 = haddr + 0x1008;
	linkstatus = haddr + 0x728;

	do {
		// PCIE PHY Reset
		REG32(PCIE_PHY0) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
		delay_ms(2);

		REG32(0xb8000050) &= ~(1 << 1);  //perst=0 off.
		delay_ms(10);

		REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
		delay_ms(5);


		//---------------------------------------
		// PCIE Device Reset

		delay_ms(50);
		REG32(0xb8000050) |=  (1 << 1); //PERST=1
		delay_ms(10);

		counter1 = 20;
		while(--counter1)
		{
			status = REG32(linkstatus) & 0x1f;
			if ( status == 0x11 )
			{
				break;
			}
			delay_ms(50);
		}

		if (counter1 == 0)
		{
			if ( ++counter2 > 10) {
				panic_printk("PCIe reset fail!!!!\n");
				break;
			}
		}else
		{
			break;
		}
	} while (1);

//	printk("PCIE_reset_procedure3\t devid=%x\n",REG32(0xb8b10000));

}
void setBaseAddressRegister_V2(void)
{
	int tmp32 = 0, status;
	while (++tmp32 < 100) {
		REG32(0xb8b00004) = 0x00100007;
		REG32(0xb8b10004) = 0x00100007;
		REG32(0xb8b10010) = 0x18c00001;
		REG32(0xb8b10018) = 0x19000004;
		status = (REG32(0xb8b10010) ^ 0x18c00001) | ( REG32(0xb8b10018) ^ 0x19000004);
		if (!status)
			break;
		else {
			printk("set BAR fail,%x\n", status);
			printk("%x %x %x %x \n",
				   REG32(0xb8b00004) , REG32(0xb8b10004) , REG32(0xb8b10010),  REG32(0xb8b10018) );
		}
	} ;
}

#endif


/*
 *
 * Power Saving related functions
 *
 */
#ifdef PCIE_POWER_SAVING

#ifdef CONFIG_RTL_92D_DMDP
extern u32 if_priv[];
#endif

#ifdef CONFIG_RTL_92D_DMDP

void Sw_PCIE_Func2(int func)
{
#if (RTL_USED_PCIE_SLOT==1)
	int reg = 0xb8b2100c;
#else
	int reg = 0xb8b0100c;
#endif

	REG32(reg) &= ~(1);
	REG32(reg) |= func; // switch to function #
}
#endif

#if defined(__LINUX_2_6__)
extern void HostPCIe_SetPhyMdioWrite(unsigned int , unsigned int , unsigned short );
#endif
#ifdef ASPM_ENABLE
void ASPM_on_off(struct rtl8192cd_priv *priv) ;
#endif

#ifndef CONFIG_RTL_8198B
#define CLK_MANAGE	0xb8000010
#endif
#ifdef PCIE_POWER_SAVING_DEBUG
int PCIE_PowerDown(struct rtl8192cd_priv *priv, unsigned char *data)
{
//  #define PCIE_PHY0  0xb8b01008
	int round = 0;

#define dprintf printk
	int tmp, mode, portnum = 0;
	unsigned int PCIE_PHY0, linkstatus;
	unsigned int haddr, saddr;

	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		haddr = CFG_92D_SLOTH;
		saddr = CFG_92D_SLOTS;
	} else {
		haddr = CFG_92C_SLOTH;
		saddr = CFG_92C_SLOTS;
	}

	PCIE_PHY0 = haddr + 0x1008;
	linkstatus = haddr + 0x728;

#ifdef CONFIG_RTL_92D_DMDP
	Sw_PCIE_Func2(priv->pshare->wlandev_idx);
#endif

#if defined(CONFIG_RTL_92D_SUPPORT)
	portnum = RTL_USED_PCIE_SLOT;
#endif

	mode = _atoi(data, 16);

	if (strlen(data) == 0) {
		dprintf("epdn mode.\n");
		dprintf("epdn 0: D0 ->L0 \n");
		dprintf("epdn 3: D3hot ->L1 \n");
		dprintf("epdn 4: board cast PME_TurnOff \n");
		dprintf("epdn 7: enable aspm and L0 entry \n");
		dprintf("epdn 8: enable aspm and L1 entry \n");
		dprintf("epdn 9: diable  aspm \n");
		dprintf("epdn 5a: pcie reset \n");
		dprintf("epdn 6a: L0 -> L2 \n");
		dprintf("epdn 6b: L2 -> L0\n");
		dprintf("epdn 6c: L0 -> L1 \n");
		dprintf("epdn 6d: L1 -> L0\n");
		dprintf("epdn 6e: download probe rsp\n");
		dprintf("epdn a3: wake pin test\n");
		dprintf("epdn b: bar\n");
		dprintf("epdn b1: offload enable \n");
		dprintf("epdn b2: offload disable\n");
		dprintf("epdn c1: swith to 1T\n");
		dprintf("epdn c2: switch to 2T\n");
		dprintf("Link status=%x \n", REG32(linkstatus) & 0x1f );
		return 0;
	}

	if (mode == 0) {

#ifdef CONFIG_RTL_92D_DMDP
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			if (priv->pshare->wlandev_idx != 0) {
				dprintf("not Root Interface!! \n");
				return 0;
			}
			Sw_PCIE_Func2(0);
		}
#endif

#ifdef SAVING_MORE_PWR
		HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0f0f);
#endif
		tmp = REG32(0xb8b10044) & ( ~(3));
		REG32(0xb8b10044) = tmp |	(0); //D0
		delay_ms(1);
		REG32(0xb8b10044) = tmp |	(0); //D0
		dprintf("D0 \n");
		priv->pwr_state = L0;

#if defined(CONFIG_RTL_92D_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
#ifdef CONFIG_RTL_92D_DMDP
//		   	if(priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
			{
				Sw_PCIE_Func2(1);
#ifdef SAVING_MORE_PWR
				HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0f0f);
#endif
				tmp = REG32(0xb8b10044) & ( ~(3)); //D0
				REG32(0xb8b10044) = tmp |	(0); //D0
				delay_ms(1);
				REG32(0xb8b10044) = tmp |	(0); //D0
				dprintf("D0 wlan1\n");
				((struct rtl8192cd_priv *)if_priv[1])->pwr_state = L0;
			}
			Sw_PCIE_Func2(0);
#endif
			delay_ms(1);
		}
#endif

	}

	if (mode == 3) {

#if defined(CONFIG_RTL_92D_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
#ifdef CONFIG_RTL_92D_DMDP
			if (priv->pshare->wlandev_idx != 0) {
				dprintf("not Root Interface!! \n");
				return 0;
			}
//			if(priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
			{

				dprintf("DMDP, disable wlan1 !!\n");
				Sw_PCIE_Func2(1);
#ifdef SAVING_MORE_PWR
				REG32(0xb8b10080) |= (0x100);	//enable clock PM
#endif
				tmp = REG32(0xb8b10044) & ( ~(3));
				REG32(0xb8b10044) = tmp |	(3); //D3

				((struct rtl8192cd_priv *)if_priv[1])->pwr_state = L1;
#ifdef SAVING_MORE_PWR
				HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0708);
#endif
			}
			Sw_PCIE_Func2(0);
#endif
			delay_ms(1);

		}
#endif

#ifdef SAVING_MORE_PWR
		REG32(0xb8b10080) |= (0x100);	//enable clock PM
#endif
		tmp = REG32(0xb8b10044) & ( ~(3));
		REG32(0xb8b10044) = tmp |	(3); //D3
		//HostPCIe_SetPhyMdioWrite(0xd, 0x15a6);
		dprintf("D3 hot \n");
		priv->pwr_state = L1;
#ifdef SAVING_MORE_PWR
		HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0708);
#endif
	}

	if (mode == 4) {

		RTL_W8(0x1c, 0xe1); 	// reg lock, dis_prst
		RTL_W8(0x1c, 0xe1);

#ifdef SAVING_MORE_PWR
		HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0f0f);
#endif

		REG32(0xb8b01008) |= (0x200);
		dprintf("Host boardcase PME_TurnOff \n");
		priv->pwr_state = L2;
	}


	if (mode == 0xd) {

		RTL_W8(0x1c, 0x0);
		priv->pshare->phw->cur_rx = 0;
#ifdef DELAY_REFILL_RX_BUF
		priv->pshare->phw->cur_rx_refill = 0;
#endif
		memset(&(priv->pshare->phw->txhead0), 0, sizeof(int) * 12);

		RTL_W8(SPS0_CTRL, 0x2b);
		RTL_W32(BCNQ_DESA, priv->pshare->phw->tx_ringB_addr);
		RTL_W32(MGQ_DESA, priv->pshare->phw->tx_ring0_addr);
		RTL_W32(VOQ_DESA, priv->pshare->phw->tx_ring4_addr);
		RTL_W32(VIQ_DESA, priv->pshare->phw->tx_ring3_addr);
		RTL_W32(BEQ_DESA, priv->pshare->phw->tx_ring2_addr);
		RTL_W32(BKQ_DESA, priv->pshare->phw->tx_ring1_addr);
		RTL_W32(HQ_DESA, priv->pshare->phw->tx_ring5_addr);
		RTL_W32(RX_DESA, priv->pshare->phw->ring_dma_addr);

	}

	if (mode == 7)	 {
		REG32(0xb8b1070c) &= ~  ((0x7 << 27) | (0x7 << 24));
		REG32(0xb8b1070c) |=	((3) << 27) | ((1) << 24);	 //L1=8us, L0s=2us

		REG32(0xb8b00080) &= ~(0x3);
		REG32(0xb8b10080) &= ~(0x3);

		REG32(0xb8b00080) |= 1;	//L0s
		REG32(0xb8b10080) |= 1;
		priv->pwr_state = ASPM_L0s_L1;
	}

	if (mode == 8)	 {
		REG32(0xb8b1070c) &= ~  ((0x7 << 27) | (0x7 << 24));
		REG32(0xb8b1070c) |=	((1) << 27) | ((3) << 24);	 //L1=2us, L0s=4us

		REG32(0xb8b00080) &= ~(0x3);
		REG32(0xb8b10080) &= ~(0x3);

		REG32(0xb8b00080) |= 3;	//L1
		REG32(0xb8b10080) |= 3; //L1
		priv->pwr_state = ASPM_L0s_L1;

	}

	if (mode == 9)	 {
		REG32(0xb8b00080) &= ~(0x3);
		REG32(0xb8b10080) &= ~(0x3);
		priv->pwr_state = L0;
	}


	if (mode == 0x6a)	{
		priv->ps_ctrl = 1 | 32 | 0x80;
		mod_timer(&priv->ps_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(100));
	}

	if (mode == 0x6c)	{
		priv->ps_ctrl = 1 | 16 | 0x80;
//	  	mod_timer(&priv->ps_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(100));
		PCIe_power_save_tasklet((unsigned long)priv);
	}

	if ((mode == 0x6b) || (mode == 0x6d))		{
		priv->ps_ctrl = 0x82 | (priv->pwr_state << 4);
		priv->pshare->rf_ft_var.power_save &= 0xf0;

#ifdef CONFIG_RTL_92D_DMDP
		((struct rtl8192cd_priv *)if_priv[1])->pshare->rf_ft_var.power_save &= 0xf0;
#endif
		PCIe_power_save_tasklet((unsigned long)priv);
		signin_h2c_cmd(priv, _AP_OFFLOAD_CMD_ , 0 );
#ifdef CONFIG_RTL_92D_DMDP
		signin_h2c_cmd(((struct rtl8192cd_priv *)if_priv[1]), _AP_OFFLOAD_CMD_ , 0 );
#endif
	}

	if (mode == 0x6e) {
		priv->offload_ctrl = 1;
		RTL_W16(0x100 , RTL_R16(0x100) | BIT(8));		// enable sw beacon
		tasklet_hi_schedule(&priv->pshare->rx_tasklet);
	}

	if (mode == 0xc1)	{
//		PHY_ConfigBBWithParaFile(priv,	PATHB_OFF);
		switch_to_1x1(priv, PWR_STATE_IN);

	}

	if (mode == 0xc2)	 {
//		 PHY_ConfigBBWithParaFile(priv,  PATHB_ON);
		switch_to_1x1(priv, PWR_STATE_OUT);
	}

	if (mode == 0xb)	 {
		REG32(0xb8b00004) = 0x00100007;
		REG32(0xb8b10004) = 0x00100007;
		REG32(0xb8b10010) = 0x18c00001;
		REG32(0xb8b10018) = 0x19000004;
		printk("b1-00=%x, b0-04=%x, b1-04=%x, b1-10=%x, b1-18=%x\n", REG32(0xb8b10000),
			   REG32(0xb8b00004), REG32(0xb8b10004), REG32(0xb8b10010), REG32(0xb8b10018) );
	}

	if (mode == 0xb1)	 {
		unsigned int  cmd = _AP_OFFLOAD_CMD_ | (1 << 8) | (HIDDEN_AP << 16) | ((GET_MIB(priv))->dot11OperationEntry.deny_any) << 24;
		int page = ((priv->offload_ctrl) >> 7) & 0xff;
		int cmd2 = 0, cmd2e = 0;

		if (!page) {
			page = 2;
		}

		if (GET_CHIP_VER(priv) != VERSION_8192D) {
			cmd2 = (_RSVDPAGE_CMD_ | page << 8) ;
		} else {
			cmd2 = ( _RSVDPAGE_CMD_ | BIT(7) | (page << 8));
			cmd2e = (page << 8) | (page) ;
		}

//		RTL_W16(PCIE_CTRL_REG, 0xff00 );
		REG32(saddr + 0x44) |= 0x8108;

		printk("cmd: %x %x\n", cmd2, cmd2e);
		signin_h2c_cmd(priv, cmd2, cmd2e);
		delay_ms(10);
		signin_h2c_cmd(priv, cmd, 0 );
		printk("sign in h2c cmd:%x, 0x284=%x\n", cmd, RTL_R32(0x284));

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
		REG32(0xb8003000) |= BIT(16);		// GIMR
#else
		REG32(0xb8003000) |= BIT(9);		// GIMR
#endif

		delay_ms(10);
	}

	if (mode == 0xb2)	 {
		signin_h2c_cmd(priv, 0x0000, 0);	// offload disable
		RTL_W8(0x423, 0x0); 		// mac seq disable
		RTL_W8(0x286, 0);			// RW_RELEASE_ENABLE
		RTL_W16(PCIE_CTRL_REG, 0x00ff );
	}

	//static unsigned int Buffer[9];

	if (mode == 0xa3) {
		unsigned char tmp;
#ifdef RTL8676_WAKE_GPIO
		int gpio_num, irq_num;

		get_wifi_wake_pin(&gpio_num);
		irq_num = gpioGetBspIRQNum(gpio_num);

		gpioConfig(gpio_num, GPIO_FUNC_INPUT);
		gpioSetIMR(gpio_num, EN_FALL_EDGE_ISR); 	// enable interrupt in falling-edge
		REG32(BSP_GIMR) |= BIT(irq_num);

#else

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)

		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			REG32(0xb8000044) |= BIT(16) | BIT(17);                 //LEDPHASE1
			REG32(0xb8003500) &= ~(BIT(17));                        //PABCD_CNR , gpio pin
			REG32(0xb8003508) &= ~(BIT(17));                        //PABCD_DIR
			REG32(0xb8003518) &= (~(0x03 << 2));
			REG32(0xb8003518) |= (0x01 << 2);                       // PCD_IMR
		} else {
			REG32(0xb8000044) |= BIT(24);                           //LEDPHASE4
			REG32(0xb8003500) &= ~(BIT(20));                        //PABCD_CNR , gpio pin
			REG32(0xb8003508) &= ~(BIT(20));                        //PABCD_DIR
			REG32(0xb8003518) &= (~(0x03 << 8));
			REG32(0xb8003518) |= (0x01 << 8);                       // PCD_IMR
		}

		REG32(0xb8003000) |= BIT(16);		// GIMR
#else
		REG32(0xb8000040) |= 0x0c00;		//LEDPHASE1 :GPIOB7
		REG32(0xb8003500) &= ~(BIT(15));;	//PABCD_CNR , gpio pin
		REG32(0xb8003508) &= ~(BIT(15));	//PABCD_DIR
		REG32(0xb8003514) &= (~(0x03 << 30));
		REG32(0xb8003514) |= (0x01 << 30);	// PAB_IMR		// enable interrupt in falling-edge
		REG32(0xb8003000) |= BIT(9);		// GIMR
#endif

#endif
		// clear wake pin status
#ifdef CONFIG_RTL_92D_DMDP
		Sw_PCIE_Func2(priv->pshare->wlandev_idx);
#endif

		REG32(saddr + 0x44) = 0x8108;
		tmp = RTL_R8(0x690);
		if (tmp & 1)		{
			tmp ^= 0x1;
			RTL_W8(0x690, tmp);
		}
		dprintf("0xb8b10044=%x,690=%x,3000=%x, 3514=%x\n",	REG32(saddr + 0x44), RTL_R8(0x690), REG32(0xb8003000), REG32(0xb8003514) );
		RTL_W8(0x690, tmp | 0x1 );
		dprintf("0xb8b10044=%x,690=%x\n",	REG32(saddr + 0x44), RTL_R8(0x690) );
	}

	if (mode == 0x5a)
		PCIE_reset_procedure3(priv);

	//-------------------------------------------------------------
	if (mode == 0x010) {	//L0->L1->L0
		tmp = REG32(0xb8b10044) & ( ~(3)); //D0
		REG32(0xb8b10044) = tmp | (3); //D3
		REG32(0xb8b10044) = tmp | (0); //D0, wakeup

		round = 0;
		while (1) {
			if ((REG32(linkstatus) & 0x1f) == 0x11)	 //wait to L0
				break;

			if (++round > 10000) {
				panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
				break;
			}
		}

		dprintf("DID/VID=%x\n", REG32(0xb8b10000));
	}
	//-------------------------------------------------------------
	if (mode == 0x020) {		//L0->L2->L0
		tmp = REG32(0xb8b10044) & ( ~(3)); //D0

		REG32(0xb8b10044) = tmp |	(3); //D3
		delay_ms(100);

		REG32(0xb8b01008) |= (0x200);
		delay_ms(100);

		//wakeup
		REG32(CLK_MANAGE) &= ~(1 << 12);	 //perst=0 off.
		//dprintf("CLK_MANAGE=%x \n",  REG32(CLK_MANAGE));
		delay_ms(100);
		delay_ms(100);
		delay_ms(100);

		REG32(CLK_MANAGE) |=  (1 << 12);	//PERST=1
		//prom_printf("\nCLK_MANAGE(0x%x)=0x%x\n\n",CLK_MANAGE,READ_MEM32(CLK_MANAGE));

		//4. PCIE PHY Reset
		REG32(PCIE_PHY0) = 0x01; //bit7 PHY reset=0   bit0 Enable LTSSM=1
		REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1	bit0 Enable LTSSM=1

		round = 0;
		while (1) {
			if ((REG32(linkstatus) & 0x1f) == 0x11)
				break;

			if (++round > 10000) {
				panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
				break;
			}
		}

		dprintf("DID/VID=%x\n", REG32(0xb8b10000));
	}

	dprintf("Link status=%x\n", READ_MEM32(linkstatus) & 0x1f/*, READ_MEM32(linkstatus), REG32(linkstatus)*/ );

	return 0;
}
#endif


void switch_to_1x1(struct rtl8192cd_priv *priv, int mode)
{

	if (mode == PWR_STATE_IN) 	{

		priv->pshare->rf_phy_bb_backup[21] = RTL_R32(0x88c);

		priv->pshare->rf_phy_bb_backup[0] = RTL_R32(0x844);
		priv->pshare->rf_phy_bb_backup[1] = RTL_R32(0x85c);
		priv->pshare->rf_phy_bb_backup[2] = RTL_R32(0xe6c);

		priv->pshare->rf_phy_bb_backup[3] = RTL_R32(0xe70);
		priv->pshare->rf_phy_bb_backup[4] = RTL_R32(0xe74);
		priv->pshare->rf_phy_bb_backup[5] = RTL_R32(0xe78);
		priv->pshare->rf_phy_bb_backup[6] = RTL_R32(0xe7c);

		priv->pshare->rf_phy_bb_backup[7] = RTL_R32(0xe80);
		priv->pshare->rf_phy_bb_backup[8] = RTL_R32(0xe84);
		priv->pshare->rf_phy_bb_backup[9] = RTL_R32(0xe88);
		priv->pshare->rf_phy_bb_backup[10] = RTL_R32(0xe8c);

		priv->pshare->rf_phy_bb_backup[11] = RTL_R32(0xed0);
		priv->pshare->rf_phy_bb_backup[12] = RTL_R32(0xed4);
		priv->pshare->rf_phy_bb_backup[13] = RTL_R32(0xed8);
		priv->pshare->rf_phy_bb_backup[14] = RTL_R32(0xedc);

		priv->pshare->rf_phy_bb_backup[15] = RTL_R32(0xee0);
		priv->pshare->rf_phy_bb_backup[16] = RTL_R32(0xeec);

		priv->pshare->rf_phy_bb_backup[17] = RTL_R32(0xc04);
		priv->pshare->rf_phy_bb_backup[18] = RTL_R32(0xd04);
		priv->pshare->rf_phy_bb_backup[19] = RTL_R32(0x90c);
		priv->pshare->rf_phy_bb_backup[20] = RTL_R32(0x804);
		priv->pshare->rf_phy_bb_backup[22] = RTL_R32(0xa04);

#ifdef CONFIG_RTL_92D_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192D)) {
			unsigned int mask = 0xB4FFFFFF, path = 0x11;
			if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)	{
				mask = 0xDB3FFFFF;
				path = 0x22;
				RTL_W8(0xa07, 0x45);
			}
			PHY_SetBBReg(priv, 0x85c, bMaskDWord, priv->pshare->rf_phy_bb_backup[1] & mask);
			PHY_SetBBReg(priv, 0xe6c, bMaskDWord, priv->pshare->rf_phy_bb_backup[2] & mask);

			PHY_SetBBReg(priv, 0xe70, bMaskDWord, priv->pshare->rf_phy_bb_backup[3] & mask);
			PHY_SetBBReg(priv, 0xe74, bMaskDWord, priv->pshare->rf_phy_bb_backup[4] & mask);
			PHY_SetBBReg(priv, 0xe78, bMaskDWord, priv->pshare->rf_phy_bb_backup[5] & mask);
			PHY_SetBBReg(priv, 0xe7c, bMaskDWord, priv->pshare->rf_phy_bb_backup[6] & mask);

			PHY_SetBBReg(priv, 0xe80, bMaskDWord, priv->pshare->rf_phy_bb_backup[7] & mask);
			PHY_SetBBReg(priv, 0xe84, bMaskDWord, priv->pshare->rf_phy_bb_backup[8] & mask);
			PHY_SetBBReg(priv, 0xe88, bMaskDWord, priv->pshare->rf_phy_bb_backup[9] & mask);
			PHY_SetBBReg(priv, 0xe8c, bMaskDWord, priv->pshare->rf_phy_bb_backup[10] & mask);

			PHY_SetBBReg(priv, 0xed0, bMaskDWord, priv->pshare->rf_phy_bb_backup[11] & mask);
			PHY_SetBBReg(priv, 0xed4, bMaskDWord, priv->pshare->rf_phy_bb_backup[12] & mask);
			PHY_SetBBReg(priv, 0xed8, bMaskDWord, priv->pshare->rf_phy_bb_backup[13] & mask);
			PHY_SetBBReg(priv, 0xedc, bMaskDWord, priv->pshare->rf_phy_bb_backup[14] & mask);

			PHY_SetBBReg(priv, 0xee0, bMaskDWord, priv->pshare->rf_phy_bb_backup[15] & mask);
			PHY_SetBBReg(priv, 0xeec, bMaskDWord, priv->pshare->rf_phy_bb_backup[16] & mask);

			PHY_SetBBReg(priv, 0xc04, 0x000000ff, path);
			PHY_SetBBReg(priv, 0xd04, 0x0000000f, path & 0x01);
			PHY_SetBBReg(priv, 0x90c, 0x000000ff, path);
			PHY_SetBBReg(priv, 0x90c, 0x0ff00000, path);

		} else
#endif
		{
			PHY_SetBBReg(priv, 0x88c, 0x00c00000 , 0x3);

#if 1
			// standby
			PHY_SetBBReg(priv, 0x844, bMaskDWord, 0x00010000);
#else
			// power off
			PHY_SetBBReg(priv, 0x844, bMaskDWord, 0x00000000);
#endif

			PHY_SetBBReg(priv, 0x85c, bMaskDWord, 0x00db25a4);
			PHY_SetBBReg(priv, 0xe6c, bMaskDWord, 0x20db25a4);

			PHY_SetBBReg(priv, 0xe70, bMaskDWord, 0x20db25a4);
			PHY_SetBBReg(priv, 0xe74, bMaskDWord, 0x041b25a4);
			PHY_SetBBReg(priv, 0xe78, bMaskDWord, 0x041b25a4);
			PHY_SetBBReg(priv, 0xe7c, bMaskDWord, 0x041b25a4);

			PHY_SetBBReg(priv, 0xe80, bMaskDWord, 0x041b25a4);
			PHY_SetBBReg(priv, 0xe84, bMaskDWord, 0x63db25a4);
			PHY_SetBBReg(priv, 0xe88, bMaskDWord, 0x041b25a4);
			PHY_SetBBReg(priv, 0xe8c, bMaskDWord, 0x20db25a4);

			PHY_SetBBReg(priv, 0xed0, bMaskDWord, 0x20db25a4);
			PHY_SetBBReg(priv, 0xed4, bMaskDWord, 0x20db25a4);
			PHY_SetBBReg(priv, 0xed8, bMaskDWord, 0x20db25a4);
			PHY_SetBBReg(priv, 0xedc, bMaskDWord, 0x001b25a4);

			PHY_SetBBReg(priv, 0xee0, bMaskDWord, 0x001b25a4);
			PHY_SetBBReg(priv, 0xeec, bMaskDWord, 0x24db25a4);

			PHY_SetBBReg(priv, 0xc04, 0x000000ff , 0x11);
			PHY_SetBBReg(priv, 0xd04, 0x0000000f , 0x1);
			PHY_SetBBReg(priv, 0x90c, 0x000000ff , 0x11);
			PHY_SetBBReg(priv, 0x90c, 0x0ff00000 , 0x11);

			PHY_SetBBReg(priv, 0x804, 0x000000f , 0x1);
		}
	} else if (mode == PWR_STATE_OUT)	 {

#ifdef CONFIG_RTL_92C_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192C) {
			PHY_SetBBReg(priv, 0x88c, bMaskDWord, priv->pshare->rf_phy_bb_backup[21]);
			PHY_SetBBReg(priv, 0x844, bMaskDWord, priv->pshare->rf_phy_bb_backup[0]);
		}
#endif
		PHY_SetBBReg(priv, 0x85c, bMaskDWord, priv->pshare->rf_phy_bb_backup[1]);
		PHY_SetBBReg(priv, 0xe6c, bMaskDWord, priv->pshare->rf_phy_bb_backup[2]);

		PHY_SetBBReg(priv, 0xe70, bMaskDWord, priv->pshare->rf_phy_bb_backup[3]);
		PHY_SetBBReg(priv, 0xe74, bMaskDWord, priv->pshare->rf_phy_bb_backup[4]);
		PHY_SetBBReg(priv, 0xe78, bMaskDWord, priv->pshare->rf_phy_bb_backup[5]);
		PHY_SetBBReg(priv, 0xe7c, bMaskDWord, priv->pshare->rf_phy_bb_backup[6]);

		PHY_SetBBReg(priv, 0xe80, bMaskDWord, priv->pshare->rf_phy_bb_backup[7]);
		PHY_SetBBReg(priv, 0xe84, bMaskDWord, priv->pshare->rf_phy_bb_backup[8]);
		PHY_SetBBReg(priv, 0xe88, bMaskDWord, priv->pshare->rf_phy_bb_backup[9]);
		PHY_SetBBReg(priv, 0xe8c, bMaskDWord, priv->pshare->rf_phy_bb_backup[10]);

		PHY_SetBBReg(priv, 0xed0, bMaskDWord, priv->pshare->rf_phy_bb_backup[11]);
		PHY_SetBBReg(priv, 0xed4, bMaskDWord, priv->pshare->rf_phy_bb_backup[12]);
		PHY_SetBBReg(priv, 0xed8, bMaskDWord, priv->pshare->rf_phy_bb_backup[13]);
		PHY_SetBBReg(priv, 0xedc, bMaskDWord, priv->pshare->rf_phy_bb_backup[14]);

		PHY_SetBBReg(priv, 0xee0, bMaskDWord, priv->pshare->rf_phy_bb_backup[15]);
		PHY_SetBBReg(priv, 0xeec, bMaskDWord, priv->pshare->rf_phy_bb_backup[16]);

		PHY_SetBBReg(priv, 0xc04, bMaskDWord , priv->pshare->rf_phy_bb_backup[17]);
		PHY_SetBBReg(priv, 0xd04, bMaskDWord , priv->pshare->rf_phy_bb_backup[18]);
		PHY_SetBBReg(priv, 0x90c, bMaskDWord , priv->pshare->rf_phy_bb_backup[19]);
		PHY_SetBBReg(priv, 0xa04, bMaskDWord , priv->pshare->rf_phy_bb_backup[22]);

#ifdef CONFIG_RTL_92C_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192C)
			PHY_SetBBReg(priv, 0x804, bMaskDWord , priv->pshare->rf_phy_bb_backup[20]);
#endif
	}
}

#ifdef __LINUX_2_6__
irqreturn_t gpio_wakeup_isr(int irq, void *dev_instance);
#else
void gpio_wakeup_isr(int irq, void *dev_instance, struct pt_regs *regs);
#endif

//const unsigned int CLK_MANAGE =	0xb8000010;
//const unsigned int SYS_PCIE_PHY0   =(0xb8000000 +0x50);
void PCIE_reset_procedure3(struct rtl8192cd_priv *priv)

{

	//PCIE Register
	unsigned int PCIE_PHY0_REG, PCIE_PHY0, linkstatus, haddr;
	int status = 0, counter = 0;

	if (GET_CHIP_VER(priv) == VERSION_8192D)
		haddr = CFG_92D_SLOTH;
	else
		haddr = CFG_92C_SLOTH;

	PCIE_PHY0_REG  = haddr + 0x1000;
	PCIE_PHY0 = haddr + 0x1008;
	linkstatus = haddr + 0x728;


#if 0
	REG32(CLK_MANAGE) &= ~(1 << 12); //perst=0 off.
	//dprintf("CLK_MANAGE=%x \n",  REG32(CLK_MANAGE));
	delay_ms(3);
	delay_ms(3);

	REG32(CLK_MANAGE) |=  (1 << 12);	//PERST=1
	//prom_printf("\nCLK_MANAGE(0x%x)=0x%x\n\n",CLK_MANAGE,READ_MEM32(CLK_MANAGE));

	//4. PCIE PHY Reset
	REG32(PCIE_PHY0) = 0x01; //bit7 PHY reset=0   bit0 Enable LTSSM=1
	REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1	bit0
	//
	delay_ms(3);

#else
	do {
		//2.Active LX & PCIE Clock
		REG32(CLK_MANAGE) &=  (~(1 << 11));      //enable active_pcie0
		delay_ms(2);

		//4. PCIE PHY Reset
		REG32(PCIE_PHY0) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
		delay_ms(2);

		REG32(CLK_MANAGE) &= ~(1 << 12);  //perst=0 off.
		delay_ms(5);

		REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
		delay_ms(5);

		REG32(CLK_MANAGE) |=  (1 << 11);		 //enable active_pcie0

		//---------------------------------------
		// 6. PCIE Device Reset

		delay_ms(5);
		REG32(CLK_MANAGE) |=  (1 << 12); //PERST=1
		delay_ms(5);
		status = REG32(linkstatus) & 0x1f;

		if ( status == 0x11 ) {
			break;
		} else  {
			delay_ms(100);
//			printk("status=%x\n", status);
			if ( ++counter > 1000) {
//				panic_printk("PCIe reset fail!!!!\n");
				break;
			}
		}
	} while (1);



#endif

//	printk("PCIE_reset_procedure3\t devid=%x\n",REG32(0xb8b10000));

}

#ifdef ASPM_ENABLE
void ASPM_on_off(struct rtl8192cd_priv *priv)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	SAVE_INT_AND_CLI(flags);
	unsigned int haddr, saddr;
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		haddr = CFG_92D_SLOTH;
		saddr = CFG_92D_SLOTS;
	} else {
		haddr = CFG_92C_SLOTH;
		saddr = CFG_92C_SLOTS;
	}
#ifdef CONFIG_RTL_92D_DMDP
	Sw_PCIE_Func2(priv->pshare->wlandev_idx);
#endif

	if (priv->pshare->rf_ft_var.power_save & ASPM_en) {
		if (priv->pwr_state == L0) {

			REG32(haddr + 0x70c) &= ~  ((0x7 << 27) | (0x7 << 24));
			REG32(haddr + 0x70c) |=  ((3) << 27) | ((1) << 24);	//L1=8us, L0s=2us

			REG32(haddr + 0x80) &= ~(0x3);
			REG32(saddr + 0x80) &= ~(0x3);
			REG32(haddr + 0x80) |= 1;  //L0s
			REG32(saddr + 0x80) |= 1;

			priv->pwr_state = ASPM_L0s_L1;
		}
	} else if (priv->pwr_state == ASPM_L0s_L1) {
		REG32(haddr + 0x80) &= ~(0x3);
		REG32(saddr + 0x80) &= ~(0x3);
		priv->pwr_state = L0;
	}

	RESTORE_INT(flags);
}
#endif

#ifdef GPIO_WAKEPIN
int request_irq_for_wakeup_pin(struct net_device *dev)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	unsigned int saddr;

	if (GET_CHIP_VER(priv) == VERSION_8192D)
		saddr = CFG_92D_SLOTS;
	else
		saddr = CFG_92C_SLOTS;

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)

#if defined(CONFIG_RTL_ULINKER)
	{
		// GPIO B5, b8000044 [10:9] = 2'b 11
		REG32(0xb8000044) = (REG32(0xb8000044) & ~0x600) | 0x600;       //LEDPHASE1
		REG32(0xb8003500) &= ~(BIT(13));			//PABCD_CNR , gpio pin
		REG32(0xb8003508) &= ~(BIT(13));			//PABCD_DIR
		REG32(0xb8003514) &= (~(0x03 << 26));
		REG32(0xb8003514) |= (0x01 << 26);			// enable interrupt in falling-edge
		REG32(PABCD_ISR) = BIT(13) ;				// clear int status
	}
#else

#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_8197D)
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		// GPIO B7, b8000044 [17:15] =3'b 100
		REG32(0xb8000044) = (REG32(0xb8000044) & ~0x00038000) | BIT(17);       //LEDPHASE1
		REG32(0xb8003500) &= ~(BIT(15));			//PABCD_CNR , gpio pin
		REG32(0xb8003508) &= ~(BIT(15));			//PABCD_DIR
		REG32(0xb8003514) &= (~(0x03 << 30));
		REG32(0xb8003514) |= (0x01 << 30);			// enable interrupt in falling-edge
		REG32(PABCD_ISR) = BIT(15) ;				// clear int status

	} else
#endif
	{
		// GPIO A4, b8000040 [2:0] = 3'b 110
		REG32(0xb8000040) = (REG32(0xb8000040) & ~0x7) | 0x6;                //JTAG_TMS
		REG32(0xb8003500) &= ~(BIT(4));			//PABCD_CNR , gpio pin
		REG32(0xb8003508) &= ~(BIT(4));			//PABCD_DIR
		REG32(0xb8003514) &= (~(0x03 << 8));
		REG32(0xb8003514) |= (0x01 << 8);			// enable interrupt in falling-edge
		REG32(PABCD_ISR) = BIT(4) ;					// clear int status
	}
#endif /* #if defined(CONFIG_RTL_ULINKER) */

	REG32(0xb8003000) |= BIT(16);                   // GIMR

#elif defined(CONFIG_RTL_8198)
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
// GPIO C1
		REG32(0xb8000044) |= BIT(16) | BIT(17);     //LEDPHASE1
		REG32(0xb8003500) &= ~(BIT(17));			//PABCD_CNR , gpio pin
		REG32(0xb8003508) &= ~(BIT(17));			//PABCD_DIR
		REG32(0xb8003518) &= (~(0x03 << 2));
		REG32(0xb8003518) |= (0x01 << 2);			// enable interrupt in falling-edge
		REG32(PABCD_ISR) = BIT(17) ;				// clear int status

	} else
#endif
	{
// GPIO C4
		REG32(0xb8000044) |= BIT(24);                //LEDPHASE4
		REG32(0xb8003500) &= ~(BIT(20));			//PABCD_CNR , gpio pin
		REG32(0xb8003508) &= ~(BIT(20));			//PABCD_DIR
		REG32(0xb8003518) &= (~(0x03 << 8));
		REG32(0xb8003518) |= (0x01 << 8);			// enable interrupt in falling-edge
		REG32(PABCD_ISR) = BIT(20) ;					// clear int status
	}
	REG32(0xb8003000) |= BIT(16);                   // GIMR
#else
// GPIO B7
	REG32(0xb8000040) |= 0x0c00;				//LEDPHASE1 :GPIOB7
	REG32(0xb8003500) &= ~(BIT(15));;			//PABCD_CNR , gpio pin
	REG32(0xb8003508) &= ~(BIT(15));			//PABCD_DIR
	REG32(0xb8003514) &= (~(0x03 << 30));
	REG32(0xb8003514) |= (0x01 << 30);			// PAB_IMR		// enable interrupt in falling-edge
	REG32(0xb8003000) |= BIT(9);				// GIMR

	REG32(PABCD_ISR) = BIT(15) ;		// clear int status
#endif

#ifdef CONFIG_RTL_92D_DMDP
	Sw_PCIE_Func2(priv->pshare->wlandev_idx);
#endif

	REG32(saddr + 0x44) = 0x8108; 						// clear pme status

#ifdef CONFIG_RTL_92D_DMDP
	Sw_PCIE_Func2(0);
#endif

#if defined(__LINUX_2_6__)
	return request_irq(BSP_GPIO_ABCD_IRQ, gpio_wakeup_isr, IRQF_SHARED, "rtk_gpio", dev);
#else
	return request_irq(1, gpio_wakeup_isr, SA_SHIRQ, "rtl_gpio", dev);
#endif

}
#endif


void init_pcie_power_saving(struct rtl8192cd_priv *priv)
{

	unsigned int saddr;
	if (GET_CHIP_VER(priv) == VERSION_8192D)
		saddr = CFG_92D_SLOTS;
	else
		saddr = CFG_92C_SLOTS;

// Jason : clk req
#if 0
	REG32(0xb9000354) = 8;
	REG32(0xb9000358) = 0x30;
#endif

#ifdef FIB_96C
	if (REG32(SYSTEM_BASE ) == 0x80000001) {
#if defined(__LINUX_2_6__)
#else
		extern void HostPCIe_SetPhyMdioWrite(unsigned int , unsigned int , unsigned short );
#endif
		HostPCIe_SetPhyMdioWrite(0, 8, 0x18d5);	// 18dd -> 18d5
		HostPCIe_SetPhyMdioWrite(0, 0xd, 0x15a6);	// 15b6 -> 15a6
	}
#endif

// Jason , for ASPM read_reg
	if ((GET_CHIP_VER(priv) == VERSION_8192C)  || (GET_CHIP_VER(priv) == VERSION_8188C)) {
		RTL_W16(0x354, 0x18e);
		RTL_W16(0x358, 0x23);

		if ((GET_CHIP_VER(priv) == VERSION_8188C) ) {
			RTL_W16(0x354, 0x20eb);
			RTL_W16(0x358, 0x3d);
		}
	}

#ifdef CONFIG_RTL_92D_DMDP
	Sw_PCIE_Func2(priv->pshare->wlandev_idx);
#endif

	REG32(saddr + 0x80) |= 0x0100;
#ifdef ASPM_ENABLE
	REG32(saddr + 0x80) |= 0x43;
#endif
	REG32(saddr + 0x0778) |= BIT(5) << 8;

#ifdef CONFIG_RTL_92C_SUPPORT

// 92c backdoor
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
		REG32(saddr + 0x70c) |= BIT(7) << 24;
		REG32(saddr + 0x718) |= (BIT(3) | BIT(4)) << 8;
//	   	dprintf("70f=%x,719=%x\n",  REG32(0xb8b1070f), REG32(0xb8b10719) );
	}
#endif

	RTL_W8(0x690, 0x2); 	// WoW
	RTL_W8(0x302, 0x2); 	// sw L123
	RTL_W8(0x5, 0x0);		// AFSM_PCIE
	RTL_W16(PCIE_CTRL_REG, 0xff );

//	RTL_W16(0x558, 0x040a);
//	RTL_W16(0x100 , RTL_R16(0x100) | BIT(8));		// enable sw beacon

#ifdef CONFIG_RTL_92C_SUPPORT
	if (IS_TEST_CHIP(priv)) {
		priv->pshare->rf_ft_var.power_save &= (~ L2_en);
		priv->pshare->rf_ft_var.power_save &= (~ASPM_en);
	} else
#endif
	{
		RTL_W8(0x08, RTL_R8(0x08) | BIT(3));		// WAKEPAN_EN
#ifdef CONFIG_RTL_92C_SUPPORT
		if (IS_UMC_A_CUT_88C(priv))
			priv->pshare->rf_ft_var.power_save &= (~ASPM_en);
#endif

	}
#ifdef ASPM_ENABLE
	ASPM_on_off(priv);
#endif

#ifdef CONFIG_RTL_92D_DMDP
	Sw_PCIE_Func2(1);
#endif

}


int isPSConditionMatch(struct rtl8192cd_priv *priv)
{

// temporary disable Active ECO when 1 interfcae is disabled
#ifdef CONFIG_RTL_92D_DMDP
	if ( (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY) &&
			(!IS_DRV_OPEN(((struct rtl8192cd_priv *)if_priv[1 & (priv->pshare->wlandev_idx ^ 1)]))))
		return 0;
#endif

	if (!IS_DRV_OPEN(priv))
		return 1;

	if ( (priv->assoc_num == 0)
			&& (priv->pshare->rf_ft_var.power_save & (L1_en | L2_en))
#ifdef MBSSID
			&& (!priv->pmib->miscEntry.vap_enable)
#endif
#ifdef WDS
			&& (!priv->pmib->dot11WdsInfo.wdsEnabled)
#endif
#ifdef UNIVERSAL_REPEATER
			&& (!IS_DRV_OPEN(GET_VXD_PRIV(priv)))
#endif
#ifdef CLIENT_MODE
			&& !((OPMODE & WIFI_STATION_STATE) || (OPMODE & WIFI_ADHOC_STATE))
#endif
	   )
		return 1;
	else
		return 0;

}

void PCIe_power_save_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	char force;
	force = priv->ps_ctrl & 0x80;
	priv->ps_ctrl &= 0x7f;

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific)
		return ;
#endif

	if (!IS_DRV_OPEN(priv))
		return;

	if (force == 0) {
#ifdef CONFIG_RTL_92D_DMDP
		if (isPSConditionMatch((struct rtl8192cd_priv *)if_priv[0]) && isPSConditionMatch((struct rtl8192cd_priv *)if_priv[1]))
#else
		if (isPSConditionMatch(priv))
#endif
		{
			if (priv->pwr_state != L1 && priv->pwr_state != L2) {
				if ((priv->offload_ctrl >> 7) && (priv->offload_ctrl & 1) == 0) {
					if (priv->pshare->rf_ft_var.power_save & L2_en)
						priv->ps_ctrl = 0x21;
					else
						priv->ps_ctrl = 0x11;
#ifdef CONFIG_RTL_92D_DMDP
					if ((priv->pshare->wlandev_idx == 1) &&
							(!IS_DRV_OPEN(((struct rtl8192cd_priv *)if_priv[0])))  ) {
						((struct rtl8192cd_priv *)if_priv[0])->ps_ctrl = priv->ps_ctrl;
#ifdef __OSK__
							PCIe_power_save_tasklet(priv);
#else
						tasklet_schedule(&((struct rtl8192cd_priv *)if_priv[0])->pshare->ps_tasklet);
#endif
					} else if ((priv->pshare->wlandev_idx == 0) &&
							   ((!IS_DRV_OPEN(((struct rtl8192cd_priv *)if_priv[1]))) || (((struct rtl8192cd_priv *)if_priv[1])->offload_ctrl >> 7)))
#endif
#ifdef __OSK__
						PCIe_power_save_tasklet(priv);
#else
						tasklet_schedule(&priv->pshare->ps_tasklet);
#endif
				} else {
					priv->offload_ctrl = 1;
					RTL_W16(CR , RTL_R16(CR) | ENSWBCN);		// enable sw beacon
					mod_timer(&priv->ps_timer, jiffies + RTL_SECONDS_TO_JIFFIES(1));
					return;
				}
			}
		} else {
			priv->offload_ctrl = 0;
		}

	} else {
		if (priv->pwr_state == L0)
			tasklet_schedule(&priv->pshare->ps_tasklet);
	}
	mod_timer(&priv->ps_timer, jiffies + POWER_DOWN_T0);

#ifdef ASPM_ENABLE
	ASPM_on_off(priv);
#endif

}

void setBaseAddressRegister(void)
{
	int tmp32 = 0, status;
	while (++tmp32 < 100) {
		REG32(0xb8b00004) = 0x00100007;
		REG32(0xb8b10004) = 0x00100007;
		REG32(0xb8b10010) = 0x18c00001;
		REG32(0xb8b10018) = 0x19000004;
		status = (REG32(0xb8b10010) ^ 0x18c00001) | ( REG32(0xb8b10018) ^ 0x19000004);
		if (!status)
			break;
		else {
			printk("set BAR fail,%x\n", status);
			printk("%x %x %x %x \n",
				   REG32(0xb8b00004) , REG32(0xb8b10004) , REG32(0xb8b10010),  REG32(0xb8b10018) );
		}
	} ;
}

void PCIe_power_save_tasklet(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	char in_out, L1_L2;
	unsigned int tmp32 = 0, status = 0, page = 0, ctr;
#ifdef CONFIG_RTL_92D_SUPPORT
	unsigned int portnum = 0, i;
#endif
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	unsigned int saddr, haddr;
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		saddr = CFG_92D_SLOTS;
		haddr = CFG_92D_SLOTH;

		portnum = RTL_USED_PCIE_SLOT;
	} else
#endif
	{
		saddr = CFG_92C_SLOTS;
		haddr = CFG_92C_SLOTH;
	}

	priv->ps_ctrl &= 0x7f;
	in_out = priv->ps_ctrl & 0xf;
	L1_L2 = (priv->ps_ctrl >> 4) & 0x7;

#if defined(CONFIG_RTL_92D_DMDP)
	if ((in_out == PWR_STATE_IN) && (priv->pshare->wlandev_idx == 0) && (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)) {
		struct rtl8192cd_priv *priv1 = ((struct rtl8192cd_priv *)if_priv[1]);
		priv1->ps_ctrl = priv->ps_ctrl;
		PCIe_power_save_tasklet((unsigned long )priv1);
	}
#endif

	DEBUG_INFO("%s, %s, L%d\n", __FUNCTION__, (in_out == PWR_STATE_IN ? "in" : "out") , L1_L2);

	if ( in_out == PWR_STATE_IN) 	{
		SAVE_INT_AND_CLI(flags);

#ifdef CONFIG_RTL_92D_DMDP
		Sw_PCIE_Func2(priv->pshare->wlandev_idx);

		if (!IS_DRV_OPEN(priv)) {
#ifdef ASPM_ENABLE
			if ( priv->pwr_state == ASPM_L0s_L1) {
				REG32(haddr + 0x80) &= ~(0x3);
				REG32(saddr + 0x80) &= ~(0x3);
			}
#endif
			RTL_W8(PCIE_CTRL_REG + 2, 0x2); // sw L123
			RTL_W16(PCIE_CTRL_REG, 0xff00 );
			REG32(saddr + 0x44) |= 0x8108;

			if (L1_L2 == L1) {
				priv->pwr_state = L1;
			} else {
				RTL_W8(RSV_CTRL0, 0xe1);		// reg lock, dis_prst
				RTL_W8(RSV_CTRL0, 0xe1);
				priv->pwr_state = L2;
			}
			RESTORE_INT(flags);
			return;
		}
#endif

#ifdef ASPM_ENABLE
		if ( priv->pwr_state == ASPM_L0s_L1) {
			REG32(haddr + 0x80) &= ~(0x3);
			REG32(saddr + 0x80) &= ~(0x3);
		}
#endif

//		RTL_W8(0x286,  4);
		RTL_W8(PCIE_CTRL_REG + 2, 0x2); // sw L123
		REG32(saddr + 0x44) |= 0x8108; 		// clear pme status

		RTL_W16(NQOS_SEQ, priv->pshare->phw->seq);
		RTL_W8(HWSEQ_CTRL, 0x7f);			// mac seq

		ctr = 3000;
		do {
			delay_us(100);
			if (!RTL_R8(PCIE_CTRL_REG)) {
				RTL_W8(PCIE_CTRL_REG + 1, 0xfe );
				break;
			}
		} while (--ctr);
		if (!ctr) {
			status = 1;
		}
		ctr = 3000;
		do {
			delay_us(100);
			if ( ((RTL_R32(0x200) ^ RTL_R32(0x204)) == 0) &&
					(((RTL_R32(VOQ_INFO) | RTL_R32(VIQ_INFO) | RTL_R32(BEQ_INFO) | RTL_R32(BKQ_INFO)
					   | RTL_R32(MGQ_INFO) | RTL_R32(HIQ_INFO)) & 0xffff00) == 0)
			   ) {
				break;
			}
		} while (--ctr);
		if (!ctr) {
			status = 1;
		}
		RTL_W8(TXPAUSE, 0x2f);
		delay_ms(1);

#ifdef PCIE_L2_ENABLE
		if (L1_L2 == L2) {
			int 	tx_head, tx_tail, q_num;
			struct tx_desc		*phdesc, *pdesc;
			for (q_num = MGNT_QUEUE; q_num <= HIGH_QUEUE; q_num++) {
				tx_head 	= get_txhead(GET_HW(priv), q_num);
				tx_tail 	= get_txtail(GET_HW(priv), q_num);
				phdesc		= get_txdesc(GET_HW(priv), q_num);
				while (tx_tail != tx_head) {
					pdesc	  = phdesc + (tx_tail);
					pdesc->Dword0 &= set_desc(~TX_OWN);
#ifdef OSK_LOW_TX_DESC
					tx_tail = (tx_tail + 1) % ((q_num==BE_QUEUE || q_num==HIGH_QUEUE)?BE_TXDESC:NONBE_TXDESC);
#else
					tx_tail = (tx_tail + 1) % CURRENT_NUM_TX_DESC;
#endif
				}
			}
#ifdef SMP_SYNC
			if (!priv->pshare->has_triggered_tx_tasklet) {
				tasklet_schedule(&priv->pshare->tx_tasklet);
				priv->pshare->has_triggered_tx_tasklet = 1;
			}
#else
			rtl8192cd_tx_dsr((unsigned long)priv);
#endif
			rtl8192cd_rx_isr(priv);
		}
#endif
		if ((get_rf_mimo_mode(priv) == MIMO_2T2R) && (priv->pshare->rf_ft_var.power_save & _1x1_en))
			switch_to_1x1(priv,	PWR_STATE_IN);

		page = ((priv->offload_ctrl) >> 7) & 0xff;
		RTL_W16(RCR, RTL_R16(RCR) & ~(BIT(11) | BIT(12) | BIT(13)));
		RTL_W16(RXFLTMAP0, RTL_R16(RXFLTMAP0) | BIT(11) | BIT(4));

		if (priv->pshare->rf_ft_var.power_save & offload_en)  {

			if (GET_CHIP_VER(priv) != VERSION_8192D)
				status |= signin_h2c_cmd(priv, _RSVDPAGE_CMD_ | page << 8, 0);
			else
				status |= signin_h2c_cmd(priv, _RSVDPAGE_CMD_ | BIT(7) | (page << 8), (page << 8) | (page) );

			status |= signin_h2c_cmd(priv, _AP_OFFLOAD_CMD_ | (1 << 8) | (HIDDEN_AP << 16) | ((GET_MIB(priv))->dot11OperationEntry.deny_any) << 24, 0 );
#if defined(__LINUX_2_6__) || defined(__OSK__)
			RTL_W32(HIMR, 0);
#endif
			DEBUG_INFO("%s, LINE:%d, h2c %x, %x, %x\n", __FUNCTION__, __LINE__,
					   (_RSVDPAGE_CMD_ | page << 8), (_RSVDPAGE_CMD_ | BIT(7) | (page << 8), (page << 8) | (page)),
					   (_AP_OFFLOAD_CMD_ | (1 << 8) | (HIDDEN_AP << 16) | ((GET_MIB(priv))->dot11OperationEntry.deny_any) << 24)
					  );
			ctr = 3000;
			do {
				delay_us(10);
				page = RTL_R8(RXPKT_NUM + 2) & 6 ;
				if (page == 6)
					break;
			} while (--ctr);

			if (status || (page != 6)) {
				DEBUG_INFO("signin_h2c_cmd fail(ap offload), 286=%x\n", page);
#if defined(__LINUX_2_6__) || defined(__OSK__)
				RTL_W32(HIMR, priv->pshare->InterruptMask);
#endif
				if ((get_rf_mimo_mode(priv) == MIMO_2T2R) && (priv->pshare->rf_ft_var.power_save & _1x1_en))
					switch_to_1x1(priv, PWR_STATE_OUT);
				RTL_W8(HWSEQ_CTRL, 0x0); 		// mac seq disable
				RTL_W8(RXPKT_NUM + 2, 0);

				RTL_W16(RCR, RTL_R16(RCR) | (BIT(11) | BIT(13)));
				RTL_W16(RXFLTMAP0, RTL_R16(RXFLTMAP0) & ~(BIT(11) | BIT(4)));

				RTL_W8(PCIE_CTRL_REG + 1, 0x00);	// enable DMA
				RTL_W8(TXPAUSE, 0x0);

				priv->offload_ctrl = 0;
				RESTORE_INT(flags);
				return;
			}
		}
		RTL_W16(CR , RTL_R16(CR) | ENSWBCN);		// enable sw beacon

		if ( L1_L2 == L1) {

#ifdef CONFIG_RTL_92D_DMDP
			if ((priv->pshare->wlandev_idx) && (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
					&&  IS_DRV_OPEN(((struct rtl8192cd_priv *)if_priv[0])))	{
				RESTORE_INT(flags);
				return ;
			}
			Sw_PCIE_Func2(1);
			tmp32 = REG32(saddr + 0x44) & ( ~(3));
			REG32(saddr + 0x44) = tmp32 |		(3); //D3

			if (!priv->pshare->wlandev_idx)
				((struct rtl8192cd_priv *)if_priv[1])->pwr_state = L1;
			Sw_PCIE_Func2(0);
			delay_ms(1);
#endif
			priv->pwr_state = L1;

			tmp32 = REG32(saddr + 0x44) & ( ~(3)); //D0
			REG32(saddr + 0x44) = tmp32 |	(3); //D3
			//HostPCIe_SetPhyMdioWrite(0xd, 0x15a6);
			printk("D3 hot -> L1\n");
			delay_ms(1);
#if 0 //saving more power
			HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0708);
#endif
		}
		RESTORE_INT(flags);


#ifdef PCIE_L2_ENABLE
		if ( L1_L2 == L2) {
#if 0 //saving more power   leave L1 write
			HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0f0f);
#endif
			RTL_W8(RSV_CTRL0, 0xe1);		// reg lock, dis_prst
			RTL_W8(RSV_CTRL0, 0xe1);
			priv->pwr_state = L2;
#ifdef CONFIG_RTL_92D_DMDP
			if (!priv->pshare->wlandev_idx)
#endif
				REG32(haddr + 0x1008) |= (0x200);
			printk("PME turn off -> L2\n");
		}
#endif
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
		REG32(0xb8003000) |= BIT(16);		// GIMR
#else
		REG32(0xb8003000) |= BIT(9);		// GIMR
#endif
	} else if (in_out == PWR_STATE_OUT) {
#ifdef PCIE_L2_ENABLE
		if ( L1_L2 == L2) {

#ifdef CONFIG_RTL_92D_DMDP
			Sw_PCIE_Func2(priv->pshare->wlandev_idx);
			if (!priv->pshare->wlandev_idx )
#endif
				PCIE_reset_procedure3(priv);
			setBaseAddressRegister();

			SAVE_INT_AND_CLI(flags);
			priv->pwr_state = L0;
#ifdef CONFIG_RTL_92D_DMDP
			((struct rtl8192cd_priv *)if_priv[1])->pwr_state = L0;
#endif
			RTL_W8(RSV_CTRL0, 0x0);
			tmp32 = 0;
			while (1) {
				if ( !(RTL_R8(SPS0_CTRL) & BIT(3)) || (++tmp32 > 20) ) {
					RTL_W8(SPS0_CTRL, 0x2b);
					break;
				}
			}
			DEBUG_INFO("SPS0_CTRL=%d !!\n", RTL_R8(SPS0_CTRL));
		} else
#endif
		{

			SAVE_INT_AND_CLI(flags);
			if ( priv->pwr_state == L1) {

#ifdef CONFIG_RTL_92D_DMDP
				for (i = 0; i < 2; i++) {
					Sw_PCIE_Func2(i);
#endif
					ctr = 3000;
#if 0 //saving more power, leave L1 write
					HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0f0f);
#endif
					tmp32 = REG32(saddr + 0x44) & ( ~(3)); //D0
					do {
						REG32(saddr + 0x44) = tmp32 |	(0); //D0
						delay_us(1);
						REG32(saddr + 0x44) = tmp32 |	(0); //D0
						status = REG32(haddr + 0x728) & 0x1f;
						if (status == 0x11)
							break;
					} while (--ctr);

					if (status != 0x11)
						panic_printk("change to L0 fail!!!, status=%x, MAC0\n", status);
					else
#ifdef CONFIG_RTL_92D_DMDP
						((struct rtl8192cd_priv *)if_priv[i])->pwr_state = L0;
				}
#else
						priv->pwr_state = L0;
#endif
			}
		}


#ifdef CONFIG_RTL_92D_DMDP
		if ((priv->pshare->wlandev_idx == 0) && (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY) && !IS_DRV_OPEN(priv) ) {
			goto OEPN_MAC1;
		}
#endif

		if (priv->pshare->rf_ft_var.power_save & offload_en) {

#if defined(__LINUX_2_6__) || defined(__OSK__)
			RTL_W32(HIMR, priv->pshare->InterruptMask);
#endif
			signin_h2c_cmd(priv, _AP_OFFLOAD_CMD_ , 0 );		// offload
			delay_ms(2);
		}

		if ((get_rf_mimo_mode(priv) == MIMO_2T2R) && (priv->pshare->rf_ft_var.power_save & _1x1_en))
			switch_to_1x1(priv,	PWR_STATE_OUT);
#ifdef PCIE_L2_ENABLE
		if ( L1_L2 == L2) {
			priv->pshare->phw->cur_rx = 0;
#ifdef DELAY_REFILL_RX_BUF
			priv->pshare->phw->cur_rx_refill = 0;
#endif
			memset(&(priv->pshare->phw->txhead0), 0, sizeof(int) * 12);
			RTL_W32(BCNQ_DESA, priv->pshare->phw->tx_ringB_addr);
			RTL_W32(MGQ_DESA, priv->pshare->phw->tx_ring0_addr);
			RTL_W32(VOQ_DESA, priv->pshare->phw->tx_ring4_addr);
			RTL_W32(VIQ_DESA, priv->pshare->phw->tx_ring3_addr);
			RTL_W32(BEQ_DESA, priv->pshare->phw->tx_ring2_addr);
			RTL_W32(BKQ_DESA, priv->pshare->phw->tx_ring1_addr);
			RTL_W32(HQ_DESA, priv->pshare->phw->tx_ring5_addr);
			RTL_W32(RX_DESA, priv->pshare->phw->ring_dma_addr);
		}
#endif
		// wait until FW stop parsing packet
		ctr = 1000;
		do {
			if (!(RTL_R8(FWIMR) & FWIMR_RXDONE))
				break;
			delay_us(200);
		} while (--ctr) ;
		if (!ctr)
			DEBUG_ERR("stop offload fail\n");

		RTL_W8(HWSEQ_CTRL, 0x0); 		// mac seq disable
		RTL_W8(RXPKT_NUM + 2, 0);			// RW_RELEASE_ENABLE
		RTL_W16(RCR, RTL_R16(RCR) | (BIT(11) | BIT(13)));
		RTL_W16(RXFLTMAP0, RTL_R16(RXFLTMAP0) & ~(BIT(11) | BIT(4)));
		RTL_W8(PCIE_CTRL_REG + 1, 0x00);	// enable DMA
		RTL_W8(PCIE_CTRL_REG + 2, 0x3);	// sw L123
		RTL_W8(TXPAUSE, 0x0);
		RTL_W16(CR , RTL_R16(CR) & ~ENSWBCN);

#ifdef CONFIG_RTL_92D_DMDP
		if ((priv->pshare->wlandev_idx == 0) && (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)) {
			struct rtl8192cd_priv *priv1;
OEPN_MAC1:
			priv1 = ((struct rtl8192cd_priv *)if_priv[1]);
			if (IS_DRV_OPEN(priv1) ) {
				priv1->ps_ctrl = priv->ps_ctrl;
				PCIe_power_save_tasklet((unsigned long )priv1);
			}
		}
#endif

#if defined(RX_TASKLET)
		if (IS_DRV_OPEN(priv)) {
			tasklet_hi_schedule(&priv->pshare->rx_tasklet);
		}
#endif

#ifdef ASPM_ENABLE
		ASPM_on_off(priv);
#endif
		RESTORE_INT(flags);
	}
}


void PCIeWakeUp(struct rtl8192cd_priv *priv, unsigned int expTime)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	SAVE_INT_AND_CLI(flags);

	if ( (priv->pwr_state == L1) || (priv->pwr_state == L2)) {

		if (timer_pending(&priv->ps_timer))
			del_timer_sync(&priv->ps_timer);

#ifdef CONFIG_RTL_92D_DMDP
		if (priv->pshare->wlandev_idx == 1) {
			struct rtl8192cd_priv *priv0 = ((struct rtl8192cd_priv *)if_priv[0]);
			PCIeWakeUp(priv0, expTime);
		} else
#endif
		{
			priv->ps_ctrl = 0x02 | (priv->pwr_state << 4);
			PCIe_power_save_tasklet((unsigned long)priv);
		}

		mod_timer(&priv->ps_timer, jiffies + expTime);
		priv->offload_ctrl = 0;
#ifdef CONFIG_RTL_92D_DMDP
		((struct rtl8192cd_priv *)if_priv[priv->pshare->wlandev_idx ^ 1])->offload_ctrl = 0;
#endif
	}
	RESTORE_INT(flags);
}

#ifdef __LINUX_2_6__
irqreturn_t gpio_wakeup_isr(int irq, void *dev_instance)
#else
void gpio_wakeup_isr(int irq, void *dev_instance, struct pt_regs *regs)
#endif
{
	struct net_device *dev = NULL;
	struct rtl8192cd_priv *priv = NULL;

	unsigned int saddr;

	dev = (struct net_device *)dev_instance;
	priv = (struct rtl8192cd_priv *)dev->priv;

	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		saddr = CFG_92D_SLOTS;
	} else {
		saddr = CFG_92C_SLOTS;
	}

#ifdef CONFIG_RTL_92D_DMDP
	Sw_PCIE_Func2(priv->pshare->wlandev_idx);
#endif

	DEBUG_INFO("%s, PABCD_ISR=%x\t", dev->name, REG32(PABCD_ISR));

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)

#if defined(CONFIG_RTL_ULINKER)
	REG32(PABCD_ISR) = BIT(13) ; 	// clear int status
#else
#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_8197D)
	if (GET_CHIP_VER(priv) == VERSION_8192D)
		REG32(PABCD_ISR) = BIT(15) ;
	else
#endif
		REG32(PABCD_ISR) = BIT(4) ;		// clear int status
#endif /* #if defined(CONFIG_RTL_ULINKER) */

#elif defined(CONFIG_RTL_8198)
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D)
		REG32(PABCD_ISR) = BIT(17) ;
	else
#endif
		REG32(PABCD_ISR) = BIT(20) ;		// clear int status
#else
	REG32(PABCD_ISR) = BIT(15) ;			// clear int status
#endif

	DEBUG_INFO(", PABCD_ISR=%x,0xb8100044=%x\n", REG32(PABCD_ISR), REG32(saddr + 0x44));

#ifdef GPIO_WAKEPIN
#ifdef PCIE_POWER_SAVING_DEBUG
	priv->firstPkt = 1;
#endif
	if (timer_pending(&priv->ps_timer))
		del_timer_sync(&priv->ps_timer);

	if ( priv->pwr_state == L1 || priv->pwr_state == L2) {
		priv->ps_ctrl = 0x02 | (priv->pwr_state << 4);
		PCIe_power_save_tasklet((unsigned long)priv);
	}

	priv->offload_ctrl = 0;
	mod_timer(&priv->ps_timer, jiffies + (POWER_DOWN_T0));

#endif
#ifdef __LINUX_2_6__
	return IRQ_HANDLED;
#endif
}


void radio_off(struct rtl8192cd_priv *priv)
{
#if 0
	extern	void HostPCIe_Close(void);
	printk("Radio Off======>\n");
	HostPCIe_Close();
#endif
}
#endif


#if defined(RF_MIMO_SWITCH) || defined(PCIE_POWER_SAVING)

int MIMO_Mode_1Plus1(struct rtl8192cd_priv *priv)
{
	//panic_printk("switch to 1x1\n");
#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		Assert_BB_Reset(priv);
		priv->pshare->rf_phy_bb_backup[0] = RTL_R32(0x808); 	
		priv->pshare->rf_phy_bb_backup[1] = RTL_R32(0x80c); 	
		priv->pshare->rf_phy_bb_backup[2] = RTL_R32(0xa04); 	
		priv->pshare->rf_phy_bb_backup[3] = RTL_R32(0x8bc); 	
		priv->pshare->rf_phy_bb_backup[4] = RTL_R32(0xe00); 	
		priv->pshare->rf_phy_bb_backup[5] = RTL_R32(0xe90); 	
		priv->pshare->rf_phy_bb_backup[6] = RTL_R32(0xe60);
		priv->pshare->rf_phy_bb_backup[7] = RTL_R32(0xe64);
		RTL_W8(0x808, 0x11);
		RTL_W16(0x80c, 0x1111);
		RTL_W8(0xa07, (RTL_R8(0xa07) & 0xf3));	// [3:2] = 2'b 00
		RTL_W32(0x8bc, (RTL_R32(0x8bc) & 0x3FFFFF9F) | BIT(30) );
		RTL_W8(0xe00, (RTL_R8(0xe00) & 0xf0) | 0x04 );
		RTL_W8(0xe90, 0);
		RTL_W32(0xe60, 0);
		RTL_W32(0xe64, 0);
		Release_BB_Reset(priv);
	}
#endif	
#ifdef CONFIG_WLAN_HAL_8192EE
	if(GET_CHIP_VER(priv) == VERSION_8192E) {
		priv->pshare->rf_phy_bb_backup[0] = RTL_R32(0xd00); 		
		priv->pshare->rf_phy_bb_backup[23] = RTL_R32(0xe68);
		priv->pshare->rf_phy_bb_backup[24] = RTL_R32(0x82c);
		priv->pshare->rf_phy_bb_backup[25] = RTL_R32(0xa00);	
	}
#endif
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
	if ((GET_CHIP_VER(priv) == VERSION_8192C) || (GET_CHIP_VER(priv) == VERSION_8192D)) {
		priv->pshare->rf_phy_bb_backup[0] = RTL_R32(0x844);
		priv->pshare->rf_phy_bb_backup[20] = RTL_R32(0x804);			
		priv->pshare->rf_phy_bb_backup[21] = RTL_R32(0x88c);		
	}
#endif
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)
	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8192D) || (GET_CHIP_VER(priv) == VERSION_8192C)) {				
		priv->pshare->rf_phy_bb_backup[1] = RTL_R32(0x85c);
		priv->pshare->rf_phy_bb_backup[2] = RTL_R32(0xe6c);			
		priv->pshare->rf_phy_bb_backup[3] = RTL_R32(0xe70);
		priv->pshare->rf_phy_bb_backup[4] = RTL_R32(0xe74);
		priv->pshare->rf_phy_bb_backup[5] = RTL_R32(0xe78);
		priv->pshare->rf_phy_bb_backup[6] = RTL_R32(0xe7c);			
		priv->pshare->rf_phy_bb_backup[7] = RTL_R32(0xe80);
		priv->pshare->rf_phy_bb_backup[8] = RTL_R32(0xe84);
		priv->pshare->rf_phy_bb_backup[9] = RTL_R32(0xe88);
		priv->pshare->rf_phy_bb_backup[10] = RTL_R32(0xe8c);			
		priv->pshare->rf_phy_bb_backup[11] = RTL_R32(0xed0);
		priv->pshare->rf_phy_bb_backup[12] = RTL_R32(0xed4);
		priv->pshare->rf_phy_bb_backup[13] = RTL_R32(0xed8);
		priv->pshare->rf_phy_bb_backup[14] = RTL_R32(0xedc);			
		priv->pshare->rf_phy_bb_backup[15] = RTL_R32(0xee0);
		priv->pshare->rf_phy_bb_backup[16] = RTL_R32(0xeec);			
		priv->pshare->rf_phy_bb_backup[17] = RTL_R32(0xc04);
		priv->pshare->rf_phy_bb_backup[18] = RTL_R32(0xd04);
		priv->pshare->rf_phy_bb_backup[19] = RTL_R32(0x90c);
		priv->pshare->rf_phy_bb_backup[22] = RTL_R32(0xa04);
	
	}
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		unsigned int mask = 0xB4FFFFFF, path = 0x11;
		if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)	{
			mask = 0xDB3FFFFF;
			path = 0x22;
			RTL_W8(0xa07, 0x45);
		}
		PHY_SetBBReg(priv, 0x85c, bMaskDWord, priv->pshare->rf_phy_bb_backup[1] & mask);
		PHY_SetBBReg(priv, 0xe6c, bMaskDWord, priv->pshare->rf_phy_bb_backup[2] & mask);		
		PHY_SetBBReg(priv, 0xe70, bMaskDWord, priv->pshare->rf_phy_bb_backup[3] & mask);
		PHY_SetBBReg(priv, 0xe74, bMaskDWord, priv->pshare->rf_phy_bb_backup[4] & mask);
		PHY_SetBBReg(priv, 0xe78, bMaskDWord, priv->pshare->rf_phy_bb_backup[5] & mask);
		PHY_SetBBReg(priv, 0xe7c, bMaskDWord, priv->pshare->rf_phy_bb_backup[6] & mask);		
		PHY_SetBBReg(priv, 0xe80, bMaskDWord, priv->pshare->rf_phy_bb_backup[7] & mask);
		PHY_SetBBReg(priv, 0xe84, bMaskDWord, priv->pshare->rf_phy_bb_backup[8] & mask);
		PHY_SetBBReg(priv, 0xe88, bMaskDWord, priv->pshare->rf_phy_bb_backup[9] & mask);
		PHY_SetBBReg(priv, 0xe8c, bMaskDWord, priv->pshare->rf_phy_bb_backup[10] & mask);		
		PHY_SetBBReg(priv, 0xed0, bMaskDWord, priv->pshare->rf_phy_bb_backup[11] & mask);
		PHY_SetBBReg(priv, 0xed4, bMaskDWord, priv->pshare->rf_phy_bb_backup[12] & mask);
		PHY_SetBBReg(priv, 0xed8, bMaskDWord, priv->pshare->rf_phy_bb_backup[13] & mask);
		PHY_SetBBReg(priv, 0xedc, bMaskDWord, priv->pshare->rf_phy_bb_backup[14] & mask);		
		PHY_SetBBReg(priv, 0xee0, bMaskDWord, priv->pshare->rf_phy_bb_backup[15] & mask);
		PHY_SetBBReg(priv, 0xeec, bMaskDWord, priv->pshare->rf_phy_bb_backup[16] & mask);		
		PHY_SetBBReg(priv, 0xc04, 0x000000ff, path);
		PHY_SetBBReg(priv, 0xd04, 0x0000000f, path & 0x01);
		PHY_SetBBReg(priv, 0x90c, 0x000000ff, path);
		PHY_SetBBReg(priv, 0x90c, 0x0ff00000, path);		
	} 
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192C)	{
		PHY_SetBBReg(priv, 0x88c, 0x00c00000 , 0x3);
		PHY_SetBBReg(priv, 0x844, bMaskDWord, 0x00010000);				// standby
		PHY_SetBBReg(priv, 0x85c, bMaskDWord, 0x00db25a4);
		PHY_SetBBReg(priv, 0xe6c, bMaskDWord, 0x20db25a4);
		PHY_SetBBReg(priv, 0xe70, bMaskDWord, 0x20db25a4);
		PHY_SetBBReg(priv, 0xe74, bMaskDWord, 0x041b25a4);
		PHY_SetBBReg(priv, 0xe78, bMaskDWord, 0x041b25a4);
		PHY_SetBBReg(priv, 0xe7c, bMaskDWord, 0x041b25a4);
		PHY_SetBBReg(priv, 0xe80, bMaskDWord, 0x041b25a4);
		PHY_SetBBReg(priv, 0xe84, bMaskDWord, 0x63db25a4);
		PHY_SetBBReg(priv, 0xe88, bMaskDWord, 0x041b25a4);
		PHY_SetBBReg(priv, 0xe8c, bMaskDWord, 0x20db25a4);
		PHY_SetBBReg(priv, 0xed0, bMaskDWord, 0x20db25a4);
		PHY_SetBBReg(priv, 0xed4, bMaskDWord, 0x20db25a4);
		PHY_SetBBReg(priv, 0xed8, bMaskDWord, 0x20db25a4);
		PHY_SetBBReg(priv, 0xedc, bMaskDWord, 0x001b25a4);
		PHY_SetBBReg(priv, 0xee0, bMaskDWord, 0x001b25a4);
		PHY_SetBBReg(priv, 0xeec, bMaskDWord, 0x24db25a4);
		PHY_SetBBReg(priv, 0xc04, 0x000000ff , 0x11);
		PHY_SetBBReg(priv, 0xd04, 0x0000000f , 0x1);
		PHY_SetBBReg(priv, 0x90c, 0x000000ff , 0x11);
		PHY_SetBBReg(priv, 0x90c, 0x0ff00000 , 0x11);
		PHY_SetBBReg(priv, 0x804, 0x000000f , 0x1);
	}
#endif		
#ifdef CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E) {
		unsigned int mask = 0xB4FFFFFF;
		int pathB=0;
		if((RTL_R8(0xa07) & 0x80) == 0)
			pathB=1;
		PHY_SetBBReg(priv, 0x85c, bMaskDWord, priv->pshare->rf_phy_bb_backup[1] & mask);
		PHY_SetBBReg(priv, 0xe6c, bMaskDWord, priv->pshare->rf_phy_bb_backup[2] & mask);
		PHY_SetBBReg(priv, 0xe70, bMaskDWord, priv->pshare->rf_phy_bb_backup[3] & mask);
		PHY_SetBBReg(priv, 0xe74, bMaskDWord, priv->pshare->rf_phy_bb_backup[4] & mask);
		PHY_SetBBReg(priv, 0xe78, bMaskDWord, priv->pshare->rf_phy_bb_backup[5] & mask);
		PHY_SetBBReg(priv, 0xe7c, bMaskDWord, priv->pshare->rf_phy_bb_backup[6] & mask);
		PHY_SetBBReg(priv, 0xe80, bMaskDWord, priv->pshare->rf_phy_bb_backup[7] & mask);
		PHY_SetBBReg(priv, 0xe84, bMaskDWord, priv->pshare->rf_phy_bb_backup[8] & mask);
		PHY_SetBBReg(priv, 0xe88, bMaskDWord, priv->pshare->rf_phy_bb_backup[9] & mask);
		PHY_SetBBReg(priv, 0xe8c, bMaskDWord, priv->pshare->rf_phy_bb_backup[10] & mask);
		PHY_SetBBReg(priv, 0xed0, bMaskDWord, priv->pshare->rf_phy_bb_backup[11] & mask);
		PHY_SetBBReg(priv, 0xed4, bMaskDWord, priv->pshare->rf_phy_bb_backup[12] & mask);
		PHY_SetBBReg(priv, 0xed8, bMaskDWord, priv->pshare->rf_phy_bb_backup[13] & mask);
		PHY_SetBBReg(priv, 0xedc, bMaskDWord, priv->pshare->rf_phy_bb_backup[14] & mask);
		PHY_SetBBReg(priv, 0xee0, bMaskDWord, priv->pshare->rf_phy_bb_backup[15] & mask);
		PHY_SetBBReg(priv, 0xeec, bMaskDWord, priv->pshare->rf_phy_bb_backup[16] & mask);
		PHY_SetBBReg(priv, 0xe68, bMaskDWord, priv->pshare->rf_phy_bb_backup[23] & mask);			
		
		PHY_SetBBReg(priv, 0xc04, 0x000000ff , 0x11);
		PHY_SetBBReg(priv, 0xd04, 0x0000000f , 0x1);
		PHY_SetBBReg(priv, 0x90c, 0x000000ff , 0x11);
		PHY_SetBBReg(priv, 0x90c, 0x0ff00000 , 0x11);
		PHY_SetBBReg(priv, 0xd00, 0x00180000, 0);
		PHY_SetBBReg(priv, 0x82c, (BIT22 | BIT21| BIT20), 1);
		PHY_SetBBReg(priv, 0xa00, BIT7, 0); 	// ant div disable  

		if(pathB) {
			RTL_W8(0xa07, 0x40);
			PHY_SetBBReg(priv, 0xe74, BIT27, 1);
	
		} else {
			RTL_W8(0xa07, 0x80);
		}

	}
#endif		
	return 1;
}

int MIMO_Mode_2Plus2(struct rtl8192cd_priv *priv)
{
	//panic_printk("switch to 2x2\n");
#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		Assert_BB_Reset(priv);
		RTL_W32(0x808, priv->pshare->rf_phy_bb_backup[0]);	
		RTL_W32(0x80c, priv->pshare->rf_phy_bb_backup[1]);	
		RTL_W32(0xa04, priv->pshare->rf_phy_bb_backup[2]);	
		RTL_W32(0x8bc, priv->pshare->rf_phy_bb_backup[3]);	
		RTL_W32(0xe00, priv->pshare->rf_phy_bb_backup[4]);	
		RTL_W32(0xe90, priv->pshare->rf_phy_bb_backup[5]);	
		RTL_W32(0xe60, priv->pshare->rf_phy_bb_backup[6]);
		RTL_W32(0xe64, priv->pshare->rf_phy_bb_backup[7]);			
		Release_BB_Reset(priv);
	}
#endif	
#ifdef CONFIG_RTL_92C_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192C) {
		PHY_SetBBReg(priv, 0x88c, bMaskDWord, priv->pshare->rf_phy_bb_backup[21]);
		PHY_SetBBReg(priv, 0x844, bMaskDWord, priv->pshare->rf_phy_bb_backup[0]);
	}
#endif
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE)
	if ((GET_CHIP_VER(priv) == VERSION_8192D) || (GET_CHIP_VER(priv) == VERSION_8192C)	|| (GET_CHIP_VER(priv) == VERSION_8192E)) {
		PHY_SetBBReg(priv, 0x85c, bMaskDWord, priv->pshare->rf_phy_bb_backup[1]);
		PHY_SetBBReg(priv, 0xe6c, bMaskDWord, priv->pshare->rf_phy_bb_backup[2]);
		PHY_SetBBReg(priv, 0xe70, bMaskDWord, priv->pshare->rf_phy_bb_backup[3]);
		PHY_SetBBReg(priv, 0xe74, bMaskDWord, priv->pshare->rf_phy_bb_backup[4]);
		PHY_SetBBReg(priv, 0xe78, bMaskDWord, priv->pshare->rf_phy_bb_backup[5]);
		PHY_SetBBReg(priv, 0xe7c, bMaskDWord, priv->pshare->rf_phy_bb_backup[6]);
		PHY_SetBBReg(priv, 0xe80, bMaskDWord, priv->pshare->rf_phy_bb_backup[7]);
		PHY_SetBBReg(priv, 0xe84, bMaskDWord, priv->pshare->rf_phy_bb_backup[8]);
		PHY_SetBBReg(priv, 0xe88, bMaskDWord, priv->pshare->rf_phy_bb_backup[9]);
		PHY_SetBBReg(priv, 0xe8c, bMaskDWord, priv->pshare->rf_phy_bb_backup[10]);
		PHY_SetBBReg(priv, 0xed0, bMaskDWord, priv->pshare->rf_phy_bb_backup[11]);
		PHY_SetBBReg(priv, 0xed4, bMaskDWord, priv->pshare->rf_phy_bb_backup[12]);
		PHY_SetBBReg(priv, 0xed8, bMaskDWord, priv->pshare->rf_phy_bb_backup[13]);
		PHY_SetBBReg(priv, 0xedc, bMaskDWord, priv->pshare->rf_phy_bb_backup[14]);
		PHY_SetBBReg(priv, 0xee0, bMaskDWord, priv->pshare->rf_phy_bb_backup[15]);
		PHY_SetBBReg(priv, 0xeec, bMaskDWord, priv->pshare->rf_phy_bb_backup[16]);
		PHY_SetBBReg(priv, 0xc04, bMaskDWord , priv->pshare->rf_phy_bb_backup[17]);
		PHY_SetBBReg(priv, 0xd04, bMaskDWord , priv->pshare->rf_phy_bb_backup[18]);
		PHY_SetBBReg(priv, 0x90c, bMaskDWord , priv->pshare->rf_phy_bb_backup[19]);
		PHY_SetBBReg(priv, 0xa04, bMaskDWord , priv->pshare->rf_phy_bb_backup[22]);		
			
	}
#endif
#ifdef CONFIG_WLAN_HAL_8192EE
	if(GET_CHIP_VER(priv) == VERSION_8192E) {
		PHY_SetBBReg(priv, 0xd00, bMaskDWord, priv->pshare->rf_phy_bb_backup[0]);	
		PHY_SetBBReg(priv, 0xe68, bMaskDWord , priv->pshare->rf_phy_bb_backup[23]);
		PHY_SetBBReg(priv, 0x82c, bMaskDWord , priv->pshare->rf_phy_bb_backup[24]);
		PHY_SetBBReg(priv, 0xa00, bMaskDWord , priv->pshare->rf_phy_bb_backup[25]);	
	}
#endif
#ifdef CONFIG_RTL_92C_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192C)
		PHY_SetBBReg(priv, 0x804, bMaskDWord , priv->pshare->rf_phy_bb_backup[20]);
#endif
	
	if(GET_CHIP_VER(priv) == VERSION_8814A) {
		priv->pshare->rf_phy_bb_backup[0] = RTL_R32(0x808); 	
		priv->pshare->rf_phy_bb_backup[1] = RTL_R8(0x93e); 	
		priv->pshare->rf_phy_bb_backup[2] = RTL_R8(0x93f); 
		
		RTL_W8(0x808, 0x66);
		RTL_W8(0x93e, 0x2c);
		RTL_W8(0x93f, 0x00);
	}
	return 1;
}

int MIMO_Mode_3Plus3(struct rtl8192cd_priv *priv)
{
	//panic_printk("switch to 3x3\n");
	if(GET_CHIP_VER(priv) == VERSION_8814A) {
			PHY_SetBBReg(priv, 0x808, bMaskDWord, priv->pshare->rf_phy_bb_backup[0]);
			PHY_SetBBReg(priv, 0x93c, 0xff0000, priv->pshare->rf_phy_bb_backup[1]);
			PHY_SetBBReg(priv, 0x93c, bMaskByte3, priv->pshare->rf_phy_bb_backup[2]);
	}			
	return 1;
}

int MIMO_Mode_Switch(struct rtl8192cd_priv *priv, int mode)
{
	unsigned int retval = 0;	
#ifdef MP_TEST
	if (((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific))
		return 0;
#endif	

	switch(mode) {
		case MIMO_1T1R:
				retval = MIMO_Mode_1Plus1(priv);
				priv->pshare->rf_status= MIMO_1T1R;
				break;
		case MIMO_2T2R:
				retval = MIMO_Mode_2Plus2(priv);
				priv->pshare->rf_status= MIMO_2T2R;
				break;
		case MIMO_3T3R:
				retval = MIMO_Mode_3Plus3(priv);
				priv->pshare->rf_status= MIMO_3T3R;
				break;
		default:
			break;
	}
	return retval;
}

void set_MIMO_Mode(struct rtl8192cd_priv *priv, unsigned pref_mode)
{
	 if(MIMO_Mode_Switch(priv, pref_mode)) {
		if(pref_mode == MIMO_2T2R && get_rf_mimo_mode(priv) == MIMO_2T2R) {
		#if defined(WIFI_11N_2040_COEXIST_EXT)
			if((priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) && (priv->pmib->dot11nConfigEntry.dot11nUse40M != HT_CHANNEL_WIDTH_20)) {
				priv->pshare->CurrentChannelBW = priv->pmib->dot11nConfigEntry.dot11nUse40M;
				SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
				SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
		#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8723B_SUPPORT)
				update_RAMask_to_FW(priv, 1);
		#endif						
				SetTxPowerLevel(priv, priv->pmib->dot11RFEntry.dot11channel);
			}
		#endif		
		}
	 }else 
	 	panic_printk("(%s)line=%d, No such Mode. \n", __FUNCTION__, __LINE__);

}

#endif

#ifdef EN_EFUSE

#define VOLTAGE_V25		0x03

#if 1//defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8814AE)
#define MAX_2G_CHNLGRP	6
#define MAX_5G_CHNLGRP	14

struct channel_group {
	unsigned char low;
	unsigned char high;
};

static struct channel_group chnl_pwrlvlgp_2G[] = {
	//decrease 1 for array index
	{0, 1},
	{2, 4},
	{5, 7},
	{8, 10},
	{11, 12},
	{13, 13}
};

#if 0 //3 Old 
static struct channel_group chnl_pwrlvlgp_5G[] = {
	//decrease 1 for array index
	{35, 39},
	{43, 47},
	{51, 55},
	{59, 63},
	{99, 103},
	{107, 111},
	{115, 119},
	{123, 127},
	{131, 135},
	{139, 143},
	{148, 152},
	{156, 160},
	{164, 168},
	{172, 176}
};
#else //3 New
static struct channel_group chnl_pwrlvlgp_5G[] = {
	//decrease 1 for array index
	{35, 42}, //4 41-> ch 42 for 80M
	{43, 50},
	{51, 58}, //4 57-> ch 58 for 80M
	{59, 98},
	{99, 106}, //4 105-> ch 106 for 80M
	{107, 114},
	{115, 122}, //4 121-> ch 122 for 80M
	{123, 130},
	{131, 138},
	{139, 147},
	{148, 155}, //4 154-> ch 155 for 80M
	{156, 163},
	{164, 171}, //4 170-> ch 171 for 80M
	{172, 176}
};

#endif
int find_2gchnlgroup(int chnl)
{
	int index = 0;

	for (index = 0; index < MAX_2G_CHNLGRP; index++) {
		if ( (chnl >= chnl_pwrlvlgp_2G[index].low)
				&& (chnl <= chnl_pwrlvlgp_2G[index].high) )
			return index;
	}

	return -1;
}

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE) 
int find_5gchnlgroup(int chnl)
{
	int index = 0;

	for (index = 0; index < MAX_5G_CHNLGRP; index++) {
		if ( (chnl >= chnl_pwrlvlgp_5G[index].low)
				&&  (chnl <= chnl_pwrlvlgp_5G[index].high) )
			return index;
	}

	return -1;
}

void read_efusemap_2gtxpwrdiff(struct rtl8192cd_priv *priv, unsigned int chnl)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);

	if ((GET_CHIP_VER(priv) == VERSION_8812E) || 
		(GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) {
		//4 Path A
		priv->pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_A[chnl]
			= hwinfo[EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_A[chnl]
			= hwinfo[EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_OFDM2T_CCK2T_A[chnl]
			= hwinfo[EEPROM_2G_OFDM2T_TxPowerDiff + PATHA_OFFSET];
#if defined(CONFIG_WLAN_HAL_8814AE)	
		priv->pmib->dot11RFEntry.pwrdiff_40BW3S_20BW3S_A[chnl]
			= hwinfo[EEPROM_2G_HT403S_TxPowerDiff + PATHA_OFFSET];
#endif
		//4 Path B
		priv->pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_B[chnl]
			= hwinfo[EEPROM_2G_HT201S_TxPowerDiff + PATHB_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_B[chnl]
			= hwinfo[EEPROM_2G_HT402S_TxPowerDiff + PATHB_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_OFDM2T_CCK2T_B[chnl]
			= hwinfo[EEPROM_2G_OFDM2T_TxPowerDiff + PATHB_OFFSET];
#if defined(CONFIG_WLAN_HAL_8814AE)	
		priv->pmib->dot11RFEntry.pwrdiff_40BW3S_20BW3S_B[chnl]
			= hwinfo[EEPROM_2G_HT403S_TxPowerDiff + PATHB_OFFSET];
		//4 Path C
		priv->pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_C[chnl]
			= hwinfo[EEPROM_2G_HT201S_TxPowerDiff + PATHC_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_C[chnl]
			= hwinfo[EEPROM_2G_HT402S_TxPowerDiff + PATHC_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_OFDM2T_CCK2T_C[chnl]
			= hwinfo[EEPROM_2G_OFDM2T_TxPowerDiff + PATHC_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_40BW3S_20BW3S_C[chnl]
			= hwinfo[EEPROM_2G_HT403S_TxPowerDiff + PATHC_OFFSET];
		
		//4 Path D
		priv->pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_D[chnl]
			= hwinfo[EEPROM_2G_HT201S_TxPowerDiff + PATHD_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_D[chnl]
			= hwinfo[EEPROM_2G_HT402S_TxPowerDiff + PATHD_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_OFDM2T_CCK2T_D[chnl]
			= hwinfo[EEPROM_2G_OFDM2T_TxPowerDiff + PATHD_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_40BW3S_20BW3S_D[chnl]
			= hwinfo[EEPROM_2G_HT403S_TxPowerDiff + PATHD_OFFSET];
#endif
	}
}

void read_efusemap_5gtxpwrdiff(struct rtl8192cd_priv *priv, unsigned int chnl)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);

	if ((GET_CHIP_VER(priv) == VERSION_8812E) || 
		(GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) {
		//4 Path A
		priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_A[chnl]
			= hwinfo[EEPROM_5G_HT201S_TxPowerDiff + PATHA_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_A[chnl]
			= hwinfo[EEPROM_5G_HT402S_TxPowerDiff + PATHA_OFFSET];
#if defined(CONFIG_WLAN_HAL_8814AE)		
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW3S_20BW3S_A[chnl]
			= hwinfo[EEPROM_5G_HT403S_TxPowerDiff + PATHA_OFFSET];
#endif
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_A[chnl]
			= hwinfo[EEPROM_5G_HT801S_TxPowerDiff + PATHA_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_A[chnl]
			= hwinfo[EEPROM_5G_HT802S_TxPowerDiff + PATHA_OFFSET];
#if defined(CONFIG_WLAN_HAL_8814AE)				
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW3S_160BW3S_A[chnl]
			= hwinfo[EEPROM_5G_HT803S_TxPowerDiff + PATHA_OFFSET];
#endif
		
		//4 Path B
		priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_B[chnl]
			= hwinfo[EEPROM_5G_HT201S_TxPowerDiff + PATHB_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_B[chnl]
			= hwinfo[EEPROM_5G_HT402S_TxPowerDiff + PATHB_OFFSET];
#if defined(CONFIG_WLAN_HAL_8814AE)		
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW3S_20BW3S_B[chnl]
			= hwinfo[EEPROM_5G_HT403S_TxPowerDiff + PATHB_OFFSET];
#endif		
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_B[chnl]
			= hwinfo[EEPROM_5G_HT801S_TxPowerDiff + PATHB_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_B[chnl]
			= hwinfo[EEPROM_5G_HT802S_TxPowerDiff + PATHB_OFFSET];
#if defined(CONFIG_WLAN_HAL_8814AE)				
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW3S_160BW3S_B[chnl]
			= hwinfo[EEPROM_5G_HT803S_TxPowerDiff + PATHB_OFFSET];

		//4 Path C
		priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_C[chnl]
			= hwinfo[EEPROM_5G_HT201S_TxPowerDiff + PATHC_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_C[chnl]
			= hwinfo[EEPROM_5G_HT402S_TxPowerDiff + PATHC_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW3S_20BW3S_C[chnl]
			= hwinfo[EEPROM_5G_HT403S_TxPowerDiff + PATHC_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_C[chnl]
			= hwinfo[EEPROM_5G_HT801S_TxPowerDiff + PATHC_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_C[chnl]
			= hwinfo[EEPROM_5G_HT802S_TxPowerDiff + PATHC_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW3S_160BW3S_C[chnl]
			= hwinfo[EEPROM_5G_HT803S_TxPowerDiff + PATHC_OFFSET];

		//4 Path D
		priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_D[chnl]
			= hwinfo[EEPROM_5G_HT201S_TxPowerDiff + PATHD_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_D[chnl]
			= hwinfo[EEPROM_5G_HT402S_TxPowerDiff + PATHD_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW3S_20BW3S_D[chnl]
			= hwinfo[EEPROM_5G_HT403S_TxPowerDiff + PATHD_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_D[chnl]
			= hwinfo[EEPROM_5G_HT801S_TxPowerDiff + PATHD_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_D[chnl]
			= hwinfo[EEPROM_5G_HT802S_TxPowerDiff + PATHD_OFFSET];
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW3S_160BW3S_D[chnl]
			= hwinfo[EEPROM_5G_HT803S_TxPowerDiff + PATHD_OFFSET];
#endif
	}
}

int clear_5g_pwr_params(struct rtl8192cd_priv *priv, int index)
{
	if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8814A) {
		priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[index] = 0;
		priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_A[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_A[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_A[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_A[index] = 0;

		priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_B[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_B[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_B[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_B[index] = 0;

#if defined(CONFIG_WLAN_HAL_8814AE)	

		priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_C[index] = 0;
		priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_D[index] = 0;

		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW3S_20BW3S_A[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW3S_20BW3S_B[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW3S_160BW3S_A[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW3S_160BW3S_B[index] = 0;

		priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_C[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_C[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW3S_20BW3S_C[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_C[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_C[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW3S_160BW3S_C[index] = 0;

		priv->pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_D[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_D[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_40BW3S_20BW3S_D[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_D[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_D[index] = 0;
		priv->pmib->dot11RFEntry.pwrdiff_5G_80BW3S_160BW3S_D[index] = 0;

#endif
		
	}

	return 0;
}
#endif // CONFIG_RTL_8812_SUPPORT
#endif // CONFIG_RTL_8812_SUPPORT || CONFIG_WLAN_HAL_8192EE


/*-----------------------------------------------------------------------------
 * Function:	efuse_PowerSwitch
 *
 * Overview:	When we want to enable write operation, we should change to
 *				pwr on state. When we stop write, we should switch to 500k mode
 *				and disable LDO 2.5V.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/17/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static	void efuse_PowerSwitch(struct rtl8192cd_priv *priv, unsigned char bWrite, unsigned char PwrState)
{
	unsigned char tempval;
	unsigned short tmpV16;

	if (PwrState == TRUE) {
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
		if ((GET_CHIP_VER(priv) == VERSION_8812E) || 
			(GET_CHIP_VER(priv) == VERSION_8814A) || 
			(GET_CHIP_VER(priv) == VERSION_8822B)) 
		{
			tempval = 0x69;
			RTL_W8(REG_PG_PASSWD_8812, tempval);
		}
#endif
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B)
			RTL_W8(REG_EFUSE_ACCESS, EFUSE_ACCESS_ON);
#endif
#if defined(CONFIG_WLAN_HAL_8192EE)
		if( GET_CHIP_VER(priv) == VERSION_8192E)
			RTL_W8(REG_EFUSE_ACCESS, EFUSE_ACCESS_ON_8192E);
#endif


		// 1.2V Power: From VDDON with Power Cut(0x0000h[15]), defualt valid
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
		if ((GET_CHIP_VER(priv) != VERSION_8812E) && 
			(GET_CHIP_VER(priv) != VERSION_8814A) && 
			(GET_CHIP_VER(priv) != VERSION_8822B)) 
#endif
		{
			tmpV16 = RTL_R16(SYS_ISO_CTRL);
			if (!(tmpV16 & PWC_EV12V)) {
				RTL_W8(0x1c, RTL_R8(0x1c)& ~BIT(1));
				tmpV16 |= PWC_EV12V;
				RTL_W16(SYS_ISO_CTRL, tmpV16);
				RTL_W8(0x1c, RTL_R8(0x1c)|0x02);
			}
		}

		// Reset: 0x0000h[28], default valid
		tmpV16 = RTL_R16(REG_SYS_FUNC_EN);
		if (!(tmpV16 & FEN_ELDR)) {
			RTL_W8(0x1c, RTL_R8(0x1c)|0x02);
			tmpV16 |= FEN_ELDR;
			RTL_W16(REG_SYS_FUNC_EN, tmpV16);
			RTL_W8(0x1c, RTL_R8(0x1c)|0x02);
		}
		// Clock: Gated(0x0008h[5]) 8M(0x0008h[1]) clock from ANA, default valid
		tmpV16 = RTL_R16(SYS_CLKR);
		if ((!(tmpV16 & LOADER_CLK_EN)) || (!(tmpV16 & ANA8M))) {
			tmpV16 |= (LOADER_CLK_EN | ANA8M);
			RTL_W16(SYS_CLKR, tmpV16);
		}

		if (bWrite == TRUE) {
			// Enable LDO 2.5V before read/write action
			tempval = RTL_R8(EFUSE_TEST + 3);
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8814AE)
			if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8814A) {
				tempval &= ~(BIT3 | BIT4 | BIT5 | BIT6);
				tempval |= (VOLTAGE_V25 << 3);
				tempval |= BIT7;
				RTL_W8(EFUSE_TEST + 3, tempval);
			} else
#endif
			{
				tempval &= 0x0F;
				tempval |= (VOLTAGE_V25 << 4);
				RTL_W8(EFUSE_TEST + 3, (tempval | 0x80));
			}
		}
	} else {
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
		if ((GET_CHIP_VER(priv) == VERSION_8812E) || 
			(GET_CHIP_VER(priv) == VERSION_8814A) || 
			(GET_CHIP_VER(priv) == VERSION_8822B)) 
		{
			tempval = 0x0;
			RTL_W8(REG_PG_PASSWD_8812, tempval);
		}
#endif
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B)
			RTL_W8(REG_EFUSE_ACCESS, EFUSE_ACCESS_OFF);
#endif
#ifdef CONFIG_WLAN_HAL_8192EE
		if (GET_CHIP_VER(priv) == VERSION_8192E)
			RTL_W8(REG_EFUSE_ACCESS, EFUSE_ACCESS_OFF_8192E);
#endif


		if (bWrite == TRUE) {
			// Disable LDO 2.5V after read/write action
			tempval = RTL_R8(EFUSE_TEST + 3);
			RTL_W8(EFUSE_TEST + 3, (tempval & 0x7F));
		}
	}
}	/* efuse_PowerSwitch */


//
//	Description:
//		1. Execute E-Fuse read byte operation according as map offset and
//		    save to E-Fuse table.
//		2. Refered from SD1 Richard.
//
//	Assumption:
//		1. Boot from E-Fuse and successfully auto-load.
//		2. PASSIVE_LEVEL (USB interface)
//
//	Created by Roger, 2008.10.21.
//	2008/12/12 MH 	1. Reorganize code flow and reserve bytes. and add description.
//					2. Add efuse utilization collect.
//	2008/12/22 MH	Read Efuse must check if we write section 1 data again!!! Sec1
//					write addr must be after sec5.
//
static void ReadEFuse(struct rtl8192cd_priv *priv, unsigned short _offset, int _size_byte, unsigned char *pbuf)
{
	unsigned char  	efuseTbl[priv->EfuseMapLen];
	unsigned char  	rtemp8[1];
	unsigned short 	eFuse_Addr = 0;
	unsigned char  	offset, wren;
	unsigned short  i, j;
	unsigned short 	eFuseWord[priv->EfuseMaxSection][EFUSE_MAX_WORD_UNIT];
	unsigned short	efuse_utilized = 0;
	UINT8 bSupportExtendHdr = FALSE;

	if ((GET_CHIP_VER(priv) == VERSION_8192D) ||
		(GET_CHIP_VER(priv) == VERSION_8188E) ||
		(GET_CHIP_VER(priv) == VERSION_8192D) ||
		(GET_CHIP_VER(priv) == VERSION_8723B) ||
		(GET_CHIP_VER(priv) == VERSION_8812E) ||
		(GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B))
		bSupportExtendHdr = TRUE;
	//
	// Do NOT excess total size of EFuse table. Added by Roger, 2008.11.10.
	//
	if ((_offset + _size_byte) > priv->EfuseMapLen) {
		// total E-Fuse table is 128bytes
		DEBUG_INFO("ReadEFuse(): Invalid offset(%#x) with read bytes(%#x)!!\n", _offset, _size_byte);
		return;
	}

	// 0. Refresh efuse init map as all oxFF.
	for (i = 0; i < priv->EfuseMaxSection; i++)
		for (j = 0; j < EFUSE_MAX_WORD_UNIT; j++)
			eFuseWord[i][j] = 0xFFFF;

	//
	// 1. Read the first byte to check if efuse is empty!!!
	//
	//
	ReadEFuseByte(priv, eFuse_Addr, rtemp8);
	if (*rtemp8 != 0xFF) {
		efuse_utilized++;
		eFuse_Addr++;
	} else {
		//RTPRINT(FEEPROM, EFUSE_READ_ALL, ("EFUSE is empty efuse_Addr-%d efuse_data=%x\n", eFuse_Addr, *rtemp8));
		return;
	}

	//
	// 2. Read real efuse content. Filter PG header and every section data.
	//
	while ((*rtemp8 != 0xFF) && (eFuse_Addr < priv->EfuseRealContentLen)) {

		// Check PG header for section num.
		if (bSupportExtendHdr && ((*rtemp8 & 0x1F) == 0x0F)) {	//extended header
			unsigned char u1temp = ((*rtemp8 & 0xE0) >> 5);
			ReadEFuseByte(priv, eFuse_Addr, rtemp8);
			if ((*rtemp8 & 0x0F) == 0x0F) {
				eFuse_Addr++;
				ReadEFuseByte(priv, eFuse_Addr, rtemp8);
				if (*rtemp8 != 0xFF && (eFuse_Addr < priv->EfuseRealContentLen)) {
					eFuse_Addr++;
				}
				break;
			} else {
				offset = ((*rtemp8 & 0xF0) >> 1) | u1temp;
				wren = (*rtemp8 & 0x0F);
				eFuse_Addr++;
			}
		} 
		else
		{
			offset = ((*rtemp8 >> 4) & 0x0f);
			wren = (*rtemp8 & 0x0f);
		}

		if (offset < priv->EfuseMaxSection) {
			for (i = 0; i < EFUSE_MAX_WORD_UNIT; i++) {
				// Check word enable condition in the section
				if (!(wren & 0x01)) {
					ReadEFuseByte(priv, eFuse_Addr, rtemp8);
					DEBUG_INFO("ReadEFuse, Addr=%x, %02x\n", eFuse_Addr, *rtemp8);
					eFuse_Addr++;
					efuse_utilized++;
					eFuseWord[offset][i] = (*rtemp8 & 0xff);
					if (eFuse_Addr >= priv->EfuseRealContentLen)
						break;
					ReadEFuseByte(priv, eFuse_Addr, rtemp8);
					DEBUG_INFO("ReadEFuse, Addr=%x, %02x\n", eFuse_Addr, *rtemp8);
					eFuse_Addr++;
					efuse_utilized++;
					eFuseWord[offset][i] |= (((UINT16) * rtemp8 << 8) & 0xff00);
					if (eFuse_Addr >= priv->EfuseRealContentLen)
						break;
				}
				wren >>= 1;
			}
		}

		// Read next PG header
		ReadEFuseByte(priv, eFuse_Addr, rtemp8);
		if (*rtemp8 != 0xFF && (eFuse_Addr < priv->EfuseRealContentLen)) {
			efuse_utilized++;
			DEBUG_INFO("ReadEFuse, Addr=%x, %02x\n", eFuse_Addr, *rtemp8);
			eFuse_Addr++;
		}
	}

	//
	// 3. Collect 16 sections and 4 word unit into Efuse map.
	//
	for (i = 0; i < priv->EfuseMaxSection; i++) {
		for (j = 0; j < EFUSE_MAX_WORD_UNIT; j++) {
			efuseTbl[(i << 3) + (j << 1)] = (eFuseWord[i][j] & 0xff);
			efuseTbl[(i << 3) + (j << 1) + 1] = ((eFuseWord[i][j] >> 8) & 0xff);
		}
	}

	//
	// 4. Copy from Efuse map to output pointer memory!!!
	//
	for (i = 0; i < _size_byte; i++) {
		pbuf[i] = efuseTbl[_offset + i];
	}

	//
	// 5. Calculate Efuse utilization.
	//
	//efuse_usage = (unsigned char)((efuse_utilized*100)/priv->EfuseRealContentLen);
	priv->EfuseUsedBytes = efuse_utilized;

}


/*-----------------------------------------------------------------------------
 * Function:	efuse_ReadAllMap
 *
 * Overview:	Read All Efuse content
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/11/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static void efuse_ReadAllMap(struct rtl8192cd_priv *priv, unsigned char *Efuse)
{
	//
	// We must enable clock and LDO 2.5V otherwise, read all map will be fail!!!!
	//
	efuse_PowerSwitch(priv, FALSE, TRUE);
	ReadEFuse(priv, 0, priv->EfuseMapLen, Efuse);
	efuse_PowerSwitch(priv, FALSE, FALSE);
}


/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowMapUpdate
 *
 * Overview:	Transfer current EFUSE content to shadow init and modify map.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/13/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static void EFUSE_ShadowMapUpdate(struct rtl8192cd_priv *priv)
{
	/*
		if (priv->AutoloadFailFlag == TRUE )	{
			DEBUG_INFO("AutoloadFailFlag=TRUE");
			memset((&priv->EfuseMap[EFUSE_INIT_MAP][0]), 0xFF, 128);
		} else
	*/
	{
		efuse_ReadAllMap(priv, &priv->EfuseMap[EFUSE_INIT_MAP][0]);
	}

	memcpy(&priv->EfuseMap[EFUSE_MODIFY_MAP][0],
		   &priv->EfuseMap[EFUSE_INIT_MAP][0], priv->EfuseMapLen);

}

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
static void ReadTxPowerInfoFromHWPG_8188E(struct rtl8192cd_priv *priv)
{
	const int MAX_CHNL_GROUP_24G_88E = 6;	// ch1~2, ch3~5, ch6~8,ch9~11,ch12~13,CH 14 total six groups
	const int chnl_group[MAX_2G_CHANNEL_NUM] = {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5};
	
	unsigned int eeAddr = EEPROM_TX_PWR_INX_88E;
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
	
	u8 IndexCCK_Base[MAX_CHNL_GROUP_24G_88E];
	u8 IndexBW40_Base[MAX_CHNL_GROUP_24G_88E-1];
	//If only one tx, only BW20 and OFDM are used.
	u8 OFDM_Diff, BW20_Diff;
	u8 group, ch, index;

//	if (!isPGValueValid(priv, hwinfo))
//		return;

	if (/*priv->AutoloadFailFlag==FALSE &&*/ priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE) {
		for (group = 0 ; group < MAX_CHNL_GROUP_24G_88E; ++group) {
			IndexCCK_Base[group] = hwinfo[eeAddr++];
		}
		
		for (group = 0 ; group < MAX_CHNL_GROUP_24G_88E-1; ++group) {
			IndexBW40_Base[group] = hwinfo[eeAddr++];
		}
		
		BW20_Diff = (hwinfo[eeAddr] & 0xf0) >> 4;	// 4-bit field and MSB(bit3) is sign number
		OFDM_Diff = (hwinfo[eeAddr] & 0x0f);		// 4-bit field and MSB(bit3) is sign number
		eeAddr++;
		
		for (ch = 1; ch <= MAX_2G_CHANNEL_NUM; ++ch)
		{
			index = ch -1;
			group = chnl_group[index];

			priv->pmib->dot11RFEntry.pwrlevelCCK_A[index] = IndexCCK_Base[group];

			if (ch == 14)
				priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[index] = IndexBW40_Base[4];
			else
				priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[index] = IndexBW40_Base[group];
		}
		
		memset(priv->pmib->dot11RFEntry.pwrdiffHT20, BW20_Diff, sizeof(priv->pmib->dot11RFEntry.pwrdiffHT20));
		memset(priv->pmib->dot11RFEntry.pwrdiffOFDM, OFDM_Diff, sizeof(priv->pmib->dot11RFEntry.pwrdiffOFDM));
		memset(priv->pmib->dot11RFEntry.pwrlevelCCK_B, 0x00, sizeof(priv->pmib->dot11RFEntry.pwrlevelCCK_B));
		memset(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B, 0x00, sizeof(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B));
		memset(priv->pmib->dot11RFEntry.pwrdiffHT40_2S, 0x00, sizeof(priv->pmib->dot11RFEntry.pwrdiffHT40_2S));
		
		DEBUG_INFO("EFUSE Autoload success!\n");
	}
}
#endif // CONFIG_RTL_88E_SUPPORT

#ifdef CONFIG_WLAN_HAL_8192EE
static void ReadTxPowerInfoFromHWPG_8192E(struct rtl8192cd_priv *priv)
{
	int i, ch_gp;
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);

	if ( GET_CHIP_VER(priv)==VERSION_8192E ) 
	{
		for (i=0; i<MAX_2G_CHANNEL_NUM; i++) 
		{
			ch_gp = find_2gchnlgroup(i);

			if(ch_gp > -1) 
			{
				if (hwinfo[EEPROM_2G_CCK1T_TxPower+PATHA_OFFSET+ch_gp] <= 63 )
					priv->pmib->dot11RFEntry.pwrlevelCCK_A[i]= hwinfo[EEPROM_2G_CCK1T_TxPower+PATHA_OFFSET+ch_gp];
				else
					priv->pmib->dot11RFEntry.pwrlevelCCK_A[i]= 0;
				if (hwinfo[EEPROM_2G_CCK1T_TxPower+PATHB_OFFSET+ch_gp] <= 63 )
					priv->pmib->dot11RFEntry.pwrlevelCCK_B[i]= hwinfo[EEPROM_2G_CCK1T_TxPower+PATHB_OFFSET+ch_gp];
				else
					priv->pmib->dot11RFEntry.pwrlevelCCK_B[i]= 0;
	
				if (i==13)//ch 14
					ch_gp -= 1;
				
				if (hwinfo[EEPROM_2G_HT401S_TxPower+PATHA_OFFSET+ch_gp] <= 63 )
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = hwinfo[EEPROM_2G_HT401S_TxPower+PATHA_OFFSET+ch_gp];
				else
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = 0;
				if (hwinfo[EEPROM_2G_HT401S_TxPower+PATHB_OFFSET+ch_gp] <= 63 )
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = hwinfo[EEPROM_2G_HT401S_TxPower+PATHB_OFFSET+ch_gp];
				else
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = 0;
			}
	
			if (priv->pmib->dot11RFEntry.pwrlevelCCK_A[0] != 0)
			{
				priv->pmib->dot11RFEntry.pwrdiffHT40_2S[i] = ((hwinfo[EEPROM_2G_HT402S_TxPowerDiff+PATHA_OFFSET] & 0xf0) >> 4) |
					(hwinfo[EEPROM_2G_HT402S_TxPowerDiff+PATHB_OFFSET] & 0xf0);
				priv->pmib->dot11RFEntry.pwrdiffHT20[i] = ((hwinfo[EEPROM_2G_HT201S_TxPowerDiff+PATHA_OFFSET] & 0xf0) >> 4) |
					(hwinfo[EEPROM_2G_HT201S_TxPowerDiff+PATHB_OFFSET] & 0xf0);
				priv->pmib->dot11RFEntry.pwrdiffOFDM[i] = (hwinfo[EEPROM_2G_OFDM1T_TxPowerDiff+PATHA_OFFSET] & 0x0f) |
					((hwinfo[EEPROM_2G_OFDM1T_TxPowerDiff+PATHB_OFFSET] & 0x0f) << 4);
			}
			else
			{
				priv->pmib->dot11RFEntry.pwrdiffHT40_2S[i] = 0;
				priv->pmib->dot11RFEntry.pwrdiffHT20[i] = 0;
				priv->pmib->dot11RFEntry.pwrdiffOFDM[i] = 0;
			}
		}
	}

#if 0
	const int MAX_CHNL_GROUP_24G_92E = 6;	// ch1~2, ch3~5, ch6~8,ch9~11,ch12~13,CH 14 total six groups
	const int chnl_group[MAX_2G_CHANNEL_NUM] = {0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5};
	
	unsigned int eeAddr_A = EEPROM_TX_PWR_INX_8192E+PATHA_OFFSET, eeAddr_B = EEPROM_TX_PWR_INX_8192E+PATHB_OFFSET;
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
	
	u8 IndexCCK_Base_A[MAX_CHNL_GROUP_24G_92E], IndexCCK_Base_B[MAX_CHNL_GROUP_24G_92E];
	u8 IndexBW40_Base_A[MAX_CHNL_GROUP_24G_92E-1], IndexBW40_Base_B[MAX_CHNL_GROUP_24G_92E-1];
	//If only one tx, only BW20 and OFDM are used.
	u8 OFDM_Diff, BW20_Diff, BW40_2S_DIFF, BW20_2S_DIFF;
	u8 group, ch, index;

//	if (!isPGValueValid(priv, hwinfo))
//		return;

	if (/*priv->AutoloadFailFlag==FALSE &&*/ priv->pmib->efuseEntry.enable_efuse==1) {
		for (group = 0 ; group < MAX_CHNL_GROUP_24G_92E; ++group) {
			IndexCCK_Base_A[group] = hwinfo[eeAddr_A++];
			IndexCCK_Base_B[group] = hwinfo[eeAddr_B++];
		}
		
		for (group = 0 ; group < MAX_CHNL_GROUP_24G_92E-1; ++group) {
			IndexBW40_Base_A[group] = hwinfo[eeAddr_A++];
			IndexBW40_Base_B[group] = hwinfo[eeAddr_B++];
		}
		
		BW20_Diff = (hwinfo[eeAddr_A] & 0xf0) >> 4;	// 4-bit field and MSB(bit3) is sign number
		OFDM_Diff = (hwinfo[eeAddr_A] & 0x0f);		// 4-bit field and MSB(bit3) is sign number

		eeAddr_A++;

		BW40_2S_DIFF = (hwinfo[eeAddr_A] & 0xf0) >> 4;	// 4-bit field and MSB(bit3) is sign number
		BW20_2S_DIFF = (hwinfo[eeAddr_A] & 0x0f);		// 4-bit field and MSB(bit3) is sign number
		
		for (ch = 1; ch <= MAX_2G_CHANNEL_NUM; ++ch)
		{
			index = ch -1;
			group = chnl_group[index];

			priv->pmib->dot11RFEntry.pwrlevelCCK_A[index] = IndexCCK_Base_A[group];
			priv->pmib->dot11RFEntry.pwrlevelCCK_B[index] = IndexCCK_Base_B[group];

			if (ch == 14){
				priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[index] = IndexBW40_Base_A[4];
				priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[index] = IndexBW40_Base_B[4];
			}
			else{
				priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[index] = IndexBW40_Base_A[group];
				priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[index] = IndexBW40_Base_B[group];
			}
		}
		
		memset(priv->pmib->dot11RFEntry.pwrdiffHT20, BW20_Diff, sizeof(priv->pmib->dot11RFEntry.pwrdiffHT20));
		memset(priv->pmib->dot11RFEntry.pwrdiffOFDM, OFDM_Diff, sizeof(priv->pmib->dot11RFEntry.pwrdiffOFDM));
		memset(priv->pmib->dot11RFEntry.pwrdiffHT40_2S, BW40_2S_DIFF, sizeof(priv->pmib->dot11RFEntry.pwrdiffHT40_2S));
			
		DEBUG_INFO("EFUSE Autoload success!\n");
	}
#endif
}
#endif // CONFIG_WLAN_HAL_8192EE


#if defined(CONFIG_WLAN_HAL_8814AE)
static void ReadTxPowerInfoFromHWPG_8814(struct rtl8192cd_priv *priv)
{
	int i, ch_gp;
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
	for (i = 0; i < MAX_2G_CHANNEL_NUM; i++) {
		ch_gp = -1;
		ch_gp = find_2gchnlgroup(i);

		if (ch_gp > -1) {
			priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] = HAL_EFUSE_POWER_RANGE_CHECK(hwinfo[EEPROM_2G_CCK1T_TxPower + PATHA_OFFSET + ch_gp]);												
			priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] = HAL_EFUSE_POWER_RANGE_CHECK(hwinfo[EEPROM_2G_CCK1T_TxPower + PATHB_OFFSET + ch_gp]);						
			priv->pmib->dot11RFEntry.pwrlevelCCK_C[i] = HAL_EFUSE_POWER_RANGE_CHECK(hwinfo[EEPROM_2G_CCK1T_TxPower + PATHC_OFFSET + ch_gp]);												
			priv->pmib->dot11RFEntry.pwrlevelCCK_D[i] = HAL_EFUSE_POWER_RANGE_CHECK(hwinfo[EEPROM_2G_CCK1T_TxPower + PATHD_OFFSET + ch_gp]);

			if(ch_gp == 5)
				ch_gp = 4;
			priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = HAL_EFUSE_POWER_RANGE_CHECK(hwinfo[EEPROM_2G_HT401S_TxPower + PATHA_OFFSET + ch_gp]);
			priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = HAL_EFUSE_POWER_RANGE_CHECK(hwinfo[EEPROM_2G_HT401S_TxPower + PATHB_OFFSET + ch_gp]);
			priv->pmib->dot11RFEntry.pwrlevelHT40_1S_C[i] = HAL_EFUSE_POWER_RANGE_CHECK(hwinfo[EEPROM_2G_HT401S_TxPower + PATHC_OFFSET + ch_gp]);
			priv->pmib->dot11RFEntry.pwrlevelHT40_1S_D[i] = HAL_EFUSE_POWER_RANGE_CHECK(hwinfo[EEPROM_2G_HT401S_TxPower + PATHD_OFFSET + ch_gp]);
			read_efusemap_2gtxpwrdiff(priv, i);
		}
	}

	for (i = 0; i < MAX_5G_CHANNEL_NUM; i++) {
		int ch_gp = -1;
		ch_gp = find_5gchnlgroup(i);
		if (ch_gp > -1) {
			priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = HAL_EFUSE_POWER_RANGE_CHECK(hwinfo[EEPROM_5G_HT401S_TxPower + PATHA_OFFSET + ch_gp]);
			priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = HAL_EFUSE_POWER_RANGE_CHECK(hwinfo[EEPROM_5G_HT401S_TxPower + PATHB_OFFSET + ch_gp]);					
			priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_C[i] = HAL_EFUSE_POWER_RANGE_CHECK(hwinfo[EEPROM_5G_HT401S_TxPower + PATHC_OFFSET + ch_gp]);
			priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_D[i] = HAL_EFUSE_POWER_RANGE_CHECK(hwinfo[EEPROM_5G_HT401S_TxPower + PATHD_OFFSET + ch_gp]);		
			read_efusemap_5gtxpwrdiff(priv, i);
		} else {
			clear_5g_pwr_params(priv, i);
		}			
	} 
}
#endif //#if defined(CONFIG_WLAN_HAL_8814AE)



static int isPGValueValid(struct rtl8192cd_priv *priv, unsigned char *hwinfo)
{
	int j=0;
#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
		for (j = EEPROM_TxPowerCCK; j < EEPROM_TxPowerCCK + 3; j++) {
			if (hwinfo[j] > 63)
				return 0;
		}
		for (j = EEPROM_TxPowerHT40_1S; j < EEPROM_TxPowerHT40_1S + 3; j++) {
			if (hwinfo[j] > 63)
				return 0;
		}
	}
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		for (j = EEPROM_2G_TxPowerCCK; j < EEPROM_2G_TxPowerCCK + 3; j++) {
			if (hwinfo[j] > 63)
				return 0;
		}
		for (j = EEPROM_2G_TxPowerHT40_1S; j < EEPROM_2G_TxPowerHT40_1S + 3; j++) {
			if (hwinfo[j] > 63)
				return 0;
		}
		for (j = EEPROM_5GL_TxPowerHT40_1S; j < EEPROM_5GL_TxPowerHT40_1S + 3; j++) {
			if (hwinfo[j] > 63)
				return 0;
		}
		for (j = EEPROM_5GM_TxPowerHT40_1S; j < EEPROM_5GM_TxPowerHT40_1S + 3; j++) {
			if (hwinfo[j] > 63)
				return 0;
		}
		for (j = EEPROM_5GH_TxPowerHT40_1S; j < EEPROM_5GH_TxPowerHT40_1S + 3; j++) {
			if (hwinfo[j] > 63)
				return 0;
		}
	}
#endif // CONFIG_RTL_92D_SUPPORT

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		for (j = EEPROM_TX_PWR_INX_88E; j < EEPROM_TX_PWR_INX_88E+11; j++) {
			if (hwinfo[j] > 63)
				return 0;
		}
	}
#endif // CONFIG_RTL_88E_SUPPORT
#if 0	
#ifdef CONFIG_WLAN_HAL_8192EE
		if (GET_CHIP_VER(priv) == VERSION_8192E) {
			for (j = EEPROM_TX_PWR_INX_8192E+PATHA_OFFSET; j < EEPROM_TX_PWR_INX_8192E+PATHA_OFFSET+11; j++) { // path A CCK and HT40-1S
				if (hwinfo[j] > 63){
					return 0;
				}
			}
			for (j = EEPROM_TX_PWR_INX_8192E+PATHB_OFFSET; j < EEPROM_TX_PWR_INX_8192E+PATHB_OFFSET+11; j++) { // path B CCK and HT40-1S
				if (hwinfo[j] > 63)
					return 0;
			}
		}
#endif // CONFIG_WLAN_HAL_8192EE
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8822BE)
	if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8822B)) {
		/* Path A CCK Power Index */
		for ( j = EEPROM_2G_CCK1T_TxPower + PATHA_OFFSET ; j < EEPROM_2G_CCK1T_TxPower + PATHA_OFFSET + MAX_2G_CHNLGRP ; ++j )
			if (hwinfo[j] > 0x3f)
				return 0;

		/* Path B CCK Power Index */
		for ( j = EEPROM_2G_CCK1T_TxPower + PATHB_OFFSET ; j < EEPROM_2G_CCK1T_TxPower + PATHB_OFFSET + MAX_2G_CHNLGRP ; ++j )
			if (hwinfo[j] > 0x3f)
				return 0;

		/* Path A 2G BW40-1S Power Index */
		for ( j = EEPROM_2G_HT401S_TxPower + PATHA_OFFSET ; j < EEPROM_2G_HT401S_TxPower + PATHA_OFFSET + MAX_2G_CHNLGRP - 1 ; ++j )
			if (hwinfo[j] > 0x3f)
				return 0;

		/* Path B 2G BW40-1S Power Index */
		for ( j = EEPROM_2G_HT401S_TxPower + PATHB_OFFSET ; j < EEPROM_2G_HT401S_TxPower + PATHB_OFFSET + MAX_2G_CHNLGRP - 1 ; ++j )
			if (hwinfo[j] > 0x3f)
				return 0;

		/* Path A 5G BW40-1S Power Index */
		for ( j = EEPROM_5G_HT401S_TxPower + PATHA_OFFSET ; j < EEPROM_5G_HT401S_TxPower + PATHA_OFFSET + MAX_5G_CHNLGRP ; ++j )
			if (hwinfo[j] > 0x3f)
				return 0;
		
		/* Path B 5G BW40-1S Power Index */
		for ( j = EEPROM_5G_HT401S_TxPower + PATHB_OFFSET ; j < EEPROM_5G_HT401S_TxPower + PATHB_OFFSET + MAX_5G_CHNLGRP ; ++j )
			if (hwinfo[j] > 0x3f)
				return 0;
	}
#endif
	return 1;
}


void ReadTxPowerInfoFromHWPG(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
	int i;

	if(!(priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE)|| !(priv->pmib->efuseEntry.enable_efuse & EFUSE_POWER))
		return;

	if (!isPGValueValid(priv, hwinfo))
		return;

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
		ReadTxPowerInfoFromHWPG_8188E(priv);
		return;
	}
#endif
#ifdef CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E) {
		ReadTxPowerInfoFromHWPG_8192E(priv);
		return;
	}
#endif

	if (/*priv->AutoloadFailFlag==FALSE &&*/ priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE) {
		
#if defined(CONFIG_WLAN_HAL_8814AE)
		if ( GET_CHIP_VER(priv) == VERSION_8814A ) {
			ReadTxPowerInfoFromHWPG_8814(priv);
		} else
#endif //#if defined(CONFIG_WLAN_HAL_8814AE)
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8822BE)
		if ((GET_CHIP_VER(priv) == VERSION_8812E) ||  (GET_CHIP_VER(priv) == VERSION_8822B)) {
			for (i = 0; i < MAX_2G_CHANNEL_NUM; i++) {
				int ch_gp = -1;
				ch_gp = find_2gchnlgroup(i);

				if (ch_gp > -1) {
					priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] = hwinfo[EEPROM_2G_CCK1T_TxPower + PATHA_OFFSET + ch_gp];
					priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] = hwinfo[EEPROM_2G_CCK1T_TxPower + PATHB_OFFSET + ch_gp];

					if (i == 13)
						ch_gp -= 1;
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = hwinfo[EEPROM_2G_HT401S_TxPower + PATHA_OFFSET + ch_gp];
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = hwinfo[EEPROM_2G_HT401S_TxPower + PATHB_OFFSET + ch_gp];

					read_efusemap_2gtxpwrdiff(priv, i);
				}
			}

			for (i = 0; i < MAX_5G_CHANNEL_NUM; i++) {
				int ch_gp = -1;
				ch_gp = find_5gchnlgroup(i);

				if (ch_gp > -1) {
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5G_HT401S_TxPower + PATHA_OFFSET + ch_gp];
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5G_HT401S_TxPower + PATHB_OFFSET + ch_gp];

					read_efusemap_5gtxpwrdiff(priv, i);
				} else {
					clear_5g_pwr_params(priv, i);
				}
			}
		} else
#endif //#if defined(CONFIG_RTL_8812_SUPPORT)
		{
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
			u8 TxPwrCCK = 0, TxPwrHT40_1S = 0, TxPwrHT40_2SDiff = 0, TxPwrHT20Diff = 0, TxPwrOFDMDiff = 0;
#ifdef CONFIG_RTL_92C_SUPPORT
			if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
				TxPwrCCK = EEPROM_TxPowerCCK;
				TxPwrHT40_1S = EEPROM_TxPowerHT40_1S;
				TxPwrHT40_2SDiff = EEPROM_TxPowerHT40_2SDiff;
				TxPwrHT20Diff = EEPROM_TxPowerHT20Diff;
				TxPwrOFDMDiff = EEPROM_TxPowerOFDMDiff;
			}
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8192D) {
				// 2.4G Setting
				TxPwrCCK = EEPROM_2G_TxPowerCCK;
				TxPwrHT40_1S = EEPROM_2G_TxPowerHT40_1S;
				TxPwrHT40_2SDiff = EEPROM_2G_TxPowerHT40_2SDiff;
				TxPwrHT20Diff = EEPROM_2G_TxPowerHT20Diff;
				TxPwrOFDMDiff = EEPROM_2G_TxPowerOFDMDiff;

				// 5G Setting
				for (i = 0; i < MAX_5G_CHANNEL_NUM; i++) {
					if (i >= 35 && i <= 63) { // ch 36 ~ 64
						if (i >= 35 && i <= 43) { // ch 36 ~ 44
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GL_TxPowerHT40_1S];
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GL_TxPowerHT40_1S + 3];

							priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i] = hwinfo[EEPROM_5GL_TxPowerHT40_2SDiff];
							priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GL_TxPowerHT20Diff];
							priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GL_TxPowerOFDMDiff];
						} else if (i >= 45 && i <= 53) { // ch 46 ~ 54
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GL_TxPowerHT40_1S + 1];
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GL_TxPowerHT40_1S + 4];

							priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i] = hwinfo[EEPROM_5GL_TxPowerHT40_2SDiff + 1];
							priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GL_TxPowerHT20Diff + 1];
							priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GL_TxPowerOFDMDiff + 1];
						} else if (i >= 55 && i <= 63) { // ch 56 ~ 64
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GL_TxPowerHT40_1S + 2];
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GL_TxPowerHT40_1S + 5];

							priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i] = hwinfo[EEPROM_5GL_TxPowerHT40_2SDiff + 2];
							priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GL_TxPowerHT20Diff + 2];
							priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GL_TxPowerOFDMDiff + 2];
						} else {
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = 0;
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = 0;

							priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i] = 0;
							priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = 0;
							priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = 0;
						}
					} else if (i >= 99 && i <= 139) { // ch 100 ~ 140
						if (i >= 99 && i <= 111) { // ch 100 ~ 112
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GM_TxPowerHT40_1S];
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GM_TxPowerHT40_1S + 3];

							priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i] = hwinfo[EEPROM_5GM_TxPowerHT40_2SDiff];
							priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GM_TxPowerHT20Diff];
							priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GM_TxPowerOFDMDiff];
						} else if (i >= 113 && i <= 125) { // ch 114 ~ 126
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GM_TxPowerHT40_1S + 1];
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GM_TxPowerHT40_1S + 4];

							priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i] = hwinfo[EEPROM_5GM_TxPowerHT40_2SDiff + 1];
							priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GM_TxPowerHT20Diff + 1];
							priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GM_TxPowerOFDMDiff + 1];
						} else if (i >= 127 && i <= 139) { // ch 128 ~ 140
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GM_TxPowerHT40_1S + 2];
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GM_TxPowerHT40_1S + 5];

							priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i] = hwinfo[EEPROM_5GM_TxPowerHT40_2SDiff + 2];
							priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GM_TxPowerHT20Diff + 2];
							priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GM_TxPowerOFDMDiff + 2];
						} else {
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = 0;
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = 0;

							priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i] = 0;
							priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = 0;
							priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = 0;
						}
					} else if (i >= 148 && i <= 164 ) { // ch 149 ~ 165
						if (i >= 148 && i <= 152) { // ch 149 ~ 153
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GH_TxPowerHT40_1S];
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GH_TxPowerHT40_1S + 3];

							priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i] = hwinfo[EEPROM_5GH_TxPowerHT40_2SDiff];
							priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GH_TxPowerHT20Diff];
							priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GH_TxPowerOFDMDiff];
						} else if (i >= 154 && i <= 158) { // ch 155 ~ 159
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GH_TxPowerHT40_1S + 1];
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GH_TxPowerHT40_1S + 4];

							priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i] = hwinfo[EEPROM_5GH_TxPowerHT40_2SDiff + 1];
							priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GH_TxPowerHT20Diff + 1];
							priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GH_TxPowerOFDMDiff + 1];
						} else if (i >= 160 && i <= 164) { // ch 161 ~ 165
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = hwinfo[EEPROM_5GH_TxPowerHT40_1S + 2];
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = hwinfo[EEPROM_5GH_TxPowerHT40_1S + 5];

							priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i] = hwinfo[EEPROM_5GH_TxPowerHT40_2SDiff + 2];
							priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = hwinfo[EEPROM_5GH_TxPowerHT20Diff + 2];
							priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = hwinfo[EEPROM_5GH_TxPowerOFDMDiff + 2];
						} else {
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = 0;
							priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = 0;

							priv->pmib->dot11RFEntry.pwrdiff5GHT40_2S[i] = 0;
							priv->pmib->dot11RFEntry.pwrdiff5GHT20[i] = 0;
							priv->pmib->dot11RFEntry.pwrdiff5GOFDM[i] = 0;
						}
					}
				}
			}
#endif

			for (i = 0; i < MAX_2G_CHANNEL_NUM; i++) {
				if (i < 3) {
					priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] = hwinfo[TxPwrCCK];
					priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] = hwinfo[TxPwrCCK + 3];

					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = hwinfo[TxPwrHT40_1S];
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = hwinfo[TxPwrHT40_1S + 3];

					priv->pmib->dot11RFEntry.pwrdiffHT40_2S[i] = hwinfo[TxPwrHT40_2SDiff];
					priv->pmib->dot11RFEntry.pwrdiffHT20[i] = hwinfo[TxPwrHT20Diff];
					priv->pmib->dot11RFEntry.pwrdiffOFDM[i] = hwinfo[TxPwrOFDMDiff];

				} else if (i < 9) {
					priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] = hwinfo[TxPwrCCK + 1];
					priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] = hwinfo[TxPwrCCK + 4];

					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = hwinfo[TxPwrHT40_1S + 1];
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = hwinfo[TxPwrHT40_1S + 4];

					priv->pmib->dot11RFEntry.pwrdiffHT40_2S[i] = hwinfo[TxPwrHT40_2SDiff + 1];
					priv->pmib->dot11RFEntry.pwrdiffHT20[i] = hwinfo[TxPwrHT20Diff + 1];
					priv->pmib->dot11RFEntry.pwrdiffOFDM[i] = hwinfo[TxPwrOFDMDiff + 1];
				} else {
					priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] = hwinfo[TxPwrCCK + 2];
					priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] = hwinfo[TxPwrCCK + 5];

					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = hwinfo[TxPwrHT40_1S + 2];
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = hwinfo[TxPwrHT40_1S + 5];

					priv->pmib->dot11RFEntry.pwrdiffHT40_2S[i] = hwinfo[TxPwrHT40_2SDiff + 2];
					priv->pmib->dot11RFEntry.pwrdiffHT20[i] = hwinfo[TxPwrHT20Diff + 2];
					priv->pmib->dot11RFEntry.pwrdiffOFDM[i] = hwinfo[TxPwrOFDMDiff + 2];

				}
			}
#endif // CONFIG_RTL_92C_SUPPORT || CONFIG_RTL_92D_SUPPORT
		}
		DEBUG_INFO("EFUSE Autoload success!\n");
	}
}


void ReadMacAddressFromEfuse(struct rtl8192cd_priv *priv)
{
	u16 efuse_MAC=0; //sdio
#ifdef __KERNEL__
	struct sockaddr addr;
#endif

	if(!(priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE)|| !(priv->pmib->efuseEntry.enable_efuse & EFUSE_MACADDR))
		return;

#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
		efuse_MAC = EEPROM_MACADDRESS;
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
#ifdef CONFIG_RTL_92D_DMDP
		if (priv->pshare->wlandev_idx == 0) {
			efuse_MAC = EEPROM_MAC0_MACADDRESS;
		} else {
			efuse_MAC = EEPROM_MAC1_MACADDRESS;
		}
#else
		efuse_MAC = EEPROM_MAC0_MACADDRESS;
#endif
	}
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B)
		efuse_MAC = EEPROM_MAC_ADDR_88E;
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E)
		efuse_MAC = EEPROM_92E_MACADDRESS;
#endif
	
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
	if ((GET_CHIP_VER(priv) == VERSION_8812E) ||
		(GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) 
	{
		efuse_MAC = EEPROM_8812_MACADDRESS;
	}
#endif

	if (/*priv->AutoloadFailFlag==FALSE &&*/ priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE) {
		unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
		unsigned char *efuse_mac = hwinfo + efuse_MAC;
		unsigned char zero[] = {0, 0, 0, 0, 0, 0}, mac[6];

		memcpy(mac, efuse_mac, MACADDRLEN);
		/*panic_printk("wlan%d EFUSE MAC [%02x:%02x:%02x:%02x:%02x:%02x]\n", priv->pshare->wlandev_idx,
				*mac, *(mac+1), *(mac+2), *(mac+3), *(mac+4), *(mac+5)); */
#if 0
		if (memcmp(mac, zero, MACADDRLEN) && !IS_MCAST(mac)) {
#ifdef __KERNEL__
			memcpy(addr.sa_data, mac, MACADDRLEN);
			rtl8192cd_set_hwaddr(priv->dev, (void *)&addr);
#else
			rtl8192cd_set_hwaddr(priv->dev, (void *)mac);
#endif
		}
#else
		if (!memcmp(mac, zero, MACADDRLEN) || IS_MCAST(mac)) {
			memcpy(mac, zero, MACADDRLEN);
		}

#ifdef __KERNEL__
		memcpy(addr.sa_data, mac, MACADDRLEN);
		rtl8192cd_set_hwaddr(priv->dev, (void *)&addr);
#else
		rtl8192cd_set_hwaddr(priv->dev, (void *)mac);
#endif
#endif
	}
}

#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
int get_mac_addr_from_efuse(struct rtl8192cd_priv *priv, char *mac)
{
	u16 efuse_MAC=0; //sdio

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E)
		efuse_MAC = EEPROM_MAC_ADDR_88E;
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E)
		efuse_MAC = EEPROM_92E_MACADDRESS;
#endif

	if (/*priv->AutoloadFailFlag==FALSE &&*/ priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE) {
		unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
		unsigned char *efuse_mac = hwinfo + efuse_MAC;

		memcpy(mac, efuse_mac, MACADDRLEN);
		/* printk("wlan%d EFUSE MAC [%02x:%02x:%02x:%02x:%02x:%02x]\n", priv->pshare->wlandev_idx,
				*mac, *(mac+1), *(mac+2), *(mac+3), *(mac+4), *(mac+5)); */
		return 0;
	}
	else {
		memset((void *)mac, 0, 6);
		return -1;
	}
}
#endif // __ECOS && CONFIG_SDIO_HCI

void ReadThermalMeterFromEfuse(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
	u8 ther_offset = 0, TherMask = 0x1f;
	u8 ther_lower_bound = 0x7, ther_upper_bound = 0x32;
	u8 efuse_ther;

	if (!(priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE) || !(priv->pmib->efuseEntry.enable_efuse & EFUSE_THERMAL))
		return;

#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
		ther_offset = EEPROM_THERMAL_METER;
		ther_upper_bound = 0x1d;
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		ther_offset = EEPROM_92D_THERMAL_METER;
		ther_upper_bound = 0x1d;
	}
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
		ther_offset = EEPROM_THERMAL_METER_88E;
		TherMask = 0xff;
	}
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E) {
		ther_offset = EEPROM_92E_THERMAL_METER;
		TherMask = 0xff;
	}
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
	if ((GET_CHIP_VER(priv) == VERSION_8812E) ||
		(GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) 
	{
		ther_offset = EEPROM_8812_THERMAL_METER;
			TherMask = 0xff;
		}
#endif

	efuse_ther = (hwinfo[ther_offset] & TherMask);

	if ((ther_lower_bound < efuse_ther) && (efuse_ther < ther_upper_bound))
	{
		priv->pmib->dot11RFEntry.ther = efuse_ther;
		DEBUG_INFO("ThermalMeter = 0x%x\n", efuse_ther);
	}
	else
	{
		priv->pmib->dot11RFEntry.ther = 0;
		DEBUG_ERR("TPT: unreasonable target ther %d, disable tpt\n", efuse_ther);
	}
}


#ifdef CONFIG_LNA_FROM_EFUSE
void ReadLNATypeFromEfuse(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
	u8 efuse_LNAType = 0, LNAMask = 0x70;
    
	if(!(priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE)|| !(priv->pmib->efuseEntry.enable_efuse & EFUSE_LNATYPE))
		return;

    efuse_LNAType = EEPROM_RFE_OPTION_8192E;
	priv->pshare->rf_ft_var.lna_type= (hwinfo[efuse_LNAType] & LNAMask)>>4;

    if (priv->pshare->rf_ft_var.lna_type >= LNA_TYPE_MAX) {
        printk("===============Invalid LNA Type of eFuse=================\n");  
		priv->pshare->rf_ft_var.lna_type=LNA_TYPE_0;
	}
}
#endif


#ifdef CONFIG_USB_HCI
static void ReadBoardTypeFromEfuse(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
	u8 efuse_boardType;

	if (!(priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE))
		return;

#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv)== VERSION_8188C) || (GET_CHIP_VER(priv)== VERSION_8192C)) {
		efuse_boardType = EEPROM_NORMAL_BoardType;
	}
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
		efuse_boardType = EEPROM_RF_BOARD_OPTION_88E;
	}
#endif
	
	priv->pshare->BoardType = (hwinfo[efuse_boardType] & BOARD_TYPE_NORMAL_MASK) >> 5;
	DEBUG_INFO("BoardType = %d\n", priv->pshare->BoardType);
}

static void ReadIDFromEfuse(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);

	if (FALSE == priv->AutoloadFailFlag) {
#ifdef CONFIG_RTL_92C_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192C) {
			// VID, PID
			priv->pshare->EEPROMVID = le16_to_cpu( *(u16 *)&hwinfo[EEPROM_VID]);
			priv->pshare->EEPROMPID = le16_to_cpu( *(u16 *)&hwinfo[EEPROM_PID]);

			// Customer ID, 0x00 and 0xff are reserved for Realtek.
			priv->pshare->EEPROMCustomerID = hwinfo[EEPROM_CUSTOMER_ID];
			priv->pshare->EEPROMSubCustomerID = hwinfo[EEPROM_SUBCUSTOMER_ID];
		}
#endif
		
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
			// VID, PID
			priv->pshare->EEPROMVID = EF2Byte( *(u16 *)&hwinfo[EEPROM_VID_88EU] );
			priv->pshare->EEPROMPID = EF2Byte( *(u16 *)&hwinfo[EEPROM_PID_88EU] );

			// Customer ID, 0x00 and 0xff are reserved for Realtek.
			priv->pshare->EEPROMCustomerID = hwinfo[EEPROM_CUSTOMERID_88E];
			priv->pshare->EEPROMSubCustomerID = EEPROM_Default_SubCustomerID;
		}
#endif
	} else{
		priv->pshare->EEPROMVID = EEPROM_Default_VID;
		priv->pshare->EEPROMPID = EEPROM_Default_PID;

		// Customer ID, 0x00 and 0xff are reserved for Realtek.
		priv->pshare->EEPROMCustomerID = EEPROM_Default_CustomerID;
		priv->pshare->EEPROMSubCustomerID = EEPROM_Default_SubCustomerID;
	}
	
	DEBUG_INFO("EEPROMVID = 0x%04x\n", priv->pshare->EEPROMVID);
	DEBUG_INFO("EEPROMPID = 0x%04x\n", priv->pshare->EEPROMPID);
	DEBUG_INFO("EEPROMCustomerID : 0x%02x\n", priv->pshare->EEPROMCustomerID);
	DEBUG_INFO("EEPROMSubCustomerID: 0x%02x\n", priv->pshare->EEPROMSubCustomerID);
}
#endif // CONFIG_USB_HCI

void ReadCrystalCalibrationFromEfuse(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);

	if(!(priv->pmib->efuseEntry.enable_efuse & EFUSE_THERMAL)|| !(priv->pmib->efuseEntry.enable_efuse & EFUSE_CRYSTAL))
		return;
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
	if ((GET_CHIP_VER(priv) == VERSION_8812E) ||
		(GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) 
	{
		if (hwinfo[EEPROM_8812_XTAL_K] == 0xff)
			priv->pmib->dot11RFEntry.xcap = 0x0;
		else
			priv->pmib->dot11RFEntry.xcap = hwinfo[EEPROM_8812_XTAL_K] & 0x3f;	//[5:0]
	}
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
		priv->pmib->dot11RFEntry.xcap = (hwinfo[EEPROM_XTAL_88E]);	//[7:0]
	}
#endif
#ifdef CONFIG_WLAN_HAL_8192EE
	if(GET_CHIP_VER(priv) == VERSION_8192E)
	{
		if( hwinfo[EEPROM_92E_XTAL_K] > 0x3f )
			priv->pmib->dot11RFEntry.xcap = 0;
		else
			priv->pmib->dot11RFEntry.xcap = hwinfo[EEPROM_92E_XTAL_K] & 0x3f; // [5:0]
	}
#endif
#if defined(CONFIG_RTL_92D_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		priv->pmib->dot11RFEntry.xcap = (hwinfo[EEPROM_92D_XTAL_K]);	//[7:0]
	}
#endif
}

#ifdef CONFIG_SDIO_HCI
void ReadChannelPlanFromEfuse(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);

	if (!(priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE))
		return;

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		if (0x20 == hwinfo[EEPROM_ChannelPlan_88E])
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_WORLD_WIDE;
		else if (0x21 == hwinfo[EEPROM_ChannelPlan_88E])
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_ETSI;
		else if (0x22 == hwinfo[EEPROM_ChannelPlan_88E])
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_FCC;
		else if (0x23 == hwinfo[EEPROM_ChannelPlan_88E])
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_MKK1;
		else if (0x24 == hwinfo[EEPROM_ChannelPlan_88E])
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_FRANCE;
		else if (0x41 == hwinfo[EEPROM_ChannelPlan_88E])
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_GLOBAL;
	}
#endif
}
#endif

#if defined(CONFIG_RTL_8812_SUPPORT)
void ReadTxBBSwingFromEfuse(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
	unsigned int swing_setting[4] = {12, 18, 24, 30};	
#if defined(CONFIG_RTL_8812_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		int idx = 0;
		if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G) {
			idx = hwinfo[EEPROM_2G_TxBBSwing] & 0x3; // 2G PathA OFDM
			priv->pshare->OFDM_index0[0] = swing_setting[idx];
			idx = (hwinfo[EEPROM_2G_TxBBSwing]>>2) & 0x3; // 2G PathB OFDM
			priv->pshare->OFDM_index0[1] = swing_setting[idx];
		} else if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
			idx = hwinfo[EEPROM_5G_TxBBSwing] & 0x3; // 5G PathA OFDM
			priv->pshare->OFDM_index0[0] = swing_setting[idx];
			idx = (hwinfo[EEPROM_5G_TxBBSwing]>>2) & 0x3; // 5G PathB OFDM
			priv->pshare->OFDM_index0[1] = swing_setting[idx];
		}
	}
#endif
}
#endif


#ifdef CONFIG_RTL_92D_SUPPORT
void ReadDeltaValFromEfuse(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);

	if (!(priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE))
		return;

	priv->pmib->dot11RFEntry.deltaIQK = (hwinfo[EEPROM_92D_IQK_DELTA] & 0x03);		//[1:0]
	priv->pmib->dot11RFEntry.deltaLCK = (hwinfo[EEPROM_92D_LCK_DELTA] & 0x0C) >> 2;	//[3:2]
}

void ReadTRSWPAPEFromEfuse(struct rtl8192cd_priv *priv)
{
	unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);

	if (!(priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE))
		return;

	priv->pmib->dot11RFEntry.trsw_pape_C9 = (hwinfo[EEPROM_92D_TRSW_CTRL] & 0xff);
	priv->pmib->dot11RFEntry.trsw_pape_CC = (hwinfo[EEPROM_92D_PAPE_CTRL] & 0xff);

	if (priv->pmib->dot11RFEntry.trsw_pape_C9 == 0xff)
		priv->pmib->dot11RFEntry.trsw_pape_C9 = 0;
}

#endif

int EfuseMapAlloc(struct rtl8192cd_priv *priv)
{
	int i;
	for( i = 0 ; i < 2 ; i++ ) {
		priv->EfuseMap[i] = (unsigned char*)kmalloc(sizeof(unsigned char)*priv->EfuseMapLen, GFP_ATOMIC);
		if (NULL == priv->EfuseMap[i])
			return FAIL;
		memset((void *)priv->EfuseMap[i], 0xff, sizeof(unsigned char)*priv->EfuseMapLen);
	}
	
	return SUCCESS;
}

enum efuse_cmd_type {
	TYPE_HW_TX_POWER_CCK_A,
	TYPE_HW_TX_POWER_CCK_B,
	TYPE_HW_TX_POWER_CCK_C,
	TYPE_HW_TX_POWER_CCK_D,
	TYPE_HW_TX_POWER_HT40_1S_A,
	TYPE_HW_TX_POWER_HT40_1S_B,
	TYPE_HW_TX_POWER_HT40_1S_C,
	TYPE_HW_TX_POWER_HT40_1S_D,
	TYPE_HW_WLAN0_WLAN_ADDR,
	TYPE_HW_11N_THER,
	TYPE_HW_TX_POWER_5G_HT40_1S_A,
	TYPE_HW_TX_POWER_5G_HT40_1S_B,
	TYPE_HW_TX_POWER_5G_HT40_1S_C,
	TYPE_HW_TX_POWER_5G_HT40_1S_D,
	TYPE_HW_11N_XCAP,
	TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A,
	TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B,
	TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_C,
	TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_D,
	TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_A,
	TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_B,
	TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_C,
	TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_D,
	TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_A,
	TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_B,
	TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_C,
	TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_D,
	TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_A,
	TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_B,
	TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_C,
	TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_D,
	TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_A,
	TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_B,
	TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_C,
	TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_D,
	TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_A,
	TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_B,
	TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_C,
	TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_D,
	TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A,
	TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_B,
	TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_C,
	TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_D,
	TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_A,
	TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_B,
	TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_C,
	TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_D,
	TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_A,
	TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_B,
	TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_C,
	TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_D,
	TYPE_LNA_TYPE,
	TYPE_SDIO_TYPE,
	TYPE_HW_REG_DOMAIN,
	TYPE_HW_TX_POWER_DIFF_HT40_2S,
	TYPE_HW_TX_POWER_DIFF_HT20,
	TYPE_HW_TX_POWER_DIFF_OFDM,
	TYPE_HW_TX_POWER_DIFF_5G_HT40_2S,
	TYPE_HW_TX_POWER_DIFF_5G_HT20,
	TYPE_HW_TX_POWER_DIFF_5G_OFDM,
	TYPE_HW_11N_TRSWPAPE_C9,
	TYPE_HW_11N_TRSWPAPE_CC,
	TYPE_EFUSE_CMD_END
};

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
struct efuse_cmd_t efuse_cmd_8188e[] = {
	{"HW_TX_POWER_CCK_A", TYPE_HW_TX_POWER_CCK_A, EEPROM_TX_PWR_INX_88E},
	{"HW_TX_POWER_HT40_1S_A", TYPE_HW_TX_POWER_HT40_1S_A, EEPROM_TX_PWR_INX_88E + 6},
	{"HW_TX_POWER_DIFF_20BW1S_OFDM1T_A", TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A, EEPROM_TX_PWR_INX_88E + 11},
	{"HW_WLAN0_WLAN_ADDR", TYPE_HW_WLAN0_WLAN_ADDR, EEPROM_MAC_ADDR_88EE},
	{"HW_11N_THER", TYPE_HW_11N_THER, EEPROM_THERMAL_METER_88E},
	{"HW_11N_XCAP", TYPE_HW_11N_XCAP, EEPROM_XTAL_88E},
	{"HW_REG_DOMAIN", TYPE_HW_REG_DOMAIN, EEPROM_ChannelPlan_88E},
};
#endif

#if defined(CONFIG_RTL_92C_SUPPORT)
struct efuse_cmd_t efuse_cmd_8192c[] = {	
	{"HW_TX_POWER_CCK_A", TYPE_HW_TX_POWER_CCK_A, EEPROM_TxPowerCCK},
	{"HW_TX_POWER_CCK_B", TYPE_HW_TX_POWER_CCK_B, EEPROM_TxPowerCCK + 3},
	{"HW_TX_POWER_HT40_1S_A", TYPE_HW_TX_POWER_HT40_1S_A, EEPROM_TxPowerHT40_1S},
	{"HW_TX_POWER_HT40_1S_B", TYPE_HW_TX_POWER_HT40_1S_B, EEPROM_TxPowerHT40_1S + 3},
	{"HW_TX_POWER_DIFF_HT40_2S", TYPE_HW_TX_POWER_DIFF_HT40_2S, EEPROM_TxPowerHT40_2SDiff},
	{"HW_TX_POWER_DIFF_HT20", TYPE_HW_TX_POWER_DIFF_HT20, EEPROM_TxPowerHT20Diff},
	{"HW_TX_POWER_DIFF_OFDM", TYPE_HW_TX_POWER_DIFF_OFDM, EEPROM_TxPowerOFDMDiff},
	{"HW_WLAN0_WLAN_ADDR", TYPE_HW_WLAN0_WLAN_ADDR, EEPROM_MACADDRESS},
	{"HW_11N_THER", TYPE_HW_11N_THER, EEPROM_THERMAL_METER},	
};
#endif

#if defined(CONFIG_RTL_92D_SUPPORT)
struct efuse_cmd_t efuse_cmd_8192d[] = {	
	{"HW_TX_POWER_CCK_A", TYPE_HW_TX_POWER_CCK_A, EEPROM_2G_TxPowerCCK},
	{"HW_TX_POWER_CCK_B", TYPE_HW_TX_POWER_CCK_B, EEPROM_2G_TxPowerCCK + 3},
	{"HW_TX_POWER_HT40_1S_A", TYPE_HW_TX_POWER_HT40_1S_A, EEPROM_2G_TxPowerHT40_1S},
	{"HW_TX_POWER_HT40_1S_B", TYPE_HW_TX_POWER_HT40_1S_B, EEPROM_2G_TxPowerHT40_1S + 3},
	{"HW_TX_POWER_DIFF_HT40_2S", TYPE_HW_TX_POWER_DIFF_HT40_2S, EEPROM_2G_TxPowerHT40_2SDiff},
	{"HW_TX_POWER_DIFF_HT20", TYPE_HW_TX_POWER_DIFF_HT20, EEPROM_2G_TxPowerHT20Diff},
	{"HW_TX_POWER_DIFF_OFDM", TYPE_HW_TX_POWER_DIFF_OFDM, EEPROM_2G_TxPowerOFDMDiff},
	{"HW_WLAN0_WLAN_ADDR", TYPE_HW_WLAN0_WLAN_ADDR, EEPROM_MAC0_MACADDRESS},
	{"HW_11N_THER", TYPE_HW_11N_THER, EEPROM_92D_THERMAL_METER},
	{"HW_TX_POWER_5G_HT40_1S_A", TYPE_HW_TX_POWER_5G_HT40_1S_A, EEPROM_5GL_TxPowerHT40_1S},
	{"HW_TX_POWER_5G_HT40_1S_B", TYPE_HW_TX_POWER_5G_HT40_1S_B, EEPROM_5GL_TxPowerHT40_1S + 3},
	{"HW_TX_POWER_DIFF_5G_HT40_2S", TYPE_HW_TX_POWER_DIFF_5G_HT40_2S, EEPROM_5GL_TxPowerHT40_2SDiff},
	{"HW_TX_POWER_DIFF_5G_HT20", TYPE_HW_TX_POWER_DIFF_5G_HT20, EEPROM_5GL_TxPowerHT20Diff},
	{"HW_TX_POWER_DIFF_5G_OFDM", TYPE_HW_TX_POWER_DIFF_5G_OFDM, EEPROM_5GL_TxPowerOFDMDiff},
	{"HW_11N_TRSWPAPE_C9", TYPE_HW_11N_TRSWPAPE_C9, EEPROM_92D_TRSW_CTRL},
	{"HW_11N_TRSWPAPE_CC", TYPE_HW_11N_TRSWPAPE_CC, EEPROM_92D_PAPE_CTRL},
	{"HW_11N_XCAP", TYPE_HW_11N_XCAP, EEPROM_92D_XTAL_K},	
};
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8822BE)
struct efuse_cmd_t efuse_cmd_8812e[] = {
	{"HW_TX_POWER_CCK_A", TYPE_HW_TX_POWER_CCK_A, EEPROM_2G_CCK1T_TxPower+PATHA_OFFSET},
	{"HW_TX_POWER_CCK_B", TYPE_HW_TX_POWER_CCK_B, EEPROM_2G_CCK1T_TxPower+PATHB_OFFSET},
	{"HW_TX_POWER_HT40_1S_A", TYPE_HW_TX_POWER_HT40_1S_A, EEPROM_2G_HT401S_TxPower + PATHA_OFFSET},
	{"HW_TX_POWER_HT40_1S_B", TYPE_HW_TX_POWER_HT40_1S_B, EEPROM_2G_HT401S_TxPower + PATHB_OFFSET},
	{"HW_WLAN0_WLAN_ADDR", TYPE_HW_WLAN0_WLAN_ADDR, EEPROM_8812_MACADDRESS},
	{"HW_11N_THER", TYPE_HW_11N_THER, EEPROM_8812_THERMAL_METER},
	{"HW_TX_POWER_5G_HT40_1S_A", TYPE_HW_TX_POWER_5G_HT40_1S_A, EEPROM_5G_HT401S_TxPower + PATHA_OFFSET},
	{"HW_TX_POWER_5G_HT40_1S_B", TYPE_HW_TX_POWER_5G_HT40_1S_B, EEPROM_5G_HT401S_TxPower + PATHB_OFFSET},
	{"HW_11N_XCAP", TYPE_HW_11N_XCAP, EEPROM_8812_XTAL_K},
	{"HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A", TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A, EEPROM_5G_HT201S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B", TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B, EEPROM_5G_HT201S_TxPowerDiff + PATHB_OFFSET},
	{"HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_A", TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_A, EEPROM_5G_HT402S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_B", TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_B, EEPROM_5G_HT402S_TxPowerDiff + PATHB_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_A", TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_A, EEPROM_5G_HT801S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_B", TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_B, EEPROM_5G_HT801S_TxPowerDiff + PATHB_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_A", TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_A, EEPROM_5G_HT802S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_B", TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_B, EEPROM_5G_HT802S_TxPowerDiff + PATHB_OFFSET},
	{"HW_TX_POWER_DIFF_20BW1S_OFDM1T_A", TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A, EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_20BW1S_OFDM1T_B", TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_B, EEPROM_2G_HT201S_TxPowerDiff + PATHB_OFFSET},
	{"HW_TX_POWER_DIFF_40BW2S_20BW2S_A", TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_A, EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_40BW2S_20BW2S_B", TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_B, EEPROM_2G_HT402S_TxPowerDiff + PATHB_OFFSET},
};
#endif

#if defined(CONFIG_WLAN_HAL_8192EE)
struct efuse_cmd_t efuse_cmd_8192e[] = {
	{"HW_TX_POWER_CCK_A", TYPE_HW_TX_POWER_CCK_A, EEPROM_2G_CCK1T_TxPower + PATHA_OFFSET},
	{"HW_TX_POWER_CCK_B", TYPE_HW_TX_POWER_CCK_B, EEPROM_2G_CCK1T_TxPower + PATHB_OFFSET},
	{"HW_TX_POWER_HT40_1S_A", TYPE_HW_TX_POWER_HT40_1S_A, EEPROM_2G_HT401S_TxPower + PATHA_OFFSET},
	{"HW_TX_POWER_HT40_1S_B", TYPE_HW_TX_POWER_HT40_1S_B, EEPROM_2G_HT401S_TxPower + PATHB_OFFSET},
	{"HW_TX_POWER_DIFF_20BW1S_OFDM1T_A", TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A, EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_20BW1S_OFDM1T_B", TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_B, EEPROM_2G_HT201S_TxPowerDiff + PATHB_OFFSET}, 
	{"HW_TX_POWER_DIFF_40BW2S_20BW2S_A", TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_A, EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_40BW2S_20BW2S_B", TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_B, EEPROM_2G_HT402S_TxPowerDiff + PATHB_OFFSET},
	{"HW_WLAN0_WLAN_ADDR", TYPE_HW_WLAN0_WLAN_ADDR, EEPROM_92E_MACADDRESS},
	{"HW_11N_THER", TYPE_HW_11N_THER, EEPROM_92E_THERMAL_METER},
	{"HW_11N_XCAP", TYPE_HW_11N_XCAP, EEPROM_92E_XTAL_K},
	{"EFUSE_LNA_TYPE", TYPE_LNA_TYPE, EEPROM_RFE_OPTION_8192E},
#ifdef CONFIG_SDIO_HCI
	{"SD", TYPE_SDIO_TYPE, EEPROM_92E_SDIOTYPE},
#endif
};
#endif

#if defined(CONFIG_WLAN_HAL_8814AE)
struct efuse_cmd_t efuse_cmd_8814a[] = {
	{"HW_TX_POWER_CCK_A", TYPE_HW_TX_POWER_CCK_A, EEPROM_2G_CCK1T_TxPower + PATHA_OFFSET},
	{"HW_TX_POWER_CCK_B", TYPE_HW_TX_POWER_CCK_B, EEPROM_2G_CCK1T_TxPower + PATHB_OFFSET},
	{"HW_TX_POWER_CCK_C", TYPE_HW_TX_POWER_CCK_C, EEPROM_2G_CCK1T_TxPower + PATHC_OFFSET},
	{"HW_TX_POWER_CCK_D", TYPE_HW_TX_POWER_CCK_D, EEPROM_2G_CCK1T_TxPower + PATHD_OFFSET},
	{"HW_TX_POWER_HT40_1S_A", TYPE_HW_TX_POWER_HT40_1S_A, EEPROM_2G_HT401S_TxPower + PATHA_OFFSET},
	{"HW_TX_POWER_HT40_1S_B", TYPE_HW_TX_POWER_HT40_1S_B, EEPROM_2G_HT401S_TxPower + PATHB_OFFSET},
	{"HW_TX_POWER_HT40_1S_C", TYPE_HW_TX_POWER_HT40_1S_C, EEPROM_2G_HT401S_TxPower + PATHC_OFFSET},
	{"HW_TX_POWER_HT40_1S_D", TYPE_HW_TX_POWER_HT40_1S_D, EEPROM_2G_HT401S_TxPower + PATHD_OFFSET},
	{"HW_WLAN0_WLAN_ADDR", TYPE_HW_WLAN0_WLAN_ADDR, EEPROM_8812_MACADDRESS},
	{"HW_11N_THER", TYPE_HW_11N_THER, EEPROM_8812_THERMAL_METER},
	{"HW_TX_POWER_5G_HT40_1S_A", TYPE_HW_TX_POWER_5G_HT40_1S_A, EEPROM_5G_HT401S_TxPower + PATHA_OFFSET},
	{"HW_TX_POWER_5G_HT40_1S_B", TYPE_HW_TX_POWER_5G_HT40_1S_B, EEPROM_5G_HT401S_TxPower + PATHB_OFFSET},
	{"HW_TX_POWER_5G_HT40_1S_C", TYPE_HW_TX_POWER_5G_HT40_1S_C, EEPROM_5G_HT401S_TxPower + PATHC_OFFSET},
	{"HW_TX_POWER_5G_HT40_1S_D", TYPE_HW_TX_POWER_5G_HT40_1S_D, EEPROM_5G_HT401S_TxPower + PATHD_OFFSET},
	{"HW_11N_XCAP", TYPE_HW_11N_XCAP, EEPROM_8812_XTAL_K},
	{"HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A", TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A, EEPROM_5G_HT201S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B", TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B, EEPROM_5G_HT201S_TxPowerDiff + PATHB_OFFSET},
	{"HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_C", TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_C, EEPROM_5G_HT201S_TxPowerDiff + PATHC_OFFSET},
	{"HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_D", TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_D, EEPROM_5G_HT201S_TxPowerDiff + PATHD_OFFSET},
	{"HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_A", TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_A, EEPROM_5G_HT402S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_B", TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_B, EEPROM_5G_HT402S_TxPowerDiff + PATHB_OFFSET},
	{"HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_C", TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_C, EEPROM_5G_HT402S_TxPowerDiff + PATHC_OFFSET},
	{"HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_D", TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_D, EEPROM_5G_HT402S_TxPowerDiff + PATHD_OFFSET},
	{"HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_A", TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_A, EEPROM_5G_HT403S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_B", TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_B, EEPROM_5G_HT403S_TxPowerDiff + PATHB_OFFSET},
	{"HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_C", TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_C, EEPROM_5G_HT403S_TxPowerDiff + PATHC_OFFSET},
	{"HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_D", TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_D, EEPROM_5G_HT403S_TxPowerDiff + PATHD_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_A", TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_A, EEPROM_5G_HT801S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_B", TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_B, EEPROM_5G_HT801S_TxPowerDiff + PATHB_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_C", TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_C, EEPROM_5G_HT801S_TxPowerDiff + PATHC_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_D", TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_D, EEPROM_5G_HT801S_TxPowerDiff + PATHD_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_A", TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_A, EEPROM_5G_HT802S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_B", TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_B, EEPROM_5G_HT802S_TxPowerDiff + PATHB_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_C", TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_C, EEPROM_5G_HT802S_TxPowerDiff + PATHC_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_D", TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_D, EEPROM_5G_HT802S_TxPowerDiff + PATHD_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_A", TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_A, EEPROM_5G_HT803S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_B", TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_B, EEPROM_5G_HT803S_TxPowerDiff + PATHB_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_C", TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_C, EEPROM_5G_HT803S_TxPowerDiff + PATHC_OFFSET},
	{"HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_D", TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_D, EEPROM_5G_HT803S_TxPowerDiff + PATHD_OFFSET},
	{"HW_TX_POWER_DIFF_20BW1S_OFDM1T_A", TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A, EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_20BW1S_OFDM1T_B", TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_B, EEPROM_2G_HT201S_TxPowerDiff + PATHB_OFFSET},
	{"HW_TX_POWER_DIFF_20BW1S_OFDM1T_C", TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_C, EEPROM_2G_HT201S_TxPowerDiff + PATHC_OFFSET},
	{"HW_TX_POWER_DIFF_20BW1S_OFDM1T_D", TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_D, EEPROM_2G_HT201S_TxPowerDiff + PATHD_OFFSET},
	{"HW_TX_POWER_DIFF_40BW2S_20BW2S_A", TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_A, EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_40BW2S_20BW2S_B", TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_B, EEPROM_2G_HT402S_TxPowerDiff + PATHB_OFFSET},
	{"HW_TX_POWER_DIFF_40BW2S_20BW2S_C", TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_C, EEPROM_2G_HT402S_TxPowerDiff + PATHC_OFFSET},
	{"HW_TX_POWER_DIFF_40BW2S_20BW2S_D", TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_D, EEPROM_2G_HT402S_TxPowerDiff + PATHD_OFFSET},
	{"HW_TX_POWER_DIFF_40BW3S_20BW3S_A", TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_A, EEPROM_2G_HT403S_TxPowerDiff + PATHA_OFFSET},
	{"HW_TX_POWER_DIFF_40BW3S_20BW3S_B", TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_B, EEPROM_2G_HT403S_TxPowerDiff + PATHB_OFFSET},
	{"HW_TX_POWER_DIFF_40BW3S_20BW3S_C", TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_C, EEPROM_2G_HT403S_TxPowerDiff + PATHC_OFFSET},
	{"HW_TX_POWER_DIFF_40BW3S_20BW3S_D", TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_D, EEPROM_2G_HT403S_TxPowerDiff + PATHD_OFFSET},
};
#endif


int EfuseCmdInit(struct rtl8192cd_priv *priv)
{
	u1Byte i, efuseCmdNum;
	struct efuse_cmd_t *efuseCmd;

	switch(GET_CHIP_VER(priv)) {
#if defined(CONFIG_RTL_92C_SUPPORT)
	case VERSION_8188C:
	case VERSION_8192C:
	{
		efuseCmd = efuse_cmd_8192c;
		efuseCmdNum = ARRAY_SIZE(efuse_cmd_8192c);
		break;
	}
#endif
#if defined(CONFIG_RTL_92D_SUPPORT)
	case VERSION_8192D:
	{
		efuseCmd = efuse_cmd_8192d;
		efuseCmdNum = ARRAY_SIZE(efuse_cmd_8192d);
		break;
	}
#endif
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	case VERSION_8188E:
	case VERSION_8723B:
	{
		efuseCmd = efuse_cmd_8188e;
		efuseCmdNum = ARRAY_SIZE(efuse_cmd_8188e);
		break;
	}
#endif
#if defined(CONFIG_WLAN_HAL_8192EE)
	case VERSION_8192E:
	{
		efuseCmd = efuse_cmd_8192e;
		efuseCmdNum = ARRAY_SIZE(efuse_cmd_8192e);
		break;
	}
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8822BE)
	case VERSION_8812E:
	case VERSION_8822B:
	{
		efuseCmd = efuse_cmd_8812e;
		efuseCmdNum = ARRAY_SIZE(efuse_cmd_8812e);
		break;
	}
#endif
#if defined(CONFIG_WLAN_HAL_8814AE)
	case VERSION_8814A:
	{
		efuseCmd = efuse_cmd_8814a;
		efuseCmdNum = ARRAY_SIZE(efuse_cmd_8814a);
		break;
	}
#endif
	default:
		DEBUG_ERR("No efuse cmd support\n");
		return FAIL;
	}

	priv->efuseCmd = efuseCmd;
	priv->efuseCmdNum = efuseCmdNum;

	DEBUG_INFO("efuseCmdNum=%d\n", priv->efuseCmdNum);		
	for ( i = 0 ; i != priv->efuseCmdNum ; ++i ) {
		DEBUG_INFO("efuseCmd[%d]=%s\n", i, priv->efuseCmd[i].name);
	}
	return SUCCESS;

#if 0
	priv->EfuseCmd = (char**)kmalloc(sizeof(char*)*priv->EfuseCmdNum, GFP_ATOMIC);
	if (NULL == priv->EfuseCmd)
		return FAIL;

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if( GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B)
	{
		priv->EfuseCmd[0] = "HW_TX_POWER_CCK_A";
		priv->EfuseCmd[1] = "HW_TX_POWER_HT40_1S_A";
		priv->EfuseCmd[2] = "HW_TX_POWER_DIFF_HT20_OFDM";
		priv->EfuseCmd[3] = "HW_WLAN0_WLAN_ADDR";
		priv->EfuseCmd[4] = "EFUSE_MAP0";
		priv->EfuseCmd[5] = "EFUSE_MAP1";
		priv->EfuseCmd[6] = "HW_11N_THER";
		priv->EfuseCmd[7] = "HW_11N_XCAP";
#ifdef CONFIG_SDIO_HCI
		priv->EfuseCmd[8] = "HW_REG_DOMAIN";
#endif

	}
#endif

#ifdef CONFIG_RTL_92C_SUPPORT
	if( GET_CHIP_VER(priv) == VERSION_8192C || GET_CHIP_VER(priv) == VERSION_8188C )
	{
		priv->EfuseCmd[0] = "HW_TX_POWER_CCK_A";
		priv->EfuseCmd[1] = "HW_TX_POWER_CCK_B";
		priv->EfuseCmd[2] = "HW_TX_POWER_HT40_1S_A";
		priv->EfuseCmd[3] = "HW_TX_POWER_HT40_1S_B";
		priv->EfuseCmd[4] = "HW_TX_POWER_DIFF_HT40_2S";
		priv->EfuseCmd[5] = "HW_TX_POWER_DIFF_HT20";
		priv->EfuseCmd[6] = "HW_TX_POWER_DIFF_OFDM";
		priv->EfuseCmd[7] = "HW_WLAN0_WLAN_ADDR";
		priv->EfuseCmd[8] = "EFUSE_MAP0";
		priv->EfuseCmd[9] = "EFUSE_MAP1";
		priv->EfuseCmd[10] = "HW_11N_THER";
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if( GET_CHIP_VER(priv) == VERSION_8192D )
	{
		priv->EfuseCmd[0] = "HW_TX_POWER_CCK_A";
		priv->EfuseCmd[1] = "HW_TX_POWER_CCK_B";
		priv->EfuseCmd[2] = "HW_TX_POWER_HT40_1S_A";
		priv->EfuseCmd[3] = "HW_TX_POWER_HT40_1S_B";
		priv->EfuseCmd[4] = "HW_TX_POWER_DIFF_HT40_2S";
		priv->EfuseCmd[5] = "HW_TX_POWER_DIFF_HT20";
		priv->EfuseCmd[6] = "HW_TX_POWER_DIFF_OFDM";
		priv->EfuseCmd[7] = "HW_WLAN0_WLAN_ADDR";
		priv->EfuseCmd[8] = "EFUSE_MAP0";
		priv->EfuseCmd[9] = "EFUSE_MAP1";
		priv->EfuseCmd[10] = "HW_11N_THER";
		priv->EfuseCmd[11] = "HW_TX_POWER_5G_HT40_1S_A";
		priv->EfuseCmd[12] = "HW_TX_POWER_5G_HT40_1S_B";
		priv->EfuseCmd[13] = "HW_TX_POWER_DIFF_5G_HT40_2S";
		priv->EfuseCmd[14] = "HW_TX_POWER_DIFF_5G_HT20";
		priv->EfuseCmd[15] = "HW_TX_POWER_DIFF_5G_OFDM";
		priv->EfuseCmd[16] = "HW_11N_TRSWPAPE_C9";
		priv->EfuseCmd[17] = "HW_11N_TRSWPAPE_CC";
		priv->EfuseCmd[18] = "HW_11N_XCAP";
	}
#endif

#if defined(CONFIG_WLAN_HAL_8192EE)
	if( GET_CHIP_VER(priv) == VERSION_8192E)
	{
		priv->EfuseCmd[0] = "HW_TX_POWER_CCK_A";
		priv->EfuseCmd[1] = "HW_TX_POWER_CCK_B";
		priv->EfuseCmd[2] = "HW_TX_POWER_HT40_1S_A";
		priv->EfuseCmd[3] = "HW_TX_POWER_HT40_1S_B";
		priv->EfuseCmd[4] = "HW_TX_POWER_DIFF_HT20_A";
		priv->EfuseCmd[5] = "HW_TX_POWER_DIFF_HT20_B";
		priv->EfuseCmd[6] = "HW_TX_POWER_DIFF_HT20_OFDM_A";
		priv->EfuseCmd[7] = "HW_TX_POWER_DIFF_HT20_OFDM_B";
		priv->EfuseCmd[8] = "HW_TX_POWER_DIFF_HT40_2S_A";
		priv->EfuseCmd[9] = "HW_TX_POWER_DIFF_HT40_2S_B";
		priv->EfuseCmd[10] = "HW_WLAN0_WLAN_ADDR";
		priv->EfuseCmd[11] = "EFUSE_MAP0";
		priv->EfuseCmd[12] = "EFUSE_MAP1";
		priv->EfuseCmd[13] = "HW_11N_THER";
		priv->EfuseCmd[14] = "HW_11N_XCAP";
		priv->EfuseCmd[15] = "EFUSE_TEST";
		priv->EfuseCmd[16] = "EFUSE_LNA_TYPE";
#ifdef CONFIG_SDIO_HCI
		priv->EfuseCmd[17] = "SD";
#endif
	}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
	if( GET_CHIP_VER(priv) == VERSION_8812E )
	{
		priv->EfuseCmd[0] = "HW_TX_POWER_CCK_A";
		priv->EfuseCmd[1] = "HW_TX_POWER_CCK_B";
		priv->EfuseCmd[2] = "HW_TX_POWER_HT40_1S_A";
		priv->EfuseCmd[3] = "HW_TX_POWER_HT40_1S_B";
		priv->EfuseCmd[4] = "HW_TX_POWER_DIFF_HT40_2S";
		priv->EfuseCmd[5] = "HW_TX_POWER_DIFF_HT20";
		priv->EfuseCmd[6] = "HW_TX_POWER_DIFF_OFDM";
		priv->EfuseCmd[7] = "HW_WLAN0_WLAN_ADDR";
		priv->EfuseCmd[8] = "EFUSE_MAP0";
		priv->EfuseCmd[9] = "EFUSE_MAP1";
		priv->EfuseCmd[10] = "HW_11N_THER";
		priv->EfuseCmd[11] = "HW_TX_POWER_5G_HT40_1S_A";
		priv->EfuseCmd[12] = "HW_TX_POWER_5G_HT40_1S_B";
		priv->EfuseCmd[13] = "HW_TX_POWER_DIFF_5G_HT40_2S";
		priv->EfuseCmd[14] = "HW_TX_POWER_DIFF_5G_HT20";
		priv->EfuseCmd[15] = "HW_TX_POWER_DIFF_5G_OFDM";
		priv->EfuseCmd[16] = "HW_11N_TRSWPAPE_C9";
		priv->EfuseCmd[17] = "HW_11N_TRSWPAPE_CC";
		priv->EfuseCmd[18] = "HW_11N_XCAP";
		priv->EfuseCmd[19] = "HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A";
		priv->EfuseCmd[20] = "HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B";
		priv->EfuseCmd[21] = "HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_A";
		priv->EfuseCmd[22] = "HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_B";
		priv->EfuseCmd[23] = "HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_A";
		priv->EfuseCmd[24] = "HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_B";
		priv->EfuseCmd[25] = "HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_A";
		priv->EfuseCmd[26] = "HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_B";
		priv->EfuseCmd[27] = "HW_TX_POWER_DIFF_20BW1S_OFDM1T_A";
		priv->EfuseCmd[28] = "HW_TX_POWER_DIFF_20BW1S_OFDM1T_B";
		priv->EfuseCmd[29] = "HW_TX_POWER_DIFF_40BW2S_20BW2S_A";
		priv->EfuseCmd[30] = "HW_TX_POWER_DIFF_40BW2S_20BW2S_B";		
	}
#endif
#ifdef CONFIG_WLAN_HAL_8814AE
	if( GET_CHIP_VER(priv) == VERSION_8814A)
	{//1 EXTEND
		priv->EfuseCmd[0] = "HW_TX_POWER_CCK_A";
		priv->EfuseCmd[1] = "HW_TX_POWER_CCK_B";
		priv->EfuseCmd[2] = "HW_TX_POWER_CCK_C";
		priv->EfuseCmd[3] = "HW_TX_POWER_CCK_D";
		priv->EfuseCmd[4] = "HW_TX_POWER_HT40_1S_A";
		priv->EfuseCmd[5] = "HW_TX_POWER_HT40_1S_B";
		priv->EfuseCmd[6] = "HW_TX_POWER_HT40_1S_C";
		priv->EfuseCmd[7] = "HW_TX_POWER_HT40_1S_D";
		priv->EfuseCmd[8] = "HW_TX_POWER_DIFF_HT40_2S";
		priv->EfuseCmd[9] = "HW_TX_POWER_DIFF_HT20";
		priv->EfuseCmd[10] = "HW_TX_POWER_DIFF_OFDM";
		priv->EfuseCmd[11] = "HW_WLAN0_WLAN_ADDR";
		priv->EfuseCmd[12] = "EFUSE_MAP0";
		priv->EfuseCmd[13] = "EFUSE_MAP1";
		priv->EfuseCmd[14] = "HW_11N_THER";
		priv->EfuseCmd[15] = "HW_TX_POWER_5G_HT40_1S_A";
		priv->EfuseCmd[16] = "HW_TX_POWER_5G_HT40_1S_B";
		priv->EfuseCmd[17] = "HW_TX_POWER_5G_HT40_1S_C";
		priv->EfuseCmd[18] = "HW_TX_POWER_5G_HT40_1S_D";
		priv->EfuseCmd[19] = "HW_TX_POWER_DIFF_5G_HT40_2S";
		priv->EfuseCmd[20] = "HW_TX_POWER_DIFF_5G_HT20";
		priv->EfuseCmd[21] = "HW_TX_POWER_DIFF_5G_OFDM";
		priv->EfuseCmd[22] = "HW_11N_TRSWPAPE_C9";
		priv->EfuseCmd[23] = "HW_11N_TRSWPAPE_CC";
		priv->EfuseCmd[24] = "HW_11N_XCAP";
		priv->EfuseCmd[25] = "HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A";
		priv->EfuseCmd[26] = "HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B";
		priv->EfuseCmd[27] = "HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_C";
		priv->EfuseCmd[28] = "HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_D";
		priv->EfuseCmd[29] = "HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_A";
		priv->EfuseCmd[30] = "HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_B";
		priv->EfuseCmd[31] = "HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_C";
		priv->EfuseCmd[32] = "HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_D";	
		priv->EfuseCmd[33] = "HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_A";
		priv->EfuseCmd[34] = "HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_B";
		priv->EfuseCmd[35] = "HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_C";
		priv->EfuseCmd[36] = "HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_D";		
		priv->EfuseCmd[37] = "HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_A";
		priv->EfuseCmd[38] = "HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_B";
		priv->EfuseCmd[39] = "HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_C";
		priv->EfuseCmd[40] = "HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_D";		
		priv->EfuseCmd[41] = "HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_A";
		priv->EfuseCmd[42] = "HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_B";
		priv->EfuseCmd[43] = "HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_C";
		priv->EfuseCmd[44] = "HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_D";
		priv->EfuseCmd[45] = "HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_A";
		priv->EfuseCmd[46] = "HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_B";
		priv->EfuseCmd[47] = "HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_C";
		priv->EfuseCmd[48] = "HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_D";		
		priv->EfuseCmd[49] = "HW_TX_POWER_DIFF_20BW1S_OFDM1T_A";
		priv->EfuseCmd[50] = "HW_TX_POWER_DIFF_20BW1S_OFDM1T_B";
		priv->EfuseCmd[51] = "HW_TX_POWER_DIFF_20BW1S_OFDM1T_C";
		priv->EfuseCmd[52] = "HW_TX_POWER_DIFF_20BW1S_OFDM1T_D";
		priv->EfuseCmd[53] = "HW_TX_POWER_DIFF_40BW2S_20BW2S_A";
		priv->EfuseCmd[54] = "HW_TX_POWER_DIFF_40BW2S_20BW2S_B";		
		priv->EfuseCmd[55] = "HW_TX_POWER_DIFF_40BW2S_20BW2S_C";
		priv->EfuseCmd[56] = "HW_TX_POWER_DIFF_40BW2S_20BW2S_D";	
		priv->EfuseCmd[57] = "HW_TX_POWER_DIFF_40BW3S_20BW3S_A";
		priv->EfuseCmd[58] = "HW_TX_POWER_DIFF_40BW3S_20BW3S_B";		
		priv->EfuseCmd[59] = "HW_TX_POWER_DIFF_40BW3S_20BW3S_C";
		priv->EfuseCmd[60] = "HW_TX_POWER_DIFF_40BW3S_20BW3S_D";		
	}                
#endif
	
	return SUCCESS;
#endif
}

void EfuseInit(struct rtl8192cd_priv *priv)
{
	
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if( GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B)
	{
		priv->EfuseRealContentLen = 256;
		priv->EfuseMapLen = 512;
		priv->EfuseMaxSection= 64;
		priv->EfuseOobProtectBytes= 18;
	}
#endif

#ifdef CONFIG_RTL_92C_SUPPORT
	if( GET_CHIP_VER(priv) == VERSION_8192C || GET_CHIP_VER(priv) == VERSION_8188C )
	{
		priv->EfuseRealContentLen = 512;
		priv->EfuseMapLen = 128;
		priv->EfuseMaxSection= 16;
		priv->EfuseOobProtectBytes= 15;
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if( GET_CHIP_VER(priv) == VERSION_8192D )
	{
		priv->EfuseRealContentLen = 1024;
		priv->EfuseMapLen= 256;
		priv->EfuseMaxSection= 32;
		priv->EfuseOobProtectBytes= 18;
	}
#endif

#if defined(CONFIG_WLAN_HAL_8192EE)
	if( GET_CHIP_VER(priv) == VERSION_8192E)
	{
		priv->EfuseRealContentLen = 512;
		priv->EfuseMapLen = 512;
		priv->EfuseMaxSection = 64;
		priv->EfuseOobProtectBytes = 15;
	}	
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
	if( GET_CHIP_VER(priv) == VERSION_8812E )
	{
		priv->EfuseRealContentLen = 1024;
		priv->EfuseMapLen= 512;
		priv->EfuseMaxSection= 64;
		priv->EfuseOobProtectBytes= 506;
	}
#endif

#ifdef CONFIG_WLAN_HAL_8814AE
	if( GET_CHIP_VER(priv) == VERSION_8814A )
	{
		priv->EfuseRealContentLen = 1024;
		priv->EfuseMapLen= 512;
		priv->EfuseMaxSection= 64;
		priv->EfuseOobProtectBytes= 506;
	}
#endif

#ifdef CONFIG_WLAN_HAL_8822BE
	if( GET_CHIP_VER(priv) == VERSION_8822B)
	{
		priv->EfuseRealContentLen = 1024;
		priv->EfuseMapLen= 1024;
		priv->EfuseMaxSection= 128;
		priv->EfuseOobProtectBytes= 506;
	}
#endif
}

//
//	Description:
//		Read HW adapter information by E-Fuse or EEPROM according CR9346 reported.
//
//	Assumption:
//		1. CR9346 regiser has verified.
//		2. PASSIVE_LEVEL (USB interface)
//
//	Created by Roger, 2008.10.21.
//
int ReadAdapterInfo8192CE(struct rtl8192cd_priv *priv)
{
	if (!(priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE))
		return 0;

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		unsigned char			tmpU1b;
		tmpU1b = RTL_R8(CR9346);

		// To check system boot selection.
		if (tmpU1b & CmdEERPOMSEL)	{
			DEBUG_INFO("Boot from EEPROM\n");
		} else	{
			DEBUG_INFO("Boot from EFUSE\n");
		}

		// To check autoload success or not.
		if (tmpU1b & CmdEEPROM_En)	{
			DEBUG_INFO("Autoload OK!!\n");
			priv->AutoloadFailFlag = FALSE;
#if 0
			EFUSE_ShadowMapUpdate(priv);
			ReadTxPowerInfoFromHWPG(priv);
			ReadMacAddressFromEfuse(priv);
#endif
		} else { // Auto load fail.
			DEBUG_INFO("AutoLoad Fail reported from CR9346!!\n");
			priv->AutoloadFailFlag = TRUE;
		}

#ifdef CONFIG_SDIO_HCI
		// card enable before eFuse access
		if(_CardEnable(priv) == FAIL) {
			printk(KERN_ERR "%s: run power on flow fail\n", __func__);
			return -EIO;
		}
#endif

		EfuseInit(priv);
		if (EfuseMapAlloc(priv) == FAIL) {
			printk(KERN_ERR "%s: can't allocate efuse map\n", __func__);
			return -ENOMEM;
		}
		if (EfuseCmdInit(priv) == FAIL) {
			printk(KERN_ERR "%s: can't allocate efuse cmd\n", __func__);
			return -ENOMEM;
		}
		EFUSE_ShadowMapUpdate(priv);

#ifdef CONFIG_SDIO_HCI
		// card disable after finishing eFuse access
#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv))
			GET_HAL_INTERFACE(priv)->StopHWHandler(priv);
		else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
			rtl8192cd_stop_hw(priv);
#endif // CONFIG_SDIO_HCI
	}
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	else {
		int i;
		
		EfuseInit(priv);
		for( i = 0 ; i < 2 ; i++ )
			priv->EfuseMap[i] = priv->proot_priv->EfuseMap[i];
		priv->efuseCmd = priv->proot_priv->efuseCmd;
	}
#endif


	ReadTxPowerInfoFromHWPG(priv);
	ReadThermalMeterFromEfuse(priv);
	ReadMacAddressFromEfuse(priv);

#ifdef CONFIG_LNA_FROM_EFUSE
	if (GET_CHIP_VER(priv) == VERSION_8192E)
		ReadLNATypeFromEfuse(priv);
#endif

#ifdef CONFIG_USB_HCI
	ReadIDFromEfuse(priv);
	ReadBoardTypeFromEfuse(priv);
#endif
#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		ReadDeltaValFromEfuse(priv);
		ReadTRSWPAPEFromEfuse(priv);
	}
#endif

	if ((GET_CHIP_VER(priv) == VERSION_8192D) ||
		(GET_CHIP_VER(priv) == VERSION_8188E) || 
		(GET_CHIP_VER(priv) == VERSION_8812E) || 
		(GET_CHIP_VER(priv) == VERSION_8192E) ||
		(GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B))
	{
		ReadCrystalCalibrationFromEfuse(priv);
	}

#ifdef CONFIG_SDIO_HCI
	if (GET_CHIP_VER(priv)==VERSION_8188E)
		ReadChannelPlanFromEfuse(priv);
#endif

	return 0;
}
#if 0
#ifdef CONFIG_RTL_88E_SUPPORT
static char FLASH_NAME_PARAM[][50] = {
	"HW_TX_POWER_CCK_A",
	"HW_TX_POWER_HT40_1S_A",
	"HW_TX_POWER_DIFF_HT20_OFDM",
	"HW_WLAN0_WLAN_ADDR",
	"EFUSE_MAP0",
	"EFUSE_MAP1",
	"HW_11N_THER",
	"HW_11N_XCAP",
#ifdef CONFIG_SDIO_HCI
	"HW_REG_DOMAIN",
#endif
};
#elif defined(CONFIG_WLAN_HAL_8192EE)
static char FLASH_NAME_PARAM[][50] = {
	"HW_TX_POWER_CCK_A",
	"HW_TX_POWER_CCK_B",
	"HW_TX_POWER_HT40_1S_A",
	"HW_TX_POWER_HT40_1S_B",
	"HW_TX_POWER_DIFF_HT20_OFDM",
	"HW_TX_POWER_DIFF_HT40_2S",
	"HW_WLAN0_WLAN_ADDR",
	"EFUSE_MAP0",
	"EFUSE_MAP1",
	"HW_11N_THER",
	"HW_11N_XCAP"		//there's more
};
#else
static char FLASH_NAME_PARAM[][50] = {
	"HW_TX_POWER_CCK_A",
	"HW_TX_POWER_CCK_B",
	"HW_TX_POWER_HT40_1S_A",
	"HW_TX_POWER_HT40_1S_B",
	"HW_TX_POWER_DIFF_HT40_2S",
	"HW_TX_POWER_DIFF_HT20",
	"HW_TX_POWER_DIFF_OFDM",
	"HW_WLAN0_WLAN_ADDR",
	"EFUSE_MAP0",
	"EFUSE_MAP1",
	"HW_11N_THER",
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
	"HW_TX_POWER_5G_HT40_1S_A",
	"HW_TX_POWER_5G_HT40_1S_B",
	"HW_TX_POWER_DIFF_5G_HT40_2S",
	"HW_TX_POWER_DIFF_5G_HT20",
	"HW_TX_POWER_DIFF_5G_OFDM",
	"HW_11N_TRSWPAPE_C9",
	"HW_11N_TRSWPAPE_CC",
	"HW_11N_XCAP",
#endif
#if defined(CONFIG_RTL_8812_SUPPORT)
	"HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A",
	"HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B",
	"HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_A",
	"HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_B",
	"HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_A",
	"HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_B",
	"HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_A",
	"HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_B",
	"HW_TX_POWER_DIFF_20BW1S_OFDM1T_A",
	"HW_TX_POWER_DIFF_20BW1S_OFDM1T_B",
	"HW_TX_POWER_DIFF_40BW2S_20BW2S_A",
	"HW_TX_POWER_DIFF_40BW2S_20BW2S_B"
#endif
};
#endif

#define EFUSECMD_NUM_92C 11
#define EFUSECMD_NUM_92D 19
#define EFUSECMD_NUM_8812 31
#ifdef CONFIG_SDIO_HCI
#define EFUSECMD_NUM_88E 9
#else
#define EFUSECMD_NUM_88E 8
#endif
#define EFUSECMD_NUM_92E 11

#endif

static int getEEPROMOffset(struct rtl8192cd_priv *priv, int type)
{
	int offset = 0;

#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
		switch (type) {
		case 0:
			offset = EEPROM_TxPowerCCK;
			break;
		case 1:
			offset = EEPROM_TxPowerCCK + 3;
			break;
		case 2:
			offset = EEPROM_TxPowerHT40_1S;
			break;
		case 3:
			offset = EEPROM_TxPowerHT40_1S + 3;
			break;
		case 4:
			offset = EEPROM_TxPowerHT40_2SDiff;
			break;
		case 5:
			offset = EEPROM_TxPowerHT20Diff;
			break;
		case 6:
			offset = EEPROM_TxPowerOFDMDiff;
			break;
		case 7:
			offset = EEPROM_MACADDRESS;
			break;
		case 10:
			offset = EEPROM_THERMAL_METER;
			break;
		default:
			offset = -1;
			panic_printk("NOT SUPPORT!!\n");
			break;
		}
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		switch (type) {
		case 0:
			offset = EEPROM_2G_TxPowerCCK;
			break;
		case 1:
			offset = EEPROM_2G_TxPowerCCK + 3;
			break;
		case 2:
			offset = EEPROM_2G_TxPowerHT40_1S;
			break;
		case 3:
			offset = EEPROM_2G_TxPowerHT40_1S + 3;
			break;
		case 4:
			offset = EEPROM_2G_TxPowerHT40_2SDiff;
			break;
		case 5:
			offset = EEPROM_2G_TxPowerHT20Diff;
			break;
		case 6:
			offset = EEPROM_2G_TxPowerOFDMDiff;
			break;
		case 7:
#ifdef CONFIG_RTL_92D_DMDP
			if (priv->pshare->wlandev_idx == 1)
				offset = EEPROM_MAC1_MACADDRESS;
			else
#endif
				offset = EEPROM_MAC0_MACADDRESS;
			break;
		case 8:
			offset = 0x00;
			break;
		case 9:
			offset = 0x32;
			break;
		case 10:
			offset = EEPROM_92D_THERMAL_METER;
			break;
		case 11:
			offset = EEPROM_5GL_TxPowerHT40_1S;
			break;
		case 12:
			offset = EEPROM_5GL_TxPowerHT40_1S + 3;
			break;
		case 13:
			offset = EEPROM_5GL_TxPowerHT40_2SDiff;
			break;
		case 14:
			offset = EEPROM_5GL_TxPowerHT20Diff;
			break;
		case 15:
			offset = EEPROM_5GL_TxPowerOFDMDiff;
			break;
		case 16:
			offset = EEPROM_92D_TRSW_CTRL;
			break;
		case 17:
			offset = EEPROM_92D_PAPE_CTRL;
			break;
		case 18:
			offset = EEPROM_92D_XTAL_K;
			break;
		default:
			offset = -1;
			panic_printk("NOT SUPPORT!!\n");
			break;
		}
	}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		switch (type) {
		case 0:
			offset = EEPROM_2G_CCK1T_TxPower + PATHA_OFFSET;
			break;
		case 1:
			offset = EEPROM_2G_CCK1T_TxPower + PATHB_OFFSET;
			break;
		case 2:
			offset = EEPROM_2G_HT401S_TxPower + PATHA_OFFSET;
			break;
		case 3:
			offset = EEPROM_2G_HT401S_TxPower + PATHA_OFFSET;
			break;
		case 4:
			offset = EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 5:
			offset = EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 6:
			offset = EEPROM_2G_OFDM1T_TxPowerDiff + PATHA_OFFSET;
			break;
		case 7:
			offset = EEPROM_8812_MACADDRESS;
			break;
		case 8:
			offset = 0x00;
			break;
		case 9:
			offset = 0x32;
			break;
		case 10:
			offset = EEPROM_8812_THERMAL_METER;
			break;
		case 11:
			offset = EEPROM_5G_HT401S_TxPower + PATHA_OFFSET;
			break;
		case 12:
			offset = EEPROM_5G_HT401S_TxPower + PATHB_OFFSET;
			break;
		case 18:
			offset = EEPROM_8812_XTAL_K;
			break;
		case 19:
			offset = EEPROM_5G_HT201S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 20:
			offset = EEPROM_5G_HT201S_TxPowerDiff + PATHB_OFFSET;
			break;
		case 21:
			offset = EEPROM_5G_HT402S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 22:
			offset = EEPROM_5G_HT402S_TxPowerDiff + PATHB_OFFSET;
			break;
		case 23:
			offset = EEPROM_5G_HT801S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 24:
			offset = EEPROM_5G_HT801S_TxPowerDiff + PATHB_OFFSET;
			break;
		case 25:
			offset = EEPROM_5G_HT802S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 26:
			offset = EEPROM_5G_HT802S_TxPowerDiff + PATHB_OFFSET;
			break;
		case 27:
			offset = EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 28:
			offset = EEPROM_2G_HT201S_TxPowerDiff + PATHB_OFFSET;
			break;
		case 29:
			offset = EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 30:
			offset = EEPROM_2G_HT402S_TxPowerDiff + PATHB_OFFSET;
			break;
		default:
			offset = -1;
			panic_printk("NOT SUPPORT!!\n");
			break;
		}
	}
#endif

#ifdef CONFIG_WLAN_HAL_8814AE
	if (GET_CHIP_VER(priv) == VERSION_8814A) {
		switch (type) {
		//4 CCK
		case 0:
			offset = EEPROM_2G_CCK1T_TxPower + PATHA_OFFSET;
			break;
		case 1:
			offset = EEPROM_2G_CCK1T_TxPower + PATHB_OFFSET;
			break;
		case 2:
			offset = EEPROM_2G_CCK1T_TxPower + PATHC_OFFSET;
			break;
		case 3:
			offset = EEPROM_2G_CCK1T_TxPower + PATHD_OFFSET;
			break;		
		//4 2G HT40 
		case 4:
			offset = EEPROM_2G_HT401S_TxPower + PATHA_OFFSET;
			break;
		case 5:
			offset = EEPROM_2G_HT401S_TxPower + PATHB_OFFSET;
			break;
		case 6:
			offset = EEPROM_2G_HT401S_TxPower + PATHC_OFFSET;
			break;
		case 7:
			offset = EEPROM_2G_HT401S_TxPower + PATHD_OFFSET;
			break;
		//4 N-mode 2G DIFF
		case 8:
			offset = EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 9:
			offset = EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 10:
			offset = EEPROM_2G_OFDM1T_TxPowerDiff + PATHA_OFFSET;
			break;
		case 11:
			offset = EEPROM_8812_MACADDRESS;
			break;
		case 12:
			offset = 0x00;
			break;
		case 13:
			offset = 0x32;
			break;
		case 14:
			offset = EEPROM_8812_THERMAL_METER;
			break;
		//4 5G HT40	
		case 15:
			offset = EEPROM_5G_HT401S_TxPower + PATHA_OFFSET;
			break;
		case 16:
			offset = EEPROM_5G_HT401S_TxPower + PATHB_OFFSET;
			break;
		case 17:
			offset = EEPROM_5G_HT401S_TxPower + PATHC_OFFSET;
			break;
		case 18:
			offset = EEPROM_5G_HT401S_TxPower + PATHD_OFFSET;
			break;			
		case 24:
			offset = EEPROM_8812_XTAL_K;
			break;
		//4 AC-mode 5G DIFF
		case 25:
			offset = EEPROM_5G_HT201S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 26:
			offset = EEPROM_5G_HT201S_TxPowerDiff + PATHB_OFFSET;
			break;
		case 27:
			offset = EEPROM_5G_HT201S_TxPowerDiff + PATHC_OFFSET;
			break;
		case 28:
			offset = EEPROM_5G_HT201S_TxPowerDiff + PATHD_OFFSET;
			break;
		//4 40M 20M 2S
		case 29:
			offset = EEPROM_5G_HT402S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 30:
			offset = EEPROM_5G_HT402S_TxPowerDiff + PATHB_OFFSET;
			break;
		case 31:
			offset = EEPROM_5G_HT402S_TxPowerDiff + PATHC_OFFSET;
			break;
		case 32:
			offset = EEPROM_5G_HT402S_TxPowerDiff + PATHD_OFFSET;
			break;
		//4 40M 20M 3S
		case 33:
			offset = EEPROM_5G_HT403S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 34:
			offset = EEPROM_5G_HT403S_TxPowerDiff + PATHB_OFFSET;
			break;
		case 35:
			offset = EEPROM_5G_HT403S_TxPowerDiff + PATHC_OFFSET;
			break;
		case 36:
			offset = EEPROM_5G_HT403S_TxPowerDiff + PATHD_OFFSET;
			break;
		//4 80M 1S
		case 37:
			offset = EEPROM_5G_HT801S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 38:
			offset = EEPROM_5G_HT801S_TxPowerDiff + PATHB_OFFSET;
			break;
		case 39:
			offset = EEPROM_5G_HT801S_TxPowerDiff + PATHC_OFFSET;
			break;
		case 40:
			offset = EEPROM_5G_HT801S_TxPowerDiff + PATHD_OFFSET;
			break;
		//4 80M 2S	
		case 41:
			offset = EEPROM_5G_HT802S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 42:
			offset = EEPROM_5G_HT802S_TxPowerDiff + PATHB_OFFSET;
			break;
		case 43:
			offset = EEPROM_5G_HT802S_TxPowerDiff + PATHC_OFFSET;
			break;
		case 44:
			offset = EEPROM_5G_HT802S_TxPowerDiff + PATHD_OFFSET;
			break;
		//4 80M 3S
		case 45:
			offset = EEPROM_5G_HT803S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 46:
			offset = EEPROM_5G_HT803S_TxPowerDiff + PATHB_OFFSET;
			break;
		case 47:
			offset = EEPROM_5G_HT803S_TxPowerDiff + PATHC_OFFSET;
			break;
		case 48:
			offset = EEPROM_5G_HT803S_TxPowerDiff + PATHD_OFFSET;
			break;				
		//4 AC-mode 2G DIFF	
		case 49:
			offset = EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 50:
			offset = EEPROM_2G_HT201S_TxPowerDiff + PATHB_OFFSET;
			break;
		case 51:
			offset = EEPROM_2G_HT201S_TxPowerDiff + PATHC_OFFSET;
			break;
		case 52:
			offset = EEPROM_2G_HT201S_TxPowerDiff + PATHD_OFFSET;
			break;			
		//4 40M 20M 2S	
		case 53:
			offset = EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 54:
			offset = EEPROM_2G_HT402S_TxPowerDiff + PATHB_OFFSET;
			break;
		case 55:
			offset = EEPROM_2G_HT402S_TxPowerDiff + PATHC_OFFSET;
			break;
		case 56:
			offset = EEPROM_2G_HT402S_TxPowerDiff + PATHD_OFFSET;
			break;
		//4 40M 20M 3S	
		case 57:
			offset = EEPROM_2G_HT403S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 58:
			offset = EEPROM_2G_HT403S_TxPowerDiff + PATHB_OFFSET;
			break;
		case 59:
			offset = EEPROM_2G_HT403S_TxPowerDiff + PATHC_OFFSET;
			break;
		case 60:
			offset = EEPROM_2G_HT403S_TxPowerDiff + PATHD_OFFSET;
			break;
		default:
			offset = -1;
			panic_printk("NOT SUPPORT!!\n");
			break;
		}
	}
#endif


#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B) {
		switch (type) {
		case 0:
			offset = EEPROM_TX_PWR_INX_88E; //HW_TX_POWER_CCK_A
			break;
		case 1:
			offset = EEPROM_TX_PWR_INX_88E + 6; //HW_TX_POWER_HT40_1S_A
			break;
		case 2:
			offset = EEPROM_TX_PWR_INX_88E + 11; //HW_TX_POWER_DIFF_HT20_OFDM
			break;
		case 3:
			offset = EEPROM_MAC_ADDR_88E; // HW_WLAN0_WLAN_ADDR
			break;
		case 4:
			offset = 0x00;
			break;
		case 5:
			offset = 0x32;
			break;
		case 6:
			offset = EEPROM_THERMAL_METER_88E; //HW_11N_THER
			break;
		case 7:
			offset = EEPROM_XTAL_88E; //HW_11N_XCAP
			break;
#ifdef CONFIG_SDIO_HCI
		case 8:
			offset = EEPROM_ChannelPlan_88E; //HW_REG_DOMAIN
			break;
#endif
		default:
			offset = -1;
			panic_printk("NOT SUPPORT!!\n");
			break;
		}
	}
#endif

#if defined(CONFIG_WLAN_HAL_8192EE)
	if(GET_CHIP_VER(priv) == VERSION_8192E)
	{
		switch(type)
		{
		case 0:		//	HW_TX_POWER_CCK_A
			offset = EEPROM_2G_CCK1T_TxPower + PATHA_OFFSET;
				break;
		case 1:		//	HW_TX_POWER_CCK_B
			offset = EEPROM_2G_CCK1T_TxPower + PATHB_OFFSET;
				break;
		case 2:		//	HW_TX_POWER_HT40_1S_A
			offset = EEPROM_2G_HT401S_TxPower + PATHA_OFFSET;
				break;
		case 3:		//	HW_TX_POWER_HT40_1S_B
			offset = EEPROM_2G_HT401S_TxPower + PATHB_OFFSET;
				break;
		case 4:		//	HW_TX_POWER_DIFF_HT20_A
			offset = EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET;
				break;
		case 5:		//	HW_TX_POWER_DIFF_HT20_B
			offset = EEPROM_2G_HT201S_TxPowerDiff + PATHB_OFFSET;
				break;
		case 6:		//	HW_TX_POWER_DIFF_HT20_OFDM_A
			offset = EEPROM_2G_OFDM1T_TxPowerDiff + PATHA_OFFSET;
				break;
		case 7:		//	HW_TX_POWER_DIFF_HT20_OFDM_B
			offset = EEPROM_2G_OFDM1T_TxPowerDiff + PATHB_OFFSET;
			break;
		case 8:		//	HW_TX_POWER_DIFF_HT40_2S_A
			offset = EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET;
			break;
		case 9:		//	HW_TX_POWER_DIFF_HT40_2S_B
			offset = EEPROM_2G_HT402S_TxPowerDiff + PATHB_OFFSET;
			break;			
		case 10:		//	HW_WLAN0_WLAN_ADDR
			offset = EEPROM_92E_MACADDRESS;
			break;
		case 11:		//	EFUSE_MAP0
				offset = 0x00;
				break;
		case 12:		//	EFUSE_MAP1
				offset = 0x32;
				break;
		case 13:		//	HW_11N_THER
			offset = EEPROM_92E_THERMAL_METER;
				break;
		case 14:		//	HW_11N_XCAP
			offset = EEPROM_92E_XTAL_K;
			break;
		case 15:		//	EFUSE_TEST
			offset = 0x02;
			break;
		case 16:		//	EFUSE_LNA_TYPE
			offset = EEPROM_RFE_OPTION_8192E;
			break;	
#ifdef CONFIG_SDIO_HCI
		case 17:
			offset = EEPROM_92E_SDIOTYPE;
			break;
#endif
		default:
			offset = -1;
			panic_printk("NOT SUPPORT!!\n");
			break;
		}
	}
#endif
	return offset;
}


/*  11/16/2008 MH Add description. Get current efuse area enabled word!!. */
static UINT8 efuse_CalculateWordCnts( UINT8	word_en)
{
	UINT8 word_cnts = 0;
	if (!(word_en & BIT(0)))	word_cnts++; // 0 : write enable
	if (!(word_en & BIT(1)))	word_cnts++;
	if (!(word_en & BIT(2)))	word_cnts++;
	if (!(word_en & BIT(3)))	word_cnts++;
	return word_cnts;
}	// efuse_CalculateWordCnts


/*-----------------------------------------------------------------------------
 * Function:	efuse_GetCurrentSize
 *
 * Overview:	Get current efuse size!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static UINT16 efuse_GetCurrentSize(struct rtl8192cd_priv *priv)
{
	INT32 bContinual = TRUE;
	UINT16 efuse_addr = 0;
	UINT8 hoffset = 0, hworden = 0;
	UINT8 efuse_data, word_cnts = 0;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_RTL_8723B_SUPPORT)
	UINT8 bSupportExtendHdr = ((GET_CHIP_VER(priv) == VERSION_8192D) || (GET_CHIP_VER(priv) == VERSION_8188E) || (GET_CHIP_VER(priv) == VERSION_8192E)
			|| (GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8723B));
#endif


	do	{
		ReadEFuseByte(priv, efuse_addr, &efuse_data) ;
		if (efuse_data != 0xFF) {
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_RTL_8723B_SUPPORT)
			if (bSupportExtendHdr && ((efuse_data & 0x1F) == 0x0F)) {	//extended header
				hoffset = efuse_data;
				efuse_addr++;
				ReadEFuseByte(priv, efuse_addr , &efuse_data);
				if ((efuse_data & 0x0F) == 0x0F) {
					efuse_addr++;
					continue;
				} else {
					hoffset = ((hoffset & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
					hworden = efuse_data & 0x0F;
				}
			} else
#endif
			{
				hoffset = (efuse_data >> 4) & 0x0F;
				hworden =  efuse_data & 0x0F;
			}
			word_cnts = efuse_CalculateWordCnts(hworden);
			//read next header
			efuse_addr = efuse_addr + (word_cnts * 2) + 1;
		} else {
			bContinual = FALSE ;
		}
	} while (bContinual  && (efuse_addr  < priv->EfuseRealContentLen) );

	return efuse_addr;

}	// efuse_GetCurrentSize}


/*-----------------------------------------------------------------------------
 * Function:	efuse_WordEnableDataRead
 *
 * Overview:	Read allowed word in current efuse section data.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Create Version 0.
 * 11/21/2008 	MHC		Fix Write bug when we only enable late word.
 *
 *---------------------------------------------------------------------------*/
static void efuse_WordEnableDataRead(UINT8 word_en, UINT8 *sourdata, UINT8 *targetdata)
{
	if (!(word_en & BIT(0)))	{
		targetdata[0] = sourdata[0];
		targetdata[1] = sourdata[1];
	}
	if (!(word_en & BIT(1)))	{
		targetdata[2] = sourdata[2];
		targetdata[3] = sourdata[3];
	}
	if (!(word_en & BIT(2)))	{
		targetdata[4] = sourdata[4];
		targetdata[5] = sourdata[5];
	}
	if (!(word_en & BIT(3)))	{
		targetdata[6] = sourdata[6];
		targetdata[7] = sourdata[7];
	}
}	// efuse_WordEnableDataRead


/*-----------------------------------------------------------------------------
 * Function:	efuse_PgPacketRead
 *
 * Overview:	Receive dedicated Efuse are content. For92s, we support 16
 *				area now. It will return 8 bytes content for every area.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Reorganize code Arch and assign as local API.
 *
 *---------------------------------------------------------------------------*/
static INT32 efuse_PgPacketRead(struct rtl8192cd_priv *priv, UINT8 offset, UINT8 *data)
{
	UINT8 ReadState = PG_STATE_HEADER;
	INT32 bContinual = TRUE, bDataEmpty = TRUE ;
	UINT16 efuse_addr = 0;
	UINT8 hoffset = 0, hworden = 0, efuse_data, word_cnts = 0, tmpidx = 0;
	UINT8 tmpdata[8];
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	UINT8 tmp_header;
	UINT8 bSupportExtendHdr = ((GET_CHIP_VER(priv) == VERSION_8192D) || (GET_CHIP_VER(priv) == VERSION_8188E)
			|| (GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8723B));
#endif

	if (data == NULL)
		return FALSE;
	if (offset > priv->EfuseMaxSection)
		return FALSE;

	memset(data, 0xff, sizeof(UINT8)*PGPKT_DATA_SIZE);
	memset(tmpdata, 0xff, sizeof(UINT8)*PGPKT_DATA_SIZE);

	//
	// <Roger_TODO> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	// Skip dummy parts to prevent unexpected data read from Efuse.
	// By pass right now. 2009.02.19.
	//
	while (bContinual && (efuse_addr  < priv->EfuseRealContentLen) )	{
		//-------  Header Read -------------
		if (ReadState & PG_STATE_HEADER)		{
			ReadEFuseByte(priv, efuse_addr , &efuse_data);
			if (efuse_data != 0xFF) {
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
				if (bSupportExtendHdr && ((efuse_data & 0x1F) == 0x0F)) {
					tmp_header = efuse_data;
					efuse_addr++;
					ReadEFuseByte(priv, efuse_addr , &efuse_data);
					if ((efuse_data & 0x0F) != 0x0F) {
						hoffset = ((tmp_header & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
						hworden = efuse_data & 0x0F;
					} else {
						efuse_addr++;
						continue;
					}

				} else
#endif
				{
					hoffset = (efuse_data >> 4) & 0x0F;
					hworden =  efuse_data & 0x0F;
				}
				word_cnts = efuse_CalculateWordCnts(hworden);
				bDataEmpty = TRUE ;

				if (hoffset == offset) {
					for (tmpidx = 0; tmpidx < word_cnts * 2 ; tmpidx++) {
						ReadEFuseByte(priv, efuse_addr + 1 + tmpidx , &efuse_data);
						tmpdata[tmpidx] = efuse_data;
						if (efuse_data != 0xff) {
							bDataEmpty = FALSE;
						}
					}
					if (bDataEmpty == FALSE) {
						ReadState = PG_STATE_DATA;
					} else { //read next header
						efuse_addr = efuse_addr + (word_cnts * 2) + 1;
						ReadState = PG_STATE_HEADER;
					}
				} else { //read next header
					efuse_addr = efuse_addr + (word_cnts * 2) + 1;
					ReadState = PG_STATE_HEADER;
				}
			} else {
				bContinual = FALSE ;
			}
		}
		//-------  Data section Read -------------
		else if (ReadState & PG_STATE_DATA)	{
			efuse_WordEnableDataRead(hworden, tmpdata, data);
			efuse_addr = efuse_addr + (word_cnts * 2) + 1;
			ReadState = PG_STATE_HEADER;
		}
	}
	if (	(data[0] == 0xff) && (data[1] == 0xff) && (data[2] == 0xff)  && (data[3] == 0xff) &&
			(data[4] == 0xff) && (data[5] == 0xff) && (data[6] == 0xff)  && (data[7] == 0xff))
		return FALSE;
	else
		return TRUE;

}	// efuse_PgPacketRead


/*  11/16/2008 MH Write one byte to reald Efuse. */
static INT32 WriteEFuseByte(struct rtl8192cd_priv *priv, UINT16 addr, UINT8 data)
{
	UINT8 tmpidx = 0;
	INT32 bResult;
	u4Byte efuseValue = 0;

//	DEBUG_INFO("Addr = %x Data=%x\n", addr, data);

	// -----------------e-fuse reg ctrl ---------------------------------
	//address
#if defined(CONFIG_WLAN_HAL_8192EE)
	if( GET_CHIP_VER(priv) == VERSION_8192E && !IS_TEST_CHIP(priv) && IS_HARDWARE_TYPE_8192E(priv) && 
		(_GET_HAL_DATA(priv)->cutVersion != ODM_CUT_A) )
	{
		//0x34[11]: SW force PGMEN input of efuse to high. (for the bank selected by 0x34[9:8])
		RTL_W8(EFUSE_TEST + 1, (RTL_R8(EFUSE_TEST + 1) & 0xf7) | (0x01 << 3));	
		RTL_W32(EFUSE_CTRL, 0x90600000|(addr<<8 | data));		
	}
	else
#endif		
	{
		efuseValue = RTL_R32(EFUSE_CTRL);		
		efuseValue |= (BIT21|BIT31);
		efuseValue &= ~(0x3FFFF);
		efuseValue |= ((addr<<8 | data) & 0x3FFFF);		
		RTL_W32(EFUSE_CTRL, efuseValue);

#if 0
	RTL_W8( EFUSE_CTRL + 1, (UINT8)(addr & 0xff));
	RTL_W8( EFUSE_CTRL + 2, 	(RTL_R8(EFUSE_CTRL + 2) & 0xFC ) | (UINT8)((addr >> 8) & 0x03));
	RTL_W8( EFUSE_CTRL, data);//data

	RTL_W8( EFUSE_CTRL + 3, 0xF2	); //write cmd
#endif
	}

	while ((0x80 &  RTL_R8(EFUSE_CTRL + 3)) && (tmpidx < 100) ) {
		delay_ms(1);
		tmpidx++;
	}

	if (tmpidx < 100)	{
		bResult = TRUE;
	} else	{
		bResult = FALSE;
	}

#if defined(CONFIG_WLAN_HAL_8192EE)	
	if( GET_CHIP_VER(priv) == VERSION_8192E )
	{
		if (!IS_TEST_CHIP(priv) && IS_HARDWARE_TYPE_8192E(priv) && 
			( _GET_HAL_DATA(priv)->cutVersion != ODM_CUT_A ) )
		{
			//0x34[11]: SW force PGMEN input of efuse to high. (for the bank selected by 0x34[9:8])		
			RTL_W8(EFUSE_TEST + 1, (RTL_R8(EFUSE_TEST + 1) & 0xf7) | (0x00 << 3));		
		}
	}
#endif	

	return bResult;
}	//


/*-----------------------------------------------------------------------------
 * Function:	efuse_WordEnableDataWrite
 *
 * Overview:	Write necessary word unit into current efuse section!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Reorganize Efuse operate flow!!.
 *
 *---------------------------------------------------------------------------*/
static UINT8 efuse_WordEnableDataWrite(struct rtl8192cd_priv *priv,
									   UINT16 	efuse_addr, UINT8 word_en, UINT8 *data)
{
	UINT16 tmpaddr = 0;
	UINT16 start_addr = efuse_addr;
	UINT8 badworden = 0x0F;
	UINT8 tmpdata[8];

	//memset(tmpdata,0xff,PGPKT_DATA_SIZE);
	memset(tmpdata,  0xff, PGPKT_DATA_SIZE);
	DEBUG_INFO("word_en = %x efuse_addr=%x\n", word_en, efuse_addr);

	if (!(word_en & BIT(0)))	{
		tmpaddr = start_addr;
		WriteEFuseByte(priv, start_addr++, data[0]);
		WriteEFuseByte(priv, start_addr++, data[1]);
		ReadEFuseByte(priv, tmpaddr, &tmpdata[0]);
		ReadEFuseByte(priv, tmpaddr + 1, &tmpdata[1]);
		if ((data[0] != tmpdata[0]) || (data[1] != tmpdata[1])) {
			badworden &= (~ BIT(0));
		}
	}
	if (!(word_en & BIT(1)))	{
		tmpaddr = start_addr;
		WriteEFuseByte(priv, start_addr++, data[2]);
		WriteEFuseByte(priv, start_addr++, data[3]);
		ReadEFuseByte(priv, tmpaddr    , &tmpdata[2]);
		ReadEFuseByte(priv, tmpaddr + 1, &tmpdata[3]);
		if ((data[2] != tmpdata[2]) || (data[3] != tmpdata[3])) {
			badworden &= ( ~ BIT(1));
		}
	}
	if (!(word_en & BIT(2)))	{
		tmpaddr = start_addr;
		WriteEFuseByte(priv, start_addr++, data[4]);
		WriteEFuseByte(priv, start_addr++, data[5]);
		ReadEFuseByte(priv, tmpaddr, &tmpdata[4]);
		ReadEFuseByte(priv, tmpaddr + 1, &tmpdata[5]);
		if ((data[4] != tmpdata[4]) || (data[5] != tmpdata[5])) {
			badworden &= ( ~ BIT(2));
		}
	}
	if (!(word_en & BIT(3))) {
		tmpaddr = start_addr;
		WriteEFuseByte(priv, start_addr++, data[6]);
		WriteEFuseByte(priv, start_addr++, data[7]);
		ReadEFuseByte(priv, tmpaddr, &tmpdata[6]);
		ReadEFuseByte(priv, tmpaddr + 1, &tmpdata[7]);
		if ((data[6] != tmpdata[6]) || (data[7] != tmpdata[7])) {
			badworden &= ( ~ BIT(3));
		}
	}
	return badworden;
}	// efuse_WordEnableDataWrite


//
//	Description:
//		This routine will calculate current shadow map that
//		how much bytes needs to be updated.
//
//	Assumption:
//		We shall call this routine before programming physical Efuse content.
//
//	Return Value:
//		TRUE: Efuse has enough capacity to program.
//		FALSE: Efuse do NOT has enough capacity to program.
//
//	Created by Roger, 2008.04.21.
//
static int EFUSE_ShadowUpdateChk(struct rtl8192cd_priv *priv)
{
	UINT8	SectionIdx, i, Base;
	UINT16	WordsNeed = 0, HdrNum = 0, TotalBytes = 0, EfuseUsed = 0;
	UINT8	bWordChanged, bResult = TRUE;

	// Efuse contain total 16 sections.
	for (SectionIdx = 0; SectionIdx < priv->EfuseMaxSection; SectionIdx++)	{
		Base = SectionIdx * 8;
		bWordChanged = FALSE;

		// One section contain 4 words = 8 bytes.
		for (i = 0; i < 8; i = i + 2)		{
			if ((priv->EfuseMap[EFUSE_INIT_MAP][Base + i] != priv->EfuseMap[EFUSE_MODIFY_MAP][Base + i]) ||
					(priv->EfuseMap[EFUSE_INIT_MAP][Base + i + 1] != priv->EfuseMap[EFUSE_MODIFY_MAP][Base + i + 1])) {
				WordsNeed++;
				bWordChanged = TRUE;
			}
		}

		// We shall append Efuse header If any WORDs changed in this section.
		if (bWordChanged == TRUE) {
			if (SectionIdx >= EFUSE_MAX_SECTION_BASE)
				HdrNum += 2;
			else
				HdrNum++;
		}
	}

	TotalBytes = HdrNum + WordsNeed * 2;
	EfuseUsed = priv->EfuseUsedBytes;

	// Calculate whether updated map has enough capacity.
	if ((TotalBytes + EfuseUsed) >= (priv->EfuseRealContentLen - priv->EfuseOobProtectBytes))
		bResult = FALSE;

	DEBUG_INFO("EFUSE_ShadowUpdateChk(): TotalBytes(%#x), HdrNum(%#x), WordsNeed(%#x), EfuseUsed(%d)\n",
			   TotalBytes, HdrNum, WordsNeed, EfuseUsed);

	return bResult;
}


/*-----------------------------------------------------------------------------
 * Function:	efuse_PgPacketWrite
 *
 * Overview:	Send A G package for different section in real efuse area.
 *				For 92S, One PG package contain 8 bytes content and 4 word
 *				unit. PG header = 0x[bit7-4=offset][bit3-0word enable]
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008 	MHC		Reorganize code Arch and assign as local API.
 *
 *---------------------------------------------------------------------------*/
static int efuse_PgPacketWrite(struct rtl8192cd_priv *priv,
							   UINT8 offset, UINT8 word_en, UINT8 *data)
{
	UINT8 WriteState = PG_STATE_HEADER;
	INT32 bContinual = TRUE, bDataEmpty = TRUE, bResult = TRUE, bExtendedHeader = FALSE;
	UINT16 efuse_addr = 0;
	UINT8 efuse_data, pg_header = 0, pg_header_temp = 0;

	UINT8 tmp_word_cnts = 0, target_word_cnts = 0;
	UINT8 tmp_header, match_word_en, tmp_word_en;

	PGPKT_STRUCT target_pkt;
	PGPKT_STRUCT tmp_pkt;

	UINT8 originaldata[sizeof(UINT8) * 8];
	UINT8 tmpindex = 0, badworden = 0x0F;
	static INT32 repeat_times = 0;

	//
	// <Roger_Notes> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	// So we have to prevent unexpected data string connection, which will cause
	// incorrect data auto-load from HW. The total size is equal or smaller than 498bytes
	// (i.e., offset 0~497, and dummy 1bytes) expected after CP test.
	// 2009.02.19.
	//
	if ( efuse_GetCurrentSize(priv) >= (priv->EfuseRealContentLen - priv->EfuseOobProtectBytes))	{
		DEBUG_INFO ("efuse_PgPacketWrite error \n");
		return FALSE;
	}

	tmp_pkt.offset = 0;
	tmp_pkt.word_en = 0;
	// Init the 8 bytes content as 0xff
	target_pkt.offset = offset;
	target_pkt.word_en = word_en;
	memset(target_pkt.data,  0xFF, sizeof(UINT8) * 8);

	efuse_WordEnableDataRead(word_en, data, target_pkt.data);
	target_word_cnts = efuse_CalculateWordCnts(target_pkt.word_en);

	//
	// <Roger_Notes> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP.
	// So we have to prevent unexpected data string connection, which will cause
	// incorrect data auto-load from HW. Dummy 1bytes is additional.
	// 2009.02.19.
	//
	while ( bContinual && (efuse_addr  < (priv->EfuseRealContentLen - priv->EfuseOobProtectBytes)) )	{
		if (WriteState == PG_STATE_HEADER)		{
			bDataEmpty = TRUE;
			badworden = 0x0F;
			//************  so *******************
			DEBUG_INFO("EFUSE PG_STATE_HEADER\n");
			ReadEFuseByte(priv, efuse_addr , &efuse_data);
			if (efuse_data != 0xFF) {
				if ((efuse_data & 0x1F) == 0x0F) {	//extended header
					tmp_header = efuse_data;
					efuse_addr++;
					ReadEFuseByte(priv, efuse_addr , &efuse_data);
					if ((efuse_data & 0x0F) == 0x0F) {	//wren fail
						efuse_addr++;
						continue;
					} else {
						tmp_pkt.offset = ((tmp_header & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
						tmp_pkt.word_en = efuse_data & 0x0F;
					}
				} else {
					tmp_header  =  efuse_data;
					tmp_pkt.offset 	= (tmp_header >> 4) & 0x0F;
					tmp_pkt.word_en 	= tmp_header & 0x0F;
				}

				tmp_word_cnts =  efuse_CalculateWordCnts(tmp_pkt.word_en);

				//************  so-1 *******************
				if (tmp_pkt.offset  != target_pkt.offset)				{
					efuse_addr = efuse_addr + (tmp_word_cnts * 2) + 1; //Next pg_packet
#ifdef EFUSE_ERROE_HANDLE
					WriteState = PG_STATE_HEADER;
#endif
				} else	{
					//************  so-2 *******************
					for (tmpindex = 0 ; tmpindex < (tmp_word_cnts * 2) ; tmpindex++)	{
						ReadEFuseByte(priv, (efuse_addr + 1 + tmpindex) , &efuse_data);
						if (efuse_data != 0xFF)
							bDataEmpty = FALSE;
					}
					//************  so-2-1 *******************
					if (bDataEmpty == FALSE)	{
						efuse_addr = efuse_addr + (tmp_word_cnts * 2) + 1; //Next pg_packet
#ifdef EFUSE_ERROE_HANDLE
						WriteState = PG_STATE_HEADER;
#endif
					} else {
						//************  so-2-2 *******************
						match_word_en = 0x0F;
						if (   !( (target_pkt.word_en & BIT(0)) | (tmp_pkt.word_en & BIT(0))  ))	{
							match_word_en &= (~ BIT(0));
						}
						if (   !( (target_pkt.word_en & BIT(1)) | (tmp_pkt.word_en & BIT(1))  ))	{
							match_word_en &= (~ BIT(1));
						}
						if (   !( (target_pkt.word_en & BIT(2)) | (tmp_pkt.word_en & BIT(2))  ))	{
							match_word_en &= (~ BIT(2));
						}
						if (   !( (target_pkt.word_en & BIT(3)) | (tmp_pkt.word_en & BIT(3))  ))	{
							match_word_en &= (~ BIT(3));
						}

						//************  so-2-2-A *******************
						if ((match_word_en & 0x0F) != 0x0F)		{
							badworden = efuse_WordEnableDataWrite(priv, efuse_addr + 1, tmp_pkt.word_en , target_pkt.data);

							//************  so-2-2-A-1 *******************
							if (0x0F != (badworden & 0x0F))	{
								UINT8 reorg_offset = offset;
								UINT8 reorg_worden = badworden;
								efuse_PgPacketWrite(priv, reorg_offset, reorg_worden, originaldata);
							}

							tmp_word_en = 0x0F;
							if (  (target_pkt.word_en & BIT(0)) ^ (match_word_en & BIT(0))  )	{
								tmp_word_en &= (~ BIT(0));
							}
							if (   (target_pkt.word_en & BIT(1)) ^ (match_word_en & BIT(1)) )	{
								tmp_word_en &=  (~ BIT(1));
							}
							if (   (target_pkt.word_en & BIT(2)) ^ (match_word_en & BIT(2)) )	{
								tmp_word_en &= (~ BIT(2));
							}
							if (   (target_pkt.word_en & BIT(3)) ^ (match_word_en & BIT(3)) )	{
								tmp_word_en &= (~ BIT(3));
							}

							//************  so-2-2-A-2 *******************
							if ((tmp_word_en & 0x0F) != 0x0F) {
								//reorganize other pg packet
								efuse_addr = efuse_GetCurrentSize(priv);
								target_pkt.offset = offset;
								target_pkt.word_en = tmp_word_en;
							} else {
								bContinual = FALSE;
							}
#ifdef EFUSE_ERROE_HANDLE
							WriteState = PG_STATE_HEADER;
							repeat_times++;
							if (repeat_times > EFUSE_REPEAT_THRESHOLD_) {
								bContinual = FALSE;
								bResult = FALSE;
							}
#endif
						} else { //************  so-2-2-B *******************
							//reorganize other pg packet
							efuse_addr = efuse_addr + (2 * tmp_word_cnts) + 1; //next pg packet addr
							target_pkt.offset = offset;
							target_pkt.word_en = target_pkt.word_en;
#ifdef EFUSE_ERROE_HANDLE
							WriteState = PG_STATE_HEADER;
#endif
						}
					}
				}
				DEBUG_INFO("EFUSE PG_STATE_HEADER-1\n");
			} else {
				//************  s1: header == oxff  *******************
				bExtendedHeader = FALSE;

				if (target_pkt.offset >= EFUSE_MAX_SECTION_BASE) {
					pg_header = ((target_pkt.offset & 0x07) << 5) | 0x0F;

					DEBUG_INFO("efuse_PgPacketWrite extended pg_header[2:0] |0x0F 0x%x \n", pg_header);

					WriteEFuseByte(priv, efuse_addr, pg_header);
					ReadEFuseByte(priv, efuse_addr, &tmp_header);

					while (tmp_header == 0xFF) {
						DEBUG_INFO("efuse_PgPacketWrite extended pg_header[2:0] wirte fail \n");

						repeat_times++;

						if (repeat_times > EFUSE_REPEAT_THRESHOLD_) {
							bContinual = FALSE;
							bResult = FALSE;
							efuse_addr++;
							break;
						}
						WriteEFuseByte(priv, efuse_addr, pg_header);
						ReadEFuseByte(priv, efuse_addr, &tmp_header);
					}

					if (!bContinual)
						break;

					if (tmp_header == pg_header) {
						efuse_addr++;
						pg_header_temp = pg_header;
						pg_header = ((target_pkt.offset & 0x78) << 1 ) | target_pkt.word_en;

						DEBUG_INFO("efuse_PgPacketWrite extended pg_header[6:3] | worden 0x%x word_en 0x%x \n", pg_header, target_pkt.word_en);

						WriteEFuseByte(priv, efuse_addr, pg_header);
						ReadEFuseByte(priv, efuse_addr, &tmp_header);

						while (tmp_header == 0xFF) {
							repeat_times++;

							if (repeat_times > EFUSE_REPEAT_THRESHOLD_) {
								bContinual = FALSE;
								bResult = FALSE;
								break;
							}
							WriteEFuseByte(priv, efuse_addr, pg_header);
							ReadEFuseByte(priv, efuse_addr, &tmp_header);
						}

						if (!bContinual)
							break;

						if ((tmp_header & 0x0F) == 0x0F) {	//wren PG fail
							repeat_times++;

							if (repeat_times > EFUSE_REPEAT_THRESHOLD_) {
								bContinual = FALSE;
								bResult = FALSE;
								break;
							} else {
								efuse_addr++;
								continue;
							}
						} else if (pg_header != tmp_header) {	//offset PG fail
							bExtendedHeader = TRUE;
							tmp_pkt.offset = ((pg_header_temp & 0xE0) >> 5) | ((tmp_header & 0xF0) >> 1);
							tmp_pkt.word_en =  tmp_header & 0x0F;
							tmp_word_cnts =  efuse_CalculateWordCnts(tmp_pkt.word_en);
						}
					} else if ((tmp_header & 0x1F) == 0x0F) {	//wrong extended header
						efuse_addr += 2;
						continue;
					}
				} else {
					pg_header = ((target_pkt.offset << 4) & 0xf0) | target_pkt.word_en;
					WriteEFuseByte(priv, efuse_addr, pg_header);
					ReadEFuseByte(priv, efuse_addr, &tmp_header);
				}
				if (tmp_header == pg_header)	{ //************  s1-1*******************
					WriteState = PG_STATE_DATA;
				}	else
#ifdef EFUSE_ERROE_HANDLE
					if (tmp_header == 0xFF) { //************  s1-3: if Write or read func doesn't work *******************
						//efuse_addr doesn't change
						WriteState = PG_STATE_HEADER;
						repeat_times++;
						if (repeat_times > EFUSE_REPEAT_THRESHOLD_) {
							bContinual = FALSE;
							bResult = FALSE;
						}
					} else
#endif
					{
						//************  s1-2 : fixed the header procedure *******************
						if (!bExtendedHeader) {
							tmp_pkt.offset = (tmp_header >> 4) & 0x0F;
							tmp_pkt.word_en =  tmp_header & 0x0F;
							tmp_word_cnts =  efuse_CalculateWordCnts(tmp_pkt.word_en);
						}

						//************  s1-2-A :cover the exist data *******************
						//memset(originaldata,0xff,sizeof(UINT8)*8);
						memset(originaldata, 0xff, sizeof(UINT8) * 8);

						if (efuse_PgPacketRead( priv, tmp_pkt.offset, originaldata))	{	//check if data exist
							badworden = efuse_WordEnableDataWrite(priv, efuse_addr + 1, tmp_pkt.word_en, originaldata);
							if (0x0F != (badworden & 0x0F))						{
								UINT8 reorg_offset = tmp_pkt.offset;
								UINT8 reorg_worden = badworden;
								efuse_PgPacketWrite(priv, reorg_offset, reorg_worden, originaldata);
								efuse_addr = efuse_GetCurrentSize(priv);
							} else {
								efuse_addr = efuse_addr + (tmp_word_cnts * 2) + 1; //Next pg_packet
							}
						} else	{
							//************  s1-2-B: wrong address*******************
							efuse_addr = efuse_addr + (tmp_word_cnts * 2) + 1; //Next pg_packet
						}

#ifdef EFUSE_ERROE_HANDLE
						WriteState = PG_STATE_HEADER;
						repeat_times++;
						if (repeat_times > EFUSE_REPEAT_THRESHOLD_) {
							bContinual = FALSE;
							bResult = FALSE;
						}
#endif
						DEBUG_INFO("EFUSE PG_STATE_HEADER-2\n");
					}

			}
		}
		//write data state
		else if (WriteState == PG_STATE_DATA) {
			//************  s1-1  *******************
			DEBUG_INFO("EFUSE PG_STATE_DATA\n");
			badworden = 0x0f;
			badworden = efuse_WordEnableDataWrite(priv, efuse_addr + 1, target_pkt.word_en, target_pkt.data);
			if ((badworden & 0x0F) == 0x0F) {
				//************  s1-1-A *******************
				bContinual = FALSE;
			} else {
				//reorganize other pg packet //************  s1-1-B *******************
				efuse_addr = efuse_addr + (2 * target_word_cnts) + 1; //next pg packet addr
				//===========================
				target_pkt.offset = offset;
				target_pkt.word_en = badworden;
				target_word_cnts =  efuse_CalculateWordCnts(target_pkt.word_en);
				//===========================
#ifdef EFUSE_ERROE_HANDLE
				WriteState = PG_STATE_HEADER;
				repeat_times++;
				if (repeat_times > EFUSE_REPEAT_THRESHOLD_) {
					bContinual = FALSE;
					bResult = FALSE;
				}
#endif
				DEBUG_INFO("EFUSE PG_STATE_HEADER-3\n");
			}
		}
	}

	if (efuse_addr  >= (priv->EfuseRealContentLen - priv->EfuseOobProtectBytes)) 	{
		DEBUG_INFO("efuse_PgPacketWrite(): efuse_addr(%#x) Out of size!!\n", efuse_addr);
	}

	return TRUE;
}	// efuse_PgPacketWrite


/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowUpdate
 *
 * Overview:	Compare init and modify map to update Efuse!!!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008 	MHC		Create Version 0.
 * 12/11/2008	MHC		92SE PH workaround to prevent HW autoload fail.
 * 12/30/2008	Roger	Fix the bug that EFUSE will writed out-of boundary.
 * 02/16/2009	Roger	Revise PCIe autoload fail case and compatible with USB interface to
 *						overcome MP issue.
 *
 *---------------------------------------------------------------------------*/
static int EFUSE_ShadowUpdate(struct rtl8192cd_priv *priv)
{
	UINT16			i, offset, base = 0;
	UINT8			word_en = 0x0F;

	//
	// <Roger_Notes> We have to check whether current Efuse capacity is big enough to program!!
	// 2009.04.21.
	//
	if (!EFUSE_ShadowUpdateChk(priv))	{
		//
		// <Roger_Notes> We shall reload current Efuse all map and synchronize current modified
		// map to prevent inconsistent Efuse content.
		// 2009.04.21.
		//
		EFUSE_ShadowMapUpdate(priv);
		DEBUG_INFO("<---EFUSE_ShadowUpdate(): Efuse out of capacity!!\n");
		return FALSE;
	}
	// For Efuse write action, we must enable LDO2.5V and 40MHZ clk.
	efuse_PowerSwitch(priv, TRUE, TRUE);

	//
	// Efuse support 16 write are with PG header packet!!!!
	//
	for (offset = 0; offset < priv->EfuseMaxSection; offset++) 	{
		// From section(0) to section(15) sequential write.
		word_en = 0x0F;
		base = offset * 8;
		//
		// Decide Word Enable Bit for the Efuse section
		// One section contain 4 words = 8 bytes!!!!!
		//
		for (i = 0; i < 8; i++)		{
			if (priv->EfuseMap[EFUSE_INIT_MAP][base + i] != priv->EfuseMap[EFUSE_MODIFY_MAP][base + i]) 	{
				word_en &= ~(BIT(i / 2));
				DEBUG_INFO("Section(%#x) Addr[%x] %x update to %x, Word_En=%02x\n",
						   offset, base + i, priv->EfuseMap[EFUSE_INIT_MAP][base + i],	priv->EfuseMap[EFUSE_MODIFY_MAP][base + i], word_en);
			}
		}

		//
		// This section will need to update, call Efuse real write section !!!!
		//
		if (word_en != 0x0F)	{
			UINT8	tmpdata[8];
			memcpy(tmpdata, (&priv->EfuseMap[EFUSE_MODIFY_MAP][base]), 8);

			//
			// <Roger_Notes> Break programming process if efuse capacity is NOT available.
			// 2009.04.20.
			//
			if (!efuse_PgPacketWrite(priv, (UINT8)offset, word_en, tmpdata))	{
				DEBUG_INFO("EFUSE_ShadowUpdate(): PG section(%#x) fail!!\n", offset);
				break;
			}
		}
	}

	// For warm reboot, we must resume Efuse clock to 500K.
	efuse_PowerSwitch(priv, TRUE, FALSE);

	//
	// <Roger_Notes> We update both init shadow map again and modified map
	// while WPG do loading operation after previous programming.
	// 2008.12.30.
	//
	EFUSE_ShadowMapUpdate(priv);

	ReadTxPowerInfoFromHWPG(priv);
	ReadThermalMeterFromEfuse(priv);
	
#ifdef CONFIG_LNA_FROM_EFUSE
	if (GET_CHIP_VER(priv) == VERSION_8192E)
		ReadLNATypeFromEfuse(priv);
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		ReadDeltaValFromEfuse(priv);
		ReadTRSWPAPEFromEfuse(priv);
	}
#endif

	if ((GET_CHIP_VER(priv) == VERSION_8192D) ||
		(GET_CHIP_VER(priv) == VERSION_8188E) ||
		(GET_CHIP_VER(priv) == VERSION_8723B) ||
		(GET_CHIP_VER(priv) == VERSION_8812E) || 
		(GET_CHIP_VER(priv) == VERSION_8192E) ||
		(GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B))
		ReadCrystalCalibrationFromEfuse(priv);

	DEBUG_INFO ("<---EFUSE_ShadowUpdate()\n");
	return TRUE;
}	// EFUSE_ShadowUpdate

#if 0
static void shadowMapWrite(struct rtl8192cd_priv *priv, int type, char *value, unsigned char *hwinfo)
{
	int  i, offset ;
	offset = getEEPROMOffset(priv, type);
	//printk("[%s] offset=%d\n",__FUNCTION__,offset);

	if (offset < 0)
		return;

#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
		if (offset == EEPROM_MACADDRESS) {
			for (i = 0; i < MACADDRLEN; i++) {
				get_array_val(hwinfo + offset + i, value + i * 2, 2);
			}
		} else if (offset == EEPROM_THERMAL_METER) {
			get_array_val(hwinfo + offset,   value,     2);
		} else {
			get_array_val(hwinfo + offset,   value,     2);
			get_array_val(hwinfo + offset + 1, value + 3 * 2, 2);
			get_array_val(hwinfo + offset + 2, value + 9 * 2, 2);
		}
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		if (offset == 0 || offset == 0x32) {
			for (i = 0; i < 0x32; i++) {
				get_array_val(hwinfo + offset + i, value + i * 2, 2);
			}
		} else if (offset == EEPROM_MAC0_MACADDRESS || offset == EEPROM_MAC1_MACADDRESS) {
			for (i = 0; i < MACADDRLEN; i++) {
				get_array_val(hwinfo + offset + i, value + i * 2, 2);
			}
		} else if (offset == EEPROM_92D_THERMAL_METER ||
				   offset == EEPROM_92D_TRSW_CTRL ||
				   offset == EEPROM_92D_PAPE_CTRL ||
				   offset == EEPROM_92D_XTAL_K ) {
			get_array_val(hwinfo + offset, value, 2);
		} else if ((offset == EEPROM_2G_TxPowerCCK) ||
				   (offset == EEPROM_2G_TxPowerCCK + 3) ||
				   (offset == EEPROM_2G_TxPowerHT40_1S) ||
				   (offset == EEPROM_2G_TxPowerHT40_1S + 3) ||
				   (offset == EEPROM_2G_TxPowerHT40_2SDiff) ||
				   (offset == EEPROM_2G_TxPowerHT20Diff) ||
				   (offset == EEPROM_2G_TxPowerOFDMDiff)) {
			get_array_val(hwinfo + offset,   value,     2);
			get_array_val(hwinfo + offset + 1, value + 3 * 2, 2);
			get_array_val(hwinfo + offset + 2, value + 9 * 2, 2);
		} else if (offset == EEPROM_5GL_TxPowerHT40_1S) {
			get_array_val(hwinfo + EEPROM_5GL_TxPowerHT40_1S,   value + 35 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GL_TxPowerHT40_1S + 1, value + 45 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GL_TxPowerHT40_1S + 2, value + 55 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GM_TxPowerHT40_1S,   value + 99 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GM_TxPowerHT40_1S + 1, value + 113 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GM_TxPowerHT40_1S + 2, value + 127 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GH_TxPowerHT40_1S,   value + 148 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GH_TxPowerHT40_1S + 1, value + 154 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GH_TxPowerHT40_1S + 2, value + 160 * 2, 2);
		} else if (offset == EEPROM_5GL_TxPowerHT40_1S + 3) {
			get_array_val(hwinfo + EEPROM_5GL_TxPowerHT40_1S + 3, value + 35 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GL_TxPowerHT40_1S + 4, value + 45 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GL_TxPowerHT40_1S + 5, value + 55 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GM_TxPowerHT40_1S + 3, value + 99 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GM_TxPowerHT40_1S + 4, value + 113 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GM_TxPowerHT40_1S + 5, value + 127 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GH_TxPowerHT40_1S + 3, value + 148 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GH_TxPowerHT40_1S + 4, value + 154 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GH_TxPowerHT40_1S + 5, value + 160 * 2, 2);
		} else if (offset == EEPROM_5GL_TxPowerHT40_2SDiff) {
			get_array_val(hwinfo + EEPROM_5GL_TxPowerHT40_2SDiff,   value + 35 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GL_TxPowerHT40_2SDiff + 1, value + 45 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GL_TxPowerHT40_2SDiff + 2, value + 55 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GM_TxPowerHT40_2SDiff,   value + 99 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GM_TxPowerHT40_2SDiff + 1, value + 113 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GM_TxPowerHT40_2SDiff + 2, value + 127 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GH_TxPowerHT40_2SDiff,   value + 148 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GH_TxPowerHT40_2SDiff + 1, value + 154 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GH_TxPowerHT40_2SDiff + 2, value + 160 * 2, 2);
		} else if (offset == EEPROM_5GL_TxPowerHT20Diff) {
			get_array_val(hwinfo + EEPROM_5GL_TxPowerHT20Diff,   value + 35 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GL_TxPowerHT20Diff + 1, value + 45 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GL_TxPowerHT20Diff + 2, value + 55 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GM_TxPowerHT20Diff,   value + 99 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GM_TxPowerHT20Diff + 1, value + 113 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GM_TxPowerHT20Diff + 2, value + 127 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GH_TxPowerHT20Diff,   value + 148 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GH_TxPowerHT20Diff + 1, value + 154 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GH_TxPowerHT20Diff + 2, value + 160 * 2, 2);
		} else if (offset == EEPROM_5GL_TxPowerOFDMDiff) {
			get_array_val(hwinfo + EEPROM_5GL_TxPowerOFDMDiff,   value + 35 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GL_TxPowerOFDMDiff + 1, value + 45 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GL_TxPowerOFDMDiff + 2, value + 55 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GM_TxPowerOFDMDiff,   value + 99 * 2,  2);
			get_array_val(hwinfo + EEPROM_5GM_TxPowerOFDMDiff + 1, value + 113 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GM_TxPowerOFDMDiff + 2, value + 127 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GH_TxPowerOFDMDiff,   value + 148 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GH_TxPowerOFDMDiff + 1, value + 154 * 2, 2);
			get_array_val(hwinfo + EEPROM_5GH_TxPowerOFDMDiff + 2, value + 160 * 2, 2);
		}
	}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		if (offset == 0 || offset == 0x32) {
			for (i = 0; i < 0x32; i++) {
				get_array_val(hwinfo + offset + i, value + i * 2, 2);
			}
		} else if (offset == EEPROM_8812_MACADDRESS) {
			for (i = 0; i < MACADDRLEN; i++) {
				get_array_val(hwinfo + offset + i, value + i * 2, 2);
			}
		} else if (offset == EEPROM_8812_THERMAL_METER ||
				   offset == EEPROM_8812_XTAL_K ) {
			get_array_val(hwinfo + offset, value, 2);
		} else if ((offset == EEPROM_2G_CCK1T_TxPower + PATHA_OFFSET) ||
				   (offset == EEPROM_2G_CCK1T_TxPower + PATHB_OFFSET) ||
				   (offset == EEPROM_2G_HT401S_TxPower + PATHA_OFFSET) ||
				   (offset == EEPROM_2G_HT401S_TxPower + PATHB_OFFSET) ||
				   (offset == EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET) ||
				   (offset == EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET) ||
				   (offset == EEPROM_2G_OFDM1T_TxPowerDiff + PATHA_OFFSET)) {
			get_array_val(hwinfo + offset,   value,     2);
			get_array_val(hwinfo + offset + 1, value + 3 * 2, 2);
			get_array_val(hwinfo + offset + 2, value + 9 * 2, 2);
		} else if (offset == EEPROM_5G_HT401S_TxPower + PATHA_OFFSET) {
			for (i = 0 ; i < MAX_5G_CHNLGRP ; i++)
				get_array_val(hwinfo + offset + i, value + i * 2,  2);
		} else if (offset == EEPROM_5G_HT401S_TxPower + PATHB_OFFSET) {
			for (i = 0 ; i < MAX_5G_CHNLGRP ; i++)
				get_array_val(hwinfo + offset + i, value + i * 2,  2);
		} else if (offset == EEPROM_5G_HT201S_TxPowerDiff + PATHA_OFFSET) {
			get_array_val(hwinfo + offset, value, 2);
		} else if (offset == EEPROM_5G_HT201S_TxPowerDiff + PATHB_OFFSET) {
			get_array_val(hwinfo + offset, value, 2);
		} else if (offset == EEPROM_5G_HT402S_TxPowerDiff + PATHA_OFFSET) {
			get_array_val(hwinfo + offset, value, 2);
		} else if (offset == EEPROM_5G_HT402S_TxPowerDiff + PATHB_OFFSET) {
			get_array_val(hwinfo + offset, value, 2);
		} else if (offset == EEPROM_5G_HT801S_TxPowerDiff + PATHA_OFFSET) {
			get_array_val(hwinfo + offset, value, 2);
		} else if (offset == EEPROM_5G_HT801S_TxPowerDiff + PATHB_OFFSET) {
			get_array_val(hwinfo + offset, value, 2);
		} else if (offset == EEPROM_5G_HT802S_TxPowerDiff + PATHA_OFFSET) {
			get_array_val(hwinfo + offset, value, 2);
		} else if (offset == EEPROM_5G_HT802S_TxPowerDiff + PATHB_OFFSET) {
			get_array_val(hwinfo + offset, value, 2);
		} else if (offset == EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET) {
			for (i = 0 ; i < MAX_2G_CHNLGRP ; i++)
				get_array_val(hwinfo + offset + i, value + i * 2,  2);
		} else if (offset == EEPROM_2G_HT201S_TxPowerDiff + PATHB_OFFSET) {
			for (i = 0 ; i < MAX_2G_CHNLGRP ; i++)
				get_array_val(hwinfo + offset + i, value + i * 2, 2);
		} else if (offset == EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET) {
			for (i = 0 ; i < MAX_2G_CHNLGRP ; i++)
				get_array_val(hwinfo + offset + i, value + i * 2,  2);
		} else if (offset == EEPROM_2G_HT402S_TxPowerDiff + PATHB_OFFSET) {
			for (i = 0 ; i < MAX_2G_CHNLGRP ; i++)
				get_array_val(hwinfo + offset + i, value + i * 2, 2);
		}
	}
#endif

#ifdef CONFIG_WLAN_HAL_8814AE
	if (GET_CHIP_VER(priv) == VERSION_8814A) {

		switch(offset){
			case 0:
		  //case 0x32:
				for (i = 0; i < 0x32; i++) {
					get_array_val(hwinfo + offset + i, value + i * 2, 2);
				}
				break;
			case EEPROM_8812_MACADDRESS:
				for (i = 0; i < MACADDRLEN; i++) {
					get_array_val(hwinfo + offset + i, value + i * 2, 2);
				}
				break;
			case EEPROM_8812_THERMAL_METER:
			case EEPROM_8812_XTAL_K:
				get_array_val(hwinfo + offset, value, 2);
				break;
			case EEPROM_2G_CCK1T_TxPower + PATHA_OFFSET:
			case EEPROM_2G_CCK1T_TxPower + PATHB_OFFSET:
			case EEPROM_2G_CCK1T_TxPower + PATHC_OFFSET:
			case EEPROM_2G_CCK1T_TxPower + PATHD_OFFSET:
				for (i = 0 ; i < MAX_2G_CHNLGRP ; i++){
					//printk("%c",(unsigned char)_atoi(value + i * 2, 16));
					get_array_val(hwinfo + offset + i, value + i * 2,  2);
				}
				break;
			case EEPROM_2G_HT401S_TxPower + PATHA_OFFSET:
			case EEPROM_2G_HT401S_TxPower + PATHB_OFFSET:
			case EEPROM_2G_HT401S_TxPower + PATHC_OFFSET:
			case EEPROM_2G_HT401S_TxPower + PATHD_OFFSET:
				for (i = 0 ; i < MAX_2G_CHNLGRP-1 ; i++){
					//printk("%c",(unsigned char)_atoi(value + i * 2, 16));
					get_array_val(hwinfo + offset + i, value + i * 2,  2);
				}
				break;
			case EEPROM_5G_HT401S_TxPower + PATHA_OFFSET:
			case EEPROM_5G_HT401S_TxPower + PATHB_OFFSET:
			case EEPROM_5G_HT401S_TxPower + PATHC_OFFSET:
			case EEPROM_5G_HT401S_TxPower + PATHD_OFFSET:
				for (i = 0 ; i < MAX_5G_CHNLGRP ; i++){
					//printk("%c",(unsigned char)_atoi(value + i * 2, 16));
					get_array_val(hwinfo + offset + i, value + i * 2,  2);
				}
				//printk("\n");
				break;
			case EEPROM_5G_HT201S_TxPowerDiff + PATHA_OFFSET:
			case EEPROM_5G_HT201S_TxPowerDiff + PATHB_OFFSET:
			case EEPROM_5G_HT201S_TxPowerDiff + PATHC_OFFSET:
			case EEPROM_5G_HT201S_TxPowerDiff + PATHD_OFFSET:
			case EEPROM_5G_HT402S_TxPowerDiff + PATHA_OFFSET:
			case EEPROM_5G_HT402S_TxPowerDiff + PATHB_OFFSET:
			case EEPROM_5G_HT402S_TxPowerDiff + PATHC_OFFSET:
			case EEPROM_5G_HT402S_TxPowerDiff + PATHD_OFFSET:
			case EEPROM_5G_HT403S_TxPowerDiff + PATHA_OFFSET:
			case EEPROM_5G_HT403S_TxPowerDiff + PATHB_OFFSET:
			case EEPROM_5G_HT403S_TxPowerDiff + PATHC_OFFSET:
			case EEPROM_5G_HT403S_TxPowerDiff + PATHD_OFFSET:
			case EEPROM_5G_HT801S_TxPowerDiff + PATHA_OFFSET:	
			case EEPROM_5G_HT801S_TxPowerDiff + PATHB_OFFSET:	
			case EEPROM_5G_HT801S_TxPowerDiff + PATHC_OFFSET:	
			case EEPROM_5G_HT801S_TxPowerDiff + PATHD_OFFSET:	
			case EEPROM_5G_HT802S_TxPowerDiff + PATHA_OFFSET:	
			case EEPROM_5G_HT802S_TxPowerDiff + PATHB_OFFSET:	
			case EEPROM_5G_HT802S_TxPowerDiff + PATHC_OFFSET:	
			case EEPROM_5G_HT802S_TxPowerDiff + PATHD_OFFSET:	
			case EEPROM_5G_HT803S_TxPowerDiff + PATHA_OFFSET:	
			case EEPROM_5G_HT803S_TxPowerDiff + PATHB_OFFSET:	
			case EEPROM_5G_HT803S_TxPowerDiff + PATHC_OFFSET:	
			case EEPROM_5G_HT803S_TxPowerDiff + PATHD_OFFSET:
				get_array_val(hwinfo + offset, value, 2);
				break;
			case EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET:
			case EEPROM_2G_HT201S_TxPowerDiff + PATHB_OFFSET:
			case EEPROM_2G_HT201S_TxPowerDiff + PATHC_OFFSET:
			case EEPROM_2G_HT201S_TxPowerDiff + PATHD_OFFSET:
			case EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET:
			case EEPROM_2G_HT402S_TxPowerDiff + PATHB_OFFSET:
			case EEPROM_2G_HT402S_TxPowerDiff + PATHC_OFFSET:
			case EEPROM_2G_HT402S_TxPowerDiff + PATHD_OFFSET:
			case EEPROM_2G_HT403S_TxPowerDiff + PATHA_OFFSET:
			case EEPROM_2G_HT403S_TxPowerDiff + PATHB_OFFSET:
			case EEPROM_2G_HT403S_TxPowerDiff + PATHC_OFFSET:
			case EEPROM_2G_HT403S_TxPowerDiff + PATHD_OFFSET:
				get_array_val(hwinfo + offset, value, 2);
		}
	}
#endif


#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if(GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B){
		if (offset == 0 || offset == 0x32) {
			for (i = 0; i < 0x32; i++) {
				get_array_val(hwinfo + offset + i, value + i * 2, 2);
			}
		}else if (offset == EEPROM_MAC_ADDR_88E) {
			for (i = 0; i < MACADDRLEN; i++) {
				get_array_val(hwinfo + offset + i, value + i * 2, 2);
			}
		} else if (offset == EEPROM_THERMAL_METER_88E ||
				   offset == EEPROM_XTAL_88E) {	   
				get_array_val(hwinfo + offset, value, 2);
		}
#ifdef CONFIG_SDIO_HCI
		else if (offset == EEPROM_ChannelPlan_88E) {
				get_array_val(hwinfo + offset, value, 2);
		}
#endif
		else if (offset == EEPROM_TX_PWR_INX_88E){ // CCK_A 
				get_array_val(hwinfo + offset,	 value, 	2);
				get_array_val(hwinfo + offset + 1, value + 2 * 2, 2);
				get_array_val(hwinfo + offset + 2, value + 5 * 2, 2);
				get_array_val(hwinfo + offset + 3, value + 8 * 2, 2);
				get_array_val(hwinfo + offset + 4, value + 11 * 2, 2);
				get_array_val(hwinfo + offset + 5, value + 13 * 2, 2);

		} else if (offset == EEPROM_TX_PWR_INX_88E + 6){ // HT40_1S_A
				get_array_val(hwinfo + offset,	 value, 	2);
				get_array_val(hwinfo + offset + 1, value + 2 * 2, 2);
				get_array_val(hwinfo + offset + 2, value + 5 * 2, 2);
				get_array_val(hwinfo + offset + 3, value + 8 * 2, 2);
				get_array_val(hwinfo + offset + 4, value + 11 * 2, 2);
		} else if (offset == EEPROM_TX_PWR_INX_88E + 11){ //DIFF_HT20_OFDM
				get_array_val(hwinfo + offset,	 value, 	2);
		}
	}
#endif

#if 0
#ifdef CONFIG_WLAN_HAL_8192EE
	if(GET_CHIP_VER(priv) == VERSION_8192E){
		if (offset == 0 || offset == 0x32) {
			for (i = 0; i < 0x32; i++) {
				get_array_val(hwinfo + offset + i, value + i * 2, 2);
			}
		}else if (offset == EEPROM_MAC_ADDR_8192EE) {
			for (i = 0; i < MACADDRLEN; i++) {
				get_array_val(hwinfo + offset + i, value + i * 2, 2);
			}
		} else if (offset == EEPROM_THERMAL_METER_8192E ||
				   offset == EEPROM_XTAL_8192E) {	   
				get_array_val(hwinfo + offset, value, 2);
		} else if ((offset == EEPROM_TX_PWR_INX_8192E + PATHA_OFFSET)||
					(offset == EEPROM_TX_PWR_INX_8192E + PATHB_OFFSET)){ // CCK_A  and CCK_B
				get_array_val(hwinfo + offset,	 value, 	2);
				get_array_val(hwinfo + offset + 1, value + 2 * 2, 2);
				get_array_val(hwinfo + offset + 2, value + 5 * 2, 2);
				get_array_val(hwinfo + offset + 3, value + 8 * 2, 2);
				get_array_val(hwinfo + offset + 4, value + 11 * 2, 2);
				get_array_val(hwinfo + offset + 5, value + 13 * 2, 2);

		} else if ((offset == EEPROM_TX_PWR_INX_8192E + PATHA_OFFSET + 6)||
		    		(offset == EEPROM_TX_PWR_INX_8192E + PATHB_OFFSET + 6)){ // HT40_1S_A and HT40_1S_B
				get_array_val(hwinfo + offset,	 value, 	2);
				get_array_val(hwinfo + offset + 1, value + 2 * 2, 2);
				get_array_val(hwinfo + offset + 2, value + 5 * 2, 2);
				get_array_val(hwinfo + offset + 3, value + 8 * 2, 2);
				get_array_val(hwinfo + offset + 4, value + 11 * 2, 2);
		} else if (offset == EEPROM_TX_PWR_INX_8192E + PATHA_OFFSET + 11){ //DIFF_HT20_OFDM
				get_array_val(hwinfo + offset,	 value, 	2);
		} else if (offset == EEPROM_TX_PWR_INX_8192E + PATHA_OFFSET + 12){ //DIFF_HT40_2S_HT20_2S
				get_array_val(hwinfo + offset,	 value, 	2);
		}
	}
#endif
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
	if(GET_CHIP_VER(priv) == VERSION_8192E)
	{
		if (offset == 0 || offset == 0x32) 
		{			
			for (i = 0; i < 0x32; i++)
				get_array_val(hwinfo + offset + i, value + i * 2, 2);
		}
		else if (offset == EEPROM_92E_MACADDRESS) 
		{
			for (i = 0; i < MACADDRLEN; i++)
				get_array_val(hwinfo + offset + i, value + i * 2, 2);			
		} 
		else if (offset == EEPROM_92E_THERMAL_METER || offset == EEPROM_92E_XTAL_K)
		{
			get_array_val(hwinfo + offset, value, 2);
		} 
		else if (	(offset == EEPROM_2G_CCK1T_TxPower + PATHA_OFFSET) ||	// CCK_A  and CCK_B
					(offset == EEPROM_2G_CCK1T_TxPower + PATHB_OFFSET))
		{ 
			get_array_val(hwinfo + offset,	 value, 	2);
			get_array_val(hwinfo + offset + 1, value + 2 * 2, 2);
			get_array_val(hwinfo + offset + 2, value + 5 * 2, 2);
			get_array_val(hwinfo + offset + 3, value + 8 * 2, 2);
			get_array_val(hwinfo + offset + 4, value + 11 * 2, 2);
			get_array_val(hwinfo + offset + 5, value + 13 * 2, 2);
		} 
		else if (	(offset == EEPROM_2G_HT401S_TxPower + PATHA_OFFSET) ||	// HT40_1S_A and HT40_1S_B
		    		(offset == EEPROM_2G_HT401S_TxPower + PATHB_OFFSET))
		{ 
			get_array_val(hwinfo + offset,	 value, 	2);
			get_array_val(hwinfo + offset + 1, value + 2 * 2, 2);
			get_array_val(hwinfo + offset + 2, value + 5 * 2, 2);
			get_array_val(hwinfo + offset + 3, value + 8 * 2, 2);
			get_array_val(hwinfo + offset + 4, value + 11 * 2, 2);
		} 
		else if( (offset == EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET) || // TX_POWER_DIFF_HT20_A & TX_POWER_DIFF_HT20_OFDM_A
			     (offset == EEPROM_2G_HT201S_TxPowerDiff + PATHB_OFFSET) || // TX_POWER_DIFF_HT20_B & TX_POWER_DIFF_HT20_OFDM_B
			     (offset == EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET) || // TX_POWER_DIFF_HT40_2S_A
			     (offset == EEPROM_2G_HT402S_TxPowerDiff + PATHB_OFFSET))	// TX_POWER_DIFF_HT40_2S_B
		{ 
			unsigned char orig_value = hwinfo[offset],
						input_value = (unsigned char)_atoi(value, 16);
						
			if( (strcmp(priv->EfuseCmd[type], "HW_TX_POWER_DIFF_HT20_A") == 0)	|| // write [7:4]
			    (strcmp(priv->EfuseCmd[type], "HW_TX_POWER_DIFF_HT20_B") == 0) 	||
			    (strcmp(priv->EfuseCmd[type], "HW_TX_POWER_DIFF_HT40_2S_A") == 0) ||
			    (strcmp(priv->EfuseCmd[type], "HW_TX_POWER_DIFF_HT40_2S_B") == 0))
			{				
				hwinfo[offset] = (orig_value & 0x0f) | ((input_value << 4) & 0xf0);
			}
			else if((strcmp(priv->EfuseCmd[type], "HW_TX_POWER_DIFF_HT20_OFDM_A") == 0) || // write [3:0]
				    (strcmp(priv->EfuseCmd[type], "HW_TX_POWER_DIFF_HT20_OFDM_B") == 0)) 
			{
				hwinfo[offset] = (orig_value & 0xf0) | (input_value & 0x0f);
			}
		}
		else if(offset==EEPROM_RFE_OPTION_8192E)
		{

				int RFE_value;
				RFE_value=_atoi(value, 10);
                if((RFE_value != LNA_TYPE_0) && (RFE_value != LNA_TYPE_1) && (RFE_value != LNA_TYPE_2) && (RFE_value != LNA_TYPE_3))
                {
                    printk("Invalid LNA type value! Set default~\n");
                    RFE_value = LNA_TYPE_0;
				}
				
				hwinfo[offset] = (RFE_value << 4); //0xCA[6:4] : LNA TYPE
		}
#ifdef CONFIG_SDIO_HCI
                else if( offset == EEPROM_92E_SDIOTYPE ) {
                        printk("val=%c\n", *value);

                        if ( *value == '2' ) {
                              hwinfo[offset]   = 0x3F;
                              hwinfo[offset+1] = 0x00;
                              // hwinfo[offset+2] = 0xFF;
                              hwinfo[offset+3] = 0x02;
                              hwinfo[offset+4] = 0x23;
                              hwinfo[offset+5] = 0x00;
                              hwinfo[offset+6] = 0x00;
                        } else if ( *value == '3' ) {
                              hwinfo[offset]   = 0x6E;
                              hwinfo[offset+1] = 0x51;
                              // hwinfo[offset+2] = 0xFF;
                              hwinfo[offset+3] = 0x03;
                              hwinfo[offset+4] = 0x34;
                              hwinfo[offset+5] = 0xFF;
                              hwinfo[offset+6] = 0xFF;
                        }
                }
#endif
		#if 0
		else if (offset == 0x02 )	// EFUSE_TEST
		{ 		
			unsigned char tmp_efuse[priv->EfuseMapLen];
			int is_fail = 0;
			int exec_cnt = _atoi(value, 10);
			unsigned int j;

			printk("***EFUSE_TEST BEGIN***\n");
			for( j = 0 ; j != exec_cnt ; ++j )
			{
				if( (j != 0) && (j % 1000 == 0) )
					printk("EFUSE_TEST execute %d times\n", j);
				efuse_ReadAllMap(priv, &tmp_efuse[0]);
				if( memcmp(&tmp_efuse[0], &priv->EfuseMap[EFUSE_INIT_MAP][0], priv->EfuseMapLen) != 0 )
				{
					is_fail = 1;
					break;
				}					
			}
			printk("***EFUSE_TEST END***\n");

			if( is_fail )
				printk("EFUSE_TEST FAIL!!\n");
			else
				printk("EFUSE_TEST SUCCESS!!\n");
		}
		#endif
	}
#endif


}
#endif

static void shadowMapWrite(struct rtl8192cd_priv *priv, u1Byte index, u1Byte *value, u1Byte *hwinfo)
{
	u1Byte i, offset, val;
	u1Byte type = TYPE_EFUSE_CMD_END;
	u4Byte len = 0;

	type = priv->efuseCmd[index].type;
	offset = priv->efuseCmd[index].offset;	
	DEBUG_INFO("write EfuseMap[0x%02x], type=%d\n", offset, type);

	switch(type) 
	{
	case TYPE_HW_TX_POWER_CCK_A:
	case TYPE_HW_TX_POWER_CCK_B:
	case TYPE_HW_TX_POWER_CCK_C:
	case TYPE_HW_TX_POWER_CCK_D:
	{
		u1Byte cck_channel_group;

		if ((GET_CHIP_VER(priv) == VERSION_8188C) ||
			(GET_CHIP_VER(priv) == VERSION_8192C) ||
			(GET_CHIP_VER(priv) == VERSION_8192D)) {
			cck_channel_group = 3;
		} else {
			cck_channel_group = MAX_2G_CHNLGRP;
		}

		for ( i = 0 ; i < cck_channel_group ; ++i )
			get_array_val(&hwinfo[offset+i], &value[i*2], 2);
		break;
	}
	case TYPE_HW_TX_POWER_HT40_1S_A:
	case TYPE_HW_TX_POWER_HT40_1S_B:
	case TYPE_HW_TX_POWER_HT40_1S_C:
	case TYPE_HW_TX_POWER_HT40_1S_D:
	{
		u1Byte ht40_channel_group;

		if ((GET_CHIP_VER(priv) == VERSION_8188C) ||
			(GET_CHIP_VER(priv) == VERSION_8192C) ||
			(GET_CHIP_VER(priv) == VERSION_8192D)) {
			ht40_channel_group = 3;
		} else {
			ht40_channel_group = MAX_2G_CHNLGRP - 1;
		}

		for ( i = 0 ; i < ht40_channel_group ; ++i )
			get_array_val(&hwinfo[offset+i], &value[i*2], 2);
		break;
	}
	case TYPE_HW_WLAN0_WLAN_ADDR:
	{
#if defined(CONFIG_RTL_92D_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8192D)
			if (priv->pshare->wlandev_idx == 1)
				offset = EEPROM_MAC1_MACADDRESS;
#endif

		for( i = 0 ; i < MACADDRLEN ; ++i ) 		
			get_array_val(&hwinfo[offset+i], &value[i*2], 2);
		break;
	}
	case TYPE_HW_TX_POWER_5G_HT40_1S_A:
	case TYPE_HW_TX_POWER_5G_HT40_1S_B:
	case TYPE_HW_TX_POWER_5G_HT40_1S_C:
	case TYPE_HW_TX_POWER_5G_HT40_1S_D:
	{
		for ( i = 0; i < MAX_5G_CHNLGRP ; ++i) 
			get_array_val(&hwinfo[offset+i], &value[i*2],  2);
		break;
	}
	case TYPE_HW_11N_THER:
	case TYPE_HW_11N_XCAP:
	case TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A:
	case TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B:
	case TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_C:
	case TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_D:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_A:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_B:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_C:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_D:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_A:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_B:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_C:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_D:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_A:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_B:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_C:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_D:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_A:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_B:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_C:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_D:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_A:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_B:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_C:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_D:
	case TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A:
	case TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_B:
	case TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_C:
	case TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_D:
	case TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_A:
	case TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_B:
	case TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_C:
	case TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_D:
	case TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_A:
	case TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_B:
	case TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_C:
	case TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_D:
	case TYPE_HW_REG_DOMAIN:
	case TYPE_HW_11N_TRSWPAPE_C9:
	case TYPE_HW_11N_TRSWPAPE_CC:
	{
		get_array_val(&hwinfo[offset], value, 2);
		break;
	}
	case TYPE_LNA_TYPE:
	{
		u1Byte rfe_value = _atoi(value, 10);
		if (rfe_value >= LNA_TYPE_MAX)
		{
			DEBUG_WARN("Invalid LNA type value! Set default~\n");
			rfe_value = LNA_TYPE_0;
		}				
		hwinfo[offset] = (rfe_value << 4); //0xCA[6:4] : LNA TYPE		
		break;
	}
	case TYPE_SDIO_TYPE:
	{
		if (*value == '2') {
			hwinfo[offset] =   0x3F;
			hwinfo[offset+1] = 0x00;
			// hwinfo[offset+2] = 0xFF;
			hwinfo[offset+3] = 0x02;
			hwinfo[offset+4] = 0x23;
			hwinfo[offset+5] = 0x00;
			hwinfo[offset+6] = 0x00;
		} else if (*value == '3') {
			hwinfo[offset]  =  0x6E;
			hwinfo[offset+1] = 0x51;
			// hwinfo[offset+2] = 0xFF;
			hwinfo[offset+3] = 0x03;
			hwinfo[offset+4] = 0x34;
			hwinfo[offset+5] = 0xFF;
			hwinfo[offset+6] = 0xFF;
		}
		break;
	}
	case TYPE_HW_TX_POWER_DIFF_HT40_2S:
	case TYPE_HW_TX_POWER_DIFF_HT20:
	case TYPE_HW_TX_POWER_DIFF_OFDM:
	case TYPE_HW_TX_POWER_DIFF_5G_HT40_2S:
	case TYPE_HW_TX_POWER_DIFF_5G_HT20:
	case TYPE_HW_TX_POWER_DIFF_5G_OFDM:
	{
		u1Byte tx_diff_channel_group = 3;
		for ( i = 0 ; i < tx_diff_channel_group ; ++i )
			get_array_val(&hwinfo[offset+i], value[i*2], 2);
		break;		
	}
	case TYPE_EFUSE_CMD_END:
	default:
		DEBUG_WARN("efuse command not found\n");
		return 0;
	}
}

#if 0
static int converToFlashFormat(struct rtl8192cd_priv *priv, unsigned char *out, unsigned char *hwinfo, int type)
{
	int  i, offset, len = 0;
	offset = getEEPROMOffset(priv, type);
	//printk("[%s]offset=%d\n",__FUNCTION__,offset);
	if (offset < 0)
		return 0;

	len += sprintf(out, "%s=", priv->EfuseCmd[type]);

#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
		if (offset == EEPROM_MACADDRESS) {
			for (i = 0; i < MACADDRLEN; i++) {
				sprintf(out + len, "%02x", hwinfo[offset + i]);
				len += 2;
			}
		} else if (offset == EEPROM_THERMAL_METER) {
			sprintf(out + len, "%02x", hwinfo[offset]);
			len += 2;
		} else {
			for (i = 0; i < MAX_2G_CHANNEL_NUM; i++) {
				if (i < 3) {
					sprintf(out + len, "%02x", hwinfo[offset]);
					len += 2;
				} else if (i < 9) {
					sprintf(out + len, "%02x", hwinfo[offset + 1]);
					len += 2;
				} else {
					sprintf(out + len, "%02x", hwinfo[offset + 2]);
					len += 2;
				}
			}
		}
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
		if (offset == EEPROM_MAC0_MACADDRESS || offset == EEPROM_MAC1_MACADDRESS) {
			for (i = 0; i < MACADDRLEN; i++) {
				sprintf(out + len, "%02x", hwinfo[offset + i]);
				len += 2;
			}
		} else if (offset == EEPROM_92D_THERMAL_METER  ||
				   offset == EEPROM_92D_TRSW_CTRL ||
				   offset == EEPROM_92D_PAPE_CTRL ||
				   offset == EEPROM_92D_XTAL_K ) {
			sprintf(out + len, "%02x", hwinfo[offset]);
			len += 2;
		} else if ((offset == EEPROM_2G_TxPowerCCK) ||
				   (offset == EEPROM_2G_TxPowerCCK + 3) ||
				   (offset == EEPROM_2G_TxPowerHT40_1S) ||
				   (offset == EEPROM_2G_TxPowerHT40_1S + 3) ||
				   (offset == EEPROM_2G_TxPowerHT40_2SDiff) ||
				   (offset == EEPROM_2G_TxPowerHT20Diff) ||
				   (offset == EEPROM_2G_TxPowerOFDMDiff)) {
			for (i = 0; i < MAX_2G_CHANNEL_NUM; i++) {
				if (i < 3) {
					sprintf(out + len, "%02x", hwinfo[offset]);
					len += 2;
				} else if (i < 9) {
					sprintf(out + len, "%02x", hwinfo[offset + 1]);
					len += 2;
				} else {
					sprintf(out + len, "%02x", hwinfo[offset + 2]);
					len += 2;
				}
			}
		} else if (offset == EEPROM_5GL_TxPowerHT40_1S) {
			for (i = 0; i < MAX_5G_CHANNEL_NUM; i++) {
				if (i >= 35 && i <= 63) { // ch 36 ~ 64
					if (i >= 35 && i <= 43) { // ch 36 ~ 44
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_1S]);
						len += 2;
					} else if (i >= 45 && i <= 53) { // ch 46 ~ 54
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_1S + 1]);
						len += 2;
					} else if (i >= 55 && i <= 63) { // ch 56 ~ 64
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_1S + 2]);
						len += 2;
					} else {
						sprintf(out + len, "00");
						len += 2;
					}
				} else if (i >= 99 && i <= 139) { // ch 100 ~ 140
					if (i >= 99 && i <= 111) { // ch 100 ~ 112
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_1S]);
						len += 2;
					} else if (i >= 113 && i <= 125) { // ch 114 ~ 126
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_1S + 1]);
						len += 2;
					} else if (i >= 127 && i <= 139) { // ch 128 ~ 140
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_1S + 2]);
						len += 2;
					} else {
						sprintf(out + len, "00");
						len += 2;
					}
				} else if (i >= 148 && i <= 164 ) { // ch 149 ~ 165
					if (i >= 148 && i <= 152) { // ch 149 ~ 153
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_1S]);
						len += 2;
					} else if (i >= 154 && i <= 158) { // ch 155 ~ 159
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_1S + 1]);
						len += 2;
					} else if (i >= 160 && i <= 164) { // ch 161 ~ 165
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_1S + 2]);
						len += 2;
					} else {
						sprintf(out + len, "00");
						len += 2;
					}
				} else {
					sprintf(out + len, "00");
					len += 2;
				}
			}
		} else if (offset == EEPROM_5GL_TxPowerHT40_1S + 3) {
			for (i = 0; i < MAX_5G_CHANNEL_NUM; i++) {
				if (i >= 35 && i <= 63) { // ch 36 ~ 64
					if (i >= 35 && i <= 43) { // ch 36 ~ 44
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_1S + 3]);
						len += 2;
					} else if (i >= 45 && i <= 53) { // ch 46 ~ 54
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_1S + 4]);
						len += 2;
					} else if (i >= 55 && i <= 63) { // ch 56 ~ 64
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_1S + 5]);
						len += 2;
					} else {
						sprintf(out + len, "00");
						len += 2;
					}
				} else if (i >= 99 && i <= 139) { // ch 100 ~ 140
					if (i >= 99 && i <= 111) { // ch 100 ~ 112
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_1S + 3]);
						len += 2;
					} else if (i >= 113 && i <= 125) { // ch 114 ~ 126
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_1S + 4]);
						len += 2;
					} else if (i >= 127 && i <= 139) { // ch 128 ~ 140
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_1S + 5]);
						len += 2;
					} else {
						sprintf(out + len, "00");
						len += 2;
					}
				} else if (i >= 148 && i <= 164 ) { // ch 149 ~ 165
					if (i >= 148 && i <= 152) { // ch 149 ~ 153
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_1S + 3]);
						len += 2;
					} else if (i >= 154 && i <= 158) { // ch 155 ~ 159
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_1S + 4]);
						len += 2;
					} else if (i >= 160 && i <= 164) { // ch 161 ~ 165
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_1S + 5]);
						len += 2;
					} else {
						sprintf(out + len, "00");
						len += 2;
					}
				} else {
					sprintf(out + len, "00");
					len += 2;
				}
			}
		} else if (offset == EEPROM_5GL_TxPowerHT40_2SDiff) {
			for (i = 0; i < MAX_5G_CHANNEL_NUM; i++) {
				if (i >= 35 && i <= 63) { // ch 36 ~ 64
					if (i >= 35 && i <= 43) { // ch 36 ~ 44
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_2SDiff]);
						len += 2;
					} else if (i >= 45 && i <= 53) { // ch 46 ~ 54
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_2SDiff + 1]);
						len += 2;
					} else if (i >= 55 && i <= 63) { // ch 56 ~ 64
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT40_2SDiff + 2]);
						len += 2;
					} else {
						sprintf(out + len, "00");
						len += 2;
					}
				} else if (i >= 99 && i <= 139) { // ch 100 ~ 140
					if (i >= 99 && i <= 111) { // ch 100 ~ 112
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_2SDiff]);
						len += 2;
					} else if (i >= 113 && i <= 125) { // ch 114 ~ 126
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_2SDiff + 1]);
						len += 2;
					} else if (i >= 127 && i <= 139) { // ch 128 ~ 140
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT40_2SDiff + 2]);
						len += 2;
					} else {
						sprintf(out + len, "00");
						len += 2;
					}
				} else if (i >= 148 && i <= 164 ) { // ch 149 ~ 165
					if (i >= 148 && i <= 152) { // ch 149 ~ 153
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_2SDiff]);
						len += 2;
					} else if (i >= 154 && i <= 158) { // ch 155 ~ 159
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_2SDiff + 1]);
						len += 2;
					} else if (i >= 160 && i <= 164) { // ch 161 ~ 165
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT40_2SDiff + 2]);
						len += 2;
					} else {
						sprintf(out + len, "00");
						len += 2;
					}
				} else {
					sprintf(out + len, "00");
					len += 2;
				}
			}
		} else if (offset == EEPROM_5GL_TxPowerHT20Diff) {
			for (i = 0; i < MAX_5G_CHANNEL_NUM; i++) {
				if (i >= 35 && i <= 63) { // ch 36 ~ 64
					if (i >= 35 && i <= 43) { // ch 36 ~ 44
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT20Diff]);
						len += 2;
					} else if (i >= 45 && i <= 53) { // ch 46 ~ 54
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT20Diff + 1]);
						len += 2;
					} else if (i >= 55 && i <= 63) { // ch 56 ~ 64
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GL_TxPowerHT20Diff + 2]);
						len += 2;
					} else {
						sprintf(out + len, "00");
						len += 2;
					}
				} else if (i >= 99 && i <= 139) { // ch 100 ~ 140
					if (i >= 99 && i <= 111) { // ch 100 ~ 112
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT20Diff]);
						len += 2;
					} else if (i >= 113 && i <= 125) { // ch 114 ~ 126
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT20Diff + 1]);
						len += 2;
					} else if (i >= 127 && i <= 139) { // ch 128 ~ 140
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GM_TxPowerHT20Diff + 2]);
						len += 2;
					} else {
						sprintf(out + len, "00");
						len += 2;
					}
				} else if (i >= 148 && i <= 164 ) { // ch 149 ~ 165
					if (i >= 148 && i <= 152) { // ch 149 ~ 153
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT20Diff]);
						len += 2;
					} else if (i >= 154 && i <= 158) { // ch 155 ~ 159
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT20Diff + 1]);
						len += 2;
					} else if (i >= 160 && i <= 164) { // ch 161 ~ 165
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GH_TxPowerHT20Diff + 2]);
						len += 2;
					} else {
						sprintf(out + len, "00");
						len += 2;
					}
				} else {
					sprintf(out + len, "00");
					len += 2;
				}
			}
		} else if (offset == EEPROM_5GL_TxPowerOFDMDiff) {
			for (i = 0; i < MAX_5G_CHANNEL_NUM; i++) {
				if (i >= 35 && i <= 63) { // ch 36 ~ 64
					if (i >= 35 && i <= 43) { // ch 36 ~ 44
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GL_TxPowerOFDMDiff]);
						len += 2;
					} else if (i >= 45 && i <= 53) { // ch 46 ~ 54
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GL_TxPowerOFDMDiff + 1]);
						len += 2;
					} else if (i >= 55 && i <= 63) { // ch 56 ~ 64
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GL_TxPowerOFDMDiff + 2]);
						len += 2;
					} else {
						sprintf(out + len, "00");
						len += 2;
					}
				} else if (i >= 99 && i <= 139) { // ch 100 ~ 140
					if (i >= 99 && i <= 111) { // ch 100 ~ 112
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GM_TxPowerOFDMDiff]);
						len += 2;
					} else if (i >= 113 && i <= 125) { // ch 114 ~ 126
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GM_TxPowerOFDMDiff + 1]);
						len += 2;
					} else if (i >= 127 && i <= 139) { // ch 128 ~ 140
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GM_TxPowerOFDMDiff + 2]);
						len += 2;
					} else {
						sprintf(out + len, "00");
						len += 2;
					}
				} else if (i >= 148 && i <= 164 ) { // ch 149 ~ 165
					if (i >= 148 && i <= 152) { // ch 149 ~ 153
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GH_TxPowerOFDMDiff]);
						len += 2;
					} else if (i >= 154 && i <= 158) { // ch 155 ~ 159
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GH_TxPowerOFDMDiff + 1]);
						len += 2;
					} else if (i >= 160 && i <= 164) { // ch 161 ~ 165
						sprintf(out + len, "%02x", hwinfo[EEPROM_5GH_TxPowerOFDMDiff + 2]);
						len += 2;
					} else {
						sprintf(out + len, "00");
						len += 2;
					}
				} else {
					sprintf(out + len, "00");
					len += 2;
				}
			}
		}
	}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		if (offset == EEPROM_8812_MACADDRESS) {
			for (i = 0; i < MACADDRLEN; i++) {
				sprintf(out + len, "%02x", hwinfo[offset + i]);
				len += 2;
			}
		} else if (offset == EEPROM_8812_THERMAL_METER  ||
				   offset == EEPROM_8812_XTAL_K ) {
			sprintf(out + len, "%02x", hwinfo[offset]);
			len += 2;
		} else if ((offset == EEPROM_2G_CCK1T_TxPower + PATHA_OFFSET) ||
				   (offset == EEPROM_2G_CCK1T_TxPower + PATHB_OFFSET) ||
				   (offset == EEPROM_2G_HT401S_TxPower + PATHA_OFFSET) ||
				   (offset == EEPROM_2G_HT401S_TxPower + PATHB_OFFSET) ) {
			for (i = 0; i < MAX_2G_CHANNEL_NUM; i++) {
				int chnl_gp = find_2gchnlgroup(i);

				if (chnl_gp > -1) {
					sprintf(out + len, "%02x", hwinfo[offset + chnl_gp]);
					len += 2;
				} else {
					sprintf(out + len, "%02x", 0);
					len += 2;
				}
			}
		} else if (offset == EEPROM_5G_HT401S_TxPower + PATHA_OFFSET) {
			for (i = 0; i < MAX_5G_CHNLGRP; i++) {
				sprintf(out + len, "%02x", hwinfo[offset + i]);
				len += 2;
			}
		} else if (offset == EEPROM_5G_HT401S_TxPower + PATHB_OFFSET) {
			for (i = 0; i < MAX_5G_CHNLGRP; i++) {
				sprintf(out + len, "%02x", hwinfo[offset + i]);
				len += 2;
			}
		} else if (offset == EEPROM_5G_HT201S_TxPowerDiff + PATHA_OFFSET) {
			for (i = 0; i < MAX_5G_CHNLGRP; i++) {

				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
			}
		} else if (offset == EEPROM_5G_HT201S_TxPowerDiff + PATHB_OFFSET) {
			for (i = 0; i < MAX_5G_CHNLGRP; i++) {
				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
			}
		} else if (offset == EEPROM_5G_HT402S_TxPowerDiff + PATHA_OFFSET) {
			for (i = 0; i < MAX_5G_CHNLGRP; i++) {

				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
			}
		} else if (offset == EEPROM_5G_HT402S_TxPowerDiff + PATHB_OFFSET) {
			for (i = 0; i < MAX_5G_CHNLGRP; i++) {
				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
			}
		} else if (offset == EEPROM_5G_HT801S_TxPowerDiff + PATHA_OFFSET) {
			for (i = 0; i < MAX_5G_CHNLGRP; i++) {

				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
			}
		} else if (offset == EEPROM_5G_HT801S_TxPowerDiff + PATHB_OFFSET) {
			for (i = 0; i < MAX_5G_CHNLGRP; i++) {
				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
			}
		} else if (offset == EEPROM_5G_HT802S_TxPowerDiff + PATHA_OFFSET) {
			for (i = 0; i < MAX_5G_CHNLGRP; i++) {

				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
			}
		} else if (offset == EEPROM_5G_HT802S_TxPowerDiff + PATHB_OFFSET) {
			for (i = 0; i < MAX_5G_CHNLGRP; i++) {
				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
			}
		} else if (offset == EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET) {
			for (i = 0; i < MAX_2G_CHANNEL_NUM; i++) {
				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
			}
		} else if (offset == EEPROM_2G_HT201S_TxPowerDiff + PATHB_OFFSET) {
			for (i = 0; i < MAX_2G_CHANNEL_NUM; i++) {
				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
			}
		} else if (offset == EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET) {
			for (i = 0; i < MAX_2G_CHANNEL_NUM; i++) {
				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
			}
		} else if (offset == EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET) {
			for (i = 0; i < MAX_2G_CHANNEL_NUM; i++) {
				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
			}
		}
	}
#endif
#ifdef CONFIG_WLAN_HAL_8814AE
	if (GET_CHIP_VER(priv) == VERSION_8814A) {
		switch(offset){
			case EEPROM_8812_MACADDRESS:
				for (i = 0; i < MACADDRLEN; i++) {
					sprintf(out + len, "%02x", hwinfo[offset + i]);
					len += 2;
				}
				break;
			case EEPROM_8812_THERMAL_METER:
			case EEPROM_8812_XTAL_K:
				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
				break;
			case EEPROM_2G_CCK1T_TxPower + PATHA_OFFSET:
			case EEPROM_2G_CCK1T_TxPower + PATHB_OFFSET:
			case EEPROM_2G_CCK1T_TxPower + PATHC_OFFSET:
			case EEPROM_2G_CCK1T_TxPower + PATHD_OFFSET:
				for (i = 0; i < MAX_2G_CHNLGRP; i++) {
					sprintf(out + len, "%02x", hwinfo[offset + i]);
					len += 2;
				}
				break;
			case EEPROM_2G_HT401S_TxPower + PATHA_OFFSET:
			case EEPROM_2G_HT401S_TxPower + PATHB_OFFSET:
			case EEPROM_2G_HT401S_TxPower + PATHC_OFFSET:
			case EEPROM_2G_HT401S_TxPower + PATHD_OFFSET:
				for (i = 0; i < MAX_2G_CHNLGRP-1; i++) {
					sprintf(out + len, "%02x", hwinfo[offset + i]);
					len += 2;
				}
				break;
			case EEPROM_5G_HT401S_TxPower + PATHA_OFFSET:
			case EEPROM_5G_HT401S_TxPower + PATHB_OFFSET:
			case EEPROM_5G_HT401S_TxPower + PATHC_OFFSET:
			case EEPROM_5G_HT401S_TxPower + PATHD_OFFSET:
				for (i = 0; i < MAX_5G_CHNLGRP; i++) {
					sprintf(out + len, "%02x", hwinfo[offset + i]);
					len += 2;
				}
				break;
			case EEPROM_5G_HT201S_TxPowerDiff + PATHA_OFFSET:
			case EEPROM_5G_HT201S_TxPowerDiff + PATHB_OFFSET:
			case EEPROM_5G_HT201S_TxPowerDiff + PATHC_OFFSET:
			case EEPROM_5G_HT201S_TxPowerDiff + PATHD_OFFSET:
			case EEPROM_5G_HT402S_TxPowerDiff + PATHA_OFFSET:
			case EEPROM_5G_HT402S_TxPowerDiff + PATHB_OFFSET:
			case EEPROM_5G_HT402S_TxPowerDiff + PATHC_OFFSET:
			case EEPROM_5G_HT402S_TxPowerDiff + PATHD_OFFSET:
			case EEPROM_5G_HT403S_TxPowerDiff + PATHA_OFFSET:
			case EEPROM_5G_HT403S_TxPowerDiff + PATHB_OFFSET:
			case EEPROM_5G_HT403S_TxPowerDiff + PATHC_OFFSET:
			case EEPROM_5G_HT403S_TxPowerDiff + PATHD_OFFSET:
			case EEPROM_5G_HT801S_TxPowerDiff + PATHA_OFFSET:	
			case EEPROM_5G_HT801S_TxPowerDiff + PATHB_OFFSET:	
			case EEPROM_5G_HT801S_TxPowerDiff + PATHC_OFFSET:	
			case EEPROM_5G_HT801S_TxPowerDiff + PATHD_OFFSET:	
			case EEPROM_5G_HT802S_TxPowerDiff + PATHA_OFFSET:	
			case EEPROM_5G_HT802S_TxPowerDiff + PATHB_OFFSET:	
			case EEPROM_5G_HT802S_TxPowerDiff + PATHC_OFFSET:	
			case EEPROM_5G_HT802S_TxPowerDiff + PATHD_OFFSET:	
			case EEPROM_5G_HT803S_TxPowerDiff + PATHA_OFFSET:	
			case EEPROM_5G_HT803S_TxPowerDiff + PATHB_OFFSET:	
			case EEPROM_5G_HT803S_TxPowerDiff + PATHC_OFFSET:	
			case EEPROM_5G_HT803S_TxPowerDiff + PATHD_OFFSET:
			case EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET:
			case EEPROM_2G_HT201S_TxPowerDiff + PATHB_OFFSET:
			case EEPROM_2G_HT201S_TxPowerDiff + PATHC_OFFSET:
			case EEPROM_2G_HT201S_TxPowerDiff + PATHD_OFFSET:
			case EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET:
			case EEPROM_2G_HT402S_TxPowerDiff + PATHB_OFFSET:
			case EEPROM_2G_HT402S_TxPowerDiff + PATHC_OFFSET:
			case EEPROM_2G_HT402S_TxPowerDiff + PATHD_OFFSET:
			case EEPROM_2G_HT403S_TxPowerDiff + PATHA_OFFSET:
			case EEPROM_2G_HT403S_TxPowerDiff + PATHB_OFFSET:
			case EEPROM_2G_HT403S_TxPowerDiff + PATHC_OFFSET:
			case EEPROM_2G_HT403S_TxPowerDiff + PATHD_OFFSET:
				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
				break;
		}
		//printk("[%s]out=%s\n",__FUNCTION__,out);
	}
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if(GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B){
		if (offset == EEPROM_MAC_ADDR_88E) {
			for (i = 0; i < MACADDRLEN; i++) {
				sprintf(out + len, "%02x", hwinfo[offset + i]);
				len += 2;
			}
		} else if (offset == EEPROM_THERMAL_METER_88E ||
				   offset == EEPROM_XTAL_88E) {
				   
			sprintf(out + len, "%02x", hwinfo[offset]);
			len += 2;
		}
#ifdef CONFIG_SDIO_HCI
		else if (offset == EEPROM_ChannelPlan_88E) {
			sprintf(out + len, "%02x", hwinfo[offset]);
			len += 2;
		}
#endif
		else if (offset == EEPROM_TX_PWR_INX_88E ||    // CCK_A
				   offset == EEPROM_TX_PWR_INX_88E + 6){ // HT40_1S_A
				   
			for (i = 0; i < MAX_2G_CHANNEL_NUM; i++) {
				if (i < 2) {
					sprintf(out + len, "%02x", hwinfo[offset]);
					len += 2;
				} else if (i < 5) {
					sprintf(out + len, "%02x", hwinfo[offset + 1]);
					len += 2;
				} else if (i < 8) {
					sprintf(out + len, "%02x", hwinfo[offset + 2]);
					len += 2;
				} else if (i < 11) {
					sprintf(out + len, "%02x", hwinfo[offset + 3]);
					len += 2;
				} else if (i < 13) {
					sprintf(out + len, "%02x", hwinfo[offset + 4]);
					len += 2;
				} else {
					if( offset == EEPROM_TX_PWR_INX_88E)
						sprintf(out + len, "%02x", hwinfo[offset + 5]);
					else
						sprintf(out + len, "%02x", hwinfo[offset + 4]);
					len += 2;
				}
			}
		} 
		else if (offset == EEPROM_TX_PWR_INX_88E + 11 ){ // DIFF_HT20_OFDM
			sprintf(out + len, "%02x", hwinfo[offset]);
			len += 2;
		}
	}
#endif

#if	defined(CONFIG_WLAN_HAL_8192EE)
	if ( GET_CHIP_VER(priv)==VERSION_8192E ) 
	{
		switch(offset)
		{
		case EEPROM_2G_CCK1T_TxPower + PATHA_OFFSET:	//	HW_TX_POWER_CCK_A
		case EEPROM_2G_CCK1T_TxPower + PATHB_OFFSET:	//	HW_TX_POWER_CCK_B
			for ( i = 0; i < MAX_2G_CHANNEL_NUM ; i++ ) 
			{
				int ch_gp = find_2gchnlgroup(i);
				if(ch_gp > -1) 
					len += sprintf(out + len, "%02x", hwinfo[offset + ch_gp]);
			}
			break;
		case EEPROM_2G_HT401S_TxPower + PATHA_OFFSET:	//	HW_TX_POWER_HT40_1S_A
		case EEPROM_2G_HT401S_TxPower + PATHB_OFFSET:	//	HW_TX_POWER_HT40_1S_B
			for ( i = 0; i < MAX_2G_CHANNEL_NUM ; i++ )
			{
				int ch_gp = find_2gchnlgroup(i);
				if(ch_gp > -1) 
				{
					if(i == 13)
						ch_gp -= 1;					
					len += sprintf(out + len, "%02x", hwinfo[offset + ch_gp]);
				}
			}
			break;
		case EEPROM_2G_HT201S_TxPowerDiff + PATHA_OFFSET:	//	HW_TX_POWER_DIFF_HT20_A and HW_TX_POWER_DIFF_HT20_OFDM_A
			for ( i = 0; i < MAX_2G_CHANNEL_NUM ; i++ )
			{
				if( strcmp(priv->EfuseCmd[type], "HW_TX_POWER_DIFF_HT20_A") == 0 )				
					len += sprintf(out + len, "%02x", (hwinfo[offset]&0xf0) >> 4);				
				else if( strcmp(priv->EfuseCmd[type], "HW_TX_POWER_DIFF_HT20_OFDM_A") == 0 )
					len += sprintf(out + len, "%02x", (hwinfo[offset]&0x0f) );
			}
			break;
		case EEPROM_2G_HT201S_TxPowerDiff + PATHB_OFFSET:	//	HW_TX_POWER_DIFF_HT20_B and HW_TX_POWER_DIFF_HT20_OFDM_B
			for ( i = 0; i < MAX_2G_CHANNEL_NUM ; i++ )
			{
				if( strcmp(priv->EfuseCmd[type], "HW_TX_POWER_DIFF_HT20_B") == 0 )
					len += sprintf(out + len, "%02x", (hwinfo[offset]&0xf0));
				else if( strcmp(priv->EfuseCmd[type], "HW_TX_POWER_DIFF_HT20_OFDM_B") == 0 )
					len += sprintf(out + len, "%02x", (hwinfo[offset]&0x0f) << 4 );			
			}
			break;
		case EEPROM_2G_HT402S_TxPowerDiff + PATHA_OFFSET:	//	HW_TX_POWER_DIFF_HT40_2S_A
		case EEPROM_2G_HT402S_TxPowerDiff + PATHB_OFFSET:	//	HW_TX_POWER_DIFF_HT40_2S_B
			for ( i = 0; i < MAX_2G_CHANNEL_NUM ; i++ )
			{
				if( strcmp(priv->EfuseCmd[type], "HW_TX_POWER_DIFF_HT40_2S_A") == 0 )
					len += sprintf(out + len, "%02x", (hwinfo[offset]&0xf0) >> 4);
				else if( strcmp(priv->EfuseCmd[type], "HW_TX_POWER_DIFF_HT40_2S_B") == 0 )
					len += sprintf(out + len, "%02x", (hwinfo[offset]&0xf0));
			}				
			break;
		case EEPROM_92E_MACADDRESS:	//	HW_WLAN0_WLAN_ADDR
			for( i = 0 ; i < MACADDRLEN ; i++ )			
				len += sprintf(out + len, "%02x", hwinfo[offset + i]);
			break;
		case EEPROM_92E_THERMAL_METER:	//	HW_11N_THER
		case EEPROM_92E_XTAL_K:			//	HW_11N_XCAP
			len += sprintf(out + len, "%02x", hwinfo[offset]);
			break;
		case EEPROM_RFE_OPTION_8192E:	//	EFUSE_LNA_TYPE
			len += sprintf(out + len, "%02x", (hwinfo[offset]&0x70) >> 4);
			break;
#ifdef CONFIG_SDIO_HCI
		case EEPROM_92E_SDIOTYPE:
			for( i = 0 ; i < SDIOTYPELEN; i++ )
				len += sprintf(out + len, "%02x", hwinfo[offset + i]);
			break; 
#endif
		default:
			break;
		}
	}
#endif

#if 0
#ifdef CONFIG_WLAN_HAL_8192EE
		if(GET_CHIP_VER(priv) == VERSION_8192E){
			if (offset == EEPROM_MAC_ADDR_8192EE) {
				for (i = 0; i < MACADDRLEN; i++) {
					sprintf(out + len, "%02x", hwinfo[offset + i]);
					len += 2;
				}
			} else if (offset == EEPROM_THERMAL_METER_8192E ||
					   offset == EEPROM_XTAL_8192E) {
					   
				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
			} else if ((offset == EEPROM_TX_PWR_INX_8192E + PATHA_OFFSET)||		// CCK_A
					   (offset == EEPROM_TX_PWR_INX_8192E + PATHB_OFFSET)||		// CCK_B
					   (offset == EEPROM_TX_PWR_INX_8192E + PATHA_OFFSET + 6)||	// HT40_1S_A
					   (offset == EEPROM_TX_PWR_INX_8192E + PATHB_OFFSET + 6)){ 	// HT40_1S_B
					   
				for (i = 0; i < MAX_2G_CHANNEL_NUM; i++) {
					if (i < 2) {
						sprintf(out + len, "%02x", hwinfo[offset]);
						len += 2;
					} else if (i < 5) {
						sprintf(out + len, "%02x", hwinfo[offset + 1]);
						len += 2;
					} else if (i < 8) {
						sprintf(out + len, "%02x", hwinfo[offset + 2]);
						len += 2;
					} else if (i < 11) {
						sprintf(out + len, "%02x", hwinfo[offset + 3]);
						len += 2;
					} else if (i < 13) {
						sprintf(out + len, "%02x", hwinfo[offset + 4]);
						len += 2;
					} else {
						if((offset == EEPROM_TX_PWR_INX_8192E + PATHA_OFFSET)||		// CCK_A
						   (offset == EEPROM_TX_PWR_INX_8192E + PATHB_OFFSET))		// CCK_B
							sprintf(out + len, "%02x", hwinfo[offset + 5]);
						else
							sprintf(out + len, "%02x", hwinfo[offset + 4]);
						len += 2;
					}
				}
			} 
			else if (offset == EEPROM_TX_PWR_INX_8192E + PATHA_OFFSET + 11 ){ // DIFF_HT20_OFDM
				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
			}
			else if (offset == EEPROM_TX_PWR_INX_8192E + PATHA_OFFSET + 12 ){ // DIFF_HT40_2S_HT20_2S
				sprintf(out + len, "%02x", hwinfo[offset]);
				len += 2;
			}
		}
#endif
#endif

	out[len] = '\0';
	return len + 1;
}
#endif

static int converToFlashFormat(struct rtl8192cd_priv *priv, u1Byte *data, u1Byte *hwinfo)
{
	u1Byte i, offset;
	u1Byte len = 0;
	u1Byte type = TYPE_EFUSE_CMD_END;

	for ( i = 0 ; i != priv->efuseCmdNum ; ++i ) {
		if (!strcmp(data, priv->efuseCmd[i].name)) {
			type = priv->efuseCmd[i].type;
			offset = priv->efuseCmd[i].offset;
			DEBUG_INFO("read efuse_map[0x%02x], type=%d\n", offset, type);
			len += sprintf(data, "%s=", priv->efuseCmd[i].name);
			break;
		}
	}

	switch(type) 
	{
	case TYPE_HW_TX_POWER_CCK_A:
	case TYPE_HW_TX_POWER_CCK_B:
	case TYPE_HW_TX_POWER_CCK_C:
	case TYPE_HW_TX_POWER_CCK_D:
	{
		u1Byte cck_channel_group;

		if ((GET_CHIP_VER(priv) == VERSION_8188C) ||
			(GET_CHIP_VER(priv) == VERSION_8192C) ||
			(GET_CHIP_VER(priv) == VERSION_8192D)) {
			cck_channel_group = 3;
		} else {
			cck_channel_group = MAX_2G_CHNLGRP;
		}

		for ( i = 0 ; i < cck_channel_group ; ++i )
			len += sprintf(data + len, "%02x", hwinfo[offset + i]);
		break;
	}
	case TYPE_HW_TX_POWER_HT40_1S_A:
	case TYPE_HW_TX_POWER_HT40_1S_B:
	case TYPE_HW_TX_POWER_HT40_1S_C:
	case TYPE_HW_TX_POWER_HT40_1S_D:
	{
		u1Byte ht40_channel_group;

		if ((GET_CHIP_VER(priv) == VERSION_8188C) ||
			(GET_CHIP_VER(priv) == VERSION_8192C) ||
			(GET_CHIP_VER(priv) == VERSION_8192D)) {
			ht40_channel_group = 3;
		} else {
			ht40_channel_group = MAX_2G_CHNLGRP - 1;
		}

		for ( i = 0 ; i < ht40_channel_group ; ++i )
			len += sprintf(data + len, "%02x", hwinfo[offset + i]);
		break;
	}
	case TYPE_HW_WLAN0_WLAN_ADDR:
	{
#if defined(CONFIG_RTL_92D_SUPPORT)
		if (GET_CHIP_VER(priv) == VERSION_8192D)
			if (priv->pshare->wlandev_idx == 1)
				offset = EEPROM_MAC1_MACADDRESS;
#endif

		for( i = 0 ; i < MACADDRLEN ; ++i ) 		
			len += sprintf(data + len, "%02x", hwinfo[offset + i]);
		break;
	}
	case TYPE_HW_TX_POWER_5G_HT40_1S_A:
	case TYPE_HW_TX_POWER_5G_HT40_1S_B:
	case TYPE_HW_TX_POWER_5G_HT40_1S_C:
	case TYPE_HW_TX_POWER_5G_HT40_1S_D:
	{
		for ( i = 0; i < MAX_5G_CHNLGRP ; ++i) 
			len += sprintf(data + len, "%02x", hwinfo[offset + i]);
		break;
	}
	case TYPE_HW_11N_THER:
	case TYPE_HW_11N_XCAP:
	case TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A:
	case TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B:
	case TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_C:
	case TYPE_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_D:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_A:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_B:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_C:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_D:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_A:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_B:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_C:
	case TYPE_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_D:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_A:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_B:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_C:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_D:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_A:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_B:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_C:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_D:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_A:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_B:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_C:
	case TYPE_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_D:
	case TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A:
	case TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_B:
	case TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_C:
	case TYPE_HW_TX_POWER_DIFF_20BW1S_OFDM1T_D:
	case TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_A:
	case TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_B:
	case TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_C:
	case TYPE_HW_TX_POWER_DIFF_40BW2S_20BW2S_D:
	case TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_A:
	case TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_B:
	case TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_C:
	case TYPE_HW_TX_POWER_DIFF_40BW3S_20BW3S_D:
	case TYPE_HW_REG_DOMAIN:
	case TYPE_HW_11N_TRSWPAPE_C9:
	case TYPE_HW_11N_TRSWPAPE_CC:
	{
		len += sprintf(data + len, "%02x", hwinfo[offset]);
		break;
	}
	case TYPE_LNA_TYPE:
	{
		len += sprintf(data + len, "%02x", (hwinfo[offset]&0x70) >> 4);
		break;
	}
	case TYPE_SDIO_TYPE:
	{
		for( i = 0 ; i < SDIOTYPELEN ; ++i )
			len += sprintf(data + len, "%02x", hwinfo[offset + i]);
		break;
	}
	case TYPE_HW_TX_POWER_DIFF_HT40_2S:
	case TYPE_HW_TX_POWER_DIFF_HT20:
	case TYPE_HW_TX_POWER_DIFF_OFDM:
	case TYPE_HW_TX_POWER_DIFF_5G_HT40_2S:
	case TYPE_HW_TX_POWER_DIFF_5G_HT20:
	case TYPE_HW_TX_POWER_DIFF_5G_OFDM:
	{
		u1Byte tx_diff_channel_group = 3;
		for ( i = 0 ; i < tx_diff_channel_group ; ++i )
			len += sprintf(data + len, "%02x", hwinfo[offset + i]);
		break;		
	}
	case TYPE_EFUSE_CMD_END:
	default:
		DEBUG_WARN("efuse command not found\n");
		return 0;
	}

	return len;
}

int efuse_get(struct rtl8192cd_priv *priv, unsigned char *data)
{
	return converToFlashFormat(priv, data, &(priv->EfuseMap[EFUSE_INIT_MAP][0]));
#if 0
	int j, len;
#if 0
#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C))
		para_num = EFUSECMD_NUM_92C;
#endif
#if defined(CONFIG_RTL_92D_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8192D)
		para_num = EFUSECMD_NUM_92D;
#endif
#if defined(CONFIG_RTL_8812_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8812E)
		para_num = EFUSECMD_NUM_8812;
#endif
#if defined(CONFIG_RTL_88E_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8188E)
		para_num = EFUSECMD_NUM_88E;
#endif
#if defined(CONFIG_WLAN_HAL_8192EE)
	if (GET_CHIP_VER(priv) == VERSION_8192E)
		para_num = EFUSECMD_NUM_92E;
#endif
#endif
	printk("[%s] data=%s\n",__FUNCTION__,data);

	for( j = 0 ; j < priv->EfuseCmdNum ; j++ )
	{
		if (strcmp(data, priv->EfuseCmd[j]) == 0) 
		{	
		//	printk("[%s]priv->EfuseCmd[j]=%s\n",__FUNCTION__,priv->EfuseCmd[j]);
			len =  converToFlashFormat(priv, data, &(priv->EfuseMap[EFUSE_INIT_MAP][0]), j);
			//printk("%s\n", data);

			return len;
		}
	}
	return 0;
#endif
}


int efuse_set(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	int j;

	for( j = 0 ; j < priv->efuseCmdNum ; j++ ) {
		val = get_value_by_token(data, priv->efuseCmd[j].name);
	
		if(val) {
			DEBUG_INFO("efuse cmd=%s, val=%s\n", priv->efuseCmd[j].name, val + 1);
			shadowMapWrite(priv, j, val + 1, &(priv->EfuseMap[EFUSE_MODIFY_MAP][0]));
			break;
		}
	}

	return 0;
#if 0
#if 0
#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C))
		para_num = EFUSECMD_NUM_92C;
#endif
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
	if ((GET_CHIP_VER(priv) == VERSION_8192D) || (GET_CHIP_VER(priv) == VERSION_8812E))
		para_num = EFUSECMD_NUM_92D;
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E)
		para_num = EFUSECMD_NUM_88E;
#endif
#ifdef CONFIG_WLAN_HAL_8192EE
		if (GET_CHIP_VER(priv) == VERSION_8192E)
			para_num = EFUSECMD_NUM_92E;
#endif
#endif
	int i=0;
	printk("[%s] data=%s\n",__FUNCTION__,data);
	for( j = 0 ; j < priv->EfuseCmdNum ; j++ )
	{
	//	printk("[%s] priv->EfuseCmd[j]=%s\n",__FUNCTION__,priv->EfuseCmd[j]);
		val = get_value_by_token(data, priv->EfuseCmd[j]);
	
		if(val){
			shadowMapWrite(priv, j, val + 1, &(priv->EfuseMap[EFUSE_MODIFY_MAP][0]));
			break; //4 Added
		}
	}
	return 0;
#endif
}


int efuse_sync(struct rtl8192cd_priv *priv, unsigned char *data)
{
	DEBUG_INFO("efuse sync\n");
	EFUSE_ShadowUpdate(priv);
	return 0;
}

#ifdef RTK_NL80211
void read_efuse_mac_address(struct rtl8192cd_priv *priv,char * efusemac)
{
	u8 efuse_MAC = 0;
	if(!efusemac)
		return;

	EfuseInit(priv);
	if (EfuseMapAlloc(priv) == FAIL) {
		printk(KERN_ERR "%s: can't allocate efuse map\n", __func__);
		return -ENOMEM;
	}
	if (EfuseCmdInit(priv) == FAIL) {
		printk(KERN_ERR "%s: can't allocate efuse cmd\n", __func__);
		return -ENOMEM;
	}
	EFUSE_ShadowMapUpdate(priv); 
#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
		efuse_MAC = EEPROM_MACADDRESS;
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D) {
#ifdef CONFIG_RTL_92D_DMDP
		if (priv->pshare->wlandev_idx == 0) {
			efuse_MAC = EEPROM_MAC0_MACADDRESS;
		} else {
			efuse_MAC = EEPROM_MAC1_MACADDRESS;
		}
#else
		efuse_MAC = EEPROM_MAC0_MACADDRESS;
#endif
	}
#endif

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8723B)
		efuse_MAC = EEPROM_MAC_ADDR_88E;
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E) {
		efuse_MAC = EEPROM_92E_MACADDRESS;	
	}
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
	if ((GET_CHIP_VER(priv) == VERSION_8812E) ||
		(GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) 
	{
		efuse_MAC = EEPROM_8812_MACADDRESS;
	}
#endif

	//if (/*priv->AutoloadFailFlag==FALSE &&*/ priv->pmib->efuseEntry.enable_efuse == 1) 
	{
		unsigned char *hwinfo = &(priv->EfuseMap[EFUSE_INIT_MAP][0]);
		unsigned char *efuse_mac = hwinfo + efuse_MAC;
		unsigned char zero[] = {0, 0, 0, 0, 0, 0}, mac[6];

		memcpy(mac,efuse_mac, MACADDRLEN);
		/*printk("wlan%d EFUSE MAC [%02x:%02x:%02x:%02x:%02x:%02x]\n", priv->pshare->wlandev_idx,
				*mac, *(mac+1), *(mac+2), *(mac+3), *(mac+4), *(mac+5));*/

		if (!memcmp(mac, zero, MACADDRLEN) || IS_MCAST(mac)) {
			memcpy(mac, zero, MACADDRLEN);
		}
		memcpy(efusemac,mac,MACADDRLEN);

	}
}
#endif

#endif // EN_EFUSE

void ReadEFuseByte(struct rtl8192cd_priv *priv, unsigned short _offset, unsigned char *pbuf)
{
	unsigned int   	value32;
	unsigned char 	readbyte;
	unsigned short 	retry;

#if defined(CONFIG_WLAN_HAL_8192EE)
	if( GET_CHIP_VER(priv) == VERSION_8192E )
	{
		if (!IS_TEST_CHIP(priv) && IS_HARDWARE_TYPE_8192E(priv) && 
			(_GET_HAL_DATA(priv)->cutVersion != ODM_CUT_A) )
		{
			//0x34[11]: SW force PGMEN input of efuse to high. (for the bank selected by 0x34[9:8])		
			RTL_W8(EFUSE_TEST + 1, (RTL_R8(EFUSE_TEST + 1) & 0xf7) | (0x00 << 3));		
		}
	}
#endif
	
	//Write Address
	RTL_W8(EFUSE_CTRL + 1, (_offset & 0xff));
	readbyte = RTL_R8(EFUSE_CTRL + 2);
	RTL_W8(EFUSE_CTRL + 2, ((_offset >> 8) & 0x03) | (readbyte & 0xfc));

	//Write bit 32 0
	readbyte = RTL_R8(EFUSE_CTRL + 3);
	RTL_W8(EFUSE_CTRL + 3, 0x72); //read cmd
	//RTL_W8(EFUSE_CTRL+3, (readbyte & 0x7f));

	//Check bit 32 read-ready
	retry = 0;
	value32 = RTL_R32(EFUSE_CTRL);

	while (!(((value32 >> 24) & 0xff) & 0x80) && (retry < 10000)) {
		value32 = RTL_R32(EFUSE_CTRL);
		retry++;
	}

	// 20100205 Joseph: Add delay suggested by SD1 Victor.
	// This fix the problem that Efuse read error in high temperature condition.
	// Designer says that there shall be some delay after ready bit is set, or the
	// result will always stay on last data we read.
	delay_us(50);
	value32 = RTL_R32(EFUSE_CTRL);

	*pbuf = (unsigned char)(value32 & 0xff);
}	/* ReadEFuseByte */

#if defined(CONFIG_SDIO_HCI) || defined(POWER_TRIM)
#define RF_GAIN_OFFSET_ON       BIT4
#define REG_RF_BB_GAIN          0x55
#define RF_GAIN_OFFSET_MASK     0xfffff
#define EFUSE_GAIN_FLAG_OFFSET  0xC1

/*
  @action=
  	0: set RF gain offset from efuse
  	1: restore original RF gain offset from backup
*/
void power_trim(struct rtl8192cd_priv *priv, unsigned char action)
{
	unsigned short efuse_gain_offset;
	u8 val=0xff, valb;
	u8 patha_4bit, pathb_4bit;
	u32 res, resb;
	char tmp[2];
	int i;
	
#if 0
	// if we want to write a physical address,
	 efuse_PowerSwitch(priv, TRUE, TRUE);
	 WriteEFuseByte(priv, EFUSE_GAIN_OFFSET, 0x60);
	 efuse_PowerSwitch(priv, TRUE, FALSE);
#endif

	if(GET_CHIP_VER(priv) != VERSION_8197F){
		return;
	}


	if(GET_CHIP_VER(priv) == VERSION_8192E){
		efuse_gain_offset = 0x1F6;
	}
#if defined(CONFIG_WLAN_HAL_8197F)	
	else if(GET_CHIP_VER(priv) == VERSION_8197F){
		efuse_gain_offset = 0x51;
	}
#endif	
	else{
		efuse_gain_offset = 0xF6;
	}

	/*set RF gain offset from efuse*/
	if(action == 0){
#if defined(CONFIG_WLAN_HAL_8197F)
		if(GET_CHIP_VER(priv) == VERSION_8197F){
			load_efuse_data_to_reg();
			priv->pshare->kfree_value = val = read_efuse_byte(efuse_gain_offset); /*0x51 = 81*/
			panic_printk("efuse 0x51=%x\n", val);
		}else
#endif		
		{
			ReadEFuseByte(priv, efuse_gain_offset, &val);
		}
		
		valb = ((val & 0xF0) >> 4);
		panic_printk("val & 0xF= %x\n",val & 0xF);

		/* path A is calibrated with K-free*/
		if ( (val & 0xF) == 0xF ) {
			panic_printk("path A no calibrated k-free value!\n");
		}else{
			res = PHY_QueryRFReg(priv, RF92CD_PATH_A, REG_RF_BB_GAIN, bMask20Bits, 1);
			priv->pshare->kfree_reg_backup_pathA = res;
			panic_printk("orig RF A 0x55 = 0x%x\n",res);
#if defined(CONFIG_WLAN_HAL_8197F)			
			if(GET_CHIP_VER(priv) == VERSION_8197F){
				priv->pshare->kfree_reg_0x65_pathA = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x65, BIT17, 1);
				PHY_SetRFReg(priv, RF92CD_PATH_A, 0x65, BIT17, 0);
				/* set RF 0x55[19, 16:14] <-- efuse 0x51[0, 3:1]
				     set RF 0x55[6:5] <--2b'11  */
				res &= 0xFFF63F9F;  /* backup 0x55[19, 16:14, 6:5]*/
				res |= ((val&0xF) & BIT0) << 19;
				res |= (((val&0xF) & (BIT3|BIT2|BIT1))>>1) << 14;
				res |= 3 << 5;
				panic_printk("res=%x\n",res);
			}else
#endif			
			{
				/* set 0x55[18:15]*/
				res &= 0xfff87fff;
				res |= (val & 0x0f)<< 15;
			}
			PHY_SetRFReg(priv, RF92CD_PATH_A, REG_RF_BB_GAIN, bMask20Bits, res);
			val = PHY_QueryRFReg(priv, RF92CD_PATH_A, REG_RF_BB_GAIN, bMask20Bits, 1);		
			panic_printk("write RF A offset 0x%02x val [0x%05x],  read back [0x%05x]\n",
				REG_RF_BB_GAIN, res&0xfffff, val&0xfffff);
		}
		/* path B is calibrated with K-free*/
		if ( valb == 0xF ) {
			panic_printk("path B no calibrated k-free value!\n");
		}else{
			res = PHY_QueryRFReg(priv, RF92CD_PATH_B, REG_RF_BB_GAIN, bMask20Bits, 1);
			priv->pshare->kfree_reg_backup_pathB = res;
			panic_printk("orig RF B 0x55 = 0x%x\n",res);
#if defined(CONFIG_WLAN_HAL_8197F)			
			if(GET_CHIP_VER(priv) == VERSION_8197F){
				priv->pshare->kfree_reg_0x65_pathB = PHY_QueryRFReg(priv, RF92CD_PATH_B, 0x65, BIT17, 1);
				PHY_SetRFReg(priv, RF92CD_PATH_B, 0x65, BIT17, 0);
				/* set RF 0x55[19, 16:14] <-- efuse 0x51[0, 3:1]
				     set RF 0x55[6:5] <--2b'11  */
				res &= 0xFFF63F9F;  /* backup 0x55[19, 16:14, 6:5]*/
				res |= ((val&0xF) & BIT0) << 19;
				res |= (((val&0xF) & (BIT3|BIT2|BIT1))>>1) << 14;
				res |= 3 << 5;
				panic_printk("res=%x\n",res);
			}else
#endif			
			{
				/* set 0x55[18:15]*/
				res &= 0xfff87fff;
				res |= (val & 0x0f) << 15;
			}
			PHY_SetRFReg(priv, RF92CD_PATH_B, REG_RF_BB_GAIN, bMask20Bits, res);
			val = PHY_QueryRFReg(priv, RF92CD_PATH_B, REG_RF_BB_GAIN, bMask20Bits, 1);		
			panic_printk("write RF B offset 0x%02x val [0x%05x],  read back [0x%05x]\n",
				REG_RF_BB_GAIN, res&0xfffff, val&0xfffff);
		}
	}/*restore original RF gain offset from backup*/
	else if(action == 1){
		/*Restore path A 0x55*/
		if(priv->pshare->kfree_reg_backup_pathA){
			panic_printk("orig RF A 0x55 = 0x%x\n", priv->pshare->kfree_reg_backup_pathA);
			res = PHY_QueryRFReg(priv, RF92CD_PATH_A, REG_RF_BB_GAIN, bMask20Bits, 1);
#if defined(CONFIG_WLAN_HAL_8197F)			
			if(GET_CHIP_VER(priv) == VERSION_8197F){
				PHY_SetRFReg(priv, RF92CD_PATH_A, 0x65, BIT17, priv->pshare->kfree_reg_0x65_pathA&BIT0);
				/* RF 0x55[19, 16:14]*/
				res &= 0xFFF63F9F;
				res |= (priv->pshare->kfree_reg_backup_pathA & (BIT19|BIT16|BIT15|BIT14|BIT6|BIT5));
			}else
#endif			
			{
				/* RF 0x55[18:15]*/
				res &= 0xfff87fff;			
				res |= (priv->pshare->kfree_reg_backup_pathA & (BIT18|BIT17|BIT16|BIT15));
			}
			PHY_SetRFReg(priv, RF92CD_PATH_A, 0x55, bMask20Bits, res);
		}else{
			panic_printk("path A didn't apply k-free\n");
		}
		
		/*Restore path B 0x55*/
		if(priv->pshare->kfree_reg_backup_pathB){			
			panic_printk("orig RF B 0x55 = 0x%x\n", priv->pshare->kfree_reg_backup_pathB);
			res = PHY_QueryRFReg(priv, RF92CD_PATH_B, REG_RF_BB_GAIN, bMask20Bits, 1);
#if defined(CONFIG_WLAN_HAL_8197F)			
			if(GET_CHIP_VER(priv) == VERSION_8197F){
				PHY_SetRFReg(priv, RF92CD_PATH_B, 0x65, BIT17, priv->pshare->kfree_reg_0x65_pathB&BIT0);
				/* RF 0x55[19, 16:14]*/
				res &= 0xFFF63F9F;
				res |= (priv->pshare->kfree_reg_backup_pathB & (BIT19|BIT16|BIT15|BIT14|BIT6|BIT5));
			}else
#endif			
			{
				/* RF 0x55[18:15]*/
				res &= 0xfff87fff;			
				res |= (priv->pshare->kfree_reg_backup_pathB & (BIT18|BIT17|BIT16|BIT15));
			}
			PHY_SetRFReg(priv, RF92CD_PATH_B, 0x55, bMask20Bits, res);		
		}else{
			panic_printk("path B didn't apply k-free\n");
		}
	}else{
		panic_printk("Error: Not support action %d for kfree()\n", action);
	}
}
#endif

#ifdef POWER_TRIM
void do_kfree(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *value;
	u8 val,valb;
	u32 res,resb;

	if (!netif_running(priv->dev)) {
		panic_printk("\nFail: interface not opened\n");
		return;
	}
	
	if (IS_D_CUT_8192E(priv) == FALSE && !GET_CHIP_VER(priv) == VERSION_8197F) {
		panic_printk("Fail: %s() only support 97F/ 92E D-cut !\n", __FUNCTION__);
		return;
	}
	
	value = get_value_by_token((char *)data, "on");
	if (value) {
		if (priv->pmib->dot11RFEntry.kfree_enable)
			return;
		
		power_trim(priv, 0);			
		priv->pmib->dot11RFEntry.kfree_enable = 1;
		panic_printk("kfree on\n");
	}

	value = get_value_by_token((char *)data, "off");
	if (value) {
		if (priv->pmib->dot11RFEntry.kfree_enable == 0)
			return;
		power_trim(priv, 1);
		priv->pmib->dot11RFEntry.kfree_enable = 0;
		panic_printk("kfree off\n");
	}
}
#endif

#ifdef THER_TRIM
/* 
*  load thermal trim value from efuse
*/
void ther_trim_efuse(struct rtl8192cd_priv *priv){
	unsigned short efuse_gain_offset;
	u8 val=0xff;

	if(GET_CHIP_VER(priv) != VERSION_8197F){
		return;
	}

#if defined(CONFIG_WLAN_HAL_8197F)	
	if(GET_CHIP_VER(priv) == VERSION_8197F){
		efuse_gain_offset = 0x50; /* efuse thermal trim offset*/
		load_efuse_data_to_reg();
		val = read_efuse_byte(efuse_gain_offset); /*0x51 = 81*/
		
	/* fake value */
//	val = 14;
		
		phy_printk("efuse 0x50=%x\n", val);
		if(val == 0xFF){
			phy_printk("efuse 0xEF=0xFF, not to apply thermal trim\n");
			priv->pshare->rf_ft_var.ther_trim_val = 0;
			return;
		}else{
			if(val & 0x1){ /* positive */
				priv->pshare->rf_ft_var.ther_trim_val = val>>1;
				phy_printk(" positive ther_trim_val=%d\n", priv->pshare->rf_ft_var.ther_trim_val);				
			}else{ /* negative */				
				priv->pshare->rf_ft_var.ther_trim_val = -(val>>1);
				phy_printk(" negative ther_trim_val=%d\n", priv->pshare->rf_ft_var.ther_trim_val);
			}
			if(priv->pshare->rf_ft_var.ther_trim_val < -12 || priv->pshare->rf_ft_var.ther_trim_val > 12){
				priv->pshare->rf_ft_var.ther_trim_val = 0;
				phy_printk(" unreasonable value, fineal ther_trim_val=%d\n", priv->pshare->rf_ft_var.ther_trim_val);
			}
		}
	}
#endif
}

/*
* apply or un-do thermal trim value to mib ther
*/
void ther_trim_act(struct rtl8192cd_priv *priv, unsigned char action){
	int val;
	
	if(GET_CHIP_VER(priv) != VERSION_8197F)
		return;

	if(priv->pshare->rf_ft_var.ther_trim_val < -12 || priv->pshare->rf_ft_var.ther_trim_val > 12)
		return;

	/*set apply thermal trim*/
	if(action == 0){
		val = priv->pmib->dot11RFEntry.ther + priv->pshare->rf_ft_var.ther_trim_val;
		GDEBUG("Apply thermal trim\n");
	}else{ /*undo thermal trim*/
		val = priv->pmib->dot11RFEntry.ther - priv->pshare->rf_ft_var.ther_trim_val;
		phy_printk("Undo thermal trim\n");
	}
		
	if(val < 0)
		priv->pmib->dot11RFEntry.ther = 0;
	else
		priv->pmib->dot11RFEntry.ther = val;
	phy_printk("ther_trim_val=%d, final ther=%u\n", priv->pshare->rf_ft_var.ther_trim_val, priv->pmib->dot11RFEntry.ther);
}
#endif

#if defined(CONFIG_WLAN_HAL_8822BE)
// for 8822B PCIE D-cut patch only
// Normal driver and MP driver need this patch
//
void _setTxACaliValue(
	struct rtl8192cd_priv *priv,
	IN	u1Byte	 eRFPath,
	IN	u1Byte	 offset,
	IN	u1Byte	 TxABiaOffset
	)
{
	u4Byte modiTxAValue = 0;
	u1Byte tmp1Byte = 0;
	BOOLEAN bMinus = FALSE;
	u1Byte compValue = 0;
	
	switch(offset)
	{
		case 0x0:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X10124);
			break;
		case 0x1:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X10524);
			break;
		case 0x2:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X10924);
			break;
		case 0x3:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X10D24);
			break;
		case 0x4:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X30164);
			break;
		case 0x5:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X30564);
			break;
		case 0x6:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X30964);
			break;
		case 0x7:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X30D64);
			break;
		case 0x8:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X50195);
			break;
		case 0x9:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X50595);
			break;
		case 0xa:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X50995);
			break;
		case 0xb:
			PHY_SetRFReg(priv, eRFPath, 0x18, 0xFFFFF, 0X50D95);
			break;
		default:
			panic_printk("Invalid TxA band offset...\n");
			return;
			break;
	}

	// Get TxA value
	modiTxAValue = PHY_QueryRFReg(priv, eRFPath, 0x61, 0xFFFFF,1);
	tmp1Byte = (u1Byte)modiTxAValue&(BIT3|BIT2|BIT1|BIT0);

	// check how much need to calibration
	switch(TxABiaOffset)
	{
		case 0xF8:
			bMinus = TRUE;
			compValue = 4;
			break;
			
		case 0xF6:
			bMinus = TRUE;
			compValue = 3;
			break;
			
		case 0xF4:
			bMinus = TRUE;
			compValue = 2;
			break;
			
		case 0xF2:
			bMinus = TRUE;
			compValue = 1;
			break;
			
		case 0xF3:
			bMinus = FALSE;
			compValue = 1;
			break;
			
		case 0xF5:
			bMinus = FALSE;
			compValue = 2;	
			break;
			
		case 0xF7:
			bMinus = FALSE;
			compValue = 3;
			break;
			
		case 0xF9:
			bMinus = FALSE;
			compValue = 4;
			break;
		
		// do nothing case
		case 0xF0:
		default:
			return;
			break;
	}

	// calc correct value to calibrate
	if(bMinus)
	{
		if(tmp1Byte >= compValue)
		{
			tmp1Byte -= compValue;
			//modiTxAValue += tmp1Byte;
		}
		else
		{
			tmp1Byte = 0;
		}
	}
	else
	{
		tmp1Byte += compValue;
		if(tmp1Byte >= 7)
		{
			tmp1Byte = 7;
		}
	}
	
	// Write back to RF reg
	PHY_SetRFReg(priv, eRFPath, 0x30, 0xFFFF, (offset<<12|(modiTxAValue&0xFF0)|tmp1Byte));
}

void _txaBiasCali4eachPath(
	struct rtl8192cd_priv *priv,
	IN	u1Byte	 eRFPath,
	IN	u1Byte	 efuseValue
	)
{
	// switch on set TxA bias
	PHY_SetRFReg(priv, eRFPath, 0xEF, 0xFFFFF, 0x200);

	// Set 12 sets of TxA value
	_setTxACaliValue(priv, eRFPath, 0x0, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x1, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x2, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x3, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x4, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x5, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x6, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x7, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x8, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0x9, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0xa, efuseValue);
	_setTxACaliValue(priv, eRFPath, 0xb, efuseValue);

	// switch off set TxA bias
	PHY_SetRFReg(priv, eRFPath, 0xEF, 0xFFFFF, 0x0);
}

void TxACurrentCalibration(struct rtl8192cd_priv *priv)
{
	//HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(priv);
	//PEFUSE_HAL		pEfuseHal = &(pHalData->EfuseHal);
	//u1Byte 			eFuseContent[DCMD_EFUSE_MAX_SECTION_NUM * EFUSE_MAX_WORD_UNIT * 2];
	u1Byte			efuse0x3D8, efuse0x3D7;
	u4Byte			origRF0x18PathA = 0, origRF0x18PathB = 0;
	u8 val=0xff;
	// save original 0x18 value
	origRF0x18PathA = PHY_QueryRFReg(priv, ODM_RF_PATH_A, 0x18, 0xFFFFF,1);
	origRF0x18PathB = PHY_QueryRFReg(priv, ODM_RF_PATH_B, 0x18, 0xFFFFF,1);
	
	
#if 0
	// if we want to write a physical address,
	 efuse_PowerSwitch(priv, TRUE, TRUE);
	 WriteEFuseByte(priv, 0x3D7, 0xF6);
	 WriteEFuseByte(priv, 0x3D8, 0xF9);
	 efuse_PowerSwitch(priv, TRUE, FALSE);
#endif
	// Step 1 : read efuse content
#if 0	
	GET_MACHALAPI_INTERFACE(priv)->halmac_dump_efuse_map(priv->pHalmac_adapter,1);
	efuse0x3D7=*(priv->pHalmac_adapter->pHalEfuse_map+0x3D7);
	efuse0x3D8=*(priv->pHalmac_adapter->pHalEfuse_map+0x3D8);

	panic_printk("********************************\n");
	panic_printk("8822 efuse content 0x3D7 = 0x%x\n",efuse0x3D7);
	panic_printk("8822 efuse content 0x3D8 = 0x%x\n",efuse0x3D8);
	panic_printk("********************************\n");
#endif	
#if 1
	ReadEFuseByte(priv, 0x3D7, &efuse0x3D7);
	panic_printk("efuse content 0x3D7 = 0x%x\n",efuse0x3D7);
	ReadEFuseByte(priv, 0x3D8, &efuse0x3D8);
	panic_printk("efuse content 0x3D8 = 0x%x\n",efuse0x3D8);
#endif
	// Step 2 : check efuse content to judge whether need to calibration or not
	if(efuse0x3D7 == 0xFF)
	{
		panic_printk("efuse content 0x3D7 == 0xFF, No need to do TxA cali\n");
		return;
	}

	// Step 3 : write RF register for calibration
	_txaBiasCali4eachPath(priv, ODM_RF_PATH_A, efuse0x3D7);
	_txaBiasCali4eachPath(priv, ODM_RF_PATH_B, efuse0x3D8);
	

	// restore original 0x18 value
	PHY_SetRFReg(priv, ODM_RF_PATH_A, 0x18, 0xFFFFF, origRF0x18PathA);
	PHY_SetRFReg(priv, ODM_RF_PATH_B, 0x18, 0xFFFFF, origRF0x18PathB);
}

#endif


#ifdef TX_EARLY_MODE
void enable_em(struct rtl8192cd_priv *priv)
{
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		RTL_W32(EARLY_MODE_CTRL, RTL_R32(EARLY_MODE_CTRL) | 0x8000001f);
		priv->pshare->aggrmax_bak = RTL_R16(PROT_MODE_CTRL + 2);
		//RTL_W16(PROT_MODE_CTRL+2, 0x0808);
		RTL_W16(PROT_MODE_CTRL+2, 0x0c0c);
	} else
#endif
	{
		RTL_W32(EARLY_MODE_CTRL, RTL_R32(EARLY_MODE_CTRL) | 0x8000000f); // enable signel AMPDU, early mode for vi/vo/be/bk queue
	}

	RTL_W16(TCR, RTL_R16(TCR) | WMAC_TCR_ERRSTEN2);
}

void disable_em(struct rtl8192cd_priv *priv)
{
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		RTL_W32(EARLY_MODE_CTRL, RTL_R32(EARLY_MODE_CTRL) & ~0x8000001f);
		if (priv->pshare->aggrmax_bak != 0)
			RTL_W16(PROT_MODE_CTRL + 2, (priv->pshare->aggrmax_bak & 0xffff));
	} else
#endif
	{
		RTL_W32(EARLY_MODE_CTRL, RTL_R32(EARLY_MODE_CTRL) & ~0x8000000f); // disable signel AMPDU, early mode for vi/vo/be/bk queue
	}

	RTL_W16(TCR, RTL_R16(TCR) & ~WMAC_TCR_ERRSTEN2 );
}
#endif



#ifdef RTLWIFINIC_GPIO_CONTROL


void reg_bit_set(struct rtl8192cd_priv *priv, unsigned int reg, unsigned int value)
{
	unsigned int tmp_value = 0;

	if(!(reg%4)){
		tmp_value = RTL_R32(reg);
		tmp_value |= value;
		RTL_W32(reg, tmp_value);
	}
	else if(!(reg%2)){
		tmp_value = RTL_R16(reg);
		tmp_value |= value;
		RTL_W16(reg, tmp_value);
	}
	else {
		tmp_value = RTL_R8(reg);
		tmp_value |= value;
		RTL_W8(reg, tmp_value);
	}
}

void reg_bit_clear(struct rtl8192cd_priv *priv, unsigned int reg, unsigned int value)
{
	unsigned int tmp_value = 0;

	if(!(reg%4)){
		tmp_value = RTL_R32(reg);
		tmp_value &= (~value);
		RTL_W32(reg, tmp_value);
	}
	else if(!(reg%2)){
		tmp_value = RTL_R16(reg);
		tmp_value &= (~value);
		RTL_W16(reg, tmp_value);
	}
	else {
		tmp_value = RTL_R8(reg);
		tmp_value &= (~value);
		RTL_W8(reg, tmp_value);
	}
}

void assign_pin_as_gpio(struct rtl8192cd_priv *priv, unsigned int num)
{
	//need to assign some specific values to assign pin fun as gpio (follow docu from MAC team)

	switch(num)
	{
		case 0:
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(2)|BIT(10)));
				reg_bit_set(priv, 0x40, BIT(9));
				reg_bit_clear(priv, 0x66, (BIT(0)|BIT(1)|BIT(2)));

				return;
		case 1:
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(2)|BIT(10)));
				reg_bit_set(priv, 0x40, BIT(9));
				reg_bit_clear(priv, 0x66, (BIT(0)|BIT(1)|BIT(2)));

				return;			
		case 2:
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(2)|BIT(10)));
				reg_bit_set(priv, 0x40, BIT(9));
				reg_bit_clear(priv, 0x66, (BIT(0)|BIT(1)|BIT(2)));
				reg_bit_clear(priv, 0x54, BIT(18));

				return;
		case 3:
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(2)|BIT(10)));
				reg_bit_set(priv, 0x40, BIT(9));
				reg_bit_clear(priv, 0x66, (BIT(0)|BIT(1)|BIT(2)));
				reg_bit_clear(priv, 0x54, BIT(18));

				return;
		case 4:
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(19)));
				reg_bit_clear(priv, 0x66, (BIT(4)|BIT(8)));
				reg_bit_clear(priv, 0x70, (BIT(18)|BIT(27)));
			
				return;

		case 5:
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(19)));
				reg_bit_clear(priv, 0x66, (BIT(4)|BIT(8)));
				reg_bit_clear(priv, 0x70, BIT(27));
			
				return;

		case 6:
				reg_bit_clear(priv, 0x38, (BIT(3)|BIT(6)));
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(4)|BIT(19)));
				reg_bit_clear(priv, 0x66, (BIT(4)|BIT(8)));
				reg_bit_clear(priv, 0x64, BIT(25));

				return;
		case 7:
				reg_bit_clear(priv, 0x38, (BIT(3)|BIT(6)));
				reg_bit_clear(priv, 0x40, (BIT(0)|BIT(1)|BIT(4)|BIT(8)|BIT(19)));
				reg_bit_clear(priv, 0x66, (BIT(4)|BIT(8)));
				reg_bit_clear(priv, 0x64, BIT(26));
			
				return;
		case 8:
				reg_bit_clear(priv, 0x48, BIT(16));
				reg_bit_clear(priv, 0x4c, BIT(21));
			
				return;

		case 9:
				reg_bit_clear(priv, 0x48, BIT(16));
				reg_bit_clear(priv, 0x66, BIT(6));
				reg_bit_clear(priv, 0x68, (BIT(0)|BIT(3)));

				return;
				
		case 10:

				return;
				
		case 11:

				reg_bit_clear(priv, 0x40, BIT(27));
				reg_bit_clear(priv, 0x66, BIT(6));
				reg_bit_clear(priv, 0x6a, (BIT(0)|BIT(3)));
				
				return;
				
		case 12:

				reg_bit_clear(priv, 0x38, BIT(6));
				reg_bit_clear(priv, 0x66, BIT(6));
				
				return;
				
		case 13:
			
				reg_bit_clear(priv, 0x4c, BIT(22));
				return;
				
		case 14:

				reg_bit_clear(priv, 0x4c, BIT(22));
				return;

		case 15:

				reg_bit_clear(priv, 0x66, BIT(7));
				return;

		default:
			
			return;

	}

}

void write_gpio_8822(struct rtl8192cd_priv *priv, unsigned int num, unsigned int value)
{
	unsigned int reg_gpio_ctrl = 0x0;
	unsigned int offset = 0;
	unsigned int tmp_value = 0x0;

	if(num < 8)
	{
		reg_gpio_ctrl = 0x44;
		offset = num;
	}
	else if(num < 16) 
	{
		reg_gpio_ctrl = 0x60;
		offset = (num - 8);
	}
	else
		return;

	//panic_printk("[%s] num=%d value=%d \n", __FUNCTION__, num, value);

	tmp_value = RTL_R32(reg_gpio_ctrl);

	if(value)
		tmp_value |= (BIT(8+offset)); 
	else
		tmp_value &= (~(BIT(8+offset)));

	RTL_W32(reg_gpio_ctrl, tmp_value);

}


int read_gpio_8822(struct rtl8192cd_priv *priv, unsigned int num)
{
	unsigned int reg_gpio_ctrl = 0x0;
	unsigned int offset = 0;
	unsigned int tmp_value = 0x0;
	unsigned int ret = 0;

	if(num < 8)
	{
		reg_gpio_ctrl = 0x44;
		offset = num;
	}
	else if(num < 16) 
	{
		reg_gpio_ctrl = 0x60;
		offset = (num - 8);
	}
	else
		return;

	tmp_value = RTL_R32(reg_gpio_ctrl);

	if(tmp_value & BIT(offset))
		ret = 1;
	else
		ret = 0;

	//panic_printk("[%s] num=%d ret=%d \n", __FUNCTION__, num, ret);

	return ret;

}

void config_gpio_8822(struct rtl8192cd_priv *priv, unsigned int num, unsigned int dir)
{
	unsigned int reg_gpio_ctrl = 0x0;
	unsigned int offset = 0;
	unsigned int tmp_value = 0x0;

	if(num < 8)
	{
		reg_gpio_ctrl = 0x44;
		offset = num;
	}
	else if(num < 16) 
	{
		reg_gpio_ctrl = 0x60;
		offset = (num - 8);
	}
	else
		return;

	//panic_printk("[%s] num=%d dir=0x%x \n", __FUNCTION__, num, dir);

	priv->pshare->phw->GPIO_dir[num] = dir;

	assign_pin_as_gpio(priv, num);

	tmp_value = RTL_R32(reg_gpio_ctrl);
	tmp_value &= (~(BIT(24+offset))); //clear interrupt mode 

	if(dir == 0x10) 
		tmp_value |= (BIT(16+offset)); //gpio = output
	else if(dir == 0x1) 
		tmp_value &= (~(BIT(16+offset))); //gpio = input

	RTL_W32(reg_gpio_ctrl, tmp_value);

}


struct rtl8192cd_priv *root_priv;

void RTLWIFINIC_GPIO_init_priv(struct rtl8192cd_priv *priv)
{
	root_priv = priv;
}

void RTLWIFINIC_GPIO_config(unsigned int gpio_num, unsigned int direction)
{
	struct rtl8192cd_priv *priv = root_priv;
	
	if (!root_priv)
		return;

	if (GET_CHIP_VER(priv) == VERSION_8822B){
		
		config_gpio_8822(priv, gpio_num, direction);
		
		return;
	}

#ifdef PCIE_POWER_SAVING
	PCIeWakeUp(priv, POWER_DOWN_T0);
#endif

	if ((gpio_num >= 0) && (gpio_num <= 7)) {
		priv->pshare->phw->GPIO_dir[gpio_num] = direction;

		if (direction == 0x01) {
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) & ~(BIT(gpio_num + 24) | BIT(gpio_num + 16)));
			return;
		} else if (direction == 0x10) {
			RTL_W32(GPIO_PIN_CTRL, (RTL_R32(GPIO_PIN_CTRL) & ~BIT(gpio_num + 24)) | (BIT(gpio_num + 16) | BIT(gpio_num + 8)));
			return;
		}
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) {
		if ((gpio_num >= 8) && (gpio_num <= 15)) {
			priv->pshare->phw->GPIO_dir[gpio_num] = direction;

			if (direction == 0x01) {
				RTL_W32(0x060, RTL_R32(0x060) & ~(BIT(gpio_num + 16) | BIT(gpio_num + 8)));
				return;
			} else if (direction == 0x10) {
				RTL_W32(0x060, (RTL_R32(0x060) & ~BIT(gpio_num + 16)) | (BIT(gpio_num + 8) | BIT(gpio_num)));
				return;
			}
		}
	}
	else {
		if ((gpio_num >= 8) && (gpio_num <= 11)) {
			priv->pshare->phw->GPIO_dir[gpio_num] = direction;

			if (direction == 0x01) {
				RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) & ~(BIT(gpio_num + 20) | BIT(gpio_num + 16)));
				return;
			} else if (direction == 0x10) {
				RTL_W32(GPIO_MUXCFG, (RTL_R32(GPIO_MUXCFG) & ~BIT(gpio_num + 20)) | (BIT(gpio_num + 16) | BIT(gpio_num + 12)));
				return;
			}
		}
	}

	panic_printk("GPIO %d action %d not support!\n", gpio_num, direction);
	return;
}

void RTLWIFINIC_GPIO_write(unsigned int gpio_num, unsigned int value)
{

	struct rtl8192cd_priv *priv = root_priv;
	if (!root_priv)
		return;
	
#ifdef PCIE_POWER_SAVING
	PCIeWakeUp(priv, POWER_DOWN_T0);
#endif

	if (priv->pshare->phw->GPIO_dir[gpio_num] != 0x10)
		RTLWIFINIC_GPIO_config(gpio_num, 0x10);

	if (GET_CHIP_VER(priv) == VERSION_8822B){
		write_gpio_8822(priv, gpio_num, value);
		return;
	}

	if (((gpio_num >= 0) && (gpio_num <= 7)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x10)) {
		if (value)
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) & ~BIT(gpio_num + 8));
		else
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) | BIT(gpio_num + 8));
		return;
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) {
		if (((gpio_num >= 8) && (gpio_num <= 15)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x10)) {
			if (value)
				RTL_W32(0x060, RTL_R32(0x060) & ~BIT(gpio_num));
			else
				RTL_W32(0x060, RTL_R32(0x060) | BIT(gpio_num));
			return;
		}
	}
	else {
		if (((gpio_num >= 8) && (gpio_num <= 11)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x10)) {
			if (value)
				RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) & ~BIT(gpio_num + 12));
			else
				RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) | BIT(gpio_num + 12));
			return;
		}
	}

	panic_printk("GPIO %d set value %d not support!\n", gpio_num, value);
	return;
}


int RTLWIFINIC_GPIO_read(unsigned int gpio_num)
{
	struct rtl8192cd_priv *priv = root_priv;
	unsigned int val32;
	if (!root_priv)
		return -1;

	if (priv->pshare->phw->GPIO_dir[gpio_num] != 0x01)
		RTLWIFINIC_GPIO_config(gpio_num, 0x01);

	if (GET_CHIP_VER(priv) == VERSION_8822B){
		return read_gpio_8822(priv, gpio_num);
	}

	if (((gpio_num >= 0) && (gpio_num <= 7)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x01)) {
#ifdef PCIE_POWER_SAVING
		if ((priv->pwr_state == L2) || (priv->pwr_state == L1))
			val32 = priv->pshare->phw->GPIO_cache[0];
		else
#endif
			val32 = RTL_R32(GPIO_PIN_CTRL);
		if (val32 & BIT(gpio_num))
			return 0;
		else
			return 1;
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) {
		if (((gpio_num >= 8) && (gpio_num <= 15)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x01)) {
			val32 = RTL_R32(0x060);
			if (val32 & BIT(gpio_num - 8))
				return 0;
			else
				return 1;
		}
	}
	else {
		if (((gpio_num >= 8) && (gpio_num <= 11)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x01)) {
#ifdef PCIE_POWER_SAVING
			if ((priv->pwr_state == L2) || (priv->pwr_state == L1))
				val32 = priv->pshare->phw->GPIO_cache[1];
			else
#endif
				val32 = RTL_R32(GPIO_MUXCFG);
			if (val32 & BIT(gpio_num + 8))
				return 0;
			else
				return 1;
		}
	}

	panic_printk("GPIO %d get value not support!\n", gpio_num);
	return -1;
}

void RTLWIFINIC_GPIO_config_proc(struct rtl8192cd_priv *priv, unsigned int gpio_num, unsigned int direction)
{

#ifdef PCIE_POWER_SAVING
	PCIeWakeUp(priv, POWER_DOWN_T0);
#endif

	if ((gpio_num >= 0) && (gpio_num <= 7)) {
		priv->pshare->phw->GPIO_dir[gpio_num] = direction;

		if (direction == 0x01) {
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) & ~(BIT(gpio_num + 24) | BIT(gpio_num + 16)));
			return;
		} else if (direction == 0x10) {
			RTL_W32(GPIO_PIN_CTRL, (RTL_R32(GPIO_PIN_CTRL) & ~BIT(gpio_num + 24)) | (BIT(gpio_num + 16) | BIT(gpio_num + 8)));
			return;
		}
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) {
		if ((gpio_num >= 8) && (gpio_num <= 15)) {
			priv->pshare->phw->GPIO_dir[gpio_num] = direction;

			if (direction == 0x01) {
				RTL_W32(0x060, RTL_R32(0x060) & ~(BIT(gpio_num + 16) | BIT(gpio_num + 8)));
				return;
			} else if (direction == 0x10) {
				RTL_W32(0x060, (RTL_R32(0x060) & ~BIT(gpio_num + 16)) | (BIT(gpio_num + 8) | BIT(gpio_num)));
				return;
			}
		}
	}
	else {
		if ((gpio_num >= 8) && (gpio_num <= 11)) {
			priv->pshare->phw->GPIO_dir[gpio_num] = direction;

			if (direction == 0x01) {
				RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) & ~(BIT(gpio_num + 20) | BIT(gpio_num + 16)));
				return;
			} else if (direction == 0x10) {
				RTL_W32(GPIO_MUXCFG, (RTL_R32(GPIO_MUXCFG) & ~BIT(gpio_num + 20)) | (BIT(gpio_num + 16) | BIT(gpio_num + 12)));
				return;
			}
		}
	}

	panic_printk("GPIO %d action %d not support!\n", gpio_num, direction);
	return;
}

void RTLWIFINIC_GPIO_write_proc(struct rtl8192cd_priv *priv, unsigned int gpio_num, unsigned int value)
{
#ifdef PCIE_POWER_SAVING
	PCIeWakeUp(priv, POWER_DOWN_T0);
#endif

	if (priv->pshare->phw->GPIO_dir[gpio_num] != 0x10)
		RTLWIFINIC_GPIO_config(gpio_num, 0x10);

	if (((gpio_num >= 0) && (gpio_num <= 7)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x10)) {
		if (value)
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) & ~BIT(gpio_num + 8));
		else
			RTL_W32(GPIO_PIN_CTRL, RTL_R32(GPIO_PIN_CTRL) | BIT(gpio_num + 8));
		return;
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) {
		if (((gpio_num >= 8) && (gpio_num <= 15)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x10)) {
			if (value)
				RTL_W32(0x060, RTL_R32(0x060) & ~BIT(gpio_num));
			else
				RTL_W32(0x060, RTL_R32(0x060) | BIT(gpio_num));
			return;
		}
	}
	else {
		if (((gpio_num >= 8) && (gpio_num <= 11)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x10)) {
			if (value)
				RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) & ~BIT(gpio_num + 12));
			else
				RTL_W32(GPIO_MUXCFG, RTL_R32(GPIO_MUXCFG) | BIT(gpio_num + 12));
			return;
		}
	}

	panic_printk("GPIO %d set value %d not support!\n", gpio_num, value);
	return;
}


int RTLWIFINIC_GPIO_read_proc(struct rtl8192cd_priv *priv, unsigned int gpio_num)
{
	unsigned int val32;

	if (priv->pshare->phw->GPIO_dir[gpio_num] != 0x01)
		RTLWIFINIC_GPIO_config(gpio_num, 0x01);

	if (((gpio_num >= 0) && (gpio_num <= 7)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x01)) {
#ifdef PCIE_POWER_SAVING
		if ((priv->pwr_state == L2) || (priv->pwr_state == L1))
			val32 = priv->pshare->phw->GPIO_cache[0];
		else
#endif
			val32 = RTL_R32(GPIO_PIN_CTRL);
		if (val32 & BIT(gpio_num))
			return 0;
		else
			return 1;
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8814A) ||
		(GET_CHIP_VER(priv) == VERSION_8822B)) {
		if (((gpio_num >= 8) && (gpio_num <= 15)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x01)) {
			val32 = RTL_R32(0x060);
			if (val32 & BIT(gpio_num - 8))
				return 0;
			else
				return 1;
		}
	}
	else {
		if (((gpio_num >= 8) && (gpio_num <= 11)) && (priv->pshare->phw->GPIO_dir[gpio_num] == 0x01)) {
#ifdef PCIE_POWER_SAVING
			if ((priv->pwr_state == L2) || (priv->pwr_state == L1))
				val32 = priv->pshare->phw->GPIO_cache[1];
			else
#endif
				val32 = RTL_R32(GPIO_MUXCFG);
			if (val32 & BIT(gpio_num + 8))
				return 0;
			else
				return 1;
		}
	}

	panic_printk("GPIO %d get value not support!\n", gpio_num);
	return -1;
}
#endif

#ifdef CONFIG_RTL8672
// Note: The following GPIO enumeration must have the same order with the RTL8192CD_GPIO of
//       GPIO_DEF enumeration in gpio.h
enum WIFI_GPIO {
	WIFI_GPIO_0 = 0, WIFI_GPIO_1, WIFI_GPIO_2, WIFI_GPIO_3,
	WIFI_GPIO_4, WIFI_GPIO_5, WIFI_GPIO_6, WIFI_GPIO_7,
	WIFI_GPIO_8, WIFI_GPIO_9, WIFI_GPIO_10, WIFI_GPIO_11,
	WIFI_GPIO_12, WIFI_GPIO_13, WIFI_GPIO_14, WIFI_GPIO_15,
	WIFI_GPIO_92E_ANTSEL_P
};

void rtl8192cd_gpio_config(unsigned int wlan_idx, unsigned int gpio_num, int direction)
{
	struct rtl8192cd_priv *priv;
	unsigned int regval, mask;
	
//	if (wlan_idx >= sizeof(wlan_device)/sizeof(struct _device_info_))
//		return;
#if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
	if (wlan_idx >= 2)
		return;
#else
	wlan_idx = 0;
#endif
	
	priv = wlan_device[wlan_idx].priv;
	if (NULL == priv)
		return;
	
	if ((gpio_num >= 0) && (gpio_num <= 7)) {
		regval = RTL_R32(GPIO_PIN_CTRL);
		mask = BIT(gpio_num + 16);
		if (regval & mask) {
			if ((regval & BIT(gpio_num + 24)) || (direction == 0x01))	// input
				RTL_W32(GPIO_PIN_CTRL, regval & ~(BIT(gpio_num + 24) | mask));
		} else {
			if ((regval & BIT(gpio_num + 24)) || (direction == 0x10))	// output
				RTL_W32(GPIO_PIN_CTRL, (regval & ~ BIT(gpio_num + 24)) | mask);
		}
		return;
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A)) {
		if ((gpio_num >= 8) && (gpio_num <= 15)) {
			regval = RTL_R32(0x060);
			mask = BIT(gpio_num + 8);
			if (regval & mask) {
				if ((regval & BIT(gpio_num + 16)) || (direction == 0x01))	// input
					RTL_W32(0x060, regval & ~(BIT(gpio_num + 16) | mask));
			} else {
				if ((regval & BIT(gpio_num + 16)) || (direction == 0x10))	// output
					RTL_W32(0x060, (regval & ~ BIT(gpio_num + 16)) | mask);
			}
			return;
		} else if (gpio_num == WIFI_GPIO_92E_ANTSEL_P) {
			if (GET_CHIP_VER(priv) == VERSION_8192E) {
				regval = RTL_R32(LEDCFG);
				if (!(regval & BIT23))
					RTL_W32(LEDCFG, regval | BIT23);	// SW control ANT_SEL[P:N]
				return;
			}
		}
	}
	else {
		if ((gpio_num >= 8) && (gpio_num <= 11)) {
			regval = RTL_R32(GPIO_MUXCFG);
			mask = BIT(gpio_num + 16);
			if (regval & mask) {
				if ((regval & BIT(gpio_num + 20)) || (direction == 0x01))	// input
					RTL_W32(GPIO_MUXCFG, regval & ~(BIT(gpio_num + 20) | mask));
			} else {
				if ((regval & BIT(gpio_num + 20)) || (direction == 0x10))	// output
					RTL_W32(GPIO_MUXCFG, (regval & ~ BIT(gpio_num + 20)) | mask);
			}
			return;
		}
	}
}
EXPORT_SYMBOL(rtl8192cd_gpio_config);

#if defined (CONFIG_WIFI_SIMPLE_CONFIG) && defined (CONFIG_CT_PUSHKEYEVENT)
void rtl8192cd_gpio_config_interrupt(unsigned int wlan_idx, unsigned int gpio_num)
{
	struct rtl8192cd_priv *priv;
	unsigned int regval;
	
#if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
	if (wlan_idx >= 2)
		return;
#else
	wlan_idx = 0;
#endif
	
	priv = wlan_device[wlan_idx].priv;
	if (NULL == priv)
		return;
	
	if ((gpio_num >= 0) && (gpio_num <= 7)) {
		regval = RTL_R32(GPIO_PIN_CTRL);
		RTL_W32(GPIO_PIN_CTRL, ( regval | BIT(gpio_num + 24) ) & ( ~BIT(gpio_num + 16) ) );
		return;
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A)) {
	#if 0
		if ((gpio_num >= 8) && (gpio_num <= 15)) {
			regval = RTL_R32(0x060);
			mask = BIT(gpio_num + 8);
			if (regval & mask) {
				if ((regval & BIT(gpio_num + 16)) || (direction == 0x01))	// input
					RTL_W32(0x060, regval & ~(BIT(gpio_num + 16) | mask));
			} else {
				if ((regval & BIT(gpio_num + 16)) || (direction == 0x10))	// output
					RTL_W32(0x060, (regval & ~ BIT(gpio_num + 16)) | mask);
			}
			return;
		} else if (gpio_num == WIFI_GPIO_92E_ANTSEL_P) {
			if (GET_CHIP_VER(priv) == VERSION_8192E) {
				regval = RTL_R32(LEDCFG);
				if (!(regval & BIT23))
					RTL_W32(LEDCFG, regval | BIT23);	// SW control ANT_SEL[P:N]
				return;
			}
		}
	#endif 
	}
	else {
		if ((gpio_num >= 8) && (gpio_num <= 11)) {
			regval = RTL_R32(GPIO_MUXCFG);
			RTL_W32(GPIO_MUXCFG,( regval | BIT(gpio_num + 20) ) & ( ~BIT(gpio_num + 16) ) );
		}
			return;
	}
}
EXPORT_SYMBOL(rtl8192cd_gpio_config_interrupt);


void rtl8192cd_gpio_interrupt_trrigger_mode(unsigned int wlan_idx, unsigned int gpio_num, unsigned int mode)
{
	struct rtl8192cd_priv *priv;
		unsigned int regval;
#if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
			if (wlan_idx >= 2)
				return;
#else
			wlan_idx = 0;
#endif
		priv = wlan_device[wlan_idx].priv;
		if (NULL == priv)
			return;

	if (mode)//negative edge
	{	
		regval = RTL_R32(GPIO_INTM);
		RTL_W32(GPIO_INTM, RTL_R32(GPIO_INTM)|(0x01<<gpio_num) );
	}
	else//positive edge
		RTL_W32( GPIO_INTM, RTL_R32(GPIO_INTM)&(~(0x01<<gpio_num)) );
}
EXPORT_SYMBOL(rtl8192cd_gpio_interrupt_trrigger_mode);

int rtl8192cd_gpio_get_interrupt_num(unsigned int wlan_idx)
{
	struct rtl8192cd_priv *priv;
#if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
	if (wlan_idx >= 2)
		return;
#else
	wlan_idx = 0;
#endif
	priv = wlan_device[wlan_idx].priv;
	if (priv && priv->dev)
		return priv->dev->irq;
	return -1;
}
EXPORT_SYMBOL(rtl8192cd_gpio_get_interrupt_num);

/* Martin ZHU add 2015-5-21 */
void rtl8192cd_gpio_fs_interrupt_enable(unsigned int wlan_idx, unsigned int gpio_num)
{
	struct rtl8192cd_priv *priv;
	unsigned int reg;
#if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
		if (wlan_idx >= 2)
			return;
#else
		wlan_idx = 0;
#endif
	priv = wlan_device[wlan_idx].priv;
	if (NULL == priv)
		return;

	reg = RTL_R32(FSIMR);
	reg = reg | 1 << (gpio_num+12);
	RTL_W32(FSIMR, reg);//Enable Firmware system GPIOx interrupt source
	reg = RTL_R32(HSIMR);
	RTL_W32(HSIMR, reg| 0x01);//Enable Host system GPIO12_0 interrupt source	
}
EXPORT_SYMBOL(rtl8192cd_gpio_fs_interrupt_enable);

int rtl8192cd_gpio_fs_interrupt_status_read(unsigned int wlan_idx, unsigned int gpio_num)
{
	struct rtl8192cd_priv *priv;
	unsigned int status0,status1;
	
#if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
		if (wlan_idx >= 2)
			return;
#else
		wlan_idx = 0;
#endif
		priv = wlan_device[wlan_idx].priv;
		if (NULL == priv)
			return 0;

	status0 = RTL_R32(FSISR);//read Firmware system interrupt status register
	//status0 &= RTL_R32(FSIMR);//read Firmware system interrupt mask register,then and with status0
	RTL_W32(FSISR, status0);//clear related bit

	status1 = RTL_R32(HSISR);//read Host system interrupt status register
	status1 &= RTL_R32(HSIMR);//read Host system interrupt status register, then and with status1
	RTL_W32(HSISR, status1);//clear related bit 
	
	if ( ( (status0 >> (12 + gpio_num)) & 0x01 ) && (status1 & 0x01) )
		return 1;

	return 0;
}
EXPORT_SYMBOL(rtl8192cd_gpio_fs_interrupt_status_read);
#endif //end fo CONFIG_WIFI_SIMPLE_CONFIG && CONFIG_CT_PUSHKEYEVENT

void rtl8192cd_gpio_write(unsigned int wlan_idx, unsigned int gpio_num, int value)
{
	struct rtl8192cd_priv *priv;
	unsigned int regval, mask;
	
//	if (wlan_idx >= sizeof(wlan_device)/sizeof(struct _device_info_))
//		return;
#if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
	if (wlan_idx >= 2)
		return;
#else
	wlan_idx = 0;
#endif
	
	priv = wlan_device[wlan_idx].priv;
	if (NULL == priv)
		return;
	
	if ((gpio_num >= 0) && (gpio_num <= 7)) {
		regval = RTL_R32(GPIO_PIN_CTRL);
		mask = BIT(gpio_num + 8);
		if (regval & mask) {
			if (!value)
				RTL_W32(GPIO_PIN_CTRL, regval & ~mask);
		} else {
			if (value)
				RTL_W32(GPIO_PIN_CTRL, regval | mask);
		}
		return;
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A)) {
		if ((gpio_num >= 8) && (gpio_num <= 15)) {
			regval = RTL_R32(0x060);
			mask = BIT(gpio_num);
			if (regval & mask) {
				if (!value)
					RTL_W32(0x060, regval & ~mask);
			} else {
				if (value)
					RTL_W32(0x060, regval | mask);
			}
			return;
		} else if (gpio_num == WIFI_GPIO_92E_ANTSEL_P) {
			if (GET_CHIP_VER(priv) == VERSION_8192E) {
				regval = RTL_R32(0x064);
				if (regval & BIT0) {
					if (!value)
						RTL_W32(0x064, regval & ~BIT0);
				} else {
					if (value)
						RTL_W32(0x064, regval | BIT0);
				}
				return;
			}
		}
	}
	else {
		if ((gpio_num >= 8) && (gpio_num <= 11)) {
			regval = RTL_R32(GPIO_MUXCFG);
			mask = BIT(gpio_num + 12);
			if (regval & mask) {
				if (!value)
					RTL_W32(GPIO_MUXCFG, regval & ~mask);
			} else {
				if (value)
					RTL_W32(GPIO_MUXCFG, regval | mask);
			}
			return;
		}
	}
}
EXPORT_SYMBOL(rtl8192cd_gpio_write);

int rtl8192cd_gpio_read(unsigned int wlan_idx, unsigned int gpio_num)
{
	struct rtl8192cd_priv *priv;
	unsigned int val32;
	
//	if (wlan_idx >= sizeof(wlan_device)/sizeof(struct _device_info_))
//		return -1;
#if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
	if (wlan_idx >= 2)
		return -1;
#else
	wlan_idx = 0;
#endif
	
	priv = wlan_device[wlan_idx].priv;
	if (NULL == priv)
		return -1;
	
	if ((gpio_num >= 0) && (gpio_num <= 7)) {
		val32 = RTL_R32(GPIO_PIN_CTRL);
		if (val32 & BIT(gpio_num + 16))
			return (val32 & BIT(gpio_num + 8)) ? 1 : 0;
		else
			return (val32 & BIT(gpio_num)) ? 1 : 0;
	}

	if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8881A)) {
		if ((gpio_num >= 8) && (gpio_num <= 15)) {
			val32 = RTL_R32(0x060);
			if (val32 & BIT(gpio_num + 8))	// output
				return (val32 & BIT(gpio_num)) ? 1 : 0;
			else	// input
				return (val32 & BIT(gpio_num - 8)) ? 1 : 0;
		} else if (gpio_num == WIFI_GPIO_92E_ANTSEL_P) {
			if (GET_CHIP_VER(priv) == VERSION_8192E)
				return (RTL_R32(0x064) & BIT0) ? 1 : 0;
		}
	}
	else {
		if ((gpio_num >= 8) && (gpio_num <= 11)) {
			val32 = RTL_R32(GPIO_MUXCFG);
			if (val32 & BIT(gpio_num + 16))
				return (val32 & BIT(gpio_num + 12)) ? 1 : 0;
			else
				return (val32 & BIT(gpio_num + 8)) ? 1 : 0;
		}
	}
	
	return -1;
}
EXPORT_SYMBOL(rtl8192cd_gpio_read);
#endif // CONFIG_RTL8672

#if defined(CONFIG_WLAN_HAL) && defined(CONFIG_PCI_HCI)
// TODO: move into HAL
BOOLEAN
compareAvailableTXBD(
	struct rtl8192cd_priv   *priv,
	unsigned int            num,
	unsigned int            qNum,
	int                     compareFlag
)
{
	PHCI_TX_DMA_MANAGER_88XX    ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
	unsigned int                halQnum = GET_HAL_INTERFACE(priv)->MappingTxQueueHandler(priv, qNum);
#ifndef SMP_SYNC
	unsigned long               avail_txbd_flag;
#endif

	SAVE_INT_AND_CLI(avail_txbd_flag);

	if (compareFlag == 1) {
		if (ptx_dma->tx_queue[halQnum].avail_txbd_num > num)  {
			RESTORE_INT(avail_txbd_flag);
			return TRUE;
		} else {
			RESTORE_INT(avail_txbd_flag);
			return FALSE;
		}
	} else if (compareFlag == 2) {
		if (ptx_dma->tx_queue[halQnum].avail_txbd_num < num)  {
			RESTORE_INT(avail_txbd_flag);
			return TRUE;
		} else {
			RESTORE_INT(avail_txbd_flag);
			return FALSE;
		}
	} else {
		printk("%s(%d): Error setting !!! \n", __FUNCTION__, __LINE__);
	}

	RESTORE_INT(avail_txbd_flag);

	return FALSE;
}

#endif // CONFIG_WLAN_HAL && CONFIG_PCI_HCI

/* Hotsport 2.0 Release 1 */
#if defined(HS2_SUPPORT) || defined(RTK_NL80211) /*survey_dump*/ || defined(DOT11K)|| defined(CH_LOAD_CAL) || defined(RTK_SMART_ROAMING)
void start_bbp_ch_load(struct rtl8192cd_priv *priv, unsigned int units)
{
    unsigned short chip_ver = GET_CHIP_VER(priv);
    if(chip_ver == VERSION_8188C || chip_ver == VERSION_8192C || chip_ver ==VERSION_8192D || 
        chip_ver == VERSION_8188E || chip_ver == VERSION_8192E || chip_ver == VERSION_8197F) /*all N-series ic*/
    {
        RTL_W16(0x894, units); //set ch load period time units*4 usec
        RTL_W32(0x890, 0x0);	//reset
        RTL_W32(0x890, 0x100); 
        RTL_W32(0x890, 0x101); 
    }
    else /* 8812, 8881A, 8814A, all AC-series ic*/
    {
        RTL_W16(0x990, units); //set ch load period time units*4 usec
        RTL_W32(0x994, 0x0);	//reset
        RTL_W32(0x994, 0x100); 
        RTL_W32(0x994, 0x101);
    }
}


/*/return busy count ,its units = 4us*/
unsigned int read_bbp_ch_load(struct rtl8192cd_priv *priv)
{
    unsigned short chip_ver = GET_CHIP_VER(priv);
	unsigned char retry = 0;
    if(chip_ver == VERSION_8188C || chip_ver == VERSION_8192C || chip_ver ==VERSION_8192D || 
        chip_ver == VERSION_8188E || chip_ver == VERSION_8192E || chip_ver == VERSION_8197F) /*all N-series ic*/
    {
	if (RTL_R32(0x8b4) & BIT16)
	            	return (RTL_R32(0x8d0) & 0xffff) ;

    }else{ /* 8812, 8881A, 8813, all AC-series ic*/

	if (RTL_R32(0xfa4) & BIT16)
	            	return (RTL_R32(0xfa4) & 0xffff);
    }
	
	return 0;
}


/*CLM [channle loading measurement]*/
void channle_loading_measurement(struct rtl8192cd_priv *priv){

	unsigned int clm_count=0;

#ifdef USE_OUT_SRC
	if (phydm_checkCLMready(ODMPTR) ){ 		/* --Getting CLM info previous time--*/	
		clm_count = phydm_getCLMresult(ODMPTR);	
	}
	 if(clm_count >0 ){
		/*IEEE,Std 802.11-2012,page567,use 0~255 to representing 0~100%*/
		//priv->ext_stats.ch_utilization = (clm_count * 255) /CLM_SAMPLE_NUM2;
		priv->ext_stats.ch_utilization = (clm_count * 255) /CLM_SAMPLE_NUM2;
		//GDEBUG("busy time[%d]ms\n",RTK80211_SAMPLE_NUM_TO_TIME(clm_count));
	}	   
	phydm_CLMtrigger(ODMPTR);/* trigger next time CLM */
#else
	clm_count=read_bbp_ch_load(priv);	/* --Getting CLM info previous time--*/
	 if(clm_count >0 ){
		/*IEEE,Std 802.11-2012,page567,use 0~255 to representing 0~100%*/
		priv->ext_stats.ch_utilization = (clm_count * 255) /CLM_SAMPLE_NUM2;
	}	   	
	start_bbp_ch_load(priv,CLM_SAMPLE_NUM2);/* trigger next time CLM */	
#endif	

}
#endif	


#ifdef CONFIG_1RCCA_RF_POWER_SAVING
void one_path_cca_power_save(struct rtl8192cd_priv *priv, int enable)
{
	if (enable && !priv->pshare->rf_ft_var.one_path_cca_ps_active) {
		priv->pshare->rf_ft_var.one_path_cca_ps_active = 1;
		RTL_W8(0x800, RTL_R8(0x800) | BIT1);
		RTL_W8(0xc04, 0x13);
	} else if (!enable && priv->pshare->rf_ft_var.one_path_cca_ps_active) {
		priv->pshare->rf_ft_var.one_path_cca_ps_active = 0;
		RTL_W8(0x800, RTL_R8(0x800) & ~BIT1);
		RTL_W8(0xc04, 0x33);
	}
}
#endif // CONFIG_1RCCA_RF_POWER_SAVING

