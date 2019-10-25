/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	RateAdaptive.c
	
Abstract:
	Implement Rate Adaptive functions for common operations.
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------	
	2011-08-12 Page            Create.	

--*/
#ifdef __ECOS
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#endif
#include "8192cd_cfg.h"
#ifdef CONFIG_RTL_88E_SUPPORT


#ifdef TXREPORT
#include "8192cd.h"
#include "8192cd_headers.h"
#include "8192cd_hw.h"
#include "8192cd_util.h"

/*
 *	RateAdaptive.h	-	Start
 */
#define	PERENTRY	23
#define	RETRYSIZE	5
#define	RATESIZE	28
/*
 *	RateAdaptive.h	-	End
 */


static unsigned char RETRY_PENALTY[PERENTRY][RETRYSIZE+1] = {{5,4,3,2,0,3},//92 , idx=0
													{6,5,4,3,0,4},//86 , idx=1
													{6,5,4,2,0,4},//81 , idx=2
													{8,7,6,4,0,6},//75 , idx=3
													{10,9,8,6,0,8},//71	, idx=4
													{10,9,8,4,0,8},//66	, idx=5
													{10,9,8,2,0,8},//62	, idx=6
													{10,9,8,0,0,8},//59	, idx=7
													{18,17,16,8,0,16},//53 , idx=8
													{26,25,24,16,0,24},//50	, idx=9
													{34,33,32,24,0,32},//47	, idx=0x0a
													//{34,33,32,16,0,32},//43	, idx=0x0b
													//{34,33,32,8,0,32},//40 , idx=0x0c
													//{34,33,28,8,0,32},//37 , idx=0x0d
													//{34,33,20,8,0,32},//32 , idx=0x0e
													//{34,32,24,8,0,32},//26 , idx=0x0f
													//{49,48,32,16,0,48},//20	, idx=0x10
													//{49,48,24,0,0,48},//17 , idx=0x11
													//{49,47,16,16,0,48},//15	, idx=0x12
													//{49,44,16,16,0,48},//12	, idx=0x13
													//{49,40,16,0,0,48},//9 , idx=0x14
													{34,31,28,20,0,32},//43	, idx=0x0b
													{34,31,27,18,0,32},//40 , idx=0x0c
													{34,31,26,16,0,32},//37 , idx=0x0d
													{34,30,22,16,0,32},//32 , idx=0x0e
													{34,30,24,16,0,32},//26 , idx=0x0f
													{49,46,40,16,0,48},//20	, idx=0x10
													{49,45,32,0,0,48},//17 , idx=0x11
													{49,45,22,18,0,48},//15	, idx=0x12
													{49,40,28,18,0,48},//12	, idx=0x13
													{49,34,20,16,0,48},//9 , idx=0x14
													{49,22,18,14,0,48},//6 , idx=0x15
													{49,16,16,0,0,48}};//3 //3, idx=0x16


#if 0
static unsigned char	RETRY_PENALTY_UP[RETRYSIZE+1]={49,44,16,16,0,48};  // 12% for rate up

static unsigned char PT_PENALTY[RETRYSIZE+1]={34,31,30,24,0,32};

static u1Byte	RETRY_PENALTY_IDX[2][RATESIZE] = {{4,4,4,5,4,4,5,7,7,7,8,0x0a,	       // SS>TH
													4,4,4,4,6,0x0a,0x0b,0x0d,
													5,5,7,7,8,0x0b,0x0d,0x0f},	 		   // 0329 R01
													{4,4,4,5,7,7,9,9,0x0c,0x0e,0x10,0x12,	   // SS<TH
													4,4,5,5,6,0x0a,0x11,0x13,
													9,9,9,9,0x0c,0x0e,0x11,0x13}};	
#endif
// wilson modify
static unsigned char	RETRY_PENALTY_IDX[2][RATESIZE] = {{4,4,4,5,4,4,5,7,7,7,8,0x0a,	       // SS>TH
													4,4,4,4,6,0x0a,0x0b,0x0d,
													5,5,7,7,8,0x0b,0x0d,0x0f},	 		   // 0329 R01
													{0x0a,0x0a,0x0a,0x0a,0x0c,0x0c,0x0e,0x10,0x11,0x12,0x12,0x13,	   // SS<TH
													0x0e,0x0f,0x10,0x10,0x11,0x14,0x14,0x15,
													9,9,9,9,0x0c,0x0e,0x11,0x13}};	
static unsigned char	RETRY_PENALTY_UP_IDX[RATESIZE] = {0x10,0x10,0x10,0x10,0x11,0x11,0x12,0x12,0x12,0x13,0x13,0x14,	       // SS>TH
													0x13,0x13,0x14,0x14,0x15,0x15,0x15,0x15,
													0x11,0x11,0x12,0x13,0x13,0x13,0x14,0x15};	

/*static unsigned char	RSSI_THRESHOLD[RATESIZE] = {0,0,0,0,
													0,0,0,0,0,0x24,0x26,0x2a,
													0x18,0x1a,0x1d,0x1f,0x21,0x27,0x29,0x2a,
													0,0,0,0x1f,0x23,0x28,0x2a,0x2c};*/
static unsigned char	RSSI_THRESHOLD[RATESIZE] = {0,0,0,0,
													0,0,0,0,0,0x24,0x26,0x2a,
													0x13,0x15,0x17,0x18,0x1a,0x1c,0x1d,0x1f,
													0,0,0,0x1f,0x23,0x28,0x2a,0x2c};
static unsigned short	N_THRESHOLD_HIGH[RATESIZE] = {4,4,8,16,
													24,36,48,72,96,144,192,216,
													60,80,100,160,240,400,560,640,
													300,320,480,720,1000,1200,1600,2000};
static unsigned short 	N_THRESHOLD_LOW[RATESIZE] = {2,2,4,8,
													12,18,24,36,48,72,96,108,
													30,40,50,80,120,200,280,320,
													150,160,240,360,500,600,800,1000};
#if 0
static unsigned char	 TRYING_NECESSARY[RATESIZE] = {2,2,2,2,
													2,2,3,3,4,4,5,7,
													4,4,7,10,10,12,12,18,
													5,7,7,8,11,18,36,60};  // 0329 // 1207
static u1Byte	 POOL_RETRY_TH[RATESIZE] = {30,30,30,30,
													30,30,25,25,20,15,15,10,
													30,25,25,20,15,10,10,10,
													30,25,25,20,15,10,10,10}; 		
#endif

static unsigned char	DROPING_NECESSARY[RATESIZE] = {1,1,1,1,
													1,2,3,4,5,6,7,8,
													1,2,3,4,5,6,7,8,
													5,6,7,8,9,10,11,12};

#if 0
static unsigned int	INIT_RATE_FALLBACK_TABLE[16]={0x0f8ff015,  // 0: 40M BGN mode
											0x0f8ff010,   // 1: 40M GN mode
											0x0f8ff005,   // 2: BN mode/ 40M BGN mode
											0x0f8ff000,   // 3: N mode
											0x00000ff5,   // 4: BG mode
											0x00000ff0,   // 5: G mode
											0x0000000d,   // 6: B mode
											0,			// 7:
											0,			// 8:
											0,			// 9:
											0,			// 10:
											0,			// 11:
											0,			// 12:
											0,			// 13:
											0,			// 14:
											0,			// 15:
											
	};
#endif
static unsigned char PendingForRateUpFail[5]={2,10,24,40,60};

static unsigned short DefaultTxRPTTiming=0x186a; //200ms
unsigned short DynamicTxRPTTiming[6]={0x186a, 0x30d4, 0x493e, 0x61a8, 0x7a12 ,0x927c}; // 200ms-1200ms
#if 0
static unsigned short MinTxRPTTiming=0x186a; //200ms
#endif
unsigned char TxRPTTiming_idx=1;
unsigned char DynamicTxRPTTimingCounter=0;


// End Rate adaptive parameters
int RateAdaptiveInfoInit(PSTATION_RA_INFO  pRaInfo)
{
	if(pRaInfo)
	{
/*		panic_printk("%s %d, aid %d\n", __FUNCTION__, __LINE__, pRaInfo->pstat->aid);*/
		pRaInfo->DecisionRate = 0x13;
		pRaInfo->PreRate = 0x13;
		pRaInfo->HighestRate=0x13;
		pRaInfo->LowestRate=0;
		pRaInfo->RateID=0;
		pRaInfo->RateMask=0xffffffff;
		pRaInfo->RssiStaRA=0;
		pRaInfo->PreRssiStaRA=0;
		pRaInfo->SGIEnable=0;
		pRaInfo->RAUseRate=0xffffffff;
		pRaInfo->NscDown=(N_THRESHOLD_HIGH[0x13]+N_THRESHOLD_LOW[0x13])/2;
		pRaInfo->NscUp=(N_THRESHOLD_HIGH[0x13]+N_THRESHOLD_LOW[0x13])/2;
		pRaInfo->RateSGI=0;
		pRaInfo->Active=1;  //Active is not used at present. by page, 110819
		pRaInfo->RptTime = 0x927c;
		pRaInfo->DROP=0;
		pRaInfo->RTY[0]=0;
		pRaInfo->RTY[1]=0;
		pRaInfo->RTY[2]=0;
		pRaInfo->RTY[3]=0;
		pRaInfo->RTY[4]=0;
		pRaInfo->TOTAL=0;
#if 1
		pRaInfo->RAWaitingCounter=0;
		pRaInfo->RAPendingCounter=0;
#endif
#if 0
		pRaInfo->TryingState=0;
		pRaInfo->RateBeforeTrying=0x13;
#endif
#if 0 //def POWER_TRAINING_ACTIVE
		pRaInfo->PTTryState=0;
		pRaInfo->PTStage=0;
		pRaInfo->PTStopCount=0;
		pRaInfo->PTPreRate=0;
		pRaInfo->PTPreRssi=0;
		pRaInfo->PTModeSS=0;
		pRaInfo->RAstage=0;
#endif
	}
	return 0;
}

int ARFBRefresh(struct rtl8192cd_priv *priv, PSTATION_RA_INFO  pRaInfo)
{  // Wilson 2011/10/26
	unsigned int MaskFromReg;
	int i;

	// Test for Wilson
#if defined(CONFIG_PCI_HCI)
	RTL_W16(REG_88E_TXRPT_TIM, DefaultTxRPTTiming);  //200ms
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	notify_tx_report_interval_change(priv, DefaultTxRPTTiming);
#endif
	RTL_W32(DARFRC, 0x04020100);
	RTL_W32(DARFRC+4, 0x0a080706);

	switch(pRaInfo->RateID) {
	case RATR_INX_WIRELESS_NGB:
		pRaInfo->RAUseRate=(pRaInfo->RateMask)&0x0f8ff015;
		break;
	case RATR_INX_WIRELESS_NG:
		pRaInfo->RAUseRate=(pRaInfo->RateMask)&0x0f8ff010;
		break;
	case RATR_INX_WIRELESS_NB:
		pRaInfo->RAUseRate=(pRaInfo->RateMask)&0x0f8ff005;
		break;
	case RATR_INX_WIRELESS_N:
		pRaInfo->RAUseRate=(pRaInfo->RateMask)&0x0f8ff000;
		break;
	case RATR_INX_WIRELESS_GB:
		pRaInfo->RAUseRate=(pRaInfo->RateMask)&0x00000ff5;
		break;
	case RATR_INX_WIRELESS_G:
		pRaInfo->RAUseRate=(pRaInfo->RateMask)&0x00000ff0;
		break;
	case RATR_INX_WIRELESS_B:
		pRaInfo->RAUseRate=(pRaInfo->RateMask)&0x0000000d;
		break;
	case 12:
		MaskFromReg=RTL_R32(ARFR0);
		pRaInfo->RAUseRate=(pRaInfo->RateMask)&MaskFromReg;
		break;
	case 13:
		MaskFromReg=RTL_R32(ARFR1);
		pRaInfo->RAUseRate=(pRaInfo->RateMask)&MaskFromReg;
		break;
	case 14:
		MaskFromReg=RTL_R32(ARFR2);
		pRaInfo->RAUseRate=(pRaInfo->RateMask)&MaskFromReg;
		break;
	case 15:
		MaskFromReg=RTL_R32(ARFR3);
		pRaInfo->RAUseRate=(pRaInfo->RateMask)&MaskFromReg;
		break;
	default:
		pRaInfo->RAUseRate=(pRaInfo->RateMask);
		break;
	}
	// Highest rate
	if (pRaInfo->RAUseRate) {
		for (i=RATESIZE;i>=0;i--) {
			if((pRaInfo->RAUseRate)&BIT(i)) {
				pRaInfo->HighestRate=i;
				break;
			}
		}
	} else {
		pRaInfo->HighestRate=0;
	}

	// Lowest rate
	if (pRaInfo->RAUseRate) {
		for (i=0;i<RATESIZE;i++) {
			if((pRaInfo->RAUseRate)&BIT(i)) {
				pRaInfo->LowestRate=i;
				break;
			}
		}
	} else {
		pRaInfo->LowestRate=0;
	}
#if 0
	pRaInfo->RAWaitingCounter=0;
	pRaInfo->RAExtendCounter=0;
#endif
	
/*	panic_printk("%s %d: RateID=%d RateMask=%8.8x RAUseRate=%8.8x HighestRate=%d\n", 
				__FUNCTION__, __LINE__, 
				pRaInfo->RateID, pRaInfo->RateMask, pRaInfo->RAUseRate, pRaInfo->HighestRate);*/
	return 0;
}


static int RateDown(struct rtl8192cd_priv *priv, PSTATION_RA_INFO  pRaInfo)
{
	unsigned char RateID, LowestRate, HighestRate;
	unsigned char i;

	if(NULL == pRaInfo) {
		printk("%s %d: pRaInfo is NULL\n", __FUNCTION__, __LINE__);
		return -1;
	}
	RateID = pRaInfo->PreRate;
	LowestRate = pRaInfo->LowestRate;
	HighestRate = pRaInfo->HighestRate;

/*	panic_printk("%s %d: RateID=%d LowestRate=%d HighestRate=%d RateSGI=%d\n", 
				__FUNCTION__, __LINE__, 
				RateID, LowestRate, HighestRate, pRaInfo->RateSGI);*/

	if (RateID > HighestRate) {
		RateID=HighestRate;
	} else if(pRaInfo->RateSGI) {
		pRaInfo->RateSGI=0;
	} else if (RateID > LowestRate) {
		if (RateID > 0) {
			for (i=RateID-1; i>LowestRate;i--) {
				if (pRaInfo->RAUseRate & BIT(i)) {
					RateID=i;
					goto RateDownFinish;					
				}
			}
		}
	} else if (RateID <= LowestRate) {
		RateID = LowestRate;
	}

RateDownFinish:
#if 1
	if (pRaInfo->RAWaitingCounter==1) {
		pRaInfo->RAWaitingCounter+=1;
		pRaInfo->RAPendingCounter+=1;
	}
	else if(pRaInfo->RAWaitingCounter==0){
	} else {
		pRaInfo->RAWaitingCounter=0;
		pRaInfo->RAPendingCounter=0;
	}

	if(pRaInfo->RAPendingCounter>=4)
		pRaInfo->RAPendingCounter=4;
#endif
	
	pRaInfo->DecisionRate=RateID;
	RTL8188E_SetTxReportTimeByRA(priv, 2);

/*	panic_printk("=====> rate down %s %d: Rate down to RateID %d RateSGI %d\n", __FUNCTION__, __LINE__, RateID, pRaInfo->RateSGI);*/
	return 0;
}


static int RateUp(struct rtl8192cd_priv *priv, PSTATION_RA_INFO  pRaInfo)
{
	unsigned char RateID, HighestRate;
	unsigned char i;

	if(NULL == pRaInfo) {
		printk("%s %d: pRaInfo is NULL\n", __FUNCTION__, __LINE__);
		return -1;
	}
	RateID = pRaInfo->PreRate;
#if 0
	pRaInfo->RateBeforeTrying=RateID;
#endif
	HighestRate = pRaInfo->HighestRate;
/*	panic_printk("%s %d: RateID=%d HighestRate=%d\n", __FUNCTION__, __LINE__, RateID, HighestRate);*/
#if 1
	if (pRaInfo->RAWaitingCounter==1) {
		pRaInfo->RAWaitingCounter=0;
		pRaInfo->RAPendingCounter=0;
	} else if (pRaInfo->RAWaitingCounter>1) {
		pRaInfo->PreRssiStaRA=pRaInfo->RssiStaRA;
		goto RateUpfinish;
	}
	RTL8188E_SetTxReportTimeByRA(priv, 0);
/*	panic_printk("%s %d, Decrease RPT Timing\n", __FUNCTION__, __LINE__);*/
#endif
	
	if (RateID < HighestRate) {
		for (i=RateID+1; i<=HighestRate; i++) {
			if (pRaInfo->RAUseRate & BIT(i)) {
				RateID=i;
				goto RateUpfinish;
			}
		}
	} else if(RateID == HighestRate) 	{
		if (pRaInfo->SGIEnable && (pRaInfo->RateSGI != 1))
			pRaInfo->RateSGI = 1;
		else if((pRaInfo->SGIEnable) !=1 )
			pRaInfo->RateSGI = 0;
	} else /* if((sta_info_ra->Decision_rate) > (sta_info_ra->Highest_rate)) */ {
		RateID = HighestRate;
	}

RateUpfinish:
#if 1
	//if(pRaInfo->RAWaitingCounter==10)
	if(pRaInfo->RAWaitingCounter==(4+PendingForRateUpFail[pRaInfo->RAPendingCounter]))
		pRaInfo->RAWaitingCounter=0;
	else
		pRaInfo->RAWaitingCounter++;
#endif
	pRaInfo->DecisionRate=RateID;
#if 0
	pRaInfo->TryingState=1;
	StartRateTrying(Adapter,pRaInfo);
#endif
/*	panic_printk("%s %d: Rate up to RateID %d\n", __FUNCTION__, __LINE__, RateID);*/
	return 0;
}


static void ResetRaCounter(PSTATION_RA_INFO  pRaInfo){
	unsigned char RateID;
	RateID=pRaInfo->DecisionRate;
	pRaInfo->NscUp=(N_THRESHOLD_HIGH[RateID]+N_THRESHOLD_LOW[RateID])>>1;
	pRaInfo->NscDown=(N_THRESHOLD_HIGH[RateID]+N_THRESHOLD_LOW[RateID])>>1;
}

#if 0
void StartRateTrying(IN	PADAPTER	Adapter, IN PSTATION_RA_INFO  pRaInfo)
{

	// Test for Wilson
	PlatformEFIOWrite2Byte(Adapter, REG_TX_RPT_TIME,0x0138);
	RT_TRACE(COMP_RATE_ADAPTIVE, DBG_LOUD, 
					("StartRateTrying(): "));
	// Need to Modify (Try bit?, Try timing)
	

}

u1Byte EndRateTrying(IN	PADAPTER	Adapter, IN PSTATION_RA_INFO  pRaInfo)
{

	u1Byte TryResult=0;
	u4Byte pool_retry;
	pRaInfo->DecisionRate=(pRaInfo->DecisionRate)&0x7f;
	PlatformEFIOWrite2Byte(Adapter, REG_TX_RPT_TIME,0x44A5);
	pool_retry=pRaInfo->RTY[1]+pRaInfo->RTY[2]+pRaInfo->RTY[3]+pRaInfo->RTY[4]+pRaInfo->DROP;
	if (pool_retry>2)
		TryResult=0;
	else
		TryResult=1;

	return TryResult;
	

}
#endif


/*	moved to 8188e_hw.c

void SetTxRPTTiming(IN	PADAPTER	Adapter, int extend)
{
	u2Byte WriteTxRPTTiming;
	u1Byte idx;

	idx=TxRPTTiming_idx;
	if (extend==0) // back to default timing
		idx=1;  //400ms
	else if (extend==1) {// increase the timing
		idx+=1;
		if (idx>5)
			idx=5;
	}
	else if (extend==2) {// decrease the timing
		if(idx!=0)
			idx-=1;
	}
	WriteTxRPTTiming=DynamicTxRPTTiming[idx];  
	TxRPTTiming_idx=idx;
	PlatformEFIOWrite2Byte(Adapter, REG_TX_RPT_TIME, WriteTxRPTTiming);
		
}
*/


void RateDecision(struct rtl8192cd_priv *priv, PSTATION_RA_INFO  pRaInfo)
{
	unsigned char RateID = 0, RtyPtID = 0, PenaltyID1 = 0, PenaltyID2 = 0;
	//unsigned int pool_retry;
	//unsigned char Try_Result=0;
	
	if (pRaInfo->Active && (pRaInfo->TOTAL > 0)) /* STA used and data packet exits */ {
#if 0
		if (pRaInfo->TryingState==1){
			
			pRaInfo->TryingState=0;
			Try_Result=EndRateTrying(Adapter,pRaInfo);
			if (Try_Result==1) // Try good
				pRaInfo->RateBeforeTrying=pRaInfo->DecisionRate;
			else // Try bad
				pRaInfo->DecisionRate=pRaInfo->RateBeforeTrying;
			goto RateDecisionFinish;
		}
#endif
		if ((pRaInfo->RssiStaRA<(pRaInfo->PreRssiStaRA-3))|| (pRaInfo->RssiStaRA>(pRaInfo->PreRssiStaRA+3))) {
			pRaInfo->RAWaitingCounter=0;
			pRaInfo->RAPendingCounter=0;
		}
		// Start RA decision
		if (pRaInfo->PreRate > pRaInfo->HighestRate)
			RateID = pRaInfo->HighestRate;
		else 
			RateID = pRaInfo->PreRate;
		if (pRaInfo->RssiStaRA > RSSI_THRESHOLD[RateID])
			RtyPtID=0;
		else
			RtyPtID=1;
		PenaltyID1 = RETRY_PENALTY_IDX[RtyPtID][RateID]; //TODO by page
		
/*		panic_printk("%s %d: NscDown init is %d\n", __FUNCTION__, __LINE__, pRaInfo->NscDown);*/
		//pool_retry=pRaInfo->RTY[2]+pRaInfo->RTY[3]+pRaInfo->RTY[4]+pRaInfo->DROP;
		pRaInfo->NscDown += pRaInfo->RTY[0] * RETRY_PENALTY[PenaltyID1][0];
		pRaInfo->NscDown += pRaInfo->RTY[1] * RETRY_PENALTY[PenaltyID1][1];
		pRaInfo->NscDown += pRaInfo->RTY[2] * RETRY_PENALTY[PenaltyID1][2];
		pRaInfo->NscDown += pRaInfo->RTY[3] * RETRY_PENALTY[PenaltyID1][3];
		pRaInfo->NscDown += pRaInfo->RTY[4] * RETRY_PENALTY[PenaltyID1][4];
/*		panic_printk("%s %d: NscDown is %d, total*penalty[5] is %d\n", __FUNCTION__, __LINE__,
					pRaInfo->NscDown, (pRaInfo->TOTAL * RETRY_PENALTY[PenaltyID1][5]));*/
		if (pRaInfo->NscDown > (pRaInfo->TOTAL * RETRY_PENALTY[PenaltyID1][5]))
			pRaInfo->NscDown -= pRaInfo->TOTAL * RETRY_PENALTY[PenaltyID1][5];
		else
			pRaInfo->NscDown=0;
		
		// rate up
		PenaltyID2 = RETRY_PENALTY_UP_IDX[RateID];
/*		panic_printk("%s %d: NscUp init is %d\n", __FUNCTION__, __LINE__, pRaInfo->NscUp);*/
		pRaInfo->NscUp += pRaInfo->RTY[0] * RETRY_PENALTY[PenaltyID2][0];
		pRaInfo->NscUp += pRaInfo->RTY[1] * RETRY_PENALTY[PenaltyID2][1];
		pRaInfo->NscUp += pRaInfo->RTY[2] * RETRY_PENALTY[PenaltyID2][2];
		pRaInfo->NscUp += pRaInfo->RTY[3] * RETRY_PENALTY[PenaltyID2][3];
		pRaInfo->NscUp += pRaInfo->RTY[4] * RETRY_PENALTY[PenaltyID2][4];
/*		panic_printk("%s %d: NscUp is %d, total*up[5] is %d\n", __FUNCTION__, __LINE__,
					pRaInfo->NscUp, (pRaInfo->TOTAL * RETRY_PENALTY[PenaltyID2][5]));*/
		if (pRaInfo->NscUp > (pRaInfo->TOTAL * RETRY_PENALTY[PenaltyID2][5]))
			pRaInfo->NscUp -= pRaInfo->TOTAL * RETRY_PENALTY[PenaltyID2][5];
		else
			pRaInfo->NscUp = 0;
		/*printk("%s %d, RTY0 %d, RTY1 %d, RTY2 %d, RTY3 %d, RTY4 %d, userate 0x%08x\n", 
			__FUNCTION__, __LINE__, pRaInfo->RTY[0], pRaInfo->RTY[1], pRaInfo->RTY[2], pRaInfo->RTY[3], pRaInfo->RTY[4], pRaInfo->RAUseRate);*/
/*		panic_printk("%s %d: RssiStaRa= %d RtyPtID=%d PenaltyID1=0x%x  PenaltyID2=0x%x RateID=%d NscDown=%d NscUp=%d\n", 
			__FUNCTION__, __LINE__,
			pRaInfo->RssiStaRA,RtyPtID, PenaltyID1,PenaltyID2, RateID, pRaInfo->NscDown, pRaInfo->NscUp);*/
		if ((pRaInfo->NscDown < N_THRESHOLD_LOW[RateID]) ||(pRaInfo->DROP>DROPING_NECESSARY[RateID]))
			RateDown(priv, pRaInfo);
		//else if ((pRaInfo->NscUp > N_THRESHOLD_HIGH[RateID])&&(pool_retry<POOL_RETRY_TH[RateID]))
		else if (pRaInfo->NscUp > N_THRESHOLD_HIGH[RateID])
			RateUp(priv, pRaInfo);
#if 0
		RateDecisionFinish:
#endif
		if ((pRaInfo->DecisionRate)==(pRaInfo->PreRate)) 
			DynamicTxRPTTimingCounter+=1;
		else
			DynamicTxRPTTimingCounter=0;

		if (DynamicTxRPTTimingCounter>=4) {
			RTL8188E_SetTxReportTimeByRA(priv, 1);
/*			panic_printk("%s %d, <=====Rate don't change 4 times, Extend RPT Timing\n", __FUNCTION__, __LINE__);*/
			DynamicTxRPTTimingCounter=0;
		}

		pRaInfo->RptTime = DynamicTxRPTTiming[TxRPTTiming_idx];
		pRaInfo->PreRate = pRaInfo->DecisionRate;
			
		ResetRaCounter( pRaInfo);
	}
}
#endif
#endif

