/*
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
/* SPI Flash driver
 *
 * Written by sam (sam@realtek.com)
 * 2010-05-01
 *
 */

#define MTD_SPI_DEBUG		2
#define MTD_SPI_TEST_CHIP	0
#include "spi_common.h"
#include <asm/cacheflush.h>
#include <linux/delay.h>
#include <linux/sched.h>

#ifndef SPI_KERNEL
// ****** spi flash driver in bootcode
#include <linux/mtd/cfi.h>
#include <asm/rtl8196x.h>
#include <rtl_types.h>
#if (MTD_SPI_DEBUG == 0)
//0
#define NDEBUG(args...) printf(args)
#define KDEBUG(args...) printf(args)
#define LDEBUG(args...) printf(args)
#endif
//1
#if (MTD_SPI_DEBUG == 1)
#define NDEBUG(args...) printf(args)
#define KDEBUG(args...) printf(args)
#define LDEBUG(args...)
#endif
//2
#if (MTD_SPI_DEBUG == 2)
#define NDEBUG(args...) printf(args)
#define KDEBUG(args...)
#define LDEBUG(args...)
#endif
//3
#if (MTD_SPI_DEBUG == 3)
#define NDEBUG(args...)
#define KDEBUG(args...)
#define LDEBUG(args...)
#endif

#else
// ****** spi flash driver in kernel
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#define malloc	vmalloc
#define free	vfree
//0
#if (MTD_SPI_DEBUG == 0)
#define NDEBUG(args...) printk(args)
#define KDEBUG(args...) printk(args)
#define LDEBUG(args...) printk(args)
#endif
//1
#if (MTD_SPI_DEBUG == 1)
#define NDEBUG(args...) printk(args)
#define KDEBUG(args...) printk(args)
#define LDEBUG(args...)
#endif
//2
#if (MTD_SPI_DEBUG == 2)
#define NDEBUG(args...) printk(args)
#define KDEBUG(args...)
#define LDEBUG(args...)
#endif
//3
#if (MTD_SPI_DEBUG == 3)
#define NDEBUG(args...)
#define KDEBUG(args...)
#define LDEBUG(args...)
#endif

#endif

#ifdef CONFIG_RTL_8198C
 static inline void rtk_udelay(int us)
 {
         if (us >= 1000) {
                 msleep((us+999)/1000);
         } else {
                 udelay(us);
                 cond_resched();
     }
 }
#endif

/* SPI Flash Configuration Register(SFCR) (0xb800-1200) */
#define SFCR					0xb8001200			/*SPI Flash Configuration Register*/
#define SFCR_SPI_CLK_DIV(val)	((val) << 29)
#define SFCR_RBO(val)			((val) << 28)
#define SFCR_WBO(val)			((val) << 27)
//#define SFCR_SPI_TCS(val)		((val) << 23)	//8196B		/*4 bit, 1111 */
#define SFCR_SPI_TCS(val)		((val) << 22)	//8196C Later		/*5 bit, 11111 */

/* SPI Flash Configuration Register(SFCR2) (0xb800-1204) */
#define SFCR2						0xb8001204
#define SFCR2_SFCMD(val)			((val) << 24)			/*8 bit, 1111_1111 */
#define SFCR2_SFSIZE(val)			((val) << 21)			/*3 bit, 111 */
#define SFCR2_RD_OPT(val)			((val) << 20)
#define SFCR2_CMD_IO(val)			((val) << 18)			/*2 bit, 11 */
#define SFCR2_ADDR_IO(val)			((val) << 16)			/*2 bit, 11 */
#define SFCR2_DUMMY_CYCLE(val)		((val) << 13)			/*3 bit, 111 */
#define SFCR2_DATA_IO(val)			((val) << 11)			/*2 bit, 11 */
#define SFCR2_HOLD_TILL_SFDR2(val)	((val) << 10)

/* SPI Flash Control and Status Register(SFCSR)(0xb800-1208) */
#define SFCSR					0xb8001208
#define SFCSR_SPI_CSB0(val)		((val) << 31)
#define SFCSR_SPI_CSB1(val)		((val) << 30)
#define SFCSR_LEN(val)			((val) << 28)			/*2 bits*/
#define SFCSR_SPI_RDY(val)		((val) << 27)
#define SFCSR_IO_WIDTH(val)		((val) << 25)			/*2 bits*/
#define SFCSR_CHIP_SEL(val)		((val) << 24)
#define SFCSR_CMD_BYTE(val)		((val) << 16)			/*8 bit, 1111_1111 */

#define SFCSR_SPI_CSB(val)		((val) << 30)

/* SPI Flash Data Register(SFDR)(0xb800-120c) */
#define SFDR					0xb800120c

/* SPI Flash Data Register(SFDR2)(0xb8001210) */
#define SFDR2					0xb8001210


#define SPI_BLOCK_SIZE			0x10000				/* 64KB */
#define SPI_SECTOR_SIZE			0x1000				/* 4KB */
#define SPI_PAGE_SIZE			0x100				/* 256B */



#define SPICMD_WREN			(0x06 << 24)	/* 06 xx xx xx xx sets the (WEL) write enable latch bit */
#define SPICMD_WRDI			(0x04 << 24)	/* 04 xx xx xx xx resets the (WEL) write enable latch bit*/
#define SPICMD_RDID			(0x9f << 24)	/* 9f xx xx xx xx outputs JEDEC ID: 1 byte manufacturer ID & 2 byte device ID */
#define SPICMD_RDSR			(0x05 << 24)	/* 05 xx xx xx xx to read out the values of the status register */
#define SPICMD_WRSR			(0x01 << 24)	/* 01 xx xx xx xx to write new values to the status register */
#define SPICMD_READ			(0x03 << 24)	/* 03 a1 a2 a3 xx n bytes read out until CS# goes high */
#define SPICMD_FASTREAD		(0x0b << 24)	/* 0b a1 a2 a3 dd n bytes read out until CS# goes high */
#define SPICMD_2READ		(0xbb << 24)	/* bb 12 3d xx xx n bytes read out by 2 I/O until CS# goes high */
#define SPICMD_4READ		(0xeb << 24)	/* eb 3a 3d xx xx n bytes read out by 4 x I/O until CS# goes high */
#define SPICMD_SE			(0x20 << 24)	/* 20 a1 a2 a3 xx to erase the selected sector */
#define SPICMD_BE			(0xd8 << 24)	/* d8 a1 a2 a3 xx to erase the selected block */
#define SPICMD_CE			(0x60 << 24)	/* 60 xx xx xx xx to erase whole chip (cmd or 0xc7) */
#define SPICMD_PP			(0x02 << 24)	/* 02 a1 a2 a3 xx to program the selected page */
#define SPICMD_4PP			(0x38 << 24)	/* 38 3a 3d xx xx quad input to program the selected page */
#define SPICMD_CP			(0xad << 24)	/* ad a1 a2 a3 xx continously program whole chip, the address is automaticlly increase */
#define SPICMD_DP			(0xb9 << 24)	/* b9 xx xx xx xx enters deep power down mode */
#define SPICMD_RDP			(0xab << 24)	/* ab xx xx xx xx release from deep power down mode */
#define SPICMD_RES			(0xab << 24)	/* ab ?? ?? ?? xx to read out 1 byte device ID */
#define SPICMD_REMS_90		(0x90 << 24)	/* 90 ?? ?? ?? xx output the manufacter ID & device ID */
#define SPICMD_REMS_EF		(0xef << 24)	/* ef ?? ?? ?? xx output the manufacter ID & device ID */
#define SPICMD_REMS_DF		(0xdf << 24)	/* df ?? ?? ?? ?? output the manufacture ID & device ID */
#define SPICMD_ENSO			(0xb1 << 24)	/* b1 xx xx xx xx to enter the 512 bit secured OTP mode */
#define SPICMD_EXSO			(0xc1 << 24)	/* c1 xx xx xx xx to exit the 512 bit secured OTP mode */
#define SPICMD_RDSCUR		(0x2b << 24)	/* 2b xx xx xx xx to read value of secured register */
#define SPICMD_WRSCUR		(0x2f << 24)	/* 2f xx xx xx xx to set the lock down bit as "1" (once lock down, can not be updated) */
#define SPICMD_ESRY			(0x70 << 24)	/* 70 xx xx xx xx to enable SO to output RY/BY# during CP mode */
#define SPICMD_DSRY			(0x80 << 24)	/* 80 xx xx xx xx to disable SO to output RY/BY# during CP mode */

#define SPI_STATUS_REG_SRWD		0x07	/* status register write protect */
#define SPI_STATUS_CP			0x06	/* continously program mode */
#define SPI_STATUS_QE			0x06	/* quad enable */
#define SPI_STATUS_BP3			0x05	/* level of protected block */
#define SPI_STATUS_BP2			0x04	/* level of protected block */
#define SPI_STATUS_BP1			0x03	/* level of protected block */
#define SPI_STATUS_BP0			0x02	/* level of protected block */
#define SPI_STATUS_WEL			0x01	/* write enable latch */
#define SPI_STATUS_WIP			0x00	/* write in process bit */

/****** EON ******/

/****** SPANSION ******/
#define SPICMD_SPAN_QOR			(0x6b << 24)	/* 3 0 1 (1 to ?? Quad Ou// write sector use malloc buffer */
#define SPICMD_SPAN_QPP			(0x32 << 24)	/* Quad Page Programming */
#define SPAN_CONF_QUAD			0x01

/****** WINBOND ******/
#define WB_STATUS_QE		0x09	/* QUAD ENABLE (non-volatile) */
#define SPICMD_WB_QPP		(0x32 << 24)		/* Quad Page Program */

/****** SST ******/

/****** GigaDevice ******/
#define GD_STATUS_QE		0x09	/* 9 When the QE bit is set to 0 (Default) the WP# pin and HOLD# pin are enable */
#define SPICMD_GD_HPM		(0xA3 << 24)	/*  High Performance Mode A3H dummy   dummy      dummy */
#define SPICMD_GD_QPP		0x32

/****** ATMEL ******/
//                                                                                                Clock         Address    Dummy  Data
//Command                                                                     Opcode           Frequency         Bytes      Bytes Bytes
#define SPICMD_AT_READ2		(0x3b << 24)	//Dual-Output Read Array				3Bh   0011 1011    Up to 85 MHz           3         1    1+
#define SPICMD_AT_PP2		(0xa2 << 24)	//Dual-Input Byte/Page Program 			A2h   1010 0010    Up to 100 MHz          3         0    1+

/* Spanson Flash */
#define S25FL004A		0x00010212
#define S25FL016A		0x00010214
#define S25FL032A		0x00010215
#define S25FL064A		0x00010216		/*supposed support*/
#define S25FL128P		0x00012018		/*only S25FL128P0XMFI001, Uniform 64KB secotr*/
										/*not support S25FL128P0XMFI011, Uniform 256KB secotr*/
										/*because #define SPI_BLOCK_SIZE 65536 */
#define S25FL032P		0x00010215

/* MICRONIX Flash */
#define MX25L4005		0x00C22013
#define MX25L1605D		0x00C22015
#define MX25L1605E		0x00C22015
#define MX25L3205D		0x00C22016		/*supposed support*/
#define MX25L3205E		0x00C22016		/*supposed support*/
#define MX25L6405D		0x00C22017
#define MX25L6405E		0x00C22017
#define MX25L6445E		0x00C22017
#define MX25L12805D		0x00C22018
#define MX25L12845E		0x00C22018

#define MX25L1635D		0x00C22415
#define MX25L1635E		0x00C22515
#define MX25L1636E		0x00C22515
#define MX25L3235D		0x00C25E16

/* SST Flash */
#define SST25VF032B		0x00BF254A		//4MB
#define SST26VF016		0x00BF2601
#define SST26VF032		0x00BF2602

/* WinBond Flash */
#define W25Q80			0x00EF4014
#define W25Q16			0x00EF4015
#define W25Q32			0x00EF4016
#define W25Q64			0x00EF4017
#define W25Q128			0x00EF4018

/* Eon Flash */
#define EN25F32			0x001c3116
#define EN25F16			0x001c3115
#define EN25Q32			0x001c3016
#define EN25Q16			0x001c3015

/* GigaDevice Flash */
#define GIGADEVICE		0x00c80000		/*factory_id*/
#define GD_Q			0x00c84000		/*memory_type*/
#define GD25Q8			0x00c84014 /*20120302 Verified OK*/
#define GD25Q16			0x00c84015 /*20120305 Verified OK*/
#define GD25Q32			0x00c84016  /*20120305 Verified OK*/
#define GD25Q64			0x00c84017  /*20120305 Verified OK*/
#define GD25Q128			0x00c84018  /*No sample ,Supposed OK*/

/* Atmel Flash */
#define AT25DF161		0x001f4602

/* ESMT Flash */
#define ESMTDEVICE		0x008c0000		/*factory_id*/
#define ESMT_Q			0x008c4000		/*memory_type*/
#define F25L08			0x008c4014  /*Supposed OK , 8Mb*/
#define F25L16			0x008c4015  /*Supposed OK , 16Mb*/
#define F25L32			0x008c4116  /*20130708 Bootcode Verified OK 32Mb*/
#define F25L64			0x008c4117 /*20130708 Bootcode Verified OK 64Mb*/
#define F25L128			0x008c4118  /*No sample ,Supposed OK*/

#define SIZE2N_128K	0x11
#define SIZE2N_256K	0x12
#define SIZE2N_512K	0x13
#define SIZE2N_01MB	0x14
#define SIZE2N_02MB	0x15
#define SIZE2N_04MB	0x16
#define SIZE2N_08MB	0x17
#define SIZE2N_16MB	0x18
#define SIZE2N_32MB	0x19
#define SIZE2N_64MB	0x20
#define SIZE2N_128M	0x21

#define SIZE_256B	0x100
#define SIZE_004K	0x1000
#define SIZE_064K	0x10000
#define REG32(reg)   (*(volatile unsigned int *)((unsigned int)reg))
#define SPI_REG_READ(reg)	*((volatile unsigned int *)(reg))
#ifdef CONFIG_RTL_8198C
#define SPI_REG_LOAD(reg,val)	\
do { \
	while(((*((volatile unsigned int *)SFCSR) & (SFCSR_SPI_RDY(1))) == 0) && ( ((*((volatile unsigned int *)SFCSR) & (1<<30)) == 0)  )  ) \
		; \
	*((volatile unsigned int *)(reg)) = (val); \
	 *((volatile unsigned int *)(SFCSR)); \
	toggle_bit(F_2,1); \
} while(0)
#else
#define SPI_REG_LOAD(reg,val)	\
do { \
	while((*((volatile unsigned int *)SFCSR) & (SFCSR_SPI_RDY(1))) == 0) \
		; \
	*((volatile unsigned int *)(reg)) = (val); \
}  while(0)
#endif
#define IOWIDTH_SINGLE			0x00
#define IOWIDTH_DUAL			0x01
#define IOWIDTH_QUAD			0x02
#define DATA_LENTH1				0x00
#define DATA_LENTH2				0x01
#define DATA_LENTH4				0x02
#define ISFAST_NO				0x00
#define ISFAST_YES				0x01
#define ISFAST_ALL				0x02
#define DUMMYCOUNT_0			0x00
#define DUMMYCOUNT_1			0x01
#define DUMMYCOUNT_2			0x02
#define DUMMYCOUNT_3			0x03
#define DUMMYCOUNT_4			0x04
#define DUMMYCOUNT_5			0x05
#define DUMMYCOUNT_6			0x06
#define DUMMYCOUNT_7			0x07
#define DUMMYCOUNT_8			0x08
#define DUMMYCOUNT_9			0x09

struct spi_flash_type spi_flash_info[2];
//unsigned char ucDispCount = 0;
unsigned char ucSFCR2 = 154;
#if (MTD_SPI_TEST_CHIP == 1)
unsigned int test_spi_flash(unsigned char ucChip);
#endif

struct spi_flash_known spi_flash_registed[] = {
/****************************************** Micronix Flash ******************************************/
//#define MX25L1605D		0x00C22015
{0x00C22015, 0x00, SIZE2N_02MB, SIZE_064K, SIZE_004K, SIZE_256B, "MX25L1605D/E"
#if (SPI_DRIVER_MODE == 1)
, 50, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#else
, 50, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define MX25L3205D		20MHZ
//#define MX25L3206E		0x00C22016
{0x00C22016, 0x00, SIZE2N_04MB, SIZE_064K, SIZE_004K, SIZE_256B, "MX25L3206E"
#if (SPI_DRIVER_MODE == 1)
, 50, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#else
, 50, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define MX25L6405D		0x00C22017
//#define MX25L6405E		0x00C22017
//#define MX25L6445E		0x00C22017
{0x00C22017, 0x00, SIZE2N_08MB, SIZE_064K, SIZE_004K, SIZE_256B, "MX6405D/05E/45E"
#if (SPI_DRIVER_MODE == 1)
, 50, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#else
, 50, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#defien MX25L12805D		0x00C22018
//#define MX25L12845E		0x00C22018
{0x00C22018, 0x00, SIZE2N_16MB, SIZE_064K, SIZE_004K, SIZE_256B, "MX25L12805D/45E"
#if (SPI_DRIVER_MODE == 1)
, 50, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#else
, 50, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define MX25L1635D		0x00C22415
{0x00C22415, 0x00, SIZE2N_02MB, SIZE_064K, SIZE_004K, SIZE_256B, "MX25L1635D"
#if (SPI_DRIVER_MODE == 1)
, 75, ComSrlCmd_SE,  SpiRead_1443EB, mxic_spi_setQEBit,  PageWrite_144038
#else
, 75, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define MX25L1635E		0x00C22515 (clock 108 down to 54)
{0x00C22515, 0x00, SIZE2N_02MB, SIZE_064K, SIZE_004K, SIZE_256B, "MX25L1635E/36E"
#if (SPI_DRIVER_MODE == 1)
, 50, ComSrlCmd_SE,  SpiRead_1443EB, mxic_spi_setQEBit,  PageWrite_144038
#else
, 50, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define MX25L3235D		0x00C25E16
{0x00C25E16, 0x00, SIZE2N_04MB, SIZE_064K, SIZE_004K, SIZE_256B, "MX25L3235D"
#if (SPI_DRIVER_MODE == 1)
, 75, ComSrlCmd_SE,  SpiRead_1443EB, mxic_spi_setQEBit,  PageWrite_144038
#else
, 104, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define MX25L3235D		0x00C25E16
{0x00C22019, 0x00, SIZE2N_08MB, SIZE_064K, SIZE_004K, SIZE_256B, "MX25L25635F"
#if (SPI_DRIVER_MODE == 1)
, 75, ComSrlCmd_SE,  SpiRead_1443EB, mxic_spi_setQEBit,  PageWrite_144038
#else
, 104, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
/****************************************** Spanson Flash ******************************************/
//#define S25FL016A		0x00010214
{0x00010214, 0x00, SIZE2N_02MB, SIZE_064K, SIZE_064K, SIZE_256B, "S25FL016A"
#if (SPI_DRIVER_MODE == 1)
, 50, ComSrlCmd_BE, SpiRead_11110B, ComSrlCmd_NoneQeBit, PageWrite_111002
#else
, 50, ComSrlCmd_BE, SpiRead_11110B, ComSrlCmd_NoneQeBit, PageWrite_111002
#endif
},
//#define S25FL032A		0x00010215
{0x00010215, 0x00, SIZE2N_04MB, SIZE_064K, SIZE_064K, SIZE_256B, "S25FL032A"
#if (SPI_DRIVER_MODE == 1)
, 50, ComSrlCmd_BE, SpiRead_11110B, ComSrlCmd_NoneQeBit, PageWrite_111002
#else
, 50, ComSrlCmd_BE, SpiRead_11110B, ComSrlCmd_NoneQeBit, PageWrite_111002
#endif
},
//#define S25FL064A		0x00010216
{0x00010216, 0x00, SIZE2N_08MB, SIZE_064K, SIZE_064K, SIZE_256B, "S25FL064P"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_BE, SpiRead_1443EB, span_spi_setQEBit, PageWrite_114032
#else
, 104, ComSrlCmd_BE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
/****************************************** Eon Flash ******************************************/
//#define EN25F16			0x001c3115
{0x001c3115, 0x00, SIZE2N_02MB, SIZE_064K, SIZE_004K, SIZE_256B, "EN25F16"
#if (SPI_DRIVER_MODE == 1)
, 100, ComSrlCmd_SE, SpiRead_11110B, ComSrlCmd_NoneQeBit, PageWrite_111002
#else
, 100, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define EN25F32			0x001c3116
{0x001c3116, 0x00, SIZE2N_04MB, SIZE_064K, SIZE_004K, SIZE_256B, "EN25F32-100"
#if (SPI_DRIVER_MODE == 1)
, 100, ComSrlCmd_SE, SpiRead_11110B, ComSrlCmd_NoneQeBit, PageWrite_111002
#else
, 100, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define EN25Q16			0x001c3015
{0x001c3015, 0x00, SIZE2N_02MB, SIZE_064K, SIZE_004K, SIZE_256B, "EN25Q16A"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE, SpiRead_1443EB, ComSrlCmd_NoneQeBit, PageWrite_111002
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define EN25Q32			0x001c3016
{0x001c3016, 0x00, SIZE2N_04MB, SIZE_064K, SIZE_004K, SIZE_256B, "EN25Q32A"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE, SpiRead_1443EB, ComSrlCmd_NoneQeBit, PageWrite_111002
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define EN25Q64			0x001c3017
{0x001c3017, 0x00, SIZE2N_08MB, SIZE_064K, SIZE_004K, SIZE_256B, "EN25Q64A"
#if (SPI_DRIVER_MODE == 1)
, 50, ComSrlCmd_SE, SpiRead_1443EB, ComSrlCmd_NoneQeBit, PageWrite_111002
#else
, 50, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
/****************************************** SST Flash ******************************************/
//#define SST25VF032B		0x00BF254A (clock 80 down to 40)
{0x00BF254A, 0x00, SIZE2N_04MB, SIZE_064K, SIZE_004K, SIZE_256B, "SST25VF032B"
#if (SPI_DRIVER_MODE == 1)
, 40, ComSrlCmd_SE, SpiRead_11110B, ComSrlCmd_NoneQeBit, sst_PageWrite_s1
#else
, 40, ComSrlCmd_SE, SpiRead_11110B, ComSrlCmd_NoneQeBit, sst_PageWrite_s1
#endif
},
/****************************************** GigaDevice Flash ******************************************/
//#define GD25Q8			0x00c84014
{0x00c84014, 0x00, SIZE2N_01MB, SIZE_064K, SIZE_004K, SIZE_256B, "GD25Q8"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE, SpiRead_1443EB, gd_spi_setQEBit, PageWrite_111002
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define GD25Q16			0x00c84015
{0x00c84015, 0x00, SIZE2N_02MB, SIZE_064K, SIZE_004K, SIZE_256B, "GD25Q16"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE, SpiRead_1443EB, gd_spi_setQEBit, PageWrite_111002
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define GD25Q32			0x00c84016
{0x00c84016, 0x00, SIZE2N_04MB, SIZE_064K, SIZE_004K, SIZE_256B, "GD25Q32"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE, SpiRead_1443EB, gd_spi_setQEBit, PageWrite_114032
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define GD25Q64			0x00c84017
{0x00c84017, 0x00, SIZE2N_08MB, SIZE_064K, SIZE_004K, SIZE_256B, "GD25Q64"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE, SpiRead_1443EB, gd_spi_setQEBit, PageWrite_114032
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define GD25Q128			0x00c84018
{0x00c84018, 0x00, SIZE2N_16MB, SIZE_064K, SIZE_004K, SIZE_256B, "GD25Q128"
#if (SPI_DRIVER_MODE == 1)
, 84, ComSrlCmd_SE, SpiRead_1443EB, gd_spi_setQEBit, PageWrite_114032
#else
, 84, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
/****************************************** WinBond Flash ******************************************/
//#define W25Q16			0x00EF4015
{0x00EF4015, 0x00, SIZE2N_02MB, SIZE_064K, SIZE_004K, SIZE_256B, "W25Q16"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE, SpiRead_1443EB, wb_spi_setQEBit, PageWrite_114032
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define W25Q32			0x00EF4016
{0x00EF4016, 0x00, SIZE2N_04MB, SIZE_064K, SIZE_004K, SIZE_256B, "W25Q32"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE, SpiRead_1443EB, wb_spi_setQEBit, PageWrite_114032
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define W25Q64                        0x00EF4017
{0x00EF4017, 0x00, SIZE2N_08MB, SIZE_064K, SIZE_004K, SIZE_256B, "W25Q64"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE, SpiRead_1443EB, wb_spi_setQEBit, PageWrite_114032
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define W25Q128                       0x00EF4016
{0x00EF4018, 0x00, SIZE2N_16MB, SIZE_064K, SIZE_004K, SIZE_256B, "W25Q128"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE, SpiRead_1443EB, wb_spi_setQEBit, PageWrite_114032
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define W25X16			0x00EF3015
{0x00EF3015, 0x00, SIZE2N_02MB, SIZE_064K, SIZE_004K, SIZE_256B, "W25X16"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE, SpiRead_11213B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define W25X32			0x00EF3016
{0x00EF3016, 0x00, SIZE2N_04MB, SIZE_064K, SIZE_004K, SIZE_256B, "W25X32"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE, SpiRead_11213B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define W25X64			0x00EF3017
{0x00EF3016, 0x00, SIZE2N_08MB, SIZE_064K, SIZE_004K, SIZE_256B, "W25X64"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE, SpiRead_11213B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
/****************************************** ATMEL Flash ******************************************/
//#define AT25DF161		0x001f4602
{0x001f4602, 0x00, SIZE2N_02MB, SIZE_064K, SIZE_004K, SIZE_256B, "AT25DF161"
#if (SPI_DRIVER_MODE == 1)
, 85, ComSrlCmd_SE, SpiRead_11213B, ComSrlCmd_NoneQeBit, PageWrite_1120A2
#else
, 85, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
/****************************************** Qingistek ******************************************/
//#define PM25LQ032		0x007f9d46
{0x007f9d46, 0x00, SIZE2N_04MB, SIZE_064K, SIZE_004K, SIZE_256B, "PM25LQ032"
#if (SPI_DRIVER_MODE == 1)
, 85, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#else
, 85, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
/****************************************** ESMT ******************************************/
//#define PM25LQ032		0x007f9d46
{0x008c2016, 0x00, SIZE2N_04MB, SIZE_064K, SIZE_004K, SIZE_256B, "F25L32PA-100"
#if (SPI_DRIVER_MODE == 1)
, 100, ComSrlCmd_SE,  SpiRead_1221BB, ComSrlCmd_NoneQeBit,  PageWrite_111002
#else
, 50, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
/****************************************** ESMT Flash ******************************************/
//#define F25L08		0x008c4014
{0x008c4014, 0x00, SIZE2N_01MB, SIZE_064K, SIZE_004K, SIZE_256B, "F25L08"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE,  SpiRead_1443EB, mxic_spi_setQEBit,  PageWrite_144038
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define F25L16		0x008c4015
{0x008c4015, 0x00, SIZE2N_02MB, SIZE_064K, SIZE_004K, SIZE_256B, "F25L16"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE,  SpiRead_1443EB, mxic_spi_setQEBit,  PageWrite_144038
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define F25L32		0x008c4116
{0x008c4116, 0x00, SIZE2N_04MB, SIZE_064K, SIZE_004K, SIZE_256B, "F25L32"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE,  SpiRead_1443EB, mxic_spi_setQEBit,  PageWrite_144038
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define F25L64		0x008c4117
{0x008c4117, 0x00, SIZE2N_08MB, SIZE_064K, SIZE_004K, SIZE_256B, "F25L64"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE,  SpiRead_1443EB, mxic_spi_setQEBit,  PageWrite_144038
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
//#define F25L128		0x008c4118
{0x008c4118, 0x00, SIZE2N_16MB, SIZE_064K, SIZE_004K, SIZE_256B, "F25L128"
#if (SPI_DRIVER_MODE == 1)
, 80, ComSrlCmd_SE,  SpiRead_1443EB, mxic_spi_setQEBit,  PageWrite_144038
#else
, 80, ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002
#endif
},
};

unsigned int ComSrlCmd_EN4B(unsigned char ucChip, unsigned int uiLen);

// spi flash probe
void spi_regist(unsigned char ucChip)
{
	unsigned int ui, i, uiCount;
	unsigned char pucBuffer[4];

	ui = ComSrlCmd_RDID(ucChip, 4);
	ui = ComSrlCmd_RDID(ucChip, 4);
	ui = ui >> 8;
	#ifdef CONFIG_SPI_3to4BYTES_ADDRESS_SUPPORT
	/*For 8198C */
	/*
		#define MX25L25635F		0x00C22019
		It's 3to4-byte addressing capability SPI flash (32MB)	*/
	if((ui == 0xC22019)||(0x00EF4018))
	{
		//REG32(SFDR)= SPICMD_EN4B;//SPI Flash set to 4Byte mode
		ComSrlCmd_EN4B(ucChip, 4);//for SPI CS#0 , send "EN4B" command
	}
	#endif
	uiCount = sizeof(spi_flash_registed) / sizeof(struct spi_flash_known);

	for (i = 0; i < uiCount; i++)
	{
		if(spi_flash_registed[i].uiChipId == ui)
		{
			break;
		}
	}
	if(i == uiCount)
	{
		// default setting
		i = ui & 0xff;
		if((i < SIZE2N_128K) || (i > SIZE2N_128M))
		{
			i = SIZE2N_04MB;
		}
		setFSCR(ucChip, 40, 1, 1, 15);
		set_flash_info(ucChip, ui, i, SIZE_064K, SIZE_004K, SIZE_256B, "UNKNOWN", ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002, 40);
		//set_flash_info(ucChip, ui, i, SIZE_064K, SIZE_004K, SIZE_256B, "UNKNOWN", ComSrlCmd_SE,  SpiRead_11110B, ComSrlCmd_NoneQeBit,  PageWrite_111002, 40);
	}
	else
	{
		// have registed
		setFSCR(ucChip, spi_flash_registed[i].chipClock, 1, 1, 15);
		set_flash_info(ucChip, ui, spi_flash_registed[i].uiCapacityId, spi_flash_registed[i].uiBlockSize, spi_flash_registed[i].uiSectorSize, spi_flash_registed[i].uiPageSize, spi_flash_registed[i].pcChipName, spi_flash_registed[i].pfErase, spi_flash_registed[i].pfRead, spi_flash_registed[i].pfQeBit, spi_flash_registed[i].pfPageWrite, spi_flash_registed[i].chipClock);
	}
	spi_flash_info[ucChip].pfQeBit(ucChip);
	prnFlashInfo(ucChip, spi_flash_info[ucChip]);
	ui = spi_flash_info[ucChip].pfRead(ucChip, 0x00, 4, pucBuffer);
#if (MTD_SPI_TEST_CHIP == 1)
	ui = test_spi_flash(ucChip);
#endif
	LDEBUG("spi_regist: ucChip=%x; i=%x; uiCount=%x\n", ucChip, i, uiCount);
}
// set spi_flash_info struction content
void set_flash_info(unsigned char ucChip, unsigned int chip_id, unsigned int device_cap, unsigned int block_size, unsigned int sector_size, unsigned int page_size, char* chip_name, FUNC_ERASE pfErase, FUNC_READ pfRead, FUNC_SETQEBIT pfQeBit, FUNC_PAGEWRITE pfPageWrite, unsigned int chipClock)
{
	unsigned int ui = 1 << device_cap;
	spi_flash_info[ucChip].chip_id = chip_id;
	spi_flash_info[ucChip].mfr_id = (chip_id >> 16) & 0xff;
	spi_flash_info[ucChip].dev_id = (chip_id >> 8) & 0xff;
	spi_flash_info[ucChip].capacity_id = (chip_id) & 0xff;
	#if (CONFIG_SPI_3to4BYTES_ADDRESS_SUPPORT)
	spi_flash_info[ucChip].size_shift = calShift(spi_flash_info[ucChip].capacity_id, device_cap);
	#else
	spi_flash_info[ucChip].size_shift = calShift(spi_flash_info[ucChip].capacity_id, device_cap);
	#endif
	spi_flash_info[ucChip].device_size = device_cap;			// 2 ^ N (bytes)
	#if CONFIG_SPI_3to4BYTES_ADDRESS_SUPPORT
	spi_flash_info[ucChip].chip_size =  ui*4;
	#else
	//jason
	#if defined(CONFIG_RTL_8198C)
	if(REG32(0xb8000008)&0x1)
		spi_flash_info[ucChip].chip_size =  ui*4;
	else
	#endif
		spi_flash_info[ucChip].chip_size =  ui;
	#endif
	spi_flash_info[ucChip].block_size = block_size;
	spi_flash_info[ucChip].block_cnt = ui / block_size;
	spi_flash_info[ucChip].sector_size = sector_size;
	spi_flash_info[ucChip].sector_cnt = ui / sector_size;
	spi_flash_info[ucChip].page_size = page_size;
	spi_flash_info[ucChip].page_cnt = sector_size / page_size;
	spi_flash_info[ucChip].chip_name = chip_name;
	spi_flash_info[ucChip].pfErase = pfErase;
	spi_flash_info[ucChip].pfWrite = ComSrlCmd_ComWriteData;
	spi_flash_info[ucChip].pfRead = pfRead;
	spi_flash_info[ucChip].pfQeBit = pfQeBit;
	spi_flash_info[ucChip].pfPageWrite = pfPageWrite;
	spi_flash_info[ucChip].chipClock = chipClock;
	//SPI_REG_LOAD(SFCR2, 0x0bb08000);
	LDEBUG("set_flash_info: ucChip=%x; chip_id=%x; device_cap=%x; block_size=%x; sector_size=%x; page_size=%x; chip_name=%s\n", ucChip, chip_id, device_cap, block_size, sector_size, page_size, chip_name);
}

/****************************** Common function ******************************/
// get Dram Frequence
unsigned int CheckDramFreq(void)                       //JSW:For 8196C
{
	unsigned short usFreqBit;
#if defined(CONFIG_RTL8198)
	unsigned short usFreqVal[] = {65, 181, 150, 125, 156, 168, 237, 193};
#elif defined(CONFIG_RTL_8198C)
	unsigned short  usFreqVal[]={ 212,    237,     262, 287, 312,  337,  362,  387,  425,  475, 525, 575, 625,  675,  725,  775  };
#elif defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	unsigned short usFreqVal[] = {156, 193, 181, 231, 212, 125, 237, 168};
#else
	unsigned short usFreqVal[] = {65, 78, 125, 150, 156, 168, 193, 193};
#endif
	usFreqBit = (0x00001C00 & (*(unsigned int*)0xb8000008)) >> 10 ;
	LDEBUG("CheckDramFreq:usFreqVal=%dMHZ; usFreqBit=%x; B8000008=%x;\n", usFreqVal[usFreqBit], usFreqBit, (*(unsigned int*)0xb8000008));
	NDEBUG("SDRAM CLOCK:%dMHZ\n", usFreqVal[usFreqBit]);
	return usFreqVal[usFreqBit];
}
// Set FSCR register, disable this function in kernel
void setFSCR(unsigned char ucChip, unsigned int uiClkMhz, unsigned int uiRBO, unsigned int uiWBO, unsigned int uiTCS)
{
#ifndef SPI_KERNEL
	unsigned int ui, uiClk;
	uiClk = CheckDramFreq();
#ifdef CONFIG_AUTO_PROBE_LIMITED_SPI_CLK_UNDER_40MHZ
	uiClkMhz = 40;
#endif
	ui = uiClk / uiClkMhz;
	if((uiClk % uiClkMhz) > 0)
	{
		ui = ui + 1;
	}
	if((ui % 2) > 0)
	{
		ui = ui + 1;
	}
	spi_flash_info[ucChip].chip_clk = uiClk / ui;
//	SPI_REG_LOAD(SFCR, SFCR_SPI_CLK_DIV((ui-2)/2) | SFCR_RBO(uiRBO) | SFCR_WBO(uiWBO) | SFCR_SPI_TCS(uiTCS));
	SPI_REG_LOAD(SFCR, SFCR_SPI_CLK_DIV(7) | SFCR_RBO(uiRBO) | SFCR_WBO(uiWBO) | SFCR_SPI_TCS(uiTCS));
	LDEBUG("setFSCR:uiClkMhz=%d, uiRBO=%d, uiWBO=%d, uiTCS=%d, resMhz=%d, vale=%8x\n", uiClkMhz, uiRBO, uiWBO, uiTCS, spi_flash_info[ucChip].chip_clk, SPI_REG_READ(SFCR));
#endif
}
// Calculate write address group
void calAddr(unsigned int uiStart, unsigned int uiLenth, unsigned int uiSectorSize, unsigned int* uiStartAddr, unsigned int*  uiStartLen, unsigned int* uiSectorAddr, unsigned int* uiSectorCount, unsigned int* uiEndAddr, unsigned int* uiEndLen)
{
	unsigned int ui;
	// only one sector
	if ((uiStart + uiLenth) < ((uiStart / uiSectorSize + 1) * uiSectorSize))
	{	// start
		*uiStartAddr = uiStart;
		*uiStartLen = uiLenth;
		//middle
		*uiSectorAddr = 0x00;
		*uiSectorCount = 0x00;
		// end
		*uiEndAddr = 0x00;
		*uiEndLen = 0x00;
	}
	//more then one sector
	else
	{
		// start
		*uiStartAddr = uiStart;
		*uiStartLen = uiSectorSize - (uiStart % uiSectorSize);
		if(*uiStartLen == uiSectorSize)
		{
			*uiStartLen = 0x00;
		}
		// middle
		ui = uiLenth - *uiStartLen;
		*uiSectorAddr = *uiStartAddr + *uiStartLen;
		*uiSectorCount = ui / uiSectorSize;
		//end
		*uiEndAddr = *uiSectorAddr + (*uiSectorCount * uiSectorSize);
		*uiEndLen = ui % uiSectorSize;
	}
	LDEBUG("calAddr:uiStart=%x; uiSectorSize=%x; uiLenth=%x;-> uiStartAddr=%x; uiStartLen=%x; uiSectorAddr=%x; uiSectorCount=%x; uiEndAddr=%x; uiEndLen=%x;\n",uiStart, uiSectorSize, uiLenth, *uiStartAddr, *uiStartLen, *uiSectorAddr, *uiSectorCount, *uiEndAddr, *uiEndLen);
}
// Calculate chip capacity shift bit
unsigned char calShift(unsigned char ucCapacityId, unsigned char ucChipSize)
{
	unsigned int ui;
	if(ucChipSize > ucCapacityId)
	{
		ui = ucChipSize - ucCapacityId;
	}
	else
	{
		ui = ucChipSize + 0x100 -ucCapacityId;
	}
	LDEBUG("calShift: ucCapacityId=%x; ucChipSize=%x; ucReturnVal=%x\n", ucCapacityId, ucChipSize, ui);
	return (unsigned char)ui;
}
// Print spi_flash_type
void prnFlashInfo(unsigned char ucChip, struct spi_flash_type sftInfo)
{
#if (SPI_DRIVER_MODE == 1)
	NDEBUG(" ========================= Registed SPI Flash Model ========================= \n");
	NDEBUG("|No chipID  Sft chipSize blkSize secSize pageSize sdCk opCk      chipName    |\n");
	NDEBUG("|%2d %6xh %2xh %7xh %6xh %6xh %7xh %4d %4d %17s|\n", ucChip, sftInfo.chip_id, sftInfo.size_shift, sftInfo.chip_size, sftInfo.block_size, sftInfo.sector_size, sftInfo.page_size, sftInfo.chipClock, sftInfo.chip_clk, sftInfo.chip_name);
	////////1111 2222 3333 4444 5555 6666 7777 8888 9999 aaaa
	NDEBUG(" ============================================================================ \n");
#else
	NDEBUG(" ------------------------- Force into Single IO Mode ------------------------ \n");
	NDEBUG("|No chipID  Sft chipSize blkSize secSize pageSize sdCk opCk      chipName    |\n");
	NDEBUG("|%2d %6xh %2xh %7xh %6xh %6xh %7xh %4d %4d %17s|\n", ucChip, sftInfo.chip_id, sftInfo.size_shift, sftInfo.chip_size, sftInfo.block_size, sftInfo.sector_size, sftInfo.page_size, sftInfo.chipClock, sftInfo.chip_clk, sftInfo.chip_name);
	////////1111 2222 3333 4444 5555 6666 7777 8888 9999 aaaa
	NDEBUG(" ---------------------------------------------------------------------------- \n");
#endif
}

// Check WIP bit
unsigned int spiFlashReady(unsigned char ucChip)
{
	unsigned int uiCount, ui;
	uiCount = 0;
	while (1)
	{
		uiCount++;
		ui = SeqCmd_Read(ucChip, IOWIDTH_SINGLE, SPICMD_RDSR, 1);
		if ((ui & (1 << SPI_STATUS_WIP)) == 0)
		{
			break;
		}
	}
	KDEBUG("spiFlashReady: uiCount=%x\n", uiCount);
	return uiCount;
}
//toggle CS
void rstSPIFlash(unsigned char ucChip)
{
	SFCSR_CS_L(ucChip, 0, IOWIDTH_SINGLE);
	SFCSR_CS_H(ucChip, 0, IOWIDTH_SINGLE);
	SFCSR_CS_L(ucChip, 0, IOWIDTH_SINGLE);
	SFCSR_CS_H(ucChip, 0, IOWIDTH_SINGLE);
	LDEBUG("rstFPIFlash: ucChip=%x;\n", ucChip);
}
#ifdef CONFIG_RTL_8198C
#define F_1 1<<25
#define F_2 2<<25
#define F_3 4<<25
void toggle_bit(unsigned int bit,int value)
{
int count=1;
	preempt_disable();

while(count--)
{
				#if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif
	#if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif
	#if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif

#if 0

	if(value)
	 REG32(0xb8003528)= REG32(0xb8003528)|bit;
	else
	REG32(0xb8003528)= REG32(0xb8003528)&(~(bit));
#endif
		#if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif
	#if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif
	#if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif
	#if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif
	#if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif
	#if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif

}
#if 1
{
	static unsigned int cpuid = 0;
	if(cpuid !=  smp_processor_id())
	{
		cpuid=smp_processor_id();
		if(cpuid)
		{
		//////////////////////////////////////////
		   preempt_disable();//try 1

		 preempt_enable();
		////////////////////////////////////////

		}
		else
		{

		}
		///////////////////////////////
		// preempt_disable();
		//__sync(); //try 2
		//preempt_enable();
		//////////////////////////

	}
}
#endif
	preempt_enable();
}
#endif
/****************************** Layer 1 ******************************/
// set cs high
#ifdef CONFIG_RTL_8198C
#ifdef CONFIG_SPI_3to4BYTES_ADDRESS_SUPPORT
// Read Identification (EN4B) Sequence (Command 0xb7)
unsigned int ComSrlCmd_EN4B(unsigned char ucChip, unsigned int uiLen)
{
	#define REG32(reg)		(*(volatile unsigned int   *)(reg))
	unsigned int ui;

	//SPI_REG_LOAD(SFCR, (SFCR_SPI_CLK_DIV(7) | SFCR_RBO(1) | SFCR_WBO(1) | SPI_TSLCH(15)|SFCR_SPI_TCS(31)));		//SFCR default setting

	rstSPIFlash(ucChip);

	SFCSR_CS_L(ucChip, 0, IOWIDTH_SINGLE);

	SeqCmd_Order(ucChip,  IOWIDTH_SINGLE, SPICMD_EN4B);

	ui = SPI_REG_READ(SFDR);

	SFCSR_CS_H(ucChip, 0, IOWIDTH_SINGLE);

	LDEBUG("ComSrlCmd_RDID: ucChip=%x; uiLen=%x; returnValue=%x; SPICMD_RDID=%x;\n", ucChip, uiLen, ui, SPICMD_RDID);
	//printf("ComSrlCmd_EN4B: ucChip=%x; uiLen=%x; returnValue=%x; SPICMD_EN4B=%x;\n", ucChip, uiLen, ui, SPICMD_EN4B);

	REG32(0xb8000008)|=(0x1<<0);//Set bit[0] as 1 ,to inform SPI Flash controller change read cycle from 3-Byte to 4-Byte
	return ui;
}

// Read Identification (EX4B) Sequence (Command 0xe9)
unsigned int ComSrlCmd_EX4B(unsigned char ucChip, unsigned int uiLen)
{

	unsigned int ui;

	//SPI_REG_LOAD(SFCR, (SFCR_SPI_CLK_DIV(7) | SFCR_RBO(1) | SFCR_WBO(1) | SPI_TSLCH(15)|SFCR_SPI_TCS(31)));		//SFCR default setting

	rstSPIFlash(ucChip);

	SFCSR_CS_L(ucChip, 0, IOWIDTH_SINGLE);

	SeqCmd_Order(ucChip,  IOWIDTH_SINGLE, SPICMD_EX4B);

	ui = SPI_REG_READ(SFDR);

	SFCSR_CS_H(ucChip, 0, IOWIDTH_SINGLE);

	//LDEBUG("ComSrlCmd_RDID: ucChip=%x; uiLen=%x; returnValue=%x; SPICMD_RDID=%x;\n", ucChip, uiLen, ui, SPICMD_RDID);
	//printf("ComSrlCmd_EX4B: ucChip=%x; uiLen=%x; returnValue=%x; SPICMD_EX4B=%x;\n", ucChip, uiLen, ui, SPICMD_EX4B);

	REG32(0xb8000008)&=~(0x1);//Set bit[0] as 1 ,to inform SPI Flash controller change read cycle from 3-Byte to 4-Byte
	return ui;
}
#endif

void SFCSR_CS_L(unsigned char ucChip, unsigned char ucLen, unsigned char ucIOWidth)
{
	int count=0;
		LDEBUG("SFCSR_CS_L: ucChip=%x; uiLen=%x; ucIOWidth=%x;\n", ucChip, ucLen, ucIOWidth);

preempt_disable();
	 *((volatile unsigned int *)(0xa0000000));
		#if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif
	  rtk_udelay(1);

	while((*((volatile unsigned int *)SFCSR) & (SFCSR_SPI_RDY(1))) == 0)
	{

		#if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif
		rtk_udelay(1);

		count++;
		if(count>10)
		{
			count=0;
		printk("L1\r\n");
			break;
		}

	}



	SPI_REG_LOAD(SFCSR,SFCSR_SPI_CSB(1 + (ucChip)) | SFCSR_LEN(ucLen) | SFCSR_SPI_RDY(1) |  SFCSR_IO_WIDTH(ucIOWidth));

	count=0;
	count= smp_processor_id();
	while(( ( ((*((volatile unsigned int *)SFCSR) & (1<<30)) != (1<<30)) )  ))
	{
		printk("~ %d  ",count);
		 __flush_cache_all();
	}
	 count=0;
	count= smp_processor_id();
	while(( ( ((*((volatile unsigned int *)SFCSR) & (1<<30)) != (1<<30)) )  ))
	{
		printk("~ %d  ",count);
		 __flush_cache_all();
	}
	 count=0;
	count= smp_processor_id();
	while(( ( ((*((volatile unsigned int *)SFCSR) & (1<<30)) != (1<<30)) )  ))
	{
		printk("~ %d  ",count);
		 __flush_cache_all();
	}
	 count=0;

while((*((volatile unsigned int *)SFCSR) & (SFCSR_SPI_RDY(1))) == 0)
	{


		 #if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif
		 // udelay(1);

                count++;
                if(count>10)
                {
                        count=0;

                printk("L2\r\n");
  //                    break;
                }



	}
smp_wmb();
 preempt_enable();
}
#else
// set cs high
void SFCSR_CS_L(unsigned char ucChip, unsigned char ucLen, unsigned char ucIOWidth)
{
	LDEBUG("SFCSR_CS_L: ucChip=%x; uiLen=%x; ucIOWidth=%x;\n", ucChip, ucLen, ucIOWidth);
	while((*((volatile unsigned int *)SFCSR) & (SFCSR_SPI_RDY(1))) == 0);
	//*((volatile unsigned int *)(SFCSR)) = SFCSR_SPI_CSB(1 + (ucChip)) | SFCSR_LEN(uiLen) | SFCSR_SPI_RDY(1) |  SFCSR_IO_WIDTH(ucIOWidth) | SFCSR_CHIP_SEL(0) | SFCSR_CMD_BYTE(0);
	*((volatile unsigned int *)(SFCSR)) = SFCSR_SPI_CSB(1 + (ucChip)) | SFCSR_LEN(ucLen) | SFCSR_SPI_RDY(1) |  SFCSR_IO_WIDTH(ucIOWidth);
}
#endif
// set cs low
#ifdef CONFIG_RTL_8198C
void SFCSR_CS_H(unsigned char ucChip, unsigned char ucLen, unsigned char ucIOWidth)
{
	int count=0;
	preempt_disable();
	LDEBUG("SFCSR_CS_H: ucChip=%x; uiLen=%x; ucIOWidth=%x;\n", ucChip, ucLen, ucIOWidth);

		#if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif



	//toggle_bit(F_1,0);
	 //REG32(0xb8003528) &= ~(7<<25);
	while((*((volatile unsigned int *)SFCSR) & (SFCSR_SPI_RDY(1))) == 0)
	{
				 #if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif
		udelay(1);

                count++;
                if(count>10)
                {
                        count=0;
                printk("h1\r\n");
		       break;
                }



	}
 preempt_disable();
	//*((volatile unsigned int *)(SFCSR)) = SFCSR_SPI_CSB(3) | SFCSR_LEN(uiLen) | SFCSR_SPI_RDY(1) |  SFCSR_IO_WIDTH(ucIOWidth) | SFCSR_CHIP_SEL(0) | SFCSR_CMD_BYTE(0);
//	*((volatile unsigned int *)(SFCSR)) = SFCSR_SPI_CSB(3) | SFCSR_LEN(ucLen) | SFCSR_SPI_RDY(1) |  SFCSR_IO_WIDTH(ucIOWidth);
	SPI_REG_LOAD(SFCSR,(SFCSR_SPI_CSB(3) | SFCSR_LEN(ucLen) | SFCSR_SPI_RDY(1) |  SFCSR_IO_WIDTH(ucIOWidth)));
	count=0;
	preempt_enable();
	 count=smp_processor_id();
	 while(( ( ((*((volatile unsigned int *)SFCSR) & (1<<30)) == (0)) )  ))
	{
	 printk("! %d  ",count);
	__flush_cache_all();

	}
	 count=0;
	while(( ( ((*((volatile unsigned int *)SFCSR) & (1<<30)) == (0)) )  ))
	{
	 printk("! %d  ",count);
	 __flush_cache_all();

	}
	 count=0;
	while(( ( ((*((volatile unsigned int *)SFCSR) & (1<<30)) == (0)) )  ))
	{
	 printk("! %d  ",count);
	 __flush_cache_all();

	}
	 count=0;



		while((*((volatile unsigned int *)SFCSR) & (SFCSR_SPI_RDY(1))) == 0)
	    {
		    #if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif
             rtk_udelay(1);
		   count++;
                if(count>100)
                {
                        count=0;

                printk("h2\r\n");
//		break;
                }


	}
	// toggle_bit(F_1,1);

 preempt_enable();
}
#else
// set cs low
void SFCSR_CS_H(unsigned char ucChip, unsigned char ucLen, unsigned char ucIOWidth)
{
	LDEBUG("SFCSR_CS_H: ucChip=%x; uiLen=%x; ucIOWidth=%x;\n", ucChip, ucLen, ucIOWidth);
	while((*((volatile unsigned int *)SFCSR) & (SFCSR_SPI_RDY(1))) == 0);
	//*((volatile unsigned int *)(SFCSR)) = SFCSR_SPI_CSB(3) | SFCSR_LEN(uiLen) | SFCSR_SPI_RDY(1) |  SFCSR_IO_WIDTH(ucIOWidth) | SFCSR_CHIP_SEL(0) | SFCSR_CMD_BYTE(0);
	*((volatile unsigned int *)(SFCSR)) = SFCSR_SPI_CSB(3) | SFCSR_LEN(ucLen) | SFCSR_SPI_RDY(1) |  SFCSR_IO_WIDTH(ucIOWidth);
}
#endif

// Read Identification (RDID) Sequence (Command 9F)
unsigned int ComSrlCmd_RDID(unsigned char ucChip, unsigned int uiLen)
{
	unsigned int ui;
	SPI_REG_LOAD(SFCR, (SFCR_SPI_CLK_DIV(7) | SFCR_RBO(1) | SFCR_WBO(1) | SFCR_SPI_TCS(31)));		//SFCR default setting
	rstSPIFlash(ucChip);
	SFCSR_CS_L(ucChip, 0, IOWIDTH_SINGLE);
	SPI_REG_LOAD(SFDR, SPICMD_RDID);
	SFCSR_CS_L(ucChip, (uiLen - 1), IOWIDTH_SINGLE);
	ui = SPI_REG_READ(SFDR);
	SFCSR_CS_H(ucChip, 0, IOWIDTH_SINGLE);
	LDEBUG("ComSrlCmd_RDID: ucChip=%x; uiLen=%x; returnValue=%x; SPICMD_RDID=%x;\n", ucChip, uiLen, ui, SPICMD_RDID);
	return ui;
}
// One byte Command
void SeqCmd_Order(unsigned char ucChip,  unsigned char ucIOWidth, unsigned int uiCmd)
{
	LDEBUG("SeqCmd_Type1: ucChip=%x; ucIOWidth=%x; SPICMD=%x;\n", ucChip, ucIOWidth, uiCmd);
	SFCSR_CS_L(ucChip, ucIOWidth, IOWIDTH_SINGLE);
	SPI_REG_LOAD(SFDR, uiCmd);
	SFCSR_CS_H(ucChip, ucIOWidth, IOWIDTH_SINGLE);
}
// One byte Command Write
void SeqCmd_Write(unsigned char ucChip,  unsigned char ucIOWidth, unsigned int uiCmd, unsigned int uiValue, unsigned char ucValueLen)
{
	SFCSR_CS_L(ucChip, DATA_LENTH1, ucIOWidth);
	SPI_REG_LOAD(SFDR, uiCmd);
	SFCSR_CS_L(ucChip, ucValueLen - 1, ucIOWidth);
	SPI_REG_LOAD(SFDR, (uiValue << ((4 - ucValueLen) * 8)));
	SFCSR_CS_H(ucChip, DATA_LENTH1, IOWIDTH_SINGLE);
	LDEBUG("SeqCmd_Write: ucChip=%x; ucIOWidth=%x; uiCmd=%x; uiValue=%x; ucValueLen=%x;\n", ucChip, ucIOWidth, uiCmd, uiValue, ucValueLen);
}
// One byte Command Read
unsigned int SeqCmd_Read(unsigned char ucChip,  unsigned char ucIOWidth, unsigned int uiCmd, unsigned char ucRDLen)
{
	unsigned int ui;
	SFCSR_CS_L(ucChip, DATA_LENTH1, ucIOWidth);
	SPI_REG_LOAD(SFDR, uiCmd);
	SFCSR_CS_L(ucChip, ucRDLen-1, ucIOWidth);
	ui = SPI_REG_READ(SFDR);
	SFCSR_CS_H(ucChip, DATA_LENTH1, ucIOWidth);
	ui = ui >> ((4 - ucRDLen) * 8);
	LDEBUG("SeqCmd_Read: ucChip=%x; ucIOWidth=%x; uiCmd=%x; ucRDLen=%x; RetVal=%x\n", ucChip, ucIOWidth, uiCmd, ucRDLen, ui);
	return ui;
}

/****************************** Layer 2 ******************************/
// Sector Erase (SE) Sequence (Command 20)
unsigned int ComSrlCmd_SE(unsigned char ucChip, unsigned int uiAddr)
{
	int ret=0;
	SeqCmd_Order(ucChip,  IOWIDTH_SINGLE, SPICMD_WREN);

	#if defined(CONFIG_RTL_8198C)
	if(REG32(0xb8000008)&0x1)
		SeqCmd_Write(ucChip,  IOWIDTH_SINGLE, SPICMD_SE, uiAddr, 4);//JSW:For 4-byte addressing
	else
	#endif
		SeqCmd_Write(ucChip,  IOWIDTH_SINGLE, SPICMD_SE, uiAddr, 3);//JSW:For 3to4-byte addressing

	//KDEBUG("ComSrlCmd_SE: ucChip=%x; uiSector=%x; uiSectorSize=%x; SPICMD_SE=%x\n", ucChip, uiAddr, spi_flash_info[ucChip].sector_size, SPICMD_SE);
	KDEBUG("ComSrlCmd_SE: ucChip=%x; uiSector=%x; uiSectorSize=%x; SPICMD_SE=%x\n", ucChip, uiAddr, spi_flash_info[ucChip].sector_size, SPICMD_SE);
	ret= spiFlashReady(ucChip);
	return ret;
}
// Block Erase (BE) Sequence (Command D8)
unsigned int ComSrlCmd_BE(unsigned char ucChip, unsigned int uiAddr)
{
	int ret=0;
	SeqCmd_Order(ucChip,  IOWIDTH_SINGLE, SPICMD_WREN);

	#if defined(CONFIG_RTL_8198C)
	if(REG32(0xb8000008)&0x1)
		SeqCmd_Write(ucChip,  IOWIDTH_SINGLE, SPICMD_BE, uiAddr, 4);//JSW:For 4-byte addressing
	else
	#endif
		SeqCmd_Write(ucChip,  IOWIDTH_SINGLE, SPICMD_BE, uiAddr, 3);//JSW:For 3to4-byte addressing

	KDEBUG("ComSrlCmd_BE: ucChip=%x; uiBlock=%x; uiBlockSize=%x; SPICMD_BE=%x\n", ucChip, uiAddr, spi_flash_info[ucChip].block_size, SPICMD_BE);
	ret =spiFlashReady(ucChip);
	return ret;
}
// Chip Erase (CE) Sequence (Command 60 or C7)
unsigned int ComSrlCmd_CE(unsigned char ucChip)
{
	SeqCmd_Order(ucChip,  IOWIDTH_SINGLE, SPICMD_WREN);
	SeqCmd_Order(ucChip,  IOWIDTH_SINGLE, SPICMD_CE);
	KDEBUG("ComSrlCmd_CE: ucChip=%x; SPICMD_CE=%x\n", ucChip, SPICMD_CE);
	return spiFlashReady(ucChip);
}
// without QE bit
unsigned int ComSrlCmd_NoneQeBit(unsigned char ucChip)
{
	KDEBUG("ComSrlCmd_NoneQeBit: ucChip=%x;\n", ucChip);
	return 0;
}
// ucIsFast: = 0 cmd, address, dummy single IO ; =1 cmd single IO, address and dummy multi IO; =2 cmd, address and dummy multi IO;
void ComSrlCmd_InputCommand(unsigned char ucChip, unsigned int uiAddr, unsigned int uiCmd, unsigned char ucIsFast, unsigned char ucIOWidth, unsigned char ucDummyCount)
{
	int i;
#ifdef CONFIG_RTL_8198C
	int count=0;
	LDEBUG("ComSrlCmd_InputCommand: ucChip=%x; uiAddr=%x; uiCmd=%x; uiIsfast=%x; ucIOWidth=%x; ucDummyCount=%x\n", ucChip, uiAddr, uiCmd, ucIsFast, ucIOWidth, ucDummyCount);
	// input command
#if 0
	if(ucIsFast == ISFAST_ALL)
	{
		SFCSR_CS_L(ucChip, 0, ucIOWidth);
	}
	else
#endif
	{
		SFCSR_CS_L(ucChip, 0, IOWIDTH_SINGLE);
	}




	while (( ((*((volatile unsigned int *)SFCSR) & (1<<30)) == 0) ))
	{
		    #if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif
                count++;
                if(count>10000)
                {
                        count=0;
                        break;
                }

	}


	SPI_REG_LOAD(SFDR, uiCmd);				// Read Command

	// input 3 bytes address
	if(ucIsFast == ISFAST_NO)
	{
		SFCSR_CS_L(ucChip, 0, IOWIDTH_SINGLE);
	}
	else
	{
		SFCSR_CS_L(ucChip, 0, ucIOWidth);
	}

	if(REG32(0xb8000008)&0x1)
	{
		SPI_REG_LOAD(SFDR,(uiAddr << 0));//JSW for 4-Byte addressing for all read
	}

	SPI_REG_LOAD(SFDR,(uiAddr << 8));

	SPI_REG_LOAD(SFDR,(uiAddr << 16));

	SPI_REG_LOAD(SFDR,(uiAddr << 24));

        __flush_cache_all();
	//input dummy cycle
	for (i = 0; i < ucDummyCount; i++)
	{
		SPI_REG_LOAD(SFDR, 0);
	}
	if(ucIOWidth<=4096)
	{

	SFCSR_CS_L(ucChip, 3, ucIOWidth);
	// rtk_udelay(1);
	}
        rtk_udelay(500);
#else
	// input command
	if(ucIsFast == ISFAST_ALL)
	{
		SFCSR_CS_L(ucChip, 0, ucIOWidth);
	}
	else
	{
		SFCSR_CS_L(ucChip, 0, IOWIDTH_SINGLE);
	}
	SPI_REG_LOAD(SFDR, uiCmd);				// Read Command

	// input 3 bytes address
	if(ucIsFast == ISFAST_NO)
	{
		SFCSR_CS_L(ucChip, 0, IOWIDTH_SINGLE);
	}
	else
	{
		SFCSR_CS_L(ucChip, 0, ucIOWidth);
	}
	SPI_REG_LOAD(SFDR,(uiAddr << 8));
	SPI_REG_LOAD(SFDR,(uiAddr << 16));
	SPI_REG_LOAD(SFDR,(uiAddr << 24));

	//input dummy cycle
	for (i = 0; i < ucDummyCount; i++)
	{
		SPI_REG_LOAD(SFDR, 0);
	}

	SFCSR_CS_L(ucChip, 3, ucIOWidth);
#endif
}

// Set SFCR2 for memery map read
unsigned int SetSFCR2(unsigned int uiCmd, unsigned char ucIsFast, unsigned char ucIOWidth, unsigned char ucDummyCount)
{
	unsigned int ui, uiDy;
	ucSFCR2 = 0;
	ui = SFCR2_SFCMD(uiCmd) | SFCR2_SFSIZE(spi_flash_info[0].device_size - 17) | SFCR2_RD_OPT(0) | SFCR2_HOLD_TILL_SFDR2(0);
	switch (ucIsFast)
	{
		case ISFAST_NO:
		{
			ui = ui | SFCR2_CMD_IO(IOWIDTH_SINGLE) | SFCR2_ADDR_IO(IOWIDTH_SINGLE) | SFCR2_DATA_IO(ucIOWidth);
			uiDy = 1;
			break;
		}
		case ISFAST_YES:
		{
			ui = ui | SFCR2_CMD_IO(IOWIDTH_SINGLE) | SFCR2_ADDR_IO(ucIOWidth) | SFCR2_DATA_IO(ucIOWidth);
			uiDy = ucIOWidth * 2;
			break;
		}
		case ISFAST_ALL:
		{
			ui = ui | SFCR2_CMD_IO(ucIOWidth) | SFCR2_ADDR_IO(ucIOWidth) | SFCR2_DATA_IO(ucIOWidth);
			uiDy = ucIOWidth * 2;
			break;
		}
		default:
		{
			ui = ui | SFCR2_CMD_IO(IOWIDTH_SINGLE) | SFCR2_ADDR_IO(IOWIDTH_SINGLE) | SFCR2_DATA_IO(ucIOWidth);
			uiDy = 1;
			break;
		}
	}
	if (uiDy == 0)
	{
		uiDy = 1;
	}
	ui = ui | SFCR2_DUMMY_CYCLE((ucDummyCount * 4 / uiDy));		// ucDummyCount is Byte Count ucDummyCount*8 / (uiDy*2)
	SPI_REG_LOAD(SFCR2, ui);
	LDEBUG("SetSFCR2: uiCmd=%x; ucIsFast=%; ucIOWidth=%x; ucDummyCount=%x; ucSFCR2=%x; SFCR2=%x\n;", uiCmd, ucIsFast, ucIOWidth, ucDummyCount, ucSFCR2, ui);
	return ui;
}

// read template
unsigned int ComSrlCmd_ComRead(unsigned char ucChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer,unsigned int uiCmd, unsigned char ucIsFast, unsigned char ucIOWidth, unsigned char ucDummyCount)
{

	unsigned int ui, uiCount, i;
	unsigned char* puc = pucBuffer;
	LDEBUG("ComSrlCmd_ComRead: ucChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; uiCmd=%x; uiIsfast=%x; ucIOWidth=%x; ucDummyCount=%x\n", ucChip, uiAddr, uiLen, (unsigned int)pucBuffer, uiCmd, ucIsFast, ucIOWidth, ucDummyCount);
#ifdef CONFIG_RTL_8198C
        toggle_bit(F_1,0);
        toggle_bit(F_1,0);
        toggle_bit(F_1,0);
        preempt_disable();
#endif
	ComSrlCmd_InputCommand(ucChip, uiAddr, uiCmd, ucIsFast, ucIOWidth, ucDummyCount);
	if(ucSFCR2 != 0)	// set SFCR2
	{
		ui = SetSFCR2((uiCmd >> 24), ucIsFast, ucIOWidth, ucDummyCount);
	}

	uiCount = uiLen / 4;
	for( i = 0; i< uiCount; i++)					// Read 4 bytes every time.
	{
		ui = SPI_REG_READ(SFDR);
		memcpy(puc, &ui, 4);
		puc += 4;
	}

	i = uiLen % 4;
	if(i > 0)
	{
		ui = SPI_REG_READ(SFDR);					// another bytes.
		memcpy(puc, &ui, i);
		puc += i;
	}
	SFCSR_CS_H(ucChip, 0, IOWIDTH_SINGLE);
#ifdef CONFIG_RTL_8198C
	 #if 1
                 __asm__ __volatile__(  \
                ".set   push\n\t"       \
                ".set   noreorder\n\t"  \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                "nop\n\t"               \
                ".set   pop\n\t");
            #endif
            preempt_enable();
            toggle_bit(F_1,1);
            toggle_bit(F_1,0);
            toggle_bit(F_1,0);
#endif
	return uiLen;

}

// write template
unsigned int ComSrlCmd_ComWrite(unsigned char ucChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer, unsigned int uiCmd, unsigned char ucIsFast, unsigned char ucIOWidth, unsigned char ucDummyCount)
{
	unsigned int ui, uiCount, i;
	unsigned char* puc = pucBuffer;
	LDEBUG("ComSrlCmd_ComWrite: ucChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; uiCmd=%x; uiIsfast=%x; ucIOWidth=%x; ucDummyCount=%x\n", ucChip, uiAddr, uiLen, (unsigned int)pucBuffer, uiCmd, ucIsFast, ucIOWidth, ucDummyCount);
#ifdef CONFIG_RTL_8198C
        toggle_bit(F_1,0);
        preempt_disable();
#endif
	SeqCmd_Order(ucChip,  IOWIDTH_SINGLE, SPICMD_WREN);

	ComSrlCmd_InputCommand(ucChip, uiAddr, uiCmd, ucIsFast, ucIOWidth, ucDummyCount);
	uiCount = uiLen / 4;
	for (i = 0; i <  uiCount; i++)
	{
		memcpy(&ui, puc, 4);
		puc += 4;
		SPI_REG_LOAD(SFDR, ui);
	}

	i = uiLen % 4;
	if(i > 0)
	{
		memcpy(&ui, puc, i);
		puc += i;
		SFCSR_CS_L(ucChip, i-1, ucIOWidth);
		SPI_REG_LOAD(SFDR, ui);
	}
#ifdef CONFIG_RTL_8198C
		__flush_cache_all();
#endif
	SFCSR_CS_H(ucChip, 0, IOWIDTH_SINGLE);
	ui = spiFlashReady(ucChip);
#ifdef CONFIG_RTL_8198C
       preempt_enable();
       toggle_bit(F_1,1);
#endif
	return uiLen;
}

// write a whole sector once
unsigned int ComSrlCmd_ComWriteSector(unsigned char ucChip, unsigned int uiAddr, unsigned char* pucBuffer)
{
	unsigned int i, ui;
	unsigned char* puc = pucBuffer;
	LDEBUG("ComSrlCmd_ComWriteSector: ucChip=%x; uiAddr=%x; pucBuffer=%x; returnValue=%x;\n", ucChip, uiAddr, (unsigned int)pucBuffer, spi_flash_info[ucChip].sector_size);
	//prnDispAddr(uiAddr);
	NDEBUG(".");
	ui = spi_flash_info[ucChip].pfErase(ucChip, uiAddr);
	for (i = 0; i < spi_flash_info[ucChip].page_cnt; i++)
	{
		ui = spi_flash_info[ucChip].pfPageWrite(ucChip, uiAddr, spi_flash_info[ucChip].page_size, puc);
		uiAddr += spi_flash_info[ucChip].page_size;
		puc += spi_flash_info[ucChip].page_size;
	}
	return spi_flash_info[ucChip].sector_size;
}

// write sector use malloc buffer
unsigned int ComSrlCmd_BufWriteSector(unsigned char ucChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	unsigned char pucSector[spi_flash_info[ucChip].sector_size];
	unsigned int ui, uiStartAddr, uiOffset;
	LDEBUG("ComSrlCmd_BufWriteSector:ucChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x;\n", ucChip, uiAddr, uiLen, pucBuffer);
	uiOffset = uiAddr % spi_flash_info[ucChip].sector_size;
	uiStartAddr = uiAddr - uiOffset;
	// get
	ui = spi_flash_info[ucChip].pfRead(ucChip, uiStartAddr, spi_flash_info[ucChip].sector_size, pucSector);
	// modify
	memcpy(pucSector + uiOffset, pucBuffer, uiLen);
	//write back
	ui = ComSrlCmd_ComWriteSector(ucChip, uiStartAddr, pucSector);
	return ui;
}

// write data, any address any lenth
unsigned int ComSrlCmd_ComWriteData(unsigned char ucChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	unsigned int uiStartAddr, uiStartLen, uiSectorAddr, uiSectorCount, uiEndAddr, uiEndLen, i;
	unsigned char* puc = pucBuffer;
	LDEBUG("ComSrlCmd_ComWriteData:ucChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x\n", ucChip, uiAddr, uiLen, (unsigned int)pucBuffer);
	calAddr(uiAddr, uiLen, spi_flash_info[ucChip].sector_size, &uiStartAddr, &uiStartLen, &uiSectorAddr, &uiSectorCount, &uiEndAddr, &uiEndLen);
	if((uiSectorCount == 0x00) && (uiEndLen == 0x00))	// all data in the same sector
	{
		ComSrlCmd_BufWriteSector(ucChip, uiStartAddr, uiStartLen, puc);
	}
	else
	{
		if(uiStartLen > 0)
		{
			ComSrlCmd_BufWriteSector(ucChip, uiStartAddr, uiStartLen, puc);
			puc += uiStartLen;
		}
		for(i = 0; i < uiSectorCount; i++)
		{
			ComSrlCmd_ComWriteSector(ucChip, uiSectorAddr, puc);
			puc += spi_flash_info[ucChip].sector_size;
			uiSectorAddr += spi_flash_info[ucChip].sector_size;
		}
		if(uiEndLen > 0)
		{
			ComSrlCmd_BufWriteSector(ucChip, uiEndAddr, uiEndLen, puc);
		}
	}
	SeqCmd_Order(ucChip,  IOWIDTH_SINGLE, SPICMD_WRDI);
	return uiLen;
}

/****************************** Macronix ******************************/
// MX25L1605 MX25L3205 Read at High Speed (FAST_READ) Sequence (Command 0B)
unsigned int  SpiRead_11110B(unsigned char ucChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG(" SpiRead_11110B: ucChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_FASTREAD=%x;\n", ucChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_FASTREAD);
	return ComSrlCmd_ComRead(ucChip, uiAddr, uiLen, pucBuffer, SPICMD_FASTREAD, ISFAST_NO, IOWIDTH_SINGLE, DUMMYCOUNT_1);
}
// Page Program (PP) Sequence (Command 02)
unsigned int  PageWrite_111002(unsigned char ucChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG(" PageWrite_111002: ucChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_PP=%x;\n", ucChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_PP);
	return ComSrlCmd_ComWrite(ucChip, uiAddr, uiLen, pucBuffer, SPICMD_PP, ISFAST_NO, IOWIDTH_SINGLE, DUMMYCOUNT_0);
}
#if (SPI_DRIVER_MODE == 1)
// Set quad enable bit
unsigned int mxic_spi_setQEBit(unsigned char ucChip)
{
	unsigned int ui;
	SeqCmd_Order(ucChip,  IOWIDTH_SINGLE, SPICMD_WREN);
	ui = 1 << SPI_STATUS_QE;
	SeqCmd_Write(ucChip, IOWIDTH_SINGLE, SPICMD_WRSR, ui, 1);
	KDEBUG("MxicSetQEBit: ucChip=%d; statusRegister=%x; returnValue=%x\n", ucChip, SeqCmd_Read(ucChip, IOWIDTH_SINGLE, SPICMD_RDSR, 1), ui);
	return ui;
}
// MX25L1605 MX25L3205 Read at Dual IO Mode Sequence (Command BB)
unsigned int  SpiRead_1221BB(unsigned char ucChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG(" SpiRead_1221BB: ucChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_2READ=%x;\n", ucChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_2READ);
	return ComSrlCmd_ComRead(ucChip, uiAddr, uiLen, pucBuffer, SPICMD_2READ, ISFAST_YES, IOWIDTH_DUAL, DUMMYCOUNT_1);
}
// MX25L1635 MX25L3235 4 x I/O Read Mode Sequence (Command EB)
unsigned int  SpiRead_1443EB(unsigned char ucChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG(" SpiRead_1443EB: ucChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_4READ=%x;\n", ucChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_4READ);
	return ComSrlCmd_ComRead(ucChip, uiAddr, uiLen, pucBuffer, SPICMD_4READ, ISFAST_YES, IOWIDTH_QUAD, DUMMYCOUNT_3);
}
// 4 x I/O Page Program (4PP) Sequence (Command 38)
unsigned int  PageWrite_144038(unsigned char ucChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG(" PageWrite_144038: ucChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_4PP=%x;\n", ucChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_4PP);
	return ComSrlCmd_ComWrite(ucChip, uiAddr, uiLen, pucBuffer, SPICMD_4PP, ISFAST_YES, IOWIDTH_QUAD, DUMMYCOUNT_0);
}
#endif

/****************************** SST ******************************/
// Layer1 SST Byte-Program
void SstComSrlCmd_BP(unsigned char ucChip, unsigned int uiAddr, unsigned char ucValue)
{
	unsigned int ui;
	ui = SPICMD_PP | (uiAddr & 0x00ffffff);
	SFCSR_CS_L(ucChip, 3, 0);
	SPI_REG_LOAD(SFDR, ui);
	SFCSR_CS_L(ucChip, 0, IOWIDTH_SINGLE);
	SPI_REG_LOAD(SFDR, (ucValue<< 24));
	SFCSR_CS_H(ucChip, 0, IOWIDTH_SINGLE);
	LDEBUG("SstComSrlCmd_BP: ucChip=%x; uiAddr=%x; ucValue=%x; SPICMD_SST_BP=%x;\n", ucChip, uiAddr, ucValue, SPICMD_PP);
}
// Layer2 Sector Write Use BP Mode
unsigned int sst_PageWrite_s1(unsigned char ucChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	unsigned int i, ui;
	unsigned char* puc = pucBuffer;
	KDEBUG("sst_cmd_write_s1: ucChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; returnValue=%x;\n", ucChip, uiAddr, uiLen, (unsigned int)pucBuffer, uiLen);
	for (i = 0; i < uiLen; i++)
	{
		SeqCmd_Order(ucChip,  IOWIDTH_SINGLE, SPICMD_WREN);
		SstComSrlCmd_BP(ucChip, uiAddr, *puc);
		ui = spiFlashReady(ucChip);
		puc += 1;
		uiAddr = uiAddr + 1;
	}
	return uiLen;
}

/****************************** Spansion ******************************/
// Layer2 Spansion Set QE bit
#if (SPI_DRIVER_MODE == 1)
unsigned int span_spi_setQEBit(unsigned char ucChip)
{
	unsigned int ui;
	SeqCmd_Order(ucChip,  IOWIDTH_SINGLE, SPICMD_WREN);
	ui = 1 << SPAN_CONF_QUAD;
	SeqCmd_Write(ucChip, IOWIDTH_SINGLE, SPICMD_WRSR, ui, 2);
	KDEBUG("SpanSetQEBit: ucChip=%d; statusRegister=%x; returnValue=%x\n", ucChip, SeqCmd_Read(ucChip, IOWIDTH_SINGLE, SPICMD_RDSR, 2), ui);
	return spiFlashReady(ucChip);
}
#endif

/****************************** Winbond ******************************/
// Layer3 Winbond Set QE Bit
#if (SPI_DRIVER_MODE == 1)
unsigned int wb_spi_setQEBit(unsigned char ucChip)
{
	unsigned int ui;
	SeqCmd_Order(ucChip,  IOWIDTH_SINGLE, SPICMD_WREN);
	ui = 1 << (WB_STATUS_QE - 8);
	SeqCmd_Write(ucChip, IOWIDTH_SINGLE, SPICMD_WRSR, ui, 2);
	KDEBUG("WBSetQEBit: ucChip=%d; statusRegister=%x; returnValue=%x\n", ucChip, SeqCmd_Read(ucChip, IOWIDTH_SINGLE, SPICMD_RDSR, 1), ui);
	return spiFlashReady(ucChip);
}
// Quad Program Page Program (Command 32)
unsigned int PageWrite_114032(unsigned char ucChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG("PageWrite_114032: ucChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_WB_QPP=%x;\n", ucChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_WB_QPP);
	return ComSrlCmd_ComWrite(ucChip, uiAddr, uiLen, pucBuffer, SPICMD_WB_QPP, ISFAST_NO, IOWIDTH_QUAD, DUMMYCOUNT_0);
}
#endif

/****************************** Giga Device ******************************/
// Set quad enable bit
#if (SPI_DRIVER_MODE == 1)
unsigned int gd_spi_setQEBit(unsigned char ucChip)
{
	unsigned int ui;
	SeqCmd_Order(ucChip,  IOWIDTH_SINGLE, SPICMD_WREN);
	ui = 1 << (GD_STATUS_QE - 8);
	SeqCmd_Write(ucChip, IOWIDTH_SINGLE, SPICMD_WRSR, ui, 2);	// set Giga Devcie QE bit
	KDEBUG("gd_spi_setQEBit: ucChip=%d; statusRegister=%x; returnValue=%x\n", ucChip, SeqCmd_Read(ucChip, IOWIDTH_SINGLE, SPICMD_RDSR, 2));
	GdCmdSrl_HPM(ucChip);
	return ui;
}
// High Performance Mode (HPM) (A3H)
unsigned int GdCmdSrl_HPM(unsigned char ucChip)
{
	SeqCmd_Write(ucChip,  IOWIDTH_SINGLE, SPICMD_GD_HPM, 0x00, 3);	// command add 3 dummy
	KDEBUG("GdCmdSrl_HPM: ucChip=%d; SPICMD_GD_HPM=%x; \n", ucChip, SPICMD_GD_HPM);
	return spiFlashReady(ucChip);
}
#endif

/****************************** ATMEL ******************************/
// AT25DF161 Dual-Output Read Array(Command 3B)
#if (SPI_DRIVER_MODE == 1)
unsigned int SpiRead_11213B(unsigned char ucChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG("SpiRead_11213B: ucChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_AT_READ2=%x;\n", ucChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_AT_READ2);
	return ComSrlCmd_ComRead(ucChip, uiAddr, uiLen, pucBuffer, SPICMD_AT_READ2, ISFAST_NO, IOWIDTH_DUAL, DUMMYCOUNT_1);
}
// AT25DF161 Dual-Input Byte/Page Program(Command A2)
unsigned int PageWrite_1120A2(unsigned char ucChip, unsigned int uiAddr, unsigned int uiLen, unsigned char* pucBuffer)
{
	KDEBUG("PageWrite_1120A2: ucChip=%x; uiAddr=%x; uiLen=%x; pucBuffer=%x; SPICMD_AT_PP2=%x;\n", ucChip, uiAddr, uiLen, (unsigned int)pucBuffer, SPICMD_AT_PP2);
	return ComSrlCmd_ComWrite(ucChip, uiAddr, uiLen, pucBuffer, SPICMD_AT_PP2, ISFAST_NO, IOWIDTH_DUAL, DUMMYCOUNT_0);
}
#endif

#if (MTD_SPI_TEST_CHIP == 1)
unsigned int test_spi_flash(unsigned char ucChip)
{
	unsigned char pucSrc[0x100] = {0};
	unsigned char pucDst[0x100] = {0};
	unsigned int uiOffset = 0x10000;
	unsigned int uiAddr = 0x10000;
	unsigned int uiCount = spi_flash_info[ucChip].sector_cnt - (uiOffset) / spi_flash_info[ucChip].sector_size;
	unsigned int uiRet = 0;
	unsigned int uiLine = 0;
	unsigned int uiStep = 0;
	int i = 0, j = 0;
	if(uiCount > 0x1f0)
	{
		uiStep = uiCount / 0x1f0;
	}
	else
	{
		uiStep = 1;
	}
	for (i = 0; i< 0x100; i++) pucSrc[i] = i;
	NDEBUG("\nTest SPI flash: offset->0x%x, step->0x%x\n", (uiOffset - 0x80), (spi_flash_info[ucChip].sector_size * uiStep));
	uiCount = 1;	// Delete
	for (j = 0; j < uiCount; j += uiStep)
	{
		uiAddr = uiOffset + j * spi_flash_info[ucChip].sector_size - 0x80;
		spi_flash_info[ucChip].pfWrite(ucChip, uiAddr, 0x100, pucSrc);
		memset(pucDst, 0, 0x100);
		spi_flash_info[ucChip].pfRead(ucChip, uiAddr, 0x100, pucDst);
		for( i = 0; i < 0x100; i++)
		{
			if(pucDst[i] != pucSrc[i])
			{
				NDEBUG("%07x-ER", uiAddr);
				uiRet += 1;
				break;
			}
		}
		if(i == 0x100) NDEBUG("%07x-OK", uiAddr);
		uiLine++;
		if(uiLine == 6)
		{
			NDEBUG("\n");
			uiLine = 0;
		}
	}
	NDEBUG("\nTotal Count = 0x%x; OK Count = 0x%x; Error Count = 0x%x\n", uiCount, (uiCount - uiRet), uiRet);
	return uiRet;
}
#endif
