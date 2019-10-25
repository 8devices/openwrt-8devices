/*
 *  Handle routines to communicate with AUTH daemon (802.1x authenticator)
 *
 *  $Id: 8192cd_security.c,v 1.10.2.9 2011/01/10 08:19:42 jimmylin Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_SECURITY_C_
#ifdef __KERNEL__
#include <asm/uaccess.h>
#include <linux/module.h>
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#endif

#include "./8192cd_cfg.h"

#if !defined(__KERNEL__) && !defined(__ECOS)
#include "./sys-support.h"
#endif

#include "./8192cd.h"
#include "./8192cd_security.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"

#if defined(CONFIG_RTL_WAPI_SUPPORT)
#include "wapi_wai.h"
#endif

#ifdef INCLUDE_WPS
#include "./wps/wsc.h"
#endif

#ifdef RTK_NL80211
#include "8192cd_cfg80211.h" 
#endif


#define E_MSG_DOT11_2LARGE      "ItemSize Too Large"
#define E_MSG_DOT11_QFULL       "Event Queue Full"
#define E_MSG_DOT11_QEMPTY      "Event Queue Empty"

#if defined(SUPPORT_UCFGING_LED) 
extern unsigned int LED_Configuring;
extern struct timer_list	LED_TimerCFGING;
#endif

void DOT11_InitQueue(DOT11_QUEUE * q)
{
	q->Head = 0;
	q->Tail = 0;
	q->NumItem = 0;
	q->MaxItem = MAXQUEUESIZE;
}

#ifndef WITHOUT_ENQUEUE
/*--------------------------------------------------------------------------------
  Return Value
Success : return 0;
Fail	 : return E_DOT11_QFULL if queue is full
return E_DOT1ZX_2LARGE if the item size is large than allocated
--------------------------------------------------------------------------------*/
int DOT11_EnQueue(unsigned long task_priv, DOT11_QUEUE *q, unsigned char *item, int itemsize)
{
#if defined(__KERNEL__) || defined(__OSK__)
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
#endif
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	
	if (itemsize > MAXDATALEN)
		return E_DOT11_2LARGE;

	SAVE_INT_AND_CLI(flags);
	
	if (DOT11_IsFullQueue(q)) {
		RESTORE_INT(flags);
		return E_DOT11_QFULL;
	}
	
	q->ItemArray[q->Tail].ItemSize = itemsize;
	memset(q->ItemArray[q->Tail].Item, 0, sizeof(q->ItemArray[q->Tail].Item));
	memcpy(q->ItemArray[q->Tail].Item, item, itemsize);
	q->NumItem++;
	if((q->Tail+1) == MAXQUEUESIZE)
		q->Tail = 0;
	else
		q->Tail++;

	RESTORE_INT(flags);
	return 0;
}


int DOT11_DeQueue(unsigned long task_priv, DOT11_QUEUE *q, unsigned char *item, int *itemsize)
{
#if defined(__KERNEL__) || defined(__OSK__)
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
#endif
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	SAVE_INT_AND_CLI(flags);
	
	if (DOT11_IsEmptyQueue(q)) {
		RESTORE_INT(flags);
		return E_DOT11_QEMPTY;
	}
	
	memcpy(item, q->ItemArray[q->Head].Item, q->ItemArray[q->Head].ItemSize);
	*itemsize = q->ItemArray[q->Head].ItemSize;
	q->NumItem--;
	if((q->Head+1) == MAXQUEUESIZE)
		q->Head = 0;
	else
		q->Head++;

	RESTORE_INT(flags);
	return 0;
}
#endif // !WITHOUT_ENQUEUE


#if 0
void DOT11_PrintQueue(DOT11_QUEUE *q)
{
	int i, j, index;

	printk("\n/-------------------------------------------------\n[DOT11_PrintQueue]: MaxItem = %d, NumItem = %d, Head = %d, Tail = %d\n", q->MaxItem, q->NumItem, q->Head, q->Tail);
	for(i=0; i<q->NumItem; i++) {
		index = (i + q->Head) % q->MaxItem;
		printk("Queue[%d].ItemSize = %d  ", index, q->ItemArray[index].ItemSize);
		for(j=0; j<q->ItemArray[index].ItemSize; j++)
			printk(" %x", q->ItemArray[index].Item[j]);
		printk("\n");
	}
	printk("------------------------------------------------/\n");
}


char *DOT11_ErrMsgQueue(int err)
{
	switch(err)
	{
		case E_DOT11_2LARGE:
			return E_MSG_DOT11_2LARGE;
		case E_DOT11_QFULL:
			return E_MSG_DOT11_QFULL;
		case E_DOT11_QEMPTY:
			return E_MSG_DOT11_QEMPTY;
		default:
			return "E_MSG_DOT11_QNOERR";
	}
}


void DOT11_Dump(char *fun, UINT8 *buf, int size, char *comment)
{
	int i;

	printk("$$ %s $$: %s", fun, comment);
	if (buf != NULL) {
		printk("\nMessage is %d bytes %x hex", size, size);
		for (i = 0; i < size; i++) {
			if (i % 16 == 0) printk("\n\t");
			printk("%2x ", *(buf+i));
		}
	}
	printk("\n");
}
#endif


/*------------------------------------------------------------
  Set RSNIE is to set Information Element of AP
  Return Value
Success : 0
------------------------------------------------------------*/
static int DOT11_Process_Set_RSNIE(struct net_device *dev, struct iw_point *data)
{
	struct Dot11RsnIE	*pdot11RsnIE;
	struct wifi_mib		*pmib;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);


#ifdef HS2_SUPPORT
	unsigned char       rsnie_hdr_OSEN[4]={0x50, 0x6F, 0x9A, 0x12};
#endif

	DOT11_SET_RSNIE		*Set_RSNIE = (DOT11_SET_RSNIE *)data->pointer;

	DEBUG_INFO("going to set rsnie\n");
	pmib = priv->pmib;
	pdot11RsnIE = &pmib->dot11RsnIE;
	if(Set_RSNIE->Flag == DOT11_Ioctl_Set)
	{
		pdot11RsnIE->rsnielen = Set_RSNIE->RSNIELen;
		memcpy((void *)pdot11RsnIE->rsnie, Set_RSNIE->RSNIE, pdot11RsnIE->rsnielen);
		priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = 1;
		DEBUG_INFO("DOT11_Process_Set_RSNIE rsnielen=%d\n", pdot11RsnIE->rsnielen);
		#ifdef HS2_SUPPORT
        if ((*(unsigned char *)pdot11RsnIE->rsnie == _RSN_IE_1_) && (pdot11RsnIE->rsnielen >= 4) && (!memcmp((void *)(pdot11RsnIE->rsnie + 2), (void *)rsnie_hdr_OSEN, 4))) {
				HS2DEBUG("found OSEN IE in Assoc Req, set dgaf_disable = 1\n");
                priv->dgaf_disable = 1;	// under OSEN mode , all broadcast frame will transfor to unicast
        }
		#endif        
		// see whether if driver is open. If not, do not enable tx/rx, david
		if (!netif_running(priv->dev)) {
#if defined(__OSK__) && defined(UNIVERSAL_REPEATER)
			if (IS_VXD_INTERFACE(priv) && pdot11RsnIE->rsnielen > 0)
				priv->pmib->dot11OperationEntry.keep_rsnie = 1;
#endif
			return 0;
		}

		// Alway enable tx/rx in rtl8190_init_hw_PCI()
		//RTL_W8(_CR_, BIT(2) | BIT(3));
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
		if (!priv->pmib->dot1180211AuthEntry.dot11EnablePSK)
#endif
		{
			if (OPMODE & WIFI_AP_STATE) {
#ifdef MBSSID
				if (IS_ROOT_INTERFACE(priv))
#endif
				{
					if (priv->auto_channel) {
						priv->ss_ssidlen = 0;
						DEBUG_INFO("start_clnt_ss, trigger by %s, ss_ssidlen=0\n", (char *)__FUNCTION__);
						start_clnt_ss(priv);
					}
				}
			}
//for openwrt 8021x donot trigger site survey when rsnie init in init sw			
#if defined(CLIENT_MODE) && !defined(RTK_NL80211) && !defined(WIFI_WPAS_CLI)
			if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE))
				start_clnt_lookup(priv, RESCAN);
#endif
		}
	}
	else if(Set_RSNIE->Flag == DOT11_Ioctl_Query)
	{
	}
	return 0;
}


#if 0
char *DOT11_Parse_RSNIE_Err(int err)
{
	switch(err)
	{
		case ERROR_BUFFER_TOO_SMALL:
			return RSN_STRERROR_BUFFER_TOO_SMALL;
		case ERROR_INVALID_PARA:
			return RSN_STRERROR_INVALID_PARAMETER;
		case ERROR_INVALID_RSNIE:
			return RSN_STRERROR_INVALID_RSNIE;
		case ERROR_INVALID_MULTICASTCIPHER:
			return RSN_STRERROR_INVALID_MULTICASTCIPHER;
		case ERROR_INVALID_UNICASTCIPHER:
			return RSN_STRERROR_INVALID_UNICASTCIPHER;
		case ERROR_INVALID_AUTHKEYMANAGE:
			return RSN_STRERROR_INVALID_AUTHKEYMANAGE;
		case ERROR_UNSUPPORTED_RSNEVERSION:
			return RSN_STRERROR_UNSUPPORTED_RSNEVERSION;
		case ERROR_INVALID_CAPABILITIES:
			return RSN_STRERROR_INVALID_CAPABILITIES;
		default:
			return "Uknown Failure";
	}
}
#endif


/*-------------------------------------------------
  Send association response to STA
  Return Value
Success : 0
---------------------------------------------------*/
static int DOT11_Process_Association_Rsp(struct net_device *dev, struct iw_point *data, int frame_type)
{
#ifndef SMP_SYNC
	unsigned long		flags;
#endif
	struct stat_info	*pstat;
	unsigned char		*hwaddr;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	DOT11_ASSOCIATIIN_RSP	*Assoc_Rsp = (DOT11_ASSOCIATIIN_RSP *)data->pointer;

	pstat = get_stainfo(priv, (UINT8 *)Assoc_Rsp->MACAddr);

	if (pstat == NULL)
		return (-1);

	hwaddr = (unsigned char *)Assoc_Rsp->MACAddr;
	DEBUG_INFO("802.1x issue assoc_rsp sta:%02X%02X%02X%02X%02X%02X status:%d\n",
			hwaddr[0],hwaddr[1],hwaddr[2],hwaddr[3],hwaddr[4],hwaddr[5], Assoc_Rsp->Status);

	issue_asocrsp(priv, Assoc_Rsp->Status, pstat, frame_type);

	if (Assoc_Rsp->Status != 0)
	{
		SAVE_INT_AND_CLI(flags);
		if (asoc_list_del(priv, pstat))
		{
			if (pstat->expire_to > 0)
			{
				cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
				check_sta_characteristic(priv, pstat, DECREASE);
				LOG_MSG("A STA is rejected by 802.1x daemon - %02X:%02X:%02X:%02X:%02X:%02X\n",
						pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
			}
		}

		free_stainfo(priv, pstat);
		RESTORE_INT(flags);
	}
	else
		update_fwtbl_asoclst(priv, pstat);

	return 0;
}


/*-------------------------------------------------
  Send Disassociation request to STA
  Return Value
Success : 0
---------------------------------------------------*/
#ifdef CONFIG_PCI_HCI
static inline
#endif
int __DOT11_Process_Disconnect_Req(struct net_device *dev, struct iw_point *data)
{
#ifndef SMP_SYNC
	unsigned long	flags;
#endif
	struct stat_info	*pstat;
	unsigned char	*hwaddr;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	DOT11_DISCONNECT_REQ	*Disconnect_Req = (DOT11_DISCONNECT_REQ *)data->pointer;

	pstat = get_stainfo(priv, (UINT8 *)Disconnect_Req->MACAddr);

	if (pstat == NULL)
		return (-1);

	hwaddr = (unsigned char *)Disconnect_Req->MACAddr;

#ifdef WIFI_SIMPLE_CONFIG
	if (wsc_disconn_list_check(priv, hwaddr)) {
		DEBUG_INFO("Ignore issue disassoc sta:%02X%02X%02X%02X%02X%02X due to STA had reconnected\n",
				hwaddr[0],hwaddr[1],hwaddr[2],hwaddr[3],hwaddr[4],hwaddr[5]);
		return 0;
	}
#endif
	
	DEBUG_INFO("802.1x issue disassoc sta:%02X%02X%02X%02X%02X%02X reason:%d\n",
			hwaddr[0],hwaddr[1],hwaddr[2],hwaddr[3],hwaddr[4],hwaddr[5], Disconnect_Req->Reason);

#ifdef CLIENT_MODE
	if(OPMODE & WIFI_STATION_STATE){

		if((OPMODE&(  WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE))
				==(  WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE))
		{
			issue_deauth(priv, BSSID, _RSON_DEAUTH_STA_LEAVING_);					
			OPMODE_VAL(OPMODE & ~(WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE));
#ifndef WIFI_WPAS_CLI
#ifdef SMART_REPEATER_MODE
			if(GET_MIB(priv)->ap_profile.enable_profile && GET_MIB(priv)->ap_profile.profile_num > 0)			 
			{
				if((priv->site_survey->count_target > 0) && ((priv->join_index+1) > priv->site_survey->count_target)){
					start_clnt_lookup(priv, RESCAN);		
				}else{
					start_clnt_lookup(priv, DONTRESCAN);		
				}
			} else
#endif
			{
				start_clnt_lookup(priv, RESCAN);
			}
#endif // !WIFI_WPAS_CLI
			//SME_DEBUG("!!issue disconnect at wlan!!\n");
		}else{
			return (-1);
		} 

	}else
#endif
	{
		issue_deauth(priv, pstat->hwaddr, Disconnect_Req->Reason);
	}

	SAVE_INT_AND_CLI(flags);
	if (asoc_list_del(priv, pstat))
	{
		if (pstat->expire_to > 0)
		{
			cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
			check_sta_characteristic(priv, pstat, DECREASE);
			LOG_MSG("A STA is rejected by 802.1x daemon - %02X:%02X:%02X:%02X:%02X:%02X\n",
					pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
		}
	}

	free_stainfo(priv, pstat);
	RESTORE_INT(flags);

	return 0;
}

int DOT11_Process_Disconnect_Req(struct net_device *dev, struct iw_point *data)
{
#if defined(CONFIG_PCI_HCI)
	return __DOT11_Process_Disconnect_Req(dev, data);
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	notify_disconnect_sta(dev, (DOT11_DISCONNECT_REQ *)data->pointer);
	return 0;
#endif
}

/*---------------------------------------------------------------------
  Delete Group Key for AP and Pairwise Key for specific STA
  Return Value
Success : 0
-----------------------------------------------------------------------*/
int DOT11_Process_Delete_Key(struct net_device *dev, struct iw_point *data)
{
	struct stat_info	*pstat = NULL;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	DOT11_DELETE_KEY	*Delete_Key = (DOT11_DELETE_KEY *)data->pointer;
	struct wifi_mib 	*pmib = priv->pmib;

	unsigned char MULTICAST_ADD[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

	if (Delete_Key->KeyType == DOT11_KeyType_Group)
	{
		pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen = 0;
		pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKeyLen = 0;

		DEBUG_INFO("Delete Group Key\n");
		if (CamDeleteOneEntry(priv, MULTICAST_ADD, 1, 0))
			priv->pshare->CamEntryOccupied--;
#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
		if (CamDeleteOneEntry(priv, MULTICAST_ADD, 1, 0))
			priv->pshare->CamEntryOccupied--;
#endif

	}
	else if (Delete_Key->KeyType == DOT11_KeyType_Pairwise)
	{
		pstat = get_stainfo(priv, (UINT8 *)Delete_Key->MACAddr);
		if (pstat == NULL)
			return (-1);

		pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKeyLen = 0;
		pstat->dot11KeyMapping.dot11EncryptKey.dot11TMicKeyLen = 0;

		DEBUG_INFO("Delete Unicast Key\n");
		if (pstat->dot11KeyMapping.keyInCam == TRUE) {
			if (CamDeleteOneEntry(priv, (unsigned char *)Delete_Key->MACAddr, 0, 0)) {
				priv->pshare->CamEntryOccupied--;
				if (pstat)	pstat->dot11KeyMapping.keyInCam = FALSE;
			}
#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
			if (CamDeleteOneEntry(priv, (unsigned char *)Delete_Key->MACAddr, 0, 0)) {
				priv->pshare->CamEntryOccupied--;
				if (pstat)	pstat->dot11KeyMapping.keyInCam = FALSE;
			}
#endif
		}
	}

	return 0;
}


static __inline__ void set_ttkeylen(struct Dot11EncryptKey *pEncryptKey, UINT8 len)
{
	pEncryptKey->dot11TTKeyLen = len;
}


static __inline__ void set_tmickeylen(struct Dot11EncryptKey *pEncryptKey, UINT8 len)
{
	pEncryptKey->dot11TMicKeyLen = len;
}


static __inline__ void set_tkip_key(struct Dot11EncryptKey *pEncryptKey, UINT8 *src)
{
	memcpy(pEncryptKey->dot11TTKey.skey, src, pEncryptKey->dot11TTKeyLen);

	memcpy(pEncryptKey->dot11TMicKey1.skey, src + 16, pEncryptKey->dot11TMicKeyLen);

	memcpy(pEncryptKey->dot11TMicKey2.skey, src + 24, pEncryptKey->dot11TMicKeyLen);
}


static __inline__ void set_aes_key(struct Dot11EncryptKey *pEncryptKey, UINT8 *src)
{
	memcpy(pEncryptKey->dot11TTKey.skey, src, pEncryptKey->dot11TTKeyLen);

	memcpy(pEncryptKey->dot11TMicKey1.skey, src, pEncryptKey->dot11TMicKeyLen);
}


static __inline__ void set_wep40_key(struct Dot11EncryptKey *pEncryptKey, UINT8 *src)
{
	memcpy(pEncryptKey->dot11TTKey.skey, src, pEncryptKey->dot11TTKeyLen);
}


static __inline__ void set_wep104_key(struct Dot11EncryptKey *pEncryptKey, UINT8 *src)
{
	memcpy(pEncryptKey->dot11TTKey.skey, src, pEncryptKey->dot11TTKeyLen);
}


static __inline__ void clean_pn(unsigned char *pn)
{
	memset(pn, 0, 6);
}


/*---------------------------------------------------------------------
  Set Group Key for AP and Pairwise Key for specific STA
  Return Value
Success : 0
-----------------------------------------------------------------------*/
int DOT11_Process_Set_Key(struct net_device *dev, struct iw_point *data,
		DOT11_SET_KEY *pSetKey, unsigned char *pKey)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	struct wifi_mib	*pmib = priv->pmib;
	struct Dot11EncryptKey	*pEncryptKey = NULL;
	struct stat_info	*pstat = NULL;
	DOT11_SET_KEY	Set_Key;
	unsigned char	key_combo[32];
	unsigned char	MULTICAST_ADD[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	int	retVal;

	if (data) {
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
		memcpy((void *)&Set_Key, (void *)data->pointer, sizeof(DOT11_SET_KEY));
		memcpy((void *)key_combo,
				((DOT11_SET_KEY *)data->pointer)->KeyMaterial, 32);
#else
		if (copy_from_user((void *)&Set_Key, (void *)data->pointer, sizeof(DOT11_SET_KEY))) {
			DEBUG_ERR("copy_from_user error!\n");
			return -1;
		}
		if (copy_from_user((void *)key_combo,
					((DOT11_SET_KEY *)data->pointer)->KeyMaterial, 32)) {
			DEBUG_ERR("copy_from_user error!\n");
			return -1;
		}
#endif
	}
	else {
		Set_Key = *pSetKey;
		memcpy(key_combo, pKey, 32);
	}

#ifdef WDS
#ifdef LAZY_WDS
	if ((priv->pmib->dot11WdsInfo.wdsEnabled == WDS_LAZY_ENABLE ||
				(priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsNum)) &&
#else
			if (priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsNum &&
#endif		
				((priv->pmib->dot11WdsInfo.wdsPrivacy == _TKIP_PRIVACY_) ||
				 (priv->pmib->dot11WdsInfo.wdsPrivacy == _CCMP_PRIVACY_)) &&
				(Set_Key.KeyType == DOT11_KeyType_Group) &&
				!memcmp(Set_Key.MACAddr, NULL_MAC_ADDR, 6)) {
			int i;

			for (i=0; i<32; i++)
			sprintf((char *)&priv->pmib->dot11WdsInfo.wdsPskPassPhrase[i*2], "%02x", key_combo[i]);			
			priv->pmib->dot11WdsInfo.wdsPskPassPhrase[i*2] = '\0';

			for (i=0; i<NUM_WDS; i++) {
#ifdef LAZY_WDS
			if (priv->pmib->dot11WdsInfo.wdsEnabled == WDS_LAZY_ENABLE) {
			if (!memcmp(priv->pmib->dot11WdsInfo.entry[i].macAddr,
					NULL_MAC_ADDR, 6)) 
			continue;			
			}
			else
#endif				
			{
				if (i+1 > pmib->dot11WdsInfo.wdsNum)
					break;
			}	
			memcpy(Set_Key.MACAddr, priv->pmib->dot11WdsInfo.entry[i].macAddr, 6);
			Set_Key.KeyType = DOT11_KeyType_Pairwise;
			Set_Key.EncType = priv->pmib->dot11WdsInfo.wdsPrivacy;
			Set_Key.KeyIndex = 0;
			DOT11_Process_Set_Key(priv->dev, NULL, &Set_Key, key_combo);
			}
			return 0;
			}
#endif

	if(Set_Key.KeyType == DOT11_KeyType_Group)
	{
#ifndef __OSK__
		int set_gkey_to_cam = 0;
#else
		int set_gkey_to_cam = 1;
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8188E)
			set_gkey_to_cam = 1;
#endif

#ifdef UNIVERSAL_REPEATER
		if (IS_VXD_INTERFACE(priv))
			set_gkey_to_cam = 0;
		else {
			if (IS_ROOT_INTERFACE(priv)) {
				if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
					set_gkey_to_cam = 0;
			}
		}
#endif

#ifdef MBSSID
#ifdef HW_ENC_FOR_GROUP_CIPHER
		set_gkey_to_cam = 1;
#else
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
		{
			// No matter root or vap, don't set key to cam if vap is enabled.
			set_gkey_to_cam = 0;
		}
#endif
#endif
		if(Set_Key.KeyIndex == GKEY_ID_SECOND) 
			pEncryptKey = &pmib->dot11GroupKeysTable.dot11EncryptKey2;
		else
			pEncryptKey = &pmib->dot11GroupKeysTable.dot11EncryptKey;
			
		pmib->dot11GroupKeysTable.dot11Privacy = Set_Key.EncType;
		pmib->dot11GroupKeysTable.keyid = (UINT)Set_Key.KeyIndex;


		switch(Set_Key.EncType)
		{
			case DOT11_ENC_TKIP:
				set_ttkeylen(pEncryptKey, 16);
				set_tmickeylen(pEncryptKey, 8);
				set_tkip_key(pEncryptKey, key_combo);

				DEBUG_INFO("going to set TKIP group key! id %X\n", (UINT)Set_Key.KeyIndex);
				if (!SWCRYPTO) {
					if (set_gkey_to_cam)
					{
						retVal = CamDeleteOneEntry(priv, MULTICAST_ADD, Set_Key.KeyIndex, 0);
						if (retVal) {
							priv->pshare->CamEntryOccupied--;
							pmib->dot11GroupKeysTable.keyInCam = FALSE;
						}
						retVal = CamAddOneEntry(priv, MULTICAST_ADD, Set_Key.KeyIndex, DOT11_ENC_TKIP<<2, 0, key_combo);
						if (retVal) {
							priv->pshare->CamEntryOccupied++;
							pmib->dot11GroupKeysTable.keyInCam = TRUE;
						}
					}
				}
				break;

			case DOT11_ENC_WEP40:
				set_ttkeylen(pEncryptKey, 5);
				set_tmickeylen(pEncryptKey, 0);
				set_wep40_key(pEncryptKey, key_combo);

				DEBUG_INFO("going to set WEP40 group key!\n");
				if (!SWCRYPTO) {
					if (set_gkey_to_cam)
					{
						retVal = CamDeleteOneEntry(priv, MULTICAST_ADD, Set_Key.KeyIndex, 0);
						if (retVal) {
							priv->pshare->CamEntryOccupied--;
							pmib->dot11GroupKeysTable.keyInCam = FALSE;
						}
						retVal = CamAddOneEntry(priv, MULTICAST_ADD, Set_Key.KeyIndex, DOT11_ENC_WEP40<<2, 0, key_combo);
						if (retVal) {
							priv->pshare->CamEntryOccupied++;
							pmib->dot11GroupKeysTable.keyInCam = TRUE;
						}
					}
				}
				break;

			case DOT11_ENC_WEP104:
				set_ttkeylen(pEncryptKey, 13);
				set_tmickeylen(pEncryptKey, 0);
				set_wep104_key(pEncryptKey, key_combo);

				DEBUG_INFO("going to set WEP104 group key!\n");
				if (!SWCRYPTO) {
					if (set_gkey_to_cam)
					{
						retVal = CamDeleteOneEntry(priv, MULTICAST_ADD, Set_Key.KeyIndex, 0);
						if (retVal) {
							priv->pshare->CamEntryOccupied--;
							pmib->dot11GroupKeysTable.keyInCam = FALSE;
						}
						retVal = CamAddOneEntry(priv, MULTICAST_ADD, Set_Key.KeyIndex, DOT11_ENC_WEP104<<2, 0, key_combo);
						if (retVal) {
							priv->pshare->CamEntryOccupied++;
							pmib->dot11GroupKeysTable.keyInCam = TRUE;
						}
					}
				}
				break;

			case DOT11_ENC_CCMP:
				set_ttkeylen(pEncryptKey, 16);
				set_tmickeylen(pEncryptKey, 16);
				set_aes_key(pEncryptKey, key_combo);

				pEncryptKey->dot11RXPN48.val48 = 0;
				
				DEBUG_INFO("going to set CCMP-AES group key!\n");

					if (!SWCRYPTO) {
						if (set_gkey_to_cam)
						{
							retVal = CamDeleteOneEntry(priv, MULTICAST_ADD, Set_Key.KeyIndex, 0);
							if (retVal) {
								priv->pshare->CamEntryOccupied--;
								pmib->dot11GroupKeysTable.keyInCam = FALSE;
							}
							retVal = CamAddOneEntry(priv, MULTICAST_ADD, Set_Key.KeyIndex, DOT11_ENC_CCMP<<2, 0, key_combo);
							if (retVal) {
								priv->pshare->CamEntryOccupied++;
								pmib->dot11GroupKeysTable.keyInCam = TRUE;
							}
						}
					}
				break;

			case DOT11_ENC_NONE:
			default:
				DEBUG_ERR("No group encryption key is set!\n");
				set_ttkeylen(pEncryptKey, 0);
				set_tmickeylen(pEncryptKey, 0);
				break;
		}
	}
#ifdef CONFIG_IEEE80211W
		else if(Set_Key.KeyType == DOT11_KeyType_IGTK)		
		{	
			int set_gkey_to_cam = 0;
			
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8188E)
				set_gkey_to_cam = 1;
#endif //CONFIG_RTL_88E_SUPPORT
	
#ifdef MBSSID
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
			{
				// No matter root or vap, don't set key to cam if vap is enabled.
				set_gkey_to_cam = 0;
			}
#endif
	
			pmib->dot11IGTKTable.dot11Privacy = Set_Key.EncType;
			pEncryptKey = &pmib->dot11IGTKTable.dot11EncryptKey;
			pmib->dot11IGTKTable.keyid = (UINT)Set_Key.KeyIndex;
	
			set_ttkeylen(pEncryptKey, 16);
			set_tmickeylen(pEncryptKey, 16);
			set_aes_key(pEncryptKey, key_combo);
			
		}
#endif // CONFIG_IEEE80211W
	else if(Set_Key.KeyType == DOT11_KeyType_Pairwise)
	{
#ifdef WDS
		// always to store WDS key for tx-hangup re-init
#if 0
		// if driver is not opened, save the WDS key into mib. The key value will
		// be updated into CAM when driver opened. david
		if ( !IS_DRV_OPEN(priv)) { // not open or not in open
			// if interface is not open and STA is a WDS entry, save the key
			// to mib and restore it to STA table when driver is open, david
			struct net_device *pNet = getWdsDevByAddr(priv, Set_Key.MACAddr);
			if (pNet) {
				int widx = getWdsIdxByDev(priv, pNet);
				if (widx >= 0) {
					if (Set_Key.EncType == _WEP_40_PRIVACY_ ||
							Set_Key.EncType == _WEP_104_PRIVACY_)
						memcpy(pmib->dot11WdsInfo.wdsWepKey, key_combo, 32);
					else {
						memcpy(pmib->dot11WdsInfo.wdsMapingKey[widx], key_combo, 32);
						pmib->dot11WdsInfo.wdsMappingKeyLen[widx] = 32;
					}

					pmib->dot11WdsInfo.wdsKeyId = Set_Key.KeyIndex;
					pmib->dot11WdsInfo.wdsPrivacy = Set_Key.EncType;

					return 0;
				}
			}
			return -1;
		}
#endif
		int widx = -1;
		struct net_device *pNet = getWdsDevByAddr(priv, Set_Key.MACAddr);
		if (pNet) {
			widx = getWdsIdxByDev(priv, pNet);
			if (widx >= 0) {
				if (Set_Key.EncType == _WEP_40_PRIVACY_ ||
						Set_Key.EncType == _WEP_104_PRIVACY_)
					memcpy(pmib->dot11WdsInfo.wdsWepKey, key_combo, 32);
				else {
					memcpy(pmib->dot11WdsInfo.wdsMapingKey[widx], key_combo, 32);
					pmib->dot11WdsInfo.wdsMappingKeyLen[widx] = 32;
				}
				pmib->dot11WdsInfo.wdsKeyId = Set_Key.KeyIndex;
				pmib->dot11WdsInfo.wdsPrivacy = Set_Key.EncType;
			}
		}
		if ( !IS_DRV_OPEN(priv)) {
			if (widx > 0 && pmib->dot11WdsInfo.wdsMappingKeyLen[widx] > 0)
				pmib->dot11WdsInfo.wdsMappingKeyLen[widx] |= 0x80000000;
			return 0;
		}
		//----------------------------------- david+2006-06-30
#endif // WDS

#ifdef MULTI_MAC_CLONE
		if ((OPMODE & WIFI_STATION_STATE) && priv->pmib->ethBrExtInfo.macclone_enable) {			
			ACTIVE_ID = mclone_find_address(priv, Set_Key.MACAddr, NULL, MAC_CLONE_NOCARE_FIND);	
			if (ACTIVE_ID < 0) {
				DEBUG_ERR("Invalid mac address!\n");
				return 0;
			}				
			pstat = get_stainfo(priv, priv->pmib->dot11Bss.bssid);			
		}
		else
#endif		
		pstat = get_stainfo(priv, Set_Key.MACAddr);
		if (pstat == NULL) {
			DEBUG_ERR("Set key failed, invalid mac address: %02x%02x%02x%02x%02x%02x\n",
					Set_Key.MACAddr[0], Set_Key.MACAddr[1], Set_Key.MACAddr[2], Set_Key.MACAddr[3],
					Set_Key.MACAddr[4], Set_Key.MACAddr[5]);
			return (-1);
		}

		pstat->dot11KeyMapping.dot11Privacy = Set_Key.EncType;
		pEncryptKey = &pstat->dot11KeyMapping.dot11EncryptKey;
		pstat->keyid = Set_Key.KeyIndex;

#if defined(__DRAYTEK_OS__) && defined(WDS)
		if (pstat->state & WIFI_WDS)
			priv->pmib->dot11WdsInfo.wdsPrivacy = Set_Key.EncType;
#endif

		switch(Set_Key.EncType)
		{
			case DOT11_ENC_TKIP:
				set_ttkeylen(pEncryptKey, 16);
				set_tmickeylen(pEncryptKey, 8);
				set_tkip_key(pEncryptKey, key_combo);

				DEBUG_INFO("going to set TKIP Unicast key for sta %02X%02X%02X%02X%02X%02X, id=%d\n",
						Set_Key.MACAddr[0], Set_Key.MACAddr[1], Set_Key.MACAddr[2],
						Set_Key.MACAddr[3], Set_Key.MACAddr[4], Set_Key.MACAddr[5], pstat->keyid);
				if (!SWCRYPTO) {
					retVal = CamDeleteOneEntry(priv, Set_Key.MACAddr, Set_Key.KeyIndex, 0);
					if (retVal) {
						priv->pshare->CamEntryOccupied--;
						if (pstat)	pstat->dot11KeyMapping.keyInCam = FALSE;
					}
					retVal = CamAddOneEntry(priv, Set_Key.MACAddr, Set_Key.KeyIndex, DOT11_ENC_TKIP<<2, 0, key_combo);
					if (retVal) {
						priv->pshare->CamEntryOccupied++;
						if (pstat)	pstat->dot11KeyMapping.keyInCam = TRUE;
					}
					else {
						if (pstat->aggre_mthd != AGGRE_MTHD_NONE)
							pstat->aggre_mthd = AGGRE_MTHD_NONE;
					}
				}
				break;

			case DOT11_ENC_WEP40:
				set_ttkeylen(pEncryptKey, 5);
				set_tmickeylen(pEncryptKey, 0);
				set_wep40_key(pEncryptKey, key_combo);

				DEBUG_INFO("going to set WEP40 unicast key for sta %02X%02X%02X%02X%02X%02X, id=%d\n",
						Set_Key.MACAddr[0], Set_Key.MACAddr[1], Set_Key.MACAddr[2],
						Set_Key.MACAddr[3], Set_Key.MACAddr[4], Set_Key.MACAddr[5], pstat->keyid);
				if (!SWCRYPTO) {
					retVal = CamDeleteOneEntry(priv, Set_Key.MACAddr, Set_Key.KeyIndex, 0);
					if (retVal) {
						priv->pshare->CamEntryOccupied--;
						if (pstat)	pstat->dot11KeyMapping.keyInCam = FALSE;
					}
					retVal = CamAddOneEntry(priv, Set_Key.MACAddr, Set_Key.KeyIndex, DOT11_ENC_WEP40<<2, 0, key_combo);
					if (retVal) {
						priv->pshare->CamEntryOccupied++;
						if (pstat)	pstat->dot11KeyMapping.keyInCam = TRUE;
					}
					else {
						if (pstat->aggre_mthd != AGGRE_MTHD_NONE)
							pstat->aggre_mthd = AGGRE_MTHD_NONE;
					}
				}
				break;

			case DOT11_ENC_WEP104:
				set_ttkeylen(pEncryptKey, 13);
				set_tmickeylen(pEncryptKey, 0);
				set_wep104_key(pEncryptKey, key_combo);

				DEBUG_INFO("going to set WEP104 unicast key for sta %02X%02X%02X%02X%02X%02X, id=%d\n",
						Set_Key.MACAddr[0], Set_Key.MACAddr[1], Set_Key.MACAddr[2],
						Set_Key.MACAddr[3], Set_Key.MACAddr[4], Set_Key.MACAddr[5], pstat->keyid);
				if (!SWCRYPTO) {
					retVal = CamDeleteOneEntry(priv, Set_Key.MACAddr, Set_Key.KeyIndex, 0);
					if (retVal) {
						priv->pshare->CamEntryOccupied--;
						if (pstat)	pstat->dot11KeyMapping.keyInCam = FALSE;
					}
					retVal = CamAddOneEntry(priv, Set_Key.MACAddr, Set_Key.KeyIndex, DOT11_ENC_WEP104<<2, 0, key_combo);
					if (retVal) {
						priv->pshare->CamEntryOccupied++;
						if (pstat)	pstat->dot11KeyMapping.keyInCam = TRUE;
					}
					else {
						if (pstat->aggre_mthd != AGGRE_MTHD_NONE)
							pstat->aggre_mthd = AGGRE_MTHD_NONE;
					}
				}
				break;

			case DOT11_ENC_CCMP:
				set_ttkeylen(pEncryptKey, 16);
				set_tmickeylen(pEncryptKey, 16);
				set_aes_key(pEncryptKey, key_combo);

				DEBUG_INFO("going to set CCMP-AES unicast key for sta %02X%02X%02X%02X%02X%02X, id=%d\n",
						Set_Key.MACAddr[0], Set_Key.MACAddr[1], Set_Key.MACAddr[2],
						Set_Key.MACAddr[3], Set_Key.MACAddr[4], Set_Key.MACAddr[5], pstat->keyid);
				if (!SWCRYPTO) {
					retVal = CamDeleteOneEntry(priv, Set_Key.MACAddr, Set_Key.KeyIndex, 0);
					if (retVal) {
						priv->pshare->CamEntryOccupied--;
						if (pstat)	pstat->dot11KeyMapping.keyInCam = FALSE;
					}
					retVal = CamAddOneEntry(priv, Set_Key.MACAddr, Set_Key.KeyIndex, DOT11_ENC_CCMP<<2, 0, key_combo);
					if (retVal) {
						priv->pshare->CamEntryOccupied++;
						if (pstat)	pstat->dot11KeyMapping.keyInCam = TRUE;
						assign_aggre_mthod(priv, pstat);
					}
					else {
						if (pstat->aggre_mthd != AGGRE_MTHD_NONE)
							pstat->aggre_mthd = AGGRE_MTHD_NONE;
					}
				}
				break;

			case DOT11_ENC_NONE:
			default:
				DEBUG_ERR("No pairewise encryption key is set!\n");
				set_ttkeylen(pEncryptKey, 0);
				set_tmickeylen(pEncryptKey, 0);
				break;
		}
	}

	return 0;
}


/*---------------------------------------------------------------------
  Set Port Enable of Disable for STA
  Return Value
Success : 0
-----------------------------------------------------------------------*/
static int DOT11_Process_Set_Port(struct net_device *dev, struct iw_point *data)
{
	struct stat_info	*pstat;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	struct wifi_mib		*pmib = priv->pmib;
	DOT11_SET_PORT		*Set_Port = (DOT11_SET_PORT *)data->pointer;

	DEBUG_INFO("Set_Port sta %02X%02X%02X%02X%02X%02X Status %X\n",
			Set_Port->MACAddr[0],Set_Port->MACAddr[1],Set_Port->MACAddr[2],
			Set_Port->MACAddr[3],Set_Port->MACAddr[4],Set_Port->MACAddr[5],
			Set_Port->PortStatus);

	// if driver is not opened, return immediately, david
	if (!netif_running(priv->dev))
		return (-1);

	pstat = get_stainfo(priv, Set_Port->MACAddr);

	if ((pstat == NULL) || (!(pstat->state & WIFI_ASOC_STATE)))
		return (-1);

	if (Set_Port->PortStatus)
	{
		pstat->ieee8021x_ctrlport = Set_Port->PortStatus;
#ifdef INDICATE_LINK_CHANGE
		indicate_sta_link_change(priv, pstat, INCREASE, __FUNCTION__);
#endif
	}
	else
		pstat->ieee8021x_ctrlport = pmib->dot118021xAuthEntry.dot118021xDefaultPort;

#ifdef CLIENT_MODE
	if (OPMODE & WIFI_STATION_STATE) {
		priv->is_4Way_finished = pstat->ieee8021x_ctrlport;
	}
#endif

#if defined(CONFIG_RTL8672) && !defined(__OSK__)
	wlan_sta_state_notify(priv, STATE_Sta_Bss);
#endif

#ifdef P2P_SUPPORT
/*cfg p2p cfg p2p
	if((OPMODE&WIFI_P2P_SUPPORT)&&( P2PMODE ==P2P_CLIENT)){
*/
	if((rtk_p2p_is_enabled(priv)==PROPERTY_P2P)&&  rtk_p2p_chk_role(priv,P2P_CLIENT)){
		/*to indicate web server that data path is connected done(can start issue udhcpc daemon)*/
		/*cfg p2p cfg p2p
		P2P_STATE = P2P_S_CLIENT_CONNECTED_DHCPC;*/
		rtk_p2p_set_state(priv,P2P_S_CLIENT_CONNECTED_DHCPC);
#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
		/*it will trigger application to start DHCPC !!*/
		if (priv->p2p_event_indicate_cb_func)
			priv->p2p_event_indicate_cb_func("wlan0", WIFI_P2P_EVENT_START_DHCPC);
#endif        
		priv->p2pPtr->clientmode_try_connect = 0;
		priv->p2pPtr->clientmode_connected = 0;
		P2P_DEBUG("Set_Port Sta[%02X%02X%02X%02X%02X%02X],Status=%X\n\n\n",
			Set_Port->MACAddr[0],Set_Port->MACAddr[1],Set_Port->MACAddr[2],
			Set_Port->MACAddr[3],Set_Port->MACAddr[4],Set_Port->MACAddr[5],
			Set_Port->PortStatus);
	}
#endif
	return 0;
}


static int DOT11_Process_QueryRSC(struct net_device *dev, struct iw_point *data)
{
	DOT11_GKEY_TSC		*pGkey_TSC = (DOT11_GKEY_TSC *)data->pointer;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	struct wifi_mib		*pmib = priv->pmib;

	pGkey_TSC->EventId = DOT11_EVENT_GKEY_TSC;
	pGkey_TSC->IsMoreEvent = FALSE;

	// Fix bug of getting RSC for auth (should consider endian issue)
	//memcpy((void *)pGkey_TSC->KeyTSC,
	//	(void *)&(pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48.val48), 6);
	pGkey_TSC->KeyTSC[0] = pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC0;
	pGkey_TSC->KeyTSC[1] = pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC1;
	pGkey_TSC->KeyTSC[2] = pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC2;
	pGkey_TSC->KeyTSC[3] = pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC3;
	pGkey_TSC->KeyTSC[4] = pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC4;
	pGkey_TSC->KeyTSC[5] = pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48._byte_.TSC5;
	pGkey_TSC->KeyTSC[6] = 0x00;
	pGkey_TSC->KeyTSC[7] = 0x00;

	return 0;
}


static int DOT11_Porcess_EAPOL_MICReport(struct net_device *dev, struct iw_point *data)
{
#ifdef _DEBUG_RTL8192CD_
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
#endif
	struct stat_info *pstat = NULL;
#ifdef _SINUX_
	DOT11_MIC_FAILURE *MIC_Failure = (DOT11_MIC_FAILURE *)data->pointer;

	printk("MIC fail report from 802.1x daemon\n");
	printk("mac: %02x%02x%02x%02x%02x%02x\n", MIC_Failure->MACAddr[0], MIC_Failure->MACAddr[1],
			MIC_Failure->MACAddr[2], MIC_Failure->MACAddr[3], MIC_Failure->MACAddr[4], MIC_Failure->MACAddr[5]);
	mic_error_report(1);
#else	
	//DOT11_MIC_FAILURE *MIC_Failure = (DOT11_MIC_FAILURE *)data->pointer;

	DEBUG_INFO("MIC fail report from 802.1x daemon\n");
	//DEBUG_INFO("mac: %02x%02x%02x%02x%02x%02x\n", MIC_Failure->MACAddr[0], MIC_Failure->MACAddr[1],
	//MIC_Failure->MACAddr[2], MIC_Failure->MACAddr[3], MIC_Failure->MACAddr[4], MIC_Failure->MACAddr[5]);
#endif	
#ifdef RTL_WPA2
	PRINT_INFO("%s: DOT11_Indicate_MIC_Failure \n", (char *)__FUNCTION__);
#endif

	DOT11_Indicate_MIC_Failure(dev, pstat);

	return 0;
}

#ifdef CONFIG_PCI_HCI
static inline
#endif
int __DOT11_Indicate_MIC_Failure(struct net_device *dev, struct stat_info *pstat)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	struct stat_info *pstat_del;
	DOT11_MIC_FAILURE	Mic_Failure;
	DOT11_DISASSOCIATION_IND Disassociation_Ind;
	int	i;

	if (priv->pshare->skip_mic_chk || ((priv->ext_stats.tx_avarage + priv->ext_stats.rx_avarage) > (1024*1024/8)))
		return 0;

	// Indicate to upper layer
	if (pstat != NULL) // if pstat==NULL, it is called by upper layer
	{
		DEBUG_INFO("MIC error indicate to 1x in driver\n");
		//DEBUG_INFO("mac: %02x%02x%02x%02x%02x%02x\n", pstat->hwaddr[0], pstat->hwaddr[1],
		//pstat->hwaddr[2], pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5]);

#ifdef __DRAYTEK_OS__
		cb_tkip_micFailure(dev, pstat->hwaddr);
		return 0;
#endif

#if !defined(WITHOUT_ENQUEUE) && !defined(RTK_NL80211)
		Mic_Failure.EventId = DOT11_EVENT_MIC_FAILURE;
		Mic_Failure.IsMoreEvent = 0;
		memcpy(&Mic_Failure.MACAddr, pstat->hwaddr, MACADDRLEN);
		DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&Mic_Failure, sizeof(DOT11_MIC_FAILURE));
#endif
#ifdef INCLUDE_WPA_PSK
		psk_indicate_evt(priv, DOT11_EVENT_MIC_FAILURE, pstat->hwaddr, NULL, 0);
#endif

#ifdef WIFI_HAPD
		event_indicate_hapd(priv, pstat->hwaddr, HAPD_MIC_FAILURE, NULL);
#ifdef HAPD_DRV_PSK_WPS
		event_indicate(priv, pstat->hwaddr, 5);
#endif
#else
		event_indicate(priv, pstat->hwaddr, 5);
#endif
#ifdef WIFI_WPAS //_Eric ??
		event_indicate_wpas(priv, pstat->hwaddr, WPAS_MIC_FAILURE, NULL);
#endif
	}

	if (priv->MIC_timer_on)
	{
		// Second time to detect MIC error in a time period

		// Stop Timer (fill a timer before than now)
		if (timer_pending(&priv->MIC_check_timer))
			del_timer_sync(&priv->MIC_check_timer);
		priv->MIC_timer_on = FALSE;

		// Start Timer to reject assocaiton request from STA, and reject all the packet
		mod_timer(&priv->assoc_reject_timer, jiffies + REJECT_ASSOC_PERIOD);
		priv->assoc_reject_on = TRUE;
		for(i=0; i<NUM_STAT; i++)
		{
			if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)
#ifdef WDS
					&& !(priv->pshare->aidarray[i]->station.state & WIFI_WDS)
#endif
			) {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
				if (priv != priv->pshare->aidarray[i]->priv)
					continue;
#endif

				pstat_del = &(priv->pshare->aidarray[i]->station);
				if (!list_empty(&pstat_del->asoc_list))
				{
#ifdef _SINUX_
					printk("Second time to detect MIC error in a time period so disassociate the all client\n");
					mic_error_report(2);
#endif					
#ifndef WITHOUT_ENQUEUE
					memcpy((void *)Disassociation_Ind.MACAddr, (void *)pstat_del->hwaddr, MACADDRLEN);
					Disassociation_Ind.EventId = DOT11_EVENT_DISASSOCIATION_IND;
					Disassociation_Ind.IsMoreEvent = 0;
					Disassociation_Ind.Reason = _STATS_OTHER_;
					Disassociation_Ind.tx_packets = pstat_del->tx_pkts;
					Disassociation_Ind.rx_packets = pstat_del->rx_pkts;
					Disassociation_Ind.tx_bytes   = pstat_del->tx_bytes;
					Disassociation_Ind.rx_bytes   = pstat_del->rx_bytes;
					DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&Disassociation_Ind,
							sizeof(DOT11_DISASSOCIATION_IND));
#endif
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
					psk_indicate_evt(priv, DOT11_EVENT_DISASSOCIATION_IND, pstat_del->hwaddr, NULL, 0);
#endif

#ifdef RTK_NL80211
					event_indicate_cfg80211(priv, pstat_del->hwaddr, CFG80211_DEL_STA, NULL);
#endif
#ifdef WIFI_HAPD
					event_indicate_hapd(priv, pstat_del->hwaddr, HAPD_EXIRED, NULL);
#ifdef HAPD_DRV_PSK_WPS
					event_indicate(priv, pstat_del->hwaddr, 2);
#endif
#else
					event_indicate(priv, pstat_del->hwaddr, 2);
#endif

					issue_disassoc(priv, pstat_del->hwaddr, _RSON_MIC_FAILURE_);
				}
				free_stainfo(priv, pstat_del);
			}
		}
		priv->assoc_num = 0;
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
			priv->pmib->dot11ErpInfo.nonErpStaNum = 0;
			check_protection_shortslot(priv);
			priv->pmib->dot11ErpInfo.longPreambleStaNum = 0;
		}
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
			priv->ht_legacy_sta_num = 0;

		// ToDo: Should we delete group key?
#ifdef _SINUX_
		printk("-------------------------------------------------------\n");
		printk("Second time of MIC failure in a time period            \n");
		printk("-------------------------------------------------------\n");
#else
		DEBUG_INFO("-------------------------------------------------------\n");
		DEBUG_INFO("Second time of MIC failure in a time period            \n");
		DEBUG_INFO("-------------------------------------------------------\n");
#endif		
	}
	else
	{
		// First time to detect MIC error, start the timer
		mod_timer(&priv->MIC_check_timer, jiffies + MIC_TIMER_PERIOD);
		priv->MIC_timer_on = TRUE;

		DEBUG_INFO("-------------------------------------------------------\n");
		DEBUG_INFO("First time of MIC failure in a time period, start timer\n");
		DEBUG_INFO("-------------------------------------------------------\n");
	}

	return 0;
}

int DOT11_Indicate_MIC_Failure(struct net_device *dev, struct stat_info *pstat)
{
#if defined(CONFIG_PCI_HCI)
	return __DOT11_Indicate_MIC_Failure(dev, pstat);
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	notify_indicate_MIC_failure(dev, pstat);
	return 0;
#endif
}

void DOT11_Process_MIC_Timerup(unsigned long data)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)data;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	DEBUG_INFO("MIC Timer is up. Cancel Timer\n");
	priv->MIC_timer_on = FALSE;
}

void DOT11_Process_Reject_Assoc_Timerup(unsigned long data)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)data;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	DEBUG_INFO("Reject Association Request Timer is up. Cancel Timer\n");
	memset(priv->assoc_reject_mac,0,MACADDRLEN);
#ifdef CLIENT_MODE
	if (OPMODE & WIFI_STATION_STATE)
		start_clnt_lookup(priv, 0);
#endif
	priv->assoc_reject_on = FALSE;
}

#ifdef CONFIG_PCI_HCI
static inline
#endif
void __DOT11_Indicate_MIC_Failure_Clnt(struct rtl8192cd_priv *priv, unsigned char *sa)
{
	DOT11_MIC_FAILURE	Mic_Failure;
	struct stat_info 	*pstat_del;
	DOT11_DISASSOCIATION_IND Disassociation_Ind;
//	int	i;
#ifndef WITHOUT_ENQUEUE
	Mic_Failure.EventId = DOT11_EVENT_MIC_FAILURE;
	Mic_Failure.IsMoreEvent = 0;
	memcpy(&Mic_Failure.MACAddr, sa, MACADDRLEN);
	DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&Mic_Failure, sizeof(DOT11_MIC_FAILURE));
#endif
	event_indicate(priv, sa, 5);
#if defined(CLIENT_MODE) && defined(INCLUDE_WPA_PSK)
	if (OPMODE & (WIFI_STATION_STATE | WIFI_ASOC_STATE))
		psk_indicate_evt(priv, DOT11_EVENT_MIC_FAILURE, BSSID, NULL, 0);
#endif
#ifdef WIFI_WPAS
	event_indicate_wpas(priv, sa, WPAS_MIC_FAILURE, NULL);
#endif
	if (priv->MIC_timer_on) {
		// Second time to detect MIC error in a time period

		// Stop Timer (fill a timer before than now)
		if (timer_pending(&priv->MIC_check_timer))
			del_timer_sync(&priv->MIC_check_timer);
		priv->MIC_timer_on = FALSE;

		// Start Timer to reject assocaiton request from STA, and reject all the packet
		mod_timer(&priv->assoc_reject_timer, jiffies + REJECT_ASSOC_PERIOD);
		priv->assoc_reject_on = TRUE;
		memcpy(priv->assoc_reject_mac, BSSID, MACADDRLEN);

		pstat_del = get_stainfo(priv, BSSID);
		memcpy((void *)Disassociation_Ind.MACAddr, (void *)pstat_del->hwaddr, MACADDRLEN);
		DEBUG_INFO("%s() disassoc %02X:%02X:%02X:%02X:%02X:%02X\n",__func__,pstat_del->hwaddr[0],pstat_del->hwaddr[1],pstat_del->hwaddr[2],pstat_del->hwaddr[3],pstat_del->hwaddr[4],pstat_del->hwaddr[5]);

		Disassociation_Ind.EventId = DOT11_EVENT_DISASSOCIATION_IND;
		Disassociation_Ind.IsMoreEvent = 0;
		Disassociation_Ind.Reason = _STATS_OTHER_;
		Disassociation_Ind.tx_packets = pstat_del->tx_pkts;
		Disassociation_Ind.rx_packets = pstat_del->rx_pkts;
		Disassociation_Ind.tx_bytes   = pstat_del->tx_bytes;
		Disassociation_Ind.rx_bytes   = pstat_del->rx_bytes;
		DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&Disassociation_Ind,
				sizeof(DOT11_DISASSOCIATION_IND));
				
		/*
		 * To make sure the MIC Failure Report is sent before disassoc 
		 */
		delay_ms(20);
		issue_disassoc(priv, pstat_del->hwaddr, _RSON_MIC_FAILURE_);
		free_stainfo(priv, pstat_del);

		priv->assoc_num = 0;
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
			priv->pmib->dot11ErpInfo.nonErpStaNum = 0;
			check_protection_shortslot(priv);
			priv->pmib->dot11ErpInfo.longPreambleStaNum = 0;
		}
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
			priv->ht_legacy_sta_num = 0;

		DEBUG_INFO("-------------------------------------------------------\n");
		DEBUG_INFO("Second time of MIC failure in a time period            \n");
		DEBUG_INFO("-------------------------------------------------------\n");
	} else {
		// First time to detect MIC error, start the timer
		mod_timer(&priv->MIC_check_timer, jiffies + MIC_TIMER_PERIOD);
		priv->MIC_timer_on = TRUE;

		DEBUG_INFO("-------------------------------------------------------\n");
		DEBUG_INFO("First time of MIC failure in a time period, start timer\n");
		DEBUG_INFO("-------------------------------------------------------\n");
	}
}

void DOT11_Indicate_MIC_Failure_Clnt(struct rtl8192cd_priv *priv, unsigned char *sa)
{
#if defined(WIFI_WPAS_CLI)
	event_indicate_wpas(priv, sa, WPAS_MIC_FAILURE, NULL);
#elif defined(CONFIG_PCI_HCI)
	__DOT11_Indicate_MIC_Failure_Clnt(priv, sa);
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	notify_indicate_MIC_failure_clnt(priv, sa);
#endif
}

#ifdef RADIUS_ACCOUNTING
void DOT11_Process_Acc_SetExpiredTime(struct net_device *dev, struct iw_point *data)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	//WLAN_CTX        	*wCtx = (WLAN_CTX *) ( priv->pwlanCtx );
	DOT11_SET_EXPIREDTIME	*Set_ExpireTime = (DOT11_SET_EXPIREDTIME *)data->pointer;
	struct stat_info *pstat=NULL;

	if( Set_ExpireTime != NULL ){
		Set_ExpireTime->EventId = DOT11_EVENT_ACC_SET_EXPIREDTIME;
		Set_ExpireTime->IsMoreEvent = 0;
		
		if( (pstat = get_stainfo(priv, Set_ExpireTime->MACAddr)) ){
			pstat->def_expired_time = Set_ExpireTime->ExpireTime;
			DEBUG_INFO("%s: Set %02x:%02x:%02x:%02x:%02x:%02x def_expired_time = %ld!\n", (char *)__FUNCTION__,
				pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5],
				Set_ExpireTime->ExpireTime);
		}
		else{
			DEBUG_ERR("%s: ERRO, CAN NOT GET STA INFO!\n", (char *)__FUNCTION__);
		}
	}
	else{
		DEBUG_ERR("%s: NULL POINTER!\n", (char *)__FUNCTION__);
	}
}


void DOT11_Process_Acc_QueryStats(struct net_device *dev, struct iw_point *data)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	//WLAN_CTX        	*wCtx = (WLAN_CTX *) ( priv->pwlanCtx );
	struct stat_info *pstat=NULL;
	DOT11_QUERY_STATS	*pStats = (DOT11_QUERY_STATS *)data->pointer;

	if( pStats != NULL ){
		pStats->EventId = DOT11_EVENT_ACC_QUERY_STATS;
		pStats->IsMoreEvent = 0;
		
		if( (pstat = get_stainfo(priv, pStats->MACAddr)) ){
			pStats->tx_packets = pstat->tx_pkts;
			pStats->rx_packets = pstat->rx_pkts;
			pStats->tx_bytes = pstat->tx_bytes;
			pStats->rx_bytes = pstat->rx_bytes;

			pStats->IsSuccess = TRUE;
			DEBUG_INFO("%s: Get %02x:%02x:%02x:%02x:%02x:%02x stats!\n", (char *)__FUNCTION__,
				pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
		}
		else{
			pStats->IsSuccess = FALSE;
			DEBUG_ERR("%s: ERROR, CAN NOT GET STA INFO!\n", (char *)__FUNCTION__);
		}
	}
	else{
		DEBUG_ERR("%s: NULL POINTER!\n", (char *)__FUNCTION__);
	}
}


//-------------------------
//DOT11_QUERY_STATS		stats[RTL_AP_MAX_STA_NUM+1];
//data->pointer = (unsigned char *)stats;
void DOT11_Process_Acc_QueryStats_All(struct net_device *dev, struct iw_point *data)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	struct list_head *phead=NULL, *plist=NULL;
	struct stat_info *pstat=NULL;
	//WLAN_CTX        	*wCtx = (WLAN_CTX *) ( priv->pwlanCtx );
	DOT11_QUERY_STATS	*pStats = (DOT11_QUERY_STATS *)data->pointer;
	//int i;
	int cnt = 0;
#ifdef SMP_SYNC
	unsigned long flags=0;
#endif

	if( pStats != NULL ){
		phead = &priv->asoc_list;
		
		SMP_LOCK_ASOC_LIST(flags);
		
		plist = phead->next;
		while (plist != phead) {
			pstat = list_entry(plist, struct stat_info, asoc_list);
			plist = plist->next;
			
			pStats[cnt].EventId = DOT11_EVENT_ACC_QUERY_STATS_ALL;
			pStats[cnt].IsMoreEvent = 0;
			memcpy( pStats[cnt].MACAddr, pstat->hwaddr, MACADDRLEN );
			pStats[cnt].tx_packets = pstat->tx_pkts;
			pStats[cnt].rx_packets = pstat->rx_pkts;
			pStats[cnt].tx_bytes = pstat->tx_bytes;
			pStats[cnt].rx_bytes = pstat->rx_bytes;

			cnt++;
		}
		SMP_UNLOCK_ASOC_LIST(flags);
	}
	else{
		printk("%s: NULL POINTER!\n", (char *)__FUNCTION__);
	}
}
#endif


static int DOT11_Process_STA_Query_Bssid(struct net_device *dev, struct iw_point *data)
{
	DOT11_STA_QUERY_BSSID	*pQuery = (DOT11_STA_QUERY_BSSID *)data->pointer;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	pQuery->EventId = DOT11_EVENT_STA_QUERY_BSSID;
	pQuery->IsMoreEvent = FALSE;
	if ((OPMODE & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) ==
			(WIFI_STATION_STATE | WIFI_ASOC_STATE))
	{
		pQuery->IsValid = TRUE;
		memcpy(pQuery->Bssid, BSSID, 6);
	}
	else
		pQuery->IsValid = FALSE;

	return 0;
}


static int DOT11_Process_STA_Query_Ssid(struct net_device *dev, struct iw_point *data)
{
	DOT11_STA_QUERY_SSID	*pQuery = (DOT11_STA_QUERY_SSID *)data->pointer;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	pQuery->EventId = DOT11_EVENT_STA_QUERY_SSID;
	pQuery->IsMoreEvent = FALSE;
	if ((OPMODE & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) ==
			(WIFI_STATION_STATE | WIFI_ASOC_STATE))
	{
		pQuery->IsValid = TRUE;
		memcpy(pQuery->ssid, SSID, 32);
		pQuery->ssid_len = SSID_LEN;
	}
	else
		pQuery->IsValid = FALSE;

	return 0;
}


#ifdef WIFI_SIMPLE_CONFIG
static int DOT11_WSC_set_ie(struct net_device *dev, struct iw_point *data)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	DOT11_SET_RSNIE *Set_RSNIE = (DOT11_SET_RSNIE *)data->pointer;

	if (Set_RSNIE->Flag == SET_IE_FLAG_BEACON) {
		DEBUG_INFO("WSC: set beacon IE\n");
		priv->pmib->wscEntry.beacon_ielen = Set_RSNIE->RSNIELen;
		memcpy((void *)priv->pmib->wscEntry.beacon_ie, Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
#ifdef SDIO_AP_OFFLOAD
	{
#define TAG_SELECTED_REGISTRAR		0x1041

		int attr_len;
		if (search_wsc_tag(Set_RSNIE->RSNIE+6, TAG_SELECTED_REGISTRAR,
				Set_RSNIE->RSNIELen-6, &attr_len))		// found
			ap_offload_deactivate(priv, OFFLOAD_PROHIBIT_WPS);
		else
			ap_offload_activate(priv, OFFLOAD_PROHIBIT_WPS);
	}
#endif // SDIO_AP_OFFLOAD
	}
	else if (Set_RSNIE->Flag == SET_IE_FLAG_PROBE_RSP) {
		DEBUG_INFO("WSC: set probe response IE\n");
		priv->pmib->wscEntry.probe_rsp_ielen = Set_RSNIE->RSNIELen;
		memcpy((void *)priv->pmib->wscEntry.probe_rsp_ie, Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
	}
	else if (Set_RSNIE->Flag == SET_IE_FLAG_PROBE_REQ) {
		DEBUG_INFO("WSC: set probe request IE\n");
		priv->pmib->wscEntry.probe_req_ielen = Set_RSNIE->RSNIELen;
		memcpy((void *)priv->pmib->wscEntry.probe_req_ie, Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
	}
	else if (Set_RSNIE->Flag == SET_IE_FLAG_ASSOC_REQ ||
			Set_RSNIE->Flag == SET_IE_FLAG_ASSOC_RSP) {
		DEBUG_INFO("WSC: set assoc IE\n");
		priv->pmib->wscEntry.assoc_ielen = Set_RSNIE->RSNIELen;
		memcpy((void *)priv->pmib->wscEntry.assoc_ie, Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
	}
#if 0
	else if (Set_RSNIE->Flag == 3) {
		printk("WSC: set RSN IE\n");
		priv->pmib->dot11RsnIE.rsnielen = Set_RSNIE->RSNIELen;
		memcpy((void *)priv->pmib->dot11RsnIE.rsnie, Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
	}
#endif
	else {
		DEBUG_ERR("Invalid flag of set IE [%d]!\n", Set_RSNIE->Flag);
	}

#ifdef PCIE_POWER_SAVING
	if ((Set_RSNIE->Flag == SET_IE_FLAG_BEACON) || (Set_RSNIE->Flag == SET_IE_FLAG_PROBE_RSP))
		PCIeWakeUp(priv, (POWER_DOWN_T0));
#endif

	return 0;
}

#ifdef	INCLUDE_WPS
#ifndef CONFIG_MSC
static int dispatch_wscsoap(struct rtl8192cd_priv *priv ,pDOT11_WSC_SOAP soap)
{
	struct WSC_packet *packet=NULL;

	printk("\nWlan Driver receive SOAP request:%s\n",soap->action);

	packet = &soap->packet;

	if( strcmp(soap->action,"WFAGetDeviceInfo") ){
		packet->EventType = WSC_NOT_PROXY;
		packet->EventID = WSC_GETDEVINFO;
	} else if ( strcmp(soap->action,"SendMsgToSM_Dir_In") ){
		packet->EventID = WSC_SETSELECTEDREGISTRA;
	} else if ( strcmp(soap->action,"SendMsgToSM_Dir_InOut") ){
		packet->EventID = WSC_M2M4M6M8;
	} else if ( strcmp(soap->action,"WFAPutWLANResponse") ) {
		packet->EventID = WSC_PUTWLANRESPONSE;
	}
#ifdef SUPPORT_UPNP    	
	if (PWSCUpnpCallbackEventHandler(packet) != WSC_UPNP_SUCCESS) {
		DEBUG_ERR("WSCCallBack Fail!\n");
		goto error_handle;
	}
#endif	
	return 0;

error_handle:
	return -1;
}
#endif
#endif

#endif // WIFI_SIMPLE_CONFIG

#ifdef USER_ADDIE
static int DOT11_USER_set_ie(struct net_device *dev, struct iw_point *data)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	DOT11_SET_USERIE *Set_USERIE = (DOT11_SET_USERIE *)data->pointer;
	unsigned int i;
	
	if (Set_USERIE->Flag == SET_IE_FLAG_INSERT) {
		DEBUG_INFO("USER: add beacon IE\n");
		for (i=0; i<MAX_USER_IE; i++) {
			if (!priv->user_ie_list[i].used)
				break;
		}
		if (i == MAX_USER_IE)
			return -1;
		priv->user_ie_list[i].used = 1;
		priv->user_ie_list[i].ie_len = Set_USERIE->USERIELen;
		memcpy((void *)priv->user_ie_list[i].ie, Set_USERIE->USERIE, Set_USERIE->USERIELen);
	}
	else if (Set_USERIE->Flag == SET_IE_FLAG_DELETE) {
		DEBUG_INFO("USER: remove beacon IE\n");
		for (i=0; i<MAX_USER_IE; i++) {
			if (priv->user_ie_list[i].used &&
				!memcmp(priv->user_ie_list[i].ie, Set_USERIE->USERIE, Set_USERIE->USERIELen)) {
				priv->user_ie_list[i].used = 0;
				break;
			}
		}
		if (i == MAX_USER_IE)
			return -1;
	}
	else {
		DEBUG_ERR("Invalid flag of set IE [%d]!\n", Set_USERIE->Flag);
	}

	return 0;
}

#endif

#ifdef CONFIG_IEEE80211W
static int DOT11_PMF_set_ie(struct net_device *dev, struct iw_point *data)
{
#ifdef NETDEV_NO_PRIV
    struct rtl8192cd_priv *priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
    struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)(dev->priv);
#endif

	struct stat_info *pstat; 
	DOT11_SET_11W_Flags flags;				
	if (!memcpy((void *)(&flags),(void *)(data->pointer), data->length))
	{
		PMFDEBUG("memcpy fail!, data=%x, length=%d\n", data->pointer, data->length);
		return -1;
	}

	pstat= get_stainfo(priv,flags.macAddr); 			

	if(!pstat){
		//PMFDEBUG("psta is NULL\n");
	}else{
		pstat->isPMF = flags.isPMF;
    	PMFDEBUG("set sta[%02x%02x%02x:%02x%02x%02x]'s PMF=[%d]\n",pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5], flags.isPMF);		        
    }

	return 0;
}
#endif



#ifdef HS2_SUPPORT
/* Hotspot 2.0 Release 1*/
static int DOT11_HS2_set_ie(struct net_device *dev, struct iw_point *data)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
    DOT11_SET_RSNIE *Set_RSNIE = (DOT11_SET_RSNIE *)data->pointer;
    HS2_DEBUG_INFO("HS2 set ie, flag=%d\n",Set_RSNIE->Flag);
    if (Set_RSNIE->Flag == SET_IE_FLAG_INTERWORKING)
    {
        //DEBUG_INFO
        HS2_DEBUG_TRACE(1, "HS2: set interworking IE,%x,%x\n",Set_RSNIE->RSNIE[0], Set_RSNIE->RSNIE[1]);
        priv->pmib->hs2Entry.interworking_ielen = Set_RSNIE->RSNIELen;
        memcpy((void *)priv->pmib->hs2Entry.interworking_ie, Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
    }
	else if (Set_RSNIE->Flag == SET_IE_FLAG_QOSMAP)
    {
        //DEBUG_INFO
		HS2_DEBUG_TRACE(1, "HS2: set QoSMAP set IE,%x,%x\n",Set_RSNIE->RSNIE[0], Set_RSNIE->RSNIE[1]);
		if(priv->pmib->hs2Entry.nQoSMap == 2) {
			priv->pmib->hs2Entry.nQoSMap = 0;
			panic_printk("WLAN Driver cannot set more than two QoSMAP ie\n");
		}
		
        priv->pmib->hs2Entry.QoSMap_ielen[priv->pmib->hs2Entry.nQoSMap] = Set_RSNIE->RSNIELen;
	    memcpy((void *)priv->pmib->hs2Entry.QoSMap_ie[priv->pmib->hs2Entry.nQoSMap], Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
		if(priv->pmib->hs2Entry.nQoSMap == 0) {
			priv->pmib->hs2Entry.curQoSMap = 0;
			setQoSMapConf(priv);
		}
		HS2_DEBUG_TRACE(1, "QoSMap_ielen[%d]=%d\n",priv->pmib->hs2Entry.nQoSMap,priv->pmib->hs2Entry.QoSMap_ielen[priv->pmib->hs2Entry.nQoSMap]);
		priv->pmib->hs2Entry.nQoSMap++;
		
    }
    else if (Set_RSNIE->Flag == SET_IE_FLAG_ADVT_PROTO)
    {
        HS2_DEBUG_TRACE(1, "HS2: set advt proto IE\n");
        priv->pmib->hs2Entry.advt_proto_ielen = Set_RSNIE->RSNIELen;
        memcpy((void *)priv->pmib->hs2Entry.advt_proto_ie, Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
    }
    else if (Set_RSNIE->Flag == SET_IE_FLAG_ROAMING)
    {
        HS2_DEBUG_TRACE(1, "HS2: set roaming IE\n");
        priv->pmib->hs2Entry.roam_ielen = Set_RSNIE->RSNIELen;
        memcpy((void *)priv->pmib->hs2Entry.roam_ie, Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
    }
    else if (Set_RSNIE->Flag == SET_IE_FLAG_HS2)
    {
        HS2_DEBUG_TRACE(1, "HS2: set HS2 IE\n");
        priv->pmib->hs2Entry.hs2_ielen = Set_RSNIE->RSNIELen;
        memcpy((void *)priv->pmib->hs2Entry.hs2_ie, Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
        if (priv->pmib->hs2Entry.hs2_ie[4] & 0x01)
        {
            HS2_DEBUG_TRACE(1, "dgaf enable\n");
            priv->dgaf_disable = 1;
        }
        else
        {
            HS2_DEBUG_TRACE(1, "dgaf disable\n");
            priv->dgaf_disable = 0;
    	}
		//OSU_Present
		if (priv->pmib->hs2Entry.hs2_ie[4] & 0x02)
        {
        	DEBUG_INFO("OSU Provider List enable\n");
        	priv->OSU_Present = 1;
        }
    	else
    	{
    		DEBUG_INFO("OSU Provider List disable\n");
    		priv->OSU_Present = 0;
    	}
        //channel utilization
        //rtl8192cd_cu_start(priv);
    }
    else if (Set_RSNIE->Flag == SET_IE_FLAG_TIMEADVT)
    {
        HS2_DEBUG_TRACE(1, "HS2: set timeadvt IE\n");
        priv->pmib->hs2Entry.timeadvt_ielen = Set_RSNIE->RSNIELen;
        memcpy((void *)priv->pmib->hs2Entry.timeadvt_ie, Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
    }
    else if (Set_RSNIE->Flag == SET_IE_FLAG_TIMEZONE)
    {
        HS2_DEBUG_TRACE(1, "HS2: set time zone IE\n");
        priv->pmib->hs2Entry.timezone_ielen = Set_RSNIE->RSNIELen;
        memcpy((void *)priv->pmib->hs2Entry.timezone_ie, Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
    }
    else if (Set_RSNIE->Flag == SET_IE_FLAG_PROXYARP)
    {
        priv->proxy_arp = *Set_RSNIE->RSNIE;
        HS2_DEBUG_INFO("\n proxy arp in driver[%d]\n", priv->proxy_arp);
    } 
    else if (Set_RSNIE->Flag == SET_IE_FLAG_ICMPv4ECHO)
    {
        priv->pmib->hs2Entry.ICMPv4ECHO = *Set_RSNIE->RSNIE;
        HS2_DEBUG_INFO("ICMPv4ECHO in driver=[%d]\n", priv->pmib->hs2Entry.ICMPv4ECHO);
    }	
	else if (Set_RSNIE->Flag == SET_IE_FLAG_MBSSID)
	{
		HS2_DEBUG_INFO("HS2: set Multiple BSSID IE\n");
        priv->pmib->hs2Entry.MBSSID_ielen= Set_RSNIE->RSNIELen;
        memcpy((void *)priv->pmib->hs2Entry.MBSSID_ie, Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
	}
	else if (Set_RSNIE->Flag == SET_IE_FLAG_REMEDSVR)
	{
		HS2_DEBUG_INFO("HS2: set REMEDSVR string\n");
        memcpy((void *)priv->pmib->hs2Entry.remedSvrURL, Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
		HS2_DEBUG_INFO("REMEDSVR:%s\n",priv->pmib->hs2Entry.remedSvrURL);
	}
	else if (Set_RSNIE->Flag == SET_IE_FLAG_SessionInfoURL)
	{
		DEBUG_INFO("HS2: set SessionInfoURL string\n");
        memcpy((void *)priv->pmib->hs2Entry.SessionInfoURL, Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
		HS2_DEBUG_INFO("SessionInfoURL:%s\n",priv->pmib->hs2Entry.SessionInfoURL);
	}
	else if (Set_RSNIE->Flag == SET_IE_FLAG_MMPDULIMIT) {
		HS2_DEBUG_INFO("HS2: set REMEDSVR string\n");
        memcpy((void *)(&priv->pmib->hs2Entry.mmpdu_limit), Set_RSNIE->RSNIE, Set_RSNIE->RSNIELen);
		HS2_DEBUG_INFO("mmpdu_limit:%d\n",priv->pmib->hs2Entry.mmpdu_limit);
	}
    else
    {
        HS2_DEBUG_ERR("HS2: unknown ioctl flag\n");
    }
}
#endif

/*-----------------------------------------------------------------------------
  Most of the time, we don't have to worry about the racing condition of
  "event_queue" in wlan drivers, since all the queue/dequeue are handled
  in non-isr context.
  However, my guess is, someone always want to port these driver to fit different
  OS platform. At that time, please always keep in mind all the possilbe racing
  condition... That could be big disasters...
  ------------------------------------------------------------------------------*/
#if defined(CONFIG_RTL_KERNEL_MIPS16_WLAN) && defined(CONFIG_RTL8196C)
__NOMIPS16
#endif
int rtl8192cd_ioctl_priv_daemonreq(struct net_device *dev, struct iw_point *data)
{
	int		ret;
#ifndef WITHOUT_ENQUEUE
	static UINT8 QueueData[MAXDATALEN];
	int		QueueDataLen;
#elif defined(CONFIG_RTL_WAPI_SUPPORT)
	static UINT8 QueueData[MAXDATALEN];
	int		QueueDataLen;
#endif
	UINT8	val8;
	DOT11_REQUEST	req;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
	memcpy((void *)&req, (void *)(data->pointer), sizeof(DOT11_REQUEST));
#else
	if (copy_from_user((void *)&req, (void *)(data->pointer), sizeof(DOT11_REQUEST))) {
		DEBUG_ERR("copy_from_user error!\n");
		return -1;
	}
#endif
#ifdef	INCLUDE_WPS
	DOT11_GETSET_MIB *getset_mib_t;
#endif
	switch(req.EventId)
	{
		case DOT11_EVENT_NO_EVENT:
			break;

#ifndef WITHOUT_ENQUEUE
		case DOT11_EVENT_REQUEST:
			if((ret = DOT11_DeQueue((unsigned long)priv, priv->pevent_queue, QueueData, &QueueDataLen)) != 0)
			{
				val8 = DOT11_EVENT_NO_EVENT;
				if (copy_to_user((void *)((unsigned long)(data->pointer)), &val8, 1)) {
					DEBUG_ERR("copy_to_user fail!\n");
					return -1;
				}
				val8 = 0;
				if (copy_to_user((void *)((unsigned long)(data->pointer) + 1), &val8, 1)) {
					DEBUG_ERR("copy_to_user fail!\n");
					return -1;
				}
				data->length = sizeof(DOT11_NO_EVENT);
			}
			else
			{
				QueueData[1] = (priv->pevent_queue->NumItem != 0)? 1 : 0;
				if (copy_to_user((void *)data->pointer, (void *)QueueData, QueueDataLen)) {
					DEBUG_ERR("copy_to_user fail!\n");
					return -1;
				}
				data->length = QueueDataLen;
			}
			break;
#ifdef INCLUDE_WPS
		case DOT11_EVENT_REQUEST_F_INCLUDE_WPS:
			if((ret = DOT11_DeQueue((unsigned long)priv, priv->pevent_queue, QueueData, &QueueDataLen)) != 0)
			{
				val8 = DOT11_EVENT_NO_EVENT;
				memcpy((void *)((unsigned long)(data->pointer)), &val8, 1);

				val8 = 0;
				memcpy((void *)((unsigned long)(data->pointer) + 1), &val8, 1);
				data->length = sizeof(DOT11_NO_EVENT);
			}
			else
			{				
				QueueData[1] = (priv->pevent_queue->NumItem != 0)? 1 : 0;				
				memcpy((void *)data->pointer, (void *)QueueData, QueueDataLen);				
				data->length = QueueDataLen;					
				//DEBUG_INFO("%d de-queue ; pMsg=%s\n", __LINE__ , (char *)data->pointer);
			}
			break;
#endif			
#endif

		case DOT11_EVENT_ASSOCIATION_RSP:
			if(!DOT11_Process_Association_Rsp(dev, data, WIFI_ASSOCRSP))
			{}
			break;

		case DOT11_EVENT_DISCONNECT_REQ:
			if(!DOT11_Process_Disconnect_Req(dev, data))
			{}
			break;

		case DOT11_EVENT_SET_802DOT11:
			break;

		case DOT11_EVENT_SET_KEY:
#if defined(CONFIG_PCI_HCI)
			if(!DOT11_Process_Set_Key(dev, data, NULL, NULL))
			{}
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			notify_set_key(dev, (DOT11_SET_KEY *)data->pointer, NULL);
#endif
			break;

		case DOT11_EVENT_SET_PORT:
			if(!DOT11_Process_Set_Port(dev, data))
			{}
			break;

		case DOT11_EVENT_DELETE_KEY:
			if(!DOT11_Process_Delete_Key(dev, data))
			{}
			break;

		case DOT11_EVENT_SET_RSNIE:
			if(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _NO_PRIVACY_ &&
					priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WEP_40_PRIVACY_ &&
					priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WEP_104_PRIVACY_)
				if(!DOT11_Process_Set_RSNIE(dev, data))
				{}
			break;

		case DOT11_EVENT_GKEY_TSC:
			if(!DOT11_Process_QueryRSC(dev, data))
			{}
			break;

		case DOT11_EVENT_MIC_FAILURE:
			if(!DOT11_Porcess_EAPOL_MICReport(dev, data))
			{}
			break;

		case DOT11_EVENT_ASSOCIATION_INFO:
#if 0
			if(!DOT11_Process_Association_Info(dev, data))
			{}
#endif
			DEBUG_INFO("trying to process assoc info\n");
			break;

		case DOT11_EVENT_INIT_QUEUE:
			DOT11_InitQueue(priv->pevent_queue);
			break;

		case DOT11_EVENT_ACC_SET_EXPIREDTIME:
#ifdef RADIUS_ACCOUNTING
			DOT11_Process_Acc_SetExpiredTime(dev, data);
#endif
			DEBUG_INFO("trying to Set ACC Expiredtime\n");
			break;

		case DOT11_EVENT_ACC_QUERY_STATS:
#ifdef RADIUS_ACCOUNTING			
			DOT11_Process_Acc_QueryStats(dev, data);
#endif
			DEBUG_INFO("trying to Set ACC Expiredtime\n");
			break;

		case DOT11_EVENT_REASSOCIATION_RSP:
			if(!DOT11_Process_Association_Rsp(dev, data, WIFI_REASSOCRSP))
			{}
			break;

		case DOT11_EVENT_STA_QUERY_BSSID:
			if(!DOT11_Process_STA_Query_Bssid(dev, data))
			{};
			break;

		case DOT11_EVENT_STA_QUERY_SSID:
			if(!DOT11_Process_STA_Query_Ssid(dev, data))
			{};
			break;

#ifdef WIFI_SIMPLE_CONFIG
		case DOT11_EVENT_WSC_SET_IE:
			if(!DOT11_WSC_set_ie(dev, data))
			{};
			break;

#ifdef INCLUDE_WPS
#ifndef CONFIG_MSC
			// procress ioctl request from upnp

		case DOT11_EVENT_WSC_SOAP:{
						  CTX_Tp tmp = &(priv->pshare->WSC_CONT_S);
						  DOT11_WSC_SOAP *soap_evt;  //chris 0328
						  DOT11_EAP_PACKET *upnp_pkt; //chris 0328
						  struct soap_t *soap_pkt; //chris 0328

						  soap_evt = (DOT11_WSC_SOAP *) kmalloc(sizeof(DOT11_WSC_SOAP), GFP_ATOMIC);
						  upnp_pkt = (DOT11_EAP_PACKET *) kmalloc(sizeof(DOT11_EAP_PACKET), GFP_ATOMIC);
						  soap_pkt = (struct soap_t *) kmalloc(sizeof(struct soap_t), GFP_ATOMIC);

						  memset(soap_evt, 0, sizeof(DOT11_WSC_SOAP));
						  memset(upnp_pkt, 0, sizeof(DOT11_EAP_PACKET));
						  memset(soap_pkt, 0, sizeof(struct soap_t));

						  if (copy_from_user((void *)soap_evt, (void *)(data->pointer), sizeof(DOT11_WSC_SOAP))) {
							  //DEBUG_ERR("copy_from_user error!\n");
							  printk("copy_from_user error!\n");
							  kfree(soap_evt);
							  kfree(upnp_pkt);
							  kfree(soap_pkt);
							  return -1;
						  }
						  memcpy(soap_pkt->EventMac, soap_evt->packet.EventMac, MACLEN);
						  memcpy(soap_pkt->IP, soap_evt->packet.IP, IP_ADDRLEN);
						  memcpy(soap_pkt->packet, soap_evt->packet.buffer, soap_evt->packet.size);
						  soap_pkt->length = soap_evt->packet.size;

						  if( soap_evt->packet.EventID == WSC_GETDEVINFO ){
							  upnp_pkt->EventId = DOT11_EVENT_WSC_GETDEVINFO;
							  tmp->wps_profile.soap_mathods.upnpGetDeviceInfoHandler(soap_pkt);
							  printk("%s %d ; call upnpGetDeviceInfoHandler\n",__FUNCTION__,__LINE__);
						  }else if ( soap_evt->packet.EventID == WSC_M2M4M6M8 ) {
							  upnp_pkt->EventId = DOT11_EVENT_WSC_M2M4M6M8;
							  tmp->wps_profile.soap_mathods.upnpPutMessageHandler(soap_pkt);
						  }else if ( soap_evt->packet.EventID == WSC_SETSELECTEDREGISTRA ) {
							  upnp_pkt->EventId = DOT11_EVENT_WSC_PUTMESSAGE;
							  tmp->wps_profile.soap_mathods.upnpSelectedRegistrarHandler(soap_pkt);
						  }else if ( soap_evt->packet.EventID == WSC_PUTWLANRESPONSE ) {
							  upnp_pkt->EventId = DOT11_EVENT_WSC_PUTWLANRESPONSE;
							  tmp->wps_profile.soap_mathods.upnpWlanResponseHandler(soap_pkt);
						  }

						  upnp_pkt->IsMoreEvent = FALSE;
						  /*
						     if( packet.tx_size < MAX_MSG_SIZE ){
						     upnp_packet.packet_len = packet.tx_size;
						     memcpy(upnp_packet.packet, packet.tx_buffer, packet.tx_size);
						     } else {
						     upnp_packet.packet_len = 0;
						     }*/
						  if( soap_pkt->length > 0 && soap_pkt->length < MAX_MSG_SIZE ){
							  upnp_pkt->packet_len = soap_pkt->length;
							  memcpy(upnp_pkt->packet, soap_pkt->packet, soap_pkt->length);
						  } else {
							  upnp_pkt->packet_len = 0;
						  }

						  printk("Rx ; En-Queue UPnP Packet %s %d \n",__FUNCTION__ , __LINE__);
						  wsc_debug_out(">>>>>>>>>>>>>upnp_packet",  upnp_pkt, sizeof(DOT11_EAP_PACKET));
						  DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (unsigned char*)upnp_pkt, sizeof(DOT11_EAP_PACKET));
						  event_indicate(priv, NULL, -1);

						  kfree(soap_evt);
						  kfree(upnp_pkt);
						  kfree(soap_pkt);

					  }
					  break;

		case DOT11_EVENT_WSC_PIN:{
						 CTX_Tp tmp = &(priv->pshare->WSC_CONT_S);
						 DOT11_WSC_METHOD wps_pin;
						 if (copy_from_user((void *)&wps_pin, (void *)(data->pointer), sizeof(DOT11_WSC_PIN_IND))) {
							 //DEBUG_ERR("copy_from_user error!\n");
							 printk("copy_from_user error!\n");
							 return -1;
						 }
						 tmp->start = 3;
						 memcpy(tmp->peer_pin_code, wps_pin.code, PIN_LEN);
						 //evHandler_pin_input(wps_pin.code);
						 printk("PIN method invoked (from wscd)\n");
					 }
					 break;
		case DOT11_EVENT_WSC_PBC:{
						 CTX_Tp tmp = &(priv->pshare->WSC_CONT_S);
						 tmp->start = 2;
						 //evHandler_pb_press();
						 printk("PBC method invoked from wscd\n");
					 }
					 break;
		case DOT11_EVENT_WSC_PUTCONF:{
						     DOT11_WSC_GETCONF wps_getconf;
						     CTX_Tp tmp = &(priv->pshare->WSC_CONT_S);
						     memset(&wps_getconf,0,sizeof(DOT11_WSC_GETCONF));
						     WSC_CONFp wps_config = &(wps_getconf.config);
						     if (copy_from_user((void *)&wps_getconf, (void *)(data->pointer), sizeof(WSC_CONF))) {
							     DEBUG_ERR("copy_from_user error!\n");
							     return -1;
						     }
						     /*
							for verify ,wscd-deamon will report config-info at WSC_CONF
							call to kernel-mode-wps , receive_config_setting();

							in sercomm case , will not via here,sercomm will direct call wps_start()
						      */
						     printk("[wlan ioctl]:receive \"DOT11_EVENT_WSC_PUTCONF\" from  wscd \n");
						     //receive_config_setting(priv , wps_config);							
						     read_config_file(tmp, DEFAULT_CONFIG_FILENAME);
					     }
					     break;
		case DOT11_EVENT_WSC_PROXY_ON: {
						       printk("[%s] switch proxy on\n",__FUNCTION__);
						       priv->pshare->WSC_CONT_S.TotalSubscriptions = 1;
					       }
					       break;
		case DOT11_EVENT_WSC_PROXY_OFF: {
							printk("[%s] switch proxy off\n",__FUNCTION__);
							priv->pshare->WSC_CONT_S.TotalSubscriptions = 0;
						}
						break;			
#endif
#endif

#ifdef	INCLUDE_WPS
		case DOT11_EVENT_WSC_SET_MIB:

						getset_mib_t = (DOT11_GETSET_MIB *)data->pointer;
						set_mib(dev->priv, getset_mib_t->cmd);
						break;

		case DOT11_EVENT_WSC_GET_MIB:
						getset_mib_t = (DOT11_GETSET_MIB*)data->pointer;
						get_mib(dev->priv, getset_mib_t->cmd);
						break;			
#endif
#endif	//WIFI_SIMPLE_CONFIG

#ifdef CONFIG_RTL_WAPI_SUPPORT
		case DOT11_EVENT_WAPI_INIT_QUEUE:
						DOT11_InitQueue(priv->wapiEvent_queue);
						break;
		case DOT11_EVENT_WAPI_READ_QUEUE:
						if((ret = DOT11_DeQueue((unsigned long)priv, priv->wapiEvent_queue, QueueData, &QueueDataLen)) != 0)
						{
							val8 = DOT11_EVENT_NO_EVENT;
							if (copy_to_user((void *)((unsigned long)(data->pointer)), &val8, 1)) {
								DEBUG_ERR("copy_to_user fail!\n");
								return -1;
							}
							val8 = 0;
							if (copy_to_user((void *)((unsigned long)(data->pointer) + 1), &val8, 1)) {
								DEBUG_ERR("copy_to_user fail!\n");
								return -1;
							}
							data->length = sizeof(DOT11_NO_EVENT);
						}
						else
						{
							QueueData[1] = (priv->wapiEvent_queue->NumItem != 0)? 1 : 0;
							if (copy_to_user((void *)data->pointer, (void *)QueueData, QueueDataLen)) {
								DEBUG_ERR("copy_to_user fail!\n");
								return -1;
							}
							data->length = QueueDataLen;
						}
						break;
		case DOT11_EVENT_WAPI_WRITE_QUEUE:
						{
							unsigned char *kernelbuf;
							kernelbuf = (unsigned char *)kmalloc(data->length,GFP_ATOMIC);
							if(NULL == kernelbuf)
							{
								DEBUG_ERR("Memory alloc fail!\n");
								return -1;
							}
							if (copy_from_user((void *)kernelbuf, (void *)(data->pointer), data->length)) {
								DEBUG_ERR("copy_from_user error!\n");
								kfree(kernelbuf);
								return -1;
							}
							DOT11_Process_WAPI_Info(priv,kernelbuf,data->length);
							kfree(kernelbuf);
						}
						break;
#endif
#ifdef HS2_SUPPORT
/* Hotsport 2.0 Release 1 */
		case DOT11_EVENT_HS2_SET_IE:
            if(!DOT11_HS2_set_ie(dev, data))
            {};
			break;
        case DOT11_EVENT_HS2_GET_TSF:
			{
				unsigned long long  tsf;
				unsigned long tsf_sec;
				tsf = RTL_R32(TSFTR+4);
				tsf = (tsf<<32) +  RTL_R32(TSFTR);
				tsf_sec = (unsigned long)tsf/1000000;
				if (copy_to_user((void *)(data->pointer), &tsf_sec, sizeof(unsigned long)))
				{
					HS2_DEBUG_ERR("copy_to_user fail!\n");
					return -1;
				}
				data->length = sizeof(unsigned long);
			}
			break;      
        case DOT11_EVENT_HS2_GAS_RSP:


			issue_GASrsp(priv, data->pointer);
			break;
        case DOT11_EVENT_HS2_TSM_REQ:
			{
			DOT11_BSS_SessInfo_URL sesionInfoURL; 
			DOT11_HS2_TSM_REQ tsmreq;
					
			if (copy_from_user((void *)(&sesionInfoURL),(void *)(data->pointer), data->length))
			{
				DEBUG_ERR("copy_from_user fail!\n");
				return -1;
			}
			
			HS2_DEBUG_INFO("send bss tx mgmt req to STA:");
			MAC_PRINT(sesionInfoURL.macAddr);
			
			memcpy(tsmreq.MACAddr, sesionInfoURL.macAddr, 6);
			memcpy(priv->pmib->hs2Entry.sta_mac, sesionInfoURL.macAddr, 6); // used in dissassoc timer

            //tsmreq.Req_mode = 16; // ESS Disassoc Imminent
			/*HS2 R2 logo test ; ESS Disassoc Imminent  , test plan 4.12*/ 
            tsmreq.Req_mode = 0x14; //

			tsmreq.term_len = 0;

			tsmreq.Dialog_token = 5;
			tsmreq.Disassoc_timer = sesionInfoURL.SWT;
			tsmreq.Validity_intval = 0;	
			tsmreq.url_len = strlen(sesionInfoURL.URL);
			memcpy(tsmreq.Session_url, sesionInfoURL.URL, tsmreq.url_len);
			tsmreq.list_len = 0;	



            issue_BSS_TSM_req(priv, &tsmreq);

			HS2_DEBUG_INFO("issue_BSS_TSM_req and set timer to disassoc[%d]ms\n",(tsmreq.Disassoc_timer * 60 * 1000));
						
           	if (timer_pending(&priv->disassoc_timer))
        		del_timer_sync(&priv->disassoc_timer);            
			mod_timer(&priv->disassoc_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(tsmreq.Disassoc_timer * 60 * 1000));
			}
			break;
        case DOT11_EVENT_HS2_GET_RSN:
			{
				struct wifi_mib *pmib;
				pmib = GET_MIB(priv);
				if (pmib->dot11RsnIE.rsnielen) {                                
					if (copy_to_user((void *)(data->pointer), pmib->dot11RsnIE.rsnie, pmib->dot11RsnIE.rsnielen))
					{
						HS2_DEBUG_ERR("copy_to_user fail!\n");
						return -1;
					}
					data->length = pmib->dot11RsnIE.rsnielen;
				}
			}
			break;
        case DOT11_EVENT_HS2_GET_MMPDULIMIT:
            {
				struct wifi_mib *pmib;
				pmib = GET_MIB(priv);
				if (copy_to_user((void *)(data->pointer), (void *)(&pmib->hs2Entry.mmpdu_limit), sizeof(unsigned int)))
				{
					HS2_DEBUG_ERR("copy_to_user fail!\n");
					return -1;
				}
				data->length = sizeof(unsigned int);
            }
            break;
		case DOT11_EVENT_WNM_NOTIFY: // HS2.0 (case 4.10)
			{
				struct wifi_mib *pmib;
				int i;
				DOT11_WNM_NOTIFY WNM_Notify;
					
				pmib = GET_MIB(priv);
				if (copy_from_user((void *)(&WNM_Notify),(void *)(data->pointer), data->length))
				{
					DEBUG_ERR("copy_from_user fail!\n");
					return -1;
				}
				strcpy(pmib->hs2Entry.remedSvrURL,WNM_Notify.remedSvrURL);

				pmib->hs2Entry.serverMethod = WNM_Notify.serverMethod;
				memcpy(pmib->hs2Entry.sta_mac,WNM_Notify.macAddr,6);
				HS2_DEBUG_INFO("WNM Dest=");
				MAC_PRINT(pmib->hs2Entry.sta_mac);				
				HS2_DEBUG_INFO("Server URL = %s\n",pmib->hs2Entry.remedSvrURL);
				issue_WNM_Notify(priv);				
			}
			break;
		case DOT11_EVENT_WNM_DEAUTH_REQ: // HS2.0 (case 4.13)
			{
				struct wifi_mib *pmib;
				int i;
				DOT11_WNM_DEAUTH_REQ WNM_Deauth;
					
				pmib = GET_MIB(priv);
				if (copy_from_user((void *)(&WNM_Deauth),(void *)(data->pointer), data->length))
				{
					DEBUG_ERR("copy_from_user fail!\n");
					return -1;
				}
				HS2_DEBUG_INFO("WNM Deauth da=");
				MAC_PRINT(WNM_Deauth.macAddr);
				HS2_DEBUG_INFO("reason=%d, reAuthDelay=%d, WNM URL=%s\n",WNM_Deauth.reason, WNM_Deauth.reAuthDelay,WNM_Deauth.URL);
				
				issue_WNM_Deauth_Req(priv,WNM_Deauth.macAddr,WNM_Deauth.reason,WNM_Deauth.reAuthDelay,WNM_Deauth.URL);
			}
			break;
		case DOT11_EVENT_QOS_MAP_CONF:
			{
				struct wifi_mib *pmib;
				int i;
				DOT11_QoSMAPConf QoSMAPConf;
					
				pmib = GET_MIB(priv);
				if (copy_from_user((void *)(&QoSMAPConf),(void *)(data->pointer), data->length))
				{
					HS2_DEBUG_ERR("copy_from_user fail!\n");
					return -1;
				}
				if(QoSMAPConf.indexQoSMAP > 1)
				{
					HS2_DEBUG_ERR("indexQoSMAP > 1!\n");
					return -1;
				}
				HS2_DEBUG_INFO("QoSMAPConf da=");
				MAC_PRINT(QoSMAPConf.macAddr);				
				HS2_DEBUG_INFO("QoSMAP index=%d\n",QoSMAPConf.indexQoSMAP);

				memcpy(priv->pmib->hs2Entry.sta_mac,QoSMAPConf.macAddr,6);
				priv->pmib->hs2Entry.curQoSMap = QoSMAPConf.indexQoSMAP;
				setQoSMapConf(priv);
				issue_QoS_MAP_Configure(priv, QoSMAPConf.macAddr, QoSMAPConf.indexQoSMAP);
			}
			break;
#endif // HS2_SUPPORT
#ifdef CONFIG_IEEE80211W
		/*HS2 R2 logo test*/ 
        case DOT11_EVENT_INIT_PMF:
        {
				int i;
				DOT11_INIT_11W_Flags flags;				
				if (copy_from_user((void *)(&flags),(void *)(data->pointer), data->length))
				{
					PMFDEBUG("copy_from_user fail!\n");
					return -1;
				}

                PMFDEBUG("init pmf:11W[%d],SHA256[%d]\n",flags.dot11IEEE80211W, flags.dot11EnableSHA256);
			
				priv->pmib->dot1180211AuthEntry.dot11IEEE80211W = flags.dot11IEEE80211W;
                priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 = flags.dot11EnableSHA256;

				if(priv->pmib->dot1180211AuthEntry.dot11IEEE80211W == 0 && priv->pmib->dot1180211AuthEntry.dot11EnableSHA256) {
					priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 = 0;
					panic_printk("disable SHA256 when PMF is disable\n");
				}
        }
		case DOT11_EVENT_SET_PMF:
			if(!DOT11_PMF_set_ie(dev, data)) {};
				break;
		case DOT11_EVENT_GET_IGTK_PN:
			{
				struct wifi_mib *pmib;
				int i;
				unsigned char *tt;
				pmib = GET_MIB(priv);			

                #ifdef PMF_DEBUGMSG
				PMFDEBUG("get IGTK_PN:");
				tt = (unsigned char *) & pmib->dot11IGTKTable.dot11EncryptKey.dot11TXPN48;
				for (i=0;i<sizeof(unsigned long long);i++){
					panic_printk("%02X ",tt[i]);
				}
				panic_printk("\n");
                #if 0
				PMFDEBUG("dot11TXPN48.val48=%x %x %x %x %x %x\n",pmib->dot11IGTKTable.dot11EncryptKey.dot11TXPN48._byte_.TSC0
													,pmib->dot11IGTKTable.dot11EncryptKey.dot11TXPN48._byte_.TSC1
													,pmib->dot11IGTKTable.dot11EncryptKey.dot11TXPN48._byte_.TSC2
													,pmib->dot11IGTKTable.dot11EncryptKey.dot11TXPN48._byte_.TSC3
													,pmib->dot11IGTKTable.dot11EncryptKey.dot11TXPN48._byte_.TSC4
													,pmib->dot11IGTKTable.dot11EncryptKey.dot11TXPN48._byte_.TSC5);
                #endif             
                #endif

				if (copy_to_user((void *)(data->pointer), (void *) (&pmib->dot11IGTKTable.dot11EncryptKey.dot11TXPN48.val48), sizeof(pmib->dot11IGTKTable.dot11EncryptKey.dot11TXPN48.val48)))
				{
					DEBUG_ERR("copy_to_user fail!\n");
					return -1;
				}
				data->length = sizeof(pmib->dot11IGTKTable.dot11EncryptKey.dot11TXPN48.val48);
			
			}
			break;
		case DOT11_EVENT_SA_QUERY_RSP:
			{
				DOT11_SA_QUERY_RSP resp;
				if (copy_from_user((void *)(&resp),(void *)(data->pointer), data->length))
				{
					DEBUG_ERR("copy_from_user fail!\n");
					return -1;
				}
				issue_SA_Query_Rsp(dev, resp.MACAddr, resp.trans_id);
				}
				break;
#endif // CONFIG_IEEE80211W

#ifdef CONFIG_IEEE80211R
		case DOT11_EVENT_FT_GET_EVENT:
			{
				if((ret = DOT11_DeQueue((unsigned long)priv, priv->pftevent_queue, QueueData, &QueueDataLen)) != 0)
				{
					val8 = DOT11_EVENT_NO_EVENT;
					if (copy_to_user((void *)((unsigned long)(data->pointer)), &val8, 1)) {
						DEBUG_ERR("copy_to_user fail!\n");
						return -1;
					}
					val8 = 0;
					if (copy_to_user((void *)((unsigned long)(data->pointer) + 1), &val8, 1)) {
						DEBUG_ERR("copy_to_user fail!\n");
						return -1;
					}
					data->length = sizeof(DOT11_NO_EVENT);
				}
				else
				{
					QueueData[1] = (priv->pftevent_queue->NumItem != 0)? 1 : 0;
					if (copy_to_user((void *)data->pointer, (void *)QueueData, QueueDataLen)) {
						DEBUG_ERR("copy_to_user fail!\n");
						return -1;
					}
					data->length = QueueDataLen;
				}
			}
			break;

		case DOT11_EVENT_FT_GET_KEY:
			{
				DOT11_FT_GET_KEY_PROTO proto;
				struct r1_key_holder *r1_key, out;
				struct r0_key_holder *r0_key;
				
				if (copy_from_user((void *)(&proto),(void *)(data->pointer), sizeof(DOT11_FT_GET_KEY_PROTO)))
				{
					DEBUG_ERR("copy_from_user fail!\n");
					return -1;
				}

				switch (proto.Type)
				{
					case FTKEY_TYPE_PUSH:
						{
							DOT11_FT_GET_KEY getkey;
							DOT11_FT_GET_KEY_PUSH push;

							if (copy_from_user((void *)(&getkey),(void *)(data->pointer), data->length))
							{
								DEBUG_ERR("copy_from_user fail!\n");
								return -1;
							}

							r1_key = search_r1kh(priv, getkey.s1kh_id, getkey.r1kh_id);
							if (!r1_key) {
								if (derive_r1_key(priv, getkey.s1kh_id, getkey.r1kh_id, &out)) {
									DEBUG_ERR("Can't derive PMK-R1 key\n");
									return -1;
								}
								r1_key = &out;
							}

							push.EventId = getkey.EventId;
							push.IsMoreEvent = 0;
							push.Type = getkey.Type;
							push.Length = sizeof(DOT11_FT_GET_KEY_PUSH) - offsetof(DOT11_FT_GET_KEY_PUSH, timestamp);
							push.timestamp = 0;
							memcpy(push.r1kh_id, getkey.r1kh_id, MACADDRLEN);
							memcpy(push.s1kh_id, getkey.s1kh_id, MACADDRLEN);
							memcpy(push.pmk_r0_name, r1_key->pmk_r0_name, PMKID_LEN);
							memcpy(push.pmk_r1, r1_key->pmk_r1, PMK_LEN);
							memcpy(push.pmk_r1_name, r1_key->pmk_r1_name, PMKID_LEN);
							push.pairwise = cpu_to_le16(r1_key->pairwise);

							if (copy_to_user((void *)(data->pointer), (void *)(&push), sizeof(DOT11_FT_GET_KEY_PUSH)))
							{
								DEBUG_ERR("copy_to_user fail!\n");
								return -1;
							}
							data->length = sizeof(DOT11_FT_GET_KEY_PUSH);
						}
						break;
					case FTKEY_TYPE_PULL:
						{
							DOT11_FT_PULL_KEY_IND keyind;
							DOT11_FT_GET_KEY_PULL pull;

							if (copy_from_user((void *)(&keyind),(void *)(data->pointer), data->length))
							{
								DEBUG_ERR("copy_from_user fail!\n");
								return -1;
							}
							r0_key = search_r0kh(priv, keyind.s1kh_id);
							if (!r0_key) {
								DEBUG_ERR("No PMK-R0 for this STA\n");
								return -1;
							}
							r1_key = search_r1kh(priv, keyind.s1kh_id, BSSID);
							if (!r1_key) {
								DEBUG_ERR("No PMK-R1 for this STA\n");
								return -1;
							}

							if (memcmp(r1_key->pmk_r0_name, keyind.pmk_r0_name, PMKID_LEN)) {
								DEBUG_ERR("PMK-R1-Name mismatch\n");
								return -1;
							}
							
							if (derive_r1_key(priv, keyind.s1kh_id, keyind.r1kh_id, &out)) {
								DEBUG_ERR("Can't derive PMK-R1 key\n");
								return -1;
							}
							r1_key = &out;

							pull.EventId = keyind.EventId;
							pull.IsMoreEvent = 0;
							pull.Type = keyind.Type;
							pull.Length = sizeof(DOT11_FT_GET_KEY_PULL) - offsetof(DOT11_FT_GET_KEY_PULL, nonce);
							memset(pull.nonce, 0, sizeof(pull.nonce));
							memcpy(pull.r1kh_id, keyind.r1kh_id, MACADDRLEN);
							memcpy(pull.s1kh_id, keyind.s1kh_id, MACADDRLEN);
							memcpy(pull.pmk_r1, r1_key->pmk_r1, PMK_LEN);
							memcpy(pull.pmk_r1_name, r1_key->pmk_r1_name, PMKID_LEN);
							pull.pairwise = cpu_to_le16(r1_key->pairwise);

							if (copy_to_user((void *)(data->pointer), (void *)(&pull), sizeof(DOT11_FT_GET_KEY_PULL)))
							{
								DEBUG_ERR("copy_to_user fail!\n");
								return -1;
							}
							data->length = sizeof(DOT11_FT_GET_KEY_PULL);
						}
						break;
					default:
						DEBUG_ERR("DOT11_EVENT_FT_GET_KEY with wrong type (%d)\n", proto.Type);
						return -1;
				}
			}
			break;

		case DOT11_EVENT_FT_SET_KEY:
			{
				DOT11_FT_SET_KEY_PROTO proto;
				int bset = 0;
				
				if (copy_from_user((void *)(&proto),(void *)(data->pointer), sizeof(DOT11_FT_SET_KEY_PROTO)))
				{
					DEBUG_ERR("copy_from_user fail!\n");
					return -1;
				}

				switch (proto.Type)
				{
					case FTKEY_TYPE_PUSH:
					{
						DOT11_FT_SET_KEY_PUSH setkey;
						DOT11_FT_SET_KEY_PUSH *pkey = &setkey;

						if (copy_from_user((void *)pkey,(void *)(data->pointer), data->length))
						{
							DEBUG_ERR("copy_from_user fail!\n");
							return -1;
						}

						pkey->pairwise = le16_to_cpu(pkey->pairwise);

						// The key is pushed from another AP, it means the STA is associated with the AP unsing
						// initial mobility domain association. The R0KH associated with the STA should be removed
						// from current AP.
						remove_r0kh(priv, pkey->s1kh_id);
						
						bset = store_r1kh(priv, pkey->s1kh_id, pkey->r1kh_id, NULL, 0, 
							pkey->pmk_r1, pkey->pmk_r1_name, pkey->pmk_r0_name, pkey->pairwise);
						if (bset) {
							DEBUG_ERR("fail to store r1kh\n");
							return -1;
						}
						break;
					}
					case FTKEY_TYPE_PULL:
					{
						DOT11_FT_SET_KEY_PULL setkey;
						DOT11_FT_SET_KEY_PULL *pkey = &setkey;

						if (copy_from_user((void *)pkey,(void *)(data->pointer), data->length))
						{
							DEBUG_ERR("copy_from_user fail!\n");
							return -1;
						}

						pkey->pairwise = le16_to_cpu(pkey->pairwise);
						struct stat_info *pstat;
						unsigned char *pos, resp[MAX_FTACTION_LEN];
						unsigned int ielen;
						struct ft_action_param param;
						struct r1_key_holder *r1kh;

						bset = store_r1kh(priv, pkey->s1kh_id, pkey->r1kh_id, NULL, 0, 
							pkey->pmk_r1, pkey->pmk_r1_name, NULL, pkey->pairwise);
						if (bset) {
							DEBUG_ERR("fail to store r1kh\n");
							return -1;
						}

						pstat = get_stainfo(priv, pkey->s1kh_id);
						if (pstat && 
								(pstat->ft_state == state_ft_auth) && 
								(pstat->wpa_sta_info->r1kh == NULL)) {
							r1kh = pstat->wpa_sta_info->r1kh = search_r1kh(priv, pkey->s1kh_id, pkey->r1kh_id);
							if (r1kh) {
								memcpy(r1kh->r0kh_id, pstat->wpa_sta_info->cache_r0kh_id, pstat->wpa_sta_info->cache_r0kh_id_len);
								r1kh->r0kh_id_len = pstat->wpa_sta_info->cache_r0kh_id_len;
								derive_ft_keys(priv, pstat);
								
								if (pstat->wpa_sta_info->over_ds) {
									pos = resp;
									*pos++ = _FAST_BSS_TRANSITION_CATEGORY_ID_;
									*pos++ = _FT_RESPONSE_ACTION_ID_;
									memcpy(pos, pkey->s1kh_id, MACADDRLEN);
									pos += MACADDRLEN;
									memcpy(pos, pkey->r1kh_id, MACADDRLEN);
									pos += MACADDRLEN;
									*((unsigned short *)pos) = cpu_to_le16(_STATS_SUCCESSFUL_);
									pos += 2;
									
									if (pstat->wpa_sta_info->isFT) {
										pos = set_ft_rsnie_with_pmkid(priv, pos, &ielen, pstat, 2);
									}
									pos = construct_mobility_domain_ie(priv, pos, &ielen);
									if (pstat->wpa_sta_info->isFT)
										pos = construct_fast_bss_transition_ie(priv, pos, &ielen, pstat);

									param.action_code = ACTION_CODE_RESPONSE;
									memcpy(param.target_addr, pstat->wpa_sta_info->current_ap, MACADDRLEN);
									param.length = pos - resp;
									param.pdata = resp;
									
									FT_IndicateEvent(priv, pstat, DOT11_EVENT_FT_ACTION_IND, &param);								
								}
								else {
									issue_auth(priv, pstat, (unsigned short)(_STATS_SUCCESSFUL_));
								}
							}
						}
						break;
					}
					default:
						DEBUG_ERR("DOT11_EVENT_FT_SET_KEY with wrong type (%d)\n", proto.Type);
						break;
				}
			}
			break;

		case DOT11_EVENT_FT_ACTION_IND:
			{
				DOT11_FT_ACTION action;
				unsigned char *pframe, *sta_addr, *ap_addr, *ies;
				struct stat_info *pstat;
				unsigned int status = _STATS_SUCCESSFUL_;
				unsigned char *pos, resp[MAX_FTACTION_LEN];
				unsigned int ielen = 0;
				struct ft_action_param param;
				unsigned char category, action_code;
				unsigned long flags;
				
				if (copy_from_user((void *)(&action),(void *)(data->pointer), data->length))
				{
					DEBUG_ERR("copy_from_user fail!\n");
					return -1;
				}

				pframe = action.packet;
				category = *pframe;
				action_code = *(pframe + 1);
				sta_addr = pframe + 2;
				ap_addr = pframe + 8;
				ies = pframe + 14;

				if (category != _FAST_BSS_TRANSITION_CATEGORY_ID_) {
					DEBUG_ERR("Not supported FT Action category id : %d\n", pframe[0]);
					return -1;
				}
				
				SAVE_INT_AND_CLI(flags);
				
				if (action_code == _FT_REQUEST_ACTION_ID_) {
					pstat = get_stainfo(priv, sta_addr);
					if (pstat == NULL) {
						pstat = alloc_stainfo(priv, sta_addr, -1);
						if (pstat == NULL)
						{
							DEBUG_ERR("Exceed the upper limit of supported clients...\n");
							RESTORE_INT(flags);
							return -1;
						}
					}

					if (pstat->ft_auth_expire_to) {
							RESTORE_INT(flags);
							return 0;
					}
					
					pstat->state = WIFI_AUTH_SUCCESS;
					pstat->auth_seq = 0;
					if (priv->pmib->dot11FTEntry.dot11FTReassociationDeadline)
						pstat->expire_to = priv->pmib->dot11FTEntry.dot11FTReassociationDeadline;
					else
						pstat->expire_to = MAX_FTREASSOC_DEADLINE;
					pstat->AuthAlgrthm = 2;
					pstat->ft_state = state_ft_auth;
					pstat->ft_auth_expire_to = 3;
					pstat->tpcache_mgt = 0xffff;

					if (ft_check_ft_auth(priv, pstat, ies, action.packet_len - 14, &status)) {
						free_stainfo(priv, pstat);
						pstat = NULL;
						DEBUG_ERR("ft_check_ft_auth failed (%d)\n", status);
						RESTORE_INT(flags);
						return -1;
					}

					auth_list_add(priv, pstat);

					if (!pstat->wpa_sta_info->r1kh) {
						pstat->wpa_sta_info->over_ds = 1;
						memcpy(pstat->wpa_sta_info->current_ap, action.MACAddr, MACADDRLEN);
						printk("Delay action response due to poll key\n");
						RESTORE_INT(flags);
						return 0;
					}
					
					pos = resp;
					*pos++ = _FAST_BSS_TRANSITION_CATEGORY_ID_;
					*pos++ = _FT_RESPONSE_ACTION_ID_;
					memcpy(pos, sta_addr, MACADDRLEN);
					pos += MACADDRLEN;
					memcpy(pos, ap_addr, MACADDRLEN);
					pos += MACADDRLEN;
					*((unsigned short *)pos) = cpu_to_le16(_STATS_SUCCESSFUL_);
					pos += 2;
					
					if (pstat->wpa_sta_info->isFT) {
						pos = set_ft_rsnie_with_pmkid(priv, pos, &ielen, pstat, 2);
					}
					pos = construct_mobility_domain_ie(priv, pos, &ielen);
					if (pstat->wpa_sta_info->isFT)
						pos = construct_fast_bss_transition_ie(priv, pos, &ielen, pstat);
					
					param.action_code = ACTION_CODE_RESPONSE;
					memcpy(param.target_addr, action.MACAddr, MACADDRLEN);
					param.length = pos - resp;
					param.pdata = resp;
					
					FT_IndicateEvent(priv, pstat, DOT11_EVENT_FT_ACTION_IND, &param);
				}
				else if (action_code == _FT_RESPONSE_ACTION_ID_) {
					pstat = get_stainfo(priv, sta_addr);
					if (pstat) {
						issue_ft_action(priv, pstat, pframe, action.packet_len);
					}
				}
				else if (action_code == _FT_CONFIRM_ACTION_ID_) {

				}
				else if (action_code == _FT_ACK_ACTION_ID_) {

				}
				else {
					DEBUG_ERR("Not supported FT Action id : %d\n", pframe[1]);
					RESTORE_INT(flags);
					return -1;
				}

				RESTORE_INT(flags);
			}
			break;

		case DOT11_EVENT_FT_ASSOC_IND:
			{
				DOT11_FT_ASSOC_IND ft_assoc;
				unsigned char tmpBuf[20], *addr;

				if (copy_from_user((void *)(&ft_assoc),(void *)(data->pointer), data->length))
				{
					DEBUG_ERR("copy_from_user fail!\n");
					return -1;
				}

				addr = ft_assoc.MACAddr;
						memset(tmpBuf, 0, sizeof(tmpBuf));
						sprintf(tmpBuf, "%02x%02x%02x%02x%02x%02xno", 
							addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
						del_sta(priv, tmpBuf);
			}
						break;

		case DOT11_EVENT_FT_KEY_EXPIRE_IND:
			{
				DOT11_FT_KEY_EXPIRE_IND key_expire;
				unsigned char tmpBuf[20], *addr;
				
				if (copy_from_user((void *)(&key_expire),(void *)(data->pointer), data->length))
				{
					DEBUG_ERR("copy_from_user fail!\n");
					return -1;
				}

				addr = key_expire.MACAddr;
				memset(tmpBuf, 0, sizeof(tmpBuf));
				sprintf(tmpBuf, "%02x%02x%02x%02x%02x%02xyes", 
					addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
				del_sta(priv, tmpBuf);		

				remove_r0kh(priv, addr);
				remove_r1kh(priv, addr, BSSID);
			}
			break;

		case DOT11_EVENT_FT_QUERY_INFO:
			{
				DOT11_QUERY_FT_INFORMATION query_info, out;
				struct stat_info *pstat;
				
				if (copy_from_user((void *)(&query_info),(void *)(data->pointer), data->length))
				{
					DEBUG_ERR("copy_from_user fail!\n");
					return -1;
				}

				pstat = get_stainfo(priv, query_info.sta_addr);
				if (!pstat) {
					DEBUG_ERR("get_stainfo return NULL\n");
					return -1;
				}
				
				memset(&out, 0, sizeof(DOT11_QUERY_FT_INFORMATION));
				
				out.EventId = DOT11_EVENT_FT_QUERY_INFO;
				out.IsMoreEvent = 0;
				memcpy(out.sta_addr, query_info.sta_addr, MACADDRLEN);
				memcpy(out.ssid, SSID, SSID_LEN);
				out.ssid_len = SSID_LEN;
				memcpy(out.mdid, MDID, 2);
				memcpy(out.r0kh_id, R0KH_ID, R0KH_ID_LEN);
				out.r0kh_id_len = R0KH_ID_LEN;
				memcpy(out.bssid, BSSID, MACADDRLEN);
				out.over_ds = priv->pmib->dot11FTEntry.dot11FTOverDSEnabled;
				out.res_request = priv->pmib->dot11FTEntry.dot11FTResourceRequestSupported;

				if (copy_to_user((void *)(data->pointer), (void *)(&out), sizeof(DOT11_QUERY_FT_INFORMATION)))
				{
					DEBUG_ERR("copy_to_user fail!\n");
					return -1;
				}
				data->length = sizeof(DOT11_QUERY_FT_INFORMATION);
			}
			break;

		case DOT11_EVENT_FT_SET_INFO:
			{
				DOT11_SET_FT_INFORMATION set_info;
				struct stat_info *pstat;
				
				if (copy_from_user((void *)(&set_info),(void *)(data->pointer), data->length))
				{
					DEBUG_ERR("copy_from_user fail!\n");
					return -1;
				}
				
				pstat = get_stainfo(priv, set_info.sta_addr);
				if (!pstat) {
					DEBUG_ERR("get_stainfo return NULL\n");
					return -1;
				}

				pstat->wpa_sta_info->UnicastCipher_1x = set_info.UnicastCipher;
				pstat->wpa_sta_info->MulticastCipher_1x = set_info.MulticastCipher;

				if (set_info.bInstallKey) {
					pstat->wpa_sta_info->UnicastCipher = pstat->wpa_sta_info->UnicastCipher_1x;
					priv->wpa_global_info->MulticastCipher = pstat->wpa_sta_info->MulticastCipher_1x;
					install_ft_keys(priv, pstat);
				}
			}
			break;
			
		case DOT11_EVENT_FT_AUTH_INSERT_R0:
			{
				DOT11_AUTH_FT_INSERT_R0_KEY insert_r0;
				struct stat_info *pstat;

				if (copy_from_user((void *)(&insert_r0),(void *)(data->pointer), data->length))
				{
					DEBUG_ERR("copy_from_user fail!\n");
					return -1;
				}
				
				pstat = get_stainfo(priv, insert_r0.sta_addr);
				if (!pstat) {
					DEBUG_ERR("Insert R0 key failed, no pstat\n");
					return -1;
				}
				
				if (store_r0kh(priv, insert_r0.sta_addr, insert_r0.pmk_r0, insert_r0.pmk_r0_name)) {
					DEBUG_ERR("store_r0kh failed\n");
					return -1;
				}				
			}
			break;

		case DOT11_EVENT_FT_AUTH_INSERT_R1:
			{
				DOT11_AUTH_FT_INSERT_R1_KEY insert_r1;
				struct stat_info *pstat;

				if (copy_from_user((void *)(&insert_r1),(void *)(data->pointer), data->length))
				{
					DEBUG_ERR("copy_from_user fail!\n");
					return -1;
				}
				
				pstat = get_stainfo(priv, insert_r1.sta_addr);
				if (!pstat) {
					DEBUG_ERR("Insert R1 key failed, no pstat\n");
					return -1;
				}
				
				if (store_r1kh(priv, insert_r1.sta_addr, insert_r1.bssid, 
					insert_r1.r0kh_id, insert_r1.r0kh_id_len, insert_r1.pmk_r1,
					insert_r1.pmk_r1_name, insert_r1.pmk_r0_name, insert_r1.pairwise)) {
					DEBUG_ERR("store_r1kh failed\n");
					return -1;
				}
			}
			break;

		case DOT11_EVENT_FT_TRIGGER_EVENT:
			{
				DOT11_AUTH_FT_TRIGGER_EVENT in_evt;
				struct stat_info *pstat;
		
				if (copy_from_user((void *)(&in_evt),(void *)(data->pointer), data->length))
				{
					DEBUG_ERR("copy_from_user fail!\n");
					return -1;
				}				

				pstat = get_stainfo(priv, in_evt.sta_addr);
				if (!pstat) {
					DEBUG_ERR("trigger event error, no pstat\n");
					return -1;
				}	

				if (in_evt.trigger_eventid == DOT11_EVENT_FT_IMD_ASSOC_IND) {
					FT_IndicateEvent(priv, pstat, DOT11_EVENT_FT_IMD_ASSOC_IND, NULL);

					// Set key expire timer
					if (priv->pmib->dot11FTEntry.dot11FTR0KeyLifetime) {
						set_r0key_expire_time(priv, pstat->hwaddr, priv->pmib->dot11FTEntry.dot11FTR0KeyLifetime * 60);
					}	
				}
				else {
					DEBUG_ERR("Not supported event type : %d\n", in_evt.trigger_eventid);
					return -1;
				}
			}
			break;
#endif
#ifdef RSSI_MONITOR_NCR
		case DOT11_EVENT_RSSI_MONITOR_REPORT:
			{
				int more_data=0;
#ifdef MBSSID
				int i;
#endif
				priv = GET_ROOT(priv);
				ret = DOT11_DeQueue((unsigned long)priv, priv->rssimEvent_queue, QueueData, &QueueDataLen);
				if(ret != 0)
				{
#ifdef UNIVERSAL_REPEATER
					if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))	{
						ret = DOT11_DeQueue((unsigned long)priv, GET_VXD_PRIV(priv)->rssimEvent_queue, QueueData, &QueueDataLen);						
					}
#endif
#ifdef MBSSID
					if (priv->pmib->miscEntry.vap_enable) 		{
						for (i=0; i<RTL8192CD_NUM_VWLAN && (ret != 0); i++) {
							if (IS_DRV_OPEN(priv->pvap_priv[i])) {
								ret = DOT11_DeQueue((unsigned long)priv, priv->pvap_priv[i]->rssimEvent_queue, QueueData, &QueueDataLen);	
							}
						}
					}
#endif								
				}			
				if(ret != 0) {
					QueueDataLen = sizeof(rssim_msg);				
					memset(QueueData, 0, QueueDataLen);				
				} else {
					if(!DOT11_IsEmptyQueue(priv->rssimEvent_queue))
						more_data++;
#ifdef UNIVERSAL_REPEATER
						if ((more_data==0) && IS_DRV_OPEN(GET_VXD_PRIV(priv)))	{
							if(!DOT11_IsEmptyQueue(GET_VXD_PRIV(priv)->rssimEvent_queue))
								more_data++;
						}
#endif
#ifdef MBSSID
						if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) 		{
							for (i=0; i<RTL8192CD_NUM_VWLAN && (more_data == 0); i++) {
								if (IS_DRV_OPEN(priv->pvap_priv[i])) {
									if(!DOT11_IsEmptyQueue(priv->pvap_priv[i]->rssimEvent_queue))
										more_data++;
								}
							}
						}
#endif								
				}
				if (copy_to_user((void *)data->pointer, (void *)QueueData, QueueDataLen)) {
					DEBUG_ERR("copy_to_user fail!\n");	
					return -1;
				}
				data->length = QueueDataLen;			
				if(more_data)
					rssi_event_indicate(priv);
			}
			break;
			case DOT11_EVENT_RSSI_MONITOR_SETTYPE:
			{
				DOT11_RSSIM_SET_TYPE event;
				struct stat_info	*pstat;
#ifdef MBSSID
				int i;
#endif
				if (copy_from_user((void *)&event, (void *)(data->pointer), data->length)) {
					DEBUG_ERR("copy_from_user error!\n");
					return -1;
				}				
				if(pstat = get_stainfo(priv, event.hwaddr)) {
					pstat->rssim_type = event.type;
					break;
				}
#ifdef UNIVERSAL_REPEATER
				if ( IS_DRV_OPEN(GET_VXD_PRIV(priv)))	{
					if(pstat = get_stainfo(GET_VXD_PRIV(priv), event.hwaddr)) {
						pstat->rssim_type = event.type;
						break;
					}
				}
#endif
#ifdef MBSSID
				if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) 		{
					for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
						if (IS_DRV_OPEN(priv->pvap_priv[i])) {
							if(pstat = get_stainfo(priv->pvap_priv[i], event.hwaddr)) {
								pstat->rssim_type = event.type;
								break;
							}
						}
					}
				}
#endif		
			}
			break;
#endif
#ifdef USER_ADDIE
		case DOT11_EVENT_USER_SETIE:
			{
				if(DOT11_USER_set_ie(dev, data) < 0) {
					DEBUG_ERR("DOT11_USER_set_ie failed\n");
					return -1;
				};
			}
			break;
#endif
#if defined(SUPPORT_UCFGING_LED) 
		case DOT11_EVENT_UCFGING_LED:
			{
				DOT11_SET_UCFGING_LED *SetData = (DOT11_SET_UCFGING_LED *)data->pointer;
				if(LED_Configuring != SetData->State){
					LED_Configuring = SetData->State;
				//panic_printk("LED_Configuring=%d\n", LED_Configuring);
				
					if(LED_Configuring == 1){
						if (timer_pending(&priv->pshare->LED_Timer))
							del_timer_sync(&priv->pshare->LED_Timer);
						//init another timer for specific function	
							StartCFGINGTimer();
						
					}else{
						//del another timer that specific function used	
						if (timer_pending(&LED_TimerCFGING))
							del_timer_sync(&LED_TimerCFGING);
						enable_sw_LED(priv, 1);
					}
				}
 			}
			break;
#endif //SUPPORT_UCFGING_LED
		default:
						DEBUG_ERR("unknown user daemon command\n");
						break;
	} // switch(req->EventId)

	return 0;
}

