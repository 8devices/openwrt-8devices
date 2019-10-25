#ifndef __HAL_COM_RXDESC_H__
#define __HAL_COM_RXDESC_H__
/*-------------------------Modification Log-----------------------------------    
-------------------------Modification Log-----------------------------------*/

/*--------------------------Include File--------------------------------------*/
#include "HalHWCfg.h"
/*--------------------------Include File--------------------------------------*/

#if (RTL8881A_SUPPORT || RTL8192E_SUPPORT || RTL8814A_SUPPORT || RTL8822B_SUPPORT)
//3 RX Buffer Descriptor
//RXBD Desc bit Mask & Shift
//Dword 0 MSK
#define RXBD_DW0_RXBUFSIZE_MSK          0x3FFF
#define RXBD_DW0_LS_MSK                 0x1
#define RXBD_DW0_FS_MSK	                0x1
#define RXBD_DW0_TOTALRXPKTSIZE_MSK     0xFFFF

//Dword 0 SHIFT
#define RXBD_DW0_RXBUFSIZE_SH           0
#define RXBD_DW0_LS_SH                  14
#define RXBD_DW0_FS_SH                  15
#define RXBD_DW0_TOTALRXPKTSIZE_SH      16


//Dword 1 MSK
#define RXBD_DW1_PHYADDR_LOW_MSK        0xFFFFFFFF

//Dword 1 SHIFT
#define RXBD_DW1_PHYADDR_LOW_SH         0


//Dword 2 MSK
#define RXBD_DW2_PHYADDR_HIGH_MSK       0xFFFFFFFF

//Dword 2 SHIFT
#define RXBD_DW2_PHYADDR_HIGH_SH        0


//Dword 3 MSK
#define RXBD_DW3_PHYADDR_RSVD_MSK       0xFFFFFFFF

//Dword 3 SHIFT
#define RXBD_DW3_PHYADDR_RSVD_SH        0


//3  RX WiFi Info
//======RX Desc bit Mask & Shift=======
//Dword0
#define RX_DW0_PKT_LEN_MSK              0x3FFF
#define RX_DW0_CRC32_MSK                0x1
#define RX_DW0_ICVERR_MSK               0x1
#define RX_DW0_DRV_INFO_SIZE_MSK        0xF
#define RX_DW0_SECURITY_MSK             0x7
#define RX_DW0_QOS_MSK                  0x1
#define RX_DW0_SHIFT_MSK                0x3
#define RX_DW0_PHYST_MSK                0x1
#define RX_DW0_SWDEC_MSK                0x1
#if (RTL8881A_SUPPORT || RTL8192E_SUPPORT)
#define RX_DW0_RSVD28_29_MSK            0x3
#endif
#if RTL8814A_SUPPORT
#define RX_DW0_PHYPKTIDC_MSK            0x1
#define RX_DW0_RSVD_29_MSK              0x1
#endif
#define RX_DW0_EOR_MSK                  0x1
#define RX_DW0_OWN_MSK                  0x1

//Dword0
#define RX_DW0_PKT_LEN_SH               0
#define RX_DW0_CRC32_SH                 14
#define RX_DW0_ICVERR_SH                15
#define RX_DW0_DRV_INFO_SIZE_SH         16
#define RX_DW0_SECURITY_SH              20
#define RX_DW0_QOS_SH                   23
#define RX_DW0_SHIFT_SH                 24
#define RX_DW0_PHYST_SH                 26
#define RX_DW0_SWDEC_SH                 27
#if (RTL8881A_SUPPORT || RTL8192E_SUPPORT)
#define RX_DW0_RSVD28_29_SH             28
#endif
#if RTL8814A_SUPPORT
#define RX_DW0_PHYPKTIDC_SH             28
#define RX_DW0_RSVD29_SH                29
#endif
#define RX_DW0_EOR_SH                   30
#define RX_DW0_RSVD31_SH                31


//Dword1
#define RX_DW1_MACID_MSK                0x7F
#if (RTL8881A_SUPPORT || RTL8192E_SUPPORT)
#define RX_DW1_RSVD7_MSK                0x1
#endif
#if RTL8814A_SUPPORT
#define RX_DW1_EXT_SECTYPE_MSK          0x1
#endif
#define RX_DW1_TID_MSK                  0xF
#define RX_DW1_MACID_VLD_MSK            0x1
#define RX_DW1_AMSDU_MSK                0x1
#define RX_DW1_RXID_MATCH_MSK           0x1
#define RX_DW1_PAGGR_MSK                0x1
#if (RTL8881A_SUPPORT || RTL8192E_SUPPORT)
#define RX_DW1_A1_FIT_MSK               0xF
#endif
#if RTL8814A_SUPPORT
#define RX_DW1_RSVD_16_19_MSK           0xF
#endif
#define RX_DW1_CHKERR_MSK               0x1
#define RX_DW1_RX_IPV_MSK               0x1
#define RX_DW1_RX_IS_TCP_UDP_MSK        0x1
#define RX_DW1_CHK_VLD_MSK              0x1
#define RX_DW1_PAM_MSK                  0x1
#define RX_DW1_PWR_MSK                  0x1
#define RX_DW1_MD_MSK                   0x1
#define RX_DW1_MF_MSK                   0x1
#define RX_DW1_TYPE_MSK                 0x3
#define RX_DW1_MC_MSK                   0x1
#define RX_DW1_BC_MSK                   0x1

//Dword1
#define RX_DW1_MACID_SH                 0

#if (RTL8881A_SUPPORT || RTL8192E_SUPPORT)
#define RX_DW1_RSVD7_SH                 7
#endif
#if RTL8814A_SUPPORT
#define RX_DW1_EXT_SECTYPE_SH           7
#endif
#define RX_DW1_TID_SH                   8
#define RX_DW1_MACID_VLD_SH             12
#define RX_DW1_AMSDU_SH                 13
#define RX_DW1_RXID_MATCH_SH            14
#define RX_DW1_PAGGR_SH                 15
#if (RTL8881A_SUPPORT || RTL8192E_SUPPORT)
#define RX_DW1_A1_FIT_SH                16
#endif

#if RTL8814A_SUPPORT
#define RX_DW1_RSVD_16_19_SH            16
#endif
#define RX_DW1_CHKERR_SH                20
#define RX_DW1_RX_IPV_SH                21
#define RX_DW1_RX_IS_TCP_UDP_SH         22
#define RX_DW1_CHK_VLD_SH               23
#define RX_DW1_PAM_SH                   24
#define RX_DW1_PWR_SH                   25
#define RX_DW1_MD_SH                    26
#define RX_DW1_MF_SH                    27
#define RX_DW1_TYPE_SH                  28
#define RX_DW1_MC_SH                    30
#define RX_DW1_BC_SH                    31


//Dword2
#define RX_DW2_SEQ_MSK                  0xFFF
#define RX_DW2_FRAG_MSK                 0xF
#define RX_DW2_RX_IS_QOS_MSK            0x1
#define RX_DW2_RSVD17_MSK               0x1
#define RX_DW2_WLANHD_IV_LEN_MSK        0x3F
#define RX_DW2_HWRSVD_MSK               0xF
#define RX_DW2_C2HPKT_MSK               0x1
#define RX_DW2_RSVD29_30_MSK            0x3
#define RX_DW2_FCS_OK_MSK               0x1

//Dword2
#define RX_DW2_SEQ_SH                   0
#define RX_DW2_FRAG_SH                  12
#define RX_DW2_RX_IS_QOS_SH             16
#define RX_DW2_RSVD17_SH                17
#define RX_DW2_WLANHD_IV_LEN_SH         18
#define RX_DW2_HWRSVD_SH                24
#define RX_DW2_C2HPKT_SH                28
#define RX_DW2_RSVD29_30_SH             29
#define RX_DW2_FCS_OK_SH                31


//Dword3
#define RX_DW3_RX_RATE_MSK              0x7F
#if (RTL8881A_SUPPORT || RTL8192E_SUPPORT)
#define RX_DW3_RSVD7_9_MSK              0x7
#endif

#if RTL8814A_SUPPORT
#define RX_DW3_BSSID_FIT_MSK            0x7
#endif

#define RX_DW3_HTC_MSK                  0x1
#define RX_DW3_EOSP_MSK                 0x1
#define RX_DW3_BSSID_FIT_MSK            0x3
#define RX_DW3_RSVD14_15_MSK            0x3
#define RX_DW3_DMA_AGG_NUM_MSK          0xFF
#define RX_DW3_RSVD24_28_MSK            0x1F
#define RX_DW3_PATTERN_MATCH_MSK        0x1
#define RX_DW3_UNICAST_MSK              0x1
#define RX_DW3_MAGIC_WAKE_MSK           0x1

//Dword3
#define RX_DW3_RX_RATE_SH               0
#if (RTL8881A_SUPPORT || RTL8192E_SUPPORT)
#define RX_DW3_RSVD7_9_SH               7
#endif

#if RTL8814A_SUPPORT
#define RX_DW3_BSSID_FIT_SH             7
#endif
#define RX_DW3_HTC_SH                   10
#define RX_DW3_EOSP_SH                  11
#define RX_DW3_BSSID_FIT_SH             12
#define RX_DW3_RSVD14_15_SH             14
#define RX_DW3_DMA_AGG_NUM_SH           16
#define RX_DW3_RSVD24_28_SH             24
#define RX_DW3_PATTERN_MATCH_SH         29
#define RX_DW3_UNICAST_SH               30
#define RX_DW3_MAGIC_WAKE_SH            31


//Dword4
#if (RTL8881A_SUPPORT || RTL8192E_SUPPORT)
#define RX_DW4_OFDM_SGI_MSK             0x1
#define RX_DW4_CCK_SPLCP_MSK            0x1
#define RX_DW4_LDPC_MSK                 0x1
#define RX_DW4_STBC_MSK                 0x1
#define RX_DW4_NOT_SOUNDING_MSK         0x1
#define RX_DW4_BW_MSK                   0x3
#define RX_DW4_RSVD6_31_MSK             0x3FFFFFF
#endif

#if RTL8814A_SUPPORT
#define RX_DW4_PATTERN_IDX_MSK          0xFF
#define RX_DW4_RXEOF_MSK                0x1
#define RX_DW4_RX_SCRMBLER_MSK          0x7F
#define RX_DW4_RX_PRE_NDP_VLD_MSK       0x1
#define RX_DW4_MACID_MSK                0x7F
#define RX_DW4_A1_FIT_MSK               0x1F
#define RX_DW4_RSVD29_31_MSK            0x7
#endif

//Dword4
#if (RTL8881A_SUPPORT || RTL8192E_SUPPORT)
#define RX_DW4_OFDM_SGI_SH              0
#define RX_DW4_CCK_SPLCP_SH             0
#define RX_DW4_LDPC_SH                  1
#define RX_DW4_STBC_SH                  2
#define RX_DW4_NOT_SOUNDING_SH          3
#define RX_DW4_BW_SH                    4
#define RX_DW4_RSVD6_31_SH              6
#endif

#if RTL8814A_SUPPORT
#define RX_DW4_PATTERN_IDX_SH           0
#define RX_DW4_RXEOF_SH                 8
#define RX_DW4_RX_SCRMBLER_SH           9
#define RX_DW4_RX_PRE_NDP_VLD_SH        16
#define RX_DW4_MACID_SH                 17
#define RX_DW4_A1_FIT_SH                24
#define RX_DW4_RSVD29_31_SH             29
#endif


//Dword5
#define RX_DW5_TSFL_MSK                 0xFFFFFFFF

//Dword5
#define RX_DW5_TSFL_SH                  0


#endif	// (RTL8881A_SUPPORT || RTL8192E_SUPPORT || RTL8814A_SUPPORT)

#if (RTL8197F_SUPPORT)

//3 RX Buffer Descriptor
//RXBD Desc bit Mask & Shift
//Dword 0 MSK
#define RXBD_DW0_RXBUFSIZE_MSK          0x3FFF
#define RXBD_DW0_LS_MSK                 0x1
#define RXBD_DW0_FS_MSK	                0x1
#define RXBD_DW0_TOTALRXPKTSIZE_MSK     0xFFFF

//Dword 0 SHIFT
#define RXBD_DW0_RXBUFSIZE_SH           0
#define RXBD_DW0_LS_SH                  14
#define RXBD_DW0_FS_SH                  15
#define RXBD_DW0_TOTALRXPKTSIZE_SH      16


//Dword 1 MSK
#define RXBD_DW1_PHYADDR_LOW_MSK        0xFFFFFFFF

//Dword 1 SHIFT
#define RXBD_DW1_PHYADDR_LOW_SH         0


//Dword 2 MSK
#define RXBD_DW2_PHYADDR_HIGH_MSK       0xFFFFFFFF

//Dword 2 SHIFT
#define RXBD_DW2_PHYADDR_HIGH_SH        0


//Dword 3 MSK
#define RXBD_DW3_PHYADDR_RSVD_MSK       0xFFFFFFFF

//Dword 3 SHIFT
#define RXBD_DW3_PHYADDR_RSVD_SH        0

#endif 


#endif  //__HAL_COM_RXDESC_H__
