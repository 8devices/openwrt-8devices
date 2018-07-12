/*
 *      Handling routines for Mesh in 802.11 Proc
 *
 *      PS: All extern function in ../8190n_headers.h
 */
#define _MESH_PROC_C_

#ifdef CONFIG_RTL8192CD
#include "../8192cd.h"
#include "../8192cd_headers.h"
#else
#include "../8190n.h"
#include "../8190n_headers.h"
#endif
#include "./mesh.h"
#include "./mesh_route.h"
#ifdef MESH_USE_METRICOP
#include "mesh_11kv.h"
#ifdef __KERNEL__
#include <linux/init.h>
#include <asm/uaccess.h>
#endif
#endif

#if defined(_MESH_DEBUG_)
#ifdef __KERNEL__
#include <linux/init.h>
#include <asm/uaccess.h>
#endif
#include <linux/module.h>
#endif

#ifdef CONFIG_RTK_MESH

/*
 *	@brief	Printout assigned mesh MP neighbor table
 *		PS: Modify from dump__one_stainfo
 *
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
#ifdef CONFIG_RTL_PROC_NEW
static int dump_mesh_one_mpinfo(int num, struct stat_info *pstat, struct seq_file *s, void *data)
#else
static int dump_mesh_one_mpinfo(int num, struct stat_info *pstat, char *buf, char **start, off_t offset, int length, int *eof, void *data)
#endif
{
	int pos = 0,i = 0;
	signed long tmp;

	unsigned char network;

	struct net_device *dev = PROC_GET_DEV();
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);


	tmp = (signed long)(pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod - jiffies);
	if (0 > tmp)
		tmp = 0;
	
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
		network = WIRELESS_11A;
	else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
		if (!isErpSta(pstat))
			network = WIRELESS_11B;
		else {
			network = WIRELESS_11G;
			for (i=0; i<STAT_OPRATE_LEN; i++) {
				if (is_CCK_rate(STAT_OPRATE[i] & 0x7f)) {
					network |= WIRELESS_11B;
					break;
				}
			}
		}
	}
	else // 11B only
		network = WIRELESS_11B;
    
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
		if (pstat->ht_cap_len)
			network |= WIRELESS_11N;
	}

    #ifdef RTK_AC_SUPPORT
    if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)
    {        	
        if(pstat->vht_cap_len) {
            network |= WIRELESS_11AC;
        }
    }
    #endif

    
	PRINT_ONE(num,  " %d: Mesh MP_info...", 1);
	PRINT_SINGL_ARG("    state: ", pstat->mesh_neighbor_TBL.State, "%x");
	PRINT_ARRAY_ARG("    hwaddr: ",	pstat->hwaddr, "%02x", MACADDRLEN);

    if (network == WIRELESS_11B) {
        PRINT_ONE("    mode: B","%s",1);
    }
    else if (network == WIRELESS_11G) {
        PRINT_ONE("    mode: G","%s",1);
    }
    else if (network ==(WIRELESS_11G|WIRELESS_11B)) {
        PRINT_ONE("    mode: B+G","%s",1);
    }
    else if (network ==(WIRELESS_11N)) {
        PRINT_ONE("    mode: N","%s",1);
    }
    else if (network ==(WIRELESS_11G|WIRELESS_11N)) {
        PRINT_ONE("    mode: G+N","%s",1);
    }
    else if (network ==(WIRELESS_11G|WIRELESS_11B | WIRELESS_11N)) {
        PRINT_ONE("    mode: B+G+N","%s",1);
    }
    else if(network == WIRELESS_11A) {
        PRINT_ONE("    mode: A","%s",1);
    }
    else if(network == WIRELESS_11AC) {
        PRINT_ONE("    mode: AC","%s",1);
    }
    else if(network == (WIRELESS_11A|WIRELESS_11N)) {        
        PRINT_ONE("    mode: A+N","%s",1);
    }
    else if(network == (WIRELESS_11AC|WIRELESS_11N)) {
        PRINT_ONE("    mode: N+AC","%s",1);
    }
    else if(network == (WIRELESS_11A|WIRELESS_11AC|WIRELESS_11N)) {
        PRINT_ONE("    mode: A+N+AC","%s",1);   
    } 
    else {
        PRINT_ONE("    mode: --","%s",1); 
    }


	PRINT_SINGL_ARG("    Tx Packets: ", pstat->tx_pkts, "%u");
	PRINT_SINGL_ARG("    Rx Packets: ", pstat->rx_pkts, "%u");
	PRINT_SINGL_ARG("    Authentication: ", ((pstat->state & WIFI_AUTH_SUCCESS) ? 1 : 0), "%d");
	PRINT_SINGL_ARG("    Assocation: ", ((pstat->state & WIFI_ASOC_STATE) ? 1 : 0), "%d");
	PRINT_SINGL_ARG("    LocalLinkID: ", pstat->mesh_neighbor_TBL.LocalLinkID, "%lu");	// %lu=unsigned long
	PRINT_SINGL_ARG("    PeerLinkID: ", pstat->mesh_neighbor_TBL.PeerLinkID, "%lu");		// %lu=unsigned long
	PRINT_SINGL_ARG("    operating_CH: ", pstat->mesh_neighbor_TBL.Co, "%u");
	PRINT_SINGL_ARG("    CH_precedence: ", pstat->mesh_neighbor_TBL.Pl, "%lu");		// %lu=unsigned long
	//PRINT_SINGL_ARG("    R: ", pstat->mesh_neighbor_TBL.r, "%u");

#ifdef RTK_AC_SUPPORT  //vht rate , todo, dump vht rates in Mbps
	if(pstat->current_tx_rate >= VHT_RATE_ID){
		int rate = query_vht_rate(pstat);
		PRINT_SINGL_ARG("    R: ", rate, "%d");
	}
	else
#endif    
	if( is_MCS_rate(pstat->current_tx_rate) )
	{
		PRINT_SINGL_ARG("    R: ", MCS_DATA_RATEStr[(pstat->ht_current_tx_info&BIT(0))?1:0][(pstat->ht_current_tx_info&BIT(1))?1:0][(pstat->current_tx_rate - HT_RATE_ID)], "%s");
	}
	else
	{
		PRINT_SINGL_ARG("    R: ", pstat->current_tx_rate/2, "%u");
	}
    
	PRINT_SINGL_ARG("    Ept: ", pstat->mesh_neighbor_TBL.ept, "%u");
	PRINT_SINGL_ARG("    rssi: ", pstat->mesh_neighbor_TBL.Q, "%u");
#if defined(RTK_MESH_MANUALMETRIC)
	if(pstat->mesh_neighbor_TBL.manual_metric) {
		PRINT_SINGL_ARG("    manual matric: ", pstat->mesh_neighbor_TBL.manual_metric, "%lu");
	} else
#endif
	{
#ifdef RTK_MESH_METRIC_REFINE
		PRINT_SINGL_ARG("    rssi_metric: ", pstat->mesh_neighbor_TBL.rssi_metric, "%lu");
        PRINT_SINGL_ARG("    cu_metric: ", pstat->mesh_neighbor_TBL.cu_metric, "%lu");
        PRINT_SINGL_ARG("    noise_metric: ", pstat->mesh_neighbor_TBL.noise_metric, "%lu");
#endif
		PRINT_SINGL_ARG("    metric: ", pstat->mesh_neighbor_TBL.metric, "%lu");
	}
	PRINT_SINGL_ARG("    expire_Establish(jiffies): ", (pstat->mesh_neighbor_TBL.expire - jiffies), "%lu");		// %lu=unsigned long
	PRINT_SINGL_ARG("                    (Sec): ", RTL_JIFFIES_TO_SECOND(pstat->mesh_neighbor_TBL.expire - jiffies), "%lu");
	PRINT_SINGL_ARG("    expire_BootSeq & LLSA(jiffies): ", tmp, "%lu");		// %lu=unsigned long
	PRINT_SINGL_ARG("                         (mSec): ", RTL_JIFFIES_TO_MILISECONDS(tmp), "%lu");
	PRINT_SINGL_ARG("    retry: ", pstat->mesh_neighbor_TBL.retry, "%d");

	return pos;
}

#ifdef MESH_BOOTSEQ_AUTH
/*
 *	@brief	Printout mesh MP neighbor table in Auth list
 *		PS: Modify from rtl8190_proc_stainfo
 *
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
#ifdef CONFIG_RTL_PROC_NEW
int mesh_auth_mpinfo(struct seq_file *s, void *data)
#else
int mesh_auth_mpinfo(char *buf, char **start, off_t offset, int length, int *eof, void *data)
#endif
{
    struct net_device *dev = PROC_GET_DEV();
    struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

    int len = 0;

#if !defined(CONFIG_RTL_PROC_NEW)    
    off_t begin = 0;
    off_t pos = 0;
    int size;
#endif
    int num=1;
    struct list_head *phead, *plist;
    struct stat_info *pstat;

	if (netif_running(GET_ROOT(priv)->dev)&& netif_running(GET_ROOT(priv)->mesh_dev) && 1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
        
        #ifdef CONFIG_RTL_PROC_NEW
        PRINT_ONE("-- Mesh MP Auth Peer info table -- ", "%s", 1);	        
        #else
        size = sprintf(buf, "-- Mesh MP Auth Peer info table -- \n");
        CHECK_LEN;
        #endif
        
        phead = &priv->mesh_auth_hdr;
        if (!netif_running(priv->mesh_dev) || list_empty(phead))
            goto _ret;

        plist = phead->next;
        while (plist != phead) {
            pstat = list_entry(plist, struct stat_info, mesh_mp_ptr);

            #ifdef CONFIG_RTL_PROC_NEW
            dump_mesh_one_mpinfo(num++, pstat, s, data);        
            #else
            size = dump_mesh_one_mpinfo(num++, pstat, buf+len, start, offset, length,
            eof, data);
            CHECK_LEN;
            #endif
            
            plist = plist->next;
        }
    }
    else {
        #ifdef CONFIG_RTL_PROC_NEW
        PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
        #else
		size = sprintf(buf, "  Mesh mode DISABLE !!\n");
		CHECK_LEN;        
        #endif
    }	

_ret:    
#if !defined(CONFIG_RTL_PROC_NEW)    
    *eof = 1;
    *start = buf + (offset - begin);	/* Start of wanted data */
    len -= (offset - begin);	/* Start slop */
    if (len > length)
        len = length;	/* Ending slop */
#endif

    return len;
}	
#endif

/*
 *	@brief	Printout mesh MP neighbor table unEstablish list
 *		PS:Modify from rtl8190_proc_stainfo 
 *
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
#ifdef CONFIG_RTL_PROC_NEW
int mesh_unEstablish_mpinfo(struct seq_file *s, void *data)
#else
int mesh_unEstablish_mpinfo(char *buf, char **start, off_t offset, int length, int *eof, void *data)
#endif
{
    struct net_device *dev = PROC_GET_DEV();
    struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

    int len = 0;

#if !defined(CONFIG_RTL_PROC_NEW)
    off_t begin = 0;
    off_t pos = 0;
    int size;
#endif   

    int num=1;
    struct list_head *phead, *plist;
    struct stat_info *pstat;

    if (netif_running(GET_ROOT(priv)->dev)&& netif_running(GET_ROOT(priv)->mesh_dev) && 1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
        #ifdef CONFIG_RTL_PROC_NEW
        PRINT_ONE("-- Mesh MP unEstablish Peer info table -- ", "%s", 1);	
        #else
        size = sprintf(buf, "-- Mesh MP unEstablish Peer info table -- \n");
        CHECK_LEN;
        #endif


        phead = &priv->mesh_unEstablish_hdr;
        if (!netif_running(priv->mesh_dev) || list_empty(phead))
            goto _ret;

        plist = phead->next;
        while (plist != phead) {
            pstat = list_entry(plist, struct stat_info, mesh_mp_ptr);
            
            #ifdef CONFIG_RTL_PROC_NEW
            dump_mesh_one_mpinfo(num++, pstat, s, data);            
            #else
            size = dump_mesh_one_mpinfo(num++, pstat, buf+len, start, offset, length,
            eof, data);
            CHECK_LEN;
            #endif

            plist = plist->next;
        }
    }
    else {

        #ifdef CONFIG_RTL_PROC_NEW
        PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
        #else
        size = sprintf(buf, "  Mesh mode DISABLE !!\n");
        CHECK_LEN;
        #endif
    }	

_ret:

#if !defined(CONFIG_RTL_PROC_NEW)
    *eof = 1;
    *start = buf + (offset - begin);	/* Start of wanted data */
    len -= (offset - begin);	/* Start slop */
    if (len > length)
        len = length;	/* Ending slop */
#endif

    return len;
}	



/*
 *	@brief	Printout sta_info all of mesh MP neighbor flow, Throughput statistics (sounder)
 *
 *	@param	unknow
 *
 *	@retval	int: pos:unknow
 */
#ifdef CONFIG_RTL_PROC_NEW
static int dump_mesh_one_mpflow_neighbor(int num, struct stat_info *pstat, struct seq_file *s, void *data)
#else
static int dump_mesh_one_mpflow_neighbor(int num, struct stat_info *pstat, char *buf, char **start, off_t offset, int length, int *eof, void *data)
#endif
{
	struct net_device *dev = PROC_GET_DEV();
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	int pos = 0;

	if(priv->mesh_log)
	{
		PRINT_SINGL_ARG("    log_time: ",		priv->log_time, "%lu");
		PRINT_SINGL_ARG("    rx_packets: ",		pstat->rx_pkts, "%u");
		PRINT_SINGL_ARG("    rx_bytes: ",		pstat->rx_bytes,"%u");
		PRINT_SINGL_ARG("    tx_packets: ",		pstat->tx_pkts, "%u");
		PRINT_SINGL_ARG("    tx_bytes: ",		pstat->tx_bytes,"%u");
	}
	else
	{
		PRINT_SINGL_ARG("    log_time: ",		-99, "%d");
		PRINT_SINGL_ARG("    rx_packets: ",		-99, "%d");
		PRINT_SINGL_ARG("    rx_bytes: ",		-99, "%d");
		PRINT_SINGL_ARG("    tx_packets: ",		-99, "%d");
		PRINT_SINGL_ARG("    tx_bytes: ",		-99, "%d");
	}
	PRINT_ONE("", "%s", 1);

	return pos;
}



/*
 *	@brief	Printout sta_info all of mesh MP neighbor flow,  Throughput statistics (sounder)
 *
 *	@param	unknow
 *
 *	@retval	int: pos:unknow
 */
#ifdef CONFIG_RTL_PROC_NEW
int dump_mesh_one_mpflow_sta(struct seq_file *s, struct stat_info *pstat)
#else
int dump_mesh_one_mpflow_sta(struct stat_info *pstat, char *buf, char **start, off_t offset, int length, int *eof, void *data)
#endif
{
	struct net_device *dev = PROC_GET_DEV();
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	struct list_head *phead, *plist;
	struct stat_info *pstat1;
	int result = 0;
	int pos = 0;

	if(priv->mesh_log)
	{
		PRINT_SINGL_ARG("    log_time: ",		priv->log_time, "%lu");
		PRINT_SINGL_ARG("    rx_packets: ",		pstat->rx_pkts, "%u");
		PRINT_SINGL_ARG("    rx_bytes: ",		pstat->rx_bytes,"%u");
		PRINT_SINGL_ARG("    tx_packets: ",		pstat->tx_pkts, "%u");
		PRINT_SINGL_ARG("    tx_bytes: ",		pstat->tx_bytes,"%u");

		phead = &priv->mesh_mp_hdr;

		plist = phead->next;
		while (plist != phead) 
		{
			pstat1 = list_entry(plist, struct stat_info, mesh_mp_ptr);
			if(pstat->hwaddr == pstat1->hwaddr )
			{
				result = 1;
				break;
			}
	 		plist = plist->next;
		}

		PRINT_SINGL_ARG("    mesh_node: ",		result,"%u");
	}
	else
	{
		PRINT_SINGL_ARG("    log_time: ",		-99, "%d");
		PRINT_SINGL_ARG("    rx_packets: ",		-99, "%d");
		PRINT_SINGL_ARG("    rx_bytes: ",		-99, "%d");
		PRINT_SINGL_ARG("    tx_packets: ",		-99, "%d");
		PRINT_SINGL_ARG("    tx_bytes: ",		-99, "%d");
		PRINT_SINGL_ARG("    mesh_node: ",		-99, "%d");
	}
	PRINT_ONE("", "%s", 1);

	return pos;
}



/*
 *	@brief	Printout mesh MP neighbor table in association successful
 *		PS:Modify from rtl8190_proc_stainfo
 *
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
#ifdef CONFIG_RTL_PROC_NEW
int mesh_assoc_mpinfo(struct seq_file *s, void *data)
#else
int mesh_assoc_mpinfo(char *buf, char **start, off_t offset, int length, int *eof, void *data)
#endif
{
	struct net_device *dev = PROC_GET_DEV();
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	int len = 0;
#if defined(__KERNEL__) && !defined(CONFIG_RTL_PROC_NEW)
    off_t begin = 0;
    off_t pos = 0;
    int size;
#endif    

    int  num=1;
    struct list_head *phead, *plist;
    struct stat_info *pstat;

    if (netif_running(GET_ROOT(priv)->dev)&& netif_running(GET_ROOT(priv)->mesh_dev) && 1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {

        #ifdef CONFIG_RTL_PROC_NEW
        PRINT_ONE("-- Mesh MP Association peer info table --", "%s", 1);
        #else
        size = sprintf(buf, "-- Mesh MP Association peer info table -- \n");
        CHECK_LEN;
        #endif

        phead = &priv->mesh_mp_hdr;
        if (!netif_running(priv->mesh_dev) || list_empty(phead))
            goto _ret;

        plist = phead->next;
        while (plist != phead) {
            pstat = list_entry(plist, struct stat_info, mesh_mp_ptr);

            #ifdef CONFIG_RTL_PROC_NEW
            dump_mesh_one_mpinfo(num++, pstat, s, data); 
            dump_mesh_one_mpflow_neighbor(num, pstat, s, data);
            #else
            size = dump_mesh_one_mpinfo(num++, pstat, buf+len, start, offset, length,
            eof, data);
            CHECK_LEN;

            // 3 line for Throughput statistics (sounder)	
            size = dump_mesh_one_mpflow_neighbor(num, pstat, buf+len, start, offset, length,
            eof, data);
            CHECK_LEN;
            #endif

            plist = plist->next;
        }
    }
    else {
    
        #ifdef CONFIG_RTL_PROC_NEW
        PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
        #else
        size = sprintf(buf, "  Mesh mode DISABLE !!\n");
        CHECK_LEN
        #endif
    }

_ret:

#if !defined(CONFIG_RTL_PROC_NEW)   
	*eof = 1;
	*start = buf + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */
#endif
    
	return len;

}

/*
 *	@brief	Control flow Throughput statistics (sounder)
 *		
 *	@param	unknow
 *
 *	@retval	int: count:unknow
 */
int mesh_proc_flow_stats_write(struct file *file, const char *buffer, unsigned long count, void *data)
{

    struct net_device *dev = (struct net_device *)data;
    struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

    struct list_head *phead, *plist;
    struct stat_info *pstat;

    //if( priv->mesh_log )
    if(*buffer == '0') 
    {		
        // turn off log function
        priv->mesh_log = 0;
        priv->log_time = 0;
    }
    else
    {
        // reset all log variable
        phead = &priv->asoc_list;
        if (!netif_running(priv->mesh_dev) || list_empty(phead))
            goto _ret;

        plist = phead->next;

        while (plist != phead) 
        {
            pstat = list_entry(plist, struct stat_info, asoc_list);

            pstat->rx_pkts  = 0;
            pstat->rx_bytes = 0;
            pstat->tx_pkts  = 0; 
            pstat->tx_bytes = 0;

            plist = plist->next;
        }
        priv->mesh_log = 1;
    }
_ret:

    return count;
}

/*
 *	@brief	Printout flow Throughput statistics (sounder)
 *		
 *	@param	unknow
 *
 *	@retval	int: pos:unknow
 */

#ifdef CONFIG_RTL_PROC_NEW
int mesh_proc_flow_stats_read(struct seq_file *s, void *data)
#else 
int mesh_proc_flow_stats_read(char *buf, char **start, off_t offset, int length, int *eof, void *data)
#endif
{
	struct net_device *dev = PROC_GET_DEV();
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	off_t pos = 0;

	if( priv->mesh_log )
	{
		PRINT_ONE("1", "%s", 1);
	}
	else
	{
		PRINT_ONE("0", "%s", 1);
	}

	return	pos;
}


/*
 *	@brief	Print all about of mesh statistics and parameter.
 *		
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
#ifdef CONFIG_RTL_PROC_NEW
int mesh_stats(struct seq_file *s, void *data)
#else 
int mesh_stats(char *buf, char **start, off_t offset, int length, int *eof, void *data)
#endif
{
	struct net_device *dev = PROC_GET_DEV();
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	int pos = 0;
	
	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		PRINT_ONE("  Systems Statistics...", "%s", 1);
		PRINT_SINGL_ARG("    OPMODE:       ", OPMODE, "%X");
		PRINT_SINGL_ARG("    jiffies:      ", jiffies, "%lu");		// %lu=unsigned long
		
		PRINT_ONE("  Mesh Networks Statistics...", "%s", 1);
		
		PRINT_SINGL_ARG("    tx_packets:   ", priv->mesh_stats.tx_packets, "%lu");
		PRINT_SINGL_ARG("    tx_bytes:     ", priv->mesh_stats.tx_bytes, "%lu");
		PRINT_SINGL_ARG("    tx_errors:    ", priv->mesh_stats.tx_errors, "%lu");
		PRINT_SINGL_ARG("    rx_packets:   ", priv->mesh_stats.rx_packets, "%lu");
		PRINT_SINGL_ARG("    rx_bytes:     ", priv->mesh_stats.rx_bytes, "%lu");
		PRINT_SINGL_ARG("    rx_errors:    ", priv->mesh_stats.rx_errors, "%lu");
		PRINT_SINGL_ARG("    rx_crc_errors: ", priv->mesh_stats.rx_crc_errors, "%lu");

		PRINT_ONE("  WLAN Mesh Capability...", "%s", 1);	
		PRINT_SINGL_ARG("    Version:          ", priv->mesh_Version, "%u");
        PRINT_SINGL_ARG("    PathSelectProtocolID:          ", priv->mesh_profile[0].PathSelectProtocolID.value, "%u");        
		PRINT_ONE("    Peer_CAP:", "%s", 1);
		PRINT_SINGL_ARG("      Capacity:       ", MESH_PEER_LINK_CAP_NUM(priv), "%hd");
		PRINT_SINGL_ARG("      Flags:          ", (priv->mesh_PeerCAP_flags & MESH_PEER_LINK_CAP_FLAGS_MASK), "%hX");
		PRINT_SINGL_ARG("    Power_save_CAP:   ", priv->mesh_PowerSaveCAP, "%X");
		PRINT_SINGL_ARG("    SYNC_CAP:         ", priv->mesh_SyncCAP, "%X");
		PRINT_SINGL_ARG("    MDA_CAP:          ", priv->mesh_MDA_CAP, "%X");
		PRINT_SINGL_ARG("    ChannelPrecedence:", priv->mesh_ChannelPrecedence, "%lu");

#ifdef _MESH_ACL_ENABLE_
		PRINT_ONE("  Access Control List (ACL) cache content...", "%s", 1);
		PRINT_ARRAY_ARG("    MAC:          ", priv->meshAclCacheAddr, "%02x", MACADDRLEN);
		PRINT_SINGL_ARG("    Mode:         ", priv->meshAclCacheMode, "%d");
#endif

	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	return pos;
}

#ifdef CONFIG_RTL_PROC_NEW
int mesh_pathsel_routetable_info(struct seq_file *s, void *data)
#else
int mesh_pathsel_routetable_info(char *buf, char **start, off_t offset, int length, int *eof, void *data)
#endif
{
    struct net_device *dev = PROC_GET_DEV();
    struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

    int len = 0;
    int tbl_sz;

#if !defined(CONFIG_RTL_PROC_NEW)
    off_t begin = 0;
    off_t pos = 0;
    int size;
#endif

    int isPortal;
    struct path_sel_entry * ptable;
    int i=0, j=0,k;

    struct pann_mpp_tb_entry * mpptable;
    unsigned long now =  jiffies;
    int num = 1;
    unsigned long flags;


    if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
        if (!netif_running(priv->mesh_dev) )
            goto _ret;

        PRINT_ONE(num,  " %d: Mesh route table info...", num++);
        PRINT_ONE("    destMAC: My-self", "%s", 1);
        PRINT_ONE("    nexthopMAC: ---", "%s", 1);
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
        priv = (DRV_PRIV *)priv->mesh_priv_first;
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE
        if( priv->pmib->dot1180211sInfo.mesh_portal_enable )
        {
            PRINT_ONE("    portal enable: yes", "%s", 1);
        }
        else
        {
            PRINT_ONE("    portal enable: no", "%s", 1);
        }

        PRINT_ONE("    dsn: ---", "%s", 1);
        PRINT_ONE("    metric: ---", "%s", 1);
        PRINT_ONE("    hopcount: ---", "%s", 1);

        PRINT_ONE("    start: ---", "%s" ,1);
        PRINT_ONE("    end: ---", "%s", 1);
        PRINT_ONE("    diff: ---", "%s", 1);
        PRINT_ONE("    flag: ---", "%s", 1);

        PRINT_ONE("", "%s", 1);

        
        #if !defined(CONFIG_RTL_PROC_NEW)
        size = pos;
        CHECK_LEN; 
        pos = len;
        #endif        
               
        SAVE_INT_AND_CLI(flags);
        SMP_LOCK_MESH_PATH(flags);        
        mpptable = (struct pann_mpp_tb_entry *) &(priv->pann_mpp_tb->pann_mpp_pool);        

        tbl_sz = (1 << priv->pathsel_table->table_size_power);
        for(i=0;i<tbl_sz;i++)
        {        
            isPortal=0;                        
            if(priv->pathsel_table->entry_array[i].dirty != 0)
            {			
                ptable = (struct path_sel_entry*)priv->pathsel_table->entry_array[i].data;
                PRINT_ONE(num,  " %d: Mesh route table info...", num++);
                PRINT_ARRAY_ARG("    destMAC: ",	ptable->destMAC, "%02x", MACADDRLEN);
                PRINT_ARRAY_ARG("    nexthopMAC: ",	ptable->nexthopMAC, "%02x", MACADDRLEN);

                for( j=0; j<MAX_MPP_NUM ;j++ )
                {
                    if( mpptable[j].flag && !memcmp(mpptable[j].mac, ptable->destMAC, MACADDRLEN) )
                    {
                        isPortal = 1;
                        break;
                    }
                }
                if( isPortal )
                {
                  PRINT_ONE("    portal enable: yes", "%s", 1);
                }
                else
                {
                  PRINT_ONE("    portal enable: no", "%s", 1);
                } 
    			
                PRINT_SINGL_ARG("    dsn: ", ptable->dsn, "%u");	// %lu=unsigned long
                PRINT_SINGL_ARG("    metric: ", ptable->metric, "%u");
                PRINT_SINGL_ARG("    hopcount: ", ptable->hopcount, "%u");		// %lu=unsigned long
                PRINT_SINGL_ARG("    start: ", ptable->start, "%u");
                PRINT_SINGL_ARG("    end: ", ptable->end, "%u");
                PRINT_SINGL_ARG("    diff: ", ptable->end-ptable->start, "%u");
                PRINT_SINGL_ARG("    aging(Sec): ", RTL_JIFFIES_TO_SECOND(now - ptable->update_time), "%ld");
#ifdef MESH_ROUTE_MAINTENANCE
                PRINT_SINGL_ARG("    routeMaintain: ", RTL_JIFFIES_TO_SECOND(now - ptable->routeMaintain), "%ld");
#endif
                PRINT_SINGL_ARG("    interface: ", ptable->priv->dev->name, "%s");

#ifdef SUPPORT_TX_MCAST2UNI
                PRINT_SINGL_ARG("    ipmc_num: ", ptable->ipmc_num, "%d");
                for (j=0; j<MAX_IP_MC_ENTRY; j++) {
                    if (ptable->ipmc[j].used) {
                        PRINT_ARRAY_ARG("    mcmac: ",  ptable->ipmc[j].mcmac, "%02x", MACADDRLEN);
                        if (ptable->ipmc[j].srcCount) {
                            for(k = 0;k < MAX_IP_SRC_ENTRY; k++) {
                                if(ptable->ipmc[j].srcValid[k]) {
                                    PRINT_ARRAY_ARG("      client: ",  ptable->ipmc[j].srcMac[k], "%02x", MACADDRLEN);
                                }
                            }
                        }   
                    }
                }
#endif            
                PRINT_ONE("", "%s", 1);

                #if !defined(CONFIG_RTL_PROC_NEW)
                size = pos - len;
                CHECK_LEN;
                pos = len;
                #endif

            }	

           
            
        }	

        RESTORE_INT(flags);
        SMP_UNLOCK_MESH_PATH(flags);
    }
    else {
        PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
        
        #if !defined(CONFIG_RTL_PROC_NEW)
        size = pos;
        CHECK_LEN;       
        #endif          
    }

_ret:

#if !defined(CONFIG_RTL_PROC_NEW)
    *eof = 1;
    *start = buf + (offset - begin);	/* Start of wanted data */
    len -= (offset - begin);	/* Start slop */
    if (len > length)
        len = length;	/* Ending slop */
#endif

    return len;
}

/*
 *	@brief	Printout 802.11s pathselection proxy table value
 *
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
#ifdef CONFIG_RTL_PROC_NEW
int mesh_proxy_table_info(struct seq_file *s, void *data)
#else
int mesh_proxy_table_info(char *buf, char **start, off_t offset, int length, int *eof, void *data)
#endif
{
#ifdef SMP_SYNC
    unsigned long flags;
#endif

    struct net_device *dev = PROC_GET_DEV();
    struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

    int len = 0;
#if !defined(CONFIG_RTL_PROC_NEW)    
    off_t begin = 0;
    off_t pos = 0;
    int size;
#endif

    struct proxy_table_entry * ptable_entry;
    int i=0;
    int num;

    if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
        if (!netif_running(priv->mesh_dev) )
            goto _ret;

        num = 1;
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
        priv = (DRV_PRIV *)priv->mesh_priv_first;
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE
        SMP_LOCK_MESH_PROXY(flags);    
        for(i = 0; i < (1 << priv->proxy_table->table_size_power); i++)
        {
            if(priv->proxy_table->entry_array[i].dirty != 0)
            {               
                ptable_entry = (struct proxy_table_entry*)priv->proxy_table->entry_array[i].data;
                PRINT_ONE(num,  " %d: Mesh proxy table info...", num++);

                PRINT_ARRAY_ARG("    STA_MAC: ",	ptable_entry->sta, "%02x", MACADDRLEN);
                PRINT_ARRAY_ARG("    OWNER_MAC: ",	ptable_entry->owner, "%02x", MACADDRLEN);
                PRINT_ONE(ptable_entry->aging_time,  "    Aging time: %d", 1);

                #if !defined(CONFIG_RTL_PROC_NEW)
                size = pos - len;
                CHECK_LEN;
                pos = len;
                #endif
            }
        }	
        SMP_UNLOCK_MESH_PROXY(flags);    
    } 
    else {
        PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
        #if !defined(CONFIG_RTL_PROC_NEW)
        size = pos;
        CHECK_LEN;       
        #endif         
    }

_ret:

#if !defined(CONFIG_RTL_PROC_NEW)
    *eof = 1;
    *start = buf + (offset - begin);	/* Start of wanted data */
    len -= (offset - begin);	/* Start slop */
    if (len > length)
        len = length;	/* Ending slop */
#endif
    return len;
}


/*
 *	@brief	Printout 802.11s pathselection rtl8190_root_info
 *
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
#ifdef CONFIG_RTL_PROC_NEW
int mesh_root_info(struct seq_file *s, void *data)
#else
int mesh_root_info(char *buf, char **start, off_t offset, int length, int *eof, void *data)
#endif
{

    struct net_device *dev = PROC_GET_DEV();
    struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

    int len = 0;
#if !defined(CONFIG_RTL_PROC_NEW)      
    off_t begin = 0;
    off_t pos = 0;
    int size;
#endif
    if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
        if (!netif_running(priv->mesh_dev) )
            goto _ret;		
        PRINT_ARRAY_ARG("    ROOT_MAC: ",	priv->root_mac, "%02x", MACADDRLEN);
    } else {
        PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);  
    }

    #if !defined(CONFIG_RTL_PROC_NEW)      
    size = pos;
    CHECK_LEN;
    #endif

_ret:

#if !defined(CONFIG_RTL_PROC_NEW)	
    *eof = 1;
    *start = buf + (offset - begin);	/* Start of wanted data */
    len -= (offset - begin);	/* Start slop */
    if (len > length)
        len = length;	/* Ending slop */
#endif

    return len;
}

#ifdef CONFIG_RTK_VLAN_SUPPORT

#ifdef CONFIG_RTL_PROC_NEW
int mesh_vlan_info(struct seq_file *s, void *data)
#else
int mesh_vlan_info(char *buf, char **start, off_t offset, int length, int *eof, void *data)
#endif
{
	struct net_device *dev = PROC_GET_DEV();
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	int pos = 0;

	PRINT_ONE("  vlan setting...", "%s", 1);
	PRINT_SINGL_ARG("    global_vlan: ", priv->mesh_vlan.global_vlan, "%d");
	PRINT_SINGL_ARG("    is_lan: ", priv->mesh_vlan.is_lan, "%d");
	PRINT_SINGL_ARG("    vlan_enable: ", priv->mesh_vlan.vlan, "%d");
	PRINT_SINGL_ARG("    vlan_tag: ", priv->mesh_vlan.tag, "%d");
	PRINT_SINGL_ARG("    vlan_id: ", priv->mesh_vlan.id, "%d");
	PRINT_SINGL_ARG("    vlan_pri: ", priv->mesh_vlan.pri, "%d");
	PRINT_SINGL_ARG("    vlan_cfi: ", priv->mesh_vlan.cfi, "%d");
#if defined(CONFIG_RTK_BRIDGE_VLAN_SUPPORT)
	PRINT_SINGL_ARG("    vlan_forwarding_rule: ", priv->mesh_vlan.forwarding_rule, "%d");
#endif

	return pos;
}

#endif
/*
 *	@brief	Printout 802.11s pathselection portal  table value
 *
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
#ifdef CONFIG_RTL_PROC_NEW
int mesh_portal_table_info(struct seq_file *s, void *data)
#else
int mesh_portal_table_info(char *buf, char **start, off_t offset, int length, int *eof, void *data)
#endif
{
    struct net_device *dev = PROC_GET_DEV();
    struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

    int len = 0;
#if !defined(CONFIG_RTL_PROC_NEW)          
    off_t begin = 0;
    off_t pos = 0;
    int size;
#endif    
    struct pann_mpp_tb_entry * ptable;
    int i=0;
    int num;
    
    if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
        if (!netif_running(priv->mesh_dev) )
            goto _ret;

#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
        priv = (DRV_PRIV *)priv->mesh_priv_first;
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE
        ptable = (struct pann_mpp_tb_entry *) &(priv->pann_mpp_tb->pann_mpp_pool);
        num = 1;

        PRINT_SINGL_ARG("Portal enable: ", priv->pmib->dot1180211sInfo.mesh_portal_enable, "%u");

        if(priv->pmib->dot1180211sInfo.mesh_portal_enable)
        {			
            PRINT_ONE(num,  " %d: Mesh portal table info...", num++);
            if( priv->pmib->dot1180211sInfo.mesh_portal_enable )
            {
                PRINT_ARRAY_ARG("    PortalMAC: ",	priv->pmib->dot11OperationEntry.hwaddr, "%02x", MACADDRLEN);		
                PRINT_SINGL_ARG("    timeout: ", 99, "%u");
                PRINT_SINGL_ARG("    seqNum: ", 99, "%u");			
                PRINT_ONE("", "%s", 1);
            }
        }
        
        #if !defined(CONFIG_RTL_PROC_NEW)      
        size = pos;
        CHECK_LEN;
        pos = len;
        #endif
        
        for(i=0;i<MAX_MPP_NUM;i++)
        {
            if(ptable[i].flag)
            {			
                PRINT_ONE(num,  " %d: Mesh portal table info...", num++);				
                PRINT_ARRAY_ARG("    PortalMAC: ",	ptable[i].mac, "%02x", MACADDRLEN);		
                PRINT_SINGL_ARG("    timeout: ", ptable[i].timeout, "%u");	// %lu=unsigned long
                PRINT_SINGL_ARG("    seqNum: ", ptable[i].seqNum, "%u");
                PRINT_ONE("", "%s", 1);
            
                #if !defined(CONFIG_RTL_PROC_NEW)      
                size = pos - len;
                CHECK_LEN;
                pos = len;
                #endif


            }	
        }	
    }
    else {
        PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
        
        #if !defined(CONFIG_RTL_PROC_NEW)      
        size = pos;
        CHECK_LEN;
        #endif
    }


_ret:

#if !defined(CONFIG_RTL_PROC_NEW)	
    *eof = 1;
    *start = buf + (offset - begin);	/* Start of wanted data */
    len -= (offset - begin);	/* Start slop */
    if (len > length)
        len = length;	/* Ending slop */
#endif

    return len;
}

#ifdef MESH_USE_METRICOP
int mesh_metric_w (struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char local_buf[5];
	unsigned int i;

	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;

	if ( copy_from_user((void *)local_buf, (const void *)buffer, count ))
	{ return count; }

	//printk("@@XDXD OLD metricid %d\n", priv->mesh_fake_mib.metricID);

	sscanf(local_buf, "%d", &i);
	priv->mesh_fake_mib.metricID = (UINT8)i;
	//printk("@@XDXD new metricid %d\n", priv->mesh_fake_mib.metricID);

	return count;
}

#ifdef CONFIG_RTL_PROC_NEW
int mesh_metric_r(struct seq_file *s, void *data)
#else
int mesh_metric_r(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data)
#endif
{
    struct net_device *dev = (struct net_device *)data;
    DRV_PRIV *priv = (DRV_PRIV *)dev->priv;

    int ret = 0;
    if (offset > 0) {
        /* we have finished to read, return 0 */
        ret  = 0;
    } else {
        #if defined(CONFIG_RTL_PROC_NEW)
        seq_printf(s, "metric method=%d\n", priv->mesh_fake_mib.metricID);
        #else
        sprintf(buffer, "metric method=%d\n", priv->mesh_fake_mib.metricID);
        ret = strlen(buffer);
        #endif
    }
    return ret;
}
#endif // MESH_USE_METRICOP


#ifdef _MESH_PROC_DEBUG_
/*
 *	@brief	Ckear ALL stat_info (debug only)
 *		
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
static int mesh_proc_clear_table(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;
	int pos = 0;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		
#ifdef MESH_BOOTSEQ_AUTH
		while (!list_empty(&priv->mesh_auth_hdr)) {
			pstat = list_entry(priv->mesh_auth_hdr.next, struct stat_info, mesh_mp_ptr);
			free_stainfo(priv, pstat);
		}
#endif

		while (!list_empty(&priv->mesh_unEstablish_hdr)) {
			pstat = list_entry(priv->mesh_unEstablish_hdr.next, struct stat_info, mesh_mp_ptr); 
			free_stainfo(priv, pstat);
		}

		while (!list_empty(&priv->mesh_mp_hdr)) {
			pstat = list_entry(priv->mesh_mp_hdr.next, struct stat_info, mesh_mp_ptr); 
			free_stainfo(priv, pstat);
		}
		
		mesh_set_PeerLink_CAP(priv, GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor);	// Reset connection number

		PRINT_ONE("  All table clear ok...", "%s", 1);
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
	
	return pos;
}


int mesh_ProcMACParser(const char *buffer, unsigned long *count)
{
	char local_buf[19];
	char *ptr;
	UINT8 mac[MACADDRLEN];
	
	int idx = 0;

	memset(local_buf, 0, sizeof(local_buf));
			
	if (*count < 11 ) { // a:b:c:d:e:f
		return -EFAULT;
	}
	
	if (*count > 18 ) {
		*count = 18;
	}
	
	/* write data to the buffer */
	if ( copy_from_user((void *)local_buf, (const void *)buffer, *count) ) {
		printk("*** mesh_test_sme_proc_write: copy from user error\r\n");
		return -EFAULT;
	}
	
	memset(mac, 0, sizeof(mac));
	ptr = local_buf;
	while(*ptr)
	{
		unsigned char val = 0;
		
		if(idx>5)
			break;
			
		if(*ptr == ':')
		{
			ptr++;
			idx++;
			continue;
		}
		if( (*ptr>='0') && (*ptr<='9') )
		{
			val = *ptr - '0';
		}
		else if( (*ptr>='A') && (*ptr<='F') )
		{
			val = *ptr - 'A' + 10;
		}
		else if( (*ptr>='a') && (*ptr<='f') )
		{
			val = *ptr - 'a' + 10;
		}
		else
		{
			ptr++;
			continue;
		}
		mac[idx] = mac[idx]*16+val;

		ptr++;
	}
	memcpy(mesh_proc_MAC, mac, MACADDRLEN);
	
	return SUCCESS;

}


static int mesh_setMACAddr(struct file *file, const char *buffer, unsigned long count, void *data)
{
	mesh_ProcMACParser(buffer, &count);
	
	MESH_DEBUG_MSG("Set MAC Address %02X:%02X:%02X:%02X:%02X:%02X is OK !!\n",
			mesh_proc_MAC[0], mesh_proc_MAC[1], mesh_proc_MAC[2], mesh_proc_MAC[3], mesh_proc_MAC[4], mesh_proc_MAC[5]);

	return count;
}

#ifdef MESH_BOOTSEQ_AUTH
static int mesh_proc_issueAuthReq(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;

	int pos = 0;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		pstat = get_stainfo(priv, mesh_proc_MAC);
		if (NULL != pstat) {
			pstat->auth_seq = 1;
			issue_auth(priv, pstat, _STATS_SUCCESSFUL_);
			PRINT_ONE(" Sent Auth Request OK....", "%s", 1);
		} else {
			PRINT_ONE("  MAC not exist !!", "%s", 1);
		}
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	return pos;
}

static int mesh_proc_issueAuthRsp(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;

	int pos = 0;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		pstat = get_stainfo(priv, mesh_proc_MAC);
		if (NULL != pstat) {
			pstat->auth_seq = 2;
			issue_auth(priv, pstat, _STATS_SUCCESSFUL_);
			PRINT_ONE(" Sent Auth Response OK....", "%s", 1);
		} else {
			PRINT_ONE("  MAC not exist !!", "%s", 1);
		}
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
	
	return pos;
}


static int mesh_proc_issueDeAuth(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;
	
	int pos = 0;
	
	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		pstat = get_stainfo(priv, mesh_proc_MAC);
		
		if (NULL != pstat) {
			issue_deauth_MP(priv, mesh_proc_MAC, _RSON_CLS2_, TRUE);
			PRINT_ONE(" Sent DeAuth OK....", "%s", 1);
		} else
			PRINT_ONE("  MAC not exist !!", "%s", 1);

	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
	
	return pos;
}
#endif

static int mesh_proc_openConnect(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;

	int pos = 0;
	
	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		struct rx_frinfo	pfrinfo;	//Note: It isn't pointer !!
		
		PRINT_ARRAY_ARG("I will Active peer link MAC:", mesh_proc_MAC, "%02x", MACADDRLEN);
		pfrinfo.sa = mesh_proc_MAC;
		pfrinfo.rssi = priv->mesh_fake_mib.establish_rssi_threshold + 1;	// +1: Ensure connect
		start_MeshPeerLink(priv, &pfrinfo, NULL, 1);
		pstat = get_stainfo(priv, mesh_proc_MAC);
		
		if (NULL != pstat)
			PRINT_SINGL_ARG("Start active PeerLink: LocalLinkID=", pstat->mesh_neighbor_TBL.LocalLinkID, "%lu");

	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	return pos;
}

static int mesh_proc_issueOpen(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;

	int pos = 0;
	
	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		pstat = get_stainfo(priv, mesh_proc_MAC);
		if (NULL != pstat) {
			PRINT_ARRAY_ARG("I will issue Open frame MAC:", mesh_proc_MAC, "%02x", MACADDRLEN);
			issue_assocreq_MP(priv, pstat);
			PRINT_SINGL_ARG("Issue Open: LocalLinkID=", pstat->mesh_neighbor_TBL.LocalLinkID, "%lu");
			PRINT_SINGL_ARG("Issue Open: PeerLinkID=", pstat->mesh_neighbor_TBL.PeerLinkID, "%lu");
		}
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	return pos;
}

static int mesh_proc_issueConfirm(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;

	int pos = 0;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		pstat = get_stainfo(priv, mesh_proc_MAC);
		if (NULL != pstat) {
			PRINT_ARRAY_ARG("I will issue Confirm frame MAC:", mesh_proc_MAC, "%02x", MACADDRLEN);
			issue_assocrsp_MP(priv, 0, pstat, WIFI_ASSOCRSP);
			PRINT_SINGL_ARG("Issue Confirm: LocalLinkID=", pstat->mesh_neighbor_TBL.LocalLinkID, "%lu");
			PRINT_SINGL_ARG("Issue Confirm: PeerLinkID=", pstat->mesh_neighbor_TBL.PeerLinkID, "%lu");
		}
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	return pos;
}

static int mesh_proc_issueClose(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;

	int pos = 0;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		pstat = get_stainfo(priv, mesh_proc_MAC);
		if (NULL != pstat) {
			PRINT_ARRAY_ARG("I will issue Close frame MAC:", mesh_proc_MAC, "%02x", MACADDRLEN);
			issue_disassoc_MP(priv, pstat, 0, 0);
			PRINT_SINGL_ARG("Issue Close: LocalLinkID=", pstat->mesh_neighbor_TBL.LocalLinkID, "%lu");
			PRINT_SINGL_ARG("Issue Close: PeerLinkID=", pstat->mesh_neighbor_TBL.PeerLinkID, "%lu");
		}
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	return pos;
}

static int mesh_proc_closeConnect(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;

	int pos = 0;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		PRINT_ARRAY_ARG("I will Close peer link MAC:", mesh_proc_MAC, "%02x", MACADDRLEN);
		close_MeshPeerLink(priv, mesh_proc_MAC);
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	return pos;
}

#endif //_MESH_DEBUG_

#endif //  CONFIG_RTK_MESH
