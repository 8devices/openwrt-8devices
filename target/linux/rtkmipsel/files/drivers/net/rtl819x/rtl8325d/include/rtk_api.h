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
* Purpose :This header defines macro, structure, enumeration and variable types of RTK API. 
*              This file would be released to customer.
*
*  Feature : 
*
*/


#ifndef __RTK_API_H__
#define __RTK_API_H__

#include <rtk_types.h>
#include <rtk_error.h>

#define ENABLE                                    1
#define DISABLE                                    0

#define PHY_CONTROL_REG                         0
#define PHY_STATUS_REG                          1
#define PHY_AN_ADVERTISEMENT_REG                4
#define PHY_AN_LINKPARTNER_REG                  5
#define PHY_1000_BASET_CONTROL_REG              9
#define PHY_1000_BASET_STATUS_REG              10
#define PHY_RESOLVED_REG                       17

/*Qos related configuration define*/
#define QOS_DEFAULT_TICK_PERIOD                   (19-1)
#define QOS_DEFAULT_BYTE_PER_TOKEN            34
#define QOS_DEFAULT_LK_THRESHOLD                (34*3) /* Why use 0x400? */


#define QOS_DEFAULT_INGRESS_BANDWIDTH          0x3FFF /* 0x3FFF => unlimit */
#define QOS_DEFAULT_EGRESS_BANDWIDTH          0x3D08 /*( 0x3D08 + 1) * 64Kbps => 1Gbps*/
#define QOS_DEFAULT_PREIFP                          1
#define QOS_DEFAULT_PACKET_USED_PAGES_FC    0x60
#define QOS_DEFAULT_PACKET_USED_FC_EN        0
#define QOS_DEFAULT_QUEUE_BASED_FC_EN        1

#define QOS_DEFAULT_PRIORITY_SELECT_PORT        8
#define QOS_DEFAULT_PRIORITY_SELECT_1Q        0
#define QOS_DEFAULT_PRIORITY_SELECT_ACL        0
#define QOS_DEFAULT_PRIORITY_SELECT_DSCP        0

#define QOS_DEFAULT_DSCP_MAPPING_PRIORITY    0

#define QOS_DEFAULT_1Q_REMARKING_ABILITY    0
#define QOS_DEFAULT_DSCP_REMARKING_ABILITY    0
#define QOS_DEFAULT_QUEUE_GAP                20
#define QOS_DEFAULT_QUEUE_NO_MAX            6
#define QOS_DEFAULT_AVERAGE_PACKET_RATE        0x3FFF
#define QOS_DEFAULT_BURST_SIZE_IN_APR        0x3F
#define QOS_DEFAULT_PEAK_PACKET_RATE            2
#define QOS_DEFAULT_SCHEDULER_ABILITY_APR    1     /*disable*/
#define QOS_DEFAULT_SCHEDULER_ABILITY_PPR    1    /*disable*/
#define QOS_DEFAULT_SCHEDULER_ABILITY_WFQ    1    /*disable*/

#define QOS_WEIGHT_MAX                        64



#define LED_GROUP_MAX                            3

#define ACL_DEFAULT_ABILITY                    0
#define ACL_DEFAULT_UNMATCH_PERMIT            1

#define ACL_RULE_FREE                            0
#define ACL_RULE_INAVAILABLE                    1

#define RTK_ACL_BLOCK_MAX                           4
#define RTK_ACL_BLOCKRULE_MAX                       32
#define RTK_ACL_RULE_MAX                            (RTK_ACL_BLOCK_MAX*RTK_ACL_BLOCKRULE_MAX)
#define RTK_ACL_TEMPLFIELD_MAX                      9

#define FILTER_POLICING_MAX                         8
#define FILTER_LOGGING_MAX                          8
#define FILTER_PATTERN_MAX                          4

#define STORM_UNUC_INDEX                            39
#define STORM_UNMC_INDEX                            47
#define STORM_MC_INDEX                              55
#define STORM_BC_INDEX                              63

#define RTK_MAX_NUM_OF_INTERRUPT_TYPE               4
#define RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST    1
#define RTK_MAX_NUM_OF_PRIORITY                     7
#define RTK_MAX_NUM_OF_QUEUE                        4
#define RTK_MAX_NUM_OF_TRUNK_HASH_VAL               1
#define RTK_MAX_NUM_OF_PORT                         25
#define RTK_MAX_NUM_OF_PROTO_TYPE                   0xFFFF
#define RTK_MAX_NUM_OF_MSTI                         32
#define RTK_MAX_NUM_OF_LEARN_LIMIT                  0x1FFF
#define RTK_MAX_PORT_MASK                           0x1FFFFFF

#define RTK_FLOWCTRL_PAUSE_ALL                      1980
#define RTK_FLOWCTRL_DROP_ALL                       2012
#define RTK_FLOWCTRL_PAUSE_SYSTEM_ON                1200
#define RTK_FLOWCTRL_PAUSE_SYSTEM_OFF               1000
#define RTK_FLOWCTRL_DROP_SYSTEM_ON                 1200
#define RTK_FLOWCTRL_DROP_SYSTEM_OFF                1000
#define RTK_FLOWCTRL_PAUSE_SHARE_ON                 216
#define RTK_FLOWCTRL_PAUSE_SHARE_OFF                208
#define RTK_FLOWCTRL_DROP_SHARE_ON                  216
#define RTK_FLOWCTRL_DROP_SHARE_OFF                 208
#define RTK_FLOWCTRL_PAUSE_PORT_ON                  140
#define RTK_FLOWCTRL_PAUSE_PORT_OFF                 132
#define RTK_FLOWCTRL_DROP_PORT_ON                   140
#define RTK_FLOWCTRL_DROP_PORT_OFF                  132
#define RTK_FLOWCTRL_PAUSE_PORT_PRIVATE_ON          26
#define RTK_FLOWCTRL_PAUSE_PORT_PRIVATE_OFF         22
#define RTK_FLOWCTRL_DROP_PORT_PRIVATE_ON           26
#define RTK_FLOWCTRL_DROP_PORT_PRIVATE_OFF          22
#define RTK_FLOWCTRL_PORT_DROP_EGRESS               210
#define RTK_FLOWCTRL_QUEUE_DROP_EGRESS              2047
#define RTK_FLOWCTRL_PORT_GAP                       72
#define RTK_FLOWCTRL_QUEUE_GAP                      18



typedef enum rtk_cpu_insert_e
{
    CPU_INSERT_TO_ALL = 0,
    CPU_INSERT_TO_TRAPPING,
    CPU_INSERT_TO_NONE,
    CPU_INSERT_END
}rtk_cpu_insert_t;

typedef enum rtk_cpu_position_e
{
    CPU_POS_ATTER_DA = 0,
    CPU_POS_AFTER_CRC,
    CPU_POS_END
}rtk_cpu_position_t;

/* Type of port-based dot1x auth/unauth*/
typedef enum rtk_dot1x_auth_status_e
{
    UNAUTH = 0,
    AUTH,
    AUTH_STATUS_END
} rtk_dot1x_auth_status_t;

typedef enum rtk_dot1x_direction_e
{
    BOTH = 0,
    IN,
    DIRECTION_END
} rtk_dot1x_direction_t;

typedef struct
{
    uint32 data[3];
} rtk_filter_dot1as_timestamp_t;

/* unauth pkt action */
typedef enum rtk_dot1x_unauth_action_e
{
    DOT1X_ACTION_DROP = 0,
    DOT1X_ACTION_TRAP2CPU,
    DOT1X_ACTION_GUESTVLAN,
    DOT1X_ACTION_END
} rtk_dot1x_unauth_action_t;

typedef uint32  rtk_dscp_t;         /* dscp vlaue */

typedef enum rtk_enable_e
{
    DISABLED = 0,
    ENABLED,
    RTK_ENABLE_END
} rtk_enable_t;

typedef uint32  rtk_fid_t;        /* filter id type */

/* ethernet address type */
typedef struct  rtk_mac_s
{
    uint8 octet[ETHER_ADDR_LEN];
} rtk_mac_t;


typedef struct
{
     /* CVLAN enable */
    uint32        enableFilterAssignIngressCvlanIdx:1;
    uint32        enableFilterAssignIngressCvlanVid:1;
    uint32        enableFilterAssignIngressCvlanPriority:1;
    uint32        enableFilterAssignEgressCvlanIdx:1;
    uint32        enableFilterAssignEgressCvlanVid:1;
    uint32        enableFilterAssignSvid2Cvid:1;
    uint32        enableFilterAssignRelayCvlanVid:1;
    uint32        enableFilterAddCvlanVid:1;
    uint32        enableFilterSubCvlanVid:1;    
        
    /* SVLAN */
    uint32        enableFilterAssignIngressSvlanIdx:1;        
    uint32        enableFilterAssignEgressSvlanIdx:1;
    uint32        enableFilterAssignIngressSvlanVid:1;        
    uint32        enableFilterAssignEgressSvlanVid:1;        
    uint32        enableFilterAssignCvid2Svid:1;               
    uint32        enableFilterAddSvlanVid:1;
    uint32        enableFilterSubSvlanVid:1;    
        
    /* Policing enable */
    uint32         enableFilterPolicing[FILTER_POLICING_MAX];
    uint32         enableFilterLogging[FILTER_LOGGING_MAX];    

    /* forwarding enable */
    uint32        enableFilterTrap2Cpu:1;
    uint32        enableFilterCopy2Cpu:1;        
    uint32        enableFilterRedirect:1;
    uint32        enableFilterDrop:1;
    uint32        enableFilterMirror:1;
    uint32        enableFilterAddDstPort:1;
        
    /* withdraw enable */
    uint32        enableFilterWithdrawRedirect:1;
    uint32        enableFilterWithdrawCvlanOp:1;
    uint32        enableFilterWithdrawSvlanOp:1;
    uint32        enableFilterWithdrawDSCPremarking:1;
    uint32        enableFilterWithdrawPriority:1;
    
     /* QOS enable */
     uint32        enableFilterPriority:1;
     uint32        enableFilterRelayCPriority:1;     
         
     /* Remarking enable */
     uint32        enableFilterTOSRemarking:1;
     uint32        enableFilterCpriRemarking:1;
     uint32        enableFilterSpriRemarking:1;

         
     /* CVLAN acton */
     uint32        filterIngressCvlanIdx;
     uint32        filterEgressCvlanIdx;
     uint32        filterIngressCvlanVid;
     uint32        filterEgressCvlanVid;
     uint32        filterRelayCvid;
         
     /* SVLAN action */
     uint32        filterIngressSvlanIdx;
     uint32        filterEgressSvlanIdx;
     uint32        filterIngressSvlanVid;
     uint32        filterEgressSvlanVid;     
     
     /* Policing action */
     uint32        filterPolicingIdx[FILTER_POLICING_MAX];
     uint32        filterLoggingIdx[FILTER_LOGGING_MAX];

     /* Forwarding action */
    uint32        filterRedirectPortmask;
    uint32        filterAddDstPortmask;  
    
    /* QOS action */
    uint32        filterPriority:3;
    uint32        filterRelayCPriority:1;         
        
    /* Remarking action */        
    uint32        filterCpriRemarking:3;
    uint32        filterSpriRemarking:3;    
    uint32        filterTOSRemarking:6;
    
} rtk_filter_action_t;

typedef struct
{
    uint32 ipv4:1;
    uint32 ipv6:1;
    uint32 ctag:1;
    uint32 stag:1;
    uint32 pppoe:1;
    uint32 icmp:1;
    uint32 igmp:1;
    uint32 tcp:1;
    uint32 udp:1;
} rtk_filter_care_type_t;

typedef struct rtk_filter_field rtk_filter_field_t;

typedef uint32  rtk_filter_ip_t;

typedef struct
{
    rtk_filter_field_t     *fieldHead;
    rtk_filter_care_type_t careType;

    uint32    invert;
} rtk_filter_cfg_t;

typedef struct
{
    uint32 dataType;
    union
    {
        rtk_mac_t data;
        rtk_mac_t rangeStart;
    };
    union
    {
        rtk_mac_t mask;
        rtk_mac_t rangeEnd;
    };
} rtk_filter_mac_t;

typedef struct rtk_filter_value_s
{
    uint32 dataType;
    union
    {
        uint32 data;
        uint32 rangeStart;
    };
    union
    {
        uint32 mask;
        uint32 rangeEnd;
    };
} rtk_filter_value_t;

typedef struct rtk_filter_tag_s
{
    rtk_filter_value_t pri;
    rtk_filter_value_t cfi;
    rtk_filter_value_t vid;
} rtk_filter_tag_t;

typedef struct
{
    uint32 mf:1;
    uint32 df:1;
} rtk_filter_ipFlag_t;

typedef struct
{
    uint32 addr[4];
} rtk_filter_ip6_addr_t;

typedef struct
{
    uint32 dataType;
    union
    {
        rtk_filter_ip6_addr_t data;
        rtk_filter_ip6_addr_t rangeStart;
    };
    union
    {
        rtk_filter_ip6_addr_t mask;
        rtk_filter_ip6_addr_t rangeEnd;
    };
} rtk_filter_ip6_t;

typedef struct
{
    uint32 data[FILTER_PATTERN_MAX];
    uint32 mask[FILTER_PATTERN_MAX];
} rtk_filter_pattern_t;

typedef struct
{
    uint32 urg:1;
    uint32 ack:1;
    uint32 psh:1;
    uint32 rst:1;
    uint32 syn:1;
    uint32 fin:1;
    uint32 ns:1;
    uint32 cwr:1;
    uint32 ece:1;    
} rtk_filter_tcpFlag_t;


struct rtk_filter_field
{
    uint32 fieldType;
    
    union
    {
        /* SPA */
        rtk_filter_value_t     port;
        
        /* L2 struct */
        rtk_filter_mac_t       dmac;
        rtk_filter_mac_t       smac;
        rtk_filter_value_t     etherType;
        rtk_filter_tag_t       ctag;
        rtk_filter_tag_t       relayCtag;
        rtk_filter_tag_t       stag;
        rtk_filter_dot1as_timestamp_t dot1asTimeStamp;
        
        /* L3 struct */
        rtk_filter_ip_t      sip;
        rtk_filter_ip_t      dip;
        rtk_filter_value_t   protocol;
        rtk_filter_value_t   ipTos;
        rtk_filter_ipFlag_t  ipFlag;
        rtk_filter_value_t   ipOffset;
        rtk_filter_ip6_t     sipv6;
        rtk_filter_ip6_t     dipv6;
        rtk_filter_value_t   ipv6TrafficClass;        
        rtk_filter_value_t   nextHeader;
        rtk_filter_value_t   flowLabel;
        
        /* L4 struct */
        rtk_filter_value_t   tcpSrcPort;
        rtk_filter_value_t   tcpDatcPort;
        rtk_filter_tcpFlag_t tcpFlag;
        rtk_filter_value_t   tcpSeqNumber;
        rtk_filter_value_t   tcpAckNumber;
        rtk_filter_value_t   udpSrcPort;
        rtk_filter_value_t   udpDatcPort;
        rtk_filter_value_t   icmpCode;
        rtk_filter_value_t   icmpType;
        rtk_filter_value_t   igmpType;

        /* pattern match */
        rtk_filter_pattern_t pattern;
    };

    struct rtk_filter_field *next;
};

#if 0
/*this definition has been redefiend*/
typedef enum rtk_filter_field_type_e
{
    FILTER_FIELD_PORT = 0,
    
    FILTER_FIELD_DMAC = 32,
    FILTER_FIELD_SMAC ,
    FILTER_FIELD_ETHERTYPE,
    FILTER_FIELD_CTAG,
    FILTER_FIELD_STAG,
    FILTER_FIELD_ETHERTYPE,
    FILTER_FIELD_DOT1AS_TIMESTAMP,
    
    FILTER_FIELD_IPV4_SIP = 64,
    FILTER_FIELD_IPV4_DIP,
    FILTER_FIELD_IPV4_PROTOCOL,
    FILTER_FIELD_IPV4_TOS,
    FILTER_FIELD_IPV4_OFFSET,
    FILTER_FIELD_IPV4_FLAG,
    FILTER_FIELD_IPV6_SIPV6,
    FILTER_FIELD_IPV6_DIPV6,
    FILTER_FIELD_IPV6_TRAFFIC_CLASS,
    FILTER_FIELD_IPV6_NEXT_HEADER,
    FILTER_FIELD_IPV6_FLOW_LABEL,
    
    FILTER_FIELD_TCP_SPORT = 96,
    FILTER_FIELD_TCP_DPORT,
    FILTER_FIELD_TCP_FLAG,
    FILTER_FIELD_TCP_SEQ_NUMBER,
    FILTER_FIELD_TCP_ACK_NUMBER,
    FILTER_FIELD_UDP_SPORT,
    FILTER_FIELD_UDP_DPORT,
    FILTER_FIELD_ICMP_CODE,
    FILTER_FIELD_ICMP_TYPE,
    FILTER_FIELD_IGMP_TYPE,
    FILTER_FIELD_END
} rtk_filter_field_type_t;
#endif

typedef enum rtk_filter_flag_care_type_e
{
    FILTER_FLAG_CARE_DONT_CARE = 0,
    FILTER_FLAG_CARE_1,
    FILTER_FLAG_CARE_0,
    FILTER_FLAG_END
} rtk_filter_flag_care_type_t;

typedef uint32  rtk_filter_id_t;    /* filter id type */

typedef enum rtk_filter_invert_e
{
    FILTER_INVERT_DISABLE = 0,
    FILTER_INVERT_ENABLE = 0,
    FILTER_INVERT_END
} rtk_filter_invert_t;

typedef uint32 rtk_filter_port_t;

typedef uint32 rtk_filter_state_t;

typedef enum rtk_filter_state_e
{
    FILTER_STATE_DISABLE = 0,
    FILTER_STATE_ENABLE,
} rtk_filter_state_type_t;

typedef enum rtk_filter_unmatch_action_e
{
    FILTER_UNMATCH_DROP = 0,
    FILTER_UNMATCH_PERMIT,
    FILTER_UNMATCH_END,
} rtk_filter_unmatch_action_t;

typedef enum rtk_igmp_type_e
{
    IGMP_IPV4 = 0,
    IGMP_MLD,
    IGMP_TYPE_END
} rtk_igmp_type_t;

typedef uint32 rtk_int_info_t;

typedef enum rtk_int_interrupt_type_e
{
    INT_TYPE_LINK_STATUS = 0,
    INT_TYPE_METER_EXCEED,
    INT_TYPE_LEARN_LIMIT,    
    INT_TYPE_LINK_SPEED,
    INT_TYPE_CONGEST,
    INT_TYPE_GREEN_FEATURE,
    INT_TYPE_LOOP_DETECT,
    INT_TYPE_8051,
    INT_TYPE_END
}rtk_int_interrupt_type_t;

typedef enum rtk_int_advType_e
{
    ADV_L2_LEARN_PORT_MASK = 0,
    ADV_SPEED_CHANGE_PORT_MASK,
    ADV_SPECIAL_CONGESTION_PORT_MASK,
    ADV_PORT_LINKDOWN_PORT_MASK,
    ADV_PORT_LINKUP_PORT_MASK,
    ADV_METER0_15_MASK,
    ADV_METER16_31_MASK,
    ADV_METER32_47,_MASK,
    ADV_METER48_63_MASK,
    ADV_END,
} rtk_int_advType_t;

typedef enum rtk_int_polarity_e
{
    INT_POLAR_HIGH = 0,
    INT_POLAR_LOW,
    INT_POLAR_END
} rtk_int_polarity_t;

#if 0
typedef struct  rtk_int_status_s
{
    uint8 value[RTK_MAX_NUM_OF_INTERRUPT_TYPE];
} rtk_int_status_t;
#endif

typedef uint32  rtk_int_status_t;

typedef enum rtk_int_type_e
{
    INT_GLOBAL_RESERVED0 = 0,
    INT_GLOBAL_SERDES,
    INT_GLOBAL_LOOPDETECT,
    INT_GLOBAL_SALRNLIMIT,
    INT_GLOBAL_PKTDROP,
    INT_GLOBAL_LINKCHANGE,
    INT_GLOBAL_RESERVED6 = 6,
    INT_GLOBAL_RESERVED7 = 7,
    INT_GLOBAL_UDLD,
    INT_GLOBAL_MIBOVERFLOW,
    INT_GLOBAL_SAMOV,
    INT_GLOBAL_ACLHIT0,
    INT_GLOBAL_ACLHIT1,
    INT_GLOBAL_ACLHIT2,
    INT_GLOBAL_ACLHIT3,
    INT_GLOBAL_DYINGGSP,
    INT_GLOBAL_LOOPRESOLVED,
    INT_GLOBAL_END    
} rtk_int_type_t;

typedef enum rtk_l2_age_time_e
{
    AGE_TIME_300S= 0,
    AGE_TIME_2,
    AGE_TIME_3,
    AGE_TIME_4,
    AGE_TIME_END
} rtk_l2_age_time_t;

typedef enum rtk_l2_flood_type_e
{
    FLOOD_UNKNOWNDA = 0,
    FLOOD_UNKNOWNMC,
    FLOOD_BC,
    FLOOD_END
} rtk_l2_flood_type_t;

typedef uint32 rtk_l2_flushItem_t;

typedef enum rtk_l2_flushType_e
{
    FLUSH_TYPE_BY_PORT = 0,       /* physical port       */
//    FLUSH_TYPE_BY_VID ,     
//    FLUSH_TYPE_BY_VID_PORT,     /* vid + physical port */
//    FLUSH_TYPE_BY_TID,          /* trunk id            */
//    FLUSH_TYPE_BY_VID_TID,      /* vid + trunk id      */
    FLUSH_TYPE_END
} rtk_l2_flushType_t;


typedef enum rtk_l2_hash_method_e
{
    HSAH_OPT0 = 0,
    HASH_OPT1,
    HASH_END
} rtk_hash_method_t;

/* l2 limit learning count action */
typedef enum rtk_l2_limitLearnCntAction_e
{
    LIMIT_LEARN_CNT_ACTION_DROP = 0,
    LIMIT_LEARN_CNT_ACTION_FWD,
    LIMIT_LEARN_CNT_ACTION_TO_CPU,
    LIMIT_LEARN_CNT_ACTION_END
} rtk_l2_limitLearnCntAction_t;

typedef enum rtk_l2_lookup_type_e
{
    LOOKUP_MAC = 0,
    LOOKUP_SIP_DIP,
    LOOKUP_DIP,  
    LOOKUP_END
} rtk_l2_lookup_type_t;

/* l2 address table - unicast data structure */
typedef struct rtk_l2_ucastAddr_s
{
    rtk_mac_t   mac;
    uint32      fid;
    uint32      port;
    uint32      sa_block;
    uint32      auth;
    uint32      is_static;
}rtk_l2_ucastAddr_t;


typedef enum rtk_leaky_type_e
{
    LEAKY_STATIC_LUT = 0,
    LEAKY_RRCP,
    LEAKY_RLDP,
    LEAKY_END,
}rtk_leaky_type_t;

typedef enum rtk_led_blink_rate_e
{
    LED_BLINKRATE_32MS=0,         
    LED_BLINKRATE_64MS,        
    LED_BLINKRATE_128MS,
    LED_BLINKRATE_256MS,
    LED_BLINKRATE_512MS,
    LED_BLINKRATE_1024MS,
    LED_BLINKRATE_48MS,
    LED_BLINKRATE_96MS,
    LED_BLINKRATE_END,
}rtk_led_blink_rate_t;

typedef enum rtk_led_group_e
{
    LED_GROUP_0 = 0,
    LED_GROUP_1,
    LED_GROUP_2,
    LED_GROUP_END
}rtk_led_group_t;    

typedef enum rtk_led_mode_e
{
    LED_MODE_0 = 0,
    LED_MODE_1,
    LED_MODE_2,
    LED_MODE_3,
    LED_MODE_END
}rtk_led_mode_t;    


typedef uint32  rtk_mac_cnt_t;     /* meter id type  */

typedef enum rtk_mcast_type_e
{
    MCAST_L2 = 0,
    MCAST_IPV4,
    MCAST_IPV6,
    MCAST_L3,
    MCAST_END
} rtk_mcast_type_t;

typedef uint32  rtk_meter_id_t;     /* meter id type  */

typedef uint32  rtk_mode_t; 

typedef uint32  rtk_port_t;        /* port is type */

typedef enum rtk_port_duplex_e
{
    PORT_HALF_DUPLEX = 0,
    PORT_FULL_DUPLEX,
    PORT_DUPLEX_END
} rtk_port_duplex_t;

typedef enum rtk_port_linkStatus_e
{
    PORT_LINKDOWN = 0,
    PORT_LINKUP,
    PORT_LINKSTATUS_END
} rtk_port_linkStatus_t;

typedef struct  rtk_port_mac_ability_s
{
    uint32 forcemode;
    uint32 speed;
    uint32 duplex;
    uint32 link;    
    uint32 nway;    
    uint32 txpause;
    uint32 rxpause;     
    //uint32 mstmode;
    //uint32 mstfault;    
    //uint32 lpi100;
    //uint32 lpi1000;    
}rtk_port_mac_ability_t;

typedef struct rtk_port_phy_ability_s
{   
    uint32    AutoNegotiation;  /*PHY register 0.12 setting for auto-negotiation process*/
    uint32    Half_10;          /*PHY register 4.5 setting for 10BASE-TX half duplex capable*/
    uint32    Full_10;          /*PHY register 4.6 setting for 10BASE-TX full duplex capable*/
    uint32    Half_100;         /*PHY register 4.7 setting for 100BASE-TX half duplex capable*/
    uint32    Full_100;         /*PHY register 4.8 setting for 100BASE-TX full duplex capable*/
    uint32    Full_1000;        /*PHY register 9.9 setting for 1000BASE-T full duplex capable*/
    uint32    FC;               /*PHY register 4.10 setting for flow control capability*/
    uint32    AsyFC;            /*PHY register 4.11 setting for  asymmetric flow control capability*/
} rtk_port_phy_ability_t;

typedef uint32  rtk_port_phy_data_t;     /* phy page  */

typedef uint32  rtk_port_phy_page_t;     /* phy page  */

typedef enum rtk_port_phy_reg_e  
{
    PHY_REG_CONTROL             = 0,
    PHY_REG_STATUS,
    PHY_REG_IDENTIFIER_1,
    PHY_REG_IDENTIFIER_2,
    PHY_REG_AN_ADVERTISEMENT,
    PHY_REG_AN_LINKPARTNER,
    PHY_REG_1000_BASET_CONTROL  = 9,
    PHY_REG_1000_BASET_STATUS,
    PHY_REG_END                 = 32
} rtk_port_phy_reg_t;

typedef enum rtk_port_phy_test_mode_e  
{
    PHY_TEST_MODE_NORMAL= 0,
    PHY_TEST_MODE_1,
    PHY_TEST_MODE_2,
    PHY_TEST_MODE_3,
    PHY_TEST_MODE_4,
    PHY_TEST_MODE_END           
} rtk_port_phy_test_mode_t;

typedef enum rtk_port_speed_e
{
    PORT_SPEED_10M = 0,
    PORT_SPEED_100M,
    PORT_SPEED_1000M,
    PORT_SPEED_END
} rtk_port_speed_t;

typedef struct rtk_portmask_s
{
    uint32  bits[RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST];
} rtk_portmask_t;

typedef uint32  rtk_pri_t;         /* priority vlaue */
typedef uint32  rtk_dei_t;        

typedef struct rtk_priority_select_s
{   uint32 group;       /*RTL8316D has two groups of priority selection*/
    uint32 port_pri;
    uint32 dot1q_pri;
    uint32 dscp_pri;
    uint32 cvlan_pri;
    uint32 svlan_pri;
} rtk_priority_select_t;


typedef uint32  rtk_qid_t;        /* queue id type */
typedef uint8  rtk_aclBlock_id_t;        /* ACL block ID type */
typedef uint8  rtk_aclRule_id_t;        /* rule ID type */

typedef struct rtk_qos_pri2queue_s
{
    uint32 pri2queue[RTK_MAX_NUM_OF_PRIORITY];
} rtk_qos_pri2queue_t;

typedef struct rtk_qos_queue_weights_s
{
    uint32 weights[RTK_MAX_NUM_OF_QUEUE];
} rtk_qos_queue_weights_t;

typedef enum rtk_qos_scheduling_type_e
{
    WFQ = 0,        /* Weighted-Fair-Queue */
    WRR,            /* Weighted-Round-Robin */
    SCHEDULING_TYPE_END
} rtk_qos_scheduling_type_t;

typedef uint32  rtk_queue_num_t;    /* queue number*/

typedef enum rtk_rate_storm_group_e
{
    STORM_GROUP_UNKNOWN_UNICAST = 0,
    STORM_GROUP_UNKNOWN_MULTICAST,
    STORM_GROUP_MULTICAST,
    STORM_GROUP_BROADCAST,
    STORM_GROUP_END
} rtk_rate_storm_group_t;

typedef uint32  rtk_rate_t;     /* rate type  */

typedef uint64 rtk_stat_counter_t;


/* global statistic counter structure */
typedef struct rtk_stat_global_cntr_s
{
    uint32 dot1dTpLearnedEntryDiscards;
    //uint32 dot1dTpPortInDiscards;
    //uint32 OutUnicastPktsCnt;
    //uint32 OutMulticastPktsCnt;
    //uint32 OutBrocastPktsCnt;
    //uint32 egrLackResourceDrop;
}rtk_stat_global_cntr_t;

typedef enum rtk_stat_global_type_e
{
    DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX = 36,
    MIB_GLOBAL_CNTR_END
}rtk_stat_global_type_t;

/* port statistic counter structure */
typedef struct rtk_stat_port_cntr_s
{
    uint64 ifOutOctets;
    uint64 ifInOctets;
    uint32 ifInUcastPkts;
    uint32 dot3OutPauseFrames;
    uint32 dot3OutPauseOnFrames;
    uint32 dot3StatsExcessiveCollisions;
    uint32 dot3StatsLateCollisions;
    uint32 dot3StatsDeferredTransmissions;
    uint32 dot3StatsMultipleCollisionFrames;
    uint32 dot3StatsSingleCollisionFrames;
    uint32 dot3CtrlInUnknownOpcodes;
    uint32 dot3InPauseFrames;
    uint32 dot3StatsSymbolErrors;
    uint32 dot3StatsFCSErrors;
    uint32 etherStatsJabbers;
    uint32 etherStatsCollisions;
    uint32 etherStatsMcastPkts;
    uint32 etherStatsBcastPkts;
    uint32 etherStatsFragments;
    uint32 etherStatsPkts64Octets;
    uint32 etherStatsPkts65to127Octets;
    uint32 etherStatsPkts128to255Octets;
    uint32 etherStatsPkts256to511Octets;
    uint32 etherStatsPkts512to1023Octets;
    uint32 etherStatsPkts1024toMaxOctets;
    uint32 etherStatsOversizePkts;
    uint64 etherStatsOctets;
    uint32 etherStatsUndersizePkts;
    uint32 igrLackPktBufDrop;
    uint32 flowCtrlOnDropPktCnt;
    uint32 txCrcCheckFailCnt;
    uint32 smartTriggerHit0;
    uint32 smartTriggerHit1;
    uint32 ifOutUcastPkts;
    uint32 ifOutMulticastPkts;
    uint32 ifOutBrocastPkts;
}rtk_stat_port_cntr_t;

/* port statistic counter index */
typedef enum rtk_stat_port_type_e
{
    IfInOctets = 0,
    Dot3StatsFCSErrors,
    Dot3StatsSymbolErrors,
    Dot3InPauseFrames,
    Dot3ControlInUnknownOpcodes,        
    EtherStatsFragments,
    EtherStatsJabbers,
    IfInUcastPkts,
    EtherStatsDropEvents,
    EtherStatsOctets,
    EtherStatsUnderSizePkts,
    EtherOversizeStats,
    EtherStatsPkts64Octets,
    EtherStatsPkts65to127Octets,
    EtherStatsPkts128to255Octets,
    EtherStatsPkts256to511Octets,
    EtherStatsPkts512to1023Octets,
    EtherStatsPkts1024to1518Octets,
    EtherStatsMulticastPkts,
    EtherStatsBroadcastPkts,    
    IfOutOctets,
    Dot3StatsSingleCollisionFrames,
    Dot3StatMultipleCollisionFrames,
    Dot3sDeferredTransmissions,
    Dot3StatsLateCollisions,
    EtherStatsCollisions,
    Dot3StatsExcessiveCollisions,
    Dot3OutPauseFrames,
    Dot1dBasePortDelayExceededDiscards,
    Dot1dTpPortInDiscards,
    IfOutUcastPkts,
    IfOutMulticastPkts,
    IfOutBroadcastPkts,
    OutOampduPkts,
    InOampduPkts,
    PktgenPkts,
    MIB_PORT_CNTR_END
}rtk_stat_port_type_t;

typedef uint32  rtk_stg_t;        /* spanning tree instance id type */

typedef enum rtk_storm_bypass_e
{
    BYPASS_BRG_GROUP = 0,
    BYPASS_FD_PAUSE,
    BYPASS_SP_MCAST,
    BYPASS_1X_PAE,
    BYPASS_UNDEF_BRG_04,
    BYPASS_UNDEF_BRG_05,
    BYPASS_UNDEF_BRG_06,
    BYPASS_UNDEF_BRG_07,    
    BYPASS_PROVIDER_BRIDGE_GROUP_ADDRESS,
    BYPASS_UNDEF_BRG_09,
    BYPASS_UNDEF_BRG_0A,
    BYPASS_UNDEF_BRG_0B,
    BYPASS_UNDEF_BRG_0C,    
    BYPASS_PROVIDER_BRIDGE_GVRP_ADDRESS,    
    BYPASS_8021AB,
    BYPASS_UNDEF_BRG_0F,    
    BYPASS_BRG_MNGEMENT,
    BYPASS_UNDEFINED_11,
    BYPASS_UNDEFINED_12,
    BYPASS_UNDEFINED_13,
    BYPASS_UNDEFINED_14,
    BYPASS_UNDEFINED_15,
    BYPASS_UNDEFINED_16,
    BYPASS_UNDEFINED_17,
    BYPASS_UNDEFINED_18,
    BYPASS_UNDEFINED_19,
    BYPASS_UNDEFINED_1A,
    BYPASS_UNDEFINED_1B,
    BYPASS_UNDEFINED_1C,
    BYPASS_UNDEFINED_1D,
    BYPASS_UNDEFINED_1E,
    BYPASS_UNDEFINED_1F,
    BYPASS_GMRP,
    BYPASS_GVRP,
    BYPASS_UNDEF_GARP_22,
    BYPASS_UNDEF_GARP_23,
    BYPASS_UNDEF_GARP_24,
    BYPASS_UNDEF_GARP_25,
    BYPASS_UNDEF_GARP_26,
    BYPASS_UNDEF_GARP_27,
    BYPASS_UNDEF_GARP_28,
    BYPASS_UNDEF_GARP_29,
    BYPASS_UNDEF_GARP_2A,
    BYPASS_UNDEF_GARP_2B,
    BYPASS_UNDEF_GARP_2C,
    BYPASS_UNDEF_GARP_2D,
    BYPASS_UNDEF_GARP_2E,
    BYPASS_UNDEF_GARP_2F,
    BYPASS_IGMP,
    BYPASS_END,
}rtk_storm_bypass_t;

typedef uint32  rtk_stp_msti_id_t;     /* MSTI ID  */

typedef enum rtk_stp_state_e
{
    STP_STATE_DISABLED = 0,
    STP_STATE_BLOCKING,
    STP_STATE_LEARNING,
    STP_STATE_FORWARDING,
    STP_STATE_END
} rtk_stp_state_t;

typedef uint32 rtk_svlan_index_t;

typedef struct rtk_svlan_memberCfg_s{
    uint32 svid;   
    uint32 memberport;
    uint32 fid;
    uint32 priority;
    uint32 reserved1;     
    uint32 reserved2;    
    uint32 reserved3; 
    uint32 reserved4;   
}rtk_svlan_memberCfg_t;

typedef enum rtk_svlan_pri_ref_e
{
    REF_INTERNAL_PRI = 0,
    REF_CTAG_PRI,
    REF_SVLAN_PRI,
    REF_PRI_END
} rtk_svlan_pri_ref_t;


typedef uint32 rtk_svlan_tpid_t;

typedef enum rtk_switch_maxPktLen_e
{
    MAXPKTLEN_1522B = 0,
    MAXPKTLEN_1536B,
    MAXPKTLEN_1552B,
    MAXPKTLEN_16000B,   
    MAXPKTLEN_END   
} rtk_switch_maxPktLen_t;

typedef enum rtk_trap_igmp_action_e
{
    IGMP_ACTION_FORWARD = 0,
    IGMP_ACTION_TRAP2CPU,        
    IGMP_ACTION_FORWARD_COPY2_CPU,
    IGMP_ACTION_DROP,
    IGMP_ACTION_END
} rtk_trap_igmp_action_t;

typedef enum rtk_trap_mcast_action_e
{
    MCAST_ACTION_DROP = 0,    
    MCAST_ACTION_TRAP2CPU, 
    MCAST_ACTION_FORWARD,  /*flood in VLAN*/  
    MCAST_ACTION_flood,  /*flood to all ports*/  
    MCAST_ACTION_END
} rtk_trap_mcast_action_t;

typedef enum rtk_trap_reason_type_e
{
   // TRAP_REASON_RMA = 0,
    TRAP_REASON_IGMPMLD = 0,
  //  TRAP_REASON_1XEAPOL,
  //  TRAP_REASON_VLANERR,
  //  TRAP_REASON_SLPCHANGE,
   // TRAP_REASON_MULTICASTDLF,
    TRAP_REASON_CFI1,
  //  TRAP_REASON_1XUNAUTH,
    TRAP_REASON_UNMATCHIPMC,        
    TRAP_REASON_LOOKUPMISS,    
    TRAP_REASON_IPOPTHDR,    
    TRAP_REASON_END,

} rtk_trap_reason_type_t;


typedef enum rtk_trap_rma_action_e
{
    RMA_ACTION_FORWARD = 0,
    RMA_ACTION_TRAP2CPU,        
    RMA_ACTION_DROP,
    RMA_ACTION_FORWARD_COPY2_CPU,
    RMA_ACTION_END
} rtk_trap_rma_action_t;

typedef enum rtk_trap_ucast_action_e
{
    UCAST_ACTION_DROP = 0,
    UCAST_ACTION_TRAP2CPU,
    UCAST_ACTION_FORWARD, /*flood within VLAN*/
    UCAST_ACTION_FLOOD, /*flood to all ports*/
    UCAST_ACTION_END
} rtk_trap_ucast_action_t;

typedef enum rtk_trap_ucast_type_e
{
    UCAST_UNKNOWNDA = 0,
    UCAST_UNKNOWNSA,
    UCAST_UNMATCHSA,
    UCAST_END
} rtk_trap_ucast_type_t;

typedef enum rtk_trunk_group_e
{
    TRUNK_GROUP0 = 0,
    TRUNK_GROUP1,
    TRUNK_GROUP2,
    TRUNK_GROUP3,
    TRUNK_GROUP_END
} rtk_trunk_group_t;

typedef struct  rtk_trunk_hashVal2Port_s
{
    uint8 value[RTK_MAX_NUM_OF_TRUNK_HASH_VAL];
} rtk_trunk_hashVal2Port_t;

typedef uint32  rtk_vlan_proto_type_t;     /* protocol and port based VLAN protocol type  */


typedef enum rtk_vlan_acceptFrameType_e
{
    ACCEPT_FRAME_TYPE_ALL = 0,             /* untagged, priority-tagged and tagged */
    ACCEPT_FRAME_TYPE_TAG_ONLY,         /* tagged */
    ACCEPT_FRAME_TYPE_UNTAG_ONLY,     /* untagged and priority-tagged */
//  ACCEPT_FRAME_TYPE_PRITAG_AND_TAG, /* priority-tagged and tagged */
    ACCEPT_FRAME_TYPE_END
} rtk_vlan_acceptFrameType_t;


/* frame type of protocol vlan - reference 802.1v standard */
typedef enum rtk_vlan_protoVlan_frameType_e
{
    FRAME_TYPE_ETHERNET = 0,
    FRAME_TYPE_LLCOTHER,
    FRAME_TYPE_RFC1042,
    //FRAME_TYPE_SNAP8021H,
    //FRAME_TYPE_SNAPOTHER,
    FRAME_TYPE_END
} rtk_vlan_protoVlan_frameType_t;

typedef uint32  rtk_vlan_t;        /* vlan id type */

/* Protocol-and-port-based Vlan structure */
typedef struct rtk_vlan_protoAndPortInfo_s
{
    uint32                         proto_type;
    rtk_vlan_protoVlan_frameType_t frame_type;
    rtk_vlan_t                     cvid;
    rtk_pri_t                      cpri;
}rtk_vlan_protoAndPortInfo_t;

/* tagged mode of VLAN - reference realtek private specification */
typedef enum rtk_vlan_tagMode_e
{
    VLAN_TAG_MODE_ORIGINAL = 0,
    VLAN_TAG_MODE_KEEP_FORMAT,
    VLAN_TAG_MODE_REAL_KEEP_FORMAT,
    VLAN_TAG_MODE_PRI,
    VLAN_TAG_MODE_END
} rtk_vlan_tagMode_t;

/* tagged mode of VLAN - reference realtek private specification */
typedef enum rtk_rtkpp_udldMode_e
{
    UDLDMODE_NORMAL = 0,
    UDLDMODE_HEARTBEAT,
    UDLDMODE_END
} rtk_rtkpp_udldMode_t;


/*type of template field*/
typedef enum rtk_filter_field_type_e 
{
    PIETEM_FMT = 0,
    PIETEM_DMAC0,
    PIETEM_DMAC1,
    PIETEM_DMAC2,
    PIETEM_SMAC0,
    PIETEM_SMAC1,
    PIETEM_SMAC2,
    PIETEM_ETHERTYPE,
    PIETEM_OTAG,
    PIETEM_ITAG,
    PIETEM_PRIORITY,
    PIETEM_L2PROTOCOL,
    PIETEM_SIP0,
    PIETEM_SIP1,
    PIETEM_DIP0,
    PIETEM_DIP1,
    PIETEM_IPHDR,
    PIETEM_GIP0,
    PIETEM_GIP1,
    PIETEM_L4SPORT,
    PIETEM_L4DPORT,
    PIETEM_L4HDR,
    PIETEM_ICMPCODETYPE,
    PIETEM_SPM0,
    PIETEM_SPM1,
    PIETEM_RANGE0,
    PIETEM_RANGE1,
    PIETEM_PAYLOAD,
    PIETEM_UNUSED,
    PIETEM_END,
}rtk_filter_field_type_t;

typedef uint8 rtk_filter_templField_id_t;    /* template field ID*/


typedef struct rtk_rtkpp_seed_s{
    uint8 seed[8];
}rtk_rtkpp_seed_t;

typedef struct rtk_rtkpp_randomNum_s{
    uint8 number[8];
}rtk_rtkpp_randomNum_t;






typedef struct rtk_filter_pieTempl_grpIp1_s
{
        uint16 reserv  : 4;
        uint16 grpIp28_16 : 12;
}rtk_filter_pieTempl_grpIp1_t;

typedef struct rtk_filter_pieTempl_l4Hdr_s
{
        uint16 tcpFlags  : 6;
        uint16 reserv    : 2;
        uint16 igmpType  : 8;
}rtk_filter_pieTempl_l4Hdr_t;



typedef struct rtk_filter_pieTempl_otag_s
{
    uint16 opri : 3;
    uint16 dei    : 1;
    uint16 ovid     : 12;
}rtk_filter_pieTempl_otag_t;


typedef struct rtk_filter_pieTempl_fmt_s
{
    uint16 noneZeroOff : 1;
    uint16 recvPort    : 5;
    uint16 tgL2Fmt     : 2;
    uint16 itagExist   : 1;
    uint16 otagExist   : 1;
    uint16 tgL23Fmt    : 2;
    uint16 tgL4Fmt     : 3;
    uint16 ispppoe     : 1;
}rtk_filter_pieTempl_fmt_t;

typedef struct rtk_filter_pieTempl_itag_s
{
    uint16 ipri : 3;
    uint16 itagExist    : 1;
    uint16 ivid     : 12;
}rtk_filter_pieTempl_itag_t;

typedef struct rtk_filter_pieTempl_prio_s
{
    uint16 itagPri  : 3;
    uint16 itagExist : 1;
    uint16 otagPri : 3;
    uint16 dei : 1;
    uint16 otagExist : 1;
    uint16 fwdPri    : 3;
    uint16 rsved : 2;
    uint16 oampdu : 1;
    uint16 cfi    : 1;    

}rtk_filter_pieTempl_prio_t;

typedef struct rtk_filter_pieTempl_ipHdr_s
{
    uint16 dscp  : 6;
    uint16 morefrag : 1;
    uint16 noneZeroOff : 1;
    uint16 l4Proto : 8;
}rtk_filter_pieTempl_ipHdr_t;

typedef struct rtk_filter_pieTempl_icmpHdr_s
{
        uint16 icmpCode  : 8;
        uint16 icmpType  : 8;
}rtk_filter_pieTempl_icmpHdr_t;

typedef struct rtk_filter_pieTempl_rcvPortMask0_s
{
    uint16 rcvPortMask15_0;
}rtk_filter_pieTempl_rcvPortMask0_t;

typedef struct rtk_filter_pieTempl_rcvPortMask1_s
{
        uint16 rcvPortMask24_16  : 9;
        uint16 dmacType  : 2;
        uint16 doNotFrag  : 1;
        uint16 ttlType  : 2;
        uint16 rtkpp  : 2;
}rtk_filter_pieTempl_rcvPortMask1_t;

typedef struct rtk_filter_pieTempl_rangeTbl_s
{
        uint16 l4Port  : 4;
        uint16 rcvPort  : 4;
        uint16 ipRange  : 8;
}rtk_filter_pieTempl_rangeTbl_t;

typedef struct rtk_filter_pieTempl_vidRange_s
{
    uint16 vidRangeChk;
}rtk_filter_pieTempl_vidRange_t;


typedef struct rtk_filter_pieTempl_field_s
{
    union {
        rtk_filter_pieTempl_fmt_t format;
        uint16                      dmac0;
        uint16                      dmac1;
        uint16                      dmac2;
        uint16                      smac0;
        uint16                      smac1;
        uint16                      smac2;
        uint16                      ethType;
        rtk_filter_pieTempl_otag_t otag;
        rtk_filter_pieTempl_itag_t itag;
        rtk_filter_pieTempl_prio_t prio;
        uint16                      l2Proto;
        uint16                       sip15_0;
        uint16                       sip31_16;
        uint16                       dip15_0;
        uint16                       dip31_16;
        rtk_filter_pieTempl_ipHdr_t ipHdr;
        uint16                       grpIp15_0;        
        rtk_filter_pieTempl_grpIp1_t grpIph;
        uint16                        l4SrcPort;
        uint16                        l4DstPort;      
        rtk_filter_pieTempl_l4Hdr_t l4Hdr;
        rtk_filter_pieTempl_icmpHdr_t icmpHdr;
        rtk_filter_pieTempl_rcvPortMask0_t rcvSrcPm0;
        rtk_filter_pieTempl_rcvPortMask1_t rcvSrcPm1;
        rtk_filter_pieTempl_vidRange_t vidRange;
        rtk_filter_pieTempl_rangeTbl_t rangeTbl;
        uint16                        payload;               
    }un;
}rtk_filter_pieTempl_field_t;

typedef struct rtk_filter_aclField_s
{
    rtk_filter_field_type_t type;
    rtk_filter_pieTempl_field_t fieldData;
    rtk_filter_pieTempl_field_t fieldMask;
    struct rtk_filter_aclField_s* next;
}rtk_filter_aclField_t;

typedef struct rtk_filter_aclRule_field_s
{
    rtk_filter_field_type_t fieldType;
    rtk_filter_pieTempl_field_t data;
    rtk_filter_pieTempl_field_t mask;
}rtk_filter_aclRule_field_t;


typedef struct rtk_filter_aclEntry_s
{   
    rtk_aclRule_id_t ruleId;  
    uint8 fieldNum;                 /*number of fields in this entry*/
    uint8 reverse;                  /*result reversed*/
    rtk_filter_aclField_t* pField;
    uint32* pAction; /*rtl8316d_aclAct_entry_t* ACL action*/
    struct rtk_filter_aclEntry_s* prev;
    struct rtk_filter_aclEntry_s* next;
}rtk_filter_aclEntry_t;

typedef enum rtk_filter_actionType_e
{
    ACLACTTYPE_FNO = 0,      /*flow number*/
    ACLACTTYPE_DROP,     
    ACLACTTYPE_CP2CPU,   /*copy to CPU*/
    ACLACTTYPE_MIRROR,   /*mirror*/
    ACLACTTYPE_OTAG,     /*outer tag*/
    ACLACTTYPE_ITAG,     /*inner tag*/
    ACLACTTYPE_PRIRMK,   /*priority remarking*/
    ACLACTTYPE_REDIR,    /*redirection*/
    ACLACTTYPE_DSCPRMK,  /*DSCP/TOS remarking*/
    ACLACTTYPE_PRIORITY, /*priority assignment*/
    ACLACTTYPE_END,
}rtk_filter_actionType_t;

typedef uint8 rtk_filter_aclAct_drop_t;/*1:drop, 2:withdraw drop*/

typedef uint8 rtk_filter_aclAct_fno_t;
typedef uint8 rtk_filter_aclAct_cp2cpu_t;
typedef uint8 rtk_filter_aclAct_mirror_t;

typedef struct rtk_filter_aclAct_itagop_s
{
    uint32 withdraw:1;
    uint32 inVidCtl:2;
    uint32 inVidInfo:12;
    uint32 inTagOp:2;
    uint32 reserved:15;
} rtk_filter_aclAct_itagop_t;


typedef struct rtk_filter_aclAct_redirect_s
{
    uint32 withdraw;
    uint32 opcode:1;
    union
    {
        struct
        {
            uint32 dpn:5; /*destination port number, 30 means drop, 31 means flood within VLAN */
        } uniRedirect;
        struct
        {
            uint32 ftIdx;
        } multiRedirect;
    } redirect;
} rtk_filter_aclAct_redirect_t;

typedef struct rtk_filter_aclAct_prioRmk_s
{
    uint32 withdraw:1;
    uint32 tagSel:3;      /*xx1:inner priority, x1x:outer priority, 1xx:dei */
    uint32 inPri:3;         /*1: remark inner priority */
    uint32 outPri:3;       /*1: remark outer priority */
    uint32 dei:1;           /*DEI field of outer VLAN tag, used only when rmkDei is 1. */
} rtk_filter_aclAct_prioRmk_t;


typedef struct rtk_filter_aclAct_otagop_s
{
    uint32 withdraw;

    /*   0: reserved, 
      *   1: provide new outer VID as OutVIDInfo. 
      *   2: packet's new outer VID = packet's outer tag VID - OutVIDInfo
      *   3: packet's new outer VID = packet's outer tag VID + OutVIDInfo
      */

    uint32 outVidCtl:2; 
    uint32 outVidInfo:12;
    /*    0: packet sent without outer VLAN tag, 1: packet sent with outer VLAN tag
      *   others: Nop */
    uint32 outTagOp:2; 
} rtk_filter_aclAct_otagop_t;

typedef struct rtk_filter_aclAct_dscpRmk_s
{
    uint32 withdraw;
    uint32 opcode:2;
    /*0: DSCP remarking, 1: modify IP precedence, 2: modify DTR */
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
} rtk_filter_aclAct_dscpRmk_t;

typedef struct rtk_filter_aclAct_priority_s
{
    uint32 withdraw: 1;
    uint32 priority: 3;
} rtk_filter_aclAct_priority_t;

typedef struct rtk_filter_aclAction_s
{    
    union{
        rtk_filter_aclAct_fno_t fno;
        rtk_filter_aclAct_drop_t dropInfo;
        rtk_filter_aclAct_cp2cpu_t cp2cpu;
        rtk_filter_aclAct_mirror_t mirror;
        rtk_filter_aclAct_otagop_t outTagOpInfo;        
        rtk_filter_aclAct_itagop_t inTagOpInfo;
        rtk_filter_aclAct_prioRmk_t priRmkInfo;
        rtk_filter_aclAct_redirect_t redirInfo;
        rtk_filter_aclAct_dscpRmk_t dscpRmkInfo;
        rtk_filter_aclAct_priority_t prioInfo;
    }un;
}rtk_filter_aclAction_t;


#endif /* __RTK_API_H__ */

