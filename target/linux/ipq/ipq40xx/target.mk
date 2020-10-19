
SUBTARGET:=ipq40xx
BOARDNAME:=QCA IPQ40xx based boards
CPU_TYPE:=cortex-a7

DEFAULT_PACKAGES += \
	lk-ipq40xx kmod-usb-dwc3-ipq40xx kmod-usb-phy-dwc3-ipq4019 \
	fwupgrade-tools uboot-2016-ipq40xx uboot-2016-ipq40xx_standard \
	kmod-usb-dwc3-of-simple

define Target/Description
	Build firmware image for IPQ40xx SoC devices.
endef
