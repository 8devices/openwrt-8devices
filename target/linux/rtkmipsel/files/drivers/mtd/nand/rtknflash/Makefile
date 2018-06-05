rtknflash-y				+= main.o
rtknflash-y				+= ops_rtknand.o
rtknflash-y				+= rtknflash_api.o
rtknflash-$(CONFIG_RTK_NORMAL_BBT)	+= rtknflash_bbt.o rtknflash_bbtOps.o
rtknflash-$(CONFIG_RTK_REMAP_BBT)	+= rtknflash_bbt.o rtknflash_bbtOps.o

obj-y					+= rtknflash.o
#obj-$(CONFIG_MTD_NAND_RTKNFLASH)	+= rtknflash.o
