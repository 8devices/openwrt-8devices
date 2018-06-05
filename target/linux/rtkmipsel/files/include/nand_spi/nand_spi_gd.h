#ifndef GD_SPI_NAND_H
#define GD_SPI_NAND_H

/***********************************************
  * GD's opcode
  ***********************************************/
#define GD_RESET_OP         (0xFF)
#define GD_SET_FEATURE_OP   (0x1F)
#define GD_BLOCK_ERASE_OP   (0xD8)
#define GD_RDID_OP          (0x9F)
#define GD_GET_FEATURE_OP   (0x0F)
#define GD_READ_FROM_CACHE         (0x03)
#define GD_READ_FROM_CACHE_X2      (0x3B)
#define GD_READ_FROM_CACHE_X4      (0x6B)
#define GD_READ_FROM_CACHE_DIO     (0xBB)
#define GD_READ_FROM_CACHE_QIO     (0xEB)
#define GD_PROGRAM_LOAD            (0x02)
#define GD_PROGRAM_LOAD_X4         (0x32)
#define GD_WRITE_OP    (gd_cmd_info.w_cmd)
#define GD_READ_OP     (gd_cmd_info.r_cmd)


/***********************************************
  *  GD's driver function
  ***********************************************/
#define GD_RESET_FUNC           (snffcn_pio_raw_cmd)
#define GD_SET_FEATURE_FUNC     (snffcn_pio_raw_cmd)  /*quad_enable(),  gd_wait_spi_nand_ready(), gd_wait_spi_nand_ready()*/
#define GD_BLOCK_ERASE_FUNC     (snffcn_pio_raw_cmd)  /*block_erase()*/
#define GD_RDID_FUNC            (snffcn_pio_raw_cmd)
#define GD_GET_FEATURE_FUNC     (snffcn_pio_raw_cmd)
#define GD_WRITE_FUNC           (snffcn_pio_write_data)
#define GD_READ_FUNC            (snffcn_pio_read_data)


/***********************************************
  *  GD's spec definition
  ***********************************************/
#define GD_2KB_CHUNK_SPARE_SIZE    (0x840)
#define GD_RDID_GD5F1GQ4UAYIG      (0xC8F1)
#define GD_MANUFACTURER_ID         (0xC8)
#define GD_DEVICE_ID_GD5F1GQ4UAYIG (0xF1)


/********************************************
  Export Functions
  ********************************************/
plr_nand_spi_info_t *probe_gd_spi_nand_chip(void);
int gd_block_erase(u32_t blk_pge_addr);
void gd_wait_spi_nand_ready(void);
void gd_pio_read_data(void *ram_addr, u32_t wr_bytes, u32_t blk_pge_addr, u32_t col_addr);
void gd_pio_write_data(void *ram_addr, u32_t wr_bytes, u32_t blk_pge_addr, u32_t col_addr);
void gd_dma_read_data(void *dma_addr, u32_t dma_len, u32_t blk_pge_addr); //Less or More than 2112 Bytes
void gd_dma_write_data(void *dma_addr, u32_t dma_len, u32_t blk_pge_addr); //Les  or More than 2112 Bytes
void gd_2KB_chunk_read(void *dma_addr, u32_t blk_pge_addr); //Only 2112 Bytes
void gd_2KB_chunk_write(void *dma_addr, u32_t blk_pge_addr); //Only 2112 Bytes
void gd_2KB_chunk_write_with_ecc_encode(void *dma_addr, u32_t blk_pge_addr, void *p_eccbuf);
int gd_2KB_chunk_read_with_ecc_decode(void * dma_addr,u32_t blk_pge_addr, void *p_eccbuf);

#endif //#ifndef GD_SPI_NAND_H
