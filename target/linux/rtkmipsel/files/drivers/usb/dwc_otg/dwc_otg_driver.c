/* ==========================================================================
 * $File: //dwh/usb_iip/dev/software/otg_ipmate/linux/drivers/dwc_otg_driver.c $
 * $Revision: 1.19 $
 * $Date: 2010/09/14 13:01:10 $
 * $Change: 791271 $
 *
 * Synopsys HS OTG Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * ========================================================================== */

/** @file
 * The dwc_otg_driver module provides the initialization and cleanup entry
 * points for the DWC_otg driver. This module will be dynamically installed
 * after Linux is booted using the insmod command. When the module is
 * installed, the dwc_otg_driver_init function is called. When the module is
 * removed (using rmmod), the dwc_otg_driver_cleanup function is called.
 *
 * This module also defines a data structure for the dwc_otg_driver, which is
 * used in conjunction with the standard ARM lm_device structure. These
 * structures allow the OTG driver to comply with the standard Linux driver
 * model in which devices and drivers are registered with a bus driver. This
 * has the benefit that Linux can expose attributes of the driver and device
 * in its special sysfs file system. Users can then read or write files in
 * this file system to perform diagnostics on the driver components or the
 * device.
 */

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
//#include <asm/arch/lm.h>	//cathy0823
#include "lm.h"
//#include <asm/sizes.h>	//cathy0823
//#include <asm-arm/sizes.h>

#include "dwc_otg_plat.h"
#include "dwc_otg_attr.h"
#include "dwc_otg_driver.h"
//#include "dwc_otg_cil.h"	//cathy
#include "dwc_otg_pcd.h"
#include "dwc_otg_hcd.h"

#define DWC_DRIVER_VERSION	"2.60a 22-NOV-2006"
#define DWC_DRIVER_DESC		"HS OTG USB Controller driver"

static const char dwc_driver_name[] = "dwc_otg";
struct lm_device *glmdev; //wei add

/*-------------------------------------------------------------------------*/
/* Encapsulate the module parameter settings */
#ifdef CONFIG_USB_RTL8187SU_SOFTAP
dwc_otg_core_params_t dwc_otg_module_params = {
#else
static dwc_otg_core_params_t dwc_otg_module_params = {
#endif
	.opt = dwc_param_opt_default,
	.otg_cap = DWC_OTG_CAP_PARAM_NO_HNP_SRP_CAPABLE,	//no HNP, SRP
	.dma_enable = dwc_param_dma_enable_default,	//dma enable(RTL8672 uses internal DMA mode)
	.dma_burst_size = -1,	//(used for external DMA mode)
#ifdef CONFIG_USB_RTL8187SU_SOFTAP
	.speed = DWC_SPEED_PARAM_FULL,
#else
	.speed = DWC_SPEED_PARAM_HIGH,	//high speed
#endif
	.host_support_fs_ls_low_power = dwc_param_host_support_fs_ls_low_power_default,	//not support low power
	.host_ls_low_power_phy_clk = dwc_param_host_ls_low_power_phy_clk_default,	//??
	.enable_dynamic_fifo = dwc_param_enable_dynamic_fifo_default,	//dynamic fifo sizing
	.data_fifo_size = 1024,
	.dev_rx_fifo_size = 512,
	.dev_nperio_tx_fifo_size = 256,
	.dev_perio_tx_fifo_size =
	{	/* dev_perio_tx_fifo_size_1 */
			256,
	/*		-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1*/
	},	/* 15 */
	.host_rx_fifo_size = 512,
	.host_nperio_tx_fifo_size = 256,
	.host_perio_tx_fifo_size = 256,
	.max_transfer_size = -1,	//default 65535
	.max_packet_count = -1,	//default 511
	.host_channels = 5, //shared with otg device, ep0-in, ep0-out, ep1-in, ep2-out, ep3-in
	.dev_endpoints = 3,	// 1:bulk in 2:bulk out 3: interrupt
	.phy_type = DWC_PHY_TYPE_PARAM_UTMI,	//UTMI PHY
	.phy_utmi_width = 16,
	.phy_ulpi_ddr = -1,
	.phy_ulpi_ext_vbus = -1,
	.i2c_enable = -1,
	.ulpi_fs_ls = -1,
	.ts_dline = -1,
	.en_multiple_tx_fifo = 0,	//shared FIFO
	.dev_tx_fifo_size =
	{	/* dev_tx_fifo_size */
			512,
	/*		-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1*/
	},	/* 15 */
	.thr_ctl = -1,
	.tx_thr_length = -1,
	.rx_thr_length = -1,
};

/**
 * This function shows the Driver Version.
 */
static ssize_t version_show(struct device_driver *dev, char *buf)
{
	return snprintf(buf, sizeof(DWC_DRIVER_VERSION)+2,"%s\n",
		DWC_DRIVER_VERSION);
}
static DRIVER_ATTR(version, S_IRUGO, version_show, NULL);

/**
 * Global Debug Level Mask.
 */
uint32_t g_dbg_lvl = 0; /* OFF */

//wei add
int gHostMode=1;

/**
 * This function shows the driver Debug Level.
 */
static ssize_t dbg_level_show(struct device_driver *_drv, char *_buf)
{
	return sprintf(_buf, "0x%0x\n", g_dbg_lvl);
}
/**
 * This function stores the driver Debug Level.
 */
static ssize_t dbg_level_store(struct device_driver *_drv, const char *_buf,
							   size_t _count)
{
	g_dbg_lvl = simple_strtoul(_buf, NULL, 16);
		return _count;
}
static DRIVER_ATTR(debuglevel, S_IRUGO|S_IWUSR, dbg_level_show, dbg_level_store);


/**
 * This function is called during module intialization to verify that
 * the module parameters are in a valid state.
 */
static int check_parameters(dwc_otg_core_if_t *core_if)
{
	int i;
	int retval = 0;

/* Checks if the parameter is outside of its valid range of values */
#define DWC_OTG_PARAM_TEST(_param_,_low_,_high_) \
		((dwc_otg_module_params._param_ < (_low_)) || \
		(dwc_otg_module_params._param_ > (_high_)))

/* If the parameter has been set by the user, check that the parameter value is
 * within the value range of values.  If not, report a module error. */
#define DWC_OTG_PARAM_ERR(_param_,_low_,_high_,_string_) \
		do { \
			if (dwc_otg_module_params._param_ != -1) { \
				if (DWC_OTG_PARAM_TEST(_param_,(_low_),(_high_))) { \
					DWC_ERROR("`%d' invalid for parameter `%s'\n", \
						dwc_otg_module_params._param_, _string_); \
					dwc_otg_module_params._param_ = dwc_param_##_param_##_default; \
					retval ++; \
				} \
			} \
		} while (0)
//cathy, disable host parameter check
	DWC_OTG_PARAM_ERR(opt,0,1,"opt");
	DWC_OTG_PARAM_ERR(otg_cap,0,2,"otg_cap");
	DWC_OTG_PARAM_ERR(dma_enable,0,1,"dma_enable");
	DWC_OTG_PARAM_ERR(speed,0,1,"speed");
	//DWC_OTG_PARAM_ERR(host_support_fs_ls_low_power,0,1,"host_support_fs_ls_low_power");
	//DWC_OTG_PARAM_ERR(host_ls_low_power_phy_clk,0,1,"host_ls_low_power_phy_clk");
	DWC_OTG_PARAM_ERR(enable_dynamic_fifo,0,1,"enable_dynamic_fifo");
	DWC_OTG_PARAM_ERR(data_fifo_size,32,32768,"data_fifo_size");
	DWC_OTG_PARAM_ERR(dev_rx_fifo_size,16,32768,"dev_rx_fifo_size");
	DWC_OTG_PARAM_ERR(dev_nperio_tx_fifo_size,16,32768,"dev_nperio_tx_fifo_size");
	//DWC_OTG_PARAM_ERR(host_rx_fifo_size,16,32768,"host_rx_fifo_size");
	//DWC_OTG_PARAM_ERR(host_nperio_tx_fifo_size,16,32768,"host_nperio_tx_fifo_size");
	//DWC_OTG_PARAM_ERR(host_perio_tx_fifo_size,16,32768,"host_perio_tx_fifo_size");
	DWC_OTG_PARAM_ERR(max_transfer_size,2047,524288,"max_transfer_size");
	DWC_OTG_PARAM_ERR(max_packet_count,15,511,"max_packet_count");
	//DWC_OTG_PARAM_ERR(host_channels,1,16,"host_channels");
	DWC_OTG_PARAM_ERR(dev_endpoints,1,15,"dev_endpoints");
	DWC_OTG_PARAM_ERR(phy_type,0,2,"phy_type");
	DWC_OTG_PARAM_ERR(phy_ulpi_ddr,0,1,"phy_ulpi_ddr");
	DWC_OTG_PARAM_ERR(phy_ulpi_ext_vbus,0,1,"phy_ulpi_ext_vbus");
	DWC_OTG_PARAM_ERR(i2c_enable,0,1,"i2c_enable");
	DWC_OTG_PARAM_ERR(ulpi_fs_ls,0,1,"ulpi_fs_ls");
	DWC_OTG_PARAM_ERR(ts_dline,0,1,"ts_dline");

	if (dwc_otg_module_params.dma_burst_size != -1)
	{
		if (DWC_OTG_PARAM_TEST(dma_burst_size,1,1) &&
			DWC_OTG_PARAM_TEST(dma_burst_size,4,4) &&
			DWC_OTG_PARAM_TEST(dma_burst_size,8,8) &&
			DWC_OTG_PARAM_TEST(dma_burst_size,16,16) &&
			DWC_OTG_PARAM_TEST(dma_burst_size,32,32) &&
			DWC_OTG_PARAM_TEST(dma_burst_size,64,64) &&
			DWC_OTG_PARAM_TEST(dma_burst_size,128,128) &&
			DWC_OTG_PARAM_TEST(dma_burst_size,256,256))
		{
			DWC_ERROR("`%d' invalid for parameter `dma_burst_size'\n",
				  dwc_otg_module_params.dma_burst_size);
			dwc_otg_module_params.dma_burst_size = 32;
			retval ++;
		}
	}

	if (dwc_otg_module_params.phy_utmi_width != -1)
	{
		if (DWC_OTG_PARAM_TEST(phy_utmi_width,8,8) &&
			DWC_OTG_PARAM_TEST(phy_utmi_width,16,16))
		{
			DWC_ERROR("`%d' invalid for parameter `phy_utmi_width'\n",
				  dwc_otg_module_params.phy_utmi_width);
			dwc_otg_module_params.phy_utmi_width = 16;
			retval ++;
		}
	}
#if 0
	for (i=0; i<15; i++)
	{
		/** @todo should be like above */
		//DWC_OTG_PARAM_ERR(dev_perio_tx_fifo_size[i],4,768,"dev_perio_tx_fifo_size");
		if (dwc_otg_module_params.dev_perio_tx_fifo_size[i] != -1)
		{
			if (DWC_OTG_PARAM_TEST(dev_perio_tx_fifo_size[i],4,768))
			{
				DWC_ERROR("`%d' invalid for parameter `%s_%d'\n",
					  dwc_otg_module_params.dev_perio_tx_fifo_size[i], "dev_perio_tx_fifo_size", i);
				dwc_otg_module_params.dev_perio_tx_fifo_size[i] = dwc_param_dev_perio_tx_fifo_size_default;
				retval ++;
			}
		}
	}
#endif
	DWC_OTG_PARAM_ERR(en_multiple_tx_fifo,0,1,"en_multiple_tx_fifo");
#if 0
	for (i=0; i<15; i++)
	{
		/** @todo should be like above */
		//DWC_OTG_PARAM_ERR(dev_tx_fifo_size[i],4,768,"dev_tx_fifo_size");
		if (dwc_otg_module_params.dev_tx_fifo_size[i] != -1)
		{
			if (DWC_OTG_PARAM_TEST(dev_tx_fifo_size[i],4,768))
			{
				DWC_ERROR("`%d' invalid for parameter `%s_%d'\n",
					  dwc_otg_module_params.dev_tx_fifo_size[i], "dev_tx_fifo_size", i);
				dwc_otg_module_params.dev_tx_fifo_size[i] = dwc_param_dev_tx_fifo_size_default;
				retval ++;
			}
		}
	}
#endif
	DWC_OTG_PARAM_ERR(thr_ctl, 0, 7, "thr_ctl");
	DWC_OTG_PARAM_ERR(tx_thr_length, 8, 128, "tx_thr_length");
	DWC_OTG_PARAM_ERR(rx_thr_length, 8, 128, "rx_thr_length");


	/* At this point, all module parameters that have been set by the user
	 * are valid, and those that have not are left unset.  Now set their
	 * default values and/or check the parameters against the hardware
	 * configurations of the OTG core. */



/* This sets the parameter to the default value if it has not been set by the
 * user */
#define DWC_OTG_PARAM_SET_DEFAULT(_param_) \
	({ \
		int changed = 1; \
		if (dwc_otg_module_params._param_ == -1) { \
			changed = 0; \
			dwc_otg_module_params._param_ = dwc_param_##_param_##_default; \
		} \
		changed; \
	})

/* This checks the macro agains the hardware configuration to see if it is
 * valid.  It is possible that the default value could be invalid.	In this
 * case, it will report a module error if the user touched the parameter.
 * Otherwise it will adjust the value without any error. */
#define DWC_OTG_PARAM_CHECK_VALID(_param_,_str_,_is_valid_,_set_valid_) \
	({ \
			int changed = DWC_OTG_PARAM_SET_DEFAULT(_param_); \
		int error = 0; \
		if (!(_is_valid_)) { \
			if (changed) { \
				DWC_ERROR("`%d' invalid for parameter `%s'.	 Check HW configuration.\n", dwc_otg_module_params._param_,_str_); \
				error = 1; \
			} \
			dwc_otg_module_params._param_ = (_set_valid_); \
		} \
		error; \
	})

	/* OTG Cap */
	retval += DWC_OTG_PARAM_CHECK_VALID(otg_cap,"otg_cap",
				  ({
					  int valid;
					  valid = 1;
					  switch (dwc_otg_module_params.otg_cap) {
					  case DWC_OTG_CAP_PARAM_HNP_SRP_CAPABLE:
						  if (core_if->hwcfg2.b.op_mode != DWC_HWCFG2_OP_MODE_HNP_SRP_CAPABLE_OTG) valid = 0;
						  break;
					  case DWC_OTG_CAP_PARAM_SRP_ONLY_CAPABLE:
						  if ((core_if->hwcfg2.b.op_mode != DWC_HWCFG2_OP_MODE_HNP_SRP_CAPABLE_OTG) &&
							  (core_if->hwcfg2.b.op_mode != DWC_HWCFG2_OP_MODE_SRP_ONLY_CAPABLE_OTG) &&
							  (core_if->hwcfg2.b.op_mode != DWC_HWCFG2_OP_MODE_SRP_CAPABLE_DEVICE) &&
							  (core_if->hwcfg2.b.op_mode != DWC_HWCFG2_OP_MODE_SRP_CAPABLE_HOST))
						  {
							  valid = 0;
						  }
						  break;
					  case DWC_OTG_CAP_PARAM_NO_HNP_SRP_CAPABLE:
						  /* always valid */
						  break;
					  }
					  valid;
			  }),
					(((core_if->hwcfg2.b.op_mode == DWC_HWCFG2_OP_MODE_HNP_SRP_CAPABLE_OTG) ||
					(core_if->hwcfg2.b.op_mode == DWC_HWCFG2_OP_MODE_SRP_ONLY_CAPABLE_OTG) ||
					(core_if->hwcfg2.b.op_mode == DWC_HWCFG2_OP_MODE_SRP_CAPABLE_DEVICE) ||
					(core_if->hwcfg2.b.op_mode == DWC_HWCFG2_OP_MODE_SRP_CAPABLE_HOST)) ?
					DWC_OTG_CAP_PARAM_SRP_ONLY_CAPABLE :
					DWC_OTG_CAP_PARAM_NO_HNP_SRP_CAPABLE));

	retval += DWC_OTG_PARAM_CHECK_VALID(dma_enable,"dma_enable",
				((dwc_otg_module_params.dma_enable == 1) && (core_if->hwcfg2.b.architecture == 0)) ? 0 : 1,
				0);

	retval += DWC_OTG_PARAM_CHECK_VALID(opt,"opt",
				1,
				0);

	DWC_OTG_PARAM_SET_DEFAULT(dma_burst_size);

	retval += DWC_OTG_PARAM_CHECK_VALID(host_support_fs_ls_low_power,
				"host_support_fs_ls_low_power",
				1, 0);

	retval += DWC_OTG_PARAM_CHECK_VALID(enable_dynamic_fifo,
					"enable_dynamic_fifo",
					((dwc_otg_module_params.enable_dynamic_fifo == 0) ||
					(core_if->hwcfg2.b.dynamic_fifo == 1)), 0);


	retval += DWC_OTG_PARAM_CHECK_VALID(data_fifo_size,
					"data_fifo_size",
					(dwc_otg_module_params.data_fifo_size <= core_if->hwcfg3.b.dfifo_depth),
					core_if->hwcfg3.b.dfifo_depth);

	retval += DWC_OTG_PARAM_CHECK_VALID(dev_rx_fifo_size,
					"dev_rx_fifo_size",
					(dwc_otg_module_params.dev_rx_fifo_size <= dwc_read_reg32(&core_if->core_global_regs->grxfsiz)),
					dwc_read_reg32(&core_if->core_global_regs->grxfsiz));

	retval += DWC_OTG_PARAM_CHECK_VALID(dev_nperio_tx_fifo_size,
					"dev_nperio_tx_fifo_size",
					(dwc_otg_module_params.dev_nperio_tx_fifo_size <= (dwc_read_reg32(&core_if->core_global_regs->gnptxfsiz) >> 16)),
					(dwc_read_reg32(&core_if->core_global_regs->gnptxfsiz) >> 16));

	retval += DWC_OTG_PARAM_CHECK_VALID(host_rx_fifo_size,
					"host_rx_fifo_size",
					(dwc_otg_module_params.host_rx_fifo_size <= dwc_read_reg32(&core_if->core_global_regs->grxfsiz)),
					dwc_read_reg32(&core_if->core_global_regs->grxfsiz));


	retval += DWC_OTG_PARAM_CHECK_VALID(host_nperio_tx_fifo_size,
					"host_nperio_tx_fifo_size",
					(dwc_otg_module_params.host_nperio_tx_fifo_size <= (dwc_read_reg32(&core_if->core_global_regs->gnptxfsiz) >> 16)),
					(dwc_read_reg32(&core_if->core_global_regs->gnptxfsiz) >> 16));

	retval += DWC_OTG_PARAM_CHECK_VALID(host_perio_tx_fifo_size,
					"host_perio_tx_fifo_size",
					(dwc_otg_module_params.host_perio_tx_fifo_size <= ((dwc_read_reg32(&core_if->core_global_regs->hptxfsiz) >> 16))),
					((dwc_read_reg32(&core_if->core_global_regs->hptxfsiz) >> 16)));

	retval += DWC_OTG_PARAM_CHECK_VALID(max_transfer_size,
					"max_transfer_size",
					(dwc_otg_module_params.max_transfer_size < (1 << (core_if->hwcfg3.b.xfer_size_cntr_width + 11))),
					((1 << (core_if->hwcfg3.b.xfer_size_cntr_width + 11)) - 1));

	retval += DWC_OTG_PARAM_CHECK_VALID(max_packet_count,
					"max_packet_count",
					(dwc_otg_module_params.max_packet_count < (1 << (core_if->hwcfg3.b.packet_size_cntr_width + 4))),
					((1 << (core_if->hwcfg3.b.packet_size_cntr_width + 4)) - 1));

	retval += DWC_OTG_PARAM_CHECK_VALID(host_channels,
					"host_channels",
					(dwc_otg_module_params.host_channels <= (core_if->hwcfg2.b.num_host_chan + 1)),
					(core_if->hwcfg2.b.num_host_chan + 1));

	retval += DWC_OTG_PARAM_CHECK_VALID(dev_endpoints,
					"dev_endpoints",
					(dwc_otg_module_params.dev_endpoints <= (core_if->hwcfg2.b.num_dev_ep)),
					core_if->hwcfg2.b.num_dev_ep);

/*
 * Define the following to disable the FS PHY Hardware checking.  This is for
 * internal testing only.
 *
 * #define NO_FS_PHY_HW_CHECKS
 */

#ifdef NO_FS_PHY_HW_CHECKS
	retval += DWC_OTG_PARAM_CHECK_VALID(phy_type,
				"phy_type", 1, 0);
#else
	retval += DWC_OTG_PARAM_CHECK_VALID(phy_type,
				"phy_type",
				({
					int valid = 0;
					if ((dwc_otg_module_params.phy_type == DWC_PHY_TYPE_PARAM_UTMI) &&
					((core_if->hwcfg2.b.hs_phy_type == 1) ||
					 (core_if->hwcfg2.b.hs_phy_type == 3)))
					{
						valid = 1;
					}
					else if ((dwc_otg_module_params.phy_type == DWC_PHY_TYPE_PARAM_ULPI) &&
						 ((core_if->hwcfg2.b.hs_phy_type == 2) ||
						  (core_if->hwcfg2.b.hs_phy_type == 3)))
					{
						valid = 1;
					}
					else if ((dwc_otg_module_params.phy_type == DWC_PHY_TYPE_PARAM_FS) &&
						 (core_if->hwcfg2.b.fs_phy_type == 1))
					{
						valid = 1;
					}
					valid;
				}),
				({
					int set = DWC_PHY_TYPE_PARAM_FS;
					if (core_if->hwcfg2.b.hs_phy_type) {
						if ((core_if->hwcfg2.b.hs_phy_type == 3) ||
						(core_if->hwcfg2.b.hs_phy_type == 1)) {
							set = DWC_PHY_TYPE_PARAM_UTMI;
						}
						else {
							set = DWC_PHY_TYPE_PARAM_ULPI;
						}
					}
					set;
				}));
#endif

	retval += DWC_OTG_PARAM_CHECK_VALID(speed,"speed",
				(dwc_otg_module_params.speed == 0) && (dwc_otg_module_params.phy_type == DWC_PHY_TYPE_PARAM_FS) ? 0 : 1,
				dwc_otg_module_params.phy_type == DWC_PHY_TYPE_PARAM_FS ? 1 : 0);

	retval += DWC_OTG_PARAM_CHECK_VALID(host_ls_low_power_phy_clk,
				"host_ls_low_power_phy_clk",
				((dwc_otg_module_params.host_ls_low_power_phy_clk == DWC_HOST_LS_LOW_POWER_PHY_CLK_PARAM_48MHZ) && (dwc_otg_module_params.phy_type == DWC_PHY_TYPE_PARAM_FS) ? 0 : 1),
				((dwc_otg_module_params.phy_type == DWC_PHY_TYPE_PARAM_FS) ? DWC_HOST_LS_LOW_POWER_PHY_CLK_PARAM_6MHZ : DWC_HOST_LS_LOW_POWER_PHY_CLK_PARAM_48MHZ));

	DWC_OTG_PARAM_SET_DEFAULT(phy_ulpi_ddr);
	DWC_OTG_PARAM_SET_DEFAULT(phy_ulpi_ext_vbus);
	DWC_OTG_PARAM_SET_DEFAULT(phy_utmi_width);
	DWC_OTG_PARAM_SET_DEFAULT(ulpi_fs_ls);
	DWC_OTG_PARAM_SET_DEFAULT(ts_dline);

#ifdef NO_FS_PHY_HW_CHECKS
	retval += DWC_OTG_PARAM_CHECK_VALID(i2c_enable,
				"i2c_enable", 1, 0);
#else
	retval += DWC_OTG_PARAM_CHECK_VALID(i2c_enable,
				"i2c_enable",
				(dwc_otg_module_params.i2c_enable == 1) && (core_if->hwcfg3.b.i2c == 0) ? 0 : 1,
				0);
#endif

	for (i=0; i<15; i++)
	{
		int changed = 1;
		int error = 0;

		if (dwc_otg_module_params.dev_perio_tx_fifo_size[i] == -1)
		{
			changed = 0;
			dwc_otg_module_params.dev_perio_tx_fifo_size[i] = dwc_param_dev_perio_tx_fifo_size_default;
		}
		if (!(dwc_otg_module_params.dev_perio_tx_fifo_size[i] <= (dwc_read_reg32(&core_if->core_global_regs->dptxfsiz_dieptxf[i]))))
		{
			if (changed)
			{
				DWC_ERROR("`%d' invalid for parameter `dev_perio_fifo_size_%d'.	 Check HW configuration.\n", dwc_otg_module_params.dev_perio_tx_fifo_size[i],i);
				error = 1;
			}
			dwc_otg_module_params.dev_perio_tx_fifo_size[i] = dwc_read_reg32(&core_if->core_global_regs->dptxfsiz_dieptxf[i]);
		}
		retval += error;
	}


	retval += DWC_OTG_PARAM_CHECK_VALID(en_multiple_tx_fifo,"en_multiple_tx_fifo",
						((dwc_otg_module_params.en_multiple_tx_fifo == 1) && (core_if->hwcfg4.b.ded_fifo_en == 0)) ? 0 : 1,
						0);


	for (i=0; i<15; i++)
	{

		int changed = 1;
		int error = 0;

		if (dwc_otg_module_params.dev_tx_fifo_size[i] == -1)
		{
			changed = 0;
			dwc_otg_module_params.dev_tx_fifo_size[i] = dwc_param_dev_tx_fifo_size_default;
		}
		if (!(dwc_otg_module_params.dev_tx_fifo_size[i] <= (dwc_read_reg32(&core_if->core_global_regs->dptxfsiz_dieptxf[i]))))
		{
			if (changed)
			{
				DWC_ERROR("%d' invalid for parameter `dev_perio_fifo_size_%d'.	Check HW configuration.\n", dwc_otg_module_params.dev_tx_fifo_size[i],i);
				error = 1;
			}
			dwc_otg_module_params.dev_tx_fifo_size[i] = dwc_read_reg32(&core_if->core_global_regs->dptxfsiz_dieptxf[i]);
		}
		retval += error;


	}

	DWC_OTG_PARAM_SET_DEFAULT(thr_ctl);
	DWC_OTG_PARAM_SET_DEFAULT(tx_thr_length);
	DWC_OTG_PARAM_SET_DEFAULT(rx_thr_length);

	return retval;
}

/**
 * This function is the top level interrupt handler for the Common
 * (Device and host modes) interrupts.
 */
static irqreturn_t dwc_otg_common_irq(int _irq, void *_dev)
{
//	DWC_DEBUGPL("=============dwc_otg_driver : IRQ in\n"); //wei add
	dwc_otg_device_t *otg_dev = _dev;
	int32_t retval = IRQ_NONE;
	retval = dwc_otg_handle_common_intr( otg_dev->core_if );
	return IRQ_RETVAL(retval);
}

/**
 * This function is called when a lm_device is unregistered with the
 * dwc_otg_driver. This happens, for example, when the rmmod command is
 * executed. The device may or may not be electrically present. If it is
 * present, the driver stops device processing. Any resources used on behalf
 * of this device are freed.
 *
 * @param[in] _lmdev
 */

#if !DRIVER_USING_LM
static int dwc_otg_driver_remove(struct platform_device *pdev)
#else
static void dwc_otg_driver_remove(struct lm_device *pdev)
#endif
{
      //wei add, pdev= _lmdev
	dwc_otg_device_t *otg_dev = platform_get_drvdata(pdev);
	DWC_DEBUGPL(DBG_ANY, "%s(%p)\n", __func__, pdev);


	if (otg_dev == NULL)
	{
		/* Memory allocation for the dwc_otg_device failed. */
		return;
	}

	/*
	 * Free the IRQ
	 */
	if (otg_dev->common_irq_installed)
	{
#if !DRIVER_USING_LM
                free_irq(otg_dev->irq, otg_dev);
#else
		free_irq( pdev->irq, otg_dev );
#endif
	}

//#ifndef DWC_DEVICE_ONLY
if(gHostMode==1)
	if (otg_dev->hcd != NULL)
	{
                dwc_otg_hcd_remove(pdev);
	}
//#endif

//#ifndef DWC_HOST_ONLY
if(gHostMode==0)
	if (otg_dev->pcd != NULL)
	{
                dwc_otg_pcd_remove(pdev);
	}
//#endif
	if (otg_dev->core_if != NULL)
	{
		dwc_otg_cil_remove( otg_dev->core_if );
	}
#if 0	//cathy
	/*
	 * Remove the device attributes
	 */
	dwc_otg_attr_remove(pdev);
#endif
#if !DRIVER_USING_LM
	dwc_otg_attr_remove(pdev);
#endif
	/*
	 * Return the memory.
	 */
	if (otg_dev->base != NULL)
	{
		iounmap(otg_dev->base);
	}
	kfree(otg_dev);

	/*
	 * Clear the drvdata pointer.
	 */

	platform_set_drvdata(pdev, 0);

}


/**
 * This function is called when an lm_device is bound to a
 * dwc_otg_driver. It creates the driver components required to
 * control the device (CIL, HCD, and PCD) and it initializes the
 * device. The driver components are stored in a dwc_otg_device
 * structure. A reference to the dwc_otg_device is saved in the
 * lm_device. This allows the driver to access the dwc_otg_device
 * structure on subsequent calls to driver methods for this device.
 *
 * @param[in] _lmdev  lm_device definition
 */
 extern int request_irq(unsigned int, irq_handler_t handler,
		       unsigned long, const char *, void *);

#if !DRIVER_USING_LM
static int dwc_otg_driver_probe(struct platform_device *pdev)
#else
static int dwc_otg_driver_probe(struct lm_device *pdev)
#endif

{
       //_lmdev==pdev
	//printk("=>probe\n");
	int retval = 0;
	dwc_otg_device_t *otg_dev;
	int32_t snpsid;

	dev_dbg(&pdev->dev, "dwc_otg_driver_probe(%p)\n", pdev);
#if DRIVER_USING_LM
	dev_dbg(&pdev->dev, "start=0x%08x\n", (unsigned)pdev->resource.start);
#endif
	otg_dev = kmalloc(sizeof(dwc_otg_device_t), GFP_KERNEL);
	if (otg_dev == 0)
	{
		dev_err(&pdev->dev, "kmalloc of dwc_otg_device failed\n");
		retval = -ENOMEM;
		goto fail;
	}
	printk("  [S2]  otg_dev=%x\n", otg_dev);
	memset(otg_dev, 0, sizeof(*otg_dev));
	otg_dev->reg_offset = 0xFFFFFFFFUL;

#if !DRIVER_USING_LM
	#define OTG_BASE	0xb8030000
        #define USB_D_IRQ             11   //wei add ,11
	otg_dev->resource.start = OTG_BASE;		//base of OTG, 0xb8030000
	otg_dev->resource.end = otg_dev->resource.start + 0x0003ffff;
	otg_dev->resource.flags = IORESOURCE_MEM;
	otg_dev->irq = USB_D_IRQ;	//irq of usb device
	//otg_dev->id = 0;

  otg_dev->phys_addr = otg_dev->resource.start;
  otg_dev->base_len = otg_dev->resource.end - otg_dev->resource.start + 1;

  if (request_mem_region (otg_dev->phys_addr,
                          otg_dev->base_len,
                          dwc_driver_name) == NULL)
    {
      dev_err (&pdev->dev, "request_mem_region failed\n");
      retval = -EBUSY;
      goto fail;
    }
  otg_dev->base = ioremap(otg_dev->phys_addr, otg_dev->base_len);

#endif
#if DRIVER_USING_LM
	/*
	 * Map the DWC_otg Core memory into virtual address space.
	 */
//cathy, set base address
	//otg_dev->base = ioremap(pdev->resource.start, SZ_256K);
	otg_dev->base = (void *)pdev->resource.start;
#endif
	if (otg_dev->base == NULL)
	{
		dev_err(&pdev->dev, "ioremap() failed\n");
		retval = -ENOMEM;
		goto fail;
	}

	dev_dbg(&pdev->dev, "base=0x%08x\n", (unsigned)otg_dev->base);
	/*
	 * Attempt to ensure this device is really a DWC_otg Controller.
	 * Read and verify the SNPSID register contents. The value should be
	 * 0x45F42XXX, which corresponds to "OT2", as in "OTG version 2.XX".
	 */

	snpsid = dwc_read_reg32((uint32_t *)((uint8_t *)otg_dev->base + 0x40));

	if ((snpsid & 0xFFFFF000) != 0x4F542000)
	{
		dev_err(&pdev->dev, "Bad value for SNPSID: 0x%08x\n", snpsid);
		retval = -EINVAL;
		goto fail;
	}

	/*
	 * Initialize driver data to point to the global DWC_otg
	 * Device structure.
	 */
#if !DRIVER_USING_LM
  platform_set_drvdata(pdev, otg_dev);
  dev_dbg(&pdev->dev, "otg_dev=0x%p\n", otg_dev);
#else
	lm_set_drvdata( pdev, otg_dev );
	dev_dbg(&pdev->dev, "otg_dev=0x%p\n", otg_dev);
#endif
	otg_dev->core_if = dwc_otg_cil_init( otg_dev->base, &dwc_otg_module_params);
	if (otg_dev->core_if == 0)
	{
		dev_err(&pdev->dev, "CIL initialization failed!\n");
		retval = -ENOMEM;
		goto fail;
	}

	/*
	 * Validate parameter values.
	 */
	if (check_parameters(otg_dev->core_if) != 0)
	{
	printk("[SHLEE] failed on check_parameters(), goto fail and remove driver\n");	//shlee
		retval = -EINVAL;
		goto fail;
	}
#if 0	//cathy
	/*
	 * Create Device Attributes in sysfs
	 */
	dwc_otg_attr_create (pdev);
#endif
	/*
	 * Disable the global interrupt until all the interrupt
	 * handlers are installed.
	 */

	dwc_otg_disable_global_interrupts( otg_dev->core_if );

	/*
	 * Install the interrupt handler for the common interrupts before
	 * enabling common interrupts in core_init below.
	 */
#if !DRIVER_USING_LM
  DWC_DEBUGPL(DBG_CIL, "registering (common) handler for irq%d\n", otg_dev->irq);
  retval = request_irq(otg_dev->irq, dwc_otg_common_irq, IRQF_SHARED, "dwc_otg", otg_dev);
#else
	DWC_DEBUGPL( DBG_CIL, "registering (common) handler for irq%d\n", pdev->irq);
	retval = request_irq(pdev->irq, dwc_otg_common_irq, IRQF_SHARED, "dwc_otg", otg_dev );
#endif
	if (retval != 0)
	{
	#if !DRIVER_USING_LM
                DWC_ERROR("request of irq%d failed\n", otg_dev->irq);
	#else
		DWC_ERROR("request of irq%d failed\n", pdev->irq);
	#endif
		retval = -EBUSY;
		goto fail;
	}
	else
	{
		otg_dev->common_irq_installed = 1;
	}

//	otg_dev->irq=pdev->irq; //wei add sync
#ifdef CONFIG_MACH_IPMATE
	set_irq_type(pdev->irq, IRQT_LOW);
#endif
//-------------------------------------------------------------------------
	/*
	 * Initialize the DWC_otg core.
	 */
	dwc_otg_core_init( otg_dev->core_if );
//---------------------------------------------------------------------------
//#ifndef DWC_HOST_ONLY
	if(gHostMode==0)
	{
	/*
	 * Initialize the PCD
	 */
	//printk("--init pcd--\n");
  retval = dwc_otg_pcd_init(pdev);
	if (retval != 0)
	{
		DWC_ERROR("dwc_otg_pcd_init failed\n");
		otg_dev->pcd = NULL;
		goto fail;
	}
	}
//#endif
//---------------------------------------------------------------------------

//#ifndef DWC_DEVICE_ONLY
	if(gHostMode==1)
	{
	/*
	 * Initialize the HCD
	 */
	//printk("----init hcd-----\n");
        retval = dwc_otg_hcd_init(pdev);
	if (retval != 0)
	{
		DWC_ERROR("dwc_otg_hcd_init failed\n");
		otg_dev->hcd = NULL;
		goto fail;
	}
	}
//#endif

//---------------------------------------------------------------------------
	/*
	 * Enable the global interrupt after all the interrupt
	 * handlers are installed.
	 */

	//printk("otgdev,  drvdata=%x\n", lm_get_drvdata(pdev));
	 //printk("----enable irq----- \n");
	dwc_otg_enable_global_interrupts( otg_dev->core_if );
	return 0;

 fail:
	dwc_otg_driver_remove(pdev);
	return retval;
}

/**
 * This structure defines the methods to be called by a bus driver
 * during the lifecycle of a device on that bus. Both drivers and
 * devices are registered with a bus driver. The bus driver matches
 * devices to drivers based on information in the device and driver
 * structures.
 *
 * The probe function is called when the bus driver matches a device
 * to this driver. The remove function is called when a device is
 * unregistered with the bus driver.
 */
#if !DRIVER_USING_LM
static struct platform_driver dwc_otg_driver = {
  .probe      = dwc_otg_driver_probe,
  .remove     = dwc_otg_driver_remove,
  .driver     = {
    .name   = "dwc_otg",
    .owner  = THIS_MODULE,
  },
};
#else
static struct lm_driver dwc_otg_driver =
{
	.driver =
			{
		.		name	= (char*)dwc_driver_name,
			},
	.probe		= dwc_otg_driver_probe,
	.remove		= dwc_otg_driver_remove,
};
#endif

void dwc_otg_phy_write(unsigned char reg, unsigned char val)
{
	#define USB2_PHY_DELAY __delay(1000)

	if((reg < 0xE0) || (reg > 0xF6) || ((reg>0xE7)&&(reg<0xF0))) {
		printk("DWC_OTG: Wrong register address: 0x%02x\n", reg);
		return;
	}

#if 0  //8198
	int tmp = REG32(0xb8003314);  //8672 only
	tmp = tmp & 0xFF00FF00;
	REG32(0xb8003314) = (val << 16) | tmp; USB2_PHY_DELAY;
#else  //8196D otg
	unsigned int tmp = REG32(0xb8000090);  //8672 only
#if CONFIG_RTL_8198C
	tmp = tmp & ~(0xff<<(11-11));
	REG32(0xb8000090) = (val << (11-11)) | tmp; USB2_PHY_DELAY;
#else
	tmp = tmp & ~(0xff<<11);
	REG32(0xb8000090) = (val << 11) | tmp; USB2_PHY_DELAY;
#endif
#endif
	//REG32(0xb8003314) = (val << 16) | tmp; USB2_PHY_DELAY;
	REG32(0xb8030034) = ((reg & 0x0F) << 16) | 0x00004002; USB2_PHY_DELAY;
	REG32(0xb8030034) = ((reg & 0x0F) << 16) | 0x00004000; USB2_PHY_DELAY;
	REG32(0xb8030034) = ((reg & 0x0F) << 16) | 0x00004002; USB2_PHY_DELAY;
	REG32(0xb8030034) = ((reg & 0xF0) << 12) | 0x00004002; USB2_PHY_DELAY;
	REG32(0xb8030034) = ((reg & 0xF0) << 12) | 0x00004000; USB2_PHY_DELAY;
	REG32(0xb8030034) = ((reg & 0xF0) << 12) | 0x00004002; USB2_PHY_DELAY;

	return;
}

unsigned char dwc_otg_phy_read(unsigned char reg)
{
	#define USB2_PHY_DELAY __delay(200)
	unsigned char val = 0;

	if((reg < 0xE0) || (reg > 0xF6) || ((reg>0xE7)&&(reg<0xF0))) {
		printk("DWC_OTG: Wrong register address: 0x%02x\n", reg);
		return 0;
	}

	reg = reg - 0x20;
	REG32(0xb8030034) = ((reg & 0x0F) << 16) | 0x00004002; USB2_PHY_DELAY;
	REG32(0xb8030034) = ((reg & 0x0F) << 16) | 0x00004000; USB2_PHY_DELAY;
	REG32(0xb8030034) = ((reg & 0x0F) << 16) | 0x00004002; USB2_PHY_DELAY;
	REG32(0xb8030034) = ((reg & 0xF0) << 12) | 0x00004002; USB2_PHY_DELAY;
	REG32(0xb8030034) = ((reg & 0xF0) << 12) | 0x00004000; USB2_PHY_DELAY;
	REG32(0xb8030034) = ((reg & 0xF0) << 12) | 0x00004002; USB2_PHY_DELAY;

	val = (REG32(0xb8030034) & 0xFF000000) >> 24;
	//printk("DWC_OTG: phy(0x%02x) = 0x%02x\n", reg + 0x20, val);

	return val;
}


//====================================================================
//8196D OTG init
//=========================================================================
void HangUpRes(int en)
{
	#define SYS_USB_SIE 0xb8000034
	if(en==0)
	{
		REG32(SYS_USB_SIE) &= ~(1<<(27-0));  //s_usbotg_fs_ternel=0
	}
	else
	{
		REG32(SYS_USB_SIE) |= (1<<(27-0));  //s_usbotg_fs_ternel=1
	}
}
//===============================================================
void Set_SelUSBPort(int port)  // pass 1: is one port, other value is 2port
{
	#define SYS_USB_SIE 0xb8000034
	if(port==1)
	{	REG32(SYS_USB_SIE) |= (1<<18);   //one_port_host_sel=1, means: one port
	}
	else
	{	REG32(SYS_USB_SIE) &= ~(1<<18);   //one_port_host_sel=0, means: two port
	}
}
//-----------------------------------------------------------------------------------
unsigned int Get_SelUSBPort()
{
	#define SYS_USB_SIE 0xb8000034
	unsigned int oneportsel=(REG32(SYS_USB_SIE) & (1<<18))>>18;
	return (oneportsel & 0x1);
}
//-----------------------------------------------------------------------------------
TurnOn_OtgClkMgr()
{
	#define SYS_CLK_MANAGER 0xb8000010
	REG32(SYS_CLK_MANAGER) |= ((1<<19)|(1<<20));    //enable lx2, arb


	REG32(SYS_CLK_MANAGER) &= ~(1<<29);    //disable otg
	mdelay(100);
	REG32(SYS_CLK_MANAGER) |= (1<<29);    //enable otg
	mdelay(100);
}
//-----------------------------------------------------------------------------------
Set_IDDIG_Level(int force, int device)  // force: 0: auto, 1: force,    device: 1:device 0:host
{
	#define SYS_OTG_CONTROL 0xb8000098

	if(force==1)
	{

		if(device==1)
		{	REG32(SYS_OTG_CONTROL)|=(1<<3);  // otgctrl_force_dev=1 is device, 0 is host
		}
	else
		{	REG32(SYS_OTG_CONTROL)&=~(1<<3);  // otgctrl_force_dev=1 is device, 0 is host
		}

	REG32(SYS_OTG_CONTROL)|=(1<<2);  // otgctrl_mux_sel=0 see hw pin, 1 see the bit 3
}
	else
	{
		REG32(SYS_OTG_CONTROL)&= ~(1<<2);  // otgctrl_mux_sel=0 see hw pin, 1 see the bit 3
	}

}
//-----------------------------------------------------------------------------------
unsigned int Get_IDDIG_Level()
{
	#define GETBITVAL(v,bit)  ((v&(1<<bit))>>bit)

	#define SYS_OTG_CONTROL 0xb8000098
	unsigned int v=REG32(SYS_OTG_CONTROL);

	unsigned int muxsel=GETBITVAL(v,2);
	printk("OTGCTRL: Iddig is %s mode \n", (muxsel==1) ? "Force" : "Auto Det");

	unsigned int iddig;
	if(muxsel==1)
	{
		iddig=GETBITVAL(v,3);  //force
	}
	else
	{
		 iddig=GETBITVAL(v,18);	//auto
	}
	printk("OTGCTRL: Got iddig=%x, otg is %s mode \n", iddig, (iddig==1) ? "Device" : "Host");
	return (iddig&01);
}
//-----------------------------------------------------------------------------------
Enable_AutoDetectionCircuit(int en)
{
	#define SYS_OTG_CONTROL 0xb8000098
	#define SYS_PIN_MUX 0xb8000040
	if(en==1)
	{
		unsigned int r;
		REG32(SYS_PIN_MUX) &= ~(0x7);   //bit [2:0]=0

		REG32(SYS_OTG_CONTROL) |=  (1<<0);  //active_otgctrl=1, enable elvis auto-det circuit

		r=REG32(SYS_OTG_CONTROL);
		r = r  &~((0xf<<6)|(0xf<<10)|(0x7<<14));  //clear
		REG32(SYS_OTG_CONTROL) = r| (10<<6)|(10<<10)|(4<<14);     //setting

		REG32(SYS_OTG_CONTROL) |=  (1<<5);  //OTGCMP_EN=1
		REG32(SYS_OTG_CONTROL) |=  (1<<1);  //start
	}
	else
	{	REG32(SYS_OTG_CONTROL) &= ~(1<<1);  //stop
		REG32(SYS_OTG_CONTROL) &= ~(1<<5);  //OTGCMP_EN=0
		REG32(SYS_OTG_CONTROL) &= ~(1<<0);  //active_otgctrl=0 , disable elvis auto-det circuit
	}
}
//-----------------------------------------------------------------------------------
Enable_OTG_Suspend(int sel, int en)  //sel=0 src from sys, then see en, sel=1, src from otg mac,
{
	#define SYS_USB_SIE 0xb8000034

	if(sel==0)
	{
		if(en==1)
		{
			REG32(SYS_USB_SIE)&= ~(1<<(21-0));        //s_suspend_n=0
		}
		else
		{
			REG32(SYS_USB_SIE) |= (1<<(21-0));        //s_suspend_n=1
		}

		REG32(SYS_USB_SIE) &= ~(1<<(26-0));     //s_suspend_sel=0  (source from system)
	}
	else
	{
		REG32(SYS_USB_SIE) |=  (1<<(26-0));     //s_suspend_sel=1  (source from mac)
	}
}
//-----------------------------------------------------------------------------------
USBPhyReset(int reset)  //1: in reset,  0: working
{
	#define SYS_USB_PHY 0xb8000090
	if(reset==0)
#if CONFIG_RTL_8198C
	REG32(SYS_USB_PHY) &= ~(1<<(20-11));   //usbphy_reset=0
#else
	REG32(SYS_USB_PHY) &= ~(1<<20);   //usbphy_reset=0
#endif
	else
#if CONFIG_RTL_8198C
	REG32(SYS_USB_PHY) |=  (1<<(20-11));   //usbphy_reset=1
#else
	REG32(SYS_USB_PHY) |=  (1<<20);   //usbphy_reset=1
#endif
}
//---------------------------------------------------------------------------------

int otg_reset_procedure(int mode)
{

  //8196D
	//#define SYS_CLK_MANAGER 0xb8000010
	#define SYS_USB_SIE 0xb8000034
	#define SYS_USB_PHY 0xb8000090
	//#define SYS_OTG_CONTROL 0xb8000098

	Enable_OTG_Suspend(1,NULL);		//sel=0 src from sys, then see en, sel=1, src from otg mac,
									//en=1 go to suspend, en=0 not in suspend, is working

	REG32(SYS_USB_SIE) |= (1<<(22-0));     //en_usbotg=1,  connect MAC and PHY
#if CONFIG_RTL_8198C
	REG32(SYS_USB_PHY)|=(1<<(19-11));  //USBPHY_EN=1
	REG32(SYS_USB_PHY)|=(1<<(21-11));  //active_usbphy
#else
	REG32(SYS_USB_PHY)|=(1<<(19-0));  //USBPHY_EN=1
	REG32(SYS_USB_PHY)|=(1<<(21-0));  //active_usbphy
#endif
//----------------------------------------
#ifndef CONFIG_RTL_OTGCTRL  //software decide iddig
	Enable_AutoDetectionCircuit(0);  //disable autodet
	gHostMode=mode;
	if(gHostMode==1)	Set_IDDIG_Level(1,0);	 // 1:device 0:host
	else					Set_IDDIG_Level(1,1);	 // 1:device 0:host
#elif defined(CONFIG_RTL_ULINKER)
	gHostMode=1-Get_IDDIG_Level();
#else	//auto-det ckt decide iddig
    #if 0  //1: internal enable auto-det,
	Enable_AutoDetectionCircuit(1);
	Set_IDDIG_Level(0,0);
    #endif
	gHostMode=1-Get_IDDIG_Level();
#endif
//----------------------------------------
#if 1 //
	Set_SelUSBPort(0);  // pass 1: is one port, other value is 2port
#else  //dynamic get
	if(Get_SelUSBPort()==1)
	{	printk("OTG: Got one_port_host_sel=1, cannot continue init otg\n");
		return -1;
	}
#endif
//----------------------------------------
#if CONFIG_RTL_8198C

	REG32(0xb8000010) = REG32(0xb8000010) | (0x1<<29);
#endif

	TurnOn_OtgClkMgr();
#if 1
	USBPhyReset(1);  //1: in reset, 0: working

//	if(gHostMode==1)
//	USBPhyReset(0);  //1: in reset, 0: working
#endif

#if 1
#if CONFIG_RTL_8198C

//	REG32(0xb8000010) = REG32(0xb8000010) | (0x1<<29);
	//USBPhyReset(0);
#if 0
  set_u2_phy( 0xe0, 0x44);
        set_u2_phy( 0xe1, 0xe8);
        set_u2_phy( 0xe2, 0x9a);
        set_u2_phy( 0xe3, 0xa1);
        set_u2_phy( 0xe5, 0x95);
        set_u2_phy( 0xe6, 0x98);

        set_u2_phy( 0xf5, 0x49);
#endif
	 dwc_otg_phy_write(0xe0,0x44);  //disconnect, work
        dwc_otg_phy_write(0xe1,0xe9);  //disconnect, work
        dwc_otg_phy_write(0xe2,0x9a);  //disconnect, work
        dwc_otg_phy_write(0xe3,0xa1);  //disconnect, work
        dwc_otg_phy_write(0xe5,0x95);  //disconnect, work
        dwc_otg_phy_write(0xe6,0x98);  //disco

        dwc_otg_phy_write(0xf5,0x49);  //disco
	//printk("b8000010=%x\r\n",REG32(0xb8000010));

	//printk("b8000090=%x\r\n",REG32(0xb8000090));
	//int i;;
	//for(i=0xe0;i<=0xe7; i++)
		//printk("reg %x=%x\n", i,dwc_otg_phy_read(i) );

	//for(i=0xe0;i<=0xe7; i++)
	//	printk("reg %x=%x\n", i,dwc_otg_phy_read(i) );

#else
#if !defined(CONFIG_RTL_8881A)

	//USBPhyReset(0);  //1: in reset, 0: working
	//PHYPatch();  //wei add
	dwc_otg_phy_write(0xe0,0x99);  //disconnect, work
	dwc_otg_phy_write(0xe1,0xac);  //disconnect, work
	dwc_otg_phy_write(0xe2,0x98);  //disconnect, work
	dwc_otg_phy_write(0xe3,0xc1);  //disconnect, work
	dwc_otg_phy_write(0xe4,0x1);  //disconnect, work
	dwc_otg_phy_write(0xe5,0x89);  //disconnect, work
	dwc_otg_phy_write(0xe6,0x98);  //disconnect, worki
	dwc_otg_phy_write(0xe7,0x1d);  //disconnect, work
	dwc_otg_phy_write(0xf0,0xfc);  //disconnect, work
	dwc_otg_phy_write(0xf1,0x8c);  //disconnect, work
	dwc_otg_phy_write(0xf2,0x0);  //disconnect, work
	dwc_otg_phy_write(0xf3,0x11);  //disconnect, work
	dwc_otg_phy_write(0xf4,0xfb);  //disconnect, work
	dwc_otg_phy_write(0xf5,0xd2);  //disconnect,
	dwc_otg_phy_write(0xf6,0x0);  //disconnect, work

	dwc_otg_phy_write(0xe6,0xb8);  //disconnect, work
	int i;
	//for(i=0xe0;i<=0xe7; i++)
		//printk("reg %x=%x\n", i,dwc_otg_phy_read(i) );
#endif
#endif
#endif

	HangUpRes(0);
	if(gHostMode==1)
	HangUpRes(1);
	return 0;

}
//=============================================================================



/**
 * This function is called when the dwc_otg_driver is installed with the
 * insmod command. It registers the dwc_otg_driver structure with the
 * appropriate bus driver. This will cause the dwc_otg_driver_probe function
 * to be called. In addition, the bus driver will automatically expose
 * attributes defined for the device and driver in the special sysfs file
 * system.
 *
 * @return
 */

unsigned long otg_driver_loaded=0;
#define OTG_DRIVER_LOADED 1

int  dwc_otg_driver_init(void)
{
	if ( test_bit(OTG_DRIVER_LOADED, &otg_driver_loaded))
	{	printk("DWC_OTG: driver already init \n");
		return -1;
	}

#if 1  //for 8196D
	printk("-------8196D OTG init \n");
//	otg_proc_init();

#if defined(CONFIG_RTL_ULINKER)
	int rc=otg_reset_procedure(0);
#else
	int rc=otg_reset_procedure(1);
#endif
	if(rc)
	{	printk("OTG: reset procedure init fail \n");
		return rc;
	}
#endif


	#define OTG_BASE	0xb8030000
	#define USB2_PHY_DELAY __delay(200)
	int retval = 0;
	int snpsid;
	struct lm_device *lmdev;

	printk(KERN_INFO "%s: version %s\n", dwc_driver_name, DWC_DRIVER_VERSION);

       //-----------------------------
	// Kaohj --- probe OTG
	snpsid = dwc_read_reg32((uint32_t *)((uint8_t *)OTG_BASE + 0x40));

	if ((snpsid & 0xFFFFF000) != 0x4F542000)
	{
		DWC_ERROR("OTG Device not found ! Bad value for SNPSID: 0x%08x\n", snpsid);
		return 0;
	}
	#if defined(CONFIG_RTL_8881A)

	// refine for B-cut and later
	{
	unsigned int val= REG32(0xb80000dc);
	if ((val & 0x3) != 0x3)
		REG32(0xb80000dc) |= 0x3; // wlanmac_control, set active_wlanmac_sys and active_wlanmac_lx for RF revision ID

	if ((REG32(0xb86400f0) & 0x0000f000) >= 0x00001000)
	{
		//Recommand by Yozen 09272013 for FT parameters
		dwc_otg_phy_write(0xe2,0x9b);
		dwc_otg_phy_write(0xe6,0xe8);
	}
	else
	{
			dwc_otg_phy_write(0xe2,0x9c);
            dwc_otg_phy_write(0xe6,0xa8);
	}

	REG32(0xb80000dc) = val; // restore value.
	}



        #endif
#if 0
	if(IS_6085) {
		printk("PHY IS_6085 \n");
		dwc_otg_phy_write(0xE7, 0x38);	//write usb port0's phy reg 0xE7 to change vbusvalid threshold down to 3.3V
		dwc_otg_phy_write(0xE2, 0x81);
		dwc_otg_phy_write(0xE5, 0x95);
#ifndef DWC_DEVICE_ONLY
		dwc_otg_phy_write(0xE6, 0xE8);
#endif
		dwc_otg_phy_write(0xF1, 0x8E);
	}
	else if(IS_RLE0315 || IS_6166) {
		printk("PHY IS_RLE0315  or IS_6166  \n");
		printk("disable USB phy 0\n");
		REG32(0xb800330c) &= 0xFFFFFF7F;

		dwc_otg_phy_write(0xE0, 0xB8);
		dwc_otg_phy_write(0xE1, 0xA8);
		dwc_otg_phy_write(0xE2, 0x99);
		dwc_otg_phy_write(0xE3, 0x41);
		dwc_otg_phy_write(0xE5, 0x91);
		dwc_otg_phy_write(0xE6, 0x78);
		dwc_otg_phy_write(0xF4, 0xE3);
#ifndef DWC_HOST_ONLY
		dwc_otg_phy_write(0xF5, 0x12);	//change vbusvalid threshold down to 3.3V
#endif
		dwc_otg_phy_write(0xF6, 0x04);
	}
	else {
		printk("PHY IS normal\n");
		tmp = tmp & 0xFF00FF00;
//		REG32(0xb8003314) = tmp|0x00000038; USB2_PHY_DELAY;	//write usb port0's phy reg 0xE7 to change vbusvalid threshold down to 3.3V
		REG32(0xb80210A4) = 0x00370000; USB2_PHY_DELAY;
		REG32(0xb80210A4) = 0x00270000; USB2_PHY_DELAY;
		REG32(0xb80210A4) = 0x00370000; USB2_PHY_DELAY;
		REG32(0xb80210A4) = 0x003E0000; USB2_PHY_DELAY;
		REG32(0xb80210A4) = 0x002E0000; USB2_PHY_DELAY;
		REG32(0xb80210A4) = 0x003E0000; USB2_PHY_DELAY;

		dwc_otg_phy_write(0xE2, 0x82);
		dwc_otg_phy_write(0xE5, 0x91);
#ifndef DWC_DEVICE_ONLY
		dwc_otg_phy_write(0xE6, 0xE8);
#endif
		dwc_otg_phy_write(0xF1, 0x8E);
	}
#endif
#if 0	//cathy, test_J, test_K
//write phy register
	#define USB2_PHY_DELAY {int i=100; while(i>0) {i--;}}
	int tmp = REG32(0xb8003314);
	REG32(0xb8003314) = tmp|0x008E0000; USB2_PHY_DELAY;
	REG32(0xb8030034) = 0x8E014002; USB2_PHY_DELAY;
	REG32(0xb8030034) = 0x8E014000; USB2_PHY_DELAY;
	REG32(0xb8030034) = 0x8E014002; USB2_PHY_DELAY;
	REG32(0xb8030034) = 0x8E0F4002; USB2_PHY_DELAY;
	REG32(0xb8030034) = 0x8E0F4000; USB2_PHY_DELAY;
	REG32(0xb8030034) = 0x8E0F4002; USB2_PHY_DELAY;
#endif

#if 0
		//dump
		printf("PHY reg E0=%x\n", dwc_otg_phy_read(0xE0));
		printf("PHY reg E1=%x\n", dwc_otg_phy_read(0xE1));
		printf("PHY reg E2=%x\n", dwc_otg_phy_read(0xE2));
		printf("PHY reg E3=%x\n", dwc_otg_phy_read(0xE3));
		printf("PHY reg E4=%x\n", dwc_otg_phy_read(0xE4));
		printf("PHY reg E5=%x\n", dwc_otg_phy_read(0xE5));
		printf("PHY reg E6=%x\n", dwc_otg_phy_read(0xE6));
		printf("PHY reg E7=%x\n", dwc_otg_phy_read(0xE7));
		printf("PHY reg F0=%x\n", dwc_otg_phy_read(0xF0));
		printf("PHY reg F1=%x\n", dwc_otg_phy_read(0xF1));
		printf("PHY reg F2=%x\n", dwc_otg_phy_read(0xF2));
#endif
   //Enhance USB 3.0 IOT issues
       dwc_otg_phy_write(0xE2, 0x99);
#if CONFIG_RTL_8198C
       dwc_otg_phy_write(0xE6, 0xc8);
#else
       dwc_otg_phy_write(0xE6, 0xb8);
#endif


#if DRIVER_USING_LM
//cathy, allocate a lmdev device for driver
	lmdev = kzalloc(sizeof(struct lm_device), GFP_KERNEL);
	if (!lmdev) {
		printk("kzalloc Imdev in dwc_otg_driver_init failed!\n");
		return -1;
	}
	printk(" [S0] lmdev=%x \n", lmdev);
	//lmdev->resource.start = 0xb8030000;		//base of OTG, 0xb8030000
	lmdev->resource.start = OTG_BASE;		//base of OTG, 0xb8030000
	lmdev->resource.end = lmdev->resource.start + 0x0003ffff;
	lmdev->resource.flags = IORESOURCE_MEM;
//#define USB_D_IRQ             11   //wei add ,11
//	lmdev->irq = USB_D_IRQ;	//irq of usb device
	lmdev->irq = BSP_OTG_IRQ;	//irq of usb device

	lmdev->id = 0;

	lm_device_register(lmdev);
	glmdev=lmdev;  //wei add for hook
/////////////
#endif

#if DRIVER_USING_LM
	retval = lm_driver_register(&dwc_otg_driver);
	if (retval < 0)
	{
		printk(KERN_ERR "%s retval=%d\n", __func__, retval);
		return retval;
	}
	printk("  [S1] dwc_otg_driver, lmdriver=%x \n", &dwc_otg_driver );
#else
  retval = platform_driver_register (&dwc_otg_driver);
  if (retval < 0)
    {
      printk (KERN_ERR "%s registration failed. retval=%d\n",
              dwc_driver_name, retval);
      return retval;
    }
#endif


  retval = driver_create_file (&dwc_otg_driver.driver, &driver_attr_version);
  retval |= driver_create_file (&dwc_otg_driver.driver, &driver_attr_debuglevel);


  if (retval)
    {
      platform_driver_unregister (&dwc_otg_driver);
      printk (KERN_ERR "%s create_file failed. retval=%d\n",
              dwc_driver_name, retval);
      return retval;
    }



#if 0 //DRIVER_USING_LM
	retval = dwc_otg_driver_probe(lmdev);
	if(retval < 0){
		printk("%s retval=%d\n", __func__, retval);
		return retval;
	}
#endif

	set_bit(OTG_DRIVER_LOADED, &otg_driver_loaded);


  return retval;

}
#if !defined(CONFIG_RTL_OTGCTRL) && !defined(CONFIG_RTL_ULINKER)
module_init(dwc_otg_driver_init);
#endif

/**
 * This function is called when the driver is removed from the kernel
 * with the rmmod command. The driver unregisters itself with its bus
 * driver.
 *
 */
void  dwc_otg_driver_cleanup(void)
{
	if (!test_bit(OTG_DRIVER_LOADED, &otg_driver_loaded))
	{	printk("DWC_OTG: not init, cannot cleanup \n");
		return;
	}


	printk(KERN_DEBUG "dwc_otg_driver_cleanup()\n");

	driver_remove_file(&dwc_otg_driver.driver, &driver_attr_debuglevel);
	driver_remove_file(&dwc_otg_driver.driver, &driver_attr_version);


#if !DRIVER_USING_LM
  platform_driver_unregister(&dwc_otg_driver);
#else
	lm_driver_unregister(&dwc_otg_driver);
#endif

	lm_device_release(&glmdev->dev);  //wei add



	printk(KERN_INFO "%s module removed\n", dwc_driver_name);

	clear_bit(OTG_DRIVER_LOADED, &otg_driver_loaded);
}
#if !defined(CONFIG_RTL_OTGCTRL) && !defined(CONFIG_RTL_ULINKER)
module_exit(dwc_otg_driver_cleanup);
#endif

//==========================================






//=========================================

MODULE_DESCRIPTION(DWC_DRIVER_DESC);
MODULE_AUTHOR("Synopsys Inc.");
MODULE_LICENSE("GPL");

module_param_named(otg_cap, dwc_otg_module_params.otg_cap, int, 0444);
MODULE_PARM_DESC(otg_cap, "OTG Capabilities 0=HNP&SRP 1=SRP Only 2=None");
module_param_named(opt, dwc_otg_module_params.opt, int, 0444);
MODULE_PARM_DESC(opt, "OPT Mode");
module_param_named(dma_enable, dwc_otg_module_params.dma_enable, int, 0444);
MODULE_PARM_DESC(dma_enable, "DMA Mode 0=Slave 1=DMA enabled");
module_param_named(dma_burst_size, dwc_otg_module_params.dma_burst_size, int, 0444);
MODULE_PARM_DESC(dma_burst_size, "DMA Burst Size 1, 4, 8, 16, 32, 64, 128, 256");
module_param_named(speed, dwc_otg_module_params.speed, int, 0444);
MODULE_PARM_DESC(speed, "Speed 0=High Speed 1=Full Speed");
module_param_named(host_support_fs_ls_low_power, dwc_otg_module_params.host_support_fs_ls_low_power, int, 0444);
MODULE_PARM_DESC(host_support_fs_ls_low_power, "Support Low Power w/FS or LS 0=Support 1=Don't Support");
module_param_named(host_ls_low_power_phy_clk, dwc_otg_module_params.host_ls_low_power_phy_clk, int, 0444);
MODULE_PARM_DESC(host_ls_low_power_phy_clk, "Low Speed Low Power Clock 0=48Mhz 1=6Mhz");
module_param_named(enable_dynamic_fifo, dwc_otg_module_params.enable_dynamic_fifo, int, 0444);
MODULE_PARM_DESC(enable_dynamic_fifo, "0=cC Setting 1=Allow Dynamic Sizing");
module_param_named(data_fifo_size, dwc_otg_module_params.data_fifo_size, int, 0444);
MODULE_PARM_DESC(data_fifo_size, "Total number of words in the data FIFO memory 32-32768");
module_param_named(dev_rx_fifo_size, dwc_otg_module_params.dev_rx_fifo_size, int, 0444);
MODULE_PARM_DESC(dev_rx_fifo_size, "Number of words in the Rx FIFO 16-32768");
module_param_named(dev_nperio_tx_fifo_size, dwc_otg_module_params.dev_nperio_tx_fifo_size, int, 0444);
MODULE_PARM_DESC(dev_nperio_tx_fifo_size, "Number of words in the non-periodic Tx FIFO 16-32768");
module_param_named(dev_perio_tx_fifo_size_1, dwc_otg_module_params.dev_perio_tx_fifo_size[0], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_1, "Number of words in the periodic Tx FIFO 4-768");
module_param_named(dev_perio_tx_fifo_size_2, dwc_otg_module_params.dev_perio_tx_fifo_size[1], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_2, "Number of words in the periodic Tx FIFO 4-768");
module_param_named(dev_perio_tx_fifo_size_3, dwc_otg_module_params.dev_perio_tx_fifo_size[2], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_3, "Number of words in the periodic Tx FIFO 4-768");
module_param_named(dev_perio_tx_fifo_size_4, dwc_otg_module_params.dev_perio_tx_fifo_size[3], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_4, "Number of words in the periodic Tx FIFO 4-768");
module_param_named(dev_perio_tx_fifo_size_5, dwc_otg_module_params.dev_perio_tx_fifo_size[4], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_5, "Number of words in the periodic Tx FIFO 4-768");
module_param_named(dev_perio_tx_fifo_size_6, dwc_otg_module_params.dev_perio_tx_fifo_size[5], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_6, "Number of words in the periodic Tx FIFO 4-768");
module_param_named(dev_perio_tx_fifo_size_7, dwc_otg_module_params.dev_perio_tx_fifo_size[6], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_7, "Number of words in the periodic Tx FIFO 4-768");
module_param_named(dev_perio_tx_fifo_size_8, dwc_otg_module_params.dev_perio_tx_fifo_size[7], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_8, "Number of words in the periodic Tx FIFO 4-768");
module_param_named(dev_perio_tx_fifo_size_9, dwc_otg_module_params.dev_perio_tx_fifo_size[8], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_9, "Number of words in the periodic Tx FIFO 4-768");
module_param_named(dev_perio_tx_fifo_size_10, dwc_otg_module_params.dev_perio_tx_fifo_size[9], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_10, "Number of words in the periodic Tx FIFO 4-768");
module_param_named(dev_perio_tx_fifo_size_11, dwc_otg_module_params.dev_perio_tx_fifo_size[10], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_11, "Number of words in the periodic Tx FIFO 4-768");
module_param_named(dev_perio_tx_fifo_size_12, dwc_otg_module_params.dev_perio_tx_fifo_size[11], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_12, "Number of words in the periodic Tx FIFO 4-768");
module_param_named(dev_perio_tx_fifo_size_13, dwc_otg_module_params.dev_perio_tx_fifo_size[12], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_13, "Number of words in the periodic Tx FIFO 4-768");
module_param_named(dev_perio_tx_fifo_size_14, dwc_otg_module_params.dev_perio_tx_fifo_size[13], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_14, "Number of words in the periodic Tx FIFO 4-768");
module_param_named(dev_perio_tx_fifo_size_15, dwc_otg_module_params.dev_perio_tx_fifo_size[14], int, 0444);
MODULE_PARM_DESC(dev_perio_tx_fifo_size_15, "Number of words in the periodic Tx FIFO 4-768");
module_param_named(host_rx_fifo_size, dwc_otg_module_params.host_rx_fifo_size, int, 0444);
MODULE_PARM_DESC(host_rx_fifo_size, "Number of words in the Rx FIFO 16-32768");
module_param_named(host_nperio_tx_fifo_size, dwc_otg_module_params.host_nperio_tx_fifo_size, int, 0444);
MODULE_PARM_DESC(host_nperio_tx_fifo_size, "Number of words in the non-periodic Tx FIFO 16-32768");
module_param_named(host_perio_tx_fifo_size, dwc_otg_module_params.host_perio_tx_fifo_size, int, 0444);
MODULE_PARM_DESC(host_perio_tx_fifo_size, "Number of words in the host periodic Tx FIFO 16-32768");
module_param_named(max_transfer_size, dwc_otg_module_params.max_transfer_size, int, 0444);
/** @todo Set the max to 512K, modify checks */
MODULE_PARM_DESC(max_transfer_size, "The maximum transfer size supported in bytes 2047-65535");
module_param_named(max_packet_count, dwc_otg_module_params.max_packet_count, int, 0444);
MODULE_PARM_DESC(max_packet_count, "The maximum number of packets in a transfer 15-511");
module_param_named(host_channels, dwc_otg_module_params.host_channels, int, 0444);
MODULE_PARM_DESC(host_channels, "The number of host channel registers to use 1-16");
module_param_named(dev_endpoints, dwc_otg_module_params.dev_endpoints, int, 0444);
MODULE_PARM_DESC(dev_endpoints, "The number of endpoints in addition to EP0 available for device mode 1-15");
module_param_named(phy_type, dwc_otg_module_params.phy_type, int, 0444);
MODULE_PARM_DESC(phy_type, "0=Reserved 1=UTMI+ 2=ULPI");
module_param_named(phy_utmi_width, dwc_otg_module_params.phy_utmi_width, int, 0444);
MODULE_PARM_DESC(phy_utmi_width, "Specifies the UTMI+ Data Width 8 or 16 bits");
module_param_named(phy_ulpi_ddr, dwc_otg_module_params.phy_ulpi_ddr, int, 0444);
MODULE_PARM_DESC(phy_ulpi_ddr, "ULPI at double or single data rate 0=Single 1=Double");
module_param_named(phy_ulpi_ext_vbus, dwc_otg_module_params.phy_ulpi_ext_vbus, int, 0444);
MODULE_PARM_DESC(phy_ulpi_ext_vbus, "ULPI PHY using internal or external vbus 0=Internal");
module_param_named(i2c_enable, dwc_otg_module_params.i2c_enable, int, 0444);
MODULE_PARM_DESC(i2c_enable, "FS PHY Interface");
module_param_named(ulpi_fs_ls, dwc_otg_module_params.ulpi_fs_ls, int, 0444);
MODULE_PARM_DESC(ulpi_fs_ls, "ULPI PHY FS/LS mode only");
module_param_named(ts_dline, dwc_otg_module_params.ts_dline, int, 0444);
MODULE_PARM_DESC(ts_dline, "Term select Dline pulsing for all PHYs");
module_param_named(debug, g_dbg_lvl, int, 0444);
MODULE_PARM_DESC(debug, "");

module_param_named(en_multiple_tx_fifo, dwc_otg_module_params.en_multiple_tx_fifo, int, 0444);
MODULE_PARM_DESC(en_multiple_tx_fifo, "Dedicated Non Periodic Tx FIFOs 0=disabled 1=enabled");
module_param_named(dev_tx_fifo_size_1, dwc_otg_module_params.dev_tx_fifo_size[0], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_1, "Number of words in the Tx FIFO 4-768");
module_param_named(dev_tx_fifo_size_2, dwc_otg_module_params.dev_tx_fifo_size[1], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_2, "Number of words in the Tx FIFO 4-768");
module_param_named(dev_tx_fifo_size_3, dwc_otg_module_params.dev_tx_fifo_size[2], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_3, "Number of words in the Tx FIFO 4-768");
module_param_named(dev_tx_fifo_size_4, dwc_otg_module_params.dev_tx_fifo_size[3], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_4, "Number of words in the Tx FIFO 4-768");
module_param_named(dev_tx_fifo_size_5, dwc_otg_module_params.dev_tx_fifo_size[4], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_5, "Number of words in the Tx FIFO 4-768");
module_param_named(dev_tx_fifo_size_6, dwc_otg_module_params.dev_tx_fifo_size[5], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_6, "Number of words in the Tx FIFO 4-768");
module_param_named(dev_tx_fifo_size_7, dwc_otg_module_params.dev_tx_fifo_size[6], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_7, "Number of words in the Tx FIFO 4-768");
module_param_named(dev_tx_fifo_size_8, dwc_otg_module_params.dev_tx_fifo_size[7], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_8, "Number of words in the Tx FIFO 4-768");
module_param_named(dev_tx_fifo_size_9, dwc_otg_module_params.dev_tx_fifo_size[8], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_9, "Number of words in the Tx FIFO 4-768");
module_param_named(dev_tx_fifo_size_10, dwc_otg_module_params.dev_tx_fifo_size[9], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_10, "Number of words in the Tx FIFO 4-768");
module_param_named(dev_tx_fifo_size_11, dwc_otg_module_params.dev_tx_fifo_size[10], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_11, "Number of words in the Tx FIFO 4-768");
module_param_named(dev_tx_fifo_size_12, dwc_otg_module_params.dev_tx_fifo_size[11], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_12, "Number of words in the Tx FIFO 4-768");
module_param_named(dev_tx_fifo_size_13, dwc_otg_module_params.dev_tx_fifo_size[12], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_13, "Number of words in the Tx FIFO 4-768");
module_param_named(dev_tx_fifo_size_14, dwc_otg_module_params.dev_tx_fifo_size[13], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_14, "Number of words in the Tx FIFO 4-768");
module_param_named(dev_tx_fifo_size_15, dwc_otg_module_params.dev_tx_fifo_size[14], int, 0444);
MODULE_PARM_DESC(dev_tx_fifo_size_15, "Number of words in the Tx FIFO 4-768");

module_param_named(thr_ctl, dwc_otg_module_params.thr_ctl, int, 0444);
MODULE_PARM_DESC(thr_ctl, "Thresholding enable flag bit 0 - non ISO Tx thr., 1 - ISO Tx thr., 2 - Rx thr.- bit 0=disabled 1=enabled");
module_param_named(tx_thr_length, dwc_otg_module_params.tx_thr_length, int, 0444);
MODULE_PARM_DESC(tx_thr_length, "Tx Threshold length in 32 bit DWORDs");
module_param_named(rx_thr_length, dwc_otg_module_params.rx_thr_length, int, 0444);
MODULE_PARM_DESC(rx_thr_length, "Rx Threshold length in 32 bit DWORDs");
/** @page "Module Parameters"
 *
 * The following parameters may be specified when starting the module.
 * These parameters define how the DWC_otg controller should be
 * configured.	Parameter values are passed to the CIL initialization
 * function dwc_otg_cil_init
 *
 * Example: <code>modprobe dwc_otg speed=1 otg_cap=1</code>
 *

 <table>
 <tr><td>Parameter Name</td><td>Meaning</td></tr>

 <tr>
 <td>otg_cap</td>
 <td>Specifies the OTG capabilities. The driver will automatically detect the
 value for this parameter if none is specified.
 - 0: HNP and SRP capable (default, if available)
 - 1: SRP Only capable
 - 2: No HNP/SRP capable
 </td></tr>

 <tr>
 <td>dma_enable</td>
 <td>Specifies whether to use slave or DMA mode for accessing the data FIFOs.
 The driver will automatically detect the value for this parameter if none is
 specified.
 - 0: Slave
 - 1: DMA (default, if available)
 </td></tr>

 <tr>
 <td>dma_burst_size</td>
 <td>The DMA Burst size (applicable only for External DMA Mode).
 - Values: 1, 4, 8 16, 32, 64, 128, 256 (default 32)
 </td></tr>

 <tr>
 <td>speed</td>
 <td>Specifies the maximum speed of operation in host and device mode. The
 actual speed depends on the speed of the attached device and the value of
 phy_type.
 - 0: High Speed (default)
 - 1: Full Speed
 </td></tr>

 <tr>
 <td>host_support_fs_ls_low_power</td>
 <td>Specifies whether low power mode is supported when attached to a Full
 Speed or Low Speed device in host mode.
 - 0: Don't support low power mode (default)
 - 1: Support low power mode
 </td></tr>

 <tr>
 <td>host_ls_low_power_phy_clk</td>
 <td>Specifies the PHY clock rate in low power mode when connected to a Low
 Speed device in host mode. This parameter is applicable only if
 HOST_SUPPORT_FS_LS_LOW_POWER is enabled.
 - 0: 48 MHz (default)
 - 1: 6 MHz
 </td></tr>

 <tr>
 <td>enable_dynamic_fifo</td>
 <td> Specifies whether FIFOs may be resized by the driver software.
 - 0: Use cC FIFO size parameters
 - 1: Allow dynamic FIFO sizing (default)
 </td></tr>

 <tr>
 <td>data_fifo_size</td>
 <td>Total number of 4-byte words in the data FIFO memory. This memory
 includes the Rx FIFO, non-periodic Tx FIFO, and periodic Tx FIFOs.
 - Values: 32 to 32768 (default 8192)

 Note: The total FIFO memory depth in the FPGA configuration is 8192.
 </td></tr>

 <tr>
 <td>dev_rx_fifo_size</td>
 <td>Number of 4-byte words in the Rx FIFO in device mode when dynamic
 FIFO sizing is enabled.
 - Values: 16 to 32768 (default 1064)
 </td></tr>

 <tr>
 <td>dev_nperio_tx_fifo_size</td>
 <td>Number of 4-byte words in the non-periodic Tx FIFO in device mode when
 dynamic FIFO sizing is enabled.
 - Values: 16 to 32768 (default 1024)
 </td></tr>

 <tr>
 <td>dev_perio_tx_fifo_size_n (n = 1 to 15)</td>
 <td>Number of 4-byte words in each of the periodic Tx FIFOs in device mode
 when dynamic FIFO sizing is enabled.
 - Values: 4 to 768 (default 256)
 </td></tr>

 <tr>
 <td>host_rx_fifo_size</td>
 <td>Number of 4-byte words in the Rx FIFO in host mode when dynamic FIFO
 sizing is enabled.
 - Values: 16 to 32768 (default 1024)
 </td></tr>

 <tr>
 <td>host_nperio_tx_fifo_size</td>
 <td>Number of 4-byte words in the non-periodic Tx FIFO in host mode when
 dynamic FIFO sizing is enabled in the core.
 - Values: 16 to 32768 (default 1024)
 </td></tr>

 <tr>
 <td>host_perio_tx_fifo_size</td>
 <td>Number of 4-byte words in the host periodic Tx FIFO when dynamic FIFO
 sizing is enabled.
 - Values: 16 to 32768 (default 1024)
 </td></tr>

 <tr>
 <td>max_transfer_size</td>
 <td>The maximum transfer size supported in bytes.
 - Values: 2047 to 65,535 (default 65,535)
 </td></tr>

 <tr>
 <td>max_packet_count</td>
 <td>The maximum number of packets in a transfer.
 - Values: 15 to 511 (default 511)
 </td></tr>

 <tr>
 <td>host_channels</td>
 <td>The number of host channel registers to use.
 - Values: 1 to 16 (default 12)

 Note: The FPGA configuration supports a maximum of 12 host channels.
 </td></tr>

 <tr>
 <td>dev_endpoints</td>
 <td>The number of endpoints in addition to EP0 available for device mode
 operations.
 - Values: 1 to 15 (default 6 IN and OUT)

 Note: The FPGA configuration supports a maximum of 6 IN and OUT endpoints in
 addition to EP0.
 </td></tr>

 <tr>
 <td>phy_type</td>
 <td>Specifies the type of PHY interface to use. By default, the driver will
 automatically detect the phy_type.
 - 0: Full Speed
 - 1: UTMI+ (default, if available)
 - 2: ULPI
 </td></tr>

 <tr>
 <td>phy_utmi_width</td>
 <td>Specifies the UTMI+ Data Width. This parameter is applicable for a
 phy_type of UTMI+. Also, this parameter is applicable only if the
 OTG_HSPHY_WIDTH cC parameter was set to "8 and 16 bits", meaning that the
 core has been configured to work at either data path width.
 - Values: 8 or 16 bits (default 16)
 </td></tr>

 <tr>
 <td>phy_ulpi_ddr</td>
 <td>Specifies whether the ULPI operates at double or single data rate. This
 parameter is only applicable if phy_type is ULPI.
 - 0: single data rate ULPI interface with 8 bit wide data bus (default)
 - 1: double data rate ULPI interface with 4 bit wide data bus
 </td></tr>

 <tr>
 <td>i2c_enable</td>
 <td>Specifies whether to use the I2C interface for full speed PHY. This
 parameter is only applicable if PHY_TYPE is FS.
 - 0: Disabled (default)
 - 1: Enabled
 </td></tr>

 <tr>
 <td>otg_en_multiple_tx_fifo</td>
 <td>Specifies whether dedicatedto tx fifos are enabled for non periodic IN EPs.
 The driver will automatically detect the value for this parameter if none is
 specified.
 - 0: Disabled
 - 1: Enabled (default, if available)
 </td></tr>

 <tr>
 <td>dev_tx_fifo_size_n (n = 1 to 15)</td>
 <td>Number of 4-byte words in each of the Tx FIFOs in device mode
 when dynamic FIFO sizing is enabled.
 - Values: 4 to 768 (default 256)
 </td></tr>

*/
