/*
 *  Copyright (C) 2010 Realtek Semiconductors, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __RTKSD_OPS_H
#define __RTKSD_OPS_H
#include <linux/completion.h>

void sync();
int rtkcr_wait_opt_end(char *, struct rtksd_host *sdport, unsigned char cmdcode,
		       unsigned char cpu_mode);
void rtk_op_complete(struct rtksd_host *sdport);
char *rtkcr_parse_token(const char *parsed_string, const char *token);
void rtkcr_chk_param(u32 * pparam, u32 len, u8 * ptr);
int rtkcr_chk_VerA(void);
void emmc_show_config123(struct rtksd_host *sdport);
void rtkcr_set_mis_gpio(u32 gpio_num, u8 dir, u8 level);
void rtkcr_set_iso_gpio(u32 gpio_num, u8 dir, u8 level);

int mmc_fast_write(unsigned int blk_addr,
		   unsigned int data_size, unsigned char *buffer);

int mmc_fast_read(unsigned int blk_addr,
		  unsigned int data_size, unsigned char *buffer);
struct completion *rtk_int_enable(struct rtksd_host *sdport,
				  unsigned long msec);
int rtk_int_enable_and_waitfor(struct rtksd_host *sdport, u8 cmdcode,
			       unsigned long msec, unsigned long dma_msec);
void rtk_int_waitfor(struct rtksd_host *sdport, u8 cmdcode, unsigned long msec,
		     unsigned long dma_msec);

#endif

/* end of file */
