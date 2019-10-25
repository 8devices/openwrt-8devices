#ifndef _MESH_11KV_C_
#define _MESH_11KV_C_

#ifdef CONFIG_RTL8192CD
#include "../8192cd.h"
#include "../8192cd_headers.h"
#include "../8192cd_util.h"
#else
#include "../rtl8190/8190n.h"
#include "../rtl8190/8190n_headers.h"
#include "../rtl8190/8190n_util.h"
#endif
#include "mesh.h"
#include "mesh_route.h"
#include "mesh_11kv.h"

#ifdef MESH_USE_METRICOP

#define FLAG_METRIC_TX1 0x01
#define FLAG_METRIC_TX2 0x02
#define FLAG_METRIC_TX (FLAG_METRIC_TX1 | FLAG_METRIC_TX2)
#define FLAG_METRIC_RX 0x04

//Tony
#define SUBID_LINK_TEST_REQ 1
#define SUBID_LINK_TEST_ACK 1
#define SUBID_LINK_TEST_REP 2
#define SUBID_LINK_TEST_TRAFFIC 3 // In original 11v proposal (IEEE 802.11-08/0852r0), 
                                  // it is  a QoSNull data frame. However, it is easier 
                                  // for us to handle an action frame, so is it an action frame.
                                  // Format: [11sHDR] [LinkMeasureReportMain] [SubElement1]*k
                                  //         Total length of above frame is 'lenTx0' or 'lenTx0+1'
                                  //         where the format of SubElment is: 
                                  //              [ID: SUBID_LINK_TEST_TRAFFIC, 1 byte] [len: v1, 1 byte] [content: v1 bytes]


#define DIRECTION_A_TO_B 1

__inline__ UINT16 getDataRate2(struct stat_info *pstat, UINT8 rate)
{
	if (rate & 0x80)
		// N mode
		return((MCS_DATA_RATE[(pstat->ht_current_tx_info&BIT(0))?1:0][(pstat->ht_current_tx_info&BIT(1))?1:0][rate&0x7f]) / 2);
        else
		// BG mode
		return(rate/2);       // div 2, because TX rate unit:500Kb/s in BG mode

}

// fill: DialogToken[1] TPC Report element[4] RX Attena ID[1] TX Attena ID[1] RCPI[1] RSNI[1]
unsigned char *fillLinkMeasureReportMain(DRV_PRIV *priv, struct stat_info *pstat, unsigned char *pbuf, int *retlen)
{
	*pbuf = 1; // dialog (1)
	pbuf++;
	(*retlen)++;
	
	// TPC report element (4)
	pbuf+=4;
	*(retlen)+=4;

	// RX Attena, TX Attena (2)
	pbuf+=2;
	*(retlen)+=2;
	
	// RCPI[1], RSNI[1]
	pbuf+=2;
	*(retlen)+=2;
	
	return pbuf;
}

unsigned char *fillActionHeader(DRV_PRIV * priv, struct stat_info * pstat, UINT8 uiCategory, UINT8 uiField, struct tx_insn *txinsn)
{
	unsigned short  seq;
	UINT8                   tmp;
	unsigned char *pbuf = NULL;

	memset((void *)txinsn->phdr, 0, sizeof(struct  wlan_hdr));

	// Fill header
	SetFrDs(txinsn->phdr);
	SetToDs(txinsn->phdr);
	SetFrameSubType((txinsn->phdr), WIFI_11S_MESH_ACTION);
	memcpy((void *)GetAddr1Ptr((txinsn->phdr)), pstat->hwaddr, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn->phdr)), GET_MY_HWADDR, MACADDRLEN); 
	memcpy((void *)GetAddr3Ptr((txinsn->phdr)), pstat->hwaddr, MACADDRLEN);

	// Fill Address 4 (SA) in the BODY
	pbuf = txinsn->pframe;
	memcpy(pbuf, GET_MY_HWADDR, MACADDRLEN);
	pbuf += MACADDRLEN;
	txinsn->fr_len += MACADDRLEN;

	// Set Mesh Header
	pbuf = set_fixed_ie(pbuf, 1 , (unsigned char *)&(priv->mesh_HeaderFlags), &(txinsn->fr_len));
	tmp = 1;
	pbuf = set_fixed_ie(pbuf, 1 , (unsigned char *)&tmp, &(txinsn->fr_len));
	seq = getMeshSeq(priv);
	seq = cpu_to_le16(seq);
	pbuf = set_fixed_ie(pbuf , 2 , (unsigned char *)&seq, &(txinsn->fr_len));

	// Set category, action
	*pbuf = uiCategory;
	pbuf += 1;
	*pbuf = uiField;
	pbuf += 1;
        txinsn->fr_len += _MESH_ACTIVE_FIELD_OFFSET_;

	return pbuf;

/*
err_fill_hdr:
	if (txinsn->phdr)
		release_wlanhdr_to_poll(priv, txinsn->phdr);
	if (txinsn->pframe)
		release_mgtbuf_to_poll(priv, txinsn->pframe);
	txinsn->phdr = NULL;
	txinsn->pframe = NULL;

	return NULL;
*/
}

void issue_11kv_LinkMeasureReq(DRV_PRIV * priv, struct stat_info * pstat, UINT16 lenTst, UINT16 cntTst, UINT8 prioTst)
{
	UINT8	tmp;
	UINT16          val;
	unsigned char	*pbuf;
	UINT32 r;
	
	get_random_bytes((void *)&r, sizeof(UINT32));
	r%=HZ; // max diff is 1 sec
	pstat->mesh_neighbor_TBL.timeMetricUpdate = jiffies + MESH_METRIC_PERIOD_UPDATE + r;

	atomic_set(&pstat->mesh_neighbor_TBL.isMetricTesting, atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)|FLAG_METRIC_TX1);

	DECLARE_TXINSN(txinsn);

	txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;
	txinsn.is_11s = GET_MIB(priv)->dot1180211sInfo.mesh_enable;

	//Set Current TX Rate
	pstat->mesh_neighbor_TBL.spec11kv.rateTx = pstat->current_tx_rate;

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = pstat->current_tx_rate; // find_rate(priv, NULL, 0, 1);
	txinsn.lowest_tx_rate = txinsn.tx_rate;
	txinsn.fixed_rate = 1;
	txinsn.pframe  = get_mgtbuf_from_poll(priv);

	if (txinsn.pframe == NULL)
		goto issue_11kv_LinkMeasureReq_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);

	if (txinsn.phdr == NULL)
		goto issue_11kv_LinkMeasureReq_fail;
	
	memset((void *)txinsn.phdr, 0, sizeof(struct  wlan_hdr));

	// Category | Action | Dialog Token | Transimit Power Used | Max Transmit Power | Opeion sub-elements |
	//     1         1           1                   1                    1                    ?
    
        pbuf = fillActionHeader(priv, pstat, _CATEGORY_11K_ACTION_, ACTION_FILED_11K_LINKME_REQ, &txinsn);
        if (pbuf == NULL)
                goto issue_11kv_LinkMeasureReq_fail;;
	
	// Set Dialog Token
	get_random_bytes((void *)&tmp, sizeof(UINT8));;
	*pbuf = tmp; 
	pbuf += 1;
	
	//Set Transimit Power Used
	*pbuf = pstat->rssi;
	pbuf += 1;
	
	//Set Max Transmit Power
	*pbuf = 100;
	pbuf += 1;
	txinsn.fr_len += 3;
	
	//Set Opeion sub-elements
	//Sub-element ID | Length
	//     1              1	
	//Set Sub-element ID
	*pbuf = SUBID_LINK_TEST_REQ;
	pbuf += 1;
	
	//Set Sub-element Length
	*pbuf = 8;
	pbuf += 1;
	txinsn.fr_len += 2;
    
	//Packet Length | Packet Count | Packet Priority | Test Timeout | Test Direction|
	//     2               2                1              2               1
	//Set Packet Length
	if(lenTst != 0)
	{
		val = cpu_to_le16(lenTst);
		memcpy(pbuf, (void *)&val, sizeof(val));
		pstat->mesh_neighbor_TBL.spec11kv.lenTx0 = lenTst;
	}
	else
	{
		val = cpu_to_le16(priv->mesh_fake_mib.spec11kv.defPktLen);
		memcpy(pbuf, (void *)&val, sizeof(val));
		pstat->mesh_neighbor_TBL.spec11kv.lenTx0 = priv->mesh_fake_mib.spec11kv.defPktLen; 
   	} 

	pbuf += 2;
    
	//Set Packet Count
	if(cntTst != 0)
	{
		val = cpu_to_le16(cntTst);
		memcpy(pbuf, (void *)&val, sizeof(val));
		pstat->mesh_neighbor_TBL.spec11kv.cntTx0 = cntTst;
	}
	else
	{
		val = cpu_to_le16(priv->mesh_fake_mib.spec11kv.defPktCnt); 
		memcpy(pbuf, (void *)&val, sizeof(val));
		pstat->mesh_neighbor_TBL.spec11kv.cntTx0 = priv->mesh_fake_mib.spec11kv.defPktCnt;
	}

	pbuf += 2;
    
	//Set Packet Priority
	if(prioTst != 0)
		*pbuf = prioTst;
	else
		*pbuf = priv->mesh_fake_mib.spec11kv.defPktPri;
 
	pbuf += 1;

    
	//Set Test Timeout
	pstat->mesh_neighbor_TBL.spec11kv.toTx = jiffies +  (priv->mesh_fake_mib.spec11kv.defPktTO*100/1000)*HZ + MESH_METRIC_TIMEOUT_FRAME;
	val = cpu_to_le16(priv->mesh_fake_mib.spec11kv.defPktTO);
	memcpy(pbuf, (void *)&val, sizeof(val));
	pbuf += 2;
    
	//Set Test Direction
	*pbuf = 1;
	pbuf += 1;
    
	//Add totle length of sub-element
	txinsn.fr_len += 8;

	if ((DRV_FIRETX(priv, &txinsn)) == SUCCESS){
	//printk("LinkMeasurement Request was sent successsfully!! \n");
		return;
	}

issue_11kv_LinkMeasureReq_fail:
	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
}



void issue_11kv_LinkMeasureRepAck(DRV_PRIV * priv, struct stat_info * pstat)
{
	unsigned char           *pbuf;

	DECLARE_TXINSN(txinsn);

	txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;
	txinsn.is_11s = GET_MIB(priv)->dot1180211sInfo.mesh_enable;
	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate =  pstat->current_tx_rate; // find_rate(priv, NULL, 0, 1);
	txinsn.lowest_tx_rate = txinsn.tx_rate;
	txinsn.fixed_rate = 1;

	txinsn.pframe  = get_mgtbuf_from_poll(priv);
	txinsn.phdr = get_wlanhdr_from_poll(priv); 

	if ((txinsn.pframe == NULL) || (txinsn.phdr == NULL))
		goto err;

	pbuf = fillActionHeader(priv, pstat, _CATEGORY_11K_ACTION_, ACTION_FILED_11K_LINKME_REP, &txinsn);
	if (pbuf == NULL)
		goto err;

	// fill link measurement report
	pbuf = fillLinkMeasureReportMain(priv, pstat, pbuf, &txinsn.fr_len);
	if(pbuf == NULL)
		goto err;

	// name:      SubElementID    | Length | Response
	// len:           1           |  1     |    1
	// value: SUBID_LINK_TEST_ACK |  1     |  

	// fill ack
	*pbuf = SUBID_LINK_TEST_ACK;
	pbuf++;
	*pbuf = 1; // len=1
	pbuf++;
	*pbuf = 0; // accept
	txinsn.fr_len+=3;

        if ((DRV_FIRETX(priv, &txinsn)) == SUCCESS){
                return;
        }

err:
	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
}

void issue_11kv_LinkMeasureRepRep(DRV_PRIV * priv, struct stat_info * pstat)
{
	unsigned char	*pbuf;
	UINT16		val;

	DECLARE_TXINSN(txinsn);

	atomic_set(&pstat->mesh_neighbor_TBL.isMetricTesting, (atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)&~FLAG_METRIC_RX));

	txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;
	txinsn.is_11s = GET_MIB(priv)->dot1180211sInfo.mesh_enable;
	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = pstat->current_tx_rate; // pstat->mesh_neighbor_TBL.spec11kv.rateRx;
	txinsn.lowest_tx_rate = txinsn.tx_rate;
	txinsn.fixed_rate = 1;

	txinsn.pframe  = get_mgtbuf_from_poll(priv);
	txinsn.phdr = get_wlanhdr_from_poll(priv); 

	if ((txinsn.pframe == NULL) || (txinsn.phdr == NULL))
		goto err;

	pbuf = fillActionHeader(priv, pstat, _CATEGORY_11K_ACTION_, ACTION_FILED_11K_LINKME_REP, &txinsn);
	if(pbuf == NULL)
		goto err;

	// fill link measurement report
	pbuf = fillLinkMeasureReportMain(priv, pstat, pbuf, &txinsn.fr_len);
	if(pbuf == NULL)
		goto err;

	// name:      SubElementID    | Length | Pkt Length | Pkt Count | Priority
	// len:           1           |  1     |     2           2           1
	// value: SUBID_LINK_TEST_REP |  5     |  

	*pbuf = SUBID_LINK_TEST_REP;
	pbuf++;

	*pbuf = 5;
	pbuf++;

	val = cpu_to_le16(pstat->mesh_neighbor_TBL.spec11kv.lenRx1);
	memcpy(pbuf, (void *)&val, sizeof(val));
	pbuf+=2;

	val = cpu_to_le16(pstat->mesh_neighbor_TBL.spec11kv.cntRx1);
	memcpy(pbuf, (void *)&val, sizeof(val));
	pbuf+=2;

	*(pbuf) = pstat->mesh_neighbor_TBL.spec11kv.prioRx;

	txinsn.fr_len+=7;

	atomic_set(&pstat->mesh_neighbor_TBL.isMetricTesting,
		(atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)&~FLAG_METRIC_RX));
	pstat->mesh_neighbor_TBL.spec11kv.lenRx1 = 0;
	pstat->mesh_neighbor_TBL.spec11kv.cntRx1 = 0;

        if ((DRV_FIRETX(priv, &txinsn)) == SUCCESS){
                return;
        }

err:
	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
}



// format: ID[1] LEN[1] PktLen[2] PtkCnt[2] PktPrio[1] TestTimeout[2] TestDirection[1]
unsigned int On11kvLinkMeasureReq(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
	unsigned char  *pframe, *pFrameBody;
	unsigned char *ptr;
	struct stat_info *pstat;

	pframe = get_pframe(pfrinfo);  // get frame data

	if (pframe==0)
		return FAIL;

	if(is_mesh_6addr_format_without_qos(pframe)) 
		pFrameBody = pframe + WLAN_HDR_A6_MESH_DATA_LEN;
	else
		pFrameBody = pframe + WLAN_HDR_A4_MESH_DATA_LEN;

	pstat = get_stainfo(priv, GetAddr2Ptr(pframe));
	if(pstat == NULL)
		return FAIL;

	if(atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)&FLAG_METRIC_RX) // already testing
		return FAIL;

	pstat->mesh_neighbor_TBL.spec11kv.lenRx0 = 0;
	pstat->mesh_neighbor_TBL.spec11kv.cntRx0 = 0;
	pstat->mesh_neighbor_TBL.spec11kv.rateRx = 0;
	pstat->mesh_neighbor_TBL.spec11kv.prioRx = 0;

	ptr = pFrameBody+5; // id
	if(*ptr != SUBID_LINK_TEST_REQ)
		return FAIL;

	if(*(ptr+9)!= DIRECTION_A_TO_B)
		return FAIL;

	ptr++; // len
	if(*ptr<8)
		return FAIL;

	ptr++; // PktLen
	pstat->mesh_neighbor_TBL.spec11kv.lenRx0 = le16_to_cpu( *(UINT16 *)ptr);

	ptr+=2; // PktCnt
	pstat->mesh_neighbor_TBL.spec11kv.cntRx0 = le16_to_cpu( *(UINT16 *)ptr);

	ptr+=2; // PktPrio
	pstat->mesh_neighbor_TBL.spec11kv.prioRx = *ptr;

	ptr++; // TestTimeout
	pstat->mesh_neighbor_TBL.spec11kv.toRx = jiffies + (le16_to_cpu( *(UINT16 *)ptr)*100)/1000*HZ + MESH_METRIC_TIMEOUT_FRAME;

	atomic_set(&pstat->mesh_neighbor_TBL.isMetricTesting, atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)|FLAG_METRIC_RX);
	
	issue_11kv_LinkMeasureRepAck(priv, pstat);

	return SUCCESS;
}

static unsigned char bufTraffic[1024];
void issue_11kv_TestTraffic(DRV_PRIV *priv, struct stat_info *pstat)
{
	UINT8 i;
	UINT8 t;
	UINT16 len_t;
	unsigned char *pbuf; 

	DECLARE_TXINSN(txinsn);

        txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;
        txinsn.is_11s = 128;

        txinsn.q_num = 0;
        txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = pstat->mesh_neighbor_TBL.spec11kv.rateTx;
        txinsn.lowest_tx_rate = txinsn.tx_rate;
	txinsn.fixed_rate = 1;
	txinsn.pframe = bufTraffic;

        // memset(bufTraffic, 'Z', sizeof(bufTraffic)); // @@@ just for test

	if (txinsn.pframe == NULL)
		goto err;

        txinsn.phdr = (UINT8 *)get_wlanhdr_from_poll(priv);

        if (txinsn.phdr == NULL)
                goto err;

	memset((void *)txinsn.phdr, 0, sizeof(struct wlan_hdr));

        pbuf = fillActionHeader(priv, pstat, _CATEGORY_11K_ACTION_, ACTION_FILED_11K_LINKME_REP, &txinsn);
        if (pbuf == NULL)
                goto err;

        // fill link measurement report
        pbuf = fillLinkMeasureReportMain(priv, pstat, pbuf, &txinsn.fr_len);
        if(pbuf == NULL)
                goto err;

	// name:      (SubElementID       | Length | Content   )
	// len:       (    1              |  1     | max=length ) x n --> total length = lenTx0 - header
	// value: SUBID_LINK_TEST_TRAFFIC |    

	len_t = pstat->mesh_neighbor_TBL.spec11kv.lenTx0 - txinsn.fr_len - 24; // 24 is the length in phdr (A1~A3)

	t = len_t/257;

	txinsn.fr_len += len_t;

	for(i = 0;i < t;i++)
	{
		*pbuf = SUBID_LINK_TEST_TRAFFIC;
		*(pbuf+1) = 255;

		pbuf += 257;
		len_t -= 257;
	}

	switch(len_t)
	{
		case 0:
			break;
		case 1:
			txinsn.fr_len += 1;
		case 2:
			*pbuf = SUBID_LINK_TEST_TRAFFIC;
			*(pbuf+1) = 0;
			break;
		default:
			*pbuf = SUBID_LINK_TEST_TRAFFIC;
			*(pbuf+1) = len_t - 2;
	}

        if ((DRV_FIRETX(priv, &txinsn)) == SUCCESS){
                return;
        }
err:
        if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
}

// format: ID[1] LEN[1] Resp[1]
unsigned int On11kvLinkMeasureRepAck(DRV_PRIV *priv, struct rx_frinfo *pfrinfo, unsigned char *pFrameBody, unsigned char *pSubElement)
{
	unsigned char *pframe = get_pframe(pfrinfo);
	struct stat_info *pstat = get_stainfo(priv, GetAddr2Ptr(pframe));
	int i = 0;

	if((pstat == NULL) || (atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)&FLAG_METRIC_TX1)==0)
	{
		return FAIL;
	}

	atomic_set(&pstat->mesh_neighbor_TBL.isMetricTesting, (atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)&~FLAG_METRIC_TX1)|FLAG_METRIC_TX2);
	
	// Sub-element ID | Length | Response (0 / 1) |
	//        1           1           1 

        if(*(pSubElement + 1) < 1)
        {
                return FAIL;
        }

	switch(*(pSubElement + 2))
	{
		case 0: 
			break;
		case 1:
			return FAIL;
		default:
			return FAIL;
	}

	for(i = 0;i < pstat->mesh_neighbor_TBL.spec11kv.cntTx0;i++)
	{
		issue_11kv_TestTraffic(priv, pstat);
	}

	return SUCCESS;
}

// format: ID[1] LEN[1] PktLen[2] PktCnt[2] PktPrio[1]
unsigned int On11kvLinkMeasureRepRep(DRV_PRIV *priv, struct rx_frinfo *pfrinfo, unsigned char *pFrameBody, unsigned char *pSubElement)
{
	UINT16 len, cnt;

	unsigned char *pframe = get_pframe(pfrinfo);
	struct stat_info *pstat = get_stainfo(priv, GetAddr2Ptr(pframe));

	if((pstat == NULL) || (atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)&FLAG_METRIC_TX2)==0)
	{
		return FAIL;
	}
	if(*(pSubElement+1) != 5)
	{
		return FAIL; // length is wrong
	}
	pSubElement+=2;
	len = le16_to_cpu(* (UINT16 *)pSubElement);

	pSubElement+=2;
	cnt = le16_to_cpu(* (UINT16 *)pSubElement);

	pSubElement+=2; // pointer to prio

	pstat->mesh_neighbor_TBL.retryMetric = 0;
	computeMetric(priv, pstat, pstat->mesh_neighbor_TBL.spec11kv.rateTx, *pSubElement,
			pstat->mesh_neighbor_TBL.spec11kv.lenTx0*pstat->mesh_neighbor_TBL.spec11kv.cntTx0,
			pstat->mesh_neighbor_TBL.spec11kv.cntTx0,
			len, cnt);

	atomic_set(&pstat->mesh_neighbor_TBL.isMetricTesting,
		(atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)&~FLAG_METRIC_TX2));

	return SUCCESS;
}

unsigned int On11kvTestTraffic(DRV_PRIV * priv, struct rx_frinfo *pfrinfo, unsigned char *pFrameBody, unsigned char *pSubElement)
{
	unsigned char *pframe;
	struct stat_info *pstat;

	pframe = get_pframe(pfrinfo); 
	pstat = get_stainfo(priv, GetAddr2Ptr(pframe));
	// pskb = get_pskb(pfrinfo);

	if((pstat == NULL) || ((atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)&FLAG_METRIC_RX)==0))
		return FAIL;


	pstat->mesh_neighbor_TBL.spec11kv.cntRx1 ++;
	pstat->mesh_neighbor_TBL.spec11kv.lenRx1 += pfrinfo->pktlen;
	pstat->mesh_neighbor_TBL.spec11kv.rateRx = pfrinfo->rx_rate;

#if 0 
	printk("@@@ receive test traffic: cnt=%d length=%d prio=%d\n", 
			pstat->mesh_neighbor_TBL.spec11kv.cntRx1, pstat->mesh_neighbor_TBL.spec11kv.lenRx1, pstat->mesh_neighbor_TBL.spec11kv.prioRx);
#endif
	if(pstat->mesh_neighbor_TBL.spec11kv.cntRx0 == pstat->mesh_neighbor_TBL.spec11kv.cntRx1)
	{
		pstat->mesh_neighbor_TBL.retryMetric = 0;
		computeMetric(priv, pstat, pstat->mesh_neighbor_TBL.spec11kv.rateRx,
					pstat->mesh_neighbor_TBL.spec11kv.prioRx,
					pstat->mesh_neighbor_TBL.spec11kv.lenRx0,
					pstat->mesh_neighbor_TBL.spec11kv.cntRx0,
					pstat->mesh_neighbor_TBL.spec11kv.lenRx1,
					pstat->mesh_neighbor_TBL.spec11kv.cntRx1);
		issue_11kv_LinkMeasureRepRep(priv, pstat);
	}
		

	return SUCCESS;
}

unsigned int On11kvLinkMeasureRep(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
	unsigned char  *pframe, *pFrameBody;
	unsigned char *pSubElement;
	UINT8 sid;

	pframe = get_pframe(pfrinfo);  // get frame data

	if (pframe==0)
		return FAIL;

	if(is_mesh_6addr_format_without_qos(pframe)) 
		pFrameBody = pframe + WLAN_HDR_A6_MESH_DATA_LEN;
	else
		pFrameBody = pframe + WLAN_HDR_A4_MESH_DATA_LEN;

	pSubElement = pFrameBody+11; // point to "sub element id"

	sid = *pSubElement;

	switch(sid)
	{
		case SUBID_LINK_TEST_ACK:
			return On11kvLinkMeasureRepAck(priv, pfrinfo, pFrameBody, pSubElement);
		case SUBID_LINK_TEST_REP:
			return On11kvLinkMeasureRepRep(priv, pfrinfo, pFrameBody, pSubElement);
		case SUBID_LINK_TEST_TRAFFIC:
			return On11kvTestTraffic(priv, pfrinfo, pFrameBody, pSubElement);
	}

	return FAIL;
}


UINT32 computeMetric(DRV_PRIV * priv, struct stat_info * pstat, UINT8 rate, UINT8 priority,
			UINT16 lenTotalSnd, UINT16 cntSnd, UINT16 lenTotalRcv, UINT16 cntRcv)
{
	UINT32	metric_current;
	// UINT16	bt;
 	UINT16 rateMbps;

	if(pstat->mesh_neighbor_TBL.retryMetric > MESH_METRIC_MAX_RETRY)
	{
		pstat->mesh_neighbor_TBL.metric = (pstat->mesh_neighbor_TBL.metric + MESH_METRIC_UNSPECIFIED)/2;

#if 0
		printk("@@@ retry Compute Matic %lu\n", pstat->mesh_neighbor_TBL.metric);
#endif

		return pstat->mesh_neighbor_TBL.metric;
	}
 
	if(cntRcv == 0)
	{
#if 0
		printk("@@@ cntRcv ==0 Compute Matic %lu\n",pstat->mesh_neighbor_TBL.metric);
#endif
		return pstat->mesh_neighbor_TBL.metric;
	}
	rateMbps = getDataRate2(pstat, rate);

	// metric = (O + bt/rate) * 1/(1-ef) = (O + bt/rate) * cntSnd/cntRcv;
	// O is vender define here is 0
	// 1/(1-ef) = 1/(cntSnd/cntSnd - (cntSnd-cntRcv)/cntSnd) = cntSnd/cntRcv
	//bt = lenTotalRcv * 8 / cntRcv;	//receive frame size
	// metric_current = (bt * cntSnd) / (rateMbps * cntRcv); 
	metric_current = (lenTotalRcv * 8 * cntSnd) / (rateMbps * cntRcv * cntRcv);

	pstat->mesh_neighbor_TBL.metric = (pstat->mesh_neighbor_TBL.metric + metric_current)/2;

#if 0
	printk("@@@ Compute Matic %lu  rate %d  lentotalSnd %d  cntSnd %d  lenRcv %d  cntRcv %d\n",pstat->mesh_neighbor_TBL.metric,rateMbps,lenTotalSnd,cntSnd,lenTotalRcv,cntRcv);
#endif

	return pstat->mesh_neighbor_TBL.metric;
}

UINT32 getMetric(DRV_PRIV * priv, struct stat_info * pstat)
{
	return (pstat->mesh_neighbor_TBL.metric == 0)? MESH_METRIC_UNSPECIFIED : pstat->mesh_neighbor_TBL.metric;
}


void metric_update(DRV_PRIV *priv)
{
	struct stat_info        *pstat;
	struct list_head        *phead, *plist, *pprevlist;

	phead= &priv->mesh_mp_hdr;
	plist = phead->next;
	pprevlist = phead;
	while(plist != phead)
	{
		pstat = list_entry(plist, struct stat_info, mesh_mp_ptr);
		plist = plist->next;
		if(time_after(jiffies, pstat->mesh_neighbor_TBL.timeMetricUpdate))
		{
			
			if( ((pstat->mesh_neighbor_TBL.isAsym==0) && (atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)==0)) ||
					((pstat->mesh_neighbor_TBL.isAsym==1) && ((atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)&FLAG_METRIC_TX)==0))
				)
			{
				issue_11kv_LinkMeasureReq(priv, pstat,0,0,0);
			}
		} // time_after


		if((atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)&FLAG_METRIC_RX)!=0)
		{
			if(time_after(jiffies, pstat->mesh_neighbor_TBL.spec11kv.toRx))
			{
#if 0
				printk("@@@@ wait for test traffic expire (req cnt: %d/ cur cnt: %d), send rep rep\n", 
					pstat->mesh_neighbor_TBL.spec11kv.cntRx0, pstat->mesh_neighbor_TBL.spec11kv.cntRx1);
#endif

				pstat->mesh_neighbor_TBL.retryMetric++;
				computeMetric(priv, pstat, pstat->mesh_neighbor_TBL.spec11kv.rateRx, 
							pstat->mesh_neighbor_TBL.spec11kv.prioRx,
							pstat->mesh_neighbor_TBL.spec11kv.lenRx0,
							pstat->mesh_neighbor_TBL.spec11kv.cntRx0,
							pstat->mesh_neighbor_TBL.spec11kv.lenRx1,
							pstat->mesh_neighbor_TBL.spec11kv.cntRx1);
				issue_11kv_LinkMeasureRepRep(priv, pstat);
			}
		}
		if((atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)&FLAG_METRIC_TX)!=0)
		{
			if(time_after(jiffies, pstat->mesh_neighbor_TBL.spec11kv.toTx))
			{

				pstat->mesh_neighbor_TBL.retryMetric++;
				computeMetric(priv, pstat, 0, 0, 0, 0, 0, 0);
				if((atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)&FLAG_METRIC_TX1)!=0)
					atomic_set(&pstat->mesh_neighbor_TBL.isMetricTesting, 
						(atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)&~FLAG_METRIC_TX1));
				if((atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)&FLAG_METRIC_TX2)!=0)
					atomic_set(&pstat->mesh_neighbor_TBL.isMetricTesting, 
						(atomic_read(&pstat->mesh_neighbor_TBL.isMetricTesting)&~FLAG_METRIC_TX2));
			}
		}
	} // while plist <> phead
}

#endif // MESH_USE_METRICOP

#endif

