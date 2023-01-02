/*
 * Copyright (c) 2015-2016, 2018, 2020 The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _DEVSOC_CLK_H
#define _DEVSOC_CLK_H

#include <asm/arch-qca-common/uart.h>

/*
 * UART registers
 */
#define GCC_BLSP1_UART1_BCR			0x1802028
#define GCC_BLSP1_UART2_BCR			0x1803028
#define GCC_BLSP1_UART3_BCR			0x1804028

#define GCC_BLSP1_UART_BCR(id)	((id < 1) ? \
				(GCC_BLSP1_UART1_BCR):\
				(GCC_BLSP1_UART1_BCR + (0x1000 * id)))

#define GCC_BLSP1_UART_APPS_CMD_RCGR(id)	(GCC_BLSP1_UART_BCR(id) + 0x04)
#define GCC_BLSP1_UART_APPS_CFG_RCGR(id)	(GCC_BLSP1_UART_BCR(id) + 0x08)
#define GCC_BLSP1_UART_APPS_M(id)		(GCC_BLSP1_UART_BCR(id) + 0x0c)
#define GCC_BLSP1_UART_APPS_N(id)		(GCC_BLSP1_UART_BCR(id) + 0x10)
#define GCC_BLSP1_UART_APPS_D(id)		(GCC_BLSP1_UART_BCR(id) + 0x14)
#define GCC_BLSP1_UART_APPS_CBCR(id)		(GCC_BLSP1_UART_BCR(id) + 0x18)

#define GCC_UART_CFG_RCGR_MODE_MASK		0x3000
#define GCC_UART_CFG_RCGR_SRCSEL_MASK		0x0700
#define GCC_UART_CFG_RCGR_SRCDIV_MASK		0x001F

#define GCC_UART_CFG_RCGR_MODE_SHIFT		12
#define GCC_UART_CFG_RCGR_SRCSEL_SHIFT		8
#define GCC_UART_CFG_RCGR_SRCDIV_SHIFT		0

#define UART_RCGR_SRC_SEL			0x1
#define UART_RCGR_SRC_DIV			0x0
#define UART_RCGR_MODE				0x2
#define UART_CMD_RCGR_UPDATE			0x1
#define UART_CMD_RCGR_ROOT_EN			0x2
#define UART_CBCR_CLK_ENABLE			0x1

#define NOT_2D(two_d)				(~two_d)
#define NOT_N_MINUS_M(n,m)			(~(n - m))
#define CLOCK_UPDATE_TIMEOUT_US			1000

#define CMD_UPDATE      			0x1
#define ROOT_EN         			0x2
#define CLK_ENABLE      			0x1

int uart_clock_config(struct ipq_serial_platdata *plat);

#endif /*IPQ9574_CLK_H*/
