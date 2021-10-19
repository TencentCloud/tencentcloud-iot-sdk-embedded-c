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

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"

#ifdef LOG_UPLOAD

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lite-utils.h"
#include "log_upload.h"
#include "qcloud_iot_common.h"
#include "utils_hmac.h"
#include "utils_httpc.h"
#include "utils_timer.h"
#include "qcloud_iot_http.h"
#include "qcloud_iot_ca.h"

/* log post header format */
#define TIMESTAMP_SIZE  10
#define SIGNATURE_SIZE  40
#define CTRL_BYTES_SIZE 4

/* do immediate log update if buffer is lower than this threshold (about two max log item) */
#define LOG_LOW_BUFFER_THRESHOLD (LOG_UPLOAD_BUFFER_SIZE / 4)

/* log upload buffer */
static char *   sg_log_buffer  = NULL;
static uint32_t sg_write_index = 0;

#define SIGN_KEY_SIZE 24
static char sg_sign_key[SIGN_KEY_SIZE + 1] = {0};

static void *sg_sign_log_privatekey = NULL;
static int   sg_log_buffer_head_len = 0;

/* Log upload feature switch */
/* To check log http server return msg or not */
#define LOG_CHECK_HTTP_RET_CODE

typedef struct {
    const char *   url;
    const char *   ca_crt;
    int            port;
    HTTPClient     http;      /* http client */
    HTTPClientData http_data; /* http client data */

} LogHTTPStruct;

static LogHTTPStruct *sg_http_c = NULL;

typedef struct {
    const char *product_id;
    const char *device_name;
    void *      mqtt_client;
    bool        upload_only_in_comm_err;

    void *lock_buf;
    Timer upload_timer;
#ifndef LOG_UPDATE_TIME_WHEN_UPLOAD
    Timer time_update_timer;
#endif
    long system_time;

    LogSaveFunc    save_func;
    LogReadFunc    read_func;
    LogDelFunc     del_func;
    LogGetSizeFunc get_size_func;
    bool           log_save_enabled;

    bool uploader_init_done;

} LogUploaderStruct;

static LogUploaderStruct *sg_uploader               = NULL;
static bool               sg_log_uploader_init_done = false;

static int _check_server_connection()
{
    int  rc;
    char url[128] = {0};
    int  port;

    /*format URL*/
    const char *url_format = "%s://%s%s";

    HAL_Snprintf(url, 128, url_format, "http", sg_http_c->url, UPLOAD_LOG_URI_PATH);
    port = LOG_UPLOAD_SERVER_PORT;

    rc = qcloud_http_client_connect(&sg_http_c->http, url, port, sg_http_c->ca_crt);
    if (rc != QCLOUD_RET_SUCCESS)
        return rc;

    qcloud_http_client_close(&sg_http_c->http);

    return QCLOUD_RET_SUCCESS;
}

#ifdef LOG_CHECK_HTTP_RET_CODE
static bool _get_json_ret_code(char *json)
{
    char *error = NULL;
    char *v     = LITE_json_value_of("Response", json);
    if (v == NULL) {
        UPLOAD_ERR("Invalid json content: %s", STRING_PTR_PRINT_SANITY_CHECK(json));
        return false;
    }

    error = LITE_json_value_of("Error", v);
    if (NULL != error) {
        UPLOAD_ERR("upload failed; json content: %s", json);
        HAL_Free(v);
        HAL_Free(error);
        return false;
    }
    HAL_Free(v);
    return true;
}
#endif

static int _post_one_http_to_server(char *post_buf, size_t post_size)
{
    int   rc       = 0;
    char  url[128] = {0};
    int   port;
    char *upload_log_uri = UPLOAD_LOG_URI_PATH;

    if (sg_http_c == NULL)
        return QCLOUD_ERR_INVAL;

    sg_http_c->http.header = qcloud_iot_http_header_create(post_buf, post_size, sg_http_c->url, upload_log_uri,
                                                           "application/json;", sg_sign_key, sg_sign_log_privatekey);

    sg_http_c->http_data.post_content_type = "application/json;charset=utf-8";
    sg_http_c->http_data.post_buf          = post_buf;
    sg_http_c->http_data.post_buf_len      = post_size;

    /*format URL*/
    const char *url_format = "%s://%s%s";

    HAL_Snprintf(url, 128, url_format, "http", sg_http_c->url, upload_log_uri);
    port = LOG_UPLOAD_SERVER_PORT;

    rc = qcloud_http_client_common(&sg_http_c->http, url, port, sg_http_c->ca_crt, HTTP_POST, &sg_http_c->http_data);
    if (rc != QCLOUD_RET_SUCCESS) {
        qcloud_iot_http_header_destory(sg_http_c->http.header);
        UPLOAD_ERR("qcloud_http_client_common failed, rc = %d", rc);
        return rc;
    }
    UPLOAD_DBG("Log client POST size: %d", post_size);

#ifdef LOG_CHECK_HTTP_RET_CODE
    /* TODO: handle recv data from log server */
#define HTTP_RET_JSON_LENGTH     256
#define HTTP_WAIT_RET_TIMEOUT_MS 1000
    char buf[HTTP_RET_JSON_LENGTH]        = {0};
    sg_http_c->http_data.response_buf     = buf;
    sg_http_c->http_data.response_buf_len = sizeof(buf);

    rc = qcloud_http_recv_data(&sg_http_c->http, HTTP_WAIT_RET_TIMEOUT_MS, &sg_http_c->http_data);
    if (QCLOUD_RET_SUCCESS != rc) {
        UPLOAD_ERR("qcloud_http_recv_data failed, rc = %d", rc);
    } else {
        buf[HTTP_RET_JSON_LENGTH - 1] = '\0';  // json_parse relies on a string
        if (strlen(buf) > 0 && _get_json_ret_code(buf)) {
            UPLOAD_DBG("Log server return SUCCESS: %s", buf);
        } else {
            UPLOAD_ERR("Log server return FAIL: %s", buf);
            rc = QCLOUD_ERR_HTTP;
        }
    }
#endif

    qcloud_iot_http_header_destory(sg_http_c->http.header);
    qcloud_http_client_close(&sg_http_c->http);

    return rc;
}

static void _append_endchar_to_upload_buffer(char *log_buf, size_t log_size)
{
    log_buf[log_size - 2] = ']';
    log_buf[log_size - 1] = '}';
}

static void _remove_endchar_from_upload_buffer(char *log_buf, size_t log_size)
{
    log_buf[log_size - 2] = ',';
    log_buf[log_size - 1] = ' ';
}

static int _post_log_to_server(char *post_buf, size_t post_size, size_t *actual_post_payload)
{
#define LOG_DELIMITER "\", "
    int ret = QCLOUD_RET_SUCCESS;
    /* one shot upload */
    if (post_size < MAX_HTTP_LOG_POST_SIZE) {
        _append_endchar_to_upload_buffer(post_buf, post_size);
        ret = _post_one_http_to_server(post_buf, post_size);
        _remove_endchar_from_upload_buffer(post_buf, post_size);

        if (QCLOUD_RET_SUCCESS == ret) {
            *actual_post_payload = post_size - sg_log_buffer_head_len;
        } else {
            UPLOAD_ERR("one time log send failed");
            *actual_post_payload = 0;
        }
        return ret;
    }

    /* Log size is larger than one HTTP post size */
    /* Fragment the log and upload multi-times */
    UPLOAD_DBG("to post large log size %d", post_size);
    *actual_post_payload  = 0;
    size_t delimiter_len  = strlen(LOG_DELIMITER);
    size_t orig_post_size = post_size;
    size_t post_payload, upload_size, possible_size;
    do {
        char *next_log_buf = NULL;
        possible_size      = 0;
        while (possible_size < MAX_HTTP_LOG_POST_SIZE) {
            /*remember last valid position */
            upload_size = possible_size;
            /* locate the delimiter */
            next_log_buf = strstr(post_buf + upload_size, LOG_DELIMITER);
            if (next_log_buf == NULL) {
                UPLOAD_ERR("Invalid log delimiter. Total sent: %d. Left: %d",
                           *actual_post_payload + sg_log_buffer_head_len, post_size);
                return QCLOUD_ERR_INVAL;
            }
            possible_size = (size_t)(next_log_buf - post_buf + delimiter_len);
            /* end of log */
            if (next_log_buf[delimiter_len] == 0 && possible_size < sg_log_buffer_head_len) {
                upload_size = possible_size;
                break;
            }
        }

        if (upload_size == 0) {
            UPLOAD_ERR("Upload size should not be 0! Total sent: %d. Left: %d",
                       *actual_post_payload + sg_log_buffer_head_len, post_size);
            return QCLOUD_ERR_FAILURE;
        }

        _append_endchar_to_upload_buffer(post_buf, upload_size);
        ret = _post_one_http_to_server(post_buf, upload_size);
        _remove_endchar_from_upload_buffer(post_buf, post_size);

        if (QCLOUD_RET_SUCCESS != ret) {
            UPLOAD_ERR("Send log failed. Total sent: %d. Left: %d", *actual_post_payload + sg_log_buffer_head_len,
                       post_size);
            return QCLOUD_ERR_FAILURE;
        }

        /* move the left log forward and do next upload */
        memmove(post_buf + sg_log_buffer_head_len, post_buf + upload_size, post_size - upload_size);
        post_payload = upload_size - sg_log_buffer_head_len;
        post_size -= post_payload;
        *actual_post_payload += post_payload;
        memset(post_buf + post_size, 0, orig_post_size - post_size);
        UPLOAD_DBG("post log %d OK. Total sent: %d. Left: %d", upload_size,
                   *actual_post_payload + sg_log_buffer_head_len, post_size);
    } while (post_size > sg_log_buffer_head_len);

    return QCLOUD_RET_SUCCESS;
}

static void _reset_log_buffer(void)
{
    sg_write_index = sg_log_buffer_head_len;
    memset(sg_log_buffer + sg_log_buffer_head_len, 0, LOG_UPLOAD_BUFFER_SIZE - sg_log_buffer_head_len);
}

static int _save_log(char *log_buf, size_t log_size)
{
    int    rc = 0;
    size_t write_size, current_size = sg_uploader->get_size_func();

    /* overwrite the previous saved log to avoid too many saved logs */
    if ((current_size + log_size) > MAX_LOG_SAVE_SIZE) {
        UPLOAD_ERR("overwrite the previous saved log. %d", current_size);
        rc = sg_uploader->del_func();
        if (rc) {
            Log_e("fail to delete previous log");
        }
    }

    write_size = sg_uploader->save_func(log_buf, log_size);
    if (write_size != log_size) {
        Log_e("fail to save log. RC %d - log size %d", write_size, log_size);
        rc = -1;
    } else {
        rc = 0;
    }

    return rc;
}

static int _handle_saved_log(void)
{
    int    rc             = QCLOUD_RET_SUCCESS;
    size_t whole_log_size = sg_uploader->get_size_func();
    if (whole_log_size > 0) {
        /* only do the job when connection is OK */
        if (_check_server_connection() != QCLOUD_RET_SUCCESS)
            return QCLOUD_ERR_FAILURE;

        size_t buf_size = whole_log_size + sg_log_buffer_head_len + 1;
        char * log_buf  = HAL_Malloc(buf_size);
        if (log_buf != NULL) {
            /* read the whole log to buffer */
            size_t read_len = sg_uploader->read_func(log_buf + sg_log_buffer_head_len, whole_log_size);
            if (read_len == whole_log_size) {
                size_t upload_size = whole_log_size + sg_log_buffer_head_len;

                /* copy header from global log buffer */
                memcpy(log_buf, sg_log_buffer, sg_log_buffer_head_len);
                log_buf[buf_size - 1] = 0;

                size_t actual_post_payload;
                rc = _post_log_to_server(log_buf, upload_size, &actual_post_payload);
                if (rc == QCLOUD_RET_SUCCESS || rc == QCLOUD_ERR_INVAL) {
                    Log_d("handle saved log done! Size: %d. upload paylod: %d", whole_log_size, actual_post_payload);
                    sg_uploader->del_func();
                }
                HAL_Free(log_buf);
            } else {
                Log_e("fail to read whole saved log. Size: %u - read: %u", whole_log_size, read_len);
                HAL_Free(log_buf);
                return QCLOUD_ERR_FAILURE;
            }

        } else {
            Log_e("Malloc failed, size: %u", buf_size);
            return QCLOUD_ERR_FAILURE;
        }
    }

    return rc;
}

void set_log_mqtt_client(void *client)
{
    if (!sg_log_uploader_init_done)
        return;

    sg_uploader->mqtt_client = client;
}

void set_log_upload_in_comm_err(bool value)
{
    if (!sg_log_uploader_init_done)
        return;

    sg_uploader->upload_only_in_comm_err = value;
}

int append_to_upload_buffer(const char *log_content, size_t log_size)
{
    int index = 0;
    if (!sg_log_uploader_init_done)
        return -1;

    if (log_content == NULL || log_size == 0) {
        UPLOAD_ERR("invalid log content!");
        return -1;
    }

    if (HAL_MutexTryLock(sg_uploader->lock_buf) != 0) {
        UPLOAD_ERR("trylock buffer failed!");
        return -1;
    }

    /* ["log 1", "log 2", "log 3", ] */
    /* [\"\", ] 4B char */
    if ((sg_write_index + log_size + 5) > LOG_UPLOAD_BUFFER_SIZE) {
        countdown_ms(&sg_uploader->upload_timer, 0);
        HAL_MutexUnlock(sg_uploader->lock_buf);
        UPLOAD_ERR("log upload buffer is not enough!");
        return -1;
    }

    sg_log_buffer[sg_write_index++] = '\"';
    while (index < (log_size - 2)) {
        // replace may overflow buffer size
        if ((sg_write_index + 5) > LOG_UPLOAD_BUFFER_SIZE) {
            break;
        }
        // replace special char for json
        switch (log_content[index]) {
            case '\"':
                sg_log_buffer[sg_write_index++] = '\\';
                sg_log_buffer[sg_write_index++] = '\"';
                break;
            case '\\':
                sg_log_buffer[sg_write_index++] = '\\';
                sg_log_buffer[sg_write_index++] = '\\';
                break;
            case '/':
                sg_log_buffer[sg_write_index++] = '\\';
                sg_log_buffer[sg_write_index++] = '/';
                break;
            case '\b':
                sg_log_buffer[sg_write_index++] = '\\';
                sg_log_buffer[sg_write_index++] = 'b';
                break;
            case '\f':
                sg_log_buffer[sg_write_index++] = '\\';
                sg_log_buffer[sg_write_index++] = 'f';
                break;
            case '\n':
                sg_log_buffer[sg_write_index++] = '\\';
                sg_log_buffer[sg_write_index++] = 'n';
                break;
            case '\r':
                sg_log_buffer[sg_write_index++] = '\\';
                sg_log_buffer[sg_write_index++] = 'r';
                break;
            case '\t':
                sg_log_buffer[sg_write_index++] = '\\';
                sg_log_buffer[sg_write_index++] = 't';
                break;
            default:
                sg_log_buffer[sg_write_index++] = log_content[index];
        }
        index++;
    }

    /* replace \r\n to \", add  as delimiter */
    sg_log_buffer[sg_write_index++] = '\"';
    sg_log_buffer[sg_write_index++] = ',';
    sg_log_buffer[sg_write_index++] = ' ';

    HAL_MutexUnlock(sg_uploader->lock_buf);
    return 0;
}

void clear_upload_buffer(void)
{
    if (!sg_log_uploader_init_done)
        return;

    HAL_MutexLock(sg_uploader->lock_buf);
    _reset_log_buffer();
    HAL_MutexUnlock(sg_uploader->lock_buf);
}

int init_log_uploader(LogUploadInitParams *init_params)
{
    if (sg_log_uploader_init_done)
        return QCLOUD_RET_SUCCESS;

    if (init_params == NULL || init_params->product_id == NULL || init_params->device_name == NULL ||
        init_params->sign_key == NULL) {
        UPLOAD_ERR("invalid init parameters");
        return QCLOUD_ERR_INVAL;
    }

    int key_len = strlen(init_params->sign_key);
    if (key_len == 0) {
        UPLOAD_ERR("invalid key length");
        return QCLOUD_ERR_INVAL;
    }

    sg_log_buffer = HAL_Malloc(LOG_UPLOAD_BUFFER_SIZE);
    if (sg_log_buffer == NULL) {
        UPLOAD_ERR("malloc log buffer failed");
        return QCLOUD_ERR_FAILURE;
    }

#ifdef AUTH_MODE_CERT
    sg_sign_log_privatekey = qcloud_iot_http_create_privatekey((char *)init_params->sign_key);
#else
    memcpy(sg_sign_key, init_params->sign_key, key_len > SIGN_KEY_SIZE ? SIGN_KEY_SIZE : key_len);
#endif

    HAL_Snprintf(sg_log_buffer, LOG_UPLOAD_BUFFER_SIZE,
                 "{\"DeviceName\":\"%s\",\"Level\":\"ERR\",\"ProductId\":\"%s\",\"Message\":[",
                 init_params->device_name, init_params->product_id);
    sg_log_buffer_head_len = strlen(sg_log_buffer);
    sg_write_index         = sg_log_buffer_head_len;

    if (NULL == (sg_uploader = HAL_Malloc(sizeof(LogUploaderStruct)))) {
        UPLOAD_ERR("allocate for LogUploaderStruct failed");
        goto err_exit;
    }
    memset(sg_uploader, 0, sizeof(LogUploaderStruct));

    sg_uploader->product_id              = init_params->product_id;
    sg_uploader->device_name             = init_params->device_name;
    sg_uploader->mqtt_client             = NULL;
    sg_uploader->system_time             = 0;
    sg_uploader->upload_only_in_comm_err = false;

    /* all the call back functions are necessary to handle log save and re-upload */
    if (init_params->save_func != NULL && init_params->read_func != NULL && init_params->del_func != NULL &&
        init_params->get_size_func) {
        sg_uploader->save_func        = init_params->save_func;
        sg_uploader->read_func        = init_params->read_func;
        sg_uploader->del_func         = init_params->del_func;
        sg_uploader->get_size_func    = init_params->get_size_func;
        sg_uploader->log_save_enabled = true;
    } else {
        sg_uploader->log_save_enabled = false;
    }

    InitTimer(&sg_uploader->upload_timer);
    InitTimer(&sg_uploader->time_update_timer);

    if ((sg_uploader->lock_buf = HAL_MutexCreate()) == NULL) {
        UPLOAD_ERR("mutex create failed");
        goto err_exit;
    }

    if (NULL == (sg_http_c = HAL_Malloc(sizeof(LogHTTPStruct)))) {
        UPLOAD_ERR("allocate for LogHTTPStruct failed");
        goto err_exit;
    }
    memset(sg_http_c, 0, sizeof(LogHTTPStruct));

    /* set http request-header parameter */
    sg_http_c->http.header = "Accept:application/json;*/*\r\n";
    sg_http_c->url         = iot_get_log_domain(init_params->region);
    sg_http_c->port        = LOG_UPLOAD_SERVER_PORT;
    sg_http_c->ca_crt      = NULL;

    _reset_log_buffer();
    sg_log_uploader_init_done = true;

    return QCLOUD_RET_SUCCESS;
err_exit:
    HAL_Free(sg_log_buffer);
    sg_log_buffer = NULL;

    if (sg_uploader && sg_uploader->lock_buf) {
        HAL_MutexDestroy(sg_uploader->lock_buf);
        sg_uploader->lock_buf = NULL;
    }
    HAL_Free(sg_uploader);
    sg_uploader = NULL;
    HAL_Free(sg_http_c);
    sg_http_c = NULL;
    return QCLOUD_ERR_FAILURE;
}

void fini_log_uploader(void)
{
    if (!sg_log_uploader_init_done)
        return;

    HAL_MutexLock(sg_uploader->lock_buf);
    sg_log_uploader_init_done = false;
    if (sg_log_buffer) {
        _reset_log_buffer();
        HAL_Free(sg_log_buffer);
        sg_log_buffer = NULL;
    }
    HAL_MutexUnlock(sg_uploader->lock_buf);

    HAL_MutexDestroy(sg_uploader->lock_buf);
    sg_uploader->lock_buf = NULL;
    HAL_Free(sg_uploader);
    sg_uploader = NULL;
    HAL_Free(sg_http_c);
    sg_http_c = NULL;

#ifdef AUTH_MODE_CERT
    qcloud_iot_http_destory_privatekey(sg_sign_log_privatekey);
#endif
}

bool is_log_uploader_init(void)
{
    return sg_log_uploader_init_done;
}

static bool _check_force_upload(bool force_upload)
{
    if (!force_upload) {
        /* Double check if the buffer is low */
        HAL_MutexLock(sg_uploader->lock_buf);
        bool is_low_buffer = (LOG_UPLOAD_BUFFER_SIZE - sg_write_index) < LOG_LOW_BUFFER_THRESHOLD ? true : false;

        /* force_upload is false and upload_only_in_comm_err is true */
        if (sg_uploader->upload_only_in_comm_err) {
            /* buffer is low but we couldn't upload now, reset buffer */
            if (is_low_buffer)
                _reset_log_buffer();

            HAL_MutexUnlock(sg_uploader->lock_buf);
            countdown_ms(&sg_uploader->upload_timer, LOG_UPLOAD_INTERVAL_MS);
            return false;
        }
        HAL_MutexUnlock(sg_uploader->lock_buf);

        if (is_low_buffer) {
            /* buffer is low, handle it right now */
            return true;
        } else {
            return expired(&sg_uploader->upload_timer);
        }

    } else {
        return true;
    }
}

int do_log_upload(bool force_upload)
{
    int         rc;
    int         upload_log_size    = 0;
    static bool unhandle_saved_log = true;

    if (!sg_log_uploader_init_done)
        return QCLOUD_ERR_FAILURE;

    /* double check force upload */
    if (!_check_force_upload(force_upload))
        return QCLOUD_RET_SUCCESS;

    /* handle previously saved log */
    if (sg_uploader->log_save_enabled && unhandle_saved_log) {
        rc = _handle_saved_log();
        if (rc == QCLOUD_RET_SUCCESS)
            unhandle_saved_log = false;
    }

    /* no more log in buffer */
    if (sg_write_index == sg_log_buffer_head_len)
        return QCLOUD_RET_SUCCESS;

    HAL_MutexLock(sg_uploader->lock_buf);
    upload_log_size = sg_write_index;
    HAL_MutexUnlock(sg_uploader->lock_buf);

    size_t actual_post_payload;
    rc = _post_log_to_server(sg_log_buffer, upload_log_size, &actual_post_payload);
    if (rc != QCLOUD_RET_SUCCESS) {
        /* save log via user callbacks when log upload fail */
        if (sg_uploader->log_save_enabled) {
            /* new error logs should have been added, update log size */
            HAL_MutexLock(sg_uploader->lock_buf);
            /* parts of log were uploaded succesfully. Need to move the new logs forward */
            if (actual_post_payload) {
                UPLOAD_DBG("move the new log %d forward", actual_post_payload);
                memmove(sg_log_buffer + upload_log_size - actual_post_payload, sg_log_buffer + upload_log_size,
                        sg_write_index - upload_log_size);
                sg_write_index = sg_write_index - actual_post_payload;
                memset(sg_log_buffer + sg_write_index, 0, LOG_UPLOAD_BUFFER_SIZE - sg_write_index);
            }
            upload_log_size = sg_write_index;
            HAL_MutexUnlock(sg_uploader->lock_buf);
            _save_log(sg_log_buffer + sg_log_buffer_head_len, upload_log_size - sg_log_buffer_head_len);
            unhandle_saved_log = true;
        }
    }

    /* move the new log during send_log_to_server */
    HAL_MutexLock(sg_uploader->lock_buf);
    if (upload_log_size == sg_write_index) {
        _reset_log_buffer();
    } else {
        memmove(sg_log_buffer + sg_log_buffer_head_len, sg_log_buffer + upload_log_size,
                sg_write_index - upload_log_size);
        sg_write_index = sg_write_index - upload_log_size + sg_log_buffer_head_len;
        memset(sg_log_buffer + sg_write_index, 0, LOG_UPLOAD_BUFFER_SIZE - sg_write_index);
    }
    HAL_MutexUnlock(sg_uploader->lock_buf);

    countdown_ms(&sg_uploader->upload_timer, LOG_UPLOAD_INTERVAL_MS);

    return QCLOUD_RET_SUCCESS;
}

#endif

#ifdef __cplusplus
}
#endif
