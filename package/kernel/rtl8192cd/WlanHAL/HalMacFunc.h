
#ifndef __HALMACFUNC_H__
#define __HALMACFUNC_H__
/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalMacFunc.h
	
Abstract:
	Define MAC function support 
	for Driver
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2015-04-29 Eric            Create.	
--*/


void 
MACFM_software_init(
    struct rtl8192cd_priv *priv
);

void 
MACHAL_version_init(
IN  HAL_PADAPTER Adapter
);

typedef struct _MAC_VERSION_
{
    unsigned char is_MAC_v1;
    unsigned char is_MAC_v2;    
    unsigned char MACHALSupport;        
}MAC_VERSION,*PMAC_VERSION;

typedef enum _MACFun_Support_Ability_Definition
{
	//
	// MAC Func section BIT 0-19
	//
	MAC_FUN_HW_TX_SHORTCUT_REUSE_TXDESC         = BIT0,
	MAC_FUN_HW_TX_SHORTCUT_HDR_CONV             = BIT1,
	MAC_FUN_HW_SUPPORT_EACH_VAP_INT		        = BIT2,
	MAC_FUN_HW_SUPPORT_RELEASE_ONE_PACKET		= BIT3,
	MAC_FUN_HW_HW_FILL_MACID			        = BIT4,
	MAC_FUN_HW_HW_DETEC_POWER_STATE				= BIT5,
	MAC_FUN_HW_SUPPORT_MULTICAST_BMC_ENHANCE	= BIT6,
	MAC_FUN_HW_SUPPORT_TX_AMSDU             	= BIT7,
	MAC_FUN_HW_SUPPORT_H2C_PACKET             	= BIT8,	
    MAC_FUN_HW_SUPPORT_AXI_EXCEPTION           	= BIT9,		
	MAC_FUN_HW_SUPPORT_AP_OFFLOAD               = BIT10,
	
}MACFUN_SUPPORT_ABILITY,*PMACFUN_SUPPORT_ABILITY;

#define IS_SUPPORT_WLAN_HAL_HW_TX_SHORTCUT_REUSE_TXDESC(priv)	\
    (priv->pshare->hal_SupportMACfunction & MAC_FUN_HW_TX_SHORTCUT_REUSE_TXDESC)

#define IS_SUPPORT_WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV(priv)	\
    (priv->pshare->hal_SupportMACfunction & MAC_FUN_HW_TX_SHORTCUT_HDR_CONV)    
    
#define IS_SUPPORT_EACH_VAP_INT(priv)	\
    (priv->pshare->hal_SupportMACfunction & MAC_FUN_HW_SUPPORT_EACH_VAP_INT)

#define IS_SUPPORT_RELEASE_ONE_PACKET(priv)	\
    (priv->pshare->hal_SupportMACfunction & MAC_FUN_HW_SUPPORT_RELEASE_ONE_PACKET)

#define IS_SUPPORT_HW_FILL_MACID(priv)	\
    (priv->pshare->hal_SupportMACfunction & MAC_FUN_HW_HW_FILL_MACID)

#define IS_SUPPORT_HW_DETEC_POWER_STATE(priv)	\
    (priv->pshare->hal_SupportMACfunction & MAC_FUN_HW_HW_DETEC_POWER_STATE)

#define IS_SUPPORT_MULTICAST_BMC_ENHANCE(priv)	\
    (priv->pshare->hal_SupportMACfunction & MAC_FUN_HW_SUPPORT_MULTICAST_BMC_ENHANCE)

#define IS_SUPPORT_TX_AMSDU(priv)	\
    (priv->pshare->hal_SupportMACfunction & MAC_FUN_HW_SUPPORT_TX_AMSDU)

#define IS_SUPPORT_H2C_PACKET(priv)	\
    (priv->pshare->hal_SupportMACfunction & MAC_FUN_HW_SUPPORT_H2C_PACKET)

#define IS_SUPPORT_AXI_EXCEPTION(priv)	\
    (priv->pshare->hal_SupportMACfunction & MAC_FUN_HW_SUPPORT_AXI_EXCEPTION)
#define IS_SUPPORT_AP_OFFLOAD(priv)	\
    (priv->pshare->hal_SupportMACfunction & MAC_FUN_HW_SUPPORT_AP_OFFLOAD)        
#endif //__HALMACFUNC_H__


