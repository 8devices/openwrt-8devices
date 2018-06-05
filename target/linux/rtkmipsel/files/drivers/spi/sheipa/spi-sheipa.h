/*
 * SHEIPA SPI controller driver
 *
 * Author: Realtek PSP Group
 *
 * Copyright 2015, Realtek Semiconductor Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _SPI_SHEIPA_H
#define _SPI_SHEIPA_H

#include <linux/io.h>
#include <linux/scatterlist.h>
#include "dw_common.h"

/* system endian */
#define BIG_ENDIAN 0
/* FLASH base address for auto mode */
#define	FIFO_SIZE	64
#define	FIFO_HALF_SIZE	(FIFO_SIZE / 2)

/* Soc tunning dummy cycle only. */
#define DEF_RD_TUNING_DUMMY_CYCLE  0x0
#define DEF_WR_BLOCK_BOUND         256

/* General flash opcode. */
#define WRSR		0x01
#define PP		0x02
#define NORM_READ	0x03
#define WRDI		0x04	/* write disable */
#define RDSR		0x05
#define WREN       	0x06
#define FAST_READ       0x0b
#define RDID		0x9f
#define CE		0xc7	/* chip erase */
#define BE_4K		0x20	/* erase 4KiB Block */
#define SE		0xd8	/* sector erase(usually 64KiB) */

/* Support auto mode flash dummy and type info only. */
#define DEF_RD_DUAL_TYPE           RD_DUAL_IO
#define DEF_RD_QUAD_TYPE           RD_QUAD_IO
#define DEF_WR_DUAL_TYPE           WR_MULTI_NONE
#define DEF_WR_QUAD_TYPE           WR_QUAD_II
#define DEF_RD_DUAL_DUMMY_CYCLE    0x4
#define DEF_RD_QUAD_DUMMY_CYCLE    0x6
#define DEF_RD_FAST_DUMMY_CYCLE    0x8

/* Support auto mode flash opcode only. */
#define PPX2_I		0x02
#define PPX2_II		0x02
#define PPX4_I		0x02
#define	PPX4_II		0x38
#define READX2_I	0x03
#define READX2_IO      	0xbb	/* data and addr channel */
#define READX4_I	0x03
#define READX4_IO	0xeb

/* support auto mode */
#define AUTO_MODE	0xf5

/* Support address 4 byte opcode for large size flash */
#define EN4B		0xb7	/* Enter 4 byte mode */
#define EX4B		0xe9	/* Exit  4 byte mode*/
/*
 *  Used in conjunction with bitops.h to access register bitfields.
 *  They are defined as bit offset/mask pairs for each DMA register
 *  bitfield.
 */
#define bfoSPI_FLASH_CTRLR0_FAST_RD         ((uint32_t)   20)
#define bfwSPI_FLASH_CTRLR0_FAST_RD         ((uint32_t)    1)
#define bfoSPI_FLASH_CTRLR0_DATA_CH         ((uint32_t)   18)
#define bfwSPI_FLASH_CTRLR0_DATA_CH         ((uint32_t)    2)
#define bfoSPI_FLASH_CTRLR0_ADDR_CH         ((uint32_t)   16)
#define bfwSPI_FLASH_CTRLR0_ADDR_CH         ((uint32_t)    2)
#define bfoSPI_FLASH_CTRLR0_CFS             ((uint32_t)   12)
#define bfwSPI_FLASH_CTRLR0_CFS             ((uint32_t)    3)
#define bfoSPI_FLASH_CTRLR0_SRL             ((uint32_t)   11)
#define bfwSPI_FLASH_CTRLR0_SRL             ((uint32_t)    1)
#define bfoSPI_FLASH_CTRLR0_SLV_OE          ((uint32_t)   10)
#define bfwSPI_FLASH_CTRLR0_SLV_OE          ((uint32_t)    1)
#define bfoSPI_FLASH_CTRLR0_TMOD            ((uint32_t)    8)
#define bfwSPI_FLASH_CTRLR0_TMOD            ((uint32_t)    2)
#define bfoSPI_FLASH_CTRLR0_SCPOL           ((uint32_t)    7)
#define bfwSPI_FLASH_CTRLR0_SCPOL           ((uint32_t)    1)
#define bfoSPI_FLASH_CTRLR0_FRF             ((uint32_t)    4)
#define bfwSPI_FLASH_CTRLR0_FRF             ((uint32_t)    2)
#define bfoSPI_FLASH_CTRLR0_DFS             ((uint32_t)    0)
#define bfwSPI_FLASH_CTRLR0_DFS             ((uint32_t)    4)
#define bfoSPI_FLASH_CTRLR1_NDF             ((uint32_t)    0)
#define bfwSPI_FLASH_CTRLR1_NDF             ((uint32_t)   16)
#define bfoSPI_FLASH_SSIENR_SSI_EN          ((uint32_t)    0)
#define bfwSPI_FLASH_SSIENR_SSI_EN          ((uint32_t)    1)
#define bfoSPI_FLASH_MWCR_MHS               ((uint32_t)    2)
#define bfwSPI_FLASH_MWCR_MHS               ((uint32_t)    1)
#define bfoSPI_FLASH_MWCR_MDD               ((uint32_t)    1)
#define bfwSPI_FLASH_MWCR_MDD               ((uint32_t)    1)
#define bfoSPI_FLASH_MWCR_MWMOD             ((uint32_t)    0)
#define bfwSPI_FLASH_MWCR_MWMOD             ((uint32_t)    1)
#define bfoSPI_FLASH_SER                    ((uint32_t)    0)
#define bfwSPI_FLASH_SER                    ((uint32_t)    4)
#define bfoSPI_FLASH_BAUDR_SCKDV            ((uint32_t)    0)
#define bfwSPI_FLASH_BAUDR_SCKDV            ((uint32_t)   16)
#define bfoSPI_FLASH_TXFTLR_TFT             ((uint32_t)    0)
#define bfwSPI_FLASH_TXFTLR_TFT             ((uint32_t)    3)
#define bfoSPI_FLASH_RXFTLR_RFT             ((uint32_t)    0)
#define bfwSPI_FLASH_RXFTLR_RFT             ((uint32_t)    3)
#define bfoSPI_FLASH_TXFLR_TXTFL            ((uint32_t)    0)
#define bfwSPI_FLASH_TXFLR_TXTFL            ((uint32_t)    3)
#define bfoSPI_FLASH_RXFLR_RXTFL            ((uint32_t)    0)
#define bfwSPI_FLASH_RXFLR_RXTFL            ((uint32_t)    3)
#define bfoSPI_FLASH_SR_BUSY                ((uint32_t)    0)
#define bfwSPI_FLASH_SR_BUSY                ((uint32_t)    1)
#define bfoSPI_FLASH_SR_TFNF                ((uint32_t)    1)
#define bfwSPI_FLASH_SR_TFNF                ((uint32_t)    1)
#define bfoSPI_FLASH_SR_TFE                 ((uint32_t)    2)
#define bfwSPI_FLASH_SR_TFE                 ((uint32_t)    1)
#define bfoSPI_FLASH_SR_RFNE                ((uint32_t)    3)
#define bfwSPI_FLASH_SR_RFNE                ((uint32_t)    1)
#define bfoSPI_FLASH_SR_RFF                 ((uint32_t)    4)
#define bfwSPI_FLASH_SR_RFF                 ((uint32_t)    1)
#define bfoSPI_FLASH_SR_TXE                 ((uint32_t)    5)
#define bfwSPI_FLASH_SR_TXE                 ((uint32_t)    1)
#define bfoSPI_FLASH_SR_DCOL                ((uint32_t)    6)
#define bfwSPI_FLASH_SR_DCOL                ((uint32_t)    1)
#define bfoSPI_FLASH_IMR_TXEIM              ((uint32_t)     0)
#define bfwSPI_FLASH_IMR_TXEIM              ((uint32_t)     1)
#define bfoSPI_FLASH_IMR_TXOIM              ((uint32_t)     1)
#define bfwSPI_FLASH_IMR_TXOIM              ((uint32_t)     1)
#define bfoSPI_FLASH_IMR_RXUIM              ((uint32_t)     2)
#define bfwSPI_FLASH_IMR_RXUIM              ((uint32_t)     1)
#define bfoSPI_FLASH_IMR_RXOIM              ((uint32_t)     3)
#define bfwSPI_FLASH_IMR_RXOIM              ((uint32_t)     1)
#define bfoSPI_FLASH_IMR_RXFIM              ((uint32_t)     4)
#define bfwSPI_FLASH_IMR_RXFIM              ((uint32_t)     1)
#define bfoSPI_FLASH_IMR_FSEIM              ((uint32_t)     5)
#define bfwSPI_FLASH_IMR_FSEIM              ((uint32_t)     1)
#define bfoSPI_FLASH_IMR_WBEIM              ((uint32_t)     6)
#define bfwSPI_FLASH_IMR_WBEIM              ((uint32_t)     1)
#define bfoSPI_FLASH_IMR_BYEIM              ((uint32_t)     7)
#define bfwSPI_FLASH_IMR_BYEIM              ((uint32_t)     1)
#define bfoSPI_FLASH_IMR_ACTIM              ((uint32_t)     8)
#define bfwSPI_FLASH_IMR_ACTIM              ((uint32_t)     1)
#define bfoSPI_FLASH_IMR_TXEIM_PEND         ((uint32_t)     9)
#define bfwSPI_FLASH_IMR_TXEIM_PEND         ((uint32_t)     1)
#define bfoSPI_FLASH_ISR_TXEIS              ((uint32_t)     0)
#define bfwSPI_FLASH_ISR_TXEIS              ((uint32_t)     1)
#define bfoSPI_FLASH_ISR_TXOIS              ((uint32_t)     1)
#define bfwSPI_FLASH_ISR_TXOIS              ((uint32_t)     1)
#define bfoSPI_FLASH_ISR_RXUIS              ((uint32_t)     2)
#define bfwSPI_FLASH_ISR_RXUIS              ((uint32_t)     1)
#define bfoSPI_FLASH_ISR_RXOIS              ((uint32_t)     3)
#define bfwSPI_FLASH_ISR_RXOIS              ((uint32_t)     1)
#define bfoSPI_FLASH_ISR_RXFIS              ((uint32_t)     4)
#define bfwSPI_FLASH_ISR_RXFIS              ((uint32_t)     1)
#define bfoSPI_FLASH_ISR_FSEIS              ((uint32_t)     5)
#define bfwSPI_FLASH_ISR_FSEIS              ((uint32_t)     1)
#define bfoSPI_FLASH_ISR_WBEIS              ((uint32_t)     6)
#define bfwSPI_FLASH_ISR_WBEIS              ((uint32_t)     1)
#define bfoSPI_FLASH_ISR_BYEIS              ((uint32_t)     7)
#define bfwSPI_FLASH_ISR_BYEIS              ((uint32_t)     1)
#define bfoSPI_FLASH_ISR_ACTIS              ((uint32_t)     8)
#define bfwSPI_FLASH_ISR_ACTIS              ((uint32_t)     1)
#define bfoSPI_FLASH_ISR_TXEIS_PEND         ((uint32_t)     9)
#define bfwSPI_FLASH_ISR_TXEIS_PEND         ((uint32_t)     1)
#define bfoSPI_FLASH_RISR_TXEIR             ((uint32_t)     0)
#define bfwSPI_FLASH_RISR_TXEIR             ((uint32_t)     1)
#define bfoSPI_FLASH_RISR_TXOIR             ((uint32_t)     1)
#define bfwSPI_FLASH_RISR_TXOIR             ((uint32_t)     1)
#define bfoSPI_FLASH_RISR_RXUIR             ((uint32_t)     2)
#define bfwSPI_FLASH_RISR_RXUIR             ((uint32_t)     1)
#define bfoSPI_FLASH_RISR_RXOIR             ((uint32_t)     3)
#define bfwSPI_FLASH_RISR_RXOIR             ((uint32_t)     1)
#define bfoSPI_FLASH_RISR_RXFIR             ((uint32_t)     4)
#define bfwSPI_FLASH_RISR_RXFIR             ((uint32_t)     1)
#define bfoSPI_FLASH_RISR_FSEIR             ((uint32_t)     5)
#define bfwSPI_FLASH_RISR_FSEIR             ((uint32_t)     1)
#define bfoSPI_FLASH_RISR_WBEIR             ((uint32_t)     6)
#define bfwSPI_FLASH_RISR_WBEIR             ((uint32_t)     1)
#define bfoSPI_FLASH_RISR_BYEIR             ((uint32_t)     7)
#define bfwSPI_FLASH_RISR_BYEIR             ((uint32_t)     1)
#define bfoSPI_FLASH_RISR_ACTIR             ((uint32_t)     8)
#define bfwSPI_FLASH_RISR_ACTIR             ((uint32_t)     1)
#define bfoSPI_FLASH_TXOICR_TXOICR          ((uint32_t)     0)
#define bfwSPI_FLASH_TXOICR_TXOICR          ((uint32_t)     1)
#define bfoSPI_FLASH_RXOICR_RXOICR          ((uint32_t)     0)
#define bfwSPI_FLASH_RXOICR_RXOICR          ((uint32_t)     1)
#define bfoSPI_FLASH_RXUICR_RXUICR          ((uint32_t)     0)
#define bfwSPI_FLASH_RXUICR_RXUICR          ((uint32_t)     1)
#define bfoSPI_FLASH_MSTICR_MSTICR          ((uint32_t)     0)
#define bfwSPI_FLASH_MSTICR_MSTICR          ((uint32_t)     1)
#define bfoSPI_FLASH_ICR_ICR                ((uint32_t)     0)
#define bfwSPI_FLASH_ICR_ICR                ((uint32_t)     1)
#define bfoSPI_FLASH_DMACR_RDMAE            ((uint32_t)     0)
#define bfwSPI_FLASH_DMACR_RDMAE            ((uint32_t)     1)
#define bfoSPI_FLASH_DMACR_TDMAE            ((uint32_t)     1)
#define bfwSPI_FLASH_DMACR_TDMAE            ((uint32_t)     1)
#define bfoSPI_FLASH_DMATDLR_DMATDL         ((uint32_t)     0)
#define bfwSPI_FLASH_DMATDLR_DMATDL         ((uint32_t)     3)
#define bfoSPI_FLASH_DMARDLR_DMARDL         ((uint32_t)     0)
#define bfwSPI_FLASH_DMARDLR_DMARDL         ((uint32_t)     3)
#define bfoSPI_FLASH_DR0_dr0                ((uint32_t)     0)
#define bfwSPI_FLASH_DR0_dr0                ((uint32_t)    16)
#define bfoSPI_FLASH_DR1_dr1                ((uint32_t)     0)
#define bfwSPI_FLASH_DR1_dr1                ((uint32_t)    16)
#define bfoSPI_FLASH_DR2_dr2                ((uint32_t)     0)
#define bfwSPI_FLASH_DR2_dr2                ((uint32_t)    16)
#define bfoSPI_FLASH_DR3_dr3                ((uint32_t)     0)
#define bfwSPI_FLASH_DR3_dr3                ((uint32_t)    16)
#define bfoSPI_FLASH_DR4_dr4                ((uint32_t)     0)
#define bfwSPI_FLASH_DR4_dr4                ((uint32_t)    16)
#define bfoSPI_FLASH_DR5_dr5                ((uint32_t)     0)
#define bfwSPI_FLASH_DR5_dr5                ((uint32_t)    16)
#define bfoSPI_FLASH_DR6_dr6                ((uint32_t)     0)
#define bfwSPI_FLASH_DR6_dr6                ((uint32_t)    16)
#define bfoSPI_FLASH_DR7_dr7                ((uint32_t)     0)
#define bfwSPI_FLASH_DR7_dr7                ((uint32_t)    16)
#define bfoSPI_FLASH_DR8_dr8                ((uint32_t)     0)
#define bfwSPI_FLASH_DR8_dr8                ((uint32_t)    16)
#define bfoSPI_FLASH_DR9_dr9                ((uint32_t)     0)
#define bfwSPI_FLASH_DR9_dr9                ((uint32_t)    16)
#define bfoSPI_FLASH_DR10_dr10              ((uint32_t)     0)
#define bfwSPI_FLASH_DR10_dr10              ((uint32_t)    16)
#define bfoSPI_FLASH_DR11_dr11              ((uint32_t)     0)
#define bfwSPI_FLASH_DR11_dr11              ((uint32_t)    16)
#define bfoSPI_FLASH_DR12_dr12              ((uint32_t)     0)
#define bfwSPI_FLASH_DR12_dr12              ((uint32_t)    16)
#define bfoSPI_FLASH_DR13_dr13              ((uint32_t)     0)
#define bfwSPI_FLASH_DR13_dr13              ((uint32_t)    16)
#define bfoSPI_FLASH_DR14_dr14              ((uint32_t)     0)
#define bfwSPI_FLASH_DR14_dr14              ((uint32_t)    16)
#define bfoSPI_FLASH_DR15_dr15              ((uint32_t)     0)
#define bfwSPI_FLASH_DR15_dr15              ((uint32_t)    16)
#define bfoSPI_FLASH_AUTO_LEN_ADDR          ((uint32_t)    16)
#define bfwSPI_FLASH_AUTO_LEN_ADDR          ((uint32_t)     2)
#define bfoSPI_FLASH_AUTO_LEN_DUM           ((uint32_t)     0)
#define bfwSPI_FLASH_AUTO_LEN_DUM           ((uint32_t)    16)


/*  This macro is used to initialize a spi_flash_param structure.  To use
 *  this macro, the relevant C header file must also be included.  This
 *  is generated when a DesignWare device is synthesized.
 */
#define CC_DEFINE_SPI_FLASH_PARAMS(prefix) { \
    prefix ## CC_SPI_FLASH_NUM_SLAVES            ,\
    prefix ## CC_SPI_FLASH_TX_FIFO_DEPTH         ,\
    prefix ## CC_SPI_FLASH_RX_FIFO_DEPTH         ,\
    prefix ## CC_SPI_FLASH_ID                    ,\
    prefix ## CC_SPI_FLASH_DFLT_SCPOL            ,\
    prefix ## CC_SPI_FLASH_DFLT_SCPH             ,\
    prefix ## CC_SPI_FLASH_CLK_PERIOD            ,\
    prefix ## CC_SPI_FLASH_VERSION_ID            \
}

/*
 *  This data type is used to describe read type with multi_channel
 */
enum spi_flash_byte_num
{
	DATA_BYTE         = 0,
	DATA_HALF         = 1,
	DATA_WORD         = 2
};

enum spi_flash_dr_number
{
	DR0               = 0 ,
	DR1               = 1 ,
	DR2               = 2 ,
	DR3               = 3 ,
	DR4               = 4 ,
	DR5               = 5 ,
	DR6               = 6 ,
	DR7               = 7 ,
	DR8               = 8 ,
	DR9               = 9 ,
	DR10              = 10,
	DR11              = 11,
	DR12              = 12,
	DR13              = 13,
	DR14              = 14,
	DR15              = 15,
	DR16              = 16,
	DR17              = 17,
	DR18              = 18,
	DR19              = 19,
	DR20              = 20,
	DR21              = 21,
	DR22              = 22,
	DR23              = 23,
	DR24              = 24,
	DR25              = 25,
	DR26              = 26,
	DR27              = 27,
	DR28              = 28,
	DR29              = 29,
	DR30              = 30,
	DR31              = 31
};

enum flash_rd_multi_type {
	RD_MULTI_NONE = 0x00,
	RD_DUAL_O     = 0x01,
	RD_DUAL_IO    = 0x02,
	RD_QUAD_O     = 0x03,
	RD_QUAD_IO    = 0x04
};

/*
 * This data type is used to describe write type with multi_channel
 */
enum flash_wr_multi_type {
	WR_MULTI_NONE = 0x00,
	WR_DUAL_I     = 0x01,
	WR_DUAL_II    = 0x02,
	WR_QUAD_I     = 0x03,
	WR_QUAD_II    = 0x04
};

/*
 * This data type is used to describe m25p80 mode type
 */
enum flash_mode_type {
	M25P80_NORMAL = 0,
	M25P80_FAST,
	M25P80_QUAD,
	M25P80_DUAL,
	M25P80_AUTO,
	M25P80_QUAD_WRITE,
	M25P80_DUAL_WRITE,
	M25P80_NORMAL_WRITE,
	M25P80_AUTO_WRITE
};

struct sheipa_spi {
	struct spi_master	*master;
	void __iomem		*regs;
	void __iomem		*auto_regs;
	void *			comp_param;
};

/*
 * This is the structure used for accessing the spi_flash register
 * portmap.
 */
struct spi_flash_portmap {
/* Channel registers
 * The offset address for each of the channel registers
 * is shown for channel 0. For other channel numbers
 * use the following equation.
 * offset = (channel_num * 0x058) + channel_0 offset
 */
	struct {
		volatile uint32_t ctrlr0;    /* Control Reg 0           (0x000) */
		volatile uint32_t ctrlr1;
		volatile uint32_t ssienr;    /* SPIC enable Reg1        (0x008) */
		volatile uint32_t mwcr;
		volatile uint32_t ser;       /* Slave enable Reg        (0x010) */
		volatile uint32_t baudr;
		volatile uint32_t txftlr;    /* TX_FIFO threshold level (0x018) */
		volatile uint32_t rxftlr;
		volatile uint32_t txflr;     /* TX_FIFO threshold level (0x020) */
		volatile uint32_t rxflr;
		volatile uint32_t sr;        /* Destination Status Reg  (0x028) */
		volatile uint32_t imr;
		volatile uint32_t isr;       /* Interrupt Stauts Reg    (0x030) */
		volatile uint32_t risr;
		volatile uint32_t txoicr;    /* TX_FIFO overflow_INT clear (0x038) */
		volatile uint32_t rxoicr;
		volatile uint32_t rxuicr;    /* RX_FIFO underflow_INT clear (0x040) */
		volatile uint32_t msticr;
		volatile uint32_t icr;       /* Interrupt clear Reg     (0x048) */
		volatile uint32_t dmacr;
		volatile uint32_t dmatdlr;   /* DMA TX_data level       (0x050) */
		volatile uint32_t dmardlr;
		volatile uint32_t idr;       /* Identiation Scatter Reg (0x058) */
		volatile uint32_t spi_flash_version;
		union{
			volatile uint8_t  byte;
			volatile uint16_t half;
			volatile uint32_t word;
		} dr[32];
		volatile uint32_t rd_fast_single;
		volatile uint32_t rd_dual_o; /* Read dual data cmd Reg  (0x0e4) */
		volatile uint32_t rd_dual_io;
		volatile uint32_t rd_quad_o; /* Read quad data cnd Reg  (0x0ec) */
		volatile uint32_t rd_quad_io;
		volatile uint32_t wr_single; /* write single cmd Reg    (0x0f4) */
		volatile uint32_t wr_dual_i;
		volatile uint32_t wr_dual_ii;/* write dual addr/data cmd(0x0fc) */
		volatile uint32_t wr_quad_i;
		volatile uint32_t wr_quad_ii;/* write quad addr/data cnd(0x104) */
		volatile uint32_t wr_enable;
		volatile uint32_t rd_status; /* read status cmd Reg     (0x10c) */
		volatile uint32_t ctrlr2;
		volatile uint32_t fbaudr;    /* fast baud rate Reg      (0x114) */
		volatile uint32_t addr_length;
		volatile uint32_t auto_length; /* Auto addr length Reg  (0x11c) */
		volatile uint32_t valid_cmd;
		volatile uint32_t flash_size; /* Flash size Reg         (0x124) */
		volatile uint32_t flush_fifo;
	};
};

struct spi_flash_param {
	uint32_t spi_flash_num_slaves ;    /* slaves number */
	uint32_t spi_flash_tx_fifo_depth;  /* TX fifo depth number */
	uint32_t spi_flash_rx_fifo_depth;  /* RX fifo depth number */
	uint32_t spi_flash_idr        ;    /* ID code */
	uint32_t spi_flash_scpol      ;    /* Serial clock polarity */
	uint32_t spi_flash_scph       ;    /* Serial clock phase */
	uint32_t spi_flash_clk_period ;    /* serial clock period */
	uint32_t spi_flash_version_id ;    /* spi flash ID */
};

/* This function is used to wait the SSI is not at busy state. */
static void flash_wait_busy(struct sheipa_spi *dev);
/* This function is used to read status of flash. */
static uint8_t flash_get_status(struct sheipa_spi *dev);

#endif
