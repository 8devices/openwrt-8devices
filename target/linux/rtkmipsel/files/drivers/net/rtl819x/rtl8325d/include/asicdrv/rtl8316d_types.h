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
* Purpose : Definitio of those common types/macros/structures/enums for RTL8316D which
*               are not specified to registers or tables. If you need a new types/macros/structures/
*               nums for RTL8316D ASIC driver and do not know which file should hold it, just
*               put it here.
*
*  Feature : 
*
*/

#ifndef __RTL8316D_TYPES_H__
#define __RTL8316D_TYPES_H__

#include <rtk_api.h>

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILED
#define FAILED -1
#endif

#define RTL8316D_UNIT   0   /*to discuss this value*/

#define RTL8316D_MAX_PHY_PORT  24  /* number of physical ports */
#define RTL8316D_MAX_TRK_GRP   4     /* number of trunk group */
#define RTL8316D_MAX_CPU_PORT  1   /* number of cpu ports */

#define RTL8316D_MAX_PHY_N_CPU_PORT (RTL8316D_MAX_PHY_PORT + RTL8316D_MAX_CPU_PORT)
#define RTL8316D_MAX_LOGIC_PORT (RTL8316D_MAX_PHY_N_CPU_PORT + RTL8316D_MAX_TRK_GRP)
#define RTL8316D_MAX_PORT           RTL8316D_MAX_PHY_N_CPU_PORT
#define RTL8316D_MAX_PORTMASK       ((1 << RTL8316D_MAX_PORT) - 1)

#define RTL8316D_LUTWAYMAX          4       /*4-way hash*/
#define RTL8316D_LUT_AGEUNITMAX     65535

#define RTL8316D_PORTMIB_OFFSET     0x3C

#define RTL8316D_PORT_SKIP_START     16   // for different bonding: port 8 is the first port not used
#define RTL8316D_PORT_SKIP_END       24  // for different bonding: port 14 is the last port not used

#define MAC_ADDR_LEN        6
#define VLAN_Len         4
#define ETHER_TYPE_LEN      2
#define Eth_Hdr_Len         14
#define IP_HDR_LEN          20
#define TCP_HDR_LEN         20
#define UDP_HDR_LEN         8
#define IP_SIZE         4
#define FCS_LEN             4

#ifndef XBYTE
#define XBYTE ((uint8 volatile xdata *)0)
#endif

/* VLAN */
#define RTL8316D_MAX_STATIC_VLAN    32


/* 
   Offset in the reply packet
*/
#define DEST_MAC_START      0
#define SRC_MAC_START       (DEST_MAC_START   + MAC_ADDR_LEN)
#define ETHER_TYPE_START    (SRC_MAC_START    + MAC_ADDR_LEN)
#define ARP_HEAD_START      (ETHER_TYPE_START + ETHER_TYPE_LEN)
#define IP_HEAD_START       (ETHER_TYPE_START + ETHER_TYPE_LEN)
#define TCP_HEAD_START      (IP_HEAD_START    + IP_HDR_LEN)
#define UDP_HEAD_START      (IP_HEAD_START    + IP_HDR_LEN)
#define ICMP_HEAD_START     (IP_HEAD_START    + IP_HDR_LEN)
#define TCP_OPTION_START    (TCP_HEAD_START   + TCP_HDR_LEN)
#define HTTP_HEAD_START     (TCP_HEAD_START   + TCP_HDR_LEN)


/*Tx control information contains protocol generated information
 * which is needed when transmit packet*/
typedef struct tx_cntrl_s
{
    /*
     *  Protocol Stack doesn't calculate any layer2,3,4 crc. 
     *  We presume that hardware provides such functions so 
     *  that a lot of software time slots can be saved.
     */
    uint8  recL4cs;     /* Layer 4 Checksum Recalculation Enable */
    uint8  recL3cs;     /* Layer 3 Checksum Recalculation Enable */
    uint8  recCrc;      /* CRC Recalculation Enable */
    uint8  rrcpCrc;     /* RRCP crc Recalculation Enable*/
    
    /* RRCPv3 used field */
    uint16 pktLen;      /* The length of packet */

    uint8  txPortMode;  /* Method of assign TX port */
    uint32 txDsp;       /* TX Destination Port Mask */    

    uint32 txTagMode;   /* Tag mode of tx packet. Reference to above enum. */
    uint8  txPri;       /* Tx Packet Priority and cpri in ctag */
    uint16 txVid;       /* Tx vid */

} tx_cntrl_t;

#if 0
#define ETHER_ADDR_LEN  6
typedef struct ether_addr_s {
    uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;
#endif


#define swapl32(x)\
        ((((x) & 0xff000000U) >> 24) | \
         (((x) & 0x00ff0000U) >>  8) | \
         (((x) & 0x0000ff00U) <<  8) | \
         (((x) & 0x000000ffU) << 24))
#define swaps16(x)        \
        ((((x) & 0xff00) >> 8) | \
         (((x) & 0x00ff) << 8))  


#ifdef _LITTLE_ENDIAN
    #define ntohs(x)   (swaps16(x))
    #define ntohl(x)   (swapl32(x))
    #define htons(x)   (swaps16(x))
    #define htonl(x)   (swapl32(x))
#else
    #define ntohs(x)     (x)
    #define ntohl(x)      (x)
    #define htons(x)     (x)
    #define htonl(x)      (x)
#endif


#ifdef _LITTLE_ENDIAN
    #define MEM16(x)    (x)
#else
    #define MEM16(x)     (swaps16(x)) 
#endif

#define NIPQUAD(addr) \
    ((unsigned char *)&addr)[0], \
    ((unsigned char *)&addr)[1], \
    ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[3]
#define NIPQUAD_FMT "%u.%u.%u.%u"

#define NIP6(addr) \
    ntohs((addr).s6_addr16[0]), \
    ntohs((addr).s6_addr16[1]), \
    ntohs((addr).s6_addr16[2]), \
    ntohs((addr).s6_addr16[3]), \
    ntohs((addr).s6_addr16[4]), \
    ntohs((addr).s6_addr16[5]), \
    ntohs((addr).s6_addr16[6]), \
    ntohs((addr).s6_addr16[7])
#define NIP6_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"
#define NIP6_SEQFMT "%04x%04x%04x%04x%04x%04x%04x%04x"

#define IS_DABLOCK(lutEntry) ((lutEntry->port == 29) ? 1:0)

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

/* TRUNK */
#define RTK_TRUNK_MAX_PORT_MASK          0x1FFFFFF

/* Qos*/
#define RTL8316D_QOS_QUEUE_NUM              4

#define RTL8316D_QOS_MAX_PRI_NUM            8
#define RTL8316D_QOS_PRI2QID_WIDTH          2
#define RTL8316D_QOS_PRI2QID_MASK           ((1 << RTL8316D_QOS_PRI2QID_WIDTH) - 1)
#define RTL8316D_QOS_PRI2QID_OFF(pri)       ((pri) * RTL8316D_QOS_PRI2QID_WIDTH)      

#define RTL8316D_QOS_SCHED_TYPE_WIDTH       1
#define RTL8316D_QOS_SCHED_TYPE_MASK        ((1 << RTL8316D_QOS_SCHED_TYPE_WIDTH) - 1)
#define RTL8316D_QOS_SCHED_TYPE_OFF         6

/* VLAN */
#define RTL8316D_VIDMAX 0XFFF
#define RTL8316D_PRIMAX   7
#define RTL8316D_FIDMAX  0x1F
#define RTL8316D_VLANCAMMAX 16

/* Jumbo */
#define RTL8316D_JUMBO_MAX_SIZE   0x3fff

/* Qos */
typedef struct rtl8316d_primap_param_s
{
    uint8 pri[RTL8316D_QOS_MAX_PRI_NUM];   
}rtl8316d_primap_param_t;

typedef struct rtl8316d_priextra_param_s
{
    uint8 portpriority;
    uint8 ipriidx;
    uint8 opriidx;
    uint8 dscpidx;
    uint8 priwgtidx;
}rtl8316d_priextra_param_t;

typedef enum rtl8316d_qos_schedType_e
{
    SCHED_WFQ = 0,
    SCHED_WRR,
    SCHED_STRICT,
}rtl8316d_qos_schedType_t;
#if 0
typedef enum rtl8316d_vlan_tagMode_e
{
   RTL8316D_VLAN_TAG_ORIGINAL = 0,
   RTL8316D_VLAN_TAG_KEEP,
   RTL8316D_VLAN_TAG_END
} rtl8316d_vlan_tagMode_s;
#endif

/* LUT */
typedef struct rtl8316d_l2_flushItem_s
{
    uint16 fid;
    uint32 port;
    rtk_mac_t mac;    
}rtl8316d_l2_flushItem_t;

typedef enum rtl8316d_l2_flushType_e
{
    RTL8316D_FLUSH_BY_PORT = 1,       /* physical port       */
    RTL8316D_FLUSH_BY_FID = 2,          /*filtering database ID*/
    RTL8316D_FLUSH_BY_MAC = 4,       
    RTL8316D_FLUSH_END = RTL8316D_FLUSH_BY_PORT | RTL8316D_FLUSH_BY_FID | RTL8316D_FLUSH_BY_MAC
} rtl8316d_l2_flushType_t;

/* Storm Filter*/
typedef struct rtl8316d_storm_param_s
{   
    uint32 unknown_uni_inc;
    uint32 unknown_mul_inc;
    uint32 known_multi_inc;
    uint32 broad_inc;
    uint32 mode; // 0: packet; 1 : Kbps
    uint32 rate;
}rtl8316d_storm_param_t;

typedef enum rtl8316d_storm_type_inc_e
{   
    STORM_EXCLUDE,
    STORM_INCLUDE,
}rtl8316d_storm_type_inc_t;

/* PHY Access */
typedef enum rtl8316d_phy_op_e{
    RTL8316D_PHY_READ  = 3,
    RTL8316D_PHY_WRITE = 1,    
}rtl8316d_phy_op_t;

/* Port Status */
typedef struct rtl8316d_portStat_info_s {
    uint8   Link;
    uint8   AutoNego;
    rtk_port_speed_t spd;
    rtk_port_duplex_t duplex; 
    uint8   FC;          
}rtl8316d_portStat_info_t;







#endif /* __RTL8316D_TYPES_H__ */
