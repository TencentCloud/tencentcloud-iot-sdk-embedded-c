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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qcloud_iot_export.h"
#include "utils_param_check.h"
#include "utils_timer.h"
#include "utils_md5.h"

#include "resource_client.h"
#include "ota_fetch.h"
#include "resource_upload.h"
#include "resource_lib.h"

#define QCLOUD_IOT_RESOURCE_NAME_LEN 64
#define QCLOUD_IOT_RESOURCE_URL_LEN  1024

typedef struct {
    const char *product_id;  /* point to product id */
    const char *device_name; /* point to device name */
    short       current_signal_type;
    void *      resoure_mqtt; /* channel handle of signal exchanged with OTA server */

    // download
    void *                       ch_fetch;                                    /* channel handle of download */
    QCLOUD_RESOURCE_STATE_CODE_E state;                                       /* OTA state */
    uint32_t                     size_last_fetched;                           /* size of last downloaded */
    uint32_t                     size_fetched;                                /* size of already downloaded */
    uint32_t                     resource_size;                               /* size of file */
    char                         resource_url[QCLOUD_IOT_RESOURCE_URL_LEN];   /* point to URL */
    char                         resource_name[QCLOUD_IOT_RESOURCE_NAME_LEN]; /* point to string */
    char                         md5sum[33];                                  /* MD5 string */
    iot_md5_context              md5;                                         /* MD5 handle */
    int                          err;                                         /* last error code */
    Timer                        report_timer;
    uint32_t                     result_code;

    // upload
    void *                       upload_handle;                                      /* channel handle of download */
    QCLOUD_RESOURCE_STATE_CODE_E upload_state;                                       /* OTA state */
    uint32_t                     size_last_uploaded;                                 /* size of last downloaded */
    uint32_t                     size_uploaded;                                      /* size of already downloaded */
    uint32_t                     upload_resource_size;                               /* size of file */
    char                         upload_resource_url[QCLOUD_IOT_RESOURCE_URL_LEN];   /* point to URL */
    char                         upload_resource_name[QCLOUD_IOT_RESOURCE_NAME_LEN]; /* point to string */
    char                         upload_md5sum[33];                                  /* MD5 string */
    Timer                        upload_report_timer;
    iot_md5_context              upload_md5; /* MD5 handle */
    int                          upload_err; /* last error code */
    uint32_t                     upload_result_code;
} QCLOUD_RESOURCE_CONTEXT_T;

/* check ota progress */
/* return: true, valid progress state; false, invalid progress state. */
static int _qcloud_iot_resource_check_progress(IOT_OTA_Progress_Code progress)
{
    return ((progress >= 0) && (progress <= 100));
}

static void _qcloud_iot_resource_cb_proc_download(char *msg, QCLOUD_RESOURCE_CONTEXT_T *resource_handle)
{
    char *json_value = NULL;
    if (resource_handle->state >= QCLOUD_RESOURCE_STATE_START_E) {
        Log_i("In downloading or downloaded state");
        return;
    }

    if (qcloud_lib_get_json_key_value(msg, "size", &json_value) != QCLOUD_RET_SUCCESS) {
        Log_e("Get rresource_size failed!");
        return;
    }
    resource_handle->resource_size = atoi(json_value);
    HAL_Free(json_value);
    json_value = NULL;

    if (qcloud_lib_get_json_key_value(msg, "name", &json_value) != QCLOUD_RET_SUCCESS) {
        Log_e("Get resource_name failed!");
        return;
    }

    strncpy(resource_handle->resource_name, json_value, sizeof(resource_handle->resource_name));
    HAL_Free(json_value);
    json_value = NULL;

    if (qcloud_lib_get_json_key_value(msg, "md5sum", &json_value) != QCLOUD_RET_SUCCESS) {
        Log_e("Get md5sum failed!");
        return;
    }
    strncpy(resource_handle->md5sum, json_value, sizeof(resource_handle->md5sum));
    HAL_Free(json_value);
    json_value = NULL;

    if (qcloud_lib_get_json_key_value(msg, "url", &json_value) != QCLOUD_RET_SUCCESS) {
        Log_e("Get url failed!");
        return;
    }

    strncpy(resource_handle->resource_url, json_value, sizeof(resource_handle->resource_url));
    HAL_Free(json_value);
    json_value = NULL;

    resource_handle->state = QCLOUD_RESOURCE_STATE_START_E;
}

static void _qcloud_iot_resource_cb_proc_uploadtsk(char *msg, QCLOUD_RESOURCE_CONTEXT_T *resource_handle)
{
    char *json_value = NULL;
    if (resource_handle->upload_state >= QCLOUD_RESOURCE_STATE_START_E) {
        Log_i("In downloading or downloaded state");
        return;
    }

    if (qcloud_lib_get_json_key_value(msg, "size", &json_value) != QCLOUD_RET_SUCCESS) {
        Log_e("Get rresource_size failed!");
        return;
    }
    resource_handle->upload_resource_size = atoi(json_value);
    HAL_Free(json_value);
    json_value = NULL;

    if (qcloud_lib_get_json_key_value(msg, "name", &json_value) != QCLOUD_RET_SUCCESS) {
        Log_e("Get resource_name failed!");
        return;
    }
    strncpy(resource_handle->upload_resource_name, json_value, sizeof(resource_handle->upload_resource_name));
    HAL_Free(json_value);
    json_value = NULL;

    if (qcloud_lib_get_json_key_value(msg, "md5sum", &json_value) != QCLOUD_RET_SUCCESS) {
        Log_e("Get md5sum failed!");
        return;
    }
    strncpy(resource_handle->upload_md5sum, json_value, sizeof(resource_handle->md5sum));
    HAL_Free(json_value);
    json_value = NULL;

    if (qcloud_lib_get_json_key_value(msg, "url", &json_value) != QCLOUD_RET_SUCCESS) {
        Log_e("Get url failed!");
        return;
    }

    strncpy(resource_handle->upload_resource_url, json_value, sizeof(resource_handle->upload_resource_url));
    HAL_Free(json_value);
    json_value = NULL;

    resource_handle->upload_state = QCLOUD_RESOURCE_STATE_START_E;
}

static void _qcloud_iot_resource_cb_proc_uploadresult(char *msg, QCLOUD_RESOURCE_CONTEXT_T *resource_handle)
{
    char *json_value    = NULL;
    char *json_report   = NULL;
    char *json_progress = NULL;

    if (qcloud_lib_get_json_key_value(msg, "report", &json_report) != QCLOUD_RET_SUCCESS) {
        Log_e("Get report failed!");
        goto End;
    }

    if (qcloud_lib_get_json_key_value(json_report, "progress", &json_progress) != QCLOUD_RET_SUCCESS) {
        Log_e("Get progress failed!");
        goto End;
    }

    if (qcloud_lib_get_json_key_value(json_progress, "result_code", &json_value) != QCLOUD_RET_SUCCESS) {
        Log_e("Get progress failed!");
        goto End;
    }
    resource_handle->upload_result_code = atoi(json_value);
    HAL_Free(json_value);
    json_value = NULL;

End:
    HAL_Free(json_report);
    HAL_Free(json_progress);
}

/* callback when resource topic msg is received */
static void _qcloud_iot_resource_callback(void *pcontext, char *msg, uint32_t msg_len)
{
    char *json_type  = NULL;
    char *json_value = NULL;

    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)pcontext;

    if (msg == NULL || msg_len == 0) {
        Log_e("OTA response message is NULL");
        return;
    }

    if (qcloud_lib_get_json_key_value(msg, "type", &json_type) != QCLOUD_RET_SUCCESS) {
        Log_e("Get resource type failed!");
        goto End;
    }

    // download
    if ((0 == strcmp(json_type, "download")) || (0 == strcmp(json_type, "get_download_task_rsp"))) {
        _qcloud_iot_resource_cb_proc_download(msg, resource_handle);
    } else if (0 == strcmp(json_type, "create_upload_task_rsp")) {
        _qcloud_iot_resource_cb_proc_uploadtsk(msg, resource_handle);
    } else if (0 == strcmp(json_type, "upload_result")) {
        _qcloud_iot_resource_cb_proc_uploadresult(msg, resource_handle);
    } else if (0 == strcmp(json_type, "report_upload_progress_rsp")) {
        if (qcloud_lib_get_json_key_value(msg, "result_code", &json_value) != QCLOUD_RET_SUCCESS) {
            Log_e("Get upload progress failed! %s");
            goto End;
        }
        resource_handle->upload_result_code = atoi(json_value);
        HAL_Free(json_value);
        json_value = NULL;

    } else if (0 == strcmp(json_type, "report_download_progress_rsp")) {
        if (qcloud_lib_get_json_key_value(msg, "result_code", &json_value) != QCLOUD_RET_SUCCESS) {
            Log_e("Get download progress failed! %s");
            goto End;
        }
        resource_handle->result_code = atoi(json_value);
        HAL_Free(json_value);
        json_value = NULL;

    } else {
        Log_e("unsupport type %s", STRING_PTR_PRINT_SANITY_CHECK(json_type));
    }

End:
    HAL_Free(json_type);
}

static void _qcloud_iot_resource_download_reset(void *handle)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    Log_i("reset resource download state!");

    resource_handle->state = QCLOUD_RESOURCE_STATE_INITED_E;
    resource_handle->err   = 0;
    memset(resource_handle->resource_url, 0, sizeof(resource_handle->resource_url));
    memset(resource_handle->resource_name, 0, sizeof(resource_handle->resource_name));
    resource_handle->result_code   = 0;
    resource_handle->resource_size = -1;
}

static void _qcloud_iot_resource_upload_reset(void *handle)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    Log_i("reset resource upload state!");

    resource_handle->upload_state = QCLOUD_RESOURCE_STATE_INITED_E;
    resource_handle->upload_err   = 0;
    memset(resource_handle->upload_resource_url, 0, sizeof(resource_handle->upload_resource_url));
    memset(resource_handle->upload_resource_name, 0, sizeof(resource_handle->upload_resource_name));
    resource_handle->upload_result_code   = 0;
    resource_handle->upload_resource_size = -1;
}

static int _qcloud_iot_resource_report_download_progress(void *handle, IOT_OTA_Progress_Code progress,
                                                         char *resource_name)
{
    int                        ret             = QCLOUD_ERR_FAILURE;
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    if (NULL == handle) {
        Log_e("handle is NULL");
        return QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
    }

    if (QCLOUD_RESOURCE_STATE_UNINITED_E == resource_handle->state) {
        Log_e("handle is uninitialized");
        resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
        return QCLOUD_ERR_FAILURE;
    }

    if (!_qcloud_iot_resource_check_progress(progress)) {
        Log_e("progress is a invalid parameter: %d", progress);
        resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
        return QCLOUD_ERR_FAILURE;
    }

    ret = qcloud_resource_mqtt_report_progress(resource_handle->resoure_mqtt, QCLOUD_RESOURCE_REPORT_DOWNLOADING_E, 0,
                                               progress, resource_name, true);

    if (QCLOUD_RET_SUCCESS > ret) {
        Log_e("Report progress failed");
        resource_handle->err = ret;
        goto do_exit;
    }

    ret = QCLOUD_RET_SUCCESS;

do_exit:
    return ret;
}

static int _qcloud_iot_resource_report_download_result(void *handle, char *resource_name,
                                                       QCLOUD_RESOURCE_RESULTCODE_E result_code)
{
    POINTER_SANITY_CHECK(handle, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);
    POINTER_SANITY_CHECK(resource_name, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);

    int                        ret, len;
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;
    QCLOUD_RESOURCE_REPORT_E   report;

    if (QCLOUD_RESOURCE_STATE_UNINITED_E == resource_handle->state) {
        Log_e("handle is uninitialized");
        resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
        return QCLOUD_ERR_FAILURE;
    }

    len = strlen(resource_name);
    if ((len < 1) || (len > 64)) {
        Log_e("version string is invalid: must be [1, 64] chars");
        resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
        ret                  = QCLOUD_ERR_FAILURE;
        goto do_exit;
    }

    if (QCLOUD_RESOURCE_RESULTCODE_SUCCESS_E != result_code) {
        report = QCLOUD_RESOURCE_REPORT_FAILED_E;
    } else {
        report = QCLOUD_RESOURCE_REPORT_SUCCESS_E;
    }

    HAL_SleepMs(1000);
    resource_handle->result_code = -1;
    ret = qcloud_resource_mqtt_report_progress(resource_handle->resoure_mqtt, report, result_code, 0, resource_name,
                                               true);
    if (0 > ret) {
        Log_e("report download result failed");
        resource_handle->err = ret;
        ret                  = QCLOUD_ERR_FAILURE;
        goto do_exit;
    }

do_exit:
    _qcloud_iot_resource_download_reset(resource_handle);

    return ret;
}

static int _qcloud_iot_resource_report_upload_progress(void *handle, IOT_OTA_Progress_Code progress,
                                                       char *resource_name)
{
    int                        ret             = QCLOUD_ERR_FAILURE;
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    if (NULL == handle) {
        Log_e("handle is NULL");
        return QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
    }

    if (QCLOUD_RESOURCE_STATE_UNINITED_E == resource_handle->upload_state) {
        Log_e("handle is uninitialized");
        resource_handle->upload_err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
        return QCLOUD_ERR_FAILURE;
    }

    if (!_qcloud_iot_resource_check_progress(progress)) {
        Log_e("progress is a invalid parameter: %d", progress);
        resource_handle->upload_err = QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
        return QCLOUD_ERR_FAILURE;
    }

    ret = qcloud_resource_mqtt_report_progress(resource_handle->resoure_mqtt, QCLOUD_RESOURCE_REPORT_UPLOADING_E, 0,
                                               progress, resource_name, false);

    if (QCLOUD_RET_SUCCESS > ret) {
        Log_e("Report progress failed");
        resource_handle->upload_err = ret;
        goto do_exit;
    }

    ret = QCLOUD_RET_SUCCESS;

do_exit:
    return ret;
}

static int _qcloud_iot_resource_report_upload_result(void *handle, char *resource_name,
                                                     QCLOUD_RESOURCE_RESULTCODE_E result_code)
{
    POINTER_SANITY_CHECK(handle, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);
    POINTER_SANITY_CHECK(resource_name, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);

    int                        ret, len;
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;
    QCLOUD_RESOURCE_REPORT_E   report;
    Timer                      wait_rsp_result_timer;

    if (QCLOUD_RESOURCE_STATE_UNINITED_E == resource_handle->upload_state) {
        Log_e("handle is uninitialized");
        resource_handle->upload_err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
        return QCLOUD_ERR_FAILURE;
    }

    len = strlen(resource_name);
    if ((len < 1) || (len > 64)) {
        Log_e("resource name string is invalid: must be [1, 64] chars");
        resource_handle->upload_err = QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
        ret                         = QCLOUD_ERR_FAILURE;
        goto do_exit;
    }

    if (QCLOUD_RESOURCE_RESULTCODE_SUCCESS_E != result_code) {
        report = QCLOUD_RESOURCE_REPORT_FAILED_E;
    } else {
        report = QCLOUD_RESOURCE_REPORT_SUCCESS_E;
    }

    resource_handle->upload_result_code = -1;
    ret = qcloud_resource_mqtt_report_progress(resource_handle->resoure_mqtt, report, result_code, 0, resource_name,
                                               false);
    if (0 > ret) {
        Log_e("report download result failed");
        resource_handle->upload_err = ret;
        ret                         = QCLOUD_ERR_FAILURE;
        goto do_exit;
    }

    InitTimer(&wait_rsp_result_timer);
    countdown(&wait_rsp_result_timer, 5);
    ret = QCLOUD_ERR_FAILURE;
    while (false == expired(&wait_rsp_result_timer)) {
        if (QCLOUD_RET_SUCCESS != qcloud_resource_mqtt_yield(resource_handle->resoure_mqtt)) {
            break;
        }
        if (resource_handle->upload_result_code != -1) {
            ret = resource_handle->upload_result_code;
            break;
        }
    }

do_exit:
    _qcloud_iot_resource_upload_reset(resource_handle);
    return ret;
}

/* Init resource handle */
static void *_qcloud_iot_resource_init(const char *product_id, const char *device_name, void *mqtt_client)
{
    POINTER_SANITY_CHECK(product_id, NULL);
    POINTER_SANITY_CHECK(device_name, NULL);
    POINTER_SANITY_CHECK(mqtt_client, NULL);

    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = NULL;

    if (NULL == (resource_handle = HAL_Malloc(sizeof(QCLOUD_RESOURCE_CONTEXT_T)))) {
        Log_e("allocate failed");
        return NULL;
    }
    memset(resource_handle, 0, sizeof(QCLOUD_RESOURCE_CONTEXT_T));
    resource_handle->state = QCLOUD_RESOURCE_STATE_UNINITED_E;

    resource_handle->resoure_mqtt =
        qcloud_resource_mqtt_init(product_id, device_name, mqtt_client, _qcloud_iot_resource_callback, resource_handle);

    if (NULL == resource_handle->resoure_mqtt) {
        Log_e("initialize signal channel failed");
        goto do_exit;
    }

    qcloud_lib_md5_init(&(resource_handle->md5));

    qcloud_lib_md5_init(&(resource_handle->upload_md5));

    resource_handle->product_id   = product_id;
    resource_handle->device_name  = device_name;
    resource_handle->state        = QCLOUD_RESOURCE_STATE_INITED_E;
    resource_handle->upload_state = QCLOUD_RESOURCE_STATE_INITED_E;

    resource_handle->current_signal_type = MQTT_CHANNEL;

    return resource_handle;

do_exit:

    if (NULL != resource_handle->resoure_mqtt) {
        qcloud_resource_mqtt_deinit(resource_handle->resoure_mqtt);
    }

    if (NULL != resource_handle) {
        HAL_Free(resource_handle);
    }

    return NULL;
}

int QCLOUD_IOT_RESOURCE_ReportUploadSuccess(void *handle, char *resource_name)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;
    int                        ret;

    if (NULL == resource_name) {
        resource_name = resource_handle->upload_resource_name;
    }

    ret =
        _qcloud_iot_resource_report_upload_result(resource_handle, resource_name, QCLOUD_RESOURCE_RESULTCODE_SUCCESS_E);

    return ret;
}

int QCLOUD_IOT_RESOURCE_ReportUploadFail(void *handle, char *resource_name)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;
    int                        ret;

    if (NULL == resource_name) {
        resource_name = resource_handle->upload_resource_name;
    }

    ret =
        _qcloud_iot_resource_report_upload_result(resource_handle, resource_name, QCLOUD_RESOURCE_RESULTCODE_TIMEOUT_E);

    return ret;
}

void *QCLOUD_IOT_RESOURCE_Init(const char *product_id, const char *device_name, void *mqtt_client)
{
    return _qcloud_iot_resource_init(product_id, device_name, mqtt_client);
}

int QCLOUD_IOT_RESOURCE_Upload_Request(void *handle, char *resource_name, int resource_size, char *md5sum)
{
    QCLOUD_RESOURCE_UPLOAD_REQUEST_S request_data;
    int                              ret;
    QCLOUD_RESOURCE_CONTEXT_T *      resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    // _qcloud_iot_resource_upload_reset(handle);

    request_data.resource_name   = resource_name;
    request_data.resource_size   = resource_size;
    request_data.resource_md5sum = md5sum;

    ret = qcloud_resource_mqtt_upload_request(resource_handle->resoure_mqtt, &request_data);

    return ret;
}

void QCLOUD_IOT_RESOURCE_UploadMd5Update(void *handle, char *buf, int buf_len)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    qcloud_lib_md5_update(&(resource_handle->upload_md5), buf, buf_len);
}

void QCLOUD_IOT_RESOURCE_Upload_Md5_Finish(void *handle, char *md5_str)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;
    qcloud_lib_md5_finish_tolowercasehex(&(resource_handle->upload_md5), md5_str);

    QCLOUD_IOT_RESOURCE_UploadResetClientMD5(handle);
}

int QCLOUD_IOT_RESOURCE_IsStartUpload(void *handle)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    if (NULL == handle) {
        Log_e("handle is NULL");
        return 0;
    }

    if (QCLOUD_RESOURCE_STATE_UNINITED_E == resource_handle->upload_state) {
        Log_e("handle is uninitialized");
        resource_handle->upload_err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
        return 0;
    }

    resource_handle->upload_result_code = 0;

    return (QCLOUD_RESOURCE_STATE_START_E == resource_handle->upload_state);
}

/*support continuous transmission of breakpoints*/
int QCLOUD_IOT_RESOURCE_StartUpload(void *handle, char *buf)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;
    int                        ret;

    resource_handle->size_last_uploaded = 0;
    resource_handle->size_uploaded      = 0;

    Log_d("to upload resource total size: %u", resource_handle->upload_resource_size);

    // reset md5 for new download
    ret = QCLOUD_IOT_RESOURCE_UploadResetClientMD5(resource_handle);
    if (ret != QCLOUD_RET_SUCCESS) {
        Log_e("initialize md5 failed");
        return QCLOUD_ERR_FAILURE;
    }

    // reinit upload http
    qcloud_resource_upload_http_deinit(resource_handle->upload_handle);
    resource_handle->upload_handle = qcloud_resource_upload_http_init(
        resource_handle->upload_resource_url, resource_handle->upload_md5sum, resource_handle->upload_resource_size);
    if (NULL == resource_handle->upload_handle) {
        Log_e("Initialize fetch module failed");
        return QCLOUD_ERR_FAILURE;
    }

    ret = qcloud_resource_upload_http_connect(resource_handle->upload_handle);
    if (QCLOUD_RET_SUCCESS != ret) {
        Log_e("Connect fetch module failed");
        resource_handle->upload_state = QCLOUD_RESOURCE_STATE_DISCONNECTED_E;
    }

    return ret;
}

int QCLOUD_IOT_RESOURCE_UploadRecvResult(void *handle, void *buf, int buf_len)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    int ret = qcloud_resource_upload_http_recv(resource_handle->upload_handle, buf, buf_len, 2);
    if (ret < 0) {
        resource_handle->upload_state = QCLOUD_RESOURCE_STATE_END_E;
        resource_handle->upload_err   = QCLOUD_RESOURCE_ERRCODE_FETCH_FAILED_E;

        if (ret == QCLOUD_RESOURCE_ERRCODE_FETCH_AUTH_FAIL_E) {
            _qcloud_iot_resource_report_upload_result(resource_handle, resource_handle->upload_resource_name,
                                                      QCLOUD_RESOURCE_RESULTCODE_SIGN_INVALID_E);
            resource_handle->upload_err = ret;
        } else if (ret == QCLOUD_RESOURCE_ERRCODE_FETCH_NOT_EXIST_E) {
            _qcloud_iot_resource_report_upload_result(resource_handle, resource_handle->upload_resource_name,
                                                      QCLOUD_RESOURCE_RESULTCODE_FILE_NOTEXIST_E);
            resource_handle->upload_err = ret;
        } else if (ret == QCLOUD_RESOURCE_ERRCODE_FETCH_TIMEOUT_E) {
            _qcloud_iot_resource_report_upload_result(resource_handle, resource_handle->upload_resource_name,
                                                      QCLOUD_RESOURCE_RESULTCODE_TIMEOUT_E);
            resource_handle->upload_err = ret;
        }
    }

    return ret;
}

int QCLOUD_IOT_RESOURCE_UploadYield(void *handle, char *buf, uint32_t buf_len, uint32_t timeout_s)
{
    int                        ret;
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    POINTER_SANITY_CHECK(handle, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);
    POINTER_SANITY_CHECK(buf, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);
    NUMBERIC_SANITY_CHECK(buf_len, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);

    if ((QCLOUD_RESOURCE_STATE_START_E != resource_handle->upload_state) ||
        (resource_handle->upload_result_code != 0)) {
        Log_e("upload result code :%d", resource_handle->upload_result_code);
        resource_handle->upload_err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
        return QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
    }

    ret = qcloud_resource_upload_http_send_body(resource_handle->upload_handle, buf, buf_len, timeout_s);
    if (ret < 0) {
        resource_handle->upload_state = QCLOUD_RESOURCE_STATE_END_E;
        resource_handle->upload_err   = QCLOUD_RESOURCE_ERRCODE_FETCH_FAILED_E;
        if ((ret == QCLOUD_ERR_HTTP_CLOSED) || (ret == QCLOUD_ERR_HTTP_CONN)) {
            _qcloud_iot_resource_report_upload_result(resource_handle, resource_handle->upload_resource_name,
                                                      QCLOUD_RESOURCE_RESULTCODE_SIGN_INVALID_E);
            resource_handle->upload_err = ret;
            buf_len                     = QCLOUD_IOT_RESOURCE_UploadRecvResult(resource_handle, buf, buf_len);
        }

        return ret;
    } else if (0 == resource_handle->size_uploaded) {
        /* force report status in the first */
        _qcloud_iot_resource_report_upload_progress(resource_handle, IOT_OTAP_FETCH_PERCENTAGE_MIN,
                                                    resource_handle->upload_resource_name);

        InitTimer(&resource_handle->upload_report_timer);
        countdown(&resource_handle->upload_report_timer, 1);
    }

    resource_handle->size_last_uploaded = buf_len;
    resource_handle->size_uploaded += buf_len;

    /* report percent every second. */
    uint32_t percent = (resource_handle->size_uploaded * 100) / resource_handle->upload_resource_size;
    if (percent == 100) {
        _qcloud_iot_resource_report_upload_progress(resource_handle, percent, resource_handle->upload_resource_name);
    } else if (resource_handle->size_last_uploaded > 0 && expired(&resource_handle->upload_report_timer)) {
        _qcloud_iot_resource_report_upload_progress(resource_handle, percent, resource_handle->upload_resource_name);
        countdown(&resource_handle->upload_report_timer, 1);
    }

    qcloud_lib_md5_update(&(resource_handle->upload_md5), buf, buf_len);

    if (resource_handle->size_uploaded >= resource_handle->upload_resource_size) {
        resource_handle->upload_state = QCLOUD_RESOURCE_STATE_END_E;

        /* send data complete recv server reply */
        buf_len = QCLOUD_IOT_RESOURCE_UploadRecvResult(resource_handle, buf, buf_len);
    }

    return buf_len;
}

/* check whether fetch over */
int QCLOUD_IOT_RESOURCE_IsUploadFinish(void *handle)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    if (NULL == resource_handle) {
        Log_e("handle is NULL");
        return 0;
    }

    if (QCLOUD_RESOURCE_STATE_UNINITED_E == resource_handle->upload_state) {
        Log_e("handle is uninitialized");
        resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
        return 0;
    }

    return (QCLOUD_RESOURCE_STATE_END_E == resource_handle->upload_state);
}

/* Destroy OTA handle and resource */
static int _qcloud_iot_resource_destroy(void *handle)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    if (NULL == resource_handle) {
        Log_e("handle is NULL");
        return QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
    }

    if (QCLOUD_RESOURCE_STATE_UNINITED_E == resource_handle->state) {
        Log_e("handle is uninitialized");
        resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
        return QCLOUD_ERR_FAILURE;
    }

    if (QCLOUD_RESOURCE_STATE_UNINITED_E == resource_handle->upload_state) {
        Log_e("handle is uninitialized");
        resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
        return QCLOUD_ERR_FAILURE;
    }

    qcloud_resource_mqtt_deinit(resource_handle->resoure_mqtt);
    qcloud_ofc_deinit(resource_handle->ch_fetch);
    qcloud_resource_upload_http_deinit(resource_handle->upload_handle);
    qcloud_lib_md5_deinit(&(resource_handle->md5));
    qcloud_lib_md5_deinit(&(resource_handle->upload_md5));

    memset(resource_handle->resource_url, 0, sizeof(resource_handle->resource_url));
    memset(resource_handle->resource_name, 0, sizeof(resource_handle->resource_name));
    memset(resource_handle->upload_resource_url, 0, sizeof(resource_handle->upload_resource_url));
    memset(resource_handle->upload_resource_name, 0, sizeof(resource_handle->upload_resource_name));

    HAL_Free(resource_handle);

    return QCLOUD_RET_SUCCESS;
}

int QCLOUD_IOT_RESOURCE_DeInit(void *handle)
{
    return _qcloud_iot_resource_destroy(handle);
}

int QCLOUD_IOT_RESOURCE_GetDownloadTask(void *handle)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;
    return qcloud_resource_mqtt_download_get(resource_handle->resoure_mqtt);
}

/*support continuous transmission of breakpoints*/
void QCLOUD_IOT_RESOURCE_UpdateDownloadClientMd5(void *handle, char *buff, uint32_t size)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    qcloud_lib_md5_update(&(resource_handle->md5), buff, size);
}

/*support continuous transmission of breakpoints*/
int QCLOUD_IOT_RESOURCE_DownloadResetClientMD5(void *handle)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    qcloud_lib_md5_deinit(&(resource_handle->md5));
    qcloud_lib_md5_init(&(resource_handle->md5));

    return QCLOUD_RET_SUCCESS;
}

/*support continuous transmission of breakpoints*/
int QCLOUD_IOT_RESOURCE_UploadResetClientMD5(void *handle)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    qcloud_lib_md5_deinit(&(resource_handle->upload_md5));
    qcloud_lib_md5_init(&(resource_handle->upload_md5));

    return QCLOUD_RET_SUCCESS;
}

int QCLOUD_IOT_RESOURCE_ReportDownloadSuccess(void *handle, char *resource_name)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;
    int                        ret;

    if (NULL == resource_name) {
        resource_name = resource_handle->resource_name;
    }

    ret = _qcloud_iot_resource_report_download_result(resource_handle, resource_name,
                                                      QCLOUD_RESOURCE_RESULTCODE_SUCCESS_E);

    return ret;
}

int QCLOUD_IOT_RESOURCE_ReportDownloadFail(void *handle, char *resource_name)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;
    int                        ret;

    if (NULL == resource_name) {
        resource_name = resource_handle->resource_name;
    }

    ret = _qcloud_iot_resource_report_download_result(resource_handle, resource_name,
                                                      QCLOUD_RESOURCE_RESULTCODE_TIMEOUT_E);

    return ret;
}

/* check whether is downloading */
int QCLOUD_IOT_RESOURCE_IsStartDownload(void *handle)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    if (NULL == handle) {
        Log_e("handle is NULL");
        return 0;
    }

    if (QCLOUD_RESOURCE_STATE_UNINITED_E == resource_handle->state) {
        Log_e("handle is uninitialized");
        resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
        return 0;
    }

    resource_handle->result_code = 0;

    return (QCLOUD_RESOURCE_STATE_START_E == resource_handle->state);
}

/* check whether fetch over */
int QCLOUD_IOT_RESOURCE_IsDownloadFinish(void *handle)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    if (NULL == resource_handle) {
        Log_e("handle is NULL");
        return 0;
    }

    if (QCLOUD_RESOURCE_STATE_UNINITED_E == resource_handle->state) {
        Log_e("handle is uninitialized");
        resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
        return 0;
    }

    return (QCLOUD_RESOURCE_STATE_END_E == resource_handle->state);
}

/*support continuous transmission of breakpoints*/
int QCLOUD_IOT_RESOURCE_StartDownload(void *handle, uint32_t offset, uint32_t size)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;
    int                        Ret;

    Log_d("to download FW from offset: %u, size: %u", offset, size);
    resource_handle->size_fetched = offset;

    // reset md5 for new download
    if (offset == 0) {
        Ret = QCLOUD_IOT_RESOURCE_DownloadResetClientMD5(resource_handle);
        if (Ret != QCLOUD_RET_SUCCESS) {
            Log_e("initialize md5 failed");
            return QCLOUD_ERR_FAILURE;
        }
    }

    // reinit ofc
    qcloud_ofc_deinit(resource_handle->ch_fetch);
    resource_handle->ch_fetch = ofc_Init(resource_handle->resource_url, offset, size);
    if (NULL == resource_handle->ch_fetch) {
        Log_e("Initialize fetch module failed");
        return QCLOUD_ERR_FAILURE;
    }

    Ret = qcloud_ofc_connect(resource_handle->ch_fetch);
    if (QCLOUD_RET_SUCCESS != Ret) {
        Log_e("Connect fetch module failed");
        resource_handle->state = QCLOUD_RESOURCE_STATE_DISCONNECTED_E;
    }

    return Ret;
}

int QCLOUD_IOT_RESOURCE_DownloadYield(void *handle, char *buf, uint32_t buf_len, uint32_t timeout_s)
{
    int                        ret;
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    POINTER_SANITY_CHECK(handle, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);
    POINTER_SANITY_CHECK(buf, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);
    NUMBERIC_SANITY_CHECK(buf_len, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);

    if ((QCLOUD_RESOURCE_STATE_START_E != resource_handle->state) || (resource_handle->result_code != 0)) {
        Log_e("result code :%d", resource_handle->result_code);
        resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
        return QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
    }

    ret = qcloud_ofc_fetch(resource_handle->ch_fetch, buf, buf_len, timeout_s);
    if (ret < 0) {
        resource_handle->state = QCLOUD_RESOURCE_STATE_END_E;
        resource_handle->err   = QCLOUD_RESOURCE_ERRCODE_FETCH_FAILED_E;

        if (ret == IOT_OTA_ERR_FETCH_AUTH_FAIL) {  // OTA auth failed
            Log_e("recv buf %s", buf);
            _qcloud_iot_resource_report_download_result(resource_handle, resource_handle->resource_name,
                                                        QCLOUD_RESOURCE_RESULTCODE_SIGN_INVALID_E);
            resource_handle->err = ret;
        } else if (ret == IOT_OTA_ERR_FETCH_NOT_EXIST) {  // fetch not existed
            Log_e("recv buf %s", buf);
            _qcloud_iot_resource_report_download_result(resource_handle, resource_handle->resource_name,
                                                        QCLOUD_RESOURCE_RESULTCODE_FILE_NOTEXIST_E);
            resource_handle->err = ret;
        } else if (ret == IOT_OTA_ERR_FETCH_TIMEOUT) {  // fetch timeout
            Log_e("recv buf %s", buf);
            _qcloud_iot_resource_report_download_result(resource_handle, resource_handle->resource_name,
                                                        QCLOUD_RESOURCE_RESULTCODE_TIMEOUT_E);
            resource_handle->err = ret;
        }

        return ret;
    } else if (0 == resource_handle->size_fetched) {
        /* force report status in the first */
        _qcloud_iot_resource_report_download_progress(resource_handle, 0, resource_handle->resource_name);

        InitTimer(&resource_handle->report_timer);
        countdown(&resource_handle->report_timer, 1);
    }

    resource_handle->size_last_fetched = ret;
    resource_handle->size_fetched += ret;

    /* report percent every second. */
    uint32_t percent = (resource_handle->size_fetched * 100) / resource_handle->resource_size;
    if (percent == 100) {
        _qcloud_iot_resource_report_download_progress(resource_handle, percent, resource_handle->resource_name);
    } else if (resource_handle->size_last_fetched > 0 && expired(&resource_handle->report_timer)) {
        _qcloud_iot_resource_report_download_progress(resource_handle, percent, resource_handle->resource_name);
        countdown(&resource_handle->report_timer, 1);
    }

    if (resource_handle->size_fetched >= resource_handle->resource_size) {
        resource_handle->state = QCLOUD_RESOURCE_STATE_END_E;
    }

    qcloud_lib_md5_update(&(resource_handle->md5), buf, ret);

    return ret;
}

int QCLOUD_IOT_RESOURCE_DownloadIoctl(void *handle, QCLOUD_IOT_RESOURCE_CMDTYPE_E type, void *buf, size_t buf_len)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    POINTER_SANITY_CHECK(handle, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);
    POINTER_SANITY_CHECK(buf, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);
    NUMBERIC_SANITY_CHECK(buf_len, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);

    if (resource_handle->state < QCLOUD_RESOURCE_STATE_START_E) {
        resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
        return QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
    }

    switch (type) {
        case QCLOUD_IOT_RESOURCE_FETCHED_SIZE_E:
            if ((4 != buf_len) || (0 != ((unsigned long)buf & 0x3))) {
                Log_e("Invalid parameter");
                resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
                return QCLOUD_ERR_FAILURE;
            } else {
                *((uint32_t *)buf) = resource_handle->size_fetched;
                return 0;
            }

        case QCLOUD_IOT_RESOURCE_SIZE_E:
            if ((4 != buf_len) || (0 != ((unsigned long)buf & 0x3))) {
                Log_e("Invalid parameter");
                resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
                return QCLOUD_ERR_FAILURE;
            } else {
                *((uint32_t *)buf) = resource_handle->resource_size;
                return 0;
            }

        case QCLOUD_IOT_RESOURCE_NAME_E:
            strncpy(buf, resource_handle->resource_name, buf_len);
            ((char *)buf)[buf_len - 1] = '\0';
            break;

        case QCLOUD_IOT_RESOURCE_MD5SUM_E:
            strncpy(buf, resource_handle->md5sum, buf_len);
            ((char *)buf)[buf_len - 1] = '\0';
            break;

        case QCLOUD_IOT_RESOURCE_MD5CHECK_E:
            if ((4 != buf_len) || (0 != ((unsigned long)buf & 0x3))) {
                Log_e("Invalid parameter");
                resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
                return QCLOUD_ERR_FAILURE;
            } else if (resource_handle->state != QCLOUD_RESOURCE_STATE_END_E) {
                resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
                Log_e("resource can be checked in QCLOUD_RESOURCE_STATE_FETCHED state only");
                return QCLOUD_ERR_FAILURE;
            } else {
                char md5_str[33];
                qcloud_lib_md5_finish_tolowercasehex(&(resource_handle->md5), md5_str);
                Log_i("download MD5 check: origin=%s, now=%s", STRING_PTR_PRINT_SANITY_CHECK(resource_handle->md5sum),
                      md5_str);
                if (0 == strcmp(resource_handle->md5sum, md5_str)) {
                    *((uint32_t *)buf) = 1;
                } else {
                    *((uint32_t *)buf) = 0;
                    // report MD5 inconsistent TODO
                    _qcloud_iot_resource_report_download_result(resource_handle, resource_handle->resource_name,
                                                                QCLOUD_RESOURCE_RESULTCODE_MD5_NOTMATCH_E);
                }
                return 0;
            }

        default:
            Log_e("invalid cmd type");
            resource_handle->err = QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
            return QCLOUD_ERR_FAILURE;
    }

    return 0;
}

int QCLOUD_IOT_RESOURCE_UploadIoctl(void *handle, QCLOUD_IOT_RESOURCE_CMDTYPE_E type, void *buf, size_t buf_len)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    POINTER_SANITY_CHECK(handle, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);
    POINTER_SANITY_CHECK(buf, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);
    NUMBERIC_SANITY_CHECK(buf_len, QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E);

    if (resource_handle->upload_state < QCLOUD_RESOURCE_STATE_START_E) {
        resource_handle->upload_err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
        return QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
    }

    switch (type) {
        case QCLOUD_IOT_RESOURCE_FETCHED_SIZE_E:
            if ((4 != buf_len) || (0 != ((unsigned long)buf & 0x3))) {
                Log_e("Invalid parameter");
                resource_handle->upload_err = QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
                return QCLOUD_ERR_FAILURE;
            } else {
                *((uint32_t *)buf) = resource_handle->size_fetched;
                return 0;
            }

        case QCLOUD_IOT_RESOURCE_UPLOADED_SIZE_E:
            if ((4 != buf_len) || (0 != ((unsigned long)buf & 0x3))) {
                Log_e("Invalid parameter");
                resource_handle->upload_err = QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
                return QCLOUD_ERR_FAILURE;
            } else {
                *((uint32_t *)buf) = resource_handle->size_uploaded;
                return 0;
            }

        case QCLOUD_IOT_RESOURCE_SIZE_E:
            if ((4 != buf_len) || (0 != ((unsigned long)buf & 0x3))) {
                Log_e("Invalid parameter");
                resource_handle->upload_err = QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
                return QCLOUD_ERR_FAILURE;
            } else {
                *((uint32_t *)buf) = resource_handle->upload_resource_size;
                return 0;
            }

        case QCLOUD_IOT_RESOURCE_NAME_E:
            strncpy(buf, resource_handle->upload_resource_name, buf_len);
            ((char *)buf)[buf_len - 1] = '\0';
            break;

        case QCLOUD_IOT_RESOURCE_MD5SUM_E:
            strncpy(buf, resource_handle->upload_md5sum, buf_len);
            ((char *)buf)[buf_len - 1] = '\0';
            break;

        case QCLOUD_IOT_RESOURCE_MD5CHECK_E:
            if ((4 != buf_len) || (0 != ((unsigned long)buf & 0x3))) {
                Log_e("Invalid parameter");
                resource_handle->upload_err = QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
                return QCLOUD_ERR_FAILURE;
            } else if (resource_handle->upload_state != QCLOUD_RESOURCE_STATE_END_E) {
                resource_handle->upload_err = QCLOUD_RESOURCE_ERRCODE_INVALID_STATE_E;
                Log_e("resource can be checked in QCLOUD_RESOURCE_STATE_FETCHED state only");
                return QCLOUD_ERR_FAILURE;
            } else {
                char md5_str[33];
                qcloud_lib_md5_finish_tolowercasehex(&(resource_handle->upload_md5), md5_str);
                Log_e("upload MD5 check: origin=%s, now=%s",
                      STRING_PTR_PRINT_SANITY_CHECK(resource_handle->upload_md5sum), md5_str);

                if (0 == strcmp(resource_handle->upload_md5sum, md5_str)) {
                    *((uint32_t *)buf) = 1;
                } else {
                    *((uint32_t *)buf) = 0;
                    // report MD5 inconsistent TODO
                    _qcloud_iot_resource_report_upload_result(resource_handle, resource_handle->upload_resource_name,
                                                              QCLOUD_RESOURCE_RESULTCODE_MD5_NOTMATCH_E);
                }
                return 0;
            }

        default:
            Log_e("invalid cmd type");
            resource_handle->upload_err = QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
            return QCLOUD_ERR_FAILURE;
    }

    return 0;
}

/* Get last error code */
int QCLOUD_IOT_RESOURCE_DownloadGetLastError(void *handle)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    if (NULL == resource_handle) {
        Log_e("resource_handle is NULL");
        return QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
    }

    return resource_handle->err;
}

int QCLOUD_IOT_RESOURCE_UploadGetLastError(void *handle)
{
    QCLOUD_RESOURCE_CONTEXT_T *resource_handle = (QCLOUD_RESOURCE_CONTEXT_T *)handle;

    if (NULL == resource_handle) {
        Log_e("resource_handle is NULL");
        return QCLOUD_RESOURCE_ERRCODE_INVALID_PARAM_E;
    }

    return resource_handle->upload_err;
}

#ifdef __cplusplus
}
#endif
