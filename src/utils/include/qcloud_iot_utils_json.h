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

#ifndef QCLOUD_IOT_UTILS_JSON_H_
#define QCLOUD_IOT_UTILS_JSON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "qcloud_iot_export_error.h"
#include "qcloud_iot_export.h"
#include "jsmn.h"

/* JSON文档中JSONToken的最大个数 */
#define MAX_JSON_TOKEN_EXPECTED                                     (120)

extern jsmntok_t tokens[MAX_JSON_TOKEN_EXPECTED];

/**
 * @brief key比对
 *
 * @param json
 * @param tok
 * @param key
 * @return
 */
int8_t jsoneq(const char *json, jsmntok_t *tok, const char *key);

/**
 * @brief 解析JSON节点中的32位整数
 *
 * @param i
 * @param json
 * @param tok
 * @return
 */
int get_int32(int32_t *i, const char *json, jsmntok_t *tok);

/**
 * @brief 解析JSON节点中的16位整数
 *
 * @param i
 * @param json
 * @param tok
 * @return
 */
int get_int16(int16_t *i, const char *json, jsmntok_t *tok);

/**
 * @brief 解析JSON节点中的8位整数
 *
 * @param i
 * @param json
 * @param tok
 * @return
 */
int get_int8(int8_t *i, const char *json, jsmntok_t *tok);

/**
 * @brief 解析JSON节点中的无符号32位整数
 *
 * @param i
 * @param json
 * @param tok
 * @return
 */
int get_uint32(uint32_t *i, const char *json, jsmntok_t *tok);

/**
 * @brief 解析JSON节点中的无符号16位整数
 *
 * @param i
 * @param json
 * @param tok
 * @return
 */
int get_uint16(uint16_t *i, const char *json, jsmntok_t *tok);

/**
 * @brief 解析JSON节点中的无符号8位整数
 *
 * @param i
 * @param json
 * @param tok
 * @return
 */
int get_uint8(uint8_t *i, const char *json, jsmntok_t *tok);

/**
 * @brief 解析JSON节点中的单精度浮点数
 *
 * @param i
 * @param json
 * @param tok
 * @return
 */
int get_float(float *f, const char *json, jsmntok_t *tok);

/**
 * @brief 解析JSON节点中的双精度浮点数
 *
 * @param i
 * @param json
 * @param tok
 * @return
 */
int get_double(double *d, const char *json, jsmntok_t *tok);

/**
 * @brief 解析JSON节点中的布尔值
 *
 * @param i
 * @param json
 * @param tok
 * @return
 */
int get_boolean(bool *b, const char *json, jsmntok_t *tok);

/**
 * @brief 解析JSON节点中的字符串
 *
 * @param i
 * @param json
 * @param tok
 * @return
 */
int get_string(char *buf, const char *json, jsmntok_t *tok);

/**
 * @brief 检查JSON文档的有效性
 *
 * @param pJsonDoc  将被检车的JSON文档
 * @return          返回true, 则表示文档有效
 */
bool check_json_valid(const char *pJsonDoc);

/**
 * @brief 检查JSON文档的有效性并进行解析
 *
 * @param pJsonDoc      将被解析的JSON文档
 * @param pTokenCount   JSONToken的个数
 * @param pJsonTokens   解析出来的JSONToken数组
 * @return              返回true, 则表示文档有效且解析成功
 */
bool check_and_parse_json(const char *pJsonDoc, int32_t *pTokenCount, void **pJsonTokens);

/**
 * @brief 从JSON文档中解析出指定字段的值
 *
 * @param pJsonDoc         	待解析的JSON文档
 * @param valLen    		指定字段对应的value的长度
 * @return                 	成功返回字段对应的value, 失败返回NULL
 */
const char * parse_firmware_value_by_name(const char *pJsonDoc, const char *name, uint32_t *valLen);

#ifdef __cplusplus
}
#endif

#endif //QCLOUD_IOT_UTILS_JSON_H_
