//#include "common.h"
#include "phydm_precomp.h"

#include "rom_def.h"
#include "efuse.h"
//#include <linux/string.h>
//#include <rlxboard.h>
#include "sys_reg.h"




static void load_efuse_word_data(u1Byte word_enb, pu4Byte pidx, pu1Byte data);
static void dump_efuse_word_data(u4Byte loc, pu1Byte data);

//extern u4Byte get_cpu_freq(u4Byte strap_pin);
//extern void set_rom_progress(u1Byte value);
inline void set_rom_progress(u1Byte value)
{
    REG32(REG_ENABLE_IP) = (REG32(REG_ENABLE_IP) & BIT_INV_ROM_OK_CHECK) | BIT_ROM_OK_CHECK(value);
    DBG_MSG(COMP_MONITOR, DBG_TRACE, ("rom_progress: 0x%x\n", REG32(REG_ENABLE_IP)));
	panic_printk("rom_progress: 0x%x\n", REG32(REG_ENABLE_IP));
}

void get_efuse_data(u1Byte offset, u1Byte len, pu1Byte value)
{
    // TODO: endian issue ???????//
    // beforce calling this function, you must guarantee that the function "load_efuse_data_to_reg" has been called
    u1Byte UNIT = 4;
    u1Byte cnt = len / UNIT;
    u4Byte i;

    if ((len % UNIT) != 0) {
        cnt += 1;
    }

    for (i = 0; i < cnt; i++) {
        memcpy(REG32(REG_EFUSE_P0_0 + offset + i), value, UNIT);
        value += UNIT;
    }
}

#if 0
void set_efuse_options(PEFUSE_DATA pefuse_data)
{   
    u4Byte strap_pin, cpu_freq;
    //globalDebugLevel = BIT_GET_EFUSE_BOOT_CTRL_DEBUG_MSG(pefuse_data->ctrl);

    if (pefuse_data->ctrl & BIT_EFUSE_BOOT_CTRL_CHANGE_CPU_FREQ) {
        // set REG_HW_STRAP[19:18] = 2'b00;
        REG32(REG_HW_STRAP) = REG32(REG_HW_STRAP) & ~(BIT_SHIFT_STRAP_PIN_CK_CPU_DIV_SEL_0|BIT_SHIFT_STRAP_PIN_CK_CPU_DIV_SEL_1);
        strap_pin = REG32(REG_HW_STRAP);
        cpu_freq = get_cpu_freq(strap_pin);
        DBG_MSG(COMP_EFUSE, DBG_MESSAGE, ("cpu:%d MHz, strap:0x%x \n", cpu_freq, strap_pin));
    }
}
#endif
#if FPGA_VERIFY_EFUSE
void efuse_test_data(void)
{
    // Note: if you want to write different data, you must reset FPGA
    // Note: the last entry must write 0x00000000

#if 0
    write_efuse_byte(0x8F81290E, 0xFFFFFFFF, 0, 0);
    write_efuse_byte(0xAF3C1137, 0xFFFFFFFF, 0, 1);
    write_efuse_byte(0x0F00273E, 0xFFFFFFFF, 0, 2);
    write_efuse_byte(0xEF300057, 0xFFFFFFFF, 0, 3);

    write_efuse_byte(0xEF05814E, 0xFFFFFFFF, 1, 0);
    write_efuse_byte(0xFF80B04B, 0xFFFFFFFF, 1, 1);
    write_efuse_byte(0x00000000, 0xFFFFFFFF, 1, 2);
    write_efuse_byte(0x00000000, 0xFFFFFFFF, 1, 3);
#endif

#if 1
    // efuse default value is 0x00000000
    
    // 0F 8E (addr: 0x200) - C3 00

    // verify change cpu freq
    //write_efuse_byte(0x00D08E0F, 0xFFFFFFFF, 0, 0);

    // verify execute from SRAM and load address is 0xBFE00500
    //write_efuse_byte(0x00C78E0F, 0xFFFFFFFF, 0, 0);

    // 2F 8C (addr: 0x208) - 10 05 E0 BF
    // 4F 8C (addr: 0x210) - 00 10 60 A0
    //write_efuse_byte(0x05008C2F, 0xFFFFFFFF, 0, 1);
    //write_efuse_byte(0x8C4FBFE0, 0xFFFFFFFF, 0, 2);
    //write_efuse_byte(0xA0600000, 0xFFFFFFFF, 0, 3);
    //write_efuse_byte(0x00000000, 0xFFFFFFFF, 1, 0);
    //write_efuse_byte(0x00000000, 0xFFFFFFFF, 1, 1);

    // verify initial heap address
    // 0F 8E (addr: 0x200) - 00 01
    // AF 8C (addr: 0x228) - 00 00 00 80
    // CF 8E (addr: 0x230) - 50 00
    //write_efuse_byte(0x01008E0F, 0xFFFFFFFF, 0, 0);
    //write_efuse_byte(0x00008CAF, 0xFFFFFFFF, 0, 1);
    //write_efuse_byte(0x8ECF8000, 0xFFFFFFFF, 0, 2);
    //write_efuse_byte(0x00000050, 0xFFFFFFFF, 0, 3);

    // verify erase spi-nor-flash
    // 0F 8E (addr: 0x200) - C0 02
    // CF 8C (addr: 0x230) - 00 10 00 B0
    // test image len and heap len 
    // 6F 8C (addr: 0x218) - 06 00 03 00
    write_efuse_byte(0x02C08E0F, 0xFFFFFFFF, 0, 0);
    write_efuse_byte(0x10008CCF, 0xFFFFFFFF, 0, 1);
    write_efuse_byte(0x8C6FB000, 0xFFFFFFFF, 0, 2);
    write_efuse_byte(0x00030006, 0xFFFFFFFF, 0, 3);

#endif

#if 0
    // 0F 8E (addr: 0x200) - 07 00
    write_efuse_byte(0x00078E0F, 0xFFFFFFFF, 1, 0);
    // 4F 8C (addr: 0x210) - 00 00 70 A0
    write_efuse_byte(0x00008C4F, 0xFFFFFFFF, 1, 1);
    write_efuse_byte(0x0000A070, 0xFFFFFFFF, 1, 2);
    write_efuse_byte(0x00000000, 0xFFFFFFFF, 1, 3);
#endif
}
#endif // FPGA_VERIFY_EFUSE

void dump_efuse_reg_data(u4Byte max_num)
{
    u4Byte i, reg;
    for(i = 0; i < EFUSE_PORT_MAX*EFUSE_PORT_IDX_MAX && i < max_num; i++) {
        reg = REG_EFUSE_P0_0 + i*4;
        DBG_MSG(COMP_EFUSE, DBG_TRACE, ("0x%x: 0x%x\n", reg, REG32(reg)));
    }
}

#if EFUSE_WRITE_CMD
// dataPort: 0~7, dataIdx: 0~3
u1Byte write_efuse_byte(u4Byte data, u4Byte mask, u4Byte dataPort, u4Byte dataIdx)
{
    u4Byte cnt = 0;
    u4Byte cnt_max = 1000;

    REG32(REG_EFUSE_CONFIG) =  REG32(REG_EFUSE_CONFIG) | BIT_EFUSE_CONFIG_PWR_GATE_EN | BIT_EFUSE_CONFIG_BURN_CHECK;

    // suggest delay 1 us on spec.
    //udelay(1);
    delay_us(1);

    // clear all mask
    REG32(REG_EFUSE_WRITE_DATA_BIT_MASK_0) = 0;
    REG32(REG_EFUSE_WRITE_DATA_BIT_MASK_1) = 0;
    REG32(REG_EFUSE_WRITE_DATA_BIT_MASK_2) = 0;
    REG32(REG_EFUSE_WRITE_DATA_BIT_MASK_3) = 0;    

    REG32(REG_EFUSE_WRITE_DATA_0 + dataIdx * 0x4) = data;
    REG32(REG_EFUSE_WRITE_DATA_BIT_MASK_0 + dataIdx * 0x4) = mask;

    REG32(REG_EFUSE_CMD) = (REG32(REG_EFUSE_CMD) & (~0x7)) | BIT_EFUSE_CMD_ADDR(dataPort) | BIT_EFUSE_RW_CTRL;
    
    do {
        if ((REG32(REG_EFUSE_CMD) & BIT_EFUSE_CTRL_STATE) == BIT_EFUSE_CTRL_STATE) {
            DBG_MSG(COMP_EFUSE, DBG_TRACE, ("%s(%d): 0x%x, cnt:0x%02x \n", __FUNCTION__, __LINE__, REG32(REG_EFUSE_CMD), cnt));
            break;
        }
        cnt++;
        mdelay(10);
    } while(cnt < cnt_max);

    if (cnt >= cnt_max) {
        DBG_MSG(COMP_EFUSE, DBG_TRACE, ("Efuse write fail (timeout) !!!\n"));
        return STATUS_EFUSE_TIMEOUT;
    } else {
        u4Byte resultFlag = REG32(REG_EFUSE_BURN_CHECK_FAIL_0) | REG32(REG_EFUSE_BURN_CHECK_FAIL_1) |
                            REG32(REG_EFUSE_BURN_CHECK_FAIL_2) | REG32(REG_EFUSE_BURN_CHECK_FAIL_3);
        
        if (resultFlag == 0) {
            DBG_MSG(COMP_EFUSE, DBG_TRACE, ("Efuse write data done. p[%d]_[%d] = 0x%x & 0x%x, 0x%02x\n", dataPort, dataIdx, data, mask, cnt));
            return STATUS_EFUSE_SUCCESS;
        } else {
            DBG_MSG(COMP_EFUSE, DBG_TRACE, ("Efuse write fail (0x%x) !!!\n", resultFlag));
            return STATUS_EFUSE_FAIL;
        }
    }
}
#endif

//inline u1Byte load_efuse_data_to_reg(void)
u1Byte load_efuse_data_to_reg(void)
{
    u4Byte cnt = 0;
    u4Byte cnt_max = 5000;

//#ifdef CONFIG_RTL8197F
#if 1
    // patch default REG_EFUSE_TIMING_CTRL value to 0x01040A4F for RTL 8197F
    REG32(REG_EFUSE_TIMING_CTRL) = 0x01040A4F;
#endif

    REG32(REG_EFUSE_CONFIG) = REG32(REG_EFUSE_CONFIG) & (~BIT_EFUSE_CONFIG_PWR_GATE_EN);
    REG32(REG_EFUSE_CMD) = REG32(REG_EFUSE_CMD) & (~BIT_EFUSE_RW_CTRL);

    do {
        if ((REG32(REG_EFUSE_CMD) & BIT_EFUSE_CTRL_STATE) == BIT_EFUSE_CTRL_STATE) {
            break;
        }
        cnt++;
        //udelay(1);
		delay_us(1);
    } while(cnt < cnt_max);

    if (cnt < cnt_max) {
        DBG_MSG(COMP_EFUSE, DBG_MESSAGE, ("load efuse ok\n"));
        panic_printk("load efuse ok\n");
        set_rom_progress(ROM_PROGRESS_LOAD_EFUSE_TO_REG_OK);
        return STATUS_EFUSE_SUCCESS;
    } else {
        DBG_MSG(COMP_EFUSE, DBG_MESSAGE, ("load efuse fail. 0x%x(0x%x) \n", REG_EFUSE_CMD, REG32(REG_EFUSE_CMD)));
		panic_printk("load efuse fail. 0x%x(0x%x) \n", REG_EFUSE_CMD, REG32(REG_EFUSE_CMD));
        set_rom_progress(ROM_PROGRESS_LOAD_EFUSE_TO_REG_FAIL);
        return STATUS_EFUSE_FAIL;
    }
}

// dataPort: 0~7, dataIdx: 0~3
u4Byte read_efuse_byte(u4Byte idx)
{
    // efuse controller only support read 4 bytes operation. 
    u4Byte addr  = (REG_EFUSE_P0_0 + idx) & (~0x3);
    u4Byte value = REG32(addr);
    u1Byte retval = 0;

    switch(idx%4) {
        case 0:
            retval = value & 0xFF;
            break;
        case 1:
            retval = (value >> 8) & 0xFF;
            break;
        case 2:
            retval = (value >> 16) & 0xFF;
            break;
        case 3:
            retval = (value >> 24) & 0xFF;
            break;
        default:
            ;
    }

    return retval;
}

static void load_efuse_word_data(u1Byte word_enb, pu4Byte pidx, pu1Byte data)
{
    u1Byte tmpIdx = 0, j;
    for (j = 0; j < EFUSE_FORMAT_WORD_UNIT; j++) {
        if ((word_enb & (0x01<<j)) == 0) {
            data[tmpIdx] = read_efuse_byte(*pidx);
            (*pidx)++;
            tmpIdx++;

            data[tmpIdx] = read_efuse_byte(*pidx);
            (*pidx)++;
            tmpIdx++;
        } else {
            data[tmpIdx] = 0x00;
            tmpIdx++;
            data[tmpIdx] = 0x00;
            tmpIdx++;
        }
    }
}

static void read_efuse_word_data(pu1Byte data, PEFUSE_DATA pefuse_data, u4Byte loc)
{
    if (DATA_IN_THIS_OFFSET(OFFSET_EFUSE_BOOT_CTRL, loc)) {
        pefuse_data->ctrl = data[0] & 0xFF;
        DBG_MSG(COMP_EFUSE, DBG_TRACE, ("loc:0x%x, ctrl:0x%x\n", loc, pefuse_data->ctrl));
        pefuse_data->ctrl2 = data[1] & 0xFF;
	 pefuse_data->special = data[4];
        DBG_MSG(COMP_EFUSE, DBG_TRACE, ("loc:0x%x, ctrl2:0x%x\n", loc, pefuse_data->ctrl2));
    } else if (DATA_IN_THIS_OFFSET(OFFSET_EFUSE_BOOT_INIT_DDR_FROM_EXT_ADDR, loc)) {
        pefuse_data->exec_from_sram_addr = ( data[0] & 0xFF) |
                                            ( (data[1] <<  8) & 0xFF00 )   |
                                            ( (data[2] << 16) & 0xFF0000 ) |
                                            ( (data[3] << 24) & 0xFF000000 );
        DBG_MSG(COMP_EFUSE, DBG_TRACE, ("loc:0x%x, exec_from_sram_addr:0x%x\n", loc, pefuse_data->exec_from_sram_addr));
    } else if (DATA_IN_THIS_OFFSET(OFFSET_EFUSE_BOOT_UBOOT_ADDR, loc)) {
        pefuse_data->boot_addr = ( data[0] & 0xFF) |
                                    ( (data[1] <<  8) & 0xFF00 )   |
                                    ( (data[2] << 16) & 0xFF0000 ) |
                                    ( (data[3] << 24) & 0xFF000000 );
        DBG_MSG(COMP_EFUSE, DBG_TRACE, ("loc:0x%x, boot_addr:0x%x\n", loc, pefuse_data->boot_addr));
    } else if (DATA_IN_THIS_OFFSET(OFFSET_EFUSE_IMG_OFFSET, loc)) {
        pefuse_data->img_offset = ( data[0] & 0xFF) |
                                  ( (data[1] <<  8) & 0xFF00 );
        pefuse_data->heap_len = ( data[2] & 0xFF) |
                                ( (data[3] << 8) & 0xFF00);
        DBG_MSG(COMP_EFUSE, DBG_TRACE, ("loc:0x%x, img_offset:0x%x\n", loc, pefuse_data->img_offset));
        DBG_MSG(COMP_EFUSE, DBG_TRACE, ("loc:0x%x, heap_len:0x%x\n", loc, pefuse_data->heap_len));
    } else if (DATA_IN_THIS_OFFSET(OFFSET_EFUSE_DRAM, loc)) {
        pefuse_data->init_dram_val = data[0] & 0xFF;
        DBG_MSG(COMP_EFUSE, DBG_TRACE, ("loc:0x%x, init_dram_val:0x%x\n", loc, pefuse_data->init_dram_val));
    } else if (DATA_IN_THIS_OFFSET(OFFSET_EFUSE_HEAP, loc)) {
        pefuse_data->heap_start_addr = ( data[0] & 0xFF) |
                                        ( (data[1] <<  8) & 0xFF00 )   |
                                        ( (data[2] << 16) & 0xFF0000 ) |
                                        ( (data[3] << 24) & 0xFF000000 );
        DBG_MSG(COMP_EFUSE, DBG_TRACE, ("loc:0x%x, heap_addr:0x%x\n", loc, pefuse_data->heap_start_addr));
    } else if (DATA_IN_THIS_OFFSET(OFFSET_EFUSE_ERASE_SPI_NOR_FLASH_ADDR, loc)) {
        pefuse_data->erase_start_addr = ( data[0] & 0xFF) |
                                        ( (data[1] <<  8) & 0xFF00 )   |
                                        ( (data[2] << 16) & 0xFF0000 ) |
                                        ( (data[3] << 24) & 0xFF000000 );
        DBG_MSG(COMP_EFUSE, DBG_TRACE, ("loc:0x%x, erase_start_addr:0x%x\n", loc, pefuse_data->erase_start_addr));
    }
}

static void dump_efuse_content(PEFUSE_DATA pefuse_data)
{
    DBG_MSG(COMP_EFUSE, DBG_TRACE, ("ctrl:\t\t\t0x%x\n",            pefuse_data->ctrl));
    DBG_MSG(COMP_EFUSE, DBG_TRACE, ("ctrl2:\t\t\t0x%x\n",           pefuse_data->ctrl2));
    DBG_MSG(COMP_EFUSE, DBG_TRACE, ("exec_from_sram_addr:\t0x%x\n", pefuse_data->exec_from_sram_addr));
    DBG_MSG(COMP_EFUSE, DBG_TRACE, ("boot_addr:\t\t0x%x\n",         pefuse_data->boot_addr));
    DBG_MSG(COMP_EFUSE, DBG_TRACE, ("img_offset:\t\t0x%x\n",        pefuse_data->img_offset));
    DBG_MSG(COMP_EFUSE, DBG_TRACE, ("init_dram_val:\t\t0x%x\n",     pefuse_data->init_dram_val));
    DBG_MSG(COMP_EFUSE, DBG_TRACE, ("heap_start_addr:\t\t0x%x\n",   pefuse_data->heap_start_addr));
    DBG_MSG(COMP_EFUSE, DBG_TRACE, ("heap_len:\t\t0x%x\n",          pefuse_data->heap_len));
    DBG_MSG(COMP_EFUSE, DBG_TRACE, ("erase_start_addr:\t\t0x%x\n",  pefuse_data->erase_start_addr));
}

static void dump_efuse_word_data(u4Byte loc, pu1Byte data)
{
    u1Byte i;
    DBG_MSG(COMP_EFUSE, DBG_TRACE, ("0x%03x\t", loc));
    for (i = 0; i < EFUSE_FORMAT_WORD_UNIT*2; i++) {
        DBG_MSG(COMP_EFUSE, DBG_TRACE, ("%02x ", data[i]));
    }
    DBG_MSG(COMP_EFUSE, DBG_TRACE, ("\n"));
}

u1Byte load_efuse_data(PEFUSE_DATA pefuse_data, u1Byte bPrint)
{
    u4Byte i = 0;
    u1Byte efuse_val;
    u1Byte word_enb, offset;
    u1Byte offset_2_0;
    u4Byte loc;
    u1Byte efuse_temp_data[EFUSE_FORMAT_WORD_UNIT * 2];
    
    while (i < EFUSE_DATA_SIZE_MAX) {
        efuse_val = read_efuse_byte(i);
        i++;

        // 1). get header
        //      a) extened mode :  2 byte
        //      b) normal mode   :  1 byte
        if (efuse_val != 0x00) {
            if (EXT_HEADER(efuse_val)) {
                offset_2_0 = GET_HDR_OFFSET_2_0(efuse_val);

                efuse_val = read_efuse_byte(i);
                i++;

                if (efuse_val != 0x00) {
					offset = ((efuse_val & 0xF0) >> 1) | offset_2_0;
					word_enb = (efuse_val & 0x0F);

                    if (word_enb == 0xF) {
                        DBG_MSG(COMP_EFUSE, DBG_TRACE, ("%s(%d): Error: all words disabled in extended mode\n", __FUNCTION__, __LINE__));
                    }
                } else {
                    DBG_MSG(COMP_EFUSE, DBG_TRACE, ("%s(%d): Error: extended = 0xFF \n", __FUNCTION__, __LINE__));
                    break;
                }
                
            } else {
                offset = ((efuse_val >> 4) & 0x0F);
                word_enb = (efuse_val & 0x0F);
                
                if (word_enb == 0xF) {
                    DBG_MSG(COMP_EFUSE, DBG_TRACE, ("%s(%d): Error: all words disabled in normal mode\n", __FUNCTION__, __LINE__));
                }
            }
        } else {
            if (i == 1) {
                DBG_MSG(COMP_EFUSE, DBG_TRACE, ("%s(%d): eFuse is empty.\n", __FUNCTION__, __LINE__));
                set_rom_progress(ROM_PROGRESS_LOAD_EFUSE_TO_MEM_FAIL);
                return STATUS_EFUSE_EMPTY;
            } else {
                DBG_MSG(COMP_EFUSE, DBG_TRACE, ("%s(%d): read eFuse done 0x%03x bytes \n", __FUNCTION__, __LINE__, i));
                break;
            }
        }

        // 2). get data 1~4 bytes
        loc = offset * 8;

        DBG_MSG(COMP_MONITOR, DBG_TRACE, ("%s(%d): loc:0x%x, offset:0x%x, word_enb:0x%x\n", __FUNCTION__, __LINE__, loc, offset, word_enb));

        // put 8 bytes data to efuse_temp_data[]
        load_efuse_word_data(word_enb, &i, &efuse_temp_data[0]);
        read_efuse_word_data(&efuse_temp_data[0], pefuse_data, loc);

        if (bPrint == 1) {
            dump_efuse_word_data(loc, &efuse_temp_data[0]);
        }
    }

    if(bPrint == 1) {
        dump_efuse_content(pefuse_data);
    }

    set_rom_progress(ROM_PROGRESS_LOAD_EFUSE_TO_MEM_OK);
    return STATUS_EFUSE_SUCCESS;
}

