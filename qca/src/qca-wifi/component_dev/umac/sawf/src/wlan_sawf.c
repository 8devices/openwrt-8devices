/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

/**
 * DOC: wlan_sawf.c
 * This file defines functions needed by the SAWF framework.
 */

#include "wlan_sawf.h"
#include <qdf_util.h>
#include <qdf_types.h>
#include <qdf_mem.h>
#include <qdf_trace.h>
#include <qdf_module.h>

static struct sawf_ctx *g_wlan_sawf_ctx;

QDF_STATUS wlan_sawf_init(void)
{
	if (g_wlan_sawf_ctx) {
		qdf_err("SAWF global context is already allocated");
		return QDF_STATUS_E_FAILURE;
	}

	g_wlan_sawf_ctx = qdf_mem_malloc(sizeof(struct sawf_ctx));
	if (!g_wlan_sawf_ctx) {
		qdf_err("Mem alloc failed for SAWF context");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_info("SAWF: SAWF ctx is initialized");
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_sawf_deinit(void)
{
	if (!g_wlan_sawf_ctx) {
		qdf_err("SAWF gloabl context is already freed");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_free(g_wlan_sawf_ctx);

	return QDF_STATUS_SUCCESS;
}

struct sawf_ctx *wlan_get_sawf_ctx(void)
{
	return g_wlan_sawf_ctx;
}
qdf_export_symbol(wlan_get_sawf_ctx);

void wlan_print_service_class(struct wlan_sawf_scv_class_params *params)
{
	qdf_info(SAWF_LINE_FORMAT);
	qdf_info("Service ID       :%d", params->svc_id);
	qdf_info("App Name         :%s", params->app_name);
	qdf_info("Min througput    :%d", params->min_thruput_rate);
	qdf_info("Max throughput   :%d", params->max_thruput_rate);
	qdf_info("Burst Size       :%d", params->burst_size);
	qdf_info("Service Interval :%d", params->service_interval);
	qdf_info("Delay Bound      :%d", params->delay_bound);
	qdf_info("MSDU TTL         :%d", params->msdu_ttl);
	qdf_info("Priority         :%d", params->priority);
	qdf_info("TID              :%d", params->tid);
	qdf_info("MSDU Loss Rate   :%d", params->msdu_rate_loss);
}

qdf_export_symbol(wlan_print_service_class);

bool wlan_service_id_valid(uint8_t svc_id)
{
	if (svc_id <  SAWF_SVC_CLASS_MIN || svc_id > SAWF_SVC_CLASS_MAX)
		return false;
	else
		return true;
}

qdf_export_symbol(wlan_service_id_valid);

bool wlan_service_id_configured(uint8_t svc_id)
{
	struct sawf_ctx *sawf;

	sawf = wlan_get_sawf_ctx();
	if (!sawf) {
		qdf_err("SAWF ctx is invalid");
		return false;
	}

	if (!(sawf->svc_classes[svc_id - 1].configured))
		return false;

	return true;
}

qdf_export_symbol(wlan_service_id_configured);

void wlan_update_sawf_params(struct wlan_sawf_scv_class_params *params)
{
	struct sawf_ctx *sawf;
	struct wlan_sawf_scv_class_params *new_param;

	sawf = wlan_get_sawf_ctx();
	if (!sawf) {
		qdf_err("SAWF ctx is invalid");
		return;
	}

	new_param = &sawf->svc_classes[params->svc_id - 1];
	new_param->svc_id = params->svc_id;
	new_param->min_thruput_rate = params->min_thruput_rate;
	new_param->max_thruput_rate = params->max_thruput_rate;
	new_param->burst_size = params->burst_size;
	new_param->service_interval = params->service_interval;
	new_param->delay_bound = params->delay_bound;
	new_param->msdu_ttl = params->msdu_ttl;
	new_param->priority = params->priority;
	new_param->tid = params->tid;
	new_param->msdu_rate_loss = params->msdu_rate_loss;
}

qdf_export_symbol(wlan_update_sawf_params);

QDF_STATUS wlan_validate_sawf_params(struct wlan_sawf_scv_class_params *params)
{
	uint32_t value;

	value = params->min_thruput_rate;
	if (value != SAWF_DEF_PARAM_VAL && (value < SAWF_MIN_MIN_THROUGHPUT ||
	    value > SAWF_MAX_MIN_THROUGHPUT)) {
		qdf_err("Invalid Min throughput: %d", value);
		return QDF_STATUS_E_FAILURE;
	}

	value = params->max_thruput_rate;
	if (value != SAWF_DEF_PARAM_VAL && (value < SAWF_MIN_MAX_THROUGHPUT ||
	    value > SAWF_MAX_MAX_THROUGHPUT)) {
		qdf_err("Invalid Max througput: %d", value);
		return QDF_STATUS_E_FAILURE;
	}

	value = params->burst_size;
	if (value != SAWF_DEF_PARAM_VAL && (value < SAWF_MIN_BURST_SIZE ||
	    value > SAWF_MAX_BURST_SIZE)) {
		qdf_err("Invalid Burst Size: %d", value);
		return QDF_STATUS_E_FAILURE;
	}

	value = params->delay_bound;
	if (value != SAWF_DEF_PARAM_VAL && (value < SAWF_MIN_DELAY_BOUND
	    || value > SAWF_MAX_DELAY_BOUND)) {
		qdf_err("Invalid Delay Bound: %d", value);
		return QDF_STATUS_E_FAILURE;
	}

	value = params->service_interval;
	if (value != SAWF_DEF_PARAM_VAL && (value < SAWF_MIN_SVC_INTERVAL ||
	    value > SAWF_MAX_SVC_INTERVAL)) {
		qdf_err("Invalid Service Interval: %d", value);
		return QDF_STATUS_E_FAILURE;
	}

	value = params->msdu_ttl;
	if (value != SAWF_DEF_PARAM_VAL && (value < SAWF_MIN_MSDU_TTL ||
	    value > SAWF_MAX_MSDU_TTL)) {
		qdf_err("Invalid MSDU TTL: %d", value);
		return QDF_STATUS_E_FAILURE;
	}

	value = params->priority;
	if (value != SAWF_DEF_PARAM_VAL && (value < SAWF_MIN_PRIORITY ||
	    value > SAWF_MAX_PRIORITY)) {
		qdf_err("Invalid Priority: %d", value);
		return QDF_STATUS_E_FAILURE;
	}

	value = params->tid;
	if (value != SAWF_DEF_PARAM_VAL && (value < SAWF_MIN_TID ||
	    value > SAWF_MAX_TID)) {
		qdf_err("Invalid TID %d", value);
		return QDF_STATUS_E_FAILURE;
	}

	value = params->msdu_rate_loss;
	if (value != SAWF_DEF_PARAM_VAL && (value < SAWF_MIN_MSDU_LOSS_RATE ||
	    value > SAWF_MAX_MSDU_LOSS_RATE)) {
		qdf_err("Invalid MSDU Loss rate: %d", value);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(wlan_validate_sawf_params);

