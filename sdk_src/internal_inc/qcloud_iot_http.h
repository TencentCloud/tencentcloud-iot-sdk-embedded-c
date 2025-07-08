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

#ifndef QCLOUD_IOT_HTTP_H_
#define QCLOUD_IOT_HTTP_H_

#ifdef __cplusplus
extern "C" {
#endif

#define QCLOUD_HTTP_HEADER_FORMAT \
    "Accept: %s*/*\r\n"           \
    "X-TC-Algorithm: %s\r\n"      \
    "X-TC-Timestamp: %d\r\n"      \
    "X-TC-Nonce: %d\r\n"          \
    "X-TC-Signature: %s\r\n"

#define QCLOUD_SUPPORT_HMACSHA1 "hmacsha1"

#define QCLOUD_SUPPORT_RSASHA256 "rsa-sha256"

#define QCLOUD_SHA256_RESULT_LEN (32)
#define QCLOUD_SHA1_RESULT_LEN   (20)

#ifdef AUTH_MODE_CERT
#define QCLOUD_SHAX_RESULT_LEN (QCLOUD_SHA256_RESULT_LEN)
#else
#define QCLOUD_SHAX_RESULT_LEN (QCLOUD_SHA1_RESULT_LEN)
#endif

typedef struct qcloud_iot_http_header_post_sign {
    const char *host;
    const char *uri;
    char *      algorithm;
    uint32_t    timestamp;
    int         nonce;
    char *      request_body_buf;
    int         request_body_buf_len;
    char *      secretkey;
    void *      privatekey;
} QCLOUD_IOT_HTTP_HEADER_POST_SIGN;

char *qcloud_iot_http_header_create(char *request_body_buf, int request_body_buf_len, const char *host, const char *uri,
                                    char *accept_header, char *secretkey, void *privatekey, long timestamp);

void qcloud_iot_http_header_destory(char *http_header);

void *qcloud_iot_http_create_privatekey(char *privatekey_file);

void qcloud_iot_http_destory_privatekey(char *privatekey_file);

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_HTTP_H_ */
