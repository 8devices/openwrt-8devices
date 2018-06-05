/*
 *	Abstract: header file for rtl819x_swNic.c
 *
 *	Author:
 *		Joey Lin, <joey.lin@realtek.com>
 *
 *	Copyright (c) 2015 Realtek Semiconductor Corp.
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */


#ifndef RTL819X_SWNIC_H
#define	RTL819X_SWNIC_H

#define	NEW_NIC_MAX_RX_DESC_RING    6
#define	NEW_NIC_MAX_TX_DESC_RING    4

// DESCRIPTOR BIT MAP
/*TX/RX share */

#define DescOwn			(1 << 0) /* Descriptor is owned by NIC */
#define RingEnd			(1 << 1) /* End of descriptor ring */
#define LastFrag		(1 << 2) /* Final segment of a packet */
#define FirstFrag		(1 << 3) /* First segment of a packet */



/* Tx descriptor opts1 */

	#define TD_TYPE_OFFSET			29	/* [31:29] 000: Ethernet, 001:PPTP, 010:IP, 011:ICMP, 100:IGMP, 101:TCP, 110:UDP, 111:IPv6 */
	#define TD_TYPE_MASK			(0x7)
	#define TD_VI_OFFSET			28	/* [28] Pkt is VLAN tagged */
	#define TD_LI_OFFSET			27	/* [27] Pkt has LLC hdr */
	#define TD_PI_OFFSET			26	/* [26] Pkt has PPPoE hdr */
	#define TD_PPPIDX_OFFSET		23	/* [25:23] Destination PPPoE Table Idx */
	#define TD_PPPIDX_MASK			(0x7)
	#define TD_PHLEN_OFFSET			6	/* [22:6] The packet length In RTL8198E; */
										/* should support up to 64K bytes for LSO, max. */
										/* normal packet size (without LSO) need follow PCR accept packet size. */
	#define TD_PHLEN_MASK			(0x1FFFF)
	#define TD_BRIDGE_OFFSET		5	/* [5] BRIDGING */
	#define TD_HWLKUP_OFFSET		4	/* [4] HWLOOKUP */
	/* { BRIDGING, HWLOOKUP }
	   2'b00: Send directly as is to the destination portlist assigned.
	   However, if any of the following offload option bits (in this Tx table) are set,
	   software should tell SwitchCore ALL protocol types enclosed in the packet via ph_proto field.

	   2'b01: Offload destination portlist resolving to SwitchCore and Do L2/Routing/NAPT,
	   based on whole packet content. SwitchCore  ignores the ph_proto field.

	   2'b10: Invalid setting.

	   2'b11: Offload destination portlist resolving to SwitchCore and Do L2 lookup(bridging) only,
	   based on DMAC given. SwitchCore ignores the ph_proto field. */
										/* [3] FirstFrag */
										/* [2] LastFrag */
										/* [1] RingEnd */
										/* [0] DescOwn */
	#define TD_BRIDGE_MASK			(1 << 5)
	#define TD_HWLKUP_MASK			(1 << 4)



/* Tx descriptor opts2 */

	#define TD_M_LEN_OFFSET			15	/* Length of actual data in associated cluster buffer for Tx */
	#define TD_M_LEN_MASK			(0x1FFFF)
	#define TD_QID_OFFSET			12	/* CPU Queue priority for direct Tx */
	#define TD_QID_MASK				(0x7)
	#define TD_PQID_OFFSET			9	/* Queue priority for direct Tx */
	#define TD_PQID_MASK			(0x7)
	#define TD_VLANTAGSET_OFFSET	0	/* [8:6] Ext port Tag mask; [5:0] port Tag mask; [4:0] physical port; Bit[5] MII port*/
	#define TD_VLANTAGSET_MASK		(0x1FF)



/* Tx descriptor opts3 */

	#define TD_PTP_PKT_OFFSET		31	/* [31] PTP packet indication */
	#define TD_PTP_TYP_OFFSET		27	/* [30:27] msg type */
	#define TD_PTP_TYP_MASK			(0xF)
	#define TD_PTP_VER_OFFSET		25	/* [26:25] PTP version, 0: 1588v1; 1: 1588v2 or 802.1as; others: reserved */
	#define TD_PTP_VER_MASK			(0x3)
	#define TD_DPRI_OFFSET			22	/* [24:22] Destination Priority */
	#define TD_DPRI_MASK			(0x7<<22)
	#define TD_PO_OFFSET			21	/* [21] 0: PPPoE header, if present, already attached */
										/* 1: Offload PPPoE header generation to SwitchCore */
	#define TD_L3CS_OFFSET			20	/* [20] 0: IP checksum calculated. Send it as is */
										/* 1: Offload IPv4 (w/wo option) checksum calculation to SwitchCore */
	#define TD_L3CS_MASK			(1<<20)
	#define TD_L4CS_OFFSET			19	/* [19] 0: TCP/UDP/ICMP checksum calculated. Send it as is */
										/* 1: Offload TCP/UDP/ICMP checksum calculation to SwitchCore */
	#define TD_L4CS_MASK			(1<<19)
	#define TD_IPV6_OFFSET			18	/* [18] IPv6 Header flag */
	#define TD_IPV6_MASK			(1<<18)	/* [18] IPv6 Header flag */
	#define TD_IPV4_OFFSET			17	/* [17] IPv4 Header flag */
	#define TD_IPV4_MASK			(1<<17)	/* [17] IPv4 Header flag */
	#define TD_IPV4_1ST_OFFSET		16	/* [16] IPv4 first header */
	#define TD_IPV4_1ST_MASK		(1<<16)	/* [16] IPv4 first header */
										/* [15] Reserved */
	#define TD_DP_EXT_OFFSET		12	/* [14:12] Destination extension port mask: */
										/* (only for CPU direct Tx & single ext. port) */
	#define TD_DP_EXT_MASK			(0x7)
	#define TD_DVLANID_OFFSET		0	/* [11:0] (1) Source VLAN ID selected for from extension port; */
									/* (2) Destination VLAN ID from CPU (really work) */
	#define TD_DVLANID_MASK			(0xFFF)


/* Tx descriptor opts4 */

	#define TD_LSO_OFFSET			31	/* [31] LSO flag indication */
	#define TD_LSO_MASK				(1<<31)
	#define TD_DP_OFFSET			24	/* [30:24] destination port mask, Bit[6]: CPU port; Bit[5-0]: Port5-Port0 */
										/* (If pkt was sent to CPU/extension port (ex: to extension port),Bits[6]=1.need refer to dp_ext[2:0] ) */
	#define TD_DP_MASK				(0x7F)
										/* [23] Reserved */
	#define TD_LINKED_OFFSET		16	/* [22:16] For WLAN WDS multiple tunnel */
	#define TD_LINKED_MASK			(0x7F)
	#define TD_IPV6_HDRLEN_OFFSET	0	/* [15:0] IPv6 header length */
	#define TD_IPV6_HDRLEN_MASK		(0xFFFF)



/* Tx descriptor opts5 */
	#define TD_EXTSPA_OFFSET		30	/* [31:30] extspa: Source extension port number: 2'd0: from CPU, */
										/* 2'd1~2'd3: from extension port numbers */
	#define TD_EXTSPA_MASK			(0x3)
	#define TD_MSS_OFFSET			16	/* [29:16] if (type==TCP), TCP segmentation size; */
										/* if (type != TCP), means MTU for UFO/IPF, and (MTU-IP header length) MUST be 8*n. */
	#define TD_MSS_MASK				(0x3FFF)
	#define TD_FLAGS_OFFSET			8	/* [15:8] {ECN/2b,URG,ACK,PSH,RST,SYN,FIN} in TCP header, if do TSO, PSH and FIN move to last segment packet */
	#define TD_FLAGS_MASK			(0xFF)
	#define TD_IPV4_HLEN_OFFSET		4	/* [7:4] header length field of IPv4 header in packet, unit: 4-byte */
	#define TD_IPV4_HLEN_MASK		(0xF)
	#define TD_TCP_HLEN_OFFSET		0	/* [3:0] header length field of TCP header in packet, unit: 4-byte */
	#define TD_TCP_HLEN_MASK		(0xF)



/* Rx descriptor opts1 */

	#define	RD_M_EXTSIZE_OFFSET		16	/* [31:16] FirstFrag */
	#define	RD_M_EXTSIZE_MASK		(0xFFFF)
										/* [15:4] Reserved */
										/* [3] FirstFrag */
										/* [2] LastFrag */
										/* [1] RingEnd */
										/* [0] DescOwn */


	
/* Rx descriptor opts2 */

	#define	RD_EXTSPA_OFFSET		24	/* [25:24] */
	#define	RD_EXTSPA_MASK			(0x3 << 24)
	#define	RD_DP_EXT_OFFSET		20
	#define	RD_DP_EXT_MASK			(0xF << 20)
	#define	RD_QID_OFFSET			17
	#define	RD_QID_MASK				(0x7 << 17)
										/* [16:14] Reserved */
	#define	RD_LEN_OFFSET			0
	#define	RD_LEN_MASK				(0x3FFF << 0)


/* Rx descriptor opts3 */

	#define	RD_TYPE_OFFSET			29	/* [31:29] */
	#define	RD_TYPE_MASK			(0x7 << 28)
	#define	RD_VO_OFFSET			28	/* [28] */
	#define	RD_LO_OFFSET			27	/* [27] */
	#define	RD_PO_OFFSET			26	/* [26] */
	#define	RD_PPPIDX_OFFSET		23	/* [25:23] */
	#define	RD_PPPIDX_MASK			(0x7 << 23)
	#define	RD_LINKID_OFFSET		16	/* [22:16] */
	#define	RD_LINKID_MASK			(0x7F << 16)
	#define	RD_REASON_OFFSET		0	/* [15:0] */
	#define	RD_REASON_MASK			(0xFFFF << 0)


/* Rx descriptor opts4 */

	#define	RD_EXT_VLANO_OFFSET		29	/* [31:29] */
	#define	RD_EXT_VLANO_MASK		(0x7 << 29)
	#define	RD_L2ACT_OFFSET			28	/* [28] */
	#define	RD_DVLANID_OFFSET		16	/* [27:16] */
	#define RD_DVLANID_MASK			(0xfff << 16)
	#define	RD_SPA_OFFSET			13	/* [15:13] */
	#define	RD_SPA_MASK				(0x7 << 13)
	#define	RD_LAST_F_OFFSET		12	/* [12] */
	#define	RD_FRAG_OFFSET			11	/* [11] */
	#define	RD_IPV4_1ST_OFFSET		10	/* [10] */
	#define	RD_IPV6_OFFSET			9	/* [9] */
	#define	RD_IPV6_MASK			(1<<9)
	#define	RD_IPV4_OFFSET			8	/* [8] */
	#define	RD_TOS_OFFSET			0	/* [7:0] */
	#define	RD_TOS_MASK				(0xFF << 0)


/* Rx descriptor opts5 */

	#define	RD_L3CSOK_OFFSET		31	/* [31] */
	#define	RD_L3CSOK_MASK			(1<<31)
	#define	RD_L4CSOK_OFFSET		30	/* [30] */
	#define	RD_L4CSOK_MASK			(1<<30)
	#define	RD_FWD_OFFSET			29	/* [29] */
	#define	RD_ORG_OFFSET			28	/* [28] */
	#define	RD_IP_MDF_OFFSET		25	/* [27:25] */
	#define	RD_IP_MDF_MASK			(0x7 << 24)
	#define	RD_VORG_OFFSET			24	/* [24] */
	#define	RD_LORG_OFFSET			23	/* [23] */
	#define	RD_PORG_OFFSET			22	/* [22] */
	#define	RD_DPRI_OFFSET			19	/* [21:19] */
	#define	RD_DPRI_MASK			(0x7 << 19)
	#define	RD_SPRI_OFFSET			16	/* [18:16] */
	#define	RD_SPRI_MASK			(0x7 << 16)
									/* [15] Reserved */
	#define	RD_EXT_TTL_OFFSET		12	/* [14:12] */
	#define	RD_EXT_TTL_MASK			(0x7 << 12)
	#define	RD_SVLANID_OFFSET		0	/* [11:0] */
	#define	RD_SVLANID_MASK			(0xFFF << 0)

struct rx_desc{
#ifdef _LITTLE_ENDIAN
	union{
		struct{
			uint32 own:1;         //0
			uint32 eor:1;         //1			
			uint32 ls:1;          //2
			uint32 fs:1;          //3
			uint32 rcdf:1;        //4
			uint32 fae:1;         //5
			uint32 rsvd:10;       //6~15	
			uint32 m_extsize:16;  //16~31	
		}bit;
		uint32 dw;
	}opts1;
	uint32 mdata;
	union{
		struct{
			uint32 len:14;     //0~13
			uint32 rsvd:3;       //14~16
			uint32 qid:3;         //17~19
			uint32 dp_ext:4;     //20~23
			uint32 extspa:2;         //24~25			
			uint32 rsvd2:6;         //26~31			
		}bit;
		uint32 dw;
	}opts2;
	union{
		struct{
			uint32 reason:16;     //0~15
			uint32 linkid:7;      //16~22
			uint32 ppp_idx:3;     //23~25
			uint32 po:1;          //26
			uint32 lo:1;          //27			
			uint32 vo:1;          //28
			uint32 type:3;        //29~31			
		}bit;
		uint32 dw;
	}opts3;
	union{
		struct{
			uint32 tos:8;         //0~7
			uint32 ipv4:1;        //8			
			uint32 ipv6:1;        //9
			uint32 ipv4_1st:1;    //10
			uint32 frag:1;        //11
			uint32 last_f:1;      //12
			uint32 spa:3;         //13~15
			uint32 dvlanid:12;    //16~27
			uint32 l2act:1;       //28
			uint32 ext_vlano:3;   //29~31						
		}bit;
		uint32 dw;
	}opts4;
	union{
		struct{
			uint32 svlanid:12;   //0~11
			uint32 ext_ttl:3;    //12~14
			uint32 rsvd:1;       //15
			uint32 spri:3;        //16~18
			uint32 dpri:3;       //19~21			
			uint32 porg:1;        //22		
			uint32 lorg:1;    //23
			uint32 vorg:1;        //24
			uint32 ip_mdf:3;      //25~27
			uint32 org:1;         //28
			uint32 fwd:1;    //29
			uint32 l4csok:1;       //30
			uint32 l3csok:1;   //31						
		}bit;
		uint32 dw;
	}opts5;
#else
	union{
		struct{
			uint32 m_extsize:16;  //16~31	
			uint32 rsvd:10;       //6~15	
			uint32 fae:1;         //5
			uint32 rcdf:1;        //4
			uint32 fs:1;          //3
			uint32 ls:1;          //2
			uint32 eor:1;         //1			
			uint32 own:1;         //0
		}bit;
		uint32 dw;
	}opts1;
	uint32 mdata;
	union{
		struct{
			uint32 rsvd2:6;         //26~31
			uint32 extspa:2;         //24~25			
			uint32 dp_ext:4;     //20~23
			uint32 qid:3;         //17~19
			uint32 rsvd:3;       //14~16
			uint32 len:14;     //0~13
		}bit;
		uint32 dw;
	}opts2;
	union{
		struct{
			uint32 type:3;        //29~31
			uint32 vo:1;          //28
			uint32 lo:1;          //27			
			uint32 po:1;          //26
			uint32 ppp_idx:3;     //23~25
			uint32 linkid:7;      //16~22
			uint32 reason:16;     //0~15
		}bit;
		uint32 dw;
	}opts3;
	union{
		struct{
			uint32 ext_vlano:3;   //29~31			
			uint32 l2act:1;       //28
			uint32 dvlanid:12;    //16~27
			uint32 spa:3;         //13~15
			uint32 last_f:1;      //12
			uint32 frag:1;        //11
			uint32 ipv4_1st:1;    //10
			uint32 ipv6:1;        //9
			uint32 ipv4:1;        //8			
			uint32 tos:8;         //0~7
		}bit;
		uint32 dw;
	}opts4;
	union{
		struct{
			uint32 l3csok:1;   //31			
			uint32 l4csok:1;       //30
			uint32 fwd:1;    //29
			uint32 org:1;         //28
			uint32 ip_mdf:3;      //25~27
			uint32 vorg:1;        //24
			uint32 lorg:1;    //23
			uint32 porg:1;        //22		
			uint32 dpri:3;       //19~21			
			uint32 spri:3;        //16~18
			uint32 rsvd:1;       //15
			uint32 ext_ttl:3;    //12~14
			uint32 svlanid:12;   //0~11
		}bit;
		uint32 dw;
	}opts5;
#endif

#define rx_own			opts1.bit.own
#define rx_eor			opts1.bit.eor
#define rx_ls			opts1.bit.ls
#define rx_fs			opts1.bit.fs
#define rx_rcdf			opts1.bit.rcdf
#define rx_fae			opts1.bit.fae
#define rx_m_extsize	opts1.bit.m_extsize
#define rx_len			opts2.bit.len
#define rx_qid			opts2.bit.qid
#define rx_dp_ext		opts2.bit.dp_ext
#define rx_extspa		opts2.bit.extspa
#define rx_reason		opts3.bit.reason
#define rx_linkid		opts3.bit.linkid
#define rx_ppp_idx		opts3.bit.ppp_idx
#define rx_po			opts3.bit.po
#define rx_lo			opts3.bit.lo
#define rx_vo			opts3.bit.vo
#define rx_type			opts3.bit.type
#define rx_tos			opts4.bit.tos
#define rx_ipv4			opts4.bit.ipv4
#define rx_ipv6			opts4.bit.ipv6
#define rx_ipv4_1st		opts4.bit.ipv4_1st
#define rx_frag			opts4.bit.frag
#define rx_last_f		opts4.bit.last_f
#define rx_spa			opts4.bit.spa
#define rx_dvlanid		opts4.bit.dvlanid
#define rx_l2act		opts4.bit.l2act
#define rx_ext_vlano	opts4.bit.ext_vlano
#define rx_svlanid		opts5.bit.svlanid
#define rx_ext_ttl		opts5.bit.ext_ttl
#define rx_spri			opts5.bit.spri
#define rx_dpri			opts5.bit.dpri
#define rx_porg			opts5.bit.porg
#define rx_lorg			opts5.bit.lorg
#define rx_vorg			opts5.bit.vorg
#define rx_ip_mdf		opts5.bit.ip_mdf
#define rx_org			opts5.bit.org
#define rx_fwd			opts5.bit.fwd
#define rx_l4csok		opts5.bit.l4csok
#define rx_l3csok		opts5.bit.l3csok

};


//ask alan :linked[6:0]	For WLAN WDS multiple tunnel.

struct tx_desc{
#ifdef _LITTLE_ENDIAN
	union{
		struct{
			uint32 own:1;         //0
			uint32 eor:1;         //1			
			uint32 ls:1;          //2
			uint32 fs:1;          //3
			uint32 hwlkup:1;      //4
			uint32 bridge:1;      //5
			uint32 ph_len:17;//6~22	
			uint32 pppidx:3;      //23~25	
			uint32 pi:1;          //26
			uint32 li:1;          //27	
			uint32 vi:1;          //28
			uint32 type:3;        //29~31					
		}bit;
		uint32 dw;//double word
	}opts1;
	uint32 mdata;
	union{
		struct{
			uint32 vlantag:9;     //0~8
			uint32 pqid:3;        //9~11
			uint32 qid:3;         //12~14
			uint32 mlen:17;       //15~31			
		}bit;
		uint32 dw;//double word
	}opts2;
	union{	
		struct{
			uint32 dvlanid:12;     //0~11
			uint32 dp_ext:3;       //12~14
			uint32 rsvd:1;         //15
			uint32 ipv4_1st:1;     //16
			uint32 ipv4:1;         //17			
			uint32 ipv6:1;         //18
			uint32 l4cs:1;         //19
			uint32 l3cs:1;         //20
			uint32 po:1;           //21
			uint32 dpri:3;         //22~24
			uint32 ptp_ver:2;      //25~26
			uint32 ptp_type:4;     //27~30			
			uint32 ptp_pkt:1;      //31						
		}bit;
		uint32 dw;//double word
	}opts3;
	union{
		struct{
			uint32 ipv6_hdrlen:16; //0~15
			uint32 linkid:7;       //16~22
			uint32 rsvd:1;          //23
			uint32 dp:7;           //24~30
			uint32 lso:1;         //31						
		}bit;
		uint32 dw;
	}opts4;
	union{
		struct{
			uint32 tcp_hdrlen:4;   //0~3
			uint32 ipv4_hdrlen:4;  //4~7
			uint32 flags:8;        //8~15
			uint32 mss:14;         //16~29
			uint32 extspa:2;       //30~31						
		}bit;
		uint32 dw;
	}opts5;
#else
	union{
		struct{
			uint32 type:3;        //29~31			
			uint32 vi:1;          //28
			uint32 li:1;          //27	
			uint32 pi:1;          //26
			uint32 pppidx:3;      //23~25	
			uint32 ph_len:17;//6~22	
			uint32 bridge:1;      //5
			uint32 hwlkup:1;      //4
			uint32 fs:1;          //3
			uint32 ls:1;          //2
			uint32 eor:1;         //1			
			uint32 own:1;         //0
		}bit;
		uint32 dw;//double word
	}opts1;
	uint32 mdata;
	union{
		struct{
			uint32 mlen:17;       //15~31
			uint32 qid:3;         //12~14
			uint32 pqid:3;        //9~11
			uint32 vlantag:9;     //0~8
		}bit;
		uint32 dw;//double word
	}opts2;
	union{	
		struct{
			uint32 ptp_pkt:1;      //31			
			uint32 ptp_type:4;     //27~30			
			uint32 ptp_ver:2;      //25~26
			uint32 dpri:3;         //22~24
			uint32 po:1;           //21
			uint32 l3cs:1;         //20
			uint32 l4cs:1;         //19
			uint32 ipv6:1;         //18
			uint32 ipv4:1;         //17			
			uint32 ipv4_1st:1;     //16
			uint32 rsvd:1;         //15
			uint32 dp_ext:3;       //12~14
			uint32 dvlanid:12;     //0~11
		}bit;
		uint32 dw;//double word
	}opts3;
	union{
		struct{
			uint32 lso:1;         //31			
			uint32 dp:7;           //24~30
			uint32 rsvd:1;          //23
			uint32 linkid:7;       //16~22
			uint32 ipv6_hdrlen:16; //0~15
		}bit;
		uint32 dw;
	}opts4;
	union{
		struct{
			uint32 extspa:2;       //30~31			
			uint32 mss:14;         //16~29
			uint32 flags:8;        //8~15
			uint32 ipv4_hdrlen:4;  //4~7
			uint32 tcp_hdrlen:4;   //0~3
		}bit;
		uint32 dw;
	}opts5;
#endif

#define tx_own			opts1.bit.own
#define tx_eor			opts1.bit.eor
#define tx_ls			opts1.bit.ls
#define tx_fs			opts1.bit.fs 
#define tx_hwlkup		opts1.bit.hwlkup
#define tx_bridge		opts1.bit.bridge
#define tx_ph_len		opts1.bit.ph_len
#define tx_pppidx		opts1.bit.pppidx
#define tx_pi			opts1.bit.pi
#define tx_li			opts1.bit.li
#define tx_vi			opts1.bit.vi
#define tx_type			opts1.bit.type
#define tx_vlantag		opts2.bit.vlantag
#define tx_pqid			opts2.bit.pqid
#define tx_qid			opts2.bit.qid
#define tx_mlen			opts2.bit.mlen
#define tx_dvlanid		opts3.bit.dvlanid
#define tx_dp_ext		opts3.bit.dp_ext
#define tx_ipv4_1st		opts3.bit.ipv4_1st 
#define tx_ipv4			opts3.bit.ipv4
#define tx_ipv6			opts3.bit.ipv6
#define tx_l4cs			opts3.bit.l4cs
#define tx_l3cs			opts3.bit.l3cs
#define tx_po			opts3.bit.po
#define tx_dpri			opts3.bit.dpri
#define tx_ptp_ver		opts3.bit.ptp_ver
#define tx_ptp_type		opts3.bit.ptp_type
#define tx_ptp_pkt		opts3.bit.ptp_pkt 
#define tx_ipv6_hdrlen	opts4.bit.ipv6_hdrlen
#define tx_linkid		opts4.bit.linkid
#define tx_dp			opts4.bit.dp
#define tx_lso			opts4.bit.lso
#define tx_tcp_hdrlen	opts5.bit.tcp_hdrlen
#define tx_ipv4_hdrlen	opts5.bit.ipv4_hdrlen 
#define tx_flags_		opts5.bit.flags
#define tx_mss			opts5.bit.mss  
#define tx_extspa		opts5.bit.extspa

};

typedef struct dma_tx_desc {
	uint32		opts1;
	uint32		addr;
	uint32		opts2;
	uint32		opts3;
	uint32		opts4;
	uint32		opts5;
}DMA_TX_DESC;

typedef struct dma_rx_desc {
	uint32		opts1;
	uint32		addr;
	uint32		opts2;
	uint32		opts3;	
	uint32		opts4;
	uint32		opts5;
}DMA_RX_DESC;

#define IS_SWITCH_NORMAL_DESC() ((REG32(CPUICR1) & CF_PKT_HDR_TYPE_MASK) == TX_PKTHDR_8198C_DEF)

int32 New_swNic_init(uint32 userNeedRxPkthdrRingCnt[RTL865X_SWNIC_RXRING_HW_PKTDESC],
                 uint32 userNeedTxPkthdrRingCnt[RTL865X_SWNIC_TXRING_HW_PKTDESC],
                 uint32 clusterSize);

int32 New_swNic_send(void *skb, void * output, uint32 len,rtl_nicTx_info *nicTx);
int32 New_swNic_txDone(int idx);
void New_swNic_freeRxBuf(void);
void New_dumpTxRing(struct seq_file *s);
void New_dumpRxRing(struct seq_file *s);

extern uint32  size_of_cluster;

#endif

