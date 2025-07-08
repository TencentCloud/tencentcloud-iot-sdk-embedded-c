/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2018-2020 Tencent. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "lite-utils.h"
#include "qcloud_iot_export_log.h"
#include "qcloud_iot_import.h"

char *LITE_format_string(const char *fmt, ...)
{
#define TEMP_STRING_MAXLEN (512)

    va_list ap;
    char *  tmp = NULL;
    char *  dst;
    int     rc = -1;

    va_start(ap, fmt);
    tmp = HAL_Malloc(TEMP_STRING_MAXLEN);
    memset(tmp, 0, TEMP_STRING_MAXLEN);
    rc = HAL_Vsnprintf(tmp, TEMP_STRING_MAXLEN, fmt, ap);
    va_end(ap);
    LITE_ASSERT(tmp);
    LITE_ASSERT(rc < 1024);

    dst = LITE_strdup(tmp);
    HAL_Free(tmp);

    return dst;

#undef TEMP_STRING_MAXLEN
}

char *LITE_format_nstring(const int len, const char *fmt, ...)
{
    va_list ap;
    char *  tmp = NULL;
    char *  dst;
    int     rc = -1;

    va_start(ap, fmt);
    tmp = HAL_Malloc(len + 2);
    memset(tmp, 0, len + 2);
    rc = HAL_Vsnprintf(tmp, len + 1, fmt, ap);
    va_end(ap);
    LITE_ASSERT(tmp);
    LITE_ASSERT(rc < 1024);

    dst = HAL_Malloc(len + 1);
    HAL_Snprintf(dst, (len + 1), "%s", tmp);
    HAL_Free(tmp);

    return dst;
}

char *LITE_strdup(const char *src)
{
    int   len = 0;
    char *dst = NULL;

    if (!src) {
        return NULL;
    }
    len = strlen(src) + 1;
    if (len > 1024) {
        Log_e("Too long string to duplicate, abort! len = %d", len);
        return NULL;
    }

    dst = (char *)HAL_Malloc(sizeof(char) * len);
    if (!dst) {
        return NULL;
    }
    strncpy(dst, src, len);

    return dst;
}

void LITE_hexbuf_convert(unsigned char *digest, char *out, int in_len, int uppercase)
{
    static char *zEncode[] = {"0123456789abcdef", "0123456789ABCDEF"};
    int          j         = 0;
    int          i         = 0;
    int          idx       = uppercase ? 1 : 0;

    for (i = 0; i < in_len; i++) {
        int a = digest[i];

        out[j++] = zEncode[idx][(a >> 4) & 0xf];
        out[j++] = zEncode[idx][a & 0xf];
    }
}

static uint8_t _hexval_of_char(char hex)
{
    if (LITE_isdigit(hex)) {
        return (hex - '0');
    }
    if (hex >= 'a' && hex <= 'f') {
        return (hex - 'a' + 10);
    }
    if (hex >= 'A' && hex <= 'F') {
        return (hex - 'A' + 10);
    }

    return 0;
}

void LITE_hexstr_convert(char *hexstr, uint8_t *out_buf, int in_len)
{
    int     i = 0;
    uint8_t ch0, ch1;

    if (in_len % 2 != 0) {
        Log_e("hexstr length (%d) is not even", in_len);
        return;
    }

    while (i < (in_len / 2)) {
        ch0        = _hexval_of_char((char)hexstr[2 * i]);
        ch1        = _hexval_of_char((char)hexstr[2 * i + 1]);
        out_buf[i] = (ch0 << 4 | ch1);
        i++;
    }
}
