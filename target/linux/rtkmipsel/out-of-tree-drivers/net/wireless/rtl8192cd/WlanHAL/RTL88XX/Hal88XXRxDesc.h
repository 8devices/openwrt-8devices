#ifndef __HAL88XXRXDESC_H__
#define __HAL88XXRXDESC_H__

#if defined(_PC_) || defined(_PUMA6_)
#ifdef FS
#undef FS
#endif
#endif

typedef struct _RX_DESC_88XX_
{
    volatile    u4Byte     Dword0;
    volatile    u4Byte     Dword1;
    volatile    u4Byte     Dword2;
    volatile    u4Byte     Dword3;
    volatile    u4Byte     Dword4;
    volatile    u4Byte     Dword5;
} RX_DESC_88XX, *PRX_DESC_88XX;

#define SIZE_RXDESC_88XX    24

typedef enum _HCI_RX_DMA_QUEUE_88XX_
{
    HCI_RX_DMA_QUEUE_Q0 = 0,
    HCI_RX_DMA_QUEUE_MAX_NUM
} HCI_RX_DMA_QUEUE_88XX, *PHCI_RX_DMA_QUEUE_88XX;

typedef struct _RX_BUFFER_DESCRIPTOR_ 
{
    u4Byte                  Dword0;
    u4Byte                  Dword1;
#if TXBD_SEG_32_64_SEL
    u4Byte                  Dword2;
    u4Byte                  Dword3;
#endif
} RX_BUFFER_DESCRIPTOR, *PRX_BUFFER_DESCRIPTOR;

typedef struct _HCI_RX_DMA_QUEUE_STRUCT_88XX_
{
    PRX_BUFFER_DESCRIPTOR   pRXBD_head;
#ifdef CONFIG_NET_PCI
    unsigned long rxbd_dma_addr;
#endif

    u2Byte                  hw_idx;         //Mapping to HW register
    u2Byte	                host_idx;       //Mapping to HW register 
    //Special case:                                                   
    //      If we can't allocate a new SKB(RXDESC+Payload) to mapping RXBD, 
    //      we still need to maintain a pointer in order to recieve RxPkt continuously
    u2Byte	                cur_host_idx;      // the index to indicate the next location that we have received RxPkt

#if CFG_HAL_DELAY_REFILL_RX_BUF
    u2Byte                  rxbd_ok_cnt;
#endif

    u2Byte                  total_rxbd_num;
    u2Byte                  avail_rxbd_num;
    u4Byte                  reg_rwptr_idx;
#if RXBD_READY_CHECK_METHOD
    // reset signal from 0x100[1], CR[HCI_RXDMA_EN]
    u2Byte                  rxtag_seq_num;
#endif
} HCI_RX_DMA_QUEUE_STRUCT_88XX, *PHCI_RX_DMA_QUEUE_STRUCT_88XX;

typedef struct _HCI_RX_DMA_MANAGER_88XX_
{
    HCI_RX_DMA_QUEUE_STRUCT_88XX  rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM];
} HCI_RX_DMA_MANAGER_88XX, *PHCI_RX_DMA_MANAGER_88XX;

typedef struct _RX_DESC_STATUS_88XX_
{
    // RXBD
    BOOLEAN     FS;
    BOOLEAN     LS;
    u2Byte      RXBuffSize;

#if CFG_HAL_DBG
    u2Byte      remaining_pkt_len;
    u4Byte      pktBufAddr[32];
    u2Byte      pktBufLen[32];
    u2Byte      pktNum;
#endif

    //4  Note: value below Only valid in FS=1 
    // Dword0
    u2Byte      PKT_LEN;
    BOOLEAN     CRC32;
    BOOLEAN     ICVERR;
    u1Byte      DRV_INFO_SIZE;
    u1Byte      SECURITY;
    u1Byte      SHIFT;
    BOOLEAN     PHYST;
    BOOLEAN     SWDEC;
    BOOLEAN     EOR;    

    // Dword 1
    BOOLEAN     PAGGR;

    // Dword 2
    BOOLEAN     C2HPkt;

    // Dowrd 3
    u1Byte      RX_RATE;

    // Dword 4
    BOOLEAN     OFDM_SGI;
    BOOLEAN     CCK_SPLCP;
    BOOLEAN     LDPC;
    BOOLEAN     STBC;
    BOOLEAN     NOT_SOUNDING;
    u1Byte      BW;
#if CFG_HAL_HW_FILL_MACID
    u1Byte      rxMACID;
#endif
} RX_DESC_STATUS_88XX, *PRX_DESC_STATUS_88XX;

HAL_IMEM
RT_STATUS
QueryRxDesc88XX (
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,
    IN      pu1Byte         pBufAddr,
    OUT     PVOID           pRxDescStatus
);

RT_STATUS
QueryRxDesc88XX_V1 (
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_RX_DMA_QUEUE_88XX
    IN      pu1Byte         pBufAddr,
    OUT     PVOID           pRxDescStatus
);
HAL_IMEM
RT_STATUS 
UpdateRXBDInfo88XX(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_RX_DMA_QUEUE_88XX
    IN      u2Byte          rxbd_idx,
    IN      pu1Byte         pBuf,
    IN      PVOID           Callback,    // callback function    
    IN      BOOLEAN         bInit    
);

HAL_IMEM
u2Byte	
UpdateRXBDHWIdx88XX(
    IN      HAL_PADAPTER            Adapter,
	IN      u4Byte		            queueIndex
);

HAL_IMEM
VOID
UpdateRXBDHostIdx88XX (
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      u4Byte          Count
);

RT_STATUS
PrepareRXBD88XX(
    IN      HAL_PADAPTER    Adapter,
    IN      u2Byte          bufferLen,
    IN      PVOID           Callback    // callback function
);

void DumpRxBDesc88XX(
    IN      HAL_PADAPTER    Adapter,
#ifdef CONFIG_RTL_PROC_NEW
    IN      struct seq_file *s,
#endif
    IN      u4Byte          q_num 
);

#endif  //#ifndef __HAL88XXRXDESC_H__


