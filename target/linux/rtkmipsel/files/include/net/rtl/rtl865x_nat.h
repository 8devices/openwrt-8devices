#ifndef	RTL865X_NAT_H
#define	RTL865X_NAT_H
#define CONFIG_RTL865X_NAT_ADD_FAIL_CHECK	1
#define FLAG_QOS_ENABLE 1

/* NAT timeout value */
#define TCP_TIMEOUT					120	 	/* 120 secs */
#define UDP_TIMEOUT					90		/* 90 secs */
#define TCP_OVERRIDE_ELASPED_THRESHOLD	60	 	/* 60 secs */
#define UDP_OVERRIDE_ELASPED_THRESHOLD	30		/* 30 secs */

#define TCP_CLOSED_FLOW				8

#define RTL865X_PROTOCOL_UDP		0
#define RTL865X_PROTOCOL_TCP		1

#define NAT_INBOUND					0x00000001
#define NAT_OUTBOUND				0x00000002
#define NAT_PRI_PROCESSED			0x00000004
#define NAT_PRI_HALF_PROCESSED		0x00000008

#define NAT_PRE_RESERVED		0x00000100
#define RESERVE_EXPIRE_TIME	3	/*uinit:seconds*/

#define NAT_INUSE(_n_)				( ((_n_)->flags&(NAT_INBOUND|NAT_OUTBOUND)) )
#define SET_NAT_FLAGS(_n_, _v_)		((_n_)->flags |= (_v_))
#define CLR_NAT_FLAGS(_n_, _v_)		((_n_)->flags &= (~(_v_)))
#define	NAT_INVALID(_n_)			( ((_n_)->flags=0) )

#define MAX_EXTPORT_TRY_CNT 8

#define	RTL_NAPT_ACCELERATION_FAIL			-1
#define	RTL_NAPT_FULL_ACCELERATION			0
#define	RTL_NAPT_OUTBOUND_ACCELERATION 	1
#define 	RTL_NAPT_INBOUND_ACCELERATION		2

#define CONFIG_RTL_INBOUND_COLLISION_AVOIDANCE
#define CONFIG_RTL_HALF_NAPT_ACCELERATION

#define RTL_REFRESH_HW_L2_ENTRY_DECIDE_BY_HW_NAT	1

typedef struct _rtl865x_napt_entry
{
	uint32 protocol;	//RTL865X_PROTOCOL_UDP or RTL865X_PROTOCOL_TCP
	ipaddr_t intIp;
	uint32 intPort;
	ipaddr_t extIp;
	uint32 extPort;
	ipaddr_t remIp;
	uint32 remPort;
#if defined (CONFIG_RTL_EXT_PORT_SUPPORT)
	uint32 isExtNaptEntry;
#endif
}rtl865x_napt_entry;

typedef struct _rtl865x_priority
{
	int32 uplinkPrio;
	int32 downlinkPrio;
}rtl865x_priority;

typedef struct _rtl865x_qos_mark
{
	int32 uplinkMark;
	int32 downlinkMark;
}rtl865x_qos_mark;

int32 rtl865x_nat_init(void);
int32 rtl865x_nat_reinit(void);
/*
@func enum RTL_RESULT | rtl865x_addNaptConnection | Add a NAPT Flow
@parm enum RTL_NP_PROTOCOL | protocol | The protocol to add
@parm ipaddr_t | intIp | Internal IP address
@parm uint32 | intPort | Internal Port
@parm ipaddr_t | extIp | External IP address
@parm uint32 | extPort | External Port
@parm ipaddr_t | remIp | Remote IP address
@parm uint32 | remPort | Remote Port
@parm enum RTL_NP_FLAGS | flags | reserved for future used
@rvalue RTL_SUCCESS | Add success (can be ASIC-accelerated)
@rvalue RTL_SUCCESS | Add success (cannot be ASIC-accelerated)
@rvalue RTL_ERROR_PARAMETER | Error parameter is given
@rvalue RTL_EXIST | Add an existed flow
@rvalue RTL_FAILED | General failure
@comm 
	Add a NAPT Flow Entry to L4 TCP/UDP NAPT Table(1024-Entry)
@devnote
	Insert into ip_nat_setup_info() function in file net/ipv4/netfilter/ip_nat_core.c
*/
int32 rtl865x_addNaptConnection(rtl865x_napt_entry *naptEntry, rtl865x_priority *prio);
/*
@func enum RTL_RESULT | rtl865x_delNaptConnection | Delete a NAPT Flow
@parm enum RTL_NP_PROTOCOL | protocol | The protocol to delete
@parm ipaddr_t | intIp | Internal IP address
@parm uint32 | intPort | Internal Port
@parm ipaddr_t | extIp | External IP address
@parm uint32 | extPort | External Port
@parm ipaddr_t | remIp | Remote IP address
@parm uint32 | remPort | Remote Port
@rvalue RTL_SUCCESS | Delete success
@rvalue RTL_NONEXIST | Delete a non-existed flow
@rvalue RTL_FAILED | General failure
@comm 
	Delete a NAPT Flow Entry of L4 TCP/UDP NAPT Table(1024-Entry)
@devnote
	Insert into ip_nat_cleanup_conntrack() function in file net/ipv4/netfilter/ip_nat_core.c
*/
int32 rtl865x_delNaptConnection(rtl865x_napt_entry *naptEntry);

int32 rtl865x_naptSync(rtl865x_napt_entry *naptEntry, uint32 refresh);

#if defined (CONFIG_RTL_INBOUND_COLLISION_AVOIDANCE)
int rtl865x_optimizeExtPort(unsigned short origDelta, unsigned int rangeSize, unsigned short *newDelta);

int rtl865x_getAsicNaptHashScore(rtl865x_napt_entry *naptEntry, 
					                        uint32 *naptHashScore);

int32 rtl865x_preReserveConn(rtl865x_napt_entry *naptEntry);

#endif
#if defined(CONFIG_RTL_HARDWARE_NAT)
int32 rtl_nat_expire_interval_update(int proto, int32 interval);
int32 rtl865x_nat_reinit(void);
#endif

#if defined(RTL_REFRESH_HW_L2_ENTRY_DECIDE_BY_HW_NAT)
int rtl865x_check_hw_nat_by_ip(ipaddr_t ip);
#endif
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
int32 rtl865x_flush_ext_port_entry(void);

#endif
#endif

