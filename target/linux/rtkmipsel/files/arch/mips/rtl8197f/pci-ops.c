/*
 * Realtek Semiconductor Corp.
 *
 * bsp/pci-ops.c:
 * 	bsp PCI operation code
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/delay.h>

#include <asm/pci.h>
#include <asm/io.h>

#include "bspchip.h"


#define PCI_8BIT_ACCESS    1
#define PCI_16BIT_ACCESS   2
#define PCI_32BIT_ACCESS   4
#define PCI_ACCESS_READ    8
#define PCI_ACCESS_WRITE   16

#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *)   (addr)) = (val)
#define READ_MEM32(addr)         (*(volatile unsigned int *)   (addr))
#define WRITE_MEM16(addr, val)   (*(volatile unsigned short *) (addr)) = (val)
#define READ_MEM16(addr)         (*(volatile unsigned short *) (addr))
#define WRITE_MEM8(addr, val)    (*(volatile unsigned char *)  (addr)) = (val)
#define READ_MEM8(addr)          (*(volatile unsigned char *)  (addr))

extern void PCIE_Bus_Dev_Fun_Set(unsigned int id);

static int rtl8196f_pcibios_config_access(unsigned char access_type,
       unsigned int addr, unsigned int *data)
{
	/* Do 8bit/16bit/32bit access */
	if (access_type & PCI_ACCESS_WRITE)
	{
		if (access_type & PCI_8BIT_ACCESS){
			WRITE_MEM8(addr, *data);
		}else if (access_type & PCI_16BIT_ACCESS){
			WRITE_MEM16(addr, *data);
		}else{
			WRITE_MEM32(addr, *data);
		}
	}
	else if (access_type & PCI_ACCESS_READ)
	{
		if (access_type & PCI_8BIT_ACCESS){
			*data = READ_MEM8(addr);
		}else if (access_type & PCI_16BIT_ACCESS){
			*data = READ_MEM16(addr);
		}else{
			*data = READ_MEM32(addr);
		}
	}
	return 0;
}

static int rtl8196f_pcibios_read(struct pci_bus *bus, unsigned int devfn,
                                  int offset, int size, unsigned int *val)
{
	unsigned int addr = 0;

	/* PCIE devices directly connected */
	if (PCI_SLOT(devfn) == 0)
	{
		PCIE_Bus_Dev_Fun_Set( ((bus->number << 8) | devfn) & 0xffff );
		addr = BSP_PCIE_EP_CFG + offset;

		if (rtl8196f_pcibios_config_access(PCI_ACCESS_READ | size, addr, val))
		{
			return PCIBIOS_DEVICE_NOT_FOUND;
		}
	}else
	{
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	return PCIBIOS_SUCCESSFUL;
}

//========================================================================================
static int rtl8196f_pcibios_write(struct pci_bus *bus, unsigned int devfn,
                                   int offset, int size, unsigned int val)
{
	//unsigned int data = 0;
	unsigned int addr = 0;

	/* PCIE devices directly connected */
	if (PCI_SLOT(devfn) == 0)
	{
		PCIE_Bus_Dev_Fun_Set( ((bus->number << 8) | devfn) & 0xffff );
		addr = BSP_PCIE_EP_CFG + offset;

		if (rtl8196f_pcibios_config_access(PCI_ACCESS_WRITE | size, addr, &val))
		{
			return PCIBIOS_DEVICE_NOT_FOUND;
		}
	}else
	{
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

   return PCIBIOS_SUCCESSFUL;
}
//========================================================================================

struct pci_ops rtl8196f_pci_ops = {
   .read = rtl8196f_pcibios_read,
   .write = rtl8196f_pcibios_write
};
