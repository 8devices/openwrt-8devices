/*
 *      Handling routines for Mesh in 802.11 SME (Station Management Entity)
 *
 *      PS: All extern function in ../8190n_headers.h
 */
#define _MESH_ROUTE_C_

#ifdef CONFIG_RTL8192CD
#include "../8192cd.h"
#include "../8192cd_headers.h"
#else
#include "../rtl8190/8190n.h"
#include "../rtl8190/8190n_headers.h"
#endif
#include "./mesh_route.h"


#ifdef CONFIG_RTK_MESH

unsigned short getMeshSeq(DRV_PRIV *priv)
{
#ifndef SMP_SYNC
    unsigned long flags;
#endif
    SAVE_INT_AND_CLI(flags);
    if(priv->pshare->meshare.seq == 0xffff)
        priv->pshare->meshare.seq = 1;
    else
        priv->pshare->meshare.seq++;
    RESTORE_INT(flags);
    return priv->pshare->meshare.seq;
}

unsigned short getMeshMCastSeq(DRV_PRIV *priv)
{
#ifndef SMP_SYNC
    unsigned long flags;
#endif
    SAVE_INT_AND_CLI(flags);
    if(priv->pshare->meshare.seqMcast == 0xffff)
        priv->pshare->meshare.seqMcast = 1;
    else
        priv->pshare->meshare.seqMcast++;
    RESTORE_INT(flags);
    return priv->pshare->meshare.seqMcast;
}


unsigned short getMeshMulticastSeq(DRV_PRIV *priv)
{
#ifndef SMP_SYNC
    unsigned long flags;
#endif
    SAVE_INT_AND_CLI(flags);

    if(priv->seqNum == 0xffff)
        priv->seqNum = 1;
    else
        priv->seqNum++;
    RESTORE_INT(flags);
    return priv->seqNum;
}


//pepsi
#ifdef PU_STANDARD
UINT8 getPUSeq(DRV_PRIV *priv)
{
#ifndef SMP_SYNC
    unsigned long flags;
#endif
    SAVE_INT_AND_CLI(flags);
    if(priv->pshare->meshare.PUseq == 0xff)
        priv->pshare->meshare.PUseq = 1U;
    else
        priv->pshare->meshare.PUseq++;
    RESTORE_INT(flags);
    return priv->pshare->meshare.PUseq;
}
#endif


// return 0: duplicate
// return 1: ok
unsigned short chkMeshSeq(struct path_sel_entry *pPathselEntry, unsigned short seq)
{
    unsigned short idx = seq & (SZ_HASH_IDX2 - 1);

    if(seq == pPathselEntry->RecentSeq[idx]) {
        return 0;

    }

    pPathselEntry->RecentSeq[idx] = seq;
    return 1;
   
}

/*return value 0: succeed, other: fail*/
int insert_PREQ_entry(unsigned char *targetMac, DRV_PRIV *priv)
{
    struct mesh_rreq_retry_entry rreqEntry;
    struct mesh_rreq_retry_entry *retryEntry;
    int ret = 0;
    unsigned long flags;
    SAVE_INT_AND_CLI(flags);
    SMP_LOCK_MESH_PREQ(flags);

    retryEntry= (struct mesh_rreq_retry_entry*) HASH_SEARCH(priv->mesh_rreq_retry_queue, targetMac);
    if(	retryEntry== NULL)
    {
        rreqEntry.TimeStamp = rreqEntry.createTime = jiffies;		
        rreqEntry.Retries = 0;
        memcpy(rreqEntry.MACAddr, targetMac, MACADDRLEN);		
        rreqEntry.pktqueue.head = rreqEntry.pktqueue.tail = 0;
        if(HASH_TABLE_SUCCEEDED != HASH_INSERT(priv->mesh_rreq_retry_queue, targetMac, &rreqEntry)) {
            ret = 1;
            goto ret;
        }

        mesh_route_debug("PREQ to find %02x:%02x:%02x:%02x:%02x:%02x queued at %lu\n",
                targetMac[0],targetMac[1],targetMac[2],targetMac[3],targetMac[4],targetMac[5],jiffies);  

        retryEntry = (struct mesh_rreq_retry_entry*) HASH_SEARCH(priv->mesh_rreq_retry_queue, targetMac);


        if(priv->rreq_head == NULL) {           
            retryEntry->rreq_prev = retryEntry->rreq_next =  NULL;
            priv->rreq_head = priv->rreq_tail = retryEntry;
        }
        else {
            retryEntry->rreq_next = NULL;
            retryEntry->rreq_prev = priv->rreq_tail;
            priv->rreq_tail->rreq_next = retryEntry;
            priv->rreq_tail = retryEntry;
        }      
    }
    else
    {
        mesh_route_debug("PREQ to find %02x:%02x:%02x:%02x:%02x:%02x refreshed at %lu\n",
                retryEntry->MACAddr[0],retryEntry->MACAddr[1],retryEntry->MACAddr[2],retryEntry->MACAddr[3],retryEntry->MACAddr[4],retryEntry->MACAddr[5],jiffies);
        retryEntry->TimeStamp=retryEntry->createTime=jiffies;		
        retryEntry->Retries = 0;					
    }
    
ret:
    RESTORE_INT(flags);
    SMP_UNLOCK_MESH_PREQ(flags);
    
    return ret;
}

void GEN_PREQ_PACKET(char *targetMac, DRV_PRIV *priv)
{
    DOT11s_GEN_RREQ_PACKET rreq_event;
    memset((void*)&rreq_event, 0x0, sizeof(DOT11s_GEN_RREQ_PACKET));
    rreq_event.EventId = DOT11_EVENT_PATHSEL_GEN_RREQ;	
    rreq_event.IsMoreEvent = 0;
    memcpy(rreq_event.MyMACAddr,  GET_MY_HWADDR ,MACADDRLEN);
    memcpy(rreq_event.destMACAddr,  targetMac ,MACADDRLEN);
    rreq_event.TTL = _MESH_HEADER_TTL_;
    rreq_event.Seq_num = getMeshSeq(priv);
    DOT11_EnQueue2((unsigned long)priv, priv->pathsel_queue, (unsigned char*)&rreq_event, sizeof(DOT11s_GEN_RREQ_PACKET));
    notifyPathSelection(priv);
}
		   

//modify by Joule for MESH HEADER
unsigned char* getMeshHeader(DRV_PRIV *priv, int wep_mode, unsigned char* pframe)
{
	INT		payload_offset;
	struct wlan_llc_t      *e_llc;
	struct wlan_snap_t     *e_snap;
	int wlan_pkt_format = WLAN_PKT_FORMAT_OTHERS;

	payload_offset = get_hdrlen(priv, pframe);

	if (GetPrivacy(pframe)) {
		if (((wep_mode == _WEP_40_PRIVACY_) || (wep_mode == _WEP_104_PRIVACY_))) {
			payload_offset += 4;
		}
		else if ((wep_mode == _TKIP_PRIVACY_) || (wep_mode == _CCMP_PRIVACY_)) {
			payload_offset += 8;
		}
		else {
			DEBUG_ERR("unallowed wep_mode privacy=%d\n", wep_mode);
			return NULL;
		}
	}

	e_llc = (struct wlan_llc_t *) (pframe + payload_offset);
	e_snap = (struct wlan_snap_t *) (pframe + payload_offset + sizeof(struct wlan_llc_t));

	if (e_llc->dsap==0xaa && e_llc->ssap==0xaa && e_llc->ctl==0x03) {

		if ( !memcmp(e_snap->oui, oui_rfc1042, WLAN_IEEE_OUI_LEN)) {
			wlan_pkt_format = WLAN_PKT_FORMAT_SNAP_RFC1042;
			if( !memcmp(&e_snap->type, SNAP_ETH_TYPE_IPX, 2) )
				wlan_pkt_format = WLAN_PKT_FORMAT_IPX_TYPE4;
			else if( !memcmp(&e_snap->type, SNAP_ETH_TYPE_APPLETALK_AARP, 2))
				wlan_pkt_format = WLAN_PKT_FORMAT_APPLETALK;
		}
		else if ( !memcmp(e_snap->oui, SNAP_HDR_APPLETALK_DDP, WLAN_IEEE_OUI_LEN) &&
					!memcmp(&e_snap->type, SNAP_ETH_TYPE_APPLETALK_DDP, 2) )
				wlan_pkt_format = WLAN_PKT_FORMAT_APPLETALK;
		else if ( !memcmp( e_snap->oui, oui_8021h, WLAN_IEEE_OUI_LEN))
			wlan_pkt_format = WLAN_PKT_FORMAT_SNAP_TUNNEL;
	}

	if ( (wlan_pkt_format == WLAN_PKT_FORMAT_SNAP_RFC1042)
			|| (wlan_pkt_format == WLAN_PKT_FORMAT_SNAP_TUNNEL) ) {
		payload_offset +=  sizeof(struct wlan_llc_t) + sizeof(struct wlan_snap_t);
	}

	return pframe+payload_offset;
}

void notifyPathSelection(DRV_PRIV *priv)
{ 
        struct task_struct *p;
        
        if(priv->pid_pathsel != 0){
                read_lock(&tasklist_lock); 
                p = find_task_by_vpid(priv->pid_pathsel);
                read_unlock(&tasklist_lock);
                if(p)
                {
                        // printk("send signal from kernel\n");
                        send_sig(SIGUSR1,p,0); 
                }
                else {
                        priv->pid_pathsel = 0;
                }
        }
}


/*
 *	@brief	MESH MP time aging expire
 *
 *	@param	task_priv: priv
 *
 *	@retval	void
 */
 // chuangch 10.19
 #ifdef MESH_ROUTE_MAINTENANCE
 void route_maintenance(DRV_PRIV *priv)
 { 
    const int tbl_sz = 1 << priv->pathsel_table->table_size_power;
    int i;
    unsigned long now = jiffies, time_diff;
    unsigned long flags;
    int is_maintain;
    unsigned char destMAC[MACADDRLEN];

    for (i = 0; i < tbl_sz; i++){
        is_maintain = 0;
        SAVE_INT_AND_CLI(flags);
        SMP_LOCK_MESH_PATH(flags);
        if(priv->pathsel_table->entry_array[i].dirty){
            struct path_sel_entry *entry = (struct path_sel_entry*)priv->pathsel_table->entry_array[i].data;

            time_diff = RTL_JIFFIES_TO_SECOND(now - entry->routeMaintain);
            if(((entry->metric > ((int)(entry->hopcount))<<8 )
                && (time_diff > HWMP_PREQ_REFRESH_PERIOD ))
                || (time_diff > HWMP_PREQ_REFRESH_PERIOD2))
            {				
                entry->routeMaintain = jiffies;
                memcpy(destMAC, entry->destMAC, MACADDRLEN);  
                is_maintain = 1;
            }
        }
        RESTORE_INT(flags);
        SMP_UNLOCK_MESH_PATH(flags);

        if(is_maintain) {
            GEN_PREQ_PACKET(destMAC, priv);
            insert_PREQ_entry(destMAC, priv);
        }
    }
}
 #endif
 
void aodv_expire(void *task_priv)
{
    DRV_PRIV *priv = (DRV_PRIV *)task_priv;
    struct sk_buff *pskb;
    struct mesh_rreq_retry_entry *retryEntry;
    
    unsigned long flags;
    #ifdef SMP_SYNC
    unsigned long path_flag;
    #endif
    SAVE_INT_AND_CLI(flags);
    SMP_LOCK_MESH_PREQ(flags);

    retryEntry = priv->rreq_head;
    while(retryEntry) 
    {
        if(time_after(jiffies, (UINT32)(retryEntry->TimeStamp)+ HWMP_NETDIAMETER_TRAVERSAL_TIME)) {
            if (retryEntry->Retries > HWMP_MAX_PREQ_RETRIES )
            {
                mesh_route_debug("PREQ to find %02x:%02x:%02x:%02x:%02x:%02x exceed retry limit at %lu\n",
                                retryEntry->MACAddr[0],retryEntry->MACAddr[1],retryEntry->MACAddr[2],retryEntry->MACAddr[3],retryEntry->MACAddr[4],retryEntry->MACAddr[5],jiffies);

                pskb=(struct sk_buff*)deque(priv,&(retryEntry->pktqueue.head),&(retryEntry->pktqueue.tail),(unsigned int)retryEntry->pktqueue.pSkb,NUM_TXPKT_QUEUE);
                while(pskb)
                {
                    dev_kfree_skb_any(pskb);
                    pskb=(struct sk_buff*)deque(priv,&(retryEntry->pktqueue.head),&(retryEntry->pktqueue.tail),(unsigned int)retryEntry->pktqueue.pSkb,NUM_TXPKT_QUEUE);
                }
                HASH_DELETE(priv->mesh_rreq_retry_queue,retryEntry->MACAddr);

    
                if(retryEntry == priv->rreq_head) { /*head*/
                    priv->rreq_head = retryEntry->rreq_next;
                }

                if(retryEntry == priv->rreq_tail) {
                    priv->rreq_tail = retryEntry->rreq_prev;
                }

                if(retryEntry->rreq_prev) {
                    retryEntry->rreq_prev->rreq_next = retryEntry->rreq_next;
                }

                if(retryEntry->rreq_next) {
                    retryEntry->rreq_next->rreq_prev = retryEntry->rreq_prev;
                }                


#if defined(RTK_MESH_REMOVE_PATH_AFTER_AODV_TIMEOUT)
                SMP_LOCK_MESH_PATH(path_flag);
                HASH_DELETE(priv->pathsel_table,retryEntry->MACAddr);
                SMP_UNLOCK_MESH_PATH(path_flag);
                
                mesh_route_debug("Path to %02x:%02x:%02x:%02x:%02x:%02x removed at %lu\n",
                            retryEntry->MACAddr[0],retryEntry->MACAddr[1],retryEntry->MACAddr[2],retryEntry->MACAddr[3],retryEntry->MACAddr[4],retryEntry->MACAddr[5],jiffies);
#endif                
            } 
            else {
                GEN_PREQ_PACKET(retryEntry->MACAddr, priv);
                retryEntry->TimeStamp=jiffies;
                retryEntry->Retries++;

                mesh_route_debug("PREQ to find %02x:%02x:%02x:%02x:%02x:%02x retried:%d,at %lu\n",
                        retryEntry->MACAddr[0],retryEntry->MACAddr[1],retryEntry->MACAddr[2],
                        retryEntry->MACAddr[3],retryEntry->MACAddr[4],retryEntry->MACAddr[5],retryEntry->Retries,jiffies);


            } // (retryEntry->ptr!=NULL) and (not too old)
        } // if(time_after) 

        retryEntry = retryEntry->rreq_next;
    } // end of for(i=(priv->RreqBegin);i<AODV_RREQ_TABLE_SIZE;i++)


    RESTORE_INT(flags);
    SMP_UNLOCK_MESH_PREQ(flags); 
    return;
	 
 }
 

void init_mpp_pool(struct mpp_tb* pTB)
{
	int i;
	for (i = 0; i < MAX_MPP_NUM; i++) {
		pTB->pann_mpp_pool[i].flag = 0;
	}
	pTB->pool_count = 0;
}


//int pathsel_modify_table_entry( DRV_PRIV *priv, unsigned char destaddr[MACADDRLEN], struct path_sel_entry *pEntry)
int pathsel_modify_table_entry(DRV_PRIV *priv, struct path_sel_entry *pEntry)
{
    struct path_sel_entry *entry;
    unsigned long flags;
    
    /*prevent update path relative to any invalid neighbor*/
    if(get_stainfo(priv, pEntry->nexthopMAC)) {
        pEntry->priv = priv;                
    }
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
    else if(priv->mesh_priv_sc && IS_DRV_OPEN(priv->mesh_priv_sc) && 
            GET_MIB(priv->mesh_priv_sc)->dot1180211sInfo.mesh_enable &&
            get_stainfo(priv->mesh_priv_sc, pEntry->nexthopMAC)) {
        pEntry->priv = priv->mesh_priv_sc;
    }
#endif
    else {
        return -1;
    }

    SAVE_INT_AND_CLI(flags);
    SMP_LOCK_MESH_PATH(flags);
    entry = (struct path_sel_entry *)HASH_SEARCH(priv->pathsel_table, pEntry->destMAC);
    if(entry) {
        memcpy(entry, pEntry, (int)&((struct path_sel_entry*)0)->start);
        entry->update_time =  jiffies;
        entry->priv = pEntry->priv;



        mesh_route_debug("Update Path of %02x:%02x:%02x:%02x:%02x:%02x-%02x:%02x:%02x:%02x:%02x:%02x\n\tMetric from %d to %d\n\tDSN from %u tp %u\n",
                pEntry->nexthopMAC[0],pEntry->nexthopMAC[1],pEntry->nexthopMAC[2],
                pEntry->nexthopMAC[3],pEntry->nexthopMAC[4],pEntry->nexthopMAC[5],
                pEntry->destMAC[0],pEntry->destMAC[1],pEntry->destMAC[2],
                pEntry->destMAC[3],pEntry->destMAC[4],pEntry->destMAC[5],entry->metric,pEntry->metric,entry->dsn,pEntry->dsn);


        RESTORE_INT(flags);
        SMP_UNLOCK_MESH_PATH(flags);
        
        #if defined(RTL_MESH_TXCACHE)
        expire_mesh_txcache(priv, pEntry->destMAC);
        #endif

        return 0;
    }
    
    RESTORE_INT(flags);
    SMP_UNLOCK_MESH_PATH(flags);

    return -1;
}

#if defined(CONFIG_RTL_MESH_CROSSBAND)
/*
    action: 0 : refresh aging time only
                1: create proxy info if not exist and refresh aging time
                2: delete proxy info

*/
int sync_proxy_info(struct rtl8192cd_priv *priv,unsigned char *sta, unsigned char action)
{
#ifdef SMP_SYNC
    unsigned long flags;
#endif
    struct proxy_table_entry Entry, *pEntry;

    if(!IS_DRV_OPEN(priv) || GET_MIB(priv)->dot1180211sInfo.mesh_enable == 0) {
        return -1;
    }

    SMP_LOCK_MESH_PROXY(flags);    
    pEntry = HASH_SEARCH(priv->proxy_table, sta);

    if(action == 2) { //delete        
        if(pEntry) {           
            mesh_proxy_debug("[Sync]Remove Proxy table: %02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x\n",
                            pEntry->owner[0],pEntry->owner[1],pEntry->owner[2],pEntry->owner[3],pEntry->owner[4],pEntry->owner[5],
                            pEntry->sta[0],pEntry->sta[1],pEntry->sta[2],pEntry->sta[3],pEntry->sta[4],pEntry->sta[5]);
            HASH_DELETE(priv->proxy_table, sta);

            #if defined(RTL_MESH_TXCACHE)
    		priv->mesh_txcache.dirty = 0;
            #endif
        }

    }
    else if(action == 0) { //refresh aging time only
        if(pEntry) {
            pEntry->aging_time = 0;
            mesh_proxy_debug("[Sync]Refresh Proxy table: %02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x\n",
                            pEntry->owner[0],pEntry->owner[1],pEntry->owner[2],pEntry->owner[3],pEntry->owner[4],pEntry->owner[5],
                            pEntry->sta[0],pEntry->sta[1],pEntry->sta[2],pEntry->sta[3],pEntry->sta[4],pEntry->sta[5]);

        }
    }
    SMP_UNLOCK_MESH_PROXY(flags);

	return 0;
}
#endif

int set_metric_manually(DRV_PRIV *priv,unsigned char *str)
{
   	struct list_head *phead, *plist;
	struct stat_info *pstat;
    unsigned char mac_address[MACADDRLEN],i=0,*ptr,ret=-1;
    unsigned int metric=0;

	ptr = str;
    for(;i<MACADDRLEN;i++) {
		if(str[i*2] < ':' && str[i*2] > '/') {
			mac_address[i] = (str[i*2]-'0')<<4;
		} else if (str[i*2] > '`' && str[i*2] < 'g') {
            mac_address[i] = (0xa+(str[i*2]-'a'))<<4;
		} else {
            panic_printk("Invalid format!\n");
			goto set_metric_manually_errorout;
		}

        if(str[i*2+1] < ':' && str[i*2+1] > '/') {
			mac_address[i] |= (str[i*2+1]-'0');
		} else if (str[i*2+1] > '`' && str[i*2+1] < 'g') {
            mac_address[i] |= 0xa+(str[i*2+1]-'a');
		} else {
            panic_printk("Invalid format!\n");
			goto set_metric_manually_errorout;
		}
	}

	if(strstr(str,"-"))
        ptr = strstr(str,"-")+1;
    else {
        panic_printk("Invalid format!\n");
		goto set_metric_manually_errorout;
	}
	sscanf(ptr,"%d",&metric);

    panic_printk("%s %d %02x:%02x:%02x:%02x:%02x:%02x %d\n",__func__,__LINE__,
        mac_address[0],mac_address[1],mac_address[2],mac_address[3],mac_address[4],mac_address[5],metric);
	
	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		phead = &priv->mesh_mp_hdr;
		if (!netif_running(priv->dev) || list_empty(phead))
			goto set_metric_manually_errorout;

		plist = phead->next;
		while (plist != phead) {
            pstat = list_entry(plist, struct stat_info, mesh_mp_ptr);

			if(!memcmp(pstat->hwaddr,mac_address,MACADDRLEN)) {
				pstat->mesh_neighbor_TBL.manual_metric = metric;
                plist = phead;
                ret = 0;
                break;
			} else
				plist = plist->next;
		}
	} else {
		panic_printk("  Mesh mode DISABLE !!\n");
	}

set_metric_manually_errorout:
	return ret;
}

#endif	// CONFIG_RTK_MESH
