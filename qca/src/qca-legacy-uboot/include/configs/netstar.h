/*
 * (C) Copyright 2005 2N TELEKOMUNIKACE, Ladislav Michl
 *
 * Configuation settings for the TI OMAP NetStar board.
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

#include <configs/omap1510.h>

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM925T	1		/* This is an arm925t CPU */
#define CONFIG_OMAP	1		/* in a TI OMAP core */
#define CONFIG_OMAP1510 1		/* which is in a 5910 */

/* Input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	150000000	/* 150MHz input clock */
#define CONFIG_XTAL_FREQ	12000000

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */

#define CONFIG_MISC_INIT_R		/* There is nothing to really init */
#define BOARD_LATE_INIT			/* but we flash the LEDs here */

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

#define CFG_DEVICE_NULLDEV		1	/* enable null device */
#define CONFIG_SILENT_CONSOLE		1	/* enable silent startup */

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x10000000	/* SDRAM Bank #1 */
#define PHYS_FLASH_1		0x00000000	/* Flash Bank #1 */

/*
 * FLASH organization
 */
#define CFG_FLASH_BASE		PHYS_FLASH_1
#define CFG_MAX_FLASH_BANKS	1
#if (PHYS_SDRAM_1_SIZE == SZ_32M)
/*#if 1*/
#define CFG_FLASH_CFI			/* Flash is CFI conformant */
#define CFG_FLASH_CFI_DRIVER		/* Use the common driver */
#define CFG_FLASH_EMPTY_INFO
#define CFG_MAX_FLASH_SECT	128
#else
#define PHYS_FLASH_1_SIZE	SZ_1M
#define CFG_MAX_FLASH_SECT	19
#define CFG_FLASH_ERASE_TOUT	(5*CFG_HZ) /* in ticks */
#define CFG_FLASH_WRITE_TOUT	(5*CFG_HZ)
#endif

#define CFG_MONITOR_BASE	PHYS_FLASH_1
#define CFG_MONITOR_LEN		SZ_256K

/*
 * Environment settings
 */
#define CFG_ENV_IS_IN_FLASH
#define ENV_IS_SOLITARY
#define CFG_ENV_ADDR		0x4000
#define CFG_ENV_SIZE		SZ_8K
#define CFG_ENV_SECT_SIZE	SZ_8K
#define CFG_ENV_ADDR_REDUND	0x6000
#define CFG_ENV_SIZE_REDUND	CFG_ENV_SIZE
#define CONFIG_ENV_OVERWRITE

/*
 * Size of malloc() pool
 */
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
/* XXX #define CFG_MALLOC_LEN		(SZ_64K - CFG_GBL_DATA_SIZE)*/
#define CFG_MALLOC_LEN		SZ_4M

/*
 * The stack size is set up in start.S using the settings below
 */
/* XXX #define CONFIG_STACKSIZE	SZ_8K	/XXX* regular stack */
#define CONFIG_STACKSIZE	SZ_1M	/* regular stack */

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_SMC91111
#define CONFIG_SMC91111_BASE	0x04000300

/*
 * NS16550 Configuration
 */
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	(-4)
#define CFG_NS16550_CLK		(CONFIG_XTAL_FREQ)	/* can be 12M/32Khz or 48Mhz  */
#define CFG_NS16550_COM1	OMAP1510_UART1_BASE	/* uart1 */

#define CONFIG_CONS_INDEX	1
#define CONFIG_BAUDRATE		115200
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*#define CONFIG_SKIP_RELOCATE_UBOOT*/
/*#define CONFIG_SKIP_LOWLEVEL_INIT */

/*
 * NAND flash
 */
#define CFG_MAX_NAND_DEVICE	1
#define NAND_MAX_CHIPS		1
#define CFG_NAND_BASE	0x04000000 + (2 << 23)

/*
 * JFFS2 partitions (mtdparts command line support)
 */
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		"nor0=omapflash.0,nand0=omapnand.0"
#define MTDPARTS_DEFAULT	"mtdparts=omapflash.0:8k@16k(env),8k(r_env),448k@576k(u-boot);omapnand.0:48M(rootfs0),48M(rootfs1),-(data)"

#if 0
#define CONFIG_COMMANDS		(CFG_CMD_BDI    | \
				 CFG_CMD_BOOTD  | \
				 CFG_CMD_DHCP   | \
				 CFG_CMD_ENV	| \
				 CFG_CMD_FLASH  | \
				 CFG_CMD_IMI    | \
				 CFG_CMD_LOADB  | \
				 CFG_CMD_NET    | \
				 CFG_CMD_MEMORY | \
				 CFG_CMD_PING   | \
				 CFG_CMD_RUN)

#else
#define CONFIG_COMMANDS		(CFG_CMD_BDI    | \
				 CFG_CMD_BOOTD  | \
				 CFG_CMD_DHCP   | \
				 CFG_CMD_ENV	| \
				 CFG_CMD_FLASH  | \
				 CFG_CMD_NAND	| \
				 CFG_CMD_IMI    | \
				 CFG_CMD_JFFS2	| \
				 CFG_CMD_LOADB  | \
				 CFG_CMD_NET    | \
				 CFG_CMD_MEMORY | \
				 CFG_CMD_PING   | \
				 CFG_CMD_RUN)

#define CONFIG_JFFS2_NAND	1	/* jffs2 on nand support */
#endif

#define CONFIG_BOOTP_MASK	CONFIG_BOOTP_DEFAULT
#define CONFIG_LOOPW

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_BOOTDELAY	3
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* allow to break in always */
#undef  CONFIG_BOOTARGS		/* the boot command will set bootargs*/
#define CFG_AUTOLOAD		"n"		/* No autoload */
#define CONFIG_BOOTCOMMAND	"run nboot"
#define CONFIG_PREBOOT		"run setup"
#define	CONFIG_EXTRA_ENV_SETTINGS				\
	"setup=setenv bootargs console=ttyS0,$baudrate "	\
		"$mtdparts\0"					\
	"ospart=0\0"						\
	"setpart="						\
	"if test -n $swapos; then "				\
		"if test $ospart -eq 0; then chpart nand0,1; else chpart nand0,0; fi; "\
		"setenv swapos; saveenv; "			\
	"else "							\
		"chpart nand0,$ospart; "			\
	"fi\0"							\
	"nfsargs=setenv bootargs $bootargs "			\
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname::off " \
		"nfsroot=$rootpath root=/dev/nfs\0"		\
	"flashargs=run setpart;setenv bootargs $bootargs "	\
		"root=/dev/mtdblock$partition ro "		\
		"rootfstype=jffs2\0"				\
	"initrdargs=setenv bootargs $bootargs "			\
		"ip=$ipaddr:$serverip:$gatewayip:$netmask:$hostname::off\0" \
	"iboot=bootp;run initrdargs;tftp;bootm\0"		\
	"fboot=run flashargs;fsload /boot/uImage;bootm\0"	\
	"nboot=bootp;run nfsargs;tftp;bootm\0"

#if 0	/* feel free to disable for development */
#define	CONFIG_AUTOBOOT_KEYED		/* Enable password protection	*/
#define CONFIG_AUTOBOOT_PROMPT	"\nNetStar PBX - boot in %d sec...\n"
#define CONFIG_AUTOBOOT_DELAY_STR	"R"	/* 1st "password"	*/
#define CONFIG_BOOT_RETRY_TIME	30
#endif

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory		*/
#define CFG_PROMPT		"# "		/* Monitor Command Prompt	*/
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "
#define CONFIG_AUTO_COMPLETE

#define CFG_MEMTEST_START	PHYS_SDRAM_1
#define CFG_MEMTEST_END		PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE

#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR		PHYS_SDRAM_1 + 0x400000	/* default load address */

/* The 1510 has 3 timers, they can be driven by the RefClk (12Mhz) or by DPLL1.
 * This time is further subdivided by a local divisor.
 */
#define CFG_TIMERBASE		OMAP1510_TIMER1_BASE
#define CFG_PVT			7		/* 2^(pvt+1), divide by 256 */
#define CFG_HZ			((CONFIG_SYS_CLK_FREQ)/(2 << CFG_PVT))

#define OMAP5910_DPLL_DIV	1
#define OMAP5910_DPLL_MUL	((CONFIG_SYS_CLK_FREQ * \
				 (1 << OMAP5910_DPLL_DIV)) / CONFIG_XTAL_FREQ)

#define OMAP5910_ARM_PER_DIV	2	/* CKL/4 */
#define OMAP5910_LCD_DIV	2	/* CKL/4 */
#define OMAP5910_ARM_DIV	0	/* CKL/1 */
#define OMAP5910_DSP_DIV	0	/* CKL/1 */
#define OMAP5910_TC_DIV		1	/* CKL/2 */
#define OMAP5910_DSP_MMU_DIV	1	/* CKL/2 */
#define OMAP5910_ARM_TIM_SEL	1	/* CKL used for MPU timers */

#define OMAP5910_ARM_EN_CLK	0x03d6	/* 0000 0011 1101 0110b  Clock Enable */
#define OMAP5910_ARM_CKCTL	((OMAP5910_ARM_PER_DIV)  |	\
				 (OMAP5910_LCD_DIV << 2) |	\
				 (OMAP5910_ARM_DIV << 4) |	\
				 (OMAP5910_DSP_DIV << 6) |	\
				 (OMAP5910_TC_DIV << 8) |	\
				 (OMAP5910_DSP_MMU_DIV << 10) |	\
				 (OMAP5910_ARM_TIM_SEL << 12))

#endif	/* __CONFIG_H */
