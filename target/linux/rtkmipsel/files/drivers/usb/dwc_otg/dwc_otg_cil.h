/* ==========================================================================
 * $File: //dwh/usb_iip/dev/software/otg_ipmate/linux/drivers/dwc_otg_cil.h $
 * $Revision: 1.1 $
 * $Date: 2007-11-19 05:39:07 $
 * $Change: 792294 $
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

#if !defined(__DWC_CIL_H__)
#define __DWC_CIL_H__

#include "dwc_otg_plat.h"
#include "dwc_otg_regs.h"
#ifdef DEBUG
#include "linux/timer.h"
#endif

#include "dwc_otg_cil_def.h"
/*
 * The following functions support initialization of the CIL driver component
 * and the DWC_otg controller.
 */
extern dwc_otg_core_if_t *dwc_otg_cil_init(const uint32_t *_reg_base_addr,
										   dwc_otg_core_params_t *_core_params);
extern void dwc_otg_cil_remove(dwc_otg_core_if_t *_core_if);
extern void dwc_otg_core_init(dwc_otg_core_if_t *_core_if);
extern void dwc_otg_core_host_init(dwc_otg_core_if_t *_core_if);
extern void dwc_otg_core_dev_init(dwc_otg_core_if_t *_core_if);
extern void dwc_otg_enable_global_interrupts( dwc_otg_core_if_t *_core_if );
extern void dwc_otg_disable_global_interrupts( dwc_otg_core_if_t *_core_if );

/** @name Device CIL Functions
 * The following functions support managing the DWC_otg controller in device
 * mode.
 */
/**@{*/
extern void dwc_otg_wakeup(dwc_otg_core_if_t *_core_if);
extern void dwc_otg_read_setup_packet (dwc_otg_core_if_t *_core_if, uint32_t *_dest);
extern uint32_t dwc_otg_get_frame_number(dwc_otg_core_if_t *_core_if);
extern void dwc_otg_ep0_activate(dwc_otg_core_if_t *_core_if, dwc_ep_t *_ep);
extern void dwc_otg_ep_activate(dwc_otg_core_if_t *_core_if, dwc_ep_t *_ep);
extern void dwc_otg_ep_deactivate(dwc_otg_core_if_t *_core_if, dwc_ep_t *_ep);
extern void dwc_otg_ep_start_transfer(dwc_otg_core_if_t *_core_if, dwc_ep_t *_ep);
extern void dwc_otg_ep0_start_transfer(dwc_otg_core_if_t *_core_if, dwc_ep_t *_ep);
extern void dwc_otg_ep0_continue_transfer(dwc_otg_core_if_t *_core_if, dwc_ep_t *_ep);
extern void dwc_otg_ep_write_packet(dwc_otg_core_if_t *_core_if, dwc_ep_t *_ep, int _dma);
extern void dwc_otg_ep_set_stall(dwc_otg_core_if_t *_core_if, dwc_ep_t *_ep);
extern void dwc_otg_ep_clear_stall(dwc_otg_core_if_t *_core_if, dwc_ep_t *_ep);
extern void dwc_otg_enable_device_interrupts(dwc_otg_core_if_t *_core_if);
extern void dwc_otg_dump_dev_registers(dwc_otg_core_if_t *_core_if);
/**@}*/

#ifndef DWC_DEVICE_ONLY
/** @name Host CIL Functions
 * The following functions support managing the DWC_otg controller in host
 * mode.
 */
/**@{*/
extern void dwc_otg_hc_init(dwc_otg_core_if_t *_core_if, dwc_hc_t *_hc);
extern void dwc_otg_hc_halt(dwc_otg_core_if_t *_core_if,
				dwc_hc_t *_hc,
				dwc_otg_halt_status_e _halt_status);
extern void dwc_otg_hc_cleanup(dwc_otg_core_if_t *_core_if, dwc_hc_t *_hc);
extern void dwc_otg_hc_start_transfer(dwc_otg_core_if_t *_core_if, dwc_hc_t *_hc);
extern int dwc_otg_hc_continue_transfer(dwc_otg_core_if_t *_core_if, dwc_hc_t *_hc);
extern void dwc_otg_hc_do_ping(dwc_otg_core_if_t *_core_if, dwc_hc_t *_hc);
extern void dwc_otg_hc_write_packet(dwc_otg_core_if_t *_core_if, dwc_hc_t *_hc);
extern void dwc_otg_enable_host_interrupts(dwc_otg_core_if_t *_core_if);
extern void dwc_otg_disable_host_interrupts(dwc_otg_core_if_t *_core_if);

/**
 * This function Reads HPRT0 in preparation to modify.	It keeps the
 * WC bits 0 so that if they are read as 1, they won't clear when you
 * write it back
 */
static inline uint32_t dwc_otg_read_hprt0(dwc_otg_core_if_t *_core_if)
{
	hprt0_data_t hprt0;
	hprt0.d32 = dwc_read_reg32(_core_if->host_if->hprt0);
	hprt0.b.prtena = 0;
	hprt0.b.prtconndet = 0;
	hprt0.b.prtenchng = 0;
	hprt0.b.prtovrcurrchng = 0;
	return hprt0.d32;
}
#endif
extern void dwc_otg_dump_host_registers(dwc_otg_core_if_t *_core_if);
/**@}*/

/** @name Common CIL Functions
 * The following functions support managing the DWC_otg controller in either
 * device or host mode.
 */
/**@{*/

extern void dwc_otg_read_packet(dwc_otg_core_if_t *core_if,
				uint8_t *dest,
				uint16_t bytes);

extern void dwc_otg_dump_global_registers(dwc_otg_core_if_t *_core_if);

extern void dwc_otg_flush_tx_fifo( dwc_otg_core_if_t *_core_if,
								   const int _num );
extern void dwc_otg_flush_rx_fifo( dwc_otg_core_if_t *_core_if );
extern void dwc_otg_core_reset( dwc_otg_core_if_t *_core_if );

/**
 * This function returns the Core Interrupt register.
 */
static inline uint32_t dwc_otg_read_core_intr(dwc_otg_core_if_t *_core_if)
{
	return (dwc_read_reg32(&_core_if->core_global_regs->gintsts) &
		dwc_read_reg32(&_core_if->core_global_regs->gintmsk));
}

/**
 * This function returns the OTG Interrupt register.
 */
static inline uint32_t dwc_otg_read_otg_intr (dwc_otg_core_if_t *_core_if)
{
	return (dwc_read_reg32 (&_core_if->core_global_regs->gotgint));
}

/**
 * This function reads the Device All Endpoints Interrupt register and
 * returns the IN endpoint interrupt bits.
 */
static inline uint32_t dwc_otg_read_dev_all_in_ep_intr(dwc_otg_core_if_t *_core_if)
{
	uint32_t v;
	v = dwc_read_reg32(&_core_if->dev_if->dev_global_regs->daint) &
			dwc_read_reg32(&_core_if->dev_if->dev_global_regs->daintmsk);
	return (v & 0xffff);

}

/**
 * This function reads the Device All Endpoints Interrupt register and
 * returns the OUT endpoint interrupt bits.
 */
static inline uint32_t dwc_otg_read_dev_all_out_ep_intr(dwc_otg_core_if_t *_core_if)
{
	uint32_t v;
	v = dwc_read_reg32(&_core_if->dev_if->dev_global_regs->daint) &
			dwc_read_reg32(&_core_if->dev_if->dev_global_regs->daintmsk);
	return ((v & 0xffff0000) >> 16);
}

/**
 * This function returns the Device IN EP Interrupt register
 */
static inline uint32_t dwc_otg_read_dev_in_ep_intr(dwc_otg_core_if_t *_core_if,
												   dwc_ep_t *_ep)
{
//cathy
/*	dwc_otg_dev_if_t *dev_if = _core_if->dev_if;
	uint32_t v, msk, emp;
	msk = dwc_read_reg32(&dev_if->dev_global_regs->diepmsk);
	emp = dwc_read_reg32(&dev_if->dev_global_regs->dtknqr4_fifoemptymsk);
	msk |= ((emp >> _ep->num) & 0x1) << 7;
	v = dwc_read_reg32(&dev_if->in_ep_regs[_ep->num]->diepint) & msk;
*/
#if 1  //wei add diff from cathy
	dwc_otg_dev_if_t *dev_if = _core_if->dev_if;
	uint32_t v, diepint, diepmsk;
	diepint = dwc_read_reg32(&dev_if->in_ep_regs[_ep->num]->diepint);
	diepmsk = dwc_read_reg32(&dev_if->dev_global_regs->diepmsk);
	v =  diepint & diepmsk;
#endif
	return v;
}
/**
 * This function returns the Device OUT EP Interrupt register
 */
static inline uint32_t dwc_otg_read_dev_out_ep_intr(dwc_otg_core_if_t *_core_if,
													dwc_ep_t *_ep)
{
	dwc_otg_dev_if_t *dev_if = _core_if->dev_if;
	uint32_t v;
	v = dwc_read_reg32( &dev_if->out_ep_regs[_ep->num]->doepint) &
			dwc_read_reg32(&dev_if->dev_global_regs->doepmsk);
	return v;
}

/**
 * This function returns the Host All Channel Interrupt register
 */
static inline uint32_t dwc_otg_read_host_all_channels_intr (dwc_otg_core_if_t *_core_if)
{
	return (dwc_read_reg32 (&_core_if->host_if->host_global_regs->haint));
}

static inline uint32_t dwc_otg_read_host_channel_intr (dwc_otg_core_if_t *_core_if, dwc_hc_t *_hc)
{
	return (dwc_read_reg32 (&_core_if->host_if->hc_regs[_hc->hc_num]->hcint));
}


/**
 * This function returns the mode of the operation, host or device.
 *
 * @return 0 - Device Mode, 1 - Host Mode
 */
static inline uint32_t dwc_otg_mode(dwc_otg_core_if_t *_core_if)
{
	return (dwc_read_reg32( &_core_if->core_global_regs->gintsts ) & 0x1);
}

static inline uint8_t dwc_otg_is_device_mode(dwc_otg_core_if_t *_core_if)
{
	return (dwc_otg_mode(_core_if) != DWC_HOST_MODE);
}
static inline uint8_t dwc_otg_is_host_mode(dwc_otg_core_if_t *_core_if)
{
	return (dwc_otg_mode(_core_if) == DWC_HOST_MODE);
}

extern int32_t dwc_otg_handle_common_intr( dwc_otg_core_if_t *_core_if );

extern void dwc_otg_cil_register_pcd_callbacks( dwc_otg_core_if_t *_core_if,
												dwc_otg_cil_callbacks_t *_cb,
												void *_p);
extern void dwc_otg_cil_register_hcd_callbacks( dwc_otg_core_if_t *_core_if,
												dwc_otg_cil_callbacks_t *_cb,
												void *_p);

extern int gHostMode;
#endif
