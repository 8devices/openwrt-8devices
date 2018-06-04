/*
 * Realtek Semiconductor Corp.
 *
 * bsp/bspchip.h:
 *     bsp chip address and IRQ mapping file
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */

#ifndef _BSPCHIP_H_
#define _BSPCHIP_H_

#ifdef CONFIG_USE_UAPI
#include <generated/uapi/linux/version.h>
#else
#include <linux/version.h>
#endif

#define CONFIG_IRQ_ICTL
#define CONFIG_IRQ_GPIO

/*
 * Register access macro
 */
#ifndef REG32
#define REG32(reg)		(*(volatile unsigned int   *)(reg))
#endif
#ifndef REG16
#define REG16(reg)		(*(volatile unsigned short *)(reg))
#endif
#ifndef REG08
#define REG08(reg)		(*(volatile unsigned char  *)(reg))
#endif
#ifndef REG8
#define REG8(reg)		(*(volatile unsigned char  *)(reg))
#endif

#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *)   (addr)) = (val)
#define READ_MEM32(addr)         (*(volatile unsigned int *)   (addr))
#define WRITE_MEM16(addr, val)   (*(volatile unsigned short *) (addr)) = (val)
#define READ_MEM16(addr)         (*(volatile unsigned short *) (addr))
#define WRITE_MEM8(addr, val)    (*(volatile unsigned char *)  (addr)) = (val)
#define READ_MEM8(addr)          (*(volatile unsigned char *)  (addr))

/*
 * Clock Rate
 */
//#define BSP_CPU0_FREQ		25000000     /* 25 MHz */
//#define BSP_CPU0_FREQ		50000000     /* 50 MHz */
//#define BSP_CPU0_FREQ		66000000     /* 66 MHz */ // switch
//#define BSP_CPU0_FREQ		33868800     /* 33.8688 MHz */ // USB OTG
#define BSP_SPIC_FREQ		200000000	/* Sheipa SPI-nor 200 MHz */
#define BSP_DW_SPI_FREQ		100000000	/* 100 MHz */
#define BSP_DW_I2C_FREQ		100000000	/* 100MHz */

/*
 * IRQ Controller
 */
#define BSP_IRQ_CPU_BASE	0
#define BSP_IRQ_CPU_NUM		8

#define BSP_IRQ_ICTL_BASE	(BSP_IRQ_CPU_BASE + BSP_IRQ_CPU_NUM)
#define BSP_IRQ_ICTL_NUM	32

#define BSP_IRQ_ICTL_BASE2	(BSP_IRQ_ICTL_BASE + BSP_IRQ_ICTL_NUM)
#define BSP_IRQ_ICTL_NUM2	32

/* GPIO interrupt */
#define BSP_IRQ_GPIO_BASE		(BSP_IRQ_ICTL_BASE2 + BSP_IRQ_ICTL_NUM2)
#define BSP_GPIO_TO_IRQ(x)		(x + BSP_IRQ_GPIO_BASE)
#define BSP_IRQ_TO_GPIO(x)		(x - BSP_IRQ_GPIO_BASE)

/*
 * IRQ Mapping
 */
#define BSP_GIMR_TOCPU_MASK      (BSP_WLAN_MAC_IE|BSP_PCIE_IE|BSP_SW_IE)
#define BSP_GIMR_TOCPU_MASK2    (BSP_CPU_SI_TIMER_IE)
#define BSP_OTG_IRQ			(BSP_IRQ_ICTL_BASE +  12)
#define  BSP_USB_IRQ    			(BSP_IRQ_ICTL_BASE +  13)
#define BSP_WLAN_MAC_IRQ         (BSP_IRQ_CPU_BASE + 6)
#define BSP_PCIE_IRQ         		(BSP_IRQ_CPU_BASE + 5)
//#define BSP_PS_PCIE_IRQ_INTA	(BSP_IRQ_ICTL_BASE +  7)
//#define BSP_PS_PCIE_IRQ_INTB	(BSP_IRQ_ICTL_BASE +  8)
//#define BSP_PS_PCIE_IRQ_INTC	(BSP_IRQ_ICTL_BASE +  9)
//#define BSP_PS_PCIE_IRQ_INTD	(BSP_IRQ_ICTL_BASE + 10)
//#define BSP_PS_PCIE_IRQ_SII	(BSP_IRQ_ICTL_BASE + 11)
#define BSP_GPIO_PABCD_IRQ		(BSP_IRQ_ICTL_BASE + 16)
#define BSP_GPIO_PEFGH_IRQ		(BSP_IRQ_ICTL_BASE + 17)
//#define BSP_PS_RTC_IRQ		(BSP_IRQ_ICTL_BASE + 21)
#define BSP_DW_SSI_0_IRQ		(BSP_IRQ_ICTL_BASE + 23)
#define BSP_DW_SSI_1_IRQ		(BSP_IRQ_ICTL_BASE2 + 5)
#define  BSP_I2S_IRQ (BSP_IRQ_ICTL_BASE +  26)
#define BSP_DW_I2C_0_IRQ		(BSP_IRQ_ICTL_BASE + 30)
#define BSP_DW_I2C_1_IRQ		(BSP_IRQ_ICTL_BASE2 + 3)

//#define BSP_UART0_IRQ	(BSP_IRQ_CPU_BASE + 5)
#define BSP_UART0_IRQ	(BSP_IRQ_ICTL_BASE + 9)
#define BSP_UART1_IRQ   (BSP_IRQ_ICTL_BASE2 + 6)
#define BSP_UART2_IRQ	(BSP_IRQ_ICTL_BASE2 + 7)

//#define BSP_PS_SMC_IRQ		(BSP_IRQ_CPU_BASE + 2)

// for SW interrupt
//#define BSP_SWCORE_IRQ	(BSP_IRQ_ICTL_BASE + 15)
#define BSP_SWCORE_IRQ	(BSP_IRQ_CPU_BASE + 4)


/*
 * MIPS32R2 counter
 */
#define BSP_PERFCOUNT_IRQ	(BSP_IRQ_CPU_BASE + 7)
#define BSP_COMPARE_IRQ		(BSP_IRQ_CPU_BASE + 7)
#define BSP_ICTL_IRQ		(BSP_IRQ_CPU_BASE + 2)


/*
 * UART
 */
#define BSP_UART0_BAUD		38400
#ifdef CONFIG_RTK_FPGA
#define BSP_UART0_FREQ		(BSP_CPU0_FREQ/8) // switch
#define BSP_UART1_FREQ		(BSP_CPU1_FREQ/8) // switch
#define BSP_UART2_FREQ		(BSP_CPU1_FREQ/8) // switch
//#define BSP_UART0_FREQ		(BSP_CPU0_FREQ*12/10) // USB OTG, pcie
#else
#define BSP_UART0_FREQ		(100000000) // 100MHz
#define BSP_UART1_FREQ		(100000000) // 100MHz
#define BSP_UART2_FREQ		(100000000) // 100MHz
#endif
#define BSP_UART0_BAUD_DIVISOR  ((BSP_UART0_FREQ >> 4) / BSP_UART0_BAUD)

#define BSP_UART0_BASE		0xB8147000
#define BSP_UART0_MAP_BASE	0x18147000
#define BSP_UART0_MAPSIZE	0x100
#define BSP_UART0_RBR       (BSP_UART0_BASE + 0x024)
#define BSP_UART0_THR       (BSP_UART0_BASE + 0x024)
#define BSP_UART0_DLL       (BSP_UART0_BASE + 0x000)
#define BSP_UART0_IER       (BSP_UART0_BASE + 0x004)
#define BSP_UART0_DLM       (BSP_UART0_BASE + 0x004)
#define BSP_UART0_IIR       (BSP_UART0_BASE + 0x008)
#define BSP_UART0_FCR       (BSP_UART0_BASE + 0x008)
#define BSP_UART0_LCR       (BSP_UART0_BASE + 0x00C)
#define BSP_UART0_MCR       (BSP_UART0_BASE + 0x010)
#define BSP_UART0_LSR       (BSP_UART0_BASE + 0x014)
#define BSP_UART0_MSR       (BSP_UART0_BASE + 0x018)
#define BSP_UART0_SCR       (BSP_UART0_BASE + 0x01C)
#define BSP_UART0_STSR      (BSP_UART0_BASE + 0x020)
#define BSP_UART0_MISCR    (BSP_UART0_BASE + 0x028)
#define BSP_UART0_TXPLSR   (BSP_UART0_BASE + 0x02C)

#define BSP_UART1_BASE		0xB8147400
#define BSP_UART1_MAP_BASE	0x18147400
#define BSP_UART1_MAPSIZE	0x100
#define BSP_UART1_RBR       (BSP_UART1_BASE + 0x024)
#define BSP_UART1_THR       (BSP_UART1_BASE + 0x024)
#define BSP_UART1_DLL       (BSP_UART1_BASE + 0x000)
#define BSP_UART1_IER       (BSP_UART1_BASE + 0x004)
#define BSP_UART1_DLM       (BSP_UART1_BASE + 0x004)
#define BSP_UART1_IIR       (BSP_UART1_BASE + 0x008)
#define BSP_UART1_FCR       (BSP_UART1_BASE + 0x008)
   #define BSP_FCR_EN          0x01
   #define BSP_FCR_RXRST       0x02
   #define     BSP_RXRST             0x02
   #define BSP_FCR_TXRST       0x04
   #define     BSP_TXRST             0x04
   #define BSP_FCR_DMA         0x08
   #define BSP_FCR_RTRG        0xC0
   #define     BSP_CHAR_TRIGGER_01   0x00
   #define     BSP_CHAR_TRIGGER_04   0x40
   #define     BSP_CHAR_TRIGGER_08   0x80
   #define     BSP_CHAR_TRIGGER_14   0xC0
#define BSP_UART1_LCR       (BSP_UART1_BASE + 0x00C)
   #define BSP_LCR_WLS0        0x01
   #define     BSP_CHAR_LEN_7        0x00
   #define     BSP_CHAR_LEN_8        0x01
   #define BSP_LCR_STB         0x04
   #define     BSP_ONE_STOP          0x00
   #define     BSP_TWO_STOP          0x04
   #define BSP_LCR_PEN         0x08
   #define     BSP_PARITY_ENABLE     0x01
   #define     BSP_PARITY_DISABLE    0x00
   #define BSP_LCR_EPS         0x30
   #define     BSP_PARITY_ODD        0x00
   #define     BSP_PARITY_EVEN       0x10
   #define     BSP_PARITY_MARK       0x20
   #define     BSP_PARITY_SPACE      0x30
   #define BSP_LCR_BRK         0x40
   #define BSP_LCR_DLAB        0x80
   #define     BSP_DLAB              0x80
#define BSP_UART1_MCR       (BSP_UART1_BASE + 0x010)
#define BSP_UART1_LSR       (BSP_UART1_BASE + 0x014)
   #define BSP_LSR_DR          0x01
   #define     BSP_RxCHAR_AVAIL      0x01
   #define BSP_LSR_OE          0x02
   #define BSP_LSR_PE          0x04
   #define BSP_LSR_FE          0x08
   #define BSP_LSR_BI          0x10
   #define BSP_LSR_THRE        0x20
   #define     BSP_TxCHAR_AVAIL      0x00
   #define     BSP_TxCHAR_EMPTY      0x20
   #define BSP_LSR_TEMT        0x40
   #define BSP_LSR_RFE         0x80
#define BSP_UART1_MSR       (BSP_UART1_BASE + 0x018)
#define BSP_UART1_SCR       (BSP_UART1_BASE + 0x01C)
#define BSP_UART1_STSR      (BSP_UART1_BASE + 0x020)
#define BSP_UART1_MISCR    (BSP_UART1_BASE + 0x028)
#define BSP_UART1_TXPLSR   (BSP_UART1_BASE + 0x02C)

#define BSP_UART2_BASE          0xb8147800//0xB801E100
#define BSP_UART2_MAP_BASE      0x18147800//0x1801E100
#define BSP_UART2_MAPSIZE       0x400
#define BSP_UART2_RBR       (BSP_UART2_BASE + 0x024)
#define BSP_UART2_THR       (BSP_UART2_BASE + 0x024)
#define BSP_UART2_DLL       (BSP_UART2_BASE + 0x000)
#define BSP_UART2_IER       (BSP_UART2_BASE + 0x004)
#define BSP_UART2_DLM       (BSP_UART2_BASE + 0x004)
#define BSP_UART2_IIR       (BSP_UART2_BASE + 0x008)
#define BSP_UART2_FCR       (BSP_UART2_BASE + 0x008)
#define BSP_UART2_LCR       (BSP_UART2_BASE + 0x00C)
#define BSP_UART2_MCR       (BSP_UART2_BASE + 0x010)
#define BSP_UART2_LSR       (BSP_UART2_BASE + 0x014)
#define BSP_UART2_MSR       (BSP_UART2_BASE + 0x018)
#define BSP_UART2_SCR       (BSP_UART2_BASE + 0x01C)
#define BSP_UART2_STSR      (BSP_UART2_BASE + 0x020)
#define BSP_UART2_MISCR    (BSP_UART2_BASE + 0x028)
#define BSP_UART2_TXPLSR   (BSP_UART2_BASE + 0x02C)


/*
 * Interrupt Controller
 */

/*
---------------------------
GISR
Bit	Bit Name
31	GISR2_IP
30	DW_I2C_0_IP
39	WLAN_MAC_IP
28	USB0_WAKE_IP
27	CPU_WAKE_IP
26	I2S_IP
25	USB1_WAKE_IP
24	EFUSE_CTRL_IP
23	DW_SSI_0_IP
22	DHC_NAND_IP
21	PCIE0_IP
20	SECURITY_IP
19	PCM_IP
18	NFBI_IP
17	GPIO_EFGH_IP
16	GPIO_ABCD_IP
15	SW_IP
14	SD30_IP
13	USB_H_IP
12	USB_O_IP
11	DW_APB_TIMER_IP
10	TC3_IP
9	DW_UART_0_IP
8	DW_GDMA_IP
7	TC0_IP
6
5
4
3
2	POK33V_L_IP
1	OTG_CTRL_IP
0	NAND_CTRL_IP
----------------------------
GISR2
31	LX2_S_BTRDY_IP
30	LX1_S_BTRDY_IP
39	LX0_S_BTRDY_IP
28	LX2_BTRDY_IP
27	LX1_BTRDY_IP
26	LX0_BTRDY_IP
25	LX2_BFRAME_IP
24	LX1_BFRAME_IP
23	LX0_BFRAME_IP
22
21
20
19
18	DPI_DLL_IP
17	RXI300_IP
16	RXI310_SPIC_IP
15	CPU_SI_TIMER_IP
14	CPU_SI_PC_IP
13
12
11
10	SWR_DDR_OVER_LOAD_IP
9	Spi_flashecc_IP
8	Spi_nand_IP
7	DW_UART_2_IP
6	DW_UART_1_IP
5	DW_SSI_1_IP
4
3	DW_I2C_1_IP
2
1	TC2_IP
0	TC1_IP
---------------------------------
IRR0 IP#[07,06,05,04, 03,02,01,00]
IRR1 IP#[15,14,13,12, 11,10,09,08]
IRR2 IP#[23,22,21,20, 19,18,17,16]
IRR3 IP#[31,30,29,28, 27,26,25,24]


*/

#define SYSTEM_REG_BASE 	0xB8000000
#define BSP_HW_STRAP		(SYSTEM_REG_BASE + 0x8)
#define BSP_BOND_OPTION		(SYSTEM_REG_BASE + 0xC)
#define BSP_CLK_MANAGE1         (SYSTEM_REG_BASE + 0x0010)
#define BSP_CLK_MANAGE2		(SYSTEM_REG_BASE + 0x0014)
#define BSP_ENABLE_IP		(SYSTEM_REG_BASE + 0x0050)

#define BSP_GIMR            0xB8003000
    #define BSP_WLAN_MAC_IE     (1 << 29)
    #define BSP_PCIE_IE         (1 << 21)
    #define BSP_SW_IE           (1 << 15)
    #define BSP_UART0_IE        (1 << 9)
#define BSP_GISR            0xB8003004
#define BSP_IRR0            0xB8003008
#define BSP_IRR1            0xB800300C
#define BSP_IRR2            0xB8003010
#define BSP_IRR3            0xB8003014

#define BSP_GIMR2           0xB8003020
    #define BSP_CPU_SI_TIMER_IE     (1 << 15)
    #define BSP_CPU_SI_PC_IE           (1 << 14)
#define BSP_GISR2           0xB8003024
#define BSP_IRR4            0xB8003028
#define BSP_IRR5            0xB800302C
#define BSP_IRR6            0xB8003030
#define BSP_IRR7            0xB8003034

#define IRR_IPtoCPU(a7,a6,a5,a4,a3,a2,a1,a0) ((a7<<28)|(a6<<24)|(a5<<20)|(a4<<16)|(a3<<12)|(a2<<8)|(a1<<4)|(a0<<0))

#define BSP_IRR0_SETTING    IRR_IPtoCPU(0,0,0,0, 0,0,0,0)   //IP [7~0]
#define BSP_IRR1_SETTING    IRR_IPtoCPU(4,2,2,2, 0,0,2,0)	//IP [15~8]
#define BSP_IRR2_SETTING    IRR_IPtoCPU(2,0,5,0, 0,0,2,2)	//IP [23~16]
#define BSP_IRR3_SETTING    IRR_IPtoCPU(0,2,6,0, 0,2,0,0)	//IP [31~24]

#if defined(CONFIG_SERIAL_RTL_UART1)
#define UART1_Status 2
#else
#define UART1_Status 0
#endif

#if defined(CONFIG_SERIAL_RTL_UART2)
#define UART2_Status 2
#else
#define UART2_Status 0
#endif

#define BSP_IRR4_SETTING    IRR_IPtoCPU(UART2_Status,UART1_Status,2,0, 2,0,0,0) //UART2 7
#define BSP_IRR5_SETTING    IRR_IPtoCPU(7,0,0,0, 0,0,0,0)
#define BSP_IRR6_SETTING    IRR_IPtoCPU(0,0,0,0, 0,0,0,0)
#define BSP_IRR7_SETTING    IRR_IPtoCPU(0,0,0,0, 0,0,0,0)

/*
 * DWAPB Timer
 */
#define BSP_TIMER0_FREQ		25000000     /* 25 MHz */
#define BSP_TIMER0_BASE		0xbf201000UL
#define BSP_TIMER0_TLCR		(BSP_TIMER0_BASE + 0x00)
#define BSP_TIMER0_TCVR		(BSP_TIMER0_BASE + 0x04)
#define BSP_TIMER0_TCR		(BSP_TIMER0_BASE + 0x08)   /* timer control register */
#define BSP_TIMER0_EOI		(BSP_TIMER0_BASE + 0x0c)
#define BSP_TIMER0_IRQ		(BSP_IRQ_CPU_BASE + 6)

#define BSP_RTC_BASE		0xbfb00000UL
#define BSP_IPC_BASE		0xbfb05000UL
#define BSP_PMU_BASE		0xbfb06000UL
#define BSP_I2C_BASE		0xbfb0f000UL

/*
 * Timer/Counter
 */
#define BSP_TC_BASE         0xB8003100
#define BSP_TC0DATA         (BSP_TC_BASE + 0x00)
#define BSP_TC1DATA         (BSP_TC_BASE + 0x04)
   #define BSP_TCD_OFFSET      4
#define BSP_TC0CNT          (BSP_TC_BASE + 0x08)
#define BSP_TC1CNT          (BSP_TC_BASE + 0x0C)
#define BSP_TCCNR           (BSP_TC_BASE + 0x10)
   #define BSP_TC0EN           (1 << 31)
   #define BSP_TC0MODE_TIMER   (1 << 30)
   #define BSP_TC1EN           (1 << 29)
   #define BSP_TC1MODE_TIMER   (1 << 28)
#define BSP_TCIR            (BSP_TC_BASE + 0x14)
   #define BSP_TC0IE           (1 << 31)
   #define BSP_TC1IE           (1 << 30)
   #define BSP_TC0IP           (1 << 29)
   #define BSP_TC1IP           (1 << 28)
#define BSP_CDBR            (BSP_TC_BASE + 0x18)
   #define BSP_DIVF_OFFSET     16
#define BSP_WDTCNR          (BSP_TC_BASE + 0x1C)

#define BSP_DIVISOR         400

#if BSP_DIVISOR > (1 << 16)
#error "Exceed the Maximum Value of DivFactor"
#endif

/*
 * Wifi
 */
#define BSP_WLAN_BASE_ADDR      0xB8640000UL
#define BSP_WLAN_CONF_ADDR      0x00000000UL //for compiler's happy

/*
 * PCI-E
 */
#define BSP_PCIE_RC_CFG		0xb8b00000UL
#define BSP_PCIE_EP_CFG		0xb8b10000UL
#define BSP_PCIE_IO_BASE	0xb8c00000UL
#define BSP_PCIE_IO_SIZE	0x00200000UL
//#define BSP_PCIE_IO_MAPBASE	0x19200000UL
//#define BSP_PCIE_IO_MAPSIZE	0x00200000UL
#define BSP_PCIE_MEM_BASE	0xb9000000UL
#define BSP_PCIE_MEM_SIZE	0x00c00000UL
//#define BSP_PCIE_MEM_MAPBASE	0x19400000UL
//#define BSP_PCIE_MEM_MAPSIZE	0x00c00000UL

#define BSP_PCIE_RC_EXTENDED_REG	0xb8b01000

#define BSP_PCIE0_H_CFG     0xB8B00000
#define BSP_PCIE0_H_EXT     0xB8B01000
#define BSP_PCIE0_H_MDIO    (BSP_PCIE0_H_EXT + 0x00)
#define BSP_PCIE0_H_INTSTR  (BSP_PCIE0_H_EXT + 0x04)
#define BSP_PCIE0_H_PWRCR   (BSP_PCIE0_H_EXT + 0x08)
#define BSP_PCIE0_H_IPCFG   (BSP_PCIE0_H_EXT + 0x0C)
#define BSP_PCIE0_H_MISC    (BSP_PCIE0_H_EXT + 0x10)
#define BSP_PCIE0_D_CFG0    0xB8B10000
#define BSP_PCIE0_D_MEM     0xB9000000

#define BSP_PCIE1_H_CFG   0x00000000 //for compiler's happy
#define BSP_PCIE1_H_PWRCR 0x00000000 //for compiler's happy
#define BSP_PCIE1_D_CFG0  0x00000000 //for compiler's happy

/*
 * USB
 */
#define BSP_USB_USB3_BASE	0xb8000000UL
#define BSP_USB_USB3_SIZE	0x00100000UL
#define BSP_USB_USB3_MAPBASE	0x18000000UL
#define BSP_USB_USB3_MAPSIZE	0x00100000UL
#define BSP_USB_OTG_BASE	0xb8100000UL
#define BSP_USB_OTG_SIZE	0x00040000UL
#define BSP_USB_OTG_MAPBASE	0x18100000UL
#define BSP_USB_OTG_MAPSIZE	0x00040000UL

/*
 * SPIC
 */
#define BSP_SPIC_BASE		0x18143000UL    // 0xB8143000UL
#define BSP_SPIC_SIZE		0x00001000UL

#define BSP_SPIC_AUTO_BASE	0x10000000UL    // 0xB0000000
#define BSP_SPIC_AUTO_SIZE	0x02000000UL

/*
 * DW_SSI
 */
#define BSP_DW_SSI_0_BASE			0x1801C000UL
#define BSP_DW_SSI_0_SIZE			0x00000100UL
#define BSP_DW_SSI_1_BASE			0x1801C100UL
#define BSP_DW_SSI_1_SIZE			0x00000100UL

/*
 * DW_I2C
 */
#define BSP_DW_I2C_0_BASE		0x1801D000UL
#define BSP_DW_I2C_0_SIZE		0x00000100UL
#define BSP_DW_I2C_1_BASE		0x1801D100UL
#define BSP_DW_I2C_1_SIZE		0x00000100UL

/*
 * PIN MUX
 */
#define BSP_PIN_MUX_SEL0			0xB8000800UL
#define BSP_PIN_MUX_SEL1			0xB8000804UL
#define BSP_PIN_MUX_SEL2			0xB8000808UL
#define BSP_PIN_MUX_SEL3			0xB800080CUL
#define BSP_PIN_MUX_SEL4			0xB8000810UL
#define BSP_PIN_MUX_SEL5			0xB8000814UL
#define BSP_PIN_MUX_SEL6			0xB8000818UL
#define BSP_PIN_MUX_SEL7			0xB800081CUL
#define BSP_PIN_MUX_SEL8			0xB8000820UL
#define BSP_PIN_MUX_SEL9			0xB8000824UL
#define BSP_PIN_MUX_SEL10			0xB8000828UL
#define BSP_PIN_MUX_SEL11			0xB800082CUL
#define BSP_PIN_MUX_SEL12			0xB8000830UL
#define BSP_PIN_MUX_SEL13			0xB8000834UL
#define BSP_PIN_MUX_SEL14			0xB8000838UL
#define BSP_PIN_MUX_SEL15			0xB800083CUL
#define BSP_PIN_MUX_SEL16			0xB8000840UL
#define BSP_PIN_MUX_SEL17			0xB8000844UL
#define BSP_PIN_MUX_SEL18			0xB8000848UL

/*
 * GPIO Register Set
 */
#define BSP_GPIO_BASE				(0xB8003500UL)
#define BSP_PABCD_CNR				(BSP_GPIO_BASE + 0x00)		/* Port ABCD control */
#define BSP_PABCD_PTYPE				(BSP_GPIO_BASE + 0x04)		/* Port ABCD type */
#define BSP_PABCD_DIR				(BSP_GPIO_BASE + 0x08)		/* Port ABCD direction */
#define BSP_PABCD_DAT				(BSP_GPIO_BASE + 0x0C)		/* Port ABCD data */
#define BSP_PABCD_ISR				(BSP_GPIO_BASE + 0x10)		/* Port ABCD interrupt status */
#define BSP_PAB_IMR					(BSP_GPIO_BASE + 0x14)		/* Port AB interrupt mask */
#define BSP_PCD_IMR					(BSP_GPIO_BASE + 0x18)		/* Port CD interrupt mask */
#define BSP_PEFGH_CNR				(BSP_GPIO_BASE + 0x1C)		/* Port EFGH control */
#define BSP_PEFGH_PTYPE				(BSP_GPIO_BASE + 0x20)		/* Port EFGH type */
#define BSP_PEFGH_DIR				(BSP_GPIO_BASE + 0x24)		/* Port EFGH direction */
#define BSP_PEFGH_DAT				(BSP_GPIO_BASE + 0x28)		/* Port EFGH data */
#define BSP_PEFGH_ISR				(BSP_GPIO_BASE + 0x2C)		/* Port EFGH interrupt status */
#define BSP_PEF_IMR					(BSP_GPIO_BASE + 0x30)		/* Port EF interrupt mask */
#define BSP_PGH_IMR					(BSP_GPIO_BASE + 0x34)		/* Port GH interrupt mask */

/*
 * GPIO PIN
 */
enum BSP_GPIO_PIN
{
	BSP_GPIO_PIN_A0 = 0,
	BSP_GPIO_PIN_A1,
	BSP_GPIO_PIN_A2,
	BSP_GPIO_PIN_A3,
	BSP_GPIO_PIN_A4,
	BSP_GPIO_PIN_A5,
	BSP_GPIO_PIN_A6,
	BSP_GPIO_PIN_A7,

	BSP_GPIO_PIN_B0,
	BSP_GPIO_PIN_B1,
	BSP_GPIO_PIN_B2,
	BSP_GPIO_PIN_B3,
	BSP_GPIO_PIN_B4,
	BSP_GPIO_PIN_B5,
	BSP_GPIO_PIN_B6,
	BSP_GPIO_PIN_B7,

	BSP_GPIO_PCD_REG,
	BSP_GPIO_PIN_C0 = BSP_GPIO_PCD_REG,
	BSP_GPIO_PIN_C1,
	BSP_GPIO_PIN_C2,
	BSP_GPIO_PIN_C3,
	BSP_GPIO_PIN_C4,
	BSP_GPIO_PIN_C5,
	BSP_GPIO_PIN_C6,
	BSP_GPIO_PIN_C7,

	BSP_GPIO_PIN_D0,
	BSP_GPIO_PIN_D1,
	BSP_GPIO_PIN_D2,
	BSP_GPIO_PIN_D3,
	BSP_GPIO_PIN_D4,
	BSP_GPIO_PIN_D5,
	BSP_GPIO_PIN_D6,
	BSP_GPIO_PIN_D7,

	BSP_GPIO_2ND_REG,
	BSP_GPIO_PIN_E0 = BSP_GPIO_2ND_REG,
	BSP_GPIO_PIN_E1,
	BSP_GPIO_PIN_E2,
	BSP_GPIO_PIN_E3,
	BSP_GPIO_PIN_E4,
	BSP_GPIO_PIN_E5,
	BSP_GPIO_PIN_E6,
	BSP_GPIO_PIN_E7,

	BSP_GPIO_PIN_F0,
	BSP_GPIO_PIN_F1,
	BSP_GPIO_PIN_F2,
	BSP_GPIO_PIN_F3,
	BSP_GPIO_PIN_F4,
	BSP_GPIO_PIN_F5,
	BSP_GPIO_PIN_F6,
	BSP_GPIO_PIN_F7,

	BSP_GPIO_PGH_REG,
	BSP_GPIO_PIN_G0 = BSP_GPIO_PGH_REG,
	BSP_GPIO_PIN_G1,
	BSP_GPIO_PIN_G2,
	BSP_GPIO_PIN_G3,
	BSP_GPIO_PIN_G4,
	BSP_GPIO_PIN_G5,
	BSP_GPIO_PIN_G6,
	BSP_GPIO_PIN_G7,

	BSP_GPIO_PIN_H0,
	BSP_GPIO_PIN_H1,
	BSP_GPIO_PIN_H2,
	BSP_GPIO_PIN_H3,
	BSP_GPIO_PIN_H4,
	BSP_GPIO_PIN_H5,
	BSP_GPIO_PIN_H6,
	BSP_GPIO_PIN_H7,
	BSP_GPIO_PIN_MAX,
	BSP_UART1_PIN,
	BSP_UART2_PIN,
	TOTAL_PIN_MAX
};

#define BSP_GPIO_BIT(pin)  		(pin & ~(BSP_GPIO_2ND_REG))
#define BSP_GPIO_2BITS(pin)  	((pin & ~(BSP_GPIO_PCD_REG | BSP_GPIO_2ND_REG | BSP_GPIO_PGH_REG)) * 2)
#define BSP_GPIO_CNR_REG(pin) 	((pin & BSP_GPIO_2ND_REG) ? BSP_PEFGH_CNR : BSP_PABCD_CNR)
#define BSP_GPIO_DIR_REG(pin) 	((pin & BSP_GPIO_2ND_REG) ? BSP_PEFGH_DIR : BSP_PABCD_DIR)
#define BSP_GPIO_DAT_REG(pin) 	((pin & BSP_GPIO_2ND_REG) ? BSP_PEFGH_DAT : BSP_PABCD_DAT)
#define BSP_GPIO_ISR_REG(pin)	((pin & BSP_GPIO_2ND_REG) ? BSP_PEFGH_ISR : BSP_PABCD_ISR)
#define BSP_GPIO_IMR_REG(pin)	((pin & BSP_GPIO_2ND_REG) ? \
										((pin & BSP_GPIO_PGH_REG) ? BSP_PGH_IMR : BSP_PEF_IMR) : \
										((pin & BSP_GPIO_PCD_REG) ? BSP_PCD_IMR : BSP_PAB_IMR))

/*
 * Bonding Option
 */
#define BSP_BOND_97FB	1
#define BSP_BOND_97FN	2
#define BSP_BOND_97FS	3

#endif   /* _BSPCHIP_H */
