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

#ifndef IOT_RESOURCE_UPLOAD_H_
#define IOT_RESOURCE_UPLOAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void *qcloud_resource_upload_http_init(const char *url, char *md5sum, int upload_len);

int32_t qcloud_resource_upload_http_connect(void *handle);

int32_t qcloud_resource_upload_http_send_body(void *handle, char *buf, uint32_t buf_len, uint32_t timeout_s);

int32_t qcloud_resource_upload_http_recv(void *handle, char *buf, uint32_t bufLen, uint32_t timeout_s);

int qcloud_resource_upload_http_deinit(void *handle);

#ifdef __cplusplus
}
#endif

#endif /* IOT_RESOURCE_UPLOAD_H_ */
