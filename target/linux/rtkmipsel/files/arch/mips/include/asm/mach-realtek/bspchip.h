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
#include <linux/kernel_stat.h> //mark_bb
#include <asm/gic.h>


//wei del
//#define CONFIG_IRQ_ICTL

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


#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *)   (addr)) = (val)
#define READ_MEM32(addr)         (*(volatile unsigned int *)   (addr))
#define WRITE_MEM16(addr, val)   (*(volatile unsigned short *) (addr)) = (val)
#define READ_MEM16(addr)         (*(volatile unsigned short *) (addr))
#define WRITE_MEM8(addr, val)    (*(volatile unsigned char *)  (addr)) = (val)
#define READ_MEM8(addr)          (*(volatile unsigned char *)  (addr))

#define PADDR(addr)  ((addr) & 0x1FFFFFFF)
/*
 * SPRAM
 */
#define BSP_ISPRAM_BASE		0x0
#define BSP_DSPRAM_BASE		0x0

/*
 * GCMP Specific definitions
 */
#define GCMP_BASE_ADDR		0x1fbf8000
#define GCMP_BASE_SIZE		(256 * 1024)

/*
 * GIC Specific definitions
 */
#define GIC_BASE_ADDR		0x1bdc0000
#define GIC_BASE_SIZE		(128 * 1024)

/*
 * CPC Specific defiitions
 */
#define CPC_BASE_ADDR		0x1bde0000
#define CPC_BASE_SIZE		(24 * 1024)

/*
 * IRQ Controller
 */
#define BSP_IRQ_CPU_BASE	0
#define BSP_IRQ_CPU_NUM		8

#define BSP_IRQ_GIC_BASE	(BSP_IRQ_CPU_BASE + BSP_IRQ_CPU_NUM)  // 0+8=8



#ifdef CONFIG_IRQ_GIC
#define BSP_IRQ_GIC_NUM		(GIC_NUM_INTRS)  //28
#else
#define BSP_IRQ_GIC_NUM		0
#endif



/*
 * IRQ Mapping
 */



#define  BSP_GIC_CASCADE_IRQ	(BSP_IRQ_CPU_BASE+2)
#define BSP_TIMER0_IRQ		(BSP_IRQ_CPU_BASE + 7)

#define BSP_UART0_IRQ   		(BSP_IRQ_GIC_BASE+18)
#define BSP_UART1_IRQ			(BSP_IRQ_GIC_BASE+19)
#define BSP_UART2_IRQ			(BSP_IRQ_GIC_BASE+20)
#define BSP_PS_I2C_IRQ    		(BSP_IRQ_GIC_BASE+22)
#define BSP_SWITCH_IRQ    		(BSP_IRQ_GIC_BASE+25)
#define BSP_SWCORE_IRQ    		(BSP_IRQ_GIC_BASE+25)
#define BSP_PCIE_IRQ             (BSP_IRQ_GIC_BASE + 31)
#define BSP_PCIE2_IRQ           (BSP_IRQ_GIC_BASE+32)
#define BSP_PS_USB_IRQ_USB3 	(BSP_IRQ_GIC_BASE+34)
#define BSP_PS_SATA_IRQ  		(BSP_IRQ_GIC_BASE+38)




/*
 * MIPS32R2 counter
 */
#define BSP_PERFCOUNT_IRQ	(BSP_IRQ_CPU_BASE + 6)
#define BSP_COMPARE_IRQ		(BSP_IRQ_CPU_BASE + 7)


/*
 * DWAPB UART
 */


#define BSP_UART0_BAUD		115200
//#define BSP_UART0_MAPSIZE	0x100

#define BSP_UART0_BASE      0xB8002000
#define BSP_UART0_MAP_BASE  0x18002000
#define BSP_UART0_RBR       (BSP_UART0_BASE + 0x000)
#define BSP_UART0_THR       (BSP_UART0_BASE + 0x000)
#define BSP_UART0_DLL       (BSP_UART0_BASE + 0x000)
#define BSP_UART0_IER       (BSP_UART0_BASE + 0x004)
#define BSP_UART0_DLM       (BSP_UART0_BASE + 0x004)
#define BSP_UART0_IIR       (BSP_UART0_BASE + 0x008)
#define BSP_UART0_FCR       (BSP_UART0_BASE + 0x008)
#define BSP_UART0_LCR       (BSP_UART0_BASE + 0x00C)
#define BSP_UART0_MCR       (BSP_UART0_BASE + 0x010)
#define BSP_UART0_LSR       (BSP_UART0_BASE + 0x014)

#define BSP_UART1_BASE      0xB8002100
#define BSP_UART1_MAP_BASE  0x18002100

#define BSP_UART2_BASE      0xB8002200
#define BSP_UART2_MAP_BASE  0x18002200

#define BSP_GIMR            0xB8003000


#ifdef CONFIG_CEVT_R4K  //internal timer
/*
 * DWAPB Timer
 */

#define BSP_CPU0_FREQ		450000000 //50000000     /* 450 MHz */
#define BSP_TIMER0_FREQ		25000000     /* 25 MHz */
#define BSP_TIMER0_BASE		0xbf201000UL
#define BSP_TIMER0_TLCR		(BSP_TIMER0_BASE + 0x00)
#define BSP_TIMER0_TCVR		(BSP_TIMER0_BASE + 0x04)
#define BSP_TIMER0_TCR		(BSP_TIMER0_BASE + 0x08)   /* timer control register */
#define BSP_TIMER0_EOI		(BSP_TIMER0_BASE + 0x0c)

//watchdog releated
#define BSP_TC_BASE         0xB8003100
#define BSP_CDBR            (BSP_TC_BASE + 0x18)
#define BSP_DIVF_OFFSET     16
#define BSP_WDTCNR          (BSP_TC_BASE + 0x1C)
#define BSP_DIVISOR         200

#else   //external timer
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

#define BSP_DIVISOR         200
#define BSP_TC0_IRQ            (BSP_IRQ_GIC_BASE+14) //(14+8)	/* BSP_IRQ_LOPI_BASE + 7	*/

#endif

#ifdef CONFIG_RTL_8198C_FPGA
//#define BSP_SYS_CLK_RATE	  	(25000000)     //HS1 clock : 40 MHz
#define BSP_SYS_CLK_RATE	  	(33868800)     //HS1 clock : 40 MHz
#else
#define BSP_SYS_CLK_RATE	  	(200 * 1000 * 1000)     //HS1 clock : 200 MHz
#endif

/*
 * PCI-E
 */
#define BSP_PCIE0_H_CFG     0xB8B00000
#define BSP_PCIE0_H_EXT     0xB8B01000
#define BSP_PCIE0_H_MDIO    (BSP_PCIE0_H_EXT + 0x00)
#define BSP_PCIE0_H_INTSTR  (BSP_PCIE0_H_EXT + 0x04)
#define BSP_PCIE0_H_PWRCR   (BSP_PCIE0_H_EXT + 0x08)
#define BSP_PCIE0_H_IPCFG   (BSP_PCIE0_H_EXT + 0x0C)
#define BSP_PCIE0_H_MISC    (BSP_PCIE0_H_EXT + 0x10)
#define BSP_PCIE0_D_CFG0    0xB8B10000
#define BSP_PCIE0_D_CFG1    0xB8B11000
#define BSP_PCIE0_D_MSG     0xB8B12000

#define BSP_PCIE1_H_CFG     0xB8B20000
#define BSP_PCIE1_H_EXT     0xB8B21000
#define BSP_PCIE1_H_MDIO    (BSP_PCIE1_H_EXT + 0x00)
#define BSP_PCIE1_H_INTSTR  (BSP_PCIE1_H_EXT + 0x04)
#define BSP_PCIE1_H_PWRCR   (BSP_PCIE1_H_EXT + 0x08)
#define BSP_PCIE1_H_IPCFG   (BSP_PCIE1_H_EXT + 0x0C)
#define BSP_PCIE1_H_MISC    (BSP_PCIE1_H_EXT + 0x10)
#define BSP_PCIE1_D_CFG0    0xB8B30000
#define BSP_PCIE1_D_CFG1    0xB8B31000
#define BSP_PCIE1_D_MSG     0xB8B32000

#define BSP_PCIE0_D_IO      0xB8C00000
#define BSP_PCIE1_D_IO      0xB8E00000
#define BSP_PCIE_FUN_OFS      0xC00000
#define BSP_PCIE0_D_MEM     0xB9000000
#define BSP_PCIE0_F1_D_MEM  (BSP_PCIE0_D_MEM + BSP_PCIE_FUN_OFS)
#define BSP_PCIE1_D_MEM     0xBA000000
#define BSP_PCIE1_F1_D_MEM  (BSP_PCIE1_D_MEM + BSP_PCIE_FUN_OFS)

/*
 * USB
 */
#define BSP_USB_USB3_BASE	0xb8040000UL
#define BSP_USB_USB3_SIZE	0x00100000UL
#define BSP_USB_USB3_MAPBASE	0x18040000UL
#define BSP_USB_USB3_MAPSIZE	0x00100000UL
#define BSP_USB_OTG_BASE	0xb8100000UL
#define BSP_USB_OTG_SIZE	0x00040000UL
#define BSP_USB_OTG_MAPBASE	0x18100000UL
#define BSP_USB_OTG_MAPSIZE	0x00040000UL

/*
 * SATA
 */
#define BSP_SATA_BASE	0xb82E8000UL
#define BSP_SATA_SIZE	0x00010000UL
#define BSP_SATA_MAPBASE	0x182E8000UL
#define BSP_SATA_MAPSIZE	0x00010000UL

/*
 * I2C
 */
//#define BSP_I2C_BASE		0xb8000600UL
#define BSP_I2C_SIZE		0x00000100UL
#define BSP_I2C_MAPBASE	0x18000600UL
#define BSP_I2C_MAPSIZE		0x00000100UL

/* GPIO Register Set */
#define BSP_GPIO_BASE	(0xB8003500UL)
#define BSP_PABCD_CNR	(0x000 + BSP_GPIO_BASE) /* Port ABCD control */
#define BSP_PABCD_PTYPE	(0x004 + BSP_GPIO_BASE) /* Port ABCD type */
#define BSP_PABCD_DIR	(0x008 + BSP_GPIO_BASE) /* Port ABCD direction */
#define BSP_PABCD_DAT	(0x00C + BSP_GPIO_BASE) /* Port ABCD data */
#define BSP_PABCD_ISR	(0x010 + BSP_GPIO_BASE) /* Port ABCD interrupt status */
#define BSP_PAB_IMR	(0x014 + BSP_GPIO_BASE) /* Port AB interrupt mask */
#define BSP_PCD_IMR	(0x018 + BSP_GPIO_BASE) /* Port CD interrupt mask */
#define BSP_PEFGH_CNR	(0x01C + BSP_GPIO_BASE) /* Port ABCD control */
#define BSP_PEFGHP_TYPE	(0x020 + BSP_GPIO_BASE) /* Port ABCD type */
#define BSP_PEFGH_DIR	(0x024 + BSP_GPIO_BASE) /* Port ABCD direction */
#define BSP_PEFGH_DAT	(0x028 + BSP_GPIO_BASE) /* Port ABCD data */
#define BSP_PEFGH_ISR	(0x02C + BSP_GPIO_BASE) /* Port ABCD interrupt status */
#define BSP_PEF_IMR	(0x030 + BSP_GPIO_BASE) /* Port AB interrupt mask */
#define BSP_PGH_IMR	(0x034 + BSP_GPIO_BASE) /* Port CD interrupt mask */


/*
 * PIN MUX
 */
#define BSP_PIN_MUX_SEL1	0xB8000100UL
#define BSP_PIN_MUX_SEL2	0xB8000104UL
#define BSP_PIN_MUX_SEL3	0xB8000108UL
#define BSP_PIN_MUX_SEL4	0xB800010CUL
#define BSP_PIN_MUX_SEL5	0xB8000110UL


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

	BSP_GPIO_PIN_C0,
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

	BSP_GPIO_PIN_G0,
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

#define BSP_PINMUX_SEL_REG(pin) ((((pin >= BSP_GPIO_PIN_B0) && (pin <= BSP_GPIO_PIN_B3)) ||  \
							  ((pin >= BSP_GPIO_PIN_C0) && (pin <= BSP_GPIO_PIN_C4)) ) ? \
							  BSP_PIN_MUX_SEL2 : BSP_PIN_MUX_SEL1)

#define BSP_GPIO_BIT(pin)  		(pin & ~(BSP_GPIO_2ND_REG))
#define BSP_GPIO_CNR_REG(pin) 	((pin & BSP_GPIO_2ND_REG) ? BSP_PEFGH_CNR : BSP_PABCD_CNR)
#define BSP_GPIO_DIR_REG(pin) 	((pin & BSP_GPIO_2ND_REG) ? BSP_PEFGH_DIR : BSP_PABCD_DIR)
#define BSP_GPIO_DAT_REG(pin) 	((pin & BSP_GPIO_2ND_REG) ? BSP_PEFGH_DAT : BSP_PABCD_DAT)


#endif   /* _BSPCHIP_H */
