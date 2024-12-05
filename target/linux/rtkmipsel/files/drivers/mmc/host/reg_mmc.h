/*
 *  Copyright (C) 2013 Realtek Semiconductors, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __REG_MMC_H
#define __REG_MMC_H


//#define CR_BASE_ADDR            0x18010400
//#define EM_BASE_ADDR            0x18012000
#define CR_BASE_ADDR            0x1801097F //Afooo, fake addr for 97F emmc
#define EM_BASE_ADDR            0x18015400
//#define SDIO_BASE_ADDR          0x18010a00
#define CR_DUMMY_SYS            0x1801042c

/* ===== Realtek Card Reader Register Sets ===== */
//System bridge2 (base = 0x1801A000)
#define CR_SYNC                 	( 0x20	)

//sd pll (base = 0x18000000)
#define CR_SOFT_RESET2                (0x0004)
//#define CR_PLL_SD1                    (0x01E0)
//#define CR_PLL_SD2                    (0x01E4)
//#define CR_PLL_SD3                    (0x01E8)
//#define CR_PLL_SD4                    (0x01EC)
#define CR_PFUNC_CR                   (0x0380)

//97F
#define SYSREG_BASE_ADDR                 0xb8000000
#define EMMC_BASE_ADDR			0xb8015400
#define SYS_REG_BASE			SYSREG_BASE_ADDR //temp for DHC
#define REG_CLK_MANAGE                 ( SYSREG_BASE_ADDR + 0x10 )
#define REG_CLK_MANAGE2                 ( SYSREG_BASE_ADDR + 0x14 )
#define REG_ENABLE_IP                 ( SYSREG_BASE_ADDR + 0x50 )

#define CR_PLL_SD1                    (0x01C0) //97F REG_SD30_1
#define CR_PLL_SD2                    (0x01C4) //97F REG_SD30_2
#define CR_PLL_SD3                    (0x01C8) //97F REG_SD30_3
#define CR_PLL_SD4                    (0x01CC) //97F REG_SD30_4 
#define SYS_PLL_EMMC3           (SYSREG_BASE_ADDR + CR_PLL_SD3)

//sd wrapper (base = 0x18010400)
#define CR_SRAM_CTL                 ( 0x0   )
#define CR_DMA_CTL1                 ( 0x004 )
#define CR_DMA_CTL2                 ( 0x008 )
#define CR_DMA_CTL3                 ( 0x00C )
#define CR_SD_ISR                   ( 0x024 )
#define CR_SD_ISREN                 ( 0x028 )
#define CR_SD_PAD_CTL           	( 0x074 )
#define CR_SD_CKGEN_CTL             ( 0x078 )
#define CR_IP_CARD_INT_EN           ( 0x120 )
//emmc wrapper (base = 0x18012000)
#define EMMC_SRAM_CTL               ( 0x0   )
#define EMMC_DMA_CTL1               ( 0x004 )
#define EMMC_DMA_CTL2               ( 0x008 )
#define EMMC_DMA_CTL3               ( 0x00C )
#define EMMC_SD_ISR                 ( 0x024 )
#define EMMC_SD_ISREN               ( 0x028 )
#define EMMC_PAD_CTL                ( 0x074 )
#define EMMC_CKGEN_CTL              ( 0x078 )
#define EMMC_CPU_ACC                ( 0x080 )
#define EMMC_IP_CARD_INT_EN         ( 0x120 )
#define EMMC_CARD_PAD_DRV           ( 0x130 )
#define EMMC_CMD_PAD_DRV            ( 0x131 )
#define EMMC_DATA_PAD_DRV           ( 0x132 )
#define EMMC_SRAM_BASE_0            ( 0x200 )
#define EMMC_SRAM_BASE_1            ( 0x300 )
//sdio wrapper (base = 0x18010a00)
#define SDIO_SRAM_CTL               ( 0x0   )
#define SDIO_IP_CTL                 ( 0x010 )
#define SDIO_SD_ISR                 ( 0x030 )
#define SDIO_SD_ISREN               ( 0x034 )
//sdio host standard (base = 0x18010c00)
#define SDIO_NORML_INT_STA          ( 0x030 )

//sd & emmc
#define CR_CARD_STOP                ( 0x103 )
#define CR_CARD_OE                	( 0x104 )
#define CARD_SELECT                 ( 0x10e )
#define CARD_EXIST                 	( 0x11f )
#define CARD_INT_PEND	          	( 0x121 )
#define CARD_CLOCK_EN_CTL          	( 0x129 )
#define CARD_SD_CLK_PAD_DRIVE      	( 0x130 )
#define CARD_SD_CMD_PAD_DRIVE      	( 0x131 )
#define CARD_SD_DAT_PAD_DRIVE      	( 0x132 )
#define SD_CONFIGURE1               ( 0x180 )
#define SD_CONFIGURE2               ( 0x181 )
#define SD_CONFIGURE3               ( 0x182 )
#define SD_STATUS1                  ( 0x183 )
#define SD_STATUS2                  ( 0x184 )
#define SD_BUS_STATUS               ( 0x185 )
#define SD_CMD_MODE                 ( 0x186 )
#define SD_SAMPLE_POINT_CTL         ( 0x187 )
#define SD_PUSH_POINT_CTL           ( 0x188 )
#define SD_CMD0                     ( 0x189 )
#define SD_CMD1                     ( 0x18A )
#define SD_CMD2                     ( 0x18B )
#define SD_CMD3                     ( 0x18C )
#define SD_CMD4                     ( 0x18D )
#define SD_CMD5                     ( 0x18E )
#define SD_BYTE_CNT_L               ( 0x18F )
#define SD_BYTE_CNT_H               ( 0x190 )
#define SD_BLOCK_CNT_L              ( 0x191 )
#define SD_BLOCK_CNT_H              ( 0x192 )
#define SD_TRANSFER                 ( 0x193 )
#define SD_DDR_DETECT_START         ( 0x194 )
#define SD_CMD_STATE                ( 0x195 )
#define SD_DATA_STATE               ( 0x196 )
#define SD_BUS_TA_STATE             ( 0x197 )
#define SD_STOP_SDCLK_CFG           ( 0x198 )
#define SD_AUTO_RST_FIFO            ( 0x199 )
#define SD_DAT_PAD                  ( 0x19A )
#define SD_DUMMY_4                  ( 0x19B )
#define SD_DUMMY_5                  ( 0x19C )
#define SD_DUTY_CTL                 ( 0x19D )
#define SD_SEQ_RW_CTL               ( 0x19E )
#define SD_CONFIGURE4               ( 0x19F )
#define SD_ADDR_L                   ( 0x1A0 )
#define SD_ADDR_H                   ( 0x1A1 )
#define SD_START_ADDR_0             ( 0x1A2 )
#define SD_START_ADDR_1             ( 0x1A3 )
#define SD_START_ADDR_2             ( 0x1A4 )
#define SD_START_ADDR_3             ( 0x1A5 )
#define SD_RSP_MASK_1               ( 0x1A6 )
#define SD_RSP_MASK_2               ( 0x1A7 )
#define SD_RSP_MASK_3               ( 0x1A8 )
#define SD_RSP_MASK_4               ( 0x1A9 )
#define SD_RSP_DATA_1               ( 0x1AA )
#define SD_RSP_DATA_2               ( 0x1AB )
#define SD_RSP_DATA_3               ( 0x1AC )
#define SD_RSP_DATA_4               ( 0x1AD )
#define SD_WRITE_DELAY              ( 0x1AE )
#define SD_READ_DELAY               ( 0x1AF )

#define CR_CARD_RESP6_0         SD_CMD0         //0x18012189
#define CR_CARD_RESP6_1         SD_CMD1         //0x1801218a
#define CR_CARD_RESP6_2         SD_CMD2         //0x1801218b
#define CR_CARD_RESP6_3         SD_CMD3         //0x1801218c
#define CR_CARD_RESP6_4         SD_CMD4         //0x1801218d
#define CR_CARD_RESP6_5         SD_CMD5         //0x1801218e

/* register item define */
//for emmc pad 1.8v
//#define PLL_EMMC1		0x180001F0
#define PLL_EMMC1		0x180001C0
#define CR_PAD_CTL		(EM_BASE_ADDR+EMMC_PAD_CTL)

/* CR_DMA_CTL3     0x18010503 */
/* EMMC_DMA_CTL3 0x18012003 */

#define RESP_LEN64                (0x1)
#define RESP_LEN17                (0x2)
#define DMA_XFER                (0x00000001)
#define DDR_WR                  (0x00000001<<1)
#define RSP17_SEL               (0x00000001<<4)
#define RSP64_SEL               (0x00000001<<5)
#define DAT64_SEL               (0x00000001<<5)

/*CARD_EXIST 0x1801051f */
#define SD_WRITE_PROTECT		(0x00000001<<5)
#define XD_EXISTENCE			(0x00000001<<4)
#define MS_EXISTENCE			(0x00000001<<3)
#define SD_EXISTENCE			(0x00000001<<2)
#define SM_EXISTENCE			(0x00000001<<1)


/* SD_CONFIGURE1 0x18012180 */
#define SDCLK_DIV                   (0x00000001<<7)
#define SDCLK_NO_DIV                (0x00000000)
#define SDCLK_DIV_128               (0x00000000)
#define SDCLK_DIV_256               (0x00000001<<6)
#define NO_RST_RDWR_FIFO            (0x00000000)
#define RST_RDWR_FIFO               (0x00000001<<4)

#define MASK_CLOCK_MODE         (0x00000003<<2)
#define MASK_CLOCK_DIV          (0x00000003<<6)
#define CLOCK_DIV_NON           (0x00000002)
#define CLOCK_DIV_256           (0x00000001<<6)
#define CLOCK_DIV_128           (0x00000000)
#define EMMC_CLOCK_DIV_NON      (0x00000000)
#define EMMC_CLOCK_DIV_256      (0x00000003<<6)
#define EMMC_CLOCK_DIV_128      (0x00000002<<6)

#define SPEED_MOD_HIGH          (0x00000001<<4)
#define SPEED_MOD_NORM          (0x00000000)
#define DELAY_PHA_MASK          (0x00000003<<2)
#define DELAY_PHA_NORM          (0x00000001<<3)
#define DELAY_PHA_HIGH          (0x00000001<<2)
#define SD30_ASYNC_FIFO_RST		(0x00000001<<4)
#define ACCESS_MODE_SD20       	(0x00000000)
#define ACCESS_MODE_DDR        	(0x00000001)
#define ACCESS_MODE_SD30       	(0x00000002)
#define MODE_SEL_MASK          	(0x00000003<<2)
#define MODE_SEL_SD20          	(ACCESS_MODE_SD20<<2)
#define MODE_SEL_DDR          	(ACCESS_MODE_DDR<<2)
#define MODE_SEL_SD30          	(ACCESS_MODE_SD30<<2)

#define MASK_MODE_SELECT        (0x0c)
#define MODE_SD20               (0x00) //sdr20/50
#define MODE_DDR                (0x01<<2) //ddr50
#define MODE_SD30               (0x02<<2) //hs-200

#define MASK_BUS_WIDTH          (0x00000003)
#define BUS_WIDTH_1             (0x00000000)
#define BUS_WIDTH_4             (0x00000001)
#define BUS_WIDTH_8             (0x00000002)
#define DDR_BUS_WIDTH_4         (0x00000005)      /* Card is in 4 bit DDR mode */
#define DDR_BUS_WIDTH_8         (0x00000006)      /* Card is in 8 bit DDR mode */

/* CPU_ACC_CTL 0x18012080 */
#define CPU_MODE_EN             (0x00000001)
#define BUF_FULL		(0x00000001<<2)

/* SD_CONFIGURE2 0x18012181 */
#define CRC7_CAL_DIS            (0x00000001<<7)
#define CRC16_CAL_DIS           (0x00000001<<6)
#define IGN_WR_CRC_ERR_EN	(0x00000001<<4)
#define WAIT_BUSY_EN            (0x00000001<<3)
#define CRC7_CHK_DIS            (0x00000001<<2)

#define MASK_RESP_TYPE          (0x00000003)
#define RESP_TYPE_NON           (0x00000000)
#define RESP_TYPE_6B            (0x00000001)
#define RESP_TYPE_17B           (0x00000002)

/* SD_CONFIGURE3 0x18012182 */
#define DATA_TRANS_OVER_EN      (0x00000001<<3)
#define RESP_CHK_EN             (0x00000001<<2)
#define ADDR_BYTE_MODE          (0x00000001<<1)
#define SD_CMD_RSP_TO           (0x00000001<<0)

/* SD_TRANSFER 0x18012193 */
#define START_EN                (0x00000001<<7)
#define END_STATE               (0x00000001<<6)
#define IDLE_STATE              (0x00000001<<5)
#define ERR_STATUS              (0x00000001<<4)

#define MASK_CMD_CODE           (0x0F)
#define SD_NORMALWRITE          (0x00)
#define SD_AUTOWRITE3           (0x01)
#define SD_AUTOWRITE4           (0x02)
#define SD_AUTOREAD3            (0x05)
#define SD_AUTOREAD4            (0x06)
#define SD_SENDCMDGETRSP        (0x08)
#define SD_AUTOWRITE1           (0x09)
#define SD_AUTOWRITE2           (0x0A)
#define SD_NORMALREAD           (0x0C)
#define SD_AUTOREAD1            (0x0D)
#define SD_AUTOREAD2            (0x0E)
#define SD_CMD_UNKNOW           (0x0F)

#define EMMC_MASK_CMD_CODE        (0xFF)
#define EMMC_NORMALWRITE          (0x00)
#define EMMC_AUTOWRITE3           (0x01)
#define EMMC_AUTOWRITE4           (0x02)
#define EMMC_AUTOREAD3            (0x05)
#define EMMC_AUTOREAD4            (0x06)
#define EMMC_SENDCMDGETRSP        (0x08)
#define EMMC_AUTOWRITE1           (0x09)
#define EMMC_AUTOWRITE2           (0x0A)
#define EMMC_NORMALREAD           (0x0C)
#define EMMC_AUTOREAD1            (0x0D)
#define EMMC_AUTOREAD2            (0x0E)
#define EMMC_TUNING               (0x0F)
#define EMMC_CMD_UNKNOW           (0x10)

/* SD_CONFIGURE2 0x18012181 */
#define CRC7_STATUS             (0x00000001<<7)
#define CRC16_STATUS            (0x00000001<<6)
#define WRT_ERR_BIT             (0x00000001<<5)
#define CRC_TIMEOUT_ERR         (0x00000001<<1)
#define PATTERN_CMP_ERR         (0x00000001<<0)
#define WRITE_CRC_ERR_STAT(x)   ((x & (0x7<<2))>>2)
/* SD_STATUS2 0x18012184 */
#define SD_CMD_RESP_INVALID     (0x00000001<<1)
#define SD_CMD_RESP_TIMEOUT     (0x00000001)
/* SD_BUS_STATUS 0x18012185 */
#define SD_CLK_TOG_EN     		(0x00000001<<7)
#define SD_CLK_TOG_STOP     	(0x00000001<<6)
#define SD_DAT3_0_LEVEL     	(0x0000000F<<1)
#define SD_CMD_LEVEL     		(0x00000001<<0)


#define WRITE_DATA              (0x1)
#define CLR_WRITE_DATA          (0x0)
/* CR_IP_CARD_INT_EN 0x18010520 */
/* EMMC_IP_CARD_INT_EN 0x18012120 */
#define SD_EMMC_INT_EN          (0x00000001<<2)

/* CR_SD_ISR 0x18010424 */
/* EMMC_SD_ISR 0x18012024 */
#define ISRSTA_INT1             (0x00000001<<1) //card end
#define ISRSTA_INT2             (0x00000001<<2) //card error
#define ISRSTA_INT4             (0x00000001<<4) //DMA done
/* SDIO_SD_ISR 0x18010a30 */
#define SDIO_ISRSTA_INT1        (0x00000001<<1) //dma done
#define SDIO_ISRSTA_INT2        (0x00000001<<2) //pre-transfer error
#define SDIO_ISRSTA_INT3        (0x00000001<<3) //pcie crc if error
#define SDIO_ISRSTA_INT4        (0x00000001<<4) //ip int

/* CR_SD_ISREN 0x18010428 */
/* EMMC_SD_ISREN 0x18012028 */
#define ISRSTA_INT1EN           (0x00000001<<1) //card end
#define ISRSTA_INT2EN           (0x00000001<<2) //card error
#define ISRSTA_INT4EN           (0x00000001<<4) //DMA done
/* SDIO_SD_ISREN 0x18010a34 */
#define SDIO_ISRSTA_INT1EN      (0x00000001<<1) //sdio int1 enable

#endif
