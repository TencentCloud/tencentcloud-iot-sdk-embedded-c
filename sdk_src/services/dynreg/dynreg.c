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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "lite-utils.h"
#include "qcloud_iot_ca.h"
#include "qcloud_iot_common.h"
#include "qcloud_iot_device.h"
#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "utils_aes.h"
#include "utils_base64.h"
#include "utils_hmac.h"
#include "utils_httpc.h"
#include "utils_sha1.h"
#include "utils_md5.h"
#include "qcloud_iot_http.h"

#define REG_URL_MAX_LEN             (128)
#define DYN_REG_SIGN_LEN            (64)
#define DYN_BUFF_DATA_MORE          (10)
#define BASE64_ENCODE_OUT_LEN(x)    (((x + 3) * 4) / 3)
#define DYN_REG_RES_HTTP_TIMEOUT_MS (2000)

#ifdef AUTH_MODE_CERT
#define DYN_RESPONSE_BUFF_LEN (5 * 1024)
#define DECODE_BUFF_LEN       (5 * 1024)
#else
#define DYN_RESPONSE_BUFF_LEN (256)
#define DECODE_BUFF_LEN       (256)
#endif

/* Knuth's TAOCP section 3.6 */
#define M ((1U << 31) - 1)
#define A 48271
#define Q 44488  // M/A
#define R 3399   // M%A; R < Q !!!

#define CODE_RESAULT "code"
#define ENCRYPT_TYPE "encryptionType"
#define PSK_DATA     "psk"
#define CERT_DATA    "clientCert"
#define KEY_DATA     "clientKey"

typedef enum {
    eCERT_TYPE = 1,
    ePSK_TYPE  = 2,
} eAuthType;

/*Global value*/
static unsigned int _seed = 1;

int rand_r(unsigned int *seed)
{
    int32_t X;

    X = *seed;
    X = A * (X % Q) - R * (int32_t)(X / Q);
    if (X < 0)
        X += M;

    *seed = X;
    return X;
}

int rand_d(void)
{
    return rand_r(&_seed);
}

void srand_d(unsigned int i)
{
    _seed = i;
}

static int _get_json_resault_code(char *json)
{
    int   resault = -1;
    char *v       = LITE_json_value_of(CODE_RESAULT, json);

    if (v == NULL) {
        Log_e("Invalid json content: %s", STRING_PTR_PRINT_SANITY_CHECK(json));
        return -1;
    }

    if (LITE_get_int32((int32_t *)&resault, v) != QCLOUD_RET_SUCCESS) {
        Log_e("Invalid json content: %s", json);
        HAL_Free(v);
        return -1;
    }

    HAL_Free(v);

    return resault;
}

static int _get_json_encry_type(char *json)
{
    int   type = -1;
    char *v    = LITE_json_value_of(ENCRYPT_TYPE, json);

    if (v == NULL) {
        Log_e("Get encry type fail, %s", STRING_PTR_PRINT_SANITY_CHECK(json));
        return -1;
    }

    if (LITE_get_int32((int32_t *)&type, v) != QCLOUD_RET_SUCCESS) {
        Log_e("Invalid json content: %s", json);
        HAL_Free(v);
        return -1;
    }

    HAL_Free(v);

    return type;
}

#ifndef AUTH_MODE_CERT

static char *_get_json_psk(char *json)
{
    char *psk = LITE_json_value_of(PSK_DATA, json);

    if (psk == NULL) {
        Log_e("Get psk fail: %s", STRING_PTR_PRINT_SANITY_CHECK(json));
    }

    return psk;
}

#else
static char *_get_json_cert_data(char *json)
{
    char *cert = LITE_json_value_of(CERT_DATA, json);

    if (cert == NULL) {
        Log_e("Get clientCert fail: %s", STRING_PTR_PRINT_SANITY_CHECK(json));
    }

    return cert;
}

static char *_get_json_key_data(char *json)
{
    char *key = LITE_json_value_of(KEY_DATA, json);

    if (key == NULL) {
        Log_e("Get clientCert fail: %s", STRING_PTR_PRINT_SANITY_CHECK(json));
    }

    return key;
}

/*\\n in data change to '\n'*/
static void _deal_transfer(char *data, uint32_t dataLen)
{
    int i;

    for (i = 0; i < dataLen; i++) {
        if ((data[i] == '\\') && (data[i + 1] == 'n')) {
            data[i]     = ' ';
            data[i + 1] = '\n';
        }
    }
}

static int _cert_file_save(const char *fileName, char *data, uint32_t dataLen)
{
    FILE *   fp;
    char     filePath[FILE_PATH_MAX_LEN];
    uint32_t len;
    int      Ret = QCLOUD_ERR_FAILURE;

    memset(filePath, 0, FILE_PATH_MAX_LEN);
    HAL_Snprintf(filePath, FILE_PATH_MAX_LEN, "./certs/%s", STRING_PTR_PRINT_SANITY_CHECK(fileName));

    if ((fp = fopen(filePath, "w+")) == NULL) {
        Log_e("fail to open file %s", STRING_PTR_PRINT_SANITY_CHECK(fileName));
        goto exit;
    }

    _deal_transfer(data, dataLen);
    len = fprintf(fp, "%s", data);
    fclose(fp);

    if (len == dataLen) {
        Log_d("save %s file succes", fileName);
        Ret = QCLOUD_RET_SUCCESS;
    }

exit:
    return Ret;
}

#endif

static int _parse_devinfo(char *jdoc, DeviceInfo *pDevInfo)
{
    int           ret = 0;
    size_t        len;
    int           datalen;
    int           enType;
    unsigned int  keybits;
    char          key[UTILS_AES_BLOCK_LEN + 1];
    char          decodeBuff[DECODE_BUFF_LEN] = {0};
    unsigned char iv[16];
    char *        payload  = NULL;
    char *        response = NULL;

#ifdef AUTH_MODE_CERT
    char *clientCert;
    char *clientKey;
#else
    char *psk;
#endif

    Log_d("recv: %s", STRING_PTR_PRINT_SANITY_CHECK(jdoc));
    ret      = _get_json_resault_code(jdoc);
    response = LITE_json_value_of("Response", jdoc);
    if (NULL == response) {
        Log_e("Invalid json content: %s", response);
        ret = QCLOUD_ERR_FAILURE;
        goto exit;
    }

    payload = LITE_json_value_of("Payload", response);
    if (payload == NULL) {
        Log_e("response error, json content: %s", response);
        ret = QCLOUD_ERR_FAILURE;
        goto exit;
    } else {
        Log_d("payload:%s", payload);
    }

    ret = qcloud_iot_utils_base64decode((uint8_t *)decodeBuff, sizeof(decodeBuff), &len, (uint8_t *)payload,
                                        strlen(payload));
    if (ret != QCLOUD_RET_SUCCESS) {
        Log_e("Response decode err, response:%s", payload);
        ret = QCLOUD_ERR_FAILURE;
        goto exit;
    }

    datalen = len + (UTILS_AES_BLOCK_LEN - len % UTILS_AES_BLOCK_LEN);
    keybits = AES_KEY_BITS_128;
    memset(key, 0, UTILS_AES_BLOCK_LEN);
    strncpy(key, pDevInfo->product_secret, UTILS_AES_BLOCK_LEN);
    memset(iv, '0', UTILS_AES_BLOCK_LEN);
    ret = utils_aes_cbc((uint8_t *)decodeBuff, datalen, (uint8_t *)decodeBuff, DECODE_BUFF_LEN, UTILS_AES_DECRYPT,
                        (uint8_t *)key, keybits, iv);
    if (QCLOUD_RET_SUCCESS == ret) {
        // Log_d("The decrypted data is:%s", decodeBuff);

    } else {
        Log_e("data decry err,ret:%d", ret);
        goto exit;
    }

    enType = _get_json_encry_type(decodeBuff);
    if (enType < 0) {
        Log_e("invlid encryt type, decrypt maybe faild");
        ret = QCLOUD_ERR_FAILURE;
        goto exit;
    }

#ifdef AUTH_MODE_CERT
    if (eCERT_TYPE != enType) {
        Log_e("encryt type should be cert type");
        ret = QCLOUD_ERR_FAILURE;
        goto exit;
    }

    clientCert = _get_json_cert_data(decodeBuff);
    if (NULL != clientCert) {
        memset(pDevInfo->dev_cert_file_name, 0, MAX_SIZE_OF_DEVICE_CERT_FILE_NAME);
        HAL_Snprintf(pDevInfo->dev_cert_file_name, MAX_SIZE_OF_DEVICE_CERT_FILE_NAME, "%s_cert.crt",
                     STRING_PTR_PRINT_SANITY_CHECK(pDevInfo->device_name));
        if (QCLOUD_RET_SUCCESS != _cert_file_save(pDevInfo->dev_cert_file_name, clientCert, strlen(clientCert))) {
            Log_e("save %s file fail", pDevInfo->dev_cert_file_name);
            ret = QCLOUD_ERR_FAILURE;
        }

        HAL_Free(clientCert);

    } else {
        Log_e("Get clientCert data fail");
        ret = QCLOUD_ERR_FAILURE;
    }

    clientKey = _get_json_key_data(decodeBuff);
    if (NULL != clientKey) {
        memset(pDevInfo->dev_key_file_name, 0, MAX_SIZE_OF_DEVICE_SECRET_FILE_NAME);
        HAL_Snprintf(pDevInfo->dev_key_file_name, MAX_SIZE_OF_DEVICE_SECRET_FILE_NAME, "%s_private.key",
                     pDevInfo->device_name);
        if (QCLOUD_RET_SUCCESS != _cert_file_save(pDevInfo->dev_key_file_name, clientKey, strlen(clientKey))) {
            Log_e("save %s file fail", pDevInfo->dev_key_file_name);
            ret = QCLOUD_ERR_FAILURE;
        }

        HAL_Free(clientKey);

    } else {
        Log_e("Get clientCert data fail");
        ret = QCLOUD_ERR_FAILURE;
    }

#else
    if (ePSK_TYPE != enType) {
        Log_e("encryt type should be psk type");
        ret = QCLOUD_ERR_FAILURE;
        goto exit;
    }

    psk = _get_json_psk(decodeBuff);
    if (NULL != psk) {
        if (strlen(psk) > MAX_SIZE_OF_DEVICE_SECRET) {
            Log_e("psk exceed max len,%s", psk);
            strcpy(pDevInfo->device_secret, psk);
        } else {
            strncpy(pDevInfo->device_secret, psk, MAX_SIZE_OF_DEVICE_SECRET);
            pDevInfo->device_secret[MAX_SIZE_OF_DEVICE_SECRET] = '\0';
        }
        HAL_Free(psk);
    } else {
        Log_e("Get psk data fail");
    }
#endif
exit:

    if (payload) {
        HAL_Free(payload);
    }

    HAL_Free(response);

    return ret;
}

static int _post_reg_request_by_http(char *request_buf, DeviceInfo *pDevInfo)
{
    int            Ret = 0;
    HTTPClient     http_client; /* http client */
    HTTPClientData http_data;   /* http client data */

    char        url[REG_URL_MAX_LEN] = {0};
    int         port;
    const char *ca_crt = NULL;
    char        respbuff[DYN_RESPONSE_BUFF_LEN];
    char *      dynreg_uri = "/device/register";
    const char *url_format = "%s://%s%s";

/*format URL*/
#ifndef AUTH_WITH_NOTLS
    HAL_Snprintf(url, REG_URL_MAX_LEN, url_format, "https", DYN_REG_SERVER_URL, dynreg_uri);
    port   = DYN_REG_SERVER_PORT_TLS;
    ca_crt = iot_ca_get();
#else
    HAL_Snprintf(url, REG_URL_MAX_LEN, url_format, "http", DYN_REG_SERVER_URL, dynreg_uri);
    port = DYN_REG_SERVER_PORT;
#endif

    memset((char *)&http_client, 0, sizeof(HTTPClient));
    memset((char *)&http_data, 0, sizeof(HTTPClientData));

    http_client.header = qcloud_iot_http_header_create(request_buf, strlen(request_buf), DYN_REG_SERVER_URL, dynreg_uri,
                                                       "application/json;", pDevInfo->product_secret, NULL);
    http_data.post_content_type = "application/json; charset=utf-8";
    http_data.post_buf          = request_buf;
    http_data.post_buf_len      = strlen(request_buf);

    Ret = qcloud_http_client_common(&http_client, url, port, ca_crt, HTTP_POST, &http_data);
    if (QCLOUD_RET_SUCCESS != Ret) {
        qcloud_iot_http_header_destory(http_client.header);
        Log_e("qcloud_http_client_common failed, Ret = %d", Ret);
        return Ret;
    }

    memset(respbuff, 0, DYN_RESPONSE_BUFF_LEN);
    http_data.response_buf_len = DYN_RESPONSE_BUFF_LEN;
    http_data.response_buf     = respbuff;

    Log_e("dynamic register recvl, Ret = %d", Ret);
    Ret = qcloud_http_recv_data(&http_client, DYN_REG_RES_HTTP_TIMEOUT_MS, &http_data);
    if (QCLOUD_RET_SUCCESS != Ret) {
        Log_e("dynamic register response fail, Ret = %d", Ret);
    } else {
        /*Parse dev info*/
        Ret = _parse_devinfo(http_data.response_buf, pDevInfo);
        if (QCLOUD_RET_SUCCESS != Ret) {
            Log_e("parse device info err");
        }
    }

    qcloud_http_client_close(&http_client);
    qcloud_iot_http_header_destory(http_client.header);

    return Ret;
}

int IOT_DynReg_Device(DeviceInfo *pDevInfo)
{
    int Ret;
    if (strlen(pDevInfo->product_secret) < UTILS_AES_BLOCK_LEN) {
        Log_e("product key inllegal");
        return QCLOUD_ERR_FAILURE;
    }

    const char *http_request_body_format = "{\"ProductId\":\"%s\",\"DeviceName\":\"%s\"}";

    int request_body_len =
        strlen(http_request_body_format) + strlen(pDevInfo->product_id) + strlen(pDevInfo->device_name);
    char *request_body = HAL_Malloc(request_body_len);
    if (NULL == request_body) {
        Log_e("request body malloc failed");
        return QCLOUD_ERR_FAILURE;
    }

    HAL_Snprintf(request_body, request_body_len, http_request_body_format, pDevInfo->product_id, pDevInfo->device_name);
    Log_d("request:%s", request_body);

    /*post request*/
    Ret = _post_reg_request_by_http(request_body, pDevInfo);
    if (QCLOUD_RET_SUCCESS == Ret) {
        Log_d("request dev info success");
    } else {
        Log_e("request dev info fail");
    }

    HAL_Free(request_body);

    return Ret;
}

#ifdef __cplusplus
}
#endif
