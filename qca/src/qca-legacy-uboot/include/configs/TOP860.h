/*
 * (C) Copyright 2003
 * EMK Elektronik GmbH <www.emk-elektronik.de>
 * Reinhard Meyer <r.meyer@emk-elektronik.de>
 *
 * Configuation settings for the TOP860 board.
 *
 * -----------------------------------------------------------------
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
 * TOP860 is a simple module:
 * 16-bit wide FLASH on CS0	(2MB or more)
 * 32-bit wide DRAM on CS2 (either 4MB or 16MB)
 * FEC with Am79C874 100-Base-T and Fiber Optic
 * Ports available, but we choose SMC1 for Console
 * 8k I2C EEPROM at address 0xae, 6k user available, 2k factory set
 * 32768Hz crystal PLL set for 49.152MHz Core and 24.576MHz Bus Clock
 *
 * This config has been copied from MBX.h / MBX860T.h
 */
/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

/*-----------------------------------------------------------------------
 * CPU and BOARD type
 */
#define CONFIG_MPC860	1	/* This is a MPC860 CPU		*/
#define CONFIG_MPC860T	1	/* even better... an FEC!	*/
#define CONFIG_TOP860	1	/* ...on a TOP860 module	*/
#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/
#define	CONFIG_IDENT_STRING " EMK TOP860"

/*-----------------------------------------------------------------------
 * CLOCK settings
 */
#define	CONFIG_SYSCLK	49152000
#define	CFG_XTAL		32768
#define	CONFIG_EBDF		1
#define	CONFIG_COM		3
#define	CONFIG_RTC_MPC8xx

/*-----------------------------------------------------------------------
 * Physical memory map as defined by EMK
 */
#define CFG_IMMR		0xFFF00000	/* Internal Memory Mapped Register */
#define	CFG_FLASH_BASE	0x80000000	/* FLASH in final mapping */
#define	CFG_DRAM_BASE	0x00000000	/* DRAM in final mapping */
#define	CFG_FLASH_MAX	0x00400000	/* max FLASH to expect */
#define	CFG_DRAM_MAX	0x01000000	/* max DRAM to expect */

/*-----------------------------------------------------------------------
 * derived values
 */
#define	CFG_MF			(CONFIG_SYSCLK/CFG_XTAL)
#define	CFG_CPUCLOCK	CONFIG_SYSCLK
#define	CFG_BRGCLOCK	CONFIG_SYSCLK
#define	CFG_BUSCLOCK	(CONFIG_SYSCLK >> CONFIG_EBDF)
#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */
#define	CONFIG_8xx_GCLK_FREQ	CONFIG_SYSCLK

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	128	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define	CFG_FLASH_CFI

/*-----------------------------------------------------------------------
 * Command interpreter
 */
#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#define CONFIG_BAUDRATE		9600

/*
 * Allow partial commands to be matched to uniqueness.
 */
#define CFG_MATCH_PARTIAL_CMD

/*
 * List of available monitor commands.  Use the system default list
 * plus add some of the "non-standard" commands back in.
 * See ./cmd_confdefs.h
 */
#define CONFIG_COMMANDS	      ( CONFIG_CMD_DFL	| \
								CFG_CMD_ASKENV	| \
								CFG_CMD_DHCP	| \
								CFG_CMD_I2C		| \
								CFG_CMD_EEPROM	| \
								CFG_CMD_REGINFO	| \
								CFG_CMD_IMMAP	| \
								CFG_CMD_ELF		| \
								CFG_CMD_DATE	| \
								CFG_CMD_MII 	| \
								CFG_CMD_BEDBUG	\
						      )

#define	CONFIG_AUTOSCRIPT		1
#define	CFG_LOADS_BAUD_CHANGE	1
#undef	CONFIG_LOADS_ECHO			/* NO echo on for serial download	*/

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/

#undef	CFG_HUSH_PARSER			/* Hush parse for U-Boot	*/

#ifdef	CFG_HUSH_PARSER
 #define CFG_PROMPT_HUSH_PS2	"> "
#endif

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
 #define CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
 #define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif

#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

/*-----------------------------------------------------------------------
 * Memory Test Command
 */
#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

/*-----------------------------------------------------------------------
 * Environment handler
 * only the first 6k in EEPROM are available for user. Of that we use 256b
 */
#define	CONFIG_SOFT_I2C
#define CFG_ENV_IS_IN_EEPROM	1	/* turn on EEPROM env feature */
#define CFG_ENV_OFFSET		0x1000
#define CFG_ENV_SIZE		0x0700
#define CFG_I2C_EEPROM_ADDR 0x57
#define CFG_FACT_OFFSET		0x1800
#define CFG_FACT_SIZE		0x0800
#define CFG_I2C_FACT_ADDR	0x57
#define CFG_EEPROM_PAGE_WRITE_BITS 3
#define CFG_I2C_EEPROM_ADDR_LEN 2
#define CFG_EEPROM_SIZE 0x2000
#define	CFG_I2C_SPEED	100000
#define	CFG_I2C_SLAVE	0xFE
#define	CFG_EEPROM_PAGE_WRITE_DELAY_MS 12
#define CONFIG_ENV_OVERWRITE
#define CONFIG_MISC_INIT_R

#if defined (CONFIG_SOFT_I2C)
#define	SDA	0x00010
#define	SCL	0x00020
#define __I2C_DIR	immr->im_cpm.cp_pbdir
#define __I2C_DAT	immr->im_cpm.cp_pbdat
#define __I2C_PAR	immr->im_cpm.cp_pbpar
#define	__I2C_ODR	immr->im_cpm.cp_pbodr
#define	I2C_INIT	{ __I2C_PAR &= ~(SDA|SCL);	\
			  __I2C_ODR &= ~(SDA|SCL);	\
			  __I2C_DAT |= (SDA|SCL);	\
			  __I2C_DIR|=(SDA|SCL);	}
#define	I2C_READ	((__I2C_DAT & SDA) ? 1 : 0)
#define	I2C_SDA(x)	{ if (x) __I2C_DAT |= SDA; else __I2C_DAT &= ~SDA; }
#define	I2C_SCL(x)	{ if (x) __I2C_DAT |= SCL; else __I2C_DAT &= ~SCL; }
#define	I2C_DELAY	{ udelay(5); }
#define	I2C_ACTIVE	{ __I2C_DIR |= SDA; }
#define	I2C_TRISTATE	{ __I2C_DIR &= ~SDA; }
#endif

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*-----------------------------------------------------------------------
 * defines we need to get FEC running
 */
#define	CONFIG_NET_MULTI	1	/* the only way to get the FEC in */
#define CONFIG_FEC_ENET 	1	/* Ethernet only via FEC	*/
#define	FEC_ENET			1	/* eth.c needs it that way... */
#define CFG_DISCOVER_PHY	1
#define CONFIG_MII			1
#define CONFIG_PHY_ADDR		31

/*-----------------------------------------------------------------------
 * adresses
 */
#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0x80000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x2f00	/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	64  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_VPD_SIZE	256 /* size in bytes reserved for vpd buffer */
#define CFG_INIT_VPD_OFFSET	(CFG_GBL_DATA_OFFSET - CFG_INIT_VPD_SIZE)
#define CFG_INIT_SP_OFFSET	(CFG_INIT_VPD_OFFSET-8)

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/* Interrupt level assignments.
*/
#define FEC_INTERRUPT	SIU_LEVEL1	/* FEC interrupt */

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

/*-----------------------------------------------------------------------
 * Debug Enable Register
 *-----------------------------------------------------------------------
 *
 */
#define CFG_DER 0					/* used in start.S */

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * set up PLPRCR (PLL, Low-Power, and Reset Control Register)
 *	12	MF		calculated	Multiplication factor
 *	4	0		0000
 *	1	SPLSS	0			System PLL lock status sticky
 *	1	TEXPS	1			Timer expired status
 *	1	0		0
 *	1	TMIST	0			Timers interrupt status
 *	1	0		0
 *	1	CSRC	0			Clock source (0=DFNH, 1=DFNL)
 *	2	LPM		00			Low-power modes
 *	1	CSR		0			Checkstop reset enable
 *	1	LOLRE	0			Loss-of-lock reset enable
 *	1	FIOPD	0			Force I/O pull down
 *	5	0		00000
 */
#define CFG_PLPRCR	(PLPRCR_TEXPS | ((CFG_MF-1)<<20))

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * set up SYPCR:
 *	16	SWTC	0xffff		Software watchdog timer count
 *	8	BMT		0xff 		Bus monitor timing
 *	1	BME		1			Bus monitor enable
 *	3	0		000
 *	1	SWF		1			Software watchdog freeze
 *	1	SWE		0/1			Software watchdog enable
 *	1	SWRI	0/1			Software watchdog reset/interrupt select (1=HRESET)
 *	1	SWP		0/1			Software watchdog prescale (1=/2048)
 */
#if defined (CONFIG_WATCHDOG)
 #define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 		 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
 #define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * set up SIUMCR
 *	1	EARB	0			External arbitration
 *	3	EARP	000			External arbitration request priority
 *	4	0		0000
 *	1	DSHW	0			Data show cycles
 *	2	DBGC	00			Debug pin configuration
 *	2	DBPC	00			Debug port pins configuration
 *	1	0		0
 *	1	FRC		0			FRZ pin configuration
 *	1	DLK		0			Debug register lock
 *	1	OPAR	0			Odd parity
 *	1	PNCS	0			Parity enable for non memory controller regions
 *	1	DPC		0			Data parity pins configuration
 *	1	MPRE	0			Multiprocessor reservation enable
 *	2	MLRC	11			Multi level reservation control (00=IRQ4, 01=3State, 10=KR/RETRY, 11=SPKROUT)
 *	1	AEME	0			Async external master enable
 *	1	SEME	0			Sync external master enable
 *	1	BSC		0			Byte strobe configuration
 *	1	GB5E	0			GPL_B5 enable
 *	1	B2DD	0			Bank 2 double drive
 *	1	B3DD	0			Bank 3 double drive
 *	4	0		0000
 */
#define CFG_SIUMCR	(SIUMCR_MLRC11)

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
#define CFG_PISCR	(PISCR_PS | PISCR_PITF | PISCR_PTE)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * set up SCCR (System Clock and Reset Control Register)
 *	1	0		0
 *	2	COM		11			Clock output module (00=full, 01=half, 11=off)
 *	3	0		000
 *	1	TBS		1			Timebase source (0=OSCCLK, 1=GCLK2)
 *	1	RTDIV	0			Real-time clock divide (0=/4, 1=/512)
 *	1	RTSEL	0			Real-time clock select (0=OSCM, 1=EXTCLK)
 *	1	CRQEN	0			CPM request enable
 *	1	PRQEN	0			Power management request enable
 *	2	0		00
 *	2	EBDF	xx			External bus division factor
 *	2	0		00
 *	2	DFSYNC	00			Division factor for SYNCLK
 *	2	DFBRG	00			Division factor for BRGCLK
 *	3	DFNL	000			Division factor low frequency
 *	3	DFNH	000			Division factor high frequency
 *	5	0		00000
 */
#define SCCR_MASK	0
#ifdef CONFIG_EBDF
 #define CFG_SCCR	(SCCR_COM11 | SCCR_TBS | SCCR_EBDF01)
#else
 #define CFG_SCCR	(SCCR_COM11 | SCCR_TBS)
#endif

/*-----------------------------------------------------------------------
 * Chip Select 0 - FLASH
 *-----------------------------------------------------------------------
 * Preliminary Values
 */
/* FLASH timing: CSNT=1 ACS=10 BIH=1 SCY=4 SETA=0 TLRX=1 EHTR=1	*/
#define CFG_OR_TIMING_FLASH	(OR_CSNT_SAM  | OR_ACS_DIV4 | OR_BI | OR_SCY_4_CLK | OR_TRLX | OR_EHTR)
#define CFG_OR0_PRELIM	(-CFG_FLASH_MAX | CFG_OR_TIMING_FLASH)
#define CFG_BR0_PRELIM	(CFG_FLASH_BASE | BR_PS_16 | BR_V )

/*-----------------------------------------------------------------------
 * misc
 *-----------------------------------------------------------------------
 *
 */
/*
 * Set the autoboot delay in seconds.  A delay of -1 disables autoboot
 */
#define CONFIG_BOOTDELAY				5

/*
 * Pass the clock frequency to the Linux kernel in units of MHz
 */
#define	CONFIG_CLOCKS_IN_MHZ

#define CONFIG_PREBOOT		\
	"echo;echo"

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND	\
	"bootp;" \
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} " \
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; " \
	"bootm"

/*
 * BOOTP options
 */
#undef	CONFIG_BOOTP_MASK
#define CONFIG_BOOTP_MASK				( CONFIG_BOOTP_DEFAULT		| \
									  	  CONFIG_BOOTP_BOOTFILESIZE   \
										)


/*
 * Set default IP stuff just to get bootstrap entries into the
 * environment so that we can autoscript the full default environment.
 */
#define CONFIG_ETHADDR					9a:52:63:15:85:25
#define CONFIG_SERVERIP					10.0.4.200
#define CONFIG_IPADDR					10.0.4.111

/*-----------------------------------------------------------------------
 * Defaults for Autoscript
 */
#define CFG_LOAD_ADDR		0x00100000	/* default load address */
#define	CFG_TFTP_LOADADDR	0x00100000

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */


#endif	/* __CONFIG_H */
