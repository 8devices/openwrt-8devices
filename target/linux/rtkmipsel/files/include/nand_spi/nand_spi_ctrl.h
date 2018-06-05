#ifndef SPI_NAND_CTRL_H
#define SPI_NAND_CTRL_H


/***********************************************
  *  SPI NAND Flash Controller Regiter Address
  ***********************************************/


/*Choosing SPI-NAND I/O type*/
#if 1
#define NAND_SPI_USE_SIO //JSW:Verified OK
#endif

#if 0
#define NAND_SPI_USE_DIO //Addr & Data : x2   //JSW:Verified OK
//#define NAND_SPI_USE_DATA_IOx2  //Only Data : x2  //Supposed OK
#endif

#if 0
#define NAND_SPI_USE_QIO //Addr & Data : x4  //JSW:Verified OK
//#define NAND_SPI_USE_DATA_IOx4 //Only Data : x4 //Supposed OK
#endif



 #if 1  // JSW for 8196F MIPS24K
 #define MAX_BYTE_PER_DMA 2112 //JSW for 96F SPI_NAND
#define SNFCFR   (0xB801A400)
#define SNFCCR   (0xB801A404)
#define SNFWCMR  (0xB801A408)
#define SNFRCMR  (0xB801A40C)
#define SNFRDR   (0xB801A410)
#define SNFWDR   (0xB801A414)
#define SNFDTR   (0xB801A418)
#define SNFDRSAR (0xB801A41C)
#define SNFDIR   (0xB801A420)
#define SNFDLR   (0xB801A424)
#define SNFCDSR  (0xB801A428)
#define SNFSR    (0xB801A440)
#else // for 8198E ARM
#define SNFCFR   (0xf801A400)
#define SNFCCR   (0xf801A404)
#define SNFWCMR  (0xf801A408)
#define SNFRCMR  (0xf801A40C)
#define SNFRDR   (0xf801A410)
#define SNFWDR   (0xf801A414)
#define SNFDTR   (0xf801A418)
#define SNFDRSAR (0xf801A41C)
#define SNFDIR   (0xf801A420)
#define SNFDLR   (0xf801A424)
#define SNFCDSR  (0xf801A428)
#define SNFSR    (0xf801A440)

#if 0
typedef unsigned   char    u8_t;
typedef signed     char    s8_t;
typedef unsigned   short   u16_t;
typedef signed     short   s16_t;
typedef unsigned   long    u32_t;
typedef signed     long    s32_t;
//typedef u32_t mem_ptr_t;
typedef int sys_prot_t;
#endif

#endif


/***********************************************
  *  SNF Controller IO_WIDTH / Length / Status / Command
  ***********************************************/
#define	SIO_WIDTH 0
#define	DIO_WIDTH 1
#define	QIO_WIDTH 2
#define SNFCMR_LEN(val) ((val-1))
#define IO_WIDTH_LEN(io_width, w_len) (unsigned int)((io_width<<28)|w_len)
#define WAIT_SPI_NAND_CTRLR_RDY() while(((REG32(SNFSR)>>3)&0x1))
#define WAIT_DMA_CTRLR_RDY() while((REG32(SNFSR)&0x6)!=0)
#define DMA_READ_DIR  (0)
#define DMA_WRITE_DIR (1)


#if 1 //JSW
typedef void  (nand_spi_wait_spi_nand_rdy_t)(void);
#endif

#if 1 //JSW
typedef struct{
    unsigned int w_cmd;
    unsigned int w_addr_io;
    unsigned int w_data_io;
    unsigned int r_cmd;
    unsigned int r_addr_io;
    unsigned int r_data_io;
    nand_spi_wait_spi_nand_rdy_t *_wait_spi_nand_ready;
}cmd_info_t;
#endif


/***********************************************
  *  SPI NAND Flash Controller Drivers & Function Pointer
  ***********************************************/
#define PIO_RAW_CMD(cmd, data, w_io_len, r_io_len) \
    __spi_nand_pio_cmd((cmd##_OP << 24), cmd##_FUNC, data, w_io_len, r_io_len, 0, 0)



#define PIO_WRITE_READ_DATA(cmd, col_addr, addr_io, data_io, wr_bytes, wr_buf) \
    __spi_nand_pio_cmd((cmd##_OP << 24), cmd##_FUNC, col_addr, addr_io, data_io, wr_bytes, wr_buf)

#if 1 //JSW
typedef unsigned int snffcn_ptr(unsigned int opcode, unsigned int data, unsigned int w_io_len, unsigned int r_io_len, unsigned int wr_bytes, void *wr_buf);
unsigned int __spi_nand_pio_cmd(unsigned int opcode, snffcn_ptr sffunc, unsigned int data, unsigned int w_io_len, unsigned int r_io_len, unsigned int wr_bytes, void *wr_buf);
snffcn_ptr snffcn_pio_raw_cmd, snffcn_pio_write_data, snffcn_pio_read_data,snffcn_bbm,snffcn_read_bbm_tbl;
#endif
#define SNF_DONT_CARE (0xFFFFFFFF)


/***********************************************
  *  Common Commands for GD and Winbond
  ***********************************************/
#define SNF_WRITE_ENABLE_OP    (0x06)
#define SNF_WRITE_DISABLE_OP   (0x04)
#define SNF_PROGRAM_EXECUTE_OP (0x10)
#define SNF_PAGE_DATA_READ_OP  (0x13)

#define SNF_WRITE_ENABLE_FUNC    (snffcn_pio_raw_cmd)
#define SNF_WRITE_DISABLE_FUNC   (snffcn_pio_raw_cmd)
#define SNF_PROGRAM_EXECUTE_FUNC (snffcn_pio_raw_cmd)
#define SNF_PAGE_DATA_READ_FUNC  (snffcn_pio_raw_cmd)



/***********************************************
  *  Common Drivers for GD and Winbond
  ***********************************************/
void cs_low(void);
void cs_high(void);
void write_enable(void);
void program_execute(unsigned int blk_pge_addr, cmd_info_t *snf_cmd_info);
void page_data_read_to_cache_buf(unsigned int blk_pge_addr, cmd_info_t *snf_cmd_info);
void _pio_write(unsigned int w_data, unsigned int w_io_len);
void _dma_write_read(unsigned int dma_phy_addr, unsigned int dma_io_len, unsigned int wr_dir);
void dma_write_data(void *dma_addr, unsigned int dma_len, unsigned int blk_pge_addr, cmd_info_t *info);
void dma_read_data(void *dma_addr, unsigned int dma_len, unsigned int blk_pge_addr, cmd_info_t *info);
#endif //#ifndef SPI_NAND_CTRL_H
