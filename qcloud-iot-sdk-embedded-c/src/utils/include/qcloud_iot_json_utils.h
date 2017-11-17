#ifndef MQTT_CLIENT_C_QCLOUD_IOT_JSON_UTILS_H
#define MQTT_CLIENT_C_QCLOUD_IOT_JSON_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "qcloud_iot_export_error.h"
#include "qcloud_iot_export.h"
#include "jsmn.h"

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

#ifdef __cplusplus
}
#endif

#endif //MQTT_CLIENT_C_QCLOUD_IOT_JSON_UTILS_H
