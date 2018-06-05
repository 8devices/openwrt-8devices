/*
* Program : Protocol Header rtl8651_proto.h
* Abstract : 
* Author : Chih-Hua Huang (chhuang@realtek.com.tw)               
* $Id: rtl8651_tblDrvProto.h,v 1.1 2007-12-21 10:29:52 davidhsu Exp $
*
*  Copyright (c) 2011 Realtek Semiconductor Corp.
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.

*/

#ifndef RTL8651_TBLDRV_PROTO_H
#define RTL8651_TBLDRV_PROTO_H

#include <linux/in.h>
#include <net/rtl/rtl_types.h>

/*==========================================================================================
 * Ethernet Header for MAC ACL lookup 
 *==========================================================================================*/
#define ETHER_ADDR_LEN				6
struct	ether_header {
	uint8	ether_dhost[ETHER_ADDR_LEN];
	uint8	ether_shost[ETHER_ADDR_LEN];
	uint16	ether_type;
};

/* The number of bytes in the type field. */
#define	ETHER_TYPE_LEN		2

/* The length of the combined header. */
#define	ETHER_HDR_LEN		(ETHER_ADDR_LEN*2+ETHER_TYPE_LEN)
#define	ETHER_VLAN_HDR_LEN	(ETHER_HDR_LEN+4)


#ifndef UTILITY_H /* pktproc/utility.h also defined this structure. */
struct  ether_addr {
        uint8 octet[ETHER_ADDR_LEN];
};
#endif

typedef struct pppoeHdr_s {
  #ifdef _LITTLE_ENDIAN
	uint8	type:4, ver:4;
  #else
	uint8	ver:4, type:4;
  #endif
	uint8	code;
	uint16	sessionId, 
			length; //Length of the PPPoE payload, does not include Ethernet and PPPoE header
	uint16	proto;	//PPP protocol field
} pppoeHdr_t;

typedef struct vlanHdr_s {
#ifdef _LITTLE_ENDIAN
	uint16	vidh:4;
	uint16	cfi:1;
	uint16	priority:3;
	uint16	vidl:8;
#else
	uint16	priority:3;
	uint16	cfi:1;
	uint16	vidh:4;
	uint16	vidl:8;
#endif
	uint16	ether_type;
} vlanHdr_t;


/*==========================================================================================
 * IP Header for IP ACL lookup 
 *==========================================================================================*/
//#ifndef UTILITY_H /* pktproc/utility.h also defined this structure. */
#if !defined(_LINUX_IN_H)
struct in_addr{
	uint32    s_addr;
};

/* AF_INET Supported IP Protocols*/
#define IPPROTO_ICMP 1
#define IPPROTO_IGMP 2
#define IPPROTO_TCP	6
#define IPPROTO_UDP	17
#endif

struct ip {

#if 0 
union{
   uint8 _vhl;   /* version << 4 | header length >> 2 */
#ifdef _LITTLE_ENDIAN
	struct {
	  uint8  _hl:4,  /* header length */
	   _ver:4;	/* version */
	}s;
#else
	struct {
	  uint8 _ver:4,  /* version */
	  _hl:4;/* header length */
	}s;
#endif
 } vhl;

#define ip_vhl vhl._vhl
#define ip_hl  vhl.s._hl
#define ip_ver vhl.s._ver
#else
	/* replace bit field */
	uint8 ip_vhl;
#endif 

	uint8	ip_tos;			/* type of service */
	uint16	ip_len;			/* total length */
	uint16	ip_id;			/* identification */
	uint16	ip_off;			/* fragment offset field */
	uint8	ip_ttl;			/* time to live */
	uint8	ip_p;			/* protocol */
	uint16	ip_sum;			/* checksum */
	struct	in_addr ip_src,ip_dst;	/* source and dest address */
};
typedef struct ip ip_t;

#define	IP_RF 0x8000			/* reserved fragment flag */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */



/*==========================================================================================
 * IP Header for IP ACL lookup 
 *==========================================================================================*/
struct icmp_ra_addr {
	uint32 ira_addr;
	uint32 ira_preference;
};

struct icmp {
	uint8	icmp_type;		/* type of message, see below */
	uint8	icmp_code;		/* type sub code */
	uint16	icmp_cksum;		/* ones complement cksum of struct */
	union {
		uint8 ih_pptr;			/* ICMP_PARAMPROB */
		struct in_addr ih_gwaddr;	/* ICMP_REDIRECT */
		struct ih_idseq {
			uint16	icd_id;
			uint16	icd_seq;
		} ih_idseq;
		uint32 ih_void;

		/* ICMP_UNREACH_NEEDFRAG -- Path MTU Discovery (RFC1191) */
		struct ih_pmtu {
			uint16 ipm_void;
			uint16 ipm_nextmtu;
		} ih_pmtu;

		struct ih_rtradv {
			uint8 irt_num_addrs;
			uint8 irt_wpa;
			uint16 irt_lifetime;
		} ih_rtradv;
	} icmp_hun;
#define	icmp_pptr	icmp_hun.ih_pptr
#define	icmp_gwaddr	icmp_hun.ih_gwaddr
#define	icmp_id		icmp_hun.ih_idseq.icd_id
#define	icmp_seq	icmp_hun.ih_idseq.icd_seq
#define	icmp_void	icmp_hun.ih_void
#define	icmp_pmvoid	icmp_hun.ih_pmtu.ipm_void
#define	icmp_nextmtu	icmp_hun.ih_pmtu.ipm_nextmtu
#define	icmp_num_addrs	icmp_hun.ih_rtradv.irt_num_addrs
#define	icmp_wpa	icmp_hun.ih_rtradv.irt_wpa
#define	icmp_lifetime	icmp_hun.ih_rtradv.irt_lifetime
	union {
		struct id_ts {
			uint32 its_otime;
			uint32 its_rtime;
			uint32 its_ttime;
		} id_ts;
		struct id_ip  {
			struct ip idi_ip;
			/* options and then 64 bits of data */
		} id_ip;
		struct icmp_ra_addr id_radv;
		uint32 id_mask;
		int8	id_data[1];
	} icmp_dun;
#define	icmp_otime	icmp_dun.id_ts.its_otime
#define	icmp_rtime	icmp_dun.id_ts.its_rtime
#define	icmp_ttime	icmp_dun.id_ts.its_ttime
#define	icmp_ip		icmp_dun.id_ip.idi_ip
#define	icmp_radv	icmp_dun.id_radv
#define	icmp_mask	icmp_dun.id_mask
#define	icmp_data	icmp_dun.id_data
};


/*
 * Definition of type and code field values.
 */
#define	ICMP_ECHOREPLY		0		/* echo reply */
#define	ICMP_UNREACH		3		/* dest unreachable, codes: */
#define		ICMP_UNREACH_NET	0		/* bad net */
#define		ICMP_UNREACH_HOST	1		/* bad host */
#define		ICMP_UNREACH_PROTOCOL	2		/* bad protocol */
#define		ICMP_UNREACH_PORT	3		/* bad port */
#define		ICMP_UNREACH_NEEDFRAG	4		/* IP_DF caused drop */
#define		ICMP_UNREACH_SRCFAIL	5		/* src route failed */
#define		ICMP_UNREACH_NET_UNKNOWN 6		/* unknown net */
#define		ICMP_UNREACH_HOST_UNKNOWN 7		/* unknown host */
#define		ICMP_UNREACH_ISOLATED	8		/* src host isolated */
#define		ICMP_UNREACH_NET_PROHIB	9		/* prohibited access */
#define		ICMP_UNREACH_HOST_PROHIB 10		/* ditto */
#define		ICMP_UNREACH_TOSNET	11		/* bad tos for net */
#define		ICMP_UNREACH_TOSHOST	12		/* bad tos for host */
#define		ICMP_UNREACH_FILTER_PROHIB 13		/* admin prohib */
#define		ICMP_UNREACH_HOST_PRECEDENCE 14		/* host prec vio. */
#define		ICMP_UNREACH_PRECEDENCE_CUTOFF 15	/* prec cutoff */
#define	ICMP_SOURCEQUENCH	4		/* packet lost, slow down */
#define	ICMP_REDIRECT		5		/* shorter route, codes: */
#define		ICMP_REDIRECT_NET	0		/* for network */
#define		ICMP_REDIRECT_HOST	1		/* for host */
#define		ICMP_REDIRECT_TOSNET	2		/* for tos and net */
#define		ICMP_REDIRECT_TOSHOST	3		/* for tos and host */
#define	ICMP_ECHO		8		/* echo service */
#define	ICMP_ROUTERADVERT	9		/* router advertisement */
#define	ICMP_ROUTERSOLICIT	10		/* router solicitation */
#define	ICMP_TIMXCEED		11		/* time exceeded, code: */
#define		ICMP_TIMXCEED_INTRANS	0		/* ttl==0 in transit */
#define		ICMP_TIMXCEED_REASS	1		/* ttl==0 in reass */
#define	ICMP_PARAMPROB		12		/* ip header bad */
#define		ICMP_PARAMPROB_ERRATPTR 0		/* error at param ptr */
#define		ICMP_PARAMPROB_OPTABSENT 1		/* req. opt. absent */
#define		ICMP_PARAMPROB_LENGTH 2			/* bad length */
#define	ICMP_TSTAMP		13		/* timestamp request */
#define	ICMP_TSTAMPREPLY	14		/* timestamp reply */
#define	ICMP_IREQ		15		/* information request */
#define	ICMP_IREQREPLY		16		/* information reply */
#define	ICMP_MASKREQ		17		/* address mask request */
#define	ICMP_MASKREPLY		18		/* address mask reply */

#define	ICMP_MAXTYPE		18

/*==========================================================================================
 * IP Header for IP ACL lookup 
 *==========================================================================================*/
// igmp v3 group record
struct igmp_gr {
	uint8	igmp_gr_rt;				/* Record Type */
	uint8	igmp_gr_auxlen;			/* aux data length */
	uint16	igmp_gr_nofs;			/* number of sources */
	ipaddr_t	igmp_gr_group;			/* group address being reported */
	ipaddr_t	src_list;					/* first entry of src list */
	//	auxiliary data is unused now
 };
struct igmp {
	uint8		igmp_type;			/* version & type of IGMP message */
	uint8		igmp_code;			/* subtype for routing msgs */
	uint16		igmp_cksum;		/* IP-style checksum */

	union {
		struct in_addr	group;		/* group address being presented (v1/v2/v3 query) */
		struct v3_report{
			uint16			resv;	/* reserved */
			uint16			nofg;	/* number of group records */
		} v3_report;
	} grp;

	union {
		struct query {
			uint8	rsq;				/* 4bit: reserved, 1bit: suppress router-side processing, 3bit: querier's robustness variable*/
//			uint8	resv:4;			/* reserved */
//			uint8	s:1;				/* suppress router-side processing */
//			uint8	qrv:3;			/* querier's robustness variable */
			uint8	qqic;			/* querier's query interval code */
			uint16	nofs;			/* number of sources */
			ipaddr_t	src_list;			/* first entry of src list */
		} query;
		struct report {
			struct igmp_gr	gr_list;	/* first entry of group record */
		} report;
	} un_v3;

	#define	igmp_group		grp.group
	#define	igmp_qrsq		un_v3.query.rsq
//	#define	igmp_qresv		un_v3.query.resv
//	#define	igmp_qsflag		un_v3.query.s
//	#define	igmp_qrv		un_v3.query.qrv
	#define	igmp_qqic		un_v3.query.qqic
	#define	igmp_qnofs		un_v3.query.nofs
	#define	igmp_qsrclist	un_v3.query.src_list
	#define	igmp_rnofg		grp.v3_report.nofg
	#define	igmp_rresv		grp.v3_report.resv
	#define	igmp_grlist		un_v3.report.gr_list

};	

/* IGMP Type */
#define	IGMP_QUERY				0x11		/* igmp group membership query */
#define	IGMP_V1_REPORT		0x12		/* igmp v1 membership report */
#define	IGMP_DVMRP			0x13		/* DVMRP */
#define	IGMP_PIMV1				0x14		/* PIM v1 */
#define	IGMP_CISCOTRACE		0x15		/* CISCO trace messages */
#define	IGMP_V2_REPORT		0x16		/* igmp v2 membership report */
#define	IGMP_V2_LEAVE			0x17		/* igmp v2 leave group message */
#define	IGMP_MTRACE_RESPONSE	0x1e		/* multicast traceroute response */
#define	IGMP_MTRACE			0x1f		/* multicast traceroute */
#define	IGMP_V3_REPORT		0x22		/* igmp v3 membership report */
#define	IGMP_MROUTER_ADV		0x24		/* igmp multicast router advertisement */
#define	IGMP_MROUTER_SOL		0x25		/* igmp multicast router solicitation */
#define	IGMP_MROUTER_TERM	0x26		/* igmp multicast router termination */

/* IGMP v3 Group Record Type */
#define	IGMPV3_MODE_IS_INCLUDE			0x01
#define	IGMPV3_MODE_IS_EXCLUDE			0x02
#define	IGMPV3_CHANGE_TO_INCLUDE_MODE	0x03
#define	IGMPV3_CHANGE_TO_EXCLUDE_MODE	0x04
#define	IGMPV3_ALLOW_NEW_SOURCES		0x05
#define	IGMPV3_BLOCK_OLD_SOURCES		0x06

/*==========================================================================================
 * IP Header for IP ACL lookup 
 *==========================================================================================*/
typedef	uint32 tcp_seq;
struct tcphdr {
	uint16	th_sport;		/* source port */
	uint16	th_dport;		/* destination port */
	tcp_seq	th_seq;			/* sequence number */
	tcp_seq	th_ack;			/* acknowledgement number */
#if 0	
	#ifdef  _LITTLE_ENDIAN
		uint8	th_x2:4,		/* (unused) */
			th_off:4;			/* data offset */
	#else
		uint8	th_off:4,		/* data offset */
			th_x2:4;			/* (unused) */
	#endif
#else
	/* replace bit field */
	uint8  th_off_x;
	
#endif 

	uint8	th_flags;
		#define	TH_FIN	0x01
		#define	TH_SYN	0x02
		#define	TH_RST	0x04
		#define	TH_PUSH 0x08
		#define	TH_ACK	0x10
		#define	TH_URG	0x20
		#define	TH_ECE	0x40		
		#define	TH_CWR	0x80		
		#define	TH_FLAGS	(TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)

	uint16	th_win;			/* window */
	uint16	th_sum;			/* checksum */
	uint16	th_urp;			/* urgent pointer */

	//Optional TCP options. Max: 40 bytes.
	#define	TCPOPT_EOL			0
	#define	TCPOPT_NOP			1
	#define	TCPOPT_MAXSEG		2
	#define TCPOLEN_MAXSEG	4
	#define TCPOPT_WINDOW	3
	#define TCPOLEN_WINDOW	3
	#define TCPOPT_SACK_PERMITTED		4		/* Experimental */
	#define TCPOLEN_SACK_PERMITTED	2
	#define TCPOPT_SACK		5		/* Experimental */
	#define TCPOPT_TIMESTAMP	8
	#define TCPOLEN_TIMESTAMP		10
	#define TCPOLEN_TSTAMP_APPA	(TCPOLEN_TIMESTAMP+2) /* appendix A */
	#define TCPOPT_TSTAMP_HDR	\
	    (TCPOPT_NOP<<24|TCPOPT_NOP<<16|TCPOPT_TIMESTAMP<<8|TCPOLEN_TIMESTAMP)

	#define	TCPOPT_CC		11		/* CC options: RFC-1644 */
	#define TCPOPT_CCNEW	12
	#define TCPOPT_CCECHO	13
	#define TCPOLEN_CC		6
	#define	TCPOLEN_CC_APPA		(TCPOLEN_CC+2)
	#define	TCPOPT_CC_HDR(ccopt)		\
	    (TCPOPT_NOP<<24|TCPOPT_NOP<<16|(ccopt)<<8|TCPOLEN_CC)
};


/*==========================================================================================
 * IP Header for IP ACL lookup 
 *==========================================================================================*/
struct udphdr
{
	uint16    uh_sport;				   /* source port */
	uint16    uh_dport;				   /* destination port */
	uint16    uh_ulen;				   /* udp length */
	uint16    uh_sum;				   /* udp checksum */
};

/*==========================================================================================
 * TFTP header for RRQ/WRQ
 *==========================================================================================*/

struct tftpRequest {
	uint16	opcode;
	char 	*string;
};

#define TFTP_RRQ	0x01
#define TFTP_WRQ	0x02
#define TFTP_DATA	0x03
#define TFTP_ACK	0x04
#define TFTP_ERR	0x05

/*==========================================================================================
 * AH Header 
 *==========================================================================================*/
#define AH_AUTHLEN  	12              	/* authenticator length of 96bits */
#define AH_BASIC_LEN 8      			/* basic AH header is 8 bytes, nh,hl,rv,spi and the ah_hl, says how many bytes after that to cover. */
typedef struct ahHdr_s			/* Generic AH header */
{
	uint8	ah_nh;					/* Next header (protocol) */
	uint8	ah_hl;					/* AH length, in 32-bit words */
	uint16	ah_rv;				/* reserved, must be 0 */
	uint32	ah_spi;				/* Security Parameters Index */
        uint32   ah_rpl;                 		/* Replay prevention */
	uint8	ah_data[AH_AUTHLEN];		/* Authentication hash */
} ahHdr_t;

/*==========================================================================================
 * ESP Header 
 *==========================================================================================*/
typedef struct espHdr_s
{
	uint32 spi;
	uint32 seq;
} espHdr_t;

typedef struct cbcHdr64_s
{
	uint32 iv[2];
} cbcHdr64_t;

typedef struct cbcHdr128_s
{
	uint32 iv[4];
} cbcHdr128_t;


#if 0
#define ntohs(x)	(x)
#define ntohl(x)	(x)
#define htons(x)	(x)
#define htonl(x)	(x)

#define NTOHL(d)
#define NTOHS(d)
#define HTONL(d)
#define HTONS(d)
#endif

#if !defined(_LINUX_IN_H)
/*
 * Protocols (RFC 1700)
 */
#define	IPPROTO_IP		0		/* dummy for IP */
#define	IPPROTO_HOPOPTS		0		/* IP6 hop-by-hop options */
#define	IPPROTO_ICMP		1		/* control message protocol */
#define	IPPROTO_IGMP		2		/* group mgmt protocol */
#define	IPPROTO_GGP		3		/* gateway^2 (deprecated) */
#define IPPROTO_IPV4		4 		/* IPv4 encapsulation */
#define IPPROTO_IPIP		IPPROTO_IPV4	/* for compatibility */
#define	IPPROTO_TCP		6		/* tcp */
#define	IPPROTO_ST		7		/* Stream protocol II */
#define	IPPROTO_EGP		8		/* exterior gateway protocol */
#define	IPPROTO_PIGP		9		/* private interior gateway */
#define	IPPROTO_RCCMON		10		/* BBN RCC Monitoring */
#define	IPPROTO_NVPII		11		/* network voice protocol*/
#define	IPPROTO_PUP		12		/* pup */
#define	IPPROTO_ARGUS		13		/* Argus */
#define	IPPROTO_EMCON		14		/* EMCON */
#define	IPPROTO_XNET		15		/* Cross Net Debugger */
#define	IPPROTO_CHAOS		16		/* Chaos*/
#define	IPPROTO_UDP		17		/* user datagram protocol */
#define	IPPROTO_MUX		18		/* Multiplexing */
#define	IPPROTO_MEAS		19		/* DCN Measurement Subsystems */
#define	IPPROTO_HMP		20		/* Host Monitoring */
#define	IPPROTO_PRM		21		/* Packet Radio Measurement */
#define	IPPROTO_IDP		22		/* xns idp */
#define	IPPROTO_TRUNK1		23		/* Trunk-1 */
#define	IPPROTO_TRUNK2		24		/* Trunk-2 */
#define	IPPROTO_LEAF1		25		/* Leaf-1 */
#define	IPPROTO_LEAF2		26		/* Leaf-2 */
#define	IPPROTO_RDP		27		/* Reliable Data */
#define	IPPROTO_IRTP		28		/* Reliable Transaction */
#define	IPPROTO_TP		29 		/* tp-4 w/ class negotiation */
#define	IPPROTO_BLT		30		/* Bulk Data Transfer */
#define	IPPROTO_NSP		31		/* Network Services */
#define	IPPROTO_INP		32		/* Merit Internodal */
#define	IPPROTO_SEP		33		/* Sequential Exchange */
#define	IPPROTO_3PC		34		/* Third Party Connect */
#define	IPPROTO_IDPR		35		/* InterDomain Policy Routing */
#define	IPPROTO_XTP		36		/* XTP */
#define	IPPROTO_DDP		37		/* Datagram Delivery */
#define	IPPROTO_CMTP		38		/* Control Message Transport */
#define	IPPROTO_TPXX		39		/* TP++ Transport */
#define	IPPROTO_IL		40		/* IL transport protocol */
#define	IPPROTO_IPV6		41		/* IP6 header */
#define	IPPROTO_SDRP		42		/* Source Demand Routing */
#define	IPPROTO_ROUTING		43		/* IP6 routing header */
#define	IPPROTO_FRAGMENT	44		/* IP6 fragmentation header */
#define	IPPROTO_IDRP		45		/* InterDomain Routing*/
#define	IPPROTO_RSVP		46 		/* resource reservation */
#define	IPPROTO_GRE		47		/* General Routing Encap. */
#define	IPPROTO_MHRP		48		/* Mobile Host Routing */
#define	IPPROTO_BHA		49		/* BHA */
#define	IPPROTO_ESP		50		/* IP6 Encap Sec. Payload */
#define	IPPROTO_AH		51		/* IP6 Auth Header */
#define	IPPROTO_INLSP		52		/* Integ. Net Layer Security */
#define	IPPROTO_SWIPE		53		/* IP with encryption */
#define	IPPROTO_NHRP		54		/* Next Hop Resolution */
/* 55-57: Unassigned */
#define	IPPROTO_ICMPV6		58		/* ICMP6 */
#define	IPPROTO_NONE		59		/* IP6 no next header */
#define	IPPROTO_DSTOPTS		60		/* IP6 destination option */
#define	IPPROTO_AHIP		61		/* any host internal protocol */
#define	IPPROTO_CFTP		62		/* CFTP */
#define	IPPROTO_HELLO		63		/* "hello" routing protocol */
#define	IPPROTO_SATEXPAK	64		/* SATNET/Backroom EXPAK */
#define	IPPROTO_KRYPTOLAN	65		/* Kryptolan */
#define	IPPROTO_RVD		66		/* Remote Virtual Disk */
#define	IPPROTO_IPPC		67		/* Pluribus Packet Core */
#define	IPPROTO_ADFS		68		/* Any distributed FS */
#define	IPPROTO_SATMON		69		/* Satnet Monitoring */
#define	IPPROTO_VISA		70		/* VISA Protocol */
#define	IPPROTO_IPCV		71		/* Packet Core Utility */
#define	IPPROTO_CPNX		72		/* Comp. Prot. Net. Executive */
#define	IPPROTO_CPHB		73		/* Comp. Prot. HeartBeat */
#define	IPPROTO_WSN		74		/* Wang Span Network */
#define	IPPROTO_PVP		75		/* Packet Video Protocol */
#define	IPPROTO_BRSATMON	76		/* BackRoom SATNET Monitoring */
#define	IPPROTO_ND		77		/* Sun net disk proto (temp.) */
#define	IPPROTO_WBMON		78		/* WIDEBAND Monitoring */
#define	IPPROTO_WBEXPAK		79		/* WIDEBAND EXPAK */
#define	IPPROTO_EON		80		/* ISO cnlp */
#define	IPPROTO_VMTP		81		/* VMTP */
#define	IPPROTO_SVMTP		82		/* Secure VMTP */
#define	IPPROTO_VINES		83		/* Banyon VINES */
#define	IPPROTO_TTP		84		/* TTP */
#define	IPPROTO_IGP		85		/* NSFNET-IGP */
#define	IPPROTO_DGP		86		/* dissimilar gateway prot. */
#define	IPPROTO_TCF		87		/* TCF */
#define	IPPROTO_IGRP		88		/* Cisco/GXS IGRP */
#define	IPPROTO_OSPFIGP		89		/* OSPFIGP */
#define	IPPROTO_SRPC		90		/* Strite RPC protocol */
#define	IPPROTO_LARP		91		/* Locus Address Resoloution */
#define	IPPROTO_MTP		92		/* Multicast Transport */
#define	IPPROTO_AX25		93		/* AX.25 Frames */
#define	IPPROTO_IPEIP		94		/* IP encapsulated in IP */
#define	IPPROTO_MICP		95		/* Mobile Int.ing control */
#define	IPPROTO_SCCSP		96		/* Semaphore Comm. security */
#define	IPPROTO_ETHERIP		97		/* Ethernet IP encapsulation */
#define	IPPROTO_ENCAP		98		/* encapsulation header */
#define	IPPROTO_APES		99		/* any private encr. scheme */
#define	IPPROTO_GMTP		100		/* GMTP*/
#define	IPPROTO_IPCOMP		108		/* payload compression (IPComp) */
/* 101-254: Partly Unassigned */
#define	IPPROTO_PIM		103		/* Protocol Independent Mcast */
#define	IPPROTO_PGM		113		/* PGM */
/* 255: Reserved */
/* BSD Private, local use, namespace incursion */
#define	IPPROTO_RAW		255		/* raw IP packet */
#define	IPPROTO_MAX		256

/* last return value of *_input(), meaning "all job for this pkt is done".  */
#define	IPPROTO_DONE		257
#endif

/*
 * Ports < IPPORT_RESERVED are reserved for
 * privileged processes (e.g. root).         (IP_PORTRANGE_LOW)
 * Ports > IPPORT_USERRESERVED are reserved
 * for servers, not necessarily privileged.  (IP_PORTRANGE_DEFAULT)
 */
#define	IPPORT_RESERVED		1024
#define	IPPORT_USERRESERVED	5000

#endif /* RTL8651_TBLDRV_PROTO_H */

