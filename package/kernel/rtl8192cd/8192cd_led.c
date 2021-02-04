/*
 *   Handling routines for LED lightening functions
 *
 *  $Id: 8192cd_led.c,v 1.1 2012/05/04 12:49:07 jimmylin Exp $
 *
 *  Copyright (c) 2012 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_LED_C_

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"

#if defined(SUPPORT_UCFGING_LED) 
struct timer_list	LED_TimerCFGING;
unsigned int		LED_CFGING_Interval;
unsigned char		LED_CFGING_Toggle;
unsigned char		LED_CFGING_ToggleStart;
unsigned int 		LED_Configuring=0;
#endif

#ifdef __OSK__
#ifdef WIFI_SW_LED  
extern struct rtl8192cd_priv *wifi_root_priv;
void rtl8192cd_LED_SW(u8 on) 
{
	if(wifi_root_priv){
		if(on)
			control_wireless_led(wifi_root_priv,1);
		else
			control_wireless_led(wifi_root_priv,0);
	}
	return;
}

void rtl8192cd_get_LED_control()
{
	if(wifi_root_priv){
		control_wireless_led(wifi_root_priv,0);
	}
}

void rtl8192cd_release_LED_control()
{
	if(wifi_root_priv){
		control_wireless_led(wifi_root_priv,2);
	}
}
#endif
#endif

#if defined(CONFIG_RTL_8197F)
#if defined(CONFIG_OPENWRT_SDK) || defined(__LINUX_2_6__)
#include <bspchip.h>
#elif defined(__ECOS)
#include <cyg/hal/bspchip.h>
#else
#include <bsp/bspchip.h>
#endif
#if !defined(__ECOS)
#include <linux/gpio.h>
#endif
#endif


// for SW LED ----------------------------------------------------
#ifdef RTL8190_SWGPIO_LED
static void set_swGpio_LED(struct rtl8192cd_priv *priv, unsigned int ledNum, int flag)
{
	unsigned int ledItem;	/* parameter to decode GPIO item */

	if (ledNum >= SWLED_GPIORT_CNT)
		return;

	ledItem = SWLED_GPIORT_ITEM(LED_ROUTE, ledNum);

	if (ledItem & SWLED_GPIORT_ENABLEMSK)
	{
		/* get the corresponding information (GPIO number/Active high or low) of LED */
		int gpio;
		int activeMode;	/* !=0 : Active High, ==0 : Active Low */

		gpio = ledItem & SWLED_GPIORT_RTBITMSK;
		activeMode = ledItem & SWLED_GPIORT_HLMSK;

		if (flag) {	/* Turn ON LED */
			if (activeMode)	/* Active High */
				RTL_W8(0x90, RTL_R8(0x90) | BIT(gpio));
			else			/* Active Low */
				RTL_W8(0x90, RTL_R8(0x90) &~ BIT(gpio));
		}
		else {	/* Turn OFF LED */
			if (activeMode)	/* Active High */
				RTL_W8(0x90, RTL_R8(0x90) &~ BIT(gpio));
			else			/* Active Low */
				RTL_W8(0x90, RTL_R8(0x90) | BIT(gpio));
		}
	}
}
#endif // RTL8190_SWGPIO_LED


static void set_sw_LED0(struct rtl8192cd_priv *priv, int flag)
{
#ifdef RTL8190_SWGPIO_LED
    if (LED_ROUTE)
        set_swGpio_LED(priv, 0, flag);
#elif defined(CONFIG_RTL8672) || defined(NOT_RTK_BSP)
    if (flag)
        RTL_W32(LEDCFG, (RTL_R32(LEDCFG) & 0xfffffff0) | LED0SV);
    else
        RTL_W32(LEDCFG, RTL_R32(LEDCFG) & 0xfffffff0);
#else
    if ((GET_CHIP_VER(priv) == VERSION_8188E)||(GET_CHIP_VER(priv) == VERSION_8192E)) {
        #ifdef RTLWIFINIC_GPIO_CONTROL
        if (flag)
            RTLWIFINIC_GPIO_write_proc(priv, 5, 0);
        else
            RTLWIFINIC_GPIO_write_proc(priv, 5, 1);
        #endif
    }
    else if ((GET_CHIP_VER(priv) == VERSION_8812E)) {
        if (flag)
            RTL_W32(LEDCFG, (RTL_R32(LEDCFG) & 0xfffff0ff) | LED1SV);
        else
            RTL_W32(LEDCFG, RTL_R32(LEDCFG) & 0xfffff0ff);
    }
#if defined(CONFIG_WLAN_HAL_8814AE)
	else if (GET_CHIP_VER(priv) == VERSION_8814A) {
		if (flag)
			RTL_W8(LEDCFG+1, (RTL_R8(LEDCFG+1) | BIT(3)));
		else
			RTL_W8(LEDCFG+1, RTL_R8(LEDCFG+1) & 0xF7);
	}
#endif	
    else if (GET_CHIP_VER(priv) == VERSION_8881A) {
        #ifndef __OSK__
        if (flag)
            writel(readl(IO_TYPE_CAST(0xb800350c)) | BIT(24), IO_TYPE_CAST(0xb800350c));
        else
            writel(readl(IO_TYPE_CAST(0xb800350c)) & ~BIT(24), IO_TYPE_CAST(0xb800350c));
        #else
        setWlan5gLed(flag);/*tony 20150203: we should use gpio api, avoid gpio abnormal behavior.*/
        #endif
    }    
    #if defined(CONFIG_RTL_8197F) && !defined(__ECOS)
    else if(GET_CHIP_VER(priv) == VERSION_8197F) {
#if 1
        if(flag) /* led off*/
            writel(readl(IO_TYPE_CAST(0xb8003528)) | BIT(1), IO_TYPE_CAST(0xb8003528));
        else  /*led on*/
            writel(readl(IO_TYPE_CAST(0xb8003528)) & ~BIT(1), IO_TYPE_CAST(0xb8003528)); 
#else

        if(flag) /* led off*/
            gpio_set_value(BSP_GPIO_PIN_E1, 1);
        else  /*led on*/
            gpio_set_value(BSP_GPIO_PIN_E1, 0);   
#endif
    }
    #endif
    #if defined(CONFIG_WLAN_HAL_8822BE) 
    else if(GET_CHIP_VER(priv) == VERSION_8822B) {
        if(flag) /* led off*/
            RTL_W32(0x60, (RTL_R32(0x60) & 0xFFFFFEFF) | BIT8);
        else  /*led on*/
            RTL_W32(0x60, (RTL_R32(0x60) & 0xFFFFFEFF));   
    }
    #endif    
    else {
        if (flag)
            RTL_W32(LEDCFG, (RTL_R32(LEDCFG) & 0xfffffff0) | LED0SV);
        else
            RTL_W32(LEDCFG, RTL_R32(LEDCFG) & 0xfffffff0);
    }
#endif
}


static void set_sw_LED1(struct rtl8192cd_priv *priv, int flag)
{
#ifdef RTL8190_SWGPIO_LED
	if (LED_ROUTE)
		set_swGpio_LED(priv, 1, flag);
#elif defined(CONFIG_RTL8672) || defined(NOT_RTK_BSP)
	if (flag)
		RTL_W32(LEDCFG, (RTL_R32(LEDCFG) & 0xfffff0ff) | LED1SV);
	else
		RTL_W32(LEDCFG, RTL_R32(LEDCFG) & 0xfffff0ff);
#else
	if ((GET_CHIP_VER(priv) == VERSION_8188E)||(GET_CHIP_VER(priv) == VERSION_8192E)) {
#ifdef RTLWIFINIC_GPIO_CONTROL
		if (flag)
			RTLWIFINIC_GPIO_write_proc(priv, 5, 0);
		else
			RTLWIFINIC_GPIO_write_proc(priv, 5, 1);
#endif		
	}
#if defined(CONFIG_RTL_92D_SUPPORT)
	else if (GET_CHIP_VER(priv) == VERSION_8192D) {
		if (flag)
			RTL_W32(LEDCFG, (RTL_R32(LEDCFG) & 0xfffff0ff) | LED1SV_92D);
		else
			RTL_W32(LEDCFG, RTL_R32(LEDCFG) & 0xfffff0ff);
	}
#endif	
	else if (GET_CHIP_VER(priv) == VERSION_8192C){
		if (flag)
			RTL_W32(LEDCFG, (RTL_R32(LEDCFG) & 0xfffff0ff) | LED1SV);
		else
			RTL_W32(LEDCFG, RTL_R32(LEDCFG) & 0xfffff0ff);
	}
#endif
}


static void set_sw_LED2(struct rtl8192cd_priv *priv, int flag)
{
#ifdef RTL8190_SWGPIO_LED
	if (LED_ROUTE)
		set_swGpio_LED(priv, 2, flag);
#elif defined(CONFIG_RTL8672) || defined(NOT_RTK_BSP)
	if (flag)
		RTL_W32(LEDCFG, (RTL_R32(LEDCFG) & 0xfff0ffff) | LED2SV);
	else
		RTL_W32(LEDCFG, RTL_R32(LEDCFG) & 0xfff0ffff);
#else

	if ((GET_CHIP_VER(priv) == VERSION_8188E)||(GET_CHIP_VER(priv) == VERSION_8192E)) {
#ifdef RTLWIFINIC_GPIO_CONTROL
		if (flag)
			RTLWIFINIC_GPIO_write_proc(priv, 5, 0);
		else
			RTLWIFINIC_GPIO_write_proc(priv, 5, 1);
#endif		
	}
#if defined(CONFIG_RTL_92D_SUPPORT)
	else if (GET_CHIP_VER(priv) == VERSION_8192D) {
		if (flag)
			RTL_W32(LEDCFG, (RTL_R32(LEDCFG) & 0xfff0ffff) | LED2SV_92D);
		else
			RTL_W32(LEDCFG, RTL_R32(LEDCFG) & 0xfff0ffff);
	}
#endif	
	else if (GET_CHIP_VER(priv) == VERSION_8192C){
		if (flag)
			RTL_W32(LEDCFG, (RTL_R32(LEDCFG) & 0xfff0ffff) | LED2SV);
		else
			RTL_W32(LEDCFG, RTL_R32(LEDCFG) & 0xfff0ffff);
	}
#endif
}

static void LED_Interval_timeout(unsigned long task_priv)
{
    struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
    int led_on_time= LED_ON_TIME;

    if (!(priv->drv_state & DRV_STATE_OPEN))
        return;

#ifdef PCIE_POWER_SAVING
    if ((priv->pwr_state != L1) && (priv->pwr_state != L2))
#elif defined(PCIE_POWER_SAVING_TEST) 
    if ((priv->pwr_state != L1) && (priv->pwr_state != L2))        
#endif
    {
        if (!priv->pshare->set_led_in_progress) {
#if defined(CONFIG_RTL8672) || defined(NOT_RTK_BSP)
            #ifdef CONFIG_RTL_88E_SUPPORT
            if (GET_CHIP_VER(priv) == VERSION_8188E)
                set_sw_LED2(priv, priv->pshare->LED_Toggle);
            else
            #endif
            #ifdef CONFIG_WLAN_HAL_8192EE
            if (GET_CHIP_VER(priv) == VERSION_8192E)
                set_sw_LED0(priv, priv->pshare->LED_Toggle);
            else
            #endif
            #if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8814AE)
            if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8814A)
                set_sw_LED1(priv, priv->pshare->LED_Toggle);
            else
            #endif
#endif // CONFIG_RTL8672 || NOT_RTK_BSP
            if ((LED_TYPE == LEDTYPE_SW_LINKTXRX) ||
                (LED_TYPE == LEDTYPE_SW_LINKTXRXDATA) ||
                (LED_TYPE == LEDTYPE_SW_ENABLETXRXDATA) ||
                ((LED_TYPE == LEDTYPE_SW_ADATA_GDATA) && (priv->pshare->curr_band == BAND_5G)) ||
                (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRX) ||
                (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_ENABLETXRXDATA) ||
                (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX) ||
                (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRXDATA) ||
                (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_ASOCTXRXDATA) ||
                (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_ENABLETXRXDATA) ||
                (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_ENABLETXRXDATA_92D) ||
                (LED_TYPE == LEDTYPE_SW_LED1_GPIO9_LINKTXRX_92D) ||
                (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX_92D))
            {
                if ((LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRX) ||
                    (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_ENABLETXRXDATA) ||
                    (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX) ||
                    (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRXDATA) ||
                    (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_ASOCTXRXDATA) ||
                    (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_ENABLETXRXDATA) ||
                    (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_ENABLETXRXDATA_92D) ||
                    (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX_92D))
                    set_sw_LED2(priv, priv->pshare->LED_Toggle);
                else if (LED_TYPE == LEDTYPE_SW_LED1_GPIO9_LINKTXRX_92D)
                    set_sw_LED1(priv, priv->pshare->LED_Toggle);
                else
                    set_sw_LED0(priv, priv->pshare->LED_Toggle);
            } else {
                set_sw_LED1(priv, priv->pshare->LED_Toggle);
            }
		}
	}

    if( (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_ASOCTXRXDATA) && 
        (!(OPMODE & WIFI_ASOC_STATE)))  //client not assco  , mark_led
    {		
        led_on_time = LED_NOBLINK_TIME;
    }

    if ( priv->pshare->LED_Toggle == priv->pshare->LED_ToggleStart) {
        mod_timer(&priv->pshare->LED_Timer, jiffies + priv->pshare->LED_Interval);
    } else {
        if 	(LED_TYPE == LEDTYPE_SW_CUSTOM1)
            mod_timer(&priv->pshare->LED_Timer, jiffies + priv->pshare->LED_Interval);
        else
            mod_timer(&priv->pshare->LED_Timer, jiffies + led_on_time); 
    }

    priv->pshare->LED_Toggle = (priv->pshare->LED_Toggle + 1) % 2;
}


void enable_sw_LED(struct rtl8192cd_priv *priv, int init)
{
#if defined(SUPPORT_UCFGING_LED) 

    if(init){
        if(LED_Configuring ==1)
            return;
    }
#endif	
#if (defined(HW_ANT_SWITCH) || defined(SW_ANT_SWITCH))&&( defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
    int b23 = RTL_R32(LEDCFG) & BIT(23);
#endif

    if (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_ASOCTXRXDATA) 
        if (!(OPMODE & WIFI_STATION_STATE)) // if it is not Client mode , then run orignal 12 type
            LED_TYPE = LEDTYPE_SW_LED2_GPIO8_ENABLETXRXDATA ;  
	     
	// configure mac to use SW LED
#if defined(CONFIG_RTL8672) || defined(NOT_RTK_BSP)
    #ifdef CONFIG_RTL_88E_SUPPORT
    if (GET_CHIP_VER(priv) == VERSION_8188E)
        RTL_W32(LEDCFG, (RTL_R32(LEDCFG)&0xFF00FFFF) | LED2EN | LED2SV);
    else
    #endif
    #ifdef CONFIG_WLAN_HAL_8192EE
    if (GET_CHIP_VER(priv) == VERSION_8192E)
        RTL_W32(LEDCFG, (RTL_R32(LEDCFG)&0xFFFFFF00) | LED0SV);
    else
    #endif
#else // !CONFIG_RTL8672 && !NOT_RTK_BSP

    #if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE) //mark_ecos	
    if ((GET_CHIP_VER(priv) == VERSION_8188E)||(GET_CHIP_VER(priv) == VERSION_8192E))
    {
        #ifdef RTLWIFINIC_GPIO_CONTROL
        RTLWIFINIC_GPIO_config_proc(priv, 5, 0x10);
        #endif
    }
    else 
    #endif
#endif // CONFIG_RTL8672 || NOT_RTK_BSP
    if (GET_CHIP_VER(priv) == VERSION_8812E)
        RTL_W32(LEDCFG, BIT(13) | LED1SV);
    else
    #if defined(CONFIG_WLAN_HAL_8881A)
    if (GET_CHIP_VER(priv) == VERSION_8881A) {
        writel(readl(IO_TYPE_CAST(0xb8000044)) | BIT(15) | BIT(16), IO_TYPE_CAST(0xb8000044));
        writel(readl(IO_TYPE_CAST(0xb8003500)) & ~BIT(24), IO_TYPE_CAST(0xb8003500));
        writel(readl(IO_TYPE_CAST(0xb8003508)) | BIT(24), IO_TYPE_CAST(0xb8003508));
        #ifndef __OSK__
        writel(readl(IO_TYPE_CAST(0xb800350c)) | BIT(24), IO_TYPE_CAST(0xb800350c));
        #else
        setWlan5gLed(LED_OFF);/*tony 20150203: we should use gpio api, avoid gpio abnormal behavior.*/
        #endif
    } else
    #endif
	#ifdef CONFIG_WLAN_HAL_8814AE
	if (GET_CHIP_VER(priv) == VERSION_8814A) {
		RTL_W8(LEDCFG+1, (RTL_R8(LEDCFG+1)&0x70) | BIT(3));
	} else
	#endif
    #if defined(CONFIG_RTL_8197F)&& !defined(__ECOS)
    if (GET_CHIP_VER(priv) == VERSION_8197F) {
	  int err;
	  #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
        //r = gpio_request_one(BSP_GPIO_PIN_E1, GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED, "2G Wifi");
        writel(readl(IO_TYPE_CAST(0xb8000844)) | BIT(28), IO_TYPE_CAST(0xb8000844));
        writel(readl(IO_TYPE_CAST(0xb8003524)) | BIT(1), IO_TYPE_CAST(0xb8003524));
	  #else
		if((err = gpio_request_one(BSP_GPIO_PIN_E1, GPIOF_OUT_INIT_HIGH, "2G Wifi")) == 0)
			err = gpio_export(BSP_GPIO_PIN_E1, false);
	  #endif
        if(err) {
            printk("gpio request error(%d)\n", err);
        }
    } else
    #endif
    #if defined(CONFIG_WLAN_HAL_8822BE)  
    if (GET_CHIP_VER(priv) == VERSION_8822B) {
        RTL_W32(LEDCFG, (RTL_R32(LEDCFG) & (~LED2EN)));
        RTL_W32(0x60, (RTL_R32(0x60) | BIT16 | BIT24));
    } else
    #endif    
	{
        if (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX)
            RTL_W32(LEDCFG, (RTL_R32(LEDCFG)&0xFF00FFFF) | LED2EN | LED2SV);
        else if (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_ENABLETXRXDATA)
            RTL_W32(LEDCFG, (RTL_R32(LEDCFG)&0xFF00FFFF) | LED2EN | LED2SV);
        #ifdef CONFIG_RTL_92D_SUPPORT
        else if ((LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX_92D) ||
                 (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_ENABLETXRXDATA_92D))
            RTL_W32(LEDCFG,(RTL_R32(LEDCFG)&0xFF00FFFF)| LED2DIS_92D | LED2SV_92D);
        else if (LED_TYPE == LEDTYPE_SW_LED1_GPIO9_LINKTXRX_92D)
            RTL_W32(LEDCFG, (RTL_R32(LEDCFG)&0xFFFF00FF)|LED1DIS_92D | LED1SV_92D); 	
        #endif
        else if ((LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRX) ||
                 (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_ENABLETXRXDATA) ||
                 (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_ASOCTXRXDATA) || 	
                 (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRXDATA))
            RTL_W32(LEDCFG, (RTL_R32(LEDCFG)&0xFF00FFFF) | GP8_LED | LED2EN | LED2SV);
        else
            RTL_W32(LEDCFG, LED2SV | LED1SV | LED0SV);
    }

    priv->pshare->LED_Interval = LED_INTERVAL_TIME;
    priv->pshare->LED_Toggle = 0;
    priv->pshare->LED_ToggleStart = LED_OFF;
    priv->pshare->LED_tx_cnt_log = 0;
    priv->pshare->LED_rx_cnt_log = 0;
    priv->pshare->LED_tx_cnt = 0;
    priv->pshare->LED_rx_cnt = 0;

    if ((LED_TYPE == LEDTYPE_SW_ENABLE_TXRXDATA) ||
        (LED_TYPE == LEDTYPE_SW_ENABLETXRXDATA)) {
        set_sw_LED0(priv, LED_ON);
        set_sw_LED1(priv, LED_OFF);

        if (LED_TYPE == LEDTYPE_SW_ENABLETXRXDATA)
         priv->pshare->LED_ToggleStart = LED_ON;
    } else if ((LED_TYPE == LEDTYPE_SW_LED2_GPIO10_ENABLETXRXDATA) ||
               (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_ENABLETXRXDATA_92D)) {
        set_sw_LED2(priv, LED_ON);
        priv->pshare->LED_ToggleStart = LED_ON;
    } else if (LED_TYPE == LEDTYPE_SW_ADATA_GDATA) {
        priv->pshare->LED_ToggleStart = LED_ON;
        if (priv->pshare->curr_band == BAND_5G) {
            set_sw_LED0(priv, LED_ON);
            set_sw_LED1(priv, LED_OFF);
        }
        else {	// 11G
            set_sw_LED0(priv, LED_OFF);
            set_sw_LED1(priv, LED_ON);
        }
    }
    else if (LED_TYPE == LEDTYPE_SW_ENABLETXRXDATA_1) {
        set_sw_LED0(priv, LED_OFF);
        set_sw_LED1(priv, LED_ON);
        priv->pshare->LED_ToggleStart = LED_ON;
    }
    else if ((LED_TYPE == LEDTYPE_SW_LED2_GPIO8_ENABLETXRXDATA) ||
             (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_ASOCTXRXDATA) ) {
        set_sw_LED2(priv, LED_ON);
        priv->pshare->LED_ToggleStart = LED_ON;
    }
    else if ((LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRX) ||
             (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX) ||
             (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRXDATA)) {
        set_sw_LED2(priv, LED_OFF);
    }
    else {
        set_sw_LED0(priv, LED_OFF);
        set_sw_LED1(priv, LED_OFF);
        set_sw_LED2(priv, LED_OFF);
    }

    #if (defined(HW_ANT_SWITCH) || defined(SW_ANT_SWITCH))&&( defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
    RTL_W32(LEDCFG, b23 | RTL_R32(LEDCFG));
    #endif

    if (init) {
        init_timer(&priv->pshare->LED_Timer);
        #if defined(CONFIG_PCI_HCI)
        priv->pshare->LED_Timer.data = (unsigned long) priv;
        priv->pshare->LED_Timer.function = &LED_Interval_timeout;
        #elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
        priv->pshare->LED_Timer.data = (unsigned long) &priv->pshare->LED_Timer_event;
        priv->pshare->LED_Timer.function = timer_event_timer_fn;
        INIT_TIMER_EVENT_ENTRY(&priv->pshare->LED_Timer_event,
        LED_Interval_timeout, (unsigned long)priv);
        #endif

        mod_timer(&priv->pshare->LED_Timer, jiffies + priv->pshare->LED_Interval);
    }
}

#if defined(SUPPORT_UCFGING_LED) 
void LED_Interval_timeout2(unsigned long task_priv)
{

#ifdef RTLWIFINIC_GPIO_CONTROL
		RTLWIFINIC_GPIO_write(5, LED_CFGING_Toggle);
#endif		
	mod_timer(&LED_TimerCFGING, jiffies + LED_UCFGING_TIME); 
	LED_CFGING_Toggle = (LED_CFGING_Toggle + 1) % 2;		
}

void StartCFGINGTimer(void )
{
	
	init_timer(&LED_TimerCFGING);
	LED_TimerCFGING.data = 0;
	LED_TimerCFGING.function = &LED_Interval_timeout2;
	LED_CFGING_Interval = LED_UCFGING_TIME;
	LED_CFGING_Toggle = 0;
	LED_CFGING_ToggleStart = LED_OFF;
	mod_timer(&LED_TimerCFGING, jiffies + LED_CFGING_Interval);
}
#endif

void disable_sw_LED(struct rtl8192cd_priv *priv)
{
    if ((LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRX) ||
        (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_ENABLETXRXDATA) ||
        (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX) ||
        (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRXDATA) ||
        (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_ASOCTXRXDATA)) {
        set_sw_LED2(priv, LED_OFF);
    } else {
        set_sw_LED0(priv, LED_OFF);
        set_sw_LED1(priv, LED_OFF);
        set_sw_LED2(priv, LED_OFF);
    }
    if (timer_pending(&priv->pshare->LED_Timer))
        del_timer_sync(&priv->pshare->LED_Timer);
}


void calculate_sw_LED_interval(struct rtl8192cd_priv *priv)
{
    unsigned int delta = 0;
    int i, scale_num=0;

    if (priv->pshare->set_led_in_progress)
        return;

#if defined(SUPPORT_UCFGING_LED) 
    if(LED_Configuring ==1)
        return;
#endif	

    if( (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_ASOCTXRXDATA) && 
         (!(OPMODE & WIFI_ASOC_STATE)))  //client not assco  , mark_led
    {        
        priv->pshare->LED_Interval = LED_NOBLINK_TIME; // force one second
        priv->pshare->LED_tx_cnt_log = priv->pshare->LED_tx_cnt; // sync tx/rx cnt
        priv->pshare->LED_rx_cnt_log = priv->pshare->LED_rx_cnt;      
        return ;
    }

    // calculate counter delta
    delta += UINT32_DIFF(priv->pshare->LED_tx_cnt, priv->pshare->LED_tx_cnt_log);
    delta += UINT32_DIFF(priv->pshare->LED_rx_cnt, priv->pshare->LED_rx_cnt_log);
    priv->pshare->LED_tx_cnt_log = priv->pshare->LED_tx_cnt;
    priv->pshare->LED_rx_cnt_log = priv->pshare->LED_rx_cnt;

    // update interval according to delta
    if (delta == 0) {
        if (LED_TYPE == LEDTYPE_SW_CUSTOM1) {
            if (priv->pshare->LED_Interval != RTL_SECONDS_TO_JIFFIES(1)) {
                priv->pshare->LED_Interval = RTL_SECONDS_TO_JIFFIES(1);
                mod_timer(&priv->pshare->LED_Timer, jiffies + priv->pshare->LED_Interval);
            }
        } else {
            if (priv->pshare->LED_Interval == LED_NOBLINK_TIME)
                mod_timer(&priv->pshare->LED_Timer, jiffies + priv->pshare->LED_Interval);
            else
                priv->pshare->LED_Interval = LED_NOBLINK_TIME;
        }
    } else {
        if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) ||
            (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A))
            scale_num = LED_MAX_PACKET_CNT_AG / LED_MAX_SCALE;
        else
            scale_num = LED_MAX_PACKET_CNT_B / LED_MAX_SCALE;

        if ((LED_TYPE == LEDTYPE_SW_LINK_TXRX) ||
            (LED_TYPE == LEDTYPE_SW_LINKTXRX) ||
            (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRX) ||
            (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX) ||
            (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRXDATA) ||
            (LED_TYPE == LEDTYPE_SW_LED1_GPIO9_LINKTXRX_92D) ||
            (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX_92D) ||
            (LED_TYPE == LEDTYPE_SW_CUSTOM1))
            scale_num = scale_num*2;

        for (i=1; i<=LED_MAX_SCALE; i++) {
            if (delta < i*scale_num)
                break;
        }

        if (priv->pshare->rf_ft_var.ledBlinkingFreq > 1) {
            i = i*priv->pshare->rf_ft_var.ledBlinkingFreq;
            if (i > LED_MAX_SCALE)
                i = LED_MAX_SCALE;
        }

        priv->pshare->LED_Interval = ((LED_MAX_SCALE-i+1)*LED_INTERVAL_TIME)/LED_MAX_SCALE;

        if (priv->pshare->LED_Interval < LED_ON_TIME)
            priv->pshare->LED_Interval = LED_ON_TIME;
    }

    if ((LED_TYPE == LEDTYPE_SW_LINKTXRX) ||
        (LED_TYPE == LEDTYPE_SW_LINKTXRXDATA) ||
        (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRX) ||
        (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX) ||
        (LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRXDATA) ||
        (LED_TYPE == LEDTYPE_SW_LED1_GPIO9_LINKTXRX_92D) ||
        (LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX_92D)) {
        if (priv->link_status)
            priv->pshare->LED_ToggleStart = LED_ON;
        else
            priv->pshare->LED_ToggleStart = LED_OFF;
    } else {
        if (priv->pshare->set_led_in_progress)
            return;

        if ((LED_TYPE == LEDTYPE_SW_LINK_TXRX) ||
            (LED_TYPE == LEDTYPE_SW_LINK_TXRXDATA)) {
            if (priv->link_status)
                set_sw_LED0(priv, LED_ON);
            else
                set_sw_LED0(priv, LED_OFF);
        } else if (LED_TYPE == LEDTYPE_SW_ADATA_GDATA) {
            if (priv->pshare->curr_band == BAND_5G) {
                set_sw_LED0(priv, LED_ON);
                set_sw_LED1(priv, LED_OFF);
            } else {	// 11A
                set_sw_LED0(priv, LED_OFF);
                set_sw_LED1(priv, LED_ON);
            }
        }
    }
}

#if 0

void set_wireless_LED_steady_on(int led_num, struct net_device *dev)
{
	struct rtl8192cd_priv *priv;

	if (led_num != LED_0 && led_num != LED_1 && led_num != LED_2)
		return;

#ifdef NETDEV_NO_PRIV
	if (dev == NULL || netdev_priv(dev) == NULL)
		return;

	priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
	if (dev == NULL || dev->priv == NULL)
		return;

	priv = (struct rtl8192cd_priv *)dev->priv;
#endif


	if (priv->pshare == NULL)
		return;

	priv->pshare->set_led_in_progress = 1;

	if ((LED_TYPE >= LEDTYPE_HW_TX_RX) && (LED_TYPE <= LEDTYPE_HW_LINKACT_INFRA)) {
		enable_sw_LED(priv, 0);
	}
	else if ((LED_TYPE >= LEDTYPE_SW_LINK_TXRX) && (LED_TYPE < LEDTYPE_SW_MAX)) {
		if (timer_pending(&priv->pshare->LED_Timer))
			del_timer_sync(&priv->pshare->LED_Timer);
	}

	if (led_num == LED_0)
		set_sw_LED0(priv, LED_ON);
	else if (led_num == LED_1)
		set_sw_LED1(priv, LED_ON);
	else
		set_sw_LED2(priv, LED_ON);
}


void recover_wireless_LED(struct net_device *dev)
{
	struct rtl8192cd_priv *priv;

#ifdef NETDEV_NO_PRIV
	if (dev == NULL || netdev_priv(dev) == NULL)
		return;
	
	priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
	if (dev == NULL || dev->priv == NULL)
		return;

	priv = (struct rtl8192cd_priv *)dev->priv;
#endif

	if (!priv->pshare->set_led_in_progress)
		return;

	// for HW/SW LED
	if ((LED_TYPE >= LEDTYPE_HW_TX_RX) && (LED_TYPE <= LEDTYPE_HW_LINKACT_INFRA)) {
		set_sw_LED0(priv, LED_OFF);
		set_sw_LED1(priv, LED_OFF);
		set_sw_LED2(priv, LED_OFF);
		enable_hw_LED(priv, LED_TYPE);
	}
	else if ((LED_TYPE >= LEDTYPE_SW_LINK_TXRX) && (LED_TYPE < LEDTYPE_SW_MAX)) {
		enable_sw_LED(priv, 0);
		mod_timer(&priv->pshare->LED_Timer, jiffies + priv->pshare->LED_Interval);
	}

	priv->pshare->set_led_in_progress = 0;
}
#endif


void control_wireless_led(struct rtl8192cd_priv *priv, int enable)
{
	if (enable == 0) {
		priv->pshare->set_led_in_progress = 1;
		set_sw_LED0(priv, LED_OFF);
		set_sw_LED1(priv, LED_OFF);
		set_sw_LED2(priv, LED_OFF);
	}
	else if (enable == 1) {
		priv->pshare->set_led_in_progress = 1;
		set_sw_LED0(priv, LED_ON);
		set_sw_LED1(priv, LED_ON);
		set_sw_LED2(priv, LED_ON);
	}
	else if (enable == 2) {
		set_sw_LED0(priv, priv->pshare->LED_ToggleStart);
		set_sw_LED1(priv, priv->pshare->LED_ToggleStart);
		set_sw_LED2(priv, priv->pshare->LED_ToggleStart);
		priv->pshare->set_led_in_progress = 0;
	}
#if defined(__ECOS) && (defined(CONFIG_CUTE_MAHJONG) || defined(CONFIG_RTL_ULINKER))
	else if (enable == 3) {
		unsigned int saved_led_in_progress=priv->pshare->set_led_in_progress;
		priv->pshare->set_led_in_progress = 1;
		
		writel(readl(IO_TYPE_CAST(0xb8000044)) & (~(BIT(15) | BIT(16))), IO_TYPE_CAST(0xb8000044));
	
		//switch to hw led
		priv->pshare->set_led_in_progress = saved_led_in_progress;		
	}
	else if (enable == 4) {
		unsigned int saved_led_in_progress=priv->pshare->set_led_in_progress; 
		priv->pshare->set_led_in_progress = 1;
		writel(readl(IO_TYPE_CAST(0xb8000044)) | BIT(15) | BIT(16), IO_TYPE_CAST(0xb8000044));
		writel(readl(IO_TYPE_CAST(0xb8003500)) & (~BIT(24)), IO_TYPE_CAST(0xb8003500));	
 		writel(readl(IO_TYPE_CAST(0xb8003508)) | BIT(24), IO_TYPE_CAST(0xb8003508)); 
		//switch to sw led
		priv->pshare->set_led_in_progress = saved_led_in_progress;
	}			
#endif
}


#ifdef CONFIG_RTL_ULINKER
static struct rtl8192cd_priv *root_priv = NULL;

void enable_sys_LED(struct rtl8192cd_priv *priv)
{
#ifdef RTLWIFINIC_GPIO_CONTROL
	RTLWIFINIC_GPIO_config(4, 0x10);
#endif
	root_priv = priv;
}


void renable_sw_LED(void)
{
	struct rtl8192cd_priv *priv = root_priv;

	if ((LED_TYPE >= LEDTYPE_SW_LINK_TXRX) && (LED_TYPE < LEDTYPE_SW_MAX)) {
		priv->pshare->set_led_in_progress = 1;
		disable_sw_LED(priv);
		priv->pshare->set_led_in_progress = 0;
		enable_sw_LED(priv, 1);
	}
}

#endif /* #ifdef CONFIG_RTL_ULINKER */

