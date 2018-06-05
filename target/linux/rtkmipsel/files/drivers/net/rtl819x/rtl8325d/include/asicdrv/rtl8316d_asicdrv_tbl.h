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
* Purpose :  Table ASIC driver related Macro, structure, enumeration, 
*                variable types declaration and definition.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/

#ifndef __RTL8316D_ASICDRV_DRV_H__
#define __RTL8316D_ASICDRV_TBL_H__

#define RTL8316D_VLAN_NUMBER    512
#define RTL8316D_VLANCAM_NUMBER  16
#define RTL8316D_ADDRTBL_NUMBER   8192
#define RTL8316D_FWD_NUMBER   256

#ifdef _LITTLE_ENDIAN
/*keil c is Big endian*/
#undef _LITTLE_ENDIAN
#endif

#if 0
typedef enum rtl8316d_table_list_e
{
    VLAN = 0,
    VLAN_CAM,
    L2_TABLE,
    FORWARDING,
    ACTION_TABLE,
    ACL_COUNTER,
    POLICER,
    PIE_TABLE,
    TABLE_LIST_END = 8
} rtl8316d_table_list_t;


typedef struct rtl8316d_table_s
{
    uint8 type;              /* access table type */
    uint16 size;              /* table size */
    uint8 datareg_num;       /* total data registers */
    uint8 field_num;         /* total field numbers */
} rtl8316d_table_t;
#endif

typedef struct rtl8316d_tbl_vlanParam_s
{
    uint32 untag ;  /*untag set*/
    uint8 fid;     /*filtering database ID*/
    uint8 valid;
    uint8 vbfwd;  /*VLAN based forwarding. 0: based on inner VID, 1: based on outer VID*/
    uint8 ucslkfid;   /*unicast DA lookup use FID or VID, 0: VID, 1: FID*/
    uint32 member;   /*member port*/
    uint16 vid;
}rtl8316d_tbl_vlanParam_t;

/***************address talbe***************************************/

#if 0
typedef struct LUTTABLE{

    ipaddr_t sip;
    ipaddr_t dip;
    ether_addr_t mac;

    uint16 type : 1;            /*0: MAC+FID; 1: SIP+DIP*/
    uint16 isftidx : 1;         /*0: single port; 1: multiple ports*/
    uint16 port : 5;            /*destination port*/
    uint16 sa_blk : 1;          /*source MAC blocked*/
    uint16 auth : 1;
    uint16 static_bit : 1;
    uint16 age : 3;

    uint16 ftidx : 8;       /*forwarding table index*/
    
    uint16 fid : 12;
    
}rtl8316d_luttb;

#endif

/*address table lookup entry*/
typedef struct rtl8316d_tblasic_l2Ucst_s
{

#ifndef _LITTLE_ENDIAN
    /*big-endian*/
    uint32 aed48_33 : 16;
    uint32 aed32_17 : 16;

    uint32 aed16_1  : 16;
    uint32 aed0     : 1;
    uint32 aet      : 1;
    uint32 isftidx  : 1;
    uint32 portnum  : 5;
    uint32 sablk    : 1;
    uint32 macauth  : 1;
    uint32 stat   : 1;
    uint32 age      : 2;
    uint32 reserved : 3;

#else /*little endian*/
    uint32 aed32_17 : 16;
    uint32 aed48_33 : 16;

    uint32 reserved : 3;
    uint32 age      : 2;
    uint32 stat   : 1;
    uint32 macauth  : 1;
    uint32 sablk    : 1;
    uint32 portnum  : 5;
    uint32 isftidx  : 1;
    uint32 aet      : 1;
    uint32 aed0     : 1;
    uint32 aed16_1  : 16;
#endif
}rtl8316d_tblasic_l2Ucst_t;

/*address table lookup entry*/
typedef struct rtl8316d_tblasic_l2Mcst_s
{

#ifndef _LITTLE_ENDIAN
    /*big-endian*/
    uint32 aed48_33 : 16;
    uint32 aed32_17 : 16;

    uint32 aed16_1  : 16;
    uint32 aed0     : 1;
    uint32 aet      : 1;
    uint32 isftidx  : 1;
    uint32 ftidx    : 8;
    uint32 reserved : 5;

#else /*little endian*/
    uint32 aed32_17 : 16;
    uint32 aed48_33 : 16;

    uint32 reserved : 5;
    uint32 ftidx    : 8;
    uint32 isftidx  : 1;
    uint32 aet      : 1;
    uint32 aed0     : 1;
    uint32 aed16_1  : 16;
#endif
}rtl8316d_tblasic_l2Mcst_t;


/*address table lookup entry*/
typedef struct rtl8316d_tblasic_lutParam_s
{
    uint8 aet ; /*address entry type. 0:FID+MAC, 1:SIP+DIP*/
    uint8 isftidx; /*0: port number, 1: forwarding table index */    

    uint16 row;
    uint8 column;
    
    union {
        struct {
            ether_addr_t mac;  
            uint16 fid; /*robin*/               
        }ether;
        struct {
            uint32 grpip;
            uint32 srcip;
        }ip;
    }key;
    
    union {
        struct{
        uint8 ftindex;
        uint8 reserved;
        }ftidx;
        struct {
            /*8051 keil C  Big Endian Order*/   
            uint16 reserved : 6;
            uint16 age      : 2;
            uint16 stat     : 1;
            uint16 macauth  : 1;
            uint16 sablk    : 1;
            uint16 portnum : 5;         
        }mac;
    }info;

    uint8 hashAlg;
  //  uint64 aed;
}rtl8316d_tblasic_lutParam_t;

#define lutEther   key.ether
#define lutIp       key.ip
#define lutInfo     info.mac

/*forwarding table*/
typedef struct rtl8316d_tblasic_fwdTbl_s
{

#ifndef _LITTLE_ENDIAN
    /*big-endian*/
    uint32 reserved : 4;
    uint32 portmask : 25;
    uint32 crsvlan  : 1;
    uint32 age      : 2;

#else /*little endian*/
    uint32 age      : 2;
    uint32 crsvlan  : 1;
    uint32 portmask : 25;
    uint32 reserved : 4;
#endif
}rtl8316d_tblasic_fwdTbl_t;

typedef struct rtl8316d_tblasic_fwdTblParam_s
{
    uint32 reserved : 4;
    uint32 portmask : 25;
    uint32 crsvlan  : 1;
    uint32 age      : 2;
}rtl8316d_tblasic_fwdTblParam_t;

typedef struct rtl8316d_lut_opeartions_s{
    uint16 opType : 1; /*0: clear, 1:search*/
    uint16 defPortNum:1;
    uint16 defFid : 1;
    uint16 fid: 12;
    uint16 cpuown: 1;
    uint16 defMac : 1;
    uint16 found : 1;   
    uint16 portNum : 5;
    uint16 startIdx : 13; 
    uint16 lastIdx : 13; /*clear to here, or index to the entry found*/

    ether_addr_t mac;
} rtl8316d_lut_opeartions_t;


/**************************************vlan table*******************************/
/*VLAN table*/

typedef struct rtl8316d_tblasic_vlanTbl_s
{

    /*big-endian*/
    uint32 untag ;      /*untag set*/
    uint8 fid;             /*filtering database ID*/
    uint8 valid;
    uint8 vbfwd ;       /*VLAN based forwarding. 0: based on inner VID, 1: based on outer VID*/

    uint8 reserved ;
    uint8 ucslkfid;      /*unicast DA lookup use FID or VID, 0: VID, 1: FID*/
    uint32 member;   /*member port*/
    uint8 vidmsb;      /*left-most 5-bit of VID*/
} rtl8316d_tblasic_vlanTbl_t;

extern int8 rtl8316d_tableRead(rtk_table_list_t table, uint16 addr, uint32 *value);
extern int8 rtl8316d_tableWrite(rtk_table_list_t table, uint16 addr, uint32 *value);

extern void rtl8316d_vlanParam2Table(uint32 tbl[], rtl8316d_tbl_vlanParam_t* pVlanParam);
extern void rtl8316d_vlanTable2Param(uint32 tbl[], rtl8316d_tbl_vlanParam_t* pVlanParam, uint16 index);

extern int16 rtl8316d_addrTbl_hashAlg_get(uint32* hashAlg);
extern void rtl8316d_addrTbl_hashAlg_set(uint32 hashAlg);
extern int8 rtl8316d_addrTbl_entry_reset(uint16 start, uint16 end);
extern uint16 rtl8316d_addrTbl_hash(uint8 hashAlg, rtl8316d_tblasic_l2Ucst_t* l2uentry, rtl8316d_tblasic_lutParam_t* lutEntry);
extern int8 rtl8316d_addrTbl_setAsicEntry(uint8 hashAlg, rtl8316d_tblasic_lutParam_t* lutEntry);
extern int8 rtl8316d_addrTbl_getAsicEntry(uint8 hashAlg, rtl8316d_tblasic_lutParam_t* lutEntry);
extern int8 rtl8316d_addrTbl_entry_fourceAdd(uint8 hashAlg, uint16 index, rtl8316d_tblasic_lutParam_t* lutEntry);

#define  RTL8316D_SDK    1

#ifdef  RTL8316D_SDK
extern rtk_api_ret_t rtl8316d_fwd_getAsicEntry(uint16 index, rtl8316d_tblasic_fwdTblParam_t * fwd);
extern rtk_api_ret_t rtl8316d_fwd_setAsicEntry(uint16 index, rtl8316d_tblasic_fwdTblParam_t * fwd);
#endif

extern int8 rtl8316d_addrTbl_op(rtl8316d_lut_opeartions_t* lutOp);
extern rtk_api_ret_t rtl8316d_vlan_setAsicEntry(uint32 index, rtl8316d_tbl_vlanParam_t* pVlanEntry);
extern rtk_api_ret_t rtl8316d_vlan_getAsicEntry(uint32 index, rtl8316d_tbl_vlanParam_t* pVlanEntry);
extern rtk_api_ret_t rtl8316d_vlan_setCamEntry(uint32 index, rtl8316d_tbl_vlanParam_t* pVlanEntry);
extern rtk_api_ret_t rtl8316d_vlan_getCamEntry(uint32 index, rtl8316d_tbl_vlanParam_t* pVlanEntry);

#endif /*__RTL8316D_ASICDRV_TBL_H__*/
