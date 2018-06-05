/*
* Copyright (C) 2009 Realtek Semiconductor Corp.
* All Rights Reserved.
*
* This program is the proprietary software of Realtek Semiconductor
* Corporation and/or its licensors, and only be used, duplicated,
* modified or distributed under the authorized license from Realtek.
*
* ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER 
* THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
* 
* $Revision: 8334 $
* $Date: 2010-02-09 13:22:42 +0800 (Tue, 09 Feb 2010) $
*
* Purpose : ASIC-level driver implementation for MIB.
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/

#include <rtk_api_ext.h>
#include <rtk_api.h>
#include <asicdrv/rtl8316d_types.h> /*the file in SDK*/
#include <rtl8316d_reg_struct.h>
#include <rtl8316d_debug.h>

//static rtk_portmask_t iso_msk[MAX_LOGIC_PORT];
//uint32 mib_low = 0;


/* Function Name:
 *      rtl8316d_getPortMIBAddr
 * Description:
 *     Get 16D's per-port MIB counter address by rtk_stat_port_type_t.
 * Input:
 *      cntr_idx  - the MIB counter Index
 *      port -  port ID
 * Output:
 *      pMibAddr   - MIB address returned 
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameter, maybe specific MIB counter not supported by RTL8316D.
 * Note:
 *      This function works as a MIB counter address converter. It returns the specific RTL8316D's MIB counter address.
 */
rtk_api_ret_t rtl8316d_getPortMIBAddr(rtk_stat_port_type_t cntr_idx, uint8 port, uint16* pMibAddr)
{
    /*map rtk_stat_port_type_t cntr_idx to RTL8316D's internal MIB counter address*/
    uint8 addrMap[MIB_PORT_CNTR_END] = {
       /*0~9*/
       14, 27, 29, 19, 30, 23, 25, 20, 31, 16,
       /*10~19*/
       41, 40, 34, 35, 36, 37, 38, 39, 21, 22,
       /*20~29*/
       0, 7, 8, 10, 11, 9, 12, 3, 0xFF, 32,
       /*30~*/
       4, 5, 6, 13, 33, 0xFF,
    };

    if (cntr_idx >= MIB_PORT_CNTR_END)
    {
        return RT_ERR_INPUT;
    }
    
    *pMibAddr = addrMap[cntr_idx];

    /*MIB counter retrived not supported by RTL8316D*/
    if (*pMibAddr == 0xFF)
        return RT_ERR_INPUT;
    
     *pMibAddr = *pMibAddr + (port*RTL8316D_PORTMIB_OFFSET);

    return RT_ERR_OK ;
}


/* Function Name:
 *      rtl8316d_setAsicMIBsCounterReset
 * Description:
 *     Set MIBs global/queue manage reset or per-port reset.
 * Input:
 *      greset  - Global reset 
 *      qmreset  - Queue maganement reset
 *      pmask  - Port reset mask  
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameter.
 * Note:
 *      ASIC will clear all MIBs counter by global resetting and clear counters associated with a particular port by mapped port resetting. 
 */
rtk_api_ret_t rtl8316d_setAsicMIBsCounterReset(uint32 greset, uint32 qmreset, uint32 pmask)
{
    uint32 regVal;
    uint32 port;
    uint32 cnt;
    uint32 grant; /*CPU occupy or not*/

    regVal = 0;
    /*MIB counter occupied by CPU now*/
    reg_field_read(RTL8316D_UNIT, MIB_COUNTER_CONTROL0, CPU_OCCUPY_MIB, &grant);
    reg_field_write(RTL8316D_UNIT, MIB_COUNTER_CONTROL0, CPU_OCCUPY_MIB, 1);
    
    reg_field_set(RTL8316D_UNIT, MIB_COUNTER_CONTROL1, SYSCOUNTERRESET, greset, &regVal);

    /*In RTL8316D, queue m and port m share the same reset bit*/
    pmask |= qmreset;
    pmask &= ((uint32)1 << RTL8316D_MAX_PORT) - 1;
    regVal |= pmask; 

    //rtlglue_printf("The register value is: %lx\n", regVal);
    reg_write(RTL8316D_UNIT, MIB_COUNTER_CONTROL1, regVal);

    cnt = 0;
    while(cnt++ < 0x100)
    {
        reg_read(RTL8316D_UNIT, MIB_COUNTER_CONTROL1, &regVal);
        if (regVal == 0)
            break;
    }

    if (cnt == 0x100)
    {
        rtlglue_printf("Time out!\n");
        return RT_ERR_OK;
    }
    
    /*clear overflags */
    if (greset)
    {
       reg_field_write(RTL8316D_UNIT, SYSTEM_MIB_COUNTER_OVERFLOW_FLAG, DOT1DTPLEARNEDENTRYDISCARDSOF_FLG, 1);
    }
    
    for (port = 0; port < RTL8316D_MAX_PORT; port++)
    {
        if (pmask & ((uint32)1 << port))
        {
            reg_write(RTL8316D_UNIT, PORT0_MIBCOUNTER_OVERFLOW_FLAG0+port, 0xFFFFFFFF);
            reg_write(RTL8316D_UNIT, PORT0_MIBCOUNTER_OVERFLOW_FLAG1+port, 0xFFFFFFFF);
        }
    }

    /*return the grant if needed*/
    reg_field_write(RTL8316D_UNIT, MIB_COUNTER_CONTROL0, CPU_OCCUPY_MIB, grant);
        
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8316d_getAsicMIBCounter
 * Description:
 *     Set MIBs global/queue manage reset or per-port reset.
 * Input:
 *      mibAddr  - mib counter address
 * Output:
 *      counterH  - higher 32-bit of the counter returned.
 *      counterL  - lower 32-bit of the counter returned.
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_NULL_POINTER - Invalid input parameter.
 * Note:
 *      ASIC driver, return specificed MIB counter. 
 */
rtk_api_ret_t rtl8316d_getAsicMIBCounter(uint16 mibAddr, uint32* counterH, uint32* counterL)
{
    uint32 regVal;    
    uint32 grant; /*CPU occupy or not*/
    uint32 cnt=0;

    if (counterH == NULL || counterL == NULL)
        return RT_ERR_NULL_POINTER;
    
    reg_field_read(RTL8316D_UNIT, MIB_COUNTER_CONTROL0, CPU_OCCUPY_MIB, &grant);
    /*MIB counter occupied by CPU now*/
    reg_field_write(RTL8316D_UNIT, MIB_COUNTER_CONTROL0, CPU_OCCUPY_MIB, 1);

    reg_field_write(RTL8316D_UNIT, MIB_COUNTER_CONTROL0, ENMIBCOUNTER, 1);
    reg_field_write(RTL8316D_UNIT, MIB_COUNTER_CONTROL0, ENDBMIBCOUNTER, 1);
    
    reg_read(RTL8316D_UNIT, INDIRECT_ACCESS_MIB_COUNTER_CONTROLFOR_CPU, &regVal);
    reg_field_set(RTL8316D_UNIT, INDIRECT_ACCESS_MIB_COUNTER_CONTROLFOR_CPU, CPU_READCOUNTERTRIG, 1, &regVal);
    reg_field_set(RTL8316D_UNIT, INDIRECT_ACCESS_MIB_COUNTER_CONTROLFOR_CPU, CPU_MIBCOUNTERADDR, mibAddr, &regVal);

    reg_write(RTL8316D_UNIT, INDIRECT_ACCESS_MIB_COUNTER_CONTROLFOR_CPU, regVal);

    while(cnt++ < 0x100)
    {
        reg_field_read(RTL8316D_UNIT, INDIRECT_ACCESS_MIB_COUNTER_CONTROLFOR_CPU, CPU_READCOUNTERTRIG, &regVal);
        if (regVal == 0)
            break;
    }

    if (cnt == 0x100)
    {
        rtlglue_printf("Time out!\n");
        return RT_ERR_OK;
    }
    
    reg_read(RTL8316D_UNIT, INDIRECT_ACCESS_MIB_COUNTER_DATA0_FOR_CPU, counterL);
    reg_read(RTL8316D_UNIT, INDIRECT_ACCESS_MIB_COUNTER_DATA1_FOR_CPU, counterH);

    /*return the grant if needed*/
    reg_field_write(RTL8316D_UNIT, MIB_COUNTER_CONTROL0, CPU_OCCUPY_MIB, grant);
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_stat_global_reset
 * Description:
 *     Reset global MIB counter.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameter.
 * Note:
 *      Reset MIB counter of ports. API will use global reset while port mask is all-ports.
 */
rtk_api_ret_t rtk_stat_global_reset2(void)
{
    rtk_api_ret_t retVal;

    if((retVal=rtl8316d_setAsicMIBsCounterReset(TRUE,FALSE,0))!=RT_ERR_OK)
        return retVal; 

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_stat_port_reset
 * Description:
 *     Reset per port MIB counter by port.
 * Input:
 *      port -  port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameter.
 * Note:
 *     Reset MIB counter of ports. API will use global reset while port mask is all-ports.
 */
rtk_api_ret_t rtk_stat_port_reset(rtk_port_t port)
{
    rtk_api_ret_t retVal;

    if(port >= RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 
    
    if((retVal=rtl8316d_setAsicMIBsCounterReset(FALSE,FALSE,(uint32)1<<port))!=RT_ERR_OK)
        return retVal; 
        
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_stat_global_get
 * Description:
 *      get global MIB counter.
 * Input:
 *      cntr_idx -  counter index
 * Output:
 *      pCntr -  counter returned
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_STAT_INVALID_GLOBAL_CNTR - Invalid input parameter.
 * Note:
 *     Get global MIB counter.
 */
rtk_api_ret_t rtk_stat_global_get(rtk_stat_global_type_t cntr_idx, rtk_stat_counter_t *pCntr)
{
    rtk_api_ret_t retVal;
    uint32 cntrH, cntrL;

    if(cntr_idx != DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX)
        return RT_ERR_STAT_INVALID_GLOBAL_CNTR;

    /*only one global MIB counter*/
    if((retVal = rtl8316d_getAsicMIBCounter(0x83E, &cntrH, &cntrL)) != RT_ERR_OK)
        return retVal;

     *pCntr = (uint64)cntrH << 32 | cntrL;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_stat_port_get
 * Description:
 *      Get port MIB counter.
 * Input:
 *      port -  port id.
 *      cntr_idx -  counter index.
 * Output:
 *      pCntr -  counter returned.
 * Return:
 *      RT_ERR_OK  - 
 *      RT_ERR_FAILED   - 
 *      RT_ERR_INPUT - Invalid input parameter.
 * Note:
 *     Get port MIB counter.
 */
rtk_api_ret_t rtk_stat_port_get(rtk_port_t port, rtk_stat_port_type_t cntr_idx, rtk_stat_counter_t *pCntr)
{
    rtk_api_ret_t retVal;
    uint32 cntrH, cntrL;
    uint16 mibAddr;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if(cntr_idx>=MIB_PORT_CNTR_END)
        return RT_ERR_STAT_INVALID_PORT_CNTR;

    if (pCntr == NULL)
        return RT_ERR_INPUT;

    if ((retVal = rtl8316d_getPortMIBAddr(cntr_idx, port, &mibAddr)) != RT_ERR_OK)
        return retVal;

    if((retVal=rtl8316d_getAsicMIBCounter(mibAddr, &cntrH, &cntrL)) != RT_ERR_OK)
        return retVal;

    *pCntr = (uint64)cntrH << 32 | cntrL;

    return RT_ERR_OK;
}


