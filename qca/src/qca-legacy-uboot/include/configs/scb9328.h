/*
 * Copyright (C) 2003 ETC s.r.o.
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
 *
 * Written by Peter Figuli <peposh@etc.sk>, 2003.
 *
 * 2003/13/06 Initial MP10 Support copied from wepep250
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_ARM920T		1     /* this is an ARM920T CPU	    */
#define CONFIG_IMX		1     /* in a Motorola MC9328MXL Chip */
#define CONFIG_SCB9328		1     /* on a scb9328tronix board */
#undef	CONFIG_USE_IRQ		      /* don't need use IRQ/FIQ	   */

#define CONFIG_IMX_SERIAL1
/*
 * Select serial console configuration
 */


/*
 * Definition of u-boot build in commands. Check out CONFIG_CMD_DFL if
 * neccessary in include/cmd_confdefs.h file. (Un)comment for getting
 * functionality or size of u-boot code.
 */
#define CONFIG_COMMANDS		(CONFIG_CMD_DFL		\
				& ~CFG_CMD_LOADS	\
				& ~CFG_CMD_CONSOLE	\
				& ~CFG_CMD_AUTOSCRIPT	\
				| CFG_CMD_NET		\
				| CFG_CMD_PING		\
				| CFG_CMD_DHCP		\
				)

#include <cmd_confdefs.h>

/*
 * Boot options. Setting delay to -1 stops autostart count down.
 * NOTE: Sending parameters to kernel depends on kernel version and
 * 2.4.19-rmk6-pxa1 patch used while my u-boot coding didn't accept
 * parameters at all! Do not get confused by them so.
 */
#define CONFIG_BOOTDELAY   -1
#define CONFIG_BOOTARGS	   "console=ttySMX0,115200n8 root=/dev/mtdblock3 rootfstype=jffs2 mtdparts=scb9328_flash:128k(U-boot)ro,128k(U-boot_env),1m(kernel),4m(root),4m(fs) eval_board=evk9328"
#define CONFIG_BOOTCOMMAND "bootm 10040000"
#define CONFIG_SHOW_BOOT_PROGRESS
#define CONFIG_ETHADDR		80:81:82:83:84:85
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		10.10.10.9
#define CONFIG_SERVERIP		10.10.10.10

/*
 * General options for u-boot. Modify to save memory foot print
 */
#define CFG_LONGHELP				      /* undef saves memory  */
#define CFG_PROMPT		"scb9328> "	      /* prompt string	     */
#define CFG_CBSIZE		256		      /* console I/O buffer  */
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* print buffer size   */
#define CFG_MAXARGS		16		      /* max command args    */
#define CFG_BARGSIZE		CFG_CBSIZE	      /* boot args buf size  */

#define CFG_MEMTEST_START	0x08100000	      /* memtest test area   */
#define CFG_MEMTEST_END		0x08F00000

#undef	CFG_CLKS_IN_HZ			     /* use HZ for freq. display     */

#define CFG_HZ			3686400	     /* incrementer freq: 3.6864 MHz */
#define CFG_CPUSPEED		0x141	     /* core clock - register value  */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_BAUDRATE 115200
/*
 * Definitions related to passing arguments to kernel.
 */
#define CONFIG_CMDLINE_TAG	     1	 /* send commandline to Kernel	     */
#define CONFIG_SETUP_MEMORY_TAGS     1	 /* send memory definition to kernel */
#define CONFIG_INITRD_TAG	     1	 /* send initrd params		     */
#undef	CONFIG_VFD			 /* do not send framebuffer setup    */


/*
 * Malloc pool need to host env + 128 Kb reserve for other allocations.
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + (128<<10) )


#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

#define CONFIG_STACKSIZE	(120<<10)      /* stack size		     */

#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4<<10)	       /* IRQ stack		     */
#define CONFIG_STACKSIZE_FIQ	(4<<10)	       /* FIQ stack		     */
#endif

/* SDRAM Setup Values
0x910a8300 Precharge Command CAS 3
0x910a8200 Precharge Command CAS 2

0xa10a8300 AutoRefresh Command CAS 3
0xa10a8200 Set AutoRefresh Command CAS 2 */

#define PRECHARGE_CMD 0x910a8200
#define AUTOREFRESH_CMD 0xa10a8200

/*
 * SDRAM Memory Map
 */
/* SH FIXME */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of SDRAM */
#define SCB9328_SDRAM_1		0x08000000	/* SDRAM bank #1	   */
#define SCB9328_SDRAM_1_SIZE	0x01000000	/* 16 MB		   */

/*
 * Flash Controller settings
 */

/*
 * Hardware drivers
 */


/*
 * Configuration for FLASH memory for the Synertronixx board
 */

/* #define SCB9328_FLASH_32M */

/* 32MB */
#ifdef SCB9328_FLASH_32M
#define CFG_MAX_FLASH_BANKS		1	/* FLASH banks count (not chip count)*/
#define CFG_MAX_FLASH_SECT		256	/* number of sector in FLASH bank    */
#define SCB9328_FLASH_BUS_WIDTH		2	/* we use 16 bit FLASH memory...     */
#define SCB9328_FLASH_INTERLEAVE	1	/* ... made of 1 chip		     */
#define SCB9328_FLASH_BANK_SIZE	 0x02000000	/* size of one flash bank	     */
#define SCB9328_FLASH_SECT_SIZE	 0x00020000	/* size of erase sector		     */
#define SCB9328_FLASH_BASE	 0x10000000	/* location of flash memory	     */
#define SCB9328_FLASH_UNLOCK		1	/* perform hw unlock first	     */
#else

/* 16MB */
#define CFG_MAX_FLASH_BANKS		1	/* FLASH banks count (not chip count)*/
#define CFG_MAX_FLASH_SECT		128	/* number of sector in FLASH bank    */
#define SCB9328_FLASH_BUS_WIDTH		2	/* we use 16 bit FLASH memory...     */
#define SCB9328_FLASH_INTERLEAVE	1	/* ... made of 1 chip		     */
#define SCB9328_FLASH_BANK_SIZE	 0x01000000	/* size of one flash bank	     */
#define SCB9328_FLASH_SECT_SIZE	 0x00020000	/* size of erase sector		     */
#define SCB9328_FLASH_BASE	 0x10000000	/* location of flash memory	     */
#define SCB9328_FLASH_UNLOCK		1	/* perform hw unlock first	     */
#endif /* SCB9328_FLASH_32M */

/* This should be defined if CFI FLASH device is present. Actually benefit
   is not so clear to me. In other words we can provide more informations
   to user, but this expects more complex flash handling we do not provide
   now.*/
#undef	CFG_FLASH_CFI

#define CFG_FLASH_ERASE_TOUT	(2*CFG_HZ)    /* timeout for Erase operation */
#define CFG_FLASH_WRITE_TOUT	(2*CFG_HZ)    /* timeout for Write operation */

#define CFG_FLASH_BASE		SCB9328_FLASH_BASE

/*
 * This is setting for JFFS2 support in u-boot.
 * Right now there is no gain for user, but later on booting kernel might be
 * possible. Consider using XIP kernel running from flash to save RAM
 * footprint.
 * NOTE: Enable CFG_CMD_JFFS2 for JFFS2 support.
 */
#define CFG_JFFS2_FIRST_BANK		0
#define CFG_JFFS2_FIRST_SECTOR		5
#define CFG_JFFS2_NUM_BANKS		1

/*
 * Environment setup. Definitions of monitor location and size with
 * definition of environment setup ends up in 2 possibilities.
 * 1. Embeded environment - in u-boot code is space for environment
 * 2. Environment is read from predefined sector of flash
 * Right now we support 2. possiblity, but expecting no env placed
 * on mentioned address right now. This also needs to provide whole
 * sector for it - for us 256Kb is really waste of memory. U-boot uses
 * default env. and until kernel parameters could be sent to kernel
 * env. has no sense to us.
 */

/* Setup for PA23 which is Reset Default PA23 but has to become
   CS5 */

#define CFG_GPR_A_VAL		0x00800000
#define CFG_GIUS_A_VAL		0x0043fffe

#define CFG_MONITOR_BASE	0x10000000
#define CFG_MONITOR_LEN		0x20000		/* 128b ( 1 flash sector )  */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		0x10020000	/* absolute address for now  */
#define CFG_ENV_SIZE		0x20000

#define	 CONFIG_ENV_OVERWRITE  1		/* env is not writable now   */

/*
 * CSxU_VAL:
 * 63| x	x x x | x x x x | x x  x    x | x x x x | x x x x | x x x x | x x x x | x x x x|32
 *   |DTACK_SEL|0|BCD |	  BCS	| PSZ|PME|SYNC|	 DOL	| CNC|	  WSC	    | 0| WWS  |	  EDC  |
 *
 * CSxL_VAL:
 * 31| x x x x | x x x x | x x x x | x x x x | x x x x |  x x x x | x x	 x x | x x  x	 x| 0
 *   |	 OEA   |   OEN	 |   WEA   |   WEN   |	 CSA   |EBC| DSZ  | 0|SP|0|WP| 0 0|PA|CSEN|
 */

#define CFG_CS0U_VAL 0x000F2000
#define CFG_CS0L_VAL 0x11110d01
#define CFG_CS1U_VAL 0x000F0a00
#define CFG_CS1L_VAL 0x11110601
#define CFG_CS2U_VAL 0x0
#define CFG_CS2L_VAL 0x0

#define CFG_CS3U_VAL 0x000FFFFF
#define CFG_CS3L_VAL 0x00000303

#define CFG_CS4U_VAL 0x000F0a00
#define CFG_CS4L_VAL 0x11110301

/* CNC == 3 too long
   #define CFG_CS5U_VAL 0x0000C210 */

/* #define CFG_CS5U_VAL 0x00008400
   mal laenger mahcen, ob der bei 150MHz laenger haelt dann und
   kaum langsamer ist */
/* #define CFG_CS5U_VAL 0x00009400
   #define CFG_CS5L_VAL 0x11010D03 */

#define CFG_CS5U_VAL 0x00008400
#define CFG_CS5L_VAL 0x00000D03

#define CONFIG_DRIVER_DM9000		1
#define CONFIG_DRIVER_DM9000		1
#define CONFIG_DM9000_BASE		0x16000000
#define DM9000_IO			CONFIG_DM9000_BASE
#define DM9000_DATA			(CONFIG_DM9000_BASE+4)
/* #define CONFIG_DM9000_USE_8BIT */
#define CONFIG_DM9000_USE_16BIT
/* #define CONFIG_DM9000_USE_32BIT */

/* f_{dpll}=2*f{ref}*(MFI+MFN/(MFD+1))/(PD+1)
   f_ref=16,777MHz

   0x002a141f: 191,9944MHz
   0x040b2007: 144MHz
   0x042a141f: 96MHz
   0x0811140d: 64MHz
   0x040e200e: 150MHz
   0x00321431: 200MHz

   0x08001800: 64MHz mit 16er Quarz
   0x04001800: 96MHz mit 16er Quarz
   0x04002400: 144MHz mit 16er Quarz

   31 |x x x x|x x x x|x x x x|x x x x|x x x x|x x x x|x x x x|x x x x| 0
      |XXX|--PD---|-------MFD---------|XXX|--MFI--|-----MFN-----------|	    */

#define CPU200

#ifdef CPU200
#define CFG_MPCTL0_VAL 0x00321431
#else
#define CFG_MPCTL0_VAL 0x040e200e
#endif

/* #define BUS64 */
#define BUS72

#ifdef BUS72
#define CFG_SPCTL0_VAL 0x04002400
#endif

#ifdef BUS96
#define CFG_SPCTL0_VAL 0x04001800
#endif

#ifdef BUS64
#define CFG_SPCTL0_VAL 0x08001800
#endif

/* Das ist der BCLK Divider, der aus der System PLL
   BCLK und HCLK erzeugt:
   31 | xxxx xxxx xxxx xxxx xx10 11xx xxxx xxxx | 0
   0x2f008403 : 192MHz/2=96MHz, 144MHz/2=72MHz PRESC=1->BCLKDIV=2
   0x2f008803 : 192MHz/3=64MHz, 240MHz/3=80MHz PRESC=1->BCLKDIV=2
   0x2f001003 : 192MHz/5=38,4MHz
   0x2f000003 : 64MHz/1
   Bit 22: SPLL Restart
   Bit 21: MPLL Restart */

#ifdef BUS64
#define CFG_CSCR_VAL 0x2f030003
#endif

#ifdef BUS72
#define CFG_CSCR_VAL 0x2f030403
#endif

/*
 * Well this has to be defined, but on the other hand it is used differently
 * one may expect. For instance loadb command do not cares :-)
 * So advice is - do not relay on this...
 */
#define CFG_LOAD_ADDR 0x08400000

#define MHZ16QUARZINUSE

#ifdef MHZ16QUARZINUSE
#define CONFIG_SYSPLL_CLK_FREQ 16000000
#else
#define CONFIG_SYSPLL_CLK_FREQ 16780000
#endif

#define CONFIG_SYS_CLK_FREQ 16780000

/* FMCR Bit 0 becomes 0 to make CS3 CS3 :P */
#define CFG_FMCR_VAL 0x00000001

/* Bit[0:3] contain PERCLK1DIV for UART 1
   0x000b00b ->b<- -> 192MHz/12=16MHz
   0x000b00b ->8<- -> 144MHz/09=16MHz
   0x000b00b ->3<- -> 64MHz/4=16MHz */

#ifdef BUS96
#define CFG_PCDR_VAL 0x000b00b5
#endif

#ifdef BUS64
#define CFG_PCDR_VAL 0x000b00b3
#endif

#ifdef BUS72
#define CFG_PCDR_VAL 0x000b00b8
#endif

#endif	/* __CONFIG_H */
