#ifndef RTL865X_6RD_H
#define RTL865X_6RD_H

typedef struct rtl865x_6rd_s {
	ipaddr_t    ce_ip_addr;
	uint8       ce_ip_mask_len;/*LSB*/
	inv6_addr_t six_rd_prefix;
	uint8       six_rd_prefix_len;
	ipaddr_t    br_ip_addr;
	uint8       br_ip_mask_len;/*MSB*/
	uint16      mtu;
	uint8       valid;
	char 	name[MAX_IFNAMESIZE];
	uint8	index;
} rtl865x_6rd_s;

int32 rtl8198c_initIpv66RDTable(void);
int32 rtl8198c_reinitIpv66RDTable(void);
rtl865x_6rd_s * rtl865x_get6RDEntry(void);
rtl865x_6rd_s * _rtl865x_getIpv66RDEntryByIndex(int32 entryIdx);
int32 _rtl865x_getIpv66RDEntryByName(char * devName);
int32 _rtl865x_getIpv66RDEntrybyIndex(uint32 entryIdx,rtl865x_6rd_s *entry);
int32 _rtl865x_addIpv6DsLiteEntry(struct ip_tunnel *t);
int32 _rtl865x_addIpv66RDEntryByBR(char *devname);
int32 _rtl865x_delIpv66RDEntry(uint32 entryIdx);

#endif
