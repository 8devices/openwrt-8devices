/*
* Copyright c                  Realtek Semiconductor Corporation, 2009  
* All rights reserved.
* 
* Program : Switch table basic operation driver
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

#include <net/rtl/rtl_types.h>
//#include "types.h"
#include "asicRegs.h"
#include "asicTabs.h"
//#include "rtl_glue.h"
#include "rtl865x_hwPatch.h"
#include "rtl865x_asicBasic.h"

/*
 *  According to ghhuang's suggest,
 *    we DO NOT need to access 8 ASIC entries at once.
 *  We just need to access several entries as we need.
 */

#define RTL865X_FAST_ASIC_ACCESS

/*
 *  For RTL865xC Access protection mechanism
 *
 *	We define 2 different MACROs for ASIC table
 *	read/write protection correspondingly
 *
 */
//#define		RTL865XC_ASIC_READ_PROTECTION				/* Enable/Disable ASIC read protection */
#define		RTL865XC_ASIC_WRITE_PROTECTION				/* Enable/Disable ASIC write protection */

//#undef		RTL865X_READ_MULTIPLECHECK
#define 		RTL865X_READ_MULTIPLECHECK
#define		RTL865X_READ_MULTIPLECHECK_CNT		2
#define		RTL865X_READ_MULTIPLECHECK_MAX_RETRY	10
#define 	SIG_REG	(BSP_REVR+0x0C)
#define		ID_REG	(BSP_REVR+0x00)


//int8 RtkHomeGatewayChipName[16];
//int32 RtkHomeGatewayChipRevisionID;
//int32 RtkHomeGatewayChipNameID;

#ifdef RTL865X_FAST_ASIC_ACCESS
static uint32 _rtl8651_asicTableSize[] =
{
        2 /*TYPE_L2_SWITCH_TABLE*/,
        1 /*TYPE_ARP_TABLE*/,
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	 3 /*TYPE_L3_ROUTING_TABLE*/,
#else
	 2 /*TYPE_L3_ROUTING_TABLE*/,
#endif
        3 /*TYPE_MULTICAST_TABLE*/,
        5 /*TYPE_NETIF_TABLE*/,
        3 /*TYPE_EXT_INT_IP_TABLE*/,
        3 /*TYPE_VLAN_TABLE*/,
        3 /*TYPE_VLAN1_TABLE*/,          
    4 /*TYPE_SERVER_PORT_TABLE*/,
    3 /*TYPE_L4_TCP_UDP_TABLE*/,
    3 /*TYPE_L4_ICMP_TABLE*/,
    1 /*TYPE_PPPOE_TABLE*/,
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    11 /*TYPE_ACL_RULE_TABLE*/,
#else
    8 /*TYPE_ACL_RULE_TABLE*/,
#endif
    1 /*TYPE_NEXT_HOP_TABLE*/,
    3 /*TYPE_RATE_LIMIT_TABLE*/,
    1 /*TYPE_ALG_TABLE*/,
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)  
   9 /*TYPE_DS_LITE_TABLE*/,	
   6 /*TYPE_6RD_TABLE*/,
   6 /*TYPE_L3_V6_ROUTING_TABLE*/,
   1 /*TYPE_NEXT_HOP_V6_TABLE*/,
   3 /*TYPE_ARP_V6_TABLE*/,
   9 /*TYPE_MULTICAST_V6_TABLE*/,
#endif
};
#endif

static void _rtl8651_asicTableAccessForward(uint32 tableType, uint32 eidx, void *entryContent_P) 
{
	ASSERT_CSP(entryContent_P);

	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

#ifdef RTL865X_FAST_ASIC_ACCESS

	{
		register uint32 index;

		for( index = 0; index < _rtl8651_asicTableSize[tableType]; index++ )
		{
			WRITE_MEM32(TCR0+(index<<2), *((uint32 *)entryContent_P + index));
		}

	}
#else
	WRITE_MEM32(TCR0, *((uint32 *)entryContent_P + 0));
	WRITE_MEM32(TCR1, *((uint32 *)entryContent_P + 1));
	WRITE_MEM32(TCR2, *((uint32 *)entryContent_P + 2));
	WRITE_MEM32(TCR3, *((uint32 *)entryContent_P + 3));
	WRITE_MEM32(TCR4, *((uint32 *)entryContent_P + 4));
	WRITE_MEM32(TCR5, *((uint32 *)entryContent_P + 5));
	WRITE_MEM32(TCR6, *((uint32 *)entryContent_P + 6));
	WRITE_MEM32(TCR7, *((uint32 *)entryContent_P + 7));

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if( _rtl8651_asicTableSize[tableType]>8)
	{	
		WRITE_MEM32(TCR8, *((uint32 *)entryContent_P + 8));
		WRITE_MEM32(TCR9, *((uint32 *)entryContent_P + 9));
		WRITE_MEM32(TCR10, *((uint32 *)entryContent_P + 10));
	}
#endif

#endif	
	WRITE_MEM32(SWTAA, ((uint32) rtl8651_asicTableAccessAddrBase(tableType) + eidx * RTL8651_ASICTABLE_ENTRY_LENGTH));//Fill address
}

int32 _rtl8651_addAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P) 
{
	_rtl8651_asicTableAccessForward(tableType, eidx, entryContent_P);

	#ifdef RTL865XC_ASIC_WRITE_PROTECTION
	if (RTL819X_TLU_CHECK)	/* No need to stop HW table lookup process */
	{
      		WRITE_MEM32(SWTCR0,EN_STOP_TLU|READ_MEM32(SWTCR0));
		while ( (READ_MEM32(SWTCR0) & STOP_TLU_READY)==0);
	}
	#endif

	WRITE_MEM32(SWTACR, ACTION_START | CMD_ADD );//Activate add command

	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done
    
	if ( (READ_MEM32(SWTASR) & TABSTS_MASK) != TABSTS_SUCCESS )//Check status
	{
		#ifdef RTL865XC_ASIC_WRITE_PROTECTION
		if(RTL819X_TLU_CHECK)	/* No need to stop HW table lookup process */
		{
			WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));
		}
		#endif

		return FAILED;
	}

	#ifdef RTL865XC_ASIC_WRITE_PROTECTION
	if (RTL819X_TLU_CHECK)	/* No need to stop HW table lookup process */
	{
		WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));
	}
	#endif

	return SUCCESS;
}
static unsigned int mcastForceAddOpCnt=0;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
static unsigned int mcastForceAddOpCnt6=0;
#endif
unsigned int _rtl865x_getForceAddMcastOpCnt(void)
{
	return mcastForceAddOpCnt;
}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
unsigned int _rtl8198C_getForceAddMcastv6OpCnt(void)
{
	return mcastForceAddOpCnt6;
}
#endif
int32 _rtl8651_forceAddAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P) 
{
	if(tableType==TYPE_MULTICAST_TABLE)
	{
		mcastForceAddOpCnt++;
	}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if(tableType==TYPE_MULTICAST_V6_TABLE)
	{
		mcastForceAddOpCnt6++;
	}
#endif
	#ifdef RTL865XC_ASIC_WRITE_PROTECTION
	if (RTL819X_TLU_CHECK)	/* No need to stop HW table lookup process */
	{
		WRITE_MEM32(SWTCR0,EN_STOP_TLU|READ_MEM32(SWTCR0));
		while ( (READ_MEM32(SWTCR0) & STOP_TLU_READY)==0);
	}
	#endif

	_rtl8651_asicTableAccessForward(tableType, eidx, entryContent_P);

 	WRITE_MEM32(SWTACR, ACTION_START | CMD_FORCE);//Activate add command
	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

	#ifdef RTL865XC_ASIC_WRITE_PROTECTION
	if (RTL819X_TLU_CHECK)	/* No need to stop HW table lookup process */
	{
		WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));
	}
	#endif

	return SUCCESS;
}

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define MAX_ENTRY_CONTENT_SIZE	11
#else
#define MAX_ENTRY_CONTENT_SIZE	8
#endif

int32 _rtl8651_readAsicEntry(uint32 tableType, uint32 eidx, void *entryContent_P) 
{
	uint32 *entryAddr;
	uint32 tmp;/* dummy variable, don't remove it */

	#ifdef RTL865X_READ_MULTIPLECHECK
	uint32 entryContent[RTL865X_READ_MULTIPLECHECK_CNT][MAX_ENTRY_CONTENT_SIZE];	// CONFIG_RTL_8198C
	uint32 entryContentIdx;
	uint32 entryContent_new = RTL865X_READ_MULTIPLECHECK_CNT;/* to indicate which content is newer */
	uint32 entryCompare_max_count = RTL865X_READ_MULTIPLECHECK_MAX_RETRY;
	uint32 needRetry;
	#endif
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	int i;
#endif
	
	#ifdef RTL865XC_ASIC_READ_PROTECTION
	if (RTL819X_TLU_CHECK)	/* No need to stop HW table lookup process */
	{
		WRITE_MEM32(SWTCR0,EN_STOP_TLU|READ_MEM32(SWTCR0));
		while ( (READ_MEM32(SWTCR0) & STOP_TLU_READY)==0);
	}
	#endif

	ASSERT_CSP(entryContent_P);
	entryAddr = (uint32 *) (
		(uint32) rtl8651_asicTableAccessAddrBase(tableType) + (eidx<<5 /*RTL8651_ASICTABLE_ENTRY_LENGTH*/) ) ;
		/*(uint32) rtl8651_asicTableAccessAddrBase(tableType) + eidx * RTL8651_ASICTABLE_ENTRY_LENGTH);*/

	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command ready
    
#ifdef RTL865X_READ_MULTIPLECHECK
		do
		{
			for (	entryContentIdx = 0 ;
				entryContentIdx < RTL865X_READ_MULTIPLECHECK_CNT ;
			   	entryContentIdx ++ )
			{
				entryContent[entryContentIdx][0] = *(entryAddr + 0);
				entryContent[entryContentIdx][1] = *(entryAddr + 1);
				entryContent[entryContentIdx][2] = *(entryAddr + 2);
				entryContent[entryContentIdx][3] = *(entryAddr + 3);
				entryContent[entryContentIdx][4] = *(entryAddr + 4);
				entryContent[entryContentIdx][5] = *(entryAddr + 5);
				entryContent[entryContentIdx][6] = *(entryAddr + 6);
				entryContent[entryContentIdx][7] = *(entryAddr + 7);
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
				if(_rtl8651_asicTableSize[tableType]>8)
				{
				/*
				   The LSB 256bits is defined as before, but bit 335 to bit256 is moved to add addr[15].
				   for example: 
					ACL rule 0 Virtual Address : bit [255:0]  : 0xbb0c_0000 ~ 0xbb0c_001c
						                                  bit [335:256] : 0xbb0c_8000 ~ 0xbb0c_8008
				   
					entryContent[entryContentIdx][8] = *(entryAddr + 0 + (BIT(15) / 4));
					entryContent[entryContentIdx][9] = *(entryAddr + 1 + (BIT(15) / 4));
					entryContent[entryContentIdx][10] = *(entryAddr + 2 + (BIT(15) / 4));
				 */	
				entryContent[entryContentIdx][8] = *(entryAddr + 8192);
				entryContent[entryContentIdx][9] = *(entryAddr + 8193);
				entryContent[entryContentIdx][10] = *(entryAddr + 8194);
				}
#endif

			}
			entryContent_new = RTL865X_READ_MULTIPLECHECK_CNT-1;

			needRetry = FALSE;

			for (	entryContentIdx = 1 ;
				entryContentIdx < RTL865X_READ_MULTIPLECHECK_CNT ;
			   	entryContentIdx ++ )
			{
				int32 idx;

				for ( idx = 0 ; idx < MAX_ENTRY_CONTENT_SIZE ; idx ++ )	// CONFIG_RTL_8198C
				{
					if (	entryContent[entryContentIdx][idx] !=
						entryContent[0][idx]	)
					{
						needRetry = TRUE;
						goto retry;
					}
				}
			}
retry:
		entryCompare_max_count --;
		} while (	( needRetry == TRUE ) &&
		      		( entryCompare_max_count > 0 ) );
	
	/* Update entryAddr for newer one */
	entryAddr = &( entryContent[ entryContent_new ][0] );
#endif

#if defined(RTL865X_READ_MULTIPLECHECK) && (defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F))
	#ifdef RTL865X_FAST_ASIC_ACCESS
	for( i = 0; i < _rtl8651_asicTableSize[tableType]; i++ )
	{
		*((uint32 *)entryContent_P + i) = entryContent[ entryContent_new ][i];
	}
	#else
	for( i = 0; i < MAX_ENTRY_CONTENT_SIZE; i++ )
	{
		*((uint32 *)entryContent_P + i) = entryContent[ entryContent_new ][i];
	}
	#endif

#else
#ifdef RTL865X_FAST_ASIC_ACCESS
	{
		register uint32 index;

		for( index = 0; index < _rtl8651_asicTableSize[tableType]; index++ )
		{
			#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
			if(index<8)
				*((uint32 *)entryContent_P + index) = READ_MEM32((uint32)(entryAddr + index));
			else
				*((uint32 *)entryContent_P + index) = READ_MEM32((uint32)(entryAddr + index-8+8192));
			#else
			*((uint32 *)entryContent_P + index) = READ_MEM32((uint32)(entryAddr + index));
			#endif
		}
	}
#else
	*((uint32 *)entryContent_P + 0) = *(entryAddr + 0);
	*((uint32 *)entryContent_P + 1) = *(entryAddr + 1);
	*((uint32 *)entryContent_P + 2) = *(entryAddr + 2);
	*((uint32 *)entryContent_P + 3) = *(entryAddr + 3);
	*((uint32 *)entryContent_P + 4) = *(entryAddr + 4);
	*((uint32 *)entryContent_P + 5) = *(entryAddr + 5);
	*((uint32 *)entryContent_P + 6) = *(entryAddr + 6);
	*((uint32 *)entryContent_P + 7) = *(entryAddr + 7);
    
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if(_rtl8651_asicTableSize[tableType]>8)
	{
		*((uint32 *)entryContent_P + 8) = *(entryAddr + 8192);
		*((uint32 *)entryContent_P + 9) = *(entryAddr + 8193);
		*((uint32 *)entryContent_P + 10) = *(entryAddr+ 8194);
	}
#endif
#endif
#endif

	/* Dummy read. Must read an un-used table entry to refresh asic latch */
	tmp = *(uint32 *)((uint32) rtl8651_asicTableAccessAddrBase(TYPE_ACL_RULE_TABLE) + 1024 * RTL8651_ASICTABLE_ENTRY_LENGTH);
	#ifdef RTL865XC_ASIC_READ_PROTECTION
	if (RTL819X_TLU_CHECK)	/* No need to stop HW table lookup process */
	{
		WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));
	}
	#endif

	return 0;
}

int32 _rtl8651_readAsicEntryStopTLU(uint32 tableType, uint32 eidx, void *entryContent_P) 
{
	uint32 *entryAddr;
	uint32 tmp;/* dummy variable, don't remove it */

	ASSERT_CSP(entryContent_P);
	entryAddr = (uint32 *) (
		(uint32) rtl8651_asicTableAccessAddrBase(tableType) + (eidx<<5 /*RTL8651_ASICTABLE_ENTRY_LENGTH*/) ) ;
		/*(uint32) rtl8651_asicTableAccessAddrBase(tableType) + eidx * RTL8651_ASICTABLE_ENTRY_LENGTH);*/
#if 0
	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command ready
#endif	

	if (RTL819X_TLU_CHECK)	/* No need to stop HW table lookup process */
	{
		WRITE_MEM32(SWTCR0,EN_STOP_TLU|READ_MEM32(SWTCR0));
		#if 0
		//while ( (READ_MEM32(SWTCR0) & STOP_TLU_READY)==0);
		#endif	
	}

#ifdef RTL865X_FAST_ASIC_ACCESS
	{
		register uint32 index;

		for( index = 0; index < _rtl8651_asicTableSize[tableType]; index++ )
		{
            #if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
            if(index<8)
                *((uint32 *)entryContent_P + index) = READ_MEM32((uint32)(entryAddr + index));
            else
                *((uint32 *)entryContent_P + index) = READ_MEM32((uint32)(entryAddr + index-8+8192));
            #else
			*((uint32 *)entryContent_P + index) = READ_MEM32((uint32)(entryAddr + index));
            #endif
		}
	}
#else
	*((uint32 *)entryContent_P + 0) = *(entryAddr + 0);
	*((uint32 *)entryContent_P + 1) = *(entryAddr + 1);
	*((uint32 *)entryContent_P + 2) = *(entryAddr + 2);
	*((uint32 *)entryContent_P + 3) = *(entryAddr + 3);
	*((uint32 *)entryContent_P + 4) = *(entryAddr + 4);
	*((uint32 *)entryContent_P + 5) = *(entryAddr + 5);
	*((uint32 *)entryContent_P + 6) = *(entryAddr + 6);
	*((uint32 *)entryContent_P + 7) = *(entryAddr + 7);

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    if(_rtl8651_asicTableSize[tableType]>8)
    {
        *((uint32 *)entryContent_P + 8) = *(entryAddr + 8192);
        *((uint32 *)entryContent_P + 9) = *(entryAddr + 8193);
        *((uint32 *)entryContent_P + 10) = *(entryAddr + 8194);
    }
#endif

#endif


	if (RTL819X_TLU_CHECK)	/* No need to stop HW table lookup process */
	{
		WRITE_MEM32(SWTCR0,~EN_STOP_TLU&READ_MEM32(SWTCR0));
	}

	/* Dummy read. Must read an un-used table entry to refresh asic latch */
	tmp = *(uint32 *)((uint32) rtl8651_asicTableAccessAddrBase(TYPE_ACL_RULE_TABLE) + 1024 * RTL8651_ASICTABLE_ENTRY_LENGTH);

	return 0;
}

int32 _rtl8651_delAsicEntry(uint32 tableType, uint32 startEidx, uint32 endEidx) 
{
	uint32 eidx = startEidx;

	while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done

#ifdef RTL865X_FAST_ASIC_ACCESS
	{
		register uint32 index;

		for( index = 0; index < _rtl8651_asicTableSize[tableType]; index++ )
		{
			WRITE_MEM32(TCR0+(index<<2), 0);
		}
	}
#else
	WRITE_MEM32(TCR0, 0);
	WRITE_MEM32(TCR1, 0);
	WRITE_MEM32(TCR2, 0);
	WRITE_MEM32(TCR3, 0);
	WRITE_MEM32(TCR4, 0);
	WRITE_MEM32(TCR5, 0);
	WRITE_MEM32(TCR6, 0);
	WRITE_MEM32(TCR7, 0);

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    if(_rtl8651_asicTableSize[tableType]>8)
    {
    	WRITE_MEM32(TCR8, 0);
    	WRITE_MEM32(TCR9, 0);
    	WRITE_MEM32(TCR10, 0);
    }
#endif

#endif	
	
	while (eidx <= endEidx) {
		WRITE_MEM32(SWTAA, (uint32) rtl8651_asicTableAccessAddrBase(tableType) + eidx * RTL8651_ASICTABLE_ENTRY_LENGTH);//Fill address
        
		WRITE_MEM32(SWTACR, ACTION_START | CMD_FORCE);//Activate add command

		while ( (READ_MEM32(SWTACR) & ACTION_MASK) != ACTION_DONE );//Wait for command done
    
		if ( (READ_MEM32(SWTASR) & TABSTS_MASK) != TABSTS_SUCCESS )//Check status
			return FAILED;
		
		++eidx;
	}
	return SUCCESS;
}

extern void machine_restart(char *command);
static unsigned int rtl819x_lastTxDesc=0;
static unsigned int rtl819x_lastRxDesc=0;
static unsigned int rtl819x_swHangCnt=0;
static unsigned int rtl819x_checkSwCoreTimer=0;
void rtl819x_poll_sw(void)
{
	unsigned int port6Queue0Cgst= (READ_MEM32(0xbb80610c) & (1<<16));
	unsigned int curRing0RxDesc=READ_MEM32(0xb8010004)&0xFFFFFFFC;
	unsigned int curRing0TxDesc=READ_MEM32(0xb8010020)&0xFFFFFFFC;

	if (((rtl819x_checkSwCoreTimer++) % 20)==0) 
	{
		
		//unsigned int sysDescRunOut= (READ_MEM32(0xbb806100) & (1<<27));
		if(port6Queue0Cgst==0) 
		{
			rtl819x_swHangCnt=0;
		}
		else
		{
		
			if((rtl819x_lastTxDesc==0) || (rtl819x_lastRxDesc==0))
			{
				rtl819x_lastRxDesc=curRing0RxDesc;
				rtl819x_lastTxDesc=curRing0TxDesc;
				rtl819x_swHangCnt=0;
			}
			else
			{
				if((rtl819x_lastRxDesc==curRing0RxDesc) && (rtl819x_lastTxDesc==curRing0TxDesc))
				{
					rtl819x_swHangCnt++;
				}
				else
				{
					rtl819x_swHangCnt=0;
				}
			}

		
		}
		
		
		rtl819x_lastRxDesc=curRing0RxDesc;
		rtl819x_lastTxDesc=curRing0TxDesc;

		if(rtl819x_swHangCnt>2)
		{
			rtl819x_swHangCnt=0;
			rtlglue_printf(".........................................\n");
			machine_restart(NULL);
		}
		
	}

	return;
}

#ifdef CONFIG_RTL_8881A
#include <linux/reboot.h>

int Lx1_check(void)
{
	if (REG32(GISR2) & LX1_BTRDY_IP)  {	
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)) // added by lynn_pu, 2014-10-15
		extern unsigned int get_uptime_by_sec(void);
		panic_printk("\nLexra bus 1 master timeout, GISR2= 0x%08x, uptime= %d seconds.\n\n", REG32(GISR2), get_uptime_by_sec());
#endif
		machine_restart(NULL);
	}
	return 0;
}
#endif


