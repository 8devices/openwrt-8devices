#ifndef	RTL865X_NAT_LOCAL_H 
#define	RTL865X_NAT_LOCAL_H

struct nat_host_info {
	ipaddr_t						ip;
	uint16						port;
};

struct nat_tuple {
	struct nat_host_info			int_host;
	struct nat_host_info			ext_host;
	struct nat_host_info			rem_host;
	uint32						proto;
};


struct nat_entry {
	struct nat_tuple				tuple_info;

	uint32						natip_idx;
	uint32						in;
	uint32						out;
	uint32 						flags;
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	uint32 isExtNaptEntry;
#endif
#if defined (CONFIG_RTL_INBOUND_COLLISION_AVOIDANCE)
	uint32						reserveTime;
#endif

#define int_ip_					tuple_info.int_host.ip
#define int_port_					tuple_info.int_host.port
#define ext_ip_					tuple_info.ext_host.ip
#define ext_port_					tuple_info.ext_host.port
#define rem_ip_					tuple_info.rem_host.ip
#define rem_port_					tuple_info.rem_host.port
#define proto_					tuple_info.proto

};

struct nat_table {

	int32						connNum;		/* MUST equal or more than actually conntrack number */
	int32						freeEntryNum;
	int32						tcp_timeout;
	int32						udp_timeout;	
	struct nat_entry 				nat_bucket[RTL8651_TCPUDPTBL_SIZE];	
};

typedef struct rtl865x_naptHashInfo_s{
	 uint32 outIndex;
	 uint32 inIndex;
	 uint8 outCollision; 
	 uint8 inCollision;
	 uint8 sameFourWay;
	 uint8 sameLocation;
	 uint8  inFreeCnt;
}rtl865x_naptHashInfo_t;

#endif

