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
* Purpose :  NIC ASIC driver related Macro, structure, enumeration, variable types declaration and definition.
*
*  Feature :  This file consists of following modules:
*             1)
*
*/


#ifndef __RTL8316D_NIC_DRV_H__
#define __RTL8316D_NIC_DRV_H__

#define RXSTOP_NIC  0x1FF //it's 8 byte alignment. So the actual size is the number * 8
#define TXSTOP_NIC  0x3FF

/* Sometimes, cpu will append ITAG|OTAG|CPU in reply packet,
 * This offset is the length of unused buffer space before received packet buffer.
 * So that when the cpu appends ITAG|OTAG|CPU, the contents in the packet buffer
 * don't need to move afterwards.
 */
#define CPU_TAG_LEN             8
#define OTAG_LEN                4
#define ITAG_LEN                4
#define MEMCPY_RESERVE          1  // because Rx & Tx share the same buffer, to avoid copy same memory, 1 byte is reserved.
#define HEADER_EXTENSION_SIZE   (ITAG_LEN + OTAG_LEN + CPU_TAG_LEN + MEMCPY_RESERVE)
#define MAX_PKT_PAYLOAD_LEN     1514

#define MAX_PKT_LEN             (MAX_PKT_PAYLOAD_LEN + FCS_LEN + OTAG_LEN + ITAG_LEN + CPU_TAG_LEN)
#define MIN_PKT_LEN             (60 + CPU_TAG_LEN)

#define PKT_BUFFER_SIZE         (MAX_PKT_PAYLOAD_LEN + FCS_LEN + HEADER_EXTENSION_SIZE)


#define NIC_RXSTAT_RRCPCSER_OFFSET      26
#define NIC_RXSTAT_RRCPCSER_MASK        (0x1UL << NIC_RXSTAT_RRCPCSER_OFFSET)
#define NIC_RXSTAT_PRI_OFFSET           24
#define NIC_RXSTAT_PRI_MASK             (0x3UL << NIC_RXSTAT_PRI_OFFSET)
#define NIC_RXSTAT_CPUTAG_OFFSET        23
#define NIC_RXSTAT_CPUTAG_MASK          (0x1UL << NIC_RXSTAT_CPUTAG_OFFSET)
#define NIC_RXSTAT_FRAG_OFFSET          22
#define NIC_RXSTAT_FRAG_MASK            (0x1UL << NIC_RXSTAT_FRAG_OFFSET)
#define NIC_RXSTAT_PPPOE_OFFSET         22
#define NIC_RXSTAT_PPPOE_MASK           (0x1UL << NIC_RXSTAT_PPPOE_OFFSET)
#define NIC_RXSTAT_L34FMT_OFFSET        16
#define NIC_RXSTAT_L34FMT_MASK          (0xFUL << NIC_RXSTAT_L34FMT_OFFSET)
#define NIC_RXSTAT_ITAG_OFFSET          15
#define NIC_RXSTAT_ITAG_MASK            (0x1UL << NIC_RXSTAT_ITAG_OFFSET)
#define NIC_RXSTAT_OTAG_OFFSET          14
#define NIC_RXSTAT_OTAG_MASK            (0x1UL << NIC_RXSTAT_OTAG_OFFSET)
#define NIC_RXSTAT_L2FMT_OFFSET         12
#define NIC_RXSTAT_L2FMT_MASK           (0x3UL << NIC_RXSTAT_L2FMT_OFFSET)
#define NIC_RXSTAT_L4CSER_OFFSET        11
#define NIC_RXSTAT_L4CSER_MASK          (0x1UL << NIC_RXSTAT_L4CSER_OFFSET)
#define NIC_RXSTAT_L3CSER_OFFSET        10
#define NIC_RXSTAT_L3CSER_MASK          (0x1UL << NIC_RXSTAT_L3CSER_OFFSET)
#define NIC_RXSTAT_CRCER_OFFSET         9
#define NIC_RXSTAT_CRCER_MASK           (0x1UL << NIC_RXSTAT_CRCER_OFFSET)
#define NIC_RXSTAT_OVERSIZE_OFFSET      8
#define NIC_RXSTAT_OVERSIZE_MASK        (0x1UL << NIC_RXSTAT_OVERSIZE_OFFSET)
#define NIC_RXSTAT_PKTSN_OFFSET         0
#define NIC_RXSTAT_PKTSN_MASK           (0xFF << NIC_RXSTAT_PKTSN_OFFSET)


#define NIC_TXSTAT_RRCPSRE_OFFSET        11
#define NIC_TXSTAT_RRCPSRE_MASK          (0x1 << NIC_TXSTAT_RRCPSRE_OFFSET)
#define NIC_TXSTAT_L4CSRE_OFFSET        10
#define NIC_TXSTAT_L4CSRE_MASK          (0x1 << NIC_TXSTAT_L4CSRE_OFFSET)
#define NIC_TXSTAT_L3CSRE_OFFSET        9
#define NIC_TXSTAT_L3CSRE_MASK          (0x1 << NIC_TXSTAT_L3CSRE_OFFSET)
#define NIC_TXSTAT_CRCRE_OFFSET         8
#define NIC_TXSTAT_CRCRE_MASK           (0x1 << NIC_TXSTAT_CRCRE_OFFSET)
#define NIC_TXSTAT_PKTSN_OFFSET         0
#define NIC_TXSTAT_PKTSN_MASK           (0xFF << NIC_TXSTAT_PKTSN_OFFSET)

#define L34FMT_UNKNOWN      0
#define L34FMT_ARP          1
#define L34FMT_ICMP         2
#define L34FMT_IGMP         3
#define L34FMT_TCP          4
#define L34FMT_UDP          5
#define L34FMT_IPV4         6
#define L34FMT_ICMPV6       7
#define L34FMT_TCPV6        8
#define L34FMT_UDPV6        9
#define L34FMT_IPV6         10

/* NIC initilization symbolic constant */
#define SYS_IP0             192
#define SYS_IP1             168
#define SYS_IP2             0
#define SYS_IP3             1

#define SYS_GATEWAY0        192
#define SYS_GATEWAY1        168
#define SYS_GATEWAY2        0
#define SYS_GATEWAY3        254
#define DEFAULT_GATEWAY     SYS_GATEWAY0, SYS_GATEWAY1, SYS_GATEWAY2, SYS_GATEWAY3

#define SYS_NETMASK0        255
#define SYS_NETMASK1        255
#define SYS_NETMASK2        255
#define SYS_NETMASK3        0

/*Memory Controller Reg*/
#define CPU_REGBASE_ADDR     0x7F00
#define CPU_CRSTR_ADDR       (CPU_REGBASE_ADDR+0)
#define CPU_CMDTR_ADDR       (CPU_REGBASE_ADDR+4)
#define CPU_DMAR_ADDR        (CPU_REGBASE_ADDR+8)
#define CPU_SMAR_ADDR        (CPU_REGBASE_ADDR+0xC)
#define CPU_DLR_ADDR         (CPU_REGBASE_ADDR+0x10)
#define CPU_FPNUMR_ADDR      (CPU_REGBASE_ADDR+0x14)
#define CPU_8051FRQ_ADDR     (CPU_REGBASE_ADDR+0x18)
#define CPU_IMCR_ADDR        (CPU_REGBASE_ADDR+0x1C)
#define CPU_IMDR0_ADDR       (CPU_REGBASE_ADDR+0x20)
#define CPU_IMDR1_ADDR       (CPU_REGBASE_ADDR+0x24)
#define CPU_SRAAR_ADDR       (CPU_REGBASE_ADDR+0x28)
#define CPU_SRADR_ADDR       (CPU_REGBASE_ADDR+0x2C)
#define CPU_SRACR_ADDR       (CPU_REGBASE_ADDR+0x30)
#define CPU_ECS2R_ADDR       (CPU_REGBASE_ADDR+0x40)

/*NIC Reg*/
#define NIC_REGBASE_ADDR     0x7F50
#define NIC_GCR_ADDR         (NIC_REGBASE_ADDR + 0)
#define NIC_TXSTOPR_ADDR     (NIC_REGBASE_ADDR + 4)
#define NIC_RXSTOPR_ADDR     (NIC_REGBASE_ADDR + 8)
#define NIC_RXRDLR_ADDR      (NIC_REGBASE_ADDR + 0xC)
#define NIC_TXAFSR_ADDR      (NIC_REGBASE_ADDR + 0x10)
#define NIC_RXCMDR_ADDR      (NIC_REGBASE_ADDR + 0x14)
#define NIC_TXCMDR_ADDR      (NIC_REGBASE_ADDR + 0x18)
#define NIC_ISR_ADDR         (NIC_REGBASE_ADDR + 0x1C)
#define NIC_IMR_ADDR         (NIC_REGBASE_ADDR + 0x20)
#define NIC_RXCTLR_ADDR      (NIC_REGBASE_ADDR + 0x24)
#define NIC_TXCTLR_ADDR      (NIC_REGBASE_ADDR + 0x28)
#define NIC_CRXCPR_ADDR      (NIC_REGBASE_ADDR + 0x2C)
#define NIC_CTXCPR_ADDR      (NIC_REGBASE_ADDR + 0x30)
#define NIC_MHR0_ADDR        (NIC_REGBASE_ADDR + 0x34)
#define NIC_MHR1_ADDR        (NIC_REGBASE_ADDR + 0x38)
#define NIC_MHR2_ADDR        (NIC_REGBASE_ADDR + 0x3C)
#define NIC_MHR3_ADDR        (NIC_REGBASE_ADDR + 0x40)
#define NIC_MHR4_ADDR        (NIC_REGBASE_ADDR + 0x44)
#define NIC_MHR5_ADDR        (NIC_REGBASE_ADDR + 0x48)
#define NIC_MHR6_ADDR        (NIC_REGBASE_ADDR + 0x4C)
#define NIC_MHR7_ADDR        (NIC_REGBASE_ADDR + 0x50)
#define NIC_PAHR0_ADDR       (NIC_REGBASE_ADDR + 0x54)
#define NIC_PAHR1_ADDR       (NIC_REGBASE_ADDR + 0x58)
#define NIC_PAHR2_ADDR       (NIC_REGBASE_ADDR + 0x5C)
#define NIC_PAHR3_ADDR       (NIC_REGBASE_ADDR + 0x60)
#define NIC_PAHR4_ADDR       (NIC_REGBASE_ADDR + 0x64)
#define NIC_PAHR5_ADDR       (NIC_REGBASE_ADDR + 0x68)
#define NIC_PAHR6_ADDR       (NIC_REGBASE_ADDR + 0x6C)
#define NIC_PAHR7_ADDR       (NIC_REGBASE_ADDR + 0x70)
#define NIC_SRXCURPKTR_ADDR  (NIC_REGBASE_ADDR + 0x74)
#define NIC_STXCURPKTR_ADDR  (NIC_REGBASE_ADDR + 0x78)
#define NIC_STXPktLENR_ADDR  (NIC_REGBASE_ADDR + 0x7C)
#define NIC_STXCURUNITR_ADDR  (NIC_REGBASE_ADDR + 0x80)

#define CPU_CMDTR_CMDSTATUS_OFFSET      6
#define CPU_CMDTR_CMDSTATUS_MASK        (1 << CPU_CMDTR_CMDSTATUS_OFFSET)
#define CPU_CMDTR_CMPRESULT_OFFSET      5
#define CPU_CMDTR_CMPRESULT_MASK        (1 << CPU_CMDTR_CMPRESULT_OFFSET)
#define CPU_CMDTR_FMODE_OFFSET      4
#define CPU_CMDTR_FMODE_MASK        (1 << CPU_CMDTR_FMODE_OFFSET)
#define CPU_CMDTR_CMDTYPE_OFFSET      0
#define CPU_CMDTR_CMDTYPE_MASK        (0xF << CPU_CMDTR_FMODE_OFFSET)

#define CPU_SRACR_TRIGGER_OFFSET    1
#define CPU_SRACR_TRIGGER_MASK        (1 << CPU_SRACR_TRIGGER_OFFSET)
#define CPU_SRACR_TYPE_OFFSET    1
#define CPU_SRACR_TYPE_MASK        (1 << CPU_SRACR_TYPE_OFFSET)
#define CPU_SRACR_TYPE_READ     0
#define CPU_SRACR_TYPE_WRITE    1

#define PORT_ALL_MASK   0x1FFFFFF


/* Rx Status Word for NIC */
typedef struct rx_stat_s
{
    uint8   rrcpcse;    /* 1: rrcp checksum error; 0: no error*/
    uint8   rxPri;      /* Rx packet priority */
    uint8   cputag;     /* If the cpu tag exist or not*/
    uint8   fragment;   /* The packet has fragment or not. 0:  has no fragment; 1: has fragment */
    uint8   pppoe;      /* Packet has PPPoE header. 0: is; 1: not */
    uint8   l34fmt;     /* layer 3/4 format */
    uint8   iTagExist;
    uint8   oTagExist;
    uint8   l2fmt;      /* layer 2  format */
    uint8   L4cse;      /* ICMP/IGMP/TCP/UDP checksum error . 0: correct ; 1: error*/
    uint8   L3cse;      /* IP Checksum Error. 0: correct ; 1: error*/
    uint8   crce;       /* CRC Error.   0: correct ; 1: error*/
    uint8   oversize;
    uint8   rxPktSn;    /* Rx Packet Sequence Number */
    uint16  length;
}rx_stat_t;

/* Tx Control Word for NIC*/

/* Rx CPU tag */
typedef struct rx_cpuTag_s
{
    uint16 etherType;
    uint8 protocol;
    uint8 pri;
    uint8 sphy;
    uint16 vid;
    uint8 pppoe;
    uint8 l34fmt;
    uint8 iTagExist;
    uint8 oTagExist;
    uint8 l2fmt;
    uint8 aclOTagOp;
    uint8 aclITagOp;
    uint8 rsnValid;
    uint8 reason;
}rx_cpuTag_t;

/* Tx CPU tag */
typedef struct tx_cpuTag_s
{
    uint16  etherType;
    uint8   protocol;
    uint8   dslrn;    /*Disable SMAC learning*/
    uint8   pri;      /*assigned priority*/
    uint8   normk;      /*When NORMK is 0, the packet will be remarked based on assigned priority、Tx port priority copy、swap and remarking setting. Otherwise��priority、DEI and DSCP would not be remarked by ALE*/
    uint8   aspri;    /*If ASPRI is 1, MCU8051 provides the priority (maybe include DEI and DSCP if exist) directly. If ASPRI is 0, the priority、DEI and DSCP is decided by ASIC lookup process.*/
    uint32  dmp;        /*Destination Port Mask*/
}tx_cpuTag_t;

/* ITag & OTag Information */
typedef struct vlan_tag_s
{
    uint16 etherType;
    uint16 tag;
}vlan_tag_t;

/* Packet information needed when receiving packets */
typedef struct rx_pktInfo_s
{
    rx_stat_t   rx_stat;
    uint8       da[6];
    uint8       sa[6];
    rx_cpuTag_t cpu_tag;
    vlan_tag_t  otag;
    vlan_tag_t  itag;
    uint8*        data_buf;       // packet buffer
    uint16      buf_len;        // length when used for protocol stack
    //uint16      proto_start;    // offset used when parsering the packet
}rx_pktInfo_t;

typedef struct pktBuf_s {
    uint16 size;
    uint8 *frame_info;
    uint8 *pkt_cputag;
    uint8 *pkt_vlantag;
    uint8 *pkt_head;
    uint8 *l2;
    uint8 *l3;
    uint8 *l4;
    uint8 l2fmt;          /* Layer 2 Format */
    uint8 l3fmt;          /* Layer 3 Format */
    uint8 l4fmt;          /* Layer 4 Format */

    uint8  spa;             /* Rx port */

    uint8  tx_chksum_mode;  /* Check sum config for NIC tx function */
    uint8  tx_dport_mode;   /* Tx packet mode. please reference to tx_pkt_mode_t */
    uint32 tx_dst_pmsk;
    uint8  inner_tag_mode;  /* InnerTagMode of TX pkt */
    uint8  outer_tag_mode;  /* InnerTagMode of TX pkt */
    uint32 inner_tag;       /* Inner tag of rx pkt / Inner tag of tx pkt */
    uint32 outer_tag;       /* Outer tag of rx pkt / Outer tag of tx pkt */
} pktBuf_t;

extern uint32 getReg32(uint16 address);
extern void setReg32(uint16 address, uint32 value);

extern uint16 getReg16(uint16 address);
extern void setReg16(uint16 address, uint16 value);

extern uint8 getReg8(uint16 address);
extern void setReg8(uint16 address, uint8 value);

extern uint32 rtl8316d_getSWReg(uint32 address);
extern void rtl8316d_setSWReg(uint32 address, uint32 value);

extern uint8 rtl8316d_getSWRegBit(uint32 address, uint8 pos);
extern void rtl8316d_setSWRegBit(uint32 address, uint8 pos, uint8 val);

extern uint8 xdata ppktBuf[];
extern uint8 xdata rxSeq;
extern uint8 xdata txSeq;
extern rx_pktInfo_t xdata rxPktInfo;


#endif
