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
* Purpose :  ASIC-level driver implementation for port property..
*
*  Feature :  This file consists of following modules:
*             1) 
*
*/
#include <rtk_types.h>
#include <rtk_error.h>
#include <rtk_api.h>
#include <asicdrv/rtl8316d_types.h>
#include <rtl8316d_asicDrv.h>
#include <rtl8316d_reg_struct.h>
#include <rtl8316d_asicdrv_port.h>

/* Function Name:
 *      rtk_port_phyStatus_get
 * Description:
 *      Get ethernet PHY linking status
 * Input:
 *      port             -  Port id
 * Output:
 *      pLinkStatus   -  the pointer of PHY link status 
 *      pSpeed         -  the pointer of PHY link speed
 *      pDuplex        -  the pointer of PHY duplex 
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_NULL_POINTER
 *     
 * Note:
 *      the API could get port status(link/duplex/speed).
 */
rtk_api_ret_t rtk_port_phyStatus_get(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus, rtk_port_speed_t *pSpeed, rtk_port_duplex_t *pDuplex)
{
    uint32 regval;
    uint16 reg1_val;
    
    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 
    
    if((NULL == pLinkStatus) || (NULL == pSpeed) || (NULL == pDuplex))
        return RT_ERR_NULL_POINTER;
    
    phy_reg_read((uint32)port, 1, 0, &reg1_val);
    
    /*twice read will be stable*/
    CHK_FUN_RETVAL(reg_read(RTL8316D_UNIT, PORT0_LINK_STATUS + port, &regval));
    CHK_FUN_RETVAL(reg_read(RTL8316D_UNIT, PORT0_LINK_STATUS + port, &regval));    
//    *pLinkStatus = (regval & 0x1) ? PORT_LINKUP: PORT_LINKDOWN;
    *pLinkStatus = (reg1_val >> 2) & 0x01;
    *pSpeed = (rtk_port_speed_t)((regval & (0x3 << 2)) >> 2); 
    *pDuplex = (regval & (0x1 << 4)) ? (PORT_FULL_DUPLEX) : (PORT_HALF_DUPLEX);
        
    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_adminEnable_set
 * Description:
 *      Set port admin configuration of the specific port.
 * Input:
 *      port    - port id.
 *      enable  - Back pressure status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can set port admin configuration of the specific port.
 *      The port admin configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t rtk_port_adminEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    return rtl8316d_portStat_Enable_set(port, enable);
}

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_set
 * Description:
 *      Set ethernet PHY auto-negotiation desired ability
 * Input:
 *      port       -  Port id
 *      pAbility   -  Ability structure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_PHY_REG_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_BUSYWAIT_TIMEOUT
 *     
 * Note:
 *      In auto-negotiation mode,  phy autoNegotiation ability is enabled.     
 *      If Full_1000 bit is set to 1, the AutoNegotiation will be automatic set to 1. 
 *      While both AutoNegotiation and Full_1000 are set to 0, the PHY speed and duplex selection will
 *      be set as following 100F > 100H > 10F > 10H priority sequence.
 */
rtk_api_ret_t rtk_port_phyAutoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    uint32 mode;        
    uint32 phyProperty;    
    uint32 regval, pollcnt, finished;    


    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;         

    if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;

    if(pAbility->Half_10 >= RTK_ENABLE_END  || pAbility->Full_10 >= RTK_ENABLE_END ||
       pAbility->Half_100 >= RTK_ENABLE_END || pAbility->Full_100 >= RTK_ENABLE_END ||
       pAbility->Full_1000 >= RTK_ENABLE_END || pAbility->AutoNegotiation >= RTK_ENABLE_END ||       
       pAbility->AsyFC >= RTK_ENABLE_END || pAbility->FC >= RTK_ENABLE_END)
        return RT_ERR_INPUT; 

    CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, GLOBAL_MAC_INTERFACE_CONTROL0, CHIP_TYPE_2_0, &mode));                   
    if((1 == pAbility->Full_1000))    
    {
        if(PN_PORT22 == port)
        {
            if(mode != 0x1)
                pAbility->Full_1000 = 0;                                           
        }
        
        if(PN_PORT23 == port)
        {
            if((mode != 0x1) && (mode != 0x0))
                pAbility->Full_1000 = 0;                                                           
        }
        
        if(PN_PORT24 == port)
        {
            if((mode != 0x7) && (mode != 0x5) && (mode != 0x5))
                pAbility->Full_1000 = 0;                                                                       
        }        
    }    
    
    /*for PHY auto mode setup*/
    pAbility->AutoNegotiation = 1;    
    
    /*configure port property*/
    CHK_FUN_RETVAL(reg_read(RTL8316D_UNIT, PORT0_PROPERTY_CONFIGURE + port, &phyProperty));                       
    phyProperty &= (~0x7FAUL);
    phyProperty |= ((uint32)pAbility->AutoNegotiation) << 3;
    phyProperty |= ((uint32)pAbility->Half_10) << 4;
    phyProperty |= ((uint32)pAbility->Full_10) << 5;
    phyProperty |= ((uint32)pAbility->Half_100) << 6;    
    phyProperty |= ((uint32)pAbility->Full_100) << 7;
    phyProperty |= ((uint32)pAbility->Full_1000) << 8;   
    phyProperty |= ((uint32)pAbility->FC) << 9;
    phyProperty |= ((uint32)pAbility->AsyFC) << 10;
    CHK_FUN_RETVAL(reg_write(RTL8316D_UNIT, PORT0_PROPERTY_CONFIGURE + port, phyProperty));                        
    
    /*triger, write configure to phy*/
    regval = (1UL << 25) | ( 1UL << port);
    CHK_FUN_RETVAL(reg_write(RTL8316D_UNIT, PHY_AUTO_ACCESS_MASK0, regval));
    
    for (pollcnt = 0; pollcnt < RTL8316D_MAX_POLLCNT; pollcnt ++)    
    {
        CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PHY_AUTO_ACCESS_MASK0, PHYCFG_CMD, &finished));
        if(!finished)
            break;
    }

    if(RTL8316D_MAX_POLLCNT == pollcnt)
        return RT_ERR_BUSYWAIT_TIMEOUT;

    return RT_ERR_OK;
    
}

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_get
 * Description:
 *      Get ethernet PHY auto-negotiation desired ability
 * Input:
 *      port       -  Port id
 * Output:
 *      pAbility   -  Ability structure
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_PHY_REG_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_BUSYWAIT_TIMEOUT
 *     
 * Note:
 *      In auto-negotiation mode,  phy autoNegotiation ability must be enabled.     
 *      If Full_1000 bit is set to 1, the AutoNegotiation will be automatic set to 1. 
 *      While both AutoNegotiation and Full_1000 are set to 0, the PHY speed and duplex selection will
 *      be set as following 100F > 100H > 10F > 10H priority sequence.
 */    
rtk_api_ret_t rtk_port_phyAutoNegoAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    uint32 phyProperty;
    
    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;

    /*read port property*/
    CHK_FUN_RETVAL(reg_read(RTL8316D_UNIT, PORT0_PROPERTY_CONFIGURE + port, &phyProperty));                       
    
    pAbility->AutoNegotiation = (phyProperty & ( 1UL << 3)) ? 1:0;
    pAbility->Half_10 = (phyProperty & ( 1UL << 4)) ? 1:0;
    pAbility->Full_10 = (phyProperty & ( 1UL << 5)) ? 1:0;    
    pAbility->Half_100 = (phyProperty & ( 1UL << 6)) ? 1:0;        
    pAbility->Full_100 = (phyProperty & ( 1UL << 7)) ? 1:0;    
    pAbility->Full_1000 = (phyProperty & ( 1UL << 8)) ? 1:0;        
    pAbility->FC = (phyProperty & ( 1UL << 9)) ? 1:0;        
    pAbility->AsyFC = (phyProperty & ( 1UL << 10)) ? 1:0;        

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_phyForceModeAbility_set
 * Description:
 *      Set the port PHY force mode, config its speed/duplex mode/pause/asy_pause 
 * Input:
 *      port       -  Port id
 *      pAbility   -  Ability structure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_PHY_REG_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_BUSYWAIT_TIMEOUT
 *     
 * Note:
 *      In force mode,  phy autoNegotiation ability must be disabled.
 *      If Full_1000 bit is set to 1, the AutoNegotiation will be automatic set to 1.
 *      so it could not use this API to force phy mode as Full_1000.
 */

rtk_api_ret_t rtk_port_phyForceModeAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    uint32 mode;        
    uint32 phyProperty;    
    uint32 regval, pollcnt, finished;    


    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID;         

    if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;

    if(pAbility->Half_10 >= RTK_ENABLE_END  || pAbility->Full_10 >= RTK_ENABLE_END ||
       pAbility->Half_100 >= RTK_ENABLE_END || pAbility->Full_100 >= RTK_ENABLE_END ||
       pAbility->Full_1000 >= RTK_ENABLE_END || pAbility->AutoNegotiation >= RTK_ENABLE_END ||       
       pAbility->AsyFC >= RTK_ENABLE_END || pAbility->FC >= RTK_ENABLE_END)
        return RT_ERR_INPUT; 

    CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, GLOBAL_MAC_INTERFACE_CONTROL0, CHIP_TYPE_2_0, &mode));                   
    if((1 == pAbility->Full_1000))    
    {
        if(PN_PORT22 == port)
        {
            if(mode != 0x1)
                pAbility->Full_1000 = 0;                                           
        }
        
        if(PN_PORT23 == port)
        {
            if((mode != 0x1) && (mode != 0x0))
                pAbility->Full_1000 = 0;                                                           
        }
        
        if(PN_PORT24 == port)
        {
            if((mode != 0x7) && (mode != 0x5) && (mode != 0x5))
                pAbility->Full_1000 = 0;                                                                       
        }        
    }    
    
    /*disable PHY auto mode */
    pAbility->AutoNegotiation = 0;    
    
    /*configure port property*/
    CHK_FUN_RETVAL(reg_read(RTL8316D_UNIT, PORT0_PROPERTY_CONFIGURE + port, &phyProperty));                       
    phyProperty &= (~0x7FAUL);
    phyProperty |= ((uint32)pAbility->AutoNegotiation) << 3;
    phyProperty |= ((uint32)pAbility->Half_10) << 4;
    phyProperty |= ((uint32)pAbility->Full_10) << 5;
    phyProperty |= ((uint32)pAbility->Half_100) << 6;    
    phyProperty |= ((uint32)pAbility->Full_100) << 7;
    phyProperty |= ((uint32)pAbility->Full_1000) << 8;   
    phyProperty |= ((uint32)pAbility->FC) << 9;
    phyProperty |= ((uint32)pAbility->AsyFC) << 10;
    CHK_FUN_RETVAL(reg_write(RTL8316D_UNIT, PORT0_PROPERTY_CONFIGURE + port, phyProperty));                        
    
    /*triger, write configure to phy*/
    regval = (1UL << 25) | ( 1UL << port);
    CHK_FUN_RETVAL(reg_write(RTL8316D_UNIT, PHY_AUTO_ACCESS_MASK0, regval));
    
    for (pollcnt = 0; pollcnt < RTL8316D_MAX_POLLCNT; pollcnt ++)    
    {
        CHK_FUN_RETVAL(reg_field_read(RTL8316D_UNIT, PHY_AUTO_ACCESS_MASK0, PHYCFG_CMD, &finished));
        if(!finished)
            break;
    }

    if(RTL8316D_MAX_POLLCNT == pollcnt)
        return RT_ERR_BUSYWAIT_TIMEOUT;

    return RT_ERR_OK;

}

/* Function Name:
 *      rtk_port_phyForceModeAbility_get
 * Description:
 *      Get the port PHY speed/duplex mode/pause/asy_pause in force mode
 * Input:
 *      port       -  Port id
 * Output:
 *      pAbility   -  Ability structure
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_PHY_REG_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_BUSYWAIT_TIMEOUT
 *     
 * Note:
 *      In force mode, phy autoNegotiation ability must be disabled.
 */
rtk_api_ret_t rtk_port_phyForceModeAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    uint32 phyProperty;
    
    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if(NULL == pAbility)
        return RT_ERR_NULL_POINTER;

    /*read port property*/
    CHK_FUN_RETVAL(reg_read(RTL8316D_UNIT, PORT0_PROPERTY_CONFIGURE + port, &phyProperty));                       
    
    pAbility->AutoNegotiation = (phyProperty & ( 1 << 3)) ? 1:0;
    pAbility->Half_10 = (phyProperty & ( 1 << 4)) ? 1:0;
    pAbility->Full_10 = (phyProperty & ( 1 << 5)) ? 1:0;    
    pAbility->Half_100 = (phyProperty & ( 1 << 6)) ? 1:0;        
    pAbility->Full_100 = (phyProperty & ( 1 << 7)) ? 1:0;    
    pAbility->Full_1000 = (phyProperty & ( 1 << 8)) ? 1:0;        
    pAbility->FC = (phyProperty & ( 1 << 9)) ? 1:0;        
    pAbility->AsyFC = (phyProperty & ( 1 << 10)) ? 1:0;    

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_port_backpressureEnable_set
 * Description:
 *      Set the half duplex backpressure enable status of the specific port.
 * Input:
 *      port    - port id.
 *      enable  - Back pressure status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can set the half duplex backpressure enable status of the specific port.
 *      The half duplex backpressure enable status of the port is as following:
 *      - DISABLE(Defer)
 *      - ENABLE (Backpressure)
 */
rtk_api_ret_t rtk_port_backpressureEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if(port > RTL8316D_MAX_PORT)
        return RT_ERR_PORT_ID; 

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if((retVal= reg_field_write(RTL8316D_UNIT, PORT0_L2_MISC0+port, ENBKPRS, enable))!=RT_ERR_OK)
        return retVal;        

    return RT_ERR_OK;
}

#if 0
uint8 port_en[RTL8316D_MAX_PHY_PORT];

rtk_web_portLinkStat_t port_stat[RTL8316D_MAX_PHY_PORT];

rtk_port_phy_ability_t port_ability[RTL8316D_MAX_PHY_PORT];
static uint8 first = true;


void init_port(void)
{
    int8 i;
    
    if (first == true)
    {
        for (i = 0; i < RTL8316D_MAX_PHY_PORT; i++)
        {
            port_en[i]   = 1;

            port_stat[i].Link       = TRUE;
            port_stat[i].AutoNego   = TRUE;
            port_stat[i].spd        = PORT_SPEED_100M;
            port_stat[i].duplex     = PORT_FULL_DUPLEX;
            port_stat[i].FC         = TRUE;

            port_ability[i].AutoNegotiation = TRUE;
            port_ability[i].Half_10    = TRUE;
            port_ability[i].Full_10    = TRUE;
            port_ability[i].Half_100   = TRUE;
            port_ability[i].Full_100   = TRUE;
            port_ability[i].Full_1000  = TRUE;
            port_ability[i].FC         = TRUE;
        }
        
        first = false;
    }
}


int32 rtk_web_portEn_get(rtk_port_t port, rtk_enable_t* pEnable)
{
    init_port();
    //printf("port enable get\n");
    pEnable = port_en[port];

    return RT_ERR_OK; 
}

int32 rtk_web_portEn_set(rtk_port_t port, rtk_enable_t enable)
{
    init_port();
    //printf("port enable set\n");
    port_en[port] = enable;

    return RT_ERR_OK;
}

int32 rtk_web_portLinkStatus_get(rtk_port_t port, rtk_web_portLinkStat_t* pStat)
{
    init_port();
    //printf("port %bd link status get\n", (int8)port);
    if (port == 1)
        port_stat[1].Link ^= 0x01; // reserse the link up status
    pStat->Link     = port_stat[port].Link;
    pStat->AutoNego = port_stat[port].AutoNego;
    pStat->duplex   = port_stat[port].duplex; 
    pStat->spd      = port_stat[port].spd;
    pStat->FC       = port_stat[port].FC; 

    return RT_ERR_OK;  
}

int32 rtk_web_portLinkAbility_get(rtk_port_t port, rtk_port_phy_ability_t* pAbility)
{
    init_port();
    //printf("port link ability get\n");
    pAbility->AutoNegotiation = port_ability[port].AutoNegotiation;
    pAbility->Half_10   =   port_ability[port].Half_10;
    pAbility->Full_10   =   port_ability[port].Full_10;
    pAbility->Half_100  =   port_ability[port].Half_100;
    pAbility->Full_100  =   port_ability[port].Full_100;
    pAbility->Full_1000 =   port_ability[port].Full_1000;
    pAbility->FC        =   port_ability[port].FC;

    return RT_ERR_OK;
}

int32 rtk_web_portLinkAbility_set(rtk_port_t port, rtk_port_phy_ability_t* ability)
{    
    init_port();
    //printf("port link ability set\n");
    port_ability[port].AutoNegotiation  = ability->AutoNegotiation;
    port_ability[port].Half_10  =   ability->Half_10;
    port_ability[port].Full_10  =   ability->Full_10;
    port_ability[port].Half_100 =   ability->Half_100;
    port_ability[port].Full_100 =   ability->Full_100;
    port_ability[port].Full_1000=   ability->Full_1000;
    port_ability[port].FC         =   ability->FC;

    if (ability->AutoNegotiation == 0) // Force mode
    {
        if (ability->Full_10 == 1 || ability->Full_100 == 1 || ability->Full_1000 == 1)
            port_stat[port].duplex = PORT_FULL_DUPLEX;
        else
            port_stat[port].duplex = PORT_HALF_DUPLEX;

        if (ability->Full_10 == 1 || ability->Half_10 == 1)
            port_stat[port].spd = PORT_SPEED_10M;
        else if (ability->Full_100 == 1 || ability->Half_100 == 1)
            port_stat[port].spd = PORT_SPEED_100M;
        else
            port_stat[port].spd = PORT_SPEED_1000M;
            
        port_stat[port].FC       =   ability->FC;   
    }
    else
    {
        port_stat[port].AutoNego = ability->AutoNegotiation;
        port_stat[port].duplex   = PORT_HALF_DUPLEX;
        port_stat[port].spd      = PORT_SPEED_10M;
        port_stat[port].FC       =   ability->FC;
    }

    return RT_ERR_OK;
}

int32 rtk_web_cableStat_get(rtk_port_t port, rtk_cable_info_t* stat)
{
    if (port % 3 == 0)
    {
        stat->channel_a_status = CABLE_SHORT;
        stat->channel_a_len     = 10;
        stat->channel_b_status = CABLE_SHORT;
        stat->channel_b_len     = 10;
    }
    else if (port % 3 == 1)
    {
        stat->channel_a_status = CABLE_NORMAL;
        stat->channel_a_len     = 0;
        stat->channel_b_status = CABLE_NORMAL;
        stat->channel_b_len     = 0;
    }
    else
    {
        stat->channel_a_status = CABLE_OPEN;
        stat->channel_a_len     = 10;
        stat->channel_b_status = CABLE_OPEN;
        stat->channel_b_len     = 20;
    }

    return RT_ERR_OK;
}

int32 rtk_web_cableTest_start(void)
{
    return RT_ERR_OK;
}

int32 rtk_web_cableTest_stop(void)
{
    return RT_ERR_OK;
}
#endif

