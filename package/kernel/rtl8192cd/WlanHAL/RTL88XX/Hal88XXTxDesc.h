#ifndef __HAL88XX_TXDESC_H__
#define __HAL88XX_TXDESC_H__

typedef struct _TX_DESC_88XX_
{
    volatile    u4Byte     Dword0;
    volatile    u4Byte     Dword1;
    volatile    u4Byte     Dword2;
    volatile    u4Byte     Dword3;
    volatile    u4Byte     Dword4;
    volatile    u4Byte     Dword5;
    volatile    u4Byte     Dword6;
    volatile    u4Byte     Dword7;
    volatile    u4Byte     Dword8;
    volatile    u4Byte     Dword9;
    volatile    u4Byte     Dword10;
    volatile    u4Byte     Dword11;        
} TX_DESC_88XX, *PTX_DESC_88XX;

#define SIZE_TXDESC_88XX    40
#define SIZE_TXDESC_88XX_V1    48

#define SIZE_TXDESC_88XX_MAX   48


typedef enum _HCI_TX_DMA_QUEUE_88XX_
{
    //MGT
    HCI_TX_DMA_QUEUE_MGT = 0,

    //QoS
    HCI_TX_DMA_QUEUE_BK,
    HCI_TX_DMA_QUEUE_BE,
    HCI_TX_DMA_QUEUE_VI,
    HCI_TX_DMA_QUEUE_VO,

    //HI
    HCI_TX_DMA_QUEUE_HI0,
    HCI_TX_DMA_QUEUE_HI1,
    HCI_TX_DMA_QUEUE_HI2,
    HCI_TX_DMA_QUEUE_HI3,
    HCI_TX_DMA_QUEUE_HI4,
    HCI_TX_DMA_QUEUE_HI5,
    HCI_TX_DMA_QUEUE_HI6,
    HCI_TX_DMA_QUEUE_HI7,
    HCI_TX_DMA_QUEUE_CMD,            
    // Beacon
    HCI_TX_DMA_QUEUE_BCN,
    HCI_TX_DMA_QUEUE_MAX_NUM        //14
} HCI_TX_DMA_QUEUE_88XX, *PHCI_TX_DMA_QUEUE_88XX;


#if 0
// TODO: endian....
#ifdef _BIG_ENDIAN_
typedef struct _TXBD_ELEMENT_DW0_
{
    u4Byte         Rsvd_31         :1;
    u4Byte         PsbLen          :15;
    u4Byte         Rsvd_14_15      :2;
    u4Byte         Len             :14;
} TXBD_ELEMENT_DW0,*PTXBD_ELEMENT_DW0;

typedef struct _TXBD_ELEMENT_DW_
{
    u4Byte         AmpduEn         :1;
    u4Byte         Rsvd16To30      :15;
    u4Byte         Len             :16;
} TXBD_ELEMENT_DW,*PTXBD_ELEMENT_DW;
#else // _LITTLE_ENDIAN_
typedef struct _TXBD_ELEMENT_DW0_
{
    u4Byte         Len             :14;
    u4Byte         Rsvd_14_15      :2;
    u4Byte         PsbLen          :15;
    u4Byte         Rsvd_31         :1;
} TXBD_ELEMENT_DW0,*PTXBD_ELEMENT_DW0;

typedef struct _TXBD_ELEMENT_DW_
{
    u4Byte         Len             :16;
    u4Byte         Rsvd16To30      :15;
    u4Byte         AmpduEn         :1;
} TXBD_ELEMENT_DW,*PTXBD_ELEMENT_DW;
#endif
#endif

typedef struct _TXBD_ELEMENT_
{
    u4Byte              Dword0;
    u4Byte              Dword1;
#if TXBD_SEG_32_64_SEL
    u4Byte              Dword2;
    u4Byte              Dword3;
#endif  //TXBD_SEG_32_64_SEL
} TXBD_ELEMENT,*PTXBD_ELEMENT;

typedef struct _TX_BUFFER_DESCRIPTOR_
{
    TXBD_ELEMENT	TXBD_ELE[TXBD_ELE_NUM];
} TX_BUFFER_DESCRIPTOR, *PTX_BUFFER_DESCRIPTOR;


typedef struct _HCI_TX_DMA_QUEUE_STRUCT_88XX_
{
    //TXBD       
    PTX_BUFFER_DESCRIPTOR   pTXBD_head;

    //TXBD Queue management    
    u2Byte                  hw_idx;
    u2Byte	                host_idx;

    //Two Method:
    // 1.) TXDESC Only
    // 2.) TXDESC + Payload
    PVOID                   ptx_desc_head;
    u2Byte                  total_txbd_num;
    u2Byte                  avail_txbd_num;

    // RWPtr IDX Reg
    u4Byte                  reg_rwptr_idx;
} HCI_TX_DMA_QUEUE_STRUCT_88XX, *PHCI_TX_DMA_QUEUE_STRUCT_88XX;

typedef struct _HCI_TX_DMA_MANAGER_88XX_
{
    HCI_TX_DMA_QUEUE_STRUCT_88XX  tx_queue[HCI_TX_DMA_QUEUE_MAX_NUM];
} HCI_TX_DMA_MANAGER_88XX, *PHCI_TX_DMA_MANAGER_88XX;

#if CFG_HAL_TX_AMSDU
typedef struct _TX_BUFFER_DESCRIPTOR_AMSDU_
{
    TXBD_ELEMENT	TXBD_ELE[MAX_NUM_OF_MSDU_IN_AMSDU];
} TX_BUFFER_DESCRIPTOR_AMSDU, *PTX_BUFFER_DESCRIPTOR_AMSDU;

typedef enum _HCI_TX_AMSDU_DMA_QUEUE_88XX_
{
    //QoS
    HCI_TX_AMSDU_DMA_QUEUE_BK,
    HCI_TX_AMSDU_DMA_QUEUE_BE,
    HCI_TX_AMSDU_DMA_QUEUE_VI,
    HCI_TX_AMSDU_DMA_QUEUE_VO,
    HCI_TX_AMSDU_DMA_QUEUE_MAX_NUM
} HCI_TX_AMSDU_DMA_QUEUE_88XX, *PHCI_TX_AMSDU_DMA_QUEUE_88XX;

typedef struct _HCI_TX_AMSDU_DMA_QUEUE_STRUCT_88XX_
{
    //AMSDU TXBD
    PTX_BUFFER_DESCRIPTOR_AMSDU     pTXBD_head_amsdu;

    //Current AMSDU TXBD element
    u1Byte                          cur_txbd_element;
} HCI_TX_AMSDU_DMA_QUEUE_STRUCT_88XX, *PHCI_TX_AMSDU_DMA_QUEUE_STRUCT_88XX;

typedef struct _HCI_TX_AMSDU_DMA_MANAGER_88XX_
{
    HCI_TX_AMSDU_DMA_QUEUE_STRUCT_88XX  tx_amsdu_queue[HCI_TX_AMSDU_DMA_QUEUE_MAX_NUM];
} HCI_TX_AMSDU_DMA_MANAGER_88XX, *PHCI_TX_AMSDU_DMA_MANAGER_88XX;

#endif // CFG_HAL_TX_AMSDU


//typedef struct _TXBD_INFO_
//{
//    u4Byte      Length;
//    u4Byte      AddrLow;
//} TXBD_INFO_88XX, *PTXBD_INFO_88XX;

typedef struct _TX_DESC_DATA_88XX_
{
    // header
    pu1Byte         pHdr;
    u4Byte          hdrLen;    
    u4Byte          llcLen;

    // frame
    pu1Byte         pBuf;    
    u4Byte          frLen;    

    // encryption
    pu1Byte         pMic;
    pu1Byte         pIcv;

    // TXDESC Dword 1
    u4Byte          rateId;
    u1Byte          macId;
    u1Byte          tid;
    BOOLEAN         moreData;    
	u1Byte          enDescId;
    
    // TXDESC Dword 2
    BOOLEAN         aggEn;
    u1Byte          ampduDensity;
    BOOLEAN         frag;
    BOOLEAN         bk;
    u4Byte          p_aid;	
    BOOLEAN         g_id;
#if CFG_HAL_HW_AES_IV
    BOOLEAN         hwAESIv;
#endif
    u1Byte			cca_rts;


    // TXDESC Dword 3
    BOOLEAN         RTSEn;
    BOOLEAN         HWRTSEn;
    BOOLEAN         CTS2Self;
    BOOLEAN         useRate;
    BOOLEAN         disRTSFB;
    BOOLEAN         disDataFB;
    u1Byte          maxAggNum;
    BOOLEAN         navUseHdr;
    BOOLEAN         ndpa;

    // TXDESC Dword 4
    u1Byte          RTSRate;
    u1Byte          RTSRateFBLmt;
    u1Byte          dataRate;
    BOOLEAN         rtyLmtEn;
    u1Byte          dataRtyLmt;
    u1Byte          dataRateFBLmt;    
    u1Byte          BMCRtyLmt;

    // TXDESC Dword 5
    u1Byte          dataBW;
    u1Byte          dataSC;
    u1Byte          RTSSC;
    u1Byte          dataStbc;
    u1Byte          dataLdpc;
    u1Byte          dataShort;
    u1Byte          RTSShort;
    u1Byte          TXPowerOffset;
    u1Byte          TXAnt;
	
    // TXDESC Dword 6
#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
    u1Byte          antSel;
    u1Byte          antSel_A;
    u1Byte          antSel_B;
    u1Byte          antSel_C;
#endif	//#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
	
	
#if (CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC || CFG_HAL_HW_TX_SHORTCUT_HDR_CONV)
	// TXDESC Dword 8
	BOOLEAN		 	smhEn;
	BOOLEAN			stwEn;
	BOOLEAN			stwAntDis;
	BOOLEAN			stwRateDis;
	BOOLEAN			stwRbDis;
	BOOLEAN			stwPktReDis;
	BOOLEAN			macCp;
	BOOLEAN			txwifiCp;
#endif

#if 1 //eric-8822
	u1Byte			SND_pkt_sel;
	u1Byte			is_GID;
#endif

    // encrypt
    u4Byte          iv;
    u4Byte          icv;
    u4Byte          mic;
    u4Byte          secType;
    BOOLEAN         swCrypt;

#if CFG_HAL_TX_AMSDU
    // AMSDU
    u1Byte          aggreEn;
    u4Byte          amsduLen;
#endif

} TX_DESC_DATA_88XX, *PTX_DESC_DATA_88XX;

typedef struct _H2C_PAYLOAD_88XX_
{
    u4Byte          offset0;
    u4Byte          offset4;
    u4Byte          offset8;
    u4Byte          offset12;
    u4Byte          offset16;
    u4Byte          offset20;
    u4Byte          offset24;
    u4Byte          offset28;    
} H2C_PAYLOAD_88XX, *PH2C_PAYLOAD_88XX;

VOID
TxPolling88XX(
    IN	HAL_PADAPTER        Adapter,
    IN	u1Byte              QueueIndex
);

VOID
SigninBeaconTXBD88XX
(
    IN	HAL_PADAPTER        Adapter,
    IN  pu4Byte             beaconbuf,
    IN  u2Byte              frlen
);

VOID
SetBeaconDownload88XX (
    IN	HAL_PADAPTER        Adapter,
    IN  u4Byte              Value
);

u2Byte
GetTxQueueHWIdx88XX
(
    IN	HAL_PADAPTER        Adapter,
    IN  u4Byte              q_num       //enum _TX_QUEUE_
);

BOOLEAN
FillTxHwCtrl88XX(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      PVOID           pDescData
);

HAL_IMEM
RT_STATUS
SyncSWTXBDHostIdxToHW88XX (
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex  //HCI_TX_DMA_QUEUE_88XX
);

HAL_IMEM
BOOLEAN
QueryTxConditionMatch88XX(
    IN	    HAL_PADAPTER            Adapter
);

RT_STATUS
PrepareTXBD88XX(
    IN      HAL_PADAPTER    Adapter
);

VOID
FillBeaconDesc88XX
(
    IN	HAL_PADAPTER        Adapter,
    IN  PVOID               _pdesc,
    IN  PVOID               data_content,
    IN  u2Byte              txLength,
    IN  BOOLEAN             bForceUpdate
);

VOID
FillRsrvPageDesc88XX  
(
    IN	HAL_PADAPTER        Adapter,
    IN  PVOID               _pdesc,
    IN  PVOID               data_content,
    IN  u2Byte              txLength
);

VOID
GetBeaconTXBDTXDESC88XX(
    IN	HAL_PADAPTER                Adapter,
    OUT PTX_BUFFER_DESCRIPTOR       *pTXBD,
    OUT PTX_DESC_88XX               *ptx_desc
);

VOID
FillTxDesc88XX (
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pDescData
);
						
VOID
FillShortCutTxDesc88XX(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      PVOID           pDescData,
    IN      PVOID           pTxDesc
);

VOID
FillHwShortCutTxDesc88XX (
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pDescData
);

VOID
FillTxDesc88XX_V1 (
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pDescData
);
						
VOID
FillShortCutTxDesc88XX_V1(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      PVOID           pDescData,
    IN      PVOID           pTxDesc
);

VOID
FillHwShortCutTxDesc88XX_V1 (
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pDescData
);

VOID
FillBeaconDesc88XX_V1
(
    IN	HAL_PADAPTER        Adapter,
    IN  PVOID               _pdesc,
    IN  PVOID               data_content,
    IN  u2Byte              txLength,
    IN  BOOLEAN             bForceUpdate
);

VOID
FillRsrvPageDesc88XX_V1 
(
    IN	HAL_PADAPTER        Adapter,
    IN  PVOID               _pdesc,
    IN  PVOID               data_content,
    IN  u2Byte              txLength
);

#if CFG_HAL_TX_SHORTCUT
#if 0
PVOID
GetShortCutTxDesc88XX(
    IN      HAL_PADAPTER    Adapter
);

VOID
ReleaseShortCutTxDesc88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc
);
#endif

VOID
UpdateSWTXBDHostIdx88XX (
    IN  HAL_PADAPTER                    Adapter,
    IN  PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q
);
HAL_IMEM
VOID
SetShortCutTxBuffSize88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc,
    IN  u2Byte          txPktSize
);

HAL_IMEM
u2Byte
GetShortCutTxBuffSize88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc
);

HAL_IMEM
PVOID
CopyShortCutTxDesc88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX    
    IN  PVOID           pTxDesc,
    IN  u4Byte          direction    
);

HAL_IMEM
BOOLEAN
FillShortCutTxHwCtrl88XX(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      PVOID           pDescData,
    IN      PVOID           pTxDesc,
    IN      u4Byte          direction,
    IN      BOOLEAN         useHW  
);

u2Byte
GetShortCutTxBuffSize88XX_V1(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc
);

VOID
SetShortCutTxBuffSize88XX_V1(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc,
    IN  u2Byte          txBuffSize
);



#endif // CFG_HAL_TX_SHORTCUT

void DumpTxBDesc88XX(
    IN      HAL_PADAPTER    Adapter,
#ifdef CONFIG_RTL_PROC_NEW
    IN      struct seq_file *s,
#endif
    IN      u4Byte          q_num 
);

#endif  //#ifndef __HAL88XX_TXDESC_H__


