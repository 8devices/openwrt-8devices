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

/**
 * DOC: wlan_twt_ucfg.h
 */
#ifndef _WLAN_TWT_UCFG_H_
#define _WLAN_TWT_UCFG_H_

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
/**
 * ucfg_twt_cfg_init() - ucfg API to set up TWT CFG params
 * @psoc: pointer to psoc
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS ucfg_twt_cfg_init(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_twt_enable_cmd) - ucfg API to enable TWT for a pdev
 * @pdev: pointer to pdev
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS ucfg_twt_enable_cmd(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_twt_get_bcast) - ucfg API to get broadcast enable value
 * @psoc: pointer to psoc
 * @val: pointer reference to retrieved value

 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS ucfg_twt_cfg_get_bcast(struct wlan_objmgr_psoc *psoc, bool *val);

/**
 * ucfg_twt_get_requestor) - ucfg API to get requestor value
 * @psoc: pointer to psoc
 * @val: pointer reference to retrieved value

 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS
ucfg_twt_cfg_get_requestor(struct wlan_objmgr_psoc *psoc, bool *val);

/**
 *ucfg_twt_get_responder) - ucfg API to get responder value
 * @psoc: pointer to psoc
 * @val: pointer reference to retrieved value

 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS
ucfg_twt_cfg_get_responder(struct wlan_objmgr_psoc *psoc, bool *val);

#endif
#endif /*_WLAN_TWT_UCFG_H_ */

