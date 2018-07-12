/*
 *      Handling routines for Mesh in 802.11 Utils
 *
 *      PS: All extern function in ../8190n_headers.h
 */
#define _MESH_UTIL_C_

#ifdef CONFIG_RTL8192CD
#include "../8192cd.h"
#include "../8192cd_headers.h"
#else
#include "../rtl8190/8190n.h"
#include "../rtl8190/8190n_headers.h"
#endif
#include "./mesh_util.h"
#include <linux/ctype.h>

#ifdef CONFIG_RTK_MESH
int clear_route_info(struct  rtl8192cd_priv *priv, unsigned char *delMAC)
{
    struct path_sel_entry *pEntry = 0;	
    unsigned long flags, i=0;
    int tbl_sz;
    unsigned char destMAC[MACADDRLEN];
    int is_delete;
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
	priv = (DRV_PRIV *)priv->mesh_priv_first;
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE
    tbl_sz = (1 << priv->pathsel_table->table_size_power);

    //clean up path relative to invalid neighbor
    for(;i<tbl_sz;++i)
    {
        is_delete = 0;
    
        SAVE_INT_AND_CLI(flags);
        SMP_LOCK_MESH_PATH(flags);
        if(priv->pathsel_table->entry_array[i].dirty) { 
            pEntry = (struct path_sel_entry *)(priv->pathsel_table->entry_array[i].data);
            if(memcmp(delMAC, pEntry->nexthopMAC,MACADDRLEN)==0) {
                priv->pathsel_table->entry_array[i].dirty = 0;               
                memcpy(destMAC, pEntry->destMAC, MACADDRLEN);
                is_delete = 1;
            }        
        }
        RESTORE_INT(flags);
        SMP_UNLOCK_MESH_PATH(flags);

        if(is_delete) {
            #if defined(RTL_MESH_TXCACHE)
            expire_mesh_txcache(priv, destMAC);
            #endif
        }
    } 

    remove_proxy_owner(priv, delMAC);
    
    return SUCCESS;
}

void mac12_to_6(unsigned char *mac1, unsigned char *mac2)
{
	short m=0;
	for( ; m<MACADDRLEN; m++)
		mac2[m] = (((mac1[1|(m<<1)])>='a')?(mac1[1|(m<<1)]-('a'-0xa)):(mac1[1|(m<<1)]-'0'))|((((mac1[m<<1])>='a')?(mac1[m<<1]-('a'-0xa)):(mac1[m<<1]-'0'))<<4);
}

/*
 *	@brief	Count MESH Association number and display
 *
 *	@param	priv:priv
 *	@param	pstat: pstat
 *	@param	act: action
 *			INCREASE = New peer MP (action: minus peer cap)
 *			DECREASE = Delete exist connect MP (acton: Plus peer cap)
 *
 *	@retval	void
 */
void mesh_cnt_ASSOC_PeerLink_CAP(DRV_PRIV *priv, struct stat_info *pstat, int act)
{
	UINT8	modify = TRUE;

	if (DECREASE == act) {
		if (MESH_PEER_LINK_CAP_NUM(priv) < GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor)
			MESH_PEER_LINK_CAP_NUM(priv)++;
		else {
			modify = FALSE;
			MESH_DEBUG_MSG("MESH PeerLink CAP Number Error (%d)!\n", GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor);
		}
	} else {
		if (MESH_PEER_LINK_CAP_NUM(priv) > 0)
			MESH_PEER_LINK_CAP_NUM(priv)--;
		else {
			modify = FALSE;
			MESH_DEBUG_MSG("MESH PeerLink CAP Number Error (0)!\n");
		}
	}

	if (TRUE == modify)
		update_beacon(priv);		// Because MESH_PEER_LINK_CAP_NUM modify (beacon include WLAN Mesh Capacity)

	mesh_sme_debug("Mesh assoc_num: %s(Max:%d, Remain:%d) %02X:%02X:%02X:%02X:%02X:%02X\n",
		act?"++":"--",
		GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor,
		MESH_PEER_LINK_CAP_NUM(priv),
		pstat->hwaddr[0],
		pstat->hwaddr[1],
		pstat->hwaddr[2],
		pstat->hwaddr[3],
		pstat->hwaddr[4],
		pstat->hwaddr[5]);

}

/*
 *	@brief	Set MESH Association Max number (call by web interface?)
 *			Note: If setting value(meshCapSetValue) less system current connection number,  MESH_PEER_LINK_CAP_NUM become negative, Denied any new connection.
 *			Delete exist connect ONLY!!
 *				SO 
 *					MESH_PEER_LINK_CAP_NUM = 0							: setting = connection, Denied any new connection
 *	 				MESH_PEER_LINK_CAP_NUM > 0 (< MAX_MPP_NUM)	: setting > connection ,Allow new connect
 * 					MESH_PEER_LINK_CAP_NUM = MAX_MPP_NUM		: connection = 0 ,Allow new connect
 *
 *	@param	priv:priv
 *	@param	meshCapSetValue: set connect number
 *
 *	@retval	void
 */
void mesh_set_PeerLink_CAP(DRV_PRIV *priv, UINT16 meshCapSetValue)
{
    struct list_head	*plist;

#ifndef SMP_SYNC
    unsigned long flags;
#endif

    UINT16	count = 0;

    SAVE_INT_AND_CLI(flags);

    meshCapSetValue &= MESH_PEER_LINK_CAP_CAPACITY_MASK;

    if (MAX_MPP_NUM < meshCapSetValue)
        meshCapSetValue = MAX_MPP_NUM;	// Exceed hard code MAX number, Lock on hardcode MAX number

    GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor = meshCapSetValue;	// Writeback runtime Max value (Avoid web interface fault)

    //  Recalculate current connection number by association list (Avoid problem when  CAP value error)
    plist = &(priv->mesh_mp_hdr);
    while (plist->next != &(priv->mesh_mp_hdr))	// 1.Check index  2.Check is it least element? (Because  next pointer to mesh_mp_hdr itself) 
    {
        count++;
        plist = plist->next;		// pointer to next element's list_head struct
    }

    MESH_PEER_LINK_CAP_NUM(priv) = (INT16)(meshCapSetValue - count);	// Setting - current connection number  -> CAP (NOTE: Allow POS/NEG value)
    RESTORE_INT(flags);
    init_beacon(priv);
    return;
}

int remove_proxy_owner(DRV_PRIV *priv, unsigned char *owner)
{
#ifdef SMP_SYNC
    unsigned long flags;
#endif
    struct proxy_table_entry *pxEntry = 0;
    int i, tbl_sz;
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
	priv = (DRV_PRIV *)priv->mesh_priv_first;
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE

    SMP_LOCK_MESH_PROXY(flags);
    //clean up proxy entry relative to invalid neighbor
    tbl_sz = (1 << priv->proxy_table->table_size_power);
    for(i=0;i<tbl_sz;i++)
    {
        if(priv->proxy_table->entry_array[i].dirty) {
            pxEntry = (struct proxy_table_entry *)(priv->proxy_table->entry_array[i].data);
            if(memcmp(owner,pxEntry->owner,MACADDRLEN)==0){
                priv->proxy_table->entry_array[i].dirty = 0;
                mesh_proxy_debug("[%s %d]Delete Proxy entry of %02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x\n",           
                            __func__,__LINE__,pxEntry->owner[0],pxEntry->owner[1],pxEntry->owner[2],pxEntry->owner[3],pxEntry->owner[4],pxEntry->owner[5],
                            pxEntry->sta[0],pxEntry->sta[1],pxEntry->sta[2],pxEntry->sta[3],pxEntry->sta[4],pxEntry->sta[5]);
            }    
        }
    }    
    SMP_UNLOCK_MESH_PROXY(flags);


    return 0;
}


int remove_proxy_entry(DRV_PRIV *priv, unsigned char *owner, unsigned char* sta)
{
#ifdef SMP_SYNC
    unsigned long flags;
#endif
    struct proxy_table_entry* pPXEntry;    
    int process = 0;
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
	priv = (DRV_PRIV *)priv->mesh_priv_first;
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE
    SMP_LOCK_MESH_PROXY(flags);

    pPXEntry = HASH_SEARCH(priv->proxy_table, sta);
    if( pPXEntry ) {
        if( owner == NULL) {
            if(memcmp(pPXEntry->owner, GET_MY_HWADDR, MACADDRLEN) == 0)
                process = 1;
        } 
        else{
            if(memcmp(pPXEntry->owner, owner, MACADDRLEN) == 0) 
                process = 1;
        }

        if(process)
        {
            HASH_DELETE(priv->proxy_table,sta);
            mesh_proxy_debug("[%s %d]Delete Proxy entry of %02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x\n",
                            __func__,__LINE__, pPXEntry->owner[0], pPXEntry->owner[1],pPXEntry->owner[2],pPXEntry->owner[3],pPXEntry->owner[4],pPXEntry->owner[5],
                            sta[0],sta[1],sta[2],sta[3],sta[4],sta[5]);
            #if defined(CONFIG_RTL_MESH_CROSSBAND)
            sync_proxy_info(priv->mesh_priv_sc, sta, 2); //delete proxy info of another band 
            #endif

            #if defined(RTL_MESH_TXCACHE)
            priv->mesh_txcache.dirty = 0;
            #endif
        }
    }    

    SMP_UNLOCK_MESH_PROXY(flags);

    return 0;
}


int mesh_proxy_update(DRV_PRIV *priv, unsigned char* owner, unsigned char* sta) {
#ifdef SMP_SYNC
    unsigned long flags;
#endif
    struct proxy_table_entry*   pProxyEntry = NULL, Entry;
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
	priv = (DRV_PRIV *)priv->mesh_priv_first;
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE
    SMP_LOCK_MESH_PROXY(flags);

    pProxyEntry = (struct proxy_table_entry*) HASH_SEARCH(priv->proxy_table, sta);    
    if(!pProxyEntry) {
        memcpy(Entry.sta, sta, MACADDRLEN);
        memcpy(Entry.owner, owner, MACADDRLEN);
        Entry.aging_time = 0;
        HASH_INSERT(priv->proxy_table, Entry.sta, &Entry);

        mesh_proxy_debug("[A6 forwardding]Insert Proxy table: %02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x\n",
                            Entry.owner[0],Entry.owner[1],Entry.owner[2],Entry.owner[3],Entry.owner[4],Entry.owner[5],
                            Entry.sta[0],Entry.sta[1],Entry.sta[2],Entry.sta[3],Entry.sta[4],Entry.sta[5]);
        
        #if defined(CONFIG_RTL_MESH_CROSSBAND)
        sync_proxy_info(priv->mesh_priv_sc,Entry.sta, 0); //refresh proxy info of another band 
        #endif
    } else if(memcmp(pProxyEntry->owner, owner, MACADDRLEN)) {

        mesh_proxy_debug("[A6 forwardding]Update Proxy table:from %02x:%02x:%02x:%02x:%02x:%02x to  %02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x\n",
                        pProxyEntry->owner[0],pProxyEntry->owner[1],pProxyEntry->owner[2],pProxyEntry->owner[3],pProxyEntry->owner[4],pProxyEntry->owner[5],
                        owner[0],owner[1],owner[2],owner[3],owner[4],owner[5],
                        pProxyEntry->sta[0],pProxyEntry->sta[1],pProxyEntry->sta[2],pProxyEntry->sta[3],pProxyEntry->sta[4],pProxyEntry->sta[5]);



        memcpy(pProxyEntry->owner, owner, MACADDRLEN);
        pProxyEntry->aging_time = 0;

        #if defined(RTL_MESH_TXCACHE)
        priv->mesh_txcache.dirty = 0;
        #endif
        
        #if defined(CONFIG_RTL_MESH_CROSSBAND)
        sync_proxy_info(priv->mesh_priv_sc,pProxyEntry->sta, 2); //delete proxy info of another band 
        #endif
    } else {
        pProxyEntry->aging_time = 0;
        mesh_proxy_debug("[A6 forwardding]Refresh Proxy table: %02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x\n",
                            pProxyEntry->owner[0],pProxyEntry->owner[1],pProxyEntry->owner[2],pProxyEntry->owner[3],pProxyEntry->owner[4],pProxyEntry->owner[5],
                            pProxyEntry->sta[0],pProxyEntry->sta[1],pProxyEntry->sta[2],pProxyEntry->sta[3],pProxyEntry->sta[4],pProxyEntry->sta[5]);

        #if defined(CONFIG_RTL_MESH_CROSSBAND)
        sync_proxy_info(priv->mesh_priv_sc, pProxyEntry->sta, 0); //refresh proxy info of another band 
        #endif

    }

    SMP_UNLOCK_MESH_PROXY(flags);
    return 0;

}


static int __mesh_proxy_insert(struct rtl8192cd_priv *priv, unsigned char *sta) {
#ifdef SMP_SYNC
    unsigned long flags;
#endif
    struct proxy_table_entry Entry, *pEntry;

    if(!IS_DRV_OPEN(priv) || GET_MIB(priv)->dot1180211sInfo.mesh_enable == 0) {
        return -1;
    }

    SMP_LOCK_MESH_PROXY(flags);    
    pEntry = HASH_SEARCH(priv->proxy_table, sta);    
    if(pEntry) {

        pEntry->aging_time = -1;  /*never timeout*/         

        if(memcmp(pEntry->owner, GET_MY_HWADDR, MACADDRLEN)) {
            mesh_proxy_debug("[%s %d]Update Proxy table from %02x:%02x:%02x:%02x:%02x:%02x to %02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x\n",
                            __func__,__LINE__, pEntry->owner[0],pEntry->owner[1],pEntry->owner[2],pEntry->owner[3],pEntry->owner[4],pEntry->owner[5],
                            priv->pmib->dot11OperationEntry.hwaddr[0], priv->pmib->dot11OperationEntry.hwaddr[1],
                            priv->pmib->dot11OperationEntry.hwaddr[2], priv->pmib->dot11OperationEntry.hwaddr[3],
                            priv->pmib->dot11OperationEntry.hwaddr[4], priv->pmib->dot11OperationEntry.hwaddr[5],                                
                            pEntry->sta[0],pEntry->sta[1],pEntry->sta[2],pEntry->sta[3],pEntry->sta[4],pEntry->sta[5]);
            memcpy(pEntry->owner, GET_MY_HWADDR, MACADDRLEN);
        
            #if defined(RTL_MESH_TXCACHE)
            priv->mesh_txcache.dirty = 0;
            #endif
            
            SMP_UNLOCK_MESH_PROXY(flags);
            
            #if defined(PU_STANDARD_SME)
            issue_proxyupdate_ADD(priv, sta);
            #endif    
        }
        else {
            mesh_proxy_debug("[%s %d]Refresh Proxy table: %02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x\n",
                            __func__,__LINE__,pEntry->owner[0],pEntry->owner[1],pEntry->owner[2],pEntry->owner[3],pEntry->owner[4],pEntry->owner[5],
                            pEntry->sta[0],pEntry->sta[1],pEntry->sta[2],pEntry->sta[3],pEntry->sta[4],pEntry->sta[5]); 
            SMP_UNLOCK_MESH_PROXY(flags);

        }

    }
    else {
        memcpy(Entry.sta, sta, MACADDRLEN);
        memcpy(Entry.owner, GET_MY_HWADDR, MACADDRLEN);
        Entry.aging_time = -1; /*never timeout*/
        HASH_INSERT(priv->proxy_table, Entry.sta, &Entry);
        
        SMP_UNLOCK_MESH_PROXY(flags);
        
        mesh_proxy_debug("[%s %d]Insert Proxy table: %02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x\n",
                __func__,__LINE__,Entry.owner[0],Entry.owner[1],Entry.owner[2],Entry.owner[3],Entry.owner[4],Entry.owner[5],
                Entry.sta[0],Entry.sta[1],Entry.sta[2],Entry.sta[3],Entry.sta[4],Entry.sta[5]);
        
        #if defined(PU_STANDARD_SME)
        issue_proxyupdate_ADD(priv, sta);
        #endif          
    }        
    
    return 0;

}

void mesh_proxy_insert(struct rtl8192cd_priv *priv, unsigned char *sta)
{
    if(netif_running(priv->mesh_dev)) {
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
		priv = priv->mesh_priv_first;
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE
        if(priv->pmib->dot1180211sInfo.mesh_portal_enable) {
            __mesh_proxy_insert(priv, sta); 

            #if defined(CONFIG_RTL_MESH_CROSSBAND)
            if(priv->mesh_priv_sc) {
                if(__mesh_proxy_insert(priv->mesh_priv_sc, sta)) //insert proxy info
                    mesh_proxy_debug("Sync proxy information to %s failed\n",priv->mesh_priv_sc->dev->name);
            }
            #endif
        }
    }
}

#endif // _DOT11_MESH_MODE

