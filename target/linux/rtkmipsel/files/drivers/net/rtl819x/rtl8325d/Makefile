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
    rtl8316d_asicDrv.o \
    rtl8316d_asicdrv_dot1x.o \
    rtl8316d_asicdrv_inacl.o \
    rtl8316d_asicdrv_lut.o \
    rtl8316d_asicdrv_mib.o \
    rtl8316d_asicdrv_mir.o \
    rtl8316d_asicdrv_nic.o \
    rtl8316d_asicdrv_port.o \
    rtl8316d_asicdrv_qos.o \
    rtl8316d_asicdrv_rma.o \
    rtl8316d_asicdrv_rtkpp.o \
    rtl8316d_asicdrv_stp.o \
    rtl8316d_asicdrv_svlan.o \
    rtl8316d_asicdrv_tbl.o \
    rtl8316d_asicdrv_trunk.o \
    rtl8316d_asicdrv_vlan.o \
	basic/rtl8316d_regField_list.o \
	basic/rtl8316d_reg_list.o \
	basic/rtl8316d_tableField_list.o \
	basic/rtl8316d_table_list.o

#EXTRA_CFLAGS += -O1 -DRTL_TBLDRV -D__linux__  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -Werror
EXTRA_CFLAGS += -O1 -DRTL_TBLDRV -D__linux__  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -Wno-implicit
EXTRA_CFLAGS += -I$(DIR_RTLASIC)rtl8325d/include -I$(DIR_RTLASIC)rtl8325d/include/asicdrv -I$(DIR_RTLASIC)rtl8325d/include/asicdrv/basic

EXTRA_AFLAGS += $(EXTRA_CFLAGS)
