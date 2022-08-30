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
 * DOC: wlan_sawf.h
 * This file defines data structure &  prototypes of the functions
 * needed by the SAWF framework.
 */

#ifndef _WLAN_SAWF_H_
#define _WLAN_SAWF_H_

#include <qdf_status.h>

#define SAWF_SVC_CLASS_MIN 1
#define SAWF_SVC_CLASS_MAX 128
#define WLAN_MAX_SVC_CLASS_NAME 64

#define SAWF_LINE_FORMAT "================================================"

#define SAWF_DEF_PARAM_VAL 0xFFFFFFFF
/*
 * Min throughput limit 0 - 10gbps
 * Granularity: 1Kbps
 */
#define SAWF_MIN_MIN_THROUGHPUT 0
#define SAWF_MAX_MIN_THROUGHPUT (10 * 1204 * 1024)

/*
 * Max throughput limit 0 - 10gbps.
 * Granularity: 1Kbps
 */
#define SAWF_MIN_MAX_THROUGHPUT 0
#define SAWF_MAX_MAX_THROUGHPUT (10 * 1204 * 1024)

/*
 * Service interval limit 0 - 10secs.
 * Granularity: 100µs
 */
#define SAWF_MIN_SVC_INTERVAL 0
#define SAWF_MAX_SVC_INTERVAL (10 * 100 * 100)

/*
 * Burst size 0 - 16Mbytes.
 * Granularity: 1byte
 */
#define SAWF_MIN_BURST_SIZE 0
#define SAWF_MAX_BURST_SIZE (16 * 1024 * 1024)

/*
 * Delay bound limit 0 - 10secs
 * Granularity: 100µs
 */
#define SAWF_MIN_DELAY_BOUND 0
#define SAWF_MAX_DELAY_BOUND (10 * 100 * 100)

/*
 * Msdu TTL limit 0 - 10secs.
 * Granularity: 100µs
 */
#define SAWF_MIN_MSDU_TTL 0
#define SAWF_MAX_MSDU_TTL (10 * 100 * 100)

/*
 * Priority limit 0 - 127.
 */
#define SAWF_MIN_PRIORITY 0
#define SAWF_MAX_PRIORITY 127

/*
 * TID limit 0 - 7
 */
#define SAWF_MIN_TID 0
#define SAWF_MAX_TID 7

/*
 * MSDU Loss Rate limit 0 - 1000.
 * Granularity: 0.01%
 */
#define SAWF_MIN_MSDU_LOSS_RATE 0
#define SAWF_MAX_MSDU_LOSS_RATE 10000

/**
 * struct wlan_sawf_scv_class_params- Service Class Parameters
 * @svc_id: Service ID
 * @app_name: Service class name
 * @min_thruput_rate: min throughput in kilobits per second
 * @max_thruput_rate: max throughput in kilobits per second
 * @burst_size:  burst size in bytes
 * @service_interval: service interval
 * @delay_bound: delay bound in in milli seconds
 * @msdu_ttl: MSDU Time-To-Live
 * @priority: Priority
 * @tid: TID
 * @msdu_rate_loss: MSDU loss rate in parts per million
 * @configured: indicating if the serivice class is configured.
 */

struct wlan_sawf_scv_class_params {
	uint8_t svc_id;
	char app_name[WLAN_MAX_SVC_CLASS_NAME];
	uint32_t min_thruput_rate;
	uint32_t max_thruput_rate;
	uint32_t burst_size;
	uint32_t service_interval;
	uint32_t delay_bound;
	uint32_t msdu_ttl;
	uint32_t priority;
	uint32_t tid;
	uint32_t msdu_rate_loss;
	bool configured;
};

/**
 * struct sawf_ctx- SAWF context
 * @svc_classes: List of all service classes
 */
struct sawf_ctx {
	struct wlan_sawf_scv_class_params svc_classes[SAWF_SVC_CLASS_MAX];
};

/* wlan_sawf_init() - Initialize SAWF subsytem
 *
 * Initialize the SAWF context
 *
 * Return: QDF_STATUS
 */

QDF_STATUS wlan_sawf_init(void);

/* wlan_sawf_deinit() - Deinitialize SAWF subsystem
 *
 * Deinnitialize the SAWF context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_sawf_deinit(void);

/* wlan_get_sawf_ctx() - Get service aware wifi context
 *
 * Get Service Aware Wifi Context
 *
 * Return: SAWF context
 */
struct sawf_ctx *wlan_get_sawf_ctx(void);

/* wlan_service_id_valid() - Validate the service ID
 *
 * Validate the service ID
 *
 * Return: true or false
 */
bool wlan_service_id_valid(uint8_t svc_id);

/* wlan_service_id_configured() - Is service ID configured
 *
 * Is the service ID configured
 *
 * Return: true or false
 */
bool wlan_service_id_configured(uint8_t svc_id);

/* wlan_print_service_class() - Print service class params
 *
 * Print service class params
 *
 * Return: none
 */
void wlan_print_service_class(struct wlan_sawf_scv_class_params *params);

/* wlan_update_sawf_params() - Update service class params
 *
 * Update service class params
 *
 * Return: none
 */
void wlan_update_sawf_params(struct wlan_sawf_scv_class_params *params);

/* wlan_validate_sawf_params() - Validate service class params
 *
 * Validate service class params
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_validate_sawf_params(struct wlan_sawf_scv_class_params *params);

#endif

