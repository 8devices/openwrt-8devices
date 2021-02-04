/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
 
#ifndef	__PHYDMDIG_H__
#define    __PHYDMDIG_H__

#define DIG_VERSION	"1.32"	/* 2016.09.02  YuChen. add CCK PD for 8197F*/
#define DIG_HW		1

enum lna_sat_chk_timmer {
	INIT_LNA_SAT_CHK_TIMMER,
	CANCEL_LNA_SAT_CHK_TIMMER,
	RELEASE_LNA_SAT_CHK_TIMMER
};
enum tdma_dig_timer {
	INIT_TDMA_DIG_TIMMER,
	CANCEL_TDMA_DIG_TIMMER,
	RELEASE_TDMA_DIG_TIMMER
};
#if (PHYDM_LNA_SAT_CHK_TYPE2 == 1)

	#define	DELTA_STD	2
	#define DELTA_MEAN	2
	#define SNR_STATISTIC_SHIFT	8
	
	enum lna_sat_chk_type2_status {

		ORI_TABLE_MONITOR,
		ORI_TABLE_TRAINING,
		SAT_TABLE_MONITOR,
		SAT_TABLE_TRAINING,
		SAT_TABLE_TRY_FAIL,
		ORI_TABLE_TRY_FAIL
	};

	
#endif
#define RSSI_TO_IGI						10
#define ONE_SEC_MS						1000
#define OFDM_AGC_TAB_0					0
#define OFDM_AGC_TAB_2					2

/* Pause DIG & CCKPD */
#define		DM_DIG_MAX_PAUSE_TYPE		0x7
#define		ONE_SEC_MS							1000

enum is_high_low_sel {
	TDMA_DIG_LOW_STATE = 0,
	TDMA_DIG_HIGH_STATE = 1,
	NORMAL_DIG = 2
};

enum rx_gain_min_check_state {
	OSC_CHECK_STATE		= 0,
	OSC_LOCK_STATE		= 1,
	OSC_RELEASE_STATE	= 2
};

enum fa_cca_ratio_thd {
	FA_CCA_RATIO_L2H	= 50,
	FA_CCA_RATIO_H2L	= 40
};
typedef enum tag_DIG_GoUpCheck_Level {

	DIG_GOUPCHECK_LEVEL_0,
	DIG_GOUPCHECK_LEVEL_1,
	DIG_GOUPCHECK_LEVEL_2
	
} DIG_GOUPCHECK_LEVEL;

#ifdef CFG_DIG_DAMPING_CHK
#define DIG_RECORD_NUM	4
#define DIG_LIMIT_PERIOD 60 /*60 sec*/

struct phydm_dig_recoder_strcut {
	u8		igi_bitmap; /*Do not add any new parameter before this*/
	u8		igi_history[DIG_RECORD_NUM];
	u32		fa_history[DIG_RECORD_NUM];
	u8		damping_limit_en;
	u8		damping_limit_val; /*Limit IGI_dyn_min*/
	u32		limit_time;
	u8		limit_rssi;
};
#endif

typedef struct _Dynamic_Initial_Gain_Threshold_
{
#ifdef CFG_DIG_DAMPING_CHK
	struct phydm_dig_recoder_strcut dig_recoder_t;
	u8		dig_dl_en; /*damping limit function enable*/
#endif
	u16		fa_th[3];
	BOOLEAN		bStopDIG;		// for debug
	BOOLEAN		bIgnoreDIG;
	BOOLEAN		bPSDInProgress;

	u1Byte		Dig_Enable_Flag;
	u1Byte		Dig_Ext_Port_Stage;
	
	int		   	RssiLowThresh;
	int		    	RssiHighThresh;

	u4Byte		FALowThresh;
	u4Byte		FAHighThresh;

	u1Byte		CurSTAConnectState;
	u1Byte		PreSTAConnectState;
	u1Byte		CurMultiSTAConnectState;

	u1Byte		PreIGValue;
	u1Byte		CurIGValue;
#if (PHYDM_TDMA_DIG_SUPPORT == 1)
	u1Byte		tdma_igi[2];
	RT_TIMER	phydm_tdma_dig_timer;
	u1Byte		tdma_rx_gain_max[2];
	u1Byte		tdma_rx_gain_min[2];

	/*phydm_rx_gain_min_check*/
	u1Byte		rx_gain_min_check_state[2];		//for tdma use
	u1Byte		stay_check_state_cnt[2];		// check state	
	u1Byte		goto_lock_state_cnt[2];		// check state
	u1Byte		goto_release_state_cnt[2];	// lock state
	u1Byte		stay_release_state_cnt[2];	// release state
#endif
	
	u1Byte		BackupIGValue;		//MP DIG
	u1Byte		BT30_CurIGI;
	u1Byte		IGIBackup;

	s1Byte		BackoffVal;
	s1Byte		BackoffVal_range_max;
	s1Byte		BackoffVal_range_min;
	u1Byte		rx_gain_range_max;
	u1Byte		rx_gain_range_min;
	u1Byte		Rssi_val_min;

	u1Byte		PreCCK_CCAThres;
	u1Byte		CurCCK_CCAThres;
	u1Byte		PreCCKPDState;
	u1Byte		CurCCKPDState;
	u1Byte		CCKPDBackup;
	u1Byte		pause_cckpd_level;
	u1Byte		pause_cckpd_value[DM_DIG_MAX_PAUSE_TYPE + 1];

	u1Byte		LargeFAHit;
	u1Byte		LargeFA_Timeout;		/*if (LargeFAHit), monitor "LargeFA_Timeout" sec, if timeout, LargeFAHit=0*/
	u1Byte		ForbiddenIGI;
	u4Byte		Recover_cnt;

	u1Byte		DIG_Dynamic_MIN_0;
	u1Byte		DIG_Dynamic_MIN_1;
	BOOLEAN		bMediaConnect_0;
	BOOLEAN		bMediaConnect_1;

	u4Byte		AntDiv_RSSI_max;
	u4Byte		RSSI_max;

	u1Byte		*bP2PInProcess;

	u1Byte		pause_dig_level;
	u1Byte		pause_dig_value[DM_DIG_MAX_PAUSE_TYPE + 1];

	u4Byte		cckFaMa;
	DIG_GOUPCHECK_LEVEL		DIG_GoUpCheck_Level;
	u1Byte		aaa_default;

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	BOOLEAN					bTpTarget;
	BOOLEAN					bNoiseEst;
	u4Byte					TpTrainTH_min;
	u1Byte					IGIOffset_A;
	u1Byte					IGIOffset_B;
#endif

#if (RTL8822B_SUPPORT == 1 || RTL8197F_SUPPORT == 1 || RTL8821C_SUPPORT == 1)
	u1Byte		rfGainIdx;
	u1Byte		agcTableIdx;
	u1Byte		bigJumpLmt[16];
	u1Byte		enableAdjustBigJump:1;
	u1Byte		bigJumpStep1:3;
	u1Byte		bigJumpStep2:2;
	u1Byte		bigJumpStep3:2;
#endif

#if (DIG_HW == 1)
	u1Byte		pre_rssi_min;
#endif

#if (PHYDM_TDMA_DIG_SUPPORT == 1)
	u1Byte		tdma_dig_state;
	u1Byte		tdma_dig_cnt;			// for timer check
	u1Byte		tdma_dig_block_cnt;		// for high/low state false alarm statistics
	bool		one_sec_dump;
	u1Byte		pre_tdma_dig_cnt;
#endif	
	u1Byte		normalize_1sec_factor;
	u4Byte		pre_tsf;
	u4Byte		cur_tsf;
	u4Byte		fa_start_tsf;
	u4Byte		fa_end_tsf;
	u4Byte		fa_acc_1sec_tsf;
}DIG_T,*pDIG_T;

typedef struct _FALSE_ALARM_STATISTICS{
	u4Byte		Cnt_Parity_Fail;
	u4Byte		Cnt_Rate_Illegal;
	u4Byte		Cnt_Crc8_fail;
	u4Byte		cnt_crc8_fail_vht;
	u4Byte		Cnt_Mcs_fail;
	u4Byte		cnt_mcs_fail_vht;
	u4Byte		Cnt_Ofdm_fail;
	u4Byte		Cnt_Ofdm_fail_pre;	//For RTL8881A
	u4Byte		Cnt_Cck_fail;
	u4Byte		Cnt_all;
	u4Byte		Cnt_all_pre;
	u4Byte		Cnt_Fast_Fsync;
	u4Byte		Cnt_SB_Search_fail;
	u4Byte		Cnt_OFDM_CCA;
	u4Byte		Cnt_CCK_CCA;
	u4Byte		Cnt_CCA_all;
	u4Byte		Cnt_BW_USC;	//Gary
	u4Byte		Cnt_BW_LSC;	//Gary
	u4Byte		cnt_cck_crc32_error;
	u4Byte		cnt_cck_crc32_ok;
	u4Byte		cnt_ofdm_crc32_error;
	u4Byte		cnt_ofdm_crc32_ok;
	u4Byte		cnt_ht_crc32_error;
	u4Byte		cnt_ht_crc32_ok;
	u4Byte		cnt_vht_crc32_error;
	u4Byte		cnt_vht_crc32_ok;
	u4Byte		cnt_crc32_error_all;
	u4Byte		cnt_crc32_ok_all;
	u4Byte		time_fa_all;
	BOOLEAN		cck_block_enable;
	BOOLEAN		ofdm_block_enable;
	u4Byte		dbg_port0;
	BOOLEAN		edcca_flag;
}FALSE_ALARM_STATISTICS, *PFALSE_ALARM_STATISTICS;

#if (PHYDM_TDMA_DIG_SUPPORT == 1)
typedef struct _FALSE_ALARM_STATISTICS_ACC{
	u4Byte		Cnt_Parity_Fail[2];
	u4Byte		Cnt_Rate_Illegal[2];
	u4Byte		Cnt_Crc8_fail[2];
	u4Byte		Cnt_Mcs_fail[2];
	u4Byte		Cnt_Ofdm_fail[2];
	u4Byte		Cnt_Ofdm_fail_pre[2];	//For RTL8881A
	u4Byte		Cnt_Cck_fail[2];
	u4Byte		Cnt_all[2];
	u4Byte		Cnt_all_pre[2];

	u4Byte		Cnt_Fast_Fsync[2];
	u4Byte		Cnt_SB_Search_fail[2];
	u4Byte		Cnt_OFDM_CCA[2];
	u4Byte		Cnt_CCK_CCA[2];
	u4Byte		Cnt_CCA_all[2];

	u4Byte		cnt_cck_crc32_error[2];
	u4Byte		cnt_cck_crc32_ok[2];
	u4Byte		cnt_ofdm_crc32_error[2];
	u4Byte		cnt_ofdm_crc32_ok[2];
	u4Byte		cnt_ht_crc32_error[2];
	u4Byte		cnt_ht_crc32_ok[2];
	u4Byte		cnt_vht_crc32_error[2];
	u4Byte		cnt_vht_crc32_ok[2];
	u4Byte		cnt_crc32_error_all[2];
	u4Byte		cnt_crc32_ok_all[2];

	u4Byte		Cnt_all_1sec[2];
	u4Byte		Cnt_CCA_all_1sec[2];
	u4Byte		Cnt_Cck_fail_1sec[2];
	u4Byte		Cnt_Ofdm_fail_1sec[2];
	u4Byte		fa_cca_ratio[2];
	bool		is_fa_bigger_cca[2];
}FALSE_ALARM_STATISTICS_ACC, *PFALSE_ALARM_STATISTICS_ACC;
#endif

typedef struct _LNA_SATURATION_INFO{
	u4Byte		sat_cnt_acc_patha;
	u4Byte		sat_cnt_acc_pathb;
	u4Byte		check_time;
	bool		pre_sat_status;
	bool		cur_sat_status;
	RT_TIMER	phydm_lna_sat_chk_timer;
	u4Byte		cur_timer_check_cnt;
	u4Byte		pre_timer_check_cnt;
}LNA_SATURATION_INFO, *PLNA_SATURATION_INFO;

#if (PHYDM_LNA_SAT_CHK_TYPE2 == 1)

	typedef struct _LNA_SAT_TYPE2_INFO{
		u4Byte		cur_snr_mean;
		u4Byte		pre_snr_mean;
		u4Byte		cur_lower_snr_mean;
		u4Byte		pre_lower_snr_mean;
		u1Byte		cnt_lower_snr_statistic;
		u4Byte		cur_snr_var;
		u4Byte		pre_snr_var;
		u1Byte		nxt_state;
		u1Byte		pre_state;
		bool		is_sm_done;
		bool		is_snr_done;
		bool		is_force_table;

		s1Byte		snr_statistic[1024];
		u2Byte		snr_statistic_sqr[1024];
		u4Byte		cnt_snr_statistic;
		u2Byte		total_cnt_snr_statistic;
		u1Byte		ori_table_try_fail_times;
		u1Byte		sat_table_monitor_times;
		u1Byte		force_change_period;
//		u4Byte		sum_snr_statistic;
	}LNA_SAT_TYPE2_INFO, *PLNA_SAT_TYPE2_INFO;

#endif	//#if (PHYDM_LNA_SAT_CHK_TYPE2 == 1)

typedef enum tag_Dynamic_Init_Gain_Operation_Type_Definition
{
	DIG_TYPE_THRESH_HIGH	= 0,
	DIG_TYPE_THRESH_LOW	= 1,
	DIG_TYPE_BACKOFF		= 2,
	DIG_TYPE_RX_GAIN_MIN	= 3,
	DIG_TYPE_RX_GAIN_MAX	= 4,
	DIG_TYPE_ENABLE 		= 5,
	DIG_TYPE_DISABLE 		= 6,
	DIG_OP_TYPE_MAX
}DM_DIG_OP_E;

/*
typedef enum tag_CCK_Packet_Detection_Threshold_Type_Definition
{
	CCK_PD_STAGE_LowRssi = 0,
	CCK_PD_STAGE_HighRssi = 1,
	CCK_PD_STAGE_MAX = 3,
}DM_CCK_PDTH_E;

typedef enum tag_DIG_EXT_PORT_ALGO_Definition
{
	DIG_EXT_PORT_STAGE_0 = 0,
	DIG_EXT_PORT_STAGE_1 = 1,
	DIG_EXT_PORT_STAGE_2 = 2,
	DIG_EXT_PORT_STAGE_3 = 3,
	DIG_EXT_PORT_STAGE_MAX = 4,
}DM_DIG_EXT_PORT_ALG_E;

typedef enum tag_DIG_Connect_Definition
{
	DIG_STA_DISCONNECT = 0,	
	DIG_STA_CONNECT = 1,
	DIG_STA_BEFORE_CONNECT = 2,
	DIG_MultiSTA_DISCONNECT = 3,
	DIG_MultiSTA_CONNECT = 4,
	DIG_CONNECT_MAX
}DM_DIG_CONNECT_E;


#define DM_MultiSTA_InitGainChangeNotify(Event) {DM_DigTable.CurMultiSTAConnectState = Event;}

#define DM_MultiSTA_InitGainChangeNotify_CONNECT(_ADAPTER)	\
	DM_MultiSTA_InitGainChangeNotify(DIG_MultiSTA_CONNECT)

#define DM_MultiSTA_InitGainChangeNotify_DISCONNECT(_ADAPTER)	\
	DM_MultiSTA_InitGainChangeNotify(DIG_MultiSTA_DISCONNECT)
*/

typedef enum tag_PHYDM_Pause_Type {
	PHYDM_PAUSE = BIT0,
	PHYDM_RESUME = BIT1
} PHYDM_PAUSE_TYPE;

typedef enum tag_PHYDM_Pause_Level {
/* number of pause level can't exceed DM_DIG_MAX_PAUSE_TYPE */
	PHYDM_PAUSE_LEVEL_0 = 0,
	PHYDM_PAUSE_LEVEL_1 = 1,
	PHYDM_PAUSE_LEVEL_2 = 2,
	PHYDM_PAUSE_LEVEL_3 = 3,
	PHYDM_PAUSE_LEVEL_4 = 4,
	PHYDM_PAUSE_LEVEL_5 = 5,
	PHYDM_PAUSE_LEVEL_6 = 6,
	PHYDM_PAUSE_LEVEL_7 = DM_DIG_MAX_PAUSE_TYPE		/* maximum level */
} PHYDM_PAUSE_LEVEL;

#define		DM_DIG_THRESH_HIGH			40
#define		DM_DIG_THRESH_LOW			35

#define		DM_FALSEALARM_THRESH_LOW	400
#define		DM_FALSEALARM_THRESH_HIGH	1000

#define		DM_DIG_MAX_NIC				0x3e
#define		DM_DIG_MIN_NIC				0x20
#define		DM_DIG_MAX_OF_MIN_NIC		0x3e

#if (DIG_HW == 1)
#define		DM_DIG_MAX_AP					pDM_Odm->priv->pshare->rf_ft_var.dbg_dig_upper //0x3e
#define		DM_DIG_MIN_AP					((pDM_Odm->SupportICType & (ODM_RTL8812|ODM_RTL8822B))?0x1c:0x20)//0x1c
#else
#define		DM_DIG_MAX_AP					0x3e
#define		DM_DIG_MIN_AP					((pDM_Odm->SupportICType & (ODM_RTL8812|ODM_RTL8822B))?0x1c:0x20)//0x20
#endif
#define		DM_DIG_MAX_OF_MIN			0x2A	//0x32
#define		DM_DIG_MIN_AP_DFS				0x20

#define		DM_DIG_MAX_NIC_HP			0x46
#define		DM_DIG_MIN_NIC_HP				0x2e

#define		DM_DIG_MAX_AP_HP				0x42
#define		DM_DIG_MIN_AP_HP				0x30

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
#define		DM_DIG_MAX_AP_COVERAGR		0x26
#if (DIG_HW == 1)
#define		DM_DIG_MIN_AP_COVERAGE		((pDM_Odm->SupportICType & (ODM_RTL8812 | ODM_RTL8822B))?0x1c:0x20)
#else
#define		DM_DIG_MIN_AP_COVERAGE		0x1c
#endif
#define		DM_DIG_MAX_OF_MIN_COVERAGE	0x2A//0x22

#define		DM_DIG_TP_Target_TH0			500
#define		DM_DIG_TP_Target_TH1			1000
#define		DM_DIG_TP_Training_Period		10
#endif

#define		AAA_BASE					pDM_Odm->priv->pshare->rf_ft_var.dbg_aaa_base/*4*/
#define		AAA_STEP					pDM_Odm->priv->pshare->rf_ft_var.dbg_aaa_step/*2*/

//vivi 92c&92d has different definition, 20110504
//this is for 92c
#if (DM_ODM_SUPPORT_TYPE & ODM_CE)
	#ifdef CONFIG_SPECIAL_SETTING_FOR_FUNAI_TV
	#define		DM_DIG_FA_TH0				0x80//0x20
	#else
	#define		DM_DIG_FA_TH0				0x200//0x20
	#endif
#else
	#define		DM_DIG_FA_TH0				0x200//0x20
#endif

#define		DM_DIG_FA_TH1					0x300
#define		DM_DIG_FA_TH2					0x400
//this is for 92d
#define		DM_DIG_FA_TH0_92D				0x100
#define		DM_DIG_FA_TH1_92D				0x400
#define		DM_DIG_FA_TH2_92D				0x600

#define		DM_DIG_BACKOFF_MAX			12
#define		DM_DIG_BACKOFF_MIN			-4
#define		DM_DIG_BACKOFF_DEFAULT		10

#define 		DM_DIG_FA_TH0_LPS				4 //-> 4 in lps
#define 		DM_DIG_FA_TH1_LPS				15 //-> 15 lps
#define 		DM_DIG_FA_TH2_LPS				30 //-> 30 lps
#define 		RSSI_OFFSET_DIG				0x05
#define		LARGE_FA_TIMEOUT				60


VOID
ODM_ChangeDynamicInitGainThresh(
	IN		PVOID					pDM_VOID,
	IN		u4Byte  					DM_Type,
	IN		u4Byte 					DM_Value
	);

VOID
ODM_Write_DIG(
	IN		PVOID					pDM_VOID, 	
	IN		u1Byte					CurrentIGI
	);

VOID
odm_PauseDIG(
	IN		PVOID					pDM_VOID,
	IN		PHYDM_PAUSE_TYPE		PauseType,
	IN		PHYDM_PAUSE_LEVEL		pause_level,
	IN		u1Byte					IGIValue
	);

VOID
odm_DIGInit(
	IN		PVOID					pDM_VOID
	);

VOID	
odm_DIG(
	IN		PVOID					pDM_VOID
	);

void
phydm_lna_sat_chk_watchdog(
	IN		PVOID		pDM_VOID
);

void
phydm_lna_sat_chk_init(
	IN		PVOID		pDM_VOID
);

void
phydm_lna_sat_chk_callback(
	IN		PVOID		pDM_VOID
);

void
phydm_lna_sat_chk_timers(
	IN		PVOID		pDM_VOID,
	u8		state
);

void
phydm_lna_sat_chk(
	IN		PVOID		pDM_VOID
);

void
phydm_set_ofdm_agc_tab(
	IN		PVOID		pDM_VOID,
	IN		u1Byte		tab_sel
);

u1Byte
phydm_get_ofdm_agc_tab(
	IN		PVOID		pDM_VOID
);


u1Byte
phydm_dig_current_igi_by_fa_th(
	IN		PVOID		pDM_VOID,
	u1Byte				CurrentIGI,
	pu4Byte				dm_FA_thres,
	pu1Byte				step_size,
	bool				DIG_GoUpCheck,
	u1Byte				is_high_low_sel
	);

BOOLEAN 
odm_DigAbort(
	IN		PVOID			pDM_VOID
	);
	
VOID
odm_DIGbyRSSI_LPS(
	IN		PVOID					pDM_VOID
	);

VOID 
odm_FalseAlarmCounterStatistics(
	IN		PVOID					pDM_VOID
	);

#if (PHYDM_TDMA_DIG_SUPPORT == 1)

u1Byte
phydm_rx_gain_min_check(
	IN		PVOID			pDM_VOID,
	IN		u1Byte			dym_min,
	IN		u1Byte			CurrentIGI,
	IN		u1Byte			rx_gain_min,
	bool					is_high_low_sel
	);

void
phydm_tdma_dig_timers(
	IN		PVOID		pDM_VOID,
	u8		state
	);

VOID 
phydm_tdma_low_dig(
	IN		PVOID		pDM_VOID
	);

VOID 
phydm_tdma_high_dig(
	IN		PVOID		pDM_VOID
	);

VOID
phydm_tdma_dig_timer_check(
	IN		PVOID					pDM_VOID
	);

VOID
phydm_tdma_dig_call_back(
	IN		PVOID		pDM_VOID
	);

VOID 
phydm_TDMA_FalseAlarmCounterCheck(
	IN		PVOID		pDM_VOID
	);

VOID
odm_FalseAlarmCounterAcc(
	IN		PVOID		pDM_VOID,
	IN		BOOLEAN		rssi_dump_en,
	bool				high_low_state
	);

VOID 
odm_FalseAlarmCounterAccReset(
	IN		PVOID		pDM_VOID,
	u8				high_low_idx
	);

#endif	//#if (PHYDM_TDMA_DIG_SUPPORT == 1)

VOID 
odm_FalseAlarmCounterReset(
	IN		PVOID		pDM_VOID
	);


VOID
odm_FalseAlarmCounterRegReset(
	IN		PVOID					pDM_VOID
	);

VOID
odm_FalseAlarmCounterRegHold(
	IN		PVOID					pDM_VOID
	);

VOID
odm_PauseCCKPacketDetection(
	IN		PVOID					pDM_VOID,
	IN		PHYDM_PAUSE_TYPE		PauseType,
	IN		PHYDM_PAUSE_LEVEL		pause_level,
	IN		u1Byte					CCKPDThreshold
	);

VOID 
odm_CCKPacketDetectionThresh(
	IN		PVOID					pDM_VOID
	);

VOID 
ODM_Write_CCK_CCA_Thres(
	IN		PVOID					pDM_VOID, 
	IN		u1Byte					CurCCK_CCAThres
	);

BOOLEAN
phydm_DIG_GoUpCheck(
	IN		PVOID		pDM_VOID
	);


#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
VOID
odm_MPT_DIGCallback(
	PRT_TIMER						pTimer
);

VOID
odm_MPT_DIGWorkItemCallback(
    IN 		PVOID            			pContext
    );

#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
VOID
odm_MPT_DIGCallback(
	IN		PVOID					pDM_VOID
);
#endif

#if (DM_ODM_SUPPORT_TYPE != ODM_CE)
VOID
ODM_MPT_DIG(
	IN		PVOID					pDM_VOID
);
#endif


#endif

#if (PHYDM_LNA_SAT_CHK_TYPE2 == 1)

void
phydm_snr_collect(
	IN		PVOID		pDM_VOID,
	IN		u8			rx_snr
	);

void
phydm_lna_sat_type2_sm(
	IN		PVOID		pDM_VOID
	);

void
phydm_lna_sat_chk_type2_init(
	IN		PVOID		pDM_VOID
	);

void
phydm_snr_data_processing(
	IN		PVOID		pDM_VOID
	);

void
phydm_bubble_sort(
	IN		PVOID		pDM_VOID,
	IN		u1Byte		*array,
	IN		u2Byte		array_length
);

#endif

