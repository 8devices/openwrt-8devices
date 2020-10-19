/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 * Stephan Linz <linz@li-pro.net>
 *
 * CompactFlash/IDE:
 * (C) Copyright 2004, Shlomo Kut <skut@vyyo.com>
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

#ifndef __CONFIG_H
#define __CONFIG_H

/***********************************************************************
 * Include the whole NIOS CPU configuration.
 *
 * !!! HAVE TO BE HERE !!! DON'T MOVE THIS PART !!!
 *
 ***********************************************************************/

#if	defined(CONFIG_NIOS_SAFE_32)
#include <configs/DK1C20_safe_32.h>
#elif	defined(CONFIG_NIOS_STANDARD_32)
#include <configs/DK1C20_standard_32.h>
#else
#error *** CFG_ERROR: you have to setup right NIOS CPU configuration
#endif

/*------------------------------------------------------------------------
 * BOARD/CPU -- TOP-LEVEL
 *----------------------------------------------------------------------*/
#define CONFIG_NIOS		1		/* NIOS-32 core		*/
#define	CONFIG_DK1C20		1		/* Cyclone DK-1C20 board*/
#define CONFIG_SYS_CLK_FREQ	CFG_NIOS_CPU_CLK/* 50 MHz core clock	*/
#define	CFG_HZ			1000		/* 1 msec time tick	*/
#undef  CFG_CLKS_IN_HZ
#define	CONFIG_BOARD_EARLY_INIT_F 1	/* enable early board-spec. init*/

/*------------------------------------------------------------------------
 * BASE ADDRESSES / SIZE (Flash, SRAM, SDRAM)
 *----------------------------------------------------------------------*/
#if	(CFG_NIOS_CPU_SDRAM_SIZE != 0)

#define CFG_SDRAM_BASE		CFG_NIOS_CPU_SDRAM_BASE
#define CFG_SDRAM_SIZE		CFG_NIOS_CPU_SDRAM_SIZE

#else
#error *** CFG_ERROR: you have to setup any SDRAM in NIOS CPU config
#endif

#define CFG_SRAM_BASE		CFG_NIOS_CPU_SRAM_BASE
#define CFG_SRAM_SIZE		CFG_NIOS_CPU_SRAM_SIZE
#define CFG_VECT_BASE		CFG_NIOS_CPU_VEC_BASE

/*------------------------------------------------------------------------
 * MEMORY ORGANIZATION - For the most part, you can put things pretty
 * much anywhere. This is pretty flexible for Nios. So here we make some
 * arbitrary choices & assume that the monitor is placed at the end of
 * a memory resource (so you must make sure TEXT_BASE is chosen
 * appropriately).
 *
 * 	-The heap is placed below the monitor.
 * 	-Global data is placed below the heap.
 * 	-The stack is placed below global data (&grows down).
 *----------------------------------------------------------------------*/
#define CFG_MONITOR_LEN		(256 * 1024)	/* Reserve 256k		*/
#define CFG_GBL_DATA_SIZE	128		/* Global data size rsvd*/
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)

#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MALLOC_BASE		(CFG_MONITOR_BASE - CFG_MALLOC_LEN)
#define CFG_GBL_DATA_OFFSET	(CFG_MALLOC_BASE - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP		CFG_GBL_DATA_OFFSET

/*------------------------------------------------------------------------
 * FLASH (AM29LV065D)
 *----------------------------------------------------------------------*/
#if	(CFG_NIOS_CPU_FLASH_SIZE != 0)

#define CFG_FLASH_BASE		CFG_NIOS_CPU_FLASH_BASE
#define CFG_FLASH_SIZE		CFG_NIOS_CPU_FLASH_SIZE
#define CFG_MAX_FLASH_SECT	128		/* Max # sects per bank */
#define CFG_MAX_FLASH_BANKS	1		/* Max # of flash banks */
#define CFG_FLASH_ERASE_TOUT	8000		/* Erase timeout (msec) */
#define CFG_FLASH_WRITE_TOUT	100		/* Write timeout (msec) */
#define CFG_FLASH_WORD_SIZE	unsigned char	/* flash word size	*/

#else
#error *** CFG_ERROR: you have to setup any Flash memory in NIOS CPU config
#endif

/*------------------------------------------------------------------------
 * ENVIRONMENT
 *----------------------------------------------------------------------*/
#if	(CFG_NIOS_CPU_FLASH_SIZE != 0)

#define	CFG_ENV_IS_IN_FLASH	1		/* Environment in flash */
#define CFG_ENV_ADDR		CFG_FLASH_BASE	/* Mem addr of env	*/
#define CFG_ENV_SIZE		(64 * 1024)	/* 64 KByte (1 sector)	*/
#define CONFIG_ENV_OVERWRITE			/* Serial/eth change Ok */

#else
#define	CFG_ENV_IS_NOWHERE	1		/* NO Environment	*/
#endif

/*------------------------------------------------------------------------
 * CONSOLE
 *----------------------------------------------------------------------*/
#if	(CFG_NIOS_CPU_UART_NUMS != 0)

#define CFG_NIOS_CONSOLE	CFG_NIOS_CPU_UART0 /* 1st UART is Cons. */

#if	(CFG_NIOS_CPU_UART0_BR != 0)
#define CFG_NIOS_FIXEDBAUD	1		   /* Baudrate is fixed	*/
#define CONFIG_BAUDRATE		CFG_NIOS_CPU_UART0_BR
#else
#undef	CFG_NIOS_FIXEDBAUD
#define CONFIG_BAUDRATE		115200
#endif

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#else
#error *** CFG_ERROR: you have to setup at least one UART in NIOS CPU config
#endif

/*------------------------------------------------------------------------
 * TIMER FOR TIMEBASE -- Nios doesn't have the equivalent of ppc  PIT,
 * so an avalon bus timer is required.
 *----------------------------------------------------------------------*/
#if	(CFG_NIOS_CPU_TIMER_NUMS != 0)

#if	(CFG_NIOS_CPU_TICK_TIMER == 0)

#define CFG_NIOS_TMRBASE	CFG_NIOS_CPU_TIMER0 /* TIMER0 as tick	*/
#define CFG_NIOS_TMRIRQ		CFG_NIOS_CPU_TIMER0_IRQ

#if	(CFG_NIOS_CPU_TIMER0_FP == 1)		    /* fixed period */

#if	(CFG_NIOS_CPU_TIMER0_PER >= CFG_HZ)
#define CFG_NIOS_TMRMS		(CFG_NIOS_CPU_TIMER0_PER / CFG_HZ)
#else
#error *** CFG_ERROR: you have to use a timer periode greater than CFG_HZ
#endif

#undef	CFG_NIOS_TMRCNT	/* no preloadable counter value */

#elif	(CFG_NIOS_CPU_TIMER0_FP == 0)		    /* variable period */

#if	(CFG_HZ <= 1000)
#define CFG_NIOS_TMRMS		(1000 / CFG_HZ)
#else
#error *** CFG_ERROR: sorry, CFG_HZ have to be less than 1000
#endif

#define	CFG_NIOS_TMRCNT		(CONFIG_SYS_CLK_FREQ / CFG_HZ)

#else
#error *** CFG_ERROR: you have to define CFG_NIOS_CPU_TIMER0_FP correct
#endif

#elif	(CFG_NIOS_CPU_TICK_TIMER == 1)

#define CFG_NIOS_TMRBASE	CFG_NIOS_CPU_TIMER1 /* TIMER1 as tick	*/
#define CFG_NIOS_TMRIRQ		CFG_NIOS_CPU_TIMER1_IRQ

#if	(CFG_NIOS_CPU_TIMER1_FP == 1)		    /* fixed period */

#if	(CFG_NIOS_CPU_TIMER1_PER >= CFG_HZ)
#define CFG_NIOS_TMRMS		(CFG_NIOS_CPU_TIMER1_PER / CFG_HZ)
#else
#error *** CFG_ERROR: you have to use a timer periode greater than CFG_HZ
#endif

#undef	CFG_NIOS_TMRCNT	/* no preloadable counter value */

#elif	(CFG_NIOS_CPU_TIMER1_FP == 0)		    /* variable period */

#if	(CFG_HZ <= 1000)
#define CFG_NIOS_TMRMS		(1000 / CFG_HZ)
#else
#error *** CFG_ERROR: sorry, CFG_HZ have to be less than 1000
#endif

#define	CFG_NIOS_TMRCNT		(CONFIG_SYS_CLK_FREQ / CFG_HZ)

#else
#error *** CFG_ERROR: you have to define CFG_NIOS_CPU_TIMER1_FP correct
#endif

#endif	/* CFG_NIOS_CPU_TICK_TIMER */

#else
#error *** CFG_ERROR: you have to setup at least one TIMER in NIOS CPU config
#endif

/*------------------------------------------------------------------------
 * Ethernet
 *----------------------------------------------------------------------*/
#if	(CFG_NIOS_CPU_LAN_NUMS == 1)

#if	(CFG_NIOS_CPU_LAN0_TYPE == 0)		/* LAN91C111		*/

#define	CONFIG_DRIVER_SMC91111			/* Using SMC91c111	*/
#undef	CONFIG_SMC91111_EXT_PHY			/* Internal PHY		*/
#define	CONFIG_SMC91111_BASE	(CFG_NIOS_CPU_LAN0_BASE + CFG_NIOS_CPU_LAN0_OFFS)

#if	(CFG_NIOS_CPU_LAN0_BUSW == 32)
#define	CONFIG_SMC_USE_32_BIT	1
#else	/* no */
#undef	CONFIG_SMC_USE_32_BIT
#endif

#elif	(CFG_NIOS_CPU_LAN0_TYPE == 1)		/* CS8900A		*/

	/********************************************/
	/* !!! CS8900 is __not__ tested on NIOS !!! */
	/********************************************/
#define	CONFIG_DRIVER_CS8900			/* Using CS8900		*/
#define	CS8900_BASE		(CFG_NIOS_CPU_LAN0_BASE + CFG_NIOS_CPU_LAN0_OFFS)

#if	(CFG_NIOS_CPU_LAN0_BUSW == 32)
#undef	CS8900_BUS16
#define	CS8900_BUS32		1
#else	/* no */
#define	CS8900_BUS16		1
#undef	CS8900_BUS32
#endif

#else
#error *** CFG_ERROR: invalid LAN0 chip type, check your NIOS CPU config
#endif

#define CONFIG_ETHADDR		08:00:3e:26:0a:5b
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.2.21
#define CONFIG_SERVERIP		192.168.2.16

#else
#error *** CFG_ERROR: you have to setup just one LAN only or expand your config.h
#endif

/*------------------------------------------------------------------------
 * STATUS LEDs
 *----------------------------------------------------------------------*/
#if	(CFG_NIOS_CPU_PIO_NUMS != 0)

#if	(CFG_NIOS_CPU_LED_PIO == 0)

#error *** CFG_ERROR: status LEDs at PIO0 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_LED_PIO == 1)

#error *** CFG_ERROR: status LEDs at PIO1 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_LED_PIO == 2)

#define	STATUS_LED_BASE			CFG_NIOS_CPU_PIO2
#define	STATUS_LED_BITS			CFG_NIOS_CPU_PIO2_BITS
#define	STATUS_LED_ACTIVE		1 /* LED on for bit == 1 */

#if	(CFG_NIOS_CPU_PIO2_TYPE == 1)
#define	STATUS_LED_WRONLY		1
#else
#undef	STATUS_LED_WRONLY
#endif

#elif	(CFG_NIOS_CPU_LED_PIO == 3)

#error *** CFG_ERROR: status LEDs at PIO3 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_LED_PIO == 4)

#error *** CFG_ERROR: status LEDs at PIO4 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_LED_PIO == 5)

#error *** CFG_ERROR: status LEDs at PIO5 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_LED_PIO == 6)

#error *** CFG_ERROR: status LEDs at PIO6 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_LED_PIO == 7)

#error *** CFG_ERROR: status LEDs at PIO7 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_LED_PIO == 8)

#error *** CFG_ERROR: status LEDs at PIO8 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_LED_PIO == 9)

#error *** CFG_ERROR: status LEDs at PIO9 not supported, expand your config.h

#else
#error *** CFG_ERROR: you have to set CFG_NIOS_CPU_LED_PIO in right case
#endif

#define	CONFIG_STATUS_LED		1 /* enable status led driver */

#define	STATUS_LED_BIT			(1 << 0)	/* LED[0] */
#define	STATUS_LED_STATE		STATUS_LED_BLINKING
#define	STATUS_LED_BOOT_STATE		STATUS_LED_OFF
#define	STATUS_LED_PERIOD		(CFG_HZ / 10)	/* ca. 1 Hz */
#define	STATUS_LED_BOOT			0		/* boot LED */

#if	(STATUS_LED_BITS > 1)
#define	STATUS_LED_BIT1			(1 << 1)	/* LED[1] */
#define	STATUS_LED_STATE1		STATUS_LED_OFF
#define	STATUS_LED_PERIOD1		(CFG_HZ / 50)	/* ca. 5 Hz */
#define	STATUS_LED_RED			1		/* fail LED */
#endif

#if	(STATUS_LED_BITS > 2)
#define	STATUS_LED_BIT2			(1 << 2)	/* LED[2] */
#define	STATUS_LED_STATE2		STATUS_LED_OFF
#define	STATUS_LED_PERIOD2		(CFG_HZ / 10)	/* ca. 1 Hz */
#define	STATUS_LED_YELLOW		2		/* info LED */
#endif

#if	(STATUS_LED_BITS > 3)
#define	STATUS_LED_BIT3			(1 << 3)	/* LED[3] */
#define	STATUS_LED_STATE3		STATUS_LED_OFF
#define	STATUS_LED_PERIOD3		(CFG_HZ / 10)	/* ca. 1 Hz */
#define	STATUS_LED_GREEN		3		/* info LED */
#endif

#define	STATUS_LED_PAR			1 /* makes status_led.h happy */

#endif	/* CFG_NIOS_CPU_PIO_NUMS */

/*------------------------------------------------------------------------
 * SEVEN SEGMENT LED DISPLAY
 *----------------------------------------------------------------------*/
#if	(CFG_NIOS_CPU_PIO_NUMS != 0)

#if	(CFG_NIOS_CPU_SEVENSEG_PIO == 0)

#error *** CFG_ERROR: seven segment display at PIO0 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_SEVENSEG_PIO == 1)

#error *** CFG_ERROR: seven segment display at PIO1 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_SEVENSEG_PIO == 2)

#error *** CFG_ERROR: seven segment display at PIO2 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_SEVENSEG_PIO == 3)

#define	SEVENSEG_BASE			CFG_NIOS_CPU_PIO3
#define	SEVENSEG_BITS			CFG_NIOS_CPU_PIO3_BITS
#define	SEVENSEG_ACTIVE			0 /* LED on for bit == 1 */

#if	(CFG_NIOS_CPU_PIO3_TYPE == 1)
#define	SEVENSEG_WRONLY			1
#else
#undef	SEVENSEG_WRONLY
#endif

#elif	(CFG_NIOS_CPU_SEVENSEG_PIO == 4)

#error *** CFG_ERROR: seven segment display at PIO4 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_SEVENSEG_PIO == 5)

#error *** CFG_ERROR: seven segment display at PIO5 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_SEVENSEG_PIO == 6)

#error *** CFG_ERROR: seven segment display at PIO6 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_SEVENSEG_PIO == 7)

#error *** CFG_ERROR: seven segment display at PIO7 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_SEVENSEG_PIO == 8)

#error *** CFG_ERROR: seven segment display at PIO8 not supported, expand your config.h

#elif	(CFG_NIOS_CPU_SEVENSEG_PIO == 9)

#error *** CFG_ERROR: seven segment display at PIO9 not supported, expand your config.h

#else
#error *** CFG_ERROR: you have to set CFG_NIOS_CPU_SEVENSEG_PIO in right case
#endif

#define	CONFIG_SEVENSEG			1 /* enable seven segment led driver */

/*
 * Dual 7-Segment Display pin assignment -- read more in your
 * "Nios Development Board Reference Manual"
 *
 *
 *    (U8) HI:D[15..8]     (U9) LO:D[7..0]
 *         ______               ______
 *        |  D14 |             |  D6  |
 *        |      |             |      |
 *      D9|      |D13        D1|      |D5
 *        |______|             |______|                  ___
 *        |  D8  |             |  D0  |                 | A |
 *        |      |             |      |                F|___|B
 *     D10|      |D12        D2|      |D4               | G |
 *        |______|             |______|                E|___|C
 *           D11  *               D3   *                  D  *
 *                D15                  D7                    DP
 *
 */
#define	SEVENSEG_DIGIT_HI_LO_EQUAL	1	/* high nibble equal low nibble */
#define	SEVENSEG_DIGIT_A		(1 << 6) /* bit 6 is segment A */
#define	SEVENSEG_DIGIT_B		(1 << 5) /* bit 5 is segment B */
#define	SEVENSEG_DIGIT_C		(1 << 4) /* bit 4 is segment C */
#define	SEVENSEG_DIGIT_D		(1 << 3) /* bit 3 is segment D */
#define	SEVENSEG_DIGIT_E		(1 << 2) /* bit 2 is segment E */
#define	SEVENSEG_DIGIT_F		(1 << 1) /* bit 1 is segment F */
#define	SEVENSEG_DIGIT_G		(1 << 0) /* bit 0 is segment G */
#define	SEVENSEG_DIGIT_DP		(1 << 7) /* bit 7 is decimal point */

#endif	/* CFG_NIOS_CPU_PIO_NUMS */

/*------------------------------------------------------------------------
 * ASMI - Active Serial Memory Interface.
 *
 * ASMI is for Cyclone devices only and only works when the configuration
 * is loaded via JTAG or ASMI. Please see doc/README.dk1c20 for details.
 *----------------------------------------------------------------------*/
#define CONFIG_NIOS_ASMI			   /* Enable ASMI	*/
#define CFG_NIOS_ASMIBASE	CFG_NIOS_CPU_ASMI0 /* ASMI base address	*/

/*------------------------------------------------------------------------
 * COMMANDS
 *----------------------------------------------------------------------*/
#define CONFIG_COMMANDS		(CFG_CMD_ALL & ~( \
				 CFG_CMD_ASKENV | \
				 CFG_CMD_BEDBUG | \
				 CFG_CMD_BMP	| \
				 CFG_CMD_BSP	| \
				 CFG_CMD_CACHE	| \
				 CFG_CMD_DATE	| \
				 CFG_CMD_DOC	| \
				 CFG_CMD_DTT	| \
				 CFG_CMD_EEPROM | \
				 CFG_CMD_ELF    | \
				 CFG_CMD_FDC	| \
				 CFG_CMD_FDOS	| \
				 CFG_CMD_HWFLOW	| \
				 CFG_CMD_I2C	| \
				 CFG_CMD_JFFS2	| \
				 CFG_CMD_KGDB	| \
				 CFG_CMD_NAND	| \
				 CFG_CMD_NFS	| \
				 CFG_CMD_MMC	| \
				 CFG_CMD_MII	| \
				 CFG_CMD_PCI	| \
				 CFG_CMD_PCMCIA | \
				 CFG_CMD_REISER	| \
				 CFG_CMD_SCSI	| \
				 CFG_CMD_SPI	| \
				 CFG_CMD_VFD	| \
				 CFG_CMD_USB	| \
				 CFG_CMD_XIMG	) )


#include <cmd_confdefs.h>

/*------------------------------------------------------------------------
 * COMPACT FLASH
 *----------------------------------------------------------------------*/
#if (CONFIG_COMMANDS & CFG_CMD_IDE)
#define CONFIG_IDE_PREINIT			/* Implement id_preinit	*/
#define CFG_IDE_MAXBUS		1		/* 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	1		/* 1 drive per IDE bus	*/

#define CFG_ATA_BASE_ADDR	0x00920a00	/* IDE/ATA base addr	*/
#define CFG_ATA_IDE0_OFFSET	0x0000		/* IDE0 offset		*/
#define CFG_ATA_DATA_OFFSET	0x0040		/* Data IO offset	*/
#define CFG_ATA_REG_OFFSET	0x0040		/* Register offset	*/
#define CFG_ATA_ALT_OFFSET	0x0100		/* Alternate reg offset	*/
#define CFG_ATA_STRIDE          4		/* Width betwix addrs	*/
#define CONFIG_DOS_PARTITION

/* Board-specific cf regs */
#define CFG_CF_PRESENT		0x009209b0	/* CF Present PIO base	*/
#define CFG_CF_POWER		0x009209c0	/* CF Power FET PIO base*/
#define CFG_CF_ATASEL		0x009209d0	/* CF ATASEL PIO base	*/

#endif /* CONFIG_COMMANDS & CFG_CMD_IDE */

/*------------------------------------------------------------------------
 * KGDB
 *----------------------------------------------------------------------*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	9600
#endif

/*------------------------------------------------------------------------
 * MISC
 *----------------------------------------------------------------------*/
#define	CFG_LONGHELP			    /* undef to save memory	*/
#define	CFG_PROMPT		"DK1C20 > " /* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256	    /* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		16	    /* max number of command args*/
#define CFG_BARGSIZE		CFG_CBSIZE  /* Boot Argument Buffer Size */

#if	(CFG_SRAM_SIZE != 0)
#define	CFG_LOAD_ADDR		CFG_SRAM_BASE	/* Default load address	*/
#else
#undef	CFG_LOAD_ADDR
#endif

#if	(CFG_SDRAM_SIZE != 0)
#define	CFG_MEMTEST_START	CFG_SDRAM_BASE	/* SDRAM til stack area */
#define	CFG_MEMTEST_END		(CFG_INIT_SP - (1024 * 1024)) /* 1MB stack */
#else
#undef	CFG_MEMTEST_START
#undef	CFG_MEMTEST_END
#endif

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition, whole device */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		""
#define MTDPARTS_DEFAULT	""
*/

#endif	/* __CONFIG_H */
