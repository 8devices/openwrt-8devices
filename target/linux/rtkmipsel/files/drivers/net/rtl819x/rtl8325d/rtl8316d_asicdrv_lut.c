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
* Purpose : ASIC-level driver implementation for LookUp Table (i.e. address table).
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/

#include <rtk_types.h>
#include <rtk_api_ext.h>
#include <asicdrv/rtl8316d_types.h> /*specify the file in SDK ASIC driver directory*/
#include <rtl8316d_table_struct.h> /*specify the file in SDK ASIC driver directory*/
#include <rtl8316d_asicDrv.h>
#include <rtl8316d_asicdrv_tbl.h>
#include <rtl8316d_asicdrv_qos.h>
#include <rtl8316d_reg_struct.h>
#include <rtl8316d_general_reg.h>
#include <rtl8316d_debug.h>


#define MAX_TBL_SIZE 0xf


#define DBG_LUT     1

#if DBG_LUT
#define DEBUG_LUT(x) do {  rtlglue_printf x ; } while(0)
#else
#define DEBUG_LUT(x) do { } while(0)
#endif

static uint8 fwdTblStat[RTL8316D_FWD_NUMBER/8];

#if 0
static rtk_l2_ucastAddr_t lut_tbl[MAX_TBL_SIZE];
static uint8 tbl_init = FALSE;

int32 rtk_web_lutGetTblByIdx(uint32 idx, rtk_l2_ucastAddr_t *l2Param)
{
    static uint8 tbl_init = FALSE;
    rtk_l2_ucastAddr_t invalid_entry;
    uint8 i;
    
    if (tbl_init == FALSE)
    {
        for (i = 0; i < MAX_TBL_SIZE; i++)
        {
            memset(&lut_tbl[i], 0, sizeof(rtk_l2_ucastAddr_t));
        }
        
        printf("Entry 0 is initialized in fowarding table!\n");
        lut_tbl[0].auth = 1;
        lut_tbl[0].fid = 456;
        lut_tbl[0].is_static = TRUE;
        lut_tbl[0].mac.octet[0] = 0x10; 
        lut_tbl[0].mac.octet[1] = 0x10;
        lut_tbl[0].mac.octet[2] = 0x10;
        lut_tbl[0].mac.octet[3] = 0x10;
        lut_tbl[0].mac.octet[4] = 0x10;
        lut_tbl[0].mac.octet[5] = 0x10;
        lut_tbl[0].port = 3;
        lut_tbl[0].sa_block = 0;
        
        tbl_init = TRUE;
    }   

    memset(&invalid_entry, 0, sizeof(rtk_l2_ucastAddr_t));

    if (memcmp(&lut_tbl[idx%MAX_TBL_SIZE],&invalid_entry, sizeof(rtk_l2_ucastAddr_t)) == 0)
        return RT_ERR_FAILED;

    memcpy(l2Param, &lut_tbl[idx%MAX_TBL_SIZE], sizeof(rtk_l2_ucastAddr_t));

    #if 0
    printf("Table Read: Get MAC: %02bx:%02bx:%02bx:%02bx:%02bx:%02bx, fid: %ld\n", l2Param->mac.octet[0],\
        l2Param->mac.octet[1],l2Param->mac.octet[2],l2Param->mac.octet[3],l2Param->mac.octet[4],\
        l2Param->mac.octet[5], l2Param->fid);        
    #endif
    return RT_ERR_OK;
}
#endif

/* Function Name:
 *      rtk_l2_init
 * Description:
 *      Initialize l2 module of the specified device.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      Should initialize l2 module before calling any l2 APIs.
 */  
rtk_api_ret_t rtk_l2_init(void)
{
    rtk_api_ret_t retVal;

    if((retVal=rtl8316d_setAsicLutIpMulticastLookup(ENABLE))!=RT_ERR_OK)
        return retVal;

    if((retVal=rtl8316d_setAsicLutAgeTimerSpeed(0x2A))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}    


/* Function Name:
 *      rtk_l2_addr_add
 * Description:
 *      Set LUT unicast entry.
 * Input:
 *      pMac  - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 *      port - Port id.
 *      fid     - filtering database for the input LUT entry.
 *      is_static  - static or dynamic for the input LUT entry.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_L2_FID - Invalid FID .
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_FAILED - 
 * Note:
 *      If the unicast mac address already existed in LUT, it will udpate the status of the entry. 
 *      Otherwise, it will find an empty or asic auto learned entry to write. If all the entries 
 *      with the same hash value can't be replaced, ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
rtk_api_ret_t rtk_l2_addr_add(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data)
{
    rtl8316d_tblasic_lutParam_t l2Table;
    rtk_api_ret_t retVal;
    uint32 hashAlg;
    uint16 row;
    uint8 column;
    uint8 available;

    DEBUG_LUT(("MAC Address Add Info:\n"));
    DEBUG_LUT(("pMac MAC:%02bx:%02bx:%02bx:%02bx:%02bx:%02bx\n", \
        pMac->octet[0],pMac->octet[1],pMac->octet[2],\
        pMac->octet[3],pMac->octet[4],pMac->octet[5]));
    DEBUG_LUT(("FID:    %08lx\n", pL2_data->fid));
    DEBUG_LUT(("Static: %08lx\n", pL2_data->is_static));
    DEBUG_LUT(("Port:   %08lx\n", pL2_data->port));
         
    /* must be unicast address */
    if((pMac == NULL) || (pMac->octet[0] & 0x1))
        return RT_ERR_MAC;

    if (pL2_data == NULL)
        return RT_ERR_NULL_POINTER; 

    if((pL2_data->port > RTL8316D_MAX_PORT) && (!IS_DABLOCK(pL2_data)))
        return RT_ERR_PORT_ID;    

    /*get maximum when works as IVL*/
    if(pL2_data->fid > RTL8316D_VIDMAX)
        return RT_ERR_L2_FID;

    if(pL2_data->is_static>= RTK_ENABLE_END)
        return RT_ERR_ENABLE; 

    if(pL2_data->sa_block>= RTK_ENABLE_END)
        return RT_ERR_ENABLE; 

    if(pL2_data->auth>= RTK_ENABLE_END)
        return RT_ERR_ENABLE; 

    memset(&l2Table,0,sizeof(l2Table));
    available = 0xFF; /*invalid*/
    
    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.lutEther.fid = pL2_data->fid;

    rtl8316d_addrTbl_hashAlg_get(&hashAlg);
    rtl8316d_addrTbl_hash(hashAlg, NULL, &l2Table); 
    row = l2Table.row;
    
    for (column = 0; column < RTL8316D_LUTWAYMAX; column++)
    {   
        memset(&l2Table,0,sizeof(l2Table));
        l2Table.row = row;
        l2Table.column = column;
        
        rtl8316d_addrTbl_getAsicEntry(hashAlg, &l2Table); 
        if ((l2Table.aet == 0) && (l2Table.isftidx == 0) 
            && (l2Table.lutInfo.age == 0) && (l2Table.lutInfo.stat == 0)
            && (l2Table.lutInfo.macauth == 0) && (l2Table.lutInfo.sablk == 0)
            && (l2Table.lutInfo.portnum != 29))
        {   /*empty entry found, mark it*/
            if (available == 0xFF)
            {
                available = column;
            }
            continue;
        }
        
        /*valid entry, type: FID+MAC --> single port*/
        if (l2Table.aet == 0 && l2Table.isftidx == 0)
        {
            if ((l2Table.lutEther.fid == pL2_data->fid) 
                 && !(memcmp(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN)))
            {   /*entry exists in L2 table already, modify it*/
                l2Table.lutInfo.age = 3;
                l2Table.lutInfo.stat = pL2_data->is_static;
                l2Table.lutInfo.macauth = pL2_data->auth;
                l2Table.lutInfo.sablk = pL2_data->sa_block;
                l2Table.lutInfo.portnum = pL2_data->port;

                retVal = rtl8316d_addrTbl_setAsicEntry(hashAlg, &l2Table);
                return retVal;
            }
        }
    }

    if (available != 0xFF)
    {
        memset(&l2Table,0,sizeof(l2Table));
        memcpy(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN);
        l2Table.aet = 0;
        l2Table.isftidx = 0;
        l2Table.lutEther.fid = pL2_data->fid;
        l2Table.lutInfo.age = 3;
        l2Table.lutInfo.stat = pL2_data->is_static;
        l2Table.lutInfo.macauth = pL2_data->auth;
        l2Table.lutInfo.sablk = pL2_data->sa_block;
        l2Table.lutInfo.portnum = pL2_data->port;
        l2Table.row    = row;
        l2Table.column = available;        
        retVal = rtl8316d_addrTbl_setAsicEntry(hashAlg, &l2Table);
        return retVal;
    }

    return RT_ERR_L2_INDEXTBL_FULL;    
}

/* Function Name:
 *      rtk_l2_addr_get
 * Description:
 *      Get LUT unicast entry.
 * Input:
 *      pMac  - 6 bytes unicast(I/G bit is 0) mac address to get.
 *      fid - filtering database.
 * Output:
 *      pL2_data     -entry returned
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_FID - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND -No such LUT entry.
 *      RT_ERR_FAILED - 
 * Note:
 *      If the unicast mac address existed in LUT, it will return the port and fid where
 *      the mac is learned. Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
rtk_api_ret_t rtk_l2_addr_get(rtk_mac_t *pMac, rtk_fid_t fid, rtk_l2_ucastAddr_t *pL2_data)
{
    rtl8316d_tblasic_lutParam_t l2Table;
    uint16 row;    
    uint32 hashAlg;
    uint8 column;
        
    /* must be unicast address */
    if((pMac == NULL) || (pMac->octet[0] & 0x1))
        return RT_ERR_MAC;  
    
    /*get maximum when works as IVL*/
    if(fid > RTL8316D_VIDMAX)
        return RT_ERR_L2_FID;  

    if (pL2_data == NULL)
        return RT_ERR_INPUT; 
    
    memset(&l2Table, 0, sizeof(l2Table));
    memcpy(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.lutEther.fid = fid;

    rtl8316d_addrTbl_hashAlg_get(&hashAlg);
    rtl8316d_addrTbl_hash(hashAlg, NULL, &l2Table);   
    row = l2Table.row;
    
    for (column = 0; column < RTL8316D_LUTWAYMAX; column++)
    {
        memset(&l2Table,0,sizeof(l2Table));
        l2Table.row = row;
        l2Table.column = column;
        
        rtl8316d_addrTbl_getAsicEntry(hashAlg, &l2Table); 
        /*entry type: FID+MAC --> single port*/
        if (l2Table.aet == 0 && l2Table.isftidx == 0 
            && ((l2Table.lutInfo.age != 0) || (l2Table.lutInfo.portnum == 29) 
                    || (l2Table.lutInfo.stat == 1) || (l2Table.lutInfo.macauth == 1)
                    || (l2Table.lutInfo.sablk) == 1))
        {
            if ((l2Table.lutEther.fid == fid) 
                 && !(memcmp(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN)))
            {   /*entry exists in L2 table already*/    
                memcpy(pL2_data->mac.octet,pMac->octet,ETHER_ADDR_LEN);  
                pL2_data->fid  = fid;
                pL2_data->port = l2Table.lutInfo.portnum;
                pL2_data->is_static = l2Table.lutInfo.stat;
                pL2_data->auth = l2Table.lutInfo.macauth;
                pL2_data->sa_block = l2Table.lutInfo.sablk; 
                return RT_ERR_OK;
            }
        }
    }

    return RT_ERR_L2_ENTRY_NOTFOUND;
}

/* Function Name:
 *      rtk_l2_addr_del
 * Description:
 *      Delete LUT unicast entry.
 * Input:
 *      mac  - 6 bytes unicast mac address to be deleted.
 *      fid - filtering database for the mac address to be deleted.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_FID - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND -No such LUT entry.
 *      RT_ERR_FAILED - 
 * Note:
 *      If the mac has existed in the LUT, it will be deleted.
 *      Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
rtk_api_ret_t rtk_l2_addr_del(rtk_mac_t *pMac, rtk_fid_t fid)
{
    rtl8316d_tblasic_lutParam_t l2Table;
    rtk_api_ret_t retVal;
    uint16 row;    
    uint32 hashAlg;
    uint8 column;
        
    /* must be unicast address */
    if((pMac == NULL) || (pMac->octet[0] & 0x1))
        return RT_ERR_MAC;  

    /*get maximum when works as IVL*/
    if(fid > RTL8316D_VIDMAX)
        return RT_ERR_L2_FID;  
 
    memset(&l2Table,0,sizeof(l2Table)); 
    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.lutEther.fid = fid;

    rtl8316d_addrTbl_hashAlg_get(&hashAlg);
    rtl8316d_addrTbl_hash(hashAlg, NULL, &l2Table);   
    row = l2Table.row;
    
    for (column = 0; column < RTL8316D_LUTWAYMAX; column++)
    {
        memset(&l2Table,0,sizeof(l2Table));
        l2Table.row = row;
        l2Table.column = column;
        
        rtl8316d_addrTbl_getAsicEntry(hashAlg, &l2Table); 
        /*entry type: FID+MAC --> single port*/
        if (l2Table.aet == 0 && l2Table.isftidx == 0 
            && ((l2Table.lutInfo.age != 0) || (l2Table.lutInfo.portnum == 29) 
                    || (l2Table.lutInfo.stat == 1) || (l2Table.lutInfo.macauth == 1)
                    || (l2Table.lutInfo.sablk) == 1))
        {
            if ((l2Table.lutEther.fid == fid) 
                 && !(memcmp(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN)))
            {   
                /*entry exists in L2 table already, remove it*/
                memcpy(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN);
                l2Table.lutEther.fid = fid;
                l2Table.lutInfo.age = 0;
                l2Table.lutInfo.stat = 0;
                l2Table.lutInfo.macauth = 0;
                l2Table.lutInfo.sablk = 0;
                l2Table.lutInfo.portnum = 0;
            
                retVal = rtl8316d_addrTbl_setAsicEntry(hashAlg, &l2Table);
                return retVal;
            }
        }
    }

    return RT_ERR_L2_ENTRY_NOTFOUND;        
}



/* Function Name:
 *      rtl8316d_addrTbl_hashAlg_set
 * Description:
 *      Set address table hash algorithm
 * Input:
 *      hashAlg  - algorithm, 0 or 1
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED - 
 * Note:
 *      
 */ 
void rtl8316d_addrTbl_hashAlg_set(uint32 hashAlg)
{   
  //  rtl8316d_setSWRegBit(ADDRESS_TABLE_LOOKUP_CONTROL_ADDR, 11, hashAlg);   
  reg_field_write(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_CONTROL, HASHALG, hashAlg);
}


/* Function Name:
 *      rtl8316d_addrTbl_hashAlg_get
 * Description:
 *      Get address table hash algorithm
 * Input:
 *      hashAlg  -  algorithm returned, 0 or 1
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED - 
 * Note:
 *      
 */
int16 rtl8316d_addrTbl_hashAlg_get(uint32* hashAlg)
{       
  //  *hashAlg = rtl8316d_getSWRegBit(ADDRESS_TABLE_LOOKUP_CONTROL_ADDR, 11);   
    if (reg_field_read(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_CONTROL, HASHALG, hashAlg) != SUCCESS)
        return FAILED;
    
    return SUCCESS;
}

int16 _rtl8316d_lut_codeAed_hash0(rtl8316d_tblasic_l2Ucst_t* asicEntry, rtl8316d_tblasic_lutParam_t* lutEntry)
{
   // uint64 tmp, ald, index;
    uint16 xdata ald59_48, xdata ald47_32, xdata ald31_16, xdata ald15_0; 
    uint16 xdata index;
        
    if(lutEntry == NULL)
        return FAILED;

    if (lutEntry->aet == 1)
    {   /*source IP + group IP*/
        if (asicEntry != NULL)
        {
            asicEntry->aed48_33 = lutEntry->lutIp.grpip & 0xFFFF;          /*group IP[15:0]*/
            asicEntry->aed32_17 = lutEntry->lutIp.srcip >> 16;             /*source IP[31:16]*/
            asicEntry->aed16_1 = lutEntry->lutIp.srcip & 0xF800;           /*source IP[15:11]*/
            asicEntry->aed16_1 |= (lutEntry->lutIp.grpip >> 17) & 0x7FF;   /*group IP[27:17]*/
            asicEntry->aed0 = lutEntry->lutIp.grpip >> 16;                 /*group IP[16]*/
            asicEntry->aet = 1;
        }
        ald59_48 = (lutEntry->lutIp.grpip >> 16) & 0xFFF;              /*group IP[27:16]*/
        ald47_32 = lutEntry->lutIp.grpip & 0xFFFF;                     /*group IP[15:0]*/
        ald31_16 = lutEntry->lutIp.srcip >> 16;                        /*srouce IP[31:16]*/
        ald15_0  = lutEntry->lutIp.srcip & 0xFFFF;                     /*source IP[15:0]*/
    }
    else
    {
        if (asicEntry != NULL)
        {
            /*DMAC + FID*/
            asicEntry->aed48_33 = lutEntry->lutEther.mac.octet[0] << 8;           /*MAC[47:40]*/
            asicEntry->aed48_33 |= lutEntry->lutEther.mac.octet[1];               /*MAC[39:32]*/
            asicEntry->aed32_17 = lutEntry->lutEther.mac.octet[2] << 8;           /*MAC[31:24]*/  
            asicEntry->aed32_17 |= lutEntry->lutEther.mac.octet[3];               /*MAC[23:16]*/  
            asicEntry->aed16_1 = (lutEntry->lutEther.mac.octet[4] & 0xF8) << 8;   /*MAC[15:11]*/  
            asicEntry->aed16_1 |= (lutEntry->lutEther.fid >> 1) & (0x7FF);  /*FID[11:1]*/
            asicEntry->aed0     = (lutEntry->lutEther.fid & 0x1);           /*FID[0]*/
            asicEntry->aet = 0;
        }
        ald59_48 = lutEntry->lutEther.fid & 0xFFF;                      /*FID[11:0]*/
        ald47_32 = lutEntry->lutEther.mac.octet[0] << 8;                      /*MAC[47:40]*/
        ald47_32 |= lutEntry->lutEther.mac.octet[1];                          /*MAC[39:32]*/
        ald31_16 = lutEntry->lutEther.mac.octet[2] << 8;                      /*MAC[31:24]*/
        ald31_16 |= lutEntry->lutEther.mac.octet[3];                          /*MAC[23:16]*/
        ald15_0 = lutEntry->lutEther.mac.octet[4] << 8;                       /*MAC[15:8]*/ 
        ald15_0 |= lutEntry->lutEther.mac.octet[5];                           /*MAC[7:0]*/        
    }

    index = ald15_0;
    index ^= (ald15_0 >> 11) | (ald31_16 << 5);
    index ^= (ald31_16 >> 6) | (ald47_32 << 10);
    index ^= (ald47_32 >> 1);
    index ^= (ald47_32 >> 12) | (ald59_48 << 4);
    index ^= (ald59_48 >> 7);
    lutEntry->row = index & 0x7FF;
    
    return lutEntry->row << 2;
}

int8 _rtl8316d_lut_decodeAed_hash0(uint16 index, rtl8316d_tblasic_l2Ucst_t* asicEntry, rtl8316d_tblasic_lutParam_t* lutEntry)
{
    uint16 xdata ald10_0;
    if(asicEntry == NULL || lutEntry == NULL)
        return FAILED;

    index = index >> 2;             /*13-bit --> 11-bit*/
    lutEntry->row = index;

    ald10_0 = index;
    ald10_0 ^=(asicEntry->aed16_1 >> 11) | (asicEntry->aed32_17<<5);
    ald10_0 ^=(asicEntry->aed32_17>>6) | (asicEntry->aed48_33<<10);
    ald10_0 ^=(asicEntry->aed48_33>>1);
    ald10_0 ^=(asicEntry->aed48_33>>12) | (asicEntry->aed16_1<<5) |(asicEntry->aed0<<4);
    ald10_0 ^=((asicEntry->aed16_1 & 0x07c0)>>6);

    ald10_0 = ald10_0 & 0x7ff;
    if (asicEntry->aet == 1)
    {   /*source IP + group IP*/
        lutEntry->lutIp.srcip = (ald10_0 & 0x7ff) | (asicEntry->aed16_1 & 0xf800) | (((uint32)asicEntry->aed32_17)<<16);
        lutEntry->lutIp.grpip = asicEntry->aed48_33 |(((uint32)asicEntry->aed0) << 16) |(((uint32)(asicEntry->aed16_1 & 0x07ff)) << 17);
        lutEntry->lutIp.grpip = (lutEntry->lutIp.grpip & 0xfffffff) | 0xE0000000;
        lutEntry->aet = 1;
    }
    else
    {
        lutEntry->lutEther.mac.octet[5] = (ald10_0 & 0xff);
        lutEntry->lutEther.mac.octet[4] = (ald10_0 >> 8) |((asicEntry->aed16_1 & 0xf800)>>8);
        lutEntry->lutEther.mac.octet[3] = asicEntry->aed32_17 & 0xff;
        lutEntry->lutEther.mac.octet[2] = asicEntry->aed32_17 >> 8;
        lutEntry->lutEther.mac.octet[1] = asicEntry->aed48_33 & 0xff;
        lutEntry->lutEther.mac.octet[0] = asicEntry->aed48_33 >> 8;

        lutEntry->lutEther.fid = ((asicEntry->aed16_1 & 0x7ff) << 1) | asicEntry->aed0;
        lutEntry->aet = 0;
    }

    return SUCCESS;

}

uint16 _rtl8316d_lut_codeAed_hash1(rtl8316d_tblasic_l2Ucst_t* asicEntry, rtl8316d_tblasic_lutParam_t* lutEntry)
{
   // uint64 tmp, ald, index;
    uint16 xdata ald59_48, xdata ald47_32, xdata ald31_16, xdata ald15_0; 
    uint16 xdata index;
        
    if(lutEntry == NULL)
        return FAILED;

    if (lutEntry->aet == 1)
    {   /*source IP + group IP*/
        if (asicEntry != NULL)
        {
            asicEntry->aed48_33 = (lutEntry->lutIp.srcip >> 20);         /*source IP[31:20]*/
            asicEntry->aed48_33 |= ((lutEntry->lutIp.grpip & 0xf) << 12);        /*groupIP[3:0]*/
            asicEntry->aed32_17 = (lutEntry->lutIp.srcip >> 9) & 1;            /*source IP [9]*/
            asicEntry->aed32_17 |= (lutEntry->lutIp.srcip <<1) & 0x3e;   
            asicEntry->aed32_17 |= (lutEntry->lutIp.srcip >>4) & 0xffc0;  
            asicEntry->aed16_1 = (lutEntry->lutIp.grpip >> 5) & 0x7FF;   /*group IP[15:5]*/
            asicEntry->aed16_1 |= ((lutEntry->lutIp.grpip >> 27) & 1) << 11;   /*group IP[27]*/
            asicEntry->aed16_1 |= ((lutEntry->lutIp.srcip >> 5) & 0xf) << 12;    /*source IP[8:5]*/
            asicEntry->aed0 = (lutEntry->lutIp.grpip >> 4) & 1;              /*group IP[4]*/
            asicEntry->aet = 1;
        }
        ald59_48 = (lutEntry->lutIp.grpip >> 4) & 0xFFF;             /*group IP[15:4]*/
        ald47_32 = lutEntry->lutIp.grpip << 12;                      /*group IP[3:0]*/ 
        ald47_32 |= lutEntry->lutIp.srcip >> 20;                     /*source IP[31:20]*/ 
        ald31_16 = (lutEntry->lutIp.srcip >> 9) & 1;                       /*source IP[9]*/
        ald31_16 |= (lutEntry->lutIp.srcip & 0x1f) << 1;
        ald31_16 |= (lutEntry->lutIp.srcip & 0xffc00) >>4;
        ald15_0 = (lutEntry->lutIp.srcip >> 5) << 12;                /*source IP[8:5]*/
        ald15_0 |= (lutEntry->lutIp.grpip >> 16) & 0xFFF;            /*group IP[27:16]*/
    }
    else
    {
        /*DMAC + FID*/
        if (asicEntry != NULL)
        {
            asicEntry->aed48_33 = (lutEntry->lutEther.mac.octet[3] & 0xf0)>> 4;       /*MAC[20:23]*/
            asicEntry->aed48_33 |= lutEntry->lutEther.mac.octet[2] << 4;       /*MAC[31:24]*/
            asicEntry->aed48_33 |= ((uint32)(lutEntry->lutEther.mac.octet[1] & 0xf)) <<12;       /*MAC[32:35]*/

            asicEntry->aed32_17 = (lutEntry->lutEther.mac.octet[5] & 0x1f) << 1;       /*MAC[0:4]*/
            asicEntry->aed32_17 |= (lutEntry->lutEther.mac.octet[4] & 0x02) >>1; /*MAC[9]*/
            asicEntry->aed32_17 |= ((uint32)(lutEntry->lutEther.mac.octet[4] & 0xfc)) <<4;  /*MAC[15:10]*/
            asicEntry->aed32_17 |= ((uint32)(lutEntry->lutEther.mac.octet[3]& 0x0f)) <<12; /*MAC[16:19]*/

            asicEntry->aed16_1 = lutEntry->lutEther.mac.octet[1] >>5;            /*MAC[37:39]*/
            asicEntry->aed16_1 |= ((uint32)lutEntry->lutEther.mac.octet[0]) << 3;  /*MAC[40:47]*/
            asicEntry->aed16_1 |= (((uint32)lutEntry->lutEther.mac.octet[5]) << 7) & 0x7000;  /*MAC[7:5]*/
            asicEntry->aed16_1 |= lutEntry->lutEther.fid & 0x0800;           /*FID[11]*/
            asicEntry->aed16_1 |= ((uint32)(lutEntry->lutEther.mac.octet[4] & 0x01)) << 15;           /*MAC[8]*/

            asicEntry->aed0    = (lutEntry->lutEther.mac.octet[1] >> 4) & 1;            /*MAC[36]*/
            asicEntry->aet = 0;
        }

        ald59_48 = (lutEntry->lutEther.mac.octet[0] << 4) & 0x0FF0;          /*MAC[47:40]*/
        ald59_48 |= (lutEntry->lutEther.mac.octet[1] >> 4);                  /*MAC[39:36]*/

        ald47_32 = (lutEntry->lutEther.mac.octet[1] & 0x0F) << 12;           /*MAC[35:32]*/
        ald47_32 |= lutEntry->lutEther.mac.octet[2] << 4;                    /*MAC[31:24]*/
        ald47_32 |= lutEntry->lutEther.mac.octet[3] >> 4;                    /*MAC[23:20]*/
        
        ald31_16 = ((uint32)lutEntry->lutEther.mac.octet[3]) << 12;                    /*MAC[19:16]*/
        ald31_16 |= ((uint32)(lutEntry->lutEther.mac.octet[4] >> 2)) << 6;             /*MAC[15:10]*/
        ald31_16 |= (lutEntry->lutEther.mac.octet[5] & 0x1F) << 1;           /*MAC[4:0]*/
        ald31_16 |= (lutEntry->lutEther.mac.octet[4] >> 1) & 0x1;            /*MAC[9]*/

        ald15_0 = (lutEntry->lutEther.mac.octet[4] << 15) & 0x8000;          /*MAC[8]*/       
        ald15_0 |= (lutEntry->lutEther.mac.octet[5] & 0xE0 ) << 7;           /*MAC[7:5]*/  
        ald15_0 |= lutEntry->lutEther.fid & 0xFFF;                     /*FID[11:0]*/              
    }


    index = ald15_0;
    index ^= (ald15_0 >> 11) | (ald31_16 << 5);
    index ^= (ald31_16 >> 6) | (ald47_32 << 10);
    index ^= (ald47_32 >> 1);
    index ^= (ald47_32 >> 12) | (ald59_48 << 4);
    index ^= (ald59_48 >> 7);
    lutEntry->row = index & 0x7FF;
    
    return lutEntry->row << 2;
}

int8 _rtl8316d_lut_decodeAed_hash1(uint16 index, rtl8316d_tblasic_l2Ucst_t* asicEntry, rtl8316d_tblasic_lutParam_t* lutEntry)
{
   uint16 xdata ald10_0;
        
    if(asicEntry == NULL || lutEntry == NULL)
        return FAILED;

    index = index >> 2;             /*13-bit --> 11-bit*/

    lutEntry->row = index;

    ald10_0 = index;
    ald10_0 ^=(asicEntry->aed16_1 >> 11) | (asicEntry->aed32_17<<5);
    ald10_0 ^=(asicEntry->aed32_17>>6) | (asicEntry->aed48_33<<10);
    ald10_0 ^=(asicEntry->aed48_33>>1);
    ald10_0 ^=(asicEntry->aed48_33>>12) | (asicEntry->aed16_1<<5) |(asicEntry->aed0<<4);
    ald10_0 ^=((asicEntry->aed16_1 & 0x07c0)>>6);

    ald10_0 = ald10_0 & 0x7ff;


    if (asicEntry->aet == 1)
    {   /*source IP + group IP*/
        lutEntry->lutIp.srcip = ((asicEntry->aed32_17>>1) & 0x1f) |( (asicEntry->aed16_1 & 0xf000) >> 7) | ( (asicEntry->aed32_17 & 0x1) <<9);
        lutEntry->lutIp.srcip = lutEntry->lutIp.srcip | ((((uint32)asicEntry->aed32_17) & 0xffc0)<<4) |((((uint32)asicEntry->aed48_33) & 0xfff)<<20);
        lutEntry->lutIp.grpip = (asicEntry->aed48_33 >>12) |(((uint32)ald10_0)<< 16) |((asicEntry->aed16_1 & 0x07ff) << 5) |
            ((asicEntry->aed16_1 & 0x0800) << 16) | (asicEntry->aed0 << 4);
        lutEntry->lutIp.grpip = (lutEntry->lutIp.grpip & 0xfffffff) | 0xE0000000;
        lutEntry->aet = 1;
    }
    else
    {
        lutEntry->lutEther.mac.octet[5] = ((asicEntry->aed32_17>>1) & 0x1f) | ((asicEntry->aed16_1 >> 7) & 0x00e0);
        lutEntry->lutEther.mac.octet[4] = ((asicEntry->aed16_1>>15) & 1) | ((asicEntry->aed32_17 & 1)<<1) | ((asicEntry->aed32_17&0x0fc0) >> 4);
        lutEntry->lutEther.mac.octet[3] = ((asicEntry->aed32_17&0xf000) >> 12)|((asicEntry->aed48_33 & 0x000f)<<4);
        lutEntry->lutEther.mac.octet[2] = (asicEntry->aed48_33 >>4) & 0xff;
        lutEntry->lutEther.mac.octet[1] = (asicEntry->aed48_33 >>12) |(asicEntry->aed0 << 4)|((asicEntry->aed16_1 & 7) << 5) ;
        lutEntry->lutEther.mac.octet[0] = (asicEntry->aed16_1 >> 3) & 0xff;

        lutEntry->lutEther.fid = ald10_0 |(asicEntry->aed16_1 & 0x0800);
        lutEntry->aet = 0;
    }

    return SUCCESS;

}


/* Function Name:
 *      rtl8316d_addrTbl_hash
 * Description:
 *      Get address table index
 * Input:
 *      hashAlg  - hash algorithm
 *      lutEntry     - address table entry with information provided
 * Output:
 *      l2uentry -  ASIC address table
 * Return:
 *      index - 13-bit entry index
 * Note:
 *      None
 */
uint16 rtl8316d_addrTbl_hash(uint8 hashAlg, rtl8316d_tblasic_l2Ucst_t* l2uentry, rtl8316d_tblasic_lutParam_t* lutEntry)
{
    uint16 xdata index;  
    
    if (lutEntry == NULL)
    {
        return FAILED;
    }
   
    if (0 == hashAlg)
    {
        index = _rtl8316d_lut_codeAed_hash0(l2uentry, lutEntry);
    }
    else
    {
        index = _rtl8316d_lut_codeAed_hash1(l2uentry, lutEntry);
    }

//    rtlglue_printf("AddrTbl index is %d\n", index);
    return index;
}


/* Function Name:
 *      rtl8316d_addrTbl_setAsicEntry
 * Description:
 *      Set address table entry
 * Input:
 *      hashAlg  - hash algorithm
 *      lutEntry     - address table entry with information provided
 * Output:
 *      None
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     The entry index is provided by lutEntry->row and lutEntry->column
 */
int8 rtl8316d_addrTbl_setAsicEntry(uint8 hashAlg, rtl8316d_tblasic_lutParam_t* lutEntry)
{
    rtl8316d_tblasic_l2Ucst_t xdata l2uentry;
    rtl8316d_tblasic_l2Mcst_t xdata l2mentry;
    uint16 xdata index;
    int8 xdata retvalue;

    if (lutEntry == NULL)
        return -1;

    memset(&l2uentry, 0, sizeof (l2uentry));
    memset(&l2mentry, 0, sizeof (l2mentry));

    if (0 == hashAlg)
    {
        index = _rtl8316d_lut_codeAed_hash0(&l2uentry, lutEntry);
    }
    else
    {
        index = _rtl8316d_lut_codeAed_hash1(&l2uentry, lutEntry);
    }

    index = (lutEntry->row << 2) | lutEntry->column;   
    DEBUG_INFO(RTL8316D_DEBUG_ADDRLOOKUP,"index is 0x%X", index);
    if (0 == lutEntry->isftidx)
    {
        l2uentry.aet = lutEntry->aet;
        l2uentry.isftidx= lutEntry->isftidx;
        l2uentry.portnum= lutEntry->lutInfo.portnum;
        l2uentry.sablk = lutEntry->lutInfo.sablk;
        l2uentry.macauth = lutEntry->lutInfo.macauth;
        l2uentry.stat = lutEntry->lutInfo.stat;
        l2uentry.age = lutEntry->lutInfo.age;

        retvalue = rtl8316d_tableWrite(L2_TABLE, index, (uint32 *)&l2uentry);
      
        DEBUG_INFO(RTL8316D_DEBUG_ADDRLOOKUP, "HashAlg %d, Add address table entry[0x%-4X] %s!:\n", (uint16)hashAlg, (uint16)index, (retvalue == SUCCESS) ? "SUCCESS":"FAILED");
        DEBUG_INFO(RTL8316D_DEBUG_ADDRLOOKUP, "Entry mac is %02bx:%02bx:%02bx:%02bx:%02bx:%02bx, fid is 0x%x, aet is %d\n", 
                    lutEntry->lutEther.mac.octet[0],lutEntry->lutEther.mac.octet[1],
                    lutEntry->lutEther.mac.octet[2],lutEntry->lutEther.mac.octet[3],
                    lutEntry->lutEther.mac.octet[4],lutEntry->lutEther.mac.octet[5], 
                    (uint16)lutEntry->lutEther.fid, (uint16)lutEntry->aet);

    }
    else
    {
        l2mentry.aed0= l2uentry.aed0;
        l2mentry.aed16_1= l2uentry.aed16_1;
        l2mentry.aed32_17= l2uentry.aed32_17;
        l2mentry.aed48_33= l2uentry.aed48_33;
        l2mentry.aet= lutEntry->aet;
        l2mentry.ftidx = lutEntry->info.ftidx.ftindex;
        l2mentry.isftidx = lutEntry->isftidx;

        retvalue = rtl8316d_tableWrite(L2_TABLE, index, (uint32 *)&l2mentry);
        DEBUG_INFO(RTL8316D_DEBUG_ADDRLOOKUP, "HashAlg %d, Add address table entry[0x%-4X] %s!:\n", (uint16)hashAlg, (uint16)index, (retvalue == SUCCESS) ? "SUCCESS":"FAILED");
        DEBUG_INFO(RTL8316D_DEBUG_ADDRLOOKUP, "Entry mac is %02bx:%02bx:%02bx:%02bx:%02bx:%02bx, fid is 0x%x, aet is %d\n", 
                    lutEntry->lutEther.mac.octet[0],lutEntry->lutEther.mac.octet[1],
                    lutEntry->lutEther.mac.octet[2],lutEntry->lutEther.mac.octet[3],
                    lutEntry->lutEther.mac.octet[4],lutEntry->lutEther.mac.octet[5], 
                    (uint16)lutEntry->lutEther.fid, (uint16)lutEntry->aet);
    }

    if (0 != retvalue)
    {
        //rtlglue_printf("Can't set address table entry %d \n", index);
        return -1;
    }  

    return SUCCESS;
}


/*
@func int32 | rtl8316d_addrTbl_getAsicEntry | get address table entry
@parm  uint32 | hashAlg   | hash algorithm
@parm  rtl8316d_tblasic_lutParam_t* | lutEntry   | 
@rvalue int32 |SUCCESS  |
@comm the entry index is provided by lutEntry->row and lutEntry->column
*/
int8 rtl8316d_addrTbl_getAsicEntry(uint8 hashAlg, rtl8316d_tblasic_lutParam_t* lutEntry)
{
    rtl8316d_tblasic_l2Ucst_t xdata l2uentry;
    rtl8316d_tblasic_l2Mcst_t xdata l2mentry;
    uint16 xdata index;
    int8 xdata retvalue;

    if (lutEntry == NULL)
        return -1;

    memset(&l2uentry, 0, sizeof (l2uentry));
    memset(&l2mentry, 0, sizeof (l2mentry));

    index = (lutEntry->row << 2) | lutEntry->column;
   // rtlglue_printf("The index is: %d\n", index);
    retvalue = rtl8316d_tableRead(L2_TABLE, index, (uint32 *)&l2uentry);
    if (0 != retvalue)
    {
       // rtlglue_printf("Can't get address table entry %d \n", index);
        return -1;
    }
    if (0 == hashAlg)
    {
        _rtl8316d_lut_decodeAed_hash0(index, &l2uentry, lutEntry);
    }
    else
    {
        _rtl8316d_lut_decodeAed_hash1(index, &l2uentry, lutEntry);
    }

    if (0 == l2uentry.isftidx)
    {
       // lutEntry->aet = l2uentry.aet;
        lutEntry->isftidx = l2uentry.isftidx;
        lutEntry->lutInfo.portnum = l2uentry.portnum;
        lutEntry->lutInfo.macauth = l2uentry.macauth;
        lutEntry->lutInfo.sablk = l2uentry.sablk;
        lutEntry->lutInfo.stat = l2uentry.stat;
        lutEntry->lutInfo.age = l2uentry.age;
    }
    else
    {
        l2mentry = *(rtl8316d_tblasic_l2Mcst_t*)&l2uentry;
       // lutEntry->aet = l2mentry.aet;
        lutEntry->isftidx = l2mentry.isftidx;
        lutEntry->info.ftidx.ftindex = l2mentry.ftidx;
    }

    return SUCCESS;
}

/*
@func int32 | rtl8316d_addrTbl_entry_reset  | reset address table entry
@parm  uint32 | start   | from 'start' index
@parm  uint32 | end   |  to 'end' index
@rvalue int32 |SUCCESS  |
@comm entries ranged from 'start' to 'end' will be cleared
*/
int8 rtl8316d_addrTbl_entry_reset(uint16 start, uint16 end)
{
    /*use ucst entry here because it is longer, so all bits will be cleared*/
    rtl8316d_tblasic_l2Ucst_t xdata l2uentry;
    uint16 xdata index;
    int8 xdata retvalue;
    
    memset(&l2uentry, 0, sizeof (l2uentry));
    retvalue = SUCCESS;
    
    for (index = start; index <= end; index++)
    {
        DEBUG_INFO(RTL8316D_DEBUG_ADDRLOOKUP, "index is 0x%X", index);
        retvalue = rtl8316d_tableWrite(L2_TABLE, index, (uint32 *)&l2uentry);
        if (0 != retvalue)
        {
           // rtlglue_printf("Can't set address table entry %d \n", index);
            return -1;
        }        
    }
    
    return SUCCESS;
}


/*force add asic entry into Address Table Entry[index]*/
int8 rtl8316d_addrTbl_entry_fourceAdd(uint8 hashAlg, uint16 index, rtl8316d_tblasic_lutParam_t* lutEntry)
{
    rtl8316d_tblasic_l2Ucst_t xdata l2uentry;
    rtl8316d_tblasic_l2Mcst_t xdata l2mentry;
    int8 retvalue;
    if (lutEntry == NULL)
        return -1;

    memset(&l2uentry, 0, sizeof (l2uentry));
    memset(&l2mentry, 0, sizeof (l2mentry));

    if (0 == hashAlg)
    {
        _rtl8316d_lut_codeAed_hash0(&l2uentry, lutEntry);
    }
    else
    {
        _rtl8316d_lut_codeAed_hash1(&l2uentry, lutEntry);
    }

    if (0 == lutEntry->isftidx)
    {
        l2uentry.aet = lutEntry->aet;
        l2uentry.isftidx= lutEntry->isftidx;
        l2uentry.portnum= lutEntry->lutInfo.portnum;
        l2uentry.sablk = lutEntry->lutInfo.sablk;
        l2uentry.macauth = lutEntry->lutInfo.macauth;
        l2uentry.stat = lutEntry->lutInfo.stat;
        l2uentry.age = lutEntry->lutInfo.age;
        l2uentry.reserved = 0;
        retvalue = rtl8316d_tableWrite(L2_TABLE, index, (uint32 *)&l2uentry);
    }
    else
    {
        l2mentry.aed0= l2uentry.aed0;
        l2mentry.aed16_1= l2uentry.aed16_1;
        l2mentry.aed32_17= l2uentry.aed32_17;
        l2mentry.aed48_33= l2uentry.aed48_33;
        l2mentry.aet= lutEntry->aet;
        l2mentry.ftidx = lutEntry->info.ftidx.ftindex;
        l2mentry.isftidx = lutEntry->isftidx;
        l2mentry.reserved = 0;
        //rtlglue_printf("aet: %bu, ftidx: %bu, istfidx:%bu\n", lutEntry->aet, lutEntry->info.ftidx.ftindex,lutEntry->isftidx);
        retvalue = rtl8316d_tableWrite(L2_TABLE, index, (uint32 *)&l2mentry);
    }

    if (0 != retvalue)
    {
       // rtlglue_printf("Can't set address table entry %d \n", index);
        return -1;
    }
    
   // rtlglue_printf("Add addrTbl entry %d\n", index);
    
    return SUCCESS;
}

#if 0
int8 rtl8316d_fwd_getAsicEntry(uint16 index, rtl8316d_tblasic_fwdTbl_t * fwd)
{
    int8 xdata retvalue;
    uint32 xdata fwd_data;

    if (index >= RTL8316D_FWD_NUMBER)
    {
        return -1;
    }

    retvalue = rtl8316d_tableRead( FORWARDING, index, &fwd_data);
    if (retvalue != 0)
    {
        rtlglue_printf("read forwarding table entry %d failed\n", index);
        return -1;
    }
    fwd->portmask = (fwd_data>>3)&0x1FFFFFFUL;
    fwd->age = (uint8)fwd_data&0x3;
    fwd->crsvlan = ((uint8)fwd_data>>2)&0x1;

    return SUCCESS;
}
#endif

inline void _rtk_copy(uint8 *dest, const uint8 *src, uint32 n)
{
    uint32 i;
    
    for(i = 0; i < n; i++)
        dest[i] = src[i];
}

/* Function Name:
 *      rtk_l2_mcastAddr_add
 * Description:
 *      Add LUT multicast entry.
 * Input:
 *      pMac  - 6 bytes multicast(I/G bit isn't 0) mac address to be written into LUT.
 *      portmask - Port mask to be forwarded to.
 *      fid     - filtering database for the input LUT entry.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_FID - Invalid FID .
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_PORT_MASK - Invalid portmask.
 * Note:
 *      If the multicast mac address already existed in the LUT, it will udpate the
 *      port mask of the entry. Otherwise, it will find an empty or asic auto learned
 *      entry to write. If all the entries with the same hash value can't be replaced, 
 *      ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
int32 rtk_l2_mcastAddr_add(rtk_mac_t *pMac, rtk_fid_t fid, rtk_portmask_t portmask)
{
    rtl8316d_tblasic_lutParam_t l2Table;
    rtl8316d_tblasic_fwdTblParam_t fwd;
    rtk_api_ret_t retVal;
    uint32 hashAlg;
    uint16 row;
    uint8 column;
    uint8 available;
    uint8 fwdIdx;
    
    if ((pMac == NULL) || !(pMac->octet[0] & 0x1))
    {
        return RT_ERR_MAC;
    }

    /*12-bit*/
    if (fid > RTL8316D_VIDMAX)
    {
        return RT_ERR_L2_FID;
    }

    if(portmask.bits[0]> RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_MASK; 

    memset(&l2Table,0,sizeof(l2Table));
    available = 0xFF; /*invalid*/
    
    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.lutEther.fid = fid;

    rtl8316d_addrTbl_hashAlg_get(&hashAlg);
    rtl8316d_addrTbl_hash(hashAlg, NULL, &l2Table); 
    row = l2Table.row;
    
    for (column = 0; column < RTL8316D_LUTWAYMAX; column++)
    {   
        memset(&l2Table,0,sizeof(l2Table));
        l2Table.row = row;
        l2Table.column = column;
        
        rtl8316d_addrTbl_getAsicEntry(hashAlg, &l2Table); 
        if ((l2Table.aet == 0) && (l2Table.isftidx == 0) 
            && (l2Table.lutInfo.age == 0) && (l2Table.lutInfo.stat == 0)
            && (l2Table.lutInfo.macauth == 0) && (l2Table.lutInfo.sablk == 0)
            && (l2Table.lutInfo.portnum != 29))
        {   /*empty entry found, mark it*/
            if (available == 0xFF)
            {
                available = column;
            }
            
            continue;
        }
        
        /*valid entry, type: FID+MAC*/
        if (l2Table.aet == 0)
        {   
            /*entry exists already*/
            if ((l2Table.lutEther.fid == fid) 
                 && !(memcmp(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN)))
            {
                if (l2Table.isftidx)
                {
                    /*update*/
                    if (rtl8316d_fwd_getAsicEntry(l2Table.info.ftidx.ftindex, &fwd) != RT_ERR_OK)
                    {
                        return RT_ERR_FAILED;
                    }
                    
                    fwd.portmask = portmask.bits[0];
                    rtl8316d_fwd_setAsicEntry(l2Table.info.ftidx.ftindex, &fwd);
                    return RT_ERR_OK;
                } else {
                    /*L2-->multicast*/
                    if (rtl8316d_fwd_fstAvalIdx(&fwdIdx) != SUCCESS)
                    {
                        return RT_ERR_FAILED;
                    }

                    memset(&fwd, 0, sizeof(fwd));
                    fwd.age = 3;
                    fwd.portmask = portmask.bits[0];                    
                    rtl8316d_fwd_setAsicEntry(fwdIdx, &fwd);
                    
                    l2Table.isftidx = 1;
                    l2Table.info.ftidx.ftindex = fwdIdx;
                    rtl8316d_addrTbl_setAsicEntry(hashAlg, &l2Table);                    
                    return RT_ERR_OK;
                }
            
            }
        }
    }

    if (available != 0xFF)
    {
        if (rtl8316d_fwd_fstAvalIdx(&fwdIdx) != SUCCESS)
        {
            return RT_ERR_FAILED;
        }
        memset(&l2Table,0,sizeof(l2Table));
        memcpy(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN);
        l2Table.lutEther.fid = fid;
        l2Table.aet = 0;
        l2Table.isftidx = 1;
        l2Table.info.ftidx.ftindex = fwdIdx;
        l2Table.column = available;        
        retVal = rtl8316d_addrTbl_setAsicEntry(hashAlg, &l2Table);

        memset(&fwd, 0, sizeof(fwd));
        fwd.age = 3;
        fwd.portmask = portmask.bits[0];                    
        rtl8316d_fwd_setAsicEntry(fwdIdx, &fwd);

        return RT_ERR_OK;
    }

    
    return RT_ERR_L2_INDEXTBL_FULL;

}

/* Function Name:
 *      rtk_l2_mcastAddr_get
 * Description:
 *      Get LUT multicast entry.
 * Input:
 *      pMac  - 6 bytes multicast(I/G bit is 1) mac address to get.
 *      pPort - pointer to returned port number.
 *      fid     - filtering database for the input LUT entry.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_ENTRY_NOTFOUND - No such LUT entry.
 * Note:
 *      If the unicast mac address existed in the LUT, it will return the port mask. Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
rtk_api_ret_t rtk_l2_mcastAddr_get(rtk_mac_t *pMac, rtk_fid_t fid, rtk_portmask_t *pPortmask)
{
    rtl8316d_tblasic_lutParam_t l2Table;
    rtl8316d_tblasic_fwdTblParam_t fwd;
    uint32 hashAlg;
    uint16 row;
    uint8 column;

    if (pMac == NULL || !(pMac->octet[0] & 0x1) || pPortmask == NULL)
    {
        return RT_ERR_INPUT;
    }

    if(fid > RTL8316D_VIDMAX)
        return RT_ERR_L2_FID;      
    
    memset(&l2Table, 0, sizeof(l2Table));
    memcpy(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.lutEther.fid = fid;

    rtl8316d_addrTbl_hashAlg_get(&hashAlg);
    rtl8316d_addrTbl_hash(hashAlg, NULL, &l2Table);   
    row = l2Table.row;
    
    for (column = 0; column < RTL8316D_LUTWAYMAX; column++)
    {
        memset(&l2Table,0,sizeof(l2Table));
        l2Table.row = row;
        l2Table.column = column;
        
        rtl8316d_addrTbl_getAsicEntry(hashAlg, &l2Table); 
        /*entry type: FID+MAC --> single port*/
        if (l2Table.aet == 0 && l2Table.isftidx == 1)
        {
            if ((l2Table.lutEther.fid == fid) 
                 && !(memcmp(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN)))
            {   /*entry exists in L2 table already*/    

                rtl8316d_fwd_getAsicEntry(l2Table.info.ftidx.ftindex, &fwd);
                pPortmask->bits[0] = fwd.portmask;
                return RT_ERR_OK;
            }
        }
    }

    return RT_ERR_L2_ENTRY_NOTFOUND;

}

/* Function Name:
 *      rtk_l2_mcastAddr_del
 * Description:
 *      Delete LUT unicast entry.
 * Input:
 *      mac  -  6 bytes multicast mac address to be deleted.
 *      fid     - filtering database for the mac address to be deleted.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_MAC - Invalid MAC address.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_ENTRY_NOTFOUND - No such LUT entry.
 * Note:
 *      If the mac has existed in the LUT, it will be deleted.
 *      Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
rtk_api_ret_t rtk_l2_mcastAddr_del(rtk_mac_t *pMac, rtk_fid_t fid)
{
    rtl8316d_tblasic_lutParam_t l2Table;
//    rtl8316d_tblasic_fwdTblParam_t fwd;
    uint32 hashAlg;
    uint16 row;
    uint8 column;

    if (pMac == NULL || !(pMac->octet[0] & 0x1))
    {
        return RT_ERR_INPUT;
    }

    if(fid > RTL8316D_VIDMAX)
        return RT_ERR_L2_FID;      
    
    memset(&l2Table, 0, sizeof(l2Table));
    memcpy(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.lutEther.fid = fid;

    rtl8316d_addrTbl_hashAlg_get(&hashAlg);
    rtl8316d_addrTbl_hash(hashAlg, NULL, &l2Table);   
    row = l2Table.row;

    for (column = 0; column < RTL8316D_LUTWAYMAX; column++)
    {
        memset(&l2Table,0,sizeof(l2Table));
        l2Table.row = row;
        l2Table.column = column;
        
        rtl8316d_addrTbl_getAsicEntry(hashAlg, &l2Table); 
        /*entry type: FID+MAC --> single port*/
        if (l2Table.aet == 0 && l2Table.isftidx == 1)
        {
            if ((l2Table.lutEther.fid == fid) 
                 && !(memcmp(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN)))
            {   
                /*entry exists in L2 table already, remove it*/
                rtl8316d_fwd_delAsicEntry((uint16)l2Table.info.ftidx.ftindex);
                memset(&l2Table, 0, sizeof(l2Table));
                memcpy(l2Table.lutEther.mac.octet, pMac->octet, ETHER_ADDR_LEN);
                l2Table.lutEther.fid = fid;
                rtl8316d_addrTbl_setAsicEntry(hashAlg, &l2Table);

                return RT_ERR_OK;
            }
        }
    }

    return RT_ERR_L2_ENTRY_NOTFOUND;
}


/* Function Name:
 *      rtk_l2_ipMcastAddr_add
 * Description:
 *      Set Lut IP multicast lookup function.
 * Input:
 *      dip  -  Destination IP Address. 
 *      sip     -  Source IP Address.
 *      portmask     - Destination port mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_PORT_MASK - Invalid portmask.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_L2_FWDTBL_FULL - forwarding table is full.
 * Note:
 *      System supports L2 entry with IP multicast DIP/SIP to forward IP multicasting frame as user 
 *      desired. If this function is enabled, then system will be looked up L2 IP multicast entry to 
 *      forward IP multicast frame directly without flooding.
 */
rtk_api_ret_t rtk_l2_ipMcastAddr_add(ipaddr_t sip, ipaddr_t dip, rtk_portmask_t portmask)
{
    rtl8316d_tblasic_lutParam_t l2Table;
    rtl8316d_tblasic_fwdTblParam_t fwd;
    uint32 hashAlg;
    uint16 row;
    uint8 column;
    uint8 available;
    uint8 fwdIdx;

    if(portmask.bits[0] > RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_ID; 

    available = 0xFF; /*invalid*/
    memset(&l2Table,0,sizeof(l2Table));    
    /* fill key (SIP, DIP) to get L2 entry */
    l2Table.lutIp.grpip = dip;
    l2Table.lutIp.srcip = sip;
    l2Table.aet = 1;
    rtl8316d_addrTbl_hashAlg_get(&hashAlg);
    rtl8316d_addrTbl_hash(hashAlg, NULL, &l2Table); 
    row = l2Table.row;    
    for (column = 0; column < RTL8316D_LUTWAYMAX; column++)
    {   
        memset(&l2Table,0,sizeof(l2Table));
        l2Table.row = row;
        l2Table.column = column;
        
        rtl8316d_addrTbl_getAsicEntry(hashAlg, &l2Table); 
        if ((l2Table.aet == 0) && (l2Table.isftidx == 0) 
            && (l2Table.lutInfo.age == 0) && (l2Table.lutInfo.stat == 0)
            && (l2Table.lutInfo.macauth == 0) && (l2Table.lutInfo.sablk == 0)
            && (l2Table.lutInfo.portnum != 29))
        {   /*empty entry found, mark it*/
            if (available == 0xFF)
            {
                available = column;
            }
            continue;
        }
        
        /*valid entry, type: SIP + DIP --> port mask*/
        if (l2Table.aet == 1 && l2Table.isftidx == 1)
        {
            if (l2Table.lutIp.srcip == sip && l2Table.lutIp.grpip == dip)
            {   /*entry exists in L2 table already*/

                /*update*/
                if (rtl8316d_fwd_getAsicEntry(l2Table.info.ftidx.ftindex, &fwd) != RT_ERR_OK)
                {
                    return RT_ERR_FAILED;
                }                
                fwd.portmask = portmask.bits[0];
                fwd.age = 0x3;
                rtl8316d_fwd_setAsicEntry(l2Table.info.ftidx.ftindex, &fwd);

                return RT_ERR_OK;
            }
        }
    }

    if (available != 0xFF)
    {
        if (rtl8316d_fwd_fstAvalIdx(&fwdIdx) != RT_ERR_OK)
        {
            return RT_ERR_L2_FWDTBL_FULL;
        }
        memset(&fwd, 0, sizeof(fwd));
        fwd.portmask = portmask.bits[0];
        fwd.age = 0x3;
        rtl8316d_fwd_setAsicEntry(fwdIdx, &fwd);
        
        memset(&l2Table,0,sizeof(l2Table));
        l2Table.lutIp.grpip = dip;
        l2Table.lutIp.srcip = sip;
        l2Table.aet = 1;
        l2Table.isftidx = 1;
        l2Table.info.ftidx.ftindex = fwdIdx;
        l2Table.row    = row;
        l2Table.column = available;        
        rtl8316d_addrTbl_setAsicEntry(hashAlg, &l2Table);
        return RT_ERR_OK;
    }

    return RT_ERR_L2_INDEXTBL_FULL;  
}


/* Function Name:
 *      rtk_l2_ipMcastAddr_get
 * Description:
 *      Get Lut IP multicast lookup function setting.
 * Input:
 *      dip  -  Destination IP Address. 
 *      sip     -  Source IP Address.
 * Output:
 *      pPortmask     - Destination port mask.
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_PORT_MASK - Invalid portmask.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 * Note:
 *     The API can get Lut table of IP multicast entry.
 */
rtk_api_ret_t rtk_l2_ipMcastAddr_get(ipaddr_t sip, ipaddr_t dip, rtk_portmask_t *pPortmask)
{
    rtl8316d_tblasic_lutParam_t l2Table;
    rtl8316d_tblasic_fwdTblParam_t fwd;
    uint16 row;
    uint32 hashAlg;
    uint8 column; 
        
    if (pPortmask == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    memset(&l2Table, 0, sizeof(l2Table));
    l2Table.aet = 1;
    l2Table.lutIp.srcip = sip;
    l2Table.lutIp.grpip = dip;
    rtl8316d_addrTbl_hashAlg_get(&hashAlg);
    rtl8316d_addrTbl_hash(hashAlg, NULL, &l2Table);
    row = l2Table.row;

    for (column = 0; column < RTL8316D_LUTWAYMAX; column++)
    {        
        memset(&l2Table,0,sizeof(l2Table));
        l2Table.column = column;
        l2Table.row = row;
        rtl8316d_addrTbl_getAsicEntry(hashAlg, &l2Table);
        if ((l2Table.aet == 1) && (l2Table.isftidx == 1))
        {
            if (l2Table.lutIp.srcip == sip && l2Table.lutIp.grpip == dip)
            {/*get it*/
                rtl8316d_fwd_getAsicEntry(l2Table.info.ftidx.ftindex, &fwd);
                pPortmask->bits[0] = fwd.portmask;
                return RT_ERR_OK;
            }
        }        
    }
    
    return RT_ERR_L2_ENTRY_NOTFOUND;
}    

/* Function Name:
 *      rtk_l2_ipMcastAddr_del
 * Description:
 *      Delete a L2 ip multicast address entry from the specified device.
 * Input:
 *      dip  -  Destination IP Address. 
 *      sip     -  Source IP Address.
 * Output:
 *      pPortmask     - Destination port mask.
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_L2_ENTRY_NOTFOUND - No such LUT entry.
 * Note:
 *      The API can delete a L2 ip multicast address entry from the specified device.
 */   
rtk_api_ret_t rtk_l2_ipMcastAddr_del(ipaddr_t sip, ipaddr_t dip)
{
    rtl8316d_tblasic_lutParam_t l2Table;
    uint16 column;
    uint16 row;
    uint32 hashAlg;
    
    memset(&l2Table, 0, sizeof(l2Table));
    l2Table.aet = 1;
    l2Table.lutIp.srcip = sip;
    l2Table.lutIp.grpip = dip;

    rtl8316d_addrTbl_hashAlg_get(&hashAlg);
    rtl8316d_addrTbl_hash(hashAlg, NULL, &l2Table);
    row = l2Table.row;

    for (column = 0; column < RTL8316D_LUTWAYMAX; column++)
    {        
        memset(&l2Table,0,sizeof(l2Table));
        l2Table.column = column;
        l2Table.row = row;
        rtl8316d_addrTbl_getAsicEntry(hashAlg, &l2Table);
        if ((l2Table.aet == 1) && (l2Table.isftidx == 1))
        {
            if (l2Table.lutIp.srcip == sip && l2Table.lutIp.grpip == dip)
            {   /*get it*/
                rtl8316d_fwd_delAsicEntry(l2Table.info.ftidx.ftindex);
                memset(&l2Table, 0, sizeof(l2Table));
                rtl8316d_addrTbl_entry_fourceAdd(hashAlg, row << 2 | column, &l2Table);

                return RT_ERR_OK;
            }
        }        
    }
    
    return RT_ERR_L2_ENTRY_NOTFOUND;
}    


/* Function Name:
 *      rtk_l2_flushType_set
 * Description:
 *      Flush L2 mac address by type in the specified device.
 * Input:
 *      type  -  flush type
 *      vid     -  VLAN id
 *      key     -  key for flushed item
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_PORT_MASK - Invalid portmask.
 *      RT_ERR_VLAN_VID - Invalid VID parameter.
 * Note:
 *      This function trigger flushing of per-port L2 learning.
 *      When flushing operaton completes, the corresponding bit will be clear.
 *      The flush type as following:
 *      FLUSH_TYPE_BY_PORT        (physical port)
 *      FLUSH_TYPE_BY_FID
 *      FLUSH_TYPE_BY_MAC
 *      As for RTL8316D, flush types could be combined by '|' operator
 */ 
rtk_api_ret_t rtk_l2_flushType_set(rtk_l2_flushType_t type, rtk_vlan_t vid, uint32 portOrTid)
{
    rtl8316d_l2_flushItem_t item;

    memset(&item, 0, sizeof(rtl8316d_l2_flushItem_t));

    item.port = portOrTid;
    item.fid  = vid;
    
    switch(type)
    {
        case FLUSH_TYPE_BY_PORT:
            rtl8316d_l2_flushType_set(RTL8316D_FLUSH_BY_PORT, vid, item);
            break;
        default:
            return RT_ERR_L2_FLUSH_TYPE;
    }

    return RT_ERR_OK;
}


rtk_api_ret_t rtl8316d_setAsicLutIpMulticastLookup(uint32 enabled)
{
    rtk_api_ret_t retVal;
    
    if(enabled > 1)
        return RT_ERR_ENABLE; 

    retVal = reg_field_write(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_CONTROL, IPMC, enabled);
    
    return retVal;
}

rtk_api_ret_t rtl8316d_setAsicLutAgeTimerSpeed(uint32 ageUnit)
{
    rtk_api_ret_t retVal;
    if(ageUnit > RTL8316D_LUT_AGEUNITMAX)
        return RT_ERR_OUT_OF_RANGE;

    retVal = reg_field_write(RTL8316D_UNIT, SOURCE_MAC_LEARNING_CONTROL, AGEUNIT, ageUnit);

    return retVal;  
}

rtk_api_ret_t rtl8316d_l2_flushType_set(rtl8316d_l2_flushType_t type, rtk_vlan_t vid, rtl8316d_l2_flushItem_t key)
{
    rtl8316d_lut_opeartions_t lutOp;
//    rtk_api_ret_t retVal;    

    if(type > RTL8316D_FLUSH_END || type == 0)
        return RT_ERR_INPUT;      
    
    if (type & RTL8316D_FLUSH_BY_PORT)
    {
        if (key.port >= RTL8316D_MAX_PORT)
        {
            return RT_ERR_PORT_ID;
        }

        lutOp.defPortNum = 1;
        lutOp.portNum = key.port;        
    }

    if (type & RTL8316D_FLUSH_BY_FID)
    {
        if (key.fid >= RTL8316D_FIDMAX)
        {
            return RT_ERR_L2_FID;
        }

        lutOp.defFid = 1;
        lutOp.fid = key.fid;
    }

    if (type & RTL8316D_FLUSH_BY_MAC)
    {
        lutOp.defMac = 1;
        memcpy(lutOp.mac.octet, key.mac.octet, ETHER_ADDR_LEN);
    }
    
    rtl8316d_addrTbl_op(&lutOp);

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_l2_flushLinkDownPortAddrEnable_set
 * Description:
 *      Set HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      port  -  Port id.
 *      enable     -  link down flush status
 *      key     -  key for flushed item
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The status of flush linkdown port address is as following:
 *           -DISABLED
 *           -ENABLED
 */ 
rtk_api_ret_t rtk_l2_flushLinkDownPortAddrEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    if(port != RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_ID;

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    reg_field_write(RTL8316D_UNIT, SOURCE_MAC_LEARNING_CONTROL, LINK_DOWN_PORT_INVALID, enable);

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_l2_flushLinkDownPortAddrEnable_get
 * Description:
 *      Get HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      port  -  Port id.
 *      enable     -  link down flush status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The status of flush linkdown port address is as following:
 *           -DISABLED
 *           -ENABLED
 */ 
rtk_api_ret_t rtk_l2_flushLinkDownPortAddrEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{

    if(port != RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_ID;  

    if(pEnable == NULL)
        return RT_ERR_NULL_POINTER;   

    reg_field_read(RTL8316D_UNIT, SOURCE_MAC_LEARNING_CONTROL, LINK_DOWN_PORT_INVALID, pEnable);

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_l2_agingEnable_set
 * Description:
 *      Set L2 LUT aging status per port setting.
 * Input:
 *      port  -  Port id.
 *      enable     -  Aging status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      This API can be used to set L2 LUT aging status on port basis. 
 */ 
rtk_api_ret_t rtk_l2_agingEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    uint16 ageUnit;
    
    /*rtl8316d does not support per-port configuration*/
    if(port != RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_ID;

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;
    
    /*0xFA--> 300s*/
    ageUnit = (enable == ENABLED) ? 0xFA : 0;
    reg_field_write(RTL8316D_UNIT, SOURCE_MAC_LEARNING_CONTROL, AGEUNIT, ageUnit);

    return RT_ERR_OK;
}    


/* Function Name:
 *      rtk_l2_agingEnable_get
 * Description:
 *      Get L2 LUT aging status per port setting.
 * Input:
 *      port  -  Port id.
 * Output:
 *      pEnable     -  Aging status
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      This API can be used to get L2 LUT aging function on port basis.  
 */
rtk_api_ret_t rtk_l2_agingEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    uint32 ageUnit;
    
    /*rtl8316d does not support per-port configuration*/
    if(port != RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_ID;

    if(pEnable == NULL)
        return RT_ERR_NULL_POINTER;

    reg_field_read(RTL8316D_UNIT, SOURCE_MAC_LEARNING_CONTROL, AGEUNIT, &ageUnit);
    *pEnable = (ageUnit == 0)? DISABLED : ENABLED;
    
    return RT_ERR_OK;
}    


/* Function Name:
 *      rtk_l2_limitLearningCnt_set
 * Description:
 *       Set per-Port auto learning limit number
 * Input:
 *      port  -  Port id.
 *      mac_cnt  - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_LIMITED_L2ENTRY_NUM - Invalid auto learning limit number
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can set per-port ASIC auto learning limit number from 0(disable learning) 
 *      to 0x1FFFF.
 */
rtk_api_ret_t rtk_l2_limitLearningCnt_set(rtk_port_t port, rtk_mac_cnt_t mac_cnt)
{
    if(port >= RTK_MAX_NUM_OF_PORT)
        return RT_ERR_PORT_ID;

    if(mac_cnt > RTK_MAX_NUM_OF_LEARN_LIMIT)
        return RT_ERR_LIMITED_L2ENTRY_NUM;

    if (mac_cnt == 0)
    {   /*disable*/
        reg_field_write(RTL8316D_UNIT, PORT0_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0+port, MACNUMCTL, 0);
    } else {
        /*enable*/
        reg_field_write(RTL8316D_UNIT, PORT0_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0+port, MACNUMCTL, 1);
        reg_field_write(RTL8316D_UNIT, PORT0_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0+port, MAXMACNUM, mac_cnt);
    }
    
    return RT_ERR_OK;
}    


/* Function Name:
 *      rtk_l2_limitLearningCnt_get
 * Description:
 *       Get per-Port auto learning limit number
 * Input:
 *      port  -  Port id.
 * Output:
 *      pMac_cnt     -  Auto learning entries limit number
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_LIMITED_L2ENTRY_NUM - Invalid auto learning limit number
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *     The API can get per-port ASIC auto learning limit number.
 */
rtk_api_ret_t rtk_l2_limitLearningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt)
{
    if(port >= RTK_MAX_NUM_OF_PORT)
        return RT_ERR_PORT_ID;

    if(pMac_cnt == NULL)
        return RT_ERR_NULL_POINTER;

    /*MAC constraint enabled?*/
    reg_field_read(RTL8316D_UNIT, PORT0_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0+port, MACNUMCTL, pMac_cnt);

    if (*pMac_cnt != 0)
    { /*MAC constraint enabled*/
        reg_field_read(RTL8316D_UNIT, PORT0_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0+port, MAXMACNUM, pMac_cnt);
    }  
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitLearningCntAction_set
 * Description:
 *      Configure auto learn over limit number action.
 * Input:
 *      port  -  Port id.
 *      action  -  Auto learning entries limit action
 * Output:
 *     None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_NOT_ALLOWED - Invalid learn over action
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can set SA unknown packet action while auto learn limit number is over 
 *      The action symbol as following:
 *          -LIMIT_LEARN_CNT_ACTION_DROP,
 *          -LIMIT_LEARN_CNT_ACTION_FWD,
 *          -LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
rtk_api_ret_t rtk_l2_limitLearningCntAction_set(rtk_port_t port, rtk_l2_limitLearnCntAction_t action)
{
    if(port >= RTK_MAX_NUM_OF_PORT)
        return RT_ERR_PORT_ID;

    reg_field_write(RTL8316D_UNIT, PORT0_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0+port, L2LIMACT, action);

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitLearningCntAction_get
 * Description:
 *      Configure auto learn over limit number action.
 * Input:
 *      port  -  Port id.
 * Output:
 *      pAction  -  Learn over action
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can get SA unknown packet action while auto learn limit number is over 
 *      The action symbol as following:
 *          -LIMIT_LEARN_CNT_ACTION_DROP,
 *          -LIMIT_LEARN_CNT_ACTION_FWD,
 *          -LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
rtk_api_ret_t rtk_l2_limitLearningCntAction_get(rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction)
{
    if(port >= RTK_MAX_NUM_OF_PORT)
        return RT_ERR_PORT_ID;
    if (pAction == NULL)
        return RT_ERR_NULL_POINTER;

    reg_field_read(RTL8316D_UNIT, PORT0_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0+port, L2LIMACT, pAction);

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_learningCnt_get
 * Description:
 *      Get per-Port auto learning number
 * Input:
 *      port  -  Port id.
 * Output:
 *      pMac_cnt  -  ASIC auto learning entries number
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      The API can get per-port ASIC auto learning number
 */
rtk_api_ret_t rtk_l2_learningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt)
{
    if(port >= RTK_MAX_NUM_OF_PORT)
        return RT_ERR_PORT_ID;
    if (pMac_cnt == NULL)
        return RT_ERR_NULL_POINTER;

    reg_field_read(RTL8316D_UNIT, PORT0_MAC_ADDRESS_NUMBER_CONSTRAIN_CONTROL0+port, CURMACNUM, pMac_cnt);

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_l2_floodPortMask_set
 * Description:
 *      Set flooding portmask
 * Input:
 *      type  -  flooding type.
 *      flood_portmask  -  flooding porkmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_PORT_MASK - Invalid portmask.
 * Note:
 *      This API set the flooding mask.
 *      The flooding type is as following:
 *           -FLOOD_UNKNOWNDA
 *           -FLOOD_UNKNOWNMC
 *           -FLOOD_BC
 */
rtk_api_ret_t rtk_l2_floodPortMask_set(rtk_l2_flood_type_t floood_type, rtk_portmask_t flood_portmask)
{
//    rtk_api_ret_t retVal;

    if(floood_type >= FLOOD_END)
        return RT_ERR_INPUT; 

    if (flood_portmask.bits[0] > RTK_MAX_PORT_MASK)
        return RT_ERR_PORT_MASK; 
    
    /*not supported in RTL8316D*/
    
    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_l2_floodPortMask_get
 * Description:
 *      Get flooding portmask
 * Input:
 *      type  -  flooding type.
 * Output:
 *      pFlood_portmask  -  flooding porkmask
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API get the flooding mask.
 *      The flooding type is as following:
 *           -FLOOD_UNKNOWNDA
 *           -FLOOD_UNKNOWNMC
 *           -FLOOD_BC
 */
rtk_api_ret_t rtk_l2_floodPortMask_get(rtk_l2_flood_type_t floood_type, rtk_portmask_t *pFlood_portmask)
{
//    rtk_api_ret_t retVal;

    if(floood_type >= FLOOD_END)
        return RT_ERR_INPUT; 

    return RT_ERR_FAILED;
}


/* Function Name:
 *      rtk_l2_localPktPermit_set
 * Description:
 *      Set permittion of frames if source port and destination port are the same.
 * Input:
 *      port  -  Port id.
 *      permit  -  permittion status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_ENABLE - Invalid permit value.
 * Note:
 *      This API is setted to permit frame if its source port is equal to destination port. 
 */
rtk_api_ret_t rtk_l2_localPktPermit_set(rtk_port_t port, rtk_enable_t permit)
{
    rtk_api_ret_t retVal;

    if(port >= RTK_MAX_NUM_OF_PORT)
        return RT_ERR_PORT_ID;

    if(permit >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if((retVal = reg_field_write(RTL8316D_UNIT, PORT0_SOURCE_PORT_FILTER+port, SPFT, permit)) != SUCCESS)
    {
        return retVal;
    }
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_localPktPermit_get
 * Description:
 *      Get permittion of frames if source port and destination port are the same.
 * Input:
 *      port  -  Port id.
 * Output:
 *      pPermit  -  permittion status
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_ENABLE - Invalid permit value.
 * Note:
 *      This API is to get permittion status for frames if its source port is equal to destination port.
 */
rtk_api_ret_t rtk_l2_localPktPermit_get(rtk_port_t port, rtk_enable_t *pPermit)
{
    rtk_api_ret_t retVal;

    if(port >= RTK_MAX_NUM_OF_PORT)
        return RT_ERR_PORT_ID;

    if(pPermit == NULL)
        return RT_ERR_NULL_POINTER;

    if((retVal = reg_field_read(RTL8316D_UNIT, PORT0_SOURCE_PORT_FILTER+port, SPFT, pPermit)) != SUCCESS)
    {
        return retVal;
    }
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_l2_aging_set
 * Description:
 *      Set LUT agging out speed
 * Input:
 *      aging_time  -  Agging out time.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_ENABLE - Invalid permit value.
 * Note:
 *      The API can set LUT agging out period for each entry and the range is from 14s to 800s.
 */
rtk_api_ret_t rtk_l2_aging_set(rtk_l2_age_time_t aging_time) 
{


    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_l2_aging_get
 * Description:
 *      Get LUT agging out time
 * Input:
 *      None
 * Output:
 *      pAging_time  -  Agging out time.
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_ENABLE - Invalid permit value.
 * Note:
 *      The API can get LUT agging out period for each entry. 
 */
rtk_api_ret_t rtk_l2_aging_get(rtk_l2_age_time_t *pAging_time)
{

    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_l2_hashMethod_set
 * Description:
 *      Select LUT hash algorithm.
 * Input:
 *      mode - hash algorithm, HASH_OPT0 or HASH_OPT1 .
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid mode.
 * Note:
 *      
 */
rtk_api_ret_t rtk_l2_hashMethod_set(rtk_hash_method_t mode)
{
    if (mode >= HASH_END)
        return RT_ERR_INPUT;
    
    return reg_field_write(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_CONTROL, HASHALG, mode);
}

/* Function Name:
 *      rtk_l2_hashMethod_get
 * Description:
 *      Get LUT hash algorithm.
 * Input:
 *      None
 * Output:
 *      mode - hash algorithm, HASH_OPT0 or HASH_OPT1 .
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_NULL_POINTER - Invalid mode pointer.
 * Note:
 *      
 */
rtk_api_ret_t rtk_l2_hashMethod_get(rtk_hash_method_t *pMode)
{
    if (pMode == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }
    
    return reg_field_read(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_CONTROL, HASHALG, pMode);
}

/* Function Name:
 *      rtk_l2_ipMcastAddrLookup_set
 * Description:
 *      Configure the method of lookuping address table for IP multicast packets.
 * Input:
 *      type - method type.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_NULL_POINTER - Invalid mode pointer.
 *      RT_ERR_INPUT - Invalid type.
 * Note:
 *      As for IP multicast packets, they could use MAC or SIP+DIP to lookup address table for forwarding.
 */
rtk_api_ret_t rtk_l2_ipMcastAddrLookup_set(rtk_l2_lookup_type_t type)
{
    switch (type)
    {
        case LOOKUP_MAC:    /*go through*/
        case LOOKUP_SIP_DIP:
            reg_field_write(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_CONTROL, IPMC, type);
            break;
        case LOOKUP_DIP: /*RTL8316D not support yet*/
            return RT_ERR_INPUT;
            //break;
        default:
            return RT_ERR_INPUT;
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_ipMcastAddrLookup_get
 * Description:
 *      Get the method of lookuping address table for IP multicast packets.
 * Input:
 *      None
 * Output:
 *      type - method type.
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_NULL_POINTER - Invalid mode pointer.
 * Note:
 *      As for IP multicast packets, they could use MAC or SIP+DIP to lookup address table for forwarding.
 */
rtk_api_ret_t rtk_l2_ipMcastAddrLookup_get(rtk_l2_lookup_type_t *pType)
{
    if (pType == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    if (reg_field_read(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_CONTROL, IPMC, pType) != SUCCESS)
    {
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}


rtk_api_ret_t rtl8316d_fwd_setAsicEntry(uint16 index, rtl8316d_tblasic_fwdTblParam_t * fwd)
{
    rtl8316d_tblasic_fwdTbl_t asicentry;
    uint32 retvalue;

    if (index >= RTL8316D_FWD_NUMBER || fwd == NULL)
    {
        return -1;
    }

    memset(&asicentry, 0, sizeof(asicentry));
    asicentry.age = fwd->age;
    asicentry.crsvlan = fwd->crsvlan;
    asicentry.portmask = fwd->portmask;

    retvalue = table_write(RTL8316D_UNIT, FORWARDING, index, (uint32 *)&asicentry);
    if (retvalue != 0)
    {
        rtlglue_printf("write forwarding table entry %d failed\n", index);
        return -1;
    }

    return SUCCESS;
}


rtk_api_ret_t rtl8316d_fwd_getAsicEntry(uint16 index, rtl8316d_tblasic_fwdTblParam_t * fwd)
{
    rtl8316d_tblasic_fwdTbl_t asicentry;
    uint32 retvalue;

    if (index >= RTL8316D_FWD_NUMBER || fwd == NULL)
    {
        return -1;
    }

    memset(&asicentry, 0, sizeof(asicentry));
    memset(fwd, 0, sizeof(rtl8316d_tblasic_fwdTblParam_t));
    retvalue = table_read(RTL8316D_UNIT, FORWARDING, index, (uint32 *)&asicentry);
    if (retvalue != 0)
    {
        rtlglue_printf("read forwarding table entry %d failed\n", index);
        return -1;
    }

    fwd->age = asicentry.age;
    fwd->crsvlan = asicentry.crsvlan;
    fwd->portmask  = asicentry.portmask;

    return SUCCESS;
}

int8 rtl8316d_fwd_delAsicEntry(uint16 index)
{
    rtl8316d_tblasic_fwdTbl_t asicEntry;

    if (index >= RTL8316D_FWD_NUMBER)
    {
        return -1;
    }

    memset(&asicEntry, 0, sizeof(asicEntry));       
    fwdTblStat[index/8] &= ~(1 << (index%8));
    rtl8316d_tableWrite(FORWARDING, index, (uint32*)&asicEntry);
    
    return SUCCESS;
}


rtk_api_ret_t rtl8316d_fwd_fstAvalIdx(uint8* idx)
{   
    uint8 i, j;

    for (i = 0; i < RTL8316D_FWD_NUMBER/8; i++ )
        for (j = 0; j < 8; j++)
        {
            if (((fwdTblStat[i]>>j) & 0x1) == 0)
            {
                *idx = i*8 + j;
                return RT_ERR_OK;
            }
                
        }
        
    return RT_ERR_FAILED;
}

int8 rtl8316d_addrTbl_op(rtl8316d_lut_opeartions_t* lutOp)
{
    uint32 xdata regVal, xdata fieldVal;
    uint32 xdata mac;

    if (lutOp == NULL)
        return -1;
    
    regVal = fieldVal = mac = 0;
    reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL0, OPERATION, lutOp->opType, &regVal);
    reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL0, CPUOWNINCL, lutOp->cpuown, &regVal);
    reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL0, FID, lutOp->fid, &regVal);
    reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL0, SPFID, lutOp->defFid, &regVal);
    reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL0, SPPORT, lutOp->defPortNum, &regVal);
    reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL0, PORTNUMBER, lutOp->portNum, &regVal);
    reg_write(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL0, regVal);

    if (lutOp->defMac)
    {
         regVal = 0;
         mac = lutOp->mac.octet[0]<<8 | lutOp->mac.octet[1];
         reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL1, SPMAC, 1, &regVal);
         reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL1, SPMAC_47_32, mac, &regVal);
         reg_write(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL1, regVal);

         mac = lutOp->mac.octet[2]<<24 | lutOp->mac.octet[3]<<16 | lutOp->mac.octet[4]<<8 | lutOp->mac.octet[5];
         reg_field_write(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL2, SPMAC_31_0, mac);
    }
     regVal = 0;
     reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL3, STARTIDX, lutOp->startIdx, &regVal);
     reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL3, LASTIDX, lutOp->lastIdx, &regVal);
 //    ASSERT(reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL3, FOUND, lutOp->found, &regVal) == SUCCESS);
     reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL3, ACTTRIGGER, 1, &regVal);
     reg_write(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL3, regVal);

     do{
         reg_field_read(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL3, ACTTRIGGER, &regVal);
     } while(regVal != 0);

    /*search operation should return information to user*/
    if (lutOp->opType == 1)
    {
       reg_read(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL0, &regVal);
       reg_field_get(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL0, CPUOWNINCL, &fieldVal, regVal);
       lutOp->cpuown = fieldVal;
       reg_field_get(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL0, FID, &fieldVal, regVal);
       lutOp->fid = fieldVal;
       reg_field_get(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL0, PORTNUMBER, &fieldVal, regVal);
       lutOp->portNum = fieldVal;

        /*if MAC not provided, return it to user*/
        if (lutOp->defMac == 0)
        {
            reg_field_get(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL1, SPMAC_47_32, &mac, regVal);
            lutOp->mac.octet[0] = (mac>>8) & 0xFF;
            lutOp->mac.octet[1] = mac & 0xFF;    

            reg_field_read(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL2, SPMAC_31_0, &mac);
            lutOp->mac.octet[2] = (mac >> 24) & 0xFF;
            lutOp->mac.octet[3] = (mac >> 16) & 0xFF;
            lutOp->mac.octet[4] = (mac >> 8) & 0xFF;
            lutOp->mac.octet[5] = mac & 0xFF;
        }
        
        reg_read(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL3, &regVal);
        reg_field_get(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL3, STARTIDX, &fieldVal, regVal);
        lutOp->startIdx = fieldVal;
        reg_field_get(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL3, LASTIDX, &fieldVal, regVal);
        lutOp->lastIdx = fieldVal;
        reg_field_get(RTL8316D_UNIT, ADDRESS_TABLE_OPERATE_CONTROL3, FOUND, &fieldVal, regVal);
        lutOp->found = fieldVal;
    }
    
    return SUCCESS;
}


/* Function Name:
 *      rtk_trap_unknownUnicastPktAction_set
 * Description:
 *      Set unknown unicast packet action configuration.
 * Input:
 *      type - Unknown unicast packet type.
 *      ucast_action - Unknown unicast action. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_NULL_POINTER - Invalid mode pointer.
 *      RT_ERR_NOT_ALLOWED - Invalid action.
 * Note:
 *    This API can set unknown unicast packet action configuration.
 *   The unknown unicast packet type is as following:
 *        -UCAST_UNKNOWNDA
 *   The unknown unicast action is as following:
 *        -UCAST_ACTION_FORWARD 
 *        -UCAST_ACTION_DROP
 *        -UCAST_ACTION_TRAP2CPU
 *        -UCAST_ACTION_FLOOD
 *    UCAST_ACTION_FORWARD means flood within VLAN, while UCAST_ACTION_FLOOD would flood to all ports.
 */
rtk_api_ret_t rtk_trap_unknownUnicastPktAction_set(rtk_trap_ucast_type_t type, rtk_trap_ucast_action_t ucast_action)
{
    rtk_api_ret_t retVal;

    if (ucast_action >= UCAST_ACTION_END)
    {
        return RT_ERR_INPUT;
    }
   
    switch (type)
    {
        case UCAST_UNKNOWNDA:
            if((retVal= reg_field_write(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_MISS_CONTROL, L2UMISSOP, ucast_action))!=RT_ERR_OK)
                return retVal; 
            break;
        case UCAST_UNKNOWNSA:
                return RT_ERR_NOT_ALLOWED;
        case UCAST_UNMATCHSA:
                return RT_ERR_NOT_ALLOWED;
        default:
            return RT_ERR_INPUT;
    }
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_unknownUnicastPktAction_get
 * Description:
 *      Get unknown unicast packet action configuration.
 * Input:
 *      type - Unknown unicast packet type.
 * Output:
 *      pUcast_action - unknown unicast action. 
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *    This API can get unknown unicast packet action configuration.
 *   The unknown unicast packet type is as following:
 *        -UCAST_UNKNOWNDA
 *   The unknown unicast action is as following:
 *        -UCAST_ACTION_FORWARD 
 *        -UCAST_ACTION_DROP
 *        -UCAST_ACTION_TRAP2CPU
 *        -UCAST_ACTION_FLOOD
 *    UCAST_ACTION_FORWARD means flood within VLAN, while UCAST_ACTION_FLOOD would flood to all ports.
 */
rtk_api_ret_t rtk_trap_unknownUnicastPktAction_get(rtk_trap_ucast_type_t type, rtk_trap_ucast_action_t *pUcast_action)
{
    rtk_api_ret_t retVal;
//    uint32 action;

    if (pUcast_action == NULL)
    {
        return RT_ERR_NULL_POINTER; 
    }
    
    switch (type)
    {
        case UCAST_UNKNOWNDA:
            if((retVal= reg_field_read(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_MISS_CONTROL, L2UMISSOP, pUcast_action)) != RT_ERR_OK)
                return retVal; 
            break;
        case UCAST_UNKNOWNSA:
                return RT_ERR_NOT_ALLOWED;
        case UCAST_UNMATCHSA:
                return RT_ERR_NOT_ALLOWED;
        default:
            return RT_ERR_INPUT;
    }
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_trap_unknownMcastPktAction_set
 * Description:
 *      Set behavior of unknown multicast
 * Input:
 *      port - Port id.
 *      type - unknown multicast packet type.
 *      mcast_action - unknown multicast action. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_NOT_ALLOWED - Invalid operation.
 * Note:
 *      When receives an unknown multicast packet, switch may trap, drop or flood this packet
 *      The unknown multicast packet type is as following:
 *          -MCAST_L2
 *          -MCAST_IPV4
 *          -MCAST_IPV6
 *          -MCAST_L3
 *      The unknown multicast action is as following:
 *          -MCAST_ACTION_DROP 
 *          -MCAST_ACTION_TRAP2CPU    
 *          -MCAST_ACTION_FORWARD
 *          -MCAST_ACTION_FLOOD
 *       MCAST_ACTION_FORWARD means flood in VLAN, while MCAST_ACTION_FLOOD means flood to all ports. 
 */
rtk_api_ret_t rtk_trap_unknownMcastPktAction_set(rtk_port_t port, rtk_mcast_type_t type, rtk_trap_mcast_action_t mcast_action)
{
    rtk_api_ret_t retVal;

    if(port >= RTK_MAX_NUM_OF_PORT)
        return RT_ERR_PORT_ID;

    if(type >= MCAST_END)
        return RT_ERR_INPUT; 

    if (mcast_action >= MCAST_ACTION_END)
        return RT_ERR_INPUT; 
    

    switch (type)
    {
        case MCAST_L2:
                retVal = reg_field_write(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_MISS_CONTROL, L2MMISSOP, mcast_action);
            break;
        case MCAST_IPV4:
                retVal = reg_field_write(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_MISS_CONTROL, V4MCSLKMSOP, mcast_action);
            break;
        case MCAST_IPV6:
                retVal = reg_field_write(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_MISS_CONTROL, V6MCSLKMSOP, mcast_action);
            break;
        case MCAST_L3:
                retVal = reg_field_write(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_MISS_CONTROL, L3MMISSOP, mcast_action);
            break;
        default:
           return RT_ERR_INPUT;
    }

    if (retVal != SUCCESS)
        return retVal;
    
    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_trap_unknownMcastPktAction_get
 * Description:
 *      Get behavior of unknown multicast
 * Input:
 *      port - Port id.
 *      type - unknown multicast packet type.
 * Output:
 *      pMcast_action - unknown multicast action. 
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input type.
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_NULL_POINTER - Null pointer input.
 * Note:
 *      When receives an unknown multicast packet, switch may trap, drop or flood this packet
 *      The unknown multicast packet type is as following:
 *          -MCAST_L2
 *          -MCAST_IPV4
 *          -MCAST_IPV6
 *          -MCAST_L3
 *      The unknown multicast action is as following:
 *          -MCAST_ACTION_DROP 
 *          -MCAST_ACTION_TRAP2CPU    
 *          -MCAST_ACTION_FORWARD
 *          -MCAST_ACTION_FLOOD
 *       MCAST_ACTION_FORWARD means flood in VLAN, while MCAST_ACTION_FLOOD means flood to all ports. 
 */
rtk_api_ret_t rtk_trap_unknownMcastPktAction_get(rtk_port_t port, rtk_mcast_type_t type, rtk_trap_mcast_action_t *pMcast_action)
{
    rtk_api_ret_t retVal;

    if(port > RTK_MAX_NUM_OF_PORT)
        return RT_ERR_PORT_ID; 

    if(type >= MCAST_END)
        return RT_ERR_INPUT; 

    if (pMcast_action == NULL)
        return RT_ERR_NULL_POINTER;
    
    switch (type)
    {
        case MCAST_L2:
                retVal = reg_field_read(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_MISS_CONTROL, L2MMISSOP, pMcast_action);
            break;
        case MCAST_IPV4:
                retVal = reg_field_read(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_MISS_CONTROL, V4MCSLKMSOP, pMcast_action);
            break;
        case MCAST_IPV6:
                retVal = reg_field_read(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_MISS_CONTROL, V6MCSLKMSOP, pMcast_action);
            break;
        case MCAST_L3:
                retVal = reg_field_read(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_MISS_CONTROL, L3MMISSOP, pMcast_action);
            break;
        default:
            return RT_ERR_INPUT;
    }

    if (retVal != SUCCESS)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_trap_igmpCtrlPktAction_set
 * Description:
 *      Set IGMP/MLD trap function
 * Input:
 *      type -  IGMP/MLD packet type.
 *      igmp_action - IGMP/MLD action. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input type.
 *      RT_ERR_NOT_ALLOWED - Invalid action
 * Note:
 *      This API can set both IPv4 IGMP/IPv6 MLD trapping function.
 *      IGMP and MLD can be set seperately.
 *      The IGMP/MLD packet type is as following:
 *           -IGMP_IPV4
 *           -IGMP_MLD
 *      The IGMP/MLD action is as following:
 *           -IGMP_ACTION_FORWARD
 *           -IGMP_ACTION_TRAP2CPU
 *           -IGMP_ACTION_FORWARD_COPY2_CPU
 *           -IGMP_ACTION_DROP
 */
rtk_api_ret_t rtk_trap_igmpCtrlPktAction_set(rtk_igmp_type_t type, rtk_trap_igmp_action_t igmp_action)
{
    rtk_api_ret_t retVal;

    if(type >= IGMP_TYPE_END)
        return RT_ERR_INPUT; 

    if (igmp_action >= IGMP_ACTION_END)
        return RT_ERR_NOT_ALLOWED; 

    if (type == IGMP_IPV4)
    {
        retVal = reg_field_write(RTL8316D_UNIT, IGMP_MLD_CONTROL, IGMPTRAP, igmp_action);
    } else {
        retVal = reg_field_write(RTL8316D_UNIT, IGMP_MLD_CONTROL, MLDTRAP, igmp_action);
    }
    

    if (retVal != SUCCESS)
    {
        return retVal;
    }
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_igmpCtrlPktAction_get
 * Description:
 *      Get IGMP/MLD trap function
 * Input:
 *      type -  IGMP/MLD packet type.
 * Output:
 *      pIgmp_action - IGMP/MLD action. 
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input type.
 * Note:
 *      This API can get both IPv4 IGMP/IPv6 MLD trapping function.
 *      The IGMP/MLD packet type is as following:
 *           -IGMP_IPV4
 *           -IGMP_MLD
 *      The IGMP/MLD action is as following:
 *           -IGMP_ACTION_FORWARD
 *           -IGMP_ACTION_TRAP2CPU
 *           -IGMP_ACTION_FORWARD_COPY2_CPU
 *           -IGMP_ACTION_DROP
 */
rtk_api_ret_t rtk_trap_igmpCtrlPktAction_get(rtk_igmp_type_t type, rtk_trap_igmp_action_t *pIgmp_action)
{
    rtk_api_ret_t retVal;

    if(type >= IGMP_TYPE_END)
        return RT_ERR_INPUT;     

    if (pIgmp_action == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    if (type == IGMP_IPV4)
    {
        retVal = reg_field_read(RTL8316D_UNIT, IGMP_MLD_CONTROL, IGMPTRAP, pIgmp_action);
    } else {
        retVal = reg_field_read(RTL8316D_UNIT, IGMP_MLD_CONTROL, MLDTRAP, pIgmp_action);
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_trap_reasonTrapToCPUPriority_get
 * Description:
 *       Get priority assigned to packets when trapped to CPU
 * Input:
 *      type -  trap reason.
 * Output:
 *      pPriority - priority retrived. 
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input type.
 *      RT_ERR_NULL_POINTER - Null pointer input
 * Note:
 *      There are a couple of reasons that packets would be trapped to CPU. This API could be used to
 *       retrive the priority assigned to packets trapped to CPU for specific reason.
 */
rtk_api_ret_t rtk_trap_reasonTrapToCPUPriority_get(rtk_trap_reason_type_t type, rtk_pri_t *pPriority)
{
    rtk_api_ret_t retVal;
    
    if (pPriority == NULL)
    {
        return RT_ERR_NULL_POINTER;
    }

    retVal = RT_ERR_FAILED;

    switch(type)
    {
        case TRAP_REASON_UNMATCHIPMC:
                retVal = reg_field_read(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_CONTROL, IPMCPRI, pPriority);
            break;

        default:
            return RT_ERR_INPUT;
    }

    return retVal;
}


/* Function Name:
 *      rtk_trap_reasonTrapToCPUPriority_set
 * Description:
 *       Set priority assigned to packets when trapped to CPU
 * Input:
 *      type -  trap reason.
 *      priority - priority assigned . 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK - 
 *      RT_ERR_FAILED  - 
 *      RT_ERR_INPUT - Invalid input priority or type.
 * Note:
 *      There are a couple of reasons that packets would be trapped to CPU. This API could be used to
 *       set the priority assigned to packets trapped to CPU for specific reason.
 */
rtk_api_ret_t rtk_trap_reasonTrapToCPUPriority_set(rtk_trap_reason_type_t type, rtk_pri_t priority)
{
//    rtk_api_ret_t retVal;
    uint32 regVal;

    if (priority > RTL8316D_PRI_MAX)
    {
        return RT_ERR_INPUT;
    }
    
    switch(type)
    {
        case TRAP_REASON_UNMATCHIPMC:
            if (reg_read(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_CONTROL, &regVal))
            {
                reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_CONTROL, DFIPMCPRI, 1, &regVal);
                reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_CONTROL, IPMCPRI, priority, &regVal);
                reg_write(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_CONTROL, regVal);
            }
            break;
        case TRAP_REASON_LOOKUPMISS:
            if (reg_read(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_MISS_CONTROL, &regVal))
            {
                reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_MISS_CONTROL, DFRLMPRI, 1, &regVal);
                reg_field_set(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_MISS_CONTROL, LMPRI, priority, &regVal);
                reg_write(RTL8316D_UNIT, ADDRESS_TABLE_LOOKUP_MISS_CONTROL, regVal);
            }
            break;
        case TRAP_REASON_IGMPMLD:
            if (reg_read(RTL8316D_UNIT, IGMP_MLD_CONTROL, &regVal))
            {
                reg_field_set(RTL8316D_UNIT, IGMP_MLD_CONTROL, DFIGMPPRI, 1, &regVal);
                reg_field_set(RTL8316D_UNIT, IGMP_MLD_CONTROL, IGMPPRI, priority, &regVal);
                reg_write(RTL8316D_UNIT, IGMP_MLD_CONTROL, regVal);
            }
            break;
        case TRAP_REASON_CFI1:
            if (reg_read(RTL8316D_UNIT, CFI_CONTROL, &regVal))
            {
                reg_field_set(RTL8316D_UNIT, CFI_CONTROL, DFCFIPRI, 1, &regVal);
                reg_field_set(RTL8316D_UNIT, CFI_CONTROL, CFIPRI, priority, &regVal);
                reg_write(RTL8316D_UNIT, CFI_CONTROL, regVal);
            }
            break;
        default:
            return RT_ERR_INPUT;
    }

    return RT_ERR_OK;
}



