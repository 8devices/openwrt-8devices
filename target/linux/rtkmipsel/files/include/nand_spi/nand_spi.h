#ifndef WINBOND_SPI_NAND_H
#define WINBOND_SPI_NAND_H




#define Strtoul simple_strtoul

#ifdef __UBOOT__
#define CONF_SPI_NAND_UBOOT_COMMAND 1
#else
#define CONF_SPI_NAND_UBOOT_COMMAND 0
#endif

/***********************************************
  * Winbond's opcode
  ***********************************************/
#define WINBOND_RESET_OP         (0xFF)
#define WINBOND_SET_FEATURE_OP   (0x1F)
#define WINBOND_BLOCK_ERASE_OP   (0xD8)
#define WINBOND_RDID_OP          (0x9F)
#define WINBOND_GET_FEATURE_OP   (0x0F)
#define WINBOND_NORMAL_READ      (0x03)
#define WINBOND_FAST_READ        (0x0B)
#define WINBOND_FAST_READ_X2     (0x3B)
#define WINBOND_FAST_READ_X4     (0x6B)
#define WINBOND_FAST_READ_DIO    (0xBB)
#define WINBOND_FAST_READ_QIO    (0xEB)
#define WINBOND_PROGRAM_LOAD     (0x02)
#define WINBOND_PROGRAM_LOAD_X4  (0x32)
#define WINBOND_WRITE_OP    (winbond_cmd_info.w_cmd)
#define WINBOND_READ_OP     (winbond_cmd_info.r_cmd)
/* BBM releated */
#define WINBOND_BBM_OP			(0xA1)
#define WINBOND_READ_BBM_TBL_OP	(0xA5)



/***********************************************
  * Winbond's driver function
  ***********************************************/
#define WINBOND_RESET_FUNC           (snffcn_pio_raw_cmd)
#define WINBOND_SET_FEATURE_FUNC     (snffcn_pio_raw_cmd)  /*quad_enable(),  winbond_wait_spi_nand_ready(), winbond_wait_spi_nand_ready()*/
#define WINBOND_BLOCK_ERASE_FUNC     (snffcn_pio_raw_cmd) /*block_erase()*/
#define WINBOND_RDID_FUNC            (snffcn_pio_raw_cmd)
#define WINBOND_GET_FEATURE_FUNC     (snffcn_pio_raw_cmd)
#define WINBOND_WRITE_FUNC           (snffcn_pio_write_data)
#define WINBOND_READ_FUNC            (snffcn_pio_read_data)
/* BBM releated */
#define WINBOND_BBM_FUNC			 (snffcn_bbm)
#define WINBOND_READ_BBM_TBL_FUNC	 (snffcn_read_bbm_tbl)

/***********************************************
  *  Winbond's spec definition
  ***********************************************/
 /* W25N01GV */
#define WINBOND_2KB_CHUNK_SPARE_SIZE    (0x840)
#define WINBOND_RDID_W25N01GV           (0xEFAA21)
#define WINBOND_MANUFACTURER_ID         (0xEF)
#define WINBOND_DEVICE_ID_W25N01GV      (0xAA21)

/* W25N02GV */
#define WINBOND_2KB_CHUNK_SPARE_SIZE    (0x840)
#define WINBOND_RDID_W25N02GV           (0xEFAB21)
#define WINBOND_MANUFACTURER_ID         (0xEF)
#define WINBOND_DEVICE_ID_W25N02GV      (0xAB21)

/* ESMT F50L1G41A */
#define WINBOND_2KB_CHUNK_SPARE_SIZE    (0x840)
#define ESMT_RDID_F50L1G41A				(0xC8217F)
#define ESMT_MANUFACTURER_ID			(0xC8)
#define ESMT_DEVICE_ID_F50L1G41A      	(0x217F)


/* MXIC MX35LF1GE4AB */
#define WINBOND_2KB_CHUNK_SPARE_SIZE    (0x840)
#define MXIC_RDID_MX35LF1GE4AB			(0xC212)
#define MXIC_MANUFACTURER_ID			(0xC2)
#define MXIC_DEVICE_ID_MX35LF1GE4AB     (0x12)


/********************************************
  Export functions
  ********************************************/
//plr_nand_spi_info_t *probe_winbond_spi_nand_chip(void);
int winbond_block_erase(unsigned int blk_pge_addr);
void winbond_wait_spi_nand_ready(void);
void winbond_pio_read_data(void *ram_addr, unsigned int wr_bytes, unsigned int blk_pge_addr, unsigned int col_addr);
void winbond_pio_write_data(void *ram_addr, unsigned int wr_bytes, unsigned int blk_pge_addr, unsigned int col_addr);
void winbond_dma_read_data(void *dma_addr, unsigned int dma_len, unsigned int blk_pge_addr); //Less or More than 2112 Bytes
void winbond_dma_write_data(void *dma_addr, unsigned int dma_len, unsigned int blk_pge_addr); //Les  or More than 2112 Bytes
void winbond_2KB_chunk_read(void *dma_addr, unsigned int blk_pge_addr); //Only 2112 Bytes
void winbond_2KB_chunk_write(void *dma_addr, unsigned int blk_pge_addr); //Only 2112 Bytes
void winbond_2KB_chunk_write_with_ecc_encode(void * dma_addr,unsigned int blk_pge_addr, void *p_eccbuf);
int winbond_2KB_chunk_read_with_ecc_decode(void * dma_addr,unsigned int blk_pge_addr, void *p_eccbuf);

//#define printf(fmt,args...)	prom_printf(fmt ,##args)

#endif //#ifdef WINBOND_SPI_NAND_H
