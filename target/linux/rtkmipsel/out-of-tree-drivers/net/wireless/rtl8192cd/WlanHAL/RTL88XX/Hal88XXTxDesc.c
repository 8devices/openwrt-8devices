/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXTxDesc.c

Abstract:
	Defined RTL88XX HAL tx desc common function

Major Change History:
	When            Who                         What
	---------- ---------------   -------------------------------
	2012-03-29  Lun-Wu Yeh            Add PrepareTxDesc88XX().
--*/

#if !defined(__ECOS) && !defined(CPTCFG_CFG80211_MODULE)
#include "HalPrecomp.h"
#else
#include "../HalPrecomp.h"
#endif
#if defined(__ECOS)
#include "halmac_api.h"
#endif

#ifdef __ECOS
#undef printk
#define printk	ecos_pr_fun
//typedef void pr_fun(char *fmt, ...);
extern pr_fun *ecos_pr_fun;
#endif
#ifdef TRXBD_CACHABLE_REGION
#ifndef __ECOS
#include <asm/cacheflush.h>
#endif
#endif

#ifdef CONFIG_RTL_PROC_NEW
#define PROC_PRINT(fmt, arg...)	seq_printf(s, fmt, ## arg)
#else
#define PROC_PRINT	printk
#endif


#if (HAL_DEV_BUS_TYPE & (HAL_RT_EMBEDDED_INTERFACE | HAL_RT_PCI_INTERFACE))
void DumpTxBDesc88XX(
    IN      HAL_PADAPTER    Adapter,
#ifdef CONFIG_RTL_PROC_NEW
    IN      struct seq_file *s,
#endif
    IN      u4Byte          q_num 
)
{
#if IS_EXIST_RTL8881AEM
    u4Byte TXBD_NUM_8881A[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        TX_MGQ_TXBD_NUM,
        TX_BKQ_TXBD_NUM, TX_BEQ_TXBD_NUM, TX_VIQ_TXBD_NUM, TX_VOQ_TXBD_NUM,
        TX_HI0Q_TXBD_NUM, TX_HI1Q_TXBD_NUM, TX_HI2Q_TXBD_NUM, TX_HI3Q_TXBD_NUM,
        TX_HI4Q_TXBD_NUM, TX_HI5Q_TXBD_NUM, TX_HI6Q_TXBD_NUM, TX_HI7Q_TXBD_NUM,
        TX_BCNQ_TXBD_NUM_V2
    };
#endif  //IS_EXIST_RTL8881AEM

#if (IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM || IS_EXIST_RTL8822BE)
    u4Byte TXBD_NUM_V1[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        TX_MGQ_TXBD_NUM,
        TX_BKQ_TXBD_NUM, TX_BEQ_TXBD_NUM, TX_VIQ_TXBD_NUM, TX_VOQ_TXBD_NUM,
        TX_HI0Q_TXBD_NUM, TX_HI1Q_TXBD_NUM, TX_HI2Q_TXBD_NUM, TX_HI3Q_TXBD_NUM,
        TX_HI4Q_TXBD_NUM, TX_HI5Q_TXBD_NUM, TX_HI6Q_TXBD_NUM, TX_HI7Q_TXBD_NUM,
        TX_BCNQ_TXBD_NUM_V1
    };
#endif // #if (IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM || IS_EXIST_RTL8822BE)

#if 0
#if IS_EXIST_RTL8192EE
    u4Byte TXBD_NUM_8192E[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        TX_MGQ_TXBD_NUM,
        TX_BKQ_TXBD_NUM, TX_BEQ_TXBD_NUM, TX_VIQ_TXBD_NUM, TX_VOQ_TXBD_NUM,
        TX_HI0Q_TXBD_NUM, TX_HI1Q_TXBD_NUM, TX_HI2Q_TXBD_NUM, TX_HI3Q_TXBD_NUM,
        TX_HI4Q_TXBD_NUM, TX_HI5Q_TXBD_NUM, TX_HI6Q_TXBD_NUM, TX_HI7Q_TXBD_NUM,
        TX_BCNQ_TXBD_NUM_V1
    };
#endif  //IS_EXIST_RTL8192EE

#if IS_EXIST_RTL8814AE
    u4Byte TXBD_NUM_8814AE[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        TX_MGQ_TXBD_NUM,
        TX_BKQ_TXBD_NUM, TX_BEQ_TXBD_NUM, TX_VIQ_TXBD_NUM, TX_VOQ_TXBD_NUM,
        TX_HI0Q_TXBD_NUM, TX_HI1Q_TXBD_NUM, TX_HI2Q_TXBD_NUM, TX_HI3Q_TXBD_NUM,
        TX_HI4Q_TXBD_NUM, TX_HI5Q_TXBD_NUM, TX_HI6Q_TXBD_NUM, TX_HI7Q_TXBD_NUM,
        TX_BCNQ_TXBD_NUM_V1
    };
#endif  //IS_EXIST_RTL8814AE

#endif 


    // TXBD_RWPtr_Reg: no entry for beacon queue, set NULL here
    u4Byte  TXBD_RWPtr_Reg[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        REG_MGQ_TXBD_IDX,
        REG_BKQ_TXBD_IDX, REG_BEQ_TXBD_IDX, REG_VIQ_TXBD_IDX, REG_VOQ_TXBD_IDX,
        REG_HI0Q_TXBD_IDX, REG_HI1Q_TXBD_IDX, REG_HI2Q_TXBD_IDX, REG_HI3Q_TXBD_IDX,
        REG_HI4Q_TXBD_IDX, REG_HI5Q_TXBD_IDX, REG_HI6Q_TXBD_IDX, REG_HI7Q_TXBD_IDX,        
        0
    };

    u4Byte  TXBD_Reg[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        REG_MGQ_TXBD_DESA,
        REG_BKQ_TXBD_DESA, REG_BEQ_TXBD_DESA, REG_VIQ_TXBD_DESA, REG_VOQ_TXBD_DESA,
        REG_HI0Q_TXBD_DESA, REG_HI1Q_TXBD_DESA, REG_HI2Q_TXBD_DESA, REG_HI3Q_TXBD_DESA,
        REG_HI4Q_TXBD_DESA, REG_HI5Q_TXBD_DESA, REG_HI6Q_TXBD_DESA, REG_HI7Q_TXBD_DESA,
        REG_BCNQ_TXBD_DESA
    };
	PHCI_TX_DMA_MANAGER_88XX    ptx_dma         = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);;
	int                         i               = 0;
	PTX_DESC_88XX               ptx_desc_head   = ptx_dma->tx_queue[q_num].ptx_desc_head;
	PTX_BUFFER_DESCRIPTOR       ptxbd = ptx_dma->tx_queue[q_num].pTXBD_head;
    pu4Byte                     pTXBD_NUM;

#ifdef NOT_RTK_BSP
	if ((NULL == ptx_desc_head) || (NULL == ptxbd))
		return;
#endif

#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881A(Adapter) ) {
        pTXBD_NUM           = TXBD_NUM_8881A;
    }
#endif  //IS_EXIST_RTL8881AEM

#if (IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM || IS_EXIST_RTL8822BE)
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) || IS_HARDWARE_TYPE_8814AE(Adapter)|| IS_HARDWARE_TYPE_8197F(Adapter)|| IS_HARDWARE_TYPE_8822B(Adapter)) {
        pTXBD_NUM           = TXBD_NUM_V1;
    }
#endif //#if (IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM || IS_EXIST_RTL8822BE)

#if 0

#if IS_EXIST_RTL8192EE
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) ) {
        pTXBD_NUM           = TXBD_NUM_8192E;
    }
#endif  //IS_EXIST_RTL8192EE    
		
#if IS_EXIST_RTL8814AE
    if ( IS_HARDWARE_TYPE_8814AE(Adapter) ) {
        pTXBD_NUM           = TXBD_NUM_8814AE;
    }
#endif  //IS_EXIST_RTL8814AE 

#endif 		
	PROC_PRINT("q_num:%d, hw_idx/host_idx= %d/%d\n", q_num, ptx_dma->tx_queue[q_num].hw_idx, ptx_dma->tx_queue[q_num].host_idx);

	PROC_PRINT("total_txbd_num=%d,avail_txbd_num= %d,reg_rwptr_idx:%x\n",
		ptx_dma->tx_queue[q_num].total_txbd_num, ptx_dma->tx_queue[q_num].avail_txbd_num, ptx_dma->tx_queue[q_num].reg_rwptr_idx);

	PROC_PRINT("RWreg(%x):%08x\n", TXBD_RWPtr_Reg[q_num], HAL_RTL_R32(TXBD_RWPtr_Reg[q_num]));

#ifdef CONFIG_NET_PCI
	if (HAL_IS_PCIBIOS_TYPE(Adapter)) {
		PROC_PRINT("pTXBD_head=%p, %08lx, reg(%x):%08x\n",
			ptx_dma->tx_queue[q_num].pTXBD_head , 
			_GET_HAL_DATA(Adapter)->txBD_dma_ring_addr[q_num],
			TXBD_Reg[q_num], HAL_RTL_R32(TXBD_Reg[q_num]));

		for (i=0;i<pTXBD_NUM[q_num];i++ ){
			PROC_PRINT("ptxbd[%d], addr:%p,%08lx\n[0] 0x%08x 0x%08x [1] 0x%08x 0x%08x\n",
				 i,
				&ptxbd[i],
				_GET_HAL_DATA(Adapter)->txBD_dma_ring_addr[q_num] + i * sizeof(TX_BUFFER_DESCRIPTOR),
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[0].Dword0), 
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[0].Dword1),
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[1].Dword0),
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[1].Dword1)  );
			PROC_PRINT("[2] 0x%08x 0x%08x [3] 0x%08x 0x%08x\n",
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[2].Dword0), 
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[2].Dword1),
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[3].Dword0),
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[3].Dword1)  );

			PROC_PRINT("ptx_desc_head[%d], addr:%p,%08lx\n",
				i,
				&ptx_desc_head[i],
				_GET_HAL_DATA(Adapter)->txDesc_dma_ring_addr[q_num] + i * sizeof(TX_DESC_88XX));
			PROC_PRINT("%08x %08x %08x %08x ",  (u4Byte)GET_DESC(ptx_desc_head[i].Dword0),  (u4Byte)GET_DESC(ptx_desc_head[i].Dword1),  (u4Byte)GET_DESC(ptx_desc_head[i].Dword2),  (u4Byte)GET_DESC(ptx_desc_head[i].Dword3));
			PROC_PRINT("%08x %08x %08x %08x ",  (u4Byte)GET_DESC(ptx_desc_head[i].Dword4), (u4Byte)GET_DESC( ptx_desc_head[i].Dword5),  (u4Byte)GET_DESC(ptx_desc_head[i].Dword6),  (u4Byte)GET_DESC(ptx_desc_head[i].Dword7));
			PROC_PRINT("%08x %08x\n", (u4Byte)GET_DESC(ptx_desc_head[i].Dword8),  (u4Byte)GET_DESC(ptx_desc_head[i].Dword9));
		}
	}else 
#endif
	{
		PROC_PRINT("pTXBD_head=%p, %08lx, reg(%x):%08x\n",
			ptx_dma->tx_queue[q_num].pTXBD_head , 
			HAL_VIRT_TO_BUS1(Adapter, (PVOID)ptx_dma->tx_queue[q_num].pTXBD_head,sizeof(TX_BUFFER_DESCRIPTOR) * pTXBD_NUM[q_num], HAL_PCI_DMA_TODEVICE),
			TXBD_Reg[q_num], HAL_RTL_R32(TXBD_Reg[q_num]));

	for (i=0;i<pTXBD_NUM[q_num];i++ ){
		PROC_PRINT("ptxbd[%d], addr:%08x,%08x: Dword0: 0x%x, Dword1: 0x%x, [1] 0x%x 0x%x,\n",
                    i,
                    (u4Byte)&ptxbd[i],
                    (u4Byte)HAL_VIRT_TO_BUS1(Adapter, (PVOID)&ptxbd[i],sizeof(TX_BUFFER_DESCRIPTOR), HAL_PCI_DMA_TODEVICE),
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[0].Dword0), 
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[0].Dword1),
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[1].Dword0),
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[1].Dword1)  );
		PROC_PRINT("[2]: Dword0: 0x%x, Dword1: 0x%x, [3] 0x%x 0x%x,\n",
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[2].Dword0), 
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[2].Dword1),
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[3].Dword0),
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[3].Dword1)  );

		PROC_PRINT("ptx_desc_head[%03d],",  i  );
		PROC_PRINT("%08x %08x %08x %08x ",  
				(u4Byte)GET_DESC(ptx_desc_head[i].Dword0),  
				(u4Byte)GET_DESC(ptx_desc_head[i].Dword1), 
				(u4Byte)GET_DESC(ptx_desc_head[i].Dword2),  
				(u4Byte)GET_DESC(ptx_desc_head[i].Dword3));
		PROC_PRINT("%08x %08x %08x %08x ",  
				(u4Byte)GET_DESC(ptx_desc_head[i].Dword4),  
				(u4Byte)GET_DESC(ptx_desc_head[i].Dword5),  
				(u4Byte)GET_DESC(ptx_desc_head[i].Dword6),  
				(u4Byte)GET_DESC(ptx_desc_head[i].Dword7));
		PROC_PRINT("%08x %08x\n", 
				(u4Byte)GET_DESC(ptx_desc_head[i].Dword8),  
				(u4Byte)GET_DESC(ptx_desc_head[i].Dword9));
	}
	}
	PROC_PRINT("\n");
}

#ifdef __ECOS
#undef printk
#endif

// TODO: 
//Note: PrepareTXBD88XX is necessary to be done after calling PrepareRXBD88XX
RT_STATUS
PrepareTXBD88XX(
    IN      HAL_PADAPTER Adapter
)
{    
    PHCI_RX_DMA_MANAGER_88XX    prx_dma;
    PHCI_TX_DMA_MANAGER_88XX    ptx_dma;
    PTX_BUFFER_DESCRIPTOR       ptxbd_head;
    PTX_DESC_88XX               ptx_desc_head;
    PTX_BUFFER_DESCRIPTOR       ptxbd;
    
    PTX_BUFFER_DESCRIPTOR       ptxbd_bcn_head;
    PTX_DESC_88XX               ptxdesc_bcn_head;
    PTX_BUFFER_DESCRIPTOR       ptxbd_bcn_cur;    

    PTX_DESC_88XX               ptx_desc;

    
    HCI_TX_DMA_QUEUE_88XX       q_num;
    pu4Byte                     pTXBD_NUM;
    u4Byte                      i;
    u4Byte                      beacon_offset;
    u4Byte                      TotalTXBDNum_NoBcn;
    u4Byte                      TXDESCSize;
    u4Byte                      HCI_TX_DMA_QUEUE_MAX;
    u4Byte                      TXBD_RWPtr_Reg_CMDQ;
    u4Byte                      TXBD_Reg_CMDQ;    

    //HCI_TX_DMA_QUEUE_MAX = HCI_TX_DMA_QUEUE_MAX_NUM;
        
#if CFG_HAL_TX_AMSDU
    pu1Byte                         pdesc_dma_buf_amsdu, desc_dma_buf_start_amsdu;
    PHCI_TX_AMSDU_DMA_MANAGER_88XX  ptx_dma_amsdu;
    PTX_BUFFER_DESCRIPTOR_AMSDU     ptxbd_head_amsdu;
#endif

#ifdef WLAN_SUPPORT_H2C_PACKET
    PH2C_PAYLOAD_88XX   h2c_buf_start = (PH2C_PAYLOAD_88XX)_GET_HAL_DATA(Adapter)->h2c_buf;
    //pu1Byte  h2c_buf_start = _GET_HAL_DATA(Adapter)->h2c_buf;
    PTX_DESC_88XX               ph2c_tx_desc;
#endif //WLAN_SUPPORT_H2C_PACKET


#if IS_EXIST_RTL8881AEM
    u4Byte TXBD_NUM_8881A[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        TX_MGQ_TXBD_NUM,
        TX_BKQ_TXBD_NUM, TX_BEQ_TXBD_NUM, TX_VIQ_TXBD_NUM, TX_VOQ_TXBD_NUM,
        TX_HI0Q_TXBD_NUM, TX_HI1Q_TXBD_NUM, TX_HI2Q_TXBD_NUM, TX_HI3Q_TXBD_NUM,
        TX_HI4Q_TXBD_NUM, TX_HI5Q_TXBD_NUM, TX_HI6Q_TXBD_NUM, TX_HI7Q_TXBD_NUM,
        TX_CMDQ_TXBD_NUM, TX_BCNQ_TXBD_NUM_V2
    };
#endif  //IS_EXIST_RTL8881AEM

#if (IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM || IS_EXIST_RTL8822BE)
    u4Byte TXBD_NUM_V1[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        TX_MGQ_TXBD_NUM,
        TX_BKQ_TXBD_NUM, TX_BEQ_TXBD_NUM, TX_VIQ_TXBD_NUM, TX_VOQ_TXBD_NUM,
        TX_HI0Q_TXBD_NUM, TX_HI1Q_TXBD_NUM, TX_HI2Q_TXBD_NUM, TX_HI3Q_TXBD_NUM,
        TX_HI4Q_TXBD_NUM, TX_HI5Q_TXBD_NUM, TX_HI6Q_TXBD_NUM, TX_HI7Q_TXBD_NUM,
        TX_CMDQ_TXBD_NUM, TX_BCNQ_TXBD_NUM_V1
    };
#endif  //#if (IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM || IS_EXIST_RTL8822BE)

    // setting CMDQ register by chip
#if (IS_EXIST_RTL8881AEM || IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE)
        if ( IS_HARDWARE_TYPE_8881A(Adapter) || IS_HARDWARE_TYPE_8192EE(Adapter) || IS_HARDWARE_TYPE_8814AE(Adapter)) {
            TXBD_RWPtr_Reg_CMDQ = 0;
            TXBD_Reg_CMDQ       = 0;
        }
#endif  //#if (IS_EXIST_RTL8881AEM || IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM)
    
#if IS_EXIST_RTL8822BE || IS_EXIST_RTL8197FEM
        if ( IS_HARDWARE_TYPE_8822B(Adapter) || IS_HARDWARE_TYPE_8197F(Adapter))  {
            TXBD_RWPtr_Reg_CMDQ = REG_H2CQ_TXBD_IDX;
            TXBD_Reg_CMDQ       = REG_H2CQ_TXBD_DESA;
        }
#endif //IS_EXIST_RTL8822BE


    // TXBD_RWPtr_Reg: no entry for beacon queue, set NULL here
    u4Byte  TXBD_RWPtr_Reg[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        REG_MGQ_TXBD_IDX,
        REG_BKQ_TXBD_IDX, REG_BEQ_TXBD_IDX, REG_VIQ_TXBD_IDX, REG_VOQ_TXBD_IDX,
        REG_HI0Q_TXBD_IDX, REG_HI1Q_TXBD_IDX, REG_HI2Q_TXBD_IDX, REG_HI3Q_TXBD_IDX,
        REG_HI4Q_TXBD_IDX, REG_HI5Q_TXBD_IDX, REG_HI6Q_TXBD_IDX, REG_HI7Q_TXBD_IDX,        
        TXBD_RWPtr_Reg_CMDQ,0
    };

    u4Byte  TXBD_Reg[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        REG_MGQ_TXBD_DESA,
        REG_BKQ_TXBD_DESA, REG_BEQ_TXBD_DESA, REG_VIQ_TXBD_DESA, REG_VOQ_TXBD_DESA,
        REG_HI0Q_TXBD_DESA, REG_HI1Q_TXBD_DESA, REG_HI2Q_TXBD_DESA, REG_HI3Q_TXBD_DESA,
        REG_HI4Q_TXBD_DESA, REG_HI5Q_TXBD_DESA, REG_HI6Q_TXBD_DESA, REG_HI7Q_TXBD_DESA,
        TXBD_Reg_CMDQ,REG_BCNQ_TXBD_DESA
    };

#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881A(Adapter) ) {
        pTXBD_NUM           = TXBD_NUM_8881A;
        TotalTXBDNum_NoBcn  = TOTAL_NUM_TXBD_NO_BCN;
    }
#endif  //IS_EXIST_RTL8881AEM

#if (IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM || IS_EXIST_RTL8822BE)
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) || IS_HARDWARE_TYPE_8814AE(Adapter) ||  IS_HARDWARE_TYPE_8197F(Adapter) ||IS_HARDWARE_TYPE_8822B(Adapter)  ) {
        pTXBD_NUM           = TXBD_NUM_V1;
        TotalTXBDNum_NoBcn  = TOTAL_NUM_TXBD_NO_BCN;        
    }
#endif  //(IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM || IS_EXIST_RTL8822BE)

// setting TXDESC size by chip
#if (IS_EXIST_RTL8881AEM || IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM)
    if ( IS_HARDWARE_TYPE_8881A(Adapter) || IS_HARDWARE_TYPE_8192EE(Adapter) || IS_HARDWARE_TYPE_8814AE(Adapter) || IS_HARDWARE_TYPE_8197F(Adapter) ) {
        TXDESCSize = SIZE_TXDESC_88XX;
    }
#endif  //#if (IS_EXIST_RTL8881AEM || IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM)

#if IS_EXIST_RTL8822BE
    if ( IS_HARDWARE_TYPE_8822B(Adapter))  {
        TXDESCSize = SIZE_TXDESC_88XX_V1;
    }
#endif //IS_EXIST_RTL8822BE



#if 0
#if IS_EXIST_RTL8192EE
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) ) {
        pTXBD_NUM           = TXBD_NUM_8192E;
        TotalTXBDNum_NoBcn  = TOTAL_NUM_TXBD_NO_BCN;        
    }
#endif  //IS_EXIST_RTL8192EE

#if IS_EXIST_RTL8814AE
    if ( IS_HARDWARE_TYPE_8814AE(Adapter) ) {
        pTXBD_NUM           = TXBD_NUM_8814AE;
        TotalTXBDNum_NoBcn  = TOTAL_NUM_TXBD_NO_BCN;        
    }
#endif  //IS_EXIST_RTL8814AE

#if IS_EXIST_RTL8197FEM
    if ( IS_HARDWARE_TYPE_8197F(Adapter) ) {
        pTXBD_NUM           = TXBD_NUM_8814AE;
        TotalTXBDNum_NoBcn  = TOTAL_NUM_TXBD_NO_BCN;        
    }
#endif  //IS_EXIST_RTL8814AE

#if IS_EXIST_RTL8822BE
    if ( IS_HARDWARE_TYPE_8822(Adapter) ) {
        pTXBD_NUM           = TXBD_NUM_8814AE;
        TotalTXBDNum_NoBcn  = TOTAL_NUM_TXBD_NO_BCN;        
    }
#endif  //IS_EXIST_RTL8814AE
#endif // if 0

    prx_dma         = (PHCI_RX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PRxDMA88XX);
    ptx_dma         = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);

#if CFG_HAL_TX_AMSDU
    ptx_dma_amsdu   = (PHCI_TX_AMSDU_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMAAMSDU88XX);
#endif

#ifdef CONFIG_NET_PCI
    unsigned long tmp_tx_dma_ring_addr =0, tmp_tx_dma_ring_addr2=0;
    unsigned long tmp_tx_dma_ring_addr3 =0, tmp_tx_dma_ring_addr4=0;

    if (!HAL_IS_PCIBIOS_TYPE(Adapter))
        goto original;

    //No Beacon
    printk("head:%p, ring_dma_addr:%08lx, size:%x\n", prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].pRXBD_head, 
        _GET_HAL_DATA(Adapter)->ring_dma_addr, prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].total_rxbd_num * sizeof(RX_BUFFER_DESCRIPTOR));
        tmp_tx_dma_ring_addr4 = (unsigned long)prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].pRXBD_head +  prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].total_rxbd_num * sizeof(RX_BUFFER_DESCRIPTOR);
    ptxbd_head  = (PTX_BUFFER_DESCRIPTOR)( tmp_tx_dma_ring_addr4);

    for (i=0;i<HCI_RX_DMA_QUEUE_MAX_NUM;i++){
        tmp_tx_dma_ring_addr = _GET_HAL_DATA(Adapter)->ring_dma_addr + prx_dma->rx_queue[i].total_rxbd_num * sizeof(RX_BUFFER_DESCRIPTOR);
    }


    printk("ptxbd_head:%p, tmp_tx_dma_ring_addr:%08lx\n", ptxbd_head, tmp_tx_dma_ring_addr);
    tmp_tx_dma_ring_addr4 =0;
    //No Beacon
    // TODO: need to bug fix
    ptx_desc_head   = (PTX_DESC_88XX)((pu1Byte)ptxbd_head + \
                        sizeof(TX_BUFFER_DESCRIPTOR) * TotalTXBDNum_NoBcn);

    tmp_tx_dma_ring_addr2 = tmp_tx_dma_ring_addr + sizeof(TX_BUFFER_DESCRIPTOR) * TotalTXBDNum_NoBcn;
    printk("ptx_desc_head:%p, tmp_tx_dma_ring_addr2:%08lx, size: %x, %x\n", ptx_desc_head, tmp_tx_dma_ring_addr2, sizeof(TX_BUFFER_DESCRIPTOR) , sizeof(TX_DESC_88XX) );
    ptxbd_bcn_head  = (PTX_BUFFER_DESCRIPTOR)((pu1Byte)ptx_desc_head + \
                        sizeof(TX_DESC_88XX) * TotalTXBDNum_NoBcn);

    tmp_tx_dma_ring_addr3 = tmp_tx_dma_ring_addr2 + sizeof(TX_DESC_88XX) * TotalTXBDNum_NoBcn;
 printk("ptxbd_bcn_head:%p, tmp_tx_dma_ring_addr3:%08lx,\n", ptxbd_bcn_head, tmp_tx_dma_ring_addr3);
#if IS_RTL8881A_SERIES
    if (IS_HARDWARE_TYPE_8881A(Adapter)) {
        ptxdesc_bcn_head = (PTX_DESC_88XX)((pu1Byte)ptxbd_bcn_head + \
                                (1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_V2);
        beacon_offset = TXBD_BEACON_OFFSET_V2;
        tmp_tx_dma_ring_addr4 = tmp_tx_dma_ring_addr3+(1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_V2;
    }
#endif
#if IS_RTL8192E_SERIES
    if (IS_HARDWARE_TYPE_8192E(Adapter)) {
        ptxdesc_bcn_head = (PTX_DESC_88XX)((pu1Byte)ptxbd_bcn_head + \
                                (1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_V1);
        beacon_offset = TXBD_BEACON_OFFSET_V1;
        tmp_tx_dma_ring_addr4 = tmp_tx_dma_ring_addr3+(1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_V1;
    }
#endif
#if IS_RTL8814A_SERIES
    if (IS_HARDWARE_TYPE_8814A(Adapter)) {
        ptxdesc_bcn_head = (PTX_DESC_88XX)((pu1Byte)ptxbd_bcn_head + \
                                (1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_V1);
        beacon_offset = TXBD_BEACON_OFFSET_V1;
        tmp_tx_dma_ring_addr4 = tmp_tx_dma_ring_addr3+(1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_V1;
    }
#endif

#if IS_RTL8197F_SERIES
    if (IS_HARDWARE_TYPE_8197F(Adapter)) {
        ptxdesc_bcn_head = (PTX_DESC_88XX)((pu1Byte)ptxbd_bcn_head + \
                                (1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_8197F);
        beacon_offset = TXBD_BEACON_OFFSET_8197F;
        tmp_tx_dma_ring_addr4 = tmp_tx_dma_ring_addr3+(1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_8197F;
    }
#endif

#if IS_RTL8822B_SERIES
	if (IS_HARDWARE_TYPE_8822B(Adapter)) {
		ptxdesc_bcn_head = (PTX_DESC_88XX)((pu1Byte)ptxbd_bcn_head + \
				(1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_V1);
	beacon_offset = TXBD_BEACON_OFFSET_V1;
	tmp_tx_dma_ring_addr4 = tmp_tx_dma_ring_addr3+(1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_V1;
	}
#endif
printk("ptxdesc_bcn_head:%p, tmp_tx_dma_ring_addr4:%08lx, \n", ptxdesc_bcn_head, tmp_tx_dma_ring_addr4);
    // initiate all tx queue data structures 
    for (q_num = 0; q_num < HCI_TX_DMA_QUEUE_MAX_NUM; q_num++)
    {
        ptx_dma->tx_queue[q_num].hw_idx         = 0;
        ptx_dma->tx_queue[q_num].host_idx       = 0;
        ptx_dma->tx_queue[q_num].total_txbd_num = pTXBD_NUM[q_num];
        ptx_dma->tx_queue[q_num].avail_txbd_num = pTXBD_NUM[q_num];
        ptx_dma->tx_queue[q_num].reg_rwptr_idx  = TXBD_RWPtr_Reg[q_num];

        if ( 0 == q_num ) {
            ptx_dma->tx_queue[q_num].pTXBD_head    = ptxbd_head;
            ptx_dma->tx_queue[q_num].ptx_desc_head = ptx_desc_head;
        }
        else {
            if ( HCI_TX_DMA_QUEUE_BCN != q_num ) {
                ptx_dma->tx_queue[q_num].pTXBD_head    = (PTX_BUFFER_DESCRIPTOR)((pu1Byte)ptx_dma->tx_queue[q_num-1].pTXBD_head + sizeof(TX_BUFFER_DESCRIPTOR) * pTXBD_NUM[q_num-1]);
                ptx_dma->tx_queue[q_num].ptx_desc_head = (PTX_DESC_88XX)((pu1Byte)ptx_dma->tx_queue[q_num-1].ptx_desc_head + sizeof(TX_DESC_88XX)*pTXBD_NUM[q_num-1]);

                tmp_tx_dma_ring_addr += sizeof(TX_BUFFER_DESCRIPTOR) *pTXBD_NUM[q_num-1] ;
                tmp_tx_dma_ring_addr2 += sizeof(TX_DESC_88XX) *pTXBD_NUM[q_num-1] ;
            }
            else {
                ptx_dma->tx_queue[q_num].pTXBD_head    = ptxbd_bcn_head;
                ptx_dma->tx_queue[q_num].ptx_desc_head = ptxdesc_bcn_head;
                tmp_tx_dma_ring_addr = tmp_tx_dma_ring_addr3;
                tmp_tx_dma_ring_addr2 = tmp_tx_dma_ring_addr4;
            }
        }
        
        ptxbd    = ptx_dma->tx_queue[q_num].pTXBD_head;
        ptx_desc = ptx_dma->tx_queue[q_num].ptx_desc_head;
	
		if(TXBD_RWPtr_Reg[q_num])
	        HAL_RTL_W32(TXBD_RWPtr_Reg[q_num], 0);
        HAL_RTL_W32(TXBD_Reg[q_num], tmp_tx_dma_ring_addr);
        _GET_HAL_DATA(Adapter)->txBD_dma_ring_addr[q_num] = tmp_tx_dma_ring_addr;
        _GET_HAL_DATA(Adapter)->txDesc_dma_ring_addr[q_num] = tmp_tx_dma_ring_addr2;
    
printk("%s(%d), q_num:%d, TXBD_RWPtr_Reg:0x%x, TXBD_Reg:0x%x, ptxbd:%p %08lx, ptx_desc:%p %08lx\n", __FUNCTION__, __LINE__, q_num, TXBD_RWPtr_Reg[q_num], TXBD_Reg[q_num], ptxbd, tmp_tx_dma_ring_addr, ptx_desc, tmp_tx_dma_ring_addr2);

        // assign LowAddress and TxDescLength to each TXBD element
        if (q_num != HCI_TX_DMA_QUEUE_BCN) {
            for(i = 0; i < pTXBD_NUM[q_num]; i++)        
            {   
                SET_DESC_FIELD_CLR(ptxbd[i].TXBD_ELE[0].Dword0, sizeof(TX_DESC_88XX), TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
                SET_DESC_FIELD_CLR(ptxbd[i].TXBD_ELE[0].Dword1, tmp_tx_dma_ring_addr2 + sizeof(TX_DESC_88XX)*i,TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);
            }
        } else {
            // beacon...
            for (i = 0; i < 1+HAL_NUM_VWLAN; i++)
            {
                ptxbd_bcn_cur = (pu1Byte)ptxbd + beacon_offset * i;
                SET_DESC_FIELD_CLR(ptxbd_bcn_cur->TXBD_ELE[0].Dword0, \
                        sizeof(TX_DESC_88XX), \
                        TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
               
               SET_DESC_FIELD_CLR(ptxbd_bcn_cur->TXBD_ELE[0].Dword1, tmp_tx_dma_ring_addr2 + sizeof(TX_DESC_88XX)*i, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);
#if 1
                printk ("ptxbd_bcn[%d]: 0x%x, Dword0: 0x%x, Dword1: 0x%x \n", \
                                                i, (u4Byte)ptxbd_bcn_cur, \
                                                (u4Byte)GET_DESC(ptxbd_bcn_cur->TXBD_ELE[0].Dword0), \
                                                (u4Byte)GET_DESC(ptxbd_bcn_cur->TXBD_ELE[0].Dword1)
                                                );             
#endif
            }
        }        
    }   

    return RT_STATUS_SUCCESS;
#endif

original:

#if CFG_HAL_TX_AMSDU
    if ( IS_SUPPORT_TX_AMSDU(Adapter) ) {
        desc_dma_buf_start_amsdu = _GET_HAL_DATA(Adapter)->desc_dma_buf_amsdu;
        PlatformZeroMemory(desc_dma_buf_start_amsdu, _GET_HAL_DATA(Adapter)->desc_dma_buf_len_amsdu);

#ifdef TRXBD_CACHABLE_REGION
        _dma_cache_wback((unsigned long)((PVOID)(desc_dma_buf_start_amsdu)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), _GET_HAL_DATA(Adapter)->desc_dma_buf_len_amsdu);
#endif

        pdesc_dma_buf_amsdu = (pu1Byte)(((unsigned long)desc_dma_buf_start_amsdu) + \
            (HAL_PAGE_SIZE - (((unsigned long)desc_dma_buf_start_amsdu) & (HAL_PAGE_SIZE-1))));
        
        //Transfer to Non-cachable address
#ifdef TRXBD_CACHABLE_REGION    
        // Do nothing for un-cachable      
#else
        pdesc_dma_buf_amsdu = (pu1Byte)HAL_TO_NONCACHE_ADDR((u4Byte)pdesc_dma_buf_amsdu);
#endif
    }
#endif // CFG_HAL_TX_AMSDU
    //No Beacon
    ptxbd_head      = (PTX_BUFFER_DESCRIPTOR)(prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].pRXBD_head + \
                        prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].total_rxbd_num);

    //No Beacon
    ptx_desc_head   = (PTX_DESC_88XX)((pu1Byte)ptxbd_head + \
                        sizeof(TX_BUFFER_DESCRIPTOR) * TotalTXBDNum_NoBcn);
    
    ptxbd_bcn_head  = (PTX_BUFFER_DESCRIPTOR)((pu1Byte)ptx_desc_head + \
                        sizeof(TX_DESC_88XX) * TotalTXBDNum_NoBcn);

#if IS_RTL8881A_SERIES
    if (IS_HARDWARE_TYPE_8881A(Adapter)) {
        ptxdesc_bcn_head = (PTX_DESC_88XX)((pu1Byte)ptxbd_bcn_head + \
                                (1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_V2);
        beacon_offset = TXBD_BEACON_OFFSET_V2;
    }
#endif
#if IS_RTL8192E_SERIES || IS_RTL8814A_SERIES || IS_RTL8197F_SERIES || IS_RTL8822B_SERIES
    if (IS_HARDWARE_TYPE_8192E(Adapter) || IS_HARDWARE_TYPE_8814A(Adapter) || IS_HARDWARE_TYPE_8197F(Adapter)|| IS_HARDWARE_TYPE_8822B(Adapter)) {
        ptxdesc_bcn_head = (PTX_DESC_88XX)((pu1Byte)ptxbd_bcn_head + \
                                (1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_V1);
        beacon_offset = TXBD_BEACON_OFFSET_V1;
    }
#endif

    // initiate all tx queue data structures 
    for (q_num = 0; q_num < HCI_TX_DMA_QUEUE_MAX_NUM; q_num++)
    {
        ptx_dma->tx_queue[q_num].hw_idx         = 0;
        ptx_dma->tx_queue[q_num].host_idx       = 0;
        ptx_dma->tx_queue[q_num].total_txbd_num = pTXBD_NUM[q_num];
        ptx_dma->tx_queue[q_num].avail_txbd_num = pTXBD_NUM[q_num];
        ptx_dma->tx_queue[q_num].reg_rwptr_idx  = TXBD_RWPtr_Reg[q_num];

        if ( 0 == q_num ) {
            ptx_dma->tx_queue[q_num].pTXBD_head    = ptxbd_head;
            ptx_dma->tx_queue[q_num].ptx_desc_head = ptx_desc_head;
        }
        else {
            if ( HCI_TX_DMA_QUEUE_BCN != q_num ) {
                ptx_dma->tx_queue[q_num].pTXBD_head    = ptx_dma->tx_queue[q_num-1].pTXBD_head + pTXBD_NUM[q_num-1];
                ptx_dma->tx_queue[q_num].ptx_desc_head = ((PTX_DESC_88XX)ptx_dma->tx_queue[q_num-1].ptx_desc_head) + pTXBD_NUM[q_num-1];
            }
            else {
                ptx_dma->tx_queue[q_num].pTXBD_head    = ptxbd_bcn_head;
                ptx_dma->tx_queue[q_num].ptx_desc_head = ptxdesc_bcn_head;
            }
        }
        
        ptxbd    = ptx_dma->tx_queue[q_num].pTXBD_head;
        ptx_desc = ptx_dma->tx_queue[q_num].ptx_desc_head;

		if(TXBD_RWPtr_Reg[q_num])
        	HAL_RTL_W32(TXBD_RWPtr_Reg[q_num], 0);
		if(TXBD_Reg[q_num]){        
            HAL_RTL_W32(TXBD_Reg[q_num], HAL_VIRT_TO_BUS((u4Byte)ptxbd) + CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL);
#ifdef PCIE_POWER_SAVING_TEST            
            _GET_HAL_DATA(Adapter)->txBD_dma_ring_addr[q_num] = HAL_VIRT_TO_BUS((u4Byte)ptxbd) + CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL;
#endif
        }

#if 0

        RT_TRACE_F(COMP_INIT, DBG_TRACE, ("QNum: %ld, TXBDHead: 0x%lx, TXDESCHead: 0x%lx\n", \
                                            (u4Byte)q_num, \
                                            (u4Byte)ptx_dma->tx_queue[q_num].pTXBD_head, \
                                            (u4Byte)ptx_dma->tx_queue[q_num].ptx_desc_head
                                            ));
#endif

        // assign LowAddress and TxDescLength to each TXBD element
        if (q_num != HCI_TX_DMA_QUEUE_BCN) {
            for(i = 0; i < pTXBD_NUM[q_num]; i++)        
            {   
                SET_DESC_FIELD_CLR(ptxbd[i].TXBD_ELE[0].Dword0, \
                        TXDESCSize, \
                        TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
                SET_DESC_FIELD_CLR(ptxbd[i].TXBD_ELE[0].Dword1, \
                        HAL_VIRT_TO_BUS((u4Byte)&ptx_desc[i]) + CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL, \
                        TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);

                PlatformZeroMemory(&(ptxbd[i].TXBD_ELE[1]), sizeof(TXBD_ELEMENT)*(TXBD_ELE_NUM-1));
#ifdef WLAN_SUPPORT_H2C_PACKET
                if(HCI_TX_DMA_QUEUE_CMD == q_num)
                {
                    SET_DESC_FIELD_CLR(ptxbd[i].TXBD_ELE[1].Dword0, \
                        H2C_PACKET_PAYLOAD_MAX_SIZE, \
                        TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
                    SET_DESC_FIELD_CLR(ptxbd[i].TXBD_ELE[1].Dword1, \
                        HAL_VIRT_TO_BUS(&(h2c_buf_start[i])) + CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL, \
                        TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);
                    // h2c TXBD just use 1 page
                    SET_DESC_FIELD_NO_CLR(ptxbd[i].TXBD_ELE[0].Dword0, 1, TXBD_DW0_PSLEN_MSK, TXBD_DW0_PSLEN_SH);   

                    ph2c_tx_desc = &ptx_desc[i];

                    SET_TX_DESC_TXPKTSIZE_NO_CLR(ph2c_tx_desc, 32);
                    SET_TX_DESC_OFFSET_NO_CLR(ph2c_tx_desc, 0);
                    SET_TX_DESC_QSEL(ph2c_tx_desc, TXDESC_QSEL_CMD); 
#ifdef TRXBD_CACHABLE_REGION
                    _dma_cache_wback((unsigned long)(&(ptxbd[i].TXBD_ELE[1])-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), 
                            sizeof(TX_BUFFER_DESCRIPTOR));
                    _dma_cache_wback((unsigned long)(&ptx_desc[i]-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), 
                            TXDESCSize);
#endif						                   
                }

                
#endif //WLAN_SUPPORT_H2C_PACKET
#ifdef TRXBD_CACHABLE_REGION
                _dma_cache_wback((unsigned long)(&(ptxbd[i].TXBD_ELE[0])-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), 
                        sizeof(TX_BUFFER_DESCRIPTOR));
#endif						
#if 0
               if(HCI_TX_DMA_QUEUE_CMD == q_num)
               {
               RT_TRACE_F(COMP_INIT, DBG_TRACE, ("ptxbd[%ld]:TXBD_ELE[0] Dword0: 0x%lx, Dword1: 0x%lx\n", \
                                                i, \
                                                (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[0].Dword0), \
                                                (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[0].Dword1)
                                                ));
               RT_TRACE_F(COMP_INIT, DBG_TRACE, ("ptxbd[%ld]:TXBD_ELE[1] Dword0: 0x%lx, Dword1: 0x%lx\n", \
                                                i, \
                                                (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[1].Dword0), \
                                                (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[1].Dword1)
                                                ));               
                }
#endif

            }
        } else {
            // beacon...
            //for (i = 0; i < 1+4; i++)
            for (i = 0; i < 1+HAL_NUM_VWLAN; i++)
            {
                ptxbd_bcn_cur = (PTX_BUFFER_DESCRIPTOR)((pu1Byte)ptxbd + beacon_offset * i);
                SET_DESC_FIELD_CLR(ptxbd_bcn_cur->TXBD_ELE[0].Dword0, \
                        TXDESCSize, \
                        TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
                SET_DESC_FIELD_CLR(ptxbd_bcn_cur->TXBD_ELE[0].Dword1, \
                        HAL_VIRT_TO_BUS((u4Byte)&ptx_desc[i]) + CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL, \
                        TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);
#ifdef TRXBD_CACHABLE_REGION
                _dma_cache_wback((unsigned long)(&(ptxbd_bcn_cur->TXBD_ELE[0])-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), 
                    sizeof(TXBD_ELEMENT));
#endif //#ifdef TRXBD_CACHABLE_REGION                
#if 1
                RT_TRACE_F(COMP_INIT, DBG_TRACE, ("ptxbd_bcn[%ld]: 0x%lx, Dword0: 0x%lx, Dword1: 0x%lx \n", \
                                                i, (u4Byte)ptxbd_bcn_cur, \
                                                (u4Byte)GET_DESC(ptxbd_bcn_cur->TXBD_ELE[0].Dword0), \
                                                (u4Byte)GET_DESC(ptxbd_bcn_cur->TXBD_ELE[0].Dword1)
                                                ));             
#endif
            }
        }        
    }   


#if CFG_HAL_TX_AMSDU
    if ( IS_SUPPORT_TX_AMSDU(Adapter) ) {

    //ptxbd_head_amsdu    = ptx_dma_amsdu;
    //ptx_dma_amsdu = pdesc_dma_buf_amsdu;

    // BK, BE, VI, VO
    ptx_dma_amsdu->tx_amsdu_queue[HCI_TX_AMSDU_DMA_QUEUE_BK].pTXBD_head_amsdu = pdesc_dma_buf_amsdu;
    ptx_dma_amsdu->tx_amsdu_queue[HCI_TX_AMSDU_DMA_QUEUE_BE].pTXBD_head_amsdu = (pu1Byte)ptx_dma_amsdu->tx_amsdu_queue[HCI_TX_AMSDU_DMA_QUEUE_BK].pTXBD_head_amsdu + TX_BKQ_TXBD_NUM * sizeof(TXBD_ELEMENT)* MAX_NUM_OF_MSDU_IN_AMSDU;
    ptx_dma_amsdu->tx_amsdu_queue[HCI_TX_AMSDU_DMA_QUEUE_VI].pTXBD_head_amsdu = (pu1Byte)ptx_dma_amsdu->tx_amsdu_queue[HCI_TX_AMSDU_DMA_QUEUE_BE].pTXBD_head_amsdu + TX_BEQ_TXBD_NUM * sizeof(TXBD_ELEMENT)* MAX_NUM_OF_MSDU_IN_AMSDU;
    ptx_dma_amsdu->tx_amsdu_queue[HCI_TX_AMSDU_DMA_QUEUE_VO].pTXBD_head_amsdu = (pu1Byte)ptx_dma_amsdu->tx_amsdu_queue[HCI_TX_AMSDU_DMA_QUEUE_VI].pTXBD_head_amsdu + TX_VIQ_TXBD_NUM * sizeof(TXBD_ELEMENT)* MAX_NUM_OF_MSDU_IN_AMSDU;
    }
#endif

    return RT_STATUS_SUCCESS;
}


static BOOLEAN 
IsTXBDFull88XX(
    IN   HAL_PADAPTER               Adapter,
    IN   u4Byte                     queueIndex  //HCI_TX_DMA_QUEUE_88XX
)
{
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;    

    ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    cur_q       = &(ptx_dma->tx_queue[queueIndex]);
    
    if (HAL_CIRC_SPACE_RTK(cur_q->host_idx, cur_q->hw_idx, cur_q->total_txbd_num) == 0) {
        // case: full
        RT_TRACE(COMP_SEND, DBG_LOUD, ("TXBD is Full !!!\n") );
        return _TRUE;
    }
    return _FALSE;
}


#if IS_EXIST_RTL8192EE || IS_EXIST_RTL8881AEM || IS_EXIST_RTL8814AE 

static VOID
SetTxDescQSel88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PTX_DESC_88XX   ptx_desc,
    IN  u1Byte          drvTID
)
{
    u1Byte  q_select;
    //u4Byte  val=0;
    
	switch (queueIndex) {
    	case HCI_TX_DMA_QUEUE_HI0:
    		q_select = TXDESC_QSEL_HIGH;
    		break;
#if  CFG_HAL_SUPPORT_MBSSID
    	case HCI_TX_DMA_QUEUE_HI1:
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI2:
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI3:	
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI4:
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI5:
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI6:
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI7:
            q_select = TXDESC_QSEL_HIGH;
    		break;
#endif  //CFG_HAL_SUPPORT_MBSSID

    	case HCI_TX_DMA_QUEUE_MGT:
    		q_select = TXDESC_QSEL_MGT;
    		break;
            
#if CFG_HAL_MAC_LOOPBACK && CFG_HAL_WIFI_WMM
    	case HCI_TX_DMA_QUEUE_BE:
    		q_select = TXDESC_QSEL_TID0;
    		break;
#endif  //CFG_HAL_MAC_LOOPBACK && CFG_HAL_WIFI_WMM

    	default:
    		// data packet
#if CFG_HAL_RTL_MANUAL_EDCA
    		if (HAL_VAR_MANUAL_EDCA) {
    			switch (queueIndex) {
        			case HCI_TX_DMA_QUEUE_VO:
        				q_select = TXDESC_QSEL_TID6;
        				break;
        			case HCI_TX_DMA_QUEUE_VI:
        				q_select = TXDESC_QSEL_TID4;
                		break;
        			case HCI_TX_DMA_QUEUE_BE:
        				q_select = TXDESC_QSEL_TID0;
        	    		break;
        		    default:
        				q_select = TXDESC_QSEL_TID1;
        				break;
    			}
    		}
    		else {
                q_select = drvTID;
    		}
#else
            q_select = drvTID;
#endif  //CFG_HAL_RTL_MANUAL_EDCA
            break;
	}    

    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, q_select, TX_DW1_QSEL_MSK, TX_DW1_QSEL_SH);
    //ptx_desc->Dword1 |= val;
}

static VOID
SetSecType(
    IN  HAL_PADAPTER    Adapter,
    IN  PTX_DESC_88XX   ptx_desc,
    IN  PVOID           pDescData 
)
{
    PTX_DESC_DATA_88XX  pdesc_data  = (PTX_DESC_DATA_88XX)pDescData;
    
    switch(pdesc_data->secType) {
    case _WEP_40_PRIVACY_:
    case _WEP_104_PRIVACY_:
    case _TKIP_PRIVACY_:
        SET_DESC_FIELD_CLR(ptx_desc->Dword1, TXDESC_SECTYPE_WEP40_OR_TKIP,
                                        TX_DW1_SECTYPE_MSK, TX_DW1_SECTYPE_SH);
        break;
#if CFG_HAL_RTL_HW_WAPI_SUPPORT
    case _WAPI_SMS4_:
        SET_DESC_FIELD_CLR(ptx_desc->Dword1, TXDESC_SECTYPE_WAPI,
                                        TX_DW1_SECTYPE_MSK, TX_DW1_SECTYPE_SH);
        break;
#endif        
    case _CCMP_PRIVACY_:
        SET_DESC_FIELD_CLR(ptx_desc->Dword1, TXDESC_SECTYPE_AES,
                                        TX_DW1_SECTYPE_MSK, TX_DW1_SECTYPE_SH);
        break;
    default:
#if 0
        SET_DESC_FIELD_CLR(ptx_desc->Dword1, TXDESC_SECTYPE_NO_ENCRYPTION,
                                        TX_DW1_SECTYPE_MSK, TX_DW1_SECTYPE_SH);        
#endif
        break;
    }   
}

#endif //#if IS_EXIST_RTL8192EE || IS_EXIST_RTL8881AEM || IS_EXIST_RTL8814AE 

#if IS_EXIST_RTL8192EE || IS_EXIST_RTL8881AEM || IS_EXIST_RTL8814AE 

VOID
FillTxDesc88XX (
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pDescData
)
{
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PTX_DESC_88XX                   ptx_desc;
    PTX_DESC_DATA_88XX              pdesc_data = (PTX_DESC_DATA_88XX)pDescData;
    u4Byte                          val;
    u4Byte                          tmp,tmpCache;
    u4Byte                          TXDESCSize;

    TXDESCSize = SIZE_TXDESC_88XX;
 
    //Dword 0
    u2Byte  TX_DESC_TXPKTSIZE        = pdesc_data->hdrLen + pdesc_data->llcLen + pdesc_data->frLen;
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    u1Byte	TX_DESC_OFFSET			 = ((pdesc_data->smhEn == TRUE) ? (HAL_HW_TXSC_HDR_CONV_ADD_OFFSET+TXDESCSize) : TXDESCSize);
#else
    u1Byte  TX_DESC_OFFSET           = TXDESCSize; //HAL_TXDESC_OFFSET_SIZE;
#endif

#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    BOOLEAN TX_DESC_BMC              = (pdesc_data->smhEn == TRUE) ? 
    					((HAL_IS_MCAST(GetEthDAPtr((pu1Byte)pdesc_data->pHdr))) ? 1 : 0) : 
				        ((HAL_IS_MCAST(GetAddr1Ptr((pu1Byte)pdesc_data->pHdr))) ? 1 : 0);   // when multicast or broadcast, BMC = 1
#else
    BOOLEAN TX_DESC_BMC              = ((HAL_IS_MCAST(GetAddr1Ptr((pu1Byte)pdesc_data->pHdr))) ? 1 : 0);   // when multicast or broadcast, BMC = 1        
#endif

//    BOOLEAN TX_DESC_HT               = 0;
//    BOOLEAN TX_DESC_LINIP            = 0;
//    BOOLEAN TX_DESC_NOACM            = 0;
//    BOOLEAN TX_DESC_GF               = 0;

    //Dword 1 
    u1Byte  TX_DESC_MACID            = pdesc_data->macId; // MACID/MBSSID ?
    u1Byte  TX_DESC_RATE_ID          = pdesc_data->rateId;
    BOOLEAN TX_DESC_MORE_DATA        = pdesc_data->moreData;    
    BOOLEAN TX_DESC_EN_DESC_ID       = pdesc_data->enDescId;
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    u1Byte  TX_DESC_PKT_OFFSET       = 0; // unit: 8 bytes. Early mode: 1 units, (8 * 1 = 8 bytes for early mode info)
#endif
    
    //Dword 2
    BOOLEAN TX_DESC_AGG_EN           = pdesc_data->aggEn; 
    BOOLEAN TX_DESC_BK               = pdesc_data->bk;
    BOOLEAN TX_DESC_MOREFRAG         = pdesc_data->frag;
    u1Byte  TX_DESC_AMPDU_DENSITY    = pdesc_data->ampduDensity;
    u4Byte  TX_DESC_P_AID      		 = pdesc_data->p_aid;
    u1Byte  TX_DESC_G_ID      		 = pdesc_data->g_id;
#if CFG_HAL_HW_AES_IV
    BOOLEAN TX_DESC_HW_AES_IV        = pdesc_data->hwAESIv;
#endif // CFG_HAL_HW_AES_IV
    u1Byte	TX_DESC_CCA_RTS			 = pdesc_data->cca_rts;


    //Dword 3
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV_LLC
    u1Byte  TX_DESC_WHEADER_LEN      = ((pdesc_data->smhEn == TRUE) ? (HAL_ETH_HEADER_LEN_MAX >> 1) : ((pdesc_data->hdrLen) >> 1)); // unit: 2 bytes
#else
    u1Byte  TX_DESC_WHEADER_LEN      = ((pdesc_data->smhEn == TRUE) ? (HAL_ETH_HEADER_LEN_MAX >> 1) : ((pdesc_data->hdrLen+pdesc_data->llcLen+pdesc_data->iv) >> 1)); // unit: 2 bytes
#endif // CFG_HAL_HW_TX_SHORTCUT_HDR_CONV_LLC
#endif // CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    BOOLEAN TX_DESC_USERATE          = pdesc_data->useRate;    
    BOOLEAN TX_DESC_DISRTSFB         = pdesc_data->disRTSFB;
    BOOLEAN TX_DESC_DISDATAFB        = pdesc_data->disDataFB;
    BOOLEAN TX_DESC_CTS2SELF         = pdesc_data->CTS2Self;
    BOOLEAN TX_DESC_RTS_EN           = pdesc_data->RTSEn;
    BOOLEAN TX_DESC_HW_RTS_EN        = pdesc_data->HWRTSEn;
    BOOLEAN TX_DESC_NAVUSEHDR        = pdesc_data->navUseHdr;
    u1Byte  TX_DESC_MAX_AGG_NUM      = pdesc_data->maxAggNum;
    BOOLEAN TX_DESC_NDPA		     = pdesc_data->ndpa;

    //Dword 4
    u1Byte  TX_DESC_DATERATE         = pdesc_data->dataRate;
    u1Byte  TX_DESC_DATA_RATEFB_LMT  = pdesc_data->dataRateFBLmt;
    u1Byte  TX_DESC_RTS_RATEFB_LMT   = pdesc_data->RTSRateFBLmt;
    BOOLEAN TX_DESC_RTY_LMT_EN       = pdesc_data->rtyLmtEn;   
    u1Byte  TX_DESC_DATA_RT_LMT      = pdesc_data->dataRtyLmt;
    u1Byte  TX_DESC_RTSRATE          = pdesc_data->RTSRate;
    u1Byte  TX_DESC_BMCRtyLmt        = pdesc_data->BMCRtyLmt;

    //Dword 5
    u1Byte  TX_DESC_DATA_SC          = pdesc_data->dataSC;
    u1Byte  TX_DESC_DATA_SHORT       = pdesc_data->dataShort;
    u1Byte  TX_DESC_DATA_BW          = pdesc_data->dataBW;
    u1Byte  TX_DESC_DATA_STBC        = pdesc_data->dataStbc;
	u1Byte  TX_DESC_DATA_LDPC        = pdesc_data->dataLdpc;
    u1Byte  TX_DESC_RTS_SHORT        = pdesc_data->RTSShort;
    u1Byte  TX_DESC_RTS_SC           = pdesc_data->RTSSC;   
    u1Byte	TX_DESC_POWER_OFFSET	 = pdesc_data->TXPowerOffset;
    u1Byte	TX_ANT					 = pdesc_data->TXAnt;
	
    //Dword 6
#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
    u1Byte  TX_DESC_ANTSEL			 = pdesc_data->antSel;	
    u1Byte  TX_DESC_ANTSEL_A			 = pdesc_data->antSel_A;	
    u1Byte  TX_DESC_ANTSEL_B			 = pdesc_data->antSel_B;	
    u1Byte  TX_DESC_ANTSEL_C			 = pdesc_data->antSel_C;	
#endif	//#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))

    //Dword 7
    // use for CFG_HAL_TX_SHORTCUT
    u2Byte  TX_DESC_TXBUFF           = pdesc_data->frLen;

	// Dword 8
#if CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
	BOOLEAN	TX_DESC_TXWIFI_CP		= pdesc_data->txwifiCp;
#endif // CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
	BOOLEAN TX_DESC_MAC_CP			= pdesc_data->macCp;
	BOOLEAN TX_DESC_SMH_EN			= pdesc_data->smhEn;
#endif // CFG_HAL_HW_TX_SHORTCUT_HDR_CONV

    //Dword 9
#if CFG_HAL_HW_SEQ
    u2Byte TX_DESC_SEQ               = 0;
#else
    u2Byte TX_DESC_SEQ               = GetSequence(pdesc_data->pHdr);
#endif

#if 0 //(CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC||CFG_HAL_HW_TX_SHORTCUT_HDR_CONV)
	if (pdesc_data->smhEn != 0 || pdesc_data->macCp != 0)
		printk("%s(%d): macCp:0x%x, smhEn:0x%x \n", __FUNCTION__, __LINE__, pdesc_data->macCp, pdesc_data->smhEn);

	if (pdesc_data->txwifiCp != 0)
		printk("%s(%d): txwifiCp:0x%x \n", __FUNCTION__, __LINE__, pdesc_data->txwifiCp);
#endif

#if 0 // show TX Rpt Info
    {
        int baseReg = 0x8080, offset, i;
        int lenTXDESC = 10, lenHdrInfo = 20;
    
        if (pdesc_data->smhEn != 0) {
            HAL_RTL_W8(0x106, 0x7F);
            HAL_RTL_W32(0x140, 0x662);
        
            for(i = 0; i < lenTXDESC; i++) {
                printk("%08X ", (u4Byte)GET_DESC(HAL_RTL_R32(baseReg)));
                if (i%4==3)
                    printk("\n");
                baseReg += 4;
            }
            printk("\n");
        
            for(i = 0; i < lenHdrInfo; i++) {
                printk("%08X ", (u4Byte)GET_DESC(HAL_RTL_R32(baseReg)));
                if (i%4==3)
                    printk("\n");
                baseReg += 4;
            }
            printk("\n");
        }
    }
#endif

	if(Adapter->pshare->rf_ft_var.manual_ampdu)
		TX_DESC_MAX_AGG_NUM = 0;

#if CFG_HAL_MULTICAST_BMC_ENHANCE
    if(TX_DESC_BMC)
    {
        TX_DESC_BMCRtyLmt = pdesc_data->BMCRtyLmt;
    }
#endif


    ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    cur_q       = &(ptx_dma->tx_queue[queueIndex]);
    ptx_desc    = ((PTX_DESC_88XX)(cur_q->ptx_desc_head)) + cur_q->host_idx;

    //Clear All Bit
    PlatformZeroMemory((PVOID)ptx_desc, TXDESCSize);

    if (pdesc_data->secType != _NO_PRIVACY_) {
        if (pdesc_data->swCrypt == FALSE) {
            SetSecType(Adapter, ptx_desc, pdesc_data);
            // for hw sec: 1) WEP's iv, 2) TKIP's iv and eiv, 3) CCMP's ccmp header are all in pdesc_data->iv
            TX_DESC_TXPKTSIZE += pdesc_data->iv;
        } else {
            // for sw sec
            TX_DESC_TXPKTSIZE += (pdesc_data->iv + pdesc_data->icv + pdesc_data->mic);
        }
    }


    //4 Set Dword0
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword0, TX_DESC_TXPKTSIZE, TX_DW0_TXPKSIZE_MSK, TX_DW0_TXPKSIZE_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword0, TX_DESC_OFFSET, TX_DW0_OFFSET_MSK, TX_DW0_OFFSET_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword0, TX_DESC_BMC, TX_DW0_BMC_MSK, TX_DW0_BMC_SH);

    //4 Set Dword1
    SetTxDescQSel88XX(Adapter, queueIndex, ptx_desc, pdesc_data->tid);        
    if ( (queueIndex >= HCI_TX_DMA_QUEUE_HI0) && (queueIndex <= HCI_TX_DMA_QUEUE_HI7) ) {
        //MacID has written in SetTxDescQSel88XX()
#ifdef HW_ENC_FOR_GROUP_CIPHER
        SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, TX_DESC_MACID, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
#endif
    } else {
        SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, TX_DESC_MACID, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
    }
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, TX_DESC_RATE_ID, TX_DW1_RATE_ID_MSK, TX_DW1_RATE_ID_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, TX_DESC_MORE_DATA, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
    if (TX_DESC_EN_DESC_ID)
        SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_EN_DESC_ID_MSK, TX_DW1_EN_DESC_ID_SH);

    //4 Set Dword2
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword2, TX_DESC_AGG_EN, TX_DW2_AGG_EN_MSK, TX_DW2_AGG_EN_SH);    
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword2, TX_DESC_BK, TX_DW2_BK_MSK, TX_DW2_BK_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword2, TX_DESC_MOREFRAG, TX_DW2_MOREFRAG_MSK, TX_DW2_MOREFRAG_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword2, TX_DESC_AMPDU_DENSITY, TX_DW2_AMPDU_DENSITY_MSK, TX_DW2_AMPDU_DENSITY_SH);    
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword2, TX_DESC_P_AID, TX_DW2_P_AID_MSK, TX_DW2_P_AID_SH);    
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword2, TX_DESC_G_ID, TX_DW2_G_ID_MSK, TX_DW2_G_ID_SH);
#if IS_EXIST_RTL8814AE
    if (IS_HARDWARE_TYPE_8814AE(Adapter)) {
#if CFG_HAL_HW_AES_IV
        SET_DESC_FIELD_NO_CLR(ptx_desc->Dword2, TX_DESC_HW_AES_IV, TX_DW2_HW_AES_IV_MSK, TX_DW2_HW_AES_IV_SH);
#endif // CFG_HAL_HW_AES_IV
    }
#endif
#if 1 //eric-ac2
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword2, TX_DESC_CCA_RTS, TX_DW2_CCA_RTS_MSK, TX_DW2_CCA_RTS_SH);
#endif


    //4 Set Dword3
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_USERATE, TX_DW3_USERATE_MSK, TX_DW3_USERATE_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_DISRTSFB, TX_DW3_DISRTSFB_MSK, TX_DW3_DISRTSFB_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_DISDATAFB, TX_DW3_DISDATAFB_MSK, TX_DW3_DISDATAFB_SH);

    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_CTS2SELF, TX_DW3_CTS2SELF_MSK, TX_DW3_CTS2SELF_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_RTS_EN, TX_DW3_RTSEN_MSK, TX_DW3_RTSEN_SH);
   
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_HW_RTS_EN, TX_DW3_HW_RTS_EN_MSK, TX_DW3_HW_RTS_EN_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_NAVUSEHDR, TX_DW3_NAVUSEHDR_MSK, TX_DW3_NAVUSEHDR_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_MAX_AGG_NUM, TX_DW3_MAX_AGG_NUM_MSK, TX_DW3_MAX_AGG_NUM_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_NDPA, TX_DW3_NDPA_MSK, TX_DW3_NDPA_SH);

    //4 Set Dword4
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, TX_DESC_DATERATE, TX_DW4_DATARATE_MSK, TX_DW4_DATARATE_SH);
#if IS_EXIST_RTL8881AEM || IS_EXIST_RTL8192EE
	if (IS_HARDWARE_TYPE_8881A(Adapter) || IS_HARDWARE_TYPE_8192EE(Adapter)) {
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, TX_DESC_DATA_RATEFB_LMT, TX_DW4_DATA_RATEFB_LMT_MSK, TX_DW4_DATA_RATEFB_LMT_SH);
	    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, TX_DESC_RTS_RATEFB_LMT, TX_DW4_RTS_RATEFB_LMT_MSK, TX_DW4_RTS_RATEFB_LMT_SH);
	}
#endif
#if IS_RTL88XX_MAC_V2
    if ( _GET_HAL_DATA(Adapter)->MacVersion.is_MAC_v2) {
		if (TX_DESC_BMC) { 
		    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, 0, TX_DW4_DATA_RTY_LOWEST_RATE_MSK, TX_DW4_DATA_RTY_LOWEST_RATE_SH);
	    } else {
		    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, TX_DESC_DATA_RATEFB_LMT, TX_DW4_DATA_RTY_LOWEST_RATE_MSK, TX_DW4_DATA_RTY_LOWEST_RATE_SH);
	    }
	    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, TX_DESC_RTS_RATEFB_LMT, TX_DW4_RTS_RTY_LOWEST_RATE_MSK, TX_DW4_RTS_RTY_LOWEST_RATE_SH);

#if CFG_HAL_MULTICAST_BMC_ENHANCE
        if(TX_DESC_BMC) {
    	    SET_DESC_FIELD_CLR(ptx_desc->Dword4, TX_DESC_BMCRtyLmt, TX_DW4_DATA_RTY_LOWEST_RATE_MSK, TX_DW4_DATA_RTY_LOWEST_RATE_SH);    
        }
#endif //#if CFG_HAL_MULTICAST_BMC_ENHANCE        
	}
#endif //IS_RTL88XX_MAC_V2

    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, TX_DESC_RTY_LMT_EN, TX_DW4_RTY_LMT_EN_MSK, TX_DW4_RTY_LMT_EN_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, TX_DESC_DATA_RT_LMT, TX_DW4_DATA_RT_LMT_MSK, TX_DW4_DATA_RT_LMT_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, TX_DESC_RTSRATE, TX_DW4_RTSRATE_MSK, TX_DW4_RTSRATE_SH);

    //4 Set Dword5
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_DESC_DATA_SC, TX_DW5_DATA_SC_MSK, TX_DW5_DATA_SC_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_DESC_DATA_SHORT, TX_DW5_DATA_SHORT_MSK, TX_DW5_DATA_SHORT_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_DESC_DATA_BW, TX_DW5_DATA_BW_MSK, TX_DW5_DATA_BW_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_DESC_DATA_STBC, TX_DW5_DATA_STBC_MSK, TX_DW5_DATA_STBC_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_DESC_DATA_LDPC, TX_DW5_DATA_LDPC_MSK, TX_DW5_DATA_LDPC_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_DESC_RTS_SHORT, TX_DW5_RTS_SHORT_MSK, TX_DW5_RTS_SHORT_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_DESC_RTS_SC, TX_DW5_RTS_SC_MSK, TX_DW5_RTS_SC_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_DESC_POWER_OFFSET, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);
	if (Adapter->pmib->dot11RFEntry.bcn2path || Adapter->pmib->dot11RFEntry.tx2path)
		SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_ANT, TX_DW5_TX_ANT_MSK, TX_DW5_TX_ANT_SH);

    //4 Set Dword6
#if CFG_HAL_SUPPORT_MBSSID
    if (HAL_IS_VAP_INTERFACE(Adapter)) {
    // set MBSSID for each VAP_ID
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6,HAL_VAR_VAP_INIT_SEQ, TX_DW6_MBSSID_MSK, TX_DW6_MBSSID_SH);  
    }         
#endif //#if CFG_HAL_SUPPORT_MBSSID

#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
   if(TX_DESC_ANTSEL & BIT(0))
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, TX_DESC_ANTSEL_A, TX_DW6_ANTSEL_A_MSK, TX_DW6_ANTSEL_A_SH);
   if(TX_DESC_ANTSEL & BIT(1))
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, TX_DESC_ANTSEL_B, TX_DW6_ANTSEL_B_MSK, TX_DW6_ANTSEL_B_SH);
   if(TX_DESC_ANTSEL & BIT(2))
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, TX_DESC_ANTSEL_C, TX_DW6_ANTSEL_C_MSK, TX_DW6_ANTSEL_C_SH);
#endif	//#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))

    //4 Set Dword7
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword7, TX_DESC_TXBUFF, TX_DW7_SW_TXBUFF_MSK, TX_DW7_SW_TXBUFF_SH);

	//4 Set Dword8
    //4 Set Dword9
#if CFG_HAL_HW_SEQ
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, 1, TX_DW8_EN_HWSEQ_MSK, TX_DW8_EN_HWSEQ_SH);
#else
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword9, TX_DESC_SEQ, TX_DW9_SEQ_MSK, TX_DW9_SEQ_SH);    
#endif

#if IS_RTL88XX_MAC_V2
    if ( _GET_HAL_DATA(Adapter)->MacVersion.is_MAC_v2) {
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
		SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, TX_DESC_PKT_OFFSET, TX_DW1_PKT_OFFSET_MSK, TX_DW1_PKT_OFFSET_SH);
		SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_WHEADER_LEN, TX_DW3_WHEADER_V1_MSK, TX_DW3_WHEADER_V1_SH);
		SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, TX_DESC_MAC_CP, TX_DW8_MAC_CP_MSK, TX_DW8_MAC_CP_SH);
		SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, TX_DESC_SMH_EN, TX_DW8_SMH_EN_MSK, TX_DW8_SMH_EN_SH);
#endif
#if CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
        if(IS_SUPPORT_WLAN_HAL_HW_TX_SHORTCUT_REUSE_TXDESC(Adapter)) {
		    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, TX_DESC_TXWIFI_CP, TX_DW8_TXWIFI_CP_MSK, TX_DW8_TXWIFI_CP_SH);
        }
#endif
	}
#endif // IS_RTL88XX_MAC_V2


#if 0
	3.) STW_ANT_DIS:  
	ant_mapA, ant_mapB, ant_mapC, ant_mapD, ANTSEL_A, ANTSEL_B, Ntx_map, TXPWR_OFFSET 
	4.) STW_RATE_DIS:  
	USE_RATE, Data rate, DATA_SHORT, DATA_BW, TRY_RATE
	5.) STW_RB_DIS:  
	RATE_ID, DISDATAFB, DISRTSFB, RTS_RATEFB_LMT, DATA_RATEFB_LMT
	6.) STW_PKTRE_DIS:	
	RTY_LMT_EN,  DATA_RT_LMT,  BAR_RTY_TH
#endif

#if 0
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, 0x7, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, 0x7, TX_DW6_ANTSEL_A_MSK, TX_DW6_ANTSEL_A_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, 0x7, TX_DW6_ANTSEL_B_MSK, TX_DW6_ANTSEL_B_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, 0x3, TX_DW6_ANT_MAP_A_MSK, TX_DW6_ANT_MAP_A_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, 0x3, TX_DW6_ANT_MAP_B_MSK, TX_DW6_ANT_MAP_B_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, 0x3, TX_DW6_ANT_MAP_C_MSK, TX_DW6_ANT_MAP_C_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, 0x3, TX_DW6_ANT_MAP_D_MSK, TX_DW6_ANT_MAP_D_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword7, 0xF, TX_DW7_NTX_MAP_MSK, TX_DW7_NTX_MAP_SH);
#endif

#if 0
	if(queueIndex == HCI_TX_DMA_QUEUE_BE) {
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, 0x1, TX_DW3_USERATE_MSK, TX_DW3_USERATE_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, 0x7F, TX_DW4_DATARATE_MSK, TX_DW4_DATARATE_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, 0x1, TX_DW4_TRY_RATE_MSK, TX_DW4_TRY_RATE_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, 0x1, TX_DW5_DATA_SHORT_MSK, TX_DW5_DATA_SHORT_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, 0x3, TX_DW5_DATA_BW_MSK, TX_DW5_DATA_BW_SH);
	}
#endif

#if 0
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 0x1F, TX_DW1_RATE_ID_MSK, TX_DW1_RATE_ID_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, 0x1, TX_DW3_DISDATAFB_MSK, TX_DW3_DISDATAFB_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, 0x1, TX_DW3_DISRTSFB_MSK, TX_DW3_DISRTSFB_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, 0xF, TX_DW4_RTS_RATEFB_LMT_MSK, TX_DW4_RTS_RATEFB_LMT_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, 0x1F, TX_DW4_DATA_RATEFB_LMT_MSK, TX_DW4_DATA_RATEFB_LMT_SH);
#endif

#if 0
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, 0x1, TX_DW4_RTY_LMT_EN_MSK, TX_DW4_RTY_LMT_EN_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, 0x3F, TX_DW4_DATA_RT_LMT_MSK, TX_DW4_DATA_RT_LMT_SH);
	// TODO: bug ? still copy this field into desc
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, 0x3, TX_DW8_BAR_RTY_TH_MSK, TX_DW8_BAR_RTY_TH_SH);
#endif
#ifdef TRXBD_CACHABLE_REGION
//    _dma_cache_wback((unsigned long)((PVOID)ptx_desc-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(TX_DESC_88XX));
#endif //#ifdef TRXBD_CACHABLE_REGION
}

#endif //#if IS_EXIST_RTL8192EE || IS_EXIST_RTL8881AEM || IS_EXIST_RTL8814AE 

VOID
UpdateSWTXBDHostIdx88XX (
    IN      HAL_PADAPTER                    Adapter,
    IN      PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q
)
{
    cur_q->host_idx++;
    cur_q->host_idx = cur_q->host_idx % cur_q->total_txbd_num;
    cur_q->avail_txbd_num--;
}

enum _TxPktFinalIO88XX_FLAG_{
    TxPktFinalIO88XX_WRITE = 0,
    TxPktFinalIO88XX_CHECK = 1
};

static RT_STATUS
TxPktFinalIO88XX(
    IN      HAL_PADAPTER                    Adapter,
    IN      PTX_BUFFER_DESCRIPTOR           cur_txbd,
    IN      u4Byte                          CtrlFlag,   //enum _TxPktFinalIO88XX_FLAG_{
    IN      u4Byte                          DwordSettingValue
)
{
    switch(CtrlFlag) {
        case TxPktFinalIO88XX_WRITE:
            SET_DESC_FIELD_CLR(cur_txbd->TXBD_ELE[0].Dword0, DwordSettingValue, 
                TXBD_DW0_PSLEN_MSK, TXBD_DW0_PSLEN_SH);
            return RT_STATUS_SUCCESS;
            break;

        case TxPktFinalIO88XX_CHECK:
            if (0 == GET_DESC_FIELD(cur_txbd->TXBD_ELE[0].Dword0, 
                        TXBD_DW0_PSLEN_MSK, TXBD_DW0_PSLEN_SH)) {
                RT_TRACE(COMP_SEND, DBG_WARNING, 
                    ("cur_txbd->TXBD_ELE[0].Dword0 value(0x%lx) error\n", cur_txbd->TXBD_ELE[0].Dword0));
                return RT_STATUS_FAILURE;
            } else {
                return RT_STATUS_SUCCESS;
            }
            break;

        default:
            // Error Case
            RT_TRACE(COMP_SEND, DBG_SERIOUS, ("TxPktFinalIO88XX setting error: 0x%x \n", CtrlFlag));
            return RT_STATUS_FAILURE;
            break;
    }
}


HAL_IMEM
RT_STATUS
SyncSWTXBDHostIdxToHW88XX (
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex  //HCI_TX_DMA_QUEUE_88XX
)
{
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;
    u2Byte                          LastHostIdx;

    ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    cur_q   = &(ptx_dma->tx_queue[queueIndex]);

    
    if ( cur_q->host_idx == 0 ) {
        LastHostIdx = cur_q->total_txbd_num - 1;
    }
    else {
        LastHostIdx = cur_q->host_idx - 1;
    }
#if WLAN_HAL_TXDESC_CHECK_ADDR_LEN
#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881A(Adapter) ) {
    pu4Byte pTxDescPhyAddr, pTxDescLen, pTxPSBLen;
    PTX_BUFFER_DESCRIPTOR cur_txbd;

    cur_txbd       = (PTX_BUFFER_DESCRIPTOR)(_GET_HAL_DATA(Adapter)->cur_txbd);
#if 0
    pTxDescLen     = HAL_TO_NONCACHE_ADDR((u4Byte)(&(_GET_HAL_DATA(Adapter)->cur_tx_desc_len)));
    pTxDescPhyAddr = HAL_TO_NONCACHE_ADDR((u4Byte)(&(_GET_HAL_DATA(Adapter)->cur_tx_desc_phy_addr)));
#endif
    pTxPSBLen      = HAL_TO_NONCACHE_ADDR((u4Byte)(&(_GET_HAL_DATA(Adapter)->cur_tx_psb_len)));


    if (*pTxPSBLen != GET_DESC_FIELD(cur_txbd->TXBD_ELE[0].Dword0, TXBD_DW0_PSLEN_MSK, TXBD_DW0_PSLEN_SH)) {
        panic_printk("%s(%d): Fail: CopyPSBLen:0x%x, PSBLen:0x%x \n", __FUNCTION__, __LINE__, *pTxPSBLen, GET_DESC_FIELD(cur_txbd->TXBD_ELE[0].Dword0, TXBD_DW0_PSLEN_MSK, TXBD_DW0_PSLEN_SH));
    } else {
        //panic_printk("%s(%d): Pass: CopyPSBLen:0x%x, PSBLen:0x%x \n", __FUNCTION__, __LINE__, *pTxPSBLen, GET_DESC_FIELD(cur_txbd->TXBD_ELE[0].Dword0, TXBD_DW0_PSLEN_MSK, TXBD_DW0_PSLEN_SH));    
    }

#if 0
    if (*pTxDescPhyAddr != (cur_txbd->TXBD_ELE[0].Dword1)) {
        panic_printk("%s(%d): Fail: copyPhyAddr:0x%x, phyAddr:0x%x \n", __FUNCTION__, __LINE__, *pTxDescPhyAddr, cur_txbd->TXBD_ELE[0].Dword1);
    } else {  
      //  panic_printk("%s(%d): Pass: copyPhyAddr:0x%x, phyAddr:0x%x \n", __FUNCTION__, __LINE__, *pTxDescPhyAddr, cur_txbd->TXBD_ELE[0].Dword1);
    }

    if (*pTxDescLen != (cur_txbd->TXBD_ELE[0].Dword0)) {
         panic_printk("%s(%d): Fail: copyTxDescLen:0x%x, txDescLen:0x%x \n", __FUNCTION__, __LINE__, *pTxDescLen, cur_txbd->TXBD_ELE[0].Dword0);
    } else {
       //  panic_printk("%s(%d): Pass: copyTxDescLen:0x%x, txDescLen:0x%x \n", __FUNCTION__, __LINE__, *pTxDescLen, cur_txbd->TXBD_ELE[0].Dword0);
    }
#endif
   }
#endif // IS_EXIST_RTL8881AEM
#endif // WLAN_HAL_TXDESC_CHECK_ADDR_LEN
    //Avoid that IO operation is completed before Packet is put into DRAM
    //So, add one read operation
    if (RT_STATUS_SUCCESS == TxPktFinalIO88XX(Adapter, (cur_q->pTXBD_head + LastHostIdx), TxPktFinalIO88XX_CHECK, 0)) {
        HAL_RTL_W16(cur_q->reg_rwptr_idx, cur_q->host_idx);
    } else {
        RT_TRACE_F(COMP_SEND, DBG_WARNING, ("TxPktFinalIO88XX check fail !!! cur_q->host_idx:0x%lx \n", cur_q->host_idx));
    }

//    RT_TRACE_F(COMP_SEND, DBG_TRACE, ("cur_q->host_idx:0x%lx \n", cur_q->host_idx))

    return RT_STATUS_SUCCESS;
}

//Note: This function can't be used by Beacon
static VOID
SetTxBufferDesc88XX (
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      PVOID           pDescData 
)
{
    PTX_DESC_DATA_88XX              pdesc_data = (PTX_DESC_DATA_88XX)pDescData;
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PTX_BUFFER_DESCRIPTOR           cur_txbd;
    u1Byte                          i;
    u4Byte                          TotalLen    = 0;
    u4Byte                          PSBLen;    
    // if each queue num is different, need modify this number....
    u4Byte                          TXBDSegNum  = TXBD_ELE_NUM; 
    u4Byte                          hdrLen      = pdesc_data->hdrLen + pdesc_data->llcLen;
    u4Byte                          payloadLen  = pdesc_data->frLen;
    unsigned long dma_addr;
    u4Byte                          TXDESCSize;
    
#if CFG_HAL_TX_AMSDU
    PHCI_TX_AMSDU_DMA_MANAGER_88XX  ptx_dma_amsdu;
    PTX_BUFFER_DESCRIPTOR_AMSDU     cur_txbd_amsdu;
#endif

#ifdef MERGE_TXDESC_HEADER_PAYLOAD
    PTX_DESC_88XX                   cur_txdesc;
#endif

    ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    cur_q       = &(ptx_dma->tx_queue[queueIndex]);
    cur_txbd    = cur_q->pTXBD_head + cur_q->host_idx;

#if (IS_EXIST_RTL8881AEM || IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM)
    if ( IS_HARDWARE_TYPE_8881A(Adapter) || IS_HARDWARE_TYPE_8192EE(Adapter) || IS_HARDWARE_TYPE_8814AE(Adapter) || IS_HARDWARE_TYPE_8197F(Adapter) ) {
        TXDESCSize = SIZE_TXDESC_88XX;
    }
#endif  //#if (IS_EXIST_RTL8881AEM || IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM)

#if IS_EXIST_RTL8822BE
    if ( IS_HARDWARE_TYPE_8822B(Adapter))  {
        TXDESCSize = SIZE_TXDESC_88XX_V1;
    }
#endif //IS_EXIST_RTL8822BE


#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    // TODO: check it....right or wrong..
	if (pdesc_data->smhEn == FALSE)
#endif
    if (pdesc_data->secType != _NO_PRIVACY_) {
		// TODO: wapi...
        // WEP:  1) icv for hw encrypt
        // TKIP: 1) iv contains eiv,   2) payload contains mic, 3) icv for hw encrypt
        // CCMP: 1) iv is CCMP header, 2) mic for hw encrypt
        hdrLen += pdesc_data->iv;
    }

#ifdef MERGE_TXDESC_HEADER_PAYLOAD

//if (queueIndex == HCI_TX_DMA_QUEUE_BE) {
    //memcpy((skb->data - (txcfg->hdr_len + txcfg->llc + txcfg->iv)), txcfg->phdr, (txcfg->hdr_len + txcfg->llc + txcfg->iv));
    //release_wlanllchdr_to_poll(Adapter, txcfg->phdr);
    //txcfg->phdr = skb->data - (txcfg->hdr_len + txcfg->llc + txcfg->iv);

    PlatformZeroMemory(&(cur_txbd->TXBD_ELE[1]), sizeof(TXBD_ELEMENT)*(TXBD_ELE_NUM-1));

    //cur_txdesc  = (PTX_DESC_88XX)cur_q->ptx_desc_head + cur_q->host_idx;


    HAL_memcpy((pdesc_data->pBuf - hdrLen), pdesc_data->pHdr, hdrLen);
    //release_wlanllchdr_to_poll(Adapter, pdesc_data->pHdr);
#if 1
    pdesc_data->pHdr = pdesc_data->pBuf - hdrLen;
#else
    HAL_memcpy((pdesc_data->pBuf - hdrLen - SIZE_TXDESC_88XX), cur_txdesc, SIZE_TXDESC_88XX);
    pdesc_data->pHdr = pdesc_data->pBuf - hdrLen - SIZE_TXDESC_88XX;
#endif

    TotalLen = SIZE_TXDESC_88XX + hdrLen + payloadLen;
    PSBLen   = (TotalLen%PBP_PSTX_SIZE_V1) == 0 ? (TotalLen/PBP_PSTX_SIZE_V1):((TotalLen/PBP_PSTX_SIZE_V1)+1);

    tempDW0Value = GET_DESC_FIELD(cur_txbd->TXBD_ELE[0].Dword0, 0xFFFFFFFF, 0);
    tempDW0Value = GET_DESC_FIELD(cur_txbd->TXBD_ELE[0].Dword1, 0xFFFFFFFF, 0); 

    SET_DESC_FIELD_CLR(cur_txbd->TXBD_ELE[0].Dword0, PSBLen, TXBD_DW0_PSLEN_MSK, TXBD_DW0_PSLEN_SH);

  	SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[1].Dword0, (hdrLen + payloadLen), TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
    SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[1].Dword1,
        HAL_VIRT_TO_BUS1(Adapter, (PVOID)pdesc_data->pHdr, (hdrLen + payloadLen), HAL_PCI_DMA_TODEVICE) + CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL,
        TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);

    cur_txbd->TXBD_ELE[2].Dword0 = SET_DESC(0);
    cur_txbd->TXBD_ELE[3].Dword0 = SET_DESC(0);

    _dma_cache_wback((unsigned long)((PVOID)(cur_txbd)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), 
        sizeof(TX_BUFFER_DESCRIPTOR));

    _dma_cache_wback(((GET_DESC_FIELD(cur_txbd->TXBD_ELE[0].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH)|0x80000000) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL),
        SIZE_TXDESC_88XX);

    _dma_cache_wback(((GET_DESC_FIELD(cur_txbd->TXBD_ELE[1].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH)|0x80000000) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL),
        (hdrLen + payloadLen));

    UpdateSWTXBDHostIdx88XX(Adapter, cur_q);

    return;
//}
#endif

#if CFG_HAL_TX_AMSDU
    if ( IS_SUPPORT_TX_AMSDU(Adapter) && (pdesc_data->aggreEn >= FG_AGGRE_MSDU_FIRST) ) {
        ptx_dma_amsdu  = (PHCI_TX_AMSDU_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMAAMSDU88XX);
        cur_txbd_amsdu = ptx_dma_amsdu->tx_amsdu_queue[queueIndex-1].pTXBD_head_amsdu + cur_q->host_idx;
    }

    if (pdesc_data->aggreEn <= FG_AGGRE_MSDU_FIRST)
#endif
    {
        PlatformZeroMemory(&(cur_txbd->TXBD_ELE[1]), sizeof(TXBD_ELEMENT)*(TXBD_ELE_NUM-1));
    }
    
#if CFG_HAL_TX_AMSDU
    if (pdesc_data->aggreEn <= FG_AGGRE_MSDU_FIRST)
#endif
    {

        SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[1].Dword0,
                    hdrLen, TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);

        dma_addr = HAL_VIRT_TO_BUS1(Adapter, (PVOID)pdesc_data->pHdr, hdrLen, HAL_PCI_DMA_TODEVICE);
        SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[1].Dword1,
                    dma_addr + CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL,
                    TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);
    }

#if (TXBD_ELE_NUM >= 4)

#if CFG_HAL_TX_AMSDU
    if ( IS_SUPPORT_TX_AMSDU(Adapter) && (pdesc_data->aggreEn == FG_AGGRE_MSDU_FIRST)) {
        // postpone fill this field until the last payload (i.e., FG_AGGRE_MSDU_LAST), because we don't know the exact numbers of the MSDU in this AMSDU
        //SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[2].Dword0, sizeof(TX_BUFFER_DESCRIPTOR_AMSDU), TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
        
        SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[2].Dword0,
                    1, TXBD_DW0_EXTENDTXBUF_MSK, TXBD_DW0_EXTENDTXBUF_SH);

        // (queueIndex-1) for mapping HCI_TX_DMA_QUEUE_88XX to HCI_TX_AMSDU_DMA_QUEUE_88XX
        ptx_dma_amsdu->tx_amsdu_queue[queueIndex-1].cur_txbd_element = 0;
        PlatformZeroMemory(cur_txbd_amsdu, sizeof(TX_BUFFER_DESCRIPTOR_AMSDU));

        SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[2].Dword1,
                HAL_VIRT_TO_BUS1(Adapter, (PVOID)cur_txbd_amsdu, sizeof(TX_BUFFER_DESCRIPTOR_AMSDU), HAL_PCI_DMA_TODEVICE) + CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL,
                TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);
    }
#endif // CFG_HAL_TX_AMSDU


#if CFG_HAL_TX_AMSDU
    if ( IS_SUPPORT_TX_AMSDU(Adapter) && (pdesc_data->aggreEn >= FG_AGGRE_MSDU_FIRST)) {
    // for FG_AGGRE_MSDU_FIRST, FG_AGGRE_MSDU_MIDDLE, and FG_AGGRE_MSDU_LAST
        // current AMSDU TXBD
        SET_DESC_FIELD_NO_CLR(cur_txbd_amsdu->TXBD_ELE[ptx_dma_amsdu->tx_amsdu_queue[queueIndex-1].cur_txbd_element].Dword0,
                    payloadLen, TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
        
        SET_DESC_FIELD_NO_CLR(cur_txbd_amsdu->TXBD_ELE[ptx_dma_amsdu->tx_amsdu_queue[queueIndex-1].cur_txbd_element].Dword1,
                HAL_VIRT_TO_BUS1(Adapter, (PVOID)pdesc_data->pBuf, payloadLen, HAL_PCI_DMA_TODEVICE) + CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL,
                TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);

		i=ptx_dma_amsdu->tx_amsdu_queue[queueIndex-1].cur_txbd_element;
#if defined(CONFIG_NET_PCI) && defined(NOT_RTK_BSP)
		HAL_CACHE_SYNC_WBACK(Adapter, 
		GET_DESC_FIELD(cur_txbd_amsdu->TXBD_ELE[i].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL, 
		GET_DESC_FIELD(cur_txbd_amsdu->TXBD_ELE[i].Dword0, TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH), 
		HAL_PCI_DMA_TODEVICE);
#else
	if(payloadLen)
		_dma_cache_wback(((GET_DESC_FIELD(cur_txbd_amsdu->TXBD_ELE[i].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH)|0x80000000) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL),
            payloadLen);
#endif
		
        ptx_dma_amsdu->tx_amsdu_queue[queueIndex-1].cur_txbd_element++;

        if (pdesc_data->aggreEn != FG_AGGRE_MSDU_LAST) {
            //for FG_AGGRE_MSDU_FIRST or FG_AGGRE_MSDU_MIDDLE
            return;
        }
    } else
#endif
    {
        SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[2].Dword0,
                    payloadLen, TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);

        if (payloadLen) {
            dma_addr = HAL_VIRT_TO_BUS1(Adapter, (PVOID)pdesc_data->pBuf, payloadLen, HAL_PCI_DMA_TODEVICE);
            SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[2].Dword1,
                   dma_addr + CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL,
                    TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);
        }
    }
    
    // for sw encryption: 1) WEP's icv and TKIP's icv, 2) CCMP's mic, 3) no security
    if (pdesc_data->pIcv != NULL) {
        SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[3].Dword0,
                    pdesc_data->icv,
                    TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
        
        dma_addr = HAL_VIRT_TO_BUS1(Adapter, (PVOID)pdesc_data->pIcv, pdesc_data->icv, HAL_PCI_DMA_TODEVICE);
        SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[3].Dword1,
                    dma_addr + CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL,
                    TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);

    } else if (pdesc_data->pMic != NULL) {
        SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[3].Dword0,
                    pdesc_data->mic,
                    TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
        
        dma_addr = HAL_VIRT_TO_BUS1(Adapter, (PVOID)pdesc_data->pMic, pdesc_data->mic, HAL_PCI_DMA_TODEVICE);
        SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[3].Dword1,
                    dma_addr + CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL,
                    TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);
    } else {
        cur_txbd->TXBD_ELE[3].Dword0 = SET_DESC(0);
    }
#else
    #error "Error, TXBD_ELE_NUM<4 is invalid Setting unless we modify overall architecture"
#endif  //   (TXBD_ELE_NUM >= 4) 

#if CFG_HAL_TX_AMSDU
    if ( IS_SUPPORT_TX_AMSDU(Adapter) && (pdesc_data->aggreEn == FG_AGGRE_MSDU_LAST)) {
        // fill AMSDU packet total size into TXDESC->DW0 TX_DESC_TXPKTSIZE field
        PTX_DESC_88XX ptx_desc = ((PTX_DESC_88XX)(cur_q->ptx_desc_head)) + cur_q->host_idx;
        SET_DESC_FIELD_CLR(ptx_desc->Dword0, pdesc_data->amsduLen, TX_DW0_TXPKSIZE_MSK, TX_DW0_TXPKSIZE_SH);

        // fill the exact MSDU numbers into TXBD
        SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[2].Dword0, 
            ptx_dma_amsdu->tx_amsdu_queue[queueIndex-1].cur_txbd_element * sizeof(TXBD_ELEMENT), 
            TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);

        // count total length for "dword0 Length0"
        TotalLen += GET_DESC_FIELD(cur_txbd->TXBD_ELE[0].Dword0, TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
        TotalLen += GET_DESC_FIELD(cur_txbd->TXBD_ELE[1].Dword0, TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
        
        // cur_txbd->TXBD_ELE[2].Dword1 point to the AMSDU TXBD
        for (i = 0; i < ptx_dma_amsdu->tx_amsdu_queue[queueIndex-1].cur_txbd_element; i++) {
            TotalLen += GET_DESC_FIELD(cur_txbd_amsdu->TXBD_ELE[i].Dword0, 
                        TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
        }
        TotalLen += GET_DESC_FIELD(cur_txbd->TXBD_ELE[3].Dword0, TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
    } else
#endif
    {
        // count total length for "dword0 Length0"
        for (i = 0; i < TXBDSegNum; i++) {
            TotalLen += GET_DESC_FIELD(cur_txbd->TXBD_ELE[i].Dword0, 
                        TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
        }
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
        if (pdesc_data->smhEn == TRUE) {
            //TotalLen += (HAL_HW_TXSC_HDR_CONV_OFFSET - HAL_TXDESC_OFFSET_SIZE);
            TotalLen += HAL_HW_TXSC_HDR_CONV_ADD_OFFSET;
        }
#endif		
    }

#if (IS_EXIST_RTL8192EE || IS_EXIST_RTL8197FEM)
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) || IS_HARDWARE_TYPE_8197F(Adapter) ) {
        PSBLen   = (TotalLen%PBP_PSTX_SIZE) == 0 ? (TotalLen/PBP_PSTX_SIZE):((TotalLen/PBP_PSTX_SIZE)+1);        
    }
#endif
#if (IS_EXIST_RTL8814AE || IS_EXIST_RTL8822BE) 
    if (IS_HARDWARE_TYPE_8814AE(Adapter) || IS_HARDWARE_TYPE_8822B(Adapter) ) {
        PSBLen   = (TotalLen%PBP_PSTX_SIZE_V1) == 0 ? (TotalLen/PBP_PSTX_SIZE_V1):((TotalLen/PBP_PSTX_SIZE_V1)+1);
    }
#endif
#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881A(Adapter) ) {
        if (IS_HAL_TEST_CHIP(Adapter)) {
            PSBLen   = TotalLen;
        }
        else {
            PSBLen   = (TotalLen%PBP_PSTX_SIZE) == 0 ? (TotalLen/PBP_PSTX_SIZE):((TotalLen/PBP_PSTX_SIZE)+1);                    
        }
    }
#endif
    //3 Final one HW IO of Tx Pkt
    TxPktFinalIO88XX(Adapter, cur_txbd, TxPktFinalIO88XX_WRITE, PSBLen);

#if WLAN_HAL_TXDESC_CHECK_ADDR_LEN
#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881A(Adapter) ) {
    _GET_HAL_DATA(Adapter)->cur_txbd                = cur_txbd;
#if 0
    _GET_HAL_DATA(Adapter)->cur_tx_desc_phy_addr    = cur_txbd->TXBD_ELE[0].Dword1;
    _GET_HAL_DATA(Adapter)->cur_tx_desc_len         = cur_txbd->TXBD_ELE[0].Dword0;
#endif
    _GET_HAL_DATA(Adapter)->cur_tx_psb_len          = PSBLen;
    }
#endif //IS_EXIST_RTL8881AEM
#endif // WLAN_HAL_TXDESC_CHECK_ADDR_LEN

    //4 Cache flush
#ifdef TRXBD_CACHABLE_REGION
    _dma_cache_wback((unsigned long)((PVOID)(cur_txbd)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(TX_BUFFER_DESCRIPTOR));

    _dma_cache_wback(((GET_DESC_FIELD(cur_txbd->TXBD_ELE[0].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH)|0x80000000) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL),
        TXDESCSize);


#if CFG_HAL_TX_AMSDU
    if ( IS_SUPPORT_TX_AMSDU(Adapter) ) {
        if (pdesc_data->aggreEn == FG_AGGRE_MSDU_LAST) {
            hdrLen = GET_DESC_FIELD(cur_txbd->TXBD_ELE[1].Dword0, TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
        }
    }
#endif // CFG_HAL_TX_AMSDU

    _dma_cache_wback(((GET_DESC_FIELD(cur_txbd->TXBD_ELE[1].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH)|0x80000000) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL),
        (u4Byte)hdrLen);

#if CFG_HAL_TX_AMSDU
    if ( IS_SUPPORT_TX_AMSDU(Adapter) && (pdesc_data->aggreEn == FG_AGGRE_MSDU_LAST)) {
    	_dma_cache_wback((unsigned long)((PVOID)(cur_txbd_amsdu)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(TX_BUFFER_DESCRIPTOR_AMSDU));
    }else
#endif // CFG_HAL_TX_AMSDU
    {
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
        if (pdesc_data->smhEn == TRUE) {
			if (hdrLen) {
		    	_dma_cache_wback(((GET_DESC_FIELD(cur_txbd->TXBD_ELE[1].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH)|0x80000000) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL),
		        	(u4Byte)hdrLen);
			}
            // Do nothing here ...
        } else
#endif
        {
			if (hdrLen) {
			    _dma_cache_wback(((GET_DESC_FIELD(cur_txbd->TXBD_ELE[1].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH)|0x80000000) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL),
		   		    (u4Byte)hdrLen);
			}

			if (payloadLen) {
			    _dma_cache_wback(((GET_DESC_FIELD(cur_txbd->TXBD_ELE[2].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH)|0x80000000) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL),
		   		    (u4Byte)payloadLen);
			} 

			if (pdesc_data->icv) {
				_dma_cache_wback(((GET_DESC_FIELD(cur_txbd->TXBD_ELE[3].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH)|0x80000000) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL),
		   		    pdesc_data->icv);
			} else if (pdesc_data->mic) {
				_dma_cache_wback(((GET_DESC_FIELD(cur_txbd->TXBD_ELE[3].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH)|0x80000000) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL),
		   		    pdesc_data->mic);
			}

        }
    }

// TODO:  write back icv/mic on 8198C

// TODO: consider both enable TX_AMSDU and HW_TX_SHORTCUT...

#else // !TRXBD_CACHABLE_REGION
#if defined(CONFIG_NET_PCI) && defined(NOT_RTK_BSP)
        // hdr/payload/icv/mic has sync via pci_map_single in get_physical_addr
     if (!HAL_IS_PCIBIOS_TYPE(Adapter))
#endif
     {
        if (hdrLen) {
            HAL_CACHE_SYNC_WBACK(Adapter, 
                GET_DESC_FIELD(cur_txbd->TXBD_ELE[1].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL, 
                hdrLen, 
                HAL_PCI_DMA_TODEVICE);
        }
        
        if (payloadLen) {
            HAL_CACHE_SYNC_WBACK(Adapter, 
                GET_DESC_FIELD(cur_txbd->TXBD_ELE[2].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL, 
                payloadLen, 
                HAL_PCI_DMA_TODEVICE);
        }

        if (pdesc_data->pIcv != NULL) {
            HAL_CACHE_SYNC_WBACK(Adapter, 
                GET_DESC_FIELD(cur_txbd->TXBD_ELE[3].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL, 
                pdesc_data->icv, 
                HAL_PCI_DMA_TODEVICE);
        } else if (pdesc_data->pMic != NULL) {
            HAL_CACHE_SYNC_WBACK(Adapter, 
                GET_DESC_FIELD(cur_txbd->TXBD_ELE[3].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL,
                pdesc_data->mic, 
                HAL_PCI_DMA_TODEVICE);
        }
    }
#ifdef CONFIG_NET_PCI
    if (HAL_IS_PCIBIOS_TYPE(Adapter)) {
#if CFG_HAL_TX_AMSDU
	if (pdesc_data->aggreEn == FG_AGGRE_MSDU_LAST)
		HAL_CACHE_SYNC_WBACK(Adapter, _GET_HAL_DATA(Adapter)->desc_dma_buf_addr_amsdu[queueIndex-1] + cur_q->host_idx * sizeof(TX_BUFFER_DESCRIPTOR_AMSDU), sizeof(TX_BUFFER_DESCRIPTOR_AMSDU), HAL_PCI_DMA_TODEVICE);
#endif
        HAL_CACHE_SYNC_WBACK(Adapter, _GET_HAL_DATA(Adapter)->txDesc_dma_ring_addr[queueIndex] + cur_q->host_idx * sizeof(TX_DESC_88XX), sizeof(TX_DESC_88XX), HAL_PCI_DMA_TODEVICE);
        HAL_CACHE_SYNC_WBACK(Adapter, _GET_HAL_DATA(Adapter)->txBD_dma_ring_addr[queueIndex] + cur_q->host_idx * sizeof(TX_BUFFER_DESCRIPTOR), sizeof(TX_BUFFER_DESCRIPTOR), HAL_PCI_DMA_TODEVICE);
    }
#endif
#endif // TRXBD_CACHABLE_REGION

#if 0 //eric-8822 CFG_HAL_DBG

    RT_TRACE_F(COMP_SEND, DBG_TRACE, ("\nq_idx: %d, txbd[%d], \n", queueIndex, cur_q->host_idx));

    //TXDESC
    RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%08lx, Dword1: 0x%08lx\n", 0, GET_DESC(cur_txbd->TXBD_ELE[0].Dword0), GET_DESC(cur_txbd->TXBD_ELE[0].Dword1)));
    PTX_DESC_88XX   ptx_desc = ((PTX_DESC_88XX)(cur_q->ptx_desc_head)) + cur_q->host_idx;
    RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "TXDESC:\n", ptx_desc, TXDESCSize);

    //Header
    RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%08lx, Dword1: 0x%08lx\n", 1, GET_DESC(cur_txbd->TXBD_ELE[1].Dword0), GET_DESC(cur_txbd->TXBD_ELE[1].Dword1)));
    if(pdesc_data->iv != 0) {
        RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "Header(+iv):\n", pdesc_data->pHdr, pdesc_data->hdrLen + pdesc_data->iv + pdesc_data->llcLen);
    } else {
        RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "Header:\n", pdesc_data->pHdr, pdesc_data->hdrLen + pdesc_data->llcLen);
    }

    //Payload
    RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%08lx, Dword1: 0x%08lx\n", 2, GET_DESC(cur_txbd->TXBD_ELE[2].Dword0), GET_DESC(cur_txbd->TXBD_ELE[2].Dword1)));
    RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "Payload:\n", pdesc_data->pBuf, pdesc_data->frLen);    

    //MIC or ICV
    RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%08lx, Dword1: 0x%08lx\n", 3, GET_DESC(cur_txbd->TXBD_ELE[3].Dword0), GET_DESC(cur_txbd->TXBD_ELE[3].Dword1)));
    RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "Icv:\n", pdesc_data->pIcv, pdesc_data->icv);

#endif  //CFG_HAL_DBG

    UpdateSWTXBDHostIdx88XX(Adapter, cur_q);

}


BOOLEAN
FillTxHwCtrl88XX(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      PVOID           pDescData
)
{
#if CFG_HAL_TX_AMSDU
    PTX_DESC_DATA_88XX  pdesc_data = (PTX_DESC_DATA_88XX)pDescData;
     
    if (pdesc_data->aggreEn <= FG_AGGRE_MSDU_FIRST)
#endif
    {
        GET_HAL_INTERFACE(Adapter)->FillTxDescHandler(Adapter, queueIndex, pDescData);
    }
    SetTxBufferDesc88XX(Adapter, queueIndex, pDescData);
    
    return _TRUE;
}

HAL_IMEM
BOOLEAN
QueryTxConditionMatch88XX(
    IN	HAL_PADAPTER        Adapter
)
{
    PHCI_TX_DMA_MANAGER_88XX    ptx_dma;
    HCI_TX_DMA_QUEUE_88XX       QueueIdx;
    HCI_TX_DMA_QUEUE_88XX       q_max;
    u32                         count = TX_CONDITION_MATCH_TXBD_CNT;

#if CFG_HAL_SUPPORT_MBSSID
    // excluding beacon queue...
    q_max = HCI_TX_DMA_QUEUE_HI7;
#else
    // only check MGT, VO, VI, BE, BK, HI0 queue
    q_max = HCI_TX_DMA_QUEUE_HI0;
#endif

    ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);

    for (QueueIdx = 0; QueueIdx <= q_max; QueueIdx++)
    {
        if (HAL_CIRC_CNT_RTK(ptx_dma->tx_queue[QueueIdx].host_idx, \
                    ptx_dma->tx_queue[QueueIdx].hw_idx, \
                    ptx_dma->tx_queue[QueueIdx].total_txbd_num) > count)
            return _TRUE;
    }
    return _FALSE;
}


VOID
TxPolling88XX(
    IN	HAL_PADAPTER        Adapter,
    IN	u1Byte              QueueIndex
)
{
    //QueueIndex, ex. TXPOLL_BEACON_QUEUE

    if ( TXPOLL_BEACON_QUEUE == QueueIndex ) {
        PlatformEFIOWrite2Byte(Adapter, REG_RX_RXBD_NUM, PlatformEFIORead2Byte(Adapter, REG_RX_RXBD_NUM) | BIT12);
    }
    else {
        //Do Nothing
    }
}

#if IS_EXIST_RTL8192EE || IS_EXIST_RTL8881AEM || IS_EXIST_RTL8814AE 

VOID
FillBeaconDesc88XX
(
    IN	HAL_PADAPTER        Adapter,
    IN  PVOID               _pdesc,
    IN  PVOID               data_content,
    IN  u2Byte              txLength,
    IN  BOOLEAN             bForceUpdate
)
{
    u4Byte  TXDESCSize;
    PTX_DESC_88XX       pdesc = (PTX_DESC_88XX)_pdesc;

    TXDESCSize = SIZE_TXDESC_88XX;

    PlatformZeroMemory(pdesc, TXDESCSize);

    //Dword0
    SET_DESC_FIELD_CLR(pdesc->Dword0, 1, TX_DW0_BMC_MSK, TX_DW0_BMC_SH);
    SET_DESC_FIELD_CLR(pdesc->Dword0, TXDESCSize, TX_DW0_OFFSET_MSK, TX_DW0_OFFSET_SH);
    SET_DESC_FIELD_CLR(pdesc->Dword0, txLength, TX_DW0_TXPKSIZE_MSK, TX_DW0_TXPKSIZE_SH);

    //Dword1
    SET_DESC_FIELD_CLR(pdesc->Dword1, TXDESC_QSEL_BCN, TX_DW1_QSEL_MSK, TX_DW1_QSEL_SH);


#if CFG_HAL_SUPPORT_MBSSID
        if (HAL_IS_VAP_INTERFACE(Adapter)) {
    
        // set MBSSID for each VAP_ID
      
        SET_DESC_FIELD_CLR(pdesc->Dword1, HAL_VAR_VAP_INIT_SEQ, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
        SET_DESC_FIELD_CLR(pdesc->Dword6, HAL_VAR_VAP_INIT_SEQ, TX_DW6_MBSSID_MSK, TX_DW6_MBSSID_SH);  
    
        }         
#endif //#if CFG_HAL_SUPPORT_MBSSID


    SET_DESC_FIELD_CLR(pdesc->Dword9, GetSequence(data_content), TX_DW9_SEQ_MSK, TX_DW9_SEQ_SH);

    SET_DESC_FIELD_CLR(pdesc->Dword3, 1, TX_DW3_DISDATAFB_MSK, TX_DW3_DISDATAFB_SH);
    SET_DESC_FIELD_CLR(pdesc->Dword3, 1, TX_DW3_USERATE_MSK, TX_DW3_USERATE_SH);

/*cy wang cfg p2p rm*/
    if (HAL_VAR_IS_40M_BW) {
        if (HAL_VAR_OFFSET_2ND_CHANNEL == HT_2NDCH_OFFSET_BELOW) {
            SET_DESC_FIELD_CLR(pdesc->Dword5, TXDESC_DATASC_LOWER, TX_DW5_DATA_SC_MSK, TX_DW5_DATA_SC_SH);
        }
		else {
            SET_DESC_FIELD_CLR(pdesc->Dword5, TXDESC_DATASC_UPPER, TX_DW5_DATA_SC_MSK, TX_DW5_DATA_SC_SH);
		}
	}

    //Dword4    /*cy wang cfg p2p , 6m rate beacon*//*cy wang cfg p2p*/
    #ifdef P2P_SUPPORT	// 2014-0328 use 6m rate send beacon
    if(Adapter->pmib->p2p_mib.p2p_enabled){          
        SET_DESC_FIELD_CLR(pdesc->Dword4, 4, TX_DW4_RTSRATE_MSK, TX_DW4_RTSRATE_SH);        
        SET_DESC_FIELD_CLR(pdesc->Dword4, 4, TX_DW4_DATARATE_MSK, TX_DW4_DATARATE_SH);        
    }    
    #endif   
/*cy wang cfg p2p*/
/*
		 * Intel IOT, dynamic enhance beacon tx AGC
*/

	if (Adapter->pmib->dot11StationConfigEntry.beacon_rate != 0xff)
		SET_DESC_FIELD_CLR(pdesc->Dword4, Adapter->pmib->dot11StationConfigEntry.beacon_rate, TX_DW4_DATARATE_MSK, TX_DW4_DATARATE_SH);

	if(Adapter->pmib->dot11RFEntry.bcnagc==1) {
		if(Adapter->pshare->rf_ft_var.bcn_pwr_idex+6 <= Adapter->pshare->rf_ft_var.bcn_pwr_max)			
			SET_DESC_FIELD_CLR(pdesc->Dword5, 4, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);	// +3dB
	} else if (Adapter->pmib->dot11RFEntry.bcnagc==2)  {
		if(Adapter->pshare->rf_ft_var.bcn_pwr_idex+12 <= Adapter->pshare->rf_ft_var.bcn_pwr_max) {
			SET_DESC_FIELD_CLR(pdesc->Dword5, 5, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);	// +6dB
		} else if(Adapter->pshare->rf_ft_var.bcn_pwr_idex+6 <= Adapter->pshare->rf_ft_var.bcn_pwr_max) {
			SET_DESC_FIELD_CLR(pdesc->Dword5, 4, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);	// +3dB
		}
	} else {
		if (Adapter->bcnTxAGC ==1 ) {
			SET_DESC_FIELD_CLR(pdesc->Dword5, 4, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);	// +3dB
		} else if (Adapter->bcnTxAGC ==2) {
			SET_DESC_FIELD_CLR(pdesc->Dword5, 5, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);	// +6dB
		} else {
			SET_DESC_FIELD_CLR(pdesc->Dword5, 0, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);
		}

		if (Adapter->pmib->dot11RFEntry.txpwr_reduction) {
			if (Adapter->pmib->dot11RFEntry.txpwr_reduction <= 3)
				SET_DESC_FIELD_CLR(pdesc->Dword5, Adapter->pmib->dot11RFEntry.txpwr_reduction, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);
		}
	}
	if (Adapter->pmib->dot11RFEntry.bcn2path && IS_HARDWARE_TYPE_8192EE(Adapter) )
		SET_DESC_FIELD_CLR(pdesc->Dword5, 3, TX_DW5_TX_ANT_MSK, TX_DW5_TX_ANT_SH);

    // TODO: Why ?
    HAL_VAR_IS_40M_BW_BAK   = HAL_VAR_IS_40M_BW;
    HAL_VAR_TX_BEACON_LEN   = txLength;

    SET_DESC_FIELD_CLR(pdesc->Dword7, txLength, TX_DW7_SW_TXBUFF_MSK, TX_DW7_SW_TXBUFF_SH);


#if 0   // TODO: Filen: test code ?
#if (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
    if (IS_ROOT_INTERFACE(Adapter)) {
		if (Adapter->pshare->rf_ft_var.swq_dbg	== 30) {
			pdesc->Dword9 |= set_desc((1122 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
		}
		else {
			pdesc->Dword9 |= set_desc((5566 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
		}
    }
    else {
        pdesc->Dword9 |= set_desc((GetSequence(data_content) & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);        
    }
#else
    if (Adapter->pshare->rf_ft_var.swq_dbg == 30) {
        pdesc->Dword9 |= set_desc((1122 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
    }
    else {
        pdesc->Dword9 |= set_desc((5566 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
    }
#endif  //(defined(UNIVERSAL_REPEATER) || defined(MBSSID))
#endif

    // Group Bit Control
    SET_DESC_FIELD_CLR(pdesc->Dword9, (HAL_VAR_TIM_OFFSET-24), TX_DW9_GROUPBIT_IE_OFFSET_MSK, TX_DW9_GROUPBIT_IE_OFFSET_SH);
    // Auto set bitmap control by HW
    if (HAL_OPMODE & WIFI_ADHOC_STATE) {
        SET_DESC_FIELD_CLR(pdesc->Dword9, 0, TX_DW9_GROUPBIT_IE_ENABLE_MSK, TX_DW9_GROUPBIT_IE_ENABLE_SH);
    } else {
        SET_DESC_FIELD_CLR(pdesc->Dword9, 1, TX_DW9_GROUPBIT_IE_ENABLE_MSK, TX_DW9_GROUPBIT_IE_ENABLE_SH);
    }
    // TODO: Check with Button


#ifdef TRXBD_CACHABLE_REGION
    _dma_cache_wback((unsigned long)((PVOID)(pdesc)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), TXDESCSize);
#endif //#ifdef TRXBD_CACHABLE_REGION
}


VOID
FillRsrvPageDesc88XX 
(
    IN	HAL_PADAPTER        Adapter,
    IN  PVOID               _pdesc,
    IN  PVOID               data_content,
    IN  u2Byte              txLength
)
{
	u4Byte	TXDESCSize;
	PTX_DESC_88XX		pdesc = (PTX_DESC_88XX)_pdesc;

	TXDESCSize = SIZE_TXDESC_88XX;

	PlatformZeroMemory(pdesc, TXDESCSize);

	//Dword0
	//SET_DESC_FIELD_CLR(pdesc->Dword0, 1, TX_DW0_BMC_MSK, TX_DW0_BMC_SH);
	SET_DESC_FIELD_CLR(pdesc->Dword0, TXDESCSize, TX_DW0_OFFSET_MSK, TX_DW0_OFFSET_SH);
	SET_DESC_FIELD_CLR(pdesc->Dword0, txLength, TX_DW0_TXPKSIZE_MSK, TX_DW0_TXPKSIZE_SH);

	//Dword1
	SET_DESC_FIELD_CLR(pdesc->Dword1, TXDESC_QSEL_MGT, TX_DW1_QSEL_MSK, TX_DW1_QSEL_SH);


#if 0//CFG_HAL_SUPPORT_MBSSID
		if (HAL_IS_VAP_INTERFACE(Adapter)) {
	
		// set MBSSID for each VAP_ID
	  
		SET_DESC_FIELD_CLR(pdesc->Dword1, HAL_VAR_VAP_INIT_SEQ, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
		SET_DESC_FIELD_CLR(pdesc->Dword6, HAL_VAR_VAP_INIT_SEQ, TX_DW6_MBSSID_MSK, TX_DW6_MBSSID_SH);  
	
		}		  
#endif //#if CFG_HAL_SUPPORT_MBSSID


	SET_DESC_FIELD_CLR(pdesc->Dword9, GetSequence(data_content), TX_DW9_SEQ_MSK, TX_DW9_SEQ_SH);

	SET_DESC_FIELD_CLR(pdesc->Dword3, 1, TX_DW3_DISDATAFB_MSK, TX_DW3_DISDATAFB_SH);
	SET_DESC_FIELD_CLR(pdesc->Dword3, 1, TX_DW3_USERATE_MSK, TX_DW3_USERATE_SH);
#if 0
/*cy wang cfg p2p rm*/
	if (HAL_VAR_IS_40M_BW) {
		if (HAL_VAR_OFFSET_2ND_CHANNEL == HT_2NDCH_OFFSET_BELOW) {
			SET_DESC_FIELD_CLR(pdesc->Dword5, TXDESC_DATASC_LOWER, TX_DW5_DATA_SC_MSK, TX_DW5_DATA_SC_SH);
		}
		else {
			SET_DESC_FIELD_CLR(pdesc->Dword5, TXDESC_DATASC_UPPER, TX_DW5_DATA_SC_MSK, TX_DW5_DATA_SC_SH);
		}
	}

	//Dword4	/*cy wang cfg p2p , 6m rate beacon*//*cy wang cfg p2p*/
#ifdef P2P_SUPPORT	// 2014-0328 use 6m rate send beacon
	if(Adapter->pmib->p2p_mib.p2p_enabled){ 		 
		SET_DESC_FIELD_CLR(pdesc->Dword4, 4, TX_DW4_RTSRATE_MSK, TX_DW4_RTSRATE_SH);		
		SET_DESC_FIELD_CLR(pdesc->Dword4, 4, TX_DW4_DATARATE_MSK, TX_DW4_DATARATE_SH);		  
	}	 
#endif   
/*cy wang cfg p2p*/
#endif
/*
		 * Intel IOT, dynamic enhance beacon tx AGC
*/

	if (Adapter->pmib->dot11StationConfigEntry.prsp_rate != 0xff)
		SET_DESC_FIELD_CLR(pdesc->Dword4, Adapter->pmib->dot11StationConfigEntry.prsp_rate, TX_DW4_DATARATE_MSK, TX_DW4_DATARATE_SH);
#if 0

	if(Adapter->pmib->dot11RFEntry.bcnagc==1) {
		if(Adapter->pshare->rf_ft_var.bcn_pwr_idex+6 <= Adapter->pshare->rf_ft_var.bcn_pwr_max) 		
			SET_DESC_FIELD_CLR(pdesc->Dword5, 4, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);	// +3dB
	} else if (Adapter->pmib->dot11RFEntry.bcnagc==2)  {
		if(Adapter->pshare->rf_ft_var.bcn_pwr_idex+12 <= Adapter->pshare->rf_ft_var.bcn_pwr_max) {
			SET_DESC_FIELD_CLR(pdesc->Dword5, 5, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);	// +6dB
		} else if(Adapter->pshare->rf_ft_var.bcn_pwr_idex+6 <= Adapter->pshare->rf_ft_var.bcn_pwr_max) {
			SET_DESC_FIELD_CLR(pdesc->Dword5, 4, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);	// +3dB
		}
	} else {
		if (Adapter->bcnTxAGC ==1 ) {
			SET_DESC_FIELD_CLR(pdesc->Dword5, 4, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);	// +3dB
		} else if (Adapter->bcnTxAGC ==2) {
			SET_DESC_FIELD_CLR(pdesc->Dword5, 5, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);	// +6dB
		} else {
			SET_DESC_FIELD_CLR(pdesc->Dword5, 0, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);
		}

		if (Adapter->pmib->dot11RFEntry.txpwr_reduction) {
			if (Adapter->pmib->dot11RFEntry.txpwr_reduction <= 3)
				SET_DESC_FIELD_CLR(pdesc->Dword5, Adapter->pmib->dot11RFEntry.txpwr_reduction, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);
		}
	}
#endif
	if (Adapter->pmib->dot11RFEntry.bcn2path && IS_HARDWARE_TYPE_8192EE(Adapter) )
		SET_DESC_FIELD_CLR(pdesc->Dword5, 3, TX_DW5_TX_ANT_MSK, TX_DW5_TX_ANT_SH);
#if 0
	// TODO: Why ?
	HAL_VAR_IS_40M_BW_BAK	= HAL_VAR_IS_40M_BW;
	HAL_VAR_TX_BEACON_LEN	= txLength;
#endif

	SET_DESC_FIELD_CLR(pdesc->Dword7, txLength, TX_DW7_SW_TXBUFF_MSK, TX_DW7_SW_TXBUFF_SH);


#if 0   // TODO: Filen: test code ?
#if (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
	if (IS_ROOT_INTERFACE(Adapter)) {
		if (Adapter->pshare->rf_ft_var.swq_dbg	== 30) {
			pdesc->Dword9 |= set_desc((1122 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
		}
		else {
			pdesc->Dword9 |= set_desc((5566 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
		}
	}
	else {
		pdesc->Dword9 |= set_desc((GetSequence(data_content) & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);		  
	}
#else
	if (Adapter->pshare->rf_ft_var.swq_dbg == 30) {
		pdesc->Dword9 |= set_desc((1122 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
	}
	else {
		pdesc->Dword9 |= set_desc((5566 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
	}
#endif  //(defined(UNIVERSAL_REPEATER) || defined(MBSSID))
#endif

#if 0
	// Group Bit Control
	SET_DESC_FIELD_CLR(pdesc->Dword9, (HAL_VAR_TIM_OFFSET-24), TX_DW9_GROUPBIT_IE_OFFSET_MSK, TX_DW9_GROUPBIT_IE_OFFSET_SH);
	// Auto set bitmap control by HW
	if (HAL_OPMODE & WIFI_ADHOC_STATE) {
		SET_DESC_FIELD_CLR(pdesc->Dword9, 0, TX_DW9_GROUPBIT_IE_ENABLE_MSK, TX_DW9_GROUPBIT_IE_ENABLE_SH);
	} else {
		SET_DESC_FIELD_CLR(pdesc->Dword9, 1, TX_DW9_GROUPBIT_IE_ENABLE_MSK, TX_DW9_GROUPBIT_IE_ENABLE_SH);
	}
#endif
	// TODO: Check with Button


#ifdef TRXBD_CACHABLE_REGION
	_dma_cache_wback((unsigned long)((PVOID)(pdesc)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), TXDESCSize);
#endif //#ifdef TRXBD_CACHABLE_REGION
}

#endif //#if IS_EXIST_RTL8192EE || IS_EXIST_RTL8881AEM || IS_EXIST_RTL8814AE 

VOID
GetBeaconTXBDTXDESC88XX(
    IN	HAL_PADAPTER                Adapter,
    OUT PTX_BUFFER_DESCRIPTOR       *pTXBD,
    OUT PTX_DESC_88XX               *ptx_desc
)
{
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    u4Byte                          TXBDBeaconOffset;

    //3 Get TXBD PTR & Get TXDESC PTR
#if (IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM || IS_EXIST_RTL8822BE)
    if (IS_HARDWARE_TYPE_8192E(Adapter) || IS_HARDWARE_TYPE_8814A(Adapter) || IS_HARDWARE_TYPE_8197F(Adapter) || IS_HARDWARE_TYPE_8822B(Adapter)) {    
        TXBDBeaconOffset = TXBD_BEACON_OFFSET_V1;        
    }
#endif  //IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM
    
#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881A(Adapter) ) {
        TXBDBeaconOffset = TXBD_BEACON_OFFSET_V2;        
    }
#endif  //IS_EXIST_RTL8881AEM

    #if CFG_HAL_DBG
    //Error Check
    if ( TXBDBeaconOffset % sizeof(TX_BUFFER_DESCRIPTOR) != 0 ) {
        RT_TRACE(COMP_SEND, DBG_SERIOUS, ("TXBDBeaconOffset is mismatched\n")); 
        return;
    }
    #endif  //CFG_HAL_DBG
    
#if CFG_HAL_SUPPORT_MBSSID
    if (HAL_IS_VAP_INTERFACE(Adapter)) {
        *pTXBD       = (PTX_BUFFER_DESCRIPTOR)((dma_addr_t)ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].pTXBD_head +
                (HAL_VAR_VAP_INIT_SEQ * TXBDBeaconOffset));

        *ptx_desc    = (PTX_DESC_88XX)((dma_addr_t)ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].ptx_desc_head + 
                    HAL_VAR_VAP_INIT_SEQ*sizeof(TX_DESC_88XX));
    } else {
        *pTXBD       = ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].pTXBD_head;
        *ptx_desc    = (PTX_DESC_88XX)ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].ptx_desc_head;
    }
#else
    *pTXBD       = ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].pTXBD_head;
    *ptx_desc    = (PTX_DESC_88XX)ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].ptx_desc_head;
#endif

}

VOID
SetBeaconDownload88XX (
    IN	HAL_PADAPTER        Adapter,
    IN  u4Byte              Value
) 
{
    PTX_BUFFER_DESCRIPTOR       pTXBD;
    PTX_DESC_88XX               ptx_desc;
	u4Byte						TXBDBeaconOffset;
	//3 Get TXBD PTR & Get TXDESC PTR
#if IS_RTL8192E_SERIES || IS_RTL8814A_SERIES || IS_RTL8197F_SERIES || IS_RTL8822B_SERIES
    if (IS_HARDWARE_TYPE_8192E(Adapter) || IS_HARDWARE_TYPE_8814A(Adapter) || IS_HARDWARE_TYPE_8197F(Adapter) || IS_HARDWARE_TYPE_8822B(Adapter)) {    	
		TXBDBeaconOffset = TXBD_BEACON_OFFSET_V1;		
	}
#endif  //IS_RTL8192E_SERIES || IS_RTL8814A_SERIES || IS_RTL8197F_SERIES
	
#if IS_RTL8881A_SERIES
	if ( IS_HARDWARE_TYPE_8881A(Adapter) ) {
		TXBDBeaconOffset = TXBD_BEACON_OFFSET_V2;		
	}
#endif  //IS_RTL8881A_SERIES

    GetBeaconTXBDTXDESC88XX(Adapter, &pTXBD, &ptx_desc);

    switch(Value) {
        case HW_VAR_BEACON_ENABLE_DOWNLOAD:
            SET_DESC_FIELD_CLR(pTXBD->TXBD_ELE[0].Dword0, 1, TXBD_DW0_BCN_OWN_MSK, TXBD_DW0_BCN_OWN_SH);
            break;
        case HW_VAR_BEACON_DISABLE_DOWNLOAD:
            SET_DESC_FIELD_CLR(pTXBD->TXBD_ELE[0].Dword0, 0, TXBD_DW0_BCN_OWN_MSK, TXBD_DW0_BCN_OWN_SH);
            break;
        default:
            RT_TRACE(COMP_BEACON, DBG_SERIOUS, ("SetBeaconDownload88XX setting error: 0x%x \n", Value));
            break;
    }

    //write back cache: TXBD
#ifdef CONFIG_NET_PCI
    if (HAL_IS_PCIBIOS_TYPE(Adapter)) {
        u4Byte uiTmp=0;
#if CFG_HAL_SUPPORT_MBSSID
        if (HAL_IS_VAP_INTERFACE(Adapter)) {
            uiTmp=HAL_VAR_VAP_INIT_SEQ * TXBDBeaconOffset;
        }
#endif
        HAL_CACHE_SYNC_WBACK(Adapter, _GET_HAL_DATA(Adapter)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_BCN] + uiTmp, sizeof(TX_BUFFER_DESCRIPTOR), HAL_PCI_DMA_TODEVICE);
    } else 
 #endif
    {
#ifdef TRXBD_CACHABLE_REGION
        _dma_cache_wback((unsigned long)((PVOID)(&(pTXBD->TXBD_ELE[0].Dword0))-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), 
            sizeof(pTXBD->TXBD_ELE[0].Dword0));
#else
        HAL_CACHE_SYNC_WBACK(Adapter, HAL_VIRT_TO_BUS1(Adapter, (PVOID)pTXBD, sizeof(TX_BUFFER_DESCRIPTOR), HAL_PCI_DMA_TODEVICE),
                        sizeof(TX_BUFFER_DESCRIPTOR), HAL_PCI_DMA_TODEVICE);
#endif //#ifdef TRXBD_CACHABLE_REGION
    }
}

VOID
SigninBeaconTXBD88XX
(
    IN	HAL_PADAPTER        Adapter,
    IN  pu4Byte             beaconbuf,
    IN  u2Byte              frlen
)
{
    PTX_BUFFER_DESCRIPTOR           pTXBD;
    PTX_DESC_88XX                   ptx_desc;
    u4Byte                          TotalLen;
    u4Byte                          PSBLen;
    u4Byte                          TXDESCSize;
    
    GetBeaconTXBDTXDESC88XX(Adapter, &pTXBD, &ptx_desc);

#if (IS_EXIST_RTL8881AEM || IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM)
    if ( IS_HARDWARE_TYPE_8881A(Adapter) || IS_HARDWARE_TYPE_8192EE(Adapter) || IS_HARDWARE_TYPE_8814AE(Adapter) || IS_HARDWARE_TYPE_8197F(Adapter) ) {
        TXDESCSize = SIZE_TXDESC_88XX;
    }
#endif  //#if (IS_EXIST_RTL8881AEM || IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM)

#if IS_EXIST_RTL8822BE
    if ( IS_HARDWARE_TYPE_8822B(Adapter))  {
        TXDESCSize = SIZE_TXDESC_88XX_V1;
    }
#endif //IS_EXIST_RTL8822BE


#if 0 // CFG_HAL_DBG
        //TXDESC
        RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%lx, Dword1: 0x%lx\n", 0, GET_DESC(pTXBD->TXBD_ELE[0].Dword0), GET_DESC(pTXBD->TXBD_ELE[0].Dword1)));
        RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "TXDESC:\n", ptx_desc, TXDESCSize);

        //Header + Payload
        RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%lx, Dword1: 0x%lx\n", 1, GET_DESC(pTXBD->TXBD_ELE[1].Dword0), GET_DESC(pTXBD->TXBD_ELE[1].Dword1)));
        RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "Content:\n", beaconbuf, frlen);

        RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%lx, Dword1: 0x%lx\n", 2, GET_DESC(pTXBD->TXBD_ELE[2].Dword0), GET_DESC(pTXBD->TXBD_ELE[2].Dword1)));
        RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%lx, Dword1: 0x%lx\n\n\n", 3, GET_DESC(pTXBD->TXBD_ELE[3].Dword0), GET_DESC(pTXBD->TXBD_ELE[3].Dword1)));
#if 0
        DumpTxPktBuf(Adapter);

        PHCI_RX_DMA_MANAGER_88XX prx_dma = (PHCI_RX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PRxDMA88XX);
        PHCI_TX_DMA_MANAGER_88XX ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);

        //No Beacon
        PTX_BUFFER_DESCRIPTOR ptxbd_head = (PTX_BUFFER_DESCRIPTOR)(prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].pRXBD_head +
        prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].total_rxbd_num);

        PTX_DESC_88XX ptx_desc_head = (PTX_DESC_88XX)((pu1Byte)ptxbd_head +
        sizeof(TX_BUFFER_DESCRIPTOR) * TOTAL_NUM_TXBD_NO_BCN);

        PTX_BUFFER_DESCRIPTOR ptxbd_bcn_head  = (PTX_BUFFER_DESCRIPTOR)((pu1Byte)ptx_desc_head +
        TXDESCSize * TOTAL_NUM_TXBD_NO_BCN);

        PTX_DESC_88XX ptxdesc_bcn_head = (PTX_DESC_88XX)((pu1Byte)ptxbd_bcn_head +
          (1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_V1);
          
        PTX_BUFFER_DESCRIPTOR ptxbd_bcn_cur;

        PTX_BUFFER_DESCRIPTOR ptxbd = ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].pTXBD_head;
        PTX_DESC_88XX ptx_desc      = ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].ptx_desc_head;
        u4Byte i;

        for (i = 0; i < 1+HAL_NUM_VWLAN; i++)
        {
           ptxbd_bcn_cur = (pu1Byte)ptxbd + TXBD_BEACON_OFFSET_V1 * i;
               RT_TRACE_F(COMP_INIT, DBG_TRACE, ("ptxbd_bcn[%ld]: Dword0: 0x%lx, Dword1: 0x%lx\n",
                          i,
                          (u4Byte)GET_DESC(ptxbd_bcn_cur->TXBD_ELE[0].Dword0),
                          (u4Byte)GET_DESC(ptxbd_bcn_cur->TXBD_ELE[0].Dword1)
                          ));             
        }
#endif
#endif

#if 0   // TODO: Filen
#ifdef DFS
        if (!priv->pmib->dot11DFSEntry.disable_DFS &&
            (timer_pending(&GET_ROOT(priv)->ch_avail_chk_timer))) {
            pdesc->Dword0 &= set_desc(~(TX_OWN));
            RTL_W16(PCIE_CTRL_REG, RTL_R16(PCIE_CTRL_REG)| (BCNQSTOP));
    
            return;
        }
#endif
#endif

    GET_HAL_INTERFACE(Adapter)->FillBeaconDescHandler(Adapter, ptx_desc, (PVOID)beaconbuf, frlen, _FALSE);
    //FillBeaconDesc88XX(Adapter, ptx_desc, (PVOID)beaconbuf, frlen, _FALSE);

    //Segment 1: Payload
    SET_DESC_FIELD_CLR(pTXBD->TXBD_ELE[1].Dword0, frlen, TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
    SET_DESC_FIELD_CLR(pTXBD->TXBD_ELE[1].Dword1,
                HAL_VIRT_TO_BUS1(Adapter, (PVOID)beaconbuf, frlen, HAL_PCI_DMA_TODEVICE) + CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL,
                TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);

    //Segment 0: Wifi Info
    //PrepareTxDesc88XX has done    
    TotalLen = TXDESCSize + frlen;

#if IS_EXIST_RTL8192EE || IS_EXIST_RTL8197FEM 
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) || IS_HARDWARE_TYPE_8197F(Adapter)) {
        PSBLen   = (TotalLen%PBP_PSTX_SIZE) == 0 ? (TotalLen/PBP_PSTX_SIZE):((TotalLen/PBP_PSTX_SIZE)+1);        
    }
#endif //IS_EXIST_RTL8192EE || IS_EXIST_RTL8197FEM

#if IS_EXIST_RTL8814AE || IS_RTL8822B_SERIES
    if ( IS_HARDWARE_TYPE_8814AE(Adapter)|| IS_HARDWARE_TYPE_8822B(Adapter)) {
        PSBLen   = (TotalLen%PBP_PSTX_SIZE_V1) == 0 ? (TotalLen/PBP_PSTX_SIZE_V1):((TotalLen/PBP_PSTX_SIZE_V1)+1);        
    }
#endif //IS_EXIST_RTL8814AE

#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881A(Adapter) ) {
        PSBLen   = TotalLen;
    }
#endif

    SET_DESC_FIELD_CLR(pTXBD->TXBD_ELE[0].Dword0, PSBLen, TXBD_DW0_BCN_PSLEN_MSK, TXBD_DW0_BCN_PSLEN_SH);

    pTXBD->TXBD_ELE[2].Dword0 = SET_DESC(0);
    pTXBD->TXBD_ELE[2].Dword1 = SET_DESC(0);
    pTXBD->TXBD_ELE[3].Dword0 = SET_DESC(0);    
    pTXBD->TXBD_ELE[3].Dword1 = SET_DESC(0);

    //3 Write Cache Sync Back
#if 0
    static int ki = 0;
    if(ki < 10)
    {
        RT_TRACE_F(COMP_SEND, DBG_TRACE, ("pTXBD = %x \n", pTXBD));            
        RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "Beacon TXBD\n", pTXBD, 40);
    
        RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%lx, Dword1: 0x%lx\n", 0, GET_DESC(pTXBD->TXBD_ELE[0].Dword0), GET_DESC(pTXBD->TXBD_ELE[0].Dword1)));
        RT_TRACE_F(COMP_SEND, DBG_TRACE, ("ptx_desc = %x,beaconbuf = %x \n", ptx_desc,beaconbuf));        
        RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "TXDESC:\n", ptx_desc, TXDESCSize);

        //Header + Payload
        RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%lx, Dword1: 0x%lx\n", 1, GET_DESC(pTXBD->TXBD_ELE[1].Dword0), GET_DESC(pTXBD->TXBD_ELE[1].Dword1)));
        RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "Content:\n", beaconbuf, frlen);

        ki++;
    }
        //RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%lx, Dword1: 0x%lx\n", 2, GET_DESC(pTXBD->TXBD_ELE[2].Dword0), GET_DESC(pTXBD->TXBD_ELE[2].Dword1)));
        //RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%lx, Dword1: 0x%lx\n\n\n", 3, GET_DESC(pTXBD->TXBD_ELE[3].Dword0), GET_DESC(pTXBD->TXBD_ELE[3].Dword1)));
#endif
        
#ifdef TRXBD_CACHABLE_REGION
    _dma_cache_wback((unsigned long)((PVOID)(pTXBD)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(TX_BUFFER_DESCRIPTOR));

    _dma_cache_wback(((GET_DESC_FIELD(pTXBD->TXBD_ELE[0].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH)|0x80000000) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL),
        TXDESCSize);

    _dma_cache_wback(((GET_DESC_FIELD(pTXBD->TXBD_ELE[1].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH)|0x80000000) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL),
        (u4Byte)frlen);
#else
    //write back cache: TXDESC    
    HAL_CACHE_SYNC_WBACK(Adapter,
        GET_DESC_FIELD(pTXBD->TXBD_ELE[0].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL,
        TXDESCSize, HAL_PCI_DMA_TODEVICE);
    
    //write back cache: Payload    
    HAL_CACHE_SYNC_WBACK(Adapter,
        GET_DESC_FIELD(pTXBD->TXBD_ELE[1].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH) - CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL,
        (u4Byte)frlen, HAL_PCI_DMA_TODEVICE);
#endif //#ifdef TRXBD_CACHABLE_REGION
}



u2Byte
GetTxQueueHWIdx88XX
(
    IN	HAL_PADAPTER        Adapter,
    IN  u4Byte              q_num       //enum _TX_QUEUE_
)
{
    PHCI_TX_DMA_MANAGER_88XX    ptx_dma;

    ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);

    return (BIT_MASK_QUEUE_IDX &
        (HAL_RTL_R32(ptx_dma->tx_queue[MappingTxQueue88XX(Adapter, q_num)].reg_rwptr_idx)>>BIT_SHIFT_QUEUE_HW_IDX));    
}

#if CFG_HAL_TX_SHORTCUT

#if 0
PVOID
GetShortCutTxDesc88XX(
    IN  HAL_PADAPTER    Adapter
)
{
    // TODO: pre-allocate a TXDESC pool when system startup
    return (PVOID)HALMalloc(Adapter, sizeof(TX_DESC_88XX));
}

VOID
ReleaseShortCutTxDesc88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc
)
{
    // TODO: right ?, release to the TXDESC pool
    HAL_free(pTxDesc);
}
#endif

/**
 * direction: 
 *      1) 0x01: store current TXBD's txdesc to driver layer
 *      2) 0x02: copy backup txdesc from driver layer to current TXBD's TXDESC
 */
HAL_IMEM
PVOID
CopyShortCutTxDesc88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex, //HCI_TX_DMA_QUEUE_88XX    
    IN  PVOID           pTxDesc,
    IN  u4Byte          direction
)
{
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PTX_DESC_88XX                   cur_txdesc;
    u4Byte                          TXDESCSize;
#if 0// CFG_HAL_DBG
    PTX_BUFFER_DESCRIPTOR           cur_txbd;
#endif // CFG_HAL_DBG

    ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    cur_q       = &(ptx_dma->tx_queue[queueIndex]);
    cur_txdesc  = (PTX_DESC_88XX)cur_q->ptx_desc_head + cur_q->host_idx;

#if (IS_EXIST_RTL8881AEM || IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM)
    if ( IS_HARDWARE_TYPE_8881A(Adapter) || IS_HARDWARE_TYPE_8192EE(Adapter) || IS_HARDWARE_TYPE_8814AE(Adapter) || IS_HARDWARE_TYPE_8197F(Adapter) ) {
        TXDESCSize = SIZE_TXDESC_88XX;
    }
#endif  //#if (IS_EXIST_RTL8881AEM || IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM)

#if IS_EXIST_RTL8822BE
    if ( IS_HARDWARE_TYPE_8822B(Adapter))  {
        TXDESCSize = SIZE_TXDESC_88XX_V1;
    }
#endif //IS_EXIST_RTL8822BE

#if 0 // CFG_HAL_DBG
    cur_txbd    = cur_q->pTXBD_head + cur_q->host_idx;

    if (HAL_VIRT_TO_BUS((u4Byte)cur_txdesc) != GET_DESC(cur_txbd->TXBD_ELE[0].Dword1)) {
        printk("%s(%d): cur_txdesc: 0x%08x, cur_txbd[0].Dword1: 0x%08x \n", __FUNCTION__, __LINE__,
            HAL_VIRT_TO_BUS((u4Byte)cur_txdesc), GET_DESC(cur_txbd->TXBD_ELE[0].Dword1));
    }    
#endif // CFG_HAL_DBG

    if (0x01 == direction) {
        HAL_memcpy(pTxDesc, cur_txdesc, TXDESCSize);
    } else { // 0x02 == direction
        HAL_memcpy(cur_txdesc, pTxDesc, TXDESCSize);
    }

    return cur_txdesc;
}


#if IS_EXIST_RTL8192EE || IS_EXIST_RTL8881AEM || IS_EXIST_RTL8814AE 

HAL_IMEM
VOID
SetShortCutTxBuffSize88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc,
    IN  u2Byte          txBuffSize
)
{
    PTX_DESC_88XX   ptx_desc = (PTX_DESC_88XX) pTxDesc;
    SET_DESC_FIELD_CLR(ptx_desc->Dword7, txBuffSize, TX_DW7_SW_TXBUFF_MSK, TX_DW7_SW_TXBUFF_SH);
}

HAL_IMEM
u2Byte
GetShortCutTxBuffSize88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc
)
{
    PTX_DESC_88XX   ptx_desc = (PTX_DESC_88XX) pTxDesc;
    return  (u2Byte)GET_DESC_FIELD(ptx_desc->Dword7, TX_DW7_SW_TXBUFF_MSK, TX_DW7_SW_TXBUFF_SH);
}


HAL_IMEM
VOID
FillShortCutTxDesc88XX(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      PVOID           pDescData,
    IN      PVOID           pTxDesc
)
{
    PTX_DESC_DATA_88XX  pdesc_data  = (PTX_DESC_DATA_88XX)pDescData;
    PTX_DESC_88XX       ptx_desc    = (PTX_DESC_88XX)pTxDesc;
   
    // tx shortcut can reuse TXDESC while 1) no security or 2) hw security
    // if no security iv == 0, so adding iv is ok for no security and hw security
    u2Byte  TX_DESC_TXPKTSIZE   = pdesc_data->hdrLen + pdesc_data->llcLen + pdesc_data->frLen + pdesc_data->iv;
    BOOLEAN TX_DESC_BK          = pdesc_data->bk;
    BOOLEAN TX_DESC_NAVUSEHDR   = pdesc_data->navUseHdr;
    u2Byte  TX_DESC_SEQ         = GetSequence(pdesc_data->pHdr);
    u1Byte  TX_DESC_DATA_STBC   = pdesc_data->dataStbc;
    BOOLEAN TX_DESC_RTY_LMT_EN  = pdesc_data->rtyLmtEn;   
    u1Byte  TX_DESC_DATA_RT_LMT = pdesc_data->dataRtyLmt;
    u4Byte  TXDESCSize = SIZE_TXDESC_88XX;
    //Dword 6
#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
    u1Byte  TX_DESC_ANTSEL			 = pdesc_data->antSel;	
    u1Byte  TX_DESC_ANTSEL_A			 = pdesc_data->antSel_A;
    u1Byte  TX_DESC_ANTSEL_B			 = pdesc_data->antSel_B;
    u1Byte  TX_DESC_ANTSEL_C			 = pdesc_data->antSel_C;
#endif	//#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))

    SET_DESC_FIELD_CLR(ptx_desc->Dword0, TX_DESC_TXPKTSIZE, TX_DW0_TXPKSIZE_MSK, TX_DW0_TXPKSIZE_SH);
    SET_DESC_FIELD_CLR(ptx_desc->Dword2, TX_DESC_BK, TX_DW2_BK_MSK, TX_DW2_BK_SH);
    SET_DESC_FIELD_CLR(ptx_desc->Dword3, TX_DESC_NAVUSEHDR, TX_DW3_NAVUSEHDR_MSK, TX_DW3_NAVUSEHDR_SH);
    SET_DESC_FIELD_CLR(ptx_desc->Dword9, TX_DESC_SEQ, TX_DW9_SEQ_MSK, TX_DW9_SEQ_SH);

    if (TX_DESC_RTY_LMT_EN) {
        SET_DESC_FIELD_CLR(ptx_desc->Dword4, TX_DESC_RTY_LMT_EN, TX_DW4_RTY_LMT_EN_MSK, TX_DW4_RTY_LMT_EN_SH);
        SET_DESC_FIELD_CLR(ptx_desc->Dword4, TX_DESC_DATA_RT_LMT, TX_DW4_DATA_RT_LMT_MSK, TX_DW4_DATA_RT_LMT_SH);
    } else if (TX_DESC_DATA_RT_LMT) {
        SET_DESC_FIELD_CLR(ptx_desc->Dword4, 0, TX_DW4_RTY_LMT_EN_MSK, TX_DW4_RTY_LMT_EN_SH);
        SET_DESC_FIELD_CLR(ptx_desc->Dword4, 0, TX_DW4_DATA_RT_LMT_MSK, TX_DW4_DATA_RT_LMT_SH);
    }

    // for force tx rate
    if (HAL_VAR_TX_FORCE_RATE != 0xff) {
        SET_DESC_FIELD_CLR(ptx_desc->Dword3, pdesc_data->useRate, TX_DW3_USERATE_MSK, TX_DW3_USERATE_SH);
        SET_DESC_FIELD_CLR(ptx_desc->Dword3, pdesc_data->disRTSFB, TX_DW3_DISRTSFB_MSK, TX_DW3_DISRTSFB_SH);
        SET_DESC_FIELD_CLR(ptx_desc->Dword3, pdesc_data->disDataFB, TX_DW3_DISDATAFB_MSK, TX_DW3_DISDATAFB_SH);
        SET_DESC_FIELD_CLR(ptx_desc->Dword4, pdesc_data->dataRate, TX_DW4_DATARATE_MSK, TX_DW4_DATARATE_SH);
    }

#if (BEAMFORMING_SUPPORT == 1)
	SET_DESC_FIELD_CLR(ptx_desc->Dword5, TX_DESC_DATA_STBC, TX_DW5_DATA_STBC_MSK, TX_DW5_DATA_STBC_SH);
#endif

#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
   if(TX_DESC_ANTSEL & BIT(0))
	SET_DESC_FIELD_CLR(ptx_desc->Dword6, TX_DESC_ANTSEL_A, TX_DW6_ANTSEL_A_MSK, TX_DW6_ANTSEL_A_SH);
   if(TX_DESC_ANTSEL & BIT(1))
	SET_DESC_FIELD_CLR(ptx_desc->Dword6, TX_DESC_ANTSEL_B, TX_DW6_ANTSEL_B_MSK, TX_DW6_ANTSEL_B_SH);
   if(TX_DESC_ANTSEL & BIT(2))
	SET_DESC_FIELD_CLR(ptx_desc->Dword6, TX_DESC_ANTSEL_C, TX_DW6_ANTSEL_C_MSK, TX_DW6_ANTSEL_C_SH);
#endif	//#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))

#ifdef TRXBD_CACHABLE_REGION
    _dma_cache_wback((unsigned long)((PVOID)ptx_desc-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), TXDESCSize);
#endif //#ifdef TRXBD_CACHABLE_REGION
}

#if CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
HAL_IMEM
VOID
FillHwShortCutTxDesc88XX (
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pDescData
)
{
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PTX_DESC_88XX                   ptx_desc;
    PTX_DESC_DATA_88XX              pdesc_data = (PTX_DESC_DATA_88XX)pDescData;
    u4Byte                          TXDESCSize;

    TXDESCSize = SIZE_TXDESC_88XX;          

	// Dword 0
	u2Byte  TX_DESC_TXPKTSIZE		= pdesc_data->hdrLen + pdesc_data->llcLen + pdesc_data->frLen + pdesc_data->iv;
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    u1Byte	TX_DESC_OFFSET			= ((pdesc_data->smhEn == TRUE) ? (HAL_HW_TXSC_HDR_CONV_ADD_OFFSET+TXDESCSize) : TXDESCSize);
#else
	u1Byte	TX_DESC_OFFSET			= TXDESCSize;
#endif

	// Dword 1
	u1Byte  TX_DESC_MACID       	= pdesc_data->macId;
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    u1Byte  TX_DESC_PKT_OFFSET      = 0; // unit: 8 bytes. Early mode: 1 units, (8 * 1 = 8 bytes for early mode info)
#endif

	// Dword 3
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    // Actually, no any conditions can run the (pdesc_data->hdrLen >> 1)  case in our driver.
    // Because, while calling this function, it must enable reuse TXDESC and hdr conv.
	u1Byte	TX_DESC_WHEADER_LEN		= ((pdesc_data->smhEn == TRUE) ? HAL_HW_TXSC_WHEADER_LEN : (pdesc_data->hdrLen >> 1)); // unit: 2 bytes
#endif // CFG_HAL_HW_TX_SHORTCUT_HDR_CONV

	// Dword 8
	//BOOLEAN	TX_DESC_STW_ANT_DIS		= pdesc_data->stwAntDis;
	//BOOLEAN	TX_DESC_STW_RATE_DIS	= pdesc_data->stwRateDis;
	//BOOLEAN	TX_DESC_STW_RB_DIS		= pdesc_data->stwRbDis;
	//BOOLEAN	TX_DESC_STW_PKTRE_DIS 	= pdesc_data->stwPktReDis;
#if 0
3.)	STW_ANT_DIS:  
ant_mapA, ant_mapB, ant_mapC, ant_mapD, ANTSEL_A, ANTSEL_B, Ntx_map, TXPWR_OFFSET 
4.)	STW_RATE_DIS:  
USE_RATE, Data rate, DATA_SHORT, DATA_BW, TRY_RATE
5.)	STW_RB_DIS:  
RATE_ID, DISDATAFB, DISRTSFB, RTS_RATEFB_LMT, DATA_RATEFB_LMT
6.)	STW_PKTRE_DIS:  
RTY_LMT_EN,  DATA_RT_LMT,  BAR_RTY_TH
#endif

	BOOLEAN	TX_DESC_STW_EN			 = pdesc_data->stwEn;
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
	BOOLEAN	TX_DESC_SMH_EN 			 = pdesc_data->smhEn;
#endif // CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
	
	//Dword 9
#if CFG_HAL_HW_SEQ
    u2Byte TX_DESC_SEQ               = 0;
#else
    u2Byte TX_DESC_SEQ               = GetSequence(pdesc_data->pHdr);
#endif

    //Dword 6
#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
    u1Byte  TX_DESC_ANTSEL			 = pdesc_data->antSel;	
    u1Byte  TX_DESC_ANTSEL_A			 = pdesc_data->antSel_A;
    u1Byte  TX_DESC_ANTSEL_B			 = pdesc_data->antSel_B;
    u1Byte  TX_DESC_ANTSEL_C			 = pdesc_data->antSel_C;
#endif	//#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))

#if 0 //(CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC||CFG_HAL_HW_TX_SHORTCUT_HDR_CONV)
    if (pdesc_data->stwEn != 0 || pdesc_data->smhEn != 0)
        printk("%s(%d): stwEn:0x%x, smhEn:0x%x \n", __FUNCTION__, __LINE__, pdesc_data->stwEn, pdesc_data->smhEn);
#endif

#if 0 // show TX Rpt Info
{
    int baseReg = 0x8080, offset, i;
    int lenTXDESC = 10, lenHdrInfo = 20;

	HAL_RTL_W8(0x106, 0x7F);
	HAL_RTL_W32(0x140, 0x662);

    for(i = 0; i < lenTXDESC; i++) {
#if 1
        printk("%08X ", (u4Byte)GET_DESC(HAL_RTL_R32(baseReg)));
#else
        printk("0x%x:%08X ", baseReg, (u4Byte)GET_DESC(HAL_RTL_R32(baseReg)));
#endif
        if (i%4==3)
            printk("\n");
        baseReg += 4;
    }
    printk("\n");

    for(i = 0; i < lenHdrInfo; i++) {
#if 1
        printk("%08X ", (u4Byte)GET_DESC(HAL_RTL_R32(baseReg)));
#else
        printk("0x%x:%08X ", baseReg, (u4Byte)GET_DESC(HAL_RTL_R32(baseReg)));
#endif

        if (i%4==3)
            printk("\n");
        baseReg += 4;
    }
    printk("\n");

#if 0
	printk("%08X %08X %08X %08X \n%08X %08X %08X %08X \n%08X %08X \n", 
                (u4Byte)GET_DESC(HAL_RTL_R32(0x8080)), (u4Byte)GET_DESC(HAL_RTL_R32(0x8084)), 
                (u4Byte)GET_DESC(HAL_RTL_R32(0x8088)), (u4Byte)GET_DESC(HAL_RTL_R32(0x808c)),
                (u4Byte)GET_DESC(HAL_RTL_R32(0x8090)), (u4Byte)GET_DESC(HAL_RTL_R32(0x8094)),
                (u4Byte)GET_DESC(HAL_RTL_R32(0x8098)), (u4Byte)GET_DESC(HAL_RTL_R32(0x809c)),
                (u4Byte)GET_DESC(HAL_RTL_R32(0x80a0)), (u4Byte)GET_DESC(HAL_RTL_R32(0x80a4)));
#endif
}
#endif

    ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    cur_q       = &(ptx_dma->tx_queue[queueIndex]);
    ptx_desc    = ((PTX_DESC_88XX)(cur_q->ptx_desc_head)) + cur_q->host_idx;

    //Clear All Bit
    PlatformZeroMemory((PVOID)ptx_desc, sizeof(TX_DESC_88XX));

    //4 Set Dword0
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword0, TX_DESC_TXPKTSIZE, TX_DW0_TXPKSIZE_MSK, TX_DW0_TXPKSIZE_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword0, TX_DESC_OFFSET, TX_DW0_OFFSET_MSK, TX_DW0_OFFSET_SH);

	//4 Set Dword1
    SetTxDescQSel88XX(Adapter, queueIndex, ptx_desc, pdesc_data->tid);        
    if ( (queueIndex >= HCI_TX_DMA_QUEUE_HI0) && (queueIndex <= HCI_TX_DMA_QUEUE_HI7) ) {
        //MacID has written in SetTxDescQSel88XX()
    } else {
        SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, TX_DESC_MACID, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
    }
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, TX_DESC_PKT_OFFSET, TX_DW1_PKT_OFFSET_MSK, TX_DW1_PKT_OFFSET_SH);
#endif

	//4 Set Dword3
	
    //4 Set Dword8
	//4 Set Dword9    
#if CFG_HAL_HW_SEQ
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, 1, TX_DW8_EN_HWSEQ_MSK, TX_DW8_EN_HWSEQ_SH);
#else
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword9, TX_DESC_SEQ, TX_DW9_SEQ_MSK, TX_DW9_SEQ_SH);	
#endif


#if IS_RTL88XX_MAC_V2
    if ( _GET_HAL_DATA(Adapter)->MacVersion.is_MAC_v2) {


		SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, TX_DESC_STW_EN, TX_DW8_STW_EN_MSK, TX_DW8_STW_EN_SH);

		//SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, TX_DESC_STW_ANT_DIS, TX_DW8_STW_ANT_DIS_MSK, TX_DW8_STW_ANT_DIS_SH);
		//SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, TX_DESC_STW_RATE_DIS, TX_DW8_STW_RATE_DIS_MSK, TX_DW8_STW_RATE_DIS_SH);
		//SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, TX_DESC_STW_RB_DIS, TX_DW8_STW_RB_DIS_MSK, TX_DW8_STW_RB_DIS_SH);
		//SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, TX_DESC_STW_PKTRE_DIS, TX_DW8_STW_PKTRE_DIS_MSK, TX_DW8_STW_PKTRE_DIS_SH);

        // for force tx rate
        if (HAL_VAR_TX_FORCE_RATE != 0xff) {
        /*                    
             STW_RATE_DIS:  USE_RATE, Data rate, DATA_SHORT, DATA_BW, TRY_RATE
             STW_RB_DIS:     RATE_ID, DISDATAFB, DISRTSFB, RTS_RATEFB_LMT, DATA_RATEFB_LMT
            */            
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, 1, TX_DW8_STW_RATE_DIS_MSK, TX_DW8_STW_RATE_DIS_SH);
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, 1, TX_DW8_STW_RB_DIS_MSK, TX_DW8_STW_RB_DIS_SH);
            
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, pdesc_data->useRate, TX_DW3_USERATE_MSK, TX_DW3_USERATE_SH);
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, pdesc_data->disRTSFB, TX_DW3_DISRTSFB_MSK, TX_DW3_DISRTSFB_SH);
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, pdesc_data->disDataFB, TX_DW3_DISDATAFB_MSK, TX_DW3_DISDATAFB_SH);
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, pdesc_data->dataRate, TX_DW4_DATARATE_MSK, TX_DW4_DATARATE_SH);
            
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, pdesc_data->rateId, TX_DW1_RATE_ID_MSK, TX_DW1_RATE_ID_SH);
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, pdesc_data->dataBW, TX_DW5_DATA_BW_MSK, TX_DW5_DATA_BW_SH);

            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, 1, TX_DW5_DATA_SHORT_MSK, TX_DW5_DATA_SHORT_SH);            
        }

#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
		SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_WHEADER_LEN, TX_DW3_WHEADER_V1_MSK, TX_DW3_WHEADER_V1_SH);
		SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, TX_DESC_SMH_EN, TX_DW8_SMH_EN_MSK, TX_DW8_SMH_EN_SH);
#endif // CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
	}
#endif // IS_RTL88XX_MAC_V2

#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
   if(TX_DESC_ANTSEL & BIT(0))
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, TX_DESC_ANTSEL_A, TX_DW6_ANTSEL_A_MSK, TX_DW6_ANTSEL_A_SH);
   if(TX_DESC_ANTSEL & BIT(1))
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, TX_DESC_ANTSEL_B, TX_DW6_ANTSEL_B_MSK, TX_DW6_ANTSEL_B_SH);
   if(TX_DESC_ANTSEL & BIT(2))
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, TX_DESC_ANTSEL_C, TX_DW6_ANTSEL_C_MSK, TX_DW6_ANTSEL_C_SH);
#endif	//#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))

}
#endif //CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC

#endif //#if IS_EXIST_RTL8192EE || IS_EXIST_RTL8881AEM || IS_EXIST_RTL8814AE 

HAL_IMEM
BOOLEAN
FillShortCutTxHwCtrl88XX(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      PVOID           pDescData,
    IN      PVOID           pTxDesc,
    IN      u4Byte          direction,
    IN      BOOLEAN         useHW     
)
{
    PVOID       ptx_desc;

    if (0x01 == direction) {
        GET_HAL_INTERFACE(Adapter)->FillTxDescHandler(Adapter, queueIndex, pDescData);
        CopyShortCutTxDesc88XX(Adapter, queueIndex, pTxDesc, direction);
    } else {    // 0x02 == direction

        #if CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
        if(useHW) {

            GET_HAL_INTERFACE(Adapter)->FillHwShortCutTxDescHandler(Adapter, queueIndex, pDescData);
        }
        //FillHwShortCutTxDesc88XX(Adapter, queueIndex, pDescData);
        else
        #endif // CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
        {
            ptx_desc = CopyShortCutTxDesc88XX(Adapter, queueIndex, pTxDesc, direction);
            GET_HAL_INTERFACE(Adapter)->FillShortCutTxDescHandler(Adapter, queueIndex, pDescData, ptx_desc);
            //FillShortCutTxDesc88XX(Adapter, queueIndex, pDescData, ptx_desc);
        }
    }

    SetTxBufferDesc88XX(Adapter, queueIndex, pDescData);

    return _TRUE;
}
#endif // CFG_HAL_TX_SHORTCUT

#endif // (HAL_DEV_BUS_TYPE & (HAL_RT_EMBEDDED_INTERFACE | HAL_RT_PCI_INTERFACE))

#if IS_EXIST_RTL8822BE || IS_EXIST_RTL8197FEM

HAL_IMEM
u2Byte
GetShortCutTxBuffSize88XX_V1(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc
)
{
    PTX_DESC_88XX   ptx_desc = (PTX_DESC_88XX) pTxDesc;
    return  (u2Byte)GET_TX_DESC_TIMESTAMP(ptx_desc);
}

HAL_IMEM
VOID
SetShortCutTxBuffSize88XX_V1(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc,
    IN  u2Byte          txBuffSize
)
{
    PTX_DESC_88XX   ptx_desc = (PTX_DESC_88XX) pTxDesc;
    SET_TX_DESC_TIMESTAMP(ptx_desc, txBuffSize);
}

static VOID
SetSecType_V1(
    IN  HAL_PADAPTER    Adapter,
    IN  PTX_DESC_88XX   ptx_desc,
    IN  PVOID           pDescData 
)
{
    PTX_DESC_DATA_88XX  pdesc_data  = (PTX_DESC_DATA_88XX)pDescData;
    
    switch(pdesc_data->secType) {
    case _WEP_40_PRIVACY_:
    case _WEP_104_PRIVACY_:
    case _TKIP_PRIVACY_:
        SET_TX_DESC_SEC_TYPE(ptx_desc,TXDESC_SECTYPE_WEP40_OR_TKIP);
        break;
#if CFG_HAL_RTL_HW_WAPI_SUPPORT
    case _WAPI_SMS4_:
        SET_TX_DESC_SEC_TYPE(ptx_desc,TXDESC_SECTYPE_WAPI);        
        break;
#endif        
    case _CCMP_PRIVACY_:
        SET_TX_DESC_SEC_TYPE(ptx_desc,TXDESC_SECTYPE_AES);                
        break;
    default:
#if 0
        SET_DESC_FIELD_CLR(ptx_desc->Dword1, TXDESC_SECTYPE_NO_ENCRYPTION,
                                        TX_DW1_SECTYPE_MSK, TX_DW1_SECTYPE_SH);        
#endif
        break;
    }   
}


VOID
SetTxDescQSel88XX_V1(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PTX_DESC_88XX   ptx_desc,
    IN  u1Byte          drvTID
)
{
    u1Byte  q_select;
    //u4Byte  val=0;
    
	switch (queueIndex) {
    	case HCI_TX_DMA_QUEUE_HI0:
            SET_TX_DESC_MOREDATA_NO_CLR(ptx_desc,1);
            //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            //Set MACIDMask to zero, but we have memset before
    		q_select = TXDESC_QSEL_HIGH;
    		break;
#if  CFG_HAL_SUPPORT_MBSSID
    	case HCI_TX_DMA_QUEUE_HI1:
            SET_TX_DESC_MOREDATA_NO_CLR(ptx_desc,1);
            //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI2:
            SET_TX_DESC_MOREDATA_NO_CLR(ptx_desc,1);
            //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 2, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI3:	
            SET_TX_DESC_MOREDATA_NO_CLR(ptx_desc,1);            
            //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 3, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI4:
            SET_TX_DESC_MOREDATA_NO_CLR(ptx_desc,1);               
            //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 4, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI5:
            SET_TX_DESC_MOREDATA_NO_CLR(ptx_desc,1);               
            //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 5, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI6:
            SET_TX_DESC_MOREDATA_NO_CLR(ptx_desc,1);               
            //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 6, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI7:
            SET_TX_DESC_MOREDATA_NO_CLR(ptx_desc,1);   
            //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 7, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
            q_select = TXDESC_QSEL_HIGH;
    		break;
#endif  //CFG_HAL_SUPPORT_MBSSID
        case HCI_TX_DMA_QUEUE_CMD:
            q_select = TXDESC_QSEL_CMD;
            break;     
    	case HCI_TX_DMA_QUEUE_MGT:
#if 0 //eric-8822 ?? TX HANG
			//SET_TX_DESC_MOREDATA_NO_CLR(ptx_desc,1);  
			q_select = TXDESC_QSEL_HIGH;
#else
    		q_select = TXDESC_QSEL_MGT;
#endif
    		break;
            
#if CFG_HAL_MAC_LOOPBACK && CFG_HAL_WIFI_WMM
    	case HCI_TX_DMA_QUEUE_BE:
    		q_select = TXDESC_QSEL_TID0;
    		break;
#endif  //CFG_HAL_MAC_LOOPBACK && CFG_HAL_WIFI_WMM

    	default:
    		// data packet
#if CFG_HAL_RTL_MANUAL_EDCA
    		if (HAL_VAR_MANUAL_EDCA) {
    			switch (queueIndex) {
        			case HCI_TX_DMA_QUEUE_VO:
        				q_select = TXDESC_QSEL_TID6;
        				break;
        			case HCI_TX_DMA_QUEUE_VI:
        				q_select = TXDESC_QSEL_TID4;
                		break;
        			case HCI_TX_DMA_QUEUE_BE:
        				q_select = TXDESC_QSEL_TID0;
        	    		break;
        		    default:
        				q_select = TXDESC_QSEL_TID1;
        				break;
    			}
    		}
    		else {
                q_select = drvTID;
    		}
#else
            q_select = drvTID;
#endif  //CFG_HAL_RTL_MANUAL_EDCA
            break;
	}    
    SET_TX_DESC_QSEL_NO_CLR(ptx_desc,q_select);   
    //SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, q_select, TX_DW1_QSEL_MSK, TX_DW1_QSEL_SH);
    //ptx_desc->Dword1 |= val;
}


VOID
FillBeaconDesc88XX_V1
(
    IN	HAL_PADAPTER        Adapter,
    IN  PVOID               _pdesc,
    IN  PVOID               data_content,
    IN  u2Byte              txLength,
    IN  BOOLEAN             bForceUpdate
)
{
    u4Byte  TXDESCSize;
    PTX_DESC_88XX       pdesc = (PTX_DESC_88XX)_pdesc;

#if (IS_EXIST_RTL8197FEM)
    if ( IS_HARDWARE_TYPE_8197F(Adapter) ) {
        TXDESCSize = SIZE_TXDESC_88XX;
    }
#endif  //IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM)

#if IS_EXIST_RTL8822BE
    if ( IS_HARDWARE_TYPE_8822B(Adapter))  {
        TXDESCSize = SIZE_TXDESC_88XX_V1;
    }
#endif //IS_EXIST_RTL8822BE

    PlatformZeroMemory(pdesc, TXDESCSize);

    //Dword0
    SET_TX_DESC_BMC(pdesc, 1);
    SET_TX_DESC_OFFSET(pdesc, TXDESCSize);
    SET_TX_DESC_TXPKTSIZE(pdesc, txLength);

    //Dword1
    SET_TX_DESC_QSEL(pdesc,TXDESC_QSEL_BCN);  


#if CFG_HAL_SUPPORT_MBSSID
        if (HAL_IS_VAP_INTERFACE(Adapter)) {
    
        // set MBSSID for each VAP_ID
        SET_TX_DESC_MACID(pdesc, HAL_VAR_VAP_INIT_SEQ);
        SET_TX_DESC_MBSSID(pdesc, HAL_VAR_VAP_INIT_SEQ);        
   
        }         
#endif //#if CFG_HAL_SUPPORT_MBSSID

    SET_TX_DESC_SW_SEQ(pdesc,GetSequence(data_content));
    SET_TX_DESC_DISDATAFB(pdesc,1);
    SET_TX_DESC_USE_RATE(pdesc,1);


/*cy wang cfg p2p rm*/
    if (HAL_VAR_IS_40M_BW == 1) {
        if (HAL_VAR_OFFSET_2ND_CHANNEL == HT_2NDCH_OFFSET_BELOW) {
            SET_TX_DESC_DATA_SC(pdesc,TXDESC_DATASC_LOWER);
        }
		else {
            SET_TX_DESC_DATA_SC(pdesc,TXDESC_DATASC_UPPER);
		}
	}

    //Dword4    /*cy wang cfg p2p , 6m rate beacon*//*cy wang cfg p2p*/
    #ifdef P2P_SUPPORT	// 2014-0328 use 6m rate send beacon
    if(Adapter->pmib->p2p_mib.p2p_enabled){          
        SET_TX_DESC_RTSRATE(pdesc,4);
        SET_TX_DESC_DATARATE(pdesc,4);
    }    
    #endif   
/*cy wang cfg p2p*/
/*
		 * Intel IOT, dynamic enhance beacon tx AGC
*/

	if (Adapter->pmib->dot11StationConfigEntry.beacon_rate != 0xff)
        SET_TX_DESC_DATARATE(pdesc,Adapter->pmib->dot11StationConfigEntry.beacon_rate);

#if IS_EXIST_RTL8822BE
    if(IS_HARDWARE_TYPE_8822B(Adapter)) {
    	SET_TX_DESC_RTSRATE(pdesc,4);
    	SET_TX_DESC_DATARATE(pdesc,4);
    }
#endif

	if (Adapter->bcnTxAGC ==1) {
        SET_TX_DESC_TXPWR_OFSET(pdesc,4); // +3dB
	} else if (Adapter->bcnTxAGC ==2) {
        SET_TX_DESC_TXPWR_OFSET(pdesc,5); // +6dB	
	} else {
        SET_TX_DESC_TXPWR_OFSET(pdesc,0);
	}

	if (Adapter->pmib->dot11RFEntry.txpwr_reduction) {
		if (Adapter->pmib->dot11RFEntry.txpwr_reduction <= 3)
            SET_TX_DESC_TXPWR_OFSET(pdesc,Adapter->pmib->dot11RFEntry.txpwr_reduction); 
	}

	if (Adapter->pmib->dot11RFEntry.bcn2path && IS_HARDWARE_TYPE_8192EE(Adapter) )
        SET_TX_DESC_TX_ANT(pdesc,3);

    // TODO: Why ?
    HAL_VAR_IS_40M_BW_BAK   = HAL_VAR_IS_40M_BW;
    HAL_VAR_TX_BEACON_LEN   = txLength;

    // no use ?
    //SET_DESC_FIELD_CLR(pdesc->Dword7, txLength, TX_DW7_SW_TXBUFF_MSK, TX_DW7_SW_TXBUFF_SH);


#if 0   // TODO: Filen: test code ?
#if (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
    if (IS_ROOT_INTERFACE(Adapter)) {
		if (Adapter->pshare->rf_ft_var.swq_dbg	== 30) {
			pdesc->Dword9 |= set_desc((1122 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
		}
		else {
			pdesc->Dword9 |= set_desc((5566 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
		}
    }
    else {
        pdesc->Dword9 |= set_desc((GetSequence(data_content) & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);        
    }
#else
    if (Adapter->pshare->rf_ft_var.swq_dbg == 30) {
        pdesc->Dword9 |= set_desc((1122 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
    }
    else {
        pdesc->Dword9 |= set_desc((5566 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
    }
#endif  //(defined(UNIVERSAL_REPEATER) || defined(MBSSID))
#endif

    // Group Bit Control
    SET_TX_DESC_GROUP_BIT_IE_OFFSET(pdesc,(HAL_VAR_TIM_OFFSET-24));
    
    // Auto set bitmap control by HW, no present in TXDESC document so reserved this code
    if (HAL_OPMODE & WIFI_ADHOC_STATE) {
        SET_DESC_FIELD_CLR(pdesc->Dword9, 0, TX_DW9_GROUPBIT_IE_ENABLE_MSK, TX_DW9_GROUPBIT_IE_ENABLE_SH);
    } else {
    	if(Adapter->pshare->rf_ft_var.wakeforce != 1)
        SET_DESC_FIELD_CLR(pdesc->Dword9, 1, TX_DW9_GROUPBIT_IE_ENABLE_MSK, TX_DW9_GROUPBIT_IE_ENABLE_SH);
    }
    // TODO: Check with Button


#ifdef TRXBD_CACHABLE_REGION
    _dma_cache_wback((unsigned long)((PVOID)(pdesc)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), TXDESCSize);
#endif //#ifdef TRXBD_CACHABLE_REGION
}

VOID
FillRsrvPageDesc88XX_V1 
(
    IN	HAL_PADAPTER        Adapter,
    IN  PVOID               _pdesc,
    IN  PVOID               data_content,
    IN  u2Byte              txLength
)
    {
        u4Byte  TXDESCSize;
        PTX_DESC_88XX       pdesc = (PTX_DESC_88XX)_pdesc;
    
#if (IS_EXIST_RTL8197FEM)
        if ( IS_HARDWARE_TYPE_8197F(Adapter) ) {
            TXDESCSize = SIZE_TXDESC_88XX;
        }
#endif  //IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM)
    
#if IS_EXIST_RTL8822BE
        if ( IS_HARDWARE_TYPE_8822B(Adapter))  {
            TXDESCSize = SIZE_TXDESC_88XX_V1;
        }
#endif //IS_EXIST_RTL8822BE
    
        PlatformZeroMemory(pdesc, TXDESCSize);
    
        //Dword0
        //SET_TX_DESC_BMC(pdesc, 1);
        SET_TX_DESC_OFFSET(pdesc, TXDESCSize);
        SET_TX_DESC_TXPKTSIZE(pdesc, txLength);
    
        //Dword1
        SET_TX_DESC_QSEL(pdesc,TXDESC_QSEL_BCN);  
    
    
#if CFG_HAL_SUPPORT_MBSSID
            if (HAL_IS_VAP_INTERFACE(Adapter)) {
        
            // set MBSSID for each VAP_ID
            SET_TX_DESC_MACID(pdesc, HAL_VAR_VAP_INIT_SEQ);
            SET_TX_DESC_MBSSID(pdesc, HAL_VAR_VAP_INIT_SEQ);        
       
            }         
#endif //#if CFG_HAL_SUPPORT_MBSSID
    
        SET_TX_DESC_SW_SEQ(pdesc,GetSequence(data_content));
        SET_TX_DESC_DISDATAFB(pdesc,1);
        SET_TX_DESC_USE_RATE(pdesc,1);
    
    
    /*cy wang cfg p2p rm*/
        if (HAL_VAR_IS_40M_BW == 1) {
            if (HAL_VAR_OFFSET_2ND_CHANNEL == HT_2NDCH_OFFSET_BELOW) {
                SET_TX_DESC_DATA_SC(pdesc,TXDESC_DATASC_LOWER);
            }
            else {
                SET_TX_DESC_DATA_SC(pdesc,TXDESC_DATASC_UPPER);
            }
        }
    
        //Dword4    /*cy wang cfg p2p , 6m rate beacon*//*cy wang cfg p2p*/
    #ifdef P2P_SUPPORT	// 2014-0328 use 6m rate send beacon
        if(Adapter->pmib->p2p_mib.p2p_enabled){          
            SET_TX_DESC_RTSRATE(pdesc,4);
            SET_TX_DESC_DATARATE(pdesc,4);
        }    
    #endif   
    /*cy wang cfg p2p*/
    /*
             * Intel IOT, dynamic enhance beacon tx AGC
    */
    
        if (Adapter->pmib->dot11StationConfigEntry.beacon_rate != 0xff)
            SET_TX_DESC_DATARATE(pdesc,Adapter->pmib->dot11StationConfigEntry.beacon_rate);
    
#if IS_EXIST_RTL8822BE
        if(IS_HARDWARE_TYPE_8822B(Adapter)) {
            SET_TX_DESC_RTSRATE(pdesc,4);
            SET_TX_DESC_DATARATE(pdesc,4);
        }
#endif
    
        if (Adapter->bcnTxAGC ==1) {
            SET_TX_DESC_TXPWR_OFSET(pdesc,4); // +3dB
        } else if (Adapter->bcnTxAGC ==2) {
            SET_TX_DESC_TXPWR_OFSET(pdesc,5); // +6dB   
        } else {
            SET_TX_DESC_TXPWR_OFSET(pdesc,0);
        }
    
        if (Adapter->pmib->dot11RFEntry.txpwr_reduction) {
            if (Adapter->pmib->dot11RFEntry.txpwr_reduction <= 3)
                SET_TX_DESC_TXPWR_OFSET(pdesc,Adapter->pmib->dot11RFEntry.txpwr_reduction); 
        }
    
        if (Adapter->pmib->dot11RFEntry.bcn2path && IS_HARDWARE_TYPE_8192EE(Adapter) )
            SET_TX_DESC_TX_ANT(pdesc,3);
    
        // TODO: Why ?
        HAL_VAR_IS_40M_BW_BAK   = HAL_VAR_IS_40M_BW;
        HAL_VAR_TX_BEACON_LEN   = txLength;
    
        // no use ?
        //SET_DESC_FIELD_CLR(pdesc->Dword7, txLength, TX_DW7_SW_TXBUFF_MSK, TX_DW7_SW_TXBUFF_SH);
    
    
#if 0   // TODO: Filen: test code ?
#if (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
        if (IS_ROOT_INTERFACE(Adapter)) {
            if (Adapter->pshare->rf_ft_var.swq_dbg  == 30) {
                pdesc->Dword9 |= set_desc((1122 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
            }
            else {
                pdesc->Dword9 |= set_desc((5566 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
            }
        }
        else {
            pdesc->Dword9 |= set_desc((GetSequence(data_content) & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);        
        }
#else
        if (Adapter->pshare->rf_ft_var.swq_dbg == 30) {
            pdesc->Dword9 |= set_desc((1122 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
        }
        else {
            pdesc->Dword9 |= set_desc((5566 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
        }
#endif  //(defined(UNIVERSAL_REPEATER) || defined(MBSSID))
#endif
    
        // Group Bit Control
        SET_TX_DESC_GROUP_BIT_IE_OFFSET(pdesc,(HAL_VAR_TIM_OFFSET-24));
        
        // Auto set bitmap control by HW, no present in TXDESC document so reserved this code
        if (HAL_OPMODE & WIFI_ADHOC_STATE) {
            SET_DESC_FIELD_CLR(pdesc->Dword9, 0, TX_DW9_GROUPBIT_IE_ENABLE_MSK, TX_DW9_GROUPBIT_IE_ENABLE_SH);
        } else {
            SET_DESC_FIELD_CLR(pdesc->Dword9, 1, TX_DW9_GROUPBIT_IE_ENABLE_MSK, TX_DW9_GROUPBIT_IE_ENABLE_SH);
        }
        // TODO: Check with Button
    
    
#ifdef TRXBD_CACHABLE_REGION
        _dma_cache_wback((unsigned long)((PVOID)(pdesc)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), TXDESCSize);
#endif //#ifdef TRXBD_CACHABLE_REGION
    }


VOID
FillTxDesc88XX_V1 (
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pDescData
)
{
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PTX_DESC_88XX                   ptx_desc;
    PTX_DESC_DATA_88XX              pdesc_data = (PTX_DESC_DATA_88XX)pDescData;
    u4Byte                          val;
    u4Byte                          tmp,tmpCache;
    u4Byte                          TXDESCSize;

#if (IS_EXIST_RTL8197FEM)
        if ( IS_HARDWARE_TYPE_8197F(Adapter) ) {
            TXDESCSize = SIZE_TXDESC_88XX;
        }
#endif  //IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM)
    
#if IS_EXIST_RTL8822BE
        if ( IS_HARDWARE_TYPE_8822B(Adapter))  {
            TXDESCSize = SIZE_TXDESC_88XX_V1;
        }
#endif //IS_EXIST_RTL8822BE

    //Dword 0
    u2Byte  TX_DESC_TXPKTSIZE        = pdesc_data->hdrLen + pdesc_data->llcLen + pdesc_data->frLen;
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    u1Byte	TX_DESC_OFFSET			 = ((pdesc_data->smhEn == TRUE) ? (HAL_HW_TXSC_HDR_CONV_ADD_OFFSET+TXDESCSize) : TXDESCSize);
#else
    u1Byte  TX_DESC_OFFSET           = TXDESCSize;
#endif

#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    BOOLEAN TX_DESC_BMC              = (pdesc_data->smhEn == TRUE) ? 
    					((HAL_IS_MCAST(GetEthDAPtr((pu1Byte)pdesc_data->pHdr))) ? 1 : 0) : 
				        ((HAL_IS_MCAST(GetAddr1Ptr((pu1Byte)pdesc_data->pHdr))) ? 1 : 0);   // when multicast or broadcast, BMC = 1
#else
    BOOLEAN TX_DESC_BMC              = (HAL_IS_MCAST(GetAddr1Ptr((pu1Byte)pdesc_data->pHdr))) ? 1 : 0;   // when multicast or broadcast, BMC = 1        
#endif

//    BOOLEAN TX_DESC_HT               = 0;
//    BOOLEAN TX_DESC_LINIP            = 0;
//    BOOLEAN TX_DESC_NOACM            = 0;
//    BOOLEAN TX_DESC_GF               = 0;

    //Dword 1 
    u1Byte  TX_DESC_MACID            = pdesc_data->macId; // MACID/MBSSID ?
    u1Byte  TX_DESC_RATE_ID          = pdesc_data->rateId;
    BOOLEAN TX_DESC_MORE_DATA        = pdesc_data->moreData;    
    BOOLEAN TX_DESC_EN_DESC_ID       = pdesc_data->enDescId;
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    u1Byte  TX_DESC_PKT_OFFSET       = 0; // unit: 8 bytes. Early mode: 1 units, (8 * 1 = 8 bytes for early mode info)
#endif
    
    //Dword 2
    BOOLEAN TX_DESC_AGG_EN           = pdesc_data->aggEn; 
    BOOLEAN TX_DESC_BK               = pdesc_data->bk;
    BOOLEAN TX_DESC_MOREFRAG         = pdesc_data->frag;
    u1Byte  TX_DESC_AMPDU_DENSITY    = pdesc_data->ampduDensity;
    u4Byte  TX_DESC_P_AID      		 = pdesc_data->p_aid;
    u1Byte  TX_DESC_G_ID      		 = pdesc_data->g_id;
#if CFG_HAL_HW_AES_IV
    BOOLEAN TX_DESC_HW_AES_IV        = pdesc_data->hwAESIv;
#endif // CFG_HAL_HW_AES_IV

#if 1 //eric-ac2
	u1Byte	TX_DESC_CCA_RTS 		 = pdesc_data->cca_rts;
#endif


    //Dword 3
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV_LLC
    u1Byte  TX_DESC_WHEADER_LEN      = ((pdesc_data->smhEn == TRUE) ? (HAL_ETH_HEADER_LEN_MAX >> 1) : ((pdesc_data->hdrLen) >> 1)); // unit: 2 bytes
#else
    u1Byte  TX_DESC_WHEADER_LEN      = ((pdesc_data->smhEn == TRUE) ? (HAL_ETH_HEADER_LEN_MAX >> 1) : ((pdesc_data->hdrLen+pdesc_data->llcLen+pdesc_data->iv) >> 1)); // unit: 2 bytes
#endif // CFG_HAL_HW_TX_SHORTCUT_HDR_CONV_LLC
#endif // CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    BOOLEAN TX_DESC_USERATE          = pdesc_data->useRate;    
    BOOLEAN TX_DESC_DISRTSFB         = pdesc_data->disRTSFB;
    BOOLEAN TX_DESC_DISDATAFB        = pdesc_data->disDataFB;
    BOOLEAN TX_DESC_CTS2SELF         = pdesc_data->CTS2Self;
    BOOLEAN TX_DESC_RTS_EN           = pdesc_data->RTSEn;
    BOOLEAN TX_DESC_HW_RTS_EN        = pdesc_data->HWRTSEn;
    BOOLEAN TX_DESC_NAVUSEHDR        = pdesc_data->navUseHdr;
    u1Byte  TX_DESC_MAX_AGG_NUM      = pdesc_data->maxAggNum;
    BOOLEAN TX_DESC_NDPA		     = pdesc_data->ndpa;

    //Dword 4
    u1Byte  TX_DESC_DATERATE         = pdesc_data->dataRate;
    u1Byte  TX_DESC_DATA_RATEFB_LMT  = pdesc_data->dataRateFBLmt;
    u1Byte  TX_DESC_RTS_RATEFB_LMT   = pdesc_data->RTSRateFBLmt;
    BOOLEAN TX_DESC_RTY_LMT_EN       = pdesc_data->rtyLmtEn;   
    u1Byte  TX_DESC_DATA_RT_LMT      = pdesc_data->dataRtyLmt;
    u1Byte  TX_DESC_RTSRATE          = pdesc_data->RTSRate;
    u1Byte  TX_DESC_BMCRtyLmt        = pdesc_data->BMCRtyLmt;

    //Dword 5
    u1Byte  TX_DESC_DATA_SC          = pdesc_data->dataSC;
    u1Byte  TX_DESC_DATA_SHORT       = pdesc_data->dataShort;
    u1Byte  TX_DESC_DATA_BW          = pdesc_data->dataBW;
    u1Byte  TX_DESC_DATA_STBC        = pdesc_data->dataStbc;
	u1Byte  TX_DESC_DATA_LDPC        = pdesc_data->dataLdpc;
    u1Byte  TX_DESC_RTS_SHORT        = pdesc_data->RTSShort;
    u1Byte  TX_DESC_RTS_SC           = pdesc_data->RTSSC;   
    u1Byte	TX_DESC_POWER_OFFSET	 = pdesc_data->TXPowerOffset;
    u1Byte	TX_ANT					 = pdesc_data->TXAnt;	

    //Dword 7
    // use for CFG_HAL_TX_SHORTCUT
    u2Byte  TX_DESC_TXBUFF           = pdesc_data->frLen;

	// Dword 8
#if CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
	BOOLEAN	TX_DESC_TXWIFI_CP		= pdesc_data->txwifiCp;
#endif // CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
	BOOLEAN TX_DESC_MAC_CP			= pdesc_data->macCp;
	BOOLEAN TX_DESC_SMH_EN			= pdesc_data->smhEn;
#endif // CFG_HAL_HW_TX_SHORTCUT_HDR_CONV

    //Dword 9
#if CFG_HAL_HW_SEQ
    u2Byte TX_DESC_SEQ               = 0;
#else
    u2Byte TX_DESC_SEQ               = GetSequence(pdesc_data->pHdr);
#endif

#if 1 //eric-8822 add SND_pkt_sel
	//Dword 10
	u1Byte SND_PKT_SEL               = pdesc_data->SND_pkt_sel;
	u1Byte IS_GID               	 = pdesc_data->is_GID;
#endif

#if 0 //(CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC||CFG_HAL_HW_TX_SHORTCUT_HDR_CONV)
	if (pdesc_data->smhEn != 0 || pdesc_data->macCp != 0)
		printk("%s(%d): macCp:0x%x, smhEn:0x%x \n", __FUNCTION__, __LINE__, pdesc_data->macCp, pdesc_data->smhEn);

	if (pdesc_data->txwifiCp != 0)
		printk("%s(%d): txwifiCp:0x%x \n", __FUNCTION__, __LINE__, pdesc_data->txwifiCp);
#endif

#if 0 // show TX Rpt Info
    {
        int baseReg = 0x8080, offset, i;
        int lenTXDESC = 10, lenHdrInfo = 20;
    
        if (pdesc_data->smhEn != 0) {
            HAL_RTL_W8(0x106, 0x7F);
            HAL_RTL_W32(0x140, 0x662);
        
            for(i = 0; i < lenTXDESC; i++) {
                printk("%08X ", (u4Byte)GET_DESC(HAL_RTL_R32(baseReg)));
                if (i%4==3)
                    printk("\n");
                baseReg += 4;
            }
            printk("\n");
        
            for(i = 0; i < lenHdrInfo; i++) {
                printk("%08X ", (u4Byte)GET_DESC(HAL_RTL_R32(baseReg)));
                if (i%4==3)
                    printk("\n");
                baseReg += 4;
            }
            printk("\n");
        }
    }
#endif

	if(Adapter->pshare->rf_ft_var.manual_ampdu)
		TX_DESC_MAX_AGG_NUM = 0;

#if CFG_HAL_MULTICAST_BMC_ENHANCE
    if(TX_DESC_BMC)
    {
        TX_DESC_BMCRtyLmt = pdesc_data->BMCRtyLmt;
    }
#endif


    ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    cur_q       = &(ptx_dma->tx_queue[queueIndex]);
    ptx_desc    = ((PTX_DESC_88XX)(cur_q->ptx_desc_head)) + cur_q->host_idx;

    //Clear All Bit
    PlatformZeroMemory((PVOID)ptx_desc, sizeof(TX_DESC_88XX));

    if (pdesc_data->secType != _NO_PRIVACY_) {
        if (pdesc_data->swCrypt == FALSE) {
            SetSecType_V1(Adapter, ptx_desc, pdesc_data);
            // for hw sec: 1) WEP's iv, 2) TKIP's iv and eiv, 3) CCMP's ccmp header are all in pdesc_data->iv
            TX_DESC_TXPKTSIZE += pdesc_data->iv;
        } else {
            // for sw sec
            TX_DESC_TXPKTSIZE += (pdesc_data->iv + pdesc_data->icv + pdesc_data->mic);
        }
    }

    //4 Set Dword0
    SET_TX_DESC_TXPKTSIZE_NO_CLR(ptx_desc, TX_DESC_TXPKTSIZE);
    SET_TX_DESC_OFFSET_NO_CLR(ptx_desc, TX_DESC_OFFSET);
    SET_TX_DESC_BMC_NO_CLR(ptx_desc, TX_DESC_BMC);

    //4 Set Dword1
#if 1 //eric-8822 ?? tx hang
	if (TX_DESC_NDPA) {
		//printk("[%s] tid = %d, TX_DESC_P_AID = 0x%x SND_PKT_SEL = 0x%x \n", __FUNCTION__, 
			//pdesc_data->tid, TX_DESC_P_AID, SND_PKT_SEL);

		TX_DESC_RTY_LMT_EN = 1; 
		TX_DESC_DATA_RT_LMT = 4;
		
		SET_TX_DESC_QSEL_NO_CLR(ptx_desc, TXDESC_QSEL_MGT);  //eric-mu 
	}
	else
#endif
    SetTxDescQSel88XX_V1(Adapter, queueIndex, ptx_desc, pdesc_data->tid);        

    if ( (queueIndex >= HCI_TX_DMA_QUEUE_HI0) && (queueIndex <= HCI_TX_DMA_QUEUE_HI7) ) {
        //MacID has written in SetTxDescQSel88XX()
#ifdef HW_ENC_FOR_GROUP_CIPHER
        SET_TX_DESC_MACID_NO_CLR(ptx_desc, TX_DESC_MACID);
#endif
    } else {
        SET_TX_DESC_MACID_NO_CLR(ptx_desc, TX_DESC_MACID);
    }
    SET_TX_DESC_RATE_ID_NO_CLR(ptx_desc, TX_DESC_RATE_ID);
    SET_TX_DESC_MOREDATA_NO_CLR(ptx_desc, TX_DESC_MORE_DATA);    
    if (TX_DESC_EN_DESC_ID)
        SET_TX_DESC_EN_DESC_ID_NO_CLR(ptx_desc, 1);        

	
#if IS_EXIST_RTL8822BE
	if ( IS_HARDWARE_TYPE_8822B(Adapter))
		TX_DESC_BK = 0;
#endif //IS_EXIST_RTL8822BE

    //4 Set Dword2
    SET_TX_DESC_AGG_EN_NO_CLR(ptx_desc, TX_DESC_AGG_EN);
    SET_TX_DESC_BK_NO_CLR(ptx_desc, TX_DESC_BK);
    SET_TX_DESC_MOREFRAG_NO_CLR(ptx_desc, TX_DESC_MOREFRAG);
    SET_TX_DESC_AMPDU_DENSITY_NO_CLR(ptx_desc, TX_DESC_AMPDU_DENSITY);
    SET_TX_DESC_P_AID_NO_CLR(ptx_desc, TX_DESC_P_AID);
    SET_TX_DESC_G_ID_NO_CLR(ptx_desc, TX_DESC_G_ID);

#if CFG_HAL_HW_AES_IV
    SET_TX_DESC_HW_AES_IV_NO_CLR(ptx_desc, TX_DESC_HW_AES_IV);
#endif // CFG_HAL_HW_AES_IV

	
#if 1 //eric-ac2
	SET_TX_DESC_CCA_RTS_NO_CLR(ptx_desc, TX_DESC_CCA_RTS);

	if(IS_GID)
	SET_TX_DESC_SPE_RPT_NO_CLR(ptx_desc, 1);
#endif


    //4 Set Dword3
    SET_TX_DESC_USE_RATE_NO_CLR(ptx_desc, TX_DESC_USERATE);
    SET_TX_DESC_DISRTSFB_NO_CLR(ptx_desc, TX_DESC_DISRTSFB);
    SET_TX_DESC_DISDATAFB_NO_CLR(ptx_desc, TX_DESC_DISDATAFB);

    SET_TX_DESC_CTS2SELF_NO_CLR(ptx_desc, TX_DESC_CTS2SELF);
    SET_TX_DESC_RTSEN_NO_CLR(ptx_desc, TX_DESC_RTS_EN);

    SET_TX_DESC_HW_RTS_EN_NO_CLR(ptx_desc, TX_DESC_HW_RTS_EN);
    SET_TX_DESC_NAVUSEHDR_NO_CLR(ptx_desc, TX_DESC_NAVUSEHDR);
    SET_TX_DESC_MAX_AGG_NUM_NO_CLR(ptx_desc, TX_DESC_MAX_AGG_NUM);
    SET_TX_DESC_NDPA_NO_CLR(ptx_desc, TX_DESC_NDPA);    
   

    //4 Set Dword4
    SET_TX_DESC_DATARATE_NO_CLR(ptx_desc, TX_DESC_DATERATE);    

    SET_TX_DESC_DATA_RTY_LOWEST_RATE_NO_CLR(ptx_desc, TX_DESC_DATA_RATEFB_LMT);    
    SET_TX_DESC_RTS_RTY_LOWEST_RATE_NO_CLR(ptx_desc, TX_DESC_RTS_RATEFB_LMT);        
    
#if 1 //eric-8822 tx hang
	//if(GET_TX_DESC_QSEL(ptx_desc) == TXDESC_QSEL_MGT)
	if(TX_DESC_BMC)
	SET_TX_DESC_DATA_RTY_LOWEST_RATE(ptx_desc, 0);
#else
#if CFG_HAL_MULTICAST_BMC_ENHANCE
        if(TX_DESC_BMC) {
            SET_TX_DESC_DATA_RTY_LOWEST_RATE(ptx_desc, TX_DESC_BMCRtyLmt);
        }
#endif //#if CFG_HAL_MULTICAST_BMC_ENHANCE        
#endif

#if IS_EXIST_RTL8822BE
	if ( IS_HARDWARE_TYPE_8822B(Adapter))  {
		if(TX_DESC_RTY_LMT_EN && (!TX_DESC_NDPA)){
			if(TX_DESC_DATA_RT_LMT <= Adapter->pshare->rf_ft_var.mu_retry)
			TX_DESC_DATA_RT_LMT = (Adapter->pshare->rf_ft_var.mu_retry + 1);
		}
	}
#endif //IS_EXIST_RTL8822BE

    SET_TX_DESC_RTY_LMT_EN_NO_CLR(ptx_desc, TX_DESC_RTY_LMT_EN);    
    SET_TX_DESC_RTS_DATA_RTY_LMT_NO_CLR(ptx_desc, TX_DESC_DATA_RT_LMT);    
    SET_TX_DESC_RTSRATE_NO_CLR(ptx_desc, TX_DESC_RTSRATE);    


#if 1 //eric-8822 ?? No retry for broadcast
	if(TX_DESC_BMC){
		SET_TX_DESC_RTY_LMT_EN(ptx_desc, 1);    
        	SET_TX_DESC_RTS_DATA_RTY_LMT(ptx_desc, 0);  
	}
#endif


    //4 Set Dword5

#if 0
	if(SND_PKT_SEL == 1) {
    SET_TX_DESC_DATA_SC_NO_CLR(ptx_desc, 9);    
    SET_TX_DESC_DATA_SHORT_NO_CLR(ptx_desc, TX_DESC_DATA_SHORT);    
    SET_TX_DESC_DATA_BW_NO_CLR(ptx_desc, 1); 
	}
	else
#endif
{
    SET_TX_DESC_DATA_SC_NO_CLR(ptx_desc, TX_DESC_DATA_SC);    
    SET_TX_DESC_DATA_SHORT_NO_CLR(ptx_desc, TX_DESC_DATA_SHORT);    
    SET_TX_DESC_DATA_BW_NO_CLR(ptx_desc, TX_DESC_DATA_BW);  
}
    SET_TX_DESC_DATA_STBC_NO_CLR(ptx_desc, TX_DESC_DATA_STBC);     
    SET_TX_DESC_DATA_LDPC_NO_CLR(ptx_desc, TX_DESC_DATA_LDPC); 
    SET_TX_DESC_RTS_SHORT_NO_CLR(ptx_desc, TX_DESC_RTS_SHORT); 
    SET_TX_DESC_RTS_SC_NO_CLR(ptx_desc, TX_DESC_RTS_SC); 
    SET_TX_DESC_TXPWR_OFSET_NO_CLR(ptx_desc, TX_DESC_POWER_OFFSET); 

	if (Adapter->pmib->dot11RFEntry.bcn2path || Adapter->pmib->dot11RFEntry.tx2path)
        SET_TX_DESC_TX_ANT_NO_CLR(ptx_desc, TX_ANT); 
  
    //4 Set Dword6
#if CFG_HAL_SUPPORT_MBSSID
    if (HAL_IS_VAP_INTERFACE(Adapter)) {
    // set MBSSID for each VAP_ID
    SET_TX_DESC_MBSSID_NO_CLR(ptx_desc, HAL_VAR_VAP_INIT_SEQ);     
    }         
#endif //#if CFG_HAL_SUPPORT_MBSSID

	if(IS_GID)
	SET_TX_DESC_SW_DEFINE_NO_CLR(ptx_desc, 2);

    //4 Set Dword7
    SET_TX_DESC_TIMESTAMP_NO_CLR(ptx_desc, TX_DESC_TXBUFF);         

	//4 Set Dword8
    //4 Set Dword9
#if CFG_HAL_HW_SEQ
    SET_TX_DESC_EN_HWSEQ_NO_CLR(ptx_desc, 1);
#else
    SET_TX_DESC_SW_SEQ_NO_CLR(ptx_desc, TX_DESC_SEQ);
#endif

#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    SET_TX_DESC_PKT_OFFSET_NO_CLR(ptx_desc, TX_DESC_PKT_OFFSET);
    SET_TX_DESC_WHEADER_LEN_NO_CLR(ptx_desc, TX_DESC_WHEADER_LEN);
    SET_TX_DESC_MAC_CP_NO_CLR(ptx_desc, TX_DESC_MAC_CP);    
    SET_TX_DESC_SMH_EN_NO_CLR(ptx_desc, TX_DESC_SMH_EN);
#endif
#if CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
    SET_TX_DESC_TXWIFI_CP(ptx_desc, TX_DESC_TXWIFI_CP);
#endif

#if 1 //eric-8822 ??  add SND_pkt_sel
    //4 Set Dword10
    if(SND_PKT_SEL)
	SET_TX_DESC_SND_PKT_SEL_NO_CLR(ptx_desc, SND_PKT_SEL);

	SET_TX_DESC_MU_DATARATE_NO_CLR(ptx_desc, TX_DESC_DATERATE);  
#endif


#if 0
	3.) STW_ANT_DIS:  
	ant_mapA, ant_mapB, ant_mapC, ant_mapD, ANTSEL_A, ANTSEL_B, Ntx_map, TXPWR_OFFSET 
	4.) STW_RATE_DIS:  
	USE_RATE, Data rate, DATA_SHORT, DATA_BW, TRY_RATE
	5.) STW_RB_DIS:  
	RATE_ID, DISDATAFB, DISRTSFB, RTS_RATEFB_LMT, DATA_RATEFB_LMT
	6.) STW_PKTRE_DIS:	
	RTY_LMT_EN,  DATA_RT_LMT,  BAR_RTY_TH
#endif

#if 0
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, 0x7, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, 0x7, TX_DW6_ANTSEL_A_MSK, TX_DW6_ANTSEL_A_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, 0x7, TX_DW6_ANTSEL_B_MSK, TX_DW6_ANTSEL_B_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, 0x3, TX_DW6_ANT_MAP_A_MSK, TX_DW6_ANT_MAP_A_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, 0x3, TX_DW6_ANT_MAP_B_MSK, TX_DW6_ANT_MAP_B_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, 0x3, TX_DW6_ANT_MAP_C_MSK, TX_DW6_ANT_MAP_C_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6, 0x3, TX_DW6_ANT_MAP_D_MSK, TX_DW6_ANT_MAP_D_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword7, 0xF, TX_DW7_NTX_MAP_MSK, TX_DW7_NTX_MAP_SH);
#endif

#if 0
	if(queueIndex == HCI_TX_DMA_QUEUE_BE) {
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, 0x1, TX_DW3_USERATE_MSK, TX_DW3_USERATE_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, 0x7F, TX_DW4_DATARATE_MSK, TX_DW4_DATARATE_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, 0x1, TX_DW4_TRY_RATE_MSK, TX_DW4_TRY_RATE_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, 0x1, TX_DW5_DATA_SHORT_MSK, TX_DW5_DATA_SHORT_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, 0x3, TX_DW5_DATA_BW_MSK, TX_DW5_DATA_BW_SH);
	}
#endif

#if 0
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 0x1F, TX_DW1_RATE_ID_MSK, TX_DW1_RATE_ID_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, 0x1, TX_DW3_DISDATAFB_MSK, TX_DW3_DISDATAFB_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, 0x1, TX_DW3_DISRTSFB_MSK, TX_DW3_DISRTSFB_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, 0xF, TX_DW4_RTS_RATEFB_LMT_MSK, TX_DW4_RTS_RATEFB_LMT_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, 0x1F, TX_DW4_DATA_RATEFB_LMT_MSK, TX_DW4_DATA_RATEFB_LMT_SH);
#endif

#if 0
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, 0x1, TX_DW4_RTY_LMT_EN_MSK, TX_DW4_RTY_LMT_EN_SH);
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, 0x3F, TX_DW4_DATA_RT_LMT_MSK, TX_DW4_DATA_RT_LMT_SH);
	// TODO: bug ? still copy this field into desc
	SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, 0x3, TX_DW8_BAR_RTY_TH_MSK, TX_DW8_BAR_RTY_TH_SH);
#endif
#ifdef TRXBD_CACHABLE_REGION
//    _dma_cache_wback((unsigned long)((PVOID)ptx_desc-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(TX_DESC_88XX));
#endif //#ifdef TRXBD_CACHABLE_REGION
}    




HAL_IMEM
VOID
FillShortCutTxDesc88XX_V1(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      PVOID           pDescData,
    IN      PVOID           pTxDesc
)
{
    PTX_DESC_DATA_88XX  pdesc_data  = (PTX_DESC_DATA_88XX)pDescData;
    PTX_DESC_88XX       ptx_desc    = (PTX_DESC_88XX)pTxDesc;
   
    // tx shortcut can reuse TXDESC while 1) no security or 2) hw security
    // if no security iv == 0, so adding iv is ok for no security and hw security
    u2Byte  TX_DESC_TXPKTSIZE   = pdesc_data->hdrLen + pdesc_data->llcLen + pdesc_data->frLen + pdesc_data->iv;
    BOOLEAN TX_DESC_BK          = pdesc_data->bk;
    BOOLEAN TX_DESC_NAVUSEHDR   = pdesc_data->navUseHdr;
    u2Byte  TX_DESC_SEQ         = GetSequence(pdesc_data->pHdr);
    u1Byte  TX_DESC_DATA_STBC   = pdesc_data->dataStbc;
    BOOLEAN TX_DESC_RTY_LMT_EN  = pdesc_data->rtyLmtEn;   
    u1Byte  TX_DESC_DATA_RT_LMT = pdesc_data->dataRtyLmt;
    u4Byte  TXDESCSize;

#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    BOOLEAN TX_DESC_BMC              = (pdesc_data->smhEn == TRUE) ? 
    					((HAL_IS_MCAST(GetEthDAPtr((pu1Byte)pdesc_data->pHdr))) ? 1 : 0) : 
				        ((HAL_IS_MCAST(GetAddr1Ptr((pu1Byte)pdesc_data->pHdr))) ? 1 : 0);   // when multicast or broadcast, BMC = 1
#else
    BOOLEAN TX_DESC_BMC              = (HAL_IS_MCAST(GetAddr1Ptr((pu1Byte)pdesc_data->pHdr))) ? 1 : 0;   // when multicast or broadcast, BMC = 1        
#endif


#if IS_EXIST_RTL8197FEM
    if (IS_HARDWARE_TYPE_8197F(Adapter) ) {
        TXDESCSize = SIZE_TXDESC_88XX;
    }
#endif  //IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM)

#if IS_EXIST_RTL8822BE
    if ( IS_HARDWARE_TYPE_8822B(Adapter))  {
        TXDESCSize = SIZE_TXDESC_88XX_V1;
		
		TX_DESC_BK = 0;
		
		if(TX_DESC_RTY_LMT_EN){
			if(TX_DESC_DATA_RT_LMT <= Adapter->pshare->rf_ft_var.mu_retry)
			TX_DESC_DATA_RT_LMT = (Adapter->pshare->rf_ft_var.mu_retry + 1);
		}
    }
#endif //IS_EXIST_RTL8822BE

    SET_TX_DESC_TXPKTSIZE(ptx_desc, TX_DESC_TXPKTSIZE);
    SET_TX_DESC_BK(ptx_desc, TX_DESC_BK);
    SET_TX_DESC_NAVUSEHDR(ptx_desc, TX_DESC_NAVUSEHDR);
    SET_TX_DESC_SW_SEQ(ptx_desc, TX_DESC_SEQ);

    if (TX_DESC_RTY_LMT_EN) {
        SET_TX_DESC_RTY_LMT_EN(ptx_desc, TX_DESC_RTY_LMT_EN);    
        SET_TX_DESC_RTS_DATA_RTY_LMT(ptx_desc, TX_DESC_DATA_RT_LMT);  
    } else if (TX_DESC_DATA_RT_LMT) {
        SET_TX_DESC_RTY_LMT_EN(ptx_desc, 0);    
        SET_TX_DESC_RTS_DATA_RTY_LMT(ptx_desc, 0);  
    }

#if 1 //eric-8822 ?? No retry for broadcast & avoid tx hang
	if(TX_DESC_BMC){
		SET_TX_DESC_DATA_RTY_LOWEST_RATE(ptx_desc, 0);
		SET_TX_DESC_RTY_LMT_EN(ptx_desc, 1);    
        	SET_TX_DESC_RTS_DATA_RTY_LMT(ptx_desc, 0);  
	}
#endif

    // for force tx rate
    if (HAL_VAR_TX_FORCE_RATE != 0xff) {       
        SET_TX_DESC_USE_RATE(ptx_desc, pdesc_data->useRate);
        SET_TX_DESC_DISRTSFB(ptx_desc, pdesc_data->disRTSFB);
        SET_TX_DESC_DISDATAFB(ptx_desc, pdesc_data->disDataFB);
        SET_TX_DESC_DATARATE(ptx_desc, pdesc_data->dataRate);

#if IS_EXIST_RTL8822BE
		if ( IS_HARDWARE_TYPE_8822B(Adapter)) 
		SET_TX_DESC_MU_DATARATE(ptx_desc, pdesc_data->dataRate); 
#endif
    }

#if (BEAMFORMING_SUPPORT == 1)
    SET_TX_DESC_DATA_STBC(ptx_desc, TX_DESC_DATA_STBC);     
#endif

#ifdef TRXBD_CACHABLE_REGION
    _dma_cache_wback((unsigned long)((PVOID)ptx_desc-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), TXDESCSize);
#endif //#ifdef TRXBD_CACHABLE_REGION
}

#if CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
HAL_IMEM
VOID
FillHwShortCutTxDesc88XX_V1(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pDescData
)
{
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PTX_DESC_88XX                   ptx_desc;
    PTX_DESC_DATA_88XX              pdesc_data = (PTX_DESC_DATA_88XX)pDescData;
    u4Byte                          TXDESCSize;
          
#if IS_EXIST_RTL8197FEM
        if (IS_HARDWARE_TYPE_8197F(Adapter) ) {
            TXDESCSize = SIZE_TXDESC_88XX;
        }
#endif  //IS_EXIST_RTL8814AE || IS_EXIST_RTL8197FEM)
    
#if IS_EXIST_RTL8822BE
        if ( IS_HARDWARE_TYPE_8822B(Adapter))  {
            TXDESCSize = SIZE_TXDESC_88XX_V1;
        }
#endif //IS_EXIST_RTL8822BE

	// Dword 0
	u2Byte  TX_DESC_TXPKTSIZE		= pdesc_data->hdrLen + pdesc_data->llcLen + pdesc_data->frLen + pdesc_data->iv;
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    u1Byte	TX_DESC_OFFSET			= ((pdesc_data->smhEn == TRUE) ? (HAL_HW_TXSC_HDR_CONV_ADD_OFFSET+TXDESCSize) : TXDESCSize);
#else
	u1Byte	TX_DESC_OFFSET			= TXDESCSize;
#endif

	// Dword 1
	u1Byte  TX_DESC_MACID       	= pdesc_data->macId;
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    u1Byte  TX_DESC_PKT_OFFSET      = 0; // unit: 8 bytes. Early mode: 1 units, (8 * 1 = 8 bytes for early mode info)
#endif

	// Dword 3
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    // Actually, no any conditions can run the (pdesc_data->hdrLen >> 1)  case in our driver.
    // Because, while calling this function, it must enable reuse TXDESC and hdr conv.
	u1Byte	TX_DESC_WHEADER_LEN		= ((pdesc_data->smhEn == TRUE) ? HAL_HW_TXSC_WHEADER_LEN : (pdesc_data->hdrLen >> 1)); // unit: 2 bytes
#endif // CFG_HAL_HW_TX_SHORTCUT_HDR_CONV

	// Dword 8
	//BOOLEAN	TX_DESC_STW_ANT_DIS		= pdesc_data->stwAntDis;
	//BOOLEAN	TX_DESC_STW_RATE_DIS	= pdesc_data->stwRateDis;
	//BOOLEAN	TX_DESC_STW_RB_DIS		= pdesc_data->stwRbDis;
	//BOOLEAN	TX_DESC_STW_PKTRE_DIS 	= pdesc_data->stwPktReDis;
#if 0
3.)	STW_ANT_DIS:  
ant_mapA, ant_mapB, ant_mapC, ant_mapD, ANTSEL_A, ANTSEL_B, Ntx_map, TXPWR_OFFSET 
4.)	STW_RATE_DIS:  
USE_RATE, Data rate, DATA_SHORT, DATA_BW, TRY_RATE
5.)	STW_RB_DIS:  
RATE_ID, DISDATAFB, DISRTSFB, RTS_RATEFB_LMT, DATA_RATEFB_LMT
6.)	STW_PKTRE_DIS:  
RTY_LMT_EN,  DATA_RT_LMT,  BAR_RTY_TH
#endif

	BOOLEAN	TX_DESC_STW_EN			 = pdesc_data->stwEn;
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
	BOOLEAN	TX_DESC_SMH_EN 			 = pdesc_data->smhEn;
#endif // CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
	
	//Dword 9
#if CFG_HAL_HW_SEQ
    u2Byte TX_DESC_SEQ               = 0;
#else
    u2Byte TX_DESC_SEQ               = GetSequence(pdesc_data->pHdr);
#endif


#if 0 //(CFG_HAL_HW_TX_SHORTCUT_REUSE_TXDESC||CFG_HAL_HW_TX_SHORTCUT_HDR_CONV)
    if (pdesc_data->stwEn != 0 || pdesc_data->smhEn != 0)
        printk("%s(%d): stwEn:0x%x, smhEn:0x%x \n", __FUNCTION__, __LINE__, pdesc_data->stwEn, pdesc_data->smhEn);
#endif

#if 0 // show TX Rpt Info
{
    int baseReg = 0x8080, offset, i;
    int lenTXDESC = 10, lenHdrInfo = 20;

	HAL_RTL_W8(0x106, 0x7F);
	HAL_RTL_W32(0x140, 0x662);

    for(i = 0; i < lenTXDESC; i++) {
#if 1
        printk("%08X ", (u4Byte)GET_DESC(HAL_RTL_R32(baseReg)));
#else
        printk("0x%x:%08X ", baseReg, (u4Byte)GET_DESC(HAL_RTL_R32(baseReg)));
#endif
        if (i%4==3)
            printk("\n");
        baseReg += 4;
    }
    printk("\n");

    for(i = 0; i < lenHdrInfo; i++) {
#if 1
        printk("%08X ", (u4Byte)GET_DESC(HAL_RTL_R32(baseReg)));
#else
        printk("0x%x:%08X ", baseReg, (u4Byte)GET_DESC(HAL_RTL_R32(baseReg)));
#endif

        if (i%4==3)
            printk("\n");
        baseReg += 4;
    }
    printk("\n");

#if 0
	printk("%08X %08X %08X %08X \n%08X %08X %08X %08X \n%08X %08X \n", 
                (u4Byte)GET_DESC(HAL_RTL_R32(0x8080)), (u4Byte)GET_DESC(HAL_RTL_R32(0x8084)), 
                (u4Byte)GET_DESC(HAL_RTL_R32(0x8088)), (u4Byte)GET_DESC(HAL_RTL_R32(0x808c)),
                (u4Byte)GET_DESC(HAL_RTL_R32(0x8090)), (u4Byte)GET_DESC(HAL_RTL_R32(0x8094)),
                (u4Byte)GET_DESC(HAL_RTL_R32(0x8098)), (u4Byte)GET_DESC(HAL_RTL_R32(0x809c)),
                (u4Byte)GET_DESC(HAL_RTL_R32(0x80a0)), (u4Byte)GET_DESC(HAL_RTL_R32(0x80a4)));
#endif
}
#endif

    ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    cur_q       = &(ptx_dma->tx_queue[queueIndex]);
    ptx_desc    = ((PTX_DESC_88XX)(cur_q->ptx_desc_head)) + cur_q->host_idx;

    //Clear All Bit
    PlatformZeroMemory((PVOID)ptx_desc, sizeof(TX_DESC_88XX));

    //4 Set Dword0
    SET_TX_DESC_TXPKTSIZE_NO_CLR(ptx_desc, TX_DESC_TXPKTSIZE);
    SET_TX_DESC_OFFSET_NO_CLR(ptx_desc, TX_DESC_OFFSET);    


	//4 Set Dword1
    SetTxDescQSel88XX_V1(Adapter, queueIndex, ptx_desc, pdesc_data->tid);        
    if ( (queueIndex >= HCI_TX_DMA_QUEUE_HI0) && (queueIndex <= HCI_TX_DMA_QUEUE_HI7) ) {
        //MacID has written in SetTxDescQSel88XX()
    } else {
        SET_TX_DESC_MACID_NO_CLR(ptx_desc, TX_DESC_MACID);
    }
#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
    SET_TX_DESC_PKT_OFFSET_NO_CLR(ptx_desc, TX_DESC_PKT_OFFSET);
#endif

	//4 Set Dword3
	
    //4 Set Dword8
	//4 Set Dword9    
#if CFG_HAL_HW_SEQ
    SET_TX_DESC_EN_HWSEQ_NO_CLR(ptx_desc,1);
#else
    SET_TX_DESC_SW_SEQ_NO_CLR(ptx_desc,TX_DESC_SEQ);
#endif


#if IS_RTL88XX_MAC_V2
    if ( _GET_HAL_DATA(Adapter)->MacVersion.is_MAC_v2) {


        SET_TX_DESC_STW_EN_NO_CLR(ptx_desc,TX_DESC_STW_EN);

		//SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, TX_DESC_STW_ANT_DIS, TX_DW8_STW_ANT_DIS_MSK, TX_DW8_STW_ANT_DIS_SH);
		//SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, TX_DESC_STW_RATE_DIS, TX_DW8_STW_RATE_DIS_MSK, TX_DW8_STW_RATE_DIS_SH);
		//SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, TX_DESC_STW_RB_DIS, TX_DW8_STW_RB_DIS_MSK, TX_DW8_STW_RB_DIS_SH);
		//SET_DESC_FIELD_NO_CLR(ptx_desc->Dword8, TX_DESC_STW_PKTRE_DIS, TX_DW8_STW_PKTRE_DIS_MSK, TX_DW8_STW_PKTRE_DIS_SH);

        // for force tx rate
        if (HAL_VAR_TX_FORCE_RATE != 0xff) {
        /*                    
             STW_RATE_DIS:  USE_RATE, Data rate, DATA_SHORT, DATA_BW, TRY_RATE
             STW_RB_DIS:     RATE_ID, DISDATAFB, DISRTSFB, RTS_RATEFB_LMT, DATA_RATEFB_LMT
            */            

            SET_TX_DESC_STW_ANT_DIS_NO_CLR(ptx_desc, 1);
            SET_TX_DESC_STW_RB_DIS_NO_CLR(ptx_desc, 1);
        
            SET_TX_DESC_USE_RATE_NO_CLR(ptx_desc, pdesc_data->useRate);
            SET_TX_DESC_DISRTSFB_NO_CLR(ptx_desc, pdesc_data->disRTSFB);
            SET_TX_DESC_DISDATAFB_NO_CLR(ptx_desc, pdesc_data->disDataFB);
            SET_TX_DESC_DATARATE_NO_CLR(ptx_desc, pdesc_data->dataRate);

            SET_TX_DESC_RATE_ID_NO_CLR(ptx_desc,pdesc_data->rateId);
            SET_TX_DESC_DATA_BW_NO_CLR(ptx_desc,pdesc_data->dataBW);

            SET_TX_DESC_DATA_SHORT_NO_CLR(ptx_desc,1);
        }

#if CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
        SET_TX_DESC_WHEADER_LEN_NO_CLR(ptx_desc,TX_DESC_WHEADER_LEN);
        SET_TX_DESC_SMH_EN_NO_CLR(ptx_desc,TX_DESC_SMH_EN);
#endif // CFG_HAL_HW_TX_SHORTCUT_HDR_CONV
	}
#endif // IS_RTL88XX_MAC_V2


}
#endif


#endif // #if IS_EXIST_RTL8822BE || IS_EXIST_RTL8197FEM


