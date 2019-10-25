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

#ifdef __KERNEL__
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
#endif

#include "./8192cd_cfg.h"

#ifdef __KERNEL__
#ifdef __LINUX_2_6__
#include <linux/syscalls.h>
#include <linux/file.h>
#include <asm/unistd.h>
#endif
#elif defined(__ECOS)
#include <cyg/hal/plf_intr.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#else
#include "./sys-support.h"
#endif

#include "./8192cd.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"

#ifdef __OSK__
	#ifdef CONFIG_RLE0412
	#include "asicregs.h"
	#include "gpio.h"
	#endif
#elif defined(CONFIG_RTL8672)
	#ifdef USE_RLX_BSP
		#include <bspchip.h>
		#ifdef __LINUX_3_10__
		#include <bspgpio.h>
		#else
		#include <gpio.h>
		#endif
		#ifdef CONFIG_RTL_8196C
		#undef CONFIG_RTL_8196C
		#endif
		#ifdef CONFIG_RTL8196C_REVISION_B
		#undef CONFIG_RTL8196C_REVISION_B
		#endif
	#else
		#include <platform.h>
		#include "../../../arch/mips/realtek/rtl8672/gpio.h"
	#endif
#elif defined(__ECOS)
#else
#if !defined(CONFIG_NET_PCI) && defined(CONFIG_RTL8196B)
#include <asm/rtl865x/platform.h>
#endif

#if !defined(CONFIG_NET_PCI) && defined(CONFIG_RTL8196C)
#include <asm/rtl865x/platform.h>
#endif
#endif

#if defined(CONFIG_RTL_819X) && defined(__LINUX_2_6__)
#if !defined(USE_RLX_BSP)
#include <platform.h>
#else
#if defined(CONFIG_OPENWRT_SDK) && !defined(CONFIG_ARCH_CPU_RLX) || defined(CONFIG_RTL_8197F)
#include <bspchip.h>
#else
#include <bsp/bspchip.h>
#endif	//CONFIG_OPENWRT_SDK
#endif
#endif

#ifndef REG32
	#define REG32(reg)	 	(*(volatile unsigned int *)(reg))
#endif
#ifdef CONFIG_RTL_PCIE_LINK_PROTECTION
        int pcie_link_fail_counter=0;
	void  check_pcie_link_status(void)
	{
		#if defined(CONFIG_WLAN_HAL_8814AE) && defined(CONFIG_USE_PCIE_SLOT_1)
			if( (REG32(0xb8b20728) &0x1f) != 0x11)
			{
				pcie_link_fail_counter++;
				if(pcie_link_fail_counter > 10)
				{
					panic_printk("!!Reboot by PCIE out link (8814)\r\n");
					REG32(BSP_WDTCNR) = 0;

				}
				else
				{
					pcie_link_fail_counter=0;
				}
			}
		#endif
		
	}
#endif


#ifdef CONFIG_RTL_92D_DMDP
void Sw_PCIE_Func(int func)
{
#if (RTL_USED_PCIE_SLOT==1)
	REG32(0xb8b2100c)=REG32(0xb8b2100c)|func; // switch to function #
#else
	REG32(0xb8b0100c)=REG32(0xb8b0100c)|func; // switch to function #
#endif
}
#endif

#if !defined(CONFIG_NET_PCI) && (defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X))
#define MAX_PAYLOAD_SIZE_128B    0x00

#ifndef __ECOS
#if !(defined(CONFIG_RTL8196C) || defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819X))
int rtl8196b_pci_reset(unsigned long conf_addr)
{
   /* If PCI needs to be reset, put code here.
    * Note:
    *    Software may need to do hot reset for a period of time, say ~100us.
    *    Here we put 2ms.
    */
	//Modified for PCIE PHY parameter due to RD center suggestion by Jason 12252009
	WRITE_MEM32(0xb8000044, 0x9);//Enable PCIE PLL
	delay_ms(10);
	REG32(0xb8000010)=REG32(0xb8000010)|(0x500); //Active LX & PCIE Clock in 8196B system register
	delay_ms(10);
	WRITE_MEM32(0xb800003C, 0x1);//PORT0 PCIE PHY MDIO Reset
	delay_ms(10);
	WRITE_MEM32(0xb800003C, 0x3);//PORT0 PCIE PHY MDIO Reset
	delay_ms(10);
	WRITE_MEM32(0xb8000040, 0x1);//PORT1 PCIE PHY MDIO Reset
	delay_ms(10);
	WRITE_MEM32(0xb8000040, 0x3);//PORT1 PCIE PHY MDIO Reset
	delay_ms(10);
	WRITE_MEM32(0xb8b01008, 0x1);// PCIE PHY Reset Close:Port 0
	delay_ms(10);
	WRITE_MEM32(0xb8b01008, 0x81);// PCIE PHY Reset On:Port 0
	delay_ms(10);
#ifdef PIN_208
	WRITE_MEM32(0xb8b21008, 0x1);// PCIE PHY Reset Close:Port 1
	delay_ms(10);
	WRITE_MEM32(0xb8b21008, 0x81);// PCIE PHY Reset On:Port 1
	delay_ms(10);
#endif
#ifdef OUT_CYSTALL
	WRITE_MEM32(0xb8b01000, 0xcc011901);// PCIE PHY Reset On:Port 0
	delay_ms(10);
#ifdef PIN_208
	WRITE_MEM32(0xb8b21000, 0xcc011901);// PCIE PHY Reset On:Port 1
	delay_ms(10);
#endif
#endif
	REG32(0xb8000010)=REG32(0xb8000010)|(0x01000000); //PCIE PHY Reset On:Port 0
	delay_ms(10);

#if (defined(__LINUX_2_6__) && defined(USE_RLX_BSP)) || defined(__ECOS)
   WRITE_MEM32(BSP_PCIE0_H_PWRCR, READ_MEM32(BSP_PCIE0_H_PWRCR) & 0xFFFFFF7F);
#ifdef PIN_208
   WRITE_MEM32(BSP_PCIE1_H_PWRCR, READ_MEM32(BSP_PCIE1_H_PWRCR) & 0xFFFFFF7F);
#endif
   delay_ms(100);
   WRITE_MEM32(BSP_PCIE0_H_PWRCR, READ_MEM32(BSP_PCIE0_H_PWRCR) | 0x00000080);
#ifdef PIN_208
   WRITE_MEM32(BSP_PCIE1_H_PWRCR, READ_MEM32(BSP_PCIE1_H_PWRCR) | 0x00000080);
#endif
#else
   WRITE_MEM32(PCIE0_H_PWRCR, READ_MEM32(PCIE0_H_PWRCR) & 0xFFFFFF7F);
#ifdef PIN_208
   WRITE_MEM32(PCIE1_H_PWRCR, READ_MEM32(PCIE1_H_PWRCR) & 0xFFFFFF7F);
#endif
   delay_ms(100);
   WRITE_MEM32(PCIE0_H_PWRCR, READ_MEM32(PCIE0_H_PWRCR) | 0x00000080);
#ifdef PIN_208
   WRITE_MEM32(PCIE1_H_PWRCR, READ_MEM32(PCIE1_H_PWRCR) | 0x00000080);
#endif
#endif

   delay_ms(10);

	if ((READ_MEM32(0xb8b00728)&0x1f)!=0x11)
	{
		_DEBUG_INFO("PCIE LINK FAIL\n");
		return FAIL;
	}

   // Enable PCIE host
#if (defined(__LINUX_2_6__) && defined(USE_RLX_BSP)) || defined(__ECOS)
	WRITE_MEM32(BSP_PCIE0_H_CFG + 0x04, 0x00100007);
	WRITE_MEM8(BSP_PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#else
	WRITE_MEM32(PCIE0_H_CFG + 0x04, 0x00100007);
	WRITE_MEM8(PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#endif
	return SUCCESS;
}
#endif // !defined(CONFIG_NET_PCI) && (defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X))
#endif
#endif


#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL8196C) || defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198B) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define MAX_PAYLOAD_SIZE_128B    0x00
#define MAX_PAYLOAD_SIZE_256B    0x01

#ifndef CONFIG_RTL_8198B
#define CLK_MANAGE     0xb8000010
#endif
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
#if !defined(CONFIG_NET_PCI) &&  (defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E))
#define PHY_EAT_40MHZ 1
#endif

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

#if defined(CONFIG_RTL_8198) || ((defined(CONFIG_RTL_819XD) && !defined(CONFIG_RTL8672)) || (defined(CONFIG_RTL_819X) && defined(__OSK__))) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198B) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
static int at2_mode=0;
#ifdef CONFIG_RTL_8198B
void PCIE_Device_PERST(int portnum)
{
	if (portnum==0) {
		//gpio67
		REG32(0xBB000120) |= (1<<2); //EN_GPIO
		REG32(0xBB0001DC) |= (1<<2); //SEL_GPIO select GPO
		REG32(0xBB000108) |= (1<<2); //CTRL_GPIO output 1
		mdelay(500);
		mdelay(500);
		REG32(0xBB000108) &= ~(1<<2); //CTRL_GPIO output 0
		mdelay(500);
		mdelay(500);
		REG32(0xBB000108) |= (1<<2); //CTRL_GPIO output 1
	}
	else if (portnum==1) {
		//gpio69
		REG32(0xBB000120) |= (1<<4); //EN_GPIO
		REG32(0xBB0001DC) |= (1<<4); //SEL_GPIO select GPO
		REG32(0xBB000108) |= (1<<4); //CTRL_GPIO output 1
		mdelay(500);
		mdelay(500);
		REG32(0xBB000108) &= ~(1<<4); //CTRL_GPIO output 0
		mdelay(500);
		mdelay(500);
		REG32(0xBB000108) |= (1<<4); //CTRL_GPIO output 1
	}
	else
		return;
}
#else
static void PCIE_Device_PERST(int portnum)
{
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	REG32(0xb8000010)|= (1<<12)|(1<<13)|(1<<19)|(1<<20)|(1<<18)|(1<<16);
#endif
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
#if defined(CONFIG_RTL_8198C)
		_DEBUG_INFO("Port 1 DevRESET\r\n");
#ifndef CONFIG_RTL_89xxE
                REG32(0xb800010c)= (REG32(0xb800010c)&(~(7<<10)))|(3<<10);
                delay_ms(500);
                REG32(PEFGHCNR_REG) &= ~(0x20000);               /*port F bit 4 */
                REG32(PEFGHDIR_REG) |= (0x20000);                /*port F bit 4 */
                REG32(PEFGHDAT_REG) &= ~(0x20000);
                        delay_ms(500);
                        delay_ms(500);
                REG32(PEFGHDAT_REG) |=  (0x20000);   //PERST=1
#else
                REG32(0xb8000110)= (REG32(0xb8000110)&(~(3<<18)))|(3<<18);
                delay_ms(500);
                REG32(PEFGHCNR_REG) &= ~(0x1);               /*port F bit 4 */
                REG32(PEFGHDIR_REG) |= (0x1);                /*port F bit 4 */
                REG32(PEFGHDAT_REG) &= ~(0x1);
                        delay_ms(500);
                        delay_ms(500);
                REG32(PEFGHDAT_REG) |=  (0x1);   //PERST=1
#endif

#else
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)

		REG32(0xb8000040) = (REG32(0xb8000040) & ~(7)) |(6);
							
                REG32(0xb8003500)=REG32(0xb8003500)&(~(1<<2));

                 REG32(0xb8003508)= REG32(0xb8003508)|(1<<2);




                REG32(0xb800350C) &= ~(1<<2);    //perst=0 off.
                delay_ms(300);   //PCIE standadrd: poweron: 100us, after poweron: 100ms
                delay_ms(300);

                REG32(0xb800350C) |=  (1<<2);   //PERST=1



		
#if defined(QA_BOARD)
		REG32(0xb8000040)=REG32(0xb8000040)|0x30000000;

		REG32(0xb8003500)=REG32(0xb8003500)&(~0x200000);
 
		 REG32(0xb8003508)= REG32(0xb8003508)|0x200000;




		REG32(0xb800350C) &= ~(0x200000);    //perst=0 off.  
		delay_ms(300);   //PCIE standadrd: poweron: 100us, after poweron: 100ms
		delay_ms(300); 

		REG32(0xb800350C) |=  (0x200000);   //PERST=1
#endif
#else
		REG32(PEFGHDAT_REG) &= ~(0x1000);    //perst=0 off.  
		delay_ms(300);   //PCIE standadrd: poweron: 100us, after poweron: 100ms
		delay_ms(300); 
		REG32(PEFGHDAT_REG) |=  (0x1000);   //PERST=1
#endif
#endif
#endif
#elif defined(CONFIG_RTL_92D_SUPPORT)
		REG32(CLK_MANAGE) &= ~(1<<26);    //perst=0 off.    
		delay_ms(500);   //PCIE standadrd: poweron: 100us, after poweron: 100ms
		delay_ms(500);  		
		REG32(CLK_MANAGE) |=  (1<<26);   //PERST=1
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
#endif

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

#ifdef CONFIG_RTL_8198B
#define IP_Enable_Control_Register  0xb8000600 //page74
#define PCIE_Control_Register  0xb8000504 //page7

int PCIE_reset_procedure(int portnum, int Use_External_PCIE_CLK, int mdio_reset, unsigned long conf_addr)
{
	static int pcie_reset_done[RTL_MAX_PCIE_SLOT_NUM] = {0};
	int status=FAIL;
	
	if (pcie_reset_done[portnum]) 
	{
		_DEBUG_INFO("PCIE reset Already (%d) \n", pcie_reset_done[portnum]);
		return SUCCESS;	
	}

	_DEBUG_INFO("PCIE  %d reset (%d) \n", portnum,pcie_reset_done[portnum]);

	 if(portnum==0)
 		 REG32(IP_Enable_Control_Register) |= (1<<7) ; //Enable PCIE Port0 IP clock 
	 else if(portnum==1)     REG32(IP_Enable_Control_Register) |=  (1<<6);        //Enable PCIE Port1 IP clock 
 	 else return; 

      delay_ms(500);	

	 if(mdio_reset)
 	{
 		
  		 _DEBUG_INFO("Do MDIO_RESET\n");  
 
 		if(portnum==0)      REG32(PCIE_Control_Register) |=  (1<<24);        //Reset EPHY0
 		else if(portnum==1)     REG32(PCIE_Control_Register) |=  (1<<21);        //Reset EPHY1 
 		else return; 
  
 	}  
 
    delay_ms(500);
  //add compatible, slove sata pcie card.

	if((mdio_reset)&&(portnum==0))
    {
 
  		_DEBUG_INFO("\nSet PCIE P0 EPHY\n");
  		HostPCIe_SetPhyMdioWrite(portnum, 6, 0xf848);  //25MHZ
  		HostPCIe_SetPhyMdioWrite(portnum, 0x0b, 0x0711);   //for sloving low performance
  		HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0a00);  
  		HostPCIe_SetPhyMdioWrite(portnum, 0xd, 0x1766);   
  		HostPCIe_SetPhyMdioWrite(portnum, 0x1d, 0xa0eb); 
    }
 
    if((mdio_reset)&&(portnum==1))
    {
   		 _DEBUG_INFO("\nSet PCIE P1 EPHY\n");
  		HostPCIe_SetPhyMdioWrite(portnum, 4, 0xf56b);  //25MHZ
  		HostPCIe_SetPhyMdioWrite(portnum, 0x6, 0x5881);   //for sloving low performance
 		 HostPCIe_SetPhyMdioWrite(portnum, 0x8, 0x6c1c);  
  		HostPCIe_SetPhyMdioWrite(portnum, 0x9, 0x0fbf);   
  		HostPCIe_SetPhyMdioWrite(portnum, 0xb, 0x7bb0); 
    }

	// Step 4. PCIE Device Reset
	PCIE_Device_PERST(portnum);

	PCIE_PHY_Reset(portnum);
	
     delay_ms(500);	
  
	 _DEBUG_INFO("\nPCIE_Check_Link\n");
    status=PCIE_Check_Link(portnum); 

   if(status==FAIL)
		return FAIL;

#ifdef __OSK__
	//io and mem limit, setting to no litmit
    REG32(0xb8b0001c) = (2<<4) | (0<<12);   //  [7:4]=base  [15:12]=limit
    REG32(0xb8b00020) = (2<<4) | (0<<20);   //  [15:4]=base  [31:20]=limit 
    REG32(0xb8b00024) = (2<<4) | (0<<20);   //  [15:4]=base  [31:20]=limit 
    //printk("reset PCIE 2: b8b0001c:0x%08x, b8b00020:0x%08x, b8b00024:0x%08x\n", REG32(0xb8b0001c), REG32(0xb8b00020), REG32(0xb8b00024));
#endif
		
  //set BAR

   if (portnum==0)
	{
		// Enable PCIE host
		if (pcie_reset_done[portnum] == 0) {
#if defined(__LINUX_2_6__) && defined(USE_RLX_BSP)
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
#if defined(__LINUX_2_6__) && defined(USE_RLX_BSP)
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
#else
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
#elif defined(CONFIG_RTL_8198)
		REG32(0xb8000040)|=0x300;
		REG32(PEFGHCNR_REG) &= ~(0x1000);	/*port F bit 4 */
		REG32(PEFGHDIR_REG) |= (0x1000);	/*port F bit 4 */
		REG32(PEFGHDAT_REG) |=  (0x1000);   //PERST=1		  
#endif
	}
	#endif
	#ifdef CONFIG_RTL_8881A
	REG32(0xb8000010)= REG32(0xb8000010) |(1<<14);
	#endif
	#ifdef CONFIG_RTL_8198C
	REG32(0xb8000010)= REG32(0xb8000010) |(1<<12|1<<13|1<<14|1<<16|1<<19|1<<20);//(1<<14);
	#endif

	if (pcie_reset_done[portnum]) 
		goto SET_BAR;
TRY_AGAIN:
	_DEBUG_INFO("PCIE reset (%d) \n", pcie_reset_done[portnum]);
	if(portnum==0)		    REG32(CLK_MANAGE) |=  (1<<14);        //enable active_pcie0
	else if(portnum==1)	    REG32(CLK_MANAGE) |=  (1<<16);        //enable active_pcie1	
	else return 0;

	delay_ms(500);

	#ifdef CONFIG_RTL8198_REVISION_B
	if(portnum==1)
	{
		#define PAD_CONTROL 0xb8000048
		REG32(PAD_CONTROL)|=(1<<27);
	}	
	#endif 
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
		#if defined(CONFIG_RTL8198_REVISION_B) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) ||  defined(CONFIG_RTL_8198C)
		#if defined(CONFIG_RTL_8198C)
		
		REG32(0xb8000104)=(REG32(0xb8000104)&(~(0x3<<20)))|(1<<20);  //PCIe MUX switch to PCIe reset
#if 0
		#ifdef  CONFIG_PHY_EAT_40MHZ
		delay_ms(500);
		 panic_printk("\r\nThiw WAY\r\n");
		HostPCIe_SetPhyMdioWrite(portnum, 0x3, 0x7b01);
        HostPCIe_SetPhyMdioWrite(portnum, 0x6, 0xe2b8);// e2b8
        HostPCIe_SetPhyMdioWrite(portnum, 0xF, 0x400F);
        HostPCIe_SetPhyMdioWrite(portnum, 0xd, 0x1764);// e2b8
        //HostPCIe_SetPhyMdioWrite(portnum, 0x9, 0x930c);
        HostPCIe_SetPhyMdioWrite(portnum, 0x19, 0xFC70);

        panic_printk("\r\n40MHz PCIe Parameters\r\n");
        #else

        HostPCIe_SetPhyMdioWrite(portnum, 0x3, 0x3001);
        HostPCIe_SetPhyMdioWrite(portnum, 0x6, 0xe0b8); //Hannah
        
        HostPCIe_SetPhyMdioWrite(portnum, 0xF, 0x400F);
        HostPCIe_SetPhyMdioWrite(portnum, 0x19, 0xFC70);
		printk("\r\n25MHz PCIe Parameters\r\n");
		#endif
#else
	{
	        int phy40M;
	        phy40M=(REG32(0xb8000008)&(1<<24))>>24;
        	printk("UPHY: 8198c ASIC u2 of u3 %s phy patch\n", (phy40M==1) ? "40M" : "25M");
		if(phy40M)
		{
			delay_ms(500);
			HostPCIe_SetPhyMdioWrite(portnum, 0x3, 0x7b31);
			HostPCIe_SetPhyMdioWrite(portnum, 0x6, 0xe258);// e2b8
			HostPCIe_SetPhyMdioWrite(portnum, 0xF, 0x400F);
			HostPCIe_SetPhyMdioWrite(portnum, 0xd, 0x1764);// e2b8
			HostPCIe_SetPhyMdioWrite(portnum, 0x19, 0xFC70);

			_DEBUG_TRACE("\r\n40MHz PCIe Parameters\r\n");
		}
		else
		{
			HostPCIe_SetPhyMdioWrite(portnum, 0x3, 0x3031);
			HostPCIe_SetPhyMdioWrite(portnum, 0x6, 0xe058); //Hannah
			HostPCIe_SetPhyMdioWrite(portnum, 0xF, 0x400F);
			HostPCIe_SetPhyMdioWrite(portnum, 0x19, 0xFC70);
			_DEBUG_TRACE("\r\n25MHz PCIe Parameters\r\n");

			
		}
		#if defined(CONFIG_RTL_8198C) & defined(CONFIG_WLAN_HAL_8814AE)
		if(portnum ==1)
		{
			HostPCIe_SetPhyMdioWrite(portnum, 0x0e, 0x1f45);
		}
		#endif
	}
#endif		




		#else
#ifdef CONFIG_RTL_8881A

		REG32(0xb8000050)=0xa;
		delay_ms(500);

		 REG32(0xb8000050)=0xb;

			HostPCIe_SetPhyMdioWrite(portnum, 0xb, 0x7cff);  //bokai tell, and fix
			HostPCIe_SetPhyMdioWrite(portnum, 0xb, 0x7bff);  //bokai tell, and fix
			//HostPCIe_SetPhyMdioWrite(portnum, 0xb, 0x7bff);  //bokai tell, and fix

		    if(REG32(0xb8000000)&0xf>=0x01)
			{
			#ifdef CONFIG_RTL_88E_SUPPORT
            //HostPCIe_SetPhyMdioWrite(portnum, 0xa, 0x4437);
            //HostPCIe_SetPhyMdioWrite(portnum, 0x1, 0x36a3);
            #endif

			  HostPCIe_SetPhyMdioWrite(portnum, 0x3, 0x0486);
			  HostPCIe_SetPhyMdioWrite(portnum, 0x4, 0xf144);
			  HostPCIe_SetPhyMdioWrite(portnum, 0xc, 0x097f);
			}
	        else
			{
				HostPCIe_SetPhyMdioWrite(portnum, 0xc, 0x0171);
				HostPCIe_SetPhyMdioWrite(portnum, 0x4, 0xf544);
			}
   
            HostPCIe_SetPhyMdioWrite(portnum, 0x6, 0x7081);
            HostPCIe_SetPhyMdioWrite(portnum, 0x8, 0x901c);
            HostPCIe_SetPhyMdioWrite(portnum, 0x9, 0x0ebc);
#else
		#if defined(CONFIG_RTL_8196E)
		if ((REG32(BSP_REVR) & 0xFFFFF000) == BSP_RTL8196E)
		#else		
		if ((REG32(BSP_REVR) >= BSP_RTL8198_REVISION_B) || ((REG32(BSP_REVR) & 0xFFFFF000) == BSP_RTL8197D)) 
		#endif
		{
#if defined(CONFIG_RTL_88E_SUPPORT) || (defined(SLOT_0_92C) && defined(CONFIG_SLOT_0_EXT_PA)) || (defined(SLOT_1_92C) && defined(CONFIG_SLOT_1_EXT_PA))
			HostPCIe_SetPhyMdioWrite(portnum, 0, 0xD086);  //bokai tell, and fix
#else
			HostPCIe_SetPhyMdioWrite(portnum, 0, 0xD087);  //bokai tell, and fix
#endif
			HostPCIe_SetPhyMdioWrite(portnum, 1, 0x0003);
			HostPCIe_SetPhyMdioWrite(portnum, 2, 0x4d19);   ///1119 bokai
			#ifdef CONFIG_WLAN_HAL_8192EE
			HostPCIe_SetPhyMdioWrite(portnum, 4, 0x5000);	// disable spread spectrum in 92E
			#else
#if defined(CONFIG_RTL_88E_SUPPORT) || (defined(SLOT_0_92C) && defined(CONFIG_SLOT_0_EXT_PA)) || (defined(SLOT_1_92C) && defined(CONFIG_SLOT_1_EXT_PA))			
			#ifdef  CONFIG_PHY_EAT_40MHZ
			HostPCIe_SetPhyMdioWrite(portnum, 4, 0x7C00);
			#else			
			HostPCIe_SetPhyMdioWrite(portnum, 4, 0x7000);
			#endif				
#endif			
			#endif
			#if defined(CONFIG_AUTO_PCIE_PHY_SCAN) && (defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD))
			
				if ((REG32(0xb8000008)&0x2000000)==0x2000000)  //40MHz
				{
					printk("98 - 40MHz Clock Source\n");
#if defined(CONFIG_RTL_88E_SUPPORT) || (defined(SLOT_0_92C) && defined(CONFIG_SLOT_0_EXT_PA)) || (defined(SLOT_1_92C) && defined(CONFIG_SLOT_1_EXT_PA))					
					HostPCIe_SetPhyMdioWrite(portnum, 4, 0x7C00);
					HostPCIe_SetPhyMdioWrite(portnum, 5, 0x09DB);   //40M  //1119 bokai
					HostPCIe_SetPhyMdioWrite(portnum, 6, 0x4048);   //40M
#else
					HostPCIe_SetPhyMdioWrite(portnum, 5, 0x0BCB);   //40M
					HostPCIe_SetPhyMdioWrite(portnum, 6, 0x2148);   //40M
#endif					
					HostPCIe_SetPhyMdioWrite(portnum, 7, 0x41ff);
					HostPCIe_SetPhyMdioWrite(portnum, 8, 0x13F6); 
				}
				else //25MHz
				{
					printk("98 - 25MHz Clock Source\n");
#if defined(CONFIG_RTL_88E_SUPPORT) || (defined(SLOT_0_92C) && defined(CONFIG_SLOT_0_EXT_PA)) || (defined(SLOT_1_92C) && defined(CONFIG_SLOT_1_EXT_PA))					
					HostPCIe_SetPhyMdioWrite(portnum, 4, 0x7000);
					HostPCIe_SetPhyMdioWrite(portnum, 5, 0x0B13); 
					HostPCIe_SetPhyMdioWrite(portnum, 6, 0xf048);  //25M
#else
					HostPCIe_SetPhyMdioWrite(portnum, 6, 0xf848);  //25M
#endif					
					HostPCIe_SetPhyMdioWrite(portnum, 7, 0xa7ff);  //-0.5%
					HostPCIe_SetPhyMdioWrite(portnum, 8, 0x0c56);  
				
			}
			#else

			#ifdef  CONFIG_PHY_EAT_40MHZ
			printk("40MHz Clock Source\n");
#if defined(CONFIG_RTL_88E_SUPPORT) || (defined(SLOT_0_92C) && defined(CONFIG_SLOT_0_EXT_PA)) || (defined(SLOT_1_92C) && defined(CONFIG_SLOT_1_EXT_PA))								
			HostPCIe_SetPhyMdioWrite(portnum, 5, 0x09DB);   //40M  //1119 bokai
			HostPCIe_SetPhyMdioWrite(portnum, 6, 0xf048);   //40M
#else
			HostPCIe_SetPhyMdioWrite(portnum, 5, 0x0BCB);   //40M
			HostPCIe_SetPhyMdioWrite(portnum, 6, 0x2148);   //40M
#endif
			//HostPCIe_SetPhyMdioWrite(portnum, 5, 0x08db);   //40M
			//HostPCIe_SetPhyMdioWrite(portnum, 6, 0xF148);  //40M
			#else
			printk("25MHz Clock Source\n");
#if defined(CONFIG_RTL_88E_SUPPORT) || (defined(SLOT_0_92C) && defined(CONFIG_SLOT_0_EXT_PA)) || (defined(SLOT_1_92C) && defined(CONFIG_SLOT_1_EXT_PA))								
			HostPCIe_SetPhyMdioWrite(portnum, 5, 0x0B13); 
			HostPCIe_SetPhyMdioWrite(portnum, 6, 0xf048);  //25M
#else
			HostPCIe_SetPhyMdioWrite(portnum, 6, 0xf848);  //25M
#endif
			#endif
			#endif
			
			
				
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	#else
			HostPCIe_SetPhyMdioWrite(portnum, 8, 0x18d7);  //peisi tune
#endif
			//saving more power, 8196c pe-si tune
			HostPCIe_SetPhyMdioWrite(portnum, 0x09, 0x539c); 	
			HostPCIe_SetPhyMdioWrite(portnum, 0x0a, 0x20eb); 	
			HostPCIe_SetPhyMdioWrite(portnum, 0x0d, 0x1766); 			
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)

			#ifdef CONFIG_RTL_8881A
			HostPCIe_SetPhyMdioWrite(portnum, 0x0b, 0x0311);   //for sloving low performance
			#else
			HostPCIe_SetPhyMdioWrite(portnum, 0x0b, 0x0711);   //for sloving low performance
			#endif
#else
			HostPCIe_SetPhyMdioWrite(portnum, 0x0b, 0x0511);   //for sloving low performance
#endif

#if defined(CONFIG_RTL_8196E)
			HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0f0f);
#else
			HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0a00);
#endif
			HostPCIe_SetPhyMdioWrite(portnum, 0x19, 0xFCE0);

			HostPCIe_SetPhyMdioWrite(portnum, 0x1a, 0x7e40);   //formal chip, reg 0x1a.4=0
			HostPCIe_SetPhyMdioWrite(portnum, 0x1b, 0xFC01);   //formal chip	 reg 0x1b.0=1		

			HostPCIe_SetPhyMdioWrite(portnum, 0x1e, 0xC280);	
		}
		else
		#endif
		{
		#ifndef CONFIG_RTL_8881A
			HostPCIe_SetPhyMdioWrite(portnum, 0, 0xD087);

			HostPCIe_SetPhyMdioWrite(portnum, 1, 0x0003);
			HostPCIe_SetPhyMdioWrite(portnum, 6, 0xf448); //new
			HostPCIe_SetPhyMdioWrite(portnum, 6, 0x408); 	//avoid noise infuse	 //15-12=0, 7-5=0,    0448

			HostPCIe_SetPhyMdioWrite(portnum, 7, 0x31ff);
			HostPCIe_SetPhyMdioWrite(portnum, 8, 0x18d5);  //new		
			HostPCIe_SetPhyMdioWrite(portnum, 9, 0x531c); 		
		
			HostPCIe_SetPhyMdioWrite(portnum, 0xd, 0x1766); 
			HostPCIe_SetPhyMdioWrite(portnum, 0xf, 0x0010);//ori				

			HostPCIe_SetPhyMdioWrite(portnum, 0x19, 0xFCE0); 
			HostPCIe_SetPhyMdioWrite(portnum, 0x1e, 0xC280);	
		#endif
		}
	#endif
	#endif
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
#if defined(CONFIG_RTL_8198C) & (defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE))
        WRITE_MEM8(BSP_PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_256B);  // Set MAX_PAYLOAD_SIZE to 256B
#else
        WRITE_MEM8(BSP_PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#endif
#else
			WRITE_MEM32(PCIE0_H_CFG + 0x04, 0x00100007);
			WRITE_MEM8(PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#endif
			pcie_reset_done[portnum] = 1;
		}
#ifdef CONFIG_RTL_92D_DMDP		
		else {
			Sw_PCIE_Func(1);
			//choose the PCIE port number 
			WRITE_MEM32(PCIE0_H_CFG + 0x04, 0x00100007);
			WRITE_MEM8(PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
			Sw_PCIE_Func(0);
		}
#endif
	}
	else if (portnum==1)
	{
		// Enable PCIE host
#if (defined(__LINUX_2_6__) && defined(USE_RLX_BSP))  || defined(__ECOS)
		if (pcie_reset_done[portnum] == 0) {
			WRITE_MEM32(BSP_PCIE1_H_CFG + 0x04, 0x00100007);
#if defined(CONFIG_RTL_8198C) & (defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE))
            WRITE_MEM8(BSP_PCIE1_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_256B);  // Set MAX_PAYLOAD_SIZE to 256B
#else
            WRITE_MEM8(BSP_PCIE1_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#endif       
			pcie_reset_done[portnum] = 1;
		}
		#ifdef CONFIG_RTL_92D_DMDP
		else {
			Sw_PCIE_Func(1);
			//choose the PCIE port number 
			WRITE_MEM32(BSP_PCIE1_H_CFG + 0x04, 0x00100007);
			WRITE_MEM8(BSP_PCIE1_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
			Sw_PCIE_Func(0);
		}
		#endif
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
#endif
#elif defined(CONFIG_RTL8196C) || defined(CONFIG_RTL_8196C)


static int pcie_reset_done[2] = {0};
#ifdef CONFIG_RTL8672

#if defined(__OSK__)
#define BSP_IP_SEL			(MISC_IPSEL)
#endif

#if defined(USE_RLX_BSP) || defined(__OSK__)
#define PCI_MISC			BSP_PCI_MISC
#define MISC_IP_SEL		BSP_IP_SEL
#define EN_PCIE			BSP_EN_PCIE
#define PCI_MISC			BSP_PCI_MISC
#define PCIE0_H_PWRCR	BSP_PCIE0_H_PWRCR
#ifndef __OSK__
#define MISC_PINSR		BSP_MISC_PINSR
#endif
#define CLKSEL			BSP_CLKSEL
#define PCIE0_H_CFG		BSP_PCIE0_H_CFG
#endif //USE_RLX_BSP

unsigned char clk_src_40M = 0;

struct pcie_para{
	unsigned char port;
	unsigned char reg;	
	unsigned short value;
};

enum clk_source{
	CLK35_328_6166 = 0,		//6166 35.328M clk
	CLK25_6166,				//6166 25M clk
	CLK35_328_8676,		//8676 35.328M clk
	CLK40_8676,				//8676 40M clk
	CLK40_8686,				//8686 40M clk
	CLK25_8686,                      //8686 25M clk
	CLK35_328_8676S,		//8676S 35.328M clk
	CLK40_8676S,			//8676S 40M clk
	CLK35_328_0562,			//0562 35.328M clk
	CLK40_0562,				//0562 40M clk
	CLK25_8685_P0, 			//8685 25M clk
	CLK25_8685_P1,			//8685 25M clk
	CLK25_8685S_P0,		//8685S 25M clk
	CLK25_9602C,
	NOT_DEFINED_CLK
};

struct pcie_para ePHY[][29] = {
     { {0, 1, 0x0003},   	{0, 2, 0x2d18}, 	{0, 3, 0x6d09},  	{0, 4, 0x5000},   
	{0, 0, 0x1046},	{0, 6, 0x0068}, 	{0, 5, 0x0bcb}, 	{0, 7, 0x30ff},   
	{0, 8, 0x18d7},	{0, 9, 0x530c}, 	{0, 0xa, 0x00e8},	{0, 0xb, 0x0511}, 
	{0, 0xc, 0x0828}, 	{0, 0xd, 0x17a6},	{0, 0xe, 0x98c5},	{0, 0xf, 0x0f0f}, 
	{0, 0x10, 0x000c},	{0, 0x11, 0x3c00},	{0, 0x12, 0xfc00},	{0, 0x13, 0x0c81},
	{0, 0x14, 0xde01},	{0, 0x19, 0xfc20},	{0, 0x1a, 0xfc00},	{0, 0x1b, 0xfc00},
	{0, 0x1c, 0xfc00},	{0, 0x1d, 0xa0eb},	{0, 0x1e, 0xc280},	{0, 0x1f, 0x05e0},
	{0xff,0xff,0xffff}},  //6166 35.328M clk
     {	{0, 1, 0x0003},	{0, 2, 0x2d18},	{0, 3, 0x6d09},	{0, 4, 0x5000},   
	{0, 0, 0x1047},	{0, 6, 0xf848},	{0, 5, 0x08ab},	{0, 7, 0x30ff},   
	{0, 8, 0x18d7},   	{0, 9, 0x530c},	{0, 0xa, 0x00e8},	{0, 0xb, 0x0511}, 
	{0, 0xc, 0x0828}, 	{0, 0xd, 0x17a6}, 	{0, 0xe, 0x98c5}, 	{0, 0xf, 0x0f0f}, 
	{0, 0x10, 0x000c},	{0, 0x11, 0x3c00},	{0, 0x12, 0xfc00},	{0, 0x13, 0x0c81},
	{0, 0x14, 0xde01},	{0, 0x19, 0xfc20},	{0, 0x1a, 0xfc00},	{0, 0x1b, 0xfc00},
	{0, 0x1c, 0xfc00},	{0, 0x1d, 0xa0eb},	{0, 0x1e, 0xc280},	{0, 0x1f, 0x05e0},
	{0xff,0xff,0xffff}}, //6166 25M clk
     {	{0, 1, 0x0003}, 	{0, 2, 0x2d18},	{0, 3, 0x4d09},	{0, 4, 0x5c3f},   
	{0, 0, 0x1046},   	{0, 6, 0x9048},	{0, 5, 0x2213},	{0, 7, 0x31ff},   
	{0, 8, 0x18d7},   	{0, 9, 0x539c},  	{0, 0xa, 0x00e8}, 	{0, 0xb, 0x0711}, 
	{0, 0xc, 0x0828}, 	{0, 0xd, 0x17a6},	{0, 0xe, 0x98c5},	{0, 0xf, 0x0f0f}, 
	{0, 0x10, 0x000c},	{0, 0x11, 0x3c00},	{0, 0x12, 0xfc00},	{0, 0x13, 0x0c81},
	{0, 0x14, 0xde01},	{0, 0x19, 0xfce0},	{0, 0x1a, 0x7c00},	{0, 0x1b, 0xfc00},
	{0, 0x1c, 0xfc00},	{0, 0x1d, 0xa0eb},	{0, 0x1e, 0xc280},	{0, 0x1f, 0x0600},
	{0xff,0xff,0xffff}}, //8676 35.328M clk
     {	{0, 1, 0x0003}, 	{0, 2, 0x2d18},	{0, 3, 0x4d09},	{0, 4, 0x5000},   
	{0, 0, 0x1047},   	{0, 6, 0x9148},	{0, 5, 0x23cb},	{0, 7, 0x31ff},   
	{0, 8, 0x18d7},   	{0, 9, 0x539c},  	{0, 0xa, 0x00e8}, 	{0, 0xb, 0x0711}, 
	{0, 0xc, 0x0828}, 	{0, 0xd, 0x17a6},	{0, 0xe, 0x98c5},	{0, 0xf, 0x0f0f}, 
	{0, 0x10, 0x000c},	{0, 0x11, 0x3c00},	{0, 0x12, 0xfc00},	{0, 0x13, 0x0c81},
	{0, 0x14, 0xde01},	{0, 0x19, 0xfce0},	{0, 0x1a, 0x7c00},	{0, 0x1b, 0xfc00},
	{0, 0x1c, 0xfc00},	{0, 0x1d, 0xa0eb},	{0, 0x1e, 0xc280},	{0, 0x1f, 0x0600},
	{0xff,0xff,0xffff}}, //8676 40M clk
     {	{0, 0, 0x1086},	{0, 4, 0x5800},	{0, 5, 0x05d3},	{0, 6, 0xf048},   
	{0, 0xb, 0x0711},	{0, 0xd, 0x1766},	{0, 0xf, 0x0a00},	{0, 0x1d, 0xa0eb},
	{0xff,0xff,0xffff}},//8686 40M clk
     {	{0, 6, 0xf848},	{0, 0xb, 0x0711},	{0, 0xd, 0x1766},	{0, 0xf, 0x0a00}, 
	{0, 0x01d, 0xa0eb},
	{1, 6, 0xf848}, {1, 0xb, 0x0711},       {1, 0xd, 0x1766},       {1, 0xf, 0x0a00},
        {1, 0x01d, 0xa0eb},
	{0xff,0xff,0xffff}},//8686 25M clk
     {	{0, 1, 0x0002},	{0, 2, 0x4300},	{0, 3, 0x0400},	{0, 4, 0x4644},
	{0, 0, 0x0000},	{0, 6, 0x30c0},	{0, 5, 0x8101},	{0, 7, 0x7440},
	{0, 8, 0x901c},	{0, 9, 0x0c9c},	{0, 0xa, 0x4037},	{0, 0xb, 0x2bb0},
	{0, 0xc, 0x8649}, {0, 0xd, 0x130d}, {0, 0xe, 0x0000}, {0, 0xf, 0x0026},
	{0xff,0xff,0xffff}}, //8676S 35.328M clk
     {	{0, 1, 0x0002},	{0, 2, 0x4300},	{0, 3, 0x0400},	{0, 4, 0x4644},
	{0, 0, 0x0000},	{0, 6, 0x30c0},	{0, 5, 0x8101},	{0, 7, 0x7440},
	{0, 8, 0x901c},	{0, 9, 0x0c9c},	{0, 0xa, 0x4037},	{0, 0xb, 0x2bb0},
	{0, 0xc, 0x8649}, {0, 0xd, 0x130d}, {0, 0xe, 0x0000}, {0, 0xf, 0x0026},
	{0xff,0xff,0xffff}}, //8676S 40M clk
     {	{0, 1, 0x06a3},	{0, 2, 0x4300},	{0, 3, 0x0400},	{0, 4, 0xd546},
	{0, 0, 0x0000},	{0, 6, 0xb880},	{0, 5, 0x8101},	{0, 7, 0x7c40},
	{0, 8, 0x901c},	{0, 9, 0x0c9c},	{0, 0xa, 0x4037},	{0, 0xb, 0x03b0},
	{0, 0xc, 0x0261},
	{0xff,0xff,0xffff}}, //0562 35.328M clk
     {	{0, 1, 0x06a3},	{0, 2, 0x4300},	{0, 3, 0x0400},	{0, 4, 0xd546},
	{0, 0, 0x0000},	{0, 6, 0xb880},	{0, 5, 0x8101},	{0, 7, 0x7c40},
	{0, 8, 0x901c},	{0, 9, 0x0c9c},	{0, 0xa, 0x4037},	{0, 0xb, 0x03b0},
	{0, 0xc, 0x0261},
	{0xff,0xff,0xffff}}, //0562 40M clk
     {	{0, 0, 0x404c},	{0, 1, 0x16a3},	{0, 2, 0x6340},	{0, 3, 0x370d},	
	{0, 4, 0x856a},	{0, 5, 0x8109},	{0, 6, 0x6081},	{0, 7, 0x5400},
	{0, 8, 0x9000},	{0, 9, 0x0ccc},	{0, 0xa, 0x4437},	{0, 0xb, 0x0230}, 	
	{0, 0xc, 0x0021},	{0, 0xd, 0x0000},	{0, 0xe, 0x0000},	{0, 0x1f, 0x0000}, 
	{0xff,0xff,0xffff}}, //8685 25M clk
     {	{1, 0, 0x404c},	{1, 1, 0x16a3},	{1, 2, 0x6340},	{1, 3, 0x370d},	
	{1, 4, 0x856a},	{1, 5, 0x8109},	{1, 6, 0x6081},	{1, 7, 0x5400},
	{1, 8, 0x9000},	{1, 9, 0x0ccc},	{1, 0xa, 0x4437},	{1, 0xb, 0x0230}, 	
	{1, 0xc, 0x0021},	{1, 0xd, 0x0000},	{1, 0xe, 0x0000},	{1, 0x1f, 0x0000}, 
	{0xff,0xff,0xffff}}, //8685 25M clk
      {{0, 0, 0x404c},	{0, 1, 0x16a3},	{0, 2, 0x6340},	{0, 3, 0x370d},	
        {0, 4, 0x4644}, {0, 5, 0x8109}, {0, 6, 0x2081}, {0, 7, 0x5400},
	{0, 8, 0x9000},	{0, 9, 0x0ccc},	{0, 0xa, 0x4437},	{0, 0xb, 0x0230}, 	
	{0, 0xc, 0x0021},	{0, 0xd, 0x0000},	{0, 0xe, 0x0000},	{0, 0x1f, 0x0000}, 
	{0xff,0xff,0xffff}}, //8685S 25M clk	
     {  {0, 6, 0xf848}, {0, 0xb, 0x0711},       {0, 0xd, 0x1766},       {0, 0xf, 0x0a00},
        {0, 0x01d, 0xa0eb}, {0, 0x19, 0xfc10},
        {0xff,0xff,0xffff}},//9602C

};

int PCIE_reset_procedure(int portnum, int Use_External_PCIE_CLK, int mdio_reset, unsigned long conf_addr)
{
#ifndef __OSK__
	extern void PCIE_reset_pin(int *reset);
#endif
#ifdef CONFIG_USE_PCIE_SLOT_1
	extern void PCIE1_reset_pin(int *reset);
#endif
	int PCIE_gpio_RST, i, idx;
	unsigned int PCIE_D_CFG0, PCIE_H_CFG, PCIE_H_PWRCR;
	unsigned int ENABLE_PCIE = EN_PCIE;

	if (portnum==0) {
		PCIE_D_CFG0 = BSP_PCIE0_D_CFG0;
		PCIE_H_CFG = BSP_PCIE0_H_CFG;
		PCIE_H_PWRCR = BSP_PCIE0_H_PWRCR;
#ifdef CONFIG_USE_PCIE_SLOT_1
	} else if(portnum==1) {
		PCIE_D_CFG0 = BSP_PCIE1_D_CFG0;
		PCIE_H_CFG = BSP_PCIE1_H_CFG;
		PCIE_H_PWRCR = BSP_PCIE1_H_PWRCR;
		ENABLE_PCIE = BSP_ENABLE_PCIE1;
#endif
	} else {
		printk("Error: portnum=%d\n", portnum);
		return FAIL;
	}

	if (pcie_reset_done[portnum]) 
		goto SET_BAR;

#ifndef __OSK__
#ifdef CONFIG_USE_PCIE_SLOT_1
	if (portnum==1)
		PCIE1_reset_pin(&PCIE_gpio_RST);
	else
#endif
		PCIE_reset_pin(&PCIE_gpio_RST);
#else
#ifdef CONFIG_RTL8685
#ifdef CONFIG_RTL8685S
	PCIE_gpio_RST = GPIO_D_2;
#else
#ifdef CONFIG_USE_PCIE_SLOT_1
	if (portnum==1)
#ifdef CONFIG_8685P_DUAL_BAND_GENERAL_BOARD_DBG_ON
                PCIE_gpio_RST = GPIO_D_7;
#else
		PCIE_gpio_RST = GPIO_E_2;
#endif
	else
#endif
#ifdef CONFIG_8685P_DUAL_BAND_GENERAL_BOARD_DBG_ON
                PCIE_gpio_RST = GPIO_F_1;
#else
		PCIE_gpio_RST = GPIO_D_4;
#endif
#endif
#else
#ifdef CONFIG_RTL8676S
#ifdef CONFIG_JTAG_USABLE
	PCIE_gpio_RST = GPIO_C_1; //tony 20130109: this pin shared with uart0 from spec,but actually uart1 for test chip.
#else
	PCIE_gpio_RST = GPIO_B_3; 
#endif
#else
	PCIE_gpio_RST = GPIO_H_2; //borad7
	//PCIE_gpio_RST = GPIO_A_5; //borad8
	REG32(MISC_PINMUX) &= ~4;
#endif	
#endif
#endif

	// 0. Assert PCIE Device Reset
#ifndef __OSK__
	gpioClear(PCIE_gpio_RST);
	gpioConfig(PCIE_gpio_RST, GPIO_FUNC_OUTPUT);
#else
	gpioConfig(PCIE_gpio_RST, GPIO_FUNC_OUTPUT);
#ifdef CONFIG_DUAL_CPU_MASTER
	//michael_gu patch 20140403
#ifdef CONFIG_8685P_DUAL_BAND_GENERAL_BOARD_DBG_ON
        gpioConfig(GPIO_F_1, GPIO_FUNC_OUTPUT);
        gpioSet(GPIO_F_1);
#else
        gpioConfig(GPIO_D_4, GPIO_FUNC_OUTPUT);
        gpioSet(GPIO_D_4);
#endif
#endif
#ifdef CONFIG_DUAL_CPU_SLAVE
#ifdef CONFIG_8685P_DUAL_BAND_GENERAL_BOARD_DBG_ON
        gpioConfig(GPIO_D_7, GPIO_FUNC_OUTPUT);
        gpioSet(GPIO_D_7);
#else
        gpioConfig(GPIO_E_2, GPIO_FUNC_OUTPUT);
        gpioSet(GPIO_E_2);
#endif
#endif
	gpioClear(PCIE_gpio_RST);
#endif
	delay_ms(10);

	// 1. PCIE phy mdio reset
#if !defined(CONFIG_ARCH_LUNA_SLAVE) || !defined(CONFIG_MASTER_WLAN0_ENABLE)
#if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
	if(pcie_reset_done[0]==0 && pcie_reset_done[1]==0){
#endif

	#ifndef CONFIG_RTL8676S
	REG32(PCI_MISC) = BSP_PCI_MDIO_RESET_ASSERT;
	REG32(PCI_MISC) = BSP_PCI_MDIO_RESET_RELEASE;
	#endif
#if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
	}
#endif
#endif	
	// 2. PCIE MAC reset
	REG32(MISC_IP_SEL) &= ~ENABLE_PCIE;
	REG32(MISC_IP_SEL) |= ENABLE_PCIE;
	#ifdef CONFIG_RTL8685S
	delay_ms(100);
	#endif
	
#if !defined(CONFIG_ARCH_LUNA_SLAVE) || !defined(CONFIG_MASTER_WLAN0_ENABLE)
#if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
	if(pcie_reset_done[0]==0 && pcie_reset_done[1]==0){
#endif
	if(mdio_reset)
	{
		//printk("Do MDIO_RESET\n");
		// 5.MDIO Reset
		#ifndef CONFIG_RTL8676S
		REG32(PCI_MISC) = BSP_PCI_MDIO_RESET_RELEASE;
		#else	
		REG32(PCI_MISC) &= ~(1<<20);
		delay_ms(2);
		REG32(PCI_MISC) |= (1<<20);	
		#endif
	}
#if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
	}
#endif
#endif	
	// 6. PCIE PHY Reset
	REG32(PCIE_H_PWRCR) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
	REG32(PCIE_H_PWRCR) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
	delay_ms(100);

	//----------------------------------------
	if (mdio_reset)
	{
		if (IS_6166 && (REG32(MISC_PINSR) & CLKSEL))
			idx = CLK35_328_6166;
		else if (IS_6166 && !(REG32(MISC_PINSR) & CLKSEL))
			idx = CLK25_6166;
		else if (IS_RTL8676 && !(REG32(MISC_PINSR) & CLKSEL))
			idx = CLK35_328_8676;
		else if (IS_RTL8676 && (REG32(MISC_PINSR) & CLKSEL))
			idx = CLK40_8676;
		else if (IS_RTL8686 && (REG32(MISC_PINSR) & CLKSEL)){
			printk("8686 40Mhz\n");
			idx = CLK40_8686;
		}
		else if(IS_RTL8686 && !(REG32(MISC_PINSR) & CLKSEL)){
			printk("8686 25Mhz\n");
			idx = CLK25_8686;
		}
		else if (IS_8676S_6333 && !(REG32(MISC_PINSR) & CLKSEL))
			idx = CLK35_328_8676S;
		else if (IS_8676S_6333 && (REG32(MISC_PINSR) & CLKSEL))
			idx = CLK40_8676S;
		else if (IS_8676S_0562 && !(REG32(MISC_PINSR) & CLKSEL))
			idx = CLK35_328_0562;
		else if (IS_8676S_0562 && (REG32(MISC_PINSR) & CLKSEL))
			idx = CLK40_0562;
		else if (IS_RTL8685){
			if (portnum==0){
				printk("8685 pcie port 0\n");
				idx = CLK25_8685_P0;
			}
			else{
				printk("8685 pcie port 1\n");
				idx = CLK25_8685_P1;				
			}
		}
#ifdef CONFIG_RTL8685S
		else if(IS_RTL8685S){
			printk("8685s pcie port 0\n");
			idx = CLK25_8685S_P0;
		}
#endif
		else if(IS_RTL9602C) {
			printk("RTL9602C PCIe Port 0 \n");
			idx = CLK25_9602C;
		}
		else
			idx = NOT_DEFINED_CLK;

		if (CLK40_8676 == idx)
			clk_src_40M = 1;

		for (i = 0; NOT_DEFINED_CLK != idx; ) {
			if(ePHY[idx][i].port != 0xff){
				if(portnum == ePHY[idx][i].port)
					HostPCIe_SetPhyMdioWrite(ePHY[idx][i].port, ePHY[idx][i].reg, ePHY[idx][i].value);

				i++;
			}
			else
				break;
		}
	}

	// 7. PCIE Device Reset
	gpioSet(PCIE_gpio_RST);

	// wait for LinkUP
	i = 100;
	while(--i)
	{
		if((REG32(PCIE_H_CFG + 0x0728)&0x1f)==0x11)
			break;
		delay_ms(10);
	}

	if (i == 0)
	{
		printk("Warring!! Port %d WLan device PCIE Link Failed, State=0x%x\n", portnum, REG32(PCIE_H_CFG + 0x0728));
		printk("Reset PCIE Host PHY and try again...\n");
	    // 4. PCIE PHY Reset
	    REG32(PCIE_H_PWRCR) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
	    REG32(PCIE_H_PWRCR) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1	
	    delay_ms(100);
	    // wait for LinkUP
	    i = 100;
	    while(--i)
	    {
		    if( (REG32(PCIE_H_CFG + 0x0728)&0x1f)==0x11)
			    break;
		    delay_ms(100);
	    }
		
		if (i == 0){
			printk("%s[%d]: Error!! Port %d WLan device PCIE Link failed, State=0x%x\n", __FUNCTION__, __LINE__, portnum, REG32(PCIE_H_CFG + 0x0728));
			//check pcie link fail~
			return FAIL;
		}
	}
	delay_ms(100);

	// 8. Set BAR
	REG32(PCIE_D_CFG0 + 0x10) = 0x18c00001;
	REG32(PCIE_D_CFG0 + 0x18) = 0x19000004;
	REG32(PCIE_D_CFG0 + 0x04) = 0x00180007;
	REG32(PCIE_H_CFG + 0x04) = 0x00100007;

//	DEBUG_INFO("Find Port_num=%d, Vender_Device_ID=0x%X\n", portnum, REG32(PCIE_D_CFG0 + 0x00) );

	if(i==0)
	{
		printk("Cannot LinkUP (0x%08X)\n", REG32(PCIE_H_CFG + 0x0728));
		return FAIL;
	}

#ifdef __OSK__
	//io and mem limit, setting to no litmit
    REG32(0xb8b0001c) = (2<<4) | (0<<12);   //  [7:4]=base  [15:12]=limit
    REG32(0xb8b00020) = (2<<4) | (0<<20);   //  [15:4]=base  [31:20]=limit 
    REG32(0xb8b00024) = (2<<4) | (0<<20);   //  [15:4]=base  [31:20]=limit 
    //printk("reset PCIE 2: b8b0001c:0x%08x, b8b00020:0x%08x, b8b00024:0x%08x\n", REG32(0xb8b0001c), REG32(0xb8b00020), REG32(0xb8b00024));
#endif

SET_BAR:
	// Enable PCIE host
	if (pcie_reset_done[portnum] == 0) {
		WRITE_MEM32(PCIE_H_CFG + 0x04, 0x00100007);
		WRITE_MEM8(PCIE_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
//		set_rtl8192cd_gpio();
		pcie_reset_done[portnum] = 1;
	}
#ifdef CONFIG_RTL_92D_DMDP
	else {
		Sw_PCIE_Func(1);	//choose the PCIE port number 
		WRITE_MEM32(PCIE_H_CFG + 0x04, 0x00100007);
		WRITE_MEM8(PCIE_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);
		Sw_PCIE_Func(0);
	}
#endif
	return SUCCESS;
}


#else

int PCIE_reset_procedure(int portnum, int Use_External_PCIE_CLK, int mdio_reset,unsigned long conf_addr)
{
	
	int i=100;
#define SYS_PCIE_PHY0   (0xb8000000 +0x50)
	//PCIE Register
#define CLK_MANAGE  0xb8000010
#define PCIE_PHY0_REG  0xb8b01000
	//#define PCIE_PHY1_REG  0xb8b21000
#define PCIE_PHY0  0xb8b01008
	// #define PCIE_PHY1  0xb8b21008
	int port =0;

	if (pcie_reset_done[portnum]) 
		goto SET_BAR;

	//2.Active LX & PCIE Clock
    REG32(CLK_MANAGE) |=  (1<<11);        //enable active_pcie0
    delay_ms(100);

#if 1
	if(mdio_reset)
	{
		//printk("Do MDIO_RESET\n");
		// 3.MDIO Reset
		REG32(SYS_PCIE_PHY0) = (1<<3) |(0<<1) | (0<<0);     //mdio reset=0,
		REG32(SYS_PCIE_PHY0) = (1<<3) |(0<<1) | (1<<0);     //mdio reset=1,
		REG32(SYS_PCIE_PHY0) = (1<<3) |(1<<1) | (1<<0);     //bit1 load_done=1
	}
	//4. PCIE PHY Reset
	REG32(PCIE_PHY0) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
	REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
	delay_ms(100);
#endif

	delay_ms(100);

   //----------------------------------------
   if(mdio_reset)
	{
		if (REG32(REVR) == RTL8196C_REVISION_A)
		{
		HostPCIe_SetPhyMdioWrite(port, 0, 0x5027);
		HostPCIe_SetPhyMdioWrite(port, 2, 0x6d18);
		HostPCIe_SetPhyMdioWrite(port, 6, 0x8828);
		HostPCIe_SetPhyMdioWrite(port, 7, 0x30ff);
		HostPCIe_SetPhyMdioWrite(port, 8, 0x18d7);
		HostPCIe_SetPhyMdioWrite(port, 0xa, 0xe9);
		HostPCIe_SetPhyMdioWrite(port, 0xb, 0x0511);
		HostPCIe_SetPhyMdioWrite(port, 0xd, 0x15b6);
		HostPCIe_SetPhyMdioWrite(port, 0xf, 0x0f0f);
#if 1 // PHY_EAT_40MHZ
		HostPCIe_SetPhyMdioWrite(port, 5, 0xbcb);    //[9:3]=1111001 (binary)   121 (10)
		HostPCIe_SetPhyMdioWrite(port, 6, 0x8128);  //[11]=0   [9:8]=01
#endif
		/*
		emdiow 0 5027
		emdiow 2 6d18
		emdiow 6 8828
		emdiow 7 30ff
		emdiow 8 18dd
		emdiow a e9
		emdiow b 0511
		emdiow d 15b6
		emdiow f 0f0f
		*/
		}
		else
		{
				HostPCIe_SetPhyMdioWrite(port, 0, 0xD087);
		HostPCIe_SetPhyMdioWrite(port, 1, 0x0003);
		HostPCIe_SetPhyMdioWrite(port, 2, 0x4d18);

#ifdef CONFIG_PHY_EAT_40MHZ

		printk("96C - 40MHz Clock Source\n");
#ifdef HIGH_POWER_EXT_PA
		HostPCIe_SetPhyMdioWrite(port, 5, 0x0BF3);   //40M
#else
		HostPCIe_SetPhyMdioWrite(port, 5, 0x0BCB);   //40M
#endif
		HostPCIe_SetPhyMdioWrite(port, 6, 0xF148);  //40M

#else

		printk("96C - 25MHz Clock Source\n");
		HostPCIe_SetPhyMdioWrite(port, 6, 0xf848);  //25M
#endif

		HostPCIe_SetPhyMdioWrite(port, 7, 0x31ff);
		HostPCIe_SetPhyMdioWrite(port, 8, 0x18d7);
		HostPCIe_SetPhyMdioWrite(port, 9, 0x539c);
		HostPCIe_SetPhyMdioWrite(port, 0xa, 0x20eb);
		HostPCIe_SetPhyMdioWrite(port, 0xb, 0x0511);
		HostPCIe_SetPhyMdioWrite(port, 0xd, 0x1764);
		HostPCIe_SetPhyMdioWrite(port, 0xf, 0x0a00);

#ifdef HAVING_FIB
		HostPCIe_SetPhyMdioWrite(port,8, 0x18dd);
		HostPCIe_SetPhyMdioWrite(port, 0xd, 0x1776);
#endif

		HostPCIe_SetPhyMdioWrite(port, 0x19, 0xFCE0);
		HostPCIe_SetPhyMdioWrite(port, 0x1e, 0xC280);
		}
	}

	//---------------------------------------
	// 6. PCIE Device Reset
#ifndef CONFIG_RTL_8196C_iNIC
	REG32(CLK_MANAGE) &= ~(1<<12);    //perst=0 off.
	delay_ms(300);
#endif
        //mark_inic , from jason patch
#ifdef CONFIG_RTL_8196C_iNIC
        iNIC_PCIE_Device_PERST();
#endif
#if  defined(CONFIG_RTL_8196CS) || defined(CONFIG_RTL_8197B)
	GPIO6_PCIE_Device_PERST();
#endif
	//4. PCIE PHY Reset
	REG32(PCIE_PHY0) = 0x1; //bit7 PHY reset=0   bit0 Enable LTSSM=1
	REG32(PCIE_PHY0) = 0x81;   //bit7 PHY reset=1   bit0 Enable LTSSM=1
	delay_ms(300);
#ifndef CONFIG_RTL_8196C_iNIC
        REG32(CLK_MANAGE) |=  (1<<12);   //PERST=1
#endif
	//4. PCIE PHY Reset

	//prom_printf("\nCLK_MANAGE(0x%x)=0x%x\n\n",CLK_MANAGE,READ_MEM32(CLK_MANAGE));
	delay_ms(100);
#if 1  //wait for LinkUP
	while(--i)
	{
		if( (REG32(0xb8b00728)&0x1f)==0x11)
		break;
		delay_ms(100);
	}
	if(i==0)
	{
		printk("i=%x Cannot LinkUP \n",i);
		return FAIL;
	}
#endif
	
#ifdef __OSK__
	//io and mem limit, setting to no litmit
    REG32(0xb8b0001c) = (2<<4) | (0<<12);   //  [7:4]=base  [15:12]=limit
    REG32(0xb8b00020) = (2<<4) | (0<<20);   //  [15:4]=base  [31:20]=limit 
    REG32(0xb8b00024) = (2<<4) | (0<<20);   //  [15:4]=base  [31:20]=limit 
    //printk("reset PCIE 2: b8b0001c:0x%08x, b8b00020:0x%08x, b8b00024:0x%08x\n", REG32(0xb8b0001c), REG32(0xb8b00020), REG32(0xb8b00024));
#endif
	
SET_BAR:
	if (port==0)
	{
	// Enable PCIE host
		if (pcie_reset_done[portnum] == 0) {
#if defined(__LINUX_2_6__) && defined(USE_RLX_BSP)
			WRITE_MEM32(BSP_PCIE0_H_CFG + 0x04, 0x00100007);
			WRITE_MEM8(BSP_PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B			
#else
			WRITE_MEM32(PCIE0_H_CFG + 0x04, 0x00100007);
			WRITE_MEM8(PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#endif
			pcie_reset_done[portnum] = 1;
		}
#ifdef CONFIG_RTL_92D_DMDP		
		else {
			Sw_PCIE_Func(1);
			//choose the PCIE port number 
#if defined(__LINUX_2_6__) && defined(USE_RLX_BSP)
			WRITE_MEM32(BSP_PCIE0_H_CFG + 0x04, 0x00100007);
			WRITE_MEM8(BSP_PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#else
			WRITE_MEM32(PCIE0_H_CFG + 0x04, 0x00100007);
			WRITE_MEM8(PCIE0_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#endif
			Sw_PCIE_Func(0);
		}
#endif
	}
	else if (port==1)
	{
		// Enable PCIE host
		if (pcie_reset_done[portnum] == 0) {
#if defined(__LINUX_2_6__) && defined(USE_RLX_BSP)
			WRITE_MEM32(BSP_PCIE1_H_CFG + 0x04, 0x00100007);
			WRITE_MEM8(BSP_PCIE1_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#else
			WRITE_MEM32(PCIE1_H_CFG + 0x04, 0x00100007);
			WRITE_MEM8(PCIE1_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#endif
			pcie_reset_done[portnum] = 1;
		}
#ifdef CONFIG_RTL_92D_DMDP
		else {
			Sw_PCIE_Func(1);
#if defined(__LINUX_2_6__) && defined(USE_RLX_BSP)
			//choose the PCIE port number 
			WRITE_MEM32(BSP_PCIE1_H_CFG + 0x04, 0x00100007);
			WRITE_MEM8(BSP_PCIE1_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#else
			WRITE_MEM32(PCIE1_H_CFG + 0x04, 0x00100007);
			WRITE_MEM8(PCIE1_H_CFG + 0x78, (READ_MEM8(conf_addr + 0x78) & (~0xE0)) | MAX_PAYLOAD_SIZE_128B);  // Set MAX_PAYLOAD_SIZE to 128B
#endif
			Sw_PCIE_Func(0);
		}
#endif
	}
	else
		return FAIL;
	return SUCCESS;
}

#endif	


void HostPCIe_Close(void)
{
	REG32(0xb8b10044) &= (~(3));
	REG32(0xb8b10044) |= (3);
	HostPCIe_SetPhyMdioWrite(0, 0xf, 0x0708);	
	 //.DeActive LX & PCIE Clock
#ifdef CONFIG_RTL8672
	REG32(MISC_IP_SEL) &= ~EN_PCIE;        //enable active_pcie0
#else
	REG32(CLK_MANAGE) &=(0xFFFFFFFF-  (1<<11));        //enable active_pcie0
#endif
	pcie_reset_done[0]=0;
}
#endif	/*	#elif (defined(CONFIG_RTL8196C) || defined(CONFIG_RTL_8196C))	*/

#endif	/*	!defined(CONFIG_NET_PCI)	*/
#endif


#ifdef CONFIG_RTL8671
/*6/7/04 hrchen, invalidate the dcache with a 0->1 transition*/

#ifdef CONFIG_CPU_RLX4181
int r3k_flush_dcache_range(int a, int b)
{
  int garbage_tmp;
	__asm__ __volatile__(
                ".set\tnoreorder\n\t"
                ".set\tnoat\n\t"
		"mfc0 %0, $20\n\t"
		"nop\n\t"
		"nop\n\t"
		"andi %0, 0xFDFF\n\t"
		"mtc0 %0, $20\n\t"
		"nop\n\t"
		"nop\n\t"
		"ori %0, 0x200\n\t"
		"mtc0 %0, $20\n\t"
		"nop\n\t"
		"nop\n\t"
                ".set\tat\n\t"
                ".set\treorder"
                : "=r" (garbage_tmp));
        return 0;
}
#else
int r3k_flush_dcache_range(int a, int b)
{
  int garbage_tmp;
	__asm__ __volatile__(
                ".set\tnoreorder\n\t"
                ".set\tnoat\n\t"
		"mfc0 %0, $20\n\t"
		"nop\n\t"
		"nop\n\t"
		"andi %0, 0xFFFE\n\t"
		"mtc0 %0, $20\n\t"
		"nop\n\t"
		"nop\n\t"
		"ori %0, 1\n\t"
		"mtc0 %0, $20\n\t"
		"nop\n\t"
		"nop\n\t"
                ".set\tat\n\t"
                ".set\treorder"
                : "=r" (garbage_tmp));
        return 0;
}
#endif

#ifdef _USE_DRAM_
//init DRAM
void r3k_enable_DRAM(void)
{
  int tmp, tmp1;
	//--- initialize and start COP3
	__asm__ __volatile__(
                ".set\tnoreorder\n\t"
                ".set\tnoat\n\t"
		"mfc0	%0,$12\n\t"
		"nop\n\t"
		"lui	%1,0x8000\n\t"
		"or	%1,%0\n\t"
		"mtc0	%1,$12\n\t"
		"nop\n\t"
		"nop\n\t"
                ".set\tat\n\t"
                ".set\treorder"
		: "=r" (tmp), "=r" (tmp1));

	//set base
	__asm__ __volatile__(
                ".set\tnoreorder\n\t"
                ".set\tnoat\n\t"
	 	"mtc3 %0, $4	# $4: d-ram base\n\t"
 	 	"nop\n\t"
	 	"nop\n\t"
                ".set\tat\n\t"
                ".set\treorder"
		:
		: "r" (DRAM_START_ADDR&0x0fffffff));

	//set size
	__asm__ __volatile__(
                ".set\tnoreorder\n\t"
                ".set\tnoat\n\t"
	 	"mtc3 %0, $5    # $5: d-ram top\n\t"
	 	"nop\n\t"
	 	"nop\n\t"
                ".set\tat\n\t"
                ".set\treorder"
		:
		: "r" (R3K_DRAM_SIZE-1));

	//clear DRAM
	__asm__ __volatile__(
"1:\n\t"
	 	"sw	$0,0(%1)\n\t"
	 	"addiu	%1,4\n\t"
	 	"bne	%0,%1,1b\n\t"
	 	"nop\n\t"
                ".set\tat\n\t"
                ".set\treorder"
                :
                : "r" (DRAM_START_ADDR+R3K_DRAM_SIZE), "r" (DRAM_START_ADDR));
}
#endif // _USE_DRAM_
#endif // CONFIG_RTL8671
#if defined(__ECOS) && defined(CONFIG_RTL_8197F)
static void PCIE_PHY_MDIO_Write(unsigned int portnum, unsigned int regaddr, unsigned short val)
{
	unsigned int mdioaddr;
	volatile int count;

	mdioaddr = PCIE_RC_EXTENDED_REG_MDIO;

	REG32(mdioaddr) = ( (regaddr&0x1f)<<PCIE_MDIO_REG_OFFSET) | ((val&0xffff)<<PCIE_MDIO_DATA_OFFSET)  | (1<<PCIE_MDIO_RDWR_OFFSET) ; 
	//delay
	for(count = 0; count < 5555; count++)
	{
		// do nothing, just delay
	}
}

static void PCIeMDIOPHYParameterSetRLE0600(unsigned int portnum)
{
	unsigned int temp=0;
	temp =REG32(0xb8000008);
	if((temp&(1<<24))==(1<<24))
	{
	printk("40MHz\r\n");		
		 PCIE_PHY_MDIO_Write(portnum, 0x0f, 0x12f6);
		 PCIE_PHY_MDIO_Write(portnum, 0x00, 0x0071);
		 PCIE_PHY_MDIO_Write(portnum, 0x06, 0x1ac1);
	}
	else
	{
		PCIE_PHY_MDIO_Write(portnum, 0x00, 0x0071);
		PCIE_PHY_MDIO_Write(portnum, 0x06, 0x18c1);
	}



/*
	PCIE_PHY_MDIO_Write(portnum, 0x00, 0x5083);
	PCIE_PHY_MDIO_Write(portnum, 0x04, 0xf048);
	PCIE_PHY_MDIO_Write(portnum, 0x06, 0x19e0);
	PCIE_PHY_MDIO_Write(portnum, 0x19, 0x6c69);
	PCIE_PHY_MDIO_Write(portnum, 0x1d, 0x0000);
	PCIE_PHY_MDIO_Write(portnum, 0x01, 0x0000);
	PCIE_PHY_MDIO_Write(portnum, 0x08, 0x9cc3);
	PCIE_PHY_MDIO_Write(portnum, 0x09, 0x4380);
	PCIE_PHY_MDIO_Write(portnum, 0x03, 0x7b44);
*/
	return;
}
#if 0
static void PCIeMDIOPHYParameterSetRLE0269B(unsigned int portnum)
{
	PCIE_PHY_MDIO_Write(portnum, 0x00, 0x1045);
	PCIE_PHY_MDIO_Write(portnum, 0x02, 0x4d16);
	PCIE_PHY_MDIO_Write(portnum, 0x06, 0xf018);
	PCIE_PHY_MDIO_Write(portnum, 0x08, 0x18d2);
	PCIE_PHY_MDIO_Write(portnum, 0x09, 0x930c);
	PCIE_PHY_MDIO_Write(portnum, 0x0a, 0x03c9);
	PCIE_PHY_MDIO_Write(portnum, 0x0d, 0x1566);
	return;
}
#endif
static void PCIE_MDIO_Reset_97F(unsigned int portnum)
{
	unsigned int sys_pcie_phy;

	sys_pcie_phy = SYS_PCIE_PHY;

	// MDIO Reset
	REG32(sys_pcie_phy) = (1<<3) |(0<<1) | (0<<0);     //mdio reset=0,
	REG32(sys_pcie_phy) = (1<<3) |(0<<1) | (1<<0);     //mdio reset=1,
	REG32(sys_pcie_phy) = (1<<3) |(1<<1) | (1<<0);     //bit1 load_done=1
}

void PCIE_PHY_Reset_97F(unsigned int portnum)
{
	unsigned int pcie_phy;

	pcie_phy = PCIE_RC_EXTENDED_REG_PWRCR;

    // PCIE PHY Reset
	REG32(pcie_phy) = 0x01;		//bit7:PHY reset=0   bit0: Enable LTSSM=1
	REG32(pcie_phy) = 0x81;		//bit7: PHY reset=1   bit0: Enable LTSSM=1
}

static void PCIE_MDIO_PHY_Parameter_Set(unsigned int portnum)
{
#ifdef CONFIG_RTK_FPGA
	PCIeMDIOPHYParameterSetRLE0269B(portnum);
#else
	PCIeMDIOPHYParameterSetRLE0600(portnum);
#endif
}

static void PCIE_Device_PERST_97F(unsigned int portnum)
{
	unsigned int sys_enable;

	sys_enable = SYS_ENABLE;

	REG32(sys_enable) &= ~(1<<1);    //perst=0 off.
	mdelay(300);  					//PCIE standadrd: poweron: 100us, after poweron: 100ms
	REG32(sys_enable) |=  (1<<1);   //PERST=1

}

static int PCIE_Check_Link_97F(unsigned int portnum)
{
	unsigned int dbgaddr;
	unsigned int cfgaddr;
	volatile int count = 5;

	dbgaddr = BSP_PCIE_RC_CFG + 0x728;

	//wait for LinkUP
	mdelay(10);

	while(--count)
	{
		mdelay(10);
		if( ( REG32(dbgaddr) & 0x1f) == 0x11)
		{
			break;
		}
	}

	if(count == 0){
		printk("PCIE ->  Cannot LinkUP\r\n" );
		return 0;
	}
	else  //already  linkup
	{
		cfgaddr = BSP_PCIE_EP_CFG;

		REG32(BSP_PCIE_RC_CFG + 0x04) = 0x00100007;
		REG32(BSP_PCIE_EP_CFG + 0x04) = 0x00100007;

		printk("Find PCIE Port, Device:Vender ID=%x\n", REG32(cfgaddr) );
	}

	return 1;
}

 int PCIE_reset_procedure_97F(unsigned int PCIeIdx, unsigned int mdioReset)
{
	int result=0;

	// Turn On PCIE IP
	REG32(SYS_CLK_MANAGE) |= (1<<12) | (1<<13) | (1<<18);
	REG32(SYS_CLK_MANAGE) |= (1<<14);
	mdelay(10);

	// Compatible
	//REG32(PCIE_RC_EXTENDED_REG_IPCFG) |= ((PCIeIdx*2+1) << 8);

	if(mdioReset)
	{
		printk("Do MDIO_RESET\r\n");
		mdelay(10);
		PCIE_MDIO_Reset_97F(PCIeIdx);
	}

	mdelay(10);
	PCIE_PHY_Reset_97F(PCIeIdx);
	mdelay(10);

	if(mdioReset)
	{
		PCIE_MDIO_PHY_Parameter_Set(PCIeIdx);
	}

	// PCIE Host Reset
	mdelay(10);
	PCIE_PHY_Reset_97F(PCIeIdx);

	// PCIE Device Reset
	PCIE_Device_PERST_97F(PCIeIdx);

	// Check link
	result = PCIE_Check_Link_97F(PCIeIdx);

	return result;
}

#endif

