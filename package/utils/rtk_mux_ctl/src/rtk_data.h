#ifndef RTK_DATA_H
#define RTK_DATA_H

#define MUX_VERBOSE		0x0001
#define MUX_SIMULATE		0x0002
#define MUX_DISPLAY_ONLY	0x0004
#define MUX_PC_DEBUG		0x0008
#define MUX_SHOW_SIGNALS	0x0010

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct rtl_gpio {
	uint32_t reg;
	uint8_t shift;
	uint8_t gpio;
	const char *name;
	uint32_t func_count;
	const char **func_array;
};

struct reg_mapping {
	uint32_t offset;
	uint32_t reg_count;
	const char *name;
};

struct device_in_out_func {
	struct rtl_gpio *gpio_mux;
	int gpio_mux_count;
	struct reg_mapping *regs;
	int reg_mappings_count;
	const uint32_t **reg_defaults;
	struct rtl_gpio *(*get_gpio_by_name)(const char *);
	int (*get_gpio_dir)(uint32_t *, int);
	int (*set_gpio_dir)(uint32_t *, int, int);
	int (*get_gpio_val)(uint32_t *, int);
	int (*set_gpio_val)(uint32_t *, int, int);
};

struct rtl_gpio *rtl_gpio_by_name(const char *name, struct rtl_gpio *gpio, int gpio_count)
{
	int i;

	for (i = 0; i < gpio_count; i++){
		if (!strcmp(gpio[i].name, name))
			return &gpio[i];
		}

	return NULL;
}

#include "rtk8197_data.h"

struct device_in_out_func dev_kinkan = {
	.gpio_mux			= rtk8197_gpio,
	.gpio_mux_count			= ARRAY_SIZE(rtk8197_gpio),
	.regs				= rtk8197_regs_mapping,
	.reg_mappings_count		= ARRAY_SIZE(rtk8197_regs_mapping),
	.reg_defaults			= rtk8197_reg_defaults,
	.get_gpio_by_name		= rtk8197_gpio_by_name,
	.get_gpio_dir			= rtk8197_get_gpio_dir,
	.set_gpio_dir			= rtk8197_set_gpio_dir,
	.get_gpio_val			= rtk8197_get_gpio_val,
	.set_gpio_val			= rtk8197_set_gpio_val,
};

struct device_in_out_func dev_komikan = {
	.gpio_mux			= rtk8197_gpio,
	.gpio_mux_count			= ARRAY_SIZE(rtk8197_gpio),
	.regs				= rtk8197_regs_mapping,
	.reg_mappings_count		= ARRAY_SIZE(rtk8197_regs_mapping),
	.reg_defaults			= rtk8197_reg_defaults,
	.get_gpio_by_name		= rtk8197_gpio_by_name,
	.get_gpio_dir			= rtk8197_get_gpio_dir,
	.set_gpio_dir			= rtk8197_set_gpio_dir,
	.get_gpio_val			= rtk8197_get_gpio_val,
	.set_gpio_val			= rtk8197_set_gpio_val,
};
#endif //RTK_DATA_H
