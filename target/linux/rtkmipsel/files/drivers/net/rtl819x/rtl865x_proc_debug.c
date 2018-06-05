/*
* Copyright c                  Realtek Semiconductor Corporation, 2008
* All rights reserved.
*
* Program : just for driver debug
* Abstract :
* Author : Hyking Liu (Hyking_liu@realsil.com.tw)
* -------------------------------------------------------
*/
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include "rtl865x_proc_debug.h"
#include <linux/kernel.h>
#include <linux/delay.h>

#include <net/rtl/rtl865x_netif.h>
#include "common/rtl865x_netif_local.h"
#include "common/rtl865x_eventMgr.h"
#include "common/rtl_utils.h"

#ifdef CONFIG_RTL_ADAPTABLE_TSO
	#include <linux/netdev_features.h>
	extern netdev_features_t tso_feature;
#endif	

#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
#include <net/rtl/rtl865x_ppp.h>
#include "l3Driver/rtl865x_ppp_local.h"
#include "l3Driver/rtl865x_route.h"
#if defined(CONFIG_RTL_MULTIPLE_WAN)
#include <net/rtl/rtl865x_multipleWan_api.h>
#include "l3Driver/rtl865x_multipleWan.h"
#endif

#ifdef CONFIG_RTL_HW_DSLITE_SUPPORT
#include "l3Driver/rtl865x_dslite.h"
#endif
#ifdef CONFIG_RTL_HW_6RD_SUPPORT
#include <linux/netdevice.h>
#include <net/ip_tunnels.h>
#include "l3Driver/rtl865x_6rd.h"
#endif

#endif

#include "AsicDriver/rtl865x_asicBasic.h"
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#if defined(CONFIG_RTL_LAYERED_ASIC_DRIVER_L3) || defined(CONFIG_OPENWRT_SDK)
#include "AsicDriver/rtl865x_asicL3.h"
#endif
#if defined(CONFIG_RTL_LAYERED_ASIC_DRIVER_L4) || defined(CONFIG_OPENWRT_SDK)
#include "AsicDriver/rtl865x_asicL4.h"
#endif

#include "AsicDriver/rtl865xc_asicregs.h"
#include "AsicDriver/rtl865xC_hs.h"
#if defined (CONFIG_RTL_IGMP_SNOOPING)
#include <net/rtl/rtl865x_igmpsnooping.h>
#endif

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
#include <net/rtl/rtl865x_outputQueue.h>
#endif

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include "rtl865xc_swNic.h"
#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/if.h>
#include <net/rtl/rtl_nic.h>
#endif

#if defined(CONFIG_RTL_SWITCH_NEW_DESCRIPTOR)
#include "rtl819x_swNic.h"
#endif

#include "common/rtl865x_vlan.h"
extern int32 rtl865xC_setAsicEthernetForceModeRegs(uint32 port, uint32 enForceMode, uint32 forceLink, uint32 forceSpeed, uint32 forceDuplex);
extern int32 rtl8651_setAsicEthernetPHYSpeed( uint32 port, uint32 speed );
extern int32 rtl8651_setAsicEthernetPHYDuplex( uint32 port, uint32 duplex );
extern int32 rtl8651_setAsicEthernetPHYAutoNeg( uint32 port, uint32 autoneg);
extern int32 rtl8651_setAsicEthernetPHYAdvCapality(uint32 port, uint32 capality);
extern int32 mmd_read(uint32 phyId, uint32 devId, uint32 regId, uint32 *rData);
extern int32 mmd_write(uint32 phyId, uint32 devId, uint32 regId, uint32 wData);
#if defined(CONFIG_RTL_8367R_SUPPORT)
extern int32 rtl_mirror_portBased_set(uint32 mirroring_port, uint32 Mirrored_rx_portmask, uint32 Mirrored_tx_portmask);
extern int32 rtl_mirror_portBased_get(uint32 *mirroring_port, uint32 *Mirrored_rx_portmask, uint32 *Mirrored_tx_portmask);
extern int32 rtl_mirror_portIso_set(uint32 isolation);
extern int32 rtl_mirror_portIso_get(uint32 *isolation);
#endif
#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8198C_8367RB)
extern int rtl8367b_setAsicPHYReg(uint32 phyNo, uint32 phyAddr, uint32 phyData );
#define rtl83xx_setAsicPHYReg(phyNo, phyAddr, phyData)	rtl8367b_setAsicPHYReg(phyNo, phyAddr, phyData)
extern int rtl8367b_getAsicReg(uint32 reg, uint32 *pValue);
#define rtl83xx_getAsicReg(reg, pValue)	rtl8367b_getAsicReg(reg, pValue)
extern int rtl8367b_setAsicReg(uint32 reg, uint32 value);	
#define rtl83xx_setAsicReg(reg, value)	rtl8367b_setAsicReg(reg, value)
extern int rtk_vlan_get(uint32 vid, uint32 *pMbrmsk, uint32 *pUntagmsk, uint32 *pFid);
extern int rtk_vlan_portPvid_get(uint32 port, uint32 *pPvid, uint32 *pPriority);
extern int rtl8367b_getAsicPHYOCPReg(uint32 phyNo, uint32 ocpAddr, uint32 *pRegData);
extern int rtl8367b_setAsicPHYOCPReg(uint32 phyNo, uint32 ocpAddr, uint32 ocpData);
#define rtl83xx_getAsicPHYOCPReg(phyId, regId, regData) rtl8367b_getAsicPHYOCPReg(phyId, regId, regData)
#define rtl83xx_setAsicPHYOCPReg(phyId, regId, regData) rtl8367b_setAsicPHYOCPReg(phyId, regId, regData)
#elif defined(CONFIG_RTL_83XX_SUPPORT)
#include "./rtl83xx/vlan.h"
extern int rtl8367c_setAsicPHYReg(uint32 phyNo, uint32 phyAddr, uint32 phyData );
#define rtl83xx_setAsicPHYReg(phyNo, phyAddr, phyData)	rtl8367c_setAsicPHYReg(phyNo, phyAddr, phyData)
extern int rtl8367c_getAsicReg(uint32 reg, uint32 *pValue);
#define rtl83xx_getAsicReg(reg, pValue)	rtl8367c_getAsicReg(reg, pValue)
extern int rtl8367c_setAsicReg(uint32 reg, uint32 value);
#define rtl83xx_setAsicReg(reg, value)	rtl8367c_setAsicReg(reg, value)
extern int rtk_vlan_get(uint32 vid, rtk_vlan_cfg_t *pVlanCfg);
extern int rtk_vlan_portPvid_get(uint32 port, uint32 *pPvid, uint32 *pPriority);
extern int rtl8367c_getAsicPHYOCPReg(uint32 phyNo, uint32 ocpAddr, uint32 *pRegData);
extern int rtl8367c_setAsicPHYOCPReg(uint32 phyNo, uint32 ocpAddr, uint32 ocpData);
#define rtl83xx_getAsicPHYOCPReg(phyId, regId, regData) rtl8367c_getAsicPHYOCPReg(phyId, regId, regData)
#define rtl83xx_setAsicPHYOCPReg(phyId, regId, regData) rtl8367c_setAsicPHYOCPReg(phyId, regId, regData)
#endif
extern void rtl_get_83xx_snr(void);

#if defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)
extern unsigned int rx_noBuffer_cnt;
extern unsigned int tx_ringFull_cnt;
extern unsigned int tx_drop_cnt;
unsigned int statistic_total;
unsigned int statistic_ps;
unsigned int statistic_ipv6_fp;
#if !defined(CONFIG_OPENWRT_SDK)
unsigned int statistic_fp;
#endif
#include "m24kctrl.h" //added for romperf testing	

#if defined(CONFIG_OPENWRT_SDK)
unsigned int statistic_fastpath;
EXPORT_SYMBOL(statistic_fastpath);
#endif
#endif

//#define CONFIG_FORCE_10M_100M_FULL_REFINE 1
#ifndef CONFIG_OPENWRT_SDK
#define CONFIG_TR181_ETH  1
#endif

#ifdef CONFIG_TR181_ETH
struct proc_dir_entry *rtl865x_proc_dir;
#else
static struct proc_dir_entry *rtl865x_proc_dir;
#endif

#ifndef CONFIG_RTL_PROC_NEW
#ifdef CONFIG_RTL_PROC_DEBUG	//proc debug flag
static struct proc_dir_entry *vlan_entry,*netif_entry,*l2_entry, *arp_entry,
		*nexthop_entry,*l3_entry,*ip_entry,*pppoe_entry,*napt_entry,
		*acl_entry,*storm_control,*sw_netif_entry,*sw_nexthop_entry,*sw_l3_entry,
#if defined(CONFIG_RTL_MULTIPLE_WAN)
		*advRt_entry,
#endif
#if defined(RTL_DEBUG_NIC_SKB_BUFFER)
		*nic_skb_buff,
#endif
#ifdef CONFIG_RTL_LAYERED_DRIVER
		*acl_chains_entry,
#endif
#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
		*qos_rule_entry,
#endif
		*rxRing_entry, *txRing_entry, *mbuf_entry,
		*hs_entry, *pvid_entry, *mirrorPort_entry,
		
#if defined(CONFIG_RTL_LAYERED_DRIVER_L4)
		*sw_napt_entry,
#endif
		*port_bandwidth_entry, *queue_bandwidth_entry,
		*priority_decision_entry,
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		*swMCast_entry,
#endif
#if defined (CONFIG_RTL_ENABLE_RATELIMIT_TABLE)
		*rateLimit_entry,
#endif
*hwMCast_entry;
#endif

#if defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)
/*#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)*/
static struct proc_dir_entry *prive_skb_debug_entry;
/*#endif*/

static struct proc_dir_entry *mmd_entry,	*mem_entry, *diagnostic_entry,	
		*asicCnt_entry,*phyReg_entry,*port_status_entry,*mac_entry,*fc_threshold_entry,*stats_debug_entry
#if defined (CONFIG_RTL_INBAND_CTL_API) && !defined(CONFIG_RTL_PROC_NEW)
		,*portRate_entry
#endif
		;
#define	PROC_READ_RETURN_VALUE		0
extern int32 mmd_read(uint32 phyId, uint32 devId, uint32 regId, uint32 *rData);
extern int32 mmd_write(uint32 phyId, uint32 devId, uint32 regId, uint32 wData);

#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
extern int rtl_mdio_read(unsigned int mdio_phyaddr, unsigned int reg, unsigned int *pdata);
extern int rtl_mdio_write(unsigned int mdio_phyaddr, unsigned int reg, unsigned int data);
#endif

#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT)
//#define CONFIG_RTL_PROC_NEW
static struct proc_dir_entry *l3v6_entry,	*arp6_entry, *nexthop6_entry,	
		*mcast6_entry,*ip6rd_entry,*dslite_entry,*sw_dslite_entry,	*sw_arp6_entry;
#endif

#endif
#endif

#ifndef PROC_READ_RETURN_VALUE
#define	PROC_READ_RETURN_VALUE		0
#endif

void ntohl_array(u32 *org_buf, u32 *dst_buf, unsigned int words)
{
	int i = 0;
	
	if (!org_buf || !dst_buf)
		return;
	while (words--) {
		dst_buf[i] = ntohl(org_buf[i]);
		i++;
	}

	return;
}

#if defined CONFIG_RTL_DEBUG_TOOL	//debug tool flag
#ifndef CONFIG_RTL_PROC_DEBUG
 //#ifndef RTK_X86_CLE//RTK-CNSD2-NickWu-20061222: for x86 compile
 /*cfliu: This function is only for debugging. Should not be used in production code...*/

 void memDump (void *start, uint32 size, int8 * strHeader)
 {
	 int32 row, column, index, index2, max;
	 uint32 buffer[5];
	 uint8 *buf, *line, ascii[17];
	 int8 empty = ' ';
 
	 if(!start ||(size==0))
		 return;
	 line = (uint8*)start;
 
	 /*
	 16 bytes per line
	 */
	 if (strHeader)
		 rtlglue_printf ("%s", strHeader);
	 column = size % 16;
	 row = (size / 16) + 1;
	 for (index = 0; index < row; index++, line += 16) 
	 {
#ifdef RTL865X_TEST
		 buf = (uint8*)line;
#else
		 /* for un-alignment access */
		 buffer[0] = ntohl( READ_MEM32( (((uint32)line)&~3)+ 0 ) );
		 buffer[1] = ntohl( READ_MEM32( (((uint32)line)&~3)+ 4 ) );
		 buffer[2] = ntohl( READ_MEM32( (((uint32)line)&~3)+ 8 ) );
		 buffer[3] = ntohl( READ_MEM32( (((uint32)line)&~3)+12 ) );
		 buffer[4] = ntohl( READ_MEM32( (((uint32)line)&~3)+16 ) );
		 buf = ((uint8*)buffer) + (((uint32)line)&3);
#endif
 
		 memset (ascii, 0, 17);
 
		 max = (index == row - 1) ? column : 16;
		 if ( max==0 ) break; /* If we need not dump this line, break it. */
 
		 rtlglue_printf ("\n%08x:  ", (memaddr) line);
		 
		 //Hex
		 for (index2 = 0; index2 < max; index2++)
		 {
			 if (index2 == 8)
			 rtlglue_printf ("  ");
			 rtlglue_printf ("%02X", (uint8) buf[index2]);
			 ascii[index2] = ((uint8) buf[index2] < 32) ? empty : buf[index2];
			 if ((index2+1)%4==0)
			 	rtlglue_printf ("  ");
		 }
 	
		 if (max != 16)
		 {
			 if (max < 8)
				 rtlglue_printf ("	");
			 for (index2 = 16 - max; index2 > 0; index2--)
				 rtlglue_printf ("	 ");
		 }
		
 		 //ASCII
		 rtlglue_printf ("	%s", ascii);
	 }
	 rtlglue_printf ("\n");
	 return;
 }
#endif
#endif

void memDump2(void *start, uint32 size)
{
	int i, j;
	uint32		addr=0;
	unsigned char *p = (unsigned char *)start;

	if(!start ||(size==0))
		return;
	
	addr = (uint32)start;
	if (addr < 0x80000000 || addr > 0xbfffffff) {
		return;
	}
	
	while ( (addr) & 0x03)
		addr++;

	if (addr < 0xb8000000) {

		for(i=0; i< size ; i+=16,p+=16)
		{	
			rtlglue_printf("%08X: ", (unsigned int)p);
			for(j=0; j< 16; j++) {
				rtlglue_printf("%02X ", (unsigned int)(*(p+j)));
				if (((j+1) % 4) == 0)
					rtlglue_printf ("  ");
			}
			rtlglue_printf ("\n");
		}
		rtlglue_printf ("\n");		
	}
	else {
		for(i=0; i< size ; i+=4,addr+=16)
		{	
			rtlglue_printf("%08X:	%08X	%08X	%08X	%08X\n",
			(unsigned int)addr, (unsigned int)*(unsigned long *)(addr), 
			(unsigned int)*(unsigned long *)(addr+4), 
			(unsigned int)*(unsigned long *)(addr+8), 
			(unsigned int)*(unsigned long *)(addr+12));
		}
	}
	
	return;
}

#ifdef CONFIG_RTL865X_ROMEPERF
#ifdef CONFIG_RTL_PROC_NEW
extern int32 rtl865x_perf_proc_read(struct seq_file *s, void *v);
extern int32 rtl865x_perf_proc_write(struct file *file, const char *buffer, unsigned long count, void *data);
#else
static struct proc_dir_entry *perf_dump;
extern int32 rtl865x_perf_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data );
extern int32 rtl865x_perf_proc_write(struct file *file, const char *buffer, unsigned long count, void *data);
#endif
#endif

#ifdef CONFIG_RTL_PROC_DEBUG

static uint32 queue_bandwidth_record_portmask = 0;


#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#ifdef CONFIG_RTL_PROC_NEW
int32 rtl_dumpSwMulticastInfo(struct seq_file *s);
#else
int32 rtl_dumpSwMulticastInfo(char *page, int *len);
#endif
#endif

#ifndef CONFIG_RTL_PROC_NEW
#if defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL865X_EVENT_PROC_DEBUG)
static struct proc_dir_entry *eventMgr_entry;
#endif

#if defined (CONFIG_RTL_IGMP_SNOOPING)
static struct proc_dir_entry *igmp_entry;
#endif
#endif

extern int32 rtl865x_sw_napt_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data );
extern int32  rtl865x_sw_napt_proc_write( struct file *filp, const char *buff,unsigned long len, void *data );

#ifdef CONFIG_RTL_LAYERED_DRIVER
#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
#ifdef CONFIG_RTL_PROC_NEW
extern int32 rtl865x_sw_l2_proc_read(struct seq_file *s, void *v);
extern int32 rtl865x_sw_l2_proc_write( struct file *filp, const char *buff,unsigned long len, void *data );
#else
static struct proc_dir_entry *sw_l2_entry;
extern int32 rtl865x_sw_l2_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data );
extern int32 rtl865x_sw_l2_proc_write( struct file *filp, const char *buff,unsigned long len, void *data );
#endif
#endif
#endif

#ifdef CONFIG_RTL_PROC_NEW
extern int32 sw_arp6_read(struct seq_file *s, void *v);
#else
extern int32 sw_arp6_read( char *page, char **start, off_t off, int count, int *eof, void *data );
#endif
extern int32 sw_arp6_write( struct file *filp, const char *buff,unsigned long len, void *data );

#if 0
int vlan_show(void)
{
		int i, j;

		for ( i = 0; i < RTL865XC_VLAN_NUMBER; i++ )
		{
			rtl865x_tblAsicDrv_vlanParam_t vlan;

			if ( rtl8651_getAsicVlan( i, &vlan ) == FAILED )
				continue;

			printk("  VID[%d] ", i);
			printk("\n\tmember ports:");

			for( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( vlan.memberPortMask & ( 1 << j ) )
					printk( "%d ", j);
			}

			printk("\n\tUntag member ports:");

			for( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if( vlan.untagPortMask & ( 1 << j ) )
					printk("%d ", j);
			}

			printk("\n\tFID:\t%d\n",vlan.fid);
		}

		return SUCCESS;

}
#endif

//static int32 vlan_read( char *page, char **start, off_t off, int count, int *eof, void *data )
static int32 vlan_single_show(struct seq_file *s, void *v)
{
	seq_printf(s,"%s\n", "ASIC VLAN Table:");

	{
		int i, j;

		for ( i = 0; i < RTL865XC_VLANTBL_SIZE; i++ )
		{
			rtl865x_tblAsicDrv_vlanParam_t vlan;

			if ( rtl8651_getAsicVlan( i, &vlan ) == FAILED )
				continue;

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
			seq_printf(s, "  idx [%d]  VID[%d] ", i, vlan.vid);
#else
			seq_printf(s, "  VID[%d] ", i);
#endif
			seq_printf(s, "\n\tmember ports:");

			for( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( vlan.memberPortMask & ( 1 << j ) )
					seq_printf(s,"%d ", j);
			}

			seq_printf(s,"\n\tUntag member ports:");

			for( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if( vlan.untagPortMask & ( 1 << j ) )
					seq_printf(s,"%d ", j);
			}

			seq_printf(s,"\n\tFID:\t%d\n",vlan.fid);
#if defined(CONFIG_RTL_8197F)
			seq_printf(s,"\n\tHP:\t%d\n",vlan.hp);
#endif
		}

	}

	return 0;
}

#if defined(CONFIG_RTL_DYNAMIC_IRAM_MAPPING_FOR_WAPI)
extern int switch_iram(uint32 addr);
#endif

extern int32 rtl865x_addVlan(uint16 vid);
extern int32 rtl865x_addVlanPortMember(uint16 vid, uint32 portMask);
extern int32 rtl865x_setVlanPortTag(uint16 vid,uint32 portMask,uint8 tag);
extern int32 rtl865x_setVlanFilterDatabase(uint16 vid, uint32 fid);
extern int32 rtl865x_delVlan(uint16 vid);

static int32 vlan_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[256];
	if(len>32)
	{
		goto errout;
	}
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len -1] = '\0';
		if(tmpbuf[0] == '1')
		{
		#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
		        rtl865x_AclRule_t rule;

		        memset(&rule,0,sizeof(rtl865x_AclRule_t));
		        rule.actionType_ = RTL865X_ACL_PRIORITY;
		        rule.ruleType_ = RTL865X_ACL_IP;
		        rule.dstIpAddrLB_ = 0xc0a801fe;
			 rule.dstIpAddrUB_ = 0xc0a801fe;
			 rule.priority_ = 6;
		        rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;

		        rtl865x_add_acl(&rule, RTL_DRV_LAN_NETIF_NAME, RTL865X_ACL_SYSTEM_USED);
		#endif

		}
#if defined(CONFIG_RTL_DYNAMIC_IRAM_MAPPING_FOR_WAPI)
		else if(tmpbuf[0] =='2')
		{
			switch_iram(0);

		}
		else if(tmpbuf[0] == '3')
		{
			switch_iram(1);
		}
#endif
		else
		{
			char		*strptr, *cmd_addr;
			char		*tokptr;
			int vid = 0;

			strptr = tmpbuf;
			cmd_addr = strsep(&strptr," ");
			if (cmd_addr==NULL)
			{
				goto errout;
			}
			rtlglue_printf("cmd %s\n", cmd_addr);
			if (!memcmp(cmd_addr, "dump", 4))
			{
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}

				vid = simple_strtol(tokptr, NULL, 0);
				rtlglue_printf("Vlan info:\n");
				{
					rtl865x_tblAsicDrv_vlanParam_t vlan;
					int j;

					if ( rtl8651_getAsicVlan( vid, &vlan ) == FAILED )
						return len;

					rtlglue_printf("  VID[%d] ", vid);
					rtlglue_printf("\n\tmember ports:");

					for( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
					{
						if ( vlan.memberPortMask & ( 1 << j ) )
							rtlglue_printf("%d ", j);
					}

					rtlglue_printf("\n\tUntag member ports:");

					for( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
					{
						if( vlan.untagPortMask & ( 1 << j ) )
							rtlglue_printf("%d ", j);
					}

					rtlglue_printf("\n\tFID:\t%d\n",vlan.fid);
				}
			}
			else if (	(!memcmp(cmd_addr, "add", 3)) || 
				(!memcmp(cmd_addr, "Add", 3)) ||
				(!memcmp(cmd_addr, "ADD", 3))	)
			{
				
				uint32 vid=0;
				uint32 portMask=0;
				uint32 taggedPortMask=0;
				extern int32 rtl865x_addVlanPortMember2(uint16 vid, uint32 portMask, uint32 untagPortMask);
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				
				vid=simple_strtol(tokptr, NULL, 0);

				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}

				portMask=simple_strtol(tokptr, NULL, 16);
				
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				
				taggedPortMask= simple_strtol(tokptr, NULL, 16);
				
				rtl865x_addVlan(vid);
				//rtl865x_addVlanPortMember(vid & 4095,portMask & 0x13F);
				//rtl865x_setVlanPortTag(vid,taggedPortMask,1);
				rtl865x_addVlanPortMember2(vid & 4095,portMask, taggedPortMask);
				rtl865x_setVlanFilterDatabase(vid,0);				
			}
			else if (	(!memcmp(cmd_addr, "del", 3)) || 
				(!memcmp(cmd_addr, "Del", 3)) ||
				(!memcmp(cmd_addr, "DEL", 3))	)
			{
				uint32 vid=0;
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				
				vid=simple_strtol(tokptr, NULL, 0);
				
				rtl865x_delVlan(vid);

			}
			else if (	(!memcmp(cmd_addr, "mod", 3)) )
			{
				int i;
				uint32 vid=0;
				uint32 portMask=0;
				uint32 untagPortMask=0;
				extern int32 rtl865x_addVlanPortMember2(uint16 vid, uint32 portMask, uint32 untagPortMask);
				extern int update_vlanconfig(uint32 vid, uint32 portMask, uint32 untagPortMask);

				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				
				vid=simple_strtol(tokptr, NULL, 0);

				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}

				portMask=simple_strtol(tokptr, NULL, 16);

				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				
				untagPortMask= simple_strtol(tokptr, NULL, 16); 

				rtl865x_addVlanPortMember2(vid & 4095,portMask & 0x1FF, untagPortMask & 0x1FF);

				// update pvid
				for(i=0; i<RTL8651_PORT_NUMBER + 3; i++)
				{
					if (( (1<<i) & portMask ) != 0)
					{
						rtl8651_setAsicPvid(i, vid);
					}
				}
				
				// update vlanconfig
				update_vlanconfig(vid & 4095,portMask & 0x1FF, untagPortMask & 0x1FF);
			}
			return len;
errout:
			rtlglue_printf("vlan operation only support \"dump\" as the first parameter\n");
			rtlglue_printf("dump format:	\"dump vid\"\n");
		}
	}
	return len;
}

int vlan_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, vlan_single_show, NULL));
}

static ssize_t vlan_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return vlan_write(file, userbuf,count, off);
}


struct file_operations vlan_single_seq_file_operations = {
        .open           = vlan_single_open,
	 .write		=vlan_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};


#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#define RTL8651_ACLTBL_DROP_ALL RTL865X_ACLTBL_DROP_ALL
#define RTL8651_ACLTBL_PERMIT_ALL RTL865X_ACLTBL_PERMIT_ALL
#define RTL8651_ACLTBL_ALL_TO_CPU RTL865X_ACLTBL_ALL_TO_CPU
#endif
#ifdef CONFIG_RTL_PROC_NEW
static int32 netif_read(struct seq_file *s, void *v)
{
	int8	*pst[] = { "DIS/BLK",  "LIS", "LRN", "FWD" };
	uint8 *mac;
	int32 i, j;

	seq_printf(s, "%s\n", "ASIC Network Interface Table:");
	for ( i = 0; i < RTL865XC_NETIFTBL_SIZE; i++ )
	{
		rtl865x_tblAsicDrv_intfParam_t intf;
		rtl865x_tblAsicDrv_vlanParam_t vlan;
		memset(&vlan, 0x00, sizeof(rtl865x_tblAsicDrv_vlanParam_t));

		if ( rtl8651_getAsicNetInterface( i, &intf ) == FAILED )
			continue;

		if ( intf.valid )
		{
			mac = (uint8 *)&intf.macAddr.octet[0];
			seq_printf(s,"[%d]  VID[%d] %x:%x:%x:%x:%x:%x",
				i, intf.vid, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			seq_printf(s,"  Routing %s \n",
				intf.enableRoute==TRUE? "enabled": "disabled" );

			seq_printf(s,"      ingress ");

			if ( RTL8651_ACLTBL_DROP_ALL <= intf.inAclStart )
			{
				if ( intf.inAclStart == RTL8651_ACLTBL_PERMIT_ALL )
					seq_printf(s,"permit all,");
				if ( intf.inAclStart == RTL8651_ACLTBL_ALL_TO_CPU )
					seq_printf(s,"all to cpu,");
				if ( intf.inAclStart == RTL8651_ACLTBL_DROP_ALL )
					seq_printf(s,"drop all,");
			}
			else
				seq_printf(s,"ACL %d-%d, ", intf.inAclStart, intf.inAclEnd);

			seq_printf(s,"  egress ");

			if ( RTL8651_ACLTBL_DROP_ALL <= intf.outAclStart )
			{
				if ( intf.outAclStart == RTL8651_ACLTBL_PERMIT_ALL )
					seq_printf(s,"permit all,");
				if ( intf.outAclStart==RTL8651_ACLTBL_ALL_TO_CPU )
					seq_printf(s,"all to cpu,");
				if ( intf.outAclStart==RTL8651_ACLTBL_DROP_ALL )
					seq_printf(s,"drop all,");
			}
			else
				seq_printf(s,"ACL %d-%d, ", intf.outAclStart, intf.outAclEnd);

			seq_printf(s, "\n      %d MAC Addresses, MTU %d Bytes\n", intf.macAddrNumber, intf.mtu);
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
			seq_printf(s, "      enableRouteV6:%d,    mtuv6:%d\n", intf.enableRouteV6, intf.mtuV6);
#endif

			#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
			rtl8651_findAsicVlanIndexByVid(&intf.vid);
			#endif
			rtl8651_getAsicVlan( intf.vid, &vlan );

			seq_printf(s,"\n      Untag member ports:");

			for ( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( vlan.untagPortMask & ( 1 << j ) )
					seq_printf(s,"%d ", j);
			}
			seq_printf(s, "\n      Active member ports:");

			for ( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( vlan.memberPortMask & ( 1 << j ) )
					seq_printf(s, "%d ", j);
			}

			seq_printf(s, "\n      Port state(");

			for ( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( ( vlan.memberPortMask & ( 1 << j ) ) == 0 )
					continue;
				if ((( READ_MEM32( PCRP0 + j * 4 ) & STP_PortST_MASK) >> STP_PortST_OFFSET ) > 4 )
					seq_printf(s, "--- ");
				else
					seq_printf(s, "%d:%s ", j, pst[(( READ_MEM32( PCRP0 + j * 4 ) & STP_PortST_MASK) >> STP_PortST_OFFSET )]);

			}
			seq_printf(s, ")\n\n");
		}

	}
	return 0;
}
#else
static int32 netif_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	int8	*pst[] = { "DIS/BLK",  "LIS", "LRN", "FWD" };
	uint8 *mac;
	int32 i, j;

	len = sprintf(page, "%s\n", "ASIC Network Interface Table:");
	for ( i = 0; i < RTL865XC_NETIFTBL_SIZE; i++ )
	{
		rtl865x_tblAsicDrv_intfParam_t intf;
		rtl865x_tblAsicDrv_vlanParam_t vlan;
		memset(&vlan, 0x00, sizeof(rtl865x_tblAsicDrv_vlanParam_t));

		if ( rtl8651_getAsicNetInterface( i, &intf ) == FAILED )
			continue;

		if ( intf.valid )
		{
			mac = (uint8 *)&intf.macAddr.octet[0];
			len += sprintf(page+len,"[%d]  VID[%d] %x:%x:%x:%x:%x:%x",
				i, intf.vid, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			len += sprintf(page+len,"  Routing %s \n",
				intf.enableRoute==TRUE? "enabled": "disabled" );

			len += sprintf(page+len,"      ingress ");

			if ( RTL8651_ACLTBL_DROP_ALL <= intf.inAclStart )
			{
				if ( intf.inAclStart == RTL8651_ACLTBL_PERMIT_ALL )
					len += sprintf(page+len,"permit all,");
				if ( intf.inAclStart == RTL8651_ACLTBL_ALL_TO_CPU )
					len += sprintf(page+len,"all to cpu,");
				if ( intf.inAclStart == RTL8651_ACLTBL_DROP_ALL )
					len += sprintf(page+len,"drop all,");
			}
			else
				len += sprintf(page+len,"ACL %d-%d, ", intf.inAclStart, intf.inAclEnd);

			len += sprintf(page+len,"  egress ");

			if ( RTL8651_ACLTBL_DROP_ALL <= intf.outAclStart )
			{
				if ( intf.outAclStart == RTL8651_ACLTBL_PERMIT_ALL )
					len += sprintf(page+len,"permit all,");
				if ( intf.outAclStart==RTL8651_ACLTBL_ALL_TO_CPU )
					len += sprintf(page+len,"all to cpu,");
				if ( intf.outAclStart==RTL8651_ACLTBL_DROP_ALL )
					len += sprintf(page+len,"drop all,");
			}
			else
				len += sprintf(page+len,"ACL %d-%d, ", intf.outAclStart, intf.outAclEnd);

			len += sprintf(page+len, "\n      %d MAC Addresses, MTU %d Bytes\n", intf.macAddrNumber, intf.mtu);
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
			len += sprintf(page+len, "      enableRouteV6:%d,    mtuv6:%d\n", intf.enableRouteV6, intf.mtuV6);
#endif

			#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
			rtl8651_findAsicVlanIndexByVid(&intf.vid);
			#endif
			rtl8651_getAsicVlan( intf.vid, &vlan );

			len += sprintf(page+len,"\n      Untag member ports:");

			for ( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( vlan.untagPortMask & ( 1 << j ) )
					len += sprintf(page+len,"%d ", j);
			}
			len += sprintf(page+len, "\n      Active member ports:");

			for ( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( vlan.memberPortMask & ( 1 << j ) )
					len += sprintf(page+len, "%d ", j);
			}

			len += sprintf(page+len, "\n      Port state(");

			for ( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( ( vlan.memberPortMask & ( 1 << j ) ) == 0 )
					continue;
				if ((( READ_MEM32( PCRP0 + j * 4 ) & STP_PortST_MASK) >> STP_PortST_OFFSET ) > 4 )
					len += sprintf(page+len, "--- ");
				else
					len += sprintf(page+len, "%d:%s ", j, pst[(( READ_MEM32( PCRP0 + j * 4 ) & STP_PortST_MASK) >> STP_PortST_OFFSET )]);

			}
			len += sprintf(page+len, ")\n\n");
		}

	}

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}
#endif

static int32 netif_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

#ifdef CONFIG_RTL_PROC_NEW
extern int32 sw_netif_read(struct seq_file *s, void *v);
#else
extern int32 sw_netif_read( char *page, char **start, off_t off, int count, int *eof, void *data );

#endif
static int32 sw_netif_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}
#if 0
static int32 acl_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	int8 *actionT[] = { "permit", "redirect to ether", "drop", "to cpu", "legacy drop",
					"drop for log", "mirror", "redirect to pppoe", "default redirect", "mirror keep match",
					"drop rate exceed pps", "log rate exceed pps", "drop rate exceed bps", "log rate exceed bps","priority "
					};
#ifdef CONFIG_RTL_LAYERED_DRIVER
	rtl865x_AclRule_t asic_acl;
#else
	_rtl8651_tblDrvAclRule_t asic_acl;
#endif
	rtl865x_tblAsicDrv_intfParam_t asic_intf;
	uint32 acl_start, acl_end;

	uint16 vid;
	int8 outRule;



	len = sprintf(page, "%s\n", "ASIC ACL Table:");
	for(vid=0; vid<8; vid++ )
	{
		/* Read VLAN Table */
		if (rtl8651_getAsicNetInterface(vid, &asic_intf) == FAILED)
			continue;
		if (asic_intf.valid==FALSE)
			continue;

		outRule = FALSE;
		acl_start = asic_intf.inAclStart; acl_end = asic_intf.inAclEnd;
		len += sprintf(page+len, "\nacl_start(%d), acl_end(%d)", acl_start, acl_end);
	again:
		if (outRule == FALSE)
			len += sprintf(page+len, "\n<<Ingress Rule for Netif  %d: (VID %d)>>\n", vid,asic_intf.vid);
		else
			len += sprintf(page+len, "\n<<Egress Rule for Netif %d (VID %d)>>:\n", vid,asic_intf.vid);

#ifdef CONFIG_RTL_LAYERED_DRIVER

		for(; acl_start<= acl_end;acl_start++)
		{
			if ( _rtl865x_getAclFromAsic(acl_start, &asic_acl) == FAILED)
				rtlglue_printf("=============%s(%d): get asic acl rule error!\n",__FUNCTION__, __LINE__);

			switch(asic_acl.ruleType_)
			{
			case RTL865X_ACL_MAC:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Ethernet", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tether type: %x   ether type mask: %x\n", asic_acl.typeLen_, asic_acl.typeLenMask_);
				len += sprintf(page+len, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstMac_.octet[0], asic_acl.dstMac_.octet[1], asic_acl.dstMac_.octet[2],
						asic_acl.dstMac_.octet[3], asic_acl.dstMac_.octet[4], asic_acl.dstMac_.octet[5],
						asic_acl.dstMacMask_.octet[0], asic_acl.dstMacMask_.octet[1], asic_acl.dstMacMask_.octet[2],
						asic_acl.dstMacMask_.octet[3], asic_acl.dstMacMask_.octet[4], asic_acl.dstMacMask_.octet[5]
						);

				len += sprintf(page+len, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcMac_.octet[0], asic_acl.srcMac_.octet[1], asic_acl.srcMac_.octet[2],
						asic_acl.srcMac_.octet[3], asic_acl.srcMac_.octet[4], asic_acl.srcMac_.octet[5],
						asic_acl.srcMacMask_.octet[0], asic_acl.srcMacMask_.octet[1], asic_acl.srcMacMask_.octet[2],
						asic_acl.srcMacMask_.octet[3], asic_acl.srcMacMask_.octet[4], asic_acl.srcMacMask_.octet[5]
					);
				break;

			case RTL865X_ACL_IP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
					);

				len += sprintf(page+len, "\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
				len += sprintf(page+len, "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_);
					break;

			case RTL865X_ACL_IP_RANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IP Range", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
						);
				len += sprintf(page+len, "\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
					len += sprintf(page+len, "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_);
					break;
			case RTL865X_ACL_ICMP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "ICMP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_,
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
				break;
			case RTL865X_ACL_ICMP_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "ICMP IP RANGE", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_,
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
				break;
			case RTL865X_ACL_IGMP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IGMP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
						);
				break;


			case RTL865X_ACL_IGMP_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IGMP IP RANGE", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
						);
				break;

			case RTL865X_ACL_TCP:
					len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "TCP", actionT[asic_acl.actionType_]);
					len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
						);
				len += sprintf(page+len, "\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
						);
				break;
			case RTL865X_ACL_TCP_IPRANGE:
					len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "TCP IP RANGE", actionT[asic_acl.actionType_]);
					len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
					len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
					len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
						);
					len += sprintf(page+len, "\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
					);
				break;

			case RTL865X_ACL_UDP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start,"UDP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
						);
				break;
			case RTL865X_ACL_UDP_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "UDP IP RANGE", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
					);
				break;


			case RTL865X_ACL_SRCFILTER:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Source Filter", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2],
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d   sipM: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
				break;

			case RTL865X_ACL_SRCFILTER_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Source Filter(IP RANGE)", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2],
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d   sipL: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
				break;

			case RTL865X_ACL_DSTFILTER:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Deatination Filter", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2],
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_,
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)),
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
						);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d   dipM: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
						);
				break;
			case RTL865X_ACL_DSTFILTER_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Deatination Filter(IP Range)", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2],
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_,
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)),
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
						);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d   dipL: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
					);
				break;

				default:
					len += sprintf(page+len, "asic_acl.ruleType_(0x%x)\n", asic_acl.ruleType_);

		}


				/* Action type */
		switch (asic_acl.actionType_)
		{

			case RTL865X_ACL_PERMIT:
			case RTL865X_ACL_REDIRECT_ETHER:
			case RTL865X_ACL_DROP:
			case RTL865X_ACL_TOCPU:
			case RTL865X_ACL_LEGACY_DROP:
			case RTL865X_ACL_DROPCPU_LOG:
			case RTL865X_ACL_MIRROR:
			case RTL865X_ACL_REDIRECT_PPPOE:
			case RTL865X_ACL_MIRROR_KEEP_MATCH:
				len += sprintf(page+len, "\tnetifIdx: %d   pppoeIdx: %d   l2Idx:%d  ", asic_acl.netifIdx_, asic_acl.pppoeIdx_, asic_acl.L2Idx_);
				break;

			case RTL865X_ACL_PRIORITY:
				len += sprintf(page+len, "\tprioirty: %d   ", asic_acl.priority_) ;
				break;

			case RTL865X_ACL_DEFAULT_REDIRECT:
				len += sprintf(page+len,"\tnextHop:%d  ", asic_acl.nexthopIdx_);
				break;

			case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
			case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
			case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
			case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
				len += sprintf(page+len, "\tratelimitIdx: %d  ", asic_acl.ratelimtIdx_);
				break;
			default:
				;

			}
			len += sprintf(page+len, "pktOpApp: %d\n", asic_acl.pktOpApp_);

		}
#else
		for( ; acl_start<=acl_end; acl_start++)
		{
			if (rtl8651_getAsicAclRule(acl_start, &asic_acl) == FAILED)
				rtlglue_printf("=============%s(%d): get asic acl rule error!\n",__FUNCTION__, __LINE__);

			switch(asic_acl.ruleType_)
			{
			case RTL8651_ACL_MAC:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Ethernet", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tether type: %x   ether type mask: %x\n", asic_acl.typeLen_, asic_acl.typeLenMask_);
				len += sprintf(page+len, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstMac_.octet[0], asic_acl.dstMac_.octet[1], asic_acl.dstMac_.octet[2],
						asic_acl.dstMac_.octet[3], asic_acl.dstMac_.octet[4], asic_acl.dstMac_.octet[5],
						asic_acl.dstMacMask_.octet[0], asic_acl.dstMacMask_.octet[1], asic_acl.dstMacMask_.octet[2],
						asic_acl.dstMacMask_.octet[3], asic_acl.dstMacMask_.octet[4], asic_acl.dstMacMask_.octet[5]
						);

				len += sprintf(page+len, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcMac_.octet[0], asic_acl.srcMac_.octet[1], asic_acl.srcMac_.octet[2],
						asic_acl.srcMac_.octet[3], asic_acl.srcMac_.octet[4], asic_acl.srcMac_.octet[5],
						asic_acl.srcMacMask_.octet[0], asic_acl.srcMacMask_.octet[1], asic_acl.srcMacMask_.octet[2],
						asic_acl.srcMacMask_.octet[3], asic_acl.srcMacMask_.octet[4], asic_acl.srcMacMask_.octet[5]
					);
				break;

			case RTL8651_ACL_IP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
					);

				len += sprintf(page+len, "\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
				len += sprintf(page+len, "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_);
					break;

			case RTL8652_ACL_IP_RANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IP Range", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
						);
				len += sprintf(page+len, "\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
					len += sprintf(page+len, "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_);
					break;
			case RTL8651_ACL_ICMP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "ICMP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_,
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
				break;
			case RTL8652_ACL_ICMP_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "ICMP IP RANGE", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_,
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
				break;
			case RTL8651_ACL_IGMP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IGMP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
						);
				break;


			case RTL8652_ACL_IGMP_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "IGMP IP RANGE", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
						);
				break;

			case RTL8651_ACL_TCP:
					len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "TCP", actionT[asic_acl.actionType_]);
					len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
						);
				len += sprintf(page+len, "\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
						);
				break;
			case RTL8652_ACL_TCP_IPRANGE:
					len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "TCP IP RANGE", actionT[asic_acl.actionType_]);
					len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
					len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
					len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
						);
					len += sprintf(page+len, "\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
					);
				break;

			case RTL8651_ACL_UDP:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start,"UDP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
						);
				break;
			case RTL8652_ACL_UDP_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "UDP IP RANGE", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
					);
				break;

			case RTL8651_ACL_IFSEL:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "UDP", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tgidxSel: %x\n", asic_acl.gidxSel_);
				break;
			case RTL8651_ACL_SRCFILTER:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Source Filter", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2],
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
						);
				len += sprintf(page+len, "\tsip: %d.%d.%d.%d   sipM: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
				break;

			case RTL8652_ACL_SRCFILTER_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Source Filter(IP RANGE)", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2],
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
						);
				len += sprintf(page+len, "\tsipU: %d.%d.%d.%d   sipL: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
						);
				len += sprintf(page+len, "\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
				break;

			case RTL8651_ACL_DSTFILTER:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Deatination Filter", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2],
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_,
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)),
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
						);
				len += sprintf(page+len, "\tdip: %d.%d.%d.%d   dipM: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
						);
				break;
			case RTL8652_ACL_DSTFILTER_IPRANGE:
				len += sprintf(page+len, " [%d] rule type: %s   rule action: %s\n", acl_start, "Deatination Filter(IP Range)", actionT[asic_acl.actionType_]);
				len += sprintf(page+len, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2],
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
						);
				len += sprintf(page+len, "\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_,
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)),
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
						);
				len += sprintf(page+len, "\tdipU: %d.%d.%d.%d   dipL: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
					);
				break;

				default:
					len += sprintf(page+len, "asic_acl.ruleType_(0x%x)\n", asic_acl.ruleType_);

		}


				/* Action type */
		switch (asic_acl.actionType_)
		{

			case RTL8651_ACL_PERMIT: /* 0x00 */
			case RTL8651_ACL_REDIRECT: /* 0x01 */
			case RTL8651_ACL_CPU: /* 0x03 */
			case RTL8651_ACL_DROP: /* 0x02, 0x04 */
			case RTL8651_ACL_DROP_LOG: /* 0x05 */
			case RTL8651_ACL_MIRROR: /* 0x06 */
			case RTL8651_ACL_REDIRECT_PPPOE: /* 0x07 */
			case RTL8651_ACL_MIRROR_KEEP_MATCH: /* 0x09 */
				len += sprintf(page+len, "\tdvidx: %d   hp: %d   pppoeIdx: %d   nxtHop:%d  ", asic_acl.dvid_, asic_acl.priority_,
						asic_acl.pppoeIdx_, asic_acl.nextHop_);
				break;

			case RTL8651_ACL_POLICY: /* 0x08 */
				len += sprintf(page+len, "\thp: %d   nxtHopIdx: %d  ", asic_acl.priority_, asic_acl.nhIndex);
				break;

			case RTL8651_ACL_PRIORITY: /* 0x08 */
				len += sprintf(page+len, "\tprioirty: %d   ", asic_acl.priority) ;
				break;

			case RTL8651_ACL_DROP_RATE_EXCEED_PPS: /* 0x0a */
			case RTL8651_ACL_LOG_RATE_EXCEED_PPS: /* 0x0b */
			case RTL8651_ACL_DROP_RATE_EXCEED_BPS: /* 0x0c */
			case RTL8651_ACL_LOG_RATE_EXCEED_BPS: /* 0x0d */
				len += sprintf(page+len, "\trlIdx: %d  ", asic_acl.rlIndex);
				break;
			default:
				;

			}
			len += sprintf(page+len, "pktOpApp: %d\n", asic_acl.pktOpApp);

		}

#endif

		if (outRule == FALSE)
		{
			acl_start = asic_intf.outAclStart; acl_end = asic_intf.outAclEnd;
			outRule = TRUE;
			goto again;
		}
	}

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}

static int32 acl_write( struct file *filp, const char *buff,unsigned long len, void *data )
{

#if RTL_LAYERED_DRIVER_DEBUG
	char 	tmpbuf[32];
	int32 testNo;

	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		testNo = tmpbuf[0]-'0';
		rtl865x_acl_test(testNo);
	}
#endif
	return len;
}

#endif

#if defined(CONFIG_RTL_MULTIPLE_WAN)
#ifdef CONFIG_RTL_PROC_NEW
static int32 advRt_read(struct seq_file *s, void *v)
{
	rtl_show_advRt_table(s);
	return 0;
}
#else
static int32 advRt_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len = 0;
	rtl_show_advRt_table(page, &len);
    
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}
#endif

static int32 advRt_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[32];
	rtl_advRoute_entry_t rule;
	int retval;
	if(len>32)
	{
		goto errout;
	}
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len -1] = '\0';
		if(tmpbuf[0] == '1')
		{
			//add rule
			memset(&rule,0,sizeof(rtl_advRoute_entry_t));
			rule.extIp = 0xc0a8030b;
			rule.nexthop = 0xc0a80303;
			rule.pktOpApp_ = 6;
			memcpy(rule.outIfName,"eth6",4);
			rule.ruleType_ = RTL_ADVRT_IP_RANGE;
			rule.valid_ = 1;

			rule.advrt_srcIpAddrStart_ = 0xc0a80180;
			rule.advrt_srcIpAddrEnd_ = 0xc0a801ff;
			rule.advrt_dstIpAddrStart_ = 0x0;
			rule.advrt_dstIpAddrEnd_ = 0xffffffff;

			retval = rtl_add_advRt_entry(&rule);
			rtlglue_printf("===%s(%d),retval(%d)\n",__FUNCTION__,__LINE__,retval);

			//add rule
			memset(&rule,0,sizeof(rtl_advRoute_entry_t));
			rule.extIp = 0xc0a8020b;
			rule.nexthop = 0xc0a80203;
			rule.pktOpApp_ = 6;
			memcpy(rule.outIfName,"eth1",4);
			rule.ruleType_ = RTL_ADVRT_IP_RANGE;
			rule.valid_ = 1;

			rule.advrt_srcIpAddrStart_ = 0xc0a80101;
			rule.advrt_srcIpAddrEnd_ = 0xc0a8017f;
			rule.advrt_dstIpAddrStart_ = 0x0;
			rule.advrt_dstIpAddrEnd_ = 0xffffffff;
			retval = rtl_add_advRt_entry(&rule);

			rtlglue_printf("===%s(%d),retval(%d)\n",__FUNCTION__,__LINE__,retval);
		}
		else if(tmpbuf[0] == '2')
		{
			memset(&rule,0,sizeof(rtl_advRoute_entry_t));
			rule.extIp = 0xc0a8030b;
			rule.nexthop = 0xc0a80303;
			rule.pktOpApp_ = 6;
			memcpy(rule.outIfName,"eth6",4);
			rule.ruleType_ = RTL_ADVRT_IP_RANGE;
			rule.valid_ = 1;

			rule.advrt_srcIpAddrStart_ = 0xc0a80180;
			rule.advrt_srcIpAddrEnd_ = 0xc0a801ff;
			rule.advrt_dstIpAddrStart_ = 0x0;
			rule.advrt_dstIpAddrEnd_ = 0xffffffff;

			retval = rtl_del_advRt_entry(&rule);
			rtlglue_printf("===%s(%d),retval(%d)\n",__FUNCTION__,__LINE__,retval);
		}
		else
		{
			memset(&rule,0,sizeof(rtl_advRoute_entry_t));
			rule.extIp = 0xc0a8020b;
			rule.nexthop = 0xc0a80203;
			rule.pktOpApp_ = 6;
			memcpy(rule.outIfName,"eth1",4);
			rule.ruleType_ = RTL_ADVRT_IP_RANGE;
			rule.valid_ = 1;

			rule.advrt_srcIpAddrStart_ = 0xc0a80101;
			rule.advrt_srcIpAddrEnd_ = 0xc0a8017f;
			rule.advrt_dstIpAddrStart_ = 0x0;
			rule.advrt_dstIpAddrEnd_ = 0xffffffff;
			retval = rtl_del_advRt_entry(&rule);

			rtlglue_printf("===%s(%d),retval(%d)\n",__FUNCTION__,__LINE__,retval);
		}
	}
	else
	{
errout:
	rtlglue_printf("error input\n");
	}
	return len;
}
#endif

int acl_show(struct seq_file *s, void *v)
{
	int8 *actionT[] = { "permit", "redirect to ether", "drop", "to cpu", "legacy drop",
					"drop for log", "mirror", "redirect to pppoe", "default redirect", "mirror keep match",
					"drop rate exceed pps", "log rate exceed pps", "drop rate exceed bps", "log rate exceed bps","priority "
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
                    , "change vid"
#endif
					};
#ifdef CONFIG_RTL_LAYERED_DRIVER
	rtl865x_AclRule_t asic_acl;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    rtl865x_AclRule_t asic_acl2;
    uint32 acl_temp = 0;
#endif

#else
	_rtl8651_tblDrvAclRule_t asic_acl;
#endif
	rtl865x_tblAsicDrv_intfParam_t asic_intf;
	uint32 acl_start, acl_end;

	uint16 vid;
	int8 outRule;
#if defined (CONFIG_RTL_LOCAL_PUBLIC) || defined(CONFIG_RTL_MULTIPLE_WAN) || defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	unsigned char defInAclStart, defInAclEnd,defOutAclStart,defOutAclEnd;
#endif
	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	inv6_addr_t start_ip, end_ip;
	#endif

	seq_printf(s, "%s\n", "ASIC ACL Table:");
	for(vid=0; vid<8; vid++ )
	{
		/* Read VLAN Table */
		if (rtl8651_getAsicNetInterface(vid, &asic_intf) == FAILED)
			continue;
		if (asic_intf.valid==FALSE)
			continue;

		outRule = FALSE;
		acl_start = asic_intf.inAclStart; acl_end = asic_intf.inAclEnd;
		seq_printf(s, "\nacl_start(%d), acl_end(%d)", acl_start, acl_end);
	again:
		if (outRule == FALSE)
			seq_printf(s, "\n<<Ingress Rule for Netif  %d: (VID %d)>>\n", vid,asic_intf.vid);
		else
			seq_printf(s, "\n<<Egress Rule for Netif %d (VID %d)>>:\n", vid,asic_intf.vid);

#ifdef CONFIG_RTL_LAYERED_DRIVER

		for(; acl_start<= acl_end;acl_start++)
		{
			if ( _rtl865x_getAclFromAsic(acl_start, &asic_acl) == FAILED)
				seq_printf(s, "=============%s(%d): get asic acl rule error!\n",__FUNCTION__, __LINE__);

			switch(asic_acl.ruleType_)
			{
			case RTL865X_ACL_MAC:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "Ethernet", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tether type: %x   ether type mask: %x\n", asic_acl.typeLen_, asic_acl.typeLenMask_);
				seq_printf(s, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstMac_.octet[0], asic_acl.dstMac_.octet[1], asic_acl.dstMac_.octet[2],
						asic_acl.dstMac_.octet[3], asic_acl.dstMac_.octet[4], asic_acl.dstMac_.octet[5],
						asic_acl.dstMacMask_.octet[0], asic_acl.dstMacMask_.octet[1], asic_acl.dstMacMask_.octet[2],
						asic_acl.dstMacMask_.octet[3], asic_acl.dstMacMask_.octet[4], asic_acl.dstMacMask_.octet[5]
						);

				seq_printf(s, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcMac_.octet[0], asic_acl.srcMac_.octet[1], asic_acl.srcMac_.octet[2],
						asic_acl.srcMac_.octet[3], asic_acl.srcMac_.octet[4], asic_acl.srcMac_.octet[5],
						asic_acl.srcMacMask_.octet[0], asic_acl.srcMacMask_.octet[1], asic_acl.srcMacMask_.octet[2],
						asic_acl.srcMacMask_.octet[3], asic_acl.srcMacMask_.octet[4], asic_acl.srcMacMask_.octet[5]
					);
				break;

			case RTL865X_ACL_IP:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "IP", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
					);

				seq_printf(s, "\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
				seq_printf(s, "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_);
					break;

			case RTL865X_ACL_IP_RANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "IP Range", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
						);
				seq_printf(s, "\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
					seq_printf(s, "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_);
					break;
			case RTL865X_ACL_ICMP:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "ICMP", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_,
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
				break;
			case RTL865X_ACL_ICMP_IPRANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "ICMP IP RANGE", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_,
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
				break;
			case RTL865X_ACL_IGMP:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "IGMP", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
						);
				break;


			case RTL865X_ACL_IGMP_IPRANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "IGMP IP RANGE", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
						);
				break;

			case RTL865X_ACL_TCP:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "TCP", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
						);
				seq_printf(s, "\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
						);
				break;
			case RTL865X_ACL_TCP_IPRANGE:
					seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "TCP IP RANGE", actionT[asic_acl.actionType_]);
					seq_printf(s, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
					seq_printf(s, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
					seq_printf(s, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
						);
					seq_printf(s, "\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
					);
				break;

			case RTL865X_ACL_UDP:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start,"UDP", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
						);
				break;
			case RTL865X_ACL_UDP_IPRANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "UDP IP RANGE", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
					);
				break;


			case RTL865X_ACL_SRCFILTER:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "Source Filter", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2],
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
						);
				seq_printf(s, "\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d   sipM: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
				break;

			case RTL865X_ACL_SRCFILTER_IPRANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "Source Filter(IP RANGE)", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2],
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
						);
				seq_printf(s, "\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
						);
				seq_printf(s, "\tsipU: %d.%d.%d.%d   sipL: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
				break;

			case RTL865X_ACL_DSTFILTER:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "Deatination Filter", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2],
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
						);
				seq_printf(s, "\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_,
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)),
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
						);
				seq_printf(s, "\tdip: %d.%d.%d.%d   dipM: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
						);
				break;
			case RTL865X_ACL_DSTFILTER_IPRANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "Deatination Filter(IP Range)", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2],
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
						);
				seq_printf(s, "\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_,
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)),
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
						);
				seq_printf(s, "\tdipU: %d.%d.%d.%d   dipL: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
					);
				break;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
                case RTL865X_ACL_IPV6: /* IP Rule Type: 0x0010 */
                case RTL865X_ACL_IPV6_RANGE:
					/* A ipv6 rule occupied two entry, 
					   function _rtl865x_getAclFromAsic take one entry at a time.
					   So we need to call function _rtl865x_getAclFromAsic again. */

                    acl_temp = acl_start;            
					acl_temp++;				/* move to the second entry index of ipv6 rule */
                    if (acl_temp<=acl_end)	/* it should be less than or equal to acl_end. */
                    {
                        memset(&asic_acl2, 0x00, sizeof(rtl865x_AclRule_t));
                        if ( _rtl865x_getAclFromAsic(acl_temp, &asic_acl2) == FAILED)
                            seq_printf(s,"=============%s(%d): get asic acl rule error!\n",__FUNCTION__, __LINE__);
                        if ((!asic_acl2.ipv6EntryType_) && asic_acl.ipv6EntryType_)
                        {
                            asic_acl.dstIpV6Addr_.v6_addr32[3] = asic_acl2.dstIpV6Addr_.v6_addr32[3];
                            asic_acl.dstIpV6Addr_.v6_addr32[2] = asic_acl2.dstIpV6Addr_.v6_addr32[2];
                            asic_acl.dstIpV6Addr_.v6_addr32[1] = asic_acl2.dstIpV6Addr_.v6_addr32[1];
                            asic_acl.dstIpV6Addr_.v6_addr32[0] = asic_acl2.dstIpV6Addr_.v6_addr32[0];
                            
                            asic_acl.dstIpV6AddrMask_.v6_addr32[3] = asic_acl2.dstIpV6AddrMask_.v6_addr32[3];
                            asic_acl.dstIpV6AddrMask_.v6_addr32[2] = asic_acl2.dstIpV6AddrMask_.v6_addr32[2];
                            asic_acl.dstIpV6AddrMask_.v6_addr32[1] = asic_acl2.dstIpV6AddrMask_.v6_addr32[1];
                            asic_acl.dstIpV6AddrMask_.v6_addr32[0] = asic_acl2.dstIpV6AddrMask_.v6_addr32[0];

                            asic_acl.ipv6TrafficClass_ = asic_acl2.ipv6TrafficClass_;
                            asic_acl.ipv6TrafficClassM_ = asic_acl2.ipv6TrafficClassM_; 
                            asic_acl.ipv6NextHeader_    = asic_acl2.ipv6NextHeader_; 
                            asic_acl.ipv6NextHeaderM_   = asic_acl2.ipv6NextHeaderM_; 
                            asic_acl.ipv6HttpFilter_    = asic_acl2.ipv6HttpFilter_;
                            asic_acl.ipv6HttpFilterM_ = asic_acl2.ipv6HttpFilterM_;
                            asic_acl.ipv6IdentSrcDstIp_ = asic_acl2.ipv6IdentSrcDstIp_;
                            asic_acl.ipv6IdentSrcDstIpM_ = asic_acl2.ipv6IdentSrcDstIpM_;
                            /* ActionType and ActionField useless in entry0 */
                            asic_acl.actionType_ = asic_acl2.actionType_;
                            switch(asic_acl.actionType_) {
                        
                            case RTL865X_ACL_PERMIT:
                            case RTL865X_ACL_REDIRECT_ETHER:
                            case RTL865X_ACL_DROP:
                            case RTL865X_ACL_TOCPU:
                            case RTL865X_ACL_LEGACY_DROP:
                            case RTL865X_ACL_DROPCPU_LOG:
                            case RTL865X_ACL_MIRROR:
                            case RTL865X_ACL_REDIRECT_PPPOE:
                            case RTL865X_ACL_MIRROR_KEEP_MATCH:
                                asic_acl.L2Idx_ = asic_acl2.L2Idx_ ;
                                asic_acl.netifIdx_ =  asic_acl2.netifIdx_;
                                asic_acl.pppoeIdx_ = asic_acl2.pppoeIdx_;
                                 break;
                        
                            case RTL865X_ACL_DEFAULT_REDIRECT:
                                asic_acl.nexthopIdx_ = asic_acl2.nexthopIdx_;
                                break;
                        
                            case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
                            case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
                            case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
                            case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
                                asic_acl.ratelimtIdx_ = asic_acl2.ratelimtIdx_;
                                break;
                            case RTL865X_ACL_PRIORITY:
                                asic_acl.priority_ = asic_acl2.priority_;
                                break;
                            case RTL865X_ACL_VID:
                                asic_acl.aclvid_ = asic_acl2.aclvid_;
                                break;                      
                            }
                            /* INV useless in entry 0 */
                            asic_acl.ipv6Invert_    = asic_acl2.ipv6Invert_;
                            //asic_acl.ipv6Combine_  = asic_acl2.ipv6Combine_ ;
                            //asic_acl.ipv6IPtunnel_ = asic_acl2.ipv6IPtunnel_;
                            seq_printf(s, " [%d-%d] rule type: %s   rule action: %s\n", acl_start, acl_temp, "IPv6", actionT[asic_acl.actionType_]);
                            if (RTL865X_ACL_IPV6 == asic_acl.ruleType_)
                            {
								memset((void *)&start_ip.v6_addr32[0], 0x00, sizeof(start_ip));
								memset((void *)&end_ip.v6_addr32[0], 0x00, sizeof(end_ip));
                            	ntohl_array((uint32 *)&asic_acl.srcIpV6Addr_.v6_addr32[0], (void *)&start_ip.v6_addr32[0], 4);
								ntohl_array((uint32 *)&asic_acl.srcIpV6AddrMask_.v6_addr32[0], (void *)&end_ip.v6_addr32[0], 4);
                                seq_printf(s, "\tsip: %x:%x:%x:%x:%x:%x:%x:%x  sipM: %x:%x:%x:%x:%x:%x:%x:%x\n",
                                        htons(start_ip.v6_addr16[0]), htons(start_ip.v6_addr16[1]), 
                                        htons(start_ip.v6_addr16[2]), htons(start_ip.v6_addr16[3]),
                                        htons(start_ip.v6_addr16[4]), htons(start_ip.v6_addr16[5]),
                                        htons(start_ip.v6_addr16[6]), htons(start_ip.v6_addr16[7]),
                                        htons(end_ip.v6_addr16[0]), htons(end_ip.v6_addr16[1]), 
                                        htons(end_ip.v6_addr16[2]), htons(end_ip.v6_addr16[3]),
                                        htons(end_ip.v6_addr16[4]), htons(end_ip.v6_addr16[5]),
                                        htons(end_ip.v6_addr16[6]), htons(end_ip.v6_addr16[7])
                                        );
								
								memset((void *)&start_ip.v6_addr32[0], 0x00, sizeof(start_ip));
								memset((void *)&end_ip.v6_addr32[0], 0x00, sizeof(end_ip));
                            	ntohl_array((uint32 *)&asic_acl.dstIpV6Addr_.v6_addr32[0], (void *)&start_ip.v6_addr32[0], 4);
								ntohl_array((uint32 *)&asic_acl.dstIpV6AddrMask_.v6_addr32[0], (void *)&end_ip.v6_addr32[0], 4);
                                seq_printf(s, "\tdip: %x:%x:%x:%x:%x:%x:%x:%x  dipM: %x:%x:%x:%x:%x:%x:%x:%x\n",
                                        htons(start_ip.v6_addr16[0]),htons( start_ip.v6_addr16[1]), 
                                        htons(start_ip.v6_addr16[2]), htons(start_ip.v6_addr16[3]),
                                        htons(start_ip.v6_addr16[4]), htons(start_ip.v6_addr16[5]),
                                        htons(start_ip.v6_addr16[6]), htons(start_ip.v6_addr16[7]),
                                        htons(end_ip.v6_addr16[0]), htons(end_ip.v6_addr16[1]), 
                                        htons(end_ip.v6_addr16[2]), htons(end_ip.v6_addr16[3]),
                                        htons(end_ip.v6_addr16[4]), htons(end_ip.v6_addr16[5]),
                                        htons(end_ip.v6_addr16[6]), htons(end_ip.v6_addr16[7])
                                        );


                            }
                            else if (RTL865X_ACL_IPV6_RANGE == asic_acl.ruleType_)
                            {
								memset((void *)&start_ip.v6_addr32[0], 0x00, sizeof(start_ip));
								memset((void *)&end_ip.v6_addr32[0], 0x00, sizeof(end_ip));
                            	ntohl_array((uint32 *)&asic_acl.srcIpV6AddrLB_.v6_addr32[0], (void *)&start_ip.v6_addr32[0], 4);
								ntohl_array((uint32 *)&asic_acl.srcIpV6AddrUB_.v6_addr32[0], (void *)&end_ip.v6_addr32[0], 4);
                                seq_printf(s, "\tsipLB: %x:%x:%x:%x:%x:%x:%x:%x  sipUB: %x:%x:%x:%x:%x:%x:%x:%x\n",
                                         htons(start_ip.v6_addr16[0]),  htons(start_ip.v6_addr16[1]), 
                                         htons(start_ip.v6_addr16[2]),  htons(start_ip.v6_addr16[3]),
                                         htons(start_ip.v6_addr16[4]),  htons(start_ip.v6_addr16[5]),
                                         htons(start_ip.v6_addr16[6]),  htons(start_ip.v6_addr16[7]),
                                         htons(end_ip.v6_addr16[0]),  htons(end_ip.v6_addr16[1]), 
                                         htons(end_ip.v6_addr16[2]),  htons(end_ip.v6_addr16[3]),
                                         htons( end_ip.v6_addr16[4]),  htons(end_ip.v6_addr16[5]),
                                         htons(end_ip.v6_addr16[6]),  htons(end_ip.v6_addr16[7])
                                        );
								memset((void *)&start_ip.v6_addr32[0], 0x00, sizeof(start_ip));
								memset((void *)&end_ip.v6_addr32[0], 0x00, sizeof(end_ip));
                            	ntohl_array((uint32 *)&asic_acl.dstIpV6AddrLB_.v6_addr32[0], (void *)&start_ip.v6_addr32[0], 4);
								ntohl_array((uint32 *)&asic_acl.dstIpV6AddrUB_.v6_addr32[0], (void *)&end_ip.v6_addr32[0], 4);
                                seq_printf(s, "\tdipLB: %x:%x:%x:%x:%x:%x:%x:%x  dipUB: %x:%x:%x:%x:%x:%x:%x:%x\n",
									 htons(start_ip.v6_addr16[0]),	htons(start_ip.v6_addr16[1]), 
									 htons(start_ip.v6_addr16[2]),	htons(start_ip.v6_addr16[3]),
									 htons(start_ip.v6_addr16[4]),	htons(start_ip.v6_addr16[5]),
									 htons(start_ip.v6_addr16[6]),	htons(start_ip.v6_addr16[7]),
									 htons(end_ip.v6_addr16[0]),  htons(end_ip.v6_addr16[1]), 
									 htons(end_ip.v6_addr16[2]),  htons(end_ip.v6_addr16[3]),
									 htons( end_ip.v6_addr16[4]),  htons(end_ip.v6_addr16[5]),
									 htons(end_ip.v6_addr16[6]),  htons(end_ip.v6_addr16[7])

                                        );
                            }
                            seq_printf(s, "\tFlowLabel: 0x%x   FlowLabelM: 0x%x\n",
                                    asic_acl.ipv6FlowLabel_, asic_acl.ipv6FlowLabelM_
                                    );
                            seq_printf(s, "\tInvert: %d   Combine1: %d   Combine2: %d   IPtunnel: %d\n",
                                    asic_acl.ipv6Invert_, asic_acl.ipv6Combine_, asic_acl2.ipv6Combine_, asic_acl.ipv6IPtunnel_
                                    );
                            seq_printf(s, "\tTrafficClassP: %d   TrafficClassM: %d   NextHeaderP: %d   NextHeaderM: %d\n",
                                    asic_acl.ipv6TrafficClass_, asic_acl.ipv6TrafficClassM_,asic_acl.ipv6NextHeader_,asic_acl.ipv6NextHeaderM_ 
                                    );
                            seq_printf(s, "\tHTTPP: %d   HTTPM: %d   IdentSDIPP: %d   IdentSDIPM: %d\n",
                                    asic_acl.ipv6HttpFilter_, asic_acl.ipv6HttpFilterM_,asic_acl.ipv6IdentSrcDstIp_,asic_acl.ipv6IdentSrcDstIpM_ 
                                    );
							#if defined(CONFIG_RTL_8197F)
							seq_printf(s, "\tIpfragApply: %d\n", asic_acl.ipfrag_apply_);
							#endif
                            /* update acl index */
                            acl_start = acl_temp;
                        }
                        
                    }
                break;
#endif
#if defined(CONFIG_RTL_8197F)
				case RTL865X_ACL_PM: 
					/* A pattern match rule occupied two entry, 
					   function _rtl865x_getAclFromAsic take one entry at a time.
					   So we need to call function _rtl865x_getAclFromAsic again. */
					acl_temp = acl_start;			 
					acl_temp++; 			/* move to the second entry index of pattern match rule */
					if (acl_temp<=acl_end)	/* it should be less than or equal to acl_end. */
					{
						memset(&asic_acl2, 0x00, sizeof(rtl865x_AclRule_t));
						if ( _rtl865x_getAclFromAsic(acl_temp, &asic_acl2) == FAILED)
							seq_printf(s,"=============%s(%d): get asic acl rule error!\n",__FUNCTION__, __LINE__);
						if ((!asic_acl2.ipv6EntryType_) && asic_acl.ipv6EntryType_)
						{
							/* ActionType and ActionField useless in entry0 */
							asic_acl.actionType_ = asic_acl2.actionType_;
							switch(asic_acl.actionType_) {
							case RTL865X_ACL_PERMIT:
							case RTL865X_ACL_REDIRECT_ETHER:
							case RTL865X_ACL_DROP:
							case RTL865X_ACL_TOCPU:
							case RTL865X_ACL_LEGACY_DROP:
							case RTL865X_ACL_DROPCPU_LOG:
							case RTL865X_ACL_MIRROR:
							case RTL865X_ACL_REDIRECT_PPPOE:
							case RTL865X_ACL_MIRROR_KEEP_MATCH:
								asic_acl.L2Idx_ = asic_acl2.L2Idx_ ;
								asic_acl.netifIdx_ =  asic_acl2.netifIdx_;
								asic_acl.pppoeIdx_ = asic_acl2.pppoeIdx_;
								 break;
						
							case RTL865X_ACL_DEFAULT_REDIRECT:
								asic_acl.nexthopIdx_ = asic_acl2.nexthopIdx_;
								break;
						
							case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
							case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
							case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
							case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
								asic_acl.ratelimtIdx_ = asic_acl2.ratelimtIdx_;
								break;
							case RTL865X_ACL_PRIORITY:
								asic_acl.priority_ = asic_acl2.priority_;
								break;
							case RTL865X_ACL_VID:
								asic_acl.aclvid_ = asic_acl2.aclvid_;
								break;						
							}
							/* INV useless in entry 0 */
							asic_acl.ipv6Invert_	= asic_acl2.ipv6Invert_;
							//asic_acl.ipv6Combine_  = asic_acl2.ipv6Combine_ ;
							//asic_acl.ipv6IPtunnel_ = asic_acl2.ipv6IPtunnel_;
							//output entry0's valid field
							seq_printf(s, " [%d-%d] rule type: %s	rule action: %s\n", acl_start, acl_temp, "Pattern match", actionT[asic_acl.actionType_]);
							seq_printf(s, "\tsportU: %d sportL: %d dportU: %d dportL: %d protocol: %d\n",
									asic_acl.un_ty.PM._spub, asic_acl.un_ty.PM._splb, asic_acl.un_ty.PM._dpub, asic_acl.un_ty.PM._dplb, asic_acl.un_ty.PM._protocol
									);
							seq_printf(s, "\tOffset0: %d Pattern0: 0x%04x PatternMask0: 0x%04x OR0: %d\n",
									asic_acl.un_ty.PM._offset0, asic_acl.un_ty.PM._pattern0, asic_acl.un_ty.PM._pm0, asic_acl.un_ty.PM._or0
									);
							seq_printf(s, "\tOffset1: %d Pattern1: 0x%04x PatternMask1: 0x%04x OR1: %d\n",
									asic_acl.un_ty.PM._offset1, asic_acl.un_ty.PM._pattern1, asic_acl.un_ty.PM._pm1, asic_acl.un_ty.PM._or1
									);
							seq_printf(s, "\tOffset2: %d Pattern2: 0x%04x PatternMask2: 0x%04x OR2: %d\n",
									asic_acl.un_ty.PM._offset2, asic_acl.un_ty.PM._pattern2, asic_acl.un_ty.PM._pm2, asic_acl.un_ty.PM._or2
									);
							seq_printf(s, "\tOffset3: %d Pattern3: 0x%04x PatternMask3: 0x%04x OR3: %d\n",
									asic_acl.un_ty.PM._offset3, asic_acl.un_ty.PM._pattern3, asic_acl.un_ty.PM._pm3, asic_acl.un_ty.PM._or3
									);

							//output entry1's valid field
							seq_printf(s, "\tOffset4: %d Pattern4: 0x%04x PatternMask4: 0x%04x OR4: %d\n",
									asic_acl2.un_ty.PM._offset4, asic_acl2.un_ty.PM._pattern4, asic_acl2.un_ty.PM._pm4, asic_acl2.un_ty.PM._or4
									);
							seq_printf(s, "\tOffset5: %d Pattern5: 0x%04x PatternMask5: 0x%04x OR5: %d\n",
									asic_acl2.un_ty.PM._offset5, asic_acl2.un_ty.PM._pattern5, asic_acl2.un_ty.PM._pm5, asic_acl2.un_ty.PM._or5
									);
							seq_printf(s, "\tOffset6: %d Pattern6: 0x%04x PatternMask6: 0x%04x OR6: %d\n",
									asic_acl2.un_ty.PM._offset6, asic_acl2.un_ty.PM._pattern6, asic_acl2.un_ty.PM._pm6, asic_acl2.un_ty.PM._or6
									);
							seq_printf(s, "\tOffset7: %d Pattern7: 0x%04x PatternMask7: 0x%04x OR7: %d\n",
									asic_acl2.un_ty.PM._offset7, asic_acl2.un_ty.PM._pattern7, asic_acl2.un_ty.PM._pm7, asic_acl2.un_ty.PM._or7
									);
							seq_printf(s, "\tSpecialOP: %d SrcPortPattern: %d SrcPortMask: %d PPPCTL: %d PPPCTLM: %d PPPCTLOR: %d\n",
									asic_acl2.un_ty.PM._specialop, asic_acl2.un_ty.PM._spa, asic_acl2.un_ty.PM._spam, asic_acl2.un_ty.PM._pppctl,
									asic_acl2.un_ty.PM._pppctlm, asic_acl2.un_ty.PM._pppctlor
									);
                            seq_printf(s, "\tInvert: %d   Combine1: %d   Combine2: %d   IPtunnel1: %d IPtunnel2: %d IpfragApply: %d\n",
                                    asic_acl2.ipv6Invert_, asic_acl2.ipv6Combine_, asic_acl2.ipv6Combine_, asic_acl.ipv6IPtunnel_, asic_acl2.ipv6IPtunnel_, asic_acl2.ipfrag_apply_
                                    );
							/* update acl index */
							acl_start = acl_temp;
						}
						
					}
				break;
#endif

				default:
					seq_printf(s, "asic_acl.ruleType_(0x%x)\n", asic_acl.ruleType_);

		}


				/* Action type */
		switch (asic_acl.actionType_)
		{

			case RTL865X_ACL_PERMIT:
			case RTL865X_ACL_REDIRECT_ETHER:
			case RTL865X_ACL_DROP:
			case RTL865X_ACL_TOCPU:
			case RTL865X_ACL_LEGACY_DROP:
			case RTL865X_ACL_DROPCPU_LOG:
			case RTL865X_ACL_MIRROR:
			case RTL865X_ACL_REDIRECT_PPPOE:
			case RTL865X_ACL_MIRROR_KEEP_MATCH:
				seq_printf(s, "\tnetifIdx: %d   pppoeIdx: %d   l2Idx:%d  ", asic_acl.netifIdx_, asic_acl.pppoeIdx_, asic_acl.L2Idx_);
				break;

			case RTL865X_ACL_PRIORITY:
				seq_printf(s, "\tprioirty: %d   ", asic_acl.priority_) ;
				break;

			case RTL865X_ACL_DEFAULT_REDIRECT:
				seq_printf(s,"\tnextHop:%d  ", asic_acl.nexthopIdx_);
				break;

			case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
			case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
			case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
			case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
				seq_printf(s, "\tratelimitIdx: %d  ", asic_acl.ratelimtIdx_);
				break;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
            case RTL865X_ACL_VID:
				seq_printf(s, "\taclvid: %d  ", asic_acl.aclvid_);
                break;
#endif
			default:
				;

			}
			seq_printf(s, "pktOpApp: %d\n", asic_acl.pktOpApp_);
			#if defined(CONFIG_RTL_8197F) || defined(CONFIG_RTL_8198C)
			if (!((asic_acl.ruleType_ == RTL865X_ACL_IPV6) || (asic_acl.ruleType_ == RTL865X_ACL_IPV6_RANGE)
				#if defined(CONFIG_RTL_8197F)
				|| (asic_acl.ruleType_ == RTL865X_ACL_PM)
				#endif
				)){
				seq_printf(s, "\tInvert: %d   Combine: %d	IPtunnel: %d",
						asic_acl.ipv6Invert_, asic_acl.ipv6Combine_, asic_acl.ipv6IPtunnel_);
				#if defined(CONFIG_RTL_8197F)
				seq_printf(s, " IpfragApply: %d\n", asic_acl.ipfrag_apply_);
				#else
				seq_printf(s, "\n");	
				#endif
				
			}
			#endif
		}
#else
		for( ; acl_start<=acl_end; acl_start++)
		{
			if (rtl8651_getAsicAclRule(acl_start, &asic_acl) == FAILED)
				seq_printf(s, "=============%s(%d): get asic acl rule error!\n",__FUNCTION__, __LINE__);

			switch(asic_acl.ruleType_)
			{
			case RTL8651_ACL_MAC:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "Ethernet", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tether type: %x   ether type mask: %x\n", asic_acl.typeLen_, asic_acl.typeLenMask_);
				seq_printf(s, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstMac_.octet[0], asic_acl.dstMac_.octet[1], asic_acl.dstMac_.octet[2],
						asic_acl.dstMac_.octet[3], asic_acl.dstMac_.octet[4], asic_acl.dstMac_.octet[5],
						asic_acl.dstMacMask_.octet[0], asic_acl.dstMacMask_.octet[1], asic_acl.dstMacMask_.octet[2],
						asic_acl.dstMacMask_.octet[3], asic_acl.dstMacMask_.octet[4], asic_acl.dstMacMask_.octet[5]
						);

				seq_printf(s, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcMac_.octet[0], asic_acl.srcMac_.octet[1], asic_acl.srcMac_.octet[2],
						asic_acl.srcMac_.octet[3], asic_acl.srcMac_.octet[4], asic_acl.srcMac_.octet[5],
						asic_acl.srcMacMask_.octet[0], asic_acl.srcMacMask_.octet[1], asic_acl.srcMacMask_.octet[2],
						asic_acl.srcMacMask_.octet[3], asic_acl.srcMacMask_.octet[4], asic_acl.srcMacMask_.octet[5]
					);
				break;

			case RTL8651_ACL_IP:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "IP", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
					);

				seq_printf(s, "\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
				seq_printf(s, "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_);
					break;

			case RTL8652_ACL_IP_RANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "IP Range", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
						);
				seq_printf(s, "\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
					seq_printf(s, "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_);
					break;
			case RTL8651_ACL_ICMP:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "ICMP", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_,
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
				break;
			case RTL8652_ACL_ICMP_IPRANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "ICMP IP RANGE", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_,
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
				break;
			case RTL8651_ACL_IGMP:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "IGMP", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
						);
				break;


			case RTL8652_ACL_IGMP_IPRANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "IGMP IP RANGE", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
						);
				break;

			case RTL8651_ACL_TCP:
					seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "TCP", actionT[asic_acl.actionType_]);
					seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
						);
				seq_printf(s, "\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
						);
				break;
			case RTL8652_ACL_TCP_IPRANGE:
					seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "TCP IP RANGE", actionT[asic_acl.actionType_]);
					seq_printf(s, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
					seq_printf(s, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
					seq_printf(s, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
						);
					seq_printf(s, "\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
					);
				break;

			case RTL8651_ACL_UDP:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start,"UDP", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
						);
				break;
			case RTL8652_ACL_UDP_IPRANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "UDP IP RANGE", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
					);
				break;

			case RTL8651_ACL_IFSEL:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "UDP", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tgidxSel: %x\n", asic_acl.gidxSel_);
				break;
			case RTL8651_ACL_SRCFILTER:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "Source Filter", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2],
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
						);
				seq_printf(s, "\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d   sipM: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
				break;

			case RTL8652_ACL_SRCFILTER_IPRANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "Source Filter(IP RANGE)", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2],
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
						);
				seq_printf(s, "\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
						);
				seq_printf(s, "\tsipU: %d.%d.%d.%d   sipL: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
				break;

			case RTL8651_ACL_DSTFILTER:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "Deatination Filter", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2],
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
						);
				seq_printf(s, "\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_,
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)),
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
						);
				seq_printf(s, "\tdip: %d.%d.%d.%d   dipM: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
						);
				break;
			case RTL8652_ACL_DSTFILTER_IPRANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "Deatination Filter(IP Range)", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2],
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
						);
				seq_printf(s, "\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_,
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)),
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
						);
				seq_printf(s, "\tdipU: %d.%d.%d.%d   dipL: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
					);
				break;

				default:
					seq_printf(s, "asic_acl.ruleType_(0x%x)\n", asic_acl.ruleType_);

		}


				/* Action type */
		switch (asic_acl.actionType_)
		{

			case RTL8651_ACL_PERMIT: /* 0x00 */
			case RTL8651_ACL_REDIRECT: /* 0x01 */
			case RTL8651_ACL_CPU: /* 0x03 */
			case RTL8651_ACL_DROP: /* 0x02, 0x04 */
			case RTL8651_ACL_DROP_LOG: /* 0x05 */
			case RTL8651_ACL_MIRROR: /* 0x06 */
			case RTL8651_ACL_REDIRECT_PPPOE: /* 0x07 */
			case RTL8651_ACL_MIRROR_KEEP_MATCH: /* 0x09 */
				seq_printf(s, "\tdvidx: %d   hp: %d   pppoeIdx: %d   nxtHop:%d  ", asic_acl.dvid_, asic_acl.priority_,
						asic_acl.pppoeIdx_, asic_acl.nextHop_);
				break;

			case RTL8651_ACL_POLICY: /* 0x08 */
				seq_printf(s, "\thp: %d   nxtHopIdx: %d  ", asic_acl.priority_, asic_acl.nhIndex);
				break;

			case RTL8651_ACL_PRIORITY: /* 0x08 */
				seq_printf(s, "\tprioirty: %d   ", asic_acl.priority) ;
				break;

			case RTL8651_ACL_DROP_RATE_EXCEED_PPS: /* 0x0a */
			case RTL8651_ACL_LOG_RATE_EXCEED_PPS: /* 0x0b */
			case RTL8651_ACL_DROP_RATE_EXCEED_BPS: /* 0x0c */
			case RTL8651_ACL_LOG_RATE_EXCEED_BPS: /* 0x0d */
				seq_printf(s, "\trlIdx: %d  ", asic_acl.rlIndex);
				break;
			default:
				;

			}
			seq_printf(s, "pktOpApp: %d\n", asic_acl.pktOpApp);

		}

#endif

		if (outRule == FALSE)
		{
			acl_start = asic_intf.outAclStart; acl_end = asic_intf.outAclEnd;
			outRule = TRUE;
			goto again;
		}
	}

#if defined (CONFIG_RTL_LOCAL_PUBLIC) ||defined(CONFIG_RTL_MULTIPLE_WAN) || defined(CONFIG_RTL_HW_VLAN_SUPPORT)
{

		outRule = FALSE;
		 rtl865x_getDefACLForNetDecisionMiss(&defInAclStart, &defInAclEnd,&defOutAclStart,&defOutAclEnd);
		acl_start = defInAclStart; acl_end = defInAclEnd;
		seq_printf(s, "\nacl_start(%d), acl_end(%d)", acl_start, acl_end);
again_forOutAcl:
		if (outRule == FALSE)
			seq_printf(s, "\n<<Default Ingress Rule for Netif Missed>>:\n");
		else
			seq_printf(s, "\n<<Default Egress Rule for Netif Missed>>:\n");

		for(; acl_start<= acl_end;acl_start++)
		{
			if ( _rtl865x_getAclFromAsic(acl_start, &asic_acl) == FAILED)
				seq_printf(s, "=============%s(%d): get asic acl rule error!\n",__FUNCTION__, __LINE__);

			switch(asic_acl.ruleType_)
			{
			case RTL865X_ACL_MAC:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "Ethernet", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tether type: %x   ether type mask: %x\n", asic_acl.typeLen_, asic_acl.typeLenMask_);
				seq_printf(s, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstMac_.octet[0], asic_acl.dstMac_.octet[1], asic_acl.dstMac_.octet[2],
						asic_acl.dstMac_.octet[3], asic_acl.dstMac_.octet[4], asic_acl.dstMac_.octet[5],
						asic_acl.dstMacMask_.octet[0], asic_acl.dstMacMask_.octet[1], asic_acl.dstMacMask_.octet[2],
						asic_acl.dstMacMask_.octet[3], asic_acl.dstMacMask_.octet[4], asic_acl.dstMacMask_.octet[5]
						);

				seq_printf(s, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcMac_.octet[0], asic_acl.srcMac_.octet[1], asic_acl.srcMac_.octet[2],
						asic_acl.srcMac_.octet[3], asic_acl.srcMac_.octet[4], asic_acl.srcMac_.octet[5],
						asic_acl.srcMacMask_.octet[0], asic_acl.srcMacMask_.octet[1], asic_acl.srcMacMask_.octet[2],
						asic_acl.srcMacMask_.octet[3], asic_acl.srcMacMask_.octet[4], asic_acl.srcMacMask_.octet[5]
					);
				break;

			case RTL865X_ACL_IP:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "IP", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
					);

				seq_printf(s, "\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
				seq_printf(s, "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_);
					break;

			case RTL865X_ACL_IP_RANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "IP Range", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.ipProto_, asic_acl.ipProtoMask_, asic_acl.ipFlag_, asic_acl.ipFlagMask_
						);
				seq_printf(s, "\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
						asic_acl.ipFOP_, asic_acl.ipFOM_, asic_acl.ipHttpFilter_, asic_acl.ipHttpFilterM_, asic_acl.ipIdentSrcDstIp_,
						asic_acl.ipIdentSrcDstIpM_
						);
					seq_printf(s, "\t<DF:%x> <MF:%x>\n", asic_acl.ipDF_, asic_acl.ipMF_);
					break;
			case RTL865X_ACL_ICMP:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "ICMP", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_,
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
				break;
			case RTL865X_ACL_ICMP_IPRANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "ICMP IP RANGE", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.icmpType_, asic_acl.icmpTypeMask_,
						asic_acl.icmpCode_, asic_acl.icmpCodeMask_);
				break;
			case RTL865X_ACL_IGMP:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "IGMP", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
						);
				break;


			case RTL865X_ACL_IGMP_IPRANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "IGMP IP RANGE", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos: %x   TosM: %x   type: %x   typeM: %x\n", asic_acl.tos_, asic_acl.tosMask_,
						asic_acl.igmpType_, asic_acl.igmpTypeMask_
						);
				break;

			case RTL865X_ACL_TCP:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "TCP", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
						);
				seq_printf(s, "\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
						);
				break;
			case RTL865X_ACL_TCP_IPRANGE:
					seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "TCP IP RANGE", actionT[asic_acl.actionType_]);
					seq_printf(s, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
					seq_printf(s, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
					seq_printf(s, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.tcpSrcPortLB_, asic_acl.tcpSrcPortUB_,
						asic_acl.tcpDstPortLB_, asic_acl.tcpDstPortUB_
						);
					seq_printf(s, "\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
						asic_acl.tcpFlag_, asic_acl.tcpFlagMask_, asic_acl.tcpURG_, asic_acl.tcpACK_,
						asic_acl.tcpPSH_, asic_acl.tcpRST_, asic_acl.tcpSYN_, asic_acl.tcpFIN_
					);
				break;

			case RTL865X_ACL_UDP:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start,"UDP", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
						);
				break;
			case RTL865X_ACL_UDP_IPRANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "UDP IP RANGE", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (asic_acl.dstIpAddr_>>24),
						((asic_acl.dstIpAddr_&0x00ff0000)>>16), ((asic_acl.dstIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstIpAddr_&0xff), (asic_acl.dstIpAddrMask_>>24), ((asic_acl.dstIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.dstIpAddrMask_&0x0000ff00)>>8), (asic_acl.dstIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (asic_acl.srcIpAddr_>>24),
						((asic_acl.srcIpAddr_&0x00ff0000)>>16), ((asic_acl.srcIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcIpAddr_&0xff), (asic_acl.srcIpAddrMask_>>24), ((asic_acl.srcIpAddrMask_&0x00ff0000)>>16),
						((asic_acl.srcIpAddrMask_&0x0000ff00)>>8), (asic_acl.srcIpAddrMask_&0xff)
						);
				seq_printf(s, "\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
						asic_acl.tos_, asic_acl.tosMask_, asic_acl.udpSrcPortLB_, asic_acl.udpSrcPortUB_,
						asic_acl.udpDstPortLB_, asic_acl.udpDstPortUB_
					);
				break;


			case RTL865X_ACL_SRCFILTER:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "Source Filter", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2],
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
						);
				seq_printf(s, "\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
						);
				seq_printf(s, "\tsip: %d.%d.%d.%d   sipM: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
				break;

			case RTL865X_ACL_SRCFILTER_IPRANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "Source Filter(IP RANGE)", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.srcFilterMac_.octet[0], asic_acl.srcFilterMac_.octet[1], asic_acl.srcFilterMac_.octet[2],
						asic_acl.srcFilterMac_.octet[3], asic_acl.srcFilterMac_.octet[4], asic_acl.srcFilterMac_.octet[5],
						asic_acl.srcFilterMacMask_.octet[0], asic_acl.srcFilterMacMask_.octet[1], asic_acl.srcFilterMacMask_.octet[2],
						asic_acl.srcFilterMacMask_.octet[3], asic_acl.srcFilterMacMask_.octet[4], asic_acl.srcFilterMacMask_.octet[5]
						);
				seq_printf(s, "\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
						asic_acl.srcFilterVlanIdx_, asic_acl.srcFilterVlanIdxMask_, asic_acl.srcFilterPort_, asic_acl.srcFilterPortMask_,
						(asic_acl.srcFilterIgnoreL3L4_==TRUE? 2: (asic_acl.srcFilterIgnoreL4_ == 1? 1: 0))
						);
				seq_printf(s, "\tsipU: %d.%d.%d.%d   sipL: %d.%d.%d.%d\n", (asic_acl.srcFilterIpAddr_>>24),
						((asic_acl.srcFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddr_&0xff), (asic_acl.srcFilterIpAddrMask_>>24),
						((asic_acl.srcFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.srcFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.srcFilterIpAddrMask_&0xff)
						);
				seq_printf(s, "\tsportL: %d   sportU: %d\n", asic_acl.srcFilterPortLowerBound_, asic_acl.srcFilterPortUpperBound_);
				break;

			case RTL865X_ACL_DSTFILTER:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "Deatination Filter", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2],
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
						);
				seq_printf(s, "\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_,
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)),
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
						);
				seq_printf(s, "\tdip: %d.%d.%d.%d   dipM: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
						);
				break;
			case RTL865X_ACL_DSTFILTER_IPRANGE:
				seq_printf(s, " [%d] rule type: %s   rule action: %s\n", acl_start, "Deatination Filter(IP Range)", actionT[asic_acl.actionType_]);
				seq_printf(s, "\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
						asic_acl.dstFilterMac_.octet[0], asic_acl.dstFilterMac_.octet[1], asic_acl.dstFilterMac_.octet[2],
						asic_acl.dstFilterMac_.octet[3], asic_acl.dstFilterMac_.octet[4], asic_acl.dstFilterMac_.octet[5],
						asic_acl.dstFilterMacMask_.octet[0], asic_acl.dstFilterMacMask_.octet[1], asic_acl.dstFilterMacMask_.octet[2],
						asic_acl.dstFilterMacMask_.octet[3], asic_acl.dstFilterMacMask_.octet[4], asic_acl.dstFilterMacMask_.octet[5]
						);
				seq_printf(s, "\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
						asic_acl.dstFilterVlanIdx_, asic_acl.dstFilterVlanIdxMask_,
						(asic_acl.dstFilterIgnoreL3L4_==TRUE? 2: (asic_acl.dstFilterIgnoreL4_ == 1? 1: 0)),
						asic_acl.dstFilterPortLowerBound_, asic_acl.dstFilterPortUpperBound_
						);
				seq_printf(s, "\tdipU: %d.%d.%d.%d   dipL: %d.%d.%d.%d\n", (asic_acl.dstFilterIpAddr_>>24),
						((asic_acl.dstFilterIpAddr_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddr_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddr_&0xff), (asic_acl.dstFilterIpAddrMask_>>24),
						((asic_acl.dstFilterIpAddrMask_&0x00ff0000)>>16), ((asic_acl.dstFilterIpAddrMask_&0x0000ff00)>>8),
						(asic_acl.dstFilterIpAddrMask_&0xff)
					);
				break;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
            case RTL865X_ACL_IPV6: /* IP Rule Type: 0x0010 */
            case RTL865X_ACL_IPV6_RANGE:
                /* a ipv6 rule occupied  two entry, function _rtl865x_getAclFromAsic take one entry at a time,
                             * so, need to call function _rtl865x_getAclFromAsic again.
                            */
                //rtl865x_AclRule_t asic_acl2;
                //unsigned int acl_temp = acl_start;
                acl_temp = acl_start;
                acl_temp++;
                if (acl_temp<=acl_end)//the second entry index of ipv6 rule, should  less than or equal to acl_end.
                {
                    memset(&asic_acl2, 0x00, sizeof(rtl865x_AclRule_t));
                    if ( _rtl865x_getAclFromAsic(acl_temp, &asic_acl2) == FAILED)
                        seq_printf(s, "=============%s(%d): get asic acl rule error!\n",__FUNCTION__, __LINE__);
                    if ((!asic_acl2.ipv6EntryType_) && asic_acl.ipv6EntryType_)
                    {
                        asic_acl.dstIpV6Addr_.v6_addr32[3] = asic_acl2.dstIpV6Addr_.v6_addr32[3];
                        asic_acl.dstIpV6Addr_.v6_addr32[2] = asic_acl2.dstIpV6Addr_.v6_addr32[2];
                        asic_acl.dstIpV6Addr_.v6_addr32[1] = asic_acl2.dstIpV6Addr_.v6_addr32[1];
                        asic_acl.dstIpV6Addr_.v6_addr32[0] = asic_acl2.dstIpV6Addr_.v6_addr32[0];
                        
                        asic_acl.dstIpV6AddrMask_.v6_addr32[3] = asic_acl2.dstIpV6AddrMask_.v6_addr32[3];
                        asic_acl.dstIpV6AddrMask_.v6_addr32[2] = asic_acl2.dstIpV6AddrMask_.v6_addr32[2];
                        asic_acl.dstIpV6AddrMask_.v6_addr32[1] = asic_acl2.dstIpV6AddrMask_.v6_addr32[1];
                        asic_acl.dstIpV6AddrMask_.v6_addr32[0] = asic_acl2.dstIpV6AddrMask_.v6_addr32[0];

                        asic_acl.ipv6TrafficClass_ = asic_acl2.ipv6TrafficClass_;
                        asic_acl.ipv6TrafficClassM_ = asic_acl2.ipv6TrafficClassM_; 
                        asic_acl.ipv6NextHeader_    = asic_acl2.ipv6NextHeader_; 
                        asic_acl.ipv6NextHeaderM_   = asic_acl2.ipv6NextHeaderM_; 
                        asic_acl.ipv6HttpFilter_    = asic_acl2.ipv6HttpFilter_;
                        asic_acl.ipv6HttpFilterM_ = asic_acl2.ipv6HttpFilterM_;
                        asic_acl.ipv6IdentSrcDstIp_ = asic_acl2.ipv6IdentSrcDstIp_;
                        asic_acl.ipv6IdentSrcDstIpM_ = asic_acl2.ipv6IdentSrcDstIpM_;
                        /* ActionType and ActionField useless in entry0 */
                        asic_acl.actionType_ = asic_acl2.actionType_;
                        switch(asic_acl.actionType_) {
                    
                        case RTL865X_ACL_PERMIT:
                        case RTL865X_ACL_REDIRECT_ETHER:
                        case RTL865X_ACL_DROP:
                        case RTL865X_ACL_TOCPU:
                        case RTL865X_ACL_LEGACY_DROP:
                        case RTL865X_ACL_DROPCPU_LOG:
                        case RTL865X_ACL_MIRROR:
                        case RTL865X_ACL_REDIRECT_PPPOE:
                        case RTL865X_ACL_MIRROR_KEEP_MATCH:
                            asic_acl.L2Idx_ = asic_acl2.L2Idx_ ;
                            asic_acl.netifIdx_ =  asic_acl2.netifIdx_;
                            asic_acl.pppoeIdx_ = asic_acl2.pppoeIdx_;
                             break;
                    
                        case RTL865X_ACL_DEFAULT_REDIRECT:
                            asic_acl.nexthopIdx_ = asic_acl2.nexthopIdx_;
                            break;
                    
                        case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
                        case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
                        case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
                        case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
                            asic_acl.ratelimtIdx_ = asic_acl2.ratelimtIdx_;
                            break;
                        case RTL865X_ACL_PRIORITY:
                            asic_acl.priority_ = asic_acl2.priority_;
                            break;
                        case RTL865X_ACL_VID:
                            asic_acl.aclvid_ = asic_acl2.aclvid_;
                            break;                      
                        }
                        /* INV useless in entry 0 */
                        asic_acl.ipv6Invert_    = asic_acl2.ipv6Invert_;
                        //asic_acl.ipv6Combine_  = asic_acl2.ipv6Combine_ ;
                        //asic_acl.ipv6IPtunnel_ = asic_acl2.ipv6IPtunnel_;
                        seq_printf(s, " [%d-%d] rule type: %s   rule action: %s\n", acl_start, acl_temp, "IPv6", actionT[asic_acl.actionType_]);
                        if (RTL865X_ACL_IPV6 == asic_acl.ruleType_)
                        {
							memset((void *)&start_ip.v6_addr32[0], 0x00, sizeof(start_ip));
							memset((void *)&end_ip.v6_addr32[0], 0x00, sizeof(end_ip));
							ntohl_array((uint32 *)&asic_acl.srcIpV6Addr_.v6_addr32[0], (void *)&start_ip.v6_addr32[0], 4);
							ntohl_array((uint32 *)&asic_acl.srcIpV6AddrMask_.v6_addr32[0], (void *)&end_ip.v6_addr32[0], 4);
                            seq_printf(s, "\tsip: %x:%x:%x:%x:%x:%x:%x:%x  sipM: %x:%x:%x:%x:%x:%x:%x:%x\n",
									htons(start_ip.v6_addr16[0]), htons(start_ip.v6_addr16[1]), 
									htons(start_ip.v6_addr16[2]), htons(start_ip.v6_addr16[3]),
									htons(start_ip.v6_addr16[4]), htons(start_ip.v6_addr16[5]),
									htons(start_ip.v6_addr16[6]), htons(start_ip.v6_addr16[7]),
									htons(end_ip.v6_addr16[0]), htons(end_ip.v6_addr16[1]), 
									htons(end_ip.v6_addr16[2]), htons(end_ip.v6_addr16[3]),
									htons(end_ip.v6_addr16[4]), htons(end_ip.v6_addr16[5]),
									htons(end_ip.v6_addr16[6]), htons(end_ip.v6_addr16[7])
                                    );
							memset((void *)&start_ip.v6_addr32[0], 0x00, sizeof(start_ip));
							memset((void *)&end_ip.v6_addr32[0], 0x00, sizeof(end_ip));
							ntohl_array((uint32 *)&asic_acl.dstIpV6Addr_.v6_addr32[0], (void *)&start_ip.v6_addr32[0], 4);
							ntohl_array((uint32 *)&asic_acl.dstIpV6AddrMask_.v6_addr32[0], (void *)&end_ip.v6_addr32[0], 4);
                            seq_printf(s, "\tdip: %x:%x:%x:%x:%x:%x:%x:%x  dipM: %x:%x:%x:%x:%x:%x:%x:%x\n",
									htons(start_ip.v6_addr16[0]),htons( start_ip.v6_addr16[1]), 
									htons(start_ip.v6_addr16[2]), htons(start_ip.v6_addr16[3]),
									htons(start_ip.v6_addr16[4]), htons(start_ip.v6_addr16[5]),
									htons(start_ip.v6_addr16[6]), htons(start_ip.v6_addr16[7]),
									htons(end_ip.v6_addr16[0]), htons(end_ip.v6_addr16[1]), 
									htons(end_ip.v6_addr16[2]), htons(end_ip.v6_addr16[3]),
									htons(end_ip.v6_addr16[4]), htons(end_ip.v6_addr16[5]),
									htons(end_ip.v6_addr16[6]), htons(end_ip.v6_addr16[7])
                                    );


                        }
                        else if (RTL865X_ACL_IPV6_RANGE == asic_acl.ruleType_)
                        {
                        	
							memset((void *)&start_ip.v6_addr32[0], 0x00, sizeof(start_ip));
							memset((void *)&end_ip.v6_addr32[0], 0x00, sizeof(end_ip));
							ntohl_array((uint32 *)&asic_acl.srcIpV6AddrLB_.v6_addr32[0], (void *)&start_ip.v6_addr32[0], 4);
							ntohl_array((uint32 *)&asic_acl.srcIpV6AddrUB_.v6_addr32[0], (void *)&end_ip.v6_addr32[0], 4);
                            seq_printf(s, "\tsipLB: %x:%x:%x:%x:%x:%x:%x:%x  sipUB: %x:%x:%x:%x:%x:%x:%x:%x\n",
									 htons(start_ip.v6_addr16[0]),  htons(start_ip.v6_addr16[1]), 
									 htons(start_ip.v6_addr16[2]),	htons(start_ip.v6_addr16[3]),
									 htons(start_ip.v6_addr16[4]),	htons(start_ip.v6_addr16[5]),
									 htons(start_ip.v6_addr16[6]),	htons(start_ip.v6_addr16[7]),
									 htons(end_ip.v6_addr16[0]),  htons(end_ip.v6_addr16[1]), 
									 htons(end_ip.v6_addr16[2]),  htons(end_ip.v6_addr16[3]),
									 htons( end_ip.v6_addr16[4]),  htons(end_ip.v6_addr16[5]),
									 htons(end_ip.v6_addr16[6]),  htons(end_ip.v6_addr16[7])
                                    );
							memset((void *)&start_ip.v6_addr32[0], 0x00, sizeof(start_ip));
							memset((void *)&end_ip.v6_addr32[0], 0x00, sizeof(end_ip));
							ntohl_array((uint32 *)&asic_acl.dstIpV6AddrLB_.v6_addr32[0], (void *)&start_ip.v6_addr32[0], 4);
							ntohl_array((uint32 *)&asic_acl.dstIpV6AddrUB_.v6_addr32[0], (void *)&end_ip.v6_addr32[0], 4);
                            seq_printf(s, "\tdipLB: %x:%x:%x:%x:%x:%x:%x:%x  dipUB: %x:%x:%x:%x:%x:%x:%x:%x\n",
									htons(start_ip.v6_addr16[0]),  htons(start_ip.v6_addr16[1]), 
									htons(start_ip.v6_addr16[2]),  htons(start_ip.v6_addr16[3]),
									htons(start_ip.v6_addr16[4]),  htons(start_ip.v6_addr16[5]),
									htons(start_ip.v6_addr16[6]),  htons(start_ip.v6_addr16[7]),
									htons(end_ip.v6_addr16[0]),  htons(end_ip.v6_addr16[1]), 
									htons(end_ip.v6_addr16[2]),  htons(end_ip.v6_addr16[3]),
									htons( end_ip.v6_addr16[4]),  htons(end_ip.v6_addr16[5]),
									htons(end_ip.v6_addr16[6]),  htons(end_ip.v6_addr16[7])
                                    );
                        }
                        seq_printf(s, "\tFlowLabel: 0x%x   FlowLabelM: 0x%x\n",
                                asic_acl.ipv6FlowLabel_, asic_acl.ipv6FlowLabelM_
                                );
                        seq_printf(s, "\tInvert: %d   Combine: %d   IPtunnel: %d\n",
                                asic_acl.ipv6Invert_, asic_acl.ipv6Combine_,asic_acl.ipv6IPtunnel_
                                );
                        seq_printf(s, "\tTrafficClassP: %d   TrafficClassM: %d   NextHeaderP: %d   NextHeaderM: %d\n",
                                asic_acl.ipv6TrafficClass_, asic_acl.ipv6TrafficClassM_,asic_acl.ipv6NextHeader_,asic_acl.ipv6NextHeaderM_ 
                                );
                        seq_printf(s, "\tHTTPP: %d   HTTPM: %d   IdentSDIPP: %d   IdentSDIPM: %d\n",
                                asic_acl.ipv6HttpFilter_, asic_acl.ipv6HttpFilterM_,asic_acl.ipv6IdentSrcDstIp_,asic_acl.ipv6IdentSrcDstIpM_ 
                                );
                        /* update acl index */
                        acl_start = acl_temp;
                    }
                    
                }
            break;
#endif
#if defined(CONFIG_RTL_8197F)
			case RTL865X_ACL_PM: 
				/* A pattern match rule occupied two entry, 
				   function _rtl865x_getAclFromAsic take one entry at a time.
				   So we need to call function _rtl865x_getAclFromAsic again. */
				acl_temp = acl_start;			 
				acl_temp++; 			/* move to the second entry index of pattern match rule */
				if (acl_temp<=acl_end)	/* it should be less than or equal to acl_end. */
				{
					memset(&asic_acl2, 0x00, sizeof(rtl865x_AclRule_t));
					if ( _rtl865x_getAclFromAsic(acl_temp, &asic_acl2) == FAILED)
						seq_printf(s,"=============%s(%d): get asic acl rule error!\n",__FUNCTION__, __LINE__);
					if ((!asic_acl2.ipv6EntryType_) && asic_acl.ipv6EntryType_)
					{
						/* ActionType and ActionField useless in entry0 */
						asic_acl.actionType_ = asic_acl2.actionType_;
						switch(asic_acl.actionType_) {
						case RTL865X_ACL_PERMIT:
						case RTL865X_ACL_REDIRECT_ETHER:
						case RTL865X_ACL_DROP:
						case RTL865X_ACL_TOCPU:
						case RTL865X_ACL_LEGACY_DROP:
						case RTL865X_ACL_DROPCPU_LOG:
						case RTL865X_ACL_MIRROR:
						case RTL865X_ACL_REDIRECT_PPPOE:
						case RTL865X_ACL_MIRROR_KEEP_MATCH:
							asic_acl.L2Idx_ = asic_acl2.L2Idx_ ;
							asic_acl.netifIdx_ =  asic_acl2.netifIdx_;
							asic_acl.pppoeIdx_ = asic_acl2.pppoeIdx_;
							 break;
					
						case RTL865X_ACL_DEFAULT_REDIRECT:
							asic_acl.nexthopIdx_ = asic_acl2.nexthopIdx_;
							break;
					
						case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
						case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
						case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
						case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
							asic_acl.ratelimtIdx_ = asic_acl2.ratelimtIdx_;
							break;
						case RTL865X_ACL_PRIORITY:
							asic_acl.priority_ = asic_acl2.priority_;
							break;
						case RTL865X_ACL_VID:
							asic_acl.aclvid_ = asic_acl2.aclvid_;
							break;						
						}
						/* INV useless in entry 0 */
						asic_acl.ipv6Invert_	= asic_acl2.ipv6Invert_;
						//asic_acl.ipv6Combine_  = asic_acl2.ipv6Combine_ ;
						//asic_acl.ipv6IPtunnel_ = asic_acl2.ipv6IPtunnel_;
						//output entry0's valid field
						seq_printf(s, " [%d-%d] rule type: %s	rule action: %s\n", acl_start, acl_temp, "Pattern match", actionT[asic_acl.actionType_]);
						seq_printf(s, "\tsportU: %d sportL: %d dportU: %d dportL: %d protocol: %d\n",
								asic_acl.un_ty.PM._spub, asic_acl.un_ty.PM._splb, asic_acl.un_ty.PM._dpub, asic_acl.un_ty.PM._dplb, asic_acl.un_ty.PM._protocol
								);
						seq_printf(s, "\tOffset0: %d Pattern0: 0x%04x PatternMask0: 0x%04x OR0: %d\n",
								asic_acl.un_ty.PM._offset0, asic_acl.un_ty.PM._pattern0, asic_acl.un_ty.PM._pm0, asic_acl.un_ty.PM._or0
								);
						seq_printf(s, "\tOffset1: %d Pattern1: 0x%04x PatternMask1: 0x%04x OR1: %d\n",
								asic_acl.un_ty.PM._offset1, asic_acl.un_ty.PM._pattern1, asic_acl.un_ty.PM._pm1, asic_acl.un_ty.PM._or1
								);
						seq_printf(s, "\tOffset2: %d Pattern2: 0x%04x PatternMask2: 0x%04x OR2: %d\n",
								asic_acl.un_ty.PM._offset2, asic_acl.un_ty.PM._pattern2, asic_acl.un_ty.PM._pm2, asic_acl.un_ty.PM._or2
								);
						seq_printf(s, "\tOffset3: %d Pattern3: 0x%04x PatternMask3: 0x%04x OR3: %d\n",
								asic_acl.un_ty.PM._offset3, asic_acl.un_ty.PM._pattern3, asic_acl.un_ty.PM._pm3, asic_acl.un_ty.PM._or3
								);

						//output entry1's valid field
						seq_printf(s, "\tOffset4: %d Pattern4: 0x%04x PatternMask4: 0x%04x OR4: %d\n",
								asic_acl2.un_ty.PM._offset4, asic_acl2.un_ty.PM._pattern4, asic_acl2.un_ty.PM._pm4, asic_acl2.un_ty.PM._or4
								);
						seq_printf(s, "\tOffset5: %d Pattern5: 0x%04x PatternMask5: 0x%04x OR5: %d\n",
								asic_acl2.un_ty.PM._offset5, asic_acl2.un_ty.PM._pattern5, asic_acl2.un_ty.PM._pm5, asic_acl2.un_ty.PM._or5
								);
						seq_printf(s, "\tOffset6: %d Pattern6: 0x%04x PatternMask6: 0x%04x OR6: %d\n",
								asic_acl2.un_ty.PM._offset6, asic_acl2.un_ty.PM._pattern6, asic_acl2.un_ty.PM._pm6, asic_acl2.un_ty.PM._or6
								);
						seq_printf(s, "\tOffset7: %d Pattern7: 0x%04x PatternMask7: 0x%04x OR7: %d\n",
								asic_acl2.un_ty.PM._offset7, asic_acl2.un_ty.PM._pattern7, asic_acl2.un_ty.PM._pm7, asic_acl2.un_ty.PM._or7
								);
						seq_printf(s, "\tSpecialOP: %d SrcPortPattern: %d SrcPortMask: %d PPPCTL: %d PPPCTLM: %d PPPCTLOR: %d\n",
								asic_acl2.un_ty.PM._specialop, asic_acl2.un_ty.PM._spa, asic_acl2.un_ty.PM._spam, asic_acl2.un_ty.PM._pppctl,
								asic_acl2.un_ty.PM._pppctlm, asic_acl2.un_ty.PM._pppctlor
								);
						seq_printf(s, "\tInvert: %d   Combine1: %d	 Combine2: %d	IPtunnel1: %d IPtunnel2: %d IpfragApply: %d\n",
								asic_acl2.ipv6Invert_, asic_acl2.ipv6Combine_, asic_acl2.ipv6Combine_, asic_acl.ipv6IPtunnel_, asic_acl2.ipv6IPtunnel_, asic_acl2.ipfrag_apply_
								);
						/* update acl index */
						acl_start = acl_temp;
					}
					
				}
			break;
#endif

				default:
					seq_printf(s, "asic_acl.ruleType_(0x%x)\n", asic_acl.ruleType_);

		}


				/* Action type */
		switch (asic_acl.actionType_)
		{

			case RTL865X_ACL_PERMIT:
			case RTL865X_ACL_REDIRECT_ETHER:
			case RTL865X_ACL_DROP:
			case RTL865X_ACL_TOCPU:
			case RTL865X_ACL_LEGACY_DROP:
			case RTL865X_ACL_DROPCPU_LOG:
			case RTL865X_ACL_MIRROR:
			case RTL865X_ACL_REDIRECT_PPPOE:
			case RTL865X_ACL_MIRROR_KEEP_MATCH:
				seq_printf(s, "\tnetifIdx: %d   pppoeIdx: %d   l2Idx:%d  ", asic_acl.netifIdx_, asic_acl.pppoeIdx_, asic_acl.L2Idx_);
				break;

			case RTL865X_ACL_PRIORITY:
				seq_printf(s, "\tprioirty: %d   ", asic_acl.priority_) ;
				break;

			case RTL865X_ACL_DEFAULT_REDIRECT:
				seq_printf(s,"\tnextHop:%d  ", asic_acl.nexthopIdx_);
				break;

			case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
			case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
			case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
			case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
				seq_printf(s, "\tratelimitIdx: %d  ", asic_acl.ratelimtIdx_);
				break;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
            case RTL865X_ACL_VID:
                seq_printf(s, "\taclvid: %d  ", asic_acl.aclvid_);
                break;
#endif
			default:
				;

			}
			seq_printf(s, "pktOpApp: %d\n", asic_acl.pktOpApp_);
			#if defined(CONFIG_RTL_8197F) || defined(CONFIG_RTL_8198C)
			if (!((asic_acl.ruleType_ == RTL865X_ACL_IPV6) || (asic_acl.ruleType_ == RTL865X_ACL_IPV6_RANGE)
				#if defined(CONFIG_RTL_8197F)
				|| (asic_acl.ruleType_ == RTL865X_ACL_PM)
				#endif
				)){
				seq_printf(s, "\tInvert: %d   Combine: %d	IPtunnel: %d",
						asic_acl.ipv6Invert_, asic_acl.ipv6Combine_, asic_acl.ipv6IPtunnel_);
				#if defined(CONFIG_RTL_8197F)
				seq_printf(s, " IpfragApply: %d\n", asic_acl.ipfrag_apply_);
				#else
				seq_printf(s, "\n");	
				#endif
				
			}
			#endif
		}

		if (outRule == FALSE)
		{
			acl_start = defOutAclStart; acl_end = defOutAclEnd;
			outRule = TRUE;
			goto again_forOutAcl;
		}
	}
#endif
	return 0;
}

int acl_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, acl_show, NULL));
}

struct file_operations acl_single_seq_file_operations = {
        .open           = acl_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

#if defined(CONFIG_RTL_IGMP_SNOOPING)
extern int igmp_show(struct seq_file *s, void *v);
extern int igmp_write(struct file *file, const char __user *buffer, size_t count, loff_t *data);
int igmp_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, igmp_show, NULL));
}

struct file_operations igmp_single_seq_file_operations = {
        .open           = igmp_single_open,
        .read           = seq_read,
        .write		= igmp_write,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER
int aclChains_show(struct seq_file *s, void *v)
{
#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	rtl865x_show_allAclChains(s);
#endif
	return 0;
}

int aclChains_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, aclChains_show, NULL));
}

struct file_operations aclChains_single_seq_file_operations = {
        .open           = aclChains_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
int qosRule_show(struct seq_file *s, void *v)
{
	rtl865x_show_allQosAcl(s);
	return 0;
}

int qosRule_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, qosRule_show, NULL));
}

struct file_operations qosRule_single_seq_file_operations = {
        .open           = qosRule_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

#endif

#ifdef CONFIG_RTL_PROC_NEW
static int32 hs_read(struct seq_file *s, void *v)
{
	hsb_param_t *hsb_r, dummy_hsb_r;
	hsa_param_t *hsa_r, dummy_hsa_r;
	ipaddr_t addr;
	char addr_s[100];

	hsb_r = &dummy_hsb_r;
	hsa_r = &dummy_hsa_r;
	memset((void*)hsb_r,0,sizeof(hsb_r));
	memset((void*)hsa_r,0,sizeof(hsa_r));

	virtualMacGetHsb( hsb_r );
	{
		seq_printf(s,"HSB(");
		seq_printf(s,"\ttype:%d",hsb_r->type);

		seq_printf(s,"\tspa:%d",hsb_r->spa);
		seq_printf(s,"\tlen:%d",hsb_r->len);
		seq_printf(s,"\tvid :%d\n",hsb_r->vid);
		seq_printf(s,"\tpppoe:%d",hsb_r->pppoeif);

		/* Protocol contents */
		seq_printf(s,"\ttagif:%d\tpppoeId:%d",hsb_r->tagif,hsb_r->pppoeid);
		seq_printf(s,"\tethrtype:0x%04x\n",hsb_r->ethtype);
		seq_printf(s,"\tllc_other:%d\tsnap:%d\n",hsb_r->llcothr,hsb_r->snap);
		seq_printf(s,"\tda:%02x-%02x-%02x-%02x-%02x-%02x",hsb_r->da[0],hsb_r->da[1],hsb_r->da[2],hsb_r->da[3],hsb_r->da[4],hsb_r->da[5]);
		seq_printf(s,"\tsa:%02x-%02x-%02x-%02x-%02x-%02x\n",hsb_r->sa[0],hsb_r->sa[1],hsb_r->sa[2],hsb_r->sa[3],hsb_r->sa[4],hsb_r->sa[5]);

		addr = ntohl( hsb_r->sip);
		inet_ntoa_r(addr, addr_s);
		seq_printf(s,"\tsip:%s(hex:%08x)   ",addr_s,hsb_r->sip);
		seq_printf(s,"\tsprt:%d (hex:%x)\n ",(int)hsb_r->sprt,hsb_r->sprt);
		addr  = ntohl(hsb_r->dip);
		inet_ntoa_r(addr, addr_s);
		seq_printf(s,"\tdip:%s(hex:%08x) ",addr_s,hsb_r->dip);;
		seq_printf(s,"\tdprt:%d(hex:%08x)\n",hsb_r->dprt,hsb_r->dprt);

		seq_printf(s,"\tipptl:%d,",(int)hsb_r->ipptl);
		seq_printf(s,"\tipflg:%d,",hsb_r->ipfg);
		seq_printf(s,"\tiptos:%d,",hsb_r->iptos);
		seq_printf(s,"\ttcpflg:%d\n",hsb_r->tcpfg);

		seq_printf(s,"\tdirtx:%d,",hsb_r->dirtx);
		seq_printf(s,"\tprtnmat:%d",hsb_r->patmatch);

		seq_printf(s,"\tudp_nocs:%d",hsb_r->udpnocs);
		seq_printf(s,"\tttlst:0x%x\n",hsb_r->ttlst);


		seq_printf(s,"\thp:%d",hsb_r->hiprior);
		seq_printf(s,"\tl3csok:%d\tl4csok:%d\tipfragif:%d\n",hsb_r->l3csok,hsb_r->l4csok,hsb_r->ipfo0_n);

	 	seq_printf(s,"\textspa:%d",hsb_r->extspa);
		seq_printf(s,"\turlmch:%d\n",hsb_r->urlmch);

#if	defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
   
		seq_printf(s,"\tipv4_opt:%d",hsb_r->ipv4_opt);
		seq_printf(s,"\tcputag_if:%d\n",hsb_r->cputag_if);

		seq_printf(s,"\tipv6_ext:%d",hsb_r->ipv6_ext);
		seq_printf(s,"\tipv6fo:%d\n",hsb_r->ipv6fo);

		seq_printf(s,"\tipv6flag:%d",hsb_r->ipv6flag);
		seq_printf(s,"\tqpri:%d\n",hsb_r->qpri);

		seq_printf(s,"\tptp_pkt:%d",hsb_r->ptp_pkt);
		seq_printf(s,"\tptp_ver:%d\n",hsb_r->ptp_ver);

		seq_printf(s,"\tptp_typ:%d",hsb_r->ptp_typ);
		seq_printf(s,"\tipver_1st:%d\n",hsb_r->ipver_1st);

		seq_printf(s,"\tsip6:\t%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n"
					,hsb_r->sipv6_127_96>>16,hsb_r->sipv6_127_96&0xFFFF
					,hsb_r->sipv6_95_64>>16,hsb_r->sipv6_95_64&0xFFFF
					,hsb_r->sipv6_63_32>>16,hsb_r->sipv6_63_32&0xFFFF
					,hsb_r->sipv6_31_0>>16,hsb_r->sipv6_31_0&0xFFFF);

		seq_printf(s,"\tdip6:\t%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n"
					,hsb_r->dipv6_127_96>>16,hsb_r->dipv6_127_96&0xFFFF
					,hsb_r->dipv6_95_64>>16,hsb_r->dipv6_95_64&0xFFFF
					,hsb_r->dipv6_63_32>>16,hsb_r->dipv6_63_32&0xFFFF
					,hsb_r->dipv6_31_0>>16,hsb_r->dipv6_31_0&0xFFFF);

		seq_printf(s,"\thop_limit:0x%x",hsb_r->hop_limit);
		seq_printf(s,"\ttra_cla:0x%x\n",hsb_r->tra_cla);

		seq_printf(s,"\tflow_lab:0x%x",hsb_r->flow_lab);
		seq_printf(s,"\tnxt_hdr:0x%x\n",hsb_r->nxt_hdr);
		seq_printf(s,"\tipv4:0x%x",hsb_r->ipv4);
		seq_printf(s,"\tipv6:0x%x\n",hsb_r->ipv6);
		seq_printf(s,"\tip_len:0x%x",hsb_r->ip_len);

		seq_printf(s,"\ttun_len:0x%x\n",hsb_r->tun_len);
#endif

#if	defined(CONFIG_RTL_8197F)
		seq_printf(s,"\tl3_cso:0x%x",hsb_r->l3_cso);
		seq_printf(s,"\tl4_cso:0x%x\n",hsb_r->l4_cso);

		seq_printf(s,"\tl3_csum:0x%x",hsb_r->l3_csum);
		seq_printf(s,"\tl4_csum:0x%x\n",hsb_r->l4_csum);	
#endif	

		seq_printf(s,")\n");
	}

	virtualMacGetHsa( hsa_r );
	{
		seq_printf(s,("HSA("));
		seq_printf(s,"\tmac:%02x-%02x-%02x-%02x-%02x-%02x\n",hsa_r->nhmac[0],hsa_r->nhmac[1],hsa_r->nhmac[2],hsa_r->nhmac[3],hsa_r->nhmac[4],hsa_r->nhmac[5]);

		addr =ntohl( hsa_r->trip);
		inet_ntoa_r(addr, addr_s);
		seq_printf(s,"\ttrip:%s(hex:%08x)",addr_s,hsa_r->trip);
		seq_printf(s,"\tprt:%d\tipmcast:%d\n",hsa_r->port,hsa_r->ipmcastr);
		seq_printf(s,"\tl3cs:0x%x",hsa_r->l3csdt);
		seq_printf(s,"\tl4cs:0x%x",hsa_r->l4csdt);
		seq_printf(s,"\tInternal NETIF:%d",hsa_r->egif);
		seq_printf(s,"\tl2tr:%d,\n ",hsa_r->l2tr);
		seq_printf(s,"\tl34tr:%d",hsa_r->l34tr);
		seq_printf(s,"\tdirtx:%d",hsa_r->dirtxo);
		seq_printf(s,"\ttype:%d",hsa_r->typeo);

#if defined(CONFIG_RTL_8197F)
		seq_printf(s,"\tsnapo:%d",hsa_r->llcexist);	/* 1: LLC exists */
#else
		seq_printf(s,"\tsnapo:%d",hsa_r->snapo);	/* 1: SNAP exists (copy from hsb.snap) */
#endif

		seq_printf(s,"\twhy2cpu 0x%x (%d)\n",hsa_r->why2cpu,hsa_r->why2cpu);
		seq_printf(s,"\tpppif:%d",hsa_r->pppoeifo);
		seq_printf(s,"\tpppid:%d",hsa_r->pppidx);
		seq_printf(s,"\tttl_1:0x%x",hsa_r->ttl_1if);
		seq_printf(s,"\tdpc:%d,",hsa_r->dpc);
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		seq_printf(s,"\tleno:%d(0x%x) \tmirrort:%d\n",hsa_r->leno,hsa_r->leno,hsa_r->mirrort);
#else   
		seq_printf(s,"\tleno:%d(0x%x)\n",hsa_r->leno,hsa_r->leno);
#endif
		seq_printf(s,"\tl3CrcOk:%d",hsa_r->l3csoko);
		seq_printf(s,"\tl4CrcOk:%d",hsa_r->l4csoko);
		seq_printf(s,"\tfrag:%d",hsa_r->frag);
		seq_printf(s,"\tlastFrag:%d\n",hsa_r->lastfrag);



		seq_printf(s,"\tsvid:0x%x",hsa_r->svid);
		seq_printf(s,"\tdvid:%d(0x%x)",hsa_r->dvid,hsa_r->dvid);
		seq_printf(s,"\tdestination interface :%d\n",hsa_r->difid);
		seq_printf(s,"\trxtag:%d",hsa_r->rxtag);
		seq_printf(s,"\t\tdvtag:0x%x",hsa_r->dvtag);
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		seq_printf(s,"\tspa:%d \tipv4_1st:%d",hsa_r->spao,hsa_r->ipv4_1st);
#else        
		seq_printf(s,"\tspa:%d",hsa_r->spao);
#endif       
		seq_printf(s,"\tdpext:0x%x\thwfwrd:%d\n",hsa_r->dpext,hsa_r->hwfwrd);
		seq_printf(s,"\tspcp:%d",hsa_r->spcp);
		seq_printf(s,"\t\tpriority:%d",hsa_r->priority);

		seq_printf(s,"\tdp:0x%x\n",hsa_r->dp);

#if	defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
 
		seq_printf(s,"\tcputag:%d",hsa_r->cputag);
		seq_printf(s,"\tptp_pkt:%d",hsa_r->ptp_pkt);
		seq_printf(s,"\tptp_v2:%d",hsa_r->ptp_v2);
		seq_printf(s,"\tptp_type:0x%x\n",hsa_r->ptp_type);

		seq_printf(s,"\trmdp:0x%x",hsa_r->rmdp);
		seq_printf(s,"\tdpri:0x%x",hsa_r->dpri);
		seq_printf(s,"\tmdf:0x%x\n",hsa_r->mdf);
/*
		seq_printf(s,"\tsipv6_31_0:0x%x",hsa_r->sipv6_31_0);
		seq_printf(s,"\tsipv6_63_32:0x%x",hsa_r->sipv6_63_32);
		seq_printf(s,"\tsipv6_95_64:0x%x",hsa_r->sipv6_95_64);
		seq_printf(s,"\tsipv6_127_96:0x%x\n",hsa_r->sipv6_127_96);

		seq_printf(s,"\tdipv6_31_0:0x%x",hsa_r->dipv6_31_0);
		seq_printf(s,"\tdipv6_63_32:0x%x",hsa_r->dipv6_63_32);
		seq_printf(s,"\tdipv6_95_64:0x%x",hsa_r->dipv6_95_64);
		seq_printf(s,"\tdipv6_127_96:0x%x\n",hsa_r->dipv6_127_96);
*/

		seq_printf(s,"\tsip6:\t%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n"
					,hsa_r->sipv6_127_96>>16,hsa_r->sipv6_127_96&0xFFFF
					,hsa_r->sipv6_95_64>>16,hsa_r->sipv6_95_64&0xFFFF
					,hsa_r->sipv6_63_32>>16,hsa_r->sipv6_63_32&0xFFFF
					,hsa_r->sipv6_31_0>>16,hsa_r->sipv6_31_0&0xFFFF);


		seq_printf(s,"\tdip6:\t%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n"
					,hsa_r->dipv6_127_96>>16,hsa_r->dipv6_127_96&0xFFFF
					,hsa_r->dipv6_95_64>>16,hsa_r->dipv6_95_64&0xFFFF
					,hsa_r->dipv6_63_32>>16,hsa_r->dipv6_63_32&0xFFFF
					,hsa_r->dipv6_31_0>>16,hsa_r->dipv6_31_0&0xFFFF);

		seq_printf(s,"\tip_len:0x%x",hsa_r->ip_len);
		seq_printf(s,"\tipv4_id:0x%x\n",hsa_r->ipv4_id);

	#if	defined(CONFIG_RTL_8197F)
		seq_printf(s,"\thas_ipv4:0x%x",hsa_r->has_ipv4);
		seq_printf(s,"\thas_ipv6:0x%x\n",hsa_r->has_ipv6);
	#endif 
		seq_printf(s,"\ttun_len:0x%x",hsa_r->tun_len);
		seq_printf(s,"\tmltcst_v6:0x%x",hsa_r->mltcst_v6);
		seq_printf(s,"\taddip_pri:0x%x\n",hsa_r->addip_pri);

#endif

#if	defined(CONFIG_RTL_8197F)
		seq_printf(s,"\tl3_cso:0x%x",hsa_r->l3_cso);
		seq_printf(s,"\tl4_cso:0x%x",hsa_r->l4_cso);  
#endif 
		seq_printf(s,")\n");
	}
	return 0;
}
#else
static int32 hs_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int	len;
	hsb_param_t *hsb_r, dummy_hsb_r;
	hsa_param_t *hsa_r, dummy_hsa_r;
	ipaddr_t addr;
	char addr_s[100];

	len = 0;
	hsb_r = &dummy_hsb_r;
	hsa_r = &dummy_hsa_r;
	memset((void*)hsb_r,0,sizeof(hsb_r));
	memset((void*)hsa_r,0,sizeof(hsa_r));

	virtualMacGetHsb( hsb_r );
	{
		len += sprintf(page+len,"HSB(");
		len += sprintf(page+len,"\ttype:%d",hsb_r->type);

		len += sprintf(page+len,"\tspa:%d",hsb_r->spa);
		len += sprintf(page+len,"\tlen:%d",hsb_r->len);
		len += sprintf(page+len,"\tvid :%d\n",hsb_r->vid);
		len += sprintf(page+len,"\tpppoe:%d",hsb_r->pppoeif);

		/* Protocol contents */
		len += sprintf(page+len,"\ttagif:%d\tpppoeId:%d",hsb_r->tagif,hsb_r->pppoeid);
		len += sprintf(page+len,"\tethrtype:0x%04x\n",hsb_r->ethtype);
		len += sprintf(page+len,"\tllc_other:%d\tsnap:%d\n",hsb_r->llcothr,hsb_r->snap);
		len += sprintf(page+len,"\tda:%02x-%02x-%02x-%02x-%02x-%02x",hsb_r->da[0],hsb_r->da[1],hsb_r->da[2],hsb_r->da[3],hsb_r->da[4],hsb_r->da[5]);
		len += sprintf(page+len,"\tsa:%02x-%02x-%02x-%02x-%02x-%02x\n",hsb_r->sa[0],hsb_r->sa[1],hsb_r->sa[2],hsb_r->sa[3],hsb_r->sa[4],hsb_r->sa[5]);

		addr = ntohl( hsb_r->sip);
		inet_ntoa_r(addr, addr_s);
		len += sprintf(page+len,"\tsip:%s(hex:%08x)   ",addr_s,hsb_r->sip);
		len += sprintf(page+len,"\tsprt:%d (hex:%x)\n ",(int)hsb_r->sprt,hsb_r->sprt);
		addr  = ntohl(hsb_r->dip);
		inet_ntoa_r(addr, addr_s);
		len += sprintf(page+len,"\tdip:%s(hex:%08x) ",addr_s,hsb_r->dip);;
		len += sprintf(page+len,"\tdprt:%d(hex:%08x)\n",hsb_r->dprt,hsb_r->dprt);

		len += sprintf(page+len,"\tipptl:%d,",(int)hsb_r->ipptl);
		len += sprintf(page+len,"\tipflg:%d,",hsb_r->ipfg);
		len += sprintf(page+len,"\tiptos:%d,",hsb_r->iptos);
		len += sprintf(page+len,"\ttcpflg:%d\n",hsb_r->tcpfg);

		len += sprintf(page+len,"\tdirtx:%d,",hsb_r->dirtx);
		len += sprintf(page+len,"\tprtnmat:%d",hsb_r->patmatch);

		len += sprintf(page+len,"\tudp_nocs:%d",hsb_r->udpnocs);
		len += sprintf(page+len,"\tttlst:0x%x\n",hsb_r->ttlst);


		len += sprintf(page+len,"\thp:%d",hsb_r->hiprior);
		len += sprintf(page+len,"\tl3csok:%d\tl4csok:%d\tipfragif:%d\n",hsb_r->l3csok,hsb_r->l4csok,hsb_r->ipfo0_n);

	 	len += sprintf(page+len,"\textspa:%d",hsb_r->extspa);
		len += sprintf(page+len,"\turlmch:%d\n",hsb_r->urlmch);

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
   
		len += sprintf(page+len,"\tipv4_opt:%d",hsb_r->ipv4_opt);
		len += sprintf(page+len,"\tcputag_if:%d\n",hsb_r->cputag_if);

		len += sprintf(page+len,"\tipv6_ext:%d",hsb_r->ipv6_ext);
		len += sprintf(page+len,"\tipv6fo:%d\n",hsb_r->ipv6fo);

		len += sprintf(page+len,"\tipv6flag:%d",hsb_r->ipv6flag);
		len += sprintf(page+len,"\tqpri:%d\n",hsb_r->qpri);

		len += sprintf(page+len,"\tptp_pkt:%d",hsb_r->ptp_pkt);
		len += sprintf(page+len,"\tptp_ver:%d\n",hsb_r->ptp_ver);

		len += sprintf(page+len,"\tptp_typ:%d",hsb_r->ptp_typ);
		len += sprintf(page+len,"\tipver_1st:%d\n",hsb_r->ipver_1st);

    	len += sprintf(page+len,"\tsip6:\t%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n"
                    ,hsb_r->sipv6_127_96>>16,hsb_r->sipv6_127_96&0xFFFF
            		,hsb_r->sipv6_95_64>>16,hsb_r->sipv6_95_64&0xFFFF
            		,hsb_r->sipv6_63_32>>16,hsb_r->sipv6_63_32&0xFFFF
            		,hsb_r->sipv6_31_0>>16,hsb_r->sipv6_31_0&0xFFFF);

    	len += sprintf(page+len,"\tdip6:\t%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n"
                    ,hsb_r->dipv6_127_96>>16,hsb_r->dipv6_127_96&0xFFFF
            		,hsb_r->dipv6_95_64>>16,hsb_r->dipv6_95_64&0xFFFF
            		,hsb_r->dipv6_63_32>>16,hsb_r->dipv6_63_32&0xFFFF
            		,hsb_r->dipv6_31_0>>16,hsb_r->dipv6_31_0&0xFFFF);

		len += sprintf(page+len,"\thop_limit:0x%x",hsb_r->hop_limit);
		len += sprintf(page+len,"\ttra_cla:0x%x\n",hsb_r->tra_cla);

		len += sprintf(page+len,"\tflow_lab:0x%x",hsb_r->flow_lab);
		len += sprintf(page+len,"\tnxt_hdr:0x%x\n",hsb_r->nxt_hdr);

		len += sprintf(page+len,"\tipv4:0x%x",hsb_r->ipv4);
		len += sprintf(page+len,"\tipv6:0x%x\n",hsb_r->ipv6);

		len += sprintf(page+len,"\tip_len:0x%x",hsb_r->ip_len);
		len += sprintf(page+len,"\ttun_len:0x%x\n",hsb_r->tun_len);
#endif
		len += sprintf(page+len,")\n");

	}

	virtualMacGetHsa( hsa_r );
	{
		len += sprintf(page+len,("HSA("));
		len += sprintf(page+len,"\tmac:%02x-%02x-%02x-%02x-%02x-%02x\n",hsa_r->nhmac[0],hsa_r->nhmac[1],hsa_r->nhmac[2],hsa_r->nhmac[3],hsa_r->nhmac[4],hsa_r->nhmac[5]);

		addr =ntohl( hsa_r->trip);
		inet_ntoa_r(addr, addr_s);
		len += sprintf(page+len,"\ttrip:%s(hex:%08x)",addr_s,hsa_r->trip);
		len += sprintf(page+len,"\tprt:%d\tipmcast:%d\n",hsa_r->port,hsa_r->ipmcastr);
		len += sprintf(page+len,"\tl3cs:%d",hsa_r->l3csdt);
		len += sprintf(page+len,"\tl4cs:%d",hsa_r->l4csdt);
		len += sprintf(page+len,"\tInternal NETIF:%d",hsa_r->egif);
		len += sprintf(page+len,"\tl2tr:%d,\n ",hsa_r->l2tr);
		len += sprintf(page+len,"\tl34tr:%d",hsa_r->l34tr);
		len += sprintf(page+len,"\tdirtx:%d",hsa_r->dirtxo);
		len += sprintf(page+len,"\ttype:%d",hsa_r->typeo);
		len += sprintf(page+len,"\tsnapo:%d",hsa_r->snapo);
		len += sprintf(page+len,"\twhy2cpu 0x%x (%d)\n",hsa_r->why2cpu,hsa_r->why2cpu);
		len += sprintf(page+len,"\tpppif:%d",hsa_r->pppoeifo);
		len += sprintf(page+len,"\tpppid:%d",hsa_r->pppidx);
		len += sprintf(page+len,"\tttl_1:0x%x",hsa_r->ttl_1if);
		len += sprintf(page+len,"\tdpc:%d,",hsa_r->dpc);
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		len += sprintf(page+len,"\tleno:%d(0x%x) \tmirrort:%d\n",hsa_r->leno,hsa_r->leno,hsa_r->mirrort);
#else   
		len += sprintf(page+len,"\tleno:%d(0x%x)\n",hsa_r->leno,hsa_r->leno);
#endif
		len += sprintf(page+len,"\tl3CrcOk:%d",hsa_r->l3csoko);
		len += sprintf(page+len,"\tl4CrcOk:%d",hsa_r->l4csoko);
		len += sprintf(page+len,"\tfrag:%d",hsa_r->frag);
		len += sprintf(page+len,"\tlastFrag:%d\n",hsa_r->lastfrag);



		len += sprintf(page+len,"\tsvid:0x%x",hsa_r->svid);
		len += sprintf(page+len,"\tdvid:%d(0x%x)",hsa_r->dvid,hsa_r->dvid);
		len += sprintf(page+len,"\tdestination interface :%d\n",hsa_r->difid);
		len += sprintf(page+len,"\trxtag:%d",hsa_r->rxtag);
		len += sprintf(page+len,"\t\tdvtag:0x%x",hsa_r->dvtag);
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		len += sprintf(page+len,"\tspa:%d \tipv4_1st:%d",hsa_r->spao,hsa_r->ipv4_1st);
#else        
		len += sprintf(page+len,"\tspa:%d",hsa_r->spao);
#endif       
		len += sprintf(page+len,"\tdpext:0x%x\thwfwrd:%d\n",hsa_r->dpext,hsa_r->hwfwrd);
		len += sprintf(page+len,"\tspcp:%d",hsa_r->spcp);
		len += sprintf(page+len,"\t\tpriority:%d",hsa_r->priority);

		len += sprintf(page+len,"\tdp:0x%x\n",hsa_r->dp);

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8881A)
		len += sprintf(page+len,"\tcputag:%d",hsa_r->cputag);
		len += sprintf(page+len,"\tptp_pkt:%d",hsa_r->ptp_pkt);
		len += sprintf(page+len,"\tptp_v2:%d",hsa_r->ptp_v2);
		len += sprintf(page+len,"\tptp_type:0x%x\n",hsa_r->ptp_type);
		len += sprintf(page+len,"\trmdp:0x%x",hsa_r->rmdp);
		len += sprintf(page+len,"\tdpri:0x%x\n",hsa_r->dpri);
		len += sprintf(page+len,")\n");
#elif defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
 
		len += sprintf(page+len,"\tcputag:%d",hsa_r->cputag);
		len += sprintf(page+len,"\tptp_pkt:%d",hsa_r->ptp_pkt);
		len += sprintf(page+len,"\tptp_v2:%d",hsa_r->ptp_v2);
		len += sprintf(page+len,"\tptp_type:0x%x\n",hsa_r->ptp_type);

		len += sprintf(page+len,"\trmdp:0x%x",hsa_r->rmdp);
		len += sprintf(page+len,"\tdpri:0x%x",hsa_r->dpri);
		len += sprintf(page+len,"\tmdf:0x%x\n",hsa_r->mdf);
/*
		len += sprintf(page+len,"\tsipv6_31_0:0x%x",hsa_r->sipv6_31_0);
		len += sprintf(page+len,"\tsipv6_63_32:0x%x",hsa_r->sipv6_63_32);
		len += sprintf(page+len,"\tsipv6_95_64:0x%x",hsa_r->sipv6_95_64);
		len += sprintf(page+len,"\tsipv6_127_96:0x%x\n",hsa_r->sipv6_127_96);

		len += sprintf(page+len,"\tdipv6_31_0:0x%x",hsa_r->dipv6_31_0);
		len += sprintf(page+len,"\tdipv6_63_32:0x%x",hsa_r->dipv6_63_32);
		len += sprintf(page+len,"\tdipv6_95_64:0x%x",hsa_r->dipv6_95_64);
		len += sprintf(page+len,"\tdipv6_127_96:0x%x\n",hsa_r->dipv6_127_96);
*/

	    len += sprintf(page+len,"\tsip6:\t%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n"
                    ,hsa_r->sipv6_127_96>>16,hsa_r->sipv6_127_96&0xFFFF
            		,hsa_r->sipv6_95_64>>16,hsa_r->sipv6_95_64&0xFFFF
            		,hsa_r->sipv6_63_32>>16,hsa_r->sipv6_63_32&0xFFFF
            		,hsa_r->sipv6_31_0>>16,hsa_r->sipv6_31_0&0xFFFF);


	    len += sprintf(page+len,"\tdip6:\t%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n"
                    ,hsa_r->dipv6_127_96>>16,hsa_r->dipv6_127_96&0xFFFF
            		,hsa_r->dipv6_95_64>>16,hsa_r->dipv6_95_64&0xFFFF
            		,hsa_r->dipv6_63_32>>16,hsa_r->dipv6_63_32&0xFFFF
            		,hsa_r->dipv6_31_0>>16,hsa_r->dipv6_31_0&0xFFFF);

		len += sprintf(page+len,"\tip_len:0x%x",hsa_r->ip_len);
		len += sprintf(page+len,"\tipv4_id:0x%x\n",hsa_r->ipv4_id);

		len += sprintf(page+len,"\ttun_len:0x%x",hsa_r->tun_len);
		len += sprintf(page+len,"\tmltcst_v6:0x%x",hsa_r->mltcst_v6);
		len += sprintf(page+len,"\taddip_pri:0x%x\n",hsa_r->addip_pri);
		len += sprintf(page+len,")\n");
#else
		len += sprintf(page+len,")\n");
#endif
	}

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}
#endif

static int32 hs_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

#if defined(RTL_DEBUG_NIC_SKB_BUFFER)
int nic_mbuf_show(struct seq_file *s, void *v)
{
	rtl819x_debug_skb_memory();
	return 0;
}

int nic_mbuf_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, nic_mbuf_show, NULL));
}


struct file_operations nic_mbuf_single_seq_file_operations = {
        .open           = nic_mbuf_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

int rxRing_show(struct seq_file *s, void *v)
{
	RTL_dumpRxRing(s);
	return 0;
}

static int32 rxRing_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char		tmpbuf[256];	
	char		*strptr;
	char		*tokptr;

	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		strptr = tmpbuf;

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		if(strncmp(tokptr, "clear",5) == 0)
		{
		#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
			extern int32 swNic_freeRxRing(void);
			swNic_freeRxRing();
		#endif	
		}
		
		return len;
errout:
		rtlglue_printf("wrong format\n");
	}

	return len;
}

int rxRing_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return rxRing_write(file, userbuf,count, off);
}

int rxRing_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rxRing_show, NULL));
}

struct file_operations rxRing_single_seq_file_operations = {
        .open           = rxRing_single_open,
        .write          = rxRing_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

int txRing_show(struct seq_file *s, void *v)
{
	RTL_dumpTxRing(s);
	return 0;
}

int txRing_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, txRing_show, NULL));
}

struct file_operations txRing_single_seq_file_operations = {
        .open           = txRing_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

int mbuf_show(struct seq_file *s, void *v)
{
	RTL_dumpMbufRing(s);
	return 0;
}

int mbuf_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, mbuf_show, NULL));
}

struct file_operations mbuf_single_seq_file_operations = {
        .open           = mbuf_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

#ifdef CONFIG_RTL_PROC_NEW
static int32 pvid_read(struct seq_file *s, void *v)
{
	uint32 vidp[9];
	int32  i;

	for(i=0; i<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; i++)
	{
		if (rtl8651_getAsicPVlanId(i, &vidp[i]) != SUCCESS)
		{
			seq_printf(s,"ASIC PVID get failed.\n");
		}
	}
	seq_printf(s,">> PVID Reg:\n");
	for(i=0; i<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; i++)
		seq_printf(s,"p%d: %d,", i, vidp[i]);
	seq_printf(s,"\n");

	return 0;
}
#else
static int32 pvid_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	uint32 vidp[9];
	int32  i;
	int	len;

	len = 0;
	for(i=0; i<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; i++)
	{
		if (rtl8651_getAsicPVlanId(i, &vidp[i]) != SUCCESS)
		{
			len += sprintf(page+len,"ASIC PVID get failed.\n");
		}
	}
	len += sprintf(page+len,">> PVID Reg:\n");
	for(i=0; i<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; i++)
		len += sprintf(page+len,"p%d: %d,", i, vidp[i]);
	len += sprintf(page+len,"\n");

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}
#endif

static int32 pvid_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char		tmpbuf[256];	
	char		*strptr;
	char		*tokptr;
	uint32		port=0;
	uint32		pvid=0;
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		strptr = tmpbuf;

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		port=simple_strtol(tokptr, NULL, 0);

		if(port>(RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum))
		{
			goto errout;
		}

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		pvid=simple_strtol(tokptr, NULL, 0);

		if(pvid>4096)
		{
			goto errout;
		}

		rtl8651_setAsicPvid( port,	pvid);

		return len;
		errout:
		rtlglue_printf("wrong format\n");

	}

	return len;
}

#ifdef CONFIG_RTL_PROC_NEW
static int32 mirrorPort_read(struct seq_file *s, void *v)
{
	uint32 mirrorRx, mirrorTx, mirrorPort;
	
	#if defined(CONFIG_RTL_8367R_SUPPORT)
	uint32 isolation;
	uint32 tmp_mirrorRx, tmp_mirrorTx, tmp_mirrorPort;
	rtl_mirror_portBased_get(&mirrorPort,&mirrorRx,&mirrorTx);
	rtl8651_getAsicPortMirror(&tmp_mirrorRx, &tmp_mirrorTx, &tmp_mirrorPort);
	if(tmp_mirrorRx&RTL_WANPORT_MASK)
		mirrorRx=tmp_mirrorRx;
	#else
	rtl8651_getAsicPortMirror(&mirrorRx, &mirrorTx, &mirrorPort);
	#endif
	seq_printf(s,">>Mirror Control Register:\n\n");
	seq_printf(s,"  Mirror Rx: 0x%x\n", mirrorRx);
	seq_printf(s,"  Mirror Tx: 0x%x\n", mirrorTx);
	seq_printf(s,"  Mirror Port: 0x%x\n", mirrorPort);
	#if defined(CONFIG_RTL_8367R_SUPPORT)
	rtl_mirror_portIso_get(&isolation);
	seq_printf(s,"  Port isolation: %u\n", isolation);
	#endif
	return 0;
}
#else
static int32 mirrorPort_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	uint32 mirrorRx, mirrorTx, mirrorPort;
	
	int	len;
	len = 0;
#if defined(CONFIG_RTL_8367R_SUPPORT)
	uint32 tmp_mirrorRx, tmp_mirrorTx, tmp_mirrorPort;
	uint32 isolation;
	rtl_mirror_portBased_get(&mirrorPort,&mirrorRx,&mirrorTx);
	rtl8651_getAsicPortMirror(&tmp_mirrorRx, &tmp_mirrorTx, &tmp_mirrorPort);
	if(tmp_mirrorRx&RTL_WANPORT_MASK)
		mirrorRx=tmp_mirrorRx;
#else
	rtl8651_getAsicPortMirror(&mirrorRx, &mirrorTx, &mirrorPort);
#endif
	len += sprintf(page+len,">>Mirror Control Register:\n\n");
	len += sprintf(page+len,"  Mirror Rx: 0x%x\n", mirrorRx);
	len += sprintf(page+len,"  Mirror Tx: 0x%x\n", mirrorTx);
	len += sprintf(page+len,"  Mirror Port: 0x%x\n", mirrorPort);
	#if defined(CONFIG_RTL_8367R_SUPPORT)
	rtl_mirror_portIso_get(&isolation);
	len += sprintf(page+len,"  Port isolation: %u\n", isolation);
	#endif
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}
#endif

static int32 mirrorPort_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char		tmpbuf[64];
	//uint32	*mem_addr, mem_data, mem_len;
	uint32	rx_mask,tx_mask,port_mask;
	char		*strptr, *cmd_addr;
	char		*tokptr;
	#if defined(CONFIG_RTL_8367R_SUPPORT)
	uint32 iso_port_enable;
	#endif
	if(len>64)
	{
		goto errout;
	}

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
		rtlglue_printf("cmd %s\n", cmd_addr);
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "mirror", 6))
		{
		#if defined(CONFIG_RTL_8367R_SUPPORT)
			rx_mask=simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			tx_mask = simple_strtol(tokptr,NULL,0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			port_mask = simple_strtol(tokptr, NULL, 0);
			if(rx_mask&RTL_WANPORT_MASK){
				rtl8651_setAsicPortMirror(rx_mask,0,port_mask);
				if(rtl_mirror_portBased_set(port_mask,0,tx_mask) == 0)
					rtlglue_printf("mirror rx port mask(0x%x) tx port mask(0x%x), mirror port mask(0x%x)\n",rx_mask,tx_mask,port_mask);
				else
					rtlglue_printf("set failed\n");
			}
			else{
				rtl8651_setAsicPortMirror(0,0,port_mask);
				if(rtl_mirror_portBased_set(port_mask,rx_mask,tx_mask) == 0)
					rtlglue_printf("mirror rx port mask(0x%x) tx port mask(0x%x), mirror port mask(0x%x)\n",rx_mask,tx_mask,port_mask);
				else
					rtlglue_printf("set failed\n");
			}
		#else
			rx_mask=simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			tx_mask = simple_strtol(tokptr,NULL,0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			port_mask = simple_strtol(tokptr, NULL, 0);
			rtlglue_printf("mirror rx port mask(0x%x) tx port mask(0x%x), mirror port mask(0x%x)\n",rx_mask,tx_mask,port_mask);
			rtl8651_setAsicPortMirror(rx_mask,tx_mask,port_mask);
		#endif
		}
		#if defined(CONFIG_RTL_8367R_SUPPORT)
		else if(!memcmp(cmd_addr, "port_isolation", 14)){
			iso_port_enable = simple_strtol(tokptr, NULL, 0);
			rtl_mirror_portIso_set(iso_port_enable);
		}
		#endif
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		rtlglue_printf("Mirror port configuration only support \"mirror\"as the first parameter\n");
		rtlglue_printf("mirror: \"mirror rx_portmask tx_portmask mirror_portmask\"\n");
	}

	return len;
}


#ifdef CONFIG_RTL_PROC_NEW
static int32 l2_read(struct seq_file *s, void *v)
{
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
 	uint32 row, col, port, m=0;

	seq_printf(s, "%s\n", "ASIC L2 Table:");
	for(row=0x0; row<RTL8651_L2TBL_ROW; row++)
	{
		for(col=0; col<RTL8651_L2TBL_COLUMN; col++)
		{
			memset((void*)&asic_l2, 0, sizeof(asic_l2));
			if (rtl8651_getAsicL2Table(row, col, &asic_l2) == FAILED)
			{
				continue;
			}

			if (asic_l2.isStatic && asic_l2.ageSec==0 && asic_l2.cpu && asic_l2.memberPortMask == 0 &&asic_l2.auth==0)
			{
				continue;
			}

			seq_printf(s, "%4d.[%3d,%d] %02x:%02x:%02x:%02x:%02x:%02x FID:%x mbr(",m, row, col,
					asic_l2.macAddr.octet[0], asic_l2.macAddr.octet[1], asic_l2.macAddr.octet[2],
					asic_l2.macAddr.octet[3], asic_l2.macAddr.octet[4], asic_l2.macAddr.octet[5],asic_l2.fid
			);

			m++;

			for (port = 0 ; port < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum ; port ++)
			{
				if (asic_l2.memberPortMask & (1<<port))
				{
					seq_printf(s,"%d ", port);
				}
			}

			seq_printf(s,")");
			seq_printf(s,"%s %s %s %s age:%d ",asic_l2.cpu?"CPU":"FWD", asic_l2.isStatic?"STA":"DYN",  asic_l2.srcBlk?"BLK":"", asic_l2.nhFlag?"NH":"", asic_l2.ageSec);

			if (asic_l2.auth)
			{
				seq_printf(s,"AUTH:%d",asic_l2.auth);
			}
			else
			{
				seq_printf(s,"AUTH:0");
			}

			seq_printf(s,"\n");
		}
	}

	return 0;

}
#else
static int32 l2_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
 	uint32 row, col, port, m=0;

	len = sprintf(page, "%s\n", "ASIC L2 Table:");
	for(row=0x0; row<RTL8651_L2TBL_ROW; row++)
	{
		for(col=0; col<RTL8651_L2TBL_COLUMN; col++)
		{
			memset((void*)&asic_l2, 0, sizeof(asic_l2));
			if (rtl8651_getAsicL2Table(row, col, &asic_l2) == FAILED)
			{
				continue;
			}

			if (asic_l2.isStatic && asic_l2.ageSec==0 && asic_l2.cpu && asic_l2.memberPortMask == 0 &&asic_l2.auth==0)
			{
				continue;
			}

			len += sprintf(page + len, "%4d.[%3d,%d] %02x:%02x:%02x:%02x:%02x:%02x FID:%x mbr(",m, row, col,
					asic_l2.macAddr.octet[0], asic_l2.macAddr.octet[1], asic_l2.macAddr.octet[2],
					asic_l2.macAddr.octet[3], asic_l2.macAddr.octet[4], asic_l2.macAddr.octet[5],asic_l2.fid
			);

			m++;

			for (port = 0 ; port < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum ; port ++)
			{
				if (asic_l2.memberPortMask & (1<<port))
				{
					len += sprintf(page + len,"%d ", port);
				}
			}

			len += sprintf(page + len,")");
			len += sprintf(page + len,"%s %s %s %s age:%d ",asic_l2.cpu?"CPU":"FWD", asic_l2.isStatic?"STA":"DYN",  asic_l2.srcBlk?"BLK":"", asic_l2.nhFlag?"NH":"", asic_l2.ageSec);

			if (asic_l2.auth)
			{
				len += sprintf(page + len,"AUTH:%d",asic_l2.auth);
			}
			else
			{
				len += sprintf(page + len,"AUTH:0");
			}

			len += sprintf(page + len,"\n");
		}
	}

	return len;

}
#endif
static int32 l2_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

extern uint32 _rtl865x_getAddMcastOpCnt(void);
extern uint32 _rtl865x_getDelMcastOpCnt(void);
extern uint32 _rtl865x_getForceAddMcastOpCnt(void);
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#ifdef CONFIG_RTL_PROC_NEW
static int32 rtl865x_proc_hw_mcast_read(struct seq_file *s, void *v)
{
	rtl865x_tblAsicDrv_multiCastParam_t asic;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	rtl8198C_tblAsicDrv_multiCastv6Param_t asic6;
#endif
	uint32 entry;
#if 0
	uint8 *ucsip, *ucdip;
#endif

	#if 0
	seq_printf(s, "%s\n", "ASIC Multicast Table:");
	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++)
	{
			if (rtl8651_getAsicIpMulticastTable(entry, &asic) != SUCCESS) {
				#if 0 
				seq_printf(s,"\t[%d]  (INVALID)dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(0x%x)\n", entry,
					asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff),
					asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
					asic.mbr);
				seq_printf(s,"\t       svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
					asic.age, asic.cpu);
				#endif
				continue;
			}
			else
			{
				ucsip = (unsigned char *)&asic.sip;
				ucdip = (unsigned char *)&asic.dip;
				seq_printf(s,"\t[%d]  (OK)dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(0x%x)\n", entry,
				ucdip[0], ucdip[1], ucdip[2], ucdip[3],
				ucsip[0], ucsip[1], ucsip[2], ucsip[3],
				asic.mbr);
				seq_printf(s,"\t       svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
				asic.age, asic.cpu);
			}
	}
	seq_printf(s,"\n\t TotalOpCnt:AddMcastOpCnt:%d\tDelMcastOpCnt:%d\tForceAddMcastOpCnt:%d\t \n", _rtl865x_getAddMcastOpCnt(),_rtl865x_getDelMcastOpCnt(),_rtl865x_getForceAddMcastOpCnt());
	#else
	seq_printf(s, "%s\n", "ASIC Multicast Table:");

	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++)
	{			
			#if defined(CONFIG_RTL_WTDOG)
			rtl_periodic_watchdog_kick(entry, WATCHDOG_NUM_OF_TIMES);
			#endif
			if (rtl8651_getAsicIpMulticastTable(entry, &asic) != SUCCESS) {
				continue;
			}
			seq_printf(s, "\t[%d]  (OK)dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(%x)\n", entry,
				asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff),
				asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
				asic.mbr);
			seq_printf(s,"\t       svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
				asic.age, asic.cpu);
	}	
	seq_printf(s,"\n\t TotalOpCnt:AddMcastOpCnt:%d\tDelMcastOpCnt:%d\tForceAddMcastOpCnt:%d\t \n", _rtl865x_getAddMcastOpCnt(),_rtl865x_getDelMcastOpCnt(),_rtl865x_getForceAddMcastOpCnt());
	#endif
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	seq_printf(s,"----------------------------------------------------\n");
	seq_printf(s,"%s\n", "ASIC IPV6 Multicast Table:");
	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++)
	{
			#if defined(CONFIG_RTL_WTDOG)
			rtl_periodic_watchdog_kick(entry, WATCHDOG_NUM_OF_TIMES);
			#endif
			if(rtl8198C_getAsicIpMulticastv6Table(entry,&asic6) != SUCCESS){
			#if 0 
				seq_printf(s,"\t[%d]  (INVALID)dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(0x%x)\n", entry,
					asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff),
					asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
					asic.mbr);
				seq_printf(s,"\t       svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
					asic.age, asic.cpu);
			#endif
				continue;
			}
			else
			{	
				seq_printf(s,"\t[%d]  (OK)dip:%08x:%08x:%08x:%08x\n",
					entry,
					asic6.dip.v6_addr32[0],asic6.dip.v6_addr32[1],
					asic6.dip.v6_addr32[2],asic6.dip.v6_addr32[3]);
				seq_printf(s,"\t          sip:%08x:%08x:%08x:%08x\n",
					asic6.sip.v6_addr32[0],asic6.sip.v6_addr32[1],
					asic6.sip.v6_addr32[2],asic6.sip.v6_addr32[3]);
				
				seq_printf(s,"\t          spa:%d, age:%d, cpu:%d, mbr(0x%x)\n", asic6.port, asic6.age, asic6.cpu, asic6.mbr);
			}
	}
	seq_printf(s,"\n\t TotalOpCntv6:AddMcastOpCntv6:%d\tDelMcastOpCntv6:%d\tForceAddMcastOpCntv6:%d\t \n", _rtl8198C_getAddMcastv6OpCnt(),_rtl8198C_getDelMcastv6OpCnt(),_rtl8198C_getForceAddMcastv6OpCnt());
#endif
	return 0;
}
#else
static int32 rtl865x_proc_hw_mcast_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len=0;
	rtl865x_tblAsicDrv_multiCastParam_t asic;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	rtl8198C_tblAsicDrv_multiCastv6Param_t asic6;
#endif
	uint32 entry;

	#if 1
	len = sprintf(page, "%s\n", "ASIC Multicast Table:");
	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++)
	{
			#if defined(CONFIG_RTL_WTDOG)
			rtl_periodic_watchdog_kick(entry, WATCHDOG_NUM_OF_TIMES);
			#endif
			if (rtl8651_getAsicIpMulticastTable(entry, &asic) != SUCCESS) {
				#if 0 
				len += sprintf(page+len,"\t[%d]  (INVALID)dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(0x%x)\n", entry,
					asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff),
					asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
					asic.mbr);
				len += sprintf(page+len,"\t       svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
					asic.age, asic.cpu);
				#endif
				continue;
			}
			else
			{
				len += sprintf(page+len,"\t[%d]  (OK)dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(0x%x)\n", entry,
				asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff),
				asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
				asic.mbr);
				len += sprintf(page+len,"\t       svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
				asic.age, asic.cpu);
			}
	}
	len += sprintf(page+len,"\n\t TotalOpCnt:AddMcastOpCnt:%d\tDelMcastOpCnt:%d\tForceAddMcastOpCnt:%d\t \n", _rtl865x_getAddMcastOpCnt(),_rtl865x_getDelMcastOpCnt(),_rtl865x_getForceAddMcastOpCnt());
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		len += sprintf(page+len,"----------------------------------------------------\n");
		len += sprintf(page+len,"%s\n", "ASIC IPV6 Multicast Table:");
		for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++)
		{
				#if defined(CONFIG_RTL_WTDOG)
				rtl_periodic_watchdog_kick(entry, WATCHDOG_NUM_OF_TIMES);
				#endif
				if(rtl8198C_getAsicIpMulticastv6Table(entry,&asic6) != SUCCESS){
			#if 0 
					len += sprintf(page+len,"\t[%d]	(INVALID)dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(0x%x)\n", entry,
						asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff),
						asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
						asic.mbr);
					len += sprintf(page+len,"\t		 svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
						asic.age, asic.cpu);
			#endif
					continue;
				}
				else
				{	
					len += sprintf(page+len,"\t[%d]	(OK)dip:%08x-%08x-%08x-%08x\n",
						entry,
						asic6.dip.v6_addr32[0],asic6.dip.v6_addr32[1],
						asic6.dip.v6_addr32[2],asic6.dip.v6_addr32[3]);
					len += sprintf(page+len,"\t			sip:%08x-%08x-%08x-%08x\n",
						asic6.sip.v6_addr32[0],asic6.sip.v6_addr32[1],
						asic6.sip.v6_addr32[2],asic6.sip.v6_addr32[3]);
					
					len += sprintf(page+len,"\t			spa:%d, age:%d, cpu:%d, mbr:0x%x\n", asic6.port, asic6.age, asic6.cpu, asic6.mbr);
				}
		}
		len += sprintf(page+len,"\n\t TotalOpCntv6:AddMcastOpCntv6:%d\tDelMcastOpCntv6:%d\tForceAddMcastOpCntv6:%d\t \n", _rtl8198C_getAddMcastv6OpCnt(),_rtl8198C_getDelMcastv6OpCnt(),_rtl8198C_getForceAddMcastv6OpCnt());
#endif

	#else
	len = sprintf(page, "%s\n", "ASIC Multicast Table:");

	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++)
	{
			if (rtl8651_getAsicIpMulticastTable(entry, &asic) != SUCCESS) {
				len +=sprintf(page+len,"\t[%d]  (Invalid Entry)\n", entry);
				continue;
			}
			len += sprintf(page+len, "\t[%d]  dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(%x)\n", entry,
				asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff),
				asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
				asic.mbr);
			len +=sprintf(page+len,"\t       svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
				asic.age, asic.cpu);
	}
	#endif

	if (len <= off+count)
	{
		*eof = 1;
	}

	*start = page + off;
	len -= off;

	if (len>count)
	{
		len = count;
	}

	if (len<0)
	{
	  	len = 0;
	}

	return len;
}
#endif

static int32 rtl865x_proc_hw_mcast_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[512];
	char		*strptr;
	char		*tokptr;
	char		*dotPtr;
	rtl865xc_tblAsic_ipMulticastTable_t entry;
	int16 age;
	uint32 idx;
	uint32 sip,dip;
	uint32 srcPort,svid,mbr;
	int32	i;

	if(len>512)
	{
		goto errout;
	}

	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		bzero(&entry, sizeof(entry));
		tmpbuf[len] = '\0';

		strptr=tmpbuf;

		/*valid*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		entry.valid = simple_strtol(tokptr, NULL, 0);

		/*destination ip*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		dip=0;
		for(i=0;i<4;i++)
		{
			dotPtr=strsep(&tokptr,".");
			if(dotPtr==NULL)
			{
				break;
			}
			dip=(dip<<8)|simple_strtol(dotPtr, NULL, 0);
		}

		entry.destIPAddrLsbs= dip & 0xfffffff;


		/*source ip*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		sip=0;
		for(i=0;i<4;i++)
		{
			dotPtr=strsep(&tokptr,".");
			if(dotPtr==NULL)
			{
				break;
			}
			sip=(sip<<8)|simple_strtol(dotPtr, NULL, 0);
		}

		entry.srcIPAddr=sip;


		/*mbr*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		mbr= simple_strtol(tokptr, NULL, 0);
		entry.portList 			= mbr & (RTL8651_PHYSICALPORTMASK);

		/*svid*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		svid= simple_strtol(tokptr, NULL, 0);

		/*spa*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		srcPort= simple_strtol(tokptr, NULL, 0);
		entry.srcPort 			= srcPort;

		/*extIP*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		entry.extIPIndex = simple_strtol(tokptr, NULL, 0);

		/*age*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		age=simple_strtol(tokptr, NULL, 0);


		entry.ageTime			= 0;
		while ( age > 0 )
		{
			if ( (++entry.ageTime) == 7)
				break;
			age -= 5;
		}

		/*to cpu*/
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		entry.toCPU = simple_strtol(tokptr, NULL, 0);

		idx = rtl8651_ipMulticastTableIndex(sip,dip);
		_rtl8651_forceAddAsicEntry(TYPE_MULTICAST_TABLE, idx, &entry);


	}
	else
	{
errout:
		rtlglue_printf("error input\n");
	}

	return len;
}

#ifdef CONFIG_RTL_PROC_NEW
static int32 rtl865x_proc_sw_mcast_read(struct seq_file *s, void *v)
{
	rtl_dumpSwMulticastInfo(s);
	return 0;
}
#else
static int32 rtl865x_proc_sw_mcast_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len = 0;

	rtl_dumpSwMulticastInfo(page, &len);

    if (len <= off+count) *eof = 1;
    *start = page + off;
    len -= off;
    if (len>count)
        len = count;
    if (len<0)
        len = 0;
    
    return len;

}
#endif

static int32 rtl865x_proc_sw_mcast_write( struct file *filp, const char *buff,unsigned long len, void *data )
{

	return len;
}
#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER_L3

#ifdef CONFIG_RTL_PROC_NEW
static int32 arp_read(struct seq_file *s, void *v)
{
	rtl865x_tblAsicDrv_routingParam_t asic_l3;
	rtl865x_tblAsicDrv_arpParam_t asic_arp;
	rtl865x_tblAsicDrv_l2Param_t asic_l2;

	uint32	i, j, port;
	ipaddr_t ipAddr;
	int8 ipBuf[sizeof"255.255.255.255"];


	seq_printf(s, "%s\n", "ASIC Arp Table:\n");
	for(i=0; i<RTL8651_ARPTBL_SIZE; i++)
	{
		if (rtl8651_getAsicArp(i,  &asic_arp) == FAILED)
			continue;
		for(j=0; j<RTL8651_ROUTINGTBL_SIZE; j++)
		{
			if (rtl8651_getAsicRouting(j, &asic_l3) == FAILED || asic_l3.process!= 0x02 /*RT_ARP*/)
				continue;
			if(asic_l3.arpStart <= (i>>3) &&  (i>>3) <= asic_l3.arpEnd)
			{
				// 8197F: change to big endian format for below operation
				asic_l3.ipAddr = htonl(asic_l3.ipAddr);
				asic_l3.ipMask = htonl(asic_l3.ipMask);
				
				ipAddr = (asic_l3.ipAddr & asic_l3.ipMask) + (i - (asic_l3.arpStart<<3));

				// 8197F: change to little endian format for display (inet_ntoa_r)
				ipAddr = ntohl(ipAddr);

				if(rtl8651_getAsicL2Table_Patch(asic_arp.nextHopRow, asic_arp.nextHopColumn, &asic_l2) == FAILED)
				{
					inet_ntoa_r(ipAddr, ipBuf);
					seq_printf(s,"%-16s [%3d,%d] ", ipBuf, asic_arp.nextHopRow, asic_arp.nextHopColumn);
				}
				else
				{
					inet_ntoa_r(ipAddr, ipBuf);
					seq_printf(s,"%-16s %02x-%02x-%02x-%02x-%02x-%02x (", ipBuf, asic_l2.macAddr.octet[0], asic_l2.macAddr.octet[1], asic_l2.macAddr.octet[2], asic_l2.macAddr.octet[3], asic_l2.macAddr.octet[4], asic_l2.macAddr.octet[5]);
					for(port=0; port< RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; port++){
						if(asic_l2.memberPortMask& (1<<port))
							seq_printf(s,"%d ", port);
						else
							seq_printf(s,"  ");
					}
					seq_printf(s,") %us", asic_l2.ageSec);
				}
				continue;
			}
		}

		seq_printf(s," ARP:%3d  L2:%3d,%d,aging:%d\n", i, asic_arp.nextHopRow, asic_arp.nextHopColumn,asic_arp.aging);

	}

	return 0;
}
#else
static int32 arp_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	rtl865x_tblAsicDrv_routingParam_t asic_l3;
	rtl865x_tblAsicDrv_arpParam_t asic_arp;
	rtl865x_tblAsicDrv_l2Param_t asic_l2;

	uint32	i, j, port;
	ipaddr_t ipAddr;
	int8 ipBuf[sizeof"255.255.255.255"];


	len = sprintf(page, "%s\n", "ASIC Arp Table:\n");
	for(i=0; i<RTL8651_ARPTBL_SIZE; i++)
	{
		if (rtl8651_getAsicArp(i,  &asic_arp) == FAILED)
			continue;
		for(j=0; j<RTL8651_ROUTINGTBL_SIZE; j++)
		{
			if (rtl8651_getAsicRouting(j, &asic_l3) == FAILED || asic_l3.process!= 0x02 /*RT_ARP*/)
				continue;
			if(asic_l3.arpStart <= (i>>3) &&  (i>>3) <= asic_l3.arpEnd)
			{
				ipAddr = (asic_l3.ipAddr & asic_l3.ipMask) + (i - (asic_l3.arpStart<<3));
				if(rtl8651_getAsicL2Table_Patch(asic_arp.nextHopRow, asic_arp.nextHopColumn, &asic_l2) == FAILED)
				{
					inet_ntoa_r(ipAddr, ipBuf);
					len += sprintf(page + len,"%-16s [%3d,%d] ", ipBuf, asic_arp.nextHopRow, asic_arp.nextHopColumn);
				}
				else
				{
					inet_ntoa_r(ipAddr, ipBuf);
					len += sprintf(page + len,"%-16s %02x-%02x-%02x-%02x-%02x-%02x (", ipBuf, asic_l2.macAddr.octet[0], asic_l2.macAddr.octet[1], asic_l2.macAddr.octet[2], asic_l2.macAddr.octet[3], asic_l2.macAddr.octet[4], asic_l2.macAddr.octet[5]);
					for(port=0; port< RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; port++){
						if(asic_l2.memberPortMask& (1<<port))
							len += sprintf(page + len,"%d ", port);
						else
							len += sprintf(page + len,"  ");
					}
					len += sprintf(page + len,") %us", asic_l2.ageSec);
				}
				continue;
			}
		}

		len += sprintf(page + len," ARP:%3d  L2:%3d,%d,aging:%d\n", i, asic_arp.nextHopRow, asic_arp.nextHopColumn,asic_arp.aging);

	}

	return len;
}
#endif

static int32 arp_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}


#ifdef CONFIG_RTL_PROC_NEW
static int32 nexthop_read(struct seq_file *s, void *v)
{
	rtl865x_tblAsicDrv_nextHopParam_t asic_nxthop;

	uint32 idx, refcnt, rt_flag;

	seq_printf(s, "%s\n", "ASIC Next Hop Table:\n");
	for(idx=0; idx<RTL8651_NEXTHOPTBL_SIZE; idx++) {
		refcnt = rt_flag = 0;
		if (rtl8651_getAsicNextHopTable(idx, &asic_nxthop) == FAILED)
			continue;
		seq_printf(s,"  [%d]  type(%s) IPIdx(%d) dstVid(%d) pppoeIdx(%d) nextHop([%d:%d]%d) rf(%d) rt(%d)\n", idx,
			(asic_nxthop.isPppoe==TRUE? "pppoe": "ethernet"), asic_nxthop.extIntIpIdx,
			asic_nxthop.dvid, asic_nxthop.pppoeIdx,asic_nxthop.nextHopRow ,asic_nxthop.nextHopColumn,(asic_nxthop.nextHopRow<<2)+asic_nxthop.nextHopColumn, refcnt, rt_flag);
	}

	return 0;
}
#else
static int32 nexthop_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	rtl865x_tblAsicDrv_nextHopParam_t asic_nxthop;
	uint32 idx, refcnt, rt_flag;
    rtlglue_printf("[%s:%d]\n", __FUNCTION__, __LINE__);
	len = sprintf(page, "%s\n", "ASIC Next Hop Table:\n");
	for(idx=0; idx<RTL8651_NEXTHOPTBL_SIZE; idx++) {
		refcnt = rt_flag = 0;
		if (rtl8651_getAsicNextHopTable(idx, &asic_nxthop) == FAILED)
			continue;
		len += sprintf(page + len,"  [%d]  type(%s) IPIdx(%d) dstVid(%d) pppoeIdx(%d) nextHop(%d) rf(%d) rt(%d)\n", idx,
			(asic_nxthop.isPppoe==TRUE? "pppoe": "ethernet"), asic_nxthop.extIntIpIdx,
			asic_nxthop.dvid, asic_nxthop.pppoeIdx, (asic_nxthop.nextHopRow<<2)+asic_nxthop.nextHopColumn, refcnt, rt_flag);
	}

	return len;
}
#endif

static int32 nexthop_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}


#ifdef CONFIG_RTL_PROC_NEW
static int32 l3_read(struct seq_file *s, void *v)
{
#if 0
uint32			DSLEG;
uint32			DSL_IDX;

#endif
	rtl865x_tblAsicDrv_routingParam_t asic_l3;
	int8 *str[] = { "PPPoE", "L2", "ARP", " ", "CPU", "NxtHop", "DROP", " " };
	int8 *strNetType[] = { "WAN", "DMZ", "LAN",  "RLAN"};
	uint32 idx, mask;
	int netIdx;
	uint8 *ucp;

	seq_printf(s, "%s\n", "ASIC L3 Routing Table:\n");
	for(idx=0; idx<RTL8651_ROUTINGTBL_SIZE; idx++)
	{
		if (rtl8651_getAsicRouting(idx, &asic_l3) == FAILED)
		{
			seq_printf(s,"\t[%d]  (Invalid)\n", idx);
			continue;
		}

		asic_l3.ipMask = htonl(asic_l3.ipMask);
		
		if (idx == RTL8651_ROUTINGTBL_SIZE-1)
			mask = 0;
		else for(mask=32; !(asic_l3.ipMask&0x01); asic_l3.ipMask=asic_l3.ipMask>>1)
				mask--;
		netIdx = asic_l3.internal<<1|asic_l3.DMZFlag;
//		seq_printf(s,"\t[%d]  %d.%d.%d.%d/%d process(%s) %s DSLEG(%d),DSL_IDX(%d)\n", idx, (asic_l3.ipAddr>>24),
//			((asic_l3.ipAddr&0x00ff0000)>>16), ((asic_l3.ipAddr&0x0000ff00)>>8), (asic_l3.ipAddr&0xff),
//			mask, str[asic_l3.process],strNetType[netIdx],asic_l3.DSLEG,asic_l3.DSL_IDX);
		ucp = (unsigned char *)&(asic_l3.ipAddr);
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		seq_printf(s,"\t[%d]  %d.%d.%d.%d/%d process(%s) %s DSLEG(%d),DSL_IDX(%d)\n", idx, ucp[0],
			ucp[1], ucp[2], ucp[3],
			mask, str[asic_l3.process],strNetType[netIdx],asic_l3.DSLEG,asic_l3.DSL_IDX);
#else
		seq_printf(s,"\t[%d]  %d.%d.%d.%d/%d process(%s) %s \n", idx, ucp[0],
			ucp[1], ucp[2], ucp[3],
			mask, str[asic_l3.process],strNetType[netIdx]);
#endif
		switch(asic_l3.process)
		{
		case 0x00:	/* PPPoE */
			seq_printf(s,"\t           dvidx(%d)  pppidx(%d) nxthop(%d)\n", asic_l3.vidx, asic_l3.pppoeIdx, (asic_l3.nextHopRow<<2)+asic_l3.nextHopColumn);
			break;

		case 0x01:	/* L2 */
			seq_printf(s,"              dvidx(%d) nexthop(%d)\n", asic_l3.vidx, (asic_l3.nextHopRow<<2)+asic_l3.nextHopColumn);
			break;

		case 0x02:	/* ARP */
			seq_printf(s,"             dvidx(%d) ARPSTA(%d) ARPEND(%d) IPIDX(%d)\n", asic_l3.vidx, asic_l3.arpStart<<3, asic_l3.arpEnd<<3, asic_l3.arpIpIdx);
			break;

		case 0x03:	/* Reserved */
			;

		case 0x04:	/* CPU */
			seq_printf(s,"             dvidx(%d)\n", asic_l3.vidx);
			break;

		case 0x05:	/* NAPT Next Hop */
			seq_printf(s,"              NHSTA(%d) NHNUM(%d) NHNXT(%d) NHALGO(%d) IPDOMAIN(%d)\n", asic_l3.nhStart,
				asic_l3.nhNum, asic_l3.nhNxt, asic_l3.nhAlgo, asic_l3.ipDomain);
			break;

		case 0x06:	/* DROP */
			seq_printf(s,"             dvidx(%d)\n", asic_l3.vidx);
			break;

		case 0x07:	/* Reserved */
			/* pass through */
		default:
		;
		}
	}

	return 0;
}
#else
static int32 l3_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	rtl865x_tblAsicDrv_routingParam_t asic_l3;
	int8 *str[] = { "PPPoE", "L2", "ARP", " ", "CPU", "NxtHop", "DROP", " " };
	int8 *strNetType[] = { "WAN", "DMZ", "LAN",  "RLAN"};
	uint32 idx, mask;
	int netIdx;

	len = sprintf(page, "%s\n", "ASIC L3 Routing Table:\n");
	for(idx=0; idx<RTL8651_ROUTINGTBL_SIZE; idx++)
	{
		if (rtl8651_getAsicRouting(idx, &asic_l3) == FAILED)
		{
			len += sprintf(page + len,"\t[%d]  (Invalid)\n", idx);
			continue;
		}
		if (idx == RTL8651_ROUTINGTBL_SIZE-1)
			mask = 0;
		else for(mask=32; !(asic_l3.ipMask&0x01); asic_l3.ipMask=asic_l3.ipMask>>1)
				mask--;
		netIdx = asic_l3.internal<<1|asic_l3.DMZFlag;
		len += sprintf(page + len,"\t[%d]  %d.%d.%d.%d/%d process(%s) %s \n", idx, (asic_l3.ipAddr>>24),
			((asic_l3.ipAddr&0x00ff0000)>>16), ((asic_l3.ipAddr&0x0000ff00)>>8), (asic_l3.ipAddr&0xff),
			mask, str[asic_l3.process],strNetType[netIdx]);

		switch(asic_l3.process)
		{
		case 0x00:	/* PPPoE */
			len += sprintf(page + len,"\t           dvidx(%d)  pppidx(%d) nxthop(%d)\n", asic_l3.vidx, asic_l3.pppoeIdx, (asic_l3.nextHopRow<<2)+asic_l3.nextHopColumn);
			break;

		case 0x01:	/* L2 */
			len += sprintf(page + len,"              dvidx(%d) nexthop(%d)\n", asic_l3.vidx, (asic_l3.nextHopRow<<2)+asic_l3.nextHopColumn);
			break;

		case 0x02:	/* ARP */
			len += sprintf(page + len,"             dvidx(%d) ARPSTA(%d) ARPEND(%d) IPIDX(%d)\n", asic_l3.vidx, asic_l3.arpStart<<3, asic_l3.arpEnd<<3, asic_l3.arpIpIdx);
			break;

		case 0x03:	/* Reserved */
			;

		case 0x04:	/* CPU */
			len += sprintf(page + len,"             dvidx(%d)\n", asic_l3.vidx);
			break;

		case 0x05:	/* NAPT Next Hop */
			len += sprintf(page + len,"              NHSTA(%d) NHNUM(%d) NHNXT(%d) NHALGO(%d) IPDOMAIN(%d)\n", asic_l3.nhStart,
				asic_l3.nhNum, asic_l3.nhNxt, asic_l3.nhAlgo, asic_l3.ipDomain);
			break;

		case 0x06:	/* DROP */
			len += sprintf(page + len,"             dvidx(%d)\n", asic_l3.vidx);
			break;

		case 0x07:	/* Reserved */
			/* pass through */
		default:
		;
		}
	}

	return len;
}
#endif

static int32 l3_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}


#ifdef CONFIG_RTL_PROC_NEW
static int32 ip_read(struct seq_file *s, void *v)
{
	rtl865x_tblAsicDrv_extIntIpParam_t asic_ip;
	int32	i;
	int8 intIpBuf[sizeof"255.255.255.255"];
	int8 extIpBuf[sizeof"255.255.255.255"];

	seq_printf(s, "%s\n", "ASIC IP Table:\n");

	for(i=0; i<RTL8651_IPTABLE_SIZE; i++)
	{
		if (rtl8651_getAsicExtIntIpTable(i,  &asic_ip) == FAILED)
		{
			seq_printf(s,"  [%d] (Invalid)\n", i);
			continue;
		}
		inet_ntoa_r(asic_ip.intIpAddr, intIpBuf);
		inet_ntoa_r(asic_ip.extIpAddr,extIpBuf);
		seq_printf(s,"  [%d] intip(%-14s) extip(%-14s) type(%s) nhIdx(%d)\n",
					i, intIpBuf,extIpBuf,
					(asic_ip.localPublic==TRUE? "LP" : (asic_ip.nat==TRUE ? "NAT" : "NAPT")), asic_ip.nhIndex);
	}

	return 0;
}
#else
static int32 ip_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;

	rtl865x_tblAsicDrv_extIntIpParam_t asic_ip;
	int32	i;
	int8 intIpBuf[sizeof"255.255.255.255"];
	int8 extIpBuf[sizeof"255.255.255.255"];

	len = sprintf(page, "%s\n", "ASIC IP Table:\n");

	for(i=0; i<RTL8651_IPTABLE_SIZE; i++)
	{
		if (rtl8651_getAsicExtIntIpTable(i,  &asic_ip) == FAILED)
		{
			len += sprintf(page + len,"  [%d] (Invalid)\n", i);
			continue;
		}
		inet_ntoa_r(asic_ip.intIpAddr, intIpBuf);
		inet_ntoa_r(asic_ip.extIpAddr,extIpBuf);
		len += sprintf(page + len,"  [%d] intip(%-14s) extip(%-14s) type(%s) nhIdx(%d)\n",
					i, intIpBuf,extIpBuf,
					(asic_ip.localPublic==TRUE? "LP" : (asic_ip.nat==TRUE ? "NAT" : "NAPT")), asic_ip.nhIndex);
	}

	return len;
}
#endif

static int32 ip_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}


#ifdef CONFIG_RTL_PROC_NEW
static int32 pppoe_read(struct seq_file *s, void *v)
{
	rtl865x_tblAsicDrv_pppoeParam_t asic_pppoe;
	int32	i;

	seq_printf(s, "%s\n", "ASIC PPPOE Table:\n");
	for(i=0; i<RTL8651_PPPOETBL_SIZE; i++)
	{
		if (rtl8651_getAsicPppoe(i,  &asic_pppoe) == FAILED)
			continue;
		seq_printf(s,"\t[%d]  sessionID(%d)  ageSec(%d)\n", i, asic_pppoe.sessionId, asic_pppoe.age);
	}
	return 0;
}
#else
static int32 pppoe_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	rtl865x_tblAsicDrv_pppoeParam_t asic_pppoe;
	int32	i;

	len = sprintf(page, "%s\n", "ASIC PPPOE Table:\n");
	for(i=0; i<RTL8651_PPPOETBL_SIZE; i++)
	{
		if (rtl8651_getAsicPppoe(i,  &asic_pppoe) == FAILED)
			continue;
		len += sprintf(page + len,"\t[%d]  sessionID(%d)  ageSec(%d)\n", i, asic_pppoe.sessionId, asic_pppoe.age);
	}
	return len;
}
#endif

static int32 pppoe_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

#endif

#if defined(CONFIG_RTL_LAYERED_DRIVER_L4)
int32 napt_show(struct seq_file *s, void *v)
{
	int len;
	rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_tcpudp;
	uint32 idx, entry=0;
	uint8 *ucp;

	len = seq_printf(s, "%s\n", "ASIC NAPT TCP/UDP Table:\n");

	for(idx=0; idx<RTL8651_TCPUDPTBL_SIZE; idx++) {
		#if defined(CONFIG_RTL_WTDOG)
		rtl_periodic_watchdog_kick(idx, WATCHDOG_NUM_OF_TIMES);
		#endif
		if (rtl8651_getAsicNaptTcpUdpTable(idx, &asic_tcpudp) == FAILED)
			continue;

		if (asic_tcpudp.isValid == 1 || asic_tcpudp.isDedicated == 1 ) {
//			len += seq_printf(s, "[%4d] %d.%d.%d.%d:%d {V,D}={%d,%d} col1(%d) col2(%d) static(%d) tcp(%d)\n",
//			       idx,
//			       asic_tcpudp.insideLocalIpAddr>>24, (asic_tcpudp.insideLocalIpAddr&0x00ff0000) >> 16,
//			       (asic_tcpudp.insideLocalIpAddr&0x0000ff00)>>8, asic_tcpudp.insideLocalIpAddr&0x000000ff,
//			       asic_tcpudp.insideLocalPort,
//			       asic_tcpudp.isValid, asic_tcpudp.isDedicated,
//			       asic_tcpudp.isCollision, asic_tcpudp.isCollision2, asic_tcpudp.isStatic, asic_tcpudp.isTcp );
			asic_tcpudp.insideLocalIpAddr = ntohl(asic_tcpudp.insideLocalIpAddr);
			ucp = (unsigned char *)&(asic_tcpudp.insideLocalIpAddr);
			len += seq_printf(s, "[%4d] %d.%d.%d.%d:%d {V,D}={%d,%d} col1(%d) col2(%d) static(%d) tcp(%d)\n",
			       idx, ucp[0], ucp[1], ucp[2], ucp[3],
			       asic_tcpudp.insideLocalPort,
			       asic_tcpudp.isValid, asic_tcpudp.isDedicated,
			       asic_tcpudp.isCollision, asic_tcpudp.isCollision2, asic_tcpudp.isStatic, asic_tcpudp.isTcp );

			len += seq_printf(s, "   age(%d) offset(%d) tcpflag(%d) SelEIdx(%d) SelIPIdx(%d) priValid:%d pri(%d)\n",
			        asic_tcpudp.ageSec, asic_tcpudp.offset<<10, asic_tcpudp.tcpFlag,
			        asic_tcpudp.selEIdx, asic_tcpudp.selExtIPIdx,asic_tcpudp.priValid,asic_tcpudp.priority );

#if defined(CONFIG_RTL_8197F)
			len += seq_printf(s, "   NHIDXValid(%d) NHIDX(%d)\n",
			        asic_tcpudp.NHIDXValid,asic_tcpudp.NHIDX);
#endif
			entry++;
		}
	}
	len += seq_printf(s, "Total entry: %d\n", entry);

	return 0;
}

int napt_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, napt_show, NULL));
}

struct file_operations napt_single_seq_file_operations = {
        .open           = napt_single_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

#if defined(CONFIG_RTL_LAYERED_DRIVER_L4)
extern int32 rtl865x_sw_napt_seq_read(struct seq_file *s, void *v);
extern int32 rtl865x_sw_napt_seq_write( struct file *filp, const char *buff,unsigned long len, loff_t *off );

int sw_napt_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl865x_sw_napt_seq_read, NULL));
}

static ssize_t sw_napt_proc_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return rtl865x_sw_napt_seq_write(file, userbuf,count, off);
}

struct file_operations sw_napt_single_seq_file_operations = {
        .open           = sw_napt_single_open,
	 .write		=sw_napt_proc_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif


#ifdef CONFIG_RTL_PROC_NEW
static int32 port_bandwidth_read(struct seq_file *s, void *v)
{
	uint32	regData;
	uint32	data0, data1;
	int		port;
	#if defined(CONFIG_RTL_8197F)
	uint32 	regData2;
	uint32	pBandwidth_high;
	#endif

	seq_printf(s, "Dump Port Bandwidth Info:\n");

	for(port=0;port<=CPU;port++)
	{
		regData = READ_MEM32(IBCR0+((port>>1)<<2));
		if(port&1)
		{
			data0 = (regData&IBWC_ODDPORT_MASK)>>IBWC_ODDPORT_OFFSET;
		}
		else
		{
			data0 = (regData&IBWC_EVENPORT_MASK)>>IBWC_EVENPORT_OFFSET;
		}

		regData = READ_MEM32(WFQRCRP0+((port*3)<<2));
		data1 = (regData&APR_MASK)>>APR_OFFSET;

#if defined(CONFIG_RTL_8197F)
		regData2 = READ_MEM32(IBCR3);
		pBandwidth_high = (regData2&(IBCR3_PORT_MASK<<IBCR3_PORT_OFFSET(port)))>>IBCR3_PORT_OFFSET(port);
		data0 = (pBandwidth_high<<IBCR3_HIGH_OFFSET) |data0;

		if (data0) {
			seq_printf(s, "Port%d Ingress:[%d.%03dMbps]	", port, (data0<<10)/1000000, ((data0<<10)%1000000)/1000);
		} else {
			seq_printf(s, "Port%d Ingress:FullSpeed	", port);
		}

		if (data1 != (APR_MASK>>APR_OFFSET)) {
			data1++;
			seq_printf(s, "Engress:[%d.%03dMbps]\n", (data1<<10)/1000000, ((data1<<10)%1000000)/1000);
		} else {
			seq_printf(s, "Egress:FullSpeed\n");
		}
#else
		data0++;
		if (data0)
		{
			if (data0<64)
				seq_printf(s, "Port%d Ingress:[%dKbps]	", port, (data0<<4)/1000);
			else
				seq_printf(s, "Port%d Ingress:[%d.%03dMbps]	", port, (data0<<14)/1000000, ((data0<<14)%1000000)/1000);
		}
		else
		{
			seq_printf(s, "Port%d Ingress:FullSpeed	", port);
		}

		if(data1!=(APR_MASK>>APR_OFFSET))
		{
			data1++;

			if (data1<16)
				seq_printf(s, "Engress:[%dKbps]\n", (data1<<16)/1000);
			else
				seq_printf(s, "Engress:[%d.%03dMbps]\n", (data1<<16)/1000000, ((data1<<16)%1000000)/1000);

		}
		else
			seq_printf(s, "Egress:FullSpeed\n");
#endif
	}

	return 0;
}
#else
static int32 port_bandwidth_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData;
	uint32	data0, data1;
	int		port;
	#if defined(CONFIG_RTL_8197F)
	uint32 	regData2;
	uint32	pBandwidth_high;
	#endif

	len = sprintf(page, "Dump Port Bandwidth Info:\n");

	for(port=0;port<=CPU;port++)
	{
		regData = READ_MEM32(IBCR0+((port>>1)<<2));
		if(port&1)
		{
			data0 = (regData&IBWC_ODDPORT_MASK)>>IBWC_ODDPORT_OFFSET;
		}
		else
		{
			data0 = (regData&IBWC_EVENPORT_MASK)>>IBWC_EVENPORT_OFFSET;
		}

		regData = READ_MEM32(WFQRCRP0+((port*3)<<2));
		data1 = (regData&APR_MASK)>>APR_OFFSET;

#if defined(CONFIG_RTL_8197F)
		regData2 = READ_MEM32(IBCR3);
		pBandwidth_high = (regData2&(IBCR3_PORT_MASK<<IBCR3_PORT_OFFSET(port)))>>IBCR3_PORT_OFFSET(port);
		data0 = (pBandwidth_high<<IBCR3_HIGH_OFFSET) |data0;

		if (data0) {
			len += sprintf(page+len, "Port%d Ingress:[%d.%03dMbps]	", port, (data0<<10)/1000000, ((data0<<10)%1000000)/1000);
		} else {
			len += sprintf(page+len, "Port%d Ingress:FullSpeed	", port);
		}

		if (data1 != (APR_MASK>>APR_OFFSET)) {
			data1++;
			len += sprintf(page+len, "Engress:[%d.%03dMbps]\n", (data1<<10)/1000000, ((data1<<10)%1000000)/1000);
		} else {
			len += sprintf(page+len, "Egress:FullSpeed\n");
		}
#else
		data0++;
		if (data0)
		{
			if (data0<64)
				len += sprintf(page+len, "Port%d Ingress:[%dKbps]	", port, (data0<<4)/1000);
			else
				len += sprintf(page+len, "Port%d Ingress:[%d.%03dMbps]	", port, (data0<<14)/1000000, ((data0<<14)%1000000)/1000);
		}
		else
		{
			len += sprintf(page+len, "Port%d Ingress:FullSpeed	", port);
		}

		if(data1!=(APR_MASK>>APR_OFFSET))
		{
			data1++;

			if (data1<16)
				len += sprintf(page+len, "Engress:[%dKbps]\n", (data1<<16)/1000);
			else
				len += sprintf(page+len, "Engress:[%d.%03dMbps]\n", (data1<<16)/1000000, ((data1<<16)%1000000)/1000);

		}
		else
			len += sprintf(page+len, "Egress:FullSpeed\n");
#endif
	}

	return len;
}
#endif

static int32 port_bandwidth_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}


#ifdef CONFIG_RTL_PROC_NEW
static int32 queue_bandwidth_read(struct seq_file *s, void *v)
{
	uint32	regData;
	uint32	data0, data1;
	int		port, queue;

	if (queue_bandwidth_record_portmask==0)
	{
		seq_printf(s, "Please set the dump mask firstly.\n");
		return 0;
	}
	seq_printf(s, "Dump Queue Bandwidth Info:\n");

	for(port=PHY0;port<=CPU;port++)
	{
		if ((queue_bandwidth_record_portmask&(1<<port))==0)
			continue;

		regData = READ_MEM32(QNUMCR);
		data0 = (regData>>(port*3))&7;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		seq_printf(s, "Port%d Queue number:	%d\n", port, data0+1);
#else
		seq_printf(s, "Port%d Queue number:	%d\n", port, data0);
#endif
		for(queue=QUEUE0;queue<=QUEUE5;queue++)
		{
			rtl8651_getAsicQueueFlowControlConfigureRegister(port, queue, &data0);
			seq_printf(s, "	==>Queue%d FC %s | ", queue, data0?"Enabled":"Disabled");

			rtl8651_getAsicQueueWeight(port, queue, &data0, &data1);
			if (data0 == STR_PRIO)
			{
				seq_printf(s, "Type: STR\n");
			}
			else
			{
				seq_printf(s, "Type: WFQ [weight:%d]\n", data1+1);
			}

			regData = READ_MEM32(P0Q0RGCR+(queue<<2)+((port*6)<<2));
			data1 = (regData&L1_MASK)>>L1_OFFSET;

			if(data1==(L1_MASK>>L1_OFFSET))
			{
				seq_printf(s, "	    BurstSize UnLimit | ");
			}
			else
			{
				seq_printf(s, "	    BurstSize[%dKbps] | ", data1);
			}

			data0 = (regData&APR_MASK)>>APR_OFFSET;
			data1 = (regData&PPR_MASK)>>PPR_OFFSET;
			if(data0!=(APR_MASK>>APR_OFFSET))
			{
				data0++;
				if (data0<16)
					seq_printf(s, "Engress: avgRate[%dKbps], peakRate[%d]\n", data0<<6, 1<<data1);
				else
					seq_printf(s, "Engress: avgRate[%d.%3dMbps], peakRate[%d]\n", data0>>4, (data0&0xf)<<6, 1<<data1);

			}
			else
				seq_printf(s, "Egress: avgRate & peakRateFullSpeed\n");

		}
	}

	return 0;
}
#else
static int32 queue_bandwidth_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData;
	uint32	data0, data1;
	int		port, queue;

	if (queue_bandwidth_record_portmask==0)
	{
		len = sprintf(page, "Please set the dump mask firstly.\n");
		return len;
	}
	len = sprintf(page, "Dump Queue Bandwidth Info:\n");

	for(port=PHY0;port<=CPU;port++)
	{
		if ((queue_bandwidth_record_portmask&(1<<port))==0)
			continue;

		regData = READ_MEM32(QNUMCR);
		data0 = (regData>>(port*3))&7;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		len += sprintf(page+len, "Port%d Queue number:	%d\n", port, data0+1);
#else
		len += sprintf(page+len, "Port%d Queue number:	%d\n", port, data0);
#endif
		for(queue=QUEUE0;queue<=QUEUE5;queue++)
		{
			rtl8651_getAsicQueueFlowControlConfigureRegister(port, queue, &data0);
			len += sprintf(page+len, "	==>Queue%d FC %s | ", queue, data0?"Enabled":"Disabled");

			rtl8651_getAsicQueueWeight(port, queue, &data0, &data1);
			if (data0 == STR_PRIO)
			{
				len += sprintf(page+len, "Type: STR\n");
			}
			else
			{
				len += sprintf(page+len, "Type: WFQ [weight:%d]\n", data1+1);
			}

			regData = READ_MEM32(P0Q0RGCR+(queue<<2)+((port*6)<<2));
			data1 = (regData&L1_MASK)>>L1_OFFSET;

			if(data1==(L1_MASK>>L1_OFFSET))
			{
				len += sprintf(page+len, "	    BurstSize UnLimit | ");
			}
			else
			{
				len += sprintf(page+len, "	    BurstSize[%dKbps] | ", data1);
			}

			data0 = (regData&APR_MASK)>>APR_OFFSET;
			data1 = (regData&PPR_MASK)>>PPR_OFFSET;
			if(data0!=(APR_MASK>>APR_OFFSET))
			{
				data0++;
				if (data0<16)
					len += sprintf(page+len, "Engress: avgRate[%dKbps], peakRate[%d]\n", data0<<6, 1<<data1);
				else
					len += sprintf(page+len, "Engress: avgRate[%d.%3dMbps], peakRate[%d]\n", data0>>4, (data0&0xf)<<6, 1<<data1);

			}
			else
				len += sprintf(page+len, "Egress: avgRate & peakRateFullSpeed\n");

		}
	}

	return len;
}
#endif

static int32 queue_bandwidth_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char tmpbuf[16], *tokptr, *strptr;
	int	port;
	if(len>16)
	{
		goto errout;
	}

	memset(tmpbuf, 0, 16);

	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr = tmpbuf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			return len;
		}
		queue_bandwidth_record_portmask=(uint32)simple_strtol(tokptr, NULL, 0);

		rtlglue_printf("Dump info of: ");
		for(port=PHY0;port<=CPU;port++)
		{
			if ((1<<port)&queue_bandwidth_record_portmask)
				rtlglue_printf("port%d ", port);
		}
		rtlglue_printf("\n");
	}
	else
	{
errout:
		rtlglue_printf("error input\n");
	}

	return len;
}




#ifdef CONFIG_RTL_PROC_NEW
static int32 priority_decision_read(struct seq_file *s, void *v)
{
	uint32	regData;
	int		queue;

	seq_printf(s, "Dump Priority Infor:\n");

	regData = READ_MEM32(QIDDPCR);

	seq_printf(s, "NAPT[%d] ACL[%d] DSCP[%d] 8021Q[%d] PortBase[%d]\n",
		(regData & NAPT_PRI_MASK) >> NAPT_PRI_OFFSET,
		(regData & ACL_PRI_MASK) >> ACL_PRI_OFFSET,
		(regData & DSCP_PRI_MASK) >> DSCP_PRI_OFFSET,
		(regData & BP8021Q_PRI_MASK) >> BP8021Q_PRI_OFFSET,
		(regData & PBP_PRI_MASK) >> PBP_PRI_OFFSET);

	for(queue=0;queue<RTL8651_OUTPUTQUEUE_SIZE;queue++)
	{
		if(queue < 6){
			regData = READ_MEM32(UPTCMCR0+(queue<<2));
			seq_printf(s, "Queue number %d:\n", (queue+1));
			seq_printf(s, "Piority[0~7] Mapping to Queue[ %d %d %d %d %d %d %d %d ]\n",
				regData&0x7, (regData>>3)&0x7, (regData>>6)&0x7, (regData>>9)&0x7,
				(regData>>12)&0x7, (regData>>15)&0x7, (regData>>18)&0x7, (regData>>21)&0x7);
		}
		else{
		#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
			regData = READ_MEM32(UPTCMCR6+((queue-6)<<2));
			seq_printf(s, "Queue number %d:\n", (queue+1));
			seq_printf(s, "Piority[0~7] Mapping to Queue[ %d %d %d %d %d %d %d %d ]\n",
				regData&0x7, (regData>>3)&0x7, (regData>>6)&0x7, (regData>>9)&0x7,
				(regData>>12)&0x7, (regData>>15)&0x7, (regData>>18)&0x7, (regData>>21)&0x7);
		#endif
		}
	}

	return 0;
}
#else
static int32 priority_decision_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData;
	int		queue;

	len = sprintf(page, "Dump Priority Infor:\n");

	regData = READ_MEM32(QIDDPCR);

	len += sprintf(page+len, "NAPT[%d] ACL[%d] DSCP[%d] 8021Q[%d] PortBase[%d]\n",
		(regData & NAPT_PRI_MASK) >> NAPT_PRI_OFFSET,
		(regData & ACL_PRI_MASK) >> ACL_PRI_OFFSET,
		(regData & DSCP_PRI_MASK) >> DSCP_PRI_OFFSET,
		(regData & BP8021Q_PRI_MASK) >> BP8021Q_PRI_OFFSET,
		(regData & PBP_PRI_MASK) >> PBP_PRI_OFFSET);

	for(queue=0;queue<RTL8651_OUTPUTQUEUE_SIZE;queue++)
	{
		regData = READ_MEM32(UPTCMCR0+(queue<<2));
		len += sprintf(page+len, "Queue number %d:\n", (queue+1));
		len += sprintf(page+len, "Piority[0~7] Mapping to Queue[ %d %d %d %d %d %d %d %d ]\n",
			regData&0x7, (regData>>3)&0x7, (regData>>6)&0x7, (regData>>9)&0x7,
			(regData>>12)&0x7, (regData>>15)&0x7, (regData>>18)&0x7, (regData>>21)&0x7);
	}

	return len;
}
#endif

static int32 priority_decision_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

#if defined (CONFIG_RTL_ENABLE_RATELIMIT_TABLE)
#ifdef CONFIG_RTL_PROC_NEW
static int32 rate_limit_read(struct seq_file *s, void *v)
{
	rtl865x_tblAsicDrv_rateLimitParam_t asic_rl;
	uint32 entry;

	seq_printf(s, "Dump rate limit table:\n");
	for(entry=0; entry<RTL8651_RATELIMITTBL_SIZE; entry++) {
		if (rtl8651_getAsicRateLimitTable(entry, &asic_rl) == SUCCESS) {
			seq_printf(s, " [%d]  Token(%u)  MaxToken(%u)  remainTime Unit(%u)  \n\trefillTimeUnit(%u)  refillToken(%u)\n",
				entry, asic_rl.token, asic_rl.maxToken, asic_rl.t_remainUnit, asic_rl.t_intervalUnit, asic_rl.refill_number);
		}
		else seq_printf(s, " [%d]  Invalid entry\n", entry);
	}
	seq_printf(s, "\n");

	return 0;
}
#else
static int32 rate_limit_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	rtl865x_tblAsicDrv_rateLimitParam_t asic_rl;
	uint32 entry;

	len = sprintf(page, "Dump rate limit table:\n");
	for(entry=0; entry<RTL8651_RATELIMITTBL_SIZE; entry++) {
		if (rtl8651_getAsicRateLimitTable(entry, &asic_rl) == SUCCESS) {
			len += sprintf(page+len, " [%d]  Token(%u)  MaxToken(%u)  remainTime Unit(%u)  \n\trefillTimeUnit(%u)  refillToken(%u)\n",
				entry, asic_rl.token, asic_rl.maxToken, asic_rl.t_remainUnit, asic_rl.t_intervalUnit, asic_rl.refill_number);
		}
		else len += sprintf(page+len, " [%d]  Invalid entry\n", entry);
	}
	len += sprintf(page+len, "\n");

	return len;
}
#endif

static int32 rate_limit_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}
#endif


#ifdef CONFIG_RTL_PROC_NEW
static int32 storm_read(struct seq_file *s, void *v)
{
	uint32	regData;
	uint32 port;
	uint32 totalExtPortNum=3;
	seq_printf(s, "Dump storm control info:\n");

	regData = READ_MEM32(BSCR);
	seq_printf(s, "rate(%d)\n",regData*100/30360);

	for ( port = 0; port < RTL8651_PORT_NUMBER + totalExtPortNum; port++ )
	{
		regData = READ_MEM32(PCRP0+port*4);
		seq_printf(s,"port%d, %s BCSC, %s BC, %s MC\n", port,regData&ENBCSC?"enable":"disable",regData&BCSC_ENBROADCAST?"enable":"disable",
					regData&BCSC_ENMULTICAST?"enable":"disable");
	}
	return 0;
}
#else
static int32 storm_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData;
	uint32 port;
	uint32 totalExtPortNum=3;
	len = sprintf(page, "Dump storm control info:\n");

	regData = READ_MEM32(BSCR);
	len += sprintf(page+len, "rate(%d)\n",regData*100/30360);

	for ( port = 0; port < RTL8651_PORT_NUMBER + totalExtPortNum; port++ )
	{
		regData = READ_MEM32(PCRP0+port*4);
		len+= sprintf(page+len,"port%d, %s BCSC, %s BC, %s MC\n", port,regData&ENBCSC?"enable":"disable",regData&BCSC_ENBROADCAST?"enable":"disable",
					regData&BCSC_ENMULTICAST?"enable":"disable");
	}

	if (len <= off+count)
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count)
		len = count;

	if (len<0) len = 0;

	return len;
}
#endif

static int32 storm_write(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	uint32 tmpBuf[32];
	uint32 stormCtrlType=0x3;//mc & bc
	uint32 enableStormCtrl=FALSE;
	uint32 percentage=0;
	uint32 uintVal;
	if(count>32)
	{
		goto errout;
	}

	if (buffer && !copy_from_user(tmpBuf, buffer, count))
	{
		tmpBuf[count-1]=0;
		uintVal=simple_strtoul((char *)tmpBuf, NULL, 0);
		rtlglue_printf("%s(%d) uintval(%u) \n",__FUNCTION__,__LINE__,uintVal);//Added for test
		if(uintVal>100)
		{
			enableStormCtrl=FALSE;
			percentage=0;
		}
		else
		{
			enableStormCtrl=TRUE;
			percentage=uintVal;
		}
		//printk("%s(%d),enableStormCtrl=%d,percentage=%d\n",__FUNCTION__,__LINE__,
		//	enableStormCtrl,percentage);//Added for test
		rtl865x_setStormControl(stormCtrlType,enableStormCtrl,percentage);
		return count;
	}
	else
	{
errout:
		rtlglue_printf("error input\n");
	}
	return -EFAULT;
}


#endif

#if defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)
//the following proc default established
#ifdef CONFIG_RTL_819X_SWCORE
extern int cnt_swcore ;
extern int cnt_swcore_tx;
extern int cnt_swcore_rx;
extern int cnt_swcore_link;
extern int cnt_swcore_err;
#endif
#if defined(CONFIG_RTL_8197F) &&  defined(CONFIG_FINETUNE_RUNOUT_IRQ)
extern int freeSkbThreshold;
#endif

#if defined(CONFIG_RTL_ETH_PRIV_SKB)
extern int get_buf_in_poll(void);
extern int get_buf_in_rx_skb_queue(void);
#endif
unsigned int statistic_udp_frag_rx;
unsigned int statistic_udp_frag_ps;
unsigned int statistic_udp_frag_tbl_full;
unsigned int statistic_udp_frag_add;
unsigned int statistic_udp_frag_free;

#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
unsigned int statistic_wlan_out_napt_exist;
unsigned int statistic_wlan_xmit_to_eth;
unsigned int statistic_wlan_rx_unicast_pkt;
unsigned int statistic_eth_xmit_to_wlan;
unsigned int statistic_eth_rx_ext_fwd_failed_pkt;
unsigned int statistic_eth_rx_unicast_pkt;

#endif

#ifdef CONFIG_RTL_PROC_NEW
static int32 stats_debug_entry_read(struct seq_file *s, void *v)
{
	seq_printf(s, "  Debug Statistics Info:\n");
#if defined(CONFIG_RTL_ETH_PRIV_SKB)
	seq_printf(s,"    eth_skb_free_num:	%d (pool %d, rtk_que %d)\n",
		get_buf_in_poll() + get_buf_in_rx_skb_queue(),
		get_buf_in_poll() , get_buf_in_rx_skb_queue());
#endif
	seq_printf(s,"    rx_noBuffer_cnt:	%u\n",rx_noBuffer_cnt);
	seq_printf(s,"    tx_ringFull_cnt:	%u\n",tx_ringFull_cnt);
	seq_printf(s,"    tx_drop_cnt:	%u\n",tx_drop_cnt);
	
	seq_printf(s,"    statistic_total:	%d\n",statistic_total);
	seq_printf(s,"    statistic_ps:	%d\n",statistic_ps);
	seq_printf(s,"    statistic_ipv6_fp:	%d\n",statistic_ipv6_fp);
	#if defined(CONFIG_OPENWRT_SDK)
	seq_printf(s,"    statistic_fastpath:	%d\n",statistic_fastpath);
	#else
	seq_printf(s,"    statistic_fp:	%d\n",statistic_fp);
	#endif

#ifdef CONFIG_RTL_819X_SWCORE
	seq_printf(s,"    cnt_swcore:	%d\n",cnt_swcore);
	seq_printf(s,"    cnt_swcore_tx:	%d\n",cnt_swcore_tx);
	seq_printf(s,"    cnt_swcore_rx:	%d\n",cnt_swcore_rx);
	seq_printf(s,"    cnt_swcore_link:	%d\n",cnt_swcore_link);
	seq_printf(s,"    cnt_swcore_err:	%d\n",cnt_swcore_err);

#endif

#ifdef CONFIG_RTL_ADAPTABLE_TSO
	seq_printf(s,"    TSO:		%s\n",(tso_feature & (NETIF_F_TSO | NETIF_F_TSO_ECN))? "ON":"OFF");	
#endif
	
#if defined(CONFIG_RTL_8197F) &&  defined(CONFIG_FINETUNE_RUNOUT_IRQ)
	if(	REG32(CPUIIMR)&(PKTHDR_DESC_RUNOUT_IP_ALL|MBUF_DESC_RUNOUT_IP_ALL))
	{
		seq_printf(s,"    runOutIrq:	enable\n");
	}
	else
	{
		seq_printf(s,"    runOutIrq:	disable\n");
	}
	seq_printf(s,"    freeSkbThreshold:	%d\n",freeSkbThreshold);
#endif
	seq_printf(s,"    statistic_udp_frag_rx %u\n", statistic_udp_frag_rx);
	seq_printf(s,"    statistic_udp_frag_ps %u\n", statistic_udp_frag_ps);
	seq_printf(s,"    statistic_udp_frag_tbl_full  %u\n", statistic_udp_frag_tbl_full);
	seq_printf(s,"    statistic_udp_frag_add %u\n", statistic_udp_frag_add);
	seq_printf(s,"    statistic_udp_frag_free %u\n", statistic_udp_frag_free);
	#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	seq_printf(s,"    statistic_wlan_rx_unicast_pkt %u\n", statistic_wlan_rx_unicast_pkt);
	seq_printf(s,"    statistic_wlan_out_napt_exist %u\n", statistic_wlan_out_napt_exist);
	seq_printf(s,"    statistic_wlan_xmit_to_eth %u\n", statistic_wlan_xmit_to_eth);
	seq_printf(s,"    statistic_eth_rx_unicast_pkt  %u\n", statistic_eth_rx_unicast_pkt);
	seq_printf(s,"    statistic_eth_xmit_to_wlan  %u\n", statistic_eth_xmit_to_wlan);
	seq_printf(s,"    statistic_eth_rx_ext_fwd_failed_pkt %u\n", statistic_eth_rx_ext_fwd_failed_pkt);
	
	#endif
	
	return 0;
}
#else
static int32 stats_debug_entry_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int 	len;

	len = sprintf(page, "  Debug Statistics Info:\n");
#if defined(CONFIG_RTL_ETH_PRIV_SKB)
	len += sprintf(page+len,"    eth_skb_free_num:	%d (pool %d, rtk_que %d)\n",
		get_buf_in_poll() + get_buf_in_rx_skb_queue(),
		get_buf_in_poll() , get_buf_in_rx_skb_queue());
#endif
	len += sprintf(page+len,"    rx_noBuffer_cnt:	%u\n",rx_noBuffer_cnt);
	len += sprintf(page+len,"    tx_ringFull_cnt:	%u\n",tx_ringFull_cnt);
	len += sprintf(page+len,"    tx_drop_cnt:	%u\n",tx_drop_cnt);

	len += sprintf(page+len,"    statistic_total:	%d\n",statistic_total);
	len += sprintf(page+len,"    statistic_ps:	%d\n",statistic_ps);
	len += sprintf(page+len,"    statistic_ipv6_fp:	%d\n",statistic_ipv6_fp);
	#if defined(CONFIG_OPENWRT_SDK)
	len += sprintf(page+len,"    statistic_fastpath:	%d\n",statistic_fastpath);
	#else
	len += sprintf(page+len,"    statistic_fp:	%d\n",statistic_fp);
	#endif
#ifdef CONFIG_RTL_819X_SWCORE
	len += sprintf(page+len,"	  cnt_swcore:	%d\n",cnt_swcore);
	len += sprintf(page+len,"	  cnt_swcore_tx:	%d\n",cnt_swcore_tx);
	len += sprintf(page+len,"	  cnt_swcore_rx:	%d\n",cnt_swcore_rx);
	len += sprintf(page+len,"	  cnt_swcore_link:	%d\n",cnt_swcore_link);
	len += sprintf(page+len,"	  cnt_swcore_err:	%d\n",cnt_swcore_err);	
#endif
	len += sprintf(page+len,"    statistic_udp_frag_rx %u\n", statistic_udp_frag_rx);
	len += sprintf(page+len,"    statistic_udp_frag_ps %u\n", statistic_udp_frag_ps);
	len += sprintf(page+len,"    statistic_udp_frag_tbl_full  %u\n", statistic_udp_frag_tbl_full);
	len += sprintf(page+len,"    statistic_udp_frag_add %u\n", statistic_udp_frag_add);
	len += sprintf(page+len,"    statistic_udp_frag_free %u\n", statistic_udp_frag_free);
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	len += sprintf(page+len,"    statistic_wlan_rx_unicast_pkt %u\n", statistic_wlan_rx_unicast_pkt);
	len += sprintf(page+len,"    statistic_wlan_out_napt_exist %u\n", statistic_wlan_out_napt_exist);
	len += sprintf(page+len,"    statistic_wlan_xmit_to_eth %u\n", statistic_wlan_xmit_to_eth);
	len += sprintf(page+len,"    statistic_eth_rx_unicast_pkt  %u\n", statistic_eth_rx_unicast_pkt);
	len += sprintf(page+len,"    statistic_eth_xmit_to_wlan  %u\n", statistic_eth_xmit_to_wlan);
	len += sprintf(page+len,"    statistic_eth_rx_ext_fwd_failed_pkt %u\n", statistic_eth_rx_ext_fwd_failed_pkt);
#endif

	return len;
		
}
#endif
/*echo clear > /proc/rtl865x/stats to clear the count of tx&rx no buffer count*/
static int32 stats_debug_entry_write(struct file *file, const char *buffer,
		      unsigned long len, void *data)

{
	char tmpBuf[32];
	char		*strptr;
	char 		*cmd_addr;
	char		*tokptr;

#ifdef CONFIG_RTL_ROMEPERF_24K
	char tmpBufperf[32];
	int i,j;
#endif

	if(len>32)
	{
		goto errout;
	}
	if (buffer && !copy_from_user(tmpBuf, buffer, len)) {

		tmpBuf[len] = '\0';

#ifdef CONFIG_RTL_ROMEPERF_24K
		for (j=0; j<=len; j++){
			tmpBufperf[j] = tmpBuf[j];
		}
#endif
	
		strptr=tmpBuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}

		if(strncmp(cmd_addr, "clear",5) == 0)
		{
			statistic_udp_frag_rx = 0;
			statistic_udp_frag_ps = 0;
			statistic_udp_frag_tbl_full = 0;
			statistic_udp_frag_add = 0;
			statistic_udp_frag_free = 0;
			rx_noBuffer_cnt=0;
			tx_ringFull_cnt=0;
			tx_drop_cnt=0;
			
			statistic_total = 0;
			statistic_ps = 0;
			statistic_ipv6_fp = 0;
			#if defined(CONFIG_OPENWRT_SDK)
			statistic_fastpath = 0;
			#else
			statistic_fp = 0;
			#endif
			#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
			statistic_wlan_rx_unicast_pkt = 0;
			statistic_wlan_out_napt_exist = 0;
			statistic_wlan_xmit_to_eth = 0;
			statistic_eth_rx_unicast_pkt = 0;
			statistic_eth_xmit_to_wlan = 0;
			statistic_eth_rx_ext_fwd_failed_pkt = 0;
			#endif
			
#ifdef CONFIG_RTL_ROMEPERF_24K
			cp3Value[0]=0;
			cp3Count[0]=0;
			memset( &cp3romePerfStat, 0, sizeof( cp3romePerfStat ) );
			memset( &avrgCycle, 0, sizeof(avrgCycle));
#endif			
#ifdef CONFIG_RTL_819X_SWCORE
			cnt_swcore = 0;
			cnt_swcore_tx = 0;
			cnt_swcore_rx = 0;
			cnt_swcore_link = 0;
			cnt_swcore_err = 0;
#endif


		}
		else if(strncmp(cmd_addr, "debug",5) == 0)
		{
			tokptr = strsep(&strptr," ");
			if (tokptr)
				_debug_flag = simple_strtoul(tokptr, NULL, 16);
			
			rtlglue_printf("_debug_flag= 0x%x\n", _debug_flag);
		}
#ifdef UDP_FRAGMENT_PKT_QUEUEING
		else if(strncmp(cmd_addr, "help", 4) == 0)
		{
			rtlglue_printf(">> usage:\n");
			rtlglue_printf("   echo uf_queue > /proc/rtl865x/stats\n");
			rtlglue_printf("   echo uf_queue enable > /proc/rtl865x/stats\n");
			rtlglue_printf("   echo uf_queue disable > /proc/rtl865x/stats\n");
		}
		else if(strncmp(cmd_addr, "uf_queue", 8) == 0)
		{
			tokptr = strsep(&strptr," ");
			if (tokptr) {
				if (strncmp(tokptr, "enable", 6) == 0)
					uf_enabled = 1;
				else if (strncmp(tokptr, "disable", 7) == 0)
					uf_enabled = 0;
				else
					uf_tx_desc_low = simple_strtoul(tokptr, NULL, 10);
			}
			rtlglue_printf("udp fragment queueing: %sabled\n", (uf_enabled)? "en" : "dis");
			rtlglue_printf("uf_tx_desc_low (tx desc low-water mark)= %d\n", uf_tx_desc_low);
			
			#ifdef _UF_DEBUG
			rtlglue_printf("uf_start= %d\n", uf_start);
			rtlglue_printf("_uf_cntr_tx_group_pkt= %d\n", _uf_cntr_tx_group_pkt);
			rtlglue_printf("_uf_cntr_free_group_pkt= %d\n", _uf_cntr_free_group_pkt);
			rtlglue_printf("_uf_cntr_timer_timeout= %d\n", _uf_cntr_timer_timeout);
			rtlglue_printf("_uf_cntr_tx_unfinished_group_pkt= %d\n", _uf_cntr_tx_unfinished_group_pkt);
			rtlglue_printf("_uf_cntr_queue_end= %d\n", _uf_cntr_queue_end);
			#endif
		}
#endif

#if defined(CONFIG_RTL_8197F) &&  defined(CONFIG_FINETUNE_RUNOUT_IRQ)
		else if(strncmp(cmd_addr, "freeSkbThreshold",16) == 0)
		{

			tokptr = strsep(&strptr," ");
			
			if (tokptr)
				freeSkbThreshold = simple_strtoul(tokptr, NULL, 0);
			else
				return -1;
			
			rtlglue_printf("freeSkbThreshold= %d\n", freeSkbThreshold);
			
		}
#endif
#ifdef CONFIG_RTL_ROMEPERF_24K
		else {			

			int start, end, num;
			char cmd[10];

			num = sscanf(tmpBufperf, "%s %d %d", cmd, &start, &end);

			if(strncmp(cmd, "dump",4) != 0){
				printk("invalid romeperf command!\n");
				return -1;
			}
			
			if (num !=  3) {
				printk("invalid romeperf parameter!\n");
				return -1;
			}

			if((start<0) ||(start>=CP3ROMEPERF_INDEX_MAX)||(end<0) ||(end>=CP3ROMEPERF_INDEX_MAX) ||(start>end)){
				printk("Bad index!\n");
				return -1;
			}
			
			for( i = start; i <= end; i++ )
			{
				avrgCycle[i] = div64_u64(cp3romePerfStat[i].accCycle[0],cp3romePerfStat[i].executedNum);

				if(cp3romePerfStat[i].executedNum == 0)
					avrgCycle[i] = 0;
				
				printk("index[%d] accCycle[0]=%llu executedNum=%llu average=%llu\n",i,cp3romePerfStat[i].accCycle[0],cp3romePerfStat[i].executedNum, avrgCycle[i]);


				avrgCycle[i] = div64_u64(cp3romePerfStat[i].accCycle[1],cp3romePerfStat[i].executedNum);

				if(cp3romePerfStat[i].executedNum == 0)
					avrgCycle[i] = 0;

				printk("index[%d] accCycle[1]=%llu executedNum=%llu average=%llu\n",i,cp3romePerfStat[i].accCycle[1],cp3romePerfStat[i].executedNum, avrgCycle[i]);
			}

		}
#endif //end CONFIG_RTL_ROMEPERF_24K

	}
	else
	{
errout:
			rtlglue_printf("error input\n");
	}
	return len;
}


#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)
#ifdef CONFIG_RTL_PROC_NEW
static int32 rtl819x_proc_privSkbInfo_read(struct seq_file *s, void *v)
{
	int cpu_queue_cnt,rxRing_cnt,txRing_cnt,wlan_txRing_cnt,wlan_txRing_cnt1, mbuf_pending_cnt;
	int rx_skb_queue_cnt,poll_buf_cnt;
	unsigned long flags=0;

	seq_printf(s, "original eth private buf total(%d)\n",MAX_ETH_SKB_NUM);

	SMP_LOCK_ETH(flags);
	mbuf_pending_cnt = get_mbuf_pending_times();
	cpu_queue_cnt = get_cpu_completion_queue_num();
	rxRing_cnt = get_nic_rxRing_buf();
	txRing_cnt = get_nic_txRing_buf();
#if defined(CONFIG_RTL8192CD)
	wlan_txRing_cnt = get_nic_buf_in_wireless_tx("wlan0");
	wlan_txRing_cnt1 = get_nic_buf_in_wireless_tx("wlan1");
#else
	wlan_txRing_cnt = 0;
	wlan_txRing_cnt1 = 0;
#endif
	rx_skb_queue_cnt = get_buf_in_rx_skb_queue();
	poll_buf_cnt = get_buf_in_poll();
	SMP_UNLOCK_ETH(flags);

	seq_printf(s,"cpu completion cnt(%d)\nnic rxring cnt(%d)\nnic txring cnt(%d)\nwlan0 txring cnt(%d)\nwlan1 txring cnt(%d)\nrx skb queue cnt(%d)\npool buf cnt(%d)\nmbuf pending times(%d)\nsum(%d)\n",
	cpu_queue_cnt,rxRing_cnt,txRing_cnt,wlan_txRing_cnt,wlan_txRing_cnt1, rx_skb_queue_cnt,poll_buf_cnt,mbuf_pending_cnt,
	cpu_queue_cnt+rxRing_cnt+txRing_cnt+wlan_txRing_cnt+wlan_txRing_cnt1+rx_skb_queue_cnt+poll_buf_cnt);

	return 0;
}
#else
static int32 rtl819x_proc_privSkbInfo_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	int cpu_queue_cnt,rxRing_cnt,txRing_cnt,wlan_txRing_cnt,wlan_txRing_cnt1, mbuf_pending_cnt;
	int rx_skb_queue_cnt,poll_buf_cnt;
	unsigned long flags=0;

	len = sprintf(page, "original eth private buf total(%d)\n",MAX_ETH_SKB_NUM);

	SMP_LOCK_ETH(flags);
	mbuf_pending_cnt = get_mbuf_pending_times();
	cpu_queue_cnt = get_cpu_completion_queue_num();
	rxRing_cnt = get_nic_rxRing_buf();
	txRing_cnt = get_nic_txRing_buf();
#if defined(CONFIG_RTL8192CD)
	wlan_txRing_cnt = get_nic_buf_in_wireless_tx("wlan0");
	wlan_txRing_cnt1 = get_nic_buf_in_wireless_tx("wlan1");
#else
	wlan_txRing_cnt = 0;
	wlan_txRing_cnt1 = 0;
#endif
	rx_skb_queue_cnt = get_buf_in_rx_skb_queue();
	poll_buf_cnt = get_buf_in_poll();
	SMP_UNLOCK_ETH(flags);

	len+=sprintf(page + len,"cpu completion cnt(%d)\nnic rxring cnt(%d)\nnic txring cnt(%d)\nwlan0 txring cnt(%d)\nwlan1 txring cnt(%d)\nrx skb queue cnt(%d)\npool buf cnt(%d)\nmbuf pending times(%d)\nsum(%d)\n",
	cpu_queue_cnt,rxRing_cnt,txRing_cnt,wlan_txRing_cnt,wlan_txRing_cnt1, rx_skb_queue_cnt,poll_buf_cnt,mbuf_pending_cnt,
	cpu_queue_cnt+rxRing_cnt+txRing_cnt+wlan_txRing_cnt+wlan_txRing_cnt1+rx_skb_queue_cnt+poll_buf_cnt);

	return len;
}
#endif


static int32 rtl819x_proc_privSkbInfo_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}
#else
#ifdef CONFIG_RTL_PROC_NEW
static int32 rtl819x_proc_privSkbInfo_read(struct seq_file *s, void *v)
{
	return PROC_READ_RETURN_VALUE;
}
#else
static int32 rtl819x_proc_privSkbInfo_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	return PROC_READ_RETURN_VALUE;
}
#endif

static int32 rtl819x_proc_privSkbInfo_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	#if 0
	char  tmpbuf[32];
	if(len>32)
	{
		goto errout;
	}
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len -1] = '\0';
		if(tmpbuf[0] == '1') {
			rtl_dumpIndexs();
		}
	}
	else
	{
errout:
		rtlglue_printf("error input\n");
	}
	#endif
	return len;
}
#endif


#ifdef CONFIG_RTL_PROC_NEW
static int32 diagnostic_read(struct seq_file *s, void *v)
{
	uint32	regData, regData1;
	int		port, regIdx;
	uint32	mask, offset;

	seq_printf(s, "Diagnostic Register Info:\n");

	regData = READ_MEM32(GDSR0);
	seq_printf(s, "MaxUsedDescriptor: %d CurUsed Descriptor: %d\n",
		(regData&MaxUsedDsc_MASK)>>MaxUsedDsc_OFFSET,
		(regData&USEDDSC_MASK)>>USEDDSC_OFFSET);
	seq_printf(s, "DescRunOut: %s TotalDescFC: %s ShareBufFC: %s\n",
		(regData&DSCRUNOUT)?"YES":"NO", (regData&TotalDscFctrl_Flag)?"YES":"NO", (regData&SharedBufFCON_Flag)?"YES":"NO");

	for(regIdx = 0; regIdx<2; regIdx++)
	{
		regData = READ_MEM32(PCSR0+(regIdx<<2));

		for(port=0; port<4; port++)
		{
			switch(port)
			{
				case 0:
					mask = P0OQCgst_MASK;
					offset = P0OQCgst_OFFSET;
					break;
				case 1:
					mask = P1OQCgst_MASK;
					offset = P1OQCgst_OFFSET;
					break;
				case 2:
					mask = P2OQCgst_MASK;
					offset = P2OQCgst_OFFSET;
					break;
				default:
					mask = P3OQCgst_MASK;
					offset = P3OQCgst_OFFSET;
					break;
			}
			regData1 = (regData&mask)>>offset;
			if (regData1==0)
				seq_printf(s, "Port%d not congestion\n", port+(regIdx<<2));
			else
			{
				seq_printf(s, "Port%d queue congestion mask 0x%x\n", port+(regIdx<<2), regData1);
			}
		}
	}

	for(port=0;port<=CPU;port++)
	{
		#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F) 
		seq_printf(s, "Port%d each queue used descriptor: Queue[0~7]: [ ", port);
		#else
		seq_printf(s, "Port%d each queue used descriptor: Queue[0~5]: [ ", port);
		#endif
		for(regIdx=0; regIdx<3; regIdx++)
		{
			regData = READ_MEM32(P0_DCR0+(port<<4)+(regIdx<<2));
			seq_printf(s, "%d %d ",
				((regData&Pn_EQDSCR_MASK)>>Pn_EVEN_OQDSCR_OFFSET),
				((regData&Pn_OQDSCR_MASK)>>Pn_ODD_OQDSCR_OFFSET));
		}

		regData = READ_MEM32(P0_DCR3+(port<<4));

		/*Level 7&6 priority output queue*/
		#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F) 
		seq_printf(s, "%d %d ",
			((regData&(0x3ff<<10))>>10),
			((regData&(0x3ff<<20))>>20));
		#endif

		
		seq_printf(s, "]  Input queue [%d]\n",
				((regData&Pn_EQDSCR_MASK)>>Pn_EVEN_OQDSCR_OFFSET));
	}

	return 0;
}
#else
static int32 diagnostic_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData, regData1;
	int		port, regIdx;
	uint32	mask, offset;

	len = sprintf(page, "Diagnostic Register Info:\n");

	regData = READ_MEM32(GDSR0);
	len += sprintf(page + len, "MaxUsedDescriptor: %d CurUsed Descriptor: %d\n",
		(regData&MaxUsedDsc_MASK)>>MaxUsedDsc_OFFSET,
		(regData&USEDDSC_MASK)>>USEDDSC_OFFSET);
	len += sprintf(page+len, "DescRunOut: %s TotalDescFC: %s ShareBufFC: %s\n",
		(regData&DSCRUNOUT)?"YES":"NO", (regData&TotalDscFctrl_Flag)?"YES":"NO", (regData&SharedBufFCON_Flag)?"YES":"NO");

	for(regIdx = 0; regIdx<2; regIdx++)
	{
		regData = READ_MEM32(PCSR0+(regIdx<<2));

		for(port=0; port<4; port++)
		{
			switch(port)
			{
				case 0:
					mask = P0OQCgst_MASK;
					offset = P0OQCgst_OFFSET;
					break;
				case 1:
					mask = P1OQCgst_MASK;
					offset = P1OQCgst_OFFSET;
					break;
				case 2:
					mask = P2OQCgst_MASK;
					offset = P2OQCgst_OFFSET;
					break;
				default:
					mask = P3OQCgst_MASK;
					offset = P3OQCgst_OFFSET;
					break;
			}
			regData1 = (regData&mask)>>offset;
			if (regData1==0)
				len += sprintf(page+len, "Port%d not congestion\n", port+(regIdx<<2));
			else
			{
				len += sprintf(page+len, "Port%d queue congestion mask 0x%x\n", port+(regIdx<<2), regData1);
			}
		}
	}

	for(port=0;port<=CPU;port++)
	{
		len += sprintf(page+len, "Port%d each queue used descriptor: Queue[0~5]: [ ", port);
		for(regIdx=0; regIdx<3; regIdx++)
		{
			regData = READ_MEM32(P0_DCR0+(port<<4)+(regIdx<<2));
			len += sprintf(page+len, "%d %d ",
				((regData&Pn_EQDSCR_MASK)>>Pn_EVEN_OQDSCR_OFFSET),
				((regData&Pn_OQDSCR_MASK)>>Pn_ODD_OQDSCR_OFFSET));
		}

		regData = READ_MEM32(P0_DCR3+(port<<4));
		len += sprintf(page+len, "]  Input queue [%d]\n",
				((regData&Pn_EQDSCR_MASK)>>Pn_EVEN_OQDSCR_OFFSET));
	}

	return len;
}
#endif

static int32 diagnostic_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

#if defined(CONFIG_RTL_NIC_QUEUE)
extern  int need_queue;
extern int hit_count_th;
extern int tp_th;
extern int acc_count_th;
extern int sw_buf_th;
extern int max_res_th;
extern int max_q_Len;
extern int dequeueMethod;
extern int tp_count;
extern int extra_token;
extern struct rtk_tx_queue  tx_skb_queue[TX_QUEUE_NUM];
extern struct rtk_tx_dev tx_dev_stats[TX_DEV_NUM];

static int32 tx_queue_read(struct seq_file *s, void *v)
{
	int i;
	seq_printf(s, "nic tx queue:\n");
	seq_printf(s, "need_queue %d dequeueMethod=%d tp_count=%d \n",need_queue,dequeueMethod, tp_count);
	seq_printf(s, "acc_count_th %d hit_count_th %d tp_th %d\n",acc_count_th,hit_count_th,tp_th);	
	seq_printf(s, "buffer %d extra_token %d\n",tx_skb_queue[0].tbf.buffer,extra_token);
	seq_printf(s,"\n");
	for(i=0;i<TX_QUEUE_NUM;i++) {
		seq_printf(s,"entry %d:\n",i);
		seq_printf(s,"used %d dev(%p)=%s phyPort=%d \n",tx_skb_queue[i].used,tx_skb_queue[i].dev,(tx_skb_queue[i].dev) ? tx_skb_queue[i].dev->name : "", tx_skb_queue[i].phyPort);
		if(tx_skb_queue[i].used)
			seq_printf(s,"skb queue len %d\n",skb_queue_len(&tx_skb_queue[i].list));
		seq_printf(s,"\n");
	}
	for(i=0;i<TX_DEV_NUM;i++) {
		seq_printf(s,"tx dev entry %d:\n",i);
#ifdef 	CONFIG_RTL_MULTI_LAN_DEV
		seq_printf(s,"hitcount %u dev(%p)=%s \n",tx_dev_stats[i].hit_count,tx_dev_stats[i].dev,(tx_dev_stats[i].dev) ? tx_dev_stats[i].dev->name : "");
#else
		seq_printf(s,"hitcount %u dport(%d) \n",tx_dev_stats[i].hit_count,tx_dev_stats[i].dport);
#endif
		seq_printf(s,"\n");
	}
	
	return 0;
}

static int32 tx_queue_write(struct file *filp, const char *buff,unsigned long len, void *data )
{
	char tmpbuf[64];	
	char *strptr, *tokeptr, *cmd_addr;
	int value;
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
	
		if(!memcmp(cmd_addr, "qth", 3)) {	
			
			tokeptr = strsep(&strptr," ");
			if (tokeptr==NULL)
			{
				goto errout;
			}	
			value=simple_strtol(tokeptr, NULL, 0);
			if(value)
				acc_count_th=value;

			tokeptr = strsep(&strptr," ");
			if (tokeptr==NULL)
			{
				goto errout;
			}
			value=simple_strtol(tokeptr, NULL, 0);
			if(value)
				hit_count_th=value;

			tokeptr = strsep(&strptr," ");
			if (tokeptr==NULL)
			{
				goto errout;
			}
			value=simple_strtol(tokeptr, NULL, 0);
			if(value)
				tp_th=value;


			tokeptr = strsep(&strptr," ");
			if (tokeptr==NULL)
			{
				goto errout;
			}
			value=simple_strtol(tokeptr, NULL, 0);
			if(value)
			{
				int i;
				for(i=0;i<TX_QUEUE_NUM;i++)
				{
					tx_skb_queue[i].tbf.buffer=value;
				}
			}


			tokeptr = strsep(&strptr," ");
			if (tokeptr==NULL)
			{
				goto errout;
			}
			value=simple_strtol(tokeptr, NULL, 0);
			extra_token=value;

			rtlglue_printf("acc_count_th %d  hit_count_th %d tp_th %d tbf.buffer %d extra_token %d\n",acc_count_th,hit_count_th,tp_th,tx_skb_queue[0].tbf.buffer,extra_token);		
			return len;
		}
 	}
errout:

	rtlglue_printf("tcp_queue_write qth/[acc_count_th] [hit_count_th] [tp_th] [tbf buffer] [tbf extra token]\n");
	return len;
}
#endif


#ifdef CONFIG_RTL_PROC_NEW
static int32 proc_mem_read(struct seq_file *s, void *v)
{
	return PROC_READ_RETURN_VALUE;
}
#else
static int32 proc_mem_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	return PROC_READ_RETURN_VALUE;
}
#endif

static int32 proc_mem_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[64];
	uint32	*mem_addr, mem_data, mem_len;
	char		*strptr, *cmd_addr;
	char		*tokptr;

	if(len>64)
	{
		goto errout;
	}
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
		rtlglue_printf("cmd %s\n", cmd_addr);
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "readb", 5))
		{
			mem_addr=(uint32*)simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mem_len=simple_strtol(tokptr, NULL, 0);
			memDump2(mem_addr, mem_len);
		}
		else if (!memcmp(cmd_addr, "read", 4))
		{
			mem_addr=(uint32*)simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mem_len=simple_strtol(tokptr, NULL, 0);
			memDump(mem_addr, mem_len, "");
		}
		else if (!memcmp(cmd_addr, "write", 5))
		{
			mem_addr=(uint32*)simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mem_data=simple_strtol(tokptr, NULL, 0);
			WRITE_MEM32(mem_addr, mem_data);
			rtlglue_printf("Write memory 0x%p dat 0x%x: 0x%x\n", mem_addr, mem_data, READ_MEM32(mem_addr));
		}
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		rtlglue_printf("Memory operation only support \"read\" and \"write\" as the first parameter\n");
		rtlglue_printf("Read format:	\"read mem_addr length\"\n");
		rtlglue_printf("Write format:	\"write mem_addr mem_data\"\n");
	}

	return len;
}



#ifdef CONFIG_RTL_PROC_NEW
static int32 port_status_read(struct seq_file *s, void *v)
{
	int		port;
	
#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)
	seq_printf(s, "Dump Port Status:\n");

	for(port=PHY0;port<=PHY4;port++)
	{
		extern int  rtk_port_macStatus_get(int port, rtk_port_mac_ability_t *pPortstatus);
		rtk_port_mac_ability_t sts;
		if ((rtk_port_macStatus_get(port, &sts)) == 0) {
			seq_printf(s, "Port%d ", port);

			if (sts.link == 0) {
				seq_printf(s, "LinkDown\n\n");
				continue;
			}
			else {
				seq_printf(s, "LinkUp | ");
			}
			seq_printf(s, "NWay Mode %s\n", (sts.nway) ?"Enabled":"Disabled");
			seq_printf(s, "	RXPause %s | ", (sts.rxpause) ?"Enabled":"Disabled");
			seq_printf(s, "TXPause %s\n", (sts.txpause) ?"Enabled":"Disabled");
			seq_printf(s, "	Duplex %s | ", (sts.duplex) ?"Enabled":"Disabled");
			seq_printf(s, "Speed %s\n\n", (sts.speed) ==PortStatusLinkSpeed100M?"100M":
				((sts.speed) ==PortStatusLinkSpeed1000M?"1G":
					((sts.speed) ==PortStatusLinkSpeed10M?"10M":"Unkown")));
					
		}
	}
#else
	uint32	regData, data0, regData_PCR;

	seq_printf(s, "Dump Port Status:\n");
	for(port=PHY0;port<=CPU;port++)
	{
		regData = READ_MEM32(PSRP0+((port)<<2));
		regData_PCR = READ_MEM32(PCRP0+((port)<<2));
		
		if (port==CPU)
			seq_printf(s, "CPUPort ");
		else
			seq_printf(s, "Port%d ", port);
		
		data0 = regData_PCR & EnForceMode;
		if(data0)
		{
			seq_printf(s, "Enforce Mode ");
			data0 = regData_PCR & PollLinkStatus;
			if (data0)
				seq_printf(s, " | polling LinkUp");
			else
			{
				seq_printf(s, " | disable Auto-Negotiation\n\n");
			}
		}
		else
		{
			seq_printf(s, "Force Mode disable\n");
		}
		
		regData = READ_MEM32(PSRP0+((port)<<2));
		data0 = regData & PortStatusLinkUp;

#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
		if ((port == PHY0) && gpio_simulate_mdc_mdio) {
			int rp;
			uint32 regData2;

			for (rp = 0; rp < 2; rp++)
				rtl_mdio_read(PORT0_RGMII_PHYID, 1, &regData2);

			if (regData2 & (1<<2))
				seq_printf(s, "LinkUp | ");
			else {
				seq_printf(s, "LinkDown\n\n");
				continue;
			}
		} else
#endif
		{
			if (data0)
				seq_printf(s, "LinkUp | ");
			else
			{
				seq_printf(s, "LinkDown\n\n");
				continue;
			}
		}
		data0 = regData & PortStatusNWayEnable;
		seq_printf(s, "NWay Mode %s\n", data0?"Enabled":"Disabled");
		data0 = regData & PortStatusRXPAUSE;
		seq_printf(s, "	RXPause %s | ", data0?"Enabled":"Disabled");
		data0 = regData & PortStatusTXPAUSE;
		seq_printf(s, "TXPause %s\n", data0?"Enabled":"Disabled");
		data0 = regData & PortStatusDuplex;
		seq_printf(s, "	Duplex %s | ", data0?"Enabled":"Disabled");
		data0 = (regData&PortStatusLinkSpeed_MASK)>>PortStatusLinkSpeed_OFFSET;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		seq_printf(s, "Speed %s\n", data0==PortStatusLinkSpeed100M?"100M":
			(data0==PortStatusLinkSpeed1000M?"1G":
				(data0==PortStatusLinkSpeed10M?"10M":"500M")));
#else
		seq_printf(s, "Speed %s\n", data0==PortStatusLinkSpeed100M?"100M":
			(data0==PortStatusLinkSpeed1000M?"1G":
				(data0==PortStatusLinkSpeed10M?"10M":"Unkown")));
#endif

#if defined(CONFIG_RTL_8198C)
		if(port < 5) {
			extern rtl8651_tblAsic_ethernet_t 	rtl8651AsicEthernetTable[];
			int phyid;
			uint32	data1;
			phyid = rtl8651AsicEthernetTable[port].phyId;		
		
			rtl8651_setAsicEthernetPHYReg( phyid, 31, 0xa43);
			rtl8651_getAsicEthernetPHYReg( phyid, 26, &data1);		
			rtl8651_setAsicEthernetPHYReg( phyid, 31, 0);

			seq_printf(s,"	EEE Status: %s.\n\n",(data1&0x100)?"enabled":"disabled");
		}
#else		
		data0 = (regData & (PortEEEStatus_MASK))>>PortEEEStatus_OFFSET;
		seq_printf(s,"	EEE Status %0x\n\n",data0);
#endif

	}

	#if defined(CONFIG_RTL_8198C_8367RB)
	seq_printf(s, "\nDump 8367RB Ports Status:\n");

	for(port=PHY0;port<PHY4;port++)
	{
		extern int rtk_port_phyStatus_get(int port, int *pLinkStatus, int *pSpeed, int *pDuplex);
		int status, speed, duplex;

		if ((rtk_port_phyStatus_get(port, &status, &speed, &duplex)) == 0) {
			seq_printf(s, "Port%d ", port);

			if (status == 0) {
				seq_printf(s, "LinkDown\n\n");
				continue;
			}
			else {
				seq_printf(s, "LinkUp | ");
			}

			seq_printf(s, "	%s-Duplex | ", duplex?"Full":"Half");
			
			seq_printf(s, "Speed %s\n\n", speed==PortStatusLinkSpeed100M?"100M":
				(speed==PortStatusLinkSpeed1000M?"1G":
					(speed==PortStatusLinkSpeed10M?"10M":"Unkown")));			
		}
	}	
	#endif
	
#endif

	return 0;
}
#else
static int32 port_status_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len, port;
	
#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8370_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)
	len = sprintf(page, "Dump Port Status:\n");

	for(port=PHY0;port<EXT_SWITCH_MAX_PHY_PORT;port++)
	{
		extern int  rtk_port_macStatus_get(int port, rtk_port_mac_ability_t *pPortstatus);
		rtk_port_mac_ability_t sts;
		if ((rtk_port_macStatus_get(port, &sts)) == 0) {
			len += sprintf(page+len, "Port%d ", port);

			if (sts.link == 0) {
				len += sprintf(page+len, "LinkDown\n\n");
				continue;
			}
			else {
				len += sprintf(page+len, "LinkUp | ");
			}
			len += sprintf(page+len, "NWay Mode %s\n", (sts.nway) ?"Enabled":"Disabled");
			len += sprintf(page+len, "	RXPause %s | ", (sts.rxpause) ?"Enabled":"Disabled");
			len += sprintf(page+len, "TXPause %s\n", (sts.txpause) ?"Enabled":"Disabled");
			len += sprintf(page+len, "	Duplex %s | ", (sts.duplex) ?"Enabled":"Disabled");
			len += sprintf(page+len, "Speed %s\n\n", (sts.speed) ==PortStatusLinkSpeed100M?"100M":
				((sts.speed) ==PortStatusLinkSpeed1000M?"1G":
					((sts.speed) ==PortStatusLinkSpeed10M?"10M":"Unkown")));
					
		}
	}
#else
	uint32	regData, data0, regData_PCR;

#if defined(CONFIG_RTL_8325D_SUPPORT)
	int32 rtl8325d_portStat_dump(char *page);
	len = rtl8325d_portStat_dump(page);

	len += sprintf(page+len, "\nDump 8197D Port Status:\n");
	for(port=1;port<5;port++)
#else	

	len = sprintf(page, "Dump Port Status:\n");
	for(port=PHY0;port<=CPU;port++)
#endif		
	{
		regData = READ_MEM32(PSRP0+((port)<<2));
		regData_PCR = READ_MEM32(PCRP0+((port)<<2));
		
		if (port==CPU)
			len += sprintf(page+len, "CPUPort ");
		else
			len += sprintf(page+len, "Port%d ", port);
		
		data0 = regData_PCR & EnForceMode;
		if(data0)
		{
			len += sprintf(page+len, "Enforce Mode ");
			data0 = regData_PCR & PollLinkStatus;
			if (data0)
				len += sprintf(page+len, " | polling LinkUp");
			else
			{
				len += sprintf(page+len, " | disable Auto-Negotiation\n\n");
			}
		}
		else
		{
			len += sprintf(page+len, "Force Mode disable\n");
		}
		data0 = (regData & (PortEEEStatus_MASK))>>PortEEEStatus_OFFSET;
		len += sprintf(page+len,"EEE Status %0x\n",data0);
		
		regData = READ_MEM32(PSRP0+((port)<<2));
		data0 = regData & PortStatusLinkUp;

		if (data0)
			len += sprintf(page+len, "LinkUp | ");
		else
		{
			len += sprintf(page+len, "LinkDown\n\n");
			continue;
		}
		data0 = regData & PortStatusNWayEnable;
		len += sprintf(page+len, "NWay Mode %s\n", data0?"Enabled":"Disabled");
		data0 = regData & PortStatusRXPAUSE;
		len += sprintf(page+len, "	RXPause %s | ", data0?"Enabled":"Disabled");
		data0 = regData & PortStatusTXPAUSE;
		len += sprintf(page+len, "TXPause %s\n", data0?"Enabled":"Disabled");
		data0 = regData & PortStatusDuplex;
		len += sprintf(page+len, "	Duplex %s | ", data0?"Enabled":"Disabled");
		data0 = (regData&PortStatusLinkSpeed_MASK)>>PortStatusLinkSpeed_OFFSET;
		len += sprintf(page+len, "Speed %s\n\n", data0==PortStatusLinkSpeed100M?"100M":
			(data0==PortStatusLinkSpeed1000M?"1G":
				(data0==PortStatusLinkSpeed10M?"10M":"Unkown")));
	}
#endif

	return len;
}
#endif

extern int32 rtl8651_setAsicEthernetPHYPowerDown( uint32 port, uint32 pwrDown );
extern int32 rtl8651_setAsicEthernetPHYSpeed( uint32 port, uint32 speed );
extern int32 rtl8651_setAsicEthernetPHYDuplex( uint32 port, uint32 duplex );
extern int32 rtl8651_setAsicEthernetPHYAutoNeg( uint32 port, uint32 autoneg);
extern int32 rtl8651_setAsicEthernetPHYAdvCapality(uint32 port, uint32 capality);
extern int32 rtl865xC_setAsicEthernetForceModeRegs(uint32 port, uint32 enForceMode, uint32 forceLink, uint32 forceSpeed, uint32 forceDuplex);
extern int32 rtl8651_restartAsicEthernetPHYNway(uint32 port);

static int32 port_status_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char		tmpbuf[64];
	uint32	port_mask;
	char		*strptr, *cmd_addr;
	char		*tokptr;
	int 	type;
	int 		port;
#if !defined(CONFIG_RTL_8367R_SUPPORT) && !defined(CONFIG_RTL_8370_SUPPORT) && !defined(CONFIG_RTL_83XX_SUPPORT)
	int forceMode = 0;
	int forceLink = 0;
	int forceLinkSpeed = 0;
	int forceDuplex = 0;
	uint32 advCapability = 0;
#endif

#define SPEED10M 	0
#define SPEED100M 	1
#define SPEED1000M 	2

	if(len>64)
	{
		goto errout;
	}
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len-1] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "port", 4))
		{
			port_mask=simple_strtol(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}

			if(strcmp(tokptr,"10_half") == 0)
				type = HALF_DUPLEX_10M;
			else if(strcmp(tokptr,"100_half") == 0)
				type = HALF_DUPLEX_100M;
			else if(strcmp(tokptr,"1000_half") == 0)
				type = HALF_DUPLEX_1000M;
			else if(strcmp(tokptr,"10_full") == 0)
				type = DUPLEX_10M;
			else if(strcmp(tokptr,"100_full") == 0)
				type = DUPLEX_100M;
			else if(strcmp(tokptr,"1000_full") == 0)
				type = DUPLEX_1000M;
			else if(strcmp(tokptr,"down") == 0)
				type = PORT_DOWN;
			else if(strcmp(tokptr,"up") == 0)
				type = PORT_UP;
			else if(strcmp(tokptr,"an_10m") == 0)
				type = AN_10M;
			else if(strcmp(tokptr,"an_100m") == 0)
				type = AN_100M;
			else
				type = PORT_AUTO;

#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8370_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)
			for(port = 0; port < EXT_SWITCH_MAX_PHY_PORT; port++)
			{
				extern int set_83XX_speed_mode(int port, int type);
				if (((1<<port) & port_mask) && (type != HALF_DUPLEX_1000M))
				{
					set_83XX_speed_mode(port, type);
				}
			}
#else
			/* keep CONFIG_FORCE_10M_100M_FULL_REFINE for backward compatible */
			/* design note:
  			1. When user force port to 100MF, we still set to AN mode but disable 1000MF ability.
  			2. When user force port to 10MF, we still set to AN mode but disable 100MF/100MH/1000MF ability.
  			3. When user force port to 10MF/100MF, we process it as old way.
 			 */
			#ifdef CONFIG_FORCE_10M_100M_FULL_REFINE
			if (type == DUPLEX_10M)
				type = AN_10M;
			else if (type == DUPLEX_100M)
				type = AN_100M;
			else if (type == DUPLEX_1000M)
				type = AN_AUTO;
			else if (type == PORT_AUTO)
				type = AN_AUTO;
			#endif
			
			switch(type)
			{
				case HALF_DUPLEX_10M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED10M;
					forceDuplex=FALSE;
					advCapability=(1<<HALF_DUPLEX_10M);
					break;
				}
				case HALF_DUPLEX_100M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED100M;
					forceDuplex=FALSE;
					advCapability=(1<<HALF_DUPLEX_100M);
					break;
				}
				case HALF_DUPLEX_1000M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED1000M;
					forceDuplex=FALSE;
					advCapability=(1<<HALF_DUPLEX_1000M);
					break;
				}
				case DUPLEX_10M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED10M;
					forceDuplex=TRUE;
					advCapability=(1<<DUPLEX_10M);
					break;
				}
				case DUPLEX_100M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED100M;
					forceDuplex=TRUE;
					advCapability=(1<<DUPLEX_100M);
					break;
				}
				case DUPLEX_1000M:
				{
					forceMode=TRUE;
					forceLink=TRUE;
					forceLinkSpeed=SPEED1000M;
					forceDuplex=TRUE;
					advCapability=(1<<DUPLEX_1000M);
					break;
				}	
				case AN_10M:
				{
					forceMode=FALSE;
					forceLinkSpeed=NwayAbility10MF | NwayAbility10MH;
					advCapability=(1<<HALF_DUPLEX_10M) | (1<<DUPLEX_10M);
					break;
				}	
				case AN_100M:
				{
					forceMode=FALSE;
					forceLinkSpeed=NwayAbility10MF | NwayAbility10MH | NwayAbility100MF | NwayAbility100MH;
					advCapability=(1<<PORT_AUTO);
					break;
				}	
				case AN_AUTO:
				{
					forceMode=FALSE;
					forceLinkSpeed=NwayAbility10MF | NwayAbility10MH | NwayAbility100MF | NwayAbility100MH |NwayAbility1000MF;
					advCapability=(1<<PORT_AUTO);
					break;
				}
				default:
				{
					forceMode=FALSE;
					forceLink=TRUE;
					/*all capality*/
					advCapability=(1<<PORT_AUTO);
				}
			}

#ifdef CONFIG_RTL_8198C
			if ((type == HALF_DUPLEX_1000M) || (type == DUPLEX_1000M)) {
				forceMode=FALSE;
				forceLink=TRUE;
				forceLinkSpeed=SPEED1000M;
				/*all capality*/
				advCapability=(1<<DUPLEX_1000M);
			}
#endif

			#if defined(CONFIG_RTL_FE_AUTO_DOWN_SPEED)
			local_bh_disable();
			for(port = 0; port < CPU; port++)
			{
				if ((1<<port) & port_mask) {
					fe_ads[port].force_speed_by_nway = 0;
				}
			}
			local_bh_enable();
			#endif

			if ((type == AN_10M) || (type == AN_100M) || (type == AN_AUTO)) {
				for(port = 0; port < CPU; port++)
				{
					if ((1<<port) & port_mask) {
						rtl8651_setAsicEthernetPHYAutoNeg(port,forceMode?FALSE:TRUE);
						rtl8651_setAsicEthernetPHYAdvCapality(port,advCapability);
						REG32(PCRP0 + (port * 4)) = (REG32(PCRP0 + (port * 4)) & ~(EnForceMode | PollLinkStatus | AutoNegoSts_MASK)) | forceLinkSpeed;

						#if defined(CONFIG_RTL_LONG_ETH_CABLE_REFINE) ||defined(CONFIG_RTL_FE_AUTO_DOWN_SPEED)
						if (type == AN_10M) {
							local_bh_disable();
							fe_ads[port].down_speed_renway = 1;
							fe_ads[port].force_speed_by_nway = 1;
							local_bh_enable();
						}
						#endif
					
						rtl8651_restartAsicEthernetPHYNway(port);
						mdelay(10);
					}
				}				
			}
			else
			for(port = 0; port < CPU; port++)
			{
				#if defined(CONFIG_RTL_8198C_8367RB)
				if (port >= 5)
					continue;
				#endif
				
				if((1<<port) & port_mask)
				{
					if (type == PORT_DOWN) {
						rtl8651_setAsicEthernetPHYPowerDown(port, 1);
					}
					else if (type == PORT_UP) {
						rtl8651_setAsicEthernetPHYPowerDown(port, 0);
					}
					else {
						/*Set PHY Register*/
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT) && defined(CONFIG_RTL_EXCHANGE_PORTMASK)
						if((type == PORT_AUTO) && (port == 0) && gpio_simulate_mdc_mdio){
							forceLinkSpeed = SPEED1000M;
						}
#endif						
						rtl8651_setAsicEthernetPHYSpeed(port,forceLinkSpeed);
						rtl8651_setAsicEthernetPHYDuplex(port,forceDuplex);
						rtl8651_setAsicEthernetPHYAutoNeg(port,forceMode?FALSE:TRUE);
						rtl8651_setAsicEthernetPHYAdvCapality(port,advCapability);

						rtl865xC_setAsicEthernetForceModeRegs(port, forceMode, forceLink, forceLinkSpeed, forceDuplex);
							
						rtl8651_restartAsicEthernetPHYNway(port);
						mdelay(10);
					}
				}
			}
			
			#if defined(CONFIG_RTL_8198C_8367RB)
			for(port = 5; port < 9; port++)
			{
				extern int set_83XX_speed_mode(int port, int type);
				if (((1<<port) & port_mask) && (type != HALF_DUPLEX_1000M))
				{
					set_83XX_speed_mode(port-5, type);
				}
			}
			#endif
#endif

		}
		else if (!memcmp(cmd_addr, "dump", 4))
		{
#if defined(CONFIG_RTL_FE_AUTO_DOWN_SPEED)
			if (!memcmp(tokptr, "ads", 3)) {
				extern void ads_debug(void);
				ads_debug();
			}
#endif

#if defined(CONFIG_RTL_FORCE_MDIX)
			if (!memcmp(tokptr, "fmx", 3)) {
				extern void fmx_debug(void);
				fmx_debug();
			}
#endif
		}
#ifdef CONFIG_RTL_FORCE_MDIX
		else if (!memcmp(cmd_addr, "setfmx", 6))
		{
			extern int link_up_down_interval;
			extern int thres_chg_force_mdix;
			extern int timeout_FORCEMDIX;
			
			if (tokptr==NULL)
			{
				goto errout;
			}
			link_up_down_interval=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			thres_chg_force_mdix=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			timeout_FORCEMDIX=simple_strtol(tokptr, NULL, 0);

			rtlglue_printf("link_up_down_interval= %d, thres_chg_force_mdix= %d, timeout_FORCEMDIX= %d\n",
				link_up_down_interval, thres_chg_force_mdix, timeout_FORCEMDIX);
		}
		else if (!memcmp(cmd_addr, "fmxmode", 7))
		{
			if (tokptr==NULL)
			{
				goto errout;
			}
			port_mask=simple_strtol(tokptr, NULL, 0);

			for (port = ADS_PORT_START; port < RTL8651_PHY_NUMBER; port++) {
				if (port_mask & BIT(port))
					fe_fmx[port].mdimode = 1;
				else 
					fe_fmx[port].mdimode = 0;
			}

		}
#endif		
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		rtlglue_printf("port status only support \"port\" as the first parameter\n");
		rtlglue_printf("format: \"port port_mask 10_half/100_half/10_full/100_full/1000_full/auto\"\n");
	}
	return len;
}



#ifdef CONFIG_RTL_PROC_NEW
static int32 rtl865x_proc_mibCounter_read(struct seq_file *s, void *v)
{
		uint32 i;
					extern uint32 rtl8651_returnAsicCounter(uint32 offset);
				extern uint64 rtl865xC_returnAsicCounter64(uint32 offset);
		for ( i = 0; i <= RTL8651_PORT_NUMBER; i++ )
		{
			uint32 addrOffset_fromP0 = i * MIB_ADDROFFSETBYPORT;
			
#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) || defined(CONFIG_RTL_8197F)
			if (i == 5) continue;
#endif
	
			if ( i == RTL8651_PORT_NUMBER )
				seq_printf(s,"<CPU port (extension port included)>\n");
			else
				seq_printf(s,"<Port: %d>\n", i);
	
			seq_printf(s,"Rx counters\n");
			seq_printf(s,"	 Rcv %llu bytes, TpPortInDiscard %u, DropEvent %u", 
				rtl865xC_returnAsicCounter64( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter(OFFSET_ETHERSTATSDROPEVENTS_P0 + addrOffset_fromP0 ));
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) || \
	defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
			seq_printf(s,", QmDiscard  %u", 
				rtl8651_returnAsicCounter( OFFSET_QMDISCARDCNT_P0 + addrOffset_fromP0 ));
#endif			
			seq_printf(s,"\n	 CRCAlignErr %u, SymbolErr %u, FragErr %u, JabberErr %u\n",
				rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_DOT3STATSSYMBOLERRORS_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_ETHERSTATSFRAGMEMTS_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ));
			seq_printf(s,"	 Unicast %u pkts, Multicast %u pkts, Broadcast %u pkts\n", 
				rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ));
			seq_printf(s,"	 < 64: %u pkts, 64: %u pkts, 65 -127: %u pkts, 128 -255: %u pkts\n", 
				rtl8651_returnAsicCounter( OFFSET_ETHERSTATSUNDERSIZEPKTS_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS64OCTETS_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS65TO127OCTETS_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS128TO255OCTETS_P0 + addrOffset_fromP0 ));
			seq_printf(s,"	 256 - 511: %u pkts, 512 - 1023: %u pkts, 1024 - 1518: %u pkts\n", 
				rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS256TO511OCTETS_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS512TO1023OCTETS_P0 + addrOffset_fromP0), 
				rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS1024TO1518OCTETS_P0 + addrOffset_fromP0 ) );
			seq_printf(s,"	 oversize: %u pkts, Control unknown %u pkts, Pause %u pkts\n", 
				rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0 ),
				rtl8651_returnAsicCounter( OFFSET_DOT3CONTROLINUNKNOWNOPCODES_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_DOT3INPAUSEFRAMES_P0 + addrOffset_fromP0 ));
			
			seq_printf(s,"Output counters\n");
			seq_printf(s,"	 Snd %llu bytes, Unicast %u pkts, Multicast %u pkts\n",
				rtl865xC_returnAsicCounter64( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ),
				rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ));
			seq_printf(s,"	 Broadcast %u pkts, Late collision %u, Deferred transmission %u\n",
				rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 ),
				rtl8651_returnAsicCounter( OFFSET_DOT3STATSLATECOLLISIONS_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0 ));
			seq_printf(s,"	 Collisions %u, Single collision %u, Multiple collision %u, pause %u\n",
				rtl8651_returnAsicCounter( OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_DOT3STATSSINGLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_DOT3STATSMULTIPLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ), 
				rtl8651_returnAsicCounter( OFFSET_DOT3OUTPAUSEFRAMES_P0 + addrOffset_fromP0 ));
			
		}
	
		seq_printf(s,"<Whole system counters>\n");
		seq_printf(s,"	 CpuEvent %u pkts\n", rtl8651_returnAsicCounter(OFFSET_ETHERSTATSCPUEVENTPKT));
	
		//return SUCCESS;

	//rtl865xC_dumpAsicDiagCounter(s);
	return 0;
}
#else
static int32 rtl865x_proc_mibCounter_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len=0;

	rtl865xC_dumpAsicDiagCounter(page, &len);


	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}
#endif

#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8198C_8367RB) || defined(CONFIG_RTL_83XX_SUPPORT)
extern uint32 r8367_cpu_port;
extern rtk_stat_port_cntr_t  port_cntrs;

void display_8367r_port_stat(uint32 port, rtk_stat_port_cntr_t *pPort_cntrs)
{
	if ( port > 4 )
		rtlglue_printf("\n<CPU port>\n");
	else
		rtlglue_printf("\n<Port: %d>\n", port);

	rtlglue_printf("Rx counters\n");

	rtlglue_printf("   ifInOctets  %llu\n", pPort_cntrs->ifInOctets);
	rtlglue_printf("   etherStatsOctets  %llu\n", pPort_cntrs->etherStatsOctets);
	rtlglue_printf("   ifInUcastPkts  %u\n", pPort_cntrs->ifInUcastPkts);
	rtlglue_printf("   etherStatsMcastPkts  %u\n", pPort_cntrs->etherStatsMcastPkts);
	rtlglue_printf("   etherStatsBcastPkts  %u\n", pPort_cntrs->etherStatsBcastPkts);
	
	rtlglue_printf("   StatsFCSErrors  %u\n", pPort_cntrs->dot3StatsFCSErrors);
	rtlglue_printf("   StatsSymbolErrors  %u\n", pPort_cntrs->dot3StatsSymbolErrors);
	rtlglue_printf("   InPauseFrames  %u\n", pPort_cntrs->dot3InPauseFrames);
	rtlglue_printf("   ControlInUnknownOpcodes  %u\n", pPort_cntrs->dot3ControlInUnknownOpcodes);
	rtlglue_printf("   etherStatsFragments  %u\n", pPort_cntrs->etherStatsFragments);
	rtlglue_printf("   etherStatsJabbers  %u\n", pPort_cntrs->etherStatsJabbers);
	rtlglue_printf("   etherStatsDropEvents  %u\n", pPort_cntrs->etherStatsDropEvents);

	rtlglue_printf("   etherStatsUndersizePkts  %u\n", pPort_cntrs->etherStatsUndersizePkts);
	rtlglue_printf("   etherStatsOversizePkts  %u\n", pPort_cntrs->etherStatsOversizePkts);

	rtlglue_printf("   dot1dTpPortInDiscards  %u\n", pPort_cntrs->dot1dTpPortInDiscards);
	rtlglue_printf("   inOampduPkts  %u\n", pPort_cntrs->inOampduPkts);
	
	rtlglue_printf("   Len= 64: %u pkts, 65 - 127: %u pkts, 128 - 255: %u pkts\n",
		pPort_cntrs->etherStatsPkts64Octets,
		pPort_cntrs->etherStatsPkts65to127Octets,
		pPort_cntrs->etherStatsPkts128to255Octets);
	rtlglue_printf("       256 - 511: %u pkts, 512 - 1023: %u pkts, 1024 - 1518: %u pkts\n",
		pPort_cntrs->etherStatsPkts256to511Octets,
		pPort_cntrs->etherStatsPkts512to1023Octets,
		pPort_cntrs->etherStatsPkts1024toMaxOctets);

	rtlglue_printf("\nOutput counters\n");

	rtlglue_printf("   ifOutOctets  %llu\n", pPort_cntrs->ifOutOctets);
	rtlglue_printf("   ifOutUcastPkts  %u\n", pPort_cntrs->ifOutUcastPkts);
	rtlglue_printf("   ifOutMulticastPkts  %u\n", pPort_cntrs->ifOutMulticastPkts);
	rtlglue_printf("   ifOutBrocastPkts  %u\n", pPort_cntrs->ifOutBrocastPkts);
#ifdef CONFIG_RTL_83XX_SUPPORT
	rtlglue_printf("   ifOutDiscards  %u\n", pPort_cntrs->ifOutDiscards);
#endif
	rtlglue_printf("   StatsSingleCollisionFrames  %u\n", pPort_cntrs->dot3StatsSingleCollisionFrames);
	rtlglue_printf("   StatsMultipleCollisionFrames  %u\n", pPort_cntrs->dot3StatsMultipleCollisionFrames);
	rtlglue_printf("   StatsDeferredTransmissions  %u\n", pPort_cntrs->dot3StatsDeferredTransmissions);
	rtlglue_printf("   StatsLateCollisions  %u\n", pPort_cntrs->dot3StatsLateCollisions);
	rtlglue_printf("   etherStatsCollisions  %u\n", pPort_cntrs->etherStatsCollisions);
	rtlglue_printf("   StatsExcessiveCollisions  %u\n", pPort_cntrs->dot3StatsExcessiveCollisions);
	rtlglue_printf("   OutPauseFrames  %u\n", pPort_cntrs->dot3OutPauseFrames);
	rtlglue_printf("   dot1dBasePortDelayExceededDiscards  %u\n", pPort_cntrs->dot1dBasePortDelayExceededDiscards);

	rtlglue_printf("   outOampduPkts  %u\n", pPort_cntrs->outOampduPkts);
	rtlglue_printf("   pktgenPkts  %u\n", pPort_cntrs->pktgenPkts);

}
#endif

#ifdef CONFIG_RTL_8370_SUPPORT
extern rtk_stat_port_cntr_t  port_cntrs;

void display_8370_port_stat(uint32 port, rtk_stat_port_cntr_t *p)
{
	if ( port == 9 )
		rtlglue_printf("\n<CPU port>\n");
	else
		rtlglue_printf("\n<Port: %d>\n", port);

	rtlglue_printf("Rx counters:\n");

	rtlglue_printf("   Rcv %llu bytes, Unicast %u pkts, Multicast %u pkts, Broadcast %u pkts\n", 
		p->ifInOctets, p->ifInUcastPkts, p->etherStatsMcastPkts, p->etherStatsBcastPkts);
	rtlglue_printf("   64: %u pkts, 65 -127: %u pkts, 128 -255: %u pkts\n", 
		p->etherStatsPkts64Octets, p->etherStatsPkts65to127Octets, p->etherStatsPkts128to255Octets);
	rtlglue_printf("   256 - 511: %u pkts, 512 - 1023: %u pkts, 1024 - max: %u pkts\n", 
		p->etherStatsPkts256to511Octets, p->etherStatsPkts512to1023Octets, p->etherStatsPkts1024toMaxOctets);
	rtlglue_printf("   Drop: %u, FCSError: %u, Fragment: %u, JabberErr: %u\n", 
		p->etherStatsDropEvents, p->dot3StatsFCSErrors, p->etherStatsFragments, p->etherStatsJabbers);
	rtlglue_printf("   OverSize: %u, UnderSize: %u, Unknown: %u, inOampdu: %u\n", 
		p->etherStatsOversizePkts, p->etherStatsUndersizePkts, 
		p->dot3ControlInUnknownOpcodes, p->inOampduPkts);
	rtlglue_printf("   PortInDiscard: %u, SymbolErr: %u, Pause: %u\n", 
		p->dot1dTpPortInDiscards, p->dot3StatsSymbolErrors, p->dot3InPauseFrames);
	rtlglue_printf("   etherStatsOctets  %llu\n", p->etherStatsOctets);


	rtlglue_printf("\nOutput counters:\n");

	rtlglue_printf("   Snd %llu bytes, Unicast %u pkts, Multicast %u pkts, Broadcast %u pkts\n", 
		p->ifOutOctets, p->ifOutUcastPkts, p->ifOutMulticastPkts, p->ifOutBrocastPkts);
	rtlglue_printf("   Collision: %u, SingleCol: %u, MultipleCol: %u, LateCol: %u\n", 
		p->etherStatsCollisions, p->dot3StatsSingleCollisionFrames, 
		p->dot3StatsMultipleCollisionFrames, p->dot3StatsLateCollisions);
	rtlglue_printf("   ExcessiveCol: %u, DeferredTx: %u, DelayExceededDiscard: %u\n", 
		p->dot3StatsExcessiveCollisions, p->dot3StatsDeferredTransmissions, 
		p->dot1dBasePortDelayExceededDiscards);
	rtlglue_printf("   outOampdu: %u, pktgen: %u, Pause: %u\n", 
		p->outOampduPkts, p->pktgenPkts, p->dot3OutPauseFrames);
}
#endif

static int32 rtl865x_proc_mibCounter_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[512];
	char		*strptr;
	char		*cmdptr;
	uint32	portNum=0xFFFFFFFF;

	if(len>512)
	{
		goto errout;
	}
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';

		strptr=tmpbuf;

		if(strlen(strptr)==0)
		{
			goto errout;
		}

		cmdptr = strsep(&strptr," ");
		if (cmdptr==NULL)
		{
			goto errout;
		}

		/*parse command*/
		if(strncmp(cmdptr, "clear",5) == 0)
		{
			rtl8651_clearAsicCounter();

#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8370_SUPPORT) || defined(CONFIG_RTL_8325D_SUPPORT) || defined(CONFIG_RTL_8198C_8367RB) || defined(CONFIG_RTL_83XX_SUPPORT)
			rtk_stat_global_reset();
#endif
		}
		else if(strncmp(cmdptr, "dump",4) == 0)
		{
			cmdptr = strsep(&strptr," ");
			if (cmdptr==NULL)
			{
				goto errout;
			}

#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8198C_8367RB) || defined(CONFIG_RTL_83XX_SUPPORT)
			if(strncmp(cmdptr, "8367",4) == 0 || strncmp(cmdptr, "83xx",4) == 0) {

				for (portNum=0; portNum<19; portNum++) {

					if ((portNum > 4) && (portNum != r8367_cpu_port))	// skip port 6 in 8367R; skip port 5 in 8367RB
						continue;

					memset(&port_cntrs, 0, sizeof(rtk_stat_port_cntr_t));
					rtk_stat_port_getAll(portNum, &port_cntrs);
					display_8367r_port_stat(portNum, &port_cntrs);
				}
				return len;
			}
#endif
#ifdef CONFIG_RTL_8370_SUPPORT
			if(strncmp(cmdptr, "8370",4) == 0) {	// 8370 or 83700 ~ 83709
			
				if ((cmdptr[4] >= '0') && (cmdptr[4] <= '9')) {
					memset(&port_cntrs, 0, sizeof(rtk_stat_port_cntr_t));
					rtk_stat_port_getAll((cmdptr[4] - '0'), &port_cntrs);
					display_8370_port_stat((cmdptr[4] - '0'), &port_cntrs);
				}
				else {
					for (portNum=0; portNum<=9; portNum++) {
						memset(&port_cntrs, 0, sizeof(rtk_stat_port_cntr_t));
						rtk_stat_port_getAll(portNum, &port_cntrs);
						display_8370_port_stat(portNum, &port_cntrs);
					}
				}
				return len;
			}
#endif
#ifdef CONFIG_RTL_8325D_SUPPORT
			if(strncmp(cmdptr, "8325",4) == 0) {	// 8325 or 83250 ~ 8325g
				extern int32 rtl8325d_mibCounter_dump(int port);

				if ((cmdptr[4] >= '0') && (cmdptr[4] <= '9')) {
					rtl8325d_mibCounter_dump((cmdptr[4] - '0'));
				}
				else if ((cmdptr[4] >= 'a') && (cmdptr[4] <= 'f')) {
					rtl8325d_mibCounter_dump((cmdptr[4] - 'a' + 10));
				}
				else if ((cmdptr[4] == 'g')) {
					rtl8325d_mibCounter_dump(24);
				}
				else {
					for (portNum=0; portNum<16; portNum++) {
						rtl8325d_mibCounter_dump(portNum);
					}
					rtl8325d_mibCounter_dump(24);
				}
				return len;
			}
#endif
			if(strncmp(cmdptr, "port",4) != 0)
			{
				goto errout;
			}

			cmdptr = strsep(&strptr," ");
			if (cmdptr==NULL)
			{
				goto errout;
			}
			portNum = simple_strtol(cmdptr, NULL, 0);


			if((portNum>=0) && (portNum<=RTL8651_PORT_NUMBER))
			{
				extern uint32 rtl8651_returnAsicCounter(uint32 offset);
				extern uint64 rtl865xC_returnAsicCounter64(uint32 offset);
				uint32 addrOffset_fromP0 = portNum * MIB_ADDROFFSETBYPORT;

				if ( portNum == RTL8651_PORT_NUMBER )
					rtlglue_printf("<CPU port (extension port included)>\n");
				else
					rtlglue_printf("<Port: %d>\n", portNum);

				rtlglue_printf("Rx counters\n");
				rtlglue_printf("   Rcv %llu bytes, TpPortInDiscard %u, DropEvent %u", 
					rtl865xC_returnAsicCounter64( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter(OFFSET_ETHERSTATSDROPEVENTS_P0 + addrOffset_fromP0 ));
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) || \
	defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
				rtlglue_printf(", QmDiscard  %u", 
					rtl8651_returnAsicCounter( OFFSET_QMDISCARDCNT_P0 + addrOffset_fromP0 ));
#endif			
				rtlglue_printf("\n   CRCAlignErr %u, SymbolErr %u, FragErr %u, JabberErr %u\n",
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSSYMBOLERRORS_P0 + addrOffset_fromP0 ), 
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSFRAGMEMTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ));
				rtlglue_printf("   Unicast %u pkts, Multicast %u pkts, Broadcast %u pkts\n",
					rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ));
				rtlglue_printf("   < 64: %u pkts, 64: %u pkts, 65 -127: %u pkts, 128 -255: %u pkts\n",
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSUNDERSIZEPKTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS64OCTETS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS65TO127OCTETS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS128TO255OCTETS_P0 + addrOffset_fromP0 ));
				rtlglue_printf("   256 - 511: %u pkts, 512 - 1023: %u pkts, 1024 - 1518: %u pkts\n",
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS256TO511OCTETS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS512TO1023OCTETS_P0 + addrOffset_fromP0),
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS1024TO1518OCTETS_P0 + addrOffset_fromP0 ) );
				rtlglue_printf("   oversize: %u pkts, Control unknown %u pkts, Pause %u pkts\n",
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3CONTROLINUNKNOWNOPCODES_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3INPAUSEFRAMES_P0 + addrOffset_fromP0 ));

				rtlglue_printf("Output counters\n");
				rtlglue_printf("   Snd %llu bytes, Unicast %u pkts, Multicast %u pkts\n",
					rtl865xC_returnAsicCounter64( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ));
				rtlglue_printf("   Broadcast %u pkts, Late collision %u, Deferred transmission %u\n",
					rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSLATECOLLISIONS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0 ));
				rtlglue_printf("   Collisions %u, Single collision %u, Multiple collision %u, pause %u\n",
					rtl8651_returnAsicCounter( OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSSINGLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3STATSMULTIPLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ),
					rtl8651_returnAsicCounter( OFFSET_DOT3OUTPAUSEFRAMES_P0 + addrOffset_fromP0 ));

			}
			else
			{
				goto errout;
			}
		}
		else
		{
			goto errout;
		}

	}
	else
	{
errout:
		rtlglue_printf("error input\n");
	}

	return len;
}

#if defined (CONFIG_RTL_INBAND_CTL_API)
extern int rtl_get_portRate(unsigned int port,unsigned long* rxRate, unsigned long *txRate);

#ifdef CONFIG_RTL_PROC_NEW
static int32 rtl865x_proc_portRate_read(struct seq_file *s, void *v)
{
	
	unsigned int port;
	unsigned long rxRate=0;
	unsigned long txRate=0;
	seq_printf(s, "Port Rate Info:\n");
	for(port=0;port<4;port++)
	{
		rtl_get_portRate(port,&rxRate,&txRate);
		seq_printf(s, "port%d rx:%ld tx:%ld(kbps)\n",port,rxRate,txRate);
	}
	
	
	return 0;

}
#else
static int32 rtl865x_proc_portRate_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	
	int port;
	unsigned long rxRate=0;
	unsigned long txRate=0;
	len = sprintf(page, "Port Rate Info:\n");
	for(port=0;port<4;port++)
	{
		rtl_get_portRate(port,&rxRate,&txRate);
		len += sprintf(page+len, "port%d rx:%ld tx:%ld(kbps)\n",port,rxRate,txRate);
	}
	
	
	return len;

}
#endif

static int32 rtl865x_proc_portRate_write(struct file *filp, const char *buff,unsigned long len, void *data )
{
		return len;
}

#endif


#ifdef CONFIG_RTL_PROC_NEW
static int32 proc_phyReg_read(struct seq_file *s, void *v)
{
	return PROC_READ_RETURN_VALUE;
}
#else
static int32 proc_phyReg_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	return PROC_READ_RETURN_VALUE;
}
#endif

#ifdef CONFIG_RTL_PROC_NEW
static int32 proc_mmd_read(struct seq_file *s, void *v)
{
	return PROC_READ_RETURN_VALUE;
}
#else
static int32 proc_mmd_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	return PROC_READ_RETURN_VALUE;
}
#endif

extern int32 mmd_read(uint32 phyId, uint32 devId, uint32 regId, uint32 *rData);
extern int32 mmd_write(uint32 phyId, uint32 devId, uint32 regId, uint32 wData);

/*
	echo read phy_id device_id reg_addr  > /proc/rtl865x/mmd
	echo write phy_id device_id reg_addr data_for_write > /proc/rtl865x/mmd
 */
static int32 proc_mmd_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 	tmpbuf[64];
	uint32	phyId, regId, regData, devId, dataReadBack;
	char		*strptr, *cmd_addr;
	char		*tokptr;
	int32 	ret;

	if(len>64)
	{
		goto errout;
	}
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
	
		if (!memcmp(cmd_addr, "read", 4))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			devId=simple_strtol(tokptr, NULL, 0);
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			ret = mmd_read(phyId, devId, regId, &regData);
			
			if(ret==SUCCESS)
			{
				rtlglue_printf("read phyId(%d), devId(%d), regId(%d), regData:0x%x\n", phyId,devId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}
		}
		else if (!memcmp(cmd_addr, "write", 5))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			devId=simple_strtol(tokptr, NULL, 0);
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 0);

			mmd_write(phyId, devId, regId, regData);

			/* confirm result */
			rtl8651_setAsicEthernetPHYReg( phyId, 13, (devId | 0x4000));
			ret=rtl8651_getAsicEthernetPHYReg(phyId, 14, &dataReadBack);

			if(ret==SUCCESS)
			{
				rtlglue_printf("extWrite phyId(%d), devId(%d), regId(%d), regData:0x%x, regData(read back):0x%x\n", 
					phyId, devId, regId, regData, dataReadBack);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}
		}
	}
	else
	{
errout:
		rtlglue_printf("error input!\n");
	}

	return len;
}

static const int _8198_phy_page[] = {	0, 1, 2, 3, 4, 5, 6, 32,
								  	33, 34, 35, 36,	40, 44, 45, 46,
								  	64, 65, 66, 69,	70, 80, 81, 161 };

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
void getPhyByPortPage(int port, int page)
{
	uint32	regData;
	int reg;

	//change page num
	if (page>=31)
	{
		rtl8651_setAsicEthernetPHYReg( port, 31, 7  );
		rtl8651_setAsicEthernetPHYReg( port, 30, page  );
	}
	else if (page>0)
	{
		rtl8651_setAsicEthernetPHYReg( port, 31, page  );
	}

	for(reg=0;reg<32;reg++)
	{
		rtl8651_getAsicEthernetPHYReg( port, reg, &regData);
		rtlglue_printf("port:%d,page:%d,regId:%d,regData:0x%x\n",port,page,reg,regData);
	}
	//if(page!=3)
	//{
		rtlglue_printf("------------------------------------------\n");
	//}

	//change back to page 0
	rtl8651_setAsicEthernetPHYReg(port, 31, 0 );

}

#elif defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
void getPhyByPortPage(int port, int page)
{
	uint32	regData, phyid;
	int reg;

	if (port == 0)
		phyid = 8;
	else 
		phyid = port;

	rtl8651_setAsicEthernetPHYReg( phyid, 31, page);

	for(reg=0;reg<32;reg++)
	{
		rtl8651_getAsicEthernetPHYReg( phyid, reg, &regData);
		rtlglue_printf("port:%d,page:0x%x,regId:%d,regData:0x%x\n",port,page,reg,regData);
	}
	rtlglue_printf("------------------------------------------\n");

	rtl8651_setAsicEthernetPHYReg(phyid, 31, 0 );
}
#endif


static int32 proc_phyReg_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	char 		tmpbuf[64];
	uint32	phyId, pageId,regId, regData;
	char		*strptr, *cmd_addr;
	char		*tokptr;
	int32 	ret=0;
	int 		i, j;

	if(len>64)
	{
		goto errout;
	}
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "read", 4))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
			if (gpio_simulate_mdc_mdio){
				if ((8 > phyId) && (phyId > 4))
					ret = rtl_mdio_read(phyId, regId, &regData);
				else
					ret=rtl8651_getAsicEthernetPHYReg(phyId, regId, &regData);
			}else
#endif
			{
				ret=rtl8651_getAsicEthernetPHYReg(phyId, regId, &regData);
			}

			if(ret==SUCCESS)
			{
				rtlglue_printf("read phyId(%d), regId(%d),regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}
		}
		else if (!memcmp(cmd_addr, "write", 5))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 0);
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
			if (gpio_simulate_mdc_mdio){
				if ((8 > phyId) && (phyId > 4))
					ret = rtl_mdio_write(phyId, regId,regData);
				else
					ret=rtl8651_setAsicEthernetPHYReg(phyId, regId, regData);
			}else
#endif
			{
				ret=rtl8651_setAsicEthernetPHYReg(phyId, regId, regData);
			}
			if(ret==SUCCESS)
			{
				rtlglue_printf("Write phyId(%d), regId(%d), regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}
		}
#ifdef CONFIG_8198_PORT5_RGMII
		else if (!memcmp(cmd_addr, "8370read", 8))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 16);
			ret = rtl8370_getAsicReg(regId, &regData);

			if(ret==0)
			{
				rtlglue_printf("rtl8370_getAsicReg: reg= %x, data= %x\n", regId, regData);
			}
			else
			{
				rtlglue_printf("get fail %d\n", ret);
			}
		}
		else if (!memcmp(cmd_addr, "8370write", 9))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 16);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 16);

			ret = rtl8370_setAsicReg(regId, regData);

			if(ret==0)
			{
				rtlglue_printf("rtl8370_setAsicReg: reg= %x, data= %x\n", regId, regData);
			}
			else
			{
				rtlglue_printf("set fail %d\n", ret);
			}
		}
#endif
#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8198C_8367RB) || defined(CONFIG_RTL_83XX_SUPPORT)
		else if (!memcmp(cmd_addr, "8367read", 8) || !memcmp(cmd_addr, "83xxread", 8))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 16);
			ret = rtl83xx_getAsicReg(regId, &regData);
			if(ret==0)
			{
				rtlglue_printf("rtl83xx_getAsicReg: reg= %x, data= %x\n", regId, regData);
			}
			else
			{
				rtlglue_printf("get fail %d\n", ret);
			}
		}
		else if (!memcmp(cmd_addr, "8367write", 9) || !memcmp(cmd_addr, "83xxwrite", 9))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 16);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 16);

			ret = rtl83xx_setAsicReg(regId, regData);
			if(ret==0)
			{
				rtlglue_printf("rtl83xx_setAsicReg: reg= %x, data= %x\n", regId, regData);
			}
			else
			{
				rtlglue_printf("set fail %d\n", ret);
			}
		}
		else if (!memcmp(cmd_addr, "8367phyr", 8) || !memcmp(cmd_addr, "83xxphyr", 8))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			ret=rtl83xx_getAsicPHYReg(phyId, regId, &regData);
			if(ret==SUCCESS)
			{
				rtlglue_printf("read 83XX phyId(%d), regId(%d), regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}		
		}
		else if (!memcmp(cmd_addr, "8367phyw", 8) || !memcmp(cmd_addr, "83xxphyw", 8))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 16);

			ret=rtl83xx_setAsicPHYReg(phyId, regId, regData);
			if(ret==SUCCESS)
			{
				rtlglue_printf("Write 83XX phyId(%d), regId(%d), regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}
		}
		else if (!memcmp(cmd_addr, "8367ocpr", 8) || !memcmp(cmd_addr, "83xxocpr", 8))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			ret=rtl83xx_getAsicPHYOCPReg(phyId, regId, &regData);
			if(ret==SUCCESS)
			{
				rtlglue_printf("read 83XX phyId(%d), ocpAddr(0x%x), regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}		
		}
		else if (!memcmp(cmd_addr, "8367ocpw", 8) || !memcmp(cmd_addr, "83xxocpw", 8))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 16);

			ret=rtl83xx_setAsicPHYOCPReg(phyId, regId, regData);
			if(ret==SUCCESS)
			{
				rtlglue_printf("Write 83XX phyId(%d), ocpAddr(0x%x), regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}
		}
		else if (!memcmp(cmd_addr, "8367test", 8) || !memcmp(cmd_addr, "83xxtest", 8))
		{
			extern int rtk_port_phyTestMode_set(uint32 port, int mode);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			if (tokptr[0] == 'm') {
				extern int rtk_port_phyMdx_set(uint32 port, int mode);
				if(strncmp(tokptr, "mdi_auto",8) == 0)
					regId = 0; // PHY_AUTO_CROSSOVER_MODE
				else if(strncmp(tokptr, "mdix",4) == 0)
					regId = 2; // PHY_FORCE_MDIX_MODE
				else if(strncmp(tokptr, "mdi",3) == 0)
					regId = 1; // PHY_FORCE_MDI_MODE
				else
					regId = 3;
				ret=rtk_port_phyMdx_set(phyId, regId);
				if(ret==SUCCESS)
					rtlglue_printf("set 83XX phyId(%d) to mdi mode: %d\n", phyId, regId);
				else
					rtlglue_printf("rtk_port_phyMdx_set return error\n");
			}
			else {
				regId=simple_strtol(tokptr, NULL, 0);

				ret=rtk_port_phyTestMode_set(phyId, regId);
				if(ret==SUCCESS)
					rtlglue_printf("set 83XX phyId(%d) to mode: %d\n", phyId, regId);
				else
					rtlglue_printf("rtk_port_phyTestMode_set return error\n");
			}		
		}
		else if (!memcmp(cmd_addr, "8367init", 8) || !memcmp(cmd_addr, "83xxinit", 8))
		{
			#ifdef CONFIG_RTL_8367R_SUPPORT
			extern void init_8367r(void);
			init_8367r();
			#elif defined(CONFIG_RTL_83XX_SUPPORT)		
			extern void init_83XX(void);
			init_83XX();
			#endif
		}
		else if (!memcmp(cmd_addr, "8367snr", 7) || !memcmp(cmd_addr, "83xxsnr", 7))
		{
			/* Show 83xx link up ports' SNR value */
			rtl_get_83xx_snr();
		}		
#endif

#ifdef CONFIG_RTL_8370_SUPPORT
		else if (!memcmp(cmd_addr, "8370read", 8))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			if (!memcmp(tokptr, "l2", 2))
			{
				extern void get_all_L2(void);
				get_all_L2();
			}
			else 
			{
				regId=simple_strtol(tokptr, NULL, 16);
				ret = rtl8370_getAsicReg(regId, &regData);

				if(ret==0)
					rtlglue_printf("rtl8370_getAsicReg: reg= %x, data= %x\n", regId, regData);
				else
					rtlglue_printf("get fail %d\n", ret);
			}
		}
		else if (!memcmp(cmd_addr, "8370write", 9))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 16);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 16);

			ret = rtl8370_setAsicReg(regId, regData);

			if(ret==0)
			{
				rtlglue_printf("rtl8370_setAsicReg: reg= %x, data= %x\n", regId, regData);
			}
			else
			{
				rtlglue_printf("set fail %d\n", ret);
			}
		}
		else if (!memcmp(cmd_addr, "8370phyr", 8))
		{
			extern int rtl8370_getAsicPHYReg(uint32 phyNo, uint32 phyAddr, uint32 *pRegData );

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			ret=rtl8370_getAsicPHYReg(phyId, regId, &regData);
			if(ret==SUCCESS)
			{
				rtlglue_printf("read 8370 phyId(%d), regId(%d), regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}		
		}
		else if (!memcmp(cmd_addr, "8370phyw", 8))
		{
			extern int rtl8370_setAsicPHYReg(uint32 phyNo, uint32 phyAddr, uint32 phyData );
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 16);

			ret=rtl8370_setAsicPHYReg(phyId, regId, regData);
			if(ret==SUCCESS)
			{
				rtlglue_printf("Write 8370 phyId(%d), regId(%d), regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}
		}
		else if (!memcmp(cmd_addr, "8370test", 8))
		{
			extern int rtk_port_phyTestMode_set(uint32 port, int mode);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			ret=rtk_port_phyTestMode_set(phyId, regId);
			if(ret==SUCCESS)
			{
				rtlglue_printf("set 8370 phyId(%d) to mode: %d\n", phyId, regId);
			}
			else
			{
				rtlglue_printf("rtk_port_phyTestMode_set return error\n");
			}		
		}
#endif

#ifdef CONFIG_RTL_8325D_SUPPORT
		else if (!memcmp(cmd_addr, "8325read", 8))
		{
			int32 rtlglue_reg32_read_data(uint32 reg, uint32 *value);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 16);
			ret = rtlglue_reg32_read_data(regId, &regData);

			if(ret==0)
				rtlglue_printf("reg_read: reg= %x, data= %x\n", regId, regData);
			else
				rtlglue_printf("get fail %d\n", ret);
		}
		else if (!memcmp(cmd_addr, "8325write", 9))
		{
			int32 rtlglue_reg32_write(uint32 reg, uint32 value);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 16);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 16);

			ret = rtlglue_reg32_write(regId, regData);

			if(ret==0)
			{
				rtlglue_printf("reg_write: reg= %x, data= %x\n", regId, regData);
			}
			else
			{
				rtlglue_printf("set fail %d\n", ret);
			}
		}
		else if (!memcmp(cmd_addr, "8325phyr", 8))
		{
			extern int32 phy_reg_read(uint32 phyaddr, uint32 regaddr, uint32 page, uint16 *reg_data);
			uint16 data;
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			ret=phy_reg_read(phyId, regId, 0, &data);
			if(ret==SUCCESS)
			{
				rtlglue_printf("read 8325 phyId(%d), regId(%d), regData:0x%x\n", phyId, regId, data);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}		
		}
		else if (!memcmp(cmd_addr, "8325phyw", 8))
		{
			extern int32 phy_reg_write(uint32 phyaddr, uint32 regaddr, uint32 page, uint16 reg_data);
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 16);

			ret=phy_reg_write(phyId, regId, 0, regData);
			if(ret==SUCCESS)
			{
				rtlglue_printf("Write 8325 phyId(%d), regId(%d), regData:0x%x\n", phyId, regId, regData);
			}
			else
			{
				rtlglue_printf("error input!\n");
			}
		}		
#endif
		else if (!memcmp(cmd_addr, "extRead", 7))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			pageId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			//switch page
#if !defined(CONFIG_RTL_8198C) && !defined(CONFIG_RTL_8197F)
			if (pageId>=31)
			{
				rtl8651_setAsicEthernetPHYReg( phyId, 31, 7  );
				rtl8651_setAsicEthernetPHYReg( phyId, 30, pageId  );
			}
			else 
#endif
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
			if (gpio_simulate_mdc_mdio){
				if ((8 > phyId) && (phyId > 4)) {
					if (pageId>0)
					{
						rtl_mdio_write(phyId, 31 ,pageId);
					}

					ret = rtl_mdio_read(phyId, regId, &regData);

					if(ret==SUCCESS)
					{
						rtlglue_printf("extRead phyId(%d), pageId(%d), regId(%d), regData:0x%x\n", phyId,pageId, regId, regData);
					}
					else
					{
						rtlglue_printf("error input!\n");
					}

					//change back to page 0
					rtl_mdio_write(phyId, 31 , 0);
				} else {
					if (pageId>0)
					{
						rtl8651_setAsicEthernetPHYReg( phyId, 31, pageId  );
					}

					ret=rtl8651_getAsicEthernetPHYReg(phyId, regId, &regData);

					if(ret==SUCCESS)
					{
						rtlglue_printf("extRead phyId(%d), pageId(%d), regId(%d), regData:0x%x\n", phyId,pageId, regId, regData);
					}
					else
					{
						rtlglue_printf("error input!\n");
					}

					//change back to page 0
					rtl8651_setAsicEthernetPHYReg(phyId, 31, 0);
				}
			} else
#endif
			{
				if (pageId>0)
				{
					rtl8651_setAsicEthernetPHYReg( phyId, 31, pageId  );
				}

				ret=rtl8651_getAsicEthernetPHYReg(phyId, regId, &regData);

				if(ret==SUCCESS)
				{
					rtlglue_printf("extRead phyId(%d), pageId(%d), regId(%d), regData:0x%x\n", phyId,pageId, regId, regData);
				}
				else
				{
					rtlglue_printf("error input!\n");
				}

				//change back to page 0
				rtl8651_setAsicEthernetPHYReg(phyId, 31, 0);
			}


		}
		else if (!memcmp(cmd_addr, "extWrite", 8))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			pageId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regData=simple_strtol(tokptr, NULL, 0);

			//switch page
#if !defined(CONFIG_RTL_8198C) && !defined(CONFIG_RTL_8197F)
			if (pageId>=31)
			{
				rtl8651_setAsicEthernetPHYReg( phyId, 31, 7  );
				rtl8651_setAsicEthernetPHYReg( phyId, 30, pageId  );
			}
			else 
#endif		
#if defined(CONFIG_RTL_8197F) && defined(CONFIG_RTL_8211F_SUPPORT)
			if (gpio_simulate_mdc_mdio){
				if ((8 > phyId) && (phyId > 4)){
					if (pageId>0)
					{
						rtl_mdio_write(phyId, 31, pageId);
					}

					ret = rtl_mdio_write(phyId, regId, regData);

					if(ret==SUCCESS)
					{
						rtlglue_printf("extWrite phyId(%d), pageId(%d), regId(%d), regData:0x%x\n", phyId, pageId, regId, regData);
					}
					else
					{
						rtlglue_printf("error input!\n");
					}

					//change back to page 0
					rtl_mdio_write(phyId, 31, 0);
				 }else {
					if (pageId>0)
					{
						rtl8651_setAsicEthernetPHYReg( phyId, 31, pageId  );
					}

					ret=rtl8651_setAsicEthernetPHYReg(phyId, regId, regData);

					if(ret==SUCCESS)
					{
						rtlglue_printf("extWrite phyId(%d), pageId(%d), regId(%d), regData:0x%x\n", phyId, pageId, regId, regData);
					}
					else
					{
						rtlglue_printf("error input!\n");
					}

					//change back to page 0
					rtl8651_setAsicEthernetPHYReg(phyId, 31, 0);
				}
					
			}else
#endif
			{
				if (pageId>0)
				{
					rtl8651_setAsicEthernetPHYReg( phyId, 31, pageId  );
				}

				ret=rtl8651_setAsicEthernetPHYReg(phyId, regId, regData);

				if(ret==SUCCESS)
				{
					rtlglue_printf("extWrite phyId(%d), pageId(%d), regId(%d), regData:0x%x\n", phyId, pageId, regId, regData);
				}
				else
				{
					rtlglue_printf("error input!\n");
				}

				//change back to page 0
				rtl8651_setAsicEthernetPHYReg(phyId, 31, 0);
			}

		}
#ifdef CONFIG_RTL_8198C
		else if (!memcmp(cmd_addr, "sram98c", 7))
		{
			/* command: echo sram98c [phyid: 8/1/2/3/4/999] [oper: 0=read, 1=write] 
						[reg] [sramdata:write only] > /proc/rtl865x/phyReg
			*/
			extern void Sram98C(uint32 phyid, uint32 oper, uint32 RegAddr, uint32 sramdata);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			pageId=simple_strtol(tokptr, NULL, 0);

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			regId=simple_strtol(tokptr, NULL, 0);

			if (pageId == 1) { //write
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				regData=simple_strtol(tokptr, NULL, 0);
			}			
			Sram98C(phyId, pageId, regId, regData);
			rtlglue_printf("command done.\n");
		}
		else if (!memcmp(cmd_addr, "gigalite", 8))
		{
			extern void set_giga_lite(int mode);

			/* command: echo gigalite [0(disable)/1(enable)] > /proc/rtl865x/phyReg */
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			if ((phyId == 0) || (phyId == 1)) {
				set_giga_lite(phyId);
				for (i=0;i<5;i++)
					rtl8651_restartAsicEthernetPHYNway(i);
				rtlglue_printf("giga lite is %sabled.\n", (phyId ? "en" : "dis"));
			}			
			else
				goto errout;
		}		
		else if (!memcmp(cmd_addr, "ado", 3))
		{
			extern void ado_setting(int mode);

			/* command: echo ado [0(default)/1(modified)] > /proc/rtl865x/phyReg */
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			phyId=simple_strtol(tokptr, NULL, 0);

			if (phyId == 0) {
				ado_setting(phyId);
				rtlglue_printf("ado default\n");
			}			
			else if (phyId == 1) {
				ado_setting(0);
				ado_setting(phyId);
				rtlglue_printf("ado default and modified\n");
			}			
			else
				goto errout;
		}		
#endif
		else if (!memcmp(cmd_addr, "rtct", 4))
		{
			extern void RT_cable_test(void);
			RT_cable_test();
		}
		else if (!memcmp(cmd_addr, "diag", 4))
		{
			extern void phy_diag(uint8 mdimode, uint32 portmask);
			uint8 mdimode;

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout_diag;
			}
			mdimode=simple_strtol(tokptr, NULL, 0);

			if((mdimode == 2) || (mdimode>3)) {
				printk("wrong input of mdi/mdio mode\n");
				goto errout_diag;
			}	
			
			phy_diag(mdimode, 0x1f);
		}
		else if (!memcmp(cmd_addr, "forcediag", 9))
		{
			extern void force100m_phy_diag(uint32 portmask, uint8 mdimode, bool recover);
			uint32 portmask;
			uint8 mdimode;
			bool recover;
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout_forcediag;
			}
			portmask=simple_strtol(tokptr, NULL, 0);
			if (portmask == 0) {
				printk("wrong input of portmask\n");
				goto errout_forcediag;
			}

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout_forcediag;
			}
			mdimode=simple_strtol(tokptr, NULL, 0);
			if(mdimode>1) {
				printk("wrong input of mdi/mdio mode\n");
				goto errout_forcediag;
			}				

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout_forcediag;
			}
			recover=simple_strtol(tokptr, NULL, 0);

			force100m_phy_diag(portmask, mdimode, recover);
		}
		else if (!memcmp(cmd_addr, "forcephylink", 12))
		{
			extern void force_phy_linkup(uint32 portmask, bool forcelink);
			uint32 portmask;
			bool forcelink;
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout_forcephylink;
			}
			portmask=simple_strtol(tokptr, NULL, 0);
			if (portmask == 0) {
				printk("wrong input of portmask\n");
				goto errout_forcephylink;
			}

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout_forcephylink;
			}
			forcelink=simple_strtol(tokptr, NULL, 0);

			force_phy_linkup(portmask, forcelink);
		}
		else if (!memcmp(cmd_addr, "snr", 3))
		{
			uint32 	sum;
			for (i=0;i<5;i++)
			{
				if (REG32(PSRP0 + (i * 4)) & PortStatusLinkUp)
				{
					for (j=0, sum=0;j<10;j++)
					{
						rtl8651_getAsicEthernetPHYReg(i, 29, &regData);
						sum += regData;
						mdelay(10);
					}
					sum /= 10;
					//db = -(10 * log(sum/262144));
					//printk("  port %d SNR = %d dB\n", i, db);
					rtlglue_printf("  port %d SUM = %d\n", i, sum);
				}
				else
				{
					rtlglue_printf("  port %d is link down\n", i);
				}
			}
		}
		else if (!memcmp(cmd_addr, "dumpAll", 7))
		{
			int port;
#ifdef CONFIG_RTL_8196C
			int page,reg;
#endif
			for (port=0; port<5; port++)
			{
				rtlglue_printf("==========================================\n");

#ifdef CONFIG_RTL_8196C
				for(page=0;page<4;page++)
				{
					//change page num
					rtl8651_setAsicEthernetPHYReg(port, 31, page);
					for(reg=0;reg<32;reg++)
					{
						rtl8651_getAsicEthernetPHYReg( port, reg, &regData);
						rtlglue_printf("port:%d,page:%d,regId:%d,regData:0x%x\n",port,page,reg,regData);
					}
					if(page!=3)
					{
						rtlglue_printf("------------------------------------------\n");
					}
					//change back to page 0
					rtl8651_setAsicEthernetPHYReg(port, 31, 0 );
				}
#elif defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F)
				//set pageNum {0 1 2 3 4 5 6 32 33 34 35 36 40 44 45 46 64 65 66 69 70 80 81 161}
				for (i=0; i<24; i++)
					getPhyByPortPage(port,  _8198_phy_page[i]);

#elif defined(CONFIG_RTL_8198C)
				//set pageNum: 0xa40 ~ 0xa72; 0xbc0 ~ 0xbc7; 0xc80 ~ 0xc81
				for (i=0; i<51; i++) {
					getPhyByPortPage(port, (0xa40 + i));
				}
				for (i=0; i<8; i++) {
					getPhyByPortPage(port, (0xbc0 + i));
				}
				getPhyByPortPage(port, 0xc80);
				getPhyByPortPage(port, 0xc81);
#endif
			}
		}
	}
	else
	{
errout:
		rtlglue_printf("error input!\n");
		return len;
errout_diag:
		rtlglue_printf("echo diag [mdi/mdix mode] >/proc/rtl865x/phyReg\n");		
		rtlglue_printf("[mdi/mdix mode]=0(force mdix), 1(force mdi), 3(auto mdi/mdix)\n");
		return len;
errout_forcediag:
		rtlglue_printf("echo forcediag [portmask] [mdi/mdix mode] [recover] >/proc/rtl865x/phyReg\n");		
		rtlglue_printf("[mdi/mdix mode]=0(force mdix), 1(force mdi)\n");
		rtlglue_printf("[recover]=0(stay in force 100_half and force mdi/force mdix mode), 1(back to AN mode and auto mdi/mdix mode)\n");
		return len;
errout_forcephylink:
		rtlglue_printf("echo forcephylink [portmask] [forcelinkup] >/proc/rtl865x/phyReg\n");		
		rtlglue_printf("[forcelinkup]=0(set phy forcelink bit to 0), 1(set phy forcelink bit to 1)\n");		
		return len;
	}

	return len;
}

//MACCR porc--------------------------------------------------------------------------------------------
#ifdef CONFIG_RTL_PROC_NEW
static int32 mac_config_read(struct seq_file *s, void *v)
{
	uint32	regData;
	uint32	data0;
	
	seq_printf(s, "MAC Configuration Register Info:\n");
	regData = READ_MEM32(MACCR);
	
	data0 = regData & IPG_SEL;
	seq_printf(s, "IfgSel:  ");
	if (data0)
		seq_printf(s, "internal counter =352\n");
	else
		seq_printf(s, "internal counter =480\n");
	
	data0 = regData & INFINITE_PAUSE_FRAMES  ;
	seq_printf(s, "INFINITE_PAUSE_FRAMES:  ");
	if (data0)
		seq_printf(s, "Infinite pause frame count\n");
	else
		seq_printf(s, "Maximum of 128 consecutive pause frames\n");

	data0= regData & LONG_TXE;
	seq_printf(s, "LONG_TXE:  ");
	if (data0)
		seq_printf(s, "Carrier-based backpressure\n");
	else
		seq_printf(s, "Collision-based backpressure\n");

	data0= regData & EN_48_DROP;
	seq_printf(s, "EN_48_DROP:  ");
	if (data0)
		seq_printf(s, "Enabled\n");
	else
		seq_printf(s, "Disabled\n");
	
	data0= (regData & SELIPG_MASK)>>SELIPG_OFFSET;
	seq_printf(s, "SELIPG:  ");
	if(data0==0x00)
		seq_printf(s, "7 byte-time\n");
	else if(data0==0x01)
		seq_printf(s, "8 byte-time\n");
	else if(data0==0x10)
		seq_printf(s, "10 byte-time\n");
	else
		seq_printf(s, "12 byte-time\n");
	
	data0= (regData & CF_SYSCLK_SEL_MASK)>>CF_SYSCLK_SEL_OFFSET;
	seq_printf(s, "CF_SYSCLK_SEL:  ");
	if(data0==0x00)
		seq_printf(s, "50MHz\n");
	else if(data0==0x01)
		seq_printf(s, "100MHz\n");
	else
		seq_printf(s, "reserved status\n");

	data0= (regData & CF_FCDSC_MASK)>>CF_FCDSC_OFFSET;
	seq_printf(s, "CF_FCDSC:  %d pages\n",data0);

	data0= (regData & CF_RXIPG_MASK);
	seq_printf(s, "CF_RXIPG:  %d pkts\n",data0);
	
	return 0;

}
#else
static int32 mac_config_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int		len;
	uint32	regData;
	uint32	data0;
	
	len = sprintf(page, "MAC Configuration Register Info:\n");
	regData = READ_MEM32(MACCR);
	
	data0 = regData & IPG_SEL;
	len += sprintf(page+len, "IfgSel:  ");
	if (data0)
		len += sprintf(page+len, "internal counter =352\n");
	else
		len += sprintf(page+len, "internal counter =480\n");
	
	data0 = regData & INFINITE_PAUSE_FRAMES  ;
	len += sprintf(page+len, "INFINITE_PAUSE_FRAMES:  ");
	if (data0)
		len += sprintf(page+len, "Infinite pause frame count\n");
	else
		len += sprintf(page+len, "Maximum of 128 consecutive pause frames\n");

	data0= regData & LONG_TXE;
	len += sprintf(page+len, "LONG_TXE:  ");
	if (data0)
		len += sprintf(page+len, "Carrier-based backpressure\n");
	else
		len += sprintf(page+len, "Collision-based backpressure\n");

	data0= regData & EN_48_DROP;
	len += sprintf(page+len, "EN_48_DROP:  ");
	if (data0)
		len += sprintf(page+len, "Enabled\n");
	else
		len += sprintf(page+len, "Disabled\n");
	
	data0= (regData & SELIPG_MASK)>>SELIPG_OFFSET;
	len += sprintf(page+len, "SELIPG:  ");
	if(data0==0x00)
		len += sprintf(page+len, "7 byte-time\n");
	else if(data0==0x01)
		len += sprintf(page+len, "8 byte-time\n");
	else if(data0==0x10)
		len += sprintf(page+len, "10 byte-time\n");
	else
		len += sprintf(page+len, "12 byte-time\n");
	
	data0= (regData & CF_SYSCLK_SEL_MASK)>>CF_SYSCLK_SEL_OFFSET;
	len += sprintf(page+len, "CF_SYSCLK_SEL:  ");
	if(data0==0x00)
		len += sprintf(page+len, "50MHz\n");
	else if(data0==0x01)
		len += sprintf(page+len, "100MHz\n");
	else
		len += sprintf(page+len, "reserved status\n");

	data0= (regData & CF_FCDSC_MASK)>>CF_FCDSC_OFFSET;
	len += sprintf(page+len, "CF_FCDSC:  %d pages\n",data0);

	data0= (regData & CF_RXIPG_MASK);
	len += sprintf(page+len, "CF_RXIPG:  %d pkts\n",data0);
	
	return len;

}
#endif

static int32 mac_config_write(struct file *filp, const char *buff,unsigned long len, void *data )
{
		return len;
}

//FC threshold--------------------------------------------------------------------------
#ifdef CONFIG_RTL_PROC_NEW
static int32 fc_threshold_read(struct seq_file *s, void *v)
{
	uint32	regData;
	uint32	data0;
	int port;	
	int group=3;
	seq_printf(s, "Dump FC threshold Information:\n");

	//SBFCR0
	regData = READ_MEM32(SBFCR0);
	data0 = regData & S_DSC_RUNOUT_MASK;
	seq_printf(s, "S_DSC_RUNOUT:%d\n",data0);
	//SBFCR1
	regData = READ_MEM32(SBFCR1);
	data0 = (regData & SDC_FCOFF_MASK)>>SDC_FCOFF_OFFSET;
	seq_printf(s, "SDC_FCOFF:%d, ",data0);
	data0 = (regData & SDC_FCON_MASK)>>SDC_FCON_OFFSET;
	seq_printf(s, "SDC_FCON:%d\n",data0);
	//SBFCR2
	regData = READ_MEM32(SBFCR2);
	data0 = (regData & S_Max_SBuf_FCOFF_MASK)>>S_Max_SBuf_FCOFF_OFFSET;
	seq_printf(s, "S_MaxSBuf_FCOFF:%d, ",data0);
	data0 = (regData & S_Max_SBuf_FCON_MASK)>>S_Max_SBuf_FCON_OFFSET;
	seq_printf(s, "S_MaxSBuf_FCON:%d\n",data0);
	//FCCR0,FCCR1
	regData = READ_MEM32(FCCR0);
	data0 =(regData & Q_P0_EN_FC_MASK)>>(Q_P0_EN_FC_OFFSET);	
	seq_printf(s, "Q_P0_EN_FC:%0x, ",data0);	
	data0 =(regData & Q_P1_EN_FC_MASK)>>(Q_P1_EN_FC_OFFSET);	
	seq_printf(s, "Q_P1_EN_FC:%0x, ",data0);	
	data0 =(regData & Q_P2_EN_FC_MASK)>>(Q_P2_EN_FC_OFFSET);	
	seq_printf(s, "Q_P2_EN_FC:%0x\n",data0);
	data0 =(regData & Q_P3_EN_FC_MASK)>>(Q_P3_EN_FC_OFFSET);	
	seq_printf(s, "Q_P3_EN_FC:%0x, ",data0);
	regData = READ_MEM32(FCCR1);
	data0 =(regData & Q_P4_EN_FC_MASK)>>(Q_P4_EN_FC_OFFSET);	
	seq_printf(s, "Q_P4_EN_FC:%0x, ",data0);	
	regData = READ_MEM32(FCCR0);
	data0 =(regData & Q_P5_EN_FC_MASK)>>(Q_P5_EN_FC_OFFSET);	
	seq_printf(s, "Q_P5_EN_FC:%0x\n",data0);	
	//PQPLGR
	regData = READ_MEM32(PQPLGR);
	data0 =regData & QLEN_GAP_MASK;
	seq_printf(s, "QLEN_GAP:%d\n",data0);
	//QRR
	regData = READ_MEM32(QRR);
	data0 =regData & QRST;
	seq_printf(s, "QRST:%d\n",data0);
	//IQFCTCR
	regData = READ_MEM32(IQFCTCR);
	data0 =(regData & IQ_DSC_FCON_MASK)>>IQ_DSC_FCON_OFFSET;
	seq_printf(s, "IQ_DSC_FCON:%d, ",data0);
	data0 =(regData & IQ_DSC_FCOFF_MASK)>>IQ_DSC_FCOFF_OFFSET;
	seq_printf(s, "IQ_DSC_FCOFF:%d\n",data0);
	//QNUMCR
	regData = READ_MEM32(QNUMCR);
	seq_printf(s,"The number of output queue for port(0~6) :\n");

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	data0=(regData & P0QNum_MASK )>>(P0QNum_OFFSET);
	seq_printf(s, "P0QNum%d, ",data0+1);
	data0=(regData & P1QNum_MASK )>>(P1QNum_OFFSET);
	seq_printf(s, "P1QNum%d, ",data0+1);
	data0=(regData & P2QNum_MASK )>>(P2QNum_OFFSET);
	seq_printf(s, "P2QNum%d, ",data0+1);
	data0=(regData & P3QNum_MASK )>>(P3QNum_OFFSET);
	seq_printf(s, "P3QNum%d, ",data0+1);
	data0=(regData & P4QNum_MASK )>>(P4QNum_OFFSET);
	seq_printf(s, "P4QNum%d, ",data0+1);
	data0=(regData & P5QNum_MASK )>>(P5QNum_OFFSET);
	seq_printf(s, "P5QNum%d, ",data0+1);
	data0=(regData & P6QNum_MASK )>>(P6QNum_OFFSET);
	seq_printf(s, "P6QNum%d\n",data0+1);
#else
	data0=(regData & P0QNum_MASK )>>(P0QNum_OFFSET);
	seq_printf(s, "P0QNum%d, ",data0);
	data0=(regData & P1QNum_MASK )>>(P1QNum_OFFSET);
	seq_printf(s, "P1QNum%d, ",data0);
	data0=(regData & P2QNum_MASK )>>(P2QNum_OFFSET);
	seq_printf(s, "P2QNum%d, ",data0);
	data0=(regData & P3QNum_MASK )>>(P3QNum_OFFSET);
	seq_printf(s, "P3QNum%d, ",data0);
	data0=(regData & P4QNum_MASK )>>(P4QNum_OFFSET);
	seq_printf(s, "P4QNum%d, ",data0);
	data0=(regData & P5QNum_MASK )>>(P5QNum_OFFSET);
	seq_printf(s, "P5QNum%d, ",data0);
	data0=(regData & P6QNum_MASK )>>(P6QNum_OFFSET);
	seq_printf(s, "P6QNum%d\n",data0);
#endif	
	//per port   
	for(port=PHY0;port<=CPU;port++)
	{
		if (port==CPU)
		seq_printf(s, "\nCPUPort\n");
		else
		seq_printf(s, "\nPort%d\n", port);
		
		regData = READ_MEM32(PBFCR0+((port)<<2));
		data0 = (regData & P_MaxDSC_FCOFF_MASK)>>P_MaxDSC_FCOFF_OFFSET;
		seq_printf(s, "   P_MaxDSC_FCOFF:%d, ",data0);
		data0 = (regData & P_MaxDSC_FCON_MASK)>>P_MaxDSC_FCON_OFFSET;
		seq_printf(s, "P_MaxDSC_FCON:%d\n",data0);
		//if(port<CPU)
		{
			for (group=GR0;group<=GR2;group++)
			{
				seq_printf(s, "   Port%dGroup%d\n",port,group);
				/* QDBFCRP0G0,QDBFCRP0G1,QDBFCRP0G2
				 * QDBFCRP1G0,QDBFCRP1G1,QDBFCRP1G2
				 * QDBFCRP2G0,QDBFCRP2G1,QDBFCRP2G2
				 * QDBFCRP3G0,QDBFCRP3G1,QDBFCRP3G2
				 * QDBFCRP4G0,QDBFCRP4G1,QDBFCRP4G2
				 * QDBFCRP5G0,QDBFCRP5G1,QDBFCRP5G2
				 * - Queue-Descriptor=Based Flow Control Threshold for Port 0 Group 0 */
				regData =READ_MEM32(QDBFCRP0G0+((port)*0xC)+((group)<<2));
				//seq_printf(s,"address:%0x",(QDBFCRP0G0+((port)<<2)+((group)<<2)));
				data0 = (regData & QG_DSC_FCOFF_MASK)>>QG_DSC_FCOFF_OFFSET;
				seq_printf(s, "   QG_DSC_FCOFF:%d, ",data0);
				data0 = (regData & QG_DSC_FCON_MASK)>>QG_DSC_FCON_OFFSET;
				seq_printf(s, "QG_DSC_FCON:%d, ",data0);

				/* QPKTFCRP0G0,QPKTFCRP0G1,QPKTFCRP0G2
				 * QPKTFCRP1G0,QPKTFCRP1G1,QPKTFCRP1G2
				 * QPKTFCRP2G0,QPKTFCRP2G1,QPKTFCRP2G2
				 * QPKTFCRP3G0,QPKTFCRP3G1,QPKTFCRP3G2
				 * QPKTFCRP4G0,QPKTFCRP4G1,QPKTFCRP4G2
				 * QPKTFCRP5G0,QPKTFCRP5G1,QPKTFCRP5G2
				   - Queue-Packet-Based Flow Control Register for Port 0 Group 0 */
				regData =READ_MEM32(QPKTFCRP0G0+((port)*0xC)+((group)<<2));
				//seq_printf(s,"address:%0x",(QPKTFCRP0G0+((port)<<2)+((group)<<2)));
				data0 = (regData & QG_QLEN_FCOFF_MASK)>>QG_QLEN_FCOFF_OFFSET;
				seq_printf(s, "QG_QLEN_FCOFF:%d, ",data0);
				data0 = (regData & QG_QLEN_FCON_MASK)>>QG_QLEN_FCON_OFFSET;
				seq_printf(s, "QG_QLEN_FCON:%d\n",data0);
				
			}	
		}
	}

	return 0;
}
#else
static int32 fc_threshold_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{

	int 	len;
	uint32	regData;
	uint32	data0;
	int port;	
	int group=3;
	len = sprintf(page, "Dump FC threshold Information:\n");

	//SBFCR0
	regData = READ_MEM32(SBFCR0);
	data0 = regData & S_DSC_RUNOUT_MASK;
	len += sprintf(page+len, "S_DSC_RUNOUT:%d\n",data0);
	//SBFCR1
	regData = READ_MEM32(SBFCR1);
	data0 = (regData & SDC_FCOFF_MASK)>>SDC_FCOFF_OFFSET;
	len += sprintf(page+len, "SDC_FCOFF:%d, ",data0);
	data0 = (regData & SDC_FCON_MASK)>>SDC_FCON_OFFSET;
	len += sprintf(page+len, "SDC_FCON:%d\n",data0);
	//SBFCR2
	regData = READ_MEM32(SBFCR2);
	data0 = (regData & S_Max_SBuf_FCOFF_MASK)>>S_Max_SBuf_FCOFF_OFFSET;
	len += sprintf(page+len, "S_MaxSBuf_FCOFF:%d, ",data0);
	data0 = (regData & S_Max_SBuf_FCON_MASK)>>S_Max_SBuf_FCON_OFFSET;
	len += sprintf(page+len, "S_MaxSBuf_FCON:%d\n",data0);
	//FCCR0,FCCR1
	regData = READ_MEM32(FCCR0);
	data0 =(regData & Q_P0_EN_FC_MASK)>>(Q_P0_EN_FC_OFFSET);	
	len += sprintf(page+len, "Q_P0_EN_FC:%0x, ",data0);	
	data0 =(regData & Q_P1_EN_FC_MASK)>>(Q_P1_EN_FC_OFFSET);	
	len += sprintf(page+len, "Q_P1_EN_FC:%0x, ",data0);	
	data0 =(regData & Q_P2_EN_FC_MASK)>>(Q_P2_EN_FC_OFFSET);	
	len += sprintf(page+len, "Q_P2_EN_FC:%0x\n",data0);
	data0 =(regData & Q_P3_EN_FC_MASK)>>(Q_P3_EN_FC_OFFSET);	
	len += sprintf(page+len, "Q_P3_EN_FC:%0x, ",data0);
	regData = READ_MEM32(FCCR1);
	data0 =(regData & Q_P4_EN_FC_MASK)>>(Q_P4_EN_FC_OFFSET);	
	len += sprintf(page+len, "Q_P4_EN_FC:%0x, ",data0);	
	regData = READ_MEM32(FCCR0);
	data0 =(regData & Q_P5_EN_FC_MASK)>>(Q_P5_EN_FC_OFFSET);	
	len += sprintf(page+len, "Q_P5_EN_FC:%0x\n",data0);	
	//PQPLGR
	regData = READ_MEM32(PQPLGR);
	data0 =regData & QLEN_GAP_MASK;
	len += sprintf(page+len, "QLEN_GAP:%d\n",data0);
	//QRR
	regData = READ_MEM32(QRR);
	data0 =regData & QRST;
	len += sprintf(page+len, "QRST:%d\n",data0);
	//IQFCTCR
	regData = READ_MEM32(IQFCTCR);
	data0 =(regData & IQ_DSC_FCON_MASK)>>IQ_DSC_FCON_OFFSET;
	len += sprintf(page+len, "IQ_DSC_FCON:%d, ",data0);
	data0 =(regData & IQ_DSC_FCOFF_MASK)>>IQ_DSC_FCOFF_OFFSET;
	len += sprintf(page+len, "IQ_DSC_FCOFF:%d\n",data0);
	//QNUMCR
	regData = READ_MEM32(QNUMCR);
	len += sprintf(page+len,"The number of output queue for port(0~6) :\n");

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	data0=(regData & P0QNum_MASK )>>(P0QNum_OFFSET);
	len += sprintf(page+len, "P0QNum%d, ",data0+1);
	data0=(regData & P1QNum_MASK )>>(P1QNum_OFFSET);
	len += sprintf(page+len, "P1QNum%d, ",data0+1);
	data0=(regData & P2QNum_MASK )>>(P2QNum_OFFSET);
	len += sprintf(page+len, "P2QNum%d, ",data0+1);
	data0=(regData & P3QNum_MASK )>>(P3QNum_OFFSET);
	len += sprintf(page+len, "P3QNum%d, ",data0+1);
	data0=(regData & P4QNum_MASK )>>(P4QNum_OFFSET);
	len += sprintf(page+len, "P4QNum%d, ",data0+1);
	data0=(regData & P5QNum_MASK )>>(P5QNum_OFFSET);
	len += sprintf(page+len, "P5QNum%d, ",data0+1);
	data0=(regData & P6QNum_MASK )>>(P6QNum_OFFSET);
	len += sprintf(page+len, "P6QNum%d\n",data0+1);
#else
	data0=(regData & P0QNum_MASK )>>(P0QNum_OFFSET);
	len += sprintf(page+len, "P0QNum%d, ",data0);
	data0=(regData & P1QNum_MASK )>>(P1QNum_OFFSET);
	len += sprintf(page+len, "P1QNum%d, ",data0);
	data0=(regData & P2QNum_MASK )>>(P2QNum_OFFSET);
	len += sprintf(page+len, "P2QNum%d, ",data0);
	data0=(regData & P3QNum_MASK )>>(P3QNum_OFFSET);
	len += sprintf(page+len, "P3QNum%d, ",data0);
	data0=(regData & P4QNum_MASK )>>(P4QNum_OFFSET);
	len += sprintf(page+len, "P4QNum%d, ",data0);
	data0=(regData & P5QNum_MASK )>>(P5QNum_OFFSET);
	len += sprintf(page+len, "P5QNum%d, ",data0);
	data0=(regData & P6QNum_MASK )>>(P6QNum_OFFSET);
	len += sprintf(page+len, "P6QNum%d\n",data0);
#endif	
	//per port   
	for(port=PHY0;port<=CPU;port++)
	{
		if (port==CPU)
		len += sprintf(page+len, "\nCPUPort\n");
		else
		len += sprintf(page+len, "\nPort%d\n", port);
		
		regData = READ_MEM32(PBFCR0+((port)<<2));
		data0 = (regData & P_MaxDSC_FCOFF_MASK)>>P_MaxDSC_FCOFF_OFFSET;
		len += sprintf(page+len, "   P_MaxDSC_FCOFF:%d, ",data0);
		data0 = (regData & P_MaxDSC_FCON_MASK)>>P_MaxDSC_FCON_OFFSET;
		len += sprintf(page+len, "P_MaxDSC_FCON:%d\n",data0);
		//if(port<CPU)
		{
			for (group=GR0;group<=GR2;group++)
			{
				len += sprintf(page+len, "   Port%dGroup%d\n",port,group);
				/* QDBFCRP0G0,QDBFCRP0G1,QDBFCRP0G2
				 * QDBFCRP1G0,QDBFCRP1G1,QDBFCRP1G2
				 * QDBFCRP2G0,QDBFCRP2G1,QDBFCRP2G2
				 * QDBFCRP3G0,QDBFCRP3G1,QDBFCRP3G2
				 * QDBFCRP4G0,QDBFCRP4G1,QDBFCRP4G2
				 * QDBFCRP5G0,QDBFCRP5G1,QDBFCRP5G2
				 * - Queue-Descriptor=Based Flow Control Threshold for Port 0 Group 0 */
				regData =READ_MEM32(QDBFCRP0G0+((port)<<2)+((group)<<2));
				//len+=sprintf(page+len,"address:%0x",(QDBFCRP0G0+((port)<<2)+((group)<<2)));
				data0 = (regData & QG_DSC_FCOFF_MASK)>>QG_DSC_FCOFF_OFFSET;
				len += sprintf(page+len, "   QG_DSC_FCOFF:%d, ",data0);
				data0 = (regData & QG_DSC_FCON_MASK)>>QG_DSC_FCON_OFFSET;
				len += sprintf(page+len, "QG_DSC_FCON:%d, ",data0);

				/* QPKTFCRP0G0,QPKTFCRP0G1,QPKTFCRP0G2
				 * QPKTFCRP1G0,QPKTFCRP1G1,QPKTFCRP1G2
				 * QPKTFCRP2G0,QPKTFCRP2G1,QPKTFCRP2G2
				 * QPKTFCRP3G0,QPKTFCRP3G1,QPKTFCRP3G2
				 * QPKTFCRP4G0,QPKTFCRP4G1,QPKTFCRP4G2
				 * QPKTFCRP5G0,QPKTFCRP5G1,QPKTFCRP5G2
				   - Queue-Packet-Based Flow Control Register for Port 0 Group 0 */
				regData =READ_MEM32(QPKTFCRP0G0+((port)<<2)+((group)<<2));
				//len+=sprintf(page+len,"address:%0x",(QPKTFCRP0G0+((port)<<2)+((group)<<2)));
				data0 = (regData & QG_QLEN_FCOFF_MASK)>>QG_QLEN_FCOFF_OFFSET;
				len += sprintf(page+len, "QG_QLEN_FCOFF:%d, ",data0);
				data0 = (regData & QG_QLEN_FCON_MASK)>>QG_QLEN_FCON_OFFSET;
				len += sprintf(page+len, "QG_QLEN_FCON:%d\n",data0);
				
			}	
		}
	}

	return len;
}
#endif

static int32 fc_threshold_write(struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT)
//l3v6--------------------------------------------------------------------------

#ifdef CONFIG_RTL_PROC_NEW
static int32 l3v6_read(struct seq_file *s, void *v)
{
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3	
	uint32 idx, mask;
	rtl8198C_tblAsicDrv_routingv6Param_t asic_l3v6;
	inv6_addr_t tmp_ip;
	int8 *str[] = { "PPPoE", "L2", "ARP", " ", "CPU", "NxtHop", "DROP", " " };

	seq_printf(s, "%s\n", "ASIC L3V6 Routing Table:\n");
	for (idx=0; idx<RTL8198C_ROUTINGV6TBL_SIZE; idx++)
	{
		if (rtl8198C_getAsicRoutingv6(idx, &asic_l3v6) == FAILED) {
			seq_printf(s,"\t[%d]  (Invalid)\n", idx);
			continue;
		}
		ntohl_array((uint32 *)&asic_l3v6.ipAddr.v6_addr32[0], (void *)&tmp_ip.v6_addr32[0], 4);
		mask = asic_l3v6.ipMask;

		seq_printf(s,"\t[%d]  %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d process(%s) \n", idx,
                    htons(tmp_ip.v6_addr16[0]),htons(tmp_ip.v6_addr16[1]),htons(tmp_ip.v6_addr16[2]),htons(tmp_ip.v6_addr16[3]),
            		htons(tmp_ip.v6_addr16[4]),htons(tmp_ip.v6_addr16[5]),htons(tmp_ip.v6_addr16[6]),htons(tmp_ip.v6_addr16[7]),
			        mask, str[asic_l3v6.process]);

		switch(asic_l3v6.process)
		{
			case 0x00:	/* PPPoE */
				seq_printf(s,"\t           dvidx(%d)  pppidx(%d) nxthop(%d)\n", asic_l3v6.vidx, asic_l3v6.pppoeIdx, (asic_l3v6.nextHopRow<<2)+asic_l3v6.nextHopColumn);
				break;

			case 0x01:	/* L2 */
				seq_printf(s,"              dvidx(%d) nexthop(%d)\n", asic_l3v6.vidx, (asic_l3v6.nextHopRow<<2)+asic_l3v6.nextHopColumn);
				break;

			case 0x02:	/* ARP */
				seq_printf(s,"             dvidx(%d) SUBNETIDX(%d)\n", asic_l3v6.vidx, asic_l3v6.subnet_idx);
				break;

			case 0x03:	/* Reserved */
				;

			case 0x04:	/* CPU */
				seq_printf(s,"             dvidx(%d)\n", asic_l3v6.vidx);
				break;

			case 0x05:	/* NAPT Next Hop */
				seq_printf(s,"              NHSTA(%d) NHNUM(%d) NHNXT(%d) NHALGO(%d)\n", asic_l3v6.nhStart,
						asic_l3v6.nhNum, asic_l3v6.nhNxt, asic_l3v6.nhAlgo);
				break;

			case 0x06:	/* DROP */
				seq_printf(s,"             dvidx(%d)\n", asic_l3v6.vidx);
				break;

			case 0x07:	/* Reserved */
				/* pass through */
			default:
				;
		}
		
        seq_printf(s,"\tsix_rd_eg :0x%x\tsix_rd_idx:0x%x\n", asic_l3v6.six_rd_eg, asic_l3v6.six_rd_idx);
	}	
#endif
	return 0;
}
#else
static int32 l3v6_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	uint32 idx, mask;
	rtl8198C_tblAsicDrv_routingv6Param_t asic_l3v6;
	inv6_addr_t tmp_ip;
	int8 *str[] = { "PPPoE", "L2", "ARP", " ", "CPU", "NxtHop", "DROP", " " };

	len = sprintf(page, "%s\n", "ASIC L3V6 Routing Table:\n");
	for (idx=0; idx<RTL8198C_ROUTINGV6TBL_SIZE; idx++)
	{
		if (rtl8198C_getAsicRoutingv6(idx, &asic_l3v6) == FAILED) {
			len += sprintf(page + len,"\t[%d]  (Invalid)\n", idx);
			continue;
		}
		ntohl_array((uint32 *)&asic_l3v6.ipAddr.v6_addr32[0], (void *)&tmp_ip.v6_addr32[0], 4);
        mask = asic_l3v6.ipMask;

		len += sprintf(page + len,"\t[%d]  %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d process(%s) \n", idx,
					htons(tmp_ip.v6_addr16[0]),htons(tmp_ip.v6_addr16[1]),htons(tmp_ip.v6_addr16[2]),htons(tmp_ip.v6_addr16[3]),
					htons(tmp_ip.v6_addr16[4]),htons(tmp_ip.v6_addr16[5]),htons(tmp_ip.v6_addr16[6]),htons(tmp_ip.v6_addr16[7]),
			        mask, str[asic_l3v6.process]);

		switch(asic_l3v6.process)
		{
			case 0x00:	/* PPPoE */
				len += sprintf(page + len,"\t           dvidx(%d)  pppidx(%d) nxthop(%d)\n", asic_l3v6.vidx, asic_l3v6.pppoeIdx, (asic_l3v6.nextHopRow<<2)+asic_l3v6.nextHopColumn);
				break;

			case 0x01:	/* L2 */
				len += sprintf(page + len,"              dvidx(%d) nexthop(%d)\n", asic_l3v6.vidx, (asic_l3v6.nextHopRow<<2)+asic_l3v6.nextHopColumn);
				break;

			case 0x02:	/* ARP */
				len += sprintf(page + len,"             dvidx(%d) SUBNETIDX(%d)\n", asic_l3v6.vidx, asic_l3v6.subnet_idx);
				break;

			case 0x03:	/* Reserved */
				;

			case 0x04:	/* CPU */
				len += sprintf(page + len,"             dvidx(%d)\n", asic_l3v6.vidx);
				break;

			case 0x05:	/* NAPT Next Hop */
				len += sprintf(page + len,"              NHSTA(%d) NHNUM(%d) NHNXT(%d) NHALGO(%d)\n", asic_l3v6.nhStart,
						asic_l3v6.nhNum, asic_l3v6.nhNxt, asic_l3v6.nhAlgo);
				break;

			case 0x06:	/* DROP */
				len += sprintf(page + len,"             dvidx(%d)\n", asic_l3v6.vidx);
				break;

			case 0x07:	/* Reserved */
				/* pass through */
			default:
				;
		}
		
        len += sprintf(page + len,"\tsix_rd_eg :0x%x\tsix_rd_idx:0x%x\n", asic_l3v6.six_rd_eg, asic_l3v6.six_rd_idx);
	}	

	return len;
}
#endif

static int32 l3v6_write(struct file *filp, const char *buff, unsigned long len, void *data )
{
	return len;
}

//arp6--------------------------------------------------------------------------

#ifdef CONFIG_RTL_PROC_NEW
static int32 arp6_read(struct seq_file *s, void *v)
{
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3	
    int i = 0;
    rtl8198C_tblAsicDrv_arpV6Param_t asic_arpv6;
	seq_printf(s, "%s\n", "ASIC ArpV6 Table:\n");
	for (i=0; i<RTL8198C_ARPV6TBL_SIZE; i++)
	{
		if (rtl8198C_getAsicArpV6(i,  &asic_arpv6) == FAILED)
			continue;

        	seq_printf(s, " ARPv6: idx:%3d  L2:(%3d,%d), subnet_idx:%d HOSTID==>(%x:%x:%x:%x) aging:%d\n", i, asic_arpv6.nextHopRow, 
					asic_arpv6.nextHopColumn, asic_arpv6.subnet_id, 
					asic_arpv6.hostid.v6_addr32[0], 
					asic_arpv6.hostid.v6_addr32[1], 
					asic_arpv6.hostid.v6_addr32[2],
					asic_arpv6.hostid.v6_addr32[3],
					asic_arpv6.aging);
	}
#endif
	return 0;
}
#else
static int32 arp6_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int 	len=0,i;
    rtl8198C_tblAsicDrv_arpV6Param_t asic_arpv6;
	len = sprintf(page, "%s\n", "ASIC ArpV6 Table:\n");
	for (i=0; i<RTL8198C_ARPV6TBL_SIZE; i++)
	{
		if (rtl8198C_getAsicArpV6(i,  &asic_arpv6) == FAILED)
			continue;

        	len += sprintf(page + len, " ARPv6: idx:%3d  L2:(%3d,%d), subnet_idx:%d HOSTID==>::%04x:%04x:%04x:%04x aging:%d\n", i, asic_arpv6.nextHopRow, 
					asic_arpv6.nextHopColumn, asic_arpv6.subnet_id, asic_arpv6.hostid.v6_addr16[4], asic_arpv6.hostid.v6_addr16[5], asic_arpv6.hostid.v6_addr16[6],
					asic_arpv6.hostid.v6_addr16[7], asic_arpv6.aging);
	}

	return len;
}
#endif

static int32 arp6_write(struct file *filp, const char *buff, unsigned long len, void *data )
{
	return len;
}



//nexthop6--------------------------------------------------------------------------

#ifdef CONFIG_RTL_PROC_NEW
static int32 nexthop6_read(struct seq_file *s, void *v)
{
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3	
	uint32 idx;
	rtl8198C_tblAsicDrv_nextHopV6Param_t asic_nxthopv6;

	seq_printf(s, "%s\n", "ASIC Next Hop V6 Table:\n");
	for (idx=0; idx<RTL8198C_NEXTHOPV6TBL_SIZE; idx++) 
	{
		if (rtl8198C_getAsicNextHopTableV6(idx, &asic_nxthopv6) == FAILED)
			continue;
		
		seq_printf(s,"  [%d]  type(%s) dstVid(%d) pppoeIdx(%d) nextHop(%d)(0x%x)\n", idx,
			(asic_nxthopv6.isPppoe==TRUE? "pppoe": "ethernet"), asic_nxthopv6.dvid, asic_nxthopv6.pppoeIdx,
			(asic_nxthopv6.nextHopRow<<2)+asic_nxthopv6.nextHopColumn, (asic_nxthopv6.nextHopRow<<2)+asic_nxthopv6.nextHopColumn);
	}
#endif	
	return 0;
}
#else
static int32 nexthop6_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	uint32 idx;
	rtl8198C_tblAsicDrv_nextHopV6Param_t asic_nxthopv6;

	len = sprintf(page, "%s\n", "ASIC Next Hop V6 Table:\n");
	for (idx=0; idx<RTL8198C_NEXTHOPV6TBL_SIZE; idx++) 
	{
		if (rtl8198C_getAsicNextHopTableV6(idx, &asic_nxthopv6) == FAILED)
			continue;
		
		len += sprintf(page+len,"  [%d]  type(%s) dstVid(%d) pppoeIdx(%d) nextHop(%d)(0x%x)\n", idx,
			(asic_nxthopv6.isPppoe==TRUE? "pppoe": "ethernet"), asic_nxthopv6.dvid, asic_nxthopv6.pppoeIdx,
			(asic_nxthopv6.nextHopRow<<2)+asic_nxthopv6.nextHopColumn, (asic_nxthopv6.nextHopRow<<2)+asic_nxthopv6.nextHopColumn);
	}
	
	return len;
}
#endif

static int32 nexthop6_write(struct file *filp, const char *buff, unsigned long len, void *data )
{
	return len;
}

//mcast6--------------------------------------------------------------------------

#ifdef CONFIG_RTL_PROC_NEW
static int32 mcast6_read(struct seq_file *s, void *v)
{
	#ifdef CONFIG_RTL_LAYERED_DRIVER_L3	
	uint32 entry;
	rtl8198C_tblAsicDrv_multiCastv6Param_t asic;

	#if 1
	seq_printf(s, "%s\n", "ASIC Multicast V6 Table:");
	for (entry=0; entry<RTL8198C_MULTICASTV6TBL_SIZE; entry++)
	{
			#if defined(CONFIG_RTL_WTDOG)
			rtl_periodic_watchdog_kick(entry, WATCHDOG_NUM_OF_TIMES);
			#endif
	        if (rtl8198C_getAsicIpMulticastv6Table(entry, &asic) != SUCCESS) {
			continue;
	        } else {
			seq_printf(s,"\t[%d]  (OK)dip(%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x) sip(%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x) mbr(0x%x)\n", entry,
			asic.dip.v6_addr16[0], asic.dip.v6_addr16[1],asic.dip.v6_addr16[2], asic.dip.v6_addr16[3],
			asic.dip.v6_addr16[4], asic.dip.v6_addr16[5],asic.dip.v6_addr16[6], asic.dip.v6_addr16[7],
			asic.sip.v6_addr16[0], asic.sip.v6_addr16[1],asic.sip.v6_addr16[2], asic.sip.v6_addr16[3],
			asic.sip.v6_addr16[4], asic.sip.v6_addr16[5],asic.sip.v6_addr16[6], asic.sip.v6_addr16[7],
			asic.mbr);
			seq_printf(s,"\t       spa:%d, age:%d, cpu:%d, 6rd_eg_en:%d, 6rd_eg_idx:%d\n", asic.port,
			asic.age, asic.cpu, asic.six_rd_eg, asic.six_rd_idx);
	        }
	}

	////rtlglue_printf("\n\t TotalOpCnt:AddMcastOpCnt:%d\tDelMcastOpCnt:%d\tForceAddMcastOpCnt:%d\t \n", _rtl865x_getAddMcastOpCnt(),_rtl865x_getDelMcastOpCnt(),_rtl865x_getForceAddMcastOpCnt());
	#else
	seq_printf(s, "%s\n", "ASIC Multicast Table:");

	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++)
	{
			if (rtl8651_getAsicIpMulticastTable(entry, &asic) != SUCCESS) {
				seq_printf(s,"\t[%d]  (Invalid Entry)\n", entry);
				continue;
			}
			seq_printf(s, "\t[%d]  dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(%x)\n", entry,
				asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff),
				asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
				asic.mbr);
			seq_printf(s,"\t       svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
				asic.age, asic.cpu);
	}
	#endif
	#endif

	return 0;
}
#else
static int32 mcast6_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len=0;
	uint32 entry;
	rtl8198C_tblAsicDrv_multiCastv6Param_t asic;

	#if 1
	len = sprintf(page, "%s\n", "ASIC Multicast V6 Table:");
	for (entry=0; entry<RTL8198C_MULTICASTV6TBL_SIZE; entry++)
	{
			#if defined(CONFIG_RTL_WTDOG)
			rtl_periodic_watchdog_kick(entry, WATCHDOG_NUM_OF_TIMES);
			#endif
	        if (rtl8198C_getAsicIpMulticastv6Table(entry, &asic) != SUCCESS) {
			continue;
	        } else {
			len += sprintf(page+len, "\t[%d]  (OK)dip(%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x) sip(%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x) mbr(0x%x)\n", entry,
			asic.dip.v6_addr16[0], asic.dip.v6_addr16[1],asic.dip.v6_addr16[2], asic.dip.v6_addr16[3],
			asic.dip.v6_addr16[4], asic.dip.v6_addr16[5],asic.dip.v6_addr16[6], asic.dip.v6_addr16[7],
			asic.sip.v6_addr16[0], asic.sip.v6_addr16[1],asic.sip.v6_addr16[2], asic.sip.v6_addr16[3],
			asic.sip.v6_addr16[4], asic.sip.v6_addr16[5],asic.sip.v6_addr16[6], asic.sip.v6_addr16[7],
			asic.mbr);
			len += sprintf(page+len, "\t       spa:%d, age:%d, cpu:%d, 6rd_eg_en:%d, 6rd_eg_idx:%d\n", asic.port,
			asic.age, asic.cpu, asic.six_rd_eg, asic.six_rd_idx);
	        }
	}

	////len += sprintf(page+len, "\n\t TotalOpCnt:AddMcastOpCnt:%d\tDelMcastOpCnt:%d\tForceAddMcastOpCnt:%d\t \n", _rtl865x_getAddMcastOpCnt(),_rtl865x_getDelMcastOpCnt(),_rtl865x_getForceAddMcastOpCnt());
	#else
	len = sprintf(page, "%s\n", "ASIC Multicast Table:");

	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++)
	{
			if (rtl8651_getAsicIpMulticastTable(entry, &asic) != SUCCESS) {
				len +=sprintf(page+len,"\t[%d]  (Invalid Entry)\n", entry);
				continue;
			}
			len += sprintf(page+len, "\t[%d]  dip(%d.%d.%d.%d) sip(%d.%d.%d.%d) mbr(%x)\n", entry,
				asic.dip>>24, (asic.dip&0x00ff0000)>>16, (asic.dip&0x0000ff00)>>8, (asic.dip&0xff),
				asic.sip>>24, (asic.sip&0x00ff0000)>>16, (asic.sip&0x0000ff00)>>8, (asic.sip&0xff),
				asic.mbr);
			len +=sprintf(page+len,"\t       svid:%d, spa:%d, extIP:%d, age:%d, cpu:%d\n", asic.svid, asic.port, asic.extIdx,
				asic.age, asic.cpu);
	}
	#endif

	if (len <= off+count) {
		*eof = 1;
	}

	*start = page + off;
	len -= off;

	if (len > count) {
		len = count;
	}

	if (len < 0) {
	  	len = 0;
	}

	return len;
}
#endif

static int32 mcast6_write(struct file *filp, const char *buff, unsigned long len, void *data )
{
	return len;
}

//6rd--------------------------------------------------------------------------

#ifdef CONFIG_RTL_PROC_NEW
static int32 ip6rd_read(struct seq_file *s, void *v)
{
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3	
	uint32 idx;
	rtl8198C_tblAsicDrv_6rdParam_t asic;
	inv6_addr_t tmp_ip;
	
	seq_printf(s, "%s\n", "ASIC 6RD Table:\n");
	for (idx=0; idx<RTL8198C_6RDTBL_SIZE; idx++) 
	{
		if (rtl8198C_getAsic6rdTable(idx, &asic) == FAILED)
			continue;
		
		ntohl_array((uint32 *)&asic.six_rd_prefix.v6_addr32[0], (void *)&tmp_ip.v6_addr32[0], 4);
		
		seq_printf(s, "  [%d]  ceip(%d.%d.%d.%d)/ceipm:(%d) 6RD prefix=> ::%04x:%04x:%04x:%04x/%d\n", idx,
               			(asic.ce_ip_addr>>24), ((asic.ce_ip_addr&0x00ff0000)>>16), ((asic.ce_ip_addr&0x0000ff00)>>8), (asic.ce_ip_addr&0xff),
                			asic.ce_ip_mask_len, htons(tmp_ip.v6_addr16[0]), htons(tmp_ip.v6_addr16[1]), htons(tmp_ip.v6_addr16[2]), htons(tmp_ip.v6_addr16[3]),
                 			asic.six_rd_prefix_len);
		seq_printf(s, "        brip(%d.%d.%d.%d)/bripm:(%d) 6RD MTU:%d(0x%x) valid:%d\n",
               			(asic.br_ip_addr>>24), ((asic.br_ip_addr&0x00ff0000)>>16), ((asic.br_ip_addr&0x0000ff00)>>8), (asic.br_ip_addr&0xff),
                			asic.br_ip_mask_len, asic.mtu, asic.mtu, asic.valid);
	}
#endif
	return 0;
}
#else
static int32 ip6rd_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	uint32 idx;
	rtl8198C_tblAsicDrv_6rdParam_t asic;

	len = sprintf(page, "%s\n", "ASIC 6RD Table:\n");
	for (idx=0; idx<RTL8198C_6RDTBL_SIZE; idx++) 
	{
		if (rtl8198C_getAsic6rdTable(idx, &asic) == FAILED)
			continue;
		
		len += sprintf(page+len, "  [%d]  ceip(%d.%d.%d.%d)/ceipm:(%d) 6RD prefix=> ::%04x:%04x:%04x:%04x/%d\n", idx,
               			(asic.ce_ip_addr>>24), ((asic.ce_ip_addr&0x00ff0000)>>16), ((asic.ce_ip_addr&0x0000ff00)>>8), (asic.ce_ip_addr&0xff),
                			asic.ce_ip_mask_len,asic.six_rd_prefix.v6_addr16[0], asic.six_rd_prefix.v6_addr16[1], asic.six_rd_prefix.v6_addr16[2], asic.six_rd_prefix.v6_addr16[3],
                 			asic.six_rd_prefix_len);
		len += sprintf(page+len, "        brip(%d.%d.%d.%d)/bripm:(%d) 6RD MTU:%d(0x%x) valid:%d\n",
               			(asic.br_ip_addr>>24), ((asic.br_ip_addr&0x00ff0000)>>16), ((asic.br_ip_addr&0x0000ff00)>>8), (asic.br_ip_addr&0xff),
                			asic.br_ip_mask_len, asic.mtu, asic.mtu, asic.valid);
	}

	return len;
}
#endif

static int32 ip6rd_write(struct file *filp, const char *buff, unsigned long len, void *data )
{
	return len;
}

//dslite--------------------------------------------------------------------------
#ifdef CONFIG_RTL_PROC_NEW
static int32 dslite_read(struct seq_file *s, void *v)
{
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3	
	uint32 idx;
	rtl8198C_tblAsicDrv_dsliteParam_t asic;
	inv6_addr_t tmp_ip;
	
	seq_printf(s, "%s\n", "ASIC DSLITE Table:\n");
	for (idx=0; idx<RTL8198C_DSLITETBL_SIZE; idx++) 
	{
		if (rtl8198C_getAsicDsliteTable(idx, &asic) == FAILED)
			continue;

		memset((void *)&tmp_ip.v6_addr32[0], 0x00, sizeof(tmp_ip));
		ntohl_array((uint32 *)&asic.host_ipv6_addr.v6_addr32[0], (void *)&tmp_ip.v6_addr32[0], 4);

		seq_printf(s,"\t[%d]  HOSTIP/MASK:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d \n", idx,
                    htons(tmp_ip.v6_addr16[0]),htons(tmp_ip.v6_addr16[1]),htons(tmp_ip.v6_addr16[2]),htons(tmp_ip.v6_addr16[3]),
            		htons(tmp_ip.v6_addr16[4]),htons(tmp_ip.v6_addr16[5]),htons(tmp_ip.v6_addr16[6]),htons(tmp_ip.v6_addr16[7]),
			        asic.host_ipv6_mask);
		memset((void *)&tmp_ip.v6_addr32[0], 0x00, sizeof(tmp_ip));
		ntohl_array((uint32 *)&asic.aftr_ipv6_addr.v6_addr32[0], (void *)&tmp_ip.v6_addr32[0], 4);
		
		seq_printf(s,"\t      AFTRIP/MASK:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d MTU:%d(0x%x) valid:%d\n",
                     htons(tmp_ip.v6_addr16[0]), htons(tmp_ip.v6_addr16[1]), htons(tmp_ip.v6_addr16[2]), htons(tmp_ip.v6_addr16[3]),
            		 htons(tmp_ip.v6_addr16[4]), htons(tmp_ip.v6_addr16[5]), htons(tmp_ip.v6_addr16[6]), htons(tmp_ip.v6_addr16[7]),
			        asic.aftr_ipv6_mask,asic.mtu,asic.mtu,asic.valid);
	}
#endif	
	return 0;
}
#else
static int32 dslite_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len=0;
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3	
	uint32 idx;
	rtl8198C_tblAsicDrv_dsliteParam_t asic;
	inv6_addr_t tmp_ip;
	
	len = sprintf(page, "%s\n", "ASIC DSLITE Table:\n");
	for (idx=0; idx<RTL8198C_DSLITETBL_SIZE; idx++) 
	{
		if (rtl8198C_getAsicDsliteTable(idx, &asic) == FAILED)
			continue;
		
		memset((void *)&tmp_ip.v6_addr32[0], 0x00, sizeof(tmp_ip));
		ntohl_array((uint32 *)&asic.host_ipv6_addr.v6_addr32[0], (void *)&tmp_ip.v6_addr32[0], 4);
		
		len += sprintf(page + len,"\t[%d]  HOSTIP/MASK:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d \n", idx,
					htons(tmp_ip.v6_addr16[0]),htons(tmp_ip.v6_addr16[1]),htons(tmp_ip.v6_addr16[2]),htons(tmp_ip.v6_addr16[3]),
					htons(tmp_ip.v6_addr16[4]),htons(tmp_ip.v6_addr16[5]),htons(tmp_ip.v6_addr16[6]),htons(tmp_ip.v6_addr16[7]),
			        asic.host_ipv6_mask);
		
		memset((void *)&tmp_ip.v6_addr32[0], 0x00, sizeof(tmp_ip));
		ntohl_array((uint32 *)&asic.aftr_ipv6_addr.v6_addr32[0], (void *)&tmp_ip.v6_addr32[0], 4);
		
		len += sprintf(page + len,"\t      AFTRIP/MASK:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d MTU:%d(0x%x) valid:%d\n",
					htons(tmp_ip.v6_addr16[0]), htons(tmp_ip.v6_addr16[1]), htons(tmp_ip.v6_addr16[2]), htons(tmp_ip.v6_addr16[3]),
					htons(tmp_ip.v6_addr16[4]), htons(tmp_ip.v6_addr16[5]), htons(tmp_ip.v6_addr16[6]), htons(tmp_ip.v6_addr16[7]),
			        asic.aftr_ipv6_mask,asic.mtu,asic.mtu,asic.valid);
	}
#endif
	return len;
}
#endif

static int32 dslite_write(struct file *filp, const char *buff, unsigned long len, void *data )
{
	return len;
}
//sw_dslite--------------------------------------------------------------------------
#if defined(CONFIG_RTL_PROC_NEW) && defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
static int32 sw_dslite_read(struct seq_file *s, void *v)
{
	uint32 idx;
	rtl865x_dslite_s entry;
	
	seq_printf(s, "%s\n", "SW DSLITE Table:\n");
	for (idx=0; idx<RTL8198C_DSLITETBL_SIZE; idx++) 
	{
		if (_rtl865x_getIpv6DsLiteEntrybyIndex(idx,&entry) == FAILED)
			continue;
		seq_printf(s,"\t[%d]  HOSTIP/MASK:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d \n", idx,
                    entry.host_ipv6_addr.v6_addr16[0],entry.host_ipv6_addr.v6_addr16[1],entry.host_ipv6_addr.v6_addr16[2],entry.host_ipv6_addr.v6_addr16[3],
            		entry.host_ipv6_addr.v6_addr16[4],entry.host_ipv6_addr.v6_addr16[5],entry.host_ipv6_addr.v6_addr16[6],entry.host_ipv6_addr.v6_addr16[7],
			        entry.host_ipv6_mask);
		seq_printf(s,"\t      AFTRIP/MASK:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x/%d MTU:%d(0x%x) valid:%d\n",
                    entry.aftr_ipv6_addr.v6_addr16[0],entry.aftr_ipv6_addr.v6_addr16[1],entry.aftr_ipv6_addr.v6_addr16[2],entry.aftr_ipv6_addr.v6_addr16[3],
            		entry.aftr_ipv6_addr.v6_addr16[4],entry.aftr_ipv6_addr.v6_addr16[5],entry.aftr_ipv6_addr.v6_addr16[6],entry.aftr_ipv6_addr.v6_addr16[7],
			        entry.aftr_ipv6_mask,entry.mtu,entry.mtu,entry.valid);
	}
	return 0;
}
#else
static int32 sw_dslite_read(struct seq_file *s, void *v)
{
	return 0;
}
#endif

static int32 sw_dslite_write(struct file *filp, const char *buff, unsigned long len, void *data )
{
	return len;
}

#endif////end of CONFIG_RTL_8198C
#endif



#ifdef CONFIG_RTL_PROC_NEW
extern struct proc_dir_entry proc_root;
/*stats*/
int stats_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, stats_debug_entry_read, NULL));
}
static ssize_t stats_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return stats_debug_entry_write(file, userbuf,count, off);
}
struct file_operations stats_proc_fops= {
        .open           = stats_single_open,
        .write		    = stats_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#ifdef CONFIG_RTL865X_ROMEPERF
/*perf_dump*/
int perf_dump_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl865x_perf_proc_read, NULL));
}
static ssize_t perf_dump_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return rtl865x_perf_proc_write(file, userbuf,count, off);
}
struct file_operations perf_dump_proc_fops= {
        .open           = perf_dump_single_open,
        .write		    = perf_dump_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif
#ifdef CONFIG_RTL_PROC_DEBUG
/*netif*/
int netif_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, netif_read, NULL));
}
static ssize_t netif_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return netif_write(file, userbuf,count, off);
}
struct file_operations netif_proc_fops= {
        .open           = netif_single_open,
        .write		    = netif_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*netif*/
int sw_netif_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, sw_netif_read, NULL));
}
static ssize_t sw_netif_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return sw_netif_write(file, userbuf,count, off);
}
struct file_operations sw_netif_proc_fops= {
        .open           = sw_netif_single_open,
        .write		    = sw_netif_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*acl*/
//int acl_single_open(struct inode *inode, struct file *file)
//{
//        return(single_open(file, acl_show, NULL));
//}
//static ssize_t acl_single_write(struct file * file, const char __user * userbuf,
//		     size_t count, loff_t * off)
//{
//	    return acl_write(file, userbuf,count, off);
//}
struct file_operations acl_proc_fops= {
        .open           = acl_single_open,
//        .write		    = acl_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*advRt*/
#if defined(CONFIG_RTL_MULTIPLE_WAN)
int advRt_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, advRt_read, NULL));
}
static ssize_t advRt_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return advRt_write(file, userbuf,count, off);
}
struct file_operations advRt_proc_fops= {
        .open           = advRt_single_open,
        .write		    = advRt_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif
/*storm_control*/
int storm_control_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, storm_read, NULL));
}
static ssize_t storm_control_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return storm_write(file, userbuf,count, off);
}
struct file_operations storm_control_proc_fops= {
        .open           = storm_control_single_open,
        .write		    = storm_control_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*hs*/
int hs_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, hs_read, NULL));
}
static ssize_t hs_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return hs_write(file, userbuf,count, off);
}
struct file_operations hs_proc_fops= {
        .open           = hs_single_open,
        .write		    = hs_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*pvid*/
int pvid_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, pvid_read, NULL));
}
static ssize_t pvid_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return pvid_write(file, userbuf,count, off);
}
struct file_operations pvid_proc_fops= {
        .open           = pvid_single_open,
        .write		    = pvid_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*mirrorPort*/
int mirrorPort_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, mirrorPort_read, NULL));
}
static ssize_t mirrorPort_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return mirrorPort_write(file, userbuf,count, off);
}
struct file_operations mirrorPort_proc_fops= {
        .open           = mirrorPort_single_open,
        .write		    = mirrorPort_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*l2*/
int l2_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, l2_read, NULL));
}
static ssize_t l2_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return l2_write(file, userbuf,count, off);
}
struct file_operations l2_proc_fops= {
        .open           = l2_single_open,
        .write		    = l2_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*hwMCast*/
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
int hwMCast_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl865x_proc_hw_mcast_read, NULL));
}
static ssize_t hwMCast_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return rtl865x_proc_hw_mcast_write(file, userbuf,count, off);
}
struct file_operations hwMCast_proc_fops= {
        .open           = hwMCast_single_open,
        .write		    = hwMCast_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*swMCast*/
int swMCast_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl865x_proc_sw_mcast_read, NULL));
}
static ssize_t swMCast_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return rtl865x_proc_sw_mcast_write(file, userbuf,count, off);
}
struct file_operations swMCast_proc_fops= {
        .open           = swMCast_single_open,
        .write		    = swMCast_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
/*arp*/
int arp_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, arp_read, NULL));
}
static ssize_t arp_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return arp_write(file, userbuf,count, off);
}
struct file_operations arp_proc_fops= {
        .open           = arp_single_open,
        .write		    = arp_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*nexthop*/
int nexthop_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, nexthop_read, NULL));
}
static ssize_t nexthop_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return nexthop_write(file, userbuf,count, off);
}
struct file_operations nexthop_proc_fops= {
        .open           = nexthop_single_open,
        .write		    = nexthop_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

extern int32 sw_nexthop_read(struct seq_file *s, void *v);
extern int32 sw_nexthop_write( struct file *filp, const char *buff,unsigned long len, void *data );

int sw_nexthop_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, sw_nexthop_read, NULL));
}
static ssize_t sw_nexthop_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return sw_nexthop_write(file, userbuf,count, off);
}
struct file_operations sw_nexthop_proc_fops= {
        .open           = sw_nexthop_single_open,
        .write		    = sw_nexthop_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*l3*/
int l3_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, l3_read, NULL));
}
static ssize_t l3_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return l3_write(file, userbuf,count, off);
}
struct file_operations l3_proc_fops= {
        .open           = l3_single_open,
        .write		    = l3_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*sw_l3*/
extern int32 sw_l3_read(struct seq_file *s, void *v);
extern int32 sw_l3_write( struct file *filp, const char *buff,unsigned long len, void *data );

int sw_l3_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, sw_l3_read, NULL));
}
static ssize_t sw_l3_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return sw_l3_write(file, userbuf,count, off);
}
struct file_operations sw_l3_proc_fops= {
        .open           = sw_l3_single_open,
        .write		    = sw_l3_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*ip*/
int ip_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, ip_read, NULL));
}
static ssize_t ip_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return ip_write(file, userbuf,count, off);
}
struct file_operations ip_proc_fops= {
        .open           = ip_single_open,
        .write		    = ip_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*pppoe*/
int pppoe_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, pppoe_read, NULL));
}
static ssize_t pppoe_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return pppoe_write(file, userbuf,count, off);
}
struct file_operations pppoe_proc_fops= {
        .open           = pppoe_single_open,
        .write		    = pppoe_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif
#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
/*sw_l2*/
int sw_l2_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl865x_sw_l2_proc_read, NULL));
}
static ssize_t sw_l2_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return rtl865x_sw_l2_proc_write(file, userbuf,count, off);
}
struct file_operations sw_l2_proc_fops= {
        .open           = sw_l2_single_open,
        .write		    = sw_l2_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif
/*port_bandwidth*/
int port_bandwidth_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, port_bandwidth_read, NULL));
}
static ssize_t port_bandwidth_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return port_bandwidth_write(file, userbuf,count, off);
}
struct file_operations port_bandwidth_proc_fops= {
        .open           = port_bandwidth_single_open,
        .write		    = port_bandwidth_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*queue_bandwidth*/
int queue_bandwidth_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, queue_bandwidth_read, NULL));
}
static ssize_t queue_bandwidth_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return queue_bandwidth_write(file, userbuf,count, off);
}
struct file_operations queue_bandwidth_proc_fops= {
        .open           = queue_bandwidth_single_open,
        .write		    = queue_bandwidth_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*priority_decision*/
int priority_decision_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, priority_decision_read, NULL));
}
static ssize_t priority_decision_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return priority_decision_write(file, userbuf,count, off);
}
struct file_operations priority_decision_proc_fops= {
        .open           = priority_decision_single_open,
        .write		    = priority_decision_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#if defined (CONFIG_RTL_ENABLE_RATELIMIT_TABLE)
/*rateLimit*/
int rateLimit_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rate_limit_read, NULL));
}
static ssize_t rateLimit_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return rate_limit_write(file, userbuf,count, off);
}
struct file_operations rateLimit_proc_fops= {
        .open           = rateLimit_single_open,
        .write		    = rateLimit_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif
#if defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL865X_EVENT_PROC_DEBUG)
/*eventMgr*/
int eventMgr_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl865x_event_proc_read, NULL));
}
static ssize_t eventMgr_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return rtl865x_event_proc_write(file, userbuf,count, off);
}
struct file_operations eventMgr_proc_fops= {
        .open           = eventMgr_single_open,
        .write		    = eventMgr_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif
#endif

#if defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)
/*privSkbInfo*/
int privSkbInfo_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl819x_proc_privSkbInfo_read, NULL));
}
static ssize_t privSkbInfo_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return rtl819x_proc_privSkbInfo_write(file, userbuf,count, off);
}
struct file_operations privSkbInfo_proc_fops= {
        .open           = privSkbInfo_single_open,
        .write		    = privSkbInfo_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*memory*/
int memory_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, proc_mem_read, NULL));
}
static ssize_t memory_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return proc_mem_write(file, userbuf,count, off);
}
struct file_operations memory_proc_fops= {
        .open           = memory_single_open,
        .write		    = memory_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*diagnostic*/
int diagnostic_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, diagnostic_read, NULL));
}
static ssize_t diagnostic_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return diagnostic_write(file, userbuf,count, off);
}
struct file_operations diagnostic_proc_fops= {
        .open           = diagnostic_single_open,
        .write		    = diagnostic_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*tx queue*/
#if defined(CONFIG_RTL_NIC_QUEUE)
int tx_queue_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, tx_queue_read, NULL));
}
static ssize_t tx_queue_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return tx_queue_write(file, userbuf,count, off);
}
struct file_operations tx_queue_proc_fops= {
        .open           = tx_queue_single_open,
        .write		    = tx_queue_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif
/*port_status*/
int port_status_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, port_status_read, NULL));
}
static ssize_t port_status_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return port_status_write(file, userbuf,count, off);
}
struct file_operations port_status_proc_fops= {
        .open           = port_status_single_open,
        .write		    = port_status_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*phyReg*/
int phyReg_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, proc_phyReg_read, NULL));
}
static ssize_t phyReg_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return proc_phyReg_write(file, userbuf,count, off);
}
struct file_operations phyReg_proc_fops= {
        .open           = phyReg_single_open,
        .write		    = phyReg_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*asicCounter*/
int asicCounter_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl865x_proc_mibCounter_read, NULL));
}
static ssize_t asicCounter_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return rtl865x_proc_mibCounter_write(file, userbuf,count, off);
}
struct file_operations asicCounter_proc_fops= {
        .open           = asicCounter_single_open,
        .write		    = asicCounter_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*mmd*/
int mmd_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, proc_mmd_read, NULL));
}
static ssize_t mmd_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return proc_mmd_write(file, userbuf,count, off);
}
struct file_operations mmd_proc_fops= {
        .open           = mmd_single_open,
        .write		    = mmd_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*mac*/
int mac_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, mac_config_read, NULL));
}
static ssize_t mac_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return mac_config_write(file, userbuf,count, off);
}
struct file_operations mac_proc_fops= {
        .open           = mac_single_open,
        .write		    = mac_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*fc_threshold*/
int fc_threshold_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, fc_threshold_read, NULL));
}
static ssize_t fc_threshold_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return fc_threshold_write(file, userbuf,count, off);
}
struct file_operations fc_threshold_proc_fops= {
        .open           = fc_threshold_single_open,
        .write		    = fc_threshold_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

#if defined (CONFIG_RTL_INBAND_CTL_API)
int portRate_single_open(struct inode *inode, struct file *file)
{
    return(single_open(file, rtl865x_proc_portRate_read, NULL));
}
static ssize_t portRate_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return rtl865x_proc_portRate_write(file, userbuf,count, off);
}
struct file_operations portRate_proc_fops= {
        .open           = portRate_single_open,
        .write		    = portRate_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT)
/*l3v6*/
int l3v6_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, l3v6_read, NULL));
}
static ssize_t l3v6_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return l3v6_write(file, userbuf,count, off);
}
struct file_operations l3v6_proc_fops= {
        .open           = l3v6_single_open,
        .write		    = l3v6_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*arp6*/
int arp6_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, arp6_read, NULL));
}
static ssize_t arp6_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return arp6_write(file, userbuf,count, off);
}
struct file_operations arp6_proc_fops= {
        .open           = arp6_single_open,
        .write		    = arp6_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*sw_arp6*/
int sw_arp6_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, sw_arp6_read, NULL));
}
static ssize_t sw_arp6_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	   return sw_arp6_write(file, userbuf,count, off);
}
struct file_operations sw_arp6_proc_fops= {
        .open           = sw_arp6_single_open,
        .write		    = sw_arp6_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*nexthop6*/
int nexthop6_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, nexthop6_read, NULL));
}
static ssize_t nexthop6_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return nexthop6_write(file, userbuf,count, off);
}
struct file_operations nexthop6_proc_fops= {
        .open           = nexthop6_single_open,
        .write		    = nexthop6_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*mcast6*/
int mcast6_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, mcast6_read, NULL));
}
static ssize_t mcast6_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return mcast6_write(file, userbuf,count, off);
}
struct file_operations mcast6_proc_fops= {
        .open           = mcast6_single_open,
        .write		    = mcast6_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*6rd*/
int sixrd_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, ip6rd_read, NULL));
}
static ssize_t sixrd_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return ip6rd_write(file, userbuf,count, off);
}
struct file_operations sixrd_proc_fops= {
        .open           = sixrd_single_open,
        .write		    = sixrd_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
/*dslite*/
int dslite_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, dslite_read, NULL));
}
static ssize_t dslite_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return dslite_write(file, userbuf,count, off);
}
struct file_operations dslite_proc_fops= {
        .open           = dslite_single_open,
        .write		    = dslite_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

/*sw_dslite*/
int sw_dslite_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, sw_dslite_read, NULL));
}
static ssize_t sw_dslite_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return sw_dslite_write(file, userbuf,count, off);
}
struct file_operations sw_dslite_proc_fops= {
        .open           = sw_dslite_single_open,
        .write		    = sw_dslite_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif
#endif

#else	// CONFIG_RTL_PROC_NEW

	#ifdef CONFIG_RTL_PROC_DEBUG
	extern int32 sw_nexthop_read( char *page, char **start, off_t off, int count, int *eof, void *data );
	extern int32 sw_nexthop_write( struct file *filp, const char *buff,unsigned long len, void *data );
	extern int32 sw_l3_read( char *page, char **start, off_t off, int count, int *eof, void *data );
	extern int32 sw_l3_write( struct file *filp, const char *buff,unsigned long len, void *data );	
	#endif
	
#endif	// CONFIG_RTL_PROC_NEW

#if (defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)) && defined(CONFIG_OPENWRT_SDK) //mark_bb

static int32 vlan_8367_pvid_single_show(struct seq_file *s, void *v)
{
	uint32 vidp[9],pPriority;
	int32  i;

	for(i=0; i<8; i++)
	{
		if (rtk_vlan_portPvid_get(i, &vidp[i],&pPriority) != SUCCESS)
		{
			seq_printf(s,"%s\n", "ASIC PVID get failed.");
		}
	}
	seq_printf(s,"%s\n", ">> PVID Reg:");
	for(i=0; i<8; i++)
		seq_printf(s,"p%d: %d,", i, vidp[i]);

	seq_printf(s,"%s\n", ".");

	return 0;

}
static int32 vlan_8367_single_show(struct seq_file *s, void *v)

{
	//seq_printf(s,"%s\n", "ASIC VLAN Table:");
	rtlglue_printf("%s\n", "ASIC VLAN Table:");
	{
		int i, j;
		
		for ( i = 0; i < 4095; i++ ) 
		{
			rtl865x_tblAsicDrv_vlanParam_t vlan;
			unsigned int pMbrmsk,pUntagmsk,pFid;

			if ( rtk_vlan_get( i,&pMbrmsk , &pUntagmsk,&pFid ) == FAILED )
				continue;

			if(pMbrmsk == 0)
				continue;
			
			vlan.memberPortMask = pMbrmsk;
			vlan.untagPortMask = pUntagmsk;
			vlan.fid = pFid ;
			
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
			rtlglue_printf( "  VID[%d] ",i);
#else
			vlan.vid = (unsigned short)i ;
			rtlglue_printf( "  VID[%d] ",vlan.vid);
#endif			
			rtlglue_printf("\n\tmember ports:");

			for( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if ( vlan.memberPortMask & ( 1 << j ) )
					rtlglue_printf("%d ", j);
			}

			rtlglue_printf("\n\tUntag member ports:");

			for( j = 0; j < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum; j++ )
			{
				if( vlan.untagPortMask & ( 1 << j ) )
					rtlglue_printf("%d ", j);
			}

			rtlglue_printf("\n\tFID:\t%d\n",vlan.fid);
		}

	}

	return 0;
}
static int vlan_8367_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, vlan_8367_single_show, NULL));
}

static int vlan_8367_pvid_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, vlan_8367_pvid_single_show, NULL));
}

struct file_operations vlan_8367_single_seq_file_operations = {
        .open           = vlan_8367_single_open,	 
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

struct file_operations vlan_8367_pvid_single_seq_file_operations = {
        .open           = vlan_8367_pvid_single_open,	 
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

int32 rtl865x_proc_debug_init(void)
{
	int32 retval;

#if defined(CONFIG_RTL_LOG_DEBUG)
	extern void log_print_proc_init(void);
	log_print_proc_init();
#endif
#ifdef CONFIG_RTL_PROC_NEW
	rtl865x_proc_dir = proc_mkdir(RTL865X_PROC_DIR_NAME,&proc_root);
	retval = SUCCESS;
#else
	rtl865x_proc_dir = proc_mkdir(RTL865X_PROC_DIR_NAME,NULL);
#endif
	if(rtl865x_proc_dir)
	{
		#ifdef CONFIG_RTL_PROC_DEBUG
#if !defined(CONFIG_RTL_8198C) &&  !defined(CONFIG_RTL_8197F) /* "/proc/rtl865x/stats" is duplicated */
		/*stats*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("stats",0,rtl865x_proc_dir,&stats_proc_fops,NULL);
#else
			stats_debug_entry = create_proc_entry("stats",0,rtl865x_proc_dir);
			if(stats_debug_entry != NULL)
			{
				stats_debug_entry->read_proc = stats_debug_entry_read;
				stats_debug_entry->write_proc= stats_debug_entry_write;
		
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for stats");
				retval = FAILED;
				goto out;
			}
#endif
		}
#endif
		/*vlan*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("vlan",0,rtl865x_proc_dir,&vlan_single_seq_file_operations,NULL);
#else
			vlan_entry = create_proc_entry("vlan",0,rtl865x_proc_dir);
			if(vlan_entry != NULL)
			{
				//vlan_entry->read_proc = vlan_read;
				//vlan_entry->write_proc= vlan_write;
				vlan_entry->proc_fops = &vlan_single_seq_file_operations;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for vlan");
				retval = FAILED;
				goto out;
			}
#endif
		}

		/*netif*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("netif",0,rtl865x_proc_dir,&netif_proc_fops,NULL);
#else
			netif_entry = create_proc_entry("netif",0,rtl865x_proc_dir);
			if(netif_entry != NULL)
			{
				netif_entry->read_proc = netif_read;
				netif_entry->write_proc= netif_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for netif");
				retval = FAILED;
				goto out;
			}
#endif
		}
		/*swnetif*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("sw_netif",0,rtl865x_proc_dir,&sw_netif_proc_fops,NULL);
#else
			sw_netif_entry = create_proc_entry("sw_netif",0,rtl865x_proc_dir);
			if(sw_netif_entry != NULL)
			{
				sw_netif_entry->read_proc = sw_netif_read;
				sw_netif_entry->write_proc= sw_netif_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for sw netif");
				retval = FAILED;
				goto out;
			}
#endif
		}
		
#if (defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)) && defined(CONFIG_OPENWRT_SDK) //mark_bb
		{
			 proc_create_data("8367_vlan",0,rtl865x_proc_dir, &vlan_8367_single_seq_file_operations,NULL);

		}
		/*pvid*/
		{
			proc_create_data("8367_pvid",0,rtl865x_proc_dir, &vlan_8367_pvid_single_seq_file_operations,NULL);			
			
		}
#endif
		/*acl*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("acl",0,rtl865x_proc_dir,&acl_proc_fops,NULL);
#else
			acl_entry = create_proc_entry("acl",0,rtl865x_proc_dir);
			if(acl_entry != NULL)
			{
				/*acl_entry->read_proc = acl_read;
				acl_entry->write_proc= acl_write;
				acl_entry->owner = THIS_MODULE;
				*/
				acl_entry->proc_fops = &acl_single_seq_file_operations;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for acl");
				retval = FAILED;
				goto out;
			}
#endif
		}
		#if defined(CONFIG_RTL_MULTIPLE_WAN)
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("advRt",0,rtl865x_proc_dir,&advRt_proc_fops,NULL);
#else
			advRt_entry = create_proc_entry("advRt",0,rtl865x_proc_dir);
			if(advRt_entry != NULL)
			{
				advRt_entry->read_proc = advRt_read;
				advRt_entry->write_proc= advRt_write;
				//advRt_entry->owner = THIS_MODULE;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for advRt");
				retval = FAILED;
				goto out;
			}
#endif
		}
		#endif

		/*storm control*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("storm_control",0,rtl865x_proc_dir,&storm_control_proc_fops,NULL);
#else
			storm_control = create_proc_entry("storm_control",0,rtl865x_proc_dir);
			if(storm_control != NULL)
			{
				storm_control->read_proc = storm_read;
				storm_control->write_proc= storm_write;
			}
			else
			{
				rtlglue_printf("can't create proc entry for storm control");
				retval = FAILED;
				goto out;
			}
#endif
		}

#ifdef CONFIG_RTL_LAYERED_DRIVER
		/*soft acl chains*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("soft_aclChains",0,rtl865x_proc_dir,&aclChains_single_seq_file_operations,NULL);
#else
			acl_chains_entry = create_proc_entry("soft_aclChains",0,rtl865x_proc_dir);
			if(acl_chains_entry != NULL)
			{
				acl_chains_entry->proc_fops = &aclChains_single_seq_file_operations;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for aclChains");
				retval = FAILED;
				goto out;
			}
#endif
		}
#endif

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
		/*qos rules*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("soft_qosRules",0,rtl865x_proc_dir,&qosRule_single_seq_file_operations,NULL);
#else
			qos_rule_entry = create_proc_entry("soft_qosRules",0,rtl865x_proc_dir);
			if(qos_rule_entry != NULL)
			{
				/*acl_entry->read_proc = acl_read;
				acl_entry->write_proc= acl_write;
				acl_entry->owner = THIS_MODULE;
				*/
				qos_rule_entry->proc_fops = &qosRule_single_seq_file_operations;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for aclChains");
				retval = FAILED;
				goto out;
			}
#endif
		}
#endif
		/*hs*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("hs",0,rtl865x_proc_dir,&hs_proc_fops,NULL);
#else
			hs_entry = create_proc_entry("hs",0,rtl865x_proc_dir);
			if(hs_entry != NULL)
			{
				hs_entry->read_proc = hs_read;
				hs_entry->write_proc= hs_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for hs");
				retval = FAILED;
				goto out;
			}
#endif
		}
#if defined(RTL_DEBUG_NIC_SKB_BUFFER)
		/*nic mbuf*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("nic_mbuf",0,rtl865x_proc_dir,&nic_mbuf_single_seq_file_operations,NULL);
#else
			nic_skb_buff = create_proc_entry("nic_mbuf",0,rtl865x_proc_dir);
			if(nic_skb_buff != NULL)
			{
				nic_skb_buff->proc_fops = &nic_mbuf_single_seq_file_operations;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create nic_mbuf entry for hs");
				retval = FAILED;
				goto out;
			}
#endif
		}
#endif
		/*rx ring*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("rxRing",0,rtl865x_proc_dir,&rxRing_single_seq_file_operations,NULL);
#else
			rxRing_entry = create_proc_entry("rxRing",0,rtl865x_proc_dir);
			if(rxRing_entry != NULL)
			{
				rxRing_entry->proc_fops = &rxRing_single_seq_file_operations;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create rxRing entry for hs");
				retval = FAILED;
				goto out;
			}
#endif
		}

		/*tx ring*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("txRing",0,rtl865x_proc_dir,&txRing_single_seq_file_operations,NULL);
#else
			txRing_entry = create_proc_entry("txRing",0,rtl865x_proc_dir);
			if(txRing_entry != NULL)
			{
				txRing_entry->proc_fops = &txRing_single_seq_file_operations;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create txRing entry for hs");
				retval = FAILED;
				goto out;
			}
#endif
		}

		/*mbufRing*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("mbufRing",0,rtl865x_proc_dir,&mbuf_single_seq_file_operations,NULL);
#else
			mbuf_entry = create_proc_entry("mbufRing",0,rtl865x_proc_dir);
			if(mbuf_entry != NULL)
			{
				mbuf_entry->proc_fops = &mbuf_single_seq_file_operations;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create mbuf entry for mbufRing");
				retval = FAILED;
				goto out;
			}
#endif
		}

		/*pvid*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("pvid",0,rtl865x_proc_dir,&pvid_proc_fops,NULL);
#else
			pvid_entry = create_proc_entry("pvid",0,rtl865x_proc_dir);
			if(pvid_entry != NULL)
			{
				pvid_entry->read_proc = pvid_read;
				pvid_entry->write_proc= pvid_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for pvid");
				retval = FAILED;
				goto out;
			}
#endif
		}

			/*mirrorPort*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("mirrorPort",0,rtl865x_proc_dir,&mirrorPort_proc_fops,NULL);
#else
			mirrorPort_entry = create_proc_entry("mirrorPort",0,rtl865x_proc_dir);
			if(mirrorPort_entry != NULL)
			{
				mirrorPort_entry->read_proc = mirrorPort_read;
				mirrorPort_entry->write_proc= mirrorPort_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for mirrorPort");
				retval = FAILED;
				goto out;
			}
#endif
		}

		/*l2*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("l2",0,rtl865x_proc_dir,&l2_proc_fops,NULL);
#else
			l2_entry = create_proc_entry("l2", 0, rtl865x_proc_dir);
			if(l2_entry != NULL)
			{
				l2_entry->read_proc = l2_read;
				l2_entry->write_proc = l2_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for l2");
				retval = FAILED;
				goto out;
			}
#endif
		}

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
        {
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("hwMCast",0,rtl865x_proc_dir,&hwMCast_proc_fops,NULL);
#else
            hwMCast_entry=create_proc_entry("hwMCast", 0, rtl865x_proc_dir);
            if(hwMCast_entry != NULL)
            {
                hwMCast_entry->read_proc = rtl865x_proc_hw_mcast_read;
                hwMCast_entry->write_proc = rtl865x_proc_hw_mcast_write;
                retval = SUCCESS;
            }
#endif
        }

        {
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("swMCast",0,rtl865x_proc_dir,&swMCast_proc_fops,NULL);
#else
            swMCast_entry=create_proc_entry("swMCast", 0, rtl865x_proc_dir);
            if(swMCast_entry != NULL)
            {
                swMCast_entry->read_proc = rtl865x_proc_sw_mcast_read;
                swMCast_entry->write_proc = rtl865x_proc_sw_mcast_write;
                retval = SUCCESS;
            }
#endif
        }
#endif


#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
		/*arp*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("arp",0,rtl865x_proc_dir,&arp_proc_fops,NULL);
#else
			arp_entry = create_proc_entry("arp", 0, rtl865x_proc_dir);
			if(arp_entry != NULL)
			{
				arp_entry->read_proc = arp_read;
				arp_entry->write_proc = arp_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for arp");
				retval = FAILED;
				goto out;
			}
#endif
		}

		/*nextHop*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("nexthop",0,rtl865x_proc_dir,&nexthop_proc_fops,NULL);
#else
			nexthop_entry= create_proc_entry("nexthop", 0, rtl865x_proc_dir);
			if(nexthop_entry != NULL)
			{
				nexthop_entry->read_proc = nexthop_read;
				nexthop_entry->write_proc = nexthop_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for nexthop");
				retval = FAILED;
				goto out;
			}
#endif
		}
		/*sw nextHop*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("sw_nexthop",0,rtl865x_proc_dir,&sw_nexthop_proc_fops,NULL);
#else
			sw_nexthop_entry= create_proc_entry("sw_nexthop", 0, rtl865x_proc_dir);
			if(sw_nexthop_entry != NULL)
			{
				sw_nexthop_entry->read_proc = sw_nexthop_read;
				sw_nexthop_entry->write_proc = sw_nexthop_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for nexthop");
				retval = FAILED;
				goto out;
			}
#endif
		}
		/*l3*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("l3",0,rtl865x_proc_dir,&l3_proc_fops,NULL);
#else
			l3_entry= create_proc_entry("l3", 0, rtl865x_proc_dir);
			if(l3_entry != NULL)
			{
				l3_entry->read_proc = l3_read;
				l3_entry->write_proc = l3_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for l3");
				retval = FAILED;
				goto out;
			}
#endif
		}
		/*sw l3*/
		{
#ifdef CONFIG_RTL_PROC_NEW
			proc_create_data("sw_l3",0,rtl865x_proc_dir,&sw_l3_proc_fops,NULL);
#else
			sw_l3_entry= create_proc_entry("sw_l3", 0, rtl865x_proc_dir);
			if(sw_l3_entry != NULL)
			{
				sw_l3_entry->read_proc = sw_l3_read;
				sw_l3_entry->write_proc = sw_l3_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for l3");
				retval = FAILED;
				goto out;
			}
#endif
		}

		/*ip*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("ip",0,rtl865x_proc_dir,&ip_proc_fops,NULL);
#else
			ip_entry= create_proc_entry("ip", 0, rtl865x_proc_dir);
			if(ip_entry != NULL)
			{
				ip_entry->read_proc = ip_read;
				ip_entry->write_proc = ip_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for ip");
				retval = FAILED;
				goto out;
			}
#endif
		}

		/*pppoe*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("pppoe",0,rtl865x_proc_dir,&pppoe_proc_fops,NULL);
#else
			pppoe_entry= create_proc_entry("pppoe", 0, rtl865x_proc_dir);
			if(pppoe_entry != NULL)
			{
				pppoe_entry->read_proc = pppoe_read;
				pppoe_entry->write_proc = pppoe_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for pppoe");
				retval = FAILED;
				goto out;
			}
#endif
		}

#endif
#if defined(CONFIG_RTL_LAYERED_DRIVER_L4)
		/*napt*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("napt",0,rtl865x_proc_dir,&napt_single_seq_file_operations,NULL);
#else
			napt_entry= create_proc_entry("napt", 0, rtl865x_proc_dir);
			if(napt_entry != NULL)
			{
				#if 1
				napt_entry->proc_fops = &napt_single_seq_file_operations;
				#else
				napt_entry->read_proc = napt_read;
				napt_entry->write_proc = napt_write;
				napt_entry->owner = THIS_MODULE;
				#endif
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for napt");
				retval = FAILED;
				goto out;
			}
#endif
		}

		/*software napt*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("sw_napt",0,rtl865x_proc_dir,&sw_napt_single_seq_file_operations,NULL);
#else
			sw_napt_entry= create_proc_entry("sw_napt", 0, rtl865x_proc_dir);
			if(sw_napt_entry != NULL)
			{
				#ifdef CONFIG_RTL_LAYERED_DRIVER
				sw_napt_entry->proc_fops= &sw_napt_single_seq_file_operations;
				#else
				sw_napt_entry->read_proc = rtl865x_sw_napt_proc_read;
				sw_napt_entry->write_proc = rtl865x_sw_napt_proc_write;
				#endif
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for sw_napt");
				retval = FAILED;
				goto out;
			}
#endif
		}
#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
		/*software l2*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("sw_l2",0,rtl865x_proc_dir,&sw_l2_proc_fops,NULL);
#else
			sw_l2_entry= create_proc_entry("sw_l2", 0, rtl865x_proc_dir);
			if(sw_l2_entry != NULL)
			{
				sw_l2_entry->read_proc = rtl865x_sw_l2_proc_read;
				sw_l2_entry->write_proc = rtl865x_sw_l2_proc_write;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for sw_l2");
				retval = FAILED;
				goto out;
			}
#endif
		}
#endif

		/*port_bandwidth*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("port_bandwidth",0,rtl865x_proc_dir,&port_bandwidth_proc_fops,NULL);
#else
			port_bandwidth_entry= create_proc_entry("port_bandwidth", 0, rtl865x_proc_dir);
			if(port_bandwidth_entry != NULL)
			{
				port_bandwidth_entry->read_proc = port_bandwidth_read;
				port_bandwidth_entry->write_proc = port_bandwidth_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for port_bandwidth");
				retval = FAILED;
				goto out;
			}
#endif
		}

		/*queue_bandwidth*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("queue_bandwidth",0,rtl865x_proc_dir,&queue_bandwidth_proc_fops,NULL);
#else
			queue_bandwidth_entry= create_proc_entry("queue_bandwidth", 0, rtl865x_proc_dir);
			if(queue_bandwidth_entry != NULL)
			{
				queue_bandwidth_entry->read_proc = queue_bandwidth_read;
				queue_bandwidth_entry->write_proc = queue_bandwidth_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for queue_bandwidth");
				retval = FAILED;
				goto out;
			}
#endif
		}
		/*priority_decision*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("priority_decision",0,rtl865x_proc_dir,&priority_decision_proc_fops,NULL);
#else
			priority_decision_entry= create_proc_entry("priority_decision", 0, rtl865x_proc_dir);
			if(priority_decision_entry != NULL)
			{
				priority_decision_entry->read_proc = priority_decision_read;
				priority_decision_entry->write_proc = priority_decision_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for priority_decision");
				retval = FAILED;
				goto out;
			}
#endif
		}
#if defined (CONFIG_RTL_ENABLE_RATELIMIT_TABLE)
		/* rate limit table */
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("rateLimit",0,rtl865x_proc_dir,&rateLimit_proc_fops,NULL);
#else
			rateLimit_entry= create_proc_entry("rateLimit", 0, rtl865x_proc_dir);
			if(rateLimit_entry != NULL)
			{
				rateLimit_entry->read_proc = rate_limit_read;
				rateLimit_entry->write_proc = rate_limit_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for rate limit table");
				retval = FAILED;
				goto out;
			}
#endif
		}
#endif
#if defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL865X_EVENT_PROC_DEBUG)
        {
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("eventMgr",0,rtl865x_proc_dir,&eventMgr_proc_fops,NULL);
#else
            eventMgr_entry=create_proc_entry("eventMgr", 0, rtl865x_proc_dir);
            if(eventMgr_entry != NULL)
            {
                eventMgr_entry->read_proc = rtl865x_event_proc_read;
                eventMgr_entry->write_proc = rtl865x_event_proc_write;
                retval = SUCCESS;
            }
#endif
        }
#endif

#if defined(CONFIG_RTL_IGMP_SNOOPING)
        {
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("igmp",0,rtl865x_proc_dir,&igmp_single_seq_file_operations,NULL);
#else
            igmp_entry=create_proc_entry("igmp", 0, rtl865x_proc_dir);
            if(igmp_entry != NULL)
            {
                /*
                   igmp_entry->read_proc = rtl865x_proc_igmpsnooping_read;
                   igmp_entry->write_proc = rtl865x_proc_igmpsnooping_write;
                   igmp_entry->owner = THIS_MODULE;
                   */
                igmp_entry->proc_fops = &igmp_single_seq_file_operations;
                retval = SUCCESS;
            }
#endif
        }
#endif
#endif

#ifdef CONFIG_RTL865X_ROMEPERF
		/*rome perf dump*/
		{
			rtl8651_romeperfInit();
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("perf_dump",0,rtl865x_proc_dir,&perf_dump_proc_fops,NULL);
#else
			perf_dump= create_proc_entry("perf_dump", 0, rtl865x_proc_dir);
			if(perf_dump != NULL)
			{
				perf_dump->read_proc = rtl865x_perf_proc_read;
				perf_dump->write_proc = rtl865x_perf_proc_write;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for dump_perf");
				retval = FAILED;
				goto out;
			}
#endif
		}
#endif

#if defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)
		
		/*stats*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("stats",0,rtl865x_proc_dir,&stats_proc_fops,NULL);
#else
			stats_debug_entry = create_proc_entry("stats",0,rtl865x_proc_dir);
			if(stats_debug_entry != NULL)
			{
				stats_debug_entry->read_proc = stats_debug_entry_read;
				stats_debug_entry->write_proc= stats_debug_entry_write;
				
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for stats");
				retval = FAILED;
				goto out;
			}
#endif
        }
        /*	#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)	*/
        {
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("privSkbInfo",0,rtl865x_proc_dir,&privSkbInfo_proc_fops,NULL);
#else
            prive_skb_debug_entry = create_proc_entry("privSkbInfo", 0, rtl865x_proc_dir);
            if(prive_skb_debug_entry != NULL)
            {
                prive_skb_debug_entry->read_proc = rtl819x_proc_privSkbInfo_read;
                prive_skb_debug_entry->write_proc = rtl819x_proc_privSkbInfo_write;
                retval = SUCCESS;
            }
#endif
        }

        /*memory*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("memory",0,rtl865x_proc_dir,&memory_proc_fops,NULL);
#else
			mem_entry = create_proc_entry("memory",0,rtl865x_proc_dir);
			if(mem_entry != NULL)
			{
				mem_entry->read_proc = proc_mem_read;
				mem_entry->write_proc= proc_mem_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for memory");
				retval = FAILED;
				goto out;
			}
#endif
		}
#ifdef CONFIG_RTL_NIC_QUEUE
		proc_create_data("tx_sw_queue",0,rtl865x_proc_dir,&tx_queue_proc_fops,NULL);
#endif
		/*diagnostic*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("diagnostic",0,rtl865x_proc_dir,&diagnostic_proc_fops,NULL);
#else
			diagnostic_entry= create_proc_entry("diagnostic", 0, rtl865x_proc_dir);
			if(diagnostic_entry != NULL)
			{
				diagnostic_entry->read_proc = diagnostic_read;
				diagnostic_entry->write_proc = diagnostic_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for diagnostic");
				retval = FAILED;
				goto out;
			}
#endif
		}
		/*port_status*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("port_status",0,rtl865x_proc_dir,&port_status_proc_fops,NULL);
#else
			port_status_entry= create_proc_entry("port_status", 0, rtl865x_proc_dir);
			if(port_status_entry != NULL)
			{
				port_status_entry->read_proc = port_status_read;
				port_status_entry->write_proc = port_status_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for port_status");
				retval = FAILED;
				goto out;
			}
#endif
		}

		/*phy*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("phyReg",0,rtl865x_proc_dir,&phyReg_proc_fops,NULL);
#else
			phyReg_entry= create_proc_entry("phyReg", 0, rtl865x_proc_dir);
			if(phyReg_entry != NULL)
			{
				phyReg_entry->read_proc = proc_phyReg_read;
				phyReg_entry->write_proc = proc_phyReg_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for phyTeg");
				retval = FAILED;
				goto out;
			}
#endif
		}
		/*asicCnt*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("asicCounter",0,rtl865x_proc_dir,&asicCounter_proc_fops,NULL);
#else
            asicCnt_entry=create_proc_entry("asicCounter", 0, rtl865x_proc_dir);
			if(asicCnt_entry != NULL)
			{
				asicCnt_entry->read_proc = rtl865x_proc_mibCounter_read;
				asicCnt_entry->write_proc = rtl865x_proc_mibCounter_write;
				retval = SUCCESS;
			}
#endif
		}

#if defined (CONFIG_RTL_INBAND_CTL_API)
		/*trafficRate*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("portRate",0,rtl865x_proc_dir,&portRate_proc_fops,NULL);
#else
            portRate_entry=create_proc_entry("portRate", 0, rtl865x_proc_dir);
			if(portRate_entry != NULL)
			{
				portRate_entry->read_proc = rtl865x_proc_portRate_read;
				portRate_entry->write_proc = rtl865x_proc_portRate_write;
				retval = SUCCESS;
			}
#endif
		}
#endif


		/* indirect access for special phy register */
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("mmd",0,rtl865x_proc_dir,&mmd_proc_fops,NULL);
#else
			mmd_entry= create_proc_entry("mmd", 0, rtl865x_proc_dir);
			if(mmd_entry != NULL)
			{
				mmd_entry->read_proc = proc_mmd_read;
				mmd_entry->write_proc = proc_mmd_write;

				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for mmd");
				retval = FAILED;
				goto out;
			}
#endif
		}

		/*maccr*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("mac",0,rtl865x_proc_dir,&mac_proc_fops,NULL);
#else
			mac_entry=create_proc_entry("mac", 0, rtl865x_proc_dir);
			if(mac_entry != NULL)
			{
				mac_entry->read_proc = mac_config_read;
				mac_entry->write_proc = mac_config_write;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for mac");
				retval = FAILED;
				goto out;
			}

#endif
		}
		/*FC threshold*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("fc_threshold",0,rtl865x_proc_dir,&fc_threshold_proc_fops,NULL);
#else
			fc_threshold_entry=create_proc_entry("fc_threshold", 0, rtl865x_proc_dir);
			if(fc_threshold_entry != NULL)
			{
				fc_threshold_entry->read_proc = fc_threshold_read;
				fc_threshold_entry->write_proc = fc_threshold_write;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for fc_threshold");
				retval = FAILED;
				goto out;
			}

#endif
		}
#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT)
		/*l3v6*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("l3v6",0,rtl865x_proc_dir,&l3v6_proc_fops,NULL);
#else
			l3v6_entry = create_proc_entry("l3v6", 0, rtl865x_proc_dir);
			if (l3v6_entry != NULL)
			{
				l3v6_entry->read_proc = l3v6_read;
				l3v6_entry->write_proc = l3v6_write;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for l3v6");
				retval = FAILED;
				goto out;
			}
#endif
		}
		/*arp6*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("arp6",0,rtl865x_proc_dir,&arp6_proc_fops,NULL);
#else
			arp6_entry = create_proc_entry("arp6", 0, rtl865x_proc_dir);
			if (arp6_entry != NULL)
			{
				arp6_entry->read_proc = arp6_read;
				arp6_entry->write_proc = arp6_write;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for arp6");
				retval = FAILED;
				goto out;
			}
#endif
		}
/*sw-arp6*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("sw-arp6",0,rtl865x_proc_dir,&sw_arp6_proc_fops,NULL);
#else
			sw_arp6_entry = create_proc_entry("sw_arp6", 0, rtl865x_proc_dir);
			if (sw_arp6_entry != NULL)
			{
				sw_arp6_entry->read_proc = sw_arp6_read;
				sw_arp6_entry->write_proc = sw_arp6_write;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for arp6");
				retval = FAILED;
				goto out;
			}
#endif
		}		
		/*nexthop6*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("nexthop6",0,rtl865x_proc_dir,&nexthop6_proc_fops,NULL);
#else
			nexthop6_entry = create_proc_entry("nexthop6", 0, rtl865x_proc_dir);
			if (nexthop6_entry != NULL)
			{
				nexthop6_entry->read_proc = nexthop6_read;
				nexthop6_entry->write_proc = nexthop6_write;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for nexthop6");
				retval = FAILED;
				goto out;
			}
#endif
		}

		/*mcast6*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("mcast6",0,rtl865x_proc_dir,&mcast6_proc_fops,NULL);
#else
			mcast6_entry = create_proc_entry("mcast6", 0, rtl865x_proc_dir);
			if (mcast6_entry != NULL)
			{
				mcast6_entry->read_proc = mcast6_read;
				mcast6_entry->write_proc = mcast6_write;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for mcast6");
				retval = FAILED;
				goto out;
			}
#endif
		}
		/*6rd*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("6rd",0,rtl865x_proc_dir,&sixrd_proc_fops,NULL);
#else
			ip6rd_entry = create_proc_entry("6rd", 0, rtl865x_proc_dir);
			if (ip6rd_entry != NULL)
			{
				ip6rd_entry->read_proc = ip6rd_read;
				ip6rd_entry->write_proc = ip6rd_write;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for 6rd");
				retval = FAILED;
				goto out;
			}
#endif
		}
		/*dslite*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("dslite",0,rtl865x_proc_dir,&dslite_proc_fops,NULL);
#else
			dslite_entry = create_proc_entry("dslite", 0, rtl865x_proc_dir);
			if (dslite_entry != NULL)
			{
				dslite_entry->read_proc = dslite_read;
				dslite_entry->write_proc = dslite_write;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for dslite");
				retval = FAILED;
				goto out;
			}
#endif
		}

		/*sw-dslite*/
		{
#ifdef CONFIG_RTL_PROC_NEW
            proc_create_data("sw_dslite",0,rtl865x_proc_dir,&sw_dslite_proc_fops,NULL);
#else
			sw_dslite_entry = create_proc_entry("sw_dslite", 0, rtl865x_proc_dir);
			if (sw_dslite_entry != NULL)
			{
				sw_dslite_entry->read_proc = sw_dslite_read;
				sw_dslite_entry->write_proc = sw_dslite_write;
				retval = SUCCESS;
			}
			else
			{
				rtlglue_printf("can't create proc entry for dslite");
				retval = FAILED;
				goto out;
			}
#endif
		}
#endif	//end of #if defined (CONFIG_RTL_8198C)
#endif	
			
	}
	else
	{
		retval = FAILED;
	}
#ifndef CONFIG_RTL_PROC_NEW
out:
#endif
	if(retval == FAILED)
		rtl865x_proc_debug_cleanup();

	return retval;

}

int32 rtl865x_proc_debug_cleanup(void)
{
	if(rtl865x_proc_dir)
	{
#ifdef CONFIG_RTL_PROC_DEBUG
		
		
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("vlan",rtl865x_proc_dir);
#else
		if(vlan_entry)
		{
			remove_proc_entry("vlan",rtl865x_proc_dir);
			vlan_entry = NULL;
		}
#endif

#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("netif",rtl865x_proc_dir);
#else
		if(netif_entry)
		{
			remove_proc_entry("netif", rtl865x_proc_dir);
			netif_entry = NULL;
		}
#endif

#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("acl",rtl865x_proc_dir);
#else
		if(acl_entry)
		{
			remove_proc_entry("acl", rtl865x_proc_dir);
			acl_entry = NULL;
		}
#endif
#if defined(CONFIG_RTL_MULTIPLE_WAN)
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("advRt",rtl865x_proc_dir);
#else
		if(advRt_entry)
		{
			remove_proc_entry("advRt", rtl865x_proc_dir);
			advRt_entry = NULL;
		}
#endif
#endif
#ifdef CONFIG_RTL_LAYERED_DRIVER
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("soft_aclChains",rtl865x_proc_dir);
#else
		if(acl_chains_entry)
		{
			remove_proc_entry("soft_aclChains", rtl865x_proc_dir);
			acl_chains_entry = NULL;
		}
#endif
#endif

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("soft_qosRules",rtl865x_proc_dir);
#else
		if(qos_rule_entry)
		{
			remove_proc_entry("soft_qosRules", rtl865x_proc_dir);
			qos_rule_entry = NULL;
		}
#endif
#endif
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("hs",rtl865x_proc_dir);
#else
		if(hs_entry)
		{
			remove_proc_entry("hs", rtl865x_proc_dir);
			hs_entry = NULL;
		}
#endif
#if defined(RTL_DEBUG_NIC_SKB_BUFFER)
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("nic_mbuf",rtl865x_proc_dir);
#else
		if(nic_skb_buff)
		{
			remove_proc_entry("nic_mbuf", rtl865x_proc_dir);
			nic_skb_buff = NULL;
		}
#endif
#endif
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("rxRing",rtl865x_proc_dir);
#else
		if(rxRing_entry)
		{
			remove_proc_entry("rxRing", rtl865x_proc_dir);
			rxRing_entry = NULL;
		}
#endif

#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("txRing",rtl865x_proc_dir);
#else
		if(txRing_entry)
		{
			remove_proc_entry("txRing", rtl865x_proc_dir);
			txRing_entry = NULL;
		}
#endif

#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("mbufRing",rtl865x_proc_dir);
#else
		if(mbuf_entry)
		{
			remove_proc_entry("mbufRing", rtl865x_proc_dir);
			mbuf_entry = NULL;
		}
#endif

#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("l2",rtl865x_proc_dir);
#else
		if(l2_entry)
		{
			remove_proc_entry("l2", rtl865x_proc_dir);
			l2_entry = NULL;
		}
#endif

		
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("arp",rtl865x_proc_dir);
#else
		if(arp_entry)
		{
			remove_proc_entry("arp", rtl865x_proc_dir);
			arp_entry = NULL;
		}
#endif

#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("nexthop",rtl865x_proc_dir);
#else
		if(nexthop_entry)
		{
			remove_proc_entry("nexthop", rtl865x_proc_dir);
			nexthop_entry = NULL;
		}
#endif

#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("l3",rtl865x_proc_dir);
#else
		if(l3_entry)
		{
			remove_proc_entry("l3", rtl865x_proc_dir);
			l3_entry = NULL;
		}
#endif

#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("ip",rtl865x_proc_dir);
#else
		if(ip_entry)
		{
			remove_proc_entry("ip", rtl865x_proc_dir);
			ip_entry = NULL;
		}
#endif

#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("pppoe",rtl865x_proc_dir);
#else
		if(pppoe_entry)
		{
			remove_proc_entry("pppoe", rtl865x_proc_dir);
			pppoe_entry = NULL;
		}
#endif

#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("napt",rtl865x_proc_dir);
#else
		if(napt_entry)
		{
			remove_proc_entry("napt", rtl865x_proc_dir);
		}
#endif
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("port_bandwidth",rtl865x_proc_dir);
#else
		if (port_bandwidth_entry)
		{
			remove_proc_entry("port_bandwidth", rtl865x_proc_dir);
		}
#endif

#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("queue_bandwidth",rtl865x_proc_dir);
#else
		if (queue_bandwidth_entry)
		{
			remove_proc_entry("queue_bandwidth", rtl865x_proc_dir);
		}
#endif
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("priority_decision",rtl865x_proc_dir);
#else
		if (priority_decision_entry)
		{
			remove_proc_entry("priority_decision", rtl865x_proc_dir);
		}
#endif
#if defined(CONFIG_RTL_LAYERED_DRIVER_L4)
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("sw_napt",rtl865x_proc_dir);
#else
		if(sw_napt_entry)
		{
			remove_proc_entry("sw_napt", rtl865x_proc_dir);
		}
#endif
#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("sw_l2",rtl865x_proc_dir);
#else
		if(sw_l2_entry)
		{
			remove_proc_entry("sw_l2", rtl865x_proc_dir);
		}
#endif
#endif

		#if defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL865X_EVENT_PROC_DEBUG)
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("eventMgr",rtl865x_proc_dir);
#else
		if(eventMgr_entry != NULL)
		{
			remove_proc_entry("eventMgr", rtl865x_proc_dir);
		}
#endif
		#endif

#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("hwMCast",rtl865x_proc_dir);
#else
		if(hwMCast_entry != NULL)
		{
			remove_proc_entry("hwMCast", rtl865x_proc_dir);
		}
#endif

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("swMCast",rtl865x_proc_dir);
#else
		if(swMCast_entry != NULL)
		{
			remove_proc_entry("swMCast", rtl865x_proc_dir);
		}
#endif
#endif
		
#if defined (CONFIG_RTL_IGMP_SNOOPING)
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("igmp",rtl865x_proc_dir);
#else
		if(igmp_entry!=NULL)
		{
			remove_proc_entry("igmp", rtl865x_proc_dir);
		}
#endif
#endif

#if defined (CONFIG_RTL_ENABLE_RATELIMIT_TABLE)
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("rateLimit",rtl865x_proc_dir);
#else
		if(rateLimit_entry != NULL)
		{
			remove_proc_entry("rateLimit", rtl865x_proc_dir);
		}
#endif
#endif

		
#endif

#ifdef CONFIG_RTL865X_ROMEPERF
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("perf_dump",rtl865x_proc_dir);
#else
		if(perf_dump)
		{
			remove_proc_entry("perf_dump", rtl865x_proc_dir);
		}
#endif
#endif

#if defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)

#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("stats",rtl865x_proc_dir);
#else
		if(stats_debug_entry)
		{
			remove_proc_entry("stats",rtl865x_proc_dir);
			stats_debug_entry = NULL;
		}	
#endif
		
		/*#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)*/
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("privSkbInfo",rtl865x_proc_dir);
#else
		if(prive_skb_debug_entry != NULL)
		{
			remove_proc_entry("privSkbInfo", rtl865x_proc_dir);
		}
#endif
		/*#endif*/

#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("memory",rtl865x_proc_dir);
#else
		if(mem_entry)
		{
			remove_proc_entry("memory", mem_entry);
			mem_entry = NULL;
		}
#endif
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("diagnostic",rtl865x_proc_dir);
#else
		if (diagnostic_entry)
		{
			remove_proc_entry("diagnostic", rtl865x_proc_dir);
		}
#endif
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("port_status",rtl865x_proc_dir);
#else
		if (port_status_entry)
		{
			remove_proc_entry("port_status", rtl865x_proc_dir);
		}
#endif
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("phyReg",rtl865x_proc_dir);
#else
		if (phyReg_entry)
		{
			remove_proc_entry("phyReg", rtl865x_proc_dir);
		}
#endif
		
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("mmd",rtl865x_proc_dir);
#else
		if (mmd_entry)
		{
			remove_proc_entry("mmd", rtl865x_proc_dir);
		}
#endif
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("asicCounter",rtl865x_proc_dir);
#else
		if(asicCnt_entry != NULL)
		{
			remove_proc_entry("asicCounter", rtl865x_proc_dir);
		}
#endif
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("mac",rtl865x_proc_dir);
#else
		if(mac_entry != NULL)
		{
			remove_proc_entry("mac",rtl865x_proc_dir);
		}
#endif
	
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("fc_threshold",rtl865x_proc_dir);
#else
		if(fc_threshold_entry != NULL)
		{
			remove_proc_entry("fc_threshold", rtl865x_proc_dir);
		}
#endif

#endif
		
#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT)
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("l3v6",rtl865x_proc_dir);
#else
		if (l3v6_entry != NULL)
		{
			remove_proc_entry("l3v6", rtl865x_proc_dir);
		}
#endif
		
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("arp6",rtl865x_proc_dir);
#else
		if (arp6_entry != NULL)
		{
			remove_proc_entry("arp6", rtl865x_proc_dir);
		}
#endif
		
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("nexthop6",rtl865x_proc_dir);
#else
		if (nexthop6_entry != NULL)
		{
			remove_proc_entry("nexthop6", rtl865x_proc_dir);
		}
#endif


#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("mcast6",rtl865x_proc_dir);
#else
		if (mcast6_entry != NULL)
		{
			remove_proc_entry("mcast6", rtl865x_proc_dir);
		}
#endif
		
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("6rd",rtl865x_proc_dir);
#else
		if (ip6rd_entry != NULL)
		{
			remove_proc_entry("6rd", rtl865x_proc_dir);
		}
#endif
		
#ifdef CONFIG_RTL_PROC_NEW
        remove_proc_entry("dslite",rtl865x_proc_dir);
#else
		if (dslite_entry != NULL)
		{
			remove_proc_entry("dslite", rtl865x_proc_dir);
		}
#endif
#endif		

#ifdef CONFIG_RTL_PROC_NEW
		remove_proc_entry(RTL865X_PROC_DIR_NAME, &proc_root);
#else
		remove_proc_entry(RTL865X_PROC_DIR_NAME, NULL);
		rtl865x_proc_dir = NULL;
#endif

	}

	return SUCCESS;
}


