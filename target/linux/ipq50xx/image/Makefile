include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

UBIFS_OPTS = -m 2048 -e 124KiB -c 4096 -U -F
UBI_OPTS = -m 2048 -p 128KiB

IPQ50XX_32_KERNEL_LOADADDR = 0x41208000
IPQ50XX_KERNEL_LOADADDR = 0x41080000

KERNEL_LOADADDR := $(IPQ50XX_32_KERNEL_LOADADDR)
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

DEVICE_VARS += DEVICE_DTS DEVICE_DTS_DIR
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

define Device/8devices-cherry-dvk-ipq5000
  DEVICE_TITLE := 8Devices Cherry DVK (IPQ5000)
  DEVICE_DTS := ipq5018-8dev-cherry-ipq5000
  BOARDNAME := cherry
  IMAGE_SIZE := 29568k
  BLOCKSIZE = 64k
  KERNEL = kernel-bin | lzma | fit-dummy-info lzma $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dtb
  IMAGE/sysupgrade.bin := insert-info | append-kernel $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += 8devices-cherry-dvk-ipq5000

define Device/8devices-cherry-dvk-ipq5010
  DEVICE_TITLE := 8Devices Cherry DVK (IPQ5010)
  DEVICE_DTS := ipq5018-8dev-cherry-ipq5010
  BOARDNAME := cherry
  IMAGE_SIZE := 29568k
  BLOCKSIZE = 64k
  KERNEL = kernel-bin | lzma | fit lzma $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dtb
  IMAGE/sysupgrade.bin := append-kernel $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += 8devices-cherry-dvk-ipq5010

$(eval $(call BuildImage))
