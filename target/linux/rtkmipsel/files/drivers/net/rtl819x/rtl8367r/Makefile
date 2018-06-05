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

obj-y := \
    rtk_api.o \
    rtl8367b_asicdrv_acl.o \
    rtl8367b_asicdrv.o \
    rtl8367b_asicdrv_cputag.o \
    rtl8367b_asicdrv_dot1x.o \
    rtl8367b_asicdrv_eav.o \
    rtl8367b_asicdrv_eee.o \
    rtl8367b_asicdrv_fc.o \
    rtl8367b_asicdrv_green.o \
    rtl8367b_asicdrv_hsb.o \
    rtl8367b_asicdrv_igmp.o \
    rtl8367b_asicdrv_inbwctrl.o \
    rtl8367b_asicdrv_interrupt.o \
    rtl8367b_asicdrv_led.o \
    rtl8367b_asicdrv_lut.o \
    rtl8367b_asicdrv_meter.o \
    rtl8367b_asicdrv_mib.o \
    rtl8367b_asicdrv_mirror.o \
    rtl8367b_asicdrv_misc.o \
    rtl8367b_asicdrv_phy.o \
    rtl8367b_asicdrv_port.o \
    rtl8367b_asicdrv_portIsolation.o \
    rtl8367b_asicdrv_qos.o \
    rtl8367b_asicdrv_rma.o \
    rtl8367b_asicdrv_scheduling.o \
    rtl8367b_asicdrv_storm.o \
    rtl8367b_asicdrv_svlan.o \
    rtl8367b_asicdrv_trunking.o \
    rtl8367b_asicdrv_unknownMulticast.o \
    rtl8367b_asicdrv_vlan.o

#EXTRA_CFLAGS += -O1 -DRTL_TBLDRV -D__linux__  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -Werror
EXTRA_CFLAGS += -O1 -DRTL_TBLDRV -D__linux__  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -Wno-implicit
EXTRA_CFLAGS += -I$(DIR_RTLASIC)/rtl8367r

EXTRA_AFLAGS += $(EXTRA_CFLAGS)
