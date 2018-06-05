#ifndef __RTKN_PARALLEL_H__
#define __RTKN_PARALLEL_H__

#define DBG_SHOW_MSG_ENABLE		(0)		//define if show debug message;
#define DBG_MODE_ENABLE			(0)   	//Enable debug mode or not;
#define TEST_NF_DRIVER			(0)   	//Enable test procedure for NAND flash driver
#define RTK_NAND_TEST (0) 		//+alexchang add 0702-2010
#define RTK_ARD_ALGORITHM (0)	//Enable Avoid Read Disturbance Algorithm or not
#define RTK_CP_DISABLE	(0)		//Enable content protection for NAND driver or not

//#define ERESTARTSYS	(512)

//#define RTK_ROM_CODE
//static spinlock_t	 lock_NF_CARDREADER;

/*
define mars read/write NAND HW registers
use them because standard readb/writeb have warning msgs in our gcc 2.96
ex: passing arg 2 of `writeb' makes pointer from integer without a cast
*/
#define REG_READ_U8(register)         		(*(volatile unsigned char *)(register))
#define REG_READ_U16(register)         		(*(volatile unsigned short *)(register))
#define REG_READ_U32(register)         		(*(volatile unsigned long *)(register))

#define REG_WRITE_U8(register, value)    		(*(volatile unsigned char *)(register) = value)
#define REG_WRITE_U16(register, value)    		(*(volatile unsigned short *)(register) = value)
#define REG_WRITE_U32(register, value)    		(*(volatile unsigned long *)(register) = value)

#define MTDSIZE	(sizeof (struct mtd_info) + sizeof (struct nand_chip))
#define MAX_PARTITIONS	16
#define BOOTCODE	16*1024*1024	//16MB
//#define GFP_KERNEL			0

/*
 * Searches for a NAND device
 */
extern int rtk_nand_scan (struct mtd_info *mtd, int maxchips);


//========================================================================

#if 0
/* Reserve Block Area usage */
#define	BB_INIT	0xFFFE
#define	RB_INIT	0xFFFD
#define	BBT_TAG	0xBB
#define TAG_FACTORY_PARAM	(0x82)
#define BB_DIE_INIT	0xEEEE
#define RB_DIE_INIT	BB_DIE_INIT
typedef struct /*__attribute__ ((__packed__))*/{
    u16 BB_die;
    u16 bad_block;
    u16 RB_die;
    u16 remap_block;
}BB_t;

typedef struct /*__attribute__ ((__packed__))*/{
    unsigned char  *name;
    unsigned int id;
    uint64_t 	 size;	//nand total size
    uint64_t	 chipsize;	//die size
    unsigned int PageSize;
    unsigned int BlockSize;
    unsigned short OobSize;
    unsigned char num_chips;
    unsigned char isLastPage;	//page position of block to check BB
    unsigned char CycleID5th; //If CycleID5th do not exist, set it to 0xff
    unsigned char CycleID6th; //If CycleID6th do not exist, set it to 0xff
    unsigned short ecc_num;
    unsigned char T1;
    unsigned char T2;
    unsigned char T3;
	unsigned short eccSelect;//Ecc ability select:   add by alexchang 0319-2010
} device_type_t;
#endif

/* NAND Flash Command Sets */
#define CMD_READ_ID				0x90
#define CMD_READ_STATUS		0x70
#define CMD_READ_PARAMETER 0xec

#define CMD_PG_READ_C1		0x00
#define CMD_PG_READ_C2		0x30
#define CMD_PG_READ_C3		CMD_READ_STATUS

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

#define NAND_ISR_XFER_MODE_DONE	(1 << 2)
#define NAND_ISR_AUTO_MODE_DONE	(1 << 3)
#define NAND_ISR_DMA_DONE	(1 << 4)
#define NAND_ISR_PFS_DONE	(1 << 7)	//polling flash status busy = 1

#define MCP_NONE	0
#define MCP_AES_ECB	1
#define MCP_AES_CBC	2
#define MCP_AES_CTR	3

/* winfred_wang */
/* for ecc < 0x18,donot need swap */
//#define SWAP_2K_DATA
#ifdef SWAP_2K_DATA
#define DATA_BBI_OFF  ((512*4)- 48)	/*(0~1999)512+512+512+512-48*/
#define OOB_BBI_OFF   53			/* 16+16+16+5*/
#endif

#endif