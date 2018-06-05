#include <nand_spi/util.h>
#include <nand_spi/nand_spi_ctrl.h>
#include <nand_spi/ecc_ctrl.h> //JSW
#include <nand_spi/soc.h>
#include "rtknflash_wrapper.h"


basic_io_t _lplr_basic_io;//JSW


SECTION_NAND_SPI void
cs_low(void)
{
    REG32(SNFCCR)=0; //Active CS#
    while((REG32(SNFSR)&0x1)!=0); //Polling CS# status @REG32(SNFSR) until it is ¡§0¡¨
}

SECTION_NAND_SPI void
cs_high(void)
{
    REG32(SNFCCR)=1; //De-active CS#
    while((REG32(SNFSR)&0x1)!=1); //Polling CS# status @REG32(SNFSR) until it is ¡§1¡¨
}

SECTION_NAND_SPI unsigned int
__spi_nand_pio_cmd(unsigned int opcode, snffcn_ptr sffunc, unsigned int data, unsigned int w_io_len, unsigned int r_io_len, unsigned int wr_bytes, void *wr_buf)
{
    unsigned int ret=0x0;


    WAIT_SPI_NAND_CTRLR_RDY();
    cs_high(); /* deactivate CS */


    WAIT_SPI_NAND_CTRLR_RDY();
    cs_low();  /* activate CS */


    ret = sffunc(opcode, data, w_io_len, r_io_len, wr_bytes, wr_buf);


    WAIT_SPI_NAND_CTRLR_RDY();

    cs_high(); /* deactivate CS */


    WAIT_SPI_NAND_CTRLR_RDY();


    return ret;
}

SECTION_NAND_SPI unsigned int
snffcn_pio_raw_cmd(unsigned int opcode, unsigned int data, unsigned int w_io_len, unsigned int r_io_len, unsigned int wr_bytes, void *wr_buf)
{
    unsigned int rdval = 0;
    unsigned int w_data = opcode;

    if(data != SNF_DONT_CARE){
        unsigned int temp = (w_io_len&0x3);
        if(0 != temp)
        w_data = opcode | (data<<((3-temp)*8));
    }

    WAIT_SPI_NAND_CTRLR_RDY();
    REG32(SNFWCMR) = w_io_len;
    REG32(SNFWDR)= w_data; //Trigger PIO Write
    WAIT_SPI_NAND_CTRLR_RDY();

    if (r_io_len != SNF_DONT_CARE) {
        REG32(SNFRCMR) = r_io_len; //Trigger PIO read
        WAIT_SPI_NAND_CTRLR_RDY();
        rdval = REG32(SNFRDR); //Getting r_len-BYTE data @REG32(SNFRDR)
    }
    return rdval;
}

SECTION_NAND_SPI unsigned int
snffcn_pio_write_data(unsigned int opcode, unsigned int col_addr, unsigned int addr_io, unsigned int data_io, unsigned int wr_bytes, void *wr_buf)
{
    u8_t tmp[4];
    unsigned int w_data;
    unsigned int w_io_len;

    /* Command: SIO, 1-Byte */
    w_data = opcode;
    w_io_len = IO_WIDTH_LEN(SIO_WIDTH,SNFCMR_LEN(1));
    WAIT_SPI_NAND_CTRLR_RDY();
    REG32(SNFWCMR)= w_io_len;
    REG32(SNFWDR) = w_data; //Trigger PIO Write

    /* Address: addr_io_len, 2-Byte -Byte*/
    w_data = col_addr<<16;
    w_io_len = IO_WIDTH_LEN(addr_io,SNFCMR_LEN(2));
    WAIT_SPI_NAND_CTRLR_RDY();
    REG32(SNFWCMR)= w_io_len;
    REG32(SNFWDR) = w_data; //Trigger PIO Write

    /* Data: data_io_len, wr_len-Byte */
    w_io_len = IO_WIDTH_LEN(data_io,SNFCMR_LEN(1));
    while(wr_bytes > 0){
        WAIT_SPI_NAND_CTRLR_RDY();
        if(wr_bytes < 4){
            REG32(SNFWCMR) = w_io_len | SNFCMR_LEN(wr_bytes);
            wr_bytes = 0;
        }else{
            REG32(SNFWCMR) = w_io_len | SNFCMR_LEN(4);
            wr_bytes -= 4;
        }
        /* The last few bytes may be garbage but won't be written to flash, since its length is explicitly given */
        tmp[0] = *((u8_t *)wr_buf++);
        tmp[1] = *((u8_t *)wr_buf++);
        tmp[2] = *((u8_t *)wr_buf++);
        tmp[3] = *((u8_t *)wr_buf++);
        WAIT_SPI_NAND_CTRLR_RDY();
        REG32(SNFWDR) = *((unsigned int *)tmp);
    }
    return 0;
}

SECTION_NAND_SPI unsigned int
snffcn_pio_read_data(unsigned int opcode, unsigned int col_addr, unsigned int addr_io, unsigned int data_io, unsigned int wr_bytes, void *wr_buf)
{
    unsigned int w_data;
    unsigned int w_io_len;
    unsigned int r_io_len;
    unsigned int wr_bound;
    unsigned int i;
    unsigned int dummy=0x00;
    u8_t tmp[4];

    /* Command: SIO, 1-Byte */
    w_data = opcode;
    w_io_len = IO_WIDTH_LEN(SIO_WIDTH,SNFCMR_LEN(1));
    WAIT_SPI_NAND_CTRLR_RDY();
    REG32(SNFWCMR)= w_io_len;
    REG32(SNFWDR) = w_data; //Trigger PIO Write



	  /* Address: addr_io_len, 4-Byte */
	#ifdef NAND_SPI_USE_QIO
		/*(Step4) Send 4-Byte Address for QIO , Fast read op(0xeb)need 4T's addr + 4T's dummy*/
		w_data = (col_addr<<24)|(dummy<<16);
	        w_io_len = IO_WIDTH_LEN(addr_io,SNFCMR_LEN(4));

	#else  /*(Step4) Send 3-Byte Address for SIO/DIO */
		   w_data = (col_addr<<16)|(dummy<<8);
		   w_io_len = IO_WIDTH_LEN(addr_io,SNFCMR_LEN(3));
	#endif



    WAIT_SPI_NAND_CTRLR_RDY();
    REG32(SNFWCMR)= w_io_len;
    REG32(SNFWDR) = w_data; //Trigger PIO Write
    WAIT_SPI_NAND_CTRLR_RDY();

    /* Data: data_io_len, wr_len-Byte */
    r_io_len = IO_WIDTH_LEN(data_io,SNFCMR_LEN(1));
    wr_bound = (unsigned int)wr_buf + wr_bytes;

    if (wr_bytes >= 4) {
        while ((unsigned int)wr_buf < (wr_bound & 0xFFFFFFFC)) {
            REG32(SNFRCMR)= r_io_len|SNFCMR_LEN(4);
            WAIT_SPI_NAND_CTRLR_RDY();
            *((unsigned int *)tmp) = REG32(SNFRDR);
            *((u8_t *)wr_buf++) = tmp[0];
            *((u8_t *)wr_buf++) = tmp[1];
            *((u8_t *)wr_buf++) = tmp[2];
            *((u8_t *)wr_buf++) = tmp[3];
        }
    }
    for (i=0; i<(wr_bytes & 0x3); i++) {
        REG32(SNFRCMR)= r_io_len;
        WAIT_SPI_NAND_CTRLR_RDY();
        *((u8_t *)wr_buf++) = REG32(SNFRDR) >> 24;
    }
	return 0;
}

SECTION_NAND_SPI void
_pio_write(unsigned int w_data, unsigned int w_io_len)
/* 1. Setting PIO Control Register
  * 2. Trigger it to write
  */
{
    WAIT_SPI_NAND_CTRLR_RDY();
    REG32(SNFWCMR)= w_io_len;
    REG32(SNFWDR) = w_data; //Trigger PIO Write
    WAIT_SPI_NAND_CTRLR_RDY();
}

SECTION_NAND_SPI void
write_enable(void)
{
    unsigned int w_io_len = IO_WIDTH_LEN(SIO_WIDTH,SNFCMR_LEN(1));
    PIO_RAW_CMD(SNF_WRITE_ENABLE,SNF_DONT_CARE,w_io_len,SNF_DONT_CARE);
}

SECTION_NAND_SPI void
program_execute(unsigned int blk_pge_addr, cmd_info_t *snf_cmd_info)
{
    /* 1-BYTE CMD + 1-BYTE Dummy + 2-BYTE Address */
    unsigned int w_io_len = IO_WIDTH_LEN(SIO_WIDTH,SNFCMR_LEN(4));
    PIO_RAW_CMD(SNF_PROGRAM_EXECUTE, blk_pge_addr, w_io_len, SNF_DONT_CARE);
    snf_cmd_info->_wait_spi_nand_ready();
}

SECTION_NAND_SPI void
page_data_read_to_cache_buf(unsigned int blk_pge_addr, cmd_info_t *snf_cmd_info)
{
    /* 1-BYTE CMD + 1-BYTE Dummy + 2-BYTE Address */
    unsigned int w_io_len = IO_WIDTH_LEN(SIO_WIDTH,SNFCMR_LEN(4));
    PIO_RAW_CMD(SNF_PAGE_DATA_READ, blk_pge_addr, w_io_len, SNF_DONT_CARE); // SNF_PAGE_DATA_READ=0x13
    snf_cmd_info->_wait_spi_nand_ready();
}


SECTION_NAND_SPI void
_dma_write_read(unsigned int dma_phy_addr, unsigned int dma_io_len, unsigned int wr_dir)
/* 1. Setting DMA Engine
  * 2. Trigger it to write or read
  */
{
    WAIT_SPI_NAND_CTRLR_RDY();
    REG32(SNFDRSAR) = dma_phy_addr;
    //printf("\n\n\n_dma_write_read()001\n");

   //printf("\nREG32(SNFDRSAR,0x%x)dma_phy_addr=0x%x\n",SNFDRSAR,dma_phy_addr);


    REG32(SNFDLR) = dma_io_len;
    WAIT_SPI_NAND_CTRLR_RDY();
    REG32(SNFDTR) = wr_dir; //Trigger DMA write or read

     //  printf("\n_dma_write_read()002 ,dma_io_len=0x%x \n",dma_io_len);
     //  printf("\nREG32(SNFDTR,0x%x)=0x%x\n\n\n",SNFDTR,REG32(SNFDTR) );
    WAIT_DMA_CTRLR_RDY();
}


#if 0
SECTION_NAND_SPI void
dma_write_data(void *dma_addr, unsigned int dma_len, unsigned int blk_pge_addr, cmd_info_t *snf_cmd_info)
/*
  * Write More or Less than One Chunk (More or less than 2112-Byte)
  * Start from the initial cache register (CA=0x0000)
  */
{


	unsigned int w_data, w_io_len, dma_io_len;
    unsigned int column_addr=0x0000;



    while(dma_len > 0){
        /* (Step1) Enable WEL */
        write_enable();


        /*(Step2) Enable CS */
        WAIT_SPI_NAND_CTRLR_RDY();
        cs_high(); /* deactivate CS */
        WAIT_SPI_NAND_CTRLR_RDY();
        cs_low();  /* activate CS */


        /*(Step3) 1-Byte Command */
        w_data = snf_cmd_info->w_cmd<<24;
        w_io_len = IO_WIDTH_LEN(SIO_WIDTH,SNFCMR_LEN(1));
        _pio_write(w_data, w_io_len);



        /*(Step4) 2-Byte Address */
        w_data = column_addr<<16;
        w_io_len = IO_WIDTH_LEN(snf_cmd_info->w_addr_io,SNFCMR_LEN(2));
        _pio_write(w_data, w_io_len);



        /*(Step5) DMA Write to cache register from RAM */
        if(dma_len >= MAX_BYTE_PER_DMA){
		#if 1
		dma_io_len = IO_WIDTH_LEN(snf_cmd_info->w_data_io, MAX_BYTE_PER_DMA);
        _dma_write_read(PADDR(dma_addr), dma_io_len, DMA_WRITE_DIR);
        WAIT_SPI_NAND_CTRLR_RDY();
        cs_high(); /* deactivate CS */


        /*(Step6) Write data from cache register to spi nand flash */
        program_execute(blk_pge_addr, snf_cmd_info);
	#endif


		#if 0 //cause Reboot
            _lplr_basic_io.dcache_writeback_invalidate_range((unsigned int)dma_addr, (unsigned int)(dma_addr+MAX_BYTE_PER_DMA));
		 #else
			 dma_addr+=MAX_BYTE_PER_DMA;//JSW
		#endif


            dma_len -= MAX_BYTE_PER_DMA;
        }else{ //dma_men < 512

		#if 1
		  dma_io_len = IO_WIDTH_LEN(snf_cmd_info->w_data_io, dma_len);
        _dma_write_read(PADDR(dma_addr), dma_io_len, DMA_WRITE_DIR);
        WAIT_SPI_NAND_CTRLR_RDY();
        cs_high(); /* deactivate CS */


        /*(Step6) Write data from cache register to spi nand flash */
        program_execute(blk_pge_addr, snf_cmd_info);
	#endif

        #if 0 //cause Reboot
            _lplr_basic_io.dcache_writeback_invalidate_range((unsigned int)dma_addr, (unsigned int)(dma_addr+dma_len));
          #else
		  dma_addr+=dma_len;//JSW
	#endif



            dma_len = 0;
        }

	#if 0
        _dma_write_read(PADDR(dma_addr), dma_io_len, DMA_WRITE_DIR);
        WAIT_SPI_NAND_CTRLR_RDY();
        cs_high(); /* deactivate CS */


        /*(Step6) Write data from cache register to spi nand flash */
        program_execute(blk_pge_addr, snf_cmd_info);
	#endif
    }
}

#else

SECTION_NAND_SPI void
dma_write_data(void *dma_addr, u32_t dma_len, u32_t blk_pge_addr, cmd_info_t *snf_cmd_info)
/*
  * Write More or Less than One Chunk (More or less than 2112-Byte)
  * Start from the initial cache register (CA=0x0000)
  */
{
    u32_t w_data, w_io_len, dma_io_len;
    u32_t column_addr=0x0000;

    while(dma_len > 0){
        /* (Step1) Enable WEL */
        write_enable();

        /*(Step2) Enable CS */
        WAIT_SPI_NAND_CTRLR_RDY();
        cs_high(); /* deactivate CS */
        WAIT_SPI_NAND_CTRLR_RDY();
        cs_low();  /* activate CS */

        /*(Step3) 1-Byte Command */
        w_data = snf_cmd_info->w_cmd<<24;
        w_io_len = IO_WIDTH_LEN(SIO_WIDTH,SNFCMR_LEN(1));
        _pio_write(w_data, w_io_len);

        /*(Step4) 2-Byte Address */
        w_data = column_addr<<16;
        w_io_len = IO_WIDTH_LEN(snf_cmd_info->w_addr_io,SNFCMR_LEN(2));
        _pio_write(w_data, w_io_len);


        /*(Step5) DMA Write to cache register from RAM */
        if(dma_len >= MAX_BYTE_PER_DMA){
           // _lplr_basic_io.dcache_writeback_invalidate_range((u32_t)dma_addr, (u32_t)(dma_addr+MAX_BYTE_PER_DMA));
            dma_io_len = IO_WIDTH_LEN(snf_cmd_info->w_data_io, MAX_BYTE_PER_DMA);
            dma_len -= MAX_BYTE_PER_DMA;
        }else{
           // _lplr_basic_io.dcache_writeback_invalidate_range((u32_t)dma_addr, (u32_t)(dma_addr+dma_len));
            dma_io_len = IO_WIDTH_LEN(snf_cmd_info->w_data_io, dma_len);
            dma_len = 0;
        }
        _dma_write_read(PADDR(dma_addr), dma_io_len, DMA_WRITE_DIR);
        WAIT_SPI_NAND_CTRLR_RDY();
        cs_high(); /* deactivate CS */

        /*(Step6) Write data from cache register to spi nand flash */
        program_execute(blk_pge_addr, snf_cmd_info);
    }
}

#endif

#if 0

SECTION_NAND_SPI void
dma_read_data(void *dma_addr, unsigned int dma_len, unsigned int blk_pge_addr, cmd_info_t *snf_cmd_info)
/*
  * Read More or Less than One Chunk (More or less than 2112-Byte)
  * Start from the initial cache register (CA=0x0000)
  */
{


    unsigned int w_data, w_io_len, dma_io_len;
    unsigned int column_addr=0x0000;
    unsigned int dummy=0x00;

    while(dma_len > 0){
        /*(Step1) Read data from nand flash to cache register */
        page_data_read_to_cache_buf(blk_pge_addr, snf_cmd_info);


        /*(Step2) Enable CS */
        WAIT_SPI_NAND_CTRLR_RDY();
        cs_high(); /* deactivate CS */
        WAIT_SPI_NAND_CTRLR_RDY();
        cs_low();  /* activate CS */


        /*(Step3) Send 1-Byte Command */
        w_data = snf_cmd_info->r_cmd<<24;
        w_io_len = IO_WIDTH_LEN(SIO_WIDTH,SNFCMR_LEN(1));
        _pio_write(w_data, w_io_len);


        /*(Step4) Send 3-Byte Address */
        w_data = (column_addr<<16)|(dummy<<8);
        w_io_len = IO_WIDTH_LEN(snf_cmd_info->r_addr_io,SNFCMR_LEN(3));
        _pio_write(w_data, w_io_len);


        /*(Step5) DMA Read from cache register to RAM */
        if(dma_len >= MAX_BYTE_PER_DMA){

			#if 1
			  dma_io_len = IO_WIDTH_LEN(snf_cmd_info->r_data_io, MAX_BYTE_PER_DMA);
		  printf("\ndma_io_len=0x%x\n",dma_io_len);
	    printf("\n*dma_addr=0x%x\n",(void*)dma_addr);
        _dma_write_read(PADDR(dma_addr), dma_io_len, DMA_READ_DIR);

        WAIT_SPI_NAND_CTRLR_RDY();
        cs_high(); /* deactivate CS */

			#endif
	   #if 0
            _lplr_basic_io.dcache_writeback_invalidate_range((unsigned int)dma_addr, (unsigned int)(dma_addr+MAX_BYTE_PER_DMA));
            _lplr_basic_io.dcache_writeback_invalidate_all();
	   #else
		dma_addr+=MAX_BYTE_PER_DMA;
	   #endif

            dma_len -= MAX_BYTE_PER_DMA;
        }else{

		#if 1
		  dma_io_len = IO_WIDTH_LEN(snf_cmd_info->r_data_io, dma_len);
		//  printf("\ndma_io_len=0x%x\n",dma_io_len);
	  //  printf("\n*dma_addr=0x%x\n",(void*)dma_addr);
        _dma_write_read(PADDR(dma_addr), dma_io_len, DMA_READ_DIR);

        WAIT_SPI_NAND_CTRLR_RDY();
        cs_high(); /* deactivate CS */

			#endif

          #if 0
            _lplr_basic_io.dcache_writeback_invalidate_range((unsigned int)dma_addr, (unsigned int)(dma_addr+dma_len));
	   #else
		dma_addr+=dma_len;
	   #endif


            dma_len = 0;
        }


    }
}

#else
SECTION_NAND_SPI void
dma_read_data(void *dma_addr, u32_t dma_len, u32_t blk_pge_addr, cmd_info_t *snf_cmd_info)
/*
  * Read More or Less than One Chunk (More or less than 2112-Byte)
  * Start from the initial cache register (CA=0x0000)
  */
{
    u32_t w_data, w_io_len, dma_io_len;
    u32_t column_addr=0x0000;
    u32_t dummy=0x00;




    while(dma_len > 0){
        /*(Step1) Read data from nand flash to cache register */
        page_data_read_to_cache_buf(blk_pge_addr, snf_cmd_info);

        /*(Step2) Enable CS */
        WAIT_SPI_NAND_CTRLR_RDY();
        cs_high(); /* deactivate CS */
        WAIT_SPI_NAND_CTRLR_RDY();
        cs_low();  /* activate CS */

        /*(Step3) Send 1-Byte Command */
        w_data = snf_cmd_info->r_cmd<<24; //r_cmd= 0x03
        w_io_len = IO_WIDTH_LEN(SIO_WIDTH,SNFCMR_LEN(1));
        _pio_write(w_data, w_io_len);





	#ifdef NAND_SPI_USE_QIO
		/*(Step4) Send 4-Byte Address for QIO , Fast read op(0xeb)need 4T's addr + 4T's dummy*/
		w_data = (column_addr<<24)|(dummy<<16);
		w_io_len = IO_WIDTH_LEN(snf_cmd_info->r_addr_io,SNFCMR_LEN(4));
		_pio_write(w_data, w_io_len);

	#else  /*(Step4) Send 3-Byte Address for SIO/DIO */

		w_data = (column_addr<<16)|(dummy<<8);
		w_io_len = IO_WIDTH_LEN(snf_cmd_info->r_addr_io,SNFCMR_LEN(3));
		_pio_write(w_data, w_io_len);
	#endif

        /*(Step5) DMA Read from cache register to RAM */
        if(dma_len >= MAX_BYTE_PER_DMA){
            //_lplr_basic_io.dcache_writeback_invalidate_range((u32_t)dma_addr, (u32_t)(dma_addr+MAX_BYTE_PER_DMA));
            dma_io_len = IO_WIDTH_LEN(snf_cmd_info->r_data_io, MAX_BYTE_PER_DMA);
            dma_len -= MAX_BYTE_PER_DMA;
        }else{
           // _lplr_basic_io.dcache_writeback_invalidate_range((u32_t)dma_addr, (u32_t)(dma_addr+dma_len));
            dma_io_len = IO_WIDTH_LEN(snf_cmd_info->r_data_io, dma_len);
            dma_len = 0;
        }
        _dma_write_read(PADDR(dma_addr), dma_io_len, DMA_READ_DIR);

	//dma_addr+=MAX_BYTE_PER_DMA;//JSW

        WAIT_SPI_NAND_CTRLR_RDY();
        cs_high(); /* deactivate CS */
    }
}

#endif


SECTION_NAND_SPI unsigned int
snffcn_bbm(unsigned int opcode, unsigned int data, unsigned int w_io_len, unsigned int r_io_len, unsigned int wr_bytes, void *wr_buf)
{
    unsigned int w_data;
    unsigned int w_io_len2;

    /* Command: SIO, 1-Byte */
    w_data = opcode;
    w_io_len2 = IO_WIDTH_LEN(SIO_WIDTH,SNFCMR_LEN(1));
    WAIT_SPI_NAND_CTRLR_RDY();
    REG32(SNFWCMR)= w_io_len2;
    REG32(SNFWDR) = w_data; //Trigger PIO Write

    /* Address: addr_io_len, 2-Byte -Byte*/
    w_data = data;
    w_io_len2 = IO_WIDTH_LEN(SIO_WIDTH,SNFCMR_LEN(4));
    WAIT_SPI_NAND_CTRLR_RDY();
    REG32(SNFWCMR)= w_io_len2;
    REG32(SNFWDR) = w_data; //Trigger PIO Write

    return 0;
}


SECTION_NAND_SPI unsigned int
snffcn_read_bbm_tbl(unsigned int opcode, unsigned int data, unsigned int addr_io, unsigned int data_io, unsigned int wr_bytes, void *wr_buf)
{
    unsigned int w_data = 0;
    unsigned int w_io_len = 0;
    unsigned int r_io_len;
    //unsigned int wr_bound;
    unsigned int i = 0;
    //unsigned int dummy=0x00;
    u8_t tmp[4];

    /* Command: SIO, 1-Byte */

	if(data != SNF_DONT_CARE){
        unsigned int temp = (w_io_len&0x3);
        if(0 != temp)
        w_data = opcode | (data<<((3-temp)*8));
    }
    w_io_len = IO_WIDTH_LEN(SIO_WIDTH,SNFCMR_LEN(2));
    WAIT_SPI_NAND_CTRLR_RDY();
    REG32(SNFWCMR)= w_io_len;
    REG32(SNFWDR) = w_data; //Trigger PIO Write


	r_io_len = IO_WIDTH_LEN(SIO_WIDTH,SNFCMR_LEN(4));

	while(i < wr_bytes){
		REG32(SNFRCMR) = r_io_len;
		WAIT_SPI_NAND_CTRLR_RDY();

		*((unsigned int *)tmp) = REG32(SNFRDR);
        *((u8_t *)wr_buf++) = tmp[0];
        *((u8_t *)wr_buf++) = tmp[1];
        *((u8_t *)wr_buf++) = tmp[2];
        *((u8_t *)wr_buf++) = tmp[3];

	i++;
	}

	return 0;
}
