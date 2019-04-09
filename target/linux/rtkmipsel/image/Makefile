#
# Copyright (C) 2006-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

BlockSize=64k
RAMFS_MAX_SIZE = 67108864	# 0x4000000

define mkcmdline
board=$(shell echo $(1) | tr a-z A-Z) console=$(2),$(3) hwpart=0x$(4) mtdparts=m25p80:128k(boot),128k(hwpart),256k(config),-(firmware)
endef

define test
$(shell test $(1) $(2) $(3) && echo 1)
endef

define filesize
$(shell stat -L -c %s $1)
endef

SINGLE_PROFILES :=

define SingleProfile

define Image/Prepare/Profile/$(1)/initramfs
	$$(if $$(call test,$$(call filesize,$(KDIR)/vmlinux-initramfs),-gt,$(RAMFS_MAX_SIZE)),\
		$$(error $(KDIR)/vmlinux-initramfs bigger then $(RAMFS_MAX_SIZE) bytes))
	lzma e $(KDIR)/vmlinux-initramfs -lc1 -lp2 -pb2 $(KDIR)/vmlinux-initramfs.bin.lzma

	$(call Image/BuildLoader,loader-$(SUBTARGET)-$(1)-ramfs,bin,$(call mkcmdline,$(1),$(2),$(3),$(6)),$(4),vmlinux-initramfs.bin.lzma)
endef

define Image/Prepare/Profile/$(1)/squashfs
	$(call Image/BuildLoader,loader-$(SUBTARGET)-$(1),bin,$(call mkcmdline,$(1),$(2),$(3),$(6)),$(4),vmlinux.bin.lzma)
endef

define Image/Prepare/Profile/$(1)/sdcard
	$(call Image/BuildLoader,loader-$(SUBTARGET)-$(1)-sdcard,bin,$(call mkcmdline,$(1),$(2),$(3),$(6)) root=/dev/mmcblk0p2 rootwait,$(4),vmlinux.bin.lzma)
endef

define Image/Build/Profile/$(1)/initramfs
	cp $(KDIR)/loader-$(SUBTARGET)-$(1)-ramfs.bin $(BIN_DIR)/$(IMG_PREFIX)-$(1)-nfjrom.bin
endef

define Image/Build/Profile/$(1)/squashfs
	cvimg-rtl8197f linux $(KDIR)/loader-$(SUBTARGET)-$(1).bin $(BIN_DIR)/$(IMG_PREFIX)-$(1)-linux.bin $(4) $(5)
	dd if=$(BIN_DIR)/$(IMG_PREFIX)-$(1)-linux.bin of=$(BIN_DIR)/$(IMG_PREFIX)-$(1)-linux_4k.bin bs=$(BlockSize) conv=sync
	cat $(BIN_DIR)/$(IMG_PREFIX)-$(1)-linux_4k.bin $(KDIR)/root.squashfs-4k > $(BIN_DIR)/$(IMG_PREFIX)-$(1)-fw_4k_cat.bin
	cvimg-rtl8197f fix_chksum  $(BIN_DIR)/$(IMG_PREFIX)-$(1)-fw_4k_cat.bin $(BIN_DIR)/$(IMG_PREFIX)-$(1)-squashfs-sysupgrade.bin
	rm -rf $(BIN_DIR)/$(IMG_PREFIX)-$(1)-linux_4k.bin $(BIN_DIR)/$(IMG_PREFIX)-$(1)-fw_4k_cat.bin $(BIN_DIR)/$(IMG_PREFIX)-$(1)-linux.bin
endef

define Image/Build/Profile/$(1)/sdcard
	cvimg-rtl8197f linux $(KDIR)/loader-$(SUBTARGET)-$(1)-sdcard.bin $(BIN_DIR)/sdboot/linux.bin $(5) $(6)

	rm -f $(KDIR)/sd.boot
	mkfs.fat $(KDIR)/sd.boot -C 8192

	mcopy -i $(KDIR)/sd.boot $(BIN_DIR)/sdboot/boot.bin ::boot.bin
	mcopy -i $(KDIR)/sd.boot $(BIN_DIR)/sdboot/boot.img ::boot.img
	mcopy -i $(KDIR)/sd.boot $(BIN_DIR)/sdboot/linux.bin ::linux.bin
	./generate_sdcard_img.sh $(BIN_DIR)/$(IMG_PREFIX)-$(1)-sdcard.img $(KDIR)/sd.boot $(KDIR)/root.squashfs 8 256
	rm -f $(KDIR)/sd.boot
endef

SINGLE_PROFILES += $(1)

endef

define MultiProfile

  define Image/Build/Profile/$(1)/initramfs
	$(foreach p,$(2),
		$$(call Image/Build/Profile/$p/initramfs)
	)
  endef

  define Image/Build/Profile/$(1)/squashfs
	$(foreach p,$(2),
		$$(call Image/Build/Profile/$p/squashfs)
	)
  endef

  define Image/Build/Profile/$(1)/sdcard
	$(foreach p,$(2),
		$$(call Image/Build/Profile/$p/sdcard)
	)
  endef

endef

define Image/Prepare
	lzma e $(KDIR)/vmlinux -lc1 -lp2 -pb2 $(KDIR)/vmlinux.bin.lzma

	$(call Image/Prepare/Profile/$(PROFILE)/squashfs)

	$(if $(CONFIG_TARGET_ROOTFS_INITRAMFS), $(call Image/Prepare/$(PROFILE)/initramfs))
	$(if $(CONFIG_RTK_SDCARD_IMG), $(call Image/Prepare/$(PROFILE)/sdcard))
endef

LOADER_MAKE := $(NO_TRACE_MAKE) -C lzma-loader KDIR=$(KDIR) LINUX_DIR=$(LINUX_DIR)

define Build/Clean
	$(LOADER_MAKE) clean
endef

define Image/BuildLoader
	$(LOADER_MAKE) LOADER=$(1).$(2) KERNEL_CMDLINE="$(3)" \
		LZMA_TEXT_START=$(4) LOADADDR=0x80000000 \
		LOADER_DATA="$(KDIR)/$(5)" BOARD="$(1)" \
		compile loader.$(2)
endef

ifeq ($(CONFIG_TARGET_rtkmipsel_rtl8197f_kinkan),y)
$(eval $(call SingleProfile,kinkan,ttyS0,115200,0x84000000,80000,20000))
endif
ifeq ($(CONFIG_TARGET_rtkmipsel_rtl8197f_komikan),y)
$(eval $(call SingleProfile,komikan,ttyS0,115200,0x84000000,80000,20000))
endif

define Image/Build/Initramfs
	$(call Image/Build/Profile/$(PROFILE)/initramfs)
endef

define Image/Build
	dd if=$(KDIR)/root.squashfs of=$(KDIR)/root.squashfs-4k bs=$(BlockSize) conv=sync
	$(call add_jffs2_mark,$(KDIR)/root.squashfs-4k)
	$(call Image/Build/Profile/$(PROFILE)/squashfs)
ifeq ($(CONFIG_RTK_SDCARD_IMG),y)
	$(call Image/Build/Profile/$(PROFILE)/sdcard)
endif
endef

$(eval $(call BuildImage))
