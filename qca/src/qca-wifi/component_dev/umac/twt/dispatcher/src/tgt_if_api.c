/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <wlan_lmac_if_def.h>

QDF_STATUS
target_if_twt_register_ext_tx_ops(struct wlan_lmac_if_twt_tx_ops *twt_tx_ops)
{
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(target_if_twt_register_ext_tx_ops);

void tgt_twt_register_ext_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
}

QDF_STATUS
target_if_twt_register_ext_events(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(target_if_twt_register_ext_events);

QDF_STATUS
target_if_twt_deregister_ext_events(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(target_if_twt_deregister_ext_events);
