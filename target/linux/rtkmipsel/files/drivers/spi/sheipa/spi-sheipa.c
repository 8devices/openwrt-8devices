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

#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/scatterlist.h>

/* SPIC FLASH Headder */
#include "spi-sheipa.h"

#define DRIVER_NAME "spi-sheipa"
#define CMD_ADDR_FORMAT(cmd, addr) (((cmd) & 0x000000ff) | \
				   ((addr & 0x000000ff) << 24) | \
				   ((addr & 0x0000ff00) << 8 ) | \
				   ((addr & 0x00ff0000) >> 8 ))

#define CMD_FORMAT(cmd)	(cmd & 0x000000ff)
#define ADDR_4BYTE_FORMAT(addr)	(((addr & 0x000000ff) << 24) | \
				((addr & 0x0000ff00) << 8)   | \
				((addr & 0x00ff0000) >> 8)   | \
				(addr & 0xff000000) >> 24)
/* The mode value 0 is user mode and 1 is auto mode */
static uint32_t	mode = 0;
/* Enter 4-byte address mode */
static uint32_t	enable_addr_4byte_mode = 0;
static uint32_t	addr_offset = 0;
static uint32_t	first_dummy_xfer = 0;
static uint32_t	first_flash_mode_xfer = 0;
static uint32_t	first_type_xfer = 0;
static uint8_t	first_cmd_xfer = 0;

/* SPIC Driver */
static uint32_t spi_flash_setser(struct sheipa_spi *dev, uint32_t ser_num)
{
	struct spi_flash_portmap *spi_flash_map;
	struct spi_flash_param *spi_flash_para;

	spi_flash_map = dev->regs;
	spi_flash_para = dev->comp_param;

	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;

	if(ser_num >= (1 << (spi_flash_para->spi_flash_num_slaves)))
		return DW_EINVAL;
	else {
		spi_flash_map->ser = 1 << ser_num;
		return 0;
	}
}

/*
 * This function is used to set the control register.
 */
static void spi_flash_set_tx_mode(struct sheipa_spi *dev)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = dev->regs;
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;
	spi_flash_map->ctrlr0 = spi_flash_map->ctrlr0 & 0xfffffcff;
}

/*
 * This function is used to set the control register.
 */
static void spi_flash_set_rx_mode(struct sheipa_spi *dev)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = dev->regs;
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;
	spi_flash_map->ctrlr0 = (spi_flash_map->ctrlr0 | 0x0300);
}

/*
 * This function is used to set the ctrlr1 controller.
 */
static uint32_t spi_flash_setctrlr1(struct sheipa_spi *dev, uint32_t num_frame)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map= dev->regs;
	spi_flash_map->ssienr = 0;
	if (num_frame > 0x00010000)
		return DW_ENODATA;
	else {
		spi_flash_map->ctrlr1 = num_frame;
		return 0;
	}
}

/*
 * This function is used to set the ctrlr1 controller.
 */
static uint32_t spi_flash_setdr(struct sheipa_spi *dev,
				enum spi_flash_dr_number dr_num,
				uint32_t data,
				enum spi_flash_byte_num byte_num)
{
	struct spi_flash_portmap *spi_flash_map;
	uint32_t wr_data;

	spi_flash_map= dev->regs;

	if (byte_num == DATA_BYTE)
		wr_data = (data & 0x000000ff);
	else if (byte_num == DATA_HALF) {
		wr_data = cpu_to_le16(data);
	}
	else if (byte_num == DATA_WORD) {
		wr_data = cpu_to_le32(data);
	}

	if (dr_num > DR31)
		return DW_EINVAL;
	else {
		if (byte_num == DATA_BYTE)
			spi_flash_map->dr[dr_num].byte = wr_data;
		else if (byte_num == DATA_HALF)
			 spi_flash_map->dr[dr_num].half = wr_data;
		else if (byte_num == DATA_WORD)
			 spi_flash_map->dr[dr_num].word = wr_data;
		else
			return DW_EINVAL;
	}

	return 0;
}

/*
 * This function is used to set the baud rate register.
 */
static uint32_t spi_flash_setbaudr(struct sheipa_spi *dev, uint32_t baudrate)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = dev->regs;
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;

	if ((baudrate & 0xffff0000) || (baudrate == 0))
		return DW_ENODATA;
	else {
		spi_flash_map->baudr = baudrate;
		return 0;
	}
}

/*
 * This function is used to set the fast baud rate register for fast read cmd.
 */
static uint32_t spi_flash_setfbaudr(struct sheipa_spi *dev, uint32_t fbaudrate)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = dev->regs;
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;

	if ((fbaudrate & 0xffff0000) || (fbaudrate == 0))
		return DW_ENODATA;
	else {
		spi_flash_map->fbaudr = fbaudrate;
		return 0;
	}
}

/*
 * This function is used to set the Baudr controller.
 */
static uint32_t spi_flash_set_dummy_cycle(struct sheipa_spi *dev,
					uint32_t dum_cycle)
{
	struct spi_flash_portmap *spi_flash_map;
	uint32_t cycle;

	cycle = 0;
	spi_flash_map= dev->regs;
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;

	/* if using fast_read baud_rate */
	if (((spi_flash_map->ctrlr0) & 0x00100000))
		cycle = (spi_flash_map->fbaudr);
	else
		cycle = (spi_flash_map->baudr);

	cycle = (cycle*dum_cycle*2) + DEF_RD_TUNING_DUMMY_CYCLE;
	if (cycle > 0x10000)
		return DW_ECHRNG;

	DW_BITS_SET_VAL(spi_flash_map->auto_length, bfoSPI_FLASH_AUTO_LEN_DUM,
		    cycle, bfwSPI_FLASH_AUTO_LEN_DUM);

	return 0;
}

/*
 * This function is used to read the ctrlr1 controller.
 */
static uint32_t spi_flash_getdr(struct sheipa_spi *dev,
				enum spi_flash_dr_number dr_num,
				enum spi_flash_byte_num byte_num)
{
	struct spi_flash_portmap *spi_flash_map;
	uint32_t rd_data;
	uint32_t data;

	spi_flash_map= (struct spi_flash_portmap *)dev->regs;

	if (dr_num > DR31)
		return DW_ECHRNG;
	else {
		if (byte_num == DATA_BYTE)
			 data = spi_flash_map->dr[dr_num].byte & 0x000000ff;
		else if (byte_num == DATA_HALF) {
			 rd_data = spi_flash_map->dr[dr_num].half & 0x0000ffff;
			 data = le16_to_cpu(rd_data);
		}
		else if (byte_num == DATA_WORD) {
			rd_data =  spi_flash_map->dr[dr_num].word;
			data = le32_to_cpu(rd_data);
		}
		else
			return DW_EIO;

		return data;
	}
}

/*
 * This function is used to wait the spi_flash is not at busy state.
 */
void spi_flash_wait_busy(struct sheipa_spi *dev)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map= dev->regs;

	while (1) {
		if (DW_BIT_GET_UNSHIFTED(spi_flash_map->sr, bfoSPI_FLASH_SR_TXE)) {
			printk(KERN_INFO "spi_flash: transfer error. \n");
			break;
		}
		else {
			if ((!DW_BIT_GET_UNSHIFTED(spi_flash_map->sr, bfoSPI_FLASH_SR_BUSY)))
				 break;
		}
	}
}

/*
 * The function is used to set
 * cmd(1 byte) + address (3 byte) or
 * cmd(1 byte) + address (4 byte) according to
 * enable 4 byte address mode(EN4B)
 */
static void spi_flash_set_cmd_addr(struct sheipa_spi *dev, uint32_t wr_addr,
					uint8_t wr_cmd)
{
	uint32_t wr_cmd_addr;

	/* set flash cmd and addr */
	if (!enable_addr_4byte_mode) {
		wr_cmd_addr = CMD_ADDR_FORMAT(wr_cmd, wr_addr);
		/* Write cmd, addr, data into FIFO */
		spi_flash_setdr(dev, DR0, wr_cmd_addr, DATA_WORD);
	}
	else {
		wr_cmd = CMD_FORMAT(wr_cmd);
		wr_addr = ADDR_4BYTE_FORMAT(wr_addr);
		spi_flash_setdr(dev, DR0, wr_cmd, DATA_BYTE);
		spi_flash_setdr(dev, DR1, wr_addr, DATA_WORD);
	}
}

/*
 * This function is used to set tx command such as WREN, CE command.
 */
static void flash_tx_cmd(struct sheipa_spi *dev, uint8_t cmd)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = dev->regs;

	/* Disble SPI_FLASH */
	spi_flash_map->ssienr = 0;
	/* set ctrlr0: TX mode */
	spi_flash_set_tx_mode(dev);
	spi_flash_map->ctrlr0 = (spi_flash_map->ctrlr0 & 0xfff0ffff);
	/* set flash_cmd: wren to fifo */
	spi_flash_setdr(dev, DR0, cmd, DATA_BYTE);
	/* Enable SPI_FLASH */
	spi_flash_map->ssienr = 1;
	spi_flash_wait_busy(dev);
	flash_wait_busy(dev);
}

/*
 * This function is used to set tx command such as RDID, RDSR command.
 */
static void flash_rx_cmd(struct sheipa_spi *dev, uint8_t cmd)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = (struct spi_flash_portmap *)dev->regs;

	/* Disble SPI_FLASH */
	spi_flash_map->ssienr = 0;
	/* set ctrlr0: RX_mode */
	spi_flash_set_rx_mode(dev);
	spi_flash_map->ctrlr0 = (spi_flash_map->ctrlr0 & 0xfff0ffff);
	/* set flash_cmd: write cmd to fifo */
	spi_flash_setdr(dev, DR0, cmd, DATA_BYTE);
}

/*
 * This function is used to set flash status register.
 */
static void flash_set_status(struct sheipa_spi *dev, uint32_t addr, uint8_t cmd)
{
	struct spi_flash_portmap *spi_flash_map;
	uint32_t info;
	uint32_t data;

	spi_flash_map = dev->regs;
	info = spi_flash_map->addr_length;
	/* Set flash_cmd: WREN to FIFO */
	if (!enable_addr_4byte_mode)
		data = (uint32_t)(addr >> 16);
	else /* for 4-byte address mode */
		data = (uint32_t)(addr >> 24);
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;

	/* set ctrlr0: TX mode */
	spi_flash_set_tx_mode(dev);
	spi_flash_map->ctrlr0 = (spi_flash_map->ctrlr0 & 0xfff0ffff);
	spi_flash_map->addr_length = 1;
	/* Set flash_cmd: WRSR to FIFO */
	spi_flash_setdr(dev, DR0, cmd, DATA_BYTE);
	/* Set data FIFO */
	spi_flash_setdr(dev, DR0, data, DATA_BYTE);

	spi_flash_map->ssienr = 1;
	spi_flash_wait_busy(dev);

	spi_flash_map->ssienr = 0;
	spi_flash_map->addr_length = info;

	flash_wait_busy(dev);
}

/*
 * This function is used to get flash status for flash_wait_busy.
 */
static uint8_t flash_get_status(struct sheipa_spi *dev)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = dev->regs;
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;

	/* Set Ctrlr1; 1 byte data frames */
	spi_flash_map->ctrlr1 = 1;

	/* Set tuning dummy cycles */
	DW_BITS_SET_VAL(spi_flash_map->auto_length,bfoSPI_FLASH_AUTO_LEN_DUM,
			DEF_RD_TUNING_DUMMY_CYCLE,
			bfwSPI_FLASH_AUTO_LEN_DUM);

	/* Set flash_cmd: RDSR to FIFO */
	flash_rx_cmd(dev, RDSR);

	/* Enable SPI_FLASH */
	spi_flash_map->ssienr = 1;
	spi_flash_wait_busy(dev);

	return spi_flash_getdr(dev, DR0, DATA_BYTE);
}

/*
 * this function is used to read status for mtd.
 */
static void flash_read_status(struct sheipa_spi *dev, uint8_t cmd)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = dev->regs;
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;

	/* Set tuning dummy cycles */
	DW_BITS_SET_VAL(spi_flash_map->auto_length,bfoSPI_FLASH_AUTO_LEN_DUM,
			DEF_RD_TUNING_DUMMY_CYCLE,
			bfwSPI_FLASH_AUTO_LEN_DUM);

	/* Set flash_cmd: RDSR to FIFO */
	flash_rx_cmd(dev, cmd);
}

static void flash_wait_busy(struct sheipa_spi *dev)
{
	/* Check flash is in write progress or not */
	while ((flash_get_status(dev) & 0x1));
}

/*
 * this function is used to chip erase.
 */
static void flash_chip_erase(struct sheipa_spi *dev, uint8_t cmd)
{
	flash_tx_cmd(dev, cmd);
}

/*
 * this function is used to sector erase 4kBi.
 */
static void flash_be_4k_erase(struct sheipa_spi *dev, uint32_t addr, uint8_t cmd)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = dev->regs;
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;

	/* set ctrlr0: TX_mode */
	DW_BITS_SET_VAL(spi_flash_map->ctrlr0, bfoSPI_FLASH_CTRLR0_TMOD,
			   0, bfwSPI_FLASH_CTRLR0_TMOD);

	/* set flash cmd + addr and write to fifo */
	spi_flash_set_cmd_addr(dev, addr, cmd);
	spi_flash_map->ssienr = 1;
	spi_flash_wait_busy(dev);
	flash_wait_busy(dev);
}

static void flash_se_erase(struct sheipa_spi *dev, uint32_t addr, uint8_t cmd)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = dev->regs;
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;

	/* set ctrlr0: TX_mode */
	DW_BITS_SET_VAL(spi_flash_map->ctrlr0, bfoSPI_FLASH_CTRLR0_TMOD,
			   0, bfwSPI_FLASH_CTRLR0_TMOD);

	/* set flash cmd + addr and write to fifo */
	spi_flash_set_cmd_addr(dev, addr, cmd);
	spi_flash_map->ssienr = 1;
	spi_flash_wait_busy(dev);
	flash_wait_busy(dev);
}

static void flash_write_disable(struct sheipa_spi *dev)
{
	flash_tx_cmd(dev, WRDI);
}

static void flash_write_enable(struct sheipa_spi *dev)
{
	flash_tx_cmd(dev, WREN);
}

static void flash_read_id(struct sheipa_spi *dev, uint8_t cmd)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = dev->regs;
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;
	/* Set Ctrlr1; 3 byte data frames */
	spi_flash_setctrlr1(dev, 3);
	spi_flash_set_dummy_cycle(dev, 0);
	flash_rx_cmd(dev, cmd);
}

/*
 * this function is used to send single write command.
 */
static void flash_write(struct sheipa_spi *dev, uint32_t addr,
				enum spi_flash_byte_num byte_num, uint8_t cmd)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = dev->regs;
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;
	/* set ctrlr0: TX mode, data_ch, addr_ch */
	spi_flash_set_tx_mode(dev);
	spi_flash_map->ctrlr0 = (spi_flash_map->ctrlr0 & 0xfff0ffff);
	/* set flash cmd + addr and write to fifo */
	spi_flash_set_cmd_addr(dev, addr, cmd);
}

/*
 * this function is used to send data to SPIC FIFO.
 */
static void flash_write_data(struct sheipa_spi *dev, uint32_t data,
				enum spi_flash_byte_num byte_num)
{
	if (byte_num == DATA_BYTE)
		spi_flash_setdr(dev, DR0, data, DATA_BYTE);
	else if (byte_num == DATA_HALF)
		spi_flash_setdr(dev, DR0, data, DATA_HALF);
	else if (byte_num == DATA_WORD)
		spi_flash_setdr(dev, DR0, data, DATA_WORD);
}

/*
 * this function is used to send single read command.
 */
static void flash_read(struct sheipa_spi *dev, uint32_t addr,
			enum spi_flash_byte_num byte_num, uint8_t cmd)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = dev->regs;
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;

	/* set ctrlr0: RX mode, data_ch, addr_ch */
	spi_flash_set_rx_mode(dev);
	spi_flash_map->ctrlr0 = (spi_flash_map->ctrlr0 & 0xfff0ffff);

	/* Set tuning dummy cycles */
	DW_BITS_SET_VAL(spi_flash_map->auto_length, bfoSPI_FLASH_AUTO_LEN_DUM,
			DEF_RD_TUNING_DUMMY_CYCLE,
			bfwSPI_FLASH_AUTO_LEN_DUM);

	/* set flash cmd + addr and write to fifo */
	spi_flash_set_cmd_addr(dev, addr, cmd);
}

/*
 * this function is used to send fast read command.
 */
static void flash_fastread(struct sheipa_spi *dev, uint32_t addr,
			enum spi_flash_byte_num byte_num,
			uint32_t dummy, uint8_t cmd)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = dev->regs;
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;
	spi_flash_set_dummy_cycle(dev, dummy);
	/* set ctrlr0: RX mode, data_ch, addr_ch */
	spi_flash_set_rx_mode(dev);
	spi_flash_map->ctrlr0 = (spi_flash_map->ctrlr0 & 0xfff0ffff);
	/* set flash cmd + addr and write to fifo */
	spi_flash_set_cmd_addr(dev, addr, cmd);
}

/*
 * this function is used to send 2-channel write command.
 */
static uint32_t flash_writex2(struct sheipa_spi *dev, uint32_t addr,
				enum spi_flash_byte_num byte_num,
				uint32_t type, uint8_t cmd)
{
	struct spi_flash_portmap *spi_flash_map;
	uint32_t init_data;

	spi_flash_map = dev->regs;
	/* Not support writex2 */
	if (type == WR_MULTI_NONE) {
		printk(KERN_INFO "Not support Writex2 command.\n");
		return  DW_EPERM;
	}
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;

	/* set ctrlr0: TX mode */
	init_data = spi_flash_map->ctrlr0;

	if (type == WR_DUAL_II)
		spi_flash_map->ctrlr0 = (init_data & 0xfff0fcff) | (0x00050000);
	else if (type == WR_DUAL_I)
		spi_flash_map->ctrlr0 = (init_data & 0xfff0fcff) | (0x00040000);
	else {
		printk(KERN_INFO "Not support Writex2 command.\n");
		return DW_EPERM;
	}
	/* set flash cmd + addr and write to fifo */
	spi_flash_set_cmd_addr(dev, addr, cmd);

	return 0;
}

/*
 * this function is used to send 2-channel read command.
 */
static uint32_t flash_readx2(struct sheipa_spi *dev, uint32_t addr,
				enum spi_flash_byte_num byte_num,
				uint32_t dummy, uint32_t type, uint8_t cmd)
{
	struct spi_flash_portmap *spi_flash_map;
	uint32_t ctrlr0;

	spi_flash_map = dev->regs;
	spi_flash_set_dummy_cycle(dev, dummy);
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;
	/* set ctrlr0: RX_mode */
	ctrlr0 = spi_flash_map->ctrlr0;

	if (type == RD_DUAL_IO)
		spi_flash_map->ctrlr0 = (ctrlr0 & 0xfff0fcff) | (0x00050300);
	else if (type == RD_DUAL_O)
		spi_flash_map->ctrlr0 = (ctrlr0 & 0xfff0fcff) | (0x00040300);
	else {
		printk(KERN_INFO "Not support readx2 command.\n");
		return  DW_EPERM;
	}
	/* set flash cmd + addr and write to fifo */
	spi_flash_set_cmd_addr(dev, addr, cmd);

	return 0;
}

/*
 * this function is used to send 4-channel write command.
 */
static uint32_t flash_writex4(struct sheipa_spi *dev, uint32_t addr,
				enum spi_flash_byte_num byte_num,
				uint32_t type, uint8_t cmd)
{
	struct spi_flash_portmap *spi_flash_map;
	uint32_t init_data;

	spi_flash_map = dev->regs;
	/* Not support writex4 */
	if (type == WR_MULTI_NONE) {
		printk(KERN_INFO "INFO:Not support Writex4 command.\n");
		return  DW_EPERM;
	}

	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;
	/* set ctrlr0: TX mode, data_ch, addr_ch */
	init_data = spi_flash_map->ctrlr0;

	if (type == WR_QUAD_II)
		spi_flash_map->ctrlr0 = (init_data & 0xfff0fcff) | (0x000a0000);
	else if (type == WR_QUAD_I)
		spi_flash_map->ctrlr0 = (init_data & 0xfff0fcff) | (0x00080000);
	else {
		printk(KERN_INFO "INFO:Not support Writex4 command.\n");
		return  DW_EPERM;
	}
	/* set flash cmd + addr and write to fifo */
	spi_flash_set_cmd_addr(dev, addr, cmd);

	return 0;
}

/*
 * this function is used to send 4-channel read command.
 */
static uint32_t flash_readx4(struct sheipa_spi *dev, uint32_t addr,
				enum spi_flash_byte_num byte_num,
				uint32_t dummy, uint32_t type, uint8_t cmd)
{
	struct spi_flash_portmap *spi_flash_map;
	uint32_t ctrlr0;

	spi_flash_map = dev->regs;
	spi_flash_set_dummy_cycle(dev, dummy);
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;

	/* set ctrlr0: RX_mode */
	ctrlr0 = spi_flash_map->ctrlr0;

	if (type == RD_QUAD_IO)
		spi_flash_map->ctrlr0 = (ctrlr0 & 0xfff0fcff) | (0x000a0300);
	else if (type == RD_QUAD_O)
		spi_flash_map->ctrlr0 = (ctrlr0 & 0xfff0fcff) | (0x00080300);
	else {
		printk(KERN_INFO "INFO:Not support readx4 command\n");
		return  DW_EPERM;
	}
	/* set flash cmd + addr and write to fifo */
	spi_flash_set_cmd_addr(dev, addr, cmd);

	return 0;
}

/*
 * this function is used to spic and flash initialization.
 */
static void flash_init(struct sheipa_spi *dev)
{
	struct spi_flash_portmap *spi_flash_map;
	spi_flash_map = (struct spi_flash_portmap *)dev->regs;
	/* Set baud_rate */
	spi_flash_setbaudr(dev, 8);
	/* Set fast baudr rate */
	spi_flash_setfbaudr(dev, 8);
	/* Set slave(FLASH) num */
	spi_flash_setser(dev, 0);
	/* init addr length 3 byte */
	spi_flash_map->addr_length = 3;
	/* using to init write signal */
	flash_write_disable(dev);
}

/* auto mode initial setting */
static void auto_init(struct sheipa_spi *dev)
{
	struct spi_flash_portmap *spi_flash_map;
	uint32_t info;

	spi_flash_map = (struct spi_flash_portmap *)dev->regs;
	info = 0;
	/* Disable SPI_FLASH */
	spi_flash_map->ssienr = 0;
	/* Set valid_cmd_reg: auto_cmd */
	if (DEF_WR_QUAD_TYPE == WR_QUAD_II) {
		info = 0x100;
		spi_flash_map->wr_quad_ii = PPX4_II;
	} else if (DEF_WR_QUAD_TYPE == WR_QUAD_I) {
		info = 0x080;
		spi_flash_map->wr_quad_i = PPX4_I;
	} else if (DEF_WR_DUAL_TYPE == WR_DUAL_II) {
		info = 0x040;
		spi_flash_map->wr_dual_ii = PPX2_II;
	} else if (DEF_WR_DUAL_TYPE == WR_DUAL_I) {
		info = 0x020;
		spi_flash_map->wr_dual_i = PPX2_I;
	}

	if (DEF_RD_QUAD_TYPE == RD_QUAD_IO) {
		info = info | 0x010;
		spi_flash_map->rd_quad_io = READX4_IO;
	} else if (DEF_RD_QUAD_TYPE == RD_QUAD_O) {
		info = info | 0x008;
		spi_flash_map->rd_quad_o = READX4_I;
	} else if (DEF_RD_DUAL_TYPE == RD_DUAL_IO) {
		info = info | 0x004;
		spi_flash_map->rd_dual_io = READX2_IO;
	} else if (DEF_RD_DUAL_TYPE == RD_DUAL_O) {
		info = info | 0x002;
		spi_flash_map->rd_dual_o = READX2_I;
	}
	spi_flash_map->valid_cmd = (spi_flash_map->valid_cmd | info | 0x200);
}

static void auto_write(struct sheipa_spi *dev, const u8 *buf, uint32_t len,
			uint32_t offset)
{
	uint32_t *addr;
	uint32_t i;
	uint32_t data;
	uint32_t cnt;

	i = 0;
	addr = (uint32_t *)(dev->auto_regs + offset);
	cnt = len / 4;

	for (i = 0; i < cnt; i++) {
		memcpy(&data, buf, 4);
		buf += 4;
		*addr = data;
		flash_wait_busy(dev);
		addr++;
	}
	cnt = len % 4;
	if (cnt > 0) {
		memcpy(&data, buf, cnt);
		buf += cnt;
		*addr = data;
		flash_wait_busy(dev);
		addr++;
	}
}

static void auto_read(struct sheipa_spi *dev, u8 *buf, uint32_t len,
			uint32_t offset)
{

	uint32_t *addr;
	uint32_t data;
	uint32_t i;
	uint32_t cnt;

	i = 0;
	addr = (uint32_t *)(dev->auto_regs + offset);
	/* set read dummy cycle */
	if (DEF_RD_QUAD_TYPE != RD_MULTI_NONE)
		spi_flash_set_dummy_cycle(dev, DEF_RD_QUAD_DUMMY_CYCLE);
	else if (DEF_RD_DUAL_TYPE  != RD_MULTI_NONE)
		spi_flash_set_dummy_cycle(dev, DEF_RD_DUAL_DUMMY_CYCLE);
	else
		spi_flash_set_dummy_cycle(dev, DEF_RD_TUNING_DUMMY_CYCLE);

	cnt = len / 4;
	for(i = 0; i < cnt; i++) {
		data = *addr;
		memcpy(buf, &data, 4);
		addr++;
		buf += 4;
	}

	cnt = len % 4;
	if (cnt > 0) {
		data = *addr;
		memcpy(buf, &data, 4);
		addr++;
		buf += cnt;
	}
}

/* Setting auto mode auto_length reg */
static void set_auto_length(struct sheipa_spi *dev)
{

	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = (struct spi_flash_portmap *)dev->regs;
	spi_flash_map->ssienr = 0;
	if (!enable_addr_4byte_mode) {
		/* setting auto mode CS_H_WR_LEN, address length(3 byte) */
		spi_flash_map->auto_length = ((spi_flash_map->auto_length & 0xfffcffff)
									  | 0x00030000);
	} else {
		/* setting auto mode CS_H_WR_LEN, address length(4 byte) */
		spi_flash_map->auto_length = (spi_flash_map->auto_length & 0x0ffcffff)
									 | 0x50000000;
	}
}

static void set_auto_addr_length(struct sheipa_spi *dev)
{

	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = (struct spi_flash_portmap *)dev->regs;
	spi_flash_map->ssienr = 0;
	if (!enable_addr_4byte_mode) {
		/* setting AUTO_ADDR_LENGTH (3 byte) */
		spi_flash_map->auto_length = ((spi_flash_map->auto_length & 0xfffcffff)
									  | 0x00030000);
	} else {
		/* setting AUTO_ADDR_LENGTH (4 byte) */
		spi_flash_map->auto_length = (spi_flash_map->auto_length & 0xfffcffff);
	}
}

/* Setting addr length 4 byte reg for user mode */
static void set_addr_length(struct sheipa_spi *dev)
{

	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = (struct spi_flash_portmap *)dev->regs;
	spi_flash_map->ssienr = 0;
	/* address length(4 byte) */
	spi_flash_map->addr_length = ((spi_flash_map->addr_length & 0xfffffff0)
								  | 0x00000000);
}

/* Enable 4 byte address mode */
static void flash_en4b(struct sheipa_spi *dev, uint8_t cmd)
{
	flash_tx_cmd(dev, cmd);
}

/* Exit 4 byte address mode */
static void flash_ex4b(struct sheipa_spi *dev, uint8_t cmd)
{
	flash_tx_cmd(dev, cmd);
}

/* Enable chip select */
static void enable_cs_write(struct sheipa_spi *dev)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = dev->regs;
	/* Enable SPI_FLASH */
	spi_flash_map->ssienr = 1;
	spi_flash_wait_busy(dev);
	flash_wait_busy(dev);
}

static void enable_cs_read(struct sheipa_spi *dev, uint32_t len)
{
	struct spi_flash_portmap *spi_flash_map;

	spi_flash_map = dev->regs;
	/* set receive data length */
	spi_flash_setctrlr1(dev, len);
	/* Enable SPI_FLASH */
	spi_flash_map->ssienr = 1;
	spi_flash_wait_busy(dev);
}

static inline void select_extend_op(struct sheipa_spi *dws, uint32_t addr,
				uint32_t dummy, enum flash_mode_type flash_mode,
				uint32_t type, uint8_t cmd)
{
	switch (flash_mode) {
	case M25P80_NORMAL:
	case M25P80_FAST:
	case M25P80_AUTO:
	case M25P80_NORMAL_WRITE:
	case M25P80_AUTO_WRITE:
		printk(KERN_INFO "INFO: flash mode error\n");
		break;
	case M25P80_QUAD:
		flash_readx4(dws, addr, DATA_WORD, dummy, type, cmd);
		break;
	case M25P80_DUAL:
		flash_readx2(dws, addr, DATA_WORD, dummy, type, cmd);
		break;
	case M25P80_QUAD_WRITE:
		flash_writex4(dws, addr, DATA_WORD, type, cmd);
		break;
	case M25P80_DUAL_WRITE:
		flash_writex2(dws, addr, DATA_WORD, type, cmd);
		break;
	default:
		printk(KERN_INFO "INFO:No support flash cmd:0x%x\n", cmd);
		break;
	}
}

static inline void select_op(struct sheipa_spi *dws, uint32_t addr,
				uint32_t dummy, enum flash_mode_type flash_mode,
				uint32_t type, uint8_t cmd)
{
	mode = 0;
	switch (cmd) {
	case PP:
		flash_write(dws, addr, DATA_WORD, cmd);
		break;
	case WREN:
		flash_write_enable(dws);
		break;
	case WRSR:
		flash_set_status(dws, addr, cmd);
		break;
	case RDSR:
		flash_read_status(dws, cmd);
		break;
	case RDID:
		flash_read_id(dws, cmd);
		break;
	case NORM_READ:
		flash_read(dws, addr, DATA_WORD, cmd);
		break;
	case FAST_READ:
		flash_fastread(dws, addr, DATA_WORD, dummy, cmd);
		break;
	case CE:
		flash_chip_erase(dws, cmd);
		break;
	case BE_4K:
		flash_be_4k_erase(dws, addr, cmd);
		break;
	case SE:
		flash_se_erase(dws, addr, cmd);
		break;
	case AUTO_MODE:
		mode = 1;
		set_auto_length(dws);
		break;
	/* supprot for address 4 byte mdoe */
	case EN4B:
		flash_en4b(dws, cmd);
		enable_addr_4byte_mode = 1;
		set_addr_length(dws);
		set_auto_addr_length(dws);
		break;
	case EX4B:
		flash_ex4b(dws, cmd);
		enable_addr_4byte_mode = 0;
		break;
	default:
		select_extend_op(dws, addr, dummy, flash_mode, type, cmd);
		break;
	}

}

/* send cmd(1 byte) + addr(3 byte or 4 byte) to spi fifo */
static inline void do_spi_cmd(struct sheipa_spi *dws, const u8 *buf,
				uint32_t len)
{
	uint32_t addr;
	uint32_t dummy;
	enum flash_mode_type flash_mode;
	uint32_t type;
	uint8_t	cmd;

	cmd = buf[0];
	if (!enable_addr_4byte_mode) {
		addr = (uint32_t)((buf[1] << 16) | (buf[2] << 8) | (buf[3]));
	}
	else {
		addr = (uint32_t)((buf[1] << 24) | (buf[2] << 16) |
				 (buf[3] << 8) | (buf[4]));
	}

	flash_mode = buf[5];
	dummy = buf[6];
	type = buf[7];
	addr_offset = addr;
	first_cmd_xfer = cmd;
	first_dummy_xfer = dummy;
	first_flash_mode_xfer = flash_mode;
	first_type_xfer = type;
	select_op(dws, addr, dummy, flash_mode, type, cmd);
}

static inline void do_spi_send(struct sheipa_spi *dws, const u8 *buf,
				uint32_t len)
{
	uint32_t i;
	uint32_t cnt;
	uint32_t tmp;
	uint32_t data;
	uint32_t t;
	uint32_t addr;
	uint32_t dummy;
	uint32_t type;
	enum flash_mode_type flash_mode;
	uint8_t cmd;

	if (!mode) {
		if (len <= FIFO_HALF_SIZE) {
			cnt = len / 4;
			for (i = 0; i < cnt; i++) {
				memcpy(&data, buf, 4);
				buf += 4;
				flash_write_data(dws, data, DATA_WORD);
			}
			cnt = len % 4;
			if (cnt > 0) {
				memcpy(&data, buf, cnt);
				buf += cnt;
				flash_write_data(dws, data, DATA_WORD);
			}
			enable_cs_write(dws);
		} else {
			/* first transfer data (half fifo size) */
			cnt = FIFO_HALF_SIZE / 4;
			for (i = 0; i < cnt; i++) {
				memcpy(&data, buf, 4);
				buf += 4;
				flash_write_data(dws, data, DATA_WORD);
			}
			enable_cs_write(dws);
			/* record first transfer cmd and addr */
			addr = addr_offset;
			cmd = first_cmd_xfer;
			dummy = first_dummy_xfer;
			flash_mode = first_flash_mode_xfer;
			type = first_type_xfer;
			/* t is transfer data times,
			 * t = (len - first xfer data) / fifo half size
			 */
			t = (len - FIFO_HALF_SIZE) / FIFO_HALF_SIZE;
			/* addr = base addr + offset */
			addr = addr + FIFO_HALF_SIZE;
			while (t > 0) {
				flash_write_enable(dws);
				select_op(dws, addr, dummy, flash_mode, type, cmd);
				cnt = FIFO_HALF_SIZE / 4;
				for (i = 0; i < cnt; i++) {
					memcpy(&data, buf, 4);
					buf += 4;
					flash_write_data(dws, data, DATA_WORD);
				}
				enable_cs_write(dws);
				addr = addr +  FIFO_HALF_SIZE;
				t--;
			}
			tmp = len % FIFO_HALF_SIZE;
			if (tmp > 0) {
				flash_write_enable(dws);
				select_op(dws, addr, dummy, flash_mode, type, cmd);
				cnt = tmp / 4;
				for (i = 0; i < cnt; i++) {
					memcpy(&data, buf, 4);
					buf += 4;
					flash_write_data(dws, data, DATA_WORD);
				}
				cnt = tmp % 4;
				if (cnt > 0) {
					memcpy(&data, buf, cnt);
					buf += cnt;
					flash_write_data(dws, data, DATA_WORD);
				}
				enable_cs_write(dws);
			}
		}
	} else {
		auto_write(dws, buf, len, addr_offset);
	}
}

/* receive data from spi fifo */
static inline void do_spi_recv(struct sheipa_spi *dws, u8 *buf, uint32_t len)
{
	uint32_t i;
	uint32_t cnt;
	uint32_t tmp;
	uint32_t data;
	uint32_t t;
	uint32_t addr;
	uint32_t dummy;
	uint32_t type;
	enum flash_mode_type flash_mode;
	uint8_t cmd;

	if (!mode) {
		if (len <= FIFO_SIZE) {
			enable_cs_read(dws, len);
			cnt = len / 4;
			for (i = 0; i < cnt; i++) {
				data = spi_flash_getdr(dws, DR0, DATA_WORD);
				memcpy(buf, &data, 4);
				buf += 4;
			}
			cnt = len % 4;
			if (cnt > 0) {
				data = spi_flash_getdr(dws, DR0, DATA_WORD);
				memcpy(buf, &data, 4);
				buf += cnt;
			}
		} else {
			enable_cs_read(dws, FIFO_SIZE);
			cnt = FIFO_SIZE / 4;
			for (i = 0; i < cnt; i++) {
				data = spi_flash_getdr(dws, DR0, DATA_WORD);
				memcpy(buf, &data, 4);
				buf += 4;
			}
			/* record first transfer cmd and addr */
			addr = addr_offset;
			cmd  = first_cmd_xfer;
			dummy = first_dummy_xfer;
			flash_mode = first_flash_mode_xfer;
			type = first_type_xfer;
			/* t is transfer data times,
			 * t = (len - first xfer data) / fifo size
			 */
			t = (len - FIFO_SIZE) / FIFO_SIZE;
			addr = addr + FIFO_SIZE;
			while(t > 0) {
				select_op(dws, addr, dummy, flash_mode, type, cmd);
				enable_cs_read(dws, FIFO_SIZE);
				for (i = 0; i < cnt; i++) {
					data = spi_flash_getdr(dws, DR0, DATA_WORD);
					memcpy(buf, &data, 4);
					buf += 4;
				}
				addr = addr + FIFO_SIZE;
				t--;
			}
			tmp = len % FIFO_SIZE;
			if (tmp > 0) {
				select_op(dws, addr, dummy, flash_mode, type, cmd);
				enable_cs_read(dws, tmp);
				cnt = tmp / 4;
				for (i = 0; i < cnt; i++) {
					data = spi_flash_getdr(dws, DR0, DATA_WORD);
					memcpy(buf, &data, 4);
					buf += 4;
				}
				cnt = tmp % 4;
				if (cnt > 0) {
					data = spi_flash_getdr(dws, DR0, DATA_WORD);
					memcpy(buf, &data, 4);
					buf += cnt;
				}
			}
		}
	} else {
		auto_read(dws, buf, len, addr_offset);
	}
}

static int dw_spi_transfer(struct spi_master *master, struct spi_message *msg)
{
	struct spi_transfer *t;
	struct spi_device *spi;
	unsigned long flags;
	struct sheipa_spi *dws = spi_master_get_devdata(master);
	msg->actual_length = 0;
	spi = msg->spi;

	spin_lock_irqsave(&master->bus_lock_spinlock, flags);

	list_for_each_entry(t, &msg->transfers, transfer_list) {

		if (t->tx_buf) {
			/* first xfer is the cmd */
			if (msg->actual_length == 0)
				do_spi_cmd(dws, t->tx_buf, t->len);
			else
				do_spi_send(dws, t->tx_buf, t->len);
		}
		if (t->rx_buf)
			do_spi_recv(dws, t->rx_buf, t->len);

		msg->actual_length += t->len;
	}

	spin_unlock_irqrestore(&master->bus_lock_spinlock, flags);
	msg->status = 0;
	spi_finalize_current_message(master);
	return 0;
}

static int dw_spi_setup(struct spi_device *spi)
{
	struct sheipa_spi *dws = spi_master_get_devdata(spi->master);
	struct spi_flash_param ps_para= CC_DEFINE_SPI_FLASH_PARAMS(ps_);

	/* iniitialize Flash_Device_information */
	dws->comp_param = &ps_para;
	/* user mode init setting */
	flash_init(dws);
	/* auto mode init setting */
	auto_init(dws);
	return 0;
}

static int sheipa_spi_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct sheipa_spi *spi;
	struct resource *mem;
	struct resource *auto_mem;
	int status;

	master = spi_alloc_master(&pdev->dev, sizeof *spi);
	if (master == NULL) {
		dev_dbg(&pdev->dev, "master allocation failed\n");
		return -ENOMEM;
	}

	printk(KERN_INFO "INFO: sheipa spi probe\n");
	spi = spi_master_get_devdata(master);
	spi->master = master;

	master->mode_bits = SPI_CPHA | SPI_CPOL;;
	master->bus_num = 0;
	master->num_chipselect = 1;
	master->setup = dw_spi_setup;
	master->transfer_one_message = dw_spi_transfer;

	platform_set_drvdata(pdev, spi);
	/* for spi user mode */
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (mem == NULL) {
		status = -ENODEV;
		goto err_put_master;
	}

	spi->regs = ioremap_nocache(mem->start, resource_size(mem));
	if (!spi->regs) {
		status = -ENXIO;
		goto err_put_master;
	}
	/* for auto mode */
	auto_mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (mem == NULL) {
		status = -ENODEV;
		goto err_put_master;
	}

	spi->auto_regs = ioremap_nocache(auto_mem->start, resource_size(auto_mem));
	if (!spi->auto_regs) {
		status = -ENXIO;
		goto err_put_master;
	}
	status = spi_register_master(master);
	if (status < 0)
		goto err_unmap;
	return status;

err_unmap:
	iounmap(spi->regs);
err_put_master:
	spi_master_put(master);

	return status;
}

static int sheipa_spi_remove(struct platform_device *pdev)
{
	struct sheipa_spi *spi;
	struct resource *mem;

	spi = platform_get_drvdata(pdev);
	platform_set_drvdata(pdev, NULL);

	iounmap(spi->regs);
	spi_unregister_master(spi->master);
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(mem->start, resource_size(mem));

	return 0;
}

static struct platform_driver sheipa_spi_driver = {
	.probe		= sheipa_spi_probe,
	.remove		= sheipa_spi_remove,
	.driver		= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init sheipa_spi_init(void)
{
	printk(KERN_INFO "INFO: sheipa spi driver register\n");
	return platform_driver_probe(&sheipa_spi_driver, sheipa_spi_probe);
}
subsys_initcall(sheipa_spi_init);

static void __exit sheipa_spi_exit(void)
{
	platform_driver_unregister(&sheipa_spi_driver);
}
module_exit(sheipa_spi_exit);

MODULE_DESCRIPTION("Sheipa SPI controller driver");
MODULE_AUTHOR("Realtek PSP Group");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRIVER_NAME);
