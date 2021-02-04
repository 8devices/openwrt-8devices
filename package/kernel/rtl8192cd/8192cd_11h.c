/*
 *  Software TKIP encryption/descryption routines
 *
 *  $Id: 8192cd_tkip.c,v 1.4.4.2 2010/09/30 05:27:28 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_11H_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <asm/byteorder.h>
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
#ifdef __KERNEL__
#include "./ieee802_mib.h"
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wlan/ieee802_mib.h>
#endif
#include "./8192cd_util.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"

#if (defined(DOT11D) || defined(DOT11H) || defined(DOT11K))

#define MAX_CHANNEL_SET_NUMBER	20

typedef struct _PER_CHANNEL_ENTRY_ {
    unsigned char	firstChannel;
    unsigned char	numberOfChannel;
    unsigned char	maxTxDbm;
} PER_CHANNEL_ENTRY;

typedef struct _BAND_TABLE_ELEMENT_ {
    unsigned char  channel_set_number;
    PER_CHANNEL_ENTRY	channel_set[MAX_CHANNEL_SET_NUMBER];	
} BAND_TABLE_ELEMENT;


typedef struct _COUNTRY_IE_ELEMENT_ {
    unsigned int		countryNumber;
    unsigned char 		countryA2[3];
	unsigned char		A_Band_Region;	//if support 5G A band? ;  0 == no support ; aBandRegion == real region domain
	unsigned char		G_Band_Region;	//if support 2.4G G band? ;  0 == no support ; bBandRegion == real region domain
} COUNTRY_IE_ELEMENT;


static const COUNTRY_IE_ELEMENT countryIEArray[] =
{
	/*
	 format: countryNumber | CountryCode(A2) 
	*/
	{8,"AL ",   3, 3},   /*ALBANIA*/
	{12,"DZ ",  3, 3},  /*ALGERIA*/
	{32,"AR ",  3, 3},  /*ARGENTINA*/
	{51,"AM ",  3, 3},  /*ARMENIA*/
	{36,"AU ",  3, 3},  /*AUSTRALIA*/
	{40,"AT ",  3, 3},  /*AUSTRIA*/
	{31,"AZ ",  3, 3},  /*AZERBAIJAN*/
	{48,"BH ",  3, 3},  /*BAHRAIN*/
	{112,"BY",  3, 3},  /*BELARUS*/
	{56,"BE ",  3, 3},  /*BELGIUM*/
	{84,"BZ ",  3, 3},  /*BELIZE*/
	{68,"BO ",  3, 3},  /*BOLIVIA*/
	{76,"BR ",  3, 3},  /*BRAZIL*/
	{96,"BN ",  3, 3},  /*BRUNEI*/
	{100,"BG ", 3, 3}, /*BULGARIA*/
	{124,"CA ", 1, 1}, /*CANADA*/
	{152,"CL ", 3, 3}, /*CHILE*/
	{156,"CN ",13,13}, /*CHINA*/
	{170,"CO ", 1, 1}, /*COLOMBIA*/
	{188,"CR ", 3, 3}, /*COSTA RICA*/
	{191,"HR ", 3, 3}, /*CROATIA*/
	{196,"CY ", 3, 3}, /*CYPRUS*/
	{203,"CZ ", 3, 3}, /*CZECH REPUBLIC*/
	{208,"DK ", 3, 3}, /*DENMARK*/
	{214,"DO ", 1, 1}, /*DOMINICAN REPUBLIC*/
	{218,"EC ", 3, 3}, /*ECUADOR*/
	{818,"EG ", 3, 3}, /*EGYPT*/
	{222,"SV ", 3, 3}, /*EL SALVADOR*/
	{233,"EE ", 3, 3}, /*ESTONIA*/
	{246,"FI ", 3, 3}, /*FINLAND*/
	{250,"FR ", 3, 3}, /*FRANCE*/
	{268,"GE ", 3, 3}, /*GEORGIA*/
	{276,"DE ", 3, 3}, /*GERMANY*/
	{300,"GR ", 3, 3}, /*GREECE*/
	{320,"GT ", 1, 1}, /*GUATEMALA*/
	{340,"HN ", 3, 3}, /*HONDURAS*/
	{344,"HK ", 3, 3}, /*HONG KONG*/
	{348,"HU ", 3, 3}, /*HUNGARY*/
	{352,"IS ", 3, 3}, /*ICELAND*/
	{356,"IN ", 3, 3}, /*INDIA*/
	{360,"ID ", 3, 3}, /*INDONESIA*/
	{364,"IR ", 3, 3}, /*IRAN*/
	{372,"IE ", 3, 3}, /*IRELAND*/
	{376,"IL ", 7, 7}, /*ISRAEL*/
	{380,"IT ", 3, 3}, /*ITALY*/
	{392,"JP ", 6, 6}, /*JAPAN*/
	{400,"JO ", 3, 3}, /*JORDAN*/
	{398,"KZ ", 3, 3}, /*KAZAKHSTAN*/
	{410,"KR ", 3, 3}, /*NORTH KOREA*/
	{408,"KP ", 3, 3}, /*KOREA REPUBLIC*/
	{414,"KW ", 3, 3}, /*KUWAIT*/
	{428,"LV ", 3, 3}, /*LATVIA*/
	{422,"LB ", 3, 3}, /*LEBANON*/
	{438,"LI ", 3, 3}, /*LIECHTENSTEIN*/
	{440,"LT ", 3, 3}, /*LITHUANIA*/
	{442,"LU ", 3, 3}, /*LUXEMBOURG*/
	{446,"MO ", 3, 3}, /*CHINA MACAU*/
	{807,"MK ", 3, 3}, /*MACEDONIA*/
	{458,"MY ", 3, 3}, /*MALAYSIA*/
	{484,"MX ", 1, 1}, /*MEXICO*/
	{492,"MC ", 3, 3}, /*MONACO*/
	{504,"MA ", 3, 3}, /*MOROCCO*/
	{528,"NL ", 3, 3}, /*NETHERLANDS*/
	{554,"NZ ", 3, 3}, /*NEW ZEALAND*/
	{578,"NO ", 3, 3}, /*NORWAY*/
	{512,"OM ", 3, 3}, /*OMAN*/
	{586,"PK ", 3, 3}, /*PAKISTAN*/
	{591,"PA ", 1, 1}, /*PANAMA*/
	{604,"PE ", 3, 3}, /*PERU*/
	{608,"PH ", 3, 3}, /*PHILIPPINES*/
	{616,"PL ", 3, 3}, /*POLAND*/
	{620,"PT ", 3, 3}, /*PORTUGAL*/
	{630,"PR ", 1, 1}, /*PUERTO RICO*/
	{634,"QA ", 3, 3}, /*QATAR*/
	{642,"RO ", 3, 3}, /*ROMANIA*/
	{643,"RU ",12,12}, /*RUSSIAN*/
	{682,"SA ", 3, 3}, /*SAUDI ARABIA*/
	{702,"SG ", 3, 3}, /*SINGAPORE*/
	{703,"SK ", 3, 3}, /*SLOVAKIA*/
	{705,"SI ", 3, 3}, /*SLOVENIA*/
	{710,"ZA ", 3, 3}, /*SOUTH AFRICA*/
	{724,"ES ", 3, 3}, /*SPAIN*/
	{752,"SE ", 3, 3}, /*SWEDEN*/
	{756,"CH ", 3, 3}, /*SWITZERLAND*/
	{760,"SY ", 3, 3}, /*SYRIAN ARAB REPUBLIC*/
	{158,"TW ",11,11}, /*TAIWAN*/
	{764,"TH ", 3, 3}, /*THAILAND*/
	{780,"TT ", 3, 3}, /*TRINIDAD AND TOBAGO*/
	{788,"TN ", 3, 3}, /*TUNISIA*/
	{792,"TR ", 3, 3}, /*TURKEY*/
	{804,"UA ", 3, 3}, /*UKRAINE*/
	{784,"AE ", 3, 3}, /*UNITED ARAB EMIRATES*/
	{826,"GB ", 3, 3}, /*UNITED KINGDOM*/
	{840,"US ", 1, 1}, /*UNITED STATES*/
	{858,"UY ", 3, 3}, /*URUGUAY*/
	{860,"UZ ", 1, 1}, /*UZBEKISTAN*/
	{862,"VE ", 3, 3}, /*VENEZUELA*/
	{704,"VN ", 3, 3}, /*VIET NAM*/
	{887,"YE ", 3, 3}, /*YEMEN*/
	{716,"ZW ", 3, 3}, /*ZIMBABWE*/
};


static const BAND_TABLE_ELEMENT country_ie_channel_2_4g[] = {
    /* number of channel set | array of channel sets{first channel, num of channel,  tx power} 
            transmit tx power is copy from CAMEO
       */
    
    /* (1) FCC */           {1, {{1, 11, 30}}},
    /* (2) IC */		        {1, {{1, 11, 30}}},
    /* (3) ETSI */		    {1, {{1, 13, 30}}},
    /* (4) SPAIN */		    {1, {{1, 13, 30}}},
    /* (5) FRANCE */        {1, {{10, 4, 30}}},
    /* (6) MKK */           {1, {{1, 14, 30}}},
    /* (7) ISRAEL */        {1, {{3, 11, 30}}},
    /* (8) MKK1 */          {1, {{1, 14, 30}}},
    /* (9) MKK2 */          {1, {{1, 14, 30}}},
    /* (10) MKK3 */          {1, {{1, 14, 30}}},
    /* (11) NCC (Taiwan) */  {1, {{1, 11, 30}}},
    /* (12) RUSSIAN */       {1, {{1, 13, 30}}},
    /* (13) CN */            {1, {{1, 13, 30}}},
    /* (14) Global */        {1, {{1, 14, 30}}},
    /* (15) World_wide */    {1, {{1, 13, 30}}},
    /* (16) Test */          {1, {{1, 14, 30}}},
};

static const BAND_TABLE_ELEMENT country_ie_channel_5g[] = {
    /* number of channel set | array of channel sets {first channel, num of channel,  tx power},
            transmit tx power is copy from CAMEO
      */

	/*(1) FCC */		  {20, {{36,1,30}, {40,1,30}, {44,1,30}, {48,1,30},
                            {52,1,30}, {56,1,30}, {60,1,30}, {64,1,30},
                            {100,1,20},{104,1,20},{108,1,20},{112,1,20},{116,1,20},{136,1,20},{140,1,20},
                            {149,1,30},{153,1,30},{157,1,30},{161,1,30},{165,1,30}  }
                      },
	/* (2) IC */		  {12, {{36,1,30}, {40,1,30}, {44,1,30}, {48,1,30},
                            {52,1,30}, {56,1,30}, {60,1,30}, {64,1,30},
                            {149,1,30},{153,1,30},{157,1,30},{161,1,30}  }
                      },                    
	/* (3) ETSI */	  {19, {{36,1,30}, {40,1,30}, {44,1,30}, {48,1,30}, 
	                        {52,1,30}, {56,1,30}, {60,1,30}, {64,1,30}, 
	                        {100,1,20},{104,1,20},{108,1,20},{112,1,20},{116,1,20},{120,1,20},{124,1,20},{128,1,20},{132,1,20},{136,1,20},{140,1,20}  }
                      },
	/* (4) SPAIN */    {3,  {{36,4,30},  //36, 40, 44, 48
	                        {52,4,30},  //52, 56, 60, 64
	                        {100,11,20} } //100, 104, 108, 112, 116,120,124,128,132,136,140
                      },
	/* (5) FRANCE */	  {3,  {{36,4,30},  //36, 40, 44, 48
	                        {52,4,30},  //52, 56, 60, 64
	                        {100,11,20} } //100, 104, 108, 112, 116,120,124,128,132,136,140
                      },
	/* (6) MKK */     {19,  {{36,1,30}, {40,1,30}, {44,1,30}, {48,1,30}, 
	                         {52,1,30}, {56,1,30}, {60,1,30}, {64,1,30}, 
	                         {100,1,20},{104,1,20},{108,1,20},{112,1,20},{116,1,20},{120,1,20},{124,1,20},{128,1,20},{132,1,20},{136,1,20},{140,1,20}  }
                      },
	/* (7) ISRAEL */	  {19, {{36,1,30}, {40,1,30}, {44,1,30}, {48,1,30}, 
	                        {52,1,30}, {56,1,30}, {60,1,30}, {64,1,30}, 
	                        {100,1,30},{104,1,30},{108,1,30},{112,1,30},{116,1,30},{120,1,30},{124,1,30},{128,1,30},{132,1,30},{136,1,30},{140,1,30}  }
                      },
	/* (8) MKK1 */	  {1,  {{34,4,30}  } // 34, 38, 42, 46
                      },
	/* (9) MKK2 */	  {1,  {{36,4,30}  } //36, 40, 44, 48
                      },
	/* (10) MKK3 */	  {2,  {{36,4,30}, //36, 40, 44, 48
	                        {52,4,30}  } //52, 56, 60, 64
                      },
	/* (11) NCC (Taiwan) */	
                      {15, {{56,1,30}, {60,1,30}, {64,1,30}, 
                            {100,1,20},{104,1,20},{108,1,20},{112,1,20},{116,1,20},{136,1,20},{140,1,20},
                            {149,1,30},{153,1,30},{157,1,30},{161,1,30},{165,1,30}  }
                      },
	/* (12) RUSSIAN */{16, {{36,1,20}, {40,1,20}, {44,1,20}, {48,1,20}, 
	                        {52,1,20}, {56,1,20}, {60,1,20}, {64,1,20},
	                        {132,1,30},{136,1,30},{140,1,30},
	                        {149,1,30},{153,1,30},{157,1,30},{161,1,30},{165,1,30}}
                      },
	/* (13) CN */     {13,  {{36,1,30}, {40,1,30}, {44,1,30}, {48,1,30},
                            {52,1,30}, {56,1,30}, {60,1,30}, {64,1,30},
							{149,1,30},{153,1,30},{157,1,30},{161,1,30},{165,1,30}  }
                      },
	/* (14) Global */	  {20, {{36,1,30}, {40,1,30}, {44,1,30}, {48,1,30}, 
	                        {52,1,30}, {56,1,30}, {60,1,30}, {64,1,30}, 
	                        {100,1,30},{104,1,30},{108,1,30},{112,1,30},{116,1,30},{136,1,30},{140,1,30},
	                        {149,1,30},{153,1,30},{157,1,30},{161,1,30},{165,1,30}  }
                      },
	/* (15) World_wide */	
                      {20, {{36,1,30}, {40,1,30}, {44,1,30}, {48,1,30}, 
                            {52,1,30}, {56,1,30}, {60,1,30}, {64,1,30}, 
                            {100,1,30},{104,1,30},{108,1,30},{112,1,30},{116,1,30},{136,1,30},{140,1,30},
                            {149,1,30},{153,1,30},{157,1,30},{161,1,30},{165,1,30}  }
                      },
	/* (16) Test */	  {4,  {{36,4,30}, //36, 40, 44, 48
	                      {52,4,30}, //52, 56, 60, 64
	                      {100,12,30}, //100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144
	                      {149,8,30}  } //149, 153, 157, 161, 165, 169, 173, 177
                      },
	/* (17) 5M10M */  {1, {{146,25,30}  } // 146 ~ 170
                      },
};

void check_country_channel_table(struct rtl8192cd_priv *priv) {

    unsigned char i = 0, country_num;
    priv->countryTableIdx = 0;
    if(COUNTRY_CODE_ENABLED || priv->pmib->dot11hTPCEntry.tpc_enable)
    {
        country_num = sizeof(countryIEArray)/sizeof(COUNTRY_IE_ELEMENT);
        for (i=0; i<country_num; i++) {
            if (!memcmp(priv->pmib->dot11dCountry.dot11CountryString, countryIEArray[i].countryA2, 2)) {
                priv->countryTableIdx = i + 1;
                break;
            }
        }

        if (priv->countryTableIdx == 0) {
            printk("can't found country code(%s)\n", priv->pmib->dot11dCountry.dot11CountryString);
        }
    }
}

unsigned char * construct_country_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen) {
    const COUNTRY_IE_ELEMENT * country_ie;
    const BAND_TABLE_ELEMENT * band_table = NULL;
    unsigned char temp[MAX_CHANNEL_SET_NUMBER*3 + 3 + 1];/*channel sets + country code + 1 possible padding*/
    unsigned int len = 0;
    country_ie = &(countryIEArray[priv->countryTableIdx-1]);
    if ( priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G) {
        if(country_ie->G_Band_Region) {
            band_table = &(country_ie_channel_2_4g[country_ie->G_Band_Region-1]);
        }
    }
    else {                 
        if(country_ie->A_Band_Region) {
            band_table = &(country_ie_channel_5g[country_ie->A_Band_Region-1]);
        }
    }

    if(band_table) {
        memcpy(temp + len, country_ie->countryA2, 3);
        len += 3;

        memcpy(temp + len, (unsigned char *)band_table->channel_set, band_table->channel_set_number*3);
        len += band_table->channel_set_number*3;
        
        /*add padding, the length of country ie must divided by two*/
        if(len%2) { 
            temp[len] = 0;
            len++;
        }
        
        pbuf = set_ie(pbuf, _COUNTRY_IE_, len, temp, frlen);
    }
    return pbuf;
}

unsigned char search_country_txpower(struct rtl8192cd_priv *priv, unsigned char channel)
{
    const COUNTRY_IE_ELEMENT * country_ie;
    const BAND_TABLE_ELEMENT * band_table = NULL;
    int i;
    int step;
    country_ie = &(countryIEArray[priv->countryTableIdx-1]);
    if ( priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G)
    {
        if(country_ie->G_Band_Region)
        {
            band_table = &(country_ie_channel_2_4g[country_ie->G_Band_Region-1]);
            step = 1;
        }
    }
    else
    {
        if(country_ie->A_Band_Region)
        {
            band_table = &(country_ie_channel_5g[country_ie->A_Band_Region-1]);
            step = 4;
        }
    }
    if(band_table)
    {
        for(i = 0; i < band_table->channel_set_number; i++)
        {
            if(band_table->channel_set[i].firstChannel <= channel  &&
                    channel < band_table->channel_set[i].firstChannel + band_table->channel_set[i].numberOfChannel*step)
            {
                return band_table->channel_set[i].maxTxDbm;
            }
        }
    }

    return 0;

}

#endif



#if defined(DOT11H) || defined(DOT11K)
#if defined(CLIENT_MODE) || defined(CONFIG_RTK_MESH) 
unsigned char * construct_power_capability_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen) {
    unsigned char temp[2];
    temp[0] = priv->pmib->dot11hTPCEntry.min_tx_power;
    temp[1] = priv->pmib->dot11hTPCEntry.max_tx_power;       
    pbuf = set_ie(pbuf, _PWR_CAPABILITY_IE_, 2, temp, frlen);
    return pbuf;
}
#endif

unsigned char * construct_TPC_report_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen) {
    unsigned char temp[2];
    temp[0] = priv->pmib->dot11hTPCEntry.tpc_tx_power;
    temp[1] = priv->pmib->dot11hTPCEntry.tpc_link_margin;
    pbuf = set_ie(pbuf, _TPC_REPORT_IE_, 2, temp, frlen);
    return pbuf;
}

#endif

#ifdef DOT11H
#if defined(CLIENT_MODE) || defined(CONFIG_RTK_MESH) 
unsigned char * construct_supported_channel_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen) {
    const COUNTRY_IE_ELEMENT * country_ie;    
    const BAND_TABLE_ELEMENT * band_table = NULL;
    unsigned char temp[MAX_CHANNEL_SET_NUMBER*2];/*channel sets*/
    unsigned int i,j = 0;
    if(priv->countryTableIdx) {
        country_ie = &(countryIEArray[priv->countryTableIdx-1]);
        if ( priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G) {
        if(country_ie->G_Band_Region) {
                band_table = &(country_ie_channel_2_4g[country_ie->G_Band_Region-1]);
            }
        }
        else {                 
            if(country_ie->A_Band_Region) {
                band_table = &(country_ie_channel_5g[country_ie->A_Band_Region-1]);
            }
        }        
        if(band_table) {
            for(i = 0; i < band_table->channel_set_number; i++) {            
                temp[j++] = band_table->channel_set[i].firstChannel;
                temp[j++] = band_table->channel_set[i].numberOfChannel;            
            }     
            pbuf = set_ie(pbuf, _SUPPORTED_CHANNEL_IE_, j, temp, frlen);
        }   
    }
    return pbuf;
}
#endif

void issue_TPC_report(struct rtl8192cd_priv *priv, unsigned char *da, unsigned char dialog_token) {
    unsigned char   *pbuf;
    unsigned int frlen;
    DECLARE_TXINSN(txinsn);

    txinsn.q_num = MANAGE_QUE_NUM;
    txinsn.fr_type = _PRE_ALLOCMEM_;
#ifdef P2P_SUPPORT	// 2013
    if(rtk_p2p_is_enabled(priv)){
        txinsn.tx_rate = _6M_RATE_;
    }else
#endif    
        txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
    
    txinsn.fixed_rate = 1;
    
    pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
    if (pbuf == NULL)
        goto issue_TPC_report_fail;

    txinsn.phdr = get_wlanhdr_from_poll(priv);
    if (txinsn.phdr == NULL)
        goto issue_TPC_report_fail;
    
    memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

    pbuf[0] = _SPECTRUM_MANAGEMENT_CATEGORY_ID_;
    pbuf[1] = _TPC_REPORT_ACTION_ID_;
    pbuf[2] = dialog_token;
    frlen = 3;
        
    construct_TPC_report_ie(priv, pbuf + frlen, &frlen);
    
    txinsn.fr_len += frlen;
    
    SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);

    memcpy((void *)GetAddr1Ptr((txinsn.phdr)), da, MACADDRLEN);
    memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
    memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
    
    if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
        return;
    
issue_TPC_report_fail:

    if (txinsn.phdr)
        release_wlanhdr_to_poll(priv, txinsn.phdr);
    if (txinsn.pframe)
        release_mgtbuf_to_poll(priv, txinsn.pframe);
    return;

}


#endif


