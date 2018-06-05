#
# Makefile for igmp snooping
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...
#Add mips16 Support
ifdef CONFIG_RTL865X_KERNEL_MIPS16_LAYERDRIVER
	CFLAGS_rtl865x_igmpsnooping_glue.o	= -mips16
	#CFLAGS_rtl865x_igmpsnooping.o		= -mips16
endif
DIR_RTLASIC = $(DIR_LINUX)/drivers/net/rtl819x/
CONFIG_NEW_IGMP_IMPLEMENTATION=y
ifeq ($(CONFIG_NEW_IGMP_IMPLEMENTATION),y)
obj-y := rtl865x_igmpsnooping_glue.o rtl865x_igmpsnooping_new.o igmp_delete.o
EXTRA_CFLAGS += -DCONFIG_NEW_IGMP_IMPLEMENTATION
else
obj-y := rtl865x_igmpsnooping_glue.o rtl865x_igmpsnooping.o 
endif

EXTRA_CFLAGS += -O1 -DRTL_TBLDRV -D__linux__ -D __KERNEL__ -DEXPORT_SYMTAB  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -Werror
EXTRA_CFLAGS += -I$(DIR_LINUX)/include
#EXTRA_CFLAGS += -I$(DIR_RTLASIC)/AsicDriver
#EXTRA_CFLAGS += -I$(DIR_RTLASIC)/common
EXTRA_CFLAGS += -I$(DIR_RTLASIC)
#EXTRA_CFLAGS += -I. -I..



ifeq ($(CONFIG_RTL865X_MODULE_ROMEDRV),y)
	EXTRA_CFLAGS += -G 0
	EXTRA_CFLAGS += -DMODULE
	EXTRA_CFLAGS += -mlong-calls
endif

ifeq ($(CONFIG_RTL_MESH_SUPPORT),y)
        EXTRA_CFLAGS += -DCONFIG_RTK_MESH
endif
