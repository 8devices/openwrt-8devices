/*
* Copyright (C) 2009 Realtek Semiconductor Corp.
* All Rights Reserved.
*
* This program is the proprietary software of Realtek Semiconductor
* Corporation and/or its licensors, and only be used, duplicated,
* modified or distributed under the authorized license from Realtek.
*
* ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER 
* THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
* 
* $Revision: 8334 $
* $Date: 2010-02-09 13:22:42 +0800 (Tue, 09 Feb 2010) $
*
* Purpose :  ASIC-level driver implementation for tables.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/

#include <rtk_types.h>
#include <rtk_api_ext.h>
#include <asicdrv/rtl8316d_types.h>
#include <rtl8316d_general_reg.h>
#include <rtl8316d_table_struct.h>
#include <rtl8316d_asicdrv_tbl.h>
#include <rtl8316d_asicDrv.h>
#include <rtl8316d_reg_struct.h>
#include <rtl8316d_debug.h>  /*SDK*/

//extern rtl8316d_table_t rtl8316d_table_list[];

#if 0
rtl8316d_table_t rtl8316d_table_list[] =
{
    {   /* table name               VLAN */
        /* access table type */     0,
        /* table size */            512,
        /* total data registers */  2,
        /* total field numbers */   7,
    },
    {   /* table name               VLAN_CAM */
        /* access table type */     1,
        /* table size */            16,
        /* total data registers */  3,
        /* total field numbers */   7,
    },
    {   /* table name               L2_TABLE */
        /* access table type */     2,
        /* table size */            8192,
        /* total data registers */  2,
        /* total field numbers */   9,
    },
    {   /* table name               FORWARDING */
        /* access table type */     3,
        /* table size */            256,
        /* total data registers */  1,
        /* total field numbers */   3,
    },
    {   /* table name               ACTION_TABLE */
        /* access table type */     4,
        /* table size */            128,
        /* total data registers */  4,
        /* total field numbers */   13,
    },
    {   /* table name               ACL_COUNTER */
        /* access table type */     5,
        /* table size */            64,
        /* total data registers */  2,
        /* total field numbers */   3,
    },
    {   /* table name               POLICER */
        /* access table type */     6,
        /* table size */            64,
        /* total data registers */  2,
        /* total field numbers */   4,
    },
    {   /* table name               PIE_TABLE */
        /* access table type */     7,
        /* table size */            128,
        /* total data registers */  5,
        /* total field numbers */   11,
    },
};
#endif

#define INCR         INDIRECT_CONTROL_FOR_CPU
#define INCR_EXE     EXECUTE_CPU
#define INCR_RW      ACCMD_CPU
#define INCR_TYPE    ACCTBTP_CPU
#define INCR_ADDR    ACCADDR_CPU
#define INDR_BASE    INDIRECT_DATA0_FOR_CPU

int8 rtl8316d_tableRead(rtk_table_list_t table, uint16 addr, uint32 *value)
{
    uint32 xdata reg_data = 0;
    uint32 busy = 0;
    uint16 xdata i;

    if ((table >= TABLE_LIST_END) || (addr >= 8192/*rtl8316d_table_list[table].size*/))
    {
        rtlglue_printf("table is %d\n", table);
        rtlglue_printf("addr is %d\n", addr);
        rtlglue_printf("TABLE_LIST_END is %d\n", TABLE_LIST_END);
        rtlglue_printf("rtl8316d_table_list[table].size is %d\n", rtl8316d_table_list[table].size);
        rtlglue_printf("rtl8316d_table_list[table].type is %d\n", rtl8316d_table_list[table].type);
        rtlglue_printf("rtl8316d_table_list[table].datareg_num is %d\n", rtl8316d_table_list[table].datareg_num);
        rtlglue_printf("rtl8316d_table_list[table].field_num is %d\n", rtl8316d_table_list[table].field_num);

        return -1;
    }
    
    reg_field_set(RTL8316D_UNIT, INCR, INCR_EXE, 1, &reg_data);    /* 1 = EXECUTE */
    reg_field_set(RTL8316D_UNIT, INCR, INCR_RW, 1, &reg_data);     /* 1 = READ */
    reg_field_set(RTL8316D_UNIT, INCR, INCR_TYPE, rtl8316d_table_list[table].type, &reg_data);
    reg_field_set(RTL8316D_UNIT, INCR, INCR_ADDR, addr, &reg_data);
    reg_write(RTL8316D_UNIT, INCR, reg_data);

    do {
        reg_field_read(RTL8316D_UNIT, INCR, INCR_EXE, &busy);
    } while (busy);
    
#ifdef RTL8316D_VERA  
#if 0
            rtlglue_printf("repeat (200) @ (posedge CLOCK);\n");
#endif
#endif  /*RTL8316D_VERA*/


    for (i=0; i<rtl8316d_table_list[table].datareg_num; i++)
    {
        reg_read(RTL8316D_UNIT, INDR_BASE+i, value+i);
    }
    
#if 0
    rtl8316d_setSWReg(INDIRECT_CONTROL_FOR_CPU_ADDR, 
                0xC0000000UL | ((uint32)rtl8316d_table_list[table].type<<13) | (addr&0x1FFF));

    do {
        reg_data = rtl8316d_getSWReg(INDIRECT_CONTROL_FOR_CPU_ADDR);
        if ((reg_data & 0x80000000UL) == 0)
        {
//            rtl8316d_setSWReg(INDIRECT_CONTROL_FOR_CPU_ADDR, 0);
            break;
        }
    } while (1);
//    printf("read table cmd:%lx\n", 0xC0000000UL | ((uint32)rtl8316d_table_list[table].type<<13) | (addr&0x1FFF));
    for (i=0; i<rtl8316d_table_list[table].datareg_num; i++)
    {
        *(value+i) = rtl8316d_getSWReg(INDIRECT_DATA0_FOR_CPU_ADDR+i*4);
//        printf("read table data%d :%lx\n", i, value[i]);
    }
#endif

    return SUCCESS;
}

int8 rtl8316d_tableWrite(rtk_table_list_t table, uint16 addr, uint32 *value)
{
    uint32 xdata reg_data = 0;
    uint32 busy;
    uint16 xdata i;

    //printf("The Value is: %lx %lx\n", value[0], value[1]);
    if (table >= TABLE_LIST_END)
        return -1;

    if (addr >= rtl8316d_table_list[table].size)
        return -1;

#ifdef RTL8316D_TOEEPROM
    rtlglue_printf("Start writing table:\n");
    rtlglue_printf("LutType: 0x%x, ", table);
    rtlglue_printf("LutAddr: 0x%04x, ", addr);
    rtlglue_printf("Data Length: %d\n", rtl8316d_table_list[table].datareg_num);
#endif

    for (i=0; i<rtl8316d_table_list[table].datareg_num; i++)
    {    
#ifdef RTL8316D_TOEEPROM
        rtlglue_printf("data[%d]:0x%08x ", i, *(value+i));
#endif
        //rtlglue_printf("rtl8316d_setSWReg(0x%x, 0x%x);\n", REG_ADDR(1, INDR_BASE+i), *(data+i));
        reg_write(RTL8316D_UNIT, INDR_BASE+i, *(value+i));
 //       rtlglue_printf("addr 0x%x, data 0x%x\n", INDR_BASE+i, *(data+i));
    }
#ifdef RTL8316D_TOEEPROM
            rtlglue_printf("\n");
#endif

    reg_field_set(RTL8316D_UNIT, INCR, INCR_EXE, 1, &reg_data);    /* 1 = EXECUTE */
    reg_field_set(RTL8316D_UNIT, INCR, INCR_RW, 0, &reg_data);     /* 0 = WRITE */
    //reg_field_set(unit, INCR, CPU_OCCUPY_TABLE, 1, &reg_data); /* 1 = CPU_OCCUPY */
    reg_field_set(RTL8316D_UNIT, INCR, INCR_TYPE, rtl8316d_table_list[table].type, &reg_data);
    reg_field_set(RTL8316D_UNIT, INCR, INCR_ADDR, addr, &reg_data);
    reg_write(RTL8316D_UNIT, INCR, reg_data);


    //rtlglue_printf("rtl8316d_setSWReg(0x%x, 0x%x);\n", REG_ADDR(1, INCR), reg_data);
//    rtlglue_printf("addr 0x%x, data 0x%x\n", INCR, *(data+i));
    

    do {
        reg_field_read(RTL8316D_UNIT, INCR, INCR_EXE, &busy);
       // rtlglue_printf("busy %d\n", busy);
    } while (busy);

#if 0
//    printf("Writetable cmd:%lx\n", 0x80000000UL | ((uint32)rtl8316d_table_list[table].type<<13) | (addr&0x1FFF));
    for (i=0; i<rtl8316d_table_list[table].datareg_num; i++)
    {
        rtl8316d_setSWReg(INDIRECT_DATA0_FOR_CPU_ADDR+i*4, value[i]);
//        printf("wt table data%d :%lx\n", i, value[i]);
    }
    rtl8316d_setSWReg(INDIRECT_CONTROL_FOR_CPU_ADDR, 
                0x80000000UL | ((uint32)rtl8316d_table_list[table].type<<13) | (addr&0x1FFF));

    do {
        reg_data = rtl8316d_getSWReg(INDIRECT_CONTROL_FOR_CPU_ADDR);
        if ((reg_data & 0x80000000UL) == 0)
        {
//            rtl8316d_setSWReg(INDIRECT_CONTROL_FOR_CPU_ADDR, 0);
            break;
        }
    } while (1);
#endif

    return SUCCESS;
}

void rtl8316d_vlanParam2Table(uint32 tbl[], rtl8316d_tbl_vlanParam_t* pVlanParam)
{
    pVlanParam->untag &= 0x1FFFFFFFUL;
    pVlanParam->fid &= 0x1F;
    pVlanParam->valid &= 0x1;
    pVlanParam->vbfwd &= 0x1;
    pVlanParam->ucslkfid &= 0x1;
    pVlanParam->member &= 0x1FFFFFFFUL;
    pVlanParam->vid &= 0xFFF;
   // printf("%s line %d:   pVlanParam->ucslkfid = %d \n", __FUNCTION__, __LINE__, pVlanParam->ucslkfid);

    tbl[0] = ((uint32)pVlanParam->untag<<7) | ((uint32)pVlanParam->fid<<2) | ((uint32)pVlanParam->valid<<1) | ((uint32)pVlanParam->vbfwd); 
    tbl[1] = ((uint32)pVlanParam->ucslkfid<<30) | ((uint32)pVlanParam->member<<5) | (((uint32)pVlanParam->vid>>7)&0x1F);  
}

void rtl8316d_vlanTable2Param(uint32 tbl[], rtl8316d_tbl_vlanParam_t* pVlanParam, uint16 index)
{
    pVlanParam->untag     =  tbl[0] >> 7;
    pVlanParam->fid       = (tbl[0] >> 2) & 0x1F;
    pVlanParam->valid     = (tbl[0] >> 1) & 0x1;
    pVlanParam->vbfwd     =  tbl[0] &  0x1; 
    pVlanParam->ucslkfid  = (tbl[1] >> 30) & 0x1;
    pVlanParam->member    = (tbl[1] >> 5) & 0x1FFFFFFUL;
    pVlanParam->vid       = ((uint16)(tbl[1] & 0x1F) << 7) | (index >> 2); 
}

static void rtl8316d_vlanCAMParam2Table(uint32 tbl[], rtl8316d_tbl_vlanParam_t* pVlanParam)
{
    pVlanParam->untag   &= 0x1FFFFFFFUL;
    pVlanParam->fid     &= 0x1F;
    pVlanParam->valid   &= 0x1;
    pVlanParam->vbfwd   &= 0x1;
    pVlanParam->ucslkfid &= 0x1;
    pVlanParam->member  &= 0x1FFFFFFFUL;
    pVlanParam->vid     &= 0xFFF;
    
    tbl[0] = ((uint32)pVlanParam->vbfwd) | ((uint32)pVlanParam->valid << 1) | ((uint32)pVlanParam->fid << 2) | ((uint32)pVlanParam->untag << 7);
    tbl[1] = ((uint32)pVlanParam->member) | ((uint32)pVlanParam->ucslkfid << 25);
    tbl[2] = ((uint32)pVlanParam->vid);
}

static void rtl8316d_vlanCAMTable2Param(uint32 tbl[], rtl8316d_tbl_vlanParam_t* pVlanParam)
{
    pVlanParam->untag   =  tbl[0] >> 7;
    pVlanParam->fid     = (tbl[0] >> 2) & 0x1F;
    pVlanParam->valid   = (tbl[0] >> 1) & 0x1;
    pVlanParam->vbfwd   =  tbl[0] &  0x1; 
    pVlanParam->ucslkfid= (tbl[1] >> 25) & 0x1;
    pVlanParam->member  =  tbl[1] & 0x1FFFFFFUL;
    pVlanParam->vid     =  tbl[2] & 0xFFF; 
}

#if 0
/*
@func uint32 | rtl8316d_addrTbl_hashAlg_get | get address table hash algorithm
@parm  void | 
@rvalue uint32 |hash algorithm|
@comm 
*/
uint8 rtl8316d_addrTbl_hashAlg_get(void)
{
    return rtl8316d_getSWRegBit(ADDRESS_TABLE_LOOKUP_CONTROL_ADDR,11);
}
#endif

rtk_api_ret_t rtl8316d_vlan_setAsicEntry(uint32 index, rtl8316d_tbl_vlanParam_t* pVlanEntry)
{
    uint32 retvalue;
    uint32  arr[2];    

    if (pVlanEntry == NULL)
        return FAILED;

    //printf("%s line %d:   pVlanEntry->ucslkfid = 0x%x \n", __FUNCTION__, __LINE__, pVlanEntry->ucslkfid);
    rtl8316d_vlanParam2Table(arr, pVlanEntry);
    retvalue = rtl8316d_tableWrite(VLAN, (uint16) index, arr);    
    return retvalue;
    
}

rtk_api_ret_t rtl8316d_vlan_getAsicEntry(uint32 index, rtl8316d_tbl_vlanParam_t* pVlanEntry)
{
    uint32 retvalue;
    uint32  arr[2];    

    if (pVlanEntry == NULL)
        return FAILED;
    
    memset(arr, 0, 2*sizeof(uint32));
    retvalue = rtl8316d_tableRead(VLAN, (uint16) index, arr);    
    rtl8316d_vlanTable2Param(arr, pVlanEntry, (uint16)index);

    return retvalue;
}

rtk_api_ret_t rtl8316d_vlan_setCamEntry(uint32 index, rtl8316d_tbl_vlanParam_t* pVlanEntry)
{
    uint32 retvalue;
    uint32  arr[3];    

    if (pVlanEntry == NULL)
        return FAILED;
    
    memset(arr, 0, 3*sizeof(uint32));       
    rtl8316d_vlanCAMParam2Table(arr, pVlanEntry);
    
    retvalue = rtl8316d_tableWrite(VLAN_CAM, (uint16) index, arr); 

    return retvalue;
}

rtk_api_ret_t rtl8316d_vlan_getCamEntry(uint32 index, rtl8316d_tbl_vlanParam_t* pVlanEntry)
{
    uint32 retvalue;
    uint32  arr[3];    

    if (pVlanEntry == NULL)
        return FAILED;
    
    memset(arr, 0, 3*sizeof(uint32));
    
    retvalue = rtl8316d_tableRead(VLAN_CAM, (uint16) index, arr);    
    rtl8316d_vlanCAMTable2Param(arr, pVlanEntry);

    //printf("VCAM index: %ld, vid: %ld, valid: %bd\n", index, (uint32)pVlanEntry->vid, pVlanEntry->valid);

    return retvalue;
}

