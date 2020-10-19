/*
 * (C) Copyright 2001, 2002, 2003
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* ------------------------------------------------------------------------- */
/*
 * Configuration settings for the A-3000 board (Artis Microsystems Inc.).
 * http://artismicro.com
 */

/* ------------------------------------------------------------------------- */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC824X		1
#define CONFIG_MPC8245		1
#define CONFIG_A3000		1


#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		9600
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_BOOTDELAY	5

#if 0
#define CONFIG_COMMANDS		( CONFIG_CMD_DFL | \
				  CFG_CMD_BEDBUG | \
				  CFG_CMD_BSP    | \
				  CFG_CMD_ELF    | \
				  CFG_CMD_I2C 	 | \
				  CFG_CMD_FLASH | \
				  CFG_CMD_BEDBUG | \
				  CFG_CMD_NET    | \
				  CFG_CMD_PCI )
#endif

#define CONFIG_COMMANDS		( CONFIG_CMD_DFL )

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) 	*/

#include <cmd_confdefs.h>


/*
 * Miscellaneous configurable options
 */
#undef CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"A3000> "		/* Monitor Command Prompt	*/
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/

/* Print Buffer Size
 */
#define CFG_PBSIZE	(CFG_CBSIZE + sizeof(CFG_PROMPT) + 16)
#define CFG_MAXARGS	8		/* Max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/
#define CFG_LOAD_ADDR	0x00400000	/* Default load address		*/

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_HARD_I2C		1		/* To enable I2C support	*/
#undef  CONFIG_SOFT_I2C				/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000		/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_PCI      		/* include pci support          */
#undef CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW    /* print pci devices @ startup  */

#define CONFIG_NET_MULTI		/* Multi ethernet cards support */

/* #define CONFIG_TULIP */
/* #define CONFIG_EEPRO100 */
#define CONFIG_NATSEMI

#define PCI_ENET0_IOADDR		0x80000000
#define PCI_ENET0_MEMADDR		0x80000000
#define PCI_ENET1_IOADDR		0x81000000
#define PCI_ENET1_MEMADDR		0x81000000
#define PCI_ENET2_IOADDR		0x82000000
#define PCI_ENET2_MEMADDR		0x82000000
#define PCI_ENET3_IOADDR		0x83000000
#define PCI_ENET3_MEMADDR		0x83000000


/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE	    0x00000000

#define CFG_FLASH_BASE0_PRELIM      0xFF000000      /* FLASH bank on RCS#0 */
#define CFG_FLASH_BASE1_PRELIM      0xFF000000      /* FLASH bank on RCS#1 */
#define CFG_FLASH_BASE  			CFG_FLASH_BASE0_PRELIM
#define CFG_FLASH_BANKS			{ CFG_FLASH_BASE0_PRELIM }

/* even though FLASHP_BASE is FF800000, with 4MB is RCS0, the
 * reset vector is actually located at FFB00100, but the 8245
 * takes care of us.
 */
#define CFG_RESET_ADDRESS   0xFFF00100

#define CFG_EUMB_ADDR	    0xFC000000

#define CFG_MONITOR_BASE    TEXT_BASE
#define CFG_MONITOR_LEN	    (256 << 10) /* Reserve 256 kB for Monitor	*/
#define CFG_MALLOC_LEN	    (128 << 10) /* Reserve 128 kB for malloc()	*/

#define CFG_MEMTEST_START   0x00004000	/* memtest works on		*/
#define CFG_MEMTEST_END	    0x02000000	/* 0 ... 32 MB in DRAM		*/

	/* Maximum amount of RAM.
	 */
#define CFG_MAX_RAM_SIZE    0x04000000	/* 0 .. 128 MB of (S)DRAM */


#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
#undef CFG_RAMBOOT
#else
#define CFG_RAMBOOT
#endif

/*
 * NS16550 Configuration
 */
#define CFG_NS16550
#define CFG_NS16550_SERIAL

#define CFG_NS16550_REG_SIZE	1

#define CFG_NS16550_CLK		get_bus_freq(0)

#define CFG_NS16550_COM1	(CFG_EUMB_ADDR + 0x4500)
#define CFG_NS16550_COM2	(CFG_EUMB_ADDR + 0x4600)

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */

/* #define CFG_MONITOR_BASE        TEXT_BASE */
/*#define CFG_GBL_DATA_SIZE    256*/
#define CFG_GBL_DATA_SIZE      128
#define CFG_INIT_RAM_ADDR     0x40000000
#define CFG_INIT_RAM_END      0x1000
#define CFG_GBL_DATA_OFFSET  (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)


/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 * For the detail description refer to the MPC8240 user's manual.
 */

#define CONFIG_SYS_CLK_FREQ  33333333	/* external frequency to pll */
#define CFG_HZ		     1000

	/* Bit-field values for MCCR1.
	 */
#define CFG_ROMNAL	    7
#define CFG_ROMFAL	    11
#define CFG_DBUS_SIZE       0x3

	/* Bit-field values for MCCR2.
	 */
#define CFG_TSWAIT	    0x5		    /* Transaction Start Wait States timer */
#define CFG_REFINT	    0x400	    /* Refresh interval	FIXME: was 0t430		*/

	/* Burst To Precharge. Bits of this value go to MCCR3 and MCCR4.
	 */
#define CFG_BSTOPRE	    121

	/* Bit-field values for MCCR3.
	 */
#define CFG_REFREC	    8	    /* Refresh to activate interval */

	/* Bit-field values for MCCR4.
	 */
#define CFG_PRETOACT	    3	    /* Precharge to activate interval FIXME: was 2	*/
#define CFG_ACTTOPRE	    5	    /* Activate to Precharge interval FIXME: was 5	*/
#define CFG_ACTORW	    3		/* FIXME was 2 */
#define CFG_SDMODE_CAS_LAT  3	    /* SDMODE CAS latancy */
#define CFG_SDMODE_WRAP	    0	    /* SDMODE wrap type	*/
#define CFG_REGISTERD_TYPE_BUFFER 1
#define CFG_EXTROM	    1
#define CFG_REGDIMM	    0

#define CFG_PGMAX           0x32 /* how long the 8240 reatins the currently accessed page in memory FIXME: was 0x32*/

#define CFG_SDRAM_DSCD	0x20	/* SDRAM data in sample clock delay - note bottom 3 bits MUST be 0 */

/* Memory bank settings.
 * Only bits 20-29 are actually used from these vales to set the
 * start/end addresses. The upper two bits will always be 0, and the lower
 * 20 bits will be 0x00000 for a start address, or 0xfffff for an end
 * address. Refer to the MPC8240 book.
 */

#define CFG_BANK0_START	    0x00000000
#define CFG_BANK0_END	    (CFG_MAX_RAM_SIZE - 1)
#define CFG_BANK0_ENABLE    1
#define CFG_BANK1_START	    0x3ff00000
#define CFG_BANK1_END	    0x3fffffff
#define CFG_BANK1_ENABLE    0
#define CFG_BANK2_START	    0x3ff00000
#define CFG_BANK2_END	    0x3fffffff
#define CFG_BANK2_ENABLE    0
#define CFG_BANK3_START	    0x3ff00000
#define CFG_BANK3_END	    0x3fffffff
#define CFG_BANK3_ENABLE    0
#define CFG_BANK4_START	    0x3ff00000
#define CFG_BANK4_END	    0x3fffffff
#define CFG_BANK4_ENABLE    0
#define CFG_BANK5_START	    0x3ff00000
#define CFG_BANK5_END	    0x3fffffff
#define CFG_BANK5_ENABLE    0
#define CFG_BANK6_START	    0x3ff00000
#define CFG_BANK6_END	    0x3fffffff
#define CFG_BANK6_ENABLE    0
#define CFG_BANK7_START	    0x3ff00000
#define CFG_BANK7_END	    0x3fffffff
#define CFG_BANK7_ENABLE    0

#define CFG_ODCR	    0xff

#define CFG_IBAT0L  (CFG_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT0U  (CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_IBAT1L  (CFG_INIT_RAM_ADDR | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT1U  (CFG_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)

#define CFG_IBAT2L  (0x80000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CFG_IBAT2U  (0x80000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_IBAT3L  (0xF0000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CFG_IBAT3U  (0xF0000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_DBAT0L  CFG_IBAT0L
#define CFG_DBAT0U  CFG_IBAT0U
#define CFG_DBAT1L  CFG_IBAT1L
#define CFG_DBAT1U  CFG_IBAT1U
#define CFG_DBAT2L  CFG_IBAT2L
#define CFG_DBAT2U  CFG_IBAT2U
#define CFG_DBAT3L  CFG_IBAT3L
#define CFG_DBAT3U  CFG_IBAT3U

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ	    (8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* Max number of flash banks		*/
#define CFG_MAX_FLASH_SECT	128	/* Max number of sectors per flash	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms) */


	/* Warining: environment is not EMBEDDED in the U-Boot code.
	 * It's stored in flash separately.
	 */
#define CFG_ENV_IS_IN_FLASH	    1
#define CFG_ENV_ADDR		0xFFFE0000
#define CFG_ENV_SIZE		0x00020000 /* Size of the Environment		*/
#define CFG_ENV_SECT_SIZE	0x00020000 /* Size of the Environment Sector	*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM		0x02	/* Software reboot			*/

#endif	/* __CONFIG_H */
