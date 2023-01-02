/*
 **************************************************************************
 * Copyright (c) 2016-2019, 2021, The Linux Foundation. All rights reserved.
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
 **************************************************************************
*/

#include <common.h>
#include <net.h>
#include <asm-generic/errno.h>
#include <asm/io.h>
#include <malloc.h>
#include <phy.h>
#include <net.h>
#include <miiphy.h>

#define PORT1					1
#define PORT2					2
#define PORT3					3
#define PORT4					4
#define PORT5					5
#define PORT6					6

#define IPQ9574_PORT5_MUX_PCS_UNIPHY0		0x0
#define IPQ9574_PORT5_MUX_PCS_UNIPHY1		0x1

#define PORT_GMAC_TYPE				1
#define PORT_XGMAC_TYPE				2

#define IPQ9574_PORT_MUX_MAC_TYPE		0
#define IPQ9574_PORT_MUX_XMAC_TYPE		1

struct port_mux_ctrl {
	uint32_t port1_pcs_sel:1;
	uint32_t port2_pcs_sel:1;
	uint32_t port3_pcs_sel:1;
	uint32_t port4_pcs_sel:1;
	uint32_t port5_pcs_sel:1;
	uint32_t port6_pcs_sel:1;
	uint32_t _reserved0:2;
	uint32_t port1_mac_sel:1;
	uint32_t port2_mac_sel:1;
	uint32_t port3_mac_sel:1;
	uint32_t port4_mac_sel:1;
	uint32_t port5_mac_sel:1;
	uint32_t port6_mac_sel:1;
	uint32_t _reserved1:18;
};

union port_mux_ctrl_u {
	uint32_t val;
	struct port_mux_ctrl bf;
};

enum {
	TCP_PKT,
	UDP_PKT,
};

#define ADPT_ACL_HPPE_IPV4_DIP_RULE	4
#define ADPT_ACL_HPPE_MAC_SA_RULE	1
#define ADPT_ACL_HPPE_MAC_DA_RULE	0
#define MAX_RULE			512

struct ipo_rule_reg {
        uint32_t  rule_field_0:32;
        uint32_t  rule_field_1:20;
        uint32_t  fake_mac_header:1;
        uint32_t  range_en:1;
        uint32_t  inverse_en:1;
        uint32_t  rule_type:5;
        uint32_t  src_type:3;
        uint32_t  src_0:1;
        uint32_t  src_1:7;
        uint32_t  pri:9;
        uint32_t  res_chain:1;
        uint32_t  post_routing_en:1;
        uint32_t  _reserved0:14;
};

union ipo_rule_reg_u {
        uint32_t val[3];
        struct ipo_rule_reg bf;
};

struct ipo_mask_reg {
        uint32_t  maskfield_0:32;
        uint32_t  maskfield_1:21;
        uint32_t  _reserved0:11;
};

union ipo_mask_reg_u {
        uint32_t val[2];
        struct ipo_mask_reg bf;
};

struct ipo_action {
        uint32_t  dest_info_change_en:1;
	uint32_t  fwd_cmd:2;
	uint32_t  _reserved0:15;
	uint32_t bypass_bitmap_0:14;
	uint32_t bypass_bitmap_1:18;
	uint32_t  _reserved1:14;
	uint32_t  _reserved2:32;
	uint32_t  _reserved3:32;
	uint32_t  _reserved4:32;
};

union ipo_action_u {
        uint32_t val[5];
        struct ipo_action bf;
};

#define IPQ9574_PORT_MUX_CTRL			0x10
#define IPQ9574_PORT_MUX_CTRL_NUM		1
#define IPQ9574_PORT_MUX_CTRL_INC		0x4
#define IPQ9574_PORT_MUX_CTRL_DEFAULT		0x0

#define PORT_PHY_STATUS_ADDRESS			0x44
#define PORT_PHY_STATUS_PORT5_1_OFFSET		8
#define PORT_PHY_STATUS_PORT6_OFFSET		16

#define IPQ9574_PPE_IPE_L3_BASE_ADDR		0x200000
#define IPQ9574_PPE_L3_VP_PORT_TBL_ADDR		(IPQ9574_PPE_IPE_L3_BASE_ADDR + 0x4000)
#define IPQ9574_PPE_L3_VP_PORT_TBL_INC		0x10

#define IPQ9574_PPE_TL_PORT_VP_TBL_ADDR		0x302000
#define IPQ9574_PPE_MRU_MTU_CTRL_TBL_ADDR	0x65000
#define IPQ9574_PPE_MC_MTU_CTRL_TBL_ADDR	0x60a00
#define IPQ9574_PPE_PORT_EG_VLAN_TBL_ADDR	0x20020

#define IPQ9574_PPE_UCAST_QUEUE_AC_EN_BASE_ADDR 0x848000
#define IPQ9574_PPE_MCAST_QUEUE_AC_EN_BASE_ADDR 0x84a000
#define IPQ9574_PPE_QUEUE_MANAGER_BASE_ADDR	0x800000
#define IPQ9574_PPE_UCAST_QUEUE_MAP_TBL_ADDR	0x10000
#define IPQ9574_PPE_UCAST_QUEUE_MAP_TBL_INC	0x10
#define IPQ9574_PPE_QM_UQM_TBL			(IPQ9574_PPE_QUEUE_MANAGER_BASE_ADDR +\
					 IPQ9574_PPE_UCAST_QUEUE_MAP_TBL_ADDR)
#define IPQ9574_PPE_UCAST_PRIORITY_MAP_TBL_ADDR	0x42000
#define IPQ9574_PPE_QM_UPM_TBL			(IPQ9574_PPE_QUEUE_MANAGER_BASE_ADDR +\
					 IPQ9574_PPE_UCAST_PRIORITY_MAP_TBL_ADDR)

#define IPQ9574_PPE_STP_BASE			0x060100
#define IPQ9574_PPE_MAC_ENABLE			0x001000
#define IPQ9574_PPE_MAC_SPEED			0x001004
#define IPQ9574_PPE_MAC_MIB_CTL			0x001034

#define IPQ9574_PPE_TRAFFIC_MANAGER_BASE_ADDR	0x400000
#define IPQ9574_PPE_TM_SHP_CFG_L0_OFFSET	0x00000030
#define IPQ9574_PPE_TM_SHP_CFG_L1_OFFSET	0x00000034
#define IPQ9574_PPE_TM_SHP_CFG_L0		IPQ9574_PPE_TRAFFIC_MANAGER_BASE_ADDR +\
						IPQ9574_PPE_TM_SHP_CFG_L0_OFFSET
#define IPQ9574_PPE_TM_SHP_CFG_L1		IPQ9574_PPE_TRAFFIC_MANAGER_BASE_ADDR +\
						IPQ9574_PPE_TM_SHP_CFG_L1_OFFSET

#define IPQ9574_PPE_L0_FLOW_PORT_MAP_TBL_ADDR	0x10000
#define IPQ9574_PPE_L0_FLOW_PORT_MAP_TBL_INC	0x10
#define IPQ9574_PPE_L0_FLOW_PORT_MAP_TBL	(IPQ9574_PPE_TRAFFIC_MANAGER_BASE_ADDR +\
					 IPQ9574_PPE_L0_FLOW_PORT_MAP_TBL_ADDR)

#define IPQ9574_PPE_L0_FLOW_MAP_TBL_ADDR	0x2000
#define IPQ9574_PPE_L0_FLOW_MAP_TBL_INC		0x10
#define IPQ9574_PPE_L0_FLOW_MAP_TBL		(IPQ9574_PPE_TRAFFIC_MANAGER_BASE_ADDR +\
					 IPQ9574_PPE_L0_FLOW_MAP_TBL_ADDR)

#define IPQ9574_PPE_L1_FLOW_PORT_MAP_TBL_ADDR	0x46000
#define IPQ9574_PPE_L1_FLOW_PORT_MAP_TBL_INC	0x10
#define IPQ9574_PPE_L1_FLOW_PORT_MAP_TBL	(IPQ9574_PPE_TRAFFIC_MANAGER_BASE_ADDR +\
					 IPQ9574_PPE_L1_FLOW_PORT_MAP_TBL_ADDR)

#define IPQ9574_PPE_L1_FLOW_MAP_TBL_ADDR	0x40000
#define IPQ9574_PPE_L1_FLOW_MAP_TBL_INC		0x10
#define IPQ9574_PPE_L1_FLOW_MAP_TBL		(IPQ9574_PPE_TRAFFIC_MANAGER_BASE_ADDR +\
					 IPQ9574_PPE_L1_FLOW_MAP_TBL_ADDR)

#define IPQ9574_PPE_L0_C_SP_CFG_TBL_ADDR	0x4000
#define IPQ9574_PPE_L0_C_SP_CFG_TBL		(IPQ9574_PPE_TRAFFIC_MANAGER_BASE_ADDR +\
					 IPQ9574_PPE_L0_C_SP_CFG_TBL_ADDR)

#define IPQ9574_PPE_L1_C_SP_CFG_TBL_ADDR	0x42000
#define IPQ9574_PPE_L1_C_SP_CFG_TBL		(IPQ9574_PPE_TRAFFIC_MANAGER_BASE_ADDR +\
					 IPQ9574_PPE_L1_C_SP_CFG_TBL_ADDR)

#define IPQ9574_PPE_L0_E_SP_CFG_TBL_ADDR	0x6000
#define IPQ9574_PPE_L0_E_SP_CFG_TBL		(IPQ9574_PPE_TRAFFIC_MANAGER_BASE_ADDR +\
					 IPQ9574_PPE_L0_E_SP_CFG_TBL_ADDR)

#define IPQ9574_PPE_L1_E_SP_CFG_TBL_ADDR		0x44000
#define IPQ9574_PPE_L1_E_SP_CFG_TBL		(IPQ9574_PPE_TRAFFIC_MANAGER_BASE_ADDR +\
					 IPQ9574_PPE_L1_E_SP_CFG_TBL_ADDR)

#define IPQ9574_PPE_FPGA_GPIO_BASE_ADDR		0x01008000

#define IPQ9574_PPE_MAC_PORT_MUX_OFFSET		0x10
#define IPQ9574_PPE_FPGA_GPIO_OFFSET		0xc000
#define IPQ9574_PPE_FPGA_SCHED_OFFSET		0x47a000
#define IPQ9574_PPE_TDM_CFG_DEPTH_OFFSET	0xb000
#define IPQ9574_PPE_TDM_SCHED_DEPTH_OFFSET	0x400000
#define IPQ9574_PPE_PORT_BRIDGE_CTRL_OFFSET	0x060300

#define IPQ9574_PPE_TDM_CFG_DEPTH_VAL		0x80000064
#define IPQ9574_PPE_MAC_PORT_MUX_OFFSET_VAL	0x15
#define IPQ9574_PPE_TDM_SCHED_DEPTH_VAL		0x32
#define IPQ9574_PPE_TDM_CFG_VALID		0x20
#define IPQ9574_PPE_TDM_CFG_DIR_INGRESS		0x0
#define IPQ9574_PPE_TDM_CFG_DIR_EGRESS		0x10
#define IPQ9574_PPE_PORT_EDMA			0x0
#define IPQ9574_PPE_PORT_QTI1			0x1
#define IPQ9574_PPE_PORT_QTI2			0x2
#define IPQ9574_PPE_PORT_QTI3			0x3
#define IPQ9574_PPE_PORT_QTI4			0x4
#define IPQ9574_PPE_PORT_XGMAC1			0x5
#define IPQ9574_PPE_PORT_XGMAC2			0x6
#define IPQ9574_PPE_PORT_CRYPTO1		0x7
#define IPQ9574_PPE_PORT_BRIDGE_CTRL_PROMISC_EN	0x20000
#define IPQ9574_PPE_PORT_BRIDGE_CTRL_TXMAC_EN	0x10000
#define IPQ9574_PPE_PORT_BRIDGE_CTRL_PORT_ISOLATION_BMP	0x7f00
#define IPQ9574_PPE_PORT_BRIDGE_CTRL_STATION_LRN_EN	0x8
#define IPQ9574_PPE_PORT_BRIDGE_CTRL_NEW_ADDR_LRN_EN	0x1

#define IPQ9574_PPE_PORT_EDMA_BITPOS		0x1
#define IPQ9574_PPE_PORT_QTI1_BITPOS		(1 << IPQ9574_PPE_PORT_QTI1)
#define IPQ9574_PPE_PORT_QTI2_BITPOS		(1 << IPQ9574_PPE_PORT_QTI2)
#define IPQ9574_PPE_PORT_QTI3_BITPOS		(1 << IPQ9574_PPE_PORT_QTI3)
#define IPQ9574_PPE_PORT_QTI4_BITPOS		(1 << IPQ9574_PPE_PORT_QTI4)
#define IPQ9574_PPE_PORT_XGMAC1_BITPOS		(1 << IPQ9574_PPE_PORT_XGMAC1)
#define IPQ9574_PPE_PORT_XGMAC2_BITPOS		(1 << IPQ9574_PPE_PORT_XGMAC2)
#define IPQ9574_PPE_PORT_CRYPTO1_BITPOS		(1 << IPQ9574_PPE_PORT_CRYPTO1)

#define PPE_SWITCH_NSS_SWITCH_XGMAC0		0x500000
#define NSS_SWITCH_XGMAC_MAC_TX_CONFIGURATION	0x4000
#define USS					(1 << 31)
#define SS(i)					(i << 29)
#define JD					(1 << 16)
#define TE					(1 << 0)
#define NSS_SWITCH_XGMAC_MAC_RX_CONFIGURATION	0x4000
#define MAC_RX_CONFIGURATION_ADDRESS		0x4
#define RE					(1 << 0)
#define ACS					(1 << 1)
#define CST					(1 << 2)
#define MAC_PACKET_FILTER_INC			0x4000
#define MAC_PACKET_FILTER_ADDRESS 		0x8

#define XGMAC_SPEED_SELECT_10000M 		0
#define XGMAC_SPEED_SELECT_5000M 		1
#define XGMAC_SPEED_SELECT_2500M 		2
#define XGMAC_SPEED_SELECT_1000M 		3

#define IPE_L2_BASE_ADDR		0x060000
#define PORT_BRIDGE_CTRL_ADDRESS 	0x300
#define PORT_BRIDGE_CTRL_INC		0x4
#define TX_MAC_EN			(1 << 16)

#define IPO_CSR_BASE_ADDR		0x0b0000

#define IPO_RULE_REG_ADDRESS 		0x0
#define IPO_RULE_REG_INC		0x10

#define IPO_MASK_REG_ADDRESS		0x2000
#define IPO_MASK_REG_INC		0x10

#define IPO_ACTION_ADDRESS 		0x8000
#define IPO_ACTION_INC			0x20
