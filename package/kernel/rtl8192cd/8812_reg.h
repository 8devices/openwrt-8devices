/*****************************************************************************
 *	Copyright(c) 2009,  RealTEK Technology Inc. All Right Reserved.
 *
 * Module:	__INC_HAL8812REG_H
 *
 *
 * Note:	1. Define Mac register address and corresponding bit mask map
 *			
 *
 * Export:	Constants, macro, functions(API), global variables(None).
 *
 * Abbrev:	
 *
 * History:
 *		Data		Who		Remark 
 * 
 *****************************************************************************/
#ifndef __INC_HAL8812REG_H
#define __INC_HAL8812REG_H

#ifndef WLAN_HAL_INTERNAL_USED

#if 1 //FOR_8812_IQK

#define		RF_AC						0x00	

#define		rPdp_AntA      				0xb00  
#define		rBndA						0xb30
#define		rPdp_AntB 					0xb70
#define		rBndB						0xba0

#define		RF_MODE1					0x10	 
#define		RF_MODE2					0x11	

#define		rTxAGC_B_CCK11_A_CCK2_11	0x86c

#define		RF_TX_G3					0x22

#define		RF_TXPA_G1					0x31	// RF TX PA control
#define		RF_TXPA_G2					0x32	// RF TX PA control
#define		RF_TXPA_G3					0x33	// RF TX PA control
#define		RF_LOBF_9					0x38
#define		RF_RXRF_A3					0x3C	
#define		RF_TRSW						0x3F

#define		RF_TXPA_G1					0x31	// RF TX PA control
#define		RF_TXPA_G2					0x32	// RF TX PA control
#define		RF_TXPA_G3					0x33	// RF TX PA control
#define		RF_LOBF_9					0x38
#define		RF_RXRF_A3					0x3C	
#define		RF_TRSW						0x3F

#define		RF_TXRF_A2					0x41
#define		RF_TXPA_G4					0x46	
#define		RF_TXPA_A4					0x4B	

#define		RF_IQADJ_G1					0x01
#define		RF_IQADJ_G2					0x02
#define		RF_BS_PA_APSET_G1_G4		0x03
#define		RF_BS_PA_APSET_G5_G8		0x04
#define		RF_POW_TRSW					0x05

#define		DP_OFFSET_NUM				9
#define		DP_AP_CUREVE_SELECT_NUM		3
#define		DP_gain_loss				1
#define		DP_PA_BIAS_NUM				4

#define		rTxAGC_B_CCK1_55_Mcs32		0x838

#define		RF_TXBIAS					0x16

//
// RL6052 Register definition
//

#define		RF_IQADJ_G1					0x01	// 
#define		RF_IQADJ_G2					0x02	// 
#define		RF_BS_PA_APSET_G1_G4		0x03
#define		RF_BS_PA_APSET_G5_G8		0x04
#define		RF_POW_TRSW				0x05	// 

#define		RF_GAIN_RX					0x06	// 
#define		RF_GAIN_TX					0x07	// 

#define		RF_TXM_IDAC					0x08	// 
#define		RF_IPA_G						0x09	// 
#define		RF_TXBIAS_G					0x0A
#define		RF_TXPA_AG					0x0B
#define		RF_IPA_A						0x0C	// 
#define		RF_TXBIAS_A					0x0D
#define		RF_BS_PA_APSET_G9_G11		0x0E
#define		RF_BS_IQGEN					0x0F	// 

#define		RF_MODE1					0x10	// 
#define		RF_MODE2					0x11	// 

#define		RF_RX_AGC_HP				0x12	// 
#define		RF_TX_AGC					0x13	// 
#define		RF_BIAS						0x14	// 
#define		RF_IPA						0x15	// 
#define		RF_TXBIAS					0x16
#define		RF_POW_ABILITY				0x17	// 
#define		RF_CHNLBW					0x18	// RF channel and BW switch
#define		RF_TOP						0x19	// 

#define		RF_RX_G1					0x1A	// 
#define		RF_RX_G2					0x1B	// 

#define		RF_RX_BB2					0x1C	// 
#define		RF_RX_BB1					0x1D	// 

#define		RF_RCK1						0x1E	// 
#define		RF_RCK2						0x1F	// 

#define		RF_TX_G1						0x20	// 
#define		RF_TX_G2						0x21	// 
#define		RF_TX_G3						0x22	// 

#define		RF_TX_BB1					0x23	// 


//
// PageB(0xB00)
//
#define		rPdp_AntA      					0xb00  
#define		rPdp_AntA_4    				0xb04
#define		rPdp_AntA_8    				0xb08
#define		rPdp_AntA_C    				0xb0c
#define		rPdp_AntA_10    				0xb10
#define		rPdp_AntA_14    				0xb14
#define		rPdp_AntA_18    				0xb18
#define		rPdp_AntA_1C    				0xb1c
#define		rPdp_AntA_20    				0xb20
#define		rPdp_AntA_24    				0xb24

#define		rConfig_Pmpd_AntA 			0xb28
#define		rConfig_ram64x16				0xb2c

#define		rBndA						0xb30
#define		rHssiPar						0xb34

#define		rConfig_AntA 					0xb68
#define		rConfig_AntB 					0xb6c

#define		rPdp_AntB 					0xb70
#define		rPdp_AntB_4 					0xb74
#define		rPdp_AntB_8 					0xb78
#define		rPdp_AntB_C 					0xb7c
#define		rPdp_AntB_10 					0xb80
#define		rPdp_AntB_14 					0xb84
#define		rPdp_AntB_18 					0xb88
#define		rPdp_AntB_1C 					0xb8c
#define		rPdp_AntB_20 					0xb90
#define		rPdp_AntB_24 					0xb94

#define		rConfig_Pmpd_AntB			0xb98

#define		rBndB						0xba0

#define		rAPK							0xbd8
#define		rPm_Rx0_AntA				0xbdc
#define		rPm_Rx1_AntA				0xbe0
#define		rPm_Rx2_AntA				0xbe4
#define		rPm_Rx3_AntA				0xbe8
#define		rPm_Rx0_AntB				0xbec
#define		rPm_Rx1_AntB				0xbf0
#define		rPm_Rx2_AntB				0xbf4
#define		rPm_Rx3_AntB				0xbf8

//for power tracking 
#define	 	rA_TxScale_Jaguar 		0xc1c  // Pah_A TX scaling factor
#define		rB_TxScale_Jaguar 		0xe1c  // Path_B TX scaling factor
#define	 	rC_TxScale_Jaguar2 		0x181c  // Pah_C TX scaling factor
#define		rD_TxScale_Jaguar2 		0x1A1c  // Path_D TX scaling factor
#define 		rRF_TxGainOffset		0x55


//============================================================
//       Registers for 8188E IQK
//============================================================


#define		rFPGA0_IQK					0xe28
#define		rTx_IQK_Tone_A				0xe30
#define		rRx_IQK_Tone_A				0xe34
#define		rTx_IQK_PI_A				0xe38
#define		rRx_IQK_PI_A				0xe3c

#define		rTx_IQK 					0xe40
#define		rRx_IQK						0xe44
#define		rIQK_AGC_Pts				0xe48
#define		rIQK_AGC_Rsp				0xe4c
#define		rTx_IQK_Tone_B				0xe50
#define		rRx_IQK_Tone_B				0xe54
#define		rTx_IQK_PI_B				0xe58
#define		rRx_IQK_PI_B				0xe5c
#define		rIQK_AGC_Cont				0xe60

#define		rRx_Power_Before_IQK_A		0xea0
#define		rRx_Power_Before_IQK_A_2	0xea4
#define		rRx_Power_After_IQK_A		0xea8
#define		rRx_Power_After_IQK_A_2		0xeac

#define		rTx_Power_Before_IQK_A		0xe94
#define		rTx_Power_After_IQK_A		0xe9c


#define		rTx_Power_Before_IQK_B		0xeb4
#define		rTx_Power_After_IQK_B		0xebc

#define		rRx_Power_Before_IQK_B		0xec0
#define		rRx_Power_Before_IQK_B_2	0xec4
#define		rRx_Power_After_IQK_B		0xec8
#define		rRx_Power_After_IQK_B_2		0xecc


#define		RF_RCK_OS					0x30	// RF TX PA control
#define		RF_TXPA_G1					0x31	// RF TX PA control
#define		RF_TXPA_G2					0x32	// RF TX PA control
#define		RF_TXPA_G3					0x33	// RF TX PA control
#define		RF_TX_BIAS_A				0x35
#define		RF_TX_BIAS_D				0x36
#define		RF_LOBF_9					0x38
#define		RF_RXRF_A3					0x3C	//	
#define		RF_TRSW						0x3F

#define		RF_TXRF_A2					0x41
#define		RF_TXPA_G4					0x46	
#define		RF_TXPA_A4					0x4B	

#define		RF_WE_LUT					0xEF


//#define		rFPGA0_XAB_SwitchControl	0x858	// RF Channel switch
//#define		rFPGA0_XCD_SwitchControl	0x85c

//#define		rFPGA0_XAB_RFInterfaceSW		0x870	// RF Interface Software Control
//#define		rFPGA0_XCD_RFInterfaceSW		0x874

//#define		rFPGA0_XAB_RFParameter		0x878	// RF Parameter
//#define		rFPGA0_XCD_RFParameter		0x87c
#define			rFPGA0_XCD_RFParam			0x87c

//#define		rFPGA0_AnalogParameter1		0x880	// Crystal cap setting RF-R/W protection for parameter4??
//#define		rFPGA0_AnalogParameter2		0x884
//#define		rFPGA0_AnalogParameter3		0x888
//#define		rFPGA0_AdDaClockEn			0x888	// enable ad/da clock1 for dual-phy
//#define		rFPGA0_AnalogParameter4		0x88c

//#define		rFPGA0_XA_LSSIReadBack		0x8a0	// Tranceiver LSSI Readback
//#define		rFPGA0_XB_LSSIReadBack		0x8a4
#define		rFPGA0_XC_LSSIReadBack		0x8a8
#define		rFPGA0_XD_LSSIReadBack		0x8ac

//#define		rFPGA0_PSDReport				0x8b4	// Useless now
//#define		TransceiverA_HSPI_Readback		0x8b8	// Transceiver A HSPI Readback
//#define		TransceiverB_HSPI_Readback		0x8bc	// Transceiver B HSPI Readback
//#define		rFPGA0_XAB_RFInterfaceRB		0x8e0	// Useless now // RF Interface Readback Value
#define		rFPGA0_XCD_RFInterfaceRB		0x8e4	// Useless now


#define		rBlue_Tooth					0xe6c
#define		rRx_Wait_CCA				0xe70
#define		rTx_CCK_RFON				0xe74
#define		rTx_CCK_BBON				0xe78
#define		rTx_OFDM_RFON				0xe7c
#define		rTx_OFDM_BBON				0xe80
#define		rTx_To_Rx					0xe84
#define		rTx_To_Tx					0xe88
#define		rRx_CCK						0xe8c

#define		rRx_OFDM					0xed0
#define		rRx_Wait_RIFS 				0xed4
#define		rRx_TO_Rx 					0xed8
#define		rStandby 					0xedc
#define		rSleep 						0xee0
#define		rPMPD_ANAEN					0xeec

#ifndef REG_MACID_PKT_SLEEP_0
#define REG_MACID_PKT_SLEEP_3       0x0484
#define REG_MACID_PKT_SLEEP_1		0x0488
#define REG_MACID_PKT_SLEEP_2		0x04D0
#define REG_MACID_PKT_SLEEP_0		0x04D4
#endif 
#ifndef MACID_REGION1_LIMIT
#define MACID_REGION1_LIMIT			31
#define MACID_REGION2_LIMIT			63
#define MACID_REGION3_LIMIT			95
#endif
#define REG_EDCA_VO_PARAM			0x0500
#define REG_EDCA_VI_PARAM			0x0504
#define REG_EDCA_BE_PARAM			0x0508
#define REG_EDCA_BK_PARAM			0x050C
#define REG_BCNTCFG					0x0510
#define REG_PIFS					0x0512
#define REG_RDG_PIFS				0x0513
#define REG_SIFS_CTX				0x0514
#define REG_SIFS_TRX				0x0516
#define REG_TSFTR_SNC_OFFSET		0x0518
#define REG_AGGR_BREAK_TIME			0x051A
#define REG_SLOT					0x051B
#define REG_TX_PTCL_CTRL			0x0520
#define REG_TXPAUSE					0x0522
#define REG_DIS_TXREQ_CLR			0x0523
#define REG_RD_CTRL					0x0524


#define REG_TBTT_PROHIBIT			0x0540
#define REG_RD_NAV_NXT				0x0544
#define REG_NAV_PROT_LEN			0x0546
#define REG_BCN_CTRL				0x0550
#define REG_BCN_CTRL_1				0x0551
#define REG_MBID_NUM				0x0552
#define REG_DUAL_TSF_RST			0x0553
#define REG_BCN_INTERVAL			0x0554	// The same as REG_MBSSID_BCN_SPACE
#define REG_DRVERLYINT				0x0558
#define REG_BCNDMATIM				0x0559
#define REG_ATIMWND					0x055A
#define REG_BCN_MAX_ERR				0x055D
#define REG_RXTSF_OFFSET_CCK		0x055E
#define REG_RXTSF_OFFSET_OFDM		0x055F	
#define REG_TSFTR					0x0560
#define REG_TSFTR1					0x0568				// HW Port 1 TSF Register
#define REG_P2P_CTWIN				0x0572 // 1 Byte long (in unit of TU)
#define REG_PSTIMER					0x0580
#define REG_TIMER0					0x0584
#define REG_TIMER1					0x0588
#define REG_ACMHWCTRL				0x05C0
#define REG_NOA_DESC_SEL			0x05CF
#define REG_NOA_DESC_DURATION		0x05E0
#define REG_NOA_DESC_INTERVAL		0x05E4
#define REG_NOA_DESC_START			0x05E8
#define REG_NOA_DESC_COUNT			0x05EC


#define REG_SYS_ISO_CTRL				0x0000
//#define REG_SYS_FUNC_EN					0x0002
#define REG_APS_FSMCO					0x0004
#define REG_SYS_CLKR					0x0008
#define REG_9346CR						0x000A
#define REG_EE_VPD						0x000C
#define REG_AFE_MISC					0x0010
#define REG_SPS0_CTRL					0x0011
#define REG_SPS0_CTRL_6					0x0016
#define REG_POWER_OFF_IN_PROCESS 		0x0017
#define REG_SPS_OCP_CFG					0x0018
#define REG_RSV_CTRL					0x001C
//#define REG_RF_CTRL						0x001F
#define REG_LDOA15_CTRL					0x0020
#define REG_LDOV12D_CTRL				0x0021
//#define REG_LDOHCI12_CTRL				0x0022
#define REG_LPLDO_CTRL					0x0023
//#define REG_AFE_XTAL_CTRL				0x0024
//#define REG_AFE_PLL_CTRL				0x0028
#define REG_MAC_PHY_CTRL				0x002c //for 92d, DMDP,SMSP,DMSP contrl
#define REG_EFUSE_CTRL					0x0030
#define REG_EFUSE_TEST					0x0034
#define REG_PWR_DATA					0x0038
#define REG_CAL_TIMER					0x003C
#define REG_ACLK_MON					0x003E
#define REG_GPIO_MUXCFG					0x0040
#define REG_GPIO_IO_SEL					0x0042
#define REG_MAC_PINMUX_CFG				0x0043
#define REG_GPIO_PIN_CTRL				0x0044
#define REG_GPIO_INTM					0x0048
#define REG_LEDCFG0						0x004C
#define REG_LEDCFG1						0x004D
#define REG_LEDCFG2						0x004E
#define REG_LEDCFG3						0x004F
#define REG_FSIMR						0x0050
#define REG_FSISR						0x0054
#define REG_HSIMR						0x0058
#define REG_HSISR						0x005c
#define REG_GPIO_PIN_CTRL_2				0x0060 // RTL8723 WIFI/BT/GPS Multi-Function GPIO Pin Control.
#define REG_GPIO_IO_SEL_2				0x0062 // RTL8723 WIFI/BT/GPS Multi-Function GPIO Select.
#define REG_MULTI_FUNC_CTRL				0x0068 // RTL8723 WIFI/BT/GPS Multi-Function control source.
#define REG_GPIO_OUTPUT					0x006c
#define REG_AFE_XTAL_CTRL_EXT			0x0078 //RTL8188E
#define REG_XCK_OUT_CTRL				0x007c //RTL8188E
//#define REG_MCUFWDL						0x0080
#define	REG_WOL_EVENT					0x0081 //RTL8188E
#define REG_MCUTSTCFG					0x0084
#define REG_HMEBOX_EXT_0				0x0088
#define REG_HMEBOX_EXT_1				0x008A
#define REG_HMEBOX_EXT_2				0x008C
#define REG_HMEBOX_EXT_3				0x008E
#define REG_HOST_SUSP_CNT				0x00BC	// RTL8192C Host suspend counter on FPGA platform
#define REG_HIMR_88E					0x00B0 //RTL8188E
#define REG_HISR_88E					0x00B4 //RTL8188E
#define REG_HIMRE_88E					0x00B8 //RTL8188E
#define REG_HISRE_88E					0x00BC //RTL8188E
#define REG_EFUSE_ACCESS				0x00CF	// Efuse access protection for RTL8723
#define REG_BIST_SCAN					0x00D0
#define REG_BIST_RPT					0x00D4
#define REG_BIST_ROM_RPT				0x00D8
#define REG_USB_SIE_INTF				0x00E0
#define REG_PCIE_MIO_INTF				0x00E4
#define REG_PCIE_MIO_INTD				0x00E8
#define REG_HPON_FSM					0x00EC
#define REG_SYS_CFG						0x00F0
#define REG_GPIO_OUTSTS					0x00F4	// For RTL8723 only.
#define REG_TYPE_ID						0x00FC	

#define		rConfig_AntA 				0xb68
#define		rConfig_AntB 				0xb6c

#endif //FOR_8812_IQK

//============================================================
//
//============================================================

//-----------------------------------------------------
//
//	0x0000h ~ 0x00FFh	System Configuration
//
//-----------------------------------------------------
#define REG_SYS_ISO_CTRL_8812			0x0000	// 2 Byte
#define REG_SYS_FUNC_EN_8812			0x0002	// 2 Byte
#define REG_APS_FSMCO_8812			0x0004	// 4 Byte
#define REG_SYS_CLKR_8812				0x0008	// 2 Byte
#define REG_9346CR_8812				0x000A	// 2 Byte
#define REG_EE_VPD_8812				0x000C	// 2 Byte
#define REG_AFE_MISC_8812				0x0010	// 1 Byte
#define REG_SPS0_CTRL_8812				0x0011	// 7 Byte
#define REG_SPS_OCP_CFG_8812			0x0018	// 4 Byte
#define REG_RSV_CTRL_8812				0x001C	// 3 Byte
#define REG_RF_CTRL_8812				0x001F	// 1 Byte
#define REG_LPLDO_CTRL_8812			0x0023	// 1 Byte
#define REG_AFE_XTAL_CTRL_8812		0x0024	// 4 Byte
#define REG_AFE_PLL_CTRL_8812			0x0028	// 4 Byte
#define REG_MAC_PLL_CTRL_EXT_8812		0x002c 	// 4 Byte
#define REG_EFUSE_CTRL_8812			0x0030
#define REG_EFUSE_TEST_8812			0x0034
#define REG_PWR_DATA_8812				0x0038
#define REG_CAL_TIMER_8812				0x003C
#define REG_ACLK_MON_8812				0x003E
#define REG_GPIO_MUXCFG_8812			0x0040
#define REG_GPIO_IO_SEL_8812			0x0042
#define REG_MAC_PINMUX_CFG_8812		0x0043
#define REG_GPIO_PIN_CTRL_8812			0x0044
#define REG_GPIO_INTM_8812				0x0048
#define REG_LEDCFG0_8812				0x004C
#define REG_LEDCFG1_8812				0x004D
#define REG_LEDCFG2_8812				0x004E
#define REG_LEDCFG3_8812				0x004F
#define REG_FSIMR_8812					0x0050
#define REG_FSISR_8812					0x0054
#define REG_HSIMR_8812					0x0058
#define REG_HSISR_8812					0x005c
#define REG_GPIO_STATUS_8812			0x006C
#define REG_SDIO_CTRL_8812				0x0070
#define REG_OPT_CTRL_8812				0x0074
#define REG_AFE_XTAL_CTRL_EXT_8812		0x0078
#define REG_MCUFWDL_8812				0x0080
#define REG_HIMR0_8812					0x00B0
#define REG_HISR0_8812					0x00B4
#define REG_HIMR1_8812					0x00B8
#define REG_PG_PASSWD_8812				0x00CF
#define REG_HISR1_8812					0x00BC
#define REG_HPON_FSM_8812				0x00EC
#define REG_SYS_CFG_8812				0x00F0


//-----------------------------------------------------
//
//	0x0100h ~ 0x01FFh	MACTOP General Configuration
//
//-----------------------------------------------------
#define REG_CR_8812						0x0100
#define REG_PBP_8812					0x0104
#define REG_PKT_BUFF_ACCESS_CTRL_8812	0x0106
#define REG_TRXDMA_CTRL_8812			0x010C
#define REG_TRXFF_BNDY_8812			0x0114
#define REG_TRXFF_STATUS_8812			0x0118
#define REG_RXFF_PTR_8812				0x011C
#define REG_CPWM_8812					0x012F
#define REG_FWIMR_8812					0x0130
#define REG_FWISR_8812					0x0134
#define REG_FTIMR_8812					0x0138
#define REG_PKTBUF_DBG_CTRL_8812		0x0140
#define REG_RXPKTBUF_CTRL_8812		0x0142
#define REG_PKTBUF_DBG_DATA_L_8812	0x0144
#define REG_PKTBUF_DBG_DATA_H_8812	0x0148

#define REG_TC0_CTRL_8812				0x0150
#define REG_TC1_CTRL_8812				0x0154
#define REG_TC2_CTRL_8812				0x0158
#define REG_TC3_CTRL_8812				0x015C
#define REG_TC4_CTRL_8812				0x0160
#define REG_TCUNIT_BASE_8812			0x0164
#define REG_RSVD3_8812					0x0168
#define REG_C2HEVT_MSG_NORMAL_8812	0x01A0
#define REG_C2HEVT_CLEAR_8812			0x01AF
#define REG_MCUTST_1_8812				0x01C0
#define REG_MCUTST_WOWLAN_8812		0x01C7
#define REG_FMETHR_8812				0x01C8
#define REG_HMETFR_8812				0x01CC
#define REG_HMEBOX_0_8812				0x01D0
#define REG_HMEBOX_1_8812				0x01D4
#define REG_HMEBOX_2_8812				0x01D8
#define REG_HMEBOX_3_8812				0x01DC
#define REG_LLT_INIT_8812				0x01E0
#define REG_HMEBOX_EXT0_8812			0x01F0
#define REG_HMEBOX_EXT1_8812			0x01F4
#define REG_HMEBOX_EXT2_8812			0x01F8
#define REG_HMEBOX_EXT3_8812			0x01FC

//-----------------------------------------------------
//
//	0x0200h ~ 0x027Fh	TXDMA Configuration
//
//-----------------------------------------------------
#define REG_RQPN_8812					0x0200
#define REG_FIFOPAGE_8812				0x0204
#define REG_TDECTRL_8812				0x0208
#define REG_TXDMA_OFFSET_CHK_8812	0x020C
#define REG_TXDMA_STATUS_8812			0x0210
#define REG_RQPN_NPQ_8812				0x0214

//-----------------------------------------------------
//
//	0x0280h ~ 0x02FFh	RXDMA Configuration
//
//-----------------------------------------------------
#define REG_RXDMA_AGG_PG_TH_8812		0x0280
#define REG_FW_UPD_RDPTR_8812		0x0284 // FW shall update this register before FW write RXPKT_RELEASE_POLL to 1
#define REG_RXDMA_CONTROL_8812		0x0286 // Control the RX DMA.
#define REG_RXPKT_NUM_8812			0x0287 // The number of packets in RXPKTBUF.	
#define REG_RXDMA_STATUS_8812			0x0288
#define REG_RXDMA_PRO_8812			0x0290
#define REG_EARLY_MODE_CONTROL_8812	0x02BC
#define REG_RSVD5_8812					0x02F0
#define REG_RSVD6_8812					0x02F4


//-----------------------------------------------------
//
//	0x0300h ~ 0x03FFh	PCIe
//
//-----------------------------------------------------
#define	REG_PCIE_CTRL_REG_8812		0x0300
#define	REG_INT_MIG_8812				0x0304	// Interrupt Migration 
#define	REG_BCNQ_DESA_8812			0x0308	// TX Beacon Descriptor Address
#define	REG_HQ_DESA_8812				0x0310	// TX High Queue Descriptor Address
#define	REG_MGQ_DESA_8812			0x0318	// TX Manage Queue Descriptor Address
#define	REG_VOQ_DESA_8812			0x0320	// TX VO Queue Descriptor Address
#define	REG_VIQ_DESA_8812				0x0328	// TX VI Queue Descriptor Address
#define	REG_BEQ_DESA_8812			0x0330	// TX BE Queue Descriptor Address
#define	REG_BKQ_DESA_8812			0x0338	// TX BK Queue Descriptor Address
#define	REG_RX_DESA_8812				0x0340	// RX Queue	Descriptor Address
#define	REG_MDIO_8812					0x0354	// MDIO for Access PCIE PHY
#define	REG_DBG_SEL_8812				0x0360	// Debug Selection Register
#define	REG_PCIE_HRPWM_8812			0x0361	//PCIe RPWM
#define	REG_PCIE_HCPWM_8812			0x0363	//PCIe CPWM
#define	REG_PCIE_MULTIFET_CTRL_8812	0x036A	//PCIE Multi-Fethc Control

// spec version 11
//-----------------------------------------------------
//
//	0x0400h ~ 0x047Fh	Protocol Configuration
//
//-----------------------------------------------------
#define REG_VOQ_INFORMATION_8812		0x0400
#define REG_VIQ_INFORMATION_8812		0x0404
#define REG_BEQ_INFORMATION_8812		0x0408
#define REG_BKQ_INFORMATION_8812		0x040C
#define REG_MGQ_INFORMATION_8812		0x0410
#define REG_HGQ_INFORMATION_8812		0x0414
#define REG_BCNQ_INFORMATION_8812	0x0418
#define REG_TXPKT_EMPTY_8812			0x041A

#define REG_FWHW_TXQ_CTRL_8812		0x0420
#define REG_HWSEQ_CTRL_8812			0x0423
#define REG_TXPKTBUF_BCNQ_BDNY_8812	0x0424
#define REG_TXPKTBUF_MGQ_BDNY_8812	0x0425
#define REG_LIFECTRL_CTRL_8812			0x0426
#define REG_MULTI_BCNQ_OFFSET_8812	0x0427
#define REG_SPEC_SIFS_8812				0x0428
#define REG_RL_8812						0x042A
#define REG_TXBF_CTRL_8812				0x042C
#define REG_DARFRC_8812				0x0430
#define REG_RARFRC_8812				0x0438
#define REG_RRSR_8812					0x0440
#define REG_ARFR0_8812					0x0444
#define REG_ARFR1_8812					0x044C
#define REG_CCK_CHECK_8812				0x0454
#define REG_AMPDU_MAX_TIME_8812		0x0456
#define REG_AMPDU_MAX_LENGTH_8812	0x0458
#define REG_TXPKTBUF_WMAC_LBK_BF_HD_8812	0x045D
#define REG_NDPA_OPT_CTRL_8812		0x045F
#define REG_FAST_EDCA_CTRL_8812		0x0460
#define REG_RD_RESP_PKT_TH_8812		0x0463
#define REG_DATA_SC_8812				0x0483
#define REG_TXRPT_START_OFFSET		0x04AC
#define REG_POWER_STAGE1_8812		0x04B4
#define REG_POWER_STAGE2_8812		0x04B8
#define REG_AMPDU_BURST_MODE_8812	0x04BC
#define REG_PKT_VO_VI_LIFE_TIME_8812	0x04C0
#define REG_PKT_BE_BK_LIFE_TIME_8812	0x04C2
#define REG_STBC_SETTING_8812			0x04C4
#define REG_PROT_MODE_CTRL_8812		0x04C8
#define REG_MAX_AGGR_NUM_8812		0x04CA
#define REG_RTS_MAX_AGGR_NUM_8812	0x04CB
#define REG_BAR_MODE_CTRL_8812		0x04CC
#define REG_RA_TRY_RATE_AGG_LMT_8812	0x04CF
#define REG_MACID_PKT_DROP0_8812		0x04D0

//-----------------------------------------------------
//
//	0x0500h ~ 0x05FFh	EDCA Configuration
//
//-----------------------------------------------------
#define REG_EDCA_VO_PARAM_8812		0x0500
#define REG_EDCA_VI_PARAM_8812		0x0504
#define REG_EDCA_BE_PARAM_8812		0x0508
#define REG_EDCA_BK_PARAM_8812		0x050C
#define REG_BCNTCFG_8812				0x0510
#define REG_PIFS_8812					0x0512
#define REG_RDG_PIFS_8812				0x0513
#define REG_SIFS_CTX_8812				0x0514
#define REG_SIFS_TRX_8812				0x0516
#define REG_AGGR_BREAK_TIME_8812		0x051A
#define REG_SLOT_8812					0x051B
#define REG_TX_PTCL_CTRL_8812			0x0520
#define REG_TXPAUSE_8812				0x0522
#define REG_DIS_TXREQ_CLR_8812		0x0523
#define REG_RD_CTRL_8812				0x0524
//
// Format for offset 540h-542h:
//	[3:0]:   TBTT prohibit setup in unit of 32us. The time for HW getting beacon content before TBTT.
//	[7:4]:   Reserved.
//	[19:8]:  TBTT prohibit hold in unit of 32us. The time for HW holding to send the beacon packet.
//	[23:20]: Reserved
// Description:
//	              |
//     |<--Setup--|--Hold------------>|
//	--------------|----------------------
//                |
//               TBTT
// Note: We cannot update beacon content to HW or send any AC packets during the time between Setup and Hold.
// Described by Designer Tim and Bruce, 2011-01-14.
//
#define REG_TBTT_PROHIBIT_8812			0x0540
#define REG_RD_NAV_NXT_8812			0x0544
#define REG_NAV_PROT_LEN_8812			0x0546
#define REG_BCN_CTRL_8812				0x0550
#define REG_BCN_CTRL_1_8812			0x0551
#define REG_MBID_NUM_8812				0x0552
#define REG_DUAL_TSF_RST_8812			0x0553
#define REG_BCN_INTERVAL_8812			0x0554
#define REG_DRVERLYINT_8812			0x0558
#define REG_BCNDMATIM_8812			0x0559
#define REG_ATIMWND_8812				0x055A
#define REG_BCN_MAX_ERR_8812			0x055D
#define REG_RXTSF_OFFSET_CCK_8812		0x055E
#define REG_RXTSF_OFFSET_OFDM_8812	0x055F	
#define REG_TSFTR_8812					0x0560
#define REG_CTWND_8812					0x0572
#define REG_PSTIMER_8812				0x0580
#define REG_TIMER0_8812				0x0584
#define REG_TIMER1_8812				0x0588
#define REG_ACMHWCTRL_8812			0x05C0
#define REG_SCH_TXCMD_8812			0x05F8

//-----------------------------------------------------
//
//	0x0600h ~ 0x07FFh	WMAC Configuration
//
//-----------------------------------------------------
#define REG_MAC_CR_8812				0x0600
#define REG_TCR_8812					0x0604
#define REG_RCR_8812					0x0608
#define REG_RX_PKT_LIMIT_8812			0x060C
#define REG_RX_DLK_TIME_8812			0x060D
#define REG_RX_DRVINFO_SZ_8812		0x060F

#define REG_MACID_8812					0x0610
#define REG_BSSID_8812					0x0618
#define REG_MAR_8812					0x0620
#define REG_MBIDCAMCFG_8812			0x0628

#define REG_USTIME_EDCA_8812			0x0638
#define REG_MAC_SPEC_SIFS_8812		0x063A
#define REG_RESP_SIFP_CCK_8812			0x063C
#define REG_RESP_SIFS_OFDM_8812		0x063E
#define REG_ACKTO_8812					0x0640
#define REG_CTS2TO_8812				0x0641
#define REG_EIFS_8812					0x0642

#define REG_NAV_UPPER_8812			0x0652	// unit of 128
#define REG_TRXPTCL_CTL_8812			0x0668

// Security
#define REG_CAMCMD_8812				0x0670
#define REG_CAMWRITE_8812				0x0674
#define REG_CAMREAD_8812				0x0678
#define REG_CAMDBG_8812				0x067C
#define REG_SECCFG_8812				0x0680

// Power
#define REG_WOW_CTRL_8812				0x0690
#define REG_PS_RX_INFO_8812			0x0692
#define REG_UAPSD_TID_8812				0x0693
#define REG_WKFMCAM_CMD_8812			0x0698
#define REG_WKFMCAM_NUM_8812			0x0698
#define REG_WKFMCAM_RWD_8812			0x069C
#define REG_RXFLTMAP0_8812				0x06A0
#define REG_RXFLTMAP1_8812				0x06A2
#define REG_RXFLTMAP2_8812				0x06A4
#define REG_BCN_PSR_RPT_8812			0x06A8
#define REG_BT_COEX_TABLE_8812		0x06C0
#define REG_BFMER0_INFO_8812			0x06E4
#define REG_BFMER1_INFO_8812			0x06EC
#define REG_CSI_RPT_PARAM_BW20_8812	0x06F4
#define REG_CSI_RPT_PARAM_BW40_8812	0x06F8
#define REG_CSI_RPT_PARAM_BW80_8812	0x06FC

// Hardware Port 2
#define REG_MACID1_8812				0x0700
#define REG_BSSID1_8812				0x0708
#define REG_BFMEE_SEL_8812				0x0714
#define REG_SND_PTCL_CTRL_8812		0x0718


// Misc functions
#define		rEDCCA_Jaguar			0x8a4 // EDCCA
#define		bEDCCA_Jaguar			0xffff
#define		rAGC_table_Jaguar		0x82c   // AGC tabel select
#define		bAGC_table_Jaguar		0x3
#define		rA_RFE_Pinmux_Jaguar		0xcb0  // Path_A RFE cotrol pinmux
#define		rB_RFE_Pinmux_Jaguar		0xeb0 // Path_B RFE control pinmux
#define		rA_RFE_Jaguar			0xcb8  // Path_A RFE cotrol
#define		rB_RFE_Jaguar			0xeb8 // Path_B RFE control
#define		b_sel5g_Jaguar    			0x1000 // sel5g
#define		b_LNA_sw_Jaguar			0x8000 // HW/WS control for LNA
#define		rFc_area_Jaguar			0x860   // fc_area 
#define		bFc_area_Jaguar			0x1ffe000
#define		rSingleTone_ContTx_Jaguar 0x914
#define     rfc_cck_enable_JAguar         0x808
#define     rfc_Enabl_VHT_JAguar          0x8c0

// DIG-related
#define		rA_IGI_Jaguar				0xc50	// Initial Gain for path-A
#define		rB_IGI_Jaguar				0xe50	// Initial Gain for path-B
#if defined(CONFIG_WLAN_HAL_8814AE)
#define		rC_IGI_Jaguar				0x1850	// Initial Gain for path-C
#define		rD_IGI_Jaguar				0x1a50	// Initial Gain for path-D
#endif
#define		rOFDM_FalseAlarm1_Jaguar		0xf48  // counter for break
#define		rOFDM_FalseAlarm2_Jaguar		0xf4c  // counter for spoofing
#define		rCCK_FalseAlarm_Jaguar        	0xa5c // counter for cck false alarm
#define		b_FalseAlarm_Jaguar			0xffff
#define		rCCK_CCA_Jaguar				0xa08	// cca threshold

#define		bCCK_CCA_Jaguar				0x00ff0000

#if 0
// TX AGC 
#define		rTxAGC_A_CCK11_CCK1_JAguar	0xc20
#define		rTxAGC_A_Ofdm18_Ofdm6_JAguar	0xc24
#define		rTxAGC_A_Ofdm54_Ofdm24_JAguar	0xc28
#define		rTxAGC_A_MCS3_MCS0_JAguar	0xc2c
#define		rTxAGC_A_MCS7_MCS4_JAguar	0xc30
#define		rTxAGC_A_MCS11_MCS8_JAguar	0xc34
#define		rTxAGC_A_MCS15_MCS12_JAguar	0xc38
#define		rTxAGC_A_Nss1Index3_Nss1Index0_JAguar	0xc3c
#define		rTxAGC_A_Nss1Index7_Nss1Index4_JAguar	0xc40
#define		rTxAGC_A_Nss2Index1_Nss1Index8_JAguar	0xc44
#define		rTxAGC_A_Nss2Index5_Nss2Index2_JAguar	0xc48
#define		rTxAGC_A_Nss2Index9_Nss2Index6_JAguar	0xc4c
#define		rTxAGC_B_CCK11_CCK1_JAguar	0xe20
#define		rTxAGC_B_Ofdm18_Ofdm6_JAguar	0xe24
#define		rTxAGC_B_Ofdm54_Ofdm24_JAguar	0xe28
#define		rTxAGC_B_MCS3_MCS0_JAguar	0xe2c
#define		rTxAGC_B_MCS7_MCS4_JAguar	0xe30
#define		rTxAGC_B_MCS11_MCS8_JAguar	0xe34
#define		rTxAGC_B_MCS15_MCS12_JAguar	0xe38
#define		rTxAGC_B_Nss1Index3_Nss1Index0_JAguar	0xe3c
#define		rTxAGC_B_Nss1Index7_Nss1Index4_JAguar	0xe40
#define		rTxAGC_B_Nss2Index1_Nss1Index8_JAguar	0xe44
#define		rTxAGC_B_Nss2Index5_Nss2Index2_JAguar	0xe48
#define		rTxAGC_B_Nss2Index9_Nss2Index6_JAguar	0xe4c
#define		bTxAGC_byte0_Jaguar	0xff
#define		bTxAGC_byte1_Jaguar	0xff00
#define		bTxAGC_byte2_Jaguar	0xff0000
#define		bTxAGC_byte3_Jaguar	0xff000000
#endif

#define		rOFDMCCKEN_Jaguar 		0x808 // OFDM/CCK block enable
#define		bOFDMEN_Jaguar			0x20000000
#define		bCCKEN_Jaguar			0x10000000
#define		rRxPath_Jaguar			0x808	// Rx antenna
#define		bRxPath_Jaguar			0xff
#define		rTxPath_Jaguar			0x80c	// Tx antenna
#define		bTxPath_Jaguar			0x0fffffff
#define		rCCK_RX_Jaguar			0xa04	// for cck rx path selection
#define		bCCK_RX_Jaguar			0x0c000000 
#define		rVhtlen_Use_Lsig_Jaguar	0x8c3	// Use LSIG for VHT length
#define		RF_AC_Jaguar				0x00	// 
#define		RF_RF_Top_Jaguar			0x07	// 
#define		RF_TXLOK_Jaguar				0x08	// 
#define		RF_TXAPK_Jaguar				0x0B
#define		RF_CHNLBW_Jaguar			0x18	// RF channel and BW switch
#define		RF_RCK1_Jaguar				0x1c	// 
#define		RF_RCK2_Jaguar				0x1d
#define		RF_RCK3_Jaguar				0x1e
#define		RF_ModeTableAddr			0x30
#define		RF_ModeTableData0			0x31
#define		RF_ModeTableData1			0x32
#define		RF_TxLCTank_Jaguar			0x54
#define		RF_APK_Jaguar				0x63
#define		RF_LCK						0xB4
#define		RF_WeLut_Jaguar				0xEF
#define		bRF_CHNLBW_MOD_AG_Jaguar	0x70300
#define		bRF_CHNLBW_BW				0xc00

//-----------------------------------------------------
//
//	0xFE00h ~ 0xFE55h	USB Configuration
//
//-----------------------------------------------------
/*
#define REG_USB_INFO					0xFE17
#define REG_USB_SPECIAL_OPTION		0xFE55
#define REG_USB_DMA_AGG_TO			0xFE5B
#define REG_USB_AGG_TO					0xFE5C
#define REG_USB_AGG_TH					0xFE5D


// For normal chip
#define REG_NORMAL_SIE_VID				0xFE60		// 0xFE60~0xFE61
#define REG_NORMAL_SIE_PID				0xFE62		// 0xFE62~0xFE63
#define REG_NORMAL_SIE_OPTIONAL		0xFE64
#define REG_NORMAL_SIE_EP				0xFE65		// 0xFE65~0xFE67
#define REG_NORMAL_SIE_PHY			0xFE68		// 0xFE68~0xFE6B
#define REG_NORMAL_SIE_OPTIONAL2		0xFE6C
#define REG_NORMAL_SIE_GPS_EP			0xFE6D	// 0xFE6D, for RTL8723 only.
#define REG_NORMAL_SIE_MAC_ADDR		0xFE70		// 0xFE70~0xFE75
#define REG_NORMAL_SIE_STRING			0xFE80		// 0xFE80~0xFEDF
*/

//-----------------------------------------------------
//
//	Redifine 8192C register definition for compatibility
//
//-----------------------------------------------------

// TODO: use these definition when using REG_xxx naming rule.
// NOTE: DO NOT Remove these definition. Use later.
#define	EFUSE_CTRL_8812					REG_EFUSE_CTRL_8812		// E-Fuse Control.
#define	EFUSE_TEST_8812					REG_EFUSE_TEST_8812		// E-Fuse Test.
#define	MSR_8812							(REG_CR_8812 + 2)		// Media Status register
#define	ISR_8812							REG_HISR0_8812
#define	TSFR_8812							REG_TSFTR_8812			// Timing Sync Function Timer Register.
					
#define 	PBP_8812							REG_PBP_8812

// Redifine MACID register, to compatible prior ICs.
#define	IDR0_8812							REG_MACID_8812			// MAC ID Register, Offset 0x0050-0x0053
#define	IDR4_8812							(REG_MACID_8812 + 4)	// MAC ID Register, Offset 0x0054-0x0055


//
// 9. Security Control Registers	(Offset: )
//
#define	RWCAM_8812						REG_CAMCMD_8812		//IN 8190 Data Sheet is called CAMcmd
#define	WCAMI_8812						REG_CAMWRITE_8812		// Software write CAM input content
#define	RCAMO_8812						REG_CAMREAD_8812		// Software read/write CAM config
#define	CAMDBG_8812						REG_CAMDBG_8812
#define	SECR_8812							REG_SECCFG_8812		//Security Configuration Register

/*

// Unused register
#define	UnusedRegister					0x1BF
#define	DCAM							UnusedRegister
#define	PSR								UnusedRegister
#define 	BBAddr							UnusedRegister
#define	PhyDataR						UnusedRegister



//----------------------------------------------------------------------------
//       8192C Cmd9346CR bits					(Offset 0xA, 16bit)
//----------------------------------------------------------------------------
#define	CmdEEPROM_En					BIT5	 			// EEPROM enable when set 1
#define	CmdEERPOMSEL					BIT4 			// System EEPROM select, 0: boot from E-FUSE, 1: The EEPROM used is 9346
#define	Cmd9346CR_9356SEL				BIT4

//----------------------------------------------------------------------------
//       8192C GPIO MUX Configuration Register (offset 0x40, 4 byte)
//----------------------------------------------------------------------------
#define	GPIOSEL_GPIO					BIT0
#define	GPIOSEL_ENBT					BIT5

//----------------------------------------------------------------------------
//       8192C GPIO PIN Control Register (offset 0x44, 4 byte)
//----------------------------------------------------------------------------
#define	GPIO_IN							REG_GPIO_PIN_CTRL_8195		// GPIO pins input value
#define	GPIO_OUT						(REG_GPIO_PIN_CTRL_8195+1)	// GPIO pins output value
#define	GPIO_IO_SEL					(REG_GPIO_PIN_CTRL_8195+2)	// GPIO pins output enable when a bit is set to "1"; otherwise, input is configured.
#define	GPIO_MOD						(REG_GPIO_PIN_CTRL_8195+3)
#define	HAL_8192C_HW_GPIO_WPS_BIT		BIT2

//----------------------------------------------------------------------------
//       8723/8188E Host System Interrupt Mask Register (offset 0x58, 32 byte)
//----------------------------------------------------------------------------
#define	HSIMR_GPIO12_0_INT_EN		BIT0
#define	HSIMR_SPS_OCP_INT_EN			BIT5
#define	HSIMR_RON_INT_EN				BIT6
#define	HSIMR_PDN_INT_EN				BIT7
#define	HSIMR_GPIO9_INT_EN			BIT25


//----------------------------------------------------------------------------
//       8723/8188E Host System Interrupt Status Register (offset 0x5C, 32 byte)
//----------------------------------------------------------------------------
#define	HSISR_GPIO12_0_INT			BIT0
#define	HSISR_SPS_OCP_INT				BIT5
#define	HSISR_RON_INT_EN				BIT6
#define	HSISR_PDNINT					BIT7
#define	HSISR_GPIO9_INT				BIT25

//----------------------------------------------------------------------------
//       8195 (MSR) Media Status Register	(Offset 0x4C, 8 bits)  
//----------------------------------------------------------------------------
#define	MSR_NOLINK					0x00
#define	MSR_ADHOC					0x01
#define	MSR_INFRA					0x02
#define	MSR_AP						0x03

//----------------------------------------------------------------------------
//       88EU (MSR) Media Status Register	(Offset 0x4C, 8 bits)  
//----------------------------------------------------------------------------
#define	USB_INTR_CONTENT_HISR_OFFSET		48
#define	USB_INTR_CONTENT_HISRE_OFFSET		52


//
// 6. Adaptive Control Registers  (Offset: 0x0160 - 0x01CF)
//
//----------------------------------------------------------------------------
//       8192C Response Rate Set Register	(offset 0x181, 24bits)
//----------------------------------------------------------------------------
#define	RRSR_1M						BIT0
#define	RRSR_2M						BIT1 
#define	RRSR_5_5M					BIT2 
#define	RRSR_11M					BIT3 
#define	RRSR_6M						BIT4 
#define	RRSR_9M						BIT5 
#define	RRSR_12M					BIT6 
#define	RRSR_18M					BIT7 
#define	RRSR_24M					BIT8 
#define	RRSR_36M					BIT9 
#define	RRSR_48M					BIT10 
#define	RRSR_54M					BIT11
#define	RRSR_MCS0					BIT12
#define	RRSR_MCS1					BIT13
#define	RRSR_MCS2					BIT14
#define	RRSR_MCS3					BIT15
#define	RRSR_MCS4					BIT16
#define	RRSR_MCS5					BIT17
#define	RRSR_MCS6					BIT18
#define	RRSR_MCS7					BIT19


//----------------------------------------------------------------------------
//       8192C Response Rate Set Register	(offset 0x1BF, 8bits)
//----------------------------------------------------------------------------
// WOL bit information
#define	HAL92C_WOL_PTK_UPDATE_EVENT		BIT0
#define	HAL92C_WOL_GTK_UPDATE_EVENT		BIT1


//----------------------------------------------------------------------------
//       8192C Rate Definition
//----------------------------------------------------------------------------
//CCK
#define	RATR_1M						0x00000001
#define	RATR_2M						0x00000002
#define	RATR_55M					0x00000004
#define	RATR_11M					0x00000008
//OFDM 		
#define	RATR_6M						0x00000010
#define	RATR_9M						0x00000020
#define	RATR_12M					0x00000040
#define	RATR_18M					0x00000080
#define	RATR_24M					0x00000100
#define	RATR_36M					0x00000200
#define	RATR_48M					0x00000400
#define	RATR_54M					0x00000800
//MCS 1 Spatial Stream	
#define	RATR_MCS0					0x00001000
#define	RATR_MCS1					0x00002000
#define	RATR_MCS2					0x00004000
#define	RATR_MCS3					0x00008000
#define	RATR_MCS4					0x00010000
#define	RATR_MCS5					0x00020000
#define	RATR_MCS6					0x00040000
#define	RATR_MCS7					0x00080000
//MCS 2 Spatial Stream
#define	RATR_MCS8					0x00100000
#define	RATR_MCS9					0x00200000
#define	RATR_MCS10					0x00400000
#define	RATR_MCS11					0x00800000
#define	RATR_MCS12					0x01000000
#define	RATR_MCS13					0x02000000
#define	RATR_MCS14					0x04000000
#define	RATR_MCS15					0x08000000


// NOTE: For 92CU - Ziv
//CCK
#define RATE_1M						BIT(0)
#define RATE_2M						BIT(1)
#define RATE_5_5M						BIT(2)
#define RATE_11M						BIT(3)
//OFDM 
#define RATE_6M						BIT(4)
#define RATE_9M						BIT(5)
#define RATE_12M						BIT(6)
#define RATE_18M						BIT(7)
#define RATE_24M						BIT(8)
#define RATE_36M						BIT(9)
#define RATE_48M						BIT(10)
#define RATE_54M						BIT(11)
//MCS 1 Spatial Stream
#define RATE_MCS0						BIT(12)
#define RATE_MCS1						BIT(13)
#define RATE_MCS2						BIT(14)
#define RATE_MCS3						BIT(15)
#define RATE_MCS4						BIT(16)
#define RATE_MCS5						BIT(17)
#define RATE_MCS6						BIT(18)
#define RATE_MCS7						BIT(19)
//MCS 2 Spatial Stream
#define RATE_MCS8						BIT(20)
#define RATE_MCS9						BIT(21)
#define RATE_MCS10					BIT(22)
#define RATE_MCS11					BIT(23)
#define RATE_MCS12					BIT(24)
#define RATE_MCS13					BIT(25)
#define RATE_MCS14					BIT(26)
#define RATE_MCS15					BIT(27)




// ALL CCK Rate
#define	RATE_ALL_CCK					RATR_1M|RATR_2M|RATR_55M|RATR_11M 
#define	RATE_ALL_OFDM_AG			RATR_6M|RATR_9M|RATR_12M|RATR_18M|RATR_24M|\
									RATR_36M|RATR_48M|RATR_54M	
#define	RATE_ALL_OFDM_1SS			RATR_MCS0|RATR_MCS1|RATR_MCS2|RATR_MCS3 |\
									RATR_MCS4|RATR_MCS5|RATR_MCS6	|RATR_MCS7	
#define	RATE_ALL_OFDM_2SS			RATR_MCS8|RATR_MCS9	|RATR_MCS10|RATR_MCS11|\
									RATR_MCS12|RATR_MCS13|RATR_MCS14|RATR_MCS15

#define RATE_BITMAP_ALL				0xFFFFF

// Only use CCK 1M rate for ACK
#define RATE_RRSR_CCK_ONLY_1M		0xFFFF1
//----------------------------------------------------------------------------
//       8192C BW_OPMODE bits					(Offset 0x203, 8bit)
//----------------------------------------------------------------------------
#define	BW_OPMODE_20MHZ			BIT2
#define	BW_OPMODE_5G				BIT1

//
// 10. Power Save Control Registers	 (Offset: 0x0260 - 0x02DF)
//
#define	WOW_PMEN				BIT0 // Power management Enable.
#define	WOW_WOMEN			BIT1 // WoW function on or off. 
#define	WOW_MAGIC				BIT2 // Magic packet
#define	WOW_UWF				BIT3 // Unicast Wakeup frame.

*/



//----------------------------------------------------------------------------
//       8195 IMR/ISR bits						(offset 0xB0,  8bits)
//----------------------------------------------------------------------------
#define	IMR_DISABLED_8812					0
// IMR DW0(0x00B0-00B3) Bit 0-31
#define	IMR_TIMER2_8812					BIT31		// Timeout interrupt 2
#define	IMR_TIMER1_8812					BIT30		// Timeout interrupt 1	
#define	IMR_PSTIMEOUT_8812				BIT29		// Power Save Time Out Interrupt
#define	IMR_GTINT4_8812					BIT28		// When GTIMER4 expires, this bit is set to 1	
#define	IMR_GTINT3_8812					BIT27		// When GTIMER3 expires, this bit is set to 1	
#define	IMR_TXBCN0ERR_8812				BIT26		// Transmit Beacon0 Error			
#define	IMR_TXBCN0OK_8812					BIT25		// Transmit Beacon0 OK			
#define	IMR_TSF_BIT32_TOGGLE_8812		BIT24		// TSF Timer BIT32 toggle indication interrupt			
#define	IMR_BCNDMAINT0_8812				BIT20		// Beacon DMA Interrupt 0			
#define	IMR_BCNDERR0_8812					BIT16		// Beacon Queue DMA OK0			
#define	IMR_BCNDMAINT_E_8812				BIT14		// Beacon DMA Interrupt Extension for Win7			
#define	IMR_ATIMEND_8812					BIT12		// CTWidnow End or ATIM Window End
#define	IMR_C2HCMD_8812					BIT10		// CPU to Host Command INT Status, Write 1 clear	
#define	IMR_CPWM2_8812					BIT9			// CPU power Mode exchange INT Status, Write 1 clear	
#define	IMR_CPWM_8812						BIT8			// CPU power Mode exchange INT Status, Write 1 clear	
#define	IMR_HIGHDOK_8812					BIT7			// High Queue DMA OK	
#define	IMR_MGNTDOK_8812					BIT6			// Management Queue DMA OK	
#define	IMR_BKDOK_8812					BIT5			// AC_BK DMA OK		
#define	IMR_BEDOK_8812					BIT4			// AC_BE DMA OK	
#define	IMR_VIDOK_8812					BIT3			// AC_VI DMA OK		
#define	IMR_VODOK_8812					BIT2			// AC_VO DMA OK	
#define	IMR_RDU_8812						BIT1			// Rx Descriptor Unavailable	
#define	IMR_ROK_8812						BIT0			// Receive DMA OK

// IMR DW1(0x00B4-00B7) Bit 0-31
#define	IMR_BCNDMAINT7_8812				BIT27		// Beacon DMA Interrupt 7
#define	IMR_BCNDMAINT6_8812				BIT26		// Beacon DMA Interrupt 6
#define	IMR_BCNDMAINT5_8812				BIT25		// Beacon DMA Interrupt 5
#define	IMR_BCNDMAINT4_8812				BIT24		// Beacon DMA Interrupt 4
#define	IMR_BCNDMAINT3_8812				BIT23		// Beacon DMA Interrupt 3
#define	IMR_BCNDMAINT2_8812				BIT22		// Beacon DMA Interrupt 2
#define	IMR_BCNDMAINT1_8812				BIT21		// Beacon DMA Interrupt 1
#define	IMR_BCNDOK7_8812					BIT20		// Beacon Queue DMA OK Interrup 7
#define	IMR_BCNDOK6_8812					BIT19		// Beacon Queue DMA OK Interrup 6
#define	IMR_BCNDOK5_8812					BIT18		// Beacon Queue DMA OK Interrup 5
#define	IMR_BCNDOK4_8812					BIT17		// Beacon Queue DMA OK Interrup 4
#define	IMR_BCNDOK3_8812					BIT16		// Beacon Queue DMA OK Interrup 3
#define	IMR_BCNDOK2_8812					BIT15		// Beacon Queue DMA OK Interrup 2
#define	IMR_BCNDOK1_8812					BIT14		// Beacon Queue DMA OK Interrup 1
#define	IMR_ATIMEND_E_8812				BIT13		// ATIM Window End Extension for Win7
#define	IMR_TXERR_8812					BIT11		// Tx Error Flag Interrupt Status, write 1 clear.
#define	IMR_RXERR_8812					BIT10		// Rx Error Flag INT Status, Write 1 clear
#define	IMR_TXFOVW_8812					BIT9			// Transmit FIFO Overflow
#define	IMR_RXFOVW_8812					BIT8			// Receive FIFO Overflow





/*===================================================================
=====================================================================
Here the register defines are for 92C. When the define is as same with 92C, 
we will use the 92C's define for the consistency
So the following defines for 92C is not entire!!!!!!
=====================================================================
=====================================================================*/
/*
Based on Datasheet V33---090401
Register Summary
Current IOREG MAP
0x0000h ~ 0x00FFh   System Configuration (256 Bytes)
0x0100h ~ 0x01FFh   MACTOP General Configuration (256 Bytes)
0x0200h ~ 0x027Fh   TXDMA Configuration (128 Bytes)
0x0280h ~ 0x02FFh   RXDMA Configuration (128 Bytes)
0x0300h ~ 0x03FFh   PCIE EMAC Reserved Region (256 Bytes)
0x0400h ~ 0x04FFh   Protocol Configuration (256 Bytes)
0x0500h ~ 0x05FFh   EDCA Configuration (256 Bytes)
0x0600h ~ 0x07FFh   WMAC Configuration (512 Bytes)
0x2000h ~ 0x3FFFh   8051 FW Download Region (8196 Bytes)
*/
//----------------------------------------------------------------------------
//		 8195 (TXPAUSE) transmission pause 	(Offset 0x522, 8 bits)
//----------------------------------------------------------------------------
/*
#define		StopBecon			BIT6
#define		StopHigh				BIT5
#define		StopMgt				BIT4
#define		StopVO				BIT3
#define		StopVI				BIT2
#define		StopBE				BIT1
#define		StopBK				BIT0
*/


//-----------------------------------------------------
//
//	0xFE00h ~ 0xFE55h	USB Configuration
//
//-----------------------------------------------------
/*#define REG_USB_INFO					0xFE17
#define REG_USB_SPECIAL_OPTION			0xFE55
#define REG_USB_DMA_AGG_TO			0xFE5B
#define REG_USB_AGG_TO				0xFE5C
#define REG_USB_AGG_TH				0xFE5D

#define REG_USB_HRPWM				0xFE58
#define REG_USB_HCPWM				0xFE57

//2 USB Information (0xFE17)
#define USB_IS_HIGH_SPEED					0
#define USB_IS_FULL_SPEED					1
#define USB_SPEED_MASK					BIT(5)

#define USB_NORMAL_SIE_EP_MASK			0xF
#define USB_NORMAL_SIE_EP_SHIFT			4

//2 Special Option
#define USB_AGG_EN						BIT(3)

*/
//============================================================================
//       8192C Regsiter Bit and Content definition 
//============================================================================
//-----------------------------------------------------
//
//	0x0000h ~ 0x00FFh	System Configuration
//
//-----------------------------------------------------
/*
//2 SYS_ISO_CTRL
#define ISO_MD2PP						BIT(0)
#define ISO_UA2USB					BIT(1)
#define ISO_UD2CORE					BIT(2)
#define ISO_PA2PCIE					BIT(3)
#define ISO_PD2CORE					BIT(4)
#define ISO_IP2MAC					BIT(5)
#define ISO_DIOP						BIT(6)
#define ISO_DIOE						BIT(7)
#define ISO_EB2CORE					BIT(8)
#define ISO_DIOR						BIT(9)
#define PWC_EV12V						BIT(15)


//2 SYS_FUNC_EN
#define FEN_BBRSTB					BIT(0)
#define FEN_BB_GLB_RSTn				BIT(1)
#define FEN_USBA						BIT(2)
#define FEN_UPLL						BIT(3)
#define FEN_USBD						BIT(4)
#define FEN_DIO_PCIE					BIT(5)
#define FEN_PCIEA						BIT(6)
#define FEN_PPLL						BIT(7)
#define FEN_PCIED						BIT(8)
#define FEN_DIOE						BIT(9)
#define FEN_CPUEN						BIT(10)
#define FEN_DCORE						BIT(11)
#define FEN_ELDR						BIT(12)
#define FEN_DIO_RF					BIT(13)
#define FEN_HWPDN					BIT(14)
#define FEN_MREGEN					BIT(15)

//2 APS_FSMCO
#define PFM_LDALL						BIT(0)
#define PFM_ALDN						BIT(1)
#define PFM_LDKP						BIT(2)
#define PFM_WOWL					BIT(3)
#define EnPDN							BIT(4)
#define PDN_PL						BIT(5)
#define APFM_ONMAC					BIT(8)
#define APFM_OFF						BIT(9)
#define APFM_RSM						BIT(10)
#define AFSM_HSUS						BIT(11)
#define AFSM_PCIE						BIT(12)
#define APDM_MAC					BIT(13)
#define APDM_HOST					BIT(14)
#define APDM_HPDN					BIT(15)
#define RDY_MACON					BIT(16)
#define SUS_HOST						BIT(17)
#define ROP_ALD						BIT(20)
#define ROP_PWR						BIT(21)
#define ROP_SPS						BIT(22)
#define SOP_MRST						BIT(25)
#define SOP_FUSE						BIT(26)
#define SOP_ABG						BIT(27)
#define SOP_AMB						BIT(28)
#define SOP_RCK						BIT(29)
#define SOP_A8M						BIT(30)
#define XOP_BTCK						BIT(31)

//2 SYS_CLKR
#define ANAD16V_EN					BIT(0)
#define ANA8M						BIT(1)
#define MACSLP						BIT(4)
#define LOADER_CLK_EN					BIT(5)


//2 9346CR

#define		BOOT_FROM_EEPROM		BIT(4)
#define		EEPROM_EN				BIT(5)


//2 RF_CTRL
#define RF_EN						BIT(0)
#define RF_RSTB					BIT(1)
#define RF_SDMRSTB				BIT(2)

//2 LDOV12D_CTRL
#define LDV12_EN					BIT(0)
#define LDV12_SDBY				BIT(1)
#define LPLDO_HSM					BIT(2)
#define LPLDO_LSM_DIS				BIT(3)
#define _LDV12_VADJ(x)				(((x) & 0xF) << 4)


//2 EFUSE_TEST (For RTL8723 partially)
#define EF_TRPT						BIT(7)
#define EF_CELL_SEL						(BIT(8)|BIT(9)) // 00: Wifi Efuse, 01: BT Efuse0, 10: BT Efuse1, 11: BT Efuse2
#define LDOE25_EN						BIT(31)
#define EFUSE_SEL(x)					(((x) & 0x3) << 8)
#define EFUSE_SEL_MASK				0x300
#define EFUSE_WIFI_SEL_0				0x0
#define EFUSE_BT_SEL_0					0x1
#define EFUSE_BT_SEL_1					0x2
#define EFUSE_BT_SEL_2					0x3


//2 8051FWDL
//2 MCUFWDL
#define MCUFWDL_EN					BIT(0)
#define MCUFWDL_RDY					BIT(1)
#define FWDL_ChkSum_rpt				BIT(2)
#define MACINI_RDY					BIT(3)
#define BBINI_RDY						BIT(4)
#define RFINI_RDY						BIT(5)
#define WINTINI_RDY					BIT(6)
#define RAM_DL_SEL					BIT(7)
#define ROM_DLEN						BIT(19)
#define CPRST							BIT(23)



//2 REG_SYS_CFG
#define XCLK_VLD						BIT(0)
#define ACLK_VLD						BIT(1)
#define UCLK_VLD						BIT(2)
#define PCLK_VLD						BIT(3)
#define PCIRSTB						BIT(4)
#define V15_VLD						BIT(5)
#define TRP_B15V_EN					BIT(7)
#define SIC_IDLE						BIT(8)
#define BD_MAC2						BIT(9)
#define BD_MAC1						BIT(10)
#define IC_MACPHY_MODE				BIT(11)
#define CHIP_VER						(BIT(12)|BIT(13)|BIT(14)|BIT(15))
#define BT_FUNC						BIT(16)
#define VENDOR_ID						BIT(19)
#define PAD_HWPD_IDN					BIT(22)
#define TRP_VAUX_EN					BIT(23)	// RTL ID
#define TRP_BT_EN						BIT(24)
#define BD_PKG_SEL					BIT(25)
#define BD_HCI_SEL						BIT(26)
#define TYPE_ID						BIT(27)

#define CHIP_VER_RTL_MASK				0xF000	//Bit 12 ~ 15
#define CHIP_VER_RTL_SHIFT				12

*/
//-----------------------------------------------------
//
//	0x0100h ~ 0x01FFh	MACTOP General Configuration
//
//-----------------------------------------------------
/*

//2 Function Enable Registers
//2 CR 0x0100-0x0103


#define HCI_TXDMA_EN					BIT(0)
#define HCI_RXDMA_EN					BIT(1)
#define TXDMA_EN						BIT(2)
#define RXDMA_EN						BIT(3)
#define PROTOCOL_EN					BIT(4)
#define SCHEDULE_EN					BIT(5)
#define MACTXEN						BIT(6)
#define MACRXEN						BIT(7)
#define ENSWBCN						BIT(8)
#define ENSEC							BIT(9)
#define CALTMR_EN					BIT(10)	// 32k CAL TMR enable

// Network type
#define _NETTYPE(x)					(((x) & 0x3) << 16)
#define MASK_NETTYPE					0x30000
#define NT_NO_LINK					0x0
#define NT_LINK_AD_HOC				0x1
#define NT_LINK_AP						0x2
#define NT_AS_AP						0x3


//2 PBP - Page Size Register 0x0104
#define GET_RX_PAGE_SIZE(value)			((value) & 0xF)
#define GET_TX_PAGE_SIZE(value)			(((value) & 0xF0) >> 4)
#define _PSRX_MASK					0xF
#define _PSTX_MASK					0xF0
#define _PSRX(x)						(x)
#define _PSTX(x)						((x) << 4)

#define PBP_64						0x0
#define PBP_128						0x1
#define PBP_256						0x2
#define PBP_512						0x3
#define PBP_1024						0x4


//2 TX/RXDMA 0x010C
#define RXDMA_ARBBW_EN				BIT(0)
#define RXSHFT_EN						BIT(1)
#define RXDMA_AGG_EN					BIT(2)
#define QS_VO_QUEUE					BIT(8)
#define QS_VI_QUEUE					BIT(9)
#define QS_BE_QUEUE					BIT(10)
#define QS_BK_QUEUE					BIT(11)
#define QS_MANAGER_QUEUE			BIT(12)
#define QS_HIGH_QUEUE					BIT(13)

#define HQSEL_VOQ						BIT(0)
#define HQSEL_VIQ						BIT(1)
#define HQSEL_BEQ						BIT(2)
#define HQSEL_BKQ						BIT(3)
#define HQSEL_MGTQ					BIT(4)
#define HQSEL_HIQ						BIT(5)

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


//2 REG_C2HEVT_CLEAR 0x01AF 
#define	C2H_EVT_HOST_CLOSE			0x00	// Set by driver and notify FW that the driver has read the C2H command message
#define	C2H_EVT_FW_CLOSE			0xFF		// Set by FW indicating that FW had set the C2H command message and it's not yet read by driver.



//2 LLT_INIT 0x01E0
#define _LLT_NO_ACTIVE					0x0
#define _LLT_WRITE_ACCESS				0x1
#define _LLT_READ_ACCESS				0x2

#define _LLT_INIT_DATA(x)				((x) & 0xFF)
#define _LLT_INIT_ADDR(x)				(((x) & 0xFF) << 8)
#define _LLT_OP(x)						(((x) & 0x3) << 30)
#define _LLT_OP_VALUE(x)				(((x) >> 30) & 0x3)

*/
//-----------------------------------------------------
//
//	0x0200h ~ 0x027Fh	TXDMA Configuration
//
//-----------------------------------------------------
/*
//2 TDECTL 0x0208
#define BLK_DESC_NUM_SHIFT			4
#define BLK_DESC_NUM_MASK			0xF


//2 TXDMA_OFFSET_CHK 0x020C
#define DROP_DATA_EN				BIT(9)
*/
//-----------------------------------------------------
//
//	0x0280h ~ 0x028Bh	RX DMA Configuration
//
//-----------------------------------------------------
/*
//2 REG_RXDMA_CONTROL, 0x0286h

// Write only. When this bit is set, RXDMA will decrease RX PKT counter by one. Before
// this bit is polled, FW shall update RXFF_RD_PTR first. This register is write pulse and auto clear.
#define	RXPKT_RELEASE_POLL			BIT(0)
// Read only. When RXMA finishes on-going DMA operation, RXMDA will report idle state in 
// this bit. FW can start releasing packets after RXDMA entering idle mode.
#define	RXDMA_IDLE					BIT(1)	
// When this bit is set, RXDMA will enter this mode after on-going RXDMA packet to host 
// completed, and stop DMA packet to host. RXDMA will then report Default: 0;
#define	RW_RELEASE_EN				BIT(2)
*/
//-----------------------------------------------------
//
//	0x0400h ~ 0x047Fh	Protocol Configuration
//
//-----------------------------------------------------
/*
//2 FWHW_TXQ_CTRL 0x0420
#define EN_AMPDU_RTY_NEW			BIT(7)


//2 REG_LIFECTRL_CTRL 0x0426
#define	HAL92C_EN_PKT_LIFE_TIME_BK		BIT3
#define	HAL92C_EN_PKT_LIFE_TIME_BE		BIT2
#define	HAL92C_EN_PKT_LIFE_TIME_VI		BIT1
#define	HAL92C_EN_PKT_LIFE_TIME_VO		BIT0

#define	HAL92C_MSDU_LIFE_TIME_UNIT		128		// in us, said by Tim.


//2 SPEC SIFS 0x0428 
#define _SPEC_SIFS_CCK(x)				((x) & 0xFF)
#define _SPEC_SIFS_OFDM(x)				(((x) & 0xFF) << 8)

//2 RL 0x042A
#define	RETRY_LIMIT_SHORT_SHIFT		8
#define	RETRY_LIMIT_LONG_SHIFT		0

#define _LRL(x)							((x) & 0x3F)
#define _SRL(x)							(((x) & 0x3F) << 8)
*/

//-----------------------------------------------------
//
//	0x0500h ~ 0x05FFh	EDCA Configuration
//
//-----------------------------------------------------
/*
//2 EDCA setting 0x050C
#define AC_PARAM_TXOP_LIMIT_OFFSET		16
#define AC_PARAM_ECW_MAX_OFFSET		12
#define AC_PARAM_ECW_MIN_OFFSET		8
#define AC_PARAM_AIFS_OFFSET			0


//2 BCN_CTRL 0x0550
#define EN_TXBCN_RPT						BIT(2)
#define EN_BCN_FUNCTION					BIT(3)

//2 TxPause 0x0522
#define STOP_BCNQ						BIT(6)
*/


//2 ACMHWCTRL 0x05C0
#define	AcmHw_HwEn_8812				BIT(0)
#define	AcmHw_VoqEn_8812				BIT(1)
#define	AcmHw_ViqEn_8812				BIT(2)
#define	AcmHw_BeqEn_8812				BIT(3)
#define	AcmHw_VoqStatus_8812			BIT(5)
#define	AcmHw_ViqStatus_8812			BIT(6)
#define	AcmHw_BeqStatus_8812			BIT(7)



//-----------------------------------------------------
//
//	0x0600h ~ 0x07FFh	WMAC Configuration
//
//-----------------------------------------------------
/*

//2 TCR 0x0604
#define DIS_GCLK							BIT(1)
#define PAD_SEL							BIT(2)
#define PWR_ST							BIT(6)
#define PWRBIT_OW_EN					BIT(7)
#define ACRC								BIT(8)
#define CFENDFORM						BIT(9)
#define ICV								BIT(10)
*/

//----------------------------------------------------------------------------
//       8195 (RCR) Receive Configuration Register	(Offset 0x608, 32 bits)
//----------------------------------------------------------------------------
/*
#define	RCR_APPFCS					BIT31		// WMAC append FCS after pauload
#define	RCR_APP_MIC					BIT30		// MACRX will retain the MIC at the bottom of the packet. 
#define	RCR_APP_ICV					BIT29       // MACRX will retain the ICV at the bottom of the packet.
#define	RCR_APP_PHYST_RXFF			BIT28       // HY Status is appended before RX packet in RXFF
#define	RCR_APP_BA_SSN				BIT27		// SSN of previous TXBA is appended as after original RXDESC as the 4-th DW of RXDESC.
#define	RCR_RSVD_BIT26				BIT26		// Reserved
*/
#define	RCR_TCPOFLD_EN				BIT25		// Enable TCP checksum offload
/*#define	RCR_ENMBID					BIT24		// Enable Multiple BssId. Only response ACK to the packets whose DID(A1) matching to the addresses in the MBSSID CAM Entries.
#define	RCR_LSIGEN					BIT23		// Enable LSIG TXOP Protection function. Search KEYCAM for each rx packet to check if LSIGEN bit is set.
#define	RCR_MFBEN					BIT22		// Enable immediate MCS Feedback function. When Rx packet with MRQ = 1'b1, then search KEYCAM to find sender's MCS Feedback function and send response.
*/
/*#define	RCR_RSVD_BIT19				BIT19		// Reserved
#define	RCR_TIM_PARSER_EN			BIT18		// RX Beacon TIM Parser.
#define	RCR_BM_DATA_EN				BIT17		// Broadcast data packet interrupt enable.
#define	RCR_UC_DATA_EN				BIT16		// Unicast data packet interrupt enable.
*/
/*#define	RCR_HTC_LOC_CTRL				BIT14       // MFC<--HTC=1 MFC-->HTC=0
#define	RCR_AMF						BIT13		// Accept management type frame
#define	RCR_ACF						BIT12		// Accept control type frame. Control frames BA, BAR, and PS-Poll (when in AP mode) are not controlled by this bit. They are controlled by ADF.
#define	RCR_ADF						BIT11		// Accept data type frame. This bit also regulates BA, BAR, and PS-Poll (AP mode only).
*/
/*#define	RCR_AICV						BIT9		// Accept ICV error packet
#define	RCR_ACRC32					BIT8		// Accept CRC32 error packet 
#define	RCR_CBSSID_BCN				BIT7		// Accept BSSID match packet (Rx beacon, probe rsp)
#define	RCR_CBSSID_DATA				BIT6		// Accept BSSID match packet (Data)
#define	RCR_CBSSID					RCR_CBSSID_DATA		// Accept BSSID match packet
#define	RCR_APWRMGT				BIT5		// Accept power management packet
#define	RCR_ADD3					BIT4		// Accept address 3 match packet
#define	RCR_AB						BIT3		// Accept broadcast packet 
#define	RCR_AM						BIT2		// Accept multicast packet 
#define	RCR_APM					BIT1		// Accept physical match packet
#define	RCR_AAP						BIT0		// Accept all unicast packet 

#define AAP								BIT(0)
#define APM								BIT(1)
#define AM								BIT(2)
#define AB								BIT(3)
#define ADD3								BIT(4)
#define APWRMGT						BIT(5)
#define CBSSID							BIT(6)
#define CBSSID_DATA						BIT(6)
#define CBSSID_BCN						BIT(7)
#define ACRC32							BIT(8)
#define AICV								BIT(9)
#define ADF								BIT(11)
#define ACF								BIT(12)
#define AMF								BIT(13)
#define HTC_LOC_CTRL					BIT(14)
#define UC_DATA_EN						BIT(16)
#define BM_DATA_EN						BIT(17)
#define MFBEN							BIT(22)
#define LSIGEN							BIT(23)
#define EnMBID							BIT(24)
#define APP_BASSN						BIT(27)
#define APP_PHYSTS						BIT(28)
#define APP_ICV							BIT(29)
#define APP_MIC							BIT(30)
*/

//----------------------------------------------------------------------------
//       8195 CAM Config Setting (offset 0x680, 1 byte)
//----------------------------------------------------------------------------      	       		
/*
#define	SCR_TxUseDK					BIT(0)			//Force Tx Use Default Key
#define	SCR_RxUseDK					BIT(1)			//Force Rx Use Default Key
#define	SCR_TxEncEnable				BIT(2)			//Enable Tx Encryption
#define	SCR_RxDecEnable				BIT(3)			//Enable Rx Decryption
#define	SCR_SKByA2						BIT(4)			//Search kEY BY A2
#define	SCR_NoSKMC						BIT(5)			//No Key Search Multicast
#define 	SCR_TXBCUSEDK					BIT(6)			// Force Tx Broadcast packets Use Default Key
#define 	SCR_RXBCUSEDK					BIT(7)			// Force Rx Broadcast packets Use Default Key

#define	CAM_NONE						0x0
#define	CAM_WEP40						0x01
#define	CAM_TKIP						0x02
#define	CAM_AES						0x04
#define	CAM_WEP104					0x05
#define	CAM_SMS4						0x6
        		
#define	TOTAL_CAM_ENTRY				32
#define	HALF_CAM_ENTRY				16	
       		
#define	CAM_CONFIG_USEDK				TRUE
#define	CAM_CONFIG_NO_USEDK			FALSE
       		

#define	SCR_UseDK						0x01
#define	SCR_TxSecEnable				0x02
#define	SCR_RxSecEnable				0x04
*/


//-----------------------------------------------------
//
//	0xFE00h ~ 0xFE55h	RTL8723 SDIO Configuration
//
//-----------------------------------------------------
/*
//SDIO host local register space mapping.
#define 	SDIO_LOCAL_MSK					0x0FFF
#define 	WLAN_IOREG_MSK 	             			0x7FFF
#define 	WLAN_FIFO_MSK			      		0x1FFF	// Aggregation Length[12:0]

#define  	SDIO_WITHOUT_REF_DEVICE_ID   		0	// Without reference to the SDIO Device ID
#define  	SDIO_LOCAL_DEVICE_ID           			0	// 0b[16], 000b[15:13]
#define   WLAN_TX_HIQ_DEVICE_ID 			4	// 0b[16], 100b[15:13]
#define   WLAN_TX_MIQ_DEVICE_ID 			5	// 0b[16], 101b[15:13]
#define   WLAN_TX_LOQ_DEVICE_ID 			6	// 0b[16], 110b[15:13]
#define   WLAN_RX0FF_DEVICE_ID 				7	// 0b[16], 111b[15:13]
#define   WLAN_IOREG_DEVICE_ID 				8	// 1b[16]

//SDIO Tx Free Page Index
#define 	HI_QUEUE_IDX           				0
#define 	MID_QUEUE_IDX         				1
#define 	LOW_QUEUE_IDX 	      				2
#define 	PUBLIC_QUEUE_IDX    				3

#define	SDIO_REG_TX_CTRL					0x0000 // SDIO Tx Control
#define	SDIO_REG_HIMR						0x0014 // SDIO Host Interrupt Mask
#define	SDIO_REG_HISR						0x0018 // SDIO Host Interrupt Service Routine
#define	SDIO_REG_HCPWM					0x0019 // HCI Current Power Mode
#define	SDIO_REG_RX0_REQ_LEN				0x001C // RXDMA Request Length
#define	SDIO_REG_FREE_TXPG				0x0020 // Free Tx Buffer Page
#define	SDIO_REG_HCPWM1					0x0024 // HCI Current Power Mode 1
#define	SDIO_REG_HCPWM2					0x0026 // HCI Current Power Mode 2
#define	SDIO_REG_HTSFR_INFO				0x0030 // HTSF Informaion
#define   SDIO_REG_HRPWM1					0x0080 // HCI Request Power Mode 1
#define   SDIO_REG_HRPWM2					0x0082 // HCI Request Power Mode 2
#define	SDIO_REG_HPS_CLKR					0x0084 // HCI Power Save Clock
#define   SDIO_REG_HSUS_CTRL				0x0086 // SDIO HCI Suspend Control
#define	SDIO_REG_HIMR_ON					0x0090 //SDIO Host Extension Interrupt Mask Always
#define	SDIO_REG_HISR_ON					0x0091 //SDIO Host Extension Interrupt Status Always

#define	SDIO_HIMR_DISABLED				0

// RTL8723/RTL8188E SDIO Host Interrupt Mask Register
#define	SDIO_HIMR_RX_REQUEST_MSK		BIT0
#define	SDIO_HIMR_AVAL_MSK				BIT1
#define	SDIO_HIMR_TXERR_MSK				BIT2
#define	SDIO_HIMR_RXERR_MSK				BIT3
#define	SDIO_HIMR_TXFOVW_MSK			BIT4
#define	SDIO_HIMR_RXFOVW_MSK			BIT5
#define	SDIO_HIMR_TXBCNOK_MSK			BIT6
#define	SDIO_HIMR_TXBCNERR_MSK			BIT7
#define	SDIO_HIMR_BCNERLY_INT_MSK		BIT16
#define	SDIO_HIMR_C2HCMD_MSK			BIT17
#define	SDIO_HIMR_CPWM1_MSK			BIT18
#define	SDIO_HIMR_CPWM2_MSK			BIT19
#define	SDIO_HIMR_HSISR_IND_MSK			BIT20
#define	SDIO_HIMR_GTINT3_IND_MSK		BIT21
#define	SDIO_HIMR_GTINT4_IND_MSK		BIT22
#define	SDIO_HIMR_PSTIMEOUT_MSK		BIT23
#define	SDIO_HIMR_OCPINT_MSK			BIT24
#define	SDIO_HIMR_ATIMEND_MSK			BIT25
#define	SDIO_HIMR_ATIMEND_E_MSK		BIT26
#define	SDIO_HIMR_CTWEND_MSK			BIT27

//RTL8188E SDIO Specific
#define	SDIO_HIMR_MCU_ERR_MSK			BIT28
#define	SDIO_HIMR_TSF_BIT32_TOGGLE_MSK	BIT29

// SDIO Host Interrupt Service Routine
#define	SDIO_HISR_RX_REQUEST				BIT0
#define	SDIO_HISR_AVAL					BIT1
#define	SDIO_HISR_TXERR					BIT2
#define	SDIO_HISR_RXERR					BIT3
#define	SDIO_HISR_TXFOVW				BIT4
#define	SDIO_HISR_RXFOVW				BIT5
#define	SDIO_HISR_TXBCNOK				BIT6
#define	SDIO_HISR_TXBCNERR				BIT7
#define	SDIO_HISR_BCNERLY_INT			BIT16
#define	SDIO_HISR_C2HCMD				BIT17
#define	SDIO_HISR_CPWM1					BIT18
#define	SDIO_HISR_CPWM2					BIT19
#define	SDIO_HISR_HSISR_IND				BIT20
#define	SDIO_HISR_GTINT3_IND				BIT21
#define	SDIO_HISR_GTINT4_IND				BIT22
#define	SDIO_HISR_PSTIMEOUT				BIT23
#define	SDIO_HISR_OCPINT					BIT24
#define	SDIO_HISR_ATIMEND				BIT25
#define	SDIO_HISR_ATIMEND_E				BIT26
#define	SDIO_HISR_CTWEND				BIT27

//RTL8188E SDIO Specific
#define	SDIO_HISR_MCU_ERR					BIT28
#define	SDIO_HISR_TSF_BIT32_TOGGLE		BIT29


// SDIO HCI Suspend Control Register
#define	HCI_RESUME_PWR_RDY				BIT1
#define	HCI_SUS_CTRL						BIT0


#if DEV_BUS_TYPE == RT_SDIO_INTERFACE
	#define 	MAX_TX_AGG_PACKET_NUMBER 	0x8
#else
	#define 	MAX_TX_AGG_PACKET_NUMBER 	0xFF
#endif

*/
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

#define EEPROM_8812_CHANNEL_PLAN		0xB8
#define EEPROM_8812_XTAL_K				0xB9 //[5:0]
#define EEPROM_8812_THERMAL_METER		0xBA
#define EEPROM_8812_LCK_DELTA			0xBB //[3:2]
#define EEPROM_8812_IQK_DELTA			0xBB //[1:0]

#define EEPROM_8812_VERSION				0xC4
#define EEPROM_8812_CUSTOMER_ID         0xC5
#define EEPROM_2G_TxBBSwing				0xC6
#define EEPROM_5G_TxBBSwing				0xC7

#define EEPROM_8812_MACADDRESS                  0xD0 // MAC Address
#endif

#endif //#ifndef WLAN_HAL_INTERNAL_USED

#endif // #ifndef __INC_HAL8195REG_H

