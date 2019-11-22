/*
 *  Routines to handle host CPU related functions
 *
 *  $Id: 8192cd_host.c,v 1.1 2012/05/04 13:48:37 jimmylin Exp $
 *
 *  Copyright (c) 2012 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_HOST_C_

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
//#include <linux/in.h>
#include <linux/if.h>
#include <asm/io.h>
#include <linux/major.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>

#include "./8192cd_cfg.h"

#include <linux/syscalls.h>
#include <linux/file.h>
#include <asm/unistd.h>

#include "./8192cd.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"


#if !defined(CONFIG_NET_PCI) && defined(CONFIG_RTL8196C)
#include <asm/rtl865x/platform.h>
#endif

#if !defined(USE_RLX_BSP)
#include <platform.h>
#else
#include <bspchip.h>
#endif

#ifndef REG32
	#define REG32(reg)	 	(*(volatile unsigned int *)(reg))
#endif
#ifdef CONFIG_RTL_PCIE_LINK_PROTECTION
        int pcie_link_fail_counter=0;
	void  check_pcie_link_status(void)
	{

	}
#endif



#define MAX_PAYLOAD_SIZE_128B    0x00



#define MAX_PAYLOAD_SIZE_128B    0x00
#define MAX_PAYLOAD_SIZE_256B    0x01

#define CLK_MANAGE     0xb8000010
#define PCIE0_RC_EXT_BASE (0xb8b01000)
#define PCIE1_RC_EXT_BASE (0xb8b21000)
//RC Extended register
#define PCIE0_MDIO      (PCIE0_RC_EXT_BASE+0x00)
#define PCIE1_MDIO      (PCIE1_RC_EXT_BASE+0x00)
//MDIO
#define PCIE_MDIO_DATA_OFFSET (16)
#define PCIE_MDIO_DATA_MASK (0xffff <<PCIE_MDIO_DATA_OFFSET)
#define PCIE_MDIO_REG_OFFSET (8)
#define PCIE_MDIO_RDWR_OFFSET (0)

#if !defined(CONFIG_NET_PCI) && !defined(CONFIG_PCI)
#if (defined(__LINUX_2_6__) && defined(USE_RLX_BSP)) || defined(__ECOS)
#define	PCIE0_H_CFG	BSP_PCIE0_H_CFG
#define	PCIE1_H_CFG	BSP_PCIE1_H_CFG
#endif

#define GPIO_BASE			0xB8003500
#define PEFGHCNR_REG		(0x01C + GPIO_BASE)     /* Port EFGH control */
#define PEFGHPTYPE_REG		(0x020 + GPIO_BASE)     /* Port EFGH type */
#define PEFGHDIR_REG		(0x024 + GPIO_BASE)     /* Port EFGH direction */
#define PEFGHDAT_REG		(0x028 + GPIO_BASE)     /* Port EFGH data */


void HostPCIe_SetPhyMdioWrite(unsigned int portnum, unsigned int regaddr, unsigned short val)
{
        unsigned int mdioaddr;
        volatile int i;

        if(portnum==0)          mdioaddr=PCIE0_MDIO;
        else if(portnum==1)     mdioaddr=PCIE1_MDIO;
        else return;

        REG32(mdioaddr)= ( (regaddr&0x1f)<<PCIE_MDIO_REG_OFFSET) | ((val&0xffff)<<PCIE_MDIO_DATA_OFFSET)  | (1<<PCIE_MDIO_RDWR_OFFSET) ;
        //delay
        for(i=0;i<5555;i++)  ;
	delay_ms(1);
}


#if  defined(CONFIG_RTL_8196CS) || defined(CONFIG_RTL_8197B)
static void GPIO6_PCIE_Device_PERST(void)

{

#if defined(CONFIG_RTL_8197B)
        REG32(0xb8000040)=REG32(0xb8000040)|0x00018;

        REG32(0xb8003500)=REG32(0xb8003500)&(~0x10);

        REG32(0xb8003508)= REG32(0xb8003508)|0x10;

        REG32(0xb800350c)= REG32(0xb800350c)|0x10;

        delay_ms(500);

        delay_ms(500);



        // 6. PCIE Device Reset

        //REG32(CLK_MANAGE) &= ~(1<<26);    //perst=0 off.

        REG32(0xb800350c)= REG32(0xb800350c)&(~0x10);

        delay_ms(500);   //PCIE standadrd: poweron: 100us, after poweron: 100ms

        delay_ms(500);

        REG32(0xb800350c)= REG32(0xb800350c)|0x10;
#else
        REG32(0xb8000040)=REG32(0xb8000040)|0x300000;

        REG32(0xb8003500)=REG32(0xb8003500)&(~0x40);

        REG32(0xb8003508)= REG32(0xb8003508)|0x40;

        REG32(0xb800350c)= REG32(0xb800350c)|0x40;

        delay_ms(500);

        delay_ms(500);



        // 6. PCIE Device Reset

        //REG32(CLK_MANAGE) &= ~(1<<26);    //perst=0 off.

        REG32(0xb800350c)= REG32(0xb800350c)&(~0x40);

        delay_ms(500);   //PCIE standadrd: poweron: 100us, after poweron: 100ms

        delay_ms(500);

        REG32(0xb800350c)= REG32(0xb800350c)|0x40;

        //      REG32(CLK_MANAGE) |=  (1<<26);   //PERST=1
#endif
}
#endif


#ifdef CONFIG_RTL_8196C_iNIC //mark_inic
static void iNIC_PCIE_Device_PERST(void)

{

        REG32(0xb8000040)=REG32(0xb8000040)|0x300000;

        REG32(0xb8003500)=REG32(0xb8003500)&(~0x40);

        REG32(0xb8003508)= REG32(0xb8003508)|0x40;

        REG32(0xb800350c)= REG32(0xb800350c)|0x40;

        delay_ms(500);

        delay_ms(500);



        // 6. PCIE Device Reset

        //REG32(CLK_MANAGE) &= ~(1<<26);    //perst=0 off.

        REG32(0xb800350c)= REG32(0xb800350c)&(~0x40);

        delay_ms(500);   //PCIE standadrd: poweron: 100us, after poweron: 100ms

        delay_ms(500);

        REG32(0xb800350c)= REG32(0xb800350c)|0x40;

        //      REG32(CLK_MANAGE) |=  (1<<26);   //PERST=1

}
#endif

static int at2_mode=0;
static void PCIE_Device_PERST(int portnum)
{
	if (portnum==0)
	{
	_DEBUG_INFO("DeviceRESET\r\n");
	#if 1
		REG32(CLK_MANAGE) &= ~(1<<26);    //perst=0 off.
		delay_ms(500);   //PCIE standadrd: poweron: 100us, after poweron: 100ms
		delay_ms(500);
		REG32(CLK_MANAGE) |=  (1<<26);   //PERST=1
		#endif


	}
	else if (portnum==1)
	{
	/*	PCIE Device Reset
	*	The pcei1 slot reset register depends on the hw
	*/

#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN) || (RTL_USED_PCIE_SLOT==1) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)
#if defined(CONFIG_RTL_8197B)
                GPIO6_PCIE_Device_PERST();
#else
		REG32(PEFGHDAT_REG) &= ~(0x1000);    //perst=0 off.
		delay_ms(300);   //PCIE standadrd: poweron: 100us, after poweron: 100ms
		delay_ms(300);
		REG32(PEFGHDAT_REG) |=  (0x1000);   //PERST=1
#endif
#endif
	}
	else
		return;
}


void PCIE_MDIO_Reset(unsigned int portnum)
{
        #define SYS_PCIE_PHY0   (0xb8000000 +0x50)
        #define SYS_PCIE_PHY1   (0xb8000000 +0x54)

        unsigned int sys_pcie_phy;

        if(portnum==0)  sys_pcie_phy=SYS_PCIE_PHY0;
        else if(portnum==1)     sys_pcie_phy=SYS_PCIE_PHY1;
        else return;

       // 3.MDIO Reset
           REG32(sys_pcie_phy) = (1<<3) |(0<<1) | (0<<0);     //mdio reset=0,
           REG32(sys_pcie_phy) = (1<<3) |(0<<1) | (1<<0);     //mdio reset=1,
           REG32(sys_pcie_phy) = (1<<3) |(1<<1) | (1<<0);     //bit1 load_done=1
}

void PCIE_PHY_Reset(unsigned int portnum)
{
         #define PCIE_PHY0      0xb8b01008
         #define PCIE_PHY1      0xb8b21008

        unsigned int pcie_phy;

        if(portnum==0)  pcie_phy=BSP_PCIE0_H_PWRCR;
        else if(portnum==1)     pcie_phy=BSP_PCIE1_H_PWRCR;
        else return;

        //4. PCIE PHY Reset
	REG32(pcie_phy) = 0x01; //bit7:PHY reset=0   bit0: Enable LTSSM=1
	REG32(pcie_phy) = 0x81;   //bit7: PHY reset=1   bit0: Enable LTSSM=1
}


int PCIE_Check_Link(unsigned int portnum)
{
        unsigned int dbgaddr;
        unsigned int cfgaddr=0;
        int i=10;

        if(portnum==0)  dbgaddr=0xb8b00728;
        else if(portnum==1)     dbgaddr=0xb8b20728;
        else return 1;


  //wait for LinkUP

        while(--i)
        {
              if( (REG32(dbgaddr)&0x1f)==0x11)
                        break;

                delay_ms(100);
        }

        if(i==0)
        {       if(at2_mode==0)  //not auto test, show message
					panic_printk("i=%x  Cannot LinkUP \n",i);
                return 0;
        }
        else
        {
                if(portnum==0) cfgaddr=0xb8b10000;
                else if(portnum==1) cfgaddr=0xb8b30000;

                if(at2_mode==0)
					printk("Find Port=%x Device:Vender ID=%x\n", portnum, REG32(cfgaddr) );
        }
        return 1;
}

int PCIE_reset_procedure(int portnum, int Use_External_PCIE_CLK, int mdio_reset, unsigned long conf_addr)
{
	static int pcie_reset_done[RTL_MAX_PCIE_SLOT_NUM] = {0};
	int status=FAIL;
	int retry=2;


	#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN) || (RTL_USED_PCIE_SLOT==1) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)
	if (portnum==0)
	{
		//REG32(CLK_MANAGE) |=  (1<<26);   //PERST=1  //Modified for PERST initial value to origianl GPIO setting low level.
	}
	else if (portnum==1)
	{
#if 0
		  #if defined(CONFIG_RTL_8197B)
               REG32(0xb8000040)|=0x18;
		REG32(0xb8003500) &= ~(0x10);	/*port Abit 4 */
		REG32(0xb8003508) |= (0x10);	/*port A bit 4 */
		REG32(0xb800350c) |=  (0x10);   //PERST=1

          #else
		REG32(0xb8000040)|=0x300;
		REG32(PEFGHCNR_REG) &= ~(0x1000);	/*port F bit 4 */
		REG32(PEFGHDIR_REG) |= (0x1000);	/*port F bit 4 */
		REG32(PEFGHDAT_REG) |=  (0x1000);   //PERST=1
          #endif
#endif
	}
	#endif
	#ifdef CONFIG_RTL_8881A
	REG32(0xb8000010)= REG32(0xb8000010) |(1<<14);
	#endif

	if (pcie_reset_done[portnum])
		goto SET_BAR;
TRY_AGAIN:
	_DEBUG_INFO("PCIE reset (%d) \n", pcie_reset_done[portnum]);
	if(portnum==0)		    REG32(CLK_MANAGE) |=  (1<<14);        //enable active_pcie0
	else if(portnum==1)	    REG32(CLK_MANAGE) |=  (1<<16);        //enable active_pcie1
	else return 0;

	delay_ms(500);

	delay_ms(500);


	if(mdio_reset)
	{
		if(at2_mode==0)  //no auto test, show message
		_DEBUG_INFO("Do MDIO_RESET\n");

		// 3.MDIO Reset
		PCIE_MDIO_Reset(portnum);
	}

	/*
	PCIE_PHY_Reset(portnum);
	*/
	delay_ms(500);
	delay_ms(500);

	//----------------------------------------
	if(mdio_reset)
	{
		//fix 8198 test chip pcie tx problem.
	}

	//---------------------------------------
	PCIE_Device_PERST(portnum);
	PCIE_PHY_Reset(portnum);

#ifdef CONFIG_RTL_ULINKER
	{
		extern void eth_led_recover(void);
		eth_led_recover();
	}
#endif

	delay_ms(500);
	delay_ms(500);
	status=PCIE_Check_Link(portnum);
	if(status==FAIL)
	{
		retry--;
		if(retry==0)
		return FAIL;
		else
		goto TRY_AGAIN;
	}

SET_BAR:
	if (portnum==0)
	{
		// Enable PCIE host
		if (pcie_reset_done[portnum] == 0) {
#if (defined(__LINUX_2_6__) && defined(USE_RLX_BSP)) || defined(__ECOS)
		WRITE_MEM32(BSP_PCIE0_H_CFG + 0x04, 0x00100007);
        WRITE_MEM8(BSP_PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#else
			WRITE_MEM32(PCIE0_H_CFG + 0x04, 0x00100007);
			WRITE_MEM8(PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#endif
			pcie_reset_done[portnum] = 1;
		}
	}
	else if (portnum==1)
	{
		// Enable PCIE host
#if (defined(__LINUX_2_6__) && defined(USE_RLX_BSP))  || defined(__ECOS)
		if (pcie_reset_done[portnum] == 0) {
			WRITE_MEM32(BSP_PCIE1_H_CFG + 0x04, 0x00100007);
            WRITE_MEM8(BSP_PCIE1_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
			pcie_reset_done[portnum] = 1;
		}
		#else	/*	defined(__LINUX_2_6__) && defined(USE_RLX_BSP)	*/
			WRITE_MEM32(PCIE1_H_CFG + 0x04, 0x00100007);
			WRITE_MEM8(PCIE1_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
		#endif	/*	defined(__LINUX_2_6__) && defined(USE_RLX_BSP)	*/
	}
	else
		return FAIL;

	delay_ms(500);
	return SUCCESS;

}

#endif	/*	!defined(CONFIG_NET_PCI)	*/



