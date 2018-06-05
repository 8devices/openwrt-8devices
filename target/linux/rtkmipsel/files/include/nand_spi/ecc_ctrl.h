#ifndef ECC_CTRL_H
#define ECC_CTRL_H

//#include <soc.h>

#define CONF_SPI_NAND_ECC_DBG 1

/***********************************************
  * Linker Section Information
  ***********************************************/

#if 1//ndef SECTION_NAND_SPI
    #define SECTION_NAND_SPI
#endif

/* Register Macro */
#ifndef REG32
#define REG32(reg)      (*(volatile unsigned int*)(reg))
#endif
#ifndef REG16
#define REG16(reg)      (*(volatile unsigned short *)(reg))
#endif
#ifndef REG8
#define REG8(reg)       (*(volatile unsigned char  *)(reg))
#endif



/***********************************************
  * ECC Controller Registers
  ***********************************************/

#if 1 //8196F
#define ECC_STATUS_REG       REG32(0xB801A614)
#define ECC_ENCODE_KICKOFF() REG32(0xB801A608)=1
#define ECC_DECODE_KICKOFF() REG32(0xB801A608)=0
#define SET_ECC_DMA_START_ADDR(phy_addr) REG32(0xB801A60C)=phy_addr
#define SET_ECC_DMA_TAG_ADDR(phy_addr)   REG32(0xB801A610)=phy_addr
#define WAIT_ECC_RDY() while((ECC_STATUS_REG&1))
#else //8198E
#define ECC_STATUS_REG       REG32(0xf801A614)
#define ECC_ENCODE_KICKOFF() REG32(0xf801A608)=1
#define ECC_DECODE_KICKOFF() REG32(0xf801A608)=0
#define SET_ECC_DMA_START_ADDR(phy_addr) REG32(0xf801A60C)=phy_addr
#define SET_ECC_DMA_TAG_ADDR(phy_addr)   REG32(0xf801A610)=phy_addr
#define WAIT_ECC_RDY() while((ECC_STATUS_REG&1))

//#define PADDR(addr)  (((void*)addr) & 0x1FFFFFFF)
//#define PADDR(addr)  ((addr) & 0x1FFFFFFF)
//#define PADDR(addr)
#endif

/***********************************************
  * 6-T BCH Size & Type Definition
  ***********************************************/
#define BCH6_SECTOR_SIZE  (512)
#define BCH6_SECTOR_PER_CHUNK (4)
#define BCH6_BYTE_PER_CHUNK   (BCH6_SECTOR_SIZE*BCH6_SECTOR_PER_CHUNK)
#define BCH6_TAG_SIZE     (6)
#define BCH6_SYN_SIZE     (10)
#define BCH6_ECC_BUF_SIZE (BCH6_TAG_SIZE+BCH6_SYN_SIZE)
#define BCH6_ECC_SPARE_SIZE (BCH6_ECC_BUF_SIZE*BCH6_SECTOR_PER_CHUNK)


/***********************************************
  * ECC Driver APIs
  ***********************************************/
extern SECTION_NAND_SPI void ecc_encode_bch_6t(void *dma_addr, void *p_eccbuf,unsigned int pageaddr);
extern SECTION_NAND_SPI int ecc_decode_bch_6t(void *dma_addr, void *p_eccbuf,unsigned int pageaddr);
extern SECTION_NAND_SPI int ecc_check_decode_status(void);


/* register */
#define ECCFR		0xb801A600


/* bootcode page start_page */
#define BOOT_START_PAGE		0
#define BOOT_END_PAGE		512 //(0x100000/2048)


#endif //#ifndef ECC_CTRL_H
