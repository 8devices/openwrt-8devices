

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/stat.h>	 /* permission constants */

#include <linux/platform_device.h>
#include "bspchip.h"  //wei add
#include <asm/io.h>
#define REG32(reg)   (*(volatile unsigned int *)((unsigned int)reg))
#include "lm.h"

#include "dwc_otg_plat.h"
#include "dwc_otg_attr.h"
#include "dwc_otg_driver.h"
#include "dwc_otg_pcd.h"
#include "dwc_otg_hcd.h"


#define SYS_OTG_CONTROL 0xb8000098

#define OTGCTRL_CFG_DEV_R  (1<<18)
#define OTGCTRL_VBUS_ON  (1<<19)
#define OTGCTRL_PJ_ON  (1<<20)
#define OTGCTRL_DEVICE_CONNECT (1<<22)
#define OTGCTRL_DEVICE_DISCONNECT (1<<23)


//otg driver
extern int  dwc_otg_driver_init(void);
extern void  dwc_otg_driver_cleanup(void);

//otg gad
/*
extern int eth_reg_again();
extern int eth_unreg_again();
*/


//ohci
extern int  ohci_hcd_mod_init(void);
extern void ohci_hcd_mod_exit(void);

//ehci my add
	//extern void ehci_autodet_probe(void);
	//extern void ehci_autodet_remove(void);
//ehci driver
extern  int ehci_hcd_init(void);
extern  void ehci_hcd_cleanup(void);





//tool
extern  void Set_SelUSBPort(int);
extern unsigned int Get_SelUSBPort();
extern Enable_AutoDetectionCircuit(int en);
extern Set_IDDIG_Level(int force, int device);  // force: 0: auto, 1: force,    device: 1:device 0:host
extern int otg_reset_procedure(int mode);

//
void USBPHY_UTMI_Reset(int portnum, int reset);
void USBPHY_CHIP_Active(int portnum, int active);

extern HangUpRes(int en);
