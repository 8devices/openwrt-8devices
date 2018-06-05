#ifndef _MTD_SPI_PROBE_H_
#define _MTD_SPI_PROBE_H_

#ifdef CONFIG_SMP
#define RTK_FLASH_MUTEX 1
#endif

#ifdef  RTK_FLASH_MUTEX
#include <linux/mutex.h>
#endif
struct spi_chip_mtd
{
	unsigned int 		chip_id;
	unsigned int		extra_id;
	unsigned int		sectorSize;
	unsigned int 		deviceSize;
	unsigned int 		uiClkMhz;
	char*				name;
};

struct spi_chip_info
{
	char* name;
	unsigned int chip_select;
	struct spi_chip_mtd *flash;
#ifdef  RTK_FLASH_MUTEX
	struct mutex		lock;
#endif
	void (*destroy)(struct spi_chip_info *chip_info);

	unsigned int (*read)(unsigned int  from, unsigned int  to, unsigned int  size, unsigned int uiChip);
	unsigned int (*write)(unsigned int  from, unsigned int  to, unsigned int  size, unsigned int uiChip);
	int (*erase)(unsigned int  addr, unsigned int uiChip);
};

#endif /* _MTD_SPI_PROBE_H_ */
