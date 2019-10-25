/*
 *  a4 sta functions
 *
 *  $Id: 8192cd_a4_sta.c,v 1.1 2010/10/13 06:38:58 davidhsu Exp $
 *
 *  Copyright (c) 2010 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_A4_STA_C_

#ifdef __KERNEL__
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/timer.h>
#endif

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"

#ifdef A4_STA

#define A4_VERSION 0x01

#if (A4_STA_OUI == A4_STA_OUT_REALTEK)
const unsigned char a4_oui[] = {0x00, 0xe0, 0x4c};
#elif (A4_STA_OUI == A4_STA_OUT_CUSTOMER_1)
const unsigned char a4_oui[] = {0x00, 0x0d, 0x02};
#define ENABLE_ECM_1_0
#endif

//#define A4_STA_DEBUG

static struct a4_sta_db_entry *alloc_entry(struct rtl8192cd_priv *priv)
{
    int i;

    for (i=0; i<MAX_A4_TBL_NUM; i++)
    {
        if (!priv->a4_ent[i].used)
        {
            priv->a4_ent[i].used = 1;
            return &priv->a4_ent[i].entry;
        }
    }
    return NULL;
}

static void free_entry(struct rtl8192cd_priv *priv,  struct a4_sta_db_entry *entry)
{
    int i;

    for (i=0; i<MAX_A4_TBL_NUM; i++)
    {
        if (priv->a4_ent[i].used && (entry == &priv->a4_ent[i].entry))
        {
            priv->a4_ent[i].used = 0;
            break;
        }
    }
}

static int mac_hash(unsigned char *networkAddr)
{
    unsigned long x;

    x = networkAddr[0] ^ networkAddr[1] ^ networkAddr[2] ^ networkAddr[3] ^ networkAddr[4] ^ networkAddr[5];

    return x & (A4_STA_HASH_SIZE - 1);
}

static void mac_hash_link(struct rtl8192cd_priv *priv, struct a4_sta_db_entry *ent, int hash)
{
    ent->next_hash = priv->machash[hash];

    if (ent->next_hash != NULL)
        ent->next_hash->pprev_hash = &ent->next_hash;
    priv->machash[hash] = ent;
    ent->pprev_hash = &priv->machash[hash];
}

static void mac_hash_unlink(struct a4_sta_db_entry *ent)
{
    *(ent->pprev_hash) = ent->next_hash;
    if (ent->next_hash != NULL)
        ent->next_hash->pprev_hash = ent->pprev_hash;
    ent->next_hash = NULL;
    ent->pprev_hash = NULL;
}

static unsigned long _timeout(struct rtl8192cd_priv *priv)
{
    unsigned long timeout;

    timeout = jiffies - A4_STA_AGEING_TIME*HZ;
    return timeout;
}

static  int  has_expired(struct rtl8192cd_priv *priv, struct a4_sta_db_entry *fdb)
{
    if (time_before_eq(fdb->ageing_timer, _timeout(priv)))
        return 1;

    return 0;
}

void a4_sta_del(struct rtl8192cd_priv *priv,  unsigned char *mac)
{
    struct a4_sta_db_entry *db;
    int hash;

    ASSERT(mac);

    hash = mac_hash(mac);
    db = priv->machash[hash];

    while (db != NULL)
    {
        if (!memcmp(db->mac, mac, ETH_ALEN))
        {
            #if defined(BR_SHORTCUT) && defined(RTL_CACHED_BR_STA)
            release_brsc_cache(db->mac);
            #endif           
            mac_hash_unlink(db);
            free_entry(priv, db);
#ifdef A4_STA_DEBUG
            panic_printk("%s A4 STA DEL emac:%02x%02x%02x%02x%02x%02x, wmac:%02x%02x%02x%02x%02x%02x\n",
                         priv->dev->name,
                         db->mac[0],
                         db->mac[1],
                         db->mac[2],
                         db->mac[3],
                         db->mac[4],
                         db->mac[5],
                         db->stat->hwaddr[0],
                         db->stat->hwaddr[1],
                         db->stat->hwaddr[2],
                         db->stat->hwaddr[3],
                         db->stat->hwaddr[4],
                         db->stat->hwaddr[5]);
#endif
            return;
        }
        db = db->next_hash;
    }
}


void a4_sta_update(struct rtl8192cd_priv *root_priv, struct rtl8192cd_priv *priv,  unsigned char *mac)
{
    struct rtl8192cd_priv *current_priv;
    int j;

#ifdef A4_STA_DEBUG
    panic_printk("A4 STA UPDATE emac:%02x%02x%02x%02x%02x%02x\n",
                 mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
#endif

    current_priv = root_priv;
    if(IS_DRV_OPEN(current_priv) && priv != current_priv)
    {
        a4_sta_del(current_priv, mac);
    }

#ifdef MBSSID
    if (root_priv->pmib->miscEntry.vap_enable)
    {
        for (j=0; j<RTL8192CD_NUM_VWLAN; j++)
        {
            current_priv = root_priv->pvap_priv[j];
            if(IS_DRV_OPEN(current_priv) && priv != current_priv)
            {
                a4_sta_del(current_priv, mac);
            }
        }
    }
#endif
#ifdef UNIVERSAL_REPEATER
    current_priv = GET_VXD_PRIV(root_priv);
    if(IS_DRV_OPEN(current_priv) && priv != current_priv)
    {
        a4_sta_del(current_priv, mac);
    }
#endif

}

void a4_sta_cleanup_all(struct rtl8192cd_priv *priv)
{
    int i;

    if (priv->pshare->rf_ft_var.a4_enable)
    {
        for (i=0; i<A4_STA_HASH_SIZE; i++)
        {
            struct a4_sta_db_entry *f;
            f = priv->machash[i];

            while (f != NULL)
            {
                struct a4_sta_db_entry *g;

                g = f->next_hash;
                mac_hash_unlink(f);
                free_entry(priv, f);
                f = g;
            }
        }
    }
}

void a4_sta_cleanup(struct rtl8192cd_priv *priv, struct stat_info * pstat)
{
    int i;
    struct a4_sta_db_entry *f;
    struct a4_sta_db_entry *g;
    for (i=0; i<A4_STA_HASH_SIZE; i++)
    {
        f = priv->machash[i];
        while (f != NULL)
        {
            g = f->next_hash;
            if(f->stat == pstat) {
                #if defined(BR_SHORTCUT) && defined(RTL_CACHED_BR_STA)
                release_brsc_cache(f->mac);
                #endif                
                mac_hash_unlink(f);
                free_entry(priv, f);
            }
            f = g;
        }
    }
}


void a4_sta_expire(struct rtl8192cd_priv *priv)
{
    int i;

    if (priv->pshare->rf_ft_var.a4_enable)
    {
        for (i=0; i<A4_STA_HASH_SIZE; i++)
        {
            struct a4_sta_db_entry *f;
            f = priv->machash[i];

            while (f != NULL)
            {
                struct a4_sta_db_entry *g;
                g = f->next_hash;

                if (has_expired(priv, f))
                {
#ifdef A4_STA_DEBUG
                    panic_printk("%s A4 STA Expire (%02d) emac:%02x%02x%02x%02x%02x%02x, wmac:%02x%02x%02x%02x%02x%02x\n",
                                 priv->dev->name,
                                 i,
                                 f->mac[0],
                                 f->mac[1],
                                 f->mac[2],
                                 f->mac[3],
                                 f->mac[4],
                                 f->mac[5],
                                 f->stat->hwaddr[0],
                                 f->stat->hwaddr[1],
                                 f->stat->hwaddr[2],
                                 f->stat->hwaddr[3],
                                 f->stat->hwaddr[4],
                                 f->stat->hwaddr[5]);
#endif

                    #if defined(BR_SHORTCUT) && defined(RTL_CACHED_BR_STA)
                    release_brsc_cache(f->mac);
                    #endif    
                    mac_hash_unlink(f);
                    free_entry(priv, f);
                }
                f = g;
            }
        }
    }
}

void a4_sta_add(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *mac)
{
    struct a4_sta_db_entry *db;
    int hash;
    unsigned char tmpbuf[15];

    ASSERT(mac);

    hash = mac_hash(mac);
    db = priv->machash[hash];

    while (db != NULL)
    {
        if (!memcmp(db->mac, mac, ETH_ALEN))
        {
            db->stat = pstat;
            db->ageing_timer = jiffies;
            return;
        }
        db = db->next_hash;
    }

    if(memcmp(pstat->hwaddr, mac, MACADDRLEN)) {
        sprintf((char *)tmpbuf, "%02x%02x%02x%02x%02x%02xno", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        del_sta(priv, tmpbuf); 
    }

    db = alloc_entry(priv);
    if (db == NULL)
    {
        DEBUG_ERR("alloc_entry() failed for a4_sta_db_entry!\n");
        return;
    }
    memcpy(db->mac, mac, ETH_ALEN);
    db->stat = pstat;
    db->ageing_timer = jiffies;

#ifdef A4_STA_DEBUG
    panic_printk("%s A4 STA Add emac:%02x%02x%02x%02x%02x%02x, wmac:%02x%02x%02x%02x%02x%02x\n",
                 priv->dev->name,
                 db->mac[0],
                 db->mac[1],
                 db->mac[2],
                 db->mac[3],
                 db->mac[4],
                 db->mac[5],
                 db->stat->hwaddr[0],
                 db->stat->hwaddr[1],
                 db->stat->hwaddr[2],
                 db->stat->hwaddr[3],
                 db->stat->hwaddr[4],
                 db->stat->hwaddr[5]);
#endif

    mac_hash_link(priv, db, hash);

    a4_sta_update(GET_ROOT(priv), priv, mac);

}

struct stat_info *a4_sta_lookup(struct rtl8192cd_priv *priv, unsigned char *mac)
{
    struct a4_sta_db_entry *db;

    ASSERT(mac);

    db = priv->machash[mac_hash(mac)];

    while (db != NULL)
    {
        if (!memcmp(db->mac, mac, ETH_ALEN))
        {

#ifdef A4_STA_DEBUG
            panic_printk("%s A4 STA LOOKUP emac:%02x%02x%02x%02x%02x%02x, wmac:%02x%02x%02x%02x%02x%02x\n",
                         priv->dev->name,
                         db->mac[0],
                         db->mac[1],
                         db->mac[2],
                         db->mac[3],
                         db->mac[4],
                         db->mac[5],
                         db->stat->hwaddr[0],
                         db->stat->hwaddr[1],
                         db->stat->hwaddr[2],
                         db->stat->hwaddr[3],
                         db->stat->hwaddr[4],
                         db->stat->hwaddr[5]);
#endif
            return db->stat;
        }

        db = db->next_hash;
    }
    return NULL;
}

void add_a4_client(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
    struct list_head *phead, *plist;
    struct stat_info *sta;

    if (!netif_running(priv->dev))
        return;

    phead = &priv->a4_sta_list;
    plist = phead->next;

    while (plist != phead)
    {
        sta = list_entry(plist, struct stat_info, a4_sta_list);
        if (!memcmp(sta->hwaddr, pstat->hwaddr, WLAN_ADDR_LEN))
        {
            ASSERT(pstat == sta);
            break;
        }
        plist = plist->next;
    }

    if (plist == phead)
        list_add_tail(&pstat->a4_sta_list, &priv->a4_sta_list);

    pstat->state |= WIFI_A4_STA;
}

unsigned char parse_a4_ie(struct rtl8192cd_priv *priv, unsigned char* p, int limit)
{

    unsigned char * pframe = p;
    int len = 0;
    for (;;)
    {
        p = get_ie(p, _RSN_IE_1_, &len,
                   limit - (p - pframe));
        if (p != NULL)
        {
            if (!memcmp(p+2, a4_oui, 3))
            {
                if(p[5] == 0x03 && (p[8] & BIT6))   /*check ECM 2.0*/
                {
                    return 1;
                }
            }
        }
        else
            break;

        p = p + len + 2;
    }

    return 0;
}

void a4_tx_unknown_unicast(struct rtl8192cd_priv *priv, struct sk_buff *skb)
{
    struct list_head *phead, *plist;
    struct stat_info *pstat, *sa_stat;
    struct sk_buff *newskb;


    sa_stat = get_stainfo(priv, skb->data+MACADDRLEN);
    if(sa_stat == NULL)
    {
        sa_stat = a4_sta_lookup(priv, skb->data+MACADDRLEN);
    }

    phead = &priv->a4_sta_list;
    plist = phead->next;
    while (phead && (plist != phead))
    {
        pstat = list_entry(plist, struct stat_info, a4_sta_list);
        plist = plist->next;
        if(pstat != sa_stat)
        {
            newskb = skb_copy(skb, GFP_ATOMIC);
            if (newskb)
            {
                newskb->cb[2] = (char)0xff;         // not do aggregation
                #if defined(CONFIG_RTK_MESH)
                __rtl8192cd_start_xmit_out(newskb, pstat, NULL);
                #else                
                __rtl8192cd_start_xmit_out(newskb, pstat);
                #endif
            }
            else
            {
                return;
            }
        }
    }

}

unsigned char a4_tx_mcast_to_unicast(struct rtl8192cd_priv *priv, struct sk_buff *skb)
{
    struct list_head *phead, *plist;
    struct stat_info *pstat, *sa_stat;
    struct sk_buff *newskb;

    int sta_count = 0;

    sa_stat = get_stainfo(priv, skb->data+MACADDRLEN);
    if(sa_stat == NULL)
    {
        sa_stat = a4_sta_lookup(priv, skb->data+MACADDRLEN);
    }

    phead = &priv->asoc_list;
    plist = phead->next;
    while (phead && (plist != phead))
    {
        pstat = list_entry(plist, struct stat_info, asoc_list);
        plist = plist->next;

        if(pstat != sa_stat)
        {
            if(pstat->state & WIFI_A4_STA)
            {
                newskb = skb_copy(skb, GFP_ATOMIC);
                if (newskb)
                {
                    newskb->cb[2] = (char)0xff;         // not do aggregation

                    #if defined(CONFIG_RTK_MESH)
                    __rtl8192cd_start_xmit_out(newskb, pstat, NULL);
                    #else                      
                    __rtl8192cd_start_xmit_out(newskb, pstat);
                    #endif
                }
                else
                {
                    continue;
                }
            }
            else
            {
                sta_count++;
            }
        }
    }

    if(sta_count == 0)
    {
        return 1;
    }
    return 0;
}

int a4_rx_dispatch(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo
#ifdef MBSSID
                   ,int vap_idx
#endif
                  )
{

    unsigned int opmode;
    unsigned char *pframe = get_pframe(pfrinfo);
    int reuse = 1;

#ifdef MBSSID
    if (GET_ROOT(priv)->pmib->miscEntry.vap_enable && (vap_idx >= 0))
    {
        priv = priv->pvap_priv[vap_idx];
    }
#endif

    opmode = OPMODE;

#ifdef CLIENT_MODE
    if((opmode & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) ==
            (WIFI_STATION_STATE | WIFI_ASOC_STATE))
    {
        if (IS_BSSID(priv, GetAddr2Ptr(pframe)))
        {
            reuse = a4_rx_check_reuse(priv, pfrinfo, GET_MY_HWADDR);
        }
    }
    else
#endif
        if (opmode & WIFI_AP_STATE)
        {
#ifdef UNIVERSAL_REPEATER
            if(IS_DRV_OPEN(GET_VXD_PRIV(priv)) && IS_BSSID(GET_VXD_PRIV(priv), GetAddr2Ptr(pframe)))
            {
                reuse = a4_rx_check_reuse(GET_VXD_PRIV(priv), pfrinfo, GET_MY_HWADDR);
                if (reuse == 0)
                {
                    priv = GET_VXD_PRIV(priv);
                }
            }
            else
#endif
                if (IS_BSSID(priv, GetAddr1Ptr(pframe)))
                {
                    reuse = 0;
                }

        }

    if(reuse == 0)
    {
#ifdef RTL8190_DIRECT_RX
        rtl8192cd_rx_dataframe(priv, NULL, pfrinfo);
#else
        SMP_LOCK_RX_DATA(x);
        list_add_tail(&(pfrinfo->rx_list), &(priv->rx_datalist));
        SMP_UNLOCK_RX_DATA(x);
#endif

    }

    rx_sum_up(priv, NULL, pfrinfo);

    return reuse;
}


struct stat_info *a4_rx_check_reuse_ap(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo)
{
        if(priv->pshare->rf_ft_var.a4_enable == 2) {
            /*only recieve 4 address packet from A4 client when a4_enable = 2*/
             if(((pstat->state & WIFI_A4_STA) && pfrinfo->to_fr_ds != 3) ||
                 (!(pstat->state & WIFI_A4_STA) && pfrinfo->to_fr_ds == 3)
             ) {
            return NULL;
            }
            }
        else {
            if(!(pstat->state & WIFI_A4_STA) && pfrinfo->to_fr_ds == 3)
                add_a4_client(priv, pstat);
        }

        if(pfrinfo->to_fr_ds == 3 && (pstat->state & WIFI_A4_STA))
        {
            a4_sta_add(priv, pstat, pfrinfo->sa);
        }

    return pstat;
}


#ifdef CLIENT_MODE
unsigned char a4_rx_check_reuse(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, unsigned char * myhwaddr)
{
    struct stat_info * pstat;
    unsigned char reuse = 1;
    unsigned char *pframe = get_pframe(pfrinfo);
    struct net_bridge_fdb_entry *fdb;

    pstat = get_stainfo(priv, GetAddr2Ptr(pframe));  //bssid
    if(pstat)
    {
        if(pstat->state & WIFI_A4_STA)   /*A4 AP*/
        {
            /* when a4_enable = 1,  also recieve 3-address from an A4 AP*/
            if(pfrinfo->to_fr_ds == 1 && priv->pshare->rf_ft_var.a4_enable == 1) {                                    

                if(IS_MCAST(pfrinfo->da)) {                   
                    /*filter*/
                    reuse = 0;
                    fdb = fdb_find_for_driver(GET_BR_PORT(priv->dev)->br, pfrinfo->sa);
                    if(fdb) {                       
                        /*if sa is recorded in other interface, it may be a loop packet, drop it*/
                        if(strcmp(fdb->dst->dev->name, priv->dev->name))
                            reuse = 1;                            
                    }       
                }
                else if(isEqualMACAddr(pfrinfo->da, myhwaddr)){ /*unicast*/
                    reuse = 0;
                }

            }
            /*other case, only recieve 4 address packet*/
            else if(pfrinfo->to_fr_ds == 3 && isEqualMACAddr(GetAddr1Ptr(pframe), myhwaddr))
            {
                a4_sta_add(priv, pstat, pfrinfo->sa);
                if(!IS_MCAST(pfrinfo->da))
                    a4_sta_del(priv, pfrinfo->da);
                reuse = 0;
            }
        }
        else  /*legacy AP*/
        {
            /*only recieve 3 address packet*/
            if (pfrinfo->to_fr_ds == 1 && (IS_MCAST(pfrinfo->da) || isEqualMACAddr(pfrinfo->da, myhwaddr)))
            {
                reuse = 0;
            }
        }
    }

    return reuse;
}
#endif

#ifndef __OSK__
#ifdef CONFIG_RTL_PROC_NEW
int a4_dump_sta_info(struct seq_file *s, void *data)
#else
int a4_dump_sta_info(char *buf, char **start, off_t offset,
                     int length, int *eof, void *data)
#endif
{
    struct net_device *dev = PROC_GET_DEV();
    struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
    struct a4_sta_db_entry *db;
    int i;
    int pos = 0;
    PRINT_ONE(" -- A4 STA info -- ", "%s", 1);

    for (i = 0 ; i < A4_STA_HASH_SIZE; i++)
    {
        db = priv->machash[i];
        while (db != NULL)
        {
            PRINT_ARRAY_ARG("    STA_MAC: ",    db->mac, "%02x", MACADDRLEN);
            PRINT_ARRAY_ARG("    OWNER_MAC: ",  db->stat->hwaddr, "%02x", MACADDRLEN);
            PRINT_ONE((jiffies - db->ageing_timer)/HZ,  "    Aging time: %ld", 1);
            db = db->next_hash;
        }
    }



    return pos;
}
#endif

#endif /* A4_STA */


#if defined(TV_MODE) || defined(A4_STA)
unsigned char * construct_ecm_tvm_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen, unsigned char mode)
{
    unsigned char temp[7];
    memcpy(temp, a4_oui, 3);
    *(unsigned short *)(temp + 4) = cpu_to_le16(A4_VERSION);
    temp[6] = 0;

    if(mode & BIT0)   /*ECM*/
    {
#ifdef ENABLE_ECM_1_0
        temp[3] = 0x02; /*OUI type */
        temp[6] = BIT7; /*ECM 1.0*/
        pbuf = set_ie(pbuf, _RSN_IE_1_, 7, temp, frlen);
#endif
        temp[6] = BIT6; /*ECM 2.0*/
    }

#ifdef TV_MODE
    if(mode & BIT1)  /* TV mode*/
    {
        if(priv->tv_mode_status & BIT0)  /*tv mode is enabled*/
        {
            temp[6] |= BIT7;
        }
    }
#endif

    temp[3] = 0x03; /*OUI type */
    pbuf = set_ie(pbuf, _RSN_IE_1_, 7, temp, frlen);
    return pbuf;
}
#endif

#ifdef TV_MODE
unsigned char * construct_tv_mode_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen)
{
    unsigned char temp[7];
    memcpy(temp, a4_oui, 3);
    *(unsigned short *)(temp + 4) = cpu_to_le16(A4_VERSION);

    temp[3] = 0x04; /*OUI type */
    temp[6] = BIT7; /*TV Auto*/
    pbuf = set_ie(pbuf, _RSN_IE_1_, 7, temp, frlen);
    return pbuf;
}

unsigned char parse_tv_mode_ie(struct rtl8192cd_priv *priv, unsigned char* p, int limit)
{

    unsigned char * pframe = p;
    int len = 0;
    for (;;)
    {
        p = get_ie(p, _RSN_IE_1_, &len,
                   limit - (p - pframe));
        if (p != NULL)
        {
            if (!memcmp(p+2, a4_oui, 3))
            {
                if(p[5] == 0x04 && (p[8] & BIT7))   /*check TV Auto*/
                {
                    return 1;
                }
            }
        }
        else
            break;

        p = p + len + 2;
    }

    return 0;
}

void tv_mode_auto_support_check(struct rtl8192cd_priv *priv)
{
    struct stat_info    *pstat;
    struct list_head    *phead, *plist;
    //unsigned long flags;

    phead = &priv->asoc_list;
    plist = phead->next;

    priv->tv_mode_status &= ~BIT0;
    while(plist != phead)
    {
        pstat = list_entry(plist, struct stat_info, asoc_list);
        plist = plist->next;

        if(pstat->tv_auto_support && pstat->expire_to > 0)
        {
            priv->tv_mode_status|= BIT0;
            break;
        }
    }
}

#ifdef SUPPORT_TX_MCAST2UNI
/*return: 0: no client joint, 1: has client joint*/
unsigned char tv_mode_igmp_group_check(struct rtl8192cd_priv *priv, struct sk_buff *skb)
{
    struct stat_info *pstat;
    struct list_head *phead, *plist;
    int i;

    phead = &priv->asoc_list;
    plist = phead;
    while ((plist = asoc_list_get_next(priv, plist)) != phead)
    {
        pstat = list_entry(plist, struct stat_info, asoc_list);
        /* avoid   come from STA1 and send back STA1 */
        if (!memcmp(pstat->hwaddr, &skb->data[6], 6))
        {
            continue;
        }
        if(pstat->leave || pstat->expire_to==0)
            continue;
        for (i=0; i<MAX_IP_MC_ENTRY; i++)
        {
            if (pstat->ipmc[i].used && !memcmp(&pstat->ipmc[i].mcmac[0], skb->data, 6))
            {
                return 1;
            }
        }
    }
    return 0;

}
#endif
#endif
