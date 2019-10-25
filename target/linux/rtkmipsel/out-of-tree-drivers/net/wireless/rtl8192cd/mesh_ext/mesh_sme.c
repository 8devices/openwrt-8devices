/*
 *      Handling routines for Mesh in 802.11 SME (Station Management Entity)
 *
 *      PS: All extern function in ../8190n_headers.h
 */
#define _MESH_SME_C_

#ifdef CONFIG_RTL8192CD
#include "../8192cd.h"
#include "../8192cd_headers.h"
#include "../8192cd_util.h"
#include "../ieee802_mib.h"
#else
#include "../rtl8190/8190n.h"
#include "../rtl8190/8190n_headers.h"
#include "../rtl8190/8190n_util.h"
#include "../rtl8190/ieee802_mib.h"
#endif
#include "./mesh.h"
#include "./mesh_route.h"
#ifdef MESH_USE_METRICOP
#include "mesh_11kv.h"
#endif

#ifdef CONFIG_RTK_MESH

/**
 *	@brief	Mesh State transitions table
 *	Usage: PeerLink_states_table[currentState][currentEvent]
 *	Structure: 1.CancelPeerLink 2.PassivePeerLinkOpen 3.ActivePeerLinkOpen 4.CloseReceived
 *			5.OpenReceived 6.ConfirmReceived 7,Timeout(include Retry Open Cancel)
 *	PS1: With mesh.h MESH_PEER_LINK_EVENT synchronic
 *	PS2: These state is normal transitions,  "BUT" like FAIL transitions SHALL BE modify state in code !!
**/
const UINT8 mesh_PeerLinkStatesTable[][7] =
{
    // state 0:MP_UNUSED
    {MP_UNUSED,	 MP_LISTEN, MP_OPEN_SENT, MP_UNUSED, MP_UNUSED, MP_UNUSED, MP_UNUSED},
    // state 1:MP_LISTEN
    {MP_UNUSED, MP_LISTEN, MP_OPEN_SENT, MP_LISTEN, MP_CONFIRM_SENT, MP_LISTEN, MP_LISTEN},
    // state 2:MP_OPEN_SENT
    {MP_HOLDING, MP_OPEN_SENT,  MP_OPEN_SENT, MP_HOLDING, MP_CONFIRM_SENT, MP_CONFIRM_RCVD, MP_OPEN_SENT},
    // state 3:MP_CONFIRM_RCVD
    {MP_HOLDING, MP_CONFIRM_RCVD, MP_CONFIRM_RCVD, MP_HOLDING, MP_SUBORDINATE_LINK_DOWN_E, MP_CONFIRM_RCVD,  MP_HOLDING},
    // state 4:MP_CONFIRM_SENT
    {MP_HOLDING, MP_CONFIRM_SENT, MP_CONFIRM_SENT, MP_HOLDING, MP_CONFIRM_SENT, MP_SUBORDINATE_LINK_DOWN_E, MP_CONFIRM_SENT},
    // state 5:MP_SUBORDINATE_LINK_DOWN_E
    {MP_HOLDING, MP_SUBORDINATE_LINK_DOWN_E, MP_SUBORDINATE_LINK_DOWN_E, MP_HOLDING, MP_SUBORDINATE_LINK_DOWN_E, MP_SUBORDINATE_LINK_DOWN_E, MP_SUBORDINATE_LINK_DOWN_E},
    // state 6:MP_SUBORDINATE_LINK_UP
    {MP_HOLDING, MP_SUBORDINATE_LINK_UP, MP_SUBORDINATE_LINK_UP, MP_HOLDING, MP_SUBORDINATE_LINK_UP, MP_SUBORDINATE_LINK_UP, MP_SUBORDINATE_LINK_UP},
    // state 7:MP_SUPERORDINATE_LINK_DOWN
    {MP_HOLDING, MP_SUPERORDINATE_LINK_DOWN, MP_SUPERORDINATE_LINK_DOWN, MP_HOLDING, MP_SUPERORDINATE_LINK_DOWN, MP_SUPERORDINATE_LINK_DOWN, MP_SUPERORDINATE_LINK_DOWN},
    // state 8:MP_SUPERORDINATE_LINK_UP
    {MP_HOLDING, MP_SUPERORDINATE_LINK_UP, MP_SUPERORDINATE_LINK_UP, MP_HOLDING, MP_SUPERORDINATE_LINK_UP, MP_SUPERORDINATE_LINK_UP, MP_SUPERORDINATE_LINK_UP},
    // state 9:MP_HOLDING
    {MP_HOLDING, MP_HOLDING, MP_HOLDING, MP_HOLDING, MP_HOLDING, MP_HOLDING, MP_UNUSED}
};

int init_mesh(DRV_PRIV *priv)
{
    init_timer(&priv->mesh_peer_link_timer);
    priv->mesh_peer_link_timer.data = (unsigned long) priv;
    priv->mesh_peer_link_timer.function = mesh_peer_link_timer;

#ifdef MESH_BOOTSEQ_AUTH
    init_timer(&priv->mesh_auth_timer);
    priv->mesh_auth_timer.data = (unsigned long) priv;
    priv->mesh_auth_timer.function = mesh_auth_timer;
#endif

    priv->mesh_Version = 1;
#ifdef MESH_ESTABLISH_RSSI_THRESHOLD
    priv->mesh_fake_mib.establish_rssi_threshold = DEFAULT_ESTABLISH_RSSI_THRESHOLD;
#endif

#ifdef MESH_USE_METRICOP
    // in next version, the fake_mib related values will be actually recorded in MIB
    priv->mesh_fake_mib.metricID = 1; // 0: very old version,  1: version before 2009/3/10,  2: 11s
    priv->mesh_fake_mib.isPure11s = 0;
    priv->mesh_fake_mib.intervalMetricAuto = RTL_SECONDS_TO_JIFFIES(60); // 1 Mins
    priv->mesh_fake_mib.spec11kv.defPktTO = RTL_SECONDS_TO_JIFFIES(2); // 2 * 100 = 2 secs
    priv->mesh_fake_mib.spec11kv.defPktLen = 1024; // bt=8196 bits
    priv->mesh_fake_mib.spec11kv.defPktCnt = 2;
    priv->mesh_fake_mib.spec11kv.defPktPri = 5;

    // once driver starts up, toMeshMetricAuto will be updated
    // (I think it might be put in "init one" to match our original concept...)
    priv->toMeshMetricAuto = jiffies + priv->mesh_fake_mib.intervalMetricAuto;
#endif // MESH_USE_METRICOP

    if (MAX_MPP_NUM < GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor)
        GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor = MAX_MPP_NUM;	// Exceed hard code MAX number, Lock on hardcode MAX number
    MESH_PEER_LINK_CAP_NUM(priv) = GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor;

    priv->mesh_PeerCAP_flags = 0x80;		// Bit15(Operation as MP) shall be "1"
    priv->mesh_HeaderFlags = 0; 			// NO Address Extension

    // The following info can be saved by FLASH in the future
    priv->mesh_profile[0].used = TRUE;
    priv->mesh_profile[0].PathSelectMetricID.value = 0;
    priv->mesh_profile[0].PathSelectMetricID.OUI[0] = 0x00;
    priv->mesh_profile[0].PathSelectMetricID.OUI[1] = 0x0f;
    priv->mesh_profile[0].PathSelectMetricID.OUI[2] = 0xac;


#ifdef CONFIG_RTL_MESH_CROSSBAND
    priv->mesh_profile[0].PathSelectProtocolID.value = 1;
#else
    priv->mesh_profile[0].PathSelectProtocolID.value = 0;
#endif
    priv->mesh_profile[0].PathSelectProtocolID.OUI[0] = 0x00;
    priv->mesh_profile[0].PathSelectProtocolID.OUI[1] = 0x0f;
    priv->mesh_profile[0].PathSelectProtocolID.OUI[2] = 0xac;

    priv->mesh_fix_channel = 0;

    return 0;
}


static void updateMeshMetric(DRV_PRIV *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo)
{
    /*
    // Update oneself peer pstat
    pstat->mesh_neighbor_TBL.r = getDataRate(pstat);
    pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_LocalLinkStateANNOU_TO;
    */
#ifdef RTK_MESH_METRIC_REFINE
    pstat->mesh_neighbor_TBL.rssi_metric = pfrinfo->rssi;
	pstat->mesh_neighbor_TBL.cu_metric = priv->envinfo.cu_metric;
	pstat->mesh_neighbor_TBL.noise_metric = priv->envinfo.noise_metric;
#else
    UINT16 prev_m = pstat->mesh_neighbor_TBL.metric >> 1;
    UINT16 cur_m = (pfrinfo->rssi<28)?(499+((28-pfrinfo->rssi)<<6)):((100-pfrinfo->rssi)<<1);
    pstat->mesh_neighbor_TBL.metric = (prev_m == 0)?cur_m: (prev_m + (cur_m>>1));
#endif
}


// I don't think pfrinfo should be one of the parameter. However, we don't have queryMeshMetric now...
static UINT16 getMeshMetric(struct stat_info *pstat)
{
    /*
    if(pstat->mesh_neighbor_TBL.metric)
    queryMeshMetric(); // TODO: send metric report request
    */
    if(pstat->mesh_neighbor_TBL.manual_metric)
    {
        return pstat->mesh_neighbor_TBL.manual_metric;
    }
    else
    {
        return pstat->mesh_neighbor_TBL.metric;
    }
}



/*
 *	@brief	MESH MP time aging expire
 *
 *	@param	task_priv: priv
 *
 *	@retval	void
 */
void peer_expire(DRV_PRIV* priv)
{
#ifndef SMP_SYNC
    unsigned long		flags;
#endif
    struct stat_info	*pstat;
    struct list_head	*phead, *plist;

    SAVE_INT_AND_CLI(flags);

    // Association MP (mesh_mp_hdr) check only.
    phead= &priv->mesh_mp_hdr;
    plist = phead->next;

    while(plist != phead)  	// 1.Check index  2.Check is it least element? (Because  next pointer to phead itself)
    {
        pstat = list_entry(plist, struct stat_info, mesh_mp_ptr); // Find process MP
        plist = plist->next;		// Pointer to next element list_head struct ( CAUTION:Cann't put behind free_stainfo ,Because free_stainfo (plist remove), don't know what pointer to?)
        if (time_after(jiffies, pstat->mesh_neighbor_TBL.expire + MESH_EXPIRE_TO_STAGE2))  		// expire !!
        {
            mesh_sme_debug("MESH neighbor expire!!! %02X:%02X:%02X:%02X:%02X:%02X\n",
                           pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],
                           pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
            cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
            mesh_cnt_ASSOC_PeerLink_CAP(priv, pstat, DECREASE);	// Delete 1 MP (Inside call path select process.. )

            clear_route_info(priv, pstat->hwaddr);	// call del pathselection table entry
            
            if (SUCCESS != free_stainfo(priv, pstat))
                break;	// free fail
        }
        else if(time_after(jiffies, pstat->mesh_neighbor_TBL.expire))
        {
            /*did not receive beacon for a while, send probe req to ask if the peer is alive*/
            mesh_sme_debug("MESH send probe req to neighbor: %02X:%02X:%02X:%02X:%02X:%02X\n",
                           pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],
                           pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
            issue_probereq_MP(priv, NULL, 0, pstat->hwaddr, TRUE);
        }
    }
    RESTORE_INT(flags);		// All of mesh_mp_hdr check complete
}

int pathsel_table_expire(DRV_PRIV* priv)
{
    int tbl_sz = 1 << priv->pathsel_table->table_size_power;
    int i;
    unsigned long now = jiffies;
    unsigned long flags;

    struct path_sel_entry* pPathEntry;
    unsigned char destMAC[MACADDRLEN];
    int is_delete;
    for (i = 0; i < tbl_sz; i++)
    {
        is_delete = 0;
        
        SAVE_INT_AND_CLI(flags);
        SMP_LOCK_MESH_PATH(flags);
        if (priv->pathsel_table->entry_array[i].dirty)
        {
            pPathEntry = ((struct path_sel_entry*)priv->pathsel_table->entry_array[i].data);
            if ((pPathEntry->metric !=99999) && (RTL_JIFFIES_TO_SECOND(now - pPathEntry->update_time)  > HWMP_ACTIVE_PATH_TIMEOUT))
            {
                priv->pathsel_table->entry_array[i].dirty = 0;
                memcpy(destMAC, pPathEntry->destMAC, MACADDRLEN);
                is_delete = 1;                
            }
        }
        RESTORE_INT(flags);
        SMP_UNLOCK_MESH_PATH(flags);


        if(is_delete) {
            #if defined(RTL_MESH_TXCACHE)
            expire_mesh_txcache(priv, destMAC);
            #endif
            remove_proxy_owner(priv, destMAC);  
        }
    }


    
    return 0;
}


/*  This function checks the pskb. If the owner of the sa is me, clean it from the proxy table.
 *
 *   Usage: When a proxied client roams to other MP, check and clean its proxy entry.
 */



int proxy_table_expire(DRV_PRIV* priv)
{
    unsigned long tbl_sz;
#ifdef SMP_SYNC    
    unsigned long flags;
#endif

    int i;
    struct proxy_table_entry* entry;

    SMP_LOCK_MESH_PROXY(flags);

    tbl_sz = 1 << priv->proxy_table->table_size_power;
    for (i = 0; i < tbl_sz; i++)
    {
        if (priv->proxy_table->entry_array[i].dirty)
        {
            entry = (struct proxy_table_entry*)priv->proxy_table->entry_array[i].data;
            if(entry->aging_time >= 0)   /*check if this proxy infor can be timeout*/
            {
                entry->aging_time++;
                if ( entry->aging_time > PROXY_TBL_AGING_LIMIT )
                {
                    priv->proxy_table->entry_array[i].dirty = 0;
                    mesh_proxy_debug("[%s %d]Expired Proxy entry of %02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x\n",
                        __func__,__LINE__, entry->owner[0],entry->owner[1],entry->owner[2],entry->owner[3],entry->owner[4],entry->owner[5],
                        entry->sta[0], entry->sta[1],entry->sta[2],entry->sta[3],entry->sta[4],entry->sta[5]);

                    #if defined(RTL_MESH_TXCACHE)
                    priv->mesh_txcache.dirty = 0;
                    #endif
                }
            }
        }
    }

    SMP_UNLOCK_MESH_PROXY(flags);
    return 0;
}

#ifdef PU_STANDARD
int proxyupdate_table_expire(DRV_PRIV* priv)
{
    unsigned long tbl_sz;
    int i;
    unsigned long now = jiffies;
    unsigned long flags;
    struct proxyupdate_table_entry *puEntry;

    SAVE_INT_AND_CLI(flags);
    SMP_LOCK_MESH_PROXYUPDATE(flags);

    tbl_sz = 1 << priv->proxyupdate_table->table_size_power;
    for (i = 0; i < tbl_sz; i++)
    {
        if ( priv->proxyupdate_table->entry_array[i].dirty )
        {
            puEntry = (struct proxyupdate_table_entry *)(priv->proxyupdate_table->entry_array[i].data);

            if(puEntry->retry < 10)
            {
                if ( RTL_JIFFIES_TO_SECOND(now - puEntry->update_time) > 1 )
                {
                    puEntry->retry++;
                    puEntry->update_time = now ;
                    issue_proxyupdate_MP(puEntry->priv, puEntry);
                }
            }
            else
                priv->proxyupdate_table->entry_array[i].dirty = 0U;
        }
    }

    RESTORE_INT(flags);
    SMP_UNLOCK_MESH_PROXYUPDATE(flags);
    return 0;
}
#endif // PU_STANDARD


#if defined(RTK_MESH_AODV_STANDALONE_TIMER)
void mesh_standalone_timer_expire(DRV_PRIV* priv)
{
    aodv_expire(priv);
    mod_timer(&priv->mesh_expire_timer, jiffies + MESH_AODV_EXPIRE_TO);
}
#endif


void static mesh_channel_switch_expire(DRV_PRIV *priv)
{
    if(priv->mesh_swchnl_channel)
    {
        if(priv->mesh_swchnl_counter)
            priv->mesh_swchnl_counter--;

        if(priv->mesh_swchnl_counter == 0)
        {
            if(priv->mesh_swchnl_flag & BIT1)  /*channel switch initiator*/
            {
                priv->mesh_ChannelPrecedence = priv->mesh_swchnl_precedence;
#ifdef UNIVERSAL_REPEATER
                start_clnt_auth(GET_VXD_PRIV(priv));
#endif
                priv->mesh_swchnl_channel = 0;/*set to zero to end the channel switch procedure*/
            }
            else
            {
                priv->pmib->dot11RFEntry.dot11channel = priv->mesh_swchnl_channel;
                if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40)
                {
                    if(priv->mesh_swchnl_offset != HT_2NDCH_OFFSET_DONTCARE)
                    {
                        priv->pshare->offset_2nd_chan = priv->mesh_swchnl_offset;
                        priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = priv->mesh_swchnl_offset;
                    }

#if defined(RTK_5G_SUPPORT)
                    if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
                    {
                        if((priv->pmib->dot11RFEntry.dot11channel>144) ? ((priv->pmib->dot11RFEntry.dot11channel-1)%8) : (priv->pmib->dot11RFEntry.dot11channel%8))
                        {
                            priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_ABOVE;
                            priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;
                        }
                        else
                        {
                            priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_BELOW;
                            priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
                        }
                    }
                    else
#endif
                    {
                        if ((priv->pmib->dot11RFEntry.dot11channel < 5) &&
                                (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW))
                        {
                            priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;
                            priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_ABOVE;
                        }
                        else if ((priv->pmib->dot11RFEntry.dot11channel > 9) &&
                                 (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_ABOVE))
                        {
                            priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
                            priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_BELOW;
                        }
                    }

                }

                priv->mesh_ChannelPrecedence = priv->mesh_swchnl_precedence;
                priv->mesh_swchnl_channel = 0;/*set to zero to end the channel switch procedure*/

                ap_sync_chan_to_bss(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
                SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan); // in this version, automatically establishing link
                SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);


            }
        }
    }
}

int static mesh_channel_switch_check(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
    unsigned char		*pframe, *p;
    int len;
    UINT8 swchnl_channel = 0;
    UINT8 swchnl_offset = 0;
    UINT8 swchnl_ttl = 0;
    UINT8 swchnl_flag = 0;
    UINT16 swchnl_reason;
    UINT32 swchnl_precedence = 0;
    int ret = 0;
    pframe = get_pframe(pfrinfo);

    // check  Channel Switch Announcement
    p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _CSA_IE_, &len,
               pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
    if (p)
    {
        swchnl_channel = *(p + MESH_IE_BASE_LEN + 1);
        // check  Secondary Channel Offset
        p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _SECONDARY_CHANNEL_OFFSET_IE_, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
        if (p)
        {
            swchnl_offset = *(p + MESH_IE_BASE_LEN);
        }

        // check Mesh Channel Switch
        p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _MESH_CHANNEL_SWITCH_IE_, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
        if (p)
        {
            swchnl_ttl = *(p + MESH_IE_BASE_LEN);
            swchnl_flag = *(p + MESH_IE_BASE_LEN + 1);
            memcpy(&swchnl_reason, p + MESH_IE_BASE_LEN + 2, 2);
            swchnl_reason = le16_to_cpu(swchnl_reason);
            swchnl_precedence = le32_to_cpu(*((UINT32*)(p+MESH_IE_BASE_LEN + 4)));
        }
    }

    if(swchnl_channel && swchnl_precedence && swchnl_ttl > 0)
    {
        /*if we got Channel Switch Annoucement and Mesh Channel Switch ie and ttl is greater than zero*/
        if((priv->mesh_swchnl_precedence != swchnl_precedence) && /*if this channel switch procedure is different from previous one*/
                (priv->mesh_swchnl_channel == 0 || (swchnl_precedence > priv->mesh_swchnl_precedence)))
        {
            /*if the channel switch procedure is not on-going or the precedence is greater than previous recieved one*/

            /*start channel swtich procedure*/
            priv->mesh_swchnl_channel = swchnl_channel;
            priv->mesh_swchnl_offset = swchnl_offset;
            priv->mesh_swchnl_ttl = swchnl_ttl - 1;
            priv->mesh_swchnl_flag = swchnl_flag & ~BIT1; /*erase initiator bit*/
            priv->mesh_swchnl_reason = swchnl_reason;
            priv->mesh_swchnl_precedence = swchnl_precedence;
            priv->mesh_swchnl_counter = MESH_CHANNEL_SWITCH_COUNTER;

            if(priv->auto_channel == 0)
            {
                update_beacon(priv);
            }

            ret = 1;
        }
    }
    return ret;
}

int mesh_channel_switch_initiate(struct rtl8192cd_priv *priv)
{

    int bss_channel, bss_bw, bss_offset = 0;
    struct rtl8192cd_priv * priv_root = GET_ROOT(priv);
    if(GET_MIB(priv_root)->dot1180211sInfo.mesh_enable)
    {
        if(priv_root->mesh_swchnl_channel == 0)  /*no channel switch procedure is on-going*/
        {

            bss_bw = HT_CHANNEL_WIDTH_20_40;
#ifdef RTK_AC_SUPPORT
            if(priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)          
            {
                if((priv->pmib->dot11Bss.t_stamp[1] & (BSS_BW_MASK << BSS_BW_SHIFT))
                        == (HT_CHANNEL_WIDTH_80 << BSS_BW_SHIFT))
                    bss_bw = HT_CHANNEL_WIDTH_80;
            }
#endif

            bss_channel = priv->pmib->dot11Bss.channel;
            if ((priv->pmib->dot11Bss.t_stamp[1] & (BIT(1) | BIT(2))) == (BIT(1) | BIT(2)))
                bss_offset = HT_2NDCH_OFFSET_BELOW;
            else if ((priv->pmib->dot11Bss.t_stamp[1] & (BIT(1) | BIT(2))) == BIT(1))
                bss_offset = HT_2NDCH_OFFSET_ABOVE;
            else
            {
                bss_bw = HT_CHANNEL_WIDTH_20;
                bss_offset = HT_2NDCH_OFFSET_DONTCARE;
            }


            if ((priv_root->pmib->dot11RFEntry.dot11channel != bss_channel) ||
                    (priv_root->pmib->dot11nConfigEntry.dot11nUse40M != bss_bw) ||
                    (priv_root->pmib->dot11nConfigEntry.dot11n2ndChOffset != bss_offset)  )
            {

                priv_root->mesh_swchnl_channel = bss_channel;
                priv_root->mesh_swchnl_offset = bss_offset;
                priv_root->mesh_swchnl_ttl = _MESH_HEADER_TTL_;
                priv_root->mesh_swchnl_flag = (BIT1|BIT2);/*BIT1: initiator, BIT2: reason valid*/
                priv_root->mesh_swchnl_reason = _RSON_MESH_CHANNEL_SWITCH_UNSPECIFIED_;
                SET_PSEUDO_RANDOM_NUMBER(priv_root->mesh_swchnl_precedence);
                priv_root->mesh_swchnl_counter = MESH_CHANNEL_SWITCH_COUNTER;
                return 1;
            }
        }
        else  /*channel switch procedure is on-going*/
        {
            if(priv_root->mesh_swchnl_counter)  /*it is still counting down*/
            {
                return 1;
            }
            else  /*channel switch procedure finished here*/
            {
                priv_root->mesh_swchnl_channel = 0;
            }
        }
    }
    return 0;
}

#if defined(DFS)
int static mesh_DFS_check(DRV_PRIV *priv, struct rx_frinfo *pfrinfo, UINT32 precedence)
{
    unsigned char		*pframe, *p, *pSwWrapper = NULL;
    int len;
    UINT8 swchnl_channel = 0;
    UINT8 swchnl_ttl = 0;
    UINT8 swchnl_flag = 0;
    UINT16 swchnl_reason;
    UINT32 swchnl_precedence = 0;
    UINT8 used80M = 0;
    UINT8 centerChnl;
    int ret = 0;
    pframe = get_pframe(pfrinfo);

    // check  Channel Switch Announcement
    p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _CSA_IE_, &len,
               pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
    if (p)
    {
        swchnl_channel = *(p + MESH_IE_BASE_LEN + 1);
        if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) {
            // check  EID_CH_SW_WRAPPER
            pSwWrapper = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, EID_CH_SW_WRAPPER, &len,
                       pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
            if (pSwWrapper)
            {
                used80M = pSwWrapper[4];
                centerChnl = pSwWrapper[5];
            }

        }
       
        // check Mesh Channel Switch
        p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _MESH_CHANNEL_SWITCH_IE_, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
        if (p)
        {
            swchnl_ttl = *(p + MESH_IE_BASE_LEN);
            swchnl_flag = *(p + MESH_IE_BASE_LEN + 1);
            memcpy(&swchnl_reason, p + MESH_IE_BASE_LEN + 2, 2);
            swchnl_reason = le16_to_cpu(swchnl_reason);
            swchnl_precedence = le32_to_cpu(*((UINT32*)(p+MESH_IE_BASE_LEN + 4)));
        }
    }

    if(swchnl_channel && pSwWrapper && swchnl_precedence && swchnl_ttl > 0)
    {
        if (swchnl_channel == priv->pmib->dot11RFEntry.dot11channel) {
            STADEBUG("Detect DFS from other MAP with same ch %d\n", swchnl_channel);
            return ret;
        }

        if(GET_ROOT(priv)->pmib->dot11DFSEntry.DFS_detected == 0 || (swchnl_precedence > priv->mesh_swchnl_precedence))
        {

           	if (timer_pending(&priv->DFS_timer))
    			del_timer(&priv->DFS_timer);
            
			InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, priv->pmib->dot11RFEntry.dot11channel);			
			RemoveChannel(priv, priv->available_chnl, &priv->available_chnl_num, priv->pmib->dot11RFEntry.dot11channel);
            #ifdef UNIVERSAL_REPEATER
	    	if (IS_DRV_OPEN(GET_VXD_PRIV(priv))){
				RemoveChannel(priv->pvxd_priv, priv->pvxd_priv->available_chnl, &priv->pvxd_priv->available_chnl_num, priv->pmib->dot11RFEntry.dot11channel);
				InsertChannel(priv->pvxd_priv->NOP_chnl, &priv->pvxd_priv->NOP_chnl_num, priv->pmib->dot11RFEntry.dot11channel);
    		}
            #endif           
            
            priv->pshare->dfsSwitchChannel = swchnl_channel;
            priv->pshare->dfsSwitchChCountDown = MESH_DFS_SWITCH_COUNTDOWN;
            if(used80M)
                priv->pmib->dot11nConfigEntry.dot11nUse40M = 2;
            else if(centerChnl != swchnl_channel) {
                priv->pmib->dot11nConfigEntry.dot11nUse40M = 1;
            }
            else 
                priv->pmib->dot11nConfigEntry.dot11nUse40M = 0;


            STADEBUG("Detect DFS from other MAP dfsSwitchChannel: %d, dfsSwitchChCountDown: %d, dot11nUse40M: %d\n",
                priv->pshare->dfsSwitchChannel, priv->pshare->dfsSwitchChCountDown, priv->pmib->dot11nConfigEntry.dot11nUse40M);

            
            priv->mesh_swchnl_ttl = swchnl_ttl - 1;
            priv->mesh_swchnl_flag = swchnl_flag & ~BIT1; /*erase initiator bit*/
            priv->mesh_swchnl_reason = swchnl_reason;
            priv->mesh_swchnl_precedence = swchnl_precedence;


            if( precedence && priv->auto_channel)
            {
                priv->pmib->dot11RFEntry.dot11channel = priv->pshare->dfsSwitchChannel;
                priv->mesh_ChannelPrecedence = precedence;
                priv->mesh_fix_channel = 1;
            }
            
            GET_ROOT(priv)->pmib->dot11DFSEntry.DFS_detected = 1;

        }
        ret = 1;
    }
    return ret;
}

int mesh_DFS_switch_channel(struct rtl8192cd_priv *priv)
{
    struct rtl8192cd_priv * priv_root = GET_ROOT(priv);
    if(GET_MIB(priv_root)->dot1180211sInfo.mesh_enable)
    {
        if(MESH_PEER_LINK_CAP_NUM(priv) == GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor) {            
#if 1
			STADEBUG("Detect DFS and no mesh neightbor exist, prepare to switch channel\n");
			priv_root->mesh_swchnl_ttl = _MESH_HEADER_TTL_;
			priv_root->mesh_swchnl_flag = (BIT1|BIT2);
			priv_root->mesh_swchnl_reason = _RSON_MESH_CHANNEL_SWITCH_REGULATORY_REQUIREMENTS_;
			SET_PSEUDO_RANDOM_NUMBER(priv_root->mesh_swchnl_precedence);
#else
            STADEBUG("Detect DFS and no mesh neightbor exist, rescan to join MESH\n");
            priv->pmib->dot11DFSEntry.DFS_detected = 0;
            priv->pshare->dfsSwitchChannel = 0;
            priv->auto_channel = 1;
            priv->pmib->dot11RFEntry.dot11channel = priv->available_chnl[0];
            priv->mesh_ChannelPrecedence = 0;   
            priv->mesh_fix_channel = 0;
            RTL_W8(TXPAUSE, STOP_BCN);
            start_clnt_ss(priv);
#endif			
        }
        else {           
            STADEBUG("Detect DFS and has mesh neightbor exist, prepare to switch channel\n");
            priv_root->mesh_swchnl_ttl = _MESH_HEADER_TTL_;
            priv_root->mesh_swchnl_flag = (BIT1|BIT2);/*BIT1: initiator, BIT2: reason valid*/
            priv_root->mesh_swchnl_reason = _RSON_MESH_CHANNEL_SWITCH_REGULATORY_REQUIREMENTS_;
            SET_PSEUDO_RANDOM_NUMBER(priv_root->mesh_swchnl_precedence);
        }
        return 1;
    }
    return 0;
}

#endif



/*
 *	@brief	MESH  expire
 *
 *	@param	priv
 *
 *	@retval	void
 */
void mesh_expire(DRV_PRIV* priv)
{
    peer_expire(priv);
    mesh_channel_switch_expire(priv);
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
	if(priv == (DRV_PRIV *)priv->mesh_priv_first)
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE
    {

#if !defined(RTK_MESH_AODV_STANDALONE_TIMER)
        aodv_expire(priv);
#endif
        proxy_table_expire(priv);
#ifdef PU_STANDARD
        proxyupdate_table_expire(priv);
#endif
        pathsel_table_expire(priv);

#ifdef MESH_ROUTE_MAINTENANCE
        route_maintenance(priv);
#endif
    }

#ifdef MESH_USE_METRICOP
    if(priv->mesh_fake_mib.metricID == 2)
        metric_update(priv);
#endif

}

static UINT16 getDataRateOffset(struct stat_info *pstat)
{
#ifdef RTK_AC_SUPPORT  //vht rate , todo, dump vht rates in Mbps
    if(pstat->current_tx_rate >= VHT_RATE_ID){
        return query_vht_rate(pstat);
    }
    else
#endif 
    if (is_MCS_rate(pstat->current_tx_rate))
        return(pstat->current_tx_rate&0x7f);
    else
        return(pstat->current_tx_rate/2);
}

/*
 *	@brief	MESH  get TX data rate
 *
 *	@param	pstat
 *
 *	@retval	UINT16: TX data rate (unit: 1Mbps)
 *
 *	Note: 1.Code copy from 8190n_proc.c, dump_one_stainfo()
 */
/*static UINT16 getDataRate(struct stat_info *pstat)	// fix : 0000083
{
	if (is_MCS_rate(pstat->current_tx_rate))
		// N mode
		return((MCS_DATA_RATE[(pstat->ht_current_tx_info&BIT(0))?1:0][(pstat->ht_current_tx_info&BIT(1))?1:0][pstat->current_tx_rate&0x7f]) / 2);
	else
		// BG mode
		return(pstat->current_tx_rate/2);	// div 2, because TX rate unit:500Kb/s in BG mode
}*/


/*!
  802.11s: (draft D0.02, changsl, 2006/12/19)
  @brief To determine if it's an 802.11s frame by checking the existence of "mesh id" info element
*/
static int testMesh_get_ie(unsigned char *pframe, int offset, int limit)
{
    int len = 0;
    return (get_ie(pframe + offset, _MESH_ID_IE_, &len, limit) ? 1 : 0);
}

static int testMesh_OnAssocReq(struct rx_frinfo *pfrinfo)
{
    unsigned short ie_offset=0;
    unsigned char *pframe = get_pframe(pfrinfo);
    ie_offset = (GetFrameSubType(pframe) == WIFI_ASSOCREQ) ? _ASOCREQ_IE_OFFSET_ : _REASOCREQ_IE_OFFSET_;

    return testMesh_get_ie(pframe, WLAN_HDR_A3_LEN + ie_offset,
                           pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);
}

static int testMesh_OnDisassoc(struct rx_frinfo *pfrinfo)
{
    // special: _PEER_LINK_CLOSE_IE_
    char *p = 0;
    int len = 0;

    // reason = cpu_to_le16(*(unsigned short *)((unsigned int)pframe + WLAN_HDR_A3_LEN ));
    p = get_ie(get_pframe(pfrinfo) + WLAN_HDR_A3_LEN + _DISASS_IE_OFFSET_, _PEER_LINK_CLOSE_IE_,
               &len, pfrinfo->pktlen - WLAN_HDR_A3_LEN - _DISASS_IE_OFFSET_);

    return ((p == NULL) || (len == 0)) ? 0 :1;
}

#define testMesh_OnAssocRsp(pfrinfo)\
	testMesh_get_ie(get_pframe(pfrinfo), WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_,\
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_)

#define testMesh_OnProbeReq(pfrinfo)\
	testMesh_get_ie(get_pframe(pfrinfo), WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_, \
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_)

#define testMesh_OnProbeRsp(pfrinfo)\
	testMesh_get_ie(get_pframe(pfrinfo), WLAN_HDR_A3_LEN + _PROBERSP_IE_OFFSET_,\
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBERSP_IE_OFFSET_)

#define testMesh_OnBeacon(pfrinfo)\
	testMesh_get_ie(get_pframe(pfrinfo), WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_,\
		pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_)


/* Can't resolve, Temporary reserve by popen
static int testMesh_OnAuth(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
	// we don't have any knowledge about it on the draft of 802.11s D0.02
	return 0;
}

static int testMesh_OnDeAuth(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
	// we don't have any knowledge about it on the draft of 802.11s D0.02
	return 0;
}
*/

int is_11s_mgt_frame(int num, DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
    struct stat_info * pstat;
    unsigned char *pframe;
    int isMesh = 0;
    if(!GET_MIB(priv)->dot1180211sInfo.mesh_enable)
        return 0;

    // 802.11s: (draft D1.0, changsl, 2006/12/19)
    // determine if it's an 802.11s frame by checking the existence of "mesh id" element
    switch (num)
    {
        case WIFI_ASSOCREQ:
            isMesh = testMesh_OnAssocReq(pfrinfo);
            break;
        case WIFI_ASSOCRSP:
            isMesh = testMesh_OnAssocRsp(pfrinfo);
            break;
        case WIFI_REASSOCREQ:
            isMesh = testMesh_OnAssocReq(pfrinfo);
            break;
        case WIFI_REASSOCRSP:
            isMesh = testMesh_OnAssocRsp(pfrinfo);
            break;
        case WIFI_PROBEREQ:
            isMesh = testMesh_OnProbeReq(pfrinfo);
            break;
        case WIFI_PROBERSP:
            isMesh = testMesh_OnProbeRsp(pfrinfo);
            break;
        case WIFI_BEACON:
            isMesh = testMesh_OnBeacon(pfrinfo);
            break;
        case WIFI_ATIM:
            // reserved
            break;
        case WIFI_DISASSOC:
            isMesh = testMesh_OnDisassoc(pfrinfo);
            break;
        /* Can't resolve, Temporary reserve by popen
        		case WIFI_AUTH:
        			isMesh = testMesh_OnAuth(priv, pfrinfo);
        			break;
        		case WIFI_DEAUTH:
        			isMesh = testMesh_OnDeAuth(priv, pfrinfo);
        			break;
        */
        case WIFI_WMM_ACTION:
            pframe = get_pframe(pfrinfo);
            pstat = get_stainfo(priv, GetAddr2Ptr(pframe));
            if(pstat != NULL)
            {
                if(isPossibleNeighbor(pstat))
                {
                    isMesh = 1;
                }
            }
            break;
        default:
            break;
    }
    return isMesh;
}


/**
 *	@brief	Mesh write in Mesh ID IE content.
 *
 *	@param	priv		: priv
 *	@param	meshiearray	: UINT8 array , Will be write in content.
 *	@param	isWildcard	: Is wildcard MeshID?
 *						TRUE : ID(MESH_IE) + Len(0, No Mesh_ID content)
 *						FALSE: ID(MESH_IE) + Len(variable) + Mesh_ID content(variable)
 *
 *	@retval	meshiearray	: UINT8 array ,Have content.
 *	@retval	len		: Write in array byte count.
 */
unsigned int mesh_ie_MeshID(DRV_PRIV *priv, UINT8 meshiearray[], UINT8 isWildcard)
{
    int	len;
    UINT8	*meshieptr = meshiearray;

    if (TRUE == isWildcard)
        return 0;	// Empty MeshID, Len = 0

    len = strlen(GET_MIB(priv)->dot1180211sInfo.mesh_id);
    memcpy(meshieptr, &(GET_MIB(priv)->dot1180211sInfo.mesh_id), len);

    return len;
}

unsigned int mesh_ie_OFDM(DRV_PRIV *priv, UINT8 meshiearray[])
{
    meshiearray[0] = priv->pmib->dot11RFEntry.dot11channel;
    meshiearray[1] = priv->pshare->offset_2nd_chan;

    if(priv->auto_channel)
    {
        meshiearray[2] = priv->mesh_fix_channel;
    }
    else
    {
        meshiearray[2] = 1;
    }
    return 3;
}

unsigned int mesh_ie_MeshChannelSwitch(DRV_PRIV *priv, UINT8 meshiearray[])
{
    UINT8	i, *meshieptr = meshiearray;
    UINT32	tmp4bytes;
    int	counter = 0;

    meshiearray[0] = priv->mesh_swchnl_ttl;
    meshiearray[1] = priv->mesh_swchnl_flag;
    meshieptr += 2;
    counter = 2;

    i = sizeof(priv->mesh_swchnl_reason);
    memcpy(meshieptr, &(priv->mesh_swchnl_reason), i);
    meshieptr += i;
    counter += i;


    // Channel precence
    tmp4bytes = cpu_to_le32(priv->mesh_swchnl_precedence);
    memcpy(meshieptr, &tmp4bytes, 4);
    counter += 4;

    return counter;
}

unsigned int mesh_ie_ChannelSwitchAnnoun(DRV_PRIV *priv, UINT8 meshiearray[])
{
    meshiearray[0] = 0; /*channel switch mode, reserved for MBSS*/
    meshiearray[1] = priv->mesh_swchnl_channel; /*new channel number*/
    meshiearray[2] = 0; /*channel switch count.  A value of 0 indicates that the switch occurs at any time*/
    return 3;
}

unsigned int mesh_ie_SecondaryChannelOffset(DRV_PRIV *priv, UINT8 meshiearray[])
{
    meshiearray[0] = priv->mesh_swchnl_offset; /*secondary channel offset*/
    return 1;
}


/**
 *	@brief	Mesh write in WLAN MESH Capacity IE content.
 *
 *	@param	priv		: priv
 *	@param	meshiearray	: UINT8 array ,Will be write in content.
 *
 *	@retval	meshiearray	: UINT8 array , Have content.
 *	@retval	counter		: Write in array byte count.
 */
unsigned int mesh_ie_WLANMeshCAP(DRV_PRIV *priv, UINT8 meshiearray[])
//unsigned int mesh_ie_WLANMeshCAP(struct rtl8192cd_priv *priv, UINT8 meshiearray[])
{
    int	counter = 0;
    UINT8	i, *meshieptr = meshiearray;
    UINT16	tmp2bytes, tmp2bytes_pad;
    UINT32	tmp4bytes;

    // Version
    i = sizeof(priv->mesh_Version);
    memcpy(meshieptr, &(priv->mesh_Version), i);
    meshieptr += i;
    counter += i;

    // Active Protocol ID(OUI+value) (pack) mesh_profile Configure by WEB in the future, Maybe delete, Preservation before delete
    i = sizeof(priv->mesh_profile[0].PathSelectProtocolID);
    memcpy(meshieptr, &(priv->mesh_profile[0].PathSelectProtocolID), i);
    meshieptr += i;
    counter += i;

    // Active Metric ID(OUI+value) (pack) mesh_profile Configure by WEB in the future, Maybe delete, Preservation before delete
    i = sizeof(priv->mesh_profile[0].PathSelectMetricID);
    memcpy(meshieptr, &(priv->mesh_profile[0].PathSelectMetricID), i);
    meshieptr += i;
    counter += i;

    // Peer Capacity
    if (MESH_PEER_LINK_CAP_NUM(priv) > 0)	// If cap less 0, Fix to 0.
        tmp2bytes = (UINT16)MESH_PEER_LINK_CAP_NUM(priv) & MESH_PEER_LINK_CAP_CAPACITY_MASK;
    else
        tmp2bytes = 0;

    tmp2bytes |= (((UINT16)priv->mesh_PeerCAP_flags & MESH_PEER_LINK_CAP_FLAGS_MASK) << 8);	// Flags shift to cap bit15-8
    tmp2bytes_pad = cpu_to_le16(tmp2bytes);
    memcpy(meshieptr, &tmp2bytes_pad, 2);
    meshieptr += 2;
    counter += 2;

    // Power Save Capacity
    i = sizeof(priv->mesh_PowerSaveCAP);
    memcpy(meshieptr, &(priv->mesh_PowerSaveCAP), i);
    meshieptr += i;
    counter += i;

    // Synchornization Capacity
    i = sizeof(priv->mesh_SyncCAP);
    memcpy(meshieptr, &(priv->mesh_SyncCAP), i);
    meshieptr += i;
    counter += i;

    // MDA Capacity
    i = sizeof(priv->mesh_MDA_CAP);
    memcpy(meshieptr, &(priv->mesh_MDA_CAP), i);
    meshieptr += i;
    counter += i;

    // Channel precence
    tmp4bytes = cpu_to_le32(priv->mesh_ChannelPrecedence);
    memcpy(meshieptr, &tmp4bytes, 4);
    counter += 4;

    return counter;
}


/*
 *	@brief	Mesh write in Link State Announcement IE content.
 *
 *	@param	priv		: priv
 *	@param	pstat		: Get rssi (temporary)
 *	@param	meshiearray	: UINT8 array , Will be write in content.
 *
 *	@retval	meshiearray	: UINT8 array ,Have content.
 *	@retval	counter		: Write in array byte count.
 */
static int mesh_ie_LocalLinkStateANNOU(DRV_PRIV *priv, struct rx_frinfo *pfrinfo, struct stat_info *pstat, UINT8 meshiearray[])
{
    int	counter = 0;
    UINT16	tmp;
    UINT8	*meshieptr = meshiearray;

    tmp = cpu_to_le16(getDataRateOffset(pstat));	// fix : 0000083
    memcpy(meshieptr, &tmp, 2);
    meshieptr += 2;
    counter += 2;

    // ept (temporary rssi)
    // pfrinfo->rssi = 0xaa;//test
    tmp = cpu_to_le16((UINT16)pfrinfo->rssi);
    memcpy(meshieptr, &tmp, 2);
    counter += 2;

    return counter;
}


/*
 *	@brief	Mesh write in Peer Link open content.
 *
 *	@param	priv		: priv
 *	@param	meshiearray	: UINT8 array, Will be write in content.
 *
 *	@retval	meshiearray	: UINT8 array, Have content.
 *	@retval	counter		: Write in array byte count.
 */
static int mesh_ie_PeerLinkOpen(DRV_PRIV *priv, struct stat_info *pstat, UINT8 meshiearray[])
{
    int	counter = 0;
    UINT8	size;
    UINT8	*meshieptr = meshiearray;
    UINT32	tmp4bytes;

    size = sizeof(pstat->mesh_neighbor_TBL.LocalLinkID);
    tmp4bytes = cpu_to_le32(pstat->mesh_neighbor_TBL.LocalLinkID);
    memcpy(meshieptr, &tmp4bytes, size);
    counter += size;

    return counter;
}


/*
 *	@brief	Mesh write in Peer Link open content.
 *
 *	@param	priv		: priv
 *	@param	meshiearray	: UINT8 array, Will be write in content.
 *
 *	@retval	meshiearray	: UINT8 array, Have content.
 *	@retval	counter		: Write in array byte count.
 */
static int mesh_ie_PeerLinkConfirm(DRV_PRIV *priv, struct stat_info *pstat, UINT8 meshiearray[])
{
    int	counter = 0;
    UINT8	size;
    UINT8	*meshieptr = meshiearray;
    UINT32	tmp4bytes;

    // LocalLinkID
    size = sizeof(pstat->mesh_neighbor_TBL.LocalLinkID);
    tmp4bytes = cpu_to_le32(pstat->mesh_neighbor_TBL.LocalLinkID);
    memcpy(meshieptr, &tmp4bytes, size);
    meshieptr += size;
    counter += size;

    // PeerLinkID
    size = sizeof(pstat->mesh_neighbor_TBL.PeerLinkID);
    tmp4bytes = cpu_to_le32(pstat->mesh_neighbor_TBL.PeerLinkID);
    memcpy(meshieptr, &tmp4bytes, size);
    counter += size;

    return counter;
}


/*
 *	@brief	Mesh write in Peer Link Close content.
 *
 *	@param	priv		: priv
 *	@param	meshiearray	: UINT8 array, Will be write in content.
 *
 *	@retval	meshiearray	: UINT8 array, Have content.
 *	@retval	counter		: Write in array byte count.
 */
static int mesh_ie_PeerLinkClose(DRV_PRIV *priv, struct stat_info *pstat, UINT8 meshiearray[], UINT8 reason)
{
    int	counter = 0;
    UINT8	size;
    UINT8	*meshieptr = meshiearray;
    UINT32	tmp4bytes;

    // Reason code
    size = sizeof(reason);
    memcpy(meshieptr, &reason, size);
    meshieptr += size;
    counter += size;

    // LocalLinkID
    size = sizeof(pstat->mesh_neighbor_TBL.LocalLinkID);
    tmp4bytes = cpu_to_le32(pstat->mesh_neighbor_TBL.LocalLinkID);
    memcpy(meshieptr, &tmp4bytes, size);
    meshieptr += size;
    counter += size;

    // PeerLinkID
    size = sizeof(pstat->mesh_neighbor_TBL.PeerLinkID);
    tmp4bytes = cpu_to_le32(pstat->mesh_neighbor_TBL.PeerLinkID);
    memcpy(meshieptr, &tmp4bytes, size);
    counter += size;

    return counter;
}

#ifdef PU_STANDARD
//add by pepsi 20080229
/*
 *	@brief	Mesh generate content as Proxy Update IE info.
 *
 *	@param	priv			: priv data
 *	@param	PUopt		: PU Flag
 *	@param	SN		: PU's Sequence Number
 *	@param	Addrs		: proxied addresses
 *	@param	meshiearray	: IE buffer
 *	@retval	counter		: byte count
 */
static int mesh_ie_ProxyUpdate(DRV_PRIV *priv, struct proxyupdate_table_entry * Entry, UINT8 meshiearray[])
{
    int	length = 0;
    UINT8	*meshieptr = meshiearray;

    *meshieptr = Entry->PUflag;	//add or delete
    length++;
    memcpy( meshieptr+length, &Entry->PUSN, 1);	//Sequense Number
    length++;
    memcpy(meshieptr+length,Entry->proxymac,6);	//ignore Proxy Address for BROADCAST
    length +=6;
    *(UINT16 *)(meshieptr+length) = Entry->STAcount;	//Number of Proxied addresses should be changed
    length += 2;
    memcpy(meshieptr+length, Entry->proxiedmac, 6);	//Proxied addresses list
    length += 6;

    return length;	//total length should be variable
}

//add by pepsi 20080305
/*
 *	@brief	Mesh generate content as Proxy Update Confirmation IE info.
 *
 *	@param	priv			: priv data
 *	@param	PUSN		: PU's Sequence Number relative to PU received
 *	@param	MPmac		: proxy addresses
 *	@param	meshiearray	: IE buffer
 */
static int mesh_ie_ProxyUpdateConfirm(DRV_PRIV *priv, UINT8 * PUSN,UINT8 * MPmac, UINT8 meshiearray[])
{
    int	length = 0;
    UINT8	*meshieptr = meshiearray , flag;

    flag = 0x00;
    *meshieptr = flag;	//add or delete
    length++;
    memcpy( meshieptr+length, PUSN, 1);	//Sequense Number
    length++;
    memcpy(meshieptr+length,MPmac,6);	//ignore Proxy Address for BROADCAST
    length +=6;

    return length;	//total length should be variable
}
#endif	// PU_STANDARD



/**
 *	@brief	issue de-association frame.
 *
 *	+---------------+-----+----+----+-------+-----+-------------+ \n
 *	| Frame Control | ... | DA | SA | BSSID | ... | Reason Code | \n
 *	+---------------+-----+----+----+-------+-----+-------------+ \n
 */


void issue_disassoc_MP(DRV_PRIV *priv, struct stat_info *pstat, int reason, UINT8 peerLinkReason)
{
    struct wifi_mib *pmib;
    unsigned char	*pbuf;
    unsigned short  val;
    UINT8		meshiearray[9]; // mesh IE buffer (Max byte is mesh_ie_PeerLinkClose)
    unsigned char *da = pstat->hwaddr;

    DECLARE_TXINSN(txinsn);

    txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

#ifdef RTL8190_FASTEXTDEV_FUNCALL
    rtl865x_extDev_removeHost(da, priv->WLAN_VLAN_ID);
#endif

    pmib= GET_MIB(priv);

    txinsn.is_11s = GET_MIB(priv)->dot1180211sInfo.mesh_enable;
    txinsn.q_num = MANAGE_QUE_NUM;
    txinsn.fr_type = _PRE_ALLOCMEM_;
    txinsn.tx_rate  = find_rate(priv, NULL, 0, 1);
    txinsn.lowest_tx_rate = txinsn.tx_rate;
    txinsn.fixed_rate = 1;

    pbuf = txinsn.pframe  = get_mgtbuf_from_poll(priv);

    if (pbuf == NULL)
        goto issue_disassoc_MP_fail;

    txinsn.phdr = get_wlanhdr_from_poll(priv);

    if (txinsn.phdr == NULL)
        goto issue_disassoc_MP_fail;

    memset((void *)txinsn.phdr, 0, sizeof(struct  wlan_hdr));

    val = cpu_to_le16(reason);

    pbuf = set_fixed_ie(pbuf, _RSON_CODE_, (unsigned char *)&val, &txinsn.fr_len);

    // Peer Link Close
    pbuf = set_ie(pbuf, _PEER_LINK_CLOSE_IE_, mesh_ie_PeerLinkClose(priv, pstat, meshiearray, peerLinkReason), meshiearray, &txinsn.fr_len);

    MESH_DEBUG_MSG("*** Issue Disassocation, MAC=%02X:%02X:%02X:%02X:%02X:%02X, State=%d, Reason = %d\n"
                   , pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2], pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5], pstat->mesh_neighbor_TBL.State, peerLinkReason);


    // MAC header
    SetFrameType((txinsn.phdr), WIFI_MGT_TYPE);
    SetFrameSubType((txinsn.phdr), WIFI_DISASSOC);

    memcpy((void *)GetAddr1Ptr((txinsn.phdr)), da, MACADDRLEN);
    memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
    memcpy((void *)GetAddr3Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);	/*Address 3 is TA*/

    SNMP_MIB_ASSIGN(dot11DisassociateReason, reason);
    SNMP_MIB_COPY(dot11DisassociateStation, da, MACADDRLEN);

    if ((DRV_FIRETX(priv, &txinsn)) == SUCCESS)
    {
        // stop rssi check
#ifdef RTL8192SE
        if ((OPMODE & WIFI_AP_STATE) && get_stainfo(priv, da) && (get_stainfo(priv, da)->aid <= 8))
            set_fw_reg(priv, 0xfd000013 | get_stainfo(priv, da)->aid<<16, 0, 0);
#endif
        return;
    }

issue_disassoc_MP_fail:

    if (txinsn.phdr)
        release_wlanhdr_to_poll(priv, txinsn.phdr);
    if (txinsn.pframe)
        release_mgtbuf_to_poll(priv, txinsn.pframe);
}

/**
 *	@brief issue mesh assocation request
 *		PS: Code from client code
 *	@param	priv			: priv
 *	@param	meshiearray	:
 *
 *	@retval	meshiearray	:
 *	@retval	void
 */
unsigned int issue_assocreq_MP(DRV_PRIV *priv,  struct stat_info *pstat)
{
    unsigned short	val;
    struct wifi_mib *pmib;
    unsigned char	*pbuf;
    unsigned char	*pbssrate=NULL;
    int		bssrate_len;
    unsigned char	supportRateSet[32];
    int		i, j, idx=0, supportRateSetLen=0, match=0;
    unsigned int	retval=0;
    UINT8		meshiearray[100];	// mesh IE buffer


    //#ifdef SEMI_QOS	brian 2011-0923
#ifdef WIFI_WMM
    int		k;
#endif
    DECLARE_TXINSN(txinsn);
    txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;
    pmib= GET_MIB(priv);

    txinsn.is_11s = GET_MIB(priv)->dot1180211sInfo.mesh_enable;
    txinsn.q_num = MANAGE_QUE_NUM;
    txinsn.fr_type = _PRE_ALLOCMEM_;
    txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
    txinsn.lowest_tx_rate = txinsn.tx_rate;
    txinsn.fixed_rate = 1;
    pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);

    if (pbuf == NULL)
        goto issue_assocreq_MP_fail;

    txinsn.phdr = get_wlanhdr_from_poll(priv);

    if (txinsn.phdr == NULL)
        goto issue_assocreq_MP_fail;

    memset((void *)txinsn.phdr, 0, sizeof(struct  wlan_hdr));

    val = cpu_to_le16(pmib->dot11Bss.capability);

    if ((1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) && (0 == GET_MIB(priv)->dot1180211sInfo.mesh_ap_enable))	// non-AP MP (MAP)	only, popen:802.11s Draft 1.0 P17  7.3.1.4 : ESS & IBSS are "0" (PS:val reset)
        val &= (!(BIT(1) | BIT(0)));


    // Privacy Bit
    if (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm)
        val |= cpu_to_le16(BIT(4));

    // short preamble
    if (SHORTPREAMBLE)
        val |= cpu_to_le16(BIT(5));

#ifdef DOT11H
    if(priv->pmib->dot11hTPCEntry.tpc_enable)
        val |= cpu_to_le16(BIT(8)); /* set spectrum mgt */
#endif

    // Capability Info.
    pbuf = set_fixed_ie(pbuf, _CAPABILITY_, (unsigned char *)&val, &txinsn.fr_len);

    // Listen Interval
    val	 = cpu_to_le16(3);
    pbuf = set_fixed_ie(pbuf, _LISTEN_INTERVAL_, (unsigned char *)&val, &txinsn.fr_len);

    /* SSID (client code, modify to ap code, Because recived side compare in AP mode)*/
    if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable)
        pbuf = set_ie(pbuf, _SSID_IE_, SSID_LEN, SSID, &txinsn.fr_len); // ap code
    else
        pbuf = set_ie(pbuf, _SSID_IE_, pmib->dot11Bss.ssidlen, pmib->dot11Bss.ssid, &txinsn.fr_len);	// client code

    // Supported Rates & Exterend Supported Rates
    if (pmib->dot11Bss.supportrate == 0)
    {
        // AP don't contain rate info in beacon/probe response
        // Use our rate in asoc req
        get_bssrate_set(priv, _SUPPORTEDRATES_IE_, &pbssrate, &bssrate_len);
        pbuf = set_ie(pbuf, _SUPPORTEDRATES_IE_, bssrate_len, pbssrate, &txinsn.fr_len);

        //EXT supported rates.
        if (get_bssrate_set(priv, _EXT_SUPPORTEDRATES_IE_, &pbssrate, &bssrate_len))
            pbuf = set_ie(pbuf, _EXT_SUPPORTEDRATES_IE_, bssrate_len, pbssrate, &txinsn.fr_len);
    }
    else
    {
        // See if there is any mutual supported rate
        for (i=0; dot11_rate_table[i]; i++)
        {
            int bit_mask = 1 << i;
            if (pmib->dot11Bss.supportrate & bit_mask)
            {
                val = dot11_rate_table[i];
                for (j=0; j<AP_BSSRATE_LEN; j++)
                {
                    if (val == (AP_BSSRATE[j] & 0x7f))
                    {
                        match = 1;
                        break;
                    }
                }
                if (match)
                    break;
            }
        }

        // If no supported rates match, assoc fail!
        if (!match)
        {
            DEBUG_ERR("Supported rate mismatch!\n");
            retval = 1;
            goto issue_assocreq_MP_fail;
        }

        // Use AP's rate info in asoc req
        for (i=0; dot11_rate_table[i]; i++)
        {
            int bit_mask = 1 << i;
            if (pmib->dot11Bss.supportrate & bit_mask)
            {
                val = dot11_rate_table[i];
                if (((pmib->dot11BssType.net_work_type == WIRELESS_11B) && is_CCK_rate(val)) ||
                        (pmib->dot11BssType.net_work_type != WIRELESS_11B))
                {
                    if (pmib->dot11Bss.basicrate & bit_mask)
                        val |= 0x80;

                    supportRateSet[idx] = val;
                    supportRateSetLen++;
                    idx++;
                }
            }
        }

        if (supportRateSetLen == 0)
        {
            retval = 1;
            goto issue_assocreq_MP_fail;
        }
        else if (supportRateSetLen <= 8)
            pbuf = set_ie(pbuf, _SUPPORTEDRATES_IE_ , supportRateSetLen , supportRateSet, &txinsn.fr_len);
        else
        {
            pbuf = set_ie(pbuf, _SUPPORTEDRATES_IE_, 8, supportRateSet, &txinsn.fr_len);
            pbuf = set_ie(pbuf, _EXT_SUPPORTEDRATES_IE_, supportRateSetLen-8, &supportRateSet[8], &txinsn.fr_len);
        }
    }

#ifdef DOT11H
    if(pmib->dot11hTPCEntry.tpc_enable)
    {
        pbuf = construct_power_capability_ie(priv, pbuf, &txinsn.fr_len);
        pbuf = construct_supported_channel_ie(priv, pbuf, &txinsn.fr_len);
    }
#endif

#ifdef WIFI_SIMPLE_CONFIG
    if (!(pmib->wscEntry.wsc_enable && pmib->wscEntry.assoc_ielen))
#endif
    {
        if (pmib->dot11RsnIE.rsnielen)
        {
            memcpy(pbuf, pmib->dot11RsnIE.rsnie, pmib->dot11RsnIE.rsnielen);
            pbuf += pmib->dot11RsnIE.rsnielen;
            txinsn.fr_len += pmib->dot11RsnIE.rsnielen;
        }
    }


#ifdef WIFI_WMM
    if (QOS_ENABLE)
        pbuf = set_ie(pbuf, _RSN_IE_1_, _WMM_IE_Length_, GET_WMM_IE, &txinsn.fr_len);
#endif

    if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
    {
        construct_ht_ie(priv, priv->pshare->is_40m_bw, priv->pshare->offset_2nd_chan);
        pbuf = set_ie(pbuf, _HT_CAP_, priv->ht_cap_len, (unsigned char *)&priv->ht_cap_buf, &txinsn.fr_len);

        #ifdef RTK_AC_SUPPORT
        if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)
        {
            construct_vht_ie(priv, priv->pshare->working_channel);
            pbuf = set_ie(pbuf, EID_VHTCapability, priv->vht_cap_len, (unsigned char *)&priv->vht_cap_buf, &txinsn.fr_len);
            pbuf = set_ie(pbuf, EID_VHTOperation, priv->vht_oper_len, (unsigned char *)&priv->vht_oper_buf, &txinsn.fr_len);
        }
        #endif

        MESH_DEBUG_MSG("set ht_ie while sending AssocReq\n");
    }

#ifdef WIFI_SIMPLE_CONFIG
    if (pmib->wscEntry.wsc_enable && pmib->wscEntry.assoc_ielen)
    {
        memcpy(pbuf, pmib->wscEntry.assoc_ie, pmib->wscEntry.assoc_ielen);
        pbuf += pmib->wscEntry.assoc_ielen;
        txinsn.fr_len += pmib->wscEntry.assoc_ielen;
    }
#endif

    // Realtek proprietary IE
    if (priv->pshare->rtk_ie_len)
        pbuf = set_ie(pbuf, _RSN_IE_1_, priv->pshare->rtk_ie_len, priv->pshare->rtk_ie_buf, &txinsn.fr_len);

    // Customer proprietary IE
    if (priv->pmib->miscEntry.private_ie_len)
    {
        memcpy(pbuf, pmib->miscEntry.private_ie, pmib->miscEntry.private_ie_len);
        pbuf += pmib->miscEntry.private_ie_len;
        txinsn.fr_len += pmib->miscEntry.private_ie_len;
    }

    // Mesh ID
    pbuf = set_ie(pbuf, _MESH_ID_IE_, mesh_ie_MeshID(priv, meshiearray, FALSE), meshiearray, &txinsn.fr_len);

    // WLAN Mesh Capability
    pbuf = set_ie(pbuf, _WLAN_MESH_CAP_IE_, mesh_ie_WLANMeshCAP(priv, meshiearray), meshiearray, &txinsn.fr_len);

    // Peer Link Open
    pbuf = set_ie(pbuf, _PEER_LINK_OPEN_IE_, mesh_ie_PeerLinkOpen(priv, pstat, meshiearray), meshiearray, &txinsn.fr_len);

    SetFrameType((txinsn.phdr), WIFI_MGT_TYPE);
    SetFrameSubType((txinsn.phdr), WIFI_ASSOCREQ);

    memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
    memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
    memcpy((void *)GetAddr3Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);	/*Address 3 is TA*/

    if ((DRV_FIRETX(priv, &txinsn)) == SUCCESS)
        return retval;

issue_assocreq_MP_fail:

    DEBUG_ERR("sending assoc req fail!\n");

    if (txinsn.phdr)
        release_wlanhdr_to_poll(priv, txinsn.phdr);
    if (txinsn.pframe)
        release_mgtbuf_to_poll(priv, txinsn.pframe);
    return retval;
}



/**
 *	@brief	issue association response
 *
 *	After OnAssocReq(), association response is AP to STA.

 *	+-------+-------+----+----+--------+	\n
 *	| Frame control | DA | SA |	BSS ID |	\n
 *	+-------+-------+----+----+--------+	\n
 *	\n
 *	+------------+-------------+----------------+--------------+	\n
 *	| Capability | Status Code | Association ID | Support rate |	\n
 *	+------------+-------------+----------------+--------------+	\n
 *
 *	@param	priv		device
 *	@param	status		association state
 *	@param	pstat		state information
 *	@param	pkt_type	packet type
 */
void issue_assocrsp_MP(DRV_PRIV *priv, unsigned short status, struct stat_info *pstat, int pkt_type)
{
    unsigned short	val;
    struct wifi_mib *pmib;
    unsigned char	*bssid,*pbuf;
    UINT8	meshiearray[100];	// mesh IE buffer

    DECLARE_TXINSN(txinsn);

    txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

    pmib= GET_MIB(priv);

    bssid = pmib->dot11StationConfigEntry.dot11Bssid;

    txinsn.is_11s = GET_MIB(priv)->dot1180211sInfo.mesh_enable;
    txinsn.q_num = MANAGE_QUE_NUM;
    txinsn.fr_type = _PRE_ALLOCMEM_;
    txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
    txinsn.lowest_tx_rate = txinsn.tx_rate;
    txinsn.fixed_rate = 1;
    pbuf = txinsn.pframe  = get_mgtbuf_from_poll(priv);

#ifdef CONFIG_RTL_88E_SUPPORT
    if (GET_CHIP_VER(priv)==VERSION_8188E && pstat->sta_in_firmware == 1)
    {
        RTL8188E_MACID_NOLINK(priv, (status == _STATS_SUCCESSFUL_)?0:1, REMAP_AID(pstat));
        RTL8188E_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
        pstat->txpause_flag = 0;
    }
#endif

#ifdef CONFIG_WLAN_HAL
    if(IS_HAL_CHIP(priv))
    {
        if(pstat && (REMAP_AID(pstat) < 128))
        {
            DEBUG_WARN("%s %d issue_asocrsp, set MACID 0 AID = %x \n",__FUNCTION__,__LINE__,REMAP_AID(pstat));
            GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0 ,REMAP_AID(pstat));
            pstat->txpause_flag = 0;
        }
        else
        {
            DEBUG_WARN(" MACID sleep only support 128 STA \n");
        }
    }
#endif

    if (pbuf == NULL)
        goto issue_assocrsp_MP_fail;

    txinsn.phdr = get_wlanhdr_from_poll(priv);

    if (txinsn.phdr == NULL)
        goto issue_assocrsp_MP_fail;

    memset((void *)txinsn.phdr, 0, sizeof(struct  wlan_hdr));


    if ((1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) && (0 == GET_MIB(priv)->dot1180211sInfo.mesh_ap_enable))	// non-AP MP (MAP)	only, popen:802.11s Draft 1.0 P17  7.3.1.4 : ESS & IBSS are "0" (PS:val reset)
        val = 0;
    else
        val = cpu_to_le16(BIT(0));

    if (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm)
        val |= cpu_to_le16(BIT(4));

    if (SHORTPREAMBLE)
        val |= cpu_to_le16(BIT(5));

    if (priv->pmib->dot11ErpInfo.shortSlot)
        val |= cpu_to_le16(BIT(10));

#ifdef RTK_AC_SUPPORT //for 11ac logo
    if(priv->pshare->rf_ft_var.lpwrc)
        val |= cpu_to_le16(BIT(8));	/* set spectrum mgt */
#endif

    pbuf = set_fixed_ie(pbuf, _CAPABILITY_, (unsigned char *)&val, &txinsn.fr_len);

    status = cpu_to_le16(status);
    pbuf = set_fixed_ie(pbuf, _STATUS_CODE_, (unsigned char *)&status, &txinsn.fr_len);

    val = cpu_to_le16(pstat->aid | 0xC000);
    pbuf = set_fixed_ie(pbuf, _ASOC_ID_, (unsigned char *)&val, &txinsn.fr_len);
    /*
    #ifdef RTL8192SE_ACUT
    pstat->bssrateset[0] = 0x8c; // basic rate
    pstat->bssrateset[1] = 0x92; // basic rate
    pstat->bssrateset[2] = 0x98; // basic rate
    pstat->bssrateset[3] = 0xA4; // basic rate
    pstat->bssrateset[4] = 0x30;
    pstat->bssrateset[5] = 0x48;
    pstat->bssrateset[6] = 0x60;
    pstat->bssrateset[7] = 0x6c;
    #endif
    */
    if (STAT_OPRATE_LEN <= 8)
        pbuf = set_ie(pbuf, _SUPPORTEDRATES_IE_, STAT_OPRATE_LEN, STAT_OPRATE, &txinsn.fr_len);
    else
    {
        pbuf = set_ie(pbuf, _SUPPORTEDRATES_IE_, 8, STAT_OPRATE, &txinsn.fr_len);
        pbuf = set_ie(pbuf, _EXT_SUPPORTEDRATES_IE_, STAT_OPRATE_LEN-8, STAT_OPRATE+8, &txinsn.fr_len);
    }

//#ifdef SEMI_QOS	brian 2011-0923
#ifdef WIFI_WMM
    //Set WMM Parameter Element
    if ((QOS_ENABLE) && (pstat->QosEnabled))
        pbuf = set_ie(pbuf, _RSN_IE_1_, _WMM_Para_Element_Length_, GET_WMM_PARA_IE, &txinsn.fr_len);
#endif

#ifdef WIFI_SIMPLE_CONFIG
    if (pmib->wscEntry.wsc_enable && pmib->wscEntry.assoc_ielen)
    {
        memcpy(pbuf, pmib->wscEntry.assoc_ie, pmib->wscEntry.assoc_ielen);
        pbuf += pmib->wscEntry.assoc_ielen;
        txinsn.fr_len += pmib->wscEntry.assoc_ielen;
    }
#endif

    if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && (pstat->ht_cap_len > 0))
    {
        if (priv->pshare->is_40m_bw && ((pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_)) == 0))
        {
            struct ht_cap_elmt ht_cap_buf;
            memcpy(&ht_cap_buf, &priv->ht_cap_buf, priv->ht_cap_len);
            ht_cap_buf.support_mcs[1] |= 0xff;
            pbuf = set_ie(pbuf, _HT_CAP_, priv->ht_cap_len, (unsigned char *)&ht_cap_buf, &txinsn.fr_len);
            pbuf = set_ie(pbuf, _HT_IE_, priv->ht_ie_len, (unsigned char *)&priv->ht_ie_buf, &txinsn.fr_len);
        }
        else
        {
            pbuf = set_ie(pbuf, _HT_CAP_, priv->ht_cap_len, (unsigned char *)&priv->ht_cap_buf, &txinsn.fr_len);
            pbuf = set_ie(pbuf, _HT_IE_, priv->ht_ie_len, (unsigned char *)&priv->ht_ie_buf, &txinsn.fr_len);
            pbuf = construct_ht_ie_old_form(priv, pbuf, &txinsn.fr_len);
        }

#ifdef WIFI_11N_2040_COEXIST
        if (priv->pmib->dot11nConfigEntry.dot11nCoexist && priv->pshare->is_40m_bw)
        {
            unsigned char temp_buf = _2040_COEXIST_SUPPORT_ ;
#if 0	//brian 2011-0923 wait to fix
            construct_obss_scan_para_ie(priv);
#endif
            pbuf = set_ie(pbuf, _OBSS_SCAN_PARA_IE_, priv->obss_scan_para_len,
                          (unsigned char *)&priv->obss_scan_para_buf, &txinsn.fr_len);

            pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 1, &temp_buf, &txinsn.fr_len);
        }
#endif
    }

#ifdef RTK_AC_SUPPORT //for 11ac logo
    if((priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) && (!should_restrict_Nrate(priv, pstat)))
        if((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WEP_40_PRIVACY_)
                && (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WEP_104_PRIVACY_))
        {
            // 41
            // operting mode
            {
                char tmp[8];
                memset(tmp, 0, 8);
                tmp[7] = 0x40;
                pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 8, tmp, &txinsn.fr_len);
            }

            //60, 61
            construct_vht_ie(priv, priv->pshare->working_channel);
            pbuf = set_ie(pbuf, EID_VHTCapability, priv->vht_cap_len, (unsigned char *)&priv->vht_cap_buf, &txinsn.fr_len);
            pbuf = set_ie(pbuf, EID_VHTOperation, priv->vht_oper_len, (unsigned char *)&priv->vht_oper_buf, &txinsn.fr_len);
            // 66
            if(priv->pshare->rf_ft_var.opmtest&1)
                pbuf = set_ie(pbuf, EID_VHTOperatingMode, 1, (unsigned char *)&(priv->pshare->rf_ft_var.oper_mode_field), &txinsn.fr_len);

        }
#endif


    // Realtek proprietary IE
    if (priv->pshare->rtk_ie_len)
        pbuf = set_ie(pbuf, _RSN_IE_1_, priv->pshare->rtk_ie_len, priv->pshare->rtk_ie_buf, &txinsn.fr_len);

    // Mesh ID
    pbuf = set_ie(pbuf, _MESH_ID_IE_, mesh_ie_MeshID(priv, meshiearray, FALSE), meshiearray, &txinsn.fr_len);

    // WLAN Mesh Capability
    pbuf = set_ie(pbuf, _WLAN_MESH_CAP_IE_, mesh_ie_WLANMeshCAP(priv, meshiearray), meshiearray, &txinsn.fr_len);

    if (WIFI_ASSOCRSP == pkt_type)  	// association response ONLY!!
    {
        // Peer Link Confirm
        pbuf = set_ie(pbuf, _PEER_LINK_CONFIRM_IE_, mesh_ie_PeerLinkConfirm(priv, pstat, meshiearray), meshiearray, &txinsn.fr_len);
    }


    SetFrameType((txinsn.phdr), WIFI_MGT_TYPE);
    if ((pkt_type == WIFI_ASSOCRSP) || (pkt_type == WIFI_REASSOCRSP))
        SetFrameSubType((txinsn.phdr), pkt_type);
    else
        goto issue_assocrsp_MP_fail;

    memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
    memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
    memcpy((void *)GetAddr3Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);	/*Address 3 is TA*/

    if((GET_CHIP_VER(priv) != VERSION_8812E) && (GET_CHIP_VER(priv) != VERSION_8881A))
    {
        unsigned int tmp_d2c = RTL_R32(0xd2c);

        /*
        		if(pstat->IOTPeer== HT_IOT_PEER_INTEL)
        		{
        			tmp_d2c = (tmp_d2c & (~ BIT(11)));
        		}
        		else
        		{
        			if(is_intel_connected(priv))
        				tmp_d2c = (tmp_d2c & (~ BIT(11)));
        			else
        */
        tmp_d2c = (tmp_d2c | BIT(11));
//		}

        RTL_W32(0xd2c, tmp_d2c);

#if 1
        tmp_d2c = RTL_R32(0xd2c);

        if(tmp_d2c & BIT(11))
            MESH_DEBUG_MSG("BIT(11) of 0xd2c = %d, 0xd2c = 0x%x\n", 1, tmp_d2c);
        else
            MESH_DEBUG_MSG("BIT(11) of 0xd2c = %d, 0xd2c = 0x%x\n", 0, tmp_d2c);
#endif

    }

    if ((DRV_FIRETX(priv, &txinsn)) == SUCCESS)
    {
// 2008.05.13
        /*
        #ifndef CONFIG_RTL865X_KLD
        		if(!SWCRYPTO && !IEEE8021X_FUN &&
        			(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_ ||
        			 pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)) {
        			DOT11_SET_KEY Set_Key;
        			memcpy(Set_Key.MACAddr, pstat->hwaddr, 6);
        			Set_Key.KeyType = DOT11_KeyType_Pairwise;
        			Set_Key.EncType = pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;

        			Set_Key.KeyIndex = pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
        			DOT11_Process_Set_Key(priv->dev, NULL, &Set_Key,
        				pmib->dot11DefaultKeysTable.keytype[Set_Key.KeyIndex].skey);
        		}
        #endif
        */
        return;
    }

issue_assocrsp_MP_fail:

    if (txinsn.phdr)
        release_wlanhdr_to_poll(priv, txinsn.phdr);
    if (txinsn.pframe)
        release_mgtbuf_to_poll(priv, txinsn.pframe);
}




/**
 *	@brief	issue Locak Link State announence frame
 *		PS: Code from client code
 *	@param	priv			: priv
 *
 *	@retval	void
 */
static void issue_LocalLinkStateANNOU_MP(DRV_PRIV *priv, struct rx_frinfo *pfrinfo, struct stat_info *pstat)
{
    unsigned short	seq;
    UINT8			tmp;
    unsigned char	*pbuf;
    UINT8			meshiearray[4];	// mesh IE buffer (Max byte is mesh_ie_LocalLinkStateANNOU)

    DECLARE_TXINSN(txinsn);

    txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

    txinsn.is_11s = GET_MIB(priv)->dot1180211sInfo.mesh_enable;
    txinsn.q_num = MANAGE_QUE_NUM;
    txinsn.fr_type = _PRE_ALLOCMEM_;
    txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
    txinsn.lowest_tx_rate = txinsn.tx_rate;
    txinsn.fixed_rate = 1;	// D1.06 is data frame, SHALL be remove it.
    pbuf = txinsn.pframe  = get_mgtbuf_from_poll(priv);

    if (pbuf == NULL)
        goto issue_LocalLinkStateANNOU_MP_fail;

    txinsn.phdr = get_wlanhdr_from_poll(priv);

    if (txinsn.phdr == NULL)
        goto issue_LocalLinkStateANNOU_MP_fail;

    memset((void *)txinsn.phdr, 0, sizeof(struct  wlan_hdr));

    // It is managment frame (D1.06 is data frame ??)
    SetFrDs(txinsn.phdr);
    SetToDs(txinsn.phdr);

    // Set Address 4 (SA)
    memcpy(pbuf, GET_MY_HWADDR, MACADDRLEN);
    pbuf += MACADDRLEN;
    txinsn.fr_len += MACADDRLEN;

    // Set Mesh Header's Mesh Flags
    pbuf = set_fixed_ie(pbuf, 1 , (unsigned char *)&(priv->mesh_HeaderFlags), &txinsn.fr_len);

    // Set Mesh Header's TTL(only 1 hop unicast, set "1")
    tmp = 1;
    pbuf = set_fixed_ie(pbuf, 1 , (unsigned char *)&tmp, &txinsn.fr_len);

    // Set Mesh Header's Sequence number, Haven't Address Extension, so Mesh header set here only.
    seq = getMeshSeq(priv);
    seq = cpu_to_le16(seq);
    pbuf = set_fixed_ie(pbuf , 2 , (unsigned char *)&seq, &txinsn.fr_len);

    // Set Mesh Action field (Category)	NOTE: total= 2 bytes
    *pbuf = _MESH_CATEGORY_ID_;
    pbuf += 1;
    // Set Mesh Action field (Action)
    *pbuf = _MESH_LINK_REPORT_ACTION_ID_;
    pbuf += 1;
    txinsn.fr_len += _MESH_ACTIVE_FIELD_OFFSET_;

    // Set LOCAL_LINK_STATE_ANNOU_IE
    pbuf = set_ie(pbuf, _MESH_LINK_METRIC_REPORT_IE_, mesh_ie_LocalLinkStateANNOU(priv, pfrinfo, pstat, meshiearray), meshiearray, &txinsn.fr_len);

    // D1.06 shall be use WIFI_ACTION
    SetFrameSubType((txinsn.phdr), WIFI_11S_MESH_ACTION);

    memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);
    memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
    memcpy((void *)GetAddr3Ptr((txinsn.phdr)), pstat->hwaddr, MACADDRLEN);	// Don't know how to set, How category? (Refer: Draft 1.06, Page 12, 7.2.3, Line 21~30 2007/08/11 by popen)

    if ((DRV_FIRETX(priv, &txinsn)) == SUCCESS)
        return;

issue_LocalLinkStateANNOU_MP_fail:
    if (txinsn.phdr)
        release_wlanhdr_to_poll(priv, txinsn.phdr);
    if (txinsn.pframe)
        release_mgtbuf_to_poll(priv, txinsn.pframe);
}


//pepsi 08/02/29
#ifdef PU_STANDARD

/**
 *	@brief	STA issue proxy update to all mesh nodes
 *
 */

void  issue_proxyupdate_ADD(DRV_PRIV *priv, unsigned char * proxiedmac)
{
    unsigned long flags = 0;
    
    struct proxyupdate_table_entry puEntry;
    struct path_sel_entry* pPathEntry;
    int i, tbl_sz;


    puEntry.retry = 1U;
    puEntry.STAcount = 0x0001;
    memcpy((void *)puEntry.proxymac , priv->pmib->dot11OperationEntry.hwaddr ,MACADDRLEN);
    memcpy((void *)puEntry.proxiedmac , proxiedmac ,MACADDRLEN);

    // PROXY ADD (unicast to all mesh nodes)
    tbl_sz = 1 << priv->pathsel_table->table_size_power;
    for (i = 0; i < tbl_sz; i++)
    {
        puEntry.priv = NULL;
        
        SAVE_INT_AND_CLI(flags);
        SMP_LOCK_MESH_PATH(flags);
        if (priv->pathsel_table->entry_array[i].dirty)
        {
            pPathEntry = ((struct path_sel_entry*)priv->pathsel_table->entry_array[i].data);

            memcpy(puEntry.destproxymac, pPathEntry->destMAC, MACADDRLEN);
            memcpy(puEntry.nexthopmac, pPathEntry->nexthopMAC, MACADDRLEN);
            puEntry.isMultihop = 0x01;
            puEntry.PUflag = PU_add;
            puEntry.PUSN = getPUSeq(priv);
            puEntry.update_time = jiffies;
            puEntry.priv = pPathEntry->priv;
        }
        RESTORE_INT(flags);
        SMP_UNLOCK_MESH_PATH(flags);

        if(puEntry.priv) {
            SMP_LOCK_MESH_PROXYUPDATE(flags);
            HASH_INSERT(priv->proxyupdate_table, &puEntry.PUSN, &puEntry);
            SMP_UNLOCK_MESH_PROXYUPDATE(flags);
                
            issue_proxyupdate_MP(puEntry.priv, &puEntry);
        }
    }

}

/**
 *	@brief	STA issue proxy update
 *
 */
// Format of txinsn.pframe:
//  Addr4(6), meshheader(4), Category(1), ActionField(1)
//  IE_TAG(1), IE_LEN(1),
//  PU_Flag(1), PU_Sn(1), Proxymac(6), STACnt(2), proxiedMac(6)
void issue_proxyupdate_MP(DRV_PRIV *priv, struct proxyupdate_table_entry *Entry)
{
    unsigned short	seq;
    UINT8			tmp;
    unsigned char		*pbuf;
    UINT8			meshiearray[18]= {0};	// mesh IE buffer
//	struct proxyupdate_table_entry Entry;

    DECLARE_TXINSN(txinsn);

    txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;
    txinsn.is_11s = GET_MIB(priv)->dot1180211sInfo.mesh_enable;

    txinsn.q_num = MANAGE_QUE_NUM;
    txinsn.fr_type = _PRE_ALLOCMEM_;
    txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
    txinsn.lowest_tx_rate = txinsn.tx_rate;
    txinsn.fixed_rate = 1;	// D1.06 is data frame, SHALL be remove it.
    pbuf = txinsn.pframe  = get_mgtbuf_from_poll(priv);

    if (pbuf == NULL)
        goto issue_proxyupdate_MP_fail;

    txinsn.phdr = get_wlanhdr_from_poll(priv);

    if (txinsn.phdr == NULL)
        goto issue_proxyupdate_MP_fail;

    memset((void *)txinsn.phdr, 0, sizeof(struct  wlan_hdr));

    // It is managment frame
    SetFrDs(txinsn.phdr);
    SetToDs(txinsn.phdr);

    // Set Address 4 (SA)
    memcpy(pbuf, GET_MY_HWADDR, MACADDRLEN);
    pbuf += MACADDRLEN;
    txinsn.fr_len += MACADDRLEN;

    // Set Mesh Header's Mesh Flags
    pbuf = set_fixed_ie(pbuf, 1 , (unsigned char *)&(priv->mesh_HeaderFlags), &txinsn.fr_len);

    // Set Mesh Header's TTL (value depend on test)
    tmp = 1;
    pbuf = set_fixed_ie(pbuf, 1 , (unsigned char *)&tmp, &txinsn.fr_len);

    // Set Mesh Header's Sequence number, Haven't Address Extension, so Mesh header set here only.
    seq = getMeshSeq(priv);
    seq = cpu_to_le16(seq);
    pbuf = set_fixed_ie(pbuf , 2 , (unsigned char *)&seq, &txinsn.fr_len);

    // Set Mesh Action field (Category)	NOTE: total= 2 bytes
    *pbuf = _MULTIHOP_CATEGORY_ID_;
    pbuf += 1;
    // Set  Mesh Action field (Action)
    *pbuf = _PROXY_UPDATES_ACTION_ID_;
    pbuf += 1;
    txinsn.fr_len += _MESH_ACTIVE_FIELD_OFFSET_;

    pbuf = set_ie(pbuf, _PROXY_UPDATE_IE_, mesh_ie_ProxyUpdate(priv, Entry, meshiearray), meshiearray, &txinsn.fr_len);

    // D1.06 shall be use WIFI_ACTION
    SetFrameSubType((txinsn.phdr), WIFI_11S_MESH_ACTION);

    if( Entry->isMultihop )
    {
        memcpy((void *)GetAddr1Ptr((txinsn.phdr)), Entry->nexthopmac, MACADDRLEN);
        memcpy((void *)GetAddr3Ptr((txinsn.phdr)), Entry->destproxymac, MACADDRLEN);
    }
    else
    {
        memcpy((void *)GetAddr1Ptr((txinsn.phdr)), Entry->destproxymac, MACADDRLEN);
        memset((void *)GetAddr3Ptr((txinsn.phdr)), 0xff, MACADDRLEN);
    }

    memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);


    if ((DRV_FIRETX(priv, &txinsn)) == SUCCESS)
    {
        //printk("PU SN:%02lx was sent successsfully!! \n",Entry->PUSN);
        return;
    }

issue_proxyupdate_MP_fail:
    if (txinsn.phdr)
        release_wlanhdr_to_poll(priv, txinsn.phdr);
    if (txinsn.pframe)
        release_mgtbuf_to_poll(priv, txinsn.pframe);
}//end of issue_proxyupdate

/**
 *	@brief	STA issue proxy update confirmation
 *	by pepsi
 */
// Format of txinsn.pframe:
//  Addr4(6), meshheader(4), Category(1), ActionField(1)
//  IE_TAG(1), IE_LEN(1),
//  PU_Flag(1), PU_Sn(1), srcMac(6)
void issue_proxyupdateconfirm_MP(DRV_PRIV *priv, UINT8 *PUSN, unsigned char *srcMac, unsigned char *destMac, unsigned char* nexthopMac)
{
    unsigned short	seq;
    UINT8			tmp;
    unsigned char		*pbuf;
    UINT8			meshiearray[10];	// mesh IE buffer

    DECLARE_TXINSN(txinsn);

    txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;
    txinsn.is_11s = GET_MIB(priv)->dot1180211sInfo.mesh_enable;

    txinsn.q_num = MANAGE_QUE_NUM;
    txinsn.fr_type = _PRE_ALLOCMEM_;
    txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
    txinsn.lowest_tx_rate = txinsn.tx_rate;
    txinsn.fixed_rate = 1;	// D1.06 is data frame, SHALL be remove it.
    pbuf = txinsn.pframe  = get_mgtbuf_from_poll(priv);

    if (pbuf == NULL)
        goto issue_proxyupdateconfirm_MP_fail;

    txinsn.phdr = get_wlanhdr_from_poll(priv);

    if (txinsn.phdr == NULL)
        goto issue_proxyupdateconfirm_MP_fail;

    memset((void *)txinsn.phdr, 0, sizeof(struct  wlan_hdr));

    // It is managment frame
    SetFrDs(txinsn.phdr);
    SetToDs(txinsn.phdr);

    // Set Address 4 (SA)
    memcpy(pbuf, GET_MY_HWADDR, MACADDRLEN);
    pbuf += MACADDRLEN;
    txinsn.fr_len += MACADDRLEN;

    // Set Mesh Header's Mesh Flags
    pbuf = set_fixed_ie(pbuf, 1 , (unsigned char *)&(priv->mesh_HeaderFlags), &txinsn.fr_len);

    // Set Mesh Header's TTL(only 1 hop unicast, set "1")
    tmp = 1;
    pbuf = set_fixed_ie(pbuf, 1 , (unsigned char *)&tmp, &txinsn.fr_len);

    // Set Mesh Header's Sequence number, Haven't Address Extension, so Mesh header set here only.
    seq = getMeshSeq(priv);
    seq = cpu_to_le16(seq);
    pbuf = set_fixed_ie(pbuf , 2 , (unsigned char *)&seq, &txinsn.fr_len);

    // Set Mesh Action field (Category)	NOTE: total= 2 bytes
    *pbuf = _MULTIHOP_CATEGORY_ID_;
    pbuf += 1;
    // Set  Mesh Action field (Action)
    *pbuf = _PROXY_UPDATES_CONFIRM_ACTION_ID_;
    pbuf += 1;
    txinsn.fr_len += _MESH_ACTIVE_FIELD_OFFSET_;

    // Set PROXY_UPDATE_IE
    pbuf = set_ie(pbuf, _PROXY_UPDATE_CONFIRM_IE_, mesh_ie_ProxyUpdateConfirm(priv, PUSN, srcMac, meshiearray), meshiearray, &txinsn.fr_len);

    // D1.06 shall be use WIFI_ACTION
    SetFrameSubType((txinsn.phdr), WIFI_11S_MESH_ACTION);

    memcpy((void *)GetAddr1Ptr((txinsn.phdr)), nexthopMac, MACADDRLEN);
    memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
    memcpy((void *)GetAddr3Ptr((txinsn.phdr)), destMac, MACADDRLEN);

    if ((DRV_FIRETX(priv, &txinsn)) == SUCCESS)
    {
        return;
    }

issue_proxyupdateconfirm_MP_fail:
    if (txinsn.phdr)
        release_wlanhdr_to_poll(priv, txinsn.phdr);
    if (txinsn.pframe)
        release_mgtbuf_to_poll(priv, txinsn.pframe);
}
#endif


extern void pending_add_RATid(DRV_PRIV *priv, struct stat_info *pstat);
extern void add_RATid(DRV_PRIV *priv, struct stat_info *pstat);

/*
 *	@brief	Process Mesh Peer Link establish
 *
 *	@param	priv:	Driver private data
 *	@param	pstat:	station data
 *
 *	@retval	void
 */
static void mesh_PeerLinkEstablish(DRV_PRIV *priv,  struct stat_info *pstat)
{
    unsigned long		flags;
	struct path_sel_entry Entry;
    SAVE_INT_AND_CLI(flags);

    if (!list_empty(&pstat->mesh_mp_ptr))	// mesh_unEstablish_hdr -> mesh_mp_hdr
        list_del_init(&(pstat->mesh_mp_ptr));

#ifdef MESH_BOOTSEQ_AUTH
    if (!list_empty(&pstat->auth_list))
        list_del_init(&pstat->auth_list);
#endif

    // Determine superordinate or subordinate
    if (memcmp(GET_MY_HWADDR, pstat->hwaddr, MACADDRLEN) > 0)
        pstat->mesh_neighbor_TBL.State = MP_SUPERORDINATE_LINK_DOWN;
    else
        pstat->mesh_neighbor_TBL.State = MP_SUBORDINATE_LINK_DOWN_E;

    if( priv->pmib->dot11sKeysTable.dot11Privacy)
        dot11s_mp_set_key(priv, pstat->hwaddr);

#ifdef MESH_BOOTSEQ_STRESS_TEST
    // random test(for stress test)
    UINT16	tmp;
    get_random_bytes(&(tmp), 2);
    tmp = (tmp%(MESH_BS_STRESS_TEST_MAX_TIME - MESH_BS_STRESS_TEST_MIN_TIME)) + MESH_BS_STRESS_TEST_MIN_TIME;
    pstat->mesh_neighbor_TBL.expire = jiffies + (unsigned long)tmp;
    priv->mesh_stressTestCounter++;
    MESH_DEBUG_MSG("Mesh Stress Testing: MP establish, Current expire = %d (jiffies), Test count = %lu\n", tmp, priv->mesh_stressTestCounter);
#else	// MESH_BOOTSEQ_STRESS_TEST
    pstat->mesh_neighbor_TBL.expire = jiffies + MESH_EXPIRE_TO;
#endif	// MESH_BOOTSEQ_STRESS_TEST	

    pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies;	// Start Local Link State Announcement immediately.
    pstat->state |= WIFI_ASOC_STATE;	// Can't transfer data if absence.
    pstat->expire_to = priv->expire_to;

    if (list_empty(&pstat->asoc_list))
        list_add_tail(&pstat->asoc_list, &priv->asoc_list);

    list_add_tail(&(pstat->mesh_mp_ptr), &(priv->mesh_mp_hdr));

    RESTORE_INT(flags);

    cnt_assoc_num(priv, pstat, INCREASE, (char *)__FUNCTION__);
    mesh_cnt_ASSOC_PeerLink_CAP(priv, pstat, INCREASE);

#ifdef MESH_USE_METRICOP
    pstat->mesh_neighbor_TBL.metric = 0;
    pstat->mesh_neighbor_TBL.isAsym = 0;
    pstat->mesh_neighbor_TBL.retryMetric = 0;
    atomic_set(&pstat->mesh_neighbor_TBL.isMetricTesting, 0);
    {
        UINT32 r;
        get_random_bytes((void *)&r, sizeof(UINT32));
        r%=HZ; // max diff is 1 sec
        pstat->mesh_neighbor_TBL.timeMetricUpdate = jiffies + MESH_METRIC_PERIOD_UPDATE + r;
    }
    memset(&pstat->mesh_neighbor_TBL.spec11kv, 0, sizeof(pstat->mesh_neighbor_TBL.spec11kv));

    if(time_before(jiffies, priv->toMeshMetricAuto))
    {
        pstat->mesh_neighbor_TBL.timeMetricUpdate = jiffies + 5*HZ; // 5 secs later, a metric testing will be triggered
    }
#endif

    memset((void*)&Entry, 0, sizeof(struct path_sel_entry));
    memcpy(Entry.destMAC, pstat->hwaddr, MACADDRLEN);
    memcpy(Entry.nexthopMAC, pstat->hwaddr, MACADDRLEN);
	Entry.metric =99999;
	Entry.hopcount=1;
	Entry.update_time= jiffies;
	Entry.priv = priv;
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
	priv = priv->mesh_priv_first;
#endif	
	SMP_LOCK_MESH_PATH(flags);
	HASH_INSERT( priv->pathsel_table, Entry.destMAC, &Entry);
	SMP_UNLOCK_MESH_PATH(flags);
	
    MESH_DEBUG_MSG("Mesh: New MP is establish... MAC=%02X:%02X:%02X:%02X:%02X:%02X State=%d\n"
                   , pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2], pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5], pstat->mesh_neighbor_TBL.State);

    LOG_MESH_MSG("associate to MP:%02X:%02X:%02X:%02X:%02X:%02X successfully.\n"
                 , pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2], pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5]);
}

#ifdef MESH_BOOTSEQ_AUTH
/**
 *	@brief	Auth timer
 *
 *	@param	priv: unsigned long priv data (Request of timer)
 *	PS: Check pstat in auth state ONLY
 *
 *	@retval	void
 */
void mesh_auth_timer(unsigned long pVal)
{

 
#ifndef SMP_SYNC
    unsigned long		flags;
#endif

    DRV_PRIV *priv = (DRV_PRIV *)pVal;
    struct stat_info	*pstat;
    struct list_head	*phead, *plist, *pprevlist;

    SAVE_INT_AND_CLI(flags);
restart:
    phead= &priv->mesh_auth_hdr;
    plist = phead->next;
    pprevlist = phead;

    while(plist != phead)   // 1.Check index  2.Check is it least element? (Because next pointer to phead itself)
    {
        pstat = list_entry(plist, struct stat_info, mesh_mp_ptr); // Find process MP

        if (time_after(jiffies, pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod))
        {
            MESH_DEBUG_MSG("\nMesh: Auth retry expire!!, MAC:%02X:%02X:%02X:%02X:%02X:%02X, State=%d, Retry=%d\n"
                           , pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2], pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5], pstat->mesh_neighbor_TBL.State, pstat->mesh_neighbor_TBL.retry);

            if ((MP_OPEN_SENT == pstat->mesh_neighbor_TBL.State) && (pstat->mesh_neighbor_TBL.retry++ < MESH_AUTH_RETRY_LIMIT))
            {
                RESTORE_INT(flags);
                pstat->auth_seq = 1;
                issue_auth(priv, pstat, _STATS_SUCCESSFUL_);
                pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_AUTH_RETRY_TO;
            }
            else
            {
                RESTORE_INT(flags);
                free_stainfo(priv, pstat);
            }

            SAVE_INT_AND_CLI(flags);
        }

        // Check processing pstat had been delete or move? IF true, Restart process from phead (The mechanism work correct condition is plist never lost or modify position in system memory !!)
        // Check not use "next" pointer ,Because chain in new pstat at any moment, But don't affect anymore. (Use prev pointer more saftty).
        if (plist->prev != pprevlist)
            goto restart;

        plist = plist->next;
        pprevlist = plist->prev;
    }

    if (!(list_empty(phead)))
        mod_timer(&priv->mesh_auth_timer, jiffies + MESH_TIMER_TO);

    RESTORE_INT(flags);
}
#endif

/**
 *	@brief	Peer Link check timer (unestablish)
 *	Include Retry, Open
 *
 *	@param	priv: unsigned long priv data (Require by timer)
 *	PS: Establesh MP in mesh_mp_hdr, Check for 1 sec timer, So the function don't check established MP (mesh_unEstablish_hdr)
 *
 *	@retval	void
 */
void mesh_peer_link_timer(unsigned long pVal)
{
#ifndef SMP_SYNC
    unsigned long		flags;
#endif
    DRV_PRIV *priv = (DRV_PRIV *)pVal;
    struct stat_info *pstat;
    struct list_head	*phead, *plist;

    SAVE_INT_AND_CLI(flags);
    phead= &priv->mesh_unEstablish_hdr;
    plist = phead->next;

    while(plist != phead)   // 1.Check index  2.Check is it least element? (Because next pointer to phead itself)
    {
        pstat = list_entry(plist, struct stat_info, mesh_mp_ptr); // Find process MP

        if (time_after(jiffies, pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod))
        {

            MESH_DEBUG_MSG("\nMesh: Peer link retry expire!!, MAC:%02X:%02X:%02X:%02X:%02X:%02X, State=%d, Retry=%d\n"
                           , pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2], pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5], pstat->mesh_neighbor_TBL.State, pstat->mesh_neighbor_TBL.retry);

            switch (pstat->mesh_neighbor_TBL.State)
            {
                case MP_OPEN_SENT:	// TOR
                case MP_CONFIRM_SENT:
                    if (pstat->mesh_neighbor_TBL.retry++ < MESH_PEER_LINK_RETRY_LIMIT)
                    {
                        issue_assocreq_MP(priv, pstat);
                        pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_PEER_LINK_RETRY_TO;
                        pstat->mesh_neighbor_TBL.State = mesh_PeerLinkStatesTable[pstat->mesh_neighbor_TBL.State][TimeOut];
                    }
                    else     // retry exceeded
                    {
                        issue_disassoc_MP(priv, pstat, _RSON_UNSPECIFIED_ , EXCEED_MAXIMUM_RETRIES);
                        pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_PEER_LINK_CLOSE_TO;
                        pstat->mesh_neighbor_TBL.State = MP_HOLDING;
                    }
                    break;
                case MP_CONFIRM_RCVD:	// TOO
                    issue_disassoc_MP(priv, pstat, _RSON_UNSPECIFIED_, TIMEOUT);
                    pstat->mesh_neighbor_TBL.State = mesh_PeerLinkStatesTable[pstat->mesh_neighbor_TBL.State][TimeOut];
                    break;
                case MP_HOLDING:	// TOC
                    issue_disassoc_MP(priv, pstat, _RSON_UNSPECIFIED_, CLOSE_RECEIVED);
                    pstat->mesh_neighbor_TBL.State = mesh_PeerLinkStatesTable[pstat->mesh_neighbor_TBL.State][TimeOut];
                    break;
                default:	// include MP_UNUSED
                    pstat->mesh_neighbor_TBL.State = mesh_PeerLinkStatesTable[pstat->mesh_neighbor_TBL.State][TimeOut];
                    break;
            }

            /* popen's mess
            // change state behind free_stainfo haven't affect.
            if (TRUE != holdState)	{	// In mesh_PeerLinkStatesTable is mormal state, But special case ,set MP_HOLDING shall be keep MP_HOLDING state
            	pstat->mesh_neighbor_TBL.State = mesh_PeerLinkStatesTable[pstat->mesh_neighbor_TBL.State][TimeOut];	// change state
            	MESH_DEBUG_MSG("Mesh: Peer Link Timer: Next State=%d\n", pstat->mesh_neighbor_TBL.State);
            } else
            	holdState = FALSE;
            	*/
        }

        plist = plist->next;

        if (pstat->mesh_neighbor_TBL.State == MP_HOLDING
                || pstat->mesh_neighbor_TBL.State == MP_UNUSED)
        {
            free_stainfo(priv, pstat);
        }

    }

    if (!(list_empty(phead)))
        mod_timer(&priv->mesh_peer_link_timer, jiffies + MESH_TIMER_TO);

    RESTORE_INT(flags);
}


/**
 *	@brief	Peer Link start entry (Mesh Point ONLY!!)
 *
 *	@param	priv	: priv
 *	@param	pfrinfo
  *	@param	pstat
 *	@param	cap		: Current connect amount from sa.
 *	@param	OFDMparam: Current operation channel data from sa
 *
 *	@retval	result
 *	PS: The function call frequency, High utilization function(ex: acl_query) Don't put on high utilization path.
 */
int start_MeshPeerLink(DRV_PRIV *priv, struct rx_frinfo *pfrinfo, struct stat_info *pstat, UINT16 cap)
{

#ifndef SMP_SYNC
    unsigned long	flags;
#endif

#ifdef _MESH_ACL_ENABLE_
    unsigned int		res;
    struct list_head	*phead, *plist;
    struct wlan_acl_node *paclnode;
#endif

    // If allow peer reconnect(reset) in Draft 2.0, under shall be move inside of NULL == pstat.
    if (0 >= MESH_PEER_LINK_CAP_NUM(priv))
    {
        return FAIL;
    }

    if (NULL == pstat)
    {

#ifdef MESH_ESTABLISH_RSSI_THRESHOLD
        if (pfrinfo->rssi < priv->mesh_fake_mib.establish_rssi_threshold)
        {
            return SUCCESS;
        }
#endif

#ifdef _MESH_ACL_ENABLE_	// below code copy from OnAuth ACL
        // cache compare
        if (!memcmp((void *)pfrinfo->sa, priv->meshAclCacheAddr, MACADDRLEN) && (priv->meshAclCacheMode & 2))  	// 2 = deny
        {
            return	FAIL;
        }

        SAVE_INT_AND_CLI(flags);
        phead = &priv->mesh_acl_list;
        plist = phead->next;

        //check sa
        if (priv->pmib->dot1180211sInfo.mesh_acl_mode == 1)	// 1: positive check, only those on acl_list can be connected.
            res = FAIL;
        else
            res = SUCCESS;

        while(plist != phead)
        {
            paclnode = list_entry(plist, struct wlan_acl_node, list);
            plist = plist->next;
            if (!memcmp((void *)pfrinfo->sa, paclnode->addr, MACADDRLEN))
            {
                // cache update
                memcpy(priv->meshAclCacheAddr, pfrinfo->sa ,MACADDRLEN);
                priv->meshAclCacheMode = paclnode->mode;

                if (paclnode->mode & 2)   // deny
                {
                    res = FAIL;
                    break;
                }
                else
                {
                    res = SUCCESS;
                    break;
                }
            }
        }

        RESTORE_INT(flags);

        if (res != SUCCESS)
        {
            return FAIL;
        }
#endif	// _MESH_ACL_ENABLE_

        pstat = alloc_stainfo(priv, pfrinfo->sa, -1);

        if (NULL == pstat)
        {
            return FAIL;
        }

        SAVE_INT_AND_CLI(flags);

#ifdef MESH_BOOTSEQ_AUTH
reInitial:
#endif
        SET_PSEUDO_RANDOM_NUMBER(pstat->mesh_neighbor_TBL.LocalLinkID);
        pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_PEER_LINK_LISTEN_TO;	// PassivePeerLinkOpen (line 7-10, page 113,  D1.0)
        pstat->mesh_neighbor_TBL.State = mesh_PeerLinkStatesTable[pstat->mesh_neighbor_TBL.State][PassivePeerLinkOpen];

#ifdef MESH_BOOTSEQ_AUTH
        pstat->state = WIFI_AUTH_NULL;
        pstat->auth_seq = 1;
        pstat->expire_to = priv->auth_to;
#else
        pstat->state |= WIFI_AUTH_SUCCESS;
        pstat->auth_seq = 0;
        pstat->expire_to = priv->assoc_to;
#endif

        list_add_tail(&(pstat->mesh_mp_ptr), &(priv->mesh_unEstablish_hdr));

        if (!(timer_pending(&priv->mesh_peer_link_timer)))		// start timer if stop
            mod_timer(&priv->mesh_peer_link_timer, jiffies + MESH_TIMER_TO);

        RESTORE_INT(flags);

        pstat->mesh_neighbor_TBL.Co = priv->pmib->dot11RFEntry.dot11channel;
    }
    else  	// HIGH utilization path !!
    {
#ifdef MESH_BOOTSEQ_AUTH
        if (isPossibleNeighbor(pstat) && (((WIFI_ASOC_STATE | WIFI_AUTH_SUCCESS) & pstat->state) || (MP_LISTEN != pstat->mesh_neighbor_TBL.State)))
            return SUCCESS;

        // Here: If Neighbor discovery don't detect peer MP, But peer MP request AUTH (But filter by BSS), Later.. Neighbor discovery detect peer MP.
        else if (!isPossibleNeighbor(pstat))
        {
            MESH_DEBUG_MSG("Mesh: Pstat created, But doesn't from Neighbor Discovery, Reinitial.\n");
            SAVE_INT_AND_CLI(flags);

            if (!list_empty(&pstat->asoc_list))
                cnt_assoc_num(priv, pstat, DECREASE, __FUNCTION__);

            release_stainfo(priv, pstat);
            init_stainfo(priv, pstat);
            pstat->mesh_neighbor_TBL.Co = priv->pmib->dot11RFEntry.dot11channel;	// Totti test  2008.08.06
            goto reInitial;
        }
#else

        if (MP_LISTEN != pstat->mesh_neighbor_TBL.State)
        {
            return SUCCESS;
        }
#endif
    }

    // Timer always activity !!, Pstat always creat !!, Listen Only !!
    if (0 < cap)  	// ActivePeerLinkOpen (line 10-14, page 113,  D1.0)
    {
        pstat->mesh_neighbor_TBL.retry = 0;
        pstat->mesh_neighbor_TBL.State = mesh_PeerLinkStatesTable[pstat->mesh_neighbor_TBL.State][ActivePeerLinkOpen];

#ifdef MESH_BOOTSEQ_AUTH
        SAVE_INT_AND_CLI(flags);
        if (!list_empty(&(pstat->mesh_mp_ptr)))
            list_del_init(&(pstat->mesh_mp_ptr));

        list_add_tail(&(pstat->mesh_mp_ptr), &(priv->mesh_auth_hdr));

        if (!(timer_pending(&priv->mesh_auth_timer)))	// start timer if stop
            mod_timer(&priv->mesh_auth_timer, jiffies + MESH_TIMER_TO);

        RESTORE_INT(flags);
        pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_AUTH_RETRY_TO;
        issue_auth(priv, pstat, (unsigned short)(_STATS_SUCCESSFUL_));
#else
        pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_PEER_LINK_RETRY_TO;
        issue_assocreq_MP(priv, pstat);
#endif
    }

    return	SUCCESS;
}



/**
 *	@brief	Peer Link close connect.
 *
 *	@param	priv: priv
 *	@param	da: close connect MP MAC Address
 *
 *	@retval	result
 */
int close_MeshPeerLink(DRV_PRIV *priv, UINT8 *da)
{
    struct stat_info *pstat;
#ifndef SMP_SYNC    
    unsigned long	flags;
#endif
    pstat = get_stainfo(priv, da);
    if (NULL == pstat)
        return FAIL; // after a while, peer will expire this machine; and they will re-connect

    switch(pstat->mesh_neighbor_TBL.State)  	// ( D1.0, Page.116~120)
    {
        case MP_SUBORDINATE_LINK_UP:
        case MP_SUPERORDINATE_LINK_UP:
        case MP_SUBORDINATE_LINK_DOWN_E:
        case MP_SUPERORDINATE_LINK_DOWN:
            cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
            mesh_cnt_ASSOC_PeerLink_CAP(priv, pstat, DECREASE);
        // No break, pass through
        case MP_OPEN_SENT:
        case MP_CONFIRM_RCVD:
        case MP_CONFIRM_SENT:
            issue_disassoc_MP(priv, pstat, _RSON_UNSPECIFIED_, CANCELLED);

            SAVE_INT_AND_CLI(flags);
            if (!list_empty(&pstat->mesh_mp_ptr))	// mesh_mp_hdr | mesh_unEstablish_hdr -> mesh_unEstablish_hdr
                list_del_init(&(pstat->mesh_mp_ptr));

            list_add_tail(&(pstat->mesh_mp_ptr), &(priv->mesh_unEstablish_hdr));

            if (!(timer_pending(&priv->mesh_peer_link_timer)))	// start timer if stop
                mod_timer(&priv->mesh_peer_link_timer, jiffies + MESH_TIMER_TO);

            pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_PEER_LINK_CLOSE_TO;
            RESTORE_INT(flags);

            break;
        case MP_HOLDING:
            break;
        case MP_LISTEN:
        default:	// include MP_UNUSED
            free_stainfo(priv, pstat);
            return FAIL;
    }

    pstat->mesh_neighbor_TBL.State = mesh_PeerLinkStatesTable[pstat->mesh_neighbor_TBL.State][CancelPeerLink];	// change state
    return SUCCESS;
}

#ifdef SIMPLE_CH_UNI_PROTOCOL
static void mesh_auto_channel_check(DRV_PRIV *priv, struct rx_frinfo *pfrinfo, UINT32 precedence,int force_apply)
{
    int len;
    unsigned char *p, *pframe;
    UINT8	OFDMparam, fix_channel;
    UINT8 offset_2nd_chan;
    int apply_channel = 0;

    pframe = get_pframe(pfrinfo);

    // OFDM parameter set
    p = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBERSP_IE_OFFSET_, _OFDM_PARAMETER_SET_IE_, &len,
               pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBERSP_IE_OFFSET_);
    if (NULL== p)
        return;

    OFDMparam = *(p + MESH_IE_BASE_LEN);
    offset_2nd_chan = *(p + MESH_IE_BASE_LEN + 1);
    fix_channel = *(p + MESH_IE_BASE_LEN + 2);

    if(force_apply)
    {
        apply_channel = 1;
    }
    else if(priv->mesh_swchnl_channel == 0)
    {
        if(fix_channel == priv->mesh_fix_channel)
        {
            if(priv->mesh_ChannelPrecedence < precedence ||
                    (priv->mesh_ChannelPrecedence == precedence && memcmp(GET_MY_HWADDR, pfrinfo->sa, MACADDRLEN)<0 ))
            {
                apply_channel = 1;
            }
        }
        else if(fix_channel == 1 && priv->mesh_fix_channel == 0)
        {
            apply_channel = 1;
        }
    }

    if(apply_channel)
    {
        priv->pmib->dot11RFEntry.dot11channel = OFDMparam;
        priv->mesh_ChannelPrecedence = precedence;
        priv->mesh_fix_channel = fix_channel;


        if(priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_20_40 ||
            priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_80) {
            #if defined(RTK_5G_SUPPORT) 
            if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
                if((priv->pmib->dot11RFEntry.dot11channel>144) ? ((priv->pmib->dot11RFEntry.dot11channel-1)%8) : (priv->pmib->dot11RFEntry.dot11channel%8)) {
                    priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_ABOVE;
                    priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;
                } else {
                    priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_BELOW;
                    priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
                }
            }
            else 
            #endif
            {
                if(offset_2nd_chan) {
                    GET_MIB(priv)->dot11nConfigEntry.dot11n2ndChOffset = offset_2nd_chan;
                    priv->pshare->offset_2nd_chan = offset_2nd_chan;
                }
                else {
                    if ((priv->pmib->dot11RFEntry.dot11channel < 5) &&
                            (priv->pmib->dot11nConfigEntry.dot11n2ndChOffset == HT_2NDCH_OFFSET_BELOW))
                        priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;
                    else if ((priv->pmib->dot11RFEntry.dot11channel > 9) &&
                            (priv->pmib->dot11nConfigEntry.dot11n2ndChOffset == HT_2NDCH_OFFSET_ABOVE))
                        priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
                }
            }
        }        
    }
}
#endif

/**
 *	@brief	Recived Assocation Request routine
 *		PS: Don't create stat_info when recived is not exist MP
 *	@param	priv:priv
 *	@param	pfrinfo: Recived frame
 *
 *	@retval	unsigned int:result
 */
unsigned int OnAssocReq_MP(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
    struct wifi_mib		*pmib;
    struct stat_info	*pstat;
    unsigned char		*pframe, *p;
#ifdef RTL_WPA2
#endif
    int		len;

#ifndef SMP_SYNC
    unsigned long		flags;
#endif
    unsigned char		supportRate[32];
    int					supportRateNum;
    unsigned int		status = _STATS_SUCCESSFUL_;
    unsigned short		frame_type, ie_offset=0, val16;

    UINT32	recvLocalLinkID;
    UINT8	deniedPeerLink = FALSE, establish = FALSE, is_11s = TRUE;
    UINT16	cap = 0; // peer capacity


    pmib = GET_MIB(priv);
    pframe = get_pframe(pfrinfo);
    pstat = get_stainfo(priv, GetAddr2Ptr(pframe));
  

    if (!(OPMODE & WIFI_AP_STATE)
            || (pmib->dot11sKeysTable.dot11Privacy && pmib->dot11sKeysTable.keyInCam == FALSE ))
        return FAIL;

#ifdef WDS
    if (((OPMODE & WIFI_AP_STATE) && (pmib->dot11WdsInfo.wdsPure)) && (0 == GET_MIB(priv)->dot1180211sInfo.mesh_enable))
        return FAIL;
#endif	//WDS

    frame_type = GetFrameSubType(pframe);
    if (frame_type == WIFI_ASSOCREQ)
        ie_offset = _ASOCREQ_IE_OFFSET_;
    else // WIFI_REASSOCREQ
        ie_offset = _REASOCREQ_IE_OFFSET_;

    if (pstat == (struct stat_info *)NULL)
    {
        /* Avoid race condition (MP-A sense MP-B and send assocREQ,
                     MP-B get it but haven't sense MP-A, MP-B send deauth to MP-A, MP-A free_stainfo MP-B
                     next, MP-B sense MP-A send assocREQ, MP-B get it but sta_info is removed,
                     MP-A send deauth to MP-B,  MP-B free_stainfo MP-A...LOOP
            */
        if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable)
            return SUCCESS;
        status = _RSON_CLS2_;
        goto asoc_MP_class2_error; //issue deauth
    }

    // check if this stat has been successfully authenticated/assocated
    if (!((pstat->state) & WIFI_AUTH_SUCCESS))
    {
        if (isPossibleNeighbor(pstat))
        {
            status = _STATS_FAILURE_;
            goto OnAssocReq_MP_Fail; //issue assocrsp
        }
        else
        {
            status = _RSON_CLS2_;
            goto asoc_MP_class2_error; //issue deauth
        }
    }

    if (priv->assoc_reject_on)
    {
        status = _STATS_OTHER_;
        goto OnAssocReq_MP_Fail; //issue assocrsp
    }

    // now we should check all the fields...

    // checking SSID
    p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _SSID_IE_, &len,
               pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);

    if (p == NULL)
    {
        status = _STATS_FAILURE_;
        goto OnAssocReq_MP_Fail; //issue assocrsp
    }


    if (isSTA(pstat))	// STA only
    {
        if (len == 0) // broadcast ssid, however it is not allowed in assocreq (Mesh don't care SSID)
            status = _STATS_FAILURE_;
        else
        {
            // check if ssid match
            if (memcmp((void *)(p+2), SSID, SSID_LEN))
                status = _STATS_FAILURE_;

            if (len != SSID_LEN)
                status = _STATS_FAILURE_;
        }
    }

    // check if the supported is ok
    p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _SUPPORTEDRATES_IE_, &len,
               pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);

    if (p == NULL)
    {
        DEBUG_WARN("Rx a sta assoc-req which supported rate is empty!\n");
        // use our own rate set as statoin used
        memcpy(supportRate, AP_BSSRATE, AP_BSSRATE_LEN);
        supportRateNum = AP_BSSRATE_LEN;
    }
    else
    {
        memcpy(supportRate, p+2, len);
        supportRateNum = len;

        p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _EXT_SUPPORTEDRATES_IE_ , &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);
        if (p !=  NULL)
        {
            memcpy(supportRate+supportRateNum, p+2, len);
            supportRateNum += len;
        }
    }

#ifdef __DRAYTEK_OS__
    if (status == _STATS_SUCCESSFUL_)
    {
        status = cb_assoc_request(priv->dev, GetAddr2Ptr(pframe), pframe + WLAN_HDR_A3_LEN + _ASOCREQ_IE_OFFSET_,
                                  pfrinfo->pktlen-WLAN_HDR_A3_LEN-_ASOCREQ_IE_OFFSET_);
        if (status != _STATS_SUCCESSFUL_)
        {
            DEBUG_ERR("\rReject association from draytek OS, status=%d!\n", status);
            goto OnAssocReq_MP_Fail;
        }
    }
#endif

    get_matched_rate(priv, supportRate, &supportRateNum, 0);
    update_support_rate(pstat, supportRate, supportRateNum);

    // capability field
    val16 = cpu_to_le16(*(unsigned short*)((unsigned int)pframe + WLAN_HDR_A3_LEN));
    if (!(val16 & BIT(5))) // NOT use short preamble
        pstat->useShortPreamble = 0;
    else
        pstat->useShortPreamble = 1;

    if (isSTA(pstat))	// STA only
        pstat->state |= WIFI_ASOC_STATE;

#ifdef RTL8190_FASTEXTDEV_FUNCALL
    rtl865x_extDev_addHost(	pstat->hwaddr,
                            priv->WLAN_VLAN_ID,
                            priv->dev,
                            (1 << priv->rtl8650extPortNum),
#ifdef WDS
                            priv->rtl8650linkNum[(getWdsIdxByDev(priv, priv->dev) >= 0)?(1+getWdsIdxByDev(priv, priv->dev)):0]
#else
                            priv->rtl8650linkNum[0]
#endif
                          );
#endif /* RTL8190_FASTEXTDEV_FUNCALL */

    if (status != _STATS_SUCCESSFUL_)
        goto OnAssocReq_MP_Fail;

    // now the station is qualified to join our BSS...

#ifdef WIFI_WMM
    // check if there is WMM IE
    if (QOS_ENABLE && (isSTA(pstat)))	// STA only
    {
        p = pframe + WLAN_HDR_A3_LEN + ie_offset;
        len = 0;
        for (;;)
        {
            p = get_ie(p, _RSN_IE_1_, &len,
                       pfrinfo->pktlen - (p - pframe));
            if (p != NULL)
            {
                if (!memcmp(p+2, WMM_IE, 6))
                {
                    pstat->QosEnabled = 1;
#ifdef WMM_APSD
                    if (APSD_ENABLE)
                        pstat->apsd_bitmap = *(p+8) & 0x0f;		// get QSTA APSD bitmap
#endif
                    break;
                }
            }
            else
            {
                pstat->QosEnabled = 0;
#ifdef WMM_APSD
                pstat->apsd_bitmap = 0;
#endif
                break;
            }
            p = p + len + 2;
        }
    }
    else
    {
        pstat->QosEnabled = 0;
#ifdef WMM_APSD
        pstat->apsd_bitmap = 0;
#endif
    }
#endif

#ifdef RTK_AC_SUPPORT
    if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)
    {
        p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, EID_VHTCapability, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);

        if ((p !=  NULL) && (len <= sizeof(struct vht_cap_elmt)))
        {
            pstat->vht_cap_len = len;
            memcpy((unsigned char *)&pstat->vht_cap_buf, p+2, len);
        }

        p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, EID_VHTOperation, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);

        if ((p !=  NULL) && (len <= sizeof(struct vht_oper_elmt)))
        {
            pstat->vht_oper_len = len;
            memcpy((unsigned char *)&pstat->vht_oper_buf, p+2, len);
        }
    }
#endif


    if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
    {
        p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _HT_CAP_, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);
        if (p !=  NULL)
        {
            pstat->ht_cap_len = len;
            memcpy((unsigned char *)&pstat->ht_cap_buf, p+2, len);
        }
        else
        {
            unsigned char old_ht_ie_id[] = {0x00, 0x90, 0x4c};
            p = pframe + WLAN_HDR_A3_LEN + ie_offset;
            len = 0;
            for (;;)
            {
                p = get_ie(p, _RSN_IE_1_, &len,
                           pfrinfo->pktlen - (p - pframe));
                if (p != NULL)
                {
                    if (!memcmp(p+2, old_ht_ie_id, 3) && (*(p+5) == 0x33))
                    {
                        pstat->ht_cap_len = len - 4;
                        memcpy((unsigned char *)&pstat->ht_cap_buf, p+6, pstat->ht_cap_len);
                        break;
                    }
                }
                else
                    break;

                p = p + len + 2;
            }
        }

        //AC mode only, deny N mode STA
#ifdef RTK_AC_SUPPORT
        if (!pstat->vht_cap_len && (priv->pmib->dot11StationConfigEntry.legacySTADeny & (WIRELESS_11N )))
        {
            DEBUG_ERR("AC mode only, deny non-AC STA association!\n");
            status = _STATS_RATE_FAIL_;
            goto OnAssocReq_MP_Fail;
        }
#endif

        if (pstat->ht_cap_len)
        {
            // below is the process to check HT MIMO power save
            unsigned char mimo_ps = ((cpu_to_le16(pstat->ht_cap_buf.ht_cap_info)) >> 2)&0x0003;
            pstat->MIMO_ps = 0;
            if (!mimo_ps)
                pstat->MIMO_ps |= _HT_MIMO_PS_STATIC_;
            else if (mimo_ps == 1)
                pstat->MIMO_ps |= _HT_MIMO_PS_DYNAMIC_;
            if (cpu_to_le16(pstat->ht_cap_buf.ht_cap_info) & _HTCAP_AMSDU_LEN_8K_)
            {
                pstat->is_8k_amsdu = 1;
                pstat->amsdu_level = 7935 - sizeof(struct wlan_hdr);
            }
            else
            {
                pstat->is_8k_amsdu = 0;
                pstat->amsdu_level = 3839 - sizeof(struct wlan_hdr);
            }
            if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))
                pstat->tx_bw = HT_CHANNEL_WIDTH_20_40;
#ifdef RTK_AC_SUPPORT
            if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) && (pstat->vht_cap_len))
            {
                switch(cpu_to_le32(pstat->vht_cap_buf.vht_cap_info) & 0x3)
                {
                    default:
                    case 0:
                        pstat->is_8k_amsdu = 0;
                        pstat->amsdu_level = 3895 - sizeof(struct wlan_hdr);
                        break;
                    case 1:
                        pstat->is_8k_amsdu = 1;
                        pstat->amsdu_level = 7991 - sizeof(struct wlan_hdr);
                        break;
                    case 2:
                        pstat->is_8k_amsdu = 1;
                        pstat->amsdu_level = 11454 - sizeof(struct wlan_hdr);
                        break;
                }
            }
#endif

        }

#ifdef RTK_AC_SUPPORT
        if((priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC))
        {
            if (pstat->vht_cap_len && (priv->vht_oper_buf.vht_oper_info[0] == 1))
                pstat->tx_bw = HT_CHANNEL_WIDTH_80;

            p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, EID_VHTOperatingMode, &len,
                       pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);
            if ((p !=  NULL) && (len == 1))
            {
                // check self capability....
                if((p[2] &3) <= priv->pshare->CurrentChannelBW)
                    pstat->tx_bw = p[2] &3;
                pstat->nss = ((p[2]>>4)&0x7)+1;
            }


        }
#endif

    }

#ifdef WIFI_WMM
    if (QOS_ENABLE)
    {
        if ((pstat->QosEnabled == 0) && pstat->ht_cap_len)
        {
            DEBUG_INFO("STA supports HT but doesn't support WMM, force WMM supported\n");
            pstat->QosEnabled = 1;
        }
    }
#endif




    SAVE_INT_AND_CLI(flags);
    if (list_empty(&pstat->asoc_list))
    {
        if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
        {
            construct_ht_ie(priv, priv->pshare->is_40m_bw, priv->pshare->offset_2nd_chan);
        }
    }
    RESTORE_INT(flags);


    // Realtek proprietary IE
    p = pframe + WLAN_HDR_A3_LEN + ie_offset;
    len = 0;
    for (;;)
    {
        p = get_ie(p, _RSN_IE_1_, &len,
                   pfrinfo->pktlen - (p - pframe));
        if (p != NULL)
        {
            if (!memcmp(p+2, Realtek_OUI, 3))
            {
                if (*(p+2+3) == 2)
                {
                    pstat->is_realtek_sta = TRUE;
                    pstat->IOTPeer = HT_IOT_PEER_REALTEK;

                    if (*(p+2+3+2) & RTK_CAP_IE_AP_CLIENT)
                        pstat->IOTPeer = HT_IOT_PEER_RTK_APCLIENT;

                    if (*(p+2+3+2) & RTK_CAP_IE_USE_AMPDU)
                        pstat->is_forced_ampdu = TRUE;
                    else
                        pstat->is_forced_ampdu = FALSE;
#ifdef RTK_WOW
                    if (*(p+2+3+2) & RTK_CAP_IE_USE_WOW)
                        pstat->IOTPeer = HT_IOT_PEER_REALTEK_WOW;
#endif
                    if (*(p+2+3+2) & RTK_CAP_IE_WLAN_88C92C)
                        pstat->IOTPeer = HT_IOT_PEER_REALTEK_81XX;
                }
                else
                {
                    pstat->is_realtek_sta = FALSE;
                    pstat->IOTPeer = HT_IOT_PEER_UNKNOWN;
                }
                break;
            }
        }
        else
            break;

        p = p + len + 2;
    }

    assign_tx_rate(priv, pstat, pfrinfo);
#if defined(WIFI_11N_2040_COEXIST_EXT)  
    update_40m_staMap(priv, pstat, 0);
    checkBandwidth(priv);
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
    if(GET_CHIP_VER(priv)== VERSION_8812E)
    {
        UpdateHalRAMask8812(priv, pstat, 3);
    }
#endif

#ifdef  CONFIG_WLAN_HAL
    if (IS_HAL_CHIP(priv))
    {
        GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
    }
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
    if (GET_CHIP_VER(priv)==VERSION_8188E)
    {
#ifdef TXREPORT
        add_RATid(priv, pstat);
#endif
    }
    else
#endif
    {
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
        add_update_RATid(priv, pstat);
#endif
    }
    assign_aggre_mthod(priv, pstat);
    assign_aggre_size(priv, pstat);


    // Customer proprietary IE
    if (priv->pmib->miscEntry.private_ie_len)
    {
        p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, priv->pmib->miscEntry.private_ie[0], &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);
        if (p)
        {
            memcpy(pstat->private_ie, p, len + 2);
            pstat->private_ie_len = len + 2;
        }
    }

    DEBUG_INFO("%s %02X%02X%02X%02X%02X%02X\n",
               (frame_type == WIFI_ASSOCREQ)? "OnAssocReq" : "OnReAssocReq",
               pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);

    /* 1. If 802.1x enabled, get RSN IE (if exists) and indicate ASSOIC_IND event
     * 2. Set dot118021xAlgrthm, dot11PrivacyAlgrthm in pstat
     */

    // Note: DOT11_EVENT_ASSOCIATION_IND is not necessary for MESH, because
    //		 802.11s has HELLO message to actively expire a mobile mesh device

    /* closed
    	if (IEEE8021X_FUN || IAPP_ENABLE || priv->pmib->wscEntry.wsc_enable)
    	{
    		p = pframe + WLAN_HDR_A3_LEN + ie_offset; len = 0;
    		for(;;)
    		{
    #ifdef RTL_WPA2
    			char tmpbuf[128];
    			int buf_len=0;
    			p = get_rsn_ie(priv, p, &len,
    				pfrinfo->pktlen - (p - pframe));

    			buf_len = sprintf(tmpbuf, "RSNIE len = %d, p = %s", len, (p==NULL? "NULL":"non-NULL"));
    			if (p != NULL)
    				buf_len += sprintf(tmpbuf+buf_len, ", ID = %02X\n", *(unsigned char *)p);
    			else
    				buf_len += sprintf(tmpbuf+buf_len, "\n");
    			DEBUG_INFO("%s", tmpbuf);
    #else
    			p = get_ie(p, _RSN_IE_1_, &len,
    				pfrinfo->pktlen - (p - pframe));
    #endif

    			if (p == NULL)
    				break;

    #ifdef RTL_WPA2
    			if ((*(unsigned char *)p == _RSN_IE_1_) && (len >= 4) && (!memcmp((void *)(p + 2), (void *)rsnie_hdr, 4)))
    				break;

    			if ((*(unsigned char *)p == _RSN_IE_2_) && (len >= 2) && (!memcmp((void *)(p + 2), (void *)rsnie_hdr_wpa2, 2)))
    				break;
    #else
    			if ((len >= 4) && (!memcmp((void *)(p + 2), (void *)rsnie_hdr, 4)))
    				break;
    #endif

    			p = p + len + 2;
    		}

    #ifdef WIFI_SIMPLE_CONFIG
    		if (priv->pmib->wscEntry.wsc_enable & 2) { // work as AP (not registrar)
    			unsigned char *ptmp;
    			unsigned int lentmp;
    			unsigned char passWscIE=0;
    			DOT11_WSC_ASSOC_IND wsc_Association_Ind;

    			ptmp = pframe + WLAN_HDR_A3_LEN + ie_offset; lentmp = 0;
    			for (;;)
    			{
    				ptmp = get_ie(ptmp, _WPS_IE_, &lentmp,
    					pfrinfo->pktlen - (ptmp - pframe));
    				if (ptmp != NULL) {
    					if (!memcmp(ptmp+2, WSC_IE_OUI, 4)) {
    						ptmp = search_wsc_tag(ptmp+2+4, TAG_REQUEST_TYPE, lentmp-4, &lentmp);
    						if (ptmp && (*ptmp <= MAX_REQUEST_TYPE_NUM)) {
    							DEBUG_INFO("WSC IE TAG_REQUEST_TYPE = %d has been found\n", *ptmp);
    							passWscIE = 1;
    						}
    						break;
    					}
    				}
    				else
    					break;

    				ptmp = ptmp + lentmp + 2;
    			}

    			memset(&wsc_Association_Ind, 0, sizeof(DOT11_WSC_ASSOC_IND));
    			wsc_Association_Ind.EventId = DOT11_EVENT_WSC_ASSOC_REQ_IE_IND;
    			memcpy((void *)wsc_Association_Ind.MACAddr, (void *)GetAddr2Ptr(pframe), MACADDRLEN);
    			if (passWscIE) {
    				wsc_Association_Ind.wscIE_included = 1;
    				wsc_Association_Ind.AssocIELen = lentmp + 2;
    				memcpy((void *)wsc_Association_Ind.AssocIE, (void *)(ptmp), wsc_Association_Ind.AssocIELen);
    			}
    			else {
    				if (IEEE8021X_FUN &&
    					(pstat->AuthAlgrthm == _NO_PRIVACY_) && // authentication is open
    					(p == NULL)) { // No SSN or RSN IE
    					wsc_Association_Ind.wscIE_included = 1; //treat this case as WSC IE included
    					DEBUG_INFO("Association : auth open; no SSN or RSN IE\n");
    				}
    			}

    			if ((wsc_Association_Ind.wscIE_included == 1) || !IEEE8021X_FUN)
    				DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&wsc_Association_Ind,
    						sizeof(DOT11_WSC_ASSOC_IND));

    			if (wsc_Association_Ind.wscIE_included == 1) {
    				pstat->state |= WIFI_WPS_JOIN;
    				goto OnAssocReqSuccess;
    			}
    // Brad add for DWA-652 WPS interoperability 2008/03/13--------
    			if ((pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
         				pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_) &&
         				!IEEE8021X_FUN)
    				pstat->state |= WIFI_WPS_JOIN;
    //------------------------- end

    		}
    #endif
    	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
    		(pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _NO_PRIVACY_))
    	{
    		int mask_mcs_rate = 0;
    		if 	((pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_) ||
    			 (pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_))
    			mask_mcs_rate = 2;
    		else {
    			if (p == NULL)
    				mask_mcs_rate = 1;
    			else {
    				if (*p == _RSN_IE_1_) {
    					if (is_support_wpa_aes(priv,  p, len+2) != 1)
    						mask_mcs_rate = 1;
    				}
    				else if (*p == _RSN_IE_2_) {
    					if (is_support_wpa2_aes(priv,  p, len+2) != 1)
    						mask_mcs_rate = 1;
    				}
    				else
    						mask_mcs_rate = 1;
    			}
    		}

    		if (mask_mcs_rate) {
    			pstat->is_legacy_encrpt = mask_mcs_rate;
    			assign_tx_rate(priv, pstat, pfrinfo);
    #ifdef CONFIG_RTL_88E_SUPPORT
                if (GET_CHIP_VER(priv)==VERSION_8188E) {
    #ifdef TXREPORT
    				add_RATid(priv, pstat);
    #endif
                } else
    #endif
                {
    #if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
                    add_update_RATid(priv, pstat);
    #endif
    		}
    	}
    #endif

    #ifndef WITHOUT_ENQUEUE
    		if (frame_type == WIFI_ASSOCREQ)
    		{
    			memcpy((void *)Association_Ind.MACAddr, (void *)GetAddr2Ptr(pframe), MACADDRLEN);
    			Association_Ind.EventId = DOT11_EVENT_ASSOCIATION_IND;
    			Association_Ind.IsMoreEvent = 0;
    			if (p == NULL)
    				Association_Ind.RSNIELen = 0;
    			else
    			{
    				DEBUG_INFO("assoc indication rsnie len=%d\n", len);
    #ifdef RTL_WPA2
    				// inlcude ID and Length
    				Association_Ind.RSNIELen = len + 2;
    				memcpy((void *)Association_Ind.RSNIE, (void *)(p), Association_Ind.RSNIELen);
    #else
    				Association_Ind.RSNIELen = len;
    				memcpy((void *)Association_Ind.RSNIE, (void *)(p + 2), len);
    #endif
    			}
    			// indicate if 11n sta associated
    			Association_Ind.RSNIE[MAXRSNIELEN-1] = ((pstat->ht_cap_len==0) ? 0 : 1);

    			DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&Association_Ind,
    						sizeof(DOT11_ASSOCIATION_IND));
    		}
    		else
    		{
    			memcpy((void *)Reassociation_Ind.MACAddr, (void *)GetAddr2Ptr(pframe), MACADDRLEN);
    			Reassociation_Ind.EventId = DOT11_EVENT_REASSOCIATION_IND;
    			Reassociation_Ind.IsMoreEvent = 0;
    			if (p == NULL)
    				Reassociation_Ind.RSNIELen = 0;
    			else
    			{
    				DEBUG_INFO("assoc indication rsnie len=%d\n", len);
    #ifdef RTL_WPA2
    				// inlcude ID and Length
    				Reassociation_Ind.RSNIELen = len + 2;
    				memcpy((void *)Reassociation_Ind.RSNIE, (void *)(p), Reassociation_Ind.RSNIELen);
    #else
    				Reassociation_Ind.RSNIELen = len;
    				memcpy((void *)Reassociation_Ind.RSNIE, (void *)(p + 2), len);
    #endif
    			}
    			memcpy((void *)Reassociation_Ind.OldAPaddr,
    				(void *)(pframe + WLAN_HDR_A3_LEN + _CAPABILITY_ + _LISTEN_INTERVAL_), MACADDRLEN);

    			// indicate if 11n sta associated
    			Reassociation_Ind.RSNIE[MAXRSNIELEN-1] = ((pstat->ht_cap_len==0) ? 0 : 1);

    			DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&Reassociation_Ind,
    						sizeof(DOT11_REASSOCIATION_IND));
    		}
    #endif // WITHOUT_ENQUEUE

    #ifdef INCLUDE_WPA_PSK
    		{
    			int id;
    			unsigned char *pIE;
    			int ie_len;

    			LOG_MSG("A wireless client is associated - %02X:%02X:%02X:%02X:%02X:%02X\n",
    				*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
    				*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));

    			if (frame_type == WIFI_ASSOCREQ)
    				id = DOT11_EVENT_ASSOCIATION_IND;
    			else
    				id = DOT11_EVENT_REASSOCIATION_IND;

    #ifdef RTL_WPA2
    			ie_len = len + 2;
    			pIE = p;
    #else
    			ie_len = len;
    			pIE = p + 2;
    #endif
    			psk_indicate_evt(priv, id, GetAddr2Ptr(pframe), pIE, ie_len);
    		}
    #endif // INCLUDE_WPA_PSK

    		event_indicate(priv, GetAddr2Ptr(pframe), 1);
    	}

    //#ifndef INCLUDE_WPA_PSK
    #ifdef CONFIG_RTL8186_TR
    	if (!IEEE8021X_FUN &&
    			!(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_ ||
    			 priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_))
    			LOG_MSG_NOTICE("Wireless PC connected;note:%02x-%02x-%02x-%02x-%02x-%02x;\n",
    				*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
    				*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
    #elif defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
    	if (!IEEE8021X_FUN &&
    			!(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_ ||
    			 priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_))
    			LOG_MSG_NOTICE("Wireless PC connected;note:%02x-%02x-%02x-%02x-%02x-%02x;\n",
    				*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
    				*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
    #else
    	LOG_MSG("A wireless client is associated - %02X:%02X:%02X:%02X:%02X:%02X\n",
    			*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
    			*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
    #endif
    //#endif

    	if (IEEE8021X_FUN || IAPP_ENABLE || priv->pmib->wscEntry.wsc_enable) {
    #ifndef __DRAYTEK_OS__
    		if (IEEE8021X_FUN &&	// in WPA, let user daemon check RSNIE and decide to accept or not
    			(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_ ||
    			 priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_))
    			return SUCCESS;
    #endif
    	}

    #ifdef WIFI_SIMPLE_CONFIG
    OnAssocReqSuccess:
    #endif
    */

    if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable)
    {

        // MESH ID check
        p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _MESH_ID_IE_ , &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);
        if (NULL == p)		// status shall be "_STATS_SUCCESSFUL_"
            goto OnAssocReq_MP_11frame;

        status = _STATS_OTHER_;	// for OnAssocReq_MP_Fail

        if ((len != strlen(GET_MIB(priv)->dot1180211sInfo.mesh_id))
                || memcmp(p + MESH_IE_BASE_LEN, GET_MIB(priv)->dot1180211sInfo.mesh_id, len))
            goto OnAssocReq_MP_Fail;

        // WLAN Mesh Capabity check
        p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _WLAN_MESH_CAP_IE_ , &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);
        memcpy(&cap, p + MESH_CAP_PEER_CAP_OFFSET, MESH_CAP_PEER_CAP_LEN);	// peer link capacity
        cap = le16_to_cpu(cap) & MESH_PEER_LINK_CAP_CAPACITY_MASK;

        if ((NULL == p) || (*(p + MESH_CAP_VERSION_OFFSET) != priv->mesh_Version)
                || (0 == cap) || (0 >= MESH_PEER_LINK_CAP_NUM(priv)))
            goto OnAssocReq_MP_Fail;

        //check MESH PathSelectProtocolID
        if(priv->mesh_profile[0].PathSelectProtocolID.value != *(p + MESH_CAP_PROTOCOL_OFFSET + 3))
        {
            goto OnAssocReq_MP_Fail;
        }


        if (WIFI_ASSOCREQ == frame_type)  	// Association frame ONLY
        {
            // Peer Link ID check
            p = get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _PEER_LINK_OPEN_IE_ , &len,
                       pfrinfo->pktlen - WLAN_HDR_A3_LEN - ie_offset);
            if ((NULL == p) || (MESH_LINK_ID_LEN != len))	// 1.Check is 11s frame ?, 2. Len error?(one Link ID)
                goto OnAssocReq_MP_Fail;

            memcpy(&recvLocalLinkID, p+MESH_IE_BASE_LEN, MESH_LINK_ID_LEN);
            recvLocalLinkID = le32_to_cpu(recvLocalLinkID);

        }

        // check received LocakLinkID match store? (Line 35~43, Page 114, D1.0),about configuration parameters process here
        if (pstat->mesh_neighbor_TBL.PeerLinkID != recvLocalLinkID)
        {
            if (0 == pstat->mesh_neighbor_TBL.PeerLinkID)		// pstat is first time recived open
                pstat->mesh_neighbor_TBL.PeerLinkID = recvLocalLinkID;	// Record correct ID
            else
            {
                // (out of scope of specification) Allow peer reconnect after peer brief lostconnect (e.g. signal strength unstable or web setting modify
                if ((pstat->mesh_neighbor_TBL.State == MP_SUPERORDINATE_LINK_UP) || (pstat->mesh_neighbor_TBL.State == MP_SUBORDINATE_LINK_UP)
                        || (pstat->mesh_neighbor_TBL.State == MP_SUPERORDINATE_LINK_DOWN) || (pstat->mesh_neighbor_TBL.State == MP_SUBORDINATE_LINK_DOWN_E))
                {
                    cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
                    mesh_cnt_ASSOC_PeerLink_CAP(priv, pstat, DECREASE);
                    free_stainfo(priv, pstat);
                    status = _RSON_UNSPECIFIED_;
                    goto asoc_MP_class2_error;	//issue deauth
                }

                deniedPeerLink = TRUE;
            }
        }

        MESH_DEBUG_MSG("Mesh: Reciver AssocREQ, MAC=%02X:%02X:%02X:%02X:%02X:%02X, State='%d', Denied='%d'\n Local: LocalLinkID=%lu, PeerLinkID=%lu\n  Peer: LocalLinkID=%lu\n"
                       , pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2], pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5], pstat->mesh_neighbor_TBL.State
                       , deniedPeerLink, pstat->mesh_neighbor_TBL.LocalLinkID, pstat->mesh_neighbor_TBL.PeerLinkID, recvLocalLinkID);

        if (0 >= MESH_PEER_LINK_CAP_NUM(priv))
            deniedPeerLink = TRUE;

        switch( pstat->mesh_neighbor_TBL.State)  	// ( D1.0, Page.116~120)
        {
            case MP_LISTEN:	// SOP, SCN or SCL, Timer was activity
                if (TRUE == deniedPeerLink)   // Using pstat record PeerLinkID emit close ( D1.0, Page.117, Line 30~32)
                {
                    issue_disassoc_MP(priv, pstat, _RSON_UNSPECIFIED_, INVALID_PARAMETERS);
                    pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_PEER_LINK_CLOSE_TO;
                    pstat->mesh_neighbor_TBL.State = MP_HOLDING;	// special csse
                    return SUCCESS;	// no response
                }

                pstat->mesh_neighbor_TBL.retry = 0;
                pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_PEER_LINK_RETRY_TO;
                issue_assocreq_MP(priv, pstat);
                break;
            case MP_OPEN_SENT:	// SCN or SCL, Timer was activity
                if (TRUE == deniedPeerLink)   // Using recived error LocalLink ID , To PeerLinkID emit close ( D1.0, Page.118, Line 4~6)
                {
                    pstat->mesh_neighbor_TBL.PeerLinkID = recvLocalLinkID;	// error ID
                    issue_disassoc_MP(priv, pstat, _RSON_UNSPECIFIED_, EXCEED_MAXIMUM_RETRIES);
                    pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_PEER_LINK_CLOSE_TO;
                    pstat->mesh_neighbor_TBL.State = MP_HOLDING;	// special csse
                    return SUCCESS;	// no response
                }

                pstat->mesh_neighbor_TBL.retry = 0;
                pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_PEER_LINK_RETRY_TO;
                break;
            case MP_CONFIRM_RCVD:	// SCN
                if (TRUE == deniedPeerLink)	// ignore ( D1.0, Page.118, Line 42~43)
                    return SUCCESS;	// no response

                establish = TRUE;
                mesh_PeerLinkEstablish(priv, pstat);
                break;
            case MP_SUBORDINATE_LINK_UP:	// SCN
            case MP_SUPERORDINATE_LINK_UP:
            case MP_SUBORDINATE_LINK_DOWN_E:
            case MP_SUPERORDINATE_LINK_DOWN:
            case MP_CONFIRM_SENT:	// Timer was activity
                if (TRUE == deniedPeerLink) // ignore ( D1.0, Page.119, Line 18~19, Page.120, Line 3~4)
                    return SUCCESS;	// no response

                break;
            case MP_HOLDING:
                issue_disassoc_MP(priv, pstat, _RSON_UNSPECIFIED_, CLOSE_RECEIVED);
                free_stainfo(priv, pstat); // Chris free stainfo to prevent being stuck in holding state
                return SUCCESS;	// no response
            default:	// Other abnormal state,delete it (include MP_UNUSED (Don't detect peer MP in Neighbor Discovery))
                free_stainfo(priv, pstat);
                return FAIL;	// no response
        }

        if (TRUE != establish)	// Status determine in mesh_PeerLinkEstablish, Don't determine here !!
            pstat->mesh_neighbor_TBL.State = mesh_PeerLinkStatesTable[pstat->mesh_neighbor_TBL.State][OpenReceived];	// change state

        MESH_DEBUG_MSG("Mesh: AssocREQ finish, Next State=%d\n", pstat->mesh_neighbor_TBL.State);
        status = _STATS_SUCCESSFUL_;	// All ok
    }

OnAssocReq_MP_11frame:
    if (frame_type == WIFI_ASSOCREQ)
        issue_assocrsp_MP(priv, status, pstat, WIFI_ASSOCRSP);
    else
        issue_assocrsp_MP(priv, status, pstat, WIFI_REASSOCRSP);

    return SUCCESS;

asoc_MP_class2_error:
    if ((NULL != pstat) && isSTA(pstat))	// Default is MP (PSTAT not exist) and is STA?
        is_11s = FALSE;

    issue_deauth_MP(priv, (void *)GetAddr2Ptr(pframe), status, is_11s);


    return FAIL;

OnAssocReq_MP_Fail:

    if (frame_type == WIFI_ASSOCREQ)
        issue_assocrsp_MP(priv, status, pstat, WIFI_ASSOCRSP);
    else
        issue_assocrsp_MP(priv, status, pstat, WIFI_REASSOCRSP);

    return FAIL;
}


/*
 *	@brief	Recived Assocation Response routine
 *
 *	@param	priv:priv
 *	@param	pfrinfo: recived frame
 *
 *	@retval	unsigned int: Result
 *
 *	PS: These is client code
 */
unsigned int OnAssocRsp_MP(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
    // closed	unsigned long	flags;
    struct wifi_mib	*pmib;
    struct stat_info *pstat;
    unsigned char	*pframe, *p;
    // closed	DOT11_ASSOCIATION_IND	Association_Ind;
    unsigned char	supportRate[32];
    int		supportRateNum;
    UINT16	val;
    int		len;
    UINT32	recvLocalLinkID, recvPeerLinkID;
    UINT8	establish = FALSE, deniedPeerLink = FALSE;
    UINT16	cap = 0; // peer capacity

    if (!(OPMODE & WIFI_STATION_STATE) && (0 == GET_MIB(priv)->dot1180211sInfo.mesh_enable))
        return SUCCESS;

    if (memcmp(GET_MY_HWADDR, pfrinfo->da, MACADDRLEN))	// dest address filter by hardware?
        return SUCCESS;

    if (OPMODE & WIFI_SITE_MONITOR)
        return SUCCESS;

    DEBUG_INFO("got assoc response\n");
    pmib = GET_MIB(priv);
    pframe = get_pframe(pfrinfo);

    // checking status
    val = cpu_to_le16(*(unsigned short*)((unsigned int)pframe + WLAN_HDR_A3_LEN + 2));

    if (val)
    {
        DEBUG_ERR("assoc reject, status: %d\n", val);
        goto assoc_mp_rejected;
    }

    priv->aid = cpu_to_le16(*(unsigned short*)((unsigned int)pframe + WLAN_HDR_A3_LEN + 4)) & 0x3fff;

    pstat = get_stainfo(priv, pfrinfo->sa);
    if (pstat == NULL)
    {
        return FAIL; // after a while, peer will expire this machine; and they will re-connect
    }
#if defined(CONFIG_RTL_88E_SUPPORT)

#if defined(TXREPORT)
    if (GET_CHIP_VER(priv)==VERSION_8188E)
    {
#ifdef RATEADAPTIVE_BY_ODM
        ODM_RAInfo_Init(ODMPTR, pstat->aid);
#else
        priv->pshare->RaInfo[pstat->aid].pstat = pstat;
        RateAdaptiveInfoInit(&priv->pshare->RaInfo[pstat->aid]);
#endif
    }
#endif

    if (GET_CHIP_VER(priv)==VERSION_8188E)
    {
        if (pstat && pstat->sta_in_firmware == 1)
        {
            RTL8188E_MACID_NOLINK(priv, 0, REMAP_AID(pstat));
            RTL8188E_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
            pstat->txpause_flag = 0;
        }
    }
#endif

#ifdef CONFIG_WLAN_HAL
    if(IS_HAL_CHIP(priv))
    {
        if(pstat && (REMAP_AID(pstat) < 128))
        {
            DEBUG_WARN("%s %d OnAssocRsp, set MACID 0 AID = %x \n",__FUNCTION__,__LINE__,REMAP_AID(pstat));
            GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0, REMAP_AID(pstat));
            pstat->txpause_flag = 0;
        }
        else
        {
            DEBUG_WARN(" MACID sleep only support 128 STA \n");
        }
    }
#endif

    // Realtek proprietary IE
    p = pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_;
    len = 0;
    for (;;)
    {
        p = get_ie(p, _RSN_IE_1_, &len,
                   pfrinfo->pktlen - (p - pframe));
        if (p != NULL)
        {
            if (!memcmp(p+2, Realtek_OUI, 3))
            {
                if (*(p+2+3) == 2)
                    pstat->is_realtek_sta = TRUE;
                else
                    pstat->is_realtek_sta = FALSE;
                break;
            }
        }
        else
            break;
        p = p + len + 2;
    }

    // get rates
    p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, _SUPPORTEDRATES_IE_, &len,
               pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);
    if (p == NULL)
    {
        free_stainfo(priv, pstat);
        return FAIL;
    }
    memcpy(supportRate, p+2, len);
    supportRateNum = len;
    p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, _EXT_SUPPORTEDRATES_IE_, &len,
               pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);
    if (p !=  NULL)
    {
        memcpy(supportRate+supportRateNum, p+2, len);
        supportRateNum += len;
    }

    // other capabilities
    memcpy(&val, (pframe + WLAN_HDR_A3_LEN), 2);
    val = le16_to_cpu(val);
    if (val & BIT(5))
    {
        // set preamble according to AP
        RTL_W8(RRSR+2, RTL_R8(RRSR+2) | BIT(7));
        pstat->useShortPreamble = 1;
    }
    else
    {
        // set preamble according to AP
        RTL_W8(RRSR+2, RTL_R8(RRSR+2) & ~BIT(7));
        pstat->useShortPreamble = 0;
    }

    if ((priv->pshare->curr_band == BAND_2G) && (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G))
    {
        if (val & BIT(10))
        {
            priv->pmib->dot11ErpInfo.shortSlot = 1;
            set_slot_time(priv, priv->pmib->dot11ErpInfo.shortSlot);
        }
        else
        {
            priv->pmib->dot11ErpInfo.shortSlot = 0;
            set_slot_time(priv, priv->pmib->dot11ErpInfo.shortSlot);
        }

        p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _ERPINFO_IE_, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);

        if (p && (*(p+2) & BIT(1)))	// use Protection
            priv->pmib->dot11ErpInfo.protection = 1;
        else
            priv->pmib->dot11ErpInfo.protection = 0;

        if (p && (*(p+2) & BIT(2)))	// use long preamble
            priv->pmib->dot11ErpInfo.longPreambleStaNum = 1;
        else
            priv->pmib->dot11ErpInfo.longPreambleStaNum = 0;
    }

    if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && priv->ht_cap_len)
    {
        p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, _HT_CAP_, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);
        if (p !=  NULL)
        {
            pstat->ht_cap_len = len;
            memcpy((unsigned char *)&pstat->ht_cap_buf, p+2, len);
        }
        else
        {
            pstat->ht_cap_len = 0;
            memset((unsigned char *)&pstat->ht_cap_buf, 0, sizeof(struct ht_cap_elmt));
        }

        p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, _HT_IE_, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);
        if (p !=  NULL)
        {
            pstat->ht_ie_len = len;
            memcpy((unsigned char *)&pstat->ht_ie_buf, p+2, len);
            //briansay("(briansay)got ht_ie from AssocRsp\n");
        }
        else
        {
            pstat->ht_ie_len = 0;
            //briansay("(briansay)ht_ie is not found in AssocRsp\n");
        }

        if (pstat->ht_cap_len)
        {
            if (cpu_to_le16(pstat->ht_cap_buf.ht_cap_info) & _HTCAP_AMSDU_LEN_8K_)
            {
                pstat->is_8k_amsdu = 1;
                pstat->amsdu_level = 7935 - sizeof(struct wlan_hdr);
            }
            else
            {
                pstat->is_8k_amsdu = 0;
                pstat->amsdu_level = 3839 - sizeof(struct wlan_hdr);
            }
        }
    }


//8812_client add pstat vht ie
#ifdef RTK_AC_SUPPORT
    if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)
    {
        p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, EID_VHTCapability, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);

        if ((p !=  NULL) && (len <= sizeof(struct vht_cap_elmt)))
        {
            pstat->vht_cap_len = len;
            memcpy((unsigned char *)&pstat->vht_cap_buf, p+2, len);
            MESH_DEBUG_MSG("receive vht_cap len = %d \n", len);
        }

        p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, EID_VHTOperation, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);

        if ((p !=  NULL) && (len <= sizeof(struct vht_oper_elmt)))
        {
            pstat->vht_oper_len = len;
            memcpy((unsigned char *)&pstat->vht_oper_buf, p+2, len);
            MESH_DEBUG_MSG("receive vht_oper len = %d \n", len);
        }
    }
#endif

    get_matched_rate(priv, supportRate, &supportRateNum, 1);
    update_support_rate(pstat, supportRate, supportRateNum);
    assign_tx_rate(priv, pstat, pfrinfo);
    assign_aggre_mthod(priv, pstat);
    assign_aggre_size(priv, pstat);
    /*
    	SAVE_INT_AND_CLI(flags);

    	pstat->expire_to = priv->expire_to;
    	list_add_tail(&pstat->asoc_list, &priv->asoc_list);
    	cnt_assoc_num(priv, pstat, INCREASE, (char *)__FUNCTION__);

    	if (!IEEE8021X_FUN &&
    			!(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_ ||
    			 priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_)) {
    #if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD)
    		LOG_MSG_NOTICE("Connected to AP;note:%02x-%02x-%02x-%02x-%02x-%02x;\n",
    				*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
    				*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
    #else
    	LOG_MSG("Associate to AP successfully - %02X:%02X:%02X:%02X:%02X:%02X\n",
    		*GetAddr2Ptr(pframe), *(GetAddr2Ptr(pframe)+1), *(GetAddr2Ptr(pframe)+2),
    		*(GetAddr2Ptr(pframe+3)), *(GetAddr2Ptr(pframe)+4), *(GetAddr2Ptr(pframe)+5));
    #endif
    	}

    	// now we have successfully join the give bss...
    	if (timer_pending(&priv->reauth_timer))
    		del_timer_sync(&priv->reauth_timer);
    	if (timer_pending(&priv->reassoc_timer))
    		del_timer_sync(&priv->reassoc_timer);

    	RESTORE_INT(flags);

    	OPMODE |= WIFI_ASOC_STATE;
    	update_bss(&priv->pmib->dot11StationConfigEntry, &priv->pmib->dot11Bss);
    	priv->pmib->dot11RFEntry.dot11channel = priv->pmib->dot11Bss.channel;
    	join_bss(priv);
    	priv->join_res = STATE_Sta_Bss;
    	priv->join_req_ongoing = 0;

    #ifndef WITHOUT_ENQUEUE
    	if (priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm
    #ifdef WIFI_SIMPLE_CONFIG
    		&& !(priv->pmib->wscEntry.wsc_enable)
    #endif
    		)
    	{
    		memcpy((void *)Association_Ind.MACAddr, (void *)GetAddr2Ptr(pframe), MACADDRLEN);
    		Association_Ind.EventId = DOT11_EVENT_ASSOCIATION_IND;
    		Association_Ind.IsMoreEvent = 0;
    		Association_Ind.RSNIELen = 0;
    		DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&Association_Ind,
    					sizeof(DOT11_ASSOCIATION_IND));
    		event_indicate(priv, GetAddr2Ptr(pframe), 1);
    	}
    #endif // WITHOUT_ENQUEUE

    #ifdef WIFI_SIMPLE_CONFIG
    	if (priv->pmib->wscEntry.wsc_enable) {
    		DOT11_WSC_ASSOC_IND wsc_Association_Ind;

    		memset(&wsc_Association_Ind, 0, sizeof(DOT11_WSC_ASSOC_IND));
    		wsc_Association_Ind.EventId = DOT11_EVENT_WSC_ASSOC_REQ_IE_IND;
    		memcpy((void *)wsc_Association_Ind.MACAddr, (void *)GetAddr2Ptr(pframe), MACADDRLEN);
    		DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&wsc_Association_Ind,
    			sizeof(DOT11_WSC_ASSOC_IND));
    		event_indicate(priv, GetAddr2Ptr(pframe), 1);
    		pstat->state |= WIFI_WPS_JOIN;
    	}
    #endif

    #if 0
    	// Get operating bands
    	//    |  B |  G | BG  <= AP
    	//  B |  B |  x |  B
    	//  G |  x |  G |  G
    	// BG |  B |  G | BG
    	if ((priv->pshare->curr_band == WIRELESS_11A) ||
    		(priv->pshare->curr_band == WIRELESS_11B))
    		priv->oper_band = priv->pshare->curr_band;
    	else {			// curr_band == WIRELESS_11G
    		if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11B) ||
    			!is_CCK_rate(pstat->bssrateset[0] & 0x7f))
    			priv->oper_band = WIRELESS_11G;
    		else if (is_CCK_rate(pstat->bssrateset[pstat->bssratelen-1] & 0x7f))
    			priv->oper_band = WIRELESS_11B;
    		else
    			priv->oper_band = WIRELESS_11B | WIRELESS_11G;
    	}
    #endif

    	DEBUG_INFO("assoc successful!\n");

    #ifdef UNIVERSAL_REPEATER
    	if (IS_ROOT_INTERFACE(priv))
    #endif
    	{
    		if ((pstat->ht_cap_len > 0) && (pstat->ht_ie_len > 0) &&
    				(pstat->ht_ie_buf.info0 & _HTIE_STA_CH_WDTH_) &&
    		(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))) {
    			priv->pshare->is_40m_bw = 1;
    			if ((pstat->ht_ie_buf.info0 & _HTIE_2NDCH_OFFSET_BL_) == _HTIE_2NDCH_OFFSET_BL_)
    				priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
    			else
    				priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;

    			priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
    			SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
    			SwChnl(priv, priv->pmib->dot11Bss.channel, priv->pshare->offset_2nd_chan);

    			DEBUG_INFO("%s: set chan=%d, 40M=%d, offset_2nd_chan=%d\n",
    				__FUNCTION__,
    				priv->pmib->dot11Bss.channel,
    				priv->pshare->is_40m_bw,  priv->pshare->offset_2nd_chan);

    		}
    		else {
    			priv->pshare->is_40m_bw = 0;
    			priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;
    		}
    	}
    */

    if (pstat->ht_cap_len)
    {
        if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))
            pstat->tx_bw = HT_CHANNEL_WIDTH_20_40;
        else
            pstat->tx_bw = HT_CHANNEL_WIDTH_20;
    }

// MESH section START
    if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable)
    {
        // MESH ID check
        p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, _MESH_ID_IE_, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);
        if ((NULL == p) || (len != strlen(GET_MIB(priv)->dot1180211sInfo.mesh_id))
                || memcmp(p + MESH_IE_BASE_LEN, GET_MIB(priv)->dot1180211sInfo.mesh_id, len))
            return SUCCESS;

        // WLAN Mesh Capabity check
        p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, _WLAN_MESH_CAP_IE_, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);
        if (NULL == p)
            return FAIL;

        memcpy(&cap, p + MESH_CAP_PEER_CAP_OFFSET, MESH_CAP_PEER_CAP_LEN);	// peer link capacity
        cap = le16_to_cpu(cap) & MESH_PEER_LINK_CAP_CAPACITY_MASK;
        if ((0 == cap) || (*(p + MESH_CAP_VERSION_OFFSET) != priv->mesh_Version))
            return FAIL;

        //check MESH PathSelectProtocolID
        if(priv->mesh_profile[0].PathSelectProtocolID.value != *(p + MESH_CAP_PROTOCOL_OFFSET + 3))
        {
            return FAIL;
        }


        if (WIFI_ASSOCRSP == GetFrameSubType(pframe))  	// Association frame ONLY
        {
            // Peer Link ID check
            p = get_ie(pframe + WLAN_HDR_A3_LEN + _ASOCRSP_IE_OFFSET_, _PEER_LINK_CONFIRM_IE_, &len,
                       pfrinfo->pktlen - WLAN_HDR_A3_LEN - _ASOCRSP_IE_OFFSET_);
            if ((NULL == p) || ((MESH_LINK_ID_LEN*2) != len))	// 1.Check is 11s frame ?, 2. Len error?(twice Link ID)
                return FAIL;

            memcpy(&recvLocalLinkID, p+MESH_IE_BASE_LEN, MESH_LINK_ID_LEN);
            recvLocalLinkID = le32_to_cpu(recvLocalLinkID);

            memcpy(&recvPeerLinkID, p+MESH_IE_BASE_LEN+MESH_LINK_ID_LEN, MESH_LINK_ID_LEN);
            recvPeerLinkID = le32_to_cpu(recvPeerLinkID);

        }

        // check received LocakLinkID match store value? (D1.0, Page 114, Line 35~43, Page 115, Line 1~4),about configuration parameters process here
        if (((pstat->mesh_neighbor_TBL.PeerLinkID != recvLocalLinkID) && (0 != pstat->mesh_neighbor_TBL.PeerLinkID)) 	// Allow haven't PeerLinkID record in local machine.
                || (pstat->mesh_neighbor_TBL.LocalLinkID != recvPeerLinkID) || (0 >= MESH_PEER_LINK_CAP_NUM(priv)))
            deniedPeerLink = TRUE;

        MESH_DEBUG_MSG("Mesh: Reciver AssocRSP, MAC=%02X:%02X:%02X:%02X:%02X:%02X, State='%d', Denied='%d'\n      Local: LocalLinkID=%lu,   PeerLinkID=%lu\n      Peer: LocalLinkID=%lu,   PeerLinkID=%lu\n"
                       , pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2], pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5], pstat->mesh_neighbor_TBL.State
                       , deniedPeerLink, pstat->mesh_neighbor_TBL.LocalLinkID, pstat->mesh_neighbor_TBL.PeerLinkID, recvLocalLinkID, recvPeerLinkID);

        switch(pstat->mesh_neighbor_TBL.State)  	// ( D1.0, Page.116~120)
        {
            case MP_LISTEN:	// ( D1.0, Page.117, Line 23~26), Timer was activity
                /* Spare for future.
                	if ((pstat->mesh_neighbor_TBL.LocalLinkID == recvLocalLinkID) && (TRUE != deniedPeerLink)) // (D1.0, Page 117, Line 23~26)
                		// raise an exception to the higher layer to indicate the local random rumber generator is "broken"
                */
                break;
            case MP_OPEN_SENT:	// - or SCL, Timer was activity
                if (TRUE == deniedPeerLink) // Using recived incorrect LocalLink ID, To PeerLinkID emit close ( D1.0, Page.118, Line 11~14)
                    goto	OnAssocRsp_MP_SCL;

                pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_PEER_LINK_OPEN_TO;	// because change to CONFIRM_RCVD
                break;
            case MP_CONFIRM_SENT:	// - or SCL, Timer was activity
                if (TRUE == deniedPeerLink)	// sing recived incorrect LocalLink ID, To PeerLinkID emit close (D1.0, Page.118, Line 11~14)
                    goto	OnAssocRsp_MP_SCL;

                establish = TRUE;
                mesh_PeerLinkEstablish(priv, pstat);
                break;
            case MP_SUBORDINATE_LINK_UP:
            case MP_SUPERORDINATE_LINK_UP:
            case MP_SUBORDINATE_LINK_DOWN_E:
            case MP_SUPERORDINATE_LINK_DOWN:
            case MP_CONFIRM_RCVD:	// Timer was activity
            case MP_UNUSED:			// Clear MP data by Timer.
                break;
            case MP_HOLDING:
                issue_disassoc_MP(priv, pstat, _RSON_UNSPECIFIED_, CLOSE_RECEIVED);
                free_stainfo(priv, pstat); // Chris free stainfo to prevent being stuck in holding state
                return SUCCESS;
            //break;
            default:	// Other abnormal state,delete it
                free_stainfo(priv, pstat);
                return FAIL;
        }

        if (TRUE != establish)	// Status determine in mesh_PeerLinkEstablish, Don't determine here !!
            pstat->mesh_neighbor_TBL.State = mesh_PeerLinkStatesTable[pstat->mesh_neighbor_TBL.State][ConfirmReceived];	// change state

        MESH_DEBUG_MSG("Mesh: AssocRSP finish, Next State=%d\n", pstat->mesh_neighbor_TBL.State);
    }


//8812_client , check ap support 80m ??
#ifdef RTK_AC_SUPPORT
    if((priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC))
    {
        if (pstat->vht_cap_len && (pstat->vht_oper_buf.vht_oper_info[0] == 1))
        {
            pstat->tx_bw = HT_CHANNEL_WIDTH_80;
            priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_80;
        }
    }
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
    if(GET_CHIP_VER(priv)== VERSION_8812E)
    {
        UpdateHalRAMask8812(priv, pstat, 3);
    }
#endif

#ifdef  CONFIG_WLAN_HAL
    if (IS_HAL_CHIP(priv))
    {
        GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
    }
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
    if (GET_CHIP_VER(priv)==VERSION_8188E)
    {
#ifdef TXREPORT
        add_RATid(priv, pstat);
#endif
    }
    else
#endif
    {
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
        add_update_RATid(priv, pstat);
#endif
    }

#ifdef CONFIG_WLAN_HAL_8192EE
    if (GET_CHIP_VER(priv) == VERSION_8192E)
    {
        if (OPMODE & WIFI_STATION_STATE)
        {
            if ((priv->pshare->CurrentChannelBW==0 && priv->pmib->dot11RFEntry.dot11channel==13)
                    ||(priv->pshare->CurrentChannelBW==1 && priv->pmib->dot11RFEntry.dot11channel>=11))
                Check_92E_Spur_Valid(priv, false);
        }
    }
#endif

    return SUCCESS;

OnAssocRsp_MP_SCL:	// Timer was activity

    pstat->mesh_neighbor_TBL.PeerLinkID = recvLocalLinkID;	// For SCL (send close)
    pstat->mesh_neighbor_TBL.LocalLinkID = recvPeerLinkID;	// For SCL (send close)
    issue_disassoc_MP(priv, pstat, _RSON_UNSPECIFIED_, INVALID_PARAMETERS);
    pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_PEER_LINK_CLOSE_TO;
    pstat->mesh_neighbor_TBL.State = MP_HOLDING;	// special csse

    return SUCCESS;

assoc_mp_rejected:
    return FAIL;
}




/*
 *	@brief	Recived Probe Response routine
 *
 *	@param	priv:priv
 *	@param	pfrinfo: Recived frame
 *
 *	@retval	unsigned int: Result
 *
 *	CAUTION! These code doesn't from OnProbeRsp !! ,Write myself !!
 */
unsigned int OnProbeReq_MP(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
    struct wifi_mib	*pmib;
    unsigned char	*pframe, *p, MeshIDLen;
    unsigned int	len;
    struct stat_info *pstat;

    pmib   = GET_MIB(priv);
    pframe = get_pframe(pfrinfo);

    // check Main system & profile are Enable (mesh_profile Configure by WEB in the future, Maybe delete, Preservation before delete)
    if(!((1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) && (TRUE == priv->mesh_profile[0].used)))
        goto OnProbeReqFail_MP;

    // MeshID
    p = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_, _MESH_ID_IE_, (int *)&len,
               pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_);	//Mesh ID

    MeshIDLen = strlen(GET_MIB(priv)->dot1180211sInfo.mesh_id);

    // len = 0 : wildcard MESH ID, uncondition allow.(D1.0, Page.24, Line 7~8)
    if((NULL == p) || ((0 != len) && ((len != MeshIDLen) || memcmp(p + MESH_IE_BASE_LEN, GET_MIB(priv)->dot1180211sInfo.mesh_id, len))))
        goto OnProbeReqFail_MP;

    // WLAN Mesh Capability
    p = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_, _WLAN_MESH_CAP_IE_, &len,
               pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_);
    if ((NULL == p) || (*(p + MESH_CAP_VERSION_OFFSET) != priv->mesh_Version))
        goto OnProbeReqFail_MP;

    //check MESH PathSelectProtocolID
    if(priv->mesh_profile[0].PathSelectProtocolID.value != *(p + MESH_CAP_PROTOCOL_OFFSET + 3))
    {
        goto OnProbeReqFail_MP;
    }

#ifdef RTL8190
    if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && priv->pshare->is_40m_bw)
    {
        p = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_, _HT_CAP_, (int *)&len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_);
        if (p !=  NULL)
        {
            struct ht_cap_elmt *ht_cap_buf;
            ht_cap_buf = (struct ht_cap_elmt *)(p + 2);
            if ((ht_cap_buf->ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_)) == 0)
            {
                memcpy(priv->mac_4965, GetAddr2Ptr(pframe), MACADDRLEN);
            }
        }
    }
#endif
#ifndef MESH_BOOTSEQ_STRESS_TEST	// Stop update expire when stress test.
    p = GetAddr2Ptr(pframe);
    pstat = get_stainfo(priv, p);	// query neighbor table entry
    if (pstat != NULL)
    {
        pstat->mesh_neighbor_TBL.expire = jiffies + MESH_EXPIRE_TO;	// fix: 0000087
    }
#endif


    mesh_sme_debug("******** SME OnProbeReq_MP: %s\r\n", priv->dev->name);


// ==== modified by GANTOE for wrong parameters of issue_probersp_MP 2008/12/25 ====
    //issue_probersp_MP(priv, GetAddr2Ptr(pframe), SSID, SSID_LEN, 1, TRUE);
    if ( !pmib->dot1180211sInfo.mesh_ap_enable && pmib->dot1180211sInfo.mesh_enable )
        issue_probersp_MP(priv, GetAddr2Ptr(pframe), NULL, 0, 1, TRUE, 0);
    else
        issue_probersp_MP(priv, GetAddr2Ptr(pframe), SSID, SSID_LEN, 1, TRUE, 0);
// ==== GANTOE ====
    //issue_probersp_MP(priv, GetAddr2Ptr(pframe), GET_MIB(priv)->dot1180211sInfo.mesh_id, MeshIDLen, 1, TRUE);

    return SUCCESS;


OnProbeReqFail_MP:
    return FAIL;
}



/*
 *	@brief	Recived Probe Response routine
 *
 *	@param	priv:priv
 *	@param	pfrinfo: Recived frame
 *
 *	@retval	unsigned int: Result
 *
 *	CAUTION! These code doesn't modify from OnProbeRsp !! ,Write myself !!
 */
unsigned int OnProbeRsp_MP(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
    struct stat_info *pstat;
    int len;
    unsigned char *pframe, *p;
    UINT16	cap;	// peer capacity
    unsigned char	*sa;
    int start_swchnl_proc;

#ifdef SIMPLE_CH_UNI_PROTOCOL
    UINT32  precedence=0;
#endif

    mesh_sme_debug("**********SME OnProbeRsp_MP: %s\r\n", priv->dev->name);



    if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable)
    {
        if(FALSE == priv->mesh_profile[0].used) // Configure by WEB in the future, Maybe delete, Preservation before delete
            return SUCCESS;

        pframe = get_pframe(pfrinfo);

        //Mesh ID
        p = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBERSP_IE_OFFSET_, _MESH_ID_IE_, (int *)&len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBERSP_IE_OFFSET_);
        if(NULL == p ||
                (!timer_pending(&priv->ss_timer) &&
                 (len != strlen(GET_MIB(priv)->dot1180211sInfo.mesh_id) ||
                  memcmp((const void*)(p + MESH_IE_BASE_LEN), (const void*)(GET_MIB(priv)->dot1180211sInfo.mesh_id), len)
                 )
                )
          )
            return FAIL;

        // WLAN Mesh Capability
        p = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBERSP_IE_OFFSET_, _WLAN_MESH_CAP_IE_, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _PROBERSP_IE_OFFSET_);
        if ((NULL == p) || (*(p + MESH_CAP_VERSION_OFFSET) != priv->mesh_Version))
            return FAIL;

        //check MESH PathSelectProtocolID
        if(priv->mesh_profile[0].PathSelectProtocolID.value != *(p + MESH_CAP_PROTOCOL_OFFSET + 3))
        {
            return FAIL;
        }

        #ifdef SIMPLE_CH_UNI_PROTOCOL
        precedence = le32_to_cpu(*((UINT32*)(p+MESH_CAP_CH_PRECEDENCE_OFFSET)));
        #endif

        #if defined(DFS)     
        if(mesh_DFS_check(priv, pfrinfo, precedence))            
            return SUCCESS;
        #endif
        

        memcpy(&cap, p + MESH_CAP_PEER_CAP_OFFSET, MESH_CAP_PEER_CAP_LEN);
        cap = le16_to_cpu(cap) & MESH_PEER_LINK_CAP_CAPACITY_MASK;

        sa = GetAddr2Ptr(pframe);
        pstat = get_stainfo(priv, sa);	// query neighbor table entry

        if (pstat != NULL)
        {
            UINT32	tmp32;
            // update channel precedence value
            memcpy(&tmp32, p + MESH_CAP_CH_PRECEDENCE_OFFSET, sizeof(tmp32));
            pstat->mesh_neighbor_TBL.Pl = le32_to_cpu(tmp32);

            // update OFDM_parameter (current channel)
            pstat->mesh_neighbor_TBL.Co = priv->pmib->dot11RFEntry.dot11channel;

#ifndef MESH_BOOTSEQ_STRESS_TEST	// Stop update expire when stress test.
            pstat->mesh_neighbor_TBL.expire = jiffies + MESH_EXPIRE_TO;	// fix: 0000087
#endif
        }

        start_swchnl_proc = mesh_channel_switch_check(priv, pfrinfo);

        mesh_sme_debug("New MP: %02X:%02X:%02X:%02X:%02X:%02X detected then trigger Peer-Link\n",
                       pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2], pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5]);

#ifdef SIMPLE_CH_UNI_PROTOCOL
        if( precedence && priv->auto_channel)
        {
            mesh_auto_channel_check(priv, pfrinfo, precedence, start_swchnl_proc);
        }
        if(!timer_pending(&priv->ss_timer))
#endif
            start_MeshPeerLink(priv, pfrinfo, pstat, cap);
    }

    return SUCCESS;
}


unsigned int OnBeacon_MP(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
    int len;
    unsigned char *p, *pframe;
    struct stat_info *pstat;
    UINT16	cap; // peer capacity
#ifdef SIMPLE_CH_UNI_PROTOCOL
    UINT32	precedence=0;
#endif
    int start_swchnl_proc;

    if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable)
    {
        pframe = get_pframe(pfrinfo);

        // MeshID
        p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _MESH_ID_IE_, (int *)&len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
        if((NULL == p) || (len != strlen(GET_MIB(priv)->dot1180211sInfo.mesh_id)) || memcmp(p + MESH_IE_BASE_LEN, GET_MIB(priv)->dot1180211sInfo.mesh_id, len))
            return FAIL;

        // WLAN Mesh Capability
        p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _WLAN_MESH_CAP_IE_, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
        if ((NULL == p) || (*(p + MESH_CAP_VERSION_OFFSET) != priv->mesh_Version))
        {
            printk("%s %d\n",__FUNCTION__,__LINE__);
            return FAIL;
        }

        //check MESH PathSelectProtocolID
        if(priv->mesh_profile[0].PathSelectProtocolID.value != *(p + MESH_CAP_PROTOCOL_OFFSET + 3))
        {
            printk("%s %d\n",__FUNCTION__,__LINE__);
            return FAIL;
        }


#ifdef SIMPLE_CH_UNI_PROTOCOL
        precedence = le32_to_cpu(*((UINT32*)(p+MESH_CAP_CH_PRECEDENCE_OFFSET)));
#endif

#if defined(DFS)
        if(mesh_DFS_check(priv, pfrinfo, precedence))            
            return SUCCESS;
#endif 

        memcpy(&cap, p + MESH_CAP_PEER_CAP_OFFSET, MESH_CAP_PEER_CAP_LEN);
        cap = le16_to_cpu(cap) & MESH_PEER_LINK_CAP_CAPACITY_MASK;
        pstat = get_stainfo(priv, pfrinfo->sa); // query neighbor table entry

        start_swchnl_proc = mesh_channel_switch_check(priv, pfrinfo);

        if (NULL != pstat)
        {
#ifndef MESH_BOOTSEQ_STRESS_TEST	// Stop update expire when stress test.
            pstat->mesh_neighbor_TBL.expire = jiffies + MESH_EXPIRE_TO;	// fix: 0000087
#endif
            // hookBeacon_11s(priv, pstat);
            // send Local Link State Announcement

            if (time_after(jiffies, pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod)
                    && (WIFI_ASOC_STATE & pstat->state)
                    && (MP_SUPERORDINATE_LINK_UP == pstat->mesh_neighbor_TBL.State || MP_SUPERORDINATE_LINK_DOWN == pstat->mesh_neighbor_TBL.State))
            {
                MESH_DEBUG_MSG("Mesh: OnBeacon, Issue LocalLinkStateAnnouncement\n");
                issue_LocalLinkStateANNOU_MP(priv, pfrinfo, pstat);

                // Update oneself peer pstat.
                //pstat->mesh_neighbor_TBL.r = getDataRate(pstat);	// fix : 0000083
                pstat->mesh_neighbor_TBL.r = getDataRateOffset(pstat);
                // Use rssi temporary
                pstat->mesh_neighbor_TBL.ept = (UINT16)pfrinfo->rssi;
                pstat->mesh_neighbor_TBL.Q= pfrinfo->rssi;
                pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_LocalLinkStateANNOU_TO;
                pstat->mesh_neighbor_TBL.State = MP_SUPERORDINATE_LINK_UP;
                updateMeshMetric(priv, pstat, pfrinfo);
#ifdef SIMPLE_CH_UNI_PROTOCOL
                pstat->mesh_neighbor_TBL.Pl = precedence;
#endif
            }
        }
#ifdef SIMPLE_CH_UNI_PROTOCOL
        if( precedence && priv->auto_channel)
        {
            mesh_auto_channel_check(priv, pfrinfo, precedence, start_swchnl_proc);
        }
        if(!timer_pending(&priv->ss_timer))
#endif
        {
            start_MeshPeerLink(priv, pfrinfo, pstat, cap);
        }
    }

    return SUCCESS;
}



unsigned int OnDisassoc_MP(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
    unsigned char *pframe;
    struct  stat_info   *pstat;
    unsigned char *sa;
    unsigned short reason;
    
#ifndef SMP_SYNC    
    unsigned long flags;
#endif

    unsigned char *p;
    int	len;
    UINT32	recvLocalLinkID, recvPeerLinkID;
    UINT8	deniedPeerLink = FALSE;
    struct	MESH_Neighbor_Entry tmpEntry;	// for backup

    mesh_sme_debug("**********SME OnDisassoc_MP: %s\r\n", priv->dev->name);

    pframe = get_pframe(pfrinfo);
    sa = GetAddr2Ptr(pframe);
    pstat = get_stainfo(priv, sa);

    if (pstat == NULL)
        return 0;

#ifdef RTK_WOW
    if (pstat->is_rtk_wow_sta)
        return 0;
#endif

#ifdef CONFIG_RTK_MESH		// release_stainfo and init_stainfo will clean data.
    if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable)
        memcpy(&tmpEntry, &(pstat->mesh_neighbor_TBL), sizeof(struct MESH_Neighbor_Entry));
#endif

    reason = cpu_to_le16(*(unsigned short *)((unsigned int)pframe + WLAN_HDR_A3_LEN ));
    DEBUG_INFO("receiving disassoc from station %02X%02X%02X%02X%02X%02X reason %d\n",
               pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2],
               pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5], reason);

    SAVE_INT_AND_CLI(flags);

    if (!list_empty(&pstat->asoc_list))
    {
        list_del_init(&pstat->asoc_list);
        if (pstat->expire_to > 0)
        {
            cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
            check_sta_characteristic(priv, pstat, DECREASE);
        }
#ifdef CONFIG_RTL_88E_SUPPORT
        if (GET_CHIP_VER(priv)==VERSION_8188E && pstat->sta_in_firmware == 1)
        {
            RTL8188E_MACID_NOLINK(priv, 1, REMAP_AID(pstat));
            RTL8188E_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
            pstat->txpause_flag = 0;
        }
#endif

#ifdef CONFIG_WLAN_HAL
        if(IS_HAL_CHIP(priv))
        {
            if(pstat && (REMAP_AID(pstat) < 128))
            {
                DEBUG_WARN("%s %d OnDisassoc, set MACID 0 AID = %x \n",__FUNCTION__,__LINE__,REMAP_AID(pstat));
                GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0 , REMAP_AID(pstat));
                pstat->txpause_flag = 0;
            }
            else
            {
                DEBUG_WARN(" MACID sleep only support 128 STA \n");
            }
        }
#endif
    }

    // Need change state back to autehnticated
    release_stainfo(priv, pstat);
    init_stainfo(priv, pstat);
    pstat->state |= WIFI_AUTH_SUCCESS;
    pstat->expire_to = priv->assoc_to;
    list_add_tail(&(pstat->auth_list), &(priv->auth_list));

    RESTORE_INT(flags);

    LOG_MESH_MSG("A wireless client is disassociated - %02X:%02X:%02X:%02X:%02X:%02X\n",
                 *sa, *(sa+1), *(sa+2), *(sa+3), *(sa+4), *(sa+5));

    if (IEEE8021X_FUN)
    {
        /* closed
        #ifndef WITHOUT_ENQUEUE
        	memcpy((void *)Disassociation_Ind.MACAddr, (void *)sa, MACADDRLEN);
        	Disassociation_Ind.EventId = DOT11_EVENT_DISASSOCIATION_IND;
        	Disassociation_Ind.IsMoreEvent = 0;
        	Disassociation_Ind.Reason = reason;
        	Disassociation_Ind.tx_packets = pstat->tx_pkts;
        	Disassociation_Ind.rx_packets = pstat->rx_pkts;
        	Disassociation_Ind.tx_bytes   = pstat->tx_bytes;
        	Disassociation_Ind.rx_bytes   = pstat->rx_bytes;
        	DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&Disassociation_Ind,
        				sizeof(DOT11_DISASSOCIATION_IND));
        #endif
        */
#ifdef INCLUDE_WPA_PSK
        psk_indicate_evt(priv, DOT11_EVENT_DISASSOCIATION_IND, sa, NULL, 0);
#endif
    }

    event_indicate(priv, sa, 2);

#ifdef CONFIG_RTK_MESH	// MESH section START
    if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable)
    {
        p = get_ie(pframe + WLAN_HDR_A3_LEN + _DISASS_IE_OFFSET_, _PEER_LINK_CLOSE_IE_, &len,
                   pfrinfo->pktlen - WLAN_HDR_A3_LEN - _DISASS_IE_OFFSET_);
        if ((NULL == p) || (1+(MESH_LINK_ID_LEN*2) != len)) 	// 1.Check is 11s frame?, 2. Len error? (1=reason, Add two Link ID)
            return SUCCESS;

        memcpy(&(pstat->mesh_neighbor_TBL), &tmpEntry, sizeof(struct MESH_Neighbor_Entry)); // Restore release_stainfo & init_stainfo cleaned data.	(CAUTION!! All list are remove, Shall be set list manual !)
        memcpy(&recvLocalLinkID, p+MESH_IE_BASE_LEN+1, MESH_LINK_ID_LEN);	// 1=size of Reason Code
        recvLocalLinkID = le32_to_cpu(recvLocalLinkID);

        memcpy(&recvPeerLinkID, p+MESH_IE_BASE_LEN+1+MESH_LINK_ID_LEN, MESH_LINK_ID_LEN);	//	1=size of  Reason Code
        recvPeerLinkID = le32_to_cpu(recvPeerLinkID);

        // check received LocakLinkID match store value? (D1.0, Page 114, Line 28~34), Process configuration parameters here
        if (((pstat->mesh_neighbor_TBL.LocalLinkID != recvPeerLinkID) && (0 != recvPeerLinkID)) 	// Allow peer MP have no PeerLinkID
                || (pstat->mesh_neighbor_TBL.PeerLinkID != recvLocalLinkID))  		// Denied peer MP disassoc request
        {
            SAVE_INT_AND_CLI(flags);
            list_del_init(&pstat->auth_list);
            pstat->state |= WIFI_ASOC_STATE;	// Avoid break data transmit disassoc return.
            pstat->expire_to = priv->expire_to;
            list_add_tail(&pstat->asoc_list, &priv->asoc_list);
            cnt_assoc_num(priv, pstat, INCREASE, (char *)__FUNCTION__);
            RESTORE_INT(flags);

            deniedPeerLink = TRUE;
        }
        MESH_DEBUG_MSG("\nMesh: Reciver DisAssoc, MAC=%02X:%02X:%02X:%02X:%02X:%02X State=%d, Reason=%d\n"
                       , pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2], pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5], pstat->mesh_neighbor_TBL.State, *(p+MESH_IE_BASE_LEN));
        MESH_DEBUG_MSG("   deniedPeerLink='%d'  and	below 4 LinkID \n Local: LocalLinkID=%lu, PeerLinkID=%lu	 Peer: LocalLinkID=%lu, PeerLinkID=%lu\n"
                       , deniedPeerLink, pstat->mesh_neighbor_TBL.LocalLinkID, pstat->mesh_neighbor_TBL.PeerLinkID, recvLocalLinkID, recvPeerLinkID);

        switch(pstat->mesh_neighbor_TBL.State)  	// ( D1.0, Page.116~120)
        {
            case MP_LISTEN: // ( D1.0, Page.117, Line 23~26), Timer was activity
                /* Reserve for future use.
                if ((pstat->mesh_neighbor_TBL.LocalLinkID == recvLocalLinkID) && (TRUE != deniedPeerLink)) // (D1.0, Page 117, Line 23~26)
                	// raise an exception to the higher layer to indicate the local random rumber generator is "broken"
                */
                SAVE_INT_AND_CLI(flags);
                if (list_empty(&(pstat->mesh_mp_ptr)))
                    list_add_tail(&(pstat->mesh_mp_ptr), &(priv->mesh_unEstablish_hdr));

                RESTORE_INT(flags);

                return SUCCESS;
            case MP_SUBORDINATE_LINK_UP: // 1sec Timer was activity
            case MP_SUPERORDINATE_LINK_UP:
            case MP_SUBORDINATE_LINK_DOWN_E:
            case MP_SUPERORDINATE_LINK_DOWN:
                if (TRUE == deniedPeerLink)
                {
                    SAVE_INT_AND_CLI(flags);
                    if (list_empty(&(pstat->mesh_mp_ptr)))
                        //list_add_tail(&(pstat->mesh_mp_ptr), &(priv->mesh_mp_hdr));
                        list_add_tail(&(pstat->mesh_mp_ptr), &(priv->mesh_unEstablish_hdr));

                    RESTORE_INT(flags);
                    return SUCCESS;
                }

                mesh_cnt_ASSOC_PeerLink_CAP(priv, pstat, DECREASE);
                break;
            case MP_OPEN_SENT:	// Timer was activity
            case MP_CONFIRM_RCVD:
            case MP_CONFIRM_SENT:
                if (TRUE == deniedPeerLink)
                {
                    SAVE_INT_AND_CLI(flags);
                    if (list_empty(&(pstat->mesh_mp_ptr)))
                        list_add_tail(&(pstat->mesh_mp_ptr), &(priv->mesh_unEstablish_hdr));

                    RESTORE_INT(flags);
                    return SUCCESS;
                }

                break;
            case MP_HOLDING:
                break;
            default:	// include MP_UNUSED abnormal state, Delete..
                free_stainfo(priv, pstat);
                return FAIL;
        }
        // Here, State must be "CLOSE"
        SAVE_INT_AND_CLI(flags);
        if (list_empty(&(pstat->mesh_mp_ptr)))
            list_add_tail(&(pstat->mesh_mp_ptr), &(priv->mesh_unEstablish_hdr));

        if (MP_HOLDING != pstat->mesh_neighbor_TBL.State)	// Avoid expire timer reset
            pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_PEER_LINK_CLOSE_TO;

        if (!(timer_pending(&priv->mesh_peer_link_timer)))	// start timer if stop
            mod_timer(&priv->mesh_peer_link_timer, jiffies + MESH_TIMER_TO);

        pstat->mesh_neighbor_TBL.State = mesh_PeerLinkStatesTable[pstat->mesh_neighbor_TBL.State][CloseReceived];	// change state
        RESTORE_INT(flags);
        MESH_DEBUG_MSG("Mesh: Recived DisAssoc finish normally\n");
    }
#endif	// MESH secion END

    return SUCCESS;
}


/**
 *	@brief	Recived Local Link state announcement packet
 *
 *	@param	priv: priv
 *			pfrinfo Packet frame data
 *
 *	@retval	void
 */
void OnLocalLinkStateANNOU_MP(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
    struct stat_info	*pstat;
    unsigned char 	*pframe, *p,  meshHeaderOffset;
    int	len;
    UINT16	tmp;

    pstat = get_stainfo(priv, pfrinfo->sa);

    mesh_sme_debug("Reciver Local Link State announcement !! \n");

    if ((NULL == pstat) || !(MP_SUBORDINATE_LINK_UP == pstat->mesh_neighbor_TBL.State || MP_SUBORDINATE_LINK_DOWN_E == pstat->mesh_neighbor_TBL.State))
        return;		 // source address not exist and state not match

    pframe = get_pframe(pfrinfo);

    if(*(pframe + WLAN_HDR_A4_LEN) & BIT(0))	// In mesh header,  mesh flags's Address Extension
        meshHeaderOffset = _MESH_HEADER_WITH_AE_;
    else
        meshHeaderOffset = _MESH_HEADER_WITHOUT_AE_;

    // get Local Link State Announcement IE
    p = get_ie(pframe + WLAN_HDR_A4_LEN + meshHeaderOffset + _MESH_ACTIVE_FIELD_OFFSET_,  _MESH_LINK_METRIC_REPORT_IE_, &len, pfrinfo->pktlen - WLAN_HDR_A4_LEN - meshHeaderOffset);

    if (NULL == p)
        return;

    memcpy(&tmp, p + MESH_IE_BASE_LEN, sizeof(pstat->mesh_neighbor_TBL.r));
    pstat->mesh_neighbor_TBL.r = le16_to_cpu(tmp);

    memcpy(&tmp, p + MESH_IE_BASE_LEN + sizeof(pstat->mesh_neighbor_TBL.r), sizeof(pstat->mesh_neighbor_TBL.r));
    pstat->mesh_neighbor_TBL.ept = le16_to_cpu(tmp);

    mesh_sme_debug("Reciver Local Link State announcement r = %X, ept = %X\n", pstat->mesh_neighbor_TBL.r, pstat->mesh_neighbor_TBL.ept);

    pstat->mesh_neighbor_TBL.Q= pfrinfo->rssi;
    pstat->mesh_neighbor_TBL.State = MP_SUBORDINATE_LINK_UP;
    updateMeshMetric(priv, pstat, pfrinfo);

}


#ifdef PU_STANDARD

void OnProxyUpdateConfirm_MP(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
    unsigned char 	*pframe, *p,  meshHeaderOffset;
    int	len;
    UINT8  PUSN;
    struct path_sel_entry *pthEntry=NULL;
    struct rtl8192cd_priv *xmit_priv = NULL;
    unsigned char nextHopMac[MACADDRLEN];
    unsigned long flags;

    pframe = get_pframe(pfrinfo);

    if(*(pframe + WLAN_HDR_A4_LEN) & BIT(0))	// In mesh header, mesh flags's Address Extension
        meshHeaderOffset = _MESH_HEADER_WITH_AE_;
    else
        meshHeaderOffset = _MESH_HEADER_WITHOUT_AE_;

    // get Proxy Update Confirmation IE
    p = get_ie(pframe + WLAN_HDR_A4_LEN + meshHeaderOffset + _MESH_ACTIVE_FIELD_OFFSET_,  _PROXY_UPDATE_CONFIRM_IE_, &len, pfrinfo->pktlen - WLAN_HDR_A4_LEN - meshHeaderOffset);

    if (NULL == p)
        return;
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
    priv = (DRV_PRIV *)priv->mesh_priv_first;
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE
    if( memcmp(pfrinfo->da, GET_MY_HWADDR, MACADDRLEN) && !IS_MCAST(pfrinfo->da) ) /*this procyUpdateConfirm is not for me*/
    {
        SAVE_INT_AND_CLI(flags);
        SMP_LOCK_MESH_PATH(flags);
        pthEntry = HASH_SEARCH(priv->pathsel_table,pfrinfo->da);
        if(pthEntry)
        {   xmit_priv = pthEntry->priv;
            memcpy(nextHopMac, pthEntry->nexthopMAC, MACADDRLEN);
        }
        RESTORE_INT(flags);
        SMP_UNLOCK_MESH_PATH(flags);

        if(pthEntry)
            issue_proxyupdateconfirm_MP(xmit_priv ,(unsigned char *)(p+3) ,(unsigned char *)(p+4), pfrinfo->da, nextHopMac);
        return;
    }

    PUSN = *((unsigned char *)(p+3));

    SMP_LOCK_MESH_PROXYUPDATE(flags);
    HASH_DELETE(priv->proxyupdate_table,&PUSN);
    SMP_UNLOCK_MESH_PROXYUPDATE(flags);

}

void OnProxyUpdate_MP(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
    unsigned char 	*pframe, *p,  meshHeaderOffset;
    int	len;
    struct proxyupdate_table_entry Entry;
    struct path_sel_entry *pthEntry=NULL;
    struct rtl8192cd_priv *xmit_priv = NULL;
    unsigned long flags;
    unsigned char nextHopMac[MACADDRLEN];

    pframe = get_pframe(pfrinfo);

    if(*(pframe + WLAN_HDR_A4_LEN) & BIT(0))	// In mesh header,  mesh flags's Address Extension
        meshHeaderOffset = _MESH_HEADER_WITH_AE_;
    else
        meshHeaderOffset = _MESH_HEADER_WITHOUT_AE_;

    // get Proxy Update IE
    p = get_ie(pframe + WLAN_HDR_A4_LEN + meshHeaderOffset + _MESH_ACTIVE_FIELD_OFFSET_,  _PROXY_UPDATE_IE_, &len, pfrinfo->pktlen - WLAN_HDR_A4_LEN - meshHeaderOffset);
    if (NULL == p)
        return;

    Entry.PUflag = *(p+2);
    Entry.PUSN = *(p+3);
    memcpy(Entry.proxymac, p+4, MACADDRLEN);
    Entry.STAcount = *(UINT16 *)(p+10);
    memcpy(Entry.proxiedmac, p+12, MACADDRLEN);
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
    priv = (DRV_PRIV *)priv->mesh_priv_first;
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE

    if( Entry.PUflag == PU_add )
    {
        mesh_proxy_update(priv, Entry.proxymac, Entry.proxiedmac);
    }
    else {
        remove_proxy_entry(priv, Entry.proxymac, Entry.proxiedmac);
    }

    //PU is not for me, relay it
    if( memcmp(pfrinfo->da, GET_MY_HWADDR, MACADDRLEN) && !IS_MCAST(pfrinfo->da) )
    {
        SAVE_INT_AND_CLI(flags);
        SMP_LOCK_MESH_PATH(flags);
        pthEntry = HASH_SEARCH(priv->pathsel_table, pfrinfo->da);
        if( pthEntry)
        {
            memcpy(Entry.nexthopmac, pthEntry->nexthopMAC, MACADDRLEN);
            xmit_priv = pthEntry->priv;
        }
        RESTORE_INT(flags);
        SMP_UNLOCK_MESH_PATH(flags);
        
        if(pthEntry) {
            Entry.isMultihop = 0x01;
            memcpy(Entry.destproxymac, pfrinfo->da, MACADDRLEN);
            issue_proxyupdate_MP(xmit_priv,&Entry);
        }
    }
    else
    {
        SAVE_INT_AND_CLI(flags);
        SMP_LOCK_MESH_PATH(flags);
        pthEntry = HASH_SEARCH(priv->pathsel_table, Entry.proxymac);
        if( pthEntry)
        {
            xmit_priv = pthEntry->priv;
            memcpy(nextHopMac,  pthEntry->nexthopMAC, MACADDRLEN);
        }
        RESTORE_INT(flags);
        SMP_UNLOCK_MESH_PATH(flags);
        
        if(pthEntry)
            issue_proxyupdateconfirm_MP(xmit_priv, &Entry.PUSN, GET_MY_HWADDR, Entry.proxymac, nextHopMac);
    }

}
#endif // PU_STANDARD

/*!
    @brief (fixit) process RREQ
*/
unsigned int OnPathSelectionManagFrame(DRV_PRIV *priv, struct rx_frinfo *pfrinfo, int Is_6Addr)
{
    struct stat_info	*pstat;
    unsigned char 	*pframe, *pFrameBody, action_type, element_id;
    DOT11s_RECV_PACKET RECV_data;
    DRV_PRIV * orig_priv = priv;
    pframe = get_pframe(pfrinfo);	// get frame data
    memset((void*)&RECV_data, 0, sizeof(DOT11s_RECV_PACKET));

    if(pframe == NULL)
        return SUCCESS;
    if((pstat = get_stainfo(priv, GetAddr2Ptr(pframe)))==NULL)
        return SUCCESS;

#ifdef RTK_MESH_METRIC_REFINE
    memcpy(RECV_data.InterfaceMACAddr, GET_MY_HWADDR, MACADDRLEN);
#endif

#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
    priv = priv->mesh_priv_first;
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE
    if(Is_6Addr == 1)
    {
        //printk("test recieve 6 address Manag Frame\n");
        unsigned char addr5[MACADDRLEN];
        unsigned char addr6[MACADDRLEN];
        memcpy(addr5, pframe+WLAN_HDR_A4_MESH_DATA_LEN, MACADDRLEN);
        memcpy(addr6, pframe+WLAN_HDR_A4_MESH_DATA_LEN+MACADDRLEN, MACADDRLEN);
 
        if(memcmp(pfrinfo->sa,addr6,MACADDRLEN)) // addr4 <> addr 6, Add proxy table
        {
            mesh_proxy_update(priv, pfrinfo->sa, addr6);
        }
        
        pFrameBody = pframe + WLAN_HDR_A6_MESH_DATA_LEN;
        RECV_data.Is6AddrFormat = 1;
        memcpy(RECV_data.MACAddr5, addr5, MACADDRLEN);
        memcpy(RECV_data.MACAddr6, addr6, MACADDRLEN);
    }
    else
        pFrameBody = pframe + WLAN_HDR_A4_MESH_DATA_LEN;

    action_type = *(pFrameBody+1);
    element_id = *(pFrameBody+2);
    // reason = cpu_to_le16(*(unsigned short *)((unsigned int)pframe + WLAN_HDR_A4_MESH_MGT_LEN )); //len define ref to wifi.h
    switch(action_type)
    {
        case _GANN_ACTION_ID_:
            RECV_data.EventId = DOT11_EVENT_PATHSEL_RECV_PANN;
            break;
        case _HWMP_MESH_PATHSEL_ACTION_ID_:
            switch(element_id)
            {
                case _MESH_RANN_IE_:
                    RECV_data.EventId = DOT11_EVENT_PATHSEL_RECV_RANN;
                    break;
                case _MESH_PREQ_IE_:
                    RECV_data.EventId = DOT11_EVENT_PATHSEL_RECV_RREQ;
                    break;
                case _MESH_PREP_IE_:
                    RECV_data.EventId = DOT11_EVENT_PATHSEL_RECV_RREP;
                    break;
                case _MESH_PERR_IE_:
                    RECV_data.EventId = DOT11_EVENT_PATHSEL_RECV_RERR;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    RECV_data.IsMoreEvent = 0;
    memcpy(RECV_data.MyMACAddr,  GET_MY_HWADDR ,MACADDRLEN);

    // Totti 2008.7.24 test
    //RECV_data.Pre_Hop_Metric = (140 - pfrinfo->rssi);
    // RECV_data.Pre_Hop_Metric = (pfrinfo->rssi<28)?(499+((28-pfrinfo->rssi)<<6)):((100-pfrinfo->rssi)<<1);
#ifndef MESH_USE_METRICOP
    updateMeshMetric(orig_priv, pstat, pfrinfo);
    RECV_data.Pre_Hop_Metric = getMeshMetric(pstat);
#ifdef RTK_MESH_METRIC_REFINE
    RECV_data.RSSI_Metric = pstat->mesh_neighbor_TBL.rssi_metric;
    RECV_data.Cu_Metric = pstat->mesh_neighbor_TBL.cu_metric;
    RECV_data.Noise_Metric = pstat->mesh_neighbor_TBL.noise_metric;
#endif
#else
    if(orig_priv->mesh_fake_mib.metricID == 0)
        RECV_data.Pre_Hop_Metric = (pfrinfo->rssi<28)?(499+((28-pfrinfo->rssi)<<6)):((100-pfrinfo->rssi)<<1);
    else if(orig_priv->mesh_fake_mib.metricID == 1)
        RECV_data.Pre_Hop_Metric = getMeshMetric(orig_priv, pstat, pfrinfo);
    else if(orig_priv->mesh_fake_mib.metricID == 2)
        RECV_data.Pre_Hop_Metric = getMetric(orig_priv, pstat);
    else
        return SUCCESS;
#endif

    memcpy(RECV_data.PreHopMACAddr, GetAddr2Ptr(pframe), MACADDRLEN);
    memcpy(RECV_data.DesMACAddr, GetAddr3Ptr(pframe), MACADDRLEN);

    memcpy(&(RECV_data.ReceiveData),  pFrameBody, 128);
    RECV_data.TTL = *(GetMeshHeaderTTLWithOutQOS(pframe));	// fix:RREQ broadcast flood (bug num: Productlization Phase 1 27, 2007/10/11)
    RECV_data.Seq_num = *(GetMeshHeaderSeqNumWithoutQOS(pframe));

    DOT11_EnQueue2((unsigned long)priv, priv->pathsel_queue, (unsigned char*)&RECV_data, sizeof(DOT11s_RECV_PACKET));
    notifyPathSelection(priv);

    return SUCCESS;
}


int mesh_close(DRV_PRIV *priv) 
{
    int i,tbl_sz;
    struct mesh_rreq_retry_entry *retryEntry;
    struct sk_buff *skb;
    unsigned long flags;

#if defined(RTK_MESH_AODV_STANDALONE_TIMER)
    if (timer_pending(&priv->mesh_expire_timer))
       del_timer_sync(&priv->mesh_expire_timer);
#endif


    SAVE_INT_AND_CLI(flags);
    SMP_LOCK_MESH_PREQ(flags);

    tbl_sz = 1 << priv->mesh_rreq_retry_queue->table_size_power;
    for (i = 0; i < tbl_sz; i++)
    {
        if (priv->mesh_rreq_retry_queue->entry_array[i].dirty)
        {
            retryEntry = ((struct mesh_rreq_retry_entry*)priv->mesh_rreq_retry_queue->entry_array[i].data);
            skb = (struct sk_buff*)deque(priv,&(retryEntry->pktqueue.head),&(retryEntry->pktqueue.tail),(unsigned long)retryEntry->pktqueue.pSkb,NUM_TXPKT_QUEUE);
            while(skb)
            {
                dev_kfree_skb_any(skb);
                skb = (struct sk_buff*)deque(priv,&(retryEntry->pktqueue.head),&(retryEntry->pktqueue.tail),(unsigned long)retryEntry->pktqueue.pSkb,NUM_TXPKT_QUEUE);
            }
            priv->mesh_rreq_retry_queue->entry_array[i].dirty = 0;
        }        
    }
    priv->rreq_head = NULL;
    priv->rreq_tail = NULL;

    RESTORE_INT(flags);
    SMP_UNLOCK_MESH_PREQ(flags);
    
    return 0;
}


#endif	// CONFIG_RTK_MESH
