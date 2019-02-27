/*
 * Realtek Semiconductor Corp.
 *
 * pci.c:
 *     bsp PCI initialization code
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/types.h>
#include <linux/delay.h>

#include <asm/cpu.h>
#include <asm/io.h>
#include <asm/pci.h>

#include "bspchip.h"


#define SYS_CLK_MANAGE				(SYSTEM_REG_BASE + 0x10)
#define SYS_ENABLE					(SYSTEM_REG_BASE + 0x50)
#define SYS_PCIE_PHY				(SYSTEM_REG_BASE + 0x100)

#define PCIE_RC_EXTENDED_REG_MDIO	(BSP_PCIE_RC_EXTENDED_REG + 0x00)
#define PCIE_RC_EXTENDED_REG_PWRCR	(BSP_PCIE_RC_EXTENDED_REG + 0x08)
#define PCIE_RC_EXTENDED_REG_IPCFG	(BSP_PCIE_RC_EXTENDED_REG + 0x0c)

// PCIE_RC_EXTENDED_REG_MDIO
#define PCIE_MDIO_DATA_OFFSET 		(16)
#define PCIE_MDIO_REG_OFFSET 		(8)
#define PCIE_MDIO_RDWR_OFFSET 		(0)

#define PADDR(addr)  				((addr) & 0x1FFFFFFF)

#ifdef CONFIG_PCI
extern struct pci_ops rtl8196f_pci_ops;

static struct resource rtl8196f_pci_io_resource = {
   .name   = "RTL8196f PCI IO",
   .flags  = IORESOURCE_IO,
   .start  = PADDR(BSP_PCIE_IO_BASE),
   .end    = PADDR(BSP_PCIE_IO_BASE + 0x1FFFFF)
};

static struct resource rtl8196f_pci_mem_resource = {
   .name   = "RTL8196f PCI MEM",
   .flags  = IORESOURCE_MEM,
   .start  = PADDR(BSP_PCIE_MEM_BASE),
   .end    = PADDR(BSP_PCIE_MEM_BASE + 0xFFFFFF)
};

static struct pci_controller rtl8196f_pci_controller = {
   .pci_ops        = &rtl8196f_pci_ops,
   .mem_resource   = &rtl8196f_pci_mem_resource,
   .io_resource    = &rtl8196f_pci_io_resource,
};
#endif
void PCIE_Bus_Dev_Fun_Set(unsigned int id)
{
	REG32(PCIE_RC_EXTENDED_REG_IPCFG) |= id;
}


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
static void PCIE_MDIO_Reset(unsigned int portnum)
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

static void PCIE_Device_PERST(unsigned int portnum)
{
	unsigned int sys_enable;

	sys_enable = SYS_ENABLE;

	REG32(sys_enable) &= ~(1<<1);    //perst=0 off.
	mdelay(300);  					//PCIE standadrd: poweron: 100us, after poweron: 100ms
	REG32(sys_enable) |=  (1<<1);   //PERST=1

}

int PCIE_link_ok = 0;
unsigned int PCIE_dev_id = 0;
EXPORT_SYMBOL(PCIE_link_ok);
EXPORT_SYMBOL(PCIE_dev_id);


static int PCIE_Check_Link(unsigned int portnum)
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
		PCIE_link_ok = 1;
		cfgaddr = BSP_PCIE_EP_CFG;

		REG32(BSP_PCIE_RC_CFG + 0x04) = 0x00100007;
		REG32(BSP_PCIE_EP_CFG + 0x04) = 0x00100007;

		PCIE_dev_id = REG32(cfgaddr);

		printk("Find PCIE Port, Device:Vender ID=%x\n", PCIE_dev_id);
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
		PCIE_MDIO_Reset(PCIeIdx);
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
	PCIE_Device_PERST(PCIeIdx);

	// Check link
	result = PCIE_Check_Link(PCIeIdx);

	return result;
}
#if defined(CONFIG_OPENWRT_SDK)
EXPORT_SYMBOL(PCIE_reset_procedure_97F);
#endif

static int __init bsp_pcie_init(void)
{
	//int Use_External_PCIE_CLK = 0;
	int result = 0;
	printk("<<<<<Register PCI Controller>>>>>\n");
	mdelay(1);

	result = PCIE_reset_procedure_97F(0, 1);

	mdelay(1);

	if(result)
	{
#ifdef CONFIG_PCI
		register_pci_controller(&rtl8196f_pci_controller);
#endif
	}else
	{
		REG32(SYS_CLK_MANAGE) &=  (~(1<<14));        //disable active_pcie0
	}
	return 0;
}

arch_initcall(bsp_pcie_init);
