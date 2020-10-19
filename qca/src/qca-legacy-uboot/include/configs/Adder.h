/*
 * Copyright (C) 2004-2005 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 *
 * Support for Analogue&Micro Adder boards family.
 * Tested on AdderII and Adder87x.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#if !defined(CONFIG_MPC875) && !defined(CONFIG_MPC852T)
#define CONFIG_MPC875
#endif

#define CONFIG_ADDER				/* Analogue&Micro Adder board	*/

#define	CONFIG_8xx_CONS_SMC1	1		/* Console is on SMC1		*/
#define CONFIG_BAUDRATE		38400

#define CONFIG_ETHER_ON_FEC1
#define CONFIG_ETHER_ON_FEC2

#if defined(CONFIG_ETHER_ON_FEC1) || defined(CONFIG_ETHER_ON_FEC2)
#define CFG_DISCOVER_PHY
#define FEC_ENET
#endif /* CONFIG_ETHER_ON_FEC || CONFIG_ETHER_ON_FEC2 */

#define CONFIG_8xx_OSCLK		10000000 /* 10 MHz oscillator on EXTCLK */
#define CONFIG_8xx_CPUCLK_DEFAULT	50000000
#define CFG_8xx_CPUCLK_MIN		40000000
#ifdef CONFIG_MPC852T
#define CFG_8xx_CPUCLK_MAX		50000000
#else
#define CFG_8xx_CPUCLK_MAX		133000000
#endif /* CONFIG_MPC852T */

#define CONFIG_COMMANDS		(CONFIG_CMD_DFL  \
				| CFG_CMD_DHCP   \
				| CFG_CMD_IMMAP  \
				| CFG_CMD_MII    \
				| CFG_CMD_PING   \
				)

/* This must be included AFTER the definition of CONFIG_COMMANDS */
#include <cmd_confdefs.h>

#define CONFIG_BOOTDELAY	5		/* Autoboot after 5 seconds	*/
#define CONFIG_BOOTCOMMAND	"bootm fe040000"	/* Autoboot command	*/
#define CONFIG_BOOTARGS		"root=/dev/mtdblock1 rw mtdparts=1M(ROM)ro,-(root)"

#define CONFIG_BZIP2		/* Include support for bzip2 compressed images  */
#undef	CONFIG_WATCHDOG		/* Disable platform specific watchdog		*/

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 */
#define CFG_PROMPT		"=> "		/* Monitor Command Prompt	*/
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "
#define CFG_LONGHELP				/* #undef to save memory	*/
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CFG_PBSIZE (CFG_CBSIZE + sizeof(CFG_PROMPT) + 16)  /* Print Buffer Size */
#define CFG_MAXARGS		16		/* Max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_LOAD_ADDR		0x400000	/* Default load address		*/

#define CFG_HZ			1000		/* Decrementer freq: 1 ms ticks	*/

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * RAM configuration (note that CFG_SDRAM_BASE must be zero)
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_SDRAM_MAX_SIZE	0x01000000	/* Up to 16 Mbyte		*/

#define CFG_MAMR		0x00002114

/*
 * 4096	Up to 4096 SDRAM rows
 * 1000	factor s -> ms
 * 32	PTP (pre-divider from MPTPR)
 * 4	Number of refresh cycles per period
 * 64	Refresh cycle in ms per number of rows
 */
#define CFG_PTA_PER_CLK		((4096 * 32 * 1000) / (4 * 64))

#define CFG_MEMTEST_START	0x00100000	/* memtest works on		*/
#define CFG_MEMTEST_END		0x00500000	/* 1 ... 5 MB in SDRAM		*/

#define CFG_RESET_ADDRESS	0x09900000

/*-----------------------------------------------------------------------
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MONITOR_LEN		(192 << 10)	/* Reserve 192 KB for Monitor   */
#ifdef CONFIG_BZIP2
#define CFG_MALLOC_LEN		(2500 << 10)	/* Reserve ~2.5 MB for malloc() */
#else
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 KB for malloc()  */
#endif /* CONFIG_BZIP2 */

/*-----------------------------------------------------------------------
 * Flash organisation
 */
#define CFG_FLASH_BASE		0xFE000000
#define CFG_FLASH_CFI				/* The flash is CFI compatible  */
#define CFG_FLASH_CFI_DRIVER			/* Use common CFI driver        */
#define CFG_MAX_FLASH_BANKS	1		/* Max number of flash banks	*/
#define CFG_MAX_FLASH_SECT	128		/* Max num of sects on one chip */

/* Environment is in flash */
#define CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x10000 	/* We use one complete sector	*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)

#define CONFIG_ENV_OVERWRITE

#define CFG_OR0_PRELIM		0xFF000774
#define CFG_BR0_PRELIM		(CFG_FLASH_BASE | BR_PS_16 | BR_MS_GPCM | BR_V)

#define	CFG_DIRECT_FLASH_TFTP

/*-----------------------------------------------------------------------
 * Internal Memory Map Register
 */
#define CFG_IMMR		0xFF000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x2F00		/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	128  /* Size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Configuration registers
 */
#ifdef CONFIG_WATCHDOG
#define CFG_SYPCR		(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME  | \
				 SYPCR_SWF  | SYPCR_SWE | SYPCR_SWRI | \
				 SYPCR_SWP)
#else
#define CFG_SYPCR		(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME  | \
				 SYPCR_SWF  | SYPCR_SWP)
#endif /* CONFIG_WATCHDOG */

#define CFG_SIUMCR		(SIUMCR_MLRC01 | SIUMCR_DBGC11)

/* TBSCR - Time Base Status and Control Register */
#define CFG_TBSCR		(TBSCR_TBF | TBSCR_TBE)

/* PISCR - Periodic Interrupt Status and Control */
#define CFG_PISCR       	(PISCR_PS | PISCR_PITF)

/* PLPRCR - PLL, Low-Power, and Reset Control Register */
/* #define CFG_PLPRCR      	PLPRCR_TEXPS */

/* SCCR - System Clock and reset Control Register */
#define SCCR_MASK       	SCCR_EBDF11
#define CFG_SCCR		SCCR_RTSEL

#define CFG_DER         	0

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx chips			*/

/*-----------------------------------------------------------------------
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from flash	*/
#define BOOTFLAG_WARM		0x02	/* Software reboot			*/

#endif /* __CONFIG_H */
