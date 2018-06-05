/******************************************************************************
 * include/linux/mtd/rtk_nand.h
 * Overview: Realtek Nand Flash Specific Function and Data
 * Copyright (c) 2008 Realtek Semiconductor Corp. All Rights Reserved.
 * Modification History:
 *    #000 2008-06-11 Ken-Yu   create file
 *
 *******************************************************************************/
#ifndef __RTK_NAND_H
#define __RTK_NAND_H

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
#include <linux/config.h>
#else
#include <generated/autoconf.h>
#endif

#include <linux/sched.h>
#include <linux/mtd/mtd.h>
//#include <asm/r4kcache.h>
#define NAND_CTRL_BASE  0xB801A000
#ifdef CONFIG_YAFFS_DOES_ECC	//"Lets Yaffs do its own ECC
	#define RTK_HW_ECC 0
#else
	#define RTK_HW_ECC 1
#endif

#define UNCACHE_MASK  0xa0000000

/*
define mars read/write NAND HW registers
use them because standard readb/writeb have warning msgs in our gcc 2.96
ex: passing arg 2 of `writeb' makes pointer from integer without a cast
*/
#define rtk_readb(offset)         (*(volatile unsigned char *)(offset))
#define rtk_readw(offset)         (*(volatile unsigned short *)(offset))
#define rtk_readl(offset)         (*(volatile unsigned long *)(offset))

#define rtk_writeb(val, offset)    (*(volatile unsigned char *)(offset) = val)
#define rtk_writew(val, offset)    (*(volatile unsigned short *)(offset) = val)
#define rtk_writel(val, offset)    (*(volatile unsigned long *)(offset) = val)

/*
	macro RTK_FLUSH_CACHE is to flush the cache at address "addr",
	the length is "len"
*/
#define RTK_FLUSH_CACHE(addr, len)		\
do {								\
	unsigned long dc_lsize = current_cpu_data.dcache.linesz;	\
	unsigned long end, a;								\
	a = (unsigned long ) addr & ~(dc_lsize - 1);		\
	end = ((unsigned long )addr + len - 1) & ~(dc_lsize - 1);	\
	while (1) {		\
		flush_dcache_line(a);   /* Hit_Writeback_Inv_D */	\
		if (a == end)	\
			break;	\
		a += dc_lsize;	\
	}	\
} while (0)

/* =========================== BEGIN: FOR RTK========================= */

#define MAKER_ID_RENESAS			0x07		//Maker Code of Renesas (AND-Type Flash)
#define MAKER_ID_ST						0x20		//Maker Code of ST
#define MAKER_ID_MICRON			0x2C		//Maker Code of Micron
#define MAKER_ID_TOSHIBA			0x98		//Maker Code of Toshiba
#define MAKER_ID_HYNIX				0xAD	//Maker Code of Hynix
#define MAKER_ID_SAMSUNG		0xEC	//Maker Code of Samsung
#define MAKER_ID_UNKNOWN	0xFF		//Unknown Maker Code

#define BACKUP_BBT 3
#define BOOT_SIZE	           CONFIG_BOOT_SIZE
#define USER_SPACE_SIZE        0x800000
#define VIMG_SPACE_SIZE        0x600000
#define	BB_INIT	0xFFFE
#define	RB_INIT	0xFFFD
#define	BBT_TAG	0xBB
#define BB_DIE_INIT	0xEEEE
#define RB_DIE_INIT	BB_DIE_INIT

#if 0
#ifdef CONFIG_RTK_NAND_PAGE_2K
#define KEEP_ORI_BBI /*keep original bad block indicator, for page size 2K*/
#endif

#ifdef KEEP_ORI_BBI
#define DATA_BBI_OFF  ((512*4)- 48)/*(0~1999)512+512+512+512-48*/
#define OOB_BBI_OFF  ((16*4) - 10 -1) /*(0~23)6+[10]+6+[10]+6+[10]+6+[10] []:ecc bytes*/
#else
#define OOB_BBI_OFF  0 /*(0~23)6+[10]+6+[10]+6+[10]+6+[10] []:ecc bytes*/
#define DATA_BBI_OFF 0
#endif
#endif

#ifdef CONFIG_RTK_REMAP_BBT
typedef struct  __attribute__ ((__packed__)){
    u16 BB_die;
    u16 bad_block;
    u16 RB_die;
    u16 remap_block;
}BB_t;


typedef struct  __attribute__ ((__packed__)){
    u32 block_r;
}BB_v2r;

#endif //CONFIG_RTK_REMAP_BBT
#ifdef CONFIG_RTK_NORMAL_BBT
#define NORMAL_BBT_POSITION           0x8000000 /*128MB*/
typedef struct  __attribute__ ((__packed__)){
    u16 BB_die;
    u16 bad_block;
    u16 RB_die;
    u16 block_info;
}BBT_normal;
#endif //CONFIG_RTK_NORMAL_BBT
typedef struct __attribute__ ((__packed__)){
    unsigned char  *name;
    unsigned int id;
    unsigned int size;	//nand total size
    unsigned int chipsize;	//die size
    unsigned short PageSize;
    unsigned int BlockSize;
    unsigned short OobSize;
    unsigned char num_chips;
    unsigned char isLastPage;	//page position of block to check BB
    unsigned char CycleID5th; //If CycleID5th do not exist, set it to 0xff
    unsigned char T1;
    unsigned char T2;
    unsigned char T3;
} device_type_t;

/* NAND Flash Command Sets */
#define CMD_READ_ID				0x90
#define CMD_READ_STATUS		0x70

#define CMD_PG_READ_C1		0x00
#define CMD_PG_READ_C2		0x30
#define CMD_PG_READ_C3		CMD_READ_STATUS


#define CMD_PG_READ_A		0x00
#define CMD_PG_READ_B		0x01
#define CMD_PG_READ_C		0x50

#define CMD_PG_WRITE_C1		0x80
#define CMD_PG_WRITE_C2		0x10
#define CMD_PG_WRITE_C3		CMD_READ_STATUS

#define CMD_BLK_ERASE_C1		0x60	//Auto Block Erase Setup command
#define CMD_BLK_ERASE_C2		0xd0	//CMD_ERASE_START
#define CMD_BLK_ERASE_C3		CMD_READ_STATUS	//CMD_STATUS_READ

#define CMD_RESET                 0xff
#define CMD_RANDOM_DATA_INPUT     0x85    /* RANDOM DATA write */

#define CMD_RANDOM_DATA_OUTPUT_C1 0x05    /* RANDOM DATA read */
#define CMD_RANDOM_DATA_OUTPUT_C2 0xe0

/* NAND_ISR status, 0x1801_0324 */
#define NAND_ISR_XFER_MODE_DONE	(1 << 2)
#define NAND_ISR_AUTO_MODE_DONE	(1 << 3)
#define NAND_ISR_DMA_DONE	(1 << 4)
#define NAND_ISR_PFS_DONE	(1 << 7)	//polling flash status busy = 1

/* CMYu, 20090720 */
#define MCP_NONE	0
#define MCP_AES_ECB	1
#define MCP_AES_CBC	2
#define MCP_AES_CTR	3

/*
 * Searches for a NAND device
 */
extern int rtk_nand_scan (struct mtd_info *mtd, int maxchips);

/* =========================== END: FOR RTK========================= */


/* The maximum number of NAND chips in an array */
#define NAND_MAX_CHIPS		8

/*
 * Standard NAND flash commands
 */
#define NAND_CMD_READ0		0
#define NAND_CMD_READ1		1
#define NAND_CMD_PAGEPROG	0x10
#define NAND_CMD_READOOB	0x50
#define NAND_CMD_ERASE1		0x60
#define NAND_CMD_STATUS		0x70
#define NAND_CMD_STATUS_MULTI	0x71
#define NAND_CMD_SEQIN		0x80
#define NAND_CMD_READID		0x90
#define NAND_CMD_ERASE2		0xd0
#define NAND_CMD_RESET		0xff

/* Extended commands for large page devices */
#define NAND_CMD_READSTART	0x30
#define NAND_CMD_CACHEDPROG	0x15


/*
 * Constants for ECC_MODES
 */

/* No ECC. Usage is not recommended ! */
#define NAND_ECC_NONE           0
/* Software ECC 3 byte ECC per 256 Byte data */
#define NAND_ECC_SOFT           1
/* Hardware ECC 3 byte ECC per 256 Byte data */
#define NAND_ECC_HW3_256        2
/* Hardware ECC 3 byte ECC per 512 Byte data */
#define NAND_ECC_HW3_512        3
/* Hardware ECC 3 byte ECC per 512 Byte data */
#define NAND_ECC_HW6_512        4
/* Hardware ECC 8 byte ECC per 512 Byte data */
#define NAND_ECC_HW8_512        6
/* Hardware ECC 12 byte ECC per 2048 Byte data */
#define NAND_ECC_HW12_2048      7

/* Option constants for bizarre disfunctionality and real
*  features
*/
/* Chip can not auto increment pages */
#define NAND_NO_AUTOINCR	0x00000001
/* Buswitdh is 16 bit */
#define NAND_BUSWIDTH_16	0x00000002
/* Device supports partial programming without padding */
#define NAND_NO_PADDING		0x00000004
/* Chip has cache program function */
#define NAND_CACHEPRG		0x00000008
/* Chip has copy back function */
#define NAND_COPYBACK		0x00000010
/* AND Chip which has 4 banks and a confusing page / block
 * assignment. See Renesas datasheet for further information */
#define NAND_IS_AND		0x00000020
/* Chip has a array of 4 pages which can be read without
 * additional ready /busy waits */
#define NAND_4PAGE_ARRAY	0x00000040
/* Chip requires that BBT is periodically rewritten to prevent
 * bits from adjacent blocks from 'leaking' in altering data.
 * This happens with the Renesas AG-AND chips, possibly others.  */
#define BBT_AUTO_REFRESH	0x00000080

/* Options valid for Samsung large page devices */
#define NAND_SAMSUNG_LP_OPTIONS \
	(NAND_NO_PADDING | NAND_CACHEPRG | NAND_COPYBACK)


/* Macros to identify the above */
#define NAND_CANAUTOINCR(chip) (!(chip->options & NAND_NO_AUTOINCR))
#define NAND_MUST_PAD(chip) (!(chip->options & NAND_NO_PADDING))
#define NAND_HAS_CACHEPROG(chip) ((chip->options & NAND_CACHEPRG))
#define NAND_HAS_COPYBACK(chip) ((chip->options & NAND_COPYBACK))

/* Mask to zero out the chip options, which come from the id table */
#define NAND_CHIPOPTIONS_MSK	(0x0000ffff & ~NAND_NO_AUTOINCR)

/* Non chip related options */
/* Use a flash based bad block table. This option is passed to the
 * default bad block table function. */
#define NAND_USE_FLASH_BBT	0x00010000
/* The hw ecc generator provides a syndrome instead a ecc value on read
 * This can only work if we have the ecc bytes directly behind the
 * data bytes. Applies for DOC and AG-AND Renesas HW Reed Solomon generators */
#define NAND_HWECC_SYNDROME	0x00020000
/* This option skips the bbt scan during initialization. */
#define NAND_SKIP_BBTSCAN	0x00040000

/* Options set by nand scan */
/* Nand scan has allocated oob_buf */
#define NAND_OOBBUF_ALLOC	0x40000000
/* Nand scan has allocated data_buf */
#define NAND_DATABUF_ALLOC	0x80000000

#define Boot_Select			0xB801a000
#define NAND_ADDR_MASK		(1<<31)
/*NAND_ADDR_CYCLE = 1, address cycle=3, NAND_ADDR_CYCLE=0, address cycle=4 or 5*/
#ifndef CONFIG_RTL_8198C
#define NAND_ADDR_CYCLE	(((*(volatile unsigned int *)((Boot_Select)) & NAND_ADDR_MASK) == 0) ? 1:0)
#else
//just for test
#define NAND_ADDR_CYCLE	0
#endif
#define NACMR    (NAND_CTRL_BASE + 0x08)



/*
 * nand_state_t - chip states
 * Enumeration for NAND flash chip state
 */
typedef enum {
	FL_READY,
	FL_READING,
	FL_WRITING,
	FL_ERASING,
	FL_SYNCING,
	FL_CACHEDPRG,
	FL_PM_SUSPENDED,
	FL_UNKNOWN
} nand_state_t;

/**
 * struct nand_hw_control - Control structure for hardware controller (e.g ECC generator) shared among independend devices
 * @lock:               protection lock
 * @active:		the mtd device which holds the controller currently
 * @wq:			wait queue to sleep on if a NAND operation is in progress
 *                      used instead of the per chip wait queue when a hw controller is available
 */
struct nand_hw_control {
	spinlock_t	 lock;
	struct nand_chip *active;
	wait_queue_head_t wq;
};

/**
 * struct nand_chip - NAND Private Flash Chip Data
 * @IO_ADDR_R:		[BOARDSPECIFIC] address to read the 8 I/O lines of the flash device
 * @IO_ADDR_W:		[BOARDSPECIFIC] address to write the 8 I/O lines of the flash device
 * @read_byte:		[REPLACEABLE] read one byte from the chip
 * @write_byte:		[REPLACEABLE] write one byte to the chip
 * @read_word:		[REPLACEABLE] read one word from the chip
 * @write_word:		[REPLACEABLE] write one word to the chip
 * @write_buf:		[REPLACEABLE] write data from the buffer to the chip
 * @read_buf:		[REPLACEABLE] read data from the chip into the buffer
 * @verify_buf:		[REPLACEABLE] verify buffer contents against the chip data
 * @select_chip:	[REPLACEABLE] select chip nr
 * @block_bad:		[REPLACEABLE] check, if the block is bad
 * @block_markbad:	[REPLACEABLE] mark the block bad
 * @hwcontrol:		[BOARDSPECIFIC] hardwarespecific function for accesing control-lines
 * @dev_ready:		[BOARDSPECIFIC] hardwarespecific function for accesing device ready/busy line
 *			If set to NULL no access to ready/busy is available and the ready/busy information
 *			is read from the chip status register
 * @cmdfunc:		[REPLACEABLE] hardwarespecific function for writing commands to the chip
 * @waitfunc:		[REPLACEABLE] hardwarespecific function for wait on ready
 * @calculate_ecc: 	[REPLACEABLE] function for ecc calculation or readback from ecc hardware
 * @correct_data:	[REPLACEABLE] function for ecc correction, matching to ecc generator (sw/hw)
 * @enable_hwecc:	[BOARDSPECIFIC] function to enable (reset) hardware ecc generator. Must only
 *			be provided if a hardware ECC is available
 * @erase_cmd:		[INTERN] erase command write function, selectable due to AND support
 * @scan_bbt:		[REPLACEABLE] function to scan bad block table
 * @eccmode:		[BOARDSPECIFIC] mode of ecc, see defines
 * @eccsize: 		[INTERN] databytes used per ecc-calculation
 * @eccbytes: 		[INTERN] number of ecc bytes per ecc-calculation step
 * @eccsteps:		[INTERN] number of ecc calculation steps per page
 * @chip_delay:		[BOARDSPECIFIC] chip dependent delay for transfering data from array to read regs (tR)
 * @chip_lock:		[INTERN] spinlock used to protect access to this structure and the chip
 * @wq:			[INTERN] wait queue to sleep on if a NAND operation is in progress
 * @state: 		[INTERN] the current state of the NAND device
 * @page_shift:		[INTERN] number of address bits in a page (column address bits)
 * @phys_erase_shift:	[INTERN] number of address bits in a physical eraseblock
 * @bbt_erase_shift:	[INTERN] number of address bits in a bbt entry
 * @chip_shift:		[INTERN] number of address bits in one chip
 * @data_buf:		[INTERN] internal buffer for one page + oob
 * @oob_buf:		[INTERN] oob buffer for one eraseblock
 * @oobdirty:		[INTERN] indicates that oob_buf must be reinitialized
 * @data_poi:		[INTERN] pointer to a data buffer
 * @options:		[BOARDSPECIFIC] various chip options. They can partly be set to inform nand_scan about
 *			special functionality. See the defines for further explanation
 * @badblockpos:	[INTERN] position of the bad block marker in the oob area
 * @numchips:		[INTERN] number of physical chips
 * @chipsize:		[INTERN] the size of one chip for multichip arrays
 * @pagemask:		[INTERN] page number mask = number of (pages / chip) - 1
 * @pagebuf:		[INTERN] holds the pagenumber which is currently in data_buf
 * @autooob:		[REPLACEABLE] the default (auto)placement scheme
 * @bbt:		[INTERN] bad block table pointer
 * @bbt_td:		[REPLACEABLE] bad block table descriptor for flash lookup
 * @bbt_md:		[REPLACEABLE] bad block table mirror descriptor
 * @badblock_pattern:	[REPLACEABLE] bad block scan pattern used for initial bad block scan
 * @controller:		[OPTIONAL] a pointer to a hardware controller structure which is shared among multiple independend devices
 * @priv:		[OPTIONAL] pointer to private chip date
 * @errstat:		[OPTIONAL] hardware specific function to perform additional error status checks
 *			(determine if errors are correctable)
 */

struct nand_chip {
	u_char oob_shift;
	void (*read_id) (struct mtd_info *mtd, unsigned char id[5], int chip_sel);
	int (*read_ecc_page) (struct mtd_info *mtd, u16 chipnr, unsigned int page, u_char *data,
									u_char *oob_buf);
	int (*read_oob) (struct mtd_info *mtd, u16 chipnr, int page, int len, u_char *buf);
	int (*write_ecc_page) (struct mtd_info *mtd, u16 chipnr, unsigned int page, const u_char *data,
										const u_char *oob_buf, int isBBT);
	int (*write_oob) (struct mtd_info *mtd, u16 chipnr, int page, int len, const u_char *buf);
	int (*erase_block) (struct mtd_info *mtd, u16 chipnr, int page);
	void (*sync) (struct mtd_info *mtd);
	/* CMYu, 20090422 */
	void (*suspend) (struct mtd_info *mtd);
	void (*resume) (struct mtd_info *mtd);
	/* Ken.Yu, 20080615 */
	void (*select_chip) (struct mtd_info *mtd, int chip);
	void	(*read_buf)(struct mtd_info *mtd, u_char *buf, int len);
	void 	(*cmdfunc)(struct mtd_info *mtd, unsigned command, int column, int page_addr);
	int  (*dev_ready)(struct mtd_info *mtd);
	int (*scan_bbt)(struct mtd_info *mtd);
#ifdef CONFIG_RTK_REMAP_BBT
	int (*scan_v2r_bbt)(struct mtd_info *mtd);
#endif
	int		eccmode;
	int		eccsize;
	int		eccbytes;
	int		eccsteps;
	int 		chip_delay;
	spinlock_t	chip_lock;
	wait_queue_head_t wq;
	nand_state_t 	state;
	int 		page_shift;
	int		phys_erase_shift;
	int		bbt_erase_shift;
	int		chip_shift;
	u_char 		*g_databuf;
	u_char		*g_oobbuf;
	int		oobdirty;
	u_char		*data_poi;
	unsigned int	options;
	int		badblockpos;
	int		numchips;
	unsigned long	chipsize;
	/* Ken-Yu, 20090108 */
	unsigned long	device_size;
	int		pagemask;
	int		pagebuf;
	struct nand_oobinfo	*autooob;
	//uint8_t		*bbt;
	struct nand_bbt_descr	*bbt_td;
	struct nand_bbt_descr	*bbt_md;
	struct nand_bbt_descr	*badblock_pattern;
	struct nand_hw_control  *controller;
	void		*priv;
	/* Ken-Yu, 20080618 */
	unsigned char maker_code;
	unsigned char device_code;
	unsigned int ppb;	//page per block
	unsigned int oob_size;	//spare area size
	unsigned int block_num;
	unsigned int page_num;
#ifdef CONFIG_RTK_REMAP_BBT
	BB_t *bbt;
	BB_v2r *bbt_v2r;
	unsigned int RBA;
	unsigned int RBA_PERCENT;
#endif
	unsigned int BBs;
	unsigned int block_size;
#ifdef CONFIG_RTK_NORMAL_BBT
	unsigned int bbt_num;
	unsigned int BBT_PERCENT;
	BBT_normal *bbt_nor;
#endif
	struct nand_hw_control  rtk_controller;
	/* Ken-Yu, 20081004 */
	__u32 *erase_page_flag;
	/* Ken-Yu, 20081013 */
	unsigned char active_chip;
	//unsigned int BBs;
	/* CMYu, 20090720 */
	unsigned int mcp;
};

/*
 * NAND Flash Manufacturer ID Codes
 */
#define NAND_MFR_TOSHIBA		0x98
#define NAND_MFR_SAMSUNG		0xec
#define NAND_MFR_FUJITSU		0x04
#define NAND_MFR_NATIONAL		0x8f
#define NAND_MFR_RENESAS		0x07
#define NAND_MFR_STMICRO		0x20
#define NAND_MFR_HYNIX			0xad

/**
 * struct nand_flash_dev - NAND Flash Device ID Structure
 *
 * @name:  	Identify the device type
 * @dev_id:   	device ID code
 * @pagesize:  	Pagesize in bytes. Either 256 or 512 or 0
 *		If the pagesize is 0, then the real pagesize
 *		and the eraseize are determined from the
 *		extended id bytes in the chip
 * @erasesize: 	Size of an erase block in the flash device.
 * @chipsize:  	Total chipsize in Mega Bytes
 * @options:	Bitfield to store chip relevant options
 */
struct nand_flash_dev {
	char *name;
	int dev_id;
	unsigned long pagesize;
	unsigned long chipsize;
	unsigned long erasesize;
	unsigned long options;
};

/**
 * struct nand_manufacturers - NAND Flash Manufacturer ID Structure
 * @name:	Manufacturer name
 * @id: 	manufacturer ID code of device.
*/
struct nand_manufacturers {
	int id;
	char * name;
};

extern struct nand_flash_dev nand_flash_ids[];
extern struct nand_manufacturers nand_manuf_ids[];

/**
 * struct nand_bbt_descr - bad block table descriptor
 * @options:	options for this descriptor
 * @pages:	the page(s) where we find the bbt, used with option BBT_ABSPAGE
 *		when bbt is searched, then we store the found bbts pages here.
 *		Its an array and supports up to 8 chips now
 * @offs:	offset of the pattern in the oob area of the page
 * @veroffs:	offset of the bbt version counter in the oob are of the page
 * @version:	version read from the bbt page during scan
 * @len:	length of the pattern, if 0 no pattern check is performed
 * @maxblocks:	maximum number of blocks to search for a bbt. This number of
 *		blocks is reserved at the end of the device where the tables are
 *		written.
 * @reserved_block_code: if non-0, this pattern denotes a reserved (rather than
 *              bad) block in the stored bbt
 * @pattern:	pattern to identify bad block table or factory marked good /
 *		bad blocks, can be NULL, if len = 0
 *
 * Descriptor for the bad block table marker and the descriptor for the
 * pattern which identifies good and bad blocks. The assumption is made
 * that the pattern and the version count are always located in the oob area
 * of the first block.
 */
struct nand_bbt_descr {
	int	options;
	int	pages[NAND_MAX_CHIPS];
	int	offs;
	int	veroffs;
	uint8_t	version[NAND_MAX_CHIPS];
	int	len;
	int 	maxblocks;
	int	reserved_block_code;
	uint8_t	*pattern;
};

/* Options for the bad block table descriptors */

/* The number of bits used per block in the bbt on the device */
#define NAND_BBT_NRBITS_MSK	0x0000000F
#define NAND_BBT_1BIT		0x00000001
#define NAND_BBT_2BIT		0x00000002
#define NAND_BBT_4BIT		0x00000004
#define NAND_BBT_8BIT		0x00000008
/* The bad block table is in the last good block of the device */
#define	NAND_BBT_LASTBLOCK	0x00000010
/* The bbt is at the given page, else we must scan for the bbt */
#define NAND_BBT_ABSPAGE	0x00000020
/* The bbt is at the given page, else we must scan for the bbt */
#define NAND_BBT_SEARCH		0x00000040
/* bbt is stored per chip on multichip devices */
#define NAND_BBT_PERCHIP	0x00000080
/* bbt has a version counter at offset veroffs */
#define NAND_BBT_VERSION	0x00000100
/* Create a bbt if none axists */
#define NAND_BBT_CREATE		0x00000200
/* Search good / bad pattern through all pages of a block */
#define NAND_BBT_SCANALLPAGES	0x00000400
/* Scan block empty during good / bad block scan */
#define NAND_BBT_SCANEMPTY	0x00000800
/* Write bbt if neccecary */
#define NAND_BBT_WRITE		0x00001000
/* Read and write back block contents when writing bbt */
#define NAND_BBT_SAVECONTENT	0x00002000
/* Search good / bad pattern on the first and the second page */
#define NAND_BBT_SCAN2NDPAGE	0x00004000

/* The maximum number of blocks to scan for a bbt */
#define NAND_BBT_SCAN_MAXBLOCKS	4

extern int nand_scan_bbt (struct mtd_info *mtd, struct nand_bbt_descr *bd);
extern int nand_update_bbt (struct mtd_info *mtd, loff_t offs);
extern int nand_default_bbt (struct mtd_info *mtd);
extern int nand_isbad_bbt (struct mtd_info *mtd, loff_t offs, int allowbbt);
extern int nand_erase_nand (struct mtd_info *mtd, struct erase_info *instr, int allowbbt);
extern int nand_do_read_ecc (struct mtd_info *mtd, loff_t from, size_t len,
                             size_t * retlen, u_char * buf, u_char * oob_buf,
                             struct nand_oobinfo *oobsel, int flags);

/*
* Constants for oob configuration
*/
#define NAND_SMALL_BADBLOCK_POS		5
#define NAND_LARGE_BADBLOCK_POS		0
#define CECS1 	(1<<31)
#define CECS0	(1<<30)
#define NAND_CTRL_BASE  0xB801A000
#define NACFR  (NAND_CTRL_BASE + 0x0)
#define NACR    (NAND_CTRL_BASE + 0x04)
#ifndef CONFIG_RTL_8198C
#define NASR        (NAND_CTRL_BASE + 0x20)
#endif
#define NECN			(1<<4)
#define NRER			(1<<3)
#define NWER			(1<<2)
#define NDRS			(1<<1)
#define NDWS		(1<<0)
#ifndef CONFIG_RTL_8198C
#define NANDPReg	(NAND_CTRL_BASE + 0x24)
#define NADTSAR	(NAND_CTRL_BASE + 0x54)
#endif
#define M_mask		0xe0000000
#define ECC_enable    (1<<30)
#define RBO		     (1<<29)
#define WBO		     (1<<28)
#define NAADR    (NAND_CTRL_BASE + 0x0C)
#define enNextAD		(1<<27)
#define AD2EN		(1<<26)
#define AD1EN		(1<<25)
#define AD0EN		(1<<24)
#define CE_ADDR2		16
#define CE_ADDR1		8
#define CE_ADDR0		0
#define NADCRR   (NAND_CTRL_BASE + 0x10)
#define TAG_SEL		7
#define TAG_DIS		(1<<6)
#define DESC1		(1<<5)
#define DESC0		(1<<4)
#define DMARE		(1<<3)
#define DMAWE		(1<<2)
#define LBC_128		3
#define LBC_64		2
#define LBC_32		1
#define LBC_16		0
#define NADR        (NAND_CTRL_BASE + 0x14)
#define NADFSAR (NAND_CTRL_BASE + 0x18)
#ifndef CONFIG_RTL_8198C
#define NADRSAR (NAND_CTRL_BASE + 0x1C)
#define NASR        (NAND_CTRL_BASE + 0x20)
#else
#define NADRSAR (NAND_CTRL_BASE + 0x20)
#define NADTSAR	(NAND_CTRL_BASE + 0x24)
#define NASR        (NAND_CTRL_BASE + 0x28)
#define NANDPReg	(NAND_CTRL_BASE + 0x3c
#endif
#define Chip_Seletc_Base	30
/////////////////////////////////////////////////////////////////////
//#define CONFIG_BOOT_SIZE CONFIG_RTL_HW_SETTING_OFFSET
#define CONFIG_BOOT_SIZE 0x100000

/*winfred_wang*/
#define CACHELINE_SIZE		36
#define NAND_LOADER_CHUNK	64

#define SWAP_2K_DATA
#ifdef SWAP_2K_DATA
#define DATA_BBI_OFF  ((512*4)- 48)/*(0~1999)512+512+512+512-48*/
#define OOB_BBI_OFF   23//	((16*4) - 10 -1) /*(0~23)6+[10]+6+[10]+6+[10]+6+[10] []:ecc bytes*/
#else
#define OOB_BBI_OFF  0 /*(0~23)6+[10]+6+[10]+6+[10]+6+[10] []:ecc bytes*/
#define DATA_BBI_OFF 0
#endif



#endif  /* __RTK_NAND_H */
