#ifndef __HAL88XX_DM_H__
#define __HAL88XX_DM_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXDM.h
	
Abstract:
	Defined HAL 88XX Dynamic Mechanism Related Define & Marco
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2013-08-19 Filen            Create.	
--*/

typedef enum _RSSI_LVL_DM_88XX_ {
    RSSI_LVL_LOW        = 0,
    RSSI_LVL_NORMAL     = 1,
    RSSI_LVL_HIGH       = 2,
    RSSI_LVL_MAX_NUM
}RSSI_LVL_DM_88XX, *PRSSI_LVL_DM_88XX;


typedef enum _MACDM_MODE_88XX_ {
    MACDM_MODE_STOP             = 0,
    MACDM_MODE_MAX_TP           = 1,
    MACDM_MODE_AVERAGE          = 2,
    MACDM_MODE_MIN_TP           = 3,
    MACDM_MODE_MAX_NUM
}MACDM_MODE_88XX, *PMACDM_MODE_88XX;


typedef enum _MACDM_TP_STATE_88XX_ {
    MACDM_TP_STATE_DEFAULT          = 0,
    MACDM_TP_STATE_GENERAL          = 1,
    MACDM_TP_STATE_TXOP             = 2,
    MACDM_TP_STATE_MAX_NUM
}MACDM_TP_STATE_88XX, *PMACDM_TP_STATE_88XX;


typedef enum _MACDM_TP_THRS_88XX_ {
    MACDM_TP_THRS_DEF_TO_GEN          = 0,
    MACDM_TP_THRS_GEN_TO_DEF          = 1,
    MACDM_TP_THRS_GEN_TO_TXOP         = 2,
    MACDM_TP_THRS_TXOP_TO_GEN         = 3,
    MACDM_TP_THRS_MAX_NUM
}MACDM_TP_THRS_88XX, *PMACDM_TP_THRS_88XX;


typedef enum _MACDM_AGGRE_STATE_{
    MACDM_AGGRE_STATE_NONE = 0,
    MACDM_AGGRE_STATE_TXOP,
    MACDM_AGGRE_STATE_TXOP_EARLY
} MACDM_AGGRE_STATE, *PMACDM_AGGRE_STATE;


VOID
InitMACDM88XX(
    IN  HAL_PADAPTER    Adapter
);


VOID
Timer1SecDM88XX(
    IN  HAL_PADAPTER    Adapter
);


u4Byte
CalMaxAggreNum(
    IN  u4Byte  page_size,
    IN  u4Byte  min_pageNum, //the minimum pageNum of "dedicated queue + pub queue"
    IN  BOOLEAN AMSDU_En,
    IN  u4Byte  txdesc_len
);


VOID
DecisionAggrePara(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          aggre_state,
    IN  u4Byte          TxopMaxAggreNum
);




#endif  //__HAL88XX_DM_H__

