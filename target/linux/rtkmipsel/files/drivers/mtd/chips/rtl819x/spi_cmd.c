/**
 *  SPI Flash common control code.
 *  (C) 2006 Atmark Techno, Inc.
 */

#include <linux/init.h>
#include <linux/kernel.h>
//#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>

#include "spi_flash.h"

//#define MTD_SPI_DEBUG

#if defined(MTD_SPI_DEBUG)
#define KDEBUG(args...) printk(args)
#else
#define KDEBUG(args...)
#endif

#ifdef RTK_FLASH_SPIN_LOCK
spinlock_t lock_spi;
#else
spinlock_t lock_spi;

#endif

#if 1
#else
extern int mtd_spi_erase(struct mtd_info *mtd, struct erase_info *instr);
extern int mtd_spi_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf);
extern int mtd_spi_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf);
extern void mtd_spi_sync(struct mtd_info *mtd);
extern int mtd_spi_lock(struct mtd_info *mtd, loff_t ofs, size_t len);
extern int mtd_spi_unlock(struct mtd_info *mtd, loff_t ofs, size_t len);
extern int mtd_spi_suspend(struct mtd_info *mtd);
extern void mtd_spi_resume(struct mtd_info *mtd);
#endif

#ifdef CONFIG_RTL_8198C
#define LX_CONTROL		0xb8000014
#define SYS_MSRR		0xb8001038

#define REG32(reg)		(*(volatile unsigned int *)(reg))

#if 1
#define LOCK_LX0_BUS() \
do { \
	REG32(LX_CONTROL) |= (1<<2); /* request locking Lx0 bus */ \
	while( (REG32(LX_CONTROL)&(1<<12)) == 0 ) ; /* wait for Lx0 bus lock okay */ \
	while( (REG32(SYS_MSRR)&(1<<30)) == 0 ) ; /* wait for No on going DRAM command */ \
} while(0)

#define RELEASE_LX0_BUS() \
do { \
	REG32(LX_CONTROL) &= ~(1<<2); /* release Lx0 bus */ \
} while(0)
#endif
#endif
#if 0
#define LOCK_LX0_BUS() \
do { \
	REG32(LX_CONTROL) |= (3<<2); /* request locking Lx0 bus */ \
	while( (REG32(LX_CONTROL)&(3<<12)) != (3<<12) ) ; /* wait for Lx0 bus lock okay */ \
	while( (REG32(SYS_MSRR)&(1<<30)) == 0 ) ; /* wait for No on going DRAM command */ \
} while(0)

#define RELEASE_LX0_BUS() \
do { \
	REG32(LX_CONTROL) &= ~(3<<2); /* release Lx0 bus */ \
} while(0)

//#else
//#define LOCK_LX0_BUS()
//#define RELEASE_LX0_BUS()
#endif



int mtd_spi_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct map_info *map = mtd->priv;
	struct spi_chip_info *chip_info = (struct spi_chip_info *)map->fldrv_priv;
	unsigned long adr, len;
	int ret = 0,count=0;
#if defined(RTK_FLASH_SPIN_LOCK)
	unsigned int flags;
#endif

	if (!chip_info->erase)
		return -EOPNOTSUPP;
#if 0
	// skip 1st block erase
	if (instr->addr < (mtd->erasesize ))
	{
		instr->state = MTD_ERASE_DONE;
		return 0;
	}
#endif
	if (instr->addr & (mtd->erasesize - 1))
		return -EINVAL;
/*
	if (instr->len & (mtd->erasesize -1))
		return -EINVAL;
*/

	if ((instr->len + instr->addr) > mtd->size)
		return -EINVAL;

#ifdef  RTK_FLASH_MUTEX
retry0:
	ret=mutex_trylock(&chip_info->lock);

	//mutex_lock(&chip_info->lock);

	if (!ret)
	{
		count++;
	//	printk("RR\r\n");
		if(count<100000)
				goto retry0;
		else
			mutex_lock(&chip_info->lock);
			//return 0;
	}
#endif
#ifdef RTK_FLASH_SPIN_LOCK

	spin_lock_irqsave(&lock_spi, flags);
#endif
#ifdef CONFIG_RTL_8198C
	LOCK_LX0_BUS();
#endif
//#endif
	adr = instr->addr;
	len = instr->len;

	KDEBUG("mtd_spi_erase():: adr: 0x%08lx, len: 0x%08lx\n", adr, len);


	if (len & (mtd->erasesize-1))
	{
		len = len - (len & (mtd->erasesize-1)) + mtd->erasesize;
	}

	if (len < mtd->erasesize)
		len = mtd->erasesize;



	while (len) {
		ret = chip_info->erase(adr,chip_info->chip_select);
		if (ret)
		{

//#ifdef RTK_FLASH_SPIN_LOCK
			//spin_unlock(&lock_spi);
#ifdef CONFIG_RTL_8198C
			RELEASE_LX0_BUS();
#endif
#ifdef RTK_FLASH_SPIN_LOCK
			spin_unlock_irqrestore(&lock_spi, flags);

#endif
#ifdef  RTK_FLASH_MUTEX
			mutex_unlock(&chip_info->lock);
			cfi_udelay(1);
#endif
			return ret;
		}
		adr += mtd->erasesize;
		len -= mtd->erasesize;
	}



	instr->state = MTD_ERASE_DONE;
	if (instr->callback)
		instr->callback(instr);
//	#ifdef RTK_FLASH_SPIN_LOCK
#ifdef CONFIG_RTL_8198C
		RELEASE_LX0_BUS();
#endif
	#ifdef RTK_FLASH_SPIN_LOCK
	spin_unlock_irqrestore(&lock_spi, flags);
	#endif

	#ifdef  RTK_FLASH_MUTEX
	mutex_unlock(&chip_info->lock);
	 cfi_udelay(1);
	#endif

	return 0;
}

int mtd_spi_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	struct map_info *map = mtd->priv;
	struct spi_chip_info *chip_info = (struct spi_chip_info *)map->fldrv_priv;
	int ret = 0,count=0;
#if defined(RTK_FLASH_SPIN_LOCK)
	unsigned int flags;
#endif

	if (!chip_info->read)
		return -EOPNOTSUPP;

	KDEBUG("mtd_spi_read():: adr: 0x%08x, len: %08x, cs=%d\n", (u32)from, len, chip_info->chip_select);

#ifdef  RTK_FLASH_MUTEX
retry1:
	ret=mutex_trylock(&chip_info->lock);

	//mutex_lock(&chip_info->lock);

	if (!ret)
	{
		count++;
	//	printk("RR\r\n");
		if(count<100000)
				goto retry1;
		else
			mutex_lock(&chip_info->lock);
			//return 0;
	}
#endif
#ifdef RTK_FLASH_SPIN_LOCK

	spin_lock_irqsave(&lock_spi, flags);
#endif
#ifdef CONFIG_RTL_8198C
	LOCK_LX0_BUS();
#endif
	//spin_lock(&lock_spi);
//#endif
	ret = chip_info->read(from, (u32)buf, len, chip_info->chip_select);


	if (ret)
	{
//	#ifdef RTK_FLASH_SPIN_LOCK
#ifdef CONFIG_RTL_8198C
	RELEASE_LX0_BUS();
#endif
	#ifdef RTK_FLASH_SPIN_LOCK
spin_unlock_irqrestore(&lock_spi, flags);

#endif
#ifdef  RTK_FLASH_MUTEX
	mutex_unlock(&chip_info->lock);
	 cfi_udelay(1);
#endif
		return ret;
	}
	if(retlen)
		(*retlen) = len;
//#ifdef RTK_FLASH_SPIN_LOCK
#ifdef CONFIG_RTL_8198C
	RELEASE_LX0_BUS();
#endif
#ifdef RTK_FLASH_SPIN_LOCK
spin_unlock_irqrestore(&lock_spi, flags);

#endif

#ifdef  RTK_FLASH_MUTEX
	mutex_unlock(&chip_info->lock);
	 cfi_udelay(1);
#endif

	return 0;
}

int mtd_spi_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	struct map_info *map = mtd->priv;
	struct spi_chip_info *chip_info = (struct spi_chip_info *)map->fldrv_priv;
	int ret = 0,count=0;
#if defined(RTK_FLASH_SPIN_LOCK)
	unsigned int flags;
#endif

	if (!chip_info->write)
		return -EOPNOTSUPP;

	KDEBUG(" mtd_spi_write():: adr: 0x%08x, len: 0x%08x, cs=%d\n", (u32)to, len, chip_info->chip_select);
#ifdef  RTK_FLASH_MUTEX
retry2:
	ret=mutex_trylock(&chip_info->lock);

	//mutex_lock(&chip_info->lock);

	if (!ret)
	{
		count++;
	//	printk("RR\r\n");
		if(count<100000)
				goto retry2;
		else
			mutex_lock(&chip_info->lock);
			//return 0;
	}
#endif
#ifdef RTK_FLASH_SPIN_LOCK

	spin_lock_irqsave(&lock_spi, flags);
#endif
#ifdef CONFIG_RTL_8198C
		LOCK_LX0_BUS();
#endif
//spin_lock(&lock_spi);

//#endif
	ret = chip_info->write((u32)buf, to, len, chip_info->chip_select);



	if (ret)
	{
//	#ifdef RTK_FLASH_SPIN_LOCK
#ifdef CONFIG_RTL_8198C
	RELEASE_LX0_BUS();
#endif
	#ifdef RTK_FLASH_SPIN_LOCK
	spin_unlock_irqrestore(&lock_spi, flags);

#endif
#ifdef  RTK_FLASH_MUTEX
	mutex_unlock(&chip_info->lock);
	 cfi_udelay(1);
#endif
		return ret;
	}
	if (retlen)
		(*retlen) = len;
//		#ifdef RTK_FLASH_SPIN_LOCK
#ifdef CONFIG_RTL_8198C
	RELEASE_LX0_BUS();
#endif
		#ifdef RTK_FLASH_SPIN_LOCK
	spin_unlock_irqrestore(&lock_spi, flags);

#endif
	#ifdef  RTK_FLASH_MUTEX
	mutex_unlock(&chip_info->lock);
	 cfi_udelay(1);
#endif

	return 0;
}

void mtd_spi_sync(struct mtd_info *mtd)
{
	/* Operation not supported on transport endpoint */
}

int mtd_spi_lock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	/* Operation not supported on transport endpoint */

	struct map_info *map = mtd->priv;
	struct spi_chip_info *chip_info = (struct spi_chip_info *)map->fldrv_priv;
	int ret = 0,count=0;

	return -EOPNOTSUPP;
#ifdef  RTK_FLASH_MUTEX
retry4:
	ret=mutex_trylock(&chip_info->lock);

	//mutex_lock(&chip_info->lock);

	if (!ret)
	{
		count++;
	//	printk("RR\r\n");
		if(count<100000)
				goto retry4;
		else
			mutex_lock(&chip_info->lock);
			//return 0;
	}
#endif

//	return -EOPNOTSUPP;
	return 0;
}

int mtd_spi_unlock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	/* Operation not supported on transport endpoint */
struct map_info *map = mtd->priv;
        struct spi_chip_info *chip_info = (struct spi_chip_info *)map->fldrv_priv;
        int ret = 0;

        return -EOPNOTSUPP;
#ifdef  RTK_FLASH_MUTEX
        mutex_unlock(&chip_info->lock);
	 cfi_udelay(1);
#endif
	return 0;//-EOPNOTSUPP;
}

int mtd_spi_suspend(struct mtd_info *mtd)
{
	/* Operation not supported on transport endpoint */
	return -EOPNOTSUPP;
}

void mtd_spi_resume(struct mtd_info *mtd)
{
	/* Operation not supported on transport endpoint */
}

EXPORT_SYMBOL(mtd_spi_erase);
EXPORT_SYMBOL(mtd_spi_read);
EXPORT_SYMBOL(mtd_spi_write);
EXPORT_SYMBOL(mtd_spi_sync);
EXPORT_SYMBOL(mtd_spi_lock);
EXPORT_SYMBOL(mtd_spi_unlock);
EXPORT_SYMBOL(mtd_spi_suspend);
EXPORT_SYMBOL(mtd_spi_resume);
