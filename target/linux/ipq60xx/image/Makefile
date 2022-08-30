#
# Copyright (C) 2020 UAB 8Devices. All rights reserved.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

UBIFS_OPTS = -m 2048 -e 124KiB -c 4096 -U -F
UBI_OPTS = -m 2048 -p 128KiB

KERNEL_LOADADDR := 0x41008000
KERNEL_ENTRY := $(KERNEL_LOADADDR)

INFO_IMG_PATH = $(TMP_DIR)/info.tmp
INFO_IMG_SIZE = 152

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

define Build/fit-dummy-info
	dd if=/dev/zero of=$(INFO_IMG_PATH) bs=1 count=$(INFO_IMG_SIZE)
	$(TOPDIR)/scripts/mkits.sh \
		-D $(DEVICE_NAME) -o $@.its -k $@ \
		$(if $(word 2,$(1)),-d $(word 2,$(1))) -C $(word 1,$(1)) -i $(INFO_IMG_PATH) \
		-a $(KERNEL_LOADADDR) -e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-A $(ARCH) -v $(LINUX_VERSION)
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $@.its $@.new
	@mv $@.new $@
endef

define Build/insert-info
	$(STAGING_DIR_HOST)/bin/patch-info \
		--kernel $(word 1,$^) \
		--rootfs $(word 2,$^) \
		--version $(BOARDNAME)
endef

DEVICE_NAME := mango

define Device/8devices-mango-dvk
  DEVICE_TITLE := 8Devices Mango DVK
  DEVICE_DTS := ipq6018-8dev-mango
  BOARDNAME := mango
  IMAGE_SIZE := 27776k
  BLOCKSIZE = 64k
  KERNEL = kernel-bin | lzma | fit-dummy-info lzma $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dtb
  IMAGE/sysupgrade.bin := insert-info | append-kernel $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE)
endef
$TARGET_DEVICES += 8devices-mango-dvk

$(eval $(call BuildImage))
