#ifndef RTK8197_DATA_H
#define RTK8197_DATA_H
#define RTK8197_REG_BASE		0x18000000
#define RTK8197_PINMUX_REG_BASE		RTK8197_REG_BASE + 0x800 /*  0x800 */
#define RTK8197_PINMUX_REG_COUNT	19
#define RTK8197_GPIO_REG_BASE		RTK8197_REG_BASE + 0x3500
#define RTK8197_GPIO_REG_COUNT		12

#define RTK8197_GPIO_COUNT		62

#define RTK8197_GPIO_ABCD_DIR	0x08
#define RTK8197_GPIO_ABCD_DAT	0x0c
#define RTK8197_GPIO_EFGH_DIR	0x24
#define RTK8197_GPIO_EFGH_DAT	0x28

#define RTK8197_GPIO_DIR(GPIO)		(GPIO & 0x20) ? RTK8197_GPIO_EFGH_DIR : RTK8197_GPIO_ABCD_DIR
#define RTK8197_GPIO_DAT(GPIO)		(GPIO & 0x20) ? RTK8197_GPIO_EFGH_DAT : RTK8197_GPIO_ABCD_DAT
#define RTK8197_GPIO_SHIFT(GPIO)	(GPIO & 0x1F)


const uint32_t rtk8197_default_pinmux_register[] = {
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x11111000,
	0x00100000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x30300000,
	0x20000000,
	0x00000000,
	0x00020000,
	0x00000000,
	0x00000000,
};

const uint32_t rtk8197_default_gpio_register[] = {
	0xffefffff,
	0x00000000,
	0xff100000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0xfabfffff,
	0x00000000,
	0x04400000,
	0x05400000,
	0x00000000,
	0x00000000,
	0x00000000,
};


#define str(s) #s
#define xstr(s) str(s)

#define RTL_MUX(bank, id, ...) \
	const char *rtl_mux_ ## bank ## id ## _x[] = { __VA_ARGS__ };

#define RTL_GPIO(gpio, reg, shift, bank, id) \
	{ reg,  shift, gpio, xstr(GPIO_ ## bank ## id), ARRAY_SIZE(rtl_mux_ ## bank ## id ## _x), rtl_mux_ ## bank ## id ## _x }

//RTK_MUX_00
RTL_MUX(B, 6, "RGMII", "JTAG", "NAND", "SPI1", "UART1", "II2C1", "XOUT", "PWM", "GPIO", "DBG");
RTL_MUX(B, 0, "RGMII", "NAND", "SPI0", "I2C1", "IIS", "IISA", "XOUT", "PWM", "GPIO", "DBG");
RTL_MUX(A, 7, "RGMII", "NAND", "SPI0", "IIS", "XOUT", "PWM", "GPIO", "DBG");
RTL_MUX(A, 6, "RGMII", "NAND", "SPI0", "IIS", "PWM", "GPIO", "DBG");

//RTK_MUX_01
RTL_MUX(B, 5, "RGMII", "NAND", "SPI0", "UART2", "IIS", "PCM", "EVENT", "GPIO", "DBG");
RTL_MUX(B, 4, "RGMII", "JTAG", "NAND", "SPI0", "UART2", "IIS", "PCM", "EVENT", "GPIO", "DBG");
RTL_MUX(B, 3, "RGMII", "JTAG", "NAND", "SPI0", "UART2", "IIS", "PCM", "EVENT", "GPIO", "DBG");
RTL_MUX(B, 2, "RGMII", "JTAG", "NAND", "SPI0", "UART2", "IIS", "PCM", "EVENT", "GPIO", "DBG");

//RTK_MUX_02
RTL_MUX(B, 7, "RGMII", "NAND", "SPI1", "UART1", "IIS", "I2C1", "I2C0", "EVENT", "GPIO", "DBG");
RTL_MUX(C, 0, "RGMII", "NAND", "SPI1", "UART1", "I2C0", "PWM", "GPIO", "DBG");
RTL_MUX(B, 1, "RGMII", "JTAG", "NAND", "SPI0", "SPI0_3W", "I2C1", "IISV", "IIS", "IISA", "PWM", "GPIO", "DBG");
RTL_MUX(C, 1, "RGMII", "NAND", "SPI1", "SPI1_3W", "UART1", "I2C0", "PWM", "GPIO", "DBG");
RTL_MUX(C, 2, "RGMII", "SPI0", "SPI1", "I2C0", "I2C1", "UART1", "GPIO", "DBG");
RTL_MUX(C, 3, "RGMII", "SPI0", "SPI1", "I2C0", "I2C1", "UART1", "GPIO", "DBG");

//RTK_MUX_03
//RTK_MUX_04
//RTK_MUX_05
//RTK_MUX_06
RTL_MUX(A, 1, "DEFLT_BOOT_INTF", "NFBI", "SPINOR", "SPINAND", "IIS", "I2C0", "UART1", "SPI0", "GPIO", "DBG");
RTL_MUX(A, 2, "DEFLT_BOOT_INTF", "NFBI", "SPINOR", "SPINAND", "IISA", "IISV", "I2C1", "I2C1_S", "UART1", "SPI0", "SPI0_3W", "GPIO", "DBG");

//RTK_MUX_07
RTL_MUX(A, 0, "DEFLT_BOOT_INTF", "NFBI", "SPINOR", "SPINAND", "IIS", "I2C0", "UART1", "SPI0", "GPIO", "DBG");
RTL_MUX(A, 4, "DEFLT_BOOT_INTF", "NFBI", "SPINOR", "SPINAND", "ISS", "I2C1", "XOUT", "GPIO", "DBG");
RTL_MUX(A, 5, "DEFLT_BOOT_INTF", "NFBI", "SPINOR", "SPINAND", "IIS", "I2C1", "GPIO", "DBG");
RTL_MUX(A, 3, "DEFLT_BOOT_INTF", "NFBI", "SPINOR", "SPINAND", "IIS", "I2C1", "UART1", "SPI0", "GPIO", "DBG");

//RTK_MUX_08
RTL_MUX(F, 7, "WLAN", "JTAG", "PCM", "UART1", "IIS", "PWM", "GPIO", "DBG");
RTL_MUX(F, 6, "WLAN", "JTAG", "SPI0", "IISA", "IISV", "XOUT", "GPIO", "DBG");
RTL_MUX(G, 0, "WLAN", "JTAG", "PCM", "UART1", "PWM", "XOUT", "GPIO", "DBG");
RTL_MUX(G, 1, "WLAN", "JTAG", "PCM", "UART1", "IIS", "I2C0", "PWM", "GPIO", "DBG");
RTL_MUX(G, 2, "WLAN", "JTAG", "PCM", "UART1", "IIS", "I2C0", "PWM", "GPIO", "DBG");
RTL_MUX(H, 5, "PCIE", "GPIO", "DBG");

//RTK_MUX_09
RTL_MUX(G, 3, "USBH", "LOWV", "GPIO", "DBG");
RTL_MUX(G, 4, "USBH", "GPIO", "DBG");
RTL_MUX(G, 5, "GPIO", "DBG");

//RTK_MUX_10
//RTK_MUX_11
//RTK_MUX_12
RTL_MUX(H, 4, "UART", "GPIO");
RTL_MUX(H, 3, "UART", "GPIO");

//RTK_MUX_13
RTL_MUX(G, 6, "LED", "PWM", "VP", "GPIO", "DBG");
RTL_MUX(G, 7, "LED", "PWM", "VP", "GPIO", "DBG");
RTL_MUX(H, 0, "LED", "PWM", "XOUT", "GPIO");
RTL_MUX(H, 1, "LED", "PWM", "GPIO", "DBG");

//RTK_MUX_14
RTL_MUX(H, 2, "LED", "MIIM", "GPIO", "DBG");

//RTK_MUX_15
RTL_MUX(D, 1, "MMC", "IIS", "UART1", "UART2", "NAND", "GPIO", "DBG");
RTL_MUX(D, 2, "MMC", "IIS", "UART1", "UART2", "NAND", "GPIO", "DBG");
RTL_MUX(D, 3, "MMC", "IIS", "UART1", "UART2", "NAND", "GPIO", "DBG");
RTL_MUX(D, 4, "MMC", "IIS", "UART1", "UART2", "NAND", "GPIO", "DBG");
RTL_MUX(D, 5, "MMC", "SPI1", "IISV", "IISA", "UART1", "PCM", "PWM", "GPIO", "DBG");
RTL_MUX(D, 6, "MMC", "SPI1", "IIS", "UART1", "PCM", "PWM", "GPIO", "DBG");
RTL_MUX(D, 7, "MMC", "SPI1", "I2C0", "I2C0_S", "UART1", "PCM", "PWM", "GPIO", "DBG");
RTL_MUX(E, 0, "MMC", "SPI1", "SPI1_3W", "I2C0", "UART1", "PCM", "PWM", "GPIO", "DBG");

//RTK_MUX_16
RTL_MUX(C, 4, "MMC", "SPI0", "GPIO", "DBG");
RTL_MUX(C, 5, "MMC", "SPI0", "SPI1", "IIS", "UART1", "EVENT", "GPIO", "DBG");
RTL_MUX(C, 6, "MMC", "SPI0", "SPI0_3W", "SPI1", "SPI1_3W", "IIS", "UART1", "EVENT", "GPIO", "DBG");
RTL_MUX(C, 7, "MMC", "SPI0", "SPI1", "IIS", "EVENT", "GPIO", "DBG");
RTL_MUX(D, 0, "MMC", "SPI0", "SPI1", "IIS", "EVENT", "GPIO", "DBG");

//RTK_MUX_17
RTL_MUX(E, 1, "WLAN", "GPIO", "DBG");
RTL_MUX(E, 2, "WLAN", "GPIO", "DBG");
RTL_MUX(E, 3, "WLAN", "GPIO", "DBG");
RTL_MUX(E, 4, "WLAN", "GPIO", "DBG");
RTL_MUX(E, 5, "WLAN", "GPIO", "DBG");
RTL_MUX(E, 6, "WLAN", "GPIO", "DBG");
RTL_MUX(E, 7, "WLAN", "GPIO", "DBG");
RTL_MUX(F, 0, "WLAN", "GPIO", "DBG");

//RTK_MUX_18
RTL_MUX(F, 1, "WLAN", "XOUT", "GPIO", "DBG");
RTL_MUX(F, 2, "WLAN", "SPI0", "IIS", "EVENT", "GPIO", "DBG");
RTL_MUX(F, 3, "WLAN", "SPI0", "IIS", "EVENT", "GPIO", "DBG");
RTL_MUX(F, 4, "WLAN", "SPI0", "IIS", "I2C1", "EVENT", "XOUT", "GPIO", "DBG");
RTL_MUX(F, 5, "WLAN", "SPI0", "SPI0_3W", "IIS", "I2C1", "EVENT", "GPIO", "DBG");

struct rtl_gpio rtk8197_gpio[] = {
	RTL_GPIO( 0, 0x1c, 28, A, 0),
	RTL_GPIO( 1, 0x18, 28, A, 1),
	RTL_GPIO( 2, 0x18, 24, A, 2),
	RTL_GPIO( 3, 0x1c, 16, A, 3),
	RTL_GPIO( 4, 0x1c, 24, A, 4),
	RTL_GPIO( 5, 0x1c, 20, A, 5),
	RTL_GPIO( 6, 0x00, 16, A, 6),
	RTL_GPIO( 7, 0x00, 20, A, 7),
	RTL_GPIO( 8, 0x00, 24, B, 0),
	RTL_GPIO( 9, 0x08, 16, B, 1),
	RTL_GPIO(10, 0x04, 16, B, 2),
	RTL_GPIO(11, 0x04, 20, B, 3),
	RTL_GPIO(12, 0x04, 24, B, 4),
	RTL_GPIO(13, 0x04, 28, B, 5),
	RTL_GPIO(14, 0x00, 28, B, 6),
	RTL_GPIO(15, 0x08, 24, B, 7),
	RTL_GPIO(16, 0x08, 20, C, 0),
	RTL_GPIO(17, 0x08, 12, C, 1),
	RTL_GPIO(18, 0x08,  8, C, 2),
	RTL_GPIO(19, 0x08,  4, C, 3),
	RTL_GPIO(20, 0x40, 16, C, 4),
	RTL_GPIO(21, 0x40, 12, C, 5),
	RTL_GPIO(22, 0x40,  8, C, 6),
	RTL_GPIO(23, 0x40,  4, C, 7),
	RTL_GPIO(24, 0x40,  0, D, 0),
	RTL_GPIO(25, 0x3c, 28, D, 1),
	RTL_GPIO(26, 0x3c, 24, D, 2),
	RTL_GPIO(27, 0x3c, 20, D, 3),
	RTL_GPIO(28, 0x3c, 16, D, 4),
	RTL_GPIO(29, 0x3c, 12, D, 5),
	RTL_GPIO(30, 0x3c,  8, D, 6),
	RTL_GPIO(31, 0x3c,  4, D, 7),
	RTL_GPIO(32, 0x3c,  0, E, 0),
	RTL_GPIO(33, 0x44, 28, E, 1),
	RTL_GPIO(34, 0x44, 24, E, 2),
	RTL_GPIO(35, 0x44, 20, E, 3),
	RTL_GPIO(36, 0x44, 16, E, 4),
	RTL_GPIO(37, 0x44, 12, E, 5),
	RTL_GPIO(38, 0x44,  8, E, 6),
	RTL_GPIO(39, 0x44,  4, E, 7),
	RTL_GPIO(40, 0x44,  0, F, 0),
	RTL_GPIO(41, 0x48, 28, F, 1),
	RTL_GPIO(42, 0x48, 24, F, 2),
	RTL_GPIO(43, 0x48, 20, F, 3),
	RTL_GPIO(44, 0x48, 16, F, 4),
	RTL_GPIO(45, 0x48, 12, F, 5),
	RTL_GPIO(46, 0x20, 24, F, 6),
	RTL_GPIO(47, 0x20, 28, F, 7),
	RTL_GPIO(48, 0x20, 20, G, 0),
	RTL_GPIO(49, 0x20, 16, G, 1),
	RTL_GPIO(50, 0x20, 12, G, 2),
	RTL_GPIO(51, 0x24, 28, G, 3),
	RTL_GPIO(52, 0x24, 24, G, 4),
	RTL_GPIO(53, 0x24, 20, G, 5),
	RTL_GPIO(54, 0x34, 28, G, 6),
	RTL_GPIO(55, 0x34, 24, G, 7),
	RTL_GPIO(56, 0x34, 20, H, 0),
	RTL_GPIO(57, 0x34, 16, H, 1),
	RTL_GPIO(58, 0x38, 28, H, 2),
	RTL_GPIO(59, 0x30, 24, H, 3),
	RTL_GPIO(60, 0x30, 28, H, 4),
	RTL_GPIO(61, 0x20,  0, H, 5),
};

struct reg_mapping rtk8197_regs_mapping[] =  {
	{ RTK8197_PINMUX_REG_BASE, 19, "PINMUX" },
	{ RTK8197_GPIO_REG_BASE, 14, "GPIO" },
};

const uint32_t *rtk8197_reg_defaults[] = {
	rtk8197_default_pinmux_register,
	rtk8197_default_gpio_register,
};

struct rtl_gpio *rtk8197_gpio_by_name(const char *name)
{
	return rtl_gpio_by_name(name, &rtk8197_gpio[0], ARRAY_SIZE(rtk8197_gpio));
}

int rtk8197_get_gpio_dir_word_shift(int gpio, uint32_t *word, uint8_t *shift)
{
	if ((gpio >= 0 ) && (gpio < RTK8197_GPIO_COUNT)) {
		*word = (RTK8197_GPIO_DIR(gpio)) >> 2;
		*shift = RTK8197_GPIO_SHIFT(gpio);
		return 0;
	}

	return -1;
}

int rtk8197_get_gpio_dat_word_shift(int gpio, uint32_t *word, uint8_t *shift)
{
	if ((gpio >= 0 ) && (gpio < RTK8197_GPIO_COUNT)) {
		*word = (RTK8197_GPIO_DAT(gpio)) >> 2;
		*shift = RTK8197_GPIO_SHIFT(gpio);
		return 0;
	}

	return -1;
}

int rtk8197_get_gpio_dir(uint32_t *regs, int gpio)
{
	uint32_t word;
	uint8_t shift;

	if (rtk8197_get_gpio_dir_word_shift(gpio, &word, &shift))
		return -1;

	return (regs[word] >> shift) & 0x1;
}

int rtk8197_set_gpio_dir(uint32_t * regs, int gpio, int dir)
{
	uint32_t word;
	uint8_t shift;

	if (rtk8197_get_gpio_dir_word_shift(gpio, &word, &shift))
		return -1;

	regs[word] &= ~(0x1 << shift); 
	regs[word] |= (dir << shift);

	return 0;
}

int rtk8197_get_gpio_val(uint32_t * regs, int gpio)
{
	uint32_t word;
	uint8_t shift;

	if (rtk8197_get_gpio_dat_word_shift(gpio , &word, &shift))
		return -1;

	return (regs[word] >> shift) & 0x1;
}

int rtk8197_set_gpio_val(uint32_t * regs, int gpio, int val)
{
	uint32_t word;
	uint8_t shift;

	if (rtk8197_get_gpio_dat_word_shift(gpio, &word, &shift))
		return -1;

	regs[word] &= ~(0x1 << shift);
	regs[word] |= val << shift;

	return 0;
}
#endif //RTK8197_DATA_H 
