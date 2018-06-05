/*
 *	Maintained by Chang Tien-Fu <steve1097@realtek.com>
 *	Copyright (C) Chang Tien-Fu
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/pwm.h>

//#include <asm/div64.h>
//#include <mach/hardware.h>
#define REG32(reg)      (*(volatile unsigned int*)(reg))
#define RTK_Gtimer_rate 		32000
#define RTK_PWM_REG 		0xb8000200
#define RTK_PWMDCR			12	/* Duty Cycle Register */
#define PWMDCR_MIN_DUTY		0x0
#define PWMDCR_MAX_DUTY		0x3FF

#define RTK_PWMPCR			0	/* Period Register */
#define PWMPCR_MIN_PERIOD		0x0
#define PWMPCR_MAX_PERIOD		0x3FF

#define TIMER_REG_BASE 		0xb8148000
#define TIMER_INTERVAL		0x14

#define REG_TIMER_LOAD_COUNT		0x00
#define REG_TIMER_CURRENT_VAL		0x04
#define REG_TIMER_CTL_REG 			0x08
#define REG_TIMER_EOI		0x0c
#define REG_TIMER_INT_STATUS        0x10
#define TIMER_32KHz_LOAD_COUNT		0x00
#define TIMER_16KHz_LOAD_COUNT		0x01


#define HAL_TIMER_WRITE32(addr, value)    ((*((volatile u32*)(TIMER_REG_BASE + addr))) = value)//HAL_WRITE32(TIMER_REG_BASE, addr, value)


struct rtk_pwm_chip {
	struct pwm_chip chip;
	void __iomem *base;
	struct clk *clk;
};

static inline struct rtk_pwm_chip *to_rtk(struct pwm_chip *chip)
{
	return container_of(chip, struct rtk_pwm_chip, chip);
}

static inline void rtk_pwm_set(unsigned long val, struct rtk_pwm_chip *chip,  unsigned int pwm_num)
{
	unsigned long value, base;
	value = val + (pwm_num << 24);
	base = chip->base + (pwm_num << 2);
	//printk("rtk_pwm_writel %lx to reg %lx\n", value, base);
	REG32(base)  =   (REG32(base)  & (~(0x7FFFFFF))) |  value;
	//writel(value, base);
	return;
}

/*
 * period_ns = 10^9 * (PRESCALE + 1) * (PV + 1) / PWM_CLK_RATE
 * duty_ns   = 10^9 * (PRESCALE + 1) * DC / PWM_CLK_RATE
 */
static int rtk_pwm_config(struct pwm_chip *chip, struct pwm_device *pwm,
			   int duty_ns, int period_ns)
{
	//unsigned long period_cycles, prescale, pv, dc;
	unsigned long duty_cycles, period_cycles;
	struct rtk_pwm_chip *rtk = to_rtk(chip);
	unsigned long long d, c;
	unsigned long pwm_value = 0;
	//c = clk_get_rate(rtk->clk);
	//c = RTK_Gtimer_rate;
	//c = c * period_ns;
	//do_div(c, 1000000000);
	//printk("Set duty_ns %d, period_ns %d\n", duty_ns, period_ns);
	if(duty_ns>period_ns)
	{
		duty_ns=period_ns;
		printk("User set duty > period. Set duty = period, we suggest to set again\n");
	}


	d = duty_ns;
	do_div(d, 60000);
	duty_cycles = d;
	if (duty_cycles < PWMDCR_MIN_DUTY)
	{
		duty_cycles = PWMDCR_MIN_DUTY;
		printk("RTK gtimer/PWM time resolution is 60000ns/60us. Set duty to 60us, we suggest to set again\n");
	}
	if (duty_cycles > PWMDCR_MAX_DUTY)
	{
		duty_cycles = PWMDCR_MAX_DUTY;
		printk("RTK MAX gtimer/PWM time setting is 120ms. Set duty to 120ms, we suggest to set again\n");
	}

	c = period_ns;
	do_div(c, 60000);
	period_cycles = c;
	if (period_cycles < PWMPCR_MIN_PERIOD)
	{
		period_cycles = PWMPCR_MIN_PERIOD;
		printk("RTK gtimer/PWM time resolution is 60000ns/60us. Set period to 60us, we suggest to set again\n");
	}
	if (period_cycles > PWMPCR_MAX_PERIOD)
	{
		period_cycles = PWMPCR_MAX_PERIOD;
		printk("RTK MAX gtimer/PWM time setting is 120ms. Set period to 120ms, we suggest to set again\n");
	}

/*
	//prescale = (period_cycles - 1) / 1024;
	//pv = period_cycles / (prescale + 1) - 1;

	//if (prescale > 63)
	//	return -EINVAL;

	if (duty_ns == period_ns)
		dc = OST_PWMDCCR_FDCYCLE;
	else
		dc = (pv + 1) * duty_ns / period_ns;
*/
	/*
	 * NOTE: the clock to PWM has to be enabled first
	 * before writing to the registers
	 */
	//clk_prepare_enable(rtk->clk);

	pwm_value = (duty_cycles<<RTK_PWMDCR)+(period_cycles<<RTK_PWMPCR);
	//printk("Set duty_cycles %lx, period_cycles %lx, pwm_value %lx\n", duty_cycles, period_cycles, pwm_value);
	rtk_pwm_set(pwm_value, rtk, pwm->hwpwm);

	return 0;
}

static int rtk_pwm_enable(struct pwm_chip *chip, struct pwm_device *pwm)
{
	printk("rtk_pwm_enable, pwm->hwpwm = %d\n",pwm->hwpwm);

	struct rtk_pwm_chip *rtk = to_rtk(chip);
	unsigned long base;
	base = rtk->base + (pwm->hwpwm << 2);
	//printk("rtk_pwm_writel %lx to reg %lx\n", value, base);
	REG32(base)  =   (REG32(base)  | ((0x1 << 31)));


	//Config Timer Setting
	/*
        set TimerControlReg
        0: Timer enable (0,disable; 1,enable)
        1: Timer Mode (0, free-running mode; 1, user-defined count mode)
        2: Timer Interrupt Mask (0, not masked; 1,masked)
        */
	HAL_TIMER_WRITE32((TIMER_INTERVAL*pwm->hwpwm + REG_TIMER_CTL_REG),
                        0x003);

	// set TimerLoadCount Register
	// PWM highest freq = 1/2 timer freq, controled by timer load count
	HAL_TIMER_WRITE32((TIMER_INTERVAL*pwm->hwpwm + REG_TIMER_LOAD_COUNT),
                        TIMER_16KHz_LOAD_COUNT);


	//clk_prepare_enable(rtk->clk);
	return 1;
}

static void rtk_pwm_disable(struct pwm_chip *chip, struct pwm_device *pwm)
{
	printk("rtk_pwm_disable\n");
	struct rtk_pwm_chip *rtk = to_rtk(chip);
	unsigned long base;
	base = rtk->base + (pwm->hwpwm << 2);
	//printk("rtk_pwm_writel %lx to reg %lx\n", value, base);

	REG32(base)  =   (REG32(base)  & (~(0x1 << 31)));
	//printk("rtk_pwm_disable set reg %lx to %lx\n", base, REG32(base));

	//disable dw timer
	/*HAL_TIMER_WRITE32((TIMER_INTERVAL*pwm->pwm + REG_TIMER_CTL_REG),
                        0x000);
	HAL_TIMER_WRITE32((TIMER_INTERVAL*pwm->pwm + REG_TIMER_LOAD_COUNT),
                        0xFFFFFFFF);
	*/

	//clk_disable_unprepare(rtk->clk);
}

static const struct pwm_ops rtk_pwm_ops = {
	.config = rtk_pwm_config,
	.enable = rtk_pwm_enable,
	.disable = rtk_pwm_disable,
	.owner = THIS_MODULE,
};

static int pwm_probe(struct platform_device *pdev)
{
	struct rtk_pwm_chip *rtk;
	struct resource *r;
	printk("pwm_probe\n");
	int ret;
	rtk = devm_kzalloc(&pdev->dev, sizeof(*rtk), GFP_KERNEL);
	if (rtk == NULL) {
		dev_err(&pdev->dev, "failed to allocate memory\n");
		return -ENOMEM;
	}
	/*
	rtk->clk = devm_clk_get(&pdev->dev, "OST_CLK");
	if (IS_ERR(rtk->clk))
		return PTR_ERR(rtk->clk);

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	rtk->base = devm_ioremap_resource(&pdev->dev, r);

	if (IS_ERR(rtk->base))
		return PTR_ERR(rtk->base);
*/
	rtk->base = 0xb8000200;
	rtk->chip.dev = &pdev->dev;
	rtk->chip.ops = &rtk_pwm_ops;
	//rtk->chip.base = -1;
	rtk->chip.base = -1;
	rtk->chip.npwm = 4;

	ret = pwmchip_add(&rtk->chip);
	if (ret < 0) {
		dev_err(&pdev->dev, "pwmchip_add() failed: %d\n", ret);
		return ret;
	}

	platform_set_drvdata(pdev, rtk);

	return 0;
}

static int pwm_remove(struct platform_device *pdev)
{
	struct rtk_pwm_chip *rtk = platform_get_drvdata(pdev);

	return pwmchip_remove(&rtk->chip);
}

static struct platform_driver rtk_pwm_driver = {
	.driver = {
		.name = "rtk-pwm",
		.owner = THIS_MODULE,
	},
	.probe = pwm_probe,
	.remove = pwm_remove,
};
module_platform_driver(rtk_pwm_driver);

MODULE_LICENSE("GPL v2");
