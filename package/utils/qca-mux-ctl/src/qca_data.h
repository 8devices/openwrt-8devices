#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

const uint32_t default_regs[] = {
        0x002CB309,
        0x209E9630,
        0x00820039,
        0x00000000,

        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,

        0x00000000,
        0x00000000,
    	0x00008402,
	0x00000000,

        0x0C080980,
        0x00160000,
        0x00000000,
        0x20000000,

        0x00000021,
        0x00000908,
        0x80808080,
        0x80808080,

        0x00138080,
        0x80808080,
        0x80808080,
        0x80808080,

        0x80808080,
        0x80808080,
        0x80808080,
        0x00000080
};


const char *rambutan_reg_names[] = {
        "GPIO_OE",
        "GPIO_IN",
        "GPIO_OUT",
        "GPIO_SET",
        "GPIO_CLEAR",
        "GPIO_INT",
        "GPIO_INT_TYPE",
        "GPIO_INT_POLARITY",
        "GPIO_INT_PENDING",
        "GPIO_INT_MASK",
        "GPIO_SPARE",
        "GPIO_OUT_FUNCTION0",
        "GPIO_OUT_FUNCTION1",
        "GPIO_OUT_FUNCTION2",
        "GPIO_OUT_FUNCTION3",
        "GPIO_OUT_FUNCTION4",
        "GPIO_OUT_FUNCTION5",
        "GPIO_IN_ENABLE0",
        "GPIO_IN_ENABLE1",
        "GPIO_IN_ENABLE2",
        "GPIO_IN_ENABLE3",
        "GPIO_IN_ENABLE4",
        "RESERVED",
        "RESERVED",
        "RESERVED",
        "RESERVED",
        "GPIO_IN_ENABLE9",
        "GPIO_FUNCTION"
};

const char *carambola_reg_names[] = {
        "GPIO_OE",
        "GPIO_IN",
        "GPIO_OUT",
        "GPIO_SET",
        "GPIO_CLEAR",
        "GPIO_INT",
        "GPIO_INT_TYPE",
        "GPIO_INT_POLARITY",
        "GPIO_INT_PENDING",
        "GPIO_INT_MASK",
        "GPIO_FUNCTION_1",
	"GPIO_IN_ETH_SWITCH_LED",
	"GPIO_FUNCTION_2"
};

struct gpio_func_name {
        uint32_t id;
        const char *name;
};


struct gpio_func_name rambutan_func_names[] = {
        {0, "GPIO",},
        {1, "MII_EXT_MDI",},
        {2, "RES",},
        {3, "SLIC_DATA_OUT",},
        {4, "SLIC_PCM_FS",},
        {5, "SLIC_PCM_CLK",},
        {6, "I2C_DATA",},
        {7, "I2C_CLK",},
        {8, "SPI_CLK",},
        {9, "SPI_CS_0",},
        {10, "SPI_CS_1",},
        {11, "SPI_CS_2",},
        {12, "SPI_MOSI",},
        {13, "I2S_CLK",},
        {14, "I2S_WS",},
        {15, "I2S_SD",},
        {16, "I2S_MCK",},
        {17, "SPDIF_OUT",},
        {18, "UART1_TD",},
        {19, "UART1_RTS",},
        {20, "UART1_RD ",},
        {21, "UART1_CTS",},
        {22, "UART0_SOUT",},
        {23, "SPDIF_OUT",},
        {24, "LED_SGMII_SPEED0",},
        {25, "LED_SGMII_SPEED1",},
        {26, "LED_SGMII_DUPLEX",},
        {27, "LED_SGMII_LINK_UP",},
        {28, "LED_SGMII_SPEED0_INVERT",},
        {29, "LED_SGMII_SPEED1_INVERT",},
        {30, "LED_SGMII_DUPLEX_INVERT",},
        {31, "LED_SGMII_LINK_UP_INVERT",},
        {32, "GE1_MII_MDO",},
        {33, "GE1_MII_MDC",},

        {38, "SWCOM2",},
        {39, "SWCOM3",},
        {40, "SMART_ANT_CTL_2",},
        {41, "SMART_ANT_CTL_3",},
        {42, "ATT_LED",},
        {43, "PWR_LED",},
        {44, "TX_FRAME",},
        {45, "RX_CLEAR_EXTERNAL",},
        {46, "LED_NETWORK_EN",},
        {47, "LED_POWER_EN",},

        {68, "WMAC_GLUE_WOW",},
        {70, "RX_CLEAR_EXTENSION",},
        {71, "SHIFT_STROBE",},
        {72, "SHIFT_DATA",},
        {73, "CP_NAND_CS1",},
        {74, "USB_SUSPEND",},
        {75, "ETH_TX_ERR",},
        {76, "A DDR_DQ_OE",},
        {77, "CLKREQ_N_EP",},
        {78, "CLKREQ_N_RC",},
        {79, "CLK_OBS0",},
        {80, "CLK_OBS1",},
        {81, "CLK_OBS2",},
        {82, "CLK_OBS3",},
        {83, "CLK_OBS4",},
        {84, "CLK_OBS5",},
};


struct gpio_func_name lima_func_names[] = {
        {0, "GPIO"},
        {1, "SYS_RST_L"},

        {8, "SPI_CLK"},
        {9, "SPI_CS_0"},
        {10,"SPI_CS_1"},
        {11, "SPI_CS_2"},
        {12, "SPI_MOSI"},

        {22, "UART0_SOUT"},
        {23, "SRIF_OUT"},
        {26, "LED_ACTN_0"},
        {27, "LED_ACTN_1"},
        {28, "LED_ACTN_2"},
        {29, "LED_ACTN_3"},
        {30, "LED_ACTN_4"},
        {31, "LED_COLN_0"},
        {32, "LED_COLN_1"},
        {33, "LED_COLN_2"},
        {34, "LED_COLN_3"},
        {35, "LED_COLN_4"},
        {36, "LED_DUPLEXN_0"},
        {37, "LED_DUPLEXN_1"},
        {38, "LED_DUPLEXN_2"},
        {39, "LED_DUPLEXN_3"},
        {40, "LED_DUPLEXN_4"},
        {41, "LED_LINK_0"},
        {42, "LED_LINK_1"},
        {43, "LED_LINK_2"},
        {44, "LED_LINK_3"},
        {45, "LED_LINK_4"},

        {48, "SMART_ANT_CTL_2"},
        {49, "SMART_ANT_CTL_3"},
        {50, "ATT_LED"},
        {51, "PWR_LED"},
        {52, "TX_FRAME"},
        {53, "RX_CLEAR_INTERNAL"},
        {54, "LED_NETWORK_EN"},
        {55, "LED_POWER_EN"},

        {78, "RX_CLEAR_EXTENSION"},

        {86, "USB_SUSPEND"},

        {88, "DDR_DQ_OE"},
        {89, "CLKREQ_N_RC"},
        {90, "CLK_OBS0"},
        {91, "CLK_OBS1"},
        {92, "CLK_OBS2"},
        {93, "CLK_OBS3"},
        {94, "CLK_OBS4"},
        {95, "CLK_OBS5"},
        {96, "CLK_OBS6"},
};

struct gpio_input_reg {
        uint32_t reg;
        uint32_t offset;
        const char *name;
};


struct gpio_input_reg rambutan_gpio_input_func[] = {
        {0x44, 0, "SPI_DATA_IN"},
        {0x44, 8, "UART0_SIN"},

        {0x48, 0, "I2SEXT_MCLK"},
        {0x48, 8, "I2SEXTCLK"},
        {0x48, 16, "I2S0_MIC_SD"},
        {0x48, 24, "I2S0_WS"},

        {0x4c, 0, "ETH_RX_ERR"},
        {0x4c, 8, "ETH_RX_COL"},
        {0x4c, 16, "ETH_RX_CRS"},
        {0x4c, 24, "SLICEXT_MCLK"},

        {0x50, 0, "BOOT_EXT_MDO"},
        {0x50, 8, "BOOT_EXT_MDC"},
        {0x50, 16, "MII_GE1_MDI"},

        {0x54, 0, "SLIC_DATA_IN"},
        {0x54, 8, "SLIC_PCM_FS_IN"},
        {0x54, 16, "I2C_CLK"},
        {0x54, 24, "I2C_DATA"},

        {0x68, 0, "UART1_TD"},
        {0x68, 8, "UART1_RTS"},
        {0x68, 16, "UART1_RD"},
        {0x68, 24, "UART1_CTS"},
};

struct gpio_input_reg lima_gpio_input_func[] = {
        {0x44, 0, "SPI_DATA_IN"},
        {0x44, 8, "UART0_SIN"},
};


struct function_reg {
	uint32_t reg;
	uint32_t offset;
	const char *name;
};


struct function_reg carambola_gpio_function[] = {
	{0x28, 0, "EJTAG_DISABLE"},
	{0x28, 1, "UART_EN"},
	{0x28, 2, "UART_RTS_CTS_EN"},
	{0x28, 3, "ETH_SWITCH_LED0_EN"},
	{0x28, 4, "ETH_SWITCH_LED1_EN"},
	{0x28, 5, "ETH_SWITCH_LED2_EN"},
	{0x28, 6, "ETH_SWITCH_LED3_EN"},
	{0x28, 7, "ETH_SWITCH_LED4_EN"},

	{0x28, 13, "SPI_CS_EN1"},
	{0x28, 14, "SPI_CS_EN2"},

	{0x28, 18, "SPI_EN"},
	
	{0x28, 23, "ETH_SWITCH_LED_ACTV"},
	{0x28, 24, "ETH_SWITCH_LED_COLL"},
	{0x28, 25, "ETH_SWITCH_LED_DUPL"},
	{0x28, 26, "I2SO_EN"},
	{0x28, 27, "I2S_MCKEN"},

	{0x28, 29, "I2SO_22_18_EN"},
	{0x28, 30, "SPDIF_EN"},
	{0x28, 31, "SPDIF2TCK"},

	{0x30, 0, "DIS_MIC"},
	{0x30, 1, "I2S_ON_LED"},
	{0x30, 2, "SPDIF_ON23"},
	{0x30, 3, "EN_I2SCK_ON_1"},
	{0x30, 4, "EN_I2SWS_ON_0"},
	{0x30, 5, "I2SD_ON_12"},
		
	{0x30, 8, "WPS_DISABLE"},
	{0x30, 9, "JUMPSTART_DISABLE"},
	{0x30, 10, "WLAN_LED1_EN"},
	{0x30, 11, "WLAN_LED2_EN"},
	{0x30, 12, "XLNA_EN"},
	
	{0x30, 16, "MDIO_SLV_PHY_ADR"},
};

#define FUNC_FLAG_INVERTED		0x1
struct gpio_function {
	int gpio;
	const char *name;
	const char *en_func_name;
	int flags;
};

struct gpio_function func_from_enfunc[] = {
	{0, "I2S_WS", "EN_I2SWS_ON_0"},
	{1, "I2S_CK", "EN_I2SCK_ON_1"},
	{0, "WLAN_LED1", "WLAN_LED1_EN"},
	{1, "WLAN_LED2", "WLAN_LED2_EN"},	
	{2, "SPI_CS0", "SPI_EN"},
	{3, "SPI_CLK", "SPI_EN"},
	{4, "SPI_MOSI", "SPI_EN"},
	{5, "SPI_MISO", "SPI_EN"},
	{6, "JTAG_TDI", "EJTAG_DISABLE", FUNC_FLAG_INVERTED},
	{7, "JTAG_TDO", "EJTAG_DISABLE", FUNC_FLAG_INVERTED},
	{8, "JTAG_TMS", "EJTAG_DISABLE", FUNC_FLAG_INVERTED},
	{10, "SPI_CS2", "SPI_CS_EN2"},
	{9, "SPI_CS1", "SPI_CS_EN1"},
	{9, "UART_SIN", "UART_EN"},
	{10, "UART_SOUT", "UART_EN"},
	{12, "UART_RTS", "UART_RTS_CTS_EN"},
	{11, "UART_CTS", "UART_RTS_CTS_EN"},
	{12, "I2SD", "I2SD_ON_12"},
	{13, "ETH_SWITCH_LED0", "ETH_SWITCH_LED0_EN"},
	{14, "ETH_SWITCH_LED1", "ETH_SWITCH_LED1_EN"},
	{15, "ETH_SWITCH_LED2", "ETH_SWITCH_LED2_EN"},
	{16, "ETH_SWITCH_LED3", "ETH_SWITCH_LED3_EN"},
	{17, "ETH_SWITCH_LED4", "ETH_SWITCH_LED4_EN"},
	{14, "I2S_ON_LED", "I2S_ON_LED"},
	{15, "I2S_ON_LED", "I2S_ON_LED"},
	{16, "I2S_ON_LED", "I2S_ON_LED"},
	{18, "BITCLK", "I2SO_22_18_EN"},
	{19, "WS", "I2SO_22_18_EN"},
	{20, "SD", "I2SO_22_18_EN"},
	{21, "MCK", "I2SO_22_18_EN"},
	{22, "MICIN", "I2SO_22_18_EN"},
	{23, "SPDIF", "SPDIF_ON23"},
	{28, "LNA", "XLNA_EN"}

};

struct function_reg qca95xx_gpio_function[] = {
	{0x6C, 1, "DISABLE_JTAG"},
};

struct gpio_function qca95xx_func_from_enfunc[] = {
	{0, "JTAG_TCK", "DISABLE_JTAG", FUNC_FLAG_INVERTED},
	{1, "JTAG_TDI", "DISABLE_JTAG", FUNC_FLAG_INVERTED},
	{2, "JTAG_TDO", "DISABLE_JTAG", FUNC_FLAG_INVERTED},
	{3, "JTAG_TMS", "DISABLE_JTAG", FUNC_FLAG_INVERTED}
};

struct device_in_out_func {
	struct gpio_input_reg *input_signals;
	struct gpio_func_name *output_signals;
	struct gpio_function *gpio_function_groups;
	struct function_reg *gpio_functions;
	int reg_size;
	int reg_count;
	int input_signal_count;
	int output_signal_count;
	int gpio_function_group_count;
	int gpio_function_count;
	int gpio_count;
	const char **reg_names;
};

struct device_in_out_func dev_rambutan = {
	.input_signals			= rambutan_gpio_input_func,
	.output_signals			= rambutan_func_names,
	.gpio_function_groups		= qca95xx_func_from_enfunc,
	.gpio_functions			= qca95xx_gpio_function,
	.reg_size			= 0x70,
	.reg_count			= 28,
	.input_signal_count		= ARRAY_SIZE(rambutan_gpio_input_func),
	.output_signal_count		= ARRAY_SIZE(rambutan_func_names),
	.gpio_function_group_count	= ARRAY_SIZE(qca95xx_func_from_enfunc),
	.gpio_function_count		= ARRAY_SIZE(qca95xx_gpio_function),
	.gpio_count			= 24,
	.reg_names			= rambutan_reg_names
};

struct device_in_out_func dev_lima = {
	.input_signals			= lima_gpio_input_func,
	.output_signals			= lima_func_names,
	.gpio_function_groups		= qca95xx_func_from_enfunc,
	.gpio_functions			= qca95xx_gpio_function,
	.reg_size			= 0x70,
	.reg_count			= 28,
	.input_signal_count		= ARRAY_SIZE(lima_gpio_input_func),
	.output_signal_count		= ARRAY_SIZE(lima_func_names),
	.gpio_function_group_count	= ARRAY_SIZE(qca95xx_func_from_enfunc),
	.gpio_function_count		= ARRAY_SIZE(qca95xx_gpio_function),
	.gpio_count			= 18,
	.reg_names			= rambutan_reg_names
};

struct device_in_out_func dev_carambola = {
	.input_signals			= NULL,
	.output_signals			= NULL,
	.gpio_function_groups		= func_from_enfunc,
	.gpio_functions			= carambola_gpio_function,
	.reg_size			= 0x34,
	.reg_count			= 13,
	.input_signal_count		= 0,
	.output_signal_count		= 0,
	.gpio_function_group_count	= ARRAY_SIZE(func_from_enfunc),
	.gpio_function_count		= ARRAY_SIZE(carambola_gpio_function),
	.gpio_count			= 29,
	.reg_names			= carambola_reg_names
};
