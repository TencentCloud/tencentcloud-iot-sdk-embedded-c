/*
 * device.h
 *
 *  Created on: 2017年11月2日
 *      Author: shockcao
 */

#ifndef DEVICE_H_
#define DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"

int iot_device_info_init(void);

int iot_device_info_set(const char *product_name, const char *device_name, const char *client_id);

DeviceInfo* iot_device_info_get(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_H_ */
