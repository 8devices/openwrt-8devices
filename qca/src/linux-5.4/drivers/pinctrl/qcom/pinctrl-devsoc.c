// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2016-2018,2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
		.npins = (unsigned int)ARRAY_SIZE(gpio##id##_pins),	\
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
		.intr_target_kpss_val = 3,	\
		.intr_raw_status_bit = 4,	\
		.intr_polarity_bit = 1,		\
		.intr_detection_bit = 2,	\
		.intr_detection_width = 2,	\
	}

static const struct pinctrl_pin_desc devsoc_pins[] = {
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
	PINCTRL_PIN(47, "GPIO_47"),
	PINCTRL_PIN(48, "GPIO_48"),
	PINCTRL_PIN(49, "GPIO_49"),
	PINCTRL_PIN(50, "GPIO_50"),
	PINCTRL_PIN(51, "GPIO_51"),
	PINCTRL_PIN(52, "GPIO_52"),
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
DECLARE_MSM_GPIO_PINS(47);
DECLARE_MSM_GPIO_PINS(48);
DECLARE_MSM_GPIO_PINS(49);
DECLARE_MSM_GPIO_PINS(50);
DECLARE_MSM_GPIO_PINS(51);
DECLARE_MSM_GPIO_PINS(52);

enum devsoc_functions {
	msm_mux_PTA_0,
	msm_mux_PTA_1,
	msm_mux_PTA_2,
	msm_mux_atest_char,
	msm_mux_atest_char0,
	msm_mux_atest_char1,
	msm_mux_atest_char2,
	msm_mux_atest_char3,
	msm_mux_atest_tic,
	msm_mux_audio_pri,
	msm_mux_audio_pri0,
	msm_mux_audio_pri1,
	msm_mux_audio_sec,
	msm_mux_audio_sec0,
	msm_mux_audio_sec1,
	msm_mux_blsp0_i2c,
	msm_mux_blsp0_spi,
	msm_mux_blsp0_uart0,
	msm_mux_blsp0_uart1,
	msm_mux_blsp1_i2c0,
	msm_mux_blsp1_i2c1,
	msm_mux_blsp1_spi0,
	msm_mux_blsp1_spi1,
	msm_mux_blsp1_uart0,
	msm_mux_blsp1_uart1,
	msm_mux_blsp1_uart2,
	msm_mux_blsp2_i2c0,
	msm_mux_blsp2_i2c1,
	msm_mux_blsp2_spi,
	msm_mux_blsp2_spi0,
	msm_mux_blsp2_spi1,
	msm_mux_core_voltage,
	msm_mux_cri_trng0,
	msm_mux_cri_trng1,
	msm_mux_cri_trng2,
	msm_mux_cri_trng3,
	msm_mux_cxc_clk,
	msm_mux_cxc_data,
	msm_mux_dbg_out,
	msm_mux_eud_gpio,
	msm_mux_gcc_plltest,
	msm_mux_gcc_tlmm,
	msm_mux_gpio,
	msm_mux_mac0,
	msm_mux_mac1,
	msm_mux_mdc0,
	msm_mux_mdc1,
	msm_mux_mdio0,
	msm_mux_mdio1,
	msm_mux_pcie0_clk,
	msm_mux_pcie0_wake,
	msm_mux_pcie1_clk,
	msm_mux_pcie1_wake,
	msm_mux_pcie2_clk,
	msm_mux_pcie2_wake,
	msm_mux_pll_test,
	msm_mux_prng_rosc0,
	msm_mux_prng_rosc1,
	msm_mux_prng_rosc2,
	msm_mux_prng_rosc3,
	msm_mux_pwm0,
	msm_mux_pwm1,
	msm_mux_pwm2,
	msm_mux_pwm3,
	msm_mux_qdss_cti_trig_in_a0,
	msm_mux_qdss_cti_trig_in_a1,
	msm_mux_qdss_cti_trig_in_b0,
	msm_mux_qdss_cti_trig_in_b1,
	msm_mux_qdss_cti_trig_out_a0,
	msm_mux_qdss_cti_trig_out_a1,
	msm_mux_qdss_cti_trig_out_b0,
	msm_mux_qdss_cti_trig_out_b1,
	msm_mux_qdss_traceclk_a,
	msm_mux_qdss_traceclk_b,
	msm_mux_qdss_tracectl_a,
	msm_mux_qdss_tracectl_b,
	msm_mux_qdss_tracedata_a,
	msm_mux_qdss_tracedata_b,
	msm_mux_qspi_data,
	msm_mux_qspi_clk,
	msm_mux_qspi_cs,
	msm_mux_resout,
	msm_mux_rx0,
	msm_mux_rx1,
	msm_mux_sdc_data,
	msm_mux_sdc_clk,
	msm_mux_sdc_cmd,
	msm_mux_tsens_max,
	msm_mux_wci0,
	msm_mux_wci1,
	msm_mux_wci10,
	msm_mux_wci11,
	msm_mux_wci2,
	msm_mux_wci3,
	msm_mux_wci4,
	msm_mux_wci5,
	msm_mux_wci6,
	msm_mux_wci7,
	msm_mux_wci8,
	msm_mux_wci9,
	msm_mux_wsi_clk,
	msm_mux_wsi_clk3,
	msm_mux_wsi_data,
	msm_mux_wsi_data3,
	msm_mux_wsis_reset,
	msm_mux_xfem0,
	msm_mux_xfem1,
	msm_mux_xfem2,
	msm_mux_xfem3,
	msm_mux_xfem4,
	msm_mux_xfem5,
	msm_mux_xfem6,
	msm_mux_xfem7,
	msm_mux__,
};

static const char * const gpio_groups[] = {
	"gpio0", "gpio1", "gpio2", "gpio3", "gpio4", "gpio5", "gpio6", "gpio7",
	"gpio8", "gpio9", "gpio10", "gpio11", "gpio12", "gpio13", "gpio14",
	"gpio15", "gpio16", "gpio17", "gpio18", "gpio19", "gpio20", "gpio21",
	"gpio22", "gpio23", "gpio24", "gpio25", "gpio26", "gpio27", "gpio28",
	"gpio29", "gpio30", "gpio31", "gpio32", "gpio33", "gpio34", "gpio35",
	"gpio36", "gpio37", "gpio38", "gpio39", "gpio40", "gpio41", "gpio42",
	"gpio43", "gpio44", "gpio45", "gpio46", "gpio47", "gpio48", "gpio49",
	"gpio50", "gpio51", "gpio52",
};
static const char * const PTA_0_groups[] = {
	"gpio49",
};
static const char * const PTA_1_groups[] = {
	"gpio50",
};
static const char * const PTA_2_groups[] = {
	"gpio51",
};
static const char * const atest_char_groups[] = {
	"gpio46",
};
static const char * const atest_char0_groups[] = {
	"gpio0",
};
static const char * const atest_char1_groups[] = {
	"gpio1",
};
static const char * const atest_char2_groups[] = {
	"gpio2",
};
static const char * const atest_char3_groups[] = {
	"gpio3",
};
static const char * const atest_tic_groups[] = {
	"gpio9",
};
static const char * const audio_pri_groups[] = {
	"gpio29", "gpio30", "gpio31", "gpio32",
};
static const char * const audio_pri0_groups[] = {
	"gpio34", "gpio34",
};
static const char * const audio_pri1_groups[] = {
	"gpio43", "gpio43",
};
static const char * const audio_sec_groups[] = {
	"gpio33", "gpio34", "gpio35", "gpio36",
};
static const char * const audio_sec0_groups[] = {
	"gpio30", "gpio30",
};
static const char * const audio_sec1_groups[] = {
	"gpio45", "gpio45",
};
static const char * const blsp0_i2c_groups[] = {
	"gpio16", "gpio17",
};
static const char * const blsp0_spi_groups[] = {
	"gpio14", "gpio15", "gpio16", "gpio17",
};
static const char * const blsp0_uart0_groups[] = {
	"gpio18", "gpio19",
};
static const char * const blsp0_uart1_groups[] = {
	"gpio27", "gpio28",
};
static const char * const blsp1_i2c0_groups[] = {
	"gpio29", "gpio30",
};
static const char * const blsp1_i2c1_groups[] = {
	"gpio40", "gpio41",
};
static const char * const blsp1_spi0_groups[] = {
	"gpio29", "gpio30", "gpio31", "gpio32",
};
static const char * const blsp1_spi1_groups[] = {
	"gpio25", "gpio26", "gpio27", "gpio28",
};
static const char * const blsp1_uart0_groups[] = {
	"gpio14", "gpio15", "gpio16", "gpio17",
};
static const char * const blsp1_uart1_groups[] = {
	"gpio25", "gpio26", "gpio27", "gpio28",
};
static const char * const blsp1_uart2_groups[] = {
	"gpio33", "gpio34", "gpio35", "gpio36",
};
static const char * const blsp2_i2c0_groups[] = {
	"gpio43", "gpio45",
};
static const char * const blsp2_i2c1_groups[] = {
	"gpio33", "gpio34",
};
static const char * const blsp2_spi_groups[] = {
	"gpio37",
};
static const char * const blsp2_spi0_groups[] = {
	"gpio33", "gpio34", "gpio35", "gpio36",
};
static const char * const blsp2_spi1_groups[] = {
	"gpio40", "gpio41", "gpio42", "gpio52",
};
static const char * const core_voltage_groups[] = {
	"gpio21", "gpio23",
};
static const char * const cri_trng0_groups[] = {
	"gpio17",
};
static const char * const cri_trng1_groups[] = {
	"gpio18",
};
static const char * const cri_trng2_groups[] = {
	"gpio19",
};
static const char * const cri_trng3_groups[] = {
	"gpio20",
};
static const char * const cxc_clk_groups[] = {
	"gpio49",
};
static const char * const cxc_data_groups[] = {
	"gpio50",
};
static const char * const dbg_out_groups[] = {
	"gpio48",
};
static const char * const eud_gpio_groups[] = {
	"gpio33", "gpio34", "gpio35", "gpio36", "gpio37", "gpio39",
};
static const char * const gcc_plltest_groups[] = {
	"gpio43", "gpio45",
};
static const char * const gcc_tlmm_groups[] = {
	"gpio44",
};
static const char * const mac0_groups[] = {
	"gpio18",
};
static const char * const mac1_groups[] = {
	"gpio19",
};
static const char * const mdc0_groups[] = {
	"gpio25",
};
static const char * const mdc1_groups[] = {
	"gpio27",
};
static const char * const mdio0_groups[] = {
	"gpio26",
};
static const char * const mdio1_groups[] = {
	"gpio28",
};
static const char * const pcie0_clk_groups[] = {
	"gpio37",
};
static const char * const pcie0_wake_groups[] = {
	"gpio39",
};
static const char * const pcie1_clk_groups[] = {
	"gpio46",
};
static const char * const pcie1_wake_groups[] = {
	"gpio48",
};
static const char * const pcie2_clk_groups[] = {
	"gpio43",
};
static const char * const pcie2_wake_groups[] = {
	"gpio45",
};
static const char * const pll_test_groups[] = {
	"gpio48",
};
static const char * const prng_rosc0_groups[] = {
	"gpio22",
};
static const char * const prng_rosc1_groups[] = {
	"gpio24",
};
static const char * const prng_rosc2_groups[] = {
	"gpio25",
};
static const char * const prng_rosc3_groups[] = {
	"gpio26",
};
static const char * const pwm0_groups[] = {
	"gpio43", "gpio44", "gpio45", "gpio46",
};
static const char * const pwm1_groups[] = {
	"gpio29", "gpio30", "gpio31", "gpio32",
};
static const char * const pwm2_groups[] = {
	"gpio25", "gpio26", "gpio27", "gpio28",
};
static const char * const pwm3_groups[] = {
	"gpio8", "gpio9", "gpio10", "gpio11",
};
static const char * const qdss_cti_trig_in_a0_groups[] = {
	"gpio5",
};
static const char * const qdss_cti_trig_in_a1_groups[] = {
	"gpio7",
};
static const char * const qdss_cti_trig_in_b0_groups[] = {
	"gpio47",
};
static const char * const qdss_cti_trig_in_b1_groups[] = {
	"gpio49",
};
static const char * const qdss_cti_trig_out_a0_groups[] = {
	"gpio4",
};
static const char * const qdss_cti_trig_out_a1_groups[] = {
	"gpio6",
};
static const char * const qdss_cti_trig_out_b0_groups[] = {
	"gpio46",
};
static const char * const qdss_cti_trig_out_b1_groups[] = {
	"gpio48",
};
static const char * const qdss_traceclk_a_groups[] = {
	"gpio8",
};
static const char * const qdss_traceclk_b_groups[] = {
	"gpio45",
};
static const char * const qdss_tracectl_a_groups[] = {
	"gpio9",
};
static const char * const qdss_tracectl_b_groups[] = {
	"gpio44",
};
static const char * const qdss_tracedata_a_groups[] = {
	"gpio10", "gpio11", "gpio12", "gpio13", "gpio14", "gpio15", "gpio16",
	"gpio17", "gpio18", "gpio19", "gpio20", "gpio22", "gpio24", "gpio25",
	"gpio26", "gpio27",
};
static const char * const qdss_tracedata_b_groups[] = {
	"gpio29", "gpio30", "gpio31", "gpio32", "gpio33", "gpio34", "gpio35",
	"gpio36", "gpio37", "gpio38", "gpio39", "gpio40", "gpio41", "gpio42",
	"gpio43", "gpio52",
};
static const char * const qspi_clk_groups[] = {
	"gpio13",
};
static const char * const qspi_cs_groups[] = {
	"gpio12",
};
static const char * const qspi_data_groups[] = {
	"gpio8",
	"gpio9",
	"gpio10",
	"gpio11",
};
static const char * const resout_groups[] = {
	"gpio20",
};
static const char * const rx0_groups[] = {
	"gpio48",
};
static const char * const rx1_groups[] = {
	"gpio45",
};
static const char * const sdc_clk_groups[] = {
	"gpio13",
};
static const char * const sdc_cmd_groups[] = {
	"gpio12",
};
static const char * const sdc_data_groups[] = {
	"gpio8",
	"gpio9",
	"gpio10",
	"gpio11",
};
static const char * const tsens_max_groups[] = {
	"gpio28",
};
static const char * const wci0_groups[] = {
	"gpio0", "gpio0",
};
static const char * const wci1_groups[] = {
	"gpio1", "gpio1",
};
static const char * const wci10_groups[] = {
	"gpio35", "gpio35",
};
static const char * const wci11_groups[] = {
	"gpio36", "gpio36",
};
static const char * const wci2_groups[] = {
	"gpio2", "gpio2",
};
static const char * const wci3_groups[] = {
	"gpio3", "gpio3",
};
static const char * const wci4_groups[] = {
	"gpio4", "gpio4",
};
static const char * const wci5_groups[] = {
	"gpio5", "gpio5",
};
static const char * const wci6_groups[] = {
	"gpio6", "gpio6",
};
static const char * const wci7_groups[] = {
	"gpio7", "gpio7",
};
static const char * const wci8_groups[] = {
	"gpio43", "gpio43",
};
static const char * const wci9_groups[] = {
	"gpio45", "gpio45",
};
static const char * const wsi_clk_groups[] = {
	"gpio40", "gpio42",
};
static const char * const wsi_clk3_groups[] = {
	"gpio43",
};
static const char * const wsi_data_groups[] = {
	"gpio41", "gpio52",
};
static const char * const wsi_data3_groups[] = {
	"gpio44",
};
static const char * const wsis_reset_groups[] = {
	"gpio41",
};
static const char * const xfem0_groups[] = {
	"gpio0",
};
static const char * const xfem1_groups[] = {
	"gpio1",
};
static const char * const xfem2_groups[] = {
	"gpio2",
};
static const char * const xfem3_groups[] = {
	"gpio3",
};
static const char * const xfem4_groups[] = {
	"gpio4",
};
static const char * const xfem5_groups[] = {
	"gpio5",
};
static const char * const xfem6_groups[] = {
	"gpio6",
};
static const char * const xfem7_groups[] = {
	"gpio7",
};

static const struct msm_function devsoc_functions[] = {
	FUNCTION(PTA_0),
	FUNCTION(PTA_1),
	FUNCTION(PTA_2),
	FUNCTION(atest_char),
	FUNCTION(atest_char0),
	FUNCTION(atest_char1),
	FUNCTION(atest_char2),
	FUNCTION(atest_char3),
	FUNCTION(atest_tic),
	FUNCTION(audio_pri),
	FUNCTION(audio_pri0),
	FUNCTION(audio_pri1),
	FUNCTION(audio_sec),
	FUNCTION(audio_sec0),
	FUNCTION(audio_sec1),
	FUNCTION(blsp0_i2c),
	FUNCTION(blsp0_spi),
	FUNCTION(blsp0_uart0),
	FUNCTION(blsp0_uart1),
	FUNCTION(blsp1_i2c0),
	FUNCTION(blsp1_i2c1),
	FUNCTION(blsp1_spi0),
	FUNCTION(blsp1_spi1),
	FUNCTION(blsp1_uart0),
	FUNCTION(blsp1_uart1),
	FUNCTION(blsp1_uart2),
	FUNCTION(blsp2_i2c0),
	FUNCTION(blsp2_i2c1),
	FUNCTION(blsp2_spi),
	FUNCTION(blsp2_spi0),
	FUNCTION(blsp2_spi1),
	FUNCTION(core_voltage),
	FUNCTION(cri_trng0),
	FUNCTION(cri_trng1),
	FUNCTION(cri_trng2),
	FUNCTION(cri_trng3),
	FUNCTION(cxc_clk),
	FUNCTION(cxc_data),
	FUNCTION(dbg_out),
	FUNCTION(eud_gpio),
	FUNCTION(gcc_plltest),
	FUNCTION(gcc_tlmm),
	FUNCTION(gpio),
	FUNCTION(mac0),
	FUNCTION(mac1),
	FUNCTION(mdc0),
	FUNCTION(mdc1),
	FUNCTION(mdio0),
	FUNCTION(mdio1),
	FUNCTION(pcie0_clk),
	FUNCTION(pcie0_wake),
	FUNCTION(pcie1_clk),
	FUNCTION(pcie1_wake),
	FUNCTION(pcie2_clk),
	FUNCTION(pcie2_wake),
	FUNCTION(pll_test),
	FUNCTION(prng_rosc0),
	FUNCTION(prng_rosc1),
	FUNCTION(prng_rosc2),
	FUNCTION(prng_rosc3),
	FUNCTION(pwm0),
	FUNCTION(pwm1),
	FUNCTION(pwm2),
	FUNCTION(pwm3),
	FUNCTION(qdss_cti_trig_in_a0),
	FUNCTION(qdss_cti_trig_in_a1),
	FUNCTION(qdss_cti_trig_in_b0),
	FUNCTION(qdss_cti_trig_in_b1),
	FUNCTION(qdss_cti_trig_out_a0),
	FUNCTION(qdss_cti_trig_out_a1),
	FUNCTION(qdss_cti_trig_out_b0),
	FUNCTION(qdss_cti_trig_out_b1),
	FUNCTION(qdss_traceclk_a),
	FUNCTION(qdss_traceclk_b),
	FUNCTION(qdss_tracectl_a),
	FUNCTION(qdss_tracectl_b),
	FUNCTION(qdss_tracedata_a),
	FUNCTION(qdss_tracedata_b),
	FUNCTION(qspi_data),
	FUNCTION(qspi_clk),
	FUNCTION(qspi_cs),
	FUNCTION(resout),
	FUNCTION(rx0),
	FUNCTION(rx1),
	FUNCTION(sdc_data),
	FUNCTION(sdc_clk),
	FUNCTION(sdc_cmd),
	FUNCTION(tsens_max),
	FUNCTION(wci0),
	FUNCTION(wci1),
	FUNCTION(wci10),
	FUNCTION(wci11),
	FUNCTION(wci2),
	FUNCTION(wci3),
	FUNCTION(wci4),
	FUNCTION(wci5),
	FUNCTION(wci6),
	FUNCTION(wci7),
	FUNCTION(wci8),
	FUNCTION(wci9),
	FUNCTION(wsi_clk),
	FUNCTION(wsi_clk3),
	FUNCTION(wsi_data),
	FUNCTION(wsi_data3),
	FUNCTION(wsis_reset),
	FUNCTION(xfem0),
	FUNCTION(xfem1),
	FUNCTION(xfem2),
	FUNCTION(xfem3),
	FUNCTION(xfem4),
	FUNCTION(xfem5),
	FUNCTION(xfem6),
	FUNCTION(xfem7),
};

static const struct msm_pingroup devsoc_groups[] = {
	PINGROUP(0, atest_char0, wci0, wci0, xfem0, _, _, _, _, _),
	PINGROUP(1, atest_char1, wci1, wci1, xfem1, _, _, _, _, _),
	PINGROUP(2, atest_char2, wci2, wci2, xfem2, _, _, _, _, _),
	PINGROUP(3, atest_char3, wci3, wci3, xfem3, _, _, _, _, _),
	PINGROUP(4, qdss_cti_trig_out_a0, wci4, wci4, xfem4, _, _, _, _,
		 _),
	PINGROUP(5, qdss_cti_trig_in_a0, wci5, wci5, xfem5, _, _, _, _, _),
	PINGROUP(6, qdss_cti_trig_out_a1, wci6, wci6, xfem6, _, _, _, _,
		 _),
	PINGROUP(7, qdss_cti_trig_in_a1, wci7, wci7, xfem7, _, _, _, _, _),
	PINGROUP(8, sdc_data, qspi_data, pwm3, qdss_traceclk_a, _, _, _, _, _),
	PINGROUP(9, sdc_data, qspi_data, pwm3, qdss_tracectl_a, _, atest_tic, _, _,
		 _),
	PINGROUP(10, sdc_data, qspi_data, pwm3, qdss_tracedata_a, _, _, _, _, _),
	PINGROUP(11, sdc_data, qspi_data, pwm3, qdss_tracedata_a, _, _, _, _, _),
	PINGROUP(12, sdc_cmd, qspi_cs, qdss_tracedata_a, _, _, _, _, _,
		 _),
	PINGROUP(13, sdc_clk, qspi_clk, qdss_tracedata_a, _, _, _, _, _,
		 _),
	PINGROUP(14, blsp0_spi, blsp1_uart0, qdss_tracedata_a, _, _, _, _,
		 _, _),
	PINGROUP(15, blsp0_spi, blsp1_uart0, qdss_tracedata_a, _, _, _, _,
		 _, _),
	PINGROUP(16, blsp0_spi, blsp0_i2c, blsp1_uart0, _, qdss_tracedata_a,
		 _, _, _, _),
	PINGROUP(17, blsp0_spi, blsp0_i2c, blsp1_uart0, _, cri_trng0,
		 qdss_tracedata_a, _, _, _),
	PINGROUP(18, blsp0_uart0, mac0, _, cri_trng1, qdss_tracedata_a, _,
		 _, _, _),
	PINGROUP(19, blsp0_uart0, mac1, _, cri_trng2, qdss_tracedata_a, _,
		 _, _, _),
	PINGROUP(20, resout, _, cri_trng3, qdss_tracedata_a, _, _, _, _,
		 _),
	PINGROUP(21, core_voltage, _, _, _, _, _, _, _, _),
	PINGROUP(22, _, prng_rosc0, qdss_tracedata_a, _, _, _, _, _, _),
	PINGROUP(23, core_voltage, _, _, _, _, _, _, _, _),
	PINGROUP(24, _, prng_rosc1, qdss_tracedata_a, _, _, _, _, _, _),
	PINGROUP(25, mdc0, blsp1_uart1, blsp1_spi1, pwm2, _, _, prng_rosc2,
		 qdss_tracedata_a, _),
	PINGROUP(26, mdio0, blsp1_uart1, blsp1_spi1, pwm2, _, _, prng_rosc3,
		 qdss_tracedata_a, _),
	PINGROUP(27, mdc1, blsp0_uart1, blsp1_uart1, blsp1_spi1, pwm2, _, _,
		 qdss_tracedata_a, _),
	PINGROUP(28, mdio1, blsp0_uart1, blsp1_uart1, blsp1_spi1, pwm2, _,
		 tsens_max, _, _),
	PINGROUP(29, audio_pri, blsp1_spi0, blsp1_i2c0, pwm1, _,
		 qdss_tracedata_b, _, _, _),
	PINGROUP(30, audio_pri, blsp1_spi0, blsp1_i2c0, pwm1, audio_sec0,
		 audio_sec0, _, qdss_tracedata_b, _),
	PINGROUP(31, audio_pri, blsp1_spi0, pwm1, _, qdss_tracedata_b, _, _,
		 _, _),
	PINGROUP(32, audio_pri, blsp1_spi0, pwm1, _, qdss_tracedata_b, _, _,
		 _, _),
	PINGROUP(33, audio_sec, eud_gpio, blsp1_uart2, blsp2_i2c1, blsp2_spi0,
		 _, qdss_tracedata_b, _, _),
	PINGROUP(34, audio_sec, eud_gpio, blsp1_uart2, blsp2_i2c1, blsp2_spi0,
		 audio_pri0, audio_pri0, _, qdss_tracedata_b),
	PINGROUP(35, audio_sec, eud_gpio, blsp1_uart2, wci10, wci10,
		 blsp2_spi0, _, qdss_tracedata_b, _),
	PINGROUP(36, audio_sec, eud_gpio, blsp1_uart2, wci11, wci11,
		 blsp2_spi0, _, qdss_tracedata_b, _),
	PINGROUP(37, pcie0_clk, eud_gpio, blsp2_spi, _, qdss_tracedata_b, _,
		 _, _, _),
	PINGROUP(38, _, qdss_tracedata_b, _, _, _, _, _, _, _),
	PINGROUP(39, pcie0_wake, eud_gpio, _, qdss_tracedata_b, _, _, _,
		 _, _),
	PINGROUP(40, wsi_clk, blsp1_i2c1, blsp2_spi1, _, _, qdss_tracedata_b,
		 _, _, _),
	PINGROUP(41, wsi_data, blsp1_i2c1, blsp2_spi1, _, _,
		 qdss_tracedata_b, _, wsis_reset, _),
	PINGROUP(42, wsi_clk, blsp2_spi1, _, qdss_tracedata_b, _, _, _, _,
		 _),
	PINGROUP(43, pcie2_clk, wci8, wci8, blsp2_i2c0, pwm0, audio_pri1,
		 audio_pri1, _, gcc_plltest),
	PINGROUP(44, pwm0, _, gcc_tlmm, qdss_tracectl_b, _, wsi_data3, _,
		 _, _),
	PINGROUP(45, pcie2_wake, wci9, wci9, blsp2_i2c0, rx1, pwm0, audio_sec1,
		 audio_sec1, _),
	PINGROUP(46, pcie1_clk, atest_char, pwm0, _, qdss_cti_trig_out_b0, _,
		 _, _, _),
	PINGROUP(47, _, qdss_cti_trig_in_b0, _, _, _, _, _, _, _),
	PINGROUP(48, pcie1_wake, pll_test, rx0, dbg_out, qdss_cti_trig_out_b1,
		 _, _, _, _),
	PINGROUP(49, PTA_0, cxc_clk, _, qdss_cti_trig_in_b1, _, _, _, _,
		 _),
	PINGROUP(50, PTA_1, cxc_data, _, _, _, _, _, _, _),
	PINGROUP(51, PTA_2, _, _, _, _, _, _, _, _),
	PINGROUP(52, wsi_data, blsp2_spi1, _, qdss_tracedata_b, _, _, _,
		 _, _),
};

static const struct msm_pinctrl_soc_data devsoc_pinctrl = {
	.pins = devsoc_pins,
	.npins = ARRAY_SIZE(devsoc_pins),
	.functions = devsoc_functions,
	.nfunctions = ARRAY_SIZE(devsoc_functions),
	.groups = devsoc_groups,
	.ngroups = ARRAY_SIZE(devsoc_groups),
	.ngpios = 53,
};

static int devsoc_pinctrl_probe(struct platform_device *pdev)
{
	return msm_pinctrl_probe(pdev, &devsoc_pinctrl);
}

static const struct of_device_id devsoc_pinctrl_of_match[] = {
	{ .compatible = "qcom,devsoc-pinctrl", },
	{ },
};

static struct platform_driver devsoc_pinctrl_driver = {
	.driver = {
		.name = "devsoc-pinctrl",
		.of_match_table = devsoc_pinctrl_of_match,
	},
	.probe = devsoc_pinctrl_probe,
	.remove = msm_pinctrl_remove,
};

static int __init devsoc_pinctrl_init(void)
{
	return platform_driver_register(&devsoc_pinctrl_driver);
}
arch_initcall(devsoc_pinctrl_init);

static void __exit devsoc_pinctrl_exit(void)
{
	platform_driver_unregister(&devsoc_pinctrl_driver);
}
module_exit(devsoc_pinctrl_exit);

MODULE_DESCRIPTION("QTI devsoc pinctrl driver");
MODULE_LICENSE("GPL v2");
MODULE_DEVICE_TABLE(of, devsoc_pinctrl_of_match);
