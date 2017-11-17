/*
 * device.c
 *
 *  Created on: 2017年11月2日
 *      Author: shockcao
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "device.h"

#include <stdbool.h>
#include <string.h>

static DeviceInfo   sg_device_info;
static bool         sg_devinfo_initialized;

int iot_device_info_init() {
	if (sg_devinfo_initialized) {
		Log_e("device info has been initialized.");
		return 0;
	}

	memset(&sg_device_info, 0x0, sizeof(DeviceInfo));
	sg_devinfo_initialized = true;

	Log_i("device info init success!");
	return QCLOUD_ERR_SUCCESS;
}

int iot_device_info_set(const char *product_name, const char *device_name, const char *client_id) {
	Log_i("start to set device info!");

	memset(&sg_device_info, 0x0, sizeof(DeviceInfo));
	if ((MAX_SIZE_OF_PRODUCT_NAME - 1) < strlen(product_name))
	{
		Log_e("product name length:(%lu) exceeding limitation", strlen(product_name));
		return QCLOUD_ERR_FAILURE;
	}
	if ((MAX_SIZE_OF_DEVICE_NAME - 1) < strlen(device_name))
	{
		Log_e("device name length:(%lu) exceeding limitation", strlen(device_name));
		return QCLOUD_ERR_FAILURE;
	}
	if ((MAX_SIZE_OF_CLIENT_ID - 1) < strlen(client_id))
	{
		Log_e("client_id name length:(%lu) exceeding limitation", strlen(client_id));
		return QCLOUD_ERR_FAILURE;
	}

	strncpy(sg_device_info.product_name, product_name, MAX_SIZE_OF_PRODUCT_NAME);
	strncpy(sg_device_info.device_name, device_name, MAX_SIZE_OF_DEVICE_NAME);
	strncpy(sg_device_info.client_id, client_id, MAX_SIZE_OF_CLIENT_ID);

	Log_i("device info set successfully!");
	return QCLOUD_ERR_SUCCESS;
}

DeviceInfo* iot_device_info_get(void)
{
    return &sg_device_info;
}

#ifdef __cplusplus
}
#endif
