/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __LICENSE_MANAGER_H__
#define __LICENSE_MANAGER_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/soc/qcom/qmi.h>
#include <linux/of_device.h>

#define QMI_LICENSE_MANAGER_SERVICE_MAX_MSG_LEN 10259
#define MAX_NUM_OF_LICENSES 10

#define LM_SERVICE_ID_V01 0x0423
#define LM_SERVICE_VERS_V01 0x01

#define QMI_LM_DOWNLOAD_REQ_V01 0x0101
#define QMI_LM_GET_TERMINATION_MODE_RESP_V01 0x0100
#define QMI_LM_GET_TERMINATION_MODE_REQ_V01 0x0100
#define QMI_LM_DOWNLOAD_RESP_V01 0x0101

#define QMI_MAX_CHIPINFO_ID_LEN_V01 32
#define QMI_MAX_LICENSE_SIZE_V01 10240

enum lm_termination_mode_v01 {
	LM_TERMINATION_MODE_MIN_VAL_V01 = INT_MIN,
	QMI_LICENSE_TERMINATION_AT_HOST_V01 = 0xDEAD,
	QMI_LICENSE_TERMINATION_AT_DEVICE_V01 = 0xDAED,
	LM_TERMINATION_MODE_MAX_VAL_V01 = INT_MAX,
};

struct lm_get_termination_mode_req_msg_v01 {
	u64 timestamp;
	u32 id;
	char name[QMI_MAX_CHIPINFO_ID_LEN_V01];
	u32 serialnum;
};
#define LM_GET_TERMINATION_MODE_REQ_MSG_V01_MAX_MSG_LEN 60

struct lm_get_termination_mode_resp_msg_v01 {
	enum lm_termination_mode_v01 termination_mode;
	u32 reserved;
};
#define LM_GET_TERMINATION_MODE_RESP_MSG_V01_MAX_MSG_LEN 14

struct lm_download_license_req_msg_v01 {
	u32 license_index;
	u32 reserved;
};
#define LM_DOWNLOAD_LICENSE_REQ_MSG_V01_MAX_MSG_LEN 14

struct lm_download_license_resp_msg_v01 {
	u32 reserved;
	u32 next_lic_index;
	u8 data_valid;
	u32 data_len;
	u8 data[QMI_MAX_LICENSE_SIZE_V01];
};
#define LM_DOWNLOAD_LICENSE_RESP_MSG_V01_MAX_MSG_LEN 10259

struct license_info {
	char path[PATH_MAX];
	void *buffer;
	loff_t size;
	struct page *page;
	size_t order;
};

struct lm_svc_ctx {
	struct qmi_handle *lm_svc_hdl;
	enum lm_termination_mode_v01 termination_mode;
	bool license_loaded;
	int num_of_license;
	struct license_info **license_list;
	struct list_head clients_connected;
};

struct client_info {
	int sq_node;
	int sq_port;
	struct list_head node;
};

static const char * const license_path = "/license";
static const char * const license_extn = ".pem";

struct qmi_elem_info lm_get_termination_mode_req_msg_v01_ei[] = {
	{
		.data_type      = QMI_UNSIGNED_8_BYTE,
		.elem_len       = 1,
		.elem_size      = sizeof(u64),
		.array_type       = NO_ARRAY,
		.tlv_type       = 0x01,
		.offset         = offsetof(struct
					   lm_get_termination_mode_req_msg_v01,
					   timestamp),
	},
	{
		.data_type      = QMI_UNSIGNED_4_BYTE,
		.elem_len       = 1,
		.elem_size      = sizeof(u32),
		.array_type       = NO_ARRAY,
		.tlv_type       = 0x02,
		.offset         = offsetof(struct
					   lm_get_termination_mode_req_msg_v01,
					   id),
	},
	{
		.data_type      = QMI_UNSIGNED_1_BYTE,
		.elem_len       = QMI_MAX_CHIPINFO_ID_LEN_V01,
		.elem_size      = sizeof(char),
		.array_type       = STATIC_ARRAY,
		.tlv_type       = 0x03,
		.offset         = offsetof(struct
					   lm_get_termination_mode_req_msg_v01,
					   name),
	},
	{
		.data_type      = QMI_UNSIGNED_4_BYTE,
		.elem_len       = 1,
		.elem_size      = sizeof(u32),
		.array_type       = NO_ARRAY,
		.tlv_type       = 0x04,
		.offset         = offsetof(struct
					   lm_get_termination_mode_req_msg_v01,
					   serialnum),
	},
	{
		.data_type      = QMI_EOTI,
		.array_type       = NO_ARRAY,
		.tlv_type       = QMI_COMMON_TLV_TYPE,
	},
};
EXPORT_SYMBOL(lm_get_termination_mode_req_msg_v01_ei);

struct qmi_elem_info lm_get_termination_mode_resp_msg_v01_ei[] = {
	{
		.data_type      = QMI_SIGNED_4_BYTE_ENUM,
		.elem_len       = 1,
		.elem_size      = sizeof(enum lm_termination_mode_v01),
		.array_type       = NO_ARRAY,
		.tlv_type       = 0x01,
		.offset         = offsetof(struct
					   lm_get_termination_mode_resp_msg_v01,
					   termination_mode),
	},
	{
		.data_type      = QMI_UNSIGNED_4_BYTE,
		.elem_len       = 1,
		.elem_size      = sizeof(u32),
		.array_type       = NO_ARRAY,
		.tlv_type       = 0x02,
		.offset         = offsetof(struct
					   lm_get_termination_mode_resp_msg_v01,
					   reserved),
	},
	{
		.data_type      = QMI_EOTI,
		.array_type       = NO_ARRAY,
		.tlv_type       = QMI_COMMON_TLV_TYPE,
	},
};
EXPORT_SYMBOL(lm_get_termination_mode_resp_msg_v01_ei);

struct qmi_elem_info lm_download_license_req_msg_v01_ei[] = {
	{
		.data_type      = QMI_UNSIGNED_4_BYTE,
		.elem_len       = 1,
		.elem_size      = sizeof(u32),
		.array_type       = NO_ARRAY,
		.tlv_type       = 0x01,
		.offset         = offsetof(struct
					   lm_download_license_req_msg_v01,
					   license_index),
	},
	{
		.data_type      = QMI_UNSIGNED_4_BYTE,
		.elem_len       = 1,
		.elem_size      = sizeof(u32),
		.array_type       = NO_ARRAY,
		.tlv_type       = 0x02,
		.offset         = offsetof(struct
					   lm_download_license_req_msg_v01,
					   reserved),
	},
	{
		.data_type      = QMI_EOTI,
		.array_type       = NO_ARRAY,
		.tlv_type       = QMI_COMMON_TLV_TYPE,
	},
};
EXPORT_SYMBOL(lm_download_license_req_msg_v01_ei);

struct qmi_elem_info lm_download_license_resp_msg_v01_ei[] = {
	{
		.data_type      = QMI_UNSIGNED_4_BYTE,
		.elem_len       = 1,
		.elem_size      = sizeof(u32),
		.array_type       = NO_ARRAY,
		.tlv_type       = 0x01,
		.offset         = offsetof(struct
					   lm_download_license_resp_msg_v01,
					   reserved),
	},
	{
		.data_type      = QMI_UNSIGNED_4_BYTE,
		.elem_len       = 1,
		.elem_size      = sizeof(u32),
		.array_type       = NO_ARRAY,
		.tlv_type       = 0x02,
		.offset         = offsetof(struct
					   lm_download_license_resp_msg_v01,
					   next_lic_index),
	},
	{
		.data_type      = QMI_OPT_FLAG,
		.elem_len       = 1,
		.elem_size      = sizeof(u8),
		.array_type       = NO_ARRAY,
		.tlv_type       = 0x10,
		.offset         = offsetof(struct
					   lm_download_license_resp_msg_v01,
					   data_valid),
	},
	{
		.data_type      = QMI_DATA_LEN,
		.elem_len       = 1,
		.elem_size      = sizeof(u16),
		.array_type       = NO_ARRAY,
		.tlv_type       = 0x10,
		.offset         = offsetof(struct
					   lm_download_license_resp_msg_v01,
					   data_len),
	},
	{
		.data_type      = QMI_UNSIGNED_1_BYTE,
		.elem_len       = QMI_MAX_LICENSE_SIZE_V01,
		.elem_size      = sizeof(u8),
		.array_type       = VAR_LEN_ARRAY,
		.tlv_type       = 0x10,
		.offset         = offsetof(struct
					   lm_download_license_resp_msg_v01,
					   data),
	},
	{
		.data_type      = QMI_EOTI,
		.array_type       = NO_ARRAY,
		.tlv_type       = QMI_COMMON_TLV_TYPE,
	},
};
EXPORT_SYMBOL(lm_download_license_resp_msg_v01_ei);

#endif /* __LICENSE_MANAGER_H___ */
