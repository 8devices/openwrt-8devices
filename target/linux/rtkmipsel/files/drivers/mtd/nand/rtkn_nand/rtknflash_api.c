#include "rtknflash.h"
#include <linux/kernel.h>
#include <linux/version.h>
#ifndef __UBOOT__
#include <linux/slab.h>
#endif

/* api for other module use */
#ifndef __UBOOT__
extern struct mtd_info rtkn_mtd_info;
struct mtd_info* rtk_get_nand_mtd(void)
{
	return &rtkn_mtd_info;
}
EXPORT_SYMBOL(rtk_get_nand_mtd);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
/* erase based on block */
int rtk_nand_erase(struct mtd_info *mtd, loff_t  offs,size_t len)
{
	int res,i;
	size_t block_start,block_end;
	struct erase_info einfo;
	struct nand_chip* this = (struct nand_chip*)mtd->priv;
	//struct rtknflash* rtkn = (struct rtknflash*)this->priv;

	if(len == 0){
		/* do nothing */
		return 0;
	}

	/*  according to */
	block_start = (size_t)(offs>>this->bbt_erase_shift);
	block_end = (size_t)(((offs+(loff_t)len-1)>>this->bbt_erase_shift)  + 1);

	for(i = block_start;i < block_end;i++){
		memset(&einfo, 0, sizeof(einfo));
		einfo.mtd = mtd;
		einfo.addr = ((loff_t)i)<<this->bbt_erase_shift;
		einfo.len = 1 << this->bbt_erase_shift;
		res = nand_erase_nand(mtd, &einfo, 1);

		if(res != 0)
			return res;
	}

	return 0;
}
EXPORT_SYMBOL(rtk_nand_erase);

int rtk_nand_read(struct mtd_info *mtd, uint8_t *buf, loff_t  offs,
			 size_t len)
{
	size_t retlen;

	return mtd_read(mtd, offs, len, &retlen, buf);
}
EXPORT_SYMBOL(rtk_nand_read);


int rtk_nand_write(struct mtd_info *mtd, loff_t  offs, size_t len,
			  uint8_t *buf)
{
	size_t retlen;

	return mtd_write(mtd,offs,len,&retlen,buf);

}
EXPORT_SYMBOL(rtk_nand_write);

#if 0
int rtk_nand_read_oob(struct mtd_info *mtd, uint8_t *buf, loff_t  offs,
			 size_t len)
{
	struct mtd_oob_ops ops;
	int res, ret = 0;

	ops.mode = MTD_OPS_PLACE_OOB;
	ops.ooboffs = 0;
	ops.ooblen = mtd->oobsize;

	while (len > 0) {
		ops.datbuf = buf;
		ops.len = min(len, (size_t)mtd->writesize);
		ops.oobbuf = buf + ops.len;

		res = mtd_read_oob(mtd, offs, &ops);
		if (res) {
			if (!mtd_is_bitflip_or_eccerr(res))
				return res;
			else if (mtd_is_eccerr(res) || !ret)
				ret = res;
		}

		buf += mtd->oobsize + mtd->writesize;
		len -= mtd->writesize;
		offs += mtd->writesize;
	}
	return ret;
}
EXPORT_SYMBOL(rtk_nand_read_oob);


int rtk_nand_write_oob(struct mtd_info *mtd, loff_t  offs, size_t len,
			  uint8_t *buf,uint8_t *oob)
{
	int res;
	struct mtd_oob_ops ops;

	ops.mode = MTD_OPS_PLACE_OOB;
	ops.ooboffs = 0;
	ops.ooblen = mtd->oobsize;
	ops.datbuf = buf;
	ops.oobbuf = oob;
	ops.len = len;

	return mtd_write_oob(mtd, offs, &ops);
}
EXPORT_SYMBOL(rtk_nand_write_oob);
#endif

#else
int rtk_nand_erase(struct mtd_info *mtd, loff_t  offs,size_t len)
{
	int res,i;
	size_t block_start,block_end;
	struct nand_chip* this = (struct nand_chip*)mtd->priv;
	struct rtknflash* rtkn = (struct rtknflash*)this->priv;

	if(len == 0){
		/* do nothing */
		return 0;
	}

	/*  according to */
	struct erase_info einfo;
	block_start = (size_t)(offs>>this->bbt_erase_shift);
	block_end = (size_t)(((offs+(loff_t)len-1)>>this->bbt_erase_shift)  + 1);

	for(i = block_start;i < block_end;i++){
		memset(&einfo, 0, sizeof(einfo));
		einfo.mtd = mtd;
		einfo.addr = ((loff_t)i)<<this->bbt_erase_shift;
		einfo.len = 1 << this->bbt_erase_shift;
		res = nand_erase_nand(mtd, &einfo, 1);

		if(res != 0)
			return res;
	}

	return 0;
}
EXPORT_SYMBOL(rtk_nand_erase);


int rtk_nand_read(struct mtd_info *mtd, uint8_t *buf, loff_t  offs,
			 size_t len)
{
	size_t retlen;

	return mtd->read(mtd, offs, len, &retlen, buf);
}
EXPORT_SYMBOL(rtk_nand_read);

int rtk_nand_write(struct mtd_info *mtd, loff_t  offs, size_t len,
			  uint8_t *buf)
{
	size_t retlen;

	return mtd->write(mtd,offs,len,&retlen,buf);

}
EXPORT_SYMBOL(rtk_nand_write);

#if 0
int rtk_nand_write_oob(struct mtd_info *mtd, loff_t  offs, size_t len,
			  uint8_t *buf,uint8_t *oob)
{
	struct mtd_oob_ops ops;

	ops.mode = MTD_OOB_PLACE;
	ops.ooboffs = 0;
	ops.ooblen = mtd->oobsize;
	ops.datbuf = buf;
	ops.oobbuf = oob;
	ops.len = len;

	return mtd->write_oob(mtd, offs, &ops);
}
EXPORT_SYMBOL(rtk_nand_write_oob);
#endif
#endif
