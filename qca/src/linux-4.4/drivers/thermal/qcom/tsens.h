/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __QCOM_TSENS_H__
#define __QCOM_TSENS_H__

#define ONE_PT_CALIB		0x1
#define ONE_PT_CALIB2		0x2
#define TWO_PT_CALIB		0x3

#include <linux/thermal.h>

struct tsens_device;

struct tsens_sensor {
	struct tsens_device		*tmdev;
	struct thermal_zone_device	*tzd;
	struct work_struct		notify_work;
	int				offset;
	int				id;
	int				hw_id;
	int				calib_data;
	int				calib_data_backup;
	int				slope;
	u32				status;
};

/**
 * struct tsens_ops - operations as supported by the tsens device
 * @init: Function to initialize the tsens device
 * @calibrate: Function to calibrate the tsens device
 * @get_temp: Function which returns the temp in millidegC
 * @enable: Function to enable (clocks/power) tsens device
 * @disable: Function to disable the tsens device
 * @suspend: Function to suspend the tsens device
 * @resume: Function to resume the tsens device
 * @get_trend: Function to get the thermal/temp trend
 * @set_trip_temp: Function to set trip temp
 * @get_trip_temp: Function to get trip temp
 * @set_trip_activate: Function to activate trip points
 */
struct tsens_ops {
	/* mandatory callbacks */
	int (*init)(struct tsens_device *);
	int (*calibrate)(struct tsens_device *);
	int (*get_temp)(struct tsens_device *, int, int *);
	/* optional callbacks */
	void (*panic_notify)(struct tsens_device *, int);
	int (*enable)(struct tsens_device *, int);
	void (*disable)(struct tsens_device *);
	int (*suspend)(struct tsens_device *);
	int (*resume)(struct tsens_device *);
	int (*get_trend)(struct tsens_device *, int, enum thermal_trend *);
	int (*set_trip_temp)(void *, int, int);
	int (*set_trip_activate)(void *, int,
					enum thermal_trip_activation_mode);
};

/**
 * struct tsens_data - tsens instance specific data
 * @num_sensors: Max number of sensors supported by platform
 * @ops: operations the tsens instance supports
 * @hw_ids: Subset of sensors ids supported by platform, if not the first n
 */
struct tsens_data {
	const u32		num_sensors;
	const struct tsens_ops	*ops;
	unsigned int		*hw_ids;
};

/* Registers to be saved/restored across a context loss */
struct tsens_context {
	int	threshold;
	int	control;
};

struct tsens_device {
	struct device			*dev;
	u32				num_sensors;
	u32				tsens_irq;
	struct regmap			*map;
	struct regmap_field		*status_field;
	struct tsens_context		ctx;
	bool				trdy;
	const struct tsens_ops		*ops;
	struct work_struct		tsens_work;
	void __iomem			*iomem_base;
	struct tsens_sensor		sensor[0];
};

char *qfprom_read(struct device *, const char *);
void compute_intercept_slope(struct tsens_device *, u32 *, u32 *, u32);
int code_to_degc(u32 adc_code, const struct tsens_sensor *s);
void __iomem *init_common(struct tsens_device *);
int get_temp_common(struct tsens_device *, int, int *);

extern const struct tsens_data data_8960, data_ipq8064, data_ipq807x;

extern const struct tsens_data data_ipq5018;

#endif /* __QCOM_TSENS_H__ */
