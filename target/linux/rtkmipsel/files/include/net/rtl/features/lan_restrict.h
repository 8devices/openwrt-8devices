//#include <common/rtl865x_common.h>
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_queue.h>
#include <net/rtl/rtl_nic.h>
#include <net/rtl/rtl865x_fdb_api.h>

int32 lan_restrict_getBlockAddr(int32 port , const unsigned char *swap_addr);
extern int __init lan_restrict_init(void);	
extern int	 lan_restrict_rcv(struct sk_buff *skb, struct net_device *dev);
extern int32 lan_restrict_CheckStatusByport(int32 port);
