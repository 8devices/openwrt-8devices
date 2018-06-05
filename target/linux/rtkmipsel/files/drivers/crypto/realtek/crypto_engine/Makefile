obj-y += rtl_ipsec.o
obj-y += sha1.o md5.o hmac.o
obj-$(CONFIG_CRYPTO_DEV_REALTEK_TEST) += sw_sim/
obj-$(CONFIG_CRYPTO_DEV_REALTEK_TEST) += rtl_ipsec_test.o

EXTRA_CFLAGS += -I./$(obj)

ifeq ($(CONFIG_RTL_ICTEST),y)
obj-y += rtl_ipsec_wrapper.o
EXTRA_CFLAGS += -I./$(obj) -I./$(obj)/../ -I./$(obj)/../../../AsicDriver -I./$(obj)/../../../rtl865x -I./$(obj)/../../../cle  -Wall -Wuninitialized -Wstrict-prototypes
EXTRA_CFLAGS += -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX
ifeq ($(CONFIG_RTL865X_MODEL_KERNEL),y)
EXTRA_CFLAGS += -I./$(obj)/../../../model -I./$(obj)/../../../model/865xC
EXTRA_CFLAGS += -DRTL865X_MODEL_KERNEL
endif
endif
