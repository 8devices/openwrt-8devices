/*
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Klaus Heydeck, Kieback & Peter GmbH & Co KG, heydeck@kieback-peter.de
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
 * Derived from ../tqm8xx/tqm8xx.c
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC859T		1	/* This is a MPC859T CPU		*/
#define CONFIG_KUP4X		1	/* ...on a KUP4X module			*/

#define CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1			*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		115200	/* console baudrate			*/
#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled			*/
#else
#define CONFIG_BOOTDELAY	1	/* autoboot after 1 second		*/
#endif

#define CONFIG_BOARD_TYPES	1	/* support board types			*/

#define CFG_8XX_FACT		8	/* Multiply by 8			*/
#define CFG_8XX_XIN		16000000	/* 16 MHz in			*/


#define MPC8XX_HZ ((CFG_8XX_XIN) * (CFG_8XX_FACT))

/* should ALWAYS define this, measure_gclk in speed.c is unreliable */
/* in general, we always know this for FADS+new ADS anyway */
#define CONFIG_8xx_GCLK_FREQ	 MPC8XX_HZ


#undef	CONFIG_BOOTARGS


#define CONFIG_EXTRA_ENV_SETTINGS						\
"slot_a_boot=setenv bootargs root=/dev/hda2 ip=off;"				\
  "run addhw;diskboot 200000 0:1;bootm 200000\0"				\
"usb_boot=setenv bootargs root=/dev/sda2 ip=off;\
 run addhw; sleep 2; usb reset; usb scan; usbboot 200000 0:1;\
 usb stop; bootm 200000\0"      \
"nfs_boot=dhcp;run nfsargs addip addhw;bootm 200000\0"				\
"panic_boot=echo No Bootdevice !!! reset\0"					\
"nfsargs=setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath}\0"	\
"ramargs=setenv bootargs root=/dev/ram rw\0"					\
"addip=setenv bootargs ${bootargs} ip=${ipaddr}:${serverip}:${gatewayip}"	\
 ":${netmask}:${hostname}:${netdev}:off\0"					\
"addhw=setenv bootargs ${bootargs} hw=${hw} key1=${key1} panic=1\0"		\
"netdev=eth0\0"									\
"silent=1\0"									\
"load=tftp 200000 bootloader-4x.bitmap;tftp 100000 bootloader-4x.bin\0"		\
"update=protect off 1:0-5;era 1:0-5;cp.b 100000 40000000 ${filesize};"		\
 "cp.b 200000 40040000 14000\0"

#define CONFIG_BOOTCOMMAND  \
    "run usb_boot;run_slot_a_boot;run nfs_boot;run panic_boot"


#define CONFIG_MISC_INIT_R	1
#define CONFIG_MISC_INIT_F	1

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#define	CONFIG_WATCHDOG		1	/* watchdog enabled		*/

#define CONFIG_STATUS_LED	1	/* Status LED enabled		*/

#undef	CONFIG_CAN_DRIVER		/* CAN Driver support disabled	*/

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT | CONFIG_BOOTP_BOOTFILESIZE)

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

/*
 * enable I2C and select the hardware/software driver
 */
#undef	CONFIG_HARD_I2C			/* I2C with hardware support	*/
#define	CONFIG_SOFT_I2C         1	/* I2C bit-banged		*/

#define CFG_I2C_SPEED		93000	/* 93 kHz is supposed to work	*/
#define CFG_I2C_SLAVE		0xFE

#ifdef CONFIG_SOFT_I2C
/*
 * Software (bit-bang) I2C driver configuration
 */
#define PB_SCL		0x00000020	/* PB 26 */
#define PB_SDA		0x00000010	/* PB 27 */

#define I2C_INIT	(immr->im_cpm.cp_pbdir |=  PB_SCL)
#define I2C_ACTIVE	(immr->im_cpm.cp_pbdir |=  PB_SDA)
#define I2C_TRISTATE	(immr->im_cpm.cp_pbdir &= ~PB_SDA)
#define I2C_READ	((immr->im_cpm.cp_pbdat & PB_SDA) != 0)
#define I2C_SDA(bit)	if(bit) immr->im_cpm.cp_pbdat |=  PB_SDA; \
			else    immr->im_cpm.cp_pbdat &= ~PB_SDA
#define I2C_SCL(bit)	if(bit) immr->im_cpm.cp_pbdat |=  PB_SCL; \
			else    immr->im_cpm.cp_pbdat &= ~PB_SCL
#define I2C_DELAY	udelay(2)	/* 1/4 I2C clock duration */
#endif	/* CONFIG_SOFT_I2C */


/*-----------------------------------------------------------------------
 * I2C Configuration
 */

#define CFG_I2C_PICIO_ADDR	0x21	/* PCF8574 IO Expander			*/
#define CFG_I2C_RTC_ADDR	0x51	/* PCF8563 RTC				*/


/* List of I2C addresses to be verified by POST */

#define I2C_ADDR_LIST	{CFG_I2C_PICIO_ADDR,	\
			CFG_I2C_RTC_ADDR,	\
			}


#define CONFIG_RTC_PCF8563		/* use Philips PCF8563 RTC	*/

#define CFG_DISCOVER_PHY
#define CONFIG_MII

#if 0
#define CONFIG_ETHADDR                  00:0B:64:80:00:00 /* our OUI from IEEE */
#endif
#undef CONFIG_KUP4K_LOGO

/* Define to allow the user to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE


#if 1
/* POST support */

#define CONFIG_POST		(CFG_POST_CPU	   | \
				 CFG_POST_RTC	   | \
				 CFG_POST_I2C)

#ifdef CONFIG_POST
#define CFG_CMD_POST_DIAG CFG_CMD_DIAG
#else
#define CFG_CMD_POST_DIAG 0
#endif
#endif

#define CONFIG_COMMANDS	      ( CONFIG_CMD_DFL	| \
				CFG_CMD_DATE	| \
				CFG_CMD_DHCP	| \
				CFG_CMD_FAT	| \
				CFG_CMD_I2C	| \
				CFG_CMD_IDE	| \
				CFG_CMD_NFS	| \
				CFG_CMD_POST_DIAG	| \
				CFG_CMD_SNTP	| \
				CFG_CMD_USB	)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

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

#define CFG_MEMTEST_START	0x000400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x003C00000	/* 4 ... 60 MB in DRAM	*/
#define CFG_LOAD_ADDR		0x200000	/* default load address */

#define CFG_HZ		1000		/* decrementer freq: 1 ms ticks */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 115200 }

#define CFG_CONSOLE_INFO_QUIET 1

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CFG_IMMR		0xFFF00000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x2F00	/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	64	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0x40000000
#define CFG_MONITOR_LEN		(192 << 10)	/* Reserve 256 kB for Monitor	*/
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	19	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_OFFSET		0x40000 /*   Offset   of Environment Sector	*/
#define CFG_ENV_SIZE		0x1000	/* Total Size of Environment Sector	*/
#define CFG_ENV_SECT_SIZE	0x10000

/* Address and size of Redundant Environment Sector	*/
#if 0
#define CFG_ENV_OFFSET_REDUND	(CFG_ENV_OFFSET+CFG_ENV_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#endif
/*-----------------------------------------------------------------------
 * Hardware Information Block
 */
#if 1
#define CFG_HWINFO_OFFSET	0x000F0000	/* offset of HW Info block */
#define CFG_HWINFO_SIZE		0x00000100	/* size   of HW Info block */
#define CFG_HWINFO_MAGIC	0x4B26500D	/* 'K&P<CR>' */
#endif
/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if 0 && defined(CONFIG_WATCHDOG)       /* KUP uses external TPS3705 WD */
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CFG_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC00)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)


/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CFG_PISCR	(PISCR_PS | PISCR_PITF)


/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register	15-30
 *-----------------------------------------------------------------------
 * set the PLL, the low-power modes and the reset control (15-29)
 */
#define CFG_PLPRCR	((CFG_8XX_FACT << PLPRCR_MFI_SHIFT) |	\
				PLPRCR_SPLSS | PLPRCR_TEXPS)


/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF00
#define CFG_SCCR	(SCCR_TBS | SCCR_EBDF01 |  \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00)

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 *
 */

/* KUP4K use both slots, SLOT_A as "primary". */
#define CONFIG_PCMCIA_SLOT_A 1

#define CFG_PCMCIA_MEM_ADDR	(0xE0000000)
#define CFG_PCMCIA_MEM_SIZE	( 64 << 20 )
#define CFG_PCMCIA_DMA_ADDR	(0xE4000000)
#define CFG_PCMCIA_DMA_SIZE	( 64 << 20 )
#define CFG_PCMCIA_ATTRB_ADDR	(0xE8000000)
#define CFG_PCMCIA_ATTRB_SIZE	( 64 << 20 )
#define CFG_PCMCIA_IO_ADDR	(0xEC000000)
#define CFG_PCMCIA_IO_SIZE	( 64 << 20 )

#define PCMCIA_SOCKETS_NO 1
#define PCMCIA_MEM_WIN_NO 8
/*-----------------------------------------------------------------------
 * IDE/ATA stuff (Supports IDE harddisk on PCMCIA Adapter)
 *-----------------------------------------------------------------------
 */

#define CONFIG_IDE_8xx_PCCARD	1	/* Use IDE with PC Card Adapter */

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE	 not supported	*/
#define CONFIG_IDE_LED			1   /* LED   for ide supported	*/
#undef	CONFIG_IDE_RESET		/* reset for ide not supported	*/

#define CFG_IDE_MAXBUS		1
#define CFG_IDE_MAXDEVICE	2

#define CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_IDE1_OFFSET	(4 * CFG_PCMCIA_MEM_SIZE)

#define CFG_ATA_BASE_ADDR	CFG_PCMCIA_MEM_ADDR

/* Offset for data I/O			*/
#define CFG_ATA_DATA_OFFSET	(CFG_PCMCIA_MEM_SIZE + 0x320)

/* Offset for normal register accesses	*/
#define CFG_ATA_REG_OFFSET	(2 * CFG_PCMCIA_MEM_SIZE + 0x320)

/* Offset for alternate registers	*/
#define CFG_ATA_ALT_OFFSET	0x0100


/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
#define CFG_DER 0

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */
#define FLASH_BASE0_PRELIM	0x40000000	/* FLASH bank #0	*/

/* used to re-map FLASH both when starting from SRAM or FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
#define CFG_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CFG_PRELIM_OR_AM	0xE0000000	/* OR addr mask */

/*
 * FLASH timing:
 */
#define CFG_OR_TIMING_FLASH	(OR_ACS_DIV1  | OR_TRLX | OR_CSNT_SAM | \
				 OR_SCY_2_CLK | OR_EHTR | OR_BI)

#define CFG_OR0_REMAP	(CFG_REMAP_OR_AM  | CFG_OR_TIMING_FLASH)
#define CFG_OR0_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_FLASH)
#define CFG_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_PS_16 | BR_V )


/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CFG_OR_TIMING_SDRAM	0x00000A00


#define CFG_MPTPR 0x400

/*
 * MAMR settings for SDRAM
 */
#define CFG_MAMR 0x80802114


/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/


#define CONFIG_AUTOBOOT_KEYED		/* use key strings to stop autoboot	*/
#if 0
#define CONFIG_AUTOBOOT_PROMPT		"Boote in %d Sekunden - stop mit \"2\"\n"
#endif
#define CONFIG_AUTOBOOT_STOP_STR	"."	/* easy to stop for now		*/
#define CONFIG_SILENT_CONSOLE	1

#define CONFIG_USB_STORAGE	1
#define CONFIG_USB_SL811HS	1

#endif	/* __CONFIG_H */
