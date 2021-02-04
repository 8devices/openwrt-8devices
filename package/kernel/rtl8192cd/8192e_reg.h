/*
 *  Header file of 8188E register
 *
 *	 $Id: 8188e_reg.h,v 1.1 2011/06/30 11:02:56 victoryman Exp $
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192E_REG_H_
#define _8192E_REG_H_

//============================================================
//       8188E Regsiter offset definition
//============================================================


/*
 *	Project RTL8188E follows most of registers in Project RTL8192c
 *	This file includes additional registers for RTL8188E only
 *	Header file of RTL8192C registers should always be included
 */

//
// 1. System Configure Register. (Offset 0x000 - 0x0FFh)
//
#define		REG_92E_BB_PAD_CTRL			0x64
#define		REG_92E_HMEBOX_E0			0x88
#define		REG_92E_HMEBOX_E1			0x8A
#define		REG_92E_HMEBOX_E2			0x8C
#define		REG_92E_HMEBOX_E3			0x8E
#define		REG_92E_WLLPS_CTRL			0x90
#define		REG_92E_RPWM2				0x9E
#define		REG_92E_HIMR				0xB0
#define		REG_92E_HISR				0xB4
#define		REG_92E_HIMRE				0xB8
#define		REG_92E_HISRE				0xBC
#define		REG_92E_EFUSE_DATA1			0xCC
#define		REG_92E_EFUSE_DATA0			0xCD
#define		REG_92E_EPPR				0xCF

#define		REG_92E_TQPNT1				0x218
#define		REG_92E_TQPNT2				0x21C
#define		REG_92E_TDECTRL1			0x228
#define		REG_92E_WATCHDOG			0x35C
#define		REG_92E_VOQ_IDX				0x310
#define		REG_92E_VIQ_IDX				0x314
#define		REG_92E_MGQ_DESA			0x318
#define		REG_92E_VOQ_DESA			0x320
#define		REG_92E_VIQ_DESA			0x328
#define		REG_92E_BEQ_DESA			0x330
#define		REG_92E_BKQ_DESA			0x338
#define		REG_92E_PCIE_HRPWM			0x361
#define		REG_92E_PCIE_CLK_RECOVER	0x362
#define		REG_92E_PCIE_HCPWM			0x363
#define		REG_92E_BEQ_IDX				0x364
#define		REG_92E_BKQ_IDX				0x368
#define		REG_92E_MGQ_IDX				0x36C
#define		REG_92E_HI0Q_IDX			0x370
#define		REG_92E_HI1Q_IDX			0x374
#define		REG_92E_HI2Q_IDX			0x378
#define		REG_92E_HI3Q_IDX			0x37C
#define		REG_92E_PCIE_HRPWM2			0x380
#define		REG_92E_PCIE_HCPWM2			0x382
#define		REG_92E_HCI_PCIE_H2C_MSG	0x384
#define		REG_92E_HCI_PCIE_C2H_MSG	0x388
#define		REG_92E_RXQ_IDX				0x38C
#define		REG_92E_HI4Q_IDX			0x390
#define		REG_92E_HI5Q_IDX			0x394
#define		REG_92E_HI6Q_IDX			0x398
#define		REG_92E_HI7Q_IDX			0x39C
#define		REG_92E_HQ_DES_NUM0			0x3A0
#define		REG_92E_HQ_DES_NUM1			0x3A4
#define		REG_92E_HQ_DES_NUM2			0x3A8
#define		REG_92E_HQ_DES_NUM3			0x3AC
#define		REG_92E_TSFT_CLRQ			0x3B0
#define		REG_92E_ACQ_DES_NUM0		0x3B4
#define		REG_92E_ACQ_DES_NUM1		0x3B8
#define		REG_92E_ACQ_DES_NUM2		0x3BC
#define		REG_92E_HI0Q_DESA			0x3C0
#define		REG_92E_HI1Q_DESA			0x3C8
#define		REG_92E_HI2Q_DESA			0x3D0
#define		REG_92E_HI3Q_DESA			0x3D8
#define		REG_92E_HI4Q_DESA			0x3E0
#define		REG_92E_HI5Q_DESA			0x3E8
#define		REG_92E_HI6Q_DESA			0x3F0
#define		REG_92E_HI7Q_DESA			0x3F8
#define  	REG_92E_TXPKTBUF_BCNQ_BDNY1	0x457
#define		REG_92E_MACID_NOLINK		0x484
#define		REG_92E_MACID_PAUSE			0x48C
#define		REG_92E_TXRPT_CTRL			0x4EC
#define		REG_92E_TXRPT_TIM			0x4F0
#define		REG_92E_TXRPT_STSSET		0x4F2
#define		REG_92E_TXRPT_STSVLD		0x4F4
#define		REG_92E_TXRPT_STSINF		0x4F8
#define		REG_92E_MBSSID_CTRL			0x526
#define 	REG_92E_PKT_LIFETIME_CTRL	0x528
#define 	REG_92E_ATIMWND1			0x570
#define 	REG_92E_PRE_DL_BCN_ITV		0x58F
#define 	REG_92E_ATIMWND2			0x5A0
#define 	REG_92E_ATIMWND3			0x5A1
#define 	REG_92E_ATIMWND4			0x5A2
#define 	REG_92E_ATIMWND5			0x5A3
#define 	REG_92E_ATIMWND6			0x5A4
#define 	REG_92E_ATIMWND7			0x5A5
#define 	REG_92E_ATIMUGT				0x5A6
#define 	REG_92E_HIQ_NO_LMT_EN		0x5A7
#define 	REG_92E_DTIM_COUNT_ROOT		0x5A8
#define 	REG_92E_DTIM_COUNT_VAP1		0x5A9
#define 	REG_92E_DTIM_COUNT_VAP2		0x5AA
#define 	REG_92E_DTIM_COUNT_VAP3		0x5AB
#define 	REG_92E_DTIM_COUNT_VAP4		0x5AC
#define 	REG_92E_DTIM_COUNT_VAP5		0x5AD
#define 	REG_92E_DTIM_COUNT_VAP6		0x5AE
#define 	REG_92E_DTIM_COUNT_VAP7		0x5AF
#define 	REG_92E_DIS_ATIM			0x5B0
#define 	REG_92E_UPD_HGQMD			0x604

//----------------------------------------------------------------------------
//       8192E REG_92E_HIMR bits				(Offset 0xB0-B3, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192E REG_92E_HISR bits				(Offset 0xB4-B7, 32 bits)
//----------------------------------------------------------------------------
#define	HIMR_92E_TXCCK					BIT(30)		// TXRPT interrupt when CCX bit of the packet is set	
#define	HIMR_92E_PSTIMEOUT				BIT(29)		// Power Save Time Out Interrupt
#define	HIMR_92E_GTINT4					BIT(28)		// When GTIMER4 expires, this bit is set to 1	
#define	HIMR_92E_GTINT3					BIT(27)		// When GTIMER3 expires, this bit is set to 1	
#define	HIMR_92E_TBDER					BIT(26)		// Transmit Beacon0 Error			
#define	HIMR_92E_TBDOK					BIT(25)		// Transmit Beacon0 OK, ad hoc only
#define	HIMR_92E_TSF_BIT32_TOGGLE		BIT(24)		// TSF Timer BIT32 toggle indication interrupt			
#define	HIMR_92E_BcnInt					BIT(20)		// Beacon DMA Interrupt 0			
#define	HIMR_92E_BDERR0					BIT(16)		// Beacon Queue DMA OK0			
#define	HIMR_92E_HSISR_IND_ON_INT		BIT(15)		// HSISR Indicator (HSIMR & HSISR is true, this bit is set to 1)			
#define	HIMR_92E_BCNDMAINT_E			BIT(14)		// Beacon DMA Interrupt Extension for Win7			
#define	HIMR_92E_ATIMEND				BIT(12)		// CTWidnow End or ATIM Window End
#define	HIMR_92E_HISR1_IND_INT			BIT(11)		// HISR1 Indicator (HISR1 & HIMR1 is true, this bit is set to 1)
#define	HIMR_92E_C2HCMD					BIT(10)		// CPU to Host Command INT Status, Write 1 clear	
#define	HIMR_92E_CPWM2					BIT(9)		// CPU power Mode exchange INT Status, Write 1 clear	
#define	HIMR_92E_CPWM					BIT(8)		// CPU power Mode exchange INT Status, Write 1 clear	
#define	HIMR_92E_HIGHDOK				BIT(7)		// High Queue DMA OK	
#define	HIMR_92E_MGNTDOK				BIT(6)		// Management Queue DMA OK	
#define	HIMR_92E_BKDOK					BIT(5)		// AC_BK DMA OK		
#define	HIMR_92E_BEDOK					BIT(4)		// AC_BE DMA OK	
#define	HIMR_92E_VIDOK					BIT(3)		// AC_VI DMA OK		
#define	HIMR_92E_VODOK					BIT(2)		// AC_VO DMA OK	
#define	HIMR_92E_RDU					BIT(1)		// Rx Descriptor Unavailable	
#define	HIMR_92E_ROK					BIT(0)		// Receive DMA OK

//----------------------------------------------------------------------------
//       8192E REG_92E_HIMRE bits			(Offset 0xB8-BB, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192E REG_92E_HIMSE bits			(Offset 0xBC-BF, 32 bits)
//----------------------------------------------------------------------------
#define	HIMRE_92E_BCNDMAINT7			BIT(27)		// Beacon DMA Interrupt 7
#define	HIMRE_92E_BCNDMAINT6			BIT(26)		// Beacon DMA Interrupt 6
#define	HIMRE_92E_BCNDMAINT5			BIT(25)		// Beacon DMA Interrupt 5
#define	HIMRE_92E_BCNDMAINT4			BIT(24)		// Beacon DMA Interrupt 4
#define	HIMRE_92E_BCNDMAINT3			BIT(23)		// Beacon DMA Interrupt 3
#define	HIMRE_92E_BCNDMAINT2			BIT(22)		// Beacon DMA Interrupt 2
#define	HIMRE_92E_BCNDMAINT1			BIT(21)		// Beacon DMA Interrupt 1
#define	HIMRE_92E_BCNDOK7				BIT(20)		// Beacon Queue DMA OK Interrup 7
#define	HIMRE_92E_BCNDOK6				BIT(19)		// Beacon Queue DMA OK Interrup 6
#define	HIMRE_92E_BCNDOK5				BIT(18)		// Beacon Queue DMA OK Interrup 5
#define	HIMRE_92E_BCNDOK4				BIT(17)		// Beacon Queue DMA OK Interrup 4
#define	HIMRE_92E_BCNDOK3				BIT(16)		// Beacon Queue DMA OK Interrup 3
#define	HIMRE_92E_BCNDOK2				BIT(15)		// Beacon Queue DMA OK Interrup 2
#define	HIMRE_92E_BCNDOK1				BIT(14)		// Beacon Queue DMA OK Interrup 1
#define	HIMRE_92E_ATIMEND_E				BIT(13)		// ATIM Window End Extension for Win7
#define	HIMRE_92E_TXERR					BIT(11)		// Tx Error Flag Interrupt Status, write 1 clear.
#define	HIMRE_92E_RXERR					BIT(10)		// Rx Error Flag INT Status, Write 1 clear
#define	HIMRE_92E_TXFOVW				BIT(9)		// Transmit FIFO Overflow
#define	HIMRE_92E_RXFOVW				BIT(8)		// Receive FIFO Overflow

//----------------------------------------------------------------------------
//       8192E REG_92E_HQ_DES_NUM0 bits		(Offset 0x3A0-3A3, 32 bits)
//----------------------------------------------------------------------------
#define	ACQ_92E_H1Q_DESCS_MODE_8SEG		BIT(31)	
#define ACQ_92E_H1Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H1Q_DESC_NUM_SHIFT		16
#define	ACQ_92E_H0Q_DESCS_MODE_8SEG		BIT(15)	
#define ACQ_92E_H0Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H0Q_DESC_NUM_SHIFT		0

//----------------------------------------------------------------------------
//       8192E REG_92E_HQ_DES_NUM1 bits		(Offset 0x3A4-3A7, 32 bits)
//----------------------------------------------------------------------------
#define	ACQ_92E_H3Q_DESCS_MODE_8SEG		BIT(31)	
#define ACQ_92E_H3Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H3Q_DESC_NUM_SHIFT		16
#define	ACQ_92E_H2Q_DESCS_MODE_8SEG		BIT(15)	
#define ACQ_92E_H2Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H2Q_DESC_NUM_SHIFT		0

//----------------------------------------------------------------------------
//       8192E REG_92E_HQ_DES_NUM2 bits		(Offset 0x3A8-3AB, 32 bits)
//----------------------------------------------------------------------------
#define	ACQ_92E_H5Q_DESCS_MODE_8SEG		BIT(31)	
#define ACQ_92E_H5Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H5Q_DESC_NUM_SHIFT		16
#define	ACQ_92E_H4Q_DESCS_MODE_8SEG		BIT(15)	
#define ACQ_92E_H4Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H4Q_DESC_NUM_SHIFT		0

//----------------------------------------------------------------------------
//       8192E REG_92E_HQ_DES_NUM3 bits		(Offset 0x3AC-3AF, 32 bits)
//----------------------------------------------------------------------------
#define	ACQ_92E_H7Q_DESCS_MODE_8SEG		BIT(31)	
#define ACQ_92E_H7Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H7Q_DESC_NUM_SHIFT		16
#define	ACQ_92E_H6Q_DESCS_MODE_8SEG		BIT(15)	
#define ACQ_92E_H6Q_DESC_NUM_MASK		0xfff
#define ACQ_92E_H6Q_DESC_NUM_SHIFT		0

//----------------------------------------------------------------------------
//       8192E REG_92E_CLRQ bits				(Offset 0x3B0-3B4, 32 bits)
//----------------------------------------------------------------------------
#define CLRQ_92E_ALL_IDX				0x3FFF3FFF
#define	CLRQ_92E_HI7Q_HW_IDX			BIT(29)
#define	CLRQ_92E_HI6Q_HW_IDX			BIT(28)
#define	CLRQ_92E_HI5Q_HW_IDX			BIT(27)
#define	CLRQ_92E_HI4Q_HW_IDX			BIT(26)
#define	CLRQ_92E_HI3Q_HW_IDX			BIT(25)
#define	CLRQ_92E_HI2Q_HW_IDX			BIT(24)
#define	CLRQ_92E_HI1Q_HW_IDX			BIT(23)
#define	CLRQ_92E_HI0Q_HW_IDX			BIT(22)
#define	CLRQ_92E_BKQ_HW_IDX				BIT(21)
#define	CLRQ_92E_BEQ_HW_IDX				BIT(20)
#define	CLRQ_92E_VIQ_HW_IDX				BIT(19)
#define	CLRQ_92E_VOQ_HW_IDX				BIT(18)
#define	CLRQ_92E_MGQ_HW_IDX				BIT(17)
#define	CLRQ_92E_RXQ_HW_IDX				BIT(16)
#define	CLRQ_92E_HI7Q_HOST_IDX			BIT(13)
#define	CLRQ_92E_HI6Q_HOST_IDX			BIT(12)
#define	CLRQ_92E_HI5Q_HOST_IDX			BIT(11)
#define	CLRQ_92E_HI4Q_HOST_IDX			BIT(10)
#define	CLRQ_92E_HI3Q_HOST_IDX			BIT(9)
#define	CLRQ_92E_HI2Q_HOST_IDX			BIT(8)
#define	CLRQ_92E_HI1Q_HOST_IDX			BIT(7)
#define	CLRQ_92E_HI0Q_HOST_IDX			BIT(6)
#define	CLRQ_92E_BKQ_HOST_IDX			BIT(5)
#define	CLRQ_92E_BEQ_HOST_IDX			BIT(4)
#define	CLRQ_92E_VIQ_HOST_IDX			BIT(3)
#define	CLRQ_92E_VOQ_HOST_IDX			BIT(2)
#define	CLRQ_92E_MGQ_HOST_IDX			BIT(1)
#define	CLRQ_92E_RXQ_HOST_IDX			BIT(0)

//----------------------------------------------------------------------------
//       8192E REG_92E_ACQ_DES_NUM0 bits		(Offset 0x3B4-3B7, 32 bits)
//----------------------------------------------------------------------------
#define	ACQ_92E_VIQ_DESCS_MODE_8SEG		BIT(31)	
#define ACQ_92E_VIQ_DESC_NUM_MASK		0xfff
#define ACQ_92E_VIQ_DESC_NUM_SHIFT		16
#define	ACQ_92E_VOQ_DESCS_MODE_8SEG		BIT(15)	
#define ACQ_92E_VOQ_DESC_NUM_MASK		0xfff
#define ACQ_92E_VOQ_DESC_NUM_SHIFT		0

//----------------------------------------------------------------------------
//       8192E REG_92E_ACQ_DES_NUM1 bits		(Offset 0x3B8-3BB, 32 bits)
//----------------------------------------------------------------------------
#define	ACQ_92E_BKQ_DESCS_MODE_8SEG		BIT(31)	
#define ACQ_92E_BKQ_DESC_NUM_MASK		0xfff
#define ACQ_92E_BKQ_DESC_NUM_SHIFT		16
#define	ACQ_92E_BEQ_DESCS_MODE_8SEG		BIT(15)	
#define ACQ_92E_BEQ_DESC_NUM_MASK		0xfff
#define ACQ_92E_BEQ_DESC_NUM_SHIFT		0

//----------------------------------------------------------------------------
//       8192E REG_92E_ACQ_DES_NUM2 bits		(Offset 0x3BC-3BF, 32 bits)
//----------------------------------------------------------------------------
#define ACQ_92E_RXQ_DESC_NUM_MASK		0xfff
#define ACQ_92E_RXQ_DESC_NUM_SHIFT		16
#define	ACQ_92E_MGQ_DESCS_MODE_8SEG		BIT(15)	
#define ACQ_92E_MGQ_DESC_NUM_MASK		0xfff
#define ACQ_92E_MGQ_DESC_NUM_SHIFT		0

//----------------------------------------------------------------------------
//       8192E MBID_NUM bits					(Offset 0x552, 8 bits)
//----------------------------------------------------------------------------
#define	MBID_NUM_92E_EN_PREDOWN_BCN		BIT(3)	

//----------------------------------------------------------------------------
//       8192E REG_EFUSE_ACCESS			(Offset 0xCF, 8 bits)
//----------------------------------------------------------------------------
#define EFUSE_ACCESS_ON_8192E			0x69	
#define EFUSE_ACCESS_OFF_8192E			0x00	


//====================================================
//			EEPROM/Efuse PG Offset for 8192EE/8192EU/8192ES
//====================================================

#if defined(EN_EFUSE)
#define PATHA_OFFSET 0x10
#define PATHB_OFFSET 0x3A
#define PATHC_OFFSET 0x64
#define PATHD_OFFSET 0x8E
/*2.4 GHz, PATH A, 1T*/
#define EEPROM_2G_CCK1T_TxPower			0x0 // CCK Tx Power base
#define EEPROM_2G_HT401S_TxPower		0x6 // HT40 Tx Power base
#define EEPROM_2G_HT201S_TxPowerDiff	0xB // HT20 Tx Power Diff [7:4]
#define EEPROM_2G_OFDM1T_TxPowerDiff    0xB // OFDM Tx Power Diff [3:0]
/*2.4 GHz, PATH A, 2T*/
#define EEPROM_2G_HT402S_TxPowerDiff	0xC // HT40 Tx Power Diff [7:4]
#define EEPROM_2G_HT202S_TxPowerDiff	0xC // HT20 Tx Power Diff [3:0]
#define EEPROM_2G_OFDM2T_TxPowerDiff	0xD // OFDM Tx Power Diff [7:4]
#define EEPROM_2G_CCK2T_TxPowerDiff		0xD // CCK Tx Power Diff [3:0]
/*2.4 GHz, PATH A, 3T*/
#define EEPROM_2G_HT403S_TxPowerDiff	0xE // HT40 Tx Power Diff [7:4]
#define EEPROM_2G_HT203S_TxPowerDiff	0xE // HT40 Tx Power Diff [3:0]
#define EEPROM_2G_OFDM3T_TxPowerDiff	0xF // OFDM Tx Power Diff [7:4]
#define EEPROM_2G_CCK3T_TxPowerDiff		0xF // CCK Tx Power Diff [3:0]
/*2.4 GHz, PATH A, 4T*/
#define EEPROM_2G_HT404S_TxPowerDiff    0x10 // HT40 Tx Power Diff [7:4]
#define EEPROM_2G_HT204S_TxPowerDiff    0x10 // HT20 Tx Power Diff [0:3]
#define EEPROM_2G_OFDM4T_TxPowerDiff    0x11 // OFDM Tx Power Diff [7:4]
#define EEPROM_2G_CCK4T_TxPowerDiff		0x11 // CCK Tx Power Diff [3:0]
/*5 GHz, PATH A, 1T*/
#define EEPROM_5G_HT401S_TxPower		0x12 // HT40 Tx Power Base
#define EEPROM_5G_HT201S_TxPowerDiff    0x20 // HT20 Tx Power Diff [7:4]
#define EEPROM_5G_OFDM1T_TxPowerDiff    0x20 // OFDM Tx Power Diff [3:0]
#define EEPROM_5G_HT801S_TxPowerDiff    0x26 // HT80 Tx Power Diff [7:4]
#define EEPROM_5G_HT1601S_TxPowerDiff   0x26 // HT160 Tx Power Diff [3:0]
/*5 GHz, PATH A, 2T*/
#define EEPROM_5G_HT402S_TxPowerDiff    0x21 // HT40 Tx Power Diff [7:4]
#define EEPROM_5G_HT202S_TxPowerDiff    0x21 // HT20 Tx Power Diff [3:0]
#define EEPROM_5G_OFDM2T_TxPowerDiff    0x24 // OFDM Tx Power Diff [7:4]
#define EEPROM_5G_HT802S_TxPowerDiff    0x27 // HT80 Tx Power Diff [7:4]
/*5 GHz, PATH A, 3T*/
#define EEPROM_5G_OFDM3T_TxPowerDiff    0x24 // OFDM Tx Power Diff [3:0]
#define EEPROM_5G_HT403S_TxPowerDiff    0x22 // HT40 Tx Power Diff [7:4]
#define EEPROM_5G_HT203S_TxPowerDiff    0x22 // HT20 Tx Power Diff [3:0]
#define EEPROM_5G_HT803S_TxPowerDiff    0x28 // HT80 Tx Power Diff [7:4]
#define EEPROM_5G_HT1603S_TxPowerDiff   0x28 // HT160 Tx Power Diff [3:0]
/*5 GHz, PATH A, 4T*/
#define EEPROM_5G_HT404S_TxPowerDiff    0x23 // HT40 Tx Power Diff [7:4]
#define EEPROM_5G_HT204S_TxPowerDiff    0x23 // HT20 Tx Power Diff [3:0]
#define EEPROM_5G_OFDM4T_TxPowerDiff    0x25 // OFDM Tx Power Diff [3:0]
#define EEPROM_5G_HT804S_TxPowerDiff    0x29 // HT80 Tx Power Diff [7:4]
#define EEPROM_5G_HT1604S_TxPowerDiff   0x29 // HT160 Tx Power Diff [3:0]

#define EEPROM_92E_CHANNEL_PLAN		0xB8
#define EEPROM_92E_XTAL_K 				0xB9 //Crystal Calibration [5:0]
#define EEPROM_92E_THERMAL_METER		0xBA //Thermal meter
#ifdef CONFIG_SDIO_HCI
#define EEPROM_92E_SDIOTYPE             0xD0
#define EEPROM_92E_MACADDRESS		0x11A // MAC Address
#else
#define EEPROM_92E_MACADDRESS		0xD0 // MAC Address
#endif
#define EEPROM_RFE_OPTION_8192E			0xCA // 0xCA[6:4]: LNA Type ; 0xCA[3:2]:PA/LNA ; 0xCA[1:0]:RFE Type

#endif

#if 0

#define PATHA_OFFSET 0x10
#define PATHB_OFFSET 0x3A


// 0x10 ~ 0x63 = TX power area.
#define	EEPROM_TX_PWR_INX_8192E				0x0

#define	EEPROM_ChannelPlan_8192E			0xB8
#define	EEPROM_XTAL_8192E					0xB9
#define	EEPROM_THERMAL_METER_8192E			0xBA
#define	EEPROM_IQK_LCK_8192E				0xBB
#define	EEPROM_2G_5G_PA_TYPE_8192E			0xBC
#define	EEPROM_2G_LNA_TYPE_GAIN_SEL_8192E	0xBD
#define	EEPROM_5G_LNA_TYPE_GAIN_SEL_8192E	0xBF

#define	EEPROM_RF_BOARD_OPTION_8192E		0xC1
#define	EEPROM_RF_FEATURE_OPTION_8192E		0xC2
#define	EEPROM_RF_BT_SETTING_8192E			0xC3
#define	EEPROM_VERSION_8192E				0xC4
#define	EEPROM_CustomID_8192E				0xC5
#define	EEPROM_TX_BBSWING_2G_8192E			0xC6
#define	EEPROM_TX_BBSWING_5G_8192E			0xC7
#define	EEPROM_TX_PWR_CALIBRATE_RATE_8192E	0xC8
#define	EEPROM_RF_ANTENNA_OPT_8192E			0xC9

// RTL8192EE
#define	EEPROM_MAC_ADDR_8192EE				0xD0
#define	EEPROM_VID_8192EE					0xD6
#define	EEPROM_DID_8192EE					0xD8
#define	EEPROM_SVID_8192EE					0xDA
#define	EEPROM_SMID_8192EE					0xDC

//RTL8192EU
#define	EEPROM_MAC_ADDR_8192EU				0xD7
#define	EEPROM_VID_8192EU					0xD0
#define	EEPROM_PID_8192EU					0xD2
#define EEPROM_PA_TYPE_8192EU               0xBC
#define EEPROM_LNA_TYPE_2G_8192EU           0xBD
#define EEPROM_LNA_TYPE_5G_8192EU           0xBF

// RTL8192ES
#define	EEPROM_MAC_ADDR_8192ES				0x11B
#endif


//-----------------------------------------------------
//
//	RTL8192E SDIO Configuration
//
//-----------------------------------------------------

// I/O bus domain address mapping
#define SDIO_LOCAL_BASE				0x10250000
#define WLAN_IOREG_BASE				0x10260000
#define FIRMWARE_FIFO_BASE			0x10270000
#define TX_HIQ_BASE				0x10310000
#define TX_MIQ_BASE				0x10320000
#define TX_LOQ_BASE				0x10330000
#define TX_EXQ_BASE				0x10350000	// 92E Add
#define RX_RX0FF_BASE				0x10340000

// SDIO host local register space mapping.
#define SDIO_LOCAL_MSK				0x0FFF
#define WLAN_IOREG_MSK				0x7FFF
#define WLAN_FIFO_MSK		      		0x1FFF	// Aggregation Length[12:0]
#define WLAN_RX0FF_MSK			      	0x0003

#define SDIO_WITHOUT_REF_DEVICE_ID		0	// Without reference to the SDIO Device ID
#define SDIO_LOCAL_DEVICE_ID			0	// 0b[16], 000b[15:13]
#define WLAN_TX_HIQ_DEVICE_ID			4	// 0b[16], 100b[15:13]
#define WLAN_TX_MIQ_DEVICE_ID			5	// 0b[16], 101b[15:13]
#define WLAN_TX_LOQ_DEVICE_ID			6	// 0b[16], 110b[15:13]
#define WLAN_TX_EXQ_DEVICE_ID			3	// 0b[16], 011b[15:13] (False map) // 0b[16], 111b[15:13] (True map)(92E New)
#define WLAN_RX0FF_DEVICE_ID			7	// 0b[16], 111b[15:13]
#define WLAN_IOREG_DEVICE_ID			8	// 1b[16]

// SDIO Tx Free Page Index (This order must match SDIO_REG_FREE_TXPG)
#define HI_QUEUE_IDX				0
#define MID_QUEUE_IDX				1
#define LOW_QUEUE_IDX				2
#define PUBLIC_QUEUE_IDX			3
#define EXTRA_QUEUE_IDX				4		// 92E New

#define SDIO_MAX_TX_QUEUE			4		// HIQ, MIQ, LOQ and EXQ
#define SDIO_MAX_RX_QUEUE			1

#define SDIO_REG_TX_CTRL			0x0000 // SDIO Tx Control
#define SDIO_REG_HIMR				0x0014 // SDIO Host Interrupt Mask
#define SDIO_REG_HISR				0x0018 // SDIO Host Interrupt Service Routine
#define SDIO_REG_RX0_REQ_LEN			0x001C // RXDMA Request Length
#define SDIO_REG_OQT_FREE_SPACE		0x001E // OQT Free Space
#define SDIO_REG_FREE_TXPG			0x0020 // Free Tx Buffer Page
#define SDIO_REG_HCPWM1				0x0025 // HCI Current Power Mode 1
#define SDIO_REG_HCPWM2				0x0026 // HCI Current Power Mode 2
#define SDIO_REG_HTSFR_INFO			0x0030 // HTSF Informaion
#define SDIO_REG_HRPWM1				0x0080 // HCI Request Power Mode 1
#define SDIO_REG_HRPWM2				0x0082 // HCI Request Power Mode 2
#define SDIO_REG_HSUS_CTRL			0x0086 // SDIO HCI Suspend Control

#define SDIO_HIMR_DISABLED			0

// RTL8188E SDIO Host Interrupt Mask Register
#define SDIO_HIMR_RX_REQUEST_MSK		BIT0
#define SDIO_HIMR_AVAL_MSK			BIT1
#define SDIO_HIMR_TXERR_MSK			BIT2
#define SDIO_HIMR_RXERR_MSK			BIT3
#define SDIO_HIMR_TXFOVW_MSK			BIT4
#define SDIO_HIMR_RXFOVW_MSK			BIT5
#define SDIO_HIMR_TXBCNOK_MSK			BIT6
#define SDIO_HIMR_TXBCNERR_MSK			BIT7
#define SDIO_HIMR_BCNERLY_INT_MSK		BIT16
#define SDIO_HIMR_C2HCMD_MSK			BIT17
#define SDIO_HIMR_CPWM1_MSK			BIT18
#define SDIO_HIMR_CPWM2_MSK			BIT19
#define SDIO_HIMR_HSISR_IND_MSK			BIT20
#define SDIO_HIMR_GTINT3_IND_MSK		BIT21
#define SDIO_HIMR_GTINT4_IND_MSK		BIT22
#define SDIO_HIMR_PSTIMEOUT_MSK			BIT23
#define SDIO_HIMR_OCPINT_MSK			BIT24
#define SDIO_HIMR_ATIMEND_MSK			BIT25
#define SDIO_HIMR_ATIMEND_E_MSK			BIT26
#define SDIO_HIMR_CTWEND_MSK			BIT27

//RTL8188E SDIO Specific
#define	SDIO_HIMR_MCU_ERR_MSK			BIT28
#define	SDIO_HIMR_TSF_BIT32_TOGGLE_MSK	BIT29

// SDIO Host Interrupt Service Routine
#define SDIO_HISR_RX_REQUEST			BIT0
#define SDIO_HISR_AVAL				BIT1
#define SDIO_HISR_TXERR				BIT2
#define SDIO_HISR_RXERR				BIT3
#define SDIO_HISR_TXFOVW			BIT4
#define SDIO_HISR_RXFOVW			BIT5
#define SDIO_HISR_TXBCNOK			BIT6
#define SDIO_HISR_TXBCNERR			BIT7
#define SDIO_HISR_BCNERLY_INT			BIT16
#define SDIO_HISR_C2HCMD			BIT17
#define SDIO_HISR_CPWM1				BIT18
#define SDIO_HISR_CPWM2				BIT19
#define SDIO_HISR_HSISR_IND			BIT20
#define SDIO_HISR_GTINT3_IND			BIT21
#define SDIO_HISR_GTINT4_IND			BIT22
#define SDIO_HISR_PSTIMEOUT			BIT23
#define SDIO_HISR_OCPINT			BIT24
#define SDIO_HISR_ATIMEND			BIT25
#define SDIO_HISR_ATIMEND_E			BIT26
#define SDIO_HISR_CTWEND			BIT27

//RTL8188E SDIO Specific
#define	SDIO_HISR_MCU_ERR					BIT28
#define	SDIO_HISR_TSF_BIT32_TOGGLE		BIT29

#define MASK_SDIO_HISR_CLEAR		(SDIO_HISR_TXERR |\
									SDIO_HISR_RXERR |\
									SDIO_HISR_TXFOVW |\
									SDIO_HISR_RXFOVW |\
									SDIO_HISR_TXBCNOK |\
									SDIO_HISR_TXBCNERR |\
									SDIO_HISR_C2HCMD |\
									SDIO_HISR_CPWM1 |\
									SDIO_HISR_CPWM2 |\
									SDIO_HISR_HSISR_IND |\
									SDIO_HISR_GTINT3_IND |\
									SDIO_HISR_GTINT4_IND |\
									SDIO_HISR_PSTIMEOUT |\
									SDIO_HISR_OCPINT)

// SDIO HCI Suspend Control Register
#define HCI_RESUME_PWR_RDY			BIT1
#define HCI_SUS_CTRL				BIT0

// SDIO Tx FIFO related
#define SDIO_TX_FREE_PG_QUEUE			5	// The number of Tx FIFO free page
#define SDIO_TX_FIFO_PAGE_SZ 			256

#endif

