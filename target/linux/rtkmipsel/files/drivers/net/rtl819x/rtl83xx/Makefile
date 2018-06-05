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
    rtl8367c_asicdrv_acl.o \
    rtl8367c_asicdrv.o \
    rtl8367c_asicdrv_cputag.o \
    rtl8367c_asicdrv_dot1x.o \
    rtl8367c_asicdrv_eav.o \
    rtl8367c_asicdrv_eee.o \
    rtl8367c_asicdrv_fc.o \
    rtl8367c_asicdrv_green.o \
    rtl8367c_asicdrv_hsb.o \
    rtl8367c_asicdrv_igmp.o \
    rtl8367c_asicdrv_inbwctrl.o \
    rtl8367c_asicdrv_interrupt.o \
    rtl8367c_asicdrv_led.o \
    rtl8367c_asicdrv_lut.o \
    rtl8367c_asicdrv_meter.o \
    rtl8367c_asicdrv_mib.o \
    rtl8367c_asicdrv_mirror.o \
    rtl8367c_asicdrv_misc.o \
    rtl8367c_asicdrv_phy.o \
    rtl8367c_asicdrv_port.o \
    rtl8367c_asicdrv_portIsolation.o \
    rtl8367c_asicdrv_qos.o \
    rtl8367c_asicdrv_rma.o \
    rtl8367c_asicdrv_scheduling.o \
    rtl8367c_asicdrv_storm.o \
    rtl8367c_asicdrv_svlan.o \
    rtl8367c_asicdrv_trunking.o \
    rtl8367c_asicdrv_unknownMulticast.o \
    rtl8367c_asicdrv_vlan.o \
	rtl8367c_asicdrv_oam.o \
	rtl8367c_asicdrv_rldp.o \
	rtl8367c_phy_test.o \
	acl.o \
    cpu.o \
    dot1x.o \
    eee.o \
    igmp.o \
    interrupt.o \
    l2.o \
    leaky.o \
    led.o \
    mirror.o \
    oam.o \
    port.o \
    ptp.o \
    qos.o \
    rate.o \
    rldp.o \
    rtk_switch.o \
    stat.o \
    storm.o \
    svlan.o \
    trap.o \
    trunk.o \
    vlan.o \
	oam.o

#EXTRA_CFLAGS += -O1 -DRTL_TBLDRV -D__linux__  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -Werror
EXTRA_CFLAGS += -O1 -DRTL_TBLDRV -D__linux__  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -Wno-implicit
EXTRA_CFLAGS += -I$(DIR_RTLASIC)/rtl83xx

EXTRA_AFLAGS += $(EXTRA_CFLAGS)
