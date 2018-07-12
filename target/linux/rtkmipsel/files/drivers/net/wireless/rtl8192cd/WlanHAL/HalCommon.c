/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalCommon.c
	
Abstract:
	Defined HAL common Function
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-27 Filen            Create.	
--*/

#include "HalPrecomp.h"

u4Byte txBaseAddr[HCI_TX_DMA_QUEUE_MAX_NUM];
u4Byte txDescBaseAddr[HCI_TX_DMA_QUEUE_MAX_NUM];
u4Byte rxBaseAddr[1];

VOID
HalGeneralDummy(
	IN	HAL_PADAPTER    Adapter
	)
{
}

RT_STATUS 
HAL_ReadTypeID(
	IN	HAL_PADAPTER	Adapter
	)
{
    u1Byte                  value8;
    u4Byte                  value32;
    u1Byte                  HCI;
    RT_STATUS               rtResult = RT_STATUS_FAILURE;

    value8 = PlatformEFIORead1Byte(Adapter, REG_SYS_CFG2);
    HCI    = PlatformEFIORead1Byte(Adapter, REG_SYS_STATUS1);

    RT_TRACE(COMP_INIT, DBG_LOUD, ("REG_SYS_CFG2(0xFC): 0x%x \n, REG_SYS_STATUS1(0xF4): 0x%x\n", value8, HCI));
    
    switch(value8)
    {
#if     IS_EXIST_RTL8881AEM
        case HAL_HW_TYPE_ID_8881A:
            _GET_HAL_DATA(Adapter)->HardwareType = HARDWARE_TYPE_RTL8881AEM;
            rtResult = RT_STATUS_SUCCESS;
            break;
#endif            

#if     (IS_EXIST_RTL8192EE || IS_EXIST_RTL8192EU)
        case HAL_HW_TYPE_ID_8192E:
            // Eric KY porting HAL temp
            switch(BIT_GET_HCI_SEL(HCI)) {
                //  2'b01: USB-MF
                //  2'b10: PCIE-MF
                //  2'b11: MIX mode, BT-USB1.1, WFIF-PCIE
                //  2'b00: SDIO_UART
                
                case 0x2:
                case 0x3:
                _GET_HAL_DATA(Adapter)->HardwareType = HARDWARE_TYPE_RTL8192EE;
                    rtResult = RT_STATUS_SUCCESS;
                    break;
                    
                case 0x1:
                _GET_HAL_DATA(Adapter)->HardwareType = HARDWARE_TYPE_RTL8192EU;
                    rtResult = RT_STATUS_SUCCESS;
                    break;
                    
                default:
                    RT_TRACE(COMP_INIT, DBG_SERIOUS, (" 92E HCI_SEL Error(0x%x) \n", HCI));
                    break;
            }
            break;
#endif

#if IS_EXIST_RTL8192ES
	case HAL_HW_TYPE_ID_8192E:
		_GET_HAL_DATA(Adapter)->HardwareType = HARDWARE_TYPE_RTL8192ES;
		rtResult = RT_STATUS_SUCCESS;
		break;
#endif

#if     IS_RTL8814A_SERIES
        case HAL_HW_TYPE_ID_8814A:
            switch(BIT_GET_HCI_SEL_V2(HCI)) {
                //BIT_HCI_SEL_V1
                //  01: USB-Phy
                //  10: HSIC
                //  00: SDIO
                //  11: PCIe

                case 0x3:
                    _GET_HAL_DATA(Adapter)->HardwareType = HARDWARE_TYPE_RTL8814AE;
                    rtResult = RT_STATUS_SUCCESS;
                    break;
                    
                case 0x1:
                    _GET_HAL_DATA(Adapter)->HardwareType = HARDWARE_TYPE_RTL8814AU;
            rtResult = RT_STATUS_SUCCESS;
            break;
                    
                default:
                    RT_TRACE(COMP_INIT, DBG_SERIOUS, (" 8814A HCI_SEL Error(0x%x) \n", HCI));
                    break;
            }
            break;
#endif            

#if IS_RTL8197F_SERIES
        case HAL_HW_TYPE_ID_8197F:
            _GET_HAL_DATA(Adapter)->HardwareType = HARDWARE_TYPE_RTL8197FEM;
            rtResult = RT_STATUS_SUCCESS;
        break;
#endif // #if IS_RTL8197F_SERIES
#if IS_RTL8822B_SERIES
        case HAL_HW_TYPE_ID_8822B:
            printk("IS_RTL8822B_SERIES value8 = %x \n",value8);

            // TODO : if need other interface, need to read HCI 
            _GET_HAL_DATA(Adapter)->HardwareType = HARDWARE_TYPE_RTL8822BE;
            rtResult = RT_STATUS_SUCCESS;
        break;    
#endif // #if IS_RTL8822B_SERIES

        default:
            RT_TRACE(COMP_INIT, DBG_SERIOUS, (" Chip TypeID Error (REG_SYS_CFG2: 0x%x) \n", value8));
            break;
    }

    RT_TRACE(COMP_INIT, DBG_LOUD, (" HardwareType: %d \n", _GET_HAL_DATA(Adapter)->HardwareType));

    //3 Check if it is test chip
    value32 = PlatformEFIORead4Byte(Adapter, REG_SYS_CFG1);
    if ( value32 & BIT23 ) {
        _GET_HAL_DATA(Adapter)->bTestChip = _TRUE;
    }
    else {
        _GET_HAL_DATA(Adapter)->bTestChip = _FALSE;
    }

    //3 Check CutVersion
    _GET_HAL_DATA(Adapter)->cutVersion = GET_BIT_CHIP_VER(value32);
    
// recognize 92E b /c cut 	
#if  IS_EXIST_RTL8192EE 
	if(_GET_HAL_DATA(Adapter)->bTestChip) {
		if( _GET_HAL_DATA(Adapter)->HardwareType == HARDWARE_TYPE_RTL8192EE) {
			if(((value32>>12)& 0xf) == 0x0)
				_GET_HAL_DATA(Adapter)->cutVersion	= ODM_CUT_B;
			else if(((value32>>12)& 0xf) == 0x2)
				_GET_HAL_DATA(Adapter)->cutVersion	= ODM_CUT_C;
		}
	} else {
		if( _GET_HAL_DATA(Adapter)->HardwareType == HARDWARE_TYPE_RTL8192EE) {
			if(((value32>>12)& 0xf) == 0x0)
				_GET_HAL_DATA(Adapter)->cutVersion	= ODM_CUT_A;
			else if(((value32>>12)& 0xf) == 0x1)
				_GET_HAL_DATA(Adapter)->cutVersion	= ODM_CUT_B;
			else if(((value32>>12)& 0xf) == 0x2)
				_GET_HAL_DATA(Adapter)->cutVersion	= ODM_CUT_C;
			else if(((value32>>12)& 0xf) == 0x3)
				_GET_HAL_DATA(Adapter)->cutVersion	= ODM_CUT_D;		
		}
	}
#endif

    return rtResult;
}

static u1Byte   hal_idx = 0;

u1Byte
GetHALIndex(
    VOID
)
{
    u1Byte  CurrentIndex = hal_idx;

    hal_idx++;
    
    return CurrentIndex;
}

VOID
ResetHALIndex(
    VOID
)
{
    hal_idx = 0;
}

VOID
DecreaseHALIndex(
    VOID
)
{
    hal_idx--;
}


RT_STATUS
HalAssociateNic(
    HAL_PADAPTER        Adapter,
    BOOLEAN			    IsDefaultAdapter    
)
{
    RT_STATUS       status  = RT_STATUS_FAILURE;

    if ( IsDefaultAdapter ) {

	Adapter->pshare->use_hal = 1;

#if IS_RTL88XX_GENERATION

        //Mapping
        MappingVariable88XX(Adapter);

        //Allocate Memory
        Adapter->HalFunc = (PVOID)HALMalloc(Adapter, sizeof(HAL_INTERFACE));
        Adapter->HalData = (PVOID)HALMalloc(Adapter, sizeof(HAL_DATA_TYPE));

        if ( (NULL == Adapter->HalFunc) || (NULL == Adapter->HalData) ) {
            RT_TRACE(COMP_INIT, DBG_SERIOUS, ("Allocate HAL Memory Failed\n"));
            return RT_STATUS_FAILURE;
        }
        else {
            RT_TRACE(COMP_INIT, DBG_LOUD, ("HalFunc(0x%p), HalData(0x%p)\n", Adapter->HalFunc, Adapter->HalData) );
            PlatformZeroMemory(Adapter->HalFunc, sizeof(HAL_INTERFACE));
            PlatformZeroMemory(Adapter->HalData, sizeof(HAL_DATA_TYPE));
        }

        status = HAL_ReadTypeID(Adapter);
        if ( RT_STATUS_SUCCESS != status ) {
            RT_TRACE(COMP_INIT, DBG_SERIOUS, (" HAL_ReadTypeID Error\n"));
            return status;
        }
        else {
            RT_TRACE(COMP_INIT, DBG_LOUD, (" HAL_ReadTypeID OK\n"));
        }

#if IS_EXIST_RTL8192EE
        if ( IS_HARDWARE_TYPE_8192EE(Adapter) ) {
            RT_TRACE(COMP_INIT, DBG_LOUD, (" IS_HARDWARE_TYPE_8192EE\n"));
            status = hal_Associate_8192EE(Adapter, TRUE);    
            if (RT_STATUS_SUCCESS != status) {
                return status;
            }
        }
#endif

#if IS_EXIST_RTL8192ES
	if ( IS_HARDWARE_TYPE_8192ES(Adapter) ) {
		status = hal_Associate_8192ES(Adapter, TRUE);    
		if (RT_STATUS_SUCCESS != status) {
			return status;
		}
	}
#endif

#if IS_EXIST_RTL8881AEM
        if ( IS_HARDWARE_TYPE_8881A(Adapter) ) {
            RT_TRACE(COMP_INIT, DBG_LOUD, (" IS_HARDWARE_TYPE_8881A\n"));
            status = hal_Associate_8881A(Adapter, TRUE);
            if (RT_STATUS_SUCCESS != status) {
                return status;
            }
        }
#endif

#if IS_EXIST_RTL8814AE
        if ( IS_HARDWARE_TYPE_8814AE(Adapter) ) {
            RT_TRACE(COMP_INIT, DBG_LOUD, (" IS_HARDWARE_TYPE_8814AE\n"));
            status = hal_Associate_8814AE(Adapter, TRUE);
            if (RT_STATUS_SUCCESS != status) {
                return status;
            }
        }
#endif

#if IS_EXIST_RTL8197FEM
        if ( IS_HARDWARE_TYPE_8197F(Adapter) ) {
            RT_TRACE(COMP_INIT, DBG_LOUD, (" IS_HARDWARE_TYPE_8197F\n"));
            status = hal_Associate_8197F(Adapter, TRUE);
            if (RT_STATUS_SUCCESS != status) {
                return status;
            }
        }
#endif

#if IS_EXIST_RTL8822BE
        if ( IS_HARDWARE_TYPE_8822BE(Adapter) ) {
            RT_TRACE(COMP_INIT, DBG_LOUD, (" IS_HARDWARE_TYPE_8822BE\n"));
            status = hal_Associate_8822BE(Adapter, TRUE);
            if (RT_STATUS_SUCCESS != status) {
                return status;
            }
        }
#endif
        printk("MACHAL_version_init\n");

        MACHAL_version_init(Adapter);
#endif  //IS_RTL88XX_GENERATION

        //Assign HAL device index
        _GET_HAL_DATA(Adapter)->devIdx = GetHALIndex();
    }
    else {
        // Virtual Adapter
    }
    
    return  status;
}

RT_STATUS
HalDisAssociateNic(
    HAL_PADAPTER        Adapter,
    BOOLEAN			    IsDefaultAdapter    
)
{


    if ( IsDefaultAdapter ) {
#if IS_RTL88XX_GENERATION
        //Free Memory
        if ( Adapter->HalFunc ) {
            HAL_free(Adapter->HalFunc);
        }

        if ( Adapter->HalData ) {
            HAL_free(Adapter->HalData);            
        }
#endif //  IS_RTL88XX_GENERATION
    }
    else {
        // Virtual Adapter        
    }

    return RT_STATUS_SUCCESS;
}

VOID SoftwareCRC32 (
    IN  pu1Byte     pBuf,
    IN  u2Byte      byteNum,
    OUT pu4Byte     pCRC32
)
{
    u4Byte a, b;
    u1Byte mask, smask;
    u4Byte CRCMask = 0x00000001, POLY = 0xEDB88320;
    u4Byte CRC_32 = 0xffffffff;
    u4Byte i,j;

    smask = 0x01;
    for(i=0; i<byteNum; i++)
    {
        mask = smask;
        for(j=0; j<8; j++)
        {
            a = ((CRC_32 & CRCMask) != 0);
            b = ((pBuf[i] & mask) != 0);

            CRC_32 >>= 1;
            mask <<= 1;

            if(a^b)
                CRC_32 ^= POLY;
        }
    }
    *(pCRC32) = CRC_32 ^ 0xffffffff;
    *(pCRC32) = HAL_cpu_to_le32(*(pCRC32));
}

VOID SoftwareCRC32_RXBuffGather (
    IN  pu1Byte     pPktBufAddr,
    IN  pu2Byte     pPktBufLen,  
    IN  u2Byte      pktNum,
    OUT pu4Byte     pCRC32
)
{
    u4Byte a, b;
    u1Byte mask, smask;
    u4Byte CRCMask = 0x00000001, POLY = 0xEDB88320;
    u4Byte CRC_32 = 0xffffffff;
    u4Byte i,j;
    u1Byte num;

    smask = 0x01;

    for (num = 0; num < pktNum; num++) 
    {
        for(i=0; i< pPktBufLen[num]; i++)
        {
            mask = smask;
            for(j=0; j<8; j++)
            {
                a = ((CRC_32 & CRCMask) != 0);
                b = ((  *((pu1Byte)( *((pu4Byte)pPktBufAddr + num) )+ i)   & mask) != 0);

                CRC_32 >>= 1;
                mask <<= 1;

                if(a^b)
                    CRC_32 ^= POLY;
            }
        }
    }
    
    *(pCRC32) = CRC_32 ^ 0xffffffff;
    *(pCRC32) = HAL_cpu_to_le32(*(pCRC32));
}

VOID DumpTxPktBuf(
    IN	HAL_PADAPTER        Adapter
)
{
    u4Byte  addr;
    u4Byte  cnt = 0;
    u4Byte  num = 5;
    u4Byte  value;
    u4Byte  dataL, dataH;
    u4Byte  round = 0;

    addr = HAL_RTL_R8(REG_BCNQ_INFO) * 256 / 8;

    #if (IS_EXIST_RTL8881AEM || IS_EXIST_RTL8192EE || IS_EXIST_RTL8814AE )    
    if ( IS_HARDWARE_TYPE_8881A(Adapter) || IS_HARDWARE_TYPE_8192EE(Adapter) || IS_HARDWARE_TYPE_8814AE(Adapter) ) 
        HAL_RTL_W8(REG_PKT_BUFF_ACCESS_CTRL,0x69);
    #endif

    RT_TRACE_F(COMP_SEND, DBG_TRACE, ("Addr:%lx \n", addr));

    do 
    {
        HAL_RTL_W32(REG_PKTBUF_DBG_CTRL, addr);
        value = 0;        
        do
        {
            value = HAL_RTL_R32(REG_PKTBUF_DBG_CTRL) & BIT23;        
			if (++round > 10000) {
				panic_printk("%s[%d] while (1) goes too many\n", __FUNCTION__, __LINE__);
				break;
			}
        } while(value != BIT23);

        dataL = HAL_RTL_R32(REG_PKTBUF_DBG_DATA_L);
        dataH = HAL_RTL_R32(REG_PKTBUF_DBG_DATA_H);

        RT_TRACE_F(COMP_SEND, DBG_TRACE, ("Data[%ld]: %08lx, %08lx \n", cnt, \
            (u4Byte)GET_DESC(dataL), (u4Byte)GET_DESC(dataH)));

        addr++;
        cnt++;

    } while(cnt < num);
}

u1Byte 
GetXorWithCRC(
    IN u1Byte a,
    IN u1Byte b
)
{
	if((a^b)&0x1)
		return 1;
	else
		return 0;
}
	

u1Byte 
CRC5(
    IN pu1Byte dwInput,
    IN u1Byte len
)
{
    u1Byte poly5 = 0x05;  
    u1Byte crc5  = 0x1f;
    u1Byte i  = 0x0 ;
    u1Byte udata;
    u1Byte BitCount;


	for(i=0 ; i<len ; i++)
	{
		udata = *(dwInput+i);
		BitCount = 0;
		while(BitCount!=8)
		{        
	        if (GetXorWithCRC(udata>>(BitCount),crc5 >> 4)) // bit4 != bit4?			
	        {				
	            crc5 <<= 1;
	            crc5 ^= poly5;
	        }
	        else
	        {
	            crc5 <<= 1;
			}

	        BitCount++;
		}
    }

    crc5 ^= 0x1f;
  
    return (crc5 & 0x1f);
} 





#if IS_EXIST_PCI || IS_EXIST_EMBEDDED
VOID
CheckAddrRange(
    IN      HAL_PADAPTER    Adapter,
    IN      u1Byte          type,
    IN      u4Byte          addr,
    IN      u4Byte          qNum,
    IN      u4Byte          offset
)
{   
    u4Byte lowAddr, hiAddr, TXBDBeaconOffset;

#if IS_RTL8192E_SERIES
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) ) {
        TXBDBeaconOffset = TXBD_BEACON_OFFSET_V1;        
    }
#endif  //IS_RTL8192E_SERIES
    
#if IS_RTL8881A_SERIES
    if ( IS_HARDWARE_TYPE_8881A(Adapter) ) {
        TXBDBeaconOffset = TXBD_BEACON_OFFSET_V2;        
    }
#endif  //IS_RTL8881A_SERIES

    if (type == 0) {
        // TXBD
        lowAddr = txBaseAddr[qNum] + offset * sizeof(TX_BUFFER_DESCRIPTOR);
        hiAddr  = lowAddr + sizeof(TX_BUFFER_DESCRIPTOR);            
    } else if (type == 1) {
        // TX DESC
        lowAddr = HAL_VIRT_TO_BUS(txDescBaseAddr[qNum] + offset * sizeof(TX_DESC_88XX));
        hiAddr  = HAL_VIRT_TO_BUS(lowAddr + sizeof(TX_DESC_88XX));        
    } else if (type == 2) {    
        // RXBD
        lowAddr = rxBaseAddr[qNum] + offset * sizeof(RX_BUFFER_DESCRIPTOR);
        hiAddr  = lowAddr + sizeof(RX_BUFFER_DESCRIPTOR);
    } else if (type == 3) {
        // TXBD Beacon
        lowAddr = txBaseAddr[HCI_TX_DMA_QUEUE_BCN] + offset * TXBDBeaconOffset;
        hiAddr  = lowAddr + sizeof(TX_BUFFER_DESCRIPTOR);            
    } else {
        RT_TRACE_F(COMP_INIT, DBG_TRACE, ("Error: Unknown type \n"));
    }

    if (addr < lowAddr || addr > hiAddr) {
        RT_TRACE_F(COMP_INIT, DBG_TRACE, ("Address error:(%d) 0x%lx, range:(0x%lx, 0x%lx) \n", type, addr, lowAddr, hiAddr));
    } else {
//        RT_TRACE_F(COMP_INIT, DBG_TRACE, ("Address ok:(%d) 0x%lx, range:(0x%lx, 0x%lx) \n", type, addr, lowAddr, hiAddr));
    }
}

VOID
DumpTRXBDAddr(
    IN   HAL_PADAPTER   Adapter
)
{
    PHCI_TX_DMA_MANAGER_88XX    ptx_dma;
    PHCI_RX_DMA_MANAGER_88XX    prx_dma;
    u4Byte                      idx, q_num;

    prx_dma = (PHCI_RX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PRxDMA88XX);
    ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);

    // RXBD
    for (idx = 0; idx < prx_dma->rx_queue[0].total_rxbd_num; idx++) {
        RT_TRACE(COMP_SEND, DBG_TRACE, \
            ("RXBD: 0x%08lx\n", (u4Byte)(prx_dma->rx_queue[0].pRXBD_head + idx)) );
    }

    // RX DESC in Dword1
    for (idx = 0; idx < prx_dma->rx_queue[0].total_rxbd_num; idx++) {
        RT_TRACE(COMP_SEND, DBG_TRACE, \
            ("RX DW1: 0x%08lx\n", (u4Byte)(prx_dma->rx_queue[0].pRXBD_head[idx].Dword1)) );
    }   

    // TXBD
    for (q_num = 0; q_num <= HCI_TX_DMA_QUEUE_HI7; q_num++) {
        for (idx = 0; idx < ptx_dma->tx_queue[q_num].total_txbd_num; idx++) {
            RT_TRACE(COMP_SEND, DBG_TRACE, \
                ("TXBD: 0x%08lx\n", (u4Byte)(ptx_dma->tx_queue[q_num].pTXBD_head + idx)) );
        }
    }

    // TX DESC
    for (q_num = 0; q_num <= HCI_TX_DMA_QUEUE_HI7; q_num++) {
        for (idx = 0; idx < ptx_dma->tx_queue[q_num].total_txbd_num; idx++) {
            RT_TRACE(COMP_SEND, DBG_TRACE, \
                ("TXDESC: 0x%08lx\n", (u4Byte)((PTX_DESC_88XX)ptx_dma->tx_queue[q_num].ptx_desc_head + idx)) );
        }        
    }

    // TX DESC in Dword1
    for (q_num = 0; q_num <= HCI_TX_DMA_QUEUE_HI7; q_num++) {
        for (idx = 0; idx < ptx_dma->tx_queue[q_num].total_txbd_num; idx++) {
            RT_TRACE(COMP_SEND, DBG_TRACE, \
                ("TX DW1: 0x%08lx\n", (u4Byte)GET_DESC(ptx_dma->tx_queue[q_num].pTXBD_head[idx].TXBD_ELE[0].Dword1)) );
        }
    }   
    
}
#endif // IS_EXIST_PCI || IS_EXIST_EMBEDDED

#if 0 //Filen
#define VALID_ADDR_UPBOUND    0x8FFFFFFF
#define VALID_ADDR_LOWBOUND   0x80000000
#define CHECK_ADDR_VALID(ptr) ((((unsigned long)ptr<=VALID_ADDR_UPBOUND) && ((unsigned long)ptr>=VALID_ADDR_LOWBOUND))? TRUE:FALSE)

BOOLEAN
CheckSKBPtrOk(
    struct rtl8192cd_priv   *priv,
    struct sk_buff          *pskb
)
{
    if (!CHECK_ADDR_VALID(pskb)) {
        printk("Error:SKB address is invalid(0x%x)\n", pskb);
        return FALSE;
    }

    if (!CHECK_ADDR_VALID(pskb->head)) {
        printk("Error:pskb->head address is invalid(0x%x)\n", pskb);
        return FALSE;
    }

    if (!CHECK_ADDR_VALID(pskb->data)) {
        printk("Error:pskb->data address is invalid(0x%x)\n", pskb);
        return FALSE;
    }

    if (!CHECK_ADDR_VALID(pskb->tail)) {
        printk("Error:pskb->tail address is invalid(0x%x)\n", pskb);
        return FALSE;
    }

    if (!CHECK_ADDR_VALID(pskb->end)) {
        printk("Error:pskb->end address is invalid(0x%x)\n", pskb);
        return FALSE;
    }    

    return TRUE;    
}
#endif

BOOLEAN
LoadFileToIORegTable(
    IN  pu1Byte     pRegFileStart,
    IN  u4Byte      RegFileLen,
    OUT pu1Byte     pTableStart,
    IN  u4Byte      TableEleNum
)
{
    u1Byte          *line_head, *next_head;
    u4Byte          u4bRegOffset, u4bRegValue;
    s4Byte          num, len = 0;
    PIOREG_FORMAT   reg_table = (PIOREG_FORMAT)pTableStart;

    next_head = pRegFileStart;

    while(1) {
        line_head = next_head;
        next_head = get_line(&line_head);
        if (line_head == NULL)
            break;
        
        if (line_head[0] == '/')
            continue;

        num = get_offset_val(line_head, &u4bRegOffset, &u4bRegValue);
        if (num > 0) {
            reg_table[len].offset   = u4bRegOffset;
            reg_table[len].value    = u4bRegValue;
            len++;

            if (u4bRegOffset == 0xffff)
                break;

            if (len > TableEleNum)
                return _FALSE;
        }
        else {
            RT_TRACE_F(COMP_INIT, DBG_SERIOUS, ("num(%d)\n", num));
            return _FALSE;
        }
    }

    return _TRUE;
}


BOOLEAN
LoadFileToOneParaTable(
    IN  pu1Byte     pFileStart,
    IN  u4Byte      FileLen,
    OUT pu1Byte     pTableStart,
    IN  u4Byte      TableEleNum
)
{
    u1Byte          *line_head, *next_head;
    u4Byte          u4bValue0, u4bValue1;
    s4Byte          num, len = 0;
    pu4Byte         OneParatable = (pu4Byte)pTableStart;

    next_head = pFileStart;

    while(1) {
        line_head = next_head;
        next_head = get_line(&line_head);
        if (line_head == NULL)
            break;
        
        if (line_head[0] == '/')
            continue;

        num = get_offset_val(line_head, &u4bValue0, &u4bValue1);
        if (num == 1) {
            OneParatable[len]   = u4bValue0;
            len++;

            if (u4bValue0 == 0xffff)
                break;

            if (len > TableEleNum)
                return _FALSE;
        }
        else {
            RT_TRACE_F(COMP_INIT, DBG_SERIOUS, ("num(%d)\n", num));
            return _FALSE;
        }
    }

    return _TRUE;
}





