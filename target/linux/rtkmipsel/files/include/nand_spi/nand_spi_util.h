#ifndef NAND_SPI_UTIL_H
#define NAND_SPI_UTIL_H
//#include <soc.h>


#define chunk_buffer ((void*)(OTTO_PLR_CHUNK_BUFFER))
#define oob_buffer   ((oob_t*)(OTTO_PLR_OOB_BUFFER))
#define ecc_buffer   ((void*)(OTTO_PLR_ECC_BUFFER))

typedef oob6t_t oob_t;

int nsu_init(void);
int nsu_logical_chunk_read(void *data, u32_t page_num);
extern plr_nand_spi_info_t plr_spi_info SECTION_SDATA;

#endif
