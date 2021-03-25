/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2018-2020 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef IOT_RESOURCE_CLIENT_H_
#define IOT_RESOURCE_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "qcloud_iot_export_resource.h"

enum { MQTT_CHANNEL };

typedef struct qcloud_resource_upload_request {
    int   resource_size;
    char *resource_name;
    char *resource_md5sum;
} QCLOUD_RESOURCE_UPLOAD_REQUEST_S;

typedef void (*OnResourceMessageCallback)(void *pcontext, char *msg, uint32_t msgLen);

void *qcloud_resource_mqtt_init(const char *productId, const char *deviceName, void *channel,
                                OnResourceMessageCallback callback, void *context);

int qcloud_resource_mqtt_deinit(void *resource_mqtt);

int qcloud_resource_mqtt_yield(void *resource_mqtt);

int qcloud_resource_mqtt_upload_request(void *resource_mqtt, void *request_data);

int qcloud_resource_mqtt_download_get(void *resource_mqtt);

int qcloud_resource_mqtt_report_progress(void *resource_mqtt, QCLOUD_RESOURCE_REPORT_E state,
                                         QCLOUD_RESOURCE_RESULTCODE_E resultcode, int percent, char *resource_name,
                                         bool download);

#ifdef __cplusplus
}
#endif

#endif /* IOT_RESOURCE_CLIENT_H_ */
