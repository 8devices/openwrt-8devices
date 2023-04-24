#ifndef __SYS_REG_H__
#define __SYS_REG_H__

#define REG_SYS_REG_BASE 0xB8000000

#define REG_HW_STRAP    (REG_SYS_REG_BASE + 0x0008)
#define REG_BOND_OPTION (REG_SYS_REG_BASE + 0x000C)
#define REG_CLK_MANAGE  (REG_SYS_REG_BASE + 0x0010)
#define REG_CLK_MANAGE2 (REG_SYS_REG_BASE + 0x0014)
#define REG_ENABLE_IP   (REG_SYS_REG_BASE + 0x0050)
#define REG_DEBUG_SEL           (REG_SYS_REG_BASE + 0x00C0)
#define REG_FW_DBG_SEL_OPTION   (REG_SYS_REG_BASE + 0x00C8)
#define REG_SD30_1		(REG_SYS_REG_BASE + 0x01C0)
#define REG_SD30_2		(REG_SYS_REG_BASE + 0x01C4)
#define REG_SD30_3		(REG_SYS_REG_BASE + 0x01C8)
#define REG_SD30_4		(REG_SYS_REG_BASE + 0x01CC)

#define REG_PINMUX_00   (REG_SYS_REG_BASE + 0x0800)
#define REG_PINMUX_01   (REG_SYS_REG_BASE + 0x0804)
#define REG_PINMUX_02   (REG_SYS_REG_BASE + 0x0808)
#define REG_PINMUX_03   (REG_SYS_REG_BASE + 0x080C)
#define REG_PINMUX_04   (REG_SYS_REG_BASE + 0x0810)
#define REG_PINMUX_05   (REG_SYS_REG_BASE + 0x0814)
#define REG_PINMUX_06   (REG_SYS_REG_BASE + 0x0818)
#define REG_PINMUX_07   (REG_SYS_REG_BASE + 0x081C)
#define REG_PINMUX_08   (REG_SYS_REG_BASE + 0x0820)
#define REG_PINMUX_09   (REG_SYS_REG_BASE + 0x0824)
#define REG_PINMUX_10   (REG_SYS_REG_BASE + 0x0828)
#define REG_PINMUX_11   (REG_SYS_REG_BASE + 0x082C)
#define REG_PINMUX_12   (REG_SYS_REG_BASE + 0x0830)
#define REG_PINMUX_13   (REG_SYS_REG_BASE + 0x0834)
#define REG_PINMUX_14   (REG_SYS_REG_BASE + 0x0838)
#define REG_PINMUX_15   (REG_SYS_REG_BASE + 0x083C)
#define REG_PINMUX_16   (REG_SYS_REG_BASE + 0x0840)
#define REG_PINMUX_17   (REG_SYS_REG_BASE + 0x0844)
#define REG_PINMUX_18   (REG_SYS_REG_BASE + 0x0848)

//2 REG_HW_STRAP                         (Offset 0x0008)
#define     BIT_SHIFT_STRAP_PIN_BOOT_SEL        0
#define     BIT_MASK_STRAP_PIN_BOOT_SEL         0xf
#define     BIT_STRAP_PIN_BOOT_SEL(x)           (((x) & BIT_MASK_STRAP_PIN_BOOT_SEL) << BIT_SHIFT_STRAP_PIN_BOOT_SEL)
#define     BIT_GET_STRAP_PIN_BOOT_SEL(x)       (((x) >> BIT_SHIFT_STRAP_PIN_BOOT_SEL) & BIT_MASK_STRAP_PIN_BOOT_SEL)
#define     BIT_INV_STRAP_PIN_BOOT_SEL          (~(BIT_MASK_STRAP_PIN_BOOT_SEL << BIT_SHIFT_STRAP_PIN_BOOT_SEL))

#define     BIT_SHIFT_STRAP_PIN_DRAM_TYPE_INV   4
#define     BIT_MASK_STRAP_PIN_DRAM_TYPE_INV    0x1
#define     BIT_STRAP_PIN_DRAM_TYPE_INV         BIT(4)
#define     BIT_GET_STRAP_PIN_DRAM_TYPE_INV(x)  (((x) >> BIT_SHIFT_STRAP_PIN_DRAM_TYPE_INV) & BIT_MASK_STRAP_PIN_DRAM_TYPE_INV)

#define     BIT_SHIFT_STRAP_PIN_DRAM_FEQ        5
#define     BIT_MASK_STRAP_PIN_DRAM_FEQ         0x1
#define     BIT_STRAP_PIN_DRAM_FEQ              BIT(5) // Note: two definitions of BIT(5)
#define     BIT_GET_STRAP_PIN_DRAM_FEQ(x)       (((x) >> BIT_SHIFT_STRAP_PIN_DRAM_FEQ) & BIT_MASK_STRAP_PIN_DRAM_FEQ)

#define     BIT_STRAP_PIN_HW_DBG_OPTION         BIT(5) // Note: two definitions of BIT(5)
#define     BIT_STRAP_PIN_HW_DBG_DISABLE        BIT(6)
#define     BIT_STRAP_PIN_DDR_LDO_SEL           BIT(7)

#define     BIT_SHIFT_STRAP_PIN_PHY_ID_SEL      8
#define     BIT_MASK_STRAP_PIN_PHY_ID_SEL       0x3
#define     BIT_STRAP_PIN_PHY_ID_SEL(x)         (((x) & BIT_MASK_STRAP_PIN_PHY_ID_SEL) << BIT_SHIFT_STRAP_PIN_PHY_ID_SEL)

#define     BIT_STRAP_PIN_DISABLE_LOAD_EFUSE_VALUE  BIT(10)
#define     BIT_STRAP_PIN_DISABLE_EXT_RSTN          BIT(11)

#define     BIT_STRAP_PIN_CK_BUS_DIV2_SEL       BIT(12)
#define     BIT_STRAP_PIN_CK_BUS_DIV4_SEL       BIT(13)
#define     BIT_STARP_PIN_CLKCPU_FROM_CLK200    BIT(14)

#define     BIT_SHIFT_STRAP_PIN_CK_CPU_FREQ_SEL 15
#define     BIT_MASK_STRAP_PIN_CK_CPU_FREQ_SEL  0xf
#define     BIT_STRAP_PIN_CK_CPU_FREQ_SEL(x)    (((x) & BIT_MASK_STRAP_PIN_CK_CPU_FREQ_SEL) << BIT_SHIFT_STRAP_PIN_CK_CPU_FREQ_SEL)
#define     BIT_GET_STRAP_PIN_CK_CPU_FREQ_SEL(x)    (((x) >> BIT_SHIFT_STRAP_PIN_CK_CPU_FREQ_SEL) & BIT_MASK_STRAP_PIN_CK_CPU_FREQ_SEL)
#define     BIT_INV_STRAP_PIN_CK_CPU_FREQ_SEL   (~(BIT_MASK_STRAP_PIN_CK_CPU_FREQ_SEL << BIT_SHIFT_STRAP_PIN_CK_CPU_FREQ_SEL))

#define     BIT_SHIFT_STRAP_PIN_CK_CPU_DIV_SEL_0    BIT(19)
#define     BIT_SHIFT_STRAP_PIN_CK_CPU_DIV_SEL_1    BIT(20)
#define     BIT_SHIFT_STRAP_PIN_CK_CPU_DIV_SEL      19
#define     BIT_MASK_STRAP_PIN_CK_CPU_DIV_SEL       0x3
#define     BIT_STRAP_PIN_CK_CPU_DIV_SEL(x)         (((x) & BIT_MASK_STRAP_PIN_CK_CPU_DIV_SEL) << BIT_SHIFT_STRAP_PIN_CK_CPU_DIV_SEL)
#define     BIT_GET_STRAP_PIN_CK_CPU_DIV_SEL(x)     (((x) >> BIT_SHIFT_STRAP_PIN_CK_CPU_DIV_SEL) & BIT_MASK_STRAP_PIN_CK_CPU_DIV_SEL)
#define     BIT_INV_STRAP_PIN_CK_CPU_DIV_SEL        (~(BIT_MASK_STRAP_PIN_CK_CPU_DIV_SEL << BIT_SHIFT_STRAP_PIN_CK_CPU_DIV_SEL))

#define     BIT_STRAP_PIN_SWITCH_PHY_SEL        BIT(21)
#define     BIT_STRAP_PIN_EVER_REBOOT_ONCE      BIT(22)
#define     BIT_STRAP_PIN_MIIM_SLV              BIT(23)
#define     BIT_STRAP_PIN_SEL_40M               BIT(24)
#define     BIT_STRAP_STRAP_TEST_MODE           BIT(25)

// Note: two definitions of BIT(26)~BIT(28)
#define     BIT_SHIFT_STRAP_PIN_MII_RX_DLY      26
#define     BIT_MASK_STRAP_PIN_MII_RX_DLY       0x7
#define     BIT_STRAP_PIN_MII_RX_DLY(x)         (((x) & BIT_MASK_STRAP_PIN_MII_RX_DLY) << BIT_SHIFT_STRAP_PIN_MII_RX_DLY)

// Note: two definitions of BIT(26)~BIT(28)
#define     BIT_SHIFT_STRAP_PIN_NAFC_ECC        26
#define     BIT_MASK_STRAP_PIN_NAFC_ECC         0x7
#define     BIT_STRAP_PIN_NAFC_ECC(x)           (((x) & BIT_MASK_STRAP_PIN_NAFC_ECC) << BIT_SHIFT_STRAP_PIN_NAFC_ECC)

#define     BIT_STRAP_PIN_MII_TX_DLY            BIT(29)

#define     BIT_SHIFT_STRAP_PIN_MII_MODE        30
#define     BIT_MASK_STRAP_PIN_MII_MODE         0x3
#define     BIT_STRAP_PIN_MII_MODE(x)           (((x) & BIT_MASK_STRAP_PIN_MII_MODE) << BIT_SHIFT_STRAP_PIN_MII_MODE)


#define		STRAP_PIN_CK_CPU_FREQ_SEL_DEFAULT	0x7


//2 REG_BOND_OPTION                         (Offset 0x000C)
#define     BIT_SHIFT_CHIP_ID_CODE              0
#define     BIT_MASK_CHIP_ID_CODE               0xf
#define     BIT_CHIP_ID_CODE(x)                 (((x) & BIT_MASK_CHIP_ID_CODE) << BIT_SHIFT_CHIP_ID_CODE)
#define     BIT_GET_CHIP_ID_CODE(x)             (((x) >> BIT_SHIFT_CHIP_ID_CODE) & BIT_MASK_CHIP_ID_CODE)
#define     BIT_INV_CHIP_ID_CODE                (~(BIT_MASK_CHIP_ID_CODE << BIT_SHIFT_CHIP_ID_CODE))
#define     IS_FB_BONDING(x)                    (x <= 3)

//2 REG_CLK_MANAGE                         (Offset 0x0010)
#define BIT_ACTIVE_USBOTG                     	BIT(29)
#define BIT_SEL_EXT_CK_45M                    	BIT(25)
#define BIT_ACTIVE_PCM                        	BIT(23)
#define BIT_ACTIVE_IIS                        	BIT(22)
#define BIT_ACTIVE_LX2ARB                     	BIT(20)
#define BIT_ACTIVE_LX2                        	BIT(19)
#define BIT_ACTIVE_IPSEC                      	BIT(17)
#define BIT_ACTIVE_PCIE0                      	BIT(14)
#define BIT_ACTIVE_LX1ARB                     	BIT(13)
#define BIT_ACTIVE_LX1                        	BIT(12)
#define BIT_ACTIVE_SWCORE                     	BIT(11)
#define BIT_SWITCH_RST_PROTECT                	BIT(10)
#define BIT_ACTIVE_GDMA                       	BIT(9)

//2 REG_CLK_MANAGE2                        (Offset 0x0014)
#define BIT_ACTIVE_NFCTRL                     	BIT(21)
#define BIT_ACTIVE_SPIFLSHECC                 	BIT(20)
#define BIT_ACTIVE_SPINAND                    	BIT(19)
#define BIT_ACTIVE_SD30                       	BIT(18)
#define BIT_ACTIVE_SD30_PLL5M                 	BIT(17)
#define BIT_ACTIVE_DW_SSI0                    	BIT(16)
#define BIT_ACTIVE_DW_SSI0_CLKGATE            	BIT(15)
#define BIT_ACTIVE_DW_SSI1                    	BIT(14)
#define BIT_ACTIVE_DW_SSI1_CLKGATE            	BIT(13)
#define BIT_ACTIVE_DW_I2C0                    	BIT(12)
#define BIT_ACTIVE_DW_I2C0_CLKGATE            	BIT(11)
#define BIT_ACTIVE_DW_I2C1                    	BIT(10)
#define BIT_ACTIVE_DW_I2C1_CLKGATE            	BIT(9)
#define BIT_ACTIVE_RT_UART0                   	BIT(8)
#define BIT_ACTIVE_RT_UART1                   	BIT(7)
#define BIT_ACTIVE_RT_UART2                   	BIT(6)
#define BIT_ACTIVE_DW_TIMER                   	BIT(5)
#define BIT_ACTIVE_APB_BUS                    	BIT(4)
#define BIT_ACTIVE_USB2HOST                   	BIT(3)

//2 REG_ENABLE_IP                        (Offset 0x0050)
#define BIT_SHIFT_ROM_OK_CHECK                  24
#define BIT_MASK_ROM_OK_CHECK                   0xff
#define BIT_ROM_OK_CHECK(x)                     (((x) & BIT_MASK_ROM_OK_CHECK) << BIT_SHIFT_ROM_OK_CHECK)
#define BIT_INV_ROM_OK_CHECK                    (~(BIT_MASK_ROM_OK_CHECK << BIT_SHIFT_ROM_OK_CHECK))

#define BIT_SSI0_TX_OEN                         BIT(8)
#define BIT_SSI1_TX_OEN                         BIT(7)
#define BIT_SSI0_SSI_MAS_SEL                    BIT(6)
#define BIT_SSI1_SSI_MAS_SEL                    BIT(5)
#define BIT_CLK_EN_SD30                         BIT(4)
#define BIT_ENPCM                               BIT(3)
#define BIT_EN_IPSEC                            BIT(2)
#define BIT_PCIE_RSTN                           BIT(1)

//2 REG_SD30_1
#define BIT_SHIFT_REG_TUNED3318                 19
#define BIT_MASK_REG_TUNED3318                  0x7
#define BIT_REG_TUNED3318(x)                    (((x) & BIT_MASK_REG_TUNED3318) << BIT_SHIFT_REG_TUNED3318)
#define BIT_INV_REG_TUNED3318                   (~(BIT_MASK_REG_TUNED3318 << BIT_SHIFT_REG_TUNED3318))
#define BIT_REG_TUNED3318_VAL_18V               0x1
#define BIT_REG_TUNED3318_VAL_33V               0x7

//2 REG_SD30_2
#define BIT_SSCLDO_EN                           BIT(0)

//2 REG_SD30_3

//2 REG_SD30_4
#define BIT_SSC_PLL_POW                         BIT(2)
#define BIT_SSC_PLL_RSTB                        BIT(1)
#define BIT_SSC_RSTB                            BIT(0)

//2 REG_DEBUG_SEL
#define BIT_BYPASS_SLEEP                        BIT(31)

//2 REG_FW_DBG_SEL_OPTION
#define BIT_BYPASS_SLEEP_EN                     BIT(31)

#endif

