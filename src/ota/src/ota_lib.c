/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

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

#include "ota_lib.h"

#include <string.h>
#include <stdio.h>

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "qcloud_iot_utils_md5.h"
#include "qcloud_iot_utils_json.h"

#include "ota_client.h"

static const char* qcloud_otalib_json_value_of(const char *json, const char *key, uint32_t *val_len);

/* Get the specific @key value, and copy to @dest */
/* 0, successful; -1, failed */
static int _qcloud_otalib_get_firmware_fixlen_para(const char *json_doc,
                                        size_t json_doc_len,
                                        const char *key,
                                        char *dest,
                                        size_t dest_len)
{
    IOT_FUNC_ENTRY;

    const char *pvalue;
    uint32_t val_len;

    pvalue = qcloud_otalib_json_value_of(json_doc, key, &val_len);
    if (NULL == pvalue) {
        Log_e("Not '%s' key in json doc of OTA", key);
        IOT_FUNC_EXIT_RC(IOT_OTA_ERR_FAIL);
    }

    if (val_len > dest_len) {
        Log_e("value length of the key is too long");
        IOT_FUNC_EXIT_RC(IOT_OTA_ERR_FAIL);
    }

    memcpy(dest, pvalue, val_len);

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}


/* Get variant length parameter of firmware, and copy to @dest */
/* 0, successful; -1, failed */
static int _qcloud_otalib_get_firmware_varlen_para(const char *json_doc,
                                        size_t json_doc_len,
                                        const char *key,
                                        char **dest)
{
#define OTA_FIRMWARE_JSON_VALUE_MAX_LENGTH (64)

    IOT_FUNC_ENTRY;

    const char *pvalue = NULL;
    uint32_t val_len;

    pvalue = qcloud_otalib_json_value_of(json_doc, key, &val_len);
    if (NULL ==  pvalue) {
        Log_e("Not %s key in json doc of OTA", key);
        IOT_FUNC_EXIT_RC(IOT_OTA_ERR_FAIL);
    }

    if (NULL == (*dest = HAL_Malloc(val_len + 1))) {
        Log_e("allocate for dest failed");
        IOT_FUNC_EXIT_RC(IOT_OTA_ERR_FAIL);
    }

    memcpy(*dest, pvalue, val_len);
    (*dest)[val_len] = '\0';

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);

#undef OTA_FIRMWARE_JSON_VALUE_MAX_LENGTH
}


static const char* qcloud_otalib_json_value_of(const char *json, const char *key, uint32_t *val_len)
{
    const char *val = parse_firmware_value_by_name(json, key, val_len);
    if (val == NULL) {
        Log_e("Fail to parse JSON");
    }
    return val;
}

void *qcloud_otalib_md5_init(void)
{
    iot_md5_context *ctx = HAL_Malloc(sizeof(iot_md5_context));
    if (NULL == ctx) {
        return NULL;
    }

    utils_md5_init(ctx);
    utils_md5_starts(ctx);

    return ctx;
}

void qcloud_otalib_md5_update(void *md5, const char *buf, size_t buf_len)
{
    utils_md5_update(md5, (unsigned char *)buf, buf_len);
}

void qcloud_otalib_md5_finalize(void *md5, char *output_str)
{
    int i;
    unsigned char buf_out[16];
    utils_md5_finish(md5, buf_out);

    for (i = 0; i < 16; ++i) {
        output_str[i * 2] = utils_hb2hex(buf_out[i] >> 4);
        output_str[i * 2 + 1] = utils_hb2hex(buf_out[i]);
    }
    output_str[32] = '\0';
}

void qcloud_otalib_md5_deinit(void *md5)
{
    if (NULL != md5) {
        HAL_Free(md5);
    }
}

int qcloud_otalib_get_firmware_type(const char *json, uint32_t jsonLen, char **type)
{
    return _qcloud_otalib_get_firmware_varlen_para(json, jsonLen, "type", type);
}

int qcloud_otalib_get_report_version_result(const char *json, uint32_t jsonLen)
{
    IOT_FUNC_ENTRY;
    char *result_code = NULL;
    if (0 != _qcloud_otalib_get_firmware_varlen_para(json, jsonLen, "result_code", &result_code) || strcmp(result_code, "0") != 0) {
        Log_e("get value of type key failed");
        IOT_FUNC_EXIT_RC(IOT_OTA_ERR_FAIL);
    }
    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

int qcloud_otalib_get_params(const char *json, uint32_t jsonLen, char **type, char **url, char **version, char *md5,
                     uint32_t *fileSize)
{
#define OTA_FILESIZE_STR_LEN    (16)

    IOT_FUNC_ENTRY;

    char file_size_str[OTA_FILESIZE_STR_LEN + 1];

    /* get type */
    if (0 != _qcloud_otalib_get_firmware_varlen_para(json, jsonLen, "type", type)) {
        Log_e("get value of type key failed");
        IOT_FUNC_EXIT_RC(IOT_OTA_ERR_FAIL);
    }

    /* get version */
    if (0 != _qcloud_otalib_get_firmware_varlen_para(json, jsonLen, "version", version)) {
        Log_e("get value of version key failed");
        IOT_FUNC_EXIT_RC(IOT_OTA_ERR_FAIL);
    }

    /* get URL */
    if (0 != _qcloud_otalib_get_firmware_varlen_para(json, jsonLen, "url", url)) {
        Log_e("get value of url key failed");
        IOT_FUNC_EXIT_RC(IOT_OTA_ERR_FAIL);
    }

    /* get md5 */
    if (0 != _qcloud_otalib_get_firmware_fixlen_para(json, jsonLen, "md5sum", md5, 32)) {
        Log_e("get value of md5 key failed");
        IOT_FUNC_EXIT_RC(IOT_OTA_ERR_FAIL);
    }

    /* get file size */
    if (0 != _qcloud_otalib_get_firmware_fixlen_para(json, jsonLen, "file_size", file_size_str, OTA_FILESIZE_STR_LEN)) {
        Log_e("get value of size key failed");
        IOT_FUNC_EXIT_RC(IOT_OTA_ERR_FAIL);
    }
    file_size_str[OTA_FILESIZE_STR_LEN] = '\0';
    *fileSize = atoi(file_size_str);

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);

#undef OTA_FILESIZE_STR_LEN
}

int qcloud_otalib_gen_info_msg(char *buf, size_t bufLen, uint32_t id, const char *version)
{
    IOT_FUNC_ENTRY;

    int ret;
    ret = HAL_Snprintf(buf,
                       bufLen,
                       "{\"type\": \"report_version\", \"report\":{\"version\":\"%s\"}}",
                       version);

    if (ret < 0) {
        Log_e("HAL_Snprintf failed");
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_FAILURE);
    }

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

int qcloud_otalib_gen_report_msg(char *buf, size_t bufLen, uint32_t id, const char *version, int progress, IOT_OTAReportType reportType)
{
    IOT_FUNC_ENTRY;

    int ret;

    switch (reportType) {
        /* 上报开始下载 */
        case IOT_OTAR_DOWNLOAD_BEGIN:
        ret = HAL_Snprintf(buf,
                            bufLen,
                            "{\"type\": \"report_progress\", \"report\": {\"progress\": {\"state\":\"downloading\", \"percent\":\"0\", \"result_code\":\"0\", \"result_msg\":\"\"}, \"version\": \"%s\"}}", version);
        break;
        /* 上报下载进度 */
        case IOT_OTAR_DOWNLOADING:
        ret = HAL_Snprintf(buf,
                            bufLen,
                            "{\"type\": \"report_progress\", \"report\": {\"progress\": {\"state\":\"downloading\", \"percent\":\"%d\", \"result_code\":\"0\", \"result_msg\":\"\"}, \"version\": \"%s\"}}",
                            progress, version);
        break;
        case IOT_OTAR_DOWNLOAD_TIMEOUT:
        case IOT_OTAR_FILE_NOT_EXIST:
        case IOT_OTAR_MD5_NOT_MATCH:
        case IOT_OTAR_AUTH_FAIL:
        case IOT_OTAR_UPGRADE_FAIL:
        ret = HAL_Snprintf(buf,
                            bufLen,
                            "{\"type\": \"report_progress\", \"report\": {\"progress\": {\"state\":\"fail\", \"result_code\":\"%d\", \"result_msg\":\"time_out\"}, \"version\": \"%s\"}}", reportType, version);
        break;
        /* 上报开始升级 */
        case IOT_OTAR_UPGRADE_BEGIN:
        ret = HAL_Snprintf(buf,
                       bufLen,
                       "{\"type\": \"report_progress\", \"report\":{\"progress\":{\"state\":\"burning\", \"result_code\":\"0\", \"result_msg\":\"\"}, \"version\":\"%s\"}}",
                       version);
        break;

        /* 上报升级完成 */
        case IOT_OTAR_UPGRADE_SUCCESS:
        ret = HAL_Snprintf(buf,
                       bufLen,
                       "{\"type\": \"report_progress\", \"report\":{\"progress\":{\"state\":\"done\", \"result_code\":\"0\", \"result_msg\":\"\"}, \"version\":\"%s\"}}",
                       version);
        break;
        
        default:
        IOT_FUNC_EXIT_RC(IOT_OTA_ERR_FAIL);
        break;
    }


    if (ret < 0) {
        Log_e("HAL_Snprintf failed");
        IOT_FUNC_EXIT_RC(IOT_OTA_ERR_FAIL);
    } else if (ret >= bufLen) {
        Log_e("msg is too long");
        IOT_FUNC_EXIT_RC(IOT_OTA_ERR_STR_TOO_LONG);
    }

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

#ifdef __cplusplus
}
#endif
