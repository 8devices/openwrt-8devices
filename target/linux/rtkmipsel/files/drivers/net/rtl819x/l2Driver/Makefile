#
# Makefile for the Tulip ethernet driver
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...
#Add mips16 Support

DIR_RTLASIC = $(DIR_LINUX)/drivers/net/rtl819x/

ifdef CONFIG_RTL865X_KERNEL_MIPS16_LAYERDRIVER
	CFLAGS_rtl865x_fdb.o		= -mips16
	#CFLAGS_rtl865x_outputQueue.o 	= -mips16
endif

	ifdef CONFIG_RTL_LAYERED_DRIVER_L2
		obj-y := rtl865x_fdb.o
		ifdef CONFIG_RTL_HW_QOS_SUPPORT
			obj-y += rtl865x_outputQueue.o
		endif
	endif

ifeq ($(CONFIG_OPENWRT_SDK),y)
else
	obj-y += rtl865x_hw_qos_config.o
endif

ifeq ($(CONFIG_RTL_STP),y)
	obj-y += rtl865x_stp.o
endif
ifeq ($(CONFIG_RTL_HW_STP),y)
	obj-y += rtl865x_stp.o
endif

EXTRA_CFLAGS += -O1 -DRTL_TBLDRV -D__linux__  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -Werror
#EXTRA_CFLAGS += -I$(DIR_RTLASIC)/AsicDriver
#EXTRA_CFLAGS += -I$(DIR_RTLASIC)/l2Driver
#EXTRA_CFLAGS += -I$(DIR_RTLASIC)/common
#EXTRA_CFLAGS += -I$(DIR_RTLASIC)/cle
EXTRA_CFLAGS += -I$(DIR_RTLASIC)

ifeq ($(CONFIG_RTL865X_MODULE_ROMEDRV),y)
	EXTRA_CFLAGS += -D__KERNEL__ 
	EXTRA_CFLAGS += -G 0
	EXTRA_CFLAGS += -DMODULE
	EXTRA_CFLAGS += -mlong-calls
	EXTRA_CFLAGS += -DEXPORT_SYMTAB 
	EXTRA_CFLAGS += -DCONFIG_RTL865X_MODULE_INTERNAL
#	EXTRA_CFLAGS += -DMODVERSIONS 
#	EXTRA_CFLAGS += -include $(DIR_LINUX)/include/linux/modversions.h
endif

EXTRA_AFLAGS += $(EXTRA_CFLAGS)
