#
# Makefile for the 865x High Performance
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...
# Note 3! Pay attention to the important comments line(21-25 and 42-46)!!!!!

#obj-y := mmap.o

ifeq ($(CONFIG_OPENWRT_SDK),y)
export DIR_LINUX := $(srctree)
endif

DIR_RTLASIC = $(DIR_LINUX)/drivers/net/rtl819x/
EXTRA_CFLAGS += -DCONFIG_RTL865X_MODULE_INTERNAL

#obj-y := rtl_glue.o
#Add mips16 Support
ifdef CONFIG_RTL865X_KERNEL_MIPS16_LAYERDRIVER
	#CFLAGS_rtl865xc_swNic.o		= -mips16
	#CFLAGS_rtl_nic.o		= -mips16
	#CFLAGS_romeper.o		= -mips16
endif

#
#	The following configuration is used for 865xC model code only.
#	In fact, we will port more and more driver files from 865xB to 865xC.
#

obj-y += rtl865xc_swNic.o
obj-y += rtl_nic.o
obj-$(CONFIG_RTL_ROMEPERF_24K) += m24kctrl.o


obj-$(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT) += rtl_multi_wan.o

obj-$(CONFIG_RTL_DEBUG_TOOL) += rtl865x_proc_debug.o
obj-$(CONFIG_RTL_PROC_DEBUG) += rtl865x_proc_debug.o

ifeq ($(CONFIG_RTL_8197F),y)
obj-y += rtl819x_swNic.o
endif

obj-$(CONFIG_RTL865X_ROMEPERF) += romeperf.o
obj-$(CONFIG_RTL_ETH_802DOT1X_SUPPORT) += rtl_dot1x.o
ifeq ($(CONFIG_OPENWRT_SDK),y)
obj-$(CONFIG_SWCONFIG) += rtl819x_switch.o
endif
### for 8196B/8196C/8196E, use rlx5181_perCounter.S
## obj-$(CONFIG_RTL865X_ROMEPERF) += rlx5181_perCounter.o
### for 8198/8196D/8197D/8881A, use rlx5281_perCounter.S
obj-$(CONFIG_RTL865X_ROMEPERF) += rlx5281_perCounter.o

#===>Attention: the following line define subdir with type ":=",not "+=", so to be
#	    sure that the other addition to subdir should not be placed before this line,or 
#	    the addition would be layed over!!
#

obj-y  += rtl865x/built-in.o
subdir-y += rtl865x

EXTRA_CFLAGS += -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -D__KERNEL__ -Wno-implicit 
ifeq (y, $(filter y, $(CONFIG_RTL_8367R_SUPPORT) $(CONFIG_RTL_83XX_SUPPORT)))
else
ifeq ($(CONFIG_RTL_AP_PACKAGE), y)
else
EXTRA_CFLAGS += -Werror
endif
endif

#EXTRA_CFLAGS += -DCONFIG_RTL865X_IGMP_SNOOPING

ifeq ($(CONFIG_MESH_ENABLE),y)
        EXTRA_CFLAGS += -DCONFIG_RTK_MESH #-DMESH_USE_METRICOP
        ifeq ($(CONFIG_11S_TEST_MODE),y)
        EXTRA_CFLAGS += -D_11s_TEST_MODE_
        endif
endif

#######################################################################################################_CFLAGS
#
#	FAST EXTERNAL DEVICES related files
#
#	Fast path for external device process
#
########################################################################################################

EXTRA_CFLAGS += -I$(DIR_LINUX)/include
EXTRA_CFLAGS += -I$(DIR_ROOT)/target/bsp
EXTRA_CFLAGS += -I$(DIR_RTLASIC)
EXTRA_CFLAGS += -I$(DIR_BOARD)
EXTRA_CFLAGS += -D__KERNEL__ 
EXTRA_CFLAGS += -DEXPORT_SYMTAB 
ifeq ($(CONFIG_RTL865X_MODULE_ROMEDRV),y)
EXTRA_CFLAGS += -G 0
EXTRA_CFLAGS += -DMODULE
EXTRA_CFLAGS += -mlong-calls
endif		#$(CONFIG_RTL865X_MODULE_ROMEDRV)!=y

EXTRA_AFLAGS += $(EXTRA_CFLAGS)
