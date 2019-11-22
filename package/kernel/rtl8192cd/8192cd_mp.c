/*
 *  MP routines
 *
 *  $Id: 8192cd_mp.c,v 1.25.2.8 2010/11/24 12:17:18 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_MP_C_

#include <linux/module.h>
#include <linux/circ_buf.h>

#include "./8192cd_cfg.h"


#include "./8192cd_headers.h"
#include "./8192cd_tx.h"
#include "./8192cd_debug.h"

#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
#include <asm/mips16_lib.h>
#endif

//#include "./WlanHAL/RTL88XX/HalPrecomp.h"
#include "./WlanHAL/HalPrecompInc.h"



#ifdef MP_TEST

#ifdef _MP_TELNET_SUPPORT_
extern void mp_pty_write_monitor(int en);
extern int mp_pty_is_hit(void);
extern int mp_pty_write(const unsigned char *buf, int count);
int mp_printk(const char *fmt, ...)
{
	va_list args;
	int r;
	int mp_printed_len;
	static char mp_printk_buf[1024];

	va_start(args, fmt);
	r = vprintk(fmt, args);
	va_end(args);

	va_start(args, fmt);
	mp_printed_len = vscnprintf(mp_printk_buf, sizeof(mp_printk_buf), fmt, args);
	va_end(args);
	mp_pty_write( mp_printk_buf, mp_printed_len );

	return r;
}
#define printk mp_printk
#else
#if defined(CONFIG_PANIC_PRINTK)
#define printk panic_printk
#endif
#endif //_MP_TELNET_SUPPORT_

#ifdef _LITTLE_ENDIAN_
typedef struct _R_ANTENNA_SELECT_OFDM {
	unsigned int		r_tx_antenna:4;
	unsigned int		r_ant_l:4;
	unsigned int		r_ant_non_ht:4;
	unsigned int		r_ant_ht1:4;
	unsigned int		r_ant_ht2:4;
	unsigned int		r_ant_ht_s1:4;
	unsigned int		r_ant_non_ht_s1:4;
	unsigned int		OFDM_TXSC:2;
	unsigned int		Reserved:2;
} R_ANTENNA_SELECT_OFDM;

typedef struct _R_ANTENNA_SELECT_CCK {
	unsigned char		r_cckrx_enable_2:2;
	unsigned char		r_cckrx_enable:2;
	unsigned char		r_ccktx_enable:4;
} R_ANTENNA_SELECT_CCK;

#else // _BIG_ENDIAN_

typedef struct _R_ANTENNA_SELECT_OFDM {
	unsigned int		Reserved:2;
	unsigned int		OFDM_TXSC:2;
	unsigned int		r_ant_non_ht_s1:4;
	unsigned int		r_ant_ht_s1:4;
	unsigned int		r_ant_ht2:4;
	unsigned int		r_ant_ht1:4;
	unsigned int		r_ant_non_ht:4;
	unsigned int		r_ant_l:4;
	unsigned int		r_tx_antenna:4;
} R_ANTENNA_SELECT_OFDM;

typedef struct _R_ANTENNA_SELECT_CCK {
	unsigned char		r_ccktx_enable:4;
	unsigned char		r_cckrx_enable:2;
	unsigned char		r_cckrx_enable_2:2;
} R_ANTENNA_SELECT_CCK;
#endif


extern int PHYCheckIsLegalRfPath8192cPci(struct rtl8192cd_priv *priv, unsigned int eRFPath);
static void mp_chk_sw_ant(struct rtl8192cd_priv *priv);


/*
 * _RT_PMAC_TX_INFO: Tx info struct for pseudo MAC Tx
 */
#if (IC_LEVEL >= LEVEL_8814) || (IC_LEVEL == LEVEL_92E)
typedef struct _RT_PMAC_TX_INFO {
#ifdef _LITTLE_ENDIAN_
	UCHAR			bEnPMacTx:1;		/* 0: Disable PMac 1: Enable PMac*/
	UCHAR			Mode:3;				/* 0: Packet TX 3:Continuous TX */
	UCHAR			Ntx:4;				/* 0-7     Order of Ntx, Mode, and bEnPMacTx is reversed for MP API's due to difference alignment of C struct across platforms*/
#else
	UCHAR			Ntx:4;				/* 0-7     Order of Ntx, Mode, and bEnPMacTx is reversed for MP API's due to difference alignment of C struct across platforms*/
	UCHAR			Mode:3;				/* 0: Packet TX 3:Continuous TX */
	UCHAR			bEnPMacTx:1;		/* 0: Disable PMac 1: Enable PMac*/
#endif
	UCHAR			TX_RATE;			/* MPT_RATE_E*/
	UCHAR			TX_RATE_HEX;
	UCHAR			TX_SC;
#ifdef _LITTLE_ENDIAN_
	UCHAR			bSGI:1;
	UCHAR			bSPreamble:1;
	UCHAR			bSTBC:1;
	UCHAR			bLDPC:1;
	UCHAR			NDP_sound:1;
	UCHAR			BandWidth:3;    	/* Order of BandWidth, NDP_sound, bLDPC, bSPreamble, and bSGI is reversed for MP API's */
#else
	UCHAR			BandWidth:3;    	/* Order of BandWidth, NDP_sound, bLDPC, bSPreamble, and bSGI is reversed for MP API's */
	UCHAR			NDP_sound:1;
	UCHAR			bLDPC:1;
	UCHAR			bSTBC:1;
	UCHAR			bSPreamble:1;
	UCHAR			bSGI:1;
#endif
	UCHAR			m_STBC;				/* bSTBC + 1*/
	USHORT			PacketPeriod;
	UINT			PacketCount;
	UINT			PacketLength;
	UCHAR			PacketPattern;
	USHORT			SFD;
	UCHAR			SignalField;
	UCHAR			ServiceField;
	USHORT			LENGTH;
	UCHAR			CRC16[2];  			/* BYTE Order of USHORT and UINT  are reversed already in MP API */
	UCHAR			LSIG[3];
	UCHAR			HT_SIG[6];
	UCHAR			VHT_SIG_A[6];
	UCHAR			VHT_SIG_B[4];
	UCHAR			VHT_SIG_B_CRC;
	UCHAR			VHT_Delimiter[4];
	UCHAR			MacAddress[6];
} RT_PMAC_TX_INFO, *PRT_PMAC_TX_INFO;
#endif

extern unsigned int TxPwrTrk_OFDM_SwingTbl[TxPwrTrk_OFDM_SwingTbl_Len];
extern unsigned char TxPwrTrk_CCK_SwingTbl[TxPwrTrk_CCK_SwingTbl_Len][8];
extern unsigned char TxPwrTrk_CCK_SwingTbl_CH14[TxPwrTrk_CCK_SwingTbl_Len][8];



#include <bspchip.h>

#define _GIMR_				BSP_GIMR
#define _ICU_UART0_MSK_		BSP_UART0_IE
#define _UART0_RBR_			BSP_UART0_RBR
#define _UART0_LSR_			BSP_UART0_LSR

#ifdef B2B_TEST
#define MP_PACKET_HEADER		("wlan-tx-test")
#define MP_PACKET_HEADER_LEN	12
#endif



#define 	VERSION_8188C  0x1000
#define	VERSION_8192C  0x1001
#define VERSION_8192D 0x1002

#define CHECKICIS92C() ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C))
#define CHECKICIS92D()  (GET_CHIP_VER(priv)==VERSION_8192D)
#define CHECKICIS8812()  (GET_CHIP_VER(priv)==VERSION_8812E)
#define CHECKICIS8881A()  (GET_CHIP_VER(priv)==VERSION_8881A)
#define CHECKICIS8814()  (GET_CHIP_VER(priv)==VERSION_8814A)
#define CHECKICIS8822()  (GET_CHIP_VER(priv)==VERSION_8822B)
#define CHECKICIS97F()  (GET_CHIP_VER(priv)==VERSION_8197F)

#define RTL_UART_R8(reg)		readb((unsigned char *)reg)
#define RTL_UART_R32(reg)		readl((unsigned char *)reg)
#define RTL_UART_W8(reg, val)	writeb(val, (unsigned char *)reg)
#define RTL_UART_W32(reg, val)	writel(val, (unsigned char *)reg)
#define DISABLE_UART0_INT() \
	do { \
		RTL_UART_W32(_GIMR_, RTL_UART_R32(_GIMR_) & ~_ICU_UART0_MSK_); \
		RTL_UART_R8(_UART0_RBR_); \
		RTL_UART_R8(_UART0_RBR_); \
	} while (0)

#define RESTORE_UART0_INT() \
	do { \
		RTL_UART_W32(_GIMR_, RTL_UART_R32(_GIMR_) | _ICU_UART0_MSK_); \
	} while (0)

static inline int IS_KEYBRD_HIT(void)
{
	if (RTL_UART_R8(_UART0_LSR_) & 1) { // data ready
		RTL_UART_R8(_UART0_RBR_);	 // clear rx FIFO
		return 1;
	}
	return 0;
}


unsigned char * get_bssid_mp(unsigned char *pframe)
{
	unsigned char 	*bssid;
	unsigned int	to_fr_ds	= (GetToDs(pframe) << 1) | GetFrDs(pframe);

	switch (to_fr_ds) {
		case 0x00:	// ToDs=0, FromDs=0
			bssid = GetAddr3Ptr(pframe);
			break;
		case 0x01:	// ToDs=0, FromDs=1
			bssid = GetAddr2Ptr(pframe);
			break;
		case 0x02:	// ToDs=1, FromDs=0
			bssid = GetAddr1Ptr(pframe);
			break;
		default:	// ToDs=1, FromDs=1
			bssid = GetAddr2Ptr(pframe);
			break;
	}

	return bssid;
}


static __inline__ int isLegalRate(unsigned int rate)
{
	unsigned int res = 0;

	switch(rate)
	{
	case _1M_RATE_:
	case _2M_RATE_:
	case _5M_RATE_:
	case _6M_RATE_:
	case _9M_RATE_:
	case _11M_RATE_:
	case _12M_RATE_:
	case _18M_RATE_:
	case _24M_RATE_:
	case _36M_RATE_:
	case _48M_RATE_:
	case _54M_RATE_:
		res = 1;
		break;
	case _MCS0_RATE_:
	case _MCS1_RATE_:
	case _MCS2_RATE_:
	case _MCS3_RATE_:
	case _MCS4_RATE_:
	case _MCS5_RATE_:
	case _MCS6_RATE_:
	case _MCS7_RATE_:
	case _MCS8_RATE_:
	case _MCS9_RATE_:
	case _MCS10_RATE_:
	case _MCS11_RATE_:
	case _MCS12_RATE_:
	case _MCS13_RATE_:
	case _MCS14_RATE_:
	case _MCS15_RATE_:
	case _MCS16_RATE_:
	case _MCS17_RATE_:
	case _MCS18_RATE_:
	case _MCS19_RATE_:
	case _MCS20_RATE_:
	case _MCS21_RATE_:
	case _MCS22_RATE_:
	case _MCS23_RATE_:
		res = 1;
		break;
#ifdef RTK_AC_SUPPORT  		//vht rate
	case	_NSS1_MCS0_RATE_:
	case	_NSS1_MCS1_RATE_:
	case	_NSS1_MCS2_RATE_:
	case	_NSS1_MCS3_RATE_:
	case	_NSS1_MCS4_RATE_:
	case	_NSS1_MCS5_RATE_:
	case	_NSS1_MCS6_RATE_:
	case	_NSS1_MCS7_RATE_:
	case	_NSS1_MCS8_RATE_:
	case	_NSS1_MCS9_RATE_:
	case	_NSS2_MCS0_RATE_:
	case	_NSS2_MCS1_RATE_:
	case	_NSS2_MCS2_RATE_:
	case	_NSS2_MCS3_RATE_:
	case	_NSS2_MCS4_RATE_:
	case	_NSS2_MCS5_RATE_:
	case	_NSS2_MCS6_RATE_:
	case	_NSS2_MCS7_RATE_:
	case	_NSS2_MCS8_RATE_:
	case	_NSS2_MCS9_RATE_:
	case	_NSS3_MCS0_RATE_:
	case	_NSS3_MCS1_RATE_:
	case	_NSS3_MCS2_RATE_:
	case	_NSS3_MCS3_RATE_:
	case	_NSS3_MCS4_RATE_:
	case	_NSS3_MCS5_RATE_:
	case	_NSS3_MCS6_RATE_:
	case	_NSS3_MCS7_RATE_:
	case	_NSS3_MCS8_RATE_:
	case	_NSS3_MCS9_RATE_:
	case	_NSS4_MCS0_RATE_:
	case	_NSS4_MCS1_RATE_:
	case	_NSS4_MCS2_RATE_:
	case	_NSS4_MCS3_RATE_:
	case	_NSS4_MCS4_RATE_:
	case	_NSS4_MCS5_RATE_:
	case	_NSS4_MCS6_RATE_:
	case	_NSS4_MCS7_RATE_:
	case	_NSS4_MCS8_RATE_:
	case	_NSS4_MCS9_RATE_:
		res = 1;
		break;
#endif
	default:
		res = 0;
		break;
	}

	return res;
}

#ifdef MP_PSD_SUPPORT
int GetPSDData(struct rtl8192cd_priv *priv,unsigned int point)
{
	int psd_val;
	if((GET_CHIP_VER(priv)== VERSION_8812E) || (GET_CHIP_VER(priv)== VERSION_8881A) || (GET_CHIP_VER(priv)== VERSION_8814A) || (GET_CHIP_VER(priv)== VERSION_8822B)) {
		psd_val = RTL_R32(0x910);
		psd_val &= 0xFFBFFC00;
		psd_val |= point;

		RTL_W32(0x910, psd_val);
		delay_ms(2);
		psd_val |= 0x00400000;

		RTL_W32(0x910, psd_val);
		delay_ms(2);
		psd_val = RTL_R32(0xF44);

		psd_val &= 0x0000FFFF;
	} else
	{
	psd_val = RTL_R32(0x808);
	psd_val &= 0xFFBFFC00;
	psd_val |= point;

	RTL_W32(0x808, psd_val);
	delay_ms(2);
	psd_val |= 0x00400000;

	RTL_W32(0x808, psd_val);
	delay_ms(2);
	psd_val = RTL_R32(0x8B4);

	psd_val &= 0x0000FFFF;
	}
	return psd_val;
}
#endif
#if 0
static void mp_RL5975e_Txsetting(struct rtl8192cd_priv *priv)
{
	RF92CD_RADIO_PATH_E eRFPath;
	unsigned int rfReg0x14, rfReg0x15, rfReg0x2c;

	// reg0x14
	rfReg0x14 = 0x5ab;
	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
	{
		//channel = 1, 11, in 20MHz mode, set RF-reg[0x14] = 0x59b
		if(priv->pshare->working_channel == 1 || priv->pshare->working_channel == 11)
		{
			if(!is_CCK_rate(priv->pshare->mp_datarate)) //OFDM, MCS rates
				rfReg0x14 = 0x59b;
		}
	}
	else
	{
		//channel = 3, 9, in 40MHz mode, set RF-reg[0x14] = 0x59b
		if(priv->pshare->working_channel == 3 || priv->pshare->working_channel == 9)
			rfReg0x14 = 0x59b;
	}
	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++)
	{
		if (!PHYCheckIsLegalRfPath8192cPci(priv, eRFPath))
			continue;
/*
		if (get_rf_mimo_mode(priv) == MIMO_1T2R) {
			if ((eRFPath == RF92CD_PATH_A) || (eRFPath == RF92CD_PATH_B))
				continue;
		}
		else if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
			if ((eRFPath == RF92CD_PATH_B) || (eRFPath == RF90_PATH_D))
				continue;
		}
*/
		PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, 0x14, bMask20Bits, rfReg0x14);
		delay_us(100);
	}

	// reg0x15
	rfReg0x15 = 0xf80;
	if(priv->pshare->mp_datarate == 4)
	{
		if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
			rfReg0x15 = 0xfc0;
	}
	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++)
	{
		if (!PHYCheckIsLegalRfPath8192cPci(priv, eRFPath))
			continue;
/*
		if (get_rf_mimo_mode(priv) == MIMO_1T2R) {
			if ((eRFPath == RF92CD_PATH_A) || (eRFPath == RF92CD_PATH_B))
				continue;
		}
		else if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
			if ((eRFPath == RF92CD_PATH_B) || (eRFPath == RF90_PATH_D))
				continue;
		}
*/
		PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, 0x15, bMask20Bits, rfReg0x15);
		delay_us(100);
	}

	//reg0x2c
	if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
	{
		rfReg0x2c = 0x3d7;
		if(is_CCK_rate(priv->pshare->mp_datarate)) //CCK rate
		{
			if(priv->pshare->working_channel == 1 || priv->pshare->working_channel == 11)
				rfReg0x2c = 0x3f7;
		}
	}
	else
	{
		rfReg0x2c = 0x3ff;
	}
	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++)
	{
		if (!PHYCheckIsLegalRfPath8192cPci(priv, eRFPath))
			continue;
/*
		if (get_rf_mimo_mode(priv) == MIMO_1T2R) {
			if ((eRFPath == RF92CD_PATH_A) || (eRFPath == RF92CD_PATH_B))
				continue;
		}
		else if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
			if ((eRFPath == RF92CD_PATH_B) || (eRFPath == RF90_PATH_D))
				continue;
		}
*/
		PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, 0x2c, bMask20Bits, rfReg0x2c);
		delay_us(100);
	}

	if (priv->pshare->rf_ft_var.use_frq_2_3G)
		PHY_SetRFReg(priv, RF90_PATH_C, 0x2c, 0x60, 0);
}


static void mp_RF_RxLPFsetting(struct rtl8192cd_priv *priv)
{
	unsigned int rfBand_A=0, rfBand_B=0, rfBand_C=0, rfBand_D=0;

	//==================================================
	//because the EVM issue, CCK ACPR spec, asked by bryant.
	//when BAND_20MHZ_MODE, should overwrite CCK Rx path RF, let the bandwidth
	//from 10M->8M, we should overwrite the following values to the cck rx rf.
	//RF_Reg[0xb]:bit[11:8] = 0x4, otherwise RF_Reg[0xb]:bit[11:8] = 0x0
	switch(priv->pshare->mp_antenna_rx)
	{
	case ANTENNA_A:
	case ANTENNA_AC:
	case ANTENNA_ABCD:
		rfBand_A = 0x500; //for TxEVM, CCK ACPR
		break;
	case ANTENNA_B:
	case ANTENNA_BD:
		rfBand_B = 0x500; //for TxEVM, CCK ACPR
		break;
	case ANTENNA_C:
	case ANTENNA_CD:
		rfBand_C = 0x500; //for TxEVM, CCK ACPR
		break;
	case ANTENNA_D:
		rfBand_D = 0x500; //for TxEVM, CCK ACPR
		break;
	}

	if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
	{
		if(!rfBand_A)
			rfBand_A = 0x100;
		if(!rfBand_B)
			rfBand_B = 0x100;
		if(!rfBand_C)
			rfBand_C = 0x100;
		if(!rfBand_D)
			rfBand_D = 0x100;
	}
	else
	{
		rfBand_A = 0x300;
		rfBand_B = 0x300;
		rfBand_C = 0x300;
		rfBand_D = 0x300;
	}

	PHY_SetRFReg(priv, RF92CD_PATH_A, 0x0b, bMask20Bits, rfBand_A);
	delay_us(100);
	PHY_SetRFReg(priv, RF92CD_PATH_B, 0x0b, bMask20Bits, rfBand_B);
	delay_us(100);
/*
	PHY_SetRFReg(priv, RF90_PATH_C, 0x0b, bMask20Bits, rfBand_C);
	delay_us(100);
	PHY_SetRFReg(priv, RF90_PATH_D, 0x0b, bMask20Bits, rfBand_D);
	delay_us(100);
*/
}
#endif


static void mp_8192CD_tx_setting(struct rtl8192cd_priv *priv)
{
	unsigned int odd_pwr = 0;
//	extern int get_CCK_swing_index(struct rtl8192cd_priv*);
//#ifndef CONFIG_RTL_92D_SUPPORT
#if 1//!defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)
	extern void set_CCK_swing_index(struct rtl8192cd_priv*, short );
#endif

}


static void mpt_StartCckContTx(struct rtl8192cd_priv *priv)
{
	unsigned int cckrate;

	if(CHECKICIS8812() || CHECKICIS8881A() || CHECKICIS8814() || CHECKICIS8822()) {
		// 1. if CCK block on?
		if(!PHY_QueryBBReg(priv, 0x808, BIT(28)))
			PHY_SetBBReg(priv, 0x808, BIT(28), bEnable);//set CCK block on

		//Turn Off All Test Mode
		PHY_SetBBReg(priv, 0x914, BIT(16), bDisable);
		PHY_SetBBReg(priv, 0x914, BIT(17), bDisable);
		PHY_SetBBReg(priv, 0x914, BIT(18), bDisable);
	}
	else
	{
		// 1. if CCK block on?
		if (!PHY_QueryBBReg(priv, rFPGA0_RFMOD, bCCKEn))
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, bEnable);//set CCK block on

		//Turn Off All Test Mode
		PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bDisable);
		PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bDisable);
		PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bDisable);
	}
	//Set CCK Tx Test Rate
	switch (priv->pshare->mp_datarate)
	{
		case 2:
			cckrate = 0;
			break;
		case 4:
			cckrate = 1;
			break;
		case 11:
			cckrate = 2;
			break;
		case 22:
			cckrate = 3;
			break;
		default:
			cckrate = 0;
			break;
	}
	PHY_SetBBReg(priv, rCCK0_System, bCCKTxRate, cckrate);

	PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, 0x2);    //transmit mode
	PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, 0x1);  //turn on scramble setting

	PHY_SetBBReg(priv, 0xa70, 0x4000, 0x1);  //0xa71[6]=1'b1. Fix rate selection issue
#if 1//def CONFIG_RTL8672
	// Commented out for background mode, sync with SD7, 2010-07-08 by Annie ---
	// We will set 0x820 and 0x828 under Tx mode in mp_ctx(), 2010-09-17 by Family.
	//	PHY_SetBBReg(priv, 0x820, 0x400, 0x1);
	//	PHY_SetBBReg(priv, 0x828, 0x400, 0x1);
	//---
#else //CONFIG_RTL8672
	PHY_SetBBReg(priv, 0x820, 0x400, 0x1);
	PHY_SetBBReg(priv, 0x828, 0x400, 0x1);
#endif //CONFIG_RTL8672

}


static void mpt_StopCckCoNtTx(struct rtl8192cd_priv *priv)
{
	PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, 0x0);    //normal mode
	PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, 0x1);  //turn on scramble setting


	{
		PHY_SetBBReg(priv, 0x820, 0x400, 0x0);
		PHY_SetBBReg(priv, 0x828, 0x400, 0x0);
	}

	PHY_SetBBReg(priv, 0xa70, 0x4000, 0x0);  // restore 0xa71[6]
}


static void mpt_StartOfdmContTx(struct rtl8192cd_priv *priv)
{
	unsigned int go=0;

	// 1. if OFDM block on?
	if(CHECKICIS8812() || CHECKICIS8881A() || CHECKICIS8814() || CHECKICIS8822()) {
		if(!PHY_QueryBBReg(priv, 0x808, BIT(29)))
			PHY_SetBBReg(priv, 0x808, BIT(29), bEnable);//set CCK block on
	} else
	{
		if (!PHY_QueryBBReg(priv, rFPGA0_RFMOD, bOFDMEn))
			PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, bEnable);//set OFDM block on
	}

	if (CHECKICIS92D() || CHECKICIS8812() || CHECKICIS8881A() || CHECKICIS8814() || CHECKICIS8822())
	{
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)
			go=1;
		else
			go=0;
	}
	else
		go=1;

	if(go==1 || CHECKICIS8814())
	{
		// 2. set CCK test mode off, set to CCK normal mode
		PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, bDisable);

		// 3. turn on scramble setting
		PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, bEnable);
	}

	// 4. Turn On Continue Tx and turn off the other test modes.
	if(CHECKICIS8812() || CHECKICIS8881A() || CHECKICIS8814() || CHECKICIS8822()) {
		PHY_SetBBReg(priv, 0x914, BIT(16), bEnable);
		PHY_SetBBReg(priv, 0x914, BIT(17), bDisable);
		PHY_SetBBReg(priv, 0x914, BIT(18), bDisable);
	}
	else
	{
		PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bEnable);
		PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bDisable);
		PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bDisable);
	}
#if 1//def CONFIG_RTL8672
	// Commented out for background mode, sync with SD7, 2010-07-08 by Annie ---
	// We will set 0x820 and 0x828 under Tx mode in mp_ctx(), 2010-09-17 by Family.
	//	PHY_SetBBReg(priv, 0x820, 0x400, 0x1);
	//	PHY_SetBBReg(priv, 0x828, 0x400, 0x1);
	//---
#else //CONFIG_RTL8672
	PHY_SetBBReg(priv, 0x820, 0x400, 0x1);
	PHY_SetBBReg(priv, 0x828, 0x400, 0x1);
#endif //CONFIG_RTL8672

}


static void mpt_StopOfdmContTx(struct rtl8192cd_priv *priv)
{
	if(CHECKICIS8812() || CHECKICIS8881A() || CHECKICIS8814() || CHECKICIS8822()) {
		PHY_SetBBReg(priv, 0x914, BIT(16), bDisable);
		PHY_SetBBReg(priv, 0x914, BIT(17), bDisable);
		PHY_SetBBReg(priv, 0x914, BIT(18), bDisable);
	}
	else
	{
		PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bDisable);
		PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bDisable);
		PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bDisable);
	}

	//Delay 10 ms
	delay_ms(10);

	{
		PHY_SetBBReg(priv, 0x820, 0x400, 0x0);
		PHY_SetBBReg(priv, 0x828, 0x400, 0x0);
	}
}


static void mpt_ProSetCarrierSupp(struct rtl8192cd_priv *priv, int enable)
{
	if (enable)
	{ // Start Carrier Suppression.
        if(CHECKICIS8812() || CHECKICIS8881A() || CHECKICIS8814() || CHECKICIS8822()) {
			// 1. if CCK block on?
			if(!PHY_QueryBBReg(priv, 0x808, BIT(28)))
				PHY_SetBBReg(priv, 0x808, BIT(28), bEnable);//set CCK block on

			//Turn Off All Test Mode
			PHY_SetBBReg(priv, 0x914, BIT(16), bDisable);
			PHY_SetBBReg(priv, 0x914, BIT(17), bDisable);
			PHY_SetBBReg(priv, 0x914, BIT(18), bDisable);
		}
		else
		{
			// 1. if CCK block on?
			if(!PHY_QueryBBReg(priv, rFPGA0_RFMOD, bCCKEn))
				PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, bEnable);//set CCK block on

			//Turn Off All Test Mode
			PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bDisable);
			PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bDisable);
			PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bDisable);
		}
		PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, 0x2);    //transmit mode
		PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, 0x0);  //turn off scramble setting
   		//Set CCK Tx Test Rate
		//PHY_SetBBReg(pAdapter, rCCK0_System, bCCKTxRate, pMgntInfo->ForcedDataRate);
		PHY_SetBBReg(priv, rCCK0_System, bCCKTxRate, 0x0);    //Set FTxRate to 1Mbps
	}
	else
	{ // Stop Carrier Suppression.
		PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, 0x0);    //normal mode
		PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, 0x1);  //turn on scramble setting

		//BB Reset
/*
		PHY_SetBBReg(priv, rPMAC_Reset, bBBResetB, 0x0);
		PHY_SetBBReg(priv, rPMAC_Reset, bBBResetB, 0x1);
*/
	}
}




/*
 * start mp testing. stop beacon and change to mp mode
 */
void mp_start_test(struct rtl8192cd_priv *priv)
{

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (OPMODE & WIFI_MP_STATE)
	{
		printk("\nFail: already in MP mode\n");
		return;
	}

#ifdef MP_SWITCH_LNA
	priv->pshare->rx_packet_ss_a = 0;
	priv->pshare->rx_packet_ss_b = 0;
#endif

	if (IS_VXD_INTERFACE(priv)) {
		printk("\nFail: only root interface supports MP mode\n");
		return;
	}
	else if (IS_ROOT_INTERFACE(priv) && IS_DRV_OPEN(GET_VXD_PRIV(priv)))
		rtl8192cd_close(GET_VXD_PRIV(priv)->dev);

	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable &&
		IS_VAP_INTERFACE(priv)) {
		printk("\nFail: only root interface supports MP mode\n");
		return;
	} else if (IS_ROOT_INTERFACE(priv)) {
		int i;
		if (priv->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i]))
					rtl8192cd_close(priv->pvap_priv[i]->dev);
			}
		}
	}

	// initialize rate to 54M (or 1M ?)
	priv->pshare->mp_datarate = _54M_RATE_;

	// initialize antenna
	priv->pshare->mp_antenna_tx = ANTENNA_A;
	priv->pshare->mp_antenna_rx = ANTENNA_A;
	if(CHECKICIS8822())
		config_phydm_trx_mode_8822b(ODMPTR,priv->pshare->mp_antenna_tx,priv->pshare->mp_antenna_rx,1);
	else
	if (CHECKICIS97F()) {//wish
		PHY_SetBBReg(priv, 0x998, BIT6, 0x0);
		PHY_SetBBReg(priv, 0xa00, BIT8, 0x0);
		//config_phydm_trx_mode_8197f(ODMPTR,priv->pshare->mp_antenna_tx,priv->pshare->mp_antenna_rx,0);
	}else
		mp_chk_sw_ant(priv);

	if (CHECKICIS8812() || CHECKICIS8881A()||CHECKICIS8814() ||CHECKICIS8822()) {
        //Do Nothing
	}
    else
	if ((get_rf_mimo_mode(priv) == MIMO_2T2R) && priv->pmib->dot11RFEntry.txbf) {
		// Tx Path Selection by ctrl_reg in MP mode
		PHY_SetBBReg(priv, 0x90C, BIT(30), 0);
	}


	// initialize swing index
#if 1
	{
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G) {

			if (GET_CHIP_VER(priv) == VERSION_8192E || GET_CHIP_VER(priv) == VERSION_8197F) {
#ifdef HIGH_POWER_EXT_PA
				if (priv->pshare->rf_ft_var.use_ext_pa)
					priv->pshare->mp_cck_swing_idx = 30;
				else
#endif
					priv->pshare->mp_cck_swing_idx = 20;
			} else
			{
				priv->pshare->mp_cck_swing_idx = 12;
			}
		}
		//printk("==> mp_ofdm_swing_idx=%d\n", priv->pshare->mp_ofdm_swing_idx);
		//printk("==> mp_cck_swing_idx=%d\n", priv->pshare->mp_cck_swing_idx);
	}
#endif
	// change mode to mp mode
	OPMODE_VAL(OPMODE | WIFI_MP_STATE);

	// disable beacon
	RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
	RTL_W8(TXPAUSE, STOP_BCN);

	priv->pmib->dot11StationConfigEntry.autoRate = 0;
	priv->pmib->dot11StationConfigEntry.protectionDisabled = 1;
	priv->pmib->dot11ErpInfo.ctsToSelf = 0;
	priv->pmib->dot11ErpInfo.protection = 0;
	priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
	priv->pmib->dot11DFSEntry.disable_tx = 0;
	OPMODE_VAL(OPMODE & ~WIFI_STATION_STATE);
	OPMODE_VAL(OPMODE | WIFI_AP_STATE);

	// stop site survey
	if (timer_pending(&priv->ss_timer))
		del_timer_sync(&priv->ss_timer);

	// stop receiving packets
	if (IS_HAL_CHIP(priv))
		RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_APWRMGT | RCR_AMF | RCR_ADF |RCR_ACRC32 |RCR_AB | RCR_AM | RCR_APM | RCR_AAP));
	else if(CONFIG_WLAN_NOT_HAL_EXIST)
	RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_AB | RCR_AM | RCR_APM | RCR_AAP));
	// Global setting for MP no ack CCK
	RTL_W8(0x700, 0xe0);

	// stop dynamic mechanism
//	if ((get_rf_mimo_mode(priv) == MIMO_2T4R) && (priv->pmib->dot11BssType.net_work_type != WIRELESS_11B))
//		rx_path_by_rssi(priv, NULL, FALSE);
//	tx_power_control(priv, NULL, FALSE);

	// DIG off and set initial gain
	priv->pshare->rf_ft_var.dig_enable = 0;
	set_DIG_state(priv, 0);
	delay_ms(1);

	{
			RTL_W8(0xc50, 0x20);
		if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
		if ((GET_CHIP_VER(priv)==VERSION_8812E)||(GET_CHIP_VER(priv)==VERSION_8822B))
			RTL_W8(0xe50, 0x20);
		else
			RTL_W8(0xc58, 0x20);
		}

	}

	RTL_W8(0xa0a, 0x83);

	priv->pshare->rf_ft_var.tpt_period=2;
	priv->pshare->mp_rssi_weight = 5;
	priv->pshare->mp_FA_cnt = 0;
	priv->pshare->mp_CCA_cnt = 0;

	mp_8192CD_tx_setting(priv);


#if 0//def CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E) {
		if (priv->pshare->PLL_reset_ok == false)
			Check_92E_Spur_Valid(priv, true);
	}
#endif

#if (defined(SW_ANT_SWITCH) || defined(HW_ANT_SWITCH))
	{
		ODMPTR->antdiv_period=0;
		if (ODMPTR->antdiv_select == ODMPTR->pre_AntType)
			ODMPTR->pre_AntType = (ODMPTR->antdiv_select ? 0:1);
		if (ODMPTR->antdiv_select == ODMPTR->DM_FatTable.RxIdleAnt)
			ODMPTR->DM_FatTable.RxIdleAnt = ((ODMPTR->antdiv_select==1)? 2:1);

		ODM_AntDiv(ODMPTR);
	}
#endif



#if 1//def GREEN_HILL
	printk("Enter testing mode\n");
#else
	printk("\nUsage:\n");
	printk("  iwpriv wlanx mp_stop\n");
	printk("  iwpriv wlanx mp_rate {2-108,128-143,144-163}\n");
	printk("  iwpriv wlanx mp_channel {1-14}\n");
	printk("  iwpriv wlanx mp_bandwidth [40M={0|1|2},shortGI={0|1}]\n");
	printk("        - default: BW=0, shortGI=0\n");
	printk("  iwpriv wlanx mp_txpower [patha=x,pathb=y]\n");
	printk("        - if no parameters, driver will set tx power according to flash setting.\n");
	printk("  iwpriv wlanx mp_phypara xcap=x\n");
	printk("  iwpriv wlanx mp_bssid 001122334455\n");
	printk("  iwpriv wlanx mp_ant_tx {a,b,ab}\n");
	printk("  iwpriv wlanx mp_ant_rx {a,b,ab}\n");
	printk("  iwpriv wlanx mp_arx {start,stop,filter_SA, filter_DA, filter_BSSID}\n");
	printk("  iwpriv wlanx mp_ctx [time=t,count=n,background,stop,pkt,cs,stone,scr]\n");
	printk("        - if \"time\" is set, tx in t sec. if \"count\" is set, tx with n packet.\n");
	printk("        - if \"background\", it will tx continuously until \"stop\" is issued.\n");
	printk("        - if \"pkt\", send cck packet in packet mode (not h/w).\n");
	printk("        - if \"cs\", send cck packet with carrier suppression.\n");
	printk("        - if \"stone\", send packet in single-tone.\n");
	printk("        - default: tx infinitely (no background).\n");
	printk("  iwpriv wlanx mp_query {rssi,weight=x}\n");
#ifdef MP_PSD_SUPPORT
	printk("  iwpriv wlanx mp_psd\n");
#endif
	printk("  iwpriv wlanx mp_ther\n");
	printk("  iwpriv wlanx mp_pwrtrk [ther={7-29}, stop]\n");

	printk("  iwpriv wlanx mp_reset_stats\n");
	printk("        - to reset tx and rx count\n");
	printk("  iwpriv wlanx mp_get_pwr\n");
#ifdef B2B_TEST
	printk("  iwpriv wlanx mp_tx [da=xxxxxx,time=n,count=n,len=n,retry=n,err=n]\n");
	printk("        - if \"time\" is set, tx in t sec. if \"count\" is set, tx with n packet.\n");
	printk("        - if \"time=-1\", tx infinitely.\n");
	printk("        - If \"err=1\", display statistics when tx err.\n");
 	printk("        - default: da=ffffffffffff, time=0, count=1000, len=1500,\n");
 	printk("              retry=6(mac retry limit), err=1.\n");
#if 0
	printk("  iwpriv wlanx mp_rx [ra=xxxxxx,quiet=t,interval=n]\n");
	printk("        - ra: rx mac. defulat is burn-in mac\n");
	printk("        - quiet_time: quit rx if no rx packet during quiet_time. default is 5s\n");
	printk("        - interval: report rx statistics periodically in sec.\n");
	printk("              default is 0 (no report).\n");
#endif
	printk("  iwpriv wlanx mp_brx {start[,ra=xxxxxx],stop}\n");
	printk("        - start: start rx immediately.\n");
 	printk("        - ra: rx mac. defulat is burn-in mac.\n");
	printk("        - stop: stop rx immediately.\n");
#endif// B2B_TEST
#endif // GREEN_HILL

	if (!priv->pshare->rf_ft_var.mp_specific)
		printk("!!! Warning: Please set MIB mp_specific first\n");

}


/*
 * stop mp testing. recover system to previous status.
 */
void mp_stop_test(struct rtl8192cd_priv *priv)
{
	int i;

	if (!netif_running(priv->dev))	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))	{
		printk("Fail: not in MP mode\n");
		return;
	}

#if 1//def CONFIG_RTL8672
	// make sure mp_ctx action stop, otherwise it will cause memory leak(skb_pool_ptr) for linux SDK
	//	 ,or crash dump for OSK because free skb_pool using rtl_kfree_skb()
	if (OPMODE & WIFI_MP_CTX_BACKGROUND) {
		mp_ctx(priv, "stop");
	}
#endif

	// enable beacon
	RTL_W8(TXPAUSE, RTL_R8(TXPAUSE) & ~STOP_BCN);
	OPMODE_VAL(OPMODE & ~WIFI_MP_STATE);

	priv->pshare->ThermalValue = 0;


	printk("Please restart the interface\n");
}


/*
 * set data rate
 */
void mp_set_datarate(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned char rate, rate_org;
	char tmpbuf[32];
#if 0
	RF92CD_RADIO_PATH_E eRFPath;
#endif

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}
	if (OPMODE & WIFI_MP_CTX_BACKGROUND)
	{
		printk("Fail: In MP background mode, please stop and retry it again\n");
		return;
	}

	rate = _atoi((char *)data, 10);
	if(CHECKICIS92D() || CHECKICIS8812() || CHECKICIS8881A() || CHECKICIS8814() || CHECKICIS8822()) {
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
			if (is_CCK_rate(rate)) {
				printk("(%d/2) Mbps data rate is not supported in A band\n", rate);
				return;
			}
		}
	}

	/* Translate old rate coding to new rate coding. 8814  doesn't have to translate
	     becuase it uses mp_version so that MP tool gives new rate coding */
	if(GET_CHIP_VER(priv) != VERSION_8814A){
		if( rate >= 144 && rate <= 144 + 19){// For VHT NSS1 MCS0 to NSS2 MCS 9
			printk("Translate old rate coding (%d) to new one (%d)\n", rate, rate - 144 + VHT_RATE_ID);
			rate = rate - 144 + VHT_RATE_ID;  // Change base from 144 to _NSS1_MCS0_RATE_ (160)
		}
		else if( rate > 144 + 19){
			printk("[Error] Rate %d is for VHT NSS3. Cannot be transfered to new rate coding!\n", rate);
		}
	}

	if(!isLegalRate(rate))
	{
		printk("[Error] (%d/2) Mbps data rate may not be supported\n", rate);
		return;
	}

	rate_org = priv->pshare->mp_datarate;
	priv->pshare->mp_datarate = rate;
#if 0
	for (eRFPath = RF92CD_PATH_A; eRFPath<priv->pshare->phw->NumTotalRFPath; eRFPath++) {
		if (!PHYCheckIsLegalRfPath8192cPci(priv, eRFPath))
			continue;
		if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20) {
				PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, rRfChannel, (BIT(10) | BIT(11)), 0x01);
		} else if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40) {
					PHY_SetRFReg(priv, (RF92CD_RADIO_PATH_E)eRFPath, rRfChannel, (BIT(11)), 0x00);
		} else if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_80) {
		}
		delay_us(100);
	}
#endif
/*
	if(CHECKICIS92C()) {
		if (is_CCK_rate(priv->pshare->mp_datarate)) {
			PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x0f400);
		} else {
			if(IS_UMC_A_CUT_88C(priv) || GET_CHIP_VER(priv) == VERSION_8192C )
				PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f000);
			else
				PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f200);
		}
	}
*/
	mp_8192CD_tx_setting(priv);

	if (rate <= 108)
		sprintf(tmpbuf, "Set data rate to %d Mbps\n", rate/2);
	else if (rate >= HT_RATE_ID && rate < VHT_RATE_ID)
		sprintf(tmpbuf, "Set data rate to MCS%d\n", (rate-HT_RATE_ID));
	else
		sprintf(tmpbuf, "Set data rate to NSSS%d MCS%d\n", (rate - VHT_RATE_ID)/10 + 1, (rate - VHT_RATE_ID)%10);
	printk(tmpbuf);
}

#if (IC_LEVEL >= LEVEL_8814)
int mp_AC_find_Rx_Gain_Offset_ch_group(int channel)
{
	int idx = 0;
	if (channel >= 165) {
		idx = 6;
	} else if (channel >= 149) {
		idx = 5;
	} else if (channel >= 132) {
		idx = 4;
	} else if (channel >= 116) {
		idx = 3;
	} else if (channel >= 100) {
		idx = 2;
	} else if (channel >= 52) {
		idx = 1;
	} else { //(channel >= 36)
		idx = 0;
	}

	return idx;

}

int mp_TransRxGainOffset2IG(int data)
{
	if ((data==6)||(data==7))
		return 0x1a;
	else if ((data==4)||(data==5))
		return 0x1c;
	else if ((data == 2)||(data == 3))
		return 0x1e;
	else if (data == 14)
		return 0x22;
	else if ((data == 12)||(data == 13))
		return 0x24;
	else if ((data == 10)||(data == 11))
		return 0x26;
	else if ((data == 8)||(data == 9))
		return 0x28;
	else // ((data == 0)||(data == 1)||(data == 15))
		return 0x20;

}

static unsigned int get_ig_reg(enum _CHIP_VERSION_ chip, unsigned char path){

	if(path >= RF92CD_PATH_MAX){
		panic_printk("[Error] [%s] path num not supported!\n",__FUNCTION__);
		return 0;
	}

	/* N-series IG registers*/
	if(chip == VERSION_8197F || chip == VERSION_8192E || chip == VERSION_8188E || chip == VERSION_8192C){
		switch(path){
		case RF92CD_PATH_A:
			return 0xc50;
		case RF92CD_PATH_B:
			return 0xc58;
		default:
			return 0xc50;
		}
	}/* AC-series IG registers*/
	else if(chip == VERSION_8822B || chip == VERSION_8814A || chip == VERSION_8812E){
		switch(path){
		case RF92CD_PATH_A:
			return 0xc50;
		case RF92CD_PATH_B:
			return 0xe50;
		default:
			return 0xc50;
		}
	}else{
		panic_printk("[Error] [%s] ic not supported!\n",__FUNCTION__);
		return 0;
	}
}

void mp_set_rx_gain(struct rtl8192cd_priv *priv, unsigned char *data)
{
	int group_idx,rx_gain_offset,initial_gain;
	unsigned char path;
	unsigned int ig_reg;
	char *val;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	if (get_value_by_token((char *)data, "on")){

		if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)
			group_idx = 0;
		else if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
			group_idx = mp_AC_find_Rx_Gain_Offset_ch_group(priv->pmib->dot11RFEntry.dot11channel);

		for(path = priv->pshare->phw->rx_path_start; path <= priv->pshare->phw->rx_path_end; path++){
			if((ig_reg = get_ig_reg(GET_CHIP_VER(priv),path)) == 0){
				panic_printk("[Error] get_ig_reg fail!\n",__FUNCTION__);
				return;
			}

			rx_gain_offset = *((priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A)+path*MAX_5G_CHANNEL_NUM+group_idx);
			initial_gain = mp_TransRxGainOffset2IG(rx_gain_offset);
			panic_printk("path %d group_idx=%d, rx_gain_offset=%d, initial_gain %x, IG reg %x\n",path, group_idx, rx_gain_offset, initial_gain, ig_reg);

			if ((initial_gain > 0x28) || (initial_gain < 0x1a)){
				panic_printk("Error initial_gain %x!!!\n",initial_gain);
				continue;
			}
			RTL_W8(ig_reg, (RTL_R8(ig_reg) & 0x80)| initial_gain);
		}
		return;
	}else if (get_value_by_token((char *)data, "off")){
		for(path = priv->pshare->phw->rx_path_start; path <= priv->pshare->phw->rx_path_end; path++){
			if((ig_reg = get_ig_reg(GET_CHIP_VER(priv),path)) == 0){
				panic_printk("[Error] get_ig_reg fail!\n",__FUNCTION__);
				return;
			}
			RTL_W8(ig_reg, (RTL_R8(ig_reg) & 0x80)| 0x20);
		}
		return;
	}else
		panic_printk("Please input 'iwpriv wlanx mp_set_rx_gain on/off'\n");
}

#endif

/*
 * set channel
 */
void mp_set_channel(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned char channel, channel_org;
	char tmpbuf[48];
	unsigned int eRFPath;
	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	channel = (unsigned char)_atoi((char *)data, 10);
	priv->pshare->working_channel = channel;

	if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)
	{
		if (priv->pshare->is_40m_bw &&
			((channel < 3) || (channel > 12))) {
			sprintf(tmpbuf, "channel %d is invalid\n", channel);
			printk(tmpbuf);
			return;
		}
	}

    channel_org = priv->pmib->dot11RFEntry.dot11channel;
	priv->pmib->dot11RFEntry.dot11channel = channel;


	if(GET_CHIP_VER(priv)==VERSION_8822B)
	{
		GET_HAL_INTERFACE(priv)->PHYUpdateBBRFValHandler(priv, channel,priv->pshare->offset_2nd_chan);
		for(eRFPath = 0 ; eRFPath < 2 ; eRFPath++) {
		   PHY_SetRFReg(priv, eRFPath, rRfChannel, 0xff, channel);
		}

		priv->pmib->dot11RFEntry.dot11channel = channel;

		GET_HAL_INTERFACE(priv)->PHYSetOFDMTxPowerHandler(priv, channel);

		if (priv->pshare->curr_band == BAND_2G)
			GET_HAL_INTERFACE(priv)->PHYSetCCKTxPowerHandler(priv, channel);

		PHY_IQCalibrate(priv);
        sprintf(tmpbuf, "Change channel %d to channel %d\n", channel_org, channel);
	    printk(tmpbuf);

		return;
	}

// TODO: 8814AE BB/RF


	if (priv->pshare->rf_ft_var.use_frq_2_3G)
		channel += 14;

	{
		unsigned int val_read;
		unsigned int val= channel;

		val_read = PHY_QueryRFReg(priv, 0, 0x18, bMask20Bits, 1);
	if(CHECKICIS92D())
		val_read &= 0xffffff00;
	else
		val_read &= 0xfffffff0;

		for(eRFPath = RF92CD_PATH_A; eRFPath < RF92CD_PATH_MAX; eRFPath++) {
			{
				PHY_SetRFReg(priv, eRFPath, 0x18, bMask20Bits, val_read | val);
			}
		}

		channel = val;
	}






//#ifndef CONFIG_RTL_92D_SUPPORT
	if(CHECKICIS92C()) {
		if (priv->pshare->rf_ft_var.use_frq_2_3G)
			channel -= 14;
	}
//#endif

	if (GET_CHIP_VER(priv) == VERSION_8192E ||
		(GET_CHIP_VER(priv) == VERSION_8197F )) {
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

	priv->pshare->working_channel = channel;

	//CCK Shaping Filter
	if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G
#ifdef HIGH_POWER_EXT_LNA
		&& !(GET_CHIP_VER(priv) == VERSION_8188E && priv->pshare->rf_ft_var.use_ext_lna)
#endif
		)
		set_CCK_swing_index(priv, priv->pshare->mp_cck_swing_idx);

//	mp_8192CD_tx_setting(priv);


    if(GET_CHIP_VER(priv) == VERSION_8197F){
        /* only A cut needs synthesize */
        if(_GET_HAL_DATA(priv)->cutVersion == ODM_CUT_A)
            set_lck_cv(priv, channel);
        PHY_IQCalibrate(priv);
    }
#if 0//def TXPWR_LMT
	if (!priv->pshare->rf_ft_var.disable_txpwrlmt){
		int i;
		int max_idx;

		find_pwr_limit(priv);

		if (!priv->pshare->txpwr_lmt_OFDM || !priv->pshare->tgpwr_OFDM){
			DEBUG_INFO("No limit for OFDM TxPower\n");
			max_idx=255;
		}else{
			// maximum additional power index
			max_idx = (priv->pshare->txpwr_lmt_OFDM - priv->pshare->tgpwr_OFDM);
		}

		for (i=0; i<=7; i++) {
			priv->pshare->phw->OFDMTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->OFDMTxAgcOffset_A[i], max_idx);
			priv->pshare->phw->OFDMTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->OFDMTxAgcOffset_B[i], max_idx);
			//printk("priv->pshare->phw->OFDMTxAgcOffset_A[%d]=%x\n",i, priv->pshare->phw->OFDMTxAgcOffset_A[i]);
			//printk("priv->pshare->phw->OFDMTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->OFDMTxAgcOffset_B[i]);
		}


		if (!priv->pshare->txpwr_lmt_HT1S || !priv->pshare->tgpwr_HT1S){
			DEBUG_INFO("No limit for HT1S TxPower\n");
			max_idx=255;
		}else{
			// maximum additional power index
			max_idx = (priv->pshare->txpwr_lmt_HT1S - priv->pshare->tgpwr_HT1S);
		}

		for (i=0; i<=7; i++) {
			priv->pshare->phw->MCSTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx);
			priv->pshare->phw->MCSTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_B[i], max_idx);
			//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_A[i]);
			//printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_B[i]);
		}

		if (!priv->pshare->txpwr_lmt_HT2S || !priv->pshare->tgpwr_HT2S){
			DEBUG_INFO("No limit for HT2S TxPower\n");
			max_idx=255;
		}else{
			// maximum additional power index
			max_idx = (priv->pshare->txpwr_lmt_HT2S - priv->pshare->tgpwr_HT2S);
		}

		for (i=8; i<=15; i++) {
			priv->pshare->phw->MCSTxAgcOffset_A[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_A[i], max_idx);
			priv->pshare->phw->MCSTxAgcOffset_B[i] = POWER_MIN_CHECK(priv->pshare->phw->MCSTxAgcOffset_B[i], max_idx);
			//printk("priv->pshare->phw->MCSTxAgcOffset_A[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_A[i]);
			//printk("priv->pshare->phw->MCSTxAgcOffset_B[%d]=%x\n",i, priv->pshare->phw->MCSTxAgcOffset_B[i]);
		}

		if (!priv->pshare->txpwr_lmt_CCK || !priv->pshare->tgpwr_CCK){
			DEBUG_INFO("No limit for CCK TxPower\n");
			max_idx=255;
		}else{
			// maximum additional power index
			max_idx = (priv->pshare->txpwr_lmt_CCK - priv->pshare->tgpwr_CCK);
		}

		for (i=0; i<=3; i++) {
			priv->pshare->phw->CCKTxAgc_A[i] = POWER_MIN_CHECK(priv->pshare->phw->CCKTxAgc_A[i], max_idx);
			priv->pshare->phw->CCKTxAgc_B[i] = POWER_MIN_CHECK(priv->pshare->phw->CCKTxAgc_B[i], max_idx);
			//printk("priv->pshare->phw->CCKTxAgc_A[%d]=%x\n",i, priv->pshare->phw->CCKTxAgc_A[i]);
			//printk("priv->pshare->phw->CCKTxAgc_B[%d]=%x\n",i, priv->pshare->phw->CCKTxAgc_B[i]);
		}
	}
#endif

	sprintf(tmpbuf, "Change channel %d to channel %d\n", channel_org, channel);
	printk(tmpbuf);
}

#ifdef CONFIG_RF_DPK_SETTING_SUPPORT
/*
 * set dpk
 */
int mp_set_dpk(struct rtl8192cd_priv *priv, unsigned char *data)
{
	int i = 0, j = 0;
	unsigned char tmp_data[64] = {0};

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return 0;
	}


	if (!strcmp(data, "start")) {

		sprintf(data, " ");

		ODMPTR->RFCalibrateInfo.DPPathAResult[0] = 0;
		ODMPTR->RFCalibrateInfo.DPPathBResult[0] = 0;
		ODMPTR->RFCalibrateInfo.DPPathAResult[1] = 0;
		ODMPTR->RFCalibrateInfo.DPPathBResult[1] = 0;
		ODMPTR->RFCalibrateInfo.DPPathAResult[2] = 0;
		ODMPTR->RFCalibrateInfo.DPPathBResult[2] = 0;
		ODMPTR->RFCalibrateInfo.bDPPathAOK = 0;
		ODMPTR->RFCalibrateInfo.bDPPathBOK = 0;

		for (i = 0; i < 3; i++)
			do_dpk_8197f(ODMPTR, TRUE, i);

		if (ODMPTR->RFCalibrateInfo.bDPPathAOK == 1 && ODMPTR->RFCalibrateInfo.bDPPathBOK == 1)
			printk("DPK is successful\n");
		else
			printk("Check DPK value\n");

		return strlen(data);

	} else if (!strcmp(data, "on")) {

		phy_dpk_enable_disable_8197f(ODMPTR, true);
		printk("DPK on\n");

		sprintf(data, " ");
		return strlen(data);

	} else if (!strcmp(data, "off")) {

		phy_dpk_enable_disable_8197f(ODMPTR, false);
		printk("DPK off\n");

		sprintf(data, " ");
		return strlen(data);

	} else if (!strcmp(data, "RF_DPK_DP_PATH_A_OK")) {

		sprintf(data, "%d ", ODMPTR->RFCalibrateInfo.bDPPathAOK);

		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "RF_DPK_DP_PATH_B_OK")) {

		sprintf(data, "%d ", ODMPTR->RFCalibrateInfo.bDPPathBOK);

		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "RF_DPK_PWSF_2G_A")) {
		sprintf(data, "");

		for (i = 0; i < 3 ; i++)
		{
			sprintf(tmp_data, "%d ", ODMPTR->RFCalibrateInfo.pwsf_2g_a[i]);
			strcat(data, tmp_data);
		}
		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "RF_DPK_PWSF_2G_B")) {
		sprintf(data, "");

		for (i = 0; i < 3 ; i++)
		{
			sprintf(tmp_data, "%d ", ODMPTR->RFCalibrateInfo.pwsf_2g_b[i]);
			strcat(data, tmp_data);
		}
		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "RF_DPK_LUT_2G_EVEN_A0")) {
		sprintf(data, "");

		for (i = 0; i < 64 ; i++)
		{
			sprintf(tmp_data, "%d ", ODMPTR->RFCalibrateInfo.lut_2g_even_a[0][i]);
			strcat(data, tmp_data);
		}
		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "RF_DPK_LUT_2G_ODD_A0")) {
		sprintf(data, "");

		for (i = 0; i < 64 ; i++)
		{
			sprintf(tmp_data, "%d ", ODMPTR->RFCalibrateInfo.lut_2g_odd_a[0][i]);
			strcat(data, tmp_data);
		}
		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "RF_DPK_LUT_2G_EVEN_B0")) {
		sprintf(data, "");

		for (i = 0; i < 64 ; i++)
		{
			sprintf(tmp_data, "%d ", ODMPTR->RFCalibrateInfo.lut_2g_even_b[0][i]);
			strcat(data, tmp_data);
		}
		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "RF_DPK_LUT_2G_ODD_B0")) {
		sprintf(data, "");

		for (i = 0; i < 64 ; i++)
		{
			sprintf(tmp_data, "%d ", ODMPTR->RFCalibrateInfo.lut_2g_odd_b[0][i]);
			strcat(data, tmp_data);
		}
		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "RF_DPK_LUT_2G_EVEN_A1")) {
		sprintf(data, "");

		for (i = 0; i < 64 ; i++)
		{
			sprintf(tmp_data, "%d ", ODMPTR->RFCalibrateInfo.lut_2g_even_a[1][i]);
			strcat(data, tmp_data);
		}
		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "RF_DPK_LUT_2G_ODD_A1")) {
		sprintf(data, "");

		for (i = 0; i < 64 ; i++)
		{
			sprintf(tmp_data, "%d ", ODMPTR->RFCalibrateInfo.lut_2g_odd_a[1][i]);
			strcat(data, tmp_data);
		}
		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "RF_DPK_LUT_2G_EVEN_B1")) {
		sprintf(data, "");

		for (i = 0; i < 64 ; i++)
		{
			sprintf(tmp_data, "%d ", ODMPTR->RFCalibrateInfo.lut_2g_even_b[1][i]);
			strcat(data, tmp_data);
		}
		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "RF_DPK_LUT_2G_ODD_B1")) {
		sprintf(data, "");

		for (i = 0; i < 64 ; i++)
		{
			sprintf(tmp_data, "%d ", ODMPTR->RFCalibrateInfo.lut_2g_odd_b[1][i]);
			strcat(data, tmp_data);
		}
		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "RF_DPK_LUT_2G_EVEN_A2")) {
		sprintf(data, "");

		for (i = 0; i < 64 ; i++)
		{
			sprintf(tmp_data, "%d ", ODMPTR->RFCalibrateInfo.lut_2g_even_a[2][i]);
			strcat(data, tmp_data);
		}
		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "RF_DPK_LUT_2G_ODD_A2")) {
		sprintf(data, "");

		for (i = 0; i < 64 ; i++)
		{
			sprintf(tmp_data, "%d ", ODMPTR->RFCalibrateInfo.lut_2g_odd_a[2][i]);
			strcat(data, tmp_data);
		}
		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "RF_DPK_LUT_2G_EVEN_B2")) {
		sprintf(data, "");

		for (i = 0; i < 64 ; i++)
		{
			sprintf(tmp_data, "%d ", ODMPTR->RFCalibrateInfo.lut_2g_even_b[2][i]);
			strcat(data, tmp_data);
		}
		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "RF_DPK_LUT_2G_ODD_B2")) {
		sprintf(data, "");

		for (i = 0; i < 64 ; i++)
		{
			sprintf(tmp_data, "%d ", ODMPTR->RFCalibrateInfo.lut_2g_odd_b[2][i]);
			strcat(data, tmp_data);
		}
		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	} else if (!strcmp(data, "sram_read")) {
		sprintf(data, " ");

		phy_lut_sram_read_8197f(ODMPTR, phy_dpk_channel_transfer_8197f(ODMPTR));

		printk("DPK_Result:%s\n", data);
		return strlen(data) + 1;

	}
	else
	{
		sprintf(data, " ");
		printk("DPK command not found\n");
		return strlen(data);
	}
}

void mp_tx_dpk_tracking(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	unsigned int target_ther = 0;
	int tmp_ther;
	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}

	val = get_value_by_token((char *)data, "stop");
	if (val) {
		if (priv->pshare->mp_tx_dpk_tracking == FALSE)
			return;
		priv->pshare->mp_tx_dpk_tracking = FALSE;
		priv->pmib->dot11RFEntry.ther = 0;
		printk("mp tx dpk tracking stop\n");
		return;
	}

	val = get_value_by_token((char *)data, "ther=");
	if (val)
		target_ther = _atoi(val, 10);
	else if (priv->pmib->dot11RFEntry.ther)
		target_ther = priv->pmib->dot11RFEntry.ther;
	target_ther = target_ther & 0xff;

	if (!target_ther) {
		printk("Fail: tx dpk tracking has no target thermal value\n");
		return;
	}

	if ((target_ther < 0x05) || (target_ther > 0x3c)) {
		panic_printk("Tx DPK Tracking: unreasonable target ther %d, disable DPK tracking\n", target_ther);
		target_ther = 0;
		return;
	}

	if(priv->pmib->dot11RFEntry.ther && priv->pshare->ThermalValue)
		priv->pshare->ThermalValue += (target_ther - priv->pmib->dot11RFEntry.ther);

	priv->pmib->dot11RFEntry.ther = target_ther;

	priv->pshare->mp_tx_dpk_tracking = TRUE;
	printk("mp tx dpk tracking start, target value=%d\n", target_ther);
}

#endif




void mp_set_tx_power_8822(struct rtl8192cd_priv *priv, char pwrA, char pwrB)
{
	unsigned char i, pwr;
	for(i = 0 ; i <= 1 ; i++){
		switch(i){
		case 0:
			pwr = pwrA;
			break;
		case 1:
			pwr = pwrB;
			break;
		}
		PHYSetTxPower88XX(priv, TX_AGC_OFDM_6M, TX_AGC_VHT_NSS2_MCS9, i, pwr);
	}
}

/*
 * set tx power
 */
void mp_set_tx_power(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int channel = priv->pmib->dot11RFEntry.dot11channel;
	char *val, tmpbuf[64];
	unsigned int writeVal;
	char baseA,baseB, byte[4];
	int i;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	if (strlen(data) == 0) {
		priv->pshare->mp_txpwr_patha = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[channel-1];
		priv->pshare->mp_txpwr_pathb = priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[channel-1];
	} else {
		val = get_value_by_token((char *)data, "patha=");
		if (val) {
			priv->pshare->mp_txpwr_patha = _atoi(val, 10);
		}

		val = get_value_by_token((char *)data, "pathb=");
		if (val) {
			priv->pshare->mp_txpwr_pathb = _atoi(val, 10);
		}
	}
/*
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		if(priv->pshare->mp_txpwr_patha > HP_OFDM_POWER_MAX)
			priv->pshare->mp_txpwr_patha = HP_OFDM_POWER_MAX;
		if(priv->pshare->mp_txpwr_pathb > HP_OFDM_POWER_MAX)
			priv->pshare->mp_txpwr_pathb = HP_OFDM_POWER_MAX;

		sprintf(tmpbuf, "Set OFDM power level path_A:%d path_B:%d\n",
			priv->pshare->mp_txpwr_patha, priv->pshare->mp_txpwr_pathb);
		printk(tmpbuf);

	}
#endif
*/

	{
	baseA = priv->pshare->mp_txpwr_patha;
	baseB = priv->pshare->mp_txpwr_pathb;

	if(CHECKICIS97F()){//wish
		unsigned char path, pwr, rate;
		for(path = 0 ; path <= 1 ; path++){
			switch(path){
			case 0:
				pwr = baseA;
				break;
			case 1:
				pwr = baseB;
				break;
			}
			for (rate = ODM_RATE1M; rate <= ODM_RATEMCS15; rate++) //cck to mcs15
				config_phydm_write_txagc_8197f((&(priv->pshare->_dmODM)),pwr,path,rate);
		}
		return;
	}

	if(GET_CHIP_VER(priv)==VERSION_8822B)
	{

		panic_printk("Set 8822 power level path_A:%d path_B:%d\n", priv->pshare->mp_txpwr_patha,
				priv->pshare->mp_txpwr_pathb);

		mp_set_tx_power_8822(priv, baseA, baseB);
		return;
	}


	/**************path-A**************/
	// 18M ~ 6M
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseA + priv->pshare->phw->OFDMTxAgcOffset_A[i]);
		else
			byte[i] = baseA;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_A_Rate18_06, writeVal);

	// 54M ~ 24M
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseA + priv->pshare->phw->OFDMTxAgcOffset_A[i+4]);
		else
			byte[i] = baseA;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_A_Rate54_24, writeVal);

	// MCS3 ~ MCS0
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseA + priv->pshare->phw->MCSTxAgcOffset_A[i]);
		else
			byte[i] = baseA;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_A_Mcs03_Mcs00, writeVal);

	// MCS7 ~ MCS4
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseA + priv->pshare->phw->MCSTxAgcOffset_A[i+4]);
		else
			byte[i] = baseA;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_A_Mcs07_Mcs04, writeVal);

	// MCS11 ~ MCS8
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
		{
			//_TXPWR_REDEFINE ?? #if 0 in FOX
			byte[i] = POWER_RANGE_CHECK(baseA + priv->pshare->phw->MCSTxAgcOffset_A[i+8]);
		}
		else
			byte[i] = baseA;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_A_Mcs11_Mcs08, writeVal);

	// MCS15 ~ MCS12
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
		{
			//_TXPWR_REDEFINE ?? #if 0 in FOX
			byte[i] = POWER_RANGE_CHECK(baseA + priv->pshare->phw->MCSTxAgcOffset_A[i+12]);
		}
		else
			byte[i] = baseA;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_A_Mcs15_Mcs12, writeVal);


	/**************path-B**************/
	// 18M ~ 6M
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->OFDMTxAgcOffset_B[i]);
		else
			byte[i] = baseB;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_B_Rate18_06, writeVal);


	// 54M ~ 24M
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->OFDMTxAgcOffset_B[i+4]);
		else
			byte[i] = baseB;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_B_Rate54_24, writeVal);

	// MCS3 ~ MCS0
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->MCSTxAgcOffset_B[i]);
		else
			byte[i] = baseB;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_B_Mcs03_Mcs00, writeVal);

	// MCS7 ~ MCS4
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->MCSTxAgcOffset_B[i+4]);
		else
			byte[i] = baseB;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_B_Mcs07_Mcs04, writeVal);

	// MCS11 ~ MCS8
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
		{
			//_TXPWR_REDEFINE ?? #if 0 in FOX
			byte[i] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->MCSTxAgcOffset_B[i+8]);
		}
		else
			byte[i] = baseB;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_B_Mcs11_Mcs08, writeVal);

	// MCS15 ~ MCS12
	for (i=0; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
		{
			//_TXPWR_REDEFINE ?? #if 0 in FOX
			byte[i] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->MCSTxAgcOffset_B[i+12]);
		}
		else
			byte[i] = baseB;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_OFDM_POWER_MAX);
	}
#endif
	writeVal = (byte[0]<<24) | (byte[1]<<16) |(byte[2]<<8) | byte[3];
	RTL_W32(rTxAGC_B_Mcs15_Mcs12, writeVal);

/*
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		if(priv->pshare->mp_txpwr_patha > HP_CCK_POWER_MAX)
			priv->pshare->mp_txpwr_patha = HP_CCK_POWER_MAX;
		if(priv->pshare->mp_txpwr_pathb > HP_CCK_POWER_MAX)
			priv->pshare->mp_txpwr_pathb = HP_CCK_POWER_MAX;

		sprintf(tmpbuf, "Set CCK power level path_A:%d path_B:%d\n",
			priv->pshare->mp_txpwr_patha, priv->pshare->mp_txpwr_pathb);
		printk(tmpbuf);
	}
#endif
*/

	// CCK-A 1M
	if (priv->pshare->rf_ft_var.pwr_by_rate)
		writeVal = POWER_RANGE_CHECK(baseA + priv->pshare->phw->CCKTxAgc_A[3]);
	else
		writeVal = baseA;
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		writeVal = POWER_MIN_CHECK(writeVal, HP_CCK_POWER_MAX);
	}
#endif
	PHY_SetBBReg(priv, rTxAGC_A_CCK1_Mcs32, 0x0000ff00, writeVal);

	// CCK-B 11M ~ 2M
	for (i=1; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->CCKTxAgc_B[i]);
		else
			byte[i] = baseB;
	}
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		for (i=1; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_CCK_POWER_MAX);
	}
#endif
	writeVal = (byte[1]<<16) |(byte[2]<<8) | byte[3];
	PHY_SetBBReg(priv, rTxAGC_B_CCK5_1_Mcs32, 0xffffff00, writeVal);

	// CCK-A 11M ~ 2M CCK-B 11M
	for (i=1; i<4; i++) {
		if (priv->pshare->rf_ft_var.pwr_by_rate)
			byte[i] = POWER_RANGE_CHECK(baseA + priv->pshare->phw->CCKTxAgc_A[i-1]);
		else
			byte[i] = baseA;
	}
	if (priv->pshare->rf_ft_var.pwr_by_rate)
		byte[0] = POWER_RANGE_CHECK(baseB + priv->pshare->phw->CCKTxAgc_B[0]);
	else
		byte[0] = baseB;
#ifdef HIGH_POWER_EXT_PA
	if (priv->pshare->rf_ft_var.use_ext_pa) {
		for (i=0; i<4; i++)
			byte[i] = POWER_MIN_CHECK(byte[i], HP_CCK_POWER_MAX);
	}
#endif
	writeVal = (byte[1]<<24) | (byte[2]<<16) |(byte[3]<<8) | byte[0];
	PHY_SetBBReg(priv, rTxAGC_A_CCK11_2_B_CCK11, 0xffffffff, writeVal);

}
	mp_8192CD_tx_setting(priv);

#ifdef HIGH_POWER_EXT_PA
	if (!priv->pshare->rf_ft_var.use_ext_pa)
#endif
	{
		sprintf(tmpbuf, "Set power level path_A:%d path_B:%d\n",
			priv->pshare->mp_txpwr_patha, priv->pshare->mp_txpwr_pathb);
		printk(tmpbuf);
	}

}


/*
 * continuous tx
 *  command: "iwpriv wlanx mp_ctx [time=t,count=n,background,stop,pkt,cs,stone,scr]"
 *			  if "time" is set, tx in t sec. if "count" is set, tx with n packet
 *			  if "background", it will tx continuously until "stop" is issue
 *			  if "pkt", send cck packets with packet mode (not hardware)
 *			  if "cs", send cck packet with carrier suppression
 *			  if "stone", send packet in single-tone
 *			  default: tx infinitely (no background)
 */
void mp_ctx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int orgTCR;
	unsigned char pbuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
	int payloadlen=1500, time=-1;
	struct sk_buff *skb;
	struct wlan_ethhdr_t *pethhdr;
	int len, i=0, q_num;
	unsigned char pattern;
	char *val;
	unsigned long end_time=0;
	unsigned long flags=0;
	unsigned long flags2=0;
	int tx_from_isr=0, background=0;
	struct rtl8192cd_hw *phw = GET_HW(priv);
	volatile unsigned int head, tail;
	RF92CD_RADIO_PATH_E eRFPath;

/*
// We need to turn off ADC before entering CTX mode
	RTL_W32(0xe70, (RTL_R32(0xe70) & 0xFE1FFFFF ) );
	delay_us(100);
*/
	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	orgTCR = RTL_R32(TCR);

#if (IC_LEVEL >= LEVEL_8814) || (IC_LEVEL == LEVEL_92E)
	if(CHECKICIS8814() || CHECKICIS97F() ||CHECKICIS8822() || (GET_CHIP_VER(priv) == VERSION_8192E)) //wish
		mp_set_tmac_tx(priv);
#endif
	// get count
	val = get_value_by_token((char *)data, "count=");
	if (val) {
		priv->pshare->mp_ctx_count = _atoi(val, 10);
		if (priv->pshare->mp_ctx_count)
			time = 0;
	}

	// get time
	val = get_value_by_token((char *)data, "time=");
	if (val) {
		if (!memcmp(val, "-1", 2))
			time = -1;
		else
			time = RTL_SECONDS_TO_JIFFIES(_atoi(val, 10));
		if (time > 0)
			end_time = jiffies + time;
	}

	// get payload len
	val = get_value_by_token((char *)data, "len=");
	if (val) {
		priv->pshare->mp_pkt_len = _atoi(val, 10);
		if (priv->pshare->mp_pkt_len < 20) {
			printk("len should be greater than 20!\n");
			return;
		}
	}

	// get background
	val = get_value_by_token((char *)data, "background");
	if (val)
		background = 1;

	// get carrier suppression mode
	val = get_value_by_token((char *)data, "cs");
	if (val) {
		if (!is_CCK_rate(priv->pshare->mp_datarate)) {
			printk("Specify carrier suppression but not CCK rate!\n");
			return;
		}
		else
			OPMODE_VAL(OPMODE | WIFI_MP_CTX_CCK_CS);
	}

	// get single-tone
	val = get_value_by_token((char *)data, "stone");
	if (val)
		OPMODE_VAL(OPMODE | WIFI_MP_CTX_ST);

	// get single-carrier
	val = get_value_by_token((char *)data, "scr");
	if (val) {
		if (is_CCK_rate(priv->pshare->mp_datarate)) {
			printk("Specify single carrier but CCK rate!\n");
			return;
		}
		else
			OPMODE_VAL(WIFI_MP_CTX_SCR);
	}

	// get stop
	val = get_value_by_token((char *)data, "stop");
	if (val) {

		if (!(OPMODE & WIFI_MP_CTX_BACKGROUND)) {
			printk("Error! Continuous-Tx is not on-going.\n");
			return;
		}
		goto stop_tx;
	}


	// get tx-isr flag, which is set in ISR when Tx ok
	val = get_value_by_token((char *)data, "tx-isr");
	if (val) {
		if (OPMODE & WIFI_MP_CTX_BACKGROUND) {

			if (((OPMODE & WIFI_MP_CTX_ST) || !(OPMODE & (WIFI_MP_CTX_PACKET|WIFI_MP_CTX_CCK_CS)))
					&& (priv->net_stats.tx_packets > 0))
				return;

			SMP_LOCK(flags);
			tx_from_isr = 1;
			time = -1;
		}
	}

	if (priv->pshare->mp_pkt_len)
		payloadlen = priv->pshare->mp_pkt_len;
	else
		payloadlen = 1500;

	if (!tx_from_isr && (OPMODE & WIFI_MP_CTX_BACKGROUND)) {
		printk("Continuous-Tx is on going. You can't issue any tx command except 'stop'.\n");
		return;
	}
	// get packet mode
		val = get_value_by_token((char *)data, "pkt");
		if (val)
			OPMODE_VAL(OPMODE | WIFI_MP_CTX_PACKET);

	if (background) {
		priv->pshare->skb_pool_ptr = kmalloc(sizeof(struct sk_buff)*NUM_MP_SKB, GFP_KERNEL);
		if (priv->pshare->skb_pool_ptr == NULL) {
			printk("Allocate skb fail!\n");
			return;
		}
		memset(priv->pshare->skb_pool_ptr, 0, sizeof(struct sk_buff)*NUM_MP_SKB);
		for (i=0; i<NUM_MP_SKB; i++) {
			priv->pshare->skb_pool[i] = (struct sk_buff *)(priv->pshare->skb_pool_ptr + i * sizeof(struct sk_buff));
			priv->pshare->skb_pool[i]->head = kmalloc(RX_BUF_LEN, GFP_KERNEL);
			if (priv->pshare->skb_pool[i]->head == NULL) {
				for (i=0; i<NUM_MP_SKB; i++) {
					if (priv->pshare->skb_pool[i]->head)
						kfree(priv->pshare->skb_pool[i]->head);
					else
						break;
				}
				kfree(priv->pshare->skb_pool_ptr);
				printk("Allocate skb fail!\n");
				return;
			}
			else {
				priv->pshare->skb_pool[i]->data = priv->pshare->skb_pool[i]->head;
				priv->pshare->skb_pool[i]->tail = priv->pshare->skb_pool[i]->data;
				priv->pshare->skb_pool[i]->end = priv->pshare->skb_pool[i]->head + RX_BUF_LEN;
				priv->pshare->skb_pool[i]->len = 0;
			}
		}
		priv->pshare->skb_head = 0;
		priv->pshare->skb_tail = 0;

		/*disable interrupt and change OPMODE here to avoid re-enter*/
		SAVE_INT_AND_CLI(flags);
		SMP_LOCK(flags);

		head = get_txhead(phw, BE_QUEUE);
		tail = get_txtail(phw, BE_QUEUE);

		while (head != tail) {
			DEBUG_INFO("BEQ head/tail=%d/%d\n", head, tail);
			rtl8192cd_tx_dsr((unsigned long)priv);
			delay_us(50);
			tail = get_txtail(phw, BE_QUEUE);
		}

		OPMODE_VAL(OPMODE | WIFI_MP_CTX_BACKGROUND);
		time = -1; // set as infinite
	}

	len = payloadlen + WLAN_ETHHDR_LEN;
	pattern = 0xAB;
	q_num = BE_QUEUE;

	if (!tx_from_isr) {
		if (time < 0) // infinite
			printk("Start continuous DA=%02x%02x%02x%02x%02x%02x len=%d infinite=yes",
				pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5], payloadlen);
			else if (time > 0) // by time
				printk("Start continuous DA=%02x%02x%02x%02x%02x%02x len=%d time=%ds",
					pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5],
					payloadlen, time/HZ);
			else // by count
				printk("Start TX DA=%02x%02x%02x%02x%02x%02x len=%d count=%d",
					pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5],
				payloadlen, priv->pshare->mp_ctx_count);

		if (!background) {
	  		printk(", press any key to escape.\n");
		} else
	  	printk(".\n");

		if (OPMODE & WIFI_MP_CTX_PACKET) {
			RTL_W16(TX_PTCL_CTRL, RTL_R16(TX_PTCL_CTRL) & ~DIS_CW);
			RTL_W32(EDCA_BE_PARA, (RTL_R32(EDCA_BE_PARA) & 0xffffff00) | (10 + 2 * 20));
		} else {
			RTL_W16(TX_PTCL_CTRL, RTL_R16(TX_PTCL_CTRL) | DIS_CW);
			RTL_W32(EDCA_BE_PARA, (RTL_R32(EDCA_BE_PARA) & 0xffffff00) | 0x01);

			if (is_CCK_rate(priv->pshare->mp_datarate)) {
				if (OPMODE & WIFI_MP_CTX_CCK_CS)
					mpt_ProSetCarrierSupp(priv, TRUE);
				else
					mpt_StartCckContTx(priv); //2 Continuous Tx CCK
			} else {
				if (!((OPMODE & WIFI_MP_CTX_ST) && (OPMODE & WIFI_MP_CTX_SCR)))
					mpt_StartOfdmContTx(priv);//2 Continuous Tx OFDM
				OPMODE_VAL(OPMODE | WIFI_MP_CTX_OFDM_HW);
			}
		}
		if (IS_HAL_CHIP(priv))
			RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_APWRMGT | RCR_AMF | RCR_ADF |RCR_ACRC32 |RCR_AB | RCR_AM | RCR_APM | RCR_AAP));
		else if(CONFIG_WLAN_NOT_HAL_EXIST)
		RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_AB | RCR_AM | RCR_APM | RCR_AAP));


		if (!background) {
	  		DISABLE_UART0_INT();
#ifdef _MP_TELNET_SUPPORT_
			mp_pty_write_monitor(1);
#endif //_MP_TELNET_SUPPORT_
		}

		memset(&priv->net_stats, 0,  sizeof(struct net_device_stats));
	}



        if (is_CCK_rate(priv->pshare->mp_datarate)) {
                PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x0f400);
        } else {
		{
				PHY_SetRFReg(priv, 0, 0x26, bMask20Bits, 0x4f200);
		}
	}

	i = 0;
	while (1)
	{

#ifdef _MP_TELNET_SUPPORT_
		if (!in_atomic() && !tx_from_isr && !background)
			schedule();
		if (!tx_from_isr && !background && (IS_KEYBRD_HIT()||mp_pty_is_hit()))
			break;
#else
#ifdef ALLOW_TELNET
		if (!tx_from_isr && !background && IS_KEYBRD_HIT(priv))
#else
		if (!tx_from_isr && !background && IS_KEYBRD_HIT())
#endif
			break;
#endif //_MP_TELNET_SUPPORT_

		if (time) {
			if (time != -1) {
				if (jiffies > end_time)
					break;
			}
			else {
				if ((priv->pshare->mp_ctx_count > 0) && (priv->pshare->mp_ctx_pkt >= priv->pshare->mp_ctx_count)) {
					if (background) {
						RESTORE_INT(flags);
						SMP_UNLOCK(flags);
					} else if (tx_from_isr) {
						SMP_UNLOCK(flags);
					}
					delay_ms(10);
					return;
				}
			}
		}
		else {
			if (i >= priv->pshare->mp_ctx_count)
				break;
		}
		i++;
		priv->pshare->mp_ctx_pkt++;

if ((OPMODE & WIFI_MP_CTX_ST) &&  //2 Tx Single Tone
			(i == 1)) {
			i++;

			{
				switch (priv->pshare->mp_antenna_tx) {
				case ANTENNA_B:
					eRFPath = RF92CD_PATH_B;
					break;
				case ANTENNA_A:
				default:
					eRFPath = RF92CD_PATH_A;
					break;
				}
			}
			if (CHECKICIS8812() || CHECKICIS8881A() || CHECKICIS8814() || CHECKICIS8822()) {
				PHY_SetBBReg(priv, rFPGA0_RFMOD, BIT(28), 0x0);
				PHY_SetBBReg(priv, rFPGA0_RFMOD, BIT(29), 0x0);

				//4 load LO
				PHY_SetRFReg(priv, eRFPath, LNA_Low_Gain_3, BIT(1), 1);

				//4 rf tx mode
#ifdef HIGH_POWER_EXT_PA
				if(priv->pshare->rf_ft_var.use_ext_pa)
					PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x20000); //From suggestion of BS (RF Team)
				else
#endif
					PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x20010);
				delay_us(100);

				if (eRFPath == RF92CD_PATH_A) {
					//4 3 wire off
					PHY_SetBBReg(priv, 0xc00, BIT(1)|BIT(0), 0);

					//4 RFE software pull low
					priv->pshare->RegCB0 = RTL_R32(0xcb0);
					RTL_W32(0xcb0, 0x77777777);
				if(CHECKICIS8822()){
					priv->pshare->RegCB4 = RTL_R32(0xcb4);
					RTL_W32(0xcb4, 0x00007777);
					//trsw, pape pull high
					PHY_SetBBReg(priv, 0xcbc, BIT(0)|BIT(1), 0x3);
					PHY_SetBBReg(priv, 0xcbc, BIT(3), 0x1);
				}
				}
				/*
#if defined(CONFIG_WLAN_HAL_8814AE)
				else if (eRFPath == RF92CD_PATH_C) {
					PHY_SetBBReg(priv, 0xc00, BIT(1)|BIT(0), 0);
					priv->pshare->RegCB0 = RTL_R32(0xcb0);
					RTL_W32(0xcb0, 0x77777777);
					PHY_SetBBReg(priv, 0xcb4, BIT(21), 1);
					PHY_SetBBReg(priv, 0xcb4, BIT(24), 1);
				}
				else if (eRFPath == RF92CD_PATH_D) {
					PHY_SetBBReg(priv, 0xc00, BIT(1)|BIT(0), 0);
					priv->pshare->RegCB0 = RTL_R32(0xcb0);
					RTL_W32(0xcb0, 0x77777777);
					PHY_SetBBReg(priv, 0xcb4, BIT(21), 1);
					PHY_SetBBReg(priv, 0xcb4, BIT(24), 1);
				}
#endif
*/
				else {
					//4 3 wire off
					PHY_SetBBReg(priv, 0xe00, BIT(1)|BIT(0), 0);

					//4 RFE software pull low
					priv->pshare->RegEB0 = RTL_R32(0xeb0);
					RTL_W32(0xeb0, 0x77777777);
				if(CHECKICIS8822()){
					priv->pshare->RegEB4 = RTL_R32(0xeb4);
					RTL_W32(0xeb4, 0x00007777);
					//trsw, pape pull high
					PHY_SetBBReg(priv, 0xebc, BIT(4)|BIT(5), 0x3);
					PHY_SetBBReg(priv, 0xebc, BIT(11), 0x1);
				}
				}
			} else
			{



	if (GET_CHIP_VER(priv)==VERSION_8197F){

		PHY_SetBBReg(priv, 0x4c, BIT(23), 0x1);
		PHY_SetBBReg(priv, 0x4c, BIT(26), 0x1);
		PHY_SetBBReg(priv, 0x64, BIT(7), 0x1);
		PHY_SetBBReg(priv, 0x64, BIT(1), 0x1);
		PHY_SetBBReg(priv, 0x64, BIT(0), 0x1);
        PHY_SetBBReg(priv, 0x78, BIT(16), 0x1);

		PHY_SetBBReg(priv, 0x88c, BIT(23)|BIT(22)|BIT(21)|BIT(20), 0xf);
		PHY_SetRFReg(priv, eRFPath, 0x57, BIT(1) ,0x1);
		PHY_SetRFReg(priv, eRFPath, 0x0, bMask20Bits ,0x20000);

	}

				// Start Single Tone.
				PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x0);
				PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 0x0);

			{
				PHY_SetRFReg(priv, RF92CD_PATH_A, 0x21, bMask20Bits, 0xd4000);
			}
			delay_us(100);

			if (GET_CHIP_VER(priv) != VERSION_8197F){
#ifdef HIGH_POWER_EXT_PA
				if(priv->pshare->rf_ft_var.use_ext_pa)
					PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x20000); //From suggestion of BS (RF Team)
				else
#endif
					PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x20010);
			}
			delay_us(100);

#ifdef HIGH_POWER_EXT_PA
			if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
			if(priv->pshare->rf_ft_var.use_ext_pa) {
				PHY_SetBBReg(priv, 0x860, BIT(10), 0x1);
				PHY_SetBBReg(priv, 0x864, BIT(10), 0x1);
				PHY_SetBBReg(priv, 0x870, BIT(10), 0x1);
				PHY_SetBBReg(priv, 0x870, BIT(26), 0x1);
			}
			}
#endif
			}
		}

		if ((OPMODE & WIFI_MP_CTX_SCR) &&
			(i == 1)) {

			if (CHECKICIS8812() || CHECKICIS8881A() || CHECKICIS8814() || CHECKICIS8822()) {
				// 1. if OFDM block on?
				if(!PHY_QueryBBReg(priv, 0x808, BIT(29)))
					PHY_SetBBReg(priv, 0x808, BIT(29), bEnable);//set CCK block on

				// 2. set CCK test mode off, set to CCK normal mode
				PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, bDisable);

				// 3. turn on scramble setting
				PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, bEnable);

				// 4. Turn On Continue Tx and turn off the other test modes.
				PHY_SetBBReg(priv, rSingleTone_ContTx_Jaguar, BIT(16) | BIT(17) | BIT(18), 0x2);
			} else
			{
				// 1. if OFDM block on?
				if (!PHY_QueryBBReg(priv, rFPGA0_RFMOD, bOFDMEn))
					PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, bEnable);//set OFDM block on

				// 2. set CCK test mode off, set to CCK normal mode
				PHY_SetBBReg(priv, rCCK0_System, bCCKBBMode, bDisable);

				// 3. turn on scramble setting
				PHY_SetBBReg(priv, rCCK0_System, bCCKScramble, bEnable);

				// 4. Turn On Continue Tx and turn off the other test modes.
				PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bDisable);
				PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bEnable);
				PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bEnable);
				PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMDisPwsavTx, bEnable);

				PHY_SetBBReg(priv, rOFDM1_TRxMesaure1, 0xfff, 0x404);
			}
		}

		if ((OPMODE & WIFI_MP_CTX_OFDM_HW) && (i > 1)) {
			if (background) {
				RESTORE_INT(flags);
				SMP_UNLOCK(flags);
				return;
			} else if (tx_from_isr) {
				priv->net_stats.tx_packets = 1;
				OPMODE_VAL(OPMODE & ~WIFI_MP_CTX_BACKGROUND_PENDING);
				SMP_UNLOCK(flags);
				return;
			} else {
				continue;
			}
		}

		if (background || tx_from_isr) {
			if (CIRC_SPACE(priv->pshare->skb_head, priv->pshare->skb_tail, NUM_MP_SKB) > 1) {
				skb = priv->pshare->skb_pool[priv->pshare->skb_head];
				priv->pshare->skb_head = (priv->pshare->skb_head + 1) & (NUM_MP_SKB - 1);
			} else {
				OPMODE_VAL(OPMODE | WIFI_MP_CTX_BACKGROUND_PENDING);
				priv->pshare->mp_ctx_pkt--;
				if (background)
					RESTORE_INT(flags);
				SMP_UNLOCK(flags);
				return;
			}
		} else {
			skb = dev_alloc_skb(len);
		}

		if (skb != NULL) {
			DECLARE_TXINSN(txinsn);

			skb->dev = priv->dev;
			skb_put(skb, len);

			pethhdr = (struct wlan_ethhdr_t *)(skb->data);
			memcpy((void *)pethhdr->daddr, pbuf, MACADDRLEN);
			memcpy((void *)pethhdr->saddr, BSSID, MACADDRLEN);
			pethhdr->type = htons(payloadlen);

			memset(skb->data+WLAN_ETHHDR_LEN, pattern, payloadlen);

			txinsn.q_num	= q_num; //using low queue for data queue
			txinsn.fr_type	= _SKB_FRAME_TYPE_;
			txinsn.pframe	= skb;
			skb->cb[1] = 0;

#ifdef MCAST2UI_REFINE
	        	memcpy(&skb->cb[10], skb->data, 6);
#endif
#ifdef ENABLE_RTL_SKB_STATS
			rtl_atomic_inc(&priv->rtl_tx_skb_cnt);
#endif
			txinsn.tx_rate = priv->pshare->mp_datarate;
			txinsn.fixed_rate = 1;
			txinsn.retry	= 0;
			txinsn.phdr		= get_wlanllchdr_from_poll(priv);
			if (NULL == txinsn.phdr)
				goto congestion_handle;

			memset((void *)txinsn.phdr, 0, sizeof(struct wlanllc_hdr));
			SetFrDs(txinsn.phdr);
			SetFrameType(txinsn.phdr, WIFI_DATA);

			if(rtl8192cd_firetx(priv, &txinsn) == CONGESTED) {
congestion_handle:
				//printk("Congested\n");
				i--;
				priv->pshare->mp_ctx_pkt--;
				if (txinsn.phdr)
					release_wlanllchdr_to_poll(priv, txinsn.phdr);

				if (background || tx_from_isr) {
					skb->tail = skb->data = skb->head;
					skb->len = 0;
					priv->pshare->skb_head = (priv->pshare->skb_head + NUM_MP_SKB - 1) & (NUM_MP_SKB - 1);
				} else if (skb) {
					rtl_kfree_skb(priv, skb, _SKB_TX_);
				}

				if (tx_from_isr) {
					head = get_txhead(phw, BE_QUEUE);
					tail = get_txtail(phw, BE_QUEUE);
					if (head == tail) // if Q empty,invoke 1s-timer to send
						OPMODE_VAL(OPMODE | (WIFI_MP_CTX_BACKGROUND | WIFI_MP_CTX_BACKGROUND_PENDING));
					SMP_UNLOCK(flags);
					return;
				} else {
					SAVE_INT_AND_CLI(flags2);
					rtl8192cd_tx_dsr((unsigned long)priv);
					RESTORE_INT(flags2);
				}
			}
			else if ((1 == priv->net_stats.tx_packets) &&
				((OPMODE & (WIFI_MP_CTX_PACKET| WIFI_MP_CTX_BACKGROUND| WIFI_MP_CTX_ST|
				WIFI_MP_CTX_CCK_CS))==WIFI_MP_CTX_BACKGROUND) )
			{
				#define CHECK_TX_MODE_CNT	40
				int check_cnt;

				// must sure RF is in TX mode before enabling TXAGC function.
				if ( priv->pshare->mp_antenna_tx & ANTENNA_A ) {
					check_cnt = 0;
					while ( (PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x00, 0x70000, 1) != 0x02) && (check_cnt < CHECK_TX_MODE_CNT) ) {
						delay_ms(1); ++check_cnt;
					}
					printk("RF(A) # of check tx mode = %d (%d)\n", check_cnt, tx_from_isr);
					PHY_SetBBReg(priv, 0x820, 0x400, 0x1);
				}

				if ( priv->pshare->mp_antenna_tx & ANTENNA_B ) {
					check_cnt = 0;
					while ( (PHY_QueryRFReg(priv, RF92CD_PATH_B, 0x00, 0x70000, 1) != 0x02) && (check_cnt < CHECK_TX_MODE_CNT) ) {
						delay_ms(1); ++check_cnt;
				}
					printk("RF(B) # of check tx mode = %d\n", check_cnt);
					PHY_SetBBReg(priv, 0x828, 0x400, 0x1);
				}
				/*
#if defined(CONFIG_WLAN_HAL_8813AE)
				if(CHECKICIS8813()){
					if ( priv->pshare->mp_antenna_tx & ANTENNA_C ) {
					check_cnt = 0;
					while ( (PHY_QueryRFReg(priv, RF92CD_PATH_C, 0x00, 0x70000, 1) != 0x02) && (check_cnt < CHECK_TX_MODE_CNT) ) {
						delay_ms(1); ++check_cnt;
					}
						printk("RF(C) # of check tx mode = %d\n", check_cnt);
						PHY_SetBBReg(priv, 0x828, 0x400, 0x1);  //8814??
					}
				}
#endif    */

				if ( background ) {
					RESTORE_INT(flags);
					SMP_UNLOCK(flags);
				} else if (tx_from_isr) {
					OPMODE_VAL(OPMODE & ~WIFI_MP_CTX_BACKGROUND_PENDING);
					SMP_UNLOCK(flags);
				}
				return;

				#undef CHECK_TX_MODE_CNT
			}
		}
		else
		{
			i--;
			priv->pshare->mp_ctx_pkt--;

			//printk("Can't allocate sk_buff\n");
			if (tx_from_isr) {
				head = get_txhead(phw, BE_QUEUE);
				tail = get_txtail(phw, BE_QUEUE);
				if (head == tail) // if Q empty,invoke 1s-timer to send
					OPMODE_VAL(OPMODE | (WIFI_MP_CTX_BACKGROUND | WIFI_MP_CTX_BACKGROUND_PENDING));
				SMP_UNLOCK(flags);
				return;
			}
			delay_ms(1);
			SAVE_INT_AND_CLI(flags2);
			rtl8192cd_tx_dsr((unsigned long)priv);
			RESTORE_INT(flags2);
		}

		if ((background || tx_from_isr) && (i == CURRENT_NUM_TX_DESC/4)) {
			OPMODE_VAL(OPMODE & ~WIFI_MP_CTX_BACKGROUND_PENDING);
			if (background)
				RESTORE_INT(flags);
			SMP_UNLOCK(flags);
			return;
		}
	}

	if (!tx_from_isr && !background) {
		RESTORE_UART0_INT();
#ifdef _MP_TELNET_SUPPORT_
		mp_pty_write_monitor(0);
#endif //_MP_TELNET_SUPPORT_
	}

stop_tx:

	RTL_W32(TCR, orgTCR);
/*
// turn on ADC
	RTL_W32(0xe70, (RTL_R32(0xe70) | 0x01e00000) );
	delay_us(100);
*/

	priv->pshare->mp_ctx_count = 0;
	priv->pshare->mp_ctx_pkt = 0;

	if (OPMODE & WIFI_MP_CTX_PACKET) {
		if (!(OPMODE & WIFI_MP_CTX_BACKGROUND))
			OPMODE_VAL(OPMODE & ~WIFI_MP_CTX_PACKET);
	} else {
		if (is_CCK_rate(priv->pshare->mp_datarate)) {
			if (OPMODE & WIFI_MP_CTX_CCK_CS) {
				OPMODE_VAL(OPMODE & ~WIFI_MP_CTX_CCK_CS);
				mpt_ProSetCarrierSupp(priv, FALSE);
			} else {
				mpt_StopCckCoNtTx(priv);
			}
		} else {
			mpt_StopOfdmContTx(priv);
			if (OPMODE & WIFI_MP_CTX_OFDM_HW)
				OPMODE_VAL(OPMODE & ~WIFI_MP_CTX_OFDM_HW);
			if (OPMODE & WIFI_MP_CTX_ST) {
				OPMODE_VAL(OPMODE & ~WIFI_MP_CTX_ST);

				if ((get_rf_mimo_mode(priv) == MIMO_1T2R) || (get_rf_mimo_mode(priv) == MIMO_1T1R)) {
					// eRFPath = RF90_PATH_C;
					eRFPath = RF92CD_PATH_A;
				} else if (get_rf_mimo_mode(priv) == MIMO_2T2R) {
					switch (priv->pshare->mp_antenna_tx) {
					case ANTENNA_B:
						eRFPath = RF92CD_PATH_B;
						break;
				// 8814 merge issue
					case ANTENNA_A:
					default:
						eRFPath = RF92CD_PATH_A;
						break;
					}
				}
				// Stop Single Tone.
				if (CHECKICIS8812() || CHECKICIS8881A() || CHECKICIS8814() || CHECKICIS8822()) {
					PHY_SetBBReg(priv, 0x808, BIT(28), 0x1);
					PHY_SetBBReg(priv, 0x808, BIT(29), 0x1);

					PHY_SetRFReg(priv, eRFPath, LNA_Low_Gain_3, BIT(1), 0);
					delay_us(100);
					PHY_SetRFReg(priv, eRFPath, 0x00, bRFRegOffsetMask, 0x30000); // PAD all on.
					delay_us(100);

					if (eRFPath == RF92CD_PATH_A) {
						PHY_SetBBReg(priv, 0xc00, BIT(1)|BIT(0), 0x3);
						RTL_W32(0xcb0, priv->pshare->RegCB0);
						PHY_SetBBReg(priv, 0xcb4, BIT(21), 0);
				 	if (CHECKICIS8822()){
						RTL_W32(0xcb4, priv->pshare->RegCB4);
						PHY_SetBBReg(priv, 0xcbc, BIT(1)|BIT(0), 0);
						PHY_SetBBReg(priv, 0xcbc, BIT(3), 0);
					}
					} else {
						PHY_SetBBReg(priv, 0xe00, BIT(1)|BIT(0), 0x3);
						RTL_W32(0xeb0, priv->pshare->RegEB0);
						PHY_SetBBReg(priv, 0xeb4, BIT(21), 0);
						if (CHECKICIS8822()){
							RTL_W32(0xeb4, priv->pshare->RegEB4);
							PHY_SetBBReg(priv, 0xebc, BIT(5)|BIT(4), 0);
							PHY_SetBBReg(priv, 0xebc, BIT(11), 0);
						}
					}
				} else
				{


	if (GET_CHIP_VER(priv)==VERSION_8197F){

		PHY_SetBBReg(priv, 0x4c, BIT(23), 0x0);
		PHY_SetBBReg(priv, 0x4c, BIT(26), 0x0);
		PHY_SetBBReg(priv, 0x64, BIT(7), 0x0);
		PHY_SetBBReg(priv, 0x64, BIT(1), 0x0);
		PHY_SetBBReg(priv, 0x64, BIT(0), 0x0);
        PHY_SetBBReg(priv, 0x78, BIT(16), 0x0);

		PHY_SetBBReg(priv, 0x88c, BIT(23)|BIT(22)|BIT(21)|BIT(20), 0x0);
		PHY_SetRFReg(priv, eRFPath, 0x57, BIT(1) ,0x0);
		PHY_SetRFReg(priv, eRFPath, 0x0, bMask20Bits ,0x33da5);

	}



				{
					PHY_SetBBReg(priv, rFPGA0_RFMOD, bCCKEn, 0x1);
					PHY_SetBBReg(priv, rFPGA0_RFMOD, bOFDMEn, 0x1);
					PHY_SetRFReg(priv, RF92CD_PATH_A, 0x21, bMask20Bits, 0x54000);
				}
				delay_us(100);

				if (GET_CHIP_VER(priv)!=VERSION_8197F)
					PHY_SetRFReg(priv, eRFPath, 0x00, bMask20Bits, 0x30000); // PAD all on.

				delay_us(100);

#ifdef HIGH_POWER_EXT_PA
				if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
				if(priv->pshare->rf_ft_var.use_ext_pa) {
					PHY_SetBBReg(priv, 0x870, BIT(10), 0x0);
					PHY_SetBBReg(priv, 0x870, BIT(26), 0x0);
				}
				}
#endif
			}
			}

			if (OPMODE & WIFI_MP_CTX_SCR) {
				OPMODE_VAL(OPMODE & ~WIFI_MP_CTX_SCR);

				//Turn off all test modes.
                if (CHECKICIS8812() || CHECKICIS8881A() || CHECKICIS8814() || CHECKICIS8822()) {
					PHY_SetBBReg(priv, 0x914, BIT(16), bDisable);	// continue tx
					PHY_SetBBReg(priv, 0x914, BIT(17), bDisable);		// single carrier tx
					PHY_SetBBReg(priv, 0x914, BIT(18), bDisable);		// single tone tx
					PHY_SetBBReg(priv, 0x914, 0xffff, 0);
				} else
				{
					PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMContinueTx, bDisable);
					PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleCarrier, bDisable);
					PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMSingleTone, bDisable);
					PHY_SetBBReg(priv, rOFDM1_LSTF, bOFDMDisPwsavTx, bDisable);

					PHY_SetBBReg(priv, rOFDM1_TRxMesaure1, 0xfff, 0);
				}
				//Delay 10 ms
				delay_ms(10);
				//BB Reset
/*
				PHY_SetBBReg(priv, rPMAC_Reset, bBBResetB, 0x0);
				PHY_SetBBReg(priv, rPMAC_Reset, bBBResetB, 0x1);
*/
			}
		}

		PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter1, bContTxHSSI, 0);
	}

	if (OPMODE & WIFI_MP_CTX_BACKGROUND) {

		printk("Stop continuous TX\n");

		SAVE_INT_AND_CLI(flags);
		SMP_LOCK(flags);
		OPMODE_VAL(OPMODE | WIFI_MP_CTX_BACKGROUND_STOPPING);
		OPMODE_VAL(OPMODE & ~WIFI_MP_CTX_BACKGROUND_PENDING);
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);

		while (priv->pshare->skb_head != priv->pshare->skb_tail) {
			DEBUG_INFO("[%s %d] skb_head/skb_tail=%d/%d, head/tail=%d/%d\n",
				__FUNCTION__, __LINE__, priv->pshare->skb_head, priv->pshare->skb_tail,
				get_txhead(phw, BE_QUEUE), get_txtail(phw, BE_QUEUE));

			rtl8192cd_tx_dsr((unsigned long)priv);
			delay_us(50);
		}

		SAVE_INT_AND_CLI(flags);
		SMP_LOCK(flags);
		OPMODE_VAL(OPMODE & ~(WIFI_MP_CTX_BACKGROUND | WIFI_MP_CTX_BACKGROUND_PENDING | WIFI_MP_CTX_PACKET |
				WIFI_MP_CTX_ST | WIFI_MP_CTX_SCR | WIFI_MP_CTX_CCK_CS |WIFI_MP_CTX_OFDM_HW |
				WIFI_MP_CTX_BACKGROUND_STOPPING));
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);

		for (i=0; i<NUM_MP_SKB; i++)
			kfree(priv->pshare->skb_pool[i]->head);
		kfree(priv->pshare->skb_pool_ptr);
	}
}

/*
  * MP pseudo MAC Tx, so-called hardware Tx
  */
#if (IC_LEVEL >= LEVEL_8814) || (IC_LEVEL == LEVEL_92E)
void mp_set_tmac_tx(struct rtl8192cd_priv *priv)
{
	if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B)){
	if(PHY_QueryBBReg(priv, 0xb00, bMaskDWord) & BIT8)
		PHY_SetBBReg(priv, 0xb00, BIT8, 0); /* Turn on TMAC */
	}else{ //N-series
		if(PHY_QueryBBReg(priv, 0xf00, bMaskDWord) & BIT10)
			PHY_SetBBReg(priv, 0xf00, BIT10, 0); /* Turn on TMAC */
	}

	if(!CHECKICIS97F() && (GET_CHIP_VER(priv) != VERSION_8192E)){
		if((PHY_QueryBBReg(priv, 0xA84, bMaskDWord) & BIT31) == 0)
			PHY_SetBBReg(priv, 0xA84, BIT31, 1); /* Turn on TMAC CCK */
	}
}

void mp_pmac_tx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	RT_PMAC_TX_INFO  PMacTxInfo;
	int iPMacTxInfo[sizeof(RT_PMAC_TX_INFO)+1], i, j, k=0;
	unsigned char sPMacTxInfo[sizeof(RT_PMAC_TX_INFO)+1], tmp[10];
	u4Byte u4bTmp, offset;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	DEBUG_INFO("\nstrlen(data)=%d\n",strlen(data));
	DEBUG_INFO("data=%s\n",data);
	i=0;
	while(i<strlen(data)){
		j=0;
		while((data[i]>='0' && data[i]<='9') || data[i]=='-'){ /* parse integer seperated by other symbols like ',' ,space, .etc */
			tmp[j++] = data[i++];
		}
		tmp[j]='\0';
		iPMacTxInfo[k++] = simple_strtol(tmp,NULL,10);		/* array to integer */
		sPMacTxInfo[k-1] = (unsigned char)iPMacTxInfo[k-1];
		i++;
	}
	tmp[j]='\0';
	iPMacTxInfo[k++] = simple_strtol(tmp,NULL,10);
	sPMacTxInfo[k-1] = (unsigned char)iPMacTxInfo[k-1];
	i++;

	memcpy(&PMacTxInfo,sPMacTxInfo,sizeof(RT_PMAC_TX_INFO));


	DEBUG_INFO("\nOrigin:\nPacketPeriod=%u\nPacketCount=%u\n",PMacTxInfo.PacketPeriod,PMacTxInfo.PacketCount);
	DEBUG_INFO("PacketLength=%u\nSFD=%u\n",PMacTxInfo.PacketLength,PMacTxInfo.SFD);
	DEBUG_INFO("LENGTH=%u\n",PMacTxInfo.LENGTH);


	PMacTxInfo.LENGTH = ntohs(PMacTxInfo.LENGTH);
	PMacTxInfo.SFD = ntohs(PMacTxInfo.SFD);
	PMacTxInfo.PacketPeriod = ntohs(PMacTxInfo.PacketPeriod);
	PMacTxInfo.PacketCount = ntohl(PMacTxInfo.PacketCount);
	PMacTxInfo.PacketLength = ntohl(PMacTxInfo.PacketLength);


	DEBUG_INFO("\nFinal:\nbEnPMacTx=%u\nMode=%u\nNtx=%u\n",PMacTxInfo.bEnPMacTx,PMacTxInfo.Mode,PMacTxInfo.Ntx);
	DEBUG_INFO("TX_RATE=%u\nTX_RATE_HEX=%u\nTX_SC=%u\n",PMacTxInfo.TX_RATE,PMacTxInfo.TX_RATE_HEX,PMacTxInfo.TX_SC);
	DEBUG_INFO("bSGI=%u\nbSPreamble=%u\nbSTBC=%u\n",PMacTxInfo.bSGI,PMacTxInfo.bSPreamble,PMacTxInfo.bSTBC);
	DEBUG_INFO("bLDPC=%u\nNDP_sound=%u\nBandWidth=%u\n",PMacTxInfo.bLDPC,PMacTxInfo.NDP_sound,PMacTxInfo.BandWidth);
	DEBUG_INFO("m_STBC=%u\nPacketPeriod=%u\nPacketCount=%u\n",PMacTxInfo.m_STBC,PMacTxInfo.PacketPeriod,PMacTxInfo.PacketCount);
	DEBUG_INFO("PacketLength=%u\nPacketPattern=%u\nSFD=%u\n",PMacTxInfo.PacketLength,PMacTxInfo.PacketPattern,PMacTxInfo.SFD);
	DEBUG_INFO("SignalField=%u\nServiceField=%u\nLENGTH=%u\n",PMacTxInfo.SignalField,PMacTxInfo.ServiceField,PMacTxInfo.LENGTH);
	for(i=0;i<2;i++){
		DEBUG_INFO("CRC16[%d]=%u\n",i,PMacTxInfo.CRC16[i]);
	}
	for(i=0;i<3;i++){
		DEBUG_INFO("LSIG[%d]=%u\n",i,PMacTxInfo.LSIG[i]);
	}
	for(i=0;i<6;i++){
		DEBUG_INFO("HT_SIG[%d]=%u\n",i,PMacTxInfo.HT_SIG[i]);
	}
	for(i=0;i<6;i++){
		DEBUG_INFO("VHT_SIG_A[%d]=%u\n",i,PMacTxInfo.VHT_SIG_A[i]);
	}
	for(i=0;i<6;i++){
		DEBUG_INFO("VHT_SIG_B[%d]=%u\n",i,PMacTxInfo.VHT_SIG_B[i]);
	}
	DEBUG_INFO("VHT_SIG_B_CRC=%u\n", PMacTxInfo.VHT_SIG_B_CRC);
	for(i=0;i<4;i++){
		DEBUG_INFO("VHT_Delimiter[%d]=%u\n",i,PMacTxInfo.VHT_Delimiter[i]);
	}
	for(i=0;i<6;i++){
		DEBUG_INFO("MacAddress[%d]=%u\n",i,PMacTxInfo.MacAddress[i]);
	}
	if((GET_CHIP_VER(priv) == VERSION_8197F)||(GET_CHIP_VER(priv) == VERSION_8192E)){
		PHY_SetBBReg(priv, 0xc04, 0xf0, 4); //Disable Rx antenna suggested by BB Yen-Cheng
	}
	if(PMacTxInfo.bEnPMacTx == FALSE){
		printk("Stop PMac Tx\n");
		panic_printk("[%s]bEnPMacTx = false\n",__FUNCTION__);
		if(PMacTxInfo.Mode == 3){ 		/* Continuous Tx */
			if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
			    PHY_SetBBReg(priv, 0xb04, 0xf, 2);		/*	TX Stop	*/
			else
			    PHY_SetBBReg(priv, 0xf04, 0xf, 2);		/*	TX Stop	*/
			if(is_CCK_rate(PMacTxInfo.TX_RATE))
				mpt_StopCckCoNtTx(priv);
			else
				mpt_StopOfdmContTx(priv);
		}
		else if(is_CCK_rate(PMacTxInfo.TX_RATE)){
			if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B)){
				PHY_SetBBReg(priv, 0xb04, 0xf, 2);		/*	STOP TX CCK */
				u4bTmp = PHY_QueryBBReg(priv, 0xf50, bMaskLWord);
				PHY_SetBBReg(priv, 0xb1c, bMaskLWord, u4bTmp+50);
				PHY_SetBBReg(priv, 0xb04, 0xf, 2);		/*	TX Stop */
			}else{
				PHY_SetBBReg(priv, 0xf04, 0xf, 2);		/*	STOP TX CCK */
				u4bTmp = PHY_QueryBBReg(priv, 0x9d0, bMaskHWord);
				PHY_SetBBReg(priv, 0xf18, bMaskHWord, u4bTmp+50);
				PHY_SetBBReg(priv, 0xf04, 0xf, 2);		/*	TX Stop */
			}
		}
		else{
			if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
				PHY_SetBBReg(priv, 0xb04, 0xf, 2); /* TX Stop*/
			else
				PHY_SetBBReg(priv, 0xf04, 0xf, 2); /* TX Stop*/
		}

		if(PMacTxInfo.Mode == 2){ 		/* Single Tone Tx */
			if(is_CCK_rate(PMacTxInfo.TX_RATE))   /* Stop HW TX -> Stop Continous TX -> Stop RF Setting*/
				mpt_StopCckCoNtTx(priv);
			else
				mpt_StopOfdmContTx(priv);

		}
		return;
	}

	printk("Start PMac Tx\n");
	if(PMacTxInfo.Mode == 3){	 /* Continuous Tx */
		PMacTxInfo.PacketCount = 1;
		if(is_CCK_rate(PMacTxInfo.TX_RATE))
			mpt_StartCckContTx(priv);
		else
			mpt_StartOfdmContTx(priv);
	}
	else if(PMacTxInfo.Mode == 2){	/* Single Tone Tx */
		PMacTxInfo.PacketCount = 1;   /* Continous TX -> HW TX -> RF Setting*/
		if(is_CCK_rate(PMacTxInfo.TX_RATE))
			mpt_StartCckContTx(priv);
		else
			mpt_StartOfdmContTx(priv);
	}
	else if(PMacTxInfo.Mode == 0)  /*Packet Tx*/
	{
		if(is_CCK_rate(PMacTxInfo.TX_RATE) && PMacTxInfo.PacketCount == 0)
			PMacTxInfo.PacketCount = 0xffff;
	}

	if(is_CCK_rate(PMacTxInfo.TX_RATE))	{
		u4bTmp = PMacTxInfo.PacketCount |  (PMacTxInfo.SFD << 16 ); /*  0xb1c[0:15] TX packet count 0xb1C[31:16]	SFD;*/
		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
			PHY_SetBBReg(priv, 0xb1c, bMaskDWord, u4bTmp);
		else{
			PHY_SetBBReg(priv, 0xf40, 0xffff0000, PMacTxInfo.SFD);
			PHY_SetBBReg(priv, 0xf18, 0xffff0000, PMacTxInfo.PacketCount);
		}

		u4bTmp = PMacTxInfo.SignalField |  (PMacTxInfo.ServiceField << 8 ) |  (PMacTxInfo.LENGTH << 16 ); /* 0xb40 7:0 SIGNAL	15:8 SERVICE	31:16 LENGTH*/
		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
			PHY_SetBBReg(priv, 0xb40, bMaskDWord, u4bTmp);
		else//97F
			PHY_SetBBReg(priv, 0xf44, bMaskDWord, u4bTmp);

		u4bTmp = PMacTxInfo.CRC16[0] |  (PMacTxInfo.CRC16[1] << 8 );
		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
			PHY_SetBBReg(priv, 0xb44, bMaskLWord, u4bTmp);
		else//97F
			PHY_SetBBReg(priv, 0xf48, bMaskLWord, u4bTmp);

		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B)){
			if(PMacTxInfo.bSPreamble)
				PHY_SetBBReg(priv, 0xb0c, BIT27, 0);
			else
				PHY_SetBBReg(priv, 0xb0c, BIT27, 1);
		}else{//97F
			if(PMacTxInfo.bSPreamble)
				PHY_SetBBReg(priv, 0xf40, BIT0, 0);
			else
				PHY_SetBBReg(priv, 0xf40, BIT0, 1);
		}
	}
	else{
		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
			PHY_SetBBReg(priv, 0xb18, 0xfffff, PMacTxInfo.PacketCount);
		else{//97F
			PHY_SetBBReg(priv, 0xf14, 0x00f00000, PMacTxInfo.PacketCount >> 16); // OFDM tx packet counter
			PHY_SetBBReg(priv, 0xf18, 0x0000ffff, PMacTxInfo.PacketCount & 0xffff); // OFDM tx packet counter
		}
		u4bTmp = PMacTxInfo.LSIG[0] | ((PMacTxInfo.LSIG[1]) << 8 ) | ((PMacTxInfo.LSIG[2]) << 16 ) | ((PMacTxInfo.PacketPattern) << 24 );

		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
			PHY_SetBBReg(priv, 0xb08, bMaskDWord, u4bTmp);	/*	Set 0xb08[23:0] = LSIG, 0xb08[31:24] =  Data init octet*/
		else//97F
			PHY_SetBBReg(priv, 0xf08, bMaskDWord, u4bTmp);	/*	Set 0xb08[23:0] = LSIG */

		if(PMacTxInfo.PacketPattern == 0x12)
			u4bTmp = 0x3000000;
		else
			u4bTmp = 0;
	}

	if(is_HT_rate(PMacTxInfo.TX_RATE)){
		u4bTmp |= PMacTxInfo.HT_SIG[0] | ((PMacTxInfo.HT_SIG[1]) << 8 ) | ((PMacTxInfo.HT_SIG[2]) << 16 );

		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
		    PHY_SetBBReg(priv, 0xb0c, bMaskDWord, u4bTmp);
		else//97F
		    PHY_SetBBReg(priv, 0xf0c, bMaskDWord, u4bTmp);

		u4bTmp = PMacTxInfo.HT_SIG[3] | ((PMacTxInfo.HT_SIG[4]) << 8 ) | ((PMacTxInfo.HT_SIG[5]) << 16 );

		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
		    PHY_SetBBReg(priv, 0xb10, 0xffffff, u4bTmp);
		else//97F
		    PHY_SetBBReg(priv, 0xf10, 0xffffff, u4bTmp);
	}
#ifdef RTK_AC_SUPPORT
	else if(is_VHT_rate(PMacTxInfo.TX_RATE)){
		u4bTmp |= PMacTxInfo.VHT_SIG_A[0] | ((PMacTxInfo.VHT_SIG_A[1]) << 8 ) | ((PMacTxInfo.VHT_SIG_A[2]) << 16 );
		PHY_SetBBReg(priv, 0xb0c, bMaskDWord, u4bTmp);
		u4bTmp = PMacTxInfo.VHT_SIG_A[3] | ((PMacTxInfo.VHT_SIG_A[4]) << 8 ) | ((PMacTxInfo.VHT_SIG_A[5]) << 16 );
		PHY_SetBBReg(priv, 0xb10, 0xffffff, u4bTmp);

		//memcpy(&u4bTmp, PMacTxInfo.VHT_SIG_B, 4);
		//PHY_SetBBReg(priv, 0xb14, bMaskDWord, u4bTmp);
		RTL_W8(0xb14,PMacTxInfo.VHT_SIG_B[0]);
		RTL_W8(0xb15,PMacTxInfo.VHT_SIG_B[1]);
		RTL_W8(0xb16,PMacTxInfo.VHT_SIG_B[2]);
		RTL_W8(0xb17,PMacTxInfo.VHT_SIG_B[3]);
	}
#endif
#ifdef RTK_AC_SUPPORT
	if(is_VHT_rate(PMacTxInfo.TX_RATE))	{
		u4bTmp = (PMacTxInfo.VHT_SIG_B_CRC << 24) | PMacTxInfo.PacketPeriod;	/* for TX interval*/
		PHY_SetBBReg(priv, 0xb20, bMaskDWord, u4bTmp);

		//memcpy(&u4bTmp, PMacTxInfo.VHT_Delimiter, 4);
		//PHY_SetBBReg(priv, 0xb24, bMaskDWord, u4bTmp);
		RTL_W8(0xb24,PMacTxInfo.VHT_Delimiter[0]);
		RTL_W8(0xb25,PMacTxInfo.VHT_Delimiter[1]);
		RTL_W8(0xb26,PMacTxInfo.VHT_Delimiter[2]);
		RTL_W8(0xb27,PMacTxInfo.VHT_Delimiter[3]);

		PHY_SetBBReg(priv, 0xb28, bMaskDWord, 0x00000040);	/* 0xb28 - 0xb34 24 byte Probe Request MAC Header*/
															/*  & Duration & Frame control		*/
		u4bTmp = PMacTxInfo.MacAddress[0] | (PMacTxInfo.MacAddress[1] << 8) | (PMacTxInfo.MacAddress[2] << 16)  | (PMacTxInfo.MacAddress[3] << 24); // Address1 [0:3]
		PHY_SetBBReg(priv, 0xb2C, bMaskDWord, u4bTmp);
		PHY_SetBBReg(priv, 0xb38, bMaskDWord, u4bTmp);		/* Address3 [3:0]	*/

		//u4bTmp =PMacTxInfo.MacAddress[4] | (PMacTxInfo.MacAddress[5] << 8) | (Adapter->CurrentAddress[0] << 16)  | (Adapter->CurrentAddress[1] << 24);
		u4bTmp =PMacTxInfo.MacAddress[4] | (PMacTxInfo.MacAddress[5] << 8) | (0xff << 16)  | (0xff << 24); /* Address2[0:1] & Address1 [5:4]*/
		PHY_SetBBReg(priv, 0xb30, bMaskDWord, u4bTmp);

		//u4bTmp = Adapter->CurrentAddress[2] | (Adapter->CurrentAddress[3] << 8) | (Adapter->CurrentAddress[4] << 16)  | (Adapter->CurrentAddress[5] << 24);
		u4bTmp = 0xff | (0xff << 8) | (0xff << 16)  | (0xff << 24);	/* Address2 [5:2]*/
		PHY_SetBBReg(priv, 0xb34, bMaskDWord, u4bTmp);

		/* Sequence Control & Address3 [5:4]*/
		//u4bTmp = PMacTxInfo.MacAddress[4] | (PMacTxInfo.MacAddress[5] << 8) ;
		//PHY_SetBBReg(priv, 0xb38, bMaskDWord, u4bTmp);
	}
	else
#endif
	{
		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B)){
			PHY_SetBBReg(priv, 0xb20, bMaskDWord, PMacTxInfo.PacketPeriod);	/* for TX interval*/
			PHY_SetBBReg(priv, 0xb24, bMaskDWord, 0x00000040);/*  & Duration & Frame control	*/
		} //97F
		else{
			PHY_SetBBReg(priv, 0xf1c, bMaskDWord, PMacTxInfo.PacketPeriod);	/* for TX interval*/
			PHY_SetBBReg(priv, 0xf20, bMaskDWord, 0x00000040);/*  & Duration & Frame control	*/
		}

		/* 0xb24 - 0xb38 24 byte Probe Request MAC Header*/
		u4bTmp = PMacTxInfo.MacAddress[0] | (PMacTxInfo.MacAddress[1] << 8) | (PMacTxInfo.MacAddress[2] << 16)  | (PMacTxInfo.MacAddress[3] << 24);
		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B)){
			PHY_SetBBReg(priv, 0xb28, bMaskDWord, u4bTmp);	/* Address1 [0:3]*/
			PHY_SetBBReg(priv, 0xb34, bMaskDWord, u4bTmp);	/* Address3 [3:0]*/
		}else{//97F
			PHY_SetBBReg(priv, 0xf24, bMaskDWord, u4bTmp);	/* Address1 [0:3]*/
			PHY_SetBBReg(priv, 0xf30, bMaskDWord, u4bTmp);	/* Address3 [3:0]*/
		}

		//u4bTmp =PMacTxInfo.MacAddress[4] | (PMacTxInfo.MacAddress[5] << 8) | (Adapter->CurrentAddress[0] << 16)  | (Adapter->CurrentAddress[1] << 24);
		u4bTmp =PMacTxInfo.MacAddress[4] | (PMacTxInfo.MacAddress[5] << 8) | (0xff << 16)  | (0xff << 24); /* Address2[0:1] & Address1 [5:4]*/
		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
			PHY_SetBBReg(priv, 0xb2c, bMaskDWord, u4bTmp);
		else//97F
			PHY_SetBBReg(priv, 0xf28, bMaskDWord, u4bTmp);

		//u4bTmp = Adapter->CurrentAddress[2] | (Adapter->CurrentAddress[3] << 8) | (Adapter->CurrentAddress[4] << 16)  | (Adapter->CurrentAddress[5] << 24);
		u4bTmp = 0xff | (0xff << 8) | (0xff << 16)  | (0xff << 24);	/* Address2 [5:2]*/
		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
			PHY_SetBBReg(priv, 0xb30, bMaskDWord, u4bTmp);
		else//97F
			PHY_SetBBReg(priv, 0xf2c, bMaskDWord, u4bTmp);

		u4bTmp = PMacTxInfo.MacAddress[4] | (PMacTxInfo.MacAddress[5] << 8) ;/* Sequence Control & Address3 [5:4]*/
		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
			PHY_SetBBReg(priv, 0xb38, bMaskDWord, u4bTmp);
		else//97F
			PHY_SetBBReg(priv, 0xf34, bMaskDWord, u4bTmp);
	}

	if((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
		PHY_SetBBReg(priv, 0xb48, bMaskByte3, PMacTxInfo.TX_RATE_HEX);
	else//97F
		PHY_SetBBReg(priv, 0xf4c, bMaskByte3, PMacTxInfo.TX_RATE_HEX);

	u4bTmp = (PMacTxInfo.TX_SC) | ((PMacTxInfo.BandWidth) << 4) | ((PMacTxInfo.m_STBC  -1) << 6) | ((PMacTxInfo.NDP_sound) << 8);
	if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
		PHY_SetBBReg(priv, 0xb4c, 0x1ff, u4bTmp); /* 0xb4c 3:0 TXSC	5:4	BW	7:6 m_STBC	8 NDP_Sound*/
	else//97F
		PHY_SetBBReg(priv, 0xf50, 0x1ff, u4bTmp); /* 0xb4c 3:0 TXSC	5:4	BW	7:6 m_STBC	8 NDP_Sound*/

	if(CHECKICIS8814()||CHECKICIS8822())
		if(IS_HAL_TEST_CHIP(priv))
			offset = 0xb4c;
		else
			offset = 0xb44;

	if(is_OFDM_rate(PMacTxInfo.TX_RATE)){
		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
			PHY_SetBBReg(priv, offset, 0xc0000000, 0);
		else//97F
			PHY_SetBBReg(priv, 0xf38, BIT8, 0);
	}else if(is_HT_rate(PMacTxInfo.TX_RATE)){
		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
			PHY_SetBBReg(priv, offset, 0xc0000000, 1);
		else//97F
			PHY_SetBBReg(priv, 0xf38, BIT8, 1);
	}
#ifdef RTK_AC_SUPPORT
	else if(is_VHT_rate(PMacTxInfo.TX_RATE))
		PHY_SetBBReg(priv, offset, 0xc0000000, 2);
#endif

	if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
		PHY_SetBBReg(priv, 0xb00, BIT8, 1);		/*	Turn on PMAC*/
	else//97F
		PHY_SetBBReg(priv, 0xf00, BIT10, 1);		/*	Turn on PMAC*/
	//PHY_SetBBReg(priv, 0xb04, 0xf, 2);			/*	TX Stop*/
	if(is_CCK_rate(PMacTxInfo.TX_RATE)){
		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
			PHY_SetBBReg(priv, 0xb04, 0xf, 8);		/*	TX CCK ON	*/
		else//97F
			PHY_SetBBReg(priv, 0xf04, 0xf, 8);		/*	TX CCK ON	*/

		if(!CHECKICIS97F() || (GET_CHIP_VER(priv) != VERSION_8192E))
			PHY_SetBBReg(priv, 0xA84, BIT31, 0);
	}
	else{
		if ((GET_CHIP_VER(priv) == VERSION_8814A)||(GET_CHIP_VER(priv) == VERSION_8822B))
			PHY_SetBBReg(priv, 0xb04, 0xf, 4);		/*	TX Ofdm ON	*/
		else//97F
			PHY_SetBBReg(priv, 0xf04, 0xf, 4);		/*	TX Ofdm ON	*/
		}
	// TODO: Single Tone
   /*
	if(PMacTxInfo.Mode == OFDM_Single_Tone_TX)
		mpt_SetSingleTone_8814A(Adapter, TRUE, TRUE);
	*/
}
#endif


int mp_query_stats(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return 0;
	}

	val = get_value_by_token((char *)data, "weight=");
	if (val) {
		priv->pshare->mp_rssi_weight = _atoi(val, 10);
		if ((priv->pshare->mp_rssi_weight < 0) || (priv->pshare->mp_rssi_weight > 100)) {
			printk("Waring! rssi_weight should be 0~100\n");
			priv->pshare->mp_rssi_weight = 5;
		}
	}

	val = get_value_by_token((char *)data, "rssi");
	if (val) {
#ifdef _OUTSRC_COEXIST
		if (IS_OUTSRC_CHIP(priv))
#endif
		{
			odm_FalseAlarmCounterStatistics(ODMPTR);
			priv->pshare->mp_FA_cnt = ODMPTR->FalseAlmCnt.Cnt_all;
			priv->pshare->mp_CCA_cnt = ODMPTR->FalseAlmCnt.Cnt_CCA_all;
		}

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
		if (!IS_OUTSRC_CHIP(priv))
#endif
		{
			FA_statistic(priv);
			priv->pshare->mp_FA_cnt = priv->pshare->FA_total_cnt;
			priv->pshare->mp_CCA_cnt = priv->pshare->CCA_total_cnt;
		}
#endif
		sprintf(data, "%d%%  rx %s%d (ss %d %d )(FA %d)(CCA %d)\n",
				priv->pshare->mp_rssi,
				((priv->pshare->mp_rx_rate&0x80) && (priv->pshare->mp_rx_rate< 0x90))? "MCS" : "",
				((priv->pshare->mp_rx_rate&0x80)? priv->pshare->mp_rx_rate&0x7f : priv->pshare->mp_rx_rate/2),
				priv->pshare->mp_rf_info.mimorssi[0], priv->pshare->mp_rf_info.mimorssi[1],
				priv->pshare->mp_FA_cnt,
				priv->pshare->mp_CCA_cnt);
		return strlen(data)+1;
	}


	sprintf(data, "Tx OK:%d, Tx Fail:%d, Rx OK:%lu, CRC error:%lu",
		(int)(priv->net_stats.tx_packets-priv->net_stats.tx_errors),
		(int)priv->net_stats.tx_errors,
		priv->net_stats.rx_packets, priv->net_stats.rx_crc_errors);
	return strlen(data)+1;
}

#if (IC_LEVEL >= LEVEL_8814) || (IC_LEVEL == LEVEL_92E)
void mp_reset_rx_mac(struct rtl8192cd_priv *priv){

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if(!(OPMODE & WIFI_MP_STATE)){
		printk("Fail: not in MP mode\n");
		return;
	}

	printk("Reset MAC Rx counter\n");
	PHY_SetBBReg(priv, 0x664, BIT27, 0x1);
	PHY_SetBBReg(priv, 0x664, BIT27, 0x0);
}

void mp_reset_rx_phy(struct rtl8192cd_priv *priv){

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if(!(OPMODE & WIFI_MP_STATE)){
		printk("Fail: not in MP mode\n");
		return;
	}
	/* Jaguar AC series*/
	if(GET_CHIP_VER(priv)==VERSION_8814A || GET_CHIP_VER(priv)==VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8881A || GET_CHIP_VER(priv)==VERSION_8822B){
		PHY_SetBBReg(priv, 0xB58, BIT0, 0x1);
		PHY_SetBBReg(priv, 0xB58, BIT0, 0x0);
	}else{ /* N series */
		PHY_SetBBReg(priv, 0xF14, BIT16, 0x1);
		PHY_SetBBReg(priv, 0xF14, BIT16, 0x0);
		PHY_SetBBReg(priv, 0xD00, BIT27, 0x1);
		PHY_SetBBReg(priv, 0xD00, BIT27, 0x0);
		//PHY_SetBBReg(priv, 0xF00, BIT9, 0x0);
		//PHY_SetBBReg(priv, 0xF00, BIT9, 0x1);
	}
}


void mp_reset_rx_macphy(struct rtl8192cd_priv *priv){

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}
	mp_reset_rx_mac(priv);
	mp_reset_rx_phy(priv);
}
#endif

#if (IC_LEVEL >= LEVEL_8814)
void mp_IQCalibrate(struct rtl8192cd_priv *priv){

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	printk("===> PHY_IQCalibrate\n");
	PHY_IQCalibrate(priv);
	printk("===> PHY_IQCalibrate\n");
}

void mp_LCCalibrate(struct rtl8192cd_priv *priv){

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	printk("===> PHY_LCCalibrate\n");
	if(CHECKICIS8822())
		;
	else
	if(CHECKICIS97F())//wish
		;
	printk("<=== PHY_LCCalibrate\n");
}

void mp_cal_rx_mac(struct rtl8192cd_priv *priv){

	UINT64 mac_cck_ok=0, mac_ofdm_ok=0, mac_ht_ok=0, mac_vht_ok=0;
	UINT64 mac_cck_err=0, mac_ofdm_err=0, mac_ht_err=0, mac_vht_err=0;

	//printk("===> mp_cal_rx_mac\n");

	PHY_SetBBReg(priv, 0x664, BIT26|BIT28|BIT29|BIT30|BIT31, 0x0);		//0:  0000_00 0000_0000
	mac_ofdm_ok     = PHY_QueryBBReg(priv, 0x664, bMaskLWord);
	PHY_SetBBReg(priv, 0x664, BIT26|BIT28|BIT29|BIT30|BIT31, 0x04);	//1: 0010_00 0000_1000
	mac_ofdm_err    = PHY_QueryBBReg(priv, 0x664, bMaskLWord);

	PHY_SetBBReg(priv, 0x664, BIT26|BIT28|BIT29|BIT30|BIT31, 0x0C);	//3: 0011_00 0000_1100
	mac_cck_ok      = PHY_QueryBBReg(priv, 0x664, bMaskLWord);
	PHY_SetBBReg(priv, 0x664, BIT26|BIT28|BIT29|BIT30|BIT31, 0x10);	//4: 0000_10 0000_0010
	mac_cck_err     = PHY_QueryBBReg(priv, 0x664, bMaskLWord);
	PHY_SetBBReg(priv, 0x664, BIT26|BIT28|BIT29|BIT30|BIT31, 0x18);	//6: 0001_10 0000_0110
	mac_ht_ok       = PHY_QueryBBReg(priv, 0x664, bMaskLWord);
	PHY_SetBBReg(priv, 0x664, BIT26|BIT28|BIT29|BIT30|BIT31, 0x1C);	//7:  0011_10 0000_1110
	mac_ht_err      = PHY_QueryBBReg(priv, 0x664, bMaskLWord);

	PHY_SetBBReg(priv, 0x664, BIT26|BIT28|BIT29|BIT30|BIT31, 0x1);	//16: 1000_00 0010_0000
//	mac_vht_ok     = (pHalData->BandSet == BAND_ON_BOTH) ? 	PHY_QueryMacReg(priv, 0x664, bMaskLWord) : 0;
	if(GET_CHIP_VER(priv)==VERSION_8814A || GET_CHIP_VER(priv)==VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8881A)
		mac_vht_ok     = PHY_QueryBBReg(priv, 0x664, bMaskLWord);
	else
		mac_vht_ok     = 0;

	PHY_SetBBReg(priv, 0x664, BIT26|BIT28|BIT29|BIT30|BIT31, 0x5);	//17:  1010_00 0010_1000
	mac_vht_err    = PHY_QueryBBReg(priv, 0x664, bMaskLWord);


/*	if( IS_HARDWARE_TYPE_8192ES(pAdapter)
		|| IS_HARDWARE_TYPE_8723BS(pAdapter)
		|| IS_HARDWARE_TYPE_8821S(pAdapter)
		|| IS_HARDWARE_TYPE_8814A(pAdapter)
		|| IS_HARDWARE_TYPE_8821B(pAdapter)
		//Wait for 8822B USB/SDIO ready, hsiao_ho 2014.05.27
		//|| IS_HARDWARE_TYPE_8822B(pAdapter)
		)
	{*/
		priv->pshare->mp_rx_mac_ok = mac_cck_ok + mac_ofdm_ok + mac_ht_ok + mac_vht_ok;
		priv->pshare->mp_rx_mac_crc_err = mac_cck_err + mac_ofdm_err + mac_ht_err + mac_vht_err;
/*	}
	else
	{
		mpt_ProResetRxCounters_MAC(pAdapter);

		pMptCtx->MptRxMacOkCnt += mac_cck_ok + mac_ofdm_ok + mac_ht_ok + mac_vht_ok;
		pMptCtx->MptRxMacCrcErrCnt += mac_cck_err + mac_ofdm_err + mac_ht_err + mac_vht_err;
	}
*/
	//pMptCtx->MptIoValue = (ULONG)pMptCtx->MptRxMacOkCnt;
}


void mp_cal_rx_phy(struct rtl8192cd_priv *priv){

	UINT64		cck_ok=0, ofdm_ok=0, ht_ok=0, vht_ok=0;
	UINT64		cck_err=0, ofdm_err=0, ht_err=0, vht_err=0;

	/* Jaguar AC series*/
	if(GET_CHIP_VER(priv)==VERSION_8814A || GET_CHIP_VER(priv)==VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8881A){
		cck_ok      = PHY_QueryBBReg(priv, 0xF04, 0x3FFF);		// [13:0]
		ofdm_ok     = PHY_QueryBBReg(priv, 0xF14, 0x3FFF);		// [13:0]
		ht_ok       = PHY_QueryBBReg(priv, 0xF10, 0x3FFF);		// [13:0]
		vht_ok      = PHY_QueryBBReg(priv, 0xF0C, 0x3FFF);		// [13:0]

		cck_err     = PHY_QueryBBReg(priv, 0xF04, 0x3FFF0000); // [29:16]
		ofdm_err    = PHY_QueryBBReg(priv, 0xF14, 0x3FFF0000); // [29:16]
		ht_err      = PHY_QueryBBReg(priv, 0xF10, 0x3FFF0000); // [29:16]
		vht_err     = PHY_QueryBBReg(priv, 0xF0C, 0x3FFF0000); // [29:16]
	}else {
		cck_ok      = PHY_QueryBBReg(priv, 0xF88, bMaskDWord);
		ofdm_ok     = PHY_QueryBBReg(priv, 0xF94, bMaskLWord);
		ht_ok       = PHY_QueryBBReg(priv, 0xF90, bMaskLWord);
		vht_ok      = 0;

		cck_err     = PHY_QueryBBReg(priv, 0xF84, bMaskDWord);
		ofdm_err    = PHY_QueryBBReg(priv, 0xF94, bMaskHWord);
		ht_err      = PHY_QueryBBReg(priv, 0xF90, bMaskHWord);
		vht_err     = 0;
	}
/*
	if( IS_HARDWARE_TYPE_8192ES(pAdapter)
		|| IS_HARDWARE_TYPE_8723BS(pAdapter)
		|| IS_HARDWARE_TYPE_8821S(pAdapter)
		|| IS_HARDWARE_TYPE_8814A(pAdapter)
		|| IS_HARDWARE_TYPE_8821B(pAdapter)
		//Wait for 8822B USB/SDIO ready, hsiao_ho 2014.05.27
		//|| IS_HARDWARE_TYPE_8822B(pAdapter)
		)
	{*/
		priv->pshare->mp_rx_phy_ok = cck_ok + ofdm_ok + ht_ok + vht_ok;
		priv->pshare->mp_rx_phy_crc_err = cck_err + ofdm_err + ht_err + vht_err;
/*	}
	else
	{
		mpt_ProResetRxCounters_PHY(pAdapter);

		pMptCtx->MptRxPhyOkCnt += cck_ok + ofdm_ok + ht_ok + vht_ok;
		pMptCtx->MptRxPhyCrcErrCnt += cck_err + ofdm_err + ht_err + vht_err;
	}	*/

	//pMptCtx->MptIoValue = (ULONG)pMptCtx->MptRxPhyOkCnt;

}

int mp_query_rx_macphy(struct rtl8192cd_priv *priv, unsigned char *data){

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return 0;
	}

	mp_cal_rx_mac(priv);
	mp_cal_rx_phy(priv);

	sprintf(data,"Rx MAC OK:%llu, Rx MAC Error:%llu, Rx PHY OK:%llu, Rx PHY Error:%llu",
		priv->pshare->mp_rx_mac_ok, priv->pshare->mp_rx_mac_crc_err,
		priv->pshare->mp_rx_phy_ok, priv->pshare->mp_rx_phy_crc_err);

	return strlen(data)+1;
}


#endif
void mp_txpower_tracking(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	unsigned int target_ther = 0;
	int tmp_ther;
	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}

	val = get_value_by_token((char *)data, "stop");
	if (val) {
		if (priv->pshare->mp_txpwr_tracking==FALSE)
			return;
		priv->pshare->mp_txpwr_tracking = FALSE;
		printk("mp tx power tracking stop\n");
		return;
	}

	val = get_value_by_token((char *)data, "ther=");
	if (val)
		target_ther = _atoi(val, 10);
	else if (priv->pmib->dot11RFEntry.ther)
		target_ther = priv->pmib->dot11RFEntry.ther;
	target_ther &= 0xff;

	if (!target_ther) {
		printk("Fail: tx power tracking has no target thermal value\n");
		return;
	}


	{
		if ((target_ther < 0x05) || (target_ther > 0x3c)) {
			panic_printk("Tx Power Tracking: unreasonable target ther %d, disable power tracking\n", target_ther);
			target_ther = 0;
			return;
		}
	}

	if(priv->pmib->dot11RFEntry.ther && priv->pshare->ThermalValue)
		priv->pshare->ThermalValue += (target_ther - priv->pmib->dot11RFEntry.ther );

	priv->pmib->dot11RFEntry.ther = target_ther;

	priv->pshare->mp_txpwr_tracking = TRUE;
	printk("mp tx power tracking start, target value=%d\n", target_ther);
}


void mp_dig(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}

	if ((GET_CHIP_VER(priv) != VERSION_8192D) && (GET_CHIP_VER(priv) != VERSION_8812E) && (GET_CHIP_VER(priv) != VERSION_8814A) && (GET_CHIP_VER(priv) != VERSION_8822B)) {
		printk("Fail: %s() only support 92D/8812/8814/8822 !\n", __FUNCTION__);
		return;
	}


	val = get_value_by_token((char *)data, "on");
	if (val) {
		if (priv->pshare->mp_dig_on
		)
			return;

		priv->pshare->mp_dig_on = 1;

		priv->pshare->mp_dig_reg_backup = RTL_R8(0xc50);
		printk("mp dig on! backup IG: 0x%x\n", priv->pshare->mp_dig_reg_backup);

		ODM_MPT_DIG(ODMPTR);
	}

	val = get_value_by_token((char *)data, "off");
	if (val) {
		if (priv->pshare->mp_dig_on==0)
			return;

		priv->pshare->mp_dig_on = 0;
		ODM_CancelTimer(ODMPTR, &ODMPTR->MPT_DIGTimer);
		if (GET_CHIP_VER(priv) == VERSION_8814A) {
			RTL_W8(0xc50, priv->pshare->mp_dig_reg_backup);
			RTL_W8(0xe50, priv->pshare->mp_dig_reg_backup);
			RTL_W8(0x1850, priv->pshare->mp_dig_reg_backup);
			RTL_W8(0x1a50, priv->pshare->mp_dig_reg_backup);
		} else if ((GET_CHIP_VER(priv) == VERSION_8812E)||(GET_CHIP_VER(priv) == VERSION_8822B)) {
			RTL_W8(0xc50, priv->pshare->mp_dig_reg_backup);
			RTL_W8(0xe50, priv->pshare->mp_dig_reg_backup);
		} else if (GET_CHIP_VER(priv) == VERSION_8192D) {
			RTL_W8(0xc50, priv->pshare->mp_dig_reg_backup);
			RTL_W8(0xc58, priv->pshare->mp_dig_reg_backup);
		}

		printk("mp dig off\n");
		return;
	}
}


int mp_query_tssi(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int i=0, j=0, val32, tssi, tssi_total=0, tssi_reg, reg_backup;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return 0;
	}

	if (priv->pshare->mp_txpwr_tracking) {
		priv->pshare->mp_txpwr_tracking = FALSE;
		sprintf(data, "8651");
		return strlen(data)+1;
	}

	if (is_CCK_rate(priv->pshare->mp_datarate)) {
		reg_backup = RTL_R32(rFPGA0_AnalogParameter4);
		RTL_W32(rFPGA0_AnalogParameter4, (reg_backup & 0xfffff0ff));

		while (i < 5) {
			j++;
			delay_ms(10);
			val32 = PHY_QueryBBReg(priv, rCCK0_TRSSIReport, bMaskByte0);
			tssi = val32 & 0x7f;
			if (tssi > 10) {
				tssi_total += tssi;
				i++;
			}
			if (j > 20)
				break;
		}

		RTL_W32(rFPGA0_AnalogParameter4, reg_backup);

		if (i > 0)
			tssi = tssi_total / i;
		else
			tssi = 0;
	} else {
//		if (priv->pshare->mp_antenna_tx == ANTENNA_A)
			tssi_reg = rFPGA0_XAB_RFInterfaceRB;
//		else
//			tssi_reg = rFPGA0_XCD_RFInterfaceRB;

		reg_backup = PHY_QueryBBReg(priv, rOFDM0_TRxPathEnable, 0x0000000f);
		PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, 0x0000000f, 0x0000000f);

		PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(25), 1);

		while (i < 5) {
			delay_ms(5);
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(25), 0);
			val32 = PHY_QueryBBReg(priv, tssi_reg, bMaskDWord);
			PHY_SetBBReg(priv, rFPGA0_XAB_RFParameter, BIT(25), 1);
			tssi = ((val32 & 0x04000000) >> 20) |
				   ((val32 & 0x00600000) >> 17) |
				   ((val32 & 0x00000c00) >> 8)  |
				   ((val32 & 0x00000060) >> 5);
			if (tssi) {
				tssi_total += tssi;
				i++;
			}
		}

		PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, 0x0000000f, reg_backup);

		tssi = tssi_total / 5;
	}

	sprintf(data, "%d", tssi);
	return strlen(data)+1;
}


int mp_query_ther(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int ther=0;
	int tmp_ther;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return 0;
	}

	// enable power and trigger
	if ((GET_CHIP_VER(priv)==VERSION_8814A) || (GET_CHIP_VER(priv)==VERSION_8812E) || (GET_CHIP_VER(priv)==VERSION_8881A) || (GET_CHIP_VER(priv)==VERSION_8822B))
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x42, BIT(17), 0x1);
	else
	if ((GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8723B) || (GET_CHIP_VER(priv) == VERSION_8197F))
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x42, (BIT(17) | BIT(16)), 0x03);
	else
	if (CHECKICIS92D())
		PHY_SetRFReg(priv, RF92CD_PATH_A, RF_T_METER_92D, bMask20Bits, 0x30000);
	else
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x24, bMask20Bits, 0x60);

	// delay for 10ms
	delay_ms(10);

	// query rf reg 0x24[4:0], for thermal meter value
	if ((GET_CHIP_VER(priv)==VERSION_8814A) || (GET_CHIP_VER(priv)==VERSION_8812E) || (GET_CHIP_VER(priv)==VERSION_8881A) || (GET_CHIP_VER(priv)==VERSION_8822B))
		ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x42, 0xfc00, 1);
	else
	if ((GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8723B) || (GET_CHIP_VER(priv) == VERSION_8197F))
		ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x42, 0xfc00, 1);
	else
	if (CHECKICIS92D())
		ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, RF_T_METER_92D, 0xf800, 1);
	else
		ther = PHY_QueryRFReg(priv, RF92CD_PATH_A, 0x24, bMask20Bits, 1) & 0x01f;

#ifdef THER_TRIM
	if(GET_CHIP_VER(priv) == VERSION_8197F){
		if(priv->pshare->rf_ft_var.ther_trim_enable){
			phy_printk(" original ther=%u\n", ther);
			tmp_ther = ther + priv->pshare->rf_ft_var.ther_trim_val;
			if(tmp_ther < 0)
				ther = 0;
			else
				ther = tmp_ther;
			phy_printk(" ther=%u, ther_trim_val=%d\n", ther, priv->pshare->rf_ft_var.ther_trim_val);
		}
	}
#endif

	sprintf(data, "%d", ther);
	return strlen(data)+1;
}


int mp_query_rssi(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int rssi=0;
	char *val;
	int rf_path=0;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return 0;
	}

	val = get_value_by_token((char *)data, "path=");
	if (val) {
		rf_path = _atoi(val, 10);

	}

	rssi=priv->pshare->mimorssi[rf_path];
	//panic_printk("\npriv->pshare->mimorssi[%d] %d\n",rf_path,priv->pshare->mimorssi[rf_path]);

	sprintf(data, "%d", rssi);
	return strlen(data)+1;
}


#ifdef MP_PSD_SUPPORT
int mp_query_psd(struct rtl8192cd_priv *priv, unsigned char * data)
{
	char *val;
	unsigned int i, psd_pts=0, psd_start=0, psd_stop=0;
	int psd_data=0;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return 0;
	}

	if (strlen(data) == 0) { //default value
		psd_pts = 128;
		psd_start = 64;
		psd_stop = 128;

	}
	else
	{
		val = get_value_by_token((char *)data, "pts=");
		if (val) {
			psd_pts = _atoi(val,10);
		}else {
			psd_pts = 128;
		}

		val = get_value_by_token((char *)data, "start=");
		if (val) {
			psd_start=_atoi(val,10);
		}else {
			psd_start = 64;
		}

		val = get_value_by_token((char *)data, "stop=");
		if (val) {
			psd_stop=_atoi(val,10);
		}else {
			psd_stop = psd_pts;
		}
	}

	data[0] = '\0';

	i = psd_start;
	while( i < psd_stop) {

		if( i>= psd_pts) {
			psd_data = GetPSDData(priv,(i-psd_pts));
		}
		else {
			psd_data = GetPSDData(priv,i);
		}
		sprintf(data, "%s%x ",data, psd_data);
		i++;
	}

	panic_printk("\"read psd = %s\"\n", data);
	panic_printk("Length is %d\n",strlen(data));
	printk("read psd = %s\n", data);
	printk("Length is %d\n",strlen(data));
	delay_ms(500);

	return strlen(data)+1;
}
#endif

int mp_get_txpwr(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int pwrA=0,pwrB=0;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return 0;
	}

	switch (priv->pshare->mp_datarate){
	case 2:
		pwrA = PHY_QueryBBReg(priv, 0xe08, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x838, bMaskByte1);
		break;
	case 4:
		pwrA = PHY_QueryBBReg(priv, 0x86c, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x838, bMaskByte2);
		break;
	case 11:
		pwrA = PHY_QueryBBReg(priv, 0x86c, bMaskByte2);
		pwrB = PHY_QueryBBReg(priv, 0x838, bMaskByte3);
		break;
	case 22:
		pwrA = PHY_QueryBBReg(priv, 0x86c, bMaskByte3);
		pwrB = PHY_QueryBBReg(priv, 0x86c, bMaskByte0);
		break;
	case 12:
		pwrA = PHY_QueryBBReg(priv, 0xe00, bMaskByte0);
		pwrB = PHY_QueryBBReg(priv, 0x830, bMaskByte0);
		break;
	case 18:
		pwrA = PHY_QueryBBReg(priv, 0xe00, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x830, bMaskByte1);
		break;
	case 24:
		pwrA = PHY_QueryBBReg(priv, 0xe00, bMaskByte2);
		pwrB = PHY_QueryBBReg(priv, 0x830, bMaskByte2);
		break;
	case 36:
		pwrA = PHY_QueryBBReg(priv, 0xe00, bMaskByte3);
		pwrB = PHY_QueryBBReg(priv, 0x830, bMaskByte3);
		break;
	case 48:
		pwrA = PHY_QueryBBReg(priv, 0xe04, bMaskByte0);
		pwrB = PHY_QueryBBReg(priv, 0x834, bMaskByte0);
		break;
	case 72:
		pwrA = PHY_QueryBBReg(priv, 0xe04, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x834, bMaskByte1);
		break;
	case 96:
		pwrA = PHY_QueryBBReg(priv, 0xe04, bMaskByte2);
		pwrB = PHY_QueryBBReg(priv, 0x834, bMaskByte2);
		break;
	case 108:
		pwrA = PHY_QueryBBReg(priv, 0xe04, bMaskByte3);
		pwrB = PHY_QueryBBReg(priv, 0x834, bMaskByte3);
		break;
	case 128:
		pwrA = PHY_QueryBBReg(priv, 0xe10, bMaskByte0);
		pwrB = PHY_QueryBBReg(priv, 0x83c, bMaskByte0);
		break;
	case 129:
		pwrA = PHY_QueryBBReg(priv, 0xe10, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x83c, bMaskByte1);
		break;
	case 130:
		pwrA = PHY_QueryBBReg(priv, 0xe10, bMaskByte2);
		pwrB = PHY_QueryBBReg(priv, 0x83c, bMaskByte2);
		break;
	case 131:
		pwrA = PHY_QueryBBReg(priv, 0xe10, bMaskByte3);
		pwrB = PHY_QueryBBReg(priv, 0x83c, bMaskByte3);
		break;
	case 132:
		pwrA = PHY_QueryBBReg(priv, 0xe14, bMaskByte0);
		pwrB = PHY_QueryBBReg(priv, 0x848, bMaskByte0);
		break;
	case 133:
		pwrA = PHY_QueryBBReg(priv, 0xe14, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x848, bMaskByte1);
		break;
	case 134:
		pwrA = PHY_QueryBBReg(priv, 0xe14, bMaskByte2);
		pwrB = PHY_QueryBBReg(priv, 0x848, bMaskByte2);
		break;
	case 135:
		pwrA = PHY_QueryBBReg(priv, 0xe14, bMaskByte3);
		pwrB = PHY_QueryBBReg(priv, 0x848, bMaskByte3);
		break;
	case 136:
		pwrA = PHY_QueryBBReg(priv, 0xe18, bMaskByte0);
		pwrB = PHY_QueryBBReg(priv, 0x84c, bMaskByte0);
		break;
	case 137:
		pwrA = PHY_QueryBBReg(priv, 0xe18, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x84c, bMaskByte1);
		break;
	case 138:
		pwrA = PHY_QueryBBReg(priv, 0xe18, bMaskByte2);
		pwrB = PHY_QueryBBReg(priv, 0x84c, bMaskByte2);
		break;
	case 139:
		pwrA = PHY_QueryBBReg(priv, 0xe18, bMaskByte3);
		pwrB = PHY_QueryBBReg(priv, 0x84c, bMaskByte3);
		break;
	case 140:
		pwrA = PHY_QueryBBReg(priv, 0xe1c, bMaskByte0);
		pwrB = PHY_QueryBBReg(priv, 0x868, bMaskByte0);
		break;
	case 141:
		pwrA = PHY_QueryBBReg(priv, 0xe1c, bMaskByte1);
		pwrB = PHY_QueryBBReg(priv, 0x868, bMaskByte1);
		break;
	case 142:
		pwrA = PHY_QueryBBReg(priv, 0xe1c, bMaskByte2);
		pwrB = PHY_QueryBBReg(priv, 0x868, bMaskByte2);
		break;
	case 143:
		pwrA = PHY_QueryBBReg(priv, 0xe1c, bMaskByte3);
		pwrB = PHY_QueryBBReg(priv, 0x868, bMaskByte3);
		break;
	}

	sprintf(data, "%d %d", pwrA, pwrB);
	return strlen(data)+1;
}


#ifdef B2B_TEST
/* Do checksum and verification for configuration data */
static unsigned char byte_checksum(unsigned char *data, int len)
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	sum = ~sum + 1;
	return sum;
}

static int is_byte_checksum_ok(unsigned char *data, int len)
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	if (sum == 0)
		return 1;
	else
		return 0;
}


static void mp_init_sta(struct rtl8192cd_priv *priv,unsigned char *da_mac)
{
	struct stat_info *pstat;
	unsigned char *da;

	da = da_mac;
	// prepare station info
	if (memcmp(da, "\x0\x0\x0\x0\x0\x0", 6) && !IS_MCAST(da))
	{
		pstat = get_stainfo(priv, da);
		if (pstat == NULL)
		{
			pstat = alloc_stainfo(priv, da, -1);
			pstat->state = WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE;
			memcpy(pstat->bssrateset, AP_BSSRATE, AP_BSSRATE_LEN);
			pstat->bssratelen = AP_BSSRATE_LEN;
			pstat->expire_to = 30000;
			asoc_list_add(priv, pstat);
			cnt_assoc_num(priv, pstat, INCREASE, (char *)__FUNCTION__);
			if (QOS_ENABLE)
				pstat->QosEnabled = 1;
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
				pstat->ht_cap_len = priv->ht_cap_len;
				memcpy(&pstat->ht_cap_buf, &priv->ht_cap_buf, priv->ht_cap_len);
			}
			pstat->current_tx_rate = priv->pshare->mp_datarate;
			update_fwtbl_asoclst(priv, pstat);
//			add_update_RATid(priv, pstat);
		}
	}
}


/*
 * tx pakcet.
 *  command: "iwpriv wlanx mp_tx,da=xxx,time=n,count=n,len=n,retry=n,tofr=n,wait=n,delay=n,err=n"
 *		default: da=ffffffffffff, time=0,count=1000, len=1500, retry=6, tofr=0, wait=0, delay=0(ms), err=1
 *		note: if time is set, it will take time (in sec) rather count.
 *		     if "time=-1", tx will continue tx until ESC. If "err=1", display statistics when tx err.
 */
int mp_tx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned int orgTCR;
	unsigned char increaseIFS=0; // set to 1 to increase the inter frame spacing while in PER test
	unsigned char pbuf[6]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	int count=1000, payloadlen=1500, retry=6, tofr=0, wait=0, delay=0, time=0;
	int err=1;
	struct sk_buff *skb;
	struct wlan_ethhdr_t *pethhdr;
	int len, i, q_num, ret, resent;
	unsigned char pattern=0xab;
	char *val;
	struct rtl8192cd_hw *phw = GET_HW(priv);
	static int last_tx_err;
	unsigned long end_time=0;
	unsigned long flags;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return 0;
	}

	orgTCR = RTL_R32(TCR);

	// get da
	val = get_value_by_token((char *)data, "da=");
	if (val) {
		ret = get_array_val(pbuf, val, 12);
		if (ret != 6) {
			printk("Error da format\n");
			return 0;
		}
	}

	// get time
	val = get_value_by_token((char *)data, "time=");
	if (val) {
		if (!memcmp(val, "-1", 2))
			time = -1;
		else {
			time = _atoi(val, 10);
			time = time*HZ;	// in 10ms
		}
	}

	// get count
	val = get_value_by_token((char *)data, "count=");
	if (val) {
		count = _atoi(val, 10);
	}

	// get payload len
	val = get_value_by_token((char *)data, "len=");
	if (val) {
		payloadlen = _atoi(val, 10);
		if (payloadlen < 20) {
			printk("len should be greater than 20!\n");
			return 0;
		}
	}

	// get retry number
	val = get_value_by_token((char *)data, "retry=");
	if (val) {
		retry = _atoi(val, 10);
	}

	// get tofr
	val = get_value_by_token((char *)data, "tofr=");
	if (val) {
		tofr = _atoi(val, 10);
	}

	// get wait
	val = get_value_by_token((char *)data, "wait=");
	if (val) {
		wait = _atoi(val, 10);
	}

	// get err
	val = get_value_by_token((char *)data, "err=");
	if (val) {
		err = _atoi(val, 10);
	}

	len = payloadlen + WLAN_ETHHDR_LEN;
	q_num = BE_QUEUE;

	if (time)
		printk("Start TX DA=%02x%02x%02x%02x%02x%02x len=%d tofr=%d retry=%d wait=%s time=%ds",
			pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5],
			payloadlen, tofr, retry, (wait ? "yes" : "no"), ((time > 0) ? time/100 : -1));
	else
		printk("Start TX DA=%02x%02x%02x%02x%02x%02x count=%d len=%d tofr=%d retry=%d wait=%s",
			pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5],
			count, payloadlen, tofr, retry, (wait ? "yes" : "no"));

	printk(", press any key to escape.\n");

	{
		RTL_W32(HIMR, RTL_R32(HIMR) | HIMR_BEDOK);
	}

//	RTL_W32(_RCR_, _NO_ERLYRX_);
	RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_AB | RCR_AM | RCR_APM | RCR_AAP));

	if (increaseIFS) {
//		RTL_W32(_TCR_, RTL_R32(_TCR_) | _DISCW_);
		RTL_W16(TX_PTCL_CTRL, RTL_R16(TX_PTCL_CTRL) | DIS_CW);
	}

	memset(&priv->net_stats, 0, sizeof(struct net_device_stats));
	priv->ext_stats.tx_retrys=0;
	last_tx_err = 0;

	if (time > 0) {
		end_time = jiffies + time;
	}

	i = 0;
	resent = 0;

	DISABLE_UART0_INT();

	mp_init_sta(priv, &pbuf[0]);

	while (1)
	{
		if ( IS_KEYBRD_HIT())
			break;

		if (time) {
			if (time != -1) {
				if (jiffies > end_time)
					break;
			}
		}
		else {
			if (!resent && i >= count)
				break;
		}
		if (!resent)
			i++;

		skb = dev_alloc_skb(len);

		if (skb != NULL)
		{
			DECLARE_TXINSN(txinsn);

			skb->dev = priv->dev;
			skb_put(skb, len);

			pethhdr = (struct wlan_ethhdr_t *)(skb->data);
			memcpy((void *)pethhdr->daddr, pbuf, MACADDRLEN);
			memcpy((void *)pethhdr->saddr, BSSID, MACADDRLEN);
			pethhdr->type = htons(payloadlen);

			// construct tx patten
			memset(skb->data+WLAN_ETHHDR_LEN, pattern, payloadlen);

			memcpy(skb->data+WLAN_ETHHDR_LEN, MP_PACKET_HEADER, MP_PACKET_HEADER_LEN); // header
			memcpy(skb->data+WLAN_ETHHDR_LEN+12, &i, 4); // packet sequence
			skb->data[len-1] = byte_checksum(skb->data+WLAN_ETHHDR_LEN,	payloadlen-1); // checksum

			txinsn.q_num	= q_num; //using low queue for data queue
			txinsn.fr_type	= _SKB_FRAME_TYPE_;
			txinsn.pframe	= skb;
			txinsn.tx_rate = priv->pshare->mp_datarate;
			txinsn.fixed_rate = 1;
			txinsn.retry	= retry;
			txinsn.phdr		= get_wlanllchdr_from_poll(priv);
#ifdef ENABLE_RTL_SKB_STATS
			rtl_atomic_inc(&priv->rtl_tx_skb_cnt);
#endif
			if (NULL == txinsn.phdr)
				goto congestion_handle;
			skb->cb[1] = 0;

#ifdef MCAST2UI_REFINE
			memcpy(&skb->cb[10], skb->data, 6);
#endif

			memset((void *)txinsn.phdr, 0, sizeof(struct wlanllc_hdr));

			if (tofr & 2)
				SetToDs(txinsn.phdr);
			if (tofr & 1)
				SetFrDs(txinsn.phdr);

			SetFrameType(txinsn.phdr, WIFI_DATA);

			if (wait) {
				while (1) {
					volatile unsigned int head, tail;
					head = get_txhead(phw, BE_QUEUE);
					tail = get_txtail(phw, BE_QUEUE);
					if (head == tail)
						break;
					delay_ms(1);
				}
			}

			if(rtl8192cd_firetx(priv, &txinsn) == CONGESTED)
			{
congestion_handle:
				if (txinsn.phdr)
					release_wlanllchdr_to_poll(priv, txinsn.phdr);
				if (skb)
					rtl_kfree_skb(priv, skb, _SKB_TX_);

				//printk("CONGESTED : busy waiting...\n");
				delay_ms(1);
				resent = 1;

				SAVE_INT_AND_CLI(flags);
				rtl8192cd_tx_dsr((unsigned long)priv);
				RESTORE_INT(flags);
			}
			else {
				SAVE_INT_AND_CLI(flags);
				rtl8192cd_tx_dsr((unsigned long)priv);
				RESTORE_INT(flags);

				if (err && ((int)priv->net_stats.tx_errors) != last_tx_err) { // err happen
					printk("\tout=%d\tfail=%d\n", (int)priv->net_stats.tx_packets,
							(int)priv->net_stats.tx_errors);
					last_tx_err = (int)priv->net_stats.tx_errors;
				}
				else {
					if ( (i%10000) == 0 )
						printk("Tx status: ok=%d\tfail=%d\tretry=%ld\n", (int)(priv->net_stats.tx_packets-priv->net_stats.tx_errors),
							(int)priv->net_stats.tx_errors, priv->ext_stats.tx_retrys);
				}
				resent = 0;
			}
			if (delay)
				delay_ms(delay);
		}
		else
		{
			printk("Can't allocate sk_buff\n");
			delay_ms(1);
			resent = 1;

			SAVE_INT_AND_CLI(flags);
			rtl8192cd_tx_dsr((unsigned long)priv);
			RESTORE_INT(flags);
		}
	}

	RESTORE_UART0_INT();

	// wait till all tx is done
	printk("\nwaiting tx is finished...");
	i = 0;
	while (1) {
		volatile unsigned int head, tail;
		head = get_txhead(phw, BE_QUEUE);
		tail = get_txtail(phw, BE_QUEUE);
		if (head == tail)
			break;

		SAVE_INT_AND_CLI(flags);
		rtl8192cd_tx_dsr((unsigned long)priv);
		RESTORE_INT(flags);

		delay_ms(1);

		if (i++ >10000)
			break;
	}
	printk("done.\n");

	RTL_W32(TCR, orgTCR);

	sprintf(data, "Tx result: ok=%d,fail=%d", (int)(priv->net_stats.tx_packets-priv->net_stats.tx_errors),
			(int)priv->net_stats.tx_errors);
	return strlen(data)+1;
}


/*
 * validate rx packet. rx packet format:
 *	|wlan-header(24 byte)|MP_PACKET_HEADER (12 byte)|sequence(4 bytes)|....|checksum(1 byte)|
 *
 */
void mp_validate_rx_packet(struct rtl8192cd_priv *priv, unsigned char *data, int len)
{
	int tofr = get_tofr_ds(data);
	unsigned int type=GetFrameType(data);
	int header_size = 24;
	unsigned long sequence;
	unsigned short fr_seq;

	if (!priv->pshare->mp_rx_waiting)
		return;

	if (type != WIFI_DATA)
		return;

	fr_seq = GetTupleCache(data);
	if (GetRetry(data) && fr_seq == priv->pshare->mp_cached_seq) {
		priv->pshare->mp_rx_dup++;
		return;
	}

	if (tofr == 3)
		header_size = 30;

	if (len < (header_size+20) )
		return;

	// see if test header matched
	if (memcmp(&data[header_size], MP_PACKET_HEADER, MP_PACKET_HEADER_LEN))
		return;

	priv->pshare->mp_cached_seq = fr_seq;

	memcpy(&sequence, &data[header_size+MP_PACKET_HEADER_LEN], 4);

	if (!is_byte_checksum_ok(&data[header_size], len-header_size)) {
#if 0
		printk("mp_rx: checksum error!\n");
#endif
		printk("mp_brx: checksum error!\n");
	}
	else {
		if (sequence <= priv->pshare->mp_rx_sequence) {
#if 0
			printk("mp_rx: invalid sequece (%ld) <= current (%ld)!\n",
									sequence, priv->pshare->mp_rx_sequence);
#endif
			printk("mp_brx: invalid sequece (%ld) <= current (%ld)!\n",
									sequence, priv->pshare->mp_rx_sequence);
		}
		else {
			if (sequence > (priv->pshare->mp_rx_sequence+1))
				priv->pshare->mp_rx_lost_packet += (sequence-priv->pshare->mp_rx_sequence-1);
			priv->pshare->mp_rx_sequence = sequence;
			priv->pshare->mp_rx_ok++;
		}
	}
}


#if 0
/*
 * Rx test packet.
 *   command: "iwpriv wlanx mp_rx [ra=xxxxxx,quiet=t,interval=n]"
 *	           - ra: rx mac. defulat is burn-in mac
 *             - quiet_time: quit rx if no rx packet during quiet_time. default is 5s
 *             - interval: report rx statistics periodically in sec. default is 0 (no report)
 */
static void mp_rx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	unsigned char pbuf[6];
	int quiet_time=5, interval_time=0, quiet_period=0, interval_period=0, ret;
	unsigned int o_rx_ok, o_rx_lost_packet, mac_changed=0;
	unsigned long reg, counter=0;

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}

	// get ra
	val = get_value_by_token((char *)data, "ra=");
	if (val) {
		ret =0;
		if (strlen(val) >=12)
			ret = get_array_val(pbuf, val, 12);
		if (ret != 6) {
			printk("Error mac format\n");
			return;
		}
		printk("set ra to %02X:%02X:%02X:%02X:%02X:%02X\n",
				pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5]);

		memcpy(&reg, pbuf, 4);
		RTL_W32(_IDR0_, (cpu_to_le32(reg)));

		memcpy(&reg, pbuf+4, 4);
		RTL_W32(_IDR0_ + 4, (cpu_to_le32(reg)));
		mac_changed = 1;
	}

	// get quiet time
	val = get_value_by_token((char *)data, "quiet=");
	if (val)
		quiet_time = _atoi(val, 10);

	// get interval time
	val = get_value_by_token((char *)data, "interval=");
	if (val)
		interval_time = _atoi(val, 10);

	RTL_W32(_RCR_, _ENMARP_ | _APWRMGT_ | _AMF_ | _ADF_ | _NO_ERLYRX_ |
			_RX_DMA64_ | _ACRC32_ | _AB_ | _AM_ | _APM_ | _AAP_);

	priv->pshare->mp_cached_seq = 0;
	priv->pshare->mp_rx_ok = 0;
	priv->pshare->mp_rx_sequence = 0;
	priv->pshare->mp_rx_lost_packet = 0;
	priv->pshare->mp_rx_dup = 0;

	printk("Waiting for rx packet, quit if no packet in %d sec", quiet_time);

#if (defined(CONFIG_RTL_EB8186) && defined(__KERNEL__)) || defined(CONFIG_RTL865X)
	printk(", or press any key to escape.\n");
	DISABLE_UART0_INT();
#else
	printk(".\n");
#endif

	priv->pshare->mp_rx_waiting = 1;

	while (1) {
		// save old counter
		o_rx_ok = priv->pshare->mp_rx_ok;
		o_rx_lost_packet = priv->pshare->mp_rx_lost_packet;

#if (defined(CONFIG_RTL_EB8186) && defined(__KERNEL__)) || defined(CONFIG_RTL865X)
		if ( IS_KEYBRD_HIT())
			break;
#endif

		delay_ms(1000);

		if (interval_time && ++interval_period == interval_time) {
			printk("\tok=%ld\tlost=%ld\n", priv->pshare->mp_rx_ok, priv->pshare->mp_rx_lost_packet);
			interval_period=0;
		}
		else {
			if ((priv->pshare->mp_rx_ok-counter) > 10000) {
				printk("Rx status: ok=%ld\tlost=%ld, duplicate=%ld\n", priv->pshare->mp_rx_ok, priv->pshare->mp_rx_lost_packet, priv->pshare->mp_rx_dup);
				counter += 10000;
			}
		}

		if (o_rx_ok == priv->pshare->mp_rx_ok && o_rx_lost_packet == priv->pshare->mp_rx_lost_packet)
			quiet_period++;
		else
			quiet_period = 0;

		if (quiet_period >= quiet_time)
			break;
	}

//	printk("\nRx result: ok=%ld\tlost=%ld, duplicate=%ld\n\n", priv->pshare->mp_rx_ok, priv->pshare->mp_rx_lost_packet, priv->mp_rx_dup);
	printk("\nRx reseult: ok=%ld\tlost=%ld\n\n", priv->pshare->mp_rx_ok, priv->pshare->mp_rx_lost_packet);

	priv->pshare->mp_rx_waiting = 0;

	if (mac_changed) {
		memcpy(pbuf, priv->pmib->dot11OperationEntry.hwaddr, MACADDRLEN);

		memcpy(&reg, pbuf, 4);
		RTL_W32(_IDR0_, (cpu_to_le32(reg)));

		memcpy(&reg, pbuf+4, 4);
		RTL_W32(_IDR0_ + 4, (cpu_to_le32(reg)));
	}

#if (defined(CONFIG_RTL_EB8186) && defined(__KERNEL__)) || defined(CONFIG_RTL865X)
	RESTORE_UART0_INT();
#endif

}
#endif


/*
 * Rx test packet.
 *   command: "iwpriv wlanx mp_brx start[,ra=xxxxxx]"
 *	           - ra: rx mac. defulat is burn-in mac
 *   command: "iwpriv wlanx mp_brx stop"
 *               - stop rx immediately
 */
int mp_brx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	unsigned char pbuf[6];
	int ret;
	unsigned long reg;
	unsigned long	flags;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return 0;
	}

	SAVE_INT_AND_CLI(flags);

	if (!strcmp(data, "stop"))
		goto stop_brx;

	// get start
	val = get_value_by_token((char *)data, "start");
	if (val) {
		// get ra if it exists
		val = get_value_by_token((char *)data, "ra=");
		if (val) {
			ret =0;
			if (strlen(val) >=12)
				ret = get_array_val(pbuf, val, 12);
			if (ret != 6) {
				printk("Error mac format\n");
				RESTORE_INT(flags);
				return 0;
			}
			printk("set ra to %02X:%02X:%02X:%02X:%02X:%02X\n",
				pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5]);

			memcpy(&reg, pbuf, 4);
			RTL_W32(MACID, (cpu_to_le32(reg)));

			memcpy(&reg, pbuf+4, 4);
			RTL_W32(MACID + 4, (cpu_to_le32(reg)));
			priv->pshare->mp_mac_changed = 1;
		}
	}
	else {
		RESTORE_INT(flags);
		return 0;
	}

	priv->pshare->mp_cached_seq = 0;
	priv->pshare->mp_rx_ok = 0;
	priv->pshare->mp_rx_sequence = 0;
	priv->pshare->mp_rx_lost_packet = 0;
	priv->pshare->mp_rx_dup = 0;
	priv->pshare->mp_rx_waiting = 1;

	// record the start time of MP throughput test
	priv->pshare->txrx_start_time = jiffies;

	OPMODE_VAL(OPMODE | WIFI_MP_RX);
//	RTL_W32(RCR, _ENMARP_ | _APWRMGT_ | _AMF_ | _ADF_ | _NO_ERLYRX_ |
//			_RX_DMA64_ | _ACRC32_ | _AB_ | _AM_ | _APM_);
	RTL_W32(RCR, RCR_HTC_LOC_CTRL | RCR_AMF | RCR_ADF | RCR_ACRC32 | RCR_APWRMGT | RCR_AB | RCR_AM | RCR_APM);

	memset(&priv->net_stats, 0,  sizeof(struct net_device_stats));
	memset(&priv->ext_stats, 0,  sizeof(struct extra_stats));

	RESTORE_INT(flags);
	return 0;

stop_brx:
	OPMODE_VAL(OPMODE & ~WIFI_MP_RX);

//	RTL_W32(RCR, _ENMARP_ | _NO_ERLYRX_ | _RX_DMA64_);
	RTL_W32(RCR, RCR_HTC_LOC_CTRL);
	priv->pshare->mp_rx_waiting = 0;

	//record the elapsed time of MP throughput test
	priv->pshare->txrx_elapsed_time = jiffies - priv->pshare->txrx_start_time;

	if (priv->pshare->mp_mac_changed) {
		memcpy(pbuf, priv->pmib->dot11OperationEntry.hwaddr, MACADDRLEN);

		memcpy(&reg, pbuf, 4);
		RTL_W32(MACID, (cpu_to_le32(reg)));

		memcpy(&reg, pbuf+4, 4);
		RTL_W32(MACID + 4, (cpu_to_le32(reg)));
	}
	priv->pshare->mp_mac_changed = 0;

	RESTORE_INT(flags);

	sprintf(data, "Rx reseult: ok=%ld,lost=%ld,elapsed time=%ld",
		priv->pshare->mp_rx_ok, priv->pshare->mp_rx_lost_packet, priv->pshare->txrx_elapsed_time);
	return strlen(data)+1;
}
#endif // B2B_TEST


void mp_set_bandwidth(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	int bw=0, shortGI=0;
//	unsigned int regval, i, val32;
//	unsigned char *CCK_SwingEntry;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}

	// get 20M~40M , 40M=0(20M), 1(40M) or 2(80M)
	val = get_value_by_token((char *)data, "40M=");
	if (val) {
		bw = _atoi(val, 10);
	}

    // get shortGI=1 or 0.
	val = get_value_by_token((char *)data, "shortGI=");
	if (val) {
		shortGI = _atoi(val, 10);
	}

	// modify short GI
	if(shortGI) {
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M = 1;
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = 1;
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = 1;
		printk("shortGI = 1\n");
	} else {
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M = 0;
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = 0;
		priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = 0;
		printk("shortGI = 0\n");
	}

	// modify BW
	if (bw == 0) {
		priv->pshare->is_40m_bw = 0;
		priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20;
		priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;
		priv->pmib->dot11nConfigEntry.dot11nUse40M=0;
		printk("CurrentChannelBW: 20MHz\n");
	}
	else if (bw == 1) {
		priv->pshare->is_40m_bw = 1;
		priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20_40;
		priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
		priv->pmib->dot11nConfigEntry.dot11nUse40M=1;
		printk("CurrentChannelBW: 40MHz\n");
	}
	else if (bw == 2) {
		priv->pshare->is_40m_bw = 2;
		priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_80;
		priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
		priv->pmib->dot11nConfigEntry.dot11nUse40M=2;
		printk("CurrentChannelBW: 80MHz\n");
	}

	SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);


//	mp_8192CD_tx_setting(priv);
	return; // end here
}


/*
 * auto-rx
 * accept CRC32 error pkt
 * accept destination address pkt
 * drop tx pkt (implemented in other functions)
 */
#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
int mp_arx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return 0;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return 0;
	}
	if (OPMODE & WIFI_MP_CTX_BACKGROUND) {
		printk("Fail: In MP background mode, please stop and retry it again\n");
		return 0;
	}

	if (!strcmp(data, "start")) {
#if 0
#endif
		OPMODE_VAL(OPMODE | WIFI_MP_RX);

#if 0//defined(CONFIG_RTL_92D_SUPPORT)
		priv->pshare->mp_dig_on = 1;
		mod_timer(&priv->pshare->MP_DIGTimer, jiffies + RTL_MILISECONDS_TO_JIFFIES(700));
#endif
#ifdef MP_SWITCH_LNA
		priv->pshare->rx_packet_ss_a = 0;
		priv->pshare->rx_packet_ss_b = 0;
		PHY_SetBBReg(priv, 0xd00, BIT(27)|BIT(26), 0x3);
		RTL_W32(RCR, RCR_APWRMGT | RCR_AMF | RCR_ADF |RCR_ACRC32 |RCR_AB | RCR_AM | RCR_APM | RCR_AAP | RCR_APP_PHYSTS);
#else
		RTL_W32(RCR, RCR_APWRMGT | RCR_AMF | RCR_ADF |RCR_ACRC32 |RCR_AB | RCR_AM | RCR_APM | RCR_AAP | RCR_APP_PHYSTS);
#endif
		if (priv->pshare->rf_ft_var.use_frq_2_3G)
			PHY_SetBBReg(priv, rCCK0_System, bCCKEqualizer, 0x0);

		memset(&priv->net_stats, 0,  sizeof(struct net_device_stats));
		memset(&priv->ext_stats, 0,  sizeof(struct extra_stats));


	} else if (!strcmp(data, "stop")) {
		OPMODE_VAL(OPMODE & ~WIFI_MP_RX);
		/*	priv->pshare->mp_dig_on = 0;	*/
		if (IS_HAL_CHIP(priv))
			RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_APWRMGT | RCR_AMF | RCR_ADF |RCR_ACRC32 |RCR_AB | RCR_AM | RCR_APM | RCR_AAP | RCR_APP_PHYSTS));
		else if(CONFIG_WLAN_NOT_HAL_EXIST)
		RTL_W32(RCR, RTL_R32(RCR) & ~(RCR_AB | RCR_AM | RCR_APM | RCR_AAP | RCR_APP_PHYSTS));


		if (priv->pshare->rf_ft_var.use_frq_2_3G)
			PHY_SetBBReg(priv, rCCK0_System, bCCKEqualizer, 0x1);

//clear filter:
		if((priv->pshare->mp_filter_flag) || (OPMODE & WIFI_MP_ARX_FILTER)) {
			OPMODE_VAL(OPMODE & ~WIFI_MP_ARX_FILTER);
			priv->pshare->mp_filter_flag=0;
			memset(priv->pshare->mp_filter_DA,0,6);
			memset(priv->pshare->mp_filter_SA,0,6);
			memset(priv->pshare->mp_filter_BSSID,0,6);
		}

		sprintf(data, "Received packet OK:%lu  CRC error:%lu\n", priv->net_stats.rx_packets, priv->net_stats.rx_crc_errors);
		return strlen(data)+1;
	} else if (!memcmp(data, "filter_SA",9)) {
		OPMODE_VAL(OPMODE | WIFI_MP_ARX_FILTER);
		memset(priv->pshare->mp_filter_SA,'\0',6);

		priv->pshare->mp_filter_flag |=0x1;
		val = get_value_by_token((char *)data, "filter_SA=");
		val[2]='\0';
		priv->pshare->mp_filter_SA[0]=_atoi(val,16);
		val[5]='\0';
		priv->pshare->mp_filter_SA[1]=_atoi(val+3,16);
		val[8]='\0';
		priv->pshare->mp_filter_SA[2]=_atoi(val+6,16);
		val[11]='\0';
		priv->pshare->mp_filter_SA[3]=_atoi(val+9,16);
		val[14]='\0';
		priv->pshare->mp_filter_SA[4]=_atoi(val+12,16);
		val[17]='\0';
		priv->pshare->mp_filter_SA[5]=_atoi(val+15,16);
		sprintf(data,"flag: %x\nSA: %02x:%02x:%02x:%02x:%02x:%02x\n",priv->pshare->mp_filter_flag,
															priv->pshare->mp_filter_SA[0],
															priv->pshare->mp_filter_SA[1],
															priv->pshare->mp_filter_SA[2],
															priv->pshare->mp_filter_SA[3],
															priv->pshare->mp_filter_SA[4],
															priv->pshare->mp_filter_SA[5]);
		return strlen(data)+1;

	} else if (!memcmp(data, "filter_DA",9)) {
		OPMODE_VAL(OPMODE | WIFI_MP_ARX_FILTER);
		priv->pshare->mp_filter_flag |=0x2;
		memset(priv->pshare->mp_filter_DA,'\0',6);

		val = get_value_by_token((char *)data, "filter_DA=");
		val[2]='\0';
		priv->pshare->mp_filter_DA[0]=_atoi(val,16);
		val[5]='\0';
		priv->pshare->mp_filter_DA[1]=_atoi(val+3,16);
		val[8]='\0';
		priv->pshare->mp_filter_DA[2]=_atoi(val+6,16);
		val[11]='\0';
		priv->pshare->mp_filter_DA[3]=_atoi(val+9,16);
		val[14]='\0';
		priv->pshare->mp_filter_DA[4]=_atoi(val+12,16);
		val[17]='\0';
		priv->pshare->mp_filter_DA[5]=_atoi(val+15,16);
		sprintf(data,"flag: %x\nDA: %02x:%02x:%02x:%02x:%02x:%02x\n",priv->pshare->mp_filter_flag,
															priv->pshare->mp_filter_DA[0],
															priv->pshare->mp_filter_DA[1],
															priv->pshare->mp_filter_DA[2],
															priv->pshare->mp_filter_DA[3],
															priv->pshare->mp_filter_DA[4],
															priv->pshare->mp_filter_DA[5]);
		return strlen(data)+1;
	} else if (!memcmp(data, "filter_BSSID",12)) {
		OPMODE_VAL(OPMODE | WIFI_MP_ARX_FILTER);
		priv->pshare->mp_filter_flag |=0x4;
		memset(priv->pshare->mp_filter_BSSID,'\0',6);

		val = get_value_by_token((char *)data, "filter_BSSID=");
		val[2]='\0';
		priv->pshare->mp_filter_BSSID[0]=_atoi(val,16);
		val[5]='\0';
		priv->pshare->mp_filter_BSSID[1]=_atoi(val+3,16);
		val[8]='\0';
		priv->pshare->mp_filter_BSSID[2]=_atoi(val+6,16);
		val[11]='\0';
		priv->pshare->mp_filter_BSSID[3]=_atoi(val+9,16);
		val[14]='\0';
		priv->pshare->mp_filter_BSSID[4]=_atoi(val+12,16);
		val[17]='\0';
		priv->pshare->mp_filter_BSSID[5]=_atoi(val+15,16);
		sprintf(data,"flag: %x\nBSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",priv->pshare->mp_filter_flag,
															priv->pshare->mp_filter_BSSID[0],
															priv->pshare->mp_filter_BSSID[1],
															priv->pshare->mp_filter_BSSID[2],
															priv->pshare->mp_filter_BSSID[3],
															priv->pshare->mp_filter_BSSID[4],
															priv->pshare->mp_filter_BSSID[5]);
		return strlen(data)+1;
	} else if( (!memcmp(data, "filter_clean",12))  || (!memcmp(data, "filter_init",11))) {
		OPMODE_VAL(OPMODE &  (~WIFI_MP_ARX_FILTER));
		priv->pshare->mp_filter_flag=0;
		memset(priv->pshare->mp_filter_SA,'\0',6);
		memset(priv->pshare->mp_filter_DA,'\0',6);
		memset(priv->pshare->mp_filter_BSSID,'\0',6);
		sprintf(data,"reset arx filter table\n");
		return strlen(data)+1;
	} else if (!memcmp(data, "filter_print",12)) {
		sprintf(data,"flag: %x\nSA: %02x:%02x:%02x:%02x:%02x:%02x\nDA: %02x:%02x:%02x:%02x:%02x:%02x\nBSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",priv->pshare->mp_filter_flag,
															priv->pshare->mp_filter_SA[0],
															priv->pshare->mp_filter_SA[1],
															priv->pshare->mp_filter_SA[2],
															priv->pshare->mp_filter_SA[3],
															priv->pshare->mp_filter_SA[4],
															priv->pshare->mp_filter_SA[5],
															priv->pshare->mp_filter_DA[0],
															priv->pshare->mp_filter_DA[1],
															priv->pshare->mp_filter_DA[2],
															priv->pshare->mp_filter_DA[3],
															priv->pshare->mp_filter_DA[4],
															priv->pshare->mp_filter_DA[5],
															priv->pshare->mp_filter_BSSID[0],
															priv->pshare->mp_filter_BSSID[1],
															priv->pshare->mp_filter_BSSID[2],
															priv->pshare->mp_filter_BSSID[3],
															priv->pshare->mp_filter_BSSID[4],
															priv->pshare->mp_filter_BSSID[5]);

		return strlen(data)+1;
	}

	return 0;
}


/*
 * set bssid
 */
void mp_set_bssid(struct rtl8192cd_priv *priv, unsigned char *data)
{
	unsigned char pbuf[6];
	int ret;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}

	ret = get_array_val(pbuf, (char *)data, strlen(data));

	if (ret != 6) {
		printk("Error bssid format\n");
		return;
	} else {
		printk("set bssid to %02X:%02X:%02X:%02X:%02X:%02X\n",
			pbuf[0], pbuf[1], pbuf[2], pbuf[3], pbuf[4], pbuf[5]);
	}

	memcpy(BSSID, pbuf, MACADDRLEN);
}




static void mp_chk_sw_ant(struct rtl8192cd_priv *priv)
{
	R_ANTENNA_SELECT_OFDM	*p_ofdm_tx;	/* OFDM Tx register */
	R_ANTENNA_SELECT_CCK	*p_cck_txrx;
	unsigned char			r_rx_antenna_ofdm=0, r_ant_select_cck_val=0;
	unsigned char			chgTx=0, chgRx=0;
	unsigned int			r_ant_sel_cck_val=0, r_ant_select_ofdm_val=0, r_ofdm_tx_en_val=0;

	p_ofdm_tx = (R_ANTENNA_SELECT_OFDM *)&r_ant_select_ofdm_val;
	p_cck_txrx = (R_ANTENNA_SELECT_CCK *)&r_ant_select_cck_val;

	p_ofdm_tx->r_ant_ht1			= 0x1;
	p_ofdm_tx->r_ant_non_ht 		= 0x3;
	p_ofdm_tx->r_ant_ht2			= 0x2;

	// 原因是Tx 3-wire enable需隨Tx Ant path打開才會開啟，
	// 所以需在設BB 0x824與0x82C時，同時將BB 0x804[3:0]設為3(同時打開Ant. A and B)。
	// 要省電的情況下，A Tx時 0x90C=0x11111111，B Tx時 0x90C=0x22222222，AB同時開就維持原來設定0x3321333


	switch(priv->pshare->mp_antenna_tx)
	{
	case ANTENNA_A:
		p_ofdm_tx->r_tx_antenna		= 0x1;
		r_ofdm_tx_en_val			= 0x1;
		p_ofdm_tx->r_ant_l 			= 0x1;
		p_ofdm_tx->r_ant_ht_s1 		= 0x1;
		p_ofdm_tx->r_ant_non_ht_s1 	= 0x1;
		p_cck_txrx->r_ccktx_enable	= 0x8;
		chgTx = 1;
			PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, 0xe, 2);
			PHY_SetBBReg(priv, rFPGA0_XB_HSSIParameter2, 0xe, 1);
			r_ofdm_tx_en_val			= 0x3;
			// Power save
			r_ant_select_ofdm_val = 0x11111111;

#ifdef HIGH_POWER_EXT_PA
			if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
			if(priv->pshare->rf_ft_var.use_ext_pa) {
			PHY_SetBBReg(priv, 0x870, BIT(26), 1);
			PHY_SetBBReg(priv, 0x870, BIT(10), 0);
			}
			}
#endif
		break;
	case ANTENNA_B:
		p_ofdm_tx->r_tx_antenna		= 0x2;
		r_ofdm_tx_en_val			= 0x2;
		p_ofdm_tx->r_ant_l 			= 0x2;
		p_ofdm_tx->r_ant_ht_s1 		= 0x2;
		p_ofdm_tx->r_ant_non_ht_s1 	= 0x2;
		p_cck_txrx->r_ccktx_enable	= 0x4;
		chgTx = 1;
			PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, 0xe, 1);
			PHY_SetBBReg(priv, rFPGA0_XB_HSSIParameter2, 0xe, 2);
			r_ofdm_tx_en_val			= 0x3;
			// Power save
			r_ant_select_ofdm_val = 0x22222222;

#ifdef HIGH_POWER_EXT_PA
		if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
		if (priv->pshare->rf_ft_var.use_ext_pa) {
			PHY_SetBBReg(priv, 0x870, BIT(26), 0);
			PHY_SetBBReg(priv, 0x870, BIT(10), 1);
			}
		}
#endif
		break;
	case ANTENNA_AB:
		p_ofdm_tx->r_tx_antenna		= 0x3;
		r_ofdm_tx_en_val			= 0x3;
		p_ofdm_tx->r_ant_l 			= 0x3;
		p_ofdm_tx->r_ant_ht_s1 		= 0x3;
		p_ofdm_tx->r_ant_non_ht_s1 	= 0x3;
		p_cck_txrx->r_ccktx_enable	= 0xC;
		chgTx = 1;
		PHY_SetBBReg(priv, rFPGA0_XA_HSSIParameter2, 0xe, 2);
		PHY_SetBBReg(priv, rFPGA0_XB_HSSIParameter2, 0xe, 2);
		// Disable Power save
		r_ant_select_ofdm_val		= 0x3321333;

#ifdef HIGH_POWER_EXT_PA
		if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
		if (priv->pshare->rf_ft_var.use_ext_pa) {
			PHY_SetBBReg(priv, 0x870, BIT(26), 0);
			PHY_SetBBReg(priv, 0x870, BIT(10), 0);
		}
		}
#endif
		break;
	default:
		break;
	}

	//
	// r_rx_antenna_ofdm, bit0=A, bit1=B, bit2=C, bit3=D
	// r_cckrx_enable : CCK default, 0=A, 1=B, 2=C, 3=D
	// r_cckrx_enable_2 : CCK option, 0=A, 1=B, 2=C, 3=D
	//
	switch(priv->pshare->mp_antenna_rx)
	{
	case ANTENNA_A:
		r_rx_antenna_ofdm 			= 0x11;	// A
		p_cck_txrx->r_cckrx_enable 	= 0x0;	// default: A
		p_cck_txrx->r_cckrx_enable_2= 0x0;	// option: A
		chgRx = 1;
		PHY_SetRFReg(priv, 0, 7, 0x3, 0x0);
		break;
	case ANTENNA_B:
		r_rx_antenna_ofdm 			= 0x22;	// B
		p_cck_txrx->r_cckrx_enable 	= 0x1;	// default: B
		p_cck_txrx->r_cckrx_enable_2= 0x1;	// option: B
		chgRx = 1;
		PHY_SetRFReg(priv, 0, 7, 0x3, 0x1);
		break;
	case ANTENNA_AB:	// For 8192S and 8192E/U...
		r_rx_antenna_ofdm 			= 0x33;	// AB
		p_cck_txrx->r_cckrx_enable 	= 0x0;	// default:A
		p_cck_txrx->r_cckrx_enable_2= 0x1;		// option:B
		chgRx = 1;
		break;
	}

	if(chgTx && chgRx) {
		r_ant_sel_cck_val = r_ant_select_cck_val;//(r_ant_select_cck_val<<24);
		PHY_SetBBReg(priv, rFPGA1_TxInfo, 0x0fffffff, r_ant_select_ofdm_val);		//OFDM Tx
		PHY_SetBBReg(priv, rFPGA0_TxInfo, 0x0000000f, r_ofdm_tx_en_val);		//OFDM Tx
		PHY_SetBBReg(priv, rOFDM0_TRxPathEnable, 0x000000ff, r_rx_antenna_ofdm);	//OFDM Rx
		PHY_SetBBReg(priv, rOFDM1_TRxPathEnable, 0x0000000f, r_rx_antenna_ofdm);	//OFDM Rx
		PHY_SetBBReg(priv, rCCK0_AFESetting, bMaskByte3, r_ant_sel_cck_val);		//CCK TxRx
	}

}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void mp_set_ant_tx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}
	unsigned char tx2path=0;

	if (!strcmp(data, "a")) {
		priv->pshare->mp_antenna_tx = ANTENNA_A;
		priv->pshare->mp_antenna_rx = ANTENNA_A;
	} else if (!strcmp(data, "b")) {
		priv->pshare->mp_antenna_tx = ANTENNA_B;
		priv->pshare->mp_antenna_rx = ANTENNA_B;
	} else if  (!strcmp(data, "c")) {
		priv->pshare->mp_antenna_tx = ANTENNA_C;
		priv->pshare->mp_antenna_rx = ANTENNA_C;
	} else if  (!strcmp(data, "d")) {
		priv->pshare->mp_antenna_tx = ANTENNA_D;
		priv->pshare->mp_antenna_rx = ANTENNA_D;
	} else if  (!strcmp(data, "ab")) {
		priv->pshare->mp_antenna_tx = ANTENNA_AB;
		priv->pshare->mp_antenna_rx = ANTENNA_AB;
		tx2path = 1;
	} else if  (!strcmp(data, "bc")) {
		priv->pshare->mp_antenna_tx = ANTENNA_BC;
		priv->pshare->mp_antenna_rx = ANTENNA_BC;
	} else if  (!strcmp(data, "cd")) {
		priv->pshare->mp_antenna_tx = ANTENNA_CD;
		priv->pshare->mp_antenna_rx = ANTENNA_CD;
	} else if  (!strcmp(data, "abc")) {
		priv->pshare->mp_antenna_tx = ANTENNA_ABC;
		priv->pshare->mp_antenna_rx = ANTENNA_ABC;
	} else if  (!strcmp(data, "bcd")) {
		priv->pshare->mp_antenna_tx = ANTENNA_BCD;
		priv->pshare->mp_antenna_rx = ANTENNA_BCD;
	} else if  (!strcmp(data, "abcd")) {
		priv->pshare->mp_antenna_tx = ANTENNA_ABCD;
		priv->pshare->mp_antenna_rx = ANTENNA_ABCD;
	} else {
		printk("Usage: mp_ant_tx {a,b,ab}\n");
		return;
	}
	if (CHECKICIS8822())
		config_phydm_trx_mode_8822b(ODMPTR,priv->pshare->mp_antenna_tx,priv->pshare->mp_antenna_rx,1);
	else
	if (CHECKICIS97F()) //wish
		config_phydm_trx_mode_8197f(ODMPTR,priv->pshare->mp_antenna_tx,priv->pshare->mp_antenna_rx,tx2path);
	else
		mp_chk_sw_ant(priv);
	mp_8192CD_tx_setting(priv);

	printk("switch Tx antenna to %s\n", data);
}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
void mp_set_ant_rx(struct rtl8192cd_priv *priv, unsigned char *data)
{
	int temp_IG;

	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}
	unsigned char tx2path=0;
	if (!strcmp(data, "a")) {
		priv->pshare->mp_antenna_rx = ANTENNA_A;
	} else if (!strcmp(data, "b")) {
		priv->pshare->mp_antenna_rx = ANTENNA_B;
	} else if  (!strcmp(data, "c")) {
		priv->pshare->mp_antenna_rx = ANTENNA_C;
	} else if  (!strcmp(data, "d")) {
		priv->pshare->mp_antenna_rx = ANTENNA_D;
	} else if (!strcmp(data, "ab")) {
		priv->pshare->mp_antenna_rx = ANTENNA_AB;
		tx2path = 1;
	} else if  (!strcmp(data, "bc")) {
		priv->pshare->mp_antenna_rx = ANTENNA_BC;
	} else if  (!strcmp(data, "cd")) {
		priv->pshare->mp_antenna_rx = ANTENNA_CD;
	} else if  (!strcmp(data, "abc")) {
		priv->pshare->mp_antenna_rx = ANTENNA_ABC;
	} else if  (!strcmp(data, "bcd")) {
		priv->pshare->mp_antenna_rx = ANTENNA_BCD;
	} else if  (!strcmp(data, "abcd")) {
		priv->pshare->mp_antenna_rx = ANTENNA_ABCD;
	} else {
		printk("Usage: mp_ant_tx {a,b,ab}\n");

		return;
	}


		if (CHECKICIS8822()){
			priv->pshare->mp_antenna_tx=priv->pshare->mp_antenna_rx; //suggested by BB and RF
			config_phydm_trx_mode_8822b(ODMPTR,priv->pshare->mp_antenna_tx,priv->pshare->mp_antenna_rx,1);
		}else
		if (CHECKICIS97F())//wish
			config_phydm_trx_mode_8197f(ODMPTR,priv->pshare->mp_antenna_tx,priv->pshare->mp_antenna_rx,tx2path);
		else
			mp_chk_sw_ant(priv);
	printk("switch Rx antenna to %s\n", data);
}


void mp_set_phypara(struct rtl8192cd_priv *priv, unsigned char *data)
{
	char *val;
	int xcap=-32, sign;

	if (!netif_running(priv->dev))
	{
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE))
	{
		printk("Fail: not in MP mode\n");
		return;
	}


	// get CrystalCap value
	val = get_value_by_token((char *)data, "xcap=");
	if (val) {
		if (*val == '-') {
			sign = 1;
			val++;
		}
		else
			sign = 0;

		xcap = _atoi(val, 10);
		if (sign)
			xcap = 0 - xcap;
	}

	// set CrystalCap value
	if(GET_CHIP_VER(priv) == VERSION_8192D){
		if (xcap != -32) {
			/*
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter1, bXtalCap01, (xcap & 0x00000003));
			PHY_SetBBReg(priv, rFPGA0_AnalogParameter2, bXtalCap23, ((xcap & 0x0000000c) >> 2));
			*/
			PHY_SetBBReg(priv, 0x24, 0xF0, xcap & 0x0F);
			PHY_SetBBReg(priv, 0x28, 0xF0000000, ((xcap & 0xF0) >> 4));
		}
	}
	if (GET_CHIP_VER(priv) == VERSION_8822B) {
		if(xcap > 0 && xcap < 0x3F) {
			PHY_SetBBReg(priv, 0x10, 0x00000040, 0x01); // control by WiFi
			PHY_SetBBReg(priv, 0x28, 0x0000007e, xcap & 0x3F); // 0x28[6:1]
			PHY_SetBBReg(priv, 0x24, 0x7e000000, xcap & 0x3F); // 0x24[30:25]
		}
	}

	if (GET_CHIP_VER(priv) == VERSION_8197F) {
		if(priv->pmib->dot11RFEntry.share_xcap){
			PHY_SetBBReg(priv, 0x24, BIT(30) | BIT(29) | BIT(28) | BIT(27) | BIT(26) | BIT(25), 0x3F);
			PHY_SetBBReg(priv, 0x28, BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1), 0x3F);
		}else{
			if (xcap > 0 && xcap < 0x3F) {
				PHY_SetBBReg(priv, 0x24, BIT(30) | BIT(29) | BIT(28) | BIT(27) | BIT(26) | BIT(25), xcap & 0x3F);
				PHY_SetBBReg(priv, 0x28, BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2) | BIT(1), xcap & 0x3F);
			}
		}
	}



	printk("Set xcap=%d\n", xcap);
}

// TODO: 8814AE BB/RF


void mp_reset_stats(struct rtl8192cd_priv *priv)
{
	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
		return;
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
		return;
	}

	memset(&priv->net_stats, 0,  sizeof(struct net_device_stats));
	memset(&priv->ext_stats, 0,  sizeof(struct extra_stats));
}

void mp_help(struct rtl8192cd_priv *priv)
{
	if (!netif_running(priv->dev)) {
		printk("\nFail: interface not opened\n");
	}

	if (!(OPMODE & WIFI_MP_STATE)) {
		printk("Fail: not in MP mode\n");
	}

	printk("\nUsage:\n");
	printk("  iwpriv wlanx mp_help}\n");
	printk("  iwpriv wlanx mp_stop\n");
	printk("  iwpriv wlanx mp_rate {2-108,128-143,144-163}\n");
	printk("  iwpriv wlanx mp_channel {1-14}\n");
	printk("  iwpriv wlanx mp_bandwidth [40M={0|1|2},shortGI={0|1}]\n");
	printk("		- default: BW=0, shortGI=0\n");
	printk("  iwpriv wlanx mp_txpower [patha=x,pathb=y]\n");
	printk("		- if no parameters, driver will set tx power according to flash setting.\n");
	printk("  iwpriv wlanx mp_phypara xcap=x\n");
	printk("  iwpriv wlanx mp_bssid 001122334455\n");
	printk("  iwpriv wlanx mp_ant_tx {a,b,ab}\n");
	printk("  iwpriv wlanx mp_ant_rx {a,b,ab}\n");
	printk("  iwpriv wlanx mp_arx {start,stop}\n");
	printk("  iwpriv wlanx mp_ctx [time=t,count=n,background,stop,pkt,cs,stone,scr]\n");
	printk("		- if \"time\" is set, tx in t sec. if \"count\" is set, tx with n packet.\n");
	printk("		- if \"background\", it will tx continuously until \"stop\" is issued.\n");
	printk("		- if \"pkt\", send cck packet in packet mode (not h/w).\n");
	printk("		- if \"cs\", send cck packet with carrier suppression.\n");
	printk("		- if \"stone\", send packet in single-tone.\n");
	printk("		- default: tx infinitely (no background).\n");
	printk("  iwpriv wlanx mp_query\n");
#ifdef MP_PSD_SUPPORT
	printk("  iwpriv wlanx mp_psd\n");
#endif
	printk("  iwpriv wlanx mp_ther\n");
#if (IC_LEVEL >= LEVEL_8812)
	printk("  iwpriv wlanx mp_pwrtrk [ther={7-63}, stop]\n");
#else
	printk("  iwpriv wlanx mp_pwrtrk [ther={7-29}, stop]\n");
#endif

	printk("  iwpriv wlanx mp_reset_stats\n");
	printk("		- to reset tx and rx count\n");
	printk("  iwpriv wlanx mp_get_pwr\n");
#ifdef B2B_TEST
	printk("  iwpriv wlanx mp_tx [da=xxxxxx,time=n,count=n,len=n,retry=n,err=n]\n");
	printk("		- if \"time\" is set, tx in t sec. if \"count\" is set, tx with n packet.\n");
	printk("		- if \"time=-1\", tx infinitely.\n");
	printk("		- If \"err=1\", display statistics when tx err.\n");
	printk("		- default: da=ffffffffffff, time=0, count=1000, len=1500,\n");
	printk("			  retry=6(mac retry limit), err=1.\n");
#if 0
	printk("  iwpriv wlanx mp_rx [ra=xxxxxx,quiet=t,interval=n]\n");
	printk("		- ra: rx mac. defulat is burn-in mac\n");
	printk("		- quiet_time: quit rx if no rx packet during quiet_time. default is 5s\n");
	printk("		- interval: report rx statistics periodically in sec.\n");
	printk("			  default is 0 (no report).\n");
#endif
	printk("  iwpriv wlanx mp_brx {start[,ra=xxxxxx],stop}\n");
	printk("		- start: start rx immediately.\n");
	printk("		- ra: rx mac. defulat is burn-in mac.\n");
	printk("		- stop: stop rx immediately.\n");
#endif// B2B_TEST
	printk("  iwpriv wlanx mp_version}\n");

}

#endif	// MP_TEST

