/*
 * (C) Copyright 2001
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
/* #define CONFIG_MPC8240	   1 */
#define CONFIG_MPC8245		1
#define CONFIG_EXALION		1

#if defined (CONFIG_MPC8240)
    /* #warning	 ---------- eXalion with MPC8240 --------------- */
#elif defined (CONFIG_MPC8245)
    /* #warning	 ++++++++++ eXalion with MPC8245 +++++++++++++++ */
#elif defined (CONFIG_MPC8245) && defined (CONFIG_MPC8245)
#error #### Both types of MPC824x defined (CONFIG_8240 and CONFIG_8245)
#else
#error #### Specific type of MPC824x must be defined (i.e. CONFIG_MPC8240)
#endif
/* older kernels need clock in MHz newer in Hz */
					/* #define CONFIG_CLOCKS_IN_MHZ 1 */ /* clocks passsed to Linux in MHz	    */
#undef CONFIG_CLOCKS_IN_MHZ

#define CONFIG_BOOTDELAY	10


						    /*#define CONFIG_DRAM_SPEED	      66   */ /* MHz			     */

#define CONFIG_COMMANDS		(   CONFIG_CMD_DFL  | \
				    CFG_CMD_FLASH   | \
				    CFG_CMD_SDRAM   | \
				    CFG_CMD_I2C	    | \
				    CFG_CMD_IDE	    | \
				    CFG_CMD_FAT	    | \
				    CFG_CMD_ENV	    | \
				    CFG_CMD_PCI )

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any)	*/
#include <cmd_confdefs.h>


/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP		1	/* undef to save memory		*/
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt	*/
#define CFG_CBSIZE		256	/* Console I/O Buffer Size	*/
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size	*/
#define CFG_MAXARGS		8	/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/
#define CFG_LOAD_ADDR		0x00100000	/* default load address		*/

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_MISC_INIT_R	1

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_MAX_RAM_SIZE	0x10000000	/* 1 GBytes - initdram() will	   */
					     /* return real value.		*/

#define CFG_RESET_ADDRESS	0xFFF00100

#undef	CFG_RAMBOOT
#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	    */
#define CFG_MONITOR_BASE	TEXT_BASE

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area
 */
#define CFG_INIT_DATA_SIZE	128

#define CFG_INIT_RAM_ADDR	0x40000000
#define CFG_INIT_RAM_END	0x1000
#define CFG_INIT_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_INIT_DATA_SIZE)

#define CFG_GBL_DATA_SIZE	 256	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET


#if defined (CONFIG_MPC8240)
#define CFG_FLASH_BASE	    0xFFE00000
#define CFG_FLASH_SIZE	    (2 * 1024 * 1024)	/* onboard 2MByte flash	    */
#elif defined (CONFIG_MPC8245)
#define CFG_FLASH_BASE	    0xFFC00000
#define CFG_FLASH_SIZE	    (4 * 1024 * 1024)	/* onboard 4MByte flash	    */
#else
#error Specific type of MPC824x must be defined (i.e. CONFIG_MPC8240)
#endif

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SECT_SIZE	0x20000 /* Size of one Flash sector */
#define CFG_ENV_SIZE		CFG_ENV_SECT_SIZE	/* Use one Flash sector for enviroment	*/
#define CFG_ENV_ADDR		0xFFFC0000
#define CFG_ENV_OFFSET		0	/* starting right at the beginning  */

#define CFG_MALLOC_LEN		(128 * 1024)	/* Reserve 128 kB for malloc()	*/

#define CFG_ALT_MEMTEST		1	/* use real memory test	    */
#define CFG_MEMTEST_START	0x00004000	/* memtest works on	    */
#define CFG_MEMTEST_END		0x02000000	/* 0 ... 32 MB in DRAM	    */

#define CFG_EUMB_ADDR		0xFC000000

/* #define CFG_ISA_MEM		   0xFD000000 */
#define CFG_ISA_IO		0xFE000000

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* Max number of flash banks	    */
#define CFG_MAX_FLASH_SECT	64	/* Max number of sectors per flash  */

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms) */
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms) */

#define FLASH_BASE0_PRELIM	CFG_FLASH_BASE
#define FLASH_BASE1_PRELIM	0


/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#define CFG_FLASH_CFI		1	/* Flash is CFI conformant		*/
#define CFG_FLASH_CFI_DRIVER	1	/* Use the common driver		*/
#define CFG_MAX_FLASH_SECT	64	/* max number of sectors on one chip	*/
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_FLASH_INCREMENT	0	/* there is only one bank		*/
#define CFG_FLASH_PROTECTION	1	/* use hardware protection		*/
#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/


/*-----------------------------------------------------------------------
 * PCI stuff
 */
#define CONFIG_PCI		1	/* include pci support		*/
#undef	CONFIG_PCI_PNP

#define CONFIG_NET_MULTI	1	/* Multi ethernet cards support */

#define CONFIG_EEPRO100		1

#define PCI_ENET0_MEMADDR	0x80000000	/* Intel 82559ER */
#define PCI_ENET0_IOADDR	0x80000000
#define PCI_ENET1_MEMADDR	0x81000000	/* Intel 82559ER */
#define PCI_ENET1_IOADDR	0x81000000
#define PCI_ENET2_MEMADDR	0x82000000	/* Broadcom BCM569xx */
#define PCI_ENET2_IOADDR	0x82000000
#define PCI_ENET3_MEMADDR	0x83000000	/* Broadcom BCM56xx */
#define PCI_ENET3_IOADDR	0x83000000

/*-----------------------------------------------------------------------
 * NS16550 Configuration
 */
#define CFG_NS16550		1
#define CFG_NS16550_SERIAL	1

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		38400

#define CFG_NS16550_REG_SIZE	1

#if (CONFIG_CONS_INDEX == 1)
#define CFG_NS16550_CLK		1843200 /* COM1 only !	*/
#else
#define CFG_NS16550_CLK ({ extern ulong get_bus_freq (ulong); get_bus_freq (0); })
#endif

#define CFG_NS16550_COM1	(CFG_ISA_IO + 0x3F8)
#define CFG_NS16550_COM2	(CFG_EUMB_ADDR + 0x4500)
#define CFG_NS16550_COM3	(CFG_EUMB_ADDR + 0x4600)

/*-----------------------------------------------------------------------
 * select i2c support configuration
 *
 * Supported configurations are {none, software, hardware} drivers.
 * If the software driver is chosen, there are some additional
 * configuration items that the driver uses to drive the port pins.
 */
#define CONFIG_HARD_I2C		1	/* To enable I2C support	*/
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

/*-----------------------------------------------------------------------
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
#define CFG_HZ			1000

#define CONFIG_SYS_CLK_FREQ	33333333	/* external frequency to pll	*/
#define CONFIG_PLL_PCI_TO_MEM_MULTIPLIER  2	/* for MPC8240 only		*/

				       /*#define CONFIG_133MHZ_DRAM	 1 */ /* For 133 MHZ DRAM only !!!!!!!!!!!    */

#if defined (CONFIG_MPC8245)
/* Bit-field values for PMCR2.							*/
#if defined (CONFIG_133MHZ_DRAM)
#define CFG_DLL_EXTEND		0x80	/* use DLL extended range - 133MHz only */
#define CFG_PCI_HOLD_DEL	0x20	/* delay and hold timing - 133MHz only	*/
#endif

/* Bit-field values for MIOCR1.							*/
#if !defined (CONFIG_133MHZ_DRAM)
#define CFG_DLL_MAX_DELAY	0x04	/*  longer DLL delay line - 66MHz only	*/
#endif
/* Bit-field values for MIOCR2.							*/
#define CFG_SDRAM_DSCD		0x20	/* SDRAM data in sample clock delay	*/
					/*	- note bottom 3 bits MUST be 0	*/
#endif

/* Bit-field values for MCCR1.							*/
#define CFG_ROMNAL		7	/*rom/flash next access time		*/
#define CFG_ROMFAL	       11	/*rom/flash access time			*/

/* Bit-field values for MCCR2.							*/
#define CFG_TSWAIT		0x5	/* Transaction Start Wait States timer	*/
#if defined (CONFIG_133MHZ_DRAM)
#define CFG_REFINT		1300	/* no of clock cycles between CBR	*/
#else  /* refresh cycles */
#define CFG_REFINT		750
#endif

/* Burst To Precharge. Bits of this value go to MCCR3 and MCCR4.		*/
#if defined (CONFIG_133MHZ_DRAM)
#define CFG_BSTOPRE		1023
#else
#define CFG_BSTOPRE		250
#endif

/* Bit-field values for MCCR3.							*/
/* the following are for SDRAM only						*/

#if defined (CONFIG_133MHZ_DRAM)
#define CFG_REFREC		9	/* Refresh to activate interval		*/
#else
#define CFG_REFREC		5	/* Refresh to activate interval		*/
#endif
#if defined (CONFIG_MPC8240)
#define CFG_RDLAT		2	/* data latency from read command	*/
#endif

/* Bit-field values for MCCR4.	*/
#if defined (CONFIG_133MHZ_DRAM)
#define CFG_PRETOACT		3	/* Precharge to activate interval	*/
#define CFG_ACTTOPRE		7	/* Activate to Precharge interval	*/
#define CFG_ACTORW		5	/* Activate to R/W			*/
#define CFG_SDMODE_CAS_LAT	3	/* SDMODE CAS latency			*/
#else
#if 0
#define CFG_PRETOACT		2	/* Precharge to activate interval	*/
#define CFG_ACTTOPRE		3	/* Activate to Precharge interval	*/
#define CFG_ACTORW		3	/* Activate to R/W			*/
#define CFG_SDMODE_CAS_LAT	2	/* SDMODE CAS latency			*/
#endif
#define CFG_PRETOACT		2	/* Precharge to activate interval	*/
#define CFG_ACTTOPRE		5	/* Activate to Precharge interval	*/
#define CFG_ACTORW		3	/* Activate to R/W			*/
#define CFG_SDMODE_CAS_LAT	3	/* SDMODE CAS latency			*/
#endif
#define CFG_SDMODE_WRAP		0	/* SDMODE wrap type			*/
#define CFG_SDMODE_BURSTLEN	2	/* SDMODE Burst length 2=4, 3=8		*/
#define CFG_REGDIMM		0
#if defined (CONFIG_MPC8240)
#define CFG_REGISTERD_TYPE_BUFFER   0
#elif defined (CONFIG_MPC8245)
#define CFG_REGISTERD_TYPE_BUFFER   1
#define CFG_EXTROM		    0
#else
#error Specific type of MPC824x must be defined (i.e. CONFIG_MPC8240)
#endif


/*-----------------------------------------------------------------------
 memory bank settings
 * only bits 20-29 are actually used from these vales to set the
 * start/end address the upper two bits will be 0, and the lower 20
 * bits will be set to 0x00000 for a start address, or 0xfffff for an
 * end address
 */
#define CFG_BANK0_START		0x00000000
#define CFG_BANK0_END		(CFG_MAX_RAM_SIZE - 1)
#define CFG_BANK0_ENABLE	1
#define CFG_BANK1_START		0x3ff00000
#define CFG_BANK1_END		0x3fffffff
#define CFG_BANK1_ENABLE	0
#define CFG_BANK2_START		0x3ff00000
#define CFG_BANK2_END		0x3fffffff
#define CFG_BANK2_ENABLE	0
#define CFG_BANK3_START		0x3ff00000
#define CFG_BANK3_END		0x3fffffff
#define CFG_BANK3_ENABLE	0
#define CFG_BANK4_START		0x00000000
#define CFG_BANK4_END		0x00000000
#define CFG_BANK4_ENABLE	0
#define CFG_BANK5_START		0x00000000
#define CFG_BANK5_END		0x00000000
#define CFG_BANK5_ENABLE	0
#define CFG_BANK6_START		0x00000000
#define CFG_BANK6_END		0x00000000
#define CFG_BANK6_ENABLE	0
#define CFG_BANK7_START		0x00000000
#define CFG_BANK7_END		0x00000000
#define CFG_BANK7_ENABLE	0

/*-----------------------------------------------------------------------
 * Memory bank enable bitmask, specifying which of the banks defined above
 are actually present. MSB is for bank #7, LSB is for bank #0.
 */
#define CFG_BANK_ENABLE		0x01

#if defined (CONFIG_MPC8240)
#define CFG_ODCR		0xDF	/* configures line driver impedances,	*/
					/* see 8240 book for bit definitions	*/
#elif defined (CONFIG_MPC8245)
#if defined (CONFIG_133MHZ_DRAM)
#define CFG_ODCR		0xFE	/* configures line driver impedances - 133MHz	*/
#else
#define CFG_ODCR		0xDE	/* configures line driver impedances - 66MHz	*/
#endif
#else
#error Specific type of MPC824x must be defined (i.e. CONFIG_MPC8240)
#endif

#define CFG_PGMAX		0x32	/* how long the 8240 retains the	*/
					/* currently accessed page in memory	*/
					/* see 8240 book for details		*/

/*-----------------------------------------------------------------------
 * Block Address Translation (BAT) register settings.
 */
/* SDRAM 0 - 256MB */
#define CFG_IBAT0L	(CFG_SDRAM_BASE | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT0U	(CFG_SDRAM_BASE | BATU_BL_256M | BATU_VS | BATU_VP)

/* stack in DCACHE @ 1GB (no backing mem) */
#define CFG_IBAT1L	(CFG_INIT_RAM_ADDR | BATL_PP_10 | BATL_MEMCOHERENCE)
#define CFG_IBAT1U	(CFG_INIT_RAM_ADDR | BATU_BL_128K | BATU_VS | BATU_VP)

/* PCI memory */
#define CFG_IBAT2L	(0x80000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CFG_IBAT2U	(0x80000000 | BATU_BL_256M | BATU_VS | BATU_VP)

/* Flash, config addrs, etc */
#define CFG_IBAT3L	(0xF0000000 | BATL_PP_10 | BATL_CACHEINHIBIT)
#define CFG_IBAT3U	(0xF0000000 | BATU_BL_256M | BATU_VS | BATU_VP)

#define CFG_DBAT0L	CFG_IBAT0L
#define CFG_DBAT0U	CFG_IBAT0U
#define CFG_DBAT1L	CFG_IBAT1L
#define CFG_DBAT1U	CFG_IBAT1U
#define CFG_DBAT2L	CFG_IBAT2L
#define CFG_DBAT2U	CFG_IBAT2U
#define CFG_DBAT3L	CFG_IBAT3L
#define CFG_DBAT3U	CFG_IBAT3U


/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif


/*-----------------------------------------------------------------------
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM		0x02	/* Software reboot			*/


/* values according to the manual */
#define CONFIG_DRAM_50MHZ	1
#define CONFIG_SDRAM_50MHZ

#undef	NR_8259_INTS
#define NR_8259_INTS		1

/*-----------------------------------------------------------------------
 * IDE/ATA stuff
 */
#define CFG_IDE_MAXBUS	    1	/* max. 2 IDE busses	*/
#define CFG_IDE_MAXDEVICE   (CFG_IDE_MAXBUS*1)	/* max. 2 drives per IDE bus */

#define CFG_ATA_BASE_ADDR   CFG_ISA_IO	/* base address */
#define CFG_ATA_IDE0_OFFSET 0x01F0	/* ide0 offste */
#define CFG_ATA_IDE1_OFFSET 0x0170	/* ide1 offset */
#define CFG_ATA_DATA_OFFSET 0	/* data reg offset  */
#define CFG_ATA_REG_OFFSET  0	/* reg offset */
#define CFG_ATA_ALT_OFFSET  0x200	/* alternate register offset */

#define CONFIG_ATAPI

#undef	CONFIG_IDE_8xx_DIRECT	/* no pcmcia interface required */
#undef	CONFIG_IDE_LED		/* no led for ide supported	*/
#undef	CONFIG_IDE_RESET	/* reset for ide supported...	 */
#undef	CONFIG_IDE_RESET_ROUTINE	/* with a special reset function */

/*-----------------------------------------------------------------------
 * DISK Partition support
 */
#define CONFIG_DOS_PARTITION

/*-----------------------------------------------------------------------
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#endif /* __CONFIG_H */
