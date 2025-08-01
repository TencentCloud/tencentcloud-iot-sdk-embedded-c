/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2018-2020 Tencent. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef QCLOUD_IOT_UTILS_HMAC_H_
#define QCLOUD_IOT_UTILS_HMAC_H_

#include <string.h>

void utils_hmac_md5(const char *msg, int msg_len, char *digest, const char *key, int key_len);

void utils_hmac_sha1(const char *msg, int msg_len, char *digest, const char *key, int key_len);

int utils_hmac_sha1_hex(const char *msg, int msg_len, char *digest, const char *key, int key_len);

#endif
