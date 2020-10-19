#
# Copyright (C) 2018 UAB 8Devices. All rights reserved.
# Copyright (c) 2014 The Linux Foundation. All rights reserved.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

UBIFS_OPTS = -m 2048 -e 124KiB -c 4096 -U -F

define Device/Default
  KERNEL = kernel-bin | lzma | fit lzma $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dtb
  KERNEL_IN_UBI := 1
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := append-ubi
  BLOCKSIZE := 128KiB
  PAGESIZE := 2048
  UBINIZE_OPTS :=
  DEVICE_DTS_DIR := $(DTS_DIR)
  DEVICE_PACKAGES := kmod-qca-edma kmod-qca-psdk kmod-qca-ssdk-hnat kmod-qca_85xx_sw
endef
DEVICE_VARS += DEVICE_DTS DEVICE_DTS_DIR

ifeq ($(SUBTARGET),ipq60xx)
include ipq60xx.mk
endif

$(eval $(call BuildImage))
