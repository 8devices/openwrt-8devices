
#ifndef RTL865X_FDB_API_H
#define RTL865X_FDB_API_H

#define RTL_LAN_FID								0
#if defined (CONFIG_RTL_IVL_SUPPORT)	
#define RTL_WAN_FID								1
#else
#define RTL_WAN_FID								0
#endif

#define FDB_STATIC						0x01		/* flag for FDB: process static entry only */
#define FDB_DYNAMIC						0x02		/* flag for FDB: process dynamic entry only */

#define RTL865x_FDB_NUMBER				4
#define RTL865x_L2_TYPEI			0x0001		/* Referenced by ARP/PPPoE */
#define RTL865x_L2_TYPEII			0x0002		/* Referenced by Protocol */
#define RTL865x_L2_TYPEIII			0x0004		/* Referenced by PCI/Extension Port */
#define CONFIG_RTL865X_SYNC_L2			1
#define RTL865X_FDBENTRY_TIMEOUT		0x1001		/*fdb entry time out*/
#define RTL865X_FDBENTRY_450SEC		0x1002		/*fdb entry 450s timing*/
#define RTL865X_FDBENTRY_300SEC		0x1004		/*fdb entry 300s timing*/
#define RTL865X_FDBENTRY_150SEC		0x1008		/*fdb entry 150s timing*/

/*#define ETHER_ADDR_LEN				6
typedef struct ether_addr_s {
	uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;*/
/*
typedef struct rtl865x_tblAsicDrv_l2Param_s {
	ether_addr_t	macAddr;
	uint32 		memberPortMask; //extension ports [rtl8651_totalExtPortNum-1:0] are located at bits [RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum-1:RTL8651_PORT_NUMBER]
	uint32 		ageSec;
	uint32	 	cpu:1,
				srcBlk:1,
				isStatic:1,				
				nhFlag:1,
				fid:2,
				auth:1;

} rtl865x_tblAsicDrv_l2Param_t;*/

//extern typedef struct rtl865x_tblAsicDrv_l2Param_s	rtl865x_tblAsicDrv_l2Param_t;



void update_hw_l2table(const char *srcName,const unsigned char *addr);
int32 rtl_get_hw_fdb_age(uint32 fid,ether_addr_t *mac, uint32 flags);
int32 rtl865x_addAuthFDBEntry(const unsigned char *addr, int32 auth, int32  port, int32 srcblk);
int32 rtl865x_setRestrictPortNum(int32 port, uint8 isEnable, int32 number);
int32 rtl865x_check_authfdbentry_Byport(int32 port_num, const unsigned char  *macAddr);
int32 rtl865x_enableLanPortNumRestrict(uint8 isEnable);
int32 rtl865x_delAuthLanFDBEntry(uint16 l2Type,  const unsigned char *addr);
int32 rtl865x_delLanFDBEntry(uint16 l2Type,  const unsigned char *addr);

int32 rtl865x_arrangeFdbEntry(const unsigned char *timeout_addr, int32 *port);
//extern int32 rtl865x_Lookup_fdb_entry(uint32 fid, ether_addr_t *mac, uint32 flags, uint32 *col_num, rtl865x_tblAsicDrv_l2Param_t *L2buff);

int32 rtl865x_getPortNum(const unsigned char *addr);
int32 rtl865x_ConvertPortMasktoPortNum(int32 portmask);
int32 rtl865x_addFDBEntry(const unsigned char *addr);
int32 rtl865x_isEthMac(void  *mac);

#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
int32 rtl865x_addExtFDBEntry(const unsigned char *addr,int extPort);
int32 rtl865x_delExtFDBEntry(void);

#endif

#if defined(CONFIG_RTL865X_LANPORT_RESTRICTION)
#define	LAN_RESTRICT_PORT_NUMBER		9
#define RTL_LAN_RESTRICT_STAT2	2
#define RTL_LAN_RESTRICT_STAT1	1
#define RTL_LAN_RESTRICT_STAT0	0

typedef struct _lan_restrict_info
{
	int16 		port_num;
	int16		enable;
	int32	max_num;
	int32	curr_num;
}lan_restrict_info;

extern lan_restrict_info	lan_restrict_tbl[LAN_RESTRICT_PORT_NUMBER];


//int32 rtl865x_addAuthFDBEntry_hooks(struct net_bridge_fdb_entry *fdb,const unsigned char *addr);


int32 lan_restrict_getBlockAddr(int32 port , const unsigned char *swap_addr);
int32 lan_restrict_CheckStatusByport(int32 port);





int32 rtl_check_fdb_entry_check_exist(uint32 fid, ether_addr_t *mac, uint32 flags);
int32 rtl_check_fdb_entry_check_srcBlock(uint32 fid, ether_addr_t *mac, int32 *SrcBlk);
int32 lanrestrict_callbackFn_for_add_fdb(void *param);
int32 lanrestrict_callbackFn_for_del_fdb(void *param);
int32 lanrestrict_unRegister_event(void);
int32 lanrestrict_register_event(void);
#endif	/*	defined(CONFIG_RTL865X_LANPORT_RESTRICTION)	*/

#if defined(CONFIG_RTL_HARDWARE_NAT) //&&defined(REINIT_SWITCH_CORE)
void rtl819x_init_l2_bakup_entry(void);
int32 rtl819x_save_inused_l2_entry(ipaddr_t ip, ether_addr_t *mac, uint32 fid);
int rtl819x_restore_hw_l2_table(void);
#endif

#endif
