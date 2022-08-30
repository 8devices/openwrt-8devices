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
#include "license_manager.h"

struct qmi_handle *lm_clnt_hdl;

static struct lm_svc_ctx *lm_svc;

static struct kobject *lm_kobj;

void license_table_creation(uint32_t lic_file_count);

static ssize_t file_count_store(struct kobject *kobj, struct kobj_attribute *attr,
			   const char *buf, size_t count)
{
	uint32_t lic_file_count, ret;
	bool *license_loaded;

	ret = kstrtouint(buf, 10, (unsigned int *)&lic_file_count);

	if (ret){
		pr_err("Error while parsing the input %d\n", ret);
		return ret;
	}

	if (!lm_svc) {
		pr_err("License Service is not running\n");
		return count;
	}

	license_loaded = &lm_svc->license_loaded;

	if (*license_loaded) {
		pr_err("License files loaded already\n");
		return count;
	}
	pr_info("Given license files count: %d\n",lic_file_count);

	if (lic_file_count > MAX_NUM_OF_LICENSES) {
		lic_file_count = MAX_NUM_OF_LICENSES;
		pr_err("Number of license input is greather than"
			" MAX_NUM_OF_LICENSES 10, so assiging it to max\n");
	}

	license_table_creation(lic_file_count);
	*license_loaded = true;
	pr_info("Loaded license files count: %d\n", lm_svc->num_of_license);

	return count;
}

volatile int file_count;
static struct kobj_attribute lm_svc_attr =
	__ATTR(file_count, 0660, NULL, file_count_store);

void license_table_creation(uint32_t lic_file_count)
{
	int i,ret;
	struct license_info **license_list = lm_svc->license_list;
	int *num_of_license_loaded = &lm_svc->num_of_license;

	for (i=0; i < lic_file_count; i++) {
		struct license_info *license;

		license = kzalloc(sizeof(struct license_info), GFP_KERNEL);

		if (!license) {
			ret = -ENOMEM;
			pr_err("Memory allocation for license info failed %d",
								ret);
			continue;
		}

		ret = snprintf(license->path, PATH_MAX, "%s/license_%d%s",
					license_path, i+1, license_extn);
		if (ret >= PATH_MAX) {
			ret = -ENAMETOOLONG;
			pr_err("License file path too long, failed with error code %d\n", ret);
			continue;
		}
		/*Allocate memory to hold the license file*/
		license->order = get_order(QMI_MAX_LICENSE_SIZE_V01);
		license->page = alloc_pages(GFP_KERNEL, license->order);
		if (!license->page) {
			ret = -ENOMEM;
			pr_err("Mem allocation for %s failed with error code %d\n",license->path, ret);
			continue;
		} else {
			/* get the mapped virtual address of the page */
			license->buffer = page_address(license->page);
		}
		memset(license->buffer, 0, QMI_MAX_LICENSE_SIZE_V01);
		ret = kernel_read_file_from_path(license->path,
			&license->buffer, &license->size,
			QMI_MAX_LICENSE_SIZE_V01, READING_FIRMWARE_PREALLOC_BUFFER);
		if (ret) {
			if (ret == -EFBIG){
				pr_err("Loading %s failed size bigger than MAX_LICENSE_SIZE %d\n",license->path, QMI_MAX_LICENSE_SIZE_V01);
			} else if(ret != -ENOENT) {
				pr_err("Loading %s failed with error %d\n", license->path, ret);
			} else {
				pr_err("Loading %s failed with no such file or directory\n",license->path);
			}
			free_pages((unsigned long)license->buffer, license->order);
			kfree(license);
		} else {
			pr_debug("License file %s of size %lld loaded into buffer 0x%p\n", license->path, license->size, license->buffer);

			license_list[*num_of_license_loaded] = license;
			*num_of_license_loaded = *num_of_license_loaded + 1;
		}
	}
}

void free_license_table(void)
{
	int i;
	struct license_info **license_list = lm_svc->license_list;
	int num_of_license = lm_svc->num_of_license;

	for ( i = 0; i < num_of_license; i++) {
		if(license_list[i]->buffer) {
			free_pages((unsigned long)license_list[i]->buffer,
					license_list[i]->order);
			kfree(license_list[i]);
		}
	}
	kfree(license_list);

}

static void qmi_handle_license_termination_mode_req(struct qmi_handle *handle,
			struct sockaddr_qrtr *sq,
			struct qmi_txn *txn,
			const void *decoded_msg)
{
	struct lm_get_termination_mode_req_msg_v01 *req;
	struct lm_get_termination_mode_resp_msg_v01 *resp;
	struct client_info *client;
	int ret;
	bool *license_loaded = &lm_svc->license_loaded;

	if(*license_loaded == false) {
		license_table_creation(MAX_NUM_OF_LICENSES);
		*license_loaded = true;
	}

	req = (struct lm_get_termination_mode_req_msg_v01 *)decoded_msg;

	resp = kzalloc(sizeof(*resp), GFP_KERNEL);
	if (!resp) {
		pr_err("%s: Memory allocation failed for resp buffer\n",
							__func__);
		return;
	}

	client = kzalloc(sizeof(*client), GFP_KERNEL);
	if (!client) {
		pr_err("%s: Memory allocation failed for client ctx\n",
							__func__);
		goto free_resp_mem;
	}

	pr_debug("License termination: Request rcvd: node_id: 0x%x, timestamp: "
			"0x%llx,chip_id: 0x%x,chip_name :%s,serialno:0x%x\n",
			sq->sq_node, req->timestamp, req->id,
			req->name, req->serialnum);

	client->sq_node = sq->sq_node;
	client->sq_port = sq->sq_port;
	list_add_tail(&client->node, &lm_svc->clients_connected);

	resp->termination_mode =  lm_svc->termination_mode;
	resp->reserved = 77;

	ret = qmi_send_response(handle, sq, txn,
			QMI_LM_GET_TERMINATION_MODE_RESP_V01,
			LM_GET_TERMINATION_MODE_RESP_MSG_V01_MAX_MSG_LEN,
			lm_get_termination_mode_resp_msg_v01_ei, resp);
	if (ret < 0)
		pr_err("%s: Sending license termination response failed"
					"with error_code:%d\n", __func__, ret);
	pr_debug("License termination: Response sent, license termination mode "
			"0x%x\n", resp->termination_mode);

free_resp_mem:
	kfree(resp);
}

static void qmi_handle_license_download_req(struct qmi_handle *handle,
			struct sockaddr_qrtr *sq,
			struct qmi_txn *txn,
			const void *decoded_msg)
{

	struct lm_download_license_req_msg_v01 *req;
	struct lm_download_license_resp_msg_v01 *resp;
	struct license_info **license_list = lm_svc->license_list;
	int num_of_license = lm_svc->num_of_license;
	int ret, req_lic_index;

	req = (struct lm_download_license_req_msg_v01 *)decoded_msg;

	resp = kzalloc(sizeof(*resp), GFP_KERNEL);
	if (!resp) {
		pr_err("%s: Memory allocation failed for resp buffer\n",
							__func__);
		return;
	}

	pr_debug("License download: Request rcvd, node_id: 0x%x, ReservedValue :0x%x,"
			"LicenseIndex %d\n", sq->sq_node, req->reserved,
			req->license_index);

	if (num_of_license <= 0) {
		resp->next_lic_index = 0;
		resp->data_valid = 0;
		resp->reserved = 0xdead;

		goto send_resp;
	}

	if (req->license_index < 0 || (req->license_index > num_of_license)) {
		pr_err("%s: unexpected license_index in request: %d\n",
					__func__, req->license_index);
		goto free_resp_buf;
	}
	req_lic_index = req->license_index;

	memcpy(resp->data, license_list[req_lic_index]->buffer,
				license_list[req_lic_index]->size);
	resp->data_len = license_list[req_lic_index]->size;
	resp->data_valid = 1;
	resp->reserved = 0xacac;

	if ((req_lic_index+1) < num_of_license)
		resp->next_lic_index = req_lic_index+1;
	else
		resp->next_lic_index = 0;

send_resp:
	ret = qmi_send_response(handle, sq, txn,
			QMI_LM_DOWNLOAD_RESP_V01,
			LM_DOWNLOAD_LICENSE_RESP_MSG_V01_MAX_MSG_LEN,
			lm_download_license_resp_msg_v01_ei, resp);
	if (ret < 0)
		pr_err("%s: Sending license termination response failed"
					"with error_code:%d\n",__func__,ret);

	pr_debug("License download: Response sent, license buffer "
			"size :0x%x, valid: %d next_lic_index %d\n",
			resp->data_len, resp->data_valid, resp->next_lic_index);
free_resp_buf:
	kfree(resp);

}

static void lm_qmi_svc_disconnect_cb(struct qmi_handle *qmi,
	unsigned int node, unsigned int port)
{
	struct client_info *itr, *tmp;

	if (!list_empty(&lm_svc->clients_connected)) {
		list_for_each_entry_safe(itr, tmp, &lm_svc->clients_connected,
								node) {
			if (itr->sq_node == node && itr->sq_port == port) {
				pr_info("Received LM QMI client disconnect "
					"from node:0x%x port:%d\n",
					node, port);
				list_del(&itr->node);
				kfree(itr);
			}
		}
	}
}

static struct qmi_ops lm_server_ops = {
	.del_client = lm_qmi_svc_disconnect_cb,
};
static struct qmi_msg_handler lm_req_handlers[] = {
	{
		.type = QMI_REQUEST,
		.msg_id = QMI_LM_GET_TERMINATION_MODE_REQ_V01,
		.ei = lm_get_termination_mode_req_msg_v01_ei,
		.decoded_size = LM_GET_TERMINATION_MODE_REQ_MSG_V01_MAX_MSG_LEN,
		.fn = qmi_handle_license_termination_mode_req,
	},
	{
		.type = QMI_REQUEST,
		.msg_id = QMI_LM_DOWNLOAD_REQ_V01,
		.ei = lm_download_license_req_msg_v01_ei,
		.decoded_size = LM_DOWNLOAD_LICENSE_REQ_MSG_V01_MAX_MSG_LEN,
		.fn = qmi_handle_license_download_req,
	},
	{}
};

static int license_manager_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	bool host_license_termination = false;
	bool device_license_termination = false;
	int ret = 0;
	struct lm_svc_ctx *svc;

	svc = kzalloc(sizeof(struct lm_svc_ctx), GFP_KERNEL);
	if (!svc)
		return -ENOMEM;

	device_license_termination = of_property_read_bool(node,
						"device-license-termination");
	pr_debug("device-license-termination : %s \n",
				device_license_termination ? "true" : "false");

	if (device_license_termination)
		goto skip_device_mode;

	host_license_termination = of_property_read_bool(node,
						"host-license-termination");
	pr_debug("host-license-termination : %s \n",
				host_license_termination ? "true" : "false");

	if (!host_license_termination) {
		pr_debug("License Termination mode not given in DT,"
			"Assuming Device license termination by default \n");
		device_license_termination = true;
	}

skip_device_mode:
	svc->termination_mode =  host_license_termination ?
			QMI_LICENSE_TERMINATION_AT_HOST_V01 :
				QMI_LICENSE_TERMINATION_AT_DEVICE_V01;

	svc->license_list = (struct license_info**)
				kzalloc(sizeof(struct license_info*) *
				MAX_NUM_OF_LICENSES, GFP_KERNEL);

	if (!svc->license_list) {
		ret = -ENOMEM;
		pr_err("%s:Mem allocation failed for license_list %d\n",
							__func__, ret);
		goto free_lm_svc;
	}

	svc->license_loaded = false;

	svc->lm_svc_hdl = kzalloc(sizeof(struct qmi_handle), GFP_KERNEL);
	if (!svc->lm_svc_hdl) {
		ret = -ENOMEM;
		pr_err("%s:Mem allocation failed for LM svc handle %d\n",
							__func__, ret);
		goto free_lm_license_list;
	}
	ret = qmi_handle_init(svc->lm_svc_hdl,
				QMI_LICENSE_MANAGER_SERVICE_MAX_MSG_LEN,
				&lm_server_ops,
				lm_req_handlers);
	if (ret < 0) {
		pr_err("%s:Error registering license manager svc %d\n",
							__func__, ret);
		goto free_lm_svc_handle;
	}
	ret = qmi_add_server(svc->lm_svc_hdl, LM_SERVICE_ID_V01,
					LM_SERVICE_VERS_V01,
					0);
	if (ret < 0) {
		pr_err("%s: failed to add license manager svc server :%d\n",
							__func__, ret);
		goto release_lm_svc_handle;
	}

	INIT_LIST_HEAD(&svc->clients_connected);

	lm_svc = svc;

	/* Creating a directory in /sys/kernel/ */
	lm_kobj = kobject_create_and_add("license_manager", kernel_kobj);
	if (lm_kobj) {
		if (sysfs_create_file(lm_kobj, &lm_svc_attr.attr)) {
			pr_err("Cannot create sysfs file for license manager\n");
			kobject_put(lm_kobj);
		}
	} else {
		pr_err("Unable to create license manager sysfs entry\n");
	}
	pr_info("License Manager driver registered\n");

	return 0;

release_lm_svc_handle:
	qmi_handle_release(svc->lm_svc_hdl);
free_lm_svc_handle:
	kfree(svc->lm_svc_hdl);
free_lm_license_list:
	kfree(svc->license_list);
free_lm_svc:
	kfree(svc);

	return ret;
}

static int license_manager_remove(struct platform_device *pdev)
{
	struct lm_svc_ctx *svc = lm_svc;
	struct client_info *itr, *tmp;

	qmi_handle_release(svc->lm_svc_hdl);

	if (!list_empty(&svc->clients_connected)) {
		list_for_each_entry_safe(itr, tmp, &svc->clients_connected,
								node) {
			list_del(&itr->node);
			kfree(itr);
		}
	}

	free_license_table();
	kfree(svc->lm_svc_hdl);
	kfree(svc);

	lm_svc = NULL;

	return 0;
}

static const struct of_device_id of_license_manager_match[] = {
	{.compatible = "qti,license-manager-service"},
	{  /* sentinel value */ },
};

static struct platform_driver license_manager_driver = {
	.probe		= license_manager_probe,
	.remove		= license_manager_remove,
	.driver		= {
		.name	= "license_manager",
		.of_match_table	= of_license_manager_match,
	},
};

static int __init license_manager_init(void)
{
	return platform_driver_register(&license_manager_driver);
}
module_init(license_manager_init);

static void __exit license_manager_exit(void)
{
	platform_driver_unregister(&license_manager_driver);
}
module_exit(license_manager_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("License manager driver");
