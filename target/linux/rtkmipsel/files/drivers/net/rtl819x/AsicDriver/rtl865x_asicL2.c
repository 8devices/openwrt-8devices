/*
* Copyright c                  Realtek Semiconductor Corporation, 2009
* All rights reserved.
*
* Program : Switch table Layer2 switch driver,following features are included:
*	PHY/MII/Port/STP/QOS
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)
*/
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
//#include "assert.h"
#include "rtl865x_asicBasic.h"
#include "rtl865x_asicCom.h"
#include "rtl865x_asicL2.h"
#if (defined(CONFIG_RTL_8197F) && (defined(CONFIG_RTL_8211F_SUPPORT) || defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT))) && defined(CONFIG_RTL819X_GPIO) && !defined(CONFIG_OPENWRT_SDK)
#include <../bsp/bspchip.h>
#endif
#include "asicRegs.h"
//#include "rtl_utils.h"
#include "rtl865x_hwPatch.h"

#include <linux/delay.h>

#if defined(CONFIG_RTL819X_GPIO) && !defined(CONFIG_OPENWRT_SDK)
#include <linux/platform_device.h>
#include <linux/platform_data/rtl819x-gpio.h>
#include <linux/gpio.h>
#endif

static uint8 fidHashTable[]={0x00,0x0f,0xf0,0xff};

__DRAM_FWD int32		rtl865x_wanPortMask;
int32		rtl865x_lanPortMask = RTL865X_PORTMASK_UNASIGNED;

int32		rtl865x_maxPreAllocRxSkb = RTL865X_PREALLOC_SKB_UNASIGNED;
int32		rtl865x_rxSkbPktHdrDescNum = RTL865X_PREALLOC_SKB_UNASIGNED;
int32		rtl865x_txSkbPktHdrDescNum = RTL865X_PREALLOC_SKB_UNASIGNED;
int32 	miiPhyAddress;
rtl8651_tblAsic_ethernet_t 	rtl8651AsicEthernetTable[9];//RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum

#if defined(REINIT_SWITCH_CORE)
extern int rtl865x_duringReInitSwtichCore;
#endif

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
static uint32 _rtl865xC_QM_orgDescUsage = 0;	/* Original Descriptor Usage in HW */
#endif

/* For Bandwidth control - RTL865xB Backward compatible only */
#define _RTL865XB_BANDWIDTHCTRL_X1			(1 << 0)
#define _RTL865XB_BANDWIDTHCTRL_X4			(1 << 1)
#define _RTL865XB_BANDWIDTHCTRL_X8			(1 << 2)
#define _RTL865XB_BANDWIDTHCTRL_CFGTYPE		2		/* Ingress (0) and Egress (1) : 2 types of configuration */
static int32 _rtl865xB_BandwidthCtrlMultiplier = _RTL865XB_BANDWIDTHCTRL_X1;
static uint32 _rtl865xB_BandwidthCtrlPerPortConfiguration[RTL8651_PORT_NUMBER][_RTL865XB_BANDWIDTHCTRL_CFGTYPE /* Ingress (0), Egress (1) */ ];
static uint32 _rtl865xC_BandwidthCtrlNum[] = {	0,	/* BW_FULL_RATE */
														131072,	/* BW_128K */
														262144,	/* BW_256K */
														524288,	/* BW_512K */
														1048576,	/* BW_1M */
														2097152,	/* BW_2M */
														4194304,	/* BW_4M */
														8388608	/* BW_8M */
														};

#define	RTL865XC_INGRESS_16KUNIT	16384
#define	RTL865XC_EGRESS_64KUNIT	65535

#define RTL8198C_PORT0_PHY_ID		8
#define RTL8198C_GPHY_CALIBRATION		1
//#define ENABLE_8198C_GREEN			1

#if defined(CONFIG_RTL_8198C)
int giga_lite_enabled = 0;
int phy_link_sts = 0;
#endif

#ifdef CONFIG_RTL8196C_ETH_IOT
extern uint32 port_link_sts, port_linkpartner_eee;
#endif

#if defined(CONFIG_RTL_8196C)

#define RTL8196C_EEE_REFINE		1

int eee_enabled = 1;
void eee_phy_enable(void);
void eee_phy_disable(void);
void eee_phy_enable_by_port(int port);
#elif defined(RTL8198_EEE_MAC)
int eee_enabled = 1;
void eee_phy_enable_98(void);
void eee_phy_disable_98(void);
#else
int eee_enabled = 0;
#endif

#if defined(CONFIG_RTL_8198_NFBI_BOARD)
#define RTL8198_NFBI_PORT5_GMII 1 //mark_nfbi , default port5 set to GMII , you can undef here to set to MII mode!!!
//#undef RTL8198_NFBI_PORT5_GMII  //mark_nfbi , default port5 set to GMII , you can undef here to set to MII mode!!!
#endif


#if defined(CONFIG_RTL_8198)

#define PORT5_RGMII_GMII		1

//static void qos_init(void);
#endif

#define	QNUM_IDX_123		0
#define	QNUM_IDX_45		1
#define	QNUM_IDX_6		2

#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
static int32 _rtl865x_setQosThresholdByQueueIdx(uint32 qidx);

#if 0
static rtl865xC_outputQueuePara_t	outputQueuePara[3] = {
										{
											1, 		/* default: Bandwidth Control Include/exclude Preamble & IFG */
											20, 		/* default: Per Queue Physical Length Gap = 20 */
											504, 		/* default: Descriptor Run Out Threshold = 504 */
											180, 		/*default: System shared buffer flow control turn off threshold = 212 */
											196,		/*default: System shared buffer flow control turn on threshold = 248 */
											500, 		/*default: system flow control turn off threshold = 500*/
											502,		/*default: system flow control turn on threshold = 502*/
											330, 		/*default: port base flow control turn off threshold = 0xf8*/
											400,		/*default: port base flow control turn on threshold = 0x108*/
											31, 		/* Queue-Descriptor=Based Flow Control turn off Threshold =0x14 */
											48, 		/* Queue-Descriptor=Based Flow Control turn on Threshold = 0x21 */
											0x03, 	/* Queue-Packet=Based Flow Control turn off Threshold = 0x03 */
											0x05	/* Queue-Packet=Based Flow Control turn on Threshold =0x05 */
										},
										{
											1, 		/* default: Bandwidth Control Include/exclude Preamble & IFG */
											20, 		/* default: Per Queue Physical Length Gap = 20 */
											504, 		/* default: Descriptor Run Out Threshold = 504 */
											120, 		/*default: System shared buffer flow control turn off threshold = 212 */
											136,		/*default: System shared buffer flow control turn on threshold = 248 */
											330, 		/*default: system flow control turn off threshold = 500*/
											344,		/*default: system flow control turn on threshold = 502*/
											248, 		/*default: port base flow control turn off threshold = 0xf8*/
											264,		/*default: port base flow control turn on threshold = 0x108*/
											20, 		/* Queue-Descriptor=Based Flow Control turn off Threshold =0x14 */
											33, 		/* Queue-Descriptor=Based Flow Control turn on Threshold = 0x21 */
											0x03, 	/* Queue-Packet=Based Flow Control turn off Threshold = 0x03 */
											0x05	/* Queue-Packet=Based Flow Control turn on Threshold =0x05 */
										},
										{
											1, 		/* default: Bandwidth Control Include/exclude Preamble & IFG */
											20, 		/* default: Per Queue Physical Length Gap = 20 */
											500, 		/* default: Descriptor Run Out Threshold = 504 */
											324, 		/*default: System shared buffer flow control turn off threshold = 212 */
											340,		/*default: System shared buffer flow control turn on threshold = 248 */
											330, 		/*default: system flow control turn off threshold = 500*/
											400,		/*default: system flow control turn on threshold = 502*/
											240, 		/*default: port base flow control turn off threshold = 0xf8*/
											282,		/*default: port base flow control turn on threshold = 0x108*/
											20, 		/* Queue-Descriptor=Based Flow Control turn off Threshold =0x14 */
											28, 		/* Queue-Descriptor=Based Flow Control turn on Threshold = 0x21 */
											10, 	/* Queue-Packet=Based Flow Control turn off Threshold = 0x03 */
											11	/* Queue-Packet=Based Flow Control turn on Threshold =0x05 */
										}
										};
#else
// sync from kernel 2.4 qos_init() -- 20110330
static rtl865xC_outputQueuePara_t	outputQueuePara[3] = {
										{
											1, 		/* default: Bandwidth Control Include/exclude Preamble & IFG */
											0x48, 		/* default: Per Queue Physical Length Gap = 20 */
											500, 		/* default: Descriptor Run Out Threshold = 504 */
											252, 		/*default: System shared buffer flow control turn off threshold = 212 */
											310,		/*default: System shared buffer flow control turn on threshold = 248 */
											280, 		/*default: system flow control turn off threshold = 500*/
											400,		/*default: system flow control turn on threshold = 502*/
											258, 		/*default: port base flow control turn off threshold = 0xf8*/
											320,		/*default: port base flow control turn on threshold = 0x108*/
											16, 		/* Queue-Descriptor=Based Flow Control turn off Threshold =0x14 */
											21, 		/* Queue-Descriptor=Based Flow Control turn on Threshold = 0x21 */
											5, 	/* Queue-Packet=Based Flow Control turn off Threshold = 0x03 */
											6	/* Queue-Packet=Based Flow Control turn on Threshold =0x05 */
										},
										{
											1, 		/* default: Bandwidth Control Include/exclude Preamble & IFG */
											0x48, 		/* default: Per Queue Physical Length Gap = 20 */
											500, 		/* default: Descriptor Run Out Threshold = 504 */
											252, 		/*default: System shared buffer flow control turn off threshold = 212 */
											310,		/*default: System shared buffer flow control turn on threshold = 248 */
											280, 		/*default: system flow control turn off threshold = 500*/
											400,		/*default: system flow control turn on threshold = 502*/
											258, 		/*default: port base flow control turn off threshold = 0xf8*/
											320,		/*default: port base flow control turn on threshold = 0x108*/
											16, 		/* Queue-Descriptor=Based Flow Control turn off Threshold =0x14 */
											21, 		/* Queue-Descriptor=Based Flow Control turn on Threshold = 0x21 */
											5, 	/* Queue-Packet=Based Flow Control turn off Threshold = 0x03 */
											6	/* Queue-Packet=Based Flow Control turn on Threshold =0x05 */
										},
										{
											1, 		/* default: Bandwidth Control Include/exclude Preamble & IFG */
											0x48, 		/* default: Per Queue Physical Length Gap = 20 */
											500, 		/* default: Descriptor Run Out Threshold = 504 */
											252, 		/*default: System shared buffer flow control turn off threshold = 212 */
											310,		/*default: System shared buffer flow control turn on threshold = 248 */
											280, 		/*default: system flow control turn off threshold = 500*/
											400,		/*default: system flow control turn on threshold = 502*/
											258, 		/*default: port base flow control turn off threshold = 0xf8*/
											320,		/*default: port base flow control turn on threshold = 0x108*/
											16, 		/* Queue-Descriptor=Based Flow Control turn off Threshold =0x14 */
											21, 		/* Queue-Descriptor=Based Flow Control turn on Threshold = 0x21 */
											5, 	/* Queue-Packet=Based Flow Control turn off Threshold = 0x03 */
											6	/* Queue-Packet=Based Flow Control turn on Threshold =0x05 */
										}
										};
#endif
#elif defined(CONFIG_RTL_8198C)
//port base flow control turn off threshold  for 98C
static uint32 rtl_portFCOFF=0x34c ;
static uint32 rtl_portFCON=0x358;
#elif defined(CONFIG_RTL_8197F)
// todo: need to find out
static uint32 rtl_portFCOFF=0x1A6;
static uint32 rtl_portFCON=0x1AC;
#endif

static void _rtl8651_syncToAsicEthernetBandwidthControl(void);
#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
static int32 _rtl865xC_QM_init( void );
extern int32 swNic_freeRxRing(void);
#if defined (CONFIG_RTL_HW_QOS_DEBUG)
extern int net_ratelimit(void);
#endif
#endif

#if defined(CONFIG_RTL_8196C)
// EEE PHY -- Page 4
// register 16
#define P4R16_eee_10_cap                           (1 << 13)	// enable EEE 10M
#define P4R16_eee_nway_en                           (1 << 12)	// enable Next Page exchange in nway for EEE 100M
#define P4R16_tx_quiet_en                            (1 << 9)	// enable ability to turn off pow100tx when TX Quiet state
#define P4R16_rx_quiet_en                            (1 << 8)	// enable ability to turn off pow100rx when RX Quiet state

// register 25
#define P4R25_rg_dacquiet_en                            (1 << 10)	// enable ability to turn off DAC when TX Quiet state
#define P4R25_rg_ldvquiet_en                            (1 << 9)		// enable ability to turn off line driver when TX Quiet state
#define P4R25_rg_eeeprg_rst                            (1 << 6)		// reset for EEE programmable finite state machine
#define P4R25_rg_ckrsel                            		(1 << 5)		// select ckr125 as RX 125MHz clock
#define P4R25_rg_eeeprg_en                            (1 << 4)		// enable EEE programmable finite state machine

static const unsigned short phy_data[]={
	0x5000,  // write, address 0
	0x6000,  // write, address 1
	0x7000,  // write, address 2
	0x4000,  // write, address 3
	0xD36C,  // write, address 4
	0xFFFF,  // write, address 5
	0x5060,  // write, address 6
	0x61C5,  // write, address 7
	0x7000,  // write, address 8
	0x4001,  // write, address 9
	0x5061,  // write, address 10
	0x87F5,  // write, address 11
	0xCE60,  // write, address 12
	0x0026,  // write, address 13
	0x8E03,  // write, address 14
	0xA021,  // write, address 15
	0x300B,  // write, address 16
	0x58A0,  // write, address 17
	0x629C,  // write, address 18
	0x7000,  // write, address 19
	0x4002,  // write, address 20
	0x58A1,  // write, address 21
	0x87EA,  // write, address 22
	0xAE25,  // write, address 23
	0xA018,  // write, address 24
	0x3016,  // write, address 25
	0x6894,  // write, address 26
	0x6094,  // write, address 27
	0x5123,  // write, address 28
	0x63C2,  // write, address 29
	0x5127,  // write, address 30
	0x4003,  // write, address 31
	0x87E0,  // write, address 32
	0x8EF3,  // write, address 33
	0xA10E,  // write, address 34
	0xCC40,  // write, address 35
	0x0007,  // write, address 36
	0xCA40,  // write, address 37
	0xFFE0,  // write, address 38
	0xA202,  // write, address 39
	0x3020,  // write, address 40
	0x7008,  // write, address 41
	0x3020,  // write, address 42
	0xCC44,  // write, address 43
	0xFFF4,  // write, address 44
	0xCC44,  // write, address 45
	0xFFF2,  // write, address 46
	0x3000,  // write, address 47
	0x5220,  // write, address 48
	0x4004,  // write, address 49
	0x3000,  // write, address 50
	0x64A0,  // write, address 51
	0x5429,  // write, address 52
	0x4005,  // write, address 53
	0x87CA,  // write, address 54
	0xCE18,  // write, address 55
	0xFFC8,  // write, address 56
	0xCE64,  // write, address 57
	0xFFD0,  // write, address 58
	0x3036,  // write, address 59
	0x65C0,  // write, address 60
	0x50A9,  // write, address 61
	0x4006,  // write, address 62
	0xA3DB,  // write, address 63
	0x303F,  // write, address 64
};

static int ram_code_done=0;

void set_ram_code(void)
{
	uint32 reg;
	int i, len=sizeof(phy_data)/sizeof(unsigned short);

	if (ram_code_done)
		return;

	rtl8651_getAsicEthernetPHYReg( 4, 0x19, &reg );

	// turn on rg_eeeprg_rst
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_en)) | P4R25_rg_eeeprg_rst));

	// turn on mem_mdio_mode
	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0180);

	// begin to write all RAM
	for(i=0;i<len;i++) {
		rtl8651_setAsicEthernetPHYReg(4, 0x1d, phy_data[i]);
	}

	for(i=0;i<63;i++) {
		rtl8651_setAsicEthernetPHYReg(4, 0x1d, 0);
	}

	// finish reading all RAM
	// turn off mem_mdio_mode
	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0080);

	// turn off rg_eeeprg_rst, enable EEE programmable finite state machine
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_rst)) | P4R25_rg_eeeprg_en));

	ram_code_done = 1;
}

static const unsigned short phy_data_b[]={
  0x5000,  // write, address 0
  0x6000,  // write, address 1
  0x7000,  // write, address 2
  0x4000,  // write, address 3
  0x8700,  // write, address 4
  0xD344,  // write, address 5
  0xFFFF,  // write, address 6
  0xCA6C,  // write, address 7
  0xFFFD,  // write, address 8
  0x5460,  // write, address 9
  0x61C5,  // write, address 10
  0x7000,  // write, address 11
  0x4001,  // write, address 12
  0x5461,  // write, address 13
  0x4001,  // write, address 14
  0x87F1,  // write, address 15
  0xCE60,  // write, address 16
  0x0026,  // write, address 17
  0x8E03,  // write, address 18
  0xA021,  // write, address 19
  0x300F,  // write, address 20
  0x5CA0,  // write, address 21
  0x629C,  // write, address 22
  0x7000,  // write, address 23
  0x4002,  // write, address 24
  0x5CA1,  // write, address 25
  0x87E6,  // write, address 26
  0xAE25,  // write, address 27
  0xA018,  // write, address 28
  0x301A,  // write, address 29
  0x6E94,  // write, address 30
  0x6694,  // write, address 31
  0x5523,  // write, address 32
  0x63C2,  // write, address 33
  0x5527,  // write, address 34
  0x4003,  // write, address 35
  0x87DC,  // write, address 36
  0x8EF3,  // write, address 37
  0xA10E,  // write, address 38
  0xCC40,  // write, address 39
  0x0007,  // write, address 40
  0xCA40,  // write, address 41
  0xFFDF,  // write, address 42
  0xA202,  // write, address 43
  0x3024,  // write, address 44
  0x7008,  // write, address 45
  0x3024,  // write, address 46
  0xCC44,  // write, address 47
  0xFFF4,  // write, address 48
  0xCC44,  // write, address 49
  0xFFF2,  // write, address 50
  0x3000,  // write, address 51
  0x5620,  // write, address 52
  0x4004,  // write, address 53
  0x3000,  // write, address 54
  0x64A0,  // write, address 55
  0x5429,  // write, address 56
  0x4005,  // write, address 57
  0x87C6,  // write, address 58
  0xCE18,  // write, address 59
  0xFFC4,  // write, address 60
  0xCE64,  // write, address 61
  0xFFCF,  // write, address 62
  0x303A,  // write, address 63
  0x65C0,  // write, address 64
  0x54A9,  // write, address 65
  0x4006,  // write, address 66
  0xA3DB,  // write, address 67
  0x3043,  // write, address 68
};

#ifdef RTL8196C_EEE_REFINE
void cmp_eee_ram_code(int mode)
{
	uint32 reg;
	int i, len=sizeof(phy_data_b)/sizeof(unsigned short);

	printk(" => comparing eee ram code\n");

	// change to page 4
	rtl8651_setAsicEthernetPHYReg(4, 31, 4);

	for(i=0;i<5;i++) {
		rtl8651_getAsicEthernetPHYReg(i, 0x19, &reg );
		rtl8651_setAsicEthernetPHYReg(i, 0x19, ((reg & ~(P4R25_rg_eeeprg_en)) | P4R25_rg_eeeprg_rst));
	}

	rtl8651_setAsicEthernetPHYReg(4, 29, 0x0000);
	rtl8651_setAsicEthernetPHYReg(4, 28, 0x0180);
	rtl8651_setAsicEthernetPHYReg(4, 28, 0x017F);
	rtl8651_setAsicEthernetPHYReg(4, 28, 0x0180);

	for(i=0;i<len;i++) {
		rtl8651_getAsicEthernetPHYReg(4, 0x1d, &reg);
		if (mode == 0)
			printk(" => [%d]  %x\n", i, reg);
		else if (mode == 1) {
			if (reg != phy_data_b[i])
				printk(" => different: idx= %d, readback: %x, expect: %x\n",i, reg, phy_data_b[i]);				
		}
	}

	rtl8651_setAsicEthernetPHYReg(4, 28, 0x017E);
	rtl8651_setAsicEthernetPHYReg(4, 29, 0x0001);
	rtl8651_setAsicEthernetPHYReg(4, 28, 0x017F);
	rtl8651_setAsicEthernetPHYReg(4, 28, 0x0180);
	rtl8651_setAsicEthernetPHYReg(4, 28, 0x0080);

	for(i=0;i<5;i++) {
		rtl8651_getAsicEthernetPHYReg(i, 0x19, &reg );
		rtl8651_setAsicEthernetPHYReg(i, 0x19, ((reg & ~(P4R25_rg_eeeprg_rst)) | P4R25_rg_eeeprg_en));
	}

	// switch to page 0
	rtl8651_setAsicEthernetPHYReg(4, 31, 0 );
	printk(" => compare done\n");
	return;
}

void set_ram_code_b(void)
{
	uint32 reg;
	int i, len=sizeof(phy_data_b)/sizeof(unsigned short);

	if (ram_code_done)
		return;

	/* 2. disable all of the fephy access 
	    wr  0   4  25  0x0760  
	    wr  1   4  25  0x0760
	    wr  2   4  25  0x0760
	    wr  3   4  25  0x0760
	    wr  4   4  25  0x0760
	    ( bit 6 = 1  /  bit4 = 0 )
	 */
	for(i=0;i<5;i++) {
		rtl8651_getAsicEthernetPHYReg(i, 0x19, &reg );
		rtl8651_setAsicEthernetPHYReg(i, 0x19, ((reg & ~(P4R25_rg_eeeprg_en)) | P4R25_rg_eeeprg_rst));
	}
	/* 3. enable memory access
	    wr  4  4  29  0x0000  //  
           wr  4  4  28  0x0180  //  
	    wr  4  4  28  0x017F  // mem_mdio_en = 1 / ram bist control = 0x00
	    wr  4   4  28  0x0180 // mem_mdio_en = 1
	 */
	rtl8651_setAsicEthernetPHYReg(4, 29, 0x0000);
	rtl8651_setAsicEthernetPHYReg(4, 28, 0x0180);
	rtl8651_setAsicEthernetPHYReg(4, 28, 0x017F);
	rtl8651_setAsicEthernetPHYReg(4, 28, 0x0180);

	// 4. start to write patch 
	for(i=0;i<len;i++) {
		rtl8651_setAsicEthernetPHYReg(4, 0x1d, phy_data_b[i]);
	}

	/* 5. disable memory access
	    wr  4  4  28   0x017E  //
	    wr  4  4  29   0x0001  //  ram 0x7E  =  0x01
	    wr  4  4  28   0x017F  //  ram bist control = 0x01
	    wr  4  4  28   0x0180  //  addr = 0x00	    
	    wr  4  4  28   0x0080  //  mem_mdio_en = 0
	 */
	rtl8651_setAsicEthernetPHYReg(4, 28, 0x017E);
	rtl8651_setAsicEthernetPHYReg(4, 29, 0x0001);
	rtl8651_setAsicEthernetPHYReg(4, 28, 0x017F);
	rtl8651_setAsicEthernetPHYReg(4, 28, 0x0180);
	rtl8651_setAsicEthernetPHYReg(4, 28, 0x0080);

	/* 6. reenable all of the fephy access
	    wr  0   4  25  0x0730  
	    wr  1   4  25  0x0730
	    wr  2   4  25  0x0730
	    wr  3   4  25  0x0730
	    wr  4   4  25  0x0730
	    ( bit 6 = 0  /  bit4 = 1 )	
	 */
	for(i=0;i<5;i++) {
		rtl8651_getAsicEthernetPHYReg(i, 0x19, &reg );
		rtl8651_setAsicEthernetPHYReg(i, 0x19, ((reg & ~(P4R25_rg_eeeprg_rst)) | P4R25_rg_eeeprg_en));
	}
	ram_code_done = 1;
}
#else
void set_ram_code_b(void)
{
	uint32 reg;
	int i, len=sizeof(phy_data_b)/sizeof(unsigned short);

	if (ram_code_done)
		return;

	rtl8651_getAsicEthernetPHYReg(4, 0x19, &reg );
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_en)) | P4R25_rg_eeeprg_rst));
	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0180);

	for(i=0;i<len;i++) {
		rtl8651_setAsicEthernetPHYReg(4, 0x1d, phy_data_b[i]);
	}

	rtl8651_setAsicEthernetPHYReg(4, 0x1c, 0x0080);
	rtl8651_setAsicEthernetPHYReg(4, 0x19, ((reg & ~(P4R25_rg_eeeprg_rst)) | P4R25_rg_eeeprg_en));

	ram_code_done = 1;
}
#endif

void eee_phy_enable_by_port(int port)
{
	uint32 reg;

	// change to page 4
	rtl8651_setAsicEthernetPHYReg(port, 31, 4);

	// enable EEE N-way & set turn off power at quiet state
	rtl8651_getAsicEthernetPHYReg( port, 16, &reg );
	reg |= (P4R16_eee_nway_en | P4R16_tx_quiet_en | P4R16_rx_quiet_en);

#ifdef CONFIG_RTL8196C_ETH_IOT
	reg |= P4R16_eee_10_cap;	// enable 10M_EEE also.
#endif
	rtl8651_setAsicEthernetPHYReg( port, 16, reg );

	// enable EEE turn off DAC and line driver when TX Quiet state
	rtl8651_getAsicEthernetPHYReg( port, 25, &reg );
//	reg = reg & 0xF9FF | P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en;
	reg |= (P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en | P4R25_rg_eeeprg_en);

	rtl8651_setAsicEthernetPHYReg( port, 25, reg );

	rtl8651_setAsicEthernetPHYReg( port, 17, 0xa2a2 );
	rtl8651_setAsicEthernetPHYReg( port, 19, 0xc5c2 );
	rtl8651_setAsicEthernetPHYReg( port, 24, 0xc0f3 );

	if ((REG32(REVR)==RTL8196C_REVISION_A) && (port==4)){
		set_ram_code();
	}
	else if ((REG32(REVR) == RTL8196C_REVISION_B) && (port == 4)) {
		set_ram_code_b();
	}

	// switch to page 0
	rtl8651_setAsicEthernetPHYReg(port, 31, 0 );
}

#ifdef RTL8196C_EEE_REFINE
void eee_phy_enable(void)
{
	int i;

	// EEE PHY enable
	for (i=0; i<MAX_PORT_NUMBER; i++)
	{
		eee_phy_enable_by_port(i);
		rtl8651_restartAsicEthernetPHYNway(i+1);
	}
}
#else
void eee_phy_enable(void)
{
	int i;
	uint32 reg;

	// EEE PHY enable
	for (i=0; i<MAX_PORT_NUMBER; i++)
	{
		// change to page 4
		rtl8651_setAsicEthernetPHYReg(i, 31, 4);

		// enable EEE N-way & set turn off power at quiet state
		rtl8651_getAsicEthernetPHYReg( i, 16, &reg );
		reg |= (P4R16_eee_nway_en | P4R16_tx_quiet_en | P4R16_rx_quiet_en);
		rtl8651_setAsicEthernetPHYReg( i, 16, reg );

		// enable EEE turn off DAC and line driver when TX Quiet state
		rtl8651_getAsicEthernetPHYReg( i, 25, &reg );
//		reg = reg & 0xF9FF | P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en;
		reg |= (P4R25_rg_dacquiet_en | P4R25_rg_ldvquiet_en | P4R25_rg_eeeprg_en);
		rtl8651_setAsicEthernetPHYReg( i, 25, reg );

		rtl8651_setAsicEthernetPHYReg( i, 17, 0xa2a2 );
		rtl8651_setAsicEthernetPHYReg( i, 19, 0xc5c2 );
		rtl8651_setAsicEthernetPHYReg( i, 24, 0xc0f3 );

		if ((REG32(REVR)==RTL8196C_REVISION_A) && (i==4)){
			set_ram_code();
		}
		else if ((REG32(REVR) == RTL8196C_REVISION_B) && (i == 4)) {
			set_ram_code_b();
		}

		// switch to page 0
		rtl8651_setAsicEthernetPHYReg(i, 31, 0 );

		rtl8651_restartAsicEthernetPHYNway(i+1);
	}
}
#endif

void eee_phy_disable(void)
{
	int i;
	uint32 reg;

	// EEE PHY disable
	for (i=0; i<MAX_PORT_NUMBER; i++)
	{
		// change to page 4
		rtl8651_setAsicEthernetPHYReg(i, 31, 4);

		// disable (EEE N-way & turn off power at quiet state)
		rtl8651_getAsicEthernetPHYReg( i, 16, &reg );
		reg = reg & 0xECFF;
		rtl8651_setAsicEthernetPHYReg( i, 16, reg );

		// switch to page 0
		rtl8651_setAsicEthernetPHYReg(i, 31, 0 );

		rtl8651_restartAsicEthernetPHYNway(i+1);
	}

	// EEE MAC disable

}
#endif

#if defined(RTL8198_EEE_MAC)
// EEE PHY -- Page 4
void Set_GPHYWB(unsigned int phyid, unsigned int page, unsigned int reg, unsigned int mask, unsigned int val);

void eee_phy_enable_98(void)
{
	int i;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	// EEE PHY enable
	for (i=0; i<5; i++)
	{
		// change to page 4
		Set_GPHYWB(i,32,27,0xffff - 0xffff,0x2fce);
		Set_GPHYWB(i,32,21,0xffff - 0xffff,0x0100);
		Set_GPHYWB(i,5,5,0xffff - 0xffff,0x8b84);
		Set_GPHYWB(i,5,6,0xffff - 0xffff,0x0062);

		/* enable "EEE auto off" for JMicron's bug */
		Set_GPHYWB(i,5,5,0,0x857a);
		Set_GPHYWB(i,5,6,0,0x0770);

		rtl8651_restartAsicEthernetPHYNway(i+1);
	}

//	for(i=0; i<5; i++)
//		REG32(PCRP0+i*4) &= ~(EnForceMode);
}

void eee_phy_disable_98(void)
{
	int i;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	for (i=0; i<5; i++)
	{
		// change to page 4
		Set_GPHYWB(i,32,27,0xffff - 0xffff,0x2f4e);
		Set_GPHYWB(i,32,21,0xffff - 0xffff,0x0);
		Set_GPHYWB(i,5,5,0xffff - 0xffff,0x8b84);
		Set_GPHYWB(i,5,6,0xffff - 0xffff,0x0042);

		rtl8651_restartAsicEthernetPHYNway(i+1);
	}
	// EEE PHY disable

//	for(i=0; i<5; i++)
//		REG32(PCRP0+i*4) &= ~(EnForceMode);
}
#endif

#if defined(CONFIG_RTL_8198)
int rtl8198_power_saving_config(uint32 mode)
{
	unsigned long flags=0;
	int i, _8198_ALDPS, _8198_green_eth;

	if(mode == 0)
	{
		// 8198 green ethernet / EEE / ALDPS off
		eee_enabled = 0;
		_8198_green_eth = 0;
		_8198_ALDPS = 0;
	}
	else if(mode == 1)
	{
		// 8198 green ethernet / EEE / ALDPS on
		eee_enabled = 1;
		_8198_green_eth = 1;
		_8198_ALDPS = 1;
	}
	else if(mode == 2)
	{
		// 8198 green ethernet on / EEE off
		eee_enabled = 0;
		_8198_green_eth = 1;
		_8198_ALDPS = 1;
	}
	else if(mode == 3)
	{
		// 8198 green ethernet off / EEE on
		eee_enabled = 1;
		_8198_green_eth = 0;
		_8198_ALDPS = 1;
	}
	else {
		return (-1);
	}

	SMP_LOCK_ETH(flags);

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	/*
		8198 ALDPS feature is on by default.
		It is in PHY page 44, reg 21, bit 0: 1 enable, 0 disable
	*/
	if (_8198_ALDPS) {
		Set_GPHYWB(999, 44, 21, 0xfffe, 1);
	}
	else {
		Set_GPHYWB(999, 44, 21, 0xfffe, 0);
	}

	/*
		write Page 5 Reg 5 = 0x85E4
		read Page 5 Reg 6; #bit0 = 1, for enable green Rx
		write Page 5 Reg 5 = 0x85E7
		read Page 5 Reg 6; #bit0 = 1, for enable green Tx

	*/
	if (_8198_green_eth) {
		Set_GPHYWB(999, 5, 5, 0, 0x85e4);
		Set_GPHYWB(999, 5, 6, 0xfffe, 1);

		Set_GPHYWB(999, 5, 5, 0, 0x85e7);
		Set_GPHYWB(999, 5, 6, 0xfffe, 1);
	}
	else {
		Set_GPHYWB(999, 5, 5, 0, 0x85e4);
		Set_GPHYWB(999, 5, 6, 0xfffe, 0);

		Set_GPHYWB(999, 5, 5, 0, 0x85e7);
		Set_GPHYWB(999, 5, 6, 0xfffe, 0);
	}

	if (eee_enabled) {
		eee_phy_enable_98();
	}
	else {
		eee_phy_disable_98();
	}

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);

	SMP_UNLOCK_ETH(flags);

	return 0;
}
#endif

#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
void set_phy_pwr_save(int id, int val)
{
	uint32 reg_val;
	int i, start, end;

	if (id == 99)
		{ start=0; end=4; }
	else if (id <= 4)
		{ start = end = id; }
	else
		return;
	for(i=start; i<=end; i++)
	{
		rtl8651_getAsicEthernetPHYReg( i, 24, &reg_val);

		if (val == 1)
			rtl8651_setAsicEthernetPHYReg( i, 24, (reg_val | BIT(15)) );
		else
			rtl8651_setAsicEthernetPHYReg( i, 24, (reg_val & (~BIT(15))) );
	}
}
#endif

int32 mmd_read(uint32 phyId, uint32 devId, uint32 regId, uint32 *rData)
{
	rtl8651_setAsicEthernetPHYReg( phyId, 13, devId);
	rtl8651_setAsicEthernetPHYReg( phyId, 14, regId);
	rtl8651_setAsicEthernetPHYReg( phyId, 13, (devId | 0x4000));
	return (rtl8651_getAsicEthernetPHYReg(phyId, 14, rData));
}

int32 mmd_write(uint32 phyId, uint32 devId, uint32 regId, uint32 wData)
{
	rtl8651_setAsicEthernetPHYReg( phyId, 13, devId);
	rtl8651_setAsicEthernetPHYReg( phyId, 14, regId);
	rtl8651_setAsicEthernetPHYReg( phyId, 13, (devId | 0x4000));
	rtl8651_setAsicEthernetPHYReg( phyId, 14, wData);
	return SUCCESS;
}

uint32 rtl8651_filterDbIndex(ether_addr_t * macAddr,uint16 fid)
{
    return ( macAddr->octet[0] ^ macAddr->octet[1] ^
                    macAddr->octet[2] ^ macAddr->octet[3] ^
                    macAddr->octet[4] ^ macAddr->octet[5] ^fidHashTable[fid]) & 0xFF;
}

int32 rtl8651_setAsicL2Table(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *l2p)
{
	rtl865xc_tblAsic_l2Table_t entry;

	if((row >= RTL8651_L2TBL_ROW) || (column >= RTL8651_L2TBL_COLUMN) || (l2p == NULL))
		return FAILED;
	if(l2p->macAddr.octet[5] != ((row^(fidHashTable[l2p->fid])^ l2p->macAddr.octet[0] ^ l2p->macAddr.octet[1] ^ l2p->macAddr.octet[2] ^ l2p->macAddr.octet[3] ^ l2p->macAddr.octet[4] ) & 0xff))
		return FAILED;

	memset(&entry, 0,sizeof(entry));
	entry.mac47_40 = l2p->macAddr.octet[0];
	entry.mac39_24 = (l2p->macAddr.octet[1] << 8) | l2p->macAddr.octet[2];
	entry.mac23_8 = (l2p->macAddr.octet[3] << 8) | l2p->macAddr.octet[4];


	if( l2p->memberPortMask  > RTL8651_PHYSICALPORTMASK) //this MAC is on extension port
		entry.extMemberPort = (l2p->memberPortMask >>RTL8651_PORT_NUMBER);

	entry.memberPort = l2p->memberPortMask & RTL8651_PHYSICALPORTMASK;
	entry.toCPU = l2p->cpu==TRUE? 1: 0;
	entry.isStatic = l2p->isStatic==TRUE? 1: 0;
	entry.nxtHostFlag = l2p->nhFlag==TRUE? 1: 0;

	/* RTL865xC: modification of age from ( 2 -> 3 -> 1 -> 0 ) to ( 3 -> 2 -> 1 -> 0 ). modification of granularity 100 sec to 150 sec. */
	entry.agingTime = ( l2p->ageSec > 300 )? 0x03: ( l2p->ageSec <= 300 && l2p->ageSec > 150 )? 0x02: (l2p->ageSec <= 150 && l2p->ageSec > 0 )? 0x01: 0x00;

	entry.srcBlock = (l2p->srcBlk==TRUE)? 1: 0;
	entry.fid=l2p->fid;
	entry.auth=l2p->auth;
	return _rtl8651_forceAddAsicEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);
}

int32 rtl8651_delAsicL2Table(uint32 row, uint32 column)
{
	rtl865xc_tblAsic_l2Table_t entry;

	if(row >= RTL8651_L2TBL_ROW || column >= RTL8651_L2TBL_COLUMN)
		return FAILED;

	memset(&entry,0,sizeof(entry));
	return _rtl8651_forceAddAsicEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);
}


ether_addr_t cachedDA;
static uint32 cachedMbr;
unsigned int rtl8651_asicL2DAlookup(uint8 *dmac){
	uint32 column;
	rtl865xc_tblAsic_l2Table_t	entry;

	uint32 row = rtl8651_filterDbIndex((ether_addr_t *)dmac, 0);
	//rtlglue_printf("mac %02x %02x %02x %02x %02x %02x \n",	mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

	//cache miss...
	cachedMbr=0;
	for(column=0;column<RTL8651_L2TBL_COLUMN; column++) {
/* Should be fixed 		WRITE_MEM32(TEACR,READ_MEM32(TEACR)|0x1);ASIC patch: disable L2 Aging while reading L2 table */
		_rtl8651_readAsicEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);
/*		WRITE_MEM32(TEACR,READ_MEM32(TEACR)&~0x1); ASIC patch: enable L2 Aging aftrer reading L2 table */
		if(entry.agingTime == 0 && entry.isStatic == 0)
			continue;
		if(	dmac[0]==entry.mac47_40 &&
		    	dmac[1]==(entry.mac39_24>>8) &&
		    	dmac[2]==(entry.mac39_24 & 0xff)&&
		    	dmac[3]==(entry.mac23_8 >> 8)&&
		    	dmac[4]==(entry.mac23_8 & 0xff)&&
			dmac[5]== (row ^dmac[0]^dmac[1]^dmac[2]^dmac[3]^dmac[4])){

			cachedDA=*((ether_addr_t *)dmac);
			cachedMbr =(entry.extMemberPort<<RTL8651_PORT_NUMBER) | entry.memberPort;
			return cachedMbr;
		}
	}
	if(column==RTL8651_L2TBL_COLUMN)
		return 0xffffffff;//can't find this MAC, broadcast it.
	return cachedMbr;
}



int32 rtl8651_getAsicL2Table(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *l2p) {
	rtl865xc_tblAsic_l2Table_t   entry;

	if((row >= RTL8651_L2TBL_ROW) || (column >= RTL8651_L2TBL_COLUMN) || (l2p == NULL))
		return FAILED;

/*	RTL865XC should fix this problem.WRITE_MEM32(TEACR,READ_MEM32(TEACR)|0x1); ASIC patch: disable L2 Aging while reading L2 table */
	_rtl8651_readAsicEntry(TYPE_L2_SWITCH_TABLE, row<<2 | column, &entry);
	//WRITE_MEM32(TEACR,READ_MEM32(TEACR)&0x1); ASIC patch: enable L2 Aging aftrer reading L2 table */

	if(entry.agingTime == 0 && entry.isStatic == 0 )
		return FAILED;
	l2p->macAddr.octet[0] = entry.mac47_40;
	l2p->macAddr.octet[1] = entry.mac39_24 >> 8;
	l2p->macAddr.octet[2] = entry.mac39_24 & 0xff;
	l2p->macAddr.octet[3] = entry.mac23_8 >> 8;
	l2p->macAddr.octet[4] = entry.mac23_8 & 0xff;
	l2p->macAddr.octet[5] = row ^ l2p->macAddr.octet[0] ^ l2p->macAddr.octet[1] ^ l2p->macAddr.octet[2] ^ l2p->macAddr.octet[3] ^ l2p->macAddr.octet[4]  ^(fidHashTable[entry.fid]);
	l2p->cpu = entry.toCPU==1? TRUE: FALSE;
	l2p->srcBlk = entry.srcBlock==1? TRUE: FALSE;
	l2p->nhFlag = entry.nxtHostFlag==1? TRUE: FALSE;
	l2p->isStatic = entry.isStatic==1? TRUE: FALSE;
	l2p->memberPortMask = (entry.extMemberPort<<RTL8651_PORT_NUMBER) | entry.memberPort;

	/* RTL865xC: modification of age from ( 2 -> 3 -> 1 -> 0 ) to ( 3 -> 2 -> 1 -> 0 ). modification of granularity 100 sec to 150 sec. */
	l2p->ageSec = entry.agingTime * 150;

	l2p->fid=entry.fid;
	l2p->auth=entry.auth;
	return SUCCESS;
}

int32 rtl8651_setAsicPortMirror(uint32 mRxMask, uint32 mTxMask,uint32 mPortMask)
{
	uint32 pmcr = 0;
	pmcr = ((mTxMask<<MirrorTxPrtMsk_OFFSET)&MirrorTxPrtMsk_MASK) |
		((mRxMask << MirrorRxPrtMsk_OFFSET) & MirrorRxPrtMsk_MASK)|
		((mPortMask<<MirrorPortMsk_OFFSET) & MirrorPortMsk_MASK);

	WRITE_MEM32(PMCR,pmcr);

	return SUCCESS;
}

int32 rtl8651_getAsicPortMirror(uint32 *mRxMask, uint32 *mTxMask, uint32 *mPortMask)
{
	uint32 pmcr = READ_MEM32( PMCR );

	if ( mPortMask )
	{
		*mPortMask = ( pmcr & MirrorPortMsk_MASK ) >> MirrorPortMsk_OFFSET;
	}

	if ( mRxMask )
	{
		*mRxMask = ( pmcr & MirrorRxPrtMsk_MASK ) >> MirrorRxPrtMsk_OFFSET;
	}

	if ( mTxMask )
	{
		*mTxMask = ( pmcr & MirrorTxPrtMsk_MASK ) >> MirrorTxPrtMsk_OFFSET;
	}

	return SUCCESS;
}

int32 rtl8651_clearAsicL2Table(void)
{
	rtl8651_clearSpecifiedAsicTable(TYPE_L2_SWITCH_TABLE, RTL8651_L2TBL_ROW*RTL8651_L2TBL_COLUMN);
	rtl8651_clearSpecifiedAsicTable(TYPE_RATE_LIMIT_TABLE, RTL8651_RATELIMITTBL_SIZE);
	return SUCCESS;
}

inline int32 convert_setAsicL2Table(uint32 row, uint32 column, ether_addr_t * mac, int8 cpu,
		int8 srcBlk, uint32 mbr, uint32 ageSec, int8 isStatic, int8 nhFlag,int8 fid, int8 auth)
{
	rtl865x_tblAsicDrv_l2Param_t l2;

	memset(&l2,0,sizeof(rtl865x_tblAsicDrv_l2Param_t));

	l2.ageSec				= ageSec;
	l2.cpu				= cpu;
	l2.isStatic				= isStatic;
	l2.memberPortMask		= mbr;
	l2.nhFlag				= nhFlag;
	l2.srcBlk				= srcBlk;
//#ifdef RTL865XC_LAN_PORT_NUM_RESTRIT
//	if(enable4LanPortNumRestrict == TRUE)
	l2.fid=fid;
	l2.auth = auth;
//#endif
	memcpy(&l2.macAddr, mac, 6);
	return rtl8651_setAsicL2Table(row, column, &l2);
}

/*
 * RTL8651 L2 entry:
 *		For each L2 entry added by driver table as a static entry, the aging time
 *		will not be updated by ASIC.
 * Action:
 *		set the entry is a dynamic entry and turn on the 'nhFlag',
 *		then the aging time of this entry will be updated and once aging time expired,
 *		it won't be removed by ASIC automatically.
 */
int32 rtl8651_setAsicL2Table_Patch(uint32 row, uint32 column, ether_addr_t * mac, int8 cpu,
		int8 srcBlk, uint32 mbr, uint32 ageSec, int8 isStatic, int8 nhFlag, int8 fid,int8 auth)
{
#if 0
	ether_addr_t bcast_mac = { {0xff, 0xff, 0xff, 0xff, 0xff, 0xff} };
	ether_addr_t cpu_mac = {{0x00,0x00,0x0a,0x00,0x00,0x0f}};

	/*
		In RTL865xC, we need to turn on the CPU bit of broadcast mac to let broadcast packets being trapped to CPU.
	*/

	if ( memcmp( &bcast_mac, mac, sizeof(ether_addr_t) ) == 0 || memcmp(&cpu_mac,mac,sizeof(ether_addr_t)) == 0)
	{
		return convert_setAsicL2Table(
				row,
				column,
				mac,
				TRUE,	/* Set CPU bit to TRUE */
				FALSE,
				mbr,
				500,
				isStatic,	/* No one will be broadcast/multicast source */
				nhFlag,/* No one will be broadcast/multicast source */
				TRUE
		);
	}
#endif
	if(mac->octet[0]&0x1)
	{
		return convert_setAsicL2Table(
				row,
				column,
				mac,
				cpu,
				FALSE,
				mbr,
				ageSec,
				isStatic,	/* No one will be broadcast/multicast source */
				nhFlag,	/* No one will be broadcast/multicast source */
				fid,
				TRUE
		);
	}
	else {
		int8 dStatic=isStatic/*, dnhFlag=(isStatic==TRUE? TRUE: FALSE)*/;
		int8 dnhFlag = nhFlag;
#if defined(CONFIG_RTL865X_PPTPL2TP)||defined(CONFIG_RTL865XB_PPTPL2TP)
		extern rtl8651_tblDrv_miiTunneling_t tunnel;
		uint32 MBR = (1 << tunnel.loopbackPort);
		if (tunnel.valid && mbr==MBR) {
			dStatic = TRUE;
			dnhFlag = FALSE;
		}
#endif
		return convert_setAsicL2Table(
				row,
				column,
				mac,
				cpu,
				srcBlk,
				mbr,
				ageSec,
				dStatic,
				dnhFlag,
				fid,
				auth
//				FALSE,							/* patch here!! always dynamic entry */
//				(isStatic==TRUE? TRUE: FALSE)	/* patch here!! nhFlag always turned on if static entry*/
		);
	}
}

#ifdef CONFIG_RTL_HW_TX_CSUM
int getL2MbrByMac(unsigned char *mac)
{
	int idx;
	rtl865x_tblAsicDrv_l2Param_t l2_t;

	idx = (((fidHashTable[0])^ mac[0] ^ mac[1] ^ mac[2] ^ mac[3] ^ mac[4]  ^ mac[5]) & 0xff);

	if (rtl8651_getAsicL2Table_Patch(idx, 0, &l2_t) == SUCCESS)
		return (l2_t.memberPortMask);
	else
		return (-1);
}
#endif

#if 0
int32 rtl8651_getAsicL2Table_Patch(uint32 row, uint32 column, ether_addr_t * mac, int8 * cpu,
	int8 * srcBlk, int8 * isStatic, uint32 * mbr, uint32 * ageSec, int8 *nhFlag)
{
	rtl865x_tblAsicDrv_l2Param_t l2;

	int32 retval = rtl8651_getAsicL2Table(row, column, &l2);
	if (mac) memcpy(mac, &l2.macAddr, 6);
	if (cpu) *cpu = l2.cpu;
	if (srcBlk) *srcBlk = l2.srcBlk;
	if (isStatic) *isStatic = l2.isStatic;
	if (mbr) *mbr = l2.memberPortMask;
	if (ageSec) *ageSec = l2.ageSec;
	if (nhFlag) *nhFlag = l2.nhFlag;
	if (isStatic != NULL) *isStatic = TRUE; /* patch!!, always TRUE(static entry */
	if (nhFlag != NULL) *nhFlag = FALSE;  /* always false */
	return retval;
}
#else

int32 rtl8651_getAsicL2Table_Patch(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *asic_l2_t)
{
	int32 retval = rtl8651_getAsicL2Table(row, column, asic_l2_t);
#ifdef CONFIG_RTL865XB_EXP_INVALID
	if (retval == SUCCESS) {
		asic_l2_t->isStatic	= TRUE;
		asic_l2_t->nhFlag	= FALSE;
	}
#endif
	return retval;
}
#endif

#if !defined(CONFIG_RTL_819X)
#define	PAGE_SELECT_REGID		31
#define	PAGE_SELECT_OFFSET		0
#define	PAGE_SELECT_MASK		0xF
#define		EXTRTL_8214_REGBASE_1		CONFIG_EXTRTL8212_PHYID_P1
#define		EXTRTL_8214_REGBASE_3		CONFIG_EXTRTL8212_PHYID_P3
static inline unsigned int rtl865x_probeP1toP4GigaPHYChip(void)
{
	unsigned int uid,tmp;
	unsigned int i;

	/* Read */
	for(i=0; i<4; i++)  //probe p1-p4
	{
		rtl8651_getAsicEthernetPHYReg( CONFIG_EXTRTL8212_PHYID_P1+i, 2, &tmp );
		uid=tmp<<16;
		rtl8651_getAsicEthernetPHYReg( CONFIG_EXTRTL8212_PHYID_P1+i, 3, &tmp );
		uid=uid | tmp;

		if( uid==0x001CC912 )  //0x001cc912 is 8212 two giga port , 0x001cc940 is 8214 four giga port
		{
			return 1;
		}
		else if(uid==0x001CC940)
		{
			//printk("Find Port1-4 8214 PHY Chip! \r\n");
			//FixPHYChip();
			//RstGigaPhy();
			return 1;
		}
	}
	return 0;
}

static inline unsigned int rtl865x_probeP5GigaPHYChip(void)
{
	unsigned int uid,tmp;

	/* Read */
	rtl8651_getAsicEthernetPHYReg( CONFIG_EXTRTL8212_PHYID_P5, 0, &tmp );
	rtl8651_setAsicEthernetPHYReg(CONFIG_EXTRTL8212_PHYID_P5,0x10,0x01FE);

	/* Read */
	rtl8651_getAsicEthernetPHYReg( CONFIG_EXTRTL8212_PHYID_P5, 2, &tmp );
	uid=tmp<<16;
	rtl8651_getAsicEthernetPHYReg( CONFIG_EXTRTL8212_PHYID_P5, 3, &tmp );
	uid=uid | tmp;

	if( uid==0x001CC912 )  //0x001cc912 is 8212 two giga port , 0x001cc940 is 8214 four giga port
	{	//printk("Find Port5   have 8211 PHY Chip! \r\n");
		return 1;
	}

	return 0;
}

static void	rtl865x_fix8214Bug(void)
{
	/*	52_phy_write 0x12 9 21 0xDD0A	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, PAGE_SELECT_REGID, 0x0009);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 21, 0xDD0A);
	/*	52_phy_write 0x13 9 21 0xDD0A	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, PAGE_SELECT_REGID, 0x0009);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 21, 0xDD0A);
	/*	52_phy_write 0x14 8 28 0x0003	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, PAGE_SELECT_REGID, 0x0008);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 28, 0x0003);

	/*	mdcmdio_cmd w 0x12 31  0x0002
	*	mdcmdio_cmd w 0x12 8  0x3672
	*	mdcmdio_cmd w 0x12 9  0x8c00
	*	mdcmdio_cmd w 0x12 12  0x5b15
	*	mdcmdio_cmd w 0x12 18  0x0edd
	*	mdcmdio_cmd w 0x12 27  0x5c5c
	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, PAGE_SELECT_REGID, 0x0002);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 8, 0x3672);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 9, 0x8c00);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 12, 0x5b15);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 18, 0x0edd);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 27, 0x5c5c);

	/*	mdcmdio_cmd w 0x13 31  0x0002
	*	mdcmdio_cmd w 0x13 8  0x3672
	*	mdcmdio_cmd w 0x13 9  0x8c00
	*	mdcmdio_cmd w 0x13 12  0x5b15
	*	mdcmdio_cmd w 0x13 18  0x0edd
	*	mdcmdio_cmd w 0x13 27  0x5c5c
	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, PAGE_SELECT_REGID, 0x0002);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 8, 0x3672);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 9, 0x8c00);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 12, 0x5b15);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 18, 0x0edd);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 27, 0x5c5c);

	/*	mdcmdio_cmd w 0x14 31  0x0002
	*	mdcmdio_cmd w 0x14 8  0x3672
	*	mdcmdio_cmd w 0x14 9  0x8c00
	*	mdcmdio_cmd w 0x14 12  0x5b15
	*	mdcmdio_cmd w 0x14 18  0x0edd
	*	mdcmdio_cmd w 0x14 27  0x5c5c
	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, PAGE_SELECT_REGID, 0x0002);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 8, 0x3672);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 9, 0x8c00);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 12, 0x5b15);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 18, 0x0edd);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 27, 0x5c5c);
	/*	mdcmdio_cmd w 0x15 31  0x0002
	*	mdcmdio_cmd w 0x15 8  0x3672
	*	mdcmdio_cmd w 0x15 9  0x8c00
	*	mdcmdio_cmd w 0x15 12  0x5b15
	*	mdcmdio_cmd w 0x15 18  0x0edd
	*	mdcmdio_cmd w 0x15 27  0x5c5c
	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, PAGE_SELECT_REGID, 0x0002);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, 8, 0x3672);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, 9, 0x8c00);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, 12, 0x5b15);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, 18, 0x0edd);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, 27, 0x5c5c);

	/*
	*	#RTL8214 Enable AUTO - K
	*	52_phy_write 0x12 0 20 0x8000
	*	52_phy_write 0x13 0 20 0x8000
	*	52_phy_write 0x14 0 20 0x8000
	*	52_phy_write 0x15 0 20 0x8000
	*/

	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 20, 0x8000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 20, 0x8000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 20, 0x8000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, 20, 0x8000);

	/*
	*	52_phy_write 0x12 0 20 0x8040
	*	52_phy_write 0x13 0 20 0x8040
	*	52_phy_write 0x14 0 20 0x8040
	*	52_phy_write 0x15 0 20 0x8040
	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, 20, 0x8040);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 20, 0x8040);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, 20, 0x8040);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, PAGE_SELECT_REGID, 0x0000);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, 20, 0x8040);
	/*
		#change to default page
		52_phy_read 0x12 8 0
	*/
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1, PAGE_SELECT_REGID, 0x0008);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, PAGE_SELECT_REGID, 0x0008);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3, PAGE_SELECT_REGID, 0x0008);
	rtl8651_setAsicEthernetPHYReg(EXTRTL_8214_REGBASE_3+1, PAGE_SELECT_REGID, 0x0008);

#if 0
	/* Test */
	rtl8651_getAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, PAGE_SELECT_REGID, &rData);
	rtl8651_getAsicEthernetPHYReg(EXTRTL_8214_REGBASE_1+1, 15, &rData);
	rtl865x_setAsicEthernetPHYPage(EXTRTL_8214_REGBASE_1+1, 8);
	printk("*********************\nrData 0x%x\n*********************\n", rData);
#endif
}
#endif

static int32 _rtl8651_initAsicPara( rtl8651_tblAsic_InitPara_t *para )
{
	memset(&rtl8651_tblAsicDrvPara, 0, sizeof(rtl8651_tblAsic_InitPara_t));

	if ( para )
	{
		/* Parameter != NULL, check its correctness */
		if (para->externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT1234_RTL8212)
		{
			ASICDRV_ASSERT(para->externalPHYId[1] != 0);
			ASICDRV_ASSERT(para->externalPHYId[2] != 0);
			ASICDRV_ASSERT(para->externalPHYId[3] != 0);
			ASICDRV_ASSERT(para->externalPHYId[4] != 0);
			ASICDRV_ASSERT(para->externalPHYId[2] == (para->externalPHYId[1] + 1));
			ASICDRV_ASSERT(para->externalPHYId[4] == (para->externalPHYId[3] + 1));
		}
		if (para->externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
		{
			ASICDRV_ASSERT(para->externalPHYId[5] != 0);
		}

		/* ============= Check passed : set it =============  */
		memcpy(&rtl8651_tblAsicDrvPara, para, sizeof(rtl8651_tblAsic_InitPara_t));
	}

	return SUCCESS;
}

void Set_GPHYWB(unsigned int phyid, unsigned int page, unsigned int reg, unsigned int mask, unsigned int val)
{
	unsigned int data=0;
	unsigned int i, wphyid=0;	//start
	unsigned int wphyid_end=1;   //end

	if(phyid==999)
	{	i=0;
		wphyid_end=5;    //total phyid=0~4
	}
	else
	{	i=phyid;
		wphyid_end=phyid+1;
	}

	for(; i<wphyid_end; i++)
	{
		wphyid = i;
		//change page
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		if (i == 0)
			wphyid = RTL8198C_PORT0_PHY_ID;
		
		if(page > 0)
			rtl8651_setAsicEthernetPHYReg( wphyid, 31, page  );
#else
		if(page>=31)
		{	rtl8651_setAsicEthernetPHYReg( wphyid, 31, 7  );
			rtl8651_setAsicEthernetPHYReg( wphyid, 30, page  );
		}
		else
		{
			rtl8651_setAsicEthernetPHYReg( wphyid, 31, page  );
		}
#endif
		if(mask != 0)
		{
			rtl8651_getAsicEthernetPHYReg( wphyid, reg, &data);
			data = data&mask;
		}
		rtl8651_setAsicEthernetPHYReg( wphyid, reg, data|val  );


		//switch to page 0
		rtl8651_setAsicEthernetPHYReg( wphyid, 31, 0  );
	}
}

#ifdef CONFIG_RTL8196C_REVISION_B

#ifdef CONFIG_RTL8196C_ETH_IOT
void set_gray_code_by_port(int port)
{
        uint32 val;
 
        rtl8651_setAsicEthernetPHYReg( 4, 31, 1  );
        rtl8651_getAsicEthernetPHYReg( 4, 20, &val  );
        rtl8651_setAsicEthernetPHYReg( 4, 20, val + (0x1 << port)  );
        rtl8651_setAsicEthernetPHYReg( 4, 31, 0  );
 
        rtl8651_setAsicEthernetPHYReg( port, 31, 1  );
        rtl8651_setAsicEthernetPHYReg( port, 19,  0x5400 );
        if (port<4) rtl8651_setAsicEthernetPHYReg( port, 19,  0x5440 );
        if (port<3) rtl8651_setAsicEthernetPHYReg( port, 19,  0x54c0 );
        if (port<2) rtl8651_setAsicEthernetPHYReg( port, 19,  0x5480 );
        if (port<1) rtl8651_setAsicEthernetPHYReg( port, 19,  0x5580 );
        rtl8651_setAsicEthernetPHYReg( port, 31, 0  );

        rtl8651_setAsicEthernetPHYReg( 4, 31, 1  );
        rtl8651_setAsicEthernetPHYReg( 4, 20, 0xb20  );
        rtl8651_setAsicEthernetPHYReg( 4, 31, 0  );
}
#endif

void Setting_RTL8196C_PHY(void)
{
	int i, j;
	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	/*
	  #=========ADC Bias Current =========================
	  #RG1X_P4~0 [12:10] = Reg_pi_fix [2:0], 5 ->7
	  phywb all 1 17 12-10 0x7
	*/
	Set_GPHYWB(999, 1, 17, 0xffff-(7<<10), 0x7<<10);

	/*
	  #=========patch for eee============================
	  #1. page4¡Breg24¡Glpi_rx_ti_timer_cnt change to f3
	  phywb all 4 24 7-0 0xf3

	  #2. page4¡Breg16¡Grg_txqt_ps_sel change to 1
	  phywb all 4 16 3 1
	*/
	Set_GPHYWB(999, 4, 24, 0xff00, 0xf3);
	Set_GPHYWB(999, 4, 16, 0xffff-(1<<3), 1<<3);
	/*
	  #=========patch for IOL Tx amp.=====================
	  #<a>modify 100M DAC current default value:
	  #Port#0~#4(per port control)
	  #Page1,Reg19,bit[13:11]:
	  #original value 200uA(3b'100),--> change to 205u(3b'000)   => change to 205u(3b010)

	  phywb all 1 19 13-11 0x2

	  #<b>modify bandgap voltage default value:
	  #Port#0~#4 (Global all ports contorl setting),
	  #Page1,Reg23,bit[8:6],

	  #original value 1.312V(3b'110),-->change to 1.212V(3b'100).

	  phywb all 1 23 8-6 0x4

	  #<c>modify TX CS cap default value:
	  #Port#0~#4 (Global all ports contorl setting),
	  #Page1,Reg18,bit[5:3],

	  #original value Reserved bits(3b'000),-->change to 600fF(3b'011). =>change to 750fF(3b'110)
	  phywb all 1 18 5-3 0x6
	*/

	Set_GPHYWB(999, 1, 19, 0xffff-(7<<11), 0x2<<11);
	Set_GPHYWB(999, 1, 23, 0xffff-(7<<6)  , 0x4<<6);
	Set_GPHYWB(999, 1, 18, 0xffff-(7<<3), 0x6<<3);


	/* 20100223 from Maxod: 100M half duplex enhancement */
 	REG32(MACCR)= (REG32(MACCR) & ~CF_RXIPG_MASK) | 0x05;

	/* fix the link down / link up issue with SmartBit 3101B when DUT(8196c) set to Auto-negotiation
	    and SmartBit force to 100M Full-duplex */
 	REG32(MACCR)= (REG32(MACCR) & ~SELIPG_MASK) | SELIPG_11;

	/*20100222 from Anson:Switch Corner test pass setting*/
	 /*
		REG21 default=0x2c5
		After snr_ub(page0 reg21.7-4) = 3 and snr_lb(page0 reg21.3-0)=2 ,REG21=0x232
		REG22 default=0x5b85
		After adtune_lb(page0 reg22.6-4)=4 (10uA) ,REG21=0x5b45

		REG0 default=0x1100
		restart AN
		page0 reg0.9 =1 , ,REG0=0x1300
	*/
	//rtl8651_setAsicEthernetPHYReg( i, 0x15, 0x232 );
	//Set_GPHYWB(999, 0, 21, 0xffff-(0xff<<0), 0x32<<0);
	// test 96C to 96C restart AN 100 times, result is pass ==> page0 reg21.14(disable the equlizar)=1
#ifdef CONFIG_RTL8196C_ETH_IOT
	// enable "equalizer reset", i.e. page 0 , reg21, bit14= 0
	Set_GPHYWB(999, 0, 21, (~0x40ff), 0x0032);
#else
	Set_GPHYWB(999, 0, 21, (~0x40ff), 0x4032);
#endif

	//rtl8651_setAsicEthernetPHYReg( i, 0x16, 0x5b45 );
	//Set_GPHYWB(999, 0, 22, 0xffff-(7<<4), 0x4<<4);
	Set_GPHYWB(999, 0, 22, 0xffff-(7<<4), 0x5<<4);
	//rtl8651_setAsicEthernetPHYReg( i, 0x0, 0x1300 );
	Set_GPHYWB(999, 0, 0, 0xffff-(1<<9), 0x1<<9);

	/*20100225 from Anson:Switch Force cailibration
	#change calibration update method for patch first pkt no update impedance
	phywb all 1 29 1 0
	#--------------Patch for impedance update fail cause rx crc error with long calbe--------
	#Froce cailibration
	phywb all 1 29 2 1
	#Force impedance value = 0x8888
	phywb all 1 28 15-0 0x8888
	#-----------------------------------------------------------------------------------------
	#Select clock (ckt125[4]) edge trigger mlt3[1:0] = negative for patch four corner fail issue(only tx timing)
	phywb all 1 17 2-1 0x3
	*/
	//Set_GPHYWB(999, 1, 29, 0xffff-(1<<1), 0x0<<1);
	//Set_GPHYWB(999, 1, 29, 0xffff-(1<<2), 0x1<<2);
	//Set_GPHYWB(999, 1, 28, 0xffff-(0xffff), 0x8888);
	Set_GPHYWB(999, 1, 17, 0xffff-(3<<1), 0x3<<1);

	/*20100222 from Yozen:AOI TEST pass setting*/
	Set_GPHYWB(999, 1, 18, 0xffff-(0xffff), 0x9004);

	// for "DSP recovery fail when link partner = force 100F"
	Set_GPHYWB(999, 4, 26, 0xffff-(0xfff<<4), 0xff8<<4);

	// fix unlink IOT issue
	Set_GPHYWB(999, 0, 26, 0xffff-(0x1<<14), 0x0<<14);
    Set_GPHYWB(999, 0, 17, 0xffff-(0xf<<8), 0xe<<8);

#ifdef CONFIG_RTL8196C_ETH_IOT
        for(j=0; j<5; j++) {
                set_gray_code_by_port(j);
        }
#endif

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);

	printk("  Set 8196C PHY Patch OK\n");

}
#endif

#ifdef CONFIG_RTL_8198
static const unsigned int phy_para[]={
	//###################### PHY parameter patch ################################
	0x1f, 0x0005, //Page 5
	0x13, 0x0003, //Page 5 ########## EMA =3#############
	//0x01, 0x0700; #Page 5 Reg 1 = 0x0700, NEC ON	(20100112)
	0x05,0x8B82,//Page 5 Reg 5 = 0x8B82, Fix 100M re-link fail issue (20100110)
	0x06,0x05CB,//Page 5 Reg 6 = 0x05CB, Fix 100M re-link fail issue (20100110)
	0x1f,0x0002,//Page 2
	0x04,0x80C2,//Page 2 Reg 4 0x80C2, Fix 100M re-link fail issue (20100110)
	0x05,0x0938,//Page 2 Reg 5 0x0938, Disable 10M standby mode (20100112)

	0x1F,0x0003,//Page 3
	0x12,0xC4D2,//Page 3 Reg 18 = 0xC4D2, GAIN upper bond=24
	0x0D,0x0207,//Page 3 Reg 13 = 0x0207 (20100112)
	0x01,0x3554, //#Page 3 Reg  1 = 0x3554 (20100423)
	0x02,0x63E8, //#Page 3 Reg  2 = 0x63E8 (20100423)
	0x03,0x99C2, //#Page 3 Reg  3 = 0x99C2 (20100423)
	0x04,0x0113, //#Page 3 Reg  4 = 0x0113 (20100423)

	0x1f,0x0001,//Page 1
	0x07,0x267E,//Page 1 Reg  7 = 0x267E, Channel Gain offset (20100111)
	0x1C,0xE5F7,//Page 1 Reg 28 = 0xE5F7, Cable length offset (20100111)
	0x1B,0x0424,//Page 1 Reg 27 = 0x0424, SD threshold (20100111)

	//#Add by Gary for Channel Estimation fine tune 20100430
	//0x1f,0x0002, //# change to Page 1 (Global)
	//0x08,0x0574, //# Page1 Reg8 (CG_INITIAL_MASTER)
	//0x09,0x2724, //# Page1 Reg9 (CB0_INITIAL_GIGA)
	//0x1f,0x0003, //# change to Page 3 (Global)
	//0x1a,0x06f6, //# Page3 Reg26 (CG_INITIAL_SLAVE)

	//#Add by Gary for Channel Estimation fine tune 20100430
	//#Page1 Reg8 (CG_INITIAL_MASTER)
	//0x1f, 0x0005,
	//0x05, 0x83dd,
	//0x06, 0x0574,
	//#Page1 Reg9 (CB0_INITIAL_GIGA)
	//0x1f, 0x0005,
	//0x05, 0x83e0,
	//0x06, 0x2724,
	//#Page3 Reg26 (CG_INITIAL_SLAVE)
	//0x1f, 0x0005,
	//0x05, 0x843d,
	//0x06, 0x06f6 ,

	//#NC FIFO
	0x1f,0x0007,//ExtPage
	0x1e,0x0042,//ExtPage 66
	0x18,0x0000,//Page 66 Reg 24 = 0x0000, NC FIFO (20100111)
	0x1e,0x002D,//ExtPage 45
	0x18,0xF010,//Page 45 Reg 24 = 0xF010, Enable Giga Down Shift to 100M (20100118)

	0x1e,0x002c, //#ExtPage 44
	0x18,0x008B, //#Page 44 Reg 24 = 0x008B, Enable deglitch circuit (20100426)

	//############################ EEE giga patch ################################

	//0x1f 0x0007;
	0x1e,0x0028,
	0x16,0xf640,//phywb $phyID 40 22 15-0 0xF640

	0x1e,0x0021,
	0x19,0x2929,//phywb $phyID 33 25 15-0 0x2929

	0x1a,0x1005,//phywb $phyID 33 26 15-0 0x1005

	0x1e,0x0020,
	0x17,0x000a,//phywb $phyID 32 23 15-0 0x000a

	0x1b,0x2f4a,//Disable EEE PHY mode
	0x15,0x0100,//EEE ability, Disable EEEP

	0x1e,0x0040,//
	0x1a,0x5110,//	phywb $phyID 64 26 15-0 0x5110
	0x18,0x0000,// programable mode

	0x1e,0x0041,//
	0x15,0x0e02,//phywb $phyID 65 21 15-0 0x0e02

	0x16,0x2185,//phywb $phyID 65 22 15-0 0x2185
	0x17,0x000c,//phywb $phyID 65 23 15-0 0x000c
	0x1c,0x0008,//phywb $phyID 65 28 15-0 0x0008
	0x1e,0x0042,//
	0x15,0x0d00,//phywb $phyID 66 21 15-0 0x0d00

	#if 1
	//############################ EEE Run code patch #################################
	//###proc 67R_ram_code_20100211_inrx_uc_98_1
	//###proc ram_code_0223_uc {} {
	//#replace 0xfff6, lock uc ram code version!
	31, 0x5,
	5,  0xfff6,
	6,  0x0080,
	5,  0x8b6e,
	6,  0x0000,
	15, 0x0100,

	//### force MDI/MDIX
	0x1f, 0x0007,
	0x1e, 0x002d,
	0x18, 0xf030,

	//### pcs nctl patch code (0423)
	0x1f, 0x0007,
	0x1e, 0x0023,
	0x16, 0x0005,

	//### startpoint
	 0x15, 0x005c,
	0x19, 0x0068,
	0x15, 0x0082,
	0x19, 0x000a,
	0x15, 0x00a1,
	0x19, 0x0081,
	0x15 ,0x00af,
	0x19, 0x0080,
	0x15, 0x00d4,
	0x19, 0x0000,
	0x15, 0x00e4,
	0x19, 0x0081,
	0x15, 0x00e7,
	0x19, 0x0080,
	0x15, 0x010d,
	0x19, 0x0083,
	0x15, 0x0118,
	0x19, 0x0083,
	0x15, 0x0120,
	0x19, 0x0082,
	0x15, 0x019c,
	0x19, 0x0081,
	0x15, 0x01a4,
	0x19, 0x0080,
	0x15, 0x01cd,
	0x19, 0x0000,
	0x15, 0x01dd,
	0x19, 0x0081,
	0x15, 0x01e0,
	0x19, 0x0080,
	//### endpoint

	0x16, 0x0000,
	//### end of pcs nctl patch code

	//inrx
	0x1f, 0x0007,
	0x1e, 0x0040,
	0x18, 0x0004,
	0x1f,0x0000,
	0x17,0x2160,
	0x1f,0x0007,
	0x1e,0x0040,

	//### startpoint
	0x18,0x0004,
	0x19,0x4000,
	0x18,0x0014,
	0x19,0x7f00,
	0x18,0x0024,
	0x19,0x0000,
	0x18,0x0034,
	0x19,0x0100,
	0x18,0x0044,
	0x19,0xe000,
	0x18,0x0054,
	0x19,0x0000,
	0x18,0x0064,
	0x19,0x0000,
	0x18,0x0074,
	0x19,0x0000,
	0x18,0x0084,
	0x19,0x0400,
	0x18,0x0094,
	0x19,0x8000,
	0x18,0x00a4,
	0x19,0x7f00,
	0x18,0x00b4,
	0x19,0x4000,
	0x18,0x00c4,
	0x19,0x2000,
	0x18,0x00d4,
	0x19,0x0100,
	0x18,0x00e4,
	0x19,0x8400,
	0x18,0x00f4,
	0x19,0x7a00,
	0x18,0x0104,
	0x19,0x4000,
	0x18,0x0114,
	0x19,0x3f00,
	0x18,0x0124,
	0x19,0x0100,
	0x18,0x0134,
	0x19,0x7800,
	0x18,0x0144,
	0x19,0x0000,
	0x18,0x0154,
	0x19,0x0000,
	0x18,0x0164,
	0x19,0x0000,
	0x18,0x0174,
	0x19,0x0400,
	0x18,0x0184,
	0x19,0x8000,
	0x18,0x0194,
	0x19,0x7f00,
	0x18,0x01a4,
	0x19,0x8300,
	0x18,0x01b4,
	0x19,0x8300,
	0x18,0x01c4,
	0x19,0xe200,
	0x18,0x01d4,
	0x19,0x0a00,
	0x18,0x01e4,
	0x19,0x8800,
	0x18,0x01f4,
	0x19,0x0300,
	0x18,0x0204,
	0x19,0xe100,
	0x18,0x0214,
	0x19,0x4600,
	0x18,0x0224,
	0x19,0x4000,
	0x18,0x0234,
	0x19,0x7f00,
	0x18,0x0244,
	0x19,0x0000,
	0x18,0x0254,
	0x19,0x0100,
	0x18,0x0264,
	0x19,0x4000,
	0x18,0x0274,
	0x19,0x3e00,
	0x18,0x0284,
	0x19,0x0000,
	0x18,0x0294,
	0x19,0xe000,
	0x18,0x02a4,
	0x19,0x1200,
	0x18,0x02b4,
	0x19,0x8000,
	0x18,0x02c4,
	0x19,0x7f00,
	0x18,0x02d4,
	0x19,0x8900,
	0x18,0x02e4,
	0x19,0x8300,
	0x18,0x02f4,
	0x19,0xe000,
	0x18,0x0304,
	0x19,0x0000,
	0x18,0x0314,
	0x19,0x4000,
	0x18,0x0324,
	0x19,0x7f00,
	0x18,0x0334,
	0x19,0x0000,
	0x18,0x0344,
	0x19,0x2000,
	0x18,0x0354,
	0x19,0x4000,
	0x18,0x0364,
	0x19,0x3e00,
	0x18,0x0374,
	0x19,0xfd00,
	0x18,0x0384,
	0x19,0x0000,
	0x18,0x0394,
	0x19,0x1200,
	0x18,0x03a4,
	0x19,0xab00,
	0x18,0x03b4,
	0x19,0x0c00,
	0x18,0x03c4,
	0x19,0x0600,
	0x18,0x03d4,
	0x19,0xa000,
	0x18,0x03e4,
	0x19,0x3d00,
	0x18,0x03f4,
	0x19,0xfb00,
	0x18,0x0404,
	0x19,0xe000,
	0x18,0x0414,
	0x19,0x0000,
	0x18,0x0424,
	0x19,0x4000,
	0x18,0x0434,
	0x19,0x7f00,
	0x18,0x0444,
	0x19,0x0000,
	0x18,0x0454,
	0x19,0x0100,
	0x18,0x0464,
	0x19,0x4000,
	0x18,0x0474,
	0x19,0xc600,
	0x18,0x0484,
	0x19,0xff00,
	0x18,0x0494,
	0x19,0x0000,
	0x18,0x04a4,
	0x19,0x1000,
	0x18,0x04b4,
	0x19,0x0200,
	0x18,0x04c4,
	0x19,0x7f00,
	0x18,0x04d4,
	0x19,0x4000,
	0x18,0x04e4,
	0x19,0x7f00,
	0x18,0x04f4,
	0x19,0x0200,
	0x18,0x0504,
	0x19,0x0200,
	0x18,0x0514,
	0x19,0x5200,
	0x18,0x0524,
	0x19,0xc400,
	0x18,0x0534,
	0x19,0x7400,
	0x18,0x0544,
	0x19,0x0000,
	0x18,0x0554,
	0x19,0x1000,
	0x18,0x0564,
	0x19,0xbc00,
	0x18,0x0574,
	0x19,0x0600,
	0x18,0x0584,
	0x19,0xfe00,
	0x18,0x0594,
	0x19,0x4000,
	0x18,0x05a4,
	0x19,0x7f00,
	0x18,0x05b4,
	0x19,0x0000,
	0x18,0x05c4,
	0x19,0x0a00,
	0x18,0x05d4,
	0x19,0x5200,
	0x18,0x05e4,
	0x19,0xe400,
	0x18,0x05f4,
	0x19,0x3c00,
	0x18,0x0604,
	0x19,0x0000,
	0x18,0x0614,
	0x19,0x1000,
	0x18,0x0624,
	0x19,0x8a00,
	0x18,0x0634,
	0x19,0x7f00,
	0x18,0x0644,
	0x19,0x4000,
	0x18,0x0654,
	0x19,0x7f00,
	0x18,0x0664,
	0x19,0x0100,
	0x18,0x0674,
	0x19,0x2000,
	0x18,0x0684,
	0x19,0x0000,
	0x18,0x0694,
	//### 0x2219:0x4600 =>0xe600 #0309
	0x19,0xe600,
	0x18,0x06a4,
	//### 0x2219:0xfc00 =>0xff00 #0309
	0x19,0xff00,
	0x18,0x06b4,
	0x19,0x0000,
	0x18,0x06c4,
	0x19,0x5000,
	0x18,0x06d4,
	0x19,0x9d00,
	0x18,0x06e4,
	0x19,0xff00,
	0x18,0x06f4,
	0x19,0x4000,
	0x18,0x0704,
	0x19,0x7f00,
	0x18,0x0714,
	0x19,0x0000,
	0x18,0x0724,
	0x19,0x2000,
	0x18,0x0734,
	0x19,0x0000,
	0x18,0x0744,
	0x19,0xe600,
	0x18,0x0754,
	0x19,0xff00,
	0x18,0x0764,
	0x19,0x0000,
	0x18,0x0774,
	0x19,0x5000,
	0x18,0x0784,
	0x19,0x8500,
	0x18,0x0794,
	0x19,0x7f00,
	0x18,0x07a4,
	0x19,0xac00,
	0x18,0x07b4,
	0x19,0x0800,
	0x18,0x07c4,
	0x19,0xfc00,
	0x18,0x07d4,
	0x19,0x4000,
	0x18,0x07e4,
	0x19,0x7f00,
	0x18,0x07f4,
	0x19,0x0400,
	0x18,0x0804,
	0x19,0x0200,
	0x18,0x0814,
	0x19,0x0000,
	0x18,0x0824,
	0x19,0xff00,
	0x18,0x0834,
	0x19,0x7f00,
	0x18,0x0844,
	0x19,0x0000,
	0x18,0x0854,
	0x19,0x4200,
	0x18,0x0864,
	0x19,0x0500,
	0x18,0x0874,
	0x19,0x9000,
	0x18,0x0884,
	0x19,0x8000,
	0x18,0x0894,
	0x19,0x7d00,
	0x18,0x08a4,
	0x19,0x8c00,
	0x18,0x08b4,
	0x19,0x8300,
	0x18,0x08c4,
	0x19,0xe000,
	0x18,0x08d4,
	0x19,0x0000,
	0x18,0x08e4,
	0x19,0x4000,
	0x18,0x08f4,
	0x19,0x0400,
	0x18,0x0904,
	0x19,0xff00,
	0x18,0x0914,
	0x19,0x0500,
	0x18,0x0924,
	0x19,0x8500,
	0x18,0x0934,
	0x19,0x8c00,
	0x18,0x0944,
	0x19,0xfa00,
	0x18,0x0954,
	0x19,0xe000,
	0x18,0x0964,
	0x19,0x0000,
	0x18,0x0974,
	0x19,0x4000,
	0x18,0x0984,
	0x19,0x5f00,
	0x18,0x0994,
	0x19,0x0400,
	0x18,0x09a4,
	0x19,0x0000,
	0x18,0x09b4,
	0x19,0xfe00,
	0x18,0x09c4,
	0x19,0x7300,
	0x18,0x09d4,
	0x19,0x0d00,
	0x18,0x09e4,
	0x19,0x0300,
	0x18,0x09f4,
	0x19,0x4000,
	0x18,0x0a04,
	0x19,0x2000,
	0x18,0x0a14,
	0x19,0x0000,
	0x18,0x0a24,
	0x19,0x0400,
	0x18,0x0a34,
	0x19,0xda00,
	0x18,0x0a44,
	0x19,0x0600,
	0x18,0x0a54,
	0x19,0x7d00,
	0x18,0x0a64,
	0x19,0x4000,
	0x18,0x0a74,
	0x19,0x5f00,
	0x18,0x0a84,
	0x19,0x0400,
	0x18,0x0a94,
	0x19,0x0000,
	0x18,0x0aa4,
	0x19,0x0000,
	0x18,0x0ab4,
	0x19,0x7300,
	0x18,0x0ac4,
	0x19,0x0d00,
	0x18,0x0ad4,
	0x19,0x0300,
	0x18,0x0ae4,
	0x19,0x0400,
	0x18,0x0af4,
	0x19,0xce00,
	0x18,0x0b04,
	0x19,0x0900,
	0x18,0x0b14,
	0x19,0x9d00,
	0x18,0x0b24,
	0x19,0x0800,
	0x18,0x0b34,
	0x19,0x9000,
	0x18,0x0b44,
	0x19,0x0700,
	0x18,0x0b54,
	0x19,0x7900,
	0x18,0x0b64,
	0x19,0x4000,
	0x18,0x0b74,
	0x19,0x7f00,
	0x18,0x0b84,
	0x19,0x0400,
	0x18,0x0b94,
	0x19,0x0000,
	0x18,0x0ba4,
	0x19,0x0000,
	0x18,0x0bb4,
	0x19,0x0400,
	0x18,0x0bc4,
	0x19,0x7300,
	0x18,0x0bd4,
	0x19,0x0d00,
	0x18,0x0be4,
	0x19,0x0100,
	0x18,0x0bf4,
	0x19,0x0900,
	0x18,0x0c04,
	0x19,0x8e00,
	0x18,0x0c14,
	0x19,0x0800,
	0x18,0x0c24,
	0x19,0x7d00,
	0x18,0x0c34,
	0x19,0x4000,
	0x18,0x0c44,
	0x19,0x7f00,
	0x18,0x0c54,
	0x19,0x0000,
	0x18,0x0c64,
	0x19,0x0000,
	0x18,0x0c74,
	0x19,0x0200,
	0x18,0x0c84,
	0x19,0x0000,
	0x18,0x0c94,
	0x19,0x7000,
	0x18,0x0ca4,
	0x19,0x0c00,
	0x18,0x0cb4,
	0x19,0x0100,
	0x18,0x0cc4,
	0x19,0x0900,
	0x18,0x0cd4,
	0x19,0x7f00,
	0x18,0x0ce4,
	0x19,0x4000,
	0x18,0x0cf4,
	0x19,0x7f00,
	0x18,0x0d04,
	0x19,0x3400,
	0x18,0x0d14,
	0x19,0x8300,
	0x18,0x0d24,
	0x19,0x8200,
	0x18,0x0d34,
	0x19,0x0000,
	0x18,0x0d44,
	0x19,0x7000,
	0x18,0x0d54,
	0x19,0x0d00,
	0x18,0x0d64,
	0x19,0x0100,
	0x18,0x0d74,
	0x19,0x0f00,
	0x18,0x0d84,
	0x19,0x7f00,
	0x18,0x0d94,
	0x19,0x9a00,
	0x18,0x0da4,
	0x19,0x7d00,
	0x18,0x0db4,
	0x19,0x4000,
	0x18,0x0dc4,
	0x19,0x7f00,
	0x18,0x0dd4,
	0x19,0x1400,
	0x18,0x0de4,
	0x19,0x0000,
	0x18,0x0df4,
	0x19,0x8200,
	0x18,0x0e04,
	0x19,0x0000,
	0x18,0x0e14,
	0x19,0x7000,
	0x18,0x0e24,
	0x19,0x0f00,
	0x18,0x0e34,
	0x19,0x0100,
	0x18,0x0e44,
	0x19,0x9b00,
	0x18,0x0e54,
	0x19,0x7f00,
	0x18,0x0e64,
	0x19,0x4000,
	0x18,0x0e74,
	0x19,0x1f00,
	0x18,0x0e84,
	0x19,0x0200,
	0x18,0x0e94,
	0x19,0x0600,
	0x18,0x0ea4,
	0x19,0x7100,
	0x18,0x0eb4,
	0x19,0x1d00,
	0x18,0x0ec4,
	0x19,0x0100,
	0x18,0x0ed4,
	0x19,0x4000,
	0x18,0x0ee4,
	0x19,0x1f00,
	0x18,0x0ef4,
	0x19,0x0200,
	0x18,0x0f04,
	0x19,0x0600,
	0x18,0x0f14,
	0x19,0x7100,
	0x18,0x0f24,
	0x19,0x0d00,
	0x18,0x0f34,
	0x19,0x0100,
	0x18,0x0f44,
	0x19,0x4000,
	0x18,0x0f54,
	0x19,0x1f00,
	0x18,0x0f64,
	0x19,0x0200,
	0x18,0x0f74,
	0x19,0x0600,
	0x18,0x0f84,
	0x19,0x7100,
	0x18,0x0f94,
	0x19,0x0d00,
	0x18,0x0fa4,
	0x19,0x0100,
	0x18,0x0fb4,
	0x19,0x4000,
	0x18,0x0fc4,
	0x19,0x1f00,
	0x18,0x0fd4,
	0x19,0x0200,
	0x18,0x0fe4,
	0x19,0x0600,
	0x18,0x0ff4,
	0x19,0x7100,
	0x18,0x1004,
	0x19,0x0d00,
	0x18,0x1014,
	0x19,0x0100,
	0x18,0x1024,
	0x19,0x4000,
	0x18,0x1034,
	0x19,0x1f00,
	0x18,0x1044,
	0x19,0x0200,
	0x18,0x1054,
	0x19,0x0600,
	0x18,0x1064,
	0x19,0x7100,
	0x18,0x1074,
	0x19,0x0d00,
	0x18,0x1084,
	0x19,0x0100,
	0x18,0x1094,
	0x19,0x4000,
	0x18,0x10a4,
	0x19,0x1f00,
	0x18,0x10b4,
	0x19,0x0200,
	0x18,0x10c4,
	0x19,0x0600,
	0x18,0x10d4,
	0x19,0x7100,
	0x18,0x10e4,
	0x19,0x0d00,
	0x18,0x10f4,
	0x19,0x0100,
	0x18,0x1104,
	0x19,0x4000,
	0x18,0x1114,
	0x19,0x7f00,
	0x18,0x1124,
	0x19,0x0400,
	0x18,0x1134,
	0x19,0x9000,
	0x18,0x1144,
	0x19,0x0200,
	0x18,0x1154,
	0x19,0x0600,
	0x18,0x1164,
	0x19,0x7300,
	0x18,0x1174,
	0x19,0x0d00,
	0x18,0x1184,
	0x19,0x0100,
	0x18,0x1194,
	0x19,0x0b00,
	0x18,0x11a4,
	0x19,0x9500,
	0x18,0x11b4,
	0x19,0x9400,
	0x18,0x11c4,
	0x19,0x0400,
	0x18,0x11d4,
	0x19,0x4000,
	0x18,0x11e4,
	0x19,0x4000,
	0x18,0x11f4,
	0x19,0x0500,
	0x18,0x1204,
	0x19,0x8000,
	0x18,0x1214,
	0x19,0x7800,
	0x18,0x1224,
	0x19,0x4000,
	0x18,0x1234,
	0x19,0x7f00,
	0x18,0x1244,
	0x19,0x0400,
	0x18,0x1254,
	0x19,0x0000,
	0x18,0x1264,
	0x19,0x0200,
	0x18,0x1274,
	0x19,0x0000,
	0x18,0x1284,
	0x19,0x7000,
	0x18,0x1294,
	0x19,0x0f00,
	0x18,0x12a4,
	0x19,0x0100,
	0x18,0x12b4,
	0x19,0x9b00,
	0x18,0x12c4,
	0x19,0x7f00,
	0x18,0x12d4,
	0x19,0xe100,
	0x18,0x12e4,
	0x19,0x1000,
	0x18,0x12f4,
	0x19,0x4000,
	0x18,0x1304,
	0x19,0x7f00,
	0x18,0x1314,
	0x19,0x0500,
	0x18,0x1324,
	0x19,0x0000,
	0x18,0x1334,
	0x19,0x0000,
	0x18,0x1344,
	0x19,0x0600,
	0x18,0x1354,
	0x19,0x7300,
	0x18,0x1364,
	0x19,0x0d00,
	0x18,0x1374,
	0x19,0x0100,
	0x18,0x1384,
	0x19,0x0400,
	0x18,0x1394,
	0x19,0x0600,
	0x18,0x13a4,
	0x19,0x4000,
	0x18,0x13b4,
	0x19,0x4000,
	0x18,0x13c4,
	0x19,0x0400,
	0x18,0x13d4,
	0x19,0xe000,
	0x18,0x13e4,
	0x19,0x7d00,
	0x18,0x13f4,
	0x19,0x0500,
	0x18,0x1404,
	0x19,0x7800,
	0x18,0x1414,
	0x19,0x4000,
	0x18,0x1424,
	0x19,0x4000,
	0x18,0x1434,
	0x19,0x0400,
	0x18,0x1444,
	0x19,0xe000,
	0x18,0x1454,
	0x19,0x9700,
	0x18,0x1464,
	0x19,0x4000,
	0x18,0x1474,
	0x19,0x7f00,
	0x18,0x1484,
	0x19,0x0000,
	0x18,0x1494,
	0x19,0x0100,
	0x18,0x14a4,
	0x19,0x4400,
	0x18,0x14b4,
	0x19,0x0000,
	0x18,0x14c4,
	0x19,0x0000,
	0x18,0x14d4,
	0x19,0x0000,
	0x18,0x14e4,
	0x19,0x4000,
	0x18,0x14f4,
	0x19,0x8000,
	0x18,0x1504,
	0x19,0x7f00,
	0x18,0x1514,
	0x19,0x4000,
	0x18,0x1524,
	0x19,0x3f00,
	0x18,0x1534,
	0x19,0x0400,
	0x18,0x1544,
	0x19,0x5000,
	0x18,0x1554,
	0x19,0xf800,
	0x18,0x1564,
	0x19,0x0000,
	0x18,0x1574,
	0x19,0xe000,
	0x18,0x1584,
	0x19,0x4000,
	0x18,0x1594,
	0x19,0x8000,
	0x18,0x15a4,
	0x19,0x7f00,
	0x18,0x15b4,
	0x19,0x8900,
	0x18,0x15c4,
	0x19,0x8300,
	0x18,0x15d4,
	0x19,0xe000,
	0x18,0x15e4,
	0x19,0x0000,
	0x18,0x15f4,
	0x19,0x4000,
	0x18,0x1604,
	0x19,0x7f00,
	0x18,0x1614,
	0x19,0x0200,
	0x18,0x1624,
	0x19,0x1000,
	0x18,0x1634,
	0x19,0x0000,
	0x18,0x1644,
	0x19,0xfc00,
	0x18,0x1654,
	0x19,0xfd00,
	0x18,0x1664,
	0x19,0x0000,
	0x18,0x1674,
	0x19,0x4000,
	0x18,0x1684,
	0x19,0xbc00,
	0x18,0x1694,
	0x19,0x0e00,
	0x18,0x16a4,
	0x19,0xfe00,
	0x18,0x16b4,
	0x19,0x8a00,
	0x18,0x16c4,
	0x19,0x8300,
	0x18,0x16d4,
	0x19,0xe000,
	0x18,0x16e4,
	0x19,0x0000,
	0x18,0x16f4,
	0x19,0x4000,
	0x18,0x1704,
	0x19,0x7f00,
	0x18,0x1714,
	0x19,0x0100,
	0x18,0x1724,
	0x19,0xff00,
	0x18,0x1734,
	0x19,0x0000,
	0x18,0x1744,
	//### 0x2219 : 0xfc00 ##0309
	0x19,0xfc00,
	0x18,0x1754,
	//### 0x2219 : 0xff00 ##0309
	0x19,0xff00,
	0x18,0x1764,
	0x19,0x0000,
	0x18,0x1774,
	0x19,0x4000,
	0x18,0x1784,
	0x19,0x9d00,
	0x18,0x1794,
	0x19,0xff00,
	0x18,0x17a4,
	0x19,0x4000,
	0x18,0x17b4,
	0x19,0x7f00,
	0x18,0x17c4,
	0x19,0x0000,
	0x18,0x17d4,
	0x19,0xff00,
	0x18,0x17e4,
	0x19,0x0000,
	0x18,0x17f4,
	0x19,0xfc00,
	0x18,0x1804,
	0x19,0xff00,
	0x18,0x1814,
	0x19,0x0000,
	0x18,0x1824,
	0x19,0x4000,
	0x18,0x1834,
	0x19,0x8900,
	0x18,0x1844,
	0x19,0x8300,
	0x18,0x1854,
	0x19,0xe000,
	0x18,0x1864,
	0x19,0x0000,
	0x18,0x1874,
	0x19,0xac00,
	0x18,0x1884,
	0x19,0x0800,
	0x18,0x1894,
	0x19,0xfa00,
	0x18,0x18a4,
	0x19,0x4000,
	0x18,0x18b4,
	0x19,0x7f00,
	0x18,0x18c4,
	0x19,0x0400,
	0x18,0x18d4,
	0x19,0x0200,
	0x18,0x18e4,
	0x19,0x0000,
	0x18,0x18f4,
	0x19,0xfd00,
	0x18,0x1904,
	0x19,0x7f00,
	0x18,0x1914,
	0x19,0x0000,
	0x18,0x1924,
	0x19,0x4000,
	0x18,0x1934,
	0x19,0x0500,
	0x18,0x1944,
	0x19,0x9000,
	0x18,0x1954,
	0x19,0x8000,
	0x18,0x1964,
	0x19,0x7d00,
	0x18,0x1974,
	0x19,0x8c00,
	0x18,0x1984,
	0x19,0x8300,
	0x18,0x1994,
	0x19,0xe000,
	0x18,0x19a4,
	0x19,0x0000,
	0x18,0x19b4,
	0x19,0x4000,
	0x18,0x19c4,
	0x19,0x0400,
	0x18,0x19d4,
	0x19,0xff00,
	0x18,0x19e4,
	0x19,0x0500,
	0x18,0x19f4,
	0x19,0x8500,
	0x18,0x1a04,
	0x19,0x8c00,
	0x18,0x1a14,
	0x19,0xfa00,
	0x18,0x1a24,
	0x19,0xe000,
	0x18,0x1a34,
	0x19,0x0000,
	0x18,0x1a44,
	0x19,0x4000,
	0x18,0x1a54,
	0x19,0x5f00,
	0x18,0x1a64,
	0x19,0x0400,
	0x18,0x1a74,
	0x19,0x0000,
	0x18,0x1a84,
	0x19,0xfc00,
	0x18,0x1a94,
	0x19,0x7300,
	0x18,0x1aa4,
	0x19,0x0d00,
	0x18,0x1ab4,
	0x19,0x0100,
	0x18,0x1ac4,
	0x19,0x4000,
	0x18,0x1ad4,
	0x19,0x2000,
	0x18,0x1ae4,
	0x19,0x0000,
	0x18,0x1af4,
	0x19,0x0400,
	0x18,0x1b04,
	0x19,0xda00,
	0x18,0x1b14,
	0x19,0x0600,
	0x18,0x1b24,
	0x19,0x7d00,
	0x18,0x1b34,
	0x19,0x4000,
	0x18,0x1b44,
	0x19,0x5f00,
	0x18,0x1b54,
	0x19,0x0400,
	0x18,0x1b64,
	0x19,0x0000,
	0x18,0x1b74,
	0x19,0x0000,
	0x18,0x1b84,
	0x19,0x7300,
	0x18,0x1b94,
	0x19,0x0d00,
	0x18,0x1ba4,
	0x19,0x0100,
	0x18,0x1bb4,
	0x19,0x0400,
	0x18,0x1bc4,
	0x19,0xce00,
	0x18,0x1bd4,
	0x19,0x0800,
	0x18,0x1be4,
	0x19,0x9200,
	0x18,0x1bf4,
	0x19,0x0900,
	0x18,0x1c04,
	0x19,0x9b00,
	0x18,0x1c14,
	0x19,0x0700,
	0x18,0x1c24,
	0x19,0x7900,
	0x18,0x1c34,
	0x19,0x4000,
	0x18,0x1c44,
	0x19,0x7f00,
	0x18,0x1c54,
	0x19,0x0400,
	0x18,0x1c64,
	0x19,0x0000,
	0x18,0x1c74,
	0x19,0x0000,
	0x18,0x1c84,
	0x19,0x0400,
	0x18,0x1c94,
	0x19,0x7300,
	0x18,0x1ca4,
	0x19,0x0d00,
	0x18,0x1cb4,
	0x19,0x0100,
	0x18,0x1cc4,
	0x19,0x0900,
	0x18,0x1cd4,
	0x19,0x8e00,
	0x18,0x1ce4,
	0x19,0x0800,
	0x18,0x1cf4,
	0x19,0x7d00,
	0x18,0x1d04,
	0x19,0x4000,
	0x18,0x1d14,
	0x19,0x7f00,
	0x18,0x1d24,
	0x19,0x0000,
	0x18,0x1d34,
	0x19,0x0000,
	0x18,0x1d44,
	0x19,0x0000,
	0x18,0x1d54,
	0x19,0x0000,
	0x18,0x1d64,
	0x19,0x7000,
	0x18,0x1d74,
	0x19,0x0c00,
	0x18,0x1d84,
	0x19,0x0100,
	0x18,0x1d94,
	0x19,0x0900,
	0x18,0x1da4,
	0x19,0x7f00,
	0x18,0x1db4,
	0x19,0x4000,
	0x18,0x1dc4,
	0x19,0x7f00,
	0x18,0x1dd4,
	0x19,0x0400,
	0x18,0x1de4,
	0x19,0x0000,
	0x18,0x1df4,
	0x19,0x0000,
	0x18,0x1e04,
	0x19,0x0000,
	0x18,0x1e14,
	0x19,0x7000,
	0x18,0x1e24,
	0x19,0x0d00,
	0x18,0x1e34,
	0x19,0x0100,
	0x18,0x1e44,
	0x19,0x0b00,
	0x18,0x1e54,
	0x19,0x7f00,
	0x18,0x1e64,
	0x19,0x9a00,
	0x18,0x1e74,
	0x19,0x7f00,
	0x18,0x1e84,
	0x19,0x4000,
	0x18,0x1e94,
	0x19,0x7f00,
	0x18,0x1ea4,
	0x19,0x0400,
	0x18,0x1eb4,
	0x19,0x0000,
	0x18,0x1ec4,
	0x19,0x0000,
	0x18,0x1ed4,
	0x19,0x0000,
	0x18,0x1ee4,
	0x19,0x7100,
	0x18,0x1ef4,
	0x19,0x0d00,
	0x18,0x1f04,
	0x19,0x0100,
	0x18,0x1f14,
	0x19,0x9400,
	0x18,0x1f24,
	0x19,0x7f00,
	0x18,0x1f34,
	0x19,0x4000,
	0x18,0x1f44,
	0x19,0x7f00,
	0x18,0x1f54,
	0x19,0x0500,
	0x18,0x1f64,
	0x19,0x0000,
	0x18,0x1f74,
	0x19,0x0000,
	0x18,0x1f84,
	0x19,0x0000,
	0x18,0x1f94,
	0x19,0x7300,
	0x18,0x1fa4,
	0x19,0x0d00,
	0x18,0x1fb4,
	0x19,0x0100,
	0x18,0x1fc4,
	0x19,0x0500,
	0x18,0x1fd4,
	0x19,0x8800,
	0x18,0x1fe4,
	0x19,0x0400,
	0x18,0x1ff4,
	0x19,0x7d00,
	0x18,0x2004,
	0x19,0x4000,
	0x18,0x2014,
	0x19,0x4000,
	0x18,0x2024,
	0x19,0x0400,
	0x18,0x2034,
	0x19,0xe100,
	0x18,0x2044,
	0x19,0x8a00,
	0x18,0x2054,
	0x19,0x4000,
	0x18,0x2064,
	0x19,0x4000,
	0x18,0x2074,
	0x19,0x0400,
	0x18,0x2084,
	0x19,0xe100,
	0x18,0x2094,
	0x19,0xa400,
	0x18,0x20a4,
	0x19,0x4000,
	0x18,0x20b4,
	0x19,0x7f00,
	0x18,0x20c4,
	0x19,0x0000,
	0x18,0x20d4,
	0x19,0x0100,
	0x18,0x20e4,
	0x19,0x4000,
	0x18,0x20f4,
	0x19,0x3e00,
	0x18,0x2104,
	0x19,0x0000,
	0x18,0x2114,
	0x19,0xe000,
	0x18,0x2124,
	0x19,0x1200,
	0x18,0x2134,
	0x19,0x8000,
	0x18,0x2144,
	0x19,0x7f00,
	0x18,0x2154,
	0x19,0x8900,
	0x18,0x2164,
	0x19,0x8300,
	0x18,0x2174,
	0x19,0xe000,
	0x18,0x2184,
	0x19,0x0000,
	0x18,0x2194,
	0x19,0x4000,
	0x18,0x21a4,
	0x19,0x7f00,
	0x18,0x21b4,
	0x19,0x0000,
	0x18,0x21c4,
	0x19,0x2000,
	0x18,0x21d4,
	0x19,0x4000,
	0x18,0x21e4,
	0x19,0x3e00,
	0x18,0x21f4,
	0x19,0xff00,
	0x18,0x2204,
	0x19,0x0000,
	0x18,0x2214,
	0x19,0x1200,
	0x18,0x2224,
	0x19,0x8000,
	0x18,0x2234,
	0x19,0x7f00,
	0x18,0x2244,
	0x19,0x8600,
	0x18,0x2254,
	0x19,0x8500,
	0x18,0x2264,
	0x19,0x8900,
	0x18,0x2274,
	0x19,0xfd00,
	0x18,0x2284,
	0x19,0xe000,
	0x18,0x2294,
	0x19,0x0000,
	0x18,0x22a4,
	0x19,0x9500,
	0x18,0x22b4,
	0x19,0x0400,
	0x18,0x22c4,
	0x19,0x4000,
	0x18,0x22d4,
	0x19,0x4000,
	0x18,0x22e4,
	0x19,0x1000,
	0x18,0x22f4,
	0x19,0x4000,
	0x18,0x2304,
	0x19,0x3f00,
	0x18,0x2314,
	0x19,0x0200,
	0x18,0x2324,
	0x19,0x4000,
	0x18,0x2334,
	0x19,0x3700,
	0x18,0x2344,
	0x19,0x7f00,
	0x18,0x2354,
	0x19,0x0000,
	0x18,0x2364,
	0x19,0x0200,
	0x18,0x2374,
	0x19,0x0200,
	0x18,0x2384,
	0x19,0x9000,
	0x18,0x2394,
	0x19,0x8000,
	0x18,0x23a4,
	0x19,0x7d00,
	0x18,0x23b4,
	0x19,0x8900,
	0x18,0x23c4,
	0x19,0x8300,
	0x18,0x23d4,
	0x19,0xe000,
	0x18,0x23e4,
	0x19,0x0000,
	0x18,0x23f4,
	0x19,0x4000,
	0x18,0x2404,
	0x19,0x0400,
	0x18,0x2414,
	0x19,0xff00,
	0x18,0x2424,
	0x19,0x0200,
	0x18,0x2434,
	0x19,0x8500,
	0x18,0x2444,
	0x19,0x8900,
	0x18,0x2454,
	0x19,0xfa00,
	0x18,0x2464,
	0x19,0xe000,
	0x18,0x2474,
	0x19,0x0000,
	0x18,0x2484,
	0x19,0x4000,
	0x18,0x2494,
	0x19,0x7f00,
	0x18,0x24a4,
	0x19,0x0000,
	0x18,0x24b4,
	0x19,0x0000,
	0x18,0x24c4,
	0x19,0x4000,
	0x18,0x24d4,
	0x19,0x3700,
	0x18,0x24e4,
	0x19,0x7300,
	0x18,0x24f4,
	0x19,0x0500,
	0x18,0x2504,
	0x19,0x0200,
	0x18,0x2514,
	0x19,0x0100,
	0x18,0x2524,
	0x19,0xd800,
	0x18,0x2534,
	0x19,0x0400,
	0x18,0x2544,
	0x19,0x7d00,
	0x18,0x2554,
	0x19,0x4000,
	0x18,0x2564,
	0x19,0x7f00,
	0x18,0x2574,
	0x19,0x0000,
	0x18,0x2584,
	0x19,0x0000,
	0x18,0x2594,
	0x19,0x4000,
	0x18,0x25a4,
	0x19,0x0000,
	0x18,0x25b4,
	0x19,0x7200,
	0x18,0x25c4,
	0x19,0x0400,
	0x18,0x25d4,
	0x19,0x0000,
	0x18,0x25e4,
	0x19,0x0800,
	0x18,0x25f4,
	0x19,0x7f00,
	0x18,0x2604,
	0x19,0x4000,
	0x18,0x2614,
	0x19,0x7f00,
	0x18,0x2624,
	0x19,0x0000,
	0x18,0x2634,
	0x19,0x0000,
	0x18,0x2644,
	0x19,0xc000,
	0x18,0x2654,
	0x19,0x0000,
	0x18,0x2664,
	0x19,0x7200,
	0x18,0x2674,
	0x19,0x0500,
	0x18,0x2684,
	0x19,0x0000,
	0x18,0x2694,
	0x19,0x0400,
	0x18,0x26a4,
	0x19,0xeb00,
	0x18,0x26b4,
	0x19,0x8400,
	0x18,0x26c4,
	0x19,0x7d00,
	0x18,0x26d4,
	0x19,0x4000,
	0x18,0x26e4,
	0x19,0x7f00,
	0x18,0x26f4,
	0x19,0x0000,
	0x18,0x2704,
	0x19,0x0000,
	0x18,0x2714,
	0x19,0x4000,
	0x18,0x2724,
	0x19,0x0000,
	0x18,0x2734,
	0x19,0x7200,
	0x18,0x2744,
	0x19,0x0700,
	0x18,0x2754,
	0x19,0x0000,
	0x18,0x2764,
	0x19,0x0400,
	0x18,0x2774,
	0x19,0xde00,
	0x18,0x2784,
	0x19,0x9b00,
	0x18,0x2794,
	0x19,0x7d00,
	0x18,0x27a4,
	0x19,0x4000,
	0x18,0x27b4,
	0x19,0x7f00,
	0x18,0x27c4,
	0x19,0x0000,
	0x18,0x27d4,
	0x19,0x9000,
	0x18,0x27e4,
	0x19,0x4000,
	0x18,0x27f4,
	0x19,0x0400,
	0x18,0x2804,
	0x19,0x7300,
	0x18,0x2814,
	0x19,0x1500,
	0x18,0x2824,
	0x19,0x0000,
	0x18,0x2834,
	0x19,0x0400,
	0x18,0x2844,
	0x19,0xd100,
	0x18,0x2854,
	0x19,0x9400,
	0x18,0x2864,
	0x19,0x9200,
	0x18,0x2874,
	0x19,0x8000,
	0x18,0x2884,
	0x19,0x7b00,
	0x18,0x2894,
	0x19,0x4000,
	0x18,0x28a4,
	0x19,0x7f00,
	0x18,0x28b4,
	0x19,0x0000,
	0x18,0x28c4,
	0x19,0x0000,
	0x18,0x28d4,
	0x19,0x4000,
	0x18,0x28e4,
	0x19,0x0000,
	0x18,0x28f4,
	0x19,0x7200,
	0x18,0x2904,
	0x19,0x0700,
	0x18,0x2914,
	0x19,0x0000,
	0x18,0x2924,
	0x19,0x0400,
	0x18,0x2934,
	0x19,0xc200,
	0x18,0x2944,
	0x19,0x9b00,
	0x18,0x2954,
	0x19,0x7d00,
	0x18,0x2964,
	0x19,0xe200,
	0x18,0x2974,
	0x19,0x7a00,
	0x18,0x2984,
	0x19,0x4000,
	0x18,0x2994,
	0x19,0x7f00,
	0x18,0x29a4,
	0x19,0x0000,
	0x18,0x29b4,
	0x19,0x0000,
	0x18,0x29c4,
	0x19,0x4000,
	0x18,0x29d4,
	0x19,0x3700,
	0x18,0x29e4,
	0x19,0x7300,
	0x18,0x29f4,
	0x19,0x0500,
	0x18,0x2a04,
	0x19,0x0000,
	0x18,0x2a14,
	0x19,0x0100,
	0x18,0x2a24,
	0x19,0x0300,
	0x18,0x2a34,
	0x19,0xe200,
	0x18,0x2a44,
	0x19,0x2a00,
	0x18,0x2a54,
	0x19,0x0200,
	0x18,0x2a64,
	0x19,0x7b00,
	0x18,0x2a74,
	0x19,0xe200,
	0x18,0x2a84,
	0x19,0x4800,
	//### endpoint

	0x1f,0x0000,
	0x17,0x2100,
	0x1f,0x0007,
	0x1e,0x0040,
	0x18,0x0000,
	//### end of inrx dspctl patch code

	//### inrx eyesch patch code
	0x1f,0x0007,
	0x1e,0x0042,
	0x15,0x0f00,
	0x1f,0x0000,
	0x17,0x2160,
	0x1f,0x0001,
	0x10,0xf25e,
	0x1f,0x0007,
	0x1e,0x0042,

	//### startpoint
	0x15,0x0f00,
	0x16,0x7408,
	0x15,0x0e00,
	0x15,0x0f00,
	0x15,0x0f01,
	0x16,0x4000,
	0x15,0x0e01,
	0x15,0x0f01,
	0x15,0x0f02,
	0x16,0x9400,
	0x15,0x0e02,
	0x15,0x0f02,
	0x15,0x0f03,
	0x16,0x7408,
	0x15,0x0e03,
	0x15,0x0f03,
	0x15,0x0f04,
	0x16,0x4008,
	0x15,0x0e04,
	0x15,0x0f04,
	0x15,0x0f05,
	0x16,0x9400,
	0x15,0x0e05,
	0x15,0x0f05,
	0x15,0x0f06,
	0x16,0x0803,
	0x15,0x0e06,
	0x15,0x0f06,
	//### endpoint

	0x1f, 0x0001,
	0x10, 0xf05e,
	0x1f, 0x0007,
	0x1e, 0x0042,
	0x15,0x0d00,
	0x15,0x0100,
	0x1f,0x0000,
	0x17,0x2100,
	//### end of inrx eyesch patch code

	//### release MDI/MDIX force mode
	0x1f, 0x0007,
	0x1e, 0x002d,
	0x18, 0xf010,

	//### uc patch code (20110103 add foce giga mode)
	0x1f,0x0005,
	//### startpoint

	5, 0x8000,
	6, 0xeeff,
	6, 0xfc8b,
	6, 0xeeff,
	6, 0xfda0,
	6, 0x0280,
	6, 0x33f7,
	6, 0x00e0,
	6, 0xfff7,
	6, 0xa080,
	6, 0x02ae,
	6, 0xf602,
	6, 0x842b,
	6, 0x0201,
	6, 0x4802,
	6, 0x015b,
	6, 0x0280,
	6, 0xabe0,
	6, 0x8b8c,
	6, 0xe18b,
	6, 0x8d1e,
	6, 0x01e1,
	6, 0x8b8e,
	6, 0x1e01,
	6, 0xa000,
	6, 0xe4ae,
	6, 0xd8bf,
	6, 0x846c,
	6, 0xd785,
	6, 0x80d0,
	6, 0x6c02,
	6, 0x28b4,
	6, 0xeee1,
	6, 0x4477,
	6, 0xeee1,
	6, 0x4565,
	6, 0xee8b,
	6, 0x85c2,
	6, 0xee8a,
	6, 0xe86e,
	6, 0xee85,
	6, 0x7100,
	6, 0xee85,
	6, 0x7200,
	6, 0xee85,
	6, 0x7302,
	6, 0xee85,
	6, 0x7a03,
	6, 0xee85,
	6, 0x7bb8,
	6, 0xee85,
	6, 0x7400,
	6, 0xee85,
	6, 0x7500,
	6, 0xee85,
	6, 0x7000,
	6, 0xd407,
	6, 0xf7e4,
	6, 0x8b96,
	6, 0xe58b,
	6, 0x97d4,
	6, 0x0802,
	6, 0xe48b,
	6, 0x94e5,
	6, 0x8b95,
	6, 0xd100,
	6, 0xbf84,
	6, 0x5d02,
	6, 0x2959,
	6, 0xbf8b,
	6, 0x88ec,
	6, 0x0019,
	6, 0xa98b,
	6, 0x90f9,
	6, 0xeeff,
	6, 0xf600,
	6, 0xeeff,
	6, 0xf7fc,
	6, 0xe0e1,
	6, 0x40e1,
	6, 0xe141,
	6, 0xf72f,
	6, 0xf628,
	6, 0xe4e1,
	6, 0x40e5,
	6, 0xe141,
	6, 0x04f8,
	6, 0xe08b,
	6, 0x8ead,
	6, 0x2017,
	6, 0xf620,
	6, 0xe48b,
	6, 0x8e02,
	6, 0x25da,
	6, 0x0226,
	6, 0xb402,
	6, 0x8169,
	6, 0x0202,
	6, 0x3402,
	6, 0x82b1,
	6, 0x0283,
	6, 0x4ae0,
	6, 0x8b8e,
	6, 0xad23,
	6, 0x05f6,
	6, 0x23e4,
	6, 0x8b8e,
	6, 0xe08b,
	6, 0x8ead,
	6, 0x2408,
	6, 0xf624,
	6, 0xe48b,
	6, 0x8e02,
	6, 0x277d,
	6, 0xe08b,
	6, 0x8ead,
	6, 0x260b,
	6, 0xf626,
	6, 0xe48b,
	6, 0x8e02,
	6, 0x056e,
	6, 0x021c,
	6, 0x9a02,
	6, 0x80fb,
	6, 0x0281,
	6, 0x55fc,
	6, 0x04f8,
	6, 0xe08b,
	6, 0x83ad,
	6, 0x2321,
	6, 0xe0e0,
	6, 0x22e1,
	6, 0xe023,
	6, 0xad29,
	6, 0x20e0,
	6, 0x8b83,
	6, 0xad21,
	6, 0x06e1,
	6, 0x8b84,
	6, 0xad28,
	6, 0x3ce0,
	6, 0x8b85,
	6, 0xad21,
	6, 0x06e1,
	6, 0x8b84,
	6, 0xad29,
	6, 0x30bf,
	6, 0x3144,
	6, 0x0228,
	6, 0xe8ae,
	6, 0x28ee,
	6, 0x8ae2,
	6, 0x00ee,
	6, 0x8ae3,
	6, 0x00ee,
	6, 0x8ae4,
	6, 0x00ee,
	6, 0x8ae5,
	6, 0x00ee,
	6, 0x8b72,
	6, 0x00e0,
	6, 0x8b83,
	6, 0xad21,
	6, 0x08e0,
	6, 0x8b84,
	6, 0xf620,
	6, 0xe48b,
	6, 0x84bf,
	6, 0x3147,
	6, 0x0228,
	6, 0xe8fc,
	6, 0x04f8,
	6, 0xe0e0,
	6, 0x38e1,
	6, 0xe039,
	6, 0xac2e,
	6, 0x08ee,
	6, 0xe08e,
	6, 0x36ee,
	6, 0xe08f,
	6, 0x20fc,
	6, 0x04f8,
	6, 0xfaef,
	6, 0x69e0,
	6, 0x8b85,
	6, 0xad21,
	6, 0x39e0,
	6, 0xe022,
	6, 0xe1e0,
	6, 0x2358,
	6, 0xc059,
	6, 0x021e,
	6, 0x01e1,
	6, 0x8b72,
	6, 0x1f10,
	6, 0x9e26,
	6, 0xe48b,
	6, 0x72ad,
	6, 0x211d,
	6, 0xe18b,
	6, 0x84f7,
	6, 0x29e5,
	6, 0x8b84,
	6, 0xac27,
	6, 0x0dac,
	6, 0x2605,
	6, 0x0204,
	6, 0xa2ae,
	6, 0x0d02,
	6, 0x820c,
	6, 0xae08,
	6, 0x0282,
	6, 0x38ae,
	6, 0x0302,
	6, 0x81b1,
	6, 0xef96,
	6, 0xfefc,
	6, 0x04d1,
	6, 0x00bf,
	6, 0x845d,
	6, 0x0229,
	6, 0x59d1,
	6, 0x03bf,
	6, 0x316b,
	6, 0x0229,
	6, 0x59d1,
	6, 0x00bf,
	6, 0x316e,
	6, 0x0229,
	6, 0x59d1,
	6, 0x00bf,
	6, 0x8463,
	6, 0x0229,
	6, 0x59d1,
	6, 0x0fbf,
	6, 0x3162,
	6, 0x0229,
	6, 0x59d1,
	6, 0x01bf,
	6, 0x3165,
	6, 0x0229,
	6, 0x59d1,
	6, 0x01bf,
	6, 0x3168,
	6, 0x0229,
	6, 0x59e0,
	6, 0x8b85,
	6, 0xad24,
	6, 0x0ed1,
	6, 0x00bf,
	6, 0x3177,
	6, 0xad23,
	6, 0x03bf,
	6, 0x317a,
	6, 0x0229,
	6, 0x59e0,
	6, 0x8b83,
	6, 0xad22,
	6, 0x08d1,
	6, 0x00bf,
	6, 0x315c,
	6, 0x0229,
	6, 0x5904,
	6, 0xd102,
	6, 0xbf31,
	6, 0x6b02,
	6, 0x2959,
	6, 0xd106,
	6, 0xbf31,
	6, 0x6202,
	6, 0x2959,
	6, 0xd107,
	6, 0xbf31,
	6, 0x6502,
	6, 0x2959,
	6, 0xd107,
	6, 0xbf31,
	6, 0x6802,
	6, 0x2959,
	6, 0xd101,
	6, 0xbf84,
	6, 0x5d02,
	6, 0x2959,
	6, 0x0282,
	6, 0x7104,
	6, 0xd102,
	6, 0xbf31,
	6, 0x6b02,
	6, 0x2959,
	6, 0xd101,
	6, 0xbf84,
	6, 0x5d02,
	6, 0x2959,
	6, 0xd011,
	6, 0x0228,
	6, 0x0459,
	6, 0x03ef,
	6, 0x01d1,
	6, 0x00a0,
	6, 0x0002,
	6, 0xd101,
	6, 0xbf31,
	6, 0x6e02,
	6, 0x2959,
	6, 0xd111,
	6, 0xad20,
	6, 0x020c,
	6, 0x11ad,
	6, 0x2102,
	6, 0x0c12,
	6, 0xbf84,
	6, 0x6302,
	6, 0x2959,
	6, 0x04f8,
	6, 0xe08b,
	6, 0x85ad,
	6, 0x2437,
	6, 0xe0ea,
	6, 0xcae1,
	6, 0xeacb,
	6, 0xad29,
	6, 0x2ee0,
	6, 0xeacc,
	6, 0xe1ea,
	6, 0xcdad,
	6, 0x2925,
	6, 0xe0e0,
	6, 0x08e1,
	6, 0xe009,
	6, 0xad20,
	6, 0x1ce0,
	6, 0xe00a,
	6, 0xe1e0,
	6, 0x0bad,
	6, 0x2013,
	6, 0xd103,
	6, 0xbf31,
	6, 0x77e0,
	6, 0x8b85,
	6, 0xad23,
	6, 0x05d1,
	6, 0x01bf,
	6, 0x317a,
	6, 0x0229,
	6, 0x59fc,
	6, 0x04f8,
	6, 0xf9fb,
	6, 0xe08b,
	6, 0x85ad,
	6, 0x2522,
	6, 0xe0e0,
	6, 0x22e1,
	6, 0xe023,
	6, 0xe2e0,
	6, 0x36e3,
	6, 0xe037,
	6, 0x5ac4,
	6, 0x0d01,
	6, 0x5802,
	6, 0x1e20,
	6, 0xe385,
	6, 0x71ac,
	6, 0x3160,
	6, 0xac3a,
	6, 0x08ac,
	6, 0x3e26,
	6, 0xae67,
	6, 0xaf83,
	6, 0x46ad,
	6, 0x3761,
	6, 0xe085,
	6, 0x7210,
	6, 0xe485,
	6, 0x72e1,
	6, 0x8573,
	6, 0x1b10,
	6, 0x9e02,
	6, 0xae51,
	6, 0xd100,
	6, 0xbf84,
	6, 0x6002,
	6, 0x2959,
	6, 0xee85,
	6, 0x7200,
	6, 0xae43,
	6, 0xad36,
	6, 0x27e0,
	6, 0x857a,
	6, 0xe185,
	6, 0x7bef,
	6, 0x74e0,
	6, 0x8574,
	6, 0xe185,
	6, 0x751b,
	6, 0x749e,
	6, 0x2e14,
	6, 0xe485,
	6, 0x74e5,
	6, 0x8575,
	6, 0xef74,
	6, 0xe085,
	6, 0x7ae1,
	6, 0x857b,
	6, 0x1b47,
	6, 0x9e0f,
	6, 0xae19,
	6, 0xee85,
	6, 0x7400,
	6, 0xee85,
	6, 0x7500,
	6, 0xae0f,
	6, 0xac39,
	6, 0x0cd1,
	6, 0x01bf,
	6, 0x8460,
	6, 0x0229,
	6, 0x59ee,
	6, 0x8572,
	6, 0x00e6,
	6, 0x8571,
	6, 0xfffd,
	6, 0xfc04,
	6, 0xf8e0,
	6, 0x8b85,
	6, 0xad27,
	6, 0x30e0,
	6, 0xe036,
	6, 0xe1e0,
	6, 0x37e1,
	6, 0x8570,
	6, 0x1f10,
	6, 0x9e23,
	6, 0xe485,
	6, 0x70ac,
	6, 0x200b,
	6, 0xac21,
	6, 0x0dac,
	6, 0x250f,
	6, 0xac27,
	6, 0x11ae,
	6, 0x1202,
	6, 0x8383,
	6, 0xae0d,
	6, 0x0283,
	6, 0x88ae,
	6, 0x0802,
	6, 0x838f,
	6, 0xae03,
	6, 0x0283,
	6, 0xa2fc,
	6, 0x04ee,
	6, 0x8576,
	6, 0x0004,
	6, 0x0283,
	6, 0xaf02,
	6, 0x83f2,
	6, 0x04f8,
	6, 0xf9e0,
	6, 0x8b87,
	6, 0xad26,
	6, 0x08d1,
	6, 0x01bf,
	6, 0x8469,
	6, 0x0229,
	6, 0x59fd,
	6, 0xfc04,
	6, 0x0284,
	6, 0x1304,
	6, 0xee85,
	6, 0x7600,
	6, 0xee85,
	6, 0x7702,
	6, 0x04f8,
	6, 0xf9e0,
	6, 0x8b85,
	6, 0xad26,
	6, 0x38d0,
	6, 0x0b02,
	6, 0x2804,
	6, 0x5882,
	6, 0x7882,
	6, 0x9f2d,
	6, 0xe085,
	6, 0x76e1,
	6, 0x8577,
	6, 0x1f10,
	6, 0x9eb5,
	6, 0x10e4,
	6, 0x8576,
	6, 0xe0e0,
	6, 0x00e1,
	6, 0xe001,
	6, 0xf727,
	6, 0xe4e0,
	6, 0x00e5,
	6, 0xe001,
	6, 0xe2e0,
	6, 0x20e3,
	6, 0xe021,
	6, 0xad30,
	6, 0xf7f6,
	6, 0x27e4,
	6, 0xe000,
	6, 0xe5e0,
	6, 0x01fd,
	6, 0xfc04,
	6, 0xf8fa,
	6, 0xef69,
	6, 0xe08b,
	6, 0x86ad,
	6, 0x2212,
	6, 0xe0e0,
	6, 0x14e1,
	6, 0xe015,
	6, 0xad26,
	6, 0x89e1,
	6, 0x8578,
	6, 0xbf84,
	6, 0x6602,
	6, 0x2959,
	6, 0xef96,
	6, 0xfefc,
	6, 0x04f8,
	6, 0xfaef,
	6, 0x69e0,
	6, 0x8b86,
	6, 0xad22,
	6, 0x09e1,
	6, 0x8579,
	6, 0xbf84,
	6, 0x6602,
	6, 0x2959,
	6, 0xef96,
	6, 0xfefc,
	6, 0x04f8,
	6, 0xfaef,
	6, 0x69e0,
	6, 0x8b86,
	6, 0xac20,
	6, 0x1abf,
	6, 0x8580,
	6, 0xd06c,
	6, 0x0228,
	6, 0xbbe0,
	6, 0xe0e4,
	6, 0xe1e0,
	6, 0xe558,
	6, 0x0668,
	6, 0xc0d1,
	6, 0xd2e4,
	6, 0xe0e4,
	6, 0xe5e0,
	6, 0xe5ef,
	6, 0x96fe,
	6, 0xfc04,
	6, 0xa0e0,
	6, 0xeaf0,
	6, 0xe07c,
	6, 0x55e2,
	6, 0x3211,
	6, 0xe232,
	6, 0x88e2,
	6, 0x0070,
	6, 0xe426,
	6, 0x70e0,
	6, 0x7699,
	6, 0xe000,
	6, 0x2508,
	6, 0x0726,
	6, 0x4072,
	6, 0x2726,
	6, 0x7e28,
	6, 0x04b7,
	6, 0x2925,
	6, 0x762a,
	6, 0x68e5,
	6, 0x2bad,
	6, 0x002c,
	6, 0xdbf0,
	6, 0x2d67,
	6, 0xbb2e,
	6, 0x7b0f,
	6, 0x2f73,
	6, 0x6531,
	6, 0xaccc,
	6, 0x3223,
	6, 0x0033,
	6, 0x2d17,
	6, 0x347f,
	6, 0x5235,
	6, 0x1000,
	6, 0x3606,
	6, 0x0037,
	6, 0x0cc0,
	6, 0x387f,
	6, 0xce3c,
	6, 0xe5f7,
	6, 0x3d3d,
	6, 0xa465,
	6, 0x303e,
	6, 0x6700,
	6, 0x5369,
	6, 0xd20f,
	6, 0x6a01,
	6, 0x2c6c,
	6, 0x2b13,
	6, 0x6ee1,
	6, 0x006f,
	6, 0x12f7,
	6, 0x7100,
	6, 0x6b73,
	6, 0x06eb,
	6, 0x7494,
	6, 0xc776,
	6, 0x980a,
	6, 0x7750,
	6, 0x0078,
	6, 0x8a15,
	6, 0x797f,
	6, 0x6f7a,
	6, 0x06a6,
	//### endpoint

	//#unlock uc ramcode version
	5,  0xe142,
	6,  0x0701,
	5,  0xe140,
	6,  0x0405,
	15, 0x0000,

	//### end of uc patch code
	//#Enable negear EEE Nway ability autooff
	0x1f,0x0005,
	0x05,0x8b84,
	0x06,0x0026,
	0x1f,0x0000,

	//#lpi patch code-maxod-20110103
	31, 0x0007,
	30, 0x0023,
	22, 0x0006,
	//#quiet time to 22ms
	21, 0x147,
	25, 0x96,
	//#lpi_tx_wake_timer to 1.3us
	21, 0x16d,
	25, 0x26,
	22, 0x0002,
	31, 0x0000,

	//#Add by Gary for Channel Estimation fine tune 20100430
	//#Page1 Reg8 (CG_INITIAL_MASTER)
	0x1f, 0x0005,
	0x05, 0x83dd,
	0x06, 0x0574,
	//#Page1 Reg9 (CB0_INITIAL_GIGA)
	0x1f, 0x0005,
	0x05, 0x83e0,
	0x06, 0x2724,
	//#Page3 Reg26 (CG_INITIAL_SLAVE)
	0x1f, 0x0005,
	0x05, 0x843d,
	0x06, 0x06f6 ,

	0x1f, 0x0000,
	#endif
};

static const unsigned int default_val[]={
	999,0x1f,0x0002,

	2,0x11,0x7e00,

	3,0x1f,0x0002,
	3,0x17,0xff00,
	3,0x18,0x0005,
	3,0x19,0x0005,
	3,0x1a,0x0005,
	3,0x1b,0x0005,
	3,0x1c,0x0005,

	4,0x1f,0x0002,
	4,0x13,0x00aa,
	4,0x14,0x00aa,
	4,0x15,0x00aa,
	4,0x16,0x00aa,
	4,0x17,0x00aa,
	4,0x18,0x0f0a,
	4,0x19,0x50ab,
	4,0x1a,0x0000,
	4,0x1b,0x0f0f,

	999,0x1f,0x0000,
};

void Setting_RTL8198_GPHY(void)
{
	int i=0, port =0, len=0;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	if (REG32(BSP_REVR) == BSP_RTL8198_REVISION_A)
	{
		/*
		 #Access command format: phywb {all: phyID=0,1,2,3,4} {page} {RegAddr} {Bit location} {Bit value}
		 #when writing gpage 72, must do mdcmdio_cmd write $phyID 31 0x7, then mdcmdio_cmd write $phyID 30 $PageNum
		 phywb all 72 21 15-0 0x7092
		 phywb all 72 22 15-0 0x7092
		 phywb all 72 23 15-0 0x7092
		 phywb all 72 24 15-0 0x7092
		 phywb all 72 25 15-0 0x7092
		 phywb all 72 26 15-0 0x7092
		*/

		//	Set_GPHYWB(3, 2, 20, 0, 0x2000);

		Set_GPHYWB(999, 72, 21, 0, 0x7092);
		Set_GPHYWB(999, 72, 22, 0, 0x7092);
		Set_GPHYWB(999, 72, 23, 0, 0x7092);
		Set_GPHYWB(999, 72, 24, 0, 0x7092);
		Set_GPHYWB(999, 72, 25, 0, 0x7092);
		Set_GPHYWB(999, 72, 26, 0, 0x7092);

		/*
		 set PageNum 2; #All of GPHY register in the Page#2
		 #Array format = {{PhyID List1}  {RegAddr1 RegData1 RegAddr2 RegData2}, ...}

		set AFE_Reg     {{0 1 2 3 4} { 0 0x0000 1  0x065a 2 0x8c01  3  0x0428 4 0x80c8  5   0x0978  6  0x0678 7  0x3620 8 0x0000 9 0x0007 10 0x0000}
	       	               {2}         {11 0x0063 12 0xeb65 13 0x51d1 14 0x5dcb 15 0x3044 16 0x1000 17  0x7e00 18 0x0000}
	              	        {3}         {19 0x3d22 20 0x2000 21 0x6040 22 0x0000 23 0xff00 24 0x0005 25 0x0005 26 0x0005 27 0x0005 28 0x0005}
	                     	 {4}         {19 0x00aa 20 0x00aa 21 0x00aa 22 0x00aa 23 0x00aa 24 0x0f0a 25 0x5050 26 0x0000 27 0x0f0f }}
		*/

	       //phyid=all
		Set_GPHYWB(999, 2,  0, 0, 0x0000);
		Set_GPHYWB(999, 2,  1, 0, 0x065a);
		Set_GPHYWB(999, 2,  2, 0, 0x8c01);
		Set_GPHYWB(999, 2,  3, 0, 0x0428);
		Set_GPHYWB(999, 2,  4, 0, 0x80c8);
		Set_GPHYWB(999, 2,  5, 0, 0x0978);
		Set_GPHYWB(999, 2,  6, 0, 0x0678);
		Set_GPHYWB(999, 2,  7, 0, 0x3620);
		Set_GPHYWB(999, 2,  8, 0, 0x0000);
		Set_GPHYWB(999, 2,  9, 0, 0x0007);
		Set_GPHYWB(999, 2,  10, 0, 0x0000);

	       //phyid=2
		Set_GPHYWB( 2,   2, 11, 0, 0x0063);
		Set_GPHYWB( 2,   2, 12, 0, 0xeb65);
		Set_GPHYWB( 2,   2, 13, 0, 0x51d1);
		Set_GPHYWB( 2,   2, 14, 0, 0x5dcb);
		Set_GPHYWB( 2,   2, 15, 0, 0x3044);
		Set_GPHYWB( 2,   2, 16, 0, 0x1000);
		Set_GPHYWB( 2,   2, 17, 0, 0x7e00);
		Set_GPHYWB( 2,   2, 18, 0, 0x0000);

	       //phyid=3
		Set_GPHYWB( 3,   2, 19, 0, 0x3d22);
		Set_GPHYWB( 3,   2, 20, 0, 0x2000);
	   	Set_GPHYWB( 3,   2, 21, 0, 0x6040);
		Set_GPHYWB( 3,   2, 22, 0, 0x0000);
		Set_GPHYWB( 3,   2, 23, 0, 0xff00);
		Set_GPHYWB( 3,   2, 24, 0, 0x0005);
		Set_GPHYWB( 3,   2, 25, 0, 0x0005);
		Set_GPHYWB( 3,   2, 26, 0, 0x0005);
		Set_GPHYWB( 3,   2, 27, 0, 0x0005);
		Set_GPHYWB( 3,   2, 28, 0, 0x0005);

	       //phyid=4
		Set_GPHYWB( 4,   2, 19, 0, 0x00aa);
		Set_GPHYWB( 4,   2, 20, 0, 0x00aa);
		Set_GPHYWB( 4,   2, 21, 0, 0x00aa);
		Set_GPHYWB( 4,   2, 22, 0, 0x00aa);
		Set_GPHYWB( 4,   2, 23, 0, 0x00aa);
		Set_GPHYWB( 4,   2, 24, 0, 0x0f0a);
		Set_GPHYWB( 4,   2, 25, 0, 0x5050);
		Set_GPHYWB( 4,   2, 26, 0, 0x0000);
		Set_GPHYWB( 4,   2, 27, 0, 0x0f0f);

		/*
		 #=========== INRX Para. =================================

		 phywb all 0 21 0x1006
	           #dfse_mode[15:14]=3(full), Fine tune aagc_lvl_fnet[10:0]
	           phywb all 1 12 15-0 0xdbf0

	           #cb0_i_giga[12:0]
	           phywb all 1 9  15-0 0x2576
	           phywb all 1 7  15-0 0x287E
	           phywb all 1 10 15-0 0x68E5
	           phywb all 1 29 15-0 0x3DA4
	           phywb all 1 28 15-0 0xE7F7
	           phywb all 1 20 15-0 0x7F52
	           phywb all 1 24 15-0 0x7FCE
	           phywb all 1 8  15-0 0x04B7
	           phywb all 1 6  15-0 0x4072
	           phywb all 1 16 15-0 0xF05E
	           phywb all 1 27 15-0 0xB414
		*/

		Set_GPHYWB( 999,   1, 12, 0, 0xdbf0);

		Set_GPHYWB( 999,   1, 9, 0, 0x2576);
		Set_GPHYWB( 999,   1, 7, 0, 0x287E);
		Set_GPHYWB( 999,   1, 10, 0, 0x68E5);
		Set_GPHYWB( 999,   1, 29, 0, 0x3DA4);
		Set_GPHYWB( 999,   1, 28, 0, 0xE7F7);
		Set_GPHYWB( 999,   1, 20, 0, 0x7F52);
		Set_GPHYWB( 999,   1, 24, 0, 0x7FCE);
		Set_GPHYWB( 999,   1, 8, 0, 0x04B7);
		Set_GPHYWB( 999,   1, 6, 0, 0x4072);
		Set_GPHYWB( 999,   1, 16, 0, 0xF05E);
		Set_GPHYWB( 999,   1, 27, 0, 0xB414);

		/*
		 #=========== Cable Test =================================

		  phywb all 3 26 15-0 0x06A6
		  phywb all 3 16 15-0 0xF05E
		  phywb all 3 19 15-0 0x06EB
		  phywb all 3 18 15-0 0xF4D2
		  phywb all 3 14 15-0 0xE120
		  phywb all 3 0  15-0 0x7C00

		  phywb all 3 2  15-0 0x5FD0
		  phywb all 3 13 15-0 0x0207

		  #disable jabber detect
		   phywb all 0 16 15-0 0x05EF

		  #Patch for EEE GMII issue
		  phywb all 32 26 15-0 0x0103
		  phywb all 32 22 15-0 0x0004
		*/
		Set_GPHYWB( 999,   3, 26, 0, 0x06A6);
		Set_GPHYWB( 999,   3, 16, 0, 0xF05E);
		Set_GPHYWB( 999,   3, 19, 0, 0x06EB);
		Set_GPHYWB( 999,   3, 18, 0, 0xF4D2);
		Set_GPHYWB( 999,   3, 14, 0, 0xE120);
		Set_GPHYWB( 999,   3, 00, 0, 0x7C00);

		Set_GPHYWB( 999,   3, 02, 0, 0x5FD0);
		Set_GPHYWB( 999,   3, 13, 0, 0x0207);

		Set_GPHYWB( 999,   0, 16, 0, 0x05EF);

		Set_GPHYWB( 999,   3, 26, 0, 0x0103);
		Set_GPHYWB( 999,   3, 22, 0, 0x0004);

		/*
		 disable aldps_en, for power measurement
		 hywb all 44 21 15-0 0x0350
		*/
		Set_GPHYWB( 999,   44, 21, 0, 0x0350);
	}
	else
	{
		Set_GPHYWB(999, 0, 0, 0xffff-POWER_DOWN, POWER_DOWN); // set power down
	
		len=sizeof(default_val)/sizeof(unsigned int);
		for(i=0;i<len;i=i+3)
		{

			if(default_val[i]==999)
			{
				for(port=0; port<5; port++)
					rtl8651_setAsicEthernetPHYReg(port, default_val[i+1], default_val[i+2]);
			}
			else
			{
				port=default_val[i];
				rtl8651_setAsicEthernetPHYReg(port, default_val[i+1], default_val[i+2]);
			}
		}
		len=sizeof(phy_para)/sizeof(unsigned int);
		for(port=0; port<5; port++)
		{
			for(i=0;i<len;i=i+2)
			{
				rtl8651_setAsicEthernetPHYReg(port, phy_para[i], phy_para[i+1]);
			}
		}
		Set_GPHYWB( 999,   5, 5, 0, 0x8b84);
		Set_GPHYWB( 999,   5, 6, 0, 0x0006);
		Set_GPHYWB( 999,   2, 8, 0, 0x0020);

		// for the IOT issue with IC+ when EEE N-way.
		Set_GPHYWB( 999,   172, 24, 0, 0x0006);

#ifdef CONFIG_RTL_8198_ESD
		Set_GPHYWB(999, 44, 27, 0xffff-(0xf<<12), 0x4<<12);
#endif
	}

//	for(i=0; i<5; i++)
//		REG32(PCRP0+i*4) &= ~(EnForceMode);

	printk("==Set GPHY Parameter OK\n");
}

int rtl8198_force_giga(int port)
{
	if (port < 0 || port > 4)
		return 0;

	REG32(PCRP0+ port*4) |= (EnForceMode);

	rtl8651_setAsicEthernetPHYReg(port, 31, 0x5);
	rtl8651_setAsicEthernetPHYReg(port, 5, 0x8b86);
	rtl8651_setAsicEthernetPHYReg(port, 6, 0x0040);
	rtl8651_setAsicEthernetPHYReg(port, 31, 0x0);

	REG32(PCRP0+ port*4) = REG32(PCRP0+ port*4) & ~(EnForceMode | NwayAbility100MF | NwayAbility100MH | NwayAbility10MF | NwayAbility10MH); // disable Nway 10/100 ability

	rtl8651_restartAsicEthernetPHYNway(port);
	return 0;
}

int rtl8198_disable_force(int port)
{
	if (port < 0 || port > 4)
		return 0;

	REG32(PCRP0+ port*4) |= (EnForceMode);

	rtl8651_setAsicEthernetPHYReg(port, 31, 0x5);
	rtl8651_setAsicEthernetPHYReg(port, 5, 0x8b86);
	rtl8651_setAsicEthernetPHYReg(port, 6, 0x0000);
	rtl8651_setAsicEthernetPHYReg(port, 31, 0x0);

	REG32(PCRP0+ port*4) = (REG32(PCRP0+ port*4) & ~(EnForceMode)) | NwayAbility100MF | NwayAbility100MH | NwayAbility10MF | NwayAbility10MH; // enable Nway 10/100 ability

	rtl8651_restartAsicEthernetPHYNway(port);
	return 0;
}

#if defined(PORT5_RGMII_GMII)
unsigned int ExtP5GigaPhyMode=0;
void ProbeP5GigaPHYChip(void)
{
	unsigned int uid,tmp;
	unsigned int i;

	//printk("In Setting port5 \r\n");

	//REG32(0xB8000010)=0x01FFFCB9;

	for(i=0; i<=5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);


	/* Read */
	rtl8651_setAsicEthernetPHYReg(GIGA_P5_PHYID,0x10,0x01FE);
	rtl8651_getAsicEthernetPHYReg(GIGA_P5_PHYID, 0, &tmp );

	//printk("Read port5 phyReg0= 0x%x \r\n",tmp);

	rtl8651_getAsicEthernetPHYReg( GIGA_P5_PHYID, 2, &tmp );
	//printk("Read port5 UPChipID= 0x%x \r\n",tmp);
	uid=tmp<<16;
	rtl8651_getAsicEthernetPHYReg( GIGA_P5_PHYID, 3, &tmp );
	//printk("Read port5 downChipID= 0x%x \r\n",tmp);
	uid=uid | tmp;

	if( uid==0x001CC912 )  //0x001cc912 is 8212 two giga port , 0x001cc940 is 8214 four giga port
	{
		//printk("Find Port5   have 8211 PHY Chip! \r\n");
		ExtP5GigaPhyMode=1;
		//return 1;
	}
	else
	{
		//printk("NO Find Port5 8211 PHY Chip! \r\n");
		//ExtP5GigaPhyMode=0;
		//return 1;
	}
	for(i=0; i<=5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);

}
#endif

void disable_phy_power_down(void)
{
	int i;
	uint32 statCtrlReg0;

	for (i=0; i<5; i++)
	{
		rtl8651_getAsicEthernetPHYReg( i, 0, &statCtrlReg0 );
 
		statCtrlReg0 &= (~POWER_DOWN);
 
		/* write PHY reg 0 */
		rtl8651_setAsicEthernetPHYReg( i, 0, statCtrlReg0 );

		REG32(PCRP0+i*4) &= ~(EnForceMode);
	}
	mdelay(3000);
}
#endif

int32 rtl865x_platform_check(void)
{
	uint32 bondOptReg=0;
	bondOptReg=REG32(0xB800000C);
	if(((bondOptReg&0x0F)!=0x7) && ((bondOptReg&0x0F)!=0x9))
	{
		printk("current chip doesn't supported,system halt...\n");
		while(1);
	}

	return SUCCESS;
}

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
unsigned int Get_P0_PhyMode(void)
{
	/*
		00: External  phy
		01: embedded phy
		10: olt
		11: deb_sel
	*/
	#define GET_BITVAL(v,bitpos,pat) ((v& ((unsigned int)pat<<bitpos))>>bitpos)
	#define RANG1 1
	#define RANG2 3
	#define RANG3  7
	#define RANG4 0xf

	unsigned int v=REG32(HW_STRAP);
	unsigned int mode=GET_BITVAL(v, 6, RANG1) *2 + GET_BITVAL(v, 7, RANG1);

	return (mode&3);
}

unsigned int Get_P0_MiiMode(void)
{
	/*
		0: MII-PHY
		1: MII-MAC
		2: GMII-MAC
		3: RGMII
	*/
	#define GET_BITVAL(v,bitpos,pat) ((v& ((unsigned int)pat<<bitpos))>>bitpos)
	#define RANG1 1
	#define RANG2 3
	#define RANG3  7
	#define RANG4 0xf

	unsigned int v=REG32(HW_STRAP);
	unsigned int mode=GET_BITVAL(v, 27, RANG2);

	return mode;
}

unsigned int Get_P0_RxDelay(void)
{
	#define GET_BITVAL(v,bitpos,pat) ((v& ((unsigned int)pat<<bitpos))>>bitpos)
	#define RANG1 1
	#define RANG2 3
	#define RANG3  7
	#define RANG4 0xf

	unsigned int v=REG32(HW_STRAP);
	unsigned int val=GET_BITVAL(v, 29, RANG3);
	return val;
}

unsigned int Get_P0_TxDelay(void)
{
	#define GET_BITVAL(v,bitpos,pat) ((v& ((unsigned int)pat<<bitpos))>>bitpos)
	#define RANG1 1
	#define RANG2 3
	#define RANG3  7
	#define RANG4 0xf

	unsigned int v=REG32(HW_STRAP);
	unsigned int val=GET_BITVAL(v, 17, RANG1);
	return val;
}

int Setting_RTL8197D_PHY(void)
{
	int i;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	/*
	  page 	addr rtl8197d-default 	rtl8197d-new value Purpose
	  0 		21 	0x02c5 			0x0232 			Green: up/low bond to 3/2
	  0 		22 	0x5b85 			0x5bd5 			Green: ad current from 2 to 3
	  1 		18 	0x901c 			0x9004 			finetune AOI waveform
	  1 		19 	0x4400 			0x5400 			finetune 100M DAC current
	  1 		25 	0x00da 			0x00d0 			enable pwdn10rx at pwr saving enable snr threshold = 18dB

	  4 		16 	0x4007 			0x737f 			enable EEE, fine tune EEE parameter
	  4 		24 	0xc0a0 			0xc0f3 			change EEE wake idle to 10us
	  4 		25 	0x0130 			0x0730 			turn off tx/rx pwr at LPI state
	*/

	// only do the PHY setting in this revision IC, no need for the new one.
	if (REG32(REVR) == 0x8197C000) {

		Set_GPHYWB(999, 0, 21, 0, 0x0232);

		/* purpose: to avoid 100M N-way link fail issue Set_p="1" */
		Set_GPHYWB(999, 0, 22, 0, 0x5bd5);

	 	/* purpose: to adjust AOI waveform */
		Set_GPHYWB(999, 1, 18, 0, 0x9004);

		/* purpose: to enhance ethernet 100Mhz output voltage about 1.0(v) */
		Set_GPHYWB(999, 1, 19, 0, 0x5400);

		Set_GPHYWB(999, 1, 25, 0, 0x00d0);  //enable pwdn10rx at pwr saving enable snr threshold = 18dB

		Set_GPHYWB(999, 4, 16, 0, 0x737f);// enable EEE, fine tune EEE parameter
		Set_GPHYWB(999, 4, 24, 0, 0xc0f3);	//change EEE wake idle to 10us
		Set_GPHYWB(999, 4, 25, 0, 0x0730);	 // turn off tx/rx pwr at LPI state
	}

	// fix unlink IOT issue
	Set_GPHYWB(999, 0, 26, 0xffff-(0x1<<14), 0x0<<14);
	Set_GPHYWB(999, 0, 17, 0xffff-(0xf<<8), 0xe<<8);

	/* fine tune port on/off threshold to 160/148 */
	REG32(PBFCR0) = 0x009400A0;
	REG32(PBFCR1) = 0x009400A0;
	REG32(PBFCR2) = 0x009400A0;
	REG32(PBFCR3) = 0x009400A0;
	REG32(PBFCR4) = 0x009400A0;
	REG32(PBFCR6) = 0x009400A0;

	/* modify egress leaky bucket parameter, default inaccuracy is 5~10%, the new one is 1~2% after modification */
	REG32(ELBPCR) = 0x0000400B;
	REG32(ELBTTCR) = 0x000000C0;

	/* 100M half duplex enhancement */
	/* fix SmartBits half duplex backpressure IOT issue */
 	REG32(MACCR)= (REG32(MACCR) & ~(CF_RXIPG_MASK | SELIPG_MASK)) | (0x05 | SELIPG_11); 

	/* enlarge "Flow control DSC tolerance" from 24 pages to 48 pages
	    to prevent the hardware may drop incoming packets
	    after flow control triggered and Pause frame sent */
 	REG32(MACCR)= (REG32(MACCR) & ~CF_FCDSC_MASK) | (0x30 << CF_FCDSC_OFFSET);

	/* default enable MAC EEE */
	REG32(EEECR) = 0x28739ce7;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);

	return 0;
}
#endif

#ifdef CONFIG_RTL_8196E
int Setting_RTL8196E_PHY(void)
{
	int i;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	// write page1, reg16, bit[15:13] Iq Current 110:175uA (default 100: 125uA)
	Set_GPHYWB(999, 1, 16, 0xffff-(0x7<<13), 0x6<<13);

	// disable power saving mode in A-cut only
	if (REG32(REVR) == 0x8196e000)
		Set_GPHYWB(999, 0, 0x18, 0xffff-(1<<15), 0<<15);

	/* B-cut and later,
	    just increase a little power in long RJ45 cable case for Green Ethernet feature.
	 */
	else 
	{
		// adtune_lb setting
		Set_GPHYWB(999, 0, 22, 0xffff-(0x7<<4), 0x4<<4);
		//Setting SNR lb and hb
		Set_GPHYWB(999, 0, 21, 0xffff-(0xff<<0), 0xc2<<0);
		//auto bais current
		Set_GPHYWB(999, 1, 19, 0xffff-(0x1<<0), 0x0<<0);
		Set_GPHYWB(999, 0, 22, 0xffff-(0x1<<3), 0x0<<3);
	}

	// fix Ethernet IOT issue
	if ( ((REG32(BOND_OPTION) & BOND_ID_MASK) != BOND_8196ES)  &&
		((REG32(BOND_OPTION) & BOND_ID_MASK) != BOND_8196ES1)  &&
		((REG32(BOND_OPTION) & BOND_ID_MASK) != BOND_8196ES2)  &&
		((REG32(BOND_OPTION) & BOND_ID_MASK) != BOND_8196ES3)  )
	{
		Set_GPHYWB(999, 0, 26, 0xffff-(0x1<<14), 0x0<<14);
		Set_GPHYWB(999, 0, 17, 0xffff-(0xf<<8), 0xe<<8);
	}

#if defined(CONFIG_RTL_DISABLE_ETH_MIPS16)
	/*jwj: Refine off 140, on 152 for smartflow test.*/
	REG32(PBFCR6) = 0x008C0098;
#endif
	/* 100M half duplex enhancement */
	/* fix SmartBits half duplex backpressure IOT issue */
 	REG32(MACCR)= (REG32(MACCR) & ~(CF_RXIPG_MASK | SELIPG_MASK)) | (0x05 | SELIPG_11); 

	/* enlarge "Flow control DSC tolerance" from 24 pages to 48 pages
	    to prevent the hardware may drop incoming packets
	    after flow control triggered and Pause frame sent */
 	REG32(MACCR)= (REG32(MACCR) & ~CF_FCDSC_MASK) | (0x30 << CF_FCDSC_OFFSET);
	
	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);

	return 0;
}
#endif

#if defined(CONFIG_RTL_8198C)
// if oper = 0 (read), sramdata is ignored
void Sram98C(uint32 phyid, uint32 oper, uint32 RegAddr, uint32 sramdata)
{
	uint32 srdata=0;
	uint32 i, wphyid=0;	//start
	uint32 wphyid_end=1;   //end

	if(phyid==999)
	{	i=0;
		wphyid_end=5;    //total phyid=0~4
	}
	else
	{	i=phyid;
		wphyid_end=phyid+1;
	}

	for(; i<wphyid_end; i++)
	{
		wphyid = i;
		//change page
		if (i == 0)
			wphyid = RTL8198C_PORT0_PHY_ID;

		if (oper == 0) { // read

			rtl8651_setAsicEthernetPHYReg( wphyid, 31, 0x0a43  );
			rtl8651_setAsicEthernetPHYReg( wphyid, 27, RegAddr  );
			rtl8651_getAsicEthernetPHYReg( wphyid, 28, &srdata  );
			rtl8651_setAsicEthernetPHYReg( wphyid, 31, 0  );
			panic_printk("phy= %d, RegAddr 0x%x = 0x%x\n",wphyid, RegAddr, ((srdata & 0xff00)>>8));
		}
		else if (oper == 1) { // write

			rtl8651_setAsicEthernetPHYReg( wphyid, 31, 0x0a43  );
			rtl8651_setAsicEthernetPHYReg( wphyid, 27, RegAddr  );
			rtl8651_getAsicEthernetPHYReg( wphyid, 28, &srdata  );

			rtl8651_setAsicEthernetPHYReg( wphyid, 27, RegAddr  );
			rtl8651_setAsicEthernetPHYReg( wphyid, 28, ((sramdata << 8) | (srdata & 0x00ff)) );
			rtl8651_setAsicEthernetPHYReg( wphyid, 31, 0  );
		}		
	}
}

#if 0
// mode=0: disable; 1:enable
void giga_lite_eee(int mode)
{
	int i;

	if (mode == 1) {
		// enable 500M eee capability, page a4a, reg17, bit9=1
		Set_GPHYWB(999, 0xa4a, 17, 0xffff-(0x0200), 0x0200);

		// Enable MAC EEE ability for 500M port, 0xbb805108~1c bit12=1
		for(i=0; i<=5; i++) {
			REG32(EXTPCR0+i*4) |= (FRC_Pn_EEE_GELITE);		
		}
	}
	else if (mode == 0) {
		Set_GPHYWB(999, 0xa4a, 17, 0xffff-(0x0200), 0);

		for(i=0; i<=5; i++) {
			REG32(EXTPCR0+i*4) &= ~(FRC_Pn_EEE_GELITE);		
		}
	}
}
#endif

// mode=0: disable; 1:enable
void set_giga_lite(int mode)
{
	int i;

	giga_lite_enabled = mode;

	if (mode == 1) {
		phy_link_sts = 0;
		// enable 500M, page a42, reg20, bit7,9=11
		Set_GPHYWB(999, 0xa42, 20, 0xffff-(0x0280), 0x0280);

		// MAC set 500M ability, enable 500M bit0,1=0b11
		for(i=0; i<=5; i++) {
			//REG32(EXTPCR0+i*4) |= (CF_HW_500M_AN | CF_500M_EN);		
			//REG32(EXTPCR0+i*4) |= (CF_500M_EN);		
			REG32(EXTPCR0+i*4) &= ~(CF_HW_500M_AN | CF_500M_EN);		
		}
	}
	else if (mode == 0) {
		Set_GPHYWB(999, 0xa42, 20, 0xffff-(0x0280), 0);
	
		for(i=0; i<=5; i++) {
			REG32(EXTPCR0+i*4) &= ~(CF_HW_500M_AN | CF_500M_EN);		
		}
	}
}

#ifdef CONFIG_RTL_GIGA_LITE_REFINE
static int _8198c_link_speed = 0;

void set_giga_lite2(void)
{
	int i, phyid;
	uint32 phyData, data2;
	uint32 cur_link_sts=0;

	for(i=0; i<5; i++) {
		if (i==0)
			phyid = RTL8198C_PORT0_PHY_ID;
		else
			phyid = i;
		rtl8651_getAsicEthernetPHYReg(phyid, 1, &phyData);

		/*check link status*/
		if (phyData & (1<<2))
			cur_link_sts |= (1<<i);

		if ((phy_link_sts & (1<<i)) != (cur_link_sts & (1<<i))) {

			if (phyData & (1<<2)) {		// link up

				//Link Speed check bit4,5
				//00:10M, 01:100M, 10:1000M, 11:500M
				//page a43, reg26
				rtl8651_setAsicEthernetPHYReg(phyid, 31, 0xa43);
				rtl8651_getAsicEthernetPHYReg(phyid, 26, &data2);
				rtl8651_setAsicEthernetPHYReg(phyid, 31, 0);

				if ((data2 & 0x30) == 0x30) { // link at 500Mbps
					_8198c_link_speed |= (1<<i);
					//REG32(EXTPCR0+i*4) |= (CF_HW_500M_AN);		
					REG32(EXTPCR0+i*4) |= (CF_HW_500M_AN | CF_500M_EN);					
				}
			}
			else {		// link down

				if (_8198c_link_speed & (1<<i)) {
					//REG32(EXTPCR0+i*4) &= ~(CF_HW_500M_AN);		
					REG32(EXTPCR0+i*4) &= ~(CF_HW_500M_AN | CF_500M_EN);					
					rtl8651_restartAsicEthernetPHYNway(i);
					mdelay(100);
					_8198c_link_speed &= ~(1<<i);
				}
			}
		}
	}
	phy_link_sts = cur_link_sts;	
}
#endif

static const unsigned int phy_98c_para[]={
	
	27, 0xB820,
	28, 0x0290,
	27, 0xA012,
	28, 0x0000,
	27, 0xA014,
	28, 0x2c04,
	28, 0x2c12,
	28, 0x2c14,
	28, 0x2c14,
	28, 0x8620,
	28, 0xa480,
	28, 0x609f,
	28, 0x3084,
	28, 0x58ae,
	28, 0x2c06,
	28, 0xd710,
	28, 0x6096,
	28, 0xd71e,
	28, 0x7fa4,
	28, 0x28ae,
	28, 0x8480,
	28, 0xa101,
	28, 0x2a65,
	28, 0x8104,
	28, 0x0800,
	27, 0xA01A,
	28, 0x0000,
	27, 0xA006,
	28, 0x0fff,
	27, 0xA004,
	28, 0x0fff,
	27, 0xA002,
	28, 0x05e9,
	27, 0xA000,
	28, 0x3a5a,
	27, 0xB820,
	28, 0x0210,		
};

int phy_refine(void)
{
	int i,j,pid,ready=0,len;
	uint32 data;
	
	for(i=0;i<5;i++)
	{
		if (i==0)
			pid = RTL8198C_PORT0_PHY_ID;
		else 
			pid = i;
		
		// refine request & wait ready
		Set_GPHYWB(pid, 0xb82, 16, (0xffff & ~(1<<4)), (1<<4));

		rtl8651_setAsicEthernetPHYReg( pid, 31, 0xb80 );
		while (!ready)
		{
			rtl8651_getAsicEthernetPHYReg( pid, 16, &data );
			ready = (data >> 6) & 0x1;
		}
		rtl8651_setAsicEthernetPHYReg( pid, 31, 0);
		
		// set key & lock
		rtl8651_setAsicEthernetPHYReg( pid, 27, 0x8146);
		rtl8651_setAsicEthernetPHYReg( pid, 28, 0x4800);
		rtl8651_setAsicEthernetPHYReg( pid, 27, 0xb82e);
		rtl8651_setAsicEthernetPHYReg( pid, 28, 0x0001);

		// NCTL refine
		len = sizeof(phy_98c_para)/sizeof(unsigned int);
		for(j=0;j<len;j=j+2)
		{
			rtl8651_setAsicEthernetPHYReg(pid, phy_98c_para[j], phy_98c_para[j+1]);
		}

		// clear key & lock
		rtl8651_setAsicEthernetPHYReg( pid, 27, 0x0000);
		rtl8651_setAsicEthernetPHYReg( pid, 28, 0x0000);
		Set_GPHYWB(pid, 0xb82, 23, 0, 0x0000);
		rtl8651_setAsicEthernetPHYReg( pid, 27, 0x8146);
		rtl8651_setAsicEthernetPHYReg( pid, 28, 0x0000);
		
		// refine release & wait ready
		Set_GPHYWB(pid, 0xb82, 16, (0xffff & ~(1<<4)), (0<<4));

		rtl8651_setAsicEthernetPHYReg( pid, 31, 0xb80 );
		while (ready)
		{
			rtl8651_getAsicEthernetPHYReg( pid, 16, &data );
			ready = (data >> 6) & 0x1;
		}
		rtl8651_setAsicEthernetPHYReg( pid, 31, 0);
	}

	return 0;
}

#ifdef RTL8198C_GPHY_CALIBRATION
static const unsigned int phy_98c_ado[]={	
    27, 0x83de,
    28, 0xaf83,
    28, 0xeaaf,
    28, 0x83ed,
    28, 0xaf83,
    28, 0xf0af,
    28, 0x83f3,
    28, 0xaf0c,
    28, 0x0caf,
    28, 0x83ed,
    28, 0xaf83,
    28, 0xf0af,
    28, 0x83f3,
    27, 0xb818,
    28, 0x0bf1,
    27, 0xb81a,
    28, 0xfffd,
    27, 0xb81c,
    28, 0xfffd,
    27, 0xb81e,
    28, 0xfffd,
    27, 0xb832,
    28, 0x0001,
};

int Ado_RamCode_Efuse(int phyport)
{
	unsigned int readReg;
	int j , len;
	//#Patch request
	//#POLL	a46	21	10	8	3
	//#wr 0xB82 rg16[4] = 1     // set patch_req
	//#polling 0xB80 rg16[6] = 1   // patch rdy

	while(1)
	{
		rtl8651_setAsicEthernetPHYReg( phyport, 31, 0xa46 );
		rtl8651_getAsicEthernetPHYReg( phyport, 21, &readReg );
		rtl8651_setAsicEthernetPHYReg( phyport, 31, 0);

		if(((readReg >> 8) & 7) == 3)
			break;
	}

	Set_GPHYWB(phyport, 0xb82, 16, (0xffff & ~(1<<4)), (1<<4));

	while(1)
	{
		rtl8651_setAsicEthernetPHYReg( phyport, 31, 0xb80 );
		rtl8651_getAsicEthernetPHYReg( phyport, 16, &readReg );
		rtl8651_setAsicEthernetPHYReg( phyport, 31, 0);

		if(((readReg >> 6) & 1) == 1)
			break;
	}

	len = sizeof(phy_98c_ado)/sizeof(unsigned int);
	for(j=0;j<len;j=j+2)
	{
		Set_GPHYWB(phyport, 0, phy_98c_ado[j], 0, phy_98c_ado[j+1]);
	}

	//#Patch request clear
	//#wr 0xB82 rg16[4] = 0
	Set_GPHYWB(phyport, 0xb82, 16, (0xffff & ~(1<<4)), (0<<4));
	return 0;
}
#endif

void ado_default(void)
{
	int phyport, i;

	for(i=0;i<5;i++)
	{
		if (i==0)
			phyport = 8;
		else 
			phyport = i;

		Ado_RamCode_Efuse(phyport);
		
		//#adc_ioffset adjustment
		Set_GPHYWB(phyport, 0xbcf, 22, 0, 0x7777);
		Set_GPHYWB(phyport, 0, 0, ~(1<<9), (1<<9));
	}
}

static const unsigned int phy_ado_patch[]={	
	27, 0x83de,
	28, 0xaf83,
	28, 0xeaaf,
	28, 0x83f1,
	28, 0xaf83,
	28, 0xf4af,
	28, 0x8438,
	28, 0x0d42,
	28, 0x583f,
	28, 0xaf32,
	28, 0x53af,
	28, 0x0c0c,
	28, 0xe482,
	28, 0x4502,
	28, 0x83fd,
	28, 0xaf30,
	28, 0xc9f8,
	28, 0xf9fa,
	28, 0xef69,
	28, 0xfafb,
	28, 0xbf87,
	28, 0xf7d1,
	28, 0x08ec,
	28, 0x0019,
	28, 0xb1fb,
	28, 0xd204,
	28, 0x82d6,
	28, 0x0000,
	28, 0xd380,
	28, 0x0233,
	28, 0x311a,
	28, 0x67b3,
	28, 0xf90d,
	28, 0x67ef,
	28, 0x120c,
	28, 0x11bf,
	28, 0x87f7,
	28, 0x1a91,
	28, 0xef46,
	28, 0xdc19,
	28, 0xdda2,
	28, 0x00e0,
	28, 0xfffe,
	28, 0xef96,
	28, 0xfefd,
	28, 0xfc04,
	28, 0xaf84,
	28, 0x3800,
	27, 0xb818,
	28, 0x3251,
	27, 0xb81a,
	28, 0x0bf1,
	27, 0xb81c,
	28, 0x30c6,
	27, 0xb81e,
	28, 0xfffd,
	27, 0xb832,
	28, 0x0007,
	31, 0x0000,
};  

void Ado_RamCode(int phyport)
{   
	unsigned int readReg;
	int j , len;
	//#Patch request
	//#POLL	a46	21	10	8	3
	//#wr 0xB82 rg16[4] = 1     // set patch_req
	//#polling 0xB80 rg16[6] = 1   // patch rdy
    
	while(1)
	{
		rtl8651_setAsicEthernetPHYReg( phyport, 31, 0xa46 );
		rtl8651_getAsicEthernetPHYReg( phyport, 21, &readReg );
		rtl8651_setAsicEthernetPHYReg( phyport, 31, 0);
    
		if(((readReg >> 8) & 7) == 3)
			break;
	}
    
	Set_GPHYWB(phyport, 0xb82, 16, (0xffff & ~(1<<4)), (1<<4));
    
	while(1)
	{
		rtl8651_setAsicEthernetPHYReg( phyport, 31, 0xb80 );
		rtl8651_getAsicEthernetPHYReg( phyport, 16, &readReg );
		rtl8651_setAsicEthernetPHYReg( phyport, 31, 0);

		if(((readReg >> 6) & 1) == 1)
			break;
	}

	//#Load patch code
	len = sizeof(phy_ado_patch)/sizeof(unsigned int);
	for(j=0;j<len;j=j+2)
	{
		Set_GPHYWB(phyport, 0, phy_ado_patch[j], 0, phy_ado_patch[j+1]);
	}

	//#Patch request clear
	//#wr 0xB82 rg16[4] = 0
	Set_GPHYWB(phyport, 0xb82, 16, (0xffff & ~(1<<4)), (0<<4));
}

#define _ADC_CAL(ASF, A) \
	if((A >> 8) == 1) \
		ASF =  7 - ((256 - (A & 0xFF))  / 9); \
	else \
		ASF =  8 + ((A & 0xFF)  / 9);
	
void Adc_Bias_Cal(int phyport)
{
	unsigned int adc_biasA,adc_biasB,adc_biasC,adc_biasD;
	unsigned int adc_biasAsf;
	unsigned int adc_mdf=0, readReg;

	//#adc-bias calculate enable
	//#	wr 0xA47 rg17[9] = 1
	Set_GPHYWB(phyport, 0xA47, 17, (0xffff & ~(1<<9)), (1<<9));
	//#Polling adc-bias calculate done
	//#    polling 0xA47 rg17[9] = 0
	while(1)
	{
		rtl8651_setAsicEthernetPHYReg( phyport, 31, 0xA47 );
		rtl8651_getAsicEthernetPHYReg( phyport, 17, &readReg );
		rtl8651_setAsicEthernetPHYReg( phyport, 31, 0);

		if(((readReg >> 9) & 1) == 0)
			break;
	}
	
	//#adc-bias calculate dump
	//#   wr 0xA43 rg27 = 0x87f7
	//#   rd 0xA43 rg28[8:0]      // read out_int
	//#// adc_bias_p0 = (out_int - floor(out_int/2^8)*2^9)/2^8; % s9.8f, uc cal ado_dat of portA's avg of 128 point, get portA adc_bias
	rtl8651_setAsicEthernetPHYReg( phyport, 27, 0x87f7 );
	rtl8651_getAsicEthernetPHYReg( phyport, 28, &readReg );
	adc_biasA = readReg & 0x1ff;
	
	//#   wr 0xA43 rg27 = 0x87f9
	//#   rd 0xA43 rg28[8:0]      // read out_int
	//#// adc_bias_p0 = (out_int - floor(out_int/2^8)*2^9)/2^8; % s9.8f, uc cal ado_dat of portA's avg of 128 point, get portB adc_bias
	rtl8651_setAsicEthernetPHYReg( phyport, 27, 0x87f9 );
	rtl8651_getAsicEthernetPHYReg( phyport, 28, &readReg );
	adc_biasB = readReg & 0x1ff;
	
	//#   wr 0xA43 rg27 = 0x87fb
	//#   rd 0xA43 rg28[8:0]      // read out_int
	//#// adc_bias_p0 = (out_int - floor(out_int/2^8)*2^9)/2^8; % s9.8f, uc cal ado_dat of portA's avg of 128 point, get portC adc_bias
	rtl8651_setAsicEthernetPHYReg( phyport, 27, 0x87fb );
	rtl8651_getAsicEthernetPHYReg( phyport, 28, &readReg );
	adc_biasC = readReg & 0x1ff;
	
	//#   wr 0xA43 rg27 = 0x87fd
	//#   rd 0xA43 rg28[8:0]      // read out_int
	//#// adc_bias_p0 = (out_int - floor(out_int/2^8)*2^9)/2^8; % s9.8f, uc cal ado_dat of portA's avg of 128 point, get portD adc_bias
	rtl8651_setAsicEthernetPHYReg( phyport, 27, 0x87fd );
	rtl8651_getAsicEthernetPHYReg( phyport, 28, &readReg );
	adc_biasD = readReg & 0x1ff;

	_ADC_CAL(adc_biasAsf, adc_biasA);
	adc_mdf |= adc_biasAsf;		

	_ADC_CAL(adc_biasAsf, adc_biasB);
	adc_mdf |= (adc_biasAsf << 4) ;		

	_ADC_CAL(adc_biasAsf, adc_biasC);
	adc_mdf |= (adc_biasAsf << 8);	

	_ADC_CAL(adc_biasAsf, adc_biasD);
	adc_mdf |= (adc_biasAsf << 12);	

	//#adc_ioffset adjustment
	//#1. Ioffset, 4 port's ioffset_pX will set to page 0xBCF reg 22
	Set_GPHYWB(phyport, 0xbcf, 22, 0, adc_mdf);
}

void force_giga_slave(unsigned int pid)
{
	/*
	% dec2hex(mcu_wr('8010','f77b')) % disable force 1G
	%%% disable force 1G, turn off internal n-way %%%
	dec2hex(reg_pw('a43', 27, 15,0,'8010'));
	dec2hex(reg_pw('a43', 28, 11,11,'0'));
	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	dec2hex(reg_pw('a46', 21, 1,1,'1'));% lock main

	pcs_state = '0'; 
	while (~strcmp(pcs_state , '1'))
	 pcs_state = dec2hex(reg_pr('0a60',16,7,0));
	 disp(sprintf('pcs_state = %s', pcs_state ));
	end

	dec2hex(reg_pw('a40', 0, 15,0,'0140'));% force 1G
	dec2hex(reg_pw('a4a', 19, 7,6,'2')); %force slave
	dec2hex(reg_pw('a44', 20, 2,2,'1'));%frc tp1
	dec2hex(reg_pw('a46', 21, 1,1,'0'));%release lock main
	*/
	unsigned int readReg;

	Set_GPHYWB(pid, 0xa43, 27, 0, 0x8010);
	Set_GPHYWB(pid, 0xa43, 28, (0xffff & ~(1<<11)), (0<<11));
	Set_GPHYWB(pid, 0xa46, 21, (0xffff & ~(1<<1)), (1<<1));

	while(1)
	{
		rtl8651_setAsicEthernetPHYReg( pid, 31, 0xa60 );
		rtl8651_getAsicEthernetPHYReg( pid, 16, &readReg );
		rtl8651_setAsicEthernetPHYReg( pid, 31, 0);
		if((readReg & 0xff) == 1)
			break;
	}
	
	Set_GPHYWB(pid, 0xa40, 0, 0, 0x0140);
	Set_GPHYWB(pid, 0xa4a, 19, (0xffff & ~(3<<6)), (2<<6));
	Set_GPHYWB(pid, 0xa44, 20, (0xffff & ~(1<<2)), (1<<2));
	Set_GPHYWB(pid, 0xa46, 21, (0xffff & ~(1<<1)), (0<<1));
}

void Release_Force_slave(unsigned int pid)
{
	/*
	wr 0xA43 rg27 = 0x8010
	wr 0xA43 rg28 = 0xFF77   // set bit11=1 
	wr 0xA46 rg21[1] = 1     // lock main
	wr 0xA40 rg16 = 0x1040   // disable force 1G, external n-way on 
	wr 0xA4A rg19[7:6] = 2'b 00 // disable force slave
	wr 0xA44 rg20[2] = 1'b0   // disable frc tp1
	wr 0xA46 rg21[1] = 1'b0   // release main
	*/
	Set_GPHYWB(pid, 0xa43, 27, 0, 0x8010);
	Set_GPHYWB(pid, 0xa43, 28, (0xffff & ~(1<<11)), (1<<11));
	Set_GPHYWB(pid, 0xa46, 21, (0xffff & ~(1<<1)), (1<<1));
	
	Set_GPHYWB(pid, 0xa40, 0, 0, 0x1040);
	Set_GPHYWB(pid, 0xa4a, 19, (0xffff & ~(3<<6)), (0<<6));
	Set_GPHYWB(pid, 0xa44, 20, (0xffff & ~(1<<2)), (0<<2));
	Set_GPHYWB(pid, 0xa46, 21, (0xffff & ~(1<<1)), (0<<1));
}

void ado_modified(void)
{
	int phyport, i;
	
	for(i=0;i<5;i++)
	{
		if (i==0)
			phyport = 8;
		else 
			phyport = i;

		//#Initial
		//#1. Green disable
		//#	wr 0xA43 rg27 = 0x8011
		//# wr 0xA43 rg28[15] = 0    (default f777)
		Set_GPHYWB(phyport, 0xa43, 27, 0, 0x8011);
		Set_GPHYWB(phyport, 0xa43, 28, (0xffff & ~(1<<15)), (0<<15));
		//#3. set slave_sd_thd to MAX
		//#      wr 0xA43 rg27 = 0x8120
		//#      wr 0xA43 rg28 = 0xff00	
		Set_GPHYWB(phyport, 0xa43, 27, 0, 0x8120);
		Set_GPHYWB(phyport, 0xa43, 28, 0, 0xff00);
		//#4. Force PGA
		//#      wr 0xA81 rg16[4:0] = 5'b0     //external PGA level selection
		//#      wr 0xa81 rg16[9:6] = 4'b1111   //force aagc_code to aagc_reg.
		Set_GPHYWB(phyport, 0xa81, 16, (0xffff & ~(0x1f<<0)), (0<<0));
		Set_GPHYWB(phyport, 0xa81, 16, (0xffff & ~(0xf<<6)), (0xf<<6));

		//#Load patch code
		Ado_RamCode(phyport);

		//#Force_slave_TP1
		force_giga_slave(phyport);

		//#adc-bias calculate enable
		//#adc_ioffset adjustment
		Adc_Bias_Cal(phyport);

		//#Release_Force_slave_TP1
		Release_Force_slave(phyport);

		//# backfill
		//#1. set slave_sd_thd to default
		//#      wr 0xA43 rg27 = 0x8120
		//#      wr 0xA43 rg28 = 0x0e00
		rtl8651_setAsicEthernetPHYReg( phyport, 27, 0x8120 );
		rtl8651_setAsicEthernetPHYReg( phyport, 28, 0x0e00);
		//#2. Release Force PGA
		//#      wr 0xA81 rg16 = 0x0000  //  set bit[4:0]=5b'0 & set bit[9:6]=4b'0
		Set_GPHYWB(phyport, 0xA81, 16, 0, 0);
		//#3. Green enable 
		//#	wr 0xA43 rg27 = 0x8011
		//#	wr 0xa43 rg28 = 0xf777    // set bit15=1
		rtl8651_setAsicEthernetPHYReg( phyport, 27, 0x8011 );
		rtl8651_setAsicEthernetPHYReg( phyport, 28, 0xf777);
		//#
		//#5. Reset phy
		//#wr 0xA40 rg16 = 0x9200    //Phy reset
		Set_GPHYWB(phyport, 0xA40, 16, 0, 0x9200);
	}	
}

void ado_setting(int mode)
{
	if (mode == 0)
		ado_default();
	else if (mode == 1)
		ado_modified();		
}

static const unsigned int phy_ado_data[]={	
	27, 0x83de,
	28, 0xaf83,
	28, 0xeaaf,
	28, 0x83ed,
	28, 0xaf85,
	28, 0x85af,
	28, 0x8588,
	28, 0xaf0c,
	28, 0x0cf6,
	28, 0x0102,
	28, 0x83f5,
	28, 0xaf00,
	28, 0x8ff8,
	28, 0xf9cd,
	28, 0xf9fa,
	28, 0xef69,
	28, 0xfafb,
	28, 0xe080,
	28, 0x13ac,
	28, 0x2303,
	28, 0xaf85,
	28, 0x21d1,
	28, 0x00bf,
	28, 0x8597,
	28, 0x0241,
	28, 0x05d1,
	28, 0x0fbf,
	28, 0x859a,
	28, 0x0241,
	28, 0x05bf,
	28, 0x859d,
	28, 0x0245,
	28, 0xbbbf,
	28, 0x85a0,
	28, 0x0245,
	28, 0xb3bf,
	28, 0x85a3,
	28, 0x0245,
	28, 0xb3bf,
	28, 0x85a6,
	28, 0x0245,
	28, 0xb3ee,
	28, 0x87f4,
	28, 0x001f,
	28, 0x44e0,
	28, 0x87f4,
	28, 0xbfa8,
	28, 0xc4ef,
	28, 0x591a,
	28, 0x54e6,
	28, 0x87f5,
	28, 0xe787,
	28, 0xf6e1,
	28, 0x87f4,
	28, 0xd000,
	28, 0xbf85,
	28, 0x8b4c,
	28, 0x0003,
	28, 0x1a49,
	28, 0xe487,
	28, 0xf7e5,
	28, 0x87f8,
	28, 0xee87,
	28, 0xf907,
	28, 0xee87,
	28, 0xfaff,
	28, 0xee87,
	28, 0xfb00,
	28, 0xee87,
	28, 0xfc10,
	28, 0xee87,
	28, 0xfd00,
	28, 0x0285,
	28, 0x2bef,
	28, 0x67ad,
	28, 0x5f03,
	28, 0x7fff,
	28, 0xffe2,
	28, 0x87f9,
	28, 0xe387,
	28, 0xfa09,
	28, 0xef56,
	28, 0xef67,
	28, 0x0bc6,
	28, 0x0244,
	28, 0xf4ad,
	28, 0x5015,
	28, 0xcce4,
	28, 0x87f9,
	28, 0xe587,
	28, 0xfae0,
	28, 0x87f7,
	28, 0xe187,
	28, 0xf8ef,
	28, 0x9402,
	28, 0x4143,
	28, 0xe587,
	28, 0xfee1,
	28, 0x87fd,
	28, 0x3904,
	28, 0x9e38,
	28, 0xe187,
	28, 0xfd11,
	28, 0xe587,
	28, 0xfde0,
	28, 0x87f7,
	28, 0xe187,
	28, 0xf8ef,
	28, 0x9402,
	28, 0x4143,
	28, 0xad37,
	28, 0x08e5,
	28, 0x87fc,
	28, 0xe087,
	28, 0xfbae,
	28, 0x06e5,
	28, 0x87fb,
	28, 0xe087,
	28, 0xfc1a,
	28, 0x100d,
	28, 0x11d0,
	28, 0x0008,
	28, 0xe087,
	28, 0xf7e1,
	28, 0x87f8,
	28, 0xef94,
	28, 0x0802,
	28, 0x4105,
	28, 0xae8e,
	28, 0xe187,
	28, 0xfed0,
	28, 0x0008,
	28, 0xe087,
	28, 0xf7e1,
	28, 0x87f8,
	28, 0xef94,
	28, 0x0802,
	28, 0x4105,
	28, 0xe187,
	28, 0xf439,
	28, 0x039e,
	28, 0x0ae1,
	28, 0x87f4,
	28, 0x11e5,
	28, 0x87f4,
	28, 0xaf84,
	28, 0x33d1,
	28, 0x00bf,
	28, 0x859a,
	28, 0x0241,
	28, 0x05bf,
	28, 0x85a6,
	28, 0x0245,
	28, 0xbbbf,
	28, 0x859d,
	28, 0x0245,
	28, 0xb3bf,
	28, 0x85a0,
	28, 0x0245,
	28, 0xbbbf,
	28, 0x85a3,
	28, 0x0245,
	28, 0xbbff,
	28, 0xfeef,
	28, 0x96fe,
	28, 0xfdc5,
	28, 0xfdfc,
	28, 0x04f8,
	28, 0xf9fa,
	28, 0xef69,
	28, 0xfa1f,
	28, 0x441f,
	28, 0x77e2,
	28, 0x87f5,
	28, 0xe387,
	28, 0xf6ef,
	28, 0x95e2,
	28, 0x87f3,
	28, 0x1f66,
	28, 0x160c,
	28, 0x61b2,
	28, 0xfcda,
	28, 0x19db,
	28, 0x890c,
	28, 0x570d,
	28, 0x581a,
	28, 0x45ef,
	28, 0x54ef,
	28, 0x323a,
	28, 0x7f9e,
	28, 0x063b,
	28, 0x809e,
	28, 0x08ae,
	28, 0x0a17,
	28, 0x5c00,
	28, 0xffae,
	28, 0x0487,
	28, 0x6cff,
	28, 0x00b6,
	28, 0xdc4f,
	28, 0x007f,
	28, 0x0d48,
	28, 0x1a74,
	28, 0xe087,
	28, 0xf338,
	28, 0x089e,
	28, 0x050d,
	28, 0x7180,
	28, 0xaef9,
	28, 0xfeef,
	28, 0x96fe,
	28, 0xfdfc,
	28, 0x04af,
	28, 0x8585,
	28, 0xaf85,
	28, 0x8830,
	28, 0xbcfc,
	28, 0x74bc,
	28, 0xfcb8,
	28, 0xbcfc,
	28, 0xfcbc,
	28, 0xfc40,
	28, 0xa810,
	28, 0x96a8,
	28, 0x10aa,
	28, 0xbcd2,
	28, 0xccbc,
	28, 0xd2ee,
	28, 0xbcd2,
	28, 0xffbc,
	28, 0xd200,
	27, 0xb818,
	28, 0x0bf1,
	27, 0xb81a,
	28, 0x008d,
	27, 0xb81c,
	28, 0xfffd,
	27, 0xb81e,
	28, 0xfffd,
	27, 0xb832,
	28, 0x0003,
	31, 0x0000,
	27, 0x87f2,
	28, 0x000f,	
};  

int phy_ready(int phyport)
{
	uint32 readReg, count=0;
	
	while(1)
	{
		rtl8651_setAsicEthernetPHYReg( phyport, 31, 0xa46 );
		rtl8651_getAsicEthernetPHYReg( phyport, 21, &readReg );
		rtl8651_setAsicEthernetPHYReg( phyport, 31, 0);

		if(((readReg >> 8) & 7) == 2)
			return 1;
		else if((count++) >= 10) 
			return 0;
	}
}

/*
 must do this adc refinement in phy_state 2 (page 0xa46, reg 21, bit 9-8),
 so put ado_refine() in bsp_swcore_init() of setup.c.
 if we do ado_refine() in re865x_probe(), the phy_state will be 3.
 */
void ado_refine(void)
{
	int i,phyport;
	int j, len;
	uint32 readReg;
	unsigned int efuse_value[5] ;

	REG32(SYS_CLK_MAG) |= CM_ACTIVE_SWCORE;

	for(i=0;i<5;i++) {
		REG32(PCRP0+i*4) |= (EnForceMode);
		efuse_value[i] = 0;
	}
	
	if ((REG32(REVR) & 0xfff) > 0) {
		REG32(EFUSE_TIMING_CONTROL)=0x030c174f; 
		REG32(EFUSE_CONFIG)=0x00040112; 

		for(i=0; i<5; i++) {
			REG32(EFUSE_CONFIG) |= EFUSE_CFG_INT_STS;		/* clear efuse interrupt status bit */
			REG32(EFUSE_CMD) = i;
			while( ( REG32(EFUSE_CONFIG) & EFUSE_CFG_INT_STS ) == 0 );		/* Wait efuse_interrupt */
			efuse_value[i] = REG32(EFUSE_RW_DATA);
		}
	}

	rtl8651_getAsicEthernetPHYReg( RTL8198C_PORT0_PHY_ID, 0, &readReg );
	
	for(i=0;i<5;i++)
	{
		if (i==0)
			phyport = RTL8198C_PORT0_PHY_ID;
		else 
			phyport = i;
		
		rtl8651_getAsicEthernetPHYReg( phyport, 0, &readReg );
		
		if((readReg & POWER_DOWN) == 0) // power_down=1, check next condition
			continue;
		
		if ((phy_ready(phyport)) == 0) // phy state=2, check next condition
			continue;

		if ((efuse_value[i] & 0x80))   // bit7=0, efuse does not be calibrated, do boot up K
			continue;

		/* boot up K:
		 * Activate IC internal mechanism to calibrate "ADC-bias offset" value.
		 * hardware will auto adopt the new value to GPHY register.
		 */
		len = sizeof(phy_ado_data)/sizeof(unsigned int);
		for(j=0;j<len;j=j+2)
		{
			Set_GPHYWB(phyport, 0, phy_ado_data[j], 0, phy_ado_data[j+1]);
		}	
	}
}

int Setting_RTL8198C_GPHY(void)
{
	int i, phyid;
	unsigned int data;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

#if 0
	// phy power down
	Set_GPHYWB(999, 0, 0, 0, 0x1900);
	// reset phy register
	Set_GPHYWB(999, 0xc41, 20, 0, 1);
	mdelay(50);	
	Set_GPHYWB(999, 0, 0, 0, 0x8000);
	mdelay(300);	
#endif

	phy_refine();

	// to disable the PCS INT (GPHY PCS interrupt (MAC_INTRUPT  signal )). 
	Set_GPHYWB(999, 0xa42, 18, 0, 0);

	// read to clear INT status. 
	for(i=0; i<5; i++) {
		phyid = rtl8651AsicEthernetTable[i].phyId;		
		
		rtl8651_setAsicEthernetPHYReg( phyid, 31, 0xa42);
		rtl8651_getAsicEthernetPHYReg( phyid, 29, &data);		
		rtl8651_setAsicEthernetPHYReg( phyid, 31, 0);
	}

	// fix MDI 10pf codedsp shift
	Set_GPHYWB(999, 0xBCD, 21, 0, 0x2222);	// codedsp register = 2
	Set_GPHYWB(999, 0, 27, 0, 0x8277);
	Set_GPHYWB(999, 0, 28, 0xffff - 0xff00, 0x02 << 8);	// uc codedsp = 2

	// giga master sd_thd
	Set_GPHYWB(999, 0, 27, 0, 0x8101);
	Set_GPHYWB(999, 0, 28, 0, 0x4000);

	/* to enable auto down speed mechanism when INRX detect 2-pair cable connected */
	Set_GPHYWB(999, 0xa44, 17, 0xffff-(1<<2), 1<<2);

	/* enlarge "Flow control DSC tolerance" from 36 pages to 48 pages
	    to prevent the hardware may drop incoming packets
	    after flow control triggered and Pause frame sent */
 	REG32(MACCR)= (REG32(MACCR) & ~CF_FCDSC_MASK) | (0x30 << CF_FCDSC_OFFSET);

	/* set "System clock selection" to 100MHz 
		2'b00: 50MHz, 2'b01: 100MHz, 2'b10, 2'b11 reserved
	*/
 	REG32(MACCR)= (REG32(MACCR) & ~(CF_SYSCLK_SEL_MASK)) | (0x01 << CF_SYSCLK_SEL_OFFSET); 

	/* set "Cport MAC clock selection with NIC interface" to lx_clk */
 	REG32(MACCTRL1) |= CF_CMAC_CLK_SEL;

#ifdef RTL8198C_GPHY_CALIBRATION
	{
	unsigned int efuse_value[5] ;
	unsigned int cali_rc[5] ; // bit 31 ~ 28
	unsigned int cali_r[5] ; // bit 27 ~ 24

	REG32(EFUSE_TIMING_CONTROL)=0x030c174f; 
	REG32(EFUSE_CONFIG)=0x00040112; 

	//read efuse
	for(i=0; i<5; i++) {

		REG32(EFUSE_CONFIG) |= EFUSE_CFG_INT_STS;		/* clear efuse interrupt status bit */
		REG32(EFUSE_CMD) = i;
		while( ( REG32(EFUSE_CONFIG) & EFUSE_CFG_INT_STS ) == 0 );		/* Wait efuse_interrupt */
		efuse_value[i] = REG32(EFUSE_RW_DATA);
		cali_rc[i] = (efuse_value[i] >> 28) & 0xF;
		cali_r[i] = (efuse_value[i] >> 24) & 0xF;
	}

 	// to backfill RC & R value
	for(i=0; i<5; i++) {
		phyid = rtl8651AsicEthernetTable[i].phyId;		
		
		rtl8651_setAsicEthernetPHYReg( phyid, 31, 0xbcd);

		if ((cali_rc[i] != 0) && (cali_rc[i] != 0xF)) {
			rtl8651_setAsicEthernetPHYReg( phyid, 0x16, (cali_rc[i] | (cali_rc[i] << 4) | (cali_rc[i] << 8) | (cali_rc[i] << 12)));
			rtl8651_setAsicEthernetPHYReg( phyid, 0x17, (cali_rc[i] | (cali_rc[i] << 4) | (cali_rc[i] << 8) | (cali_rc[i] << 12)));
		}
			
		rtl8651_setAsicEthernetPHYReg( phyid, 31, 0xbce);

		if ((cali_r[i] != 0) && (cali_r[i] != 0xF)) {
			rtl8651_setAsicEthernetPHYReg( phyid, 0x10, (cali_r[i] | (cali_r[i] << 4) | (cali_r[i] << 8) | (cali_r[i] << 12)));
			rtl8651_setAsicEthernetPHYReg( phyid, 0x11, (cali_r[i] | (cali_r[i] << 4) | (cali_r[i] << 8) | (cali_r[i] << 12)));
		}

		rtl8651_setAsicEthernetPHYReg( phyid, 31, 0);
	}

	for(i=0;i<5;i++)
	{
		if ((efuse_value[i] & 0x80)) { // bit7 = 1, efuse has been calibrated
		 	// to backfill ADC-bias offset value
			phyid = rtl8651AsicEthernetTable[i].phyId;		
			Ado_RamCode_Efuse(phyid);								
			Set_GPHYWB(i, 0xbcf, 22, 0, (efuse_value[i]>>8)&0xffff);
		}
	}
	}
#endif

#ifdef ENABLE_8198C_GREEN
	//giga Green setting
	Sram98C(999, 1, 0x809a, 0x89);
	Sram98C(999, 1, 0x809b, 0x11);
	Sram98C(999, 1, 0x80a3, 0x92);
	Sram98C(999, 1, 0x80a4, 0x33);
	Sram98C(999, 1, 0x80a0, 0x0);

	//100M Green setting
	Sram98C(999, 1, 0x8088, 0x89);
	Sram98C(999, 1, 0x8089, 0x11);
	Sram98C(999, 1, 0x808e, 0x0);
#else
	/* disable Green feature (Link-On and Cable Length Power Saving), provide by JiroJiang */
	Sram98C(999, 1, 0x809a, 0xa4);
	Sram98C(999, 1, 0x809b, 0x44);
	Sram98C(999, 1, 0x80a3, 0xa4);
	Sram98C(999, 1, 0x80a4, 0x44);
	Sram98C(999, 1, 0x80a0, 0x23);
	Sram98C(999, 1, 0x8088, 0xa4);
	Sram98C(999, 1, 0x8089, 0x44);
	Sram98C(999, 1, 0x808e, 0x23);
#endif

#ifdef CONFIG_RTL_GIGA_LITE_ENABLED
	set_giga_lite(1);
#endif

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);

	return 0;
}
#endif

#if defined(CONFIG_RTL_8881A)
int Setting_RTL8881A_PHY(void)
{
	int i;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);
#if 0
		// Page1, Reg23, Vref[7:6] 10
		Set_GPHYWB(999, 1, 23, 0xffff-(3<<6), 2<<6);

		// Page1, Reg16, cf1[6:4] 110->001
		Set_GPHYWB(999, 1, 16, 0xffff-((7<<4)), ((1<<4)));

        Set_GPHYWB(999,1, 29, 0xFFFE, 0x0);
		mdelay(1);
		Set_GPHYWB(999,0, 0, 0xffff, 0x200);
		mdelay(1);
        Set_GPHYWB(999,1, 29, 0xffff, 0x1);
	    mdelay(1);	
		Set_GPHYWB(999,0, 21, 0xff00, (1<<1|1<<4|1<<5));
		mdelay(1);
#else
if(((REG32(0xb8000000)&0xff)>0))
{
	#define PLL2 0xb8000058
	REG32(PLL2 )|=(5<<6);//20120906:Set RBG_L[8:6]=101 from RDC suggestion

	Set_GPHYWB(999, 1, 23, 0xffff-(3<<6), 2<<6);//SwitchPatch #2 Page1,Reg.23[7:6] = 10
    mdelay(1);
	Set_GPHYWB(999, 1, 16, 0xffff-(7<<4), 1<<4);//SwitchPatch #3 Page1,Reg.16[6:4] = 001
    mdelay(1);
	//Set_GPHYWB(999, 1, 29, 0xffff-(1<<0), 0<<0);//SwitchPatch #4 Page1,Reg.29[0] = 0 //A cut don't need it
	//Set_GPHYWB(999, 0, 0, 0xffff-(1<<9), 1<<9);//SwitchPatch #4 Page0,Reg.0[9] = 1 //A cut don't need it
	//Set_GPHYWB(999, 1, 29, 0xffff-(1<<0), 1<<0);//SwitchPatch #4 Page1.Reg.29[0] =1 //A cut don't need it

	//write page 31 = reg4 to check the PHY parameter

	Set_GPHYWB(999, 4, 16, 0, 0x4077);//SwitchPatch #5 Page4,Reg.16 = 0x4077
    mdelay(1);
	Set_GPHYWB(999, 4, 24, 0, 0xc0f3);//SwitchPatch #6 Page4,Reg.24 = 0xc0f3
    mdelay(1);
	Set_GPHYWB(999, 4, 25, 0, 0x7630);//SwitchPatch #7 Page4,Reg.25 = 0x7630
    mdelay(1);
	//REG32( 0xBB804160)=0x0E739CE7 ;//SwitchPatch #8 BB804160 = 0x0E739CE7 ,Set EEE MAC register
	REG32( 0xBB804160)=0x0 ;//SwitchPatch #8 BB804160 = 0x0E739CE7 ,Set EEE MAC register
    mdelay(1);
	Set_GPHYWB(999, 0, 21, 0xffff-(15<<4), 2<<4);//Patch #9 Page0,Reg.21[7:4]=0010  ,Set Green ethernet
    mdelay(1);
	Set_GPHYWB(999, 0, 21, 0xffff-(15<<0), 1<<0);//Patch #10 Page0,Reg.21[3:0]=0001  ,Set Green ethernet
    mdelay(1);
	Set_GPHYWB(999, 1, 19, 0xffff-(3<<3), 1<<3);//Patch #11 Page1,Reg.19[4:3]=01  ,....corner internetl SWR Pass 1.12V
    mdelay(1);
	Set_GPHYWB(999, 1, 17, 0xffff-(1<<14), 1<<14);//Patch #12 Page1,Reg.17[14]=1  ,Fix Low SNR issue.
    mdelay(1);
	Set_GPHYWB(999, 1, 16, 0xffff-(3<<12), 0<<0);//Patch #13 Page1,Reg.16[13:12]=00  ,IOL For 10M MAU
    mdelay(1);
}
else
{
	Set_GPHYWB(999, 1, 23, 0xffff-(3<<6), 2<<6);//SwitchPatch #2 Page1,Reg.23[7:6] = 10
    mdelay(1);
	Set_GPHYWB(999, 1, 16, 0xffff-(7<<4), 1<<4);//SwitchPatch #3 Page1,Reg.16[6:4] = 001
    mdelay(1);
	Set_GPHYWB(999, 1, 29, 0xffff-(1<<0), 0<<0);//SwitchPatch #4 Page1,Reg.29[0] = 0 //A cut don't need it
	mdelay(1);
	Set_GPHYWB(999, 0, 0, 0xffff-(1<<9), 1<<9);//SwitchPatch #4 Page0,Reg.0[9] = 1 //A cut don't need it
	mdelay(1);
	Set_GPHYWB(999, 1, 29, 0xffff-(1<<0), 1<<0);//SwitchPatch #4 Page1.Reg.29[0] =1 //A cut don't need it
	mdelay(1);

	//write page 31 = reg4 to check the PHY parameter

	Set_GPHYWB(999, 4, 16, 0, 0x7377);//SwitchPatch #5 Page4,Reg.16 = 0x7377
    mdelay(1);
	Set_GPHYWB(999, 4, 24, 0, 0xc0f3);//SwitchPatch #6 Page4,Reg.24 = 0xc0f3
    mdelay(1);
	Set_GPHYWB(999, 4, 25, 0, 0x7630);//SwitchPatch #7 Page4,Reg.25 = 0x7630
    mdelay(1);
	REG32( 0xBB804160)=0x0E739CE7 ;//SwitchPatch #8 BB804160 = 0x0E739CE7 ,Set EEE MAC register
    mdelay(1);
	Set_GPHYWB(999, 0, 21, 0xffff-(15<<4), 3<<4);//Patch #9 Page0,Reg.21[7:4]=0011  ,Set Green ethernet
    mdelay(1);
	Set_GPHYWB(999, 0, 21, 0xffff-(15<<0), 2<<0);//Patch #10 Page0,Reg.21[3:0]=0010  ,Set Green ethernet
	mdelay(1);
}
#endif		
		
	// refine for B-cut and later
	{
	unsigned int val= REG32(0xb80000dc);
	if ((val & 0x3) != 0x3)
		REG32(0xb80000dc) |= 0x3; // wlanmac_control, set active_wlanmac_sys and active_wlanmac_lx for RF revision ID

	if ((REG32(0xb86400f0) & 0x0000f000) >= 0x00001000)
		Set_GPHYWB(999, 1, 16, 0xffff-(7<<4), 4<<4); // fix 100M IOL overshoot issue

	REG32(0xb80000dc) = val; // restore value.
	}

	/* enlarge "Flow control DSC tolerance" from 24 pages to 48 pages
	    to prevent the hardware may drop incoming packets
	    after flow control triggered and Pause frame sent */
 	REG32(MACCR)= (REG32(MACCR) & ~CF_FCDSC_MASK) | (0x30 << CF_FCDSC_OFFSET);
	
	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);

	return 0;
}
#endif

#if defined(CONFIG_RTL_8197F)
#define RCAL_OFFSET		(-2)

static const unsigned char thr_cum[5] = {6,7,7,6,6};
static const unsigned char rtct_result[8][32] = 
	{ "Impedance about 100 ohm",
	  "Impedance about 100 ohm",
	  "Impedance < 75 ohm           ",
	  "Short                        ",
	  "Impedance > 150 ohm          ",
	  "Open                         ",
	  "Unknown",
	  "Unknown" };

// for RTCT (RealTek Cable diagnostic Test) feature
void phy_setting_for_RTCT(void)
{
	int i, phyid;
	unsigned long flags=0;

	SMP_LOCK_ETH(flags);
	for(i=0; i<5; i++) {
		phyid = rtl8651AsicEthernetTable[i].phyId;		

		// Disable ALDPS mode (power saving mode)
		Set_GPHYWB(phyid, 0, 24, 0xffff-(1<<15), 0<<15);

		// Update RTCT Cable distance detection range (0~140m) :
		// 		idx_timout[1:0] value for support test > 130 ~140m cable  
		Set_GPHYWB(phyid, 2, 20, 0xffff-(3<<14), 0<<14);

		// Update RTCT parameters
		//		(1) Update PHYId($phyId) RTCT parmeters: (basic parameter)
		Set_GPHYWB(phyid, 2, 16, 0xffff-(3<<10), 3<<10);
		Set_GPHYWB(phyid, 2, 26, 0xffff-((7<<3)|(7<<0)), ((4<<3)|(7<<0)));
		Set_GPHYWB(phyid, 2, 21, 0xffff-(0x1f<<11), 16<<11);
		Set_GPHYWB(phyid, 2, 24, 0xffff-((1<<14)|(1<<12)|(3<<9)), ((0<<14)|(0<<12)|(1<<9)));

		// 		(2) Update PHYId($phyId) RTCT parmeters: (Echo cancellation)
		Set_GPHYWB(phyid, 2, 20, 0xffff-((0x1f<<9)|(0x1ff<<0)), ((6<<9)|(68<<0)));
		Set_GPHYWB(phyid, 2, 26, 0xffff-(0x1f<<6), 8<<6);
		Set_GPHYWB(phyid, 2, 25, 0xffff-((0xff<<8)|(0xff<<0)), ((thr_cum[i]<<8)|(88<<0)));
		
		// 		(3) Update PHYId($phyId) RTCT parmeters: (Threshold adjust)
		Set_GPHYWB(phyid, 2, 17, 0xffff-(0x1ff<<0), (100<<0));
		Set_GPHYWB(phyid, 2, 18, 0xffff-(0x1ff<<0), (197<<0));
		Set_GPHYWB(phyid, 2, 19, 0xffff-(0x1ff<<0), (271<<0));
		Set_GPHYWB(phyid, 2, 16, 0xffff-(0x3ff<<0), (178<<0));
		Set_GPHYWB(phyid, 2, 21, 0xffff-(0x7ff<<0), 456<<0);
		Set_GPHYWB(phyid, 2, 22, 0xffff-(0x7ff<<0), 287<<0);
		Set_GPHYWB(phyid, 2, 23, 0xffff-(0x3ff<<0), 100<<0);
		Set_GPHYWB(phyid, 2, 24, 0xffff-(0x1ff<<0), 52<<0);
		Set_GPHYWB(phyid, 2, 26, 0xffff-(0x1f<11), 14<<11);
		Set_GPHYWB(phyid, 2, 19, 0xffff-(0xf<<12), 6<<12);
		Set_GPHYWB(phyid, 2, 18, 0xffff-(0x1f<<10), 8<<10);
	 
		// Enable ALDPS mode
		Set_GPHYWB(phyid, 0, 24, 0xffff-(1<<15), 1<<15);
	}
	SMP_UNLOCK_ETH(flags);
	
}

int Setting_RTL8197F_PHY(void)
{
	int i, phyid;
	unsigned int data, force_r;

	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

 	REG32(MACCR)= (REG32(MACCR) & ~CF_FCDSC_MASK) | (0x48 << CF_FCDSC_OFFSET);

	// if enable 16K-byte jumbo frame support, set cf_fcdsc[6:0]= 0x7F
 	//REG32(MACCR)= (REG32(MACCR) & ~CF_FCDSC_MASK) | (0x7F << CF_FCDSC_OFFSET);

	/* amendment #1
		purpose: to refine the Auot-k R result.
	 */
	for(i=0; i<5; i++) {
		phyid = rtl8651AsicEthernetTable[i].phyId;		
		
		Set_GPHYWB(phyid, 1, 29, 0xffff-(3<<1), 1<<1); // enable auto-k once only after power on; force to Auto-K mode

		rtl8651_setAsicEthernetPHYReg( phyid, 31, 1);
		rtl8651_getAsicEthernetPHYReg( phyid, 28, &data); // read back the auto-K R value
		force_r = (data >> 12) & 0xf;
		if (force_r >= 2)
			force_r += RCAL_OFFSET;

		data = (data & ~0xF000) | (force_r << 12);
		rtl8651_setAsicEthernetPHYReg( phyid, 28, data);		
		rtl8651_setAsicEthernetPHYReg( phyid, 31, 0);

		Set_GPHYWB(phyid, 1, 29, 0xffff-(1<<2), 1<<2); // enable forceR mode
		Set_GPHYWB(phyid, 0, 0, 0xffff-(1<<15), 1<<15); // reset PHY
	}
	/* amendment #2
		purpose: to solve 100M IOL Vout too large issue. 
				 to finetune bandgap current and finetune 100M Vout voltage. 
	 */
	Set_GPHYWB(4, 1, 23, 0xffff-0xf880, 0x7000);
	Set_GPHYWB(999, 1, 18, 0xffff-(0xf<<4), 5<<4);

	/* amendment #3
		purpose: to solve the IOL rise/falling time and symmetry violate spec issue.
	 */
	Set_GPHYWB(999, 1, 16, 0xffff-(7<<13), 7<<13);	 
	Set_GPHYWB(999, 1, 17, 0xffff-(7<<11), 6<<11);	 

	/* amendment #4
		purpose: to improve/solve IOL 10M TPIDL mask fit fail issue.
	 */
	Set_GPHYWB(999, 1, 16, 0xffff-(3<<8), 0<<8);	 

	/* amendment #5
		purpose: to solve PHY not accept the 16kB jumbo frame issue.
	 */
	Set_GPHYWB(999, 0, 23, 0xffff-(1<<8), 1<<8);	 

	phy_setting_for_RTCT();
	
	for(i=0; i<5; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);
	return 0;
}
#endif

// RTCT (RealTek Cable diagnostic Test):
/* output format
RTCT test result:
---------------------------------------------------------------------------------
Port#   Channel    Cable_status                    "Distance to abnormal point"
---------------------------------------------------------------------------------
[0]     A          Normal (Link up)                -
[0]     B          Normal (Link up)                -
[1]     A          Open                            <= 4 meter
[1]     B          Open                            <= 4 meter
[2]     A          Short                           7 meter
[2]     B          Short                           7 meter
[3]     A          Impedance mismatch (<75 ohm)    28 meter
[3]     B          Impedance mismatch (<75 ohm)    28 meter
[4]     A          Impedance mismatch (>150 ohm)   50 meter
[4]     B          Impedance mismatch (>150 ohm)   50 meter
 */
void RT_cable_test(void)
{
#if defined(CONFIG_RTL_8197F)
/* 
 * test result: {Normal} {Open} {Short} {GT_150_ohm} {LT_75_ohm}, 
 *				the distance measurement of abnormal cases.
 * limitation: only for 0 to 120 meter cable. 
 * 			   the test result can't be diagnosed correctly if cable length is greater than 120 meter.
 */
	int i, j, k, phyid;
	unsigned int data, data2;
	unsigned long flags=0;

	printk("RTCT test result:\n");
	printk("---------------------------------------------------------------------------------\n");
	printk("Port#   Channel    Cable_status                    \"Distance to abnormal point\"\n");
	printk("---------------------------------------------------------------------------------\n");

	SMP_LOCK_ETH(flags);
	for(i=0; i<5; i++) {

		printk("[%d]     ", i);
		data = REG32(PSRP0+((i)<<2));
		if ((data & PortStatusLinkUp) != 0)
		{
			printk("A          %s (Link up)\n", rtct_result[0]);
			printk("[%d]     B          %s (Link up)\n", i, rtct_result[0]);
			continue;
		}
				
		phyid = rtl8651AsicEthernetTable[i].phyId;		

		//for(j=1; j>=0; j--) 
		for(j=0; j<=1; j++) 
		{
			// Disable ALDPS mode (power saving mode)
			Set_GPHYWB(phyid, 0, 24, 0xffff-(1<<15), 0<<15);

			// Select channel
			Set_GPHYWB(phyid, 2, 16, 0xffff-(1<<14), j<<14);

			if (j == 0) {
				// channel B(we display ch_A here): force mdix mode
				Set_GPHYWB(phyid, 0, 28, 0xffff-(3<<1), 0<<1);
				printk("A          ");
			}
			else {
				// channel A(we display ch_B here): force mdi mode
				Set_GPHYWB(phyid, 0, 28, 0xffff-(3<<1), 1<<1);
				printk("[%d]     B          ", i);
			}
			mdelay(500);
				
			// Enable Virtual Cable Doctor and start to test 
			Set_GPHYWB(phyid, 2, 16, 0xffff-(1<<15), 1<<15);
          
			// Check and wait test process is finished
			rtl8651_setAsicEthernetPHYReg( phyid, 31, 2);
			k = 100;
			while (k--) {
				rtl8651_getAsicEthernetPHYReg( phyid, 28, &data);
				if ((data & (1<<12)) == (1<<12))
					break;
				mdelay(10);
			}

			if ((data & (1<<12)) == (1<<12)) {
				rtl8651_getAsicEthernetPHYReg( phyid, 29, &data2);
				k = (data >> 9) & 0x7;

				if ((k <= 1) || (k >= 6))
					printk("%s\n", rtct_result[k]);
				else {
					printk("%s   ", rtct_result[k]);
					data2 = (data2 & 0xfff) / 20;
					if (data2 <= 4)
						printk("<= 4 meter\n");
					else
						printk("%d meter\n", data2);
				}
			}
			else {
				printk("time out\n");
			}
			rtl8651_setAsicEthernetPHYReg( phyid, 31, 0);			          
		}

		// recover to auto mdi/mdix mode
		Set_GPHYWB(phyid, 0, 28, 0xffff-(1<<2), 1<<2);
		
		// Resume to the Normal operating mode, disable RTCT test. 
		Set_GPHYWB(phyid, 2, 16, 0xffff-(1<<15), 0<<15);
	 
		// Enable ALDPS mode
		Set_GPHYWB(phyid, 0, 24, 0xffff-(1<<15), 1<<15);
	}
	SMP_UNLOCK_ETH(flags);
	
#else
	// the test method may different for the other SoC
#endif
}

struct _phy_sts
{
	uint32		power_down: 1;
	uint32		AN_mode_loc: 1;
	uint32		AN_cap_rmt: 1;	
	uint32		force_spd: 1;
	uint32		force_dpx: 1;
	uint32		ability_loc: 4;
	uint32		ability_rmt: 4;
	uint32		ability_compare: 4;
	uint32		AN_complete: 1;
	uint32		link: 1;
	uint32		force_spd_dpx: 2;
	uint32		speed_100: 1;
	uint32		reserved: 10;
};

#define _PHY_10_100M_ABILITY	(CAPABLE_100BASE_TX_FD|CAPABLE_100BASE_TX_HD|CAPABLE_10BASE_TX_FD|CAPABLE_10BASE_TX_HD)

static const char str_link[2][5] = { "Down", "Up  " };
static const char str_speed[6][5] = { "10H ", "10F ", "100H", "100F", "none", "-   " };
static const char str_powerdown[2][10] = { "No       ", "PowerDown" };
static const char str_AN_mode[2][6] = { "Force", "AN   " };
static const char str_AN_cap[2][4] = { "No ", "Yes" };
static const char str_ability[10] = { "{1,1,1,1}" };

#define test_bit_(_n,_p) (_n & (1u << _p))

void get_snr(int port, int mdimode)
{
	int i, phyid;
	unsigned int data=0, data_mdi=0, sum_snr=0, snr_tmp=0;

	phyid = rtl8651AsicEthernetTable[port].phyId;		

	for (i=0; i<10; i++)
	{
           rtl8651_setAsicEthernetPHYReg( phyid, 25, (0x6964) );
           rtl8651_getAsicEthernetPHYReg( phyid, 26, &data );
           rtl8651_setAsicEthernetPHYReg( phyid, 26, ((data&0xFF00) |0x9E) ); // Close new_SD.
           rtl8651_getAsicEthernetPHYReg( phyid, 17, &data );
           rtl8651_setAsicEthernetPHYReg( phyid, 17, ((data&0xFFF0)) );
           rtl8651_getAsicEthernetPHYReg( phyid, 29, &snr_tmp );

           sum_snr += snr_tmp;
	}
	sum_snr = sum_snr/10;

	//get final result of mid/mdix mode
	rtl8651_getAsicEthernetPHYReg( phyid, 30, &data);
	data &= (1<<7);
	rtl8651_getAsicEthernetPHYReg( phyid, 28, &data_mdi);
	data_mdi = ((data_mdi & (0x3<<1))>>1) ;

	// db value is calculated thru -(10 * log10(sum_snr/pow(2,18)))
	printk("     %04d         %s(%s)", sum_snr, (data == 0) ? "mdix" : "mdi",(data_mdi==3) ? "Auto":	"Force");

}

/* output format
**** Extract the PHY Diagnostic Information ****
---------------------------------------------------------------------------------------
Port#   Link   Speed   100M_SNR   PowerDown   AN-mode   AN-Cap    ability     ability
                                              (local)   (remote)  (local)     (remote)  
---------------------------------------------------------------------------------------
[0]     Up     10F     -          No          AN        Yes       {0,0,1,1}   {1,1,1,1}
[1]     Down   -       -          PowerDown   Force     No        {1,1,1,1}   NA
[2]     Down   -       -          No          AN        No        {1,1,1,1}   NA
[3]     Up     100F    1197       No          AN        Yes       {1,1,1,1}   {1,1,1,1}
[4]     Down   -       -          No          AN        No        {1,1,1,1}   NA

## NOTE: ability(local/remote): is the bitmap of {100F 100H 10F 10H}
 */
void phy_diag(uint8 mdimode, uint32 portmask)
{
	int i, phyid=0, diffmdi=0;
	unsigned int data=0, data_org=0;
	struct _phy_sts phy_sts[5];
	const char *str1;
	char str2[10];
	unsigned long flags=0;

	SMP_LOCK_ETH(flags);
	for(i=0; i<5; i++) {
		if ((1<<i) & portmask) {

			phyid = rtl8651AsicEthernetTable[i].phyId;	

			//change back to page 0
			rtl8651_setAsicEthernetPHYReg(phyid, 31, 0);
			
			//set auto mdi/mdix, force mdi, force mdix
			rtl8651_getAsicEthernetPHYReg( phyid, 28, &data);
			data_org = data;
			data = (data & ~(0x3<<1)) | (mdimode<<1);
			if ((data_org & (0x3<<1)) != (data & (0x3<<1))) {
				diffmdi++;
				//phy restart Nway
				rtl8651_setAsicEthernetPHYReg( phyid, 28, data);
				rtl8651_restartAsicEthernetPHYNway(i);
			}
		}
	}
	
	if (diffmdi > 0)
		mdelay(7000); //wait for partner being static

	// get phy status
	for(i=0; i<5; i++) {
		if ((1<<i) & portmask) {			

			phyid = rtl8651AsicEthernetTable[i].phyId;	
			
			rtl8651_getAsicEthernetPHYReg( phyid, 0, &data);
			phy_sts[i].power_down = (data & POWER_DOWN) >> 11;
			phy_sts[i].AN_mode_loc = (data & ENABLE_AUTONEGO) >> 12;
			phy_sts[i].force_spd = (data & SPEED_SELECT_100M) >> 13;
			phy_sts[i].force_dpx = (data & SELECT_FULL_DUPLEX) >> 8;
			
			rtl8651_getAsicEthernetPHYReg( phyid, 1, &data);
			phy_sts[i].AN_complete = (data & STS_AUTONEGO_COMPLETE) >> 5;
			phy_sts[i].link = (data & STS_LINK_ESTABLISHED) >> 2;
			
			rtl8651_getAsicEthernetPHYReg( phyid, 4, &data);
			phy_sts[i].ability_loc = (data & _PHY_10_100M_ABILITY) >> 5;
			rtl8651_getAsicEthernetPHYReg( phyid, 5, &data);
			phy_sts[i].ability_rmt = (data & _PHY_10_100M_ABILITY) >> 5;
			phy_sts[i].ability_compare = phy_sts[i].ability_loc & phy_sts[i].ability_rmt;
			rtl8651_getAsicEthernetPHYReg( phyid, 6, &data);
			phy_sts[i].AN_cap_rmt = data & 0x1;

			phy_sts[i].force_spd_dpx = (phy_sts[i].force_spd << 1) | phy_sts[i].force_dpx;
		}
	}

	printk("\n**** Extract the PHY Diagnostic Information ****\n");
	printk("--------------------------------------------------------------------------------------------------------------------\n");
	printk("Port#   Link   Speed   PowerDown   AN-mode   AN-Cap    ability     ability     ANerr     100M_SNR     mdi/mdix mode \n");
	printk("                                   (local)   (remote)  (local)     (remote)   \n");
	printk("--------------------------------------------------------------------------------------------------------------------\n");
	for(i=0; i<5; i++) {
		if ((1<<i) & portmask) {

			phyid = rtl8651AsicEthernetTable[i].phyId;	
			
			printk("[%d]     %s   ", i, str_link[phy_sts[i].link]);

			phy_sts[i].speed_100 = 0;
			if (phy_sts[i].link) {
				if (phy_sts[i].AN_mode_loc) {
					if (test_bit_(phy_sts[i].ability_compare,3)) {
						str1 = str_speed[3];
						phy_sts[i].speed_100 = 1;
					}
					else if (test_bit_(phy_sts[i].ability_compare,2)) {
						str1 = str_speed[2];
						phy_sts[i].speed_100 = 1;
					}
					else if (test_bit_(phy_sts[i].ability_compare,1))
						str1 = str_speed[1];
					else if (test_bit_(phy_sts[i].ability_compare,0))
						str1 = str_speed[0];
					else
						str1 = str_speed[4];
				}
				else {
					str1 = str_speed[phy_sts[i].force_spd_dpx];
					if (phy_sts[i].force_spd)
						phy_sts[i].speed_100 = 1;
				}
			}
			else
				str1 =  str_speed[5];
			printk("%s    ", str1);

			printk("%s   ", str_powerdown[phy_sts[i].power_down]);
			printk("%s     ", str_AN_mode[phy_sts[i].AN_mode_loc]);
			printk("%s       ", str_AN_cap[phy_sts[i].AN_cap_rmt]);

			strcpy(str2, str_ability);		
			if (!test_bit_(phy_sts[i].ability_loc,3))
				str2[1] = '0';
			if (!test_bit_(phy_sts[i].ability_loc,2))
				str2[3] = '0';
			if (!test_bit_(phy_sts[i].ability_loc,1))
				str2[5] = '0';
			if (!test_bit_(phy_sts[i].ability_loc,0))
				str2[7] = '0';
			printk("%s   ", str2);

			if (phy_sts[i].AN_cap_rmt == 0)
				printk("NA        ");
			else {
				strcpy(str2, str_ability);		
				if (!test_bit_(phy_sts[i].ability_rmt,3))
					str2[1] = '0';
				if (!test_bit_(phy_sts[i].ability_rmt,2))
					str2[3] = '0';
				if (!test_bit_(phy_sts[i].ability_rmt,1))
					str2[5] = '0';
				if (!test_bit_(phy_sts[i].ability_rmt,0))
					str2[7] = '0';
				printk("%s ", str2);
			}

			rtl8651_getAsicEthernetPHYReg( phyid, 30, &data);
			data &= (1<<15);
			printk("  %d       ", (data == 0) ? 0 : 1);		

			// to get the 100M SNR value
			if ((phy_sts[i].link) && (phy_sts[i].speed_100))
					get_snr(i, mdimode);
			else 
				printk("             %s       ",str_speed[5]);

			printk("\n");
		}
	}

	SMP_UNLOCK_ETH(flags);
	
	printk("\n## NOTE: ability(local/remote) is the bitmap of {100F 100H 10F 10H}\n");
	printk("         snr value translating to dB value is by: -(10 * log10(snr/pow(2,18)))\n");
	printk("**ANerr bit maybe read clear by other program.\n");
}

void enable_EEE(void)
{
#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8370_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)
	int i;
	extern int rtk_eee_portEnable_set(uint32 port, uint32 enable);
	for (i=0;i<EXT_SWITCH_MAX_PHY_PORT;i++) {
		rtk_eee_portEnable_set(i, 1);
	}
#elif defined(CONFIG_RTL_8198)
	eee_phy_enable_98();

#elif defined(CONFIG_RTL_8198C)
	// enable 10M EEE
	Set_GPHYWB(999, 0xa43, 25, 0xffff-(1<<4), 1<<4);

	Set_GPHYWB(999, 0xa5d, 16, 0xffff-(0x3<<1), 0x3<<1);
	REG32(EEECR) = 0x1F1F1F1F;
	REG32(EEEABICR1) = 0x1F1F;
	
#elif defined(CONFIG_RTL_8197F)
	int i;

	for(i=0; i<RTL8651_PHY_NUMBER; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	REG32(EEECR) = 0x0F0F0F0F;
	REG32(EEEABICR1) = 0x00000F0F;

	Set_GPHYWB(999, 4, 16, 0xffff-((0x3<<12)|(0x3<<8)), ((0x3<<12)|(0x3<<8)));
	
	for(i=0; i<RTL8651_PHY_NUMBER; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);
#else

	int i;

	for(i=0; i<RTL8651_PHY_NUMBER; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);
#if defined(CONFIG_RTL_8881A)
	Set_GPHYWB(999,4, 16, 0, 0x7377);
	mdelay(1);
	Set_GPHYWB(999,4, 24, 0, 0xc0f3);
	mdelay(1);
	Set_GPHYWB(999,4, 25, 0, 0x7630);
	//enable MAC EEE
	REG32(EEECR) = 0x0E739CE7;

#elif defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)
	//enable 100M EEE and 10M EEE
	Set_GPHYWB(999, 4, 16, 0xffff-(0x3<<12), 0x3<<12);

	//enable MAC EEE
	REG32(EEECR) = 0x0E739CE7;
	
#elif defined(CONFIG_RTL_8196C)

	#ifdef RTL8196C_EEE_REFINE
	{
	/* 1. to link down all of the PHY */
	uint32 reg;
	 
	for(i=0; i<RTL8651_PHY_NUMBER; i++) {
		rtl8651_getAsicEthernetPHYReg( i, 0, &reg );
		reg |= (POWER_DOWN); 
		rtl8651_setAsicEthernetPHYReg( i, 0, reg );
	}	 

	for(i=0; i<RTL8651_PHY_NUMBER; i++) {
		eee_phy_enable_by_port(i);
	}

	/* 7. set PHY POWER_DOWN=0 */	 
	for(i=0; i<RTL8651_PHY_NUMBER; i++) {
		rtl8651_getAsicEthernetPHYReg( i, 0, &reg );
		reg &= (~POWER_DOWN); 
		rtl8651_setAsicEthernetPHYReg( i, 0, reg );
	}	
	}
	#else
	for(i=0; i<RTL8651_PHY_NUMBER; i++) {
		eee_phy_enable_by_port(i);
	}
	#endif

	// set FRC_P0_EEE_100, EN_P0_TX_EEE and EN_P0_RX_EEE
	//REG32(EEECR) = 0x0E739CE7;		// consult with Jim and Anson, we do not use this setting.
	// set EN_P0_TX_EEE and EN_P0_RX_EEE
	REG32(EEECR) = 0x06318C63;

#ifdef CONFIG_POCKET_ROUTER_SUPPORT
	#define ETH_PORT_START		4
	#define ETH_PORT_END		4
#else
	#define ETH_PORT_START		0
	#define ETH_PORT_END		4
#endif
	for ( i = ETH_PORT_START ; i <= ETH_PORT_END; i++ ) {
		/* enable phy 100 eee ability */
		mmd_write(i, 7, 60, 0x2);
	}	
#endif

	for(i=0; i<RTL8651_PHY_NUMBER; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);
#endif	
}

void disable_EEE(void)
{
#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8370_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)
	int i;
	extern int rtk_eee_portEnable_set(uint32 port, uint32 enable);
	for (i=0;i<EXT_SWITCH_MAX_PHY_PORT;i++) {
		rtk_eee_portEnable_set(i, 0);
	}
#elif defined(CONFIG_RTL_8198)
	eee_phy_disable_98();

#elif defined(CONFIG_RTL_8198C)
	//disable 10M EEE
	Set_GPHYWB(999, 0xa43, 25, 0xffff-(1<<4), 0<<4);

	Set_GPHYWB(999, 0xa5d, 16, 0xffff-(0x3<<1), 0);
	REG32(EEECR) = 0;
	REG32(EEEABICR1) = 0;

#elif defined(CONFIG_RTL_8197F)
	int i;

	for(i=0; i<RTL8651_PHY_NUMBER; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	//disable EEE MAC
	REG32(EEECR) = 0;
	REG32(EEEABICR1) = 0;

	//disable 100M EEE and 10M EEE
	Set_GPHYWB(999, 4, 16, 0xffff-((0x3<<12)|(0x3<<8)), 0);
	
	for(i=0; i<RTL8651_PHY_NUMBER; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);
#else
	// for CONFIG_RTL_8196C, CONFIG_RTL_819XD and CONFIG_RTL_8196E
	int i;

	for(i=0; i<RTL8651_PHY_NUMBER; i++)
		REG32(PCRP0+i*4) |= (EnForceMode);

	//disable EEE MAC
	REG32(EEECR) = 0;

	//disable 100M EEE and 10M EEE
	Set_GPHYWB(999, 4, 16, 0xffff-(0x3<<12), 0x0<<12);
	
	for(i=0; i<RTL8651_PHY_NUMBER; i++)
		REG32(PCRP0+i*4) &= ~(EnForceMode);
#endif
}

#if defined(CONFIG_RTL_8198C)
void set_8198C_EEE(int mode, uint32 port_mask)
{
	int i;

	for(i=0; i<4; i++) {
		if (((1<<i) & port_mask) != 0) { // need to set port i
			if (mode == 0) { // disable
				Set_GPHYWB(i, 0xa43, 25, 0xffff-(1<<4), 0<<4);
				Set_GPHYWB(i, 0xa5d, 16, 0xffff-(0x3<<1), 0);
				REG32(EEECR) &= ~(0xFF << (i*8));
			}
			else {
				Set_GPHYWB(i, 0xa43, 25, 0xffff-(1<<4), 1<<4);
				Set_GPHYWB(i, 0xa5d, 16, 0xffff-(0x3<<1), 0x3<<1);
				REG32(EEECR) = (REG32(EEECR) & ~(0xFF << (i*8))) | (0x1F << (i*8));
			}
			rtl8651_restartAsicEthernetPHYNway(i);
			mdelay(10);
		}
	}
	if (((1<<4) & port_mask) != 0) { // need to set port 4
		if (mode == 0) { // disable
			Set_GPHYWB(4, 0xa43, 25, 0xffff-(1<<4), 0<<4);
			Set_GPHYWB(4, 0xa5d, 16, 0xffff-(0x3<<1), 0);
			REG32(EEEABICR1) = 0;
		}
		else {
			Set_GPHYWB(4, 0xa43, 25, 0xffff-(1<<4), 1<<4);
			Set_GPHYWB(4, 0xa5d, 16, 0xffff-(0x3<<1), 0x3<<1);
			REG32(EEEABICR1) = 0x1F;
		}
		rtl8651_restartAsicEthernetPHYNway(4);
		mdelay(10);
	}
}
#endif

#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8370_SUPPORT) || defined(CONFIG_RTL_8325D_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT) || defined(CONFIG_RTL_8198C_8367RB)
#define REG_IOCFG_GPIO		0x00000018
	
/* define GPIO port */
enum GPIO_PORT
{
	GPIO_PORT_A = 0,
	GPIO_PORT_B,
	GPIO_PORT_C,
	GPIO_PORT_D,
	GPIO_PORT_E,
	GPIO_PORT_F,
	GPIO_PORT_G,
	GPIO_PORT_H,
	GPIO_PORT_I,
	GPIO_PORT_MAX,
};

extern int32 smi_init(uint32 port, uint32 pinSCK, uint32 pinSDA);
extern int32 smi_init_83xx(uint32 port_sck, uint32 port_sda, uint32 pinSCK, uint32 pinSDA);
extern int rtk_vlan_init(void);
#endif

#ifdef CONFIG_RTL_8367R_SUPPORT
extern int RTL8367R_init(void);
#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined(CONFIG_RTL_DNS_TRAP) || defined(CONFIG_RTL_VLAN_8021Q)
extern int rtk_filter_igrAcl_init(void);
#endif

// for linking issue of \linux-2.6.30\net\rtl\fastpath\9xD\filter.S 
#if !defined(CONFIG_RTL_IGMP_SNOOPING)
int igmp_delete_init_netlink(void) { return 0; }
EXPORT_SYMBOL(igmp_delete_init_netlink);
#endif

#if defined(CONFIG_RTL_8367MB_SUPPORT)
#define GPIO_PINMUX_MASK	(3<<10)
#define GPIO_RESET	19	//GPIO_G3
#define RESET_CNR		PEFGH_CNR
#define RESET_DIR		PEFGH_DIR
#define RESET_DAT		PEFGH_DAT

#define I2C_PINMUX_MASK	(3<<22)
#define I2C_PORT		GPIO_PORT_G
#define I2C_CLOCK		7
#define I2C_DATA		6

#else // 8367RB
#define GPIO_PINMUX_MASK	(3<<12)
#define GPIO_RESET	1	//GPIO_A1
#define RESET_CNR		PABCD_CNR
#define RESET_DIR		PABCD_DIR
#define RESET_DAT		PABCD_DAT

#define I2C_PINMUX_MASK	(3<<13)
#define I2C_PORT		GPIO_PORT_F
#define I2C_CLOCK		5
#define I2C_DATA		6
#endif

void init_8367r(void)
{
#ifdef CONFIG_RTL_8881A
	int i=0;
	int ret=0;
	REG32(0xb8000044) |=  (3<<12); // reg_iocfg_fcs1, set to GPIO mode
	REG32(PABCD_CNR) &= (~(0x00004000)); //set GPIO pin, A1
	REG32(PABCD_DIR) |= (0x00004000); //output pin

	for (i=0; i<3; i++) 
	{
		// for 8367r h/w reset pin
		REG32(PABCD_DAT) &= (~(0x00004000)); 
		mdelay(1000);
		REG32(PABCD_DAT) |= (0x00004000); 
		mdelay(1000);

		// set to GPIO mode
		#ifndef CONFIG_MTD_NAND 
		REG32(PIN_MUX_SEL)=0xc;// ((REG32(PIN_MUX_SEL)&(~(7<<2))|0x3<<2));

		// MDC: F5, MDIO: F6
		WRITE_MEM32(PEFGH_CNR, READ_MEM32(PEFGH_CNR) & (~(0x00006000)));	//set GPIO pin, F5 and F6
		WRITE_MEM32(PEFGH_DIR, READ_MEM32(PEFGH_DIR) | ((0x00006000))); //output pin

		//smi_init(GPIO_PORT_A, 3, 2);
		smi_init(GPIO_PORT_F, 5, 6);
		#else
		REG32(PIN_MUX_SEL2) = (REG32(PIN_MUX_SEL2)&(~(0xe38))) | (0x618);
		WRITE_MEM32(PABCD_CNR, READ_MEM32(PABCD_CNR) & (~(0x00002800)));
		WRITE_MEM32(PABCD_DIR, READ_MEM32(PABCD_DIR) | ((0x00002800)));
		smi_init(GPIO_PORT_B, 5, 3);

		#endif
		mdelay(1000);
		ret = RTL8367R_init();
		if (ret == 0) 
			break;
	}
#elif defined(CONFIG_RTL_8197F)
	#undef GPIO_RESET
	#define GPIO_RESET	26		// GPIO_H2
	int ret, i;

	// always do init_8367r() when call rtl865x_reinitSwitchCore()
	//#if defined(REINIT_SWITCH_CORE)
	//if (rtl865x_duringReInitSwtichCore == 1)
	//	return;
	//#endif

	#if defined(CONFIG_OPENWRT_SDK)
	// Reset 8367RB: REG_PINMUX_14, BIT_REG_IOCFG_LED_P0
	REG32(PIN_MUX_SEL14) = (REG32(PIN_MUX_SEL14) & ~(0xF<<28)) | (2<<28);
	REG32(PEFGH_CNR) &= ~(1<<GPIO_RESET);
	REG32(PEFGH_DIR) |= (1<<GPIO_RESET);
	#endif
#ifndef CONFIG_PARALLEL_NAND_FLASH
	REG32(PIN_MUX_SEL) = 0; // TODO: need to find out
	REG32(PIN_MUX_SEL1) = 0; // TODO: need to find out
#endif

	for (i=0; i<3; i++) 
	{
		// for 8367r h/w reset pin
		REG32(PEFGH_DAT) &= ~(1<<GPIO_RESET);
		mdelay(1000);
		REG32(PEFGH_DAT) |= (1<<GPIO_RESET);
		mdelay(1000);

		#if defined(CONFIG_OPENWRT_SDK)
		// set to GPIO mode
		// [11:8]	BIT_REG_IOCFG_P0MDC, 0110:GPIO
		// [7:4]	BIT_REG_IOCFG_P0MDIO, 0110:GPIO
#ifndef CONFIG_MTD_NAND
		REG32(PIN_MUX_SEL2) = 0x660; // TODO: need to find out
#endif
		REG32(PABCD_CNR) &= ~(0x000C0000);	//set GPIO pin, C2 and C3
		REG32(PABCD_DIR) |= (0x000C0000);	//output pin
		#endif
		
		// MDC: C2, MDIO: C3
		smi_init(GPIO_PORT_C, 2, 3);				
		mdelay(1000);
		ret = RTL8367R_init();
		if (ret == 0) 
			break;
	}
#else	
	int ret, i;

	#if defined(CONFIG_RTL_8367MB_SUPPORT)
	REG32(PIN_MUX_SEL) |= GPIO_PINMUX_MASK; // reg_iocfg_fcs1, set to GPIO mode
	REG32(RESET_CNR) &= ~(1<<GPIO_RESET); //set GPIO pin, A1
	REG32(RESET_DIR) |= (1<<GPIO_RESET); //output pin
	#endif
	
	for (i=0; i<3; i++) {
		// for 8367r h/w reset pin
		REG32(RESET_DAT) &= ~(1<<GPIO_RESET); 
		mdelay(1000);

		REG32(RESET_DAT) |= (1<<GPIO_RESET); 
		mdelay(1000);

		// set to GPIO mode
		REG32(PIN_MUX_SEL) |= (REG_IOCFG_GPIO);

		// MDC: F5, MDIO: F6
		WRITE_MEM32(PEFGH_CNR, READ_MEM32(PEFGH_CNR) & (~I2C_PINMUX_MASK));	//set GPIO pin
		WRITE_MEM32(PEFGH_DIR, READ_MEM32(PEFGH_DIR) | (I2C_PINMUX_MASK)); //output pin

		smi_init(I2C_PORT, I2C_CLOCK, I2C_DATA);
	
		ret = RTL8367R_init();
		if (ret == 0) 
			break;
	}
#endif
	// 8367_VLAN
	rtk_vlan_init();

	// clear mib counter
	rtk_stat_global_reset();

#ifdef CONFIG_RTL_CPU_TAG
	RTL8367R_vlan_set();
	RTL8367R_cpu_tag(1);
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT)|| defined(CONFIG_RTL_DNS_TRAP) || defined(CONFIG_RTL_VLAN_8021Q)
	rtk_filter_igrAcl_init();
#endif
}
#endif

#ifdef CONFIG_RTL_8198C_8367RB
extern int32 smi_init(uint32 port, uint32 pinSCK, uint32 pinSDA);
extern int RTL8367R_init_switch_mode(void);
void init_8367rb_for_8198c(void)
{
	int i, ret = -1;
	
	REG32(PIN_MUX_SEL3) = (REG32(PIN_MUX_SEL3) & ~(3<<30)) | (0x3<<30);
	REG32(PEFGH_CNR) &= (~(0x00000040));
	REG32(PEFGH_DIR) |= (0x00000040);

	for (i=0; i<3; i++) 
	{
		// for 8367r h/w reset pin	
		REG32(PEFGH_DAT) &= (~(0x00000040)); 
		mdelay(1000);
		REG32(PEFGH_DAT) |= (0x00000040); 
		mdelay(1000);
		
		// set to GPIO mode
		REG32(PIN_MUX_SEL4) = (REG32(PIN_MUX_SEL4) & ~(7<<0)) | (0x3<<0);

		//P5 Rx/Tx need to be set as MII mode
		REG32(PIN_MUX_SEL4) = (REG32(PIN_MUX_SEL4) & ~((0xf<<3) | (0x3<<23))) | (0x1<<3);
		REG32(PIN_MUX_SEL5) &= ~((0x3ff<<14) | (0x3<<0));

		// MDC: F5, MDIO: F6
		WRITE_MEM32(PEFGH_CNR, READ_MEM32(PEFGH_CNR) & (~(0x00006000)));	//set GPIO pin, F5 and F6
		WRITE_MEM32(PEFGH_DIR, READ_MEM32(PEFGH_DIR) | ((0x00006000))); //output pin

		smi_init(GPIO_PORT_F, 5, 6);				
		mdelay(1000);
		
		ret = RTL8367R_init_switch_mode();
		if (ret == 0) 
			break;
	}
}
#endif

#ifdef CONFIG_RTL_8370_SUPPORT
extern int RTL8370_init(void);

void init_8370(void)
{	
	int ret, i;

	REG32(PIN_MUX_SEL) |=  (3<<12); // reg_iocfg_fcs1, set to GPIO mode
	REG32(PABCD_CNR) &= (~(0x00000002)); //set GPIO pin, A1
	REG32(PABCD_DIR) |= (0x00000002); //output pin

	for (i=0; i<3; i++) {
		// for 8370 h/w reset pin
		REG32(PABCD_DAT) &= ~(0x00000002); 
		mdelay(1000);

		REG32(PABCD_DAT) |= (0x00000002); 
		mdelay(1000);

		// set to GPIO mode
		REG32(PIN_MUX_SEL) |= (0x3c000000); // bit 27~26 = 0b11, bit 29~28 = 0b11

		//MA22(GPIOC5) => clock
		//MA21(GPIOC4) => IO
		// MDC: C5, MDIO: C4
		WRITE_MEM32(PABCD_CNR, READ_MEM32(PABCD_CNR) & (~(0x00300000)));	//set GPIO pin, C5 and C4
		WRITE_MEM32(PABCD_DIR, READ_MEM32(PABCD_DIR) | ((0x00300000))); //output pin

		smi_init(GPIO_PORT_C, 5, 4);
	
		ret = RTL8370_init();
		if (ret == 0) 
			break;
	}

	// 8370_VLAN
	rtk_vlan_init();

	// clear mib counter
	rtk_stat_global_reset();
}
#endif

#ifdef CONFIG_RTL_83XX_SUPPORT
extern int RTL83XX_init(void);
#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined(CONFIG_RTL_DNS_TRAP)
extern int rtk_filter_igrAcl_init(void);
#endif

// for linking issue of \linux-2.6.30\net\rtl\fastpath\9xD\filter.S 
#if !defined(CONFIG_RTL_IGMP_SNOOPING)
int igmp_delete_init_netlink(void) { return 0; }
EXPORT_SYMBOL(igmp_delete_init_netlink);
#endif

#if defined(CONFIG_RTL_8367MB_SUPPORT)
#define GPIO_PINMUX_MASK	(3<<10)
#define GPIO_RESET	19	//GPIO_G3
#define RESET_CNR		PEFGH_CNR
#define RESET_DIR		PEFGH_DIR
#define RESET_DAT		PEFGH_DAT

#define I2C_PINMUX_MASK	(3<<22)
#define I2C_PORT		GPIO_PORT_G
#define I2C_CLOCK		7
#define I2C_DATA		6

#else // 8367RB
#define GPIO_PINMUX_MASK	(3<<12)
#define GPIO_RESET	1	//GPIO_A1
#define RESET_CNR		PABCD_CNR
#define RESET_DIR		PABCD_DIR
#define RESET_DAT		PABCD_DAT

#define I2C_PINMUX_MASK	(3<<13)
#define I2C_PORT		GPIO_PORT_F
#define I2C_CLOCK		5
#define I2C_DATA		6
#endif

void init_83XX(void)
{
#ifdef CONFIG_RTL_8881A
	int i=0;
	int ret=0;
	REG32(0xb8000044) |=  (3<<12); // reg_iocfg_fcs1, set to GPIO mode
	REG32(PABCD_CNR) &= (~(0x00004000)); //set GPIO pin, A1
	REG32(PABCD_DIR) |= (0x00004000); //output pin

	for (i=0; i<3; i++) 
	{
		// for 8367r h/w reset pin
		REG32(PABCD_DAT) &= (~(0x00004000)); 
		mdelay(1000);
		REG32(PABCD_DAT) |= (0x00004000); 
		mdelay(1000);

		// set to GPIO mode
		#ifndef CONFIG_MTD_NAND 
		REG32(PIN_MUX_SEL)=0xc;// ((REG32(PIN_MUX_SEL)&(~(7<<2))|0x3<<2));

		// MDC: F5, MDIO: F6
		WRITE_MEM32(PEFGH_CNR, READ_MEM32(PEFGH_CNR) & (~(0x00006000)));	//set GPIO pin, F5 and F6
		WRITE_MEM32(PEFGH_DIR, READ_MEM32(PEFGH_DIR) | ((0x00006000))); //output pin

		//smi_init(GPIO_PORT_A, 3, 2);
		smi_init(GPIO_PORT_F, 5, 6);
		#else
		REG32(PIN_MUX_SEL2) = (REG32(PIN_MUX_SEL2)&(~(0xe38))) | (0x618);
		WRITE_MEM32(PABCD_CNR, READ_MEM32(PABCD_CNR) & (~(0x00002800)));
		WRITE_MEM32(PABCD_DIR, READ_MEM32(PABCD_DIR) | ((0x00002800)));
		smi_init(GPIO_PORT_B, 5, 3);

		#endif
		mdelay(1000);
		ret = RTL8367R_init();
		if (ret == 0) 
			break;
	}
#elif defined(CONFIG_RTL_8197F)
	#undef GPIO_RESET
	#define GPIO_RESET	26		// GPIO_H2
	#define GPIO_RESET_97FN		9		// GPIO_F1

	int ret, i, bond_id;
	extern uint32 rtl819x_bond_option(void);

	#if defined(REINIT_SWITCH_CORE)
	if (rtl865x_duringReInitSwtichCore == 1)
		return;
	#endif

	#if defined(CONFIG_OPENWRT_SDK)
	// Reset 8367RB: REG_PINMUX_14, BIT_REG_IOCFG_LED_P0
	REG32(PIN_MUX_SEL14) = (REG32(PIN_MUX_SEL14) & ~(0xF<<28)) | (2<<28);
	REG32(PEFGH_CNR) &= ~(1<<GPIO_RESET);
	REG32(PEFGH_DIR) |= (1<<GPIO_RESET);
	#endif

	REG32(PIN_MUX_SEL) = 0; // TODO: need to find out
	REG32(PIN_MUX_SEL1) = 0; // TODO: need to find out

	bond_id = rtl819x_bond_option();

	for (i=0; i<3; i++) 
	{
		if (bond_id == BSP_BOND_97FN) {
			// for 8367r h/w reset pin
			REG32(PEFGH_DAT) &= ~(1<<GPIO_RESET_97FN);
			mdelay(1000);
			REG32(PEFGH_DAT) |= (1<<GPIO_RESET_97FN);
			mdelay(1000);
		} else {
			// for 8367r h/w reset pin
			REG32(PEFGH_DAT) &= ~(1<<GPIO_RESET);
			mdelay(1000);
			REG32(PEFGH_DAT) |= (1<<GPIO_RESET);
			mdelay(1000);
		}

		#if defined(CONFIG_OPENWRT_SDK)
		// set to GPIO mode
		// [11:8]	BIT_REG_IOCFG_P0MDC, 0110:GPIO
		// [7:4]	BIT_REG_IOCFG_P0MDIO, 0110:GPIO
		REG32(PIN_MUX_SEL2) = 0x660; // TODO: need to find out
		REG32(PABCD_CNR) &= ~(0x000C0000);	//set GPIO pin, C2 and C3
		REG32(PABCD_DIR) |= (0x000C0000);	//output pin
		#endif
		
		if (bond_id == BSP_BOND_97FN) {
			// MDC: G6, MDIO: C3
			smi_init_83xx(GPIO_PORT_G, GPIO_PORT_C, 6, 3);	
		} else {
			// MDC: C2, MDIO: C3
			smi_init(GPIO_PORT_C, 2, 3);
		}				
		mdelay(1000);

		ret = RTL83XX_init();
		if (ret == 0) 
			break;
	}
#else	
	int ret, i;

	#if defined(CONFIG_RTL_8367MB_SUPPORT)
	REG32(PIN_MUX_SEL) |= GPIO_PINMUX_MASK; // reg_iocfg_fcs1, set to GPIO mode
	REG32(RESET_CNR) &= ~(1<<GPIO_RESET); //set GPIO pin, A1
	REG32(RESET_DIR) |= (1<<GPIO_RESET); //output pin
	#endif
	
	for (i=0; i<3; i++) {
		// for 8367r h/w reset pin
		REG32(RESET_DAT) &= ~(1<<GPIO_RESET); 
		mdelay(1000);

		REG32(RESET_DAT) |= (1<<GPIO_RESET); 
		mdelay(1000);

		// set to GPIO mode
		REG32(PIN_MUX_SEL) |= (REG_IOCFG_GPIO);

		// MDC: F5, MDIO: F6
		WRITE_MEM32(PEFGH_CNR, READ_MEM32(PEFGH_CNR) & (~I2C_PINMUX_MASK));	//set GPIO pin
		WRITE_MEM32(PEFGH_DIR, READ_MEM32(PEFGH_DIR) | (I2C_PINMUX_MASK)); //output pin

		smi_init(I2C_PORT, I2C_CLOCK, I2C_DATA);
	
		ret = RTL83XX_init();
		if (ret == 0) 
			break;
	}
#endif
	// 8367_VLAN
	ret = rtk_vlan_init();

	// clear mib counter
	rtk_stat_global_reset();

#ifdef CONFIG_RTL_CPU_TAG
	RTL83XX_vlan_set();
	RTL83XX_cpu_tag(1);
#endif

	{ //enable PHY
	extern rtk_api_ret_t rtk_port_phyEnableAll_set(rtk_enable_t enable);
	rtk_port_phyEnableAll_set(ENABLED);
	}	

#if defined(CONFIG_RTK_VLAN_SUPPORT)|| defined(CONFIG_RTL_DNS_TRAP)
	rtk_filter_igrAcl_init();
#endif
}
#endif

#ifdef CONFIG_RTL_8325D_SUPPORT
//extern void rtl8316d_sys_reboot(void);
extern int RTL8325D_init(void);

void init_8325d(void)
{	
	int ret;

	// set to GPIO mode
	REG32(PIN_MUX_SEL) |= (0xC00); // bit 11~10 = 0b11
	
	REG32(PEFGH_CNR) &= (~(0x00020000)); //set GPIO pin, G1
	REG32(PEFGH_DIR) |= (0x00020000); //output pin

	// for 8325d h/w reset pin
	REG32(PEFGH_DAT) &= ~(0x00020000); 
	mdelay(1000);

	REG32(PEFGH_DAT) |= (0x00020000); 
	mdelay(1000);

	//I2C data¡GGPIOG6, P0_RXD5
	//I2C clock¡GGPIOG7, P0_RXD4
	WRITE_MEM32(PEFGH_CNR, READ_MEM32(PEFGH_CNR) & (~(0x00C00000)));	//set GPIO pin, G6 and G7
	WRITE_MEM32(PEFGH_DIR, READ_MEM32(PEFGH_DIR) | ((0x00C00000))); //output pin

	smi_init(GPIO_PORT_G, 7, 6);

	//rtl8316d_sys_reboot();
	//mdelay(1000);
		
	ret = RTL8325D_init();
}
#endif

#if defined(CONFIG_RTL_8211F_SUPPORT)
#if defined(CONFIG_RTL_8197F)
int gpio_simulate_mdc_mdio = 0;

int init_p0(void)
{
	unsigned int data;

	REG32(PCRP0) = (REG32(PCRP0) & ~(0x1F << ExtPHYID_OFFSET)) \
		| ((PORT0_RGMII_PHYID << ExtPHYID_OFFSET) | MIIcfg_RXER |  EnablePHYIf | MacSwReset);
	REG32(MACCR) |= (1<<12);
	REG32(P0GMIICR) = (REG32(P0GMIICR) & ~((1<<4)|(7<<0))) | ((1<<4) | (5<<0));
#ifndef CONFIG_PARALLEL_NAND_FLASH
	REG32(PIN_MUX_SEL) = 0; // TODO: need to find out
	REG32(PIN_MUX_SEL1) = 0; // TODO: need to find out
	REG32(PIN_MUX_SEL2) = 0; // TODO: need to find out
#endif
	
	// Reset 8211E/8211F: REG_PINMUX_14, BIT_REG_IOCFG_LED_P0
	if(gpio_simulate_mdc_mdio){
		extern int nfbi_init(void);

		gpio_request_one(GPIO_RESET_97FN, (GPIOF_DIR_OUT | GPIOF_EXPORT_DIR_FIXED), "reset pin"); 
		gpio_set_value(GPIO_RESET_97FN, 0);
		mdelay(500);
		gpio_set_value(GPIO_RESET_97FN, 1);
		mdelay(300);
		
		REG32(PCRP0) = (REG32(PCRP0) & ~AutoNegoSts_MASK) \
			| (EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex); //forcemode
		nfbi_init();
	} else {
		gpio_request_one(GPIO_RESET, (GPIOF_DIR_OUT | GPIOF_EXPORT_DIR_FIXED), "reset pin"); 
		gpio_set_value(GPIO_RESET, 0);
		mdelay(500);
		gpio_set_value(GPIO_RESET, 1);
		mdelay(300);
	}

	REG32(PITCR) |= (1<<0);
	REG32(P0GMIICR) |=(Conf_done);

	//pad control
//TODO:	REG32(0xb8000048) |= BIT(28);

	if(gpio_simulate_mdc_mdio){
		rtl_mdio_read(PORT0_RGMII_PHYID, 4, &data );
		// Advertise support of asymmetric pause 
		// Advertise support of pause frames
		rtl_mdio_write(PORT0_RGMII_PHYID, 4, (data | 0xC00));
	} else {
		rtl8651_getAsicEthernetPHYReg(PORT0_RGMII_PHYID, 4, &data );
		// Advertise support of asymmetric pause 
		// Advertise support of pause frames
		rtl8651_setAsicEthernetPHYReg(PORT0_RGMII_PHYID, 4, (data | 0xC00));
	}

	return 0;
}
#else // CONFIG_RTL_8881A or CONFIG_RTL_819XD
int init_p0(void)
{
	unsigned int data;

	REG32(PCRP0) |=  (PORT0_RGMII_PHYID << ExtPHYID_OFFSET) | MIIcfg_RXER |  EnablePHYIf | MacSwReset;	
	REG32(MACCR) |= (1<<12);
	REG32(P0GMIICR) = (REG32(P0GMIICR) & ~((1<<4)|(7<<0))) | ((1<<4) | (3<<0));

	#if defined(CONFIG_RTL_8881A)
	// 0xb8000040 bit [4:2] Configure P0 mdc/mdio PAD as P0-MDIO (3'b100)
	// 0xb8000040 bit [9:7] Configure P0 PAD as P0-MII (3'b000)
	REG32(PIN_MUX_SEL) = (REG32(PIN_MUX_SEL) & ~((7<<7) | (7<<2))) | (4<<2);
	#endif
	
	// Reset 8211F
	REG32(PIN_MUX_SEL) |= (3<<10);
	REG32(PEFGH_CNR) &= ~(1<<GPIO_RESET);
	REG32(PEFGH_DIR) |= (1<<GPIO_RESET);
	REG32(PEFGH_DAT) &= ~(1<<GPIO_RESET);
	mdelay(500);
	REG32(PEFGH_DAT) |= (1<<GPIO_RESET);
	mdelay(300);

	REG32(PITCR) |= (1<<0);
	REG32(P0GMIICR) |=(Conf_done);

	//pad control
	REG32(0xb8000048) |= BIT(28);

	rtl8651_getAsicEthernetPHYReg(PORT0_RGMII_PHYID, 4, &data );
	// Advertise support of asymmetric pause 
	// Advertise support of pause frames
	rtl8651_setAsicEthernetPHYReg(PORT0_RGMII_PHYID, 4, (data | 0xC00));

	//panic_printk(" --> config port 0 done.\n");
	return 0;
}
#endif
#endif

#if defined(CONFIG_RTL_8198C_8211F)
int init_p5(void)
{
	unsigned int data;

	REG32(PIN_MUX_SEL4) = REG32(PIN_MUX_SEL4) & ~(0x01E00000);
	REG32(PIN_MUX_SEL5) = REG32(PIN_MUX_SEL5) & ~(0x0000C000);

	// 8211F Reset pin
	#define GPIO_RESET		10		// GPIO_F2

	// PIN_MUX_SEL3 b[20:18] reg_iocfg_p0rxd0 = 0b011[GPIO]
	REG32(PIN_MUX_SEL3) = (REG32(PIN_MUX_SEL3) & ~(7<<18)) | (3<<18);
	REG32(PEFGH_CNR) &= ~(1<<GPIO_RESET);
	REG32(PEFGH_DIR) |= (1<<GPIO_RESET);
	REG32(PEFGH_DAT) &= ~(1<<GPIO_RESET);
	mdelay(500);
	REG32(PEFGH_DAT) |= (1<<GPIO_RESET);
	mdelay(300);

#define _98C_P5_RGMII_TX_DELAY	0
#define _98C_P5_RGMII_RX_DELAY	3
#define _98C_P5_PHY_ID			5

	REG32(P5GMIICR) = (REG32(P5GMIICR) & ~((1<<4)|(7<<0))) | ((_98C_P5_RGMII_TX_DELAY<<4) | (_98C_P5_RGMII_RX_DELAY<<0));
	REG32(PCRP5) = (REG32(PCRP5) & ~(0x1f << ExtPHYID_OFFSET)) 
			| ((_98C_P5_PHY_ID << ExtPHYID_OFFSET) | MIIcfg_RXER |  EnablePHYIf | MacSwReset | AutoNegoSts_MASK);
	REG32(P5GMIICR) |= (Conf_done);	

	rtl8651_getAsicEthernetPHYReg(_98C_P5_PHY_ID, 4, &data );
	// Advertise support of asymmetric pause 
	// Advertise support of pause frames
	rtl8651_setAsicEthernetPHYReg(_98C_P5_PHY_ID, 4, (data | 0xC00));

	return 0;
}
#endif

#ifdef CONFIG_8198_PORT5_RGMII
void init_8198_p5(void)
{
	/* define GPIO port */
	enum GPIO_PORT
	{
	GPIO_PORT_A = 0,
	GPIO_PORT_B,
	GPIO_PORT_C,
	GPIO_PORT_D,
	GPIO_PORT_E,
	GPIO_PORT_F,
	GPIO_PORT_G,
	GPIO_PORT_H,
	GPIO_PORT_I,
	GPIO_PORT_MAX,
	};

	#define REG_IOCFG_GPIO		0x00000018

	extern int32 smi_init(uint32 port, uint32 pinSCK, uint32 pinSDA);
	extern int RTL8370_init(void);

	// set to GPIO mode
	REG32(PIN_MUX_SEL) |= (REG_IOCFG_GPIO);


	WRITE_MEM32(PABCD_CNR, READ_MEM32(PABCD_CNR) & (~(0x0000000C)));	//set GPIO pin
	WRITE_MEM32(PABCD_DIR, READ_MEM32(PABCD_DIR) | ((0x0000000C))); //output pin

	smi_init(GPIO_PORT_A, 3, 2);

	RTL8370_init();
}
#endif

#if defined(CONFIG_RTL819X_GPIO) && (defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)) && !defined(CONFIG_OPENWRT_SDK)
static int rtl819x_8367r_reset_pin_probe(struct platform_device *dev) {
	struct rtl819x_gpio_platdata *pdata = dev->dev.platform_data;
	int ret = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	ret = devm_gpio_request_one(&dev->dev, pdata->gpio, GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED, pdata->name);
#else
	if(!(ret = gpio_request_one(pdata->gpio, GPIOF_OUT_INIT_HIGH, pdata->name)))
		ret = gpio_export(pdata->gpio, false);
#endif
	
	if(ret < 0)
		return ret;
	
	if(pdata->flags & RTL819X_GPIO_ACTLOW)
		gpio_sysfs_set_active_low(pdata->gpio, 1);
	
	return ret;
}

static struct platform_driver rtl819x_8367r_reset_pin_driver = {
	.probe		= rtl819x_8367r_reset_pin_probe,
//	.remove		= rtl819x_8367r_reset_pin_remove,
	.driver		= {
		.name	= "rtl819x_8367r_reset_pin",
		.owner	= THIS_MODULE,
	}, 
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
module_platform_driver(rtl819x_8367r_reset_pin_driver);
#else
static int __init rtl819x_8367r_reset_pin_driver_init(void)
{
	return platform_driver_register(&rtl819x_8367r_reset_pin_driver);
}
module_init(rtl819x_8367r_reset_pin_driver_init);
#endif

static int rtl819x_8367r_i2c_pin_probe(struct platform_device *dev) {
	struct rtl819x_gpio_platdata *pdata = dev->dev.platform_data;
	int ret = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	ret = devm_gpio_request_one(&dev->dev, pdata->gpio, GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED, pdata->name);
#else
	if(!(ret = gpio_request_one(pdata->gpio, GPIOF_OUT_INIT_HIGH, pdata->name)))
		ret = gpio_export(pdata->gpio, false);
#endif
	
	if(ret < 0)
		return ret;
	
	if(pdata->flags & RTL819X_GPIO_ACTLOW)
		gpio_sysfs_set_active_low(pdata->gpio, 1);
	
	return ret;
}

static struct platform_driver rtl819x_8367r_i2c_pin_driver = {
	.probe		= rtl819x_8367r_i2c_pin_probe,
//	.remove		= rtl819x_8367r_i2c_pin_remove,
	.driver		= {
		.name	= "rtl819x_8367r_i2c_pin",
		.owner	= THIS_MODULE,
	}, 
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
module_platform_driver(rtl819x_8367r_i2c_pin_driver);
#else
static int __init rtl819x_8367r_i2c_pin_driver_init(void)
{
	return platform_driver_register(&rtl819x_8367r_i2c_pin_driver);
}
module_init(rtl819x_8367r_i2c_pin_driver_init);
#endif

#endif

#if defined(CONFIG_RTL_8197F) && (defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT))
#define P0_RGMII_TX_DELAY	0
#define P0_RGMII_RX_DELAY	5

void init_8197f_p0(void)
{
	REG32(PCRP0) = (REG32(PCRP0) & ~(ExtPHYID_MASK)) | (5 << ExtPHYID_OFFSET)
					| MIIcfg_RXER |  EnablePHYIf | MacSwReset;
	REG32(PCRP0) = (REG32(PCRP0) & ~(AutoNegoSts_MASK)) | (EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex);

	REG32(P0GMIICR) = (REG32(P0GMIICR) & ~(3<<23)) | (LINK_RGMII<<23);
	REG32(P0GMIICR) = (REG32(P0GMIICR) & ~((1<<4)|(7<<0))) | ((P0_RGMII_TX_DELAY<<4) | (P0_RGMII_RX_DELAY<<0) | (3<<CF_SEL_RGTXC_OFFSET));

	#ifdef CONFIG_RTL_CPU_TAG
	REG32(P0GMIICR) |= (CFG_CPUC_TAG|CFG_TX_CPUC_TAG);
	// enable port 0 router mode
	REG32(MACCR1) |= PORT0_ROUTER_MODE;        	
	#endif

	REG32(PITCR) |= (1<<0);   //00: embedded , 01L GMII/MII/RGMII
	REG32(MACCR) |= (1<<12);   //giga link		
	REG32(P0GMIICR) |=(Conf_done);

	REG32(PAD_CTRL_1) = (REG32(PAD_CTRL_1) & ~(PAD_P0_RGMII_DN_MASK|PAD_P0_RGMII_DP_MASK|PAD_P0_RGMII_MODE_MASK|PAD_P0_TX_E2_MASK))
					| (0x6 << PAD_P0_RGMII_DN_OFFSET) | (0x6 << PAD_P0_RGMII_DP_OFFSET);

	return;
}
#endif

#if (defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) \
	|| defined(CONFIG_RTL_8197F)) && !(defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT))
autoDownSpeed_t fe_ads[RTL8651_PHY_NUMBER];
#endif

#if defined(CONFIG_RTL_FE_AUTO_DOWN_SPEED)

#define restart_nway(port)	rtl8651_restartAsicEthernetPHYNway(port)
/*
 * NWAY time:
 *	TXDIS: 1.5sec
 *	FLGC: 1sec
 *	ABD+CACK+ACKD=(3+3+8)*24ms=0.4sec
 *	AutoX: 11 * 64ms = 0.7sec
 *	1.5+1+0.4+0.7=3.6sec, add some margin and take 6sec.
 */
#define ADS_NWAY_TIME		6
#define ADS_FAILED_COUNT_TO_DOWN_SPEED		3

//autoDownSpeed_t fe_ads[RTL8651_PHY_NUMBER];
uint32 ads_time;

inline int get_anerr(int port)
{
	uint32 data, phyid;

	phyid = rtl8651AsicEthernetTable[port].phyId;
	rtl8651_getAsicEthernetPHYReg( phyid, 30, &data );
	return ((data >> 15) & 1);
}

void ads_init(void)
{
	int i;

	/* Depiction 1 */
	ads_time = 0;
	for (i=ADS_PORT_START; i<RTL8651_PHY_NUMBER; i++) {
		fe_ads[i].time_flag = 0;
		fe_ads[i].down_speed = 0;
		fe_ads[i].fail_counter = 0;
		fe_ads[i].state = ADS_LINKDOWN;
		fe_ads[i].down_speed_renway = 0;
		fe_ads[i].cb_snr_down_speed = 0;
		fe_ads[i].cb_snr_cache_lpi_down_speed = 0;
		fe_ads[i].force_speed_by_nway = 0;
		fe_ads[i].an_to_force_100mf = 0;
	}
}

inline void set_nway_ability(int port)
{
	if (fe_ads[port].down_speed == 0)
		REG32(PCRP0+(port*4)) = (REG32(PCRP0+(port*4)) & ~AutoNegoSts_MASK) | 
			(NwayAbility100MF|NwayAbility100MH|NwayAbility10MF|NwayAbility10MH);
	else
		REG32(PCRP0+(port*4)) = (REG32(PCRP0+(port*4)) & ~AutoNegoSts_MASK) | 
			(NwayAbility10MF|NwayAbility10MH);
}

void ads_debug(void)
{
	int i;
	
	rtlglue_printf(" --> ads_time= %u\n", ads_time);	
	for (i=ADS_PORT_START; i<5; i++) {
		rtlglue_printf(" => [%d] time_flag= %u\n", i, fe_ads[i].time_flag);
		rtlglue_printf("       fail_counter= %u, anerr_down_speed= %d\n", fe_ads[i].fail_counter, fe_ads[i].down_speed);
		rtlglue_printf("       state= %u, anerr= %d, cb_snr_down_speed= %d\n", 
			fe_ads[i].state, fe_ads[i].anerr, fe_ads[i].cb_snr_down_speed);
		rtlglue_printf("       cb_snr_cache_lpi_down_speed= %d\n", 
			fe_ads[i].cb_snr_cache_lpi_down_speed);
	}
}

/*
 * this function is for the port unlinked issue with long cable.
 * we can check phy reg 30 bit 15 for AN error.
 * if the "AN error" count > 3  in a 6-second duration, we down speed to 10M.
 */
inline void ads_check(void)
{
	int i;
	unsigned long flags=0;

	SMP_LOCK_ETH(flags);
	
	ads_time++;
	
	for (i=ADS_PORT_START; i<RTL8651_PHY_NUMBER; i++) {

		/* Conditional expression 1 */
		if ((REG32(PCRP0+(i*4))&EnForceMode) ||(fe_ads[i].force_speed_by_nway==1)) {
			/* Depiction 2 */
			fe_ads[i].down_speed = 0;
			fe_ads[i].fail_counter = 0;
			fe_ads[i].state = ADS_LINKDOWN;
			fe_ads[i].anerr = get_anerr(i);
			continue;
		}

		/* Conditional expression 2 */
		if (fe_ads[i].state == ADS_LINKUP) {

			/* Conditional expression 3 */			
			if ((REG32(PSRP0+(i*4))&PortStatusLinkUp)==0) {			
				/* Depiction 3 */
				fe_ads[i].down_speed = 0;
				set_nway_ability(i);
				fe_ads[i].fail_counter = 0;
				restart_nway(i);
				fe_ads[i].anerr = get_anerr(i);
				fe_ads[i].state = ADS_LINKDOWN;
				//rtlglue_printf(" => ADS_D3: port %d: LINKDOWN (down_speed=%d)\n", i,fe_ads[i].down_speed);
			}
			continue;
		}
		
		/* Conditional expression 4 */
		if ((REG32(PSRP0+(i*4))&PortStatusLinkUp)!=0) {			
			/* Depiction 4 */
			//rtlglue_printf(" => ADS_D4: port %d: LINKUP (down_speed=%d)\n", i,fe_ads[i].down_speed);
			fe_ads[i].state = ADS_LINKUP;
			continue;
		}

		/* Conditional expression 5 */
		if (!((fe_ads[i].fail_counter) || (fe_ads[i].down_speed))) {

			fe_ads[i].anerr = get_anerr(i);
			
			/* Conditional expression 6 */
			if (fe_ads[i].anerr) {

				//rtlglue_printf(" => ADS_C6_Yes: port %d.\n", i);
				/* Depiction 5 */
				fe_ads[i].fail_counter++;
				fe_ads[i].time_flag = ads_time;
			}
			continue;
		}

		/* Conditional expression 7 */
		if ((ads_time - fe_ads[i].time_flag) > ADS_NWAY_TIME) {			
			/* unplug cable */
			/* Depiction 6 */
			fe_ads[i].down_speed = 0;
			set_nway_ability(i);
			fe_ads[i].fail_counter = 0;
			restart_nway(i);
			continue;
		}

		fe_ads[i].anerr = get_anerr(i);
		
		/* Conditional expression 8 */
		if (fe_ads[i].anerr) {
			//rtlglue_printf(" => ADS_C8: port %d: anerr= 1\n", i);
			
			/* abnormal happened again */
			/* Conditional expression 9 */
			if (fe_ads[i].fail_counter > ADS_FAILED_COUNT_TO_DOWN_SPEED) {
				//rtlglue_printf(" => ADS_C9: port %d: fail_counter= %d (>3)\n", i,fe_ads[i].fail_counter);
				/* accumulated count reach the treshold */
				/* Depiction 7 */
				fe_ads[i].down_speed ^= 1;
				set_nway_ability(i);
				fe_ads[i].fail_counter = 0;
				restart_nway(i);
			}
			else {
				/* keep accumulating */
				/* Depiction 8 */
				fe_ads[i].fail_counter++;
			}
			fe_ads[i].time_flag = ads_time;
		}
	}
	SMP_UNLOCK_ETH(flags);
}
#endif

#ifdef CONFIG_RTL_FORCE_MDIX
forceMdix_t fe_fmx[RTL8651_PHY_NUMBER];
/* threshold */
int link_up_down_interval = 2;
int thres_chg_force_mdix = 5;
int timeout_FORCEMDIX  = 5;

void fmx_init(void)
{
	//enable HW PTP timer
	REG32(PTPCR) = 0;
	REG32(STR1) = 1;

	memset(fe_fmx, 0, RTL8651_PHY_NUMBER * sizeof(forceMdix_t));
}

inline void fmx_check(void)
{
	int port, phyid;
	uint32 data;
	unsigned long flags=0;

	SMP_LOCK_ETH(flags);
	
	for (port = ADS_PORT_START; port < RTL8651_PHY_NUMBER; port++) {
		phyid = rtl8651AsicEthernetTable[port].phyId;
		if (fe_fmx[port].state == FMX_FORCEMDIX) {
			if (REG32(STR0) - fe_fmx[port].time1 > timeout_FORCEMDIX ) {
				fe_fmx[port].state = FMX_IDLE;
				rtl8651_getAsicEthernetPHYReg( phyid, 28, &data);
				data |= (0x3<<1);
				rtl8651_setAsicEthernetPHYReg( phyid, 28, data); // set auto MDI/MDIX
			}				
		} else if (fe_fmx[port].state == FMX_FAILCOUNTING) {
			if ( (REG32(STR0) - fe_fmx[port].time1) > (link_up_down_interval * (thres_chg_force_mdix+1)) ) {
				fe_fmx[port].state = FMX_IDLE;
				fe_fmx[port].fail_counter = 0;
			}			
		}

	}
	SMP_UNLOCK_ETH(flags);
}

void fmx_debug(void)
{
	int port;

	rtlglue_printf("link_up_down_interval= %d\n", link_up_down_interval);
	rtlglue_printf("thres_chg_force_mdix= %d\n", thres_chg_force_mdix);
	rtlglue_printf("timeout_FORCEMDIX= %d\n", timeout_FORCEMDIX);
	
	for (port=0; port<5; port++) {
		rtlglue_printf(" => port : [%d]\n", port);
		rtlglue_printf(" time1 = %d\n", fe_fmx[port].time1);
		rtlglue_printf(" state = %d\n", fe_fmx[port].state);
		rtlglue_printf(" fail_counter = %d\n", fe_fmx[port].fail_counter);
		rtlglue_printf(" mdimode = %d\n", fe_fmx[port].mdimode);
	}
}	
#endif

#define REG32_ANDOR(x,y,z)   (REG32(x)=(REG32(x)& (y))|(z))

// rtl865x_initAsicL2() is too long, move something out
void initAsic_PHY(void)
{
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT1234_RTL8212)
	{

		printk("\nEnable Port1~Port4 GigaPort.\n\n");
		/* Patch for 'RGMII port does not get descritpors'. Set to MII PHY mode first and later we'll change to RGMII mode again. */
		rtl865xC_setAsicEthernetMIIMode(0, LINK_MII_PHY);

		/*
			# According to Hardware SD: David & Maxod,

			Set Port5_GMII Configuration Register.
			- RGMII Output Timing compensation control : 0 ns
			- RGMII Input Timing compensation control : 0 ns
		*/
		rtl865xC_setAsicEthernetRGMIITiming(0, RGMII_TCOMP_0NS, RGMII_RCOMP_0NS);

		/* Set P1 - P4 to SerDes Interface. */
		WRITE_MEM32(PITCR, Port4_TypeCfg_SerDes | Port3_TypeCfg_SerDes | Port2_TypeCfg_SerDes | Port1_TypeCfg_SerDes );
	}
	else if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		/* Patch for 'RGMII port does not get descritpors'. Set to MII PHY mode first and later we'll change to RGMII mode again. */
		rtl865xC_setAsicEthernetMIIMode(RTL8651_MII_PORTNUMBER, LINK_MII_PHY);

		/*
			# According to Hardware SD: David & Maxod,

			Set Port5_GMII Configuration Register.
			- RGMII Output Timing compensation control : 0 ns
			- RGMII Input Timing compensation control : 0 ns
		*/
		rtl865xC_setAsicEthernetRGMIITiming(RTL8651_MII_PORTNUMBER, RGMII_TCOMP_0NS, RGMII_RCOMP_0NS);
	}

#if defined(CONFIG_RTL_8198C)
	rtl8651AsicEthernetTable[0].phyId = RTL8198C_PORT0_PHY_ID;	/* Default value of port 0's embedded phy id -- 8 */
	rtl8651AsicEthernetTable[1].phyId = 1;	/* Default value of port 1's embedded phy id -- 1 */
	rtl8651AsicEthernetTable[2].phyId = 2;	/* Default value of port 2's embedded phy id -- 2 */
	rtl8651AsicEthernetTable[3].phyId = 3;	/* Default value of port 3's embedded phy id -- 3 */
	rtl8651AsicEthernetTable[4].phyId = 4;	/* Default value of port 4's embedded phy id -- 4 */
	rtl8651AsicEthernetTable[0].isGPHY = TRUE;
	rtl8651AsicEthernetTable[1].isGPHY = TRUE;
	rtl8651AsicEthernetTable[2].isGPHY = TRUE;
	rtl8651AsicEthernetTable[3].isGPHY = TRUE;
	rtl8651AsicEthernetTable[4].isGPHY = TRUE;

#elif defined(CONFIG_RTL_8197F)
	rtl8651AsicEthernetTable[0].phyId = RTL8198C_PORT0_PHY_ID;	/* Default value of port 0's embedded phy id -- 8 */
	rtl8651AsicEthernetTable[1].phyId = 1;	/* Default value of port 1's embedded phy id -- 1 */
	rtl8651AsicEthernetTable[2].phyId = 2;	/* Default value of port 2's embedded phy id -- 2 */
	rtl8651AsicEthernetTable[3].phyId = 3;	/* Default value of port 3's embedded phy id -- 3 */
	rtl8651AsicEthernetTable[4].phyId = 4;	/* Default value of port 4's embedded phy id -- 4 */
	rtl8651AsicEthernetTable[0].isGPHY = FALSE;
	rtl8651AsicEthernetTable[1].isGPHY = FALSE;
	rtl8651AsicEthernetTable[2].isGPHY = FALSE;
	rtl8651AsicEthernetTable[3].isGPHY = FALSE;
	rtl8651AsicEthernetTable[4].isGPHY = FALSE;

#ifdef CONFIG_RTL_8211F_SUPPORT
	rtl8651AsicEthernetTable[0].phyId = PORT0_RGMII_PHYID; 
	rtl8651AsicEthernetTable[0].isGPHY = TRUE;
#endif

#else
	memset( &rtl8651AsicEthernetTable[0], 0, ( RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum ) * sizeof(rtl8651_tblAsic_ethernet_t) );
	/* Record the PHYIDs of physical ports. Default values are 0. */
	rtl8651AsicEthernetTable[0].phyId = 0;	/* Default value of port 0's embedded phy id -- 0 */
	rtl8651AsicEthernetTable[0].isGPHY = FALSE;

	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT1234_RTL8212)
	{
		rtl8651AsicEthernetTable[1].phyId = rtl8651_tblAsicDrvPara.externalPHYId[1];
		rtl8651AsicEthernetTable[2].phyId = rtl8651_tblAsicDrvPara.externalPHYId[2];
		rtl8651AsicEthernetTable[3].phyId = rtl8651_tblAsicDrvPara.externalPHYId[3];
		rtl8651AsicEthernetTable[4].phyId = rtl8651_tblAsicDrvPara.externalPHYId[4];
		rtl8651AsicEthernetTable[1].isGPHY = TRUE;
		rtl8651AsicEthernetTable[2].isGPHY = TRUE;
		rtl8651AsicEthernetTable[3].isGPHY = TRUE;
		rtl8651AsicEthernetTable[4].isGPHY = TRUE;
	} else
	{	/* USE internal 10/100 PHY */
		rtl8651AsicEthernetTable[1].phyId = 1;	/* Default value of port 1's embedded phy id -- 1 */
		rtl8651AsicEthernetTable[2].phyId = 2;	/* Default value of port 2's embedded phy id -- 2 */
		rtl8651AsicEthernetTable[3].phyId = 3;	/* Default value of port 3's embedded phy id -- 3 */
		rtl8651AsicEthernetTable[4].phyId = 4;	/* Default value of port 4's embedded phy id -- 4 */
		rtl8651AsicEthernetTable[1].isGPHY = FALSE;
		rtl8651AsicEthernetTable[2].isGPHY = FALSE;
		rtl8651AsicEthernetTable[3].isGPHY = FALSE;
		rtl8651AsicEthernetTable[4].isGPHY = FALSE;
	}

	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		rtl8651AsicEthernetTable[RTL8651_MII_PORTNUMBER].phyId = rtl8651_tblAsicDrvPara.externalPHYId[5];
		rtl8651AsicEthernetTable[RTL8651_MII_PORTNUMBER].isGPHY = TRUE;
		rtl8651_setAsicEthernetMII(	rtl8651AsicEthernetTable[RTL8651_MII_PORTNUMBER].phyId,
									P5_LINK_RGMII,
									TRUE );
	}
#endif

#ifdef CONFIG_RTL8196C_REVISION_B
	if (REG32(REVR) == RTL8196C_REVISION_B)
		Setting_RTL8196C_PHY();

#elif defined(CONFIG_RTL_8198C)
	Setting_RTL8198C_GPHY();

#elif defined(CONFIG_RTL_8196E)
	Setting_RTL8196E_PHY();

#elif defined(CONFIG_RTL_8881A)
	Setting_RTL8881A_PHY();

#elif defined(CONFIG_RTL_819XD)
	Setting_RTL8197D_PHY();

#elif defined(CONFIG_RTL_8198)
	Setting_RTL8198_GPHY();

#elif defined(CONFIG_RTL_8197F)
	Setting_RTL8197F_PHY();

#endif
}

void disable_unused_phy_power(void)
{
	extern uint32 rtl819x_bond_option(void);
	uint32 statCtrlReg0, port_mask=0;
	int32 index;

	#if defined(CONFIG_RTL_8196E) && defined(CONFIG_RTL_ULINKER) /* disable unused port for saving power */
	port_mask = 0xF;
	#endif
	
	#if  defined(CONFIG_RTL_8196E) //mark_es
	if ( ((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196ES)  ||
		((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196ES1)  ||
		((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196ES2)  ||
		((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196ES3)  )
	{
		port_mask = 0x1E;
	}
	#endif

	#if defined(CONFIG_RTL_8197F)
	if (rtl819x_bond_option() == 3){ /* 8197FS */
		port_mask = 0xE;
 	}
	#endif

	for (index=0; index<RTL8651_PHY_NUMBER; index++) {

		if (!(port_mask & BIT(index)))
			continue;
		
		/* read current PHY reg 0 value */
		rtl8651_getAsicEthernetPHYReg( index, 0, &statCtrlReg0 );

		REG32(PCRP0+(index*4)) |= EnForceMode;
		statCtrlReg0 |= POWER_DOWN;

		/* write PHY reg 0 */
		rtl8651_setAsicEthernetPHYReg( index, 0, statCtrlReg0 );
	}
	return;
}

/*=========================================
  * init Layer2 Asic
  * rtl865x_initAsicL2 mainly configure basic&L2 Asic.
  * =========================================*/
int32 rtl865x_initAsicL2(rtl8651_tblAsic_InitPara_t *para)
{
	int32 index;

#if defined (CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	uint32 token= 0, tick = 0, hiThreshold = 0;
	uint32 maccrData =0, dataTemp = 0;
#endif

#if (defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)) && !(defined(CONFIG_RTL_8211F_SUPPORT))
	unsigned int P0phymode, P0miimode, P0txdly, P0rxdly;
#endif

#ifdef CONFIG_RTL8196C_ETH_IOT
	port_link_sts = 0;
	port_linkpartner_eee = 0;
#endif

#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
	rtl865x_probeSdramSize();
#endif

	ASICDRV_INIT_CHECK(_rtl8651_initAsicPara(para));

	initAsic_PHY();

#ifdef CONFIG_8198_PORT5_RGMII
	init_8198_p5();
#endif

#if defined(CONFIG_RTL_8367R_SUPPORT)
	init_8367r();

#elif defined(CONFIG_RTL_8198C_8367RB)
	init_8367rb_for_8198c();

#elif defined(CONFIG_RTL_8370_SUPPORT)
	init_8370();

#elif defined(CONFIG_RTL_8325D_SUPPORT)
	init_8325d();

#elif defined(CONFIG_RTL_83XX_SUPPORT)
	init_83XX();
#endif

	/* 	2006.12.12
		We turn on bit.10 (ENATT2LOG).

		* Current implementation of unnumbered pppoe in multiple session
		When wan type is multiple-session, and one session is unnumbered pppoe, WAN to unnumbered LAN is RP --> NPI.
		And user adds ACL rule to trap dip = unnumbered LAN to CPU.

		However, when pktOpApp of this ACL rule is set, it seems that this toCPU ACL does not work.
		Therefore, we turn on this bit (ENATT2LOG) to trap pkts (WAN --> unnumbered LAN) to CPU.

	*/
	WRITE_MEM32( SWTCR1, READ_MEM32( SWTCR1 ) | EnNATT2LOG );

	/*
	  * Turn on ENFRAG2ACLPT for Rate Limit. For those packets which need to be trapped to CPU, we turn on
	  * this bit to tell ASIC ACL and Protocol Trap to process these packets. If this bit is not turnned on, packets
	  * which need to be trapped to CPU will not be processed by ASIC ACL and Protocol Trap.
	  * NOTE: 	If this bit is turned on, the backward compatible will disable.
	  *																- chhuang
	  */
	WRITE_MEM32( SWTCR1, READ_MEM32( SWTCR1 ) | ENFRAGTOACLPT );

#ifdef CONFIG_RTL865X_LIGHT_ROMEDRV
	WRITE_MEM32( SWTCR1, READ_MEM32( SWTCR1 ) | L4EnHash1 );    /*Turn on Napt Enhanced hash1*/
#endif

	/*
	  * Cannot turn on EnNAP8651B due to:
	  * If turn on, NAT/LP/ServerPort will reference nexthop. This will result in referecing wrong L2 entry when
	  * the destination host is in the same subnet as WAN.
	  */

	rtl8651_totalExtPortNum=3; //this replaces all RTL8651_EXTPORT_NUMBER defines
	rtl8651_allExtPortMask = 0x7<<RTL8651_MAC_NUMBER; //this replaces all RTL8651_EXTPORTMASK defines

	//Disable layer2, layer3 and layer4 function
	//Layer 2 enabled automatically when a VLAN is added
	//Layer 3 enabled automatically when a network interface is added.
	//Layer 4 enabled automatically when an IP address is setup.
	rtl8651_setAsicOperationLayer(1);
	rtl8651_clearAsicCommTable();
	rtl8651_clearAsicL2Table();
	//rtl8651_clearAsicAllTable();//MAY BE OMITTED. FULL_RST clears all tables already.
	rtl8651_setAsicSpanningEnable(FALSE);

#ifdef RTL865XB_URL_FILTER
	WRITE_MEM32( SWTCR1, READ_MEM32( SWTCR1 ) | EN_51B_CPU_REASON );	/* Use 8650B's new reason bit definition. */
#endif

    /* Init PHY LED style */
#ifdef CONFIG_RTL_819X
#if defined (CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)

#elif defined(CONFIG_RTL_8881A)

	#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)
	REG32(PIN_MUX_SEL) &= ~( (3<<8) | (3<<10) | (1<<15) );
	REG32(PIN_MUX_SEL2) &= ~ ((3<<0) | (3<<3) | (3<<6) | (3<<9) | (7<<15) );  //S0-S3, P0-P1
	
	#else
	#if defined(CONFIG_I2C_GPIO_MFI_COPROCESSOR_DRIVER)
	REG32(PIN_MUX_SEL) &= ~( (1<<15) ); 
	REG32(PIN_MUX_SEL2) &= ~ ((3<<0) | (3<<3) | (3<<6) | (3<<9) | (3<<12) | (7<<15) );  //S0-S3, P0-P1
	#else
	REG32(PIN_MUX_SEL) &= ~( (3<<8) | (3<<10) | (3<<3) | (1<<15) );  //let P0 to mii mode
	REG32(PIN_MUX_SEL2) &= ~ ((3<<0) | (3<<3) | (3<<6) | (3<<9) | (3<<12) | (7<<15) );  //S0-S3, P0-P1
	#endif
	#endif
	REG32(LEDCREG)  = (2<<20) | (0<<18) | (0<<16) | (0<<14) | (0<<12) | (0<<10) | (0<<8);  //P0-P5

#elif defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	/*
		#LED = direct mode
		set mode 0x0
		swwb 0xbb804300 21-20 0x2 19-18 $mode 17-16 $mode 15-14 $mode 13-12 $mode 11-10 $mode 9-8 $mode
	*/
	#ifdef 	CONFIG_RTK_VOIP_BOARD	
	//for GMII/RGMII
	//REG32(PIN_MUX_SEL) &= ~( (3<<8) | (3<<10) | (3<<3) | (1<<15) );  //let P0 to mii mode
	REG32(PIN_MUX_SEL2) &= ~ ((3<<0) | (3<<3) | (3<<6) | (3<<9) | (3<<12) );  //LED0~LED4

	#elif defined(CONFIG_RTL_8367MB_SUPPORT)
	REG32(PIN_MUX_SEL) &= ~( (3<<8) | (1<<15) );
	REG32(PIN_MUX_SEL2) &= ~ ((3<<0) | (3<<3) | (3<<6) | (3<<9) | (3<<12) | (7<<15) );  //S0-S3, P0-P1

	#elif defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8370_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)
	REG32(PIN_MUX_SEL) &= ~( (3<<8) | (3<<10) | (1<<15) );
	REG32(PIN_MUX_SEL2) &= ~ ((3<<0) | (3<<3) | (3<<6) | (3<<9) | (3<<12) | (7<<15) );  //S0-S3, P0-P1
	
	#elif defined(CONFIG_RTL_8325D_SUPPORT)
	REG32(PIN_MUX_SEL) &= ~( (3<<8) | (3<<3) | (1<<15) );
	REG32(PIN_MUX_SEL2) &= ~ ((3<<0) | (3<<3) | (3<<6) | (3<<9) | (3<<12) | (7<<15) );  //S0-S3, P0-P1

	#else
	#ifndef CONFIG_MTD_NAND
	REG32(PIN_MUX_SEL) &= ~( (3<<8) | (3<<10) | (3<<3) | (1<<15) );  //let P0 to mii mode
	#endif
#if defined(CONFIG_RTL_8196E) && defined(CONFIG_APPLE_MFI_SUPPORT)
		
	if (((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196E3))
	{
		REG32(PIN_MUX_SEL2) &= ~ ((3<<12) | (7<<15) );  //P4
	}
	else if (((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196EU3))
	{
		REG32(PIN_MUX_SEL2) &= ~ ((7<<15)); 
	}
	else
		REG32(PIN_MUX_SEL2) &= ~ ((3<<0) | (3<<3) | (3<<6) | (3<<9) | (3<<12) | (7<<15) );  //S0-S3, P0-P1
		
#elif defined(CONFIG_RTL_96E_GPIOB5_RESET)
	REG32(PIN_MUX_SEL2) &= ~ ((3<<12));  
#else
	REG32(PIN_MUX_SEL2) &= ~ ((3<<0) | (3<<3) | (3<<6) | (3<<9) | (3<<12) | (7<<15) );  //S0-S3, P0-P1
#endif
	#endif
	REG32(LEDCREG)  = (2<<20) | (0<<18) | (0<<16) | (0<<14) | (0<<12) | (0<<10) | (0<<8);  //P0-P5

#elif defined (CONFIG_RTL_8198C)
#ifndef CONFIG_USING_JTAG
	REG32(PIN_MUX_SEL3) = (REG32(PIN_MUX_SEL3) & ~ (0x7FFF)) | ((1<<0) | (1<<3) | (1<<6) | (1<<9) | (1<<12));  //LED0~LED4
#else
	REG32(PIN_MUX_SEL3) = (REG32(PIN_MUX_SEL3) & ~ (0x7FFF));
#endif
	REG32(LEDCR0) = (REG32(LEDCR0) & ~ LEDTOPOLOGY_MASK) |LEDMODE_DIRECT;
	REG32(DIRECTLCR) = (REG32(DIRECTLCR) & ~ LEDONSCALEP0_MASK) |(7<<LEDONSCALEP0_OFFSET);

#elif defined(CONFIG_RTL_8197F)
	#if !(defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT))
	{
		extern uint32 rtl819x_bond_option(void);
		if (rtl819x_bond_option() != 3)	
			REG32(PIN_MUX_SEL13) = 0;

		#if !defined(CONFIG_RTL_8211F_SUPPORT)
		REG32(PIN_MUX_SEL14) = 0;
		#endif
	}
	#endif
#if defined(CONFIG_APPLE_MFI_SUPPORT)
    REG32(PIN_MUX_SEL15) = ((REG32(PIN_MUX_SEL15)&(0xFFFFFF00)) | (0x77));
#endif 
    REG32(LEDCR0) = (REG32(LEDCR0) & ~LEDTOPOLOGY_MASK) | LEDMODE_DIRECT;	
#endif
#endif

#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
	//MAC Control (0xBC803000)
/*	WRITE_MEM32(MACCR,READ_MEM32(MACCR)&~DIS_IPG);//Set IFG range as 96+-4bit time*/
	WRITE_MEM32(MACCR,READ_MEM32(MACCR)&~NORMAL_BACKOFF);//Normal backoff
	WRITE_MEM32(MACCR,READ_MEM32(MACCR)&~BACKOFF_EXPONENTIAL_3);//Exponential parameter is 9
	WRITE_MEM32(MACCR,READ_MEM32(MACCR)|INFINITE_PAUSE_FRAMES);//send pause frames infinitely.
	WRITE_MEM32(MACCR,READ_MEM32(MACCR)|DIS_MASK_CGST);
#endif

	miiPhyAddress = -1;		/* not ready to use mii port 5 */

/*	WRITE_MEM32(MACCR,READ_MEM32(MACCR)&~(EN_FX_P4 | EN_FX_P3 | EN_FX_P2 | EN_FX_P1 | EN_FX_P0));//Disable FX mode (UTP mode)*/
	/* Initialize MIB counters */
	rtl8651_clearAsicCounter();

	rtl865xC_setNetDecisionPolicy( NETIF_VLAN_BASED );	/* Net interface Multilayer-Decision-Based Control -- Set to VLAN-Based mode. */
	//rtl865xC_setNetDecisionPolicy( NETIF_PORT_BASED );	/* Net interface Multilayer-Decision-Based Control -- Set to port-Based mode. */
	//WRITE_MEM32(PLITIMR,0);

	/*FIXME:Hyking init in Layer4 2*/
	//WRITE_MEM32(SWTCR0,READ_MEM32(SWTCR0)&~NAPTR_NOT_FOUND_DROP);//When reverse NAPT entry not found, CPU process it.
	//rtl8651_setAsicNaptAutoAddDelete(FALSE, TRUE);
	WRITE_MEM32( VCR0, READ_MEM32( VCR0 ) & (~EN_ALL_PORT_VLAN_INGRESS_FILTER) );		/* Disable VLAN ingress filter of all ports */ /* Please reference to the maintis bug 2656# */
#ifdef CONFIG_RTL_VLAN_PASSTHROUGH_SUPPORT    
    WRITE_MEM32( VCR0, READ_MEM32( VCR0 ) | EN_1QTAGVIDIGNORE );		/* enable ignore 802.1q vlan */
#endif
	/*WRITE_MEM32( VCR0, READ_MEM32( VCR0 ) & (~EN_ALL_PORT_VLAN_INGRESS_FILTER) );*/		/* Enable VLAN ingress filter of all ports */
	WRITE_MEM32(SWTCR0,READ_MEM32(SWTCR0)&~WAN_ROUTE_MASK);//Set WAN route toEnable (Allow traffic from WAN port to WAN port)
	WRITE_MEM32(SWTCR0,READ_MEM32(SWTCR0)|NAPTF2CPU);//When packet destination to switch. Just send to CPU
	WRITE_MEM32(SWTCR0,(READ_MEM32(SWTCR0)&(~LIMDBC_MASK))|LIMDBC_VLAN);//When packet destination to switch. Just send to CPU

	/*FIXME:Hyking init in Layer3 1*/
	//rtl8651_setAsicMulticastEnable(TRUE); /* Enable multicast table */

	/* Enable unknown unicast / multicast packet to be trapped to CPU. */
//	WRITE_MEM32( FFCR, READ_MEM32( FFCR ) | EN_UNUNICAST_TOCPU );
	WRITE_MEM32( FFCR, READ_MEM32( FFCR ) & ~EN_UNUNICAST_TOCPU );
	WRITE_MEM32( FFCR, READ_MEM32( FFCR ) | EN_UNMCAST_TOCPU );
/*	WRITE_MEM32(SWTMCR,READ_MEM32(SWTMCR)&~MCAST_TO_CPU);*/
/*	WRITE_MEM32(SWTMCR,READ_MEM32(SWTMCR)|EN_BCAST);//Enable special broadcast handling*/
/*	WRITE_MEM32(SWTMCR,READ_MEM32(SWTMCR)&~BCAST_TO_CPU);//When EN_BCAST enables, this bit is invalid*/
/*	WRITE_MEM32(SWTMCR,READ_MEM32(SWTMCR)&~BRIDGE_PKT_TO_CPU);//Current no bridge protocol is supported*/

	/*FIXME:Hyking init in Layer3 1*/
	//WRITE_MEM32(ALECR, READ_MEM32(ALECR)|(uint32)EN_PPPOE);//enable PPPoE auto encapsulation
	WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L2_CHKSUM_ERR);//Don't allow chcksum error pkt be forwarded.

	WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L3_CHKSUM_ERR);
	WRITE_MEM32(CSCR,READ_MEM32(CSCR)&~ALLOW_L4_CHKSUM_ERR);
	WRITE_MEM32(CSCR,READ_MEM32(CSCR)|EN_ETHER_L3_CHKSUM_REC); //Enable L3 checksum Re-calculation
	WRITE_MEM32(CSCR,READ_MEM32(CSCR)|EN_ETHER_L4_CHKSUM_REC); //Enable L4 checksum Re-calculation

/*	WRITE_MEM32(MISCCR,READ_MEM32(MISCCR)&~FRAG2CPU);//IP fragment packet does not need to send to CPU when initial ASIC
	WRITE_MEM32(MISCCR,READ_MEM32(MISCCR)&~MULTICAST_L2_MTU_MASK);
	WRITE_MEM32(MISCCR,READ_MEM32(MISCCR)|(1522&MULTICAST_L2_MTU_MASK));//Multicast packet layer2 size 1522 at most*/
	/* follow RTL865xB's convention, we use 1522 as default multicast MTU */

	/*FIXME:Hyking init in Layer3 1*/
	//rtl8651_setAsicMulticastMTU(1522);

	//Set all Protocol-Based Reg. to 0

	for (index=0;index<32;index++)
		WRITE_MEM32(PBVCR0+index*4,  0x00000000);
	//Enable TTL-1
	/*FIXME:Hyking init in Layer3 1*/
	//WRITE_MEM32(TTLCR,READ_MEM32(TTLCR)|(uint32)EN_TTL1);//Don't hide this router. enable TTL-1 when routing on this gateway.


	for (index=0; index<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; index++) {

		rtl865xC_setAsicSpanningTreePortState(index, RTL8651_PORTSTA_FORWARDING);

		rtl8651_setAsicEthernetBandwidthControl(index, TRUE, RTL8651_BC_FULL);
		rtl8651_setAsicEthernetBandwidthControl(index, FALSE, RTL8651_BC_FULL);
	}

	// 08-15-2012, set TRXRDY bit in rtl865x_start() in rtl865x_asicCom.c
	/* Enable TX/RX After ALL ASIC configurations are done */
	//WRITE_MEM32( SIRR, READ_MEM32(SIRR)| TRXRDY );

	/* Initiate Bandwidth control backward compatible mode : Set all of them to FULL-Rate */
	{
		int32 portIdx, typeIdx;
		_rtl865xB_BandwidthCtrlMultiplier = _RTL865XB_BANDWIDTHCTRL_X1;
		for ( portIdx = 0 ; portIdx < RTL8651_PORT_NUMBER ; portIdx ++ )
		{
			for ( typeIdx = 0 ; typeIdx < _RTL865XB_BANDWIDTHCTRL_CFGTYPE ; typeIdx ++ )
			{
				_rtl865xB_BandwidthCtrlPerPortConfiguration[portIdx][typeIdx] = BW_FULL_RATE;
			}
		}
		/* Sync the configuration to ASIC */
		_rtl8651_syncToAsicEthernetBandwidthControl();
	}

	/* ===============================
	    =============================== */
	{
		uint32 port;
		uint32 maxPort;

		maxPort =	(rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)?
					RTL8651_MAC_NUMBER:
					RTL8651_PHY_NUMBER;

		for ( port = 0 ; port < maxPort ; port ++ )
		{
			rtl8651_setAsicFlowControlRegister(port, TRUE);
			rtl865xC_setAsicPortPauseFlowControl(port, TRUE, TRUE);
		}
	}

	/* ===============================
	 	EEE setup
	    =============================== */
#if defined(CONFIG_RTL_EEE_DISABLED) || defined(CONFIG_MP_PSD_SUPPORT) || defined(CONFIG_RTL_8370_SUPPORT)
	eee_enabled = 0;
#else
	eee_enabled = 1;
#endif

	if (eee_enabled) {	    
		enable_EEE();
	}
	else {
		disable_EEE();
	}
		
	/* ===============================
	 	(1) Handling port 0.
	    =============================== */
	rtl8651_restartAsicEthernetPHYNway(0);	/* Restart N-way of port 0 to let embedded phy patch take effect. */

	/* ===============================
	 	(2) Handling port 1 - port 4.
	    =============================== */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT1234_RTL8212)
	{

	} else
	{
		/* Restart N-way of port 1 - port 4 to let embedded phy patch take effect. */
		{
			uint32 port;

			/* Restart N-way of port 1 - port 4 */
			for ( port = 1; port < RTL8651_PHY_NUMBER; port++ )
			{
				rtl8651_restartAsicEthernetPHYNway(port);
			}
		}
	}

	/* ===============================
		(3) Handling port 5.
	    =============================== */


	/* =====================
		QoS-related patch
	    ===================== */
	{
		#define DEFAULT_ILB_UBOUND 0x3FBE  /*added by Mark for suggested Leacky Bucket value*/
		#define DEFAULT_ILB_LBOUND 0x3FBC

		uint32 i;
		#if 0 
		/*desigener suggest: hiThreshold=3*token, 
		in Setting_RTL8197D_PHY or some function similar, it is set as designeer suggested*/
		uint32 token, tick, hiThreshold,i;
		rtl8651_getAsicLBParameter( &token, &tick, &hiThreshold );
		hiThreshold = 0x400;	/* New default vlue. */
		rtl8651_setAsicLBParameter( token, tick, hiThreshold );
		#endif
		/*Mantis(2307): Ingress leaky bucket need to be initized with suggested value . added by mark*/
		WRITE_MEM32( ILBPCR1, DEFAULT_ILB_UBOUND << UpperBound_OFFSET | DEFAULT_ILB_LBOUND << LowerBound_OFFSET );
		for(i=0;i<=(RTL8651_PHY_NUMBER/2);i++) /*Current Token Register is 2 bytes per port*/
			WRITE_MEM32( ILB_CURRENT_TOKEN + 4*i , DEFAULT_ILB_UBOUND << UpperBound_OFFSET | DEFAULT_ILB_UBOUND );

	}

	#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
	/* Initiate Queue Management system */
	_rtl865xC_QM_init();
	#endif

	/*
		Init QUEUE Number configuration for RTL865xC : For Port 0~5 and CPU Port - All ports have 1 queue for each.
	*/
	{
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		for ( index = PHY0 ; index <= CPU ; index ++ )
		{
			/*
				Init QUEUE Number configuration for RTL865xC : For Port 0~5 and CPU Port - All ports have 1 queue for each.
			*/
			rtl8651_setAsicOutputQueueNumber(index, QNUM1	/* According to DataSheet of QNUMCR : All ports use 1 queue by default */);
		}

		for ( index = 0 ; index <= 8 ; index ++ )
		{
			REG32(V4VLDSCPCR0 + (index * 4)) = (REG32(V4VLDSCPCR0 + (index * 4)) & ~(CF_IPM4DSCP_ACT_MASK | CF_IPM4PRI_ACT_MASK)) 
				| ((CF_IPM4DSCP_ACT_REMARK << CF_IPM4DSCP_ACT_OFFSET) | (CF_IPM4PRI_ACT_ORIG << CF_IPM4PRI_ACT_OFFSET));
		}
#elif !defined(CONFIG_RTL_819XD) && !defined(CONFIG_RTL_8196E)
	/*	The default value was just as same as what we want	*/
		rtl865xC_lockSWCore();

		/* Enable Flow Control for each QUEUE / Port */
		{
			int32 port, queue;

			for ( port = PHY0 ; port <= CPU ; port ++ )
			{
				/*	Set the Ingress & Egress bandwidth control NO LIMIT */
				/* Has been done by _rtl8651_syncToAsicEthernetBandwidthControl() */
				/*
				rtl8651_setAsicPortIngressBandwidth(port, (IBWC_ODDPORT_MASK>>IBWC_ODDPORT_OFFSET));
				rtl8651_setAsicPortEgressBandwidth(port, (APR_MASK>>APR_OFFSET));
				*/

				/*
					Init QUEUE Number configuration for RTL865xC : For Port 0~5 and CPU Port - All ports have 1 queue for each.
				*/
				rtl8651_setAsicOutputQueueNumber(port, QNUM1	/* According to DataSheet of QNUMCR : All ports use 1 queue by default */);

				for ( queue = QUEUE0 ; queue <= QUEUE5 ; queue ++ )
				{
					rtl8651_setAsicQueueFlowControlConfigureRegister( port, queue, TRUE);
					#if 0
					/*	Please keep the queue setting as above.
					*	Default set queue FC disable will corrupt wlan<->lan performance
					*	2011/01/10 ZhaoBo
					*/
					rtl8651_setAsicQueueFlowControlConfigureRegister( port, queue, FALSE);
					#endif
				}
			}
		}

		rtl865xC_waitForOutputQueueEmpty();
		rtl8651_resetAsicOutputQueue();
		rtl865xC_unLockSWCore();
#endif
	}

	/* set default include IFG */
	WRITE_MEM32( QOSFCR, BC_withPIFG_MASK);

		{
		#if defined(CONFIG_RTL_8197F)
		rtl8651_setAsicPriorityDecision(2, 1, 1, 1, 1, 1);
		#else
		rtl8651_setAsicPriorityDecision(2, 1, 1, 1, 1);
		#endif

		WRITE_MEM32(PBPCR, 0);

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		for(index=0;index<=CPU;index++)
			rtl8651_setAsicPortBasedFlowControlRegister(PHY0+index,rtl_portFCON , rtl_portFCOFF);

#elif defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
		/* Set the threshold value for qos sytem */
		_rtl865x_setQosThresholdByQueueIdx(QNUM_IDX_123);
#endif

	/*	clear dscp priority assignment, otherwise, pkt with dscp value 0 will be assign priority 1		*/
		WRITE_MEM32(DSCPCR0,0);
		WRITE_MEM32(DSCPCR1,0);
		WRITE_MEM32(DSCPCR2,0);
		WRITE_MEM32(DSCPCR3,0);
		WRITE_MEM32(DSCPCR4,0);
		WRITE_MEM32(DSCPCR5,0);
		WRITE_MEM32(DSCPCR6,0);
	}

#if 1
#if defined(CONFIG_RTL_8198_NFBI_BOARD)
    //WRITE_MEM32(PIN_MUX_SEL_2, 0); //for led control

	REG32(PCRP0) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	REG32(PCRP1) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	REG32(PCRP2) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	REG32(PCRP3) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	REG32(PCRP4) &= (0xFFFFFFFF-(0x00000000|MacSwReset));
	REG32(PCRP5) &= (0xFFFFFFFF-(0x00000000|MacSwReset));

	REG32(PCRP0) = REG32(PCRP0) | (0 << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset;
	REG32(PCRP1) = REG32(PCRP1) | (1 << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset;
	REG32(PCRP2) = REG32(PCRP2) | (2 << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset;
	REG32(PCRP3) = REG32(PCRP3) | (3 << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset;
	REG32(PCRP4) = REG32(PCRP4) | (4 << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset;

	//port5 STP forwarding?
	REG32(PITCR) = REG32(PITCR) & 0xFFFFF3FF; //configure port 5 to be a MII interface

	// for EMI issue, use "GMII/MII MAC auto mode" instead
	//rtl865xC_setAsicEthernetMIIMode(5, LINK_MII_PHY); //port 5 MII PHY mode
	rtl865xC_setAsicEthernetMIIMode(5, LINK_MII_MAC); //port 5 MII MAC mode
	REG32(P5GMIICR) = REG32(P5GMIICR) | 0x40; //Conf_done=1

#if defined(RTL8198_NFBI_PORT5_GMII) //GMII mode
           #define GMII_PIN_MUX 0xc0
           REG32(PIN_MUX_SEL)= REG32(PIN_MUX_SEL)&(~(GMII_PIN_MUX));
	REG32(PCRP5) = 0 | (0x10<<ExtPHYID_OFFSET) |
			EnForceMode| ForceLink|ForceSpeed1000M|ForceDuplex |
			MIIcfg_RXER | EnablePHYIf | MacSwReset;
#else //MII mode
	REG32(PCRP5) = 0 | (0x10<<ExtPHYID_OFFSET) |
			EnForceMode| ForceLink|ForceSpeed100M |ForceDuplex |
			MIIcfg_RXER | EnablePHYIf | MacSwReset;
#endif

#elif defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	#define GMII_PIN_MUX 0xf00
       #if defined(CONFIG_8198_PORT5_GMII) || defined(CONFIG_8198_PORT5_RGMII)
           REG32(PIN_MUX_SEL)= REG32(PIN_MUX_SEL)&(~(GMII_PIN_MUX));
       #endif
	//WRITE_MEM32(PIN_MUX_SEL_2, 0);

#ifdef CONFIG_GIGABYTE_PHY_LINK_MODE_10_100 // use 10/100 only
       WRITE_MEM32(PCRP0, READ_MEM32(PCRP0) & ~(1<<22));
       WRITE_MEM32(PCRP1, READ_MEM32(PCRP1) & ~(1<<22));
       WRITE_MEM32(PCRP2, READ_MEM32(PCRP2) & ~(1<<22));
       WRITE_MEM32(PCRP3, READ_MEM32(PCRP3) & ~(1<<22));
       WRITE_MEM32(PCRP4, READ_MEM32(PCRP4) & ~(1<<22));
#endif

       WRITE_MEM32(PCRP0, READ_MEM32(PCRP0) & ~MacSwReset);
       WRITE_MEM32(PCRP1, READ_MEM32(PCRP1) & ~MacSwReset);
       WRITE_MEM32(PCRP2, READ_MEM32(PCRP2) & ~MacSwReset);
       WRITE_MEM32(PCRP3, READ_MEM32(PCRP3) & ~MacSwReset);
       WRITE_MEM32(PCRP4, READ_MEM32(PCRP4) & ~MacSwReset);

       WRITE_MEM32(PCRP0, READ_MEM32(PCRP0) | ((0<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset) ); /* Jumbo Frame */
       WRITE_MEM32(PCRP1, READ_MEM32(PCRP1) | ((1<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset) ); /* Jumbo Frame */
       WRITE_MEM32(PCRP2, READ_MEM32(PCRP2) | ((2<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset) ); /* Jumbo Frame */
       WRITE_MEM32(PCRP3, READ_MEM32(PCRP3) | ((3<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset) ); /* Jumbo Frame */
       WRITE_MEM32(PCRP4, READ_MEM32(PCRP4) | ((4<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset) ); /* Jumbo Frame */

#if defined(PORT5_RGMII_GMII)
		if(ExtP5GigaPhyMode)
		{
			REG32(PCRP5) &= (0x83FFFFFF-(0x00000000|MacSwReset));
			REG32(PCRP5) = REG32(PCRP5) | (GIGA_P5_PHYID << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset | 1<<20;
			REG32(P5GMIICR) = REG32(P5GMIICR) | Conf_done ;//| P5txdely;
		}
#endif
#if defined(CONFIG_8198_PORT5_GMII)
		REG32(PITCR) = REG32(PITCR) & 0xFFFFF3FF; //configure port 5 to be a MII interface
                rtl865xC_setAsicEthernetMIIMode(5, LINK_MII_MAC); //port 5  GMII/MII MAC auto mode
		REG32(P5GMIICR) = REG32(P5GMIICR) | 0x40; //Conf_done=1
                REG32(PCRP5) = 0 | (0x5<<ExtPHYID_OFFSET) |      //JSW@20100309:For external 8211BN GMII ,PHYID must be 5
                               	EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex |
                                MIIcfg_RXER | EnablePHYIf | MacSwReset;
#elif defined(CONFIG_8198_PORT5_RGMII)
		REG32(PITCR) = REG32(PITCR) & 0xFFFFF3FF; //configure port 5 to be a MII interface
		rtl865xC_setAsicEthernetMIIMode(5, LINK_RGMII); //port 5  GMII/MII MAC auto mode

		REG32(P5GMIICR) = REG32(P5GMIICR) | Conf_done;
		REG32(PCRP5) = 0 | (0x5<<ExtPHYID_OFFSET) |      //JSW@20100309:For external 8211BN GMII ,PHYID must be 5
                               	EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex |
                                MIIcfg_RXER | EnablePHYIf | MacSwReset;
#elif defined(CONFIG_8198C_8211FS) 
		#define GPIO_RESET   24   // GPIO_ H0 
		// Reset 8211FS
		REG32(PIN_MUX_SEL4) = (REG32(PIN_MUX_SEL4) & ~( 0xf <<3)) | (0x3<<3);
		REG32(PEFGH_CNR) &= ~(1<<GPIO_RESET);
		REG32(PEFGH_DIR) |= (1<<GPIO_RESET);
		REG32(PEFGH_DAT) &= ~(1<<GPIO_RESET);
		mdelay(500);
		REG32(PEFGH_DAT) |= (1<<GPIO_RESET);
		mdelay(300);

		REG32(PIN_MUX_SEL4) &= ~((1 << 21) | (1 << 22) | (1 << 23) | (1 << 24) | (1 << 25)) ; 
		REG32(PAD_CONTROL_2) |= ( 1 << 22 ); 
		REG32(PIN_MUX_SEL5) &= ~((1 << 14) | (1 << 15));

		// P5txdly = 1; P5rxdly = 5
		REG32_ANDOR(P5GMIICR, ~((1<<4)|(7<<0)) , (1<<4) | (5<<0) );			
		// force mode
		REG32(PCRP5) = (REG32(PCRP5) & ~AutoNegoSts_MASK) | (EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex);
		REG32(PCRP5) = (REG32(PCRP5) & ~(0x1f << ExtPHYID_OFFSET)) 
				| ((0x5 << ExtPHYID_OFFSET) | MIIcfg_RXER |  EnablePHYIf | MacSwReset);

		REG32(P5GMIICR) |= (Conf_done);

		//RGMII FIFO reset
		{
		uint32 regData;
		rtl8651_setAsicEthernetPHYReg(0x5, 31, 0xD08);
		rtl8651_getAsicEthernetPHYReg(0x5, 16, &regData);
		regData &= ~(1<<6);
		rtl8651_setAsicEthernetPHYReg(0x5, 16, regData);
		regData |= (1<<6);
		rtl8651_setAsicEthernetPHYReg(0x5, 16, regData);
		rtl8651_setAsicEthernetPHYReg(0x5, 31, 0); //change back to page 0
		}

#elif defined(CONFIG_RTL_8198C_8211F)
		init_p5();
#endif
	WRITE_MEM32(PCRP0,(REG32(PCRP0) & (0xFFFFFFFF-(MacSwReset))));
	WRITE_MEM32(PCRP0,(REG32(PCRP0) | (0 << ExtPHYID_OFFSET) | EnablePHYIf |MacSwReset));

#ifdef CONFIG_RTL_8197B
	/* disable MAC Gbe ability/Gbe 1000F ability of PHY. */
       WRITE_MEM32(PCRP0, READ_MEM32(PCRP0) & ~NwayAbility1000MF);
       WRITE_MEM32(PCRP1, READ_MEM32(PCRP1) & ~NwayAbility1000MF);
       WRITE_MEM32(PCRP2, READ_MEM32(PCRP2) & ~NwayAbility1000MF);
       WRITE_MEM32(PCRP3, READ_MEM32(PCRP3) & ~NwayAbility1000MF);
       WRITE_MEM32(PCRP4, READ_MEM32(PCRP4) & ~NwayAbility1000MF);
	Set_GPHYWB(999, 0, 9, 0, 0);
#endif

#ifdef CONFIG_RTL_8198C_8367RB
	// 8198C port 5 setting for 8367RB
	// P5txdly = 1; P5rxdly = 5
	REG32(P5GMIICR) = (REG32(P5GMIICR) & ~((1<<4)|(7<<0))) | ((1<<4) | (5<<0));
	REG32(PCRP5) = (REG32(PCRP5) & ~AutoNegoSts_MASK) | (EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex);
	REG32(PCRP5) = (REG32(PCRP5) & ~(0x1f << ExtPHYID_OFFSET)) 
			| ((0x15 << ExtPHYID_OFFSET) | MIIcfg_RXER |  EnablePHYIf | MacSwReset);
	REG32(P5GMIICR) |= (Conf_done);		
#endif
#if defined(CONFIG_RTL_8211F_SUPPORT)
	{
#ifdef CONFIG_RTL_8197F
	int ret;
	extern unsigned int rtl819x_bond_option(void);
	ret = rtl819x_bond_option();
	if (ret == 2) //97FN:2
		gpio_simulate_mdc_mdio = 1;
#endif
	init_p0();
	}
#elif defined(CONFIG_RTL_8197F) && (defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT))
	init_8197f_p0();

#elif defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	{
#if defined(CONFIG_RTL_8211DS_SUPPORT)&&defined(CONFIG_RTL_8197D)
	int i;
	uint32 reg_tmp=0;
#endif

	P0phymode=1;

#if (defined(CONFIG_RTL_8211DS_SUPPORT)&&defined(CONFIG_RTL_8197D)) || defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8370_SUPPORT) || defined(CONFIG_RTL_8325D_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)
	P0phymode = 3;

#elif defined(CONFIG_RTL_8196E)
	if ( ((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196ES)  ||
		((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196ES1)  ||
		((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196ES2)  ||
		((REG32(BOND_OPTION) & BOND_ID_MASK) == BOND_8196ES3)  )
	{
		P0phymode=Get_P0_PhyMode();
	}
#endif

	if(P0phymode==1)  //embedded phy
	{
		REG32(PCRP0) |=  (0 << ExtPHYID_OFFSET) | EnablePHYIf | MacSwReset;	//emabedded
	}
	else //external phy
	{
		P0miimode=Get_P0_MiiMode();

	#if (defined(CONFIG_RTL_8211DS_SUPPORT)&&defined(CONFIG_RTL_8197D)) || defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8370_SUPPORT) || defined(CONFIG_RTL_8325D_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)
		P0miimode = 3;
	#endif

		REG32(PCRP0) = (REG32(PCRP0) & ~(ExtPHYID_MASK)) | (P0_EXT_PHY_ID << ExtPHYID_OFFSET)
						| MIIcfg_RXER |  EnablePHYIf | MacSwReset;	//external

		if(P0miimode==0)
			REG32_ANDOR(P0GMIICR, ~(3<<23)  , LINK_MII_PHY<<23);
		else if(P0miimode==1)
			REG32_ANDOR(P0GMIICR, ~(3<<23)  , LINK_MII_MAC<<23);
		else if(P0miimode==2)
			REG32_ANDOR(P0GMIICR, ~(3<<23)  , LINK_MII_MAC<<23);  //GMII
		else if(P0miimode==3)
			REG32_ANDOR(P0GMIICR, ~(3<<23)  , LINK_RGMII<<23);

		if(P0miimode==3)
		{
			P0txdly=Get_P0_TxDelay();
			P0rxdly=Get_P0_RxDelay();

			#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8370_SUPPORT) || defined(CONFIG_RTL_8325D_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)
			P0txdly = 1;
			P0rxdly = 5;
			#endif

			REG32_ANDOR(P0GMIICR, ~((1<<4)|(3<<0)) , (P0txdly<<4) | (P0rxdly<<0) );

			#if defined(CONFIG_RTL_8211DS_SUPPORT)&&defined(CONFIG_RTL_8197D)
				//Set GPIOC0 to PHY reset.
				REG32_ANDOR(0xb8000044, 0xFFFFFFFF, (1<<20));
				REG32_ANDOR(0xb8003500, ~(1<<16), 0);
				REG32_ANDOR(0xb8003508, 0xFFFFFFFF, (1<<16));
				REG32_ANDOR(0xb800350c, ~(1<<16), 0); //Set Reset to low

				REG32(0xbb804104) &=0x3FFFFFF;
				REG32(0xbb804104) |=0x18FF0000;	//set PCR0 phyid
				REG32(0xbb80414c) =0x37d55;			//set port MII
				REG32(0xbb804100) =0x1;

				for(i=0; i<5; i++)
					REG32(PCRP0+i*4) |= (EnForceMode);

				REG32_ANDOR(0xb800350c, 0xFFFFFFFF, (1<<16));//Set Reset to high
				mdelay(30);
				//__delay(50000);

				rtl8651_getAsicEthernetPHYReg(P0_EXT_PHY_ID, 0, &reg_tmp);//Learning PHY ID

				for(i=0; i<5; i++)
					REG32(PCRP0+i*4) &= ~(EnForceMode);
			#endif
		}

#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)
#ifdef CONFIG_RTL_CPU_TAG
		REG32(P0GMIICR) |= (CFG_CPUC_TAG|CFG_TX_CPUC_TAG);

		// enable port 0 router mode
		REG32(MACCR1) |= PORT0_ROUTER_MODE;        	
#endif
#endif

		if(P0miimode==0)
			REG32_ANDOR(PCRP0, ~AutoNegoSts_MASK, EnForceMode| ForceLink|ForceSpeed100M |ForceDuplex) ;
		else if(P0miimode==1)
			REG32_ANDOR(PCRP0, ~AutoNegoSts_MASK, EnForceMode| ForceLink|ForceSpeed100M |ForceDuplex) ;
		else if(P0miimode==2)
			REG32_ANDOR(PCRP0, ~AutoNegoSts_MASK, EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex );
		#if !defined(CONFIG_RTL_8211DS_SUPPORT)
		else if(P0miimode==3)
			REG32_ANDOR(PCRP0, ~AutoNegoSts_MASK, EnForceMode| ForceLink|ForceSpeed1000M |ForceDuplex );
		#endif

		#if defined(CONFIG_RTL_8325D_SUPPORT)
		REG32_ANDOR(PCRP0, ~AutoNegoSts_MASK, EnForceMode| ForceLink|ForceSpeed100M |ForceDuplex) ;
		#endif

		REG32(PITCR) |= (1<<0);   //00: embedded , 01L GMII/MII/RGMII

		if((P0miimode==2)  ||(P0miimode==3)) {
			REG32(MACCR) |= (1<<12);   //giga link
		}
		REG32(P0GMIICR) |=(Conf_done);
	}
	}
#endif

#if defined(CONFIG_RTL_8881A)
	// MACCR b.13-b.12 System clock selection, 2'b00: 50MHz, 2'b01: 100MHz
	REG32(MACCR) |= (1<<12);
	// MAC Configuration Register 1, b.0, Cport MAC clock selection with NIC interface, 1: lx_clk, 0: lx_clk/2
	REG32(0xbb805100) |= 1;
#endif

//set Leaky bucket parameter
/* 	system clock = 50MHz --> token=0x40, tick=0x0b, l2=3*token=0xC0
    system clock = 100MHz --> token=0x36, tick=0x13, l2=3*token=0xA2*/
#if defined (CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	maccrData = READ_MEM32(MACCR);
	dataTemp= (maccrData & CF_SYSCLK_SEL_MASK)>>CF_SYSCLK_SEL_OFFSET;
	if(dataTemp==0x00)
	{
		token = 0x40;
		tick = 0x0b;
		hiThreshold = 0xC0;
		rtl8651_setAsicLBParameter( token, tick, hiThreshold );
	}
	else if(dataTemp==0x01)
	{
		token = 0x36;
		tick = 0x13;
		hiThreshold = 0xA2;
		rtl8651_setAsicLBParameter( token, tick, hiThreshold );
	}
	
#endif

#elif defined(CONFIG_RTL_8196C)
    WRITE_MEM32(PCRP0, (READ_MEM32(PCRP0)&(0xFFFFFFFF-(0x00400000|MacSwReset))) );
	TOGGLE_BIT_IN_REG_TWICE(PCRP0,EnForceMode);

    WRITE_MEM32(PCRP1, (READ_MEM32(PCRP1)&(0xFFFFFFFF-(0x00400000|MacSwReset))) );
	TOGGLE_BIT_IN_REG_TWICE(PCRP1,EnForceMode);
    WRITE_MEM32(PCRP2, (READ_MEM32(PCRP2)&(0xFFFFFFFF-(0x00400000|MacSwReset))) );
	TOGGLE_BIT_IN_REG_TWICE(PCRP2,EnForceMode);
    WRITE_MEM32(PCRP3, (READ_MEM32(PCRP3)&(0xFFFFFFFF-(0x00400000|MacSwReset))) );
	TOGGLE_BIT_IN_REG_TWICE(PCRP3,EnForceMode);
    WRITE_MEM32(PCRP4, (READ_MEM32(PCRP4)&(0xFFFFFFFF-(0x00400000|MacSwReset))) );
	TOGGLE_BIT_IN_REG_TWICE(PCRP4,EnForceMode);

    WRITE_MEM32(PCRP0, (READ_MEM32(PCRP0)|(rtl8651AsicEthernetTable[0].phyId<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset ) ); /* Jumbo Frame */
	TOGGLE_BIT_IN_REG_TWICE(PCRP0,EnForceMode);
    WRITE_MEM32(PCRP1, (READ_MEM32(PCRP1)|(rtl8651AsicEthernetTable[1].phyId<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset ) ); /* Jumbo Frame */
	TOGGLE_BIT_IN_REG_TWICE(PCRP1,EnForceMode);
    WRITE_MEM32(PCRP2, (READ_MEM32(PCRP2)|(rtl8651AsicEthernetTable[2].phyId<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset ) ); /* Jumbo Frame */
	TOGGLE_BIT_IN_REG_TWICE(PCRP2,EnForceMode);
    WRITE_MEM32(PCRP3, (READ_MEM32(PCRP3)|(rtl8651AsicEthernetTable[3].phyId<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset ) ); /* Jumbo Frame */
	TOGGLE_BIT_IN_REG_TWICE(PCRP3,EnForceMode);
    WRITE_MEM32(PCRP4, (READ_MEM32(PCRP4)|(rtl8651AsicEthernetTable[4].phyId<<ExtPHYID_OFFSET)|EnablePHYIf|MacSwReset ) ); /* Jumbo Frame */
	TOGGLE_BIT_IN_REG_TWICE(PCRP4,EnForceMode);
#else
    WRITE_MEM32(PCRP0, (READ_MEM32(PCRP0)|(rtl8651AsicEthernetTable[0].phyId<<ExtPHYID_OFFSET)|EnablePHYIf ) ); /* Jumbo Frame */
    WRITE_MEM32(PCRP1, (READ_MEM32(PCRP1)|(rtl8651AsicEthernetTable[1].phyId<<ExtPHYID_OFFSET)|EnablePHYIf ) ); /* Jumbo Frame */
    WRITE_MEM32(PCRP2, (READ_MEM32(PCRP2)|(rtl8651AsicEthernetTable[2].phyId<<ExtPHYID_OFFSET)|EnablePHYIf ) ); /* Jumbo Frame */
    WRITE_MEM32(PCRP3, (READ_MEM32(PCRP3)|(rtl8651AsicEthernetTable[3].phyId<<ExtPHYID_OFFSET)|EnablePHYIf ) ); /* Jumbo Frame */
    WRITE_MEM32(PCRP4, (READ_MEM32(PCRP4)|(rtl8651AsicEthernetTable[4].phyId<<ExtPHYID_OFFSET)|EnablePHYIf ) ); /* Jumbo Frame */
#endif


    if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
    {
        WRITE_MEM32(PCRP5, ( (READ_MEM32(PCRP5))|(rtl8651AsicEthernetTable[5].phyId<<ExtPHYID_OFFSET)|EnablePHYIf ) ); /* Jumbo Frame */
    }
#endif

	/*disable pattern match*/
	{
		int pnum;
		 for(pnum=0;pnum<RTL8651_PORT_NUMBER;pnum++)
	        {
	                rtl8651_setAsicPortPatternMatch(pnum, 0, 0, 0x2);
	        }
	}

#if defined(CONFIG_RTL_8196C)
	//enable RTL8196C 10M power saving mode
	{
		int tmp,idx;

		for(idx=0;idx<MAX_PORT_NUMBER;idx++) {
			rtl8651_getAsicEthernetPHYReg( idx, 0x18, &tmp );
#ifdef CONFIG_RTL8196C_GREEN_ETHERNET
			tmp |= BIT(15); //enable power saving mode
#else
			tmp &= ~BIT(15); //disable power saving mode
#endif
			rtl8651_setAsicEthernetPHYReg( idx, 0x18, tmp );
		}
	}
	REG32(MPMR) |= PM_MODE_ENABLE_AUTOMATIC_POWER_DOWN;

	// Configure LED-SIG0/LED-SIG1/LED-SIG2/LED-SIG3/LED-PHASE0/LED-PHASE1/LED-PHASE2/LED-PHASE3 PAD as LED-SW

#ifndef CONFIG_POCKET_ROUTER_SUPPORT
	#ifndef CONFIG_MTD_NAND
	REG32(PIN_MUX_SEL) &= ~(0xFFFF);
	#endif
#endif

#if defined(PATCH_GPIO_FOR_LED)
	REG32(PIN_MUX_SEL) |= (0xFFFF);
#endif

#endif  // end of defined(CONFIG_RTL_8196C)

#if defined(CONFIG_RTL_8198) && !defined(CONFIG_RTL_819XD)
        REG32(MPMR) |= PM_MODE_ENABLE_AUTOMATIC_POWER_DOWN;
#endif

#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F)
	disable_unused_phy_power();
#endif

#if defined(CONFIG_RTL_8198C)
	// for the board which has 512Mbytes RAM
	REG32(DMA_CR0) |= (1 << HsbAddrMark_OFFSET);

	#if defined(PATCH_GPIO_FOR_LED)
	REG32(PIN_MUX_SEL3) = (REG32(PIN_MUX_SEL3) & ~0x7FFF) | (0x36DB);
	#endif

	/*fix jwj: let ipv6 with exten header to cpu*/
	REG32(IPV6CR1) = (REG32(IPV6CR1) & ~0x3) | (0x3);

#elif defined(CONFIG_RTL_8881A)
	#if !defined(CONFIG_RTL_8367R_SUPPORT) && !defined(CONFIG_RTL_8211F_SUPPORT) && !defined(CONFIG_RTL_83XX_SUPPORT)
	REG32(PIN_MUX_SEL) |= 0x180;
	#endif

	#if defined(PATCH_GPIO_FOR_LED)
	REG32(PIN_MUX_SEL2) = (REG32(PIN_MUX_SEL2) & ~(0x3FFF)) | (0x36DB);
	#endif

#elif defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	#if defined(PATCH_GPIO_FOR_LED)
	REG32(PIN_MUX_SEL2) |= (0x3FFF);
	#endif

#elif defined(CONFIG_RTL_8197F)
	/*fix jwj: let ipv6 with exten header to cpu*/
	REG32(IPV6CR1) = (REG32(IPV6CR1) & ~0x3) | (0x3);

	REG32(SYS_CLK_MAG) |= (CM_ACTIVE_LX2_CLK | CM_ACTIVE_LX2_ARB);
	REG32(CPUICR1) |= ((1 << CF_NIC_LITTLE_ENDIAN_OFFSET) | (1 << CF_TXRX_DIV_LX_OFFSET) | (1 << CF_TSO_ID_SEL));
	REG32(MACCTRL1) |= CF_CMAC_CLK_SEL;

	#if defined(PATCH_GPIO_FOR_LED) && !defined(CONFIG_RTL_8367R_SUPPORT) && !defined(CONFIG_RTL_83XX_SUPPORT)
	REG32(PIN_MUX_SEL13) = 0x33320000;
	REG32(PIN_MUX_SEL14) = 0x20000000;
	#endif

	#if !defined(CONFIG_RTL_8367R_SUPPORT) && !defined(CONFIG_RTL_8211F_SUPPORT) && !defined(CONFIG_RTL_83XX_SUPPORT)
	{
	extern uint32 rtl819x_bond_option(void);
	if (rtl819x_bond_option() == 3)
		REG32(EPHY_CONTROL) &= ~EN_ROUTER_MODE;	
	}		
	#endif	

	#if defined(CONFIG_RTL_8211F_SUPPORT)
	/* fix the issue: giga port <-> FE port (thru HW L2 forward) throughput unbalance issue */
	REG32(PQPLGR) = (REG32(PQPLGR) & ~0xff) | 0x34;
	REG32(QRR) = 0;
	#endif		
#endif

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	// WARNING: TX_RING2_EN bit must be 1 if TxRing2 is used, TX_RING3_EN bit must be 1 if TxRing3 is used.
	#if defined(RTL_MULTIPLE_RX_TX_RING)
	REG32(TXRINGCR) &= ~(TX_RING3_EN|TX_RING2_EN);
	#else
	//REG32(TXRINGCR) &= ~(TX_RING3_EN|TX_RING2_EN|TX_RING1_EN);
	REG32(TXRINGCR) &= ~(TX_RING3_EN|TX_RING2_EN); // enable ring1 and ring0 to let the hardware state machine healthy (by Alan Ho)
	#endif
#endif
#if defined(CONFIG_RTL_8197F)
	//default enable acl pattern match, and latch begin from 1st byte in pkt (from dmac).
	REG32(MACCTRL1) |=  (CF_CMAC_LATPKT_EN);
	REG32(MACCTRL1) &=  ~(CF_CMAC_LATPKT_TYPE);
	//REG32(MACCTRL1) |=  (CF_CMAC_LATPKT_TYPE);
#endif

#if defined(CONFIG_RTL_FE_AUTO_DOWN_SPEED)
	ads_init();
#endif

#if defined(CONFIG_RTL_FORCE_MDIX)
	fmx_init();
#endif

	return SUCCESS;
}

#ifdef CONFIG_RTL_JUMBO_FRAME
int32 rtl865x_set_jumbo_frame_size(uint32 size, int op_mode)
{
	
	/*clear jumbo frame size*/
	WRITE_MEM32(PCRP0, (READ_MEM32(PCRP0)&(~AcptMaxLen_16K)) | MacSwReset ); /* Jumbo Frame */
	WRITE_MEM32(PCRP1, (READ_MEM32(PCRP1)&(~AcptMaxLen_16K)) | MacSwReset ); /* Jumbo Frame */
	WRITE_MEM32(PCRP2, (READ_MEM32(PCRP2)&(~AcptMaxLen_16K)) | MacSwReset ); /* Jumbo Frame */
	WRITE_MEM32(PCRP3, (READ_MEM32(PCRP3)&(~AcptMaxLen_16K)) | MacSwReset ); /* Jumbo Frame */
	if (op_mode != 0) //not GATEWAY_MODE
		WRITE_MEM32(PCRP4, (READ_MEM32(PCRP4)&(~AcptMaxLen_16K)) | MacSwReset ); /* Jumbo Frame */
	WRITE_MEM32(PCRP6, (READ_MEM32(PCRP6)&(~AcptMaxLen_16K)) | MacSwReset ); /* Jumbo Frame */	
	switch(size){
		case 1552:
			WRITE_MEM32(PCRP0, READ_MEM32(PCRP0) | (AcptMaxLen_1552|MacSwReset) ); /* Jumbo Frame */
			WRITE_MEM32(PCRP1, READ_MEM32(PCRP1) | (AcptMaxLen_1552|MacSwReset) ); /* Jumbo Frame */
			WRITE_MEM32(PCRP2, READ_MEM32(PCRP2) | (AcptMaxLen_1552|MacSwReset) ); /* Jumbo Frame */
			WRITE_MEM32(PCRP3, READ_MEM32(PCRP3) | (AcptMaxLen_1552|MacSwReset) ); /* Jumbo Frame */
			if (op_mode != 0) //not GATEWAY_MODE
				WRITE_MEM32(PCRP4, READ_MEM32(PCRP4) | (AcptMaxLen_1552|MacSwReset) ); /* Jumbo Frame */
			WRITE_MEM32(PCRP6, READ_MEM32(PCRP6) | (AcptMaxLen_1552|MacSwReset) ); /* Jumbo Frame */	
			break;
		case 9000:
			WRITE_MEM32(PCRP0, READ_MEM32(PCRP0) | (AcptMaxLen_9K|MacSwReset) ); /* Jumbo Frame */
			WRITE_MEM32(PCRP1, READ_MEM32(PCRP1) | (AcptMaxLen_9K|MacSwReset) ); /* Jumbo Frame */
			WRITE_MEM32(PCRP2, READ_MEM32(PCRP2) | (AcptMaxLen_9K|MacSwReset) ); /* Jumbo Frame */
			WRITE_MEM32(PCRP3, READ_MEM32(PCRP3) | (AcptMaxLen_9K|MacSwReset) ); /* Jumbo Frame */
			if (op_mode != 0) //not GATEWAY_MODE
				WRITE_MEM32(PCRP4, READ_MEM32(PCRP4) | (AcptMaxLen_9K|MacSwReset) ); /* Jumbo Frame */
			WRITE_MEM32(PCRP6, READ_MEM32(PCRP6) | (AcptMaxLen_9K|MacSwReset) ); /* Jumbo Frame */	
			break;
		case 16000:
			WRITE_MEM32(PCRP0, READ_MEM32(PCRP0) | (AcptMaxLen_16K|MacSwReset) ); /* Jumbo Frame */
			WRITE_MEM32(PCRP1, READ_MEM32(PCRP1) | (AcptMaxLen_16K|MacSwReset) ); /* Jumbo Frame */
			WRITE_MEM32(PCRP2, READ_MEM32(PCRP2) | (AcptMaxLen_16K|MacSwReset) ); /* Jumbo Frame */
			WRITE_MEM32(PCRP3, READ_MEM32(PCRP3) | (AcptMaxLen_16K|MacSwReset) ); /* Jumbo Frame */
			if (op_mode != 0) //not GATEWAY_MODE
				WRITE_MEM32(PCRP4, READ_MEM32(PCRP4) | (AcptMaxLen_16K|MacSwReset) ); /* Jumbo Frame */
			WRITE_MEM32(PCRP6, READ_MEM32(PCRP6) | (AcptMaxLen_16K|MacSwReset) ); /* Jumbo Frame */	
			break;
		default:
			return 1;
			break;
	}
	return 0;
	
}
#endif


int32 rtl8651_setAsicPortPatternMatch(uint32 port, uint32 pattern, uint32 patternMask, int32 operation) {
	//not for ext port
	if(port>=RTL8651_PORT_NUMBER)
		return FAILED;

	if(pattern==0&&patternMask==0){
		//bit 0~13 is reseved...
		//if((READ_MEM32(PPMAR)&0x2000)==0) //system pattern match not enabled.
			//return SUCCESS;
		WRITE_MEM32(PPMAR,READ_MEM32(PPMAR)&~(1<<(port+26)));
		if((READ_MEM32(PPMAR)&0xfc000000)==0)
			WRITE_MEM32(PPMAR,READ_MEM32(PPMAR)&~(1<<13)); //turn off system pattern match switch.

		return SUCCESS;
	}
	if(operation>3)
		return FAILED; //valid operations: 0(drop), 1(mirror to cpu),2(fwd to cpu), 3(to mirror port)
	WRITE_MEM32(PPMAR,READ_MEM32(PPMAR)|((1<<(port+26))|(1<<13))); //turn on system pattern match and turn on pattern match on indicated port.
	WRITE_MEM32(PPMAR,(READ_MEM32(PPMAR) & (~(0x3<<(14+2*port))))|(operation<<(14+2*port)));   //specify operation
	WRITE_MEM32(PATP0+4*port,pattern);
	WRITE_MEM32(MASKP0+4*port,patternMask);
	return SUCCESS;
}

int32 rtl8651_getAsicPortPatternMatch(uint32 port, uint32 *pattern, uint32 *patternMask, int32 *operation)
{
	//not for ext port
	if(port>=RTL8651_PORT_NUMBER)
		return FAILED;
	if(((READ_MEM32(PPMAR)& (1<<13))==0)||((READ_MEM32(PPMAR)& (1<<(26+port)))==0))
		return FAILED;
	if(pattern)
		*pattern=READ_MEM32(PATP0+4*port);
	if(patternMask)
		*patternMask=READ_MEM32(MASKP0+4*port);
	if(operation)
		*operation=(READ_MEM32(PPMAR)>>(14+2*port))&0x3;
	return SUCCESS;
}

/*
@func int32		| rtl8651_setAsicSpanningEnable 	| Enable/disable ASIC spanning tree support
@parm int8		| spanningTreeEnabled | TRUE to indicate spanning tree is enabled; FALSE to indicate spanning tree is disabled.
@rvalue SUCCESS	| 	Success
@comm
Global switch to enable or disable ASIC spanning tree support.
If ASIC spanning tree support is enabled, further configuration would be refered by ASIC to prcoess packet forwarding / MAC learning.
If ASIC spanning tree support is disabled, all MAC learning and packet forwarding would be done regardless of port state.
Note that the configuration does not take effect for spanning tree BPDU CPU trapping. It is set in <p rtl8651_setAsicResvMcastAddrToCPU()>.
@xref <p rtl865xC_setAsicSpanningTreePortState()>, <p rtl8651_getAsicMulticastSpanningTreePortState()>, <p rtl865xC_getAsicSpanningTreePortState()>
 */
int32 rtl8651_setAsicSpanningEnable(int8 spanningTreeEnabled)
{
	if(spanningTreeEnabled == TRUE)
	{
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)|(EN_STP));
		WRITE_MEM32(RMACR ,READ_MEM32(RMACR)|MADDR00);

	}else
	{
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)&~(EN_STP));
		WRITE_MEM32(RMACR, READ_MEM32(RMACR)&~MADDR00);

	}
	return SUCCESS;
}

/*
@func int32		| rtl8651_getAsicSpanningEnable 	| Getting the ASIC spanning tree support status
@parm int8*		| spanningTreeEnabled | The pointer to get the status of ASIC spanning tree configuration status.
@rvalue FAILED	| 	Failed
@rvalue SUCCESS	| 	Success
@comm
Get the ASIC global switch to enable or disable ASIC spanning tree support.
The switch can be set by calling <p rtl8651_setAsicSpanningEnable()>
@xref <p rtl8651_setAsicSpanningEnable()>, <p rtl865xC_setAsicSpanningTreePortState()>, <p rtl8651_getAsicMulticastSpanningTreePortState()>, <p rtl865xC_getAsicSpanningTreePortState()>
 */
int32 rtl8651_getAsicSpanningEnable(int8 *spanningTreeEnabled)
{
	if(spanningTreeEnabled == NULL)
		return FAILED;
	*spanningTreeEnabled = (READ_MEM32(MSCR)&(EN_STP)) == (EN_STP)? TRUE: FALSE;
	return SUCCESS;
}

/*
@func int32		| rtl865xC_setAsicSpanningTreePortState 	| Configure Spanning Tree Protocol Port State
@parm uint32 | port | port number under consideration
@parm uint32 | portState | Spanning tree port state: RTL8651_PORTSTA_DISABLED, RTL8651_PORTSTA_BLOCKING, RTL8651_PORTSTA_LISTENING, RTL8651_PORTSTA_LEARNING, RTL8651_PORTSTA_FORWARDING
@rvalue SUCCESS	| 	Success
@rvalue FAILED | Failed
@comm
Config IEEE 802.1D spanning tree port sate into ASIC.
 */
int32 rtl865xC_setAsicSpanningTreePortState(uint32 port, uint32 portState)
{
	uint32 offset = port * 4;

	if ( port >= RTL865XC_PORT_NUMBER )
		return FAILED;

	switch(portState)
	{
		case RTL8651_PORTSTA_DISABLED:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~STP_PortST_MASK) ) | STP_PortST_DISABLE );
			break;
		case RTL8651_PORTSTA_BLOCKING:
		case RTL8651_PORTSTA_LISTENING:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~STP_PortST_MASK) ) | STP_PortST_BLOCKING );
			break;
		case RTL8651_PORTSTA_LEARNING:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~STP_PortST_MASK) ) | STP_PortST_LEARNING );
			break;
		case RTL8651_PORTSTA_FORWARDING:
			WRITE_MEM32( PCRP0 + offset, ( READ_MEM32( PCRP0 + offset ) & (~STP_PortST_MASK) ) | STP_PortST_FORWARDING );
			break;
		default:
			return FAILED;
	}

	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset, EnForceMode);
	return SUCCESS;
}

/*
@func int32		| rtl865xC_getAsicSpanningTreePortState 	| Retrieve Spanning Tree Protocol Port State
@parm uint32 | port | port number under consideration
@parm uint32 | portState | pointer to memory to store the port state
@rvalue SUCCESS	| 	Success
@rvalue FAILED | Failed
@comm
Possible spanning tree port state: RTL8651_PORTSTA_DISABLED, RTL8651_PORTSTA_BLOCKING, RTL8651_PORTSTA_LISTENING, RTL8651_PORTSTA_LEARNING, RTL8651_PORTSTA_FORWARDING
 */
int32 rtl865xC_getAsicSpanningTreePortState(uint32 port, uint32 *portState)
{
	uint32 reg;
	uint32 offset = port * 4;

	if ( port >= RTL865XC_PORT_NUMBER || portState == NULL )
		return FAILED;

	reg = ( READ_MEM32( PCRP0 + offset ) & STP_PortST_MASK );

	switch(reg)
	{
		case STP_PortST_DISABLE:
			*portState = RTL8651_PORTSTA_DISABLED;
			break;
		case STP_PortST_BLOCKING:
			*portState = RTL8651_PORTSTA_BLOCKING;
			break;
		case STP_PortST_LEARNING:
			*portState = RTL8651_PORTSTA_LEARNING;
			break;
		case STP_PortST_FORWARDING:
			*portState = RTL8651_PORTSTA_FORWARDING;
			break;
		default:
			return FAILED;
	}
	return SUCCESS;

}

/*
@func int32		| rtl8651_getAsicMulticastSpanningTreePortState 	| Retrieve Spanning Tree Protocol Port State
@parm uint32 | port | port number under consideration
@parm uint32 | portState | pointer to memory to store the port state
@rvalue SUCCESS	| 	Success
@rvalue FAILED | Failed
@comm
In RTL865xC platform, Multicast spanning tree configuration is gotten by this API.
@xref  <p rtl865xC_getAsicSpanningTreePortState()>
 */
int32 rtl8651_getAsicMulticastSpanningTreePortState(uint32 port, uint32 *portState)
{
	uint32 reg;
	uint32 offset = port * 4;

	if ( port >= RTL865XC_PORT_NUMBER || portState == NULL )
		return FAILED;

	reg = ( READ_MEM32( PCRP0 + offset ) & IPMSTP_PortST_MASK );

	switch(reg)
	{
		case IPMSTP_PortST_DISABLE:
			*portState = RTL8651_PORTSTA_DISABLED;
			break;
		case IPMSTP_PortST_BLOCKING:
			*portState = RTL8651_PORTSTA_BLOCKING;
			break;
		case IPMSTP_PortST_LEARNING:
			*portState = RTL8651_PORTSTA_LEARNING;
			break;
		case IPMSTP_PortST_FORWARDING:
			*portState = RTL8651_PORTSTA_FORWARDING;
			break;
		default:
			return FAILED;
	}
	return SUCCESS;
}

/*=========================================
  * ASIC DRIVER API: MDC/MDIO Control
  *=========================================*/

int32 rtl8651_getAsicEthernetPHYReg(uint32 phyId, uint32 regId, uint32 *rData)
{
	uint32 status;

	WRITE_MEM32( MDCIOCR, COMMAND_READ | ( phyId << PHYADD_OFFSET ) | ( regId << REGADD_OFFSET ) );

#if defined(CONFIG_RTL_8198)
	if (REG32(REVR) == BSP_RTL8198_REVISION_A)
		mdelay(10);
#elif defined(CONFIG_RTL8196C_REVISION_B)
	if (REG32(REVR) == RTL8196C_REVISION_A)
		mdelay(10);	//wei add, for 8196C revision A. mdio data read will delay 1 mdc clock.
#endif

	do { status = READ_MEM32( MDCIOSR ); } while ( ( status & MDC_STATUS ) != 0 );

	status &= 0xffff;
	*rData = status;

	return SUCCESS;
}

int32 rtl8651_setAsicEthernetPHYReg(uint32 phyId, uint32 regId, uint32 wData)
{
	WRITE_MEM32( MDCIOCR, COMMAND_WRITE | ( phyId << PHYADD_OFFSET ) | ( regId << REGADD_OFFSET ) | wData );

	while( ( READ_MEM32( MDCIOSR ) & MDC_STATUS ) != 0 );		/* wait until command complete */

	return SUCCESS;
}

int32 rtl8651_getAsicEthernetPHYStatus(uint32 port, uint32 *rData)
{
	uint32 statCtrlReg1, phyid;

	/* port number validation */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

	/* PHY id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* read current PHY reg 1*/
	rtl8651_getAsicEthernetPHYReg( phyid, 1, &statCtrlReg1 );

	/*assign value*/
	*rData=statCtrlReg1;
	return SUCCESS;
}

int32 rtl8651_restartAsicEthernetPHYNway(uint32 port)
{
	uint32 statCtrlReg0, phyid;

	/* port number validation */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

	/* PHY id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

#ifdef CONFIG_RTL_8211F_SUPPORT
	if (rtl8651AsicEthernetTable[port].isGPHY == TRUE) {
        	if(gpio_simulate_mdc_mdio){
                	rtl_mdio_read(phyid, 0, &statCtrlReg0 );
	                statCtrlReg0 |= RESTART_AUTONEGO;
        	        rtl_mdio_write(phyid, 0, statCtrlReg0);
		        return SUCCESS;
	        }
	}
#endif

	/* read current PHY reg 0 */
	rtl8651_getAsicEthernetPHYReg( phyid, 0, &statCtrlReg0 );

	/* enable 'restart Nway' bit */
	statCtrlReg0 |= RESTART_AUTONEGO;

	/* write PHY reg 0 */
	rtl8651_setAsicEthernetPHYReg( phyid, 0, statCtrlReg0 );

	return SUCCESS;
}

int32 rtl8651_setAsicEthernetPHYPowerDown( uint32 port, uint32 pwrDown )
{
	uint32 statCtrlReg0, phyid;

	/* port number validation */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

	/* PHY id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* read current PHY reg 0 value */
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if (gpio_simulate_mdc_mdio && (phyid==PORT0_RGMII_PHYID))
		rtl_mdio_read(phyid, 0, &statCtrlReg0);
	else
#endif
		rtl8651_getAsicEthernetPHYReg( phyid, 0, &statCtrlReg0 );

	if ( pwrDown ){
		REG32(PCRP0+port*4) |= EnForceMode;
		REG32(PCRP0+port*4) |= PollLinkStatus;
        REG32(PCRP0+port*4) &= ~ForceLink;
		statCtrlReg0 |= POWER_DOWN;
	}
	else{
		//REG32(PCRP0+port*4) &= ~EnForceMode;
		statCtrlReg0 &= ~POWER_DOWN;
	}
	/* write PHY reg 0 */
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if (gpio_simulate_mdc_mdio && (phyid==PORT0_RGMII_PHYID)){
		rtl_mdio_write(phyid, 0, statCtrlReg0);
		return SUCCESS;
	}
	else
#endif
	{
		rtl8651_setAsicEthernetPHYReg( phyid, 0, statCtrlReg0 );
	}
	if ( !pwrDown ){
		REG32(PCRP0+port*4) &= ~EnForceMode;
		REG32(PCRP0+port*4) &= ~ PollLinkStatus;
	}
	return SUCCESS;

}

int32 rtl8651_setAsicEthernetPHYAdvCapality(uint32 port, uint32 capality)
{
	uint32 statCtrlReg4, statCtrlReg9, phyid;

	/* port number validation */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

	/* PHY id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* read current PHY reg 4 value */
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if (gpio_simulate_mdc_mdio && (phyid==PORT0_RGMII_PHYID)){
		rtl_mdio_read(phyid, 4, &statCtrlReg4);
		rtl_mdio_read(phyid, 9, &statCtrlReg9);
	} else
#endif	
	{
		rtl8651_getAsicEthernetPHYReg( phyid, 4, &statCtrlReg4);
		rtl8651_getAsicEthernetPHYReg( phyid, 9, &statCtrlReg9);
	}
	/*Clear Duplex and Speed bits*/
	statCtrlReg4 &= ~(0xF<<5);
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if (phyid==PORT0_RGMII_PHYID)
		statCtrlReg9 &= ~(0x1<<9); // Adv 1000M
#endif

	if (capality & (1<<DUPLEX_100M))
	{
		statCtrlReg4 |= (1<<8);
	}
	if (capality & (1<<HALF_DUPLEX_100M))
	{
		statCtrlReg4 |= (1<<7);
	}
	if (capality & (1<<DUPLEX_10M))
	{
		statCtrlReg4 |= (1<<6);
	}
	if (capality & (1<<HALF_DUPLEX_10M))
	{
		statCtrlReg4 |= (1<<5);
	}
	if(capality & (1<<PORT_AUTO))
	{
		/*Set All Duplex and Speed All Supported*/
		statCtrlReg4 |=(0xF <<5);
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if (phyid==PORT0_RGMII_PHYID)
		statCtrlReg9 |=(0x1 <<9); // Adv 1000M
#endif
	}

	/* write PHY reg 4 */
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if (gpio_simulate_mdc_mdio && (phyid==PORT0_RGMII_PHYID)){
		rtl_mdio_write(phyid, 4, statCtrlReg4);
		rtl_mdio_write(phyid, 9, statCtrlReg9);
	} else
#endif
	{
		rtl8651_setAsicEthernetPHYReg( phyid, 4, statCtrlReg4 );
		rtl8651_setAsicEthernetPHYReg( phyid, 9, statCtrlReg9 );
	}

	return SUCCESS;
}
int32 rtl8651_setAsicEthernetPHYSpeed( uint32 port, uint32 speed )
{
	uint32 statCtrlReg0, phyid;

	/* port number validation */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

	/* PHY id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* read current PHY reg 0 value */
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if (gpio_simulate_mdc_mdio && (phyid==PORT0_RGMII_PHYID))
		rtl_mdio_read(phyid, 0, &statCtrlReg0);
	else
#endif
		rtl8651_getAsicEthernetPHYReg( phyid, 0, &statCtrlReg0 );

	if (0 == speed )
	{
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if (phyid==PORT0_RGMII_PHYID)
		statCtrlReg0 &= ~(SPEED_SELECT_100M | SPEED_SELECT_1000M);
	else
		statCtrlReg0 &= ~SPEED_SELECT_100M;
#else
		/*10M*/
		statCtrlReg0 &= ~SPEED_SELECT_100M;
#endif
	}
	else if (1 == speed)
	{
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if (phyid==PORT0_RGMII_PHYID)
		statCtrlReg0 = (statCtrlReg0 & ~SPEED_SELECT_1000M) | SPEED_SELECT_100M;
	else
		statCtrlReg0 |= SPEED_SELECT_100M;
#else
		/*100M*/
		statCtrlReg0 |= SPEED_SELECT_100M;
#endif
	}
	else if(2 == speed)
	{
		/*1000M*/
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if (phyid==PORT0_RGMII_PHYID)
		statCtrlReg0 = (statCtrlReg0 & ~SPEED_SELECT_100M) | SPEED_SELECT_1000M;
#endif		
	}

	/* write PHY reg 0 */
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if (gpio_simulate_mdc_mdio && (phyid==PORT0_RGMII_PHYID))
		rtl_mdio_write(phyid, 0, statCtrlReg0);
	else
#endif
		rtl8651_setAsicEthernetPHYReg( phyid, 0, statCtrlReg0 );

	return SUCCESS;

}

int32 rtl8651_setAsicEthernetPHYDuplex( uint32 port, uint32 duplex )
{
	uint32 statCtrlReg0, phyid;

	/* port number validation */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

	/* PHY id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* read current PHY reg 0 value */
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if (gpio_simulate_mdc_mdio && (phyid==PORT0_RGMII_PHYID))
		rtl_mdio_read(phyid, 0, &statCtrlReg0);
	else
#endif
		rtl8651_getAsicEthernetPHYReg( phyid, 0, &statCtrlReg0 );

	if ( duplex )
		statCtrlReg0 |= SELECT_FULL_DUPLEX;
	else
		statCtrlReg0 &= ~SELECT_FULL_DUPLEX;

	/* write PHY reg 0 */
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if (gpio_simulate_mdc_mdio && (phyid==PORT0_RGMII_PHYID))
		rtl_mdio_write(phyid, 0, statCtrlReg0);
	else
#endif	
		rtl8651_setAsicEthernetPHYReg( phyid, 0, statCtrlReg0 );

	return SUCCESS;

}

int32 rtl8651_setAsicEthernetPHYAutoNeg( uint32 port, uint32 autoneg)
{
	uint32 statCtrlReg0, phyid;

	/* port number validation */
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

	/* PHY id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* read current PHY reg 0 value */
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if (gpio_simulate_mdc_mdio && (phyid==PORT0_RGMII_PHYID))
		rtl_mdio_read(phyid, 0, &statCtrlReg0);
	else
#endif
		rtl8651_getAsicEthernetPHYReg( phyid, 0, &statCtrlReg0 );

	if (autoneg)
		statCtrlReg0 |= ENABLE_AUTONEGO;
	else
		statCtrlReg0 &= ~ENABLE_AUTONEGO;

	/* write PHY reg 0 */
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if (gpio_simulate_mdc_mdio && (phyid==PORT0_RGMII_PHYID))
		rtl_mdio_write(phyid, 0, statCtrlReg0);
	else
#endif	
		rtl8651_setAsicEthernetPHYReg( phyid, 0, statCtrlReg0 );

	return SUCCESS;

}

int32 rtl865xC_setAsicPortPauseFlowControl(uint32 port, uint8 rxEn, uint8 txEn)
{
	uint32 offset = port<<2;
	uint32 pauseFC = 0;

	if(rxEn!=0)
		pauseFC |= PauseFlowControlDtxErx;
	if(txEn!=0)
		pauseFC |= PauseFlowControlEtxDrx;

	WRITE_MEM32(PCRP0+offset, (~(PauseFlowControl_MASK)&(READ_MEM32(PCRP0+offset)))|pauseFC);

	TOGGLE_BIT_IN_REG_TWICE(PCRP0 + offset,EnForceMode);
	return SUCCESS;
}

int32 rtl865xC_getAsicPortPauseFlowControl(uint32 port, uint8 *rxEn, uint8 *txEn)
{
	uint32 offset = port<<2;
	uint32 pauseFC = 0;

	pauseFC = ((PauseFlowControl_MASK)&(READ_MEM32(PCRP0+offset)));

	if (pauseFC&PauseFlowControlDtxErx)
		*rxEn = TRUE;

	if (pauseFC&PauseFlowControlEtxDrx)
		*txEn = TRUE;

	return SUCCESS;
}

/*=========================================
  * ASIC DRIVER API: ETHERNET MII
  *=========================================*/
int32 rtl865xC_setAsicEthernetMIIMode(uint32 port, uint32 mode)
{
	if ( port != 0 && port != RTL8651_MII_PORTNUMBER )
		return FAILED;
	if ( mode != LINK_RGMII && mode != LINK_MII_MAC && mode != LINK_MII_PHY )
		return FAILED;

	if ( port == 0 )
	{
		/* MII port MAC interface mode configuration */
		WRITE_MEM32( P0GMIICR, ( READ_MEM32( P0GMIICR ) & ~CFG_GMAC_MASK ) | ( mode << LINKMODE_OFFSET ) );
	}
	else
	{
		/* MII port MAC interface mode configuration */
		WRITE_MEM32( P5GMIICR, ( READ_MEM32( P5GMIICR ) & ~CFG_GMAC_MASK ) | ( mode << LINKMODE_OFFSET ) );
	}
	return SUCCESS;

}

int32 rtl865xC_setAsicEthernetRGMIITiming(uint32 port, uint32 Tcomp, uint32 Rcomp)
{
	if ( port != 0 && port != RTL8651_MII_PORTNUMBER )
		return FAILED;
	if ( Tcomp < RGMII_TCOMP_0NS || Tcomp > RGMII_TCOMP_7NS || Rcomp < RGMII_RCOMP_0NS || Rcomp > RGMII_RCOMP_2DOT5NS )
		return FAILED;

	if ( port == 0 )
	{
		WRITE_MEM32(P0GMIICR, ( ( ( READ_MEM32(P0GMIICR) & ~RGMII_TCOMP_MASK ) | Tcomp ) & ~RGMII_RCOMP_MASK ) | Rcomp );
	}
	else
	{
		WRITE_MEM32(P5GMIICR, ( ( ( READ_MEM32(P5GMIICR) & ~RGMII_TCOMP_MASK ) | Tcomp ) & ~RGMII_RCOMP_MASK ) | Rcomp );
	}

	return SUCCESS;
}

/* For backward-compatible issue, this API is used to set MII port 5. */
int32 rtl8651_setAsicEthernetMII(uint32 phyAddress, int32 mode, int32 enabled)
{
	/* Input validation */
	if ( phyAddress < 0 || phyAddress > 31 )
		return FAILED;
	if ( mode != P5_LINK_RGMII && mode != P5_LINK_MII_MAC && mode != P5_LINK_MII_PHY )
		return FAILED;

	/* Configure driver level information about mii port 5 */
	if ( enabled )
	{
		if ( miiPhyAddress >= 0 && miiPhyAddress != phyAddress )
			return FAILED;

		miiPhyAddress = phyAddress;
	}
	else
	{
		miiPhyAddress = -1;
	}

	/* MII port MAC interface mode configuration */
	WRITE_MEM32( P5GMIICR, ( READ_MEM32( P5GMIICR ) & ~CFG_GMAC_MASK ) | ( mode << P5_LINK_OFFSET ) );

	return SUCCESS;
}

int32 rtl8651_getAsicEthernetMII(uint32 *phyAddress)
{
	*phyAddress=miiPhyAddress;
	return SUCCESS;
}


/*vlan remark*/
int32 rtl8651_setAsicVlanRemark(enum PORTID port, enum PRIORITYVALUE priority, int remark)
{
	int regValue;
	if ((port < PHY0) || (port > CPU) || (priority < PRI0) || (priority > PRI7) ||(remark < PRI0) || (remark > PRI7))
		return FAILED;

	WRITE_MEM32(RLRC, (READ_MEM32(RLRC) &~(0x7)) |0x7);

	regValue = READ_MEM32(RMCR1P) & ~((0x7<<(3*priority)) |(0x1 << (24 + port)));
	regValue |= ((remark << (3*priority)) |(0x1 << (24 + port)));
	WRITE_MEM32( RMCR1P, regValue);

	return SUCCESS;
}

int32 rtl8651_getAsicVlanRemark(enum PORTID port, enum PRIORITYVALUE priority, int* remark)
{
	int regValue;
	if ((port < PHY0) || (port > CPU) || (priority < PRI0) || (priority > PRI7) ||(remark == NULL))
		return FAILED;

	regValue = READ_MEM32(RMCR1P);
	if(regValue & (0x1<<(24+port))){
		*remark = (regValue>>(3*priority)) & 0x7;
	}else{
			return FAILED;
	}

	return SUCCESS;
}


/*vlan remark*/
int32 rtl8651_setAsicDscpRemark(enum PORTID port, enum PRIORITYVALUE priority, int remark)
{
	int regValue0;
	if ((port < PHY0) || (port > CPU) || (priority < PRI0) || (priority > PRI7) ||(remark < 0) ||(remark > 63))
		return FAILED;

	WRITE_MEM32(RLRC, (READ_MEM32(RLRC) &~(0x7<<3)) |(0x7<<3));

	if(priority < 5){
		regValue0 = READ_MEM32(DSCPRM0) &~(0x3f<<(6*priority));
		regValue0 |= remark << (6*priority);
		WRITE_MEM32( DSCPRM0, regValue0);

		regValue0 = READ_MEM32(DSCPRM1) &~(0x1 << (23 + port));
		regValue0 |= 0x1 << (23 + port);
		WRITE_MEM32( DSCPRM1, regValue0);
	}else{
		regValue0 = READ_MEM32(DSCPRM1) & ~((0x3f<<(6*(priority -5))) |(0x1 << (23 + port)));
		regValue0 |= ((remark<<(6*(priority -5))) |(0x1 << (23 + port)));
		WRITE_MEM32( DSCPRM1, regValue0);
	}

	return SUCCESS;
}

int32 rtl8651_getAsicDscpRemark(enum PORTID port, enum PRIORITYVALUE priority, int* remark)
{
	int regValue0, regValue1;
	if ((port < PHY0) || (port > CPU) || (priority < PRI0) || (priority > PRI7) || (remark == NULL))
		return FAILED;

	regValue1 = READ_MEM32(DSCPRM1);
	if(regValue1 & (0x1 << (23 + port))){
		if(priority < 5){
			regValue0 = READ_MEM32(DSCPRM0);
			*remark = (regValue0>>(6*priority)) & 0x3f;
		}else{
			*remark = (regValue1>>(6*(priority-5))) & 0x3f;
		}
	}else{
			return FAILED;
	}

	return SUCCESS;
}



/*=========================================
  * ASIC DRIVER API: Packet Scheduling Control Register
  *=========================================*/
/*
@func int32 | rtl8651_setAsicPriorityDecision | set priority selection
@parm uint32 | portpri | output queue decision priority assign for Port Based Priority.
@parm uint32 | dot1qpri | output queue decision priority assign for 1Q Based Priority.
@parm uint32 | dscppri | output queue decision priority assign for DSCP Based Priority
@parm uint32 | aclpri | output queue decision priority assign for ACL Based Priority.
@parm uint32 | natpri | output queue decision priority assign for NAT Based Priority.
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
#if defined(CONFIG_RTL_8197F)
int32 rtl8651_setAsicPriorityDecision( uint32 portpri, uint32 dot1qpri, uint32 dscppri, uint32 aclpri, uint32 natpri, uint32 vidpri )
#else
int32 rtl8651_setAsicPriorityDecision( uint32 portpri, uint32 dot1qpri, uint32 dscppri, uint32 aclpri, uint32 natpri)
#endif
{
	/* Invalid input parameter */
	if ((portpri < 0) || (portpri > 0xF) || (dot1qpri < 0) || (dot1qpri > 0xF) ||
		(dscppri < 0) || (dscppri > 0xF) || (aclpri < 0) || (aclpri > 0xF) ||
		(natpri < 0) || (natpri > 0xF))
		return FAILED;

	#if defined(CONFIG_RTL_8197F)
	if((vidpri < 0) || (vidpri > 0xF))
		return FAILED;
	#endif

	#if defined(CONFIG_RTL_8197F)
	WRITE_MEM32(QIDDPCR, (portpri << PBP_PRI_OFFSET) | (dot1qpri << BP8021Q_PRI_OFFSET) |
		                 (dscppri << DSCP_PRI_OFFSET) | (aclpri << ACL_PRI_OFFSET) |
		                 (natpri << NAPT_PRI_OFFSET) | (vidpri << VID_PRI_OFFSET));
	#else
	WRITE_MEM32(QIDDPCR, (portpri << PBP_PRI_OFFSET) | (dot1qpri << BP8021Q_PRI_OFFSET) |
		                 (dscppri << DSCP_PRI_OFFSET) | (aclpri << ACL_PRI_OFFSET) |
		                 (natpri << NAPT_PRI_OFFSET));
	#endif
		
	return SUCCESS;
}

#if defined(CONFIG_RTL_8197F)
int32 rtl8651_getAsicPriorityDecision( uint32* portpri, uint32* dot1qpri, uint32* dscppri, uint32* aclpri, uint32* natpri, uint32* vidpri )
#else
int32 rtl8651_getAsicPriorityDecision( uint32* portpri, uint32* dot1qpri, uint32* dscppri, uint32* aclpri, uint32* natpri)
#endif
{
	uint32 temp;
	if((portpri == NULL) ||(dot1qpri == NULL) ||(dscppri == NULL) ||(aclpri == NULL) ||(natpri == NULL))
		return FAILED;

	temp = READ_MEM32(QIDDPCR);
	*portpri = (temp >> PBP_PRI_OFFSET) & 0xf;
	*dot1qpri = (temp >> BP8021Q_PRI_OFFSET) & 0xf;
	*dscppri = (temp >> DSCP_PRI_OFFSET) & 0xf;
	*aclpri = (temp >> ACL_PRI_OFFSET) & 0xf;
	*natpri  = (temp >> NAPT_PRI_OFFSET) & 0xf;
	#if defined(CONFIG_RTL_8197F)
	*vidpri = (temp >> VID_PRI_OFFSET) & 0xf;
	#endif

	return SUCCESS;
}


int32 rtl8651_setAsicQueueFlowControlConfigureRegister(enum PORTID port, enum QUEUEID queue, uint32 enable)
{
	switch (port)
	{
		case PHY0:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x1<<(queue+Q_P0_EN_FC_OFFSET))) | (enable << (queue+Q_P0_EN_FC_OFFSET)));  break;
		case PHY1:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x1<<(queue+Q_P1_EN_FC_OFFSET))) | (enable << (queue+Q_P1_EN_FC_OFFSET)));  break;
		case PHY2:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x1<<(queue+Q_P2_EN_FC_OFFSET))) | (enable << (queue+Q_P2_EN_FC_OFFSET)));  break;
		case PHY3:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x1<<(queue+Q_P3_EN_FC_OFFSET))) | (enable << (queue+Q_P3_EN_FC_OFFSET)));  break;
		case PHY4:
			WRITE_MEM32(FCCR1, (READ_MEM32(FCCR1) & ~(0x1<<(queue+Q_P4_EN_FC_OFFSET))) | (enable << (queue+Q_P4_EN_FC_OFFSET)));  break;
		case PHY5:
			WRITE_MEM32(FCCR1, (READ_MEM32(FCCR1) & ~(0x1<<(queue+Q_P5_EN_FC_OFFSET))) | (enable << (queue+Q_P5_EN_FC_OFFSET)));  break;
		case CPU:
			WRITE_MEM32(FCCR1, (READ_MEM32(FCCR1) & ~(0x1<<(queue+Q_P6_EN_FC_OFFSET))) | (enable << (queue+Q_P6_EN_FC_OFFSET)));  break;
		default:
			return FAILED;
	}

	return SUCCESS;
}

int32 rtl8651_getAsicQueueFlowControlConfigureRegister(enum PORTID port, enum QUEUEID queue, uint32 *enable)
{
	if (enable != NULL)
	{
		switch (port)
		{
			case PHY0:
				*enable = (READ_MEM32(FCCR0) & (0x1<<(queue+Q_P0_EN_FC_OFFSET))) >> (queue+Q_P0_EN_FC_OFFSET);  break;
			case PHY1:
				*enable = (READ_MEM32(FCCR0) & (0x1<<(queue+Q_P1_EN_FC_OFFSET))) >> (queue+Q_P1_EN_FC_OFFSET);  break;
			case PHY2:
				*enable = (READ_MEM32(FCCR0) & (0x1<<(queue+Q_P2_EN_FC_OFFSET))) >> (queue+Q_P2_EN_FC_OFFSET);  break;
			case PHY3:
				*enable = (READ_MEM32(FCCR0) & (0x1<<(queue+Q_P3_EN_FC_OFFSET))) >> (queue+Q_P3_EN_FC_OFFSET);  break;
			case PHY4:
				*enable = (READ_MEM32(FCCR1) & (0x1<<(queue+Q_P4_EN_FC_OFFSET))) >> (queue+Q_P4_EN_FC_OFFSET);  break;
			case PHY5:
				*enable = (READ_MEM32(FCCR1) & (0x1<<(queue+Q_P5_EN_FC_OFFSET))) >> (queue+Q_P5_EN_FC_OFFSET);  break;
			case CPU:
				*enable = (READ_MEM32(FCCR1) & (0x1<<(queue+Q_P6_EN_FC_OFFSET))) >> (queue+Q_P6_EN_FC_OFFSET);  break;
			default:
				return FAILED;
		}
	}

	return SUCCESS;
}

/*
@func int32 | rtl8651_setAsicLBParameter | set Leaky Bucket Paramters
@parm uint32 | token | Token is used for adding budget in each time slot.
@parm uint32 | tick | Tick is used for time slot size slot.
@parm uint32 | hiThreshold | leaky bucket token high-threshold register
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicLBParameter( uint32 token, uint32 tick, uint32 hiThreshold )
{
	WRITE_MEM32( ELBPCR, (READ_MEM32(ELBPCR) & ~(Token_MASK | Tick_MASK)) | (token << Token_OFFSET) | (tick << Tick_OFFSET));
	WRITE_MEM32( ELBTTCR, (READ_MEM32(ELBTTCR) & ~0xFFFF/*L2_MASK*/) | (hiThreshold << L2_OFFSET));
	WRITE_MEM32( ILBPCR2, (READ_MEM32(ILBPCR2) & ~(ILB_feedToken_MASK|ILB_Tick_MASK)) | (token << ILB_feedToken_OFFSET) | (tick << ILB_Tick_OFFSET) );
	return SUCCESS;
}


/*
@func int32 | rtl8651_getAsicLBParameter | get Leaky Bucket Paramters
@parm uint32* | pToken | pointer to return token
@parm uint32* | pTick | pointer to return tick
@parm uint32* | pHiThreshold | pointer to return hiThreshold
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicLBParameter( uint32* pToken, uint32* pTick, uint32* pHiThreshold )
{
	uint32 regValue;

	regValue = READ_MEM32(ELBPCR);

	if (pToken != NULL)
		*pToken = (regValue & Token_MASK) >> Token_OFFSET;
	if (pTick != NULL)
		*pTick = (regValue & Tick_MASK) >> Tick_OFFSET;
	if (pHiThreshold != NULL)
		*pHiThreshold = (READ_MEM32(ELBTTCR) & 0xFF) >> L2_OFFSET;

	return SUCCESS;
}

/*port based priority*/
int32 rtl8651_setAsicPortBasedPriority( enum PORTID port, enum PRIORITYVALUE priority )
{
	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7) ||(port < PHY0) || (port> EXT3) )
		return FAILED;

	WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(0x7 << (port*3))) | (priority << (port*3)));

	return SUCCESS;
}

int32 rtl8651_getAsicPortBasedPriority( enum PORTID port, enum PRIORITYVALUE* priority )
{
	/* Invalid input parameter */
	if ((port < PHY0) || (port> EXT3) ||(priority == NULL))
		return FAILED;

	*priority = (READ_MEM32(PBPCR) >> (3*port)) & 0x7;

	return SUCCESS;
}



/*
@func int32 | rtl8651_setAsicQueueRate | set per queue rate
@parm enum PORTID | port | the port number
@parm enum QUEUEID | queueid | the queue ID wanted to set
@parm uint32 | pprTime | Peak Packet Rate (in times of APR). 0~6: PPR = (2^pprTime)*apr. 7: disable PPR
@parm uint32 | aprBurstSize | Bucket Burst Size of Average Packet Rate (unit: 1KByte). 0xFF: disable
@parm uint32 | apr | Average Packet Rate (unit: 64Kbps). 0x3FFF: unlimited rate
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicQueueRate( enum PORTID port, enum QUEUEID queueid, uint32 pprTime, uint32 aprBurstSize, uint32 apr )
{
	uint32 reg1, regValue;
	
	if ((port < PHY0) || (port > CPU) || (queueid < QUEUE0) || (queueid > MAX_QUEUE))
			return FAILED;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)	
	if(queueid < QUEUE6)
		reg1 = P0Q0RGCR + (port * 0x18) + (queueid * 0x4);  /* offset to get corresponding register */
	else
		reg1 = P0Q6RGCR + (port * 0x8) + ((queueid-QUEUE6) * 0x4);	/* offset to get corresponding register */

#else
	reg1 = P0Q0RGCR + (port * 0x18) + (queueid * 0x4);  /* offset to get corresponding register */
#endif

	regValue = READ_MEM32(reg1) & ~(PPR_MASK | L1_MASK | APR_MASK);
	regValue |= ((pprTime << PPR_OFFSET) | (aprBurstSize << L1_OFFSET) | (apr << APR_OFFSET));
	WRITE_MEM32( reg1, regValue);
	

	return SUCCESS;
}


/*
@func int32 | rtl8651_getAsicQueueRate | get per queue rate configuration
@parm enum PORTID | port | the port number
@parm enum QUEUEID | queueid | the queue ID wanted to set
@parm uint32* | pPprTime | pointer to Peak Packet Rate (in times of APR). 0~6: PPR = (2^pprTime)*apr. 7: disable PPR
@parm uint32* | pAprBurstSize | pointer to APR Burst Size (unit: 1KBytes). 0xff: disable
@parm uint32* | pApr | pointer to Average Packet Rate (unit: 64Kbps). 0x3FFF: unlimited rate
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicQueueRate( enum PORTID port, enum QUEUEID queueid, uint32* pPprTime, uint32* pAprBurstSize, uint32* pApr )
{
	uint32 reg1, regValue;

	if ((port < PHY0) || (port > CPU) || (queueid < QUEUE0) || (queueid > MAX_QUEUE))
		return FAILED;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if(queueid < QUEUE6)
		reg1 = P0Q0RGCR + (port * 0x18) + (queueid * 0x4);	/* offset to get corresponding register */
	else
		reg1 = P0Q6RGCR + (port * 0x8) + ((queueid-QUEUE6) * 0x4);	/* offset to get corresponding register */
#else
	reg1 = P0Q0RGCR + (port * 0x18) + (queueid * 0x4);  /* offset to get corresponding register */
#endif

	regValue = READ_MEM32(reg1);

	if (pPprTime != NULL)
		*pPprTime = (regValue & PPR_MASK) >> PPR_OFFSET;
	if (pAprBurstSize != NULL)
		*pAprBurstSize = (regValue & L1_MASK) >> L1_OFFSET;
	if (pApr != NULL)
		*pApr = (regValue & APR_MASK) >> APR_OFFSET;
	return SUCCESS;
}

/*
@func int32 | rtl8651_setAsicPortIngressBandwidth | set per-port total ingress bandwidth
@parm enum PORTID | port | the port number
@parm uint32 | bandwidth | the total ingress bandwidth (unit: 16Kbps), 0:disable
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicPortIngressBandwidth( enum PORTID port, uint32 bandwidth)
{
	uint32 reg1;
	#if defined(CONFIG_RTL_8197F)
	uint32 bandwidth_high = 0;
	#endif

	/* For ingress bandwidth control, its only for PHY0 to PHY5 */
	if ((port < PHY0) || (port > PHY5))
		return FAILED;

	reg1 = IBCR0 + ((port / 2) * 0x04);		/* offset to get corresponding register */

	if ( port % 2)
	{	/* ODD-port */
		WRITE_MEM32( reg1, ((READ_MEM32(reg1) & ~(IBWC_ODDPORT_MASK)) | ((bandwidth << IBWC_ODDPORT_OFFSET) & IBWC_ODDPORT_MASK)));
	} else
	{	/* EVEN-port */
		WRITE_MEM32( reg1, ((READ_MEM32(reg1) & ~(IBWC_EVENPORT_MASK)) | ((bandwidth << IBWC_EVENPORT_OFFSET) & IBWC_EVENPORT_MASK)));
	}

	#if defined(CONFIG_RTL_8197F)
	bandwidth_high = bandwidth>>IBCR3_HIGH_OFFSET;
	WRITE_MEM32( IBCR3, ((READ_MEM32(IBCR3) & (~(IBCR3_PORT_MASK<<IBCR3_PORT_OFFSET(port)))) |((bandwidth_high&IBCR3_PORT_MASK)<<IBCR3_PORT_OFFSET(port))));
	#endif

	return SUCCESS;
}

/*
@func int32 | rtl8651_getAsicPortIngressBandwidth | get per-port total ingress bandwidth
@parm enum PORTID | port | the port number
@parm uint32* | pBandwidth | pointer to the returned total ingress bandwidth (unit: 16Kbps), 0:disable
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicPortIngressBandwidth( enum PORTID port, uint32* pBandwidth )
{
	uint32 reg1, regValue;
	#if defined(CONFIG_RTL_8197F)
	uint32 pBandwidth_high = 0;
	#endif

	/* For ingress bandwidth control, its only for PHY0 to PHY5 */
	if ((port < PHY0) || (port > PHY5))
		return FAILED;

	reg1 = IBCR0 + ((port / 2) * 0x04);		/* offset to get corresponding register */

	regValue = READ_MEM32(reg1);

	if (pBandwidth != NULL)
	{
		*pBandwidth = (port % 2)?
						/* Odd port */((regValue & IBWC_ODDPORT_MASK) >> IBWC_ODDPORT_OFFSET):
						/* Even port */((regValue & IBWC_EVENPORT_MASK) >> IBWC_EVENPORT_OFFSET);
	}

	#if defined(CONFIG_RTL_8197F)
	regValue = READ_MEM32(IBCR3);
	pBandwidth_high = (regValue&(IBCR3_PORT_MASK<<IBCR3_PORT_OFFSET(port)))>>IBCR3_PORT_OFFSET(port);
	*pBandwidth = (pBandwidth_high<<IBCR3_HIGH_OFFSET) |*pBandwidth;
	#endif

	return SUCCESS;
}


/*
@func int32 | rtl8651_setAsicPortEgressBandwidth | set per-port total egress bandwidth
@parm enum PORTID | port | the port number
@parm uint32 | bandwidth | the total egress bandwidth (unit: 64kbps). 0x3FFF: disable
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicPortEgressBandwidth( enum PORTID port, uint32 bandwidth )
{
	uint32 reg1;

#ifdef CONFIG_RTK_VOIP_QOS
	extern int wan_port_check(int port);
#endif
	if ((port < PHY0) || (port > CPU))
		return FAILED;

	reg1 = WFQRCRP0 + (port * 0xC);  /* offset to get corresponding register */
	WRITE_MEM32( reg1, (READ_MEM32(reg1) & ~(APR_MASK)) | (bandwidth << APR_OFFSET));

#ifdef CONFIG_RTK_VOIP_QOS
	if(wan_port_check(port))
	{
		if(bandwidth < 161 )
			rtl8651_cpu_tx_fc(0);//disable flow control
		else
			rtl8651_cpu_tx_fc(1);//enable flow control
	}
#endif
	return SUCCESS;
}


/*
@func int32 | rtl8651_getAsicPortEgressBandwidth | get per-port total egress bandwidth
@parm enum PORTID | port | the port number
@parm uint32* | pBandwidth | pointer to the returned total egress bandwidth (unit: 64kbps). 0x3FFF: disable
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicPortEgressBandwidth( enum PORTID port, uint32* pBandwidth )
{
	uint32 reg1, regValue;

	if ((port < PHY0) || (port > CPU))
		return FAILED;

	reg1 = WFQRCRP0 + (port * 0xC);  /* offset to get corresponding register */
	regValue = READ_MEM32(reg1);

	if (pBandwidth != NULL)
		*pBandwidth = (regValue & APR_MASK) >> APR_OFFSET;

	return SUCCESS;
}

/*set queue type as STRICT*/
int32 rtl8651_setAsicQueueStrict( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE queueType)
{
	uint32 reg1, regOFFSET, regValue;

	if ((port < PHY0) || (port > CPU) || (queueid < QUEUE0) || (queueid > MAX_QUEUE))
		return FAILED;
	if ((queueType < STR_PRIO) || (queueType > WFQ_PRIO))
		return FAILED;
	
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if(queueid < QUEUE6)
	{
		reg1 = WFQWCR0P0 + (port * 0xC) + ((queueid >> 2) * 0x4);	/* offset to get corresponding register */
		regOFFSET = (queueid % 4) * 0x8;	/* used to offset register value */

		regValue = READ_MEM32(reg1) & ~((WEIGHT0_MASK | SCHE0_MASK) << regOFFSET);
		regValue |= (queueType << (SCHE0_OFFSET + regOFFSET));
		WRITE_MEM32( reg1, regValue);
	}
	else
	{
		reg1 = WFQWCR1P0_98C + (port * 0x4) + (((queueid-QUEUE6) >> 2) * 0x4);	/* offset to get corresponding register */
		regOFFSET = ((queueid- QUEUE6) % 4) * 0x8;	/* used to offset register value */

		regValue = READ_MEM32(reg1) & ~((WEIGHT0_MASK | SCHE0_MASK) << regOFFSET);
		regValue |= (queueType << (SCHE0_OFFSET + regOFFSET));
		WRITE_MEM32( reg1, regValue);
	}
#else
	reg1 = WFQWCR0P0 + (port * 0xC) + ((queueid >> 2) * 0x4);	/* offset to get corresponding register */
	regOFFSET = (queueid % 4) * 0x8;	/* used to offset register value */

	regValue = READ_MEM32(reg1) & ~((WEIGHT0_MASK | SCHE0_MASK) << regOFFSET);
	regValue |= (queueType << (SCHE0_OFFSET + regOFFSET));
	WRITE_MEM32( reg1, regValue);
#endif	
	return SUCCESS;
}

int32 rtl8651_getAsicQueueStrict( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE *pQueueType)
{
	uint32 reg1, regOFFSET, regValue;
	
	if ((port < PHY0) || (port > CPU) || (queueid < QUEUE0) || (queueid > MAX_QUEUE) ||(pQueueType == NULL))
		return FAILED;
	
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)	
	if(queueid < QUEUE6)
	{
		reg1 = WFQWCR0P0 + (port * 0xC) + ((queueid >> 2) * 0x4);  /* offset to get corresponding register */
		regOFFSET = (queueid % 4) * 0x8;	/* used to offset register value */
		regValue = READ_MEM32(reg1);

		if (pQueueType != NULL)
			*pQueueType = ((regValue & (SCHE0_MASK << regOFFSET)) >> SCHE0_OFFSET) >> regOFFSET;
	}
	else
	{
		reg1 = WFQWCR1P0_98C + (port * 0x4) + (((queueid -QUEUE6)>>2) * 0x4);  /* offset to get corresponding register */
		regOFFSET = ((queueid- QUEUE6)% 4) * 0x8;	/* used to offset register value */
		regValue = READ_MEM32(reg1);

		if (pQueueType != NULL)
			*pQueueType = ((regValue & (SCHE0_MASK << regOFFSET)) >> SCHE0_OFFSET) >> regOFFSET;
	}
#else
	reg1 = WFQWCR0P0 + (port * 0xC) + ((queueid >> 2) * 0x4);  /* offset to get corresponding register */
	regOFFSET = (queueid % 4) * 0x8;	/* used to offset register value */
	regValue = READ_MEM32(reg1);

	if (pQueueType != NULL)
		*pQueueType = ((regValue & (SCHE0_MASK << regOFFSET)) >> SCHE0_OFFSET) >> regOFFSET;
#endif
	return SUCCESS;
}


/*
@func int32 | rtl8651_setAsicQueueWeight | set WFQ weighting
@parm enum PORTID | port | the port number
@parm enum QUEUEID | queueid | the queue ID wanted to set
@parm enum QUEUETYPE | queueType | the specified queue type
@parm uint32 | weight | the weight value wanted to set (valid:0~127)
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicQueueWeight( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE queueType, uint32 weight )
{
	uint32 reg1, regOFFSET, regValue;

	if ((port < PHY0) || (port > CPU) || (queueid < QUEUE0) || (queueid > MAX_QUEUE))
		return FAILED;

	if ((queueType < STR_PRIO) || (queueType > WFQ_PRIO))
		return FAILED;
	if((weight < 0) && (weight > 127))
		return FAILED;
	
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)	
	if(queueid < QUEUE6){
		reg1 = WFQWCR0P0 + (port * 0xC) + ((queueid >> 2) * 0x4);	/* offset to get corresponding register */	
		regOFFSET = (queueid % 4) * 0x8;	/* used to offset register value */
	}
	else{
		reg1 = WFQWCR1P0_98C + (port * 0x4) + (((queueid -QUEUE6) >> 2) * 0x4);	/* offset to get corresponding register */
		regOFFSET = ((queueid- QUEUE6)% 4) * 0x8;	/* used to offset register value */
	}	
		
#else
	reg1 = WFQWCR0P0 + (port * 0xC) + ((queueid >> 2) * 0x4);	/* offset to get corresponding register */
	regOFFSET = (queueid % 4) * 0x8;	/* used to offset register value */
#endif

	regValue = READ_MEM32(reg1) & ~((WEIGHT0_MASK | SCHE0_MASK) << regOFFSET);
	regValue |= ((queueType << (SCHE0_OFFSET + regOFFSET)) | (weight << (WEIGHT0_OFFSET + regOFFSET)));
	WRITE_MEM32( reg1, regValue);
	
	return SUCCESS;
}


/*
@func int32 | rtl8651_getAsicQueueWeight | get WFQ weighting
@parm enum PORTID | port | the port number
@parm enum QUEUEID | queueid | the queue ID wanted to set
@parm enum QUEUETYPE* | pQueueType | pointer to the returned queue type
@parm uint32* | pWeight | pointer to the returned weight value
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicQueueWeight( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE *pQueueType, uint32 *pWeight )
{
	uint32 reg1, regOFFSET, regValue;

	if ((port < PHY0) || (port > CPU) || (queueid < QUEUE0) || (queueid > MAX_QUEUE))
		return FAILED;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if(queueid < QUEUE6)
		reg1 = WFQWCR0P0 + (port * 0xC) + ((queueid >> 2) * 0x4);	/* offset to get corresponding register */
	else
		reg1 = WFQWCR1P0_98C + (port * 0x4) + (((queueid -QUEUE6) >> 2) * 0x4); /* offset to get corresponding register */

	if(queueid < QUEUE6)
		regOFFSET = (queueid% 4) * 0x8;
	else
		regOFFSET = ((queueid - QUEUE6)% 4) * 0x8;
	
	//regOFFSET = ((queueid- QUEUE6)% 4) * 0x8;	/* used to offset register value */			
#else
	reg1 = WFQWCR0P0 + (port * 0xC) + ((queueid >> 2) * 0x4);  /* offset to get corresponding register */
	regOFFSET = (queueid % 4) * 0x8;	/* used to offset register value */
#endif
	regValue = READ_MEM32(reg1);

	if (pQueueType != NULL)
		*pQueueType = ((regValue & (SCHE0_MASK << regOFFSET)) >> SCHE0_OFFSET) >> regOFFSET;
	if (pWeight != NULL)
		*pWeight = ((regValue & (WEIGHT0_MASK << regOFFSET)) >> WEIGHT0_OFFSET) >> regOFFSET;

	return SUCCESS;
}

/*
@func int32 | rtl8651_setAsicOutputQueueNumber | set output queue number for a specified port
@parm enum PORTID | port | the port number (valid: physical ports(0~5) and CPU port(6) )
@parm enum QUEUENUM | qnum | the output queue number
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicOutputQueueNumber( enum PORTID port, enum QUEUENUM qnum )
{
	/* Invalid input parameter */

	enum QUEUENUM orgQnum;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if ((port < PHY0) || (port > CPU) || (qnum < QNUM1) || (qnum > QNUM8))
		return FAILED;
	if(qnum==QNUM8)
		WRITE_MEM32(PQGCR8, (READ_MEM32(PQGCR8) |EN_8PRI_Q_MASK));
	else
		WRITE_MEM32(PQGCR8, (READ_MEM32(PQGCR8) &(~EN_8PRI_Q_MASK)));

	
	orgQnum = (READ_MEM32(QNUMCR) >> (3*port)) & 0x7;
	
	WRITE_MEM32(QNUMCR, (READ_MEM32(QNUMCR) & ~(0x7 << (3*port))) | ((qnum-1) << (3*port)));
#else

	if ((port < PHY0) || (port > CPU) || (qnum < QNUM1) || (qnum > QNUM6))
		return FAILED;

	orgQnum = (READ_MEM32(QNUMCR) >> (3*port)) & 0x7;
	WRITE_MEM32(QNUMCR, (READ_MEM32(QNUMCR) & ~(0x7 << (3*port))) | (qnum << (3*port)));
#endif

#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
	if (qnum==6)
	{
		if (orgQnum!=6)
			_rtl865x_setQosThresholdByQueueIdx(QNUM_IDX_6);
	}
	else if (qnum>3)
	{
		if((orgQnum==6)||orgQnum<4)
			_rtl865x_setQosThresholdByQueueIdx(QNUM_IDX_45);
	}
	else
	{
		if(orgQnum>3)
			_rtl865x_setQosThresholdByQueueIdx(QNUM_IDX_123);
	}
#endif

	return SUCCESS;
}


/*
@func int32 | rtl8651_getAsicOutputQueueNumber | get output queue number for a specified port
@parm enum PORTID | port | the port number (valid: physical ports(0~5) and CPU port(6) )
@parm enum QUEUENUM | qnum | the output queue number
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicOutputQueueNumber( enum PORTID port, enum QUEUENUM *qnum )
{
	/* Invalid input parameter */
	if ((port < PHY0) || (port > CPU))
		return FAILED;

	if (qnum != NULL)
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		*qnum = ((READ_MEM32(QNUMCR) >> (3*port)) & 0x7)+1;
#else
		*qnum = (READ_MEM32(QNUMCR) >> (3*port)) & 0x7;
#endif

	return SUCCESS;
}

/*
@func int32 | rtl8651_setAsicPriorityToQIDMappingTable | set user priority to QID mapping table parameter
@parm enum QUEUENUM | qnum | the output queue number
@parm enum PRIORITYVALUE | priority | priority
@parm enum QUEUEID | qid | queue ID
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicPriorityToQIDMappingTable( enum QUEUENUM qnum, enum PRIORITYVALUE priority, enum QUEUEID qid )
{
	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7))
		return FAILED;
	
	if ((qid < QUEUE0) || (qid > MAX_QUEUE))
		return FAILED;
	
	switch (qnum)
	{
		case QNUM1:
			WRITE_MEM32(UPTCMCR0, (READ_MEM32(UPTCMCR0) & ~(0x7 << (priority*3))) | (qid << (priority*3))); break;
		case QNUM2:
			WRITE_MEM32(UPTCMCR1, (READ_MEM32(UPTCMCR1) & ~(0x7 << (priority*3))) | (qid << (priority*3))); break;
		case QNUM3:
			WRITE_MEM32(UPTCMCR2, (READ_MEM32(UPTCMCR2) & ~(0x7 << (priority*3))) | (qid << (priority*3))); break;
		case QNUM4:
			WRITE_MEM32(UPTCMCR3, (READ_MEM32(UPTCMCR3) & ~(0x7 << (priority*3))) | (qid << (priority*3))); break;
		case QNUM5:
			WRITE_MEM32(UPTCMCR4, (READ_MEM32(UPTCMCR4) & ~(0x7 << (priority*3))) | (qid << (priority*3))); break;
		case QNUM6:
			WRITE_MEM32(UPTCMCR5, (READ_MEM32(UPTCMCR5) & ~(0x7 << (priority*3))) | (qid << (priority*3))); break;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		case QNUM7:
			WRITE_MEM32(UPTCMCR6, (READ_MEM32(UPTCMCR6) & ~(0x7 << (priority*3))) | (qid << (priority*3))); break;			
		case QNUM8: 
			WRITE_MEM32(UPTCMCR7, (READ_MEM32(UPTCMCR7) & ~(0x7 << (priority*3))) | (qid << (priority*3))); break;
#endif
		default:
			return FAILED;
	}

	return SUCCESS;
}

int32 rtl8651_getAsicPriorityToQIDMappingTable( enum QUEUENUM qnum, enum PRIORITYVALUE priority, enum QUEUEID* qid )
{
	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7) ||(qid == NULL))
		return FAILED;

	switch (qnum)
	{
		case QNUM1:
			*qid = (READ_MEM32(UPTCMCR0) >> (priority*3)) & 0x7;
			break;
		case QNUM2:
			*qid = (READ_MEM32(UPTCMCR1) >> (priority*3)) & 0x7;
			break;
		case QNUM3:
			*qid = (READ_MEM32(UPTCMCR2) >> (priority*3)) & 0x7;
			break;
		case QNUM4:
			*qid = (READ_MEM32(UPTCMCR3) >> (priority*3)) & 0x7;
			break;
		case QNUM5:
			*qid = (READ_MEM32(UPTCMCR4) >> (priority*3)) & 0x7;
			break;
		case QNUM6:
			*qid = (READ_MEM32(UPTCMCR5) >> (priority*3)) & 0x7;
			break;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		case QNUM7:
			*qid = (READ_MEM32(UPTCMCR6) >> (priority*3)) & 0x7;
			break;
		case QNUM8: 
			*qid = (READ_MEM32(UPTCMCR7) >> (priority*3)) & 0x7;
			break;
#endif	
		default:
			return FAILED;
	}

	return SUCCESS;
}


/*
@func int32 | rtl8651_setAsicCPUPriorityToQIDMappingTable | set user priority to QID mapping table parameter based on destination port & priority information
@parm enum PORTID | port | the destination port
@parm enum PRIORITYVALUE | priority | priority
@parm enum QUEUEID | qid | queue ID
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicCPUPriorityToQIDMappingTable( enum PORTID port, enum PRIORITYVALUE priority, enum QUEUEID qid )
{
	uint32	reg;
	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7))
		return FAILED;

	if ((qid < QUEUE0) || (qid > MAX_QUEUE))
		return FAILED;

	if (port<CPU || port>MULTEXT)
		return FAILED;

	reg = (uint32)(((uint32*)CPUQIDMCR0) + (port-CPU));

	WRITE_MEM32(reg, (READ_MEM32(reg) & ~(0x7 << (priority<<2))) | (qid << (priority<<2)));

	return SUCCESS;
}


int32 rtl8651_setAsicSystemBasedFlowControlRegister(uint32 sharedON, uint32 sharedOFF, uint32 fcON, uint32 fcOFF, uint32 drop)
{
	/* Invalid input parameter */
	if ((sharedON > (SDC_FCON_MASK >> SDC_FCON_OFFSET)) ||
		(sharedOFF > (S_DSC_FCOFF_MASK >> S_DSC_FCOFF_OFFSET)) ||
		(fcON > ((S_Max_SBuf_FCON_MASK >> S_Max_SBuf_FCON_OFFSET))) ||
		(fcOFF > (S_Max_SBuf_FCOFF_MASK >> S_Max_SBuf_FCOFF_OFFSET)) ||
		(drop > (S_DSC_RUNOUT_MASK >> S_DSC_RUNOUT_OFFSET)))
		return FAILED;

	WRITE_MEM32(SBFCR0, (READ_MEM32(SBFCR0) & ~(S_DSC_RUNOUT_MASK)) | (drop << S_DSC_RUNOUT_OFFSET));
	WRITE_MEM32(SBFCR1, (READ_MEM32(SBFCR1) & ~(S_DSC_FCON_MASK | S_DSC_FCOFF_MASK)) | ( fcON<< S_DSC_FCON_OFFSET) | (fcOFF << S_DSC_FCOFF_OFFSET));
	WRITE_MEM32(SBFCR2, (READ_MEM32(SBFCR2) & ~(S_Max_SBuf_FCON_MASK | S_Max_SBuf_FCOFF_MASK)) | (sharedON << S_Max_SBuf_FCON_OFFSET) | (sharedOFF << S_Max_SBuf_FCOFF_OFFSET));
	return SUCCESS;
}

int32 rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF)
{
	/* Invalid input parameter */
	if ((port < PHY0) || (port > CPU))
		return FAILED;

	if ((fcON > (QG_DSC_FCON_MASK >> QG_DSC_FCON_OFFSET)) ||
		(fcOFF > (QG_DSC_FCOFF_MASK >> QG_DSC_FCOFF_OFFSET)))
		return FAILED;


	switch (queue)
	{
		case QUEUE0:
			WRITE_MEM32((QDBFCRP0G0+(port*0xC)), (READ_MEM32(QDBFCRP0G0+(port*0xC)) & ~(QG_DSC_FCON_MASK | QG_DSC_FCOFF_MASK)) | (fcON << QG_DSC_FCON_OFFSET) | (fcOFF << QG_DSC_FCOFF_OFFSET));
			break;
		case QUEUE1:
		case QUEUE2:
		case QUEUE3:
		case QUEUE4:
			WRITE_MEM32((QDBFCRP0G1+(port*0xC)), (READ_MEM32(QDBFCRP0G1+(port*0xC)) & ~(QG_DSC_FCON_MASK | QG_DSC_FCOFF_MASK)) | (fcON << QG_DSC_FCON_OFFSET) | (fcOFF << QG_DSC_FCOFF_OFFSET));
			break;
		case QUEUE5:
			WRITE_MEM32((QDBFCRP0G2+(port*0xC)), (READ_MEM32(QDBFCRP0G2+(port*0xC)) & ~(QG_DSC_FCON_MASK | QG_DSC_FCOFF_MASK)) | (fcON << QG_DSC_FCON_OFFSET) | (fcOFF << QG_DSC_FCOFF_OFFSET));
			break;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		case QUEUE6:			
			WRITE_MEM32((QDBFCRP0G3+(port*0x8)), (READ_MEM32(QDBFCRP0G3+(port*0x8)) & ~(QG_DSC_FCON_MASK | QG_DSC_FCOFF_MASK)) | (fcON << QG_DSC_FCON_OFFSET) | (fcOFF << QG_DSC_FCOFF_OFFSET));
			break;
		case QUEUE7:		
			WRITE_MEM32((QDBFCRP0G4+(port*0x8)), (READ_MEM32(QDBFCRP0G3+(port*0x8)) & ~(QG_DSC_FCON_MASK | QG_DSC_FCOFF_MASK)) | (fcON << QG_DSC_FCON_OFFSET) | (fcOFF << QG_DSC_FCOFF_OFFSET));
			break;	
#endif				
		default:
			return FAILED;
	}
	return SUCCESS;
}

int32 rtl8651_setAsicQueuePacketBasedFlowControlRegister(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF)
{
	/* Invalid input parameter */
	if ((port < PHY0) || (port > CPU))
		return FAILED;

	if ((fcON > (QG_QLEN_FCON_MASK>> QG_QLEN_FCON_OFFSET)) ||
		(fcOFF > (QG_QLEN_FCOFF_MASK >> QG_QLEN_FCOFF_OFFSET)))
		return FAILED;

	switch (queue)
	{
		case QUEUE0:
			WRITE_MEM32((QPKTFCRP0G0+(port*0xC)), (READ_MEM32(QPKTFCRP0G0+(port*0xC)) & ~(QG_QLEN_FCON_MASK | QG_QLEN_FCOFF_MASK)) | (fcON << QG_QLEN_FCON_OFFSET) | (fcOFF << QG_QLEN_FCOFF_OFFSET));
			break;
		case QUEUE1:
		case QUEUE2:
		case QUEUE3:
		case QUEUE4:
			WRITE_MEM32((QPKTFCRP0G1+(port*0xC)), (READ_MEM32(QPKTFCRP0G1+(port*0xC)) & ~(QG_QLEN_FCON_MASK | QG_QLEN_FCOFF_MASK)) | (fcON << QG_QLEN_FCON_OFFSET) | (fcOFF << QG_QLEN_FCOFF_OFFSET));
			break;
		case QUEUE5:
			WRITE_MEM32((QPKTFCRP0G2+(port*0xC)), (READ_MEM32(QPKTFCRP0G2+(port*0xC)) & ~(QG_QLEN_FCON_MASK | QG_QLEN_FCOFF_MASK)) | (fcON << QG_QLEN_FCON_OFFSET) | (fcOFF << QG_QLEN_FCOFF_OFFSET));
			break;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		case QUEUE6:		
			WRITE_MEM32((QPKTFCRP0G3+(port*0x8)), (READ_MEM32(QPKTFCRP0G3+(port*0x8)) & ~(QG_QLEN_FCON_MASK | QG_QLEN_FCOFF_MASK)) | (fcON << QG_QLEN_FCON_OFFSET) | (fcOFF << QG_QLEN_FCOFF_OFFSET));
			break;
		case QUEUE7:		
			WRITE_MEM32((QPKTFCRP0G4+(port*0x8)), (READ_MEM32(QPKTFCRP0G4+(port*0x8)) & ~(QG_QLEN_FCON_MASK | QG_QLEN_FCOFF_MASK)) | (fcON << QG_QLEN_FCON_OFFSET) | (fcOFF << QG_QLEN_FCOFF_OFFSET));
			break;	
#endif				
		default:
			return FAILED;
	}

	return SUCCESS;
}

int32 rtl8651_setAsicPortBasedFlowControlRegister(enum PORTID port, uint32 fcON, uint32 fcOFF)
{
	/* Invalid input parameter */
	if ((fcON > (P_MaxDSC_FCON_MASK >> P_MaxDSC_FCON_OFFSET)) ||
		(fcOFF > (P_MaxDSC_FCOFF_MASK >> P_MaxDSC_FCOFF_OFFSET)))
		return FAILED;

	switch (port)
	{
		case PHY0:
			WRITE_MEM32(PBFCR0, (READ_MEM32(PBFCR0) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;
		case PHY1:
			WRITE_MEM32(PBFCR1, (READ_MEM32(PBFCR1) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;
		case PHY2:
			WRITE_MEM32(PBFCR2, (READ_MEM32(PBFCR2) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;
		case PHY3:
			WRITE_MEM32(PBFCR3, (READ_MEM32(PBFCR3) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;
		case PHY4:
			WRITE_MEM32(PBFCR4, (READ_MEM32(PBFCR4) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;
		case PHY5:
			WRITE_MEM32(PBFCR5, (READ_MEM32(PBFCR5) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;
		case CPU:
			WRITE_MEM32(PBFCR6, (READ_MEM32(PBFCR6) & ~(P_MaxDSC_FCON_MASK | P_MaxDSC_FCOFF_MASK)) | (fcON << P_MaxDSC_FCON_OFFSET) | (fcOFF << P_MaxDSC_FCOFF_OFFSET)); break;
		default:
			return FAILED;
	}

	return SUCCESS;
}

int32 rtl8651_setAsicPerQueuePhysicalLengthGapRegister(uint32 gap)
{
	/* Invalid input parameter */
	if (gap > (QLEN_GAP_MASK >> QLEN_GAP_OFFSET))
		return FAILED;

	WRITE_MEM32(PQPLGR, (READ_MEM32(PQPLGR) & ~(QLEN_GAP_MASK)) | (gap << QLEN_GAP_OFFSET));
	return SUCCESS;
}

/* 	note: the dynamic mechanism: adjust the flow control threshold value according to the number of Ethernet link up ports.
	buffer threshold setting:
	sys on = 208, share on = 192 for link port <=3
	0xbb804504 = 0x00c000d0
	0xbb804508 = 0x00b000c0

	sys on = 172, share on = 98 , for link port > 3
	0xbb804504 = 0x00A000AC
	0xbb804508 = 0x004A0062

	1. default threshold setting is link port <=3
	2. got link change interrupt and link port > 3, then change threhosld for link port > 3
	3. got link change interrupt and link port <= 3, then change threhosld for link port <= 3
 */

#ifdef CONFIG_RTL_8197D_DYN_THR
int32 rtl819x_setQosThreshold(uint32 old_sts, uint32 new_sts)
{
	int32 i, link_up_ports=0;
	uint32 j = new_sts;

	if (old_sts == new_sts)
		return SUCCESS;

	for (i=0; i<5; i++) {
		if ((j & 0x1) == 1)
			link_up_ports++;
		j = j >> 1;
	}

	if (link_up_ports <= DYN_THR_LINK_UP_PORTS) {
		WRITE_MEM32(SBFCR1, (READ_MEM32(SBFCR1) & ~(S_DSC_FCON_MASK | S_DSC_FCOFF_MASK)) | ( DYN_THR_AGG_fcON<< S_DSC_FCON_OFFSET) | (DYN_THR_AGG_fcOFF << S_DSC_FCOFF_OFFSET));
		WRITE_MEM32(SBFCR2, (READ_MEM32(SBFCR2) & ~(S_Max_SBuf_FCON_MASK | S_Max_SBuf_FCOFF_MASK)) | (DYN_THR_AGG_sharedON << S_Max_SBuf_FCON_OFFSET) | (DYN_THR_AGG_sharedOFF << S_Max_SBuf_FCOFF_OFFSET));
	}
	else {
		WRITE_MEM32(SBFCR1, (READ_MEM32(SBFCR1) & ~(S_DSC_FCON_MASK | S_DSC_FCOFF_MASK)) | ( DYN_THR_DEF_fcON<< S_DSC_FCON_OFFSET) | (DYN_THR_DEF_fcOFF << S_DSC_FCOFF_OFFSET));
		WRITE_MEM32(SBFCR2, (READ_MEM32(SBFCR2) & ~(S_Max_SBuf_FCON_MASK | S_Max_SBuf_FCOFF_MASK)) | (DYN_THR_DEF_sharedON << S_Max_SBuf_FCON_OFFSET) | (DYN_THR_DEF_sharedOFF << S_Max_SBuf_FCOFF_OFFSET));
	}

	return SUCCESS;
}
#endif

#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
static int32 _rtl865x_setQosThresholdByQueueIdx(uint32 qidx)
{
	/* Set the threshold value for qos sytem */
	int32 retval;
	int32	i,j;

	printk("Set threshould idx %d\n", qidx);
	retval = rtl8651_setAsicSystemBasedFlowControlRegister(outputQueuePara[qidx].systemSBFCON, outputQueuePara[qidx].systemSBFCOFF, outputQueuePara[qidx].systemFCON, outputQueuePara[qidx].systemFCOFF, outputQueuePara[qidx].drop);
	if (retval!= SUCCESS)
	{
		rtlglue_printf("Set System Base Flow Control Para Error.\n");
		return retval;
	}

	for(i =0; i < RTL8651_OUTPUTQUEUE_SIZE; i++)
	{
		retval = rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(0, i, outputQueuePara[qidx].queueDescFCON, outputQueuePara[qidx].queueDescFCOFF);
		if (retval!= SUCCESS)
		{
			rtlglue_printf("Set Queue Descriptor Base Flow Control Para Error.\n");
			return retval;
		}
		for(j=1;j<=CPU;j++)
			rtl8651_setAsicQueueDescriptorBasedFlowControlRegister(PHY0+j, i, outputQueuePara[qidx].queueDescFCON, outputQueuePara[qidx].queueDescFCOFF);


		retval = rtl8651_setAsicQueuePacketBasedFlowControlRegister(0, i, outputQueuePara[qidx].queuePktFCON, outputQueuePara[qidx].queuePktFCOFF);
		if (retval!= SUCCESS)
		{
			rtlglue_printf("Set Queue Packet Base Flow Control Para Error.\n");
			return retval;
		}
		for(j=1;j<=CPU;j++)
			rtl8651_setAsicQueuePacketBasedFlowControlRegister(PHY0+j, i, outputQueuePara[qidx].queuePktFCON, outputQueuePara[qidx].queuePktFCOFF);

	}

	retval = rtl8651_setAsicPortBasedFlowControlRegister(0, outputQueuePara[qidx].portFCON, outputQueuePara[qidx].portFCOFF);
	if (retval!= SUCCESS)
	{
		rtlglue_printf("Set Port Base Flow Control Para Error.\n");
		return retval;
	}
	for(j=1;j<=CPU;j++)
		rtl8651_setAsicPortBasedFlowControlRegister(PHY0+j, outputQueuePara[qidx].portFCON, outputQueuePara[qidx].portFCOFF);

	retval = rtl8651_setAsicPerQueuePhysicalLengthGapRegister(outputQueuePara[qidx].gap);
	if (retval!= SUCCESS)
	{
		rtlglue_printf("Set Queue Physical Lenght Gap Reg Error.\n");
		return retval;
	}

	return SUCCESS;
}
#endif

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
static int32 _rtl865xC_QM_init( void )
{
	uint32 originalDescGetReady;
	int32 cnt;
	rtlglue_printf("Start to initiate QM\n");


	/*
	   1. Get the original decriptor usage for QM.
	 */
	WRITE_MEM32( SIRR, READ_MEM32(SIRR)| TRXRDY );
 	rtl865xC_lockSWCore();
	cnt = 0;

	do
	{
		int32 idx;

		originalDescGetReady = TRUE;	/* by default, set it to TRUE */
		_rtl865xC_QM_orgDescUsage = 0;	/* by default, set it to 0 */

		for ( idx = 0 ; idx < RTL865XC_QM_DESC_READROBUSTPARAMETER ; idx ++ )
		{
			uint32 currentDescUsage;

			currentDescUsage = (READ_MEM32( GDSR0 ) & USEDDSC_MASK) >> USEDDSC_OFFSET;
			if (	( currentDescUsage == 0 /* It's impossible */ ) ||
				(( _rtl865xC_QM_orgDescUsage != 0 ) &&
				 ( currentDescUsage != _rtl865xC_QM_orgDescUsage) ) )
			{
				rtlglue_printf("INIT swCore descriptor count Failed : (%d [%d])\n", currentDescUsage, _rtl865xC_QM_orgDescUsage);
				originalDescGetReady = FALSE;
			} else
			{
				_rtl865xC_QM_orgDescUsage = currentDescUsage;
			}
		}
		cnt++;
	} while ( originalDescGetReady != TRUE && cnt< 200000);

	if (_rtl865xC_QM_orgDescUsage<12)
		_rtl865xC_QM_orgDescUsage = 12;

	rtl865xC_unLockSWCore();

	return SUCCESS;
}
#endif

/*
@func int32 | rtl865xC_waitForOutputQueueEmpty | wait until output queue empty
@rvalue SUCCESS |
@comm
	The function will not return until all the output queue is empty.
 */
int32 rtl865xC_waitForOutputQueueEmpty(void)
{
#if defined(CONFIG_OPENWRT_SDK)
#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
	int32	currentDescUsage;
	int32	cnt, i;
	unsigned long latetime=jiffies;
	unsigned long currtime =jiffies;
#if defined (CONFIG_RTL_HW_QOS_DEBUG)
	printk("1.[%s]:[%d].\n",__FUNCTION__,__LINE__);
#endif
	#if defined(CONFIG_RTL_8196C)
	while((READ_MEM32(LAGCR1)&OUTPUTQUEUE_STAT_MASK_CR1)!=OUTPUTQUEUE_STAT_MASK_CR1);
	#elif defined(CONFIG_RTL_8198C)	|| defined(CONFIG_RTL_8197F)
	while( ((READ_MEM32(LAGCR1)&OUTPUTQUEUE_STAT_MASK_CR1)^OUTPUTQUEUE_STAT_MASK_CR1) ||
       	((READ_MEM32(LAGCR2)&OUTPUTQUEUE_STAT_MASK_CR2)^OUTPUTQUEUE_STAT_MASK_CR2) )
	{
		/*free rx ring when cpu port queue not empty */
		currtime =jiffies;
		if((time_before_eq(latetime+5*HZ,  currtime))
		&&((READ_MEM32(LAGCR2)&OUTPUTQUEUE_STAT_MASK_CPU)^OUTPUTQUEUE_STAT_MASK_CPU))
		{
			swNic_freeRxRing( );
		}
#if defined (CONFIG_RTL_HW_QOS_DEBUG)

		if (net_ratelimit() )
			printk("LAGCR1:%x,LAGCR2:%x.\n",READ_MEM32(LAGCR1),READ_MEM32(LAGCR2));
#else
				;	/* do nothing */
#endif
		
	}
	#else
	while ( ((READ_MEM32(LAGCR0)&OUTPUTQUEUE_STAT_MASK_CR0)^OUTPUTQUEUE_STAT_MASK_CR0) ||
       	((READ_MEM32(LAGCR1)&OUTPUTQUEUE_STAT_MASK_CR1)^OUTPUTQUEUE_STAT_MASK_CR1) )
   	{
#if defined (CONFIG_RTL_HW_QOS_DEBUG)

		if (net_ratelimit() )
			printk("LAGCR0:%x,LAGCR1:%x.\n",READ_MEM32(LAGCR0),READ_MEM32(LAGCR1));
#else
				;	/* do nothing */
#endif
		
	}
	#endif

	/*	There are something wrong when check the input queue is empty or not	*/
	currentDescUsage = (READ_MEM32( GDSR0 ) & USEDDSC_MASK) >> USEDDSC_OFFSET;
	cnt = (currentDescUsage-_rtl865xC_QM_orgDescUsage)<<10;
	i = 0;
	while ( ((currentDescUsage = ((READ_MEM32( GDSR0 ) & USEDDSC_MASK) >> USEDDSC_OFFSET)) > _rtl865xC_QM_orgDescUsage) && (i<cnt) )
	{
#if defined (CONFIG_RTL_HW_QOS_DEBUG)
		if (net_ratelimit() )
		printk("Waiting for input queue empty.  ==> currently used %d-%d=%d.\n",currentDescUsage, _rtl865xC_QM_orgDescUsage,(currentDescUsage-_rtl865xC_QM_orgDescUsage));
#else
		;	/* do nothing */
#endif
		i++;
	}
#if defined (CONFIG_RTL_HW_QOS_DEBUG)
	printk("2.[%s]:[%d].\n",__FUNCTION__,__LINE__);
#endif	
#endif
#else
#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
	int32	currentDescUsage;
	int32	cnt, i;

	#if defined(CONFIG_RTL_8196C)
	while((READ_MEM32(LAGCR1)&OUTPUTQUEUE_STAT_MASK_CR1)!=OUTPUTQUEUE_STAT_MASK_CR1);
	#elif defined(CONFIG_RTL_8198C)	|| defined(CONFIG_RTL_8197F)
	while( ((READ_MEM32(LAGCR1)&OUTPUTQUEUE_STAT_MASK_CR1)^OUTPUTQUEUE_STAT_MASK_CR1) ||
       	((READ_MEM32(LAGCR2)&OUTPUTQUEUE_STAT_MASK_CR2)^OUTPUTQUEUE_STAT_MASK_CR2) );
	#else
	while ( ((READ_MEM32(LAGCR0)&OUTPUTQUEUE_STAT_MASK_CR0)^OUTPUTQUEUE_STAT_MASK_CR0) ||
       	((READ_MEM32(LAGCR1)&OUTPUTQUEUE_STAT_MASK_CR1)^OUTPUTQUEUE_STAT_MASK_CR1) );
	#endif

	/*	There are something wrong when check the input queue is empty or not	*/
	currentDescUsage = (READ_MEM32( GDSR0 ) & USEDDSC_MASK) >> USEDDSC_OFFSET;
	cnt = (currentDescUsage-_rtl865xC_QM_orgDescUsage)<<10;
	//printk("currentDescUsage:%d,_rtl865xC_QM_orgDescUsage:%d,[%s]:[%d].\n",currentDescUsage,_rtl865xC_QM_orgDescUsage,__FUNCTION__,__LINE__);
	i = 0;
	while ( ((currentDescUsage = ((READ_MEM32( GDSR0 ) & USEDDSC_MASK) >> USEDDSC_OFFSET)) > _rtl865xC_QM_orgDescUsage) && (i<cnt) )
	{
#if 0
		extern int net_ratelimit(void);
		if (net_ratelimit() && i<120)
		rtlglue_printf("Waiting for input queue empty.  ==> currently used %d.\n", (currentDescUsage-_rtl865xC_QM_orgDescUsage));
#else
		;	/* do nothing */
#endif
		i++;
	}
#endif
#endif
       return SUCCESS;
}

/*
@func int32 | rtl8651_resetAsicOutputQueue | reset output queue
@rvalue SUCCESS |
@comm
	When reset is done, all queue pointer will be reset to the initial base address.
 */

 int32 rtl8651_resetAsicOutputQueue(void)
 {
	uint32	i;
	uint32	scr, pauseTicks;

 	WRITE_MEM32(QRR, 0x0);
	#if defined(CONFIG_RTL_819XD) ||defined(CONFIG_RTL_8196E) ||defined(CONFIG_RTL_8198C) ||defined(CONFIG_RTL_8197F)
	scr = (REG32(MACCR) & 0x00003000) >> 12;
	switch( scr )
	{
		case 0: pauseTicks = 6250000; break;		//lexra clock 50Mhz
		case 1: pauseTicks = 12500000; break;		//lexra clock 100Mhz
		default:pauseTicks = 6250000; break;		//lexra clock 50Mhz		
	}
	#else
	scr = (REG32(SCCR) & 0x00000070) >> 4;
	switch( scr )
	{
		case 0: pauseTicks = 12500000; break;
		case 1: pauseTicks = 25000000; break;
		case 2: pauseTicks = 31250000; break;
		case 3: pauseTicks = 32500000; break;
		case 4: pauseTicks = 33750000; break;
		case 5: pauseTicks = 35000000; break;
		case 6: pauseTicks = 36250000; break;
		case 7: pauseTicks = 37500000; break;
		default:pauseTicks = 25000000; break;
							
	}
	#endif
	/* waiting 500ms */

	pauseTicks = pauseTicks<<2;

	for(i=pauseTicks;i>0;i--)
	{
		i = i;
	}
	
 	WRITE_MEM32(QRR, 0x1);

	for(i=pauseTicks;i>0;i--)
	{
		i = i;
	}
	
	WRITE_MEM32(QRR, 0x0);

 	return SUCCESS;
 }


/*
 *	_rtl8651_syncToAsicEthernetBandwidthControl()
 *
 *	Sync SW bandwidth control () configuration to ASIC:
 *
 *
 *		_rtl865xB_BandwidthCtrlPerPortConfiguration -----> Translate from RTL865xB Index to ACTUAL
 *														 	 token count in RTL865xC
 *																		|
 *																---------
 *																|
 *		_rtl865xB_BandwidthCtrlMultiplier	---- Translate using         ---->*
 *										 RTL865xB's mechanism		|
 *																|
 *											---------------------
 *											|
 *											-- > Actual Token count which need to set to ASIC.
 *												 => Set it to ASIC if value in SW is different from ASIC.
 *
*/
static void _rtl8651_syncToAsicEthernetBandwidthControl(void)
{
	uint32 port;
	uint32 cfgTypeIdx;
	int32 retval;

	for ( port = 0 ; port < RTL8651_PORT_NUMBER ; port ++ )
	{
		for ( cfgTypeIdx = 0 ; cfgTypeIdx < _RTL865XB_BANDWIDTHCTRL_CFGTYPE ; cfgTypeIdx ++ )
		{
			uint32 currentSwBandwidthCtrlBasicSetting;
			uint32 currentSwBandwidthCtrlMultiplier;
			uint32 currentSwBandwidthCtrlSetting;
			uint32 currentAsicBandwidthCtrlSetting;

			/*
				We would check for rate and _rtl865xB_BandwidthCtrlMultiplier for the rate-multiply.

				In RTL865xB, the bits definition is as below.

				SWTECR

				bit 14(x8)		bit 15 (x4)		Result
				=============================================
				0				0				x1
				0				1				x4
				1				0				x8
				1				1				x8
			*/
			if (_rtl865xB_BandwidthCtrlMultiplier & _RTL865XB_BANDWIDTHCTRL_X8)
			{	/* case {b'10, b'11} */
				currentSwBandwidthCtrlMultiplier = 8;
			} else if ( _rtl865xB_BandwidthCtrlMultiplier & _RTL865XB_BANDWIDTHCTRL_X4)
			{	/* case {b'01} */
				currentSwBandwidthCtrlMultiplier = 4;
			} else
			{	/* case {b'00} */
				currentSwBandwidthCtrlMultiplier = 1;
			}

			/* Calculate Current SW configuration : 0 : Full Rate */
			/* Mix BASIC setting and Multiplier -> to get the ACTUAL bandwidth setting */

			currentSwBandwidthCtrlBasicSetting = ((_rtl865xC_BandwidthCtrlNum[_rtl865xB_BandwidthCtrlPerPortConfiguration[port][cfgTypeIdx]])*(currentSwBandwidthCtrlMultiplier));
			currentSwBandwidthCtrlSetting = (cfgTypeIdx == 0)?
					/* Ingress */
					(((currentSwBandwidthCtrlBasicSetting%RTL865XC_INGRESS_16KUNIT)<(RTL865XC_INGRESS_16KUNIT>>1))?(currentSwBandwidthCtrlBasicSetting/RTL865XC_INGRESS_16KUNIT):((currentSwBandwidthCtrlBasicSetting/RTL865XC_INGRESS_16KUNIT)+1)):
					/* Egress */
					(((currentSwBandwidthCtrlBasicSetting%RTL865XC_EGRESS_64KUNIT)<(RTL865XC_EGRESS_64KUNIT>>1))?(currentSwBandwidthCtrlBasicSetting/RTL865XC_EGRESS_64KUNIT):((currentSwBandwidthCtrlBasicSetting/RTL865XC_EGRESS_64KUNIT)+1));

			/* Get Current ASIC configuration */
			retval = (cfgTypeIdx == 0)?
					/* Ingress */
					(rtl8651_getAsicPortIngressBandwidth(	port,
															&currentAsicBandwidthCtrlSetting)):
					/* Egress */
					(rtl8651_getAsicPortEgressBandwidth(		port,
															&currentAsicBandwidthCtrlSetting));

			if ( retval != SUCCESS )
			{
				assert(0);
				goto out;
			}
			#if defined(CONFIG_RTL_8197F)
				if((!( (currentSwBandwidthCtrlSetting) == 0 && (currentAsicBandwidthCtrlSetting == 0xfffff) ) /* for FULL Rate case */) ||
				( currentSwBandwidthCtrlSetting != currentAsicBandwidthCtrlSetting ))
				{
					retval = (cfgTypeIdx == 0)?
					/* Ingress */
					(rtl8651_setAsicPortIngressBandwidth(	port,
															(currentSwBandwidthCtrlSetting == 0)?
																(0	/* For Ingress Bandwidth control, 0 means "disabled" */):
																(currentSwBandwidthCtrlSetting))		):
					/* Egress */
					(rtl8651_setAsicPortEgressBandwidth(		port,
															(currentSwBandwidthCtrlSetting == 0)?
																(0xfffff	/* For Egress Bandwidth control, 0xfffff means "disabled" */):
																(currentSwBandwidthCtrlSetting))		);
					if ( retval != SUCCESS )
					{
						assert(0);
						goto out;
					}
				}
			#else
			/* SYNC configuration to HW if the configuration is different */
			if (	(!( (currentSwBandwidthCtrlSetting) == 0 && (currentAsicBandwidthCtrlSetting == 0x3fff) ) /* for FULL Rate case */) ||
				( currentSwBandwidthCtrlSetting != currentAsicBandwidthCtrlSetting ))
			{
#if 0
				if (cfgTypeIdx==0)
				{
					rtlglue_printf("set ingress bandwidth port %d, %d.\n", port, (currentSwBandwidthCtrlSetting == 0)?
																(0	/* For Ingress Bandwidth control, 0 means "disabled" */):
																(currentSwBandwidthCtrlSetting));
				}
				else
				{
					rtlglue_printf("set ingress bandwidth port %d, %d.\n", port, (currentSwBandwidthCtrlSetting == 0)?
																(0x3fff	/* For Egress Bandwidth control, 0x3fff means "disabled" */):
																(currentSwBandwidthCtrlSetting));
				}
#endif
				retval = (cfgTypeIdx == 0)?
					/* Ingress */
					(rtl8651_setAsicPortIngressBandwidth(	port,
															(currentSwBandwidthCtrlSetting == 0)?
																(0	/* For Ingress Bandwidth control, 0 means "disabled" */):
																(currentSwBandwidthCtrlSetting))		):
					/* Egress */
					(rtl8651_setAsicPortEgressBandwidth(		port,
															(currentSwBandwidthCtrlSetting == 0)?
																(0x3fff	/* For Egress Bandwidth control, 0x3fff means "disabled" */):
																(currentSwBandwidthCtrlSetting))		);

				if ( retval != SUCCESS )
				{
					assert(0);
					goto out;
				}
			}
			#endif
		}
	}
out:
	return;
}


/*
@func int32 | rtl8651_setAsicEthernetBandwidthControl | set ASIC per-port total ingress bandwidth
@parm uint32 | port | the port number
@parm int8 | input | Ingress or egress control to <p port>
@parm uint32 | rate | rate to set.
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
The <p rate> can be set to several different values:
BW_FULL_RATE
BW_128K
BW_256K
BW_512K
BW_1M
BW_2M
BW_4M
BW_8M

Note: This function is backward compatible to RTL865xB.
 */
int32 rtl8651_setAsicEthernetBandwidthControl(uint32 port, int8 input, uint32 rate)
{
	uint32 *currentConfig_p;

	if ( port >= RTL8651_PORT_NUMBER )
	{
		goto err;
	}

	switch ( rate )
	{
		case BW_FULL_RATE:
		case BW_128K:
		case BW_256K:
		case BW_512K:
		case BW_1M:
		case BW_2M:
		case BW_4M:
		case BW_8M:
			break;
		default:
			goto err;
	}

	currentConfig_p = &(_rtl865xB_BandwidthCtrlPerPortConfiguration[port][(input)?0 /* Ingress */:1 /* Egress */]);

	/* We just need to re-config HW when it's updated */
	if ( *currentConfig_p != rate )
	{
		/* Update configuration table */
		*currentConfig_p = rate;

		/* sync the configuration to ASIC */
		_rtl8651_syncToAsicEthernetBandwidthControl();
	}

	return SUCCESS;
err:
	return FAILED;
}


int32 rtl8651_setAsicFlowControlRegister(uint32 port, uint32 enable)
{
	uint32 phyid, statCtrlReg4;

	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}
	/* phy id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* Read */
	rtl8651_getAsicEthernetPHYReg( phyid, 4, &statCtrlReg4 );

	if ( enable && ( statCtrlReg4 & CAPABLE_PAUSE ) == 0 )
	{
		statCtrlReg4 |= CAPABLE_PAUSE;
	}
	else if ( enable == 0 && ( statCtrlReg4 & CAPABLE_PAUSE ) )
	{
		statCtrlReg4 &= ~CAPABLE_PAUSE;
	}
	else
		return SUCCESS;	/* The configuration does not change. Do nothing. */

	rtl8651_setAsicEthernetPHYReg( phyid, 4, statCtrlReg4 );

	/* restart N-way. */
	rtl8651_restartAsicEthernetPHYNway(port);

	return SUCCESS;
}

int32 rtl8651_getAsicFlowControlRegister(uint32 port, uint32 *enable)
{
	uint32 phyid, statCtrlReg4;

	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}
	/* phy id determination */
	phyid = rtl8651AsicEthernetTable[port].phyId;

	/* Read */
	rtl8651_getAsicEthernetPHYReg( phyid, 4, &statCtrlReg4 );

	*enable = ( statCtrlReg4 & CAPABLE_PAUSE )? TRUE: FALSE;

	return SUCCESS;
}

/*
@func int32 | rtl8651_setAsicSystemInputFlowControlRegister | Set System input queue flow control register
@parm uint32 | fcON		| Threshold for Flow control OFF
@parm uint32 | fcOFF		| Threshold for Flow control ON
@rvalue SUCCESS |
@comm
Set input-queue flow control threshold on RTL865xC platform.
 */
int32 rtl8651_setAsicSystemInputFlowControlRegister(uint32 fcON, uint32 fcOFF)
{
	/* Check the correctness */
	if (	(fcON > ( IQ_DSC_FCON_MASK >> IQ_DSC_FCON_OFFSET )) ||
	   	(fcOFF > ( IQ_DSC_FCOFF_MASK >> IQ_DSC_FCOFF_OFFSET ))	)
	{
		return FAILED;
	}

	/* Write the flow control threshold value into ASIC */
	WRITE_MEM32(	IQFCTCR,
			(	(READ_MEM32(IQFCTCR) & ~(IQ_DSC_FCON_MASK | IQ_DSC_FCOFF_MASK)) |
				(fcON << IQ_DSC_FCON_OFFSET) |
				(fcOFF << IQ_DSC_FCOFF_OFFSET))	);
	return SUCCESS;
}
/*
@func int32 | rtl8651_getAsicSystemInputFlowControlRegister | Get System input queue flow control register
@parm uint32* | fcON		| pointer to get Threshold for Flow control OFF
@parm uint32* | fcOFF		| pointer to get Threshold for Flow control ON
@rvalue SUCCESS |
@comm
Set input-queue flow control threshold on RTL865xC platform.
 */
int32 rtl8651_getAsicSystemInputFlowControlRegister(uint32 *fcON, uint32 *fcOFF)
{
	uint32 iqfctcr;

	iqfctcr = READ_MEM32( IQFCTCR );

	if ( fcON )
	{
		*fcON = ( iqfctcr & IQ_DSC_FCON_MASK ) >> IQ_DSC_FCON_OFFSET;
	}

	if ( fcOFF )
	{
		*fcOFF = ( iqfctcr & IQ_DSC_FCOFF_MASK ) >> IQ_DSC_FCOFF_OFFSET;
	}

	return SUCCESS;
}


int32 rtl865xC_setAsicEthernetForceModeRegs(uint32 port, uint32 enForceMode, uint32 forceLink, uint32 forceSpeed, uint32 forceDuplex)
{
	uint32 offset = port * 4;
	uint32 PCR = READ_MEM32( PCRP0 + offset );

	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
	{
		if ( port > RTL8651_MAC_NUMBER )
		{
			return FAILED;
		}
	} else
	{
		if ( port > RTL8651_PHY_NUMBER )
		{
			return FAILED;
		}
	}

#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
	if(gpio_simulate_mdc_mdio && (rtl8651AsicEthernetTable[port].isGPHY == TRUE)){
		PCR &= ~ForceSpeedMask;
		PCR &= ~ForceDuplex;
		
		if ( forceSpeed == 2 )
			PCR |= ForceSpeed1000M;
		else if ( forceSpeed == 1 )
			PCR |= ForceSpeed100M;
		else
			PCR |= ForceSpeed10M;

		if ( forceDuplex )
			PCR |= ForceDuplex;

		WRITE_MEM32( PCRP0 + offset, PCR );
		mdelay(10);

		return SUCCESS;
	}
#endif

	PCR &= ~EnForceMode;
	PCR &= ~ForceLink;
	PCR &= ~ForceSpeedMask;
	PCR &= ~ForceDuplex;

	if ( enForceMode )
	{
		PCR |= EnForceMode;

                //ForceMode with polling link status, disable Auto-Negotiation but polling phy's link status
                PCR |= PollLinkStatus;

		if ( forceLink )
			PCR |= ForceLink;

		if ( forceSpeed == 2 )
			PCR |= ForceSpeed1000M;
		else if ( forceSpeed == 1 )
			PCR |= ForceSpeed100M;
		else
			PCR |= ForceSpeed10M;

		if ( forceDuplex )
			PCR |= ForceDuplex;
	}
	else {
		PCR = (PCR & ~(PollLinkStatus)) | (ForceLink | AutoNegoSts_MASK);

#ifdef CONFIG_RTL_8198C
		/* user set force_1000M mode, we set auto-negotiation mode but disable 10/100M ability */
		if (forceSpeed == 2 /*SPEED1000M*/) {
			PCR = (PCR & ~(AutoNegoSts_MASK)) | (NwayAbility1000MF);
		}
#endif
	}

	WRITE_MEM32( PCRP0 + offset, PCR );
	mdelay(10);
	return SUCCESS;

}


void force100m_phy_diag(uint32 portmask, uint8 mdimode, bool recover)
{
	int i, phyid=0;
	unsigned int data;
	unsigned long flags=0;	

	SMP_LOCK_ETH(flags);
	for(i=0; i<5; i++) {
		if ((1<<i) & portmask) {
			
			if ((REG32(PCRP0+i*4)&EnForceMode) == 0) {
				/*Set PHY Register*/					
				rtl8651_setAsicEthernetPHYSpeed(i,1); //link speed=100M
				rtl8651_setAsicEthernetPHYDuplex(i,0); // half duplex
				rtl8651_setAsicEthernetPHYAutoNeg(i,0); // Forcemode
				rtl8651_setAsicEthernetPHYAdvCapality(i,(1<<DUPLEX_100M));	
				rtl865xC_setAsicEthernetForceModeRegs(i, 1, 1, 1, 0);

				//phy restart Nway
				rtl8651_restartAsicEthernetPHYNway(i);	
			}
		}
	}

	// to get the 100M SNR value
	phy_diag(mdimode, portmask);

	if (recover) {
		for(i=0; i<5; i++) {
			if ((1<<i) & portmask) {			
				/*Set PHY Register Back to AN mode*/				
				rtl8651_setAsicEthernetPHYAutoNeg(i,TRUE);
				rtl8651_setAsicEthernetPHYAdvCapality(i,(1<<PORT_AUTO));
				REG32(PCRP0 + (i * 4)) = (REG32(PCRP0 + (i * 4)) & ~(EnForceMode | PollLinkStatus | 
							AutoNegoSts_MASK)) | NwayAbility10MF | NwayAbility10MH | 
							NwayAbility100MF | NwayAbility100MH |NwayAbility1000MF;

				//port phyid
				phyid = rtl8651AsicEthernetTable[i].phyId;

				//port i auto MDI/MDIX mode
				rtl8651_getAsicEthernetPHYReg( phyid, 28, &data);
				data |= (0x3<<1);
				rtl8651_setAsicEthernetPHYReg( phyid, 28, data);
				
				rtl8651_restartAsicEthernetPHYNway(i);
			}
		}
	}
	SMP_UNLOCK_ETH(flags);

}

void force_phy_linkup(uint32 portmask, bool forcelink)
{
	int i, phyid;
	unsigned int data;
	unsigned long flags=0;	

	SMP_LOCK_ETH(flags);
	for(i=0; i<5; i++) {
		if ((1<<i) & portmask) {
			phyid = rtl8651AsicEthernetTable[i].phyId;
			
			rtl8651_getAsicEthernetPHYReg(phyid, 23, &data);
			if(forcelink)
				data |= (1<<15);
			else
				data &= ~(1<<15);
				
			rtl8651_setAsicEthernetPHYReg(phyid, 23, data);

			if(forcelink)
				printk("port %d PHY is force link up\n", i);
			else
				printk("port %d is not PHY force link up\n", i);
		}
	}
	SMP_UNLOCK_ETH(flags);
}

#define	MULTICAST_STORM_CONTROL	1
#define	BROADCAST_STORM_CONTROL	2
#define RTL865XC_MAXALLOWED_BYTECOUNT	30360	/* Used for BSCR in RTL865xC. Means max allowable byte count for 10Mbps port */
#if defined(CONFIG_RTL_8197F)
#define UNKNOWN_UNICAST_STORM_CONTROL 1
#define UNKNOWN_MULTICAST_SOTRM_CONTROL 2
#define ARP_STORM_CONTROL 4
#define DHCP_STORM_CONTROL 8
#define IGMP_MLD_STORM_CONTROL 16
#define UNKOWN_UNICAST_COUNTER 0
#define UNKOWN_MULTICAST_COUNTER 1
#define ARP_COUNTER 2
#define DHCP_COUNTER 3
#define IGMP_MLD_COUNTER 3
#endif
static int32 rtl865xC_setBrdcstStormCtrlRate(uint32 percentage)
{
	uint32 rate = RTL865XC_MAXALLOWED_BYTECOUNT * percentage / 100;

	WRITE_MEM32( BSCR, rate );
	return SUCCESS;
}


static int32 rtl8651_perPortStormControl(uint32 type, uint32 portNum, uint32 enable)
{
	uint32 regAddress;
	uint32 oldRegValue;
	uint32 newRegValue;
	uint32 totalExtPortNum=3;

	if(portNum>=RTL8651_PORT_NUMBER + totalExtPortNum)
	{
		rtlglue_printf("wrong port number\n");
		return FAILED;
	}

	regAddress=PCRP0 + portNum * 4;

	oldRegValue=READ_MEM32(regAddress);

	newRegValue=oldRegValue;

	if((type & BROADCAST_STORM_CONTROL) !=0)
	{
		if(enable == TRUE)
		{
			newRegValue = newRegValue |ENBCSC |BCSC_ENBROADCAST;
		}

		if(enable==FALSE)
		{
			newRegValue = newRegValue & (~BCSC_ENBROADCAST);
		}

	}

	if((type & MULTICAST_STORM_CONTROL) !=0)
	{
		if(enable == TRUE)
		{
			newRegValue = newRegValue | ENBCSC |BCSC_ENMULTICAST;
		}

		if(enable==FALSE)
		{
			newRegValue = newRegValue & (~BCSC_ENMULTICAST);
		}

	}

	if((newRegValue & (BCSC_ENMULTICAST |BCSC_ENBROADCAST ))==0)
	{
		/*no needn't storm control*/
		newRegValue = newRegValue & (~ENBCSC);
	}

	if(newRegValue!=oldRegValue)
	{
		WRITE_MEM32(regAddress, newRegValue);
	}

	TOGGLE_BIT_IN_REG_TWICE(regAddress,EnForceMode);

	return SUCCESS;


}

int32 rtl865x_setStormControl(uint32 type,uint32 enable,uint32 percentage)
{
	uint32 port;
	uint32 totalExtPortNum=3;
	for ( port = 0; port < RTL8651_PORT_NUMBER + totalExtPortNum; port++ )
	{
		if(enable==TRUE)
		{
			/*speed unit Mbps*/
			if(percentage>100)
			{
				rtl865xC_setBrdcstStormCtrlRate(100);
			}
			else
			{
				rtl865xC_setBrdcstStormCtrlRate(percentage);
			}

			rtl8651_perPortStormControl(type, port, TRUE);
		}
		else
		{
			rtl865xC_setBrdcstStormCtrlRate(100);
			rtl8651_perPortStormControl(type, port, FALSE);

		}

	}

	return SUCCESS;
}

#if defined(CONFIG_RTL_8197F)
static int32 rtl865x_setExtendStormCtrlRate(uint32 type, uint32 percentage)
{
	uint32 counter;
	uint32 port;
	uint32 regData, data0;
	uint32 minSpeed = PortStatusLinkSpeedReserved;
	uint32 rate = RTL865XC_MAXALLOWED_BYTECOUNT * percentage / 100 / 4;		/* counter unit 4 bytes*/

	for(port=0; port<RTL8651_PORT_NUMBER; port++){
		regData = READ_MEM32(PSRP0+((port)<<2));
		data0 = regData & PortStatusLinkUp;
		if (data0){
			data0 = (regData&PortStatusLinkSpeed_MASK)>>PortStatusLinkSpeed_OFFSET;
			if(data0 < minSpeed)
				minSpeed = data0;
		}
		else
			continue;
	}

	/* counter timing unit & rate */
	regData = READ_MEM32(EXTSTMCR3);
	switch(type){
		case UNKNOWN_UNICAST_STORM_CONTROL:
			counter = 0;
			regData |= (minSpeed<<TUSSCM0_OFFSET);
			WRITE_MEM32(EXTSTMCR4, rate);
			break;
		case UNKNOWN_MULTICAST_SOTRM_CONTROL:
			counter = 1;
			regData |= (minSpeed<<TUSSCM1_OFFSET);
			WRITE_MEM32(EXTSTMCR4, rate<<16);
			break;
		case ARP_STORM_CONTROL:
			counter = 2;
			regData |= (minSpeed<<TUSSCM2_OFFSET);
			WRITE_MEM32(EXTSTMCR5, rate);
			break;
		case DHCP_STORM_CONTROL:
			counter = 3;
			regData |= (minSpeed<<TUSSCM3_OFFSET);
			WRITE_MEM32(EXTSTMCR5, rate<<16);
			break;
		case IGMP_MLD_STORM_CONTROL:
			counter = 3;
			regData |= (minSpeed<<TUSSCM3_OFFSET);
			WRITE_MEM32(EXTSTMCR5, rate<<16);
			break;
		default:
			counter = 0;
			break;
	}
	
	WRITE_MEM32(EXTSTMCR3, regData);
	return SUCCESS;
}
static int32 rtl865x_setPerPortExtendStormControl(uint32 type, uint32 port,uint32 enable)
{
	uint32 regAddress;
	uint32 oldRegValue;
	uint32 newRegValue;
	uint32 enCounterDrop = 0;
	uint32 counter = 0;
	
	if(port >= RTL8651_PORT_NUMBER)
	{
		rtlglue_printf("wrong port number\n");
		return FAILED;
	}

	regAddress=EXTSTMCR0 + (port>>1)*4;
	oldRegValue=READ_MEM32(regAddress);
	newRegValue=oldRegValue;

	enCounterDrop = READ_MEM32(EXTSTMCR3);

	type = (port & 0x1)? (type << OPSCS_OFFSET): type;
	
	if(enable==TRUE){
		switch(type){
			case UNKNOWN_UNICAST_STORM_CONTROL:
				enCounterDrop |= ENDROPSC0;
				counter = 0;
				break;
			case UNKNOWN_MULTICAST_SOTRM_CONTROL:
				enCounterDrop |= ENDROPSC1;
				counter = 1;
				break;
			case ARP_STORM_CONTROL:
				enCounterDrop |= ENDROPSC2;
				counter = 2;
				break;
			case DHCP_STORM_CONTROL:
				enCounterDrop |= ENDROPSC3;
				counter = 3;
				break;
			case IGMP_MLD_STORM_CONTROL:
				enCounterDrop |= ENDROPSC3;
				counter = 3;
				break;
			default:
				break;
		}

		counter = (port & 0x1)? (counter << OPUUSC_OFFSET): (counter << EPUUSC_OFFSET);
		newRegValue = newRegValue | type | counter;
		
	}
	else{
		switch(type){
			case UNKNOWN_UNICAST_STORM_CONTROL:
				enCounterDrop &= ~ENDROPSC0;
				counter = 0;
				break;
			case UNKNOWN_MULTICAST_SOTRM_CONTROL:
				enCounterDrop &= ~ENDROPSC1;
				counter = 1;
				break;
			case ARP_STORM_CONTROL:
				enCounterDrop &= ~ENDROPSC2;
				counter = 2;
				break;
			case DHCP_STORM_CONTROL:
				enCounterDrop &= ~ENDROPSC3;
				counter = 3;
				break;
			case IGMP_MLD_STORM_CONTROL:
				enCounterDrop &= ~ENDROPSC3;
				counter = 3;
				break;
			default:
				break;
		}

		counter = (port & 0x1)? (counter << OPUUSC_OFFSET): (counter << EPUUSC_OFFSET);
		newRegValue = newRegValue & ~type & ~counter;
	}

	WRITE_MEM32(EXTSTMCR3, enCounterDrop);	

	if(newRegValue!=oldRegValue)
	{
		WRITE_MEM32(regAddress, newRegValue);
	}
	
	return SUCCESS;
}
int32 rtl865x_setExtendStormControl(uint32 type,uint32 enable,uint32 percentage)
{
	uint32 port;
	
	if(enable==TRUE)
	{
		/*speed unit Mbps*/
		if(percentage>100)
		{
			rtl865x_setExtendStormCtrlRate(type, 100);
		}
		else
		{
			rtl865x_setExtendStormCtrlRate(type, percentage);
		}

		for(port=0; port<RTL8651_PORT_NUMBER; port++)
			rtl865x_setPerPortExtendStormControl(type, port, TRUE);
	}
	else
	{
		rtl865x_setExtendStormCtrlRate(type, 100);
		
		for(port=0; port<RTL8651_PORT_NUMBER; port++)
			rtl865x_setPerPortExtendStormControl(type, port, FALSE);

	}

	return SUCCESS;
}

#endif


/*
@func int32 | rtl8651_setAsic802D1xMacBaseAbility | set 802.1x mac based ability
@parm enum PORTID | port | the port number (physical port: 0~5, extension port: 6~8)
@parm uint32* | isEnable | 1: enabled, 0: disabled.
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsic802D1xMacBaseAbility( enum PORTID port, uint32 isEnable )
{
	/* Invalid input parameter */
	if ((port < PHY0) || (port > EXT2))
		return FAILED;

	/* Invalid input parameter */
	if ((isEnable != TRUE) && (isEnable != FALSE))
		return FAILED;

	switch (port)
	{
		case PHY0:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P0En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P0En)));
			break;

		case PHY1:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P1En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P1En)));
			break;

		case PHY2:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P2En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P2En)));
			break;

		case PHY3:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P3En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P3En)));
			break;

		case PHY4:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P4En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P4En)));
			break;

		case PHY5:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P5En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P5En)));
			break;

		case CPU:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P6En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P6En)));
			break;

		case EXT1:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P7En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P7En)));
			break;

		case EXT2:
		WRITE_MEM32(DOT1XMACCR, isEnable == TRUE ? (READ_MEM32(DOT1XMACCR) | ( Dot1xMAC_P8En)):
											(READ_MEM32(DOT1XMACCR) & ~( Dot1xMAC_P8En)));
			break;

		case EXT3:
			return FAILED;

		case MULTEXT:
			return FAILED;

	}
	return SUCCESS;
}

/*
@func int32 | rtl8651_setAsic802D1xMacBaseDirection | set 802.1x mac based direction
@parm enum uint32 | dir | OperCOnntrolledDirections for MAC-Based ACCESS Control. 0:BOTH, 1:IN
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsic802D1xMacBaseDirection(int32 dir)
{


	/* Invalid input parameter */
	if ((dir != Dot1xMAC_OPDIR_IN) && (dir != Dot1xMAC_OPDIR_BOTH))
	{
		return FAILED;
	}

	if(dir == Dot1xMAC_OPDIR_IN)
	{
		WRITE_MEM32(DOT1XMACCR,(READ_MEM32(DOT1XMACCR)) |Dot1xMAC_OPDIR);
	}
	else
	{
		WRITE_MEM32(DOT1XMACCR,(READ_MEM32(DOT1XMACCR)) &(~Dot1xMAC_OPDIR));
	}
	return SUCCESS;
}

/*
@func int32 | rtl8651_setAsicGuestVlanProcessControl | set guest vlan process control
@parm enum  uint32 | process |default process for unauthenticated client  (00~11)<<12
@rvalue SUCCESS
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicGuestVlanProcessControl( uint32 process)
{
	/* Invalid input parameter */
	if((process < Dot1xUNAUTHBH_DROP) || (process > Dot1xUNAUTHBH_RESERVED))
		return FAILED;

	WRITE_MEM32(GVGCR, ((READ_MEM32(GVGCR)) & 0x0fff) | process);

	return SUCCESS;
}


/*
@func int32 | rtl8651_setAsicDot1qAbsolutelyPriority | set 802.1Q absolutely priority
@parm enum PRIORITYVALUE | srcpriority | priority value
@parm enum PRIORITYVALUE | priority | absolute priority value
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicDot1qAbsolutelyPriority( enum PRIORITYVALUE srcpriority, enum PRIORITYVALUE priority )
{
	/* Invalid input parameter */
	if ((srcpriority < PRI0) || (srcpriority > PRI7) || (priority < PRI0) || (priority > PRI7))
		return FAILED;

	switch (srcpriority)
	{
		case PRI0:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI0_MASK)) | (priority << EN_8021Q2LTMPRI0)); break;
		case PRI1:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI1_MASK)) | (priority << EN_8021Q2LTMPRI1)); break;
		case PRI2:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI2_MASK)) | (priority << EN_8021Q2LTMPRI2)); break;
		case PRI3:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI3_MASK)) | (priority << EN_8021Q2LTMPRI3)); break;
		case PRI4:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI4_MASK)) | (priority << EN_8021Q2LTMPRI4)); break;
		case PRI5:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI5_MASK)) | (priority << EN_8021Q2LTMPRI5)); break;
		case PRI6:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI6_MASK)) | (priority << EN_8021Q2LTMPRI6)); break;
		case PRI7:
			WRITE_MEM32(LPTM8021Q, (READ_MEM32(LPTM8021Q) & ~(EN_8021Q2LTMPRI7_MASK)) | (priority << EN_8021Q2LTMPRI7)); break;
	}

	return SUCCESS;
}


/*
@func int32 | rtl8651_getAsicDot1qAbsolutelyPriority | get 802.1Q absolutely priority
@parm enum PRIORITYVALUE | srcpriority | priority value
@parm enum PRIORITYVALUE* | pPriority | pPriority will return the absolute priority value
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicDot1qAbsolutelyPriority( enum PRIORITYVALUE srcpriority, enum PRIORITYVALUE *pPriority )
{

	/* Invalid input parameter */
	if ((srcpriority < PRI0) || (srcpriority > PRI7))
		return FAILED;

	if (pPriority != NULL)
	{
		switch (srcpriority)
		{
			case PRI0:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI0_MASK) >> EN_8021Q2LTMPRI0;  break;
			case PRI1:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI1_MASK) >> EN_8021Q2LTMPRI1;  break;
			case PRI2:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI2_MASK) >> EN_8021Q2LTMPRI2;  break;
			case PRI3:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI3_MASK) >> EN_8021Q2LTMPRI3;  break;
			case PRI4:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI4_MASK) >> EN_8021Q2LTMPRI4;  break;
			case PRI5:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI5_MASK) >> EN_8021Q2LTMPRI5;  break;
			case PRI6:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI6_MASK) >> EN_8021Q2LTMPRI6;  break;
			case PRI7:
				*pPriority = (READ_MEM32(LPTM8021Q) & EN_8021Q2LTMPRI7_MASK) >> EN_8021Q2LTMPRI7;  break;
		}
	}

	return SUCCESS;
}


#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)

/*
@func int32 | rtl8651_flushAsicDot1qAbsolutelyPriority | set 802.1Q absolutely priority the default value 0
@parm void
@rvalue SUCCESS |
@comm
 */
int32 rtl8651_flushAsicDot1qAbsolutelyPriority(void)
{
	WRITE_MEM32(LPTM8021Q, 0);
	return SUCCESS;
}
#else
int32 rtl851_setDefaultAsicDot1qAbsolutelyPriority(void)
{
	//default 802.1p -> queue prio: 0->0, 1->1, 2->2...
	WRITE_MEM32(LPTM8021Q, 0xFAC688);
	return SUCCESS;
}
#endif

#ifdef CONFIG_RTL_LINKCHG_PROCESS
/*
@func int32 | rtl8651_updateAsicLinkAggregatorLMPR | Arrange the table which maps hashed index to port.
@parm	uint32	|	portMask |  Specify the port mask for the aggregator.
@rvalue SUCCESS | Update the mapping table successfully.
@rvalue FAILED | When the port mask is invalid, return FAILED
@comm
RTL865x provides an aggregator port. This API updates the table which maps hashed index to port.
If portmask = 0: clear all aggregation port mappings.
Rearrange policy is round-robin. ie. if port a,b,c is in portmask, then hash block 0~7's port number is a,b,c,a,b,c,a,b
*/
int32  rtl8651_updateAsicLinkAggregatorLMPR(int32 portmask)
{
	uint32 hIdx, portIdx, reg;

	/* Clear all mappings */
	WRITE_MEM32( LAGHPMR0, 0 );

	if ( portmask == 0 )
	{
		return SUCCESS;
	}

	reg = 0;
	portIdx = 0;
	for ( hIdx = 0; hIdx < RTL865XC_LAGHASHIDX_NUMBER; hIdx++ )
	{
		while ( ( ( 1 << portIdx ) & portmask ) == 0 )	/* Don't worry about infinite loop because portmask won't be 0. */
		{
			portIdx = ( portIdx + 1 ) % ( RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum );
		}

		reg |= ( portIdx << ( hIdx * LAG_HASHIDX_BITNUM ) );
		portIdx = ( portIdx + 1 ) % ( RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum );
	}

	WRITE_MEM32( LAGHPMR0, reg );
	return SUCCESS;
}



#if 0
static uint32 _rtl8651_findAsicLinkupPortmask(uint32 portMask)
{
	uint32 port, lnkUp_portMask = portMask;
	for ( port = 0; port < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; port++ )
	{
		if ( ( portMask & ( 1 << port ) ) && (rtl8651AsicEthernetTable[port].linkUp == FALSE) )
		{
			lnkUp_portMask &= ~( 1 << port );
		}
	}
	return lnkUp_portMask;
}
#endif

int32 rtl8651_setAsicEthernetLinkStatus(uint32 port, int8 linkUp)
{
	int8 notify;
//	uint32 portmask;

	if (port >= (RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum))
	{
		return FAILED;
	}

	notify = (rtl8651AsicEthernetTable[port].linkUp != ((linkUp==TRUE)? TRUE: FALSE))?TRUE:FALSE;


	rtl8651AsicEthernetTable[port].linkUp = (linkUp == TRUE)? TRUE: FALSE;

#if 0
	/*
		If the specified port is a member of the aggregator,
		update the table which maps hashed index to the port
		because the member port of the aggregator link changes.
	*/
	portmask = READ_MEM32( LAGCR0 ) & TRUNKMASK_MASK;
	if ( portmask & ( 1 << port ) )
	{
		/* Find the link-up portmask */
		uint32 lnkUp_portMask = _rtl8651_findAsicLinkupPortmask(portmask);
		rtl8651_updateAsicLinkAggregatorLMPR( lnkUp_portMask );
	}
#endif

	return SUCCESS;
}
#endif

#if defined (CONFIG_RTL_ENABLE_RATELIMIT_TABLE)
int32 rtl8651_setAsicRateLimitTable(uint32 index, rtl865x_tblAsicDrv_rateLimitParam_t *rateLimit_t)
{
	rtl8651_tblAsic_rateLimitTable_t entry;

	if (rateLimit_t == NULL || index >= RTL8651_RATELIMITTBL_SIZE)
		return FAILED;
	memset(&entry,0,sizeof(rtl8651_tblAsic_rateLimitTable_t));
	entry.maxToken				= rateLimit_t->maxToken&0xFFFFFF;
	entry.refill				= rateLimit_t->refill_number&0xFFFFFF;
	entry.refillTime			= rateLimit_t->t_intervalUnit&0x3F;
	entry.refillRemainTime		= rateLimit_t->t_remainUnit&0x3F;
	entry.token					= rateLimit_t->token&0xFFFFFF;
	return _rtl8651_forceAddAsicEntry(TYPE_RATE_LIMIT_TABLE, index, &entry);
}


int32 rtl8651_delAsicRateLimitTable(uint32 index)
{
	rtl8651_tblAsic_rateLimitTable_t entry;

	if (index >= RTL8651_RATELIMITTBL_SIZE)
		return FAILED;
	memset(&entry,0,sizeof(rtl8651_tblAsic_rateLimitTable_t));
	return _rtl8651_forceAddAsicEntry(TYPE_RATE_LIMIT_TABLE, index, &entry);
}


int32 rtl8651_getAsicRateLimitTable(uint32 index, rtl865x_tblAsicDrv_rateLimitParam_t *rateLimit_t)
{
	rtl8651_tblAsic_rateLimitTable_t entry;

	if (rateLimit_t == NULL || index >= RTL8651_RATELIMITTBL_SIZE)
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_RATE_LIMIT_TABLE, index, &entry);
	if (entry.refillTime == 0)
		return FAILED;
	rateLimit_t->token			= entry.token & 0xFFFFFF;
	rateLimit_t->maxToken		= entry.maxToken & 0xFFFFFF;
	rateLimit_t->t_remainUnit = entry.refillRemainTime&0x3F;
	rateLimit_t->t_intervalUnit = entry.refillTime&0x3F;
	rateLimit_t->refill_number	= entry.refill&0xFFFFFF;
	return SUCCESS;
}
#endif


int32 rtl8651_setPortFlowControlConfigureRegister(enum PORTID port,uint32 enable)
{
	int enable_port = 0;
	if(enable)
		enable_port = 0x3f;
		
	switch (port)
	{
		case PHY0:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x3F<<Q_P0_EN_FC_OFFSET)) | (enable_port << Q_P0_EN_FC_OFFSET));  break;
		case PHY1:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x3F<<Q_P1_EN_FC_OFFSET)) | (enable_port << Q_P1_EN_FC_OFFSET));  break;
		case PHY2:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x3F<<Q_P2_EN_FC_OFFSET)) | (enable_port << Q_P2_EN_FC_OFFSET));  break;
		case PHY3:
			WRITE_MEM32(FCCR0, (READ_MEM32(FCCR0) & ~(0x3F<<Q_P3_EN_FC_OFFSET)) | (enable_port << Q_P3_EN_FC_OFFSET));  break;
		case PHY4:
			WRITE_MEM32(FCCR1, (READ_MEM32(FCCR1) & ~(0x3F<<Q_P4_EN_FC_OFFSET)) | (enable_port << Q_P4_EN_FC_OFFSET));  break;
		case PHY5:
			WRITE_MEM32(FCCR1, (READ_MEM32(FCCR1) & ~(0x3F<<Q_P5_EN_FC_OFFSET)) | (enable_port << Q_P5_EN_FC_OFFSET));  break;
		case CPU:
			WRITE_MEM32(FCCR1, (READ_MEM32(FCCR1) & ~(0x1<<Q_P6_EN_FC_OFFSET)) | (enable_port << Q_P6_EN_FC_OFFSET));  break;
		default:
			return FAILED;
	}

	return SUCCESS;
}
/*
@func int32 | rtl8651_setAsicPortPriority | set port based priority
@parm enum PORTID | port | the port number (valid: physical ports(0~5) and extension ports(7~9) )
@parm enum PRIORITYVALUE | priority | priority value.
@rvalue SUCCESS | 
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_setAsicPortPriority( enum PORTID port, enum PRIORITYVALUE priority )
{
	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7))
		return FAILED; 

	switch (port)
	{
		case PHY0:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P0_MASK)) | (priority << PBPRI_P0_OFFSET)); break;
		case PHY1:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P1_MASK)) | (priority << PBPRI_P1_OFFSET)); break;
		case PHY2:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P2_MASK)) | (priority << PBPRI_P2_OFFSET)); break;
		case PHY3:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P3_MASK)) | (priority << PBPRI_P3_OFFSET)); break;
		case PHY4:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P4_MASK)) | (priority << PBPRI_P4_OFFSET)); break;
		case PHY5:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P5_MASK)) | (priority << PBPRI_P5_OFFSET)); break;
		case EXT1:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P6_MASK)) | (priority << PBPRI_P6_OFFSET)); break;
		case EXT2:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P7_MASK)) | (priority << PBPRI_P7_OFFSET)); break;
		case EXT3:
			WRITE_MEM32(PBPCR, (READ_MEM32(PBPCR) & ~(PBPRI_P8_MASK)) | (priority << PBPRI_P8_OFFSET)); break;
		case CPU: /* fall thru */
		default:
			return FAILED;
	}

	return SUCCESS;
}
/*
@func int32 | rtl8651_getAsicPortPriority | get port based priority
@parm enum PORTID | port | the port number (valid: physical ports(0~5) and extension ports(7~9) )
@parm enum PRIORITYVALUE* | pPriority | pPriority will return the priority of the specified port.
@rvalue SUCCESS |
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8651_getAsicPortPriority( enum PORTID port, enum PRIORITYVALUE *pPriority )
{
        if (pPriority != NULL)
        {
                switch (port)
                {
                        case PHY0:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P0_MASK) >> PBPRI_P0_OFFSET;  break;
                        case PHY1:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P1_MASK) >> PBPRI_P1_OFFSET;  break;
                        case PHY2:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P2_MASK) >> PBPRI_P2_OFFSET;  break;
                        case PHY3:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P3_MASK) >> PBPRI_P3_OFFSET;  break;
                        case PHY4:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P4_MASK) >> PBPRI_P4_OFFSET;  break;
                        case PHY5:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P5_MASK) >> PBPRI_P5_OFFSET;  break;
                        case EXT1:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P6_MASK) >> PBPRI_P6_OFFSET;  break;
                        case EXT2:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P7_MASK) >> PBPRI_P7_OFFSET;  break;
                        case EXT3:
                                *pPriority = (READ_MEM32(PBPCR) & PBPRI_P8_MASK) >> PBPRI_P8_OFFSET;  break;
                        case CPU: /* fall thru */
                        default:
                                return FAILED;
                }
        }

        return SUCCESS;
}

/*
 * @func int32 | rtl8651_setAsicDscpPriority | set DSCP-based priority
 * @parm uint32 | dscp | DSCP value
 * @parm enum PRIORITYVALUE | priority | priority value
 * @rvalue SUCCESS |
 * @rvalue FAILED | invalid parameter
 * @comm
 *  */
int32 rtl8651_setAsicDscpPriority( uint32 dscp, enum PRIORITYVALUE priority )
{
	/* Invalid input parameter */
	if ((dscp < 0) || (dscp > 63))
		return FAILED;
	if ((priority < PRI0) || (priority > PRI7))
		return FAILED;

	if ((0 <= dscp) && (dscp <= 9))
		WRITE_MEM32(DSCPCR0, (READ_MEM32(DSCPCR0) & ~(0x7 << (dscp*3))) | (priority << (dscp*3)));
	else if ((10 <= dscp) && (dscp <= 19))
		WRITE_MEM32(DSCPCR1, (READ_MEM32(DSCPCR1) & ~(0x7 << ((dscp%10)*3))) | (priority << ((dscp%10)*3)));
	else if ((20 <= dscp) && (dscp <= 29))
		WRITE_MEM32(DSCPCR2, (READ_MEM32(DSCPCR2) & ~(0x7 << ((dscp%10)*3))) | (priority << ((dscp%10)*3)));
	else if ((30 <= dscp) && (dscp <= 39))
		WRITE_MEM32(DSCPCR3, (READ_MEM32(DSCPCR3) & ~(0x7 << ((dscp%10)*3))) | (priority << ((dscp%10)*3)));
	else if ((40 <= dscp) && (dscp <= 49))
		WRITE_MEM32(DSCPCR4, (READ_MEM32(DSCPCR4) & ~(0x7 << ((dscp%10)*3))) | (priority << ((dscp%10)*3)));
	else if ((50 <= dscp) && (dscp <= 59))
		WRITE_MEM32(DSCPCR5, (READ_MEM32(DSCPCR5) & ~(0x7 << ((dscp%10)*3))) | (priority << ((dscp%10)*3)));
	else if ((60 <= dscp) && (dscp <= 63))
		WRITE_MEM32(DSCPCR6, (READ_MEM32(DSCPCR6) & ~(0x7 << ((dscp%10)*3))) | (priority << ((dscp%10)*3)));

	return SUCCESS;
}


/*
 * @func int32 | rtl8651_setAsicDscpPriority | set DSCP-based priority
 * @parm uint32 | dscp | DSCP value
 * @parm enum PRIORITYVALUE* | pPriority | pPriority will return the priority of the specified DSCP
 * @rvalue SUCCESS |
 * @rvalue FAILED | invalid parameter
 * @comm
 *  */
int32 rtl8651_getAsicDscpPriority( uint32 dscp, enum PRIORITYVALUE *pPriority )
{
	/* Invalid input parameter */
	if ((dscp < 0) || (dscp > 63))
		return FAILED;

	if (pPriority != NULL)
	{
		if ((0 <= dscp) && (dscp <= 9))
			*pPriority = (READ_MEM32(DSCPCR0) & (0x7 << (dscp*3))) >> (dscp*3);
		else if ((10 <= dscp) && (dscp <= 19))
			*pPriority = (READ_MEM32(DSCPCR1) & (0x7 << ((dscp%10)*3))) >> ((dscp%10)*3);
		else if ((20 <= dscp) && (dscp <= 29))
			*pPriority = (READ_MEM32(DSCPCR2) & (0x7 << ((dscp%10)*3))) >> ((dscp%10)*3);
		else if ((30 <= dscp) && (dscp <= 39))
			*pPriority = (READ_MEM32(DSCPCR3) & (0x7 << ((dscp%10)*3))) >> ((dscp%10)*3);
		else if ((40 <= dscp) && (dscp <= 49))
			*pPriority = (READ_MEM32(DSCPCR4) & (0x7 << ((dscp%10)*3))) >> ((dscp%10)*3);
		else if ((50 <= dscp) && (dscp <= 59))
			*pPriority = (READ_MEM32(DSCPCR5) & (0x7 << ((dscp%10)*3))) >> ((dscp%10)*3);
		else if ((60 <= dscp) && (dscp <= 63))
			*pPriority = (READ_MEM32(DSCPCR6) & (0x7 << ((dscp%10)*3))) >> ((dscp%10)*3);
	}

	return SUCCESS;
}

#ifdef CONFIG_RTK_VOIP_QOS

int32 rtl8651_reset_dscp_priority(void)
{
	//clear dscp priority assignment, otherwise pkt with dscp value 0 will be assign priority 1
	WRITE_MEM32(DSCPCR0,0);
	WRITE_MEM32(DSCPCR1,0);
	WRITE_MEM32(DSCPCR2,0);
	WRITE_MEM32(DSCPCR3,0);
	WRITE_MEM32(DSCPCR4,0);
	WRITE_MEM32(DSCPCR5,0);
	WRITE_MEM32(DSCPCR6,0);
	return 0;
}
int32 rtl8651_cpu_tx_fc(int enable)
{
        if(enable)
                REG32(PSRP6_RW) |= 0x40;
        else
                REG32(PSRP6_RW) &= 0xffffffbf;
        return 0;
}
int32 rtl8651_setQueueNumber(int port, int qnum)
{
        rtl865xC_lockSWCore();
        rtl8651_setAsicOutputQueueNumber(port, qnum);
        rtl865xC_waitForOutputQueueEmpty();
        rtl8651_resetAsicOutputQueue();
        rtl865xC_unLockSWCore();
        return SUCCESS;
}
#endif
#ifdef CONFIG_RTK_VOIP_PORT_LINK
int rtl8651_getAsicEthernetLinkStatus(uint32 port, int8 *linkUp)
{

        int status = READ_MEM32( PSRP0 + port * 4 );

        if(status & PortStatusLinkUp)
                *linkUp = TRUE;
        else
                *linkUp = FALSE;


	return SUCCESS;
}
#endif

#if 1//defined(CONFIG_RTL_LINK_AGGREGATION)
int32 rtl8651_setLAHashIndexToPortMapping( uint32 hash_index, uint32 port_num)
{
	uint32 reg = 0;
	
	if ((hash_index >= RTL865XC_LAGHASHIDX_NUMBER) || (port_num >=RTL8651_AGGREGATOR_NUMBER)){
		return FAILED;
	}

	reg = READ_MEM32(LAGHPMR0);
	reg &= ~(0xf << (hash_index * LAG_HASHIDX_BITNUM)); 
	reg |= (port_num << (hash_index * LAG_HASHIDX_BITNUM));
	//panic_printk("%s %d reg=0x%x port_num=%u hash_index=%u \n", __FUNCTION__, __LINE__, reg, port_num, hash_index);
	WRITE_MEM32(LAGHPMR0, reg);

	return SUCCESS;
}
int32 rtl8651_getLAHashIndexToPortMapping( uint32 hash_index, uint32 *port_num)
{
	uint32 reg = 0;
	
	if ((hash_index >= RTL865XC_LAGHASHIDX_NUMBER) || (!port_num)){
		return FAILED;
	}

	reg = READ_MEM32(LAGHPMR0);
	*port_num = (reg >> (hash_index * LAG_HASHIDX_BITNUM)) & 0xf;
	//panic_printk("%s %d reg=0x%x *port_num=%u hash_index=%u \n", __FUNCTION__, __LINE__, reg, *port_num, hash_index);

	return SUCCESS;
}


int32 rtl8651_setLATrunkMask(uint32 trunk_mask)
{
	uint32 reg = 0;

	reg = READ_MEM32(LAGCR0);
	//set trunk mask
	reg &= ~(TRUNKMASK_MASK);
	reg |= (trunk_mask & TRUNKMASK_MASK);
	
	//panic_printk("%s %d reg=0x%x trunk_mask=0x%x  \n", __FUNCTION__, __LINE__, reg, trunk_mask);
	WRITE_MEM32(LAGCR0, reg);

	return SUCCESS;
}

int32 rtl8651_setLAHashSel(uint32 hash_sel)
{
	uint32 reg = 0;
	
	reg = READ_MEM32(LAGCR0);
	//set hash select
	reg &= ~(LAG_THASH_SELECT_MASK);
	reg |= ((hash_sel & 0x3) << LAG_THASH_SELECT_OFFSET);
	
	//panic_printk("%s %d reg=0x%x hash_sel=%u \n", __FUNCTION__, __LINE__, reg, hash_sel);
	WRITE_MEM32(LAGCR0, reg);

	return SUCCESS;
}
int32 rtl8651_setLAFtkCongest(uint32 ftk_congest)
{
	uint32 reg = 0;

	reg = READ_MEM32(LAGCR0);
	
	//set Force Trunk Congest 
	reg &= ~(LAG_FTK_CONGEST_MASK);
	reg |= ((ftk_congest & 0x1) << LAG_FTK_CONGEST_OFFSET);
	
	//panic_printk("%s %d reg=%u ftk_congest=%u \n", __FUNCTION__, __LINE__, reg, ftk_congest);
	WRITE_MEM32(LAGCR0, reg);

	return SUCCESS;
}

int32 rtl8651_getLATrunkMask(uint32 *trunk_mask)
{
	uint32 reg = 0;

	if (!trunk_mask)
		return FAILED;
	
	reg = READ_MEM32(LAGCR0);
	*trunk_mask = (reg & TRUNKMASK_MASK);
	//panic_printk("%s %d reg=0x%x *trunk_mask=0x%x  \n", __FUNCTION__, __LINE__, reg, *trunk_mask);

	return SUCCESS;
}

int32 rtl8651_getLAHashSel(uint32 *hash_sel)
{
	uint32 reg = 0;

	if (!hash_sel)
		return FAILED;
	
	reg = READ_MEM32(LAGCR0);
	*hash_sel = (reg >> LAG_THASH_SELECT_OFFSET) & 0x3;	
	//panic_printk("%s %d reg=0x%x *hash_sel=%u \n", __FUNCTION__, __LINE__, reg, *hash_sel);

	return SUCCESS;
}
int32 rtl8651_getLAFtkCongest(uint32 *ftk_congest)
{
	uint32 reg = 0;

	if (!ftk_congest)
		return FAILED;
	
	reg = READ_MEM32(LAGCR0);
	*ftk_congest = (reg >> LAG_FTK_CONGEST_OFFSET) & 0x1;
	//panic_printk("%s %d reg=%u *ftk_congest=%u \n", __FUNCTION__, __LINE__, reg, *ftk_congest);

	return SUCCESS;
}

#endif
