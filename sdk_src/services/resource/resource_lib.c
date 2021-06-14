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

#include "resource_lib.h"

#include <stdio.h>
#include <string.h>

#include "lite-utils.h"
#include "ota_client.h"
#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "utils_md5.h"
#include "utils_base64.h"

int qcloud_lib_get_json_key_value(char *json_obj, char *json_key, char **value)
{
    *value = LITE_json_value_of(json_key, json_obj);
    if (NULL == *value) {
        Log_e("Not '%s' key in json doc", STRING_PTR_PRINT_SANITY_CHECK(json_key));
        return QCLOUD_RESOURCE_ERRCODE_FAIL_E;
    }

    return QCLOUD_RET_SUCCESS;
}

void qcloud_lib_md5_init(void *ctx_md5)
{
    iot_md5_context *ctx = (iot_md5_context *)ctx_md5;
    if (NULL == ctx) {
        return;
    }

    utils_md5_init(ctx);
    utils_md5_starts(ctx);

    return;
}

void qcloud_lib_md5_update(void *md5, const char *buf, size_t buf_len)
{
    utils_md5_update(md5, (unsigned char *)buf, buf_len);
}

void qcloud_lib_md5_finish_tolowercasehex(void *md5, char *output_str)
{
    int           i;
    unsigned char buf_out[16];
    utils_md5_finish(md5, buf_out);

    for (i = 0; i < 16; ++i) {
        output_str[i * 2]     = utils_hb2hex(buf_out[i] >> 4);
        output_str[i * 2 + 1] = utils_hb2hex(buf_out[i]);
    }
    output_str[32] = '\0';
}

int qcloud_lib_base64encode_md5sum(char *md5sumhex, char *outbase64, int outbase64len)
{
    char   md5[17];
    size_t olen;

    for (int index = 0; index < 16; index++) {
        if (md5sumhex[index * 2] < 'a') {
            md5[index] = ((md5sumhex[index * 2] - '0') << 4);
        } else {
            md5[index] = ((10 + (md5sumhex[index * 2] - 'a')) << 4);
        }

        if (md5sumhex[index * 2 + 1] < 'a') {
            md5[index] += ((md5sumhex[index * 2 + 1] - '0'));
        } else {
            md5[index] += (10 + (md5sumhex[index * 2 + 1] - 'a'));
        }
    }

    qcloud_iot_utils_base64encode((unsigned char *)outbase64, outbase64len, &olen, (unsigned char *)md5, 16);

    return (olen > outbase64len) ? QCLOUD_ERR_FAILURE : QCLOUD_RET_SUCCESS;
}

void qcloud_lib_md5_deinit(void *md5)
{
    if (NULL != md5) {
        memset(md5, 0, sizeof(iot_md5_context));
    }
}

#ifdef __cplusplus
}
#endif
