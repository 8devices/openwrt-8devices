/*
 * Realtek MMC/SD/SDIO driver
 *
 * Authors:
 * Copyright (C) 2008-2013 Realtek Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/mbus.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/mmc/host.h>
#include <asm/unaligned.h>

#include <linux/sched.h>	//liao
#include <linux/wait.h>		//liao
#include <linux/slab.h>		//liao
#include <linux/semaphore.h>	//liao
#include <linux/mmc/card.h>	//liao
#include <linux/mmc/mmc.h>	//liao
#include <linux/mmc/sd.h>	//liao
#include <linux/workqueue.h>	//liao
#include <linux/completion.h>	//liao
//#include <rbus/reg_mmc.h>               //liao
//#include <rbus/reg_sys.h>               //liao
//#include <rbus/reg_iso.h>               //liao
#include "rtkemmc.h"		//liao
#include "rtksd_ops.h"		//liao
#include "../mmc_debug.h"	//liao

#ifdef MMC_DBG
static unsigned sd_reg = 0;
#endif
extern volatile unsigned char g_cmd[];

DECLARE_COMPLETION(rtk_emmc_wait);

#ifdef GPIO_DEBUG
void trans_db_gpio(void)
{
	//set debug GPIO
	u32 reginfo;
	//1. 0xB800_0804[31:28] = F    --> pin share as gpio
	//2. 0x1801_BC00[4] = 1  --> output mode
	//3. 0x1801_BC18[4]   is output data
	cr_writel(cr_readl(GP0DIR_reg) | 0x10, GP0DIR_reg);

	reginfo = cr_readl(GP0DATO_reg);
	if (reginfo & 0x10) {
		mmcrtk("GP HI\n");
		cr_writel(reginfo & ~0x10, GP0DATO_reg);
	} else {
		mmcrtk("GP LO\n");
		cr_writel(reginfo | 0x10, GP0DATO_reg);
	}

}

void trans_rst_gpio(void)
{
	//set rst GPIO
	u32 reginfo;
	cr_writel(cr_readl(GP0DIR_reg) | 0x00100000, GP0DIR_reg);
	reginfo = cr_readl(GP0DATO_reg);

	if (reginfo & 0x00100000) {
		cr_writel(reginfo & ~0x00100000, GP0DATO_reg);
	} else {
		cr_writel(reginfo | 0x00100000, GP0DATO_reg);
	}

}
#else
#define trans_db_gpio()
#define trans_rst_gpio()
#endif

void sync(void)
{
	//asm volatile("DMB");
	//cr_writel(0x0, 0x1801a020);
	//asm volatile("DMB");
	return;
}

EXPORT_SYMBOL_GPL(sync);

//#define INT_BLOCK_R_GAP 0x200
//#define INT_BLOCK_W_GAP 5
#define TIMEOUT_MS 3000
int rtkcr_wait_opt_end(char *drv_name, struct rtksd_host *sdport, u8 cmdcode,
		       u8 cpu_mode)
{
	volatile u8 sd_transfer_reg;
	volatile int loops = 0, dma_val = 0;
	volatile int err = CR_TRANS_OK;
	volatile unsigned long timeend = 0;
	volatile unsigned int sd_trans = 0;
	unsigned long flags;
	unsigned int dma_to = 0;

	MMCPRINTF
	    ("\n%s - info :\ncmd0 : 0x%02x,cmd1 : 0x%02x,cmd2 : 0x%02x,cmd3 : 0x%02x,cmd4: 0x%02x\n",
	     drv_name, g_cmd[0], g_cmd[1], g_cmd[2], g_cmd[3], g_cmd[4]);
	switch (cmdcode) {
	case 0x2:
	case 0x7:
	case 0x8:
		dma_to = TIMEOUT_MS;
		break;
	default:
		dma_to = 0;
		break;
	}
	err = rtk_int_enable_and_waitfor(sdport, cmdcode, TIMEOUT_MS, dma_to);
	if (err != 0)
		return err;
	sync();
	MMCPRINTF
	    ("\n%s - info1 :\ncmd0 : 0x%02x,cmd1 : 0x%02x,cmd2 : 0x%02x,cmd3 : 0x%02x,cmd4: 0x%02x\n",
	     drv_name, g_cmd[0], g_cmd[1], g_cmd[2], g_cmd[3], g_cmd[4]);
	err = CR_DMA_FAIL;
	timeend = jiffies + msecs_to_jiffies(TIMEOUT_MS);
	while (time_before(jiffies, timeend)) {
		sync();
		if (!(cr_readl(sdport->base + EMMC_DMA_CTL3) & DMA_XFER)) {
			err = 0;
			break;
		}
	}
	if (err) {
		printk(KERN_INFO
		       "\n%s - trans dma fail (cmd/2193/status1/status2/bus_status/cfg1/cfg2/cfg3/dma) : \n\t0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%02x 0x%02x 0x%02x 0x%08x\n",
		       __func__, g_cmd[0], cr_readb(sdport->base + SD_TRANSFER),
		       cr_readb(sdport->base + SD_STATUS1),
		       cr_readb(sdport->base + SD_STATUS2),
		       cr_readb(sdport->base + SD_BUS_STATUS),
		       cr_readb(sdport->base + SD_CONFIGURE1),
		       cr_readb(sdport->base + SD_CONFIGURE2),
		       cr_readb(sdport->base + SD_CONFIGURE3),
		       cr_readl(sdport->base + CR_DMA_CTL3));
		return err;
	}
	if (g_cmd[0] == 21) {
		cr_writel(DAT64_SEL | DDR_WR, sdport->base + CR_DMA_CTL3);
		sync();
		err = CR_TRANSFER_TO;
		timeend = jiffies + msecs_to_jiffies(TIMEOUT_MS);
		while (time_before(jiffies, timeend)) {
			sd_transfer_reg = cr_readb(sdport->base + SD_TRANSFER);
			sync();
			if ((sd_transfer_reg & (END_STATE | IDLE_STATE)) ==
			    (END_STATE | IDLE_STATE)) {
				err = 0;
				break;
			}
		}
		if (err) {
			MMCPRINTF(KERN_INFO
				  "\n%s - cmd21 dma fail0 (cmd/2193/status1/status2/bus_status/cfg1/cfg2/cfg3/dma) : \n\t0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%02x 0x%02x 0x%02x 0x%08x\n",
				  drv_name, g_cmd[0],
				  cr_readb(sdport->base + SD_TRANSFER),
				  cr_readb(sdport->base + SD_STATUS1),
				  cr_readb(sdport->base + SD_STATUS2),
				  cr_readb(sdport->base + SD_BUS_STATUS),
				  cr_readb(sdport->base + SD_CONFIGURE1),
				  cr_readb(sdport->base + SD_CONFIGURE2),
				  cr_readb(sdport->base + SD_CONFIGURE3),
				  cr_readl(sdport->base + CR_DMA_CTL3));
			return err;
		}
		sync();
		err = CR_DMA_FAIL;
		timeend = jiffies + msecs_to_jiffies(TIMEOUT_MS);
		while (time_before(jiffies, timeend)) {
			dma_val = cr_readl(sdport->base + CR_DMA_CTL3);
			sync();
			if ((dma_val & DMA_XFER) != DMA_XFER) {
				return 0;
			}
		}
		if (err) {
			MMCPRINTF(KERN_INFO
				  "\n%s - cmd21 dma fail1 (cmd/2193/status1/status2/bus_status/cfg1/cfg2/cfg3/dma) : \n\t0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%02x 0x%02x 0x%02x 0x%08x\n",
				  drv_name, g_cmd[0],
				  cr_readb(sdport->base + SD_TRANSFER),
				  cr_readb(sdport->base + SD_STATUS1),
				  cr_readb(sdport->base + SD_STATUS2),
				  cr_readb(sdport->base + SD_BUS_STATUS),
				  cr_readb(sdport->base + SD_CONFIGURE1),
				  cr_readb(sdport->base + SD_CONFIGURE2),
				  cr_readb(sdport->base + SD_CONFIGURE3),
				  cr_readl(sdport->base + CR_DMA_CTL3));
			return err;
		}
	}
	return err;
}

EXPORT_SYMBOL_GPL(rtkcr_wait_opt_end);

int rtk_int_enable_and_waitfor(struct rtksd_host *sdport, u8 cmdcode,
			       unsigned long msec, unsigned long dma_msec)
{
	unsigned long timeend = 0;

	sdport->int_status = 0;
	sdport->sd_trans = -1;
	sdport->sd_status1 = 0;
	sdport->sd_status2 = 0;
	sdport->bus_status = 0;
	sdport->dma_trans = 0;

#ifndef ENABLE_EMMC_INT_MODE
	sdport->int_waiting = NULL;
#else
	sdport->int_waiting = &rtk_emmc_wait;
	MMCPRINTF("rtk wait complete addr = %08x\n",
		  (unsigned int)sdport->int_waiting);
	/* timeout timer fire */
	if (&sdport->timer) {
		MMCPRINTF("timer started : ");
#if 0
		if (sdport->tmout) {
			timeend = sdport->tmout;
			sdport->tmout = 0;
		} else
#endif
			timeend = msecs_to_jiffies(msec) + sdport->tmout;

		MMCPRINTF("TO = 0x%08x\n", timeend);
		MMCPRINTF("remove mod_timer temply\n", timeend);
		mod_timer(&sdport->timer, (jiffies + timeend));
	}
	//wait for
#endif
	rtk_int_waitfor(sdport, cmdcode, msec, dma_msec);
	//smp_wmb();
	sync();
	if (sdport->sd_trans & ERR_STATUS)	//transfer error
	{
		printk(KERN_INFO
		       "\n - trans error (cmd/SD_TRANS/status1/status2/bus_status/cfg1/cfg2/cfg3) : \n\t0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%02x 0x%02x 0x%02x\n",
		       g_cmd[0], cr_readb(sdport->base + SD_TRANSFER),
		       cr_readb(sdport->base + SD_STATUS1),
		       cr_readb(sdport->base + SD_STATUS2),
		       cr_readb(sdport->base + SD_BUS_STATUS),
		       cr_readb(sdport->base + SD_CONFIGURE1),
		       cr_readb(sdport->base + SD_CONFIGURE2),
		       cr_readb(sdport->base + SD_CONFIGURE3));
		return CR_TRANSFER_FAIL;
	}
	if ((sdport->sd_trans & (END_STATE | IDLE_STATE)) != (END_STATE | IDLE_STATE))	//transfer error
	{
		printk(KERN_INFO
		       "\n - trans timeout (cmd/SD_TRANS/status1/status2/bus_status/cfg1/cfg2/cfg3) : \n\t0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%02x 0x%02x 0x%02x\n",
		       g_cmd[0], cr_readb(sdport->base + SD_TRANSFER),
		       cr_readb(sdport->base + SD_STATUS1),
		       cr_readb(sdport->base + SD_STATUS2),
		       cr_readb(sdport->base + SD_BUS_STATUS),
		       cr_readb(sdport->base + SD_CONFIGURE1),
		       cr_readb(sdport->base + SD_CONFIGURE2),
		       cr_readb(sdport->base + SD_CONFIGURE3));
		return CR_TRANSFER_FAIL;
	}
	return 0;
}

EXPORT_SYMBOL_GPL(rtk_int_enable_and_waitfor);

void rtk_int_waitfor(struct rtksd_host *sdport, u8 cmdcode, unsigned long msec,
		     unsigned long dma_msec)
{
#ifndef ENABLE_EMMC_INT_MODE
	unsigned long timeend = 0;
	u32 sd_trans = 0, sd_status1 = 0, sd_status2 = 0, bus_status =
	    0, dma_trans = 0;
	unsigned long flags;

	spin_lock_irqsave(&sdport->lock, flags);

	if (sdport->rtflags & RTKCR_FOPEN_LOG) {
		MMCPRINTF(" rtkemmc : register settings(base=0x%08x,0x%08x)\n",
			  sdport->base, sdport->base + SD_CMD0);
		MMCPRINTF
		    (" cmd0:0x%02x cmd1:0x%02x cmd2:0x%02x cmd3:0x%02x cmd4:0x%02x cmd5:0x%02x\n",
		     cr_readb(sdport->base + SD_CMD0),
		     cr_readb(sdport->base + SD_CMD1),
		     cr_readb(sdport->base + SD_CMD2),
		     cr_readb(sdport->base + SD_CMD3),
		     cr_readb(sdport->base + SD_CMD4),
		     cr_readb(sdport->base + SD_CMD5));
		MMCPRINTF
		    (" trans:0x%02x status1:0x%02x status2:0x%02x bus_status:0x%02x\n",
		     cr_readb(sdport->base + SD_TRANSFER),
		     cr_readb(sdport->base + SD_STATUS1),
		     cr_readb(sdport->base + SD_STATUS2),
		     cr_readb(sdport->base + SD_BUS_STATUS));
		MMCPRINTF
		    (" configure1:0x%02x configure2:0x%02x configure3:0x%02x\n",
		     cr_readb(sdport->base + SD_CONFIGURE1),
		     cr_readb(sdport->base + SD_CONFIGURE2),
		     cr_readb(sdport->base + SD_CONFIGURE3));
		MMCPRINTF
		    (" byteH:0x%02x byteL:0x%02x blkH:0x%02x blkL:0x%02x\n",
		     cr_readb(sdport->base + SD_BYTE_CNT_H),
		     cr_readb(sdport->base + SD_BYTE_CNT_L),
		     cr_readb(sdport->base + SD_BLOCK_CNT_H),
		     cr_readb(sdport->base + SD_BLOCK_CNT_L));
		MMCPRINTF
		    (" CPU_ACC:0x%08x dma_ctl1:0x%08x dma_ctl2:0x%08x dma_ctl3:0x%08x\n",
		     cr_readl(sdport->base + EMMC_CPU_ACC),
		     cr_readl(sdport->base + CR_DMA_CTL1),
		     cr_readl(sdport->base + CR_DMA_CTL2),
		     cr_readl(sdport->base + CR_DMA_CTL3));
		MMCPRINTF
		    (" card_pad_drv:0x%08x cmd_pad_drv:0x%08x data_pad_drv:0x%08x EMMC_CKGEN_CTL:0x%08x, SYS_PLL_EMMC3=0x%08x\n",
		     cr_readb(sdport->base + EMMC_CARD_PAD_DRV),
		     cr_readb(sdport->base + EMMC_CMD_PAD_DRV),
		     cr_readb(sdport->base + EMMC_DATA_PAD_DRV),
		     cr_readl(sdport->base + EMMC_CKGEN_CTL),
		     cr_readl(SYS_PLL_EMMC3));
	}
	//cmd fire
#ifdef MMC_DBG
	sd_reg = cr_readb(sdport->base + SD_CMD0);
#endif
	sync();
	cr_writeb((u8) (cmdcode | START_EN), sdport->base + SD_TRANSFER);
	sync();

	timeend = jiffies + msecs_to_jiffies(msec);
	while (time_before(jiffies, timeend)) {
		if ((cr_readb(sdport->base + SD_TRANSFER) &
		     (END_STATE | IDLE_STATE)) == (END_STATE | IDLE_STATE))
			break;
		if ((cr_readb(sdport->base + SD_TRANSFER) & ERR_STATUS) ==
		    (ERR_STATUS))
			break;
	}

	if (dma_msec > 0) {
		timeend = jiffies + msecs_to_jiffies(dma_msec);
		while (time_before(jiffies, timeend)) {
			if ((cr_readb(sdport->base + CR_DMA_CTL3) & DMA_XFER) !=
			    (DMA_XFER))
				break;
		}
		rtkcr_get_dma_trans(sdport->base, &dma_trans);
		sdport->dma_trans = dma_trans;
	}
	MMCPRINTF("exit from polling\n");

	rtkcr_get_sd_trans(sdport->base, &sd_trans);
	rtkcr_get_sd_sta(sdport->base, &sd_status1, &sd_status2, &bus_status);

	sdport->sd_trans = sd_trans;
	sdport->sd_status1 = sd_status1;
	sdport->sd_status2 = sd_status2;
	sdport->bus_status = bus_status;
	MMCPRINTF("int sts : 0x%08x sd_trans : 0x%08x, sd_st1 : 0x%08x\n",
		  sdport->int_status, sdport->sd_trans, sdport->sd_status1);
	MMCPRINTF("int st2 : 0x%08x bus_sts : 0x%08x dma_trans : 0x%08x\n",
		  sdport->sd_status2, sdport->bus_status, sdport->dma_trans);
	spin_unlock_irqrestore(&sdport->lock, flags);
#else
	sync();
	if (sdport->rtflags & RTKCR_FOPEN_LOG) {
		//if ((g_cmd[0] == 0x47)||(g_cmd[0] == 0x45)||(g_cmd[0] == 0x46)){
		printk(" rtkemmc : register settings(base=0x%08x,0x%02x)\n",
		       sdport->base, cr_readb(sdport->base + SD_CMD0));
		printk
		    (" cmd0:0x%02x cmd1:0x%02x cmd2:0x%02x cmd3:0x%02x cmd4:0x%02x cmd5:0x%02x\n",
		     cr_readb(sdport->base + SD_CMD0),
		     cr_readb(sdport->base + SD_CMD1),
		     cr_readb(sdport->base + SD_CMD2),
		     cr_readb(sdport->base + SD_CMD3),
		     cr_readb(sdport->base + SD_CMD4),
		     cr_readb(sdport->base + SD_CMD5));
		printk
		    (" trans:0x%02x status1:0x%02x status2:0x%02x bus_status:0x%02x\n",
		     cr_readb(sdport->base + SD_TRANSFER),
		     cr_readb(sdport->base + SD_STATUS1),
		     cr_readb(sdport->base + SD_STATUS2),
		     cr_readb(sdport->base + SD_BUS_STATUS));
		printk
		    (" configure1:0x%02x configure2:0x%02x configure3:0x%02x\n",
		     cr_readb(sdport->base + SD_CONFIGURE1),
		     cr_readb(sdport->base + SD_CONFIGURE2),
		     cr_readb(sdport->base + SD_CONFIGURE3));
		printk(" byteH:0x%02x byteL:0x%02x blkH:0x%02x blkL:0x%02x\n",
		       cr_readb(sdport->base + SD_BYTE_CNT_H),
		       cr_readb(sdport->base + SD_BYTE_CNT_L),
		       cr_readb(sdport->base + SD_BLOCK_CNT_H),
		       cr_readb(sdport->base + SD_BLOCK_CNT_L));
		printk
		    (" CPU_ACC:0x%08x dma_ctl1:0x%08x dma_ctl2:0x%08x dma_ctl3:0x%08x sample_ctl:0x%08x pull_ctl:0x%08x\n",
		     cr_readl(sdport->base + EMMC_CPU_ACC),
		     cr_readl(sdport->base + CR_DMA_CTL1),
		     cr_readl(sdport->base + CR_DMA_CTL2),
		     cr_readl(sdport->base + CR_DMA_CTL3),
		     cr_readb(sdport->base + SD_SAMPLE_POINT_CTL),
		     cr_readb(sdport->base + SD_PUSH_POINT_CTL));
		printk
		    (" card_pad_drv:0x%08x cmd_pad_drv:0x%08x data_pad_drv:0x%08x EMMC_CKGEN_CTL:0x%08x, SYS_PLL_EMMC3=0x%08x\n",
		     cr_readb(sdport->base + EMMC_CARD_PAD_DRV),
		     cr_readb(sdport->base + EMMC_CMD_PAD_DRV),
		     cr_readb(sdport->base + EMMC_DATA_PAD_DRV),
		     cr_readl(sdport->base + EMMC_CKGEN_CTL),
		     cr_readl(SYS_PLL_EMMC3));
	}

	if (sdport->int_waiting) {
#ifdef MMC_DBG
		sd_reg = cr_readb(sdport->base + SD_CMD0);
		MMCPRINTF("========== S(0x%02x) ==========\n", sd_reg);
#endif
		rtkcr_hold_int_dec(sdport->base);
		rtkcr_clr_int_sta(sdport->base);
		rtkcr_en_int(sdport->base);
		rtkcr_clr_int_sta(sdport->base);

		sync();
		//cmd fire
		cr_writeb((u8) (cmdcode | START_EN),
			  sdport->base + SD_TRANSFER);
		sync();

		MMCPRINTF("do wait for 1\n");
		wait_for_completion(sdport->int_waiting);
		MMCPRINTF("do wait for 2\n");
	}
#endif
}

EXPORT_SYMBOL_GPL(rtk_int_waitfor);

void rtk_op_complete(struct rtksd_host *sdport)
{
	if (sdport->int_waiting) {
		MMCPRINTF("int wait complete 1\n");
		MMCPRINTF("1 rtk wait complete addr = %08x\n",
			  (unsigned int)sdport->int_waiting);
		struct completion *waiting = sdport->int_waiting;
		//sdport->int_waiting = NULL;
		complete(waiting);
		MMCPRINTF("========== D ==========\n");
		sync();
		MMCPRINTF("int wait complete 2\n");
	} else
		MMCPRINTF("int wait not complete\n");
}

EXPORT_SYMBOL_GPL(rtk_op_complete);

char *rtkcr_parse_token(const char *parsed_string, const char *token)
{
	const char *ptr = parsed_string;
	const char *start, *end, *value_start, *value_end;
	char *ret_str;

	while (1) {
		value_start = value_end = 0;
		for (; *ptr == ' ' || *ptr == '\t'; ptr++) ;
		if (*ptr == '\0')
			break;
		start = ptr;
		for (;
		     *ptr != ' ' && *ptr != '\t' && *ptr != '=' && *ptr != '\0';
		     ptr++) ;
		end = ptr;
		if (*ptr == '=') {
			ptr++;
			if (*ptr == '"') {
				ptr++;
				value_start = ptr;
				for (; *ptr != '"' && *ptr != '\0'; ptr++) ;
				if (*ptr != '"'
				    || (*(ptr + 1) != '\0' && *(ptr + 1) != ' '
					&& *(ptr + 1) != '\t')) {
					printk
					    ("system_parameters error! Check your parameters     .");
					break;
				}
			} else {
				value_start = ptr;
				for (;
				     *ptr != ' ' && *ptr != '\t' && *ptr != '\0'
				     && *ptr != '"'; ptr++) ;
				if (*ptr == '"') {
					printk
					    ("system_parameters error! Check your parameters.");
					break;
				}
			}
			value_end = ptr;
		}

		if (!strncmp(token, start, end - start)) {
			if (value_start) {
				ret_str =
				    kmalloc(value_end - value_start + 1,
					    GFP_KERNEL);
				// KWarning: checked ok by alexkh@realtek.com
				if (ret_str) {
					strncpy(ret_str, value_start,
						value_end - value_start);
					ret_str[value_end - value_start] = '\0';
				}
				return ret_str;
			} else {
				ret_str = kmalloc(1, GFP_KERNEL);
				// KWarning: checked ok by alexkh@realtek.com
				if (ret_str)
					strcpy(ret_str, "");
				return ret_str;
			}
		}
	}

	return (char *)NULL;
}

EXPORT_SYMBOL_GPL(rtkcr_parse_token);

void rtkcr_chk_param(u32 * pparam, u32 len, u8 * ptr)
{
	u32 value, i;
	mmcrtk("\n");

	*pparam = 0;
	for (i = 0; i < len; i++) {
		value = ptr[i] - '0';
		// KWarning: checked ok by alexkh@realtek.com
		if ((value >= 0) && (value <= 9)) {
			*pparam += value << (4 * (len - 1 - i));
			continue;
		}

		value = ptr[i] - 'a';
		// KWarning: checked ok by alexkh@realtek.com
		if ((value >= 0) && (value <= 5)) {
			value += 10;
			*pparam += value << (4 * (len - 1 - i));
			continue;
		}

		value = ptr[i] - 'A';
		// KWarning: checked ok by alexkh@realtek.com
		if ((value >= 0) && (value <= 5)) {
			value += 10;
			*pparam += value << (4 * (len - 1 - i));
			continue;
		}
	}
}

EXPORT_SYMBOL_GPL(rtkcr_chk_param);

u32 verA_magic_num = 0;
int rtkcr_chk_VerA(void)
{
	mmcrtk("\n");
//TODO : chk
#if 0
	if (!verA_magic_num)
		verA_magic_num = cr_readl(0xb8060000);

	if (verA_magic_num == 0x62270000) {
		return 1;
	} else {
		return 0;
	}
#else
	return verA_magic_num;	//0: polling, 1: interrupt
#endif
}

EXPORT_SYMBOL_GPL(rtkcr_chk_VerA);

void emmc_show_config123(struct rtksd_host *sdport)
{
	u32 reginfo;
	u32 clksel_sht;
	u32 iobase = sdport->base;

	if (iobase == EM_BASE_ADDR) {
		reginfo = cr_readl(iobase + EMMC_CKGEN_CTL);
		//clksel_sht = EMMC_CLKSEL_SHT;
	} else if (iobase == CR_BASE_ADDR) {
		reginfo = cr_readl(iobase + CR_SD_CKGEN_CTL);
		//clksel_sht = SD_CLKSEL_SHT;
	}

	printk("CFG1=0x%x CFG2=0x%x CFG3=0x%x bus clock CKGEN=%08x\n",
	       cr_readb(iobase + SD_CONFIGURE1),
	       cr_readb(iobase + SD_CONFIGURE2),
	       cr_readb(iobase + SD_CONFIGURE3), reginfo);
}

EXPORT_SYMBOL_GPL(emmc_show_config123);
/* end of file */
