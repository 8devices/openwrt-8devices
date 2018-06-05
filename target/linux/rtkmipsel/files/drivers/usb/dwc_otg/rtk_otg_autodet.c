
#include "rtk_otg_autodet.h"
#include <linux/proc_fs.h> //wei add
//--------------------------------------------------------------------------------------------
void dump_autodet_reg()
{
	#define GETBITVAL(v,bit)  ((v&(1<<bit))>>bit)

	unsigned int val=REG32(SYS_OTG_CONTROL)	;  //auto_det_ctrl
	printk("Otg_Ctrl_Reg=%x \n", val);

	int force=GETBITVAL(val,2);
	printk("  b02 [I] Otg_Mux_Sel=%x, %s mode \n", force, force? "Force": "Auto" );
	printk("  b19 [I] Vbus_On=%x \n", GETBITVAL(val,19));
	printk("  b20 [I] PJ_On=%x \n", GETBITVAL(val,20));
	printk("  b21 [I] PJ_Toggle=%x \n", GETBITVAL(val,21));
	printk("  b22 [I] Phy Device_Connect=%x \n", GETBITVAL(val,22));
	printk("  b23 [I] Phy Host_Disconnect=%x \n", GETBITVAL(val,23));

	int connect_sel=GETBITVAL(val,17);
	printk("  b17 [I] OTG Connect Sel=%x, From Phy %s  \n", connect_sel, connect_sel?"NOT DisConnect":"Connect" );
	printk("  b24 [O] Pow_PJ_On=%x \n", GETBITVAL(val,24));
	printk("  b25 [O] Pow_VB_On=%x \n", GETBITVAL(val,25));

	if(force)
	printk("  b03 [I] Otg_force_dev=%x, force use \n", GETBITVAL(val,3));
	printk("  b18 [O] Otg_cfg_dev_r=%x, auto use \n", GETBITVAL(val,18));


	#define GET_MVAL(v,bitpos,pat) ((v& ((unsigned int)pat<<bitpos))>>bitpos)
	#define RANG1 1
	#define RANG2 3
	#define RANG3  7
	#define RANG4 0xf
	printk("  b05    OTGCMP_EN=%x \n", GET_MVAL(val,5, RANG1));
	printk("  b09:06 Vref_L=%x\n", GET_MVAL(val,6, RANG4));
	printk("  b13:10 Vref_H=%x\n", GET_MVAL(val,10, RANG4));
	printk("  b16:14 Vbus_th=%x \n", GET_MVAL(val,14, RANG3));


	printk("\n");

}

//--------------------------------------------------------------------------------------------
void USBPHY_UTMI_Reset(int portnum, int reset)  //1: in reset,  0: working
{
	#define SYS_USB_PHY 0xb8000090
	if(portnum==0)
	{	if(reset==0)	REG32(SYS_USB_PHY) &= ~(1<<9);   //usbphy_reset=0
		else			REG32(SYS_USB_PHY) |=  (1<<9);   //usbphy_reset=1
	}
	else if(portnum==1)
	{	if(reset==0)	REG32(SYS_USB_PHY) &= ~(1<<20);   //usbphy_reset=0
		else			REG32(SYS_USB_PHY) |=  (1<<20);   //usbphy_reset=1
	}
}
//--------------------------------------------------------------------------------------------
void USBPHY_CHIP_Active(int portnum, int active)  //1: in reset,  0: working
{
	#define SYS_USB_PHY 0xb8000090
	if(portnum==0)
	{	if(active==0)	REG32(SYS_USB_PHY) &= ~(1<<10);   //usbphy_reset=0
		else
		{	REG32(SYS_USB_PHY) |=  (1<<8);   //usbphy_en=1
			REG32(SYS_USB_PHY) |=  (1<<10);   //usbphy_reset=1
		}
	}
	else if(portnum==1)
	{	if(active==0)	REG32(SYS_USB_PHY) &= ~(1<<21);   //usbphy_reset=0
		else
		{	REG32(SYS_USB_PHY) |=  (1<<19);   //usbphy_en=1
			REG32(SYS_USB_PHY) |=  (1<<21);   //usbphy_reset=1
		}

	}
}
//--------------------------------------------------------------------------------------------
unsigned int TurnOn_OTGCtrl_Interrupt(unsigned int en)
{
	int r=(REG32(BSP_GIMR)&(1<<1))>>1;
	if(en==1)
	{	//printk("====================en\n");
		REG32(BSP_GISR)=BSP_OTGCTRL_IP;  //GISR bit 1
		REG32(BSP_GIMR) |= BSP_OTGCTRL_IE;   //auto-det

	}
	else
	{	//printk("====================dis\n");
		REG32(BSP_GIMR) &= ~BSP_OTGCTRL_IE;   //auto-det
		REG32(BSP_GISR)=BSP_OTGCTRL_IP;  //GISR bit 1
	}
	return r;
}

//--------------------------------------------------------------------------------------------
int otg_proc_write_procmem(struct file *file, const char *buf, unsigned long count, void *data)
{

    unsigned char tmp[200];


    if (count < 2 || count > 200)
        return -EFAULT;

    memset(tmp, '\0', 200);
    if (buf && !copy_from_user(tmp, buf, count))
    {
		tmp[count-1]=0;


#if 0
        if (!strcmp(tmp, "host 1"))
	{	printk("Host Connect !\n");
		gHostMode=1;
		otg_reset_procedure(0);
		//dwc_otg_driver_init();
		dwc_otg_driver_probe(glmdev);
	}
        else if (!strcmp(tmp, "host 0"))
	{	printk("Host Dis-Connect !\n");
		//dwc_otg_driver_cleanup();
		dwc_otg_driver_remove(glmdev);
	}
	//--------------------------------------
	else if(!strcmp(tmp, "device 1"))
	{	printk("Device Connect !\n");
		gHostMode=0;
		otg_reset_procedure(0);
		dwc_otg_driver_probe(glmdev);
	}
        else if (!strcmp(tmp, "device 0"))
	{	printk("Device Dis-Connect !\n");
		dwc_otg_driver_remove(glmdev);
	}
	//--------------------------------------

	if(!strcmp(tmp, "gad 1"))
	{	printk("gad reg !\n");
		eth_reg_again();

	}
       else if (!strcmp(tmp, "gad 0"))
	{	printk("gad un-reg !\n");
		eth_unreg_again();
	}
	//--------------------------------------

	else if(!strcmp(tmp, "lm 1"))
	{	printk("lm reg !\n");
		lm_driver_register();
	}
        else if (!strcmp(tmp, "lm 0"))
	{	printk("lm unreg !\n");
		lm_driver_unregister();

	}
#endif
	//--------------------------------------
	if(!strcmp(tmp, "otg 1"))
	{	printk("otg drv reg !\n");
		dwc_otg_driver_init();
	}
       else if (!strcmp(tmp, "otg 0"))
	{	printk("otg drv unreg !\n");
		dwc_otg_driver_cleanup();

	}
	//--------------------------------------
	else if(!strcmp(tmp, "ehci 1"))
	{	printk("ehci drv reg !\n");
		//ehci_autodet_probe();
		ehci_hcd_init();
	}
       else if (!strcmp(tmp, "ehci 0"))
	{	printk("ehci drv unreg !\n");
		//ehci_autodet_remove();
		ehci_hcd_cleanup();
	}
	//--------------------------------------
	else if(!strcmp(tmp, "ohci 1"))
	{	printk("ohci drv reg !\n");
		ohci_hcd_mod_init();
	}
       else if (!strcmp(tmp, "ohci 0"))
	{	printk("ohci drv unreg !\n");
		ohci_hcd_mod_exit();
	}
	//--------------------------------------
	else if(!strcmp(tmp, "port ?"))
	{	printk("curr port is %x\n", Get_SelUSBPort());
	}
	else if(!strcmp(tmp, "port 1"))
	{	printk("port 1 \n");
		Set_SelUSBPort(1);
	}
       else if (!strcmp(tmp, "port 0"))
	{	printk("port 0 \n");
		Set_SelUSBPort(0);
	}
	//--------------------------------------
	else if(!strcmp(tmp, "autodet 1"))
	{
		Enable_AutoDetectionCircuit(1);
		Set_IDDIG_Level(0,0);
	}
       else if (!strcmp(tmp, "autodet 0"))
	{
		Enable_AutoDetectionCircuit(0);
		Set_IDDIG_Level(1,0);
	}
	//--------------------------------------
	else if(!strcmp(tmp, "idpin 1"))
	{
		Set_IDDIG_Level(1,1);  //force device
	}
       else if (!strcmp(tmp, "idpin 0"))
	{
		Set_IDDIG_Level(1,0);  //force host
	}
	//--------------------------------------


        else if (!strcmp(tmp, "dump otg"))
	{

		extern int gHostMode;
		printk("gHostMode=%d\n", gHostMode);

		extern unsigned long otg_driver_loaded;
		printk("otg_driver_loaded=%d\n", otg_driver_loaded);

#if 0
		extern struct lm_device *glmdev; //wei add
		struct lm_device *pdev=glmdev;
		printk("glmdev=%x\n", pdev);
		dwc_otg_device_t *otg_dev = platform_get_drvdata(pdev);
		printk("otg_dev=%x\n", otg_dev);

		/*
		dwc_otg_hcd_t *dwc_otg_hcd = otg_dev->hcd;
		printk("dwc_otg_hcd=%x\n",dwc_otg_hcd );
		struct usb_hcd *hcd = dwc_otg_hcd_to_hcd(dwc_otg_hcd);
		printk("hcd=%x\n",jcd );
		*/
#endif
	}

        else if (!strcmp(tmp, "dump otgctrl"))
	{
		dump_autodet_reg();
        }
        else
        {
           // printk("=>%s, count=%d\n",(char *) tmp, (int)count);
           printk("Unknow command ! \n");

//            printk("host <0|1>  \n");
//           printk("device <0|1> \n");
//           printk("gad <0|1 >! \n");
           printk("otg <0|1> \n");
           printk("ehci <0|1> \n");
           printk("port <0|1|?> \n");
           printk("autodet <0|1> \n");
           printk("idpin <0|1> \n");

           printk("dump otg \n");
           printk("dump otgctrl \n");

        }
    }
    //printk("WRITE PROC\n");
    return count;

}
//--------------------------------------------------------------------------------------------
int otg_proc_read_procmem(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int i, j, len = 0;
    int limit = count - 80; /* Don't print more than this */

        len += sprintf(buf+len,"\nOtg Debug List  \n" );
//        len += sprintf(buf+len," host <0|1>   \n" );
//        len += sprintf(buf+len," device <0|1>  \n" );
//        len += sprintf(buf+len," gad <0|1 >  \n" );
        len += sprintf(buf+len," otg <0|1>  \n" );
        len += sprintf(buf+len," ehci <0|1>  \n" );


	//printk("dump: dump internal value \n");
    return len;

}

//========================================================================

typedef struct otgctrl_work_hdr
{
//	struct work_struct	start_work;
	struct delayed_work start_work;

	unsigned int curr_val;
	//unsigned int last_val;
	unsigned int mode;
/*
	plugin:

	0: disconnect
	1: device connect without power
	2: device connect with power
	3: no device connect but with power (PC plug in)

	4: device plugin
	5: pc pluigin

*/

	unsigned int idpin;
	/* 1: mac is device mode
	    0: mac is host mode
	*/
	unsigned int nFirst;

}otgctrl_work_hdr_t;
otgctrl_work_hdr_t otgctrl_work;

//---------------------------------------------------------------------------------------
struct timer_list otg_timer;
#if 1
static void otg_timer_isr(unsigned long data)
{
	printk("timer !\n");
//	mod_timer(&otg_timer, jiffies + 500);
//	del_timer(&otg_timer);
}
#endif
//--------------------------------------------------------------------------------------------
#define P1_HOST_USING_EHCI 1
static void otgctrl_work_func(struct work_struct *work)
//static void otg_timer_isr(unsigned long data)
{
	  int old=TurnOn_OTGCtrl_Interrupt(0);

	dump_autodet_reg();

//	otgctrl_work_hdr_t *priv = container_of(work, otgctrl_work_hdr_t, start_work);
	otgctrl_work_hdr_t *priv =&otgctrl_work;


	unsigned int v=(priv->curr_val) ;

#if 0  //software only see idpin result
	//plugin: PC-> usb disk
	if(
	     ( (v&OTGCTRL_CFG_DEV_R)==0)	 &&
	     ( (priv->idpin==1)   ||  (priv->nFirst==0) )
	  )

	{	printk("OTGCTRL: Det is Host mode \n" );
	       int old=TurnOn_OTGCtrl_Interrupt(0);

		//close
		dwc_otg_driver_cleanup();

		//port
	Set_SelUSBPort(1);
		//mdelay(100);

		//open
		ehci_hcd_init();
		ohci_hcd_mod_init();

	      TurnOn_OTGCtrl_Interrupt(old);


         priv->idpin=0;
	  priv->nFirst=1;

	}
	//plugin: usb disk  -> PC
	else if(
		      ((v&OTGCTRL_CFG_DEV_R))	 &&
		      ( (priv->idpin==0) ||  (priv->nFirst==0)   )
		   )
	{
		printk("OTGCTRL: Det is Device mode \n" );
		int old=TurnOn_OTGCtrl_Interrupt(0);

		//close
		ehci_hcd_cleanup();
		//ohci_hcd_mod_exit();


		//port
		Set_SelUSBPort(0);
		//mdelay(100);
		//HangUpRes(0);

		//open
		dwc_otg_driver_init();
		TurnOn_OTGCtrl_Interrupt(old);



		priv->idpin=1;
		priv->nFirst=1;
	}
#endif
#if 1   //software decide the state machine
  if(v&OTGCTRL_PJ_ON)
  {
       //connect=1
	//if( (v&OTGCTRL_DEVICE_CONNECT) &&(priv->mode==0))
	if(  ((v&OTGCTRL_DEVICE_DISCONNECT)==0) &&(priv->mode==0) )
	{
		//change port
		//USBPHY_CHIP_Active(1,0);
		//USBPHY_UTMI_Reset(1, 1);

		//USBPHY_CHIP_Active(1,1);
		//USBPHY_UTMI_Reset(1, 0);

		printk("OTGCTRL: PJ on, Det device plugin\n");

		#if P1_HOST_USING_EHCI //EHCI
			Set_SelUSBPort(1);
			ehci_hcd_init();
			ohci_hcd_mod_init();
		#else
			dwc_otg_driver_init();
		#endif
			priv->mode=1;


	}
	//dis-connect=1
	else if( (v&OTGCTRL_DEVICE_DISCONNECT) && (priv->mode==1) )
	{
		#if P1_HOST_USING_EHCI
			ehci_hcd_cleanup();
			//ohci_hcd_mod_exit();
		#else
			dwc_otg_driver_cleanup();
		#endif


		priv->mode=0;
	}
	//---------------------
	else if( (v&OTGCTRL_DEVICE_DISCONNECT) && (priv->mode==0) &&(v&OTGCTRL_VBUS_ON) )
	{
		Set_SelUSBPort(0);
		printk("OTGCTRL: Vbus on, Det host plugin\n");
		priv->mode=3;
		dwc_otg_driver_init();

	}
	else if( (v&OTGCTRL_DEVICE_DISCONNECT) && (priv->mode==3)  )
	{
		dwc_otg_driver_cleanup();
		priv->mode=0;

	}
	//---------------------
  }
  //==============================
  else // OTGCTRL_PJ_ON=0
  {
	if(  ((v&OTGCTRL_DEVICE_DISCONNECT)==0) &&(priv->mode==0) )
	{
		Set_SelUSBPort(0);
		printk("OTGCTRL: Vbus on, Det Device plugin\n");
		priv->mode=4;
		dwc_otg_driver_init();

	}
	else if( (v&OTGCTRL_DEVICE_DISCONNECT) && (priv->mode==4)  )
	{
		dwc_otg_driver_cleanup();
		priv->mode=0;

	}
	//---------------
	else if(  (v&OTGCTRL_DEVICE_DISCONNECT) &&(priv->mode==0) )
	{
		Set_SelUSBPort(0);
		printk("OTGCTRL: Vbus on, Det host plugin\n");
		priv->mode=5;
		dwc_otg_driver_init();

	}
	else if( (v&OTGCTRL_DEVICE_DISCONNECT) && (priv->mode==5)  )
	{
		dwc_otg_driver_cleanup();
		priv->mode=0;

	}
  }
#endif

	  TurnOn_OTGCtrl_Interrupt(old);
}

//--------------------------------------------------------------------------------------------
static irqreturn_t otg_ctrl_irq(int _irq, void *_dev)
{	int32_t retval = IRQ_NONE;
	printk("=> OTG_CTRL IRQ in\n");

	REG32(BSP_GISR)=BSP_OTGCTRL_IP;  //GISR bit 1

	//printk("0xb8003000=%x \n", REG32(0xb8003000) );
	//dump_autodet_reg();

	unsigned int v=REG32(SYS_OTG_CONTROL);

	//if(v&OTGCTRL_DEVICE_CONNECT)
	{
		otgctrl_work.curr_val=v;

		cancel_delayed_work(&otgctrl_work.start_work);
		//flush_work(&otgctrl_work.start_work);
		cancel_delayed_work_sync(&otgctrl_work.start_work);
		//flush_scheduled_work();



		//schedule_work(&otgctrl_work.start_work);
		PREPARE_DELAYED_WORK(&otgctrl_work.start_work, otgctrl_work_func);
		schedule_delayed_work(&otgctrl_work.start_work, 100);




	}
	retval=IRQ_HANDLED;
	return (retval);
}
//--------------------------------------------------------------------------------------------
static irqreturn_t usb0_wakeup_irq(int _irq, void *_dev)
{
	int32_t retval = IRQ_NONE;

	//printk("=> usbphy0_wakeup IRQ in\n");
	REG32(0xb8000004)=(1<<0);
	retval=IRQ_HANDLED;
	return (retval);
}
//--------------------------------------------------------------------------------------------
static irqreturn_t usb1_wakeup_irq(int _irq, void *_dev)
{
	int32_t retval = IRQ_NONE;

	//printk("=> usbphy1_wakeup IRQ in\n");
	REG32(0xb8000004)=(1<<2);
	retval=IRQ_HANDLED;
	return (retval);
}

//--------------------------------------------------------------------------------------------
struct device gOtgCtrlDev;
struct device gUSB0CtrlDev;
struct device gUSB1CtrlDev;

//--------------------------------------------------------------------------------------------
void otg_ctrl_init(void)
{
	Enable_AutoDetectionCircuit(0);

 //wei add for test
	struct proc_dir_entry *entry=create_proc_entry("otg", 0, NULL);
	if (entry)
	{  entry->write_proc=otg_proc_write_procmem;
		entry->read_proc=otg_proc_read_procmem;
	}

	int retval = request_irq(BSP_OTGCTRL_IRQ, otg_ctrl_irq, IRQF_DISABLED, "otg_ctrl", &gOtgCtrlDev );
	if(retval!=0)
	{	printk("=>otg_ctrl request_irq fail\n");
		return;
	}

	retval = request_irq(BSP_USB0_WAKEUP_IRQ, usb0_wakeup_irq, IRQF_DISABLED, "usb0_ctrl", &gUSB0CtrlDev );
	if(retval!=0)
	{	printk("=>usb0 wakeup  request_irq fail\n");
		return;
	}

	retval = request_irq(BSP_USB1_WAKEUP_IRQ, usb1_wakeup_irq, IRQF_DISABLED, "usb1_ctrl", &gUSB1CtrlDev );
	if(retval!=0)
	{	printk("=>usb1 wakeup request_irq fail\n");
		return;
	}


#if 1
	memset(&otgctrl_work,0,sizeof(otgctrl_work_hdr_t));
	//INIT_WORK(&otgctrl_work.start_work, otgctrl_work_func);
	INIT_DELAYED_WORK(&otgctrl_work.start_work, otgctrl_work_func);
#endif
#if 0
	init_timer(&otg_timer);
	otg_timer.expires = jiffies + 500;
	otg_timer.data = (unsigned long)(0);
	otg_timer.function = otg_timer_isr;
	add_timer(&otg_timer);
#endif
	//clear all interrupt
	REG32(BSP_GISR)=(1<<1);  //GISR bit 1
	REG32(0xb8000004)=(1<<0);  //phy0
	REG32(0xb8000004)=(1<<2);  //phy1

	//enable
	REG32(BSP_GIMR) |= BSP_OTGCTRL_IE;   //auto-det
	REG32(BSP_GIMR) |= BSP_USB0_WAKEUP_IE;   //bit 28=usb0
	REG32(BSP_GIMR) |= BSP_USB1_WAKEUP_IE;   //bit 25=usb1


#ifdef CONFIG_RTL_OTGCTRL
		//top, enable auto-det
		HangUpRes(1);

		//REG32(SYS_OTG_CONTROL)&=~(1<<17);   //0: connect is come from connect
		REG32(SYS_OTG_CONTROL)|=(1<<17);   //1: connect is come from dis-connect

		//Set_SelUSBPort(0);

		USBPHY_CHIP_Active(1,1);
		USBPHY_UTMI_Reset(1,0);

		Enable_AutoDetectionCircuit(0);
		//Enable_AutoDetectionCircuit(1);
		Set_IDDIG_Level(0,0);
#endif
	printk("OTGCTRL: init ok\n");
}
//--------------------------------------------------------------------------------------------
//module_init(otg_ctrl_init);
//postcore_initcall(otg_ctrl_init);  //before ehci and ohci
late_initcall(otg_ctrl_init);  //before ehci and ohci
