/*
 * Copyright (C) 2004 by FS Forth-Systeme GmbH.
 * All rights reserved.
 * Markus Pietrek <mpietrek@fsforth.de>
 *
 * Configuation settings for the NetSilicon NS9750 DevBoard
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

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM926EJS	1	/* This is an ARM926EJS Core	*/
#define	CONFIG_NS9750		1	/* in an NetSilicon NS9750 SoC     */
#define CONFIG_NS9750DEV	1	/* on an NetSilicon NS9750 DevBoard  */

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	324403200 /* Don't use PLL. SW11-4 off */

#define CPU_CLK_FREQ		(CONFIG_SYS_CLK_FREQ/2)
#define AHB_CLK_FREQ		(CONFIG_SYS_CLK_FREQ/4)
#define BBUS_CLK_FREQ		(CONFIG_SYS_CLK_FREQ/8)

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */
/*@TODO #define CONFIG_STATUS_LED*/
#define CONFIG_USE_IRQ

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE       128     /* size in bytes reserved for initial
					 * data */

/*
 * Hardware drivers
 */
#define CFG_NS9750_UART			1	/* use on-chip UART */
#define CONFIG_DRIVER_NS9750_ETHERNET	1	/* use on-chip ethernet */

/*
 * select serial console configuration
 */
#define CONFIG_CONS_INDEX          1 		/* Port B */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		38400

/***********************************************************
 * Command definition
 ***********************************************************/
#if 0 /* @TODO */
#define CONFIG_COMMANDS \
			(CONFIG_CMD_DFL	 | \
			CFG_CMD_CACHE	 | \
			/*CFG_CMD_NAND	 |*/ \
			/*CFG_CMD_EEPROM |*/ \
			/*CFG_CMD_I2C	 |*/ \
			/*CFG_CMD_USB	 |*/ \
			CFG_CMD_REGINFO  | \
			CFG_CMD_DATE	 | \
			CFG_CMD_ELF)
#else
#define CONFIG_COMMANDS \
			(CONFIG_CMD_BDI | \
			CFG_CMD_NET | \
			CFG_CMD_PING	 | \
			CFG_CMD_CONSOLE	 | \
			CFG_CMD_LOADB	 | \
			CFG_CMD_LOADS	 | \
			CFG_CMD_MEMORY)
#endif

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_BOOTDELAY	3
/*#define CONFIG_BOOTARGS    	"root=ramfs devfs=mount console=ttySA0,9600" */

#define CONFIG_ETHADDR		00:04:f3:ff:ff:fb /*@TODO unset */
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR		192.168.42.30
#define CONFIG_SERVERIP		192.168.42.1

/*#define CONFIG_BOOTFILE	"elinos-lart" */
/*#define CONFIG_BOOTCOMMAND	"tftp; bootm" */

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	1		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT		"NS9750DEV # "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00000000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x00780000	/* 7,5 MB in DRAM	*/ /* @TODO */

#undef  CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define	CFG_LOAD_ADDR		0x00600000	/* default load address	*/ /* @TODO */

#define	CFG_HZ			(CPU_CLK_FREQ/64)

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define NS9750_ETH_PHY_ADDRESS	(0x0000)

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
/* TODO */
#define CONFIG_NR_DRAM_BANKS	2	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x00000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x00800000 /* 8 MB */
#define PHYS_SDRAM_2		0x10000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_2_SIZE	0x00800000 /* 8 MB */

#define PHYS_FLASH_1		0x50000000 /* Flash Bank #1 */

#define CFG_FLASH_BASE		PHYS_FLASH_1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/* @TODO*/
#define CONFIG_AMD_LV400	1	/* uncomment this if you have a LV400 flash */
#if 0
#define CONFIG_AMD_LV800	1	/* uncomment this if you have a LV800 flash */
#endif

#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#ifdef CONFIG_AMD_LV800
#define PHYS_FLASH_SIZE		0x00100000 /* 1MB */
#define CFG_MAX_FLASH_SECT	(19)	/* max number of sectors on one chip */
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x0F0000) /* addr of environment */
#endif
#ifdef CONFIG_AMD_LV400
#define PHYS_FLASH_SIZE		0x00080000 /* 512KB */
#define CFG_MAX_FLASH_SECT	(11)	/* max number of sectors on one chip */
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x070000) /* addr of environment */
#endif

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(5*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(5*CFG_HZ) /* Timeout for Flash Write */

/* @TODO */
/*#define	CFG_ENV_IS_IN_FLASH	1*/
#define CFG_ENV_IS_NOWHERE
#define CFG_ENV_SIZE		0x10000	/* Total Size of Environment Sector */

#ifdef CONFIG_STATUS_LED

extern void __led_init(led_id_t mask, int state);
extern void __led_toggle(led_id_t mask);
extern void __led_set(led_id_t mask, int state);

#endif /* CONFIG_STATUS_LED */

#endif	/* __CONFIG_H */
