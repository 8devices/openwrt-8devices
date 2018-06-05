#define seconds 4
#define CONN_DEV_NUM 512
#define RTL_DEV_STATS_TIMER_EXPIRE (seconds*HZ)
enum packet_dir{
    PACKET_DIR_ORIGINAL,
    PACKET_DIR_REPLY,
    PACKET_DIR_MAX
};

struct connected_dev{
    struct list_head list;
    unsigned int dev_ip;
    unsigned char mac[6];
    void *dev_stats;
    unsigned int used;
    unsigned int last_jiffies;
};

struct dev_stats{
    struct list_head list;
    unsigned int up_bps_speed;
    unsigned int up_pps_speed;
    unsigned int down_bps_speed;
    unsigned int down_pps_speed;

    unsigned int last_upbytes;
    unsigned int last_uppackets;
    unsigned int last_downbytes;
    unsigned int last_downpackets;

    unsigned int upbytes;
    unsigned int uppackets;
    unsigned int downbytes;
    unsigned int downpackets;
    void *conn_dev;
};
extern void rtl_update_dev_stats_hook(struct nf_conn *ct,struct sk_buff *skb,enum packet_dir packet_dir_t);
extern void rtl_build_con_dev_stats_hook(struct nf_conn *ct, struct sk_buff *skb);
extern void rtl_dec_con_dev_ref_hook(struct nf_conn *ct);
