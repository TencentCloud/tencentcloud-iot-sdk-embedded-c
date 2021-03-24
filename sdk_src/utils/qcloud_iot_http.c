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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "qcloud_iot_import.h"
#include "utils_md5.h"
#include "utils_hmac.h"
#include "utils_sha1.h"

#include "qcloud_iot_http.h"
#include "utils_param_check.h"
#include "qcloud_iot_export_error.h"
#include "utils_base64.h"

#if 1
/* Knuth's TAOCP section 3.6 */
#define M ((1U << 31) - 1)
#define A 48271
#define Q 44488  // M/A
#define R 3399   // M%A; R < Q !!!

/*Global value*/
static unsigned int _seed = 1;

int qcloud_rand_r(unsigned int *seed)
{
    int32_t X;

    X = *seed;
    X = A * (X % Q) - R * (int32_t)(X / Q);
    if (X < 0)
        X += M;

    *seed = X;
    return X;
}

int qcloud_rand_d(void)
{
    return qcloud_rand_r(&_seed);
}

void qcloud_srand_d(unsigned int i)
{
    _seed = i;
}

#ifdef AUTH_MODE_CERT
void *qcloud_iot_http_create_privatekey(char *privatekey_file)
{
    return HAL_TLS_Get_PrivateKey_FromFile(privatekey_file);
}

void qcloud_iot_http_destory_privatekey(char *privatekey_file)
{
    HAL_TLS_Destory_PrivateKey(privatekey_file);
}

void qcloud_iot_http_body_sha256(char *inbuf, int inbuf_len, char *outsha256value)
{
    size_t        i = 0;
    unsigned char out[QCLOUD_SHA256_RESULT_LEN];

    HAL_TLS_Calc_SHA256((unsigned char *)inbuf, inbuf_len, out);

    for (i = 0; i < QCLOUD_SHA256_RESULT_LEN; ++i) {
        outsha256value[i * 2]     = utils_hb2hex(out[i] >> 4);
        outsha256value[i * 2 + 1] = utils_hb2hex(out[i]);
    }
}
#endif

void qcloud_iot_http_body_sha1(char *inbuf, int inbuf_len, char *outbuf)
{
    iot_sha1_context context;
    size_t           i       = 0;
    unsigned char    out[20] = {0};

    utils_sha1_init(&context);
    utils_sha1_starts(&context);
    utils_sha1_update(&context, (unsigned char *)inbuf, inbuf_len);
    utils_sha1_finish(&context, out);
    for (i = 0; i < 20; ++i) {
        outbuf[i * 2]     = utils_hb2hex(out[i] >> 4);
        outbuf[i * 2 + 1] = utils_hb2hex(out[i]);
    }
}

int qcloud_iot_http_post_sign_calc(QCLOUD_IOT_HTTP_HEADER_POST_SIGN *post_sign, char *signout)
{
    int    ret                                              = QCLOUD_RET_SUCCESS;
    int    sign_len                                         = 0;
    char   request_buf_shax[QCLOUD_SHAX_RESULT_LEN * 2 + 1] = {0};
    char * sign                                             = NULL;
    size_t olen;

    POINTER_SANITY_CHECK(post_sign, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(post_sign->host, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(post_sign->uri, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(post_sign->algorithm, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(post_sign->request_body_buf, QCLOUD_ERR_INVAL);
    if ((post_sign->secretkey == NULL) && (post_sign->privatekey == NULL)) {
        Log_e("http post sign secretkey & privatekey is NULL");
        return QCLOUD_ERR_INVAL;
    }

    /*
        StringToSign =
            HTTPRequestMethod + \n +       ----> POST
            CanonicalHost + \n +           ----> HOST NAME
            CanonicalURI + \n +            ----> URI
            CanonicalQueryString + \n +    ----> POST Default ""
            Algorithm + \n +               ----> hmacsha1
            RequestTimestamp + \n +        ----> timestamp
            Nonce + \n +                   ----> nonce
            HashedCanonicalRequest         ----> request body sha256 result lowercase hex encode
    */

    int sign_string_len = (strlen("POST") + 1 + strlen(post_sign->host) + 1 + strlen(post_sign->uri) + 1 + strlen("") +
                           1 + strlen(post_sign->algorithm) + 1 + 20 + 1 + 20 + 1 + QCLOUD_SHAX_RESULT_LEN * 2);

    char *sign_string = HAL_Malloc(sign_string_len);
    if (NULL == sign_string) {
        Log_e("malloc sign string fail");
        return QCLOUD_ERR_FAILURE;
    }

    /*cal hmac sha1*/
    if (0 == strncmp(post_sign->algorithm, QCLOUD_SUPPORT_HMACSHA1, sizeof(QCLOUD_SUPPORT_HMACSHA1) - 1)) {
        qcloud_iot_http_body_sha1(post_sign->request_body_buf, post_sign->request_body_buf_len, request_buf_shax);
        // create sign string
        HAL_Snprintf(sign_string, sign_string_len, "%s\n%s\n%s\n%s\n%s\n%d\n%d\n%s", "POST", post_sign->host,
                     post_sign->uri, "", post_sign->algorithm, post_sign->timestamp, post_sign->nonce,
                     request_buf_shax);

        sign_len = QCLOUD_SHA1_RESULT_LEN;
        sign     = HAL_Malloc(sign_len);
        if (NULL == sign) {
            HAL_Free(sign_string);
            Log_e("malloc sign fail");
            return QCLOUD_ERR_FAILURE;
        }

        utils_hmac_sha1_hex(sign_string, strlen(sign_string), sign, post_sign->secretkey, strlen(post_sign->secretkey));
    } else if (0 == strncmp(post_sign->algorithm, QCLOUD_SUPPORT_RSASHA256, sizeof(QCLOUD_SUPPORT_RSASHA256) - 1)) {
#ifdef AUTH_MODE_CERT
        qcloud_iot_http_body_sha256(post_sign->request_body_buf, post_sign->request_body_buf_len, request_buf_shax);

        // create sign string
        HAL_Snprintf(sign_string, sign_string_len, "%s\n%s\n%s\n%s\n%s\n%d\n%d\n%s", "POST", post_sign->host,
                     post_sign->uri, "", post_sign->algorithm, post_sign->timestamp, post_sign->nonce,
                     request_buf_shax);

        sign_len = HAL_TLS_Get_RSASHA256_Result_Len(post_sign->privatekey);
        sign     = HAL_Malloc(sign_len);
        if (NULL == sign) {
            HAL_Free(sign_string);
            Log_e("malloc sign fail");
            return QCLOUD_ERR_FAILURE;
        }

        HAL_TLS_Calc_Sign_RSASHA256(post_sign->privatekey, sign_string, strlen(sign_string), sign);
#endif

    } else {
        Log_e("not support algorithm :%s", post_sign->algorithm);
        ret = QCLOUD_ERR_FAILURE;
    }

    /*base64 encode*/
    qcloud_iot_utils_base64encode((uint8_t *)signout, sign_len * 2, &olen, (const uint8_t *)sign, sign_len);

    HAL_Free(sign_string);
    HAL_Free(sign);

    return ret;
}

char *qcloud_iot_http_header_create(char *request_body_buf, int request_body_buf_len, const char *host, const char *uri,
                                    char *accept_header, char *secretkey, void *privatekey)
{
    int      nonce;
    uint32_t timestamp;
    char *   sign     = NULL;
    int      sign_len = 0;

    QCLOUD_IOT_HTTP_HEADER_POST_SIGN post_sign;

    qcloud_srand_d(HAL_GetTimeMs());
    nonce     = qcloud_rand_d();
    timestamp = HAL_Timer_current_sec();

    memset(&post_sign, 0, sizeof(QCLOUD_IOT_HTTP_HEADER_POST_SIGN));
    post_sign.host      = host;
    post_sign.uri       = uri;
    post_sign.secretkey = secretkey;
    if (NULL != secretkey) {
        post_sign.algorithm = QCLOUD_SUPPORT_HMACSHA1;
        sign_len            = QCLOUD_SHA1_RESULT_LEN;
    }

#ifdef AUTH_MODE_CERT
    post_sign.privatekey = privatekey;
    if (NULL != privatekey) {
        post_sign.algorithm = QCLOUD_SUPPORT_RSASHA256;
        sign_len            = HAL_TLS_Get_RSASHA256_Result_Len(post_sign.privatekey);
    }
#endif

    post_sign.request_body_buf     = request_body_buf;
    post_sign.request_body_buf_len = request_body_buf_len;
    post_sign.timestamp            = timestamp;
    post_sign.nonce                = nonce;

    sign = HAL_Malloc(sign_len * 2);
    if (NULL == sign) {
        Log_e("malloc sign fail");
        return NULL;
    }

    memset(sign, 0, sign_len * 2);

    if (QCLOUD_RET_SUCCESS != qcloud_iot_http_post_sign_calc(&post_sign, sign)) {
        HAL_Free(sign);
        Log_e("http post sign calc");
        return NULL;
    }
    int http_header_len = (sizeof(QCLOUD_HTTP_HEADER_FORMAT) + strlen(accept_header) + strlen(post_sign.algorithm) +
                           20 + 20 + sign_len * 2);

    char *http_header = HAL_Malloc(http_header_len);
    if (NULL == http_header) {
        HAL_Free(sign);
        Log_e("malloc http header fail");
        return NULL;
    }

    HAL_Snprintf(http_header, http_header_len, QCLOUD_HTTP_HEADER_FORMAT, accept_header, post_sign.algorithm, timestamp,
                 nonce, sign);

    HAL_Free(sign);

    return http_header;
}

void qcloud_iot_http_header_destory(char *http_header)
{
    HAL_Free(http_header);
}
#endif
#ifdef __cplusplus
}
#endif
