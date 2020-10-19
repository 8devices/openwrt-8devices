/*
 * Copyright (c) 2015-2017, 2019 The Linux Foundation. All rights reserved.
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

#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/pinctrl.h>

#include "pinctrl-msm.h"

#define FUNCTION(fname)			                \
	[msm_mux_##fname] = {		                \
		.name = #fname,				\
		.groups = fname##_groups,               \
		.ngroups = ARRAY_SIZE(fname##_groups),	\
	}

#define REG_SIZE 0x1000
#define PINGROUP(id, f1, f2, f3, f4, f5, f6, f7, f8, f9)	\
	{					        \
		.name = "gpio" #id,			\
		.pins = gpio##id##_pins,		\
		.npins = (unsigned)ARRAY_SIZE(gpio##id##_pins),	\
		.funcs = (int[]){			\
			msm_mux_gpio, /* gpio mode */	\
			msm_mux_##f1,			\
			msm_mux_##f2,			\
			msm_mux_##f3,			\
			msm_mux_##f4,			\
			msm_mux_##f5,			\
			msm_mux_##f6,			\
			msm_mux_##f7,			\
			msm_mux_##f8,			\
			msm_mux_##f9			\
		},				        \
		.nfuncs = 10,				\
		.ctl_reg = REG_SIZE * id,	        	\
		.io_reg = 0x4 + REG_SIZE * id,		\
		.intr_cfg_reg = 0x8 + REG_SIZE * id,		\
		.intr_status_reg = 0xc + REG_SIZE * id,	\
		.intr_target_reg = 0x8 + REG_SIZE * id,	\
		.mux_bit = 2,			\
		.pull_bit = 0,			\
		.drv_bit = 6,			\
		.oe_bit = 9,			\
		.in_bit = 0,			\
		.out_bit = 1,			\
		.intr_enable_bit = 0,		\
		.intr_status_bit = 0,		\
		.intr_target_bit = 5,		\
		.intr_target_kpss_val = 3,  \
		.intr_raw_status_bit = 4,	\
		.intr_polarity_bit = 1,		\
		.intr_detection_bit = 2,	\
		.intr_detection_width = 2,	\
	}

static const struct pinctrl_pin_desc ipq5018_pins[] = {
	PINCTRL_PIN(0, "GPIO_0"),
	PINCTRL_PIN(1, "GPIO_1"),
	PINCTRL_PIN(2, "GPIO_2"),
	PINCTRL_PIN(3, "GPIO_3"),
	PINCTRL_PIN(4, "GPIO_4"),
	PINCTRL_PIN(5, "GPIO_5"),
	PINCTRL_PIN(6, "GPIO_6"),
	PINCTRL_PIN(7, "GPIO_7"),
	PINCTRL_PIN(8, "GPIO_8"),
	PINCTRL_PIN(9, "GPIO_9"),
	PINCTRL_PIN(10, "GPIO_10"),
	PINCTRL_PIN(11, "GPIO_11"),
	PINCTRL_PIN(12, "GPIO_12"),
	PINCTRL_PIN(13, "GPIO_13"),
	PINCTRL_PIN(14, "GPIO_14"),
	PINCTRL_PIN(15, "GPIO_15"),
	PINCTRL_PIN(16, "GPIO_16"),
	PINCTRL_PIN(17, "GPIO_17"),
	PINCTRL_PIN(18, "GPIO_18"),
	PINCTRL_PIN(19, "GPIO_19"),
	PINCTRL_PIN(20, "GPIO_20"),
	PINCTRL_PIN(21, "GPIO_21"),
	PINCTRL_PIN(22, "GPIO_22"),
	PINCTRL_PIN(23, "GPIO_23"),
	PINCTRL_PIN(24, "GPIO_24"),
	PINCTRL_PIN(25, "GPIO_25"),
	PINCTRL_PIN(26, "GPIO_26"),
	PINCTRL_PIN(27, "GPIO_27"),
	PINCTRL_PIN(28, "GPIO_28"),
	PINCTRL_PIN(29, "GPIO_29"),
	PINCTRL_PIN(30, "GPIO_30"),
	PINCTRL_PIN(31, "GPIO_31"),
	PINCTRL_PIN(32, "GPIO_32"),
	PINCTRL_PIN(33, "GPIO_33"),
	PINCTRL_PIN(34, "GPIO_34"),
	PINCTRL_PIN(35, "GPIO_35"),
	PINCTRL_PIN(36, "GPIO_36"),
	PINCTRL_PIN(37, "GPIO_37"),
	PINCTRL_PIN(38, "GPIO_38"),
	PINCTRL_PIN(39, "GPIO_39"),
	PINCTRL_PIN(40, "GPIO_40"),
	PINCTRL_PIN(41, "GPIO_41"),
	PINCTRL_PIN(42, "GPIO_42"),
	PINCTRL_PIN(43, "GPIO_43"),
	PINCTRL_PIN(44, "GPIO_44"),
	PINCTRL_PIN(45, "GPIO_45"),
	PINCTRL_PIN(46, "GPIO_46"),
	PINCTRL_PIN(47, "SDC1_CLK"),
	PINCTRL_PIN(48, "SDC1_CMD"),
	PINCTRL_PIN(49, "SDC1_DATA"),
	PINCTRL_PIN(50, "SDC2_CLK"),
	PINCTRL_PIN(51, "SDC2_CMD"),
	PINCTRL_PIN(52, "SDC2_DATA"),
};

#define DECLARE_MSM_GPIO_PINS(pin) \
	static const unsigned int gpio##pin##_pins[] = { pin }
DECLARE_MSM_GPIO_PINS(0);
DECLARE_MSM_GPIO_PINS(1);
DECLARE_MSM_GPIO_PINS(2);
DECLARE_MSM_GPIO_PINS(3);
DECLARE_MSM_GPIO_PINS(4);
DECLARE_MSM_GPIO_PINS(5);
DECLARE_MSM_GPIO_PINS(6);
DECLARE_MSM_GPIO_PINS(7);
DECLARE_MSM_GPIO_PINS(8);
DECLARE_MSM_GPIO_PINS(9);
DECLARE_MSM_GPIO_PINS(10);
DECLARE_MSM_GPIO_PINS(11);
DECLARE_MSM_GPIO_PINS(12);
DECLARE_MSM_GPIO_PINS(13);
DECLARE_MSM_GPIO_PINS(14);
DECLARE_MSM_GPIO_PINS(15);
DECLARE_MSM_GPIO_PINS(16);
DECLARE_MSM_GPIO_PINS(17);
DECLARE_MSM_GPIO_PINS(18);
DECLARE_MSM_GPIO_PINS(19);
DECLARE_MSM_GPIO_PINS(20);
DECLARE_MSM_GPIO_PINS(21);
DECLARE_MSM_GPIO_PINS(22);
DECLARE_MSM_GPIO_PINS(23);
DECLARE_MSM_GPIO_PINS(24);
DECLARE_MSM_GPIO_PINS(25);
DECLARE_MSM_GPIO_PINS(26);
DECLARE_MSM_GPIO_PINS(27);
DECLARE_MSM_GPIO_PINS(28);
DECLARE_MSM_GPIO_PINS(29);
DECLARE_MSM_GPIO_PINS(30);
DECLARE_MSM_GPIO_PINS(31);
DECLARE_MSM_GPIO_PINS(32);
DECLARE_MSM_GPIO_PINS(33);
DECLARE_MSM_GPIO_PINS(34);
DECLARE_MSM_GPIO_PINS(35);
DECLARE_MSM_GPIO_PINS(36);
DECLARE_MSM_GPIO_PINS(37);
DECLARE_MSM_GPIO_PINS(38);
DECLARE_MSM_GPIO_PINS(39);
DECLARE_MSM_GPIO_PINS(40);
DECLARE_MSM_GPIO_PINS(41);
DECLARE_MSM_GPIO_PINS(42);
DECLARE_MSM_GPIO_PINS(43);
DECLARE_MSM_GPIO_PINS(44);
DECLARE_MSM_GPIO_PINS(45);
DECLARE_MSM_GPIO_PINS(46);

enum ipq5018_functions {
	msm_mux_gpio,
	msm_mux_atest_char0,
	msm_mux_,
	msm_mux_wci0,
	msm_mux_qdss_cti_trig_out_a0,
	msm_mux_xfem0,
	msm_mux_atest_char1,
	msm_mux_qdss_cti_trig_in_a0,
	msm_mux_wci1,
	msm_mux_xfem1,
	msm_mux_atest_char2,
	msm_mux_qdss_cti_trig_out_a1,
	msm_mux_wci2,
	msm_mux_xfem2,
	msm_mux_atest_char3,
	msm_mux_qdss_cti_trig_in_a1,
	msm_mux_wci3,
	msm_mux_xfem3,
	msm_mux_sdc13,
	msm_mux_qspi3,
	msm_mux_blsp1_spi1,
	msm_mux_btss0,
	msm_mux_dbg_out,
	msm_mux_qdss_traceclk_a,
	msm_mux_burn0,
	msm_mux_sdc12,
	msm_mux_qspi2,
	msm_mux_cxc_clk,
	msm_mux_blsp1_i2c1,
	msm_mux_btss1,
	msm_mux_qdss_tracectl_a,
	msm_mux_burn1,
	msm_mux_sdc11,
	msm_mux_qspi1,
	msm_mux_cxc_data,
	msm_mux_btss2,
	msm_mux_qdss_tracedata_a,
	msm_mux_sdc10,
	msm_mux_qspi0,
	msm_mux_mac0,
	msm_mux_btss3,
	msm_mux_sdc1_cmd,
	msm_mux_qspi_cs,
	msm_mux_mac1,
	msm_mux_btss4,
	msm_mux_sdc1_clk,
	msm_mux_qspi_clk,
	msm_mux_blsp0_spi,
	msm_mux_blsp1_uart0,
	msm_mux_gcc_plltest,
	msm_mux_gcc_tlmm,
	msm_mux_blsp0_i2c,
	msm_mux_pcie0_clk,
	msm_mux_cri_trng0,
	msm_mux_cri_trng1,
	msm_mux_pcie0_wake,
	msm_mux_cri_trng,
	msm_mux_pcie1_clk,
	msm_mux_btss5,
	msm_mux_prng_rosc,
	msm_mux_blsp1_spi0,
	msm_mux_btss6,
	msm_mux_pcie1_wake,
	msm_mux_blsp1_i2c0,
	msm_mux_btss7,
	msm_mux_blsp0_uart0,
	msm_mux_pll_test,
	msm_mux_eud_gpio,
	msm_mux_audio_rxmclk,
	msm_mux_audio_pdm0,
	msm_mux_blsp2_spi1,
	msm_mux_blsp1_uart2,
	msm_mux_btss8,
	msm_mux_qdss_tracedata_b,
	msm_mux_audio_rxbclk,
	msm_mux_btss9,
	msm_mux_audio_rxfsync,
	msm_mux_audio_pdm1,
	msm_mux_blsp2_i2c1,
	msm_mux_btss10,
	msm_mux_audio_rxd,
	msm_mux_btss11,
	msm_mux_audio_txmclk,
	msm_mux_wsa_swrm,
	msm_mux_blsp2_spi,
	msm_mux_btss12,
	msm_mux_audio_txbclk,
	msm_mux_blsp0_uart1,
	msm_mux_btss13,
	msm_mux_audio_txfsync,
	msm_mux_audio_txd,
	msm_mux_wsis_reset,
	msm_mux_blsp2_spi0,
	msm_mux_blsp1_uart1,
	msm_mux_blsp2_i2c0,
	msm_mux_mdc,
	msm_mux_wsi_clk3,
	msm_mux_mdio,
	msm_mux_atest_char,
	msm_mux_wsi_data3,
	msm_mux_qdss_traceclk_b,
	msm_mux_reset_out,
	msm_mux_qdss_tracectl_b,
	msm_mux_pwm0,
	msm_mux_qdss_cti_trig_out_b0,
	msm_mux_wci4,
	msm_mux_xfem4,
	msm_mux_pwm1,
	msm_mux_qdss_cti_trig_in_b0,
	msm_mux_wci5,
	msm_mux_xfem5,
	msm_mux_pwm2,
	msm_mux_qdss_cti_trig_out_b1,
	msm_mux_wci6,
	msm_mux_xfem6,
	msm_mux_pwm3,
	msm_mux_qdss_cti_trig_in_b1,
	msm_mux_wci7,
	msm_mux_xfem7,
	msm_mux_led0,
	msm_mux_led2,
	msm_mux_NA,
};

static const char * const atest_char0_groups[] = {
	"gpio0",
};
static const char * const _groups[] = {
	"gpio0", "gpio1", "gpio2", "gpio3", "gpio4", "gpio5", "gpio6", "gpio7",
	"gpio8", "gpio9", "gpio10", "gpio11", "gpio12", "gpio13", "gpio14",
	"gpio15", "gpio16", "gpio17", "gpio18", "gpio19", "gpio20", "gpio21",
	"gpio22", "gpio23", "gpio24", "gpio25", "gpio26", "gpio27", "gpio28",
	"gpio29", "gpio30", "gpio31", "gpio32", "gpio33", "gpio34", "gpio35",
	"gpio36", "gpio37", "gpio38", "gpio39", "gpio40", "gpio41", "gpio42",
	"gpio43", "gpio44", "gpio45", "gpio46",
};
static const char * const wci0_groups[] = {
	"gpio0", "gpio0",
};
static const char * const qdss_cti_trig_out_a0_groups[] = {
	"gpio0",
};
static const char * const xfem0_groups[] = {
	"gpio0",
};
static const char * const atest_char1_groups[] = {
	"gpio1",
};
static const char * const qdss_cti_trig_in_a0_groups[] = {
	"gpio1",
};
static const char * const wci1_groups[] = {
	"gpio1", "gpio1",
};
static const char * const xfem1_groups[] = {
	"gpio1",
};
static const char * const atest_char2_groups[] = {
	"gpio2",
};
static const char * const qdss_cti_trig_out_a1_groups[] = {
	"gpio2",
};
static const char * const wci2_groups[] = {
	"gpio2", "gpio2",
};
static const char * const xfem2_groups[] = {
	"gpio2",
};
static const char * const atest_char3_groups[] = {
	"gpio3",
};
static const char * const qdss_cti_trig_in_a1_groups[] = {
	"gpio3",
};
static const char * const wci3_groups[] = {
	"gpio3", "gpio3",
};
static const char * const xfem3_groups[] = {
	"gpio3",
};
static const char * const sdc13_groups[] = {
	"gpio4",
};
static const char * const qspi3_groups[] = {
	"gpio4",
};
static const char * const blsp1_spi1_groups[] = {
	"gpio4", "gpio5", "gpio6", "gpio7",
};
static const char * const btss0_groups[] = {
	"gpio4",
};
static const char * const dbg_out_groups[] = {
	"gpio4",
};
static const char * const qdss_traceclk_a_groups[] = {
	"gpio4",
};
static const char * const burn0_groups[] = {
	"gpio4",
};
static const char * const sdc12_groups[] = {
	"gpio5",
};
static const char * const qspi2_groups[] = {
	"gpio5",
};
static const char * const cxc_clk_groups[] = {
	"gpio5",
};
static const char * const blsp1_i2c1_groups[] = {
	"gpio5", "gpio6",
};
static const char * const btss1_groups[] = {
	"gpio5",
};
static const char * const qdss_tracectl_a_groups[] = {
	"gpio5",
};
static const char * const burn1_groups[] = {
	"gpio5",
};
static const char * const sdc11_groups[] = {
	"gpio6",
};
static const char * const qspi1_groups[] = {
	"gpio6",
};
static const char * const cxc_data_groups[] = {
	"gpio6",
};
static const char * const btss2_groups[] = {
	"gpio6",
};
static const char * const qdss_tracedata_a_groups[] = {
	"gpio6", "gpio7", "gpio8", "gpio9", "gpio10", "gpio11", "gpio12",
	"gpio13", "gpio14", "gpio15", "gpio16", "gpio17", "gpio18", "gpio19",
	"gpio20", "gpio21",
};
static const char * const sdc10_groups[] = {
	"gpio7",
};
static const char * const qspi0_groups[] = {
	"gpio7",
};
static const char * const mac0_groups[] = {
	"gpio7",
};
static const char * const btss3_groups[] = {
	"gpio7",
};
static const char * const sdc1_cmd_groups[] = {
	"gpio8",
};
static const char * const qspi_cs_groups[] = {
	"gpio8",
};
static const char * const mac1_groups[] = {
	"gpio8",
};
static const char * const btss4_groups[] = {
	"gpio8",
};
static const char * const sdc1_clk_groups[] = {
	"gpio9",
};
static const char * const qspi_clk_groups[] = {
	"gpio9",
};
static const char * const blsp0_spi_groups[] = {
	"gpio10", "gpio11", "gpio12", "gpio13",
};
static const char * const blsp1_uart0_groups[] = {
	"gpio10", "gpio11", "gpio12", "gpio13",
};
static const char * const gcc_plltest_groups[] = {
	"gpio10", "gpio12",
};
static const char * const gcc_tlmm_groups[] = {
	"gpio11",
};
static const char * const blsp0_i2c_groups[] = {
	"gpio12", "gpio13",
};
static const char * const pcie0_clk_groups[] = {
	"gpio14",
};
static const char * const cri_trng0_groups[] = {
	"gpio14",
};
static const char * const cri_trng1_groups[] = {
	"gpio15",
};
static const char * const pcie0_wake_groups[] = {
	"gpio16",
};
static const char * const cri_trng_groups[] = {
	"gpio16",
};
static const char * const pcie1_clk_groups[] = {
	"gpio17",
};
static const char * const btss5_groups[] = {
	"gpio17",
};
static const char * const prng_rosc_groups[] = {
	"gpio17",
};
static const char * const blsp1_spi0_groups[] = {
	"gpio18", "gpio19", "gpio20", "gpio21",
};
static const char * const btss6_groups[] = {
	"gpio18",
};
static const char * const pcie1_wake_groups[] = {
	"gpio19",
};
static const char * const blsp1_i2c0_groups[] = {
	"gpio19", "gpio20",
};
static const char * const btss7_groups[] = {
	"gpio19",
};
static const char * const blsp0_uart0_groups[] = {
	"gpio20", "gpio21",
};
static const char * const pll_test_groups[] = {
	"gpio22",
};
static const char * const eud_gpio_groups[] = {
	"gpio22", "gpio31", "gpio32", "gpio33", "gpio34", "gpio35",
};
static const char * const audio_rxmclk_groups[] = {
	"gpio23", "gpio23",
};
static const char * const audio_pdm0_groups[] = {
	"gpio23", "gpio24",
};
static const char * const blsp2_spi1_groups[] = {
	"gpio23", "gpio24", "gpio25", "gpio26",
};
static const char * const blsp1_uart2_groups[] = {
	"gpio23", "gpio24", "gpio25", "gpio26",
};
static const char * const btss8_groups[] = {
	"gpio23",
};
static const char * const qdss_tracedata_b_groups[] = {
	"gpio23", "gpio24", "gpio25", "gpio26", "gpio27", "gpio28", "gpio29",
	"gpio30", "gpio31", "gpio32", "gpio33", "gpio34", "gpio35", "gpio36",
	"gpio37", "gpio38",
};
static const char * const audio_rxbclk_groups[] = {
	"gpio24",
};
static const char * const btss9_groups[] = {
	"gpio24",
};
static const char * const audio_rxfsync_groups[] = {
	"gpio25",
};
static const char * const audio_pdm1_groups[] = {
	"gpio25", "gpio26",
};
static const char * const blsp2_i2c1_groups[] = {
	"gpio25", "gpio26",
};
static const char * const btss10_groups[] = {
	"gpio25",
};
static const char * const audio_rxd_groups[] = {
	"gpio26",
};
static const char * const btss11_groups[] = {
	"gpio26",
};
static const char * const audio_txmclk_groups[] = {
	"gpio27", "gpio27",
};
static const char * const wsa_swrm_groups[] = {
	"gpio27", "gpio28",
};
static const char * const blsp2_spi_groups[] = {
	"gpio27",
};
static const char * const btss12_groups[] = {
	"gpio27",
};
static const char * const audio_txbclk_groups[] = {
	"gpio28",
};
static const char * const blsp0_uart1_groups[] = {
	"gpio28", "gpio29",
};
static const char * const btss13_groups[] = {
	"gpio28",
};
static const char * const audio_txfsync_groups[] = {
	"gpio29",
};
static const char * const audio_txd_groups[] = {
	"gpio30",
};
static const char * const wsis_reset_groups[] = {
	"gpio30",
};
static const char * const blsp2_spi0_groups[] = {
	"gpio31", "gpio32", "gpio33", "gpio34",
};
static const char * const blsp1_uart1_groups[] = {
	"gpio31", "gpio32", "gpio33", "gpio34",
};
static const char * const blsp2_i2c0_groups[] = {
	"gpio33", "gpio34",
};
static const char * const mdc_groups[] = {
	"gpio36",
};
static const char * const wsi_clk3_groups[] = {
	"gpio36",
};
static const char * const mdio_groups[] = {
	"gpio37",
};
static const char * const atest_char_groups[] = {
	"gpio37",
};
static const char * const wsi_data3_groups[] = {
	"gpio37",
};
static const char * const qdss_traceclk_b_groups[] = {
	"gpio39",
};
static const char * const reset_out_groups[] = {
	"gpio40",
};
static const char * const qdss_tracectl_b_groups[] = {
	"gpio40",
};
static const char * const pwm0_groups[] = {
	"gpio42",
};
static const char * const qdss_cti_trig_out_b0_groups[] = {
	"gpio42",
};
static const char * const wci4_groups[] = {
	"gpio42", "gpio42",
};
static const char * const xfem4_groups[] = {
	"gpio42",
};
static const char * const pwm1_groups[] = {
	"gpio43",
};
static const char * const qdss_cti_trig_in_b0_groups[] = {
	"gpio43",
};
static const char * const wci5_groups[] = {
	"gpio43", "gpio43",
};
static const char * const xfem5_groups[] = {
	"gpio43",
};
static const char * const pwm2_groups[] = {
	"gpio44",
};
static const char * const qdss_cti_trig_out_b1_groups[] = {
	"gpio44",
};
static const char * const wci6_groups[] = {
	"gpio44", "gpio44",
};
static const char * const xfem6_groups[] = {
	"gpio44",
};
static const char * const pwm3_groups[] = {
	"gpio45",
};
static const char * const qdss_cti_trig_in_b1_groups[] = {
	"gpio45",
};
static const char * const wci7_groups[] = {
	"gpio45", "gpio45",
};
static const char * const xfem7_groups[] = {
	"gpio45",
};
static const char * const led0_groups[] = {
	"gpio46", "gpio30",
};
static const char * const led2_groups[] = {
	"gpio30",
};

static const char * const gpio_groups[] = {
	"gpio0", "gpio1", "gpio2", "gpio3", "gpio4", "gpio5", "gpio6", "gpio7",
	"gpio8", "gpio9", "gpio10", "gpio11", "gpio12", "gpio13", "gpio14",
	"gpio15", "gpio16", "gpio17", "gpio18", "gpio19", "gpio20", "gpio21",
	"gpio22", "gpio23", "gpio24", "gpio25", "gpio26", "gpio27", "gpio28",
	"gpio29", "gpio30", "gpio31", "gpio32", "gpio33", "gpio34", "gpio35",
	"gpio36", "gpio37", "gpio38", "gpio39", "gpio40", "gpio41", "gpio42",
	"gpio43", "gpio44", "gpio45", "gpio46",
};

static const struct msm_function ipq5018_functions[] = {
	FUNCTION(gpio),
	FUNCTION(atest_char0),
	FUNCTION(),
	FUNCTION(wci0),
	FUNCTION(qdss_cti_trig_out_a0),
	FUNCTION(xfem0),
	FUNCTION(atest_char1),
	FUNCTION(qdss_cti_trig_in_a0),
	FUNCTION(wci1),
	FUNCTION(xfem1),
	FUNCTION(atest_char2),
	FUNCTION(qdss_cti_trig_out_a1),
	FUNCTION(wci2),
	FUNCTION(xfem2),
	FUNCTION(atest_char3),
	FUNCTION(qdss_cti_trig_in_a1),
	FUNCTION(wci3),
	FUNCTION(xfem3),
	FUNCTION(sdc13),
	FUNCTION(qspi3),
	FUNCTION(blsp1_spi1),
	FUNCTION(btss0),
	FUNCTION(dbg_out),
	FUNCTION(qdss_traceclk_a),
	FUNCTION(burn0),
	FUNCTION(sdc12),
	FUNCTION(qspi2),
	FUNCTION(cxc_clk),
	FUNCTION(blsp1_i2c1),
	FUNCTION(btss1),
	FUNCTION(qdss_tracectl_a),
	FUNCTION(burn1),
	FUNCTION(sdc11),
	FUNCTION(qspi1),
	FUNCTION(cxc_data),
	FUNCTION(btss2),
	FUNCTION(qdss_tracedata_a),
	FUNCTION(sdc10),
	FUNCTION(qspi0),
	FUNCTION(mac0),
	FUNCTION(btss3),
	FUNCTION(sdc1_cmd),
	FUNCTION(qspi_cs),
	FUNCTION(mac1),
	FUNCTION(btss4),
	FUNCTION(sdc1_clk),
	FUNCTION(qspi_clk),
	FUNCTION(blsp0_spi),
	FUNCTION(blsp1_uart0),
	FUNCTION(gcc_plltest),
	FUNCTION(gcc_tlmm),
	FUNCTION(blsp0_i2c),
	FUNCTION(pcie0_clk),
	FUNCTION(cri_trng0),
	FUNCTION(cri_trng1),
	FUNCTION(pcie0_wake),
	FUNCTION(cri_trng),
	FUNCTION(pcie1_clk),
	FUNCTION(btss5),
	FUNCTION(prng_rosc),
	FUNCTION(blsp1_spi0),
	FUNCTION(btss6),
	FUNCTION(pcie1_wake),
	FUNCTION(blsp1_i2c0),
	FUNCTION(btss7),
	FUNCTION(blsp0_uart0),
	FUNCTION(pll_test),
	FUNCTION(eud_gpio),
	FUNCTION(audio_rxmclk),
	FUNCTION(audio_pdm0),
	FUNCTION(blsp2_spi1),
	FUNCTION(blsp1_uart2),
	FUNCTION(btss8),
	FUNCTION(qdss_tracedata_b),
	FUNCTION(audio_rxbclk),
	FUNCTION(btss9),
	FUNCTION(audio_rxfsync),
	FUNCTION(audio_pdm1),
	FUNCTION(blsp2_i2c1),
	FUNCTION(btss10),
	FUNCTION(audio_rxd),
	FUNCTION(btss11),
	FUNCTION(audio_txmclk),
	FUNCTION(wsa_swrm),
	FUNCTION(blsp2_spi),
	FUNCTION(btss12),
	FUNCTION(audio_txbclk),
	FUNCTION(blsp0_uart1),
	FUNCTION(btss13),
	FUNCTION(audio_txfsync),
	FUNCTION(audio_txd),
	FUNCTION(wsis_reset),
	FUNCTION(blsp2_spi0),
	FUNCTION(blsp1_uart1),
	FUNCTION(blsp2_i2c0),
	FUNCTION(mdc),
	FUNCTION(wsi_clk3),
	FUNCTION(mdio),
	FUNCTION(atest_char),
	FUNCTION(wsi_data3),
	FUNCTION(qdss_traceclk_b),
	FUNCTION(reset_out),
	FUNCTION(qdss_tracectl_b),
	FUNCTION(pwm0),
	FUNCTION(qdss_cti_trig_out_b0),
	FUNCTION(wci4),
	FUNCTION(xfem4),
	FUNCTION(pwm1),
	FUNCTION(qdss_cti_trig_in_b0),
	FUNCTION(wci5),
	FUNCTION(xfem5),
	FUNCTION(pwm2),
	FUNCTION(qdss_cti_trig_out_b1),
	FUNCTION(wci6),
	FUNCTION(xfem6),
	FUNCTION(pwm3),
	FUNCTION(qdss_cti_trig_in_b1),
	FUNCTION(wci7),
	FUNCTION(xfem7),
	FUNCTION(led0),
	FUNCTION(led2),
};

static const struct msm_pingroup ipq5018_groups[] = {
	PINGROUP(0, atest_char0, NA, qdss_cti_trig_out_a0, wci0, wci0, xfem0,
		 NA, NA, NA),
	PINGROUP(1, atest_char1, NA, qdss_cti_trig_in_a0, wci1, wci1, xfem1,
		 NA, NA, NA),
	PINGROUP(2, atest_char2, NA, qdss_cti_trig_out_a1, wci2, wci2, xfem2,
		 NA, NA, NA),
	PINGROUP(3, atest_char3, NA, qdss_cti_trig_in_a1, wci3, wci3, xfem3,
		 NA, NA, NA),
	PINGROUP(4, sdc13, qspi3, blsp1_spi1, btss0, dbg_out, qdss_traceclk_a,
		 NA, burn0, NA),
	PINGROUP(5, sdc12, qspi2, cxc_clk, blsp1_spi1, blsp1_i2c1, btss1, NA,
		 qdss_tracectl_a, NA),
	PINGROUP(6, sdc11, qspi1, cxc_data, blsp1_spi1, blsp1_i2c1, btss2, NA,
		 qdss_tracedata_a, NA),
	PINGROUP(7, sdc10, qspi0, mac0, blsp1_spi1, btss3, NA,
		 qdss_tracedata_a, NA, NA),
	PINGROUP(8, sdc1_cmd, qspi_cs, mac1, btss4, NA, qdss_tracedata_a, NA,
		 NA, NA),
	PINGROUP(9, sdc1_clk, qspi_clk, NA, qdss_tracedata_a, NA, NA, NA, NA,
		 NA),
	PINGROUP(10, blsp0_spi, blsp1_uart0, NA, gcc_plltest, qdss_tracedata_a,
		 NA, NA, NA, NA),
	PINGROUP(11, blsp0_spi, blsp1_uart0, NA, gcc_tlmm, qdss_tracedata_a,
		 NA, NA, NA, NA),
	PINGROUP(12, blsp0_spi, blsp0_i2c, blsp1_uart0, NA, gcc_plltest,
		 qdss_tracedata_a, NA, NA, NA),
	PINGROUP(13, blsp0_spi, blsp0_i2c, blsp1_uart0, NA, qdss_tracedata_a,
		 NA, NA, NA, NA),
	PINGROUP(14, pcie0_clk, NA, NA, cri_trng0, qdss_tracedata_a, NA, NA,
		 NA, NA),
	PINGROUP(15, NA, NA, cri_trng1, qdss_tracedata_a, NA, NA, NA, NA, NA),
	PINGROUP(16, pcie0_wake, NA, NA, cri_trng, qdss_tracedata_a, NA, NA,
		 NA, NA),
	PINGROUP(17, pcie1_clk, btss5, NA, prng_rosc, qdss_tracedata_a, NA, NA,
		 NA, NA),
	PINGROUP(18, blsp1_spi0, btss6, NA, qdss_tracedata_a, NA, NA, NA, NA,
		 NA),
	PINGROUP(19, pcie1_wake, blsp1_spi0, blsp1_i2c0, btss7, NA,
		 qdss_tracedata_a, NA, NA, NA),
	PINGROUP(20, blsp0_uart0, blsp1_spi0, blsp1_i2c0, NA, qdss_tracedata_a,
		 NA, NA, NA, NA),
	PINGROUP(21, blsp0_uart0, blsp1_spi0, NA, qdss_tracedata_a, NA, NA, NA,
		 NA, NA),
	PINGROUP(22, NA, pll_test, eud_gpio, NA, NA, NA, NA, NA, NA),
	PINGROUP(23, audio_rxmclk, audio_pdm0, audio_rxmclk, blsp2_spi1,
		 blsp1_uart2, btss8, NA, qdss_tracedata_b, NA),
	PINGROUP(24, audio_rxbclk, audio_pdm0, blsp2_spi1, blsp1_uart2, btss9,
		 NA, qdss_tracedata_b, NA, NA),
	PINGROUP(25, audio_rxfsync, audio_pdm1, blsp2_i2c1, blsp2_spi1,
		 blsp1_uart2, btss10, NA, qdss_tracedata_b, NA),
	PINGROUP(26, audio_rxd, audio_pdm1, blsp2_i2c1, blsp2_spi1,
		 blsp1_uart2, btss11, NA, qdss_tracedata_b, NA),
	PINGROUP(27, audio_txmclk, wsa_swrm, audio_txmclk, blsp2_spi, btss12,
		 NA, qdss_tracedata_b, NA, NA),
	PINGROUP(28, audio_txbclk, wsa_swrm, blsp0_uart1, btss13,
		 qdss_tracedata_b, NA, NA, NA, NA),
	PINGROUP(29, audio_txfsync, NA, blsp0_uart1, NA, qdss_tracedata_b, NA,
		 NA, NA, NA),
	PINGROUP(30, audio_txd, led2, led0, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(31, blsp2_spi0, blsp1_uart1, NA, qdss_tracedata_b, eud_gpio,
		 NA, NA, NA, NA),
	PINGROUP(32, blsp2_spi0, blsp1_uart1, NA, qdss_tracedata_b, eud_gpio,
		 NA, NA, NA, NA),
	PINGROUP(33, blsp2_i2c0, blsp2_spi0, blsp1_uart1, NA, qdss_tracedata_b,
		 eud_gpio, NA, NA, NA),
	PINGROUP(34, blsp2_i2c0, blsp2_spi0, blsp1_uart1, NA, qdss_tracedata_b,
		 eud_gpio, NA, NA, NA),
	PINGROUP(35, NA, qdss_tracedata_b, eud_gpio, NA, NA, NA, NA, NA, NA),
	PINGROUP(36, mdc, qdss_tracedata_b, NA, wsi_clk3, NA, NA, NA, NA, NA),
	PINGROUP(37, mdio, atest_char, qdss_tracedata_b, NA, wsi_data3, NA, NA,
		 NA, NA),
	PINGROUP(38, qdss_tracedata_b, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(39, qdss_traceclk_b, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(40, reset_out, qdss_tracectl_b, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(41, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(42, pwm0, qdss_cti_trig_out_b0, wci4, wci4, xfem4, NA, NA, NA,
		 NA),
	PINGROUP(43, pwm1, qdss_cti_trig_in_b0, wci5, wci5, xfem5, NA, NA, NA,
		 NA),
	PINGROUP(44, pwm2, qdss_cti_trig_out_b1, wci6, wci6, xfem6, NA, NA, NA,
		 NA),
	PINGROUP(45, pwm3, qdss_cti_trig_in_b1, wci7, wci7, xfem7, NA, NA, NA,
		 NA),
	PINGROUP(46, led0, NA, NA, NA, NA, NA, NA, NA, NA),
};

static const struct msm_pinctrl_soc_data ipq5018_pinctrl = {
	.pins = ipq5018_pins,
	.npins = ARRAY_SIZE(ipq5018_pins),
	.functions = ipq5018_functions,
	.nfunctions = ARRAY_SIZE(ipq5018_functions),
	.groups = ipq5018_groups,
	.ngroups = ARRAY_SIZE(ipq5018_groups),
	.ngpios = 47,
	.gpio_pull = &msm_gpio_pull,
};

static int ipq5018_pinctrl_probe(struct platform_device *pdev)
{
	return msm_pinctrl_probe(pdev, &ipq5018_pinctrl);
}

static const struct of_device_id ipq5018_pinctrl_of_match[] = {
	{ .compatible = "qcom,ipq5018-pinctrl", },
	{ },
};

static struct platform_driver ipq5018_pinctrl_driver = {
	.driver = {
		.name = "ipq5018-pinctrl",
		.owner = THIS_MODULE,
		.of_match_table = ipq5018_pinctrl_of_match,
	},
	.probe = ipq5018_pinctrl_probe,
	.remove = msm_pinctrl_remove,
};

static int __init ipq5018_pinctrl_init(void)
{
	return platform_driver_register(&ipq5018_pinctrl_driver);
}
arch_initcall(ipq5018_pinctrl_init);

static void __exit ipq5018_pinctrl_exit(void)
{
	platform_driver_unregister(&ipq5018_pinctrl_driver);
}
module_exit(ipq5018_pinctrl_exit);

MODULE_DESCRIPTION("Qualcomm Technologies Inc ipq5018 pinctrl driver");
MODULE_LICENSE("GPL v2");
MODULE_DEVICE_TABLE(of, ipq5018_pinctrl_of_match);
