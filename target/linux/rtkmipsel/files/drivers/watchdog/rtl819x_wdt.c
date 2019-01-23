#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/watchdog.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_platform.h>

#define DRIVER_NAME	 "rtl819x-wdt"

#define WDOG_REG_CLOCK		0x00
#define WDOG_REG_CTRL		0x04

#define WDT_DISABLE	0xA5000000
#define WDT_CLEAR		BIT(23)

#define BSP_DIVF_OFFSET     16
// decide  timer to  TIMER_BASE_SEC*2^n second base , default 4 sec means timer is 4,8,16....etc
// 0 is forbidden!!
unsigned int TIMER_BASE_SEC=4 ;
#define TIMER_MAX_N  9
#define TIMER_DEFAULT_N  2

#define OVSEL_L_OFFSET 21
#define OVSEL_H_OFFSET 17
#define OVSEL_MASK 0x03  // only 2bit

//#define WDT_STOP_NOKICK

#if defined(WDT_STOP_NOKICK)
static unsigned char wdt_stopped=0;
#endif
static void __iomem *rtl819x_wdt_base;

static bool nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, bool, 0);
MODULE_PARM_DESC(nowayout,
		"Watchdog cannot be stopped once started (default="
		__MODULE_STRING(WATCHDOG_NOWAYOUT) ")");


static inline void rtk_wdt_w32(unsigned reg, u32 val)
{
	iowrite32(val, rtl819x_wdt_base + reg);
}

static inline u32 rtk_wdt_r32(unsigned reg)
{
	return ioread32(rtl819x_wdt_base + reg);
}

static inline int rtk_get_match_timer_idx(unsigned int t)
{
	int i=0;
	unsigned int timer=0;
	for(i=0;i<TIMER_MAX_N;i++)
	{
		timer =  TIMER_BASE_SEC * (1<< i);
		if(timer >= t ) //find a timer that > set_timerout
			return i;
	}
	printk("Timerout is too long can't find a HW timer to match , use HW max_timeout  instead!!\n");
	return TIMER_MAX_N;
}

static inline unsigned int rtk_get_match_timer(unsigned int t)
{
	int rtk_t_idx=rtk_get_match_timer_idx(t);
	unsigned int rtk_t;
	rtk_t =  TIMER_BASE_SEC * (1<< rtk_t_idx);

	return rtk_t;
}
static void rtk_set_timer(unsigned int t)
{
	int rtk_t_idx=rtk_get_match_timer_idx(t);
	unsigned int reg_val = rtk_wdt_r32(WDOG_REG_CTRL);


	//clear related bit (OVSEL High and low)
	reg_val = reg_val & ~((OVSEL_MASK << OVSEL_L_OFFSET) | (OVSEL_MASK <<OVSEL_H_OFFSET));

	//set low 2 bit
	reg_val  = reg_val | ((rtk_t_idx&OVSEL_MASK) << OVSEL_L_OFFSET);

	//set high bit
	reg_val  = reg_val | (((rtk_t_idx&(OVSEL_MASK<<2))>>2) << OVSEL_H_OFFSET);

	rtk_wdt_w32(WDOG_REG_CTRL, reg_val);
}

static int rtl819x_wdt_ping(struct watchdog_device *w)
{
	unsigned int reg_val = rtk_wdt_r32(WDOG_REG_CTRL);
#if defined(WDT_STOP_NOKICK)
	if(wdt_stopped) {
		printk("rtl819x_wdt_ping ignored, wdt stopped\n");
	} else
#endif
	{
		reg_val |= WDT_CLEAR;
		rtk_wdt_w32(WDOG_REG_CTRL, reg_val);
		//printk("rtl819x_wdt_ping\n");
	}

	return 0;
}

static int rtl819x_wdt_set_timeout(struct watchdog_device *w, unsigned int t)
{
	unsigned int rtk_t;
	rtk_t= rtk_get_match_timer(t);
	//printk("rtl819x_wdt_set_timeout t=%d ,rtk_t=%d\n",t,rtk_t);
	w->timeout = rtk_t;
	rtk_set_timer(rtk_t);
	rtl819x_wdt_ping(w);

	return 0;
}

static int rtl819x_wdt_start(struct watchdog_device *w)
{
	u32 t;

	//printk("rtl819x_wdt_start\n");
	rtl819x_wdt_set_timeout(w, w->timeout);

	t = rtk_wdt_r32(WDOG_REG_CTRL);
	t &= ~(WDT_DISABLE); //enable watch dog
	rtk_wdt_w32(WDOG_REG_CTRL, t);

	return 0;
}

static int rtl819x_wdt_stop(struct watchdog_device *w)
{
	u32 t;

	//rtl819x_wdt_ping(w);
#if defined(WDT_STOP_NOKICK)
	if(w->timeout == 32) {
		wdt_stopped=1;
		printk("rtl819x wdt stopped\n");
	}
#endif
	/*Occasionally, write 0xa5000000 to control register then make system reset immediately.
	  So, OR current value as dummy bits. This is not side affect to function of disabling watchdog counting
	  Please reference JIRA:WQCAP-98	*/
	/* Updated. Base on designer's advice, kick watchdog before stop it */
	t=(rtk_wdt_r32(WDOG_REG_CTRL) | WDT_CLEAR);
	rtk_wdt_w32(WDOG_REG_CTRL, t); //kick watchdog
	rtk_wdt_w32(WDOG_REG_CTRL, WDT_DISABLE);
	//printk("rtl819x_wdt_stop\n");
	return 0;
}

static struct watchdog_info rtl819x_wdt_info = {
	.identity = DRIVER_NAME,
	.options = WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING | WDIOF_MAGICCLOSE,
};

static struct watchdog_ops rtl819x_wdt_ops = {
	.owner = THIS_MODULE,
	.start = rtl819x_wdt_start,
	.stop = rtl819x_wdt_stop,
	.ping = rtl819x_wdt_ping,
	.set_timeout = rtl819x_wdt_set_timeout,
};

static struct watchdog_device rtl819x_wdt_dev = {
	.info = &rtl819x_wdt_info,
	.ops = &rtl819x_wdt_ops,
};

static int rtl819x_wdt_clk_init(void)
{
	u32 clk;

	//formula
	// (   N * 2^15 /200*2^20 (lexbus) )
	// write N=(200*2^5)*4=16800 to cloclk control firset
	// write Low OVSEL to 2^15
	// then the default timer will be 4 sec , the timer can be configured to 4x2^n (n=0~9)
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	clk = 200*32*TIMER_BASE_SEC;
#else
	clk = 200*10*TIMER_BASE_SEC;
#endif
	clk = clk << BSP_DIVF_OFFSET;
	rtk_wdt_w32(WDOG_REG_CLOCK, clk);

	//disable watch timer and reset  OVSEL register
	rtk_wdt_w32(WDOG_REG_CTRL, WDT_DISABLE); //disable watchdog
	return 0;
}


static int rtl819x_wdt_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret;

	//printk("rtl819x_wdt_probe!!\n");
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	//rtl819x_wdt_base = devm_request_and_ioremap(&pdev->dev, res);
	rtl819x_wdt_base = devm_ioremap_resource(&pdev->dev, res); //mark_cc
	if (IS_ERR(rtl819x_wdt_base))
		return PTR_ERR(rtl819x_wdt_base);

	//device_reset(&pdev->dev);
	//init rtl819x watchdog clock
       rtl819x_wdt_clk_init();
	rtl819x_wdt_dev.dev = &pdev->dev;
	//rtl819x_wdt_dev.bootstatus = rtl819x_wdt_bootcause();
	rtl819x_wdt_dev.max_timeout = TIMER_BASE_SEC * (1<<TIMER_MAX_N); //TIMER_BASE_SEC * 2^N
	rtl819x_wdt_dev.timeout = TIMER_BASE_SEC * (1<<TIMER_DEFAULT_N);

	watchdog_set_nowayout(&rtl819x_wdt_dev, nowayout);

	ret = watchdog_register_device(&rtl819x_wdt_dev);
	if (!ret)
		dev_info(&pdev->dev, "Initialized\n");

	return 0;
}

static int rtl819x_wdt_remove(struct platform_device *pdev)
{
	watchdog_unregister_device(&rtl819x_wdt_dev);

	return 0;
}

static void rtl819x_wdt_shutdown(struct platform_device *pdev)
{
	rtl819x_wdt_stop(&rtl819x_wdt_dev);
}

static struct platform_driver rtl819x_wdt_driver = {
	.probe		= rtl819x_wdt_probe,
	.remove		= rtl819x_wdt_remove,
	.shutdown	= rtl819x_wdt_shutdown,
	.driver		= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		//.of_match_table = of_match_ptr(rtl819x_wdt_match),
	},
};

module_platform_driver(rtl819x_wdt_driver);

MODULE_DESCRIPTION("Realtek RTL819X hardware watchdog driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" DRIVER_NAME);
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
