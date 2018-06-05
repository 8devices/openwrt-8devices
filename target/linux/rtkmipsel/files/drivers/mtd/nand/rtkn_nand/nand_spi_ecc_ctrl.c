
#include <nand_spi/ecc_ctrl.h>
#include <nand_spi/nand_spi_ctrl.h>
#include <nand_spi/util.h>
#include <nand_spi/soc.h>
#include <asm/io.h>
#include "rtknflash_wrapper.h"
//typedef unsigned   long    u32_t;


//#define ECC_DECODE_FAIL -1
//#define ECC_DECODE_SUCCESS 0

#if 0
typedef unsigned char u8_t;
typedef signed char s8_t;
typedef unsigned short u16_t;
typedef signed short s16_t;
typedef unsigned long u32_t;
typedef signed long s32_t;
typedef u32_t mem_ptr_t;
typedef int sys_prot_t;
#endif


int
ecc_check_decode_status(void
)
/* Return value =-1   : ECC decode fail
  * Return value =0~6: Number of bits that is correctted
  */
{
  unsigned int ecsr_val = ECC_STATUS_REG;
  int ret = (((ecsr_val >> 8) & 0x1)) ? ECC_DECODE_FAIL : ECC_DECODE_SUCCESS;

  //printf("register value=%x\n",ecsr_val);


  if (ECC_DECODE_FAIL == ret) {
    if (!((ecsr_val >> 4) & 0x1))
      printf(" <ecc decode fail>\n");   //Can't recover
     else{
		/* all ff/ after erase command */
		return 0;
     }
  }
#if  CONF_SPI_NAND_ECC_DBG
  else {
    ret = ((ecsr_val >> 12) & 0xFF);    //Can recover and return uumber of bits that is correctted
    //printf(" <ecc decode pass>, corrected bit=%d \n", ret);
  }
#endif
  return ret;
}




//void ecc_encode_bch_6t(void *dma_addr, void *p_eccbuf)
void
ecc_encode_bch_6t(void *dma_addr, void *p_eccbuf, unsigned int pageaddr
)
{
  unsigned int j;
  unsigned int ecc_start_addr = (unsigned int) dma_addr;
  u8_t *tag_src_addr = (u8_t *) (ecc_start_addr + BCH6_BYTE_PER_CHUNK); //ecc_start_addr + (512bytes * 4)
  u8_t *syn_src_addr = (u8_t *) (tag_src_addr + BCH6_TAG_SIZE * BCH6_SECTOR_PER_CHUNK); //tag addr+=6*4



  // 1. Cache Flush ......
  // _lplr_basic_io.dcache_writeback_invalidate_range((unsigned   int)dma_addr, (unsigned   int)(dma_addr+BCH6_BYTE_PER_CHUNK));

  /* set DMA_endian */
  if(pageaddr >= BOOT_END_PAGE)
	REG32(ECCFR) = REG32(ECCFR) | (1<<13);
  else
	REG32(ECCFR) = REG32(ECCFR) & ~(1<<13);

  // 2. Set ECC dma tag address
  SET_ECC_DMA_TAG_ADDR(PADDR(p_eccbuf));

  /* Move tag to buffer, encode 512B, repeat 4 times */
  for (j = 0; j < BCH6_SECTOR_PER_CHUNK;
       j++, ecc_start_addr += BCH6_SECTOR_SIZE, tag_src_addr +=
       BCH6_TAG_SIZE, syn_src_addr += BCH6_SYN_SIZE) {

    //3. Coypy Tag & Cache Flush ......
    //memcpy (void * dest, const void *src, size_t n);
    memcpy(p_eccbuf, tag_src_addr, BCH6_TAG_SIZE);

    /* cache invalid and writeback */
    dma_cache_wback_inv((unsigned long)p_eccbuf,(unsigned long)(BCH6_SYN_SIZE+BCH6_TAG_SIZE));
    dma_cache_wback_inv((unsigned long)ecc_start_addr,(unsigned long)BCH6_SECTOR_SIZE);

    // _lplr_basic_io.dcache_writeback_invalidate_range((unsigned   int)p_eccbuf, (unsigned   int)(p_eccbuf+BCH6_ECC_BUF_SIZE));

    //4. ECC Encode
    SET_ECC_DMA_START_ADDR(PADDR(ecc_start_addr));
    ECC_ENCODE_KICKOFF();
    WAIT_ECC_RDY();

    //5. Store Tag & Syndrome
    memcpy(tag_src_addr, p_eccbuf, BCH6_TAG_SIZE);



    memcpy(syn_src_addr, (p_eccbuf + BCH6_TAG_SIZE), BCH6_SYN_SIZE);



    //JSW  _lplr_basic_io.dcache_writeback_invalidate_range((unsigned   int)tag_src_addr, (unsigned   int)(tag_src_addr+BCH6_ECC_BUF_SIZE));
  }
}

int
ecc_decode_bch_6t(void *dma_addr, void *p_eccbuf, unsigned int pageaddr
)
/*  Return value:
  *  -1: ECC decode fail
  *  0~6: Number of bits that is correctted
  */
{
  int ret = ECC_DECODE_SUCCESS;
  unsigned int j;
  unsigned int ecc_start_addr = (unsigned int) dma_addr;
  u8_t *tag_src_addr = (u8_t *) (ecc_start_addr + BCH6_BYTE_PER_CHUNK); //BCH6_BYTE_PER_CHUNK=512*4=2048
  u8_t *syn_src_addr = (u8_t *) (tag_src_addr + BCH6_TAG_SIZE * BCH6_SECTOR_PER_CHUNK); //offset=2048+24

  // 1. Cache Flush ......

  // _lplr_basic_io.dcache_writeback_invalidate_range((unsigned   int)dma_addr,(unsigned   int)(dma_addr+BCH6_ECC_SPARE_SIZE));

	/* set DMA_endian */
  if(pageaddr >= BOOT_END_PAGE)
	REG32(ECCFR) = REG32(ECCFR) | (1<<13);
  else
	REG32(ECCFR) = REG32(ECCFR) & ~(1<<13);

  SET_ECC_DMA_TAG_ADDR(PADDR(p_eccbuf));

  /* 2. Move 1st Tag & Syndrome to buffer, decode 1st 512B */
  for (j = 0; j < BCH6_SECTOR_PER_CHUNK;
       j++, ecc_start_addr += BCH6_SECTOR_SIZE, tag_src_addr +=
       BCH6_TAG_SIZE, syn_src_addr += BCH6_SYN_SIZE) {
    memcpy(p_eccbuf, tag_src_addr, BCH6_TAG_SIZE);
    memcpy((p_eccbuf + BCH6_TAG_SIZE), syn_src_addr, BCH6_SYN_SIZE);

	/* cache invalid and writeback */
    dma_cache_wback_inv((unsigned long)p_eccbuf,(unsigned long)(BCH6_SYN_SIZE+BCH6_TAG_SIZE));
    dma_cache_wback_inv((unsigned long)ecc_start_addr,(unsigned long)BCH6_SECTOR_SIZE);

    //_lplr_basic_io.dcache_writeback_invalidate_range((unsigned   int)p_eccbuf, (unsigned   int)(p_eccbuf+BCH6_ECC_BUF_SIZE));

    SET_ECC_DMA_START_ADDR(PADDR((ecc_start_addr)));
    ECC_DECODE_KICKOFF();
    WAIT_ECC_RDY();

    ret = ecc_check_decode_status();
    if (ECC_DECODE_FAIL == ret) {
      printf("ECC_DECODE_FAIL,ecc_start_addr = 0x%x\n", ecc_start_addr);
      return ret;
    }

    memcpy(tag_src_addr, p_eccbuf, BCH6_TAG_SIZE);
    //  _lplr_basic_io.dcache_writeback_invalidate_range((unsigned   int)tag_src_addr, (unsigned   int)(tag_src_addr+BCH6_ECC_BUF_SIZE));
  }
  return ret;
}
