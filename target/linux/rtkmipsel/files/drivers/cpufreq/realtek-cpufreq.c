/*
 * cpufreq driver for the Realtek processors.
 *
 * Copyright (c) 2018, UAB 8Devices. All rights reserved.
 */

#include <asm/cevt-r4k.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/clocksource.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>


extern struct clocksource clocksource_mips;
struct clocksource * curr_cs = &clocksource_mips;

int reinit_r4k_clocksource(void)
{
	struct clocksource * new_cs;

	new_cs = (struct clocksource *) kzalloc(sizeof(struct clocksource), GFP_KERNEL);
	new_cs->name	= curr_cs->name;
	new_cs->read	= curr_cs->read;
	new_cs->mask	= curr_cs->mask;
	new_cs->flags	= curr_cs->flags;
	new_cs->rating	= curr_cs->rating;

        if (!cpu_has_counter || !mips_hpt_frequency)
                return -ENXIO;

        clocksource_register_hz(new_cs, mips_hpt_frequency);
	clocksource_unregister(curr_cs);
	curr_cs = new_cs;

        return 0;
}

static int realtek_cpufreq_target(struct cpufreq_policy *policy,
			     unsigned long target_freq,
			     unsigned int relation)
{
	struct cpufreq_freqs freqs;

	freqs.old	= cpufreq_generic_get(0);
	freqs.new	= target_freq;
	freqs.flags	= 0;

	cpufreq_freq_transition_begin(policy, &freqs);
	clk_set_rate(policy->clk, ((unsigned long)target_freq * 1000ul));
	reinit_r4k_clocksource();
	cpufreq_freq_transition_end(policy, &freqs, 0);
	raw_current_cpu_data.udelay_val = loops_per_jiffy;

	return 0;
}

static int realtek_cpufreq_verify(struct cpufreq_policy *policy)
{
	return 0;
}

static int realtek_cpufreq_cpu_init(struct cpufreq_policy *policy)
{
	struct clk *cpuclk;//= &per_cpu(realtek_cpuclk, cpu);
	struct cpufreq_frequency_table *freq_table;
	struct device *dev;
	struct clk_lookup *cl;

	cpuclk = clk_get_sys("cpu_clk", NULL);

	if (IS_ERR(cpuclk)) {
		dev_err(dev, "couldn't get CPU clk\n");
		return PTR_ERR(cpuclk);
	}
	policy->clk = cpuclk;
	policy->min =  200000;
	policy->max = 1000000;

	policy->cpuinfo.transition_latency = CPUFREQ_ETERNAL;
	dev_info(dev, "CPU Frequencies - Minimum %u.%03u MHz, "
	       "Maximum %u.%03u MHz.\n",
	       policy->min / 1000, policy->min % 1000,
	       policy->max / 1000, policy->max % 1000);

	return 0;
}

static int realtek_cpufreq_cpu_exit(struct cpufreq_policy *policy)
{
	return 0;
}

static struct cpufreq_driver realtek_cpufreq_driver = {
	.name		= "reatek",
	.get		= cpufreq_generic_get,
	.init		= realtek_cpufreq_cpu_init,
	.attr		= cpufreq_generic_attr,
	.target		= realtek_cpufreq_target,
	.verify		= realtek_cpufreq_verify,
	.exit		= realtek_cpufreq_cpu_exit,
};


static int __init realtek_cpufreq_module_init(void)
{
	pr_notice("Realtek CPU frequency driver.\n");
	return cpufreq_register_driver(&realtek_cpufreq_driver);
}

static void __exit realtek_cpufreq_module_exit(void)
{
	cpufreq_unregister_driver(&realtek_cpufreq_driver);
}

module_init(realtek_cpufreq_module_init);
module_exit(realtek_cpufreq_module_exit);

MODULE_AUTHOR("Rytis Zigmantavicius <rytis.z@8devices.com>");
MODULE_DESCRIPTION("cpufreq driver for Realtek");
MODULE_LICENSE("GPL");
