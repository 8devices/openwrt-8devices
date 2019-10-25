/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalPwrSeqCmd.c
	
Abstract:
	Implement HW Power sequence configuration CMD handling routine for Realtek devices.
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2011-07-07 Roger            Create.
	
--*/
#ifdef __ECOS
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#endif


/*
#if !defined(__ECOS) && !defined(CPTCFG_CFG80211_MODULE)
#include "mp_precomp.h"
#else
#include "../mp_precomp.h"
#endif
*/
#ifdef __KERNEL__
#include <linux/kernel.h>
#endif
#include "8192cd.h"
#include "8192cd_debug.h"
#include "8192cd_headers.h"
#include "8192cd_util.h"

#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)

#define TRUE		1
#define FALSE	0

//
//	Description: 
//		This routine deal with the Power Configuration CMDs parsing for RTL8723/RTL8188E Series IC.
//
//	Assumption:
//		We should follow specific format which was released from HW SD. 
//
//	2011.07.07, added by Roger.
//
unsigned int HalPwrSeqCmdParsing(struct rtl8192cd_priv *priv, unsigned char CutVersion, unsigned char FabVersion, 
							 unsigned char InterfaceType, WLAN_PWR_CFG PwrSeqCmd[])
{
		
	WLAN_PWR_CFG 	PwrCfgCmd = {0};
	unsigned int 		bPollingBit = FALSE;
	unsigned int		AryIdx=0;
	unsigned char		value = 0;
	unsigned int		offset = 0;
	unsigned int 		pollingCount = 0; // polling autoload done.
	unsigned int		maxPollingCnt = 5000;
	unsigned int		round = 0;
	
	do {
		PwrCfgCmd=PwrSeqCmd[AryIdx];

		DEBUG_INFO("%s %d, ENTRY, offset:0x%x, cut_msk:0x%x, fab_msk:0x%x, if_msk:0x%x, base:0x%x, cmd:0x%x, msk:0x%x, value:0x%x\n", 
			__FUNCTION__, __LINE__, GET_PWR_CFG_OFFSET(PwrCfgCmd), GET_PWR_CFG_CUT_MASK(PwrCfgCmd), 
			GET_PWR_CFG_FAB_MASK(PwrCfgCmd), GET_PWR_CFG_INTF_MASK(PwrCfgCmd), GET_PWR_CFG_BASE(PwrCfgCmd), 
			GET_PWR_CFG_CMD(PwrCfgCmd), GET_PWR_CFG_MASK(PwrCfgCmd), GET_PWR_CFG_VALUE(PwrCfgCmd));

		//2 Only Handle the command whose FAB, CUT, and Interface are matched
		if((GET_PWR_CFG_FAB_MASK(PwrCfgCmd)&FabVersion)&&
			(GET_PWR_CFG_CUT_MASK(PwrCfgCmd)&CutVersion)&&
			(GET_PWR_CFG_INTF_MASK(PwrCfgCmd)&InterfaceType)) {
			switch(GET_PWR_CFG_CMD(PwrCfgCmd))
			{
			case PWR_CMD_READ:
				DEBUG_INFO("%s %d, PWR_CMD_READ\n", __FUNCTION__, __LINE__);
				break;

			case PWR_CMD_WRITE:
				DEBUG_INFO("%s %d, PWR_CMD_WRITE\n", __FUNCTION__, __LINE__);
				offset = GET_PWR_CFG_OFFSET(PwrCfgCmd);

#ifdef CONFIG_SDIO_HCI
				//
				// <Roger_Notes> We should deal with interface specific address mapping for some interfaces, e.g., SDIO interface
				// 2011.07.07.
				//
				if (GET_PWR_CFG_BASE(PwrCfgCmd) == PWR_BASEADDR_SDIO)
				{
					// Read Back SDIO Local value
					value = SdioLocalCmd52Read1Byte(priv, offset);

					value &= ~(GET_PWR_CFG_MASK(PwrCfgCmd));
					value |= (GET_PWR_CFG_VALUE(PwrCfgCmd) & GET_PWR_CFG_MASK(PwrCfgCmd));

					// Write Back SDIO Local value
					SdioLocalCmd52Write1Byte(priv, offset, value);
				}
				else
#endif
				{
					//Read the value from system register		
					value = RTL_R8(offset);
					value = value&(~(GET_PWR_CFG_MASK(PwrCfgCmd)));
					value = value|(GET_PWR_CFG_VALUE(PwrCfgCmd)&GET_PWR_CFG_MASK(PwrCfgCmd));

					//Write the value back to sytem register
					RTL_W8(offset, value);
				}
				break;

			case PWR_CMD_POLLING:
				DEBUG_INFO("%s %d, PWR_CMD_POLLING\n", __FUNCTION__, __LINE__);
				bPollingBit = FALSE;					
				offset = GET_PWR_CFG_OFFSET(PwrCfgCmd);				

				do {
#ifdef CONFIG_SDIO_HCI
					if (GET_PWR_CFG_BASE(PwrCfgCmd) == PWR_BASEADDR_SDIO)
						value = SdioLocalCmd52Read1Byte(priv, offset);
					else
#endif
					value = RTL_R8(offset);
					value=value&GET_PWR_CFG_MASK(PwrCfgCmd);
					if(value==(GET_PWR_CFG_VALUE(PwrCfgCmd)&GET_PWR_CFG_MASK(PwrCfgCmd)))
						bPollingBit=TRUE;
					else	
						delay_us(10);
					
					if(pollingCount++ > maxPollingCnt){
						DEBUG_WARN("%s %d, PWR_CMD_POLLING, Fail to polling Offset[0x%x]\n", __FUNCTION__, __LINE__, offset);
						return FALSE;
					}
				}while(!bPollingBit);

				break;

			case PWR_CMD_DELAY:
				DEBUG_INFO("%s %d, PWR_CMD_DELAY\n", __FUNCTION__, __LINE__);
				if(GET_PWR_CFG_VALUE(PwrCfgCmd) == PWRSEQ_DELAY_US)
					delay_us(GET_PWR_CFG_OFFSET(PwrCfgCmd));
				else
					delay_us(GET_PWR_CFG_OFFSET(PwrCfgCmd)*1000);
				break; 

			case PWR_CMD_END:
				// When this command is parsed, end the process
				DEBUG_INFO("%s %d, PWR_CMD_END\n", __FUNCTION__, __LINE__);
				return TRUE;
				break;

			default:
				DEBUG_ERR("%s %d, Unknown CMD!!\n", __FUNCTION__, __LINE__);
				break;
			}

		}
		
		AryIdx++;//Add Array Index

		if (++round > 10000) {
			panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
			break;
		}
	}while(1);

	return TRUE;
}
#endif

