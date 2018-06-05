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
* $Revision:  $
* $Date: $
*
* Purpose : ASIC-level driver header for ingress Access Control List.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/

#ifndef __RTL8316D_ASICDRV_INACL_H__
#define __RTL8316D_ASICDRV_INACL_H__

#define RTL8316D_ACLFIELD_MAX   9
#define RTL8316D_PIEPHY_BLKNUM 2
#define RTL8316D_PIEPHY_BLKENTRY    64
#define RTL8316D_PIELOGIC_BLKNUM   4
#define RTL8316D_PIELOGIC_BLKENTRY  32
#define RTL8316D_ACLACT_ENTRYNUM (RTL8316D_PIEPHY_BLKNUM*RTL8316D_PIEPHY_BLKENTRY)
#define RTL8316D_PIE_ENTRYNUM  (RTL8316D_PIEPHY_BLKNUM*RTL8316D_PIEPHY_BLKENTRY)
#define RTL8316D_ACLRULEID_AUTO     0xFF
#define RTL8316D_ACLACTINFO_LENGTH         0x3

typedef struct rtl8316d_asicPieTempl_fmt_s
{
#ifndef _LITTLE_ENDIAN
    uint16 noneZeroOff : 1;
    uint16 recvPort    : 5;
    uint16 tgL2Fmt     : 2;
    uint16 itagExist   : 1;
    uint16 otagExist   : 1;
    uint16 tgL23Fmt    : 2;
    uint16 tgL4Fmt     : 3;
    uint16 ispppoe     : 1;
#else
    uint16 ispppoe     : 1;
    uint16 tgL4Fmt     : 3;
    uint16 tgL23Fmt    : 2;
    uint16 otagExist   : 1;
    uint16 itagExist   : 1;
    uint16 tgL2Fmt     : 2;
    uint16 recvPort    : 5;
    uint16 noneZeroOff : 1;
#endif
}rtl8316d_asicPieTempl_fmt_t;



typedef struct rtl8316d_asicPieTempl_otag_s
{
#ifndef _LITTLE_ENDIAN
    uint16 opri : 3;
    uint16 dei    : 1;
    uint16 ovid     : 12;
#else
    uint16 ovid     : 12;
    uint16 dei    : 1;
    uint16 opri : 3;
#endif
}rtl8316d_asicPieTempl_otag_t;



typedef struct rtl8316d_asicPieTempl_itag_s
{
#ifndef _LITTLE_ENDIAN
    uint16 ipri : 3;
    uint16 itagExist    : 1;
    uint16 ivid     : 12;
#else
    uint16 ivid     : 12;
    uint16 itagExist    : 1;
    uint16 ipri : 3;
#endif
}rtl8316d_asicPieTempl_itag_t;



typedef struct rtl8316d_asicPieTempl_prio_s
{
#ifndef _LITTLE_ENDIAN
    uint16 itagPri  : 3;
    uint16 itagExist : 1;
    uint16 otagPri : 3;
    uint16 dei : 1;
    uint16 otagExist : 1;
    uint16 fwdPri    : 3;
    uint16 rsved : 2;
    uint16 oampdu : 1;
    uint16 cfi    : 1;    
#else
    uint16 cfi    : 1;    
    uint16 oampdu : 1;
    uint16 rsved : 2;
    uint16 fwdPri    : 3;
    uint16 otagExist : 1;
    uint16 dei : 1;
    uint16 otagPri : 3;
    uint16 itagExist : 1;
    uint16 itagPri  : 3;
#endif
}rtl8316d_asicPieTempl_prio_t;


typedef struct rtl8316d_asicPieTempl_ipHdr_s
{
#ifndef _LITTLE_ENDIAN
    uint16 dscp  : 6;
    uint16 morefrag : 1;
    uint16 noneZeroOff : 1;
    uint16 l4Proto : 8;
#else
    uint16 l4Proto : 8;
    uint16 noneZeroOff : 1;
    uint16 morefrag : 1;
    uint16 dscp  : 6;
#endif
}rtl8316d_asicPieTempl_ipHdr_t;

#if 0
typedef struct rtl8316d_asicPieTempl_grpIp0_s
{
    uint16 grpIp15_0;
}rtl8316d_asicPieTempl_grpIp0_t;

typedef struct rtl8316d_pieTempl_grpIp0Param_s
{
    uint16 grpIp15_0;
}rtl8316d_pieTempl_grpIp0Param_t;
#endif

typedef struct rtl8316d_asicPieTempl_grpIp1_s
{
#ifndef _LITTLE_ENDIAN
        uint16 reserv  : 4;
        uint16 grpIp28_16 : 12;
#else
        uint16 grpIp28_16 : 12;
        uint16 reserv  : 4;
#endif
}rtl8316d_asicPieTempl_grpIp1_t;



typedef struct rtl8316d_asicPieTempl_l4Hdr_s
{
#ifndef _LITTLE_ENDIAN
        uint16 tcpFlags  : 6;
        uint16 reserv    : 2;
        uint16 igmpType  : 8;
#else
        uint16 igmpType  : 8;
        uint16 reserv    : 2;
        uint16 tcpFlags  : 6;
#endif
}rtl8316d_asicPieTempl_l4Hdr_t;

typedef struct rtl8316d_asicPieTempl_icmpHdr_s
{
#ifndef _LITTLE_ENDIAN
        uint16 icmpCode  : 8;
        uint16 icmpType  : 8;
#else
        uint16 icmpType  : 8;
        uint16 icmpCode  : 8;
#endif
}rtl8316d_asicPieTempl_icmpHdr_t;



typedef struct rtl8316d_asicPieTempl_rcvPortMask0_s
{
    uint16 rcvPortMask15_0;
}rtl8316d_asicPieTempl_rcvPortMask0_t;


typedef struct rtl8316d_asicPieTempl_rcvPortMask1_s
{
#ifndef _LITTLE_ENDIAN
        uint16 rcvPortMask24_16  : 9;
        uint16 dmacType  : 2;
        uint16 doNotFrag  : 1;
        uint16 ttlType  : 2;
        uint16 rtkpp  : 2;
#else
        uint16 rtkpp  : 2;
        uint16 ttlType  : 2;
        uint16 doNotFrag  : 1;
        uint16 dmacType  : 2;
        uint16 rcvPortMask24_16  : 9;
#endif
}rtl8316d_asicPieTempl_rcvPortMask1_t;


typedef struct rtl8316d_asicPieTempl_vidRange_s
{
    uint16 vidRangeChk;
}rtl8316d_asicPieTempl_vidRange_t;


typedef struct rtl8316d_asicPieTempl_rangeTbl_s
{
#ifndef _LITTLE_ENDIAN
        uint16 l4Port  : 4;
        uint16 rcvPort  : 4;
        uint16 ipRange  : 8;
#else
        uint16 ipRange  : 8;
        uint16 rcvPort  : 4;
        uint16 l4Port  : 4;
#endif
}rtl8316d_asicPieTempl_rangeTbl_t;


typedef struct rtl8316d_asicPieTempl_field_s
{
    union {
        rtl8316d_asicPieTempl_fmt_t format;
        uint16                      dmac0;
        uint16                      dmac1;
        uint16                      dmac2;
        uint16                      smac0;
        uint16                      smac1;
        uint16                      smac2;
        uint16                      ethType;
        rtl8316d_asicPieTempl_otag_t otag;
        rtl8316d_asicPieTempl_itag_t itag;
        rtl8316d_asicPieTempl_prio_t prio;
        uint16                      l2Proto;
        uint16                       sip15_0;
        uint16                       sip31_16;
        uint16                       dip15_0;
        uint16                       dip31_16;
        rtl8316d_asicPieTempl_ipHdr_t ipHdr;
        uint16                       grpIp15_0;        
        rtl8316d_asicPieTempl_grpIp1_t grpIph;
        uint16                        l4SrcPort;
        uint16                        l4DstPort;      
        rtl8316d_asicPieTempl_l4Hdr_t l4Hdr;
        rtl8316d_asicPieTempl_icmpHdr_t icmpHdr;
        rtl8316d_asicPieTempl_rcvPortMask0_t rcvSrcPm0;
        rtl8316d_asicPieTempl_rcvPortMask1_t rcvSrcPm1;
        rtl8316d_asicPieTempl_vidRange_t vidRange;
        rtl8316d_asicPieTempl_rangeTbl_t rangeTbl;
        uint16                        payload;               
    }un;
}rtl8316d_asicPieTempl_field_t;


typedef struct rtl8316d_tblAsic_aclOtagop_s
{
#ifndef _LITTLE_ENDIAN
    uint32 withdraw:1;
    /*   0: reserved, 
      *   1: provide new outer VID as OutVIDInfo. 
      *   2: packet's new outer VID = packet's outer tag VID - OutVIDInfo
      *   3: packet's new outer VID = packet's outer tag VID + OutVIDInfo
      */
    uint32 outVidCtl:2;  
    uint32 outVidInfo:12;
    /*   0: packet sent without outer VLAN tag, 1: packet sent with outer VLAN tag
      *  others: Nop 
      */
    uint32 outTagOp:2;
    uint32 reserved:15;


#else
    uint32 reserved:15;
    uint32 outTagOp:2;
    /*   0: packet sent without outer VLAN tag, 1: packet sent with outer VLAN tag
      *  others: Nop 
      */
    uint32 outVidInfo:12;
    /*   0: reserved, 
     *    1: provide new outer VID as OutVIDInfo. 
     *    2: packet's new outer VID = packet's outer tag VID - OutVIDInfo
     *    3: packet's new outer VID = packet's outer tag VID + OutVIDInfo
     */
    uint32 outVidCtl:2;
    uint32 withdraw:1;

#endif
} rtl8316d_tblAsic_aclOtagop_t;

typedef struct rtl8316d_aclAct_otagopParam_s
{
    uint32 withdraw;
    /*   0: reserved, 
     *1: provide new outer VID as OutVIDInfo. 
     *2: packet's new outer VID = packet's outer tag VID - OutVIDInfo
     *3: packet's new outer VID = packet's outer tag VID + OutVIDInfo
     */

    uint32 outVidCtl:2;
    uint32 outVidInfo:12;
    /*0: packet sent without outer VLAN tag, 1: packet sent with outer VLAN tag
      *others: Nop 
      */
    uint32 outTagOp:2;
} rtl8316d_aclAct_otagopParam_t;




typedef struct rtl8316d_tblAsic_aclIntagop_s
{
#ifndef _LITTLE_ENDIAN
    uint32 withdraw:1;
    uint32 inVidCtl:2;
    uint32 inVidInfo:12;
    uint32 inTagOp:2;
    uint32 reserved:15;
#else
    uint32 reserved:15;
    uint32 inTagOp:2;
    uint32 inVidInfo:12;
    uint32 inVidCtl:2;
    uint32 withdraw:1;
#endif
} rtl8316d_tblAsic_aclIntagop_t;

typedef struct rtl8316d_aclAct_itagopParam_s
{
    uint32 withdraw:1;
    uint32 inVidCtl:2;
    uint32 inVidInfo:12;
    uint32 inTagOp:2;
    uint32 reserved:15;
} rtl8316d_aclAct_itagopParam_t;



typedef struct rtl8316d_tblAsic_aclRedirect_s
{
#ifndef _LITTLE_ENDIAN
    union
    {
        struct
        {
            uint32 withdraw:1;   /*1: withdraw */
            uint32 opcode:1;     /*0: uniRedirect/trap to CPU/drop/flood, 1: multiRedirect, 2: uniRoute, 3: multi Route */
            uint32 reserved0:1;
            uint32 dpn:5;           /*destination port number, 30 means drop, 31 means flood within VLAN */
            uint32 reserved:24;
        } uniRedirect;
        struct
        {
            uint32 withdraw:1;
            uint32 opcode:1;
            uint32 ftIdx:8;
            uint32 reserved:22;
        } multiRedirect;
    } redirect;

#else
    union
    {
       struct
        {
            uint32 reserved:24;
            uint32 dpn:5;         /*destination port number, 30 means drop, 31 means flood within VLAN */
            uint32 reserved0:1;
            uint32 opcode:1;    /*0: uniRedirect/trap to CPU/drop/flood, 1: multiRedirect, 2: uniRoute, 3: multi Route */
            uint32 withdraw:1; /*1: withdraw */
        } uniRedirect;
        struct
        {
            uint32 reserved:22;
            uint32 ftIdx:8;
            uint32 opcode:1;
            uint32 withdraw:1;
        } multiRedirect;
    } redirect;

#endif

} rtl8316d_tblAsic_aclRedirect_t;


typedef struct rtl8316d_aclAct_redirectParam_s
{
    uint32 withdraw;
    uint32 opcode : 1; /*0:uniredirect 1:multiRedirect*/
    union
    {
        struct
        {
            uint32 dpn:5;   /*destination port number, 30 means drop, 31 means flood within VLAN */
        } uniRedirect;
        struct
        {
            uint32 ftIdx;
        } multiRedirect;
    } redirect;
} rtl8316d_aclAct_redirectParam_t;


typedef struct rtl8316d_tblAsic_aclPrioRmk_s
{
#ifndef _LITTLE_ENDIAN
    uint32 withdraw:1;
    uint32 tagSel:3;    /*xx1:inner tag, x1x:outer tag, 1xx:dei */
    uint32 inPri:3;      /*1: remark inner priority */
    uint32 outPri:3;    /*1: remark outer priority */
    uint32 dei:1;       /*DEI field of outer VLAN tag, used only when rmkDei is 1. */
    uint32 reserved:21;
#else
    uint32 reserved:21;
    uint32 dei:1;       /*DEI field of outer VLAN tag, used only when rmkDei is 1. */
    uint32 outPri:3;   /*1: remark outer priority */
    uint32 inPri:3;     /*1: remark inner priority */
    uint32 tagSel:3;  /*xx1:inner tag, x1x:outer tag, 1xx:dei */
    uint32 withdraw:1;
#endif
} rtl8316d_tblAsic_aclPrioRmk_t;

typedef struct rtl8316d_aclAct_prioRmkParam_s
{
    uint32 withdraw:1;
    uint32 tagSel:3;    /*xx1:inner priority, x1x:outer priority, 1xx:dei */
    uint32 inPri:3;       /*1: remark inner priority */
    uint32 outPri:3;    /*1: remark outer priority */
    uint32 dei:1;       /*DEI field of outer VLAN tag, used only when rmkDei is 1. */
} rtl8316d_aclAct_prioRmkParam_t;


typedef struct rtl8316d_tblAsic_aclDscpRmk_s
{
#ifndef _LITTLE_ENDIAN
    union
    {
        struct
       {
            uint32 withdraw:1;
            uint32 opcode:2;    /*0: DSCP remarking, 1: modify IP precedence, 2: modify DTR */
            uint32 acldscp:6;
            uint32 reserved:23;
        } dscp;
        struct
        {
            uint32 withdraw:1;
            uint32 opcode:2;
            uint32 ipPrece:3;
            uint32 reserved0:3;
            uint32 reserved:23;
    } ipPrec;
    struct
    {
        uint32 withdraw:1;
        uint32 opcode:2;
        uint32 reserved0:3;
        uint32 dtr:3;
        uint32 reserved:23;
    } dtr;
    } dscpRmk;
#else
    union
    {
        struct
        {
            uint32 reserved:23;
            uint32 acldscp:6;
            uint32 opcode:2; /*0: DSCP remarking, 1: modify IP precedence, 2: modify DTR */
            uint32 withdraw:1;
        } dscp;
        struct
        {
            uint32 reserved:23;
            uint32 reserved0:3;
            uint32 ipPrece:3;
            uint32 opcode:2;
            uint32 withdraw:1;
        } ipPrec;
        struct
        {
            uint32 reserved:23;
            uint32 dtr:3;
            uint32 reserved0:3;
            uint32 opcode:2;
            uint32 withdraw:1;
        } dtr;
    } dscpRmk;
#endif
} rtl8316d_tblAsic_aclDscpRmk_t;

typedef struct rtl8316d_aclAct_dscpRmkParam_s
{
    uint32 withdraw;
    uint32 opcode;  /*0: DSCP remarking, 1: modify IP precedence, 2: modify DTR */
    union
    {
        struct
        {
            uint32 acldscp:6;
        } dscp;
        struct
        {
            uint32 ipPrece:3;
        } ipPrec;
        struct
        {
            uint32 dtr:3;
        } dtr;
    } dscpRmk;
} rtl8316d_aclAct_dscpRmkParam_t;



typedef struct rtl8316d_tblAsic_aclPriority_s
{
#ifndef _LITTLE_ENDIAN
    uint32 withdraw: 1;
    uint32 priority: 3;
    uint32 reserved: 28;
#else
    uint32 reserved: 28;
    uint32 priority: 3;
    uint32 withdraw: 1;
#endif
} rtl8316d_tblAsic_aclPriority_t;

typedef struct rtl8316d_aclAct_prioParam_s
{
    uint32 withdraw: 1;
    uint32 priority: 3;
} rtl8316d_aclAct_prioParam_t;

/*ACL action table*/
typedef struct rtl8316d_tblasic_aclActTbl_s
{

#ifndef _LITTLE_ENDIAN
    /*big-endian*/
    uint32 fno  : 1;  /*flow number*/
    uint32 drop    : 2;     /*00b:permit; 01b:drop; 10b: withdraw drop; 11: reserved*/
    uint32 copytocpu  : 1;
    uint32 mirror      : 1; 
    uint32 otag      : 1;       /*outer tag operation*/
    uint32 itag      : 1;       /*inner tag operation*/
    uint32 priormk     : 1;     /*priority/DEI remarking*/
    uint32 redir        : 1;    /*redirection*/
    uint32 dscprmk      : 1;    /*DSCP remarking*/
    uint32 prioasn     : 1;    /*forwarding priority (tx queue) assignment*/
    uint32 reserved     : 21;
        
    uint32 actinfo[3];
#else /*little endian*/

    uint32 reserved     : 21;
    uint32 prioasn     : 1;    /*forwarding priority (tx queue) assignment*/
    uint32 dscprmk      : 1;    /*DSCP remarking*/
    uint32 redir        : 1;    /*redirection*/
    uint32 priormk     : 1;     /*priority/DEI remarking*/
    uint32 itag      : 1;       /*inner tag operation*/
    uint32 otag      : 1;       /*outer tag operation*/
    uint32 mirror      : 1; 
    uint32 copytocpu  : 1;
    uint32 drop    : 2;     /*00b:permit; 01b:drop; 10b: withdraw drop; 11: reserved*/
    uint32 fno      : 1;  /*flow number*/
        
    uint32 actinfo[3];
#endif
}rtl8316d_tblasic_aclActTbl_t; 

/*ACL action table*/
typedef struct rtl8316d_aclAct_entry_s
{
    uint32 fno  : 1;            /*flow number*/
    uint32 drop    : 2;         /*00b:permit; 01b:drop; 10b: withdraw drop; 11: reserved*/
    uint32 copytocpu  : 1;
    uint32 mirror      : 1; 
    uint32 otag      : 1;       /*outer tag operation*/
    uint32 itag      : 1;       /*inner tag operation*/
    uint32 priormk     : 1;     /*priority/DEI remarking*/
    uint32 redir        : 1;    /*redirection*/
    uint32 dscprmk      : 1;    /*DSCP remarking*/
    uint32 prioasn     : 1;     /*forwarding priority (tx queue) assignment*/
    uint32 actNum     : 4;
    uint32 reserved     : 17;
        
    rtl8316d_aclAct_otagopParam_t outTagOpInfo;
    rtl8316d_aclAct_itagopParam_t inTagOpInfo;
    rtl8316d_aclAct_prioRmkParam_t PriRmkInfo;
    rtl8316d_aclAct_redirectParam_t redirInfo;
    rtl8316d_aclAct_dscpRmkParam_t dscpRmkInfo;
    rtl8316d_aclAct_prioParam_t prioInfo;
}rtl8316d_aclAct_entry_t; 





typedef struct rtl8316d_aclEntry_s
{
    rtk_filter_aclRule_field_t ruleField[RTL8316D_ACLFIELD_MAX];
    rtl8316d_aclAct_entry_t* pAction;
}rtl8316d_aclEntry_t;


/**************************policer***********************/
/*ACL policer table*/
typedef struct rtl8316d_tblasic_policer_s
{

#ifndef _LITTLE_ENDIAN
    /*big-endian*/
    uint32 type         : 1;     /*0: counter; 1: policer*/
    uint32 reserved0    : 5;     
    uint32 tokencnt     : 18;      /*policer token counter*/
    uint32 threshold      : 8; 
        
    uint32 reserved1 : 16;       
    uint32 rate     : 16;    
#else /*little endian*/
    uint32 threshold      : 8; 
    uint32 tokencnt     : 18;      /*policer token counter*/
    uint32 reserved0    : 5;     
    uint32 type         : 1;     /*0: counter; 1: policer*/

    uint32 rate         : 16;    
    uint32 reserved1 : 16;       
#endif
}rtl8316d_tblasic_policer_t; 


/*ACL policer table*/
typedef struct rtl8316d_tblasic_policerParam_s
{
    uint32 type         : 1;     /*0: counter; 1: policer*/
    uint32 reserved0    : 5;     
    uint32 tokencnt     : 18;      /*policer token counter*/
    uint32 threshold      : 8; 
        
    uint32 reserved1 : 16;       
    uint32 rate     : 16;   

}rtl8316d_tblasic_policerParam_t; 

/*ACL counter table*/
typedef struct rtl8316d_tblasic_aclCounter_s
{

#ifndef _LITTLE_ENDIAN
    /*big-endian*/
    uint32 type         : 1;     /*0: counter; 1: policer*/
    uint32 reserved0    : 20;     
    uint32 cntmode      : 1;      /*counter mode, 0: packet-based counter; 1: byte-based counter*/
    uint32 counterH      : 10;   /*MSB 10-bit of counter*/
        
    uint32 counterL;            /*LSB 32-bit of counter*/  
#else /*little endian*/
    uint32 counterH      : 10;   /*MSB 10-bit of counter*/
    uint32 cntmode      : 1;      /*counter mode, 0: packet-based counter; 1: byte-based counter*/
    uint32 reserved0    : 20;     
    uint32 type         : 1;     /*0: counter; 1: policer*/

    uint32 counterL;            /*LSB 32-bit of counter*/  
#endif
}rtl8316d_tblasic_aclCounter_t; 

/*ACL counter table*/
typedef struct rtl8316d_tblasic_aclCounterParam_s
{
    uint32 type         : 1;     /*0: counter; 1: policer*/
    uint32 reserved0    : 20;     
    uint32 cntmode      : 1;      /*counter mode, 0: packet-based counter; 1: byte-based counter*/
    uint32 counterH         :10;
    uint32 counterL;  
}rtl8316d_tblasic_aclCounterParam_t; 


typedef struct rtl8316d_tblAsic_l4PortRangeTable_s
{
#ifndef _LITTLE_ENDIAN
       /*word 0*/
        uint32   upperBound : 16;
        uint32   lowerBound : 16;

        /*word 1*/    
        uint32  reserve0 : 29;
        uint32  destPort : 1; /*0: compare source Layer4 port, 1: compare destination Layer4 port*/
        uint32  type : 2;      /*00: Entry invalid; 01: compare TCP port range; 10: compare UDP port range; 11 compare TCP or UDP port range.*/

#else  /*_LITTLE_ENDIAN*/
      /*word 0*/
        uint32 lowerBound : 16;
        uint32 upperBound : 16;

      /*word 1*/
        uint32  type    : 2; /*00: Entry invalid; 01: compare TCP port range; 10: compare UDP port range; 11 compare TCP or UDP port range.*/
        uint32  destPort : 1;
        uint32 reserve0 : 29;

#endif /*_LITTLE_ENDIAN*/
}  rtl8316d_tblAsic_l4PortRangeTable_t;


typedef struct rtl8316d_tblAsic_l4PortRangeParam_s
{

    uint16  type    : 2; /*00: Entry invalid; 01: compare TCP port range; 10: compare UDP port range; 11 compare TCP or UDP port range.*/
    uint16  isDestPort : 1;

    uint16  lowerBound;
    uint16  upperBound;
}  rtl8316d_tblAsic_l4PortRangeParam_t;


typedef struct rtl8316d_tblAsic_vidRangeTable_s
{
#ifndef _LITTLE_ENDIAN
    /*word 0*/
    uint32  reserved0  : 6;
    uint32  upperBound      : 12;
    uint32  lowerBound      : 12;
    uint32  type : 2; /*00: entry invalid; 01 compare C-VID only; 10: compare S-VID only; 11: compare either C-VID or S-VID.*/

#else
    /*word 0*/
    uint32 type : 2; /*00: entry invalid; 01 compare C-VID only; 10: compare S-VID only; 11: compare either C-VID or S-VID.*/
    uint32 lowerBound      : 12;
    uint32 upperBound      : 12;
    uint32 reserved0  : 6;

#endif
} rtl8316d_tblAsic_vidRangeTable_t;

typedef struct rtl8316d_tblAsic_vidRangeParam_s
{
    uint32 type ; /*00: entry invalid; 01 compare C-VID only; 10: compare S-VID only; 11: compare either C-VID or S-VID.*/
    uint32 upperBound ;
    uint32 lowerBound ;
    
} rtl8316d_tblAsic_vidRangeParam_t;


typedef struct rtl8316d_tblAsic_ipRangeTable_s{
#ifndef _LITTLE_ENDIAN
        uint32 lowerBound;
        uint32 upperBound;

        uint32 reserved : 30;
        uint32 type     : 2;/*0: invalid; 1:compare IPv4 source IP; 
                                                2: compare IPv4 destination IP; 
                                                3: either source or destination IP*/

#else
        uint32 lowerBound;
        uint32 upperBound;

        uint32 type     : 2;
        uint32 reserved : 30;
#endif
} rtl8316d_tblAsic_ipRangeTable_t;

typedef struct rtl8316d_tblAsic_ipRangeParam_s{
    uint32 lowerBound;
    uint32 upperBound;
    
    uint32 reserved : 30;
    uint32 type     : 2;    /*0: invalid; 1:compare IPv4 source IP; 
                                                2: compare IPv4 destination IP; 
                                                3: either source or destination IP*/
} rtl8316d_tblAsic_ipRangeParam_t;

typedef struct rtl8316d_tblAsic_srcPortMaskTable_s{
#ifndef _LITTLE_ENDIAN
        uint32 reserved    : 7;
        uint32 srcPortMask : 25;
#else
        uint32 srcPortMask : 25;
        uint32 reserved        : 7;
#endif
} rtl8316d_tblAsic_srcPortMaskTable_t;

typedef struct rtl8316d_tblAsic_srcPortMaskParam_s{
        uint32  srcPortMask : 25;
} rtl8316d_tblAsic_srcPortMaskParam_t;


typedef struct rtk_filter_aclGroup_s
{       
    uint16 entryNum;
    rtk_filter_aclEntry_t* aclEntry;
}rtk_filter_aclGroup_t;


#if 0
rtk_api_ret_t rtk_filter_igrAcl_init(void);
rtk_api_ret_t rtk_filter_igrAcl_reInit(void);
int32 rtl8316d_actTbl_getAsicEntry(uint32 index, rtl8316d_aclAct_entry_t * actEntry_p);
int32 rtl8316d_actTbl_setAsicEntry(uint32 index, rtl8316d_aclAct_entry_t * actEntry_p);
/*convert raw ASIC data to readable ACL rule entry*/
int32 rtl8316d_inAclRule_getAsicEntry(uint32 index, rtl8316d_aclEntry_t* pAclEntry);
/*convert raw ASIC data to readable ACL rule entry*/
int32 rtl8316d_inAclRule_setAsicEntry(uint32 index, rtl8316d_aclEntry_t* pAclEntry);
rtk_api_ret_t rtk_filter_templ_set(rtk_aclBlock_id_t blockId, rtk_filter_templField_id_t fieldId, rtk_filter_field_type_t fieldType);
rtk_api_ret_t rtk_filter_templ_get(rtk_aclBlock_id_t blockId, rtk_filter_templField_id_t fieldId, rtk_filter_field_type_t* pFieldType);
rtk_api_ret_t rtk_filter_aclEntry_alloc(rtk_filter_aclEntry_t** ppAclEntry);
rtk_api_ret_t rtk_filter_aclEntry_init(rtk_filter_aclEntry_t* pAclEntry);
rtk_api_ret_t rtk_filter_aclEntry_free(rtk_filter_aclEntry_t* pAclEntry);
rtk_api_ret_t rtk_filter_aclField_add(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_field_type_t type, rtk_filter_pieTempl_field_t fieldData, rtk_filter_pieTempl_field_t fieldMask);
rtk_api_ret_t rtk_filter_aclField_del(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_field_type_t type);
rtk_api_ret_t rtk_filter_aclField_getByType(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_field_type_t type, rtk_filter_aclField_t* pField);
rtk_api_ret_t rtk_filter_aclField_getFirst(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_aclField_t* pField);

rtk_api_ret_t rtk_filter_aclField_getNext(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_field_type_t type, rtk_filter_aclField_t* pField);
rtk_api_ret_t rtk_filter_aclField_replace(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_aclField_t* pField);
rtk_api_ret_t rtk_filter_aclRule_append(rtk_filter_aclEntry_t* pAclEntry, rtk_aclBlock_id_t blockId);
rtk_api_ret_t rtk_filter_aclRule_insert(rtk_filter_aclEntry_t* pAclEntry, rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId);
rtk_api_ret_t rtk_filter_aclRule_remove(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_filter_aclEntry_t** ppAclEntry);
rtk_api_ret_t rtk_filter_aclRule_get(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_filter_aclEntry_t** ppAclEntry);
rtk_api_ret_t rtk_filter_aclRule_getFirst(rtk_aclBlock_id_t blockId, rtk_filter_aclEntry_t** ppAclEntry);
rtk_api_ret_t rtk_filter_aclAction_set(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_actionType_t actType, rtk_filter_aclAction_t* pAction);
rtk_api_ret_t rtk_filter_aclAction_get(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_actionType_t actType, rtk_filter_aclAction_t* pAction);
rtk_api_ret_t rtk_filter_aclAction_del(rtk_filter_aclEntry_t* pAclEntry, rtk_filter_actionType_t actType);
rtk_api_ret_t rtk_filter_reverseHit_set(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_enable_t reverse);
rtk_api_ret_t rtk_filter_reverseHit_get(rtk_aclBlock_id_t blockId, rtk_aclRule_id_t ruleId, rtk_enable_t* pReverse);
rtk_api_ret_t rtk_filter_igrAcl_apply(rtk_aclBlock_id_t blockId);
#endif


#endif /*__RTL8316D_ASICDRV_INACL_H__*/

