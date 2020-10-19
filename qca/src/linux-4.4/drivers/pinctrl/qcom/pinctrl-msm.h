/*
 * Copyright (c) 2013, Sony Mobile Communications AB.
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
#ifndef __PINCTRL_MSM_H__
#define __PINCTRL_MSM_H__

struct pinctrl_pin_desc;

/**
 * struct msm_function - a pinmux function
 * @name:    Name of the pinmux function.
 * @groups:  List of pingroups for this function.
 * @ngroups: Number of entries in @groups.
 */
struct msm_function {
	const char *name;
	const char * const *groups;
	unsigned ngroups;
};

/**
 * struct msm_pingroup - Qualcomm pingroup definition
 * @name:                 Name of the pingroup.
 * @pins:	          A list of pins assigned to this pingroup.
 * @npins:	          Number of entries in @pins.
 * @funcs:                A list of pinmux functions that can be selected for
 *                        this group. The index of the selected function is used
 *                        for programming the function selector.
 *                        Entries should be indices into the groups list of the
 *                        struct msm_pinctrl_soc_data.
 * @ctl_reg:              Offset of the register holding control bits for this group.
 * @io_reg:               Offset of the register holding input/output bits for this group.
 * @intr_cfg_reg:         Offset of the register holding interrupt configuration bits.
 * @intr_status_reg:      Offset of the register holding the status bits for this group.
 * @intr_target_reg:      Offset of the register specifying routing of the interrupts
 *                        from this group.
 * @mux_bit:              Offset in @ctl_reg for the pinmux function selection.
 * @pull_bit:             Offset in @ctl_reg for the bias configuration.
 * @drv_bit:              Offset in @ctl_reg for the drive strength configuration.
 * @oe_bit:               Offset in @ctl_reg for controlling output enable.
 * @in_bit:               Offset in @io_reg for the input bit value.
 * @out_bit:              Offset in @io_reg for the output bit value.
 * @od_bit:		  Offset in @io_reg for controlling output drain.
 * @intr_enable_bit:      Offset in @intr_cfg_reg for enabling the interrupt for this group.
 * @intr_status_bit:      Offset in @intr_status_reg for reading and acking the interrupt
 *                        status.
 * @intr_target_bit:      Offset in @intr_target_reg for configuring the interrupt routing.
 * @intr_target_kpss_val: Value in @intr_target_bit for specifying that the interrupt from
 *                        this gpio should get routed to the KPSS processor.
 * @intr_raw_status_bit:  Offset in @intr_cfg_reg for the raw status bit.
 * @intr_polarity_bit:    Offset in @intr_cfg_reg for specifying polarity of the interrupt.
 * @intr_detection_bit:   Offset in @intr_cfg_reg for specifying interrupt type.
 * @intr_detection_width: Number of bits used for specifying interrupt type,
 *                        Should be 2 for SoCs that can detect both edges in hardware,
 *                        otherwise 1.
 */
struct msm_pingroup {
	const char *name;
	const unsigned *pins;
	unsigned npins;

	unsigned *funcs;
	unsigned nfuncs;

	u32 ctl_reg;
	u32 io_reg;
	u32 intr_cfg_reg;
	u32 intr_status_reg;
	u32 intr_target_reg;

	unsigned mux_bit:5;

	unsigned pull_bit:5;
	unsigned drv_bit:5;

	unsigned oe_bit:5;
	unsigned in_bit:5;
	unsigned out_bit:5;
	unsigned od_bit:5;
	unsigned vm_bit:5;
	unsigned pull_res:5;

	unsigned intr_enable_bit:5;
	unsigned intr_status_bit:5;
	unsigned intr_ack_high:1;

	unsigned intr_target_bit:5;
	unsigned intr_target_kpss_val:5;
	unsigned intr_raw_status_bit:5;
	unsigned intr_polarity_bit:5;
	unsigned intr_detection_bit:5;
	unsigned intr_detection_width:5;
};

/**
 * struct msm_pinctrl_gpio_pull - pinctrl pull value bit field descriptor
 */
struct msm_pinctrl_gpio_pull {
	unsigned no_pull;
	unsigned pull_down;
	unsigned pull_up;
	unsigned keeper;
};

/**
 * struct msm_pinctrl_soc_data - Qualcomm pin controller driver configuration
 * @pins:       An array describing all pins the pin controller affects.
 * @npins:      The number of entries in @pins.
 * @functions:  An array describing all mux functions the SoC supports.
 * @nfunctions: The number of entries in @functions.
 * @groups:     An array describing all pin groups the pin SoC supports.
 * @ngroups:    The numbmer of entries in @groups.
 * @ngpio:      The number of pingroups the driver should expose as GPIOs.
 * @gpio_pull_val: The pull value bit field descriptor.
 */
struct msm_pinctrl_soc_data {
	const struct pinctrl_pin_desc *pins;
	unsigned npins;
	const struct msm_function *functions;
	unsigned nfunctions;
	const struct msm_pingroup *groups;
	unsigned ngroups;
	unsigned ngpios;
	const struct msm_pinctrl_gpio_pull *gpio_pull;
};

static const struct msm_pinctrl_gpio_pull msm_gpio_pull = {
	.no_pull = 0,
	.pull_down = 1,
	.keeper = 2,
	.pull_up = 3,
};

#define DRV_TYPE_D		0
#define DRV_TYPE_C		1
#define DRV_TYPE_B		3
#define DRV_TYPE_A		7

enum drv_cap {
	DRV_CAP_HIGH,
	DRV_CAP_HALF,
	DRV_CAP_QUARTER,
};

enum pull_res {
	RES_10_KOHM,
	RES_1_5_KOHM,
	RES_35_KOHM,
	RES_20_KOHM,
};

#define HP_1_8V		1
#define HP_2_8V		0
#define HP_3_3V		0

int msm_pinctrl_probe(struct platform_device *pdev,
		      const struct msm_pinctrl_soc_data *soc_data);
int msm_pinctrl_remove(struct platform_device *pdev);

#endif
