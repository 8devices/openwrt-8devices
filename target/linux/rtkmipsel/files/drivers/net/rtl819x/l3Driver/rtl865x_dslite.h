#ifndef RTL865X_DSLITE_H
#define RTL865X_DSLITE_H


typedef struct rtl865x_dslite_s {
	inv6_addr_t host_ipv6_addr;
	uint32      host_ipv6_mask;
	inv6_addr_t aftr_ipv6_addr;
    uint32      aftr_ipv6_mask;
	char name[MAX_IFNAMESIZE];	/* name of tunnel device */
	uint32      mtu;
	uint32      valid;
	uint8		index;
	uint8		arpIdx;
	inv6_addr_t	gw_ipv6_addr;
} rtl865x_dslite_s;

//int32 rtl865x_updatev6SwNetif(char *netifName,int iftype);
int32 rtl8198c_initIpv6DsLiteTable(void);
rtl865x_dslite_s *  rtl865x_getDsLiteEntry(void);
int32  _rtl865x_getIpv6DsLiteEntryByName(char * devName);
rtl865x_dslite_s *  _rtl865x_getIpv6DsLiteEntryByIndex(int32 entryIdx);
int32  _rtl865x_getIpv6DsLiteEntryByAfterAddr(inv6_addr_t aftr_ipv6_addr,uint32 aftr_ipv6_mask);
int32  _rtl865x_getIpv6DsLiteEntrybyIndex(uint32 entryIdx,rtl865x_dslite_s *entry);

#endif
