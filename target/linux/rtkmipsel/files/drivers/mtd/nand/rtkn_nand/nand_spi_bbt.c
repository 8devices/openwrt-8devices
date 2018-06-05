#include "rtknflash.h"
#include <linux/kernel.h>
#ifndef __UBOOT__
#include <linux/slab.h>
#endif

/* need to do */
int rtkn_scan_bbt(struct mtd_info *mtd)
{

}

int rtkn_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip)
{

}


int rtkn_block_markbad(struct mtd_info *mtd, loff_t ofs)
{

}
