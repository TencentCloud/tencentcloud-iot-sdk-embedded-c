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

#ifndef IOT_RESOURCE_LIB_H_
#define IOT_RESOURCE_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

void qcloud_lib_md5_init(void *ctx_md5);

void qcloud_lib_md5_update(void *md5, const char *buf, size_t buf_len);

void qcloud_lib_md5_finish_tolowercasehex(void *md5, char *output_str);

void qcloud_lib_md5_deinit(void *md5);

int qcloud_lib_get_json_key_value(char *json_obj, char *json_key, char **value);

int qcloud_lib_base64encode_md5sum(char *md5sumhex, char *outbase64, int outbase64len);

#ifdef __cplusplus
}
#endif

#endif /* IOT_RESOURCE_LIB_H_ */
