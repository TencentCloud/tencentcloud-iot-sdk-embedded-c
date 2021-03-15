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

#include <string.h>

#include "qcloud_iot_ca.h"
#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "utils_httpc.h"
#include "resource_lib.h"

#define RESOURCE_HTTP_HEAD_CONTENT_LEN 256

/* resource upload channel */

typedef struct {
    const char *   url;
    HTTPClient     http;      /* http client */
    HTTPClientData http_data; /* http client data */

} ResourceUploadHTTPStruct;

#ifdef RESOURCE_USE_HTTPS
static int is_begin_with(const char *str1, char *str2)
{
    if (str1 == NULL || str2 == NULL)
        return -1;
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    if ((len1 < len2) || (len1 == 0 || len2 == 0))
        return -1;
    char *p = str2;
    int   i = 0;
    while (*p != '\0') {
        if (*p != str1[i])
            return 0;
        p++;
        i++;
    }
    return 1;
}
#endif

static char sg_head_content[RESOURCE_HTTP_HEAD_CONTENT_LEN];
void *      qcloud_resource_upload_http_init(const char *url, char *md5sum)
{
    ResourceUploadHTTPStruct *resource_http;
    char                      base64md5[33] = {0};

    if (NULL == (resource_http = HAL_Malloc(sizeof(ResourceUploadHTTPStruct)))) {
        Log_e("allocate for resource_http failed");
        return NULL;
    }

    qcloud_lib_base64encode_md5sum(md5sum, base64md5, sizeof(base64md5));

    memset(resource_http, 0, sizeof(ResourceUploadHTTPStruct));
    memset(sg_head_content, 0, RESOURCE_HTTP_HEAD_CONTENT_LEN);

    HAL_Snprintf(sg_head_content, RESOURCE_HTTP_HEAD_CONTENT_LEN, "Content-MD5:%s\r\n", base64md5);
    resource_http->http.header = sg_head_content;

    Log_e("http head:%s", sg_head_content);

    resource_http->http_data.post_content_type = "application/x-www-form-urlencoded";

    // resource_http->http.header = sg_head_content;
    resource_http->url = url;

    return resource_http;
}

int32_t qcloud_resource_upload_http_connect(void *handle, char *upload_buf, int total_size)
{
    IOT_FUNC_ENTRY;

    ResourceUploadHTTPStruct *resource_http = (ResourceUploadHTTPStruct *)handle;
    resource_http->http_data.post_buf       = upload_buf;
    resource_http->http_data.post_buf_len   = total_size;

    int         port   = 80;
    const char *ca_crt = NULL;

#ifdef RESOURCE_USE_HTTPS
    if (is_begin_with(resource_http->url, "https")) {
        port   = 443;
        ca_crt = iot_https_ca_get();
    }
#endif

    int rc;

    if (resource_http->http.network_stack.handle == 0) {
        rc = qcloud_http_client_connect(&resource_http->http, resource_http->url, port, ca_crt);
        if (rc != QCLOUD_RET_SUCCESS)
            return rc;
    }

    rc = qcloud_http_client_send_header(&resource_http->http, resource_http->url, HTTP_PUT, &resource_http->http_data);
    if (rc != QCLOUD_RET_SUCCESS) {
        Log_e("http_client_send_request is error,rc = %d", rc);
        qcloud_http_client_close(&resource_http->http);
        return rc;
    }

    IOT_FUNC_EXIT_RC(rc);
}

int32_t qcloud_resource_upload_http_continue(void *handle, char *buf, uint32_t bufLen, uint32_t timeout_s)
{
    IOT_FUNC_ENTRY;

    ResourceUploadHTTPStruct *resource_http = (ResourceUploadHTTPStruct *)handle;
    int                       rc;

    resource_http->http_data.post_buf     = buf;
    resource_http->http_data.post_buf_len = bufLen;

    rc = qcloud_http_client_send_userdata(&resource_http->http, &resource_http->http_data, timeout_s);

    return rc;
}

int32_t qcloud_resource_upload_http_recv(void *handle, char *buf, uint32_t bufLen, uint32_t timeout_s)
{
    IOT_FUNC_ENTRY;

    int                       diff;
    ResourceUploadHTTPStruct *resource_http = (ResourceUploadHTTPStruct *)handle;
    int                       rc;

    resource_http->http_data.response_buf     = buf;
    resource_http->http_data.response_buf_len = bufLen;
    diff = resource_http->http_data.response_content_len - resource_http->http_data.retrieve_len;

    rc = qcloud_http_recv_data(&resource_http->http, timeout_s * 1000, &resource_http->http_data);

    if (resource_http->http.response_code >= 400) {
        Log_e("upload http recv response code:%d", resource_http->http.response_code);
        if (QCLOUD_RET_SUCCESS == rc) {
            Log_e("upload recv buf :%s", buf);
        }
        return QCLOUD_RESOURCE_ERRCODE_FETCH_TIMEOUT_E;
    }

    if (QCLOUD_RET_SUCCESS != rc) {
        if (rc == QCLOUD_ERR_HTTP_NOT_FOUND)
            IOT_FUNC_EXIT_RC(QCLOUD_RESOURCE_ERRCODE_FETCH_NOT_EXIST_E);

        if (rc == QCLOUD_ERR_HTTP_AUTH)
            IOT_FUNC_EXIT_RC(QCLOUD_RESOURCE_ERRCODE_FETCH_AUTH_FAIL_E);

        if (rc == QCLOUD_ERR_HTTP_TIMEOUT)
            IOT_FUNC_EXIT_RC(QCLOUD_RESOURCE_ERRCODE_FETCH_TIMEOUT_E);

        IOT_FUNC_EXIT_RC(rc);
    }

    IOT_FUNC_EXIT_RC(resource_http->http_data.response_content_len - resource_http->http_data.retrieve_len - diff);
}

int qcloud_resource_upload_http_deinit(void *handle)
{
    ResourceUploadHTTPStruct *resource_http = (ResourceUploadHTTPStruct *)handle;
    if (NULL == resource_http)
        IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);

    if (resource_http->http.network_stack.is_connected(&resource_http->http.network_stack))
        resource_http->http.network_stack.disconnect(&resource_http->http.network_stack);

    HAL_Free(handle);
    IOT_FUNC_EXIT_RC(QCLOUD_RET_SUCCESS);
}

#ifdef __cplusplus
}
#endif
