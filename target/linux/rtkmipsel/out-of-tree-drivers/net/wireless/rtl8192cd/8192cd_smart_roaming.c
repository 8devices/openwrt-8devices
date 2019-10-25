
/*
 *  Software Smart Roaming routines
 *
 *  $Id: rtl_smart_roaming.c, v 1.4.4.2 2016/04/30 05:27:28 Tracy Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_SMART_ROAMING_C_

#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/timer.h>
#include "8192cd.h"
#include "8192cd_util.h"
#include "8192cd_headers.h"

#define NETLINK_RTK 31
#define MAX_PAYLOAD	2048


struct timer_list		*timer_wlan0;
struct timer_list		*timer_wlan1;

struct sock *rtl_smart_roaming_nl = NULL;

int wlan0_used,wlan1_used,wlan0_deleted,wlan1_deleted,msg_dropcounter;

int pid = 0;

static void sr_timer_rec_add() {
    if (wlan0_used && wlan0_deleted ){
        add_timer(timer_wlan0);
        wlan0_deleted = 0;
    }
    if (wlan1_used && wlan1_deleted) {
        add_timer(timer_wlan1);
        wlan1_deleted = 0;
    }
}


static void sr_timer_rec_del(){
    if(wlan0_used && (wlan0_deleted == 0)){
        del_timer_sync(timer_wlan0);
        wlan0_used = 0;
        wlan0_deleted = 1;
    }
    if(wlan1_used && (wlan1_deleted == 0)){ 
        del_timer_sync(timer_wlan1);
        wlan1_used = 0;
        wlan1_deleted = 1;
    }
}

//init timer when wlan interface up
void timer_ready(struct rtl8192cd_priv *priv)
{
	if ((!strcmp(priv->dev->name, "wlan0")) && (wlan0_used != 1)){
		init_timer(&priv->send_timer_wlan0);
		priv->send_timer_wlan0.expires = jiffies+RTL_SECONDS_TO_JIFFIES(5);
		priv->send_timer_wlan0.data = (unsigned long) priv;
		priv->send_timer_wlan0.function = send_roam_info;
        priv->wlanid = 0;
        timer_wlan0 = &(priv->send_timer_wlan0);
		wlan0_used = 1;
        wlan0_deleted = 1; 

 		priv->pmib->sr_profile.load_min += 10;       
	}

	else if ((!strcmp(priv->dev->name, "wlan1")) && (wlan1_used != 1)){
		init_timer(&priv->send_timer_wlan1);
		priv->send_timer_wlan1.expires = jiffies+RTL_SECONDS_TO_JIFFIES(4);
		priv->send_timer_wlan1.data = (unsigned long) priv;
		priv->send_timer_wlan1.function = send_roam_info;
        priv->wlanid = 1;
        timer_wlan1 = &(priv->send_timer_wlan1);
		wlan1_used = 1;
        wlan1_deleted = 1; 

		priv->pmib->sr_profile.signal_min += 10;
	}
	if(pid != 0){
        sr_timer_rec_add();
    }
}

//delete timer when wlan interface down
void timer_del(struct rtl8192cd_priv *priv)
{
    if (!strcmp(priv->dev->name, "wlan0")){
			del_timer_sync(&priv->send_timer_wlan0);
			wlan0_used = 0;
            wlan0_deleted = 1;
    }
    else if (!strcmp(priv->dev->name, "wlan1")){
			del_timer_sync(&priv->send_timer_wlan1);
		    wlan1_used = 0;
            wlan1_deleted = 1;              
       }
}

//netlink send msg 
void rtl_netlink_rcv(struct sk_buff *skb) 
{
	struct nlmsghdr *nlh = NULL;
	unsigned char *message;
	
	if(skb == NULL) {				
		panic_printk(KERN_INFO "%s: skb is NULL\n", __FUNCTION__);		
		return ;	
	}

	nlh=(struct nlmsghdr*)skb->data;
	//printk(KERN_INFO "kernel received msg payload:%s\n",(unsigned char*)NLMSG_DATA(nlh));

	message = NLMSG_DATA(nlh);

	if (*message == 'S')
	{
		pid = nlh->nlmsg_pid; //pid of sending process 
        sr_timer_rec_add();
    }

	
	if (*message == 'E')
	{
	    panic_printk("\nSR received delete!\n");
		sr_timer_rec_del();
	}		
	
}

//netlink send msg 
void rtl_netlink_sendmsg(int pid, struct sock *nl_sk,char *data, int data_len)
{
	struct nlmsghdr *nlh;
	struct sk_buff *skb;
	unsigned int skblen;
	unsigned char *datab;
	const char *fn;
	int err;
	
	if(data_len > MAX_PAYLOAD)
	{
		err = -ENOBUFS;		
		fn = "data_len";		
		goto msg_fail;
	}
	
	skblen = NLMSG_SPACE(data_len + 4); //+len
	skb = alloc_skb(skblen, GFP_ATOMIC);
	
	if(!skb)
	{
		err = -ENOBUFS;		
		fn = "alloc_skb";		
		goto msg_fail;
	}
	
	nlh = nlmsg_put(skb,0,0,0,skblen-sizeof(*nlh),0);
	
	if(!nlh)
	{
		err = -ENOBUFS;		
		fn = "nlmsg_put";		
		goto msg_fail_skb;
	}
	
#if defined(__LINUX_3_10__)
	NETLINK_CB(skb).portid = 0; //from kernel 
#else
	NETLINK_CB(skb).pid = 0; //from kernel 
#endif
	NETLINK_CB(skb).dst_group = 0; //unicast

	datab = NLMSG_DATA(nlh);
	memset(datab, 0, data_len+4); 
	memcpy(datab,&data_len,4); //+ total len
	memcpy(datab+4,data, data_len);

	nlh->nlmsg_len = NLMSG_HDRLEN + data_len + 4;
	
	//printk("check data_len before send=%d\n",data_len);
	err= netlink_unicast(nl_sk, skb, pid, MSG_DONTWAIT);

	if (err < 0)
	{
		fn = "nlmsg_unicast";				
		goto msg_fail;	 //nlmsg_unicast already kfree_skb 
	}
	
	return;
	
msg_fail_skb:	
	kfree_skb(skb);
	
msg_fail:
	if(msg_dropcounter < 3){
		msg_dropcounter++;
		panic_printk("[%s] drop msg: pid=%d msglen=%d %s: err=%d\n",__FUNCTION__, pid, data_len, fn, err);
	}
	return;
}

void notify_new_sta(struct rtl8192cd_priv *priv, unsigned char *mac,int type, unsigned char rssi)
{
	int offset =0;
	
	unsigned char send_buf[10]={0};
	unsigned char channel_util;
	
	if (!strcmp(priv->dev->name, "wlan0"))
		priv->wlanid = 0;
	else if (!strcmp(priv->dev->name, "wlan1"))
		priv->wlanid = 1;

	send_buf[offset] = type;
	offset+=sizeof(unsigned char);

	memcpy(send_buf+offset,&(priv->wlanid),sizeof(unsigned char));
	offset+=sizeof(unsigned char);

    memcpy(send_buf+offset,&(priv->pmib->dot11RFEntry.dot11channel),sizeof(unsigned char));
    offset+=sizeof(unsigned char);

	channel_util = 255 - priv->ext_stats.ch_utilization;
	memcpy(send_buf+offset,&(channel_util),sizeof(channel_util));
	offset+=sizeof(channel_util);	

	memcpy(send_buf+offset, mac, MACADDRLEN);
	offset += MACADDRLEN;

	memcpy(send_buf+offset,&(rssi),sizeof(rssi));
	offset+=sizeof(rssi);	    

	rtl_netlink_sendmsg(pid,rtl_smart_roaming_nl,send_buf,offset);
}

//Get neighbor channel unicast Null data rssi 
void add_neighbor_unicast_sta(struct rtl8192cd_priv *priv,unsigned char* addr, unsigned char rssi)
{
	int i, idx=-1, idx2 =0;
	unsigned char rssi_input;	
	for (i = 0; i < MAX_NEIGHBOR_STA; i++) {
		if (priv->neigbor_sta[i].used == 0) {
			if (idx < 0)
				idx = i; //search for empty entry
			continue;
		}
		if (!memcmp(priv->neigbor_sta[i].addr, addr, MACADDRLEN)) {
			idx2 = i;
			break;      // check if it is already in the list
		}
	}
	if (idx >= 0){
		rssi_input = rssi;
		memcpy(priv->neigbor_sta[idx].addr, addr, MACADDRLEN);	
		priv->neigbor_sta[idx].used = 1;		
		priv->neigbor_sta[idx].Entry = idx;   //check which entry is the neighbor sta recorded
		priv->neigbor_sta[idx].rssi = rssi_input;
		priv->NeighborStaEntryOccupied++;
		
	}
	else if (idx2){
		rssi_input = ((priv->neigbor_sta[idx2].rssi * 7)+(rssi * 3)) / 10;			
		priv->neigbor_sta[idx2].rssi = rssi_input;
		
		return;
	}
	else if (priv->NeighborStaEntryOccupied == MAX_NEIGHBOR_STA) {// sta list full, need to replace sta
			idx = priv->NeighborStaEntryNum;	
			for (i = 0; i < MAX_NEIGHBOR_STA; i++) {
				if (!memcmp(priv->neigbor_sta[i].addr, addr, MACADDRLEN))					
					return;		// check if it is already in the list			
			}
			memcpy(priv->neigbor_sta[idx].addr, addr, MACADDRLEN);		
			priv->neigbor_sta[idx].used = 1;
			priv->neigbor_sta[idx].Entry = idx;		
			priv->neigbor_sta[idx].rssi = rssi;
			priv->NeighborStaEntryNum++;			
			if( priv->NeighborStaEntryNum == MAX_NEIGHBOR_STA)	
				priv->NeighborStaEntryNum = 0; // Reset entry counter;
			return;
		}
}

//construct associated sta info 
unsigned char construct_assoc_sta(struct rtl8192cd_priv *priv,unsigned char *send_buf, int * offset)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	unsigned char	data_rate;
	unsigned char	assoc_num = 0;
	unsigned int	link_time = 0;
	unsigned int	tx_throughput = 0;
	unsigned int	rx_throughput = 0;
#ifdef SMP_SYNC
	unsigned long flags = 0;
#endif
	
	phead = &priv->asoc_list;
	
	if (list_empty(phead)) {
		return 0;
	}
	
	SMP_LOCK_ASOC_LIST(flags);
	plist = phead->next;

	//construct associated sta info
	while (plist != phead) {
		
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;		

#ifdef CONFIG_RTK_MESH
		if( isMeshPoint(pstat))
			continue;
#endif
#ifdef WDS
		if(pstat->state & WIFI_WDS)
			continue;
#endif
//printk("pstat->IOTPeer=%d\n",pstat->IOTPeer);
		if (pstat && pstat->IOTPeer == HT_IOT_PEER_RTK_APCLIENT)
		    continue;

		if(pstat->expire_to==0) // exclude expired STA
			continue;	

		memcpy(send_buf+*offset,&(pstat->hwaddr), MACADDRLEN);
		*offset += MACADDRLEN;
		
		memcpy(send_buf+*offset,&(pstat->rssi),sizeof(unsigned char));
		*offset+=sizeof(unsigned char);

		//data rate only when tx pkts increase within 5s
		if (pstat->tx_pkts > pstat->tx_pkts_pre)
			data_rate = (pstat->current_tx_rate >= VHT_RATE_ID) ? ((pstat->current_tx_rate - VHT_RATE_ID)%10):((pstat->current_tx_rate >= HT_RATE_ID)? (pstat->current_tx_rate - HT_RATE_ID) : pstat->current_tx_rate/2);
		else
			data_rate = 255; //will not trigger data_rate threshold

		memcpy(send_buf+*offset,&data_rate,sizeof(unsigned char));
		*offset+=sizeof(unsigned char);
		
		//int link_time: host byte order convert to network byte order
		link_time=htonl(pstat->link_time);			 	
		memcpy(send_buf+*offset,&(link_time),sizeof(link_time));
		*offset+=sizeof(link_time);

                //int tx_tp: host byte order convert to network byte order
                tx_throughput = htonl(pstat->tx_avarage);
                memcpy(send_buf+*offset, &(tx_throughput), sizeof(tx_throughput));
                *offset += sizeof(tx_throughput);

                //int rx_tp: host byte order convert to network byte order
                rx_throughput = htonl(pstat->rx_avarage);
                memcpy(send_buf+*offset, &(rx_throughput),sizeof(rx_throughput));
                *offset += sizeof(rx_throughput);

        assoc_num++;
	}
	SMP_UNLOCK_ASOC_LIST(flags);	
	return assoc_num;
}

//construct 11k neighbor report info
int construct_neighbor_report(struct rtl8192cd_priv *priv,unsigned char *send_buf,int * offset)
{	
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
	int i;
	
#ifdef SMP_SYNC
	unsigned long flags = 0;
#endif
	
	phead = &priv->asoc_list;
	
	if (list_empty(phead)) {
		return 0;
	}
	
	SMP_LOCK_ASOC_LIST(flags);
	plist = phead->next;
	
	while (plist != phead) {
			
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;	
		
#ifdef CONFIG_RTK_MESH
		if( isMeshPoint(pstat))
			continue;
#endif
#ifdef WDS
		if(pstat->state & WIFI_WDS)
			continue;
#endif
		if (pstat && pstat->IOTPeer == HT_IOT_PEER_RTK_APCLIENT)
		    continue;

		if(pstat->expire_to==0) // exclude expired STA
			continue;		
#if defined(DOT11K) && defined(CONFIG_IEEE80211V)
		if (pstat->rcvNeighborReport && pstat->rm.neighbor_ap_num)
		{
			//check assoc sta support 11k or 11v
			if (pstat->rm.rm_cap[0] & 0x10){
				if(pstat->bssTransSupport)
					priv->sta_flag = 11;
				else
					priv->sta_flag = 10; 
			}
							
			memcpy(send_buf+*offset,&(priv->sta_flag),sizeof(unsigned char));
			*offset+=sizeof(unsigned char);					
			
			memcpy(send_buf+*offset,&(pstat->hwaddr),MACADDRLEN);
			*offset+=MACADDRLEN;		

			memcpy(send_buf+*offset,&(pstat->rm.neighbor_ap_num),sizeof(unsigned char));
			*offset+=sizeof(unsigned char);
			
			for (i = 0 ; i < pstat->rm.neighbor_ap_num; i++)
			{		
				memcpy(send_buf+*offset,&(priv->rm_neighbor_report[i].bssid),6);
				*offset+=MACADDRLEN;				
				
				memcpy(send_buf+*offset,&(priv->rm_neighbor_report[i].subelemnt.preference),1);
				*offset+=sizeof(unsigned char);	
				
				memcpy(send_buf+*offset,&(pstat->rm.beacon_report[i].info.RCPI),1);
				*offset+=sizeof(unsigned char);	
			}

		}
#endif
	}
	SMP_UNLOCK_ASOC_LIST(flags);
	
	return 1;
}

//clear all info after send 
void clear_send_info(struct rtl8192cd_priv *priv)
{
	priv->NeighborStaEntryOccupied = 0;	
	memset(&priv->neigbor_sta, 0, sizeof(priv->neigbor_sta));
}

//construct all wlan info and send 
void construct_netlink_send(struct rtl8192cd_priv *priv)
{
	int offset=0, assoc_offset, i;
	unsigned int neighbor_sta_num, assoc_sta_num, sta_num;
	unsigned char send_buf[2048]={0};
	unsigned char channel_util;
		
	send_buf[offset] = WLAN_STA_INFO;
	offset+=sizeof(unsigned char);
	if (!strcmp(priv->dev->name, "wlan0"))
		priv->wlanid = 0;
	else if (!strcmp(priv->dev->name, "wlan1"))
		priv->wlanid = 1;
	memcpy(send_buf+offset,&(priv->wlanid),sizeof(unsigned char));
	offset+=sizeof(unsigned char);

	memcpy(send_buf+offset,&(priv->pmib->dot11RFEntry.dot11channel),sizeof(unsigned char));
	offset+=sizeof(unsigned char);

	channel_util = 255 - priv->ext_stats.ch_utilization;
	memcpy(send_buf+offset,&(channel_util),sizeof(channel_util));
	offset+=sizeof(channel_util);	
	
	neighbor_sta_num = priv->NeighborStaEntryOccupied;
	sta_num = htonl(neighbor_sta_num);
	memcpy(send_buf+offset,&sta_num,sizeof(neighbor_sta_num));
	offset+=sizeof(sta_num);	

	//construct neighbor unicast table info
	for (i = 0; i < neighbor_sta_num; i++)
	{
		memcpy(send_buf+offset,&(priv->neigbor_sta[i].addr) ,MACADDRLEN);
		offset += MACADDRLEN;	

        send_buf[offset] = priv->neigbor_sta[i].rssi;
        offset++;		
	}
	
	assoc_offset = offset;
	offset += sizeof(assoc_sta_num);
	assoc_sta_num = construct_assoc_sta(priv,send_buf,&offset);
	assoc_sta_num = htonl(assoc_sta_num);
	memcpy(send_buf+assoc_offset,&assoc_sta_num,sizeof(unsigned int));

	construct_neighbor_report(priv,send_buf,&offset);

	//send all wlan info via netlink
	rtl_netlink_sendmsg(pid,rtl_smart_roaming_nl,send_buf,offset);
}


int rtl_netlink_init(void) 
{

#if defined(__LINUX_3_10__)
	struct netlink_kernel_cfg cfg = {
		.input = rtl_netlink_rcv,
	};

	rtl_smart_roaming_nl = netlink_kernel_create(&init_net, NETLINK_RTK, &cfg);
#else	
	rtl_smart_roaming_nl = netlink_kernel_create(&init_net, NETLINK_RTK, 0, rtl_netlink_rcv, NULL, THIS_MODULE);
#endif
		
	if(!rtl_smart_roaming_nl)
	{
		panic_printk(KERN_ERR "rtl_smart_roaming_nl: Cannot create netlink socket");
		return -ENOMEM;
	}
	
	return 0;
}

void rtl_netlink_exit(void) 
{
	netlink_kernel_release(rtl_smart_roaming_nl);
    pid = 0;	
}

__inline__ static int smart_roaming_block_mac_hash(unsigned char *networkAddr, int hash_size)
{
    unsigned long x;

    x = networkAddr[0] ^ networkAddr[1] ^ networkAddr[2] ^ networkAddr[3] ^ networkAddr[4] ^ networkAddr[5];

    return x & (hash_size - 1);
}
__inline__ static void smart_roaming_block_mac_hash_link(struct smart_roaming_block_link_list *link, struct smart_roaming_block_link_list **head)
{
    link->next_hash = *head;

    if (link->next_hash != NULL)
        link->next_hash->pprev_hash = &link->next_hash;
    *head = link;
    link->pprev_hash = head;
}
__inline__ static void smart_roaming_block_mac_hash_unlink(struct smart_roaming_block_link_list *link)
{
    *(link->pprev_hash) = link->next_hash;
    if (link->next_hash != NULL)
        link->next_hash->pprev_hash = link->pprev_hash;
    link->next_hash = NULL;
    link->pprev_hash = NULL;
}


void smart_roaming_block_init(struct rtl8192cd_priv *priv)
{
	priv->sr_block.sr_block_status = 1;

    /* memory allocated structure*/
    priv->sr_block.sr_block_ent = (struct smart_roaming_block_entry *)
                kmalloc((sizeof(struct smart_roaming_block_entry) * SMART_ROAMING_BLOCK_MAX_NUM), GFP_ATOMIC);
    if (!priv->sr_block.sr_block_ent) {
        panic_printk(KERN_ERR "Can't kmalloc for smart_roaming_block_entry (size %d)\n", sizeof(struct smart_roaming_block_entry) * SMART_ROAMING_BLOCK_MAX_NUM);
        goto err;
    }          
    memset(priv->sr_block.sr_block_machash, 0, sizeof(priv->sr_block.sr_block_machash));
    memset(priv->sr_block.sr_block_ent, 0, sizeof(struct smart_roaming_block_entry) * SMART_ROAMING_BLOCK_MAX_NUM);

#ifdef SMP_SYNC
    spin_lock_init(&(priv->sr_block.sr_block_lock));
#endif

    return;

err:
    if(priv->sr_block.sr_block_ent) 
        kfree(priv->sr_block.sr_block_ent);

    return;
}

void smart_roaming_block_deinit(struct rtl8192cd_priv *priv)
{
	priv->sr_block.sr_block_status = 0;

	if(priv->sr_block.sr_block_ent)
		kfree(priv->sr_block.sr_block_ent);              
}

static struct smart_roaming_block_entry *smart_roaming_block_lookup(struct rtl8192cd_priv *priv, unsigned char *mac)
{
    unsigned long offset;
    int hash;
    struct smart_roaming_block_link_list	*link;
    struct smart_roaming_block_entry		*ent;

    offset = (unsigned long)(&((struct smart_roaming_block_entry *)0)->link_list);
    hash = smart_roaming_block_mac_hash(mac, SMART_ROAMING_BLOCK_HASH_SIZE);
    link = priv->sr_block.sr_block_machash[hash];
    while (link != NULL)
    {
        ent = (struct smart_roaming_block_entry *)((unsigned long)link - offset);
        if (ent->used && !memcmp(ent->mac, mac, MACADDRLEN))
        {
            return ent;
        }
        link = link->next_hash;
    }

    return NULL;
}

void smart_roaming_block_add(struct rtl8192cd_priv *priv, unsigned char *mac)
{
    struct smart_roaming_block_entry * ent = NULL;
    int i, hash;

    unsigned long flags;    
    SAVE_INT_AND_CLI(flags);
#ifdef SMP_SYNC  
	SMP_LOCK_SR_BLOCK_LIST(flags);
#endif
    for (i=0; i<SMART_ROAMING_BLOCK_MAX_NUM; i++)
    {
        if (!priv->sr_block.sr_block_ent[i].used)
        {
            ent = &(priv->sr_block.sr_block_ent[i]);
            break;
        }
    }

    if(ent)
    {
		ent->used = 1;
        memcpy(ent->mac, mac, MACADDRLEN);
        ent->aging = priv->pmib->sr_profile.block_aging;
        hash = smart_roaming_block_mac_hash(mac, SMART_ROAMING_BLOCK_HASH_SIZE);
        smart_roaming_block_mac_hash_link(&(ent->link_list), &(priv->sr_block.sr_block_machash[hash]));
#ifdef SMP_SYNC 
		SMP_UNLOCK_SR_BLOCK_LIST(flags);
#endif
		RESTORE_INT(flags); 		   
        return;
    }
#ifdef SMP_SYNC
	SMP_UNLOCK_SR_BLOCK_LIST(flags);
#endif
	RESTORE_INT(flags); 		   
    return;
}

void smart_roaming_block_expire(struct rtl8192cd_priv *priv, unsigned char *mac)
{
	int i;
	unsigned long offset;
	struct smart_roaming_block_link_list *link, *temp_link;
	struct smart_roaming_block_entry * ent;

	unsigned long flags = 0;	
	SAVE_INT_AND_CLI(flags);
#ifdef SMP_SYNC
	SMP_LOCK_SR_BLOCK_LIST(flags);
#endif	
	offset = (unsigned long)(&((struct smart_roaming_block_entry *)0)->link_list);

	for (i=0; i<SMART_ROAMING_BLOCK_HASH_SIZE; i++)
	{
		link = priv->sr_block.sr_block_machash[i];
		while (link != NULL)
		{
			temp_link = link->next_hash;
			ent = (struct smart_roaming_block_entry *)((unsigned long)link - offset);
			if(ent->used && !mac && ent->aging)
			{
				ent->aging -= 1;
				if(ent->aging == 0)
				{
					ent->used = 0;
					smart_roaming_block_mac_hash_unlink(link);
				}
			}
			else if(ent->used && mac && !memcmp(ent->mac, mac, MACADDRLEN)){
				ent->used = 0;
				smart_roaming_block_mac_hash_unlink(link);
			}			
			link = temp_link;
		}
	}
	
	RESTORE_INT(flags);
#ifdef SMP_SYNC
	SMP_UNLOCK_SR_BLOCK_LIST(flags);	
#endif
}

unsigned char smart_roaming_block_check_request(struct rtl8192cd_priv *priv, unsigned char *mac)
{
    struct smart_roaming_block_entry	*block_ent;
    unsigned char ret = 0;
    unsigned long flags = 0;
	
    SAVE_INT_AND_CLI(flags);
#ifdef SMP_SYNC
    SMP_LOCK_SR_BLOCK_LIST(flags);
#endif	
    block_ent =  smart_roaming_block_lookup(priv, mac);
    if(block_ent && block_ent->used && block_ent->aging)
		ret = 1;
	
    RESTORE_INT(flags);
#ifdef SMP_SYNC
	SMP_UNLOCK_SR_BLOCK_LIST(flags);
#endif
    return ret;
}


