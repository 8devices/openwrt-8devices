#ifndef __RTKNFLASH_WRAPPER_H__
#define __RTKNFLASH_WRAPPER_H__

#ifdef __UBOOT__
#define dma_cache_inv(x,y)			invalidate_dcache_range(x,(x+y))
#define dma_cache_wback(x,y)		flush_dcache_range(x,(x+y))
#define dma_cache_wback_inv(x,y)	flush_dcache_range(x,(x+y))

#define spin_lock_irqsave(x,y)
#define spin_unlock_irqrestore(x,y)
#define printk(fmt,args...)	printf(fmt ,##args)
#endif

#ifdef __RTK_BOOT__
#define printf(fmt,args...)	prom_printf(fmt ,##args)
#define printk(fmt,args...)	prom_printf(fmt ,##args)
#define pr_err(fmt,args...)	prom_printf(fmt ,##args)

#define kmalloc(x,y)		malloc(x)
#define kfree				free

#define local_irq_save(x)
#define local_irq_restore(x)

#define dma_cache_inv(x,y)			invalidate_dcache_range(x,(x+y))
#define dma_cache_wback(x,y)		flush_dcache_range(x,(x+y))
#define dma_cache_wback_inv(x,y)	flush_dcache_range(x,(x+y))


#define KERNEL_VERSION(a,b,c) 		(((a) << 16) + ((b) << 8) + (c))
#define LINUX_VERSION_CODE			KERNEL_VERSION(3,10,0)

#define smp_wmb()
#define BUG()
#endif

#ifdef __UBOOT__
void  board_nand_init(void);
#else
int  board_nand_init(void);
#endif

#if !(defined(__UBOOT__) || defined(__RTK_BOOT__))
#include <linux/kernel.h>
#define printf(fmt,args...)	printk(fmt ,##args)
#endif


/* test */
//#define printf_test	prom_printf
#define printf_test

#define printf_test2	//prom_printf

#endif
