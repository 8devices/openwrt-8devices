/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Copyright (c) 2011, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_twt_public_structs.h>
#include <wlan_twt_common.h>
#include <wlan_twt_ext_type.h>
#include <wlan_twt_api.h>
#include <wlan_twt_priv.h>
#include <cfg_ucfg_api.h>
#include <init_deinit_lmac.h>

QDF_STATUS wlan_twt_cfg_init(struct wlan_objmgr_psoc *psoc)
{
	struct twt_psoc_priv_obj *twt_psoc;
	struct twt_ic_cfg_params *twt_cfg;

	if (!psoc) {
		twt_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	twt_cfg = &twt_psoc->cfg_params;

	twt_cfg->twt_enable = cfg_get(psoc, CFG_OL_TWT_ENABLE);
	twt_cfg->sta_cong_timer_ms =
			cfg_get(psoc, CFG_OL_TWT_STA_CONG_TIMER_MS);
	twt_cfg->mbss_support =
			cfg_get(psoc, CFG_OL_TWT_MBSS_SUPPORT);
	twt_cfg->default_slot_size =
			cfg_get(psoc, CFG_OL_TWT_DEFAULT_SLOT_SIZE);
	twt_cfg->congestion_thresh_setup =
			cfg_get(psoc, CFG_OL_TWT_CONGESTION_THRESH_SETUP);
	twt_cfg->congestion_thresh_teardown =
			cfg_get(psoc, CFG_OL_TWT_CONGESTION_THRESH_TEARDOWN);
	twt_cfg->congestion_thresh_critical =
			cfg_get(psoc, CFG_OL_TWT_CONGESTION_THRESH_CRITICAL);
	twt_cfg->interference_thresh_teardown =
			cfg_get(psoc, CFG_OL_TWT_INTERFERENCE_THRESH_TEARDOWN);
	twt_cfg->interference_thresh_setup =
			cfg_get(psoc, CFG_OL_TWT_INTERFERENCE_THRESH_SETUP);
	twt_cfg->min_no_sta_setup =
			cfg_get(psoc, CFG_OL_TWT_MIN_NUM_STA_SETUP);
	twt_cfg->min_no_sta_teardown =
			cfg_get(psoc, CFG_OL_TWT_MIN_NUM_STA_TEARDOWN);
	twt_cfg->no_of_bcast_mcast_slots =
			cfg_get(psoc, CFG_OL_TWT_NUM_BCMC_SLOTS);
	twt_cfg->min_no_twt_slots =
			cfg_get(psoc, CFG_OL_TWT_MIN_NUM_SLOTS);
	twt_cfg->max_no_sta_twt =
			cfg_get(psoc, CFG_OL_TWT_MAX_NUM_STA_TWT);
	twt_cfg->mode_check_interval =
			cfg_get(psoc, CFG_OL_TWT_MODE_CHECK_INTERVAL);
	twt_cfg->add_sta_slot_interval =
			cfg_get(psoc, CFG_OL_TWT_ADD_STA_SLOT_INTERVAL);
	twt_cfg->remove_sta_slot_interval =
			cfg_get(psoc, CFG_OL_TWT_REMOVE_STA_SLOT_INTERVAL);
	twt_cfg->b_twt_enable =
			cfg_get(psoc, CFG_OL_B_TWT_ENABLE);

	qdf_info("ME-DBG");
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_bcast(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc;
	psoc_twt_ext_cfg_params_t *twt_cfg;

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	twt_cfg = &twt_psoc->cfg_params;
	*val = twt_cfg->b_twt_enable;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_requestor(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc;
	psoc_twt_ext_cfg_params_t *twt_cfg;
	struct twt_tgt_caps *twt_caps;

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	twt_cfg = &twt_psoc->cfg_params;
	twt_caps = &twt_psoc->twt_caps;
	*val = twt_cfg->twt_enable && twt_caps->twt_requestor;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_responder(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc;
	psoc_twt_ext_cfg_params_t *twt_cfg;
	struct twt_tgt_caps *twt_caps;

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	twt_cfg = &twt_psoc->cfg_params;
	twt_caps = &twt_psoc->twt_caps;
	*val = twt_cfg->twt_enable && twt_caps->twt_responder;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_congestion_timeout(struct wlan_objmgr_psoc *psoc,
				    uint32_t *val)
{
	struct twt_psoc_priv_obj *twt_psoc;
	psoc_twt_ext_cfg_params_t *twt_cfg;

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	twt_cfg = &twt_psoc->cfg_params;
	*val = twt_cfg->sta_cong_timer_ms;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_bcast_requestor(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc;
	psoc_twt_ext_cfg_params_t *twt_cfg;

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	twt_cfg = &twt_psoc->cfg_params;
	*val = twt_cfg->b_twt_enable;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_bcast_responder(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc;
	psoc_twt_ext_cfg_params_t *twt_cfg;

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	twt_cfg = &twt_psoc->cfg_params;
	*val = twt_cfg->b_twt_enable;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_set_requestor_flag(struct wlan_objmgr_psoc *psoc, bool val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_set_responder_flag(struct wlan_objmgr_psoc *psoc, bool val)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_twt_send_enable_cmd(struct wlan_objmgr_pdev *pdev)
{

	struct wlan_objmgr_psoc *psoc;
	struct twt_psoc_priv_obj *twt_psoc;
	psoc_twt_ext_cfg_params_t *twt_cfg;
	struct twt_enable_param twt_param;

	if (!pdev) {
		twt_err("null pdev");
		return QDF_STATUS_E_FAILURE;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	twt_cfg = &twt_psoc->cfg_params;

	if (!twt_cfg->twt_enable) {
		qdf_info("TWT INI is disabled. Do not send enable cmd to FW");
		return QDF_STATUS_E_FAILURE;
	}

	twt_param.pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	twt_param.sta_cong_timer_ms = twt_cfg->sta_cong_timer_ms;
	twt_param.mbss_support = twt_cfg->mbss_support;
	twt_param.default_slot_size = twt_cfg->default_slot_size;
	twt_param.congestion_thresh_setup =
				twt_cfg->congestion_thresh_setup;
	twt_param.congestion_thresh_teardown =
				twt_cfg->congestion_thresh_teardown;
	twt_param.congestion_thresh_critical =
				twt_cfg->congestion_thresh_critical;
	twt_param.interference_thresh_teardown =
				twt_cfg->interference_thresh_teardown;
	twt_param.interference_thresh_setup =
				twt_cfg->interference_thresh_setup;
	twt_param.min_no_sta_setup =
				twt_cfg->min_no_sta_setup;
	twt_param.min_no_sta_teardown =
				twt_cfg->min_no_sta_teardown;
	twt_param.no_of_bcast_mcast_slots =
				twt_cfg->no_of_bcast_mcast_slots;
	twt_param.min_no_twt_slots =
				twt_cfg->min_no_twt_slots;
	twt_param.max_no_sta_twt =
				twt_cfg->max_no_sta_twt;
	twt_param.mode_check_interval =
				twt_cfg->mode_check_interval;
	twt_param.add_sta_slot_interval =
		twt_cfg->add_sta_slot_interval;
	twt_param.remove_sta_slot_interval =
		twt_cfg->remove_sta_slot_interval;
	twt_param.b_twt_enable = twt_cfg->b_twt_enable;

	return wlan_twt_responder_enable(psoc, &twt_param, NULL);
}
