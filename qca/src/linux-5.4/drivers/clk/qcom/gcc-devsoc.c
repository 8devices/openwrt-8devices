// SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
/*
 * Copyright (c) 2016-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/clk-provider.h>
#include <linux/regmap.h>

#include <linux/reset-controller.h>
#include <dt-bindings/clock/qcom,gcc-devsoc.h>
#include <dt-bindings/reset/qcom,gcc-devsoc.h>

#include "common.h"
#include "clk-regmap.h"
#include "clk-pll.h"
#include "clk-rcg.h"
#include "clk-branch.h"
#include "clk-alpha-pll.h"
#include "clk-regmap-divider.h"
#include "clk-regmap-mux.h"
#include "reset.h"

static int clk_dummy_is_enabled(struct clk_hw *hw)
{
	return 1;
};

static int clk_dummy_enable(struct clk_hw *hw)
{
	return 0;
};

static void clk_dummy_disable(struct clk_hw *hw)
{
	return;
};

static u8 clk_dummy_get_parent(struct clk_hw *hw)
{
	return 0;
};

static int clk_dummy_set_parent(struct clk_hw *hw, u8 index)
{
	return 0;
};

static int clk_dummy_set_rate(struct clk_hw *hw, unsigned long rate,
			      unsigned long parent_rate)
{
	return 0;
};

static int clk_dummy_determine_rate(struct clk_hw *hw,
				struct clk_rate_request *req)
{
	return 0;
};

static unsigned long clk_dummy_recalc_rate(struct clk_hw *hw,
					   unsigned long parent_rate)
{
	return parent_rate;
};

static const struct clk_ops clk_dummy_ops = {
	.is_enabled = clk_dummy_is_enabled,
	.enable = clk_dummy_enable,
	.disable = clk_dummy_disable,
	.get_parent = clk_dummy_get_parent,
	.set_parent = clk_dummy_set_parent,
	.set_rate = clk_dummy_set_rate,
	.recalc_rate = clk_dummy_recalc_rate,
	.determine_rate = clk_dummy_determine_rate,
};

#define DEFINE_DUMMY_CLK(clk_name)				\
(&(struct clk_regmap) {						\
	.hw.init = &(struct clk_init_data){			\
		.name = #clk_name,				\
		.parent_names = (const char *[]){ "xo"},	\
		.num_parents = 1,				\
		.ops = &clk_dummy_ops,				\
	},							\
})

static struct clk_regmap *gcc_devsoc_dummy_clks[] = {
	[GPLL0_MAIN] = DEFINE_DUMMY_CLK(gpll0_main.clkr),
	[GPLL0] = DEFINE_DUMMY_CLK(gpll0.clkr),
	[GPLL2_MAIN] = DEFINE_DUMMY_CLK(gpll2_main.clkr),
	[GPLL2] = DEFINE_DUMMY_CLK(gpll2.clkr),
	[GPLL4_MAIN] = DEFINE_DUMMY_CLK(gpll4_main.clkr),
	[GPLL4] = DEFINE_DUMMY_CLK(gpll4.clkr),
	[UBI32_PLL_MAIN] = DEFINE_DUMMY_CLK(ubi32_pll_main.clkr),
	[UBI32_PLL] = DEFINE_DUMMY_CLK(ubi32_pll.clkr),
	[ADSS_PWM_CLK_SRC] = DEFINE_DUMMY_CLK(adss_pwm_clk_src.clkr),
	[APSS_AHB_CLK_SRC] = DEFINE_DUMMY_CLK(apss_ahb_clk_src.clkr),
	[APSS_AHB_POSTDIV_CLK_SRC] = DEFINE_DUMMY_CLK(apss_ahb_postdiv_clk_src.clkr),
	[APSS_AXI_CLK_SRC] = DEFINE_DUMMY_CLK(apss_axi_clk_src.clkr),
	[BLSP1_QUP1_I2C_APPS_CLK_SRC] = DEFINE_DUMMY_CLK(blsp1_qup1_i2c_apps_clk_src.clkr),
	[BLSP1_QUP1_SPI_APPS_CLK_SRC] = DEFINE_DUMMY_CLK(blsp1_qup1_spi_apps_clk_src.clkr),
	[BLSP1_QUP2_I2C_APPS_CLK_SRC] = DEFINE_DUMMY_CLK(blsp1_qup2_i2c_apps_clk_src.clkr),
	[BLSP1_QUP2_SPI_APPS_CLK_SRC] = DEFINE_DUMMY_CLK(blsp1_qup2_spi_apps_clk_src.clkr),
	[BLSP1_QUP3_I2C_APPS_CLK_SRC] = DEFINE_DUMMY_CLK(blsp1_qup3_i2c_apps_clk_src.clkr),
	[BLSP1_QUP3_SPI_APPS_CLK_SRC] = DEFINE_DUMMY_CLK(blsp1_qup3_spi_apps_clk_src.clkr),
	[BLSP1_UART1_APPS_CLK_SRC] = DEFINE_DUMMY_CLK(blsp1_uart1_apps_clk_src.clkr),
	[BLSP1_UART2_APPS_CLK_SRC] = DEFINE_DUMMY_CLK(blsp1_uart2_apps_clk_src.clkr),
	[CRYPTO_CLK_SRC] = DEFINE_DUMMY_CLK(crypto_clk_src.clkr),
	[GCC_ADSS_PWM_CLK] = DEFINE_DUMMY_CLK(gcc_adss_pwm_clk.clkr),
	[GCC_APSS_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_apss_ahb_clk.clkr),
	[GCC_APSS_AXI_CLK] = DEFINE_DUMMY_CLK(gcc_apss_axi_clk.clkr),
	[GCC_BLSP1_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_blsp1_ahb_clk.clkr),
	[GCC_BLSP1_QUP1_I2C_APPS_CLK] = DEFINE_DUMMY_CLK(gcc_blsp1_qup1_i2c_apps_clk.clkr),
	[GCC_BLSP1_QUP1_SPI_APPS_CLK] = DEFINE_DUMMY_CLK(gcc_blsp1_qup1_spi_apps_clk.clkr),
	[GCC_BLSP1_QUP2_I2C_APPS_CLK] = DEFINE_DUMMY_CLK(gcc_blsp1_qup2_i2c_apps_clk.clkr),
	[GCC_BLSP1_QUP2_SPI_APPS_CLK] = DEFINE_DUMMY_CLK(gcc_blsp1_qup2_spi_apps_clk.clkr),
	[GCC_BLSP1_QUP3_I2C_APPS_CLK] = DEFINE_DUMMY_CLK(gcc_blsp1_qup3_i2c_apps_clk.clkr),
	[GCC_BLSP1_QUP3_SPI_APPS_CLK] = DEFINE_DUMMY_CLK(gcc_blsp1_qup3_spi_apps_clk.clkr),
	[GCC_BLSP1_UART1_APPS_CLK] = DEFINE_DUMMY_CLK(gcc_blsp1_uart1_apps_clk.clkr),
	[GCC_BLSP1_UART2_APPS_CLK] = DEFINE_DUMMY_CLK(gcc_blsp1_uart2_apps_clk.clkr),
	[GCC_BTSS_LPO_CLK] = DEFINE_DUMMY_CLK(gcc_btss_lpo_clk.clkr),
	[GCC_CMN_BLK_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_cmn_blk_ahb_clk.clkr),
	[GCC_CMN_BLK_SYS_CLK] = DEFINE_DUMMY_CLK(gcc_cmn_blk_sys_clk.clkr),
	[GCC_CRYPTO_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_crypto_ahb_clk.clkr),
	[GCC_CRYPTO_AXI_CLK] = DEFINE_DUMMY_CLK(gcc_crypto_axi_clk.clkr),
	[GCC_CRYPTO_CLK] = DEFINE_DUMMY_CLK(gcc_crypto_clk.clkr),
	[GCC_DCC_CLK] = DEFINE_DUMMY_CLK(gcc_dcc_clk.clkr),
	[GCC_GEPHY_RX_CLK] = DEFINE_DUMMY_CLK(gcc_gephy_rx_clk.clkr),
	[GCC_GEPHY_TX_CLK] = DEFINE_DUMMY_CLK(gcc_gephy_tx_clk.clkr),
	[GCC_GMAC0_CFG_CLK] = DEFINE_DUMMY_CLK(gcc_gmac0_cfg_clk.clkr),
	[GCC_GMAC0_PTP_CLK] = DEFINE_DUMMY_CLK(gcc_gmac0_ptp_clk.clkr),
	[GCC_GMAC0_RX_CLK] = DEFINE_DUMMY_CLK(gcc_gmac0_rx_clk.clkr),
	[GCC_GMAC0_SYS_CLK] = DEFINE_DUMMY_CLK(gcc_gmac0_sys_clk.clkr),
	[GCC_GMAC0_TX_CLK] = DEFINE_DUMMY_CLK(gcc_gmac0_tx_clk.clkr),
	[GCC_GMAC1_CFG_CLK] = DEFINE_DUMMY_CLK(gcc_gmac1_cfg_clk.clkr),
	[GCC_GMAC1_PTP_CLK] = DEFINE_DUMMY_CLK(gcc_gmac1_ptp_clk.clkr),
	[GCC_GMAC1_RX_CLK] = DEFINE_DUMMY_CLK(gcc_gmac1_rx_clk.clkr),
	[GCC_GMAC1_SYS_CLK] = DEFINE_DUMMY_CLK(gcc_gmac1_sys_clk.clkr),
	[GCC_GMAC1_TX_CLK] = DEFINE_DUMMY_CLK(gcc_gmac1_tx_clk.clkr),
	[GCC_GP1_CLK] = DEFINE_DUMMY_CLK(gcc_gp1_clk.clkr),
	[GCC_GP2_CLK] = DEFINE_DUMMY_CLK(gcc_gp2_clk.clkr),
	[GCC_GP3_CLK] = DEFINE_DUMMY_CLK(gcc_gp3_clk.clkr),
	[GCC_LPASS_CORE_AXIM_CLK] = DEFINE_DUMMY_CLK(gcc_lpass_core_axim_clk.clkr),
	[GCC_LPASS_SWAY_CLK] = DEFINE_DUMMY_CLK(gcc_lpass_sway_clk.clkr),
	[GCC_MDIO0_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_mdio0_ahb_clk.clkr),
	[GCC_MDIO1_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_mdio1_ahb_clk.clkr),
	[GCC_PCIE0_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_pcie0_ahb_clk.clkr),
	[GCC_PCIE0_AUX_CLK] = DEFINE_DUMMY_CLK(gcc_pcie0_aux_clk.clkr),
	[GCC_PCIE0_AXI_M_CLK] = DEFINE_DUMMY_CLK(gcc_pcie0_axi_m_clk.clkr),
	[GCC_PCIE0_AXI_S_BRIDGE_CLK] = DEFINE_DUMMY_CLK(gcc_pcie0_axi_s_bridge_clk.clkr),
	[GCC_PCIE0_AXI_S_CLK] = DEFINE_DUMMY_CLK(gcc_pcie0_axi_s_clk.clkr),
	[GCC_PCIE1_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_pcie1_ahb_clk.clkr),
	[GCC_PCIE1_AUX_CLK] = DEFINE_DUMMY_CLK(gcc_pcie1_aux_clk.clkr),
	[GCC_PCIE1_AXI_M_CLK] = DEFINE_DUMMY_CLK(gcc_pcie1_axi_m_clk.clkr),
	[GCC_PCIE1_AXI_S_BRIDGE_CLK] = DEFINE_DUMMY_CLK(gcc_pcie1_axi_s_bridge_clk.clkr),
	[GCC_PCIE1_AXI_S_CLK] = DEFINE_DUMMY_CLK(gcc_pcie1_axi_s_clk.clkr),
	[GCC_PRNG_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_prng_ahb_clk.clkr),
	[GCC_Q6_AXIM_CLK] = DEFINE_DUMMY_CLK(gcc_q6_axim_clk.clkr),
	[GCC_Q6_AXIM2_CLK] = DEFINE_DUMMY_CLK(gcc_q6_axim2_clk.clkr),
	[GCC_Q6_AXIS_CLK] = DEFINE_DUMMY_CLK(gcc_q6_axis_clk.clkr),
	[GCC_Q6_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_q6_ahb_clk.clkr),
	[GCC_Q6_AHB_S_CLK] = DEFINE_DUMMY_CLK(gcc_q6_ahb_s_clk.clkr),
	[GCC_Q6_TSCTR_1TO2_CLK] = DEFINE_DUMMY_CLK(gcc_q6_tsctr_1to2_clk.clkr),
	[GCC_Q6SS_ATBM_CLK] = DEFINE_DUMMY_CLK(gcc_q6ss_atbm_clk.clkr),
	[GCC_Q6SS_PCLKDBG_CLK] = DEFINE_DUMMY_CLK(gcc_q6ss_pclkdbg_clk.clkr),
	[GCC_Q6SS_TRIG_CLK] = DEFINE_DUMMY_CLK(gcc_q6ss_trig_clk.clkr),
	[GCC_QDSS_AT_CLK] = DEFINE_DUMMY_CLK(gcc_qdss_at_clk.clkr),
	[GCC_QDSS_CFG_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_qdss_cfg_ahb_clk.clkr),
	[GCC_QDSS_DAP_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_qdss_dap_ahb_clk.clkr),
	[GCC_QDSS_DAP_CLK] = DEFINE_DUMMY_CLK(gcc_qdss_dap_clk.clkr),
	[GCC_QDSS_ETR_USB_CLK] = DEFINE_DUMMY_CLK(gcc_qdss_etr_usb_clk.clkr),
	[GCC_QDSS_EUD_AT_CLK] = DEFINE_DUMMY_CLK(gcc_qdss_eud_at_clk.clkr),
	[GCC_QDSS_STM_CLK] = DEFINE_DUMMY_CLK(gcc_qdss_stm_clk.clkr),
	[GCC_QDSS_TRACECLKIN_CLK] = DEFINE_DUMMY_CLK(gcc_qdss_traceclkin_clk.clkr),
	[GCC_QDSS_TSCTR_DIV8_CLK] = DEFINE_DUMMY_CLK(gcc_qdss_tsctr_div8_clk.clkr),
	[GCC_QPIC_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_qpic_ahb_clk.clkr),
	[GCC_QPIC_CLK] = DEFINE_DUMMY_CLK(gcc_qpic_clk.clkr),
	[GCC_QPIC_IO_MACRO_CLK] = DEFINE_DUMMY_CLK(gcc_qpic_io_macro_clk.clkr),
	[GCC_SDCC1_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_sdcc1_ahb_clk.clkr),
	[GCC_SDCC1_APPS_CLK] = DEFINE_DUMMY_CLK(gcc_sdcc1_apps_clk.clkr),
	[GCC_SLEEP_CLK_SRC] = DEFINE_DUMMY_CLK(gcc_sleep_clk_src.clkr),
	[GCC_SNOC_GMAC0_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_snoc_gmac0_ahb_clk.clkr),
	[GCC_SNOC_GMAC0_AXI_CLK] = DEFINE_DUMMY_CLK(gcc_snoc_gmac0_axi_clk.clkr),
	[GCC_SNOC_GMAC1_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_snoc_gmac1_ahb_clk.clkr),
	[GCC_SNOC_GMAC1_AXI_CLK] = DEFINE_DUMMY_CLK(gcc_snoc_gmac1_axi_clk.clkr),
	[GCC_SNOC_LPASS_AXIM_CLK] = DEFINE_DUMMY_CLK(gcc_snoc_lpass_axim_clk.clkr),
	[GCC_SNOC_LPASS_SWAY_CLK] = DEFINE_DUMMY_CLK(gcc_snoc_lpass_sway_clk.clkr),
	[GCC_SNOC_UBI0_AXI_CLK] = DEFINE_DUMMY_CLK(gcc_snoc_ubi0_axi_clk.clkr),
	[GCC_SYS_NOC_PCIE0_AXI_CLK] = DEFINE_DUMMY_CLK(gcc_sys_noc_pcie0_axi_clk.clkr),
	[GCC_SYS_NOC_PCIE1_AXI_CLK] = DEFINE_DUMMY_CLK(gcc_sys_noc_pcie1_axi_clk.clkr),
	[GCC_SYS_NOC_QDSS_STM_AXI_CLK] = DEFINE_DUMMY_CLK(gcc_sys_noc_qdss_stm_axi_clk.clkr),
	[GCC_SYS_NOC_USB0_AXI_CLK] = DEFINE_DUMMY_CLK(gcc_sys_noc_usb0_axi_clk.clkr),
	[GCC_SYS_NOC_WCSS_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_sys_noc_wcss_ahb_clk.clkr),
	[GCC_UBI0_AXI_CLK] = DEFINE_DUMMY_CLK(gcc_ubi0_axi_clk.clkr),
	[GCC_UBI0_CFG_CLK] = DEFINE_DUMMY_CLK(gcc_ubi0_cfg_clk.clkr),
	[GCC_UBI0_CORE_CLK] = DEFINE_DUMMY_CLK(gcc_ubi0_core_clk.clkr),
	[GCC_UBI0_DBG_CLK] = DEFINE_DUMMY_CLK(gcc_ubi0_dbg_clk.clkr),
	[GCC_UBI0_NC_AXI_CLK] = DEFINE_DUMMY_CLK(gcc_ubi0_nc_axi_clk.clkr),
	[GCC_UBI0_UTCM_CLK] = DEFINE_DUMMY_CLK(gcc_ubi0_utcm_clk.clkr),
	[GCC_UNIPHY_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_uniphy_ahb_clk.clkr),
	[GCC_UNIPHY_RX_CLK] = DEFINE_DUMMY_CLK(gcc_uniphy_rx_clk.clkr),
	[GCC_UNIPHY_SYS_CLK] = DEFINE_DUMMY_CLK(gcc_uniphy_sys_clk.clkr),
	[GCC_UNIPHY_TX_CLK] = DEFINE_DUMMY_CLK(gcc_uniphy_tx_clk.clkr),
	[GCC_USB0_AUX_CLK] = DEFINE_DUMMY_CLK(gcc_usb0_aux_clk.clkr),
	[GCC_USB0_EUD_AT_CLK] = DEFINE_DUMMY_CLK(gcc_usb0_eud_at_clk.clkr),
	[GCC_USB0_LFPS_CLK] = DEFINE_DUMMY_CLK(gcc_usb0_lfps_clk.clkr),
	[GCC_USB0_MASTER_CLK] = DEFINE_DUMMY_CLK(gcc_usb0_master_clk.clkr),
	[GCC_USB0_MOCK_UTMI_CLK] = DEFINE_DUMMY_CLK(gcc_usb0_mock_utmi_clk.clkr),
	[GCC_USB0_PHY_CFG_AHB_CLK] = DEFINE_DUMMY_CLK(gcc_usb0_phy_cfg_ahb_clk.clkr),
	[GCC_USB0_SLEEP_CLK] = DEFINE_DUMMY_CLK(gcc_usb0_sleep_clk.clkr),
	[GCC_WCSS_ACMT_CLK] = DEFINE_DUMMY_CLK(gcc_wcss_acmt_clk.clkr),
	[GCC_WCSS_AHB_S_CLK] = DEFINE_DUMMY_CLK(gcc_wcss_ahb_s_clk.clkr),
	[GCC_WCSS_AXI_M_CLK] = DEFINE_DUMMY_CLK(gcc_wcss_axi_m_clk.clkr),
	[GCC_WCSS_AXI_S_CLK] = DEFINE_DUMMY_CLK(gcc_wcss_axi_s_clk.clkr),
	[GCC_WCSS_DBG_IFC_APB_BDG_CLK] = DEFINE_DUMMY_CLK(gcc_wcss_dbg_ifc_apb_bdg_clk.clkr),
	[GCC_WCSS_DBG_IFC_APB_CLK] = DEFINE_DUMMY_CLK(gcc_wcss_dbg_ifc_apb_clk.clkr),
	[GCC_WCSS_DBG_IFC_ATB_BDG_CLK] = DEFINE_DUMMY_CLK(gcc_wcss_dbg_ifc_atb_bdg_clk.clkr),
	[GCC_WCSS_DBG_IFC_ATB_CLK] = DEFINE_DUMMY_CLK(gcc_wcss_dbg_ifc_atb_clk.clkr),
	[GCC_WCSS_DBG_IFC_DAPBUS_BDG_CLK] = DEFINE_DUMMY_CLK(gcc_wcss_dbg_ifc_dapbus_bdg_clk.clkr),
	[GCC_WCSS_DBG_IFC_DAPBUS_CLK] = DEFINE_DUMMY_CLK(gcc_wcss_dbg_ifc_dapbus_clk.clkr),
	[GCC_WCSS_DBG_IFC_NTS_BDG_CLK] = DEFINE_DUMMY_CLK(gcc_wcss_dbg_ifc_nts_bdg_clk.clkr),
	[GCC_WCSS_DBG_IFC_NTS_CLK] = DEFINE_DUMMY_CLK(gcc_wcss_dbg_ifc_nts_clk.clkr),
	[GCC_WCSS_ECAHB_CLK] = DEFINE_DUMMY_CLK(gcc_wcss_ecahb_clk.clkr),
	[GCC_XO_CLK] = DEFINE_DUMMY_CLK(gcc_xo_clk.clkr),
	[GCC_XO_CLK_SRC] = DEFINE_DUMMY_CLK(gcc_xo_clk_src.clkr),
	[GMAC0_RX_CLK_SRC] = DEFINE_DUMMY_CLK(gmac0_rx_clk_src.clkr),
	[GMAC0_RX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(gmac0_rx_div_clk_src.clkr),
	[GMAC0_TX_CLK_SRC] = DEFINE_DUMMY_CLK(gmac0_tx_clk_src.clkr),
	[GMAC0_TX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(gmac0_tx_div_clk_src.clkr),
	[GMAC1_RX_CLK_SRC] = DEFINE_DUMMY_CLK(gmac1_rx_clk_src.clkr),
	[GMAC1_RX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(gmac1_rx_div_clk_src.clkr),
	[GMAC1_TX_CLK_SRC] = DEFINE_DUMMY_CLK(gmac1_tx_clk_src.clkr),
	[GMAC1_TX_DIV_CLK_SRC] = DEFINE_DUMMY_CLK(gmac1_tx_div_clk_src.clkr),
	[GMAC_CLK_SRC] = DEFINE_DUMMY_CLK(gmac_clk_src.clkr),
	[GP1_CLK_SRC] = DEFINE_DUMMY_CLK(gp1_clk_src.clkr),
	[GP2_CLK_SRC] = DEFINE_DUMMY_CLK(gp2_clk_src.clkr),
	[GP3_CLK_SRC] = DEFINE_DUMMY_CLK(gp3_clk_src.clkr),
	[LPASS_AXIM_CLK_SRC] = DEFINE_DUMMY_CLK(lpass_axim_clk_src.clkr),
	[LPASS_SWAY_CLK_SRC] = DEFINE_DUMMY_CLK(lpass_sway_clk_src.clkr),
	[PCIE0_AUX_CLK_SRC] = DEFINE_DUMMY_CLK(pcie0_aux_clk_src.clkr),
	[PCIE0_AXI_CLK_SRC] = DEFINE_DUMMY_CLK(pcie0_axi_clk_src.clkr),
	[PCIE1_AUX_CLK_SRC] = DEFINE_DUMMY_CLK(pcie1_aux_clk_src.clkr),
	[PCIE1_AXI_CLK_SRC] = DEFINE_DUMMY_CLK(pcie1_axi_clk_src.clkr),
	[PCNOC_BFDCD_CLK_SRC] = DEFINE_DUMMY_CLK(pcnoc_bfdcd_clk_src.clkr),
	[Q6_AXI_CLK_SRC] = DEFINE_DUMMY_CLK(q6_axi_clk_src.clkr),
	[QDSS_AT_CLK_SRC] = DEFINE_DUMMY_CLK(qdss_at_clk_src.clkr),
	[QDSS_STM_CLK_SRC] = DEFINE_DUMMY_CLK(qdss_stm_clk_src.clkr),
	[QDSS_TSCTR_CLK_SRC] = DEFINE_DUMMY_CLK(qdss_tsctr_clk_src.clkr),
	[QDSS_TRACECLKIN_CLK_SRC] = DEFINE_DUMMY_CLK(qdss_traceclkin_clk_src.clkr),
	[QPIC_IO_MACRO_CLK_SRC] = DEFINE_DUMMY_CLK(qpic_io_macro_clk_src.clkr),
	[SDCC1_APPS_CLK_SRC] = DEFINE_DUMMY_CLK(sdcc1_apps_clk_src.clkr),
	[SYSTEM_NOC_BFDCD_CLK_SRC] = DEFINE_DUMMY_CLK(system_noc_bfdcd_clk_src.clkr),
	[UBI0_AXI_CLK_SRC] = DEFINE_DUMMY_CLK(ubi0_axi_clk_src.clkr),
	[UBI0_CORE_CLK_SRC] = DEFINE_DUMMY_CLK(ubi0_core_clk_src.clkr),
	[USB0_AUX_CLK_SRC] = DEFINE_DUMMY_CLK(usb0_aux_clk_src.clkr),
	[USB0_LFPS_CLK_SRC] = DEFINE_DUMMY_CLK(usb0_lfps_clk_src.clkr),
	[USB0_MASTER_CLK_SRC] = DEFINE_DUMMY_CLK(usb0_master_clk_src.clkr),
	[USB0_MOCK_UTMI_CLK_SRC] = DEFINE_DUMMY_CLK(usb0_mock_utmi_clk_src.clkr),
	[WCSS_AHB_CLK_SRC] = DEFINE_DUMMY_CLK(wcss_ahb_clk_src.clkr),
	[PCIE0_PIPE_CLK_SRC] = DEFINE_DUMMY_CLK(pcie0_pipe_clk_src.clkr),
	[PCIE1_PIPE_CLK_SRC] = DEFINE_DUMMY_CLK(pcie1_pipe_clk_src.clkr),
	[GCC_PCIE0_PIPE_CLK] = DEFINE_DUMMY_CLK(gcc_pcie0_pipe_clk.clkr),
	[GCC_PCIE1_PIPE_CLK] = DEFINE_DUMMY_CLK(gcc_pcie1_pipe_clk.clkr),
	[USB0_PIPE_CLK_SRC] = DEFINE_DUMMY_CLK(usb0_pipe_clk_src.clkr),
	[GCC_USB0_PIPE_CLK] = DEFINE_DUMMY_CLK(gcc_usb0_pipe_clk.clkr),
};

static const struct qcom_reset_map gcc_devsoc_resets[] = {
	[GCC_APC0_VOLTAGE_DROOP_DETECTOR_BCR] = { 0x78000, 0 },
	[GCC_BLSP1_BCR] = { 0x01000, 0 },
	[GCC_BLSP1_QUP1_BCR] = { 0x02000, 0 },
	[GCC_BLSP1_QUP2_BCR] = { 0x03008, 0 },
	[GCC_BLSP1_QUP3_BCR] = { 0x04008, 0 },
	[GCC_BLSP1_UART1_BCR] = { 0x02038, 0 },
	[GCC_BLSP1_UART2_BCR] = { 0x03028, 0 },
	[GCC_BOOT_ROM_BCR] = { 0x13008, 0 },
	[GCC_BTSS_BCR] = { 0x1c000, 0 },
	[GCC_CMN_BLK_BCR] = { 0x56300, 0 },
	[GCC_CMN_LDO_BCR] = { 0x33000, 0 },
	[GCC_CE_BCR] = { 0x33014, 0 },
	[GCC_CRYPTO_BCR] = { 0x16000, 0 },
	[GCC_DCC_BCR] = { 0x77000, 0 },
	[GCC_DCD_BCR] = { 0x2a000, 0 },
	[GCC_DDRSS_BCR] = { 0x1e000, 0 },
	[GCC_EDPD_BCR] = { 0x3a000, 0 },
	[GCC_GEPHY_BCR] = { 0x56000, 0 },
	[GCC_GEPHY_MDC_SW_ARES] = { 0x56004, 0, 0x3 },
	[GCC_GEPHY_DSP_HW_ARES] = { 0x56004, 1 },
	[GCC_GEPHY_RX_ARES] = { 0x56004, 2 },
	[GCC_GEPHY_TX_ARES] = { 0x56004, 3 },
	[GCC_GMAC0_BCR] = { 0x19000, 0 },
	[GCC_GMAC0_CFG_ARES] = { 0x68428, 0 },
	[GCC_GMAC0_SYS_ARES] = { 0x68428, 1 },
	[GCC_GMAC1_BCR] = { 0x19100, 0 },
	[GCC_GMAC1_CFG_ARES] = { 0x68438, 0 },
	[GCC_GMAC1_SYS_ARES] = { 0x68438, 1 },
	[GCC_IMEM_BCR] = { 0x0e000, 0 },
	[GCC_LPASS_BCR] = { 0x2e000, 0 },
	[GCC_MDIO0_BCR] = { 0x58000, 0 },
	[GCC_MDIO1_BCR] = { 0x58010, 0 },
	[GCC_MPM_BCR] = { 0x2c000, 0 },
	[GCC_PCIE0_BCR] = { 0x75004, 0 },
	[GCC_PCIE0_LINK_DOWN_BCR] = { 0x750a8, 0 },
	[GCC_PCIE0_PHY_BCR] = { 0x75038, 0 },
	[GCC_PCIE0PHY_PHY_BCR] = { 0x7503c, 0 },
	[GCC_PCIE0_PIPE_ARES] = { 0x75040, 0 },
	[GCC_PCIE0_SLEEP_ARES] = { 0x75040, 1 },
	[GCC_PCIE0_CORE_STICKY_ARES] = { 0x75040, 2 },
	[GCC_PCIE0_AXI_MASTER_ARES] = { 0x75040, 3 },
	[GCC_PCIE0_AXI_SLAVE_ARES] = { 0x75040, 4 },
	[GCC_PCIE0_AHB_ARES] = { 0x75040, 5 },
	[GCC_PCIE0_AXI_MASTER_STICKY_ARES] = { 0x75040, 6 },
	[GCC_PCIE0_AXI_SLAVE_STICKY_ARES] = { 0x75040, 7 },
	[GCC_PCIE1_BCR] = { 0x76004, 0 },
	[GCC_PCIE1_LINK_DOWN_BCR] = { 0x76044, 0 },
	[GCC_PCIE1_PHY_BCR] = { 0x76038, 0 },
	[GCC_PCIE1PHY_PHY_BCR] = { 0x7603c, 0 },
	[GCC_PCIE1_PIPE_ARES] = { 0x76040, 0 },
	[GCC_PCIE1_SLEEP_ARES] = { 0x76040, 1 },
	[GCC_PCIE1_CORE_STICKY_ARES] = { 0x76040, 2 },
	[GCC_PCIE1_AXI_MASTER_ARES] = { 0x76040, 3 },
	[GCC_PCIE1_AXI_SLAVE_ARES] = { 0x76040, 4 },
	[GCC_PCIE1_AHB_ARES] = { 0x76040, 5 },
	[GCC_PCIE1_AXI_MASTER_STICKY_ARES] = { 0x76040, 6 },
	[GCC_PCIE1_AXI_SLAVE_STICKY_ARES] = { 0x76040, 7 },
	[GCC_PCNOC_BCR] = { 0x27018, 0 },
	[GCC_PCNOC_BUS_TIMEOUT0_BCR] = { 0x48000, 0 },
	[GCC_PCNOC_BUS_TIMEOUT1_BCR] = { 0x48008, 0 },
	[GCC_PCNOC_BUS_TIMEOUT2_BCR] = { 0x48010, 0 },
	[GCC_PCNOC_BUS_TIMEOUT3_BCR] = { 0x48018, 0 },
	[GCC_PCNOC_BUS_TIMEOUT4_BCR] = { 0x48020, 0 },
	[GCC_PCNOC_BUS_TIMEOUT5_BCR] = { 0x48028, 0 },
	[GCC_PCNOC_BUS_TIMEOUT6_BCR] = { 0x48030, 0 },
	[GCC_PCNOC_BUS_TIMEOUT7_BCR] = { 0x48038, 0 },
	[GCC_PCNOC_BUS_TIMEOUT8_BCR] = { 0x48040, 0 },
	[GCC_PCNOC_BUS_TIMEOUT9_BCR] = { 0x48048, 0 },
	[GCC_PCNOC_BUS_TIMEOUT10_BCR] = { 0x48050, 0 },
	[GCC_PCNOC_BUS_TIMEOUT11_BCR] = { 0x48058, 0 },
	[GCC_PRNG_BCR] = { 0x13000, 0 },
	[GCC_Q6SS_DBG_ARES] = { 0x59110, 0 },
	[GCC_Q6_AHB_S_ARES] = { 0x59110, 1 },
	[GCC_Q6_AHB_ARES] = { 0x59110, 2 },
	[GCC_Q6_AXIM2_ARES] = { 0x59110, 3 },
	[GCC_Q6_AXIM_ARES] = { 0x59110, 4 },
	[GCC_Q6_AXIS_ARES] = { 0x59158, 0 },
	[GCC_QDSS_BCR] = { 0x29000, 0 },
	[GCC_QPIC_BCR] = { 0x57018, 0 },
	[GCC_QUSB2_0_PHY_BCR] = { 0x41030, 0 },
	[GCC_SDCC1_BCR] = { 0x42000, 0 },
	[GCC_SEC_CTRL_BCR] = { 0x1a000, 0 },
	[GCC_SPDM_BCR] = { 0x2f000, 0 },
	[GCC_SYSTEM_NOC_BCR] = { 0x26000, 0 },
	[GCC_TCSR_BCR] = { 0x28000, 0 },
	[GCC_TLMM_BCR] = { 0x34000, 0 },
	[GCC_UBI0_AXI_ARES] = { 0x68010, 0 },
	[GCC_UBI0_AHB_ARES] = { 0x68010, 1 },
	[GCC_UBI0_NC_AXI_ARES] = { 0x68010, 2 },
	[GCC_UBI0_DBG_ARES] = { 0x68010, 3 },
	[GCC_UBI0_UTCM_ARES] = { 0x68010, 6 },
	[GCC_UBI0_CORE_ARES] = { 0x68010, 7 },
	[GCC_UBI32_BCR] = { 0x19064, 0 },
	[GCC_UNIPHY_BCR] = { 0x56100, 0 },
	[GCC_UNIPHY_AHB_ARES] = { 0x56104, 0 },
	[GCC_UNIPHY_SYS_ARES] = { 0x56104, 1 },
	[GCC_UNIPHY_RX_ARES] = { 0x56104, 4 },
	[GCC_UNIPHY_TX_ARES] = { 0x56104, 5 },
	[GCC_UNIPHY_SOFT_RESET] = {0x56104, 0, 0x32 },
	[GCC_USB0_BCR] = { 0x3e070, 0 },
	[GCC_USB0_PHY_BCR] = { 0x3e034, 0 },
	[GCC_WCSS_BCR] = { 0x18000, 0 },
	[GCC_WCSS_DBG_ARES] = { 0x59008, 0 },
	[GCC_WCSS_ECAHB_ARES] = { 0x59008, 1 },
	[GCC_WCSS_ACMT_ARES] = { 0x59008, 2 },
	[GCC_WCSS_DBG_BDG_ARES] = { 0x59008, 3 },
	[GCC_WCSS_AHB_S_ARES] = { 0x59008, 4 },
	[GCC_WCSS_AXI_M_ARES] = { 0x59008, 5 },
	[GCC_WCSS_AXI_S_ARES] = { 0x59008, 6 },
	[GCC_WCSS_Q6_BCR] = { 0x18004, 0 },
	[GCC_WCSSAON_RESET] = { 0x59010, 0},
	[GCC_GEPHY_MISC_ARES] = { 0x56004, 0, 0xf },
};

static const struct of_device_id gcc_devsoc_match_table[] = {
	{ .compatible = "qcom,gcc-devsoc" },
	{ }
};
MODULE_DEVICE_TABLE(of, gcc_devsoc_match_table);

static const struct regmap_config gcc_devsoc_regmap_config = {
	.reg_bits       = 32,
	.reg_stride     = 4,
	.val_bits       = 32,
	.max_register   = 0x7fffc,
	.fast_io	= true,
};

static const struct qcom_cc_desc gcc_devsoc_desc = {
	.config = &gcc_devsoc_regmap_config,
	.clks = gcc_devsoc_dummy_clks,
	.num_clks = ARRAY_SIZE(gcc_devsoc_dummy_clks),
	.resets = gcc_devsoc_resets,
	.num_resets = ARRAY_SIZE(gcc_devsoc_resets),
};

static int gcc_devsoc_probe(struct platform_device *pdev)
{
	int ret;
	struct regmap *regmap;
	struct qcom_cc_desc devsoc_desc = gcc_devsoc_desc;

	regmap = qcom_cc_map(pdev, &devsoc_desc);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	ret = qcom_cc_really_probe(pdev, &devsoc_desc, regmap);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register devsoc GCC clocks\n");
		return ret;
	}

	dev_info(&pdev->dev, "Registered DevSoC GCC clocks provider");

	return ret;
}

static struct platform_driver gcc_devsoc_driver = {
	.probe = gcc_devsoc_probe,
	.driver = {
		.name   = "qcom,gcc-devsoc",
		.of_match_table = gcc_devsoc_match_table,
	},
};

static int __init gcc_devsoc_init(void)
{
	return platform_driver_register(&gcc_devsoc_driver);
}
core_initcall(gcc_devsoc_init);

static void __exit gcc_devsoc_exit(void)
{
	platform_driver_unregister(&gcc_devsoc_driver);
}
module_exit(gcc_devsoc_exit);

MODULE_DESCRIPTION("Qualcomm Technologies, Inc. GCC IPQ5018 Driver");
MODULE_LICENSE("Dual BSD/GPLv2");
MODULE_ALIAS("platform:gcc-devsoc");
