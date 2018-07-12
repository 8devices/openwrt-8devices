/*
 *  Header file of 8192D register
 *
 *	 $Id: 8192d_reg.h,v 1.2 2010/05/07 14:29:47 victoryman Exp $
 *
 *  Copyright (c) 2010 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192D_REG_H_
#define _8192D_REG_H_

//============================================================
//       8192D Regsiter offset definition
//============================================================


/*
 *	Project RTL8192d follows most of registers in Project RTL8192c
 *	This file includes additional registers for RTL8192d only
 *	Header file of RTL8192C registers should always be included
 */


/*
 *	Register 0x2C overlaps Project RTL8192c
 */
#define		MAC_PHY_CTRL_T					0x2C	// DMDP,SMSP,DMSP contrl
#define		MAC_PHY_CTRL_MP					0xF8	// DMDP,SMSP,DMSP contrl

#define		TXDMA_DBG						0x218	// TXDMA debug port
#define		BIST_START_PAUSE				0x240
#define		D_BIST_RDY						0x244
#define		D_BIST_FAIL					0x248
#define		D_BIST_RPT						0x24C	// BIST Report
#define		MAC_SEL						0x250	// Mac Select
#define		RXDMA_DBG						0x28C	// RXDMA debug port
#define		BT_COEX_GLB_CTRL				0x2C0	// Bt-Coexistence Control
#define		BT_COEX_TBL					0x2C4	// BT-Coexistence Arbiter Decision Table
#define		BT_SSI							0x2D4	// Bt Signal Strength Index
#define		BT_COEXT_CSR_ENH_INTF_CTRL	0x2D6	// Bt-Coexistence CSR Enhanced Interface Control
#define		BT_ACT_STATS					0x2D8	// Bt Activity Statistics
#define		BT_CMD_WLAN_RPT				0x2E0	// Bt Command Report from WLAN
#define		BT_CMD_BT_RPT					0x2E1	// Bt Command Report from BT
#define		BT_CMD_LATCH					0x2E2	// Bt Command Data Latched

#define		EARLY_MODE_CTRL				0x4D0	// Early mode control
#define		OTHERMAC_TBTTBK_CRTL			0x4D8	// Dis tbttbk othermac
#define		PTCL_PKT_NUM					0x4E3	// Pkt nums in protocol function
#define		PTCL_DBG						0x4E4	// Ptcl debug port
#define		DROP_PKT_NUM					0x4E8	// Tx drop pkt num
#define		LIFT_DROP_NUM					0x4EA	// Lift drop pkt num

#define		Pkt_Lifetime_CTRL				0x528	// Packet Lifetime Control
#define		SCH_TXCMD_NOA					0x5CF	// Tx_Cmd_Sel and Noa_Sel
#define		SCH_DBG						0x5DC	// Scheduler Debug Port
#define		DMR								0x5F0	// Dual MAC Co-Existence
#define		D_SCH_TXCMD					0x5F8	// TX_CMD Register

/*
 *	Register 0x6C0-6CF overlaps Project RTL8192c
 */
#define		WL_PRI_MASK					0x6C0	// WLAN Activity Priority Mask
#define		BT_COEX_CTRL				0x6C4	// BT-Coexistence Control


#define		MACID1						0x700	// MAC ID1 Register (Offset 0700h~0705h)

#define		SW_AES_REG					0x750	// Sw Access Aes Engine Data in/Data out/Security key Register
#define		SW_AES_CONF					0x760	// Sw Access Aes Engine configure
#define		WLAN_ACT_MSK_CTRL			0x768
#define		WMAC_DBG					0x77C	// WMAC DBG

//----------------------------------------------------------------------------
//       8192D DUALMAC_DUALPHY special bits
//----------------------------------------------------------------------------
#define		RSV_MAC0_CTRL				0x081	// bit-7 for mac0 enable
#define		RSV_MAC1_CTRL				0x053	// bit-0 for mac1 enable
#define		MAC0_EN			BIT(7)	// 1 for MAC0 enable
#define		BAND_STAT		BIT(1)	// 0:5G; 1:2G
#define		MAC1_EN			BIT(0)	// 1 for MAC1 enable
#define		RSV_MAC0_FWCTRL				0x018	// bit-0 for MAC0_WINTINI_RDY
#define		RSV_MAC1_FWCTRL				0x01a	// bit-0 for MAC1_WINTINI_RDY
#define		MAC0_WINTINI_RDY	BIT(0)	// 1: init ready
#define		MAC1_WINTINI_RDY	BIT(0)	// 1: init ready


//----------------------------------------------------------------------------
//       8192D RF_CTRL bits					(Offset 0x1F, 8 bits)
//----------------------------------------------------------------------------
#define		FW_DL_INPROC	BIT(5)	// 1 for firmware download inprogress

//----------------------------------------------------------------------------
//       8192D MAC_PHY_CTRL bits			(Offset 0x2C, 8 bits)
//----------------------------------------------------------------------------
#define		VAUX_EN			BIT(4)	// 1 for the system supporting aux power
#define		R_SINGLE_FUNC	BIT(3)	// 1 to force PCIE in single function mode
#define		SUPER_MAC_EN	BIT(2)	// 1 to enable super mac mode
#define		DUAL_PHY_EN		BIT(1)	// 1 to enable dual phy mode
#define		DUAL_MAC_EN		BIT(0)	// 1 to enable dual mac mode



//----------------------------------------------------------------------------
//       8192D LEDCFG bits						(Offset 0x4C-4F, 32 bits)
//----------------------------------------------------------------------------

#define		LED2DIS_92D				BIT(23)	// LED2 Disabled for analog signal usage,
										// 1: disable (input mode), 0:Enable (output mode).
#define		LED2SV_92D				BIT(19)	// LED2 software value.
#define		LED1DIS_92D				BIT(15)	// LED1 Disabled for analog signal usage,
										// 1: disable (input mode), 0:Enable (output mode).
#define		LED1SV_92D				BIT(11)	// LED1 software value.
#define		LED0DIS_92D				BIT(7)	// LED0 Disabled for analog signal usage,
										// 1: disable (input mode), 0:Enable (output mode).
#define		LED0SV_92D				BIT(3)	// LED0 software value.
#define		LED1CM_SHIFT_92D		16



//----------------------------------------------------------------------------
//       8192D BIST_START_PAUSE bits		(Offset 0x240-243, 32 bits)
//----------------------------------------------------------------------------
#define		BSP_PCIE_PDATASRAM_REAL	BIT(30)
#define		BSP_PCIE_PHDRSRAM0		BIT(29)
#define		TXDMA_BSP_MAC1			BIT(28)
#define		TXLLT_BSP_MAC1			BIT(27)
#define		TXOQT_BSP_MAC1			BIT(26)
#define		TXPKT_BSP_MAC1			BIT(25)
#define		RXPKT_BSP_MAC1			BIT(24)
#define		TXDBUF_BSP_MAC1			BIT(23)
#define		TXFIFO_BSP_MAC1			BIT(22)
#define		RXFIFO_BSP_MAC1			BIT(21)
#define		WOL_BSP_MAC1				BIT(20)
#define		KEY_BSP_MAC1				BIT(19)
#define		RXBA_BSP_MAC1				BIT(18)
#define		RC4_SBOX1_BSP_MAC1		BIT(17)
#define		RC4_SBOX0_BSP_MAC1		BIT(16)
#define		BSP_PCIE_PHDRSRAM1		BIT(15)
#define		BSP_PCIE_RTYRAM_REAL		BIT(14)
#define		BSP_PCIE_RTYSOTRAM		BIT(13)
#define		TXDMA_BSP_MAC0			BIT(12)
#define		TXLLT_BSP_MAC0			BIT(11)
#define		TXOQT_BSP_MAC0			BIT(10)
#define		TXPKT_BSP_MAC0			BIT(9)
#define		RXPKT_BSP_MAC0			BIT(8)
#define		TXDBUF_BSP_MAC0			BIT(7)
#define		TXFIFO_BSP_MAC0			BIT(6)
#define		RXFIFO_BSP_MAC0			BIT(5)
#define		WOL_BSP_MAC0				BIT(4)
#define		KEY_BSP_MAC0				BIT(3)
#define		RXBA_BSP_MAC0				BIT(2)
#define		RC4_SBOX1_BSP_MAC0		BIT(1)
#define		RC4_SBOX0_BSP_MAC0		BIT(0)

//----------------------------------------------------------------------------
//       8192D D_BIST_RDY bits				(Offset 0x244-247, 32 bits)
//----------------------------------------------------------------------------
#define		BD_PCIE_PDATASRAM_REAL	BIT(30)
#define		BD_PCIE_PHDRSRAM0		BIT(29)
#define		TXDMA_BD_MAC1			BIT(28)
#define		TXLLT_BD_MAC1				BIT(27)
#define		TXOQT_BD_MAC1				BIT(26)
#define		TXPKT_BD_MAC1				BIT(25)
#define		RXPKT_BD_MAC1				BIT(24)
#define		TXDBUF_BD_MAC1			BIT(23)
#define		TXFIFO_BD_MAC1			BIT(22)
#define		RXFIFO_BD_MAC1			BIT(21)
#define		WOL_BD_MAC1				BIT(20)
#define		KEY_BD_MAC1				BIT(19)
#define		RXBA_BD_MAC1				BIT(18)
#define		RC4_SBOX1_BD_MAC1		BIT(17)
#define		RC4_SBOX0_BD_MAC1		BIT(16)
#define		BD_PCIE_PHDRSRAM1		BIT(15)
#define		BD_PCIE_RTYRAM_REAL		BIT(14)
#define		BD_PCIE_RTYSOTRAM			BIT(13)
#define		TXDMA_BD_MAC0			BIT(12)
#define		TXLLT_BD_MAC0				BIT(11)
#define		TXOQT_BD_MAC0				BIT(10)
#define		TXPKT_BD_MAC0				BIT(9)
#define		RXPKT_BD_MAC0				BIT(8)
#define		TXDBUF_BD_MAC0			BIT(7)
#define		TXFIFO_BD_MAC0			BIT(6)
#define		RXFIFO_BD_MAC0			BIT(5)
#define		WOL_BD_MAC0				BIT(4)
#define		KEY_BD_MAC0				BIT(3)
#define		RXBA_BD_MAC0				BIT(2)
#define		RC4_SBOX1_BD_MAC0		BIT(1)
#define		RC4_SBOX0_BD_MAC0		BIT(0)

//----------------------------------------------------------------------------
//       8192D D_BIST_FAIL bits				(Offset 0x248-24B, 32 bits)
//----------------------------------------------------------------------------
#define		BF_PCIE_PDATASRAM_REAL	BIT(30)
#define		BF_PCIE_PHDRSRAM0			BIT(29)
#define		TXDMA_BF_MAC1				BIT(28)
#define		TXLLT_BF_MAC1				BIT(27)
#define		TXOQT_BF_MAC1				BIT(26)
#define		TXPKT_BF_MAC1				BIT(25)
#define		RXPKT_BF_MAC1				BIT(24)
#define		TXDBUF_BF_MAC1			BIT(23)
#define		TXFIFO_BF_MAC1			BIT(22)
#define		RXFIFO_BF_MAC1			BIT(21)
#define		WOL_BF_MAC1				BIT(20)
#define		KEY_BF_MAC1				BIT(19)
#define		RXBA_BF_MAC1				BIT(18)
#define		RC4_SBOX1_BF_MAC1		BIT(17)
#define		RC4_SBOX0_BF_MAC1		BIT(16)
#define		BF_PCIE_PHDRSRAM1			BIT(15)
#define		BF_PCIE_RTYRAM_REAL		BIT(14)
#define		BF_PCIE_RTYSOTRAM			BIT(13)
#define		TXDMA_BF_MAC0				BIT(12)
#define		TXLLT_BF_MAC0				BIT(11)
#define		TXOQT_BF_MAC0				BIT(10)
#define		TXPKT_BF_MAC0				BIT(9)
#define		RXPKT_BF_MAC0				BIT(8)
#define		TXDBUF_BF_MAC0			BIT(7)
#define		TXFIFO_BF_MAC0			BIT(6)
#define		RXFIFO_BF_MAC0			BIT(5)
#define		WOL_BF_MAC0				BIT(4)
#define		KEY_BF_MAC0				BIT(3)
#define		RXBA_BF_MAC0				BIT(2)
#define		RC4_SBOX1_BF_MAC0		BIT(1)
#define		RC4_SBOX0_BF_MAC0		BIT(0)

//----------------------------------------------------------------------------
//       8192D D_BIST_RPT bits				(Offset 0x24C-24F, 32 bits)
//----------------------------------------------------------------------------
#define		D_MAC_BIST_FAIL			BIT(31)	// At leaset one mac BIST FAIL
#define		D_USB_IRAM_FAIL			BIT(30)
#define		D_USB_RAM1_FAIL			BIT(29)
#define		D_USB_RAM2_FAIL			BIT(28)
#define		D_USB_PRAM_FAIL			BIT(27)
#define		D_USB_PROM_FAIL			BIT(26)
#define		D_USB_RXDMA_FAIL_MAC0	BIT(25)
#define		D_USB_RXDMA_FAIL_MAC1	BIT(24)
#define		D_PCIE_PD_BIST_FAIL		BIT(20)
#define		D_PCIE_PH0_BIST_FAIL		BIT(19)
#define		D_PCIE_PH1_BIST_FAIL		BIT(18)
#define		D_PCIE_RETRY_BIST_FAIL	BIT(17)
#define		D_PCIE_SOT_BIST_FAIL		BIT(16)
#define		D_MAC_BIST_RDY			BIT(10)
#define		D_USB_BIST_RDY			BIT(9)
#define		D_PCIE_BIST_RDY			BIT(8)
#define		D_MAC_BIST_START			BIT(2)
#define		D_USB_BIST_START			BIT(1)
#define		D_PCIE_BIST_START			BIT(0)

//----------------------------------------------------------------------------
//       8192D MAC_SEL bits					(Offset 0x250-253, 32 bits)
//----------------------------------------------------------------------------
#define		SIC_LBK_MAC_SEL		BIT(3)
#define		MAC0_TXRPT_SEL_8051	BIT(2)	// Mac0/1 tx report selection
#define		MAC0_SEL_MACPHY		BIT(1)	// Mac0/1 phy seletion
#define		R_MAC0_SEL_DBG		BIT(0)	// Mac0/1 debug port selection

//----------------------------------------------------------------------------
//       8192D BT_COEX_GLB_CTRL bits		(Offset 0x2C0-2C3, 32 bits)
//----------------------------------------------------------------------------
#define		ERR_CHK_TH_Shift	24		// In RTK 2wire mode, the interval of bt clock will be counted to check whether the communication functions well. This register indicates the check threshold.
#define		ERR_CHK_TH_Mask	0x0FF
#define		ARB_WIN_WL_Shift	16		// Arbitration window if WLAN device active first
#define		ARB_WIN_WL_Mask	0x0FF
#define		ARB_WIN_BT_Shift	8		// Arbitration window if bluetooth device active first
#define		ARB_WIN_BT_Mask	0x0FF
#define		CSR_2W				BIT(7)	// indicates whether the current operating mode is CSR 2-wire coexstence, which is only available when r_BT_MODE = 2'b10 and r_ENHBT = 1'b0.
#define		ANT_SEL_Shift		5		// 2 bits indicates the antenna usage of the wireless and bluetooth device, bit[0] for WL device, bit[1] for BT device 0: use ant a; 1: use ant b
#define		ANT_SEL_Mask		0x03
#define		WL_BAND_Shift		3		// Indicating the operating band of the chip, bit[0] for a path, bit[1] for b path; 0: 2.4G; 1: 5G
#define		WL_BAND_Mask		0x03
#define		STATIS_BT_RST		BIT(2)	// Reset BT_ACT_STATISTICS Counters. Write ¡§1¡¨ pulse.
#define		STATIS_BT_EN		BIT(1)	// bit is set, the BT_ACT_STATISTICS counters starts counting
#define		ENHBT				BIT(0)	// Used with r_BT_MODE to select Enhanced BT mode

//----------------------------------------------------------------------------
//       8192D BT_SSI bits					(Offset 0x2D4-2D5, 16 bits)
//----------------------------------------------------------------------------
#define		BT_TSSI_Shift		8		// BT tx signal strength index
#define		BT_TSSI_Mask		0x03F
#define		BT_RSSI_Shift		0		// BT rx signal strength index
#define		BT_RSSI_Mask		0x03F

//----------------------------------------------------------------------------
//       8192D BT_COEXT_CSR_ENH_INTF_CTRL bits		(Offset 0x2D6-2D7, 16 bits)
//----------------------------------------------------------------------------
#define		BT_TRX_DELAY_Shift			8		// When BT_PRI is high, if BT goes low from high, then BT will TX after BT_TX_DELAY time. WLAN can pause WLAN TX or RX after this delay to avoid impacting on BT. Unit: 8us
#define		BT_TRX_DELAY_Mask			0x0F
#define		BT_TRX_INIT_DETECT_Shift	4		// After BT_PRI asserting for BT_TRX_INIT_DETECT us, if BT_STAT is low, then BT will receive packets; otherwise, if BT goes high, BT will TX data. Unit 4us.
#define		BT_TRX_INIT_DETECT_Mask	0x0F
#define		BT_PRI_DETECT_TO_Shift		0		// After BT_PRI asserting, if BT_STAT is asserted within this TO duration, BT will initiate high priority activities; otherwise, if BT_STAT is not asserted within this duration, it would be low priority activities. Units 1us.
#define		BT_PRI_DETECT_TO_Mask	0x0F

//----------------------------------------------------------------------------
//       8192D BT_ACT_STATS bits			(Offset 0x2D8-2DF, 64 bits)
//----------------------------------------------------------------------------
#define		STATS_BT_LO_RX_Shift		16		// Counters for BT low priority RX. It counts up when STATIS_BT_EN is set. This counter will reset when STTIS_BT_RST is written by 1 pulse. This counter cannot wrap around when overflow occurs. Under overflow, this counter is kept with 0xFFFF.
#define		STATS_BT_LO_RX_Mask		0x0FFFF
#define		STATS_BT_LO_TX_Shift		0		// Counters for BT low priority TX. It counts up when STATIS_BT_EN is set. This counter will reset when STATIS_BT_RST is written by 1 pulse. This counter cannot wrap around when overflow occurs. Under overflow, this counter is kept with 0xFFFF.
#define		STATS_BT_LO_TX_Mask		0x0FFFF

#define		STATS_BT_HI_RX_Shift		16		// Counters for BT high priority RX. It counts up when STATIS_BT_EN is set. This counter will reset when STATIS_BT_RST is written by 1 pulse. This counter cannot wrap around when overflow occurs. Under overflow, this counter is kept with 0xFFFF.
#define		STATS_BT_HI_RX_Mask		0x0FFFF
#define		STATS_BT_HI_TX_Shift		0		// Counters for BT high priority TX. It counts up when STATIS_BT_EN is set. This counter will reset when STATIS_BT_RST is written by 1 pulse. This counter cannot wrap around when overflow occurs. Under overflow, this counter is kept with 0xFFFF.
#define		STATS_BT_HI_TX_Mask		0x0FFFF

//----------------------------------------------------------------------------
//       8192D BT_CMD_LATCH bits			(Offset 0x2E2-2E3, 16 bits)
//----------------------------------------------------------------------------
#define		BT_CMD_BT_STAT_Shift		8		// Latched data from BT_STAT after CMD pattern is matched.
#define		BT_CMD_BT_STAT_Mask		0x0FF
#define		BT_CMD_BT_PRI_Shift		0		// Latched data from BT_PRI after CMD pattern is matched.
#define		BT_CMD_BT_PRI_Mask		0x0FF

//----------------------------------------------------------------------------
//       8192D EARLY_MODE_CTRL bits			(Offset 0x4D0-4D3, 32 bits)
//----------------------------------------------------------------------------
#define		SINGLE_AMPDU_EN		BIT(31)	// Single pkt will be tx as single ampdu if enabled
#define		SINGLE_LEN_TH_Shift	8		// Early mode work on if (single ampdu > thd), default: 512B
#define		SINGLE_LEN_TH_Mask	0x07FF
#define		EARLY_MODE_EN_Shift	0		// Early mode enable for {BK, BE, VI, VO}
#define		EARLY_MODE_EN_Mask	0x0F

//----------------------------------------------------------------------------
//       8192D OTHERMAC_TBTTBK_CRTL bits	(Offset 0x4D8-4DB, 32 bits)
//----------------------------------------------------------------------------
#define		DIS_TBTTBK_OTHERMAC	BIT(0)	// Ptcl gen cmd will not cross the othermac's tbtt if set 0

//----------------------------------------------------------------------------
//       8192D Pkt_Lifetime_CTRL bits			(Offset 0x528-52A, 24 bits)
//----------------------------------------------------------------------------
#define		EN_NAVEND_RST_TXOP	BIT(17)	// set this bit, TXOP will be reset while NAV end.
#define		EN_FILTER_CCA			BIT(16)	// Enable CCA filter threshold
#define		CCA_FILTER_THRS_Shift	8		// CCA filter threshold value
#define		CCA_FILTER_THRS_Mask	0x0FF
#define		EDCCA_THRS_Shift		0		// EDCCA threshold value
#define		EDCCA_THRS_Mask		0x0FF

//----------------------------------------------------------------------------
//       8192D SCH_TXCMD_NOA bits			(Offset 0x5CF, 8 bits)
//----------------------------------------------------------------------------
#define		NOA_SEL				BIT(4)	// Noa1 and noa2 parameter select
#define		SCH_TXCMD_SEL_Shift	0		// Select to read which dword of tx_cmd
#define		SCH_TXCMD_SEL_Mask	0x0F

//----------------------------------------------------------------------------
//       8192D DMR bits						(Offset 0x5F0-5F1, 16 bits)
//----------------------------------------------------------------------------
#define		DMR_R_DIS_TXOVER_TBTT_OTHERMAC		BIT(8)	//set 1, the packet can¡¦t be sent out whose pkt and nav time will be over the tbtt of the other mac
#define		DMR_R_BLOCK_BY_SETBCN				BIT(7)	// tx backoff reset at tbtt of the other MAC in both DMDP and DMSP mode to prevent bcn tx blocked by the tx of the other mac
#define		DMR_R_BLOCK_BY_TXPTCLACT				BIT(6)	// tx blocked by txptcl_active of the other MAC in DMSP mode to protect the whole tx procedure including rx response
#define		DMR_R_BLOCK_BY_TXNAV					BIT(5)	// 1 to enable the co-existence mechanism for blocking Tx to protect rx response and NAV by TXNAV of another MAC in both DMDP and DMSP modes.
#define		DMR_R_BLOCK_BY_TXON					BIT(4)	// 1 to enable the co-existence mechanism for blocking Tx by another MAC Tx in both DMDP and DMSP modes.
#define		DMR_R_BLOCK_BY_RXNAV					BIT(3)	// 1 to enable the co-existence mechanism for blocking Tx to protect tx response and txop by RXNAV of another MAC in both DMDP and DMSP modes.
#define		DMR_R_BLOCK_BY_EDCCA					BIT(2)	// 1 to enable the co-existence mechanism for blocking Tx by EDCCA of another MAC in DMDP mode.
#define		DMR_R_BLOCK_BY_RXON					BIT(1)	// 1 to enable the co-existence mechanism for blocking Tx by another MAC Rx in DMDP mode
#define		DMR_R_INTFR_COEXIST_EN				BIT(0)	// 1 to enable dual MAC co-existence mechanism for avoiding RF interference in DMDP mode.

//----------------------------------------------------------------------------
//       8192D WL_PRI_MASK bits				(Offset 0x6C0-6C3, 32 bits)
//----------------------------------------------------------------------------
#define		PRI_MASK_WAITRESP	BIT(28)	// Priority Mask for RX Response Packet
#define		PRI_MASK_RXOFDM		BIT(27)	// Priority Mask for RX OFDM
#define		PRI_MASK_RXCCK		BIT(26)	// Priority Mask for RX CCK
#define		PRI_MASK_TXAC_Shift	19		// Priority Mask for Tx Queue
#define		PRI_MASK_TXAC_Mask	0x07F
#define		PRI_MASK_NAV_Shift		11		// Priority Mask for TX NAV
#define		PRI_MASK_NAV_Mask	0x0FF
#define		PRI_MASK_TXCCK		BIT(10)	// Priority Mask for TX CCK
#define		PRI_MASK_TXOFDM		BIT(9)	// Priority Mask for TX OFDM
#define		PRI_MASK_RTY			BIT(8)	// Priority Mask for Tx Retry Packet
#define		PRI_MASK_NUM_Shift		4		// Priority Mask for Tx packet number
#define		PRI_MASK_NUM_Mask	0x0F
#define		PRI_MASK_NEAR_TBTT	BIT(2)	// Priority Mask for near tbtt
#define		PRI_MASK_TX_RESP		BIT(1)	// Priority Mask for Tx Response Packet
#define		PRI_MASK_RX_RESP		BIT(0)	// Priority Mask for RX Response Packet

//----------------------------------------------------------------------------
//       8192D BT_COEX_CTRL bits			(Offset 0x6C4-6C7, 32 bits)
//----------------------------------------------------------------------------
#define		PROTECT_RX_RSP	BIT(11)	// When wlan tx packets, this bit decides whether WLAN_ACT to BT device will overprotect wlan active not be interfered until rx response frame finish.
#define		WLRX_TER_BY_CTL	BIT(10)	// When wlan receive a control packet without fit address, whether the signal indicating wlan rx can be terminated as soon as the address check finish is not only controlled by r_WLRX_TER_BY_AD, but also should be refered to this bit.
#define		WLRX_TER_BY_AD	BIT(9)	// When wlan receive the packet without fit address, the signal indicating wlan rx can be terminated as soon as the address check finish. This feature can be enabled by this bit.
#define		D_OOB				BIT(8)	// indicates the BT is out of the band of WLAN device. It's available only when r_BT_MODE is not the 2wire mode
#define		WL_CHNNL_Shift		2		// WLAN channel information only for 2.4G band
#define		WL_CHNNL_Mask		0x03F
#define		RX_ISO_OK			BIT(1)	// Isolation check result if OOB is true when WLAN is RX. 1: pass; 0: fail
#define		TX_ISO_OK			BIT(0)	// Isolation check result if OOB is true when WLAN is TX. 1: pass; 0: fail

//----------------------------------------------------------------------------
//       8192D SW_AES_CONF bits				(Offset 0x760, 8 bits)
//----------------------------------------------------------------------------
#define		SET_SWAES_REG			BIT(7)	// Set software aes engine request,to start aes engine calculation
#define		CLR_SWAES_REQ			BIT(6)	// Clear software aes engine request
#define		R_WMAC_SWAES_WE		BIT(3)	// Write enable of register ro_WMAC_SWAES_RD
#define		R_WMAC_SWAES_SEL		BIT(0)	// 1,the content of register 0750-075F used to be aes engine data in or data out; 0, the content of register 0750-075F uesed to be aes engine security key

//----------------------------------------------------------------------------
//       8192D WLAN_ACT_MSK_CTRL bits		(Offset 0x768-76E, 56 bits)
//----------------------------------------------------------------------------
#define		GNTALL_WL_MASK					BIT(19)	// bit indicates whether to grant all devices or bt device only when WLAN_ACT Mask active.
#define		WL_ACT_MASK_Enable				BIT(17)	// Enable WLAN_ACT Mask Function
//XXX
#define		WACTMSK_RX_RESP_ON_DUR_Shift	7
#define		WACTMSK_RX_RESP_ON_DUR_Mask	0x03

#define		WACTMSK_TX_DATA_OFF_DUR_Shift	24
#define		WACTMSK_TX_DATA_OFF_DUR_Mask	0x0FF
#define		WACTMSK_TX_DATA_ON_DUR_Shift	16
#define		WACTMSK_TX_DATA_ON_DUR_Mask	0x0FF
#define		WACTMSK_TX_RESP_OFF_DUR_Shift	8
#define		WACTMSK_TX_RESP_OFF_DUR_Mask	0x0FF
#define		WACTMSK_TX_RESP_ON_DUR_Shift	0
#define		WACTMSK_TX_RESP_ON_DUR_Mask	0x0FF


#if defined(CONFIG_RTL_92D_SUPPORT) && defined(EN_EFUSE)

#define EEPROM_MAC0_MACADDRESS			0x55 // MAC-0 MAC Address
#define EEPROM_MAC1_MACADDRESS			0x5B // MAC-1 MAC Address

#define EEPROM_2G_TxPowerCCK			0x61 // 2.4G CCK Tx Power base
#define	EEPROM_2G_TxPowerHT40_1S		0x67 // 2.4G HT40 Tx Power base
#define	EEPROM_2G_TxPowerHT40_2SDiff	0x6D // 2.4G HT40 Tx Power diff
#define EEPROM_2G_TxPowerHT20Diff		0x70 // 2.4G HT20 Tx Power diff
#define EEPROM_2G_TxPowerOFDMDiff		0x73 // 2.4G OFDM Tx Power diff

#define	EEPROM_5GL_TxPowerHT40_1S		0x7C // 5G Ch.36-44 HT40 Tx Power base
#define	EEPROM_5GL_TxPowerHT40_2SDiff	0x82 // 5G Ch.36-44 HT40 Tx Power diff
#define EEPROM_5GL_TxPowerHT20Diff		0x85 // 5G Ch.36-44 HT20 Tx Power diff
#define EEPROM_5GL_TxPowerOFDMDiff		0x88 // 5G Ch.36-44 OFDM Tx Power diff

#define	EEPROM_5GM_TxPowerHT40_1S		0x91 // 5G Ch.100-112 HT40 Tx Power base
#define	EEPROM_5GM_TxPowerHT40_2SDiff	0x97 // 5G Ch.100-112 HT40 Tx Power diff
#define EEPROM_5GM_TxPowerHT20Diff		0x9A // 5G Ch.100-112 HT20 Tx Power diff
#define EEPROM_5GM_TxPowerOFDMDiff		0x9D // 5G Ch.100-112 OFDM Tx Power diff

#define	EEPROM_5GH_TxPowerHT40_1S		0xA6 // 5G Ch.149-153 HT40 Tx Power base
#define	EEPROM_5GH_TxPowerHT40_2SDiff	0xAC // 5G Ch.149-153 HT40 Tx Power diff
#define EEPROM_5GH_TxPowerHT20Diff		0xAF // 5G Ch.149-153 HT20 Tx Power diff
#define EEPROM_5GH_TxPowerOFDMDiff		0xB2 // 5G Ch.149-153 OFDM Tx Power diff

#define EEPROM_92D_IQK_DELTA			0xBC
#define EEPROM_92D_LCK_DELTA			0xBC
#define EEPROM_92D_XTAL_K				0xBD //[7:0]
#define EEPROM_92D_TSSI_A_5G			0xBE
#define EEPROM_92D_TSSI_B_5G			0xBF
#define EEPROM_92D_TSSI_AB_5G			0xC0
#define EEPROM_92D_THERMAL_METER		0xC3 //[4:0]
#define EEPROM_92D_TRSW_CTRL			0xC9
#define EEPROM_92D_PAPE_CTRL			0xCC

#endif


#endif

