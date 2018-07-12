/*
 *  Header file for API-compatible handling routines
 *
 *
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */


#ifndef _8192CD_COMAPI_H_
#define _8192CD_COMAPI_H_
 
#include "./8192cd.h"

typedef struct rtl8192cd_priv	RTL_PRIV;
#define MAX_CONFIG_FILE_SIZE (64*1024) // for 8192, added to 64k
#define MAX_PARAM_BUF_SIZE (1024) // for 8192, added to 20k

#ifdef CONFIG_RTL_COMAPI_CFGFILE

/* Following is a example for PARAMETERs completely compatible to other vendors'
  * configure file - chris 2010/02/01 */
#undef VENDOR_PARAM_COMPATIBLE
#ifdef VENDOR_PARAM_COMPATIBLE

int Set_CountryRegion_Proc (RTL_PRIV *priv, char *arg);
int Set_CountryRegionABand_Proc (RTL_PRIV *priv, char *arg);
int Set_SSID_Proc (RTL_PRIV *priv, char *arg);


struct mib_cfg_func{
	char name[20];	/*mib name*/
	int (*set_proc)(RTL_PRIV *priv, char *arg);
};

static struct mib_cfg_func *TMP_MIBCFG, RTL_SUPPORT_MIBCFG[] = {
	{"CountryRegion",				Set_CountryRegion_Proc},
	{"CountryRegionABand",			Set_CountryRegionABand_Proc},
	{"SSID",						Set_SSID_Proc}
};

int Set_CountryRegion_Proc(RTL_PRIV *priv, char *arg)
{
	int val = simple_strtol(arg, 0 ,10);
	if (DOMAIN_FCC <= val && val <= DOMAIN_MAX ) {
		priv->pmib->dot11StationConfigEntry.dot11RegDomain = val;
		return TRUE;
	} else {
		printk("contry region out of range [%d-%d]\n", DOMAIN_FCC, DOMAIN_MAX);
		return FALSE;
	}
}

int Set_CountryRegionABand_Proc(RTL_PRIV *priv, char *arg)
{
	int val = simple_strtol(arg, 0 ,10);
	priv->pmib->dot11StationConfigEntry.dot11RegDomainABand = val;
	return TRUE;
}

int Set_SSID_Proc(RTL_PRIV *priv, char *arg)
{
	strcpy(priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, arg);
	return TRUE;
}

#else
extern int set_mib(struct rtl8192cd_priv *priv, unsigned char *data);

#endif //VENDOR_PARAM_COMPATIBLE

#endif //CONFIG_RTL_COMAPI_CFGFILE

#ifdef WIFI_WPAS_CLI

#define WPA_IE_ID				0xdd
#define WPA2_IE_ID				0x30
#define RSN_HEADER_LEN				4
#define WPA_SELECTOR_LEN 			4
#define RSN_SELECTOR_LEN			4
#define VENDOR_SPECIFIC_IE			221

#ifdef CONFIG_RTL_WAPI_SUPPORT
#ifndef IW_AUTH_WAPI_VERSION_1
#define IW_AUTH_WAPI_VERSION_1		0x00000008
#endif

#ifndef IW_AUTH_KEY_MGMT_WAPI_PSK
#define IW_AUTH_KEY_MGMT_WAPI_PSK	0x04
#endif

#ifndef IW_AUTH_KEY_MGMT_WAPI_CERT
#define IW_AUTH_KEY_MGMT_WAPI_CERT	0x08
#endif

#ifndef IW_AUTH_WAPI_ENABLED
#define IW_AUTH_WAPI_ENABLED		0x20
#endif

#ifndef IW_ENCODE_ALG_SM4
#define IW_ENCODE_ALG_SM4		0x20
#endif
#endif // CONFIG_RTL_WAPI_SUPPORT
#endif // WIFI_WPAS_CLI

#endif // _8192CD_COMAPI_H_



