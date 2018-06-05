
// #include <util.h>

#include <nand_spi/ecc_ctrl.h>
#include <nand_spi/nand_spi_ctrl.h>
#include <nand_spi/nand_spi.h>
#include <nand_spi/util.h>
#ifdef __RTK_BOOT__
#include <malloc.h>
#endif
#ifdef __UBOOT__
#include <common.h>
#include <malloc.h>
#endif
#include <asm/io.h>
#include "rtknflash_wrapper.h"

//#define VZERO 0

#define SECTION_RO

#define CONFIG_AVOID_ERROR_INPUT_HANG_SPI_NAND 1

plr_nand_spi_info_t *probe_winbond_spi_nand_chip(void
);


/*Choosing SPI-NAND I/O type
    Definition in nand_spi_ctrl.h

*/

#ifdef NAND_SPI_USE_SIO
SECTION_RO cmd_info_t winbond_cmd_info = {
    .w_cmd = WINBOND_PROGRAM_LOAD,
    .w_addr_io = SIO_WIDTH,
    .w_data_io = SIO_WIDTH,
    .r_cmd = WINBOND_NORMAL_READ,
    .r_addr_io =        SIO_WIDTH,
    .r_data_io = SIO_WIDTH,
    ._wait_spi_nand_ready = winbond_wait_spi_nand_ready,
};
#endif


#ifdef NAND_SPI_USE_DIO
SECTION_RO cmd_info_t winbond_cmd_info = {
    .w_cmd = WINBOND_PROGRAM_LOAD, //Wonbond's program_load only x1 & x4
    .w_addr_io = SIO_WIDTH,
    .w_data_io = SIO_WIDTH,
    .r_cmd = WINBOND_FAST_READ_DIO,
    .r_addr_io =        DIO_WIDTH,
    .r_data_io = DIO_WIDTH,
    ._wait_spi_nand_ready = winbond_wait_spi_nand_ready,
};
#endif

#ifdef NAND_SPI_USE_DATA_IOx2
SECTION_RO cmd_info_t winbond_cmd_info = {
    .w_cmd = WINBOND_PROGRAM_LOAD,
    .w_addr_io = SIO_WIDTH,
    .w_data_io = SIO_WIDTH,
    .r_cmd = WINBOND_FAST_READ_X2,
    .r_addr_io =        SIO_WIDTH,
    .r_data_io = DIO_WIDTH,
    ._wait_spi_nand_ready = winbond_wait_spi_nand_ready,
};
#endif



#ifdef NAND_SPI_USE_QIO
SECTION_RO cmd_info_t winbond_cmd_info = {
    .w_cmd = WINBOND_PROGRAM_LOAD_X4,
    .w_addr_io = SIO_WIDTH, //address can't be quad
    .w_data_io = QIO_WIDTH,
    .r_cmd = WINBOND_FAST_READ_QIO,
    .r_addr_io =        QIO_WIDTH,
    .r_data_io = QIO_WIDTH,
    ._wait_spi_nand_ready = winbond_wait_spi_nand_ready,
};
#endif

#ifdef NAND_SPI_USE_DATA_IOx4
SECTION_RO cmd_info_t winbond_cmd_info = {
    .w_cmd = WINBOND_PROGRAM_LOAD_X4,
    .w_addr_io = SIO_WIDTH,
    .w_data_io = QIO_WIDTH,
    .r_cmd = WINBOND_FAST_READ_X4,
    .r_addr_io =        SIO_WIDTH,
    .r_data_io = QIO_WIDTH,
    ._wait_spi_nand_ready = winbond_wait_spi_nand_ready,
};
#endif

//MXIC
SECTION_RO plr_nand_spi_info_t MXIC_MX35LF1GE4AB_info = {
    .man_id = MXIC_MANUFACTURER_ID,.dev_id =
        MXIC_DEVICE_ID_MX35LF1GE4AB,.num_block = 1024,.num_chunk_per_block =
        64,.chunk_size = 2048,._probe =
        probe_winbond_spi_nand_chip,._erase_block =
        winbond_block_erase,._pio_write =
        winbond_pio_write_data,._pio_read =
        winbond_pio_read_data,._chunk_read =
        winbond_2KB_chunk_read,._chunk_write =
        winbond_2KB_chunk_write,._dma_read =
        winbond_dma_read_data,._dma_write =
        winbond_dma_write_data,._chunk_read_ecc =
        winbond_2KB_chunk_read_with_ecc_decode,._chunk_write_ecc =
        winbond_2KB_chunk_write_with_ecc_encode,._ecc_encode =
        ecc_encode_bch_6t,._ecc_decode =
        ecc_decode_bch_6t,._chk_ecc_error_sts = ecc_check_decode_status
};


//esmt
SECTION_RO plr_nand_spi_info_t ESMT_F50L1G41A_info = {
    .man_id = ESMT_MANUFACTURER_ID,.dev_id =
        ESMT_DEVICE_ID_F50L1G41A,.num_block = 1024,.num_chunk_per_block =
        64,.chunk_size = 2048,._probe =
        probe_winbond_spi_nand_chip,._erase_block =
        winbond_block_erase,._pio_write =
        winbond_pio_write_data,._pio_read =
        winbond_pio_read_data,._chunk_read =
        winbond_2KB_chunk_read,._chunk_write =
        winbond_2KB_chunk_write,._dma_read =
        winbond_dma_read_data,._dma_write =
        winbond_dma_write_data,._chunk_read_ecc =
        winbond_2KB_chunk_read_with_ecc_decode,._chunk_write_ecc =
        winbond_2KB_chunk_write_with_ecc_encode,._ecc_encode =
        ecc_encode_bch_6t,._ecc_decode =
        ecc_decode_bch_6t,._chk_ecc_error_sts = ecc_check_decode_status
};

//winbond
SECTION_RO plr_nand_spi_info_t winbond_W25N01GV_info = {
    .man_id = WINBOND_MANUFACTURER_ID,.dev_id =
        WINBOND_DEVICE_ID_W25N01GV,.num_block = 1024,.num_chunk_per_block =
        64,.chunk_size = 2048,._probe =
        probe_winbond_spi_nand_chip,._erase_block =
        winbond_block_erase,._pio_write =
        winbond_pio_write_data,._pio_read =
        winbond_pio_read_data,._chunk_read =
        winbond_2KB_chunk_read,._chunk_write =
        winbond_2KB_chunk_write,._dma_read =
        winbond_dma_read_data,._dma_write =
        winbond_dma_write_data,._chunk_read_ecc =
        winbond_2KB_chunk_read_with_ecc_decode,._chunk_write_ecc =
        winbond_2KB_chunk_write_with_ecc_encode,._ecc_encode =
        ecc_encode_bch_6t,._ecc_decode =
        ecc_decode_bch_6t,._chk_ecc_error_sts = ecc_check_decode_status
};


SECTION_RO plr_nand_spi_info_t winbond_W25N02GV_info = {
    .man_id = WINBOND_MANUFACTURER_ID,.dev_id =
        WINBOND_DEVICE_ID_W25N02GV,.num_block = 1024,.num_chunk_per_block =
        64,.chunk_size = 2048,._probe =
        probe_winbond_spi_nand_chip,._erase_block =
        winbond_block_erase,._pio_write =
        winbond_pio_write_data,._pio_read =
        winbond_pio_read_data,._chunk_read =
        winbond_2KB_chunk_read,._chunk_write =
        winbond_2KB_chunk_write,._dma_read =
        winbond_dma_read_data,._dma_write =
        winbond_dma_write_data,._chunk_read_ecc =
        winbond_2KB_chunk_read_with_ecc_decode,._chunk_write_ecc =
        winbond_2KB_chunk_write_with_ecc_encode,._ecc_encode =
        ecc_encode_bch_6t,._ecc_decode =
        ecc_decode_bch_6t,._chk_ecc_error_sts = ecc_check_decode_status
};


SECTION_NAND_SPI unsigned int
winbond_get_feature_register(unsigned int feature_addr
)
{
    unsigned int    w_io_len = IO_WIDTH_LEN(SIO_WIDTH, SNFCMR_LEN(2));
    unsigned int    r_io_len = IO_WIDTH_LEN(SIO_WIDTH, SNFCMR_LEN(1));
    unsigned int    ret =
        PIO_RAW_CMD(WINBOND_GET_FEATURE, feature_addr, w_io_len, r_io_len);
    return ((ret >> 24) & 0xFF);
}

SECTION_NAND_SPI void
winbond_set_feature_register(unsigned int feature_addr, unsigned int setting
)
{
    unsigned int    w_io_len = IO_WIDTH_LEN(SIO_WIDTH, SNFCMR_LEN(3));
    PIO_RAW_CMD(WINBOND_SET_FEATURE, ((feature_addr << 8) | setting),
                w_io_len, SNF_DONT_CARE);
}

#ifdef NAND_SPI_USE_QIO
SECTION_NAND_SPI void
winbond_quad_enable(void
)
{
    unsigned int    feature_addr = 0xA0;
    unsigned int    value = 0xFD;
    value &= winbond_get_feature_register(feature_addr);
    winbond_set_feature_register(feature_addr, value);
}
#endif

SECTION_NAND_SPI void
winbond_block_unprotect(void
)
{
    unsigned int    feature_addr = 0xA0;
    unsigned int    value = 0x00;
    value &= winbond_get_feature_register(feature_addr);
    winbond_set_feature_register(feature_addr, value);
}

SECTION_NAND_SPI void
winbond_set_buffer_mode(void
)
{
    unsigned int    feature_addr = 0xB0;
    unsigned int    value = winbond_get_feature_register(feature_addr);
    value |= (1 << 3);
    winbond_set_feature_register(feature_addr, value);
}

SECTION_NAND_SPI void
winbond_disable_on_die_ecc(void
)
{
    unsigned int    feature_addr = 0xB0;
    unsigned int    value = winbond_get_feature_register(feature_addr);
    value &= ~(1 << 4);
    winbond_set_feature_register(feature_addr, value);
}


SECTION_NAND_SPI void
winbond_wait_spi_nand_ready(void
)
{
    unsigned int    feature_addr = 0xC0;
    unsigned int    oip = winbond_get_feature_register(feature_addr);

    while ((oip & 0x1) != 0) {
        oip = winbond_get_feature_register(feature_addr);
    }
}

SECTION_NAND_SPI int
winbond_block_erase(unsigned int blk_pge_addr
)
{

	unsigned int    w_io_len;
	unsigned int    efail;
    write_enable();

    /*
     * 1-BYTE CMD + 1-BYTE Dummy + 2-BYTE Address
     */
    w_io_len = IO_WIDTH_LEN(SIO_WIDTH, SNFCMR_LEN(4));
    PIO_RAW_CMD(WINBOND_BLOCK_ERASE, blk_pge_addr, w_io_len,
                SNF_DONT_CARE);
    winbond_wait_spi_nand_ready();

    /*
     * Check Erase Status
     */
    efail =
        ((winbond_get_feature_register(0xC0) >> 2) & 0x1);
    if (1 == efail) {
        printf("  <<ERROR>> Block erase Fail: blk_pge_addr=0x%x\n",
               blk_pge_addr);
        return -1;
    }
    return 0;
}


#if CONF_SPI_NAND_UBOOT_COMMAND
static
winbond_block_erase_uboot(cmd_tbl_t * cmdtp, int flag, int argc,
                          char *argv[]
)
{

	#if CONFIG_AVOID_ERROR_INPUT_HANG_SPI_NAND
    if ((argv[1])) {
        /*
         * Avoid error input format hang
         */
    } else {
        printf("Wrong argument number!\r\n");
        printf
            ("Usage:snwbbe <blk_pge_addr>  : SPI-NAND Winbond block erasein hex\r\n");
        return CMD_RET_FAILURE;
    }
#endif


    unsigned int    blk_pge_addr =
        Strtoul((const u8 *) (argv[1]), (u8 **) NULL, 16);

    winbond_block_erase(blk_pge_addr);
    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(snwbbe, 2, 0, winbond_block_erase_uboot,
           "snwbbe <blk_pge_addr>  : SPI-NAND Winbond block erase \n", "");
#endif



SECTION_NAND_SPI void
winbond_pio_read_data(void *ram_addr, unsigned int wr_bytes,
                      unsigned int blk_pge_addr, unsigned int col_addr
)
/*
 * PIO Read Less than One Chunk (Less than 2112-Byte)
 * Start from the assigned cache register address (CA=col_addr)
 */
{

#if 1
    if (wr_bytes > 2112) {
        printf
            ("\nFail => PIO Read Less than One Chunk (Less than 2112 (0x840)-Byte) \n");
        return;
    }
#endif

    page_data_read_to_cache_buf(blk_pge_addr, &winbond_cmd_info);
    PIO_WRITE_READ_DATA(WINBOND_READ, col_addr, winbond_cmd_info.r_addr_io,
                        winbond_cmd_info.r_data_io, wr_bytes, ram_addr);
}

#if CONF_SPI_NAND_UBOOT_COMMAND
static
winbond_pio_read_data_uboot(cmd_tbl_t * cmdtp, int flag, int argc,
                            char *argv[]
)
{

#if CONFIG_AVOID_ERROR_INPUT_HANG_SPI_NAND
   if ((argv[1]) && (argv[2]) && (argv[3]) && (argv[4])) {
        /*
         * Avoid error input format hang
         */
    } else {
        printf("Wrong argument number!\r\n");
        printf
            ("Usage:snwbpior <ram_addr><wr_bytes><blk_pge_addr><col_addr>  : SPI-NAND Winbond pio read in hex\r\n");
        return CMD_RET_FAILURE;
    }
#endif

#if 1

    void           *ram_addr =
        Strtoul((const u8 *) (argv[1]), (u8 **) NULL, 16);
    unsigned int    wr_bytes =
        Strtoul((const u8 *) (argv[2]), (u8 **) NULL, 16);
    unsigned int    blk_pge_addr =
        Strtoul((const u8 *) (argv[3]), (u8 **) NULL, 16);
    unsigned int    col_addr =
        Strtoul((const u8 *) (argv[4]), (u8 **) NULL, 16);


#endif

    winbond_pio_read_data(ram_addr, wr_bytes, blk_pge_addr, col_addr);
    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(snwbpior, 5, 0, winbond_pio_read_data_uboot,
           "snwbpior <ram_addr><wr_bytes><blk_pge_addr><col_addr>  : SPI-NAND Winbond pio read \n",
           "");
#endif


SECTION_NAND_SPI void
winbond_pio_write_data(void *ram_addr, unsigned int wr_bytes,
                       unsigned int blk_pge_addr, unsigned int col_addr
)
/*
 * PIO Write Less than One Chunk (Less than 2112-Byte)
 * Start from the assigned cache register address (CA=col_addr)
 */
{
#if 1
    if (wr_bytes > 2112) {
        printf
            ("\nFail => PIO Write Less than One Chunk  (Less than 2112 (0x840)-Byte) \n");
        return;
    }
#endif
    write_enable();
    PIO_WRITE_READ_DATA(WINBOND_WRITE, col_addr,
                        winbond_cmd_info.w_addr_io,
                        winbond_cmd_info.w_data_io, wr_bytes, ram_addr);
    program_execute(blk_pge_addr, &winbond_cmd_info);
}


#if CONF_SPI_NAND_UBOOT_COMMAND
static
winbond_pio_write_data_uboot(cmd_tbl_t * cmdtp, int flag, int argc,
                             char *argv[]
)
{
#if CONFIG_AVOID_ERROR_INPUT_HANG_SPI_NAND
   if ((argv[1]) && (argv[2]) && (argv[3]) && (argv[4])) {
        /*
         * Avoid error input format hang
         */
    } else {
        printf("Wrong argument number!\r\n");
        printf
            ("Usage:snwbpiow <ram_addr><wr_bytes><blk_pge_addr><col_addr>  : SPI-NAND Winbond pio write in hex\r\n");
        return CMD_RET_FAILURE;
    }
#endif


#if 1

    void           *ram_addr =
        Strtoul((const u8 *) (argv[1]), (u8 **) NULL, 16);
    unsigned int    wr_bytes =
        Strtoul((const u8 *) (argv[2]), (u8 **) NULL, 16);
    unsigned int    blk_pge_addr =
        Strtoul((const u8 *) (argv[3]), (u8 **) NULL, 16);
    unsigned int    col_addr =
        Strtoul((const u8 *) (argv[4]), (u8 **) NULL, 16);


#endif

    winbond_pio_write_data(ram_addr, wr_bytes, blk_pge_addr, col_addr);
    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(snwbpiow, 5, 0, winbond_pio_write_data_uboot,
           "snwbpiow <ram_addr><wr_bytes><blk_pge_addr><col_addr>  : SPI-NAND Winbond pio write \n",
           "");
#endif


SECTION_NAND_SPI void
winbond_dma_read_data(void *dma_addr, unsigned int dma_len, unsigned int blk_pge_addr
)
/*
 * Read More or Less than One Chunk (More or less than 2112-Byte)
 * Start from the initial cache register (CA=0x0000)
 */
{
	/* cache invalid and writeback */
	dma_cache_wback_inv((unsigned long)dma_addr,(unsigned long)dma_len);

    dma_read_data(dma_addr, dma_len, blk_pge_addr, &winbond_cmd_info);
}


#if CONF_SPI_NAND_UBOOT_COMMAND
static int
winbond_dma_read_data_uboot(cmd_tbl_t * cmdtp, int flag, int argc,
                            char *argv[]
)
{

#if CONFIG_AVOID_ERROR_INPUT_HANG_SPI_NAND
   if ((argv[1]) && (argv[2]) && (argv[3]) ) {
        /*
         * Avoid error input format hang
         */
    } else {
        printf("Wrong argument number!\r\n");
        printf
            ("Usage:snwbdmar <dma_addr><dma_len><blk_pge_addr>  : SPI-NAND Winbond DMA read in hex\r\n");
        return CMD_RET_FAILURE;
    }
#endif


#if 1
    void           *dma_addr =
        Strtoul((const u8 *) (argv[1]), (u8 **) NULL, 16);
    unsigned int    dma_len =
        Strtoul((const u8 *) (argv[2]), (u8 **) NULL, 16);
    unsigned int    blk_pge_addr =
        Strtoul((const u8 *) (argv[3]), (u8 **) NULL, 16);
#endif


#if 1
    winbond_dma_read_data(dma_addr, dma_len, blk_pge_addr);
#else
    if (i = 0; i <=; i++) {
        dma_addr +=;
        winbond_dma_read_data(dma_addr, dma_len, blk_pge_addr);
    }

#endif


    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(snwbdmar, 4, 0, winbond_dma_read_data_uboot,
           "snwbdmar <dma_addr><dma_len><blk_pge_addr>  : SPI-NAND Winbond DMA read \n",
           "");
#endif




SECTION_NAND_SPI void
winbond_dma_write_data(void *dma_addr, unsigned int dma_len, unsigned int blk_pge_addr
)
/*
 * Write More or Less than One Chunk (More or less than 2112-Byte)
 * Start from the initial cache register (CA=0x0000)
 */
{
	/* flush cache */
	dma_cache_wback_inv((unsigned long)dma_addr,(unsigned long)dma_len);

    dma_write_data(dma_addr, dma_len, blk_pge_addr, &winbond_cmd_info);
}



#if CONF_SPI_NAND_UBOOT_COMMAND
static int
winbond_dma_write_data_uboot(cmd_tbl_t * cmdtp, int flag, int argc,
                             char *argv[]
)
{

#if CONFIG_AVOID_ERROR_INPUT_HANG_SPI_NAND
   if ((argv[1]) && (argv[2]) && (argv[3]) ) {
        /*
         * Avoid error input format hang
         */
    } else {
        printf("Wrong argument number!\r\n");
        printf
            ("Usage:snwbdmaw <dma_addr><dma_len><blk_pge_addr>  : SPI-NAND Winbond DMA write in hex\r\n");
        return CMD_RET_FAILURE;
    }
#endif

#if 1
    void           *dma_addr =
        Strtoul((const u8 *) (argv[1]), (u8 **) NULL, 16);
    unsigned int    dma_len =
        Strtoul((const u8 *) (argv[2]), (u8 **) NULL, 16);
    unsigned int    blk_pge_addr =
        Strtoul((const u8 *) (argv[3]), (u8 **) NULL, 16);


#endif

    // winbond_dma_write_data(void *dma_addr, unsigned int dma_len,
    // unsigned int blk_pge_addr);
    winbond_dma_write_data(dma_addr, dma_len, blk_pge_addr);
    return CMD_RET_SUCCESS;

}


U_BOOT_CMD(snwbdmaw, 4, 0, winbond_dma_write_data_uboot,
           "snwbdmaw <dma_addr><dma_len><blk_pge_addr>  : SPI-NAND Winbond DMA write \n",
           "");
#endif


SECTION_NAND_SPI void
winbond_2KB_chunk_read(void *dma_addr, unsigned int blk_pge_addr
)
/*
 * Read One Chunk (2048-Byte SRC Data + 64-Byte Spare Data)
 * Start from the initial cache register (CA=0x0000)
 */
{
    winbond_dma_read_data(dma_addr, WINBOND_2KB_CHUNK_SPARE_SIZE,
                          blk_pge_addr);
}

#if 0   // CONF_SPI_NAND_UBOOT_COMMAND
static int
winbond_2KB_chunk_read_uboot(cmd_tbl_t * cmdtp, int flag, int argc,
                             char *argv[]
)
{
#if 0
    unsigned int    flash_src_addr =
        Strtoul((const u8 *) (argv[1]), (u8 **) NULL, 16);
    unsigned int    dram_dest_addr =
        Strtoul((const u8 *) (argv[2]), (u8 **) NULL, 16);
    unsigned int    length =
        Strtoul((const u8 *) (argv[3]), (u8 **) NULL, 16);
#else
    void           *dma_addr =
        Strtoul((const u8 *) (argv[1]), (u8 **) NULL, 16);
    // unsigned int dma_len= Strtoul((const u8*)(argv[2]), (u8 **)NULL,
    // 16);
    unsigned int    blk_pge_addr =
        Strtoul((const u8 *) (argv[2]), (u8 **) NULL, 16);

    // printf("\ndma_addr =0x%x\n",*dma_addr);
    // printf("\ndma_len =0x%x\n",dma_len);
    printf("\nblk_pge_addrr =0x%x\n", blk_pge_addr);
#endif

    // winbond_dma_write_data(void *dma_addr, unsigned int dma_len,
    // unsigned int blk_pge_addr);
    winbond_2KB_chunk_read(dma_addr, blk_pge_addr);
    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(snwb2kr, 3, 0, winbond_2KB_chunk_read_uboot,
           "snwb2kr <dma_addr><blk_pge_addr>  : SPI-NAND Winbond DMA 2KB_chunk_read\n",
           "");
#endif




SECTION_NAND_SPI void
winbond_2KB_chunk_write(void *dma_addr, unsigned int blk_pge_addr
)
/*
 * Write One Chunk (2048-Byte SRC Data + 64-Byte Spare Data)
 * Start from the initial cache register (CA=0x0000)
 */
{
    winbond_dma_write_data(dma_addr, WINBOND_2KB_CHUNK_SPARE_SIZE,
                           blk_pge_addr);
}


#if 0   // CONF_SPI_NAND_UBOOT_COMMAND
static int
winbond_2KB_chunk_write_uboot(cmd_tbl_t * cmdtp, int flag, int argc,
                              char *argv[]
)
{
#if 0
    unsigned int    flash_src_addr =
        Strtoul((const u8 *) (argv[1]), (u8 **) NULL, 16);
    unsigned int    dram_dest_addr =
        Strtoul((const u8 *) (argv[2]), (u8 **) NULL, 16);
    unsigned int    length =
        Strtoul((const u8 *) (argv[3]), (u8 **) NULL, 16);
#else
    void           *dma_addr =
        Strtoul((const u8 *) (argv[1]), (u8 **) NULL, 16);
    // unsigned int dma_len= Strtoul((const u8*)(argv[2]), (u8 **)NULL,
    // 16);
    unsigned int    blk_pge_addr =
        Strtoul((const u8 *) (argv[2]), (u8 **) NULL, 16);

    // printf("\ndma_addr =0x%x\n",*dma_addr);
    // printf("\ndma_len =0x%x\n",dma_len);
    printf("\nblk_pge_addrr =0x%x\n", blk_pge_addr);
#endif

    // winbond_dma_write_data(void *dma_addr, unsigned int dma_len,
    // unsigned int blk_pge_addr);
    winbond_2KB_chunk_write(dma_addr, blk_pge_addr);
    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(snwb2kw, 3, 0, winbond_2KB_chunk_write_uboot,
           "snwb2kw <dma_addr><blk_pge_addr>  : SPI-NAND Winbond DMA 2KB_chunk_write\n",
           "");
#endif




SECTION_NAND_SPI void
winbond_2KB_chunk_write_with_ecc_encode(void *dma_addr,
                                        unsigned int blk_pge_addr,
                                        void *p_eccbuf
)
{
    ecc_encode_bch_6t(dma_addr, p_eccbuf,blk_pge_addr);

    winbond_2KB_chunk_write(dma_addr, blk_pge_addr);
}


#if CONF_SPI_NAND_UBOOT_COMMAND
static int
winbond_2KB_chunk_write_with_ecc_encode_uboot(cmd_tbl_t * cmdtp, int flag,
                                              int argc, char *argv[]
)
{

#if CONFIG_AVOID_ERROR_INPUT_HANG_SPI_NAND
   if ((argv[1]) && (argv[2]) && (argv[3]) ) {
        /*
         * Avoid error input format hang
         */
    } else {
        printf("Wrong argument number!\r\n");
        printf
            ("Usage:snwbwecc <dma_addr><blk_pge_addr><p_eccbuf>  : SPI-NAND Winbond chunk_write with ecc in hex\r\n");
        return CMD_RET_FAILURE;
    }
#endif


#if 1
    void           *dma_addr =
        Strtoul((const u8 *) (argv[1]), (u8 **) NULL, 16);
    unsigned int    blk_pge_addr =
        Strtoul((const u8 *) (argv[2]), (u8 **) NULL, 16);
    void           *p_eccbuf =
        Strtoul((const u8 *) (argv[3]), (u8 **) NULL, 16);




#endif

    // winbond_dma_write_data(void *dma_addr, unsigned int dma_len,
    // unsigned int blk_pge_addr);
    winbond_2KB_chunk_write_with_ecc_encode(dma_addr, blk_pge_addr,
                                            p_eccbuf);
    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(snwbwecc, 4, 0, winbond_2KB_chunk_write_with_ecc_encode_uboot,
           "snwbwecc <dma_addr><blk_pge_addr><p_eccbuf>  : SPI-NAND Winbond chunk_write with ecc\n",
           "");
#endif


SECTION_NAND_SPI int
winbond_2KB_chunk_read_with_ecc_decode(void *dma_addr,
                                       unsigned int blk_pge_addr,
                                       void *p_eccbuf
)
{
	//int i;
	//unsigned char* value = (unsigned char*)dma_addr;
    winbond_2KB_chunk_read(dma_addr, blk_pge_addr);     // whole
                                                        // chunk=2048+64
                                                        // bytes
#if 0
    printf("page_addr = %d\n",blk_pge_addr);

	for(i = 0;i < (2048+64);i++){
		if(*(value+i) != 0xff)
			printf("%d value not 0xff\n",i);
	}
#endif

    return ecc_decode_bch_6t(dma_addr, p_eccbuf,blk_pge_addr);
}

#if CONF_SPI_NAND_UBOOT_COMMAND
static int
winbond_2KB_chunk_read_with_ecc_decode_uboot(cmd_tbl_t * cmdtp, int flag,
                                             int argc, char *argv[]
)
{
#if CONFIG_AVOID_ERROR_INPUT_HANG_SPI_NAND
   if ((argv[1]) && (argv[2]) && (argv[3]) ) {
        /*
         * Avoid error input format hang
         */
    } else {
        printf("Wrong argument number!\r\n");
        printf
            ("Usage:snwbrecc <dma_addr><blk_pge_addr><p_eccbuf>  : SPI-NAND Winbond chunk_read with ecc in hex\r\n");
        return CMD_RET_FAILURE;
    }
#endif

#if 1
    void           *dma_addr =
        Strtoul((const u8 *) (argv[1]), (u8 **) NULL, 16);
    unsigned int    blk_pge_addr =
        Strtoul((const u8 *) (argv[2]), (u8 **) NULL, 16);
    void           *p_eccbuf =
        Strtoul((const u8 *) (argv[3]), (u8 **) NULL, 16);
#endif

    // winbond_dma_write_data(void *dma_addr, unsigned int dma_len,
    // unsigned int blk_pge_addr);
    winbond_2KB_chunk_read_with_ecc_decode(dma_addr, blk_pge_addr,
                                           p_eccbuf);

    return CMD_RET_SUCCESS;
}

U_BOOT_CMD(snwbrecc, 4, 0, winbond_2KB_chunk_read_with_ecc_decode_uboot,
           "snwbrecc <dma_addr><blk_pge_addr><p_eccbuf>  : SPI-NAND Winbond chunk_read with ecc\n",
           "");
#endif



SECTION_NAND_SPI void
winbond_reset(void
)
{
    unsigned int    w_io_len = IO_WIDTH_LEN(SIO_WIDTH, SNFCMR_LEN(1));
    PIO_RAW_CMD(WINBOND_RESET, SNF_DONT_CARE, w_io_len, SNF_DONT_CARE);
}


#if 1
SECTION_NAND_SPI unsigned int
winbond_read_id(void
)
{

#if 1
    unsigned int    dummy = 0x00;
    unsigned int    w_io_len = IO_WIDTH_LEN(SIO_WIDTH, SNFCMR_LEN(2));  // JSW:write
                                                                        // length
                                                                        // =
                                                                        // 2byte

    unsigned int    r_io_len = IO_WIDTH_LEN(SIO_WIDTH, SNFCMR_LEN(3));  // JSW:write
                                                                        // length
                                                                        // =
                                                                        // 3byte


    // #define WINBOND_RDID 0x9f
    unsigned int    ret =
        PIO_RAW_CMD(WINBOND_RDID, dummy, w_io_len, r_io_len);

    return ((ret >> 8) & 0xFFFFFF);
#else
    winbond_reset();

    /*
     * (Step2) Enable CS
     */
    WAIT_SPI_NAND_CTRLR_RDY();
    cs_high();  /* deactivate CS */
    WAIT_SPI_NAND_CTRLR_RDY();
    cs_low();   /* activate CS */

    unsigned int    rdid = winbond_read_id();

    printf("\nrdid =0x%x\n", rdid);


}

return VZERO;

#endif
}

#endif



SECTION_NAND_SPI plr_nand_spi_info_t *
probe_winbond_spi_nand_chip(void
)
{
	unsigned int    rdid;
    winbond_reset();    // caused BUF=1
    rdid = winbond_read_id();
    if (WINBOND_RDID_W25N01GV == rdid) {
        winbond_set_buffer_mode();
        winbond_block_unprotect();
        winbond_disable_on_die_ecc();
#if 0//def NAND_SPI_USE_DIO
#error
        winbond_cmd_info.r_cmd = WINBOND_FAST_READ_DIO;
        winbond_cmd_info.r_addr_io = DIO_WIDTH;
        winbond_cmd_info.r_data_io = DIO_WIDTH;
#endif
        printf("\nFound recognized ID, rdid=0x%x\n", rdid);
        return &winbond_W25N01GV_info;
    }else if(WINBOND_RDID_W25N02GV == rdid) {
        winbond_set_buffer_mode();
        winbond_block_unprotect();
        winbond_disable_on_die_ecc();
#if 0//def NAND_SPI_USE_DIO
#error
        winbond_cmd_info.r_cmd = WINBOND_FAST_READ_DIO;
        winbond_cmd_info.r_addr_io = DIO_WIDTH;
        winbond_cmd_info.r_data_io = DIO_WIDTH;
#endif
        printf("\nFound recognized ID, rdid=0x%x\n", rdid);
        return &winbond_W25N02GV_info;
    }else if(ESMT_RDID_F50L1G41A == rdid) {
        winbond_set_buffer_mode();
        winbond_block_unprotect();
        winbond_disable_on_die_ecc();
#if 0//def NAND_SPI_USE_DIO
#error
        winbond_cmd_info.r_cmd = WINBOND_FAST_READ_DIO;
        winbond_cmd_info.r_addr_io = DIO_WIDTH;
        winbond_cmd_info.r_data_io = DIO_WIDTH;
#endif
        printf("\nFound recognized ID, rdid=0x%x\n", rdid);
        return &ESMT_F50L1G41A_info;
    }else if(MXIC_RDID_MX35LF1GE4AB == (rdid>>8)) {
        winbond_set_buffer_mode();
        winbond_block_unprotect();
        winbond_disable_on_die_ecc();
#if 0//def NAND_SPI_USE_DIO
#error
        winbond_cmd_info.r_cmd = WINBOND_FAST_READ_DIO;
        winbond_cmd_info.r_addr_io = DIO_WIDTH;
        winbond_cmd_info.r_data_io = DIO_WIDTH;
#endif
        printf("\nFound recognized ID, rdid=0x%x\n", rdid);
        return &MXIC_MX35LF1GE4AB_info;
    }
    else {
        printf("\nNo recognized ID, rdid=0x%x\n", rdid);
    }
    return VZERO;
    // return CMD_RET_SUCCESS;
}


#if CONF_SPI_NAND_UBOOT_COMMAND
static int
probe_winbond_spi_nand_chip_uboot(cmd_tbl_t * cmdtp, int flag, int argc,
                                  char *argv[]
)
{
    // dram_normal_patterns();


    /*
     * Select cs0 or cs1 for user mode , auto mode's setting is
     * FLASH_SIZE(offset + 0x124)
     */
#if 0
    u32             chip_select =
        Strtoul((const u8 *) (argv[1]), (u8 **) NULL, 16);
    // SPI_ChipSelect(chip_select);

#endif



    // flash_EN4B(&dev);
    probe_winbond_spi_nand_chip();



    return CMD_RET_SUCCESS;
}



U_BOOT_CMD(snwb, 1, 0, probe_winbond_spi_nand_chip_uboot,
           "snwb  : Probe SPI NAND for Winbond series \n", "");
#endif








#if CONF_SPI_NAND_UBOOT_COMMAND
static int
SPI_NAND_RDID(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]
)
{
    // dram_normal_patterns();


    unsigned int    spi_nand_id = 0x0;
    /*
     * Select cs0 or cs1 for user mode , auto mode's setting is
     * FLASH_SIZE(offset + 0x124)
     */
#if 0
    u32             chip_select =
        Strtoul((const u8 *) (argv[1]), (u8 **) NULL, 16);
    // SPI_ChipSelect(chip_select);

#endif

#if 0
    if (argc < 2) {
        printf("Wrong argument number!\r\n");
        printf("Usage:snid<chip_select>\r\n");


        return CMD_RET_FAILURE;
    }


#endif


    // flash_EN4B(&dev);
    spi_nand_id = winbond_read_id();

    printf("\nGet spi_nand_id =0x%x\n", spi_nand_id);

    return CMD_RET_SUCCESS;
}



U_BOOT_CMD(snid, 1, 0, SPI_NAND_RDID,
           "snid  : Send RDID(0x9f) and get SPI-NAND ID  \n", "");
#endif


#if 0
SECTION_NAND_SPI unsigned int
winbond_read_status(void
)
{

#if 1
    unsigned int    dummy = 0x0C;
    unsigned int    w_io_len = IO_WIDTH_LEN(SIO_WIDTH, SNFCMR_LEN(2));  // JSW:write
                                                                        // length
                                                                        // =
                                                                        // 2byte

    unsigned int    r_io_len = IO_WIDTH_LEN(SIO_WIDTH, SNFCMR_LEN(1));  // JSW:write
                                                                        // length
                                                                        // =
                                                                        // 3byte


    // #define WINBOND_RDID 0x9f
    unsigned int    ret =
        PIO_RAW_CMD(WINBOND_READ_STATUS, dummy, w_io_len, r_io_len);

    return ((ret >> 24) & 0xff);
#else
    winbond_reset();

    /*
     * (Step2) Enable CS
     */
    WAIT_SPI_NAND_CTRLR_RDY();
    cs_high();  /* deactivate CS */
    WAIT_SPI_NAND_CTRLR_RDY();
    cs_low();   /* activate CS */

    unsigned int    rdid = winbond_read_id();

    printf("\nrdid =0x%x\n", rdid);


}

return VZERO;

#endif
}

#endif



void winbond_bbm(unsigned int lba,unsigned int pba)
{
	unsigned int data = ((lba & 0xffff)<< 16) | (pba & 0xffff);

	PIO_RAW_CMD(WINBOND_BBM, data, SNF_DONT_CARE, SNF_DONT_CARE);
}


#if CONF_SPI_NAND_UBOOT_COMMAND
static int
SPI_NAND_BBM(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]
)
{
	unsigned long lba = Strtoul((const u8 *) (argv[1]), (u8 **) NULL, 16);
	unsigned long pba = Strtoul((const u8 *) (argv[1]), (u8 **) NULL, 16);

    winbond_bbm(lba,pba);

    return 0;
}


U_BOOT_CMD(snbbm, 1, 0, SPI_NAND_BBM,
           "snbbm  : Send BBM(0xA1h) and set bad block table  \n", "");
#endif


SECTION_NAND_SPI void
winbond_read_bbm_tbl(void)
{
	int i,j = 0;
	unsigned short lba,pba;
	#define BBT_NUM 20

	unsigned int    dummy = 0x00;
	unsigned char buf[BBT_NUM*4];

	PIO_WRITE_READ_DATA(WINBOND_READ_BBM_TBL, dummy, SNF_DONT_CARE, SNF_DONT_CARE,BBT_NUM,buf);


	for(i = 0; i < 20;i++)
	{
		lba = (buf[j] <<8) | buf[j+1] ;
		pba = (buf[j+2] << 8) | buf[j+3];

		j += 4;
		//printf("
		printf("lba=%u,pba=%u\n",lba,pba);
	}

    return;


}


#if CONF_SPI_NAND_UBOOT_COMMAND
static int
SPI_NAND_READ_BBM(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]
)
{
	#define TBL_NUM	20

	char buf

    winbond_read_bbm_tbl(buf);

	return 0;
}



U_BOOT_CMD(snrbbm, 1, 0, SPI_NAND_READ_BBM,
           "snrbbm  : Send BBM_LUT(0xa5) and get BBM Table  \n", "");
#endif


// REG_SPI_NAND_PROBE_FUNC(probe_winbond_spi_nand_chip);
