/*
 * (C) Copyright 2001-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#undef CFG_RAMBOOT

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC8260		1	/* This is a MPC8260 CPU	*/
#define CONFIG_PM828		1	/* ...on a PM828 module */
#define CONFIG_CPM2		1	/* Has a CPM2 */

#undef CONFIG_DB_CR826_J30x_ON		/* J30x jumpers on D.B. carrier */

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define CONFIG_PREBOOT	"echo;echo Type \"run flash_nfs\" to mount root filesystem over NFS;echo"

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND							\
	"bootp;"								\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;"	\
	"bootm"

/* enable I2C and select the hardware/software driver */
#undef	CONFIG_HARD_I2C
#define CONFIG_SOFT_I2C		1	/* I2C bit-banged		*/
# define CFG_I2C_SPEED		50000
# define CFG_I2C_SLAVE		0xFE
/*
 * Software (bit-bang) I2C driver configuration
 */
#define I2C_PORT	3		/* Port A=0, B=1, C=2, D=3 */
#define I2C_ACTIVE	(iop->pdir |=  0x00010000)
#define I2C_TRISTATE	(iop->pdir &= ~0x00010000)
#define I2C_READ	((iop->pdat & 0x00010000) != 0)
#define I2C_SDA(bit)	if(bit) iop->pdat |=  0x00010000; \
			else	iop->pdat &= ~0x00010000
#define I2C_SCL(bit)	if(bit) iop->pdat |=  0x00020000; \
			else	iop->pdat &= ~0x00020000
#define I2C_DELAY	udelay(5)	/* 1/4 I2C clock duration */


#define CONFIG_RTC_PCF8563
#define CFG_I2C_RTC_ADDR	0x51

/*
 * select serial console configuration
 *
 * if either CONFIG_CONS_ON_SMC or CONFIG_CONS_ON_SCC is selected, then
 * CONFIG_CONS_INDEX must be set to the channel number (1-2 for SMC, 1-4
 * for SCC).
 *
 * if CONFIG_CONS_NONE is defined, then the serial console routines must
 * defined elsewhere (for example, on the cogent platform, there are serial
 * ports on the motherboard which are used for the serial console - see
 * cogent/cma101/serial.[ch]).
 */
#define CONFIG_CONS_ON_SMC		/* define if console on SMC */
#undef	CONFIG_CONS_ON_SCC		/* define if console on SCC */
#undef	CONFIG_CONS_NONE		/* define if console on something else*/
#define CONFIG_CONS_INDEX	2	/* which serial channel for console */

/*
 * select ethernet configuration
 *
 * if CONFIG_ETHER_ON_SCC is selected, then
 *   - CONFIG_ETHER_INDEX must be set to the channel number (1-4)
 *   - CONFIG_NET_MULTI must not be defined
 *
 * if CONFIG_ETHER_ON_FCC is selected, then
 *   - one or more CONFIG_ETHER_ON_FCCx (x=1,2,3) must also be selected
 *   - CONFIG_NET_MULTI must be defined
 *
 * if CONFIG_ETHER_NONE is defined, then either the ethernet routines must be
 * defined elsewhere (as for the console), or CFG_CMD_NET must be removed
 * from CONFIG_COMMANDS to remove support for networking.
 */
#define CONFIG_NET_MULTI
#undef	CONFIG_ETHER_NONE		/* define if ether on something else */

#undef	CONFIG_ETHER_ON_SCC		/* define if ether on SCC	*/
#define CONFIG_ETHER_INDEX    1		/* which SCC channel for ethernet */

#define CONFIG_ETHER_ON_FCC		/* define if ether on FCC	*/
/*
 * - Rx-CLK is CLK11
 * - Tx-CLK is CLK10
 */
#define CONFIG_ETHER_ON_FCC1
# define CFG_CMXFCR_MASK1	(CMXFCR_FC1|CMXFCR_RF1CS_MSK|CMXFCR_TF1CS_MSK)
#ifndef CONFIG_DB_CR826_J30x_ON
# define CFG_CMXFCR_VALUE1	(CMXFCR_RF1CS_CLK11|CMXFCR_TF1CS_CLK10)
#else
# define CFG_CMXFCR_VALUE1	(CMXFCR_RF1CS_CLK11|CMXFCR_TF1CS_CLK12)
#endif
/*
 * - Rx-CLK is CLK15
 * - Tx-CLK is CLK14
 */
#define CONFIG_ETHER_ON_FCC2
# define CFG_CMXFCR_MASK2	(CMXFCR_FC2|CMXFCR_RF2CS_MSK|CMXFCR_TF2CS_MSK)
# define CFG_CMXFCR_VALUE2	(CMXFCR_RF2CS_CLK13|CMXFCR_TF2CS_CLK14)
/*
 * - RAM for BD/Buffers is on the 60x Bus (see 28-13)
 * - Enable Full Duplex in FSMR
 */
# define CFG_CPMFCR_RAMTYPE	0
# define CFG_FCC_PSMR		(FCC_PSMR_FDE|FCC_PSMR_LPB)

/* system clock rate (CLKIN) - equal to the 60x and local bus speed */
#define CONFIG_8260_CLKIN	100000000	/* in Hz */

#if defined(CONFIG_CONS_NONE) || defined(CONFIG_CONS_USE_EXTC)
#define CONFIG_BAUDRATE		230400
#else
#define CONFIG_BAUDRATE		9600
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT|CONFIG_BOOTP_BOOTFILESIZE)

#ifdef CONFIG_PCI
#define CONFIG_COMMANDS	       (CONFIG_CMD_DFL	| \
				CFG_CMD_BEDBUG	| \
				CFG_CMD_DATE	| \
				CFG_CMD_DHCP	| \
				CFG_CMD_DOC	| \
				CFG_CMD_EEPROM	| \
				CFG_CMD_I2C	| \
				CFG_CMD_NFS	| \
				CFG_CMD_PCI	| \
				CFG_CMD_SNTP	)
#else	/* ! PCI */
#define CONFIG_COMMANDS	       (CONFIG_CMD_DFL	| \
				CFG_CMD_BEDBUG	| \
				CFG_CMD_DATE	| \
				CFG_CMD_DHCP	| \
				CFG_CMD_DOC	| \
				CFG_CMD_EEPROM	| \
				CFG_CMD_I2C	| \
				CFG_CMD_NFS	| \
				CFG_CMD_SNTP	)
#endif	/* CONFIG_PCI */

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Disk-On-Chip configuration
 */
#define CFG_NAND_LEGACY

#define CFG_DOC_SHORT_TIMEOUT
#define CFG_MAX_DOC_DEVICE	1	/* Max number of DOC devices	*/

#define CFG_DOC_SUPPORT_2000
#define CFG_DOC_SUPPORT_MILLENNIUM

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END 0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define CFG_LOAD_ADDR	0x100000	/* default load address */

#define CFG_HZ		1000		/* decrementer freq: 1 ms ticks */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CFG_RESET_ADDRESS 0xFDFFFFFC	/* "bad" address		*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	     (8 << 20)	     /* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * Flash and Boot ROM mapping
 */

#define CFG_BOOTROM_BASE	0xFF800000
#define CFG_BOOTROM_SIZE	0x00080000
#define CFG_FLASH0_BASE		0x40000000
#define CFG_FLASH0_SIZE		0x02000000
#define CFG_DOC_BASE		0xFF800000
#define CFG_DOC_SIZE		0x00100000


/* Flash bank size (for preliminary settings)
 */
#define CFG_FLASH_SIZE CFG_FLASH0_SIZE

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max num of memory banks	*/
#define CFG_MAX_FLASH_SECT	135	/* max num of sects on one chip */

#define CFG_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)	*/

#if 0
/* Start port with environment in flash; switch to EEPROM later */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		(CFG_FLASH_BASE+0x40000)
#define CFG_ENV_SIZE		0x40000
#define CFG_ENV_SECT_SIZE	0x40000
#else
/* Final version: environment in EEPROM */
#define CFG_ENV_IS_IN_EEPROM	1
#define CFG_I2C_EEPROM_ADDR	0x58
#define CFG_I2C_EEPROM_ADDR_LEN 1
#define CFG_EEPROM_PAGE_WRITE_BITS	4
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10	/* and takes up to 10 msec */
#define CFG_ENV_OFFSET		512
#define CFG_ENV_SIZE		(2048 - 512)
#endif

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 *
 * if you change bits in the HRCW, you must also change the CFG_*
 * defines for the various registers affected by the HRCW e.g. changing
 * HRCW_DPPCxx requires you to also change CFG_SIUMCR.
 */
#if defined(CONFIG_BOOT_ROM)
#define CFG_HRCW_MASTER		(HRCW_BPS01 | HRCW_CIP | HRCW_ISB100 | HRCW_BMS)
#else
#define CFG_HRCW_MASTER		(HRCW_CIP | HRCW_ISB100 | HRCW_BMS)
#endif

/* no slaves so just fill with zeros */
#define CFG_HRCW_SLAVE1		0
#define CFG_HRCW_SLAVE2		0
#define CFG_HRCW_SLAVE3		0
#define CFG_HRCW_SLAVE4		0
#define CFG_HRCW_SLAVE5		0
#define CFG_HRCW_SLAVE6		0
#define CFG_HRCW_SLAVE7		0

/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CFG_IMMR		0xF0000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x4000	/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	128 /* size in bytes reserved for initial data*/
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 *
 * 60x SDRAM is mapped at CFG_SDRAM_BASE, local SDRAM
 * is mapped at SDRAM_BASE2_PRELIM.
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		CFG_FLASH0_BASE
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()*/

#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
# define CFG_RAMBOOT
#endif

#ifdef	CONFIG_PCI
#define CONFIG_PCI_PNP
#define CONFIG_EEPRO100
#define CFG_RX_ETH_BUFFER	8		/* use 8 rx buffer on eepro100	*/
#endif

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH*/
#define BOOTFLAG_WARM		0x02	/* Software reboot		   */


/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32	/* For MPC8260 CPU		*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * HIDx - Hardware Implementation-dependent Registers			 2-11
 *-----------------------------------------------------------------------
 * HID0 also contains cache control - initially enable both caches and
 * invalidate contents, then the final state leaves only the instruction
 * cache enabled. Note that Power-On and Hard reset invalidate the caches,
 * but Soft reset does not.
 *
 * HID1 has only read-only information - nothing to set.
 */
#define CFG_HID0_INIT	(HID0_ICE|HID0_DCE|HID0_ICFI|HID0_DCI|\
				HID0_IFEM|HID0_ABE)
#define CFG_HID0_FINAL	(HID0_ICE|HID0_IFEM|HID0_ABE)
#define CFG_HID2	0

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register					 5-5
 *-----------------------------------------------------------------------
 * turn on Checkstop Reset Enable
 */
#define CFG_RMR		RMR_CSRE

/*-----------------------------------------------------------------------
 * BCR - Bus Configuration					 4-25
 *-----------------------------------------------------------------------
 */

#define BCR_APD01	0x10000000
#define CFG_BCR		(BCR_APD01|BCR_ETM|BCR_LETM)	/* 8260 mode */

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				 4-31
 *-----------------------------------------------------------------------
 */
#if 0
#define CFG_SIUMCR	(SIUMCR_DPPC00|SIUMCR_APPC10|SIUMCR_CS10PC01)
#else
#define CFG_SIUMCR	(SIUMCR_DPPC10|SIUMCR_APPC10)
#endif


/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				 4-35
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Watchdog & Bus Monitor Timer max, 60x Bus Monitor enable
 */
#if defined(CONFIG_WATCHDOG)
#define CFG_SYPCR	(SYPCR_SWTC|SYPCR_BMT|SYPCR_PBME|SYPCR_LBME|\
			 SYPCR_SWRI|SYPCR_SWP|SYPCR_SWE)
#else
#define CFG_SYPCR	(SYPCR_SWTC|SYPCR_BMT|SYPCR_PBME|SYPCR_LBME|\
			 SYPCR_SWRI|SYPCR_SWP)
#endif /* CONFIG_WATCHDOG */

/*-----------------------------------------------------------------------
 * TMCNTSC - Time Counter Status and Control			 4-40
 *-----------------------------------------------------------------------
 * Clear once per Second and Alarm Interrupt Status, Set 32KHz timersclk,
 * and enable Time Counter
 */
#define CFG_TMCNTSC	(TMCNTSC_SEC|TMCNTSC_ALR|TMCNTSC_TCF|TMCNTSC_TCE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		 4-42
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Set 32KHz timersclk, and enable
 * Periodic timer
 */
#define CFG_PISCR	(PISCR_PS|PISCR_PTF|PISCR_PTE)

/*-----------------------------------------------------------------------
 * SCCR - System Clock Control					 9-8
 *-----------------------------------------------------------------------
 */
#define CFG_SCCR	(SCCR_DFBRG00)

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration				13-7
 *-----------------------------------------------------------------------
 */
#define CFG_RCCR	0

/*
 * Init Memory Controller:
 *
 * Bank Bus	Machine PortSz	Device
 * ---- ---	------- ------	------
 *  0	60x	GPCM	64 bit	FLASH
 *  1	60x	SDRAM	64 bit	SDRAM
 *
 */

	/* Initialize SDRAM on local bus
	 */
#define CFG_INIT_LOCAL_SDRAM


/* Minimum mask to separate preliminary
 * address ranges for CS[0:2]
 */
#define CFG_MIN_AM_MASK 0xC0000000

/*
 * we use the same values for 32 MB and 128 MB SDRAM
 * refresh rate = 7.68 uS (100 MHz Bus Clock)
 */
#define CFG_MPTPR	0x2000
#define CFG_PSRT	0x16

#define CFG_MRS_OFFS	0x00000000


#if defined(CONFIG_BOOT_ROM)
/*
 * Bank 0 - Boot ROM (8 bit wide)
 */
#define CFG_BR0_PRELIM	((CFG_BOOTROM_BASE & BRx_BA_MSK)|\
			 BRx_PS_8			|\
			 BRx_MS_GPCM_P			|\
			 BRx_V)

#define CFG_OR0_PRELIM	(P2SZ_TO_AM(CFG_BOOTROM_SIZE)	|\
			 ORxG_CSNT			|\
			 ORxG_ACS_DIV1			|\
			 ORxG_SCY_5_CLK			|\
			 ORxG_EHTR			|\
			 ORxG_TRLX)

/*
 * Bank 1 - Flash (64 bit wide)
 */
#define CFG_BR1_PRELIM	((CFG_FLASH_BASE & BRx_BA_MSK)	|\
			 BRx_PS_64			|\
			 BRx_MS_GPCM_P			|\
			 BRx_V)

#define CFG_OR1_PRELIM	(P2SZ_TO_AM(CFG_FLASH_SIZE)	|\
			 ORxG_CSNT			|\
			 ORxG_ACS_DIV1			|\
			 ORxG_SCY_5_CLK			|\
			 ORxG_EHTR			|\
			 ORxG_TRLX)

#else	/* ! CONFIG_BOOT_ROM */

/*
 * Bank 0 - Flash (64 bit wide)
 */
#define CFG_BR0_PRELIM	((CFG_FLASH_BASE & BRx_BA_MSK)	|\
			 BRx_PS_64			|\
			 BRx_MS_GPCM_P			|\
			 BRx_V)

#define CFG_OR0_PRELIM	(P2SZ_TO_AM(CFG_FLASH_SIZE)	|\
			 ORxG_CSNT			|\
			 ORxG_ACS_DIV1			|\
			 ORxG_SCY_5_CLK			|\
			 ORxG_EHTR			|\
			 ORxG_TRLX)

/*
 * Bank 1 - Disk-On-Chip
 */
#define CFG_BR1_PRELIM	((CFG_DOC_BASE & BRx_BA_MSK)	|\
			 BRx_PS_8			|\
			 BRx_MS_GPCM_P			|\
			 BRx_V)

#define CFG_OR1_PRELIM	(P2SZ_TO_AM(CFG_DOC_SIZE)	|\
			 ORxG_CSNT			|\
			 ORxG_ACS_DIV1			|\
			 ORxG_SCY_5_CLK			|\
			 ORxG_EHTR			|\
			 ORxG_TRLX)

#endif /* CONFIG_BOOT_ROM */

/* Bank 2 - SDRAM
 */

#ifndef CFG_RAMBOOT
#define CFG_BR2_PRELIM	((CFG_SDRAM_BASE & BRx_BA_MSK)	|\
			 BRx_PS_64			|\
			 BRx_MS_SDRAM_P			|\
			 BRx_V)

	/* SDRAM initialization values for 8-column chips
	 */
#define CFG_OR2_8COL	(CFG_MIN_AM_MASK		|\
			 ORxS_BPD_4			|\
			 ORxS_ROWST_PBI0_A9		|\
			 ORxS_NUMR_12)

#define CFG_PSDMR_8COL	(PSDMR_SDAM_A13_IS_A5		|\
			 PSDMR_BSMA_A14_A16		|\
			 PSDMR_SDA10_PBI0_A10		|\
			 PSDMR_RFRC_7_CLK		|\
			 PSDMR_PRETOACT_2W		|\
			 PSDMR_ACTTORW_2W		|\
			 PSDMR_LDOTOPRE_1C		|\
			 PSDMR_WRC_1C			|\
			 PSDMR_CL_2)

	/* SDRAM initialization values for 9-column chips
	 */
#define CFG_OR2_9COL	(CFG_MIN_AM_MASK		|\
			 ORxS_BPD_4			|\
			 ORxS_ROWST_PBI0_A7		|\
			 ORxS_NUMR_13)

#define CFG_PSDMR_9COL	(PSDMR_SDAM_A14_IS_A5		|\
			 PSDMR_BSMA_A13_A15		|\
			 PSDMR_SDA10_PBI0_A9		|\
			 PSDMR_RFRC_7_CLK		|\
			 PSDMR_PRETOACT_2W		|\
			 PSDMR_ACTTORW_2W		|\
			 PSDMR_LDOTOPRE_1C		|\
			 PSDMR_WRC_1C			|\
			 PSDMR_CL_2)

#define CFG_OR2_PRELIM	 CFG_OR2_9COL
#define CFG_PSDMR	 CFG_PSDMR_9COL

#endif /* CFG_RAMBOOT */

#endif	/* __CONFIG_H */
