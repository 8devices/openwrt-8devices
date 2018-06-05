#
# Makefile for the Tulip ethernet driver
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...
#Add mips16 Support

#DIR_RTLASIC = $(DIR_LINUX)/drivers/net/rtl819x/

ifeq ($(CONFIG_RTK_IPTABLES_FAST_PATH),m)
EXTRA_CFLAGS += -DCONFIG_FAST_PATH_MODULE
EXTRA_CFLAGS += -DCONFIG_RTK_IPTABLES_FAST_PATH
endif

ifdef CONFIG_RTL865X_KERNEL_MIPS16_LAYERDRIVER
	CFLAGS_rtl865xC_layer2asic.o	= -mips16
	CFLAGS_rtl865xC_tblAsicDrv.o	= -mips16
	CFLAGS_rtl865xC_tblBasicDrv.o	= -mips16
	CFLAGS_rtl865xC_hs.o		= -mips16
	CFLAGS_rtl8651_tblAsicDrv_cmd.o		= -mips16
endif

obj-y := \
	rtl865x_asicBasic.o \
	rtl865x_asicCom.o \
	rtl865x_asicL2.o \
	rtl865xC_hs.o

	ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER_L3
		obj-y += rtl865x_asicL3.o
	endif

	ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER_L4
		obj-y += rtl865x_asicL4.o 
	endif

#obj-$(CONFIG_RTL865X_CLE) += rtl8651_tblAsicDrv_cmd.o

		
EXTRA_CFLAGS += -O1 -DRTL_TBLDRV -D__linux__  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -Werror
#EXTRA_CFLAGS += -I$(DIR_RTLASIC)/AsicDriver
#EXTRA_CFLAGS += -I$(DIR_RTLASIC)/l2Driver
#EXTRA_CFLAGS += -I$(DIR_RTLASIC)/common
#EXTRA_CFLAGS += -I$(DIR_RTLASIC)/cle
#EXTRA_CFLAGS += -I$(DIR_RTLASIC)

ifeq ($(CONFIG_RTL865X_MODULE_ROMEDRV),y)
	EXTRA_CFLAGS += -D__KERNEL__ 
	EXTRA_CFLAGS += -G 0 asdfasdfasdfasdfasdfasdfasdfasdfasdf
	EXTRA_CFLAGS += -DMODULE asdfasdf
	EXTRA_CFLAGS += -mlong-calls
	EXTRA_CFLAGS += -DEXPORT_SYMTAB 
	EXTRA_CFLAGS += -DCONFIG_RTL865X_MODULE_INTERNAL
endif

EXTRA_AFLAGS += $(EXTRA_CFLAGS)
