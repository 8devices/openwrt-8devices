/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _ADPT_APPE_SHAPER_H_
#define _ADPT_APPE_SHAPER_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#define APPE_SHAPER_FLOW_ID_MIN           0
#define APPE_SHAPER_FLOW_ID_MAX           63
#define APPE_SHAPER_QUEUE_ID_MIN          0
#define APPE_SHAPER_QUEUE_ID_MAX          299

sw_error_t
adpt_appe_queue_shaper_ctrl_set(a_uint32_t dev_id,
	fal_shaper_ctrl_t *queue_shaper_ctrl);

sw_error_t
adpt_appe_flow_shaper_ctrl_set(a_uint32_t dev_id,
	fal_shaper_ctrl_t *flow_shaper_ctrl);

#ifndef IN_SHAPER_MINI
sw_error_t
adpt_appe_queue_shaper_ctrl_get(a_uint32_t dev_id,
	fal_shaper_ctrl_t *queue_shaper_ctrl);

sw_error_t
adpt_appe_flow_shaper_ctrl_get(a_uint32_t dev_id,
	fal_shaper_ctrl_t *flow_shaper_ctrl);
#endif
#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif
