/*
* Copyright (C) 2009 Realtek Semiconductor Corp.
* All Rights Reserved.
*
* This program is the proprietary software of Realtek Semiconductor
* Corporation and/or its licensors, and only be used, duplicated,
* modified or distributed under the authorized license from Realtek.
*
* ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER
* THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*
* $Revision: 8334 $
* $Date: 2010-02-09 13:22:42 +0800 (Tue, 09 Feb 2010) $
*
* Purpose :
*
*  Feature :
*
*/



#ifndef __RTL8316D_ASIC_DRV_H__
#define __RTL8316D_ASIC_DRV_H__


#define INIT_HASH_ALG    0       //hash algorithm
#define RTL8316D_MAX_POLLCNT  1000UL

#define CHK_FUN_RETVAL(f) \
do { \
        if (RT_ERR_OK != (f)) \
            return (RT_ERR_SMI); \
} while(0)


typedef enum rtl8316d_vlan_tagMode_e
{
    RTL8316D_VLAN_TAG_ORIGINAL = 0,
    RTL8316D_VLAN_TAG_KEEP,
    RTL8316D_VLAN_TAG_REAL_KEEP,
    RTL8316D_VLAN_TAG_END
}  rtl8316d_vlan_tagMode_s;

typedef enum rtl8316d_portRole_e
{
   RTL8316D_PORTROLE_C = 0,
   RTL8316D_PORTROLE_S,
   RTL8316D_PORTROLE_END
} rtl8316d_vlan_portRole_s;

typedef struct rtl8316d_phyAddr_s
{
    uint8 phyAddrType; /*0: single address, 1: phy mask*/
    union{
        uint32 address;
        uint32 phyMask;
    }un;

}rtl8316d_phyAddr_t;

typedef struct rtl8316d_phyRegAddr_s
{
    uint8 phyRegType; /*0: clause 22, 1: clause45*/
    union{
        struct{
            uint8 page;     /*5-bit long*/
            uint8 addr;     /*5-bit long*/
        }clause22;

        uint16 mmdAddr;     /*5-bit long*/
    }un;

}rtl8316d_phyRegAddr_t;

extern void rtk_switch_init(void);
extern void rtl8316d_swInfo_MAC_get(uint8* mac);
extern void rtl8316d_sys_reboot(void);

extern rtk_api_ret_t rtl8316d_qos_priMap_get(uint8 port, rtl8316d_primap_param_t* map);
extern rtk_api_ret_t rtl8316d_qos_priMap_set(uint8 port, rtl8316d_primap_param_t* map);

extern rtk_api_ret_t rtl8316d_qos_priextra_get(uint8 port, rtl8316d_priextra_param_t * config);
extern rtk_api_ret_t rtl8316d_qos_priextra_set(uint8 port, rtl8316d_priextra_param_t * config);

extern rtk_api_ret_t rtl8316d_qos_schedAlogrithm_get(uint8 port, uint8 queue, rtl8316d_qos_schedType_t* type);
extern rtk_api_ret_t rtl8316d_qos_schedAlogrithm_set(uint8 port, uint8 queue, rtl8316d_qos_schedType_t type);

extern rtk_api_ret_t rtl8316d_setAsicPortIngressBandwidthEnable(uint32 port,uint32 enable);
extern rtk_api_ret_t rtl8316d_getAsicPortIngressBandwidthEnable(uint32 port,uint32 *enable);
extern rtk_api_ret_t rtl8316d_setAsicPortIngressBandwidth(uint32 port,uint32 rate, uint32 ifg_include);
extern rtk_api_ret_t rtl8316d_getAsicPortIngressBandwidth(uint32 port,uint32 *rate, uint32 *ifg_include);

extern rtk_api_ret_t rtl8316d_setAsicPortEgressBandwidthEnable(uint32 port,uint32 enable);
extern rtk_api_ret_t rtl8316d_getAsicPortEgressBandwidthEnable(uint32 port,uint32 *enable);
extern rtk_api_ret_t rtl8316d_setAsicPortEgressBandwidth(uint32 port,uint32 rate, uint32 ifg_include);
extern rtk_api_ret_t rtl8316d_getAsicPortEgressBandwidth(uint32 port,uint32 *rate, uint32 *ifg_include);

extern rtk_api_ret_t rtl8316d_vlan_portPvid_set(uint32 port, uint32 vid, uint32 pri);
extern rtk_api_ret_t rtl8316d_vlan_portPvid_get(uint32 port, uint32 *pVid, uint32 *pPri);

extern rtk_api_ret_t rtl8316d_vlan_portAcceptFrameType_set(uint32 port, rtk_vlan_acceptFrameType_t accept_frame_type);
extern rtk_api_ret_t rtl8316d_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type);
extern rtk_api_ret_t rtl8316d_vlan_tagMode_set(rtk_port_t port, rtl8316d_vlan_tagMode_s tag_rxc, rtl8316d_vlan_tagMode_s tag_rxs );
extern rtk_api_ret_t rtl8316d_vlan_tagMode_get(rtk_port_t port, rtl8316d_vlan_tagMode_s *pTag_rxC, rtl8316d_vlan_tagMode_s *pTag_rxS);
extern rtk_api_ret_t rtl8316d_vlan_tagAware_set(rtk_port_t port, uint32 enabled);
extern rtk_api_ret_t rtl8316d_vlan_tagAware_get(rtk_port_t port, uint32 *pEnabled);
extern rtk_api_ret_t rtl8316d_vlan_ingFilterEnable_set(rtk_port_t port, uint32 enabled);
extern rtk_api_ret_t rtl8316d_vlan_ingFilterEnable_get(rtk_port_t port, uint32 *pEnabled);
extern rtk_api_ret_t rtl8316d_vlan_portRole_set(rtk_port_t port, rtl8316d_vlan_portRole_s portrole);
extern rtk_api_ret_t rtl8316d_vlan_portRole_get(rtk_port_t port, rtl8316d_vlan_portRole_s *pPortrole);
extern rtk_api_ret_t rtl8316d_vlan_pidEntry_set(uint32 entry, uint32 pid);
extern rtk_api_ret_t rtl8316d_vlan_pidEntry_get(uint32 entry, uint32* pPid);
extern rtk_api_ret_t rtl8316d_vlan_portTpid_set(rtk_port_t port, uint32 tpidmask);
extern rtk_api_ret_t rtl8316d_vlan_portTpid_get(rtk_port_t port, uint32* pTpidmask);
extern rtk_api_ret_t rtl8316d_svlan_portSpid_set(rtk_port_t port, uint32 spidmask);
extern rtk_api_ret_t rtl8316d_svlan_portSpid_get(rtk_port_t port, uint32* pSpidmask);
extern rtk_api_ret_t rtl8316d_svlan_fwdBaseOvid_set(rtk_port_t port, uint32 enabled);
extern rtk_api_ret_t rtl8316d_svlan_fwdBaseOvid_get(rtk_port_t port, uint32 *pEnabled);
extern rtk_api_ret_t rtl8316d_svlan_portPvid_set(uint32 port, uint32 svid, uint32 pri, uint32 dei);
extern rtk_api_ret_t rtl8316d_svlan_portPvid_get(uint32 port, uint32 *pSvid, uint32 *pPri, uint32 *pDei);
extern rtk_api_ret_t rtl8316d_svlan_portAcceptFrameType_set(uint32 port, rtk_vlan_acceptFrameType_t accept_frame_type);
extern rtk_api_ret_t rtl8316d_svlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type);
extern rtk_api_ret_t rtl8316d_svlan_tagMode_set(rtk_port_t port, rtl8316d_vlan_tagMode_s tag_rxc, rtl8316d_vlan_tagMode_s tag_rxs );
extern rtk_api_ret_t rtl8316d_svlan_tagMode_get(rtk_port_t port, rtl8316d_vlan_tagMode_s *pTag_rxc, rtl8316d_vlan_tagMode_s *pTag_rxs);
extern rtk_api_ret_t rtl8316d_svlan_tagAware_set(rtk_port_t port, uint32 enabled);
extern rtk_api_ret_t rtl8316d_svlan_tagAware_set(rtk_port_t port, uint32 enabled);

extern rtk_api_ret_t rtl8316d_setAsicLutIpMulticastLookup(uint32 enabled);
extern rtk_api_ret_t rtl8316d_setAsicLutAgeTimerSpeed(uint32 ageUnit);

extern rtk_api_ret_t rtl8316d_getAsicMIBCounter(uint16 mibAddr, uint32* counterH, uint32* counterL);
extern rtk_api_ret_t rtl8316d_setAsicMIBsCounterReset(uint32 greset, uint32 qmreset, uint32 pmask);
extern rtk_api_ret_t rtl8316d_getPortMIBAddr(rtk_stat_port_type_t cntr_idx, uint8 port, uint16* mibAddr);

extern rtk_api_ret_t rtl8316d_jumbo_size_set(uint32 maxLength);
extern rtk_api_ret_t rtl8316d_jumbo_size_get(uint32* maxLength);

extern rtk_api_ret_t rtl8316d_setAsicLutIpMulticastLookup(uint32 enabled);
extern rtk_api_ret_t rtl8316d_setAsicLutAgeTimerSpeed(uint32 ageUnit);
extern rtk_api_ret_t rtl8316d_l2_flushType_set(rtl8316d_l2_flushType_t type, rtk_vlan_t vid, rtl8316d_l2_flushItem_t key);
extern int8 rtl8316d_fwd_delAsicEntry(uint16 index);
extern rtk_api_ret_t rtl8316d_iso_mask_get(uint32 *mask);
extern rtk_api_ret_t rtl8316d_iso_mask_set(uint32 mask);

extern rtk_api_ret_t rtl8316d_mirror_portBased_get(rtk_port_t* pMirroring_port, rtk_portmask_t *pMirrored_rx_mask, rtk_portmask_t *pMirrored_tx_mask);
extern rtk_api_ret_t rtl8316d_mirror_portBased_set(rtk_port_t mirroring_port, rtk_portmask_t *pMirrored_rx_mask, rtk_portmask_t *pMirrored_tx_mask);

extern rtk_api_ret_t rtl8316d_storm_get(uint32 port, rtl8316d_storm_param_t * storminfo);
extern rtk_api_ret_t rtl8316d_storm_set(uint32 port, rtl8316d_storm_param_t * storminfo);
extern rtk_api_ret_t rtl8316d_setAsicPHYReg(rtl8316d_phyAddr_t* pPhyAddr, rtl8316d_phyRegAddr_t* pRegAddr, uint32 data);
extern rtk_api_ret_t rtl8316d_getAsicPHYReg(uint32 phyAddr, rtl8316d_phyRegAddr_t* pRegAddr, uint32 *data);

#endif
