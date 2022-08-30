/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
#include "dp_peer.h"
#include "qdf_nbuf.h"
#include "dp_types.h"
#include "dp_internal.h"
#include "dp_tx.h"
#include "dp_mesh_latency.h"
#include <target_if.h>
#include <wmi_unified_ap_api.h>

#define DP_MESH_LATENCY_VALID_TID_SHIFT 0x0
#define DP_MESH_LATENCY_VALID_TID_MASK 0x7
#define DP_MESH_LATENCY_VALID_MSDUQ_SHIFT 0x4
#define DP_MESH_LATENCY_VALID_MSDUQ_MASK 0xF

/**
 * dp_mesh_latency_get_ac_frm_service_intervald() - convert from
 * service interval to custom ac
 * @tid - TID to be converted to service interval
 */
static int
dp_mesh_latency_get_ac_frm_service_interval(uint32_t *service_interval)
{

	if (*service_interval <= DP_MESH_LATENCY_SERVICE_INTERVAL_10MS) {
		*service_interval = DP_MESH_LATENCY_SERVICE_INTERVAL_10MS;
		return WMI_HOST_AC_VO;
	} else if (*service_interval <= DP_MESH_LATENCY_SERVICE_INTERVAL_20MS) {
		*service_interval = DP_MESH_LATENCY_SERVICE_INTERVAL_20MS;
		return WMI_HOST_AC_VO;
	} else if (*service_interval <= DP_MESH_LATENCY_SERVICE_INTERVAL_40MS) {
		*service_interval = DP_MESH_LATENCY_SERVICE_INTERVAL_40MS;
		return WMI_HOST_AC_VI;
	} else if (*service_interval >= DP_MESH_LATENCY_SERVICE_INTERVAL_80MS) {
		*service_interval = DP_MESH_LATENCY_SERVICE_INTERVAL_80MS;
		return WMI_HOST_AC_BE;
	}

	return WMI_HOST_AC_BE;
}

/**
 * dp_mesh_latency_update_peer_parameter() - Update peer mesh latency parameter
 * @soc_hdl - soc handle
 * @peer_mac - destination mac address
 * @dl_burst_size - per peer additive burst size for DL
 * @dl_service_interval - per peer service interval for DL
 * @ul_burst_size - per peer additive burst size for UL
 * @ul_service_interval - per peer service interval for UL
 * @priority - user priority/TID
 * @add_or_sub - bit to Addition/Substraction of latency parameters
 *
 * Return: 0 when peer has active mscs session and valid user priority
 */
QDF_STATUS dp_mesh_latency_update_peer_parameter(struct cdp_soc_t *soc_hdl,
		uint8_t *peer_mac, uint32_t service_interval_dl,
		uint32_t burst_size_dl, uint32_t service_interval_ul,
		uint32_t burst_size_ul, uint16_t priority, uint8_t add_or_sub)
{
	struct dp_peer *peer = NULL;
	struct dp_vdev *vdev;
	QDF_STATUS status = 0;
	uint8_t tid, ac, msduq;
	struct dp_soc *dpsoc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct cdp_soc_t *cdp_soc = NULL;
	struct dp_ast_entry *ase = NULL;
	if (!dpsoc) {
		QDF_TRACE(QDF_MODULE_ID_DP_CDP, QDF_TRACE_LEVEL_ERROR,
				"%s: Invalid soc\n", __func__);
		return QDF_STATUS_E_INVAL;
	}

	cdp_soc = &dpsoc->cdp_soc;
	if (!dpsoc->ast_offload_support) {
		qdf_spin_lock_bh(&dpsoc->ast_lock);
		ase = dp_peer_ast_hash_find_soc(dpsoc, peer_mac);
		if (!ase) {
			qdf_spin_unlock_bh(&dpsoc->ast_lock);
			return QDF_STATUS_E_INVAL;
		}

		qdf_spin_unlock_bh(&dpsoc->ast_lock);
		peer = dp_peer_get_ref_by_id(dpsoc, ase->peer_id,
					DP_MOD_ID_AST);
	} else {
		QDF_TRACE(QDF_MODULE_ID_DP_CDP, QDF_TRACE_LEVEL_ERROR,
				"%s: Mesh Latency support for WIFI7 peers need to be added\n", __func__);
	}

	if (!peer) {
		return QDF_STATUS_E_INVAL;
	}
	vdev = peer->vdev;
	if (!vdev) {
		/*
		 * No VAP found with this peer mac
		 */
		dp_peer_unref_delete(peer, DP_MOD_ID_AST);
		return QDF_STATUS_E_INVAL;
	}

	/*
	 * Derive TID and MSDUq from priority
	 */
	tid = (priority >> DP_MESH_LATENCY_VALID_TID_SHIFT)
					& DP_MESH_LATENCY_VALID_TID_MASK;
	msduq = (priority >> DP_MESH_LATENCY_VALID_MSDUQ_SHIFT)
					& DP_MESH_LATENCY_VALID_MSDUQ_MASK;

	ac = TID_TO_WME_AC(tid);;

	/*
	 * Update per TID peer mesh latency related parameters
	 */
	peer->mesh_latency_params[tid].service_interval_dl = service_interval_dl;
	peer->mesh_latency_params[tid].burst_size_dl = burst_size_dl;
	peer->mesh_latency_params[tid].ac = ac;
	peer->mesh_latency_params[tid].msduq = msduq;

	status = cdp_soc->ol_ops->peer_update_mesh_latency_params(dpsoc->ctrl_psoc,
			vdev->vdev_id, peer_mac, tid, service_interval_dl,
			burst_size_dl, service_interval_ul, burst_size_ul, add_or_sub, ac);
	/*
	 * Unref the peer
	 */
	dp_peer_unref_delete(peer, DP_MOD_ID_AST);
	return status;
}

qdf_export_symbol(dp_mesh_latency_update_peer_parameter);
