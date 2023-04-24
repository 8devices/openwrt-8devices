#ifndef __RTL_WLAN_BITDEF_H__
#define __RTL_WLAN_BITDEF_H__

/*-------------------------Modification Log-----------------------------------
    Base on MAC_Register.doc SVN391
-------------------------Modification Log-----------------------------------*/

/*--------------------------Include File--------------------------------------*/
#include "HalHWCfg.h"
/*--------------------------Include File--------------------------------------*/

//3 ============Programming guide Start=====================
/*
    1. For all bit define, it should be prefixed by "BIT_"
    2. For all bit mask, it should be prefixed by "BIT_MASK_"
    3. For all bit shift, it should be prefixed by "BIT_SHIFT_"
    4. For other case, prefix is not needed

Example:
#define BIT_SHIFT_MAX_TXDMA         16
#define BIT_MASK_MAX_TXDMA          0x7
#define BIT_MAX_TXDMA(x)                (((x) & BIT_MASK_MAX_TXDMA)<<BIT_SHIFT_MAX_TXDMA)


*/
//3 ============Programming guide End=====================

#if 1
// TODO: temp setting, we need to move to matching file
// Some setting can be replaced after normal MAC reg.h are released

// Loopback mode.
#define		LBMODE_NORMAL			0x00
#define		LBMODE_MAC				0x0B
#define		LBMODE_MAC_DLY			0x03

/*
Network Type
00: No link
01: Link in ad hoc network
10: Link in infrastructure network
11: AP mode
Default: 00b.
*/
#define	MSR_NOLINK				0x00
#define	MSR_ADHOC				0x01
#define	MSR_INFRA				0x02
#define	MSR_AP					0x03

//----------------------------------------------------------------------------
//      (PBP) Packet Buffer Page Register	(Offset 0x104[7:4], 4 bits)  
//----------------------------------------------------------------------------
#define PBP_UNIT                128

//TXBD_IDX Common
#define BIT_SHIFT_QUEUE_HOST_IDX    0
#define BIT_SHIFT_QUEUE_HW_IDX      16
#define BIT_MASK_QUEUE_IDX          0x0FFF
#endif

//---------------------------------------------------
//
// transform from excel
//
//---------------------------------------------------


#define CPU_OPT_WIDTH 0x1F




#if (RTL8881A_SUPPORT)

#define BIT_AFE_MBIAS                         	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_PWC_EV12V                         	BIT(15)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_PWC_EBCOEB                        	BIT(15)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_PWC_EV25V                         	BIT(14)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_PA33V_EN                          	BIT(13)
#define BIT_PA12V_EN                          	BIT(12)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_PC_A15V                           	BIT(12)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_UA33V_EN                          	BIT(11)
#define BIT_UA12V_EN                          	BIT(10)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_Iso_AFE_output_signal             	BIT(10)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_ISO_RFDIO                         	BIT(9)
#define BIT_ISO_EB2CORE                       	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_ISO_DIOE                          	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_ISO_DIOP                          	BIT(6)
#define BIT_ISO_IP2MAC_WA2PP                  	BIT(5)
#define BIT_ISO_PD2CORE                       	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_ISO_PA2PCIE                       	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_ISO_UD2CORE                       	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_ISO_HD2CORE                       	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_ISO_UA2USB                        	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_ISO_WD2PP                         	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_ISO_CTRL                       (Offset 0x0000)

#define BIT_ISO_MD2PP                         	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_FUNC_EN                        (Offset 0x0002)

#define BIT_FEN_MREGEN                        	BIT(15)
#define BIT_FEN_HWPDN                         	BIT(14)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_FUNC_EN                        (Offset 0x0002)

#define BIT_EN_25_1                           	BIT(13)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_FUNC_EN                        (Offset 0x0002)

#define BIT_FEN_ELDR                          	BIT(12)
#define BIT_FEN_DCORE                         	BIT(11)
#define BIT_FEN_CPUEN                         	BIT(10)
#define BIT_FEN_DIOE                          	BIT(9)
#define BIT_FEN_PCIED                         	BIT(8)
#define BIT_FEN_PPLL                          	BIT(7)
#define BIT_FEN_PCIEA                         	BIT(6)
#define BIT_FEN_DIO_PCIE                      	BIT(5)
#define BIT_FEN_USBD                          	BIT(4)
#define BIT_FEN_UPLL                          	BIT(3)
#define BIT_FEN_USBA                          	BIT(2)
#define BIT_FEN_BB_GLB_RSTn                   	BIT(1)
#define BIT_FEN_BBRSTB                        	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_SOP_EABM                          	BIT(31)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_SKP_ALD                           	BIT(31)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_SOP_ACKF                          	BIT(30)
#define BIT_SOP_ERCK                          	BIT(29)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_SOP_ESWR                          	BIT(28)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_SOP_AFEP                          	BIT(28)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_SOP_PWMM                          	BIT(27)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_SOP_EPWM                          	BIT(27)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_SOP_EECK                          	BIT(26)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_ROP_ENXT                          	BIT(25)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_SOP_EXTL                          	BIT(24)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_CHIPOFF_EN                        	BIT(23)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_SYM_OP_RING_12M                   	BIT(22)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_DIS_USB3_SUS_ALD                  	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_ROP_SWPR                          	BIT(21)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_DIS_HW_LPLDM                      	BIT(20)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_SOP_ALD                           	BIT(20)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_OPT_SWRST_WLMCU                   	BIT(19)
#define BIT_RDY_SYSPWR                        	BIT(17)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_EN_WLON                           	BIT(16)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_APDM_HPDN                         	BIT(15)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_HSUS                              	BIT(14)
#define BIT_PDN_SEL                           	BIT(13)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_AFSM_PCIE_SUS_EN                  	BIT(12)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_AFSM_WLSUS_EN                     	BIT(11)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_APFM_SWLPS                        	BIT(10)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_APFM_SWLPS_EN                     	BIT(10)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_APFM_OFFMAC                       	BIT(9)
#define BIT_APFN_ONMAC                        	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_CHIP_PDN_EN                       	BIT(7)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_BT_SUSEN                          	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_RDY_MACDIS                        	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_PD_RF                             	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_RING_CLK_12M_EN                   	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_EnPDN                             	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_PFM_WOWL                          	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_SW_WAKE                           	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_PFM_LDKP                          	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_WL_HCI_ALD                        	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_PFM_ALDN                          	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_PW_CTRL                        (Offset 0x0004)

#define BIT_PFM_LDALL                         	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)

#define BIT_LDO_DUMMY                         	BIT(15)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)

#define BIT_ANA_CLK_EN                        	BIT(15)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)

#define BIT_CPU_CLK_EN                        	BIT(14)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)

#define BIT_SYMREG_CLK_EN                     	BIT(13)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)

#define BIT_RING_CLK_EN                       	BIT(13)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)

#define BIT_HCI_CLK_EN                        	BIT(12)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)

#define BIT_SYS_CLK_EN                        	BIT(12)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)

#define BIT_MAC_CLK_EN                        	BIT(11)
#define BIT_SEC_CLK_EN                        	BIT(10)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)

#define BIT_PHY_SSC_RSTB                      	BIT(9)
#define BIT_EXT_32K_EN                        	BIT(8)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)

#define BIT_EXT32K_EN                         	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)

#define BIT_WL_CLK_TEST                       	BIT(7)
#define BIT_OP_SPS_PWM_EN                     	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)


#define BIT_SHIFT_MAC_CLK_SEL_V1              	6
#define BIT_MASK_MAC_CLK_SEL_V1               	0x3
#define BIT_MAC_CLK_SEL_V1(x)                 	(((x) & BIT_MASK_MAC_CLK_SEL_V1) << BIT_SHIFT_MAC_CLK_SEL_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)

#define BIT_LOADER_CLK_EN                     	BIT(5)
#define BIT_MACSLP                            	BIT(4)
#define BIT_WAKEPAD_EN                        	BIT(3)
#define BIT_ROMD16V_EN                        	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)

#define BIT_CKANA8M_EN                        	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)

#define BIT_ANA8M_EN                          	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CLK_CTRL                       (Offset 0x0008)

#define BIT_CNTD16V_EN                        	BIT(0)

//2 REG_SYS_EEPROM_CTRL                    (Offset 0x000A)


#define BIT_SHIFT_VPDIDX                      	8
#define BIT_MASK_VPDIDX                       	0xff
#define BIT_VPDIDX(x)                         	(((x) & BIT_MASK_VPDIDX) << BIT_SHIFT_VPDIDX)


#define BIT_SHIFT_EEM1_0                      	6
#define BIT_MASK_EEM1_0                       	0x3
#define BIT_EEM1_0(x)                         	(((x) & BIT_MASK_EEM1_0) << BIT_SHIFT_EEM1_0)

#define BIT_AUTOLOAD_SUS                      	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_EEPROM_CTRL                    (Offset 0x000A)

#define BIT_EERPOMSEL                         	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_EEPROM_CTRL                    (Offset 0x000A)

#define BIT_EEPROMSEL                         	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_EEPROM_CTRL                    (Offset 0x000A)

#define BIT_EECS                              	BIT(3)
#define BIT_EESK                              	BIT(2)
#define BIT_EEDI                              	BIT(1)
#define BIT_EEDO                              	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_EE_VPD                             (Offset 0x000C)


#define BIT_SHIFT_VPD_DATA                    	0
#define BIT_MASK_VPD_DATA                     	0xffffffffL
#define BIT_VPD_DATA(x)                       	(((x) & BIT_MASK_VPD_DATA) << BIT_SHIFT_VPD_DATA)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_EE_VPD                             (Offset 0x000C)


#define BIT_SHIFT_VDP_DATA                    	0
#define BIT_MASK_VDP_DATA                     	0xffffffffL
#define BIT_VDP_DATA(x)                       	(((x) & BIT_MASK_VDP_DATA) << BIT_SHIFT_VDP_DATA)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL1                      (Offset 0x0010)

#define BIT_SW18_C2_BIT0                      	BIT(31)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL1                      (Offset 0x0010)


#define BIT_SHIFT_R1_L1_V1                    	30
#define BIT_MASK_R1_L1_V1                     	0x3
#define BIT_R1_L1_V1(x)                       	(((x) & BIT_MASK_R1_L1_V1) << BIT_SHIFT_R1_L1_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL1                      (Offset 0x0010)


#define BIT_SHIFT_SW18_C1                     	29
#define BIT_MASK_SW18_C1                      	0x3
#define BIT_SW18_C1(x)                        	(((x) & BIT_MASK_SW18_C1) << BIT_SHIFT_SW18_C1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL1                      (Offset 0x0010)


#define BIT_SHIFT_C3_L1_V1                    	28
#define BIT_MASK_C3_L1_V1                     	0x3
#define BIT_C3_L1_V1(x)                       	(((x) & BIT_MASK_C3_L1_V1) << BIT_SHIFT_C3_L1_V1)


#define BIT_SHIFT_C2_L1_V1                    	26
#define BIT_MASK_C2_L1_V1                     	0x3
#define BIT_C2_L1_V1(x)                       	(((x) & BIT_MASK_C2_L1_V1) << BIT_SHIFT_C2_L1_V1)


#define BIT_SHIFT_C1_L1_V1                    	24
#define BIT_MASK_C1_L1_V1                     	0x3
#define BIT_C1_L1_V1(x)                       	(((x) & BIT_MASK_C1_L1_V1) << BIT_SHIFT_C1_L1_V1)

#define BIT_reg_type_L_V3                     	BIT(23)
#define BIT_FPWM_L1_V1                        	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL1                      (Offset 0x0010)

#define BIT_SW18_SELD_BIT0                    	BIT(19)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL1                      (Offset 0x0010)


#define BIT_SHIFT_V15adj_L1                   	19
#define BIT_MASK_V15adj_L1                    	0x7
#define BIT_V15adj_L1(x)                      	(((x) & BIT_MASK_V15adj_L1) << BIT_SHIFT_V15adj_L1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL1                      (Offset 0x0010)

#define BIT_SW18_POWOCP                       	BIT(18)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL1                      (Offset 0x0010)


#define BIT_SHIFT_IN_L1                       	16
#define BIT_MASK_IN_L1                        	0x7
#define BIT_IN_L1(x)                          	(((x) & BIT_MASK_IN_L1) << BIT_SHIFT_IN_L1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL1                      (Offset 0x0010)


#define BIT_SHIFT_SW18_OCP                    	15
#define BIT_MASK_SW18_OCP                     	0x7
#define BIT_SW18_OCP(x)                       	(((x) & BIT_MASK_SW18_OCP) << BIT_SHIFT_SW18_OCP)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL1                      (Offset 0x0010)


#define BIT_SHIFT_STD_L1                      	14
#define BIT_MASK_STD_L1                       	0x3
#define BIT_STD_L1(x)                         	(((x) & BIT_MASK_STD_L1) << BIT_SHIFT_STD_L1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL1                      (Offset 0x0010)


#define BIT_SHIFT_CF_L_BIT0_TO_1              	13
#define BIT_MASK_CF_L_BIT0_TO_1               	0x3
#define BIT_CF_L_BIT0_TO_1(x)                 	(((x) & BIT_MASK_CF_L_BIT0_TO_1) << BIT_SHIFT_CF_L_BIT0_TO_1)

#define BIT_SW18_FPWM                         	BIT(11)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL1                      (Offset 0x0010)


#define BIT_SHIFT_VOL_L1                      	10
#define BIT_MASK_VOL_L1                       	0xf
#define BIT_VOL_L1(x)                         	(((x) & BIT_MASK_VOL_L1) << BIT_SHIFT_VOL_L1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL1                      (Offset 0x0010)

#define BIT_SW18_SWEN                         	BIT(9)
#define BIT_SW18_LDEN                         	BIT(8)
#define BIT_MAC_ID_EN                         	BIT(7)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL1                      (Offset 0x0010)

#define BIT_LDO11_EN                          	BIT(6)
#define BIT_AFE_P3_PC                         	BIT(5)
#define BIT_AFE_P2_PC                         	BIT(4)
#define BIT_AFE_P1_PC                         	BIT(3)
#define BIT_AFE_P0_PC                         	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL1                      (Offset 0x0010)

#define BIT_AFE_BGEN                          	BIT(0)

//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)

#define BIT_POW_ZCD_L                         	BIT(31)
#define BIT_Enable_ZCDOut_L                   	BIT(30)

#define BIT_SHIFT_Reg_delay                   	28
#define BIT_MASK_Reg_delay                    	0x3
#define BIT_Reg_delay(x)                      	(((x) & BIT_MASK_Reg_delay) << BIT_SHIFT_Reg_delay)


#define BIT_SHIFT_SW18_v15ADJ                 	24
#define BIT_MASK_SW18_v15ADJ                  	0x7
#define BIT_SW18_v15ADJ(x)                    	(((x) & BIT_MASK_SW18_v15ADJ) << BIT_SHIFT_SW18_v15ADJ)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)

#define BIT_OCPSL                             	BIT(24)
#define BIT_REG_LDOF_L_V1                     	BIT(23)
#define BIT_PARSW_dummy                       	BIT(22)
#define BIT_CLAMP_MAX_DUTY                    	BIT(21)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)


#define BIT_SHIFT_SW18_VOL                    	20
#define BIT_MASK_SW18_VOL                     	0xf
#define BIT_SW18_VOL(x)                       	(((x) & BIT_MASK_SW18_VOL) << BIT_SHIFT_SW18_VOL)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)


#define BIT_SHIFT_TBOX_L1_V1                  	19
#define BIT_MASK_TBOX_L1_V1                   	0x3
#define BIT_TBOX_L1_V1(x)                     	(((x) & BIT_MASK_TBOX_L1_V1) << BIT_SHIFT_TBOX_L1_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)


#define BIT_SHIFT_SW18_IN                     	17
#define BIT_MASK_SW18_IN                      	0x7
#define BIT_SW18_IN(x)                        	(((x) & BIT_MASK_SW18_IN) << BIT_SHIFT_SW18_IN)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)


#define BIT_SHIFT_REG_DELAY_V3                	17
#define BIT_MASK_REG_DELAY_V3                 	0x3
#define BIT_REG_DELAY_V3(x)                   	(((x) & BIT_MASK_REG_DELAY_V3) << BIT_SHIFT_REG_DELAY_V3)

#define BIT_REG_CLAMP_D_L_V2                  	BIT(16)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)


#define BIT_SHIFT_SW18_TBOX                   	15
#define BIT_MASK_SW18_TBOX                    	0x3
#define BIT_SW18_TBOX(x)                      	(((x) & BIT_MASK_SW18_TBOX) << BIT_SHIFT_SW18_TBOX)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)

#define BIT_REG_BYPASS_L_V3                   	BIT(15)
#define BIT_Enable_ZCDOUT_L_V3                	BIT(14)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)

#define BIT_SW18_SEL                          	BIT(13)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)

#define BIT_POW_ZCD_L_V3                      	BIT(13)
#define BIT_AREN_L1_V1                        	BIT(12)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)


#define BIT_SHIFT_SW18_STD                    	11
#define BIT_MASK_SW18_STD                     	0x3
#define BIT_SW18_STD(x)                       	(((x) & BIT_MASK_SW18_STD) << BIT_SHIFT_SW18_STD)

#define BIT_SW18_SD                           	BIT(10)
#define BIT_SW18_AREN                         	BIT(9)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)


#define BIT_SHIFT_OCP_V3                      	9
#define BIT_MASK_OCP_V3                       	0x7
#define BIT_OCP_V3(x)                         	(((x) & BIT_MASK_OCP_V3) << BIT_SHIFT_OCP_V3)

#define BIT_POWOCP_V3                         	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)


#define BIT_SHIFT_SW18_R3                     	7
#define BIT_MASK_SW18_R3                      	0x3
#define BIT_SW18_R3(x)                        	(((x) & BIT_MASK_SW18_R3) << BIT_SHIFT_SW18_R3)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)


#define BIT_SHIFT_CF_L_V3                     	6
#define BIT_MASK_CF_L_V3                      	0x3
#define BIT_CF_L_V3(x)                        	(((x) & BIT_MASK_CF_L_V3) << BIT_SHIFT_CF_L_V3)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)


#define BIT_SHIFT_SW18_R2                     	5
#define BIT_MASK_SW18_R2                      	0x3
#define BIT_SW18_R2(x)                        	(((x) & BIT_MASK_SW18_R2) << BIT_SHIFT_SW18_R2)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)


#define BIT_SHIFT_CFC_L_BIT0_TO_1_V1          	4
#define BIT_MASK_CFC_L_BIT0_TO_1_V1           	0x3
#define BIT_CFC_L_BIT0_TO_1_V1(x)             	(((x) & BIT_MASK_CFC_L_BIT0_TO_1_V1) << BIT_SHIFT_CFC_L_BIT0_TO_1_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)


#define BIT_SHIFT_SW18_R1                     	3
#define BIT_MASK_SW18_R1                      	0x3
#define BIT_SW18_R1(x)                        	(((x) & BIT_MASK_SW18_R1) << BIT_SHIFT_SW18_R1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)


#define BIT_SHIFT_R3_L1_V1                    	2
#define BIT_MASK_R3_L1_V1                     	0x3
#define BIT_R3_L1_V1(x)                       	(((x) & BIT_MASK_R3_L1_V1) << BIT_SHIFT_R3_L1_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)


#define BIT_SHIFT_SW18_C3                     	1
#define BIT_MASK_SW18_C3                      	0x3
#define BIT_SW18_C3(x)                        	(((x) & BIT_MASK_SW18_C3) << BIT_SHIFT_SW18_C3)

#define BIT_SW18_C2_BIT1                      	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_SWR_CTRL2                      (Offset 0x0014)


#define BIT_SHIFT_R2_L1_V1                    	0
#define BIT_MASK_R2_L1_V1                     	0x3
#define BIT_R2_L1_V1(x)                       	(((x) & BIT_MASK_R2_L1_V1) << BIT_SHIFT_R2_L1_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_SWR_CTRL3                      (Offset 0x0018)

#define BIT_SPS18_OCP_DIS                     	BIT(31)

#define BIT_SHIFT_SPS18_OCP_TH                	16
#define BIT_MASK_SPS18_OCP_TH                 	0x7fff
#define BIT_SPS18_OCP_TH(x)                   	(((x) & BIT_MASK_SPS18_OCP_TH) << BIT_SHIFT_SPS18_OCP_TH)


#define BIT_SHIFT_OCP_WINDOW                  	0
#define BIT_MASK_OCP_WINDOW                   	0xffff
#define BIT_OCP_WINDOW(x)                     	(((x) & BIT_MASK_OCP_WINDOW) << BIT_SHIFT_OCP_WINDOW)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RSV_CTRL                           (Offset 0x001C)

#define BIT_HREG_DBG                          	BIT(23)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_RSV_CTRL                           (Offset 0x001C)

#define BIT_HREG_DBG_V1                       	BIT(23)
#define BIT_MCU_RST                           	BIT(11)
#define BIT_WLOCK_90                          	BIT(10)
#define BIT_WLOCK_70                          	BIT(9)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RSV_CTRL                           (Offset 0x001C)

#define BIT_WLMCUIOIF                         	BIT(8)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_RSV_CTRL                           (Offset 0x001C)

#define BIT_WLOCK_78                          	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RSV_CTRL                           (Offset 0x001C)

#define BIT_LOCK_ALL_EN                       	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RSV_CTRL                           (Offset 0x001C)

#define BIT_R_DIS_PRST                        	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_RSV_CTRL                           (Offset 0x001C)

#define BIT_R_DIS_PRST_1                      	BIT(6)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RSV_CTRL                           (Offset 0x001C)

#define BIT_WLOCK_1C_B6                       	BIT(5)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_RSV_CTRL                           (Offset 0x001C)

#define BIT_R_DIS_PRST_0                      	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RSV_CTRL                           (Offset 0x001C)

#define BIT_WLOCK_40                          	BIT(4)
#define BIT_WLOCK_08                          	BIT(3)
#define BIT_WLOCK_04                          	BIT(2)
#define BIT_WLOCK_00                          	BIT(1)
#define BIT_WLOCK_ALL                         	BIT(0)

//2 REG_RF_CTRL                            (Offset 0x001F)

#define BIT_RF_SDMRSTB                        	BIT(2)
#define BIT_RF_RSTB                           	BIT(1)
#define BIT_RF_EN                             	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_LDO_CTRL                       (Offset 0x0020)

#define BIT_LPLDH12_RSV1                      	BIT(31)
#define BIT_LPLDH12_RSV0                      	BIT(30)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_LDO_CTRL                       (Offset 0x0020)


#define BIT_SHIFT_LPLDH12_RSV                 	29
#define BIT_MASK_LPLDH12_RSV                  	0x7
#define BIT_LPLDH12_RSV(x)                    	(((x) & BIT_MASK_LPLDH12_RSV) << BIT_SHIFT_LPLDH12_RSV)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_LDO_CTRL                       (Offset 0x0020)

#define BIT_LPLDH12_SLP                       	BIT(28)

#define BIT_SHIFT_LPLDH12_VADJ                	24
#define BIT_MASK_LPLDH12_VADJ                 	0xf
#define BIT_LPLDH12_VADJ(x)                   	(((x) & BIT_MASK_LPLDH12_VADJ) << BIT_SHIFT_LPLDH12_VADJ)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_LDO_CTRL                       (Offset 0x0020)

#define BIT_LDH12_EN                          	BIT(16)
#define BIT_LDOV12W_EN                        	BIT(8)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_LDO_CTRL                       (Offset 0x0020)


#define BIT_SHIFT_ANAPAR_RFC2                 	8
#define BIT_MASK_ANAPAR_RFC2                  	0xff
#define BIT_ANAPAR_RFC2(x)                    	(((x) & BIT_MASK_ANAPAR_RFC2) << BIT_SHIFT_ANAPAR_RFC2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_LDO_CTRL                       (Offset 0x0020)


#define BIT_SHIFT_LDA12_VOADJ                 	4
#define BIT_MASK_LDA12_VOADJ                  	0xf
#define BIT_LDA12_VOADJ(x)                    	(((x) & BIT_MASK_LDA12_VOADJ) << BIT_SHIFT_LDA12_VOADJ)

#define BIT_Reg_vos                           	BIT(3)
#define BIT_LDA12_EN                          	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_LDO_CTRL                       (Offset 0x0020)


#define BIT_SHIFT_ANAPAR_RFC1                 	0
#define BIT_MASK_ANAPAR_RFC1                  	0xff
#define BIT_ANAPAR_RFC1(x)                    	(((x) & BIT_MASK_ANAPAR_RFC1) << BIT_SHIFT_ANAPAR_RFC1)


//2 REG_AFE_CTRL1                          (Offset 0x0024)

#define BIT_XQSEL_V3                          	BIT(31)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_Reg_cc                      	30
#define BIT_MASK_Reg_cc                       	0x3
#define BIT_Reg_cc(x)                         	(((x) & BIT_MASK_Reg_cc) << BIT_SHIFT_Reg_cc)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)

#define BIT_ckdelay_afe_V1                    	BIT(30)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)

#define BIT_CKDLY_DIG                         	BIT(28)
#define BIT_CKDLY_USB                         	BIT(27)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_XTAL_GPIO_V1                	27
#define BIT_MASK_XTAL_GPIO_V1                 	0x7
#define BIT_XTAL_GPIO_V1(x)                   	(((x) & BIT_MASK_XTAL_GPIO_V1) << BIT_SHIFT_XTAL_GPIO_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)

#define BIT_CKDLY_AFE                         	BIT(26)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_XTAL_DIG_DRV_1_TO_0         	25
#define BIT_MASK_XTAL_DIG_DRV_1_TO_0          	0x3
#define BIT_XTAL_DIG_DRV_1_TO_0(x)            	(((x) & BIT_MASK_XTAL_DIG_DRV_1_TO_0) << BIT_SHIFT_XTAL_DIG_DRV_1_TO_0)

#define BIT_XTAL_GDIG                         	BIT(24)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_XTAL_GPIO                   	23
#define BIT_MASK_XTAL_GPIO                    	0x7
#define BIT_XTAL_GPIO(x)                      	(((x) & BIT_MASK_XTAL_GPIO) << BIT_SHIFT_XTAL_GPIO)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_XTAL_RDRV_RF2_1_TO_0        	22
#define BIT_MASK_XTAL_RDRV_RF2_1_TO_0         	0x3
#define BIT_XTAL_RDRV_RF2_1_TO_0(x)           	(((x) & BIT_MASK_XTAL_RDRV_RF2_1_TO_0) << BIT_SHIFT_XTAL_RDRV_RF2_1_TO_0)

#define BIT_XTAL_GMN_4                        	BIT(21)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_MAC_CLK_SEL                 	20
#define BIT_MASK_MAC_CLK_SEL                  	0x3
#define BIT_MAC_CLK_SEL(x)                    	(((x) & BIT_MASK_MAC_CLK_SEL) << BIT_SHIFT_MAC_CLK_SEL)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_XTAL_RDRV_1_TO_0            	19
#define BIT_MASK_XTAL_RDRV_1_TO_0             	0x3
#define BIT_XTAL_RDRV_1_TO_0(x)               	(((x) & BIT_MASK_XTAL_RDRV_1_TO_0) << BIT_SHIFT_XTAL_RDRV_1_TO_0)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_XTAL_DIG_DRV                	18
#define BIT_MASK_XTAL_DIG_DRV                 	0x3
#define BIT_XTAL_DIG_DRV(x)                   	(((x) & BIT_MASK_XTAL_DIG_DRV) << BIT_SHIFT_XTAL_DIG_DRV)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)

#define BIT_XTAL_GMP_4                        	BIT(18)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)

#define BIT_XTAL_GATE_DIG                     	BIT(17)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_XTAL_ADRV_1_TO_0            	16
#define BIT_MASK_XTAL_ADRV_1_TO_0             	0x3
#define BIT_XTAL_ADRV_1_TO_0(x)               	(((x) & BIT_MASK_XTAL_ADRV_1_TO_0) << BIT_SHIFT_XTAL_ADRV_1_TO_0)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_XTAL_RF_DRV                 	15
#define BIT_MASK_XTAL_RF_DRV                  	0x3
#define BIT_XTAL_RF_DRV(x)                    	(((x) & BIT_MASK_XTAL_RF_DRV) << BIT_SHIFT_XTAL_RF_DRV)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)

#define BIT_XTAL_GAFE                         	BIT(15)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)

#define BIT_XTAL_RF_GATE                      	BIT(14)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_XTAL_DDRV_1_TO_0            	13
#define BIT_MASK_XTAL_DDRV_1_TO_0             	0x3
#define BIT_XTAL_DDRV_1_TO_0(x)               	(((x) & BIT_MASK_XTAL_DDRV_1_TO_0) << BIT_SHIFT_XTAL_DDRV_1_TO_0)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_XTAL_AFE_DRV                	12
#define BIT_MASK_XTAL_AFE_DRV                 	0x3
#define BIT_XTAL_AFE_DRV(x)                   	(((x) & BIT_MASK_XTAL_AFE_DRV) << BIT_SHIFT_XTAL_AFE_DRV)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)

#define BIT_XTAL_GUSB                         	BIT(12)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)

#define BIT_XTAL_GATE_AFE                     	BIT(11)

#define BIT_SHIFT_XTAL_USB_DRV                	9
#define BIT_MASK_XTAL_USB_DRV                 	0x3
#define BIT_XTAL_USB_DRV(x)                   	(((x) & BIT_MASK_XTAL_USB_DRV) << BIT_SHIFT_XTAL_USB_DRV)

#define BIT_XTAL_GATE_USB                     	BIT(8)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_XTAL_GMN_3_TO_0             	8
#define BIT_MASK_XTAL_GMN_3_TO_0              	0xf
#define BIT_XTAL_GMN_3_TO_0(x)                	(((x) & BIT_MASK_XTAL_GMN_3_TO_0) << BIT_SHIFT_XTAL_GMN_3_TO_0)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_XTAL_GMP                    	4
#define BIT_MASK_XTAL_GMP                     	0xf
#define BIT_XTAL_GMP(x)                       	(((x) & BIT_MASK_XTAL_GMP) << BIT_SHIFT_XTAL_GMP)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_XTAL_GMP_3_TO_0             	4
#define BIT_MASK_XTAL_GMP_3_TO_0              	0xf
#define BIT_XTAL_GMP_3_TO_0(x)                	(((x) & BIT_MASK_XTAL_GMP_3_TO_0) << BIT_SHIFT_XTAL_GMP_3_TO_0)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_XTAL_LDO_VCM                	2
#define BIT_MASK_XTAL_LDO_VCM                 	0x3
#define BIT_XTAL_LDO_VCM(x)                   	(((x) & BIT_MASK_XTAL_LDO_VCM) << BIT_SHIFT_XTAL_LDO_VCM)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)


#define BIT_SHIFT_DRV_LDO_VCM_1_TO_0          	2
#define BIT_MASK_DRV_LDO_VCM_1_TO_0           	0x3
#define BIT_DRV_LDO_VCM_1_TO_0(x)             	(((x) & BIT_MASK_DRV_LDO_VCM_1_TO_0) << BIT_SHIFT_DRV_LDO_VCM_1_TO_0)


#endif


#if (RTL8192E_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)

#define BIT_XTAL_Dummy                        	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)

#define BIT_XQSEL_RF_INITIAL_V1               	BIT(1)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)

#define BIT_XQSEL_BIT0                        	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL1                          (Offset 0x0024)

#define BIT_XTAL_EN                           	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)


#define BIT_SHIFT_XTAL_GMN                    	28
#define BIT_MASK_XTAL_GMN                     	0xf
#define BIT_XTAL_GMN(x)                       	(((x) & BIT_MASK_XTAL_GMN) << BIT_SHIFT_XTAL_GMN)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)


#define BIT_SHIFT_IOOFFSET_3_TO_0             	28
#define BIT_MASK_IOOFFSET_3_TO_0              	0xf
#define BIT_IOOFFSET_3_TO_0(x)                	(((x) & BIT_MASK_IOOFFSET_3_TO_0) << BIT_SHIFT_IOOFFSET_3_TO_0)

#define BIT_REG_FREF_SEL_BIT3_V1              	BIT(27)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)


#define BIT_SHIFT_Reg_vo_ad                   	26
#define BIT_MASK_Reg_vo_ad                    	0x3
#define BIT_Reg_vo_ad(x)                      	(((x) & BIT_MASK_Reg_vo_ad) << BIT_SHIFT_Reg_vo_ad)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)


#define BIT_SHIFT_V12ADJ_V1                   	25
#define BIT_MASK_V12ADJ_V1                    	0x3
#define BIT_V12ADJ_V1(x)                      	(((x) & BIT_MASK_V12ADJ_V1) << BIT_SHIFT_V12ADJ_V1)

#define BIT_EN_CK160M_V1                      	BIT(24)
#define BIT_EN_CK320M_V1                      	BIT(23)
#define BIT_AGPIO                             	BIT(22)
#define BIT_REG_EDGE_SEL_V1                   	BIT(21)
#define BIT_REG_VCO_BIAS_0                    	BIT(20)

#define BIT_SHIFT_REG_PLLBIAS_2_TO_0_V1       	17
#define BIT_MASK_REG_PLLBIAS_2_TO_0_V1        	0x7
#define BIT_REG_PLLBIAS_2_TO_0_V1(x)          	(((x) & BIT_MASK_REG_PLLBIAS_2_TO_0_V1) << BIT_SHIFT_REG_PLLBIAS_2_TO_0_V1)

#define BIT_REG_IDOUBLE_V1                    	BIT(16)
#define BIT_REG_KVCO_V1                       	BIT(15)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)

#define BIT_APLL_320_GATEB                    	BIT(14)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)

#define BIT_REG_VCO_BIAS_1_V1                 	BIT(14)
#define BIT_REG_DOGB_V1                       	BIT(13)

#define BIT_SHIFT_DUMMY0                      	11
#define BIT_MASK_DUMMY0                       	0x3
#define BIT_DUMMY0(x)                         	(((x) & BIT_MASK_DUMMY0) << BIT_SHIFT_DUMMY0)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)


#define BIT_SHIFT_APLL_BIAS                   	8
#define BIT_MASK_APLL_BIAS                    	0x7
#define BIT_APLL_BIAS(x)                      	(((x) & BIT_MASK_APLL_BIAS) << BIT_SHIFT_APLL_BIAS)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)


#define BIT_SHIFT_REG_V15_3_TO_0_V1           	7
#define BIT_MASK_REG_V15_3_TO_0_V1            	0xf
#define BIT_REG_V15_3_TO_0_V1(x)              	(((x) & BIT_MASK_REG_V15_3_TO_0_V1) << BIT_SHIFT_REG_V15_3_TO_0_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)

#define BIT_APLL_KVCO                         	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)

#define BIT_REG_SEL_LDO_PC                    	BIT(6)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)

#define BIT_APLL_WDOGB                        	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)


#define BIT_SHIFT_REG_CC_1_TO_0_V1            	4
#define BIT_MASK_REG_CC_1_TO_0_V1             	0x3
#define BIT_REG_CC_1_TO_0_V1(x)               	(((x) & BIT_MASK_REG_CC_1_TO_0_V1) << BIT_SHIFT_REG_CC_1_TO_0_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)

#define BIT_APLL_EDGE_SEL                     	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)

#define BIT_CKDELAY_USB_V1                    	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)

#define BIT_APLL_FREF_SEL_BIT0                	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)

#define BIT_CKDELAY_DIG_V1                    	BIT(2)
#define BIT_MPLL_EN                           	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL2                          (Offset 0x0028)

#define BIT_APLL_EN                           	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)


#define BIT_SHIFT_XTAL_RF2_DRV                	30
#define BIT_MASK_XTAL_RF2_DRV                 	0x3
#define BIT_XTAL_RF2_DRV(x)                   	(((x) & BIT_MASK_XTAL_RF2_DRV) << BIT_SHIFT_XTAL_RF2_DRV)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)

#define BIT_REG_REF_SEL_V3                    	BIT(30)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)

#define BIT_XTAL_GMN_BIT4                     	BIT(29)
#define BIT_XTAL_GMP_BIT4                     	BIT(28)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)

#define BIT_XQSEL                             	BIT(27)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)


#define BIT_SHIFT_REG_FREF_SEL_2_TO_0         	27
#define BIT_MASK_REG_FREF_SEL_2_TO_0          	0x7
#define BIT_REG_FREF_SEL_2_TO_0(x)            	(((x) & BIT_MASK_REG_FREF_SEL_2_TO_0) << BIT_SHIFT_REG_FREF_SEL_2_TO_0)


#endif


#if (RTL8881A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)

#define BIT_XQSEL_RF_INITIAL                  	BIT(27)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)

#define BIT_APLL_DUMMY                        	BIT(26)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)


#define BIT_SHIFT_XTAL_CADJ_XOUT_5_TO_0_V1    	21
#define BIT_MASK_XTAL_CADJ_XOUT_5_TO_0_V1     	0x3f
#define BIT_XTAL_CADJ_XOUT_5_TO_0_V1(x)       	(((x) & BIT_MASK_XTAL_CADJ_XOUT_5_TO_0_V1) << BIT_SHIFT_XTAL_CADJ_XOUT_5_TO_0_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)


#define BIT_SHIFT_XTAL_CADJ_XOUT              	18
#define BIT_MASK_XTAL_CADJ_XOUT               	0x3f
#define BIT_XTAL_CADJ_XOUT(x)                 	(((x) & BIT_MASK_XTAL_CADJ_XOUT) << BIT_SHIFT_XTAL_CADJ_XOUT)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)


#define BIT_SHIFT_XTAL_CADJ_XIN_V2            	15
#define BIT_MASK_XTAL_CADJ_XIN_V2             	0x3f
#define BIT_XTAL_CADJ_XIN_V2(x)               	(((x) & BIT_MASK_XTAL_CADJ_XIN_V2) << BIT_SHIFT_XTAL_CADJ_XIN_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)


#define BIT_SHIFT_XTAL_CADJ_XIN               	12
#define BIT_MASK_XTAL_CADJ_XIN                	0x3f
#define BIT_XTAL_CADJ_XIN(x)                  	(((x) & BIT_MASK_XTAL_CADJ_XIN) << BIT_SHIFT_XTAL_CADJ_XIN)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)


#define BIT_SHIFT_REG_RS_V3                   	12
#define BIT_MASK_REG_RS_V3                    	0x7
#define BIT_REG_RS_V3(x)                      	(((x) & BIT_MASK_REG_RS_V3) << BIT_SHIFT_REG_RS_V3)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)


#define BIT_SHIFT_Reg_rs                      	9
#define BIT_MASK_Reg_rs                       	0x7
#define BIT_Reg_rs(x)                         	(((x) & BIT_MASK_Reg_rs) << BIT_SHIFT_Reg_rs)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)


#define BIT_SHIFT_REG_R3_V3                   	9
#define BIT_MASK_REG_R3_V3                    	0x7
#define BIT_REG_R3_V3(x)                      	(((x) & BIT_MASK_REG_R3_V3) << BIT_SHIFT_REG_R3_V3)


#define BIT_SHIFT_REG_CS_V3                   	7
#define BIT_MASK_REG_CS_V3                    	0x3
#define BIT_REG_CS_V3(x)                      	(((x) & BIT_MASK_REG_CS_V3) << BIT_SHIFT_REG_CS_V3)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)


#define BIT_SHIFT_Reg_r3                      	6
#define BIT_MASK_Reg_r3                       	0x7
#define BIT_Reg_r3(x)                         	(((x) & BIT_MASK_Reg_r3) << BIT_SHIFT_Reg_r3)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)


#define BIT_SHIFT_REG_CP_V3                   	5
#define BIT_MASK_REG_CP_V3                    	0x3
#define BIT_REG_CP_V3(x)                      	(((x) & BIT_MASK_REG_CP_V3) << BIT_SHIFT_REG_CP_V3)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)


#define BIT_SHIFT_Reg_cs                      	4
#define BIT_MASK_Reg_cs                       	0x3
#define BIT_Reg_cs(x)                         	(((x) & BIT_MASK_Reg_cs) << BIT_SHIFT_Reg_cs)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)


#define BIT_SHIFT_REG_C3_V3                   	3
#define BIT_MASK_REG_C3_V3                    	0x3
#define BIT_REG_C3_V3(x)                      	(((x) & BIT_MASK_REG_C3_V3) << BIT_SHIFT_REG_C3_V3)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)


#define BIT_SHIFT_Reg_cp                      	2
#define BIT_MASK_Reg_cp                       	0x3
#define BIT_Reg_cp(x)                         	(((x) & BIT_MASK_Reg_cp) << BIT_SHIFT_Reg_cp)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)

#define BIT_REG_320_SEL_V3                    	BIT(2)
#define BIT_EN_SYN_V1                         	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)


#define BIT_SHIFT_Reg_c3                      	0
#define BIT_MASK_Reg_c3                       	0x3
#define BIT_Reg_c3(x)                         	(((x) & BIT_MASK_Reg_c3) << BIT_SHIFT_Reg_c3)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_CTRL3                          (Offset 0x002C)

#define BIT_IOOFFSET_BIT4                     	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_EFUSE_CTRL                         (Offset 0x0030)

#define BIT_EF_FLAG                           	BIT(31)

#define BIT_SHIFT_EF_PGPD                     	28
#define BIT_MASK_EF_PGPD                      	0x7
#define BIT_EF_PGPD(x)                        	(((x) & BIT_MASK_EF_PGPD) << BIT_SHIFT_EF_PGPD)


#define BIT_SHIFT_EF_RDT                      	24
#define BIT_MASK_EF_RDT                       	0xf
#define BIT_EF_RDT(x)                         	(((x) & BIT_MASK_EF_RDT) << BIT_SHIFT_EF_RDT)


#define BIT_SHIFT_EF_PGTS                     	20
#define BIT_MASK_EF_PGTS                      	0xf
#define BIT_EF_PGTS(x)                        	(((x) & BIT_MASK_EF_PGTS) << BIT_SHIFT_EF_PGTS)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_EFUSE_CTRL                         (Offset 0x0030)

#define BIT_EF_PDWN                           	BIT(19)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_EFUSE_CTRL                         (Offset 0x0030)

#define BIT_EF_ALDEN                          	BIT(18)

#define BIT_SHIFT_EF_ADDR                     	8
#define BIT_MASK_EF_ADDR                      	0x3ff
#define BIT_EF_ADDR(x)                        	(((x) & BIT_MASK_EF_ADDR) << BIT_SHIFT_EF_ADDR)


#define BIT_SHIFT_EF_DATA                     	0
#define BIT_MASK_EF_DATA                      	0xff
#define BIT_EF_DATA(x)                        	(((x) & BIT_MASK_EF_DATA) << BIT_SHIFT_EF_DATA)


//2 REG_LDO_EFUSE_CTRL                     (Offset 0x0034)

#define BIT_LDOE25_EN                         	BIT(31)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LDO_EFUSE_CTRL                     (Offset 0x0034)


#define BIT_SHIFT_LDOE25_VADJ_BIT0_TO_2       	28
#define BIT_MASK_LDOE25_VADJ_BIT0_TO_2        	0x7
#define BIT_LDOE25_VADJ_BIT0_TO_2(x)          	(((x) & BIT_MASK_LDOE25_VADJ_BIT0_TO_2) << BIT_SHIFT_LDOE25_VADJ_BIT0_TO_2)

#define BIT_LDOE25_VADJ_BIT3                  	BIT(27)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_LDO_EFUSE_CTRL                     (Offset 0x0034)


#define BIT_SHIFT_LDOE25_VADJ_3_TO_0          	27
#define BIT_MASK_LDOE25_VADJ_3_TO_0           	0xf
#define BIT_LDOE25_VADJ_3_TO_0(x)             	(((x) & BIT_MASK_LDOE25_VADJ_3_TO_0) << BIT_SHIFT_LDOE25_VADJ_3_TO_0)


#endif


#if (RTL8192E_SUPPORT)


//2 REG_LDO_EFUSE_CTRL                     (Offset 0x0034)

#define BIT_EFCRES_SEL                        	BIT(26)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_LDO_EFUSE_CTRL                     (Offset 0x0034)

#define BIT_EF_CSER                           	BIT(26)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LDO_EFUSE_CTRL                     (Offset 0x0034)


#define BIT_SHIFT_EF_SCAN_START               	16
#define BIT_MASK_EF_SCAN_START                	0x1ff
#define BIT_EF_SCAN_START(x)                  	(((x) & BIT_MASK_EF_SCAN_START) << BIT_SHIFT_EF_SCAN_START)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_LDO_EFUSE_CTRL                     (Offset 0x0034)


#define BIT_SHIFT_EF_SCAN_START_V2            	16
#define BIT_MASK_EF_SCAN_START_V2             	0x1ff
#define BIT_EF_SCAN_START_V2(x)               	(((x) & BIT_MASK_EF_SCAN_START_V2) << BIT_SHIFT_EF_SCAN_START_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LDO_EFUSE_CTRL                     (Offset 0x0034)


#define BIT_SHIFT_EF_SCAN_END                 	12
#define BIT_MASK_EF_SCAN_END                  	0xf
#define BIT_EF_SCAN_END(x)                    	(((x) & BIT_MASK_EF_SCAN_END) << BIT_SHIFT_EF_SCAN_END)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_LDO_EFUSE_CTRL                     (Offset 0x0034)

#define BIT_SCAN_EN                           	BIT(11)
#define BIT_SW_PG_EN                          	BIT(10)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LDO_EFUSE_CTRL                     (Offset 0x0034)


#define BIT_SHIFT_EF_CELL_SEL                 	8
#define BIT_MASK_EF_CELL_SEL                  	0x3
#define BIT_EF_CELL_SEL(x)                    	(((x) & BIT_MASK_EF_CELL_SEL) << BIT_SHIFT_EF_CELL_SEL)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LDO_EFUSE_CTRL                     (Offset 0x0034)

#define BIT_EF_TRPT                           	BIT(7)

#define BIT_SHIFT_EF_TTHD                     	0
#define BIT_MASK_EF_TTHD                      	0x7f
#define BIT_EF_TTHD(x)                        	(((x) & BIT_MASK_EF_TTHD) << BIT_SHIFT_EF_TTHD)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)


#define BIT_SHIFT_AFE_USB_CURRENT_SEL         	26
#define BIT_MASK_AFE_USB_CURRENT_SEL          	0x7
#define BIT_AFE_USB_CURRENT_SEL(x)            	(((x) & BIT_MASK_AFE_USB_CURRENT_SEL) << BIT_SHIFT_AFE_USB_CURRENT_SEL)


#define BIT_SHIFT_AFE_USB_PATH_SEL            	24
#define BIT_MASK_AFE_USB_PATH_SEL             	0x3
#define BIT_AFE_USB_PATH_SEL(x)               	(((x) & BIT_MASK_AFE_USB_PATH_SEL) << BIT_SHIFT_AFE_USB_PATH_SEL)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)


#define BIT_SHIFT_DBG_SEL_V1                  	16
#define BIT_MASK_DBG_SEL_V1                   	0xff
#define BIT_DBG_SEL_V1(x)                     	(((x) & BIT_MASK_DBG_SEL_V1) << BIT_SHIFT_DBG_SEL_V1)


#endif


#if (RTL8192E_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)

#define BIT_CLK_REQ_INPUT                     	BIT(15)
#define BIT_USB_XTAL_CLK_SEL                  	BIT(14)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)

#define BIT_USB_REG_XTAL_SEL                  	BIT(14)
#define BIT_SYSON_BTIO1POW_PAD_E2             	BIT(13)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)


#define BIT_SHIFT_SYSON_SPS0_STD_L1           	12
#define BIT_MASK_SYSON_SPS0_STD_L1            	0x3
#define BIT_SYSON_SPS0_STD_L1(x)              	(((x) & BIT_MASK_SYSON_SPS0_STD_L1) << BIT_SHIFT_SYSON_SPS0_STD_L1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)

#define BIT_SYSON_BTIOPOW_PAD_E2              	BIT(12)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)


#define BIT_SHIFT_SYSON_LDOA12V_WT            	12
#define BIT_MASK_SYSON_LDOA12V_WT             	0x3
#define BIT_SYSON_LDOA12V_WT(x)               	(((x) & BIT_MASK_SYSON_LDOA12V_WT) << BIT_SHIFT_SYSON_LDOA12V_WT)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)

#define BIT_SYSON_DBG_PAD_E2                  	BIT(11)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)

#define BIT_SYSON_SDIOPOW_PAD_E2              	BIT(11)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)

#define BIT_SYSON_LED_PAD_E2                  	BIT(10)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)

#define BIT_SYSON_GPEE_PAD_E2                 	BIT(9)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)

#define BIT_SYSON_GPEE_PAD_E2_V33             	BIT(9)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)

#define BIT_SYSON_PCI_PAD_E2                  	BIT(8)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)

#define BIT_AUTO_SW_LDO_VOL_EN_V1             	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)

#define BIT_ADJ_LDO_VOLT                      	BIT(6)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)


#define BIT_SHIFT_SYSON_LDOHCI12_WT           	6
#define BIT_MASK_SYSON_LDOHCI12_WT            	0x3
#define BIT_SYSON_LDOHCI12_WT(x)              	(((x) & BIT_MASK_SYSON_LDOHCI12_WT) << BIT_SHIFT_SYSON_LDOHCI12_WT)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)


#define BIT_SHIFT_SYSON_SPS0WWV_WT            	4
#define BIT_MASK_SYSON_SPS0WWV_WT             	0x3
#define BIT_SYSON_SPS0WWV_WT(x)               	(((x) & BIT_MASK_SYSON_SPS0WWV_WT) << BIT_SHIFT_SYSON_SPS0WWV_WT)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)


#define BIT_SHIFT_SYSON_SPS0SPS_WT            	4
#define BIT_MASK_SYSON_SPS0SPS_WT             	0x3
#define BIT_SYSON_SPS0SPS_WT(x)               	(((x) & BIT_MASK_SYSON_SPS0SPS_WT) << BIT_SHIFT_SYSON_SPS0SPS_WT)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)


#define BIT_SHIFT_SYSON_SPS0LDO_WT            	2
#define BIT_MASK_SYSON_SPS0LDO_WT             	0x3
#define BIT_SYSON_SPS0LDO_WT(x)               	(((x) & BIT_MASK_SYSON_SPS0LDO_WT) << BIT_SHIFT_SYSON_SPS0LDO_WT)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)


#define BIT_SHIFT_SYSON_SPS11VLDO_WT          	2
#define BIT_MASK_SYSON_SPS11VLDO_WT           	0x3
#define BIT_SYSON_SPS11VLDO_WT(x)             	(((x) & BIT_MASK_SYSON_SPS11VLDO_WT) << BIT_SHIFT_SYSON_SPS11VLDO_WT)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PWR_OPTION_CTRL                    (Offset 0x0038)


#define BIT_SHIFT_SYSON_RCLK_SCALE            	0
#define BIT_MASK_SYSON_RCLK_SCALE             	0x3
#define BIT_SYSON_RCLK_SCALE(x)               	(((x) & BIT_MASK_SYSON_RCLK_SCALE) << BIT_SHIFT_SYSON_RCLK_SCALE)


//2 REG_CAL_TIMER                          (Offset 0x003C)


#define BIT_SHIFT_MATCH_CNT                   	8
#define BIT_MASK_MATCH_CNT                    	0xff
#define BIT_MATCH_CNT(x)                      	(((x) & BIT_MASK_MATCH_CNT) << BIT_SHIFT_MATCH_CNT)


#define BIT_SHIFT_CAL_SCAL                    	0
#define BIT_MASK_CAL_SCAL                     	0xff
#define BIT_CAL_SCAL(x)                       	(((x) & BIT_MASK_CAL_SCAL) << BIT_SHIFT_CAL_SCAL)


//2 REG_ACLK_MON                           (Offset 0x003E)


#define BIT_SHIFT_RCLK_MON                    	5
#define BIT_MASK_RCLK_MON                     	0x7ff
#define BIT_RCLK_MON(x)                       	(((x) & BIT_MASK_RCLK_MON) << BIT_SHIFT_RCLK_MON)

#define BIT_CAL_EN                            	BIT(4)

#define BIT_SHIFT_DPSTU                       	2
#define BIT_MASK_DPSTU                        	0x3
#define BIT_DPSTU(x)                          	(((x) & BIT_MASK_DPSTU) << BIT_SHIFT_DPSTU)

#define BIT_SUS_16X                           	BIT(1)
#define BIT_RSM_EN                            	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)

#define BIT_PAD_D_PAPE_2G_E                   	BIT(31)
#define BIT_PAD_D_PAPE_5G_E                   	BIT(30)
#define BIT_PAD_D_TRSW_E                      	BIT(29)
#define BIT_PAD_D_TRSWB_E                     	BIT(28)
#define BIT_PAD_D_PAPE_2G_O                   	BIT(27)
#define BIT_PAD_D_PAPE_5G_O                   	BIT(26)
#define BIT_PAD_D_TRSW_O                      	BIT(25)
#define BIT_PAD_D_TRSWB_O                     	BIT(24)
#define BIT_EN_A_ANTSEL                       	BIT(23)
#define BIT_EN_A_ANTSELB                      	BIT(22)
#define BIT_EN_D_PAPE_2G                      	BIT(21)
#define BIT_EN_D_PAPE_5G                      	BIT(20)
#define BIT_FSPI_EN                           	BIT(19)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)

#define BIT_CKOUT33_EN                        	BIT(17)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)

#define BIT_XTAL_OUT_EN                       	BIT(17)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)

#define BIT_WLGP_SPI_EN                       	BIT(16)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)

#define BIT_SIC_LBK                           	BIT(15)
#define BIT_EnHTP                             	BIT(14)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)

#define BIT_SIC_23                            	BIT(13)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)

#define BIT_EnSIC                             	BIT(12)
#define BIT_SIC_SWRST                         	BIT(11)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)

#define BIT_EnPMAC                            	BIT(10)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)

#define BIT_ENBTCMD                           	BIT(9)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)

#define BIT_BTCMD_OUT_EN                      	BIT(9)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)

#define BIT_EnUart                            	BIT(8)

#define BIT_SHIFT_BTMode                      	6
#define BIT_MASK_BTMode                       	0x3
#define BIT_BTMode(x)                         	(((x) & BIT_MASK_BTMode) << BIT_SHIFT_BTMode)

#define BIT_EnBT                              	BIT(5)
#define BIT_EROM_EN                           	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)

#define BIT_WLRFE_6_7_EN                      	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)

#define BIT_EN_D_TRSW                         	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)

#define BIT_WLRFE_4_5_EN                      	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)

#define BIT_EN_D_TRSWB                        	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_GPIO_MUXCFG                        (Offset 0x0040)


#define BIT_SHIFT_GPIOSEL                     	0
#define BIT_MASK_GPIOSEL                      	0x3
#define BIT_GPIOSEL(x)                        	(((x) & BIT_MASK_GPIOSEL) << BIT_SHIFT_GPIOSEL)


//2 REG_GPIO_PIN_CTRL                      (Offset 0x0044)


#define BIT_SHIFT_GPIO_MOD_7_TO_0             	24
#define BIT_MASK_GPIO_MOD_7_TO_0              	0xff
#define BIT_GPIO_MOD_7_TO_0(x)                	(((x) & BIT_MASK_GPIO_MOD_7_TO_0) << BIT_SHIFT_GPIO_MOD_7_TO_0)


#define BIT_SHIFT_GPIO_IO_SEL_7_TO_0          	16
#define BIT_MASK_GPIO_IO_SEL_7_TO_0           	0xff
#define BIT_GPIO_IO_SEL_7_TO_0(x)             	(((x) & BIT_MASK_GPIO_IO_SEL_7_TO_0) << BIT_SHIFT_GPIO_IO_SEL_7_TO_0)


#define BIT_SHIFT_GPIO_OUT_7_TO_0             	8
#define BIT_MASK_GPIO_OUT_7_TO_0              	0xff
#define BIT_GPIO_OUT_7_TO_0(x)                	(((x) & BIT_MASK_GPIO_OUT_7_TO_0) << BIT_SHIFT_GPIO_OUT_7_TO_0)


#define BIT_SHIFT_GPIO_IN_7_TO_0              	0
#define BIT_MASK_GPIO_IN_7_TO_0               	0xff
#define BIT_GPIO_IN_7_TO_0(x)                 	(((x) & BIT_MASK_GPIO_IN_7_TO_0) << BIT_SHIFT_GPIO_IN_7_TO_0)


//2 REG_GPIO_INTM                          (Offset 0x0048)


#define BIT_SHIFT_MUXDBG_SEL                  	30
#define BIT_MASK_MUXDBG_SEL                   	0x3
#define BIT_MUXDBG_SEL(x)                     	(((x) & BIT_MASK_MUXDBG_SEL) << BIT_SHIFT_MUXDBG_SEL)


#endif


#if (RTL8192E_SUPPORT)


//2 REG_GPIO_INTM                          (Offset 0x0048)


#define BIT_SHIFT_MUXDBG_SEL2                 	28
#define BIT_MASK_MUXDBG_SEL2                  	0x3
#define BIT_MUXDBG_SEL2(x)                    	(((x) & BIT_MASK_MUXDBG_SEL2) << BIT_SHIFT_MUXDBG_SEL2)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_GPIO_INTM                          (Offset 0x0048)

#define BIT_GPIO_Ext_EN                       	BIT(20)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_GPIO_INTM                          (Offset 0x0048)

#define BIT_EXTWOL1_SEL                       	BIT(19)
#define BIT_EXTWOL1_EN                        	BIT(18)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_GPIO_INTM                          (Offset 0x0048)

#define BIT_EXTWOL0_SEL                       	BIT(17)
#define BIT_EXTWOL0_EN                        	BIT(16)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_GPIO_INTM                          (Offset 0x0048)


#define BIT_SHIFT_GPIO_Ext_WOL_V1             	16
#define BIT_MASK_GPIO_Ext_WOL_V1              	0xf
#define BIT_GPIO_Ext_WOL_V1(x)                	(((x) & BIT_MASK_GPIO_Ext_WOL_V1) << BIT_SHIFT_GPIO_Ext_WOL_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_GPIO_INTM                          (Offset 0x0048)

#define BIT_GPIOF_INT_MD                      	BIT(15)
#define BIT_GPIOE_INT_MD                      	BIT(14)
#define BIT_GPIOD_INT_MD                      	BIT(13)
#define BIT_GPIOC_INT_MD                      	BIT(12)
#define BIT_GPIOB_INT_MD                      	BIT(11)
#define BIT_GPIOA_INT_MD                      	BIT(10)
#define BIT_GPIO9_INT_MD                      	BIT(9)
#define BIT_GPIO8_INT_MD                      	BIT(8)
#define BIT_GPIO7_INT_MD                      	BIT(7)
#define BIT_GPIO6_INT_MD                      	BIT(6)
#define BIT_GPIO5_INT_MD                      	BIT(5)
#define BIT_GPIO4_INT_MD                      	BIT(4)
#define BIT_GPIO3_INT_MD                      	BIT(3)
#define BIT_GPIO2_INT_MD                      	BIT(2)
#define BIT_GPIO1_INT_MD                      	BIT(1)
#define BIT_GPIO0_INT_MD                      	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_PAD_ANTSEL_I                      	BIT(31)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_ANT_SEL7_EN                       	BIT(30)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_PAD_ANTSELB_I                     	BIT(30)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_ANT_SEL46_EN                      	BIT(29)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_PAD_D_PAPE_2G_I                   	BIT(29)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_ANT_SEL3_EN                       	BIT(28)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_PAD_D_PAPE_5G_I                   	BIT(28)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_TRSW_SEL_EN                       	BIT(27)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_PAD_D_TRSW_I                      	BIT(27)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_PAPE1_SEL_EN                      	BIT(26)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_PAD_D_TRSWB_I                     	BIT(26)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_LNAON_SEL_EN                      	BIT(26)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_PAPE0_SEL_EN                      	BIT(25)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_DWH_EN                            	BIT(25)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_PAPE_SEL_EN                       	BIT(25)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_ANTSEL2_EN                        	BIT(24)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_DHW_EN                            	BIT(24)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_RFE_ANT_EXT_SEL                   	BIT(24)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_ANTSEL_EN                         	BIT(23)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_DPDT_SEL_EN                       	BIT(23)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_GPIO13_14_WL_CTRL_EN              	BIT(22)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_TRXIQ_DBG_EN                      	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_LED2DIS                           	BIT(21)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_LED2EN                            	BIT(21)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_LED2PL                            	BIT(20)
#define BIT_LED2SV                            	BIT(19)

#define BIT_SHIFT_LED2CM                      	16
#define BIT_MASK_LED2CM                       	0x7
#define BIT_LED2CM(x)                         	(((x) & BIT_MASK_LED2CM) << BIT_SHIFT_LED2CM)

#define BIT_LED1DIS                           	BIT(15)
#define BIT_LED1PL                            	BIT(12)
#define BIT_LED1SV                            	BIT(11)

#define BIT_SHIFT_LED1CM                      	8
#define BIT_MASK_LED1CM                       	0x7
#define BIT_LED1CM(x)                         	(((x) & BIT_MASK_LED1CM) << BIT_SHIFT_LED1CM)

#define BIT_LED0DIS                           	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)


#define BIT_SHIFT_AFE_LDO_SWR_CHECK           	5
#define BIT_MASK_AFE_LDO_SWR_CHECK            	0x3
#define BIT_AFE_LDO_SWR_CHECK(x)              	(((x) & BIT_MASK_AFE_LDO_SWR_CHECK) << BIT_SHIFT_AFE_LDO_SWR_CHECK)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LED_CFG                            (Offset 0x004C)

#define BIT_LED0PL                            	BIT(4)
#define BIT_LED0SV                            	BIT(3)

#define BIT_SHIFT_LED0CM                      	0
#define BIT_MASK_LED0CM                       	0x7
#define BIT_LED0CM(x)                         	(((x) & BIT_MASK_LED0CM) << BIT_SHIFT_LED0CM)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_PDNINT_EN                      	BIT(31)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_SPS_OCP_INT_EN                 	BIT(29)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_SW_SPS_OCP_INT_EN                 	BIT(29)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_PWMERR_INT_EN                  	BIT(28)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_PWM_HW_ERR_EN                  	BIT(28)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIOF_INT_EN                   	BIT(27)
#define BIT_FS_GPIOE_INT_EN                   	BIT(26)
#define BIT_FS_GPIOD_INT_EN                   	BIT(25)
#define BIT_FS_GPIOC_INT_EN                   	BIT(24)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_ACT2RECOVERY_EN                   	BIT(24)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIOB_INT_EN                   	BIT(23)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_PCIE_GEN12_SWITH_EN               	BIT(23)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIOA_INT_EN                   	BIT(22)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_HCI_SUS_EN_V1                  	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO9_INT_EN                   	BIT(21)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_HCI_RES_EN_V1                  	BIT(21)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO8_INT_EN                   	BIT(20)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_HCI_RESET_EN_V1                	BIT(20)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO7_INT_EN                   	BIT(19)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_32K_LEAVE_SETTING_EN           	BIT(19)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO6_INT_EN                   	BIT(18)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_32K_ENTER_SETTING_EN           	BIT(18)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO5_INT_EN                   	BIT(17)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_SIE_LPM_RSM_EN_V1              	BIT(17)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO4_INT_EN                   	BIT(16)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_SIE_LPM_ACT_EN_V1              	BIT(16)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO3_INT_EN                   	BIT(15)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIOF_INT_EN_V1                	BIT(15)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO2_INT_EN                   	BIT(14)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIOE_INT_EN_V1                	BIT(14)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO1_INT_EN                   	BIT(13)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIOD_INT_EN_V1                	BIT(13)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO0_INT_EN                   	BIT(12)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIOC_INT_EN_V1                	BIT(12)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_HCI_SUS_EN                     	BIT(11)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIOB_INT_EN_V1                	BIT(11)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_HCI_RES_EN                     	BIT(10)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIOA_INT_EN_V1                	BIT(10)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_HCI_RESET_EN                   	BIT(9)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO9_INT_EN_V1                	BIT(9)
#define BIT_FS_GPIO8_INT_EN_V1                	BIT(8)
#define BIT_FS_GPIO7_INT_EN_V1                	BIT(7)
#define BIT_FS_GPIO6_INT_EN_V1                	BIT(6)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_TRPC_TO_INT_En                 	BIT(5)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO5_INT_EN_V1                	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_RPC_O_T_INT_En                 	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO4_INT_EN_V1                	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_32K_LEAVE_SETTING_MAK          	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO3_INT_EN_V1                	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_32K_ENTER_SETTING_MAK          	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO2_INT_EN_V1                	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_USB_LPMRSM_MSK                 	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO1_INT_EN_V1                	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_USB_LPMINT_MSK                 	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSIMR                              (Offset 0x0050)

#define BIT_FS_GPIO0_INT_EN_V1                	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_PDNINT                         	BIT(31)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_SPS_OCP_INT                    	BIT(29)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_SW_SPS_OCP_INT                    	BIT(29)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_PWMERR_INT                     	BIT(28)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_PWM_HW_ERR                     	BIT(28)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIOF_INT                      	BIT(27)
#define BIT_FS_GPIOE_INT                      	BIT(26)
#define BIT_FS_GPIOD_INT                      	BIT(25)
#define BIT_FS_GPIOC_INT                      	BIT(24)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_ACT2RECOVERY                      	BIT(24)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIOB_INT                      	BIT(23)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_PCIE_GEN12_SWITH                  	BIT(23)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIOA_INT                      	BIT(22)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_HCI_SUS_V1                     	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO9_INT                      	BIT(21)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_HCI_RES_V1                     	BIT(21)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO8_INT                      	BIT(20)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_HCI_RESET_V1                   	BIT(20)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO7_INT                      	BIT(19)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_32K_LEAVE_SETTING              	BIT(19)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO6_INT                      	BIT(18)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_32K_ENTER_SETTING              	BIT(18)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO5_INT                      	BIT(17)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_SIE_LPM_RSM_V1                 	BIT(17)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO4_INT                      	BIT(16)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_SIE_LPM_ACT_V1                 	BIT(16)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO3_INT                      	BIT(15)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIOF_INT_V1                   	BIT(15)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO2_INT                      	BIT(14)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIOE_INT_V1                   	BIT(14)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO1_INT                      	BIT(13)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIOD_INT_V1                   	BIT(13)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO0_INT                      	BIT(12)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIOC_INT_V1                   	BIT(12)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_HCI_SUS_INT                    	BIT(11)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIOB_INT_V1                   	BIT(11)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_HCI_RES_INT                    	BIT(10)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIOA_INT_V1                   	BIT(10)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_HCI_RESET_INT                  	BIT(9)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO9_INT_V1                   	BIT(9)
#define BIT_FS_GPIO8_INT_V1                   	BIT(8)
#define BIT_FS_GPIO7_INT_V1                   	BIT(7)
#define BIT_FS_GPIO6_INT_V1                   	BIT(6)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_TRPC_TO_INT_INT                	BIT(5)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO5_INT_V1                   	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_RPC_O_T_INT_INT                	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO4_INT_V1                   	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_32K_LEAVE_SETTING_INT          	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO3_INT_V1                   	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_32K_ENTER_SETTING_INT          	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO2_INT_V1                   	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_USB_LPMRSM_INT                 	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO1_INT_V1                   	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_USB_LPMINT_INT                 	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FSISR                              (Offset 0x0054)

#define BIT_FS_GPIO0_INT_V1                   	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HSIMR                              (Offset 0x0058)

#define BIT_GPIOF_INT_EN                      	BIT(31)
#define BIT_GPIOE_INT_EN                      	BIT(30)
#define BIT_GPIOD_INT_EN                      	BIT(29)
#define BIT_GPIOC_INT_EN                      	BIT(28)
#define BIT_GPIOB_INT_EN                      	BIT(27)
#define BIT_GPIOA_INT_EN                      	BIT(26)
#define BIT_GPIO9_INT_EN                      	BIT(25)
#define BIT_GPIO8_INT_EN                      	BIT(24)
#define BIT_GPIO7_INT_EN                      	BIT(23)
#define BIT_GPIO6_INT_EN                      	BIT(22)
#define BIT_GPIO5_INT_EN                      	BIT(21)
#define BIT_GPIO4_INT_EN                      	BIT(20)
#define BIT_GPIO3_INT_EN                      	BIT(19)
#define BIT_GPIO1_INT_EN                      	BIT(17)
#define BIT_GPIO2_INT_EN                      	BIT(16)
#define BIT_GPIO0_INT_EN                      	BIT(16)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HSIMR                              (Offset 0x0058)

#define BIT_PDNINT_EN                         	BIT(7)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HSIMR                              (Offset 0x0058)

#define BIT_PDN_INT_EN                        	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HSIMR                              (Offset 0x0058)

#define BIT_RON_INT_EN                        	BIT(6)
#define BIT_SPS_OCP_INT_EN                    	BIT(5)
#define BIT_GPIO15_0_INT_EN                   	BIT(0)

//2 REG_HSISR                              (Offset 0x005C)

#define BIT_GPIOF_INT                         	BIT(31)
#define BIT_GPIOE_INT                         	BIT(30)
#define BIT_GPIOD_INT                         	BIT(29)
#define BIT_GPIOC_INT                         	BIT(28)
#define BIT_GPIOB_INT                         	BIT(27)
#define BIT_GPIOA_INT                         	BIT(26)
#define BIT_GPIO9_INT                         	BIT(25)
#define BIT_GPIO8_INT                         	BIT(24)
#define BIT_GPIO7_INT                         	BIT(23)
#define BIT_GPIO6_INT                         	BIT(22)
#define BIT_GPIO5_INT                         	BIT(21)
#define BIT_GPIO4_INT                         	BIT(20)
#define BIT_GPIO3_INT                         	BIT(19)
#define BIT_GPIO1_INT                         	BIT(17)
#define BIT_GPIO2_INT                         	BIT(16)
#define BIT_GPIO0_INT                         	BIT(16)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HSISR                              (Offset 0x005C)

#define BIT_PDNINT                            	BIT(7)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HSISR                              (Offset 0x005C)

#define BIT_PDN_INT                           	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HSISR                              (Offset 0x005C)

#define BIT_RON_INT                           	BIT(6)
#define BIT_SPS_OCP_INT                       	BIT(5)
#define BIT_GPIO15_0_INT                      	BIT(0)

//2 REG_GPIO_EXT_CTRL                      (Offset 0x0060)


#define BIT_SHIFT_GPIO_MOD_15_TO_8            	24
#define BIT_MASK_GPIO_MOD_15_TO_8             	0xff
#define BIT_GPIO_MOD_15_TO_8(x)               	(((x) & BIT_MASK_GPIO_MOD_15_TO_8) << BIT_SHIFT_GPIO_MOD_15_TO_8)


#define BIT_SHIFT_GPIO_IO_SEL_15_TO_8         	16
#define BIT_MASK_GPIO_IO_SEL_15_TO_8          	0xff
#define BIT_GPIO_IO_SEL_15_TO_8(x)            	(((x) & BIT_MASK_GPIO_IO_SEL_15_TO_8) << BIT_SHIFT_GPIO_IO_SEL_15_TO_8)


#define BIT_SHIFT_GPIO_OUT_15_TO_8            	8
#define BIT_MASK_GPIO_OUT_15_TO_8             	0xff
#define BIT_GPIO_OUT_15_TO_8(x)               	(((x) & BIT_MASK_GPIO_OUT_15_TO_8) << BIT_SHIFT_GPIO_OUT_15_TO_8)


#define BIT_SHIFT_GPIO_IN_15_TO_8             	0
#define BIT_MASK_GPIO_IN_15_TO_8              	0xff
#define BIT_GPIO_IN_15_TO_8(x)                	(((x) & BIT_MASK_GPIO_IN_15_TO_8) << BIT_SHIFT_GPIO_IN_15_TO_8)


#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_PAPE_WLBT_SEL                     	BIT(29)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_BDEN                              	BIT(28)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_LNAON_WLBT_SEL                    	BIT(28)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_BB2PP_ISO                         	BIT(24)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_BTGP_JTAG_EN                      	BIT(24)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_XTAL_CLK_EXTARNAL_EN              	BIT(23)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_BTBRI_UART_EN                     	BIT(22)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_BTCOEX_PU                         	BIT(21)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_BTGP_UART1_EN                     	BIT(21)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_EEPROM_SEL_PD                     	BIT(20)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_BTGP_SPI_EN                       	BIT(20)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_BTGP_GPIO_E2                      	BIT(19)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_TST_MOD_PD                        	BIT(19)
#define BIT_BOOT_FLUSH_PD                     	BIT(18)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_BTGP_GPIO_EN                      	BIT(18)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_USB_XTAL_SEL1_PD                  	BIT(17)
#define BIT_USB_XTAL_SEL0_PD                  	BIT(16)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)


#define BIT_SHIFT_BTGP_GPIO_SL                	16
#define BIT_MASK_BTGP_GPIO_SL                 	0x3
#define BIT_BTGP_GPIO_SL(x)                   	(((x) & BIT_MASK_BTGP_GPIO_SL) << BIT_SHIFT_BTGP_GPIO_SL)


#endif


#if (RTL8192E_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_HST_WKE_DEV_SL                    	BIT(15)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_BTSUSB_PL                         	BIT(15)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_PAD_SDIO_SR                       	BIT(14)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_GPIO14_OUTPUT_PL                  	BIT(13)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_SW_DEVWHOST_POLARITY              	BIT(13)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_GPIO15_OUTPUT_PL                  	BIT(13)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_HOST_WAKE_PAD_PULL_EN             	BIT(12)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_HOST_WAKE_DEV_PLL_EN              	BIT(12)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_HOST_WAKE_PAD_SL                  	BIT(11)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_HOST_WAKE_DEV_POLARITY            	BIT(11)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_PAD_TRSW_SR                       	BIT(10)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_PAD_LNAON_SR                      	BIT(10)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_PAD_TRSW_E2                       	BIT(9)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_A_ANTSEL_SR                       	BIT(9)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_PAD_LNAON_E2                      	BIT(9)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_SW_TRSW_P_SEL_DATA                	BIT(8)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_A_ANTSEL_E2                       	BIT(8)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_SW_LNAON_G_SEL_DATA               	BIT(8)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_SW_TRSW_N_SEL_DATA                	BIT(7)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_D_PAPE_2G_SR                      	BIT(7)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_SW_LNAON_A_SEL_DATA               	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_PAD_PAPE_SR                       	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_D_PAPE_5G_SR                      	BIT(6)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_PAD_PAPE_E2                       	BIT(5)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_D_TRSW_SR                         	BIT(5)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_SW_PAPE_1_SEL_DATA                	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_D_TRSWB_SR                        	BIT(4)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_SW_PAPE_G_SEL_DATA                	BIT(4)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_SW_PAPE_0_SEL_DATA                	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_D_PAPE_2G_E2                      	BIT(3)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_SW_PAPE_A_SEL_DATA                	BIT(3)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_SW_ANTSEL_2_SEL_DATA              	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_D_PAPE_5G_E2                      	BIT(2)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_PAD_DPDT_SR                       	BIT(2)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_SW_ANTSEL_N_SEL_DATA              	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_D_TRSW_E2                         	BIT(1)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_PAD_DPDT_E2                       	BIT(1)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_SW_ANTSEL_P_SEL_DATA              	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_D_TRSWB_E2                        	BIT(0)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_PAD_CTRL1                          (Offset 0x0064)

#define BIT_SW_DPDT_SEL_DATA                  	BIT(0)

//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_ISO_BD2PP                         	BIT(31)
#define BIT_LDOV12B_EN                        	BIT(30)
#define BIT_CKEN_BTGPS                        	BIT(29)
#define BIT_FEN_BTGPS                         	BIT(28)
#define BIT_BTCPU_BOOTSEL                     	BIT(27)
#define BIT_SPI_SPEEDUP                       	BIT(26)
#define BIT_CKSL_BZSLP                        	BIT(23)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_BT_WAKE_HST_EN                    	BIT(22)
#define BIT_WAKE_BT_EN                        	BIT(21)
#define BIT_EN_BT                             	BIT(20)
#define BIT_BT_SUSN_EN                        	BIT(19)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_S3_RF_HW_EN                       	BIT(19)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_BT_HWROF_EN                       	BIT(19)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_BT_FUNC_EN                        	BIT(18)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_S2_RF_HW_EN                       	BIT(18)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_BT_HWPDN_SL                       	BIT(17)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_S1_RF_HW_EN                       	BIT(17)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_BT_DISN_EN                        	BIT(16)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_S0_RF_HW_EN                       	BIT(16)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_BT_PDN_PULL_EN                    	BIT(15)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_WL_PDN_PULL_EN                    	BIT(14)
#define BIT_EXTERNAL_REQUEST_PL               	BIT(13)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_GPIO0_2_3_PULL_LOW_EN             	BIT(12)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_ISO_BA2PP                         	BIT(11)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_PDN_PIN_SEL                       	BIT(10)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_BT_AFE_LDO_EN                     	BIT(10)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_GPIO11_PULL_LOW_EN                	BIT(9)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_BT_AFE_PLL_EN                     	BIT(9)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_GPIO4_PULL_LOW_EN                 	BIT(8)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_BT_DIG_CLK_EN                     	BIT(8)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_BT_WAKE_HST_SL                    	BIT(7)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_ASSERT_SPS_EN                     	BIT(7)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_WAKE_BT_SL                        	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_MASK_CHIPEN                       	BIT(6)
#define BIT_ASSERT_RF_EN                      	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_DOP_EHPAD                         	BIT(4)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_BIT_DOP_EHPAD                     	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_WL_HWROF_EN                       	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_SDIO_PAD_SHUTDOWNB                	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_WL_FUNC_EN                        	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_SDIO_CLK_SMT                      	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WL_BT_PWR_CTRL                     (Offset 0x0068)

#define BIT_WL_HWPDN_SL                       	BIT(1)
#define BIT_WL_HWPDN_EN                       	BIT(0)

//2 REG_SDM_DEBUG                          (Offset 0x006C)


#define BIT_SHIFT_F0N                         	23
#define BIT_MASK_F0N                          	0x7
#define BIT_F0N(x)                            	(((x) & BIT_MASK_F0N) << BIT_SHIFT_F0N)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_GSSR                               (Offset 0x006C)


#define BIT_SHIFT_GPIO_15_TO_0_VAL            	16
#define BIT_MASK_GPIO_15_TO_0_VAL             	0xffff
#define BIT_GPIO_15_TO_0_VAL(x)               	(((x) & BIT_MASK_GPIO_15_TO_0_VAL) << BIT_SHIFT_GPIO_15_TO_0_VAL)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SDM_DEBUG                          (Offset 0x006C)


#define BIT_SHIFT_F0F                         	10
#define BIT_MASK_F0F                          	0x1fff
#define BIT_F0F(x)                            	(((x) & BIT_MASK_F0F) << BIT_SHIFT_F0F)


#define BIT_SHIFT_DIVN                        	4
#define BIT_MASK_DIVN                         	0x3f
#define BIT_DIVN(x)                           	(((x) & BIT_MASK_DIVN) << BIT_SHIFT_DIVN)


#define BIT_SHIFT_BB_DBG_SEL_AFE_SDM          	0
#define BIT_MASK_BB_DBG_SEL_AFE_SDM           	0xf
#define BIT_BB_DBG_SEL_AFE_SDM(x)             	(((x) & BIT_MASK_BB_DBG_SEL_AFE_SDM) << BIT_SHIFT_BB_DBG_SEL_AFE_SDM)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_GSSR                               (Offset 0x006C)


#define BIT_SHIFT_GPIO_15_TO_0_EN             	0
#define BIT_MASK_GPIO_15_TO_0_EN              	0xffff
#define BIT_GPIO_15_TO_0_EN(x)                	(((x) & BIT_MASK_GPIO_15_TO_0_EN) << BIT_SHIFT_GPIO_15_TO_0_EN)


//2 REG_SYS_CLKR                           (Offset 0x0070)

#define BIT_BBRSTB_STANDBY_V1                 	BIT(28)
#define BIT_AFE_PORT3_ISO                     	BIT(27)
#define BIT_AFE_PORT2_ISO                     	BIT(26)
#define BIT_AFE_PORT1_ISO                     	BIT(25)
#define BIT_AFE_PORT0_ISO                     	BIT(24)
#define BIT_USB_PWR_OFF_SEL                   	BIT(23)
#define BIT_USB_HOST_PWR_OFF_EN_V1            	BIT(22)
#define BIT_SYM_LPS_BLOCK_EN_V1               	BIT(21)
#define BIT_USB_LPM_ACT_EN_V1                 	BIT(20)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SDIO_CTRL                          (Offset 0x0070)

#define BIT_SDIO_OFF_EN                       	BIT(17)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CLKR                           (Offset 0x0070)

#define BIT_SDIO_OFF_EN_V1                    	BIT(17)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SDIO_CTRL                          (Offset 0x0070)

#define BIT_SDIO_ON_EN                        	BIT(16)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CLKR                           (Offset 0x0070)

#define BIT_SDIO_ON_EN_V1                     	BIT(16)
#define BIT_USB3_MDIO_EN                      	BIT(12)
#define BIT_USB3_BG_EN                        	BIT(11)
#define BIT_USB3_MB_EN                        	BIT(10)

#define BIT_SHIFT_USB3_CK_MD                  	8
#define BIT_MASK_USB3_CK_MD                   	0x3
#define BIT_USB3_CK_MD(x)                     	(((x) & BIT_MASK_USB3_CK_MD) << BIT_SHIFT_USB3_CK_MD)

#define BIT_USB3_CKBUF                        	BIT(7)
#define BIT_USB3_IBX_EN                       	BIT(6)
#define BIT_U3_MB_MASK                        	BIT(5)
#define BIT_U3_BG_MASK                        	BIT(4)
#define BIT_DIS_USB3_MB_POLLING               	BIT(3)
#define BIT_PDN_MASK                          	BIT(2)
#define BIT_NO_PDN_CHIPOFF                    	BIT(1)
#define BIT_PDN_HCOUNT                        	BIT(0)

//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)


#define BIT_SHIFT_XTAL_SEL_0_V1               	28
#define BIT_MASK_XTAL_SEL_0_V1                	0xf
#define BIT_XTAL_SEL_0_V1(x)                  	(((x) & BIT_MASK_XTAL_SEL_0_V1) << BIT_SHIFT_XTAL_SEL_0_V1)

#define BIT_ISO_RFC2RF_3                      	BIT(27)
#define BIT_ISO_RFC2RF_2                      	BIT(26)
#define BIT_ISO_RFC2RF_1                      	BIT(25)
#define BIT_ISO_RFC2RF_0                      	BIT(24)

#define BIT_SHIFT_ANAPAR_RFC3                 	16
#define BIT_MASK_ANAPAR_RFC3                  	0xff
#define BIT_ANAPAR_RFC3(x)                    	(((x) & BIT_MASK_ANAPAR_RFC3) << BIT_SHIFT_ANAPAR_RFC3)

#define BIT_R_FORCE_CLK_U3                    	BIT(13)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_USB_HOST_PWR_OFF_EN               	BIT(12)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_R_USB2_AUTOLOAD                   	BIT(12)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_SYM_LPS_BLOCK_EN                  	BIT(11)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_FORCE_U2CK                        	BIT(11)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_USB_LPM_ACT_EN                    	BIT(10)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_FORCE_CLK                         	BIT(10)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_USB_LPM_NY                        	BIT(9)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_U2_FORCE                          	BIT(9)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_USB_SUS_DIS                       	BIT(8)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_U3_FORCE                          	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)


#define BIT_SHIFT_SDIO_PAD_E                  	5
#define BIT_MASK_SDIO_PAD_E                   	0x7
#define BIT_SDIO_PAD_E(x)                     	(((x) & BIT_MASK_SDIO_PAD_E) << BIT_SHIFT_SDIO_PAD_E)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_USB_LPPLL_EN                      	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_SDIO_H3L1                         	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_ROP_SW15                          	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)


#define BIT_SHIFT_USB23_SW_MODE               	2
#define BIT_MASK_USB23_SW_MODE                	0x3
#define BIT_USB23_SW_MODE(x)                  	(((x) & BIT_MASK_USB23_SW_MODE) << BIT_SHIFT_USB23_SW_MODE)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_PCI_CKRDY_OPT                     	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_PCLK_VLD_SEL                      	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_PCI_VAUX_EN                       	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HCI_OPT_CTRL                       (Offset 0x0074)

#define BIT_VAUX_EN                           	BIT(0)

//2 REG_AFE_XTAL_CTRL_EXT                  (Offset 0x0078)

#define BIT_SDM_Order                         	BIT(30)
#define BIT_XTAL_DRV_RF_LATCH_V1              	BIT(29)
#define BIT_XTAL_VDD_SEL_V1                   	BIT(28)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL4                          (Offset 0x0078)

#define BIT_XTAL_DRV_RF_LATCH                 	BIT(27)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_XTAL_CTRL_EXT                  (Offset 0x0078)

#define BIT_XQSEL_RF_AWAKE_V1                 	BIT(27)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL4                          (Offset 0x0078)

#define BIT_XTAL_VDD_SEL                      	BIT(26)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_XTAL_CTRL_EXT                  (Offset 0x0078)

#define BIT_GATED_XTAL_OK0_V1                 	BIT(26)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_AFE_CTRL4                          (Offset 0x0078)

#define BIT_XQSEL_RF                          	BIT(25)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_AFE_CTRL4                          (Offset 0x0078)

#define BIT_XQSEL_RF_AWAKE                    	BIT(25)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_AFE_CTRL4                          (Offset 0x0078)

#define BIT_XQSEL_RF_initial                  	BIT(24)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_AFE_CTRL4                          (Offset 0x0078)

#define BIT_XQSEL_BIT1                        	BIT(24)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_AFE_CTRL4                          (Offset 0x0078)

#define BIT_reg_vref_sel                      	BIT(23)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_XTAL_CTRL_EXT                  (Offset 0x0078)


#define BIT_SHIFT_F0N_2_TO_0                  	23
#define BIT_MASK_F0N_2_TO_0                   	0x7
#define BIT_F0N_2_TO_0(x)                     	(((x) & BIT_MASK_F0N_2_TO_0) << BIT_SHIFT_F0N_2_TO_0)


#endif


#if (RTL8192E_SUPPORT)


//2 REG_AFE_CTRL4                          (Offset 0x0078)

#define BIT_reg_lpfen                         	BIT(22)
#define BIT_reg_kvco                          	BIT(21)
#define BIT_XTAL_DRV_AGPIO_BIT1               	BIT(20)
#define BIT_XTAL_DRV_AGPIO_BIT0               	BIT(19)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL4                          (Offset 0x0078)

#define BIT_XTAL_GRF2                         	BIT(18)
#define BIT_reg_ref_sel                       	BIT(17)
#define BIT_Reg_320_sel                       	BIT(16)
#define BIT_EN_SYM                            	BIT(15)

#define BIT_SHIFT_Ioffset                     	10
#define BIT_MASK_Ioffset                      	0x1f
#define BIT_Ioffset(x)                        	(((x) & BIT_MASK_Ioffset) << BIT_SHIFT_Ioffset)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_XTAL_CTRL_EXT                  (Offset 0x0078)


#define BIT_SHIFT_F0F_12_TO_0                 	10
#define BIT_MASK_F0F_12_TO_0                  	0x1fff
#define BIT_F0F_12_TO_0(x)                    	(((x) & BIT_MASK_F0F_12_TO_0) << BIT_SHIFT_F0F_12_TO_0)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL4                          (Offset 0x0078)


#define BIT_SHIFT_APLL_FREF_SEL_BIT_2_TO_1    	8
#define BIT_MASK_APLL_FREF_SEL_BIT_2_TO_1     	0x3
#define BIT_APLL_FREF_SEL_BIT_2_TO_1(x)       	(((x) & BIT_MASK_APLL_FREF_SEL_BIT_2_TO_1) << BIT_SHIFT_APLL_FREF_SEL_BIT_2_TO_1)

#define BIT_APLL_FREF_SEL_BIT3                	BIT(7)

#define BIT_SHIFT_APLL_LDO_V12ADJ             	5
#define BIT_MASK_APLL_LDO_V12ADJ              	0x3
#define BIT_APLL_LDO_V12ADJ(x)                	(((x) & BIT_MASK_APLL_LDO_V12ADJ) << BIT_SHIFT_APLL_LDO_V12ADJ)

#define BIT_APLL_160_GATEB                    	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_XTAL_CTRL_EXT                  (Offset 0x0078)


#define BIT_SHIFT_DIVN_5_TO_0                 	4
#define BIT_MASK_DIVN_5_TO_0                  	0x3f
#define BIT_DIVN_5_TO_0(x)                    	(((x) & BIT_MASK_DIVN_5_TO_0) << BIT_SHIFT_DIVN_5_TO_0)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AFE_CTRL4                          (Offset 0x0078)

#define BIT_AFE_DUMMY                         	BIT(3)
#define BIT_reg_idouble                       	BIT(2)
#define BIT_reg_vco_bias_BIT0                 	BIT(1)
#define BIT_reg_vco_bias_BIT1                 	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_AFE_XTAL_CTRL_EXT                  (Offset 0x0078)


#define BIT_SHIFT_BB_DBG_SEL_AFE_SDM_3_TO_0   	0
#define BIT_MASK_BB_DBG_SEL_AFE_SDM_3_TO_0    	0xf
#define BIT_BB_DBG_SEL_AFE_SDM_3_TO_0(x)      	(((x) & BIT_MASK_BB_DBG_SEL_AFE_SDM_3_TO_0) << BIT_SHIFT_BB_DBG_SEL_AFE_SDM_3_TO_0)


//2 REG_LDO_SWR_CTRL                       (Offset 0x007C)

#define BIT_REF_FREF_EDGE                     	BIT(29)
#define BIT_REG_VREF_SEL_V1                   	BIT(28)

#define BIT_SHIFT_REG_CP_OFFSET_4_TO_0        	23
#define BIT_MASK_REG_CP_OFFSET_4_TO_0         	0x1f
#define BIT_REG_CP_OFFSET_4_TO_0(x)           	(((x) & BIT_MASK_REG_CP_OFFSET_4_TO_0) << BIT_SHIFT_REG_CP_OFFSET_4_TO_0)


#define BIT_SHIFT_REG_RS_SET_2_TO_0           	20
#define BIT_MASK_REG_RS_SET_2_TO_0            	0x7
#define BIT_REG_RS_SET_2_TO_0(x)              	(((x) & BIT_MASK_REG_RS_SET_2_TO_0) << BIT_SHIFT_REG_RS_SET_2_TO_0)


#define BIT_SHIFT_REG_CS_SET_1_TO_0           	18
#define BIT_MASK_REG_CS_SET_1_TO_0            	0x3
#define BIT_REG_CS_SET_1_TO_0(x)              	(((x) & BIT_MASK_REG_CS_SET_1_TO_0) << BIT_SHIFT_REG_CS_SET_1_TO_0)


#define BIT_SHIFT_REG_CP_SET_1_TO_0           	16
#define BIT_MASK_REG_CP_SET_1_TO_0            	0x3
#define BIT_REG_CP_SET_1_TO_0(x)              	(((x) & BIT_MASK_REG_CP_SET_1_TO_0) << BIT_SHIFT_REG_CP_SET_1_TO_0)

#define BIT_LPFEN                             	BIT(15)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_LDO_SWR_CTRL                       (Offset 0x007C)


#define BIT_SHIFT_LDO_HV5_DUMMY               	14
#define BIT_MASK_LDO_HV5_DUMMY                	0x3
#define BIT_LDO_HV5_DUMMY(x)                  	(((x) & BIT_MASK_LDO_HV5_DUMMY) << BIT_SHIFT_LDO_HV5_DUMMY)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_LDO_SWR_CTRL                       (Offset 0x007C)

#define BIT_REG_DOGENB                        	BIT(14)
#define BIT_REG_TEST_EN                       	BIT(13)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_LDO_SWR_CTRL                       (Offset 0x007C)


#define BIT_SHIFT_reg_vtune33                 	12
#define BIT_MASK_reg_vtune33                  	0x3
#define BIT_reg_vtune33(x)                    	(((x) & BIT_MASK_reg_vtune33) << BIT_SHIFT_reg_vtune33)


#define BIT_SHIFT_reg_standby33               	10
#define BIT_MASK_reg_standby33                	0x3
#define BIT_reg_standby33(x)                  	(((x) & BIT_MASK_reg_standby33) << BIT_SHIFT_reg_standby33)


#define BIT_SHIFT_reg_load33                  	8
#define BIT_MASK_reg_load33                   	0x3
#define BIT_reg_load33(x)                     	(((x) & BIT_MASK_reg_load33) << BIT_SHIFT_reg_load33)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_LDO_SWR_CTRL                       (Offset 0x007C)


#define BIT_SHIFT_REG_DIV_SEL                 	8
#define BIT_MASK_REG_DIV_SEL                  	0x1f
#define BIT_REG_DIV_SEL(x)                    	(((x) & BIT_MASK_REG_DIV_SEL) << BIT_SHIFT_REG_DIV_SEL)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LDO_SWR_CTRL                       (Offset 0x007C)

#define BIT_reg_bypass_L                      	BIT(7)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_LDO_SWR_CTRL                       (Offset 0x007C)

#define BIT_EN_CK200M                         	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LDO_SWR_CTRL                       (Offset 0x007C)

#define BIT_reg_LDOf_L                        	BIT(6)
#define BIT_reg_OCPS_L                        	BIT(5)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_LDO_SWR_CTRL                       (Offset 0x007C)


#define BIT_SHIFT_REG_KVCO_200M_1_TO_0        	5
#define BIT_MASK_REG_KVCO_200M_1_TO_0         	0x3
#define BIT_REG_KVCO_200M_1_TO_0(x)           	(((x) & BIT_MASK_REG_KVCO_200M_1_TO_0) << BIT_SHIFT_REG_KVCO_200M_1_TO_0)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LDO_SWR_CTRL                       (Offset 0x007C)


#define BIT_SHIFT_dummy                       	3
#define BIT_MASK_dummy                        	0x3
#define BIT_dummy(x)                          	(((x) & BIT_MASK_dummy) << BIT_SHIFT_dummy)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_LDO_SWR_CTRL                       (Offset 0x007C)


#define BIT_SHIFT_reg_cp_bias_200M_2_TO_0     	2
#define BIT_MASK_reg_cp_bias_200M_2_TO_0      	0x7
#define BIT_reg_cp_bias_200M_2_TO_0(x)        	(((x) & BIT_MASK_reg_cp_bias_200M_2_TO_0) << BIT_SHIFT_reg_cp_bias_200M_2_TO_0)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LDO_SWR_CTRL                       (Offset 0x007C)


#define BIT_SHIFT_CFC_L_BIT_1_TO_0            	1
#define BIT_MASK_CFC_L_BIT_1_TO_0             	0x3
#define BIT_CFC_L_BIT_1_TO_0(x)               	(((x) & BIT_MASK_CFC_L_BIT_1_TO_0) << BIT_SHIFT_CFC_L_BIT_1_TO_0)

#define BIT_reg_type_L                        	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_LDO_SWR_CTRL                       (Offset 0x007C)

#define BIT_XCK_OUT_EN                        	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)


#define BIT_SHIFT_RPWM                        	24
#define BIT_MASK_RPWM                         	0xff
#define BIT_RPWM(x)                           	(((x) & BIT_MASK_RPWM) << BIT_SHIFT_RPWM)

#define BIT_CPRST                             	BIT(23)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_ANA_PORT_EN                       	BIT(22)
#define BIT_MAC_PORT_EN                       	BIT(21)
#define BIT_BOOT_FSPI_EN                      	BIT(20)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_ROM_DLEN                          	BIT(19)

#define BIT_SHIFT_ROM_PGE                     	16
#define BIT_MASK_ROM_PGE                      	0x7
#define BIT_ROM_PGE(x)                        	(((x) & BIT_MASK_ROM_PGE) << BIT_SHIFT_ROM_PGE)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_FW_INIT_RDY                       	BIT(15)
#define BIT_FW_DW_RDY                         	BIT(14)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_FWDL_RsvdPage_Rdy                 	BIT(12)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)


#define BIT_SHIFT_CPU_CLK_SEL                 	12
#define BIT_MASK_CPU_CLK_SEL                  	0x3
#define BIT_CPU_CLK_SEL(x)                    	(((x) & BIT_MASK_CPU_CLK_SEL) << BIT_SHIFT_CPU_CLK_SEL)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_R_8051_ROMDLFW_EN                 	BIT(11)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_CCLK_CHG_MASK                     	BIT(11)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_r_8051_init_rdy                   	BIT(10)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_EMEM__TXBUF_CHKSUM_OK             	BIT(10)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_R_8051_SPD                        	BIT(9)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_EMEM_TXBUF_DW_RDY                 	BIT(9)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_R_8051_GAT                        	BIT(8)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_EMEM_CHKSUM_OK                    	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_RAM_DL_SEL                        	BIT(7)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_EMEM_DW_OK                        	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT || RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_WINTINI_RDY                       	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_DMEM_CHKSUM_OK                    	BIT(6)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_RFINI_RDY                         	BIT(5)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_DMEM_DW_OK                        	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_BBINI_RDY                         	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_IMEM_CHKSUM_OK                    	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_MACINI_RDY                        	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_IMEM_DW_OK                        	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT || RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_FWDL_CHK_RPT                      	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_IMEM_BOOT_LOAD_CHKSUM_OK          	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT || RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_MCUFWDL_RDY                       	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_IMEM_BOOT_LOAD_DW_OK              	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_8051FW_CTRL                        (Offset 0x0080)

#define BIT_MCUFWDL_EN                        	BIT(0)

//2 REG_MCU_TST_CFG                        (Offset 0x0084)


#define BIT_SHIFT_LBKTST                      	0
#define BIT_MASK_LBKTST                       	0xffff
#define BIT_LBKTST(x)                         	(((x) & BIT_MASK_LBKTST) << BIT_SHIFT_LBKTST)


//2 REG_HMEBOX_E0_E1                       (Offset 0x0088)


#define BIT_SHIFT_HOST_MSG_E1                 	16
#define BIT_MASK_HOST_MSG_E1                  	0xffff
#define BIT_HOST_MSG_E1(x)                    	(((x) & BIT_MASK_HOST_MSG_E1) << BIT_SHIFT_HOST_MSG_E1)


#define BIT_SHIFT_HOST_MSG_E0                 	0
#define BIT_MASK_HOST_MSG_E0                  	0xffff
#define BIT_HOST_MSG_E0(x)                    	(((x) & BIT_MASK_HOST_MSG_E0) << BIT_SHIFT_HOST_MSG_E0)


//2 REG_HMEBOX_E2_E3                       (Offset 0x008C)


#define BIT_SHIFT_HOST_MSG_E3                 	16
#define BIT_MASK_HOST_MSG_E3                  	0xffff
#define BIT_HOST_MSG_E3(x)                    	(((x) & BIT_MASK_HOST_MSG_E3) << BIT_SHIFT_HOST_MSG_E3)


#define BIT_SHIFT_HOST_MSG_E2                 	0
#define BIT_MASK_HOST_MSG_E2                  	0xffff
#define BIT_HOST_MSG_E2(x)                    	(((x) & BIT_MASK_HOST_MSG_E2) << BIT_SHIFT_HOST_MSG_E2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WLLPS_CTRL                         (Offset 0x0090)

#define BIT_WLLPSOP_EABM                      	BIT(31)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WLLPS_CTRL                         (Offset 0x0090)

#define BIT_WLLPSOP_ACKF                      	BIT(30)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WLLPS_CTRL                         (Offset 0x0090)

#define BIT_WLLPSOP_DLDM                      	BIT(29)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_WLLPS_CTRL                         (Offset 0x0090)

#define BIT_WLLPSOP_AFEP                      	BIT(29)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WLLPS_CTRL                         (Offset 0x0090)

#define BIT_WLLPSOP_ESWR                      	BIT(28)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_WLLPS_CTRL                         (Offset 0x0090)

#define BIT_LPS_DIS_SW                        	BIT(28)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WLLPS_CTRL                         (Offset 0x0090)

#define BIT_WLLPSOP_PWMM                      	BIT(27)
#define BIT_WLLPSOP_EECK                      	BIT(26)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_WLLPS_CTRL                         (Offset 0x0090)

#define BIT_WLLPSOP_ELDO                      	BIT(25)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WLLPS_CTRL                         (Offset 0x0090)

#define BIT_WLLPSOP_EXTAL                     	BIT(24)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_WLLPS_CTRL                         (Offset 0x0090)

#define BIT_LPS_BB_REG_EN                     	BIT(23)
#define BIT_LPS_BB_PWR_EN                     	BIT(22)
#define BIT_LPS_BB_GLB_EN                     	BIT(21)
#define BIT_SUS_DIS_SW                        	BIT(15)
#define BIT_SUS_SKP_PAGE0_ALD                 	BIT(14)
#define BIT_SUS_LDO_SLEEP                     	BIT(13)
#define BIT_PFM_EN_ZCD                        	BIT(12)
#define BIT_KEEP_RFC_EN                       	BIT(11)
#define BIT_MACON_NO_RFCISO_RELEASE           	BIT(10)
#define BIT_MACON_NO_AFEPORT_PWR              	BIT(9)
#define BIT_MACON_NO_CPU_EN                   	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WLLPS_CTRL                         (Offset 0x0090)

#define BIT_WL_LPS_EN                         	BIT(0)

//2 REG_GPIO_DEBOUNCE_CTRL                 (Offset 0x0098)

#define BIT_WLGP_DBC1EN                       	BIT(15)

#define BIT_SHIFT_WLGP_DBC1                   	8
#define BIT_MASK_WLGP_DBC1                    	0xf
#define BIT_WLGP_DBC1(x)                      	(((x) & BIT_MASK_WLGP_DBC1) << BIT_SHIFT_WLGP_DBC1)

#define BIT_WLGP_DBC0EN                       	BIT(7)

#define BIT_SHIFT_WLGP_DBC0                   	0
#define BIT_MASK_WLGP_DBC0                    	0xf
#define BIT_WLGP_DBC0(x)                      	(((x) & BIT_MASK_WLGP_DBC0) << BIT_SHIFT_WLGP_DBC0)


//2 REG_RPWM2                              (Offset 0x009C)


#define BIT_SHIFT_RPWM2                       	16
#define BIT_MASK_RPWM2                        	0xffff
#define BIT_RPWM2(x)                          	(((x) & BIT_MASK_RPWM2) << BIT_SHIFT_RPWM2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYSON_FSM_MON                      (Offset 0x00A0)


#define BIT_SHIFT_FSM_MON_SEL                 	24
#define BIT_MASK_FSM_MON_SEL                  	0x7
#define BIT_FSM_MON_SEL(x)                    	(((x) & BIT_MASK_FSM_MON_SEL) << BIT_SHIFT_FSM_MON_SEL)

#define BIT_DOP_ELDO                          	BIT(23)
#define BIT_FSM_MON_UPD                       	BIT(15)

#define BIT_SHIFT_FSM_PAR                     	0
#define BIT_MASK_FSM_PAR                      	0x7fff
#define BIT_FSM_PAR(x)                        	(((x) & BIT_MASK_FSM_PAR) << BIT_SHIFT_FSM_PAR)


//2 REG_PMC_DBG_CTRL1                      (Offset 0x00A8)

#define BIT_PMC_WR_OVF                        	BIT(8)

#define BIT_SHIFT_WLPMC_ERRINT                	0
#define BIT_MASK_WLPMC_ERRINT                 	0xff
#define BIT_WLPMC_ERRINT(x)                   	(((x) & BIT_MASK_WLPMC_ERRINT) << BIT_SHIFT_WLPMC_ERRINT)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR0                              (Offset 0x00B0)

#define BIT_Timeout_interrupt2_MASK           	BIT(31)
#define BIT_Timeout_interrutp1_MASK           	BIT(30)
#define BIT_PSTIMEOUT_MSK                     	BIT(29)
#define BIT_GTINT4_MSK                        	BIT(28)
#define BIT_GTINT3_MSK                        	BIT(27)
#define BIT_TXBCN0ERR_MSK                     	BIT(26)
#define BIT_TXBCN0OK_MSK                      	BIT(25)
#define BIT_TSF_BIT32_TOGGLE_MSK              	BIT(24)
#define BIT_BcnDMAInt0_MSK                    	BIT(20)
#define BIT_BcnDERR0_MSK                      	BIT(16)
#define BIT_HSISR_IND_ON_INT_MSK              	BIT(15)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR0                              (Offset 0x00B0)

#define BIT_BcnDMAInt_E_MSK                   	BIT(14)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HIMR0                              (Offset 0x00B0)

#define BIT_HISR3_IND_INT_MSK                 	BIT(14)
#define BIT_HISR2_IND_INT_MSK                 	BIT(13)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR0                              (Offset 0x00B0)

#define BIT_CTWend_MSK                        	BIT(12)
#define BIT_HISR1_IND_MSK                     	BIT(11)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HIMR0                              (Offset 0x00B0)

#define BIT_HISR1_IND_INT_MSK                 	BIT(11)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR0                              (Offset 0x00B0)

#define BIT_C2HCMD_MSK                        	BIT(10)
#define BIT_CPWM2_MSK                         	BIT(9)
#define BIT_CPWM_MSK                          	BIT(8)
#define BIT_HIGHDOK_MSK                       	BIT(7)
#define BIT_MGTDOK_MSK                        	BIT(6)
#define BIT_BKDOK_MSK                         	BIT(5)
#define BIT_BEDOK_MSK                         	BIT(4)
#define BIT_VIDOK_MSK                         	BIT(3)
#define BIT_VODOK_MSK                         	BIT(2)
#define BIT_RDU_MSK                           	BIT(1)
#define BIT_RXOK_MSK                          	BIT(0)

//2 REG_HISR0                              (Offset 0x00B4)

#define BIT_PSTIMEOUT2                	        BIT(31)
#define BIT_PSTIMEOUT1                	        BIT(30)
#define BIT_PSTIMEOUT                         	BIT(29)
#define BIT_GTINT4                            	BIT(28)
#define BIT_GTINT3                            	BIT(27)
#define BIT_TXBCN0ERR                         	BIT(26)
#define BIT_TXBCN0OK                          	BIT(25)
#define BIT_TSF_BIT32_TOGGLE                  	BIT(24)
#define BIT_BCNDMAINT0                        	BIT(20)
#define BIT_BCNDERR0                          	BIT(16)
#define BIT_HSISR_IND_ON_INT                  	BIT(15)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HISR0                              (Offset 0x00B4)

#define BIT_BcnDMAInt_E                       	BIT(14)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HISR0                              (Offset 0x00B4)

#define BIT_HISR3_IND_INT                     	BIT(14)
#define BIT_HISR2_IND_INT                     	BIT(13)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HISR0                              (Offset 0x00B4)

#define BIT_CTWEND                            	BIT(12)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HISR0                              (Offset 0x00B4)

#define BIT_HISR1_IND_INT                     	BIT(11)
#define BIT_C2HCMD                            	BIT(10)
#define BIT_CPWM2                             	BIT(9)
#define BIT_CPWM                              	BIT(8)
#define BIT_HIGHDOK                           	BIT(7)
#define BIT_MGTDOK                            	BIT(6)
#define BIT_BKDOK                             	BIT(5)
#define BIT_BEDOK                             	BIT(4)
#define BIT_VIDOK                             	BIT(3)
#define BIT_VODOK                             	BIT(2)
#define BIT_RDU                               	BIT(1)
#define BIT_RXOK                              	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_MCU_ERR_MASK                      	BIT(28)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_BcnDMAInt7__MSK                   	BIT(27)
#define BIT_BcnDMAInt6__MSK                   	BIT(26)
#define BIT_BcnDMAInt5__MSK                   	BIT(25)
#define BIT_BcnDMAInt4__MSK                   	BIT(24)
#define BIT_BcnDMAInt3_MSK                    	BIT(23)
#define BIT_BcnDMAInt2_MSK                    	BIT(22)
#define BIT_BcnDMAInt1_MSK                    	BIT(21)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_BcnDERR7_MSK                      	BIT(20)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_BcnDERR7__MSK                     	BIT(20)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_BcnDERR6_MSK                      	BIT(19)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_BcnDERR6__MSK                     	BIT(19)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_BcnDERR5_MSK                      	BIT(18)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_BcnDERR5__MSK                     	BIT(18)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_BcnDERR4_MSK                      	BIT(17)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_BcnDERR4__MSK                     	BIT(17)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_BcnDERR3_MSK                      	BIT(16)
#define BIT_BcnDERR2_MSK                      	BIT(15)
#define BIT_BcnDERR1_MSK                      	BIT(14)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_ATIMend_E_MSK                     	BIT(13)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_ATIMend__MSK                      	BIT(12)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_TXERR_MSK                         	BIT(11)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_TXERR__MSK                        	BIT(11)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_RXERR_MSK                         	BIT(10)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_RXERR__MSK                        	BIT(10)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_TXFOVW_MSK                        	BIT(9)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_TXFOVW__MSK                       	BIT(9)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_FOVW_MSK                          	BIT(8)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HIMR1                              (Offset 0x00B8)

#define BIT_FOVW__MSK                         	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HISR1                              (Offset 0x00BC)

#define BIT_MCU_ERR                           	BIT(28)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HISR1                              (Offset 0x00BC)

#define BIT_BCNDMAINT7                        	BIT(27)
#define BIT_BCNDMAINT6                        	BIT(26)
#define BIT_BCNDMAINT5                        	BIT(25)
#define BIT_BCNDMAINT4                        	BIT(24)
#define BIT_BCNDMAINT3                        	BIT(23)
#define BIT_BCNDMAINT2                        	BIT(22)
#define BIT_BCNDMAINT1                        	BIT(21)
#define BIT_BCNDERR7                          	BIT(20)
#define BIT_BCNDERR6                          	BIT(19)
#define BIT_BCNDERR5                          	BIT(18)
#define BIT_BCNDERR4                          	BIT(17)
#define BIT_BCNDERR3                          	BIT(16)
#define BIT_BCNDERR2                          	BIT(15)
#define BIT_BCNDERR1                          	BIT(14)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HISR1                              (Offset 0x00BC)

#define BIT_ATIMEND_E                         	BIT(13)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HISR1                              (Offset 0x00BC)

#define BIT_ATIMEND                           	BIT(12)
#define BIT_TXERR_INT                         	BIT(11)
#define BIT_RXERR_INT                         	BIT(10)
#define BIT_TXFOVW                            	BIT(9)
#define BIT_FOVW                              	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_DBG_PORT_SEL                       (Offset 0x00C0)


#define BIT_SHIFT_DEBUG_ST                    	0
#define BIT_MASK_DEBUG_ST                     	0xffffffffL
#define BIT_DEBUG_ST(x)                       	(((x) & BIT_MASK_DEBUG_ST) << BIT_SHIFT_DEBUG_ST)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL2                          (Offset 0x00C4)

#define BIT_FCSN_PU                           	BIT(18)
#define BIT_KEEP_PAD                          	BIT(17)
#define BIT_PAD_ALD_SKP                       	BIT(16)
#define BIT_PAD_A_ANTSEL_E                    	BIT(11)
#define BIT_PAD_A_ANTSELB_E                   	BIT(10)
#define BIT_PAD_A_ANTSEL_O                    	BIT(9)
#define BIT_PAD_A_ANTSELB_O                   	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PAD_CTRL2                          (Offset 0x00C4)

#define BIT_LD_B12V_EN                        	BIT(7)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PAD_CTRL2                          (Offset 0x00C4)

#define BIT_B15V_EN                           	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PAD_CTRL2                          (Offset 0x00C4)

#define BIT_EESK_IOSEL                        	BIT(6)
#define BIT_EESK_DATA_O                       	BIT(5)
#define BIT_EESK_DATA_I                       	BIT(4)
#define BIT_EECS_IOSEL                        	BIT(2)
#define BIT_EECS_DATA_O                       	BIT(1)
#define BIT_EECS_DATA_I                       	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_MEM_RMC                            (Offset 0x00C8)

#define BIT_MEM_RMV_SIGN                      	BIT(31)
#define BIT_MEM_RMV_2PRF1                     	BIT(29)
#define BIT_MEM_RMV_2PRF0                     	BIT(28)
#define BIT_MEM_RMV_1PRF1                     	BIT(27)
#define BIT_MEM_RMV_1PRF0                     	BIT(26)
#define BIT_MEM_RMV_1PSR                      	BIT(25)
#define BIT_MEM_RMV_ROM                       	BIT(24)

#define BIT_SHIFT_MEM_RME_WL_V2               	4
#define BIT_MASK_MEM_RME_WL_V2                	0x3f
#define BIT_MEM_RME_WL_V2(x)                  	(((x) & BIT_MASK_MEM_RME_WL_V2) << BIT_SHIFT_MEM_RME_WL_V2)


#define BIT_SHIFT_MEM_RME_HCI_V2              	0
#define BIT_MASK_MEM_RME_HCI_V2               	0x1f
#define BIT_MEM_RME_HCI_V2(x)                 	(((x) & BIT_MASK_MEM_RME_HCI_V2) << BIT_SHIFT_MEM_RME_HCI_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PMC_DBG_CTRL2                      (Offset 0x00CC)


#define BIT_SHIFT_EFUSE_BURN_GNT              	24
#define BIT_MASK_EFUSE_BURN_GNT               	0xff
#define BIT_EFUSE_BURN_GNT(x)                 	(((x) & BIT_MASK_EFUSE_BURN_GNT) << BIT_SHIFT_EFUSE_BURN_GNT)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_PMC_DBG_CTRL2                      (Offset 0x00CC)


#define BIT_SHIFT_EFUSE_PG_PWD                	24
#define BIT_MASK_EFUSE_PG_PWD                 	0xff
#define BIT_EFUSE_PG_PWD(x)                   	(((x) & BIT_MASK_EFUSE_PG_PWD) << BIT_SHIFT_EFUSE_PG_PWD)

#define BIT_DBG_READ_EN                       	BIT(16)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PMC_DBG_CTRL2                      (Offset 0x00CC)

#define BIT_STOP_WL_PMC                       	BIT(9)
#define BIT_STOP_SYM_PMC                      	BIT(8)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PMC_DBG_CTRL2                      (Offset 0x00CC)


#define BIT_SHIFT_EDATA1_V1                   	8
#define BIT_MASK_EDATA1_V1                    	0xff
#define BIT_EDATA1_V1(x)                      	(((x) & BIT_MASK_EDATA1_V1) << BIT_SHIFT_EDATA1_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PMC_DBG_CTRL2                      (Offset 0x00CC)

#define BIT_REG_RST_WLPMC                     	BIT(5)
#define BIT_REG_RST_PD12N                     	BIT(4)
#define BIT_SYSON_DIS_WLREG_WRMSK             	BIT(3)
#define BIT_SYSON_DIS_PMCREG_WRMSK            	BIT(2)

#define BIT_SHIFT_SYSON_REG_ARB               	0
#define BIT_MASK_SYSON_REG_ARB                	0x3
#define BIT_SYSON_REG_ARB(x)                  	(((x) & BIT_MASK_SYSON_REG_ARB) << BIT_SHIFT_SYSON_REG_ARB)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_PMC_DBG_CTRL2                      (Offset 0x00CC)


#define BIT_SHIFT_EDATA0_V1                   	0
#define BIT_MASK_EDATA0_V1                    	0xff
#define BIT_EDATA0_V1(x)                      	(((x) & BIT_MASK_EDATA0_V1) << BIT_SHIFT_EDATA0_V1)


//2 REG_BIST_CTRL                          (Offset 0x00D0)

#define BIT_SCAN_PLL_BYPASS                   	BIT(30)
#define BIT_DRF_BIST_FAIL_V1                  	BIT(28)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BIST_CTRL                          (Offset 0x00D0)

#define BIT_BIST_USB_DIS                      	BIT(27)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_BIST_CTRL                          (Offset 0x00D0)

#define BIT_DRF_BIST_READY_V1                 	BIT(27)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BIST_CTRL                          (Offset 0x00D0)

#define BIT_BIST_PCI_DIS                      	BIT(26)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_BIST_CTRL                          (Offset 0x00D0)

#define BIT_BIST_FAIL_V1                      	BIT(26)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BIST_CTRL                          (Offset 0x00D0)

#define BIT_BIST_BT_DIS                       	BIT(25)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_BIST_CTRL                          (Offset 0x00D0)

#define BIT_BIST_READY_V1                     	BIT(25)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BIST_CTRL                          (Offset 0x00D0)

#define BIT_BIST_WL_DIS                       	BIT(24)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_BIST_CTRL                          (Offset 0x00D0)

#define BIT_BIST_START_PAUSE_V1               	BIT(24)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BIST_CTRL                          (Offset 0x00D0)


#define BIT_SHIFT_BIST_RPT_SEL                	16
#define BIT_MASK_BIST_RPT_SEL                 	0xf
#define BIT_BIST_RPT_SEL(x)                   	(((x) & BIT_MASK_BIST_RPT_SEL) << BIT_SHIFT_BIST_RPT_SEL)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_BIST_CTRL                          (Offset 0x00D0)


#define BIT_SHIFT_MBIST_RSTNI                 	8
#define BIT_MASK_MBIST_RSTNI                  	0x3ff
#define BIT_MBIST_RSTNI(x)                    	(((x) & BIT_MASK_MBIST_RSTNI) << BIT_SHIFT_MBIST_RSTNI)

#define BIT_BIST_RESUME_PS_V1                 	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BIST_CTRL                          (Offset 0x00D0)

#define BIT_BIST_RESUME_PS                    	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_BIST_CTRL                          (Offset 0x00D0)

#define BIT_BIST_RESUME_V1                    	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BIST_CTRL                          (Offset 0x00D0)

#define BIT_BIST_RESUME                       	BIT(3)
#define BIT_BIST_NORMAL                       	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_BIST_CTRL                          (Offset 0x00D0)


#define BIT_SHIFT_BIST_MODE                   	2
#define BIT_MASK_BIST_MODE                    	0x3
#define BIT_BIST_MODE(x)                      	(((x) & BIT_MASK_BIST_MODE) << BIT_SHIFT_BIST_MODE)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BIST_CTRL                          (Offset 0x00D0)

#define BIT_BIST_RSTN                         	BIT(1)
#define BIT_BIST_CLK_EN                       	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BIST_RPT                           (Offset 0x00D4)


#define BIT_SHIFT_MBIST_REPORT                	0
#define BIT_MASK_MBIST_REPORT                 	0xffffffffL
#define BIT_MBIST_REPORT(x)                   	(((x) & BIT_MASK_MBIST_REPORT) << BIT_SHIFT_MBIST_REPORT)


#endif


#if (RTL8192E_SUPPORT)


//2 REG_MEM_CTRL                           (Offset 0x00D8)

#define BIT_RMV_SIGN                          	BIT(31)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_MEM_CTRL                           (Offset 0x00D8)

#define BIT_UMEM_RME                          	BIT(31)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_MEM_CTRL                           (Offset 0x00D8)

#define BIT_RMV_2PRF1                         	BIT(29)
#define BIT_RMV_2PRF0                         	BIT(28)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_MEM_CTRL                           (Offset 0x00D8)


#define BIT_SHIFT_BT_SPRAM                    	28
#define BIT_MASK_BT_SPRAM                     	0x3
#define BIT_BT_SPRAM(x)                       	(((x) & BIT_MASK_BT_SPRAM) << BIT_SHIFT_BT_SPRAM)


#endif


#if (RTL8192E_SUPPORT)


//2 REG_MEM_CTRL                           (Offset 0x00D8)

#define BIT_RMV_1PRF1                         	BIT(27)
#define BIT_RMV_1PRF0                         	BIT(26)
#define BIT_RMV_1PSR                          	BIT(25)
#define BIT_RMV_ROM                           	BIT(24)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_MEM_CTRL                           (Offset 0x00D8)


#define BIT_SHIFT_BT_ROM                      	24
#define BIT_MASK_BT_ROM                       	0xf
#define BIT_BT_ROM(x)                         	(((x) & BIT_MASK_BT_ROM) << BIT_SHIFT_BT_ROM)


#define BIT_SHIFT_PCI_DPRAM                   	10
#define BIT_MASK_PCI_DPRAM                    	0x3
#define BIT_PCI_DPRAM(x)                      	(((x) & BIT_MASK_PCI_DPRAM) << BIT_SHIFT_PCI_DPRAM)


#endif


#if (RTL8192E_SUPPORT)


//2 REG_MEM_CTRL                           (Offset 0x00D8)


#define BIT_SHIFT_MEM_RME_BT                  	8
#define BIT_MASK_MEM_RME_BT                   	0xf
#define BIT_MEM_RME_BT(x)                     	(((x) & BIT_MASK_MEM_RME_BT) << BIT_SHIFT_MEM_RME_BT)


#endif


#if (RTL8881A_SUPPORT)


//2 REG_MEM_CTRL                           (Offset 0x00D8)


#define BIT_SHIFT_PCI_SPRAM                   	8
#define BIT_MASK_PCI_SPRAM                    	0x3
#define BIT_PCI_SPRAM(x)                      	(((x) & BIT_MASK_PCI_SPRAM) << BIT_SHIFT_PCI_SPRAM)


#define BIT_SHIFT_USB_SPRAM                   	6
#define BIT_MASK_USB_SPRAM                    	0x3
#define BIT_USB_SPRAM(x)                      	(((x) & BIT_MASK_USB_SPRAM) << BIT_SHIFT_USB_SPRAM)


#endif


#if (RTL8192E_SUPPORT)


//2 REG_MEM_CTRL                           (Offset 0x00D8)


#define BIT_SHIFT_MEM_RME_WL                  	4
#define BIT_MASK_MEM_RME_WL                   	0xf
#define BIT_MEM_RME_WL(x)                     	(((x) & BIT_MASK_MEM_RME_WL) << BIT_SHIFT_MEM_RME_WL)


#endif


#if (RTL8881A_SUPPORT)


//2 REG_MEM_CTRL                           (Offset 0x00D8)


#define BIT_SHIFT_USB_SPRF                    	4
#define BIT_MASK_USB_SPRF                     	0x3
#define BIT_USB_SPRF(x)                       	(((x) & BIT_MASK_USB_SPRF) << BIT_SHIFT_USB_SPRF)


#endif


#if (RTL8192E_SUPPORT)


//2 REG_MEM_CTRL                           (Offset 0x00D8)


#define BIT_SHIFT_MEM_RME_HCI                 	0
#define BIT_MASK_MEM_RME_HCI                  	0xf
#define BIT_MEM_RME_HCI(x)                    	(((x) & BIT_MASK_MEM_RME_HCI) << BIT_SHIFT_MEM_RME_HCI)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_MEM_CTRL                           (Offset 0x00D8)


#define BIT_SHIFT_BIST_ROM                    	0
#define BIT_MASK_BIST_ROM                     	0xffffffffL
#define BIT_BIST_ROM(x)                       	(((x) & BIT_MASK_BIST_ROM) << BIT_SHIFT_BIST_ROM)


#endif


#if (RTL8881A_SUPPORT)


//2 REG_MEM_CTRL                           (Offset 0x00D8)


#define BIT_SHIFT_MCU_ROM                     	0
#define BIT_MASK_MCU_ROM                      	0xf
#define BIT_MCU_ROM(x)                        	(((x) & BIT_MASK_MCU_ROM) << BIT_SHIFT_MCU_ROM)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_WLAN_DBG                           (Offset 0x00DC)


#define BIT_SHIFT_WLAN_DBG                    	0
#define BIT_MASK_WLAN_DBG                     	0xffffffffL
#define BIT_WLAN_DBG(x)                       	(((x) & BIT_MASK_WLAN_DBG) << BIT_SHIFT_WLAN_DBG)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_USB_SIE_INTF                       (Offset 0x00E0)

#define BIT_RD_SEL                            	BIT(31)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_USB_SIE_INTF                       (Offset 0x00E0)

#define BIT_CPU_REG_SEL                       	BIT(31)
#define BIT_USB3_REG_SEL                      	BIT(30)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_USB_SIE_INTF                       (Offset 0x00E0)

#define BIT_USB_SIE_INTF_WE                   	BIT(25)
#define BIT_USB_SIE_INTF_BYIOREG              	BIT(24)

#define BIT_SHIFT_USB_SIE_INTF_ADDR           	16
#define BIT_MASK_USB_SIE_INTF_ADDR            	0xff
#define BIT_USB_SIE_INTF_ADDR(x)              	(((x) & BIT_MASK_USB_SIE_INTF_ADDR) << BIT_SHIFT_USB_SIE_INTF_ADDR)


#define BIT_SHIFT_USB_SIE_INTF_RD             	8
#define BIT_MASK_USB_SIE_INTF_RD              	0xff
#define BIT_USB_SIE_INTF_RD(x)                	(((x) & BIT_MASK_USB_SIE_INTF_RD) << BIT_SHIFT_USB_SIE_INTF_RD)


#define BIT_SHIFT_USB_SIE_INTF_WD             	0
#define BIT_MASK_USB_SIE_INTF_WD              	0xff
#define BIT_USB_SIE_INTF_WD(x)                	(((x) & BIT_MASK_USB_SIE_INTF_WD) << BIT_SHIFT_USB_SIE_INTF_WD)


//2 REG_PCIE_MIO_INTF                      (Offset 0x00E4)

#define BIT_PCIE_MIO_BYIOREG                  	BIT(13)
#define BIT_PCIE_MIO_RE                       	BIT(12)

#define BIT_SHIFT_PCIE_MIO_WE                 	8
#define BIT_MASK_PCIE_MIO_WE                  	0xf
#define BIT_PCIE_MIO_WE(x)                    	(((x) & BIT_MASK_PCIE_MIO_WE) << BIT_SHIFT_PCIE_MIO_WE)


#define BIT_SHIFT_PCIE_MIO_ADDR               	0
#define BIT_MASK_PCIE_MIO_ADDR                	0xff
#define BIT_PCIE_MIO_ADDR(x)                  	(((x) & BIT_MASK_PCIE_MIO_ADDR) << BIT_SHIFT_PCIE_MIO_ADDR)


//2 REG_PCIE_MIO_INTD                      (Offset 0x00E8)


#define BIT_SHIFT_PCIE_MIO_DATA               	0
#define BIT_MASK_PCIE_MIO_DATA                	0xffffffffL
#define BIT_PCIE_MIO_DATA(x)                  	(((x) & BIT_MASK_PCIE_MIO_DATA) << BIT_SHIFT_PCIE_MIO_DATA)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_HPON_FSM                           (Offset 0x00EC)

#define BIT_SUSPEND_V1                        	BIT(31)
#define BIT_FSM_RESUME_V1                     	BIT(30)
#define BIT_HOST_RESUME_SYNC_V1               	BIT(29)
#define BIT_CHIP_PDNB_V1                      	BIT(28)

#define BIT_SHIFT_FSM_SUSPEND_V1              	25
#define BIT_MASK_FSM_SUSPEND_V1               	0x7
#define BIT_FSM_SUSPEND_V1(x)                 	(((x) & BIT_MASK_FSM_SUSPEND_V1) << BIT_SHIFT_FSM_SUSPEND_V1)

#define BIT_PMC_ALD_V1                        	BIT(24)

#define BIT_SHIFT_HCI_SEL_1                   	22
#define BIT_MASK_HCI_SEL_1                    	0x3
#define BIT_HCI_SEL_1(x)                      	(((x) & BIT_MASK_HCI_SEL_1) << BIT_SHIFT_HCI_SEL_1)

#define BIT_LOAD_DONE_V1                      	BIT(21)
#define BIT_CNT_MATCH                         	BIT(20)
#define BIT_TIMEUP_V1                         	BIT(19)
#define BIT_SPS_12V_VLD                       	BIT(18)
#define BIT_PCIERST_V1                        	BIT(17)
#define BIT_HOST_CLK_VLd                      	BIT(16)
#define BIT_PMC_WR_V1                         	BIT(15)
#define BIT_PMC_DATA_V1                       	BIT(14)

#define BIT_SHIFT_PMC_ADDR_V1                 	8
#define BIT_MASK_PMC_ADDR_V1                  	0x3f
#define BIT_PMC_ADDR_V1(x)                    	(((x) & BIT_MASK_PMC_ADDR_V1) << BIT_SHIFT_PMC_ADDR_V1)

#define BIT_PMC_COUNT_EN_V1                   	BIT(7)

#define BIT_SHIFT_FSM_STATE_V1                	0
#define BIT_MASK_FSM_STATE_V1                 	0x7f
#define BIT_FSM_STATE_V1(x)                   	(((x) & BIT_MASK_FSM_STATE_V1) << BIT_SHIFT_FSM_STATE_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)


#define BIT_SHIFT_TRP_ICFG                    	28
#define BIT_MASK_TRP_ICFG                     	0xf
#define BIT_TRP_ICFG(x)                       	(((x) & BIT_MASK_TRP_ICFG) << BIT_SHIFT_TRP_ICFG)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_RF_TYPE_ID                        	BIT(27)
#define BIT_BD_HCI_SEL                        	BIT(26)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)


#define BIT_SHIFT_BD_HCI_SEL                  	26
#define BIT_MASK_BD_HCI_SEL                   	0x3
#define BIT_BD_HCI_SEL(x)                     	(((x) & BIT_MASK_BD_HCI_SEL) << BIT_SHIFT_BD_HCI_SEL)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_BD_PKG_SEL                        	BIT(25)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_SPSLDO_SEL                        	BIT(24)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_LDO_SPS_SEL                       	BIT(24)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_RTL_ID                            	BIT(23)
#define BIT_PAD_HWPD_IDN                      	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_Testmode                          	BIT(20)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)


#define BIT_SHIFT_PSC_TESTCFG                 	20
#define BIT_MASK_PSC_TESTCFG                  	0x3
#define BIT_PSC_TESTCFG(x)                    	(((x) & BIT_MASK_PSC_TESTCFG) << BIT_SHIFT_PSC_TESTCFG)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)


#define BIT_SHIFT_VENDOR_ID                   	16
#define BIT_MASK_VENDOR_ID                    	0xf
#define BIT_VENDOR_ID(x)                      	(((x) & BIT_MASK_VENDOR_ID) << BIT_SHIFT_VENDOR_ID)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)


#define BIT_SHIFT_CHIP_VER_V2                 	16
#define BIT_MASK_CHIP_VER_V2                  	0xf
#define BIT_CHIP_VER_V2(x)                    	(((x) & BIT_MASK_CHIP_VER_V2) << BIT_SHIFT_CHIP_VER_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT || RTL8814A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)


#define BIT_SHIFT_CHIP_VER                    	12
#define BIT_MASK_CHIP_VER                     	0xf
#define BIT_CHIP_VER(x)                       	(((x) & BIT_MASK_CHIP_VER) << BIT_SHIFT_CHIP_VER)
#define GET_BIT_CHIP_VER(x)                     (((x)>>BIT_SHIFT_CHIP_VER) & BIT_MASK_CHIP_VER)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)


#define BIT_IC_MACPHY_MODE                    	BIT(11)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_BD_MAC1                           	BIT(10)
#define BIT_BD_MAC2                           	BIT(9)
#define BIT_SIC_IDLE                          	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_Sw_offload_en                     	BIT(7)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_SW_OFFLOAD_EN                     	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_OCP_SHUTDN                        	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_OCP_SHUTDN_1                      	BIT(6)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_V15_VLD                           	BIT(5)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_V12_VLD                           	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_PCIRSTB                           	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_PCLK_VLD                          	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_PCLK_VLD_1                        	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_UCLK_VLD                          	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_ACLK_VLD                          	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_M200CLK_VLD_V1                    	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG1                           (Offset 0x00F0)

#define BIT_XCLK_VLD                          	BIT(0)

//2 REG_SYS_STATUS1                        (Offset 0x00F4)


#define BIT_SHIFT_RF_RL_ID                    	28
#define BIT_MASK_RF_RL_ID                     	0xf
#define BIT_RF_RL_ID(x)                       	(((x) & BIT_MASK_RF_RL_ID) << BIT_SHIFT_RF_RL_ID)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)

#define BIT_U3_CLK_VLD                        	BIT(27)
#define BIT_PRST_VLD_V1                       	BIT(26)
#define BIT_PDN                               	BIT(25)
#define BIT_OCP_SHUTDN_V1                     	BIT(24)
#define BIT_PCLK_VLD_V1                       	BIT(23)
#define BIT_U2_CLK_VLD                        	BIT(22)
#define BIT_PLL_CLK_VLD                       	BIT(21)
#define BIT_XCK_VLd                           	BIT(20)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)

#define BIT_HPHY_ICFG                         	BIT(19)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)

#define BIT_CK200M_VLd                        	BIT(19)
#define BIT_BTEN_TRAP                         	BIT(18)
#define BIT_PKG_EN_V1                         	BIT(17)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)


#define BIT_SHIFT_SEL_0xC0                    	16
#define BIT_MASK_SEL_0xC0                     	0x3
#define BIT_SEL_0xC0(x)                       	(((x) & BIT_MASK_SEL_0xC0) << BIT_SHIFT_SEL_0xC0)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)

#define BIT_TRAP_LDO_SPS_V1                   	BIT(16)
#define BIT_MACRDY                            	BIT(15)
#define BIT_12V_VLd                           	BIT(14)
#define BIT_U3PHY_RST                         	BIT(13)
#define BIT_USB2_SEL_V1                       	BIT(12)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)

#define BIT_USB_OPERATION_MODE                	BIT(10)
#define BIT_BT_PDN                            	BIT(9)
#define BIT_AUTO_WLPON                        	BIT(8)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)


#define BIT_SHIFT_TRAP_ICFG                   	8
#define BIT_MASK_TRAP_ICFG                    	0xf
#define BIT_TRAP_ICFG(x)                      	(((x) & BIT_MASK_TRAP_ICFG) << BIT_SHIFT_TRAP_ICFG)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)

#define BIT_WL_MODE                           	BIT(7)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)

#define BIT_WLAN_ID                           	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)

#define BIT_PKG_SEL_HCI                       	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)

#define BIT_ALDN                              	BIT(6)
#define BIT_BTCOEX_CMDEN                      	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)


#define BIT_SHIFT_HCI_SEL                     	4
#define BIT_MASK_HCI_SEL                      	0x3
#define BIT_HCI_SEL(x)                        	(((x) & BIT_MASK_HCI_SEL) << BIT_SHIFT_HCI_SEL)
#define GET_BIT_HCI_SEL(x)                      (((x)>>BIT_SHIFT_HCI_SEL) & BIT_MASK_HCI_SEL)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)

#define BIT_BT_EN                             	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)


#define BIT_SHIFT_PAD_HCI_SEL                 	2
#define BIT_MASK_PAD_HCI_SEL                  	0x3
#define BIT_PAD_HCI_SEL(x)                    	(((x) & BIT_MASK_PAD_HCI_SEL) << BIT_SHIFT_PAD_HCI_SEL)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)


#define BIT_SHIFT_HCI_SEL_V2                  	2
#define BIT_MASK_HCI_SEL_V2                   	0x3
#define BIT_HCI_SEL_V2(x)                     	(((x) & BIT_MASK_HCI_SEL_V2) << BIT_SHIFT_HCI_SEL_V2)
#define GET_BIT_HCI_SEL_V2(x)                   (((x)>>BIT_SHIFT_HCI_SEL_V2) & BIT_MASK_HCI_SEL_V2)


#define BIT_TST_MOD_SEL                       	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)


#define BIT_SHIFT_EFS_HCI_SEL                 	0
#define BIT_MASK_EFS_HCI_SEL                  	0x3
#define BIT_EFS_HCI_SEL(x)                    	(((x) & BIT_MASK_EFS_HCI_SEL) << BIT_SHIFT_EFS_HCI_SEL)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_STATUS1                        (Offset 0x00F4)

#define BIT_PAD_HWPDB                         	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_STATUS2                        (Offset 0x00F8)

#define BIT_SIO_ALDN                          	BIT(19)
#define BIT_USB_ALDN                          	BIT(18)
#define BIT_PCI_ALDN                          	BIT(17)
#define BIT_SYS_ALDN                          	BIT(16)

#define BIT_SHIFT_EPVID1                      	8
#define BIT_MASK_EPVID1                       	0xff
#define BIT_EPVID1(x)                         	(((x) & BIT_MASK_EPVID1) << BIT_SHIFT_EPVID1)


#define BIT_SHIFT_EPVID0                      	0
#define BIT_MASK_EPVID0                       	0xff
#define BIT_EPVID0(x)                         	(((x) & BIT_MASK_EPVID0) << BIT_SHIFT_EPVID0)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CFG2                           (Offset 0x00FC)

#define BIT_USB2_SEL_1                        	BIT(31)
#define BIT_USB3PHY_RST                       	BIT(30)
#define BIT_U3_TERM_DET                       	BIT(29)
#define BIT_USB23_DBG_SEL                     	BIT(24)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SYS_CFG2                           (Offset 0x00FC)


#define BIT_SHIFT_HW_ID                       	0
#define BIT_MASK_HW_ID                        	0xff
#define BIT_HW_ID(x)                          	(((x) & BIT_MASK_HW_ID) << BIT_SHIFT_HW_ID)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SYS_CFG2                           (Offset 0x00FC)


#define BIT_SHIFT_CHIPID                      	0
#define BIT_MASK_CHIPID                       	0xff
#define BIT_CHIPID(x)                         	(((x) & BIT_MASK_CHIPID) << BIT_SHIFT_CHIPID)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_CR                                 (Offset 0x0100)


#define BIT_SHIFT_LBMODE                      	24
#define BIT_MASK_LBMODE                       	0x1f
#define BIT_LBMODE(x)                         	(((x) & BIT_MASK_LBMODE) << BIT_SHIFT_LBMODE)


#define BIT_SHIFT_NETYPE1                     	18
#define BIT_MASK_NETYPE1                      	0x3
#define BIT_NETYPE1(x)                        	(((x) & BIT_MASK_NETYPE1) << BIT_SHIFT_NETYPE1)


#define BIT_SHIFT_NETYPE0                     	16
#define BIT_MASK_NETYPE0                      	0x3
#define BIT_NETYPE0(x)                        	(((x) & BIT_MASK_NETYPE0) << BIT_SHIFT_NETYPE0)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_CR                                 (Offset 0x0100)

#define BIT_PTA_I2C_MBOX_EN                   	BIT(12)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_CR                                 (Offset 0x0100)

#define BIT_I2C_MAILBOX_EN                    	BIT(12)
#define BIT_SHCUT_EN                          	BIT(11)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_CR                                 (Offset 0x0100)

#define BIT_32K_CAL_TMR_EN                    	BIT(10)
#define BIT_MAC_SEC_EN                        	BIT(9)
#define BIT_ENSWBCN                           	BIT(8)
#define BIT_MACRXEN                           	BIT(7)
#define BIT_MACTXEN                           	BIT(6)
#define BIT_SCHEDULE_EN                       	BIT(5)
#define BIT_PROTOCOL_EN                       	BIT(4)
#define BIT_RXDMA_EN                          	BIT(3)
#define BIT_TXDMA_EN                          	BIT(2)
#define BIT_HCI_RXDMA_EN                      	BIT(1)
#define BIT_HCI_TXDMA_EN                      	BIT(0)

//2 REG_PBP                                (Offset 0x0104)


#define BIT_SHIFT_PSTX                        	4
#define BIT_MASK_PSTX                         	0xf
#define BIT_PSTX(x)                           	(((x) & BIT_MASK_PSTX) << BIT_SHIFT_PSTX)


#define BIT_SHIFT_PSRX                        	0
#define BIT_MASK_PSRX                         	0xf
#define BIT_PSRX(x)                           	(((x) & BIT_MASK_PSRX) << BIT_SHIFT_PSRX)


//2 REG_PKT_BUFF_ACCESS_CTRL               (Offset 0x0106)


#define BIT_SHIFT_PKT_BUFF_ACCESS_CTRL        	0
#define BIT_MASK_PKT_BUFF_ACCESS_CTRL         	0xff
#define BIT_PKT_BUFF_ACCESS_CTRL(x)           	(((x) & BIT_MASK_PKT_BUFF_ACCESS_CTRL) << BIT_SHIFT_PKT_BUFF_ACCESS_CTRL)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TSF_CLK_STATE                      (Offset 0x0108)

#define BIT_TSF_CLK_IDX                       	BIT(15)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_TSF_CLK_STATE                      (Offset 0x0108)

#define BIT_TSF_CLK_STABLE                    	BIT(15)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXDMA_PQ_MAP                       (Offset 0x010C)


#define BIT_SHIFT_TXDMA_CMQ_MAP               	16
#define BIT_MASK_TXDMA_CMQ_MAP                	0x3
#define BIT_TXDMA_CMQ_MAP(x)                  	(((x) & BIT_MASK_TXDMA_CMQ_MAP) << BIT_SHIFT_TXDMA_CMQ_MAP)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXDMA_PQ_MAP                       (Offset 0x010C)


#define BIT_SHIFT_TXDMA_HIQ_MAP               	14
#define BIT_MASK_TXDMA_HIQ_MAP                	0x3
#define BIT_TXDMA_HIQ_MAP(x)                  	(((x) & BIT_MASK_TXDMA_HIQ_MAP) << BIT_SHIFT_TXDMA_HIQ_MAP)


#define BIT_SHIFT_TXDMA_MGQ_MAP               	12
#define BIT_MASK_TXDMA_MGQ_MAP                	0x3
#define BIT_TXDMA_MGQ_MAP(x)                  	(((x) & BIT_MASK_TXDMA_MGQ_MAP) << BIT_SHIFT_TXDMA_MGQ_MAP)


#define BIT_SHIFT_TXDMA_BKQ_MAP               	10
#define BIT_MASK_TXDMA_BKQ_MAP                	0x3
#define BIT_TXDMA_BKQ_MAP(x)                  	(((x) & BIT_MASK_TXDMA_BKQ_MAP) << BIT_SHIFT_TXDMA_BKQ_MAP)


#define BIT_SHIFT_TXDMA_BEQ_MAP               	8
#define BIT_MASK_TXDMA_BEQ_MAP                	0x3
#define BIT_TXDMA_BEQ_MAP(x)                  	(((x) & BIT_MASK_TXDMA_BEQ_MAP) << BIT_SHIFT_TXDMA_BEQ_MAP)


#define BIT_SHIFT_TXDMA_VIQ_MAP               	6
#define BIT_MASK_TXDMA_VIQ_MAP                	0x3
#define BIT_TXDMA_VIQ_MAP(x)                  	(((x) & BIT_MASK_TXDMA_VIQ_MAP) << BIT_SHIFT_TXDMA_VIQ_MAP)


#define BIT_SHIFT_TXDMA_VOQ_MAP               	4
#define BIT_MASK_TXDMA_VOQ_MAP                	0x3
#define BIT_TXDMA_VOQ_MAP(x)                  	(((x) & BIT_MASK_TXDMA_VOQ_MAP) << BIT_SHIFT_TXDMA_VOQ_MAP)

#define BIT_RXDMA_AGG_EN                      	BIT(2)
#define BIT_RXSHFT_EN                         	BIT(1)
#define BIT_RXDMA_ARBBW_EN                    	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_TRXFF_BNDY                         (Offset 0x0114)


#define BIT_SHIFT_RXFFOVFL_RSV_V1             	28
#define BIT_MASK_RXFFOVFL_RSV_V1              	0xf
#define BIT_RXFFOVFL_RSV_V1(x)                	(((x) & BIT_MASK_RXFFOVFL_RSV_V1) << BIT_SHIFT_RXFFOVFL_RSV_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TRXFF_BNDY                         (Offset 0x0114)


#define BIT_SHIFT_RXFF0_BNDY                  	16
#define BIT_MASK_RXFF0_BNDY                   	0xffff
#define BIT_RXFF0_BNDY(x)                     	(((x) & BIT_MASK_RXFF0_BNDY) << BIT_SHIFT_RXFF0_BNDY)


#define BIT_SHIFT_RXFFOVFL_RSV                	8
#define BIT_MASK_RXFFOVFL_RSV                 	0xf
#define BIT_RXFFOVFL_RSV(x)                   	(((x) & BIT_MASK_RXFFOVFL_RSV) << BIT_SHIFT_RXFFOVFL_RSV)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TRXFF_BNDY                         (Offset 0x0114)


#define BIT_SHIFT_RXFF0_BNDY_V1               	8
#define BIT_MASK_RXFF0_BNDY_V1                	0x3ffff
#define BIT_RXFF0_BNDY_V1(x)                  	(((x) & BIT_MASK_RXFF0_BNDY_V1) << BIT_SHIFT_RXFF0_BNDY_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TRXFF_BNDY                         (Offset 0x0114)


#define BIT_SHIFT_TXPKTBUF_PGBNDY             	0
#define BIT_MASK_TXPKTBUF_PGBNDY              	0xff
#define BIT_TXPKTBUF_PGBNDY(x)                	(((x) & BIT_MASK_TXPKTBUF_PGBNDY) << BIT_SHIFT_TXPKTBUF_PGBNDY)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_FF_STATUS                          (Offset 0x0118)


#define BIT_SHIFT_RXFF0_RDPTR_V1              	13
#define BIT_MASK_RXFF0_RDPTR_V1               	0x3ffff
#define BIT_RXFF0_RDPTR_V1(x)                 	(((x) & BIT_MASK_RXFF0_RDPTR_V1) << BIT_SHIFT_RXFF0_RDPTR_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PTA_I2C_MBOX                       (Offset 0x0118)


#define BIT_SHIFT_I2C_M_STATUS                	8
#define BIT_MASK_I2C_M_STATUS                 	0xf
#define BIT_I2C_M_STATUS(x)                   	(((x) & BIT_MASK_I2C_M_STATUS) << BIT_SHIFT_I2C_M_STATUS)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PTA_I2C_MBOX                       (Offset 0x0118)


#define BIT_SHIFT_I2C_M_BUS_GNT               	4
#define BIT_MASK_I2C_M_BUS_GNT                	0x7
#define BIT_I2C_M_BUS_GNT(x)                  	(((x) & BIT_MASK_I2C_M_BUS_GNT) << BIT_SHIFT_I2C_M_BUS_GNT)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_FF_STATUS                          (Offset 0x0118)


#define BIT_SHIFT_I2C_M_BUS_GNT_FW            	4
#define BIT_MASK_I2C_M_BUS_GNT_FW             	0x7
#define BIT_I2C_M_BUS_GNT_FW(x)               	(((x) & BIT_MASK_I2C_M_BUS_GNT_FW) << BIT_SHIFT_I2C_M_BUS_GNT_FW)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PTA_I2C_MBOX                       (Offset 0x0118)

#define BIT_I2C_GNT_FW                        	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FF_STATUS                          (Offset 0x0118)

#define BIT_I2C_M_GNT_FW                      	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PTA_I2C_MBOX                       (Offset 0x0118)


#define BIT_SHIFT_I2C_DATA_RATE               	1
#define BIT_MASK_I2C_DATA_RATE                	0x3
#define BIT_I2C_DATA_RATE(x)                  	(((x) & BIT_MASK_I2C_DATA_RATE) << BIT_SHIFT_I2C_DATA_RATE)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_FF_STATUS                          (Offset 0x0118)


#define BIT_SHIFT_I2C_M_SPEED                 	1
#define BIT_MASK_I2C_M_SPEED                  	0x3
#define BIT_I2C_M_SPEED(x)                    	(((x) & BIT_MASK_I2C_M_SPEED) << BIT_SHIFT_I2C_M_SPEED)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PTA_I2C_MBOX                       (Offset 0x0118)

#define BIT_I2C_SW_control_unlock             	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FF_STATUS                          (Offset 0x0118)

#define BIT_I2C_M_UNLOCK                      	BIT(0)

#define BIT_SHIFT_RXFF0_WTPTR_V1              	0
#define BIT_MASK_RXFF0_WTPTR_V1               	0x3ffff
#define BIT_RXFF0_WTPTR_V1(x)                 	(((x) & BIT_MASK_RXFF0_WTPTR_V1) << BIT_SHIFT_RXFF0_WTPTR_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RXFF_PTR                           (Offset 0x011C)


#define BIT_SHIFT_RXFF0_RDPTR                 	16
#define BIT_MASK_RXFF0_RDPTR                  	0xffff
#define BIT_RXFF0_RDPTR(x)                    	(((x) & BIT_MASK_RXFF0_RDPTR) << BIT_SHIFT_RXFF0_RDPTR)


#define BIT_SHIFT_RXFF0_WTPTR                 	0
#define BIT_MASK_RXFF0_WTPTR                  	0xffff
#define BIT_RXFF0_WTPTR(x)                    	(((x) & BIT_MASK_RXFF0_WTPTR) << BIT_SHIFT_RXFF0_WTPTR)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_FE1IMR                             (Offset 0x0120)

#define BIT_FS_RXDMA2_DONE_INT_EN             	BIT(28)
#define BIT_FS_RXDONE3_INT_EN                 	BIT(27)
#define BIT_FS_RXDONE2_INT_EN                 	BIT(26)
#define BIT_FS_RX_BCN_P4_INT_EN               	BIT(25)
#define BIT_FS_RX_BCN_P3_INT_EN               	BIT(24)
#define BIT_FS_RX_BCN_P2_INT_EN               	BIT(23)
#define BIT_FS_RX_BCN_P1_INT_EN               	BIT(22)
#define BIT_FS_RX_BCN_P0_INT_EN               	BIT(21)
#define BIT_FS_RX_UMD0_INT_EN                 	BIT(20)
#define BIT_FS_RX_UMD1_INT_EN                 	BIT(19)
#define BIT_FS_RX_BMD0_INT_EN                 	BIT(18)
#define BIT_FS_RX_BMD1_INT_EN                 	BIT(17)
#define BIT_FS_RXDONE_INT_EN                  	BIT(16)
#define BIT_FS_WWLAN_INT_EN                   	BIT(15)
#define BIT_FS_SOUND_DONE_INT_EN              	BIT(14)
#define BIT_FS_LP_STBY_INT_EN                 	BIT(13)
#define BIT_FS_TRL_MTR_INT_EN                 	BIT(12)
#define BIT_FS_BF1_PRETO_INT_EN               	BIT(11)
#define BIT_FS_BF0_PRETO_INT_EN               	BIT(10)
#define BIT_FS_PTCL_RELEASE_MACID_INT_EN      	BIT(9)
#define BIT_FS_WLACTOFF_INT_EN                	BIT(5)
#define BIT_FS_WLACTON_INT_EN                 	BIT(4)
#define BIT_FS_BTCMD_INT_EN                   	BIT(3)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_FEIMR                              (Offset 0x0120)

#define BIT_REG_Mailbox_TO_I2C_INT            	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FE1IMR                             (Offset 0x0120)

#define BIT_FS_REG_Mailbox_TO_I2C_INT_EN      	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FEIMR                              (Offset 0x0120)

#define BIT_TRPC_TO_INT_En                    	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FE1IMR                             (Offset 0x0120)

#define BIT_FS_TRPC_TO_INT_EN                 	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FEIMR                              (Offset 0x0120)

#define BIT_BIT_RPC_O_T_INT_En                	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FE1IMR                             (Offset 0x0120)

#define BIT_FS_RPC_O_T_INT_EN                 	BIT(0)

//2 REG_FE1ISR                             (Offset 0x0124)

#define BIT_FS_RXDMA2_DONE_INT                	BIT(28)
#define BIT_FS_RXDONE3_INT                    	BIT(27)
#define BIT_FS_RXDONE2_INT                    	BIT(26)
#define BIT_FS_RX_BCN_P4_INT                  	BIT(25)
#define BIT_FS_RX_BCN_P3_INT                  	BIT(24)
#define BIT_FS_RX_BCN_P2_INT                  	BIT(23)
#define BIT_FS_RX_BCN_P1_INT                  	BIT(22)
#define BIT_FS_RX_BCN_P0_INT                  	BIT(21)
#define BIT_FS_RX_UMD0_INT                    	BIT(20)
#define BIT_FS_RX_UMD1_INT                    	BIT(19)
#define BIT_FS_RX_BMD0_INT                    	BIT(18)
#define BIT_FS_RX_BMD1_INT                    	BIT(17)
#define BIT_FS_RXDONE_INT                     	BIT(16)
#define BIT_FS_WWLAN_INT                      	BIT(15)
#define BIT_FS_SOUND_DONE_INT                 	BIT(14)
#define BIT_FS_LP_STBY_INT                    	BIT(13)
#define BIT_FS_TRL_MTR_INT                    	BIT(12)
#define BIT_FS_BF1_PRETO_INT                  	BIT(11)
#define BIT_FS_BF0_PRETO_INT                  	BIT(10)
#define BIT_FS_PTCL_RELEASE_MACID_INT         	BIT(9)
#define BIT_FS_WLACTOFF_INT                   	BIT(5)
#define BIT_FS_WLACTON_INT                    	BIT(4)
#define BIT_FS_BCN_RX_INT_INT                 	BIT(3)

#endif


#if (RTL8192E_SUPPORT)


//2 REG_FEISR                              (Offset 0x0124)

#define BIT_Mailbox_TO_I2C                    	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FE1ISR                             (Offset 0x0124)

#define BIT_FS_Mailbox_TO_I2C_INT             	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FEISR                              (Offset 0x0124)

#define BIT_TRPC_TO_INT                       	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FE1ISR                             (Offset 0x0124)

#define BIT_FS_TRPC_TO_INT                    	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FEISR                              (Offset 0x0124)

#define BIT_RPC_O_T_INT                       	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FE1ISR                             (Offset 0x0124)

#define BIT_FS_RPC_O_T_INT                    	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_CPWM                               (Offset 0x012C)

#define BIT_CPWM_TOGGLING                     	BIT(31)

#define BIT_SHIFT_CPWM_MOD                    	24
#define BIT_MASK_CPWM_MOD                     	0x7f
#define BIT_CPWM_MOD(x)                       	(((x) & BIT_MASK_CPWM_MOD) << BIT_SHIFT_CPWM_MOD)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNOK_MB7_INT_EN             	BIT(31)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_SOUND_DONE_MSK                    	BIT(30)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNOK_MB6_INT_EN             	BIT(30)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_TRY_DONE_MSK                      	BIT(29)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNOK_MB5_INT_EN             	BIT(29)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_TXRPT_CNT_FULL_MSK                	BIT(28)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNOK_MB4_INT_EN             	BIT(28)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_WLACTOFF_INT_EN                   	BIT(27)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNOK_MB3_INT_EN             	BIT(27)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_WLACTON_INT_EN                    	BIT(26)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNOK_MB2_INT_EN             	BIT(26)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_TXPKTIN_INT_EN                    	BIT(25)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNOK_MB1_INT_EN             	BIT(25)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_TXBCNOK_MSK                       	BIT(24)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNOK_MB0_INT_EN             	BIT(24)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_TXBCNERR_MSK                      	BIT(23)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNERR_MB7_INT_EN            	BIT(23)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_RX_UMD0_EN                        	BIT(22)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNERR_MB6_INT_EN            	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_RX_UMD1_EN                        	BIT(21)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNERR_MB5_INT_EN            	BIT(21)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_RX_BMD0_EN                        	BIT(20)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNERR_MB4_INT_EN            	BIT(20)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_RX_BMD1_EN                        	BIT(19)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNERR_MB3_INT_EN            	BIT(19)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_BCN_RX_INT_EN                     	BIT(18)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNERR_MB2_INT_EN            	BIT(18)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_TBTTINT_MSK                       	BIT(17)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNERR_MB1_INT_EN            	BIT(17)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_BCNERLY_MSK                       	BIT(16)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXBCNERR_MB0_INT_EN            	BIT(16)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_BCNDMA7_MSK                       	BIT(15)
#define BIT_BCNDMA6_MSK                       	BIT(14)
#define BIT_BCNDMA5_MSK                       	BIT(13)
#define BIT_BCNDMA4_MSK                       	BIT(12)
#define BIT_BCNDMA3_MSK                       	BIT(11)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_DDMA1_LP_INT_EN                	BIT(11)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_BCNDMA2_MSK                       	BIT(10)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_DDMA1_HP_INT_EN                	BIT(10)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_BCNDMA1_MSK                       	BIT(9)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_DDMA0_LP_INT_EN                	BIT(9)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_BCNDMA0_MSK                       	BIT(8)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_DDMA0_HP_INT_EN                	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_LP_STBY_MSK                       	BIT(7)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TRXRPT_INT_EN                  	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_CTWEndINT_MSK                     	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_C2H_W_READY_INT_EN             	BIT(6)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_HRCV_MSK                          	BIT(5)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_HRCV_INT_EN                    	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_H2CCMD_MSK                        	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_H2CCMD_INT_EN                  	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_RXDONE_MSK                        	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXPKTIN_INT_EN                 	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_ERRORHDL_MSK                      	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_ERRORHDL_INT_EN                	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_TXCCX_MSK_FW                      	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXCCX_INT_EN                   	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_TXCLOSE_MSK                       	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWIMR                              (Offset 0x0130)

#define BIT_FS_TXCLOSE_INT_EN                 	BIT(0)

//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNOK_MB7_INT                	BIT(31)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_SOUND_DONE_INT                    	BIT(30)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNOK_MB6_INT                	BIT(30)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_TRY_DONE_INT                      	BIT(29)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNOK_MB5_INT                	BIT(29)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_TXRPT_CNT_FULL_INT                	BIT(28)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNOK_MB4_INT                	BIT(28)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_WLACTOFF_INT                      	BIT(27)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNOK_MB3_INT                	BIT(27)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_WLACTON_INT                       	BIT(26)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNOK_MB2_INT                	BIT(26)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_TXPKTIN_INT                       	BIT(25)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNOK_MB1_INT                	BIT(25)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_TXBCNOK_INT                       	BIT(24)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNOK_MB0_INT                	BIT(24)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_TXBCNERR_INT                      	BIT(23)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNERR_MB7_INT               	BIT(23)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_RX_UMD0_INT                       	BIT(22)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNERR_MB6_INT               	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_RX_UMD1_INT                       	BIT(21)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNERR_MB5_INT               	BIT(21)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_RX_BMD0_INT                       	BIT(20)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNERR_MB4_INT               	BIT(20)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_RX_BMD1_INT                       	BIT(19)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNERR_MB3_INT               	BIT(19)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_BCN_RX_INT_INT                    	BIT(18)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNERR_MB2_INT               	BIT(18)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_TBTTINT_INT                       	BIT(17)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNERR_MB1_INT               	BIT(17)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_BCNERLY_INT                       	BIT(16)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXBCNERR_MB0_INT               	BIT(16)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_BCNDMA7_INT                       	BIT(15)
#define BIT_BCNDMA6_INT                       	BIT(14)
#define BIT_BCNDMA5_INT                       	BIT(13)
#define BIT_BCNDMA4_INT                       	BIT(12)
#define BIT_BCNDMA3_INT                       	BIT(11)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_DDMA1_LP_INT                   	BIT(11)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_BCNDMA2_INT                       	BIT(10)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_DDMA1_HP_INT                   	BIT(10)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_BCNDMA1_INT                       	BIT(9)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_DDMA0_LP_INT                   	BIT(9)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_BCNDMA0_INT                       	BIT(8)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_DDMA0_HP_INT                   	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_LP_STBY_INT                       	BIT(7)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TRXRPT_INT                     	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_CTWEndINT_INT                     	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_C2H_W_READY_INT                	BIT(6)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_HRCV_INT                          	BIT(5)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_HRCV_INT                       	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_H2CCMD_INT                        	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_H2CCMD_INT                     	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_RXDONE_INT                        	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXPKTIN_INT                    	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_ERRORHDL_INT                      	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_ERRORHDL_INT                   	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_TXCCX_INT                         	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXCCX_INT                      	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_TXCLOSE_INT                       	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWISR                              (Offset 0x0134)

#define BIT_FS_TXCLOSE_INT                    	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_GTINT6_MSK                        	BIT(31)
#define BIT_MTI_BCNIVLEAR__INT__MSK           	BIT(28)
#define BIT_ATIMINT_MSK                       	BIT(27)
#define BIT_WWLAN_INT_EN                      	BIT(26)
#define BIT_C2H_W_READY_EN                    	BIT(25)
#define BIT_TRL_MTR_EN                        	BIT(24)
#define BIT_CLR_PS_STATUS_MSK                 	BIT(23)
#define BIT_RETRIVE_BUFFERED_MSK              	BIT(22)
#define BIT_RPWMINT2_MSK                      	BIT(21)
#define BIT_TSF_BIT32_TOGGLE_MSK_V1           	BIT(20)
#define BIT_TRIGGER_PKT_MSK                   	BIT(19)
#define BIT_FW_BTCMD_INTMSK                   	BIT(18)
#define BIT_P2P_RFOFF_INTMSK                  	BIT(17)
#define BIT_P2P_RFON_INTMSK                   	BIT(16)
#define BIT_TxBCN1err_MSK                     	BIT(15)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_FS_TIMEOUT2_EN                    	BIT(15)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_TxBCN1ok_MSK                      	BIT(14)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_FS_TIMEOUT1_EN                    	BIT(14)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_FT_ATIMend_EMSK                   	BIT(13)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_FS_TIMEOUT0_EN                    	BIT(13)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_BcnDMAInt_EMSK                    	BIT(12)
#define BIT_GTINT5_MSK                        	BIT(11)
#define BIT_EOSP_INT_MSK                      	BIT(10)
#define BIT_RX_BCN_E_MSK                      	BIT(9)
#define BIT_RPWM_INT_EN                       	BIT(8)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_FS_GTINT8_EN                      	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_PSTIMER_MSK                       	BIT(7)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_FS_GTINT7_EN                      	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_TIMEOUT1_MSK                      	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_FS_GTINT6_EN                      	BIT(6)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_TIMEOUT0_MSK                      	BIT(5)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_FS_GTINT5_EN                      	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_FT_GTINT4_MSK                     	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_FS_GTINT4_EN                      	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_FT_GTINT3_MSK                     	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_FS_GTINT3_EN                      	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_GTINT2_MSK                        	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_FS_GTINT2_EN                      	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_GTINT1_MSK                        	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_FS_GTINT1_EN                      	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_GTINT0_MSK                        	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTIMR                              (Offset 0x0138)

#define BIT_FS_GTINT0_EN                      	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_GT6INT                            	BIT(31)
#define BIT_MTI_BCNIVLEAR_INT                 	BIT(28)
#define BIT_ATIM_INT                          	BIT(27)
#define BIT_WWLAN_INT                         	BIT(26)
#define BIT_C2H_W_READY                       	BIT(25)
#define BIT_TRL_MTR_INT                       	BIT(24)
#define BIT_CLR_PS_STATUS                     	BIT(23)
#define BIT_RETRIVE_BUFFERED_INT              	BIT(22)
#define BIT_RPWM2INT                          	BIT(21)
#define BIT_TSF_BIT32_TOGGLE_V1               	BIT(20)
#define BIT_TRIGGER_PKT                       	BIT(19)
#define BIT_FW_BTCMD_INT                      	BIT(18)
#define BIT_P2P_RFOFF_INT                     	BIT(17)
#define BIT_P2P_RFON_INT                      	BIT(16)
#define BIT_Tx_BCN1ERR_INT                    	BIT(15)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_FS_TIMEOUT2_INT                   	BIT(15)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_Tx_BCN1OK_INT                     	BIT(14)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_FS_TIMEOUT1_INT                   	BIT(14)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_FT_ATIMend_E                      	BIT(13)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_FS_TIMEOUT0_INT                   	BIT(13)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_BcnDMAInt_E_V1                    	BIT(12)
#define BIT_GT5INT                            	BIT(11)
#define BIT_EOSP_INT                          	BIT(10)
#define BIT_RX_BCN_E_INT                      	BIT(9)
#define BIT_RPWMINT                           	BIT(8)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_FS_GTINT8_INT                     	BIT(8)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_PSTIMER_INT                       	BIT(7)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_FS_GTINT7_INT                     	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_TIMEOUT1_INT                      	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_FS_GTINT6_INT                     	BIT(6)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_TIMEOUT0_INT                      	BIT(5)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_FS_GTINT5_INT                     	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_FT_GT4INT                         	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_FS_GTINT4_INT                     	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_FT_GT3INT                         	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_FS_GTINT3_INT                     	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_GT2INT                            	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_FS_GTINT2_INT                     	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_GT1INT                            	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_FS_GTINT1_INT                     	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_GT0INT                            	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FTISR                              (Offset 0x013C)

#define BIT_FS_GTINT0_INT                     	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PKTBUF_DBG_CTRL                    (Offset 0x0140)


#define BIT_SHIFT_PKTBUF_WRITE_EN             	24
#define BIT_MASK_PKTBUF_WRITE_EN              	0xff
#define BIT_PKTBUF_WRITE_EN(x)                	(((x) & BIT_MASK_PKTBUF_WRITE_EN) << BIT_SHIFT_PKTBUF_WRITE_EN)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PKTBUF_DBG_CTRL                    (Offset 0x0140)

#define BIT_TXPKT_BUF_READ_EN                 	BIT(23)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PKTBUF_DBG_CTRL                    (Offset 0x0140)

#define BIT_TXRPTBUF_DBG                      	BIT(23)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PKTBUF_DBG_CTRL                    (Offset 0x0140)

#define BIT_TXRPT_BUF_READ_EN                 	BIT(20)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PKTBUF_DBG_CTRL                    (Offset 0x0140)

#define BIT_TXPKTBUF_DBG_V2                   	BIT(20)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PKTBUF_DBG_CTRL                    (Offset 0x0140)

#define BIT_RXPKT_BUF_READ_EN                 	BIT(16)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_PKTBUF_DBG_CTRL                    (Offset 0x0140)

#define BIT_RXPKTBUF_DBG                      	BIT(16)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PKTBUF_DBG_CTRL                    (Offset 0x0140)


#define BIT_SHIFT_PKTBUF_ADDR                 	0
#define BIT_MASK_PKTBUF_ADDR                  	0x1fff
#define BIT_PKTBUF_ADDR(x)                    	(((x) & BIT_MASK_PKTBUF_ADDR) << BIT_SHIFT_PKTBUF_ADDR)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_PKTBUF_DBG_CTRL                    (Offset 0x0140)


#define BIT_SHIFT_PKTBUF_DBG_ADDR             	0
#define BIT_MASK_PKTBUF_DBG_ADDR              	0x1fff
#define BIT_PKTBUF_DBG_ADDR(x)                	(((x) & BIT_MASK_PKTBUF_DBG_ADDR) << BIT_SHIFT_PKTBUF_DBG_ADDR)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PKTBUF_DBG_DATA_L                  (Offset 0x0144)


#define BIT_SHIFT_PKTBUF_DBG_DATA_L           	0
#define BIT_MASK_PKTBUF_DBG_DATA_L            	0xffffffffL
#define BIT_PKTBUF_DBG_DATA_L(x)              	(((x) & BIT_MASK_PKTBUF_DBG_DATA_L) << BIT_SHIFT_PKTBUF_DBG_DATA_L)


//2 REG_PKTBUF_DBG_DATA_H                  (Offset 0x0148)


#define BIT_SHIFT_PKTBUF_DBG_DATA_H           	0
#define BIT_MASK_PKTBUF_DBG_DATA_H            	0xffffffffL
#define BIT_PKTBUF_DBG_DATA_H(x)              	(((x) & BIT_MASK_PKTBUF_DBG_DATA_H) << BIT_SHIFT_PKTBUF_DBG_DATA_H)


//2 REG_CPWM2                              (Offset 0x014C)


#define BIT_SHIFT_L0S_TO_RCVY_NUM             	16
#define BIT_MASK_L0S_TO_RCVY_NUM              	0xff
#define BIT_L0S_TO_RCVY_NUM(x)                	(((x) & BIT_MASK_L0S_TO_RCVY_NUM) << BIT_SHIFT_L0S_TO_RCVY_NUM)

#define BIT_CPWM2_TOGGLING                    	BIT(15)

#define BIT_SHIFT_CPWM2_MOD                   	0
#define BIT_MASK_CPWM2_MOD                    	0x7fff
#define BIT_CPWM2_MOD(x)                      	(((x) & BIT_MASK_CPWM2_MOD) << BIT_SHIFT_CPWM2_MOD)


//2 REG_TC0_CTRL                           (Offset 0x0150)

#define BIT_TC0INT_EN                         	BIT(26)
#define BIT_TC0MODE                           	BIT(25)
#define BIT_TC0EN                             	BIT(24)

#define BIT_SHIFT_TC0DATA                     	0
#define BIT_MASK_TC0DATA                      	0xffffff
#define BIT_TC0DATA(x)                        	(((x) & BIT_MASK_TC0DATA) << BIT_SHIFT_TC0DATA)


//2 REG_TC1_CTRL                           (Offset 0x0154)

#define BIT_TC1INT_EN                         	BIT(26)
#define BIT_TC1MODE                           	BIT(25)
#define BIT_TC1EN                             	BIT(24)

#define BIT_SHIFT_TC1DATA                     	0
#define BIT_MASK_TC1DATA                      	0xffffff
#define BIT_TC1DATA(x)                        	(((x) & BIT_MASK_TC1DATA) << BIT_SHIFT_TC1DATA)


//2 REG_TC2_CTRL                           (Offset 0x0158)

#define BIT_TC2INT_EN                         	BIT(26)
#define BIT_TC2MODE                           	BIT(25)
#define BIT_TC2EN                             	BIT(24)

#define BIT_SHIFT_TC2DATA                     	0
#define BIT_MASK_TC2DATA                      	0xffffff
#define BIT_TC2DATA(x)                        	(((x) & BIT_MASK_TC2DATA) << BIT_SHIFT_TC2DATA)


//2 REG_TC3_CTRL                           (Offset 0x015C)

#define BIT_TC3INT_EN                         	BIT(26)
#define BIT_TC3MODE                           	BIT(25)
#define BIT_TC3EN                             	BIT(24)

#define BIT_SHIFT_TC3DATA                     	0
#define BIT_MASK_TC3DATA                      	0xffffff
#define BIT_TC3DATA(x)                        	(((x) & BIT_MASK_TC3DATA) << BIT_SHIFT_TC3DATA)


//2 REG_TC4_CTRL                           (Offset 0x0160)

#define BIT_TC4INT_EN                         	BIT(26)
#define BIT_TC4MODE                           	BIT(25)
#define BIT_TC4EN                             	BIT(24)

#define BIT_SHIFT_TC4DATA                     	0
#define BIT_MASK_TC4DATA                      	0xffffff
#define BIT_TC4DATA(x)                        	(((x) & BIT_MASK_TC4DATA) << BIT_SHIFT_TC4DATA)


//2 REG_TCUNIT_BASE                        (Offset 0x0164)


#define BIT_SHIFT_TCUNIT_BASE                 	0
#define BIT_MASK_TCUNIT_BASE                  	0x3fff
#define BIT_TCUNIT_BASE(x)                    	(((x) & BIT_MASK_TCUNIT_BASE) << BIT_SHIFT_TCUNIT_BASE)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TC5_CTRL                           (Offset 0x0168)

#define BIT_TC50INT_EN                        	BIT(26)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_TC5_CTRL                           (Offset 0x0168)

#define BIT_TC5INT_EN                         	BIT(26)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TC5_CTRL                           (Offset 0x0168)

#define BIT_TC5MODE                           	BIT(25)
#define BIT_TC5EN                             	BIT(24)

#define BIT_SHIFT_TC5DATA                     	0
#define BIT_MASK_TC5DATA                      	0xffffff
#define BIT_TC5DATA(x)                        	(((x) & BIT_MASK_TC5DATA) << BIT_SHIFT_TC5DATA)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TC6_CTRL                           (Offset 0x016C)

#define BIT_TC60INT_EN                        	BIT(26)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_TC6_CTRL                           (Offset 0x016C)

#define BIT_TC6INT_EN                         	BIT(26)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TC6_CTRL                           (Offset 0x016C)

#define BIT_TC6MODE                           	BIT(25)
#define BIT_TC6EN                             	BIT(24)

#define BIT_SHIFT_TC6DATA                     	0
#define BIT_MASK_TC6DATA                      	0xffffff
#define BIT_TC6DATA(x)                        	(((x) & BIT_MASK_TC6DATA) << BIT_SHIFT_TC6DATA)


//2 REG_MBIST_FAIL                         (Offset 0x0170)


#define BIT_SHIFT_8051_MBIST_FAIL             	26
#define BIT_MASK_8051_MBIST_FAIL              	0x7
#define BIT_8051_MBIST_FAIL(x)                	(((x) & BIT_MASK_8051_MBIST_FAIL) << BIT_SHIFT_8051_MBIST_FAIL)


#define BIT_SHIFT_USB_MBIST_FAIL              	24
#define BIT_MASK_USB_MBIST_FAIL               	0x3
#define BIT_USB_MBIST_FAIL(x)                 	(((x) & BIT_MASK_USB_MBIST_FAIL) << BIT_SHIFT_USB_MBIST_FAIL)


#define BIT_SHIFT_PCIE_MBIST_FAIL             	16
#define BIT_MASK_PCIE_MBIST_FAIL              	0x3f
#define BIT_PCIE_MBIST_FAIL(x)                	(((x) & BIT_MASK_PCIE_MBIST_FAIL) << BIT_SHIFT_PCIE_MBIST_FAIL)


#define BIT_SHIFT_MAC_MBIST_FAIL              	0
#define BIT_MASK_MAC_MBIST_FAIL               	0xfff
#define BIT_MAC_MBIST_FAIL(x)                 	(((x) & BIT_MASK_MAC_MBIST_FAIL) << BIT_SHIFT_MAC_MBIST_FAIL)


//2 REG_MBIST_START_PAUSE                  (Offset 0x0174)


#define BIT_SHIFT_8051_MBIST_START_PAUSE      	26
#define BIT_MASK_8051_MBIST_START_PAUSE       	0x7
#define BIT_8051_MBIST_START_PAUSE(x)         	(((x) & BIT_MASK_8051_MBIST_START_PAUSE) << BIT_SHIFT_8051_MBIST_START_PAUSE)


#define BIT_SHIFT_USB_MBIST_START_PAUSE       	24
#define BIT_MASK_USB_MBIST_START_PAUSE        	0x3
#define BIT_USB_MBIST_START_PAUSE(x)          	(((x) & BIT_MASK_USB_MBIST_START_PAUSE) << BIT_SHIFT_USB_MBIST_START_PAUSE)


#define BIT_SHIFT_PCIE_MBIST_START_PAUSE      	16
#define BIT_MASK_PCIE_MBIST_START_PAUSE       	0x3f
#define BIT_PCIE_MBIST_START_PAUSE(x)         	(((x) & BIT_MASK_PCIE_MBIST_START_PAUSE) << BIT_SHIFT_PCIE_MBIST_START_PAUSE)


#define BIT_SHIFT_MAC_MBIST_START_PAUSE       	0
#define BIT_MASK_MAC_MBIST_START_PAUSE        	0xfff
#define BIT_MAC_MBIST_START_PAUSE(x)          	(((x) & BIT_MASK_MAC_MBIST_START_PAUSE) << BIT_SHIFT_MAC_MBIST_START_PAUSE)


//2 REG_MBIST_DONE                         (Offset 0x0178)


#define BIT_SHIFT_8051_MBIST_DONE             	26
#define BIT_MASK_8051_MBIST_DONE              	0x7
#define BIT_8051_MBIST_DONE(x)                	(((x) & BIT_MASK_8051_MBIST_DONE) << BIT_SHIFT_8051_MBIST_DONE)


#define BIT_SHIFT_USB_MBIST_DONE              	24
#define BIT_MASK_USB_MBIST_DONE               	0x3
#define BIT_USB_MBIST_DONE(x)                 	(((x) & BIT_MASK_USB_MBIST_DONE) << BIT_SHIFT_USB_MBIST_DONE)


#define BIT_SHIFT_PCIE_MBIST_DONE             	16
#define BIT_MASK_PCIE_MBIST_DONE              	0x3f
#define BIT_PCIE_MBIST_DONE(x)                	(((x) & BIT_MASK_PCIE_MBIST_DONE) << BIT_SHIFT_PCIE_MBIST_DONE)


#define BIT_SHIFT_MAC_MBIST_DONE              	0
#define BIT_MASK_MAC_MBIST_DONE               	0xfff
#define BIT_MAC_MBIST_DONE(x)                 	(((x) & BIT_MASK_MAC_MBIST_DONE) << BIT_SHIFT_MAC_MBIST_DONE)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_MBIST_ROM_CRC_DATA                 (Offset 0x017C)


#define BIT_SHIFT_MBIST_ROM_CRC_DATA          	0
#define BIT_MASK_MBIST_ROM_CRC_DATA           	0xffffffffL
#define BIT_MBIST_ROM_CRC_DATA(x)             	(((x) & BIT_MASK_MBIST_ROM_CRC_DATA) << BIT_SHIFT_MBIST_ROM_CRC_DATA)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_MBIST_FAIL_NRML                    (Offset 0x017C)


#define BIT_SHIFT_MBIST_FAIL_NRML             	0
#define BIT_MASK_MBIST_FAIL_NRML              	0xffffffffL
#define BIT_MBIST_FAIL_NRML(x)                	(((x) & BIT_MASK_MBIST_FAIL_NRML) << BIT_SHIFT_MBIST_FAIL_NRML)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AES_DECRPT_DATA                    (Offset 0x0180)


#define BIT_SHIFT_IPS_CFG_ADDR                	0
#define BIT_MASK_IPS_CFG_ADDR                 	0xff
#define BIT_IPS_CFG_ADDR(x)                   	(((x) & BIT_MASK_IPS_CFG_ADDR) << BIT_SHIFT_IPS_CFG_ADDR)


//2 REG_AES_DECRPT_CFG                     (Offset 0x0184)


#define BIT_SHIFT_IPS_CFG_DATA                	0
#define BIT_MASK_IPS_CFG_DATA                 	0xffffffffL
#define BIT_IPS_CFG_DATA(x)                   	(((x) & BIT_MASK_IPS_CFG_DATA) << BIT_SHIFT_IPS_CFG_DATA)


//2 REG_TMETER                             (Offset 0x0190)

#define BIT_TEMP_VALID                        	BIT(31)

#define BIT_SHIFT_TEMP_VALUE                  	24
#define BIT_MASK_TEMP_VALUE                   	0x3f
#define BIT_TEMP_VALUE(x)                     	(((x) & BIT_MASK_TEMP_VALUE) << BIT_SHIFT_TEMP_VALUE)


#define BIT_SHIFT_REG_TMETER_TIMER            	8
#define BIT_MASK_REG_TMETER_TIMER             	0xfff
#define BIT_REG_TMETER_TIMER(x)               	(((x) & BIT_MASK_REG_TMETER_TIMER) << BIT_SHIFT_REG_TMETER_TIMER)


#define BIT_SHIFT_REG_TEMP_DELTA              	2
#define BIT_MASK_REG_TEMP_DELTA               	0x3f
#define BIT_REG_TEMP_DELTA(x)                 	(((x) & BIT_MASK_REG_TEMP_DELTA) << BIT_SHIFT_REG_TEMP_DELTA)

#define BIT_REG_TMETER_EN                     	BIT(0)

//2 REG_OSC_32K_CTRL                       (Offset 0x0194)


#define BIT_SHIFT_OSC_32K_CLKGEN_0            	16
#define BIT_MASK_OSC_32K_CLKGEN_0             	0xffff
#define BIT_OSC_32K_CLKGEN_0(x)               	(((x) & BIT_MASK_OSC_32K_CLKGEN_0) << BIT_SHIFT_OSC_32K_CLKGEN_0)


#define BIT_SHIFT_OSC_32K_RES_COMP            	4
#define BIT_MASK_OSC_32K_RES_COMP             	0x3
#define BIT_OSC_32K_RES_COMP(x)               	(((x) & BIT_MASK_OSC_32K_RES_COMP) << BIT_SHIFT_OSC_32K_RES_COMP)

#define BIT_OSC_32K_OUT_SEL                   	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT)


//2 REG_OSC_32K_CTRL                       (Offset 0x0194)

#define BIT_POW_CKGEN                         	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_32K_CAL_REG1                       (Offset 0x0198)

#define BIT_CAL_32K_REG_WR                    	BIT(31)
#define BIT_CAL_32K_DBG_SEL                   	BIT(22)

#define BIT_SHIFT_CAL_32K_REG_ADDR            	16
#define BIT_MASK_CAL_32K_REG_ADDR             	0x3f
#define BIT_CAL_32K_REG_ADDR(x)               	(((x) & BIT_MASK_CAL_32K_REG_ADDR) << BIT_SHIFT_CAL_32K_REG_ADDR)


#define BIT_SHIFT_CAL_32K_REG_DATA            	0
#define BIT_MASK_CAL_32K_REG_DATA             	0xffff
#define BIT_CAL_32K_REG_DATA(x)               	(((x) & BIT_MASK_CAL_32K_REG_DATA) << BIT_SHIFT_CAL_32K_REG_DATA)


//2 REG_C2HEVT                             (Offset 0x01A0)


#define BIT_SHIFT_C2HEVT_MSG                  	0
#define BIT_MASK_C2HEVT_MSG                   	0xffffffffffffffffffffffffffffffffL
#define BIT_C2HEVT_MSG(x)                     	(((x) & BIT_MASK_C2HEVT_MSG) << BIT_SHIFT_C2HEVT_MSG)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TC7_CTRL                           (Offset 0x01B0)

#define BIT_TC7INT_EN                         	BIT(26)
#define BIT_TC7MODE                           	BIT(25)
#define BIT_TC7EN                             	BIT(24)

#define BIT_SHIFT_TC7DATA                     	0
#define BIT_MASK_TC7DATA                      	0xffffff
#define BIT_TC7DATA(x)                        	(((x) & BIT_MASK_TC7DATA) << BIT_SHIFT_TC7DATA)


//2 REG_TC8_CTRL                           (Offset 0x01B4)

#define BIT_TC8INT_EN                         	BIT(26)
#define BIT_TC8MODE                           	BIT(25)
#define BIT_TC8EN                             	BIT(24)

#define BIT_SHIFT_TC8DATA                     	0
#define BIT_MASK_TC8DATA                      	0xffffff
#define BIT_TC8DATA(x)                        	(((x) & BIT_MASK_TC8DATA) << BIT_SHIFT_TC8DATA)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SW_DEFINED_PAGE1                   (Offset 0x01B8)


#define BIT_SHIFT_SW_DEFINED_PAGE1            	0
#define BIT_MASK_SW_DEFINED_PAGE1             	0xffffffffffffffffL
#define BIT_SW_DEFINED_PAGE1(x)               	(((x) & BIT_MASK_SW_DEFINED_PAGE1) << BIT_SHIFT_SW_DEFINED_PAGE1)


//2 REG_MCUTST_I                           (Offset 0x01C0)


#define BIT_SHIFT_MCUDMSG_I                   	0
#define BIT_MASK_MCUDMSG_I                    	0xffffffffL
#define BIT_MCUDMSG_I(x)                      	(((x) & BIT_MASK_MCUDMSG_I) << BIT_SHIFT_MCUDMSG_I)


//2 REG_MCUTST_II                          (Offset 0x01C4)


#define BIT_SHIFT_MCUDMSG_II                  	0
#define BIT_MASK_MCUDMSG_II                   	0xffffffffL
#define BIT_MCUDMSG_II(x)                     	(((x) & BIT_MASK_MCUDMSG_II) << BIT_SHIFT_MCUDMSG_II)


//2 REG_FMETHR                             (Offset 0x01C8)

#define BIT_FMSG_INT                          	BIT(31)

#define BIT_SHIFT_FW_MSG                      	0
#define BIT_MASK_FW_MSG                       	0xffffffffL
#define BIT_FW_MSG(x)                         	(((x) & BIT_MASK_FW_MSG) << BIT_SHIFT_FW_MSG)


//2 REG_HMETFR                             (Offset 0x01CC)


#define BIT_SHIFT_HRCV_MSG                    	24
#define BIT_MASK_HRCV_MSG                     	0xff
#define BIT_HRCV_MSG(x)                       	(((x) & BIT_MASK_HRCV_MSG) << BIT_SHIFT_HRCV_MSG)

#define BIT_INT_BOX3                          	BIT(3)
#define BIT_INT_BOX2                          	BIT(2)
#define BIT_INT_BOX1                          	BIT(1)
#define BIT_INT_BOX0                          	BIT(0)

//2 REG_HMEBOX0                            (Offset 0x01D0)


#define BIT_SHIFT_HOST_MSG_0                  	0
#define BIT_MASK_HOST_MSG_0                   	0xffffffffL
#define BIT_HOST_MSG_0(x)                     	(((x) & BIT_MASK_HOST_MSG_0) << BIT_SHIFT_HOST_MSG_0)


//2 REG_HMEBOX1                            (Offset 0x01D4)


#define BIT_SHIFT_HOST_MSG_1                  	0
#define BIT_MASK_HOST_MSG_1                   	0xffffffffL
#define BIT_HOST_MSG_1(x)                     	(((x) & BIT_MASK_HOST_MSG_1) << BIT_SHIFT_HOST_MSG_1)


//2 REG_HMEBOX2                            (Offset 0x01D8)


#define BIT_SHIFT_HOST_MSG_2                  	0
#define BIT_MASK_HOST_MSG_2                   	0xffffffffL
#define BIT_HOST_MSG_2(x)                     	(((x) & BIT_MASK_HOST_MSG_2) << BIT_SHIFT_HOST_MSG_2)


//2 REG_HMEBOX3                            (Offset 0x01DC)


#define BIT_SHIFT_HOST_MSG_3                  	0
#define BIT_MASK_HOST_MSG_3                   	0xffffffffL
#define BIT_HOST_MSG_3(x)                     	(((x) & BIT_MASK_HOST_MSG_3) << BIT_SHIFT_HOST_MSG_3)


//2 REG_LLT_INIT                           (Offset 0x01E0)


#define BIT_SHIFT_LLTE_RWM                    	30
#define BIT_MASK_LLTE_RWM                     	0x3
#define BIT_LLTE_RWM(x)                       	(((x) & BIT_MASK_LLTE_RWM) << BIT_SHIFT_LLTE_RWM)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LLT_INIT                           (Offset 0x01E0)


#define BIT_SHIFT_LLTINI_PDATA                	16
#define BIT_MASK_LLTINI_PDATA                 	0xff
#define BIT_LLTINI_PDATA(x)                   	(((x) & BIT_MASK_LLTINI_PDATA) << BIT_SHIFT_LLTINI_PDATA)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_LLT_INIT                           (Offset 0x01E0)


#define BIT_SHIFT_LLTINI_PDATA_V1             	16
#define BIT_MASK_LLTINI_PDATA_V1              	0xfff
#define BIT_LLTINI_PDATA_V1(x)                	(((x) & BIT_MASK_LLTINI_PDATA_V1) << BIT_SHIFT_LLTINI_PDATA_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LLT_INIT                           (Offset 0x01E0)


#define BIT_SHIFT_LLTINI_ADDR                 	8
#define BIT_MASK_LLTINI_ADDR                  	0xff
#define BIT_LLTINI_ADDR(x)                    	(((x) & BIT_MASK_LLTINI_ADDR) << BIT_SHIFT_LLTINI_ADDR)


#define BIT_SHIFT_LLTINI_HDATA                	0
#define BIT_MASK_LLTINI_HDATA                 	0xff
#define BIT_LLTINI_HDATA(x)                   	(((x) & BIT_MASK_LLTINI_HDATA) << BIT_SHIFT_LLTINI_HDATA)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_LLT_INIT                           (Offset 0x01E0)


#define BIT_SHIFT_LLTINI_HDATA_V1             	0
#define BIT_MASK_LLTINI_HDATA_V1              	0xfff
#define BIT_LLTINI_HDATA_V1(x)                	(((x) & BIT_MASK_LLTINI_HDATA_V1) << BIT_SHIFT_LLTINI_HDATA_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_GENTST                             (Offset 0x01E4)


#define BIT_SHIFT_GENTST                      	0
#define BIT_MASK_GENTST                       	0xffffffffL
#define BIT_GENTST(x)                         	(((x) & BIT_MASK_GENTST) << BIT_SHIFT_GENTST)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_LLT_INIT_ADDR                      (Offset 0x01E4)


#define BIT_SHIFT_LLTINI_ADDR_V1              	0
#define BIT_MASK_LLTINI_ADDR_V1               	0xfff
#define BIT_LLTINI_ADDR_V1(x)                 	(((x) & BIT_MASK_LLTINI_ADDR_V1) << BIT_SHIFT_LLTINI_ADDR_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BB_ACCESS_CTRL                     (Offset 0x01E8)


#define BIT_SHIFT_BB_WRITE_READ               	30
#define BIT_MASK_BB_WRITE_READ                	0x3
#define BIT_BB_WRITE_READ(x)                  	(((x) & BIT_MASK_BB_WRITE_READ) << BIT_SHIFT_BB_WRITE_READ)


#define BIT_SHIFT_BB_WRITE_EN                 	12
#define BIT_MASK_BB_WRITE_EN                  	0xf
#define BIT_BB_WRITE_EN(x)                    	(((x) & BIT_MASK_BB_WRITE_EN) << BIT_SHIFT_BB_WRITE_EN)


#define BIT_SHIFT_BB_ADDR                     	2
#define BIT_MASK_BB_ADDR                      	0x1ff
#define BIT_BB_ADDR(x)                        	(((x) & BIT_MASK_BB_ADDR) << BIT_SHIFT_BB_ADDR)

#define BIT_BB_ERRACC                         	BIT(0)

//2 REG_BB_ACCESS_DATA                     (Offset 0x01EC)

#define BIT_LD_RQPN                           	BIT(31)

#define BIT_SHIFT_BB_DATA                     	0
#define BIT_MASK_BB_DATA                      	0xffffffffL
#define BIT_BB_DATA(x)                        	(((x) & BIT_MASK_BB_DATA) << BIT_SHIFT_BB_DATA)


#define BIT_SHIFT_HMEBOX                      	0
#define BIT_MASK_HMEBOX                       	0xffffffffL
#define BIT_HMEBOX(x)                         	(((x) & BIT_MASK_HMEBOX) << BIT_SHIFT_HMEBOX)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RQPN_CTRL_HLPQ                     (Offset 0x0200)

#define BIT_EPQ_PUBLIC_DIS                    	BIT(27)
#define BIT_NPQ_PUBLIC_DIS                    	BIT(26)
#define BIT_LPQ_PUBLIC_DIS                    	BIT(25)
#define BIT_HPQ_PUBLIC_DIS                    	BIT(24)

#define BIT_SHIFT_PUBQ                        	16
#define BIT_MASK_PUBQ                         	0xff
#define BIT_PUBQ(x)                           	(((x) & BIT_MASK_PUBQ) << BIT_SHIFT_PUBQ)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_FIFOPAGE_CTRL_1                    (Offset 0x0200)


#define BIT_SHIFT_Tx_OQT_HE_free_space_V1     	16
#define BIT_MASK_Tx_OQT_HE_free_space_V1      	0xff
#define BIT_Tx_OQT_HE_free_space_V1(x)        	(((x) & BIT_MASK_Tx_OQT_HE_free_space_V1) << BIT_SHIFT_Tx_OQT_HE_free_space_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RQPN_CTRL_HLPQ                     (Offset 0x0200)


#define BIT_SHIFT_LPQ                         	8
#define BIT_MASK_LPQ                          	0xff
#define BIT_LPQ(x)                            	(((x) & BIT_MASK_LPQ) << BIT_SHIFT_LPQ)


#define BIT_SHIFT_HPQ                         	0
#define BIT_MASK_HPQ                          	0xff
#define BIT_HPQ(x)                            	(((x) & BIT_MASK_HPQ) << BIT_SHIFT_HPQ)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_FIFOPAGE_CTRL_1                    (Offset 0x0200)


#define BIT_SHIFT_Tx_OQT_NL_free_space_V1     	0
#define BIT_MASK_Tx_OQT_NL_free_space_V1      	0xff
#define BIT_Tx_OQT_NL_free_space_V1(x)        	(((x) & BIT_MASK_Tx_OQT_NL_free_space_V1) << BIT_SHIFT_Tx_OQT_NL_free_space_V1)


//2 REG_FIFOPAGE_CTRL_2                    (Offset 0x0204)

#define BIT_BCN_VALID_1_V1                    	BIT(31)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FIFOPAGE_INFO                      (Offset 0x0204)


#define BIT_SHIFT_TXPKTNUM                    	24
#define BIT_MASK_TXPKTNUM                     	0xff
#define BIT_TXPKTNUM(x)                       	(((x) & BIT_MASK_TXPKTNUM) << BIT_SHIFT_TXPKTNUM)


#define BIT_SHIFT_PUBQ_AVAL_PG                	16
#define BIT_MASK_PUBQ_AVAL_PG                 	0xff
#define BIT_PUBQ_AVAL_PG(x)                   	(((x) & BIT_MASK_PUBQ_AVAL_PG) << BIT_SHIFT_PUBQ_AVAL_PG)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_FIFOPAGE_CTRL_2                    (Offset 0x0204)


#define BIT_SHIFT_BCN_HEAD_1_V1               	16
#define BIT_MASK_BCN_HEAD_1_V1                	0xfff
#define BIT_BCN_HEAD_1_V1(x)                  	(((x) & BIT_MASK_BCN_HEAD_1_V1) << BIT_SHIFT_BCN_HEAD_1_V1)

#define BIT_BCN_VALID_V1                      	BIT(15)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FIFOPAGE_INFO                      (Offset 0x0204)


#define BIT_SHIFT_LPQ_AVAL_PG                 	8
#define BIT_MASK_LPQ_AVAL_PG                  	0xff
#define BIT_LPQ_AVAL_PG(x)                    	(((x) & BIT_MASK_LPQ_AVAL_PG) << BIT_SHIFT_LPQ_AVAL_PG)


#define BIT_SHIFT_HPQ_AVAL_PG                 	0
#define BIT_MASK_HPQ_AVAL_PG                  	0xff
#define BIT_HPQ_AVAL_PG(x)                    	(((x) & BIT_MASK_HPQ_AVAL_PG) << BIT_SHIFT_HPQ_AVAL_PG)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_FIFOPAGE_CTRL_2                    (Offset 0x0204)


#define BIT_SHIFT_BCN_HEAD_V1                 	0
#define BIT_MASK_BCN_HEAD_V1                  	0xfff
#define BIT_BCN_HEAD_V1(x)                    	(((x) & BIT_MASK_BCN_HEAD_V1) << BIT_SHIFT_BCN_HEAD_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_DWBCN0_CTRL                        (Offset 0x0208)


#define BIT_SHIFT_LLT_FREE_PAGE               	24
#define BIT_MASK_LLT_FREE_PAGE                	0xff
#define BIT_LLT_FREE_PAGE(x)                  	(((x) & BIT_MASK_LLT_FREE_PAGE) << BIT_SHIFT_LLT_FREE_PAGE)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AUTO_LLT_V1                        (Offset 0x0208)


#define BIT_SHIFT_Max_tx_pkt_for_USB_and_SDIO_V1	24
#define BIT_MASK_Max_tx_pkt_for_USB_and_SDIO_V1	0xff
#define BIT_Max_tx_pkt_for_USB_and_SDIO_V1(x) 	(((x) & BIT_MASK_Max_tx_pkt_for_USB_and_SDIO_V1) << BIT_SHIFT_Max_tx_pkt_for_USB_and_SDIO_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_DWBCN0_CTRL                        (Offset 0x0208)

#define BIT_BCN_VALID                         	BIT(16)

#define BIT_SHIFT_BCN_HEAD                    	8
#define BIT_MASK_BCN_HEAD                     	0xff
#define BIT_BCN_HEAD(x)                       	(((x) & BIT_MASK_BCN_HEAD) << BIT_SHIFT_BCN_HEAD)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AUTO_LLT_V1                        (Offset 0x0208)


#define BIT_SHIFT_LLT_FREE_PAGE_v1            	8
#define BIT_MASK_LLT_FREE_PAGE_v1             	0xffff
#define BIT_LLT_FREE_PAGE_v1(x)               	(((x) & BIT_MASK_LLT_FREE_PAGE_v1) << BIT_SHIFT_LLT_FREE_PAGE_v1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AUTO_LLT_V1                        (Offset 0x0208)


#define BIT_SHIFT_BLK_DESC_NUM                	4
#define BIT_MASK_BLK_DESC_NUM                 	0xf
#define BIT_BLK_DESC_NUM(x)                   	(((x) & BIT_MASK_BLK_DESC_NUM) << BIT_SHIFT_BLK_DESC_NUM)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_AUTO_LLT_V1                        (Offset 0x0208)

#define BIT_R_BCN_HEAD_SEL                    	BIT(3)
#define BIT_r_EN_BCN_SW_HEAD_SEL              	BIT(2)
#define BIT_DBG_SEL                           	BIT(1)
#define BIT_Auto_Init_LLT_V1                  	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXDMA_OFFSET_CHK                   (Offset 0x020C)

#define BIT_EM_CHKSUM_FIN                     	BIT(31)
#define BIT_EMN_PCIE_DMA_MOD                  	BIT(30)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_TXDMA_OFFSET_CHK                   (Offset 0x020C)

#define BIT_EN_TXQUE_CLR                      	BIT(29)
#define BIT_EN_PCIE_FIFO_MODE                 	BIT(28)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXDMA_OFFSET_CHK                   (Offset 0x020C)


#define BIT_SHIFT_PG_UNDER_TH                 	16
#define BIT_MASK_PG_UNDER_TH                  	0xff
#define BIT_PG_UNDER_TH(x)                    	(((x) & BIT_MASK_PG_UNDER_TH) << BIT_SHIFT_PG_UNDER_TH)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TXDMA_OFFSET_CHK                   (Offset 0x020C)


#define BIT_SHIFT_PG_UNDER_TH_v1              	16
#define BIT_MASK_PG_UNDER_TH_v1               	0xfff
#define BIT_PG_UNDER_TH_v1(x)                 	(((x) & BIT_MASK_PG_UNDER_TH_v1) << BIT_SHIFT_PG_UNDER_TH_v1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXDMA_OFFSET_CHK                   (Offset 0x020C)

#define BIT_SDIO_TXDESC_CHKSUM_EN             	BIT(13)
#define BIT_RST_RDPTR                         	BIT(12)
#define BIT_RST_WRPTR                         	BIT(11)
#define BIT_CHK_PG_TH_EN                      	BIT(10)
#define BIT_DROP_DATA_EN                      	BIT(9)
#define BIT_CHECK_OFFSET_EN                   	BIT(8)

#define BIT_SHIFT_CHECK_OFFSET                	0
#define BIT_MASK_CHECK_OFFSET                 	0xff
#define BIT_CHECK_OFFSET(x)                   	(((x) & BIT_MASK_CHECK_OFFSET) << BIT_SHIFT_CHECK_OFFSET)


//2 REG_TXDMA_STATUS                       (Offset 0x0210)

#define BIT_HI_OQT_UDN                        	BIT(17)
#define BIT_HI_OQT_OVF                        	BIT(16)
#define BIT_PAYLOAD_CHKSUM_ERR                	BIT(15)
#define BIT_PAYLOAD_UDN                       	BIT(14)
#define BIT_PAYLOAD_OVF                       	BIT(13)
#define BIT_DSC_CHKSUM_FAIL                   	BIT(12)
#define BIT_UNKNOWN_QSEL                      	BIT(11)
#define BIT_EP_QSEL_DIFF                      	BIT(10)
#define BIT_TX_OFFS_UNMATCH                   	BIT(9)
#define BIT_TXOQT_UDN                         	BIT(8)

#define BIT_SHIFT_NPQ_AVAL_PG                 	8
#define BIT_MASK_NPQ_AVAL_PG                  	0xff
#define BIT_NPQ_AVAL_PG(x)                    	(((x) & BIT_MASK_NPQ_AVAL_PG) << BIT_SHIFT_NPQ_AVAL_PG)

#define BIT_TXOQT_OVF                         	BIT(7)
#define BIT_TXDMA_SFF_UDN                     	BIT(6)
#define BIT_TXDMA_SFF_OVF                     	BIT(5)
#define BIT_LLT_NULL_PG                       	BIT(4)
#define BIT_PAGE_UDN                          	BIT(3)
#define BIT_PAGE_OVF                          	BIT(2)
#define BIT_TXFF_PG_UDN                       	BIT(1)
#define BIT_TXFF_PG_OVF                       	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RQPN_NPQ                           (Offset 0x0214)


#define BIT_SHIFT_EXQ_AVAL_PG                 	24
#define BIT_MASK_EXQ_AVAL_PG                  	0xff
#define BIT_EXQ_AVAL_PG(x)                    	(((x) & BIT_MASK_EXQ_AVAL_PG) << BIT_SHIFT_EXQ_AVAL_PG)


#define BIT_SHIFT_EXQ                         	16
#define BIT_MASK_EXQ                          	0xff
#define BIT_EXQ(x)                            	(((x) & BIT_MASK_EXQ) << BIT_SHIFT_EXQ)


#define BIT_SHIFT_NPQ                         	0
#define BIT_MASK_NPQ                          	0xff
#define BIT_NPQ(x)                            	(((x) & BIT_MASK_NPQ) << BIT_SHIFT_NPQ)


//2 REG_TQPNT1                             (Offset 0x0218)


#define BIT_SHIFT_NPQ_HIGH_TH                 	24
#define BIT_MASK_NPQ_HIGH_TH                  	0xff
#define BIT_NPQ_HIGH_TH(x)                    	(((x) & BIT_MASK_NPQ_HIGH_TH) << BIT_SHIFT_NPQ_HIGH_TH)


#define BIT_SHIFT_NPQ_LOW_TH                  	16
#define BIT_MASK_NPQ_LOW_TH                   	0xff
#define BIT_NPQ_LOW_TH(x)                     	(((x) & BIT_MASK_NPQ_LOW_TH) << BIT_SHIFT_NPQ_LOW_TH)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TQPNT1                             (Offset 0x0218)


#define BIT_SHIFT_HPQ_HIGH_TH_V1              	16
#define BIT_MASK_HPQ_HIGH_TH_V1               	0xfff
#define BIT_HPQ_HIGH_TH_V1(x)                 	(((x) & BIT_MASK_HPQ_HIGH_TH_V1) << BIT_SHIFT_HPQ_HIGH_TH_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TQPNT1                             (Offset 0x0218)


#define BIT_SHIFT_HPQ_HIGH_TH                 	8
#define BIT_MASK_HPQ_HIGH_TH                  	0xff
#define BIT_HPQ_HIGH_TH(x)                    	(((x) & BIT_MASK_HPQ_HIGH_TH) << BIT_SHIFT_HPQ_HIGH_TH)


#define BIT_SHIFT_HPQ_LOW_TH                  	0
#define BIT_MASK_HPQ_LOW_TH                   	0xff
#define BIT_HPQ_LOW_TH(x)                     	(((x) & BIT_MASK_HPQ_LOW_TH) << BIT_SHIFT_HPQ_LOW_TH)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TQPNT1                             (Offset 0x0218)


#define BIT_SHIFT_HPQ_LOW_TH_V1               	0
#define BIT_MASK_HPQ_LOW_TH_V1                	0xfff
#define BIT_HPQ_LOW_TH_V1(x)                  	(((x) & BIT_MASK_HPQ_LOW_TH_V1) << BIT_SHIFT_HPQ_LOW_TH_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TQPNT2                             (Offset 0x021C)


#define BIT_SHIFT_EXQ_HIGH_TH                 	24
#define BIT_MASK_EXQ_HIGH_TH                  	0xff
#define BIT_EXQ_HIGH_TH(x)                    	(((x) & BIT_MASK_EXQ_HIGH_TH) << BIT_SHIFT_EXQ_HIGH_TH)


#define BIT_SHIFT_EXQ_LOW_TH                  	16
#define BIT_MASK_EXQ_LOW_TH                   	0xff
#define BIT_EXQ_LOW_TH(x)                     	(((x) & BIT_MASK_EXQ_LOW_TH) << BIT_SHIFT_EXQ_LOW_TH)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TQPNT2                             (Offset 0x021C)


#define BIT_SHIFT_NPQ_HIGH_TH_V1              	16
#define BIT_MASK_NPQ_HIGH_TH_V1               	0xfff
#define BIT_NPQ_HIGH_TH_V1(x)                 	(((x) & BIT_MASK_NPQ_HIGH_TH_V1) << BIT_SHIFT_NPQ_HIGH_TH_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TQPNT2                             (Offset 0x021C)


#define BIT_SHIFT_LPQ_HIGH_TH                 	8
#define BIT_MASK_LPQ_HIGH_TH                  	0xff
#define BIT_LPQ_HIGH_TH(x)                    	(((x) & BIT_MASK_LPQ_HIGH_TH) << BIT_SHIFT_LPQ_HIGH_TH)


#define BIT_SHIFT_LPQ_LOW_TH                  	0
#define BIT_MASK_LPQ_LOW_TH                   	0xff
#define BIT_LPQ_LOW_TH(x)                     	(((x) & BIT_MASK_LPQ_LOW_TH) << BIT_SHIFT_LPQ_LOW_TH)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TQPNT2                             (Offset 0x021C)


#define BIT_SHIFT_NPQ_LOW_TH_V1               	0
#define BIT_MASK_NPQ_LOW_TH_V1                	0xfff
#define BIT_NPQ_LOW_TH_V1(x)                  	(((x) & BIT_MASK_NPQ_LOW_TH_V1) << BIT_SHIFT_NPQ_LOW_TH_V1)


//2 REG_TQPNT3                             (Offset 0x0220)


#define BIT_SHIFT_LPQ_HIGH_TH_V1              	16
#define BIT_MASK_LPQ_HIGH_TH_V1               	0xfff
#define BIT_LPQ_HIGH_TH_V1(x)                 	(((x) & BIT_MASK_LPQ_HIGH_TH_V1) << BIT_SHIFT_LPQ_HIGH_TH_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TDE_DEBUG                          (Offset 0x0220)


#define BIT_SHIFT_TDE_DEBUG                   	0
#define BIT_MASK_TDE_DEBUG                    	0xffffffffL
#define BIT_TDE_DEBUG(x)                      	(((x) & BIT_MASK_TDE_DEBUG) << BIT_SHIFT_TDE_DEBUG)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TQPNT3                             (Offset 0x0220)


#define BIT_SHIFT_LPQ_LOW_TH_V1               	0
#define BIT_MASK_LPQ_LOW_TH_V1                	0xfff
#define BIT_LPQ_LOW_TH_V1(x)                  	(((x) & BIT_MASK_LPQ_LOW_TH_V1) << BIT_SHIFT_LPQ_LOW_TH_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AUTO_LLT                           (Offset 0x0224)


#define BIT_SHIFT_TXPKTNUM_v1                 	24
#define BIT_MASK_TXPKTNUM_v1                  	0xff
#define BIT_TXPKTNUM_v1(x)                    	(((x) & BIT_MASK_TXPKTNUM_v1) << BIT_SHIFT_TXPKTNUM_v1)

#define BIT_TDE_DBG_SEL                       	BIT(23)
#define BIT_Auto_Init_LLT                     	BIT(16)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_TQPNT4                             (Offset 0x0224)


#define BIT_SHIFT_EXQ_HIGH_TH_V1              	16
#define BIT_MASK_EXQ_HIGH_TH_V1               	0xfff
#define BIT_EXQ_HIGH_TH_V1(x)                 	(((x) & BIT_MASK_EXQ_HIGH_TH_V1) << BIT_SHIFT_EXQ_HIGH_TH_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AUTO_LLT                           (Offset 0x0224)


#define BIT_SHIFT_Tx_OQT_HE_free_space        	8
#define BIT_MASK_Tx_OQT_HE_free_space         	0xff
#define BIT_Tx_OQT_HE_free_space(x)           	(((x) & BIT_MASK_Tx_OQT_HE_free_space) << BIT_SHIFT_Tx_OQT_HE_free_space)


#define BIT_SHIFT_Tx_OQT_NL_free_space        	0
#define BIT_MASK_Tx_OQT_NL_free_space         	0xff
#define BIT_Tx_OQT_NL_free_space(x)           	(((x) & BIT_MASK_Tx_OQT_NL_free_space) << BIT_SHIFT_Tx_OQT_NL_free_space)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TQPNT4                             (Offset 0x0224)


#define BIT_SHIFT_EXQ_LOW_TH_V1               	0
#define BIT_MASK_EXQ_LOW_TH_V1                	0xfff
#define BIT_EXQ_LOW_TH_V1(x)                  	(((x) & BIT_MASK_EXQ_LOW_TH_V1) << BIT_SHIFT_EXQ_LOW_TH_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_DWBCN1_CTRL                        (Offset 0x0228)

#define BIT_SW_BCN_SEL                        	BIT(20)
#define BIT_SW_BCN_SEL_EN                     	BIT(17)
#define BIT_BCN_VALID_1                       	BIT(16)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_RQPN_CTRL_1                        (Offset 0x0228)


#define BIT_SHIFT_TXPKTNUM_H                  	16
#define BIT_MASK_TXPKTNUM_H                   	0xffff
#define BIT_TXPKTNUM_H(x)                     	(((x) & BIT_MASK_TXPKTNUM_H) << BIT_SHIFT_TXPKTNUM_H)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_DWBCN1_CTRL                        (Offset 0x0228)


#define BIT_SHIFT_BCN_HEAD_1                  	8
#define BIT_MASK_BCN_HEAD_1                   	0xff
#define BIT_BCN_HEAD_1(x)                     	(((x) & BIT_MASK_BCN_HEAD_1) << BIT_SHIFT_BCN_HEAD_1)


#define BIT_SHIFT_Max_tx_pkt_for_USB_and_SDIO 	0
#define BIT_MASK_Max_tx_pkt_for_USB_and_SDIO  	0xff
#define BIT_Max_tx_pkt_for_USB_and_SDIO(x)    	(((x) & BIT_MASK_Max_tx_pkt_for_USB_and_SDIO) << BIT_SHIFT_Max_tx_pkt_for_USB_and_SDIO)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_RQPN_CTRL_1                        (Offset 0x0228)


#define BIT_SHIFT_TXPKTNUM_V1                 	0
#define BIT_MASK_TXPKTNUM_V1                  	0xffff
#define BIT_TXPKTNUM_V1(x)                    	(((x) & BIT_MASK_TXPKTNUM_V1) << BIT_SHIFT_TXPKTNUM_V1)


//2 REG_RQPN_CTRL_2                        (Offset 0x022C)

#define BIT_EXQ_PUBLIC_DIS_V1                 	BIT(19)
#define BIT_NPQ_PUBLIC_DIS_V1                 	BIT(18)
#define BIT_LPQ_PUBLIC_DIS_V1                 	BIT(17)
#define BIT_HPQ_PUBLIC_DIS_V1                 	BIT(16)

//2 REG_FIFOPAGE_INFO_1                    (Offset 0x0230)


#define BIT_SHIFT_HPQ_AVAL_PG_V1              	16
#define BIT_MASK_HPQ_AVAL_PG_V1               	0xfff
#define BIT_HPQ_AVAL_PG_V1(x)                 	(((x) & BIT_MASK_HPQ_AVAL_PG_V1) << BIT_SHIFT_HPQ_AVAL_PG_V1)


#define BIT_SHIFT_HPQ_V1                      	0
#define BIT_MASK_HPQ_V1                       	0xfff
#define BIT_HPQ_V1(x)                         	(((x) & BIT_MASK_HPQ_V1) << BIT_SHIFT_HPQ_V1)


//2 REG_FIFOPAGE_INFO_2                    (Offset 0x0234)


#define BIT_SHIFT_LPQ_AVAL_PG_V1              	16
#define BIT_MASK_LPQ_AVAL_PG_V1               	0xfff
#define BIT_LPQ_AVAL_PG_V1(x)                 	(((x) & BIT_MASK_LPQ_AVAL_PG_V1) << BIT_SHIFT_LPQ_AVAL_PG_V1)


#define BIT_SHIFT_LPQ_V1                      	0
#define BIT_MASK_LPQ_V1                       	0xfff
#define BIT_LPQ_V1(x)                         	(((x) & BIT_MASK_LPQ_V1) << BIT_SHIFT_LPQ_V1)


//2 REG_FIFOPAGE_INFO_3                    (Offset 0x0238)


#define BIT_SHIFT_NPQ_V1                      	0
#define BIT_MASK_NPQ_V1                       	0xfff
#define BIT_NPQ_V1(x)                         	(((x) & BIT_MASK_NPQ_V1) << BIT_SHIFT_NPQ_V1)


//2 REG_FIFOPAGE_INFO_4                    (Offset 0x023C)


#define BIT_SHIFT_EXQ_AVAL_PG_V1              	8
#define BIT_MASK_EXQ_AVAL_PG_V1               	0xff
#define BIT_EXQ_AVAL_PG_V1(x)                 	(((x) & BIT_MASK_EXQ_AVAL_PG_V1) << BIT_SHIFT_EXQ_AVAL_PG_V1)


#define BIT_SHIFT_EXQ_V1                      	0
#define BIT_MASK_EXQ_V1                       	0xfff
#define BIT_EXQ_V1(x)                         	(((x) & BIT_MASK_EXQ_V1) << BIT_SHIFT_EXQ_V1)


//2 REG_FIFOPAGE_INFO_5                    (Offset 0x0240)


#define BIT_SHIFT_PUBQ_AVAL_PG_V1             	8
#define BIT_MASK_PUBQ_AVAL_PG_V1              	0xff
#define BIT_PUBQ_AVAL_PG_V1(x)                	(((x) & BIT_MASK_PUBQ_AVAL_PG_V1) << BIT_SHIFT_PUBQ_AVAL_PG_V1)


#define BIT_SHIFT_PUBQ_V1                     	0
#define BIT_MASK_PUBQ_V1                      	0xfff
#define BIT_PUBQ_V1(x)                        	(((x) & BIT_MASK_PUBQ_V1) << BIT_SHIFT_PUBQ_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RXDMA_AGG_PG_TH                    (Offset 0x0280)

#define BIT_USB_RXDMA_AGG_EN                  	BIT(31)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_RXDMA_AGG_PG_TH                    (Offset 0x0280)


#define BIT_SHIFT_RXDMA_AGG_OLD_MOD           	24
#define BIT_MASK_RXDMA_AGG_OLD_MOD            	0xff
#define BIT_RXDMA_AGG_OLD_MOD(x)              	(((x) & BIT_MASK_RXDMA_AGG_OLD_MOD) << BIT_SHIFT_RXDMA_AGG_OLD_MOD)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RXDMA_AGG_PG_TH                    (Offset 0x0280)


#define BIT_SHIFT_PKT_NUM_WOL                 	16
#define BIT_MASK_PKT_NUM_WOL                  	0xff
#define BIT_PKT_NUM_WOL(x)                    	(((x) & BIT_MASK_PKT_NUM_WOL) << BIT_SHIFT_PKT_NUM_WOL)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RXDMA_AGG_PG_TH                    (Offset 0x0280)


#define BIT_SHIFT_DMA_AGG_TO_V1               	8
#define BIT_MASK_DMA_AGG_TO_V1                	0xff
#define BIT_DMA_AGG_TO_V1(x)                  	(((x) & BIT_MASK_DMA_AGG_TO_V1) << BIT_SHIFT_DMA_AGG_TO_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_RXDMA_AGG_PG_TH                    (Offset 0x0280)


#define BIT_SHIFT_DMA_AGG_TO                  	8
#define BIT_MASK_DMA_AGG_TO                   	0xf
#define BIT_DMA_AGG_TO(x)                     	(((x) & BIT_MASK_DMA_AGG_TO) << BIT_SHIFT_DMA_AGG_TO)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RXDMA_AGG_PG_TH                    (Offset 0x0280)


#define BIT_SHIFT_RXDMA_AGG_PG_TH_V1          	0
#define BIT_MASK_RXDMA_AGG_PG_TH_V1           	0xf
#define BIT_RXDMA_AGG_PG_TH_V1(x)             	(((x) & BIT_MASK_RXDMA_AGG_PG_TH_V1) << BIT_SHIFT_RXDMA_AGG_PG_TH_V1)


//2 REG_RXPKT_NUM                          (Offset 0x0284)


#define BIT_SHIFT_RXPKT_NUM                   	24
#define BIT_MASK_RXPKT_NUM                    	0xff
#define BIT_RXPKT_NUM(x)                      	(((x) & BIT_MASK_RXPKT_NUM) << BIT_SHIFT_RXPKT_NUM)

#define BIT_RXDMA_REQ                         	BIT(19)
#define BIT_RW_RELEASE_EN                     	BIT(18)
#define BIT_RXDMA_IDLE                        	BIT(17)
#define BIT_RXPKT_RELEASE_POLL                	BIT(16)

#define BIT_SHIFT_FW_UPD_RDPTR                	0
#define BIT_MASK_FW_UPD_RDPTR                 	0xffff
#define BIT_FW_UPD_RDPTR(x)                   	(((x) & BIT_MASK_FW_UPD_RDPTR) << BIT_SHIFT_FW_UPD_RDPTR)


//2 REG_RXDMA_STATUS                       (Offset 0x0288)

#define BIT_C2H_PKT_OVF                       	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RXDMA_STATUS                       (Offset 0x0288)

#define BIT_AGG_CFG_ISSUE                     	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_RXDMA_STATUS                       (Offset 0x0288)

#define BIT_AGG_CONFGI_ISSUE                  	BIT(6)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RXDMA_STATUS                       (Offset 0x0288)

#define BIT_FW_POLL_ISSUE                     	BIT(5)
#define BIT_RX_DATA_UDN                       	BIT(4)
#define BIT_RX_SFF_UDN                        	BIT(3)
#define BIT_RX_SFF_OVF                        	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RXDMA_STATUS                       (Offset 0x0288)

#define BIT_USB_REQ_LEN_OVF                   	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RXDMA_STATUS                       (Offset 0x0288)

#define BIT_RXPKT_OVF                         	BIT(0)

//2 REG_RXDMA_DPR                          (Offset 0x028C)


#define BIT_SHIFT_rde_debug                   	0
#define BIT_MASK_rde_debug                    	0xffffffffL
#define BIT_rde_debug(x)                      	(((x) & BIT_MASK_rde_debug) << BIT_SHIFT_rde_debug)


//2 REG_RXDMA_MODE                         (Offset 0x0290)


#define BIT_SHIFT_Burst_size                  	4
#define BIT_MASK_Burst_size                   	0x3
#define BIT_Burst_size(x)                     	(((x) & BIT_MASK_Burst_size) << BIT_SHIFT_Burst_size)


#define BIT_SHIFT_Burst_cnt                   	2
#define BIT_MASK_Burst_cnt                    	0x3
#define BIT_Burst_cnt(x)                      	(((x) & BIT_MASK_Burst_cnt) << BIT_SHIFT_Burst_cnt)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RXDMA_MODE                         (Offset 0x0290)

#define BIT_Dam_mode                          	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_RXDMA_MODE                         (Offset 0x0290)

#define BIT_Dma_mode                          	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_C2H_PKT                            (Offset 0x0294)

#define BIT_R_C2H_PKT_REQ                     	BIT(16)

#define BIT_SHIFT_R_C2H_STR_ADDR              	0
#define BIT_MASK_R_C2H_STR_ADDR               	0xffff
#define BIT_R_C2H_STR_ADDR(x)                 	(((x) & BIT_MASK_R_C2H_STR_ADDR) << BIT_SHIFT_R_C2H_STR_ADDR)


#endif


#if 1  //Filen: Page 0/1/2 completion


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT)


//2 REG_PCIE_CTRL                          (Offset 0x0300)

#define BIT_PCIEIO_PERSTB_SEL                 	BIT(31)

#define BIT_SHIFT_PCIE_MAX_RXDMA              	28
#define BIT_MASK_PCIE_MAX_RXDMA               	0x7
#define BIT_PCIE_MAX_RXDMA(x)                 	(((x) & BIT_MASK_PCIE_MAX_RXDMA) << BIT_SHIFT_PCIE_MAX_RXDMA)

#define BIT_MULRW                             	BIT(27)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_LX_CTRL1                           (Offset 0x0300)

#define BIT_RX_LIT_EDN_SEL                    	BIT(27)
#define BIT_TX_LIT_EDN_SEL                    	BIT(26)
#define BIT_WT_LIT_EDN                        	BIT(25)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT)


//2 REG_PCIE_CTRL                          (Offset 0x0300)


#define BIT_SHIFT_PCIE_MAX_TXDMA              	24
#define BIT_MASK_PCIE_MAX_TXDMA               	0x7
#define BIT_PCIE_MAX_TXDMA(x)                 	(((x) & BIT_MASK_PCIE_MAX_TXDMA) << BIT_SHIFT_PCIE_MAX_TXDMA)


#endif


#if (RTL8881A_SUPPORT)


//2 REG_LX_CTRL1                           (Offset 0x0300)

#define BIT_RD_LITT_EDN                       	BIT(24)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT)


//2 REG_PCIE_CTRL                          (Offset 0x0300)

#define BIT_EN_CPL_TIMEOUT_PS                 	BIT(22)
#define BIT_REG_TXDMA_FAIL_PS                 	BIT(21)
#define BIT_PCIE_RST_TRXDMA_INTF              	BIT(20)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_LX_CTRL1                           (Offset 0x0300)


#define BIT_SHIFT_MAX_RXDMA                   	20
#define BIT_MASK_MAX_RXDMA                    	0x7
#define BIT_MAX_RXDMA(x)                      	(((x) & BIT_MASK_MAX_RXDMA) << BIT_SHIFT_MAX_RXDMA)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT)


//2 REG_PCIE_CTRL                          (Offset 0x0300)

#define BIT_EN_HWENTR_L1                      	BIT(19)
#define BIT_EN_ADV_CLKGATE                    	BIT(18)
#define BIT_PCIE_EN_SWENT_L23                 	BIT(17)
#define BIT_PCIE_EN_HWEXT_L1                  	BIT(16)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_LX_CTRL1                           (Offset 0x0300)


#define BIT_SHIFT_MAX_TXDMA                   	16
#define BIT_MASK_MAX_TXDMA                    	0x7
#define BIT_MAX_TXDMA(x)                      	(((x) & BIT_MASK_MAX_TXDMA) << BIT_SHIFT_MAX_TXDMA)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PCIE_CTRL                          (Offset 0x0300)

#define BIT_RX_CLOSE_EN                       	BIT(15)
#define BIT_STOP_BCNQ                         	BIT(14)
#define BIT_STOP_MGQ                          	BIT(13)
#define BIT_STOP_VOQ                          	BIT(12)
#define BIT_STOP_VIQ                          	BIT(11)
#define BIT_STOP_BEQ                          	BIT(10)
#define BIT_STOP_BKQ                          	BIT(9)
#define BIT_STOP_RXQ                          	BIT(8)
#define BIT_STOP_HI7Q                         	BIT(7)
#define BIT_STOP_HI6Q                         	BIT(6)
#define BIT_STOP_HI5Q                         	BIT(5)
#define BIT_STOP_HI4Q                         	BIT(4)
#define BIT_STOP_HI3Q                         	BIT(3)
#define BIT_STOP_HI2Q                         	BIT(2)
#define BIT_STOP_HI1Q                         	BIT(1)
#define BIT_STOP_HI0Q                         	BIT(0)

//2 REG_INT_MIG                            (Offset 0x0304)


#define BIT_SHIFT_TXTTIMER_MATCH_NUM          	28
#define BIT_MASK_TXTTIMER_MATCH_NUM           	0xf
#define BIT_TXTTIMER_MATCH_NUM(x)             	(((x) & BIT_MASK_TXTTIMER_MATCH_NUM) << BIT_SHIFT_TXTTIMER_MATCH_NUM)


#define BIT_SHIFT_TXPKT_NUM_MATCH             	24
#define BIT_MASK_TXPKT_NUM_MATCH              	0xf
#define BIT_TXPKT_NUM_MATCH(x)                	(((x) & BIT_MASK_TXPKT_NUM_MATCH) << BIT_SHIFT_TXPKT_NUM_MATCH)


#define BIT_SHIFT_RXTTIMER_MATCH_NUM          	20
#define BIT_MASK_RXTTIMER_MATCH_NUM           	0xf
#define BIT_RXTTIMER_MATCH_NUM(x)             	(((x) & BIT_MASK_RXTTIMER_MATCH_NUM) << BIT_SHIFT_RXTTIMER_MATCH_NUM)


#define BIT_SHIFT_RXPKT_NUM_MATCH             	16
#define BIT_MASK_RXPKT_NUM_MATCH              	0xf
#define BIT_RXPKT_NUM_MATCH(x)                	(((x) & BIT_MASK_RXPKT_NUM_MATCH) << BIT_SHIFT_RXPKT_NUM_MATCH)


#define BIT_SHIFT_MIGRATE_TIMER               	0
#define BIT_MASK_MIGRATE_TIMER                	0xffff
#define BIT_MIGRATE_TIMER(x)                  	(((x) & BIT_MASK_Migrate_timer) << BIT_SHIFT_Migrate_timer)


//2 REG_BCNQ_TXBD_DESA                     (Offset 0x0308)


#define BIT_SHIFT_BCNQ_TXBD_DESA              	0
#define BIT_MASK_BCNQ_TXBD_DESA               	0xffffffffffffffffL
#define BIT_BCNQ_TXBD_DESA(x)                 	(((x) & BIT_MASK_BCNQ_TXBD_DESA) << BIT_SHIFT_BCNQ_TXBD_DESA)


//2 REG_MGQ_TXBD_DESA                      (Offset 0x0310)


#define BIT_SHIFT_MGQ_TXBD_DESA               	0
#define BIT_MASK_MGQ_TXBD_DESA                	0xffffffffffffffffL
#define BIT_MGQ_TXBD_DESA(x)                  	(((x) & BIT_MASK_MGQ_TXBD_DESA) << BIT_SHIFT_MGQ_TXBD_DESA)


//2 REG_VOQ_TXBD_DESA                      (Offset 0x0318)


#define BIT_SHIFT_VOQ_TXBD_DESA               	0
#define BIT_MASK_VOQ_TXBD_DESA                	0xffffffffffffffffL
#define BIT_VOQ_TXBD_DESA(x)                  	(((x) & BIT_MASK_VOQ_TXBD_DESA) << BIT_SHIFT_VOQ_TXBD_DESA)


//2 REG_VIQ_TXBD_DESA                      (Offset 0x0320)


#define BIT_SHIFT_VIQ_TXBD_DESA               	0
#define BIT_MASK_VIQ_TXBD_DESA                	0xffffffffffffffffL
#define BIT_VIQ_TXBD_DESA(x)                  	(((x) & BIT_MASK_VIQ_TXBD_DESA) << BIT_SHIFT_VIQ_TXBD_DESA)


//2 REG_BEQ_TXBD_DESA                      (Offset 0x0328)


#define BIT_SHIFT_BEQ_TXBD_DESA               	0
#define BIT_MASK_BEQ_TXBD_DESA                	0xffffffffffffffffL
#define BIT_BEQ_TXBD_DESA(x)                  	(((x) & BIT_MASK_BEQ_TXBD_DESA) << BIT_SHIFT_BEQ_TXBD_DESA)


//2 REG_BKQ_TXBD_DESA                      (Offset 0x0330)


#define BIT_SHIFT_BKQ_TXBD_DESA               	0
#define BIT_MASK_BKQ_TXBD_DESA                	0xffffffffffffffffL
#define BIT_BKQ_TXBD_DESA(x)                  	(((x) & BIT_MASK_BKQ_TXBD_DESA) << BIT_SHIFT_BKQ_TXBD_DESA)


//2 REG_RXQ_RXBD_DESA                      (Offset 0x0338)


#define BIT_SHIFT_RXQ_RXBD_DESA               	0
#define BIT_MASK_RXQ_RXBD_DESA                	0xffffffffffffffffL
#define BIT_RXQ_RXBD_DESA(x)                  	(((x) & BIT_MASK_RXQ_RXBD_DESA) << BIT_SHIFT_RXQ_RXBD_DESA)


//2 REG_HI0Q_TXBD_DESA                     (Offset 0x0340)


#define BIT_SHIFT_HI0Q_TXBD_DESA              	0
#define BIT_MASK_HI0Q_TXBD_DESA               	0xffffffffffffffffL
#define BIT_HI0Q_TXBD_DESA(x)                 	(((x) & BIT_MASK_HI0Q_TXBD_DESA) << BIT_SHIFT_HI0Q_TXBD_DESA)


//2 REG_HI1Q_TXBD_DESA                     (Offset 0x0348)


#define BIT_SHIFT_HI1Q_TXBD_DESA              	0
#define BIT_MASK_HI1Q_TXBD_DESA               	0xffffffffffffffffL
#define BIT_HI1Q_TXBD_DESA(x)                 	(((x) & BIT_MASK_HI1Q_TXBD_DESA) << BIT_SHIFT_HI1Q_TXBD_DESA)


//2 REG_HI2Q_TXBD_DESA                     (Offset 0x0350)


#define BIT_SHIFT_HI2Q_TXBD_DESA              	0
#define BIT_MASK_HI2Q_TXBD_DESA               	0xffffffffffffffffL
#define BIT_HI2Q_TXBD_DESA(x)                 	(((x) & BIT_MASK_HI2Q_TXBD_DESA) << BIT_SHIFT_HI2Q_TXBD_DESA)


//2 REG_HI3Q_TXBD_DESA                     (Offset 0x0358)


#define BIT_SHIFT_HI3Q_TXBD_DESA              	0
#define BIT_MASK_HI3Q_TXBD_DESA               	0xffffffffffffffffL
#define BIT_HI3Q_TXBD_DESA(x)                 	(((x) & BIT_MASK_HI3Q_TXBD_DESA) << BIT_SHIFT_HI3Q_TXBD_DESA)


//2 REG_HI4Q_TXBD_DESA                     (Offset 0x0360)


#define BIT_SHIFT_HI4Q_TXBD_DESA              	0
#define BIT_MASK_HI4Q_TXBD_DESA               	0xffffffffffffffffL
#define BIT_HI4Q_TXBD_DESA(x)                 	(((x) & BIT_MASK_HI4Q_TXBD_DESA) << BIT_SHIFT_HI4Q_TXBD_DESA)


//2 REG_HI5Q_TXBD_DESA                     (Offset 0x0368)


#define BIT_SHIFT_HI5Q_TXBD_DESA              	0
#define BIT_MASK_HI5Q_TXBD_DESA               	0xffffffffffffffffL
#define BIT_HI5Q_TXBD_DESA(x)                 	(((x) & BIT_MASK_HI5Q_TXBD_DESA) << BIT_SHIFT_HI5Q_TXBD_DESA)


//2 REG_HI6Q_TXBD_DESA                     (Offset 0x0370)


#define BIT_SHIFT_HI6Q_TXBD_DESA              	0
#define BIT_MASK_HI6Q_TXBD_DESA               	0xffffffffffffffffL
#define BIT_HI6Q_TXBD_DESA(x)                 	(((x) & BIT_MASK_HI6Q_TXBD_DESA) << BIT_SHIFT_HI6Q_TXBD_DESA)


//2 REG_HI7Q_TXBD_DESA                     (Offset 0x0378)


#define BIT_SHIFT_HI7Q_TXBD_DESA              	0
#define BIT_MASK_HI7Q_TXBD_DESA               	0xffffffffffffffffL
#define BIT_HI7Q_TXBD_DESA(x)                 	(((x) & BIT_MASK_HI7Q_TXBD_DESA) << BIT_SHIFT_HI7Q_TXBD_DESA)


//2 REG_MGQ_TXBD_NUM                       (Offset 0x0380)

#define BIT_PCIE_MGQ_FLAG                     	BIT(14)

#define BIT_SHIFT_MGQ_DESC_MODE               	12
#define BIT_MASK_MGQ_DESC_MODE                	0x3
#define BIT_MGQ_DESC_MODE(x)                  	(((x) & BIT_MASK_MGQ_DESC_MODE) << BIT_SHIFT_MGQ_DESC_MODE)


#define BIT_SHIFT_MGQ_DESC_NUM                	0
#define BIT_MASK_MGQ_DESC_NUM                 	0xfff
#define BIT_MGQ_DESC_NUM(x)                   	(((x) & BIT_MASK_MGQ_DESC_NUM) << BIT_SHIFT_MGQ_DESC_NUM)


//2 REG_RX_RXBD_NUM                        (Offset 0x0382)

#define BIT_SYS_32_64                         	BIT(15)

#define BIT_SHIFT_BCNQ_DESC_MODE              	13
#define BIT_MASK_BCNQ_DESC_MODE               	0x3
#define BIT_BCNQ_DESC_MODE(x)                 	(((x) & BIT_MASK_BCNQ_DESC_MODE) << BIT_SHIFT_BCNQ_DESC_MODE)

#define BIT_PCIE_BCNQ_FLAG                    	BIT(12)

#define BIT_SHIFT_RXQ_DESC_NUM                	0
#define BIT_MASK_RXQ_DESC_NUM                 	0xfff
#define BIT_RXQ_DESC_NUM(x)                   	(((x) & BIT_MASK_RXQ_DESC_NUM) << BIT_SHIFT_RXQ_DESC_NUM)


//2 REG_VOQ_TXBD_NUM                       (Offset 0x0384)

#define BIT_PCIE_VOQ_FLAG                     	BIT(14)

#define BIT_SHIFT_VOQ_DESC_MODE               	12
#define BIT_MASK_VOQ_DESC_MODE                	0x3
#define BIT_VOQ_DESC_MODE(x)                  	(((x) & BIT_MASK_VOQ_DESC_MODE) << BIT_SHIFT_VOQ_DESC_MODE)


#define BIT_SHIFT_VOQ_DESC_NUM                	0
#define BIT_MASK_VOQ_DESC_NUM                 	0xfff
#define BIT_VOQ_DESC_NUM(x)                   	(((x) & BIT_MASK_VOQ_DESC_NUM) << BIT_SHIFT_VOQ_DESC_NUM)


//2 REG_VIQ_TXBD_NUM                       (Offset 0x0386)

#define BIT_PCIE_VIQ_FLAG                     	BIT(14)

#define BIT_SHIFT_VIQ_DESC_MODE               	12
#define BIT_MASK_VIQ_DESC_MODE                	0x3
#define BIT_VIQ_DESC_MODE(x)                  	(((x) & BIT_MASK_VIQ_DESC_MODE) << BIT_SHIFT_VIQ_DESC_MODE)


#define BIT_SHIFT_VIQ_DESC_NUM                	0
#define BIT_MASK_VIQ_DESC_NUM                 	0xfff
#define BIT_VIQ_DESC_NUM(x)                   	(((x) & BIT_MASK_VIQ_DESC_NUM) << BIT_SHIFT_VIQ_DESC_NUM)


//2 REG_BEQ_TXBD_NUM                       (Offset 0x0388)

#define BIT_PCIE_BEQ_FLAG                     	BIT(14)

#define BIT_SHIFT_BEQ_DESC_MODE               	12
#define BIT_MASK_BEQ_DESC_MODE                	0x3
#define BIT_BEQ_DESC_MODE(x)                  	(((x) & BIT_MASK_BEQ_DESC_MODE) << BIT_SHIFT_BEQ_DESC_MODE)


#define BIT_SHIFT_BEQ_DESC_NUM                	0
#define BIT_MASK_BEQ_DESC_NUM                 	0xfff
#define BIT_BEQ_DESC_NUM(x)                   	(((x) & BIT_MASK_BEQ_DESC_NUM) << BIT_SHIFT_BEQ_DESC_NUM)


//2 REG_BKQ_TXBD_NUM                       (Offset 0x038A)

#define BIT_PCIE_BKQ_FLAG                     	BIT(14)

#define BIT_SHIFT_BKQ_DESC_MODE               	12
#define BIT_MASK_BKQ_DESC_MODE                	0x3
#define BIT_BKQ_DESC_MODE(x)                  	(((x) & BIT_MASK_BKQ_DESC_MODE) << BIT_SHIFT_BKQ_DESC_MODE)


#define BIT_SHIFT_BKQ_DESC_NUM                	0
#define BIT_MASK_BKQ_DESC_NUM                 	0xfff
#define BIT_BKQ_DESC_NUM(x)                   	(((x) & BIT_MASK_BKQ_DESC_NUM) << BIT_SHIFT_BKQ_DESC_NUM)


//2 REG_HI0Q_TXBD_NUM                      (Offset 0x038C)

#define BIT_HI0Q_FLAG                         	BIT(14)

#define BIT_SHIFT_HI0Q_DESC_MODE              	12
#define BIT_MASK_HI0Q_DESC_MODE               	0x3
#define BIT_HI0Q_DESC_MODE(x)                 	(((x) & BIT_MASK_HI0Q_DESC_MODE) << BIT_SHIFT_HI0Q_DESC_MODE)


#define BIT_SHIFT_HI0Q_DESC_NUM               	0
#define BIT_MASK_HI0Q_DESC_NUM                	0xfff
#define BIT_HI0Q_DESC_NUM(x)                  	(((x) & BIT_MASK_HI0Q_DESC_NUM) << BIT_SHIFT_HI0Q_DESC_NUM)


//2 REG_HI1Q_TXBD_NUM                      (Offset 0x038E)

#define BIT_HI1Q_FLAG                         	BIT(14)

#define BIT_SHIFT_HI1Q_DESC_MODE              	12
#define BIT_MASK_HI1Q_DESC_MODE               	0x3
#define BIT_HI1Q_DESC_MODE(x)                 	(((x) & BIT_MASK_HI1Q_DESC_MODE) << BIT_SHIFT_HI1Q_DESC_MODE)


#define BIT_SHIFT_HI1Q_DESC_NUM               	0
#define BIT_MASK_HI1Q_DESC_NUM                	0xfff
#define BIT_HI1Q_DESC_NUM(x)                  	(((x) & BIT_MASK_HI1Q_DESC_NUM) << BIT_SHIFT_HI1Q_DESC_NUM)


//2 REG_HI2Q_TXBD_NUM                      (Offset 0x0390)

#define BIT_HI2Q_FLAG                         	BIT(14)

#define BIT_SHIFT_HI2Q_DESC_MODE              	12
#define BIT_MASK_HI2Q_DESC_MODE               	0x3
#define BIT_HI2Q_DESC_MODE(x)                 	(((x) & BIT_MASK_HI2Q_DESC_MODE) << BIT_SHIFT_HI2Q_DESC_MODE)


#define BIT_SHIFT_HI2Q_DESC_NUM               	0
#define BIT_MASK_HI2Q_DESC_NUM                	0xfff
#define BIT_HI2Q_DESC_NUM(x)                  	(((x) & BIT_MASK_HI2Q_DESC_NUM) << BIT_SHIFT_HI2Q_DESC_NUM)


//2 REG_HI3Q_TXBD_NUM                      (Offset 0x0392)

#define BIT_HI3Q_FLAG                         	BIT(14)

#define BIT_SHIFT_HI3Q_DESC_MODE              	12
#define BIT_MASK_HI3Q_DESC_MODE               	0x3
#define BIT_HI3Q_DESC_MODE(x)                 	(((x) & BIT_MASK_HI3Q_DESC_MODE) << BIT_SHIFT_HI3Q_DESC_MODE)


#define BIT_SHIFT_HI3Q_DESC_NUM               	0
#define BIT_MASK_HI3Q_DESC_NUM                	0xfff
#define BIT_HI3Q_DESC_NUM(x)                  	(((x) & BIT_MASK_HI3Q_DESC_NUM) << BIT_SHIFT_HI3Q_DESC_NUM)


//2 REG_HI4Q_TXBD_NUM                      (Offset 0x0394)

#define BIT_HI4Q_FLAG                         	BIT(14)

#define BIT_SHIFT_HI4Q_DESC_MODE              	12
#define BIT_MASK_HI4Q_DESC_MODE               	0x3
#define BIT_HI4Q_DESC_MODE(x)                 	(((x) & BIT_MASK_HI4Q_DESC_MODE) << BIT_SHIFT_HI4Q_DESC_MODE)


#define BIT_SHIFT_HI4Q_DESC_NUM               	0
#define BIT_MASK_HI4Q_DESC_NUM                	0xfff
#define BIT_HI4Q_DESC_NUM(x)                  	(((x) & BIT_MASK_HI4Q_DESC_NUM) << BIT_SHIFT_HI4Q_DESC_NUM)


//2 REG_HI5Q_TXBD_NUM                      (Offset 0x0396)

#define BIT_HI5Q_FLAG                         	BIT(14)

#define BIT_SHIFT_HI5Q_DESC_MODE              	12
#define BIT_MASK_HI5Q_DESC_MODE               	0x3
#define BIT_HI5Q_DESC_MODE(x)                 	(((x) & BIT_MASK_HI5Q_DESC_MODE) << BIT_SHIFT_HI5Q_DESC_MODE)


#define BIT_SHIFT_HI5Q_DESC_NUM               	0
#define BIT_MASK_HI5Q_DESC_NUM                	0xfff
#define BIT_HI5Q_DESC_NUM(x)                  	(((x) & BIT_MASK_HI5Q_DESC_NUM) << BIT_SHIFT_HI5Q_DESC_NUM)


//2 REG_HI6Q_TXBD_NUM                      (Offset 0x0398)

#define BIT_HI6Q_FLAG                         	BIT(14)

#define BIT_SHIFT_HI6Q_DESC_MODE              	12
#define BIT_MASK_HI6Q_DESC_MODE               	0x3
#define BIT_HI6Q_DESC_MODE(x)                 	(((x) & BIT_MASK_HI6Q_DESC_MODE) << BIT_SHIFT_HI6Q_DESC_MODE)


#define BIT_SHIFT_HI6Q_DESC_NUM               	0
#define BIT_MASK_HI6Q_DESC_NUM                	0xfff
#define BIT_HI6Q_DESC_NUM(x)                  	(((x) & BIT_MASK_HI6Q_DESC_NUM) << BIT_SHIFT_HI6Q_DESC_NUM)


//2 REG_HI7Q_TXBD_NUM                      (Offset 0x039A)

#define BIT_HI7Q_FLAG                         	BIT(14)

#define BIT_SHIFT_HI7Q_DESC_MODE              	12
#define BIT_MASK_HI7Q_DESC_MODE               	0x3
#define BIT_HI7Q_DESC_MODE(x)                 	(((x) & BIT_MASK_HI7Q_DESC_MODE) << BIT_SHIFT_HI7Q_DESC_MODE)


#define BIT_SHIFT_HI7Q_DESC_NUM               	0
#define BIT_MASK_HI7Q_DESC_NUM                	0xfff
#define BIT_HI7Q_DESC_NUM(x)                  	(((x) & BIT_MASK_HI7Q_DESC_NUM) << BIT_SHIFT_HI7Q_DESC_NUM)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_HI7Q_HW_IDX                   	BIT(29)
#define BIT_CLR_HI6Q_HW_IDX                   	BIT(28)
#define BIT_CLR_HI5Q_HW_IDX                   	BIT(27)
#define BIT_CLR_HI4Q_HW_IDX                   	BIT(26)
#define BIT_CLR_HI3Q_HW_IDX                   	BIT(25)
#define BIT_CLR_HI2Q_HW_IDX                   	BIT(24)
#define BIT_CLR_HI1Q_HW_IDX                   	BIT(23)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_BCN7DOK                           	BIT(23)
#define BIT_BCN7DOKM                          	BIT(23)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_HI0Q_HW_IDX                   	BIT(22)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_BCN6DOK                           	BIT(22)
#define BIT_BCN6DOKM                          	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_BKQ_HW_IDX                    	BIT(21)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_BCN5DOK                           	BIT(21)
#define BIT_BCN5DOKM                          	BIT(21)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_BEQ_HW_IDX                    	BIT(20)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_BCN4DOK                           	BIT(20)
#define BIT_BCN4DOKM                          	BIT(20)
#define BIT_RX_OVER_RD_ERR                    	BIT(20)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_VIQ_HW_IDX                    	BIT(19)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_BCN3DOK                           	BIT(19)
#define BIT_BCN3DOKM                          	BIT(19)
#define BIT_RXDMA_STUCK                       	BIT(19)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_VOQ_HW_IDX                    	BIT(18)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_BCN2DOK                           	BIT(18)
#define BIT_BCN2DOKM                          	BIT(18)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_MGQ_HW_IDX                    	BIT(17)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_BCN1DOK                           	BIT(17)
#define BIT_BCN1DOKM                          	BIT(17)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TSFTIMER_HCI                       (Offset 0x039C)


#define BIT_SHIFT_TSFT2_HCI                   	16
#define BIT_MASK_TSFT2_HCI                    	0xffff
#define BIT_TSFT2_HCI(x)                      	(((x) & BIT_MASK_TSFT2_HCI) << BIT_SHIFT_TSFT2_HCI)

#define BIT_CLR_RXQ_HW_IDX                    	BIT(16)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_BCN0DOK                           	BIT(16)
#define BIT_BCN0DOKM                          	BIT(16)

#define BIT_SHIFT_RX_STATE                    	16
#define BIT_MASK_RX_STATE                     	0x7
#define BIT_RX_STATE(x)                       	(((x) & BIT_MASK_RX_STATE) << BIT_SHIFT_RX_STATE)

#define BIT_SRST_TX                           	BIT(15)
#define BIT_M7DOK                             	BIT(15)
#define BIT_M7DOKM                            	BIT(15)
#define BIT_TDE_NO_IDLE                       	BIT(15)
#define BIT_SRST_RX                           	BIT(14)
#define BIT_M6DOK                             	BIT(14)
#define BIT_M6DOKM                            	BIT(14)
#define BIT_TXDMA_STUCK                       	BIT(14)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_HI7Q_HOST_IDX                 	BIT(13)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_M5DOK                             	BIT(13)
#define BIT_M5DOKM                            	BIT(13)
#define BIT_TDE_FULL_ERR                      	BIT(13)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_HI6Q_HOST_IDX                 	BIT(12)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_M4DOK                             	BIT(12)
#define BIT_M4DOKM                            	BIT(12)
#define BIT_HD_SIZE_ERR                       	BIT(12)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_HI5Q_HOST_IDX                 	BIT(11)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_M3DOK                             	BIT(11)
#define BIT_M3DOKM                            	BIT(11)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_HI4Q_HOST_IDX                 	BIT(10)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_M2DOK                             	BIT(10)
#define BIT_M2DOKM                            	BIT(10)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_HI3Q_HOST_IDX                 	BIT(9)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_M1DOK                             	BIT(9)
#define BIT_M1DOKM                            	BIT(9)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_HI2Q_HOST_IDX                 	BIT(8)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_M0DOK                             	BIT(8)
#define BIT_M0DOKM                            	BIT(8)

#define BIT_SHIFT_TX_STATE                    	8
#define BIT_MASK_TX_STATE                     	0xf
#define BIT_TX_STATE(x)                       	(((x) & BIT_MASK_TX_STATE) << BIT_SHIFT_TX_STATE)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_HI1Q_HOST_IDX                 	BIT(7)
#define BIT_CLR_HI0Q_HOST_IDX                 	BIT(6)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_MGQDOK                            	BIT(6)
#define BIT_MGQDOKM                           	BIT(6)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_BKQ_HOST_IDX                  	BIT(5)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_BKQDOK                            	BIT(5)
#define BIT_BKQDOKM                           	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_BEQ_HOST_IDX                  	BIT(4)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)


#define BIT_SHIFT_HPS_CLKR                    	4
#define BIT_MASK_HPS_CLKR                     	0x3
#define BIT_HPS_CLKR(x)                       	(((x) & BIT_MASK_HPS_CLKR) << BIT_SHIFT_HPS_CLKR)

#define BIT_BEQDOK                            	BIT(4)
#define BIT_BEQDOKM                           	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_VIQ_HOST_IDX                  	BIT(3)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_LX_INT                            	BIT(3)
#define BIT_VIQDOK                            	BIT(3)
#define BIT_VIQDOKM                           	BIT(3)
#define BIT_MST_BUSY                          	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_VOQ_HOST_IDX                  	BIT(2)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_VOQDOK                            	BIT(2)
#define BIT_VOQDOKM                           	BIT(2)
#define BIT_SLV_BUSY                          	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_CLR_MGQ_HOST_IDX                  	BIT(1)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_RDUM                              	BIT(1)
#define BIT_RXDES_UNAVAIL                     	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TSFTIMER_HCI                       (Offset 0x039C)


#define BIT_SHIFT_TSFT1_HCI                   	0
#define BIT_MASK_TSFT1_HCI                    	0xffff
#define BIT_TSFT1_HCI(x)                      	(((x) & BIT_MASK_TSFT1_HCI) << BIT_SHIFT_TSFT1_HCI)

#define BIT_CLR_RXQ_HOST_IDX                  	BIT(0)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_BD_RWPTR_CLR                       (Offset 0x039C)

#define BIT_RXDOK                             	BIT(0)
#define BIT_RXDOKM                            	BIT(0)
#define BIT_EN_DBG_STUCK                      	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_VOQ_TXBD_IDX                       (Offset 0x03A0)


#define BIT_SHIFT_VOQ_HW_IDX                  	16
#define BIT_MASK_VOQ_HW_IDX                   	0xfff
#define BIT_VOQ_HW_IDX(x)                     	(((x) & BIT_MASK_VOQ_HW_IDX) << BIT_SHIFT_VOQ_HW_IDX)


#define BIT_SHIFT_VOQ_HOST_IDX                	0
#define BIT_MASK_VOQ_HOST_IDX                 	0xfff
#define BIT_VOQ_HOST_IDX(x)                   	(((x) & BIT_MASK_VOQ_HOST_IDX) << BIT_SHIFT_VOQ_HOST_IDX)


//2 REG_VIQ_TXBD_IDX                       (Offset 0x03A4)


#define BIT_SHIFT_VIQ_HW_IDX                  	16
#define BIT_MASK_VIQ_HW_IDX                   	0xfff
#define BIT_VIQ_HW_IDX(x)                     	(((x) & BIT_MASK_VIQ_HW_IDX) << BIT_SHIFT_VIQ_HW_IDX)


#define BIT_SHIFT_VIQ_HOST_IDX                	0
#define BIT_MASK_VIQ_HOST_IDX                 	0xfff
#define BIT_VIQ_HOST_IDX(x)                   	(((x) & BIT_MASK_VIQ_HOST_IDX) << BIT_SHIFT_VIQ_HOST_IDX)


//2 REG_BEQ_TXBD_IDX                       (Offset 0x03A8)


#define BIT_SHIFT_BEQ_HW_IDX                  	16
#define BIT_MASK_BEQ_HW_IDX                   	0xfff
#define BIT_BEQ_HW_IDX(x)                     	(((x) & BIT_MASK_BEQ_HW_IDX) << BIT_SHIFT_BEQ_HW_IDX)


#define BIT_SHIFT_BEQ_HOST_IDX                	0
#define BIT_MASK_BEQ_HOST_IDX                 	0xfff
#define BIT_BEQ_HOST_IDX(x)                   	(((x) & BIT_MASK_BEQ_HOST_IDX) << BIT_SHIFT_BEQ_HOST_IDX)


//2 REG_BKQ_TXBD_IDX                       (Offset 0x03AC)


#define BIT_SHIFT_BKQ_HW_IDX                  	16
#define BIT_MASK_BKQ_HW_IDX                   	0xfff
#define BIT_BKQ_HW_IDX(x)                     	(((x) & BIT_MASK_BKQ_HW_IDX) << BIT_SHIFT_BKQ_HW_IDX)


#define BIT_SHIFT_BKQ_HOST_IDX                	0
#define BIT_MASK_BKQ_HOST_IDX                 	0xfff
#define BIT_BKQ_HOST_IDX(x)                   	(((x) & BIT_MASK_BKQ_HOST_IDX) << BIT_SHIFT_BKQ_HOST_IDX)


//2 REG_MGQ_TXBD_IDX                       (Offset 0x03B0)


#define BIT_SHIFT_MGQ_HW_IDX                  	16
#define BIT_MASK_MGQ_HW_IDX                   	0xfff
#define BIT_MGQ_HW_IDX(x)                     	(((x) & BIT_MASK_MGQ_HW_IDX) << BIT_SHIFT_MGQ_HW_IDX)


#define BIT_SHIFT_MGQ_HOST_IDX                	0
#define BIT_MASK_MGQ_HOST_IDX                 	0xfff
#define BIT_MGQ_HOST_IDX(x)                   	(((x) & BIT_MASK_MGQ_HOST_IDX) << BIT_SHIFT_MGQ_HOST_IDX)


//2 REG_RXQ_RXBD_IDX                       (Offset 0x03B4)


#define BIT_SHIFT_RXQ_HW_IDX                  	16
#define BIT_MASK_RXQ_HW_IDX                   	0xfff
#define BIT_RXQ_HW_IDX(x)                     	(((x) & BIT_MASK_RXQ_HW_IDX) << BIT_SHIFT_RXQ_HW_IDX)


#define BIT_SHIFT_RXQ_HOST_IDX                	0
#define BIT_MASK_RXQ_HOST_IDX                 	0xfff
#define BIT_RXQ_HOST_IDX(x)                   	(((x) & BIT_MASK_RXQ_HOST_IDX) << BIT_SHIFT_RXQ_HOST_IDX)


//2 REG_HI0Q_TXBD_IDX                      (Offset 0x03B8)


#define BIT_SHIFT_HI0Q_HW_IDX                 	16
#define BIT_MASK_HI0Q_HW_IDX                  	0xfff
#define BIT_HI0Q_HW_IDX(x)                    	(((x) & BIT_MASK_HI0Q_HW_IDX) << BIT_SHIFT_HI0Q_HW_IDX)


#define BIT_SHIFT_HI0Q_HOST_IDX               	0
#define BIT_MASK_HI0Q_HOST_IDX                	0xfff
#define BIT_HI0Q_HOST_IDX(x)                  	(((x) & BIT_MASK_HI0Q_HOST_IDX) << BIT_SHIFT_HI0Q_HOST_IDX)


//2 REG_HI1Q_TXBD_IDX                      (Offset 0x03BC)


#define BIT_SHIFT_HI1Q_HW_IDX                 	16
#define BIT_MASK_HI1Q_HW_IDX                  	0xfff
#define BIT_HI1Q_HW_IDX(x)                    	(((x) & BIT_MASK_HI1Q_HW_IDX) << BIT_SHIFT_HI1Q_HW_IDX)


#define BIT_SHIFT_HI1Q_HOST_IDX               	0
#define BIT_MASK_HI1Q_HOST_IDX                	0xfff
#define BIT_HI1Q_HOST_IDX(x)                  	(((x) & BIT_MASK_HI1Q_HOST_IDX) << BIT_SHIFT_HI1Q_HOST_IDX)


//2 REG_HI2Q_TXBD_IDX                      (Offset 0x03C0)


#define BIT_SHIFT_HI2Q_HW_IDX                 	16
#define BIT_MASK_HI2Q_HW_IDX                  	0xfff
#define BIT_HI2Q_HW_IDX(x)                    	(((x) & BIT_MASK_HI2Q_HW_IDX) << BIT_SHIFT_HI2Q_HW_IDX)


#define BIT_SHIFT_HI2Q_HOST_IDX               	0
#define BIT_MASK_HI2Q_HOST_IDX                	0xfff
#define BIT_HI2Q_HOST_IDX(x)                  	(((x) & BIT_MASK_HI2Q_HOST_IDX) << BIT_SHIFT_HI2Q_HOST_IDX)


//2 REG_HI3Q_TXBD_IDX                      (Offset 0x03C4)


#define BIT_SHIFT_HI3Q_HW_IDX                 	16
#define BIT_MASK_HI3Q_HW_IDX                  	0xfff
#define BIT_HI3Q_HW_IDX(x)                    	(((x) & BIT_MASK_HI3Q_HW_IDX) << BIT_SHIFT_HI3Q_HW_IDX)


#define BIT_SHIFT_HI3Q_HOST_IDX               	0
#define BIT_MASK_HI3Q_HOST_IDX                	0xfff
#define BIT_HI3Q_HOST_IDX(x)                  	(((x) & BIT_MASK_HI3Q_HOST_IDX) << BIT_SHIFT_HI3Q_HOST_IDX)


//2 REG_HI4Q_TXBD_IDX                      (Offset 0x03C8)


#define BIT_SHIFT_HI4Q_HW_IDX                 	16
#define BIT_MASK_HI4Q_HW_IDX                  	0xfff
#define BIT_HI4Q_HW_IDX(x)                    	(((x) & BIT_MASK_HI4Q_HW_IDX) << BIT_SHIFT_HI4Q_HW_IDX)


#define BIT_SHIFT_HI4Q_HOST_IDX               	0
#define BIT_MASK_HI4Q_HOST_IDX                	0xfff
#define BIT_HI4Q_HOST_IDX(x)                  	(((x) & BIT_MASK_HI4Q_HOST_IDX) << BIT_SHIFT_HI4Q_HOST_IDX)


//2 REG_HI5Q_TXBD_IDX                      (Offset 0x03CC)


#define BIT_SHIFT_HI5Q_HW_IDX                 	16
#define BIT_MASK_HI5Q_HW_IDX                  	0xfff
#define BIT_HI5Q_HW_IDX(x)                    	(((x) & BIT_MASK_HI5Q_HW_IDX) << BIT_SHIFT_HI5Q_HW_IDX)


#define BIT_SHIFT_HI5Q_HOST_IDX               	0
#define BIT_MASK_HI5Q_HOST_IDX                	0xfff
#define BIT_HI5Q_HOST_IDX(x)                  	(((x) & BIT_MASK_HI5Q_HOST_IDX) << BIT_SHIFT_HI5Q_HOST_IDX)


//2 REG_HI6Q_TXBD_IDX                      (Offset 0x03D0)


#define BIT_SHIFT_HI6Q_HW_IDX                 	16
#define BIT_MASK_HI6Q_HW_IDX                  	0xfff
#define BIT_HI6Q_HW_IDX(x)                    	(((x) & BIT_MASK_HI6Q_HW_IDX) << BIT_SHIFT_HI6Q_HW_IDX)


#define BIT_SHIFT_HI6Q_HOST_IDX               	0
#define BIT_MASK_HI6Q_HOST_IDX                	0xfff
#define BIT_HI6Q_HOST_IDX(x)                  	(((x) & BIT_MASK_HI6Q_HOST_IDX) << BIT_SHIFT_HI6Q_HOST_IDX)


//2 REG_HI7Q_TXBD_IDX                      (Offset 0x03D4)


#define BIT_SHIFT_HI7Q_HW_IDX                 	16
#define BIT_MASK_HI7Q_HW_IDX                  	0xfff
#define BIT_HI7Q_HW_IDX(x)                    	(((x) & BIT_MASK_HI7Q_HW_IDX) << BIT_SHIFT_HI7Q_HW_IDX)


#define BIT_SHIFT_HI7Q_HOST_IDX               	0
#define BIT_MASK_HI7Q_HOST_IDX                	0xfff
#define BIT_HI7Q_HOST_IDX(x)                  	(((x) & BIT_MASK_HI7Q_HOST_IDX) << BIT_SHIFT_HI7Q_HOST_IDX)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT)


//2 REG_DBG_SEL_V1                         (Offset 0x03D8)


#define BIT_SHIFT_DBG_SEL                     	0
#define BIT_MASK_DBG_SEL                      	0xff
#define BIT_DBG_SEL(x)                        	(((x) & BIT_MASK_DBG_SEL) << BIT_SHIFT_DBG_SEL)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PCIE_HRPWM1_V1                     (Offset 0x03D9)


#define BIT_SHIFT_PCIE_HRPWM                  	0
#define BIT_MASK_PCIE_HRPWM                   	0xff
#define BIT_PCIE_HRPWM(x)                     	(((x) & BIT_MASK_PCIE_HRPWM) << BIT_SHIFT_PCIE_HRPWM)


//2 REG_PCIE_HCPWM1_V1                     (Offset 0x03DA)


#define BIT_SHIFT_PCIE_HCPWM                  	0
#define BIT_MASK_PCIE_HCPWM                   	0xff
#define BIT_PCIE_HCPWM(x)                     	(((x) & BIT_MASK_PCIE_HCPWM) << BIT_SHIFT_PCIE_HCPWM)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT)


//2 REG_PCIE_CTRL2                         (Offset 0x03DB)

#define BIT_DIS_TXDMA_PRE                     	BIT(7)
#define BIT_DIS_RXDMA_PRE                     	BIT(6)

#define BIT_SHIFT_HPS_CLKR_PCIE               	4
#define BIT_MASK_HPS_CLKR_PCIE                	0x3
#define BIT_HPS_CLKR_PCIE(x)                  	(((x) & BIT_MASK_HPS_CLKR_PCIE) << BIT_SHIFT_HPS_CLKR_PCIE)

#define BIT_PCIE_INT                          	BIT(3)
#define BIT_TXFLAG_EXIT_L1_EN                 	BIT(2)
#define BIT_EN_RXDMA_ALIGN                    	BIT(1)
#define BIT_EN_TXDMA_ALIGN                    	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PCIE_HRPWM2_V1                     (Offset 0x03DC)


#define BIT_SHIFT_PCIE_HRPWM2                 	0
#define BIT_MASK_PCIE_HRPWM2                  	0xffff
#define BIT_PCIE_HRPWM2(x)                    	(((x) & BIT_MASK_PCIE_HRPWM2) << BIT_SHIFT_PCIE_HRPWM2)


//2 REG_PCIE_HCPWM2_V1                     (Offset 0x03DE)


#define BIT_SHIFT_PCIE_HCPWM2                 	0
#define BIT_MASK_PCIE_HCPWM2                  	0xffff
#define BIT_PCIE_HCPWM2(x)                    	(((x) & BIT_MASK_PCIE_HCPWM2) << BIT_SHIFT_PCIE_HCPWM2)


//2 REG_PCIE_H2C_MSG_V1                    (Offset 0x03E0)


#define BIT_SHIFT_DRV2FW_INFO                 	0
#define BIT_MASK_DRV2FW_INFO                  	0xffffffffL
#define BIT_DRV2FW_INFO(x)                    	(((x) & BIT_MASK_DRV2FW_INFO) << BIT_SHIFT_DRV2FW_INFO)


//2 REG_PCIE_C2H_MSG_V1                    (Offset 0x03E4)


#define BIT_SHIFT_HCI_PCIE_C2H_MSG            	0
#define BIT_MASK_HCI_PCIE_C2H_MSG             	0xffffffffL
#define BIT_HCI_PCIE_C2H_MSG(x)               	(((x) & BIT_MASK_HCI_PCIE_C2H_MSG) << BIT_SHIFT_HCI_PCIE_C2H_MSG)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT)


//2 REG_DBI_WDATA_V1                       (Offset 0x03E8)


#define BIT_SHIFT_DBI_WDATA                   	0
#define BIT_MASK_DBI_WDATA                    	0xffffffffL
#define BIT_DBI_WDATA(x)                      	(((x) & BIT_MASK_DBI_WDATA) << BIT_SHIFT_DBI_WDATA)


//2 REG_DBI_RDATA_V1                       (Offset 0x03EC)


#define BIT_SHIFT_DBI_RDATA                   	0
#define BIT_MASK_DBI_RDATA                    	0xffffffffL
#define BIT_DBI_RDATA(x)                      	(((x) & BIT_MASK_DBI_RDATA) << BIT_SHIFT_DBI_RDATA)


//2 REG_DBI_FLAG_V1                        (Offset 0x03F0)

#define BIT_EN_STUCK_DBG                      	BIT(26)
#define BIT_RX_STUCK                          	BIT(25)
#define BIT_TX_STUCK                          	BIT(24)
#define BIT_DBI_RFLAG                         	BIT(17)
#define BIT_DBI_WFLAG                         	BIT(16)

#define BIT_SHIFT_DBI_WREN                    	12
#define BIT_MASK_DBI_WREN                     	0xf
#define BIT_DBI_WREN(x)                       	(((x) & BIT_MASK_DBI_WREN) << BIT_SHIFT_DBI_WREN)


#define BIT_SHIFT_DBI_ADDR                    	0
#define BIT_MASK_DBI_ADDR                     	0xfff
#define BIT_DBI_ADDR(x)                       	(((x) & BIT_MASK_DBI_ADDR) << BIT_SHIFT_DBI_ADDR)


//2 REG_MDIO_V1                            (Offset 0x03F4)

#define BIT_ECRC_EN                           	BIT(39)
#define BIT_MDIO_RFLAG                        	BIT(38)
#define BIT_MDIO_WFLAG                        	BIT(37)

#define BIT_SHIFT_MDIO_ADDR                   	(32 & CPU_OPT_WIDTH)
#define BIT_MASK_MDIO_ADDR                    	0x1f
#define BIT_MDIO_ADDR(x)                      	(((x) & BIT_MASK_MDIO_ADDR) << BIT_SHIFT_MDIO_ADDR)


#define BIT_SHIFT_MDIO_RDATA                  	16
#define BIT_MASK_MDIO_RDATA                   	0xffff
#define BIT_MDIO_RDATA(x)                     	(((x) & BIT_MASK_MDIO_RDATA) << BIT_SHIFT_MDIO_RDATA)


#define BIT_SHIFT_MDIO_WDATA                  	0
#define BIT_MASK_MDIO_WDATA                   	0xffff
#define BIT_MDIO_WDATA(x)                     	(((x) & BIT_MASK_MDIO_WDATA) << BIT_SHIFT_MDIO_WDATA)


#endif


#if (RTL8881A_SUPPORT)


//2 REG_BUS_MIX_CFG                        (Offset 0x03F8)


#define BIT_SHIFT_DELAY_TIME                  	24
#define BIT_MASK_DELAY_TIME                   	0xff
#define BIT_DELAY_TIME(x)                     	(((x) & BIT_MASK_DELAY_TIME) << BIT_SHIFT_DELAY_TIME)

#define BIT_RX_TIMER_DELAY_EN                 	BIT(17)

#define BIT_SHIFT_WATCH_DOG_RECORD_V1         	10
#define BIT_MASK_WATCH_DOG_RECORD_V1          	0x3fff
#define BIT_WATCH_DOG_RECORD_V1(x)            	(((x) & BIT_MASK_WATCH_DOG_RECORD_V1) << BIT_SHIFT_WATCH_DOG_RECORD_V1)

#define BIT_R_IO_TIMEOUT_FLAG_V1              	BIT(9)
#define BIT_EN_WATCH_DOG_V1                   	BIT(8)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_HCI_MIX_CFG                        (Offset 0x03FC)

#define BIT_HOST_GEN2_SUPPORT                 	BIT(21)

#define BIT_SHIFT_TXDMA_ERR_FLAG              	16
#define BIT_MASK_TXDMA_ERR_FLAG               	0xf
#define BIT_TXDMA_ERR_FLAG(x)                 	(((x) & BIT_MASK_TXDMA_ERR_FLAG) << BIT_SHIFT_TXDMA_ERR_FLAG)


#define BIT_SHIFT_EARLY_MODE_SEL              	12
#define BIT_MASK_EARLY_MODE_SEL               	0xf
#define BIT_EARLY_MODE_SEL(x)                 	(((x) & BIT_MASK_EARLY_MODE_SEL) << BIT_SHIFT_EARLY_MODE_SEL)

#define BIT_EPHY_RX50_EN                      	BIT(11)

#define BIT_SHIFT_MSI_TIMEOUT_ID_V1           	8
#define BIT_MASK_MSI_TIMEOUT_ID_V1            	0x7
#define BIT_MSI_TIMEOUT_ID_V1(x)              	(((x) & BIT_MASK_MSI_TIMEOUT_ID_V1) << BIT_SHIFT_MSI_TIMEOUT_ID_V1)

#define BIT_RADDR_RD                          	BIT(7)
#define BIT_EN_MUL_TAG                        	BIT(6)
#define BIT_EN_EARLY_MODE                     	BIT(5)
#define BIT_L0S_LINK_OFF                      	BIT(4)
#define BIT_ACT_LINK_OFF                      	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q0_INFO                            (Offset 0x0400)


#define BIT_SHIFT_QueueMacID_Q0_V1            	25
#define BIT_MASK_QueueMacID_Q0_V1             	0x7f
#define BIT_QueueMacID_Q0_V1(x)               	(((x) & BIT_MASK_QueueMacID_Q0_V1) << BIT_SHIFT_QueueMacID_Q0_V1)


#define BIT_SHIFT_QueueAC_Q0_V1               	23
#define BIT_MASK_QueueAC_Q0_V1                	0x3
#define BIT_QueueAC_Q0_V1(x)                  	(((x) & BIT_MASK_QueueAC_Q0_V1) << BIT_SHIFT_QueueAC_Q0_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q0_INFO                            (Offset 0x0400)

#define BIT_TidEmpty_Q0_V1                    	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q0_INFO                            (Offset 0x0400)


#define BIT_SHIFT_Tail_PKT_Q0_V1              	15
#define BIT_MASK_Tail_PKT_Q0_V1               	0xff
#define BIT_Tail_PKT_Q0_V1(x)                 	(((x) & BIT_MASK_Tail_PKT_Q0_V1) << BIT_SHIFT_Tail_PKT_Q0_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q0_INFO                            (Offset 0x0400)


#define BIT_SHIFT_Tail_PKT_Q0_V2              	11
#define BIT_MASK_Tail_PKT_Q0_V2               	0x7ff
#define BIT_Tail_PKT_Q0_V2(x)                 	(((x) & BIT_MASK_Tail_PKT_Q0_V2) << BIT_SHIFT_Tail_PKT_Q0_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q0_INFO                            (Offset 0x0400)


#define BIT_SHIFT_PKT_Num_Q0_V1               	8
#define BIT_MASK_PKT_Num_Q0_V1                	0x7f
#define BIT_PKT_Num_Q0_V1(x)                  	(((x) & BIT_MASK_PKT_Num_Q0_V1) << BIT_SHIFT_PKT_Num_Q0_V1)


#define BIT_SHIFT_Head_PKT_Q0                 	0
#define BIT_MASK_Head_PKT_Q0                  	0xff
#define BIT_Head_PKT_Q0(x)                    	(((x) & BIT_MASK_Head_PKT_Q0) << BIT_SHIFT_Head_PKT_Q0)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q0_INFO                            (Offset 0x0400)


#define BIT_SHIFT_Head_PKT_Q0_V1              	0
#define BIT_MASK_Head_PKT_Q0_V1               	0x7ff
#define BIT_Head_PKT_Q0_V1(x)                 	(((x) & BIT_MASK_Head_PKT_Q0_V1) << BIT_SHIFT_Head_PKT_Q0_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q1_INFO                            (Offset 0x0404)


#define BIT_SHIFT_QueueMacID_Q1_V1            	25
#define BIT_MASK_QueueMacID_Q1_V1             	0x7f
#define BIT_QueueMacID_Q1_V1(x)               	(((x) & BIT_MASK_QueueMacID_Q1_V1) << BIT_SHIFT_QueueMacID_Q1_V1)


#define BIT_SHIFT_QueueAC_Q1_V1               	23
#define BIT_MASK_QueueAC_Q1_V1                	0x3
#define BIT_QueueAC_Q1_V1(x)                  	(((x) & BIT_MASK_QueueAC_Q1_V1) << BIT_SHIFT_QueueAC_Q1_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q1_INFO                            (Offset 0x0404)

#define BIT_TidEmpty_Q1_V1                    	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q1_INFO                            (Offset 0x0404)


#define BIT_SHIFT_Tail_PKT_Q1_V1              	15
#define BIT_MASK_Tail_PKT_Q1_V1               	0xff
#define BIT_Tail_PKT_Q1_V1(x)                 	(((x) & BIT_MASK_Tail_PKT_Q1_V1) << BIT_SHIFT_Tail_PKT_Q1_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q1_INFO                            (Offset 0x0404)


#define BIT_SHIFT_Tail_PKT_Q1_V2              	11
#define BIT_MASK_Tail_PKT_Q1_V2               	0x7ff
#define BIT_Tail_PKT_Q1_V2(x)                 	(((x) & BIT_MASK_Tail_PKT_Q1_V2) << BIT_SHIFT_Tail_PKT_Q1_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q1_INFO                            (Offset 0x0404)


#define BIT_SHIFT_PKT_Num_Q1_V1               	8
#define BIT_MASK_PKT_Num_Q1_V1                	0x7f
#define BIT_PKT_Num_Q1_V1(x)                  	(((x) & BIT_MASK_PKT_Num_Q1_V1) << BIT_SHIFT_PKT_Num_Q1_V1)


#define BIT_SHIFT_Head_PKT_Q1                 	0
#define BIT_MASK_Head_PKT_Q1                  	0xff
#define BIT_Head_PKT_Q1(x)                    	(((x) & BIT_MASK_Head_PKT_Q1) << BIT_SHIFT_Head_PKT_Q1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q1_INFO                            (Offset 0x0404)


#define BIT_SHIFT_Head_PKT_Q1_V1              	0
#define BIT_MASK_Head_PKT_Q1_V1               	0x7ff
#define BIT_Head_PKT_Q1_V1(x)                 	(((x) & BIT_MASK_Head_PKT_Q1_V1) << BIT_SHIFT_Head_PKT_Q1_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q2_INFO                            (Offset 0x0408)


#define BIT_SHIFT_QueueMacID_Q2_V1            	25
#define BIT_MASK_QueueMacID_Q2_V1             	0x7f
#define BIT_QueueMacID_Q2_V1(x)               	(((x) & BIT_MASK_QueueMacID_Q2_V1) << BIT_SHIFT_QueueMacID_Q2_V1)


#define BIT_SHIFT_QueueAC_Q2_V1               	23
#define BIT_MASK_QueueAC_Q2_V1                	0x3
#define BIT_QueueAC_Q2_V1(x)                  	(((x) & BIT_MASK_QueueAC_Q2_V1) << BIT_SHIFT_QueueAC_Q2_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q2_INFO                            (Offset 0x0408)

#define BIT_TidEmpty_Q2_V1                    	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q2_INFO                            (Offset 0x0408)


#define BIT_SHIFT_Tail_PKT_Q2_V1              	15
#define BIT_MASK_Tail_PKT_Q2_V1               	0xff
#define BIT_Tail_PKT_Q2_V1(x)                 	(((x) & BIT_MASK_Tail_PKT_Q2_V1) << BIT_SHIFT_Tail_PKT_Q2_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q2_INFO                            (Offset 0x0408)


#define BIT_SHIFT_Tail_PKT_Q2_V2              	11
#define BIT_MASK_Tail_PKT_Q2_V2               	0x7ff
#define BIT_Tail_PKT_Q2_V2(x)                 	(((x) & BIT_MASK_Tail_PKT_Q2_V2) << BIT_SHIFT_Tail_PKT_Q2_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q2_INFO                            (Offset 0x0408)


#define BIT_SHIFT_PKT_Num_Q2_V1               	8
#define BIT_MASK_PKT_Num_Q2_V1                	0x7f
#define BIT_PKT_Num_Q2_V1(x)                  	(((x) & BIT_MASK_PKT_Num_Q2_V1) << BIT_SHIFT_PKT_Num_Q2_V1)


#define BIT_SHIFT_Head_PKT_Q2                 	0
#define BIT_MASK_Head_PKT_Q2                  	0xff
#define BIT_Head_PKT_Q2(x)                    	(((x) & BIT_MASK_Head_PKT_Q2) << BIT_SHIFT_Head_PKT_Q2)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q2_INFO                            (Offset 0x0408)


#define BIT_SHIFT_Head_PKT_Q2_V1              	0
#define BIT_MASK_Head_PKT_Q2_V1               	0x7ff
#define BIT_Head_PKT_Q2_V1(x)                 	(((x) & BIT_MASK_Head_PKT_Q2_V1) << BIT_SHIFT_Head_PKT_Q2_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q3_INFO                            (Offset 0x040C)


#define BIT_SHIFT_QueueMacID_Q3_V1            	25
#define BIT_MASK_QueueMacID_Q3_V1             	0x7f
#define BIT_QueueMacID_Q3_V1(x)               	(((x) & BIT_MASK_QueueMacID_Q3_V1) << BIT_SHIFT_QueueMacID_Q3_V1)


#define BIT_SHIFT_QueueAC_Q3_V1               	23
#define BIT_MASK_QueueAC_Q3_V1                	0x3
#define BIT_QueueAC_Q3_V1(x)                  	(((x) & BIT_MASK_QueueAC_Q3_V1) << BIT_SHIFT_QueueAC_Q3_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q3_INFO                            (Offset 0x040C)

#define BIT_TidEmpty_Q3_V1                    	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q3_INFO                            (Offset 0x040C)


#define BIT_SHIFT_Tail_PKT_Q3_V1              	15
#define BIT_MASK_Tail_PKT_Q3_V1               	0xff
#define BIT_Tail_PKT_Q3_V1(x)                 	(((x) & BIT_MASK_Tail_PKT_Q3_V1) << BIT_SHIFT_Tail_PKT_Q3_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q3_INFO                            (Offset 0x040C)


#define BIT_SHIFT_Tail_PKT_Q3_V2              	11
#define BIT_MASK_Tail_PKT_Q3_V2               	0x7ff
#define BIT_Tail_PKT_Q3_V2(x)                 	(((x) & BIT_MASK_Tail_PKT_Q3_V2) << BIT_SHIFT_Tail_PKT_Q3_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q3_INFO                            (Offset 0x040C)


#define BIT_SHIFT_PKT_Num_Q3_V1               	8
#define BIT_MASK_PKT_Num_Q3_V1                	0x7f
#define BIT_PKT_Num_Q3_V1(x)                  	(((x) & BIT_MASK_PKT_Num_Q3_V1) << BIT_SHIFT_PKT_Num_Q3_V1)


#define BIT_SHIFT_Head_PKT_Q3                 	0
#define BIT_MASK_Head_PKT_Q3                  	0xff
#define BIT_Head_PKT_Q3(x)                    	(((x) & BIT_MASK_Head_PKT_Q3) << BIT_SHIFT_Head_PKT_Q3)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q3_INFO                            (Offset 0x040C)


#define BIT_SHIFT_Head_PKT_Q3_V1              	0
#define BIT_MASK_Head_PKT_Q3_V1               	0x7ff
#define BIT_Head_PKT_Q3_V1(x)                 	(((x) & BIT_MASK_Head_PKT_Q3_V1) << BIT_SHIFT_Head_PKT_Q3_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_MGQ_INFO                           (Offset 0x0410)


#define BIT_SHIFT_QueueMacID_MGQ_V1           	25
#define BIT_MASK_QueueMacID_MGQ_V1            	0x7f
#define BIT_QueueMacID_MGQ_V1(x)              	(((x) & BIT_MASK_QueueMacID_MGQ_V1) << BIT_SHIFT_QueueMacID_MGQ_V1)


#define BIT_SHIFT_QueueAC_MGQ_V1              	23
#define BIT_MASK_QueueAC_MGQ_V1               	0x3
#define BIT_QueueAC_MGQ_V1(x)                 	(((x) & BIT_MASK_QueueAC_MGQ_V1) << BIT_SHIFT_QueueAC_MGQ_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_MGQ_INFO                           (Offset 0x0410)

#define BIT_TidEmpty_MGQ_V1                   	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_MGQ_INFO                           (Offset 0x0410)


#define BIT_SHIFT_Tail_PKT_MGQ_V1             	15
#define BIT_MASK_Tail_PKT_MGQ_V1              	0xff
#define BIT_Tail_PKT_MGQ_V1(x)                	(((x) & BIT_MASK_Tail_PKT_MGQ_V1) << BIT_SHIFT_Tail_PKT_MGQ_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_MGQ_INFO                           (Offset 0x0410)


#define BIT_SHIFT_Tail_PKT_MGQ_V2             	11
#define BIT_MASK_Tail_PKT_MGQ_V2              	0x7ff
#define BIT_Tail_PKT_MGQ_V2(x)                	(((x) & BIT_MASK_Tail_PKT_MGQ_V2) << BIT_SHIFT_Tail_PKT_MGQ_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_MGQ_INFO                           (Offset 0x0410)


#define BIT_SHIFT_PKT_Num_MGQ_V1              	8
#define BIT_MASK_PKT_Num_MGQ_V1               	0x7f
#define BIT_PKT_Num_MGQ_V1(x)                 	(((x) & BIT_MASK_PKT_Num_MGQ_V1) << BIT_SHIFT_PKT_Num_MGQ_V1)


#define BIT_SHIFT_Head_PKT_MGQ                	0
#define BIT_MASK_Head_PKT_MGQ                 	0xff
#define BIT_Head_PKT_MGQ(x)                   	(((x) & BIT_MASK_Head_PKT_MGQ) << BIT_SHIFT_Head_PKT_MGQ)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_MGQ_INFO                           (Offset 0x0410)


#define BIT_SHIFT_Head_PKT_MGQ_V1             	0
#define BIT_MASK_Head_PKT_MGQ_V1              	0x7ff
#define BIT_Head_PKT_MGQ_V1(x)                	(((x) & BIT_MASK_Head_PKT_MGQ_V1) << BIT_SHIFT_Head_PKT_MGQ_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIQ_INFO                           (Offset 0x0414)


#define BIT_SHIFT_QueueMacID_HIQ_V1           	25
#define BIT_MASK_QueueMacID_HIQ_V1            	0x7f
#define BIT_QueueMacID_HIQ_V1(x)              	(((x) & BIT_MASK_QueueMacID_HIQ_V1) << BIT_SHIFT_QueueMacID_HIQ_V1)


#define BIT_SHIFT_QueueAC_HIQ_V1              	23
#define BIT_MASK_QueueAC_HIQ_V1               	0x3
#define BIT_QueueAC_HIQ_V1(x)                 	(((x) & BIT_MASK_QueueAC_HIQ_V1) << BIT_SHIFT_QueueAC_HIQ_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_HIQ_INFO                           (Offset 0x0414)

#define BIT_TidEmpty_HIQ_V1                   	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIQ_INFO                           (Offset 0x0414)


#define BIT_SHIFT_Tail_PKT_HIQ_V1             	15
#define BIT_MASK_Tail_PKT_HIQ_V1              	0xff
#define BIT_Tail_PKT_HIQ_V1(x)                	(((x) & BIT_MASK_Tail_PKT_HIQ_V1) << BIT_SHIFT_Tail_PKT_HIQ_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_HIQ_INFO                           (Offset 0x0414)


#define BIT_SHIFT_Tail_PKT_HIQ_V2             	11
#define BIT_MASK_Tail_PKT_HIQ_V2              	0x7ff
#define BIT_Tail_PKT_HIQ_V2(x)                	(((x) & BIT_MASK_Tail_PKT_HIQ_V2) << BIT_SHIFT_Tail_PKT_HIQ_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HIQ_INFO                           (Offset 0x0414)


#define BIT_SHIFT_PKT_Num_HIQ_V1              	8
#define BIT_MASK_PKT_Num_HIQ_V1               	0x7f
#define BIT_PKT_Num_HIQ_V1(x)                 	(((x) & BIT_MASK_PKT_Num_HIQ_V1) << BIT_SHIFT_PKT_Num_HIQ_V1)


#define BIT_SHIFT_Head_PKT_HIQ                	0
#define BIT_MASK_Head_PKT_HIQ                 	0xff
#define BIT_Head_PKT_HIQ(x)                   	(((x) & BIT_MASK_Head_PKT_HIQ) << BIT_SHIFT_Head_PKT_HIQ)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_HIQ_INFO                           (Offset 0x0414)


#define BIT_SHIFT_Head_PKT_HIQ_V1             	0
#define BIT_MASK_Head_PKT_HIQ_V1              	0x7ff
#define BIT_Head_PKT_HIQ_V1(x)                	(((x) & BIT_MASK_Head_PKT_HIQ_V1) << BIT_SHIFT_Head_PKT_HIQ_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BCNQ_INFO                          (Offset 0x0418)


#define BIT_SHIFT_PKT_Num_BCNQ                	8
#define BIT_MASK_PKT_Num_BCNQ                 	0xff
#define BIT_PKT_Num_BCNQ(x)                   	(((x) & BIT_MASK_PKT_Num_BCNQ) << BIT_SHIFT_PKT_Num_BCNQ)


#define BIT_SHIFT_BCNQ_HEAD_PG                	0
#define BIT_MASK_BCNQ_HEAD_PG                 	0xff
#define BIT_BCNQ_HEAD_PG(x)                   	(((x) & BIT_MASK_BCNQ_HEAD_PG) << BIT_SHIFT_BCNQ_HEAD_PG)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_BCNQ_INFO                          (Offset 0x0418)


#define BIT_SHIFT_BCNQ_HEAD_PG_V1             	0
#define BIT_MASK_BCNQ_HEAD_PG_V1              	0xfff
#define BIT_BCNQ_HEAD_PG_V1(x)                	(((x) & BIT_MASK_BCNQ_HEAD_PG_V1) << BIT_SHIFT_BCNQ_HEAD_PG_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXPKT_EMPTY                        (Offset 0x041A)

#define BIT_BCNQ_EMPTY                        	BIT(11)
#define BIT_HQQ_EMPTY                         	BIT(10)
#define BIT_MQQ_EMPTY                         	BIT(9)
#define BIT_MGQ_cpu_EMPTY                     	BIT(8)
#define BIT_AC0Q_EMPTY                        	BIT(7)
#define BIT_AC1Q_EMPTY                        	BIT(6)
#define BIT_AC2Q_EMPTY                        	BIT(5)
#define BIT_AC3Q_EMPTY                        	BIT(4)
#define BIT_AC4Q_EMPTY                        	BIT(3)
#define BIT_AC5Q_EMPTY                        	BIT(2)
#define BIT_AC6Q_EMPTY                        	BIT(1)
#define BIT_AC7Q_EMPTY                        	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_CPU_MGQ_INFO                       (Offset 0x041C)

#define BIT_BCN1_POLL                         	BIT(30)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_CPU_MGQ_INFO                       (Offset 0x041C)

#define BIT_CPUMGT_POLL                       	BIT(29)
#define BIT_BCN_POLL                          	BIT(28)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_CPU_MGQ_INFO                       (Offset 0x041C)

#define BIT_CPUMGQ_FW_NUM_V1                  	BIT(12)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_CPU_MGQ_INFO                       (Offset 0x041C)

#define BIT_CPUMGQ_FW_NUM                     	BIT(8)

#define BIT_SHIFT_CPUMGQ_HEAD_PG              	0
#define BIT_MASK_CPUMGQ_HEAD_PG               	0xff
#define BIT_CPUMGQ_HEAD_PG(x)                 	(((x) & BIT_MASK_CPUMGQ_HEAD_PG) << BIT_SHIFT_CPUMGQ_HEAD_PG)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_CPU_MGQ_INFO                       (Offset 0x041C)


#define BIT_SHIFT_FW_Free_Tail_V1             	0
#define BIT_MASK_FW_Free_Tail_V1              	0xfff
#define BIT_FW_Free_Tail_V1(x)                	(((x) & BIT_MASK_FW_Free_Tail_V1) << BIT_SHIFT_FW_Free_Tail_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FWHW_TXQ_CTRL                      (Offset 0x0420)

#define BIT_Rts_limit_in_ofdm                 	BIT(23)
#define BIT_En_bcnq_dl                        	BIT(22)
#define BIT_EN_RD_RESP_NAV_BK                 	BIT(21)
#define BIT_EN_WR_FREE_TAIL                   	BIT(20)

#define BIT_SHIFT_EN_QUEUE_RPT                	8
#define BIT_MASK_EN_QUEUE_RPT                 	0xff
#define BIT_EN_QUEUE_RPT(x)                   	(((x) & BIT_MASK_EN_QUEUE_RPT) << BIT_SHIFT_EN_QUEUE_RPT)

#define BIT_EN_RTY_BK                         	BIT(7)
#define BIT_EN_USE_INI_RAT                    	BIT(6)
#define BIT_EN_RTS_NAV_BK                     	BIT(5)
#define BIT_DIS_SSN_CHECK                     	BIT(4)
#define BIT_MACID_MATCH_RTS                   	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_FWHW_TXQ_CTRL                      (Offset 0x0420)

#define BIT_EN_BCN_TRXRPT_V1                  	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_HWSEQ_CTRL                         (Offset 0x0423)

#define BIT_HWSEQ_CPUM_EN                     	BIT(7)
#define BIT_HWSEQ_BCN_EN                      	BIT(6)
#define BIT_HWSEQ_HI_EN                       	BIT(5)
#define BIT_HWSEQ_MGT_EN                      	BIT(4)
#define BIT_HWSEQ_BK_EN                       	BIT(3)
#define BIT_HWSEQ_BE_EN                       	BIT(2)
#define BIT_HWSEQ_VI_EN                       	BIT(1)
#define BIT_HWSEQ_VO_EN                       	BIT(0)

//2 REG_BCNQ_BDNY                          (Offset 0x0424)


#define BIT_SHIFT_BCNQ_PGBNDY                 	0
#define BIT_MASK_BCNQ_PGBNDY                  	0xff
#define BIT_BCNQ_PGBNDY(x)                    	(((x) & BIT_MASK_BCNQ_PGBNDY) << BIT_SHIFT_BCNQ_PGBNDY)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_BCNQ_BDNY_V1                       (Offset 0x0424)


#define BIT_SHIFT_BCNQ_PGBNDY_V1              	0
#define BIT_MASK_BCNQ_PGBNDY_V1               	0xfff
#define BIT_BCNQ_PGBNDY_V1(x)                 	(((x) & BIT_MASK_BCNQ_PGBNDY_V1) << BIT_SHIFT_BCNQ_PGBNDY_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_MGQ_BDNY                           (Offset 0x0425)


#define BIT_SHIFT_MGQ_PGBNDY                  	0
#define BIT_MASK_MGQ_PGBNDY                   	0xff
#define BIT_MGQ_PGBNDY(x)                     	(((x) & BIT_MASK_MGQ_PGBNDY) << BIT_SHIFT_MGQ_PGBNDY)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LIFETIME_EN                        (Offset 0x0426)

#define BIT_BT_INT_CPU                        	BIT(7)
#define BIT_BT_INT_PTA                        	BIT(6)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LIFETIME_EN                        (Offset 0x0426)

#define BIT_SPERPT_Entry                      	BIT(5)
#define BIT_RTYCNT_FB                         	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_LIFETIME_EN                        (Offset 0x0426)

#define BIT_EN_CTRL_RTYBIT                    	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_LIFETIME_EN                        (Offset 0x0426)

#define BIT_LIFETIME_BK_EN                    	BIT(3)
#define BIT_LIFETIME_BE_EN                    	BIT(2)
#define BIT_LIFETIME_VI_EN                    	BIT(1)
#define BIT_LIFETIME_VO_EN                    	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FW_FREE_TAIL                       (Offset 0x0427)


#define BIT_SHIFT_FW_Free_Tail                	0
#define BIT_MASK_FW_Free_Tail                 	0xff
#define BIT_FW_Free_Tail(x)                   	(((x) & BIT_MASK_FW_Free_Tail) << BIT_SHIFT_FW_Free_Tail)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SPEC_SIFS                          (Offset 0x0428)


#define BIT_SHIFT_SPEC_SIFS_OFDM_PTCL         	8
#define BIT_MASK_SPEC_SIFS_OFDM_PTCL          	0xff
#define BIT_SPEC_SIFS_OFDM_PTCL(x)            	(((x) & BIT_MASK_SPEC_SIFS_OFDM_PTCL) << BIT_SHIFT_SPEC_SIFS_OFDM_PTCL)


#define BIT_SHIFT_SPEC_SIFS_CCK_PTCL          	0
#define BIT_MASK_SPEC_SIFS_CCK_PTCL           	0xff
#define BIT_SPEC_SIFS_CCK_PTCL(x)             	(((x) & BIT_MASK_SPEC_SIFS_CCK_PTCL) << BIT_SHIFT_SPEC_SIFS_CCK_PTCL)


//2 REG_RETRY_LIMIT                        (Offset 0x042A)


#define BIT_SHIFT_SRL                         	8
#define BIT_MASK_SRL                          	0x3f
#define BIT_SRL(x)                            	(((x) & BIT_MASK_SRL) << BIT_SHIFT_SRL)


#define BIT_SHIFT_LRL                         	0
#define BIT_MASK_LRL                          	0x3f
#define BIT_LRL(x)                            	(((x) & BIT_MASK_LRL) << BIT_SHIFT_LRL)


//2 REG_TXBF_CTRL                          (Offset 0x042C)

#define BIT_r_enable_ndpa                     	BIT(31)
#define BIT_use_ndpa_paremeter                	BIT(30)
#define BIT_r_prop_txbf                       	BIT(29)
#define BIT_r_en_ndpa_int                     	BIT(28)
#define BIT_r_txbf1_80M                       	BIT(27)
#define BIT_r_txbf1_40M                       	BIT(26)
#define BIT_r_txbf1_20M                       	BIT(25)

#define BIT_SHIFT_r_txbf1_aid                 	16
#define BIT_MASK_r_txbf1_aid                  	0x1ff
#define BIT_r_txbf1_aid(x)                    	(((x) & BIT_MASK_r_txbf1_aid) << BIT_SHIFT_r_txbf1_aid)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TXBF_CTRL                          (Offset 0x042C)

#define BIT_dis_ndp_bfen                      	BIT(15)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXBF_CTRL                          (Offset 0x042C)

#define BIT_r_txbf0_80M                       	BIT(11)
#define BIT_r_txbf0_40M                       	BIT(10)
#define BIT_r_txbf0_20M                       	BIT(9)

#define BIT_SHIFT_r_txbf0_aid                 	0
#define BIT_MASK_r_txbf0_aid                  	0x1ff
#define BIT_r_txbf0_aid(x)                    	(((x) & BIT_MASK_r_txbf0_aid) << BIT_SHIFT_r_txbf0_aid)


//2 REG_DARFRC                             (Offset 0x0430)


#define BIT_SHIFT_DARF_RC8                    	(56 & CPU_OPT_WIDTH)
#define BIT_MASK_DARF_RC8                     	0x1f
#define BIT_DARF_RC8(x)                       	(((x) & BIT_MASK_DARF_RC8) << BIT_SHIFT_DARF_RC8)


#define BIT_SHIFT_DARF_RC7                    	(48 & CPU_OPT_WIDTH)
#define BIT_MASK_DARF_RC7                     	0x1f
#define BIT_DARF_RC7(x)                       	(((x) & BIT_MASK_DARF_RC7) << BIT_SHIFT_DARF_RC7)


#define BIT_SHIFT_DARF_RC6                    	(40 & CPU_OPT_WIDTH)
#define BIT_MASK_DARF_RC6                     	0x1f
#define BIT_DARF_RC6(x)                       	(((x) & BIT_MASK_DARF_RC6) << BIT_SHIFT_DARF_RC6)


#define BIT_SHIFT_DARF_RC5                    	(32 & CPU_OPT_WIDTH)
#define BIT_MASK_DARF_RC5                     	0x1f
#define BIT_DARF_RC5(x)                       	(((x) & BIT_MASK_DARF_RC5) << BIT_SHIFT_DARF_RC5)


#define BIT_SHIFT_DARF_RC4                    	24
#define BIT_MASK_DARF_RC4                     	0x1f
#define BIT_DARF_RC4(x)                       	(((x) & BIT_MASK_DARF_RC4) << BIT_SHIFT_DARF_RC4)


#define BIT_SHIFT_DARF_RC3                    	16
#define BIT_MASK_DARF_RC3                     	0x1f
#define BIT_DARF_RC3(x)                       	(((x) & BIT_MASK_DARF_RC3) << BIT_SHIFT_DARF_RC3)


#define BIT_SHIFT_DARF_RC2                    	8
#define BIT_MASK_DARF_RC2                     	0x1f
#define BIT_DARF_RC2(x)                       	(((x) & BIT_MASK_DARF_RC2) << BIT_SHIFT_DARF_RC2)


#define BIT_SHIFT_DARF_RC1                    	0
#define BIT_MASK_DARF_RC1                     	0x1f
#define BIT_DARF_RC1(x)                       	(((x) & BIT_MASK_DARF_RC1) << BIT_SHIFT_DARF_RC1)


//2 REG_RARFRC                             (Offset 0x0438)


#define BIT_SHIFT_RARF_RC8                    	(56 & CPU_OPT_WIDTH)
#define BIT_MASK_RARF_RC8                     	0x1f
#define BIT_RARF_RC8(x)                       	(((x) & BIT_MASK_RARF_RC8) << BIT_SHIFT_RARF_RC8)


#define BIT_SHIFT_RARF_RC7                    	(48 & CPU_OPT_WIDTH)
#define BIT_MASK_RARF_RC7                     	0x1f
#define BIT_RARF_RC7(x)                       	(((x) & BIT_MASK_RARF_RC7) << BIT_SHIFT_RARF_RC7)


#define BIT_SHIFT_RARF_RC6                    	(40 & CPU_OPT_WIDTH)
#define BIT_MASK_RARF_RC6                     	0x1f
#define BIT_RARF_RC6(x)                       	(((x) & BIT_MASK_RARF_RC6) << BIT_SHIFT_RARF_RC6)


#define BIT_SHIFT_RARF_RC5                    	(32 & CPU_OPT_WIDTH)
#define BIT_MASK_RARF_RC5                     	0x1f
#define BIT_RARF_RC5(x)                       	(((x) & BIT_MASK_RARF_RC5) << BIT_SHIFT_RARF_RC5)


#define BIT_SHIFT_RARF_RC4                    	24
#define BIT_MASK_RARF_RC4                     	0x1f
#define BIT_RARF_RC4(x)                       	(((x) & BIT_MASK_RARF_RC4) << BIT_SHIFT_RARF_RC4)


#define BIT_SHIFT_RARF_RC3                    	16
#define BIT_MASK_RARF_RC3                     	0x1f
#define BIT_RARF_RC3(x)                       	(((x) & BIT_MASK_RARF_RC3) << BIT_SHIFT_RARF_RC3)


#define BIT_SHIFT_RARF_RC2                    	8
#define BIT_MASK_RARF_RC2                     	0x1f
#define BIT_RARF_RC2(x)                       	(((x) & BIT_MASK_RARF_RC2) << BIT_SHIFT_RARF_RC2)


#define BIT_SHIFT_RARF_RC1                    	0
#define BIT_MASK_RARF_RC1                     	0x1f
#define BIT_RARF_RC1(x)                       	(((x) & BIT_MASK_RARF_RC1) << BIT_SHIFT_RARF_RC1)


//2 REG_RRSR                               (Offset 0x0440)


#define BIT_SHIFT_RRSR_RSC                    	21
#define BIT_MASK_RRSR_RSC                     	0x3
#define BIT_RRSR_RSC(x)                       	(((x) & BIT_MASK_RRSR_RSC) << BIT_SHIFT_RRSR_RSC)

#define BIT_RRSR_BW                           	BIT(20)

#define BIT_SHIFT_RRSC_BITMAP                 	0
#define BIT_MASK_RRSC_BITMAP                  	0xfffff
#define BIT_RRSC_BITMAP(x)                    	(((x) & BIT_MASK_RRSC_BITMAP) << BIT_SHIFT_RRSC_BITMAP)


//2 REG_ARFR0                              (Offset 0x0444)


#define BIT_SHIFT_ARFR0_V1                    	0
#define BIT_MASK_ARFR0_V1                     	0xffffffffffffffffL
#define BIT_ARFR0_V1(x)                       	(((x) & BIT_MASK_ARFR0_V1) << BIT_SHIFT_ARFR0_V1)


//2 REG_ARFR1_V1                           (Offset 0x044C)


#define BIT_SHIFT_ARFR1_V1                    	0
#define BIT_MASK_ARFR1_V1                     	0xffffffffffffffffL
#define BIT_ARFR1_V1(x)                       	(((x) & BIT_MASK_ARFR1_V1) << BIT_SHIFT_ARFR1_V1)


//2 REG_CCK_CHECK                          (Offset 0x0454)

#define BIT_CHECK_CCK_EN                      	BIT(7)
#define BIT_EN_BCN_PKT_REL                    	BIT(6)
#define BIT_BCN_PORT_SEL                      	BIT(5)
#define BIT_MOREDATA_BYPASS                   	BIT(4)
#define BIT_EN_CLR_CMD_REL_BCN_PKT            	BIT(3)

//2 REG_AMPDU_MAX_TIME_V1                  (Offset 0x0455)


#define BIT_SHIFT_AMPDU_MAX_TIME              	0
#define BIT_MASK_AMPDU_MAX_TIME               	0xff
#define BIT_AMPDU_MAX_TIME(x)                 	(((x) & BIT_MASK_AMPDU_MAX_TIME) << BIT_SHIFT_AMPDU_MAX_TIME)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AMPDU_BURST_CTRL                   (Offset 0x0455)

#define BIT_AMPDU_BURST_GLOBAL_EN             	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_BCNQ1_BDNY_V1                      (Offset 0x0456)


#define BIT_SHIFT_BCNQ1_PGBNDY_V1             	0
#define BIT_MASK_BCNQ1_PGBNDY_V1              	0xfff
#define BIT_BCNQ1_PGBNDY_V1(x)                	(((x) & BIT_MASK_BCNQ1_PGBNDY_V1) << BIT_SHIFT_BCNQ1_PGBNDY_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BCNQ1_BDNY                         (Offset 0x0457)


#define BIT_SHIFT_BCNQ1_PGBNDY                	0
#define BIT_MASK_BCNQ1_PGBNDY                 	0xff
#define BIT_BCNQ1_PGBNDY(x)                   	(((x) & BIT_MASK_BCNQ1_PGBNDY) << BIT_SHIFT_BCNQ1_PGBNDY)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_AMPDU_MAX_LENGTH                   (Offset 0x0458)


#define BIT_SHIFT_AMPDU_MAX_LENGTH            	0
#define BIT_MASK_AMPDU_MAX_LENGTH             	0xffffffffL
#define BIT_AMPDU_MAX_LENGTH(x)               	(((x) & BIT_MASK_AMPDU_MAX_LENGTH) << BIT_SHIFT_AMPDU_MAX_LENGTH)


//2 REG_ACQ_STOP                           (Offset 0x045C)

#define BIT_AC7Q_STOP                         	BIT(7)
#define BIT_AC6Q_STOP                         	BIT(6)
#define BIT_AC5Q_STOP                         	BIT(5)
#define BIT_AC4Q_STOP                         	BIT(4)
#define BIT_AC3Q_STOP                         	BIT(3)
#define BIT_AC2Q_STOP                         	BIT(2)
#define BIT_AC1Q_STOP                         	BIT(1)
#define BIT_AC0Q_STOP                         	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WMAC_LBK_BUF_HD                    (Offset 0x045D)


#define BIT_SHIFT_WMAC_LBK_BUF_HEAD           	0
#define BIT_MASK_WMAC_LBK_BUF_HEAD            	0xff
#define BIT_WMAC_LBK_BUF_HEAD(x)              	(((x) & BIT_MASK_WMAC_LBK_BUF_HEAD) << BIT_SHIFT_WMAC_LBK_BUF_HEAD)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_NDPA_RATE                          (Offset 0x045D)


#define BIT_SHIFT_r_ndpa_rate_V1              	0
#define BIT_MASK_r_ndpa_rate_V1               	0xff
#define BIT_r_ndpa_rate_V1(x)                 	(((x) & BIT_MASK_r_ndpa_rate_V1) << BIT_SHIFT_r_ndpa_rate_V1)


//2 REG_TX_HANG_CTRL                       (Offset 0x045E)

#define BIT_en_eof_V1                         	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TX_HANG_CTRL                       (Offset 0x045E)

#define BIT_DIS_OQT_BLOCK                     	BIT(1)
#define BIT_Search_Queue_en                   	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_NDPA_OPT_CTRL                      (Offset 0x045F)


#define BIT_SHIFT_bw_sigta                    	3
#define BIT_MASK_bw_sigta                     	0x3
#define BIT_bw_sigta(x)                       	(((x) & BIT_MASK_bw_sigta) << BIT_SHIFT_bw_sigta)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_NDPA_OPT_CTRL                      (Offset 0x045F)


#define BIT_SHIFT_r_ndpa_rate                 	2
#define BIT_MASK_r_ndpa_rate                  	0x3f
#define BIT_r_ndpa_rate(x)                    	(((x) & BIT_MASK_r_ndpa_rate) << BIT_SHIFT_r_ndpa_rate)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_NDPA_OPT_CTRL                      (Offset 0x045F)

#define BIT_en_bar_sigta                      	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_NDPA_OPT_CTRL                      (Offset 0x045F)


#define BIT_SHIFT_r_ndpa_bw                   	0
#define BIT_MASK_r_ndpa_bw                    	0x3
#define BIT_r_ndpa_bw(x)                      	(((x) & BIT_MASK_r_ndpa_bw) << BIT_SHIFT_r_ndpa_bw)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FAST_EDCA_CTRL                     (Offset 0x0460)


#define BIT_SHIFT_FAST_EDCA_TO_V1             	16
#define BIT_MASK_FAST_EDCA_TO_V1              	0xff
#define BIT_FAST_EDCA_TO_V1(x)                	(((x) & BIT_MASK_FAST_EDCA_TO_V1) << BIT_SHIFT_FAST_EDCA_TO_V1)


#define BIT_SHIFT_AC3_AC7_FAST_EDCA_PKT_TH    	12
#define BIT_MASK_AC3_AC7_FAST_EDCA_PKT_TH     	0xf
#define BIT_AC3_AC7_FAST_EDCA_PKT_TH(x)       	(((x) & BIT_MASK_AC3_AC7_FAST_EDCA_PKT_TH) << BIT_SHIFT_AC3_AC7_FAST_EDCA_PKT_TH)


#define BIT_SHIFT_AC2_FAST_EDCA_PKT_TH        	8
#define BIT_MASK_AC2_FAST_EDCA_PKT_TH         	0xf
#define BIT_AC2_FAST_EDCA_PKT_TH(x)           	(((x) & BIT_MASK_AC2_FAST_EDCA_PKT_TH) << BIT_SHIFT_AC2_FAST_EDCA_PKT_TH)


#define BIT_SHIFT_AC1_FAST_EDCA_PKT_TH        	4
#define BIT_MASK_AC1_FAST_EDCA_PKT_TH         	0xf
#define BIT_AC1_FAST_EDCA_PKT_TH(x)           	(((x) & BIT_MASK_AC1_FAST_EDCA_PKT_TH) << BIT_SHIFT_AC1_FAST_EDCA_PKT_TH)


#define BIT_SHIFT_AC0_FAST_EDCA_PKT_TH        	0
#define BIT_MASK_AC0_FAST_EDCA_PKT_TH         	0xf
#define BIT_AC0_FAST_EDCA_PKT_TH(x)           	(((x) & BIT_MASK_AC0_FAST_EDCA_PKT_TH) << BIT_SHIFT_AC0_FAST_EDCA_PKT_TH)


//2 REG_RD_RESP_PKT_TH                     (Offset 0x0463)


#define BIT_SHIFT_RD_RESP_PKT_TH              	0
#define BIT_MASK_RD_RESP_PKT_TH               	0x1f
#define BIT_RD_RESP_PKT_TH(x)                 	(((x) & BIT_MASK_RD_RESP_PKT_TH) << BIT_SHIFT_RD_RESP_PKT_TH)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_RD_RESP_PKT_TH                     (Offset 0x0463)


#define BIT_SHIFT_RD_RESP_PKT_TH_V1           	0
#define BIT_MASK_RD_RESP_PKT_TH_V1            	0x3f
#define BIT_RD_RESP_PKT_TH_V1(x)              	(((x) & BIT_MASK_RD_RESP_PKT_TH_V1) << BIT_SHIFT_RD_RESP_PKT_TH_V1)


//2 REG_CMDQ_INFO                          (Offset 0x0464)


#define BIT_SHIFT_QueueMacID_CMDQ_V1          	25
#define BIT_MASK_QueueMacID_CMDQ_V1           	0x7f
#define BIT_QueueMacID_CMDQ_V1(x)             	(((x) & BIT_MASK_QueueMacID_CMDQ_V1) << BIT_SHIFT_QueueMacID_CMDQ_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_CMDQ_INFO                          (Offset 0x0464)


#define BIT_SHIFT_PKT_Num_CMDQ_V2             	24
#define BIT_MASK_PKT_Num_CMDQ_V2              	0xff
#define BIT_PKT_Num_CMDQ_V2(x)                	(((x) & BIT_MASK_PKT_Num_CMDQ_V2) << BIT_SHIFT_PKT_Num_CMDQ_V2)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_CMDQ_INFO                          (Offset 0x0464)


#define BIT_SHIFT_QueueAC_CMDQ_V1             	23
#define BIT_MASK_QueueAC_CMDQ_V1              	0x3
#define BIT_QueueAC_CMDQ_V1(x)                	(((x) & BIT_MASK_QueueAC_CMDQ_V1) << BIT_SHIFT_QueueAC_CMDQ_V1)

#define BIT_TidEmpty_CMDQ_V1                  	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_CMDQ_INFO                          (Offset 0x0464)


#define BIT_SHIFT_Tail_PKT_CMDQ               	16
#define BIT_MASK_Tail_PKT_CMDQ                	0xff
#define BIT_Tail_PKT_CMDQ(x)                  	(((x) & BIT_MASK_Tail_PKT_CMDQ) << BIT_SHIFT_Tail_PKT_CMDQ)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_CMDQ_INFO                          (Offset 0x0464)


#define BIT_SHIFT_Tail_PKT_CMDQ_V2            	11
#define BIT_MASK_Tail_PKT_CMDQ_V2             	0x7ff
#define BIT_Tail_PKT_CMDQ_V2(x)               	(((x) & BIT_MASK_Tail_PKT_CMDQ_V2) << BIT_SHIFT_Tail_PKT_CMDQ_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_CMDQ_INFO                          (Offset 0x0464)


#define BIT_SHIFT_PKT_Num_CMDQ                	8
#define BIT_MASK_PKT_Num_CMDQ                 	0xff
#define BIT_PKT_Num_CMDQ(x)                   	(((x) & BIT_MASK_PKT_Num_CMDQ) << BIT_SHIFT_PKT_Num_CMDQ)


#define BIT_SHIFT_Head_PKT_CMDQ               	0
#define BIT_MASK_Head_PKT_CMDQ                	0xff
#define BIT_Head_PKT_CMDQ(x)                  	(((x) & BIT_MASK_Head_PKT_CMDQ) << BIT_SHIFT_Head_PKT_CMDQ)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_CMDQ_INFO                          (Offset 0x0464)


#define BIT_SHIFT_Head_PKT_CMDQ_V1            	0
#define BIT_MASK_Head_PKT_CMDQ_V1             	0x7ff
#define BIT_Head_PKT_CMDQ_V1(x)               	(((x) & BIT_MASK_Head_PKT_CMDQ_V1) << BIT_SHIFT_Head_PKT_CMDQ_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q4_INFO                            (Offset 0x0468)


#define BIT_SHIFT_QueueMacID_Q4_V1            	25
#define BIT_MASK_QueueMacID_Q4_V1             	0x7f
#define BIT_QueueMacID_Q4_V1(x)               	(((x) & BIT_MASK_QueueMacID_Q4_V1) << BIT_SHIFT_QueueMacID_Q4_V1)


#define BIT_SHIFT_QueueAC_Q4_V1               	23
#define BIT_MASK_QueueAC_Q4_V1                	0x3
#define BIT_QueueAC_Q4_V1(x)                  	(((x) & BIT_MASK_QueueAC_Q4_V1) << BIT_SHIFT_QueueAC_Q4_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q4_INFO                            (Offset 0x0468)

#define BIT_TidEmpty_Q4_V1                    	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q4_INFO                            (Offset 0x0468)


#define BIT_SHIFT_Tail_PKT_Q4_V1              	15
#define BIT_MASK_Tail_PKT_Q4_V1               	0xff
#define BIT_Tail_PKT_Q4_V1(x)                 	(((x) & BIT_MASK_Tail_PKT_Q4_V1) << BIT_SHIFT_Tail_PKT_Q4_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q4_INFO                            (Offset 0x0468)


#define BIT_SHIFT_Tail_PKT_Q4_V2              	11
#define BIT_MASK_Tail_PKT_Q4_V2               	0x7ff
#define BIT_Tail_PKT_Q4_V2(x)                 	(((x) & BIT_MASK_Tail_PKT_Q4_V2) << BIT_SHIFT_Tail_PKT_Q4_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q4_INFO                            (Offset 0x0468)


#define BIT_SHIFT_PKT_Num_Q4_V1               	8
#define BIT_MASK_PKT_Num_Q4_V1                	0x7f
#define BIT_PKT_Num_Q4_V1(x)                  	(((x) & BIT_MASK_PKT_Num_Q4_V1) << BIT_SHIFT_PKT_Num_Q4_V1)


#define BIT_SHIFT_Head_PKT_Q4                 	0
#define BIT_MASK_Head_PKT_Q4                  	0xff
#define BIT_Head_PKT_Q4(x)                    	(((x) & BIT_MASK_Head_PKT_Q4) << BIT_SHIFT_Head_PKT_Q4)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q4_INFO                            (Offset 0x0468)


#define BIT_SHIFT_Head_PKT_Q4_V1              	0
#define BIT_MASK_Head_PKT_Q4_V1               	0x7ff
#define BIT_Head_PKT_Q4_V1(x)                 	(((x) & BIT_MASK_Head_PKT_Q4_V1) << BIT_SHIFT_Head_PKT_Q4_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q5_INFO                            (Offset 0x046C)


#define BIT_SHIFT_QueueMacID_Q5_V1            	25
#define BIT_MASK_QueueMacID_Q5_V1             	0x7f
#define BIT_QueueMacID_Q5_V1(x)               	(((x) & BIT_MASK_QueueMacID_Q5_V1) << BIT_SHIFT_QueueMacID_Q5_V1)


#define BIT_SHIFT_QueueAC_Q5_V1               	23
#define BIT_MASK_QueueAC_Q5_V1                	0x3
#define BIT_QueueAC_Q5_V1(x)                  	(((x) & BIT_MASK_QueueAC_Q5_V1) << BIT_SHIFT_QueueAC_Q5_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q5_INFO                            (Offset 0x046C)

#define BIT_TidEmpty_Q5_V1                    	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q5_INFO                            (Offset 0x046C)


#define BIT_SHIFT_Tail_PKT_Q5_V1              	15
#define BIT_MASK_Tail_PKT_Q5_V1               	0xff
#define BIT_Tail_PKT_Q5_V1(x)                 	(((x) & BIT_MASK_Tail_PKT_Q5_V1) << BIT_SHIFT_Tail_PKT_Q5_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q5_INFO                            (Offset 0x046C)


#define BIT_SHIFT_Tail_PKT_Q5_V2              	11
#define BIT_MASK_Tail_PKT_Q5_V2               	0x7ff
#define BIT_Tail_PKT_Q5_V2(x)                 	(((x) & BIT_MASK_Tail_PKT_Q5_V2) << BIT_SHIFT_Tail_PKT_Q5_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q5_INFO                            (Offset 0x046C)


#define BIT_SHIFT_PKT_Num_Q5_V1               	8
#define BIT_MASK_PKT_Num_Q5_V1                	0x7f
#define BIT_PKT_Num_Q5_V1(x)                  	(((x) & BIT_MASK_PKT_Num_Q5_V1) << BIT_SHIFT_PKT_Num_Q5_V1)


#define BIT_SHIFT_Head_PKT_Q5                 	0
#define BIT_MASK_Head_PKT_Q5                  	0xff
#define BIT_Head_PKT_Q5(x)                    	(((x) & BIT_MASK_Head_PKT_Q5) << BIT_SHIFT_Head_PKT_Q5)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q5_INFO                            (Offset 0x046C)


#define BIT_SHIFT_Head_PKT_Q5_V1              	0
#define BIT_MASK_Head_PKT_Q5_V1               	0x7ff
#define BIT_Head_PKT_Q5_V1(x)                 	(((x) & BIT_MASK_Head_PKT_Q5_V1) << BIT_SHIFT_Head_PKT_Q5_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q6_INFO                            (Offset 0x0470)


#define BIT_SHIFT_QueueMacID_Q6_V1            	25
#define BIT_MASK_QueueMacID_Q6_V1             	0x7f
#define BIT_QueueMacID_Q6_V1(x)               	(((x) & BIT_MASK_QueueMacID_Q6_V1) << BIT_SHIFT_QueueMacID_Q6_V1)


#define BIT_SHIFT_QueueAC_Q6_V1               	23
#define BIT_MASK_QueueAC_Q6_V1                	0x3
#define BIT_QueueAC_Q6_V1(x)                  	(((x) & BIT_MASK_QueueAC_Q6_V1) << BIT_SHIFT_QueueAC_Q6_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q6_INFO                            (Offset 0x0470)

#define BIT_TidEmpty_Q6_V1                    	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q6_INFO                            (Offset 0x0470)


#define BIT_SHIFT_Tail_PKT_Q6_V1              	15
#define BIT_MASK_Tail_PKT_Q6_V1               	0xff
#define BIT_Tail_PKT_Q6_V1(x)                 	(((x) & BIT_MASK_Tail_PKT_Q6_V1) << BIT_SHIFT_Tail_PKT_Q6_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q6_INFO                            (Offset 0x0470)


#define BIT_SHIFT_Tail_PKT_Q6_V2              	11
#define BIT_MASK_Tail_PKT_Q6_V2               	0x7ff
#define BIT_Tail_PKT_Q6_V2(x)                 	(((x) & BIT_MASK_Tail_PKT_Q6_V2) << BIT_SHIFT_Tail_PKT_Q6_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q6_INFO                            (Offset 0x0470)


#define BIT_SHIFT_PKT_Num_Q6_V1               	8
#define BIT_MASK_PKT_Num_Q6_V1                	0x7f
#define BIT_PKT_Num_Q6_V1(x)                  	(((x) & BIT_MASK_PKT_Num_Q6_V1) << BIT_SHIFT_PKT_Num_Q6_V1)


#define BIT_SHIFT_Head_PKT_Q6                 	0
#define BIT_MASK_Head_PKT_Q6                  	0xff
#define BIT_Head_PKT_Q6(x)                    	(((x) & BIT_MASK_Head_PKT_Q6) << BIT_SHIFT_Head_PKT_Q6)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q6_INFO                            (Offset 0x0470)


#define BIT_SHIFT_Head_PKT_Q6_V1              	0
#define BIT_MASK_Head_PKT_Q6_V1               	0x7ff
#define BIT_Head_PKT_Q6_V1(x)                 	(((x) & BIT_MASK_Head_PKT_Q6_V1) << BIT_SHIFT_Head_PKT_Q6_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q7_INFO                            (Offset 0x0474)


#define BIT_SHIFT_QueueMacID_Q7_V1            	25
#define BIT_MASK_QueueMacID_Q7_V1             	0x7f
#define BIT_QueueMacID_Q7_V1(x)               	(((x) & BIT_MASK_QueueMacID_Q7_V1) << BIT_SHIFT_QueueMacID_Q7_V1)


#define BIT_SHIFT_QueueAC_Q7_V1               	23
#define BIT_MASK_QueueAC_Q7_V1                	0x3
#define BIT_QueueAC_Q7_V1(x)                  	(((x) & BIT_MASK_QueueAC_Q7_V1) << BIT_SHIFT_QueueAC_Q7_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q7_INFO                            (Offset 0x0474)

#define BIT_TidEmpty_Q7_V1                    	BIT(22)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q7_INFO                            (Offset 0x0474)


#define BIT_SHIFT_Tail_PKT_Q7_V1              	15
#define BIT_MASK_Tail_PKT_Q7_V1               	0xff
#define BIT_Tail_PKT_Q7_V1(x)                 	(((x) & BIT_MASK_Tail_PKT_Q7_V1) << BIT_SHIFT_Tail_PKT_Q7_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q7_INFO                            (Offset 0x0474)


#define BIT_SHIFT_Tail_PKT_Q7_V2              	11
#define BIT_MASK_Tail_PKT_Q7_V2               	0x7ff
#define BIT_Tail_PKT_Q7_V2(x)                 	(((x) & BIT_MASK_Tail_PKT_Q7_V2) << BIT_SHIFT_Tail_PKT_Q7_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_Q7_INFO                            (Offset 0x0474)


#define BIT_SHIFT_PKT_Num_Q7_V1               	8
#define BIT_MASK_PKT_Num_Q7_V1                	0x7f
#define BIT_PKT_Num_Q7_V1(x)                  	(((x) & BIT_MASK_PKT_Num_Q7_V1) << BIT_SHIFT_PKT_Num_Q7_V1)


#define BIT_SHIFT_Head_PKT_Q7                 	0
#define BIT_MASK_Head_PKT_Q7                  	0xff
#define BIT_Head_PKT_Q7(x)                    	(((x) & BIT_MASK_Head_PKT_Q7) << BIT_SHIFT_Head_PKT_Q7)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_Q7_INFO                            (Offset 0x0474)


#define BIT_SHIFT_Head_PKT_Q7_V1              	0
#define BIT_MASK_Head_PKT_Q7_V1               	0x7ff
#define BIT_Head_PKT_Q7_V1(x)                 	(((x) & BIT_MASK_Head_PKT_Q7_V1) << BIT_SHIFT_Head_PKT_Q7_V1)


//2 REG_WMAC_LBK_BUF_HD_V1                 (Offset 0x0478)


#define BIT_SHIFT_WMAC_LBK_BUF_HEAD_V1        	0
#define BIT_MASK_WMAC_LBK_BUF_HEAD_V1         	0xfff
#define BIT_WMAC_LBK_BUF_HEAD_V1(x)           	(((x) & BIT_MASK_WMAC_LBK_BUF_HEAD_V1) << BIT_SHIFT_WMAC_LBK_BUF_HEAD_V1)


//2 REG_MGQ_BDNY_V1                        (Offset 0x047A)


#define BIT_SHIFT_MGQ_PGBNDY_V1               	0
#define BIT_MASK_MGQ_PGBNDY_V1                	0xfff
#define BIT_MGQ_PGBNDY_V1(x)                  	(((x) & BIT_MASK_MGQ_PGBNDY_V1) << BIT_SHIFT_MGQ_PGBNDY_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXRPT_CTRL                         (Offset 0x047C)


#define BIT_SHIFT_SPC_READ_PTR                	24
#define BIT_MASK_SPC_READ_PTR                 	0xf
#define BIT_SPC_READ_PTR(x)                   	(((x) & BIT_MASK_SPC_READ_PTR) << BIT_SHIFT_SPC_READ_PTR)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TXRPT_CTRL                         (Offset 0x047C)


#define BIT_SHIFT_TRXRPT_TIMER_TH             	24
#define BIT_MASK_TRXRPT_TIMER_TH              	0xff
#define BIT_TRXRPT_TIMER_TH(x)                	(((x) & BIT_MASK_TRXRPT_TIMER_TH) << BIT_SHIFT_TRXRPT_TIMER_TH)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXRPT_CTRL                         (Offset 0x047C)


#define BIT_SHIFT_SPC_WRITE_PTR               	16
#define BIT_MASK_SPC_WRITE_PTR                	0xf
#define BIT_SPC_WRITE_PTR(x)                  	(((x) & BIT_MASK_SPC_WRITE_PTR) << BIT_SHIFT_SPC_WRITE_PTR)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TXRPT_CTRL                         (Offset 0x047C)


#define BIT_SHIFT_TRXRPT_LEN_TH               	16
#define BIT_MASK_TRXRPT_LEN_TH                	0xff
#define BIT_TRXRPT_LEN_TH(x)                  	(((x) & BIT_MASK_TRXRPT_LEN_TH) << BIT_SHIFT_TRXRPT_LEN_TH)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXRPT_CTRL                         (Offset 0x047C)


#define BIT_SHIFT_AC_READ_PTR                 	8
#define BIT_MASK_AC_READ_PTR                  	0xf
#define BIT_AC_READ_PTR(x)                    	(((x) & BIT_MASK_AC_READ_PTR) << BIT_SHIFT_AC_READ_PTR)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TXRPT_CTRL                         (Offset 0x047C)


#define BIT_SHIFT_TRXRPT_READ_PTR             	8
#define BIT_MASK_TRXRPT_READ_PTR              	0xff
#define BIT_TRXRPT_READ_PTR(x)                	(((x) & BIT_MASK_TRXRPT_READ_PTR) << BIT_SHIFT_TRXRPT_READ_PTR)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXRPT_CTRL                         (Offset 0x047C)


#define BIT_SHIFT_AC_WRITE_PTR                	0
#define BIT_MASK_AC_WRITE_PTR                 	0xf
#define BIT_AC_WRITE_PTR(x)                   	(((x) & BIT_MASK_AC_WRITE_PTR) << BIT_SHIFT_AC_WRITE_PTR)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TXRPT_CTRL                         (Offset 0x047C)


#define BIT_SHIFT_TRXRPT_WRITE_PTR            	0
#define BIT_MASK_TRXRPT_WRITE_PTR             	0xff
#define BIT_TRXRPT_WRITE_PTR(x)               	(((x) & BIT_MASK_TRXRPT_WRITE_PTR) << BIT_SHIFT_TRXRPT_WRITE_PTR)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_INIRTS_RATE_SEL                    (Offset 0x0480)

#define BIT_LEAG_RTS_BW_DUP                   	BIT(5)

//2 REG_BASIC_CFEND_RATE                   (Offset 0x0481)


#define BIT_SHIFT_Basic_CFEND_RATE            	0
#define BIT_MASK_Basic_CFEND_RATE             	0x1f
#define BIT_Basic_CFEND_RATE(x)               	(((x) & BIT_MASK_Basic_CFEND_RATE) << BIT_SHIFT_Basic_CFEND_RATE)


//2 REG_STBC_CFEND_RATE                    (Offset 0x0482)


#define BIT_SHIFT_STBC_CFEND_RATE             	0
#define BIT_MASK_STBC_CFEND_RATE              	0x1f
#define BIT_STBC_CFEND_RATE(x)                	(((x) & BIT_MASK_STBC_CFEND_RATE) << BIT_SHIFT_STBC_CFEND_RATE)


//2 REG_DATA_SC                            (Offset 0x0483)


#define BIT_SHIFT_TXSC_40M                    	4
#define BIT_MASK_TXSC_40M                     	0xf
#define BIT_TXSC_40M(x)                       	(((x) & BIT_MASK_TXSC_40M) << BIT_SHIFT_TXSC_40M)


#define BIT_SHIFT_TXSC_20M                    	0
#define BIT_MASK_TXSC_20M                     	0xf
#define BIT_TXSC_20M(x)                       	(((x) & BIT_MASK_TXSC_20M) << BIT_SHIFT_TXSC_20M)


//2 REG_MACID_SLEEP3                       (Offset 0x0484)


#define BIT_SHIFT_MACID127_96_PKTSLEEP        	0
#define BIT_MASK_MACID127_96_PKTSLEEP         	0xffffffffL
#define BIT_MACID127_96_PKTSLEEP(x)           	(((x) & BIT_MASK_MACID127_96_PKTSLEEP) << BIT_SHIFT_MACID127_96_PKTSLEEP)


//2 REG_MACID_SLEEP1                       (Offset 0x0488)


#define BIT_SHIFT_MACID63_32_PKTSLEEP         	0
#define BIT_MASK_MACID63_32_PKTSLEEP          	0xffffffffL
#define BIT_MACID63_32_PKTSLEEP(x)            	(((x) & BIT_MASK_MACID63_32_PKTSLEEP) << BIT_SHIFT_MACID63_32_PKTSLEEP)


//2 REG_ARFR2_V1                           (Offset 0x048C)


#define BIT_SHIFT_ARFR2_V1                    	0
#define BIT_MASK_ARFR2_V1                     	0xffffffffffffffffL
#define BIT_ARFR2_V1(x)                       	(((x) & BIT_MASK_ARFR2_V1) << BIT_SHIFT_ARFR2_V1)


//2 REG_ARFR3_V1                           (Offset 0x0494)


#define BIT_SHIFT_ARFR3_V1                    	0
#define BIT_MASK_ARFR3_V1                     	0xffffffffffffffffL
#define BIT_ARFR3_V1(x)                       	(((x) & BIT_MASK_ARFR3_V1) << BIT_SHIFT_ARFR3_V1)


//2 REG_ARFR4                              (Offset 0x049C)


#define BIT_SHIFT_ARFR4                       	0
#define BIT_MASK_ARFR4                        	0xffffffffffffffffL
#define BIT_ARFR4(x)                          	(((x) & BIT_MASK_ARFR4) << BIT_SHIFT_ARFR4)


//2 REG_ARFR5                              (Offset 0x04A4)


#define BIT_SHIFT_ARFR5                       	0
#define BIT_MASK_ARFR5                        	0xffffffffffffffffL
#define BIT_ARFR5(x)                          	(((x) & BIT_MASK_ARFR5) << BIT_SHIFT_ARFR5)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TXRPT_START_OFFSET                 (Offset 0x04AC)

#define BIT_shcut_parse_dasa                  	BIT(25)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXRPT_START_OFFSET                 (Offset 0x04AC)


#define BIT_SHIFT_LOC_AMPDU_BURST_CTRL        	24
#define BIT_MASK_LOC_AMPDU_BURST_CTRL         	0xff
#define BIT_LOC_AMPDU_BURST_CTRL(x)           	(((x) & BIT_MASK_LOC_AMPDU_BURST_CTRL) << BIT_SHIFT_LOC_AMPDU_BURST_CTRL)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TXRPT_START_OFFSET                 (Offset 0x04AC)

#define BIT_shcut_bypass                      	BIT(24)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXRPT_START_OFFSET                 (Offset 0x04AC)


#define BIT_SHIFT_LOC_BCN_RPT                 	16
#define BIT_MASK_LOC_BCN_RPT                  	0xff
#define BIT_LOC_BCN_RPT(x)                    	(((x) & BIT_MASK_LOC_BCN_RPT) << BIT_SHIFT_LOC_BCN_RPT)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TXRPT_START_OFFSET                 (Offset 0x04AC)


#define BIT_SHIFT_macid_shcut_offset          	16
#define BIT_MASK_macid_shcut_offset           	0xff
#define BIT_macid_shcut_offset(x)             	(((x) & BIT_MASK_macid_shcut_offset) << BIT_SHIFT_macid_shcut_offset)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXRPT_START_OFFSET                 (Offset 0x04AC)


#define BIT_SHIFT_LOC_TXRPT                   	8
#define BIT_MASK_LOC_TXRPT                    	0xff
#define BIT_LOC_TXRPT(x)                      	(((x) & BIT_MASK_LOC_TXRPT) << BIT_SHIFT_LOC_TXRPT)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TXRPT_START_OFFSET                 (Offset 0x04AC)


#define BIT_SHIFT_macid_ctrl_offset           	8
#define BIT_MASK_macid_ctrl_offset            	0xff
#define BIT_macid_ctrl_offset(x)              	(((x) & BIT_MASK_macid_ctrl_offset) << BIT_SHIFT_macid_ctrl_offset)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXRPT_START_OFFSET                 (Offset 0x04AC)


#define BIT_SHIFT_LOC_SRFF                    	0
#define BIT_MASK_LOC_SRFF                     	0xff
#define BIT_LOC_SRFF(x)                       	(((x) & BIT_MASK_LOC_SRFF) << BIT_SHIFT_LOC_SRFF)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TXRPT_START_OFFSET                 (Offset 0x04AC)


#define BIT_SHIFT_ampdu_txrpt_offset          	0
#define BIT_MASK_ampdu_txrpt_offset           	0xff
#define BIT_ampdu_txrpt_offset(x)             	(((x) & BIT_MASK_ampdu_txrpt_offset) << BIT_SHIFT_ampdu_txrpt_offset)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TRYING_CNT_TH                      (Offset 0x04B0)


#define BIT_SHIFT_INDEX_15                    	24
#define BIT_MASK_INDEX_15                     	0xff
#define BIT_INDEX_15(x)                       	(((x) & BIT_MASK_INDEX_15) << BIT_SHIFT_INDEX_15)


#define BIT_SHIFT_INDEX_14                    	16
#define BIT_MASK_INDEX_14                     	0xff
#define BIT_INDEX_14(x)                       	(((x) & BIT_MASK_INDEX_14) << BIT_SHIFT_INDEX_14)


#define BIT_SHIFT_INDEX_13                    	8
#define BIT_MASK_INDEX_13                     	0xff
#define BIT_INDEX_13(x)                       	(((x) & BIT_MASK_INDEX_13) << BIT_SHIFT_INDEX_13)


#define BIT_SHIFT_INDEX_12                    	0
#define BIT_MASK_INDEX_12                     	0xff
#define BIT_INDEX_12(x)                       	(((x) & BIT_MASK_INDEX_12) << BIT_SHIFT_INDEX_12)


#define BIT_SHIFT_RA_TRY_RATE_AGG_LMT         	0
#define BIT_MASK_RA_TRY_RATE_AGG_LMT          	0x1f
#define BIT_RA_TRY_RATE_AGG_LMT(x)            	(((x) & BIT_MASK_RA_TRY_RATE_AGG_LMT) << BIT_SHIFT_RA_TRY_RATE_AGG_LMT)


#define BIT_SHIFT_HW_SSN_SEQ0                 	0
#define BIT_MASK_HW_SSN_SEQ0                  	0xfff
#define BIT_HW_SSN_SEQ0(x)                    	(((x) & BIT_MASK_HW_SSN_SEQ0) << BIT_SHIFT_HW_SSN_SEQ0)


#define BIT_SHIFT_HW_SSN_SEQ1                 	0
#define BIT_MASK_HW_SSN_SEQ1                  	0xfff
#define BIT_HW_SSN_SEQ1(x)                    	(((x) & BIT_MASK_HW_SSN_SEQ1) << BIT_SHIFT_HW_SSN_SEQ1)


#define BIT_SHIFT_HW_SSN_SEQ2                 	0
#define BIT_MASK_HW_SSN_SEQ2                  	0xfff
#define BIT_HW_SSN_SEQ2(x)                    	(((x) & BIT_MASK_HW_SSN_SEQ2) << BIT_SHIFT_HW_SSN_SEQ2)


#define BIT_SHIFT_HW_SSN_SEQ3                 	0
#define BIT_MASK_HW_SSN_SEQ3                  	0xfff
#define BIT_HW_SSN_SEQ3(x)                    	(((x) & BIT_MASK_HW_SSN_SEQ3) << BIT_SHIFT_HW_SSN_SEQ3)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_POWER_STAGE1                       (Offset 0x04B4)


#define BIT_SHIFT_Power_stage1                	0
#define BIT_MASK_Power_stage1                 	0xffffff
#define BIT_Power_stage1(x)                   	(((x) & BIT_MASK_Power_stage1) << BIT_SHIFT_Power_stage1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_POWER_STAGE2                       (Offset 0x04B8)


#define BIT_SHIFT_Power_stage2                	0
#define BIT_MASK_Power_stage2                 	0xffffff
#define BIT_Power_stage2(x)                   	(((x) & BIT_MASK_Power_stage2) << BIT_SHIFT_Power_stage2)


//2 REG_SW_AMPDU_BURST_MODE_CTRL           (Offset 0x04BC)


#define BIT_SHIFT_pad_num_thres               	24
#define BIT_MASK_pad_num_thres                	0x3f
#define BIT_pad_num_thres(x)                  	(((x) & BIT_MASK_pad_num_thres) << BIT_SHIFT_pad_num_thres)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SW_AMPDU_BURST_MODE_CTRL           (Offset 0x04BC)

#define BIT_r_dma_this_queue_bk               	BIT(23)
#define BIT_r_dma_this_queue_be               	BIT(22)
#define BIT_r_dma_this_queue_vi               	BIT(21)
#define BIT_r_dma_this_queue_vo               	BIT(20)

#define BIT_SHIFT_r_total_len_th              	8
#define BIT_MASK_r_total_len_th               	0xfff
#define BIT_r_total_len_th(x)                 	(((x) & BIT_MASK_r_total_len_th) << BIT_SHIFT_r_total_len_th)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_SW_AMPDU_BURST_MODE_CTRL           (Offset 0x04BC)

#define BIT_en_new_early                      	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SW_AMPDU_BURST_MODE_CTRL           (Offset 0x04BC)

#define BIT_pre_tx_cmd                        	BIT(6)

#define BIT_SHIFT_Num_scl_en                  	4
#define BIT_MASK_Num_scl_en                   	0x3
#define BIT_Num_scl_en(x)                     	(((x) & BIT_MASK_Num_scl_en) << BIT_SHIFT_Num_scl_en)

#define BIT_bk_en                             	BIT(3)
#define BIT_be_en                             	BIT(2)
#define BIT_vi_en                             	BIT(1)
#define BIT_vo_en                             	BIT(0)

//2 REG_PKT_LIFE_TIME                      (Offset 0x04C0)


#define BIT_SHIFT_PKT_LIFTIME_BEBK            	16
#define BIT_MASK_PKT_LIFTIME_BEBK             	0xffff
#define BIT_PKT_LIFTIME_BEBK(x)               	(((x) & BIT_MASK_PKT_LIFTIME_BEBK) << BIT_SHIFT_PKT_LIFTIME_BEBK)


#define BIT_SHIFT_PKT_LIFTIME_VOVI            	0
#define BIT_MASK_PKT_LIFTIME_VOVI             	0xffff
#define BIT_PKT_LIFTIME_VOVI(x)               	(((x) & BIT_MASK_PKT_LIFTIME_VOVI) << BIT_SHIFT_PKT_LIFTIME_VOVI)


//2 REG_STBC_SETTING                       (Offset 0x04C4)


#define BIT_SHIFT_Cdend_txtime_l              	4
#define BIT_MASK_Cdend_txtime_l               	0xf
#define BIT_Cdend_txtime_l(x)                 	(((x) & BIT_MASK_Cdend_txtime_l) << BIT_SHIFT_Cdend_txtime_l)


#define BIT_SHIFT_NESS                        	2
#define BIT_MASK_NESS                         	0x3
#define BIT_NESS(x)                           	(((x) & BIT_MASK_NESS) << BIT_SHIFT_NESS)


#define BIT_SHIFT_STBC_CFEND                  	0
#define BIT_MASK_STBC_CFEND                   	0x3
#define BIT_STBC_CFEND(x)                     	(((x) & BIT_MASK_STBC_CFEND) << BIT_SHIFT_STBC_CFEND)


//2 REG_STBC_SETTING2                      (Offset 0x04C5)


#define BIT_SHIFT_Cdend_txtime_h              	0
#define BIT_MASK_Cdend_txtime_h               	0x1f
#define BIT_Cdend_txtime_h(x)                 	(((x) & BIT_MASK_Cdend_txtime_h) << BIT_SHIFT_Cdend_txtime_h)


//2 REG_QUEUE_CTRL                         (Offset 0x04C6)

#define BIT_r_use_data_bw                     	BIT(3)
#define BIT_TRI_PKT_INT_MODE1                 	BIT(2)
#define BIT_TRI_PKT_INT_MODE0                 	BIT(1)
#define BIT_ACQ_MODE_SEL                      	BIT(0)

//2 REG_SINGLE_AMPDU_CTRL                  (Offset 0x04C7)

#define BIT_EN_SINGLE_APMDU                   	BIT(7)

//2 REG_PROT_MODE_CTRL                     (Offset 0x04C8)


#define BIT_SHIFT_RTS_MAX_AGG_NUM             	24
#define BIT_MASK_RTS_MAX_AGG_NUM              	0x3f
#define BIT_RTS_MAX_AGG_NUM(x)                	(((x) & BIT_MASK_RTS_MAX_AGG_NUM) << BIT_SHIFT_RTS_MAX_AGG_NUM)


#define BIT_SHIFT_MAX_AGG_NUM                 	16
#define BIT_MASK_MAX_AGG_NUM                  	0x3f
#define BIT_MAX_AGG_NUM(x)                    	(((x) & BIT_MASK_MAX_AGG_NUM) << BIT_SHIFT_MAX_AGG_NUM)


#define BIT_SHIFT_RTS_TXTIME_TH               	8
#define BIT_MASK_RTS_TXTIME_TH                	0xff
#define BIT_RTS_TXTIME_TH(x)                  	(((x) & BIT_MASK_RTS_TXTIME_TH) << BIT_SHIFT_RTS_TXTIME_TH)


#define BIT_SHIFT_RTS_LEN_TH                  	0
#define BIT_MASK_RTS_LEN_TH                   	0xff
#define BIT_RTS_LEN_TH(x)                     	(((x) & BIT_MASK_RTS_LEN_TH) << BIT_SHIFT_RTS_LEN_TH)


//2 REG_BAR_MODE_CTRL                      (Offset 0x04CC)


#define BIT_SHIFT_BAR_RTY_LMT                 	16
#define BIT_MASK_BAR_RTY_LMT                  	0x3
#define BIT_BAR_RTY_LMT(x)                    	(((x) & BIT_MASK_BAR_RTY_LMT) << BIT_SHIFT_BAR_RTY_LMT)


#define BIT_SHIFT_BAR_PKT_TXTIME_TH           	8
#define BIT_MASK_BAR_PKT_TXTIME_TH            	0xff
#define BIT_BAR_PKT_TXTIME_TH(x)              	(((x) & BIT_MASK_BAR_PKT_TXTIME_TH) << BIT_SHIFT_BAR_PKT_TXTIME_TH)

#define BIT_BAR_EN_V1                         	BIT(6)

#define BIT_SHIFT_BAR_PKTNUM_TH_V1            	0
#define BIT_MASK_BAR_PKTNUM_TH_V1             	0x3f
#define BIT_BAR_PKTNUM_TH_V1(x)               	(((x) & BIT_MASK_BAR_PKTNUM_TH_V1) << BIT_SHIFT_BAR_PKTNUM_TH_V1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_RA_TRY_RATE_AGG_LMT                (Offset 0x04CF)


#define BIT_SHIFT_RA_TRY_RATE_AGG_LMT_V1      	0
#define BIT_MASK_RA_TRY_RATE_AGG_LMT_V1       	0x3f
#define BIT_RA_TRY_RATE_AGG_LMT_V1(x)         	(((x) & BIT_MASK_RA_TRY_RATE_AGG_LMT_V1) << BIT_SHIFT_RA_TRY_RATE_AGG_LMT_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_MACID_SLEEP2                       (Offset 0x04D0)


#define BIT_SHIFT_MACID95_64PKTSLEEP          	0
#define BIT_MASK_MACID95_64PKTSLEEP           	0xffffffffL
#define BIT_MACID95_64PKTSLEEP(x)             	(((x) & BIT_MASK_MACID95_64PKTSLEEP) << BIT_SHIFT_MACID95_64PKTSLEEP)


//2 REG_MACID_SLEEP                        (Offset 0x04D4)


#define BIT_SHIFT_MACID31_0_PKTSLEEP          	0
#define BIT_MASK_MACID31_0_PKTSLEEP           	0xffffffffL
#define BIT_MACID31_0_PKTSLEEP(x)             	(((x) & BIT_MASK_MACID31_0_PKTSLEEP) << BIT_SHIFT_MACID31_0_PKTSLEEP)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_CSI_SEQ                            (Offset 0x04DE)


#define BIT_SHIFT_HW_CSI_SEQ                  	0
#define BIT_MASK_HW_CSI_SEQ                   	0xfff
#define BIT_HW_CSI_SEQ(x)                     	(((x) & BIT_MASK_HW_CSI_SEQ) << BIT_SHIFT_HW_CSI_SEQ)


//2 REG_NULL_PKT_STATUS_V1                 (Offset 0x04E0)


#define BIT_SHIFT_PTCL_TOTAL_PG_V1            	2
#define BIT_MASK_PTCL_TOTAL_PG_V1             	0x1fff
#define BIT_PTCL_TOTAL_PG_V1(x)               	(((x) & BIT_MASK_PTCL_TOTAL_PG_V1) << BIT_SHIFT_PTCL_TOTAL_PG_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_NULL_PKT_STATUS                    (Offset 0x04E0)

#define BIT_TX_NULL_1                         	BIT(1)
#define BIT_TX_NULL_0                         	BIT(0)

//2 REG_PTCL_ERR_STATUS                    (Offset 0x04E2)


#define BIT_SHIFT_PTCL_ERR                    	0
#define BIT_MASK_PTCL_ERR                     	0xff
#define BIT_PTCL_ERR(x)                       	(((x) & BIT_MASK_PTCL_ERR) << BIT_SHIFT_PTCL_ERR)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PTCL_PKT_NUM                       (Offset 0x04E3)


#define BIT_SHIFT_PTCL_TOTAL_PG               	0
#define BIT_MASK_PTCL_TOTAL_PG                	0xff
#define BIT_PTCL_TOTAL_PG(x)                  	(((x) & BIT_MASK_PTCL_TOTAL_PG) << BIT_SHIFT_PTCL_TOTAL_PG)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_VIDEO_ENHANCEMENT_FUN              (Offset 0x04E4)

#define BIT_VIDEO_JUST_DROP                   	BIT(1)
#define BIT_VIDEO_ENHANCEMENT_FUN_EN          	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PTCL_TX_RPT                        (Offset 0x04F0)


#define BIT_SHIFT_AC_TX_RPT_INFO              	0
#define BIT_MASK_AC_TX_RPT_INFO               	0xffffffffffffffffL
#define BIT_AC_TX_RPT_INFO(x)                 	(((x) & BIT_MASK_AC_TX_RPT_INFO) << BIT_SHIFT_AC_TX_RPT_INFO)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT)


//2 REG_DUMMY_PAGE4                        (Offset 0x04FC)


#define BIT_SHIFT_DUMMY_PAGE4_V3              	20
#define BIT_MASK_DUMMY_PAGE4_V3               	0xfff
#define BIT_DUMMY_PAGE4_V3(x)                 	(((x) & BIT_MASK_DUMMY_PAGE4_V3) << BIT_SHIFT_DUMMY_PAGE4_V3)

#define BIT_MoreData_Ctrl2_En                 	BIT(19)
#define BIT_MoreData_Ctrl1_En                 	BIT(18)

#endif


#if (RTL8881A_SUPPORT)


//2 REG_DUMMY_PAGE4                        (Offset 0x04FC)


#define BIT_SHIFT_DUMMY_PAGE4_V2              	18
#define BIT_MASK_DUMMY_PAGE4_V2               	0x3fff
#define BIT_DUMMY_PAGE4_V2(x)                 	(((x) & BIT_MASK_DUMMY_PAGE4_V2) << BIT_SHIFT_DUMMY_PAGE4_V2)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_DUMMY_PAGE4                        (Offset 0x04FC)

#define BIT_EN_BCN_TRXRPT                     	BIT(17)

#endif

#endif  // wait page 3/4/5 refine


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)

//2 REG_DUMMY_PAGE4                        (Offset 0x04FC)

#define BIT_Pktin_Moredata_Replace_Enable     	BIT(16)

#define BIT_SHIFT_DUMMY_PAGE4_V1              	0
#define BIT_MASK_DUMMY_PAGE4_V1               	0xffff
#define BIT_DUMMY_PAGE4_V1(x)                 	(((x) & BIT_MASK_DUMMY_PAGE4_V1) << BIT_SHIFT_DUMMY_PAGE4_V1)

#endif

#if (RTL8814A_SUPPORT)

//2 REG_DUMMY_PAGE4_V1

#define BIT_SHIFT_DUMMY_PAGE4_V1 0
#define BIT_MASK_DUMMY_PAGE4_V1 0xffff
#define BIT_DUMMY_PAGE4_V1(x) (((x) & BIT_MASK_DUMMY_PAGE4_V1) << BIT_SHIFT_DUMMY_PAGE4_V1)


//2 REG_MOREDATA

#define BIT_SHIFT_DUMMY_PAGE4_V2 4
#define BIT_MASK_DUMMY_PAGE4_V2 0xfff
#define BIT_DUMMY_PAGE4_V2(x) (((x) & BIT_MASK_DUMMY_PAGE4_V2) << BIT_SHIFT_DUMMY_PAGE4_V2)

#define BIT_MoreData_Ctrl2_En BIT(3)
#define BIT_MoreData_Ctrl1_En BIT(2)
#define BIT_Pktin_Moredata_Replace_Enable BIT(0)

#endif

#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)

//2 REG_EDCA_VO_PARAM                      (Offset 0x0500)


#define BIT_SHIFT_TXOPLIMIT                   	16
#define BIT_MASK_TXOPLIMIT                    	0x7ff
#define BIT_TXOPLIMIT(x)                      	(((x) & BIT_MASK_TXOPLIMIT) << BIT_SHIFT_TXOPLIMIT)


#define BIT_SHIFT_CW                          	8
#define BIT_MASK_CW                           	0xff
#define BIT_CW(x)                             	(((x) & BIT_MASK_CW) << BIT_SHIFT_CW)


#define BIT_SHIFT_AIFS                        	0
#define BIT_MASK_AIFS                         	0xff
#define BIT_AIFS(x)                           	(((x) & BIT_MASK_AIFS) << BIT_SHIFT_AIFS)


//2 REG_BCNTCFG                            (Offset 0x0510)


#define BIT_SHIFT_BCNCW_MAX                   	12
#define BIT_MASK_BCNCW_MAX                    	0xf
#define BIT_BCNCW_MAX(x)                      	(((x) & BIT_MASK_BCNCW_MAX) << BIT_SHIFT_BCNCW_MAX)


#define BIT_SHIFT_BCNCW_MIN                   	8
#define BIT_MASK_BCNCW_MIN                    	0xf
#define BIT_BCNCW_MIN(x)                      	(((x) & BIT_MASK_BCNCW_MIN) << BIT_SHIFT_BCNCW_MIN)


#define BIT_SHIFT_BCNIFS                      	0
#define BIT_MASK_BCNIFS                       	0xff
#define BIT_BCNIFS(x)                         	(((x) & BIT_MASK_BCNIFS) << BIT_SHIFT_BCNIFS)


//2 REG_PIFS                               (Offset 0x0512)


#define BIT_SHIFT_PIFS                        	0
#define BIT_MASK_PIFS                         	0xff
#define BIT_PIFS(x)                           	(((x) & BIT_MASK_PIFS) << BIT_SHIFT_PIFS)


//2 REG_RDG_PIFS                           (Offset 0x0513)


#define BIT_SHIFT_RDG_PIFS                    	0
#define BIT_MASK_RDG_PIFS                     	0xff
#define BIT_RDG_PIFS(x)                       	(((x) & BIT_MASK_RDG_PIFS) << BIT_SHIFT_RDG_PIFS)


//2 REG_SIFS                               (Offset 0x0514)


#define BIT_SHIFT_SIFS_OFDM_TRX               	24
#define BIT_MASK_SIFS_OFDM_TRX                	0xff
#define BIT_SIFS_OFDM_TRX(x)                  	(((x) & BIT_MASK_SIFS_OFDM_TRX) << BIT_SHIFT_SIFS_OFDM_TRX)


#define BIT_SHIFT_SIFS_CCK_TRX                	16
#define BIT_MASK_SIFS_CCK_TRX                 	0xff
#define BIT_SIFS_CCK_TRX(x)                   	(((x) & BIT_MASK_SIFS_CCK_TRX) << BIT_SHIFT_SIFS_CCK_TRX)


#define BIT_SHIFT_SIFS_OFDM_CTX               	8
#define BIT_MASK_SIFS_OFDM_CTX                	0xff
#define BIT_SIFS_OFDM_CTX(x)                  	(((x) & BIT_MASK_SIFS_OFDM_CTX) << BIT_SHIFT_SIFS_OFDM_CTX)


#define BIT_SHIFT_SIFS_CCK_CTX                	0
#define BIT_MASK_SIFS_CCK_CTX                 	0xff
#define BIT_SIFS_CCK_CTX(x)                   	(((x) & BIT_MASK_SIFS_CCK_CTX) << BIT_SHIFT_SIFS_CCK_CTX)


//2 REG_TSFTR_SYN_OFFSET                   (Offset 0x0518)


#define BIT_SHIFT_TSFTR_SNC_OFFSET            	0
#define BIT_MASK_TSFTR_SNC_OFFSET             	0xffff
#define BIT_TSFTR_SNC_OFFSET(x)               	(((x) & BIT_MASK_TSFTR_SNC_OFFSET) << BIT_SHIFT_TSFTR_SNC_OFFSET)


//2 REG_AGGR_BREAK_TIME                    (Offset 0x051A)


#define BIT_SHIFT_AGGR_BK_TIME                	0
#define BIT_MASK_AGGR_BK_TIME                 	0xff
#define BIT_AGGR_BK_TIME(x)                   	(((x) & BIT_MASK_AGGR_BK_TIME) << BIT_SHIFT_AGGR_BK_TIME)


//2 REG_SLOT                               (Offset 0x051B)


#define BIT_SHIFT_SLOT                        	0
#define BIT_MASK_SLOT                         	0xff
#define BIT_SLOT(x)                           	(((x) & BIT_MASK_SLOT) << BIT_SHIFT_SLOT)


//2 REG_TX_PTCL_CTRL                       (Offset 0x0520)

#define BIT_DIS_EDCCA                         	BIT(15)
#define BIT_DIS_CCA                           	BIT(14)
#define BIT_LSIG_TXOP_TXCMD_NAV               	BIT(13)
#define BIT_SIFS_BK_EN                        	BIT(12)

#define BIT_SHIFT_TXQ_NAV_MSK                 	8
#define BIT_MASK_TXQ_NAV_MSK                  	0xf
#define BIT_TXQ_NAV_MSK(x)                    	(((x) & BIT_MASK_TXQ_NAV_MSK) << BIT_SHIFT_TXQ_NAV_MSK)

#define BIT_DIS_CW                            	BIT(7)
#define BIT_NAV_END_TXOP                      	BIT(6)
#define BIT_RDG_END_TXOP                      	BIT(5)
#define BIT_AC_INBCN_HOLD                     	BIT(4)
#define BIT_MGTQ_TXOP_EN                      	BIT(3)
#define BIT_MGTQ_RTSMF_EN                     	BIT(2)
#define BIT_HIQ_RTSMF_EN                      	BIT(1)
#define BIT_BCN_RTSMF_EN                      	BIT(0)

//2 REG_TXPAUSE                            (Offset 0x0522)

#define BIT_STOP_BCN_HI_MGT                   	BIT(7)
#define BIT_MAC_STOPBCNQ                      	BIT(6)
#define BIT_MAC_STOPHIQ                       	BIT(5)
#define BIT_MAC_STOPMGQ                       	BIT(4)
#define BIT_MAC_STOPBK                        	BIT(3)
#define BIT_MAC_STOPBE                        	BIT(2)
#define BIT_MAC_STOPVI                        	BIT(1)
#define BIT_MAC_STOPVO                        	BIT(0)

//2 REG_DIS_TXREQ_CLR                      (Offset 0x0523)

#define BIT_DIS_BT_CCA                        	BIT(7)
#define BIT_DIS_TXREQ_CLR_HI                  	BIT(5)
#define BIT_DIS_TXREQ_CLR_MGQ                 	BIT(4)
#define BIT_DIS_TXREQ_CLR_VO                  	BIT(3)
#define BIT_DIS_TXREQ_CLR_VI                  	BIT(2)
#define BIT_DIS_TXREQ_CLR_BE                  	BIT(1)
#define BIT_DIS_TXREQ_CLR_BK                  	BIT(0)

//2 REG_RD_CTRL                            (Offset 0x0524)

#define BIT_EN_CLR_TXREQ_INCCA                	BIT(15)
#define BIT_DIS_TX_OVER_BCNQ                  	BIT(14)
#define BIT_EN_BCNERR_INCCCA                  	BIT(13)
#define BIT_EDCCA_MSK_CNTDOWN_EN              	BIT(11)
#define BIT_DIS_TXOP_CFE                      	BIT(10)
#define BIT_DIS_LSIG_CFE                      	BIT(9)
#define BIT_DIS_STBC_CFE                      	BIT(8)
#define BIT_BKQ_RD_INIT_EN                    	BIT(7)
#define BIT_BEQ_RD_INIT_EN                    	BIT(6)
#define BIT_VIQ_RD_INIT_EN                    	BIT(5)
#define BIT_VOQ_RD_INIT_EN                    	BIT(4)
#define BIT_BKQ_RD_RESP_EN                    	BIT(3)
#define BIT_BEQ_RD_RESP_EN                    	BIT(2)
#define BIT_VIQ_RD_RESP_EN                    	BIT(1)
#define BIT_VOQ_RD_RESP_EN                    	BIT(0)

//2 REG_MBSSID_CTRL                        (Offset 0x0526)

#define BIT_MBID_BCNQ7_EN                     	BIT(7)
#define BIT_MBID_BCNQ6_EN                     	BIT(6)
#define BIT_MBID_BCNQ5_EN                     	BIT(5)
#define BIT_MBID_BCNQ4_EN                     	BIT(4)
#define BIT_MBID_BCNQ3_EN                     	BIT(3)
#define BIT_MBID_BCNQ2_EN                     	BIT(2)
#define BIT_MBID_BCNQ1_EN                     	BIT(1)
#define BIT_MBID_BCNQ0_EN                     	BIT(0)

//2 REG_P2PPS_CTRL                         (Offset 0x0527)

#define BIT_P2P_CTW_ALLSTASLEEP               	BIT(7)
#define BIT_P2P_OFF_DISTX_EN                  	BIT(6)
#define BIT_PWR_MGT_EN                        	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_P2PPS_CTRL                         (Offset 0x0527)

#define BIT_P2P_BCN_AREA_EN                   	BIT(4)
#define BIT_P2P_CTWND_EN                      	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_P2PPS_CTRL                         (Offset 0x0527)

#define BIT_P2P_NOA1_EN                       	BIT(2)
#define BIT_P2P_NOA0_EN                       	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_P2PPS_CTRL                         (Offset 0x0527)

#define BIT_P2P_BCN_SEL                       	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PKT_LIFETIME_CTRL                  (Offset 0x0528)

#define BIT_EN_P2P_CTWND1                     	BIT(23)
#define BIT_EN_BKF_CLR_TXREQ                  	BIT(22)
#define BIT_EN_TSFBIT32_RST_P2P               	BIT(21)
#define BIT_EN_BCN_TX_BTCCA                   	BIT(20)
#define BIT_DIS_PKT_TX_ATIM                   	BIT(19)
#define BIT_DIS_BCN_DIS_CTN                   	BIT(18)
#define BIT_EN_NAVEND_RST_TXOP                	BIT(17)
#define BIT_EN_FILTER_CCA                     	BIT(16)

#define BIT_SHIFT_CCA_FILTER_THRS             	8
#define BIT_MASK_CCA_FILTER_THRS              	0xff
#define BIT_CCA_FILTER_THRS(x)                	(((x) & BIT_MASK_CCA_FILTER_THRS) << BIT_SHIFT_CCA_FILTER_THRS)


#define BIT_SHIFT_EDCCA_THRS                  	0
#define BIT_MASK_EDCCA_THRS                   	0xff
#define BIT_EDCCA_THRS(x)                     	(((x) & BIT_MASK_EDCCA_THRS) << BIT_SHIFT_EDCCA_THRS)


//2 REG_P2PPS_SPEC_STATE                   (Offset 0x052B)

#define BIT_SPEC_POWER_STATE                  	BIT(7)
#define BIT_SPEC_CTWINDOW_ON                  	BIT(6)
#define BIT_SPEC_BEACON_AREA_ON               	BIT(5)
#define BIT_SPEC_CTWIN_EARLY_DISTX            	BIT(4)
#define BIT_SPEC_NOA1_OFF_PERIOD              	BIT(3)
#define BIT_SPEC_FORCE_DOZE1                  	BIT(2)
#define BIT_SPEC_NOA0_OFF_PERIOD              	BIT(1)
#define BIT_SPEC_FORCE_DOZE0                  	BIT(0)

//2 REG_TBTT_PROHIBIT                      (Offset 0x0540)


#define BIT_SHIFT_TBTT_HOLD_TIME_AP           	8
#define BIT_MASK_TBTT_HOLD_TIME_AP            	0xfff
#define BIT_TBTT_HOLD_TIME_AP(x)              	(((x) & BIT_MASK_TBTT_HOLD_TIME_AP) << BIT_SHIFT_TBTT_HOLD_TIME_AP)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TBTT_PROHIBIT                      (Offset 0x0540)


#define BIT_SHIFT_TBTT_HOLD_TIME_INFRA        	4
#define BIT_MASK_TBTT_HOLD_TIME_INFRA         	0xf
#define BIT_TBTT_HOLD_TIME_INFRA(x)           	(((x) & BIT_MASK_TBTT_HOLD_TIME_INFRA) << BIT_SHIFT_TBTT_HOLD_TIME_INFRA)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TBTT_PROHIBIT                      (Offset 0x0540)


#define BIT_SHIFT_TBTT_PROHIBIT_SETUP         	0
#define BIT_MASK_TBTT_PROHIBIT_SETUP          	0xf
#define BIT_TBTT_PROHIBIT_SETUP(x)            	(((x) & BIT_MASK_TBTT_PROHIBIT_SETUP) << BIT_SHIFT_TBTT_PROHIBIT_SETUP)


//2 REG_P2PPS_STATE                        (Offset 0x0543)

#define BIT_POWER_STATE                       	BIT(7)
#define BIT_CTWINDOW_ON                       	BIT(6)
#define BIT_BEACON_AREA_ON                    	BIT(5)
#define BIT_CTWIN_EARLY_DISTX                 	BIT(4)
#define BIT_NOA1_OFF_PERIOD                   	BIT(3)
#define BIT_FORCE_DOZE1                       	BIT(2)
#define BIT_NOA0_OFF_PERIOD                   	BIT(1)
#define BIT_FORCE_DOZE0                       	BIT(0)

//2 REG_RD_NAV_NXT                         (Offset 0x0544)


#define BIT_SHIFT_RD_NAV_PROT_NXT             	0
#define BIT_MASK_RD_NAV_PROT_NXT              	0xffff
#define BIT_RD_NAV_PROT_NXT(x)                	(((x) & BIT_MASK_RD_NAV_PROT_NXT) << BIT_SHIFT_RD_NAV_PROT_NXT)


//2 REG_NAV_PROT_LEN                       (Offset 0x0546)


#define BIT_SHIFT_NAV_PROT_LEN                	0
#define BIT_MASK_NAV_PROT_LEN                 	0xffff
#define BIT_NAV_PROT_LEN(x)                   	(((x) & BIT_MASK_NAV_PROT_LEN) << BIT_SHIFT_NAV_PROT_LEN)


//2 REG_BCN_CTRL                           (Offset 0x0550)

#define BIT_DIS_RX_BSSID_FIT                  	BIT(6)
#define BIT_DIS_TSF_UDT                       	BIT(4)
#define BIT_EN_BCN_FUNCTION                   	BIT(3)
#define BIT_EN_TXBCN_RPT                      	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_BCN_CTRL                           (Offset 0x0550)

#define BIT_EN_P2P_CTWINDOW                   	BIT(1)
#define BIT_EN_P2P_BCNQ_AREA                  	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BCN_CTRL1                          (Offset 0x0551)

#define BIT_DIS_RX_BSSID_FIT1                 	BIT(6)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_BCN_CTRL_CLINT0                    (Offset 0x0551)

#define BIT_CLI0_DIS_RX_BSSID_FIT             	BIT(6)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BCN_CTRL1                          (Offset 0x0551)

#define BIT_DIS_TSF1_UDT                      	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_BCN_CTRL_CLINT0                    (Offset 0x0551)

#define BIT_CLI0_DIS_TSF_UDT                  	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BCN_CTRL1                          (Offset 0x0551)

#define BIT_EN_BCN1_FUNCTION                  	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_BCN_CTRL_CLINT0                    (Offset 0x0551)

#define BIT_CLI0_EN_BCN_FUNCTION              	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BCN_CTRL1                          (Offset 0x0551)

#define BIT_EN_TXBCN1_RPT                     	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_BCN_CTRL_CLINT0                    (Offset 0x0551)

#define BIT_CLI0_EN_BCN_RPT                   	BIT(2)
#define BIT_CLI0_ENP2P_CTWINDOW               	BIT(1)
#define BIT_CLI0_ENP2P_BCNQ_AREA              	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_MBID_NUM                           (Offset 0x0552)

#define BIT_EN_PRE_DL_BEACON                  	BIT(3)

#define BIT_SHIFT_MBID_BCN_NUM                	0
#define BIT_MASK_MBID_BCN_NUM                 	0x7
#define BIT_MBID_BCN_NUM(x)                   	(((x) & BIT_MASK_MBID_BCN_NUM) << BIT_SHIFT_MBID_BCN_NUM)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_DUAL_TSF_RST                       (Offset 0x0553)

#define BIT_P2P_PWR_RST1                      	BIT(6)
#define BIT_SCHEDULER_RST                     	BIT(5)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_DUAL_TSF_RST                       (Offset 0x0553)

#define BIT_FREECNT_RST                       	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_DUAL_TSF_RST                       (Offset 0x0553)

#define BIT_P2P_PWR_RST0                      	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_DUAL_TSF_RST                       (Offset 0x0553)

#define BIT_TSFTR_CLI3_RST                    	BIT(4)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_DUAL_TSF_RST                       (Offset 0x0553)

#define BIT_TSFTR1_SYNC_EN                    	BIT(3)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_DUAL_TSF_RST                       (Offset 0x0553)

#define BIT_TSFTR_CLI2_RST                    	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_DUAL_TSF_RST                       (Offset 0x0553)

#define BIT_TSFTR_SYNC_EN                     	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_DUAL_TSF_RST                       (Offset 0x0553)

#define BIT_TSFTR_CLI1_RST                    	BIT(2)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_DUAL_TSF_RST                       (Offset 0x0553)

#define BIT_TSFTR1_RST                        	BIT(1)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_DUAL_TSF_RST                       (Offset 0x0553)

#define BIT_TSFTR_CLI0_RST                    	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_DUAL_TSF_RST                       (Offset 0x0553)

#define BIT_TSFTR_RST                         	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_MBSSID_BCN_SPACE                   (Offset 0x0554)


#define BIT_SHIFT_BCN_TIMER_SEL_FWRD          	28
#define BIT_MASK_BCN_TIMER_SEL_FWRD           	0x7
#define BIT_BCN_TIMER_SEL_FWRD(x)             	(((x) & BIT_MASK_BCN_TIMER_SEL_FWRD) << BIT_SHIFT_BCN_TIMER_SEL_FWRD)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_MBSSID_BCN_SPACE                   (Offset 0x0554)


#define BIT_SHIFT_BCN_SPACE1                  	16
#define BIT_MASK_BCN_SPACE1                   	0xffff
#define BIT_BCN_SPACE1(x)                     	(((x) & BIT_MASK_BCN_SPACE1) << BIT_SHIFT_BCN_SPACE1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_MBSSID_BCN_SPACE                   (Offset 0x0554)


#define BIT_SHIFT_BCN_SPACE_CLINT0            	16
#define BIT_MASK_BCN_SPACE_CLINT0             	0xfff
#define BIT_BCN_SPACE_CLINT0(x)               	(((x) & BIT_MASK_BCN_SPACE_CLINT0) << BIT_SHIFT_BCN_SPACE_CLINT0)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_MBSSID_BCN_SPACE                   (Offset 0x0554)


#define BIT_SHIFT_BCN_SPACE0                  	0
#define BIT_MASK_BCN_SPACE0                   	0xffff
#define BIT_BCN_SPACE0(x)                     	(((x) & BIT_MASK_BCN_SPACE0) << BIT_SHIFT_BCN_SPACE0)


//2 REG_DRVERLYINT                         (Offset 0x0558)


#define BIT_SHIFT_DRVERLYITV                  	0
#define BIT_MASK_DRVERLYITV                   	0xff
#define BIT_DRVERLYITV(x)                     	(((x) & BIT_MASK_DRVERLYITV) << BIT_SHIFT_DRVERLYITV)


//2 REG_BCNDMATIM                          (Offset 0x0559)


#define BIT_SHIFT_BCNDMATIM                   	0
#define BIT_MASK_BCNDMATIM                    	0xff
#define BIT_BCNDMATIM(x)                      	(((x) & BIT_MASK_BCNDMATIM) << BIT_SHIFT_BCNDMATIM)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_ATIMWND                            (Offset 0x055A)


#define BIT_SHIFT_ATIMWND                     	0
#define BIT_MASK_ATIMWND                      	0xffff
#define BIT_ATIMWND(x)                        	(((x) & BIT_MASK_ATIMWND) << BIT_SHIFT_ATIMWND)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_ATIMWND                            (Offset 0x055A)


#define BIT_SHIFT_ATIMWND0                    	0
#define BIT_MASK_ATIMWND0                     	0xffff
#define BIT_ATIMWND0(x)                       	(((x) & BIT_MASK_ATIMWND0) << BIT_SHIFT_ATIMWND0)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_USTIME_TSF                         (Offset 0x055C)


#define BIT_SHIFT_USTIME_TSF_V1               	0
#define BIT_MASK_USTIME_TSF_V1                	0xff
#define BIT_USTIME_TSF_V1(x)                  	(((x) & BIT_MASK_USTIME_TSF_V1) << BIT_SHIFT_USTIME_TSF_V1)


//2 REG_BCN_MAX_ERR                        (Offset 0x055D)


#define BIT_SHIFT_BCN_MAX_ERR                 	0
#define BIT_MASK_BCN_MAX_ERR                  	0xff
#define BIT_BCN_MAX_ERR(x)                    	(((x) & BIT_MASK_BCN_MAX_ERR) << BIT_SHIFT_BCN_MAX_ERR)


//2 REG_RXTSF_OFFSET_CCK                   (Offset 0x055E)


#define BIT_SHIFT_CCK_RXTSF_OFFSET            	0
#define BIT_MASK_CCK_RXTSF_OFFSET             	0xff
#define BIT_CCK_RXTSF_OFFSET(x)               	(((x) & BIT_MASK_CCK_RXTSF_OFFSET) << BIT_SHIFT_CCK_RXTSF_OFFSET)


//2 REG_RXTSF_OFFSET_OFDM                  (Offset 0x055F)


#define BIT_SHIFT_OFDM_RXTSF_OFFSET           	0
#define BIT_MASK_OFDM_RXTSF_OFFSET            	0xff
#define BIT_OFDM_RXTSF_OFFSET(x)              	(((x) & BIT_MASK_OFDM_RXTSF_OFFSET) << BIT_SHIFT_OFDM_RXTSF_OFFSET)


//2 REG_TSFTR                              (Offset 0x0560)


#define BIT_SHIFT_TSF_TIMER                   	0
#define BIT_MASK_TSF_TIMER                    	0xffffffffffffffffL
#define BIT_TSF_TIMER(x)                      	(((x) & BIT_MASK_TSF_TIMER) << BIT_SHIFT_TSF_TIMER)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TSFTR1                             (Offset 0x0568)


#define BIT_SHIFT_TSF_TIMER1                  	0
#define BIT_MASK_TSF_TIMER1                   	0xffffffffffffffffL
#define BIT_TSF_TIMER1(x)                     	(((x) & BIT_MASK_TSF_TIMER1) << BIT_SHIFT_TSF_TIMER1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_FREERUN_CNT                        (Offset 0x0568)


#define BIT_SHIFT_FREERUN_CNT                 	0
#define BIT_MASK_FREERUN_CNT                  	0xffffffffffffffffL
#define BIT_FREERUN_CNT(x)                    	(((x) & BIT_MASK_FREERUN_CNT) << BIT_SHIFT_FREERUN_CNT)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_ATIMWND1                           (Offset 0x0570)


#define BIT_SHIFT_ATIMWND1                    	0
#define BIT_MASK_ATIMWND1                     	0xffff
#define BIT_ATIMWND1(x)                       	(((x) & BIT_MASK_ATIMWND1) << BIT_SHIFT_ATIMWND1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_ATIMWND1                           (Offset 0x0570)


#define BIT_SHIFT_ATIMWND1_V1                 	0
#define BIT_MASK_ATIMWND1_V1                  	0xff
#define BIT_ATIMWND1_V1(x)                    	(((x) & BIT_MASK_ATIMWND1_V1) << BIT_SHIFT_ATIMWND1_V1)


//2 REG_TBTT_PROHIBIT_INFRA                (Offset 0x0571)


#define BIT_SHIFT_TBTT_PROHIBIT_INFRA         	0
#define BIT_MASK_TBTT_PROHIBIT_INFRA          	0xff
#define BIT_TBTT_PROHIBIT_INFRA(x)            	(((x) & BIT_MASK_TBTT_PROHIBIT_INFRA) << BIT_SHIFT_TBTT_PROHIBIT_INFRA)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_CTWND                              (Offset 0x0572)


#define BIT_SHIFT_CTWND                       	0
#define BIT_MASK_CTWND                        	0xff
#define BIT_CTWND(x)                          	(((x) & BIT_MASK_CTWND) << BIT_SHIFT_CTWND)


//2 REG_BCNIVLCUNT                         (Offset 0x0573)


#define BIT_SHIFT_BCNIVLCUNT                  	0
#define BIT_MASK_BCNIVLCUNT                   	0x7f
#define BIT_BCNIVLCUNT(x)                     	(((x) & BIT_MASK_BCNIVLCUNT) << BIT_SHIFT_BCNIVLCUNT)


//2 REG_BCNDROPCTRL                        (Offset 0x0574)

#define BIT_BEACON_DROP_EN                    	BIT(7)

#define BIT_SHIFT_BEACON_DROP_IVL             	0
#define BIT_MASK_BEACON_DROP_IVL              	0x7f
#define BIT_BEACON_DROP_IVL(x)                	(((x) & BIT_MASK_BEACON_DROP_IVL) << BIT_SHIFT_BEACON_DROP_IVL)


//2 REG_HGQ_TIMEOUT_PERIOD                 (Offset 0x0575)


#define BIT_SHIFT_HGQ_TIMEOUT_PERIOD          	0
#define BIT_MASK_HGQ_TIMEOUT_PERIOD           	0xff
#define BIT_HGQ_TIMEOUT_PERIOD(x)             	(((x) & BIT_MASK_HGQ_TIMEOUT_PERIOD) << BIT_SHIFT_HGQ_TIMEOUT_PERIOD)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TXCMD_TIMEOUT_PERIOD               (Offset 0x0576)


#define BIT_SHIFT_TXCMD_TIMEOUT_PERIOD        	0
#define BIT_MASK_TXCMD_TIMEOUT_PERIOD         	0xff
#define BIT_TXCMD_TIMEOUT_PERIOD(x)           	(((x) & BIT_MASK_TXCMD_TIMEOUT_PERIOD) << BIT_SHIFT_TXCMD_TIMEOUT_PERIOD)


//2 REG_MISC_CTRL                          (Offset 0x0577)

#define BIT_DIS_TRX_CAL_BCN                   	BIT(5)
#define BIT_DIS_TX_CAL_TBTT                   	BIT(4)
#define BIT_EN_FREECNT                        	BIT(3)
#define BIT_BCN_AGGRESSION                    	BIT(2)

#define BIT_SHIFT_DIS_SECONDARY_CCA           	0
#define BIT_MASK_DIS_SECONDARY_CCA            	0x3
#define BIT_DIS_SECONDARY_CCA(x)              	(((x) & BIT_MASK_DIS_SECONDARY_CCA) << BIT_SHIFT_DIS_SECONDARY_CCA)


//2 REG_BCN_CTRL_CLINT1                    (Offset 0x0578)

#define BIT_CLI1_DIS_RX_BSSID_FIT             	BIT(6)
#define BIT_CLI1_DIS_TSF_UDT                  	BIT(4)
#define BIT_CLI1_EN_BCN_FUNCTION              	BIT(3)
#define BIT_CLI1_EN_BCN_RPT                   	BIT(2)
#define BIT_CLI1_ENP2P_CTWINDOW               	BIT(1)
#define BIT_CLI1_ENP2P_BCNQ_AREA              	BIT(0)

//2 REG_BCN_CTRL_CLINT2                    (Offset 0x0579)

#define BIT_CLI2_DIS_RX_BSSID_FIT             	BIT(6)
#define BIT_CLI2_DIS_TSF_UDT                  	BIT(4)
#define BIT_CLI2_EN_BCN_FUNCTION              	BIT(3)
#define BIT_CLI2_EN_BCN_RPT                   	BIT(2)
#define BIT_CLI2_ENP2P_CTWINDOW               	BIT(1)
#define BIT_CLI2_ENP2P_BCNQ_AREA              	BIT(0)

//2 REG_BCN_CTRL_CLINT3                    (Offset 0x057A)

#define BIT_CLI3_DIS_RX_BSSID_FIT             	BIT(6)
#define BIT_CLI3_DIS_TSF_UDT                  	BIT(4)
#define BIT_CLI3_EN_BCN_FUNCTION              	BIT(3)
#define BIT_CLI3_EN_BCN_RPT                   	BIT(2)
#define BIT_CLI3_ENP2P_CTWINDOW               	BIT(1)
#define BIT_CLI3_ENP2P_BCNQ_AREA              	BIT(0)

//2 REG_P2PPS1_SPEC_STATE                  (Offset 0x057C)

#define BIT_P2P1_SPEC_POWER_STATE             	BIT(7)
#define BIT_P2P1_SPEC_CTWINDOW_ON             	BIT(6)
#define BIT_P2P1_SPEC_BCN_AREA_ON             	BIT(5)
#define BIT_P2P1_SPEC_CTWIN_EARLY_DISTX       	BIT(4)
#define BIT_P2P1_SPEC_NOA1_OFF_PERIOD         	BIT(3)
#define BIT_P2P1_SPEC_FORCE_DOZE1             	BIT(2)
#define BIT_P2P1_SPEC_NOA0_OFF_PERIOD         	BIT(1)
#define BIT_P2P1_SPEC_FORCE_DOZE0             	BIT(0)

//2 REG_P2PPS1_STATE                       (Offset 0x057D)

#define BIT_P2P1_POWER_STATE                  	BIT(7)
#define BIT_P2P1_CTWINDOW_ON                  	BIT(6)
#define BIT_P2P1_BEACON_AREA_ON               	BIT(5)
#define BIT_P2P1_CTWIN_EARLY_DISTX            	BIT(4)
#define BIT_P2P1_NOA1_OFF_PERIOD              	BIT(3)
#define BIT_P2P1_FORCE_DOZE1                  	BIT(2)
#define BIT_P2P1_NOA0_OFF_PERIOD              	BIT(1)
#define BIT_P2P1_FORCE_DOZE0                  	BIT(0)

//2 REG_P2PPS2_SPEC_STATE                  (Offset 0x057E)

#define BIT_P2P2_SPEC_POWER_STATE             	BIT(7)
#define BIT_P2P2_SPEC_CTWINDOW_ON             	BIT(6)
#define BIT_P2P2_SPEC_BCN_AREA_ON             	BIT(5)
#define BIT_P2P2_SPEC_CTWIN_EARLY_DISTX       	BIT(4)
#define BIT_P2P2_SPEC_NOA1_OFF_PERIOD         	BIT(3)
#define BIT_P2P2_SPEC_FORCE_DOZE1             	BIT(2)
#define BIT_P2P2_SPEC_NOA0_OFF_PERIOD         	BIT(1)
#define BIT_P2P2_SPEC_FORCE_DOZE0             	BIT(0)

//2 REG_P2PPS2_STATE                       (Offset 0x057F)

#define BIT_P2P2_POWER_STATE                  	BIT(7)
#define BIT_P2P2_CTWINDOW_ON                  	BIT(6)
#define BIT_P2P2_BEACON_AREA_ON               	BIT(5)
#define BIT_P2P2_CTWIN_EARLY_DISTX            	BIT(4)
#define BIT_P2P2_NOA1_OFF_PERIOD              	BIT(3)
#define BIT_P2P2_FORCE_DOZE1                  	BIT(2)
#define BIT_P2P2_NOA0_OFF_PERIOD              	BIT(1)
#define BIT_P2P2_FORCE_DOZE0                  	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_PS_TIMER                           (Offset 0x0580)


#define BIT_SHIFT_PSTIMER_INT                 	5
#define BIT_MASK_PSTIMER_INT                  	0x7ffffff
#define BIT_PSTIMER_INT(x)                    	(((x) & BIT_MASK_PSTIMER_INT) << BIT_SHIFT_PSTIMER_INT)


//2 REG_TIMER0                             (Offset 0x0584)


#define BIT_SHIFT_TIMER0_INT                  	5
#define BIT_MASK_TIMER0_INT                   	0x7ffffff
#define BIT_TIMER0_INT(x)                     	(((x) & BIT_MASK_TIMER0_INT) << BIT_SHIFT_TIMER0_INT)


//2 REG_TIMER1                             (Offset 0x0588)


#define BIT_SHIFT_TIMER1_INT                  	5
#define BIT_MASK_TIMER1_INT                   	0x7ffffff
#define BIT_TIMER1_INT(x)                     	(((x) & BIT_MASK_TIMER1_INT) << BIT_SHIFT_TIMER1_INT)


//2 REG_TBTT_CTN_AREA                      (Offset 0x058C)


#define BIT_SHIFT_TBTT_CTN_AREA               	0
#define BIT_MASK_TBTT_CTN_AREA                	0xff
#define BIT_TBTT_CTN_AREA(x)                  	(((x) & BIT_MASK_TBTT_CTN_AREA) << BIT_SHIFT_TBTT_CTN_AREA)


//2 REG_FORCE_BCN_IFS                      (Offset 0x058E)


#define BIT_SHIFT_FORCE_BCN_IFS               	0
#define BIT_MASK_FORCE_BCN_IFS                	0xff
#define BIT_FORCE_BCN_IFS(x)                  	(((x) & BIT_MASK_FORCE_BCN_IFS) << BIT_SHIFT_FORCE_BCN_IFS)


//2 REG_TXOP_MIN                           (Offset 0x0590)


#define BIT_SHIFT_TXOP_MIN                    	0
#define BIT_MASK_TXOP_MIN                     	0x3fff
#define BIT_TXOP_MIN(x)                       	(((x) & BIT_MASK_TXOP_MIN) << BIT_SHIFT_TXOP_MIN)


//2 REG_PRE_BKF_TIME                       (Offset 0x0592)


#define BIT_SHIFT_PRE_BKF_TIME                	0
#define BIT_MASK_PRE_BKF_TIME                 	0xff
#define BIT_PRE_BKF_TIME(x)                   	(((x) & BIT_MASK_PRE_BKF_TIME) << BIT_SHIFT_PRE_BKF_TIME)


//2 REG_CROSS_TXOP_CTRL                    (Offset 0x0593)

#define BIT_DTIM_BYPASS                       	BIT(2)
#define BIT_RTS_NAV_TXOP                      	BIT(1)
#define BIT_NOT_CROSS_TXOP                    	BIT(0)

//2 REG_ATIMWND2                           (Offset 0x05A0)


#define BIT_SHIFT_ATIMWND2                    	0
#define BIT_MASK_ATIMWND2                     	0xff
#define BIT_ATIMWND2(x)                       	(((x) & BIT_MASK_ATIMWND2) << BIT_SHIFT_ATIMWND2)


//2 REG_ATIMWND3                           (Offset 0x05A1)


#define BIT_SHIFT_ATIMWND3                    	0
#define BIT_MASK_ATIMWND3                     	0xff
#define BIT_ATIMWND3(x)                       	(((x) & BIT_MASK_ATIMWND3) << BIT_SHIFT_ATIMWND3)


//2 REG_ATIMWND4                           (Offset 0x05A2)


#define BIT_SHIFT_ATIMWND4                    	0
#define BIT_MASK_ATIMWND4                     	0xff
#define BIT_ATIMWND4(x)                       	(((x) & BIT_MASK_ATIMWND4) << BIT_SHIFT_ATIMWND4)


//2 REG_ATIMWND5                           (Offset 0x05A3)


#define BIT_SHIFT_ATIMWND5                    	0
#define BIT_MASK_ATIMWND5                     	0xff
#define BIT_ATIMWND5(x)                       	(((x) & BIT_MASK_ATIMWND5) << BIT_SHIFT_ATIMWND5)


//2 REG_ATIMWND6                           (Offset 0x05A4)


#define BIT_SHIFT_ATIMWND6                    	0
#define BIT_MASK_ATIMWND6                     	0xff
#define BIT_ATIMWND6(x)                       	(((x) & BIT_MASK_ATIMWND6) << BIT_SHIFT_ATIMWND6)


//2 REG_ATIMWND7                           (Offset 0x05A5)


#define BIT_SHIFT_ATIMWND7                    	0
#define BIT_MASK_ATIMWND7                     	0xff
#define BIT_ATIMWND7(x)                       	(((x) & BIT_MASK_ATIMWND7) << BIT_SHIFT_ATIMWND7)


//2 REG_ATIMUGT                            (Offset 0x05A6)


#define BIT_SHIFT_ATIM_URGENT                 	0
#define BIT_MASK_ATIM_URGENT                  	0xff
#define BIT_ATIM_URGENT(x)                    	(((x) & BIT_MASK_ATIM_URGENT) << BIT_SHIFT_ATIM_URGENT)


//2 REG_HIQ_NO_LMT_EN                      (Offset 0x05A7)

#define BIT_HIQ_NO_LMT_EN_VAP7                	BIT(7)
#define BIT_HIQ_NO_LMT_EN_VAP6                	BIT(6)
#define BIT_HIQ_NO_LMT_EN_VAP5                	BIT(5)
#define BIT_HIQ_NO_LMT_EN_VAP4                	BIT(4)
#define BIT_HIQ_NO_LMT_EN_VAP3                	BIT(3)
#define BIT_HIQ_NO_LMT_EN_VAP2                	BIT(2)
#define BIT_HIQ_NO_LMT_EN_VAP1                	BIT(1)
#define BIT_HIQ_NO_LMT_EN_ROOT                	BIT(0)

//2 REG_DTIM_COUNTER_ROOT                  (Offset 0x05A8)


#define BIT_SHIFT_DTIM_COUNT_ROOT             	0
#define BIT_MASK_DTIM_COUNT_ROOT              	0xff
#define BIT_DTIM_COUNT_ROOT(x)                	(((x) & BIT_MASK_DTIM_COUNT_ROOT) << BIT_SHIFT_DTIM_COUNT_ROOT)


//2 REG_DTIM_COUNTER_VAP1                  (Offset 0x05A9)


#define BIT_SHIFT_DTIM_COUNT_VAP1             	0
#define BIT_MASK_DTIM_COUNT_VAP1              	0xff
#define BIT_DTIM_COUNT_VAP1(x)                	(((x) & BIT_MASK_DTIM_COUNT_VAP1) << BIT_SHIFT_DTIM_COUNT_VAP1)


//2 REG_DTIM_COUNTER_VAP2                  (Offset 0x05AA)


#define BIT_SHIFT_DTIM_COUNT_VAP2             	0
#define BIT_MASK_DTIM_COUNT_VAP2              	0xff
#define BIT_DTIM_COUNT_VAP2(x)                	(((x) & BIT_MASK_DTIM_COUNT_VAP2) << BIT_SHIFT_DTIM_COUNT_VAP2)


//2 REG_DTIM_COUNTER_VAP3                  (Offset 0x05AB)


#define BIT_SHIFT_DTIM_COUNT_VAP3             	0
#define BIT_MASK_DTIM_COUNT_VAP3              	0xff
#define BIT_DTIM_COUNT_VAP3(x)                	(((x) & BIT_MASK_DTIM_COUNT_VAP3) << BIT_SHIFT_DTIM_COUNT_VAP3)


//2 REG_DTIM_COUNTER_VAP4                  (Offset 0x05AC)


#define BIT_SHIFT_DTIM_COUNT_VAP4             	0
#define BIT_MASK_DTIM_COUNT_VAP4              	0xff
#define BIT_DTIM_COUNT_VAP4(x)                	(((x) & BIT_MASK_DTIM_COUNT_VAP4) << BIT_SHIFT_DTIM_COUNT_VAP4)


//2 REG_DTIM_COUNTER_VAP5                  (Offset 0x05AD)


#define BIT_SHIFT_DTIM_COUNT_VAP5             	0
#define BIT_MASK_DTIM_COUNT_VAP5              	0xff
#define BIT_DTIM_COUNT_VAP5(x)                	(((x) & BIT_MASK_DTIM_COUNT_VAP5) << BIT_SHIFT_DTIM_COUNT_VAP5)


//2 REG_DTIM_COUNTER_VAP6                  (Offset 0x05AE)


#define BIT_SHIFT_DTIM_COUNT_VAP6             	0
#define BIT_MASK_DTIM_COUNT_VAP6              	0xff
#define BIT_DTIM_COUNT_VAP6(x)                	(((x) & BIT_MASK_DTIM_COUNT_VAP6) << BIT_SHIFT_DTIM_COUNT_VAP6)


//2 REG_DTIM_COUNTER_VAP7                  (Offset 0x05AF)


#define BIT_SHIFT_DTIM_COUNT_VAP7             	0
#define BIT_MASK_DTIM_COUNT_VAP7              	0xff
#define BIT_DTIM_COUNT_VAP7(x)                	(((x) & BIT_MASK_DTIM_COUNT_VAP7) << BIT_SHIFT_DTIM_COUNT_VAP7)


//2 REG_DIS_ATIM                           (Offset 0x05B0)

#define BIT_DIS_ATIM_VAP7                     	BIT(7)
#define BIT_DIS_ATIM_VAP6                     	BIT(6)
#define BIT_DIS_ATIM_VAP5                     	BIT(5)
#define BIT_DIS_ATIM_VAP4                     	BIT(4)
#define BIT_DIS_ATIM_VAP3                     	BIT(3)
#define BIT_DIS_ATIM_VAP2                     	BIT(2)
#define BIT_DIS_ATIM_VAP1                     	BIT(1)
#define BIT_DIS_ATIM_ROOT                     	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_EARLY_128US                        (Offset 0x05B1)


#define BIT_SHIFT_TSFT_SEL_TIMER1             	3
#define BIT_MASK_TSFT_SEL_TIMER1              	0x7
#define BIT_TSFT_SEL_TIMER1(x)                	(((x) & BIT_MASK_TSFT_SEL_TIMER1) << BIT_SHIFT_TSFT_SEL_TIMER1)


#define BIT_SHIFT_EARLY_128US                 	0
#define BIT_MASK_EARLY_128US                  	0x7
#define BIT_EARLY_128US(x)                    	(((x) & BIT_MASK_EARLY_128US) << BIT_SHIFT_EARLY_128US)


//2 REG_P2PPS1_CTRL                        (Offset 0x05B2)

#define BIT_P2P1_CTW_ALLSTASLEEP              	BIT(7)
#define BIT_P2P1_OFF_DISTX_EN                 	BIT(6)
#define BIT_P2P1_PWR_MGT_EN                   	BIT(5)
#define BIT_P2P1_NOA1_EN                      	BIT(2)
#define BIT_P2P1_NOA0_EN                      	BIT(1)

//2 REG_P2PPS2_CTRL                        (Offset 0x05B3)

#define BIT_P2P2_CTW_ALLSTASLEEP              	BIT(7)
#define BIT_P2P2_OFF_DISTX_EN                 	BIT(6)
#define BIT_P2P2_PWR_MGT_EN                   	BIT(5)
#define BIT_P2P2_NOA1_EN                      	BIT(2)
#define BIT_P2P2_NOA0_EN                      	BIT(1)

//2 REG_TIMER0_SRC_SEL                     (Offset 0x05B4)


#define BIT_SHIFT_SYNC_CLI_SEL                	4
#define BIT_MASK_SYNC_CLI_SEL                 	0x7
#define BIT_SYNC_CLI_SEL(x)                   	(((x) & BIT_MASK_SYNC_CLI_SEL) << BIT_SHIFT_SYNC_CLI_SEL)


#define BIT_SHIFT_TSFT_SEL_TIMER0             	0
#define BIT_MASK_TSFT_SEL_TIMER0              	0x7
#define BIT_TSFT_SEL_TIMER0(x)                	(((x) & BIT_MASK_TSFT_SEL_TIMER0) << BIT_SHIFT_TSFT_SEL_TIMER0)


//2 REG_NOA_UNIT_SEL                       (Offset 0x05B5)


#define BIT_SHIFT_NOA_UNIT2_SEL               	8
#define BIT_MASK_NOA_UNIT2_SEL                	0x7
#define BIT_NOA_UNIT2_SEL(x)                  	(((x) & BIT_MASK_NOA_UNIT2_SEL) << BIT_SHIFT_NOA_UNIT2_SEL)


#define BIT_SHIFT_NOA_UNIT1_SEL               	4
#define BIT_MASK_NOA_UNIT1_SEL                	0x7
#define BIT_NOA_UNIT1_SEL(x)                  	(((x) & BIT_MASK_NOA_UNIT1_SEL) << BIT_SHIFT_NOA_UNIT1_SEL)


#define BIT_SHIFT_NOA_UNIT0_SEL               	0
#define BIT_MASK_NOA_UNIT0_SEL                	0x7
#define BIT_NOA_UNIT0_SEL(x)                  	(((x) & BIT_MASK_NOA_UNIT0_SEL) << BIT_SHIFT_NOA_UNIT0_SEL)


//2 REG_P2POFF_DIS_TXTIME                  (Offset 0x05B7)


#define BIT_SHIFT_P2POFF_DIS_TXTIME           	0
#define BIT_MASK_P2POFF_DIS_TXTIME            	0xff
#define BIT_P2POFF_DIS_TXTIME(x)              	(((x) & BIT_MASK_P2POFF_DIS_TXTIME) << BIT_SHIFT_P2POFF_DIS_TXTIME)


//2 REG_MBSSID_BCN_SPACE2                  (Offset 0x05B8)


#define BIT_SHIFT_BCN_SPACE_CLINT2            	16
#define BIT_MASK_BCN_SPACE_CLINT2             	0xfff
#define BIT_BCN_SPACE_CLINT2(x)               	(((x) & BIT_MASK_BCN_SPACE_CLINT2) << BIT_SHIFT_BCN_SPACE_CLINT2)


#define BIT_SHIFT_BCN_SPACE_CLINT1            	0
#define BIT_MASK_BCN_SPACE_CLINT1             	0xfff
#define BIT_BCN_SPACE_CLINT1(x)               	(((x) & BIT_MASK_BCN_SPACE_CLINT1) << BIT_SHIFT_BCN_SPACE_CLINT1)


//2 REG_MBSSID_BCN_SPACE3                  (Offset 0x05BC)


#define BIT_SHIFT_SUB_BCN_SPACE               	16
#define BIT_MASK_SUB_BCN_SPACE                	0xff
#define BIT_SUB_BCN_SPACE(x)                  	(((x) & BIT_MASK_SUB_BCN_SPACE) << BIT_SHIFT_SUB_BCN_SPACE)


#define BIT_SHIFT_BCN_SPACE_CLINT3            	0
#define BIT_MASK_BCN_SPACE_CLINT3             	0xfff
#define BIT_BCN_SPACE_CLINT3(x)               	(((x) & BIT_MASK_BCN_SPACE_CLINT3) << BIT_SHIFT_BCN_SPACE_CLINT3)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_ACMHWCTRL                          (Offset 0x05C0)

#define BIT_BEQ_ACM_STATUS                    	BIT(7)
#define BIT_VIQ_ACM_STATUS                    	BIT(6)
#define BIT_VOQ_ACM_STATUS                    	BIT(5)
#define BIT_BEQ_ACM_EN                        	BIT(3)
#define BIT_VIQ_ACM_EN                        	BIT(2)
#define BIT_VOQ_ACM_EN                        	BIT(1)
#define BIT_ACMHWEN                           	BIT(0)

//2 REG_ACMRSTCTRL                         (Offset 0x05C1)

#define BIT_BE_ACM_RESET_USED_TIME            	BIT(2)
#define BIT_VI_ACM_RESET_USED_TIME            	BIT(1)
#define BIT_VO_ACM_RESET_USED_TIME            	BIT(0)

//2 REG_ACMAVG                             (Offset 0x05C2)


#define BIT_SHIFT_AVGPERIOD                   	0
#define BIT_MASK_AVGPERIOD                    	0xffff
#define BIT_AVGPERIOD(x)                      	(((x) & BIT_MASK_AVGPERIOD) << BIT_SHIFT_AVGPERIOD)


//2 REG_VO_ADMTIME                         (Offset 0x05C4)


#define BIT_SHIFT_VO_ADMITTED_TIME            	0
#define BIT_MASK_VO_ADMITTED_TIME             	0xffff
#define BIT_VO_ADMITTED_TIME(x)               	(((x) & BIT_MASK_VO_ADMITTED_TIME) << BIT_SHIFT_VO_ADMITTED_TIME)


//2 REG_VI_ADMTIME                         (Offset 0x05C6)


#define BIT_SHIFT_VI_ADMITTED_TIME            	0
#define BIT_MASK_VI_ADMITTED_TIME             	0xffff
#define BIT_VI_ADMITTED_TIME(x)               	(((x) & BIT_MASK_VI_ADMITTED_TIME) << BIT_SHIFT_VI_ADMITTED_TIME)


//2 REG_BE_ADMTIME                         (Offset 0x05C8)


#define BIT_SHIFT_BE_ADMITTED_TIME            	0
#define BIT_MASK_BE_ADMITTED_TIME             	0xffff
#define BIT_BE_ADMITTED_TIME(x)               	(((x) & BIT_MASK_BE_ADMITTED_TIME) << BIT_SHIFT_BE_ADMITTED_TIME)


//2 REG_EDCA_RANDOM_GEN                    (Offset 0x05CC)


#define BIT_SHIFT_RANDOM_GEN                  	0
#define BIT_MASK_RANDOM_GEN                   	0xffffff
#define BIT_RANDOM_GEN(x)                     	(((x) & BIT_MASK_RANDOM_GEN) << BIT_SHIFT_RANDOM_GEN)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXCMD_NOA_SEL                      (Offset 0x05CF)

#define BIT_NOA_SEL                           	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_TXCMD_NOA_SEL                      (Offset 0x05CF)


#define BIT_SHIFT_NOA_SEL                     	4
#define BIT_MASK_NOA_SEL                      	0x7
#define BIT_NOA_SEL(x)                        	(((x) & BIT_MASK_NOA_SEL) << BIT_SHIFT_NOA_SEL)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TXCMD_NOA_SEL                      (Offset 0x05CF)


#define BIT_SHIFT_TXCMD_SEG_SEL               	0
#define BIT_MASK_TXCMD_SEG_SEL                	0xf
#define BIT_TXCMD_SEG_SEL(x)                  	(((x) & BIT_MASK_TXCMD_SEG_SEL) << BIT_SHIFT_TXCMD_SEG_SEL)


//2 REG_NOA_PARAM                          (Offset 0x05E0)


#define BIT_SHIFT_NOA_COUNT                   	(96 & CPU_OPT_WIDTH)
#define BIT_MASK_NOA_COUNT                    	0xff
#define BIT_NOA_COUNT(x)                      	(((x) & BIT_MASK_NOA_COUNT) << BIT_SHIFT_NOA_COUNT)


#define BIT_SHIFT_NOA_START_TIME              	(64 & CPU_OPT_WIDTH)
#define BIT_MASK_NOA_START_TIME               	0xffffffffL
#define BIT_NOA_START_TIME(x)                 	(((x) & BIT_MASK_NOA_START_TIME) << BIT_SHIFT_NOA_START_TIME)


#define BIT_SHIFT_NOA_INTERVAL                	(32 & CPU_OPT_WIDTH)
#define BIT_MASK_NOA_INTERVAL                 	0xffffffffL
#define BIT_NOA_INTERVAL(x)                   	(((x) & BIT_MASK_NOA_INTERVAL) << BIT_SHIFT_NOA_INTERVAL)


#define BIT_SHIFT_NOA_DURATION                	0
#define BIT_MASK_NOA_DURATION                 	0xffffffffL
#define BIT_NOA_DURATION(x)                   	(((x) & BIT_MASK_NOA_DURATION) << BIT_SHIFT_NOA_DURATION)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_NOA_SUBIE                          (Offset 0x05ED)

#define BIT_MORE_NOA_DESC                     	BIT(19)
#define BIT_NOA_DESC1_VALID                   	BIT(18)
#define BIT_NOA_DESC0_VALID                   	BIT(17)
#define BIT_NOA_HEAD_VALID                    	BIT(16)
#define BIT_NOA_OPP_PS                        	BIT(15)

#define BIT_SHIFT_NOA_CTW                     	8
#define BIT_MASK_NOA_CTW                      	0x7f
#define BIT_NOA_CTW(x)                        	(((x) & BIT_MASK_NOA_CTW) << BIT_SHIFT_NOA_CTW)


#define BIT_SHIFT_NOA_INDEX                   	0
#define BIT_MASK_NOA_INDEX                    	0xff
#define BIT_NOA_INDEX(x)                      	(((x) & BIT_MASK_NOA_INDEX) << BIT_SHIFT_NOA_INDEX)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_P2P_RST                            (Offset 0x05F0)

#define BIT_P2P2_PWR_RST1                     	BIT(5)
#define BIT_P2P2_PWR_RST0                     	BIT(4)
#define BIT_P2P1_PWR_RST1                     	BIT(3)
#define BIT_P2P1_PWR_RST0                     	BIT(2)
#define BIT_P2P_PWR_RST1_V1                   	BIT(1)
#define BIT_P2P_PWR_RST0_V1                   	BIT(0)

//2 REG_SCHEDULER_RST                      (Offset 0x05F1)

#define BIT_SYNC_CLI                          	BIT(1)
#define BIT_SCHEDULER_RST_V1                  	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_SCH_TXCMD                          (Offset 0x05F8)


#define BIT_SHIFT_SCH_TXCMD                   	0
#define BIT_MASK_SCH_TXCMD                    	0xffffffffL
#define BIT_SCH_TXCMD(x)                      	(((x) & BIT_MASK_SCH_TXCMD) << BIT_SHIFT_SCH_TXCMD)


//2 REG_PAGE5_DUMMY                        (Offset 0x05FC)


#define BIT_SHIFT_DUMMY                       	0
#define BIT_MASK_DUMMY                        	0xffffffffL
#define BIT_DUMMY(x)                          	(((x) & BIT_MASK_DUMMY) << BIT_SHIFT_DUMMY)


#endif



#if (RTL8814A_SUPPORT)


//2 REG_WMAC_CR                            (Offset 0x0600)

#define BIT_APSDOFF_STATUS                    	BIT(7)
#define BIT_APSDOFF                           	BIT(6)
#define BIT_STANDBY_STATUS                    	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WMAC_CR                            (Offset 0x0600)

#define BIT_IC_MACPHY_M                       	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_TCR                                (Offset 0x0604)

#define BIT_WMAC_DISABLE_CCK                  	BIT(30)
#define BIT_WMAC_RAW_LEN                      	BIT(29)
#define BIT_WMAC_NOTX_IN_RXNDP                	BIT(28)
#define BIT_WMAC_EN_EOF                       	BIT(27)
#define BIT_WMAC_BF_SEL                       	BIT(26)
#define BIT_BF_SEL                            	BIT(25)
#define BIT_RXLEN_SEL                         	BIT(24)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TCR                                (Offset 0x0604)


#define BIT_SHIFT_TSFT_CMP                    	20
#define BIT_MASK_TSFT_CMP                     	0xf
#define BIT_TSFT_CMP(x)                       	(((x) & BIT_MASK_TSFT_CMP) << BIT_SHIFT_TSFT_CMP)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_TCR                                (Offset 0x0604)

#define BIT_WMAC_TCR_EN_20MST                 	BIT(19)
#define BIT_WMAC_DIS_SigTa                    	BIT(18)
#define BIT_WMAC_DIS_A2B0                     	BIT(17)
#define BIT_WMAC_MSK_SIGBCRC                  	BIT(16)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TCR                                (Offset 0x0604)

#define BIT_WMAC_TCR_ERRSTEN_3                	BIT(15)
#define BIT_WMAC_TCR_ERRSTEN_2                	BIT(14)
#define BIT_WMAC_TCR_ERRSTEN_1                	BIT(13)
#define BIT_WMAC_TCR_ERRSTEN_0                	BIT(12)
#define BIT_WMAC_TCR_TXSK_PERPKT              	BIT(11)
#define BIT_ICV                               	BIT(10)
#define BIT_CFEND_FORMAT                      	BIT(9)
#define BIT_CRC                               	BIT(8)
#define BIT_PWRBIT_OW_EN                      	BIT(7)
#define BIT_PWR_ST                            	BIT(6)
#define BIT_WMAC_TCR_UPD_TIMIE                	BIT(5)
#define BIT_WMAC_TCR_UPD_HGQMD                	BIT(4)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_TCR                                (Offset 0x0604)

#define BIT_VHTSIGA1_TXPS                     	BIT(3)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TCR                                (Offset 0x0604)

#define BIT_PAD_SEL                           	BIT(2)
#define BIT_DIS_GCLK                          	BIT(1)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_TCR                                (Offset 0x0604)

#define BIT_TSFRST                            	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_TCR                                (Offset 0x0604)

#define BIT_r_WMAC_TCR_LSIG                   	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RCR                                (Offset 0x0608)

#define BIT_APP_FCS                           	BIT(31)
#define BIT_APP_MIC                           	BIT(30)
#define BIT_APP_ICV                           	BIT(29)
#define BIT_APP_PHYSTS                        	BIT(28)
#define BIT_APP_BASSN                         	BIT(27)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_RCR                                (Offset 0x0608)

#define BIT_VHT_DACK                          	BIT(26)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RCR                                (Offset 0x0608)

#define BIT_TCPOFLD_EN                        	BIT(25)
#define BIT_ENMBID                            	BIT(24)
#define BIT_LSIGEN                            	BIT(23)
#define BIT_MFBEN                             	BIT(22)
#define BIT_DISCHKPPDLLEN                     	BIT(21)
#define BIT_PKTCTL_DLEN                       	BIT(20)
#define BIT_TIM_PARSER_EN                     	BIT(18)
#define BIT_BC_MD_EN                          	BIT(17)
#define BIT_UC_MD_EN                          	BIT(16)
#define BIT_RXSK_PERPKT                       	BIT(15)
#define BIT_HTC_LOC_CTRL                      	BIT(14)
#define BIT_AMF                               	BIT(13)
#define BIT_ACF                               	BIT(12)
#define BIT_ADF                               	BIT(11)
#define BIT_DISDECMYPKT                       	BIT(10)
#define BIT_AICV                              	BIT(9)
#define BIT_ACRC32                            	BIT(8)
#define BIT_CBSSID_BCN                        	BIT(7)
#define BIT_CBSSID_DATA                       	BIT(6)
#define BIT_APWRMGT                           	BIT(5)
#define BIT_ADD3                              	BIT(4)
#define BIT_AB                                	BIT(3)
#define BIT_AM                                	BIT(2)
#define BIT_APM                               	BIT(1)
#define BIT_AAP                               	BIT(0)

//2 REG_RX_PKT_LIMIT                       (Offset 0x060C)


#define BIT_SHIFT_RXPKTLMT                    	0
#define BIT_MASK_RXPKTLMT                     	0x3f
#define BIT_RXPKTLMT(x)                       	(((x) & BIT_MASK_RXPKTLMT) << BIT_SHIFT_RXPKTLMT)


//2 REG_RX_DLK_TIME                        (Offset 0x060D)


#define BIT_SHIFT_RX_DLK_TIME                 	0
#define BIT_MASK_RX_DLK_TIME                  	0xff
#define BIT_RX_DLK_TIME(x)                    	(((x) & BIT_MASK_RX_DLK_TIME) << BIT_SHIFT_RX_DLK_TIME)


//2 REG_RX_DRVINFO_SZ                      (Offset 0x060F)


#define BIT_SHIFT_DRVINFO_SZ                  	0
#define BIT_MASK_DRVINFO_SZ                   	0xff
#define BIT_DRVINFO_SZ(x)                     	(((x) & BIT_MASK_DRVINFO_SZ) << BIT_SHIFT_DRVINFO_SZ)


//2 REG_MACID                              (Offset 0x0610)


#define BIT_SHIFT_MACID                       	0
#define BIT_MASK_MACID                        	0xffffffffffffL
#define BIT_MACID(x)                          	(((x) & BIT_MASK_MACID) << BIT_SHIFT_MACID)


//2 REG_BSSID                              (Offset 0x0618)


#define BIT_SHIFT_BSSID                       	0
#define BIT_MASK_BSSID                        	0xffffffffffffL
#define BIT_BSSID(x)                          	(((x) & BIT_MASK_BSSID) << BIT_SHIFT_BSSID)


//2 REG_MAR                                (Offset 0x0620)


#define BIT_SHIFT_MAR                         	0
#define BIT_MASK_MAR                          	0xffffffffffffffffL
#define BIT_MAR(x)                            	(((x) & BIT_MASK_MAR) << BIT_SHIFT_MAR)


//2 REG_MBIDCAMCFG_1                       (Offset 0x0628)


#define BIT_SHIFT_MBIDCAM_RWDATA_L            	0
#define BIT_MASK_MBIDCAM_RWDATA_L             	0xffffffffL
#define BIT_MBIDCAM_RWDATA_L(x)               	(((x) & BIT_MASK_MBIDCAM_RWDATA_L) << BIT_SHIFT_MBIDCAM_RWDATA_L)


//2 REG_MBIDCAMCFG_2                       (Offset 0x062C)

#define BIT_MBIDCAM_POLL                      	BIT(31)
#define BIT_MBIDCAM_WT_EN                     	BIT(30)

#define BIT_SHIFT_MBIDCAM_ADDR                	24
#define BIT_MASK_MBIDCAM_ADDR                 	0x1f
#define BIT_MBIDCAM_ADDR(x)                   	(((x) & BIT_MASK_MBIDCAM_ADDR) << BIT_SHIFT_MBIDCAM_ADDR)

#define BIT_MBIDCAM_VALID                     	BIT(23)
#define BIT_LSIC_TXOP_EN                      	BIT(17)
#define BIT_CTS_EN                            	BIT(16)

#define BIT_SHIFT_MBIDCAM_RWDATA_H            	0
#define BIT_MASK_MBIDCAM_RWDATA_H             	0xffff
#define BIT_MBIDCAM_RWDATA_H(x)               	(((x) & BIT_MASK_MBIDCAM_RWDATA_H) << BIT_SHIFT_MBIDCAM_RWDATA_H)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_MCU_TEST_1                         (Offset 0x0630)


#define BIT_SHIFT_MCU_RSVD                    	0
#define BIT_MASK_MCU_RSVD                     	0xffffffffL
#define BIT_MCU_RSVD(x)                       	(((x) & BIT_MASK_MCU_RSVD) << BIT_SHIFT_MCU_RSVD)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_WMAC_TCR_TSFT_OFS                  (Offset 0x0630)


#define BIT_SHIFT_WMAC_TCR_TSFT_OFS           	0
#define BIT_MASK_WMAC_TCR_TSFT_OFS            	0xffff
#define BIT_WMAC_TCR_TSFT_OFS(x)              	(((x) & BIT_MASK_WMAC_TCR_TSFT_OFS) << BIT_SHIFT_WMAC_TCR_TSFT_OFS)


//2 REG_UDF_THSD                           (Offset 0x0632)


#define BIT_SHIFT_UDF_THSD                    	0
#define BIT_MASK_UDF_THSD                     	0xff
#define BIT_UDF_THSD(x)                       	(((x) & BIT_MASK_UDF_THSD) << BIT_SHIFT_UDF_THSD)


//2 REG_ZLD_NUM                            (Offset 0x0633)


#define BIT_SHIFT_ZLD_NUM                     	0
#define BIT_MASK_ZLD_NUM                      	0xff
#define BIT_ZLD_NUM(x)                        	(((x) & BIT_MASK_ZLD_NUM) << BIT_SHIFT_ZLD_NUM)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_MCU_TEST_2                         (Offset 0x0634)


#define BIT_SHIFT_MCU_RSVD_2                  	0
#define BIT_MASK_MCU_RSVD_2                   	0xffffffffL
#define BIT_MCU_RSVD_2(x)                     	(((x) & BIT_MASK_MCU_RSVD_2) << BIT_SHIFT_MCU_RSVD_2)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_STMP_THSD                          (Offset 0x0634)


#define BIT_SHIFT_STMP_THSD                   	0
#define BIT_MASK_STMP_THSD                    	0xff
#define BIT_STMP_THSD(x)                      	(((x) & BIT_MASK_STMP_THSD) << BIT_SHIFT_STMP_THSD)


//2 REG_WMAC_TXTIMEOUT                     (Offset 0x0635)


#define BIT_SHIFT_WMAC_TXTIMEOUT              	0
#define BIT_MASK_WMAC_TXTIMEOUT               	0xff
#define BIT_WMAC_TXTIMEOUT(x)                 	(((x) & BIT_MASK_WMAC_TXTIMEOUT) << BIT_SHIFT_WMAC_TXTIMEOUT)


//2 REG_MCU_TEST_2_V1                      (Offset 0x0636)


#define BIT_SHIFT_MCU_RSVD_2_V1               	0
#define BIT_MASK_MCU_RSVD_2_V1                	0xffff
#define BIT_MCU_RSVD_2_V1(x)                  	(((x) & BIT_MASK_MCU_RSVD_2_V1) << BIT_SHIFT_MCU_RSVD_2_V1)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_USTIME_EDCA                        (Offset 0x0638)


#define BIT_SHIFT_USTIME_EDCA                 	0
#define BIT_MASK_USTIME_EDCA                  	0xff
#define BIT_USTIME_EDCA(x)                    	(((x) & BIT_MASK_USTIME_EDCA) << BIT_SHIFT_USTIME_EDCA)


//2 REG_MAC_SPEC_SIFS                      (Offset 0x063A)


#define BIT_SHIFT_SPEC_SIFS_OFDM              	8
#define BIT_MASK_SPEC_SIFS_OFDM               	0xff
#define BIT_SPEC_SIFS_OFDM(x)                 	(((x) & BIT_MASK_SPEC_SIFS_OFDM) << BIT_SHIFT_SPEC_SIFS_OFDM)


#define BIT_SHIFT_SPEC_SIFS_CCK               	0
#define BIT_MASK_SPEC_SIFS_CCK                	0xff
#define BIT_SPEC_SIFS_CCK(x)                  	(((x) & BIT_MASK_SPEC_SIFS_CCK) << BIT_SHIFT_SPEC_SIFS_CCK)


//2 REG_RESP_SIFS_CCK                      (Offset 0x063C)


#define BIT_SHIFT_SIFS__R2T_CCK               	8
#define BIT_MASK_SIFS__R2T_CCK                	0xff
#define BIT_SIFS__R2T_CCK(x)                  	(((x) & BIT_MASK_SIFS__R2T_CCK) << BIT_SHIFT_SIFS__R2T_CCK)


#define BIT_SHIFT_SIFS_T2T_CCK                	0
#define BIT_MASK_SIFS_T2T_CCK                 	0xff
#define BIT_SIFS_T2T_CCK(x)                   	(((x) & BIT_MASK_SIFS_T2T_CCK) << BIT_SHIFT_SIFS_T2T_CCK)


//2 REG_RESP_SIFS_OFDM                     (Offset 0x063E)


#define BIT_SHIFT_SIFS_R2T_OFDM               	8
#define BIT_MASK_SIFS_R2T_OFDM                	0xff
#define BIT_SIFS_R2T_OFDM(x)                  	(((x) & BIT_MASK_SIFS_R2T_OFDM) << BIT_SHIFT_SIFS_R2T_OFDM)


#define BIT_SHIFT_SIFS_T2T_OFDM               	0
#define BIT_MASK_SIFS_T2T_OFDM                	0xff
#define BIT_SIFS_T2T_OFDM(x)                  	(((x) & BIT_MASK_SIFS_T2T_OFDM) << BIT_SHIFT_SIFS_T2T_OFDM)


//2 REG_ACKTO                              (Offset 0x0640)


#define BIT_SHIFT_ACKTO                       	0
#define BIT_MASK_ACKTO                        	0xff
#define BIT_ACKTO(x)                          	(((x) & BIT_MASK_ACKTO) << BIT_SHIFT_ACKTO)


//2 REG_CTS2TO                             (Offset 0x0641)


#define BIT_SHIFT_CTS2TO                      	0
#define BIT_MASK_CTS2TO                       	0xff
#define BIT_CTS2TO(x)                         	(((x) & BIT_MASK_CTS2TO) << BIT_SHIFT_CTS2TO)


//2 REG_EIFS                               (Offset 0x0642)


#define BIT_SHIFT_EIFS                        	0
#define BIT_MASK_EIFS                         	0xffff
#define BIT_EIFS(x)                           	(((x) & BIT_MASK_EIFS) << BIT_SHIFT_EIFS)


//2 REG_NAV_CTRL                           (Offset 0x0650)


#define BIT_SHIFT_NAV__UPPER                  	16
#define BIT_MASK_NAV__UPPER                   	0xff
#define BIT_NAV__UPPER(x)                     	(((x) & BIT_MASK_NAV__UPPER) << BIT_SHIFT_NAV__UPPER)


#define BIT_SHIFT_RxMyRTS_NAV                 	8
#define BIT_MASK_RxMyRTS_NAV                  	0xf
#define BIT_RxMyRTS_NAV(x)                    	(((x) & BIT_MASK_RxMyRTS_NAV) << BIT_SHIFT_RxMyRTS_NAV)


#define BIT_SHIFT_RTSRST                      	0
#define BIT_MASK_RTSRST                       	0xff
#define BIT_RTSRST(x)                         	(((x) & BIT_MASK_RTSRST) << BIT_SHIFT_RTSRST)


//2 REG_BACAMCMD                           (Offset 0x0654)

#define BIT_BACAM_POLL                        	BIT(31)
#define BIT_BACAM_RST                         	BIT(17)
#define BIT_BACAM_RW                          	BIT(16)

#define BIT_SHIFT_TXSBM                       	14
#define BIT_MASK_TXSBM                        	0x3
#define BIT_TXSBM(x)                          	(((x) & BIT_MASK_TXSBM) << BIT_SHIFT_TXSBM)


#define BIT_SHIFT_BACAM_ADDR                  	0
#define BIT_MASK_BACAM_ADDR                   	0x3f
#define BIT_BACAM_ADDR(x)                     	(((x) & BIT_MASK_BACAM_ADDR) << BIT_SHIFT_BACAM_ADDR)


//2 REG_BACAMCONTENT                       (Offset 0x0658)


#define BIT_SHIFT_BA_CONTENT_H                	(32 & CPU_OPT_WIDTH)
#define BIT_MASK_BA_CONTENT_H                 	0xffffffffL
#define BIT_BA_CONTENT_H(x)                   	(((x) & BIT_MASK_BA_CONTENT_H) << BIT_SHIFT_BA_CONTENT_H)


#define BIT_SHIFT_BA_CONTENT_L                	0
#define BIT_MASK_BA_CONTENT_L                 	0xffffffffL
#define BIT_BA_CONTENT_L(x)                   	(((x) & BIT_MASK_BA_CONTENT_L) << BIT_SHIFT_BA_CONTENT_L)


//2 REG_LBDLY                              (Offset 0x0660)


#define BIT_SHIFT_LBDLY                       	0
#define BIT_MASK_LBDLY                        	0x1f
#define BIT_LBDLY(x)                          	(((x) & BIT_MASK_LBDLY) << BIT_SHIFT_LBDLY)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_WMAC_BACAM_RPMEN                   (Offset 0x0661)

#define BIT_WMAC_BACAM_RPMEN                  	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RXERR_RPT                          (Offset 0x0664)


#define BIT_SHIFT_RXERR_RPT_SEL               	28
#define BIT_MASK_RXERR_RPT_SEL                	0xf
#define BIT_RXERR_RPT_SEL(x)                  	(((x) & BIT_MASK_RXERR_RPT_SEL) << BIT_SHIFT_RXERR_RPT_SEL)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_RXERR_RPT                          (Offset 0x0664)


#define BIT_SHIFT_RXERR_RPT_SEL_V1_3_0        	28
#define BIT_MASK_RXERR_RPT_SEL_V1_3_0         	0xf
#define BIT_RXERR_RPT_SEL_V1_3_0(x)           	(((x) & BIT_MASK_RXERR_RPT_SEL_V1_3_0) << BIT_SHIFT_RXERR_RPT_SEL_V1_3_0)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RXERR_RPT                          (Offset 0x0664)

#define BIT_RXERR_RPT_RST                     	BIT(27)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_RXERR_RPT                          (Offset 0x0664)

#define BIT_RXERR_RPT_SEL_V1_4                	BIT(26)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RXERR_RPT                          (Offset 0x0664)

#define BIT_W1S                               	BIT(23)
#define BIT_UD_Select_BSSID                   	BIT(22)

#define BIT_SHIFT_UD_Sub_Type                 	18
#define BIT_MASK_UD_Sub_Type                  	0xf
#define BIT_UD_Sub_Type(x)                    	(((x) & BIT_MASK_UD_Sub_Type) << BIT_SHIFT_UD_Sub_Type)


#define BIT_SHIFT_UD_Type                     	16
#define BIT_MASK_UD_Type                      	0x3
#define BIT_UD_Type(x)                        	(((x) & BIT_MASK_UD_Type) << BIT_SHIFT_UD_Type)


#define BIT_SHIFT_RPT_COUNTER                 	0
#define BIT_MASK_RPT_COUNTER                  	0xffff
#define BIT_RPT_COUNTER(x)                    	(((x) & BIT_MASK_RPT_COUNTER) << BIT_SHIFT_RPT_COUNTER)


//2 REG_WMAC_TRXPTCL_CTL                   (Offset 0x0668)


#define BIT_SHIFT_ACKBA_TYPSEL                	(60 & CPU_OPT_WIDTH)
#define BIT_MASK_ACKBA_TYPSEL                 	0xf
#define BIT_ACKBA_TYPSEL(x)                   	(((x) & BIT_MASK_ACKBA_TYPSEL) << BIT_SHIFT_ACKBA_TYPSEL)


#define BIT_SHIFT_ACKBA_ACKPCHK               	(56 & CPU_OPT_WIDTH)
#define BIT_MASK_ACKBA_ACKPCHK                	0xf
#define BIT_ACKBA_ACKPCHK(x)                  	(((x) & BIT_MASK_ACKBA_ACKPCHK) << BIT_SHIFT_ACKBA_ACKPCHK)


#define BIT_SHIFT_ACKBAR_TYPESEL              	(48 & CPU_OPT_WIDTH)
#define BIT_MASK_ACKBAR_TYPESEL               	0xff
#define BIT_ACKBAR_TYPESEL(x)                 	(((x) & BIT_MASK_ACKBAR_TYPESEL) << BIT_SHIFT_ACKBAR_TYPESEL)


#define BIT_SHIFT_ACKBAR_ACKPCHK              	(44 & CPU_OPT_WIDTH)
#define BIT_MASK_ACKBAR_ACKPCHK               	0xf
#define BIT_ACKBAR_ACKPCHK(x)                 	(((x) & BIT_MASK_ACKBAR_ACKPCHK) << BIT_SHIFT_ACKBAR_ACKPCHK)

#define BIT_DIS_TXBA_AMPDUFCSERR              	BIT(39)
#define BIT_DIS_TXBA_RXBARINFULL              	BIT(38)
#define BIT_DIS_TXCFE_INFULL                  	BIT(37)
#define BIT_DIS_TXCTS_INFULL                  	BIT(36)
#define BIT_EN_TXACKBA_IN_TX_RDG              	BIT(35)
#define BIT_EN_TXACKBA_IN_TXOP                	BIT(34)
#define BIT_EN_TXCTS_IN_RXNAV                 	BIT(33)
#define BIT_EN_TXCTS_INTXOP                   	BIT(32)
#define BIT_BLK_EDCA_BBSLP                    	BIT(31)
#define BIT_BLK_EDCA_BBSBY                    	BIT(30)
#define BIT_ACKTO_BLOCK_SCH_EN                	BIT(27)
#define BIT_EIFS_BLOCK_SCH_EN                 	BIT(26)
#define BIT_PLCPCHK_RST_EIFS                  	BIT(25)
#define BIT_CCA_RST_EIFS                      	BIT(24)
#define BIT_DIS_UPD_MYRXPKTNAV                	BIT(23)
#define BIT_EARLY_TXBA                        	BIT(22)

#define BIT_SHIFT_RESP_CHNBUSY                	20
#define BIT_MASK_RESP_CHNBUSY                 	0x3
#define BIT_RESP_CHNBUSY(x)                   	(((x) & BIT_MASK_RESP_CHNBUSY) << BIT_SHIFT_RESP_CHNBUSY)

#define BIT_RESP_DCTS_EN                      	BIT(19)
#define BIT_RESP_DCFE_EN                      	BIT(18)
#define BIT_RESP_SPLCPEN                      	BIT(17)
#define BIT_RESP_SGIEN                        	BIT(16)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WMAC_TRXPTCL_CTL                   (Offset 0x0668)

#define BIT_RESP_LDPC_EN                      	BIT(15)
#define BIT_DIS_RESP_ACKINCCA                 	BIT(14)
#define BIT_DIS_RESP_CTSINCCA                 	BIT(13)

#define BIT_SHIFT_r_WMAC_SECOND_CCA_TIMER     	10
#define BIT_MASK_r_WMAC_SECOND_CCA_TIMER      	0x7
#define BIT_r_WMAC_SECOND_CCA_TIMER(x)        	(((x) & BIT_MASK_r_WMAC_SECOND_CCA_TIMER) << BIT_SHIFT_r_WMAC_SECOND_CCA_TIMER)


#define BIT_SHIFT_RFMOD                       	7
#define BIT_MASK_RFMOD                        	0x3
#define BIT_RFMOD(x)                          	(((x) & BIT_MASK_RFMOD) << BIT_SHIFT_RFMOD)


#define BIT_SHIFT_RESP_CTS_DYNBW_SEL          	5
#define BIT_MASK_RESP_CTS_DYNBW_SEL           	0x3
#define BIT_RESP_CTS_DYNBW_SEL(x)             	(((x) & BIT_MASK_RESP_CTS_DYNBW_SEL) << BIT_SHIFT_RESP_CTS_DYNBW_SEL)

#define BIT_DLY_TX_WAIT_RXANTSEL              	BIT(4)
#define BIT_TXRESP_BY_RXANTSEL                	BIT(3)
#define BIT_RESP_EARLY_TXACK_RWEPTKIP         	BIT(2)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_WMAC_TRXPTCL_CTL                   (Offset 0x0668)


#define BIT_SHIFT_ORIG_DCTS_CHK               	0
#define BIT_MASK_ORIG_DCTS_CHK                	0x3
#define BIT_ORIG_DCTS_CHK(x)                  	(((x) & BIT_MASK_ORIG_DCTS_CHK) << BIT_SHIFT_ORIG_DCTS_CHK)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_CAMCMD                             (Offset 0x0670)

#define BIT_SECCAM_POLLING                    	BIT(31)
#define BIT_SECCAM_CLR                        	BIT(30)
#define BIT_MFBCAM_CLR                        	BIT(29)
#define BIT_SECCAM_WE                         	BIT(16)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_CAMCMD                             (Offset 0x0670)


#define BIT_SHIFT_SECCAM_ADDR_V1              	0
#define BIT_MASK_SECCAM_ADDR_V1               	0xff
#define BIT_SECCAM_ADDR_V1(x)                 	(((x) & BIT_MASK_SECCAM_ADDR_V1) << BIT_SHIFT_SECCAM_ADDR_V1)


#define BIT_SHIFT_WKFCAM_NUM                  	0
#define BIT_MASK_WKFCAM_NUM                   	0x7f
#define BIT_WKFCAM_NUM(x)                     	(((x) & BIT_MASK_WKFCAM_NUM) << BIT_SHIFT_WKFCAM_NUM)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_CAMCMD                             (Offset 0x0670)


#define BIT_SHIFT_SECCAM_ADDR                 	0
#define BIT_MASK_SECCAM_ADDR                  	0xff
#define BIT_SECCAM_ADDR(x)                    	(((x) & BIT_MASK_SECCAM_ADDR) << BIT_SHIFT_SECCAM_ADDR)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_CAMWRITE                           (Offset 0x0674)


#define BIT_SHIFT_CAMW_DATA                   	0
#define BIT_MASK_CAMW_DATA                    	0xffffffffL
#define BIT_CAMW_DATA(x)                      	(((x) & BIT_MASK_CAMW_DATA) << BIT_SHIFT_CAMW_DATA)


//2 REG_CAMREAD                            (Offset 0x0678)


#define BIT_SHIFT_CAMR_DATA                   	0
#define BIT_MASK_CAMR_DATA                    	0xffffffffL
#define BIT_CAMR_DATA(x)                      	(((x) & BIT_MASK_CAMR_DATA) << BIT_SHIFT_CAMR_DATA)


//2 REG_CAMDBG                             (Offset 0x067C)

#define BIT_SECCAM_INFO                       	BIT(31)
#define BIT_SEC_KEYFOUND                      	BIT(15)

#define BIT_SHIFT_CAMDBG_SEC_TYPE             	12
#define BIT_MASK_CAMDBG_SEC_TYPE              	0x7
#define BIT_CAMDBG_SEC_TYPE(x)                	(((x) & BIT_MASK_CAMDBG_SEC_TYPE) << BIT_SHIFT_CAMDBG_SEC_TYPE)


#define BIT_SHIFT_CAMDBG_MIC_KEY_IDX          	5
#define BIT_MASK_CAMDBG_MIC_KEY_IDX           	0x1f
#define BIT_CAMDBG_MIC_KEY_IDX(x)             	(((x) & BIT_MASK_CAMDBG_MIC_KEY_IDX) << BIT_SHIFT_CAMDBG_MIC_KEY_IDX)


#define BIT_SHIFT_CAMDBG_SEC_KEY_IDX          	0
#define BIT_MASK_CAMDBG_SEC_KEY_IDX           	0x1f
#define BIT_CAMDBG_SEC_KEY_IDX(x)             	(((x) & BIT_MASK_CAMDBG_SEC_KEY_IDX) << BIT_SHIFT_CAMDBG_SEC_KEY_IDX)


//2 REG_SECCFG                             (Offset 0x0680)

#define BIT_DIS_GCLK_WAPI                     	BIT(15)
#define BIT_DIS_GCLK_AES                      	BIT(14)
#define BIT_DIS_GCLK_TKIP                     	BIT(13)
#define BIT_CHK_KEYID                         	BIT(8)
#define BIT_RXBCUSEDK                         	BIT(7)
#define BIT_TXBCUSEDK                         	BIT(6)
#define BIT_NOSKMC                            	BIT(5)
#define BIT_SKBYA2                            	BIT(4)
#define BIT_RXDEC                             	BIT(3)
#define BIT_TXENC                             	BIT(2)
#define BIT_RXUHUSEDK                         	BIT(1)
#define BIT_TXUHUSEDK                         	BIT(0)

#endif


#if (RTL8814A_SUPPORT)


//2 REG_WOW_CTRL                           (Offset 0x0690)

#define BIT_WOWHCI                            	BIT(5)

#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WOW_CTRL                           (Offset 0x0690)

#define BIT_PSF_BSSIDSEL                      	BIT(4)
#define BIT_UWF                               	BIT(3)
#define BIT_MAGIC                             	BIT(2)
#define BIT_WOWEN                             	BIT(1)
#define BIT_FORCE_WAKEUP                      	BIT(0)

//2 REG_PS_RX_INFO                         (Offset 0x0692)

#define BIT_RXCTRLIN0                         	BIT(4)
#define BIT_RXMGTIN0                          	BIT(3)
#define BIT_RXDATAIN2                         	BIT(2)
#define BIT_RXDATAIN1                         	BIT(1)
#define BIT_RXDATAIN0                         	BIT(0)

//2 REG_WMMPS_UAPSD_TID                    (Offset 0x0693)

#define BIT_WMMPS_UAPSD_TID7                  	BIT(7)
#define BIT_WMMPS_UAPSD_TID6                  	BIT(6)
#define BIT_WMMPS_UAPSD_TID5                  	BIT(5)
#define BIT_WMMPS_UAPSD_TID4                  	BIT(4)
#define BIT_WMMPS_UAPSD_TID3                  	BIT(3)
#define BIT_WMMPS_UAPSD_TID2                  	BIT(2)
#define BIT_WMMPS_UAPSD_TID1                  	BIT(1)
#define BIT_WMMPS_UAPSD_TID0                  	BIT(0)

//2 REG_LPNAV_CTRL                         (Offset 0x0694)

#define BIT_LPNAV_EN                          	BIT(31)

#define BIT_SHIFT_LPNAV_EARLY                 	16
#define BIT_MASK_LPNAV_EARLY                  	0x7fff
#define BIT_LPNAV_EARLY(x)                    	(((x) & BIT_MASK_LPNAV_EARLY) << BIT_SHIFT_LPNAV_EARLY)


#define BIT_SHIFT_LPNAV_TH                    	0
#define BIT_MASK_LPNAV_TH                     	0xffff
#define BIT_LPNAV_TH(x)                       	(((x) & BIT_MASK_LPNAV_TH) << BIT_SHIFT_LPNAV_TH)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_WKFMCAM_CMD                        (Offset 0x0698)

#define BIT_WKFCAM_WE                         	BIT(16)

#define BIT_SHIFT_WKFCAM_ADDR                 	0
#define BIT_MASK_WKFCAM_ADDR                  	0x7f
#define BIT_WKFCAM_ADDR(x)                    	(((x) & BIT_MASK_WKFCAM_ADDR) << BIT_SHIFT_WKFCAM_ADDR)


//2 REG_WKFMCAM_RWD                        (Offset 0x069C)


#define BIT_SHIFT_WKFMCAM_RWD                 	0
#define BIT_MASK_WKFMCAM_RWD                  	0xffffffffL
#define BIT_WKFMCAM_RWD(x)                    	(((x) & BIT_MASK_WKFMCAM_RWD) << BIT_SHIFT_WKFMCAM_RWD)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_RXFLTMAP0                          (Offset 0x06A0)

#define BIT_MGTFLT13En                        	BIT(13)
#define BIT_MGTFLT12En                        	BIT(12)
#define BIT_MGTFLT11En                        	BIT(11)
#define BIT_MGTFLT10En                        	BIT(10)
#define BIT_MGTFLT9En                         	BIT(9)
#define BIT_MGTFLT8En                         	BIT(8)
#define BIT_MGTFLT5En                         	BIT(5)
#define BIT_MGTFLT4En                         	BIT(4)
#define BIT_MGTFLT3En                         	BIT(3)
#define BIT_MGTFLT2En                         	BIT(2)
#define BIT_MGTFLT1En                         	BIT(1)
#define BIT_MGTFLT0En                         	BIT(0)

//2 REG_RXFLTMAP1                          (Offset 0x06A2)

#define BIT_CTRLFLT15En                       	BIT(15)
#define BIT_CTRLFLT14En                       	BIT(14)
#define BIT_CTRLFLT13En                       	BIT(13)
#define BIT_CTRLFLT12En                       	BIT(12)
#define BIT_CTRLFLT11En                       	BIT(11)
#define BIT_CTRLFLT10En                       	BIT(10)
#define BIT_CTRLFLT9En                        	BIT(9)
#define BIT_CTRLFLT8En                        	BIT(8)
#define BIT_CTRLFLT7En                        	BIT(7)
#define BIT_CTRLFLT6En                        	BIT(6)

//2 REG_RXFLTMAP                           (Offset 0x06A4)

#define BIT_DATAFLT15En                       	BIT(15)
#define BIT_DATAFLT14En                       	BIT(14)
#define BIT_DATAFLT13En                       	BIT(13)
#define BIT_DATAFLT12En                       	BIT(12)
#define BIT_DATAFLT11En                       	BIT(11)
#define BIT_DATAFLT10En                       	BIT(10)
#define BIT_DATAFLT9En                        	BIT(9)
#define BIT_DATAFLT8En                        	BIT(8)
#define BIT_DATAFLT7En                        	BIT(7)
#define BIT_DATAFLT6En                        	BIT(6)
#define BIT_DATAFLT5En                        	BIT(5)
#define BIT_DATAFLT4En                        	BIT(4)
#define BIT_DATAFLT3En                        	BIT(3)
#define BIT_DATAFLT2En                        	BIT(2)
#define BIT_DATAFLT1En                        	BIT(1)
#define BIT_DATAFLT0En                        	BIT(0)

//2 REG_BCN_PSR_RPT                        (Offset 0x06A8)


#define BIT_SHIFT_DTIM_CNT                    	24
#define BIT_MASK_DTIM_CNT                     	0xff
#define BIT_DTIM_CNT(x)                       	(((x) & BIT_MASK_DTIM_CNT) << BIT_SHIFT_DTIM_CNT)


#define BIT_SHIFT_DTIM_PERIOD                 	16
#define BIT_MASK_DTIM_PERIOD                  	0xff
#define BIT_DTIM_PERIOD(x)                    	(((x) & BIT_MASK_DTIM_PERIOD) << BIT_SHIFT_DTIM_PERIOD)

#define BIT_DTIM                              	BIT(15)
#define BIT_TIM                               	BIT(14)

#define BIT_SHIFT_PS_AID_0                    	0
#define BIT_MASK_PS_AID_0                     	0x7ff
#define BIT_PS_AID_0(x)                       	(((x) & BIT_MASK_PS_AID_0) << BIT_SHIFT_PS_AID_0)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_FLC_RPC                            (Offset 0x06AC)


#define BIT_SHIFT_FLC_RPC                     	0
#define BIT_MASK_FLC_RPC                      	0xff
#define BIT_FLC_RPC(x)                        	(((x) & BIT_MASK_FLC_RPC) << BIT_SHIFT_FLC_RPC)


//2 REG_FLC_RPCT                           (Offset 0x06AD)


#define BIT_SHIFT_FLC_RPCT                    	0
#define BIT_MASK_FLC_RPCT                     	0xff
#define BIT_FLC_RPCT(x)                       	(((x) & BIT_MASK_FLC_RPCT) << BIT_SHIFT_FLC_RPCT)


//2 REG_FLC_PTS                            (Offset 0x06AE)

#define BIT_CMF                               	BIT(2)
#define BIT_CCF                               	BIT(1)
#define BIT_CDF                               	BIT(0)

//2 REG_FLC_TRPC                           (Offset 0x06AF)

#define BIT_FLC_RPCT                          	BIT(7)
#define BIT_MODE                              	BIT(6)

#define BIT_SHIFT_TRPCD                       	0
#define BIT_MASK_TRPCD                        	0x3f
#define BIT_TRPCD(x)                          	(((x) & BIT_MASK_TRPCD) << BIT_SHIFT_TRPCD)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_RXPKTMON_CTRL                      (Offset 0x06B0)


#define BIT_SHIFT_RXBKQPKT_SEQ                	20
#define BIT_MASK_RXBKQPKT_SEQ                 	0xf
#define BIT_RXBKQPKT_SEQ(x)                   	(((x) & BIT_MASK_RXBKQPKT_SEQ) << BIT_SHIFT_RXBKQPKT_SEQ)


#define BIT_SHIFT_RXBEQPKT_SEQ                	16
#define BIT_MASK_RXBEQPKT_SEQ                 	0xf
#define BIT_RXBEQPKT_SEQ(x)                   	(((x) & BIT_MASK_RXBEQPKT_SEQ) << BIT_SHIFT_RXBEQPKT_SEQ)


#define BIT_SHIFT_RXVIQPKT_SEQ                	12
#define BIT_MASK_RXVIQPKT_SEQ                 	0xf
#define BIT_RXVIQPKT_SEQ(x)                   	(((x) & BIT_MASK_RXVIQPKT_SEQ) << BIT_SHIFT_RXVIQPKT_SEQ)


#define BIT_SHIFT_RXVOQPKT_SEQ                	8
#define BIT_MASK_RXVOQPKT_SEQ                 	0xf
#define BIT_RXVOQPKT_SEQ(x)                   	(((x) & BIT_MASK_RXVOQPKT_SEQ) << BIT_SHIFT_RXVOQPKT_SEQ)

#define BIT_RXBKQPKT_ERR                      	BIT(7)
#define BIT_RXBEQPKT_ERR                      	BIT(6)
#define BIT_RXVIQPKT_ERR                      	BIT(5)
#define BIT_RXVOQPKT_ERR                      	BIT(4)
#define BIT_RXDMA_MON_EN                      	BIT(2)
#define BIT_RXPKT_MON_RST                     	BIT(1)
#define BIT_RXPKT_MON_EN                      	BIT(0)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_STATE_MON                          (Offset 0x06B4)


#define BIT_SHIFT_DMA_MON_EN                  	24
#define BIT_MASK_DMA_MON_EN                   	0x1f
#define BIT_DMA_MON_EN(x)                     	(((x) & BIT_MASK_DMA_MON_EN) << BIT_SHIFT_DMA_MON_EN)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_STATE_MON                          (Offset 0x06B4)


#define BIT_SHIFT_STATE_SEL                   	24
#define BIT_MASK_STATE_SEL                    	0x1f
#define BIT_STATE_SEL(x)                      	(((x) & BIT_MASK_STATE_SEL) << BIT_SHIFT_STATE_SEL)


#define BIT_SHIFT_STATE_INFO                  	8
#define BIT_MASK_STATE_INFO                   	0xff
#define BIT_STATE_INFO(x)                     	(((x) & BIT_MASK_STATE_INFO) << BIT_SHIFT_STATE_INFO)

#define BIT_UPD_NXT_STATE                     	BIT(7)

#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_STATE_MON                          (Offset 0x06B4)


#define BIT_SHIFT_PKT_MON_EN                  	0
#define BIT_MASK_PKT_MON_EN                   	0x7f
#define BIT_PKT_MON_EN(x)                     	(((x) & BIT_MASK_PKT_MON_EN) << BIT_SHIFT_PKT_MON_EN)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_STATE_MON                          (Offset 0x06B4)


#define BIT_SHIFT_CUR_STATE                   	0
#define BIT_MASK_CUR_STATE                    	0x7f
#define BIT_CUR_STATE(x)                      	(((x) & BIT_MASK_CUR_STATE) << BIT_SHIFT_CUR_STATE)


//2 REG_ERROR_MON                          (Offset 0x06B8)

#define BIT_MACRX_ERR_1                       	BIT(17)
#define BIT_MACRX_ERR_0                       	BIT(16)
#define BIT_MACTX_ERR_3                       	BIT(3)
#define BIT_MACTX_ERR_2                       	BIT(2)
#define BIT_MACTX_ERR_1                       	BIT(1)
#define BIT_MACTX_ERR_0                       	BIT(0)

//2 REG_SEARCH_MACID                       (Offset 0x06BC)

#define BIT_EN_TXRPTBUF_CLK                   	BIT(31)

#define BIT_SHIFT_INFO_INDEX_OFFSET           	16
#define BIT_MASK_INFO_INDEX_OFFSET            	0x1fff
#define BIT_INFO_INDEX_OFFSET(x)              	(((x) & BIT_MASK_INFO_INDEX_OFFSET) << BIT_SHIFT_INFO_INDEX_OFFSET)

#define BIT_DIS_INFOSRCH                      	BIT(14)
#define BIT_DISABLE_B0                        	BIT(13)

#define BIT_SHIFT_INFO_ADDR_OFFSET            	0
#define BIT_MASK_INFO_ADDR_OFFSET             	0x1fff
#define BIT_INFO_ADDR_OFFSET(x)               	(((x) & BIT_MASK_INFO_ADDR_OFFSET) << BIT_SHIFT_INFO_ADDR_OFFSET)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_BT_COEX_TABLE                      (Offset 0x06C0)

#define BIT_PRI_MASK_RX_RESP                  	BIT(126)
#define BIT_PRI_MASK_RXOFDM                   	BIT(125)
#define BIT_PRI_MASK_RXCCK                    	BIT(124)

#define BIT_SHIFT_PRI_MASK_TXAC               	(117 & CPU_OPT_WIDTH)
#define BIT_MASK_PRI_MASK_TXAC                	0x7f
#define BIT_PRI_MASK_TXAC(x)                  	(((x) & BIT_MASK_PRI_MASK_TXAC) << BIT_SHIFT_PRI_MASK_TXAC)


#define BIT_SHIFT_PRI_MASK_NAV                	(109 & CPU_OPT_WIDTH)
#define BIT_MASK_PRI_MASK_NAV                 	0xff
#define BIT_PRI_MASK_NAV(x)                   	(((x) & BIT_MASK_PRI_MASK_NAV) << BIT_SHIFT_PRI_MASK_NAV)

#define BIT_PRI_MASK_CCK                      	BIT(108)
#define BIT_PRI_MASK_OFDM                     	BIT(107)
#define BIT_PRI_MASK_RTY                      	BIT(106)

#define BIT_SHIFT_PRI_MASK_NUM                	(102 & CPU_OPT_WIDTH)
#define BIT_MASK_PRI_MASK_NUM                 	0xf
#define BIT_PRI_MASK_NUM(x)                   	(((x) & BIT_MASK_PRI_MASK_NUM) << BIT_SHIFT_PRI_MASK_NUM)


#define BIT_SHIFT_PRI_MASK_TYPE               	(98 & CPU_OPT_WIDTH)
#define BIT_MASK_PRI_MASK_TYPE                	0xf
#define BIT_PRI_MASK_TYPE(x)                  	(((x) & BIT_MASK_PRI_MASK_TYPE) << BIT_SHIFT_PRI_MASK_TYPE)

#define BIT_OOB                               	BIT(97)
#define BIT_ANT_SEL                           	BIT(96)

#define BIT_SHIFT_BREAK_TABLE_2               	(80 & CPU_OPT_WIDTH)
#define BIT_MASK_BREAK_TABLE_2                	0xffff
#define BIT_BREAK_TABLE_2(x)                  	(((x) & BIT_MASK_BREAK_TABLE_2) << BIT_SHIFT_BREAK_TABLE_2)


#define BIT_SHIFT_BREAK_TABLE_1               	(64 & CPU_OPT_WIDTH)
#define BIT_MASK_BREAK_TABLE_1                	0xffff
#define BIT_BREAK_TABLE_1(x)                  	(((x) & BIT_MASK_BREAK_TABLE_1) << BIT_SHIFT_BREAK_TABLE_1)


#define BIT_SHIFT_COEX_TABLE_2                	(32 & CPU_OPT_WIDTH)
#define BIT_MASK_COEX_TABLE_2                 	0xffffffffL
#define BIT_COEX_TABLE_2(x)                   	(((x) & BIT_MASK_COEX_TABLE_2) << BIT_SHIFT_COEX_TABLE_2)


#define BIT_SHIFT_COEX_TABLE_1                	0
#define BIT_MASK_COEX_TABLE_1                 	0xffffffffL
#define BIT_COEX_TABLE_1(x)                   	(((x) & BIT_MASK_COEX_TABLE_1) << BIT_SHIFT_COEX_TABLE_1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_RXCMD_0                            (Offset 0x06D0)

#define BIT_RXCMD_EN                          	BIT(31)

#define BIT_SHIFT_RXCMD_INFO                  	0
#define BIT_MASK_RXCMD_INFO                   	0x7fffffffL
#define BIT_RXCMD_INFO(x)                     	(((x) & BIT_MASK_RXCMD_INFO) << BIT_SHIFT_RXCMD_INFO)


//2 REG_RXCMD_1                            (Offset 0x06D4)


#define BIT_SHIFT_RXCMD_PRD                   	0
#define BIT_MASK_RXCMD_PRD                    	0xffff
#define BIT_RXCMD_PRD(x)                      	(((x) & BIT_MASK_RXCMD_PRD) << BIT_SHIFT_RXCMD_PRD)


//2 REG_WMAC_RESP_TXINFO                   (Offset 0x06D8)


#define BIT_SHIFT_WMAC_RESP_MFB               	25
#define BIT_MASK_WMAC_RESP_MFB                	0x7f
#define BIT_WMAC_RESP_MFB(x)                  	(((x) & BIT_MASK_WMAC_RESP_MFB) << BIT_SHIFT_WMAC_RESP_MFB)


#define BIT_SHIFT_WMAC_ANTINF_SEL             	23
#define BIT_MASK_WMAC_ANTINF_SEL              	0x3
#define BIT_WMAC_ANTINF_SEL(x)                	(((x) & BIT_MASK_WMAC_ANTINF_SEL) << BIT_SHIFT_WMAC_ANTINF_SEL)


#define BIT_SHIFT_WMAC_ANTSEL_SEL             	21
#define BIT_MASK_WMAC_ANTSEL_SEL              	0x3
#define BIT_WMAC_ANTSEL_SEL(x)                	(((x) & BIT_MASK_WMAC_ANTSEL_SEL) << BIT_SHIFT_WMAC_ANTSEL_SEL)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WMAC_RESP_TXINFO                   (Offset 0x06D8)


#define BIT_SHIFT_RESP_TXPOWER                	18
#define BIT_MASK_RESP_TXPOWER                 	0x7
#define BIT_RESP_TXPOWER(x)                   	(((x) & BIT_MASK_RESP_TXPOWER) << BIT_SHIFT_RESP_TXPOWER)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_WMAC_RESP_TXINFO                   (Offset 0x06D8)


#define BIT_SHIFT_r_WMAC_RESP_TXPOWER         	18
#define BIT_MASK_r_WMAC_RESP_TXPOWER          	0x7
#define BIT_r_WMAC_RESP_TXPOWER(x)            	(((x) & BIT_MASK_r_WMAC_RESP_TXPOWER) << BIT_SHIFT_r_WMAC_RESP_TXPOWER)


#endif


#if (RTL8192E_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WMAC_RESP_TXINFO                   (Offset 0x06D8)


#define BIT_SHIFT_RESP_TXAGC_B                	13
#define BIT_MASK_RESP_TXAGC_B                 	0x1f
#define BIT_RESP_TXAGC_B(x)                   	(((x) & BIT_MASK_RESP_TXAGC_B) << BIT_SHIFT_RESP_TXAGC_B)


#define BIT_SHIFT_RESP_TXAGC_A                	8
#define BIT_MASK_RESP_TXAGC_A                 	0x1f
#define BIT_RESP_TXAGC_A(x)                   	(((x) & BIT_MASK_RESP_TXAGC_A) << BIT_SHIFT_RESP_TXAGC_A)

#define BIT_RESP_ANTSEL_B                     	BIT(7)
#define BIT_RESP_ANTSEL_A                     	BIT(6)

#define BIT_SHIFT_RESP_TXANT_CCK              	4
#define BIT_MASK_RESP_TXANT_CCK               	0x3
#define BIT_RESP_TXANT_CCK(x)                 	(((x) & BIT_MASK_RESP_TXANT_CCK) << BIT_SHIFT_RESP_TXANT_CCK)


#define BIT_SHIFT_RESP_TXANT_L                	2
#define BIT_MASK_RESP_TXANT_L                 	0x3
#define BIT_RESP_TXANT_L(x)                   	(((x) & BIT_MASK_RESP_TXANT_L) << BIT_SHIFT_RESP_TXANT_L)


#define BIT_SHIFT_RESP_TXANT_HT               	0
#define BIT_MASK_RESP_TXANT_HT                	0x3
#define BIT_RESP_TXANT_HT(x)                  	(((x) & BIT_MASK_RESP_TXANT_HT) << BIT_SHIFT_RESP_TXANT_HT)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_WMAC_RESP_TXINFO                   (Offset 0x06D8)


#define BIT_SHIFT_WMAC_RESP_TXANT             	0
#define BIT_MASK_WMAC_RESP_TXANT              	0x3ffff
#define BIT_WMAC_RESP_TXANT(x)                	(((x) & BIT_MASK_WMAC_RESP_TXANT) << BIT_SHIFT_WMAC_RESP_TXANT)


//2 REG_BBPSF_CTRL                         (Offset 0x06DC)

#define BIT_WMAC_USE_NDPARATE                 	BIT(30)

#define BIT_SHIFT_WMAC_CSI_RATE               	24
#define BIT_MASK_WMAC_CSI_RATE                	0x3f
#define BIT_WMAC_CSI_RATE(x)                  	(((x) & BIT_MASK_WMAC_CSI_RATE) << BIT_SHIFT_WMAC_CSI_RATE)


#define BIT_SHIFT_WMAC_RESP_TXRATE            	16
#define BIT_MASK_WMAC_RESP_TXRATE             	0xff
#define BIT_WMAC_RESP_TXRATE(x)               	(((x) & BIT_MASK_WMAC_RESP_TXRATE) << BIT_SHIFT_WMAC_RESP_TXRATE)

#define BIT_BBPSF_MHCHKEN                     	BIT(4)
#define BIT_BBPSF_ERRCHKEN                    	BIT(3)

#define BIT_SHIFT_BBPSF_ERRTHR                	0
#define BIT_MASK_BBPSF_ERRTHR                 	0x7
#define BIT_BBPSF_ERRTHR(x)                   	(((x) & BIT_MASK_BBPSF_ERRTHR) << BIT_SHIFT_BBPSF_ERRTHR)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_P2P_RX_BCN_NOA                     (Offset 0x06E0)

#define BIT_NOA_PARSER_EN                     	BIT(15)
#define BIT_BSSID_SEL                         	BIT(14)

#define BIT_SHIFT_P2P_OUI_TYPE                	0
#define BIT_MASK_P2P_OUI_TYPE                 	0xff
#define BIT_P2P_OUI_TYPE(x)                   	(((x) & BIT_MASK_P2P_OUI_TYPE) << BIT_SHIFT_P2P_OUI_TYPE)


//2 REG_ASSOCIATED_BFMER0_INFO             (Offset 0x06E4)


#define BIT_SHIFT_r_WMAC_TXCSI_AID0           	(48 & CPU_OPT_WIDTH)
#define BIT_MASK_r_WMAC_TXCSI_AID0            	0x1ff
#define BIT_r_WMAC_TXCSI_AID0(x)              	(((x) & BIT_MASK_r_WMAC_TXCSI_AID0) << BIT_SHIFT_r_WMAC_TXCSI_AID0)


#define BIT_SHIFT_r_WMAC_SOUNDING_RXADD_R0    	0
#define BIT_MASK_r_WMAC_SOUNDING_RXADD_R0     	0xffffffffffffL
#define BIT_r_WMAC_SOUNDING_RXADD_R0(x)       	(((x) & BIT_MASK_r_WMAC_SOUNDING_RXADD_R0) << BIT_SHIFT_r_WMAC_SOUNDING_RXADD_R0)


//2 REG_ASSOCIATED_BFMER1_INFO             (Offset 0x06EC)


#define BIT_SHIFT_r_WMAC_TXCSI_AID1           	(48 & CPU_OPT_WIDTH)
#define BIT_MASK_r_WMAC_TXCSI_AID1            	0x1ff
#define BIT_r_WMAC_TXCSI_AID1(x)              	(((x) & BIT_MASK_r_WMAC_TXCSI_AID1) << BIT_SHIFT_r_WMAC_TXCSI_AID1)


#define BIT_SHIFT_r_WMAC_SOUNDING_RXADD_R1    	0
#define BIT_MASK_r_WMAC_SOUNDING_RXADD_R1     	0xffffffffffffL
#define BIT_r_WMAC_SOUNDING_RXADD_R1(x)       	(((x) & BIT_MASK_r_WMAC_SOUNDING_RXADD_R1) << BIT_SHIFT_r_WMAC_SOUNDING_RXADD_R1)


//2 REG_TX_CSI_RPT_PARAM_BW20              (Offset 0x06F4)


#define BIT_SHIFT_r_WMAC_BFINFO_20M_1         	16
#define BIT_MASK_r_WMAC_BFINFO_20M_1          	0xfff
#define BIT_r_WMAC_BFINFO_20M_1(x)            	(((x) & BIT_MASK_r_WMAC_BFINFO_20M_1) << BIT_SHIFT_r_WMAC_BFINFO_20M_1)


#define BIT_SHIFT_r_WMAC_BFINFO_20M_0         	0
#define BIT_MASK_r_WMAC_BFINFO_20M_0          	0xfff
#define BIT_r_WMAC_BFINFO_20M_0(x)            	(((x) & BIT_MASK_r_WMAC_BFINFO_20M_0) << BIT_SHIFT_r_WMAC_BFINFO_20M_0)


//2 REG_TX_CSI_RPT_PARAM_BW40              (Offset 0x06F8)


#define BIT_SHIFT_r_WMAC_BFINFO_40M_1         	13
#define BIT_MASK_r_WMAC_BFINFO_40M_1          	0x7fff
#define BIT_r_WMAC_BFINFO_40M_1(x)            	(((x) & BIT_MASK_r_WMAC_BFINFO_40M_1) << BIT_SHIFT_r_WMAC_BFINFO_40M_1)


#define BIT_SHIFT_r_WMAC_BFINFO_40M_0         	0
#define BIT_MASK_r_WMAC_BFINFO_40M_0          	0xfff
#define BIT_r_WMAC_BFINFO_40M_0(x)            	(((x) & BIT_MASK_r_WMAC_BFINFO_40M_0) << BIT_SHIFT_r_WMAC_BFINFO_40M_0)


//2 REG_TX_CSI_RPT_PARAM_BW80              (Offset 0x06FC)


#define BIT_SHIFT_r_WMAC_BFINFO_80M_1         	16
#define BIT_MASK_r_WMAC_BFINFO_80M_1          	0xfff
#define BIT_r_WMAC_BFINFO_80M_1(x)            	(((x) & BIT_MASK_r_WMAC_BFINFO_80M_1) << BIT_SHIFT_r_WMAC_BFINFO_80M_1)


#define BIT_SHIFT_r_WMAC_BFINFO_80M_0         	0
#define BIT_MASK_r_WMAC_BFINFO_80M_0          	0xfff
#define BIT_r_WMAC_BFINFO_80M_0(x)            	(((x) & BIT_MASK_r_WMAC_BFINFO_80M_0) << BIT_SHIFT_r_WMAC_BFINFO_80M_0)


//2 REG_MACID1                             (Offset 0x0700)


#define BIT_SHIFT_MACID1                      	0
#define BIT_MASK_MACID1                       	0xffffffffffffL
#define BIT_MACID1(x)                         	(((x) & BIT_MASK_MACID1) << BIT_SHIFT_MACID1)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_BSSID1                             (Offset 0x0708)


#define BIT_SHIFT_BSSID1                      	0
#define BIT_MASK_BSSID1                       	0xffffffffffffL
#define BIT_BSSID1(x)                         	(((x) & BIT_MASK_BSSID1) << BIT_SHIFT_BSSID1)


//2 REG_BCN_PSR_RPT1                       (Offset 0x0710)


#define BIT_SHIFT_DTIM_CNT1                   	24
#define BIT_MASK_DTIM_CNT1                    	0xff
#define BIT_DTIM_CNT1(x)                      	(((x) & BIT_MASK_DTIM_CNT1) << BIT_SHIFT_DTIM_CNT1)


#define BIT_SHIFT_DTIM_PERIOD1                	16
#define BIT_MASK_DTIM_PERIOD1                 	0xff
#define BIT_DTIM_PERIOD1(x)                   	(((x) & BIT_MASK_DTIM_PERIOD1) << BIT_SHIFT_DTIM_PERIOD1)

#define BIT_DTIM1                             	BIT(15)
#define BIT_TIM1                              	BIT(14)

#define BIT_SHIFT_PS_AID_1                    	0
#define BIT_MASK_PS_AID_1                     	0x7ff
#define BIT_PS_AID_1(x)                       	(((x) & BIT_MASK_PS_AID_1) << BIT_SHIFT_PS_AID_1)


//2 REG_BEAMFORMEE_SEL                     (Offset 0x0714)


#define BIT_SHIFT_rd_bf_sel                   	29
#define BIT_MASK_rd_bf_sel                    	0x7
#define BIT_rd_bf_sel(x)                      	(((x) & BIT_MASK_rd_bf_sel) << BIT_SHIFT_rd_bf_sel)

#define BIT_txuser_id1                        	BIT(25)

#define BIT_SHIFT_aid1                        	16
#define BIT_MASK_aid1                         	0x1ff
#define BIT_aid1(x)                           	(((x) & BIT_MASK_aid1) << BIT_SHIFT_aid1)

#define BIT_txuser_id0                        	BIT(9)

#define BIT_SHIFT_aid0                        	0
#define BIT_MASK_aid0                         	0x1ff
#define BIT_aid0(x)                           	(((x) & BIT_MASK_aid0) << BIT_SHIFT_aid0)


//2 REG_SOUNDING_PTCL_CTRL                 (Offset 0x0718)


#define BIT_SHIFT_ndp_rx_standby_timer        	24
#define BIT_MASK_ndp_rx_standby_timer         	0xff
#define BIT_ndp_rx_standby_timer(x)           	(((x) & BIT_MASK_ndp_rx_standby_timer) << BIT_SHIFT_ndp_rx_standby_timer)


#define BIT_SHIFT_CSI_RPT_OFFSET_HT           	16
#define BIT_MASK_CSI_RPT_OFFSET_HT            	0xff
#define BIT_CSI_RPT_OFFSET_HT(x)              	(((x) & BIT_MASK_CSI_RPT_OFFSET_HT) << BIT_SHIFT_CSI_RPT_OFFSET_HT)


#define BIT_SHIFT_r_WMAC_VHT_CATEGORY         	8
#define BIT_MASK_r_WMAC_VHT_CATEGORY          	0xff
#define BIT_r_WMAC_VHT_CATEGORY(x)            	(((x) & BIT_MASK_r_WMAC_VHT_CATEGORY) << BIT_SHIFT_r_WMAC_VHT_CATEGORY)

#define BIT_r_WMAC_USE_NSTS                   	BIT(7)
#define BIT_r_Disable_check_vhtsigb_crc       	BIT(6)
#define BIT_r_disable_check_vhtsiga_crc       	BIT(5)
#define BIT_r_WMAC_BFPARAM_SEL                	BIT(4)
#define BIT_r_WMAC_CSISEQ_SEL                 	BIT(3)
#define BIT_r_WMAC_CSI_WITHHTC_EN             	BIT(2)
#define BIT_r_WMAC_HT_NDPA_EN                 	BIT(1)
#define BIT_r_WMAC_VHT_NDPA_EN                	BIT(0)

//2 REG_RX_CSI_RPT                         (Offset 0x071C)

#define BIT_write_enable                      	BIT(31)
#define BIT_write_userid                      	BIT(12)

#define BIT_SHIFT_write_bw                    	10
#define BIT_MASK_write_bw                     	0x3
#define BIT_write_bw(x)                       	(((x) & BIT_MASK_write_bw) << BIT_SHIFT_write_bw)


#define BIT_SHIFT_write_cb                    	8
#define BIT_MASK_write_cb                     	0x3
#define BIT_write_cb(x)                       	(((x) & BIT_MASK_write_cb) << BIT_SHIFT_write_cb)


#define BIT_SHIFT_write_grouping              	6
#define BIT_MASK_write_grouping               	0x3
#define BIT_write_grouping(x)                 	(((x) & BIT_MASK_write_grouping) << BIT_SHIFT_write_grouping)


#define BIT_SHIFT_write_Nr                    	3
#define BIT_MASK_write_Nr                     	0x7
#define BIT_write_Nr(x)                       	(((x) & BIT_MASK_write_Nr) << BIT_SHIFT_write_Nr)


#define BIT_SHIFT_write_Nc                    	0
#define BIT_MASK_write_Nc                     	0x7
#define BIT_write_Nc(x)                       	(((x) & BIT_MASK_write_Nc) << BIT_SHIFT_write_Nc)


//2 REG_NS_ARP_CTRL                        (Offset 0x0720)

#define BIT_r_WMAC_NSARP_RSPEN                	BIT(15)
#define BIT_r_WMAC_NSARP_RARP                 	BIT(9)
#define BIT_r_WMAC_NSARP_RIPV6                	BIT(8)

#define BIT_SHIFT_r_WMAC_NSARP_MODEN          	6
#define BIT_MASK_r_WMAC_NSARP_MODEN           	0x3
#define BIT_r_WMAC_NSARP_MODEN(x)             	(((x) & BIT_MASK_r_WMAC_NSARP_MODEN) << BIT_SHIFT_r_WMAC_NSARP_MODEN)


#define BIT_SHIFT_r_WMAC_NSARP_RSPFTP         	4
#define BIT_MASK_r_WMAC_NSARP_RSPFTP          	0x3
#define BIT_r_WMAC_NSARP_RSPFTP(x)            	(((x) & BIT_MASK_r_WMAC_NSARP_RSPFTP) << BIT_SHIFT_r_WMAC_NSARP_RSPFTP)


#define BIT_SHIFT_r_WMAC_NSARP_RSPSEC         	0
#define BIT_MASK_r_WMAC_NSARP_RSPSEC          	0xf
#define BIT_r_WMAC_NSARP_RSPSEC(x)            	(((x) & BIT_MASK_r_WMAC_NSARP_RSPSEC) << BIT_SHIFT_r_WMAC_NSARP_RSPSEC)


//2 REG_NS_ARP_INFO                        (Offset 0x0724)

#define BIT_req_is_mcns                       	BIT(23)
#define BIT_req_is_ucns                       	BIT(22)
#define BIT_req_is_usns                       	BIT(21)
#define BIT_req_is_arp                        	BIT(20)
#define BIT_exprsp_mh_withQC                  	BIT(19)

#define BIT_SHIFT_exprsp_sectype              	16
#define BIT_MASK_exprsp_sectype               	0x7
#define BIT_exprsp_sectype(x)                 	(((x) & BIT_MASK_exprsp_sectype) << BIT_SHIFT_exprsp_sectype)


#define BIT_SHIFT_exprsp_chksm_7_TO_0         	8
#define BIT_MASK_exprsp_chksm_7_TO_0          	0xff
#define BIT_exprsp_chksm_7_TO_0(x)            	(((x) & BIT_MASK_exprsp_chksm_7_TO_0) << BIT_SHIFT_exprsp_chksm_7_TO_0)


#define BIT_SHIFT_exprsp_chksm_15_TO_8        	0
#define BIT_MASK_exprsp_chksm_15_TO_8         	0xff
#define BIT_exprsp_chksm_15_TO_8(x)           	(((x) & BIT_MASK_exprsp_chksm_15_TO_8) << BIT_SHIFT_exprsp_chksm_15_TO_8)


//2 REG_BEAMFORMING_INFO_NSARP             (Offset 0x072C)


#define BIT_SHIFT_Beamforming_INFO            	0
#define BIT_MASK_Beamforming_INFO             	0xffffffffL
#define BIT_Beamforming_INFO(x)               	(((x) & BIT_MASK_Beamforming_INFO) << BIT_SHIFT_Beamforming_INFO)


#define BIT_SHIFT_r_WMAC_IPV6_MYIPAD          	0
#define BIT_MASK_r_WMAC_IPV6_MYIPAD           	0xffffffffffffffffffffffffffffffffL
#define BIT_r_WMAC_IPV6_MYIPAD(x)             	(((x) & BIT_MASK_r_WMAC_IPV6_MYIPAD) << BIT_SHIFT_r_WMAC_IPV6_MYIPAD)


//2 REG_WMAC_RTX_CTX_SUBTYPE_CFG           (Offset 0x0750)


#define BIT_SHIFT_r_WMAC_CTX_SUBTYPE          	4
#define BIT_MASK_r_WMAC_CTX_SUBTYPE           	0xf
#define BIT_r_WMAC_CTX_SUBTYPE(x)             	(((x) & BIT_MASK_r_WMAC_CTX_SUBTYPE) << BIT_SHIFT_r_WMAC_CTX_SUBTYPE)


#define BIT_SHIFT_r_WMAC_RTX_SUBTYPE          	0
#define BIT_MASK_r_WMAC_RTX_SUBTYPE           	0xf
#define BIT_r_WMAC_RTX_SUBTYPE(x)             	(((x) & BIT_MASK_r_WMAC_RTX_SUBTYPE) << BIT_SHIFT_r_WMAC_RTX_SUBTYPE)


//2 REG_BT_COEX_V2                         (Offset 0x0760)

#define BIT_r_gnt_bt_rfc_sw                   	BIT(44)
#define BIT_r_gnt_bt_rfc_sw_en                	BIT(43)
#define BIT_r_gnt_bt_bb_sw                    	BIT(42)
#define BIT_r_gnt_bt_bb_sw_en                 	BIT(41)
#define BIT_r_bt_cnt_thren                    	BIT(40)

#define BIT_SHIFT_r_bt_cnt_thr                	(32 & CPU_OPT_WIDTH)
#define BIT_MASK_r_bt_cnt_thr                 	0xff
#define BIT_r_bt_cnt_thr(x)                   	(((x) & BIT_MASK_r_bt_cnt_thr) << BIT_SHIFT_r_bt_cnt_thr)

#define BIT_GNT_BT_POLARITY                   	BIT(28)
#define BIT_GNT_BT_BYPASS_PRIORITY            	BIT(24)

#define BIT_SHIFT_Timer                       	16
#define BIT_MASK_Timer                        	0xff
#define BIT_Timer(x)                          	(((x) & BIT_MASK_Timer) << BIT_SHIFT_Timer)


//2 REG_WLAN_ACT_MASK_CTRL                 (Offset 0x0768)

#define BIT_WLRX_TER_BY_CTL                   	BIT(43)
#define BIT_WLRX_TER_BY_AD                    	BIT(42)
#define BIT_ANT_DIVERSITY_SEL                 	BIT(41)
#define BIT_ANTSEL_FOR_BT_CTRL_EN             	BIT(40)
#define BIT_WLACT_LOW_GNTWL_EN                	BIT(34)
#define BIT_WLACT_HIGH_GNTBT_EN               	BIT(33)
#define BIT_NAV__UPPER                        	BIT(32)

#define BIT_SHIFT_RxMyRTS_NAV_V1              	8
#define BIT_MASK_RxMyRTS_NAV_V1               	0xff
#define BIT_RxMyRTS_NAV_V1(x)                 	(((x) & BIT_MASK_RxMyRTS_NAV_V1) << BIT_SHIFT_RxMyRTS_NAV_V1)


#define BIT_SHIFT_RTSRST_V1                   	0
#define BIT_MASK_RTSRST_V1                    	0xff
#define BIT_RTSRST_V1(x)                      	(((x) & BIT_MASK_RTSRST_V1) << BIT_SHIFT_RTSRST_V1)


//2 REG_BT_COEX_ENHANCED_INTR_CTRL         (Offset 0x076E)


#define BIT_SHIFT_BT_STAT_DELAY               	12
#define BIT_MASK_BT_STAT_DELAY                	0xf
#define BIT_BT_STAT_DELAY(x)                  	(((x) & BIT_MASK_BT_STAT_DELAY) << BIT_SHIFT_BT_STAT_DELAY)


#define BIT_SHIFT_BT_TRX_INIT_DETECT          	8
#define BIT_MASK_BT_TRX_INIT_DETECT           	0xf
#define BIT_BT_TRX_INIT_DETECT(x)             	(((x) & BIT_MASK_BT_TRX_INIT_DETECT) << BIT_SHIFT_BT_TRX_INIT_DETECT)


#define BIT_SHIFT_BT_PRI_DETECT_TO            	4
#define BIT_MASK_BT_PRI_DETECT_TO             	0xf
#define BIT_BT_PRI_DETECT_TO(x)               	(((x) & BIT_MASK_BT_PRI_DETECT_TO) << BIT_SHIFT_BT_PRI_DETECT_TO)

#define BIT_r_GRANTALL_WLMASK                 	BIT(3)
#define BIT_STATIS_BT_EN                      	BIT(2)
#define BIT_WL_ACT_MASK_Enable                	BIT(1)
#define BIT_Enhanced_BT                       	BIT(0)

//2 REG_BT_ACT_STATISTICS                  (Offset 0x0770)


#define BIT_SHIFT_STATIS_BT_LO_RX             	(48 & CPU_OPT_WIDTH)
#define BIT_MASK_STATIS_BT_LO_RX              	0xffff
#define BIT_STATIS_BT_LO_RX(x)                	(((x) & BIT_MASK_STATIS_BT_LO_RX) << BIT_SHIFT_STATIS_BT_LO_RX)


#define BIT_SHIFT_STATIS_BT_LO_TX             	(32 & CPU_OPT_WIDTH)
#define BIT_MASK_STATIS_BT_LO_TX              	0xffff
#define BIT_STATIS_BT_LO_TX(x)                	(((x) & BIT_MASK_STATIS_BT_LO_TX) << BIT_SHIFT_STATIS_BT_LO_TX)


#define BIT_SHIFT_STATIS_BT_HI_RX             	16
#define BIT_MASK_STATIS_BT_HI_RX              	0xffff
#define BIT_STATIS_BT_HI_RX(x)                	(((x) & BIT_MASK_STATIS_BT_HI_RX) << BIT_SHIFT_STATIS_BT_HI_RX)


#define BIT_SHIFT_STATIS_BT_HI_TX             	0
#define BIT_MASK_STATIS_BT_HI_TX              	0xffff
#define BIT_STATIS_BT_HI_TX(x)                	(((x) & BIT_MASK_STATIS_BT_HI_TX) << BIT_SHIFT_STATIS_BT_HI_TX)


//2 REG_BT_Statistics_Control_Register     (Offset 0x0778)


#define BIT_SHIFT_r_BT_CMD_RPT                	16
#define BIT_MASK_r_BT_CMD_RPT                 	0xffff
#define BIT_r_BT_CMD_RPT(x)                   	(((x) & BIT_MASK_r_BT_CMD_RPT) << BIT_SHIFT_r_BT_CMD_RPT)


#define BIT_SHIFT_r_RPT_FROM_BT               	8
#define BIT_MASK_r_RPT_FROM_BT                	0xff
#define BIT_r_RPT_FROM_BT(x)                  	(((x) & BIT_MASK_r_RPT_FROM_BT) << BIT_SHIFT_r_RPT_FROM_BT)


#define BIT_SHIFT_BT_HID_ISR_SET              	6
#define BIT_MASK_BT_HID_ISR_SET               	0x3
#define BIT_BT_HID_ISR_SET(x)                 	(((x) & BIT_MASK_BT_HID_ISR_SET) << BIT_SHIFT_BT_HID_ISR_SET)

#define BIT_TDMA_BT_START_NOTIFY              	BIT(5)
#define BIT_Enable_TDMA_FW_Mode               	BIT(4)
#define BIT_Enable_PTA_TDMA_Mode              	BIT(3)
#define BIT_Enable_Coexist_Tab_In_TDMA        	BIT(2)
#define BIT_GPIO2_GPIO3_EXANGE_or_NO_BT_CCA   	BIT(1)
#define BIT_RTK_BT_Enable                     	BIT(0)

//2 REG_BT_Status_Report_Register          (Offset 0x077C)


#define BIT_SHIFT_BT_PROFILE                  	24
#define BIT_MASK_BT_PROFILE                   	0xff
#define BIT_BT_PROFILE(x)                     	(((x) & BIT_MASK_BT_PROFILE) << BIT_SHIFT_BT_PROFILE)


#define BIT_SHIFT_BT_POWER                    	16
#define BIT_MASK_BT_POWER                     	0xff
#define BIT_BT_POWER(x)                       	(((x) & BIT_MASK_BT_POWER) << BIT_SHIFT_BT_POWER)


#define BIT_SHIFT_BT_PREDECT_STATUS           	8
#define BIT_MASK_BT_PREDECT_STATUS            	0xff
#define BIT_BT_PREDECT_STATUS(x)              	(((x) & BIT_MASK_BT_PREDECT_STATUS) << BIT_SHIFT_BT_PREDECT_STATUS)


#define BIT_SHIFT_BT_CMD_INFO                 	0
#define BIT_MASK_BT_CMD_INFO                  	0xff
#define BIT_BT_CMD_INFO(x)                    	(((x) & BIT_MASK_BT_CMD_INFO) << BIT_SHIFT_BT_CMD_INFO)


//2 REG_BT_Interrupt_Control_Register      (Offset 0x0780)

#define BIT_EN_MAC_NULL_PKT_NOTIFY            	BIT(31)
#define BIT_EN_WLAN_RPT_AND_BT_QUERY          	BIT(30)
#define BIT_EN_BT_STSTUS_RPT                  	BIT(29)
#define BIT_EN_BT_POWER                       	BIT(28)
#define BIT_EN_BT_CHANNEL                     	BIT(27)
#define BIT_EN_BT_SLOT_CHANGE                 	BIT(26)
#define BIT_EN_BT_PROFILE_OR_HID              	BIT(25)
#define BIT_WLAN_RPT_NOTIFY                   	BIT(24)

#define BIT_SHIFT_WLAN_RPT_DATA               	16
#define BIT_MASK_WLAN_RPT_DATA                	0xff
#define BIT_WLAN_RPT_DATA(x)                  	(((x) & BIT_MASK_WLAN_RPT_DATA) << BIT_SHIFT_WLAN_RPT_DATA)


#define BIT_SHIFT_CMD_ID                      	8
#define BIT_MASK_CMD_ID                       	0xff
#define BIT_CMD_ID(x)                         	(((x) & BIT_MASK_CMD_ID) << BIT_SHIFT_CMD_ID)


#define BIT_SHIFT_BT_DATA                     	0
#define BIT_MASK_BT_DATA                      	0xff
#define BIT_BT_DATA(x)                        	(((x) & BIT_MASK_BT_DATA) << BIT_SHIFT_BT_DATA)


//2 REG_WLAN_Report_Time_Out_Control_Register (Offset 0x0784)


#define BIT_SHIFT_WLAN_RPT_TO                 	0
#define BIT_MASK_WLAN_RPT_TO                  	0xff
#define BIT_WLAN_RPT_TO(x)                    	(((x) & BIT_MASK_WLAN_RPT_TO) << BIT_SHIFT_WLAN_RPT_TO)


//2 REG_BT_Isolation_Table_Register_Register (Offset 0x0785)


#define BIT_SHIFT_ISOLATION_CHK               	1
#define BIT_MASK_ISOLATION_CHK                	0x7fffffffffffffffffffL
#define BIT_ISOLATION_CHK(x)                  	(((x) & BIT_MASK_ISOLATION_CHK) << BIT_SHIFT_ISOLATION_CHK)

#define BIT_ISOLATION_EN                      	BIT(0)

//2 REG_BT_Interrupt_Status_Register       (Offset 0x078F)

#define BIT_BT_HID_ISR                        	BIT(7)
#define BIT_BT_QUERY_ISR                      	BIT(6)
#define BIT_MAC_NULL_PKT_NOTIFY_ISR           	BIT(5)
#define BIT_WLAN_RPT_ISR                      	BIT(4)
#define BIT_BT_POWER_ISR                      	BIT(3)
#define BIT_BT_CHANNEL_ISR                    	BIT(2)
#define BIT_BT_SLOT_CHANGE_ISR                	BIT(1)
#define BIT_BT_PROFILE_ISR                    	BIT(0)

//2 REG_BT_TDMA_Time_Register              (Offset 0x0790)


#define BIT_SHIFT_BT_TIME                     	6
#define BIT_MASK_BT_TIME                      	0x3ffffff
#define BIT_BT_TIME(x)                        	(((x) & BIT_MASK_BT_TIME) << BIT_SHIFT_BT_TIME)


#define BIT_SHIFT_BT_RPT_SAMPLE_RATE          	0
#define BIT_MASK_BT_RPT_SAMPLE_RATE           	0x3f
#define BIT_BT_RPT_SAMPLE_RATE(x)             	(((x) & BIT_MASK_BT_RPT_SAMPLE_RATE) << BIT_SHIFT_BT_RPT_SAMPLE_RATE)


//2 REG_BT_ACT_Register                    (Offset 0x0794)


#define BIT_SHIFT_bt_eisr_en                  	16
#define BIT_MASK_bt_eisr_en                   	0xff
#define BIT_bt_eisr_en(x)                     	(((x) & BIT_MASK_bt_eisr_en) << BIT_SHIFT_bt_eisr_en)

#define BIT_BT_ACT_FALLING_ISR                	BIT(10)
#define BIT_BT_ACT_RISING_ISR                 	BIT(9)
#define BIT_TDMA_TO_ISR                       	BIT(8)

#define BIT_SHIFT_BT_CH                       	0
#define BIT_MASK_BT_CH                        	0xff
#define BIT_BT_CH(x)                          	(((x) & BIT_MASK_BT_CH) << BIT_SHIFT_BT_CH)


//2 REG_OBFF_CTRL_basic                    (Offset 0x0798)

#define BIT_OBFF_EN_V1                        	BIT(31)

#define BIT_SHIFT_OBFF_STATE_V1               	28
#define BIT_MASK_OBFF_STATE_V1                	0x3
#define BIT_OBFF_STATE_V1(x)                  	(((x) & BIT_MASK_OBFF_STATE_V1) << BIT_SHIFT_OBFF_STATE_V1)

#define BIT_OBFF_ACT_RXDMA_EN                 	BIT(27)
#define BIT_OBFF_BLOCK_INT_EN                 	BIT(26)
#define BIT_OBFF_AUTOACT_EN                   	BIT(25)
#define BIT_OBFF_AUTOIDLE_EN                  	BIT(24)

#define BIT_SHIFT_WAKE_MAX_PLS                	20
#define BIT_MASK_WAKE_MAX_PLS                 	0x7
#define BIT_WAKE_MAX_PLS(x)                   	(((x) & BIT_MASK_WAKE_MAX_PLS) << BIT_SHIFT_WAKE_MAX_PLS)


#define BIT_SHIFT_WAKE_MIN_PLS                	16
#define BIT_MASK_WAKE_MIN_PLS                 	0x7
#define BIT_WAKE_MIN_PLS(x)                   	(((x) & BIT_MASK_WAKE_MIN_PLS) << BIT_SHIFT_WAKE_MIN_PLS)


#define BIT_SHIFT_WAKE_MAX_F2F                	12
#define BIT_MASK_WAKE_MAX_F2F                 	0x7
#define BIT_WAKE_MAX_F2F(x)                   	(((x) & BIT_MASK_WAKE_MAX_F2F) << BIT_SHIFT_WAKE_MAX_F2F)


#define BIT_SHIFT_WAKE_MIN_F2F                	8
#define BIT_MASK_WAKE_MIN_F2F                 	0x7
#define BIT_WAKE_MIN_F2F(x)                   	(((x) & BIT_MASK_WAKE_MIN_F2F) << BIT_SHIFT_WAKE_MIN_F2F)

#define BIT_APP_CPU_ACT_V1                    	BIT(3)
#define BIT_APP_OBFF_V1                       	BIT(2)
#define BIT_APP_IDLE_V1                       	BIT(1)
#define BIT_APP_INIT_V1                       	BIT(0)

//2 REG_OBFF_CTRL2_timer                   (Offset 0x079C)


#define BIT_SHIFT_RX_HIGH_TIMER_IDX           	24
#define BIT_MASK_RX_HIGH_TIMER_IDX            	0x7
#define BIT_RX_HIGH_TIMER_IDX(x)              	(((x) & BIT_MASK_RX_HIGH_TIMER_IDX) << BIT_SHIFT_RX_HIGH_TIMER_IDX)


#define BIT_SHIFT_RX_MED_TIMER_IDX            	16
#define BIT_MASK_RX_MED_TIMER_IDX             	0x7
#define BIT_RX_MED_TIMER_IDX(x)               	(((x) & BIT_MASK_RX_MED_TIMER_IDX) << BIT_SHIFT_RX_MED_TIMER_IDX)


#define BIT_SHIFT_RX_LOW_TIMER_IDX            	8
#define BIT_MASK_RX_LOW_TIMER_IDX             	0x7
#define BIT_RX_LOW_TIMER_IDX(x)               	(((x) & BIT_MASK_RX_LOW_TIMER_IDX) << BIT_SHIFT_RX_LOW_TIMER_IDX)


#define BIT_SHIFT_OBFF_INT_TIMER_IDX          	0
#define BIT_MASK_OBFF_INT_TIMER_IDX           	0x7
#define BIT_OBFF_INT_TIMER_IDX(x)             	(((x) & BIT_MASK_OBFF_INT_TIMER_IDX) << BIT_SHIFT_OBFF_INT_TIMER_IDX)


//2 REG_LTR_CTRL_basic                     (Offset 0x07A0)

#define BIT_LTR_EN_V1                         	BIT(31)
#define BIT_LTR_HW_EN_V1                      	BIT(30)
#define BIT_LRT_ACT_CTS_EN                    	BIT(29)
#define BIT_LTR_ACT_RXPKT_EN                  	BIT(28)
#define BIT_LTR_ACT_RXDMA_EN                  	BIT(27)
#define BIT_LTR_IDLE_NO_SNOOP                 	BIT(26)
#define BIT_SPDUP_MGTPKT                      	BIT(25)
#define BIT_RX_AGG_EN                         	BIT(24)
#define BIT_APP_LTR_ACT                       	BIT(23)
#define BIT_APP_LTR_IDLE                      	BIT(22)

#define BIT_SHIFT_HIGH_RATE_TRIG_SEL          	20
#define BIT_MASK_HIGH_RATE_TRIG_SEL           	0x3
#define BIT_HIGH_RATE_TRIG_SEL(x)             	(((x) & BIT_MASK_HIGH_RATE_TRIG_SEL) << BIT_SHIFT_HIGH_RATE_TRIG_SEL)


#define BIT_SHIFT_MED_RATE_TRIG_SEL           	18
#define BIT_MASK_MED_RATE_TRIG_SEL            	0x3
#define BIT_MED_RATE_TRIG_SEL(x)              	(((x) & BIT_MASK_MED_RATE_TRIG_SEL) << BIT_SHIFT_MED_RATE_TRIG_SEL)


#define BIT_SHIFT_LOW_RATE_TRIG_SEL           	16
#define BIT_MASK_LOW_RATE_TRIG_SEL            	0x3
#define BIT_LOW_RATE_TRIG_SEL(x)              	(((x) & BIT_MASK_LOW_RATE_TRIG_SEL) << BIT_SHIFT_LOW_RATE_TRIG_SEL)


#define BIT_SHIFT_HIGH_RATE_BD_IDX            	8
#define BIT_MASK_HIGH_RATE_BD_IDX             	0x7f
#define BIT_HIGH_RATE_BD_IDX(x)               	(((x) & BIT_MASK_HIGH_RATE_BD_IDX) << BIT_SHIFT_HIGH_RATE_BD_IDX)


#define BIT_SHIFT_LOW_RATE_BD_IDX             	0
#define BIT_MASK_LOW_RATE_BD_IDX              	0x7f
#define BIT_LOW_RATE_BD_IDX(x)                	(((x) & BIT_MASK_LOW_RATE_BD_IDX) << BIT_SHIFT_LOW_RATE_BD_IDX)


//2 REG_LTR_CTRL2_timer_threshold          (Offset 0x07A4)


#define BIT_SHIFT_RX_EMPTY_TIMER_IDX          	24
#define BIT_MASK_RX_EMPTY_TIMER_IDX           	0x7
#define BIT_RX_EMPTY_TIMER_IDX(x)             	(((x) & BIT_MASK_RX_EMPTY_TIMER_IDX) << BIT_SHIFT_RX_EMPTY_TIMER_IDX)


#define BIT_SHIFT_RX_AFULL_TH_IDX             	20
#define BIT_MASK_RX_AFULL_TH_IDX              	0x7
#define BIT_RX_AFULL_TH_IDX(x)                	(((x) & BIT_MASK_RX_AFULL_TH_IDX) << BIT_SHIFT_RX_AFULL_TH_IDX)


#define BIT_SHIFT_RX_HIGH_TH_IDX              	16
#define BIT_MASK_RX_HIGH_TH_IDX               	0x7
#define BIT_RX_HIGH_TH_IDX(x)                 	(((x) & BIT_MASK_RX_HIGH_TH_IDX) << BIT_SHIFT_RX_HIGH_TH_IDX)


#define BIT_SHIFT_RX_MED_TH_IDX               	12
#define BIT_MASK_RX_MED_TH_IDX                	0x7
#define BIT_RX_MED_TH_IDX(x)                  	(((x) & BIT_MASK_RX_MED_TH_IDX) << BIT_SHIFT_RX_MED_TH_IDX)


#define BIT_SHIFT_RX_LOW_TH_IDX               	8
#define BIT_MASK_RX_LOW_TH_IDX                	0x7
#define BIT_RX_LOW_TH_IDX(x)                  	(((x) & BIT_MASK_RX_LOW_TH_IDX) << BIT_SHIFT_RX_LOW_TH_IDX)


#define BIT_SHIFT_LTR_SPACE_IDX               	4
#define BIT_MASK_LTR_SPACE_IDX                	0x3
#define BIT_LTR_SPACE_IDX(x)                  	(((x) & BIT_MASK_LTR_SPACE_IDX) << BIT_SHIFT_LTR_SPACE_IDX)


#define BIT_SHIFT_LTR_IDLE_TIMER_IDX          	0
#define BIT_MASK_LTR_IDLE_TIMER_IDX           	0x7
#define BIT_LTR_IDLE_TIMER_IDX(x)             	(((x) & BIT_MASK_LTR_IDLE_TIMER_IDX) << BIT_SHIFT_LTR_IDLE_TIMER_IDX)


//2 REG_LTR_IDLE_latency                   (Offset 0x07A8)


#define BIT_SHIFT_LTR_IDLE_L                  	0
#define BIT_MASK_LTR_IDLE_L                   	0xffffffffL
#define BIT_LTR_IDLE_L(x)                     	(((x) & BIT_MASK_LTR_IDLE_L) << BIT_SHIFT_LTR_IDLE_L)


//2 REG_LTR_ACTIVE_latency                 (Offset 0x07AC)


#define BIT_SHIFT_LTR_ACT__L                  	0
#define BIT_MASK_LTR_ACT__L                   	0xffffffffL
#define BIT_LTR_ACT__L(x)                     	(((x) & BIT_MASK_LTR_ACT__L) << BIT_SHIFT_LTR_ACT__L)


//2 REG_Antenna_Training_Control_Register  (Offset 0x07B0)

#define BIT_APPEND_MACID_IN_RESP_EN           	BIT(50)
#define BIT_ADDR2_MATCH_EN                    	BIT(49)
#define BIT_ANTTRN_EN                         	BIT(48)

#define BIT_SHIFT_TRAIN_STA_ADDR              	0
#define BIT_MASK_TRAIN_STA_ADDR               	0xffffffffffffL
#define BIT_TRAIN_STA_ADDR(x)                 	(((x) & BIT_MASK_TRAIN_STA_ADDR) << BIT_SHIFT_TRAIN_STA_ADDR)


#endif


#if (RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8881A_SUPPORT)


//2 REG_WMAC_PKTCNT_RWD                    (Offset 0x07B8)


#define BIT_SHIFT_PKTCNT_BSSIDMAP             	4
#define BIT_MASK_PKTCNT_BSSIDMAP              	0xf
#define BIT_PKTCNT_BSSIDMAP(x)                	(((x) & BIT_MASK_PKTCNT_BSSIDMAP) << BIT_SHIFT_PKTCNT_BSSIDMAP)

#define BIT_PKTCNT_CNTRST                     	BIT(1)
#define BIT_PKTCNT_CNTEN                      	BIT(0)

//2 REG_WMAC_PKTCNT_CTRL                   (Offset 0x07BC)

#define BIT_WMAC_PKTCNT_TRST                  	BIT(9)
#define BIT_WMAC_PKTCNT_FEN                   	BIT(8)

#define BIT_SHIFT_WMAC_PKTCNT_CFGAD           	0
#define BIT_MASK_WMAC_PKTCNT_CFGAD            	0xff
#define BIT_WMAC_PKTCNT_CFGAD(x)              	(((x) & BIT_MASK_WMAC_PKTCNT_CFGAD) << BIT_SHIFT_WMAC_PKTCNT_CFGAD)


#endif


#if (RTL8814A_SUPPORT)


//2 REG_IQ_DUMP                            (Offset 0x07C0)


#define BIT_SHIFT_r_WMAC_MATCH_REF_MAC        	(64 & CPU_OPT_WIDTH)
#define BIT_MASK_r_WMAC_MATCH_REF_MAC         	0xffffffffL
#define BIT_r_WMAC_MATCH_REF_MAC(x)           	(((x) & BIT_MASK_r_WMAC_MATCH_REF_MAC) << BIT_SHIFT_r_WMAC_MATCH_REF_MAC)


#define BIT_SHIFT_r_WMAC_MASK_LA_MAC          	(32 & CPU_OPT_WIDTH)
#define BIT_MASK_r_WMAC_MASK_LA_MAC           	0xffffffffL
#define BIT_r_WMAC_MASK_LA_MAC(x)             	(((x) & BIT_MASK_r_WMAC_MASK_LA_MAC) << BIT_SHIFT_r_WMAC_MASK_LA_MAC)


#define BIT_SHIFT_DUMP_OK_ADDR                	15
#define BIT_MASK_DUMP_OK_ADDR                 	0x1ffff
#define BIT_DUMP_OK_ADDR(x)                   	(((x) & BIT_MASK_DUMP_OK_ADDR) << BIT_SHIFT_DUMP_OK_ADDR)


#define BIT_SHIFT_r_TRIG_TIME_SEL             	8
#define BIT_MASK_r_TRIG_TIME_SEL              	0x7f
#define BIT_r_TRIG_TIME_SEL(x)                	(((x) & BIT_MASK_r_TRIG_TIME_SEL) << BIT_SHIFT_r_TRIG_TIME_SEL)


#define BIT_SHIFT_r_MAC_TRIG_SEL              	6
#define BIT_MASK_r_MAC_TRIG_SEL               	0x3
#define BIT_r_MAC_TRIG_SEL(x)                 	(((x) & BIT_MASK_r_MAC_TRIG_SEL) << BIT_SHIFT_r_MAC_TRIG_SEL)

#define BIT_MAC_TRIG_REG                      	BIT(5)

#define BIT_SHIFT_r_LEVEL_PULSE_SEL           	3
#define BIT_MASK_r_LEVEL_PULSE_SEL            	0x3
#define BIT_r_LEVEL_PULSE_SEL(x)              	(((x) & BIT_MASK_r_LEVEL_PULSE_SEL) << BIT_SHIFT_r_LEVEL_PULSE_SEL)

#define BIT_en_LA_MAC                         	BIT(2)
#define BIT_r_EN_IQDUMP                       	BIT(1)
#define BIT_r_IQDATA_DUMP                     	BIT(0)

//2 REG_OFDM_CCK_LEN_MASK                  (Offset 0x07D0)


#define BIT_SHIFT_r_WMAC_RX_FIL_LEN           	(64 & CPU_OPT_WIDTH)
#define BIT_MASK_r_WMAC_RX_FIL_LEN            	0xffff
#define BIT_r_WMAC_RX_FIL_LEN(x)              	(((x) & BIT_MASK_r_WMAC_RX_FIL_LEN) << BIT_SHIFT_r_WMAC_RX_FIL_LEN)


#define BIT_SHIFT_r_WMAC_RXFIFO_FULL_TH       	(56 & CPU_OPT_WIDTH)
#define BIT_MASK_r_WMAC_RXFIFO_FULL_TH        	0xff
#define BIT_r_WMAC_RXFIFO_FULL_TH(x)          	(((x) & BIT_MASK_r_WMAC_RXFIFO_FULL_TH) << BIT_SHIFT_r_WMAC_RXFIFO_FULL_TH)

#define BIT_r_WMAC_SRCH_TXRPT_TYPE            	BIT(51)
#define BIT_r_WMAC_NDP_RST                    	BIT(50)
#define BIT_r_WMAC_POWINT_EN                  	BIT(49)
#define BIT_r_WMAC_SRCH_TXRPT_PERPKT          	BIT(48)
#define BIT_r_WMAC_SRCH_TXRPT_MID             	BIT(47)
#define BIT_r_WMAC_PFIN_TOEN                  	BIT(46)
#define BIT_r_WMAC_FIL_SECERR                 	BIT(45)
#define BIT_r_WMAC_FIL_CTLPKTLEN              	BIT(44)
#define BIT_r_WMAC_FIL_FCTYPE                 	BIT(43)
#define BIT_r_WMAC_FIL_FCPROVER               	BIT(42)
#define BIT_r_WMAC_PHYSTS_SNIF                	BIT(41)
#define BIT_r_WMAC_PHYSTS_PLCP                	BIT(40)
#define BIT_r_MAC_TCR_VBONF_RD                	BIT(39)
#define BIT_r_WMAC_TCR_MPAR_NDP               	BIT(38)
#define BIT_r_WMAC_NDP_FILTER                 	BIT(37)
#define BIT_r_WMAC_RXLEN_SEL                  	BIT(36)
#define BIT_r_WMAC_RXLEN_SEL1                 	BIT(35)
#define BIT_r_OFDM_FILTER                     	BIT(34)
#define BIT_r_WMAC_CHK_OFDM_LEN               	BIT(33)
#define BIT_r_WMAC_CHK_CCK_LEN                	BIT(32)

#define BIT_SHIFT_r_OFDM_LEN                  	26
#define BIT_MASK_r_OFDM_LEN                   	0x3f
#define BIT_r_OFDM_LEN(x)                     	(((x) & BIT_MASK_r_OFDM_LEN) << BIT_SHIFT_r_OFDM_LEN)


#define BIT_SHIFT_r_CCK_LEN                   	0
#define BIT_MASK_r_CCK_LEN                    	0xffff
#define BIT_r_CCK_LEN(x)                      	(((x) & BIT_MASK_r_CCK_LEN) << BIT_SHIFT_r_CCK_LEN)


//2 REG_NDP_SIG                            (Offset 0x07E0)


#define BIT_SHIFT_r_WMAC_TXNDP_SIGB           	0
#define BIT_MASK_r_WMAC_TXNDP_SIGB            	0x1fffff
#define BIT_r_WMAC_TXNDP_SIGB(x)              	(((x) & BIT_MASK_r_WMAC_TXNDP_SIGB) << BIT_SHIFT_r_WMAC_TXNDP_SIGB)


//2 REG_TXCMD_INFO_FOR_RSP_PKT             (Offset 0x07E4)


#define BIT_SHIFT_r_MAC_DEBUG                 	(32 & CPU_OPT_WIDTH)
#define BIT_MASK_r_MAC_DEBUG                  	0xffffffffL
#define BIT_r_MAC_DEBUG(x)                    	(((x) & BIT_MASK_r_MAC_DEBUG) << BIT_SHIFT_r_MAC_DEBUG)


#define BIT_SHIFT_r_MAC_DBG_SHIFT             	8
#define BIT_MASK_r_MAC_DBG_SHIFT              	0x7
#define BIT_r_MAC_DBG_SHIFT(x)                	(((x) & BIT_MASK_r_MAC_DBG_SHIFT) << BIT_SHIFT_r_MAC_DBG_SHIFT)


#define BIT_SHIFT_r_MAC_DBG_SEL               	0
#define BIT_MASK_r_MAC_DBG_SEL                	0x3
#define BIT_r_MAC_DBG_SEL(x)                  	(((x) & BIT_MASK_r_MAC_DBG_SEL) << BIT_SHIFT_r_MAC_DBG_SEL)

//2 REG_CPU_DMEM_CON                       (Offset 0x1080)


#define BIT_SHIFT_CPU_DMEM_CON                	0
#define BIT_MASK_CPU_DMEM_CON                 	0xff
#define BIT_CPU_DMEM_CON(x)                   	(((x) & BIT_MASK_CPU_DMEM_CON) << BIT_SHIFT_CPU_DMEM_CON)


//2 REG_HIMR2                              (Offset 0x10B0)

#define BIT_BCNDMAINT_P4_MSK                  	BIT(31)
#define BIT_BCNDMAINT_P3_MSK                  	BIT(30)
#define BIT_BCNDMAINT_P2_MSK                  	BIT(29)
#define BIT_BCNDMAINT_P1_MSK                  	BIT(28)
#define BIT_ATIMend7__MSK                     	BIT(22)
#define BIT_ATIMend6__MSK                     	BIT(21)
#define BIT_ATIMend5__MSK                     	BIT(20)
#define BIT_ATIMend4__MSK                     	BIT(19)
#define BIT_ATIMend3__MSK                     	BIT(18)
#define BIT_ATIMend2__MSK                     	BIT(17)
#define BIT_ATIMend1__MSK                     	BIT(16)
#define BIT_TXBCN7OK_MSK                      	BIT(14)
#define BIT_TXBCN6OK_MSK                      	BIT(13)
#define BIT_TXBCN5OK_MSK                      	BIT(12)
#define BIT_TXBCN4OK_MSK                      	BIT(11)
#define BIT_TXBCN3OK_MSK                      	BIT(10)
#define BIT_TXBCN2OK_MSK                      	BIT(9)
#define BIT_TXBCN1OK_MSK                      	BIT(8)
#define BIT_TXBCN7ERR_MSK                     	BIT(6)
#define BIT_TXBCN6ERR_MSK                     	BIT(5)
#define BIT_TXBCN5ERR_MSK                     	BIT(4)
#define BIT_TXBCN4ERR_MSK                     	BIT(3)
#define BIT_TXBCN3ERR_MSK                     	BIT(2)
#define BIT_TXBCN2ERR_MSK                     	BIT(1)
#define BIT_TXBCN1ERR_MSK                     	BIT(0)

//2 REG_HISR2                              (Offset 0x10B4)

#define BIT_BCNDMAINT_P4                      	BIT(31)
#define BIT_BCNDMAINT_P3                      	BIT(30)
#define BIT_BCNDMAINT_P2                      	BIT(29)
#define BIT_BCNDMAINT_P1                      	BIT(28)
#define BIT_ATIMend7                          	BIT(22)
#define BIT_ATIMend6                          	BIT(21)
#define BIT_ATIMend5                          	BIT(20)
#define BIT_ATIMend4                          	BIT(19)
#define BIT_ATIMend3                          	BIT(18)
#define BIT_ATIMend2                          	BIT(17)
#define BIT_ATIMend1                          	BIT(16)
#define BIT_TXBCN7OK                          	BIT(14)
#define BIT_TXBCN6OK                          	BIT(13)
#define BIT_TXBCN5OK                          	BIT(12)
#define BIT_TXBCN4OK                          	BIT(11)
#define BIT_TXBCN3OK                          	BIT(10)
#define BIT_TXBCN2OK                          	BIT(9)
#define BIT_TXBCN1OK                          	BIT(8)
#define BIT_TXBCN7ERR                         	BIT(6)
#define BIT_TXBCN6ERR                         	BIT(5)
#define BIT_TXBCN5ERR                         	BIT(4)
#define BIT_TXBCN4ERR                         	BIT(3)
#define BIT_TXBCN3ERR                         	BIT(2)
#define BIT_TXBCN2ERR                         	BIT(1)
#define BIT_TXBCN1ERR                         	BIT(0)

//2 REG_HIMR3                              (Offset 0x10B8)

#define BIT_TXSHORTCUT_TXDESUPDATEOK_mask     	BIT(13)
#define BIT_TXSHORTCUT__BKUPDATEOK_mask       	BIT(12)
#define BIT_TXSHORTCUT__BEUPDATEOK_mask       	BIT(11)
#define BIT_TXSHORTCUT__VIUPDATEOK_mas        	BIT(10)
#define BIT_TXSHORTCUT__VOUPDATEOK_mask       	BIT(9)
#define BIT_PWR_INT_127_mask                  	BIT(8)
#define BIT_PWR_INT_126to96_mask              	BIT(7)
#define BIT_PWR_INT_95to64_mask               	BIT(6)
#define BIT_PWR_INT_63to32_mask               	BIT(5)
#define BIT_PWR_INT_31to0_mask                	BIT(4)
#define BIT_DDMA0_LP_INT_MSK                  	BIT(1)
#define BIT_DDMA0_HP_INT_MSK                  	BIT(0)

//2 REG_HISR3                              (Offset 0x10BC)

#define BIT_TXSHORTCUT_TXDESUPDATEOK          	BIT(13)
#define BIT_TXSHORTCUT__BKUPDATEOK            	BIT(12)
#define BIT_TXSHORTCUT__BEUPDATEOK            	BIT(11)
#define BIT_TXSHORTCUT__VIUPDATEOK            	BIT(10)
#define BIT_TXSHORTCUT__VOUPDATEOK            	BIT(9)
#define BIT_PWR_INT_127                       	BIT(8)
#define BIT_PWR_INT_126to96                   	BIT(7)
#define BIT_PWR_INT_95to64                    	BIT(6)
#define BIT_PWR_INT_63to32                    	BIT(5)
#define BIT_PWR_INT_31to0                     	BIT(4)
#define BIT_DDMA0_LP_INT                      	BIT(1)
#define BIT_DDMA0_HP_INT                      	BIT(0)

//2 REG_SW_MDIO                            (Offset 0x10C0)


#define BIT_SHIFT_FW_DBG1_V1                  	5
#define BIT_MASK_FW_DBG1_V1                   	0x7ffffff
#define BIT_FW_DBG1_V1(x)                     	(((x) & BIT_MASK_FW_DBG1_V1) << BIT_SHIFT_FW_DBG1_V1)

#define BIT_SW_USB3_MD_SEL                    	BIT(4)
#define BIT_SW_PCIE_MD_SEL                    	BIT(3)
#define BIT_SW_MDCK                           	BIT(2)
#define BIT_SW_MDI                            	BIT(1)
#define BIT_MDO                               	BIT(0)

//2 REG_SW_FLUSH                           (Offset 0x10C4)

#define BIT_FLUSH_HOLDN_EN                    	BIT(25)
#define BIT_FLUSH_WR_EN                       	BIT(24)
#define BIT_SW_FLASH_CONTROL                  	BIT(23)
#define BIT_SW_FLASH_WEN_E                    	BIT(19)
#define BIT_SW_FLASH_HOLDN_E                  	BIT(18)
#define BIT_SW_FLASH_SO_E                     	BIT(17)
#define BIT_SW_FLASH_SI_E                     	BIT(16)
#define BIT_SW_FLASH_SK_O                     	BIT(13)
#define BIT_SW_FLASH_CEN_O                    	BIT(12)
#define BIT_SW_FLASH_WEN_O                    	BIT(11)
#define BIT_SW_FLASH_HOLDN_O                  	BIT(10)
#define BIT_SW_FLASH_SO_O                     	BIT(9)
#define BIT_SW_FLASH_SI_O                     	BIT(8)
#define BIT_SW_FLASH_WEN_I                    	BIT(3)
#define BIT_SW_FLASH_HOLDN_I                  	BIT(2)
#define BIT_SW_FLASH_SO_I                     	BIT(1)
#define BIT_SW_FLASH_SI_I                     	BIT(0)

//2 REG_FW_DBG0                            (Offset 0x10E0)


#define BIT_SHIFT_FW_DBG0                     	0
#define BIT_MASK_FW_DBG0                      	0xffffffffL
#define BIT_FW_DBG0(x)                        	(((x) & BIT_MASK_FW_DBG0) << BIT_SHIFT_FW_DBG0)


//2 REG_FW_DBG1                            (Offset 0x10E4)


#define BIT_SHIFT_FW_DBG1                     	0
#define BIT_MASK_FW_DBG1                      	0xffffffffL
#define BIT_FW_DBG1(x)                        	(((x) & BIT_MASK_FW_DBG1) << BIT_SHIFT_FW_DBG1)


//2 REG_FW_DBG2                            (Offset 0x10E8)


#define BIT_SHIFT_FW_DBG2                     	0
#define BIT_MASK_FW_DBG2                      	0xffffffffL
#define BIT_FW_DBG2(x)                        	(((x) & BIT_MASK_FW_DBG2) << BIT_SHIFT_FW_DBG2)


//2 REG_FW_DBG3                            (Offset 0x10EC)


#define BIT_SHIFT_FW_DBG3                     	0
#define BIT_MASK_FW_DBG3                      	0xffffffffL
#define BIT_FW_DBG3(x)                        	(((x) & BIT_MASK_FW_DBG3) << BIT_SHIFT_FW_DBG3)


//2 REG_FW_DBG4                            (Offset 0x10F0)


#define BIT_SHIFT_FW_DBG4                     	0
#define BIT_MASK_FW_DBG4                      	0xffffffffL
#define BIT_FW_DBG4(x)                        	(((x) & BIT_MASK_FW_DBG4) << BIT_SHIFT_FW_DBG4)


//2 REG_FW_DBG5                            (Offset 0x10F4)


#define BIT_SHIFT_FW_DBG5                     	0
#define BIT_MASK_FW_DBG5                      	0xffffffffL
#define BIT_FW_DBG5(x)                        	(((x) & BIT_MASK_FW_DBG5) << BIT_SHIFT_FW_DBG5)


//2 REG_FW_DBG6                            (Offset 0x10F8)


#define BIT_SHIFT_FW_DBG6                     	0
#define BIT_MASK_FW_DBG6                      	0xffffffffL
#define BIT_FW_DBG6(x)                        	(((x) & BIT_MASK_FW_DBG6) << BIT_SHIFT_FW_DBG6)


//2 REG_FW_DBG7                            (Offset 0x10FC)


#define BIT_SHIFT_FW_DBG7                     	0
#define BIT_MASK_FW_DBG7                      	0xffffffffL
#define BIT_FW_DBG7(x)                        	(((x) & BIT_MASK_FW_DBG7) << BIT_SHIFT_FW_DBG7)


//2 REG_CR_EXT                             (Offset 0x1100)


#define BIT_SHIFT_PHY_REQ_DELAY               	24
#define BIT_MASK_PHY_REQ_DELAY                	0xf
#define BIT_PHY_REQ_DELAY(x)                  	(((x) & BIT_MASK_PHY_REQ_DELAY) << BIT_SHIFT_PHY_REQ_DELAY)

#define BIT_SPD_DOWN                          	BIT(16)

#define BIT_SHIFT_NETYPE4                     	4
#define BIT_MASK_NETYPE4                      	0x3
#define BIT_NETYPE4(x)                        	(((x) & BIT_MASK_NETYPE4) << BIT_SHIFT_NETYPE4)


#define BIT_SHIFT_NETYPE3                     	2
#define BIT_MASK_NETYPE3                      	0x3
#define BIT_NETYPE3(x)                        	(((x) & BIT_MASK_NETYPE3) << BIT_SHIFT_NETYPE3)


#define BIT_SHIFT_NETYPE2                     	0
#define BIT_MASK_NETYPE2                      	0x3
#define BIT_NETYPE2(x)                        	(((x) & BIT_MASK_NETYPE2) << BIT_SHIFT_NETYPE2)


//2 REG_FWFF                               (Offset 0x1114)


#define BIT_SHIFT_PKTNUM_TH                   	24
#define BIT_MASK_PKTNUM_TH                    	0xff
#define BIT_PKTNUM_TH(x)                      	(((x) & BIT_MASK_PKTNUM_TH) << BIT_SHIFT_PKTNUM_TH)


#define BIT_SHIFT_TIMER_TH                    	16
#define BIT_MASK_TIMER_TH                     	0xff
#define BIT_TIMER_TH(x)                       	(((x) & BIT_MASK_TIMER_TH) << BIT_SHIFT_TIMER_TH)


#define BIT_SHIFT_RXPKT1ENADDR                	0
#define BIT_MASK_RXPKT1ENADDR                 	0xffff
#define BIT_RXPKT1ENADDR(x)                   	(((x) & BIT_MASK_RXPKT1ENADDR) << BIT_SHIFT_RXPKT1ENADDR)


//2 REG_FT2IMR                             (Offset 0x1120)

#define BIT_FS_ATIM_MB7_INT_EN                	BIT(23)
#define BIT_FS_ATIM_MB6_INT_EN                	BIT(22)
#define BIT_FS_ATIM_MB5_INT_EN                	BIT(21)
#define BIT_FS_ATIM_MB4_INT_EN                	BIT(20)
#define BIT_FS_ATIM_MB3_INT_EN                	BIT(19)
#define BIT_FS_ATIM_MB2_INT_EN                	BIT(18)
#define BIT_FS_ATIM_MB1_INT_EN                	BIT(17)
#define BIT_FS_ATIM_MB0_INT_EN                	BIT(16)
#define BIT_FS_TBTT4INT_EN                    	BIT(11)
#define BIT_FS_TBTT3INT_EN                    	BIT(10)
#define BIT_FS_TBTT2INT_EN                    	BIT(9)
#define BIT_FS_TBTT1INT_EN                    	BIT(8)
#define BIT_FS_TBTT0_MB7INT_EN                	BIT(7)
#define BIT_FS_TBTT0_MB6INT_EN                	BIT(6)
#define BIT_FS_TBTT0_MB5INT_EN                	BIT(5)
#define BIT_FS_TBTT0_MB4INT_EN                	BIT(4)
#define BIT_FS_TBTT0_MB3INT_EN                	BIT(3)
#define BIT_FS_TBTT0_MB2INT_EN                	BIT(2)
#define BIT_FS_TBTT0_MB1INT_EN                	BIT(1)
#define BIT_FS_TBTT0_INT_EN                   	BIT(0)

//2 REG_FT2ISR                             (Offset 0x1124)

#define BIT_FS_ATIM_MB7_INT                   	BIT(23)
#define BIT_FS_ATIM_MB6_INT                   	BIT(22)
#define BIT_FS_ATIM_MB5_INT                   	BIT(21)
#define BIT_FS_ATIM_MB4_INT                   	BIT(20)
#define BIT_FS_ATIM_MB3_INT                   	BIT(19)
#define BIT_FS_ATIM_MB2_INT                   	BIT(18)
#define BIT_FS_ATIM_MB1_INT                   	BIT(17)
#define BIT_FS_ATIM_MB0_INT                   	BIT(16)
#define BIT_FS_TBTT4INT                       	BIT(11)
#define BIT_FS_TBTT3INT                       	BIT(10)
#define BIT_FS_TBTT2INT                       	BIT(9)
#define BIT_FS_TBTT1INT                       	BIT(8)
#define BIT_FS_TBTT0_MB7INT                   	BIT(7)
#define BIT_FS_TBTT0_MB6INT                   	BIT(6)
#define BIT_FS_TBTT0_MB5INT                   	BIT(5)
#define BIT_FS_TBTT0_MB4INT                   	BIT(4)
#define BIT_FS_TBTT0_MB3INT                   	BIT(3)
#define BIT_FS_TBTT0_MB2INT                   	BIT(2)
#define BIT_FS_TBTT0_MB1INT                   	BIT(1)
#define BIT_FS_TBTT0_INT                      	BIT(0)

//2 REG_FT3IMR                             (Offset 0x1128)

#define BIT_FS_BCNDMA4_INT_EN                 	BIT(27)
#define BIT_FS_BCNDMA3_INT_EN                 	BIT(26)
#define BIT_FS_BCNDMA2_INT_EN                 	BIT(25)
#define BIT_FS_BCNDMA1_INT_EN                 	BIT(24)
#define BIT_FS_BCNDMA0_MB7_INT_EN             	BIT(23)
#define BIT_FS_BCNDMA0_MB6_INT_EN             	BIT(22)
#define BIT_FS_BCNDMA0_MB5_INT_EN             	BIT(21)
#define BIT_FS_BCNDMA0_MB4_INT_EN             	BIT(20)
#define BIT_FS_BCNDMA0_MB3_INT_EN             	BIT(19)
#define BIT_FS_BCNDMA0_MB2_INT_EN             	BIT(18)
#define BIT_FS_BCNDMA0_MB1_INT_EN             	BIT(17)
#define BIT_FS_BCNDMA0_INT_EN                 	BIT(16)
#define BIT_FS_MTI_BCNIVLEAR__INT__EN         	BIT(15)
#define BIT_FS_BCNERLY4_INT_EN                	BIT(11)
#define BIT_FS_BCNERLY3_INT_EN                	BIT(10)
#define BIT_FS_BCNERLY2_INT_EN                	BIT(9)
#define BIT_FS_BCNERLY1_INT_EN                	BIT(8)
#define BIT_FS_BCNERLY0_MB7INT_EN             	BIT(7)
#define BIT_FS_BCNERLY0_MB6INT_EN             	BIT(6)
#define BIT_FS_BCNERLY0_MB5INT_EN             	BIT(5)
#define BIT_FS_BCNERLY0_MB4INT_EN             	BIT(4)
#define BIT_FS_BCNERLY0_MB3INT_EN             	BIT(3)
#define BIT_FS_BCNERLY0_MB2INT_EN             	BIT(2)
#define BIT_FS_BCNERLY0_MB1INT_EN             	BIT(1)
#define BIT_FS_BCNERLY0_INT_EN                	BIT(0)

//2 REG_FT3ISR                             (Offset 0x112C)

#define BIT_FS_BCNDMA4_INT                    	BIT(27)
#define BIT_FS_BCNDMA3_INT                    	BIT(26)
#define BIT_FS_BCNDMA2_INT                    	BIT(25)
#define BIT_FS_BCNDMA1_INT                    	BIT(24)
#define BIT_FS_BCNDMA0_MB7_INT                	BIT(23)
#define BIT_FS_BCNDMA0_MB6_INT                	BIT(22)
#define BIT_FS_BCNDMA0_MB5_INT                	BIT(21)
#define BIT_FS_BCNDMA0_MB4_INT                	BIT(20)
#define BIT_FS_BCNDMA0_MB3_INT                	BIT(19)
#define BIT_FS_BCNDMA0_MB2_INT                	BIT(18)
#define BIT_FS_BCNDMA0_MB1_INT                	BIT(17)
#define BIT_FS_BCNDMA0_INT                    	BIT(16)
#define BIT_FS_MTI_BCNIVLEAR__INT             	BIT(15)
#define BIT_FS_BCNERLY4_INT                   	BIT(11)
#define BIT_FS_BCNERLY3_INT                   	BIT(10)
#define BIT_FS_BCNERLY2_INT                   	BIT(9)
#define BIT_FS_BCNERLY1_INT                   	BIT(8)
#define BIT_FS_BCNERLY0_MB7INT                	BIT(7)
#define BIT_FS_BCNERLY0_MB6INT                	BIT(6)
#define BIT_FS_BCNERLY0_MB5INT                	BIT(5)
#define BIT_FS_BCNERLY0_MB4INT                	BIT(4)
#define BIT_FS_BCNERLY0_MB3INT                	BIT(3)
#define BIT_FS_BCNERLY0_MB2INT                	BIT(2)
#define BIT_FS_BCNERLY0_MB1INT                	BIT(1)
#define BIT_FS_BCNERLY0_INT                   	BIT(0)

//2 REG_FW2IMR                             (Offset 0x1130)

#define BIT_FS_DMEM1_WPTR_UPDATE_INT_EN       	BIT(2)
#define BIT_FS_MGNTQ_RPTR_RELEASE_INT_EN      	BIT(1)
#define BIT_FS_MGNTQFF_TO_INT_EN              	BIT(0)

//2 REG_FW2ISR                             (Offset 0x1134)

#define BIT_FS_DMEM1_WPTR_UPDATE_INT          	BIT(2)
#define BIT_FS_MGNTQ_RPTR_RELEASE_INT         	BIT(1)
#define BIT_FS_MGNTQFF_TO_INT                 	BIT(0)

//2 REG_FT1IMR                             (Offset 0x1138)

#define BIT_FS_TXSC__DESC_DONE_INT_EN         	BIT(28)
#define BIT_FS_TXSC__BKDONE_INT_EN            	BIT(27)
#define BIT_FS_TXSC__BEDONE_INT_EN            	BIT(26)
#define BIT_FS_TXSC__VIDONE_INT_EN            	BIT(25)
#define BIT_FS_TXSC__VODONE_INT_EN            	BIT(24)
#define BIT_FS_MACID_PWRCHANGE4_INT_EN        	BIT(22)
#define BIT_FS_MACID_PWRCHANGE3_INT_EN        	BIT(21)
#define BIT_FS_MACID_PWRCHANGE2_INT_EN        	BIT(20)
#define BIT_FS_MACID_PWRCHANGE1_INT_EN        	BIT(19)
#define BIT_FS_MACID_PWRCHANGE0_INT_EN        	BIT(18)
#define BIT_FS_CTWEnd2_INT_EN                 	BIT(17)
#define BIT_FS_CTWEnd1_INT_EN                 	BIT(16)
#define BIT_FS_CTWEnd0_INT_EN                 	BIT(15)
#define BIT_FS_TX_NULL1_INT_EN                	BIT(14)
#define BIT_FS_TX_NULL0_INT_EN                	BIT(13)
#define BIT_FS_TSF_BIT32_TOGGLE_EN            	BIT(12)
#define BIT_FS_P2P_RFON2_INT_EN               	BIT(11)
#define BIT_FS_P2P_RFOFF2_INT_EN              	BIT(10)
#define BIT_FS_P2P_RFON1_INT_EN               	BIT(9)
#define BIT_FS_P2P_RFOFF1_INT_EN              	BIT(8)
#define BIT_FS_P2P_RFON0_INT_EN               	BIT(7)
#define BIT_FS_P2P_RFOFF0_INT_EN              	BIT(6)
#define BIT_FS_RX_UAPSDMD1_EN                 	BIT(5)
#define BIT_FS_RX_UAPSDMD0_EN                 	BIT(4)
#define BIT_FS_TRIGGER_PKT_EN                 	BIT(3)
#define BIT_FS_EOSP_INT_EN                    	BIT(2)
#define BIT_FS_RPWM2_INT_EN                   	BIT(1)
#define BIT_FS_RPWM_INT_EN                    	BIT(0)

//2 REG_FT1ISR                             (Offset 0x113C)

#define BIT_FS_TXSC_DESC_DONE_INT             	BIT(28)
#define BIT_FS_TXSC_BKDONE_INT                	BIT(27)
#define BIT_FS_TXSC_BEDONE_INT                	BIT(26)
#define BIT_FS_TXSC_VIDONE_INT                	BIT(25)
#define BIT_FS_TXSC_VODONE_INT                	BIT(24)
#define BIT_FS_MACID_PWRCHANGE4_INT           	BIT(22)
#define BIT_FS_MACID_PWRCHANGE3_INT           	BIT(21)
#define BIT_FS_MACID_PWRCHANGE2_INT           	BIT(20)
#define BIT_FS_MACID_PWRCHANGE1_INT           	BIT(19)
#define BIT_FS_MACID_PWRCHANGE0_INT           	BIT(18)
#define BIT_FS_CTWEnd2_INT                    	BIT(17)
#define BIT_FS_CTWEnd1_INT                    	BIT(16)
#define BIT_FS_CTWEnd0_INT                    	BIT(15)
#define BIT_FS_TX_NULL1_INT                   	BIT(14)
#define BIT_FS_TX_NULL0_INT                   	BIT(13)
#define BIT_FS_TSF_BIT32_TOGGLE_INT           	BIT(12)
#define BIT_FS_P2P_RFON2_INT                  	BIT(11)
#define BIT_FS_P2P_RFOFF2_INT                 	BIT(10)
#define BIT_FS_P2P_RFON1_INT                  	BIT(9)
#define BIT_FS_P2P_RFOFF1_INT                 	BIT(8)
#define BIT_FS_P2P_RFON0_INT                  	BIT(7)
#define BIT_FS_P2P_RFOFF0_INT                 	BIT(6)
#define BIT_FS_RX_UAPSDMD1_INT                	BIT(5)
#define BIT_FS_RX_UAPSDMD0_INT                	BIT(4)
#define BIT_FS_TRIGGER_PKT_INT                	BIT(3)
#define BIT_FS_EOSP_INT                       	BIT(2)
#define BIT_FS_RPWM2_INT                      	BIT(1)
#define BIT_FS_RPWM_INT                       	BIT(0)

//2 REG_SPWR0                              (Offset 0x1140)


#define BIT_SHIFT_MID_31to0                   	0
#define BIT_MASK_MID_31to0                    	0xffffffffL
#define BIT_MID_31to0(x)                      	(((x) & BIT_MASK_MID_31to0) << BIT_SHIFT_MID_31to0)


//2 REG_SPWR1                              (Offset 0x1144)


#define BIT_SHIFT_MID_63to32                  	0
#define BIT_MASK_MID_63to32                   	0xffffffffL
#define BIT_MID_63to32(x)                     	(((x) & BIT_MASK_MID_63to32) << BIT_SHIFT_MID_63to32)


//2 REG_SPWR2                              (Offset 0x1148)


#define BIT_SHIFT_MID_95o64                   	0
#define BIT_MASK_MID_95o64                    	0xffffffffL
#define BIT_MID_95o64(x)                      	(((x) & BIT_MASK_MID_95o64) << BIT_SHIFT_MID_95o64)


//2 REG_SPWR3                              (Offset 0x114C)


#define BIT_SHIFT_MID_127to96                 	0
#define BIT_MASK_MID_127to96                  	0xffffffffL
#define BIT_MID_127to96(x)                    	(((x) & BIT_MASK_MID_127to96) << BIT_SHIFT_MID_127to96)


//2 REG_POWSEQ                             (Offset 0x1150)


#define BIT_SHIFT_SEQNUM_MID                  	16
#define BIT_MASK_SEQNUM_MID                   	0xffff
#define BIT_SEQNUM_MID(x)                     	(((x) & BIT_MASK_SEQNUM_MID) << BIT_SHIFT_SEQNUM_MID)


#define BIT_SHIFT_REF_MID                     	0
#define BIT_MASK_REF_MID                      	0x7f
#define BIT_REF_MID(x)                        	(((x) & BIT_MASK_REF_MID) << BIT_SHIFT_REF_MID)


//2 REG_MSG2                               (Offset 0x11F0)


#define BIT_SHIFT_FW_MSG2                     	0
#define BIT_MASK_FW_MSG2                      	0xffffffffL
#define BIT_FW_MSG2(x)                        	(((x) & BIT_MASK_FW_MSG2) << BIT_SHIFT_FW_MSG2)


//2 REG_MSG3                               (Offset 0x11F4)


#define BIT_SHIFT_FW_MSG3                     	0
#define BIT_MASK_FW_MSG3                      	0xffffffffL
#define BIT_FW_MSG3(x)                        	(((x) & BIT_MASK_FW_MSG3) << BIT_SHIFT_FW_MSG3)


//2 REG_MSG4                               (Offset 0x11F8)


#define BIT_SHIFT_FW_MSG4                     	0
#define BIT_MASK_FW_MSG4                      	0xffffffffL
#define BIT_FW_MSG4(x)                        	(((x) & BIT_MASK_FW_MSG4) << BIT_SHIFT_FW_MSG4)


//2 REG_MSG5                               (Offset 0x11FC)


#define BIT_SHIFT_FW_MSG5                     	0
#define BIT_MASK_FW_MSG5                      	0xffffffffL
#define BIT_FW_MSG5(x)                        	(((x) & BIT_MASK_FW_MSG5) << BIT_SHIFT_FW_MSG5)


//2 REG_Q0_Q1_INFO                         (Offset 0x1400)


#define BIT_SHIFT_ac1_pkt_info                	16
#define BIT_MASK_ac1_pkt_info                 	0xfff
#define BIT_ac1_pkt_info(x)                   	(((x) & BIT_MASK_ac1_pkt_info) << BIT_SHIFT_ac1_pkt_info)


#define BIT_SHIFT_ac0_pkt_info                	0
#define BIT_MASK_ac0_pkt_info                 	0xfff
#define BIT_ac0_pkt_info(x)                   	(((x) & BIT_MASK_ac0_pkt_info) << BIT_SHIFT_ac0_pkt_info)


//2 REG_Q2_Q3_INFO                         (Offset 0x1404)


#define BIT_SHIFT_ac3_pkt_info                	16
#define BIT_MASK_ac3_pkt_info                 	0xfff
#define BIT_ac3_pkt_info(x)                   	(((x) & BIT_MASK_ac3_pkt_info) << BIT_SHIFT_ac3_pkt_info)


#define BIT_SHIFT_ac2_pkt_info                	0
#define BIT_MASK_ac2_pkt_info                 	0xfff
#define BIT_ac2_pkt_info(x)                   	(((x) & BIT_MASK_ac2_pkt_info) << BIT_SHIFT_ac2_pkt_info)


//2 REG_Q4_Q5_INFO                         (Offset 0x1408)


#define BIT_SHIFT_ac5_pkt_info                	16
#define BIT_MASK_ac5_pkt_info                 	0xfff
#define BIT_ac5_pkt_info(x)                   	(((x) & BIT_MASK_ac5_pkt_info) << BIT_SHIFT_ac5_pkt_info)


#define BIT_SHIFT_ac4_pkt_info                	0
#define BIT_MASK_ac4_pkt_info                 	0xfff
#define BIT_ac4_pkt_info(x)                   	(((x) & BIT_MASK_ac4_pkt_info) << BIT_SHIFT_ac4_pkt_info)


//2 REG_Q6_Q7_INFO                         (Offset 0x140C)


#define BIT_SHIFT_ac7_pkt_info                	16
#define BIT_MASK_ac7_pkt_info                 	0xfff
#define BIT_ac7_pkt_info(x)                   	(((x) & BIT_MASK_ac7_pkt_info) << BIT_SHIFT_ac7_pkt_info)


#define BIT_SHIFT_ac6_pkt_info                	0
#define BIT_MASK_ac6_pkt_info                 	0xfff
#define BIT_ac6_pkt_info(x)                   	(((x) & BIT_MASK_ac6_pkt_info) << BIT_SHIFT_ac6_pkt_info)


//2 REG_MGQ_HIQ_INFO                       (Offset 0x1410)


#define BIT_SHIFT_hiq_pkt_info                	16
#define BIT_MASK_hiq_pkt_info                 	0xfff
#define BIT_hiq_pkt_info(x)                   	(((x) & BIT_MASK_hiq_pkt_info) << BIT_SHIFT_hiq_pkt_info)


#define BIT_SHIFT_mgq_pkt_info                	0
#define BIT_MASK_mgq_pkt_info                 	0xfff
#define BIT_mgq_pkt_info(x)                   	(((x) & BIT_MASK_mgq_pkt_info) << BIT_SHIFT_mgq_pkt_info)


//2 REG_CMDQ_BCNQ_INFO                     (Offset 0x1414)


#define BIT_SHIFT_cmdq_pkt_info               	16
#define BIT_MASK_cmdq_pkt_info                	0xfff
#define BIT_cmdq_pkt_info(x)                  	(((x) & BIT_MASK_cmdq_pkt_info) << BIT_SHIFT_cmdq_pkt_info)


#define BIT_SHIFT_bcnq_pkt_info               	0
#define BIT_MASK_bcnq_pkt_info                	0xfff
#define BIT_bcnq_pkt_info(x)                  	(((x) & BIT_MASK_bcnq_pkt_info) << BIT_SHIFT_bcnq_pkt_info)


//2 REG_USEREG_SETTING                     (Offset 0x1420)

#define BIT_NDPA_USEREG                       	BIT(21)

#define BIT_SHIFT_RETRY_USEREG                	19
#define BIT_MASK_RETRY_USEREG                 	0x3
#define BIT_RETRY_USEREG(x)                   	(((x) & BIT_MASK_RETRY_USEREG) << BIT_SHIFT_RETRY_USEREG)


#define BIT_SHIFT_TRYPKT_USEREG               	17
#define BIT_MASK_TRYPKT_USEREG                	0x3
#define BIT_TRYPKT_USEREG(x)                  	(((x) & BIT_MASK_TRYPKT_USEREG) << BIT_SHIFT_TRYPKT_USEREG)

#define BIT_CTLPKT_USEREG                     	BIT(16)

//2 REG_AESIV_SETTING                      (Offset 0x1424)


#define BIT_SHIFT_AESIV_OFFSET                	0
#define BIT_MASK_AESIV_OFFSET                 	0xfff
#define BIT_AESIV_OFFSET(x)                   	(((x) & BIT_MASK_AESIV_OFFSET) << BIT_SHIFT_AESIV_OFFSET)


//2 REG_BF0_TIME_SETTING                   (Offset 0x1428)

#define BIT_bf0_timer_set                     	BIT(31)
#define BIT_bf0_timer_clr                     	BIT(30)
#define BIT_bf0_update_en                     	BIT(29)
#define BIT_bf0_timer_en                      	BIT(28)

#define BIT_SHIFT_bf0_pretime_over            	16
#define BIT_MASK_bf0_pretime_over             	0xfff
#define BIT_bf0_pretime_over(x)               	(((x) & BIT_MASK_bf0_pretime_over) << BIT_SHIFT_bf0_pretime_over)


#define BIT_SHIFT_bf0_lifetime                	0
#define BIT_MASK_bf0_lifetime                 	0xffff
#define BIT_bf0_lifetime(x)                   	(((x) & BIT_MASK_bf0_lifetime) << BIT_SHIFT_bf0_lifetime)


//2 REG_BF1_TIME_SETTING                   (Offset 0x142C)

#define BIT_bf1_timer_set                     	BIT(31)
#define BIT_bf1_timer_clr                     	BIT(30)
#define BIT_bf1_update_en                     	BIT(29)
#define BIT_bf1_timer_en                      	BIT(28)

#define BIT_SHIFT_bf1_pretime_over            	16
#define BIT_MASK_bf1_pretime_over             	0xfff
#define BIT_bf1_pretime_over(x)               	(((x) & BIT_MASK_bf1_pretime_over) << BIT_SHIFT_bf1_pretime_over)


#define BIT_SHIFT_bf1_lifetime                	0
#define BIT_MASK_bf1_lifetime                 	0xffff
#define BIT_bf1_lifetime(x)                   	(((x) & BIT_MASK_bf1_lifetime) << BIT_SHIFT_bf1_lifetime)


//2 REG_BF_TIMOUT_EN                       (Offset 0x1430)

#define BIT_EN_VHT_LDPC                       	BIT(9)
#define BIT_EN_HT_LDPC                        	BIT(8)
#define BIT_bf1_timeout_en                    	BIT(1)
#define BIT_bf0_timeout_en                    	BIT(0)

//2 REG_MACID_RELEASE0                     (Offset 0x1434)


#define BIT_SHIFT_MACID31_0_RELEASE           	0
#define BIT_MASK_MACID31_0_RELEASE            	0xffffffffL
#define BIT_MACID31_0_RELEASE(x)              	(((x) & BIT_MASK_MACID31_0_RELEASE) << BIT_SHIFT_MACID31_0_RELEASE)


//2 REG_MACID_RELEASE1                     (Offset 0x1438)


#define BIT_SHIFT_MACID63_32_RELEASE          	0
#define BIT_MASK_MACID63_32_RELEASE           	0xffffffffL
#define BIT_MACID63_32_RELEASE(x)             	(((x) & BIT_MASK_MACID63_32_RELEASE) << BIT_SHIFT_MACID63_32_RELEASE)


//2 REG_MACID_RELEASE2                     (Offset 0x143C)


#define BIT_SHIFT_MACID95_64_RELEASE          	0
#define BIT_MASK_MACID95_64_RELEASE           	0xffffffffL
#define BIT_MACID95_64_RELEASE(x)             	(((x) & BIT_MASK_MACID95_64_RELEASE) << BIT_SHIFT_MACID95_64_RELEASE)


//2 REG_MACID_RELEASE3                     (Offset 0x1440)


#define BIT_SHIFT_MACID127_96_RELEASE         	0
#define BIT_MASK_MACID127_96_RELEASE          	0xffffffffL
#define BIT_MACID127_96_RELEASE(x)            	(((x) & BIT_MASK_MACID127_96_RELEASE) << BIT_SHIFT_MACID127_96_RELEASE)


//2 REG_MACID_RELEASE_SETTING              (Offset 0x1444)

#define BIT_macid_value                       	BIT(7)

#define BIT_SHIFT_macid_offset                	0
#define BIT_MASK_macid_offset                 	0x7f
#define BIT_macid_offset(x)                   	(((x) & BIT_MASK_macid_offset) << BIT_SHIFT_macid_offset)


//2 REG_FAST_EDCA_VOVI_SETTING             (Offset 0x1448)


#define BIT_SHIFT_VI_FAST_EDCA_TO             	24
#define BIT_MASK_VI_FAST_EDCA_TO              	0xff
#define BIT_VI_FAST_EDCA_TO(x)                	(((x) & BIT_MASK_VI_FAST_EDCA_TO) << BIT_SHIFT_VI_FAST_EDCA_TO)

#define BIT_VI_THRESHOLD_SEL                  	BIT(23)

#define BIT_SHIFT_VI_FAST_EDCA_PKT_TH         	16
#define BIT_MASK_VI_FAST_EDCA_PKT_TH          	0x7f
#define BIT_VI_FAST_EDCA_PKT_TH(x)            	(((x) & BIT_MASK_VI_FAST_EDCA_PKT_TH) << BIT_SHIFT_VI_FAST_EDCA_PKT_TH)


#define BIT_SHIFT_VO_FAST_EDCA_TO             	8
#define BIT_MASK_VO_FAST_EDCA_TO              	0xff
#define BIT_VO_FAST_EDCA_TO(x)                	(((x) & BIT_MASK_VO_FAST_EDCA_TO) << BIT_SHIFT_VO_FAST_EDCA_TO)

#define BIT_VO_THRESHOLD_SEL                  	BIT(7)

#define BIT_SHIFT_VO_FAST_EDCA_PKT_TH         	0
#define BIT_MASK_VO_FAST_EDCA_PKT_TH          	0x7f
#define BIT_VO_FAST_EDCA_PKT_TH(x)            	(((x) & BIT_MASK_VO_FAST_EDCA_PKT_TH) << BIT_SHIFT_VO_FAST_EDCA_PKT_TH)


//2 REG_FAST_EDCA_BEBK_SETTING             (Offset 0x144C)


#define BIT_SHIFT_BK_FAST_EDCA_TO             	24
#define BIT_MASK_BK_FAST_EDCA_TO              	0xff
#define BIT_BK_FAST_EDCA_TO(x)                	(((x) & BIT_MASK_BK_FAST_EDCA_TO) << BIT_SHIFT_BK_FAST_EDCA_TO)

#define BIT_BK_THRESHOLD_SEL                  	BIT(23)

#define BIT_SHIFT_BK_FAST_EDCA_PKT_TH         	16
#define BIT_MASK_BK_FAST_EDCA_PKT_TH          	0x7f
#define BIT_BK_FAST_EDCA_PKT_TH(x)            	(((x) & BIT_MASK_BK_FAST_EDCA_PKT_TH) << BIT_SHIFT_BK_FAST_EDCA_PKT_TH)


#define BIT_SHIFT_BE_FAST_EDCA_TO             	8
#define BIT_MASK_BE_FAST_EDCA_TO              	0xff
#define BIT_BE_FAST_EDCA_TO(x)                	(((x) & BIT_MASK_BE_FAST_EDCA_TO) << BIT_SHIFT_BE_FAST_EDCA_TO)

#define BIT_BE_THRESHOLD_SEL                  	BIT(7)

#define BIT_SHIFT_BE_FAST_EDCA_PKT_TH         	0
#define BIT_MASK_BE_FAST_EDCA_PKT_TH          	0x7f
#define BIT_BE_FAST_EDCA_PKT_TH(x)            	(((x) & BIT_MASK_BE_FAST_EDCA_PKT_TH) << BIT_SHIFT_BE_FAST_EDCA_PKT_TH)


//2 REG_MACID_DROP0                        (Offset 0x1450)


#define BIT_SHIFT_MACID31_0_DROP              	0
#define BIT_MASK_MACID31_0_DROP               	0xffffffffL
#define BIT_MACID31_0_DROP(x)                 	(((x) & BIT_MASK_MACID31_0_DROP) << BIT_SHIFT_MACID31_0_DROP)


//2 REG_MACID_DROP1                        (Offset 0x1454)


#define BIT_SHIFT_MACID63_32_DROP             	0
#define BIT_MASK_MACID63_32_DROP              	0xffffffffL
#define BIT_MACID63_32_DROP(x)                	(((x) & BIT_MASK_MACID63_32_DROP) << BIT_SHIFT_MACID63_32_DROP)


//2 REG_MACID_DROP2                        (Offset 0x1458)


#define BIT_SHIFT_MACID95_64_DROP             	0
#define BIT_MASK_MACID95_64_DROP              	0xffffffffL
#define BIT_MACID95_64_DROP(x)                	(((x) & BIT_MASK_MACID95_64_DROP) << BIT_SHIFT_MACID95_64_DROP)


//2 REG_MACID_DROP3                        (Offset 0x145C)


#define BIT_SHIFT_MACID127_96_DROP            	0
#define BIT_MASK_MACID127_96_DROP             	0xffffffffL
#define BIT_MACID127_96_DROP(x)               	(((x) & BIT_MASK_MACID127_96_DROP) << BIT_SHIFT_MACID127_96_DROP)

#endif




#endif//__RTL_WLAN_BITDEF_H__

