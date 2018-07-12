

//#include "halbt_precomp.h"
#include "../phydm/phydm_precomp.h"
#include "halbtc_socket.h"



#define _FAIL 0
#define _SUCCESS 1
#define BT_INFO_LENGTH 8
#define RECV_DATA_MAX_LEN 255



/*global for socket TRX, it is actually rtlpriv*/
struct rtl8192cd_priv *pbtcoexadapter = NULL;
/*back up socket for UDP RX int*/
struct sock *sk_store;

void * safe_memcpy (void * dest, const void *src, u32 n , u32 max_len)
{
	if(n > max_len) {
		memcpy(dest, src, max_len);
		printk("critical error in memcpy!\n");
	} else {
		//ok case
		memcpy(dest,src, n);
	}
	return NULL;
}


void btinfo_evt_dump(struct btinfo_8761ATV *info)
{
	printk("cid:0x%02x, len:%u\n", info->cid, info->len);

	if (info->len > 2)
		printk("byte2:%s%s%s%s%s%s%s%s\n"
		, info->bConnection?"bConnection ":""
		, info->bSCOeSCO?"bSCOeSCO ":""
		, info->bInQPage?"bInQPage ":""
		, info->bACLBusy?"bACLBusy ":""
		, info->bSCOBusy?"bSCOBusy ":""
		, info->bHID?"bHID ":""
		, info->bA2DP?"bA2DP ":""
		, info->bFTP?"bFTP":""
	);

	if (info->len > 3)
		printk("retry_cnt:%u\n", info->retry_cnt);

	if (info->len > 4)
		printk("rssi:%u\n", info->rssi);

	if (info->len > 5)
		printk("byte5:%s%s\n"
		, info->eSCO_SCO?"eSCO_SCO ":""
		, info->Master_Slave?"Master_Slave ":""
	);
}


void rtl_btcoex_btinfo_cmd(struct rtl8192cd_priv *priv,u8 *buf, u16 buf_len)
{

	#define BTINFO_WIFI_FETCH 0x23
	#define BTINFO_BT_AUTO_RPT 0x27
	struct btinfo_8761ATV *info = (struct btinfo_8761ATV *)buf;
	u8 cmd_idx;
	u8 len;

	cmd_idx = info->cid;

	if (info->len > buf_len-2) {
		WARN_ON(1);
		len = buf_len-2;
	} else {
		len = info->len;
	}

	btinfo_evt_dump(info);
	
	/* transform BT-FW btinfo to WiFI-FW C2H format and notify */
	if (cmd_idx == BTINFO_WIFI_FETCH) {
		buf[1] = 0;
	} else if (cmd_idx == BTINFO_BT_AUTO_RPT) {
		buf[1] = 2;
	} else if(0x01 == cmd_idx || 0x02 == cmd_idx) {
		//troy,it should run here
		buf[1] = buf[0];
	}
	
		
	priv->pshare->btcoexist.btc_ops->btc_btinfo_notify(priv,&buf[1],len+1);

}



u8 rtl_send_complete_event_to_BT(struct rtl8192cd_priv *priv,RTW_HCI_EXT_CMD BT_RELATED_CMD,RTL_HCI_STATUS status)
{
	RTL_HCI_event *pEvent;
	u8 localBuf[6] = "";
	u8	len=0,tx_event_length = 0;
	u8 *pRetPar;
	u8 *event_data = NULL;


	printk("#LEVEL_END,rtl_send_complete_event_to_BT\n");

	pEvent = (RTL_HCI_event*)(&localBuf[0]);
	event_data = pEvent->Data;
	pEvent->EventCode = HCI_EVENT_COMMAND_COMPLETE;
	*event_data = 0x1;	//packet #
	*(event_data + 1) = HCIOPCODELOW(BT_RELATED_CMD, OGF_EXTENSION);
	*(event_data + 2) = HCIOPCODEHIGHT(BT_RELATED_CMD, OGF_EXTENSION);

	len = len + 3;
	// Return parameters starts from here
	pRetPar = &pEvent->Data[len];		
	pRetPar[0] = status;		//status
	len++;
	pEvent->Length = len;
	//total tx event length + EventCode length + sizeof(length)
	tx_event_length = pEvent->Length + 2;
	rtl_btcoex_dump_tx_msg((u8 *)pEvent, tx_event_length,"rtl_send_complete_event_to_BT");
	status = rtl_btcoex_sendmsgbysocket(priv,(u8 *)pEvent, tx_event_length);
	return status;
	//bthci_IndicateEvent(Adapter, PPacketIrpEvent, len+2);


}


u8 rtl_btcoex_parse_BT_info_notify_cmd(struct rtl8192cd_priv *priv, u8 *pcmd, u16 cmdlen)
{
	
	
	u8 curPollEnable = pcmd[0];
	u8 curPollTime = pcmd[1];
	u8 btInfoReason = pcmd[2];
	u8 btInfoLen = pcmd[3];
	u8 btinfo[BT_INFO_LENGTH];


	RTL_HCI_STATUS status = HCI_STATUS_SUCCESS;



	DBG_871X("%s\n",__func__);
	DBG_871X("current Poll Enable: %d, currrent Poll Time: %d\n",curPollEnable,curPollTime);
	DBG_871X("BT Info reason: %d, BT Info length: %d\n",btInfoReason,btInfoLen);
	DBG_871X("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n"
		,pcmd[4],pcmd[5],pcmd[6],pcmd[7],pcmd[8],pcmd[9],pcmd[10],pcmd[11]);

	memset(btinfo, 0, BT_INFO_LENGTH);
	
#if 1
	if(BT_INFO_LENGTH != btInfoLen)
	{
		status = HCI_STATUS_INVALID_HCI_CMD_PARA_VALUE;
		DBG_871X("Error BT Info Length: %d\n",btInfoLen);
		//return _FAIL;
	}
	else
#endif
	{
		if(0x1 == btInfoReason || 0x2 == btInfoReason)
		{
			safe_memcpy(btinfo, &pcmd[4], btInfoLen,BT_INFO_LENGTH);
			btinfo[0] = btInfoReason;
			rtl_btcoex_btinfo_cmd(priv,btinfo,btInfoLen);
		}
		else
		{
			DBG_871X("Other BT info reason\n");
		}
	}

	return rtl_send_complete_event_to_BT(priv,HCI_BT_INFO_NOTIFY,status);

}



u8 rtl_btcoex_parse_BT_patch_ver_info_cmd(struct rtl8192cd_priv *priv, u8 *pcmd, u16 cmdlen)
{
	RTL_HCI_STATUS status=HCI_STATUS_SUCCESS;
	u16		btPatchVer=0x0, btHciVer=0x0;
	//u16		*pU2tmp;
	

	btHciVer = pcmd[0] | pcmd[1]<<8;
	btPatchVer = pcmd[2] | pcmd[3]<<8;


	DBG_871X("%s, cmd:%02x %02x %02x %02x\n",__func__, pcmd[0] ,pcmd[1] ,pcmd[2] ,pcmd[3]);
	DBG_871X("%s, HCI Ver:%d, Patch Ver:%d\n",__func__, btHciVer,btPatchVer);
	

	priv->pshare->btcoexist.btc_ops->btc_set_bt_patch_version(btHciVer, btPatchVer);

	return rtl_send_complete_event_to_BT(priv,HCI_BT_PATCH_VERSION_NOTIFY,status);

}





//used to set HCIExtensionVer

u8 rtl_btcoex_parse_HCI_Ver_notify_cmd(struct rtl8192cd_priv *priv, u8 *pcmd, u16 cmdlen)
{
	RTL_HCI_STATUS status=HCI_STATUS_SUCCESS;
	u16 hciver = pcmd[0] | pcmd[1] <<8;

	struct bt_coex_info *pcoex_info = &priv->pshare->coex_info;
	PBT_MGNT	pBtMgnt=&pcoex_info->BtMgnt;
	pBtMgnt->ExtConfig.HCIExtensionVer = hciver;
	DBG_871X("%s, HCI Version: %d\n",__func__,pBtMgnt->ExtConfig.HCIExtensionVer);
	if(pBtMgnt->ExtConfig.HCIExtensionVer  < 4)
	{
		status = HCI_STATUS_INVALID_HCI_CMD_PARA_VALUE;
		DBG_871X("%s, Version = %d, HCI Version < 4\n",__func__,pBtMgnt->ExtConfig.HCIExtensionVer );
	}
	else
	{
		priv->pshare->btcoexist.btc_ops->btc_set_hci_version(hciver);
	}


	return rtl_send_complete_event_to_BT(priv,HCI_EXTENSION_VERSION_NOTIFY,status);

}






u8 rtl_btcoex_parse_WIFI_scan_notify_cmd(struct rtl8192cd_priv *priv, u8 *pcmd, u16 cmdlen)
{
	RTL_HCI_STATUS status=HCI_STATUS_SUCCESS;
	
	struct bt_coex_info *pcoex_info = &priv->pshare->coex_info;
	PBT_MGNT	pBtMgnt=&pcoex_info->BtMgnt;
	pBtMgnt->ExtConfig.bEnableWifiScanNotify= pcmd[0];
	DBG_871X("%s, bEnableWifiScanNotify: %d\n",__func__,pBtMgnt->ExtConfig.bEnableWifiScanNotify);
	
	return rtl_send_complete_event_to_BT(priv,HCI_ENABLE_WIFI_SCAN_NOTIFY,status);

}


u8 rtl_btcoex_parse_HCI_link_status_notify_cmd(struct rtl8192cd_priv *priv, u8 *pcmd, u16 cmdlen)
{
	RTL_HCI_STATUS	status=HCI_STATUS_SUCCESS;
	struct bt_coex_info	*pcoex_info=&priv->pshare->coex_info;
	PBT_MGNT	pBtMgnt=&pcoex_info->BtMgnt;
	//PBT_DBG		pBtDbg=&padapter->MgntInfo.BtInfo.BtDbg;
	u8		i, numOfHandle=0;
	u16		conHandle;
	u8		btProfile, btCoreSpec, linkRole;
	u8		*pTriple;

	
	//pBtDbg->dbgHciInfo.hciCmdCntLinkStatusNotify++;
	//RT_DISP_DATA(FIOCTL, IOCTL_BT_HCICMD_EXT, "LinkStatusNotify, Hex Data :\n", 
	//		&pHciCmd->Data[0], pHciCmd->Length);

	//DBG_871X("BTLinkStatusNotify\n");

	// Current only RTL8723 support this command.
	//pBtMgnt->bSupportProfile = TRUE;
	pBtMgnt->bSupportProfile = false;

	pBtMgnt->ExtConfig.NumberOfACL = 0;
	pBtMgnt->ExtConfig.NumberOfSCO = 0;
	
	numOfHandle = pcmd[0];
	//RT_DISP(FIOCTL, IOCTL_BT_HCICMD_EXT, ("numOfHandle = 0x%x\n", numOfHandle));
	//RT_DISP(FIOCTL, IOCTL_BT_HCICMD_EXT, ("HCIExtensionVer = %d\n", pBtMgnt->ExtConfig.HCIExtensionVer));
	DBG_871X("numOfHandle = 0x%x\n", numOfHandle);
	DBG_871X("HCIExtensionVer = %d\n", pBtMgnt->ExtConfig.HCIExtensionVer);
	
	pTriple = &pcmd[1];
	for(i=0; i<numOfHandle; i++)
	{	
		if(pBtMgnt->ExtConfig.HCIExtensionVer < 1)
		{
			conHandle = *((u8 *)&pTriple[0]);
			btProfile = pTriple[2];
			btCoreSpec = pTriple[3];
			if(BT_PROFILE_SCO == btProfile)
			{
				pBtMgnt->ExtConfig.NumberOfSCO++;
			}
			else
			{
				pBtMgnt->ExtConfig.NumberOfACL++;			
				pBtMgnt->ExtConfig.aclLink[i].ConnectHandle = conHandle;
				pBtMgnt->ExtConfig.aclLink[i].BTProfile = btProfile;
				pBtMgnt->ExtConfig.aclLink[i].BTCoreSpec = btCoreSpec;
			}
			//RT_DISP(FIOCTL, IOCTL_BT_HCICMD_EXT, 
			//	("Connection_Handle=0x%x, BTProfile=%d, BTSpec=%d\n",
			//		conHandle, btProfile, btCoreSpec));
			DBG_871X("Connection_Handle=0x%x, BTProfile=%d, BTSpec=%d\n", conHandle, btProfile, btCoreSpec);
			pTriple += 4;
		}
		else if(pBtMgnt->ExtConfig.HCIExtensionVer >= 1)
		{
			conHandle = *((u16*)&pTriple[0]);
			btProfile = pTriple[2];
			btCoreSpec = pTriple[3];
			linkRole = pTriple[4];
			if(BT_PROFILE_SCO == btProfile)
			{
				pBtMgnt->ExtConfig.NumberOfSCO++;
			}
			else
			{
				pBtMgnt->ExtConfig.NumberOfACL++;			
				pBtMgnt->ExtConfig.aclLink[i].ConnectHandle = conHandle;
				pBtMgnt->ExtConfig.aclLink[i].BTProfile = btProfile;
				pBtMgnt->ExtConfig.aclLink[i].BTCoreSpec = btCoreSpec;
				pBtMgnt->ExtConfig.aclLink[i].linkRole = linkRole;
			}
			//RT_DISP(FIOCTL, IOCTL_BT_HCICMD_EXT, 
			DBG_871X("Connection_Handle=0x%x, BTProfile=%d, BTSpec=%d, LinkRole=%d\n",
				conHandle, btProfile, btCoreSpec, linkRole);
			pTriple += 5;
		}	
	}
	
	
	

	//rtw_btcoex_StackUpdateProfileInfo();
	priv->pshare->btcoexist.btc_ops->btc_stack_update_profile_info();


	return rtl_send_complete_event_to_BT(priv,HCI_LINK_STATUS_NOTIFY,status);
	
	
}



u8 rtl_btcoex_parse_HCI_BT_coex_notify_cmd(struct rtl8192cd_priv *priv, u8 *pcmd, u16 cmdlen)
{

	RTL_HCI_STATUS	status=HCI_STATUS_SUCCESS;
	return rtl_send_complete_event_to_BT(priv,HCI_BT_COEX_NOTIFY,status);

}



u8 rtl_btcoex_parse_HCI_BT_operation_notify_cmd(struct rtl8192cd_priv *priv, u8 *pcmd, u16 cmdlen)
{
	RTL_HCI_STATUS	status=HCI_STATUS_SUCCESS;

	DBG_871X("%s, OP code: %d\n",__func__,pcmd[0]);

	switch(pcmd[0])
	{
		//case HCI_BT_OP_NONE:
		case 0x0:
			DBG_871X("[bt operation] : Operation None!!\n");
			break;
		//case HCI_BT_OP_INQUIRY_START:
		case 0x1:
			DBG_871X("[bt operation] : Inquiry start!!\n");
			break;
		//case HCI_BT_OP_INQUIRY_FINISH:
		case 0x2:
			DBG_871X("[bt operation] : Inquiry finished!!\n");
			//return _SUCCESS;
			break;
		//case HCI_BT_OP_PAGING_START:
		case 0x3:
			DBG_871X("[bt operation] : Paging is started!!\n");
			break;
		//case HCI_BT_OP_PAGING_SUCCESS:
		case 0x4:
			DBG_871X("[bt operation] : Paging complete successfully!!\n");
			break;
		//case HCI_BT_OP_PAGING_UNSUCCESS:
		case 0x5:
			DBG_871X("[bt operation] : Paging complete unsuccessfully!!\n");
			break;
		//case HCI_BT_OP_PAIRING_START:
		case 0x6:
			DBG_871X("[bt operation] : Pairing start!!\n");
			break;
		//case HCI_BT_OP_PAIRING_FINISH:
		case 0x7:
			DBG_871X("[bt operation] : Pairing finished!!\n");
			break;
		//case HCI_BT_OP_BT_DEV_ENABLE:
		case 0x8:
			DBG_871X("[bt operation] : BT Device is enabled!!\n");
			break;
		//case HCI_BT_OP_BT_DEV_DISABLE:
		case 0x9:
			DBG_871X("[bt operation] : BT Device is disabled!!\n");
			break;
		default:
			DBG_871X("[bt operation] : Unknown, error!!\n");
			break;
	}


	return rtl_send_complete_event_to_BT(priv,HCI_BT_OPERATION_NOTIFY,status);
}


u8 rtl_btcoex_parse_BT_AFH_MAP_notify_cmd(struct rtl8192cd_priv *priv, u8 *pcmd, u16 cmdlen)
{
	RTL_HCI_STATUS	status=HCI_STATUS_SUCCESS;
	return rtl_send_complete_event_to_BT(priv,HCI_BT_AFH_MAP_NOTIFY,status);
}

u8 rtl_btcoex_parse_BT_register_val_notify_cmd(struct rtl8192cd_priv *priv, u8 *pcmd, u16 cmdlen)
{
	
	RTL_HCI_STATUS	status=HCI_STATUS_SUCCESS;
	return rtl_send_complete_event_to_BT(priv,HCI_BT_REGISTER_VALUE_NOTIFY,status);
}

u8 rtl_btcoex_parse_HCI_BT_abnormal_notify_cmd(struct rtl8192cd_priv *priv, u8 *pcmd, u16 cmdlen)
{
	RTL_HCI_STATUS	status=HCI_STATUS_SUCCESS;
	return rtl_send_complete_event_to_BT(priv,HCI_BT_ABNORMAL_NOTIFY,status);
}

u8 rtl_btcoex_parse_HCI_query_RF_status_cmd(struct rtl8192cd_priv *priv, u8 *pcmd, u16 cmdlen)
{
	RTL_HCI_STATUS	status=HCI_STATUS_SUCCESS;
	return rtl_send_complete_event_to_BT(priv,HCI_QUERY_RF_STATUS,status);
}

/*****************************************
* HCI cmd format :
*| 15 - 0						|	
*| OPcode (OCF|OGF<<10)		|
*| 15 - 8		|7 - 0			|
*|Cmd para 	|Cmd para Length	|
*|Cmd para......				|
******************************************/

//bit 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
//	 |	OCF			             |	   OGF       |
void rtl_btcoex_parse_hci_extend_cmd(struct rtl8192cd_priv *priv, u8 *pcmd, u16 len,const u16 hci_OCF)
{

	printk("#LEVEL2,");
	//DBG_871X("%s: OCF: %x\n",__func__,hci_OCF);
	switch(hci_OCF)
	{
		case HCI_EXTENSION_VERSION_NOTIFY:
			DBG_871X("HCI_EXTENSION_VERSION_NOTIFY\n");printk("#LEVEL3,");
			panic_printk("do nothing;"); // rtl_btcoex_parse_HCI_Ver_notify_cmd(priv,pcmd, len); //wishchen
			break;
		case HCI_LINK_STATUS_NOTIFY:
			DBG_871X("HCI_LINK_STATUS_NOTIFY\n");printk("#LEVEL3,");
			rtl_btcoex_parse_HCI_link_status_notify_cmd(priv,pcmd, len);
			break;
		case HCI_BT_OPERATION_NOTIFY:
			// only for 8723a 2ant
			DBG_871X("HCI_BT_OPERATION_NOTIFY\n");printk("#LEVEL3,");
			rtl_btcoex_parse_HCI_BT_operation_notify_cmd(priv,pcmd, len);
			//
			break;
		case HCI_ENABLE_WIFI_SCAN_NOTIFY:
			DBG_871X("HCI_ENABLE_WIFI_SCAN_NOTIFY\n");printk("#LEVEL3,");
			rtl_btcoex_parse_WIFI_scan_notify_cmd(priv,pcmd, len);
			break;
		case HCI_QUERY_RF_STATUS:
			// only for 8723b 2ant
			DBG_871X("HCI_QUERY_RF_STATUS\n");printk("#LEVEL3,");
			rtl_btcoex_parse_HCI_query_RF_status_cmd(priv,pcmd, len);
			break;
		case HCI_BT_ABNORMAL_NOTIFY:
			DBG_871X("HCI_BT_ABNORMAL_NOTIFY\n");printk("#LEVEL3,");
			rtl_btcoex_parse_HCI_BT_abnormal_notify_cmd(priv,pcmd, len);
			break;
		case HCI_BT_INFO_NOTIFY:
			DBG_871X("HCI_BT_INFO_NOTIFY\n");printk("#LEVEL3,");
			rtl_btcoex_parse_BT_info_notify_cmd(priv,pcmd, len);
			break;
		case HCI_BT_COEX_NOTIFY:
			DBG_871X("HCI_BT_COEX_NOTIFY\n");printk("#LEVEL3,");
			rtl_btcoex_parse_HCI_BT_coex_notify_cmd(priv,pcmd, len);
			break;
		case HCI_BT_PATCH_VERSION_NOTIFY:
			DBG_871X("HCI_BT_PATCH_VERSION_NOTIFY\n");printk("#LEVEL3,");
			rtl_btcoex_parse_BT_patch_ver_info_cmd(priv,pcmd, len);
			break;
		case HCI_BT_AFH_MAP_NOTIFY:
			DBG_871X("HCI_BT_AFH_MAP_NOTIFY\n");printk("#LEVEL3,");
			rtl_btcoex_parse_BT_AFH_MAP_notify_cmd(priv,pcmd, len);
			break;
		case HCI_BT_REGISTER_VALUE_NOTIFY:
			DBG_871X("HCI_BT_REGISTER_VALUE_NOTIFY\n");printk("#LEVEL3,");
			rtl_btcoex_parse_BT_register_val_notify_cmd(priv,pcmd, len);
			break;
		default:
			DBG_871X("ERROR!!! Unknown OCF: %x\n",hci_OCF);
			break;
			
	}
}

void rtl_btcoex_parse_hci_cmd(struct rtl8192cd_priv *priv, u8 *pcmd, u16 len)
{
	u16 opcode = pcmd[0] | pcmd[1]<<8;
	u16 hci_OGF = HCI_OGF(opcode);
	u16 hci_OCF = HCI_OCF(opcode);
	u8 cmdlen = len -3;
	//u8 pare_len = pcmd[2];

	//DBG_871X("%s\n",__func__);
	DBG_871X("#LEVEL1,OGF: %x,OCF: %x\n",hci_OGF,hci_OCF);
	switch(hci_OGF)
	{
		case OGF_EXTENSION:
			DBG_871X("#LEVEL1,HCI_EXTENSION_CMD_OGF\n");
			rtl_btcoex_parse_hci_extend_cmd(priv, &pcmd[3], cmdlen, hci_OCF);
			break;
		default:
			DBG_871X("#LEVEL1,Other OGF: %x\n",hci_OGF);
			break;
	}
}



u16 rtl_btcoex_parse_recv_data(u8 *msg, u8 msg_size)
{
	//troy risk
	u8 *cmp_msg1 = attend_ack;
	u8 *cmp_msg2 = leave_ack;
	u8 *cmp_msg3 = bt_leave;
	u8 *cmp_msg4 = invite_req;
	u8 *cmp_msg5 = attend_req;
	u8 *cmp_msg6 = invite_rsp;
	//u8 btinfonotifycmd[2] = {0x06,0x01};
	u8 res = OTHER;


	DBG_871X("\n>>>>>>>>>>>>>>>>>>>>>>>BT_TO_WIFI");


	if(memcmp(cmp_msg1,msg,msg_size) == 0)
	{
		res = RX_ATTEND_ACK;
	}
	else if(memcmp(cmp_msg2,msg,msg_size) == 0)
	{
		res = RX_LEAVE_ACK;
	}
	else if(memcmp(cmp_msg3,msg,msg_size) == 0)
	{
		res = RX_BT_LEAVE;
	}
	else if(memcmp(cmp_msg4,msg,msg_size) == 0)
	{
		res = RX_INVITE_REQ;
	}
	else if(memcmp(cmp_msg5,msg,msg_size) == 0)
	{
		res = RX_ATTEND_REQ;
	}
	else if(memcmp(cmp_msg6,msg,msg_size) == 0)
	{
		res = RX_INVITE_RSP;
	}
#if 0
	else if (memcmp(btinfonotifycmd,msg,sizeof(btinfonotifycmd) == 0)
	{	
		DBG_871X("%s, OCF:%02x%02x\n",__func__,msg[0],msg[1]);
		DBG_871X("%s, msg:BT_INFO_NOTIFY_CMD\n",__func__);
		res = BT_INFO_NOTIFY_CMD;
	}
#endif
	else
	{
		//DBG_871X("%s, OGF|OCF:%02x%02x\n",__func__,msg[1],msg[0]);
		res = OTHER;
		printk(",other_cmd!\n");
	}


	if(OTHER != res)
		printk(",base_cmd:%s\n",msg);


	
	return res;
}


void rtl_btcoex_recvmsg_int(struct sock *sk_in, s32 bytes){


	struct rtl8192cd_priv *priv = pbtcoexadapter;	
	struct bt_coex_info *pcoex_info = &priv->pshare->coex_info;
	int i =0;
	printk("%%%%%%%%%%%%%%%%%%%%%%int\n");

	sk_store = sk_in;
	queue_delayed_work(priv->pshare->rtl_wq,
		&priv->pshare->socket_wq, 0);

}




void rtl_btcoex_recvmsgbysocket(void* data) {     //struct sock *sk, s32 bytes){



	
	u8 recv_data[RECV_DATA_MAX_LEN];
	u32 len = 0;
	u16 recv_length = 0;
	u16 parse_res = 0;
	int i =0;

	struct rtl8192cd_priv *priv = pbtcoexadapter;
	struct bt_coex_info *pcoex_info = &pbtcoexadapter->pshare->coex_info;
	struct sock *sk = sk_store;
	struct sk_buff * skb = NULL;

	if(sk == NULL)
		printk("***************critical error!\n");

	len = skb_queue_len(&sk->sk_receive_queue);
	//DBG_871X("skb queue len %i\n",len);
	while(len > 0){
			skb = skb_dequeue(&sk->sk_receive_queue);

			/*important: cut the udp header from skb->data!
			  header length is 8 byte*/
			recv_length = skb->len-8;
			memset(recv_data,0,sizeof(recv_data));
			safe_memcpy(recv_data, skb->data+8, recv_length, RECV_DATA_MAX_LEN);

			//DBG_871X("received data: %s :with len %u\n",recv_data, skb->len);
			parse_res = rtl_btcoex_parse_recv_data(recv_data,recv_length);
			//DBG_871X("parse_res; %d\n",parse_res);
			if(RX_ATTEND_ACK == parse_res) //attend ack
			{
				pcoex_info ->BT_attend = true;
				DBG_871X("RX_ATTEND_ACK!,sock_open:%d, BT_attend:%d\n",pcoex_info ->sock_open,pcoex_info ->BT_attend);
				
			}
			if(RX_ATTEND_REQ == parse_res) //attend req from BT
			{
				pcoex_info ->BT_attend = true;
				DBG_871X("RX_BT_ATTEND_REQ!,sock_open:%d, BT_attend:%d\n",pcoex_info ->sock_open,pcoex_info ->BT_attend);
				rtl_btcoex_sendmsgbysocket(pbtcoexadapter,attend_ack,sizeof(attend_ack));
			}		
			if(RX_INVITE_REQ == parse_res) //attend req from BT
			{
				pcoex_info ->BT_attend = true;
				DBG_871X("RX_INVITE_REQ!,sock_open:%d, BT_attend:%d\n",pcoex_info ->sock_open,pcoex_info ->BT_attend);
				rtl_btcoex_sendmsgbysocket(pbtcoexadapter,invite_rsp,sizeof(invite_rsp));
			}	
			if(RX_INVITE_RSP == parse_res) //attend req from BT
			{
				pcoex_info ->BT_attend = true;
				DBG_871X("RX_INVITE_RSP!,sock_open:%d, BT_attend:%d\n",pcoex_info ->sock_open,pcoex_info ->BT_attend);
			}	
			else if (RX_LEAVE_ACK == parse_res) //mean BT know wifi  will leave
			{
				pcoex_info ->BT_attend = false;
				DBG_871X("RX_LEAVE_ACK!,sock_open:%d, BT_attend:%d\n",pcoex_info ->sock_open,pcoex_info ->BT_attend);
			
			}
			else if(RX_BT_LEAVE == parse_res) //BT leave
			{
				rtl_btcoex_sendmsgbysocket(pbtcoexadapter, leave_ack,sizeof(leave_ack)); // no ack
				pcoex_info ->BT_attend = false;
				DBG_871X("RX_BT_LEAVE!sock_open:%d, BT_attend:%d\n",pcoex_info ->sock_open,pcoex_info ->BT_attend);
			}
			else//todo: check if recv data are really hci cmds
			{
				if(true == pcoex_info->BT_attend)
					rtl_btcoex_parse_hci_cmd(pbtcoexadapter,recv_data,recv_length);
			}
			len--;

			kfree_skb(skb);
			/*never do a sleep in this context!*/
	}

	
}




u8 rtl_btcoex_sendmsgbysocket_normal(struct rtl8192cd_priv *priv, u8 *msg, u8 msg_size) {

	u8 error; 
	struct msghdr	udpmsg; 
	mm_segment_t	oldfs; 
	struct iovec	iov; 
	struct bt_coex_info *pcoex_info = &priv->pshare->coex_info;


		DBG_871X("<<<<<<<<<<<<<<<<<<<<<<<<WIFI_TO_BT,msg:%s , size=%d\n",msg, msg_size);

		iov.iov_base	 = (void *)msg; 
		iov.iov_len	 = msg_size; 
		udpmsg.msg_name	 = &pcoex_info->bt_addr; 
		udpmsg.msg_namelen	= sizeof(struct sockaddr_in); 
		udpmsg.msg_iov	 = &iov; 
		udpmsg.msg_iovlen	= 1; 
		udpmsg.msg_control	= NULL; 
		udpmsg.msg_controllen = 0; 
		udpmsg.msg_flags	= MSG_DONTWAIT | MSG_NOSIGNAL; 
		udpmsg.msg_flags = 0;
		oldfs = get_fs(); 
		set_fs(KERNEL_DS); 
		error = sock_sendmsg(pcoex_info->udpsock, &udpmsg, msg_size); 
		//rtw_msleep_os(20);
		set_fs(oldfs); 
		if(error < 0) {
			DBG_871X("Error when sendimg msg, error:%d\n",error); 
			return _FAIL;
		} else{
			DBG_871X("rtl_btcoex_sendmsgbysocket_normal OK\n");
		}
		

		return _SUCCESS;


}


u8 rtl_btcoex_sendmsgbysocket_force(struct rtl8192cd_priv *priv, u8 *msg, u8 msg_size) {

	return rtl_btcoex_sendmsgbysocket_normal(priv, msg, msg_size);

}
u8 rtl_btcoex_sendmsgbysocket(struct rtl8192cd_priv *priv, u8 *msg, u8 msg_size) {

	struct bt_coex_info *pcoex_info = &priv->pshare->coex_info;

	if(true == pcoex_info->BT_attend) {
		return rtl_btcoex_sendmsgbysocket_normal(priv, msg, msg_size);
	} else {
		DBG_871X("Tx error: BT isn't up\n"); 
		return _FAIL;
	}
}


u8 rtl_btcoex_create_kernel_socket(struct rtl8192cd_priv *priv, u8 is_invite) {

	s8 kernel_socket_err; 
	int i;
	struct bt_coex_info *pcoex_info = &priv->pshare->coex_info;
	panic_printk("%s CONNECT_PORT %d\n",__func__,CONNECT_PORT);


	if(NULL == pcoex_info)
	{
		panic_printk("coex_info: NULL\n"); 
		return _FAIL;
	}
	
	kernel_socket_err = sock_create(PF_INET, SOCK_DGRAM, 0, &pcoex_info->udpsock); 
	panic_printk("binding socket,err= %d\n",kernel_socket_err);
	
	if (kernel_socket_err<0) 
	{ 
		panic_printk("Error during creation of socket error:%d\n",kernel_socket_err); 
		return _FAIL;
		
	} 
	else
	{
		memset(&(pcoex_info->sin), 0, sizeof(pcoex_info->sin)); 
		pcoex_info->sin.sin_family = AF_INET; 
		pcoex_info->sin.sin_port = htons(CONNECT_PORT); 
		pcoex_info->sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

		memset(&(pcoex_info->bt_addr), 0, sizeof(pcoex_info->bt_addr)); 
		pcoex_info->bt_addr.sin_family = AF_INET; 
		pcoex_info->bt_addr.sin_port = htons(CONNECT_PORT_BT); 
		pcoex_info->bt_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		

		sk_store = NULL;

		
		kernel_socket_err = pcoex_info->udpsock->ops->bind(pcoex_info->udpsock,(struct sockaddr *)&pcoex_info->sin,sizeof(pcoex_info->sin)); 

		// there is one process using this IP(127.0.0.1), means BT is on.
		//if(kernel_socket_err == -98){	
		if(kernel_socket_err == 0){	
			panic_printk("binding socket success\n"); 
			pcoex_info->udpsock->sk->sk_data_ready = rtl_btcoex_recvmsg_int;//rtw_btcoex_recvmsgbysocket;
			pcoex_info->sock_open |=  KERNEL_SOCKET_OK;
			pcoex_info->BT_attend = false;//troy temp

			//if(is_invite != true) // attend req and invite_req
			panic_printk("WIFI sending attend_req\n");
			rtl_btcoex_sendmsgbysocket_force(priv,attend_req,sizeof(attend_req));
			//rtw_btcoex_sendmsgbysocket(rtlpriv,invite_req,sizeof(invite_req));

			return _SUCCESS;
		}
		else
		{ 
			pcoex_info->BT_attend = false;
			sock_release(pcoex_info->udpsock); // bind fail release socket
			panic_printk("Error binding socket: %d\n",kernel_socket_err); 
			return _FAIL;
		} 
			
	}


}


void rtl_btcoex_close_kernel_socket(struct rtl8192cd_priv *priv) {

	struct bt_coex_info *pcoex_info = &priv->pshare->coex_info;
	if(pcoex_info->sock_open & KERNEL_SOCKET_OK)
	{
		DBG_871X("release kernel socket\n");

		cancel_delayed_work(&priv->pshare->socket_wq);

		sock_release(pcoex_info->udpsock);
		pcoex_info->sock_open &= ~(KERNEL_SOCKET_OK);	
		if(true == pcoex_info->BT_attend)
			pcoex_info->BT_attend = false;
		
		DBG_871X("sock_open:%d, BT_attend:%d\n",pcoex_info ->sock_open,pcoex_info ->BT_attend);
	}
	
}


void rtl_btcoex_init_socket(struct rtl8192cd_priv *priv) {

	u8 is_invite = false;
	struct bt_coex_info *pcoex_info = &priv->pshare->coex_info;
	panic_printk("[%s] init socket with 8822BEH\n",__FUNCTION__);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
		/*<delete in kernel end>*/
			priv->pshare->rtl_wq = alloc_workqueue("rtl_work_queue", 0, 0);
		/*<delete in kernel start>*/
#else
			priv->pshare->rtl_wq = create_workqueue("rtl_work_queue");
#endif

	if(!priv->pshare->rtl_wq){  
			panic_printk("[%s] Create workqueue failed!\n",__FUNCTION__);	
			return;	  
	}  

	INIT_DELAYED_WORK(&priv->pshare->socket_wq, (void *)rtl_btcoex_recvmsgbysocket);

	if(false == pcoex_info->is_exist)
	{
		memset(pcoex_info,0,sizeof(struct bt_coex_info));
		pbtcoexadapter = priv;
		//rtl_btcoex_create_nl_socket(padapter);
		rtl_btcoex_create_kernel_socket(priv,is_invite);
		pcoex_info->is_exist = true;
		panic_printk("========> coex_info->is_exist: %d\n",pcoex_info->is_exist);
	}
}

void rtl_btcoex_close_socket(struct rtl8192cd_priv *priv) {


	struct bt_coex_info *pcoex_info = &priv->pshare->coex_info;
	printk("========> coex_info->is_exist: %d\n",pcoex_info->is_exist);
	if( true == pcoex_info->is_exist)
	{
		pcoex_info->is_exist = false;
		if(pcoex_info->BT_attend == true) //inform BT wifi leave
		{
			rtl_btcoex_sendmsgbysocket(priv,wifi_leave,sizeof(wifi_leave));
			msleep(50);
		}
		//rtl_btcoex_close_nl_socket(padapter);
		rtl_btcoex_close_kernel_socket(priv);
		pbtcoexadapter = NULL;
	}
}



void rtl_btcoex_dump_tx_msg(u8 *tx_msg, u8 len, u8 *msg_name)
{
	
	u8 	i = 0;



	return;
	//DBG_871X("%s______________\n",msg_name);
	for(i=0;i<len;i++)
	{
		printk("%02x ",tx_msg[i]);
	}
	printk("\n");

}









void rtl_btcoex_SendEventExtBtCoexControl(struct rtl8192cd_priv *priv, u8 bNeedDbgRsp, u8 dataLen, void *pData)
{
	
	u8			len=0, tx_event_length = 0;
	u8 			localBuf[32] = "";
	u8			*pRetPar;
	u8			opCode=0;
	u8			*pInBuf=(u8 *)pData;
	//u8			*pOpCodeContent;
	RTL_HCI_event *pEvent;



	printk("#LEVEL_WIFI_ACTIVE,SendEventExtBtCoexControl\n");


	opCode = pInBuf[0];	

	//DBG_871X("%s, OPCode:%02x\n",__func__,opCode);

	pEvent = (RTL_HCI_event*)(&localBuf[0]);

	//len += bthci_ExtensionEventHeaderRtk(&localBuf[0], 
	//	HCI_EVENT_EXT_BT_COEX_CONTROL);
	pEvent->EventCode = HCI_EVENT_EXTENSION_RTK;
	pEvent->Data[0] = HCI_EVENT_EXT_BT_COEX_CONTROL;	//extension event code
	len ++;
	// Return parameters starts from here
	//pRetPar = &pEvent->Data[len];
	pRetPar = pEvent->Data + len;
	memcpy(&pRetPar[0], pData, dataLen); /*maybe not safe here*/
	len += dataLen;
	pEvent->Length = len;
	//total tx event length + EventCode length + sizeof(length)
	tx_event_length = pEvent->Length + 2;
	rtl_btcoex_dump_tx_msg((u8 *)pEvent, tx_event_length, "BT COEX CONTROL");
	rtl_btcoex_sendmsgbysocket(priv,(u8 *)pEvent, tx_event_length);

}


void rtl_btcoex_SendEventExtBtInfoControl(struct rtl8192cd_priv *priv, u8 dataLen, void *pData)
{
	
	RTL_HCI_event *pEvent;
	u8			*pRetPar;
	u8			len=0, tx_event_length = 0;
	u8 			localBuf[32] = "";

	struct bt_coex_info *pcoex_info = &priv->pshare->coex_info;
	PBT_MGNT		pBtMgnt = &pcoex_info->BtMgnt;


	printk("#LEVEL_WIFI_ACTIVE,SendEventExtBtInfoControl\n");

	//DBG_871X("%s\n",__func__);
	if(pBtMgnt->ExtConfig.HCIExtensionVer < 4) //not support
	{
		DBG_871X("ERROR: HCIExtensionVer = %d, HCIExtensionVer<4 !!!!\n",pBtMgnt->ExtConfig.HCIExtensionVer);
		return;
	}

	pEvent = (RTL_HCI_event *)(&localBuf[0]);
	pEvent->EventCode = HCI_EVENT_EXTENSION_RTK;
	pEvent->Data[0] = HCI_EVENT_EXT_BT_INFO_CONTROL;
	len ++;
	//pRetPar = &pEvent->Data[len];
	pRetPar = pEvent->Data + len;

	memcpy(&pRetPar[0], pData, dataLen);/*maybe not safe here*/
	len += dataLen;
	pEvent->Length = len;
	//total tx event length + EventCode length + sizeof(length)
	tx_event_length = pEvent->Length + 2;
	rtl_btcoex_dump_tx_msg((u8 *)pEvent, tx_event_length, "BT INFO CONTROL");
	rtl_btcoex_sendmsgbysocket(priv,(u8 *)pEvent, tx_event_length);
	
}

void rtl_btcoex_SendScanNotify(struct rtl8192cd_priv *priv, u8 scanType)
{
	
	u8	tx_event_length=0;
	u8 	localBuf[7] = "";
	RTL_HCI_event *pEvent;
	u8 *event_data = NULL;

	printk("#LEVEL_WIFI_ACTIVE,SendScanNotify\n");
	pEvent = (RTL_HCI_event *)(&localBuf[0]);
	pEvent->EventCode = HCI_EVENT_EXTENSION_RTK;
	event_data = pEvent->Data;
	*(event_data) = HCI_EVENT_EXT_WIFI_SCAN_NOTIFY;
	*(event_data + 1) = scanType;
	pEvent->Length = 2;
	//total tx event length + EventCode length + sizeof(length)
	tx_event_length = pEvent->Length + 2;
	rtl_btcoex_dump_tx_msg((u8 *)pEvent, tx_event_length, "WIFI SCAN OPERATION");
	rtl_btcoex_sendmsgbysocket(priv, (u8 *)pEvent, tx_event_length);
}





