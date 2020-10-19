#ifndef __CONFIG_EZKIT533_H__
#define __CONFIG_EZKIT533_H__

#define CFG_LONGHELP		1
#define CONFIG_BAUDRATE		57600
#define CONFIG_STAMP		1
#define CONFIG_BOOTDELAY	5

#define CONFIG_DRIVER_SMC91111	1
#define CONFIG_SMC91111_BASE	0x20310300
#if 0
#define CONFIG_MII
#define CFG_DISCOVER_PHY
#endif

#define CONFIG_RTC_BF533	1
#define CONFIG_BOOT_RETRY_TIME	-1	/* Enable this if bootretry required, currently its disabled */

/* CONFIG_CLKIN_HZ is any value in Hz				 */
#define CONFIG_CLKIN_HZ		 27000000
/* CONFIG_CLKIN_HALF controls what is passed to PLL 0=CLKIN	 */
/*						    1=CLKIN/2	 */
#define CONFIG_CLKIN_HALF		0
/* CONFIG_PLL_BYPASS controls if the PLL is used 0=don't bypass	 */
/*						 1=bypass PLL	 */
#define CONFIG_PLL_BYPASS		0
/* CONFIG_VCO_MULT controls what the multiplier of the PLL is.	 */
/* Values can range from 1-64					 */
#define CONFIG_VCO_MULT			22
/* CONFIG_CCLK_DIV controls what the core clock divider is	 */
/* Values can be 1, 2, 4, or 8 ONLY				 */
#define CONFIG_CCLK_DIV			1
/* CONFIG_SCLK_DIV controls what the peripheral clock divider is */
/* Values can range from 1-15					 */
#define CONFIG_SCLK_DIV			5

#if ( CONFIG_CLKIN_HALF == 0 )
#define CONFIG_VCO_HZ		( CONFIG_CLKIN_HZ * CONFIG_VCO_MULT )
#else
#define CONFIG_VCO_HZ		(( CONFIG_CLKIN_HZ * CONFIG_VCO_MULT ) / 2 )
#endif

#if (CONFIG_PLL_BYPASS == 0)
#define CONFIG_CCLK_HZ		( CONFIG_VCO_HZ / CONFIG_CCLK_DIV )
#define CONFIG_SCLK_HZ		( CONFIG_VCO_HZ / CONFIG_SCLK_DIV )
#else
#define CONFIG_CCLK_HZ		CONFIG_CLKIN_HZ
#define CONFIG_SCLK_HZ		CONFIG_CLKIN_HZ
#endif

#define CONFIG_MEM_SIZE			32	       /* 128, 64, 32, 16 */
#define CONFIG_MEM_ADD_WDTH		 9	       /* 8, 9, 10, 11	  */
#define CONFIG_MEM_MT48LC16M16A2TG_75	 1

#define CONFIG_LOADS_ECHO	1


#define CONFIG_COMMANDS			(CONFIG_CMD_DFL | \
					 CFG_CMD_PING	| \
					 CFG_CMD_ELF	| \
					 CFG_CMD_I2C	| \
					 CFG_CMD_JFFS2	| \
					 CFG_CMD_DATE)
#define CONFIG_BOOTARGS "root=/dev/mtdblock0 ip=192.168.0.15:192.168.0.2:192.168.0.1:255.255.255.0:ezkit:eth0:off"

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CFG_PROMPT		"ezkit> "	/* Monitor Command Prompt */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size */
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size */
#endif
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS		16	/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size */
#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x01F00000	/* 1 ... 31 MB in DRAM */
#define CFG_LOAD_ADDR		0x01000000	/* default load address */
#define CFG_HZ			1000	/* decrementer freq: 10 ms ticks */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CFG_SDRAM_BASE		0x00000000
#define CFG_MAX_RAM_SIZE	0x02000000
#define CFG_FLASH_BASE		0x20000000

#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CFG_MONITOR_BASE	(CFG_MAX_RAM_SIZE - CFG_MONITOR_LEN)
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/
#define CFG_MALLOC_BASE		(CFG_MONITOR_BASE - CFG_MALLOC_LEN)
#define CFG_GBL_DATA_SIZE	0x4000
#define CFG_GBL_DATA_ADDR	(CFG_MALLOC_BASE - CFG_GBL_DATA_SIZE)
#define CONFIG_STACKBASE	(CFG_GBL_DATA_ADDR  - 4)

#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */
#define CFG_FLASH0_BASE		0x20000000
#define CFG_FLASH1_BASE		0x20200000
#define CFG_FLASH2_BASE		0x20280000
#define CFG_MAX_FLASH_BANKS	3	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	40	/* max number of sectors on one chip */

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		0x20020000
#define CFG_ENV_SECT_SIZE	0x10000 /* Total Size of Environment Sector */

/* JFFS Partition offset set  */
#define CFG_JFFS2_FIRST_BANK 0
#define CFG_JFFS2_NUM_BANKS  1
/* 512k reserved for u-boot */
#define CFG_JFFS2_FIRST_SECTOR		       11


/*
 * Stack sizes
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */

#define POLL_MODE		1
#define FLASH_TOT_SECT		40
#define FLASH_SIZE		0x220000
#define CFG_FLASH_SIZE		0x220000

/*
 * Initialize PSD4256 registers for using I2C
 */
#define CONFIG_MISC_INIT_R

/*
 * I2C settings
 * By default PF1 is used as SDA and PF0 as SCL on the Stamp board
 */
#define CONFIG_SOFT_I2C			1	/* I2C bit-banged		*/
/*
 * Software (bit-bang) I2C driver configuration
 */
#define PF_SCL				PF0
#define PF_SDA				PF1

#define I2C_INIT			(*pFIO_DIR |=  PF_SCL); asm("ssync;")
#define I2C_ACTIVE			(*pFIO_DIR |=  PF_SDA); *pFIO_INEN &= ~PF_SDA; asm("ssync;")
#define I2C_TRISTATE			(*pFIO_DIR &= ~PF_SDA); *pFIO_INEN |= PF_SDA; asm("ssync;")
#define I2C_READ			((volatile)(*pFIO_FLAG_D & PF_SDA) != 0); asm("ssync;")
#define I2C_SDA(bit)			if(bit) { \
							*pFIO_FLAG_S = PF_SDA; \
							asm("ssync;"); \
						} \
					else	{ \
							*pFIO_FLAG_C = PF_SDA; \
							asm("ssync;"); \
						}
#define I2C_SCL(bit)			if(bit) { \
							*pFIO_FLAG_S = PF_SCL; \
							asm("ssync;"); \
						} \
					else	{ \
							*pFIO_FLAG_C = PF_SCL; \
							asm("ssync;"); \
						}
#define I2C_DELAY			udelay(5)	/* 1/4 I2C clock duration */

#define CFG_I2C_SPEED			50000
#define CFG_I2C_SLAVE			0xFE


#define __ADSPLPBLACKFIN__	1
#define __ADSPBF533__		1

/* 0xFF, 0x7BB07BB0, 0x22547BB0 */
/* #define AMGCTLVAL		(AMBEN_P0 | AMBEN_P1 | AMBEN_P2 | AMCKEN)
#define AMBCTL0VAL		(B1WAT_7 | B1RAT_11 | B1HT_2 | B1ST_3 | B1TT_4 | ~B1RDYPOL |	\
				~B1RDYEN | B0WAT_7 | B0RAT_11 | B0HT_2 | B0ST_3 | B0TT_4 | ~B0RDYPOL | ~B0RDYEN)
#define AMBCTL1VAL		(B3WAT_2 | B3RAT_2 | B3HT_1 | B3ST_1 | B3TT_4 | B3RDYPOL | ~B3RDYEN |	\
				B2WAT_7 | B2RAT_11 | B2HT_2 | B2ST_3 | B2TT_4 | ~B2RDYPOL | ~B2RDYEN)
*/
#define AMGCTLVAL		0xFF
#define AMBCTL0VAL		0x7BB07BB0
#define AMBCTL1VAL		0xFFC27BB0

#define CONFIG_VDSP		1

#ifdef CONFIG_VDSP
#define ET_EXEC_VDSP		0x8
#define SHT_STRTAB_VDSP		0x1
#define ELFSHDRSIZE_VDSP	0x2C
#define VDSP_ENTRY_ADDR		0xFFA00000
#endif

#endif
