rtkn_nand-y				+= main.o
rtkn_nand-y				+= ops_rtknand.o
rtkn_nand-y				+= rtknflash_api.o
rtkn_nand-$(CONFIG_PARALLEL_NAND_FLASH) += nand_parallel_ctrl.o
rtkn_nand-$(CONFIG_SPI_NAND_FLASH)	+= nand_spi.o
rtkn_nand-$(CONFIG_SPI_NAND_FLASH)	+= nand_spi_ctrl.o
rtkn_nand-$(CONFIG_SPI_NAND_FLASH)	+= nand_spi_ecc_ctrl.o
rtkn_nand-$(CONFIG_RTK_NORMAL_BBT)	+= rtknflash_bbt.o rtknflash_bbtOps.o
rtkn_nand-$(CONFIG_RTK_REMAP_BBT)	+= rtknflash_bbt.o rtknflash_bbtOps.o

obj-y					+= rtkn_nand.o

EXTRA_CFLAGS				+= -Werror
#obj-$(CONFIG_MTD_NAND_RTKNFLASH)	+= rtknflash.o
