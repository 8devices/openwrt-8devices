//============================================================
//
// File Name: Hal8192CDMOutSrc.c
//
// Description:
//
// This file is for 92CE/92CU outsource dynamic mechanism for partner.
//
//
//============================================================

#ifndef _HAL8192CDM_C_
#define _HAL8192CDM_C_

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <asm/unistd.h>

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"



#include <linux/syscalls.h>

#if defined(CONFIG_RTL_819X) && defined(USE_RLX_BSP)
#if defined(CONFIG_OPENWRT_SDK) && !defined(CONFIG_ARCH_CPU_RLX)
#include <bspchip.h>
#else
#include <bsp/bspchip.h>
#endif //CONFIG_OPENWRT_SDK
#endif


//Analog Pre-distortion calibration
#define		APK_BB_REG_NUM	5
#define		APK_AFE_REG_NUM	16
#define		APK_CURVE_REG_NUM 4
#define		PATH_NUM		2

//============================================================
// Global var
//============================================================

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
unsigned int OFDMSwingTable[] = {
	0x7f8001fe, // 0, +6.0dB
	0x788001e2, // 1, +5.5dB
	0x71c001c7, // 2, +5.0dB
	0x6b8001ae, // 3, +4.5dB
	0x65400195, // 4, +4.0dB
	0x5fc0017f, // 5, +3.5dB
	0x5a400169, // 6, +3.0dB
	0x55400155, // 7, +2.5dB
	0x50800142, // 8, +2.0dB
	0x4c000130, // 9, +1.5dB
	0x47c0011f, // 10, +1.0dB
	0x43c0010f, // 11, +0.5dB
	0x40000100, // 12, +0dB
	0x3c8000f2, // 13, -0.5dB
	0x390000e4, // 14, -1.0dB
	0x35c000d7, // 15, -1.5dB
	0x32c000cb, // 16, -2.0dB
	0x300000c0, // 17, -2.5dB
	0x2d4000b5, // 18, -3.0dB
	0x2ac000ab, // 19, -3.5dB
	0x288000a2, // 20, -4.0dB
	0x26000098, // 21, -4.5dB
	0x24000090, // 22, -5.0dB
	0x22000088, // 23, -5.5dB
	0x20000080, // 24, -6.0dB
	0x1e400079, // 25, -6.5dB
	0x1c800072, // 26, -7.0dB
	0x1b00006c, // 27. -7.5dB
	0x19800066, // 28, -8.0dB
	0x18000060, // 29, -8.5dB
	0x16c0005b, // 30, -9.0dB
	0x15800056, // 31, -9.5dB
	0x14400051, // 32, -10.0dB
	0x1300004c, // 33, -10.5dB
	0x12000048, // 34, -11.0dB
	0x11000044, // 35, -11.5dB
	0x10000040, // 36, -12.0dB
};


unsigned int TxPwrTrk_OFDM_SwingTbl[TxPwrTrk_OFDM_SwingTbl_Len] = {
	/*  +6.0dB */ 0x7f8001fe,
	/*  +5.5dB */ 0x788001e2,
	/*  +5.0dB */ 0x71c001c7,
	/*  +4.5dB */ 0x6b8001ae,
	/*  +4.0dB */ 0x65400195,
	/*  +3.5dB */ 0x5fc0017f,
	/*  +3.0dB */ 0x5a400169,
	/*  +2.5dB */ 0x55400155,
	/*  +2.0dB */ 0x50800142,
	/*  +1.5dB */ 0x4c000130,
	/*  +1.0dB */ 0x47c0011f,
	/*  +0.5dB */ 0x43c0010f,
	/*   0.0dB */ 0x40000100,
	/*  -0.5dB */ 0x3c8000f2,
	/*  -1.0dB */ 0x390000e4,
	/*  -1.5dB */ 0x35c000d7,
	/*  -2.0dB */ 0x32c000cb,
	/*  -2.5dB */ 0x300000c0,
	/*  -3.0dB */ 0x2d4000b5,
	/*  -3.5dB */ 0x2ac000ab,
	/*  -4.0dB */ 0x288000a2,
	/*  -4.5dB */ 0x26000098,
	/*  -5.0dB */ 0x24000090,
	/*  -5.5dB */ 0x22000088,
	/*  -6.0dB */ 0x20000080,
	/*  -6.5dB */ 0x1a00006c,
	/*  -7.0dB */ 0x1c800072,
	/*  -7.5dB */ 0x18000060,
	/*  -8.0dB */ 0x19800066,
	/*  -8.5dB */ 0x15800056,
	/*  -9.0dB */ 0x26c0005b,
	/*  -9.5dB */ 0x14400051,
	/* -10.0dB */ 0x24400051,
	/* -10.5dB */ 0x1300004c,
	/* -11.0dB */ 0x12000048,
	/* -11.5dB */ 0x11000044,
	/* -12.0dB */ 0x10000040
};

unsigned char TxPwrTrk_CCK_SwingTbl[TxPwrTrk_CCK_SwingTbl_Len][8] = {
	/*   0.0dB */ {0x36, 0x35, 0x2e, 0x25, 0x1c, 0x12, 0x09, 0x04},
	/*   0.5dB */ {0x33, 0x32, 0x2b, 0x23, 0x1a, 0x11, 0x08, 0x04},
	/*   1.0dB */ {0x30, 0x2f, 0x29, 0x21, 0x19, 0x10, 0x08, 0x03},
	/*   1.5dB */ {0x2d, 0x2d, 0x27, 0x1f, 0x18, 0x0f, 0x08, 0x03},
	/*   2.0dB */ {0x2b, 0x2a, 0x25, 0x1e, 0x16, 0x0e, 0x07, 0x03},
	/*   2.5dB */ {0x28, 0x28, 0x22, 0x1c, 0x15, 0x0d, 0x07, 0x03},
	/*   3.0dB */ {0x26, 0x25, 0x21, 0x1b, 0x14, 0x0d, 0x06, 0x03},
	/*   3.5dB */ {0x24, 0x23, 0x1f, 0x19, 0x13, 0x0c, 0x06, 0x03},
	/*   4.0dB */ {0x22, 0x21, 0x1d, 0x18, 0x11, 0x0b, 0x06, 0x02},
	/*   4.5dB */ {0x20, 0x20, 0x1b, 0x16, 0x11, 0x08, 0x05, 0x02},
	/*   5.0dB */ {0x1f, 0x1e, 0x1a, 0x15, 0x10, 0x0a, 0x05, 0x02},
	/*   5.5dB */ {0x1d, 0x1c, 0x18, 0x14, 0x0f, 0x0a, 0x05, 0x02},
	/*   6.0dB */ {0x1b, 0x1a, 0x17, 0x13, 0x0e, 0x09, 0x04, 0x02},
	/*   6.5dB */ {0x1a, 0x19, 0x16, 0x12, 0x0d, 0x09, 0x04, 0x02},
	/*   7.0dB */ {0x18, 0x17, 0x15, 0x11, 0x0c, 0x08, 0x04, 0x02},
	/*   7.5dB */ {0x17, 0x16, 0x13, 0x10, 0x0c, 0x08, 0x04, 0x02},
	/*   8.0dB */ {0x16, 0x15, 0x12, 0x0f, 0x0b, 0x07, 0x04, 0x01},
	/*   8.5dB */ {0x14, 0x14, 0x11, 0x0e, 0x0b, 0x07, 0x03, 0x02},
	/*   9.0dB */ {0x13, 0x13, 0x10, 0x0d, 0x0a, 0x06, 0x03, 0x01},
	/*   9.5dB */ {0x12, 0x12, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},
	/*  10.0dB */ {0x11, 0x11, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},
	/*  10.5dB */ {0x10, 0x10, 0x0e, 0x0b, 0x08, 0x05, 0x03, 0x01},
	/*  11.0dB */ {0x0f, 0x0f, 0x0d, 0x0b, 0x08, 0x05, 0x03, 0x01}
};

unsigned char TxPwrTrk_CCK_SwingTbl_CH14[TxPwrTrk_CCK_SwingTbl_Len][8] = {
	/*   0.0dB */ {0x36, 0x35, 0x2e, 0x1b, 0x00, 0x00, 0x00, 0x00},
	/*   0.5dB */ {0x33, 0x32, 0x2b, 0x19, 0x00, 0x00, 0x00, 0x00},
	/*   1.0dB */ {0x30, 0x2f, 0x29, 0x18, 0x00, 0x00, 0x00, 0x00},
	/*   1.5dB */ {0x2d, 0x2d, 0x27, 0x17, 0x00, 0x00, 0x00, 0x00},
	/*   2.0dB */ {0x2b, 0x2a, 0x25, 0x15, 0x00, 0x00, 0x00, 0x00},
	/*   2.5dB */ {0x28, 0x28, 0x22, 0x14, 0x00, 0x00, 0x00, 0x00},
	/*   3.0dB */ {0x26, 0x25, 0x21, 0x13, 0x00, 0x00, 0x00, 0x00},
	/*   3.5dB */ {0x24, 0x23, 0x1f, 0x12, 0x00, 0x00, 0x00, 0x00},
	/*   4.0dB */ {0x22, 0x21, 0x1d, 0x11, 0x00, 0x00, 0x00, 0x00},
	/*   4.5dB */ {0x20, 0x20, 0x1b, 0x10, 0x00, 0x00, 0x00, 0x00},
	/*   5.0dB */ {0x1f, 0x1e, 0x1a, 0x0f, 0x00, 0x00, 0x00, 0x00},
	/*   5.5dB */ {0x1d, 0x1c, 0x18, 0x0e, 0x00, 0x00, 0x00, 0x00},
	/*   6.0dB */ {0x1b, 0x1a, 0x17, 0x0e, 0x00, 0x00, 0x00, 0x00},
	/*   6.5dB */ {0x1a, 0x19, 0x16, 0x0d, 0x00, 0x00, 0x00, 0x00},
	/*   7.0dB */ {0x18, 0x17, 0x15, 0x0c, 0x00, 0x00, 0x00, 0x00},
	/*   7.5dB */ {0x17, 0x16, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00},
	/*   8.0dB */ {0x16, 0x15, 0x12, 0x0b, 0x00, 0x00, 0x00, 0x00},
	/*   8.5dB */ {0x14, 0x14, 0x11, 0x0a, 0x00, 0x00, 0x00, 0x00},
	/*   9.0dB */ {0x13, 0x13, 0x10, 0x0a, 0x00, 0x00, 0x00, 0x00},
	/*   9.5dB */ {0x12, 0x12, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},
	/*  10.0dB */ {0x11, 0x11, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},
	/*  10.5dB */ {0x10, 0x10, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00},
	/*  11.0dB */ {0x0f, 0x0f, 0x0d, 0x08, 0x00, 0x00, 0x00, 0x00}
};

//#ifndef USE_OUT_SRC

unsigned char CCKSwingTable_Ch1_Ch13[][8] = {
{0x36, 0x35, 0x2e, 0x25, 0x1c, 0x12, 0x09, 0x04},	// 0, +0dB
{0x33, 0x32, 0x2b, 0x23, 0x1a, 0x11, 0x08, 0x04},	// 1, -0.5dB
{0x30, 0x2f, 0x29, 0x21, 0x19, 0x10, 0x08, 0x03},	// 2, -1.0dB
{0x2d, 0x2d, 0x27, 0x1f, 0x18, 0x0f, 0x08, 0x03},	// 3, -1.5dB
{0x2b, 0x2a, 0x25, 0x1e, 0x16, 0x0e, 0x07, 0x03},	// 4, -2.0dB
{0x28, 0x28, 0x22, 0x1c, 0x15, 0x0d, 0x07, 0x03},	// 5, -2.5dB
{0x26, 0x25, 0x21, 0x1b, 0x14, 0x0d, 0x06, 0x03},	// 6, -3.0dB
{0x24, 0x23, 0x1f, 0x19, 0x13, 0x0c, 0x06, 0x03},	// 7, -3.5dB
{0x22, 0x21, 0x1d, 0x18, 0x11, 0x0b, 0x06, 0x02},	// 8, -4.0dB
{0x20, 0x20, 0x1b, 0x16, 0x11, 0x08, 0x05, 0x02},	// 9, -4.5dB
{0x1f, 0x1e, 0x1a, 0x15, 0x10, 0x0a, 0x05, 0x02},	// 10, -5.0dB
{0x1d, 0x1c, 0x18, 0x14, 0x0f, 0x0a, 0x05, 0x02},	// 11, -5.5dB
{0x1b, 0x1a, 0x17, 0x13, 0x0e, 0x09, 0x04, 0x02},	// 12, -6.0dB
{0x1a, 0x19, 0x16, 0x12, 0x0d, 0x09, 0x04, 0x02},	// 13, -6.5dB
{0x18, 0x17, 0x15, 0x11, 0x0c, 0x08, 0x04, 0x02},	// 14, -7.0dB
{0x17, 0x16, 0x13, 0x10, 0x0c, 0x08, 0x04, 0x02},	// 15, -7.5dB
{0x16, 0x15, 0x12, 0x0f, 0x0b, 0x07, 0x04, 0x01},	// 16, -8.0dB
{0x14, 0x14, 0x11, 0x0e, 0x0b, 0x07, 0x03, 0x02},	// 17, -8.5dB
{0x13, 0x13, 0x10, 0x0d, 0x0a, 0x06, 0x03, 0x01},	// 18, -9.0dB
{0x12, 0x12, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	// 19, -9.5dB
{0x11, 0x11, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x01},	// 20, -10.0dB
{0x10, 0x10, 0x0e, 0x0b, 0x08, 0x05, 0x03, 0x01},	// 21, -10.5dB
{0x0f, 0x0f, 0x0d, 0x0b, 0x08, 0x05, 0x03, 0x01},	// 22, -11.0dB
{0x0e, 0x0e, 0x0c, 0x0a, 0x08, 0x05, 0x02, 0x01},	// 23, -11.5dB
{0x0d, 0x0d, 0x0c, 0x0a, 0x07, 0x05, 0x02, 0x01},	// 24, -12.0dB
{0x0d, 0x0c, 0x0b, 0x09, 0x07, 0x04, 0x02, 0x01},	// 25, -12.5dB
{0x0c, 0x0c, 0x0a, 0x09, 0x06, 0x04, 0x02, 0x01},	// 26, -13.0dB
{0x0b, 0x0b, 0x0a, 0x08, 0x06, 0x04, 0x02, 0x01},	// 27, -13.5dB
{0x0b, 0x0a, 0x09, 0x08, 0x06, 0x04, 0x02, 0x01},	// 28, -14.0dB
{0x0a, 0x0a, 0x09, 0x07, 0x05, 0x03, 0x02, 0x01},	// 29, -14.5dB
{0x0a, 0x09, 0x08, 0x07, 0x05, 0x03, 0x02, 0x01},	// 30, -15.0dB
{0x09, 0x09, 0x08, 0x06, 0x05, 0x03, 0x01, 0x01},	// 31, -15.5dB
{0x09, 0x08, 0x07, 0x06, 0x04, 0x03, 0x01, 0x01}	// 32, -16.0dB
};

unsigned char CCKSwingTable_Ch14 [][8]= {
{0x36, 0x35, 0x2e, 0x1b, 0x00, 0x00, 0x00, 0x00},	// 0, +0dB
{0x33, 0x32, 0x2b, 0x19, 0x00, 0x00, 0x00, 0x00},	// 1, -0.5dB
{0x30, 0x2f, 0x29, 0x18, 0x00, 0x00, 0x00, 0x00},	// 2, -1.0dB
{0x2d, 0x2d, 0x17, 0x17, 0x00, 0x00, 0x00, 0x00},	// 3, -1.5dB
{0x2b, 0x2a, 0x25, 0x15, 0x00, 0x00, 0x00, 0x00},	// 4, -2.0dB
{0x28, 0x28, 0x24, 0x14, 0x00, 0x00, 0x00, 0x00},	// 5, -2.5dB
{0x26, 0x25, 0x21, 0x13, 0x00, 0x00, 0x00, 0x00},	// 6, -3.0dB
{0x24, 0x23, 0x1f, 0x12, 0x00, 0x00, 0x00, 0x00},	// 7, -3.5dB
{0x22, 0x21, 0x1d, 0x11, 0x00, 0x00, 0x00, 0x00},	// 8, -4.0dB
{0x20, 0x20, 0x1b, 0x10, 0x00, 0x00, 0x00, 0x00},	// 9, -4.5dB
{0x1f, 0x1e, 0x1a, 0x0f, 0x00, 0x00, 0x00, 0x00},	// 10, -5.0dB
{0x1d, 0x1c, 0x18, 0x0e, 0x00, 0x00, 0x00, 0x00},	// 11, -5.5dB
{0x1b, 0x1a, 0x17, 0x0e, 0x00, 0x00, 0x00, 0x00},	// 12, -6.0dB
{0x1a, 0x19, 0x16, 0x0d, 0x00, 0x00, 0x00, 0x00},	// 13, -6.5dB
{0x18, 0x17, 0x15, 0x0c, 0x00, 0x00, 0x00, 0x00},	// 14, -7.0dB
{0x17, 0x16, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00},	// 15, -7.5dB
{0x16, 0x15, 0x12, 0x0b, 0x00, 0x00, 0x00, 0x00},	// 16, -8.0dB
{0x14, 0x14, 0x11, 0x0a, 0x00, 0x00, 0x00, 0x00},	// 17, -8.5dB
{0x13, 0x13, 0x10, 0x0a, 0x00, 0x00, 0x00, 0x00},	// 18, -9.0dB
{0x12, 0x12, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	// 19, -9.5dB
{0x11, 0x11, 0x0f, 0x09, 0x00, 0x00, 0x00, 0x00},	// 20, -10.0dB
{0x10, 0x10, 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00},	// 21, -10.5dB
{0x0f, 0x0f, 0x0d, 0x08, 0x00, 0x00, 0x00, 0x00},	// 22, -11.0dB
{0x0e, 0x0e, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	// 23, -11.5dB
{0x0d, 0x0d, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x00},	// 24, -12.0dB
{0x0d, 0x0c, 0x0b, 0x06, 0x00, 0x00, 0x00, 0x00},	// 25, -12.5dB
{0x0c, 0x0c, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	// 26, -13.0dB
{0x0b, 0x0b, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00},	// 27, -13.5dB
{0x0b, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	// 28, -14.0dB
{0x0a, 0x0a, 0x09, 0x05, 0x00, 0x00, 0x00, 0x00},	// 29, -14.5dB
{0x0a, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	// 30, -15.0dB
{0x09, 0x09, 0x08, 0x05, 0x00, 0x00, 0x00, 0x00},	// 31, -15.5dB
{0x09, 0x08, 0x07, 0x04, 0x00, 0x00, 0x00, 0x00}	// 32, -16.0dB
};

#endif

//#if defined(_OUTSRC_COEXIST)
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
const int OFDM_TABLE_SIZE= sizeof(OFDMSwingTable)/sizeof(int);
const int CCK_TABLE_SIZE= sizeof(CCKSwingTable_Ch1_Ch13) >>3;
#endif




#ifdef HW_ANT_SWITCH
#define RXDVY_A_EN		((HW_DIV_ENABLE && !priv->pshare->rf_ft_var.antSw_select) ? 0x80 : 0)
#define RXDVY_B_EN		((HW_DIV_ENABLE &&  priv->pshare->rf_ft_var.antSw_select) ? 0x80 : 0)
#endif


//3 ============================================================
//3 DIG related functions
//3 ============================================================
#if 0
int getIGIFor1RCCA(int value_IGI)
{
	#define ONERCCA_LOW_TH		0x30
	#define ONERCCA_LOW_DIFF	8

	if (value_IGI < ONERCCA_LOW_TH) {
		if ((ONERCCA_LOW_TH - value_IGI) < ONERCCA_LOW_DIFF)
			return ONERCCA_LOW_TH;
		else
			return value_IGI + ONERCCA_LOW_DIFF;
	} else {
		return value_IGI;
	}
}


void set_DIG_state(struct rtl8192cd_priv *priv, int state)
{
	int value_IGI;

	if (state) {
		priv->pshare->DIG_on = 1;
		priv->pshare->restore = 0;
	}
	else {
		priv->pshare->DIG_on = 0;
		if (priv->pshare->restore == 0) {
			if (priv->pshare->rf_ft_var.use_ext_lna == 1)
				value_IGI = 0x30;
			else
				value_IGI = 0x20;

#if defined(HW_ANT_SWITCH)
			// wirte new initial gain index into regC50/C58
			if (priv->pshare->rf_ft_var.one_path_cca == 0)	{
				RTL_W8(0xc50, value_IGI | RXDVY_A_EN);
			} else if (priv->pshare->rf_ft_var.one_path_cca == 1) {
				RTL_W8(0xc50, value_IGI | RXDVY_A_EN);
			} else if (priv->pshare->rf_ft_var.one_path_cca == 2) {
				RTL_W8(0xc50, getIGIFor1RCCA(value_IGI) | RXDVY_A_EN);
			}
#else
			// Write IGI into HW
			if (priv->pshare->rf_ft_var.one_path_cca == 0) 	{
				RTL_W8(0xc50, value_IGI);
			} else if (priv->pshare->rf_ft_var.one_path_cca == 1) {
				RTL_W8(0xc50, value_IGI);
			} else if (priv->pshare->rf_ft_var.one_path_cca == 2) {
				RTL_W8(0xc50, getIGIFor1RCCA(value_IGI));
			}
#endif
			priv->pshare->restore = 1;
		}
#ifdef INTERFERENCE_CONTROL
		priv->pshare->phw->signal_strength = 0;
#endif
	}
}
#endif


void DIG_process(struct rtl8192cd_priv *priv)
{
	#define DEAD_POINT_TH		10000
	#define DOWN_IG_HIT_TH		5
	#define DEAD_POINT_HIT_TH	3

	unsigned char value_IGI;
	signed char value8;
	unsigned int IGI_target;
	unsigned char Adap_IGI_Upper;
#ifdef INTERFERENCE_CONTROL
	unsigned short thd0 = priv->pshare->threshold0;
	unsigned short thd1 = priv->pshare->threshold1;
	unsigned short thd2 = priv->pshare->threshold2;
#endif

	if (priv->pshare->DIG_on == 1)
	{
#ifdef HIGH_POWER_EXT_LNA
		if (priv->pshare->rf_ft_var.use_ext_lna == 1) {
//			priv->pshare->FA_upper = 0x42;
			priv->pshare->FA_upper = MIN_NUM(0x42, priv->pshare->rssi_min+36);
			priv->pshare->FA_lower = 0x30;
		} else
#endif
		{
			// Reset initial gain upper & lower bounds
			if (!priv->pmib->dot11DFSEntry.disable_DFS &&
				(OPMODE & WIFI_AP_STATE) &&
				(((priv->pmib->dot11RFEntry.dot11channel >= 52) &&
				(priv->pmib->dot11RFEntry.dot11channel <= 64)) ||
				((priv->pmib->dot11RFEntry.dot11channel >= 100) &&
				(priv->pmib->dot11RFEntry.dot11channel <= 140)))){
				priv->pshare->FA_upper = 0x24;
				priv->pshare->FA_lower = 0x20;
			}
			else
			{
#ifdef INTERFERENCE_CONTROL
				priv->pshare->FA_lower = 0x20;

				if (priv->pshare->rssi_min != 0xFF)
				{
//					priv->pshare->FA_upper = 0x3E;

					if (priv->pshare->rssi_min > 30)
						priv->pshare->FA_lower = 0x24;
					else if (priv->pshare->rssi_min > 25)
						priv->pshare->FA_lower = 0x22;

					// limit upper bound to prevent the minimal signal sta from disconnect
//					if ((priv->pshare->rssi_min + 10) < priv->pshare->FA_upper)
//						priv->pshare->FA_upper = priv->pshare->rssi_min + 10;
					priv->pshare->FA_upper = MIN_NUM(0x3E, priv->pshare->rssi_min+20);
				}
				else		// before link
				{
					priv->pshare->FA_upper = 0x32;

					thd0 = 500;
					thd1 = 8000;
					thd2 = 10000;
				}
#else
				if (priv->pmib->dot11RFEntry.tx2path) {
					if (priv->pmib->dot11BssType.net_work_type == WIRELESS_11B)
						priv->pshare->FA_upper = MIN_NUM(0x2A, priv->pshare->rssi_min+10);
					else
						priv->pshare->FA_upper = MIN_NUM(0x3E, priv->pshare->rssi_min+10);
				}
				else
					priv->pshare->FA_upper = MIN_NUM(0x3E, priv->pshare->rssi_min+10);
				priv->pshare->FA_lower = 0x20;

				if (priv->pshare->rssi_min > 30)
					priv->pshare->FA_lower = 0x24;
				else if (priv->pshare->rssi_min > 25)
					priv->pshare->FA_lower = 0x22;
#endif
			}
		}

		// determine a new initial gain index according to the sumation of all FA counters as well as upper & lower bounds
		value8 = RTL_R8(0xc50);

		value_IGI = (value8 & 0x7F);

#if  defined(CONFIG_RTL_NOISE_CONTROL_92C)
	if(priv->pshare->rf_ft_var.dnc_enable)
	if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C)){
		unsigned long tp_now = (priv->ext_stats.tx_avarage+priv->ext_stats.rx_avarage)>>17;
#ifdef HIGH_POWER_EXT_LNA
		if(priv->pshare->rf_ft_var.use_ext_lna) {
			if( (priv->pshare->rssi_min > 50)  )  {
				if((!priv->pshare->DNC_on) && (value_IGI >= priv->pshare->FA_upper) && (priv->pshare->FA_total_cnt > priv->pshare->threshold2)) {
					priv->pshare->DNC_on = 1;
					priv->ext_stats.tp_average_pre = tp_now;
					priv->pshare->FA_lower = 0x20;
					PHY_SetBBReg(priv, 0x870, bMaskDWord, RTL_R32(0x870)|BIT(5)|BIT(6)|BIT(21)|BIT(22));

#ifdef HW_ANT_SWITCH
					PHY_SetBBReg(priv, 0xc50, bMaskByte0, priv->pshare->FA_lower | RXDVY_A_EN);
					PHY_SetBBReg(priv, 0xc58, bMaskByte0, priv->pshare->FA_lower | RXDVY_B_EN);
#else
					PHY_SetBBReg(priv, 0xc50, bMaskByte0, priv->pshare->FA_lower);
					PHY_SetBBReg(priv, 0xc58, bMaskByte0, priv->pshare->FA_lower);
#endif

				} else if(priv->pshare->DNC_on ==1)  {
					if(tp_now < priv->ext_stats.tp_average_pre + 2) {
						priv->pshare->DNC_on = 0;
					}
					else {
						priv->pshare->DNC_on =2;
						priv->ext_stats.tp_average_pre = tp_now;
					}
				} else if(priv->pshare->DNC_on >= 2 ) {
					if(( tp_now+10 < priv->ext_stats.tp_average_pre ) || (tp_now < 1) ) {
							priv->pshare->DNC_on = 0;
					} else if(priv->pshare->DNC_on<5) {
						priv->ext_stats.tp_average_pre = tp_now;
						++priv->pshare->DNC_on;
					}
				}
			 }else {
				priv->pshare->DNC_on = 0;
			}

			if(	priv->pshare->DNC_on )
				return;
			else
				PHY_SetBBReg(priv, 0x870, bMaskDWord, RTL_R32(0x870)&  ~(BIT(5)|BIT(6)|BIT(21)|BIT(22)));

		} else
#endif
		{
			if( (priv->pshare->rssi_min > 40) && (value_IGI >= priv->pshare->FA_upper) )  {
//				unsigned long tp_now = (priv->ext_stats.tx_avarage+priv->ext_stats.rx_avarage)>>17;
				if((!priv->pshare->DNC_on) && (priv->pshare->FA_total_cnt > priv->pshare->threshold2)) {
					priv->pshare->DNC_on = 1;
					priv->ext_stats.tp_average_pre = tp_now;
				} else if(priv->pshare->DNC_on ==1)  {
					if(tp_now < priv->ext_stats.tp_average_pre + 2) {
						priv->pshare->DNC_on = 0;
					}
					else {
						priv->pshare->DNC_on = 2;
						priv->ext_stats.tp_average_pre = tp_now;
					}
				} else if(priv->pshare->DNC_on >= 2 ) {
					if((tp_now +10 < priv->ext_stats.tp_average_pre )
						|| ((priv->ext_stats.tp_average_pre < 10) && (priv->pshare->FA_total_cnt < priv->pshare->threshold1))) {
						priv->pshare->DNC_on = 0;
					} else 	if(priv->pshare->DNC_on<6) {
						priv->ext_stats.tp_average_pre = tp_now;
						++priv->pshare->DNC_on;
					}
				}
				if(priv->pshare->DNC_on) {
					priv->pshare->FA_upper = 0x3e;
				}
			}else {
				priv->pshare->DNC_on = 0;
			}
		}
	}
#endif

		if ((priv->pshare->digDeadPoint == 0) && (priv->pshare->FA_total_cnt > DEAD_POINT_TH)) {
			if ((priv->pshare->digDeadPointHitCount > 0) && (priv->pshare->digDeadPointCandidate == value_IGI)) {
				priv->pshare->digDeadPointHitCount++;
				if (priv->pshare->digDeadPointHitCount == DEAD_POINT_HIT_TH) {
					priv->pshare->digDeadPoint = priv->pshare->digDeadPointCandidate;
				}
			} else {
				priv->pshare->digDeadPointCandidate = value_IGI;
				priv->pshare->digDeadPointHitCount = 1;
			}
		}

#ifdef INTERFERENCE_CONTROL
		if (priv->pshare->FA_total_cnt < thd0) {
#else
		if (priv->pshare->FA_total_cnt < priv->pshare->threshold0) {
#endif
			priv->pshare->digDownCount++;
			if (priv->pshare->digDownCount > DOWN_IG_HIT_TH) {
				// Reset deadpoint hit count
				if ((priv->pshare->digDeadPoint == 0) && (priv->pshare->digDeadPointHitCount > 0) && (value_IGI == priv->pshare->digDeadPointCandidate))
					priv->pshare->digDeadPointHitCount = 0;

				value_IGI--;

				// Check if the new value is dead point
				if ((priv->pshare->digDeadPoint > 0) && (value_IGI == priv->pshare->digDeadPoint))
					value_IGI++;
			}
#ifdef INTERFERENCE_CONTROL
		} else if (priv->pshare->FA_total_cnt < thd1) {
#else
		} else if (priv->pshare->FA_total_cnt < priv->pshare->threshold1) {
#endif
			value_IGI += 0;
			priv->pshare->digDownCount = 0;
#ifdef INTERFERENCE_CONTROL
		} else if (priv->pshare->FA_total_cnt < thd2) {
#else
		} else if (priv->pshare->FA_total_cnt < priv->pshare->threshold2) {
#endif
			value_IGI++;
			priv->pshare->digDownCount = 0;
#ifdef INTERFERENCE_CONTROL
		} else if (priv->pshare->FA_total_cnt >= thd2) {
#else
		} else if (priv->pshare->FA_total_cnt >= priv->pshare->threshold2) {
#endif
			value_IGI += 2;
			priv->pshare->digDownCount = 0;
		} else {
			priv->pshare->digDownCount = 0;
		}

		if (value_IGI > priv->pshare->FA_upper)
			value_IGI = priv->pshare->FA_upper;
		else if (value_IGI < priv->pshare->FA_lower)
			value_IGI = priv->pshare->FA_lower;
		if (priv->pshare->rf_ft_var.adaptivity_enable && priv->pshare->rf_ft_var.adaptivity_flag == TRUE)
		{
			Adap_IGI_Upper = priv->pshare->rf_ft_var.Adaptivity_IGI_upper;
			if(value_IGI > Adap_IGI_Upper)
				value_IGI = Adap_IGI_Upper;
		}

#if defined(HW_ANT_SWITCH)
		// wirte new initial gain index into regC50/C58
		if (priv->pshare->rf_ft_var.one_path_cca == 0)	{
			RTL_W8(0xc50, value_IGI | RXDVY_A_EN);
		} else if (priv->pshare->rf_ft_var.one_path_cca == 1)	{
			RTL_W8(0xc50, value_IGI | RXDVY_A_EN);
		} else if (priv->pshare->rf_ft_var.one_path_cca == 2)		{
			RTL_W8(0xc50, getIGIFor1RCCA(value_IGI) | RXDVY_A_EN);
		}
#else
		// Write IGI into HW
		if (priv->pshare->rf_ft_var.one_path_cca == 0) {
			RTL_W8(0xc50, value_IGI);
		} else if (priv->pshare->rf_ft_var.one_path_cca == 1) {
			RTL_W8(0xc50, value_IGI);
		} else if (priv->pshare->rf_ft_var.one_path_cca == 2) {
			RTL_W8(0xc50, getIGIFor1RCCA(value_IGI));
		}
#endif

	}
}

#if 0
void check_DIG_by_rssi(struct rtl8192cd_priv *priv, unsigned char rssi_strength)
{
	unsigned int dig_on = 0;

	if (OPMODE & WIFI_SITE_MONITOR)
		return;

	if ((rssi_strength > priv->pshare->rf_ft_var.digGoUpperLevel)
		&& (rssi_strength < HP_LOWER+1) && (priv->pshare->phw->signal_strength != 2)) {
		if (priv->pshare->is_40m_bw)
			// RTL_W8(0xc87, (RTL_R8(0xc87) & 0xf) | 0x30); 92D
			RTL_W8(0xc87, 0x30);
		else
			RTL_W8(0xc30, 0x44);

		if (priv->pshare->phw->signal_strength != 3)
			dig_on++;

		priv->pshare->phw->signal_strength = 2;
	}
	else if ((rssi_strength > HP_LOWER+5) && (priv->pshare->phw->signal_strength != 3)) {
		if (priv->pshare->is_40m_bw)
			// RTL_W8(0xc87, (RTL_R8(0xc87) & 0xf) | 0x30); 92D
			RTL_W8(0xc87, 0x30);
		else
			RTL_W8(0xc30, 0x44);

		if (priv->pshare->phw->signal_strength != 2)
			dig_on++;

		priv->pshare->phw->signal_strength = 3;
	}
	else if (((rssi_strength < priv->pshare->rf_ft_var.digGoLowerLevel)
		&& (priv->pshare->phw->signal_strength != 1)) || !priv->pshare->phw->signal_strength) {
		// DIG off
//		set_DIG_state(priv, 0);

		if (priv->pshare->is_40m_bw)
			//RTL_W8(0xc87, (RTL_R8(0xc87) & 0xf) | 0x30); 92D
			RTL_W8(0xc87, 0x30);
		else
			RTL_W8(0xc30, 0x44);

		priv->pshare->phw->signal_strength = 1;
	}

	if (dig_on) {
		// DIG on
		set_DIG_state(priv, 1);
	}

	//check_DC_TH_by_rssi(priv, rssi_strength);
}


void DIG_for_site_survey(struct rtl8192cd_priv *priv, int do_ss)
{
	if (do_ss) {
		// DIG off
		set_DIG_state(priv, 0);
	}
	else {
		// DIG on
#ifndef INTERFERENCE_CONTROL
		if (priv->pshare->phw->signal_strength > 1)
#endif
		{
			set_DIG_state(priv, 1);
		}
	}
}
#endif

#if 0
//#ifdef INTERFERENCE_CONTROL
void check_NBI_by_rssi(struct rtl8192cd_priv *priv, unsigned char rssi_strength)
{
	if (OPMODE & WIFI_SITE_MONITOR)
		return;

	if (priv->pshare->phw->nbi_filter_on) {
		if (rssi_strength < 20) {
			priv->pshare->phw->nbi_filter_on = 0;
			RTL_W16(rOFDM0_RxDSP, RTL_R16(rOFDM0_RxDSP) & ~ BIT(9));	// NBI off
		}
	} else {	// NBI OFF previous
		if (rssi_strength > 25) {
			priv->pshare->phw->nbi_filter_on = 1;
			RTL_W16(rOFDM0_RxDSP, RTL_R16(rOFDM0_RxDSP) | BIT(9));		// NBI on
		}
	}
}
#endif

/*
 * dynamic CCK CCA enhance by rssi
 */
void CCK_CCA_dynamic_enhance(struct rtl8192cd_priv *priv, unsigned char rssi_strength)
{
#if 1
	unsigned int cck_fa = priv->pshare->FA_total_cnt;
	int rssi_thd = 30;

	if (rssi_strength == 0xff) {
		if (cck_fa < 1000) {
			if (priv->pshare->phw->CCK_CCA_enhanced != 2) {
				RTL_W8(0xa0a, 0x40);
				priv->pshare->phw->CCK_CCA_enhanced = 2;
			}
		} else {
			if (priv->pshare->phw->CCK_CCA_enhanced != 1) {
				RTL_W8(0xa0a, 0x83);
				priv->pshare->phw->CCK_CCA_enhanced = 1;
			}
		}
		return;
	}

	if (rssi_strength > rssi_thd+5) {
		if (priv->pshare->phw->CCK_CCA_enhanced != 0) {
			RTL_W8(0xa0a, 0xcd);
			priv->pshare->phw->CCK_CCA_enhanced = 0;
		}
	} else if (rssi_strength< rssi_thd) {
		if ((rssi_strength > 9) || (priv->assoc_num >1)) {
			if (priv->pshare->phw->CCK_CCA_enhanced != 1) {
				RTL_W8(0xa0a, 0x83);
				priv->pshare->phw->CCK_CCA_enhanced = 1;
			}
		} else {
			if(cck_fa<1000) {
				if (priv->pshare->phw->CCK_CCA_enhanced != 2) {
					RTL_W8(0xa0a, 0x40);
					priv->pshare->phw->CCK_CCA_enhanced = 2;
				}
			} else {
				if (priv->pshare->phw->CCK_CCA_enhanced != 1) {
					RTL_W8(0xa0a, 0x83);
					priv->pshare->phw->CCK_CCA_enhanced = 1;
				}
			}
		}
	}

#else

	if (rssi_strength == 0xff)
		return;

	if (!priv->pshare->phw->CCK_CCA_enhanced && (rssi_strength < 30)) {
		priv->pshare->phw->CCK_CCA_enhanced = TRUE;
		RTL_W8(0xa0a, 0x83);
	}
	else if (priv->pshare->phw->CCK_CCA_enhanced && (rssi_strength > 35)) {
		priv->pshare->phw->CCK_CCA_enhanced = FALSE;
		RTL_W8(0xa0a, 0xcd);
	}
#endif
}


//3 ============================================================
//3 Dynamic Tx Power / Power Tracking
//3 ============================================================

#ifdef HIGH_POWER_EXT_PA
void tx_power_control(struct rtl8192cd_priv *priv)
{
	unsigned long x;

	int pwr_value = 0x10101010;
	if( priv->pshare->phw->signal_strength == 3 && priv->pshare->phw->lower_tx_power== 0) {
		SAVE_INT_AND_CLI(x);
		priv->pshare->phw->power_backup[0x00] = RTL_R32(rTxAGC_A_Rate18_06);
		priv->pshare->phw->power_backup[0x01] = RTL_R32(rTxAGC_A_Rate54_24);
		priv->pshare->phw->power_backup[0x02] = RTL_R32(rTxAGC_B_Rate18_06);
		priv->pshare->phw->power_backup[0x03] = RTL_R32(rTxAGC_B_Rate54_24);
		priv->pshare->phw->power_backup[0x04] = RTL_R32(rTxAGC_A_Mcs03_Mcs00);
		priv->pshare->phw->power_backup[0x05] = RTL_R32(rTxAGC_A_Mcs07_Mcs04);
		priv->pshare->phw->power_backup[0x06] = RTL_R32(rTxAGC_A_Mcs11_Mcs08);
		priv->pshare->phw->power_backup[0x07] = RTL_R32(rTxAGC_A_Mcs15_Mcs12);
		priv->pshare->phw->power_backup[0x08] = RTL_R32(rTxAGC_B_Mcs03_Mcs00);
		priv->pshare->phw->power_backup[0x09] = RTL_R32(rTxAGC_B_Mcs07_Mcs04);
		priv->pshare->phw->power_backup[0x0a] = RTL_R32(rTxAGC_B_Mcs11_Mcs08);
		priv->pshare->phw->power_backup[0x0b] = RTL_R32(rTxAGC_B_Mcs15_Mcs12);
		priv->pshare->phw->power_backup[0x0c] = RTL_R32(rTxAGC_A_CCK11_2_B_CCK11);
		priv->pshare->phw->power_backup[0x0d] = RTL_R32(rTxAGC_A_CCK1_Mcs32);
		priv->pshare->phw->power_backup[0x0e] = RTL_R32(rTxAGC_B_CCK5_1_Mcs32);
		RTL_W32(rTxAGC_A_Rate18_06, pwr_value);
		RTL_W32(rTxAGC_A_Rate54_24, pwr_value);
		RTL_W32(rTxAGC_B_Rate18_06, pwr_value);
		RTL_W32(rTxAGC_B_Rate54_24, pwr_value);
		RTL_W32(rTxAGC_A_Mcs03_Mcs00, pwr_value);
		RTL_W32(rTxAGC_A_Mcs07_Mcs04, pwr_value);
		RTL_W32(rTxAGC_A_Mcs11_Mcs08, pwr_value);
		RTL_W32(rTxAGC_A_Mcs15_Mcs12, pwr_value);
		RTL_W32(rTxAGC_B_Mcs03_Mcs00, pwr_value);
		RTL_W32(rTxAGC_B_Mcs07_Mcs04, pwr_value);
		RTL_W32(rTxAGC_B_Mcs11_Mcs08, pwr_value);
		RTL_W32(rTxAGC_B_Mcs15_Mcs12, pwr_value);
		RTL_W32(rTxAGC_A_CCK11_2_B_CCK11, pwr_value);
		RTL_W32(rTxAGC_A_CCK1_Mcs32, (pwr_value & 0x0000ff00) | (priv->pshare->phw->power_backup[0x0d] &0xffff00ff));
		RTL_W32(rTxAGC_B_CCK5_1_Mcs32, (pwr_value & 0xffffff00) | (priv->pshare->phw->power_backup[0x0e] &0x000000ff));
		priv->pshare->phw->lower_tx_power = 1;
		RESTORE_INT(x);
	}
	else if( priv->pshare->phw->signal_strength != 3 && priv->pshare->phw->lower_tx_power) {
		SAVE_INT_AND_CLI(x);
		RTL_W32(rTxAGC_A_Rate18_06, priv->pshare->phw->power_backup[0x00]);
		RTL_W32(rTxAGC_A_Rate54_24, priv->pshare->phw->power_backup[0x01]);
		RTL_W32(rTxAGC_B_Rate18_06, priv->pshare->phw->power_backup[0x02]);
		RTL_W32(rTxAGC_B_Rate54_24, priv->pshare->phw->power_backup[0x03]);
		RTL_W32(rTxAGC_A_Mcs03_Mcs00, priv->pshare->phw->power_backup[0x04]);
		RTL_W32(rTxAGC_A_Mcs07_Mcs04, priv->pshare->phw->power_backup[0x05]);
		RTL_W32(rTxAGC_A_Mcs11_Mcs08, priv->pshare->phw->power_backup[0x06]);
		RTL_W32(rTxAGC_A_Mcs15_Mcs12, priv->pshare->phw->power_backup[0x07]);
		RTL_W32(rTxAGC_B_Mcs03_Mcs00, priv->pshare->phw->power_backup[0x08]);
		RTL_W32(rTxAGC_B_Mcs07_Mcs04, priv->pshare->phw->power_backup[0x09]);
		RTL_W32(rTxAGC_B_Mcs11_Mcs08, priv->pshare->phw->power_backup[0x0a]);
		RTL_W32(rTxAGC_B_Mcs15_Mcs12, priv->pshare->phw->power_backup[0x0b]);
		RTL_W32(rTxAGC_A_CCK11_2_B_CCK11, priv->pshare->phw->power_backup[0x0c]);
		RTL_W32(rTxAGC_A_CCK1_Mcs32,  priv->pshare->phw->power_backup[0x0d]);
		RTL_W32(rTxAGC_B_CCK5_1_Mcs32,  priv->pshare->phw->power_backup[0x0e]);
		priv->pshare->phw->lower_tx_power = 0;
		RESTORE_INT(x);
	}
}
#endif

#if 0
int get_CCK_swing_index(struct rtl8192cd_priv *priv)
{
	int TempCCk, index=12, i;
	short channel;
#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		channel=priv->pshare->working_channel;
	else
#endif
		channel = (priv->pmib->dot11RFEntry.dot11channel);

	//Query CCK default setting From 0xa24
	TempCCk = PHY_QueryBBReg(priv, rCCK0_TxFilter2, bMaskDWord)&bMaskCCK;
	TempCCk = cpu_to_le32(TempCCk);
	for(i=0 ; i<CCK_TABLE_SIZE ; i++)		{
		if(channel==14) {
			if(memcmp((void*)&TempCCk, (void*)&CCKSwingTable_Ch14[i][2], 4)==0) {
				index = i;
				break;
			}
		} else {
			if(memcmp((void*)&TempCCk, (void*)&CCKSwingTable_Ch1_Ch13[i][2], 4)==0) {
				index = i;
				break;
			}
		}
	}
	DEBUG_INFO("Initial reg0x%x = 0x%lx, CCK_index=0x%x, ch %d\n",
							rCCK0_TxFilter2, TempCCk, index, channel);
	return index;
}


void set_CCK_swing_index(struct rtl8192cd_priv *priv, short CCK_index)
{
	short channel;
#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		channel=priv->pshare->working_channel;
	else
#endif
		channel = (priv->pmib->dot11RFEntry.dot11channel);


	if(channel !=14) {
		RTL_W8( 0xa22, CCKSwingTable_Ch1_Ch13[CCK_index][0]);
		RTL_W8( 0xa23, CCKSwingTable_Ch1_Ch13[CCK_index][1]);
		RTL_W8( 0xa24, CCKSwingTable_Ch1_Ch13[CCK_index][2]);
		RTL_W8( 0xa25, CCKSwingTable_Ch1_Ch13[CCK_index][3]);
		RTL_W8( 0xa26, CCKSwingTable_Ch1_Ch13[CCK_index][4]);
		RTL_W8( 0xa27, CCKSwingTable_Ch1_Ch13[CCK_index][5]);
		RTL_W8( 0xa28, CCKSwingTable_Ch1_Ch13[CCK_index][6]);
		RTL_W8( 0xa29, CCKSwingTable_Ch1_Ch13[CCK_index][7]);
	}
	else{
		RTL_W8( 0xa22, CCKSwingTable_Ch14[CCK_index][0]);
		RTL_W8( 0xa23, CCKSwingTable_Ch14[CCK_index][1]);
		RTL_W8( 0xa24, CCKSwingTable_Ch14[CCK_index][2]);
		RTL_W8( 0xa25, CCKSwingTable_Ch14[CCK_index][3]);
		RTL_W8( 0xa26, CCKSwingTable_Ch14[CCK_index][4]);
		RTL_W8( 0xa27, CCKSwingTable_Ch14[CCK_index][5]);
		RTL_W8( 0xa28, CCKSwingTable_Ch14[CCK_index][6]);
		RTL_W8( 0xa29, CCKSwingTable_Ch14[CCK_index][7]);
	}
}
#endif

unsigned char getThermalValue(struct rtl8192cd_priv *priv)
{
	unsigned char	ThermalValue;
	int sum=0, i=0;
	PHY_SetRFReg(priv, RF92CD_PATH_A, RF_T_METER, bMask20Bits, 0x60);
	while ((PHY_QueryRFReg(priv, RF92CD_PATH_A, RF_T_METER, bMask20Bits, 1) > 0x1f) && ((i++) < 1000)) {//<20ms, test is in 20 us
		delay_us(20);
	}
	ThermalValue =(unsigned char)PHY_QueryRFReg(priv, RF92CD_PATH_A, RF_T_METER, bMask20Bits, 1) & 0x01f;
	priv->pshare->Thermal_idx = (priv->pshare->Thermal_idx+1)%8;
	priv->pshare->Thermal_log[ priv->pshare->Thermal_idx ] = ThermalValue;
	for(i=0; i<8; i++) {
		if(!priv->pshare->Thermal_log[i])
			return ThermalValue;
		sum += priv->pshare->Thermal_log[i];
	}
	return (sum+4)>>3;
}




#ifdef RX_GAIN_TRACK_92D
static void rx_gain_tracking_92D(struct rtl8192cd_priv *priv)
{
	u8	index_mapping[Rx_index_mapping_NUM] = {
						0x0f,	0x0f,	0x0f,	0x0f,	0x0b,
						0x0a,	0x09,	0x08,	0x07,	0x06,
						0x05,	0x04,	0x04,	0x03,	0x02
					};

	u8	eRFPath, curMaxRFPath;
	u32	u4tmp;

	u4tmp = (index_mapping[(priv->pmib->dot11RFEntry.ther - priv->pshare->ThermalValue_RxGain)]) << 12;

	DEBUG_INFO("===>%s interface %d  Rx Gain %x\n", __FUNCTION__, priv->pshare->wlandev_idx, u4tmp);

	if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)
		curMaxRFPath = RF92CD_PATH_B;
	else
		curMaxRFPath = RF92CD_PATH_MAX;

	for(eRFPath = RF92CD_PATH_A; eRFPath < curMaxRFPath; eRFPath++)
		PHY_SetRFReg(priv, eRFPath, 0x3C, bMask20Bits, (priv->pshare->RegRF3C[eRFPath]&(~(0xF000)))|u4tmp);

};

#endif

#if 0
//#ifdef CONFIG_RTL_88E_SUPPORT //for 88e tx power tracking

void ODM_ResetIQKResult(struct rtl8192cd_priv *priv)
{
/*
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN || DM_ODM_SUPPORT_TYPE == ODM_CE)
	PADAPTER	Adapter = pDM_Odm->Adapter;
	u1Byte		i;

	if (!IS_HARDWARE_TYPE_8192D(Adapter))
		return;
#endif
*/

	unsigned char i;

	//printk("PHY_ResetIQKResult:: settings regs %d default regs %d\n", sizeof(priv->pshare->IQKMatrixRegSetting)/sizeof(IQK_MATRIX_REGS_SETTING), IQK_Matrix_Settings_NUM);
	//0xe94, 0xe9c, 0xea4, 0xeac, 0xeb4, 0xebc, 0xec4, 0xecc

	for(i = 0; i < IQK_Matrix_Settings_NUM; i++)
	{
		{
			priv->pshare->IQKMatrixRegSetting[i].Value[0][0] =
				priv->pshare->IQKMatrixRegSetting[i].Value[0][2] =
				priv->pshare->IQKMatrixRegSetting[i].Value[0][4] =
				priv->pshare->IQKMatrixRegSetting[i].Value[0][6] = 0x100;

			priv->pshare->IQKMatrixRegSetting[i].Value[0][1] =
				priv->pshare->IQKMatrixRegSetting[i].Value[0][3] =
				priv->pshare->IQKMatrixRegSetting[i].Value[0][5] =
				priv->pshare->IQKMatrixRegSetting[i].Value[0][7] = 0x0;

			priv->pshare->IQKMatrixRegSetting[i].bIQKDone = FALSE;

		}
	}

}

#define	RF_PATH_A		0		//Radio Path A
#define	OFDM_TABLE_SIZE_92D 	43

#define bRFRegOffsetMask	0xfffff


//091212 chiyokolin
void odm_TXPowerTrackingCallback_ThermalMeter_8188E(struct rtl8192cd_priv *priv)
{

	//HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	unsigned char			ThermalValue = 0, delta, delta_LCK, delta_IQK, offset;
	unsigned char			ThermalValue_AVG_count = 0;
	unsigned int			ThermalValue_AVG = 0;
	int						ele_A=0, ele_D, TempCCk, X, value32;
	int						Y, ele_C=0;
	char					OFDM_index[2], CCK_index=0, OFDM_index_old[2]={0,0}, CCK_index_old=0, index;
	unsigned int			i = 0, j = 0;
	char 					is2T = FALSE;
	//char 					bInteralPA = FALSE;

	unsigned char			OFDM_min_index = 6, rf; //OFDM BB Swing should be less than +3.0dB, which is required by Arthur
	unsigned char			Indexforchannel = 0/*GetRightChnlPlaceforIQK(pHalData->CurrentChannel)*/;
	char					OFDM_index_mapping[2][index_mapping_NUM_88E] = {
							{0,	0,	2,	3,	4,	4,			//2.4G, decrease power
							5, 	6, 	7, 	7,	8,	9,
							10,	10,	11}, // For lower temperature, 20120220 updated on 20120220.
							{0,	0,	-1,	-2,	-3,	-4,			//2.4G, increase power
							-4, 	-4, 	-4, 	-5,	-7,	-8,
							-9,	-9,	-10},
							};
	unsigned char			Thermal_mapping[2][index_mapping_NUM_88E] = {
							{0,	2,	4,	6,	8,	10,			//2.4G, decrease power
							12, 	14, 	16, 	18,	20,	22,
							24,	26,	27},
							{0, 	2,	4,	6,	8,	10, 			//2.4G,, increase power
							12, 	14, 	16, 	18, 	20, 	22,
							25,	25,	25},
							};

	priv->pshare->TXPowerTrackingCallbackCnt++; //cosa add for debug
	priv->pshare->bTXPowerTrackingInit = TRUE;

#if (MP_DRIVER == 1)
    priv->pshare->TxPowerTrackControl = 1; //priv->pshare->TxPowerTrackControl; //_eric_?? // <Kordan> We should keep updating the control variable according to HalData.
    // <Kordan> pshare->RegA24 will be initialized when ODM HW configuring, but MP configures with para files.
    priv->pshare->RegA24 = 0x090e1317;
#endif


#ifdef MP_TEST
		if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific) {
			if(priv->pshare->mp_txpwr_tracking == FALSE)
				return;
		}
#endif

	if(priv->pshare->Power_tracking_on_88E == 0)
	{
		priv->pshare->Power_tracking_on_88E = 1;
		PHY_SetRFReg(priv, RF92CD_PATH_A, 0x42, (BIT(17) | BIT(16)), 0x03);
		return;
	}
	else
	{

		priv->pshare->Power_tracking_on_88E = 0;

		//printk("===>dm_TXPowerTrackingCallback_ThermalMeter_8188E txpowercontrol %d\n",  priv->pshare->TxPowerTrackControl);

		ThermalValue = (unsigned char)PHY_QueryRFReg(priv, RF_PATH_A, RF_T_METER_88E, 0xfc00, 1);	//0x42: RF Reg[15:10] 88E

		printk("\nReadback Thermal Meter = 0x%x pre thermal meter 0x%x EEPROMthermalmeter 0x%x\n", ThermalValue, priv->pshare->ThermalValue, priv->pmib->dot11RFEntry.ther);

	}

	if(is2T)
		rf = 2;
	else
		rf = 1;

	if(ThermalValue)
	{
//		if(!pHalData->ThermalValue)
		{
			//Query OFDM path A default setting
			ele_D = PHY_QueryBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord)&bMaskOFDM_D;

			for(i=0; i<OFDM_TABLE_SIZE_92D; i++)	//find the index
			{
				if(ele_D == (OFDMSwingTable[i]&bMaskOFDM_D))
				{
					OFDM_index_old[0] = (unsigned char)i;
					printk("Initial pathA ele_D reg0x%x = 0x%x, OFDM_index=0x%x\n",
						rOFDM0_XATxIQImbalance, ele_D, OFDM_index_old[0]);
					break;
				}
			}

			//Query OFDM path B default setting
			if(is2T)
			{
				ele_D = PHY_QueryBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord)&bMaskOFDM_D;
				for(i=0; i<OFDM_TABLE_SIZE_92D; i++)	//find the index
				{
					if(ele_D == (OFDMSwingTable[i]&bMaskOFDM_D))
					{
						OFDM_index_old[1] = (unsigned char)i;
						printk("Initial pathB ele_D reg0x%x = 0x%x, OFDM_index=0x%x\n",
							rOFDM0_XBTxIQImbalance, ele_D, OFDM_index_old[1]);
						break;
					}
				}
			}

			{
				//Query CCK default setting From 0xa24
				TempCCk = priv->pshare->RegA24;

				for(i=0 ; i<CCK_TABLE_SIZE ; i++)
				{
					if(priv->pshare->bCCKinCH14)
					{
						if(memcmp((void*)&TempCCk, (void*)&CCKSwingTable_Ch14[i][2], 4)==0)
						{
							CCK_index_old =(unsigned char) i;
							//printk("Initial reg0x%x = 0x%x, CCK_index=0x%x, ch 14 %d\n",
								//rCCK0_TxFilter2, TempCCk, CCK_index_old, priv->pshare->bCCKinCH14);
							break;
						}
					}
					else
					{
                        //printk("RegA24: 0x%X, CCKSwingTable_Ch1_Ch13[%d][2]: CCKSwingTable_Ch1_Ch13[i][2]: 0x%X\n", TempCCk, i, CCKSwingTable_Ch1_Ch13[i][2]);
						if(memcmp((void*)&TempCCk, (void*)&CCKSwingTable_Ch1_Ch13[i][2], 4)==0)
						{
							CCK_index_old =(unsigned char) i;
							//printk("Initial reg0x%x = 0x%x, CCK_index=0x%x, ch14 %d\n",
								//rCCK0_TxFilter2, TempCCk, CCK_index_old, priv->pshare->bCCKinCH14);
							break;
						}
					}
				}
			}

			if(!priv->pshare->ThermalValue)
			{
				priv->pshare->ThermalValue = priv->pmib->dot11RFEntry.ther;
				priv->pshare->ThermalValue_LCK = ThermalValue;
				priv->pshare->ThermalValue_IQK = ThermalValue;

				for(i = 0; i < rf; i++)
					priv->pshare->OFDM_index[i] = OFDM_index_old[i];
				priv->pshare->CCK_index = CCK_index_old;
			}

			if(priv->pshare->bReloadtxpowerindex)
			{
				printk("reload ofdm index for band switch\n");
			}

			//calculate average thermal meter
			{
				priv->pshare->ThermalValue_AVG[priv->pshare->ThermalValue_AVG_index] = ThermalValue;
				priv->pshare->ThermalValue_AVG_index++;
				if(priv->pshare->ThermalValue_AVG_index == AVG_THERMAL_NUM_88E)
					priv->pshare->ThermalValue_AVG_index = 0;

				for(i = 0; i < AVG_THERMAL_NUM_88E; i++)
				{
					if(priv->pshare->ThermalValue_AVG[i])
					{
						ThermalValue_AVG += priv->pshare->ThermalValue_AVG[i];
						ThermalValue_AVG_count++;
					}
				}

				if(ThermalValue_AVG_count)
				{
					ThermalValue = (unsigned char)(ThermalValue_AVG / ThermalValue_AVG_count);
					printk("AVG Thermal Meter = 0x%x \n", ThermalValue);
				}
			}
		}

		if(priv->pshare->bReloadtxpowerindex)
		{
			delta = ThermalValue > priv->pmib->dot11RFEntry.ther?(ThermalValue - priv->pmib->dot11RFEntry.ther):(priv->pmib->dot11RFEntry.ther - ThermalValue);
			priv->pshare->bReloadtxpowerindex = FALSE;
			priv->pshare->bDoneTxpower = FALSE;
		}
		else if(priv->pshare->bDoneTxpower)
		{
			delta = (ThermalValue > priv->pshare->ThermalValue)?(ThermalValue - priv->pshare->ThermalValue):(priv->pshare->ThermalValue - ThermalValue);
		}
		else
		{
			delta = ThermalValue > priv->pmib->dot11RFEntry.ther?(ThermalValue - priv->pmib->dot11RFEntry.ther):(priv->pmib->dot11RFEntry.ther - ThermalValue);
		}
		delta_LCK = (ThermalValue > priv->pshare->ThermalValue_LCK)?(ThermalValue - priv->pshare->ThermalValue_LCK):(priv->pshare->ThermalValue_LCK - ThermalValue);
		delta_IQK = (ThermalValue > priv->pshare->ThermalValue_IQK)?(ThermalValue - priv->pshare->ThermalValue_IQK):(priv->pshare->ThermalValue_IQK - ThermalValue);

		printk("Readback Thermal Meter = 0x%x \npre thermal meter 0x%x EEPROMthermalmeter 0x%x delta 0x%x \ndelta_LCK 0x%x delta_IQK 0x%x \n",   ThermalValue, priv->pshare->ThermalValue, priv->pshare->EEPROMThermalMeter, delta, delta_LCK, delta_IQK);
		printk("pre thermal meter LCK 0x%x \npre thermal meter IQK 0x%x \ndelta_LCK_bound 0x%x delta_IQK_bound 0x%x\n",   priv->pshare->ThermalValue_LCK, priv->pshare->ThermalValue_IQK, priv->pshare->Delta_LCK, priv->pshare->Delta_IQK);


		//if((delta_LCK > pHalData->Delta_LCK) && (pHalData->Delta_LCK != 0))
        if (delta_LCK >= 8) // Delta temperature is equal to or larger than 20 centigrade.
		{
            priv->pshare->ThermalValue_LCK = ThermalValue;
			PHY_LCCalibrate(priv);
		}


		if(delta > 0 && priv->pshare->TxPowerTrackControl)
		{
			delta = ThermalValue > priv->pmib->dot11RFEntry.ther?(ThermalValue - priv->pmib->dot11RFEntry.ther):(priv->pmib->dot11RFEntry.ther - ThermalValue);

			//calculate new OFDM / CCK offset
			{
				{
					if(ThermalValue > priv->pmib->dot11RFEntry.ther)
						j = 1;
					else
						j = 0;

					for(offset = 0; offset < index_mapping_NUM_88E; offset++)
					{
						if(delta < Thermal_mapping[j][offset])
						{
							if(offset != 0)
								offset--;
							break;
						}
					}
					if(offset >= index_mapping_NUM_88E)
						offset = index_mapping_NUM_88E-1;

					index = OFDM_index_mapping[j][offset];

					printk("\nj = %d delta = %d, index = %d\n\n", j, delta, index);

					for(i = 0; i < rf; i++)
						OFDM_index[i] = priv->pshare->OFDM_index[i] + OFDM_index_mapping[j][offset];
						CCK_index = priv->pshare->CCK_index + OFDM_index_mapping[j][offset];
				}

				if(is2T)
				{
					printk("temp OFDM_A_index=0x%x, OFDM_B_index=0x%x, CCK_index=0x%x\n",
						priv->pshare->OFDM_index[0], priv->pshare->OFDM_index[1], priv->pshare->CCK_index);
				}
				else
				{
					printk("temp OFDM_A_index=0x%x, CCK_index=0x%x\n",
						priv->pshare->OFDM_index[0], priv->pshare->CCK_index);
				}

				for(i = 0; i < rf; i++)
				{
					if(OFDM_index[i] > OFDM_TABLE_SIZE_92D-1)
					{
						OFDM_index[i] = OFDM_TABLE_SIZE_92D-1;
					}
					else if (OFDM_index[i] < OFDM_min_index)
					{
						OFDM_index[i] = OFDM_min_index;
					}
				}

				{
					if(CCK_index > CCK_TABLE_SIZE-1)
						CCK_index = CCK_TABLE_SIZE-1;
					else if (CCK_index < 0)
						CCK_index = 0;
				}

				if(is2T)
				{
					printk("new OFDM_A_index=0x%x, OFDM_B_index=0x%x, CCK_index=0x%x\n",
						OFDM_index[0], OFDM_index[1], CCK_index);
				}
				else
				{
					printk("new OFDM_A_index=0x%x, CCK_index=0x%x\n",
						OFDM_index[0], CCK_index);
				}
			}

			//2 temporarily remove bNOPG
			//Config by SwingTable
			if(priv->pshare->TxPowerTrackControl /*&& !pHalData->bNOPG*/)
			{
				priv->pshare->bDoneTxpower = TRUE;

				//Adujst OFDM Ant_A according to IQK result
				ele_D = (OFDMSwingTable[(unsigned char)OFDM_index[0]] & 0xFFC00000)>>22;
				X = priv->pshare->IQKMatrixRegSetting[Indexforchannel].Value[0][0];
				Y = priv->pshare->IQKMatrixRegSetting[Indexforchannel].Value[0][1];

				if(X != 0)
				{
					if ((X & 0x00000200) != 0)
						X = X | 0xFFFFFC00;
					ele_A = ((X * ele_D)>>8)&0x000003FF;

					//new element C = element D x Y
					if ((Y & 0x00000200) != 0)
						Y = Y | 0xFFFFFC00;
					ele_C = ((Y * ele_D)>>8)&0x000003FF;

					//wirte new elements A, C, D to regC80 and regC94, element B is always 0
					value32 = (ele_D<<22)|((ele_C&0x3F)<<16)|ele_A;
					PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord, value32);

					value32 = (ele_C&0x000003C0)>>6;
					PHY_SetBBReg(priv, rOFDM0_XCTxAFE, bMaskH4Bits, value32);

					value32 = ((X * ele_D)>>7)&0x01;
					PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(24), value32);

				}
				else
				{
					PHY_SetBBReg(priv, rOFDM0_XATxIQImbalance, bMaskDWord, OFDMSwingTable[(unsigned char)OFDM_index[0]]);
					PHY_SetBBReg(priv, rOFDM0_XCTxAFE, bMaskH4Bits, 0x00);
					PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(24), 0x00);
				}

				//printk("TxPwrTracking for path A: X = 0x%x, Y = 0x%x ele_A = 0x%x ele_C = 0x%x ele_D = 0x%x 0xe94 = 0x%x 0xe9c = 0x%x\n",
					//(unsigned int)X, (unsigned int)Y, (unsigned int)ele_A, (unsigned int)ele_C, (unsigned int)ele_D, (unsigned int)X, (unsigned int)Y);

				{
					//Adjust CCK according to IQK result
					if(!priv->pshare->bCCKinCH14){
						RTL_W8(0xa22, CCKSwingTable_Ch1_Ch13[(unsigned char)CCK_index][0]);
						RTL_W8(0xa23, CCKSwingTable_Ch1_Ch13[(unsigned char)CCK_index][1]);
						RTL_W8(0xa24, CCKSwingTable_Ch1_Ch13[(unsigned char)CCK_index][2]);
						RTL_W8(0xa25, CCKSwingTable_Ch1_Ch13[(unsigned char)CCK_index][3]);
						RTL_W8(0xa26, CCKSwingTable_Ch1_Ch13[(unsigned char)CCK_index][4]);
						RTL_W8(0xa27, CCKSwingTable_Ch1_Ch13[(unsigned char)CCK_index][5]);
						RTL_W8(0xa28, CCKSwingTable_Ch1_Ch13[(unsigned char)CCK_index][6]);
						RTL_W8(0xa29, CCKSwingTable_Ch1_Ch13[(unsigned char)CCK_index][7]);
					}
					else{
						RTL_W8(0xa22, CCKSwingTable_Ch14[(unsigned char)CCK_index][0]);
						RTL_W8(0xa23, CCKSwingTable_Ch14[(unsigned char)CCK_index][1]);
						RTL_W8(0xa24, CCKSwingTable_Ch14[(unsigned char)CCK_index][2]);
						RTL_W8(0xa25, CCKSwingTable_Ch14[(unsigned char)CCK_index][3]);
						RTL_W8(0xa26, CCKSwingTable_Ch14[(unsigned char)CCK_index][4]);
						RTL_W8(0xa27, CCKSwingTable_Ch14[(unsigned char)CCK_index][5]);
						RTL_W8(0xa28, CCKSwingTable_Ch14[(unsigned char)CCK_index][6]);
						RTL_W8(0xa29, CCKSwingTable_Ch14[(unsigned char)CCK_index][7]);
					}
				}

				if(is2T)
				{
					ele_D = (OFDMSwingTable[(unsigned char)OFDM_index[1]] & 0xFFC00000)>>22;

					//new element A = element D x X
					X = priv->pshare->IQKMatrixRegSetting[Indexforchannel].Value[0][4];
					Y = priv->pshare->IQKMatrixRegSetting[Indexforchannel].Value[0][5];

					//if(X != 0 && pHalData->CurrentBandType92D == ODM_BAND_ON_2_4G)
					if((X != 0) && (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G))

					{
						if ((X & 0x00000200) != 0)	//consider minus
							X = X | 0xFFFFFC00;
						ele_A = ((X * ele_D)>>8)&0x000003FF;

						//new element C = element D x Y
						if ((Y & 0x00000200) != 0)
							Y = Y | 0xFFFFFC00;
						ele_C = ((Y * ele_D)>>8)&0x00003FF;

						//wirte new elements A, C, D to regC88 and regC9C, element B is always 0
						value32=(ele_D<<22)|((ele_C&0x3F)<<16) |ele_A;
						PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord, value32);

						value32 = (ele_C&0x000003C0)>>6;
						PHY_SetBBReg(priv, rOFDM0_XDTxAFE, bMaskH4Bits, value32);

						value32 = ((X * ele_D)>>7)&0x01;
						PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(28), value32);

					}
					else
					{
						PHY_SetBBReg(priv, rOFDM0_XBTxIQImbalance, bMaskDWord, OFDMSwingTable[(unsigned char)OFDM_index[1]]);
						PHY_SetBBReg(priv, rOFDM0_XDTxAFE, bMaskH4Bits, 0x00);
						PHY_SetBBReg(priv, rOFDM0_ECCAThreshold, BIT(28), 0x00);
					}

					//printk("TxPwrTracking path B: X = 0x%x, Y = 0x%x ele_A = 0x%x ele_C = 0x%x ele_D = 0x%x 0xeb4 = 0x%x 0xebc = 0x%x\n",
						//(unsigned int)X, (unsigned int)Y, (unsigned int)ele_A, (unsigned int)ele_C, (unsigned int)ele_D, (unsigned int)X, (unsigned int)Y);
				}

				printk("TxPwrTracking 0xc80 = 0x%x, 0xc94 = 0x%x RF 0x24 = 0x%x\n\n", PHY_QueryBBReg(priv, 0xc80, bMaskDWord), PHY_QueryBBReg(priv, 0xc94, bMaskDWord), PHY_QueryRFReg(priv, RF_PATH_A, 0x24, bRFRegOffsetMask, 1));
			}
		}


#if 0 //DO NOT do IQK during 88E power tracking
		// if((delta_IQK > pHalData->Delta_IQK) && (pHalData->Delta_IQK != 0))
        if (delta_IQK >= 8) // Delta temperature is equal to or larger than 20 centigrade.
		{
			ODM_ResetIQKResult(priv);

/*
#if(DM_ODM_SUPPORT_TYPE  & ODM_WIN)
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE)
#if USE_WORKITEM
			PlatformAcquireMutex(&pHalData->mxChnlBwControl);
#else
			PlatformAcquireSpinLock(Adapter, RT_CHANNEL_AND_BANDWIDTH_SPINLOCK);
#endif
#elif((DEV_BUS_TYPE == RT_USB_INTERFACE) || (DEV_BUS_TYPE == RT_SDIO_INTERFACE))
			PlatformAcquireMutex(&pHalData->mxChnlBwControl);
#endif
#endif
*/
			priv->pshare->ThermalValue_IQK= ThermalValue;
			PHY_IQCalibrate_8188E(priv, FALSE);

/*
#if(DM_ODM_SUPPORT_TYPE  & ODM_WIN)
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE)
#if USE_WORKITEM
			PlatformReleaseMutex(&pHalData->mxChnlBwControl);
#else
			PlatformReleaseSpinLock(Adapter, RT_CHANNEL_AND_BANDWIDTH_SPINLOCK);
#endif
#elif((DEV_BUS_TYPE == RT_USB_INTERFACE) || (DEV_BUS_TYPE == RT_SDIO_INTERFACE))
			PlatformReleaseMutex(&pHalData->mxChnlBwControl);
#endif
#endif
*/
		}
#endif

		//update thermal meter value
		if(priv->pshare->TxPowerTrackControl)
		{
			//Adapter->HalFunc.SetHalDefVarHandler(Adapter, HAL_DEF_THERMAL_VALUE, &ThermalValue);
			priv->pshare->ThermalValue = ThermalValue;
		}

	}

	//printk("<===dm_TXPowerTrackingCallback_ThermalMeter_8188E\n");

	priv->pshare->TXPowercount = 0;

}




#endif




//3 ============================================================
//3 EDCA Turbo
//3 ============================================================

void init_EDCA_para(struct rtl8192cd_priv *priv, int mode)
{
	static unsigned int slot_time, VO_TXOP, VI_TXOP, sifs_time;

     struct ParaRecord EDCA[4];
#ifdef RTL_MANUAL_EDCA
     //unsigned char acm_bitmap;
#endif

	slot_time = 20;
	sifs_time = 10;

	if (mode & WIRELESS_11N)
		sifs_time = 16;

#ifdef RTL_MANUAL_EDCA
	 if( priv->pmib->dot11QosEntry.ManualEDCA ) {
		 memset(EDCA, 0, 4*sizeof(struct ParaRecord));
		 if( OPMODE & WIFI_AP_STATE )
			 memcpy(EDCA, priv->pmib->dot11QosEntry.AP_manualEDCA, 4*sizeof(struct ParaRecord));
		 else
			 memcpy(EDCA, priv->pmib->dot11QosEntry.STA_manualEDCA, 4*sizeof(struct ParaRecord));


		if ((mode & WIRELESS_11N) ||
			(mode & WIRELESS_11G)) {
			slot_time = 9;
		}


		RTL_W32(EDCA_VO_PARA, (EDCA[VO].TXOPlimit << 16) | (EDCA[VO].ECWmax << 12) | (EDCA[VO].ECWmin << 8) | (sifs_time + EDCA[VO].AIFSN * slot_time));
#ifdef WIFI_WMM
		if (QOS_ENABLE)
			RTL_W32(EDCA_VI_PARA, (EDCA[VI].TXOPlimit << 16) | (EDCA[VI].ECWmax << 12) | (EDCA[VI].ECWmin << 8) | (sifs_time + EDCA[VI].AIFSN * slot_time));
		else
#endif
			RTL_W32(EDCA_VI_PARA, (EDCA[BE].TXOPlimit << 16) | (EDCA[BE].ECWmax << 12) | (EDCA[BE].ECWmin << 8) | (sifs_time + EDCA[VI].AIFSN * slot_time));

		RTL_W32(EDCA_BE_PARA, (EDCA[BE].TXOPlimit << 16) | (EDCA[BE].ECWmax << 12) | (EDCA[BE].ECWmin << 8) | (sifs_time + EDCA[BE].AIFSN * slot_time));

		RTL_W32(EDCA_BK_PARA, (EDCA[BK].TXOPlimit << 16) | (EDCA[BK].ECWmax << 12) | (EDCA[BK].ECWmin << 8) | (sifs_time + EDCA[BK].AIFSN * slot_time));
	}else
#endif //RTL_MANUAL_EDCA
	{
		 memset(EDCA, 0, 4*sizeof(struct ParaRecord));
		 /* copy BE, BK from static data */
		 if( OPMODE & WIFI_AP_STATE )
				 memcpy(EDCA, rtl_ap_EDCA, 2*sizeof(struct ParaRecord));
		 else
				 memcpy(EDCA, rtl_sta_EDCA, 2*sizeof(struct ParaRecord));

		 /* VI, VO apply settings in AG by default */
		 if( OPMODE & WIFI_AP_STATE )
				 memcpy(&EDCA[2], &rtl_ap_EDCA[VI_AG], 2*sizeof(struct ParaRecord));
		 else
				 memcpy(&EDCA[2], &rtl_sta_EDCA[VI_AG], 2*sizeof(struct ParaRecord));

		 if ((mode & WIRELESS_11N) ||
				 (mode & WIRELESS_11G)) {
				 slot_time = 9;
		 } else {
				 /* replace with settings in B */
				 if( OPMODE & WIFI_AP_STATE )
						 memcpy(&EDCA[2], &rtl_ap_EDCA[VI], 2*sizeof(struct ParaRecord));
				 else
						 memcpy(&EDCA[2], &rtl_sta_EDCA[VI], 2*sizeof(struct ParaRecord));
		 }
		 VO_TXOP = EDCA[VO].TXOPlimit;
		 VI_TXOP = EDCA[VI].TXOPlimit;

		 RTL_W32(EDCA_VO_PARA, (VO_TXOP << 16) | (EDCA[VO].ECWmax << 12) | (EDCA[VO].ECWmin << 8) | (sifs_time + EDCA[VO].AIFSN * slot_time));
#ifdef WIFI_WMM
		 if (QOS_ENABLE)
				 RTL_W32(EDCA_VI_PARA, (VI_TXOP << 16) | (EDCA[VI].ECWmax << 12) | (EDCA[VI].ECWmin << 8) | (sifs_time + EDCA[VI].AIFSN * slot_time));
		 else
#endif
				 RTL_W32(EDCA_VI_PARA, (EDCA[BK].ECWmax << 12) | (EDCA[BK].ECWmin << 8) | (sifs_time + EDCA[VI].AIFSN * slot_time));

		 RTL_W32(EDCA_BE_PARA, ((EDCA[BE].ECWmax) << 12) | (EDCA[BE].ECWmin << 8) | (sifs_time + EDCA[BE].AIFSN * slot_time));
		 RTL_W32(EDCA_BK_PARA, (EDCA[BK].ECWmax << 12) | (EDCA[BK].ECWmin << 8) | (sifs_time + EDCA[BK].AIFSN * slot_time));


		RTL_W8(ACMHWCTRL, 0x00);
	}

	priv->pshare->iot_mode_enable = 0;
	if (priv->pshare->rf_ft_var.wifi_beq_iot)
		priv->pshare->iot_mode_VI_exist = 0;
	priv->pshare->iot_mode_VO_exist = 0;

#ifdef WMM_VIBE_PRI
	priv->pshare->iot_mode_BE_exist = 0;
#endif
#ifdef WMM_BEBK_PRI
	priv->pshare->iot_mode_BK_exist = 0;
#endif
#ifdef LOW_TP_TXOP
	priv->pshare->BE_cwmax_enhance = 0;
#endif
}

void choose_IOT_main_sta(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int i, assoc_num = GET_ROOT(priv)->assoc_num;
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable){
			for (i=0; i<RTL8192CD_NUM_VWLAN; ++i)
				assoc_num += GET_ROOT(priv)->pvap_priv[i]-> assoc_num;
		}
		if (IS_DRV_OPEN(GET_VXD_PRIV(GET_ROOT(priv))))
			assoc_num += GET_VXD_PRIV(GET_ROOT(priv))-> assoc_num;

	if (priv->pshare->rf_ft_var.low_tp_no_aggr) {
		unsigned long sta_tp = (pstat->current_tx_bytes + pstat->current_rx_bytes) >> 17;
		if  (!pstat->low_tp_disable_ampdu && sta_tp <= 2)
			pstat->low_tp_disable_ampdu = 1;
		else if (pstat->low_tp_disable_ampdu && sta_tp >= 5)
			pstat->low_tp_disable_ampdu = 0;
	}

	if ((GET_ROOT(priv)->up_time % 2) == 0) {
		unsigned int tx_2s_avg = 0;
		unsigned int rx_2s_avg = 0;
		int i=0, aggReady=0;
		unsigned long total_sum = (priv->pshare->current_tx_bytes+priv->pshare->current_rx_bytes);

		pstat->current_tx_bytes += pstat->tx_byte_cnt;
		pstat->current_rx_bytes += pstat->rx_byte_cnt;

		if (total_sum != 0) {
			if (total_sum <= 1000000) {
				tx_2s_avg = (unsigned int)((pstat->current_tx_bytes*100) / total_sum);
				rx_2s_avg = (unsigned int)((pstat->current_rx_bytes*100) / total_sum);
			} else {
				tx_2s_avg = (unsigned int)(pstat->current_tx_bytes / (total_sum / 100));
				rx_2s_avg = (unsigned int)(pstat->current_rx_bytes / (total_sum / 100));
			}
		}

		for(i=0; i<8; i++)
			aggReady += (pstat->ADDBA_ready[i]);

		if( (pstat->IOTPeer==HT_IOT_PEER_INTEL)
			|| (pstat->ht_cap_len && aggReady))	 {

			if ((assoc_num==1) || (tx_2s_avg + rx_2s_avg >= 25)) {
				priv->pshare->highTP_found_pstat = pstat;
			}	/*this STA's TXRX packet very close AP's total TXRX packet then let it as highTP_found_pstat*/
			else if(RTL_ABS((pstat->current_tx_bytes + pstat->current_rx_bytes) , total_sum)<50){
				priv->pshare->highTP_found_pstat = pstat;
			}
#ifdef CLIENT_MODE
			if (OPMODE & WIFI_STATION_STATE) {
//				if(pstat->is_ralink_sta && ((tx_2s_avg + rx_2s_avg) >= 45))
				if(pstat->IOTPeer == HT_IOT_PEER_RALINK && ((tx_2s_avg + rx_2s_avg) >= 45))
					priv->pshare->highTP_found_pstat = pstat;
			}
#endif
		}
	}
	else {
		pstat->current_tx_bytes = pstat->tx_byte_cnt;
		pstat->current_rx_bytes = pstat->rx_byte_cnt;
	}
}


void rxBB_dm(struct rtl8192cd_priv *priv)
{
	if ((priv->up_time % 3) == 1) {
		if (priv->pshare->rssi_min != 0xff) {
			if (priv->pshare->rf_ft_var.dig_enable) {
				// for DIG checking
				check_DIG_by_rssi(priv, priv->pshare->rssi_min);
			}
//#ifdef INTERFERENCE_CONTROL
			if (priv->pshare->rf_ft_var.nbi_filter_enable) {
				check_NBI_by_rssi(priv, priv->pshare->rssi_min);
			}
//#endif
		}

#ifdef MP_TEST
		if (!((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific))
#endif
		{
#ifdef HIGH_POWER_EXT_LNA
			if (!priv->pshare->rf_ft_var.use_ext_lna)
#endif
				CCK_CCA_dynamic_enhance(priv, priv->pshare->rssi_min);
		}
	}
	if (priv->pshare->rf_ft_var.adaptivity_enable){
		unsigned char IGI;
		IGI = RTL_R8(0xc50);
		rtl8192cd_Adaptivity(priv,IGI);
	}
}

/*
 * IOT related functions
 */
void IOT_engine(struct rtl8192cd_priv *priv)
{
#ifdef WIFI_WMM
	unsigned int switch_turbo = 0;
#endif
	int i;
	int assoc_num = priv->assoc_num;
	struct stat_info *pstat = priv->pshare->highTP_found_pstat;

	if(pstat) {
		if((pstat->tx_avarage + pstat->rx_avarage) < (1<<17))		// 1M bps
			pstat = NULL;
	}
#if defined(RTL_MANUAL_EDCA) && defined(WIFI_WMM)
	if(priv->pmib->dot11QosEntry.ManualEDCA)
		return ;
#endif

	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable){
		for (i=0; i<RTL8192CD_NUM_VWLAN; ++i)
			assoc_num += GET_ROOT(priv)->pvap_priv[i]-> assoc_num;
	}
	if (IS_DRV_OPEN(GET_VXD_PRIV(GET_ROOT(priv))))
		assoc_num += GET_VXD_PRIV(GET_ROOT(priv))-> assoc_num;

#ifdef WIFI_WMM
	if (QOS_ENABLE) {
		if (!priv->pmib->dot11OperationEntry.wifi_specific ||
			((OPMODE & WIFI_AP_STATE) && (priv->pmib->dot11OperationEntry.wifi_specific))) {
			if (priv->pshare->iot_mode_enable &&
				((priv->pshare->phw->VO_pkt_count > 50) ||
				(priv->pshare->phw->VI_pkt_count > 50) ||
				(priv->pshare->phw->BK_pkt_count > 50))) {
				priv->pshare->iot_mode_enable = 0;
				switch_turbo++;
			} else if ((!priv->pshare->iot_mode_enable) &&
				((priv->pshare->phw->VO_pkt_count < 50) &&
				(priv->pshare->phw->VI_pkt_count < 50) &&
				(priv->pshare->phw->BK_pkt_count < 50))) {
					priv->pshare->iot_mode_enable++;
					switch_turbo++;
			}
		}

		if ((OPMODE & WIFI_AP_STATE) && priv->pmib->dot11OperationEntry.wifi_specific) {
			if (!priv->pshare->iot_mode_VO_exist && (priv->pshare->phw->VO_pkt_count > 50)) {
				priv->pshare->iot_mode_VO_exist++;
				switch_turbo++;
			} else if (priv->pshare->iot_mode_VO_exist && (priv->pshare->phw->VO_pkt_count < 50)) {
				priv->pshare->iot_mode_VO_exist = 0;
				switch_turbo++;
			}

#ifdef WMM_VIBE_PRI
			if (priv->pshare->iot_mode_VO_exist) {
				//printk("[%s %d] BE_pkt_count=%d\n", __FUNCTION__, __LINE__, priv->pshare->phw->BE_pkt_count);
				if (!priv->pshare->iot_mode_BE_exist && (priv->pshare->phw->BE_pkt_count > 250)) {
					priv->pshare->iot_mode_BE_exist++;
					switch_turbo++;
				} else if (priv->pshare->iot_mode_BE_exist && (priv->pshare->phw->BE_pkt_count < 250)) {
					priv->pshare->iot_mode_BE_exist = 0;
					switch_turbo++;
				}
			}
#endif

#ifdef WMM_BEBK_PRI
			if (priv->pshare->phw->BE_pkt_count) {
				//printk("[%s %d] BK_pkt_count=%d\n", __FUNCTION__, __LINE__, priv->pshare->phw->BK_pkt_count);
				if (!priv->pshare->iot_mode_BK_exist && (priv->pshare->phw->BK_pkt_count > 250)) {
					priv->pshare->iot_mode_BK_exist++;
					switch_turbo++;
					EDEBUG("switch_turbo++\n");
				} else if (priv->pshare->iot_mode_BE_exist && (priv->pshare->phw->BK_pkt_count < 250)) {
					priv->pshare->iot_mode_BK_exist = 0;
					switch_turbo++;
					EDEBUG("switch_turbo++\n");
				}
			}
#endif

			if (priv->pshare->rf_ft_var.wifi_beq_iot) {
				if (!priv->pshare->iot_mode_VI_exist && (priv->pshare->phw->VI_rx_pkt_count > 50)) {
					priv->pshare->iot_mode_VI_exist++;
					switch_turbo++;
				} else if (priv->pshare->iot_mode_VI_exist && (priv->pshare->phw->VI_rx_pkt_count < 50)) {
					priv->pshare->iot_mode_VI_exist = 0;
					switch_turbo++;
				}
			}

		}

#ifdef CLIENT_MODE
        if ((OPMODE & WIFI_STATION_STATE) && (priv->pmib->dot11OperationEntry.wifi_specific))
        {
            if (priv->pshare->iot_mode_enable &&
                (((priv->pshare->phw->VO_pkt_count > 50) ||
                 (priv->pshare->phw->VI_pkt_count > 50) ||
                 (priv->pshare->phw->BK_pkt_count > 50)) ||
                 (pstat && (!pstat->ADDBA_ready[0]) & (!pstat->ADDBA_ready[3]))))
            {
                priv->pshare->iot_mode_enable = 0;
                switch_turbo++;
            }
            else if ((!priv->pshare->iot_mode_enable) &&
                (((priv->pshare->phw->VO_pkt_count < 50) &&
                 (priv->pshare->phw->VI_pkt_count < 50) &&
                 (priv->pshare->phw->BK_pkt_count < 50)) &&
                 (pstat && (pstat->ADDBA_ready[0] | pstat->ADDBA_ready[3]))))
            {
                priv->pshare->iot_mode_enable++;
                switch_turbo++;
            }
        }
#endif

		priv->pshare->phw->VO_pkt_count = 0;
		priv->pshare->phw->VI_pkt_count = 0;
		if (priv->pshare->rf_ft_var.wifi_beq_iot)
			priv->pshare->phw->VI_rx_pkt_count = 0;
		priv->pshare->phw->BK_pkt_count = 0;
#ifdef WMM_VIBE_PRI
		priv->pshare->phw->BE_pkt_count = 0;
#endif
	}
#endif

		if ((priv->up_time % 2) == 0) {
			/*
			 * decide EDCA content for different chip vendor
			 */
#ifdef WIFI_WMM
		if (QOS_ENABLE && (!priv->pmib->dot11OperationEntry.wifi_specific ||
			((OPMODE & WIFI_AP_STATE) && (priv->pmib->dot11OperationEntry.wifi_specific == 2))
#ifdef CLIENT_MODE
            || ((OPMODE & WIFI_STATION_STATE) && (priv->pmib->dot11OperationEntry.wifi_specific == 2))
#endif
			)) {
			if (pstat && pstat->rssi >= priv->pshare->rf_ft_var.txop_enlarge_upper) {
#ifdef LOW_TP_TXOP


				if (pstat->IOTPeer==HT_IOT_PEER_INTEL)

				{

					if (priv->pshare->txop_enlarge != 0xe) {
						priv->pshare->txop_enlarge = 0xe;
						if (priv->pshare->iot_mode_enable)
							switch_turbo++;
					}
				} else if (priv->pshare->txop_enlarge != 2) {
					priv->pshare->txop_enlarge = 2;
					if (priv->pshare->iot_mode_enable)
						switch_turbo++;
				}
#else //LOW_TP_TXOP

				if (priv->pshare->txop_enlarge != 2) {


				{
					if (pstat->IOTPeer==HT_IOT_PEER_INTEL)
						priv->pshare->txop_enlarge = 0xe;
					else if (pstat->IOTPeer==HT_IOT_PEER_RALINK)
						priv->pshare->txop_enlarge = 0xd;
					else
						priv->pshare->txop_enlarge = 2;

				}

					if (priv->pshare->iot_mode_enable)
						switch_turbo++;
				}
#endif //LOW_TP_TXOP
			} else if (!pstat || pstat->rssi < priv->pshare->rf_ft_var.txop_enlarge_lower) {
				if (priv->pshare->txop_enlarge) {
					priv->pshare->txop_enlarge = 0;
					if (priv->pshare->iot_mode_enable)
						switch_turbo++;
				}
			}
#ifdef LOW_TP_TXOP
			// for Intel IOT, need to enlarge CW MAX from 6 to 10


			if (pstat && pstat->IOTPeer==HT_IOT_PEER_INTEL && (((pstat->tx_avarage+pstat->rx_avarage)>>10) <
				priv->pshare->rf_ft_var.cwmax_enhance_thd))

			{
				if (!priv->pshare->BE_cwmax_enhance && priv->pshare->iot_mode_enable) {
					priv->pshare->BE_cwmax_enhance = 1;
					switch_turbo++;
				}
			} else {
				if (priv->pshare->BE_cwmax_enhance) {
					priv->pshare->BE_cwmax_enhance = 0;
					switch_turbo++;
				}
			}
#endif
		}
#endif

		priv->pshare->current_tx_bytes = 0;
		priv->pshare->current_rx_bytes = 0;
	}

#ifdef SW_TX_QUEUE
	if ((assoc_num > 1) && (AMPDU_ENABLE))
	{
		unsigned int total_tp = (unsigned int)(priv->ext_stats.tx_avarage>>17)+(unsigned int)(priv->ext_stats.rx_avarage>>17);
		if(total_tp > 0) {
		        if (((unsigned int)(priv->ext_stats.tx_avarage>>17) * 100/total_tp) > 30)
	        	{
	       			if (priv->pshare->swq_txmac_chg >= priv->pshare->rf_ft_var.swq_en_highthd)
		        	{
					if ((priv->pshare->swq_en == 0))
					{
						switch_turbo++;
						if (priv->pshare->txop_enlarge == 0)
							priv->pshare->txop_enlarge = 2;
						priv->pshare->swq_en = 1;
						priv->pshare->swqen_keeptime = priv->up_time;
					}
					else
					{
						if ((switch_turbo > 0) && (priv->pshare->txop_enlarge == 0) && (priv->pshare->iot_mode_enable != 0))
						{
							priv->pshare->txop_enlarge = 2;
							switch_turbo--;
						}
					}
				}
				else if(priv->pshare->swq_txmac_chg <= priv->pshare->rf_ft_var.swq_dis_lowthd)
				{
					priv->pshare->swq_en = 0;
					priv->pshare->swqen_keeptime = 0;
				}
				else if ((priv->pshare->swq_en == 1) && (switch_turbo > 0) && (priv->pshare->txop_enlarge == 0) && (priv->pshare->iot_mode_enable != 0))
				{
					priv->pshare->txop_enlarge = 2;
					switch_turbo--;
				}
			}
			else if (((unsigned int)(priv->ext_stats.tx_avarage>>17) * 100/total_tp) < 20)
			{
				priv->pshare->swq_en = 0;
				priv->pshare->swqen_keeptime = 0;
			}
		}
		//debug msg
		//printk("swq=%d,sw=%d,en=%d,mode=%d\n", priv->pshare->swq_en, switch_turbo, priv->pshare->txop_enlarge, priv->pshare->iot_mode_enable);
    }
#if 1//defined(CONFIG_RTL_819XD)
    else if( (assoc_num == 1) && (priv->up_time % 2)== 0 && (AMPDU_ENABLE) ) {
        //if (pstat) {
//        if ((pstat) && pstat->is_intel_sta) {
        if ((pstat) && pstat->IOTPeer == HT_IOT_PEER_INTEL) {
//			int en_thd = 14417920;
            //if ((priv->pshare->swq_en == 0) && (pstat->current_tx_bytes > en_thd) && (pstat->current_rx_bytes > en_thd) )  { //50Mbps
            	if ((pstat->current_tx_bytes > 14417920) && (priv->pshare->swq_en == 0)) { // && (pstat->current_rx_bytes > 14417920) && (priv->pshare->swq_en == 0))  { //55Mbps
                	priv->pshare->swq_en = 1;
			priv->pshare->swqen_keeptime = priv->up_time;
		}
        	//else if ((priv->pshare->swq_en == 1) && ((pstat->tx_avarage < 4587520) || (pstat->rx_avarage < 4587520))) { //35Mbps
		else if ((pstat->tx_avarage < 9175040) && (priv->pshare->swq_en == 1)) { //35Mbps
			priv->pshare->swq_en = 0;
			priv->pshare->swqen_keeptime = 0;
            	}
        }
        else {
            priv->pshare->swq_en = 0;
			priv->pshare->swqen_keeptime = 0;
        }
    }
#endif
#endif

#ifdef WIFI_WMM
#ifdef LOW_TP_TXOP
	if ((!priv->pmib->dot11OperationEntry.wifi_specific || (priv->pmib->dot11OperationEntry.wifi_specific == 2))
		&& QOS_ENABLE) {

		if (switch_turbo || priv->pshare->rf_ft_var.low_tp_txop) {
			unsigned int thd_tp;
			unsigned char under_thd;
			unsigned int curr_tp;

			if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11N | WIRELESS_11G))
			{
				// Determine the upper bound throughput threshold.
				if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
					if (priv->assoc_num && priv->assoc_num != priv->pshare->ht_sta_num)
						thd_tp = priv->pshare->rf_ft_var.low_tp_txop_thd_g;
					else
						thd_tp = priv->pshare->rf_ft_var.low_tp_txop_thd_n;
				}
				else
					thd_tp = priv->pshare->rf_ft_var.low_tp_txop_thd_g;

				// Determine to close txop.
				if(pstat && pstat->expire_to)
				{
					struct rtl8192cd_priv *tmppriv;
					struct aid_obj *aidarray;
					aidarray = container_of(pstat, struct aid_obj, station);
					tmppriv = aidarray->priv;

					curr_tp = (unsigned int)(tmppriv->ext_stats.tx_avarage>>17) + (unsigned int)(tmppriv->ext_stats.rx_avarage>>17);
				}
				else
				curr_tp = (unsigned int)(priv->ext_stats.tx_avarage>>17) + (unsigned int)(priv->ext_stats.rx_avarage>>17);
				if (curr_tp <= thd_tp && curr_tp >= priv->pshare->rf_ft_var.low_tp_txop_thd_low)
					under_thd = 1;
				else
					under_thd = 0;
			}
			else
			{
				under_thd = 0;
			}

			if (switch_turbo) {
				priv->pshare->rf_ft_var.low_tp_txop_close = under_thd;
				priv->pshare->rf_ft_var.low_tp_txop_count = 0;
			} else if (priv->pshare->iot_mode_enable && (priv->pshare->rf_ft_var.low_tp_txop_close != under_thd)) {
				priv->pshare->rf_ft_var.low_tp_txop_count++;
				if (priv->pshare->rf_ft_var.low_tp_txop_close) {
					priv->pshare->rf_ft_var.low_tp_txop_count = priv->pshare->rf_ft_var.low_tp_txop_delay;
				}
				if (priv->pshare->rf_ft_var.low_tp_txop_count == priv->pshare->rf_ft_var.low_tp_txop_delay) {
					priv->pshare->rf_ft_var.low_tp_txop_count = 0;
					priv->pshare->rf_ft_var.low_tp_txop_close = under_thd;
					switch_turbo++;
				}
			} else {
				priv->pshare->rf_ft_var.low_tp_txop_count = 0;
			}
		}
	}
#endif

	if (switch_turbo)
		IOT_EDCA_switch(priv, priv->pmib->dot11BssType.net_work_type, priv->pshare->iot_mode_enable);
#endif
}


#ifdef WIFI_WMM
void IOT_EDCA_switch(struct rtl8192cd_priv *priv, int mode, char enable)
{
	unsigned int slot_time = 20, sifs_time = 10, BE_TXOP = 47, VI_TXOP = 94;
	unsigned int vi_cw_max = 4, vi_cw_min = 3, vi_aifs;

	if (!(!priv->pmib->dot11OperationEntry.wifi_specific ||
		((OPMODE & WIFI_AP_STATE) && (priv->pmib->dot11OperationEntry.wifi_specific))
#ifdef CLIENT_MODE
        || ((OPMODE & WIFI_STATION_STATE) && (priv->pmib->dot11OperationEntry.wifi_specific))
#endif
		))
		return;

	if ((mode & WIRELESS_11N) && (priv->pshare->ht_sta_num
		))
		sifs_time = 16;

	if (mode & (WIRELESS_11G | WIRELESS_11N)) {
		slot_time = 9;
	} else {
		BE_TXOP = 94;
		VI_TXOP = 188;
	}

#if 0 //defined(CONFIG_RTL_8196D) || defined(CONFIG_RTL_8196E) || (defined(CONFIG_RTL_8197D) && !defined(CONFIG_PORT0_EXT_GIGA))
	if (priv->pshare->is_40m_bw)
	{
		BE_TXOP = 23;
	}
#endif

	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->dot11OperationEntry.wifi_specific) {
		if (priv->pshare->iot_mode_VO_exist) {
#ifdef WMM_VIBE_PRI
			if (priv->pshare->iot_mode_BE_exist) {
				vi_cw_max = 5;
				vi_cw_min = 3;
				vi_aifs = (sifs_time + ((OPMODE & WIFI_AP_STATE)?1:2) * slot_time);
			} else
#endif
			{
				vi_cw_max = 6;
				vi_cw_min = 4;
				vi_aifs = 0x2b;
			}
		} else {
			vi_aifs = (sifs_time + ((OPMODE & WIFI_AP_STATE)?1:2) * slot_time);
		}

		RTL_W32(EDCA_VI_PARA, ((VI_TXOP*(1-priv->pshare->iot_mode_VO_exist)) << 16)
			| (vi_cw_max << 12) | (vi_cw_min << 8) | vi_aifs);

#ifdef WMM_BEBK_PRI
#endif
	}

	if (!enable || (priv->pshare->rf_ft_var.wifi_beq_iot && priv->pshare->iot_mode_VI_exist)) {
		if (priv->pshare->rf_ft_var.wifi_beq_iot && priv->pshare->iot_mode_VI_exist) {
				RTL_W32(EDCA_BE_PARA, (10 << 12) | (4 << 8) | 0x4f);
		} else {
			RTL_W32(EDCA_BE_PARA, (((OPMODE & WIFI_AP_STATE)?6:10) << 12) | (4 << 8)
					| (sifs_time + 3 * slot_time));
		}
		RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) | DIS_TXOP_CFE);
	} else {
#ifdef LOW_TP_TXOP
		int txop;
		unsigned int cw_max;
		unsigned int txop_close;

		cw_max = ((priv->pshare->BE_cwmax_enhance) ? 10 : 6);
		txop_close = ((priv->pshare->rf_ft_var.low_tp_txop && priv->pshare->rf_ft_var.low_tp_txop_close) ? 1 : 0);
		txop = (txop_close ? 0 : (BE_TXOP*2));
#endif
		if (priv->pshare->ht_sta_num
			) {
/*
			if (priv->pshare->txop_enlarge == 0xf) {
				// is 8192S client
				RTL_W32(EDCA_BE_PARA, ((BE_TXOP*2) << 16) |
							(6 << 12) | (4 << 8) | (sifs_time + slot_time+ 0xf)); // 0xf is 92s circuit delay
				priv->pshare->txop_enlarge = 2;
			}
			else
*/
			if (priv->pshare->txop_enlarge == 0xe) {
#ifndef LOW_TP_TXOP
				// is intel client, use a different edca value
#if 0 //defined(CONFIG_RTL_8196D) || defined(CONFIG_RTL_8196E) || (defined(CONFIG_RTL_8197D) && !defined(CONFIG_PORT0_EXT_GIGA))
				RTL_W32(EDCA_BE_PARA, (BE_TXOP*2 << 16) | (6 << 12) | (5 << 8) | 0x1f);
#else
				//RTL_W32(EDCA_BE_PARA, (BE_TXOP*2 << 16) | (6 << 12) | (4 << 8) | 0x1f);
				if (get_rf_mimo_mode(priv) == MIMO_1T1R)
					RTL_W32(EDCA_BE_PARA, (BE_TXOP*2 << 16) | (6 << 12) | (5 << 8) | 0x1f);
				else
					RTL_W32(EDCA_BE_PARA, (BE_TXOP*2 << 16) | (8 << 12) | (5 << 8) | 0x1f);

				RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) & ~(DIS_TXOP_CFE));
#endif
				priv->pshare->txop_enlarge = 2;
			} else if (priv->pshare->txop_enlarge == 0xd) {
				// is intel ralink, use a different edca value
				RTL_W32(EDCA_BE_PARA, (BE_TXOP*2 << 16) | (4 << 12) | (3 << 8) | 0x19);
				priv->pshare->txop_enlarge = 2;
			} else {
				if (priv->pshare->txop_enlarge == 0)
					RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) | DIS_TXOP_CFE);

				if (get_rf_mimo_mode(priv) == MIMO_2T2R)
#if 0 //defined(CONFIG_RTL_8196D) || defined(CONFIG_RTL_8196E) || (defined(CONFIG_RTL_8197D) && !defined(CONFIG_PORT0_EXT_GIGA))
					RTL_W32(EDCA_BE_PARA, ((BE_TXOP*priv->pshare->txop_enlarge) << 16) |
                            (6 << 12) | (5 << 8) | (sifs_time + 3 * slot_time));
#else
					RTL_W32(EDCA_BE_PARA, ((BE_TXOP*priv->pshare->txop_enlarge) << 16) |
							(6 << 12) | (4 << 8) | (sifs_time + 3 * slot_time));
#endif
				else
#if 0 //defined(CONFIG_RTL_8196D) || defined(CONFIG_RTL_8196E) || (defined(CONFIG_RTL_8197D) && !defined(CONFIG_PORT0_EXT_GIGA))
					RTL_W32(EDCA_BE_PARA, ((BE_TXOP*priv->pshare->txop_enlarge) << 16) |
                            (5 << 12) | (4 << 8) | (sifs_time + 2 * slot_time));
#else
					RTL_W32(EDCA_BE_PARA, ((BE_TXOP*priv->pshare->txop_enlarge) << 16) |
							(5 << 12) | (3 << 8) | (sifs_time + 2 * slot_time));
#endif

#else
				// is intel client, use a different edca value
				if (get_rf_mimo_mode(priv) == MIMO_1T1R)
					RTL_W32(EDCA_BE_PARA, (BE_TXOP*2 << 16) | (6 << 12) | (5 << 8) | 0x1f);
				else
					RTL_W32(EDCA_BE_PARA, (txop << 16) | (cw_max << 12) | (4 << 8) | 0x1f);
			} else {
				txop = (txop_close ? 0: (BE_TXOP*priv->pshare->txop_enlarge));

				if (get_rf_mimo_mode(priv) == MIMO_2T2R)
					RTL_W32(EDCA_BE_PARA, (txop << 16) | (cw_max << 12) | (4 << 8) | (sifs_time + 3 * slot_time));
				else
					RTL_W32(EDCA_BE_PARA, (txop << 16) | (((priv->pshare->BE_cwmax_enhance) ? 10 : 5) << 12) |
						(3 << 8) | (sifs_time + 2 * slot_time));
#endif
			}
		} else {
#ifdef LOW_TP_TXOP
			RTL_W32(EDCA_BE_PARA, (txop << 16) | (cw_max << 12) | (4 << 8) | (sifs_time + 3 * slot_time));
#else
#if defined(CONFIG_RTL_8196D) || defined(CONFIG_RTL_8197DL) || defined(CONFIG_RTL_8196E) || (defined(CONFIG_RTL_8197D) && !defined(CONFIG_PORT0_EXT_GIGA))
			RTL_W32(EDCA_BE_PARA, (BE_TXOP*2 << 16) | (6 << 12) | (5 << 8) | (sifs_time + 3 * slot_time));
#else
			RTL_W32(EDCA_BE_PARA, (BE_TXOP*2 << 16) | (6 << 12) | (4 << 8) | (sifs_time + 3 * slot_time));
#endif
			if (priv->pshare->txop_enlarge == 0xe)
				RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) & ~(DIS_TXOP_CFE));
			else
				RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) | DIS_TXOP_CFE);
#endif
		}
/*
		if (priv->pmib->dot11OperationEntry.wifi_specific == 2) {
			RTL_W16(NAV_PROT_LEN, 0x01C0);
			RTL_W8(CFEND_TH, 0xFF);
			set_fw_reg(priv, 0xfd000ab0, 0, 0);
		}
*/
	}
}

//end of IOT_EDCA_switch
//========================================================================================================

#if 0
void check_NAV_prot_len(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned int disassoc)
{
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && pstat
		&& pstat->ht_cap_len && pstat->is_intel_sta) {
		if (!disassoc && (pstat->MIMO_ps & _HT_MIMO_PS_DYNAMIC_)) {
            setSTABitMap(&priv->pshare->mimo_ps_dynamic_sta, pstat->aid);
		} else {
            clearSTABitMap(&priv->pshare->mimo_ps_dynamic_sta, pstat->aid);
		}

		{
            if (orSTABitMap(&priv->pshare->mimo_ps_dynamic_sta) {
				RTL_W8(NAV_PROT_LEN, 0x40);
			} else {
				RTL_W8(NAV_PROT_LEN, 0x20);
			}
		}
	}
}
#endif
#endif


//3 ============================================================
//3 FA statistic functions
//3 ============================================================
#if 0

#if !defined(CONFIG_RTL_NEW_AUTOCH)
static
#endif
void reset_FA_reg(struct rtl8192cd_priv *priv)
{
#if !defined(CONFIG_RTL_NEW_AUTOCH)
	unsigned char value8;

	value8 = RTL_R8(0xd03);
	RTL_W8(0xd03, value8 | 0x08);	// regD00[27]=1 to reset these OFDM FA counters
	value8 = RTL_R8(0xd03);
	RTL_W8(0xd03, value8 & 0xF7);	// regD00[27]=0 to start counting
	value8 = RTL_R8(0xa2d);
	RTL_W8(0xa2d, value8 & 0x3F);	// regA2D[7:6]=00 to disable counting
	value8 = RTL_R8(0xa2d);
	RTL_W8(0xa2d, value8 | 0x80);	// regA2D[7:6]=10 to enable counting
#else
	/* cck CCA */
	PHY_SetBBReg(priv, 0xa2c, BIT(13) | BIT(12), 0);
	PHY_SetBBReg(priv, 0xa2c, BIT(13) | BIT(12), 2);
	/* cck FA*/
	PHY_SetBBReg(priv, 0xa2c, BIT(15) | BIT(14), 0);
	PHY_SetBBReg(priv, 0xa2c, BIT(15) | BIT(14), 2);
	/* ofdm */
	PHY_SetBBReg(priv, 0xd00, BIT(27), 1);
	PHY_SetBBReg(priv, 0xd00, BIT(27), 0);
#endif


}

#if defined(CONFIG_RTL_NEW_AUTOCH)
void hold_CCA_FA_counter(struct rtl8192cd_priv *priv)
{
	/* hold cck CCA & FA counter */
	PHY_SetBBReg(priv, 0xa2c, BIT(12), 1);
	PHY_SetBBReg(priv, 0xa2c, BIT(14), 1);

	/* hold ofdm CCA & FA counter */
	PHY_SetBBReg(priv, 0xc00, BIT(31), 1);
	PHY_SetBBReg(priv, 0xd00, BIT(31), 1);
}

void release_CCA_FA_counter(struct rtl8192cd_priv *priv)
{
	/* release cck CCA & FA counter */
	PHY_SetBBReg(priv, 0xa2c, BIT(12), 0);
	PHY_SetBBReg(priv, 0xa2c, BIT(14), 0);

	/* release ofdm CCA & FA counter */
	PHY_SetBBReg(priv, 0xc00, BIT(31), 0);
	PHY_SetBBReg(priv, 0xd00, BIT(31), 0);

}


void _FA_statistic(struct rtl8192cd_priv *priv)
{
	// read OFDM FA counters
	priv->pshare->ofdm_FA_cnt1 = RTL_R16(0xda2);
	priv->pshare->ofdm_FA_cnt2 = RTL_R16(0xda4);
	priv->pshare->ofdm_FA_cnt3 = RTL_R16(0xda6);
	priv->pshare->ofdm_FA_cnt4 = RTL_R16(0xda8);

	priv->pshare->cck_FA_cnt = (RTL_R8(0xa5b) << 8) + RTL_R8(0xa5c);

#ifdef INTERFERENCE_CONTROL
	priv->pshare->ofdm_FA_total_cnt = (unsigned int) priv->pshare->ofdm_FA_cnt1 +
			priv->pshare->ofdm_FA_cnt2 + priv->pshare->ofdm_FA_cnt3 +
			priv->pshare->ofdm_FA_cnt4 + RTL_R16(0xcf0) + RTL_R16(0xcf2);

	priv->pshare->FA_total_cnt = priv->pshare->ofdm_FA_total_cnt + priv->pshare->cck_FA_cnt;
#else
	priv->pshare->FA_total_cnt = priv->pshare->ofdm_FA_cnt1 + priv->pshare->ofdm_FA_cnt2 +
	                             priv->pshare->ofdm_FA_cnt3 + priv->pshare->ofdm_FA_cnt4 +
	                             priv->pshare->cck_FA_cnt + RTL_R16(0xcf0) + RTL_R16(0xcf2);
#endif
}
#endif

void FA_statistic(struct rtl8192cd_priv *priv)
{


#if !defined(CONFIG_RTL_NEW_AUTOCH)
	signed char value8;

	// read OFDM FA counters
	priv->pshare->ofdm_FA_cnt1 = RTL_R16(0xda2);
	priv->pshare->ofdm_FA_cnt2 = RTL_R16(0xda4);
	priv->pshare->ofdm_FA_cnt3 = RTL_R16(0xda6);
	priv->pshare->ofdm_FA_cnt4 = RTL_R16(0xda8);

	// read the CCK FA counters
	value8 = RTL_R8(0xa2d);
	RTL_W8(0xa2d, value8 | 0x40);	// regA2D[6]=1 to hold and read the CCK FA counters
	priv->pshare->cck_FA_cnt = RTL_R8(0xa5b);
	priv->pshare->cck_FA_cnt = priv->pshare->cck_FA_cnt << 8;
	priv->pshare->cck_FA_cnt += RTL_R8(0xa5c);
#ifdef INTERFERENCE_CONTROL
		priv->pshare->ofdm_FA_total_cnt = (unsigned int) priv->pshare->ofdm_FA_cnt1 +
				priv->pshare->ofdm_FA_cnt2 + priv->pshare->ofdm_FA_cnt3 +
				priv->pshare->ofdm_FA_cnt4 + RTL_R16(0xcf0) + RTL_R16(0xcf2);

		priv->pshare->FA_total_cnt = priv->pshare->ofdm_FA_total_cnt + priv->pshare->cck_FA_cnt;
#else

	priv->pshare->FA_total_cnt = priv->pshare->ofdm_FA_cnt1 + priv->pshare->ofdm_FA_cnt2 +
	                             priv->pshare->ofdm_FA_cnt3 + priv->pshare->ofdm_FA_cnt4 +
	                             priv->pshare->cck_FA_cnt + RTL_R16(0xcf0) + RTL_R16(0xcf2);
#endif
	if (priv->pshare->rf_ft_var.rssi_dump)
		priv->pshare->CCA_total_cnt = ((RTL_R8(0xa60)<<8)|RTL_R8(0xa61)) + RTL_R16(0xda0);
#else
	hold_CCA_FA_counter(priv);
	_FA_statistic(priv);

	if (priv->pshare->rf_ft_var.rssi_dump)
		priv->pshare->CCA_total_cnt = ((RTL_R8(0xa60)<<8)|RTL_R8(0xa61)) + RTL_R16(0xda0);

	release_CCA_FA_counter(priv);
#endif

	reset_FA_reg(priv);

}


//3 ============================================================
//3 Rate Adaptive
//3 ============================================================

void check_RA_by_rssi(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int level = 0;

	switch (pstat->rssi_level) {
		case 1:
			if (pstat->rssi >= priv->pshare->rf_ft_var.raGoDownUpper)
				level = 1;
			else if ((pstat->rssi >= priv->pshare->rf_ft_var.raGoDown20MLower) ||
				((priv->pshare->is_40m_bw) && (pstat->ht_cap_len) &&
				(pstat->rssi >= priv->pshare->rf_ft_var.raGoDown40MLower) &&
				(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))))
				level = 2;
			else
				level = 3;
			break;
		case 2:
			if (pstat->rssi > priv->pshare->rf_ft_var.raGoUpUpper)
				level = 1;
			else if ((pstat->rssi < priv->pshare->rf_ft_var.raGoDown40MLower) ||
				((!pstat->ht_cap_len || !priv->pshare->is_40m_bw ||
				!(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))) &&
				(pstat->rssi < priv->pshare->rf_ft_var.raGoDown20MLower)))
				level = 3;
			else
				level = 2;
			break;
		case 3:
			if (pstat->rssi > priv->pshare->rf_ft_var.raGoUpUpper)
				level = 1;
			else if ((pstat->rssi > priv->pshare->rf_ft_var.raGoUp20MLower) ||
				((priv->pshare->is_40m_bw) && (pstat->ht_cap_len) &&
				(pstat->rssi > priv->pshare->rf_ft_var.raGoUp40MLower) &&
				(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))))
				level = 2;
			else
				level = 3;
			break;
		default:
			if (isErpSta(pstat))
				DEBUG_ERR("wrong rssi level setting\n");
			break;
	}

	if (level != pstat->rssi_level) {
		pstat->rssi_level = level;
		{
		}
	}
}
#endif

#if 0
void add_RATid(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned char limit=16;
	int i;
	unsigned long flags;
	unsigned int update_reg=0;

	SAVE_INT_AND_CLI(flags);

	pstat->tx_ra_bitmap = 0;

	for (i=0; i<32; i++) {
		if (pstat->bssrateset[i])
			pstat->tx_ra_bitmap |= get_bit_value_from_ieee_value(pstat->bssrateset[i]&0x7f);
	}

	if (pstat->ht_cap_len) {
		if ((pstat->MIMO_ps & _HT_MIMO_PS_STATIC_) ||
			(get_rf_mimo_mode(priv)== MIMO_1T2R) ||
			(get_rf_mimo_mode(priv)== MIMO_1T1R))
			limit=8;

		for (i=0; i<limit; i++) {
			if (pstat->ht_cap_buf.support_mcs[i/8] & BIT(i%8))
				pstat->tx_ra_bitmap |= BIT(i+12);
		}
	}

	if (pstat->ht_cap_len) {
		unsigned int set_sgi = 0;
		if (priv->pshare->is_40m_bw && (pstat->tx_bw == HT_CHANNEL_WIDTH_20_40)
#ifdef WIFI_11N_2040_COEXIST
			&& !((OPMODE & WIFI_AP_STATE) && priv->pmib->dot11nConfigEntry.dot11nCoexist &&
			(priv->bg_ap_timeout || orForce20_Switch20Map(priv)
			))
#endif
			) {
			if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_40M_)
				&& priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M)
				set_sgi++;
		} else if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_) &&
			priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M) {
			set_sgi++;
		}

		if (set_sgi) {
				pstat->tx_ra_bitmap |= BIT(28);
		}
	}

	if ((pstat->rssi_level < 1) || (pstat->rssi_level > 3)) {
		if (pstat->rssi >= priv->pshare->rf_ft_var.raGoDownUpper)
			pstat->rssi_level = 1;
		else if ((pstat->rssi >= priv->pshare->rf_ft_var.raGoDown20MLower) ||
			((priv->pshare->is_40m_bw) && (pstat->ht_cap_len) &&
			(pstat->rssi >= priv->pshare->rf_ft_var.raGoDown40MLower) &&
			(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))))
			pstat->rssi_level = 2;
		else
			pstat->rssi_level = 3;
	}

	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) &&
		((OPMODE & WIFI_AP_STATE) || (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)))
		pstat->tx_ra_bitmap &= 0xfffffff0; //disable cck rate


	// rate adaptive by rssi
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && pstat->ht_cap_len && (!should_restrict_Nrate(priv, pstat))) {
		if ((get_rf_mimo_mode(priv) == MIMO_1T2R) || (get_rf_mimo_mode(priv) == MIMO_1T1R)) {
			switch (pstat->rssi_level) {
				case 1:
					pstat->tx_ra_bitmap &= 0x100f0000;
					break;
				case 2:
					pstat->tx_ra_bitmap &= 0x100ff000;
					break;
				case 3:
					if (priv->pshare->is_40m_bw)
						pstat->tx_ra_bitmap &= 0x100ff005;
					else
						pstat->tx_ra_bitmap &= 0x100ff001;
					break;
			}
		} else {
			switch (pstat->rssi_level) {
				case 1:
					pstat->tx_ra_bitmap &= 0x1f8f0000;
					break;
				case 2:
					pstat->tx_ra_bitmap &= 0x1f8ff000;
					break;
				case 3:
					if (priv->pshare->is_40m_bw)
						pstat->tx_ra_bitmap &= 0x010ff005;
					else
						pstat->tx_ra_bitmap &= 0x010ff001;
					break;
			}

			// Don't need to mask high rates due to new rate adaptive parameters
			//if (pstat->is_broadcom_sta)		// use MCS12 as the highest rate vs. Broadcom sta
			//	pstat->tx_ra_bitmap &= 0x81ffffff;

			// NIC driver will report not supporting MCS15 and MCS14 in asoc req
			//if (pstat->is_rtl8190_sta && !pstat->is_2t_mimo_sta)
			//	pstat->tx_ra_bitmap &= 0x83ffffff;		// if Realtek 1x2 sta, don't use MCS15 and MCS14
		}
	}
	else if (((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) && isErpSta(pstat)) ||
			((priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) &&
			((OPMODE & WIFI_AP_STATE) || (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G))))
	{
		switch (pstat->rssi_level) {
			case 1:
				pstat->tx_ra_bitmap &= 0x00000f00;
				break;
			case 2:
				pstat->tx_ra_bitmap &= 0x00000ff0;
				break;
			case 3:
				pstat->tx_ra_bitmap &= 0x00000ff5;
				break;
		}
	} else {
		pstat->tx_ra_bitmap &= 0x0000000d;
	}

// Client mode IOT issue, Button 2009.07.17
#ifdef CLIENT_MODE
	if(OPMODE & WIFI_STATION_STATE) {
		if(!pstat->is_rtl8192s_sta && pstat->is_realtek_sta && pstat->is_legacy_encrpt)
			pstat->tx_ra_bitmap &= 0x0001ffff;					// up to MCS4
	}
#endif

    update_remapAid(priv, pstat);


	RESTORE_INT(flags);
}
#endif

void set_rssi_cmd(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned long flags;
	unsigned int content = 0;

	int rssi = pstat->rssi;

#ifdef HIGH_POWER_EXT_PA
	if( priv->pshare->rf_ft_var.use_ext_pa )
		rssi += RSSI_DIFF_PA;
	if( rssi > 100)
		rssi = 100;
#endif


	SAVE_INT_AND_CLI(flags);

	/*
	 * set rssi
	 */
	 content = rssi<< 24;


	/*
	 * set macid
	 */
	 content |= REMAP_AID(pstat) << 8;

	/*
	 * set cmd id
	 */
	 content |= H2C_CMD_RSSI;

	signin_h2c_cmd(priv, content, 0);

	RESTORE_INT(flags);
}



void set_RATid_cmd(struct rtl8192cd_priv *priv, unsigned int macid, unsigned int rateid, unsigned int ratemask)
{
	unsigned int content = 0;
	unsigned short ext_content = 0;

//#ifdef CONFIG_RTL_8812_SUPPORT
	if(! CHIP_VER_92X_SERIES(priv))
		return;
//#endif

	/*
	 * set ratemask
	 */
	ext_content = ratemask & 0xffff;
	content = ((ratemask & 0xfff0000) >> 16) << 8;

	/*
	 * set short GI
	 */
	if (ratemask & BIT(28))
		content |= BIT(29);

	/*
	 * set macid (station aid)
	 */
	content |= (macid & 0x1f) << 24;

	/*
	 * set rateid (ARFR table)
	 */
	content |= (rateid & 0xf) << 20;

	/*
	 * set ext_content used
	 */
	content |= BIT(7);

	/*
	 * set cmd id
	 */
	content |= H2C_CMD_MACID;

	signin_h2c_cmd(priv, content, ext_content);
}



//3 ============================================================
//3 EDCCA
//3 ============================================================
#if 0
void check_EDCCA(struct rtl8192cd_priv *priv, short rssi)
{
	if ((priv->pshare->rf_ft_var.edcca_thd) && (priv->pmib->dot11RFEntry.dot11channel==14
		|| priv->pshare->is_40m_bw
	)) {
		if((rssi > priv->pshare->rf_ft_var.edcca_thd) && (priv->pshare->phw->EDCCA_on == 0)) {
			RTL_W32(rOFDM0_ECCAThreshold, 0xfc03fd);
			priv->pshare->phw->EDCCA_on =1;
		} else if( (rssi < priv->pshare->rf_ft_var.edcca_thd-5) && priv->pshare->phw->EDCCA_on) {
			RTL_W32(rOFDM0_ECCAThreshold, 0x7f037f);
			priv->pshare->phw->EDCCA_on =0;
		}
	}
	if ((!priv->pshare->rf_ft_var.edcca_thd) && priv->pshare->phw->EDCCA_on) {
		RTL_W32(0xc4c, 0x7f037f);
		priv->pshare->phw->EDCCA_on = 0;
	}
}
#endif

//3 ============================================================
//3 Antenna Diversity
//3 ============================================================
#ifdef SW_ANT_SWITCH

//
// 20100514 Luke/Joseph:
// Add new function to reset antenna diversity state after link.
//

void resetSwAntDivVariable(struct rtl8192cd_priv *priv)
{
	priv->pshare->RSSI_sum_R = 0;
	priv->pshare->RSSI_cnt_R = 0;
	priv->pshare->RSSI_sum_L = 0;
	priv->pshare->RSSI_cnt_L = 0;
	priv->pshare->TXByteCnt_R = 0;
	priv->pshare->TXByteCnt_L = 0;
	priv->pshare->RXByteCnt_R = 0;
	priv->pshare->RXByteCnt_L = 0;

}
void SwAntDivRestAfterLink(struct rtl8192cd_priv *priv)
{
	priv->pshare->RSSI_test = FALSE;
	priv->pshare->DM_SWAT_Table.try_flag = SWAW_STEP_RESET;
	memset(priv->pshare->DM_SWAT_Table.SelectAntennaMap, 0, sizeof(priv->pshare->DM_SWAT_Table.SelectAntennaMap));
	priv->pshare->DM_SWAT_Table.mapIndex = 0;
	priv->pshare->lastTxOkCnt = priv->net_stats.tx_bytes;
	priv->pshare->lastRxOkCnt = priv->net_stats.rx_bytes;
	resetSwAntDivVariable(priv);
}


void dm_SW_AntennaSwitchInit(struct rtl8192cd_priv *priv)
{
	if(!priv->pshare->rf_ft_var.antSw_enable)
		return;

//	if (GET_CHIP_VER(priv) == VERSION_8188C)
	if(get_rf_mimo_mode(priv)== MIMO_1T1R)
		priv->pshare->rf_ft_var.antSw_select = 0;

	//RT_TRACE(COMP_SWAS, DBG_LOUD, ("SWAS:Init SW Antenna Switch\n"));
	resetSwAntDivVariable(priv);
	priv->pshare->DM_SWAT_Table.CurAntenna = Antenna_L;
	priv->pshare->DM_SWAT_Table.try_flag = SWAW_STEP_RESET;
	memset(priv->pshare->DM_SWAT_Table.SelectAntennaMap, 0, sizeof(priv->pshare->DM_SWAT_Table.SelectAntennaMap));
	priv->pshare->DM_SWAT_Table.mapIndex = 0;

#ifdef GPIO_ANT_SWITCH
// GPIO 45 :
// GPIO_MOD     => data port
// GPIO_IO_SEL  => output
	RTL_W32(GPIO_PIN_CTRL, 0x00300000| RTL_R32(GPIO_PIN_CTRL));
	PHY_SetBBReg(priv, GPIO_PIN_CTRL, 0x3000, priv->pshare->DM_SWAT_Table.CurAntenna);
	RTL_W32(rFPGA0_XCD_RFParameter, RTL_R32(rFPGA0_XCD_RFParameter)| BIT(15)|BIT(16));		// enable ANTSEL

#else
	RTL_W32(LEDCFG, RTL_R32(LEDCFG) | BIT(23) );	//enable LED[1:0] pin as ANTSEL

	if ( !priv->pshare->rf_ft_var.antSw_select)	{
		RTL_W32(rFPGA0_XAB_RFParameter, RTL_R32(rFPGA0_XAB_RFParameter) | BIT(13) );	 //select ANTESEL from path A
		RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) | BIT(8)| BIT(9) );  // enable ANTSEL  A as SW control
		RTL_W32(rFPGA0_XA_RFInterfaceOE, (RTL_R32(rFPGA0_XA_RFInterfaceOE) & ~(BIT(8)|BIT(9)))| 0x01<<8 );	// 0x01: left antenna, 0x02: right antenna
	} else 	{
		RTL_W32(rFPGA0_XAB_RFParameter, RTL_R32(rFPGA0_XAB_RFParameter) & ~ BIT(13) );	 //select ANTESEL from path B
		RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) | BIT(24)| BIT(25) );  // enable ANTSEL B as SW control
		RTL_W32(rFPGA0_XB_RFInterfaceOE, (RTL_R32(rFPGA0_XB_RFInterfaceOE) & ~(BIT(8)|BIT(9)))| 0x01<<8 );	// 0x01: left antenna, 0x02: right antenna
	}
	RTL_W16(rFPGA0_TxInfo, (RTL_R16(rFPGA0_TxInfo)&0xf0ff) | BIT(8) );	// b11-b8=0001
#endif

	// Move the timer initialization to InitializeVariables function.
	//PlatformInitializeTimer(Adapter, &pMgntInfo->SwAntennaSwitchTimer, (RT_TIMER_CALL_BACK)dm_SW_AntennaSwitchCallback, NULL, "SwAntennaSwitchTimer");
}
//
// 20100514 Luke/Joseph:
// Add new function for antenna diversity after link.
// This is the main function of antenna diversity after link.
// This function is called in HalDmWatchDog() and dm_SW_AntennaSwitchCallback().
// HalDmWatchDog() calls this function with SWAW_STEP_PEAK to initialize the antenna test.
// In SWAW_STEP_PEAK, another antenna and a 500ms timer will be set for testing.
// After 500ms, dm_SW_AntennaSwitchCallback() calls this function to compare the signal just
// listened on the air with the RSSI of original antenna.
// It chooses the antenna with better RSSI.
// There is also a aged policy for error trying. Each error trying will cost more 5 seconds waiting
// penalty to get next try.
//
void dm_SW_AntennaSwitch(struct rtl8192cd_priv *priv, char Step)
{
	unsigned int	curTxOkCnt, curRxOkCnt;
	unsigned int	CurByteCnt, PreByteCnt;
	int 			Score_R=0, Score_L=0;
	int				RSSI_R, RSSI_L;
	char 			nextAntenna=priv->pshare->DM_SWAT_Table.CurAntenna;
	int				i;

//1 1. Determine which condition should turn off Antenna Diversity

#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		return;
#endif

//	if(!(GET_CHIP_VER(priv) == VERSION_8188C) || !priv->pshare->rf_ft_var.antSw_enable)
//		return;

	if((!priv->assoc_num)
	){
		SwAntDivRestAfterLink(priv);
		return;
	}

	// Handling step mismatch condition.
	// Peak step is not finished at last time. Recover the variable and check again.
	if( Step != priv->pshare->DM_SWAT_Table.try_flag)
	{
		SwAntDivRestAfterLink(priv);
	}

//1 2. Initialization: Select a assocaiated AP or STA as RSSI target
	if(priv->pshare->DM_SWAT_Table.try_flag == SWAW_STEP_RESET) {
#ifdef CLIENT_MODE
		if((OPMODE & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) == (WIFI_STATION_STATE | WIFI_ASOC_STATE)) {
			// Target: Infrastructure mode AP.
			priv->pshare->RSSI_target = NULL;
		}
#endif
		resetSwAntDivVariable(priv);
		priv->pshare->DM_SWAT_Table.try_flag = SWAW_STEP_PEAK;
		return;
	}
	else  {

//1 3. Antenna Diversity

		//2 Calculate TX and RX OK bytes

		curTxOkCnt = priv->net_stats.tx_bytes - priv->pshare->lastTxOkCnt;
		curRxOkCnt = priv->net_stats.rx_bytes - priv->pshare->lastRxOkCnt;
		priv->pshare->lastTxOkCnt = priv->net_stats.tx_bytes;
		priv->pshare->lastRxOkCnt = priv->net_stats.rx_bytes;

		//2 Try State
		if(priv->pshare->DM_SWAT_Table.try_flag == SWAW_STEP_DETERMINE)	{
			//3 1. Seperately caculate TX and RX OK byte counter for ant A and B
			if(priv->pshare->DM_SWAT_Table.CurAntenna == Antenna_R) {
				priv->pshare->TXByteCnt_R += curTxOkCnt;
				priv->pshare->RXByteCnt_R += curRxOkCnt;
			} else 	{
				priv->pshare->TXByteCnt_L += curTxOkCnt;
				priv->pshare->RXByteCnt_L += curRxOkCnt;
			}

			//3 2. Change anntena for testing
			if(priv->pshare->DM_SWAT_Table.RSSI_Trying != 0) {
				nextAntenna = (priv->pshare->DM_SWAT_Table.CurAntenna ) ^ Antenna_MAX;
				priv->pshare->DM_SWAT_Table.RSSI_Trying--;
			}

			//2 Try State End: Determine the best antenna

			if(priv->pshare->DM_SWAT_Table.RSSI_Trying==0) {
				nextAntenna = priv->pshare->DM_SWAT_Table.CurAntenna;
				priv->pshare->DM_SWAT_Table.mapIndex = (priv->pshare->DM_SWAT_Table.mapIndex+1)%SELANT_MAP_SIZE;

				//3 TP Mode: Determine the best antenna by throuhgput
				if(priv->pshare->DM_SWAT_Table.TestMode == TP_MODE)  {



					//3 (1) Saperately caculate total byte count for two antennas
					if(priv->pshare->DM_SWAT_Table.CurAntenna == Antenna_R) {
						CurByteCnt = (priv->pshare->TXByteCnt_R + (priv->pshare->RXByteCnt_R<<1));
						PreByteCnt = (priv->pshare->TXByteCnt_L + (priv->pshare->RXByteCnt_L<<1));
					} else {
						CurByteCnt = (priv->pshare->TXByteCnt_L + (priv->pshare->RXByteCnt_L<<1));
						PreByteCnt = (priv->pshare->TXByteCnt_R + (priv->pshare->RXByteCnt_R<<1));
					}

					//3 (2) Throughput Normalization
					if(priv->pshare->TrafficLoad == TRAFFIC_HIGH)
						CurByteCnt >>=3;
					else if(priv->pshare->TrafficLoad == TRAFFIC_LOW)
						CurByteCnt >>=1;

					if(priv->pshare->DM_SWAT_Table.CurAntenna == Antenna_R) {
						priv->pshare->DM_SWAT_Table.SelectAntennaMap[0][priv->pshare->DM_SWAT_Table.mapIndex] = PreByteCnt;
						priv->pshare->DM_SWAT_Table.SelectAntennaMap[1][priv->pshare->DM_SWAT_Table.mapIndex] = CurByteCnt;
					} else {
						priv->pshare->DM_SWAT_Table.SelectAntennaMap[0][priv->pshare->DM_SWAT_Table.mapIndex] = CurByteCnt;
						priv->pshare->DM_SWAT_Table.SelectAntennaMap[1][priv->pshare->DM_SWAT_Table.mapIndex] = PreByteCnt;
					}

					Score_R = Score_L=0;
					for (i= 0; i<SELANT_MAP_SIZE; i++) {
						Score_L += priv->pshare->DM_SWAT_Table.SelectAntennaMap[0][i];
						Score_R += priv->pshare->DM_SWAT_Table.SelectAntennaMap[1][i];
					}

					nextAntenna = (Score_L > Score_R) ? Antenna_L : Antenna_R;

					if(priv->pshare->rf_ft_var.ant_dump&8)
						panic_printk("Mode TP, select Ant%d, [Score1=%d,Score2=%d]\n", nextAntenna, Score_L, Score_R);

				}

				//3 RSSI Mode: Determine the best anntena by RSSI
				else if(priv->pshare->DM_SWAT_Table.TestMode == RSSI_MODE) {

					//2 Saperately caculate average RSSI for two antennas
					RSSI_L = RSSI_R = 0;

					if(priv->pshare->RSSI_cnt_R > 0)
						RSSI_R = priv->pshare->RSSI_sum_R/priv->pshare->RSSI_cnt_R;
					if(priv->pshare->RSSI_cnt_L > 0)
						RSSI_L = priv->pshare->RSSI_sum_L/priv->pshare->RSSI_cnt_L;

					if(RSSI_L && RSSI_R )
						nextAntenna =  (RSSI_L > RSSI_R) ? Antenna_L : Antenna_R;

					if(priv->pshare->rf_ft_var.ant_dump&8)
						panic_printk("Mode RSSI, RSSI_R=%d(%d), RSSI_L=%d(%d), Ant=%d\n",
						RSSI_R, priv->pshare->RSSI_cnt_R, RSSI_L,  priv->pshare->RSSI_cnt_L, nextAntenna);

				}

				//3 Reset state
				resetSwAntDivVariable(priv);
				priv->pshare->DM_SWAT_Table.try_flag = SWAW_STEP_PEAK;
				priv->pshare->RSSI_test = FALSE;
			}
		}

		//1 Normal State
		else if(priv->pshare->DM_SWAT_Table.try_flag == SWAW_STEP_PEAK)	{

			//3 Determine TP/RSSI mode by TRX OK count
			if((curRxOkCnt+curTxOkCnt) > TP_MODE_THD) {
				//2 Determine current traffic is high or low
				if((curTxOkCnt+curRxOkCnt) > TRAFFIC_THRESHOLD)
					priv->pshare->TrafficLoad = TRAFFIC_HIGH;
				else
					priv->pshare->TrafficLoad = TRAFFIC_LOW;

				priv->pshare->DM_SWAT_Table.RSSI_Trying = 10;
				priv->pshare->DM_SWAT_Table.TestMode = TP_MODE;
			} else	{

				int idx = 0;
				struct stat_info* pEntry = findNextSTA(priv, &idx);
				priv->pshare->RSSI_target = NULL;
				while(pEntry) {
					if(pEntry && pEntry->expire_to) {
						if(!priv->pshare->RSSI_target)
							priv->pshare->RSSI_target = pEntry;
						else if( pEntry->rssi < priv->pshare->RSSI_target->rssi )
							priv->pshare->RSSI_target = pEntry;
					}
					pEntry = findNextSTA(priv, &idx);
				};

				priv->pshare->DM_SWAT_Table.RSSI_Trying = 6;
				priv->pshare->DM_SWAT_Table.TestMode = RSSI_MODE;

				if(priv->pshare->RSSI_target == NULL)	{
					SwAntDivRestAfterLink(priv);
					return;
				}

				//3 reset state
				memset(priv->pshare->DM_SWAT_Table.SelectAntennaMap, 0, sizeof(priv->pshare->DM_SWAT_Table.SelectAntennaMap));
			}

			//3 Begin  to enter Try State
			nextAntenna = (priv->pshare->DM_SWAT_Table.CurAntenna ) ^ Antenna_MAX;
			priv->pshare->DM_SWAT_Table.try_flag = SWAW_STEP_DETERMINE;
			priv->pshare->RSSI_test = TRUE;

			//3 Reset variables
			resetSwAntDivVariable(priv);
		}
	}

//1 4.Change TRX antenna
	if(nextAntenna != priv->pshare->DM_SWAT_Table.CurAntenna) {
#ifdef GPIO_ANT_SWITCH
		PHY_SetBBReg(priv, GPIO_PIN_CTRL, 0x3000, nextAntenna);
#else
		if (!priv->pshare->rf_ft_var.antSw_select)
			PHY_SetBBReg(priv, rFPGA0_XA_RFInterfaceOE, 0x300, nextAntenna);
		else
			PHY_SetBBReg(priv, rFPGA0_XB_RFInterfaceOE, 0x300, nextAntenna);
#endif

//1 5.Reset Statistics
		priv->pshare->DM_SWAT_Table.CurAntenna = nextAntenna;
	}

//1 6.Set next timer

	if(priv->pshare->DM_SWAT_Table.RSSI_Trying == 0) {
		return;
	}

	if(priv->pshare->DM_SWAT_Table.TestMode == RSSI_MODE)	{
		mod_timer(&priv->pshare->swAntennaSwitchTimer, jiffies + RTL_MILISECONDS_TO_JIFFIES(400));		// 400 ms
	} else if(priv->pshare->DM_SWAT_Table.TestMode == TP_MODE)	{

		if(priv->pshare->TrafficLoad == TRAFFIC_HIGH)	{
			if(priv->pshare->DM_SWAT_Table.RSSI_Trying%2 == 0)
				mod_timer(&priv->pshare->swAntennaSwitchTimer, jiffies +  RTL_MILISECONDS_TO_JIFFIES(10));	// 10 ms
			else
				mod_timer(&priv->pshare->swAntennaSwitchTimer, jiffies + RTL_MILISECONDS_TO_JIFFIES(80));	// 80 ms

		} else if(priv->pshare->TrafficLoad == TRAFFIC_LOW) {
			if(priv->pshare->DM_SWAT_Table.RSSI_Trying%2 == 0)
				mod_timer(&priv->pshare->swAntennaSwitchTimer, jiffies + RTL_MILISECONDS_TO_JIFFIES(40));	// 40 ms
			else
				mod_timer(&priv->pshare->swAntennaSwitchTimer, jiffies + RTL_MILISECONDS_TO_JIFFIES(80));	// 80 ms
		}
	}

}


void dm_SW_AntennaSwitchCallback(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv*)task_priv;
	unsigned long flags = 0;
	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	SAVE_INT_AND_CLI(flags);
	dm_SW_AntennaSwitch(priv, SWAW_STEP_DETERMINE);
	RESTORE_INT(flags);
}

//
// 20100514 Luke/Joseph:
// This function is used to gather the RSSI information for antenna testing.
// It selects the RSSI of the peer STA that we want to know.
//
void dm_SWAW_RSSI_Check(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	struct stat_info* pEntry = NULL;
	pEntry = get_stainfo(priv, GetAddr2Ptr(get_pframe(pfrinfo)));

	if((priv->pshare->RSSI_target==NULL)||(priv->pshare->RSSI_target==pEntry)) {
		//1 RSSI for SW Antenna Switch
		if(priv->pshare->DM_SWAT_Table.CurAntenna == Antenna_R)
		{
			priv->pshare->RSSI_sum_R += pfrinfo->rssi;
			priv->pshare->RSSI_cnt_R++;
		} else {
			priv->pshare->RSSI_sum_L += pfrinfo->rssi;
			priv->pshare->RSSI_cnt_L++;
		}
	}
}

#ifndef HW_ANT_SWITCH

int diversity_antenna_select(struct rtl8192cd_priv *priv, unsigned char *data)
{
	int ant = _atoi(data, 16);
//	if(GET_CHIP_VER(priv) != VERSION_8188C)
//		return 0;

	if(ant==Antenna_L || ant==Antenna_R) {
#ifdef GPIO_ANT_SWITCH
		PHY_SetBBReg(priv, GPIO_PIN_CTRL, 0x3000, ant);
#else
		if (!priv->pshare->rf_ft_var.antSw_select)
			PHY_SetBBReg(priv, rFPGA0_XA_RFInterfaceOE, 0x300, ant);
		else
			PHY_SetBBReg(priv, rFPGA0_XB_RFInterfaceOE, 0x300, ant);
#endif
		priv->pshare->DM_SWAT_Table.CurAntenna = ant;
		priv->pshare->rf_ft_var.antSw_enable = 0;
		SwAntDivRestAfterLink(priv);
		memset(priv->pshare->DM_SWAT_Table.SelectAntennaMap, 0, sizeof(priv->pshare->DM_SWAT_Table.SelectAntennaMap));
		return 1;
	} else {
		priv->pshare->rf_ft_var.antSw_enable = 1;
		priv->pshare->lastTxOkCnt = priv->net_stats.tx_bytes;
		priv->pshare->lastRxOkCnt = priv->net_stats.rx_bytes;

		return 0;
	}
}
#endif
#endif
#if defined(HW_ANT_SWITCH)

void dm_HW_AntennaSwitchInit(struct rtl8192cd_priv *priv)
{
	if(!priv->pshare->rf_ft_var.antHw_enable)
		return;
#ifdef SW_ANT_SWITCH
	priv->pshare->rf_ft_var.antSw_enable =0;
#endif

//	if (GET_CHIP_VER(priv) == VERSION_8188C)
	if(get_rf_mimo_mode(priv)== MIMO_1T1R)
		priv->pshare->rf_ft_var.antSw_select = 0;

	if ( !priv->pshare->rf_ft_var.antSw_select)	{
		RTL_W32(rFPGA0_XAB_RFParameter, RTL_R32(rFPGA0_XAB_RFParameter) | BIT(13) );	 //select ANTESEL from path A
		RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) & ~(BIT(8)| BIT(9)) ); // ANTSEL as HW control
		RTL_W32(rFPGA0_XA_RFInterfaceOE, (RTL_R32(rFPGA0_XA_RFInterfaceOE) & ~(BIT(8)|BIT(9)))| 0x01<<8 );	// 0x01: left antenna, 0x02: right antenna
		RTL_W8(0xc50, RTL_R8(0xc50) | BIT(7));	// Enable Hardware antenna switch
		RTL_W32(0xc54, RTL_R32(0xc54) | BIT(23) );	// Decide final antenna by comparing 2 antennas' pwdb
	} else 	{
		RTL_W32(rFPGA0_XAB_RFParameter, RTL_R32(rFPGA0_XAB_RFParameter) & ~ BIT(13) );	 //select ANTESEL from path B
		RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) & ~(BIT(24)| BIT(25)) ); // ANTSEL as HW control
		RTL_W32(rFPGA0_XB_RFInterfaceOE, (RTL_R32(rFPGA0_XB_RFInterfaceOE) & ~(BIT(8)|BIT(9)))| 0x01<<8 );	// 0x01: left antenna, 0x02: right antenna
		RTL_W32(0xc5C, RTL_R32(0xc5c) | BIT(23) );	// Decide final antenna by comparing 2 antennas' pwdb
	}

	priv->pshare->rf_ft_var.CurAntenna = 0;

	RTL_W32(LEDCFG, RTL_R32(LEDCFG) | BIT(23) );	//enable LED[1:0] pin as ANTSEL
	RTL_W16(0xca4, (RTL_R16(0xca4) & ~(0xfff))|0x0c0); 	// Pwdb threshold=12dB
	RTL_W32(0x874, RTL_R32(0x874) & ~ BIT(23) );	// No update ANTSEL during GNT_BT=1
	RTL_W16(rFPGA0_TxInfo, (RTL_R16(rFPGA0_TxInfo)&0xf0ff) | BIT(8) );	// b11-b8=0001
	RTL_W32(0x80c, RTL_R32(0x80c) | BIT(21) );		// assign antenna by tx desc

	// CCK setting
	RTL_W8(0xa01, RTL_R8(0xa01) | BIT(7));			// enable hw ant diversity
	RTL_W8(0xa0c, (RTL_R8(0xa0c) & 0xe0) | 0x0f );	// b4=0, b3:0 = 1111	32 sample
	RTL_W8(0xa11, RTL_R8(0xa11) | BIT(5));			// do not change default optional antenna
	RTL_W8(0xa14, (RTL_R8(0xa14) & 0xe0) | 0x08 );	// default : optional = 1:1
#ifdef GPIO_ANT_SWITCH
	PHY_SetBBReg(priv, rFPGA0_XCD_RFParameter, 0x40000000, 0x01);		// enable ANTSEL
#endif

}

void setRxIdleAnt(struct rtl8192cd_priv *priv, char Ant)
{
	if(priv->pshare->rf_ft_var.CurAntenna != Ant) {
		if(Ant) {
			RTL_W32(0x858, 0x65a965a9);
//			RTL_W8(0x6d8, RTL_R8(0x6d8) | BIT(6) );
		}
		else {
			RTL_W32(0x858, 0x569a569a);
//			RTL_W8(0x6d8, RTL_R8(0x6d8) & (~ BIT(6)));
		}
		priv->pshare->rf_ft_var.CurAntenna = Ant;
	}
}

void dm_STA_Ant_Select(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int ScoreA=0, ScoreB=0, i, nextAnt= pstat->CurAntenna, idleAnt=priv->pshare->rf_ft_var.CurAntenna;

	if((priv->pshare->rf_ft_var.CurAntenna & 0x80)
		|| ((pstat->hwRxAntSel[0] + pstat->hwRxAntSel[1])==0 && (pstat->cckPktCount[0] + pstat->cckPktCount[1])<10)	)
		return;

	for(i=0; i<2; i++) {
		if(pstat->cckPktCount[i]==0 && pstat->hwRxAntSel[i]==0)
			pstat->AntRSSI[i] = 0;
	}

	if(pstat->hwRxAntSel[0] || pstat->hwRxAntSel[1]) {
		ScoreA = pstat->hwRxAntSel[0];
		ScoreB = pstat->hwRxAntSel[1];

		if(ScoreA != ScoreB) {
			if(ScoreA > ScoreB)
				nextAnt = 0;
			else
				nextAnt = 1;
		}
	} else {
		ScoreA = pstat->cckPktCount[0];
		ScoreB = pstat->cckPktCount[1];

		if(ScoreA > 5*ScoreB)
			nextAnt = 0;
		else if(ScoreB > 5*ScoreA)
			nextAnt = 1;
		else if(ScoreA > ScoreB)
			nextAnt = 1;
		else if(ScoreB > ScoreA)
			nextAnt = 0;
	}

	pstat->CurAntenna = nextAnt;

	if(priv->pshare->rf_ft_var.ant_dump&2) {
		panic_printk("id=%d, OFDM/CCK: (%d, %d/%d, %d), RSSI:(%d, %d), ant=%d, RxIdle=%d\n",
			pstat->aid,
			pstat->hwRxAntSel[1],
			pstat->hwRxAntSel[0],
			pstat->cckPktCount[1],
			pstat->cckPktCount[0],
			pstat->AntRSSI[1],
			pstat->AntRSSI[0],
			(pstat->CurAntenna==0 ? 2: 1)
			 ,((priv->pshare->rf_ft_var.CurAntenna&1)==0 ? 2 : 1)
			 );
	}

	if(pstat->AntRSSI[idleAnt]==0)
		pstat->AntRSSI[idleAnt] = pstat->AntRSSI[idleAnt^1];

// reset variables
	pstat->hwRxAntSel[1] = pstat->hwRxAntSel[0] =0;
	pstat->cckPktCount[1]= pstat->cckPktCount[0] =0;

}

void dm_HW_IdleAntennaSelect(struct rtl8192cd_priv *priv)
{
	struct stat_info	*pstat, *pstat_min=NULL;
	struct list_head	*phead, *plist;
	int rssi_min= 0xff, i;

	if(priv->pshare->rf_ft_var.CurAntenna & 0x80)
		return;

	phead = &priv->asoc_list;

	SMP_LOCK_ASOC_LIST(flags);

	plist = phead->next;
	while(plist != phead)	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;

		if((pstat->expire_to) && (pstat->AntRSSI[0] || pstat->AntRSSI[1])) {
			int rssi = (pstat->AntRSSI[0] < pstat->AntRSSI[1]) ? pstat->AntRSSI[0] : pstat->AntRSSI[1];
			if((!pstat_min) || ( rssi < rssi_min) ) {
				pstat_min = pstat;
				rssi_min = rssi;
			}
		}
	};

	SMP_UNLOCK_ASOC_LIST(flags);

	if(pstat_min)
		setRxIdleAnt(priv, pstat_min->CurAntenna);


#ifdef TX_SHORTCUT
	if (!priv->pmib->dot11OperationEntry.disable_txsc) {
		SMP_LOCK_ASOC_LIST(flags);

		plist = phead->next;
		while(plist != phead)	{
			pstat = list_entry(plist, struct stat_info, asoc_list);
			plist = plist->next;

			if(pstat->expire_to) {
				for (i=0; i<TX_SC_ENTRY_NUM; i++) {
					struct tx_desc *pdesc= &(pstat->tx_sc_ent[i].hwdesc1);
					pdesc->Dword2 &= set_desc(~ (BIT(24)|BIT(25)));
					if((pstat->CurAntenna^priv->pshare->rf_ft_var.CurAntenna)&1)
						pdesc->Dword2 |= set_desc(BIT(24)|BIT(25));
					pdesc= &(pstat->tx_sc_ent[i].hwdesc2);
					pdesc->Dword2 &= set_desc(~ (BIT(24)|BIT(25)));
					if((pstat->CurAntenna^priv->pshare->rf_ft_var.CurAntenna)&1)
						pdesc->Dword2 |= set_desc(BIT(24)|BIT(25));
				}
			}
		};

		SMP_UNLOCK_ASOC_LIST(flags);
	}
#endif

}

int diversity_antenna_select(struct rtl8192cd_priv *priv, unsigned char *data)
{
	int ant = _atoi(data, 16);


	if (ant==Antenna_L || ant==Antenna_R) {

#ifdef GPIO_ANT_SWITCH
		if(priv->pshare->rf_ft_var.antSw_enable)  {
			PHY_SetBBReg(priv, GPIO_PIN_CTRL, 0x3000, ant);
		} else
#endif
		{
			if ( !priv->pshare->rf_ft_var.antSw_select) {
				RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) | BIT(8)| BIT(9) );  //  ANTSEL A as SW control
				RTL_W8(0xc50, RTL_R8(0xc50) & (~ BIT(7)));	// rx OFDM SW control
				PHY_SetBBReg(priv, rFPGA0_XA_RFInterfaceOE, 0x300, ant);
			} else {
				RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) | BIT(24)| BIT(25) ); // ANTSEL B as HW control
				PHY_SetBBReg(priv, rFPGA0_XB_RFInterfaceOE, 0x300, ant);
			}
			RTL_W8(0xa01, RTL_R8(0xa01) & (~ BIT(7)));	// rx CCK SW control
			RTL_W32(0x80c, RTL_R32(0x80c) & (~ BIT(21))); // select ant by tx desc
			RTL_W32(0x858, 0x569a569a);
		}
		if(HW_DIV_ENABLE)
			priv->pshare->rf_ft_var.antHw_enable = BIT(5);
		priv->pshare->rf_ft_var.CurAntenna  = (ant%2);
#ifdef SW_ANT_SWITCH
		if(priv->pshare->rf_ft_var.antSw_enable)
			priv->pshare->rf_ft_var.antSw_enable = BIT(5);
		priv->pshare->DM_SWAT_Table.CurAntenna = ant;
		priv->pshare->RSSI_test =0;
#endif
	}
	else if(ant==0){
#ifdef GPIO_ANT_SWITCH
			if(priv->pshare->rf_ft_var.antHw_enable)
#endif
			{
			if (!priv->pshare->rf_ft_var.antSw_select)  {
				RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) & ~(BIT(8)| BIT(9)) );
				RTL_W8(0xc50, RTL_R8(0xc50) | BIT(7));	// OFDM HW control
			} else {
				RTL_W32(rFPGA0_XAB_RFInterfaceSW, RTL_R32(rFPGA0_XAB_RFInterfaceSW) & ~(BIT(24)| BIT(25)) );
			}
			RTL_W8(0xa01, RTL_R8(0xa01) | BIT(7));	// CCK HW control
			RTL_W32(0x80c, RTL_R32(0x80c) | BIT(21) ); // by tx desc
			priv->pshare->rf_ft_var.CurAntenna = 0;
			RTL_W32(0x858, 0x569a569a);
			priv->pshare->rf_ft_var.antHw_enable = 1;
#ifdef SW_ANT_SWITCH
			priv->pshare->rf_ft_var.antSw_enable = 0;
			priv->pshare->RSSI_test =0;
#endif
		}
#ifdef SW_ANT_SWITCH
		if(priv->pshare->rf_ft_var.antSw_enable) {
			dm_SW_AntennaSwitchInit(priv);
			RTL_W32(0x858, 0x569a569a);
			priv->pshare->lastTxOkCnt = priv->net_stats.tx_bytes;
			priv->pshare->lastRxOkCnt = priv->net_stats.rx_bytes;
			priv->pshare->rf_ft_var.antHw_enable = 0;
			priv->pshare->rf_ft_var.antSw_enable = 1;
		}
#endif

	}
#if defined(SW_ANT_SWITCH) && !defined(GPIO_ANT_SWITCH)
	else if(ant==3) {
		if(!priv->pshare->rf_ft_var.antSw_enable) {
			dm_SW_AntennaSwitchInit(priv);
			RTL_W32(0x858, 0x569a569a);
			priv->pshare->lastTxOkCnt = priv->net_stats.tx_bytes;
			priv->pshare->lastRxOkCnt = priv->net_stats.rx_bytes;
		}
		{
			if ( !priv->pshare->rf_ft_var.antSw_select)
				RTL_W8(0xc50, RTL_R8(0xc50) & (~ BIT(7)));	// rx OFDM SW control
			else
				RTL_W8(0xc58, RTL_R8(0xc58) & (~ BIT(7)));	// rx OFDM SW control
		}

		RTL_W8(0xa01, RTL_R8(0xa01) & (~ BIT(7)));		// rx CCK SW control
		RTL_W32(0x80c, RTL_R32(0x80c) & (~ BIT(21))); 	// select ant by tx desc
		priv->pshare->rf_ft_var.antHw_enable = 0;
		priv->pshare->rf_ft_var.antSw_enable = 1;

	}
#endif

	return 1;
}

#endif

//3 ============================================================
//3 Dynamic Noise Control
//3 ============================================================



//3 ============================================================
//3 Leaving STA check
//3 ============================================================
#if 0
//#if defined(DETECT_STA_EXISTANCE) //&& (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
// Check for STA existance. If STA disappears, disconnect it. Added by Annie, 2010-08-10.
void DetectSTAExistance(struct rtl8192cd_priv *priv, struct tx_rpt *report, struct stat_info *pstat)
{
	unsigned char tmpbuf[16];

	// Parameters
	const unsigned int		maxTxFailCnt = 300;		// MAX Tx fail packet count
	const unsigned int		minTxFailCnt = 30;		// MIN Tx fail packet count; this value should be less than maxTxFailCnt.
	const unsigned int		txFailSecThr= 3;			// threshold of Tx Fail Time (in second)

	// Temporarily change Retry Limit when TxFail. (tfrl: TxFailRetryLimit)
	const unsigned char	TFRL = 7;				// New Retry Limit value
	const unsigned char	TFRL_FailCnt = 2;		// Tx Fail Count threshold to set Retry Limit
	const unsigned char	TFRL_SetTime = 2;		// Time to set Retry Limit (in second)
	const unsigned char	TFRL_RcvTime = 10;		// Time to recover Retry Limit (in second)

	if(OPMODE & WIFI_STATION_STATE)
		return;

	if( report->txok != 0 )
	{ // Reset Counter
		pstat->tx_conti_fail_cnt = 0;
		pstat->tx_last_good_time = priv->up_time;
		pstat->leave = 0;
	}
	else if( report->txfail != 0 )
	{
		pstat->tx_conti_fail_cnt += report->txfail;
		DEBUG_WARN( "detect: txfail=%d, tx_conti_fail_cnt=%d\n", report->txfail, pstat->tx_conti_fail_cnt );

		if(	CHIP_VER_92X_SERIES(priv) && (priv->up_time >= (pstat->tx_last_good_time+TFRL_SetTime)) &&
			pstat->tx_conti_fail_cnt >= TFRL_FailCnt &&
			!priv->pshare->bRLShortened )
		{ // Shorten retry limit, because AP spending too much time to send out g mode STA pending packets in HW queue.
			RTL_W16(RL, (TFRL&SRL_Mask)<<SRL_SHIFT|(TFRL&LRL_Mask)<<LRL_SHIFT);
			priv->pshare->bRLShortened = TRUE;
			DEBUG_WARN( "== Shorten RetryLimit to 0x%04X ==\n", RTL_R16(RL) );
		}

		if( 	(pstat->tx_conti_fail_cnt >= maxTxFailCnt) ||
			(pstat->tx_conti_fail_cnt >= minTxFailCnt && priv->up_time >= (pstat->tx_last_good_time+txFailSecThr) )
			)
		{ // This STA is considered as disappeared, so delete it.
			DEBUG_WARN( "** tx_conti_fail_cnt=%d (min=%d,max=%d)\n", pstat->tx_conti_fail_cnt, minTxFailCnt, maxTxFailCnt);
			DEBUG_WARN( "** tx_last_good_time=%d, up_time=%d (Thr:%d)\n", (int)pstat->tx_last_good_time, (int)priv->up_time, txFailSecThr );
			DEBUG_WARN( "AP is going to del_sta %02X:%02X:%02X:%02X:%02X:%02X\n", pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5] );

			sprintf((char *)tmpbuf, "%02x%02x%02x%02x%02x%02x", pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);

//			del_sta(priv, tmpbuf);
			++(pstat->leave);

			if(CHIP_VER_92X_SERIES(priv) {
			if (timer_pending(&priv->pshare->rl_recover_timer))
				del_timer_sync (&priv->pshare->rl_recover_timer);
			mod_timer(&priv->pshare->rl_recover_timer, jiffies + EXPIRE_TO*TFRL_RcvTime);
			}

			// Reset Counter
			pstat->tx_conti_fail_cnt = 0;
			pstat->tx_last_good_time = priv->up_time;
		}
	}
}

// Timer callback function to recover hardware retry limit register. Added by Annie, 2010-08-10.
void RetryLimitRecovery(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	if( priv->pshare->bRLShortened )
	{
		RTL_W16(RL, priv->pshare->RL_setting);
		priv->pshare->bRLShortened = FALSE;
		DEBUG_WARN( "== Recover RetryLimit to 0x%04X ==\n", RTL_R16(RL) );
	}
}

// Chack STA leaving status; per interface. Added by Annie, 2010-08-10.
unsigned char NoLeavingSTA(struct rtl8192cd_priv *priv)
{
	unsigned char bStaAllOK = TRUE;
	struct list_head *phead, *plist;
	struct stat_info *pstat;

	phead = &priv->asoc_list;
	if (!netif_running(priv->dev) || list_empty(phead))
		return bStaAllOK;

	plist = phead->next;
	while (plist != phead)  {
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if( pstat->tx_conti_fail_cnt != 0 ) {
			bStaAllOK = FALSE;
			break;
		}
		plist = plist->next;
	}

	return bStaAllOK;
}

// Chack STA leaving status for all active interface and recover retry limit register value. Added by Annie, 2010-08-10.
void LeavingSTA_RLCheck(struct rtl8192cd_priv *priv)
{
	unsigned char bIfAllOK = TRUE;
	static int AllOKTimes = 0;
	int i;
	// Parameter
	const unsigned char	TFRL_RcvTime = 10;		// Time to recover Retry Limit (in second)

	if( !NoLeavingSTA(priv) )
		bIfAllOK = FALSE;

	if (IS_ROOT_INTERFACE(priv) && GET_VXD_PRIV(priv) ) {
		if( !NoLeavingSTA(GET_VXD_PRIV(priv)) )
			bIfAllOK = FALSE;
	}

	if (IS_ROOT_INTERFACE(priv)) {
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i])) {
					if( !NoLeavingSTA(priv->pvap_priv[i]) )
						bIfAllOK = FALSE;
				}
			}
		}
	}

	if( bIfAllOK ) {
		AllOKTimes ++;

		if( AllOKTimes >= TFRL_RcvTime )
			RetryLimitRecovery((unsigned long)priv);
	}
	else {
		AllOKTimes = 0;
	}
}
#endif





// 92d IQK

#endif
