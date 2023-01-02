/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file ppe_drv_port.h
 *	NSS PPE driver definitions.
 */

#ifndef _PPE_DRV_PORT_H_
#define _PPE_DRV_PORT_H_

/*
 * PPE port defines
 */
#define PPE_DRV_PHYSICAL_START	0	/* Physical port start with port 0 */
#define PPE_DRV_PHYSICAL_MAX	8	/* PPE supports 8 physical ports 0-7 */
#define PPE_DRV_VIRTUAL_MAX	192	/* PPE supports 192 virtual interfaces 64-255 */
#define PPE_DRV_VIRTUAL_START	64	/* Virtual ports start at 64 */
#define PPE_DRV_PORTS_MAX	256	/* Total ports in PPE Physical + Trunk + Virtual */

#define PPE_DRV_PORT_CPU	0	/* PPE egress port to reach CPUs */
#define PPE_DRV_PORT_EIP197	7	/* PPE egress port to reach EIP197 */

#define PPE_DRV_PORT_JUMBO_MAX	9216	/* Suggested value is 9K, but can be increased upto 10K */

typedef int32_t ppe_drv_port_t;

struct ppe_drv;
struct ppe_drv_vsi;
struct ppe_drv_l3_if;

/*
 * ppe_drv_port_type
 *	PPE Port types - values derived from HW spec
 */
enum ppe_drv_port_type {
	PPE_DRV_PORT_PHYSICAL,		/* Physical Port */
	PPE_DRV_PORT_LAG,		/* LAG Port */
	PPE_DRV_PORT_VIRTUAL,		/* Virtual Port */
	PPE_DRV_PORT_EIP,		/* EIP inline Port */
};

/*
 * ppe_drv_port_qos_res_pre
 *	PPE Port QOS resolution precedence
 *
 * QOS resolution precedence across different classfication engines
 * This ranges from 0-7, 0,7 are reserved.
 */
enum ppe_drv_port_qos_res_pre {
	PPE_DRV_PORT_QOS_RES_PREC_0_RESERVED,
	PPE_DRV_PORT_QOS_RES_PREC_1,
	PPE_DRV_PORT_QOS_RES_PREC_2,
	PPE_DRV_PORT_QOS_RES_PREC_3,
	PPE_DRV_PORT_QOS_RES_PREC_4,
	PPE_DRV_PORT_QOS_RES_PREC_5,
	PPE_DRV_PORT_QOS_RES_PREC_6,
	PPE_DRV_PORT_QOS_RES_PREC_7_RESERVED,
};

/**
 * ppe_drv_port_num_from_dev
 *	Get port index from device.
 *
 * @datatypes
 * net_device
 *
 * @param[in] dev  Net device.
 *
 * @return
 * -1 for failure else port_index.
 */
int32_t ppe_drv_port_num_from_dev(struct net_device *dev);

/**
 * ppe_drv_port_num_to_dev
 *	Port number to device.
 *
 * @param[in] port  Port number.
 *
 * @return
 * net_device.
 */
struct net_device *ppe_drv_port_num_to_dev(uint8_t port);

#endif /* _PPE_DRV_PORT_H_ */
