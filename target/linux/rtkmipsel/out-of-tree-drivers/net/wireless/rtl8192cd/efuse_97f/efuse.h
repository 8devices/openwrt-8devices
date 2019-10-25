
#ifndef __EFUSE_H__
#define __EFUSE_H__

//#define     REG_EFUSE_BASE_ADDR                 EFUSE_BASE_ADDR  // 0x18000700
//#define     REG_EFUSE_BASE_ADDR                 0x18000700
#define     REG_EFUSE_BASE_ADDR                 0xB8000700

#define     REG_EFUSE_CMD                       (REG_EFUSE_BASE_ADDR + 0x00000000)
#define     REG_EFUSE_CONFIG                    (REG_EFUSE_BASE_ADDR + 0x00000004)
#define     REG_EFUSE_TIMING_CTRL               (REG_EFUSE_BASE_ADDR + 0x00000008)
#define     REG_EFUSE_WRITE_DATA_0              (REG_EFUSE_BASE_ADDR + 0x0000000C)
#define     REG_EFUSE_WRITE_DATA_1              (REG_EFUSE_BASE_ADDR + 0x00000010)
#define     REG_EFUSE_WRITE_DATA_2              (REG_EFUSE_BASE_ADDR + 0x00000014)
#define     REG_EFUSE_WRITE_DATA_3              (REG_EFUSE_BASE_ADDR + 0x00000018)
#define     REG_EFUSE_WRITE_DATA_BIT_MASK_0     (REG_EFUSE_BASE_ADDR + 0x0000001C)
#define     REG_EFUSE_WRITE_DATA_BIT_MASK_1     (REG_EFUSE_BASE_ADDR + 0x00000020)
#define     REG_EFUSE_WRITE_DATA_BIT_MASK_2     (REG_EFUSE_BASE_ADDR + 0x00000024)
#define     REG_EFUSE_WRITE_DATA_BIT_MASK_3     (REG_EFUSE_BASE_ADDR + 0x00000028)
#define     REG_EFUSE_BURN_CHECK_FAIL_0         (REG_EFUSE_BASE_ADDR + 0x0000002C)
#define     REG_EFUSE_BURN_CHECK_FAIL_1         (REG_EFUSE_BASE_ADDR + 0x00000030)
#define     REG_EFUSE_BURN_CHECK_FAIL_2         (REG_EFUSE_BASE_ADDR + 0x00000034)
#define     REG_EFUSE_BURN_CHECK_FAIL_3         (REG_EFUSE_BASE_ADDR + 0x00000038)

#define     REG_EFUSE_P0_0                      (REG_EFUSE_BASE_ADDR + 0x00000080)
#define     REG_EFUSE_P0_1                      (REG_EFUSE_BASE_ADDR + 0x00000084)
#define     REG_EFUSE_P0_2                      (REG_EFUSE_BASE_ADDR + 0x00000088)
#define     REG_EFUSE_P0_3                      (REG_EFUSE_BASE_ADDR + 0x0000008C)
#define     REG_EFUSE_P1_0                      (REG_EFUSE_BASE_ADDR + 0x00000090)
#define     REG_EFUSE_P1_1                      (REG_EFUSE_BASE_ADDR + 0x00000094)
#define     REG_EFUSE_P1_2                      (REG_EFUSE_BASE_ADDR + 0x00000098)
#define     REG_EFUSE_P1_3                      (REG_EFUSE_BASE_ADDR + 0x0000009C)
#define     REG_EFUSE_P2_0                      (REG_EFUSE_BASE_ADDR + 0x000000A0)
#define     REG_EFUSE_P2_1                      (REG_EFUSE_BASE_ADDR + 0x000000A4)
#define     REG_EFUSE_P2_2                      (REG_EFUSE_BASE_ADDR + 0x000000A8)
#define     REG_EFUSE_P2_3                      (REG_EFUSE_BASE_ADDR + 0x000000AC)
#define     REG_EFUSE_P3_0                      (REG_EFUSE_BASE_ADDR + 0x000000B0)
#define     REG_EFUSE_P3_1                      (REG_EFUSE_BASE_ADDR + 0x000000B4)
#define     REG_EFUSE_P3_2                      (REG_EFUSE_BASE_ADDR + 0x000000B8)
#define     REG_EFUSE_P3_3                      (REG_EFUSE_BASE_ADDR + 0x000000BC)
#define     REG_EFUSE_P4_0                      (REG_EFUSE_BASE_ADDR + 0x000000C0)
#define     REG_EFUSE_P4_1                      (REG_EFUSE_BASE_ADDR + 0x000000C4)
#define     REG_EFUSE_P4_2                      (REG_EFUSE_BASE_ADDR + 0x000000C8)
#define     REG_EFUSE_P4_3                      (REG_EFUSE_BASE_ADDR + 0x000000CC)
#define     REG_EFUSE_P5_0                      (REG_EFUSE_BASE_ADDR + 0x000000D0)
#define     REG_EFUSE_P5_1                      (REG_EFUSE_BASE_ADDR + 0x000000D4)
#define     REG_EFUSE_P5_2                      (REG_EFUSE_BASE_ADDR + 0x000000D8)
#define     REG_EFUSE_P5_3                      (REG_EFUSE_BASE_ADDR + 0x000000DC)
#define     REG_EFUSE_P6_0                      (REG_EFUSE_BASE_ADDR + 0x000000E0)
#define     REG_EFUSE_P6_1                      (REG_EFUSE_BASE_ADDR + 0x000000E4)
#define     REG_EFUSE_P6_2                      (REG_EFUSE_BASE_ADDR + 0x000000E8)
#define     REG_EFUSE_P6_3                      (REG_EFUSE_BASE_ADDR + 0x000000EC)
#define     REG_EFUSE_P7_0                      (REG_EFUSE_BASE_ADDR + 0x000000F0)
#define     REG_EFUSE_P7_1                      (REG_EFUSE_BASE_ADDR + 0x000000F4)
#define     REG_EFUSE_P7_2                      (REG_EFUSE_BASE_ADDR + 0x000000F8)
#define     REG_EFUSE_P7_3                      (REG_EFUSE_BASE_ADDR + 0x000000FC)

#define     EFUSE_PORT_MAX                      8
#define     EFUSE_PORT_IDX_MAX                  4

// REG_EFUSE_CMD
#define     BIT_SHIFT_EFUSE_CMD_ADDR            0
#define     BIT_MASK_EFUSE_CMD_ADDR             0x7
#define     BIT_EFUSE_CMD_ADDR(x)               (((x) & BIT_MASK_EFUSE_CMD_ADDR) << BIT_SHIFT_EFUSE_CMD_ADDR)
#define     BIT_EFUSE_RW_CTRL                   BIT(15)
#define     BIT_EFUSE_CTRL_STATE                BIT(16)

// REG_EFUSE_CONFIG
#define     BIT_EFUSE_CONFIG_PWR_GATE_EN        BIT(0)
#define     BIT_EFUSE_CONFIG_BURN_CHECK         BIT(31)


// return status
#define     STATUS_EFUSE_SUCCESS                0
#define     STATUS_EFUSE_FAIL                   1
#define     STATUS_EFUSE_TIMEOUT                2
#define     STATUS_EFUSE_EMPTY                  3


typedef struct _EFUSE_DATA_
{
    u1Byte  ctrl;
    u1Byte  ctrl2;
	u1Byte	special;
    u4Byte  exec_from_sram_addr;
    u4Byte  boot_addr;
    u2Byte  img_offset;
    u1Byte  init_dram_val;
    u4Byte  heap_start_addr;
    u2Byte  heap_len;
    u4Byte  erase_start_addr;
} EFUSE_DATA, *PEFUSE_DATA;

#define EXT_HEADER(hdr)                 ((hdr & 0x1F ) == 0x0F)
#define ALL_WORDS_DISABLED(wde)	        ((wde & 0x0F) == 0x0F)
#define GET_HDR_OFFSET_2_0(hdr)         ((hdr & 0xE0) >> 5)
#define DATA_IN_THIS_OFFSET(addr, loc)  ((loc <= addr) && (addr < (loc+EFUSE_FORMAT_WORD_UNIT)))

#define EFUSE_DATA_SIZE_MAX                 0x80
#define EFUSE_FORMAT_WORD_UNIT              4
#define EFUSE_IMG_OFFSET_UNIT               1024
#define EFUSE_HEAP_LEN_UNIT                 1024


// function list
u1Byte write_efuse_byte(u4Byte data, u4Byte mask, u4Byte dataPort, u4Byte dataIdx);
u1Byte load_efuse_data_to_reg(void);
u4Byte read_efuse_byte(u4Byte idx);
u1Byte load_efuse_data(PEFUSE_DATA pefuse_data, u1Byte bPrint);
void dump_efuse_reg_data(u4Byte max_num);
void set_efuse_options(PEFUSE_DATA pefuse_data);
void get_efuse_data(u1Byte offset, u1Byte len, pu1Byte value);

// define efuse virtual map
// Note: each offset must be 8 byte alignment currently
#define OFFSET_EFUSE_BOOT_CTRL                      0x200

#define BIT_SHIFT_EFUSE_BOOT_CTRL_EXEC_FROM_SRAM    0
#define BIT_MASK_EFUSE_BOOT_CTRL_EXEC_FROM_SRAM     0x3
#define BIT_GET_EFUSE_BOOT_CTRL_EXEC_FROM_SRAM(x)   (((x) >> BIT_SHIFT_EFUSE_BOOT_CTRL_EXEC_FROM_SRAM) & BIT_MASK_EFUSE_BOOT_CTRL_EXEC_FROM_SRAM)
#define EFUSE_BOOT_CTRL_EXEC_FROM_SRAM_VAL_INIT_RAM             0x0
#define EFUSE_BOOT_CTRL_EXEC_FROM_SRAM_VAL_EXEC_SRAM            0x1
#define EFUSE_BOOT_CTRL_EXEC_FROM_SRAM_VAL_INIT_RAM_EXEC_SRAM   0x2
#define EFUSE_BOOT_CTRL_EXEC_FROM_SRAM_VAL_EXEC_SRAM_INIT_RAM   0x3

#define BIT_EFUSE_BOOT_CTRL_EXEC_FROM_SRAM_ADDR     BIT(2)
#define BIT_EFUSE_BOOT_CTRL_UBOOT_ADDR              BIT(3)
#define BIT_EFUSE_BOOT_CTRL_CHANGE_CPU_FREQ         BIT(4)
#define BIT_EFUSE_BOOT_CTRL_IMG_OFFSET              BIT(5)

#define BIT_SHIFT_EFUSE_BOOT_CTRL_DEBUG_MSG         6
#define BIT_MASK_EFUSE_BOOT_CTRL_DEBUG_MSG          0x3
#define BIT_GET_EFUSE_BOOT_CTRL_DEBUG_MSG(x)        (((x) >> BIT_SHIFT_EFUSE_BOOT_CTRL_DEBUG_MSG) & BIT_MASK_EFUSE_BOOT_CTRL_DEBUG_MSG)

#define BIT_EFUSE_BOOT_CTRL_INIT_HEAP                   BIT(0)
#define BIT_EFUSE_BOOT_CTRL_ERASE_SPI_NOR_FLASH_ADDR    BIT(1)

#define OFFSET_EFUSE_BOOT_INIT_DDR_FROM_EXT_ADDR    0x208
#define OFFSET_EFUSE_BOOT_UBOOT_ADDR                0x210
#define OFFSET_EFUSE_IMG_OFFSET                     0x218

#define OFFSET_EFUSE_DRAM                           0x220
#define BIT_EFUSE_DRAM_DDR2_CLK200                  BIT(0)
#define BIT_EFUSE_DRAM_ENABLE_ODT                   BIT(1)

#define OFFSET_EFUSE_HEAP                           0x228
#define OFFSET_EFUSE_ERASE_SPI_NOR_FLASH_ADDR       0x230

// efuse physical map offset
// each offset 4 byte alignment currently
#define OFFSET_EFUSE_PHYSICAL_AES_MODE              0x2C
#define EFUSE_AES_MODE_SIZE                         4
#define BIT_SHIFT_EFUSE_AES_MODE                    24
#define BIT_MASK_EFUSE_AES_MODE                     0x3
#define BIT_GET_EFUSE_AES_MODE(x)                   (((x) >> BIT_SHIFT_EFUSE_AES_MODE) & BIT_MASK_EFUSE_AES_MODE)
#define EFUSE_AES_MODE_ECB                          0x0
#define EFUSE_AES_MODE_CBC                          0x1
#define EFUSE_AES_MODE_CTR                          0x2

#define OFFSET_EFUSE_PHYSICAL_CURVE_SECURE_KEY      0x30
#define EFUSE_CURVE_KEY_SIZE                        32


#endif //_EFUSE_H_

