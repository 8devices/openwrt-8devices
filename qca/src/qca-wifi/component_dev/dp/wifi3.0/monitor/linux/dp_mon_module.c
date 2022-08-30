/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/module.h>
#include <osdep.h>
#include <wlan_objmgr_global_obj_i.h>
#include <wlan_objmgr_psoc_obj.h>
#include <dp_types.h>
#include <dp_internal.h>
#include <dp_htt.h>
#include <dp_mon.h>
#include <dp_types.h>

#ifndef EXPORT_SYMTAB
#define EXPORT_SYMTAB
#endif

MODULE_LICENSE("Dual BSD/GPL");

extern ol_ath_soc_softc_t *ol_global_soc[GLOBAL_SOC_SIZE];
extern int ol_num_global_soc;
#ifdef ATH_AHB
extern int wifi_exit_in_progress;
#endif
QDF_STATUS mon_soc_ol_attach(struct wlan_objmgr_psoc *psoc);
void mon_soc_ol_detach(struct wlan_objmgr_psoc *psoc);

static inline QDF_STATUS
dp_mon_soc_ring_config(struct dp_soc *soc)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_mon_ops *mon_ops = dp_mon_ops_get(soc);

	if (mon_ops && mon_ops->mon_soc_htt_srng_setup)
		status = mon_ops->mon_soc_htt_srng_setup(soc);

	return status;
}

static inline QDF_STATUS
dp_mon_ring_config(struct dp_soc *soc, struct dp_pdev *pdev,
		   int mac_for_pdev)
{
	int lmac_id;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_mon_ops *mon_ops = dp_mon_ops_get(soc);

	lmac_id = dp_get_lmac_id_for_pdev_id(soc, 0, mac_for_pdev);
	if (mon_ops && mon_ops->mon_pdev_htt_srng_setup)
		status = mon_ops->mon_pdev_htt_srng_setup(soc, pdev,
						     lmac_id, mac_for_pdev);

	return status;
}

#ifndef QCA_SINGLE_WIFI_3_0
static int __init monitor_mod_init(void)
#else
int monitor_mod_init(void)
#endif
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct dp_mon_soc *mon_soc;
	struct dp_soc *soc;
	struct dp_pdev *pdev;
	uint8_t index = 0;
	uint8_t pdev_id = 0;
	uint8_t pdev_count = 0;
	bool pdev_attach_success;
	struct dp_mon_ops *mon_ops = NULL;
	qdf_size_t soc_context_size;

	for (index = 0; index < WLAN_OBJMGR_MAX_DEVICES; index++) {
		psoc = g_umac_glb_obj->psoc[index];
		if (!psoc)
			continue;

		soc = wlan_psoc_get_dp_handle(psoc);
		if (!soc) {
			dp_mon_err("dp_soc is NULL, psoc = %pK", psoc);
			continue;
		}

		if (soc->arch_ops.txrx_get_mon_context_size) {
			soc_context_size = soc->arch_ops.txrx_get_mon_context_size(DP_CONTEXT_TYPE_MON_SOC);
			mon_soc = dp_context_alloc_mem(soc, DP_MON_SOC_TYPE, soc_context_size);
		} else {
			mon_soc = (struct dp_mon_soc *)qdf_mem_malloc(sizeof(*mon_soc));
		}

		if (!mon_soc) {
			dp_mon_err("%pK: mem allocation failed", soc);
			continue;
		}

		soc->monitor_soc = mon_soc;
		dp_mon_soc_cfg_init(soc);
		pdev_attach_success = false;
		dp_mon_ops_register(soc);
		mon_ops = dp_mon_ops_get(soc);
		if (!mon_ops) {
			qdf_err("%pK: mem allocation failed", soc);
			qdf_mem_free(mon_soc);
			soc->monitor_soc = NULL;
			continue;
		}

		if (mon_ops->mon_soc_attach) {
			if (mon_ops->mon_soc_attach(soc)) {
				dp_mon_err("%pK: monitor soc attach failed", soc);
			}
		}

		if (mon_ops->mon_soc_init) {
			if (mon_ops->mon_soc_init(soc)) {
				dp_mon_err("%pK: monitor soc init failed", soc);
			}
		}

		status = dp_mon_soc_ring_config(soc);
		if (status != QDF_STATUS_SUCCESS) {
			dp_mon_err("%pK: monitor soc ring config failed", soc);
			qdf_mem_free(mon_soc);
			soc->monitor_soc = NULL;
			continue;
		}

		pdev_count = psoc->soc_objmgr.wlan_pdev_count;
		for (pdev_id = 0; pdev_id < pdev_count; pdev_id++) {
			pdev = soc->pdev_list[pdev_id];
			if (!pdev)
				continue;

			status = dp_mon_pdev_attach(pdev);
			if (status != QDF_STATUS_SUCCESS) {
				dp_mon_err("mon pdev attach failed, dp pdev = %pK",
					pdev);
				continue;
			}
			status = dp_mon_pdev_init(pdev);
			if (status != QDF_STATUS_SUCCESS) {
				dp_mon_err("mon pdev init failed, dp pdev = %pK",
					pdev);
				dp_mon_pdev_detach(pdev);
				continue;
			}

			status = dp_mon_ring_config(soc, pdev, pdev_id);
			if (status != QDF_STATUS_SUCCESS) {
				dp_mon_err("mon ring config failed, dp pdev = %pK",
					pdev);
				dp_mon_pdev_deinit(pdev);
				dp_mon_pdev_detach(pdev);
				continue;
			}
			dp_monitor_tx_capture_debugfs_init(pdev);
			pdev_attach_success = true;
		}
		if (!pdev_attach_success) {
			dp_mon_err("mon attach failed for all, dp soc = %pK",
				soc);
			soc->monitor_soc = NULL;
			qdf_mem_free(mon_soc);
			continue;
		}
		mon_soc_ol_attach(psoc);
		dp_mon_register_feature_ops(soc);
		dp_mon_register_intr_ops(soc);
		dp_mon_cdp_ops_register(soc);
	}
	return 0;
}

#ifndef QCA_SINGLE_WIFI_3_0
module_init(monitor_mod_init);
#endif
/**
 * monitor_mod_exit() - module remove
 *
 * Return: void
 */
#ifndef QCA_SINGLE_WIFI_3_0
static void __exit monitor_mod_exit(void)
#else
void monitor_mod_exit(void)
#endif
{
	struct wlan_objmgr_psoc *psoc;
	struct dp_soc *soc;
	struct dp_pdev *pdev;
	struct dp_mon_soc *mon_soc;
	uint8_t index = 0;
	uint8_t pdev_id = 0;
	uint8_t pdev_count = 0;
	struct dp_mon_ops *mon_ops;
	ol_ath_soc_softc_t *temp_soc = NULL;

	for (index = 0; index < ol_num_global_soc; index++) {
		temp_soc = ol_global_soc[index];
		if (!temp_soc)
			continue;
		while (qdf_atomic_test_bit(SOC_RESET_IN_PROGRESS_BIT,
					   &temp_soc->reset_in_progress)) {
			dp_mon_err("Reset in progress SoC ID %d! waiting",
				   index);
			qdf_sleep(100);
		}
	}
#ifdef ATH_AHB
	wifi_exit_in_progress = 1;
#endif

	for (index = 0; index < WLAN_OBJMGR_MAX_DEVICES; index++) {
		psoc = g_umac_glb_obj->psoc[index];
		if (!psoc)
			continue;

		soc = wlan_psoc_get_dp_handle(psoc);
		if (!soc) {
			dp_mon_err("dp_soc is NULL");
			continue;
		}

		if (!soc->monitor_soc)
			continue;

		dp_soc_reset_mon_intr_mask(soc);
		/* add delay before proceeding to allow any pending
		 * mon process to complete */
		qdf_mdelay(500);

		mon_soc_ol_detach(psoc);
		dp_mon_cdp_ops_deregister(soc);
		dp_mon_intr_ops_deregister(soc);

		pdev_count = psoc->soc_objmgr.wlan_pdev_count;
		for (pdev_id = 0; pdev_id < pdev_count; pdev_id++) {
			pdev = soc->pdev_list[pdev_id];
			if (!pdev || !pdev->monitor_pdev)
				continue;

			dp_mon_pdev_deinit(pdev);
			dp_mon_pdev_detach(pdev);
		}
		dp_mon_feature_ops_deregister(soc);

		mon_ops = dp_mon_ops_get(soc);

		if (mon_ops && mon_ops->mon_soc_deinit)
			mon_ops->mon_soc_deinit(soc);

		if (mon_ops && mon_ops->mon_soc_detach)
			mon_ops->mon_soc_detach(soc);

		dp_mon_ops_free(soc);

		mon_soc = soc->monitor_soc;
		soc->monitor_soc = NULL;
		qdf_mem_free(mon_soc);
	}
}

#ifndef QCA_SINGLE_WIFI_3_0
module_exit(monitor_mod_exit);
#endif
