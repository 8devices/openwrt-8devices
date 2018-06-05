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
* Purpose :  Register ASIC driver related Macro, structure, enumeration, 
*                variable types declaration and definition.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/


#ifndef _RTL8316D_REGASICDRV_H_
#define _RTL8316D_REGASICDRV_H_

//#include "rtl_types.h"
//#include "rtl8316d_mib_dbg.h"
//#include "rtl8316d_mib.h"

#define rtl8316d_REGBITLENGTH               (32 - 1)
#define rtl8316d_REGDATAMAX                 0xFFFF
#define RTL8316D_UNIT                       1
#define RTL8316D_PORT_NUMBER  25

enum RTL8316D_PORTNUM
{
    PN_PORT0 = 0,
    PN_PORT1,
    PN_PORT2,
    PN_PORT3,
    PN_PORT4,
    /*5~9*/
    PN_PORT5,
    PN_PORT6,
    PN_PORT7,
    PN_PORT8,
    PN_PORT9,
    /*10~14*/
    PN_PORT10,
    PN_PORT11,
    PN_PORT12,
    PN_PORT13,
    PN_PORT14,
    /*15~19*/
    PN_PORT15,
    PN_PORT16,
    PN_PORT17,
    PN_PORT18,
    PN_PORT19,
    /*20~24*/
    PN_PORT20,
    PN_PORT21,
    PN_PORT22,
    PN_PORT23,
    PN_PORT24,
    PN_PORT_END = PN_PORT24,
    PN_DABLOCK = 29
};

/* Port number for 'dp' & 'dpext'*/
enum PORT_MASK
{
    PM_PORT_0 = (1<<PN_PORT0),
    PM_PORT_1 = (1<<PN_PORT1),
    PM_PORT_2 = (1<<PN_PORT2),
    PM_PORT_3 = (1<<PN_PORT3),
    PM_PORT_4 = (1<<PN_PORT4),
    PM_PORT_5 = (1<<PN_PORT5),
    PM_PORT_6 = (1<<PN_PORT6),
    PM_PORT_7 = (1<<PN_PORT7),
    PM_PORT_8 = (1<<PN_PORT8),
    PM_PORT_9 = (1<<PN_PORT9),
    PM_PORT_10 = (1<<PN_PORT10),
    PM_PORT_11 = (1<<PN_PORT11),
    PM_PORT_12 = (1<<PN_PORT12),
    PM_PORT_13 = (1<<PN_PORT13),
    PM_PORT_14 = (1<<PN_PORT14),
    PM_PORT_15 = (1<<PN_PORT15),
    PM_PORT_16 = (1<<PN_PORT16),
    PM_PORT_17 = (1<<PN_PORT17),
    PM_PORT_18 = (1<<PN_PORT18),
    PM_PORT_19 = (1<<PN_PORT19),
    PM_PORT_20 = (1<<PN_PORT20),
    PM_PORT_21 = (1<<PN_PORT21),
    PM_PORT_22 = (1<<PN_PORT22),
    PM_PORT_23 = (1<<PN_PORT23),
    PM_PORT_24 = (1<<PN_PORT24),

#if defined(CONFIG_RTL8311S) 

      PM_PORT_ALL = ( (1<<PN_PORT4) | (1<<PN_PORT8) | (1<<PN_PORT22) | (1<<PN_PORT23) | (1<<PN_PORT24))

#elif defined(CONFIG_RTL8316D_FPGA) 

      PM_PORT_ALL = ( (1<<PN_PORT0) | (1<<PN_PORT4) | (1<<PN_PORT8) | (1<<PN_PORT12) | (1<<PN_PORT22) | (1<<PN_PORT23))     
#else

    PM_PORT_ALL = (1<<PN_PORT0)|(1<<PN_PORT1)|(1<<PN_PORT2)|(1<<PN_PORT3)|(1<<PN_PORT4)|(1<<PN_PORT5)|
                          (1<<PN_PORT6)|(1<<PN_PORT7)|(1<<PN_PORT8)|(1<<PN_PORT9)|(1<<PN_PORT10)|(1<<PN_PORT11)|
                          (1<<PN_PORT12)|(1<<PN_PORT13)|(1<<PN_PORT14)|(1<<PN_PORT15)|(1<<PN_PORT16)|(1<<PN_PORT17)|
                          (1<<PN_PORT18)|(1<<PN_PORT19)|(1<<PN_PORT20)|(1<<PN_PORT21)|(1<<PN_PORT22)|(1<<PN_PORT23)|
                          (1<<PN_PORT24)
       
#endif
};



enum TXTAGSTAT_TYPE
{
    TXTAGSTAT_OUTERINNER = 0, 
    TXTAGSTAT_OUTER, 
    TXTAGSTAT_INNER,    
    TXTAGSTAT_UNTAG
};

enum RXPRICOPY_TYPE
{
    RXPRICOPY_ALE = 0,
    RXPRICOPY_INNER,    
    RXPRICOPY_OUTER 
};


enum INSCPUTAG_TYPE
{
    INSCPUTAG_NOT = 0, 
    INSCPUTAG_TRAP, 
    INSCPUTAG_ALL    
};

enum TOCPU_TxORGPKT_TYPE
{
    TOCPU_TxORGPKT_ALE = 0,
    TOCPU_TxORGPKT_TRAP,
    TOCPU_TxORGPKT_ALL         
};

enum TRAPCPUPORT_MASK
{
    CPU_NONE = 0,
    CPU_PORT23,
    CPU_PORT24,
    CPU_PORT23_24,
};

enum CFITRAP_TYPE
{
    CFI_FWD = 0,
    CFI_TRAP, 
    CFI_DROP,        
};

enum VIDRC_TYPE
{
    VIDRC_INVALID = 0,
    VIDRC_INNER,
    VIDRC_OUTER,
    VIDRC_INOUT,
};

enum IPRC_TYPE
{
    IPRC_INVALID = 0,
    IPRC_SIP,
    IPRC_DIP,
    IPRC_SIPDIP
};

enum L4PORTRC_TYPE
{
    L4PORTRC_INVALID = 0,
    L4PORTRC_TCP,
    L4PORTRC_UDP,
    L4PORTRC_TCPUDP
};

enum L4PORTRC_SRCDST
{
    L4PORTRC_SRC = 0,
    L4PORTRC_DST,        
};

enum Dot1XUNAUTHBH_TYPE
{
    Dot1XUNAUTHBH_DROP = 0,
    Dot1XUNAUTHBH_TRAP, 
    Dot1XUNAUTHBH_GVLAN
};
    

#if defined(CONFIG_RTL865X_CLE) || defined (RTK_X86_CLE)
extern uint32 cleDebuggingDisplay;
#endif

typedef struct rtl8316d_attkPrevCtrlParam_s
{
    uint32 reserved:17;
    uint32 synFin_deny:1;
    uint32 xmas_deny:1;
    uint32 nullScan_deny:1;
    uint32 synSportL1024_deny:1;
    uint32 tcpHdrMin_enable:1;
    uint32 smurf_deny:1;
    uint32 icmpv6PingMax_enable:1;
    uint32 icmpv4PingMax_enable:1;
    uint32 icmpFragPkt_deny:1;
    uint32 reserved1:1;
    uint32 pod_deny:1;
    uint32 tcpBlat_deny:1;
    uint32 udpBlat_deny:1;
    uint32 land_deny:1;
    uint32 daEqSa_deny:1;
} rtl8316d_attkPrevCtrlParam_t;

int32 rtl8316d_vlan_ignvid_set(uint32 portnum, uint32 ignivid, uint32 ignovid);
int32 rtl8316d_vlan_ignvid_get(uint32 portnum, uint32 * ignivid, uint32 * ignovid);
int32 rtl8316d_vlan_portBase_set(uint32 port_num, uint32 Ivid, uint32 Ipri, uint32 Ovid, uint32 Opri, uint32 Odei);
int32 rtl8316d_vlan_portBase_get(uint32 port_num, uint32 * Ivid, uint32 * Ipri, uint32 * Ovid, uint32 * Opri, uint32 * Odei);
int32 rtl8316d_vlan_acptFrameType_set(uint32 port_num, uint32 acptITag, uint32 acptIUtag, uint32 acptOTag, uint32 acptOUtag);
int32 rtl8316d_vlan_acptFrameType_get(uint32 port_num, uint32 * acptITag, uint32 * acptIUtag, uint32 * acptOTag, uint32 * acptOUtag);
int32 rtl8316d_vlan_ingressFilter_set(uint32 port_num, uint32 enable, uint32 action);
int32 rtl8316d_vlan_ingressFilter_get(uint32 port_num, uint32 * enable, uint32 * action);
int32 rtl8316d_vlan_egressFilter_set(uint32 port_num, uint32 enable);
int32 rtl8316d_vlan_egressFilter_get(uint32 port_num, uint32 * enable);
int32 rtl8316d_portPriParam_set(uint32 port_num, uint32 portPri, uint32 PortCPriDpTblIdx, uint32 PortSPriDpTblIdx, uint32 PortDscpPriDpTblIdx, uint32 PortPriWgtTblIdx);
int32 rtl8316d_portPriParam_get(uint32 port_num, uint32 * portPri, uint32 * PortCPriDpTblIdx, uint32 * PortSPriDpTblIdx, uint32 * PortDscpPriDpTblIdx, uint32 * PortPriWgtTblIdx);
int32 rtl8316d_PriorityArbitorTbl_set(uint32 table_index, uint32 portBaseWeight, uint32 dot1QBaseWeight, uint32 dscpBaseWeight, uint32 CTagBaseWeight, uint32 STagBaseWeight);
int32 rtl8316d_PriorityArbitorTbl_get(uint32 port_num, uint32 * portBaseWeight, uint32 * dot1QBaseWeight, uint32 * dscpBaseWeight, uint32 * CTagBaseWeight, uint32 * STagBaseWeight);
int32 rtl8316d_innerTagPriMapTbl_set(uint32 table_index, uint32 orig_pri, uint32 map_pri);
int32 rtl8316d_innerTagPriMapTbl_get(uint32 port_num, uint32 orig_pri, uint32 * map_pri);
int32 rtl8316d_outerTagPriMapTbl_set(uint32 table_index, uint32 orig_pri, uint32 orig_dei, uint32 map_pri);
int32 rtl8316d_outerTagPriMapTbl_get(uint32 port_num, uint32 orig_pri, uint32 orig_dei, uint32 * map_pri);
int32 rtl8316d_dscpMapTbl_set(uint32 table_index, uint32 dscp, uint32 map_pri);
int32 rtl8316d_dscpMapTbl_get(uint32 port_num, uint32 dscp, uint32 * map_pri);
int32 rtl8316d_fwdTagSelect_set(uint32 port_num, uint32 baseOnOvid);
int32 rtl8316d_fwdTagSelect_get(uint32 port_num, uint32 * baseOnOvid);

int32 rtl8316d_asic_reset(void);

int32 rtl8316d_portmove_legalPortMask(uint32 port, uint32 portMask);
int32 rtl8316d_portmove_illegalport_act(uint32 port, uint32 action);
int32 rtl8316d_portmove_legalport_act(uint32 port, uint32 action);

int32 rtl8316d_maclrnlimit_enable(uint32 port);
int32 rtl8316d_maclrnlimit_disable(uint32 port);
int32 rtl8316d_maclrnlimit_maxnum_set(uint32 port, uint32 maxNum);
int32 rtl8316d_maclrnlimit_maxnum_get(uint32 port, uint32* maxNum);
int32 rtl8316d_maclrnlimit_curnum_get(uint32 port, uint32* curNum);
int32 rtl8316d_maclrnlimit_action_set(uint32 port, uint32 action);
int32 rtl8316d_maclrnlimit_action_get(uint32 port, uint32* action);
int32 rtl8316d_maclrnlimit_cpuport_set(uint32 port);
int32 rtl8316d_maclrnlimit_cpuport_get(uint32* port);


int32 rtl8316d_srcport_selfFilter_set(uint32 port, uint32 enable);

int32 rtl8316d_ipmcst_configure(uint32 enable);

int32 rtl8316d_attacPrev_set(rtl8316d_attkPrevCtrlParam_t attkPrev_t);
int32 rtl8316d_attacPrev_get(rtl8316d_attkPrevCtrlParam_t* attkPrev);
int32 rtl8316d_maxPingPktLen_set(uint32 length);
int32 rtl8316d_maxPingPktLen_get(uint32* length);
int32 rtl8316d_minTcpHdrSize_set(uint32 length);
int32 rtl8316d_minTcpHdrSize_get(uint32* length);
int32 rtl8316d_smurfNetmask_set(uint32 length);
int32 rtl8316d_smurfNetmask_get(uint32* length);
int32 rtl8316d_rxDropReason_get(uint32 port, uint32 * reason);

int32 rtl8316d_acl_portctrl_get(uint32 port, uint32 *enable, uint32 * action);
int32 rtl8316d_acl_portctrl_set(uint32 port, uint32 enable, uint32 action);
int32 rtl8316d_acl_crcerr_get( uint32 *crcerr, uint32 * cfilk);
int32 rtl8316d_acl_crcerr_set(uint32 crcerr, uint32 cfilk);
int32 rtl8316d_policer_tblctrl_get( uint32 *include);
int32 rtl8316d_policer_tblctrl_set(uint32 include);
int32 rtl8316d_acl_copytocpu_get( uint32 *portmask);
int32 rtl8316d_acl_copytocpu_set(uint32 portmask);   
int32 rtl8316d_acl_templ_get( uint32 tps, uint32 fieldidx, uint32* fieldno);
int32 rtl8316d_acl_templ_set( uint32 tps, uint32 fieldidx, uint32 fieldno);

int32 rtl8316d_mib_debug_get(enum RTL8316D_MIB_DEBUG mibCnt, uint32* counterH, uint32* counterL);
int32 rtl8316d_mib_get(enum RTL8316D_MIB mibCnt, uint32* counterH, uint32* counterL);

#endif /*_RTL8316D_REGASICDRV_H_*/
