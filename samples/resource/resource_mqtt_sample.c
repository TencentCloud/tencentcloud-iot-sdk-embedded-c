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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lite-utils.h"
#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"

#define KEY_SIZE "downloaded_size"

#define RESOURCE_NAME_MAX_LEN       32
#define RESOURCE_PATH_MAX_LEN       128
#define RESOURCE_BUF_LEN            5000
#define RESOURCE_INFO_FILE_DATA_LEN 128

typedef struct ResourceContextData {
    void *   resource_handle;
    void *   mqtt_client;
    char     resource_name[RESOURCE_NAME_MAX_LEN];
    uint32_t resource_size;
    uint32_t resource_downloaded_size;
    char     resource_file_path[RESOURCE_PATH_MAX_LEN];
    char     resource_info_file_path[RESOURCE_PATH_MAX_LEN];

    // to make sure report is acked
    bool download_report_pub_ack;
    int  download_report_packet_id;

    bool upload_report_pub_ack;
    int  upload_report_packet_id;

#ifdef MULTITHREAD_ENABLED
    bool download_thread;
    bool upload_thread;
#endif
} ResourceContextData;

static void _event_handler(void *pclient, void *handle_context, MQTTEventMsg *msg)
{
    uintptr_t            packet_id    = (uintptr_t)msg->msg;
    ResourceContextData *resource_ctx = (ResourceContextData *)handle_context;

    switch (msg->event_type) {
        case MQTT_EVENT_UNDEF:
            Log_i("undefined event occur.");
            break;

        case MQTT_EVENT_DISCONNECT:
            Log_i("MQTT disconnect.");
            break;

        case MQTT_EVENT_RECONNECT:
            Log_i("MQTT reconnect.");
            break;

        case MQTT_EVENT_SUBCRIBE_SUCCESS:
            Log_i("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_SUBCRIBE_TIMEOUT:
            Log_i("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_SUBCRIBE_NACK:
            Log_i("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_PUBLISH_SUCCESS:
            Log_i("publish success, packet-id=%u", (unsigned int)packet_id);
            if (resource_ctx->upload_report_packet_id == packet_id) {
                resource_ctx->upload_report_pub_ack = true;
            } else if (resource_ctx->download_report_packet_id == packet_id) {
                resource_ctx->download_report_pub_ack = true;
            }
            break;

        case MQTT_EVENT_PUBLISH_TIMEOUT:
            Log_i("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_PUBLISH_NACK:
            Log_i("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;
        default:
            Log_i("Should NOT arrive here.");
            break;
    }
}

static int _setup_connect_init_params(MQTTInitParams *initParams, void *resource_ctx, DeviceInfo *device_info)
{
    initParams->product_id  = device_info->product_id;
    initParams->device_name = device_info->device_name;

#ifdef AUTH_MODE_CERT
    char  certs_dir[16] = "certs";
    char  current_path[128];
    char *cwd = getcwd(current_path, sizeof(current_path));

    if (cwd == NULL) {
        Log_e("getcwd return NULL");
        return QCLOUD_ERR_FAILURE;
    }

#ifdef WIN32
    HAL_Snprintf(initParams->cert_file, FILE_PATH_MAX_LEN, "%s\\%s\\%s", current_path, certs_dir,
                 STRING_PTR_PRINT_SANITY_CHECK(device_info->dev_cert_file_name));
    HAL_Snprintf(initParams->key_file, FILE_PATH_MAX_LEN, "%s\\%s\\%s", current_path, certs_dir,
                 STRING_PTR_PRINT_SANITY_CHECK(device_info->dev_key_file_name));
#else
    HAL_Snprintf(initParams->cert_file, FILE_PATH_MAX_LEN, "%s/%s/%s", current_path, certs_dir,
                 STRING_PTR_PRINT_SANITY_CHECK(device_info->dev_cert_file_name));
    HAL_Snprintf(initParams->key_file, FILE_PATH_MAX_LEN, "%s/%s/%s", current_path, certs_dir,
                 STRING_PTR_PRINT_SANITY_CHECK(device_info->dev_key_file_name));
#endif

#else
    initParams->device_secret = device_info->device_secret;
#endif

    initParams->command_timeout        = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
    initParams->keep_alive_interval_ms = QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;

    initParams->auto_connect_enable  = 1;
    initParams->event_handle.h_fp    = _event_handler;
    initParams->event_handle.context = resource_ctx;

    return QCLOUD_RET_SUCCESS;
}

static void _wait_for_download_pub_ack(ResourceContextData *resource_ctx, int packet_id)
{
    int wait_cnt                            = 10;
    resource_ctx->download_report_pub_ack   = false;
    resource_ctx->download_report_packet_id = packet_id;

    while (!resource_ctx->download_report_pub_ack) {
        HAL_SleepMs(500);
        IOT_MQTT_Yield(resource_ctx->mqtt_client, 500);
        if (wait_cnt-- == 0) {
            Log_e("wait report pub ack timeout!");
            break;
        }
    }
    resource_ctx->download_report_pub_ack = false;
    return;
}

static void _wait_for_upload_pub_ack(ResourceContextData *resource_ctx, int packet_id)
{
    int wait_cnt                          = 10;
    resource_ctx->upload_report_pub_ack   = false;
    resource_ctx->upload_report_packet_id = packet_id;

    while (!resource_ctx->upload_report_pub_ack) {
        HAL_SleepMs(500);
        IOT_MQTT_Yield(resource_ctx->mqtt_client, 500);
        if (wait_cnt-- == 0) {
            Log_e("wait report pub ack timeout!");
            break;
        }
    }
    resource_ctx->upload_report_pub_ack = false;
    return;
}

/**********************************************************************************
 * resource file operations START
 * these are platform-dependant functions
 * POSIX FILE is used in this sample code
 **********************************************************************************/
// calculate left MD5 for resuming download from break point
static int _cal_exist_resource_md5(ResourceContextData *resource_ctx)
{
    char   buff[RESOURCE_BUF_LEN];
    size_t rlen, total_read = 0;
    int    ret = QCLOUD_RET_SUCCESS;

    ret = QCLOUD_IOT_RESOURCE_DownloadResetClientMD5(resource_ctx->resource_handle);
    if (ret) {
        Log_e("reset MD5 failed: %d", ret);
        return QCLOUD_ERR_FAILURE;
    }

    FILE *fp = fopen(resource_ctx->resource_file_path, "ab+");
    if (NULL == fp) {
        Log_e("open file %s failed", STRING_PTR_PRINT_SANITY_CHECK(resource_ctx->resource_file_path));
        return QCLOUD_ERR_FAILURE;
    }

    // rewind(fp);
    size_t size = resource_ctx->resource_downloaded_size;

    while ((size > 0) && (!feof(fp))) {
        rlen = (size > RESOURCE_BUF_LEN) ? RESOURCE_BUF_LEN : size;
        if (rlen != fread(buff, 1, rlen, fp)) {
            Log_e("read data len not expected");
            ret = QCLOUD_ERR_FAILURE;
            break;
        }
        QCLOUD_IOT_RESOURCE_UpdateDownloadClientMd5(resource_ctx->resource_handle, buff, rlen);
        size -= rlen;
        total_read += rlen;
    }

    fclose(fp);
    Log_d("total read: %d", total_read);
    return ret;
}

/* update local firmware info for resuming download from break point */
static int _update_local_resource_info(ResourceContextData *resource_ctx)
{
    FILE *fp;
    int   wlen;
    int   ret = QCLOUD_RET_SUCCESS;
    char  data_buf[RESOURCE_INFO_FILE_DATA_LEN];

    memset(data_buf, 0, sizeof(data_buf));
    HAL_Snprintf(data_buf, sizeof(data_buf), "{\"%s\":%d}", KEY_SIZE, resource_ctx->resource_downloaded_size);

    fp = fopen(resource_ctx->resource_info_file_path, "w");
    if (NULL == fp) {
        Log_e("open file %s failed", STRING_PTR_PRINT_SANITY_CHECK(resource_ctx->resource_info_file_path));
        ret = QCLOUD_ERR_FAILURE;
        goto exit;
    }

    wlen = fwrite(data_buf, 1, strlen(data_buf), fp);
    if (wlen != strlen(data_buf)) {
        Log_e("save version to file err");
        ret = QCLOUD_ERR_FAILURE;
    }

exit:

    if (NULL != fp) {
        fclose(fp);
    }

    return ret;
}

static int _get_local_resource_info(char *file_name)
{
    int  len;
    int  rlen;
    char json_doc[RESOURCE_INFO_FILE_DATA_LEN] = {0};

    FILE *fp = fopen(file_name, "r");
    if (NULL == fp) {
        Log_e("open file %s failed", STRING_PTR_PRINT_SANITY_CHECK(file_name));
        return 0;
    }

    fseek(fp, 0L, SEEK_END);
    len = ftell(fp);
    if (len > RESOURCE_INFO_FILE_DATA_LEN) {
        Log_e("%s is too big, pls check", file_name);
        fclose(fp);
        return 0;
    }

    rewind(fp);
    rlen = fread(json_doc, 1, len, fp);
    if (len != rlen) {
        Log_e("read data len (%d) less than needed (%d), %s", rlen, len, json_doc);
        fclose(fp);
        return 0;
    }

    char *size = LITE_json_value_of(KEY_SIZE, json_doc);
    if ((NULL == size)) {
        fclose(fp);
        return 0;
    }

    int local_size = atoi(size);
    HAL_Free(size);

    if (local_size <= 0) {
        Log_w("local info offset invalid: %d", local_size);
        local_size = 0;
    }

    fclose(fp);
    return local_size;
}

static int _read_local_resource_data(char *file_name, uint32_t offset, char *buf, int len)
{
    FILE *fp  = fopen(file_name, "r");
    int   ret = 0;
    if (NULL == fp) {
        Log_e("open file %s failed", STRING_PTR_PRINT_SANITY_CHECK(file_name));
        return 0;
    }

    fseek(fp, offset, SEEK_SET);

    ret = fread(buf, len, 1, fp);
    if (1 != ret) {
        Log_e("read data frome file failed, %s, %d, %d", file_name, offset, len);
        fclose(fp);
        return QCLOUD_ERR_FAILURE;
    }
    fclose(fp);

    return 0;
}

static int _get_local_resource_size(char *file_name)
{
    FILE *fp = fopen(file_name, "r");
    if (NULL == fp) {
        Log_e("open file %s failed", STRING_PTR_PRINT_SANITY_CHECK(file_name));
        return 0;
    }

    fseek(fp, 0L, SEEK_END);
    int len = ftell(fp);

    fclose(fp);
    return len;
}

/* get local firmware offset for resuming download from break point */
static int _update_resource_downloaded_size(ResourceContextData *resource_ctx)
{
    int local_size = _get_local_resource_info(resource_ctx->resource_info_file_path);
    if (local_size == 0) {
        resource_ctx->resource_downloaded_size = 0;
        return 0;
    }

    if ((resource_ctx->resource_downloaded_size > resource_ctx->resource_size)) {
        resource_ctx->resource_downloaded_size = 0;
        return 0;
    }

    resource_ctx->resource_downloaded_size = local_size;
    Log_i("calc MD5 for resuming download from offset: %d", resource_ctx->resource_downloaded_size);
    int ret = _cal_exist_resource_md5(resource_ctx);
    if (ret) {
        Log_e("regen resource MD5 error: %d", ret);
        remove(resource_ctx->resource_info_file_path);
        resource_ctx->resource_downloaded_size = 0;
        return 0;
    }
    Log_d("local MD5 update done!");
    return local_size;
}

static int _delete_resource_info_file(char *file_name)
{
    return remove(file_name);
}

static int _save_resource_data_to_file(char *file_name, uint32_t offset, char *buf, int len)
{
    FILE *fp;
    if (offset > 0) {
        if (NULL == (fp = fopen(file_name, "ab+"))) {
            Log_e("open file failed");
            return QCLOUD_ERR_FAILURE;
        }
    } else {
        if (NULL == (fp = fopen(file_name, "wb+"))) {
            Log_e("open file failed");
            return QCLOUD_ERR_FAILURE;
        }
    }

    fseek(fp, offset, SEEK_SET);

    if (1 != fwrite(buf, len, 1, fp)) {
        Log_e("write data to file failed");
        fclose(fp);
        return QCLOUD_ERR_FAILURE;
    }
    fflush(fp);
    fclose(fp);

    return 0;
}

/**********************************************************************************
 * resource file operations END
 **********************************************************************************/

// main resource download cycle
bool process_resource_download(void *ctx)
{
    bool                 download_finished      = false;
    bool                 download_fetch_success = true;
    char                 buf_download[RESOURCE_BUF_LEN];
    int                  rc;
    ResourceContextData *resource_ctx    = (ResourceContextData *)ctx;
    void *               resource_handle = resource_ctx->resource_handle;
    int                  packetid;

    packetid = QCLOUD_IOT_RESOURCE_GetDownloadTask(resource_handle);
    _wait_for_download_pub_ack(resource_ctx, packetid);

    do {
        IOT_MQTT_Yield(resource_ctx->mqtt_client, 200);

        Log_i("wait for resource download command...");

        // recv the upgrade cmd
        if (QCLOUD_IOT_RESOURCE_IsStartDownload(resource_handle)) {
            QCLOUD_IOT_RESOURCE_DownloadIoctl(resource_handle, QCLOUD_IOT_RESOURCE_SIZE_E, &resource_ctx->resource_size,
                                              4);
            QCLOUD_IOT_RESOURCE_DownloadIoctl(resource_handle, QCLOUD_IOT_RESOURCE_NAME_E, resource_ctx->resource_name,
                                              RESOURCE_NAME_MAX_LEN);

            DeviceInfo *device_info = IOT_MQTT_GetDeviceInfo(resource_ctx->mqtt_client);

            HAL_Snprintf(resource_ctx->resource_file_path, RESOURCE_PATH_MAX_LEN, "./download_%s_%s",
                         STRING_PTR_PRINT_SANITY_CHECK(device_info->client_id),
                         STRING_PTR_PRINT_SANITY_CHECK(resource_ctx->resource_name));
            HAL_Snprintf(resource_ctx->resource_info_file_path, RESOURCE_PATH_MAX_LEN, "./download_%s.json",
                         STRING_PTR_PRINT_SANITY_CHECK(device_info->client_id));

            /* check if pre-downloading finished or not */
            /* if local resource downloaded size (resource_ctx->downloaded_size) is not zero, it will do resuming
             * download */
            _update_resource_downloaded_size(resource_ctx);

            /*set offset and start http connect*/
            rc = QCLOUD_IOT_RESOURCE_StartDownload(resource_handle, resource_ctx->resource_downloaded_size,
                                                   resource_ctx->resource_size);
            if (QCLOUD_RET_SUCCESS != rc) {
                Log_e("resource download start err,rc:%d", rc);
                download_fetch_success = false;
                break;
            }

            // download and save the fw
            do {
                int len = QCLOUD_IOT_RESOURCE_DownloadYield(resource_handle, buf_download, RESOURCE_BUF_LEN, 1);
                if (len > 0) {
                    rc = _save_resource_data_to_file(resource_ctx->resource_file_path,
                                                     resource_ctx->resource_downloaded_size, buf_download, len);
                    if (rc) {
                        Log_e("write data to file failed");
                        download_fetch_success = false;
                        break;
                    }
                } else if (len < 0) {
                    Log_e("download fail rc=%d", len);
                    download_fetch_success = false;
                    break;
                }

                /* get resource information and update local info */
                QCLOUD_IOT_RESOURCE_DownloadIoctl(resource_handle, QCLOUD_IOT_RESOURCE_FETCHED_SIZE_E,
                                                  &resource_ctx->resource_downloaded_size, 4);
                rc = _update_local_resource_info(resource_ctx);
                if (QCLOUD_RET_SUCCESS != rc) {
                    Log_e("update local resource info err,rc:%d", rc);
                }

                // quit resource download process as something wrong with mqtt
                rc = IOT_MQTT_Yield(resource_ctx->mqtt_client, 100);
                if (rc != QCLOUD_RET_SUCCESS && rc != QCLOUD_RET_MQTT_RECONNECTED) {
                    Log_e("MQTT error: %d", rc);
                    return false;
                }

            } while (!QCLOUD_IOT_RESOURCE_IsDownloadFinish(resource_handle));

            /* Must check MD5 match or not */
            if (download_fetch_success) {
                // download is finished, delete the fw info file
                _delete_resource_info_file(resource_ctx->resource_info_file_path);

                uint32_t resource_valid;
                QCLOUD_IOT_RESOURCE_DownloadIoctl(resource_handle, QCLOUD_IOT_RESOURCE_MD5CHECK_E, &resource_valid, 4);
                if (0 == resource_valid) {
                    Log_e("The resource is invalid");
                    download_fetch_success = false;
                } else {
                    Log_i("The resource is valid");
                    download_fetch_success = true;
                }
            }
            download_finished = true;
        }

        if (!download_finished)
            HAL_SleepMs(1000);

    } while (!download_finished);

    // do some post-download stuff for your need

    // report result
    if (download_fetch_success) {
        packetid = QCLOUD_IOT_RESOURCE_ReportDownloadSuccess(resource_handle, resource_ctx->resource_name);
    } else {
        packetid = QCLOUD_IOT_RESOURCE_ReportDownloadFail(resource_handle, resource_ctx->resource_name);
    }
    _wait_for_download_pub_ack(resource_ctx, packetid);

    return download_fetch_success;
}

bool process_resource_upload(void *ctx)
{
    bool                 upload_finished         = false;
    bool                 upload_resource_success = true;
    char                 buf_upload[RESOURCE_BUF_LEN];
    int                  rc;
    ResourceContextData *resource_ctx    = (ResourceContextData *)ctx;
    void *               resource_handle = resource_ctx->resource_handle;
    int                  resource_size;
    int                  offset               = 0;
    int                  readlen              = 0;
    char *               upload_resource_name = "uploadtest";
    char                 resource_md5[33]     = {0};
    int                  packet_id;

    resource_size = _get_local_resource_size(upload_resource_name);
    if (resource_size <= 0) {
        Log_e("file size is zero exit upload %s", upload_resource_name);
        return false;
    }
    /* md5 calc */
    offset  = 0;
    readlen = 0;
    QCLOUD_IOT_RESOURCE_UploadResetClientMD5(resource_handle);
    while (offset < resource_size) {
        readlen = RESOURCE_BUF_LEN > (resource_size - offset) ? (resource_size - offset) : RESOURCE_BUF_LEN;
        rc      = _read_local_resource_data(upload_resource_name, offset, buf_upload, readlen);
        if (rc) {
            Log_e("read data from file failed");
            upload_resource_success = false;
            break;
        }
        QCLOUD_IOT_RESOURCE_UploadMd5Update(resource_handle, buf_upload, readlen);
        offset += readlen;
    }

    QCLOUD_IOT_RESOURCE_Upload_Md5_Finish(resource_handle, resource_md5);
    offset  = 0;
    readlen = 0;

    /* request upload resource qos1 */
    packet_id = QCLOUD_IOT_RESOURCE_Upload_Request(resource_handle, upload_resource_name, resource_size, resource_md5);
    _wait_for_upload_pub_ack(resource_ctx, packet_id);

    do {
        IOT_MQTT_Yield(resource_ctx->mqtt_client, 200);

        Log_i("wait for resource upload command...");

        // recv the upload cmd
        if (QCLOUD_IOT_RESOURCE_IsStartUpload(resource_handle)) {
            /* start http connect ,send http header */
            rc = QCLOUD_IOT_RESOURCE_StartUpload(resource_handle, buf_upload);
            if (QCLOUD_RET_SUCCESS != rc) {
                Log_e("resource upload start err,rc:%d", rc);
                upload_resource_success = false;
                break;
            }

            // upload and read the resource
            readlen = RESOURCE_BUF_LEN > (resource_size - offset) ? (resource_size - offset) : RESOURCE_BUF_LEN;
            rc      = _read_local_resource_data(upload_resource_name, offset, buf_upload, readlen);
            if (rc) {
                Log_e("read data from file failed");
                upload_resource_success = false;
                break;
            }
            do {
                /* send resource data */
                int len = QCLOUD_IOT_RESOURCE_UploadYield(resource_handle, buf_upload, readlen, 5);
                if (len > 0) {
                    offset += len;
                    readlen = RESOURCE_BUF_LEN > (resource_size - offset) ? (resource_size - offset) : RESOURCE_BUF_LEN;
                    if (readlen != 0) {
                        rc = _read_local_resource_data(upload_resource_name, offset, buf_upload, readlen);
                        if (rc) {
                            Log_e("read data from file failed");
                            upload_resource_success = false;
                            break;
                        }
                    }
                } else if (len < 0) {
                    Log_e("upload fail rc=%d", len);
                    upload_resource_success = false;
                    break;
                }

                // quit resource upload process as something wrong with mqtt
                rc = IOT_MQTT_Yield(resource_ctx->mqtt_client, 100);
                if (rc != QCLOUD_RET_SUCCESS && rc != QCLOUD_RET_MQTT_RECONNECTED) {
                    Log_e("MQTT error: %d", rc);
                    upload_resource_success = false;
                    break;
                }

            } while (!QCLOUD_IOT_RESOURCE_IsUploadFinish(resource_handle));

            /* Must check MD5 match or not */
            if (upload_resource_success) {
                uint32_t resource_valid;
                QCLOUD_IOT_RESOURCE_UploadIoctl(resource_handle, QCLOUD_IOT_RESOURCE_MD5CHECK_E, &resource_valid, 4);
                if (0 == resource_valid) {
                    Log_e("The upload resource is invalid");
                    upload_resource_success = false;
                } else {
                    Log_i("The upload resource is valid");
                    upload_resource_success = true;
                }
            }
            upload_finished = true;
        }

        if (!upload_finished)
            HAL_SleepMs(1000);

    } while (!upload_finished);

    // report upload result
    if (upload_resource_success) {
        rc = QCLOUD_IOT_RESOURCE_ReportUploadSuccess(resource_handle, upload_resource_name);
        if (rc != QCLOUD_RET_SUCCESS) {
            Log_e("upload failed %d", rc);
            upload_resource_success = false;
        }
    } else {
        rc = QCLOUD_IOT_RESOURCE_ReportUploadFail(resource_handle, upload_resource_name);
        if (rc != QCLOUD_RET_SUCCESS) {
            Log_e("upload failed %d", rc);
        }
    }

    return upload_resource_success;
}

#ifdef MULTITHREAD_ENABLED
void resource_download_thread(void *ctx)
{
    ResourceContextData *resource_ctx = (ResourceContextData *)ctx;
    resource_ctx->download_thread     = true;
    while (false == process_resource_download(ctx)) {
        HAL_SleepMs(10000);
        Log_e("retry resource download");
    }
    Log_i("resource download success");
    resource_ctx->download_thread = false;
}

void resource_upload_thread(void *ctx)
{
    ResourceContextData *resource_ctx = (ResourceContextData *)ctx;
    resource_ctx->upload_thread       = true;
    while (false == process_resource_upload(ctx)) {
        HAL_SleepMs(10000);
        Log_e("retry resource upload");
    }
    Log_i("resource upload success");
    resource_ctx->upload_thread = false;
}
#endif

int main(int argc, char **argv)
{
    int                  rc;
    ResourceContextData *resource_ctx    = NULL;
    void *               mqtt_client     = NULL;
    void *               resource_handle = NULL;

    Log_e("enter :%s-%s", __DATE__, __TIME__);
    IOT_Log_Set_Level(eLOG_DEBUG);
    resource_ctx = (ResourceContextData *)HAL_Malloc(sizeof(ResourceContextData));
    if (resource_ctx == NULL) {
        Log_e("malloc failed");
        goto exit;
    }
    memset(resource_ctx, 0, sizeof(ResourceContextData));

    DeviceInfo device_info = {0};
    rc                     = HAL_GetDevInfo((void *)&device_info);
    if (QCLOUD_RET_SUCCESS != rc) {
        Log_e("get device info failed: %d", rc);
        goto exit;
    }

    // setup MQTT init params
    MQTTInitParams init_params = DEFAULT_MQTTINIT_PARAMS;
    rc                         = _setup_connect_init_params(&init_params, resource_ctx, &device_info);
    if (rc != QCLOUD_RET_SUCCESS) {
        Log_e("init params err,rc=%d", rc);
        return rc;
    }

    // create MQTT mqtt_client and connect to server
    mqtt_client = IOT_MQTT_Construct(&init_params);
    if (mqtt_client != NULL) {
        Log_i("Cloud Device Construct Success");
    } else {
        Log_e("Cloud Device Construct Failed");
        return QCLOUD_ERR_FAILURE;
    }

    // init resource handle
    resource_handle = QCLOUD_IOT_RESOURCE_Init(device_info.product_id, device_info.device_name, mqtt_client);
    if (NULL == resource_handle) {
        Log_e("initialize resource handle failed");
        goto exit;
    }

    resource_ctx->resource_handle = resource_handle;
    resource_ctx->mqtt_client     = mqtt_client;

#ifdef MULTITHREAD_ENABLED
    ThreadParams thread_params = {0};
    thread_params.thread_func  = resource_download_thread;
    thread_params.user_arg     = resource_ctx;
    thread_params.thread_name  = "download_resource";

    rc = HAL_ThreadCreate(&thread_params);
    if (rc) {
        Log_e("create resource download thread fail: %d", rc);
        goto exit;
    }

    HAL_SleepMs(1000);

    memset(&thread_params, 0, sizeof(thread_params));
    thread_params.thread_func = resource_upload_thread;
    thread_params.user_arg    = resource_ctx;
    thread_params.thread_name = "upload_resource";

    rc = HAL_ThreadCreate(&thread_params);
    if (rc) {
        Log_e("create resource upload thread fail: %d", rc);
    }

    HAL_SleepMs(1000);

    IOT_MQTT_StartLoop(resource_ctx->mqtt_client);
    while ((resource_ctx->download_thread == true) || (resource_ctx->upload_thread == true)) {
        HAL_SleepMs(10000);
    }
    IOT_MQTT_StopLoop(resource_ctx->mqtt_client);
#else
    int download_success      = false;
    int upload_success        = false;
    do {
        // mqtt should be ready first
        rc = IOT_MQTT_Yield(mqtt_client, 500);
        if (rc == QCLOUD_ERR_MQTT_ATTEMPTING_RECONNECT) {
            HAL_SleepMs(1000);
            continue;
        } else if (rc != QCLOUD_RET_SUCCESS && rc != QCLOUD_RET_MQTT_RECONNECTED) {
            Log_e("exit with error: %d", rc);
            break;
        }

        Log_e("resource upload enter");
        if (download_success == false) {
            upload_success = process_resource_upload(resource_ctx);
        }

        HAL_SleepMs(5000);

        Log_e("resource download enter");

        if (download_success == false) {
            download_success = process_resource_download(resource_ctx);
        }

        HAL_SleepMs(5000);
    } while ((download_success == false) || (upload_success == false));
#endif

exit:
    HAL_Free(resource_ctx);

    if (NULL != resource_handle) {
        QCLOUD_IOT_RESOURCE_DeInit(resource_handle);
    }

    IOT_MQTT_Destroy(&mqtt_client);

    return 0;
}
