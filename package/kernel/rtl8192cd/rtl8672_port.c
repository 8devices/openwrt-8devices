#include <linux/kernel.h>
#include <linux/init.h>

#if defined (CONFIG_PRINTK_FUNC)
int scrlog_printk(const char *fmt, ...) {
	
	va_list args;
	int r;
	va_start(args, fmt);
//	r = vprintk(fmt, args);
	r = scrlog_vprintk(fmt, args);
        va_end(args);
	return r;		
}
#else
int scrlog_printk(const char *fmt, ...)
{
	return 0;
}
#endif

#if defined(CONFIG_PANIC_PRINTK)
int panic_printk(const char *fmt, ...) {
	va_list args;
	int r;
	va_start(args, fmt);
	r = vprintk(fmt, args);
        va_end(args);
	return r;
}
#endif