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

#ifdef __cplusplus
extern "C" {
#endif

#include "resource_upload.h"
#include "utils_url_upload.h"
#include <string.h>

#include "qcloud_iot_ca.h"
#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "utils_httpc.h"
#include "resource_lib.h"

#define RESOURCE_HTTP_HEAD_CONTENT_LEN 256

void *qcloud_resource_upload_http_init(const char *url, char *md5sum, int upload_len)
{
    char base64md5[33] = {0};
    char header[RESOURCE_HTTP_HEAD_CONTENT_LEN];
    qcloud_lib_base64encode_md5sum(md5sum, base64md5, sizeof(base64md5));

    HAL_Snprintf(header, RESOURCE_HTTP_HEAD_CONTENT_LEN,
                 "Content-MD5:%s\r\n"
                 "Content-Type: application/x-www-form-urlencoded\r\n"
                 "Content-Length: %d\r\n",
                 base64md5, upload_len);

    return qcloud_url_upload_init(url, 0, header);
}

int32_t qcloud_resource_upload_http_connect(void *handle)
{
    return qcloud_url_upload_connect(handle, HTTP_PUT);
}

int32_t qcloud_resource_upload_http_send_body(void *handle, char *buf, uint32_t buf_len, uint32_t timeout_s)
{
    return qcloud_url_upload_body(handle, buf, buf_len, timeout_s);
}

int32_t qcloud_resource_upload_http_recv(void *handle, char *buf, uint32_t bufLen, uint32_t timeout_s)
{
    IOT_FUNC_ENTRY;
    int rc;
    rc = qcloud_url_upload_recv_response(handle, buf, bufLen, timeout_s * 1000);

    if (QCLOUD_RET_SUCCESS != rc) {
        if (rc == QCLOUD_ERR_HTTP_NOT_FOUND)
            IOT_FUNC_EXIT_RC(QCLOUD_RESOURCE_ERRCODE_FETCH_NOT_EXIST_E);

        if (rc == QCLOUD_ERR_HTTP_AUTH)
            IOT_FUNC_EXIT_RC(QCLOUD_RESOURCE_ERRCODE_FETCH_AUTH_FAIL_E);

        if (rc == QCLOUD_ERR_HTTP_TIMEOUT)
            IOT_FUNC_EXIT_RC(QCLOUD_RESOURCE_ERRCODE_FETCH_TIMEOUT_E);
    }

    IOT_FUNC_EXIT_RC(rc);
}

int qcloud_resource_upload_http_deinit(void *handle)
{
    return qcloud_url_upload_deinit(handle);
}

#ifdef __cplusplus
}
#endif
