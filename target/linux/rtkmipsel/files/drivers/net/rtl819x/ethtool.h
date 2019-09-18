#ifndef RTL819X_ETHTOOL
#define RTL819X_ETHTOOL

#include <linux/ethtool.h>

struct rtl_switch_port_mapping
{
	char ifname[IFNAMSIZ];
	int8_t port_num;
};

extern const struct ethtool_ops rtl865x_ethtool_ops;
#endif
