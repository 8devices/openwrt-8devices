#include "./btcoexist_cmd.h"

/*========================================================mp cmd*/

int rtw_mp_disable_bt_coexist(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
#if 0
	PADAPTER padapter = (PADAPTER)rtw_netdev_priv(dev);
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct hal_ops *pHalFunc = &padapter->HalFunc;

	u8 input[wrqu->data.length];
	u32 bt_coexist;

	if (copy_from_user(input, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	bt_coexist = rtw_atoi(input);

	if (bt_coexist == 0) {
		RTW_INFO("Set OID_RT_SET_DISABLE_BT_COEXIST: disable BT_COEXIST\n");
#ifdef CONFIG_BT_COEXIST
		rtw_btcoex_HaltNotify(padapter);
		rtw_btcoex_SetManualControl(padapter, _TRUE);
		/* Force to switch Antenna to WiFi*/
		rtw_write16(padapter, 0x870, 0x300);
		rtw_write16(padapter, 0x860, 0x110);
#endif
		/* CONFIG_BT_COEXIST */
	} else {
#ifdef CONFIG_BT_COEXIST
		rtw_btcoex_SetManualControl(padapter, _FALSE);
#endif
	}
#endif
	return 0;
}


/*========================================================efuse cmd*/
//Hal_EfuseParseBTCoexistInfo8812A


/*========================================================eeprom cmd*/
//Hal_ReadAntennaDiversity8812A

/*========================================================proc cmd*/
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

#if 0

static void rtw_btinfo_hdl(BT_COEX_HAL_ADAPTER *adapter, u8 *buf, u16 buf_len)
{
#define BTINFO_WIFI_FETCH 0x23
#define BTINFO_BT_AUTO_RPT 0x27
#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
	struct btinfo_8761ATV *info = (struct btinfo_8761ATV *)buf;
#else /* !CONFIG_BT_COEXIST_SOCKET_TRX */
	struct btinfo *info = (struct btinfo *)buf;
#endif /* CONFIG_BT_COEXIST_SOCKET_TRX */
	u8 cmd_idx;
	u8 len;

	cmd_idx = info->cid;

	if (info->len > buf_len - 2) {
		rtw_warn_on(1);
		len = buf_len - 2;
	} else
		len = info->len;

	/* #define DBG_PROC_SET_BTINFO_EVT */
#ifdef DBG_PROC_SET_BTINFO_EVT
#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
	RTW_INFO("%s: btinfo[0]=%x,btinfo[1]=%x,btinfo[2]=%x,btinfo[3]=%x btinfo[4]=%x,btinfo[5]=%x,btinfo[6]=%x,btinfo[7]=%x\n"
	, __func__, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
#else/* !CONFIG_BT_COEXIST_SOCKET_TRX */
	btinfo_evt_dump(info);
#endif /* CONFIG_BT_COEXIST_SOCKET_TRX */
#endif /* DBG_PROC_SET_BTINFO_EVT */

	/* transform BT-FW btinfo to WiFI-FW C2H format and notify */
	if (cmd_idx == BTINFO_WIFI_FETCH)
		buf[1] = 0;
	else if (cmd_idx == BTINFO_BT_AUTO_RPT)
		buf[1] = 2;
#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
	else if (0x01 == cmd_idx || 0x02 == cmd_idx)
		buf[1] = buf[0];
#endif /* CONFIG_BT_COEXIST_SOCKET_TRX */
	rtw_btcoex_BtInfoNotify(adapter , len + 1, &buf[1]);
}






void ConstructBtNullFunctionData(
	PADAPTER padapter,
	u8		*pframe,
	u32		*pLength,
	u8		*StaAddr,
	u8		bQoS,
	u8		AC,
	u8		bEosp,
	u8		bForcePowerSave)
{
#if 0	
	struct rtw_ieee80211_hdr	*pwlanhdr;
	u16						*fctrl;
	u32						pktlen;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8 bssid[ETH_ALEN];

	/* RTW_INFO("%s:%d\n", __FUNCTION__, bForcePowerSave); */

	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	if (NULL == StaAddr) {
		_rtw_memcpy(bssid, adapter_mac_addr(padapter), ETH_ALEN);
		StaAddr = bssid;
	}

	fctrl = &pwlanhdr->frame_ctl;
	*(fctrl) = 0;
	if (bForcePowerSave)
		SetPwrMgt(fctrl);

	SetFrDs(fctrl);
	_rtw_memcpy(pwlanhdr->addr1, StaAddr, ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr2, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr3, adapter_mac_addr(padapter), ETH_ALEN);

	SetDuration(pwlanhdr, 0);
	SetSeqNum(pwlanhdr, 0);

	if (bQoS == _TRUE) {
		struct rtw_ieee80211_hdr_3addr_qos *pwlanqoshdr;

		SetFrameSubType(pframe, WIFI_QOS_DATA_NULL);

		pwlanqoshdr = (struct rtw_ieee80211_hdr_3addr_qos *)pframe;
		SetPriority(&pwlanqoshdr->qc, AC);
		SetEOSP(&pwlanqoshdr->qc, bEosp);

		pktlen = sizeof(struct rtw_ieee80211_hdr_3addr_qos);
	} else {
		SetFrameSubType(pframe, WIFI_DATA_NULL);

		pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);
	}

	*pLength = pktlen;
#endif
}



static void SetFwRsvdPagePkt_BTCoex(BT_COEX_HAL_ADAPTER *padapter)
{
#if 0	
	PHAL_DATA_TYPE pHalData;
	struct xmit_frame	*pcmdframe;
	struct pkt_attrib	*pattrib;
	struct xmit_priv	*pxmitpriv;
	struct mlme_ext_priv	*pmlmeext;
	struct mlme_ext_info	*pmlmeinfo;
	struct pwrctrl_priv *pwrctl;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	u32	BeaconLength = 0;
	u32	NullDataLength = 0, QosNullLength = 0, BTQosNullLength = 0;
	u32	ProbeReqLength = 0;
	u8	*ReservedPagePacket;
	u8	TxDescLen = TXDESC_SIZE, TxDescOffset = TXDESC_OFFSET;
	u8	TotalPageNum = 0, CurtPktPageNum = 0, RsvdPageNum = 0;
	u16	BufIndex, PageSize = 256;
	u32	TotalPacketLen, MaxRsvdPageBufSize = 0;
	RSVDPAGE_LOC	RsvdPageLoc;


	if (IS_HARDWARE_TYPE_8812(padapter))
		PageSize = 512;
	else if (IS_HARDWARE_TYPE_8821(padapter))
		PageSize = PAGE_SIZE_TX_8821A;
	/* RTW_INFO("%s---->\n", __FUNCTION__); */

	pHalData = GET_HAL_DATA(padapter);

	pxmitpriv = &padapter->xmitpriv;
	pmlmeext = &padapter->mlmeextpriv;
	pmlmeinfo = &pmlmeext->mlmext_info;
	pwrctl = adapter_to_pwrctl(padapter);

	/* RsvdPageNum = BCNQ_PAGE_NUM_8723B + WOWLAN_PAGE_NUM_8723B; */

	if (IS_HARDWARE_TYPE_8812(padapter))
		RsvdPageNum = BCNQ_PAGE_NUM_8812;
	else if (IS_HARDWARE_TYPE_8821(padapter))
		RsvdPageNum = BCNQ_PAGE_NUM_8821;
	MaxRsvdPageBufSize = RsvdPageNum * PageSize;

	pcmdframe = rtw_alloc_cmdxmitframe(pxmitpriv);
	if (pcmdframe == NULL) {
		RTW_INFO("%s: alloc ReservedPagePacket fail!\n", __FUNCTION__);
		return;
	}

	ReservedPagePacket = pcmdframe->buf_addr;
	_rtw_memset(&RsvdPageLoc, 0, sizeof(RSVDPAGE_LOC));

	/* 3 (1) beacon */
	BufIndex = TxDescOffset;
	ConstructBeacon(padapter, &ReservedPagePacket[BufIndex], &BeaconLength);

	/* When we count the first page size, we need to reserve description size for the RSVD */
	/* packet, it will be filled in front of the packet in TXPKTBUF. */
	CurtPktPageNum = (u8)PageNum(TxDescLen + BeaconLength, PageSize);

	/* If we don't add 1 more page, the WOWLAN function has a problem. Baron thinks it's a bug of firmware */
	if (CurtPktPageNum == 1)
		CurtPktPageNum += 1;
	TotalPageNum += CurtPktPageNum;

	BufIndex += (CurtPktPageNum * PageSize);

	/* Jump to lastest third page *;RESERV 2 PAGES for TxBF NDPA */
	if (BufIndex < (MaxRsvdPageBufSize - PageSize)) {
		BufIndex = TxDescOffset + (MaxRsvdPageBufSize - PageSize);
		if (IS_HARDWARE_TYPE_8812(padapter))
			TotalPageNum = BCNQ_PAGE_NUM_8812 - 2;
		else if (IS_HARDWARE_TYPE_8821(padapter))
			TotalPageNum = BCNQ_PAGE_NUM_8821 - 2;

	}

	/* 3 (6) BT Qos null data */
	RsvdPageLoc.LocBTQosNull = TotalPageNum;
	ConstructBtNullFunctionData(
		padapter,
		&ReservedPagePacket[BufIndex],
		&BTQosNullLength,
		NULL,
		_TRUE, 0, 0, _FALSE);
	rtl8812a_fill_fake_txdesc(padapter, &ReservedPagePacket[BufIndex - TxDescLen], BTQosNullLength, _FALSE, _TRUE,  _FALSE);

	/* RTW_INFO("%s(): HW_VAR_SET_TX_CMD: BT QOS NULL DATA %p %d\n",  */
	/*	__FUNCTION__, &ReservedPagePacket[BufIndex-TxDescLen], (BTQosNullLength+TxDescLen)); */

	CurtPktPageNum = (u8)PageNum(TxDescLen + BTQosNullLength, PageSize);

	TotalPageNum += CurtPktPageNum;

	TotalPacketLen = BufIndex + BTQosNullLength;
	if (TotalPacketLen > MaxRsvdPageBufSize) {
		RTW_INFO("%s(): ERROR: The rsvd page size is not enough!!TotalPacketLen %d, MaxRsvdPageBufSize %d\n", __FUNCTION__,
			 TotalPacketLen, MaxRsvdPageBufSize);
		goto error;
	} else {
		/* update attribute */
		pattrib = &pcmdframe->attrib;
		update_mgntframe_attrib(padapter, pattrib);
		pattrib->qsel = QSLT_BEACON;
		pattrib->pktlen = pattrib->last_txcmdsz = TotalPacketLen - TxDescOffset;
#ifdef CONFIG_PCI_HCI
		dump_mgntframe(padapter, pcmdframe);
#else
		dump_mgntframe_and_wait(padapter, pcmdframe, 100);
#endif
	}

	RTW_INFO("%s: Set RSVD page location to Fw ,TotalPacketLen(%d), TotalPageNum(%d)\n", __FUNCTION__, TotalPacketLen, TotalPageNum);
	if (check_fwstate(pmlmepriv, _FW_LINKED)) {
		rtl8812_set_FwRsvdPage_cmd(padapter, &RsvdPageLoc);
#ifdef CONFIG_WOWLAN
		rtl8812_set_FwAoacRsvdPage_cmd(padapter, &RsvdPageLoc);
#endif
	}

	return;

error:

	rtw_free_xmitframe(pxmitpriv, pcmdframe);
#endif	
}


void rtl8812a_download_BTCoex_AP_mode_rsvd_page(BT_COEX_HAL_ADAPTER *padapter)
{
#if 0	
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	BOOLEAN bRecover = _FALSE;
	BOOLEAN	bcn_valid = _FALSE;
	u8	DLBcnCount = 0;
	u32 poll = 0;
	u8 val8;
	u8 v8;


	RTW_INFO("+" FUNC_ADPT_FMT ": hw_port=%d\n",
		FUNC_ADPT_ARG(padapter), get_hw_port(padapter));

	/* We should set AID, correct TSF, HW seq enable before set JoinBssReport to Fw in 88/92C. */
	/* Suggested by filen. Added by tynli. */
	rtw_write16(padapter, REG_BCN_PSR_RPT, (0xC000 | pmlmeinfo->aid));

	/* set REG_CR bit 8 */
	v8 = rtw_read8(padapter, REG_CR + 1);
	v8 |= BIT(0); /* ENSWBCN */
	rtw_write8(padapter,  REG_CR + 1, v8);

	/* Disable Hw protection for a time which revserd for Hw sending beacon. */
	/* Fix download reserved page packet fail that access collision with the protection time. */
	/* 2010.05.11. Added by tynli. */
	val8 = rtw_read8(padapter, REG_BCN_CTRL);
	val8 &= ~BIT(3);
	val8 |= BIT(4);
	rtw_write8(padapter, REG_BCN_CTRL, val8);

	/* Set FWHW_TXQ_CTRL 0x422[6]=0 to tell Hw the packet is not a real beacon frame. */
	if (pHalData->RegFwHwTxQCtrl & BIT(6))
		bRecover = _TRUE;

	/* To tell Hw the packet is not a real beacon frame. */
	rtw_write8(padapter, REG_FWHW_TXQ_CTRL + 2, pHalData->RegFwHwTxQCtrl & ~BIT(6));
	pHalData->RegFwHwTxQCtrl &= ~BIT(6);

	/* Clear beacon valid check bit. */
	rtw_hal_set_hwreg(padapter, HW_VAR_BCN_VALID, NULL);
	rtw_hal_set_hwreg(padapter, HW_VAR_DL_BCN_SEL, NULL);

	DLBcnCount = 0;
	poll = 0;
	do {
		SetFwRsvdPagePkt_BTCoex(padapter);
		DLBcnCount++;
		do {
			rtw_yield_os();
			/* rtw_mdelay_os(10); */
			/* check rsvd page download OK. */
			rtw_hal_get_hwreg(padapter, HW_VAR_BCN_VALID, (u8 *)(&bcn_valid));
			poll++;
		} while (!bcn_valid && (poll % 10) != 0 && !RTW_CANNOT_RUN(padapter));

	} while (!bcn_valid && DLBcnCount <= 100 && !RTW_CANNOT_RUN(padapter));

	if (RTW_CANNOT_RUN(padapter))
		;
	else if (!bcn_valid)
		RTW_INFO(ADPT_FMT": 1 DL RSVD page failed! DLBcnCount:%u, poll:%u\n",
			 ADPT_ARG(padapter) , DLBcnCount, poll);
	else {
		struct pwrctrl_priv *pwrctl = adapter_to_pwrctl(padapter);
		pwrctl->fw_psmode_iface_id = padapter->iface_id;
		RTW_INFO(ADPT_FMT": 1 DL RSVD page success! DLBcnCount:%u, poll:%u\n",
			 ADPT_ARG(padapter), DLBcnCount, poll);
	}

	/* 2010.05.11. Added by tynli. */
	val8 = rtw_read8(padapter, REG_BCN_CTRL);
	val8 |= BIT(3);
	val8 &= ~BIT(4);
	rtw_write8(padapter, REG_BCN_CTRL, val8);

	/* To make sure that if there exists an adapter which would like to send beacon. */
	/* If exists, the origianl value of 0x422[6] will be 1, we should check this to */
	/* prevent from setting 0x422[6] to 0 after download reserved page, or it will cause */
	/* the beacon cannot be sent by HW. */
	/* 2010.06.23. Added by tynli. */
	if (bRecover) {
		rtw_write8(padapter, REG_FWHW_TXQ_CTRL + 2, pHalData->RegFwHwTxQCtrl | BIT(6));
		pHalData->RegFwHwTxQCtrl |= BIT(6);
	}

#ifndef CONFIG_PCI_HCI
	/* Clear CR[8] or beacon packet will not be send to TxBuf anymore. */
	v8 = rtw_read8(padapter, REG_CR + 1);
	v8 &= ~BIT(0); /* ~ENSWBCN */
	rtw_write8(padapter, REG_CR + 1, v8);
#endif

#endif
}


#ifdef ENABLE_BT_COEXIST


void btinfo_evt_dump(void *sel, void *buf)
{
 struct btinfo *info = (struct btinfo *)buf;

 RTW_PRINT_SEL(sel, "cid:0x%02x, len:%u\n", info->cid, info->len);

 if (info->len > 2)
  RTW_PRINT_SEL(sel, "byte2:%s%s%s%s%s%s%s%s\n"
         , info->bConnection ? "bConnection " : ""
         , info->bSCOeSCO ? "bSCOeSCO " : ""
         , info->bInQPage ? "bInQPage " : ""
         , info->bACLBusy ? "bACLBusy " : ""
         , info->bSCOBusy ? "bSCOBusy " : ""
         , info->bHID ? "bHID " : ""
         , info->bA2DP ? "bA2DP " : ""
         , info->bFTP ? "bFTP" : ""
        );

 if (info->len > 3)
  	RTW_PRINT_SEL(sel, "retry_cnt:%u\n", info->retry_cnt);

 if (info->len > 4)
  	RTW_PRINT_SEL(sel, "rssi:%u\n", info->rssi);

 if (info->len > 5)
 	 RTW_PRINT_SEL(sel, "byte5:%s%s\n"
         , info->eSCO_SCO ? "eSCO_SCO " : ""
         , info->Master_Slave ? "Master_Slave " : ""
        );
}


#endif /* CONFIG_BT_COEXIST */


#ifdef CONFIG_BT_COEXIST
int proc_get_btcoex_dbg(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	PADAPTER padapter;
	char buf[512] = {0};
	padapter = (PADAPTER)rtw_netdev_priv(dev);

	rtw_btcoex_GetDBG(padapter, buf, 512);

	_RTW_PRINT_SEL(m, "%s", buf);

	return 0;
}

ssize_t proc_set_btcoex_dbg(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data)
{
	struct net_device *dev = data;
	PADAPTER padapter;
	u8 tmp[80] = {0};
	u32 module[2] = {0};
	u32 num;

	padapter = (PADAPTER)rtw_netdev_priv(dev);

	/* RTW_INFO("+" FUNC_ADPT_FMT "\n", FUNC_ADPT_ARG(padapter)); */

	if (NULL == buffer) {
		RTW_INFO(FUNC_ADPT_FMT ": input buffer is NULL!\n",
		FUNC_ADPT_ARG(padapter));

		return -EFAULT;
	}

	if (count < 1) {
		RTW_INFO(FUNC_ADPT_FMT ": input length is 0!\n",
		FUNC_ADPT_ARG(padapter));

		return -EFAULT;
	}

	num = count;
	if (num > (sizeof(tmp) - 1))
		num = (sizeof(tmp) - 1);

	if (copy_from_user(tmp, buffer, num)) {
		RTW_INFO(FUNC_ADPT_FMT ": copy buffer from user space FAIL!\n",
		FUNC_ADPT_ARG(padapter));

		return -EFAULT;
	}

	num = sscanf(tmp, "%x %x", module, module + 1);
	if (1 == num) {
		if (0 == module[0])
			memset(module, 0, sizeof(module));
		else
			memset(module, 0xFF, sizeof(module));
	} else if (2 != num) {
		RTW_INFO(FUNC_ADPT_FMT ": input(\"%s\") format incorrect!\n",
		FUNC_ADPT_ARG(padapter), tmp);

		if (0 == num)
			return -EFAULT;
	}

	RTW_INFO(FUNC_ADPT_FMT ": input 0x%08X 0x%08X\n",
	FUNC_ADPT_ARG(padapter), module[0], module[1]);
	rtw_btcoex_SetDBG(padapter, module);

	return count;
}

int proc_get_btcoex_info(struct seq_file *m, void *v)
{
	struct net_device *dev = m->private;
	PADAPTER padapter;
	const u32 bufsize = 30 * 100;
	u8 *pbuf = NULL;

	padapter = (PADAPTER)rtw_netdev_priv(dev);

	pbuf = rtw_zmalloc(bufsize);
	if (NULL == pbuf)
		return -ENOMEM;

	rtw_btcoex_DisplayBtCoexInfo(padapter, pbuf, bufsize);

	_RTW_PRINT_SEL(m, "%s\n", pbuf);

	rtw_mfree(pbuf, bufsize);

	return 0;
}
#endif /* CONFIG_BT_COEXIST */



#endif	


