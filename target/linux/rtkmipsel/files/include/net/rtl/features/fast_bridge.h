#if !defined(FAST_BRIDGE_H)
#define FAST_BRIDGE_H

#define	CONFIG_RTL_FASTBRIDGE		1

#define	RTL_FB_ENTRY_NUM				16
#define	RTL_FB_HASH_SIZE				16	/* must be 2s order */

#if !defined(__KERNEL__)
struct hlist_node {
	struct hlist_node *next, **pprev;
};
struct hlist_head {
	struct hlist_node *first;
};
#endif

typedef struct _rtl_fb_para
{
	uint32	enable_fb_filter:1;
	uint32	enable_fb_fwd:1;
	uint32	entry_num;
} rtl_fb_para;

typedef struct _rtl_fb_head
{
	struct hlist_head	in_used_list[RTL_FB_HASH_SIZE];
	struct hlist_head	free_list;
	uint32			used_cnt;
} rtl_fb_head;

typedef struct _rtl_fb_entry
{
	/* hlist MUST be the first filed */
	struct hlist_node	hlist;	/* belongs to the same hash entry */
	void*			dev_matchKey;
	int32			(*ndo_start_xmit) (struct sk_buff *skb,
						   struct net_device *dev);
	unsigned long		last_used;
	uint8			mac_addr[ETHER_ADDR_LEN];
} rtl_fb_entry;

int32	rtl_fb_process_in_nic(struct sk_buff *pskb, struct net_device *dev);
void		rtl_fb_del_entry(const uint8 *mac);
void		rtl_fb_flush(void);
void		rtl_fb_flush_by_dev(void* key);
unsigned long rtl_fb_get_entry_lastused(const uint8 *mac);

#define	RTL_FB_RETURN_SUCCESS	0
#define	RTL_FB_RETURN_FAILED		-1

/*****************************************************/
/*		netlink										*/
#define	FB_CMD_NO_CMD			-1
#define	FB_CMD_SET_FWD			0
#define	FB_CMD_SET_FILTER			1
#define	FB_CMD_SET_ENTRY_NUM	2
#define	FB_CMD_GET_STATUS		3
#define	FB_CMD_GET_USED_NUM		4
#define	FB_CMD_DUMP_ENTRYS		5

typedef struct _rtl_fb_nl_entry {
	uint8			name[IFNAMSIZ];
	uint8			mac_addr[ETHER_ADDR_LEN];
	unsigned long		last_used;
} rtl_fb_nl_entry;

typedef struct fb_data_info {
	rtl_fb_para		data;		/* status/flags/counter/etc */
	rtl_fb_nl_entry	*entry;		/* for get entry list */
	uint32			in_used;		/* inused count */
} fb_data_info_s, *fb_data_info_p;

typedef struct fb_cmd_info {
	int 				action;		/*	set/get items	*/
	fb_data_info_s	info;
}fb_cmd_info_s, *fb_cmd_info_p;

/*****************************************************/

#endif
