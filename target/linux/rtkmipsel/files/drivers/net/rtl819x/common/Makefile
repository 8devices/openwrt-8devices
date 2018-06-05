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

ifeq ($(CONFIG_RTK_IPTABLES_FAST_PATH),m)
EXTRA_CFLAGS += -DCONFIG_FAST_PATH_MODULE
endif
ifdef CONFIG_RTL865X_KERNEL_MIPS16_LAYERDRIVER
	CFLAGS_rtl_utils.o		= -mips16
	CFLAGS_rtl865xC_tblDrvPatch.o	= -mips16
	CFLAGS_rtl865x_vlan.o		= -mips16
	CFLAGS_rtl865x_netif.o		= -mips16
	CFLAGS_rtl865x_eventMgr.o	= -mips16
endif
	ifeq ($(CONFIG_RTL_LAYERED_ASIC_DRIVER),y)
		obj-y := rtl865x_vlan.o rtl865x_netif.o rtl865x_eventMgr.o	
	else
		obj-y := rtl865xC_tblDrvPatch.o rtl865x_vlan.o rtl865x_netif.o rtl865x_eventMgr.o
	endif

ifeq ($(CONFIG_RTL_PROC_DEBUG), y)
	obj-y += rtl_utils.o
endif

ifeq ($(CONFIG_RTL_819XD), y)
	obj-y += smi.o gpio.o
endif

ifeq ($(CONFIG_RTL_8197F), y)
	ifeq (y, $(filter y, $(CONFIG_RTL_8367R_SUPPORT) $(CONFIG_RTL_83XX_SUPPORT)))
		obj-y += smi.o gpio.o
	endif
endif

ifeq ($(CONFIG_RTL_8198C_8367RB), y)
	obj-y += smi.o gpio.o
endif

EXTRA_CFLAGS += -O1 -DRTL_TBLDRV -D__linux__  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -Werror
EXTRA_CFLAGS += -I$(DIR_RTLASIC)

ifeq ($(CONFIG_RTL865X_MODULE_ROMEDRV),y)
	EXTRA_CFLAGS += -D__KERNEL__ 
	EXTRA_CFLAGS += -G 0
	EXTRA_CFLAGS += -DMODULE
	EXTRA_CFLAGS += -mlong-calls
	EXTRA_CFLAGS += -DEXPORT_SYMTAB 
	EXTRA_CFLAGS += -DCONFIG_RTL865X_MODULE_INTERNAL
endif

EXTRA_AFLAGS += $(EXTRA_CFLAGS)
