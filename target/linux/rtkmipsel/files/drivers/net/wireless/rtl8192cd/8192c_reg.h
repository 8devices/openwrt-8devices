/*
 *  Header file of 8192C register
 *
 *	 $Id: 8192c_reg.h,v 1.7.2.2 2010/09/20 09:00:10 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192C_REG_H_
#define _8192C_REG_H_

#ifndef WLAN_HAL_INTERNAL_USED


//============================================================
//       8192C Regsiter offset definition
//============================================================


//
// MAC register 0x000 - 0x6AFh
// 1. System Configure Register. (Offset 0x000 - 0x0FFh)
// 2. MACTOP General Configuration. (Offset 0x100 - 0x1FFh)
//		General Purpose Timer. (Offset 0x150 - 0x16Fh)
// 3. TXDMA/RXDMA Configuration. (Offset 0x200 - 0x2FFh)
// 4. PCIE EMAC Reserved Region. (Offset 0x300 - 0x3FFh)
// 5. PTCL/EDCA Configuration. (Offset 0x400 - 0x4FFh)
// 6. WMAC Configuration. (Offset 0x600 - 0x7FFh)
// 		MACID Setting Register. (Offset 0x610 - 0x62Fh)
// 		Timing Control Register. (Offset 0x630 - 0x64Fh)
// 		WMAC, BA, CCX Related Register. (Offset 0x650 - 0x66Fh)
// 		Security Control Register. (Offset 0x670 - 0x68Fh)
// 		Power Save Control Register. (Offset 0x690 - 0x69Fh)
// 		General Purpose Register. (Offset 0x6A0 - 0x6AFh)
//


//
// 1. System Configure Register. (Offset 0x000 - 0x0FFh)
//
#define		SYS_ISO_CTRL		0x000	// System Isolation Interface Control.
#define		SYS_FUNC_EN		0x002	// System Function Enable.
#define		APS_FSMCO			0x004	// Auto Power Sequence Control/Option.
#define		SYS_CLKR			0x008	// System Clock.
#define		CR9346				0x00A	// 93C46 (93C56) Command.
#define		EE_VPD				0x00C	// EEPROM VPD Data.
#define		AFE_MISC			0x010	// AFE Misc.
#define		SPS0_CTRL			0x011	// Switching Power Supply 0 Control.
#define		SPS_OCP_CFG		0x018	// SPS Over Current Protection Configuration.
#define		RSV_CTRL0			0x01C	// Reserved Control.
#define		RF_CTRL				0x01F	// RF Block Control.
#define		LDOA15_CTRL		0x020	// V15 Digital LDO Control.
#define		LDOV12D_CTRL		0x021	// V12 Digital LDO Control.
#define		LDOHCI12_CTRL		0x022	// V12 Digital LDO Control.
#define		LPLDO_CTRL			0x023	// Low Power LDO Control.
#define		AFE_XTAL_CTRL		0x024	// AFE Crystal Control.
#define		AFE_PLL_CTRL		0x028	// AFE_PLL_CTRL Enable.
#define		RSV_CTRL1			0x02C	// Reserved Control.
#define		EFUSE_CTRL			0x030	// E-Fuse Control.
#define		EFUSE_TEST			0x034	// E-Fuse Test Control.
#define		PWR_DATA			0x038	// Power on Data.
#define		CAL_TIMER			0x03C	// Hardware Calibration Timer.
#define		ACLK_MON			0x03E	// Analog Clock Monitor.
#define		GPIO_MUXCFG		0x040	// GPIO_MUX Configuration.
#define		GPIO_PIN_CTRL		0x044	// GPIO[7:0] PIN Control.
#define		GPIO_INTM			0x048	// GPIO Interrupt Monitor Register.
#define		LEDCFG				0x04C	// System PINMUX Configuration.
#define		FSIMR				0x050	// Firmware System Interrupt Mask.
#define		FSISR				0x054	// Firmware System Interrupt Status.
#define		HSIMR				0x058	// Host System Interrupt Mask.
#define		HSISR				0x05C	// Host System Interrupt Status.

#define		MCUFWDL			0x080	// MCU Firmware Down Load Control.
#define		MCUTSTCFG			0x084	// MCU Firmware Test Configuration.
#define		MCUDMSG_I			0x088	// MCU Firmware Debug Message.
#define		MCUDMSG_II			0x08C	// MCU Firmware Debug Message.

#define		BIST_CTRL			0x0D0  	// BIST/SCAN CONTROL
#define		C_BIST_RPT			0x0D4  	// BIST Report
#define		BIST_ROM_RPT		0x0D8  	// BIST ROM Report
#define		USB_SIE_INTF		0x0E0  	// USB SIE Access Interface
#define		PCIE_MIO_INTF		0x0E4  	// PCIE MIO Access Interface
#define		PCIE_MIO_INTD		0x0E8	// PCIE MIO Access Interface DATA.
#define		HPON_FSM			0x0EC	// Power On FSM Monitor/Control.
#define		SYS_CFG			0x0F0	// System Configuration Status


//
// 2. MACTOP General Configuration. (Offset 0x100 - 0x1FFh)
//
#define		CR					0x100  // Command Register.
#define		PBP					0x104  // Packet Buffer Page.
#define		TRXDMA_CTRL		0x10C  // TRXDMA Control.
#define		TRXFF_BNDY			0x114  // TX/RX FIFO Boundary
#define		TRXFF_STATUS		0x118  // PKTFF Status
#define		RXFF_PTR			0x11C  // RXFF Write/Read Pointer
#define		HIMR				0x120  // Host Interrupt Mask.
#define		HISR				0x124  // Host Interrupt Status.
#define		HIMRE				0x128  // Host Interrupt Mask Extension.
#define		HISRE				0x12C  // Host Interrupt Status Extension.
#define		CPWM				0x12F  // Current Power Mode.
#define		FWIMR				0x130  // Firmware WLAN Interrupt Mask.
#define		FWISR				0x134  // Firmware WLAN Interrupt Status.
#define		FTIMR				0x138  // Firmware Timer Interrupt Mask.
#define		FTISR				0x13C  // Firmware Timer Interrupt Status.
#define		PKTBUF_DBG_CTRL	0x140  // TX/RX Packet Buffer Debug Control.
#define		PKTBUF_DBG_DATA_L	0x144  // TX/RX Packet Buffer Debug Data.
#define		PKTBUF_DBG_DATA_H	0x148  // TX/RX Packet Buffer Debug Data.
//		General Purpose Timer. (Offset 0x150 - 0x16Fh)
#define		TC0_CTRL			0x150  // Timer/Counter 0 Control.
#define		TC1_CTRL			0x154  // Timer/Counter 1 Control.
#define		TC2_CTRL			0x158  // Timer/Counter 2 Control.
#define		TC3_CTRL			0x15C  // Timer/Counter 3 Control.
#define		TC4_CTRL			0x160	// Timer/Counter 4 Control.
#define		TCUNIT_BASE		0x164	// Timer/Counter Unit Base.
//		General Purpose Timer. (Offset 0x150 - 0x16Fh)
#define		MBIST_START		0x174	// MACTOP BIST START Pulse.
#define		MBIST_DONE			0x178	// MACTOP BIST DONE.
#define		MBIST_FAIL			0x17C	// MACTOP BIST FAIL.

#define		C2H_SYNC_BYTE		0x1AF	// 8051 set to 0xFF, Driver set to 0x0 after read C2H content 
#define		MCUTST_I			0x1C0	// MCU Firmware TST-I.
#define		MCUTST_II			0x1C4	// MCU Firmware TST-II.
#define		MCU_UNLOCK		 0x01C7	// is used for Unlock MCU infinite-loop
#define		FMETHR				0x1C8	// Firmware Message Exchange to Host.
#define		HMETFR				0x1CC	// Host Message Interrupt to Firmware.
#define		HMEBOX_0	 		0x1D0	// Host Message BOX_0 to Firmware.
#define		HMEBOX_1			0x1D4	// Host Message BOX_1 to Firmware.
#define		HMEBOX_2			0x1D8	// Host Message BOX_2 to Firmware.
#define		HMEBOX_3			0x1DC	// Host Message BOX_3 to Firmware.
#define		LLT_INI				0x1E0	// LLT Entries Initial Indirect Access Control.
#define		BB_ACCEESS_CTRL	0x1E8	// BB Indirect Access Control.
#define		BB_ACCESS_DATA	0x1EC	// BaseBand Indirect Access Data.
#define		MREG_HCIDBG		0x1F0	// MAC REG for HCI debug.
#define		MREG_CPUDBG		0x1F4	// MAC REG for CPU debug.


//
// 3. TXDMA/RXDMA Configuration. (Offset 0x200 - 0x2FFh)
//
#define		RQPN				0x200	// Reserved Queue Page Number.
#define		FIFOPAGE			0x204	// Available FIFO Page Number.
#define		TDECTRL			0x208	// Tx DMA Engine Control.
#define		TXDMA_OFFSET_CHK	0x20C	// TXDMA Offset Check.
#define		TXDMA_STATUS		0x210	// TXDMA Status.
#define		RQPN_NPQ			0x214	// RQPN for Normal Queue Register

#define		RXDMA_AGG_PG_TH	0x280	// RXDMA AGG PG Threshold.
#define		RXPKT_NUM			0x284	// RX Packet Number.
#define		RXDMA_STATUS		0x288	// RXDMA Status.


//
// 4. PCIE EMAC Reserved Region. (Offset 0x300 - 0x3FFh)
//
#define		PCIE_CTRL_REG		0x300	// PCIE DMA Control.
#define		INT_MIG			0x304	// Interrupt Migration.
#define		BCNQ_DESA			0x308	// TX Beacon Descriptor Address.
#define		HQ_DESA			0x310	// TX High Queue Descriptor.
#define		MGQ_DESA			0x318	// TX Manage Queue Descriptor Address.
#define		VOQ_DESA			0x320	// TX VO Queue Descriptor Address.
#define		VIQ_DESA			0x328	// TX VI Queue Descriptor Address.
#define		BEQ_DESA			0x330	// TX BE Queue Descriptor Address.
#define		BKQ_DESA			0x338	// TX BK Queue Descriptor Address.
#define		RX_DESA			0x340	// RX Queue Descriptor Address.
#define		DBI					0x348	// Backdoor REG for Access Configuration SPACE.
#define		MDIO				0x354	// MDIO for Access PCIE PHY.
#define		DBG_SEL			0x360	// Debug Selection
#define		UART_CTRL			0x364	// UART Control.
#define		UART_TX_DESA		0x370	// UART TX Descriptor Address.
#define		UART_RX_DESA		0x378	// UART RX Descriptor Address.


//
// 5. PTCL/EDCA Configuration. (Offset 0x400 - 0x4FFh)
//
#define		VOQ_INFO			0x400	//
#define		VIQ_INFO			0x404	//
#define		BEQ_INFO			0x408	//
#define		BKQ_INFO			0x40C	//
#define		MGQ_INFO			0x410	//
#define		HIQ_INFO			0x414	//
#define		BCNQ_INFO			0x418	//
#define		TXPKT_EMPTY		0x41B	// TXPKTBUF Packet Empty.
#define		CPU_MGQ_INFO		0x41C	// TXPKTBUF Packet Empty.
#define		FWHW_TXQ_CTRL	0x420	// FWHW TX Queue Control.
#define		HWSEQ_CTRL		0x423	// HW Sequence Number Control.
#define		TXPKTBUF_BCNQ_BDNY	0x424	// BCNQ Boundary.
#define		TXPKTBUF_MGQ_BDNY	0x425	// MGQ Boundary.
#define		MULTI_BCNQ_EN		0x426	//
#define		MULTI_BCNQ_OFFSET		0x427	//
#define		SPEC_SIFS_A		0x428	//
#define		RL					0x42A	//
#define		DARFRC				0x430	// Data Auto Rate Fallback Retry Count.
#define		RARFRC				0x438	// Response Auto Rate Fallback Retry Count.
#define		RRSR				0x440	// Response Rate Set.
#define		ARFR0				0x444	// Data Auto Rate Fallback 0.
#define		ARFR1				0x448	// Auto Rate Fallback 1.
#define		ARFR2				0x44C	// Auto Rate Fallback 2.
#define		ARFR3				0x450	// Auto Rate Fallback 3.
#define		AGGLEN_LMT		0x458	// Aggregation Length.
#define		AMPDU_MIN_SPACE	0x45C	// AMPDU Min Space.
#define		TXPKTBUF_WMAC_LBK_BF_HD	0x45D	// LBK Buffer Head Page
#define		FAST_EDCA_CTRL	0x460	// Fast EDCA Mode.
#define		RD_RESP_PKT_TH	0x463	// RD Responder Packet Threshold.
#define		INIRTS_RATE_SEL	0x480	// Initial RTS Rate SEL.
#define		INIDATA_RATE_SEL	0x484	// Initial Data Rate SEL.
#define		POWER_STATUS		0x4A4	// POWER STATUS.
#define		POWER_STAGE1		0x4B4	// POWER STAGE1.
#define		POWER_STAGE2		0x4B8	// POWER STAGE2.---------------???
#define		PKT_LIFE_TIME		0x4C0	// PKT LIFT TIME.
#define		STBC_SETTING		0x4C4	// STBC.
#define		PROT_MODE_CTRL	0x4C8	// PROTECT MODE Control.
#define		BAR_MODE_CTRL		0x4CC	// BAR MODE Control.
#define		RA_TRY_RATE_AGG_LMT	0x4CF	// Rate Adaptive Try Rate Aggregation Limit.
#define		NQOS_SEQ			0x4DC	// Non-QoS SEQ Number.
#define		QOS_SEQ			0x4DE	// QoS SEQ Number.
#define		NEED_CPU_HANDLE	0x4E0	// Need CPU Handle.
#define		PTCL_ERR_STATUS	0x4E1	// Protocol Function Error Status.
#define		PKT_LOSE_RPT		0x4E2	// PKT Lose Report.
#define		Dummy				0x4FC	// Dummy.

#define		EDCA_VO_PARA		0x500	// EDCA Parameter.
#define		EDCA_VI_PARA		0x504	// EDCA Parameter.
#define		EDCA_BE_PARA		0x508	// EDCA Parameter.
#define		EDCA_BK_PARA		0x50C	// EDCA Parameter.
#define		BCNTCFG			0x510	// Beacon Time Configuration.
#define		PIFS				0x512	// PIFS.
#define		RDG_PIFS			0x513	// RDG PIFS.
#define		SIFS_CCK			0x514	// SIFS for CCK.
#define		SIFS_OFDM			0x516	// SIFS for OFDM.
#define		AGGR_BK_TIME		0x51A	// AGGR BREAK TIME.
#define		SLOT_TIME			0x51B	// Slot Time.
#define		TX_PTCL_CTRL		0x520	// TX Protocol Control.
#define		TXPAUSE			0x522	// Transmission Pause.
#define		DIS_TXREQ_CLR		0x523	// Disable TX Request Clear Function.
#define		RD_CTRL			0x524	// RD Control.
#define		REG_MBSSID_CTRL		0x526   // MBSSID  Control.

#define		TBTT_PROHIBIT		0x540	// TBTT Prohibit.
#define		RD_NAV_NXT		0x544	// RD NAV Protect Next Time.
#define		NAV_PROT_LEN		0x546	// NAV Protection Length.
#define		BCN_CTRL			0x550	// Bcnq Control.
#define		BCN_CTRL1			0x551
#define		USTIME_TSF			0x551	// US Time Tuning for TSF.
#define		MBID_NUM			0x552	// MBSSID Beacon Number.
#define		DUAL_TSF_RST		0x553
#define		MBSSID_BCN_SPACE	0x554	// MBSSID Beacon Space.
#define		DRVERLYINT			0x558	// Beacon Driver Early Interrupt.
#define		BCNDMATIM			0x559	// BCN DMA and ATIM INT Time.-----------------???
#define		ATIMWND			0x55A	// ATIM Window Time.
#define		BCN_MAX_ERR		0x55D	// BCN MAX ERROR.
#define		RXTSF_OFFSET_CCK	0x55E	// CCK BCN OFFSET.
#define		RXTSF_OFFSET_OFDM	0x55F	// OFDM BCN OFFSET.
#define		TSFTR				0x560	// TSF Timer.
#define		INIT_TSFTR			0x564	// TSF Timer Initial Value.
#define		TSFTR1				0x568	// TSF Timer1.
#define		PSTIMER			0x580	// PS TIMER and Timeout INT.
#define		TIMER0				0x584	// TIMER0 INT.
#define		TIMER1				0x588	// TIMER1 INT.
#define		ACMHWCTRL			0x5C0	// ACM HW Control.
#define		ACMRSTCTRL			0x5C1	// ACM RST.
#define		ACMAVG				0x5C2	// ACM Average.
#define		VO_ADMTIME		0x5C4	// VO Admission Time.
#define		VI_ADMTIME			0x5C6	// VI Admission Time.
#define		BE_ADMTIME		0x5C8	// BE Admission Time.
#define		EDCA_RANDOM_GEN	0x5CC	// EDCA Random Number Generator.
#define		C_SCH_TXCMD		0x5D0	// TX_CMD.
#define		NOA_PARAM			0x5E0	// P2P NoA Parameter


//
// 6. WMAC Configuration. (Offset 0x600 - 0x7FFh)
//
#define		APSD_CTRL			0x600	// APSD Control.
#define		BWOPMODE			0x603	// BW Operation Mode.
#define		TCR					0x604	// Transmission Configuration.
#define		RCR					0x608	// Receive Configuration.
#define		RX_PKT_LIMIT		0x60C	// RX Packet Length Limit.
#define		RX_DLK_TIME		0x60D	// RX Deadlock Time.
#define		RX_DRVINFO_SZ		0x60F	// RX Driver INFO Size Register.
//		MACID Setting Register. (Offset 0x610 - 0x62Fh)
#define		MACID				0x610	// MAC ID.
#define		BSSIDR				0x618	// BSSID.
#define		MAR					0x620	// Multicast Address.
#define		MBIDCAMCFG		0x628	// MBSSID CAM Configuration.
//		MACID Setting Register. (Offset 0x610 - 0x62Fh)

//		Timing Control Register. (Offset 0x630 - 0x64Fh)
#define		USTIME_EDCA		0x638	// US Time Tuning for EDCA.
#define		SPEC_SIFS_B		0x63A	// Specification SIFS.
#define		RESP_SIFS_CCK		0x63C	// Response SIFS for CCK.
#define		RESP_SIFS_OFDM	0x63E	// Response SIFS for OFDM.
#define		ACKTO				0x640	// ACK Timeout.
#define		CTS2TO				0x641	// CTS2 Timeout.
#define		EIFS				0x642	// EIFS.
//		Timing Control Register. (Offset 0x630 - 0x64Fh)

//		WMAC, BA, CCX Related Register. (Offset 0x650 - 0x66Fh)
#define		NAV_CTRL			0x650	// NAV Control.
#define		BACAMCMD			0x654	// Block ACK CAM Command.
#define		BACAMCONTENT 		0x658	// Block ACK CAM Content.
#define		LBDLY		 		0x660	// Loopback Delay.
#define		FWDLY		 		0x661	// FW Delay.
#define		RXERR_RPT	 		0x664	// RX Error Report.
#define		WMAC_TRXPTCL_CTL	0x668	// WMAC TX/RX Protocol Control.
//		WMAC, BA, CCX Related Register. (Offset 0x650 - 0x66Fh)

//		Security Control Register. (Offset 0x670 - 0x68Fh)
#define		CAMCMD				0x670	// CAM Command.
#define		CAMWRITE			0x674	// CAM Write.
#define		CAMREAD			0x678	// CAM Read.
#define		CAMDBG				0x67C	// CAM Debug.
#define		SECCFG				0x680	// Security Configuration.
//		Security Control Register. (Offset 0x670 - 0x68Fh)

//		Power Save Control Register. (Offset 0x690 - 0x69Fh)
#define		WOW_CTRL			0x690	// Wake On WLAN Control.
#define		PSSTATUS			0x691	// Power Save Status.
#define		PS_RX_INFO			0x692	// Power Save RX Information.
#define		LPNAV_CTRL			0x694	// Low Power NAV Control.
#define		WKFMCAM_CMD		0x698	// Wakeup Frame CAM Command.
#define		WKFMCAM_RWD		0x69C	// Wakeup Frame Read/Write Data.
//		Power Save Control Register. (Offset 0x690 - 0x69Fh)

//		General Purpose Register. (Offset 0x6A0 - 0x6AFh)
#define		RXFLTMAP0			0x6A0	// RX Filter Map Group 0.
#define		RXFLTMAP1			0x6A2	// RX Filter Map Group 1.
#define		RXFLTMAP2			0x6A4	// RX Filter Map Group 2.
#define		BCN_PSR_RPT		0x6A8	// Beacon Parser Report.
#define		CALB32K_CTRL		0x6AC	// Calibration 32K Control.
//		General Purpose Register. (Offset 0x6A0 - 0x6AFh)
#define		PKT_MON_CTRL		0x6B4	// Packet Monitor Control.
#define		BT_COEX_TABLE		0x6C0	// BT-Coexistence Control.
#define		WMAC_RESP_TXINFO	0x6D8	// Response TXINFO.

#define		MACID1				0x700	// MAC ID1 Register (Offset 0700h~0705h)
#define		BSSIDR1				0x708

//-----------------------------------------------------
//
//	0xFE00h ~ 0xFE55h	USB Configuration
//
//-----------------------------------------------------
#define REG_USB_INFO				0xFE17
#define REG_USB_SPECIAL_OPTION	0xFE55
#define REG_USB_DMA_AGG_TO		0xFE5B
#define REG_USB_AGG_TO				0xFE5C
#define REG_USB_AGG_TH				0xFE5D

#define REG_USB_VID					0xFE60
#define REG_USB_PID					0xFE62
#define REG_USB_OPTIONAL			0xFE64
#define REG_USB_CHIRP_K			0xFE65
#define REG_USB_PHY					0xFE66
#define REG_USB_MAC_ADDR			0xFE70

#define REG_USB_HRPWM				0xFE58
#define REG_USB_HCPWM				0xFE57

// For test chip
#define REG_TEST_USB_TXQS			0xFE48
#define REG_TEST_SIE_VID			0xFE60		// 0xFE60~0xFE61
#define REG_TEST_SIE_PID			0xFE62		// 0xFE62~0xFE63
#define REG_TEST_SIE_OPTIONAL		0xFE64
#define REG_TEST_SIE_CHIRP_K		0xFE65
#define REG_TEST_SIE_PHY			0xFE66		// 0xFE66~0xFE6B
#define REG_TEST_SIE_MAC_ADDR		0xFE70		// 0xFE70~0xFE75
#define REG_TEST_SIE_STRING		0xFE80		// 0xFE80~0xFEB9


// For normal chip
#define REG_NORMAL_SIE_VID			0xFE60		// 0xFE60~0xFE61
#define REG_NORMAL_SIE_PID			0xFE62		// 0xFE62~0xFE63
#define REG_NORMAL_SIE_OPTIONAL	0xFE64
#define REG_NORMAL_SIE_EP			0xFE65		// 0xFE65~0xFE67
#define REG_NORMAL_SIE_PHY		0xFE68		// 0xFE68~0xFE6B
#define REG_NORMAL_SIE_OPTIONAL2	0xFE6C
#define REG_NORMAL_SIE_GPS_EP		0xFE6D		// 0xFE6D, for RTL8723 only.
#define REG_NORMAL_SIE_MAC_ADDR	0xFE70		// 0xFE70~0xFE75
#define REG_NORMAL_SIE_STRING		0xFE80		// 0xFE80~0xFEDF



//============================================================================
//       8192C Regsiter Bit and Content definition
//============================================================================


//
// 1. System Configure Register. (Offset 0x000 - 0x0FFh)
//
//----------------------------------------------------------------------------
//       8192C SYS_ISO_CTRL bits				(Offset 0x0, 16bit)
//----------------------------------------------------------------------------
#define		PWC_EV12V			BIT(15)	// Power Cut for eFuse 1.2V, 1: Power valid; 0: Power invalid.
#define		PWC_EV25V			BIT(14)	// Power Cut for eFuse 2.5V, 1: force PSW open; 0: PSW turn on by efuse ctrl.
#define		ISO_DIOR			BIT(9)	// RF Digital I/O to Power On, 1: isolation; 0: attach.
#define		ISO_EB2CORE		BIT(8)	// eFuse cell output to Power On, 1: isolation; 0: attach.
#define		ISO_DIOE			BIT(7)	// Extra Digital I/O to Power On, 1: isolation; 0: attach.
#define		ISO_DIOP			BIT(6)	// PCIe Digital I/O to Power On, 1: isolation; 0: attach.
#define		ISO_IP2MAC			BIT(5)	// Analog Ips to Digital 1.2V, 1: isolation; 0: attach (no used).
#define		ISO_PD2CORE		BIT(4)	// PCIe Digital 1.2V to Power On/Core 1.2V, 1: isolation; 0: attach.
#define		ISO_PA2PCIE		BIT(3)	// PCIe Analog 1.2V to PCIe 3.3V and Digital 1.2V, 1: isolation; 0: attach.
#define		ISO_UD2CORE		BIT(2)	// USB Digital 1.2V to Power On/Core 1.2V, 1: isolation; 0: attach.
#define		ISO_UA2USB			BIT(1)	// USB Analog 1.2V to USB 3.3V and Digital 1.2V, 1: isolation; 0: attach.
#define		ISO_MD2PP			BIT(0)	// MACTOP/BB/PCIe Digital to Power On, 1: isolation; 0: attach.

//----------------------------------------------------------------------------
//       8192C SYS_FUNC_EN bits					(Offset 0x2, 16bit)
//----------------------------------------------------------------------------
#define		FEN_MREGEN		BIT(15)	// MAC I/O Registers Enable.
#define		FEN_HWPDN			BIT(14)	// 0 : force All analog blocks shutdown, 1 : keep Analog Blocks alive.
#define		FEN_DIO_RF			BIT(13)	// Enable RF Digital I/O.
#define		FEN_ELDR			BIT(12)	// Enable EEPROM Loader (Loader POR).
#define		FEN_DCORE			BIT(11)	// enable Core Digital (MACTOP POR).
#define		FEN_CPUEN			BIT(10)	// Enable MCU Core (CPU RST).
#define		FEN_DIOE			BIT(9)	// Extra Debug I/O PAD Enable.
#define		FEN_PCIED			BIT(8)	// enable PCIe eMAC.
#define		FEN_PPLL			BIT(7)	// Enable PCIe PHY_PLL (no used).
#define		FEN_PCIEA			BIT(6)	// Enable PCIe PHY.
#define		FEN_DIO_PCIE		BIT(5)	// Enable PCIe Digital I/O (no used).
#define		FEN_USBD			BIT(4)	// Enable USB_SIE.
#define		FEN_UPLL			BIT(3)	// Enable USB PHY_PLL (no used).
#define		FEN_USBA			BIT(2)	// Enable USB PHY.
#define		FEN_BB_GLB_RST	BIT(1)	// When this bit is set to "0", whole BB is reset. When this bit is set, BB is enabled.
#define		FEN_BBRSTB			BIT(0)	// When this bit is set to "0", CCK and OFDM are disabled,
										// and clock are gated. Otherwise, CCK and OFDM are enabled.

//----------------------------------------------------------------------------
//       8192C APS_FSMCO bits					(Offset 0x4, 32bit)
//----------------------------------------------------------------------------
#define		XOP_BTCK			BIT(31)	// BT Option for never turn off XTAL & SPS.
#define		SOP_A8M			BIT(30)	// Suspend option not to turn analog Clock Mode to 500K Hz, 1: keep 8M, 0: 500K.
#define		SOP_RCK			BIT(29)	// Suspend option not to turn off Hreg Clock (8M/500KHz), 1: clock on, 0: clock off.
#define		SOP_AMB			BIT(28)	// Suspend option not to turn off AFE MB, 1: MB on, 0: MB off.
#define		SOP_ABG			BIT(27)	// Suspend option not to turn off AFE BG, 1: BG on, 0: BG off.
#define		SOP_FUSE			BIT(26)	// Suspend option to turn off Efuse Power or reset Loader, 1: Fuse Pwr Off, 0: Fuse Pwr On.
#define		SOP_MRST			BIT(25)	// Suspend option not to turn off MAC Reset, 1: MAC not reset, 0: MaC reset.
#define		SOP_CRST			BIT(24)	// Susoend option not to turn off 8051 clock, 1: 8051 clock on, 0:8051 clock off.
#define		ROP_SPS			BIT(22)	// Resume option to skip turn On SPS.
#define		ROP_PWR			BIT(21)	// Resume option to skip wait Power Ready, 1: pwr up no delay, 0: wait pwr stable.
#define		ROP_ALD			BIT(20)	// Resume option to skip autoload, 1: not autoload, 0: autoload.
#define		OP_SWRST			BIT(19)	// Option for Software Reset to Reset 8051.
#define		SUS_HOST			BIT(17)	// Power FSM turn to HOST SUS.
#define		RDY_MACON			BIT(16)	// Power FSM turn all power, clock ready for MAC.
#define		APDM_HPDN			BIT(15)	// Auto Power Down to CHIP-off State (Power-Down).
#define		APDM_HOST			BIT(14)	// Auto Power Down to HOST-off State (Card-Disable).
#define		APDM_MAC			BIT(13)	// Auto Power Down to WLAN-Off State (Radio-off).
#define		AFSM_PCIE			BIT(12)	// When this bit is set, PCIE suspends via HW FSM control flow.
#define		AFSM_HSUS			BIT(11)	// 1: Host Suspend through FSM operation; 0: Host Suspend by gated.
#define		APFM_RSM			BIT(10)	// Auto Power On State Machine Resume, auto clear when EE load done.
#define		APFM_OFF			BIT(9)	// Auto FSM to Turn Off, include clock, isolation, power control.
#define		APFM_ONMAC		BIT(8)	// Auto FSM to Turn On, include clock, isolation, power control for MAC only.
#define		BT_SUSEN			BIT(7)	// Enable BT suspend control.
#define		RDY_MACDIS			BIT(6)	// Disable MAC power Ready Output for Test Mode.
#define		PDN_PL				BIT(5)	// PDn polarity control, 0: Low active, 1: High active.
#define		PDN_EN				BIT(4)	// Enable GPIO-11 as Hardware PowerDown/Resume Source.
#define		PFM_WOWL			BIT(3)	// Wake On Lan indicator for H/W FSM.
#define		PFM_LDKP			BIT(2)	// Loader Initial data when re-enable loader.
#define		PFM_ALDN			BIT(1)	// HCI Region load done, inform HCI block.
#define		PFM_LDALL			BIT(0)	// EEPROM autoload 0x00~60h Enable.

//----------------------------------------------------------------------------
//       8192C SYS_CLKR bits					(Offset 0x8, 16bit)
//----------------------------------------------------------------------------
#define		RING_CLK_EN					BIT(13)	// HPON Ring Clock Enable, 1: Enable; 0 : Disable.
#define		SYS_CLK_EN						BIT(12)	// MCU Clock Enable, 1: Enable; 0 : Disable.
#define		MAC_CLK_EN						BIT(11)	// MAC Clock Enable, 1: Enable, 0 : Disable.
#define		SEC_CLK_EN						BIT(10)	// SEC Clock Enable, 1 :Enable, 0 :Disable.
#define		PHY_SSC_RSTB					BIT(9)	// PHY SSC Enable (0: Disable, 1: Enable).
#define		CLKR_80M_SSC_EN_HOLD_PHS0	BIT(8)	// 80M Clock Hold phase 0.
#define		CLKR_80M_SSC_DIS				BIT(7)	// 80M Clock SSC Disable.
#define		LOADER_CLK_EN					BIT(5)	// Loader (eFuse/EEPROM) Clock Enable.
#define		MACSLP							BIT(4)	// MAC SLEEP.
#define		ROMD16V_EN					BIT(2)	// Analog Clock source Divide 16 Mode for EEPROM.
#define		ANA8M							BIT(1)	// Analog Clock source rate, 1: 8M Hz, 0: 500K Hz.
#define		CNTD16V_EN						BIT(0)	// Analog Clock source Divide 16 Mode for Hardware Timer.

//----------------------------------------------------------------------------
//       8192C CR9346 bits						(Offset 0xA, 16bit)
//----------------------------------------------------------------------------
#define		VPDIDX_SHIFT		8	// VPD mode, EEPROM Index.
#define		VPDIDX_Mask		0x0FF
#define		EEM1_0_SHIFT		6	// Operating Mode.
#define		EEM1_0_Mask		0x03
#define		EEPROM_EN			BIT(5)	// EEPROM/Efuse is found when set to 1.
#define		EERPOMSEL			BIT(4)	// System EEPROM select (power on selected by GPIO-0).
#define		EECS				BIT(3)	// reflect the state of the EECS.
#define		EESK				BIT(2)	// reflect the state of the EESK.
#define		EEDI				BIT(1)	// reflect the state of the EEDI.
#define		EEDO				BIT(0)	// reflect the state of the EEDO.


#ifdef EN_EFUSE
#define		CmdEEPROM_En						BIT(5)	 // EEPROM enable when set 1
#define		CmdEERPOMSEL						BIT(4) // System EEPROM select, 0: boot from E-FUSE, 1: The EEPROM used is 9346
#define		AutoLoadEEPROM						(CmdEEPROM_En|CmdEERPOMSEL)
#define		AutoLoadEFUSE						CmdEEPROM_En


#ifdef CONFIG_RTL_92C_SUPPORT

#ifdef CONFIG_PCI_HCI
#define EEPROM_VID					0x0A // SE Vendor ID.A-B
#define EEPROM_DID					0x0C // SE Device ID. C-D
#define EEPROM_SVID					0x0E // SE Vendor ID.E-F
#define EEPROM_SMID					0x10 // SE PCI Subsystem ID. 10-11
#endif
#define EEPROM_TxPowerCCK			0x5A // CCK Tx Power base
#define EEPROM_TxPowerHT40_1S		0x60 // HT40 Tx Power base
#define EEPROM_TxPowerHT40_2SDiff	0x66 // HT40 Tx Power diff
#define EEPROM_TxPowerHT20Diff		0x69 // HT20 Tx Power diff
#define EEPROM_TxPowerOFDMDiff		0x6C // OFDM Tx Power diff
#define EEPROM_MACADDRESS			0x16

#define EEPROM_CHANNEL_PLAN		0x75
#define EEPROM_TSSI_A				0x76
#define EEPROM_TSSI_B				0x77
#define EEPROM_THERMAL_METER		0x78 //[4:0]
#define EEPROM_RF_OPT1				0x79
#define EEPROM_RF_OPT2				0x7A
#define EEPROM_RF_OPT3				0x7B
#define EEPROM_RF_OPT4				0x7C
#define EEPROM_VERSION				0x7E
#define EEPROM_CUSTOMER_ID			0x7F

#define EEPROM_NORMAL_BoardType	EEPROM_RF_OPT1	//[7:5]

#ifdef CONFIG_USB_HCI

#define EEPROM_VID								0x0A
#define EEPROM_PID								0x0C
#define EEPROM_SUBCUSTOMER_ID					0x59

//should be renamed and moved to another file
typedef enum _BOARD_TYPE_8192CUSB{
	BOARD_USB_DONGLE 			= 0,		// USB dongle
	BOARD_USB_High_PA 		= 1,		// USB dongle with high power PA
	BOARD_MINICARD		  	= 2,		// Minicard
	BOARD_USB_SOLO 		 	= 3,		// USB solo-Slim module
	BOARD_USB_COMBO			= 4,		// USB Combo-Slim module
} BOARD_TYPE_8192CUSB, *PBOARD_TYPE_8192CUSB;

#endif // CONFIG_USB_HCI
#endif // CONFIG_RTL_92C_SUPPORT

#ifdef CONFIG_USB_HCI
#define BOARD_TYPE_NORMAL_MASK		0xE0

#define EEPROM_Default_PID				0x1234
#define EEPROM_Default_VID				0x5678
#define EEPROM_Default_CustomerID			0xAB
#define EEPROM_Default_SubCustomerID		0xCD
#endif // CONFIG_USB_HCI

#endif // EN_EFUSE


//----------------------------------------------------------------------------
//       8192S AFE_MISC bits						(Offset 0x10, 8bits)
//----------------------------------------------------------------------------
#define		MAC_ID_EN			BIT(7)	// Reserved for RF_ID selection to be MAC.
#define		AFE_MBEN			BIT(1)	// Enable AFE Macro Block's Mbias.
#define		AFE_BGEN			BIT(0)	// Enable AFE Macro Block's Bandgap.

//----------------------------------------------------------------------------
//       8192C SPS0_CTRL bits					(Offset 0x11-17, 56bits)
//----------------------------------------------------------------------------
#define		SW18_LDO_R			BIT(19)	// LDO_R_L1 = 1 => LDO resistor connect.
#define		SW18_V15ADJ_SHIFT		16	// v15adj_L1<2:0>  output.
#define		SW18_V15ADJ_Mask		0x07
#define		SW18_VOL_SHIFT		12	// VOL_L1<3:0>  output.
#define		SW18_VOL_Mask			0x0F
#define		SW18_IN_SHIFT			9	// IN_L1<2:0> output.
#define		SW18_IN_Mask			0x07
#define		SW18_TBOX_SHIFT		7	// TBOC_L1<1:0> output.
#define		SW18_TBOX_Mask		0x03
#define		SW18_SET_DELAY		BIT(6)	// SET_DELAY_L1=1=> nonoverlap delay increases.
#define		SW18_SEL				BIT(5)	// SD_L1 = 1 => turn off quickly.
#define		SW18_STD_SHIFT		3	// STD_L1<1:0> time.
#define		SW18_STD_Mask			0x03
#define		SW18_SD				BIT(2)	// SD_L1 = 1 => turn off quickly.
#define		SW18_AREN				BIT(1)	// 3.3->1.8 for PFM  Anti-ring.
#define		SW18_R3_B1			BIT(0)	//

#define		SW18_R3_B0			BIT(31)	//
#define		SW18_R2_SHIFT			29	//
#define		SW18_R2_Mask			0x03
#define		SW18_R1_SHIFT			27	//
#define		SW18_R1_Mask			0x03
#define		SW18_C3_SHIFT			25	//
#define		SW18_C3_Mask			0x03
#define		SW18_C2_SHIFT			23	//
#define		SW18_C2_Mask			0x03
#define		SW18_C1_SHIFT			21	//
#define		SW18_C1_Mask			0x03
#define		SW18_SCREF_SHIFT		19	// For FDUAL, tune reference frequency.
#define		SW18_SCREF_Mask		0x03
#define		SW18_SCPFM_SHIFT		17	// For FDUAL, tune pfm frequency.
#define		SW18_SCPFM_Mask		0x03
#define		SW18_PWFMTUNE_SHIFT	14	//
#define		SW18_PWFMTUNE_Mask	0x07
#define		SW18_SELSTOP			BIT(13)	// SEL_STOP_L1 = 0 => no delay.
#define		SW18_SELD_SHIFT		11	// Bit(1)=1 enable clamp maximum duty, bit(0)=1 enable clamp minimum duty
#define		SW18_SELD_Mask		0x03
#define		SW18_POWOCP			BIT(10)	// POWOCP_L1=1=> enable over current protection.
#define		SW18_OCP_SHIFT		7	//
#define		SW18_OCP_Mask			0x07
#define		SW18_SFREQ_SHIFT		5	//
#define		SW18_SFREQ_Mask		0x03
#define		SW18_SFREQC			BIT(4)	//
#define		SW18_FPWM				BIT(3)	// FPWM_L1 = 1 => force PWM, FPWM_L1 = 0 => force PFM.
#define		SW18_FDUAL			BIT(2)	// FDUAL_L1 =1 => PWM or PFM.
#define		SW18_SWEN				BIT(1)	// Enable VSPS18_SW Macro Block.
#define		SW18_LDEN				BIT(0)	// Enable VSPS18_LDO Macro Block.

//----------------------------------------------------------------------------
//       8192C SPS_OCP_CFG bits					(Offset 0x18-1B, 32bits)
//----------------------------------------------------------------------------
#define		SPS18_OCP_DIS			BIT(31)	// sps18 ocp, 0:enable, 1:disable.
#define		SPS18_OCP_TH_SHIFT	16	// Sps18 ocp threshold.
#define		SPS18_OCP_TH_Mask		0x07FFF
#define		OCP_WINDOW_SHIFT		0	// Ocp monitor window width configure, count on ana_clk/16.
#define		OCP_WINDOW_Mask		0x0FFFF

//----------------------------------------------------------------------------
//       8192C RSV_CTRL0 bits					(Offset 0x1C-1E, 24bits)
//----------------------------------------------------------------------------
#define		HREG_DBG_SHIFT	8	// HREG Access Debug.
#define		HREG_DBG_Mask		0x0FFFF
#define		LOCK_ALL_EN		BIT(7)	// 1:Enable 'Locked All' setting by Hardware, 0: Hardware Enable 'Locked All' denied.
#define		R_DIS_PRST_1		BIT(6)	// Control if PERST can reset MACCORE.
#define		R_DIS_PRST_0		BIT(5)	// Control if PERST can reset MACCORE.
#define		WLOCK_40			BIT(4)	// 1:Register Write Locked, 0: Register Accessible by SW/FW.
#define		WLOCK_08			BIT(3)	// 1:Register Write Locked, 0: Register Accessible by SW/FW.
#define		WLOCK_04			BIT(2)	// 1:Register Write Locked, 0: Register Accessible by SW/FW.
#define		WLOCK_00			BIT(1)	// 1:Register Write Locked, 0: Register Accessible by SW/FW.
#define		WLOCK_ALL			BIT(0)	// 1:Register Write Locked, 0: Register Accessible by SW/FW.

//----------------------------------------------------------------------------
//       8192C RF_CTRL bits						(Offset 0x1F, 8bits)
//----------------------------------------------------------------------------
#define		RF_SDMRSTB			BIT(2)	// Reset RF SDM Module (low active).
#define		RF_RSTB			BIT(1)	// Reset RF Module (low active).
#define		RF_EN				BIT(0)	// Enable RF Module.

//----------------------------------------------------------------------------
//       8192C LDOA15_CTRL bits					(Offset 0x20, 8bits)
//----------------------------------------------------------------------------
#define		LDA15_VOADJ_SHIFT	4	//
#define		LDA15_VOADJ_Mask	0x0F
#define		LDA15_STBY			BIT(3)	// LDOA15 Standby Mode.
#define		LDA15_OBUF			BIT(2)	// LDOA15 Output Buffered.
#define		REG_VOS			BIT(1)	// LDOA15 Add Offset voltage.
#define		LDA15_EN			BIT(0)	// Enable LDOA15 Macro Block.

//----------------------------------------------------------------------------
//       8192C LDOV12D_CTRL bits				(Offset 0x21, 8bits)
//----------------------------------------------------------------------------
#define		LDV12_VADJ_SHIFT	4	// LDOVD12 voltage adjust.
#define		LDV12_VADJ_Mask	0x0F
#define		LDV12_BUFO			BIT(2)	// LDOVD12 buffer output Mode.
#define		LDV12_SDBY			BIT(1)	// LDOVD12 standby mode.
#define		LDV12_EN			BIT(0)	// Enable LDOA12 Macro Block.

//----------------------------------------------------------------------------
//       8192C LDOHCI12_CTRL bits				(Offset 0x22, 8bits)
//----------------------------------------------------------------------------
#define		LDH12_SDBY			BIT(5)	// LDOHCI12 standby mode.
#define		LDH12_OBUF			BIT(4)	// Enable LDOHCI12 Macro Block.
#define		LDH12_VADJ_SHIFT	1	// LDOHCI12 voltage adjust.
#define		LDH12_VADJ_Mask	0x07
#define		LDH12_EN			BIT(0)	// Enable LDOHCI12 Macro Block.

//----------------------------------------------------------------------------
//       8192C LPLDO_CTRL bits					(Offset 0x23, 8bits)
//----------------------------------------------------------------------------
#define		LPLDH12_SLP			BIT(4)	// LPLDOH12 Sleep Mode.
#define		LPLDH12_VADJ_SHIFT	0	// LPLDOH12 voltage adjust.
#define		LPLDH12_VADJ_Mask		0x0F

//----------------------------------------------------------------------------
//       8192C AFE_XTAL_CTRL bits				(Offset 0x24-27, 32bits)
//----------------------------------------------------------------------------
#define		CKDLY_BT				BIT(29)	// Delay time of CLK_BT relate to CLK_DIG, 0: long time, 1: short time.
#define		CKDLY_DIG				BIT(28)	// Delay time of CLK_DIG relate to CLK_AFE, 0: long time, 1: short time.
#define		CKDLY_USB				BIT(27)	// Delay time of CLK_USB relate to CLK_AFE, 0: long time, 1: short time.
#define		CKDLY_AFE				BIT(26)	// Delay time of CLK_AFE relate to CLK_RF, 0: long time, 1: short time.
#define		XTAL_GPIO_SHIFT		23	//
#define		XTAL_GPIO_Mask		0x07
#define		XTAL_BT_DRV_SHIFT		21	// BT clock source Driving current: 11:most current, 00: less current.
#define		XTAL_BT_DRV_Mask		0x03
#define		XTAL_BT_GATE			BIT(20)	// Gated control: 1: BT Clock source gated, 0: clock enable (auto setting by trap data).
#define		XTAL_DIG_DRV_SHIFT	18	// DIG clock source 11:most current, 00: less current.
#define		XTAL_DIG_DRV_Mask		0x03
#define		XTAL_GATE_DIG			BIT(17)	// Gated control:1: DIG Clock source gated, 0: clock enable.
#define		XTAL_RF_DRV_SHIFT		15	// RF PLL Driving current: 11:most current, 00: less current.
#define		XTAL_RF_DRV_Mask		0x03
#define		XTAL_RF_GATE			BIT(14)	// Gated control: 1: RF Clock source gated, 0: clock enable.
#define		XTAL_AFE_DRV_SHIFT	12	// AFE PLL Driving current: 11:most current, 00: less current.
#define		XTAL_AFE_DRV_Mask		0x03
#define		XTAL_GATE_AFE			BIT(11)	// Gated control:1: USB Clock source gated, 0: clock enable.
#define		XTAL_USB_DRV_SHIFT	9	// USB PLL Driving current: 11:most current, 00: less current.
#define		XTAL_USB_DRV_Mask		0x03
#define		XTAL_GATE_USB			BIT(8)	// Gated control:1: USB Clock source gated, 0: clock enable.
#define		XTAL_CADJ_XI_SHIFT	4	// 1:add cap, 0:degrade cap.
#define		XTAL_CADJ_XI_Mask		0x0F
#define		XTAL_BOSC_SHIFT		2	// The bias current control of VCO block.
#define		XTAL_BOSC_Mask		0x03
#define		XTAL_BSEL				BIT(1)	// 1: schmitt trigger, 0:nand.
#define		XTAL_EN				BIT(0)	// Enable XTAL Macro block.

//----------------------------------------------------------------------------
//       8192C AFE_PLL_CTRL bits					(Offset 0x28-2B, 32bits)
//----------------------------------------------------------------------------
#define		AFE_DUMMY_SHIFT		25	// Dummy Registers.
#define		AFE_DUMMY_Mask		0x07
#define		APLL_1MEN				BIT(24)	// 1:Enable 0:output "1".
#define		APLL_40DRV_SHIFT		22	// clk diving 00: weak 11:strong.
#define		APLL_40DRV_Mask		0x03
#define		APLL_PLLDRV_SHIFT		20	// clk diving 00: weak 11:strong.
#define		APLL_PLLDRV_Mask		0x03
#define		APLL_88DRV_SHIFT		18	// clk diving 00: weak 11:strong.
#define		APLL_88DRV_Mask		0x03
#define		APLL_80DRV_SHIFT		16	// clk diving 00: weak 11:strong.
#define		APLL_80DRV_Mask		0x03
#define		APLL_80EN				BIT(15)	// 1:Enable 0:output "1".
#define		APLL_320EN				BIT(14)	// 1:Enable 0:output "1".
#define		APLL_320BIAS_SHIFT	11	// pll320 CP current selection.
#define		APLL_320BIAS_Mask		0x07
#define		APLL_BIAS_SHIFT		8	// pll CP current selection.
#define		APLL_BIAS_Mask			0x07
#define		APLL_KVCO_SHIFT		6	// pll VCO KVCO selection.
#define		APLL_KVCO_Mask		0x03
#define		APLL_LPFEN				BIT(5)	// enable PLL Bias Current LPF.
#define		APLL_WDOGB			BIT(4)	// enable watch dog 0: enable 1:disable.
#define		APLL_EDGE_SEL			BIT(3)	// Reference clock: 1 : negative edge, 0 : positive edge.
#define		APLL_FREF_SEL			BIT(2)	// Reference clock: 1 : 20MHz, 0 : 40MHz.
#define		APLL_320_EN			BIT(1)	// Enable AFE 320 PLL Macro Block.
#define		APLL_EN				BIT(0)	// Enable AFE PLL Macro Block.

//----------------------------------------------------------------------------
//       8192C EFUSE_CTRL bits					(Offset 0x30-33, 32bits)
//----------------------------------------------------------------------------
#define		EF_FLAG			BIT(31)	// Access Flag. Write "1" for Program; Write "0" for Read Access.
#define		EF_PGPD_SHIFT		28	// E-fuse program time.
#define		EF_PGPD_Mask		0x07
#define		EF_RDT_SHIFT		24	// E-fuse read time: in the unit of cycle time.
#define		EF_RDT_Mask		0x0F
#define		EF_PGT_SHIFT		20	// Programming setup time. In the unit of cycle time.(125ns).
#define		EF_PGT_Mask		0x0F
#define		EF_PD				BIT(19)	// Efuse power down.
#define		ALD_EN				BIT(18)	// Autoload Enable.
#define		EF_ADDR_SHIFT		8	// Access Address.
#define		EF_ADDR_Mask		0x03FF
#define		EF_DATA_SHIFT		0	// Access Data.
#define		EF_DATA_Mask		0x0FF

//----------------------------------------------------------------------------
//       8192C EFUSE_TEST bits					(Offset 0x34-37, 32bits)
//----------------------------------------------------------------------------
#define		LDOE25_EN			BIT(31)	// Enable LDOE25 Macro Block.
#define		LDOE25_VADJ_SHIFT	28	// LDOE25 voltage adjust.
#define		LDOE25_VADJ_Mask	0x07
#define		ADDR_END_Shift		25	// 92D_REG, Scan effuse end (1023-addr_end)
#define		ADDR_END_Mask		0x07
#define		ADDR_STR_Shift		18	// 92D_REG, Scan efuse from (addr_str*8)
#define		ADDR_STR_Mask		0x07F
#define		BYTE_CNT_EN		BIT(17)	// 92D_REG, 1: scan efuse in byte; 0: scan efuse in bit
#define		EF_SCAN_EN			BIT(16)	// 92D_REG, 1: efuse in scan process. 0: not in scan process
#define		EDATA1_SHIFT		8	// EEPROM offset 1 data (EE Byte-1).
#define		EDATA1_Mask		0x0FF
#define		EF_TRPT			BIT(7)	// Test Scan Report: 1 : Fail, 0 : OK.
#define		EF_TTHD_SHIFT		0	// Bit number for all cells scan test.
#define		EF_TTHD_Mask		0x07F

//----------------------------------------------------------------------------
//       8192C PWR_DATA bits					(Offset 0x38-3B, 32bits)
//----------------------------------------------------------------------------
#define		EDATA0_SHIFT		24	// EEPROM offset 0 data (EE Byte-0).
#define		EDATA0_Mask		0x0FF
#define		HTEST_SEL_SHIFT	16	// TEST Mode Select (EE Byte-7).
#define		HTEST_SEL_Mask	0x0FF
#define		HPON_STBP2_SHIFT	8	// HPON Stable time parameter (EE Byte-5).
#define		HPON_STBP2_Mask	0x0FF
#define		HPON_STBP1_SHIFT	0	// HPON Stable time parameter (EE Byte-2).
#define		HPON_STBP1_Mask	0x0FF

//----------------------------------------------------------------------------
//       8192C CAL_TIMER bits					(Offset 0x3C-3D, 16bits)
//----------------------------------------------------------------------------
#define		MATCH_CNT_SHIFT	8	// match counter for sleep mode.
#define		MATCH_CNT_Mask	0x0FF
#define		CAL_SCAL_SHIFT		0	// scaler fine tune for sleep mode (tune in Time unit).
#define		CAL_SCAL_Mask		0x0FF

//----------------------------------------------------------------------------
//       8192C ACLK_MON bits					(Offset 0x3E-3F, 16bits)
//----------------------------------------------------------------------------
#define		RCLK_MON_SHIFT	5	// ring clock monitor (with DPSTU scale).
#define		RCLK_MON_Mask		0x07FF
#define		CAL_TIMER_EN		BIT(4)	// calibration timer enable. This shall be turned off before sleep mode enable.
#define		DPSTU_SHIFT		2	// Deep Power Save Time Unit: 00: 64us, 01: 128us, 10: 192us, 11: 256 us.
#define		DPSTU_Mask			0x03
#define		SUS_16X			BIT(1)	// Scale timer 16x when suspend mode, (~250 S).
#define		RSM_EN				BIT(0)	// When Timer Expired, Resume Hardware StateMachine.

//----------------------------------------------------------------------------
//       8192C GPIO_MUXCFG bits					(Offset 0x40-43, 32 bits)
//----------------------------------------------------------------------------
#define		GPIO_MOD_H_SHIFT		28	// When bit is zero, corresponding port configured to data port;
										// otherwise, when a bit is set, and GPIO_IO_SEL is set to "0",
										// then it is interrupt mode..
#define		GPIO_MOD_H_Mask		0x0F
#define		GPIO_IO_SEL_H_SHIFT	24	// GPIO_IO_SEL[11:8], output when bit=1;input when bit=0.
#define		GPIO_IO_SEL_H_Mask	0x0F
#define		GPIO_OUT_H_SHIFT		20	// GPIO_OUT[11:8], GPIO pins output value.
#define		GPIO_OUT_H_Mask		0x0F
#define		GPIO_IN_H_SHIFT		16	// GPIO_IN[11:8], GPIO pins input value.
#define		GPIO_IN_H_Mask		0x0F
#define		SIC_LBK					BIT(15)	// Enable SIC_Loop_Back Mode, (auto set by ICFG).
#define		HTP_EN					BIT(14)   // 0 : Enable as Host Debug Port, 1 : Enable as Host Test Port.
#define		SIC_23					BIT(13)   // Enable SIC at 0: GPIO 9,10 pins ; 1 : GPIO 2,3 pins.
#define		SIC_EN					BIT(12)   // Enable GPIO Pins as SIC, (auto set by ICFG).
#define		SIC_SWRST				BIT(11)	// 0 : Reset SIC by SW, prevent SIC Dead Lock.
#define		PMAC_EN				BIT(10)	// PMAC Enable (auto set by ICFG).
#define		UART_SEL				BIT(9)	// Enable UART at 0: GPIO 6,7 pins; 1 : GPIO 9,10 pins.
#define		UART_EN				BIT(8)	// Enable GPIO Pins as UART.
#define		BTMode_SHIFT			6	// Bluetooth Mode.
#define		BTMode_Mask			0x03
#define		BT_EN					BIT(5)	// Enable GPIO0,1,2,3,8 Pins for Bluetooth.
#define		EROM_EN				BIT(4)	// Enable GPIO6 for EEDO, GPIO7 for EEDI, default Enable when 9346 selected
#define		TRSW1EN				BIT(3)	// Enable GPIO9,10 for TRSWN1, TRSWP1.
#define		TRSW0EN				BIT(2)	// Enable GPIO8 for TRSWN0.
#define		GPIOSEL_SHIFT			0	// GPIO Select.
#define		GPIOSEL_Mask			0x03

//----------------------------------------------------------------------------
//       8192C GPIO_PIN_CTRL bits				(Offset 0x44-47, 32 bits)
//----------------------------------------------------------------------------
#define		GPIO_MOD_L_SHIFT		24	// When bit is 0, corresponding port set as data port;
										// otherwise, when a bit=1, and GPIO_IO_SEL is set to "0",
										// then it is interrupt mode..
#define		GPIO_MOD_L_Mask		0x0FF
#define		GPIO_IO_SEL_L_SHIFT	16	// GPIO_IO_SEL[7:0], output when bit=1;input when bit=0.
#define		GPIO_IO_SEL_L_Mask	0x0FF
#define		GPIO_OUT_L_SHIFT		8	// GPIO_OUT[7:0], GPIO pins output value.
#define		GPIO_OUT_L_Mask		0x0FF
#define		GPIO_IN_L_SHIFT		0	// GPIO_IN[7:0], GPIO pins input value.
#define		GPIO_IN_L_Mask			0x0FF

//----------------------------------------------------------------------------
//       8192C GPIO_INTM bits					(Offset 0x48-4B, 32 bits)
//----------------------------------------------------------------------------
#define		MDBG_SEL_SHIFT		30	//
#define		MDBG_SEL_Mask			0x03
#define		GPIO_EXT_WOL_SHIFT	12	// External WOL source control.
#define		GPIO_EXT_WOL_Mask	0x0F
#define		GPIOB_INT_MD			BIT(11)	// 0=positive edge, 1=negative edge,  is sensed as interrupt.
#define		GPIOA_INT_MD			BIT(10)	// 0=positive edge, 1=negative edge,  is sensed as interrupt.
#define		GPIO9_INT_MD			BIT(9)	// 0=positive edge, 1=negative edge,  is sensed as interrupt.
#define		GPIO8_INT_MD			BIT(8)	// 0=positive edge, 1=negative edge,  is sensed as interrupt.
#define		GPIO7_INT_MD			BIT(7)	// 0=positive edge, 1=negative edge,  is sensed as interrupt.
#define		GPIO6_INT_MD			BIT(6)	// 0=positive edge, 1=negative edge,  is sensed as interrupt.
#define		GPIO5_INT_MD			BIT(5)	// 0=positive edge, 1=negative edge,  is sensed as interrupt.
#define		GPIO4_INT_MD			BIT(4)	// 0=positive edge, 1=negative edge,  is sensed as interrupt.
#define		GPIO3_INT_MD			BIT(3)	// 0=positive edge, 1=negative edge,  is sensed as interrupt.
#define		GPIO2_INT_MD			BIT(2)	// 0=positive edge, 1=negative edge,  is sensed as interrupt.
#define		GPIO1_INT_MD			BIT(1)	// 0=positive edge, 1=negative edge,  is sensed as interrupt.
#define		GPIO0_INT_MD			BIT(0)	// 0=positive edge, 1=negative edge,  is sensed as interrupt.

//----------------------------------------------------------------------------
//       8192C LEDCFG bits						(Offset 0x4C-4F, 32 bits)
//----------------------------------------------------------------------------
#define		GP8_LED			BIT(22)	// Enable GPIO-8 as LED2.
#define		LED2EN				BIT(21)	// Enable GPIO-10/GPIO-8 as LED2.
#define		LED2PL				BIT(20)	// LED2 polarity control.
#define		LED2SV				BIT(19)	// LED2 software value.
#define		LED2CM_SHIFT		16	// LED2 Control Mode.
#define		LED2CM_Mask		0x07
#define		LED1DIS				BIT(15)	// LED1 Disabled for analog signal usage,
										// 1: disable (input mode), 0:Enable (output mode).
#define		LED1PL				BIT(12)	// LED1 polarity control (auto-load configurable).
#define		LED1SV				BIT(11)	// LED1 software value.
#define		LED1CM_SHIFT		8	// LED1 Control Mode.
#define		LED1CM_Mask		0x07
#define		LED0DIS				BIT(7)	// LED1 Disabled for analog signal usage,
										// 1: disable (input mode), 0:Enable (output mode).
#define		LED0PL				BIT(4)	// LED0 polarity control (auto-load configurable).
#define		LED0SV				BIT(3)	// LED0 software value.
#define		LED0CM_SHIFT		0	// LED0 Control Mode.
#define		LED0CM_Mask		0x07

#define		LED_SW_CTRL			0
#define		LED_OFF				1
#define		LED_TX_RX_EVENT_ON	2
#define		LED_TX_RX_EVENT_OFF	3
#define		LED_TX_EVENT_ON		4
#define		LED_TX_EVENT_OFF		5
#define		LED_RX_EVENT_ON		6
#define		LED_RX_EVENT_OFF		7

//----------------------------------------------------------------------------
//       8192C FSIMR bits						(Offset 0x50-53, 32 bits)
//----------------------------------------------------------------------------
#define		PDNINT_EN			BIT(31)	// Enable Hardware Power Down interrupt source.
#define		SYS_TIMER_EN		BIT(30)	// Enable Hardware System Timer interrupt source.
#define		SPS_OCP_INT_EN	BIT(29)	// Enable SPS OCP alarm interrupt source.
#define		HPON_WRE_EN		BIT(28)	// Enable Continue access HPON registers interrupt source.
#define		GPIOB_INT_EN		BIT(23)	// Enable GPIOB interrupt source.
#define		GPIOA_INT_EN		BIT(22)	// Enable GPIOA interrupt source.
#define		GPIO9_INT_EN		BIT(21)	// Enable GPIO9 interrupt source.
#define		GPIO8_INT_EN		BIT(20)	// Enable GPIO8 interrupt source.
#define		GPIO7_INT_EN		BIT(19)	// Enable GPIO7 interrupt source.
#define		GPIO6_INT_EN		BIT(18)	// Enable GPIO6 interrupt source.
#define		GPIO5_INT_EN		BIT(17)	// Enable GPIO5 interrupt source.
#define		GPIO4_INT_EN		BIT(16)	// Enable GPIO4 interrupt source.
#define		GPIO3_INT_EN		BIT(15)	// Enable GPIO3 interrupt source.
#define		GPIO2_INT_EN		BIT(14)	// Enable GPIO2 interrupt source.
#define		GPIO1_INT_EN		BIT(13)	// Enable GPIO1 interrupt source.
#define		GPIO0_INT_EN		BIT(12)	// Enable GPIO0 interrupt source.

//----------------------------------------------------------------------------
//       8192C FSISR bits						(Offset 0x54-57, 32 bits)
//----------------------------------------------------------------------------
#define		PDNINT				BIT(31)	// Hardware Power Down interrupt.
#define		HWSYS_TIMER_INT	BIT(30)	// Hardware System Timer interrupt.
#define		SPS_OCP_INT		BIT(29)	// SPS OCP alarm interrupt.
#define		HPON_WRE			BIT(28)	// Continue access HPON registers interrupt. Write 1 clear.
#define		GPIOB_INT			BIT(23)	// GPIOB INT Status. Write 1 clear.
#define		GPIOA_INT			BIT(22)	// GPIOA INT Status. Write 1 clear.
#define		GPIO9_INT			BIT(21)	// GPIO9 INT Status. Write 1 clear.
#define		GPIO8_INT			BIT(20)	// GPIO8 INT Status. Write 1 clear.
#define		GPIO7_INT			BIT(19)	// GPIO7 INT Status. Write 1 clear.
#define		GPIO6_INT			BIT(18)	// GPIO6 INT Status. Write 1 clear.
#define		GPIO5_INT			BIT(17)	// GPIO5 INT Status. Write 1 clear.
#define		GPIO4_INT			BIT(16)	// GPIO4 INT Status. Write 1 clear.
#define		GPIO3_INT			BIT(15)	// GPIO3 INT Status. Write 1 clear.
#define		GPIO2_INT			BIT(14)	// GPIO2 INT Status. Write 1 clear.
#define		GPIO1_INT			BIT(13)	// GPIO1 INT Status. Write 1 clear.
#define		GPIO0_INT			BIT(12)	// GPIO0 INT Status. Write 1 clear.

//----------------------------------------------------------------------------
//       8192C MCUFWDL bits						(Offset 0x80-83, 32 bits)
//----------------------------------------------------------------------------
#define		RPWM_SHIFT		24	// Host Request Power State.
#define		RPWM_Mask			0x0FF
#define		CPRST				BIT(23)	// 8051 Reset Status.
#define		ROM_DLEN			BIT(19)	// ROM Download Enable (8051 Core will be reseted) FPGA only.
#define		ROM_PGE_SHIFT		16	// ROM Page (FPGA only).
#define		ROM_PGE_Mask		0x07
#define		MAC1_RFINI_RDY	BIT(10)	// 92D_REG, MAC1 MCU Initial RF ready
#define		MAC1_BBINI_RDY	BIT(9)	// 92D_REG, MAC1 MCU Initial BB ready
#define		MAC1_MACINI_RDY	BIT(8)	// 92D_REG, MAC1 MCU Initial MAC ready
#define		MCU_STATUS		BIT(7)	// 92D_REG, 1: SRAM, 0: ROM
#define		WINTINI_RDY		BIT(6)	// WLAN Interrupt Initial ready.
#define		MAC0_RFINI_RDY	BIT(5)	// MAC0 MCU Initial RF ready.
#define		MAC0_BBINI_RDY	BIT(4)	// MAC0 MCU Initial BB ready.
#define		MAC0_MACINI_RDY	BIT(3)	// MAC0 MCU Initial MAC ready.
#define		FWDL_CHKSUM_RPT	BIT(2)	// FWDL CheckSum report, 1: OK, 0 : Faill.
#define		MCUFWDL_RDY		BIT(1)	// Driver set this bit to notify MCU FW Download OK.
#define		MCUFWDL_EN		BIT(0)	// MCU Firmware download enable. 1:Enable, 0:Disable.

//----------------------------------------------------------------------------
//       8192C MCUTSTCFG bits					(Offset 0x84-87, 32 bits)
//----------------------------------------------------------------------------
#define		LBKTST_SHIFT		0	// LBK TST indicator.
#define		LBKTST_Mask		0x0FFFF

//----------------------------------------------------------------------------
//       8192C BIST_CTRL bits					(Offset 0xD0-D3, 32 bits)
//----------------------------------------------------------------------------
#define		BB_SPEED_SCAN		BIT(31)	// BB at speed scan mode Enable.
#define		SCAN_PLL_BYPASS	BIT(30)	// 92D_REG
#define		MBIST_RSTNI_Shift	8		// 92D_REG
#define		MBIST_RSTNI_Mask	0x07FF
#define		BIST_RESUME_PS	BIT(5)
#define		BIST_RESUME		BIT(4)
#define		BIST_DRF			BIT(3)	// 92D_REG
#define		BIST_RETEN			BIT(3)
#define		BIST_NORMAL		BIT(2)
#define		BIST_RSTN			BIT(1)
#define		BIST_CLK_EN		BIT(0)

//----------------------------------------------------------------------------
//       8192C C_BIST_RPT bits					(Offset 0xD4-D7, 32 bits)
//----------------------------------------------------------------------------
#define		C_MAC_BIST_FAIL			BIT(31)	// MAC_BIST_FAIL.
#define		C_USB_IRAM_FAIL			BIT(26)	// USB IRAM BIST FAIL.
#define		C_USB_RAM1_FAIL			BIT(25)	// USB RAM1 BIST FAIL.
#define		C_USB_PROM_FAIL			BIT(24)	// USB PROM BIST FAIL.
#define		C_PCIE_RETRY_BIST_FAIL	BIT(20)	// PCIE RETRY RAM BIST FAIL.
#define		C_PCIE_SOT_BIST_FAIL		BIT(19)	// PCIE RETRY SOT RAM BIST FAIL.
#define		C_PCIE_PH1_BIST_FAIL		BIT(18)	// PCIE PHDR RAM1 BIST FAIL.
#define		C_PCIE_PH0_BIST_FAIL		BIT(17)	// PCIE PHDR RAM0 BIST FAIL.
#define		C_PCIE_PD_BIST_FAIL		BIT(16)	// PCIE PDATA RAM BIST FAIL.
#define		C_MAC_BIST_RDY			BIT(10)	// MAC MBIST Test report Ready.
#define		C_USB_BIST_RDY			BIT(9)	// USB MBIST Test report Ready.
#define		C_PCIE_BIST_RDY			BIT(8)	// PCIE MBIST Test report Ready.
#define		C_MAC_BIST_START			BIT(2)	// MAC MBIST START PULSE.
#define		C_USB_BIST_START			BIT(1)	// USB MBIST START PULSE.
#define		C_PCIE_BIST_START			BIT(0)	// PCIE MBIST START PULSE.

//----------------------------------------------------------------------------
//       8192C USB_SIE_INTF bits					(Offset 0xE0-E3, 32 bits)
//----------------------------------------------------------------------------
#define		USB_SIE_INTF_WE			BIT(25)	// Write Enable of SIE interface.
#define		USB_SIE_INTF_BYIOREG		BIT(24)	// SIE Bypass IOREG interface.
#define		USB_SIE_INTF_ADDR_SHIFT	16	// The address of SIE register.
#define		USB_SIE_INTF_ADDR_Mask	0x0FF
#define		USB_SIE_INTF_RD_SHIFT		8	// The read data from USB SIE.
#define		USB_SIE_INTF_RD_Mask		0x0FF
#define		USB_SIE_INTF_WD_SHIFT	0	// The write data to USB SIE.
#define		USB_SIE_INTF_WD_Mask		0x0FF

//----------------------------------------------------------------------------
//       8192C PCIE_MIO_INTF bits				(Offset 0xE4-E7, 32 bits)
//----------------------------------------------------------------------------
#define		PCIE_MIO_BYIOREG		BIT(13)	// MIO bypass IOREG interface.
#define		PCIE_MIO_RE			BIT(12)	// Read Enable of MIO interface.
#define		PCIE_MIO_WE_SHIFT		8	// Write Byte Enable of MIO interface.
#define		PCIE_MIO_WE_Mask		0x0F
#define		PCIE_MIO_ADDR_SHIFT	0	// The address of PCIE MIO Access register.
#define		PCIE_MIO_ADDR_Mask	0x0FF

//----------------------------------------------------------------------------
//       8192C HPON_FSM bits					(Offset 0xEC-EF, 32 bits)
//----------------------------------------------------------------------------
#define		EVENT_FSM_SHIFT	16	// The State Machine Trigger Events.
#define		EVENT_FSM_Mask	0x0FFFF
#define		PMC_ADDR_SHIFT	8	// The Hardware Access HPON Register Addr.
#define		PMC_ADDR_Mask		0x0FF
#define		PMC_WR				BIT(7)	// The Hardware Access HPON Register Write Pulse.
#define		PMC_DATA			BIT(6)	// The Hardware Access HPON Register Data.
#define		FSM_STATE_SHIFT	0	// The State Machine Data.
#define		FSM_STATE_Mask	0x03F

//----------------------------------------------------------------------------
//       8192C SYS_CFG bits					(Offset 0xF0-F3, 32 bits)
//----------------------------------------------------------------------------
#define		TRP_ICFG_SHIFT			28	// Trapped ICFG data value.
#define		TRP_ICFG_Mask			0x0F
#define		TYPE_ID					BIT(27)	// 1: 8191C/8192C ; 0: 8188C.
#define		BD_HCI_SEL				BIT(26)	// Trapped HCI_SEL value.
#define		BD_PKG_SEL				BIT(25)	// Trapped PKG_ENG_SEL value.
#define		TRP_BT_EN				BIT(24)	// Trapped BT Chip Valid need XTAL & SPS resource.
#define		TRP_VAUX_EN			BIT(23)	// Trapped VAUX_EN.
#define		PAD_HWPD_IDN			BIT(22)	// HWPDN PAD status Indicator.
#define		PSC_TESTCFG_SHIFT		20	// Test Pin configuration.
#define		PSC_TESTCFG_Mask		0x03
#define		CHIP_VER_RL_SHIFT		16	// Chip version (RL6111).
#define		CHIP_VER_RL_Mask		0x0F
#define		CHIP_VER_RTL_SHIFT	12	// Test Chip version - 8188C (RLE0307) / 8191C (RLE0308).
#define		CHIP_VER_RTL_Mask		0x0F
#define		IC_MACPHY_MODE		BIT(11)	// 0: IC Normal Mode, 1: force IC PHY_ASIC Mode.
#define		BD_MAC1				BIT(10)	// Bonding option for MAC function-1 Enable.
#define		BD_MAC2				BIT(9)	// Bonding option for MAC function-2 Enable.
#define		SIC_IDLE				BIT(8)	// Indicator of SIC idle state.
#define		TRP_B15V_EN			BIT(7)	// Trapped option for 1.5V of board.
#define		OCP_SHUTDN			BIT(6)	// SPS OCP shutdown Chip.
#define		V15_VLD				BIT(5)	// 1.5V Power Ready, 1: Power Ready.
#define		PCIRSTB					BIT(4)	// PCIE External PERST, Pin Status.
#define		PCLK_VLD				BIT(3)	// PCIE PHY Clock Stable, 1: Clock Stable.
#define		UCLK_VLD				BIT(2)	// USB PHY Clock Stable, 1: Clock Stable.
#define		ACLK_VLD				BIT(1)	// AFE Clock Stable, 1: Clock Stable.
#define		XCLK_VLD				BIT(0)	// Xtal Clock Stable, 1: Clock Stable.


//
// 2. MACTOP General Configuration. (Offset 0x100 - 0x1FFh)
//
//----------------------------------------------------------------------------
//       8192C CR bits						(Offset 0x100-103, 32 bits)
//----------------------------------------------------------------------------
#define		LBMODE_SHIFT		24	// Loopback mode.
#define		LBMODE_Mask		0x0F
#define		NETYPE_SHIFT		16	// Network Type.
#define		NETYPE_SHIFT1		18
#define		NETYPE_Mask		0x03
#define		CALTMR_EN			BIT(10)	// for 8723 & 88E only
#define		MAC_SEC_EN		BIT(9)	// Enable MAC security engine.
#define		ENSWBCN			BIT(8)	// Enable SW TX beacon.
#define		MACRXEN			BIT(7)	// MAC Receiver Enable.
#define		MACTXEN			BIT(6)	// MAC Transmitter Enable.
#define		SCHEDULE_EN		BIT(5)	// Schedule Enable.
#define		PROTOCOL_EN		BIT(4)	// protocol Block Function Enable.
#define		RXDMA_EN			BIT(3)	// RXDMA Function Enable.
#define		TXDMA_EN			BIT(2)	// TXDMA Function Enable.
#define		HCI_RXDMA_EN		BIT(1)	// HCI to RXDMA Interface Enable.
#define		HCI_TXDMA_EN		BIT(0)	// HCI to TXDMA Interface Enable.
// Loopback mode.
#define		LB_NORMAL			0x00
#define		LB_MAC				0x0B
#define		LB_MAC_DLY			0x03
#define		LB_PHY				0x01
#define		LB_DMA				0x07
#define		LB_DUAL_MAC		0x1B	// 92D_REG
// Network Type.
#define		NETYPE_NOLINK		0x00
#define		NETYPE_ADHOC		0x01
#define		NETYPE_INFRA		0x02
#define		NETYPE_AP			0x03

//----------------------------------------------------------------------------
//       8192C PBP bits						(Offset 0x104-107, 32 bits)
//----------------------------------------------------------------------------
#define		PSTX_SHIFT			4	// Page size of transmit packet buffer.
#define		PSTX_Mask			0x0F
#define		PSRX_SHIFT			0	// Page size of receive packet buffer and C2HCMD buffer.
#define		PSRX_Mask			0x0F
// Page size
#define		PBP_64B			0x00
#define		PBP_128B			0x01
#define		PBP_256B			0x02
#define		PBP_512B			0x03
#define		PBP_1024B			0x04

//----------------------------------------------------------------------------
//       8192C TRXDMA_CTRL bits				(Offset 0x10C-10D, 16 bits)
//----------------------------------------------------------------------------
#define		HPQ_SEL_SHIFT		8	// High Priority Queue Selection.
#define		HPQ_SEL_Mask		0x03F
#define		RXDMA_AGG_EN		BIT(2)	//
#define		RXSHFT_EN			BIT(1)	// When this bit is set, RX shift to fit alignment is enable.
#define		RXDMA_ARBBW_EN	BIT(0)	// Enable RXDMA Arbitrator priority for Host interface.
// High Priority Queue Selection.
#define		HPQ_SEL_VOQ		BIT(0)
#define		HPQ_SEL_VIQ		BIT(1)
#define		HPQ_SEL_BEQ		BIT(2)
#define		HPQ_SEL_BKQ		BIT(3)
#define		HPQ_SEL_MGQ		BIT(4)
#define		HPQ_SEL_HIQ		BIT(5)

// For normal driver, 0x10C
#define _TXDMA_HIQ_MAP(x) 	 		(((x)&0x3) << 14)
#define _TXDMA_MGQ_MAP(x) 	 		(((x)&0x3) << 12)
#define _TXDMA_BKQ_MAP(x) 	 		(((x)&0x3) << 10)
#define _TXDMA_BEQ_MAP(x) 	 		(((x)&0x3) << 8 )
#define _TXDMA_VIQ_MAP(x) 	 		(((x)&0x3) << 6 )
#define _TXDMA_VOQ_MAP(x) 	 		(((x)&0x3) << 4 )

#define QUEUE_LOW					1
#define QUEUE_NORMAL				2
#define QUEUE_HIGH					3

//----------------------------------------------------------------------------
//       8192C TRXFF_BNDY bits				(Offset 0x114-117, 32 bits)
//----------------------------------------------------------------------------
#define		RXFF0_BNDY_SHIFT			16	// upper boundary of RXFF0.
#define		RXFF0_BNDY_Mask			0x0FFFF
#define		TXPKTBUF_PGBNDY_SHIFT	0	// From FWHW offload, sets the max pages controlled by TXDMA.
#define		TXPKTBUF_PGBNDY_Mask		0x0FF

//----------------------------------------------------------------------------
//       8192C TRXFF_STATUS bits				(Offset 0x118-11B, 32 bits)
//----------------------------------------------------------------------------
#define		TXPKTFF_FULL		BIT(16)	// TXDMA block the current TX pkt due to TXPKTBUF full, status bit will be set.
#define		RXFF1_OVF			BIT(1)	// When this bit is "1", RXFF1 is under overflow condition.
#define		RXFF0_OVF			BIT(0)	// When this bit is "1", RXFF0 is under overflow condition.

//----------------------------------------------------------------------------
//       8192C RXFF_PTR bits					(Offset 0x11C-11F, 32 bits)
//----------------------------------------------------------------------------
#define		RXFF0_RDPTR_SHIFT		16	// The read pointer address of RXFF0.
#define		RXFF0_RDPTR_Mask		0x0FFFF
#define		RXFF0_WTPTR_SHIFT		0	// The write pointer address of RXFF0.
#define		RXFF0_WTPTR_Mask		0x0FFFF

//----------------------------------------------------------------------------
//       8192C HIMR bits						(Offset 0x120-123, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C HISR bits						(Offset 0x124-127, 32 bits)
//----------------------------------------------------------------------------
#define		HIMR_BCNDMA7		BIT(31)	// Beacon DMA Interrupt 7.
#define		HIMR_BCNDMA6		BIT(30)	// Beacon DMA Interrupt 6.
#define		HIMR_BCNDMA5		BIT(29)	// Beacon DMA Interrupt 5.
#define		HIMR_BCNDMA4		BIT(28)	// Beacon DMA Interrupt 4.
#define		HIMR_BCNDMA3		BIT(27)	// Beacon DMA Interrupt 3.
#define		HIMR_BCNDMA2		BIT(26)	// Beacon DMA Interrupt 2.
#define		HIMR_BCNDMA1		BIT(25)	// Beacon DMA Interrupt 1.
#define		HIMR_BCNDOK7		BIT(24)	// Beacon Queue DMA OK Interrupt7.
#define		HIMR_BCNDOK6		BIT(23)	// Beacon Queue DMA OK Interrupt6.
#define		HIMR_BCNDOK5		BIT(22)	// Beacon Queue DMA OK Interrupt5.
#define		HIMR_BCNDOK4		BIT(21)	// Beacon Queue DMA OK Interrupt4.
#define		HIMR_BCNDOK3		BIT(20)	// Beacon Queue DMA OK Interrupt3.
#define		HIMR_BCNDOK2		BIT(19)	// Beacon Queue DMA OK Interrupt2.
#define		HIMR_BCNDOK1		BIT(18)	// Beacon Queue DMA OK Interrupt1.
#define		HIMR_TIMEOUT2		BIT(17)	// Time Out Interrupt 2.
#define		HIMR_TIMEOUT1		BIT(16)	// Time Out Interrupt 1.
#define		HIMR_TXFOVW		BIT(15)	// Transmit packet buffer Overflow.
#define		HIMR_PSTIMEOUT	BIT(14)	// Power Save Time Out Interrupt.
#define		HIMR_BCNDMA0		BIT(13)	// Beacon DMA Interrupt 0.
#define		HIMR_RXFOVW		BIT(12)	// Receive packet buffer Overflow.
#define		HIMR_RDU			BIT(11)	// Receive Descriptor Unavailable.
#define		HIMR_ATIM_END		BIT(10)	// ATIM Window End Interrupt.
#define		HIMR_BCNDOK0		BIT(9)	// Beacon Queue DMA OK Interrupt0.
#define		HIMR_HIGHDOK		BIT(8)	// High Queue DMA OK Interrupt.
#define		HIMR_TXBCNOK		BIT(7)	// Transmit Beacon OK Interrupt.
#define		HIMR_MGTDOK		BIT(6)	// Management Queue DMA OK Interrupt.
#define		HIMR_TXBCNERR		BIT(5)	// Transmit Beacon Error Interrupt.
#define		HIMR_BKDOK			BIT(4)	// AC_BK DMA OK Interrupt.
#define		HIMR_BEDOK			BIT(3)	// AC_BE DMA OK Interrupt.
#define		HIMR_VIDOK			BIT(2)	// AC_VI DMA OK Interrupt.
#define		HIMR_VODOK		BIT(1)	// AC_VO DMA Interrupt.
#define		HIMR_ROK			BIT(0)	// Receive DMA OK Interrupt.

//----------------------------------------------------------------------------
//       8192C HIMRE bits					(Offset 0x128-12A, 24 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C HISRE bits					(Offset 0x12C-12E, 24 bits)
//----------------------------------------------------------------------------
#define		HIMRE_TXERR		BIT(11)	// Enable Tx Error Flag Interrupt.
#define		HIMRE_RXERR		BIT(10)	// Enable Rx Error Flag Interrupt.
#define		HIMRE_C2HCMD		BIT(9)	// Enable CPU to Host Command Interrupt.
#define		HIMRE_CPWM		BIT(8)	// Enable CPU power Mode exchange Interrupt.
#define		HIMRE_HBT_CMD		BIT(7)	// 92D_REG, Host BT_CMD interrupt mask
#define		HIMRE_OCPINT		BIT(1)	// Enable SPS_OCP Interrupt.
#define		HIMRE_WLANOFF		BIT(0)	// Enable WLAN power down Interrupt.

//----------------------------------------------------------------------------
//       8192C CPWM bits					(Offset 0x12F, 8 bits)
//----------------------------------------------------------------------------
#define		CPWM_TOGGLING	BIT(7)	// When this bit is toggled, interrtup is send to HISRE.CPWMINT.
#define		CPWM_MOD_SHIFT	0	// The current power mode index.
#define		CPWM_MOD_Mask	0x07F

//----------------------------------------------------------------------------
//       8192C FWIMR bits					(Offset 0x130-133, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C FWISR bits					(Offset 0x134-137, 32 bits)
//----------------------------------------------------------------------------
#define		FWIMR_VOQ			BIT(31)	//
#define		FWIMR_VIQ			BIT(30)	//
#define		FWIMR_BEQ			BIT(29)	//
#define		FWIMR_BKQ			BIT(28)	//
#define		FWIMR_MGTQ		BIT(27)	//
#define		FWIMR_HIQ			BIT(26)	//
#define		FWIMR_TXPKTIN		BIT(25)	// interrupt is issued to inform MCU.
#define		FWIMR_TXBCNOK		BIT(24)	// interrupt is issued when a beacon frame is transmitted.
#define		FWIMR_TXBCNERR	BIT(23)	// interrupt is issued when beacon in TXPKTBUF fails to send out.
#define		FWIMR_RX_UMD0	BIT(22)	// receiving unicast data frame with More Data=0.
#define		FWIMR_RX_UMD1	BIT(21)	// receiving unicast data frame with More Data=1.
#define		FWIMR_RX_BMD0		BIT(20)	// receiving broadcast data frame with More Data=0.
#define		FWIMR_RX_BMD1		BIT(19)	// receiving broadcast data frame with More Data=1.
#define		FWIMR_BCN_RX		BIT(18)	// When receiving Beacon frame.
#define		FWIMR_TBTT		BIT(17)	// The exact TBTT time interrupt.
#define		FWIMR_BCNERLY		BIT(16)	// This interrupt is issue at the time set by DRVERLYINT register before TBTT time.
#define		FWIMR_BCNDMA7	BIT(15)	// When BCNDMA interval arrives before TBTT7, informs MCU to prepare beacon.
#define		FWIMR_BCNDMA6	BIT(14)	// When BCNDMA interval arrives before TBTT6, informs MCU to prepare beacon.
#define		FWIMR_BCNDMA5	BIT(13)	// When BCNDMA interval arrives before TBTT5, informs MCU to prepare beacon.
#define		FWIMR_BCNDMA4	BIT(12)	// When BCNDMA interval arrives before TBTT4, informs MCU to prepare beacon.
#define		FWIMR_BCNDMA3	BIT(11)	// When BCNDMA interval arrives before TBTT3, informs MCU to prepare beacon.
#define		FWIMR_BCNDMA2	BIT(10)	// When BCNDMA interval arrives before TBTT2, informs MCU to prepare beacon.
#define		FWIMR_BCNDMA1	BIT(9)	// When BCNDMA interval arrives before TBTT1, informs MCU to prepare beacon.
#define		FWIMR_BCNDMA0	BIT(8)	// When BCNDMA interval arrives before TBTT0, informs MCU to prepare beacon.
#define		FWIMR_LP_STBY		BIT(7)	// Low Power Standby Wake interrupt.
#define		FWIMR_ATIM		BIT(6)	// This interrupt is issued when it is at the time ATIM ms before ATIMWND expiring.
#define		FWIMR_HRCV		BIT(5)	// Host Recover CPU Loop Instruction.
#define		FWIMR_H2CCMD		BIT(4)	// Host To CPU Message Interrupt.
#define		FWIMR_RXDONE		BIT(3)	//
#define		FWIMR_ERRORHDL	BIT(2)	// FWHW/ TXDMA/ RXDMA/ WMAC error status interrupt.
#define		FWIMR_TXCCX		BIT(1)	//
#define		FWIMR_TXCLOSE		BIT(0)	//

//----------------------------------------------------------------------------
//       8192C FTIMR bits					(Offset 0x138-13B, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C FTISR bits					(Offset 0x13C-13F, 32 bits)
//----------------------------------------------------------------------------
#define		FTIMR_FWBT_CMD	BIT(18)	// 92D_REG, FW BT_CMD interrupt mask
#define		FTIMR_RPWM		BIT(8)	// RPWM INT Status. Write 1 clear.
#define		FTIMR_PSTIMER		BIT(7)	// Enable PSTimer interrupt.
#define		FTIMR_TIMEOUT1	BIT(6)	// Enable Timer1 interrupt.
#define		FTIMR_TIMEOUT0	BIT(5)	// Enable Timer0 interrupt.
#define		FTIMR_GT4			BIT(4)	// Enable General Timer[4] interrupt.
#define		FTIMR_GT3			BIT(3)	// Enable General Timer[3] interrupt.
#define		FTIMR_GT2			BIT(2)	// Enable General Timer[2] interrupt.
#define		FTIMR_GT1			BIT(1)	// Enable General Timer[1] interrupt.
#define		FTIMR_GT0			BIT(0)	// Enable General Timer[0] interrupt.

//----------------------------------------------------------------------------
//       8192C PKTBUF_DBG_CTRL bits			(Offset 0x140-143, 32 bits)
//----------------------------------------------------------------------------
#define		RXPKTBUF_DBG				BIT(14)	// 1: Enable RXPKTBUF debug mode.
#define		TXPKTBUF_DBG				BIT(13)	// 1: Enable TXPKTBUF debug mode.
#define		PKTBUF_DBG_ADDR_SHIFT	0	// The address of TRXPKTBUF to be read.
#define		PKTBUF_DBG_ADDR_Mask		0x01FFF

//		General Purpose Timer. (Offset 0x150 - 0x16Fh)
//----------------------------------------------------------------------------
//       8192C TC0_CTRL bits					(Offset 0x150-153, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C TC1_CTRL bits					(Offset 0x154-157, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C TC2_CTRL bits					(Offset 0x158-15B, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C TC3_CTRL bits					(Offset 0x15C-15F, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C TC4_CTRL bits					(Offset 0x160-163, 32 bits)
//----------------------------------------------------------------------------
#define		TCINT_EN			BIT(26)	// When write "1" to this bit, Counter starts counting down.
#define		TCMODE				BIT(25)	// Timer/Counter mode.
#define		TCEN				BIT(24)	// Timer/Counter enable.
#define		TCData_SHIFT		0	// Timer/Counter data register. It specifies the time-out duration.
#define		TCData_Mask		0x0FFFFFF

//----------------------------------------------------------------------------
//       8192C TCUNIT_BASE bits				(Offset 0x164-165, 16 bits)
//----------------------------------------------------------------------------
#define		TCUNIT_BASE_TIME_SHIFT	0	// num of clocks that equals to the base time unit of timer/counter.
#define		TCUNIT_BASE_TIME_Mask	0x03FFF
//		General Purpose Timer. (Offset 0x150 - 0x16Fh)

//----------------------------------------------------------------------------
//       8192C MBIST_START bits				(Offset 0x174-177, 32 bits)
//----------------------------------------------------------------------------
#define		TXLLT_BISTP		BIT(10)	// TXLLT BIST Start Pulse.
#define		TXOQT_BISTP		BIT(9)	// TXOQT BIST Start Pulse.
#define		TXPKT_BISTP		BIT(8)	// TXPKT Buffer BIST Start Pulse.
#define		RXPKT_BISTP		BIT(7)	// RXPKT Buffer BIST Start Pulse.
#define		TXDBUF_BISTP		BIT(6)	// TXDBUF BIST Start Pulse.
#define		TXFIFO_BISTP		BIT(5)	// TXFIFO BIST Start Pulse.
#define		RXFIFO_BISTP		BIT(4)	// RXFIFO BIST Start Pulse.
#define		WKEYLLT_BISTP		BIT(3)	// WKEY BIST Start Pulse.
#define		WRXBA_BISTP		BIT(2)	// WRXBA BIST Start Pulse.

//----------------------------------------------------------------------------
//       8192C MBIST_DONE bits				(Offset 0x178-17B, 32 bits)
//----------------------------------------------------------------------------
#define		TXLLT_BISTD		BIT(10)	// TXLLT BIST Done.
#define		TXOQT_BISTD		BIT(9)	// TXOQT BIST Done.
#define		TXPKT_BISTD		BIT(8)	// TXPKT Buffer BIST Done.
#define		RXPKT_BISTD		BIT(7)	// RXPKT Buffer BIST Done.
#define		TXDBUF_BISTD		BIT(6)	// TXDBUF BIST Done.
#define		TXFIFO_BISTD		BIT(5)	// TXFIFO BIST Done.
#define		RXFIFO_BISTD		BIT(4)	// RXFIFO BIST Done.
#define		WKEYLLT_BISTD		BIT(3)	// WKEY BIST Done.
#define		WRXBA_BISTD		BIT(2)	// WRXBA BIST Done.

//----------------------------------------------------------------------------
//       8192C MBIST_FAIL bits				(Offset 0x17C-17F, 32 bits)
//----------------------------------------------------------------------------
#define		TXLLT_BISTF		BIT(10)	// TXLLT BIST Fail.
#define		TXOQT_BISTF		BIT(9)	// TXOQT BIST Fail.
#define		TXPKT_BISTF		BIT(8)	// TXPKT Buffer BIST Fail.
#define		RXPKT_BISTF		BIT(7)	// RXPKT Buffer BIST Fail.
#define		TXDBUF_BISTF		BIT(6)	// TXDBUF BIST Fail.
#define		TXFIFO_BISTF		BIT(5)	// TXFIFO BIST Fail.
#define		RXFIFO_BISTF		BIT(4)	// RXFIFO BIST Fail.
#define		WKEYLLT_BISTF		BIT(3)	// WKEY BIST Fail.
#define		WRXBA_BISTF		BIT(2)	// WRXBA BIST Fail.

//----------------------------------------------------------------------------
//       8192C FMETHR bits					(Offset 0x1C8-1CB, 32 bits)
//----------------------------------------------------------------------------
#define		FMSG_INT			BIT(31)	// Toggle this bit will generate interrupt to Host.
#define		FW_MSG_SHIFT		0	// User Defined Message.
#define		FW_MSG_Mask		0x07FFFFFFF

//----------------------------------------------------------------------------
//       8192C HMETFR bits					(Offset 0x1CC-1CF, 32 bits)
//----------------------------------------------------------------------------
#define		HRCV_MSG_SHIFT	24	// User Defined Message.
#define		HRCV_MSG_Mask		0x0FF
#define		INT_BOX3			BIT(3)	// MSG_BOX_3 Valid. Enable when the Entry is write.
#define		INT_BOX2			BIT(2)	// MSG_BOX_2 Valid. Enable when the Entry is write.
#define		INT_BOX1			BIT(1)	// MSG_BOX_1 Valid. Enable when the Entry is write.
#define		INT_BOX0			BIT(0)	// MSG_BOX_0 Valid. Enable when the Entry is write.

//----------------------------------------------------------------------------
//       8192C LLT_INI bits					(Offset 0x1E0-1E3, 32 bits)
//----------------------------------------------------------------------------
#define		LLTE_RWM_SHIFT		30	//
#define		LLTE_RWM_Mask			0x03
#define		LLTINI_PDATA_SHIFT	16	// LLT Entry Write/Read DATA for MCU.
#define		LLTINI_PDATA_Mask		0x0FF
#define		LLTINI_ADDR_SHIFT		8	// LLT Entry Access Offset.
#define		LLTINI_ADDR_Mask		0x0FF
#define		LLTINI_HDATA_SHIFT	0	// LLT Entry Write/Read DATA for HCI.
#define		LLTINI_HDATA_Mask		0x0FF
// LLTE_RWM
#define		LLTE_RWM_NO_ACTIVE	0x00
#define		LLTE_RWM_WR			0x01
#define		LLTE_RWM_RD			0x03

//----------------------------------------------------------------------------
//       8192C BB_ACCEESS_CTRL bits			(Offset 0x1E8-1EB, 32 bits)
//----------------------------------------------------------------------------
#define		BB_WRITE_READ_SHIFT	30	//
#define		BB_WRITE_READ_Mask	0x03
#define		BB_WRITE_EN_SHIFT		12	// Byte Write Enable.
#define		BB_WRITE_EN_Mask		0x0F
#define		BB_ADDR_SHIFT			2	// Baseband Access Write/Read Address (in DW unit).
#define		BB_ADDR_Mask			0x03FF
#define		BB_ERRACC				BIT(0)	// Duplicate Access when previous cycle pending (write one clear).


//
// 3. TXDMA/RXDMA Configuration. (Offset 0x200 - 0x2FFh)
//
//----------------------------------------------------------------------------
//       8192C RQPN bits						(Offset 0x200-203, 32 bits)
//----------------------------------------------------------------------------
#define		LD_RQPN			BIT(31)	// Write 1 to set RQPN bit[79:0] value to page numbers for initialization.
#define		LPQ_PUBLIC_DIS		BIT(25)	// bit=1, available Tx page size excludes the public queue.
#define		HPQ_PUBLIC_DIS		BIT(24)	// bit=1, available Tx page size excludes the public queue.
#define		PUBQ_SHIFT			16	// Public Reserved Page Number.
#define		PUBQ_Mask			0x0FF
#define		LPQ_SHIFT			8	// Low Priority Queue Reserved Page Number.
#define		LPQ_Mask			0x0FF
#define		HPQ_SHIFT			0	// High Priority Queue Reserved Page Number.
#define		HPQ_Mask			0x0FF

#define _HPQ(x)			((x) & HPQ_Mask)
#define _LPQ(x)			(((x) & LPQ_Mask) << LPQ_SHIFT)
#define _PUBQ(x)			(((x) & PUBQ_Mask) << PUBQ_SHIFT)
#define _NPQ(x)			((x) & 0xFF)			// NOTE: in RQPN_NPQ register


//----------------------------------------------------------------------------
//       8192C FIFOPAGE bits					(Offset 0x204-207, 32 bits)
//----------------------------------------------------------------------------
#define		TXPKTNUM_SHIFT		24	// Packet number in TXPKTBUF.
#define		TXPKTNUM_Mask			0x0FF
#define		PUB_AVAL_PG_SHIFT		16	// Available Public Queue Page Number.
#define		PUB_AVAL_PG_Mask		0x0FF
#define		LPQ_AVAL_PG_SHIFT		8	// Available Low Priority Queue Page Number.
#define		LPQ_AVAL_PG_Mask		0x0FF
#define		HPQ_AVAL_PG_SHIFT		0	// Available High Priority Queue Page Number.
#define		HPQ_AVAL_PG_Mask		0x0FF

//----------------------------------------------------------------------------
//       8192C TDECTRL bits					(Offset 0x208-20B, 32 bits)
//----------------------------------------------------------------------------
#define		LLT_FREE_PAGE_SHIFT	24	// LLT Free Page.
#define		LLT_FREE_PAGE_Mask	0x0FF
#define		BCN_VALID				BIT(16)	// bit=1, beacon packet has finished to write to txpktbuffer.
#define		BCN_HEAD_SHIFT		8	// head page of Bcnq packet which is Tx DMA filled.
#define		BCN_HEAD_Mask			0x0FF
#define		BLK_DESC_NUM_SHIFT	4	// The Most Number of Tx Descriptor per Bulk Out Only for USB.
#define		BLK_DESC_NUM_Mask	0x0F

//----------------------------------------------------------------------------
//       8192C TXDMA_OFFSET_CHK bits		(Offset 0x20C-20F, 32 bits)
//----------------------------------------------------------------------------
#define		PG_UNDER_TH_SHIFT	16	// Page threshold value is check by Tx DMA engine.
#define		PG_UNDER_TH_Mask		0x0FF
#define		CHK_PG_TH_EN			BIT(10)	// Enable Tx DMA to check total pages if it is under page threshold.
#define		DROP_DATA_EN			BIT(9)	// Enable Tx DMA to drop the redundant data of packet.
#define		CHECK_OFFSET_EN		BIT(8)	// Enable Tx DMA to check offset value.
#define		CHECK_OFFSET_SHIFT	0	// Offset value is check by Tx DMA engine.
#define		CHECK_OFFSET_Mask		0x0FF

//----------------------------------------------------------------------------
//       8192C TXDMA_STATUS bits			(Offset 0x210-213, 32 bits)
//----------------------------------------------------------------------------
#define		PAYLOAD_UDN		BIT(14)	// Payload is under the packet length of Tx descriptor.
#define		PAYLOAD_OVF		BIT(13)	// Payload is over the packet length of Tx descriptor.
#define		DSC_CHKSUM_FAIL	BIT(12)	// Tx descriptor checksum error.
#define		UNKNOWN_QSEL		BIT(11)	// An unknown QSEL of Tx descriptor is detected.
#define		EP_QSEL_DIFF		BIT(10)	// Tx Endpoint is unmatched with the QSEL of descriptor.
#define		TX_OFFS_UNMATCH	BIT(9)	// Tx offset is unmatched.
#define		TXOQT_UDN			BIT(8)	// TXOQT Underflow.
#define		TXOQT_OVF			BIT(7)	// TXOQT Overflow.
#define		TXDMA_SFF_UDN		BIT(6)	// TXDMA Sync FIFO Underflow.
#define		TXDMA_SFF_OVF		BIT(5)	// TXDMA Sync FIFO Overflow.
#define		LLT_NULL_PG		BIT(4)	// TXDMA reads NULL page.
#define		PAGE_UDN			BIT(3)	// Total pages included PTCL un-return pages under the total reserved pages.
#define		PAGE_OVF			BIT(2)	// Total pages included PTCL un-return pages over the total reserved pages.
#define		TXFF_PG_UDN		BIT(1)	// TXFF page underflow in TDE page controller.
#define		TXFF_PG_OVF		BIT(0)	// TXFF page overflow in TDE page controller.

//----------------------------------------------------------------------------
//       8192C RXDMA_AGG_PG_TH bits		(Offset 0x280-283, 32 bits)
//----------------------------------------------------------------------------
#define		RXDMA_AGG_PG_TH_LMT(x)	((x & 0x0FF)<<0)	// DMA inform host to receive pkts, when exceeds the threshold.

//----------------------------------------------------------------------------
//       8192C RXPKT_NUM bits				(Offset 0x284-287, 32 bits)
//----------------------------------------------------------------------------
#define		RXPKT_NUM_LMT_SHIFT	24	// number of packets in RXPKTBUF.
#define		RXPKT_NUM_LMT_Mask	0x0FF
#define		RW_RELEASE_EN			BIT(18)	// bit=1, RXDMA will enter this mode after RXDMA packet to host completed and stop.
#define		RXDMA_IDLE				BIT(17)	// RXDMA finishes DMA will report idle state in this bit.
#define		RXPKT_RELEASE_POLL	BIT(16)	// bit=1, RXDMA will decrease RX PKT counter by one.
#define		FW_UPD_RDPTR_SHIFT	0	// FW updates before write RXPKT_RELEASE_POLL to 1.
#define		FW_UPD_RDPTR_Mask	0x0FFFF

//----------------------------------------------------------------------------
//       8192C RXDMA_STATUS bits			(Offset 0x288-28B, 32 bits)
//----------------------------------------------------------------------------
#define		FW_POLL_ISSUE		BIT(5)	// FW Release Poll Error.
#define		RX_DATA_UDN		BIT(4)	// RX Data Underflow. Clear by SW.
#define		RX_SFF_UDN			BIT(3)	// RX Sync FIFO Underflow.
#define		RX_SFF_OVF			BIT(2)	// RX Sync FIFO Overflow.
#define		USB_REQ_LEN_OVF	BIT(1)	// USB RXAGGEN Error due to RX length overflow.
#define		RXPKT_OVF			BIT(0)	// When RX Packet is more than 255 packets remaining in FF.


//
// 4. PCIE EMAC Reserved Region. (Offset 0x300 - 0x3FFh)
//
//----------------------------------------------------------------------------
//       8192C PCIE_CTRL_REG bits			(Offset 0x300-303, 32 bits)
//----------------------------------------------------------------------------
#define		MAX_RXDMA_SHIFT	28	// RXDMA Burst Size selection.
#define		MAX_RXDMA_Mask	0x07
#define		MAX_TXDMA_SHIFT	24	// TXDMA Burst Size selection.
#define		MAX_TXDMA_Mask	0x07
#define		En_HWENTRL		BIT(19)	// Enable HW call EMAC to enter L23 actively.
#define		EN_SWENTR_L23		BIT(17)	// Enable SW call EMAC to enter L23.
#define		EN_HWEXITL1		BIT(16)	// Enable HW call EMAC to exit L1.
#define		BCNQSTOP			BIT(15)	// BCNQ DMA STOP.
#define		HQSTOP				BIT(14)	// HQ DMA STOP.
#define		MGQSTOP			BIT(13)	// VOKQ DMA STOP.
#define		VOQSTOP			BIT(12)	// VOKQ DMA STOP.
#define		VIQSTOP			BIT(11)	// VIQ DMA STOP.
#define		BEQSTOP			BIT(10)	// BEQ DMA STOP.
#define		BKQSTOP			BIT(9)	// BKQ DMA STOP.
#define		RXQSTOP			BIT(8)	// RX DMA STOP.
#define		HQ_POLL			BIT(7)	// High Queue Polling bit, read result will be hpqflag.
#define		MGQ_POLL			BIT(6)	// Manag Queue Polling bit, read result will be mgqflag.
#define		BCNQ_POLL			BIT(4)	// BCNQ Polling bit, read result will be bpqflag.
#define		VOQ_POLL			BIT(3)	// VOQ Polling bit, read result will be voqflag.
#define		VIQ_POLL			BIT(2)	// VIQ Polling bit, read result will be viqflag.
#define		BEQ_POLL			BIT(1)	// BEQ Polling bit, read result will be beqflag.
#define		BKQ_POLL			BIT(0)	// BKQ Polling bit, read result will be bkqflag.

//----------------------------------------------------------------------------
//       8192C INT_MIG bits					(Offset 0x304-307, 32 bits)
//----------------------------------------------------------------------------
#define		TTMRMIT_Shift			28	// 92D_REG, Tx timer mitigation
#define		TTMRMIT_Mask			0x0F
#define		TNUMMIT_Shift			24	// 92D_REG, Tx number mitigation
#define		TNUMMIT_Mask			0x0F
#define		RTMRMIT_Shift			20	// 92D_REG, Rx timer mitigation
#define		RTMRMIT_Mask			0x0F
#define		RNUMMIT_Shift			16	// 92D_REG, Rx number mitigation
#define		RNUMMIT_Mask			0x0F
#define		INTMT_SHIFT			16	// Interrupt Couter for setting Interrupt Migration.
#define		INTMT_Mask				0x0FFFF
#define		MIGRATE_TIMER_SHIFT	0	// Timer for setting Interrupt Migration.
#define		MIGRATE_TIMER_Mask	0x0FFFF

//----------------------------------------------------------------------------
//       8192C DBI bits						(Offset 0x348-353, 96 bits)
//----------------------------------------------------------------------------
#define		DBI_ACC_EXT		BIT(19)	// 92D_REG, DBI access external register on ELBI
#define		DBI_MACSEL			BIT(18)	// 92D_REG, DBI access MAC1 select
#define		DBI_RFLAG			BIT(17)	// DBI Read Flag.
#define		DBI_WFLAG			BIT(16)	// DBI Write Flag.
#define		DBI_WREN_SHIFT	12	// DBI Write Enable, High active.
#define		DBI_WREN_Mask		0x0F
#define		DBI_ADDR_SHIFT	0	// DBI Address.
#define		DBI_ADDR_Mask		0x0FFF

//----------------------------------------------------------------------------//
//       8192C MDIO bits						(Offset 0x354-35B, 64 bits)
//----------------------------------------------------------------------------
#define		ECRC_EN			BIT(7)	// ECRC Enable.
#define		MDIO_RFLAG			BIT(6)	// MDIO Read Flag.
#define		MDIO_WFLAG		BIT(5)	// MDIO Write Flag.
#define		MDIO_ADDR_SHIFT	0	// MDIO Address.
#define		MDIO_ADDR_Mask	0x01F

#define		MDIO_RDATA_SHIFT	16	// MDIO Read Data.
#define		MDIO_RDATA_Mask	0x0FFFF
#define		MDIO_WDATA_SHIFT	0	// MDIO Write Data.
#define		MDIO_WDATA_Mask	0x0FFFF

//----------------------------------------------------------------------------
//       8192C UART_CTRL bits				(Offset 0x364-36B, 64 bits)
//----------------------------------------------------------------------------
#define		UART_WDATA_H_SHIFT	0	// UART Write Data [31:8], read will be UART Read Data[31:8].
#define		UART_WDATA_H_Mask	0x0FFFFFF

#define		UART_DMA_STS_SHIFT	24	// UART DMA Status.
#define		UART_DMA_STS_Mask	0x07
#define		UART_DMA_MOD			BIT(20)	// UART DMA Mode.
#define		UART_RDMA				BIT(19)	// UART RX DMA Flag.
#define		UART_TDMA				BIT(18)	// UART TX DMA Flag.
#define		UART_RCMD				BIT(17)	// UART Read Command.
#define		UART_WCMD				BIT(16)	// UART Write Command.
#define		UART_ADDR_SHIFT		10	// UART Address.
#define		UART_ADDR_Mask		0x03F
#define		UART_WDATA_L_SHIFT	0	// UART Write Data [7:0], read will be UART Read Data[7:0].
#define		UART_WDATA_L_Mask	0x0FF


//
// 5. PTCL/EDCA Configuration. (Offset 0x400 - 0x4FFh)
//
//----------------------------------------------------------------------------
//       8192C VOQ_INFO bits				(Offset 0x400-403, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C VIQ_INFO bits					(Offset 0x404-407, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C BEQ_INFO bits					(Offset 0x408-40B, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C BKQ_INFO bits					(Offset 0x40C-40F, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C MGQ_INFO bits				(Offset 0x410-413, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C HIQ_INFO bits					(Offset 0x414-417, 32 bits)
//----------------------------------------------------------------------------
#define		Q_CPU_HEAD_SHIFT	24	// MCU will handle q_pkt head.
#define		Q_CPU_HEAD_Mask	0x0FF
#define		Q_PKT_NUM_SHIFT	16	// total Q pkt number in TxPktbuffer.
#define		Q_PKT_NUM_Mask	0x0FF
#define		Q_FW_NUM_SHIFT	8	// total Q FW number in TxPktbuffer.
#define		Q_FW_NUM_Mask	0x0FF
#define		Q_HEAD_PG_SHIFT	0	// head page of the first packet of Q.
#define		Q_HEAD_PG_Mask	0x0FF

//----------------------------------------------------------------------------
//       8192C BCNQ_INFO bits				(Offset 0x418-41A, 24 bits)
//----------------------------------------------------------------------------
#define		BCNQ_PKT_NUM_SHIFT	16	// total BCNQ pkt number in TxPktbuffer.
#define		BCNQ_PKT_NUM_Mask	0x0FF
#define		BCNQ_FW_NUM_SHIFT	8	// total BCNQ FW number in TxPktbuffer.
#define		BCNQ_FW_NUM_Mask		0x0FF
#define		BCNQ_HEAD_PG_SHIFT	0	// head page of the first packet of BCNQ.
#define		BCNQ_HEAD_PG_Mask	0x0FF

//----------------------------------------------------------------------------
//       8192C TXPKT_EMPTY bits				(Offset 0x41B, 8 bits)
//----------------------------------------------------------------------------
#define		BCNQ_EMPTY		BIT(7)	// queue is empty. Report by MAC.
#define		HQQ_EMPTY			BIT(6)	// queue is empty. Report by MAC.
#define		HMGQ_EMPTY		BIT(5)	// queue is empty. Report by MAC.
#define		CMGQ_EMPTY		BIT(4)	// queue is empty. Report by MAC.
#define		BKQ_EMPTY			BIT(3)	// queue is empty. Report by MAC.
#define		BEQ_EMPTY			BIT(2)	// queue is empty. Report by MAC.
#define		VIQ_EMPTY			BIT(1)	// queue is empty. Report by MAC.
#define		VOQ_EMPTY			BIT(0)	// queue is empty. Report by MAC.

//----------------------------------------------------------------------------
//       8192C CPU_MGQ_INFO bits			(Offset 0x41C-41F, 32 bits)
//----------------------------------------------------------------------------
#define		CPUMGQ_FW_NUM_SHIFT		8	// total CPU MGQ fw number in TxPktbuffer.
#define		CPUMGQ_FW_NUM_Mask		0x0FF
#define		CPUMGQ_HEAD_PG_SHIFT	0	// head page of CPU MGQ.
#define		CPUMGQ_HEAD_PG_Mask		0x0FF

//----------------------------------------------------------------------------
//       8192C FWHW_TXQ_CTRL bits			(Offset 0x420-422, 24 bits)
//----------------------------------------------------------------------------
#define		RTS_LIMIT_IN_OFDM		BIT(23)	// 92D_REG, rts rate will not down to cck rate
#define		EN_BCNQ_DL				BIT(22)	// 92D_REG, 1: ptcl consider there is bcn pkt in txbuf; 0: mask bcn pkt num in bcnq
#define		EN_RD_RESP_NAV_BK	BIT(21)	// Control the RD responder NAV criteria for TX.
#define		EN_WR_FREE_TAIL		BIT(20)	// 1:Drive wirte Bcnq_boundary,Hw set free_tail in Boundary-1; 0:no update.
#define		CHK_LOSS_QSEL_SHIFT	17	//
#define		CHK_LOSS_QSEL_Mask	0x07
#define		EN_CHECK_LOSS			BIT(16)	// bit=1, HW will monitor packet loss from TXDMA.
#define		EN_QUEUE_RPT_SHIFT	8	// 1: HW will report tx_status  0: no report.
#define		EN_QUEUE_RPT_Mask	0x07F
#define		EN_AMPDU_RTY_NEW		BIT(7)	// AMPDU aggregation mode with retry MPDUs and new MPDUs.
#define		LIFETIME_EN			BIT(6)	// Enable lift time drop.
#define		EN_CPU_HANDLE_SHIFT	0	// PKT need cpu handle.
#define		EN_CPU_HANDLE_Mask	0x03F

//----------------------------------------------------------------------------
//       8192C HWSEQ_CTRL bits				(Offset 0x423, 8 bits)
//----------------------------------------------------------------------------
#define		HWSEQ_BCN_EN		BIT(6)	//
#define		HWSEQ_HI_EN		BIT(5)	//
#define		HWSEQ_MGT_EN		BIT(4)	//
#define		HWSEQ_BK_EN		BIT(3)	//
#define		HWSEQ_BE_EN		BIT(2)	//
#define		HWSEQ_VI_EN		BIT(1)	//
#define		HWSEQ_VO_EN		BIT(0)	//

//----------------------------------------------------------------------------
//       8192C MULTI_BCNQ_EN bits			(Offset 0x426, 8 bits)
//----------------------------------------------------------------------------
#define		MBID_BCNQ7_EN		BIT(7)	//
#define		MBID_BCNQ6_EN		BIT(6)	//
#define		MBID_BCNQ5_EN		BIT(5)	//
#define		MBID_BCNQ4_EN		BIT(4)	//
#define		MBID_BCNQ3_EN		BIT(3)	//
#define		MBID_BCNQ2_EN		BIT(2)	//
#define		MBID_BCNQ1_EN		BIT(1)	//
#define		MBID_BCNQ0_EN		BIT(0)	//

//----------------------------------------------------------------------------
//       8192C MULTI_BCNQ_OFFSET bits			(Offset 0x427, 8 bits)
//----------------------------------------------------------------------------
#define		MBCNQ_OFFSET_SHIFT	0	// page number between two consecutive BCNQ head page.
#define		MBCNQ_OFFSET_Mask	0x03

//----------------------------------------------------------------------------
//       8192C SPEC_SIFS bits					(Offset 0x428-429, 16 bits)
//----------------------------------------------------------------------------
#define		SPEC_SIFS_OFDM_SHIFT	8	// spec SIFS value for duration calculation.
#define		SPEC_SIFS_OFDM_Mask	0x0FF
#define		SPEC_SIFS_CCK_SHIFT	0	// spec SIFS value for duration calculation.
#define		SPEC_SIFS_CCK_Mask	0x0FF

//----------------------------------------------------------------------------
//       8192C RL bits							(Offset 0x42A-42B, 16 bits)
//----------------------------------------------------------------------------
#define		SRL_SHIFT			8	// Short Retry Limit.
#define		SRL_Mask			0x03F
#define		LRL_SHIFT			0	// Long Retry Limit.
#define		LRL_Mask			0x03F

//----------------------------------------------------------------------------
//       8192C DARFRC bits						(Offset 0x430-437, 64 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C RARFRC bits						(Offset 0x438-43F, 64 bits)
//----------------------------------------------------------------------------
#define		ARF_RC8_SHIFT		24	// Response Auto Rate Fallback Retry Count.
#define		ARF_RC8_Mask		0x01F
#define		ARF_RC7_SHIFT		16	// Response Auto Rate Fallback Retry Count.
#define		ARF_RC7_Mask		0x01F
#define		ARF_RC6_SHIFT		8	// Response Auto Rate Fallback Retry Count.
#define		ARF_RC6_Mask		0x01F
#define		ARF_RC5_SHIFT		0	// Response Auto Rate Fallback Retry Count.
#define		ARF_RC5_Mask		0x01F

#define		ARF_RC4_SHIFT		24	// Response Auto Rate Fallback Retry Count.
#define		ARF_RC4_Mask		0x01F
#define		ARF_RC3_SHIFT		16	// Response Auto Rate Fallback Retry Count.
#define		ARF_RC3_Mask		0x01F
#define		ARF_RC2_SHIFT		8	// Response Auto Rate Fallback Retry Count.
#define		ARF_RC2_Mask		0x01F
#define		ARF_RC1_SHIFT		0	// Response Auto Rate Fallback Retry Count.
#define		ARF_RC1_Mask		0x01F

//----------------------------------------------------------------------------
//       8192C RRSR bits							(Offset 0x440-442, 24 bits)
//----------------------------------------------------------------------------
#define		RRSR_SHORT			BIT(23)	// Respond with short preamble packet. Default is long preamble.
#define		RRSR_RSC_SHIFT		21	// Response frame sub-channel configuration.
#define		RRSR_RSC_Mask		0x03
#define		RRSC_BITMAP_SHIFT	0	// Response Auto Rate Fallback Retry Count.
#define		RRSC_BITMAP_Mask	0x0FFFFF

//----------------------------------------------------------------------------
//       8192C AGGLEN_LMT bits					(Offset 0x458-45B, 32 bits)
//----------------------------------------------------------------------------
#define		AGGLMT_MCS15S_SHIFT	28	// Aggregation Limit for MCS 15 SGI in the units of number of MPDUs.
#define		AGGLMT_MCS15S_Mask	0x0F
#define		AGGLMT_MCS15_SHIFT	24	// Aggregation Limit for MCS 13~15 in the units of number of MPDUs.
#define		AGGLMT_MCS15_Mask	0x0F
#define		AGGLMT_MCS12_SHIFT	20	// Aggregation Limit for MCS 11~12 in the units of number of MPDUs.
#define		AGGLMT_MCS12_Mask	0x0F
#define		AGGLMT_MCS10_SHIFT	16	// Aggregation Limit for MCS 8~10 in the units of number of MPDUs.
#define		AGGLMT_MCS10_Mask	0x0F
#define		AGGLMT_MCS7S_SHIFT	12	// Aggregation Limit for MCS 7 SGI in the units of number of MPDUs.
#define		AGGLMT_MCS7S_Mask	0x0F
#define		AGGLMT_MCS7_SHIFT	8	// Aggregation Limit for MCS 6~7 in the units of number of MPDUs.
#define		AGGLMT_MCS7_Mask		0x0F
#define		AGGLMT_MCS5_SHIFT	4	// Aggregation Limit for MCS 3~5 in the units of number of MPDUs.
#define		AGGLMT_MCS5_Mask		0x0F
#define		AGGLMT_MCS2_SHIFT	0	// Aggregation Limit for MCS 0~2 in the units of number of MPDUs.
#define		AGGLMT_MCS2_Mask		0x0F

//----------------------------------------------------------------------------
//       8192C AMPDU_MIN_SPACE bits				(Offset 0x45C, 8 bits)
//----------------------------------------------------------------------------
#define		MIN_SPACE_SHIFT	0	// The spacing between sub-frame.
#define		MIN_SPACE_Mask	0x07

//----------------------------------------------------------------------------
//       8192C FAST_EDCA_CTRL bits				(Offset 0x460-462, 24 bits)
//----------------------------------------------------------------------------
#define		FAST_EDCA_TO_SHIFT		16	// If pkt timeout, HW will contend channel using fast backoff.
#define		FAST_EDCA_TO_Mask		0x01F
#define		FAST_EDCA_PKT_TH_SHIFT	0	// If pkt in txpktbuffer over thrs. HW will fast backoff.
#define		FAST_EDCA_PKT_TH_Mask	0x0FFFF

//----------------------------------------------------------------------------
//       8192C RD_RESP_PKT_TH bits				(Offset 0x463, 8 bits)
//----------------------------------------------------------------------------
#define		RD_RESP_PKT_TH_LMT_SHIFT		0	// granted ACq pkt num beyond this, responder set More PPDU=1 & piggyback.
#define		RD_RESP_PKT_TH_LMT_Mask		0x01F

//----------------------------------------------------------------------------
//       8192C INIRTS_RATE_SEL bits				(Offset 0x480, 8 bits)
//----------------------------------------------------------------------------
#define		INIRTSMCS_SEL_SHIFT	0	// initial rate for Control type frame transmitted as EDCA originator.
#define		INIRTSMCS_SEL_Mask	0x03F

//----------------------------------------------------------------------------
//       8192C POWER_STAGE1 bits				(Offset 0x4B4-4B6, 24 bits)
//----------------------------------------------------------------------------
#define		PWR_STAGE1_SHIFT		0	// User define Power Stage1.
#define		PWR_STAGE1_Mask		0x0FFFFFF

//----------------------------------------------------------------------------
//       8192C POWER_STAGE2 bits				(Offset 0x4B8-4BA, 24 bits)
//----------------------------------------------------------------------------
#define		PWR_STAGE2_SHIFT		0	// User define Power Stage2.
#define		PWR_STAGE2_Mask		0x0FFFFFF

//----------------------------------------------------------------------------
//       8192C STBC_SETTING bits				(Offset 0x4C4, 8 bits)
//----------------------------------------------------------------------------
#define		NESS_SHIFT			2	//
#define		NESS_Mask			0x03
#define		STBC_CFEND_SHIFT	0	// CFEND STBC.
#define		STBC_CFEND_Mask	0x03

//----------------------------------------------------------------------------
//       8192C PROT_MODE_CTRL bits				(Offset 0x4C8-4C9, 16 bits)
//----------------------------------------------------------------------------
#define		RTS_TXTIME_TH_SHIFT	8	// RTS pkt txtime thrs
#define		RTS_TXTIME_TH_Mask	0x0FF
#define		RTS_LEN_TH_SHIFT		0	// RTS Length thrs.
#define		RTS_LEN_TH_Mask		0x0FF

//----------------------------------------------------------------------------
//       8192C BAR_MODE_CTRL bits				(Offset 0x4CC-4CE, 24 bits)
//----------------------------------------------------------------------------
#define		BAR_RTY_LMT_SHIFT			16	// BAR retry limit.
#define		BAR_RTY_LMT_Mask			0x03
#define		BAR_PKT_TXTIME_TH_SHIFT	8	// BAR pkt txtime thrs.
#define		BAR_PKT_TXTIME_TH_Mask	0x0FF
#define		BAR_PKTNUM_TH_SHIFT		0	// BAR Pktnum Thrs.
#define		BAR_PKTNUM_TH_Mask		0x0FF

//----------------------------------------------------------------------------
//       8192C RA_TRY_RATE_AGG_LMT bits			(Offset 0x4CF, 8 bits)
//----------------------------------------------------------------------------
#define		RA_BAR_RTY_LMT_SHIFT				5	// BAR retry limit.
#define		RA_BAR_RTY_LMT_Mask				0x07
#define		RA_TRY_RATE_AGG_LMT_NUM_SHIFT	0	// aggr pkt num limit for try up rate of when desc try bit=1.
#define		RA_TRY_RATE_AGG_LMT_NUM_Mask	0x01F

//----------------------------------------------------------------------------
//       8192C NQOS_SEQ bits					(Offset 0x4DC-4DD, 16 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C QOS_SEQ bits						(Offset 0x4DE-4DF, 16 bits)
//----------------------------------------------------------------------------
#define		N_QOS_SSN_SHIFT	0	// TXDESC HWSEQEN is set, MAC will overwrite SEQ control by this.
									// TXDESC HWSEQEN is set and QoS field is also set,
									// MAC will overwrite SEQ control by this.
#define		N_QOS_SSN_Mask	0x0FFF

//----------------------------------------------------------------------------
//       8192C NEED_CPU_HANDLE bits				(Offset 0x4E0, 8 bits)
//----------------------------------------------------------------------------
#define		NEED_CPU_HANDLE_Q_SHIFT	0	// 1:HW prepare ok, this q need cpu handle & poll.
#define		NEED_CPU_HANDLE_Q_Mask	0x03F

//----------------------------------------------------------------------------
//       8192C PTCL_ERR_STATUS bits				(Offset 0x4E1, 8 bits)
//----------------------------------------------------------------------------
#define		PTCL_TL_PG_OVF			BIT(7)	// Protocol Total Page out of range.
#define		TXHANG_ERR				BIT(5)	// TX time is greater than 32ms.
#define		HW_RD_NULL_STATUS_ERR	BIT(4)	// HW read null page counter error.
#define		HW_RD_NULL_CTN_ERR		BIT(3)	// HW Contention read null page counter error.
#define		HW_RD_NULL_CNT_ERR		BIT(2)	// HW read null page counter error.
#define		PTCL_TL_PG_ERR				BIT(1)	// Protocol Total page number error.
#define		DESC_QSEL_ERR				BIT(0)	// TXDESC QSEL field mismatch error.

//----------------------------------------------------------------------------
//       8192C PKT_LOSE_RPT bits					(Offset 0x4E2, 8 bits)
//----------------------------------------------------------------------------
#define		PKT_LOSE_BK		BIT(3)	// 1:queue happen pkt lose, 0:no lose.
#define		PKT_LOSE_BE		BIT(2)	// 1:queue happen pkt lose, 0:no lose.
#define		PKT_LOSE_VI		BIT(1)	// 1:queue happen pkt lose, 0:no lose.
#define		PKT_LOSE_VO		BIT(0)	// 1:queue happen pkt lose, 0:no lose.


//----------------------------------------------------------------------------
//       8192C EDCA_VO_PARA bits				(Offset 0x500-503, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C EDCA_VI_PARA bits				(Offset 0x504-507, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C EDCA_BE_PARA bits				(Offset 0x508-50B, 32 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C EDCA_BK_PARA bits				(Offset 0x50C-50F, 32 bits)
//----------------------------------------------------------------------------
#define		TXOP_SHIFT			16	// TXOP Limit.
#define		TXOP_Mask			0x0FFFF
#define		CW_SHIFT			8	// ECWmax/ECWmin
#define		CW_Mask			0x0FF
#define		AIFS_SHIFT			0	// Arbitrary Inter frame space.
#define		AIFS_Mask			0x0FF

//----------------------------------------------------------------------------
//       8192C BCNTCFG bits						(Offset 0x510-511, 16 bits)
//----------------------------------------------------------------------------
#define		BCNECW_SHIFT		8	// Beacon Contention Window.
#define		BCNECW_Mask		0x0F
#define		BCNIFS_SHIFT		0	// Beacon Interframe Space.
#define		BCNIFS_Mask		0x0FF

//----------------------------------------------------------------------------
//       8192C SIFS_CCK bits						(Offset 0x514-515, 16 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C SIFS_OFDM bits					(Offset 0x516-517, 16 bits)
//----------------------------------------------------------------------------
#define		SIFS_TRX_SHIFT		8	// SIFS time required for any immediate response.
#define		SIFS_TRX_Mask		0x0FF
#define		SIFS_CTX_SHIFT		0	// SIFS time required for consecutive TX events.
#define		SIFS_CTX_Mask		0x0FF

//----------------------------------------------------------------------------
//       8192C TX_PTCL_CTRL bits					(Offset 0x520-521, 16 bits)
//----------------------------------------------------------------------------
#define		DIS_CCA				BIT(15)	// bit=1, disable CCA.
#define		SIFS_BK_EN				BIT(12)	// SIFS Break Check Criteria enable for AMPDU aggregation.
#define		TXQ_NAV_MSK_SHIFT	8	// 1:Queue BKF must wait TX Nav finished, 0:Queue not need wait.
#define		TXQ_NAV_MSK_Mask		0x0F
#define		DIS_CW					BIT(7)	// bit=1, disable CW.
#define		RDG_END_TXOP			BIT(6)	// 1:Nav protect all txop, 0:Nav protect to next pkt.
#define		NAV_END_TXOP			BIT(5)	// 1:Nav protect all txop, 0:Nav protect to next pkt.
#define		AC_INBCN_HOLD			BIT(4)	// bit=1, HI queue will be sent first, and then AC queue will be sent.
#define		MGTQ_TXOP_EN			BIT(3)	// bit=1, MGTQ may sent packets by using ACQ TXOP.
#define		MGTQ_RTSMF_HOLD		BIT(2)	// bit=1, MGTQ will be hold until RTS or MF transaction finished.
#define		HIQ_RTSMF_HOLD		BIT(1)	// bit=1, HIQ will be hold until RTS or MF transaction finished.
#define		BCN_RTSMF_HOLD		BIT(0)	// bit=1, BCNQ will be hold until RTS or MF transaction finished.

//----------------------------------------------------------------------------
//       8192C TXPAUSE bits						(Offset 0x522, 8 bits)
//----------------------------------------------------------------------------
#define		STOP_BCN		BIT(6)	// Stop BCNQ Queue.
#define		STOP_HI			BIT(5)	// Stop High Queue.
#define		STOP_MGT		BIT(4)	// Stop Management Queue.
#define		STOP_BK			BIT(3)	// Stop BK Queue.
#define		STOP_BE			BIT(2)	// Stop BE Queue.
#define		STOP_VI			BIT(1)	// Stop VI Queue.
#define		STOP_VO			BIT(0)	// Stop VO Queue.

//----------------------------------------------------------------------------
//       8192C DIS_TXREQ_CLR bits				(Offset 0x523, 8 bits)
//----------------------------------------------------------------------------
#define		DIS_TXREQ_CLR_BCN		BIT(7)	// 1:if queue can't own channel ,txcmd will be cleared;0:Only TX_finish can clear txreq.
#define		DIS_TXREQ_CLR_HI		BIT(5)	// 1:if queue can't own channel ,txcmd will be cleared;0:Only TX_finish can clear txreq.
#define		DIS_TXREQ_CLR_MGQ		BIT(4)	// 1:if queue can't own channel ,txcmd will be cleared;0:Only TX_finish can clear txreq.
#define		DIS_TXREQ_CLR_VO		BIT(3)	// 1:if queue can't own channel ,txcmd will be cleared;0:Only TX_finish can clear txreq.
#define		DIS_TXREQ_CLR_VI		BIT(2)	// 1:if queue can't own channel ,txcmd will be cleared;0:Only TX_finish can clear txreq.
#define		DIS_TXREQ_CLR_BE		BIT(1)	// 1:if queue can't own channel ,txcmd will be cleared;0:Only TX_finish can clear txreq.
#define		DIS_TXREQ_CLR_BK		BIT(0)	// 1:if queue can't own channel ,txcmd will be cleared;0:Only TX_finish can clear txreq.

//----------------------------------------------------------------------------
//       8192C RD_CTRL bits						(Offset 0x524-525, 16 bits)
//----------------------------------------------------------------------------
#define		HIQ_NO_LMT_EN			BIT(12)
#define		EDCCA_MSK_CNTDWN		BIT(11)	// 1: disable, 0: enable,  EDCCA count down.
#define		DIS_TXOP_CFE			BIT(10)	// Disable TXOP_CFE function.
#define		DIS_LSIG_CFE			BIT(9)	// Disable LSIG_TXOP CF-END function.
#define		DIS_STBC_CFE			BIT(8)	// Disable STBC CF-END function.
#define		BKQ_RD_INIT_EN		BIT(7)	//
#define		BEQ_RD_INIT_EN		BIT(6)	//
#define		VIQ_RD_INIT_EN			BIT(5)	//
#define		VOQ_RD_INIT_EN		BIT(4)	//
#define		BKQ_RD_RESP_EN		BIT(3)	//
#define		BEQ_RD_RESP_EN		BIT(2)	//
#define		VIQ_RD_RESP_EN		BIT(1)	//
#define		VOQ_RD_RESP_EN		BIT(0)	//

//----------------------------------------------------------------------------
//       8192C TBTT_PROHIBIT bits				(Offset 0x540-541, 16 bits)
//----------------------------------------------------------------------------
#define		TBTT_HOLD_TIME_SHIFT			8	// TBTT_prohibit_hold, unit is 32 us.
#define		TBTT_HOLD_TIME_Mask			0xFFFFFF
#define		TBTT_PROHIBIT_SETUP_SHIFT	0	// TBTT_prohibit_setup, unit is 32us.
#define		TBTT_PROHIBIT_SETUP_Mask		0x0F

//----------------------------------------------------------------------------
//       8192C BCN_CTRL bits					(Offset 0x550, 8 bits)
//----------------------------------------------------------------------------

#define 	DIS_SUB_STATE		BIT(4)
#define 	DIS_SUB_STATE_N		BIT(1)
#define 	DIS_TSF_UPDATE		BIT(5)
#define 	DIS_TSF_UPDATE_N	BIT(4)
#define 	DIS_ATIM			BIT(0)


#define		BCN0_AUTO_SYNC	BIT(5)	// When this bit is set, TSFTR will update the timestamp in Beacon matched BSSID.
#define		DIS_TSF_UPT		BIT(4)	// 92D_REG, When this bit is set, tsf will not update
#define		EN_BCN_FUNCTION	BIT(3)	// bit=1, TSF and other beacon related functions are then enabled.
#define		EN_TXBCN_RPT		BIT(2)	//
#define		EN_MBSSID			BIT(1)	//
#define		PBCNQSEL			BIT(0)	//

//----------------------------------------------------------------------------
//       8192C USTIME_TSF bits					(Offset 0x551, 8 bits)
//----------------------------------------------------------------------------
#define		USTIME_TSF_TIME_SHIFT	0	// Set the microsecond time unit used by MAC TSF clock.
#define		USTIME_TSF_TIME_Mask		0x03F

//----------------------------------------------------------------------------
//       8192C MBID_NUM bits					(Offset 0x552, 8 bits)
//----------------------------------------------------------------------------
#define		MBID_BCN_NUM_SHIFT	0	// num of virtual interface num excluding the root.
#define		MBID_BCN_NUM_Mask	0x07

//----------------------------------------------------------------------------
//       8192C MBSSID_BCN_SPACE bits			(Offset 0x554-557, 32 bits)
//----------------------------------------------------------------------------
#define		BCN_SPACE2_SHIFT	16	//
#define		BCN_SPACE2_Mask	0x0FFFF
#define		BCN_SPACE1_SHIFT	0	//
#define		BCN_SPACE1_Mask	0x0FFFF

//----------------------------------------------------------------------------
//       8192C ACMHWCTRL bits					(Offset 0x5C0, 8 bits)
//----------------------------------------------------------------------------
#define		VOQ_ACM_STATUS	BIT(6)	// indicates if the used_time >= admitted_time of AC VO when HW ACM.
#define		VIQ_ACM_STATUS	BIT(5)	// indicates if the used_time >= admitted_time of AC VI when HW ACM.
#define		BEQ_ACM_STATUS	BIT(4)	// indicates if the used_time >= admitted_time of AC BE when HW ACM.
#define		VOQ_ACM_EN		BIT(3)	// enable ACM of VO queue.
#define		VIQ_ACM_EN			BIT(2)	// enable ACM of VI queue.
#define		BEQ_ACM_EN		BIT(1)	// enable ACM of BE queue.
#define		ACMHWEN			BIT(0)	// enable hardware to take control of ACM.

//----------------------------------------------------------------------------
//       8192C ACMRSTCTRL bits					(Offset 0x5C1, 8 bits)
//----------------------------------------------------------------------------
#define		VO_ACM_RST_USED_TIME		BIT(2)	// HwEn enabled, sw set this to inform hw to reset used_time.
#define		BE_ACM_RST_USED_TIME		BIT(1)	// HwEn enabled, sw set this to inform hw to reset used_time.
#define		VI_ACM_RST_USED_TIME		BIT(0)	// HwEn enabled, sw set this to inform hw to reset used_time.


//
// 6. WMAC Configuration. (Offset 0x600 - 0x7FFh)
//
//----------------------------------------------------------------------------
//       8192C APSD_CTRL bits					(Offset 0x600, 8 bits)
//----------------------------------------------------------------------------
#define		APSDOFF_STATUS	BIT(7)	//
#define		APSDOFF			BIT(6)	// bit=1, MAC issue sleep signal to disable BB/AFE/RF TRX function.

//----------------------------------------------------------------------------
//       8192C BWOPMODE bits					(Offset 0x603, 8 bits)
//----------------------------------------------------------------------------
#define		BW_20M				BIT(2)	// bit=1 indicate operating in 20Mhz bandwidth.

//----------------------------------------------------------------------------
//       8192C TCR bits							(Offset 0x604-607, 32 bits)
//----------------------------------------------------------------------------
#define		TSFT_CMP_SHIFT	16		// TSFT insertion compensation value.
#define		TSFT_CMP_Mask		0x0FF
#define		WMAC_TCR_ERRSTEN3		BIT(15)	// 92D_REG, Use phytxend_ps to reset mactx state machine
#define		WMAC_TCR_ERRSTEN2		BIT(14)	// 92D_REG, If txd fifo underflow when txtype is cmpba, reset mactx state machine
#define		WMAC_TCR_ERRSTEN1		BIT(13)	// 92D_REG, If txd fifo underflow, reset mactx state machine
#define		WMAC_TCR_ERRSTEN0		BIT(12)	// 92D_REG, Phytxend_ps comes but mactx still active, reset mactx state machine
#define		WMAC_TCR_TXSK_PERPKT		BIT(11)	// 92D_REG, Serche key for each mpdu
#define		ICV					BIT(10)	// Integrity Check Value.
#define		CFE_FORM			BIT(9)	// CF-End Frame Format.
#define		CRC					BIT(8)	// Append 32-bit Cyclic Redundancy Check.
#define		PWRBIT_OW_EN		BIT(7)	// bit=1, MAC overwrite pwr bit according to PWR_ST for data frame.
#define		PWR_ST				BIT(6)	// MAC will overwrite pwr bit accroding to PWR_ST for data frame.
#define		PAD_SEL			BIT(2)	// AMPDU Padding pattern selection.
#define		DIS_GCLK			BIT(1)	// Disable MACTX clock gating control.
#define		TSFRST				BIT(0)	// Reset TSF Timer to zero.

//----------------------------------------------------------------------------
//       8192C RCR bits							(Offset 0x608-60B, 32 bits)
//----------------------------------------------------------------------------
#define		RCR_APP_FCS		BIT(31)	// wmac RX will append FCS after payload.
#define		RCR_APP_MIC		BIT(30)	// bit=1, MACRX will retain the MIC at the bottom of the packet.
#define		RCR_APP_ICV		BIT(29)	// bit=1, MACRX will retain the ICV at the bottom of the packet.
#define		RCR_APP_PHYSTS	BIT(28)	// Append RXFF0 PHY Status Enable.
#define		RCR_APP_BASSN		BIT(27)	// Append SSN of previous TXBA Enable.
#define		RCR_MBID_EN		BIT(24)	// Enable Multiple BssId.
#define		RCR_LSIGEN			BIT(23)	// Enable LSIG TXOP Protection function.
#define		RCR_MFBEN			BIT(22)	// Enable immediate MCS Feedback function.
#define		RCR_BM_DATA_EN	BIT(17)	// BM_DATA_EN.
#define		RCR_UC_DATA_EN	BIT(16)	// Unicast data packet interrupt enable.
#define		RCR_HTC_LOC_CTRL	BIT(14)	// 1: HTC -> MFC, 0: MFC-> HTC.
#define		RCR_AMF			BIT(13)	// Accept Management Frame.
#define		RCR_ACF			BIT(12)	// Accept Control Frame.
#define		RCR_ADF			BIT(11)	// Accept Data Frame.
#define		RCR_AICV			BIT(9)	// Accept Integrity Check Value Error packets.
#define		RCR_ACRC32			BIT(8)	// Accept CRC32 Error packets.

#define		RCR_CBSSID_ADHOC		(BIT(6)|BIT(7))	// Check BSSID.
#define		RCR_CBSSID			BIT(6)	// Check BSSID.
#define		RCR_APWRMGT		BIT(5)	// Accept Power Management Packet.
#define		RCR_ADD3			BIT(4)	// Accept Address 3 Match Packets.
#define		RCR_AB				BIT(3)	// Accept Broadcast packets.
#define		RCR_AM				BIT(2)	// Accept Multicast packets.
#define		RCR_APM			BIT(1)	// Accept Physical Match packets.
#define		RCR_AAP			BIT(0)	// Accept Destination Address packets.

//----------------------------------------------------------------------------
//       8192C RX_PKT_LIMIT bits					(Offset 0x60C, 8 bits)
//----------------------------------------------------------------------------
#define		RXPKTLMT_SHIFT	0	// RX PKT Upper Limit.
#define		RXPKTLMT_Mask		0x03F

// 		MACID Setting Register. (Offset 0x610 - 0x62Fh)
//----------------------------------------------------------------------------
//       8192C MBIDCAMCFG bits					(Offset 0x628-62F, 64 bits)
//----------------------------------------------------------------------------
#define		MBIDCAM_POLL		BIT(31)	// Pooling bit.
#define		MBIDWRITE_EN		BIT(30)	// Write Enable.
#define		MBIDCAM_ADDR_SHIFT	24	// CAM Address.
#define		MBIDCAM_ADDR_Mask	0x01F
#define		MBIDCAM_VALID		BIT(23)	// CAM Valid bit.

// 		MACID Setting Register. (Offset 0x610 - 0x62Fh)

// 		Timing Control Register. (Offset 0x630 - 0x64Fh)
//----------------------------------------------------------------------------
//       8192C SPEC_SIFS bits					(Offset 0x63A-63B, 16 bits)
//----------------------------------------------------------------------------
#define		SPEC_SIFS_OFDM_SHIFT	8	// SIFS value for duration calculation.
#define		SPEC_SIFS_OFDM_Mask	0x0FF
#define		SPEC_SIFS_CCK_SHIFT	0	// SIFS value for duration calculation.
#define		SPEC_SIFS_CCK_Mask	0x0FF

//----------------------------------------------------------------------------
//       8192C RESP_SIFS_CCK bits				(Offset 0x63C-63D, 16 bits)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//       8192C RESP_SIFS_OFDM bits				(Offset 0x63E-63F, 16 bits)
//----------------------------------------------------------------------------
#define		SIFS_R2T_SHIFT		8	// SIFS time required for any immediate response.
#define		SIFS_R2T_Mask		0x0FF
#define		SIFS_T2T_SHIFT		0	// SIFS time required for consecutive TX events.
#define		SIFS_T2T_Mask		0x0FF

// 		Timing Control Register. (Offset 0x630 - 0x64Fh)

// 		WMAC, BA, CCX Related Register. (Offset 0x650 - 0x66Fh)
//----------------------------------------------------------------------------
//       8192C NAV_CTRL bits					(Offset 0x650-651, 16 bits)
//----------------------------------------------------------------------------
#define		NAV_UPPER_SHIFT	8	// When NAV update is beyond this, then update is aborted.
#define		NAV_UPPER_Mask	0x0FF
#define		RTSRST_SHIFT		0	// RTS NAV Reset Time.
#define		RTSRST_Mask		0x0FF

//----------------------------------------------------------------------------
//       8192C BACAMCMD bits					(Offset 0x654-657, 32 bits)
//----------------------------------------------------------------------------
#define		BACAM_POLL			BIT(31)	// Polling bit BA CAM.
#define		BACAM_RST			BIT(17)	// 1 to reset BA CAM. HW set invalid bit for all entries.
#define		BACAM_RW			BIT(16)	// 1=Write, 0=Read.
#define		BACAM_ADDR_SHIFT	0	// BA CAM address. Memory is double-word access.
#define		BACAM_ADDR_Mask	0x03F

//----------------------------------------------------------------------------
//       8192C LBDLY bits						(Offset 0x660, 8 bits)
//----------------------------------------------------------------------------
#define		LBDLY_LMT_SHIFT	0	// Loopback Delay.
#define		LBDLY_LMT_Mask	0x01F

//----------------------------------------------------------------------------
//       8192C FWDLY bits						(Offset 0x661, 8 bits)
//----------------------------------------------------------------------------
#define		FWDLY_LMT_SHIFT	0	// Firmware Delay.
#define		FWDLY_LMT_Mask	0x0F

//----------------------------------------------------------------------------
//       8192C RXERR_RPT bits					(Offset 0x664-667, 32 bits)
//----------------------------------------------------------------------------
#define		RXERR_RPT_SEL_SHIFT	28	//
#define		RXERR_RPT_SEL_Mask	0x0F
#define		RXERR_RPT_RST			BIT(27)	// Write "one" to set the counter to zero.
#define		RPT_COUNTER_SHIFT		0	// reported value corresponding to Report type selected.
#define		RPT_COUNTER_Mask		0x0FFFFF

//----------------------------------------------------------------------------
//       8192C WMAC_TRXPTCL_CTL bits			(Offset 0x668-66F, 64 bits)
//----------------------------------------------------------------------------
#define		ACKBA_TYPSEL_SHIFT		28	// Determine response type for RX BA.
#define		ACKBA_TYPSEL_Mask			0x0F
#define		ACKBA_ACKPCHK_SHIFT		24	//
#define		ACKBA_ACKPCHK_Mask		0x0F
#define		ACKBAR_ACKPCHK_SHIFT		12	//
#define		ACKBAR_ACKPCHK_Mask		0x0F
#define		DIS_TXBA_AMPDUFCSERR		BIT(4)	// Disable to send BA for responding RX AMPDU with FCS error subframe.
#define		DIS_TXBA_RXBARINFULL		BIT(3)	// Disable send BA for responding RX control wrapper BAR dropped by rxpkbuffer full.
#define		DIS_TXCFE_INFULL			BIT(2)	// Disable send CFE for responding RX control wrapper CFE dropped by rxpktbuffer full.
#define		DIS_TXCTS_INFULL			BIT(1)	// Disable send CTS for responding RX control wrapper RTS dropped by rxpktbuffer full.
#define		EN_TXCTS_INTXOP			BIT(0)	// Enable send CTS for responding RX RTS during our TXOP period.

#define		RESP_CHNBUSY_SHIFT		20	// Define channel busy condition in responder to determine RX busy.
#define		RESP_CHNBUSY_Mask		0x03
#define		RESP_DCTS_EN				BIT(19)	// Enable responder send dual CTS for responding Rx RTS.
#define		RESP_DCFE_EN				BIT(18)	// Enable responder send dual CF-END for responding RX CF-END.
#define		RESP_SPLCPEN				BIT(17)	// Enable responder send SPLCP CCK rsp for acknowledging Rx SPLCP CCK.
#define		RESP_SGIEN					BIT(16)	// Enable responder send SGI HT-OFDM rsp for acknowledging Rx SGI HT-OFDM pkt.

// 		WMAC, BA, CCX Related Register. (Offset 0x650 - 0x66Fh)

// 		Security Control Register. (Offset 0x670 - 0x68Fh)
//----------------------------------------------------------------------------
//       8192C CAMCMD bits						(Offset 0x670-673, 32 bits)
//----------------------------------------------------------------------------
#define		SECCAM_POLL			BIT(31)	// Security CAM Polling.
#define		SECCAM_CLR				BIT(30)	// Set 1 to clear all valid bits in CAM.
#define		MFBCAM_CLR			BIT(29)	// Write 1 to clear all MFB value in CAM.
#define		SECCAM_WE				BIT(16)	// Security CAM Write Enable.
#define		SECCAM_ADDR_SHIFT	0	// Security CAM Address Offset.
#define		SECCAM_ADDR_Mask		0x0FF

//----------------------------------------------------------------------------
//       8192C CAMDBG bits						(Offset 0x67C-67F, 32 bits)
//----------------------------------------------------------------------------
#define		SECCAM_INFO				BIT(31)	// Select TX/RX CAM Information.
#define		SEC_KEYFOUND				BIT(30)	// Security Key Found in CAM.
#define		SEC_CONFIG_SHIFT			24	// Security Configuration.
#define		SEC_CONFIG_Mask			0x03F
#define		SEC_KEYCONTENT_SHIFT		0	// Security CAM Address Offset.
#define		SEC_KEYCONTENT_Mask		0x0FFFFFF

//----------------------------------------------------------------------------
//       8192C SECCFG bits						(Offset 0x680, 8 bits)
//----------------------------------------------------------------------------
#define		CHK_KEYID			BIT(8)	// Key search engine need to check if key ID matched
#define		RXBCUSEDK			BIT(7)	// Force RX Broadcast packets Use Default Key
#define		TXBCUSEDK			BIT(6)	// Force Tx Broadcast packets Use Default Key
#define		NOSKMC				BIT(5)	// No Key Search for Multicast.
#define		SKBYA2				BIT(4)	// Search Key by A2.
#define		RXDEC				BIT(3)	// Enable Rx Decryption.
#define		TXENC				BIT(2)	// Enable Tx Encryption.
#define		RXUSEDK			BIT(1)	// Force Rx Use Default Key.
#define		TXUSEDK			BIT(0)	// Force Tx Use Default Key.

// 		Security Control Register. (Offset 0x670 - 0x68Fh)

// 		Power Save Control Register. (Offset 0x690 - 0x69Fh)
//----------------------------------------------------------------------------
//       8192C WOW_CTRL bits					(Offset 0x690, 8 bits)
//----------------------------------------------------------------------------
#define		FORCE_WAKEUP		BIT(7)	// Let Wakeup PIN be controllable by FW/Driver.
#define		UWF				BIT(3)	// Unicast Wakeup Frame.
#define		MAGIC				BIT(2)	// Magic Packet.
#define		WOWEN				BIT(1)	// WoW function on or off.
#define		PMEN				BIT(0)	// Power Management Enable.

//----------------------------------------------------------------------------
//       8192C PSSTATUS bits					(Offset 0x691, 8 bits)
//----------------------------------------------------------------------------
#define		PSSTATUS_SEL_SHIFT	0	// Indicate the present power save mode.
#define		PSSTATUS_SEL_Mask		0x0F

//----------------------------------------------------------------------------
//       8192C PS_RX_INFO bits					(Offset 0x692, 8 bits)
//----------------------------------------------------------------------------
#define		RXCTRLIN0			BIT(4)	// set to 1 if NIC received control frame with correct BSSID and DA.
#define		RXMGTIN0			BIT(3)	// set to 1 if NIC received management frame with correct BSSID and DA.
#define		RXDATAIN2			BIT(2)	// set to 1 if NIC received data frame with correct BSSID and DA.
#define		RXDATAIN1			BIT(1)	// set to 1 if NIC received data frame with correct BSSID and DA.
#define		RXDATAIN0			BIT(0)	// set to 1 if NIC received data frame with correct BSSID and DA.

//----------------------------------------------------------------------------
//       8192C LPNAV_CTRL bits					(Offset 0x694-697, 32 bits)
//----------------------------------------------------------------------------
#define		LPNAV_EN				BIT(31)	// Low Power NAV Mode Enable.
#define		LPNAV_EARLY_SHIFT		16	// in LPNAV mode, MAC will recall BB & RF into RX idle mode.
#define		LPNAV_EARLY_Mask		0x07FFF
#define		LPNAV_TH_SHIFT		0	// When NAV received is greater than this, then MAC enters LPNAV mode.
#define		LPNAV_TH_Mask			0x0FFFF

//----------------------------------------------------------------------------
//       8192C WKFMCAM_CMD bits				(Offset 0x698-69B, 32 bits)
//----------------------------------------------------------------------------
#define		WKFCAM_POLL			BIT(31)	// Wakeup Mask CAM Polling.
#define		WKFCAM_CLR			BIT(30)	// Set to one to clear all valid bits in CAM. After reset will self clear to 0.
#define		WKFCAM_WE				BIT(16)	// Wakeup Mask CAM Write Enable.
#define		WKFCAM_ADDR_SHIFT	0	// Wakeup Mask CAM Address Offset.
#define		WKFCAM_ADDR_Mask		0x03F

// 		Power Save Control Register. (Offset 0x690 - 0x69Fh)

// 		General Purpose Register. (Offset 0x6A0 - 0x6AFh)
//----------------------------------------------------------------------------
//       8192C RXFLTMAP0 bits					(Offset 0x6A0-6A1, 16 bits)
//----------------------------------------------------------------------------
#define		MGTFLTD_EN		BIT(13)	//
#define		MGTFLTC_EN			BIT(12)	//
#define		MGTFLTB_EN			BIT(11)	//
#define		MGTFLTA_EN		BIT(10)	//
#define		MGTFLT9_EN			BIT(9)	//
#define		MGTFLT8_EN			BIT(8)	//
#define		MGTFLT5_EN			BIT(5)	//
#define		MGTFLT4_EN			BIT(4)	//
#define		MGTFLT3_EN			BIT(3)	//
#define		MGTFLT2_EN			BIT(2)	//
#define		MGTFLT1_EN			BIT(1)	//
#define		MGTFLT0_EN			BIT(0)	//

//----------------------------------------------------------------------------
//       8192C RXFLTMAP1 bits					(Offset 0x6A2-6A3, 16 bits)
//----------------------------------------------------------------------------
#define		CTRLFLTF_EN		BIT(15)	//
#define		CTRLFLTE_EN		BIT(14)	//
#define		CTRLFLTD_EN		BIT(13)	//
#define		CTRLFLTC_EN		BIT(12)	//
#define		CTRLFLTB_EN		BIT(11)	//
#define		CTRLFLTA_EN		BIT(10)	//
#define		CTRLFLT9_EN		BIT(9)	//
#define		CTRLFLT8_EN		BIT(8)	//
#define		CTRLFLT7_EN		BIT(7)	//
#define		CTRLFLT6_EN		BIT(6)	//

//----------------------------------------------------------------------------
//       8192C RXFLTMAP2 bits					(Offset 0x6A4-6A5, 16 bits)
//----------------------------------------------------------------------------
#define		DATAFLTF_EN		BIT(15)	//
#define		DATAFLTE_EN		BIT(14)	//
#define		DATAFLTD_EN		BIT(13)	//
#define		DATAFLTC_EN		BIT(12)	//
#define		DATAFLTB_EN		BIT(11)	//
#define		DATAFLTA_EN		BIT(10)	//
#define		DATAFLT9_EN		BIT(9)	//
#define		DATAFLT8_EN		BIT(8)	//
#define		DATAFLT7_EN		BIT(7)	//
#define		DATAFLT6_EN		BIT(6)	//
#define		DATAFLT5_EN		BIT(5)	//
#define		DATAFLT4_EN		BIT(4)	//
#define		DATAFLT3_EN		BIT(3)	//
#define		DATAFLT2_EN		BIT(2)	//
#define		DATAFLT1_EN		BIT(1)	//
#define		DATAFLT0_EN		BIT(0)	//

//----------------------------------------------------------------------------
//       8192C BCN_PSR_RPT bits					(Offset 0x6A8-6AB, 32 bits)
//----------------------------------------------------------------------------
#define		DTIM_CNT_SHIFT	24	//
#define		DTIM_CNT_Mask		0x0FF
#define		DTIM_PERIOD_SHIFT	16	//
#define		DTIM_PERIOD_Mask	0x0FF
#define		DTIM				BIT(15)	//
#define		TIM					BIT(14)	//
#define		PS_AID_SHIFT		0	//
#define		PS_AID_Mask		0x07FF

//----------------------------------------------------------------------------
//       8192C CALB32K_CTRL bits				(Offset 0x6AC-6AF, 32 bits)
//----------------------------------------------------------------------------
#define		CALB32K_POLL		BIT(31)	//
#define		CALB32K_DONE		BIT(30)	//
#define		CAL_TIME_SHIFT		20	// Calibration time.
#define		CAL_TIME_Mask		0x03FF
#define		CAL_REPORT_SHIFT	0	//
#define		CAL_REPORT_Mask	0x0FFFFF

// 		General Purpose Register. (Offset 0x6A0 - 0x6AFh)

//----------------------------------------------------------------------------
//       8192C BT_COEX_TABLE bits				(Offset 0x6C0-6CF, 128 bits)
//----------------------------------------------------------------------------
#define		PRI_MASK_RX_RSP			BIT(30)	// Priority Mask for Rx Response Packet.
#define		PRI_MASK_RX_OFDM			BIT(29)	// Priority Mask for Rx OFDM.
#define		PRI_MASK_RX_CCK			BIT(28)	// Priority Mask for Rx CCK.
#define		PRI_MASK_TX_Q_SHIFT		21	// Priority Mask for Tx Queue.
#define		PRI_MASK_TX_Q_Mask		0x07F
#define		PRI_MASK_TX_NAV_SHIFT	13	// Priority Mask for Tx NAV.
#define		PRI_MASK_TX_NAV_Mask		0x0FF
#define		PRI_MASK_TX_CCK			BIT(12)	// Priority Mask for Tx CCK.
#define		PRI_MASK_TX_OFDM			BIT(11)	// Priority Mask for Tx OFDM.
#define		PRI_MASK_TX_RTY			BIT(10)	// Priority Mask for Tx Retry packet.
#define		PRI_MASK_TX_NUM_SHIFT	6	// Priority Mask for Tx packet num.
#define		PRI_MASK_TX_NUM_Mask		0x0F
#define		PRI_MASK_TX_TYPE_SHIFT	2	// Priority Mask for Tx packet type.
#define		PRI_MASK_TX_TYPE_Mask	0x0F
#define		C_OOB						BIT(1)	// out of band indication (0:in band,1:out of band).
#define		ANT_SEL					BIT(0)	// single or dual antenna selection (0:single,1:dual).

#define		BREAK_TABLE2_SHIFT		16	// Table used to break WLAN activity.
#define		BREAK_TABLE2_Mask			0x0FFFF
#define		BREAK_TABLE1_SHIFT		0	// Table used to break BT activity.
#define		BREAK_TABLE1_Mask			0x0FFFF

//----------------------------------------------------------------------------
//       8192C WMAC_RESP_TXINFO bits			(Offset 0x6D8-6DA, 24 bits)
//----------------------------------------------------------------------------
#define		RESP_TXAGC_B_SHIFT	13	//
#define		RESP_TXAGC_B_Mask		0x01F
#define		RESP_TXAGC_A_SHIFT	8	//
#define		RESP_TXAGC_A_Mask	0x01F
#define		RESP_ANTSEL_B			BIT(7)	//
#define		RESP_ANTSEL_A			BIT(6)	//
#define		RESP_TXANT_CCK_SHIFT	4	//
#define		RESP_TXANT_CCK_Mask	0x03
#define		RESP_TXANT_L_SHIFT	2	//
#define		RESP_TXANT_L_Mask		0x03
#define		RESP_TXANT_HT_SHIFT	0	//
#define		RESP_TXANT_HT_Mask	0x03

//========================================================
// General definitions
//========================================================

#define LAST_ENTRY_OF_TX_PKT_BUFFER	255

//-----------------------------------------------------
//
//	0xFE00h ~ 0xFE55h	USB Configuration
//
//-----------------------------------------------------

//2 USB Information (0xFE17)
#define USB_IS_HIGH_SPEED				0
#define USB_IS_FULL_SPEED				1
#define USB_SPEED_MASK					BIT(5)

#define USB_NORMAL_SIE_EP_MASK		0xF
#define USB_NORMAL_SIE_EP_SHIFT		4

#define USB_TEST_EP_MASK				0x30
#define USB_TEST_EP_SHIFT				4

//2 Special Option
#define USB_AGG_EN						BIT(3)


//2REG_C2HEVT_CLEAR
#define C2H_EVT_HOST_CLOSE		0x00	// Set by driver and notify FW that the driver has read the C2H command message
#define C2H_EVT_FW_CLOSE		0xFF	// Set by FW indicating that FW had set the C2H command message and it's not yet read by driver.

/*
//----------------------------------------------------------------------------
//       8192C Rate Definition
//----------------------------------------------------------------------------
//CCK
#define		RATR_1M				0x00000001
#define		RATR_2M				0x00000002
#define		RATR_55M				0x00000004
#define		RATR_11M				0x00000008
//OFDM
#define		RATR_6M				0x00000010
#define		RATR_9M				0x00000020
#define		RATR_12M				0x00000040
#define		RATR_18M				0x00000080
#define		RATR_24M				0x00000100
#define		RATR_36M				0x00000200
#define		RATR_48M				0x00000400
#define		RATR_54M				0x00000800
//MCS 1 Spatial Stream
#define		RATR_MCS0				0x00001000
#define		RATR_MCS1				0x00002000
#define		RATR_MCS2				0x00004000
#define		RATR_MCS3				0x00008000
#define		RATR_MCS4				0x00010000
#define		RATR_MCS5				0x00020000
#define		RATR_MCS6				0x00040000
#define		RATR_MCS7				0x00080000
//MCS 2 Spatial Stream
#define		RATR_MCS8				0x00100000
#define		RATR_MCS9				0x00200000
#define		RATR_MCS10				0x00400000
#define		RATR_MCS11				0x00800000
#define		RATR_MCS12				0x01000000
#define		RATR_MCS13				0x02000000
#define		RATR_MCS14				0x04000000
#define		RATR_MCS15				0x08000000
// ALL CCK Rate
#define	RATE_ALL_CCK				RATR_1M|RATR_2M|RATR_55M|RATR_11M
#define	RATE_ALL_OFDM_AG			RATR_6M|RATR_9M|RATR_12M|RATR_18M|RATR_24M|\
									RATR_36M|RATR_48M|RATR_54M
#define	RATE_ALL_OFDM_1SS			RATR_MCS0|RATR_MCS1|RATR_MCS2|RATR_MCS3 |\
									RATR_MCS4|RATR_MCS5|RATR_MCS6	|RATR_MCS7
#define	RATE_ALL_OFDM_2SS			RATR_MCS8|RATR_MCS9	|RATR_MCS10|RATR_MCS11|\
									RATR_MCS12|RATR_MCS13|RATR_MCS14|RATR_MCS15
*/

#endif  //WLAN_HAL_INTERNAL_USED

#endif // #ifndef __INC_HAL8192SEREG_H

