/*
 * Copyright 2004 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * mpc83xx.h
 *
 * MPC83xx specific definitions
 */

#ifndef __MPC83XX_H__
#define __MPC83XX_H__

#if defined(CONFIG_E300)
#include <asm/e300.h>
#endif

/*
 * MPC83xx cpu provide RCR register to do reset thing specially. easier
 * to implement
 */

#define MPC83xx_RESET

/*
 * System reset offset (PowerPC standard)
 */
#define EXC_OFF_SYS_RESET	0x0100

/*
 * Default Internal Memory Register Space (Freescale recomandation)
 */
#define CONFIG_DEFAULT_IMMR 0xFF400000

/*
 * Watchdog
 */
#define SWCRR      0x0204
#define SWCRR_SWTC 0xFFFF0000 /* Software Watchdog Time Count. */
#define SWCRR_SWEN 0x00000004 /* Watchdog Enable bit. */
#define SWCRR_SWRI 0x00000002 /* Software Watchdog Reset/Interrupt Select bit. */
#define SWCRR_SWPR 0x00000001 /* Software Watchdog Counter Prescale bit. */
#define SWCRR_RES  ~(SWCRR_SWTC | SWCRR_SWEN | SWCRR_SWRI | SWCRR_SWPR)

#define SWCNR      0x0208
#define SWCNR_SWCN 0x0000FFFF Software Watchdog Count Field.
#define SWCNR_RES  ~(SWCNR_SWCN)

#define SWSRR      0x020E

/*
 * Default Internal Memory Register Space (Freescale recomandation)
 */
#define IMMRBAR 0x0000
#define IMMRBAR_BASE_ADDR     0xFFF00000 /* Identifies the 12 most-significant address bits of the base of the 1 MByte internal memory window. */
#define IMMRBAR_RES           ~(IMMRBAR_BASE_ADDR)

/*
 * Default Internal Memory Register Space (Freescale recomandation)
 */
#define LBLAWBAR0 0x0020
#define LBLAWAR0  0x0024
#define LBLAWBAR1 0x0028
#define LBLAWAR1  0x002C
#define LBLAWBAR2 0x0030
#define LBLAWAR2  0x0034
#define LBLAWBAR3 0x0038
#define LBLAWAR3  0x003C


/*
 * Base Registers & Option Registers
 */
#define BR0 0x5000
#define BR1 0x5008
#define BR2 0x5010
#define BR3 0x5018
#define BR4 0x5020
#define BR5 0x5028
#define BR6 0x5030
#define BR7 0x5038

#define BR_BA		0xFFFF8000
#define BR_BA_SHIFT		15
#define BR_PS		0x00001800
#define BR_PS_SHIFT		11
#define BR_PS_8		0x00000800  /* Port Size 8 bit */
#define BR_PS_16	0x00001000  /* Port Size 16 bit */
#define BR_PS_32	0x00001800  /* Port Size 32 bit */
#define BR_DECC		0x00000600
#define BR_DECC_SHIFT		 9
#define BR_WP		0x00000100
#define BR_WP_SHIFT		 8
#define BR_MSEL		0x000000E0
#define BR_MSEL_SHIFT		 5
#define BR_MS_GPCM	0x00000000  /* GPCM */
#define BR_MS_SDRAM	0x00000060  /* SDRAM */
#define BR_MS_UPMA	0x00000080  /* UPMA */
#define BR_MS_UPMB	0x000000A0  /* UPMB */
#define BR_MS_UPMC	0x000000C0  /* UPMC */
#define BR_V		0x00000001
#define BR_V_SHIFT		 0
#define BR_RES		~(BR_BA|BR_PS|BR_DECC|BR_WP|BR_MSEL|BR_V)

#define OR0 0x5004
#define OR1 0x500C
#define OR2 0x5014
#define OR3 0x501C
#define OR4 0x5024
#define OR5 0x502C
#define OR6 0x5034
#define OR7 0x503C

#define OR_GPCM_AM		0xFFFF8000
#define OR_GPCM_AM_SHIFT		15
#define OR_GPCM_BCTLD		0x00001000
#define OR_GPCM_BCTLD_SHIFT		12
#define OR_GPCM_CSNT		0x00000800
#define OR_GPCM_CSNT_SHIFT		11
#define OR_GPCM_ACS		0x00000600
#define OR_GPCM_ACS_SHIFT		 9
#define OR_GPCM_ACS_0b10	0x00000400
#define OR_GPCM_ACS_0b11	0x00000600
#define OR_GPCM_XACS		0x00000100
#define OR_GPCM_XACS_SHIFT		 8
#define OR_GPCM_SCY		0x000000F0
#define OR_GPCM_SCY_SHIFT		 4
#define OR_GPCM_SCY_1		0x00000010
#define OR_GPCM_SCY_2		0x00000020
#define OR_GPCM_SCY_3		0x00000030
#define OR_GPCM_SCY_4		0x00000040
#define OR_GPCM_SCY_5		0x00000050
#define OR_GPCM_SCY_6		0x00000060
#define OR_GPCM_SCY_7		0x00000070
#define OR_GPCM_SCY_8		0x00000080
#define OR_GPCM_SCY_9		0x00000090
#define OR_GPCM_SCY_10		0x000000a0
#define OR_GPCM_SCY_11		0x000000b0
#define OR_GPCM_SCY_12		0x000000c0
#define OR_GPCM_SCY_13		0x000000d0
#define OR_GPCM_SCY_14		0x000000e0
#define OR_GPCM_SCY_15		0x000000f0
#define OR_GPCM_SETA		0x00000008
#define OR_GPCM_SETA_SHIFT		 3
#define OR_GPCM_TRLX		0x00000004
#define OR_GPCM_TRLX_SHIFT		 2
#define OR_GPCM_EHTR		0x00000002
#define OR_GPCM_EHTR_SHIFT		 1
#define OR_GPCM_EAD		0x00000001
#define OR_GPCM_EAD_SHIFT		 0

#define OR_UPM_AM    0xFFFF8000
#define OR_UPM_AM_SHIFT      15
#define OR_UPM_XAM   0x00006000
#define OR_UPM_XAM_SHIFT     13
#define OR_UPM_BCTLD 0x00001000
#define OR_UPM_BCTLD_SHIFT   12
#define OR_UPM_BI    0x00000100
#define OR_UPM_BI_SHIFT       8
#define OR_UPM_TRLX  0x00000004
#define OR_UPM_TRLX_SHIFT     2
#define OR_UPM_EHTR  0x00000002
#define OR_UPM_EHTR_SHIFT     1
#define OR_UPM_EAD   0x00000001
#define OR_UPM_EAD_SHIFT      0

#define OR_SDRAM_AM    0xFFFF8000
#define OR_SDRAM_AM_SHIFT      15
#define OR_SDRAM_XAM   0x00006000
#define OR_SDRAM_XAM_SHIFT     13
#define OR_SDRAM_COLS  0x00001C00
#define OR_SDRAM_COLS_SHIFT    10
#define OR_SDRAM_ROWS  0x000001C0
#define OR_SDRAM_ROWS_SHIFT     6
#define OR_SDRAM_PMSEL 0x00000020
#define OR_SDRAM_PMSEL_SHIFT    5
#define OR_SDRAM_EAD   0x00000001
#define OR_SDRAM_EAD_SHIFT      0

/*
 * Hard Reset Configration Word - High
 */
#define HRCWH_PCI_AGENT              0x00000000
#define HRCWH_PCI_HOST               0x80000000

#define HRCWH_32_BIT_PCI             0x00000000
#define HRCWH_64_BIT_PCI             0x40000000

#define HRCWH_PCI1_ARBITER_DISABLE   0x00000000
#define HRCWH_PCI1_ARBITER_ENABLE    0x20000000

#define HRCWH_PCI2_ARBITER_DISABLE   0x00000000
#define HRCWH_PCI2_ARBITER_ENABLE    0x10000000

#define HRCWH_CORE_DISABLE           0x08000000
#define HRCWH_CORE_ENABLE            0x00000000

#define HRCWH_FROM_0X00000100        0x00000000
#define HRCWH_FROM_0XFFF00100        0x04000000

#define HRCWH_BOOTSEQ_DISABLE        0x00000000
#define HRCWH_BOOTSEQ_NORMAL         0x01000000
#define HRCWH_BOOTSEQ_EXTENDED       0x02000000

#define HRCWH_SW_WATCHDOG_DISABLE    0x00000000
#define HRCWH_SW_WATCHDOG_ENABLE     0x00800000

#define HRCWH_ROM_LOC_DDR_SDRAM      0x00000000
#define HRCWH_ROM_LOC_PCI1           0x00100000
#define HRCWH_ROM_LOC_PCI2           0x00200000
#define HRCWH_ROM_LOC_LOCAL_8BIT     0x00500000
#define HRCWH_ROM_LOC_LOCAL_16BIT    0x00600000
#define HRCWH_ROM_LOC_LOCAL_32BIT    0x00700000

#define HRCWH_TSEC1M_IN_RGMII        0x00000000
#define HRCWH_TSEC1M_IN_RTBI         0x00004000
#define HRCWH_TSEC1M_IN_GMII         0x00008000
#define HRCWH_TSEC1M_IN_TBI          0x0000C000

#define HRCWH_TSEC2M_IN_RGMII        0x00000000
#define HRCWH_TSEC2M_IN_RTBI         0x00001000
#define HRCWH_TSEC2M_IN_GMII         0x00002000
#define HRCWH_TSEC2M_IN_TBI          0x00003000

#define HRCWH_BIG_ENDIAN             0x00000000
#define HRCWH_LITTLE_ENDIAN          0x00000008

/*
 * Hard Reset Configration Word - Low
 */
#define HRCWL_LCL_BUS_TO_SCB_CLK_1X1 0x00000000
#define HRCWL_LCL_BUS_TO_SCB_CLK_2X1 0x80000000

#define HRCWL_DDR_TO_SCB_CLK_1X1     0x00000000
#define HRCWL_DDR_TO_SCB_CLK_2X1     0x40000000

#define HRCWL_CSB_TO_CLKIN_16X1      0x00000000
#define HRCWL_CSB_TO_CLKIN_1X1       0x01000000
#define HRCWL_CSB_TO_CLKIN_2X1       0x02000000
#define HRCWL_CSB_TO_CLKIN_3X1       0x03000000
#define HRCWL_CSB_TO_CLKIN_4X1       0x04000000
#define HRCWL_CSB_TO_CLKIN_5X1       0x05000000
#define HRCWL_CSB_TO_CLKIN_6X1       0x06000000
#define HRCWL_CSB_TO_CLKIN_7X1       0x07000000
#define HRCWL_CSB_TO_CLKIN_8X1       0x08000000
#define HRCWL_CSB_TO_CLKIN_9X1       0x09000000
#define HRCWL_CSB_TO_CLKIN_10X1      0x0A000000
#define HRCWL_CSB_TO_CLKIN_11X1      0x0B000000
#define HRCWL_CSB_TO_CLKIN_12X1      0x0C000000
#define HRCWL_CSB_TO_CLKIN_13X1      0x0D000000
#define HRCWL_CSB_TO_CLKIN_14X1      0x0E000000
#define HRCWL_CSB_TO_CLKIN_15X1      0x0F000000

#define HRCWL_VCO_BYPASS             0x00000000
#define HRCWL_VCO_1X2                0x00000000
#define HRCWL_VCO_1X4                0x00200000
#define HRCWL_VCO_1X8                0x00400000

#define HRCWL_CORE_TO_CSB_BYPASS     0x00000000
#define HRCWL_CORE_TO_CSB_1X1        0x00020000
#define HRCWL_CORE_TO_CSB_1_5X1      0x00030000
#define HRCWL_CORE_TO_CSB_2X1        0x00040000
#define HRCWL_CORE_TO_CSB_2_5X1      0x00050000
#define HRCWL_CORE_TO_CSB_3X1        0x00060000

/*
 * LCRR - Clock Ratio Register (10.3.1.16)
 */
#define LCRR_DBYP      0x80000000
#define LCRR_DBYP_SHIFT        31
#define LCRR_BUFCMDC   0x30000000
#define LCRR_BUFCMDC_1 0x10000000
#define LCRR_BUFCMDC_2 0x20000000
#define LCRR_BUFCMDC_3 0x30000000
#define LCRR_BUFCMDC_4 0x00000000
#define LCRR_BUFCMDC_SHIFT     28
#define LCRR_ECL       0x03000000
#define LCRR_ECL_4     0x00000000
#define LCRR_ECL_5     0x01000000
#define LCRR_ECL_6     0x02000000
#define LCRR_ECL_7     0x03000000
#define LCRR_ECL_SHIFT         24
#define LCRR_EADC      0x00030000
#define LCRR_EADC_1    0x00010000
#define LCRR_EADC_2    0x00020000
#define LCRR_EADC_3    0x00030000
#define LCRR_EADC_4    0x00000000
#define LCRR_EADC_SHIFT        16
#define LCRR_CLKDIV    0x0000000F
#define LCRR_CLKDIV_2  0x00000002
#define LCRR_CLKDIV_4  0x00000004
#define LCRR_CLKDIV_8  0x00000008
#define LCRR_CLKDIV_SHIFT       0

#endif	/* __MPC83XX_H__ */
