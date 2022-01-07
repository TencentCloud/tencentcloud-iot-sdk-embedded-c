/*
 * Tencent is pleased to support the open source community by making IoT Hub
 available.
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file
 except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software
 distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 KIND,
 * either express or implied. See the License for the specific language
 governing permissions and
 * limitations under the License.
 *
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qcloud_iot_export.h"
#include "utils_getopt.h"
#include "lite-utils.h"

static DeviceInfo sg_subdev_info[] = {
    {.product_id = "E69UADXUYY", .device_name = "light1"},
    {.product_id = "E69UADXUYY", .device_name = "light2"},
    {.product_id = "E69UADXUYY", .device_name = "light3"},
    {.product_id = "YI7XCD5DRH", .device_name = "airConditioner1"},
};

#define MAX_SIZE_OF_TOPIC (128)
#define MAX_SIZE_OF_DATA  (128)

static int sg_sub_packet_id = -1;
static int sg_pub_packet_id = -1;

static int               sg_loop_count = 1;
static GatewayDeviceInfo sg_GWdevInfo;

#define FW_RUNNING_VERSION "0.1"
#define KEY_VER            "version"
#define KEY_SIZE           "downloaded_size"

#define FW_VERSION_MAX_LEN    32
#define FW_FILE_PATH_MAX_LEN  128
#define OTA_BUF_LEN           5000
#define FW_INFO_FILE_DATA_LEN 128

/* The structure of subdevice ota context */
typedef struct _SubdevOtaContext {
    void *                    ota_handle;
    char *                    product_id;
    char *                    device_name;
    struct _SubdevOtaContext *next;
} SubdevOtaContext;

typedef struct OTAContextData {
    void *ota_handle;
    void *gateway_client;
    char *product_id;
    char *device_name;
    char  fw_file_path[FW_FILE_PATH_MAX_LEN];
    char  fw_info_file_path[FW_FILE_PATH_MAX_LEN];
    // remote_version means version for the FW in the cloud and to be downloaded
    char     remote_version[FW_VERSION_MAX_LEN];
    uint32_t fw_file_size;

    // for resuming download
    /* local_version means downloading but not running */
    char local_version[FW_VERSION_MAX_LEN];
    int  downloaded_size;

    // to make sure report is acked
    bool report_pub_ack;
    int  report_packet_id;
} OTAContextData;

static SubdevOtaContext *sg_subdev_ota_context_list = NULL;

/**********************************************************************************
 * OTA file operations START
 * these are platform-dependant functions
 * POSIX FILE is used in this sample code
 **********************************************************************************/
// calculate left MD5 for resuming download from break point
static int _cal_exist_fw_md5(OTAContextData *ota_ctx)
{
    char   buff[OTA_BUF_LEN];
    size_t rlen, total_read = 0;
    int    ret = QCLOUD_RET_SUCCESS;

    ret = IOT_OTA_ResetClientMD5(ota_ctx->ota_handle);
    if (ret) {
        Log_e("reset MD5 failed: %d", ret);
        return QCLOUD_ERR_FAILURE;
    }

    FILE *fp = fopen(ota_ctx->fw_file_path, "ab+");
    if (NULL == fp) {
        Log_e("open file %s failed", STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->fw_file_path));
        return QCLOUD_ERR_FAILURE;
    }

    // rewind(fp);
    size_t size = ota_ctx->downloaded_size;

    while ((size > 0) && (!feof(fp))) {
        rlen = (size > OTA_BUF_LEN) ? OTA_BUF_LEN : size;
        if (rlen != fread(buff, 1, rlen, fp)) {
            Log_e("read data len not expected");
            ret = QCLOUD_ERR_FAILURE;
            break;
        }
        IOT_OTA_UpdateClientMd5(ota_ctx->ota_handle, buff, rlen);
        size -= rlen;
        total_read += rlen;
    }

    fclose(fp);
    Log_d("total read: %d", total_read);
    return ret;
}

/* update local firmware info for resuming download from break point */
static int _update_local_fw_info(OTAContextData *ota_ctx)
{
    FILE *fp;
    int   wlen;
    int   ret = QCLOUD_RET_SUCCESS;
    char  data_buf[FW_INFO_FILE_DATA_LEN];

    memset(data_buf, 0, sizeof(data_buf));
    HAL_Snprintf(data_buf, sizeof(data_buf), "{\"%s\":\"%s\", \"%s\":%d}", KEY_VER,
                 STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->remote_version), KEY_SIZE, ota_ctx->downloaded_size);

    fp = fopen(ota_ctx->fw_info_file_path, "w");
    if (NULL == fp) {
        Log_e("open file %s failed", STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->fw_info_file_path));
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

static int _get_local_fw_info(char *file_name, char *local_version)
{
    int  len = 0;
    int  rlen;
    char json_doc[FW_INFO_FILE_DATA_LEN] = {0};

    FILE *fp = fopen(file_name, "r");
    if (NULL == fp) {
        Log_e("open file %s failed", STRING_PTR_PRINT_SANITY_CHECK(file_name));
        return 0;
    }

    fseek(fp, 0L, SEEK_END);
    len = ftell(fp);
    if ((len < 0) || (len > FW_INFO_FILE_DATA_LEN)) {
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

    char *version = LITE_json_value_of(KEY_VER, json_doc);
    char *size    = LITE_json_value_of(KEY_SIZE, json_doc);

    if ((NULL == version) || (NULL == size)) {
        if (version)
            HAL_Free(version);
        if (size)
            HAL_Free(size);
        fclose(fp);
        return 0;
    }

    int local_size = atoi(size);
    HAL_Free(size);

    if (local_size <= 0) {
        Log_w("local info offset invalid: %d", local_size);
        HAL_Free(version);
        local_size = 0;
        return local_size;
    }

    strncpy(local_version, version, FW_VERSION_MAX_LEN);
    HAL_Free(version);
    fclose(fp);
    return local_size;
}

/* get local firmware offset for resuming download from break point */
static int _update_fw_downloaded_size(OTAContextData *ota_ctx)
{
    int local_size = _get_local_fw_info(ota_ctx->fw_info_file_path, ota_ctx->local_version);
    if (local_size == 0) {
        ota_ctx->downloaded_size = 0;
        return 0;
    }

    if ((0 != strcmp(ota_ctx->local_version, ota_ctx->remote_version)) ||
        (ota_ctx->downloaded_size > ota_ctx->fw_file_size)) {
        ota_ctx->downloaded_size = 0;
        return 0;
    }

    ota_ctx->downloaded_size = local_size;
    Log_i("calc MD5 for resuming download from offset: %d", ota_ctx->downloaded_size);
    int ret = _cal_exist_fw_md5(ota_ctx);
    if (ret) {
        Log_e("regen OTA MD5 error: %d", ret);
        remove(ota_ctx->fw_info_file_path);
        ota_ctx->downloaded_size = 0;
        return 0;
    }
    Log_d("local MD5 update done!");
    return local_size;
}

static int _delete_fw_info_file(char *file_name)
{
    return remove(file_name);
}

static int _save_fw_data_to_file(char *file_name, uint32_t offset, char *buf, int len)
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
 * OTA file operations END
 **********************************************************************************/

/*****************************************************************************************************************/
static void _wait_for_pub_ack(OTAContextData *ota_ctx, int packet_id)
{
    int wait_cnt              = 10;
    ota_ctx->report_pub_ack   = false;
    ota_ctx->report_packet_id = packet_id;

    while (sg_pub_packet_id != packet_id) {
        HAL_SleepMs(500);
        IOT_Gateway_Yield(ota_ctx->gateway_client, 500);
        if (wait_cnt-- == 0) {
            Log_e("wait report pub ack timeout!");
            break;
        }
    }
    ota_ctx->report_pub_ack = false;
    return;
}

static SubdevOtaContext *_subdev_ota_context_create(void *gateway_client, char *subdev_product_id,
                                                    char *subdev_device_name)
{
    SubdevOtaContext *ota_context = NULL;
    char *            product_id  = NULL;
    char *            device_name = NULL;

    ota_context = sg_subdev_ota_context_list;
    /* ota_handle is exist */
    while (ota_context) {
        product_id  = ota_context->product_id;
        device_name = ota_context->device_name;

        if (0 == strcmp(product_id, subdev_product_id) && (0 == strcmp(device_name, subdev_device_name))) {
            return ota_context;
        }
        ota_context = ota_context->next;
    }

    ota_context = HAL_Malloc(sizeof(SubdevOtaContext));
    if (ota_context == NULL) {
        Log_e("Not enough memory");
        return NULL;
    }

    memset(ota_context, 0, sizeof(SubdevOtaContext));

    ota_context->ota_handle =
        IOT_OTA_Init(subdev_product_id, subdev_device_name, IOT_Gateway_Get_Mqtt_Client(gateway_client));
    if (NULL == ota_context->ota_handle) {
        Log_e("subDev %s-%s initialize OTA failed", STRING_PTR_PRINT_SANITY_CHECK(subdev_product_id),
              STRING_PTR_PRINT_SANITY_CHECK(subdev_device_name));
        HAL_Free(ota_context);
        return NULL;
    }

    ota_context->product_id  = subdev_product_id;
    ota_context->device_name = subdev_device_name;

    /* add ota handle to list */
    ota_context->next          = sg_subdev_ota_context_list;
    sg_subdev_ota_context_list = ota_context;

    return ota_context;
}

static SubdevOtaContext *_subdev_ota_context_get_fetch()
{
    SubdevOtaContext *ota_context = sg_subdev_ota_context_list;
    while (NULL != ota_context) {
        if (IOT_OTA_IsFetching(ota_context->ota_handle)) {
            break;
        } else {
            ota_context = ota_context->next;
        }
    }

    return ota_context;
}

static int _subdev_ota_context_destory(char *subdev_product_id, char *subdev_device_name)
{
    SubdevOtaContext *cur_ota_context = NULL;
    SubdevOtaContext *pre_ota_context = NULL;
    char *            product_id      = NULL;
    char *            device_name     = NULL;

    cur_ota_context = pre_ota_context = sg_subdev_ota_context_list;

    if (NULL == cur_ota_context) {
        Log_e("ota_handle list is empty");
        return QCLOUD_RET_SUCCESS;
    }

    /* ota_handle is exist */
    while (cur_ota_context) {
        product_id  = cur_ota_context->product_id;
        device_name = cur_ota_context->device_name;

        if (0 == strcmp(product_id, subdev_product_id) && (0 == strcmp(device_name, subdev_device_name))) {
            if (cur_ota_context == sg_subdev_ota_context_list) {
                sg_subdev_ota_context_list = cur_ota_context->next;
            } else {
                pre_ota_context->next = cur_ota_context->next;
            }

            IOT_OTA_Destroy(cur_ota_context->ota_handle);
            HAL_Free(cur_ota_context);
            return QCLOUD_RET_SUCCESS;
        }
        pre_ota_context = cur_ota_context;
        cur_ota_context = cur_ota_context->next;
    }

    return QCLOUD_ERR_FAILURE;
}

static char *_get_subdev_fw_running_version(char *product_id, char *device_name)
{
    Log_i("%s-%s FW running version: %s", STRING_PTR_PRINT_SANITY_CHECK(product_id),
          STRING_PTR_PRINT_SANITY_CHECK(device_name), FW_RUNNING_VERSION);
    return FW_RUNNING_VERSION;
}

static bool _subdev_process_upgrade(OTAContextData *ota_ctx)
{
    Log_i("%s-%s download success, fw name:%s, fw size:%d, fw version:%s, now to upgrade subdevice ...",
          STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->product_id), STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->device_name),
          STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->fw_file_path), ota_ctx->fw_file_size,
          STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->remote_version));

    return true;
}

// main OTA cycle
bool process_ota(OTAContextData *ota_ctx)
{
    bool  download_finished     = false;
    bool  upgrade_fetch_success = true;
    char  buf_ota[OTA_BUF_LEN];
    int   rc;
    void *h_ota = ota_ctx->ota_handle;

    do {
        // recv the upgrade cmd
        if (IOT_OTA_IsFetching(h_ota)) {
            IOT_OTA_Ioctl(h_ota, IOT_OTAG_FILE_SIZE, &ota_ctx->fw_file_size, 4);
            IOT_OTA_Ioctl(h_ota, IOT_OTAG_VERSION, ota_ctx->remote_version, FW_VERSION_MAX_LEN);

            HAL_Snprintf(ota_ctx->fw_file_path, FW_FILE_PATH_MAX_LEN, "./FW_%s%s_%s.bin",
                         STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->product_id),
                         STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->device_name),
                         STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->remote_version));
            HAL_Snprintf(ota_ctx->fw_info_file_path, FW_FILE_PATH_MAX_LEN, "./FW_%s%s.json",
                         STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->product_id),
                         STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->device_name));

            /* check if pre-downloading finished or not */
            /* if local FW downloaded size (ota_ctx->downloaded_size) is not zero, it will do resuming download */
            _update_fw_downloaded_size(ota_ctx);

            /*set offset and start http connect*/
            rc = IOT_OTA_StartDownload(h_ota, ota_ctx->downloaded_size, ota_ctx->fw_file_size);
            if (QCLOUD_RET_SUCCESS != rc) {
                Log_e("OTA download start err,rc:%d", rc);
                upgrade_fetch_success = false;
                break;
            }

            // download and save the fw
            do {
                int len = IOT_OTA_FetchYield(h_ota, buf_ota, OTA_BUF_LEN, 1);
                if (len > 0) {
                    rc = _save_fw_data_to_file(ota_ctx->fw_file_path, ota_ctx->downloaded_size, buf_ota, len);
                    if (rc) {
                        Log_e("write data to file failed");
                        upgrade_fetch_success = false;
                        break;
                    }
                } else if (len < 0) {
                    Log_e("download fail rc=%d", len);
                    upgrade_fetch_success = false;
                    break;
                }

                /* get OTA information and update local info */
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_FETCHED_SIZE, &ota_ctx->downloaded_size, 4);
                rc = _update_local_fw_info(ota_ctx);
                if (QCLOUD_RET_SUCCESS != rc) {
                    Log_e("update local fw info err,rc:%d", rc);
                }

                // quit ota process as something wrong with mqtt
                rc = IOT_Gateway_Yield(ota_ctx->gateway_client, 100);
                if (rc != QCLOUD_RET_SUCCESS && rc != QCLOUD_RET_MQTT_RECONNECTED) {
                    Log_e("MQTT error: %d", rc);
                    return false;
                }

            } while (!IOT_OTA_IsFetchFinish(h_ota));

            /* Must check MD5 match or not */
            if (upgrade_fetch_success) {
                // download is finished, delete the fw info file
                _delete_fw_info_file(ota_ctx->fw_info_file_path);

                uint32_t firmware_valid;
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_CHECK_FIRMWARE, &firmware_valid, 4);
                if (0 == firmware_valid) {
                    Log_e("The firmware is invalid");
                    upgrade_fetch_success = false;
                } else {
                    Log_i("The firmware is valid");
                    upgrade_fetch_success = true;
                }
            }

            download_finished = true;
        }

        if (!download_finished)
            HAL_SleepMs(1000);

    } while (!download_finished);

    if (upgrade_fetch_success) {
        IOT_OTA_ReportUpgradeBegin(h_ota);

        // do some post-download stuff for your need
        upgrade_fetch_success = _subdev_process_upgrade(ota_ctx);
    }

    // report result
    int packet_id;
    if (upgrade_fetch_success) {
        Log_i("%s-%s upgrade success", STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->product_id),
              STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->device_name));
        packet_id = IOT_OTA_ReportUpgradeSuccess(h_ota, NULL);
    } else {
        Log_i("%s-%s upgrade failed", STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->product_id),
              STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->device_name));
        packet_id = IOT_OTA_ReportUpgradeFail(h_ota, NULL);
    }
    _wait_for_pub_ack(ota_ctx, packet_id);

    return upgrade_fetch_success;
}

/*****************************************************************************************************************/

void _event_handler(void *client, void *context, MQTTEventMsg *msg)
{
    MQTTMessage *mqtt_message = (MQTTMessage *)msg->msg;
    uintptr_t    packet_id    = (uintptr_t)msg->msg;

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

        case MQTT_EVENT_PUBLISH_RECVEIVED:
            Log_i(
                "topic message arrived but without any related handle: topic=%.*s, "
                "topic_msg=%.*s",
                mqtt_message->topic_len, STRING_PTR_PRINT_SANITY_CHECK(mqtt_message->ptopic), mqtt_message->payload_len,
                STRING_PTR_PRINT_SANITY_CHECK(mqtt_message->payload));
            break;
        case MQTT_EVENT_SUBCRIBE_SUCCESS:
            Log_i("subscribe success, packet-id=%u", (unsigned int)packet_id);
            sg_sub_packet_id = packet_id;
            break;

        case MQTT_EVENT_SUBCRIBE_TIMEOUT:
            Log_i("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            sg_sub_packet_id = packet_id;
            break;

        case MQTT_EVENT_SUBCRIBE_NACK:
            Log_i("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            sg_sub_packet_id = packet_id;
            break;

        case MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            Log_i("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            Log_i("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_UNSUBCRIBE_NACK:
            Log_i("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_PUBLISH_SUCCESS:
            Log_i("publish success, packet-id=%u", (unsigned int)packet_id);
            sg_pub_packet_id = packet_id;
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

static int _setup_gw_init_params(GatewayInitParam *gw_init_params, GatewayDeviceInfo *gw_dev_info)
{
    MQTTInitParams *init_params = &gw_init_params->init_param;
    DeviceInfo *    dev_info    = &gw_dev_info->gw_info;
    init_params->region         = dev_info->region;
    init_params->product_id     = dev_info->product_id;
    init_params->device_name    = dev_info->device_name;

#ifdef AUTH_MODE_CERT
    char  certs_dir[16] = "certs";
    char  current_path[128];
    char *cwd = getcwd(current_path, sizeof(current_path));

    if (cwd == NULL) {
        Log_e("getcwd return NULL");
        return QCLOUD_ERR_FAILURE;
    }

#ifdef WIN32
    HAL_Snprintf(init_params->cert_file, FILE_PATH_MAX_LEN, "%s\\%s\\%s", current_path, certs_dir,
                 STRING_PTR_PRINT_SANITY_CHECK(dev_info->dev_cert_file_name));
    HAL_Snprintf(init_params->key_file, FILE_PATH_MAX_LEN, "%s\\%s\\%s", current_path, certs_dir,
                 STRING_PTR_PRINT_SANITY_CHECK(dev_info->dev_key_file_name));
#else
    HAL_Snprintf(init_params->cert_file, FILE_PATH_MAX_LEN, "%s/%s/%s", current_path, certs_dir,
                 STRING_PTR_PRINT_SANITY_CHECK(dev_info->dev_cert_file_name));
    HAL_Snprintf(init_params->key_file, FILE_PATH_MAX_LEN, "%s/%s/%s", current_path, certs_dir,
                 STRING_PTR_PRINT_SANITY_CHECK(dev_info->dev_key_file_name));
#endif

#else
    init_params->device_secret = dev_info->device_secret;
#endif

    init_params->command_timeout        = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
    init_params->keep_alive_interval_ms = QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;

    init_params->auto_connect_enable  = 1;
    init_params->event_handle.h_fp    = _event_handler;
    init_params->event_handle.context = NULL;

    return QCLOUD_RET_SUCCESS;
}

static int parse_arguments(int argc, char **argv)
{
    int c;
    while ((c = utils_getopt(argc, argv, "c:l:")) != EOF) switch (c) {
            case 'c':
                if (HAL_SetDevInfoFile(utils_optarg))
                    return -1;
                break;

            case 'l':
                sg_loop_count = atoi(utils_optarg);
                if (sg_loop_count > 10000)
                    sg_loop_count = 10000;
                else if (sg_loop_count < 0)
                    sg_loop_count = 1;
                break;

            default:
                HAL_Printf(
                    "usage: %s [options]\n"
                    "  [-c <config file for DeviceInfo>] \n"
                    "  [-l <loop count>] \n",
                    argv[0]);
                return -1;
        }
    return 0;
}

/**
 * show gateway dynamic bind/unbind sub-devices
 */
#ifdef GATEWAY_DYN_BIND_SUBDEV_ENABLED
static int add_new_binded_sub_dev(GatewayDeviceInfo *pGateway, DeviceInfo *pNewSubDev)
{
    int ret;
    if (pGateway->sub_dev_num < MAX_NUM_SUB_DEV) {
        memcpy((char *)&pGateway->sub_dev_info[pGateway->sub_dev_num], (char *)pNewSubDev, sizeof(DeviceInfo));
        pGateway->sub_dev_num++;
        ret = QCLOUD_RET_SUCCESS;  // you can save gateway info to local flash for persistent storage
    } else {
        ret = QCLOUD_ERR_FAILURE;
    }

    return ret;
}

static int show_subdev_bind_unbind(void *client, GatewayParam *param)
{
    int rc;

    // adjust for your bind device info
    DeviceInfo subDev;
    memset((char *)&subDev, 0, sizeof(DeviceInfo));
    strncpy(subDev.product_id, "BIND_PID", MAX_SIZE_OF_PRODUCT_ID);
    strncpy(subDev.device_name, "BIND_DEV_NAME", MAX_SIZE_OF_DEVICE_NAME);
#ifdef AUTH_MODE_CERT
    strncpy(subDev.dev_cert_file_name, "BIND_CERT_FILE_NAME", MAX_SIZE_OF_DEVICE_CERT_FILE_NAME);
    strncpy(subDev.dev_key_file_name, "BIND_KEY_FILE_NAME", MAX_SIZE_OF_DEVICE_SECRET_FILE_NAME);

#else
    strncpy(subDev.device_secret, "BIND_DEV_PSK", MAX_SIZE_OF_DEVICE_SECRET);
#endif
    Log_d("bind subdev %s/%s", subDev.product_id, subDev.device_name);

    // bind sub dev
    rc = IOT_Gateway_Subdev_Bind(client, param, &subDev);
    if (QCLOUD_ERR_BIND_REPEATED_REQ == rc) {
        Log_d("%s/%s has been binded", subDev.product_id, subDev.device_name);
        rc = IOT_Gateway_Subdev_Unbind(client, param, &subDev);
        if (QCLOUD_RET_SUCCESS != rc) {
            Log_e("unbind %s/%s fail,rc:%d", subDev.product_id, subDev.device_name, rc);
        } else {
            Log_d("unbind %s/%s success", subDev.product_id, subDev.device_name);
            rc = IOT_Gateway_Subdev_Bind(client, param, &subDev);
        }
    }

    if (QCLOUD_RET_SUCCESS == rc) {
        Log_d("bind %s/%s success", subDev.product_id, subDev.device_name);
        add_new_binded_sub_dev(&sg_GWdevInfo, &subDev);
    } else {
        Log_e("bind %s/%s fail,rc:%d", subDev.product_id, subDev.device_name, rc);
    }

    return rc;
}

#endif

/*Gateway should enable multithread*/
int main(int argc, char **argv)
{
    int                rc       = QCLOUD_ERR_FAILURE;
    int                errCount = 0;
    int                i;
    void *             client = NULL;
    GatewayDeviceInfo *gw     = &sg_GWdevInfo;
    GatewayParam       param  = DEFAULT_GATEWAY_PARAMS;
    DeviceInfo *       subDevInfo;
    OTAContextData *   ota_ctx = NULL;
    bool               ota_success;
    SubdevOtaContext * ota_context = NULL;

    IOT_Log_Set_Level(eLOG_DEBUG);
    // parse arguments for device info file and loop test;
    rc = parse_arguments(argc, argv);
    if (rc != QCLOUD_RET_SUCCESS) {
        Log_e("parse arguments error, rc = %d", rc);
        return rc;
    }

    ota_ctx = (OTAContextData *)HAL_Malloc(sizeof(OTAContextData));
    if (ota_ctx == NULL) {
        Log_e("malloc failed");
        return QCLOUD_ERR_FAILURE;
    }
    memset(ota_ctx, 0, sizeof(OTAContextData));

    rc = HAL_GetGwDevInfo((void *)gw);
    if (QCLOUD_RET_SUCCESS != rc) {
        Log_e("Get gateway dev info err,rc:%d", rc);
        return rc;
    }

    GatewayInitParam init_params = DEFAULT_GATEWAY_INIT_PARAMS;
    rc                           = _setup_gw_init_params(&init_params, gw);
    if (rc != QCLOUD_RET_SUCCESS) {
        Log_e("init params err,rc=%d", rc);
        return rc;
    }

    client = IOT_Gateway_Construct(&init_params);
    if (client == NULL) {
        Log_e("client constructed failed.");
        return QCLOUD_ERR_FAILURE;
    }

#ifdef MULTITHREAD_ENABLED
    /* Start the default loop thread to read and handle MQTT packet*/
    rc = IOT_MQTT_StartLoop(IOT_Gateway_Get_Mqtt_Client(client));
    if (rc) {
        Log_e("MQTT start loop failed: %d", rc);
        rc = IOT_MQTT_Destroy(&client);
        return rc;
    }
#endif

    // set GateWay device info
    param.product_id  = gw->gw_info.product_id;
    param.device_name = gw->gw_info.device_name;

    // make sub-device online
    gw->sub_dev_info = sg_subdev_info;
    gw->sub_dev_num  = sizeof(sg_subdev_info) / sizeof(DeviceInfo);

    for (i = 0; i < gw->sub_dev_num; i++) {
        subDevInfo               = &gw->sub_dev_info[i];
        param.subdev_product_id  = subDevInfo->product_id;
        param.subdev_device_name = subDevInfo->device_name;

        rc = IOT_Gateway_Subdev_Online(client, &param);
        if (rc != QCLOUD_RET_SUCCESS) {
            Log_e("subDev Pid:%s devName:%s online fail.", subDevInfo->product_id, subDevInfo->device_name);
            errCount++;
        } else {
            Log_d("subDev Pid:%s devName:%s online success.", subDevInfo->product_id, subDevInfo->device_name);
            // ota init;
            // init OTA context
            ota_context = _subdev_ota_context_create(client, subDevInfo->product_id, subDevInfo->device_name);
            if (NULL == ota_context) {
                Log_e("subDev %s-%s initialize OTA Handle failed", subDevInfo->product_id, subDevInfo->device_name);
                errCount++;
            } else {
                Log_d("subDev Pid:%s devName:%s initialize OTA Handle success.", subDevInfo->product_id,
                      subDevInfo->device_name);
                char *curr_version = _get_subdev_fw_running_version(subDevInfo->product_id, subDevInfo->device_name);
                // online report version
                rc = IOT_OTA_ReportVersion(ota_context->ota_handle, curr_version);
                if (rc < 0) {
                    Log_e("%s-%s report OTA version failed", subDevInfo->product_id, subDevInfo->device_name);
                } else {
                    rc = QCLOUD_RET_SUCCESS;
                }
            }
        }
    }

    if (errCount > 0) {
        Log_e("%d of %d sub devices online fail", errCount, gw->sub_dev_num);
    }

    // gateway yield, loop proc ota
    while (QCLOUD_RET_SUCCESS == rc) {
        rc = IOT_Gateway_Yield(client, 200);
        if (QCLOUD_RET_SUCCESS != rc) {
            Log_d("Gateway Yield without mqtt err, rc:%d", rc);
            break;
        }
        ota_context = _subdev_ota_context_get_fetch();
        if (NULL != ota_context) {
            ota_ctx->ota_handle     = ota_context->ota_handle;
            ota_ctx->gateway_client = client;
            ota_ctx->product_id     = ota_context->product_id;
            ota_ctx->device_name    = ota_context->device_name;

            // OTA process
            ota_success = process_ota(ota_ctx);
            if (!ota_success) {
                Log_e("process ota failed, subdevice %s-%s", STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->product_id),
                      STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->device_name));
            } else {
                Log_e("process ota success subdevice %s-%s", STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->product_id),
                      STRING_PTR_PRINT_SANITY_CHECK(ota_ctx->device_name));
            }
        } else {
            Log_i("wait for ota upgrade command...");
            HAL_SleepMs(2000);
        }
    }

    // set GateWay device info
    param.product_id  = gw->gw_info.product_id;
    param.device_name = gw->gw_info.device_name;

    // make sub-device offline
    errCount = 0;
    for (i = 0; i < gw->sub_dev_num; i++) {
        subDevInfo               = &gw->sub_dev_info[i];
        param.subdev_product_id  = subDevInfo->product_id;
        param.subdev_device_name = subDevInfo->device_name;

        rc = _subdev_ota_context_destory(subDevInfo->product_id, subDevInfo->device_name);
        if (rc != QCLOUD_RET_SUCCESS) {
            Log_e("subDev %s-%s offline ota destory fail.", subDevInfo->product_id, subDevInfo->device_name);
        } else {
            Log_d("subDev %s-%s offline ota destory success.", subDevInfo->product_id, subDevInfo->device_name);
        }

        rc = IOT_Gateway_Subdev_Offline(client, &param);
        if (rc != QCLOUD_RET_SUCCESS) {
            Log_e("subDev Pid:%s devName:%s offline fail.", subDevInfo->product_id, subDevInfo->device_name);
            errCount++;
        } else {
            Log_d("subDev Pid:%s devName:%s offline success.", subDevInfo->product_id, subDevInfo->device_name);
        }
    }
    if (errCount > 0) {
        Log_e("%d of %d sub devices offline fail", errCount, gw->sub_dev_num);
    }

    rc = IOT_Gateway_Destroy(client);

    HAL_Free(ota_ctx);

    return rc;
}
