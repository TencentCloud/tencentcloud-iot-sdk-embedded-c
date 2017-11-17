#ifndef MQTT_CLIENT_C_QCLOUD_IOT_JSON_DOC_HELPER_H
#define MQTT_CLIENT_C_QCLOUD_IOT_JSON_DOC_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>

#include "qcloud_iot_export_shadow_json.h"

#define CLIENT_TOKEN_FIELD     "clientToken"
#define VERSION_FIELD          "version"
#define CODE_FIELD             "code"
#define MESSAGE_FIELD          "message"

/**
 * @brief 返回一个ClientToken
 *
 * @param pStrBuffer    存储ClientToken的字符串缓冲区
 * @param sizeOfBuffer  缓冲区大小
 * @return              返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int generate_client_token(char *pStrBuffer, size_t sizeOfBuffer);

/**
 * @brief 为GET和DELETE请求构造一个只带有clientToken字段的JSON文档
 *
 * @param pJsonBuffer 存储JSON文档的字符串缓冲区
 */
void build_empty_json(char *pJsonBuffer);

/**
 * @brief 从JSON文档中解析出clientToken字段
 *
 * @param pJsonDoc       待解析的JSON文档
 * @param tokenCount     JSONTOKEN的个数, 若传入为0, 则首先对整个JSON文档进行解析
 * @param pClientToken   ClientToken字段
 * @return               返回true, 表示解析成功
 */
bool parse_client_token(const char *pJsonDoc, int32_t tokenCount, char *pClientToken);

/**
 * @brief 从JSON文档中解析出version字段
 *
 * @param pJsonDoc        待解析的JSON文档
 * @param tokenCount      JSONToken的个数
 * @param pVersionNumber  JSON文档版本号
 * @return                返回true, 表示解析成功
 */
bool parse_version_num(const char *pJsonDoc, int32_t tokenCount, uint32_t *pVersionNumber);

/**
 * @brief 从JSON文档中解析出code字段
 *
 * @param pJsonDoc      待解析的JSON文档
 * @param tokenCount    JSONToken的个数
 * @param pErrorCode    响应返回错误值
 * @return              返回true, 表示解析成功
 */
bool parse_error_code(const char *pJsonDoc, int32_t tokenCount, uint16_t *pErrorCode);

/**
 * @brief 从JSON文档中解析出message字段
 *
 * @param pJsonDoc         待解析的JSON文档
 * @param tokenCount       JSONToken的个数
 * @param pErrorMessage    响应返回错误提示消息
 * @return                 返回true, 表示解析成功
 */
bool parse_error_message(const char *pJsonDoc, int32_t tokenCount, char *pErrorMessage);

/**
 * @brief 如果JSON文档中的key与某个设备属性的key匹配的话, 则更新该设备属性, 该设备属性的值不能为OBJECT类型
 *
 * @param pJsonDoc       JSON文档
 * @param tokenCount     JSONToken的个数
 * @param pProperty      设备属性
 * @param pDataLength    JSON文档key对应的value的长度
 * @param pDataPosition  JSON文档中value在JSON串中的位置
 * @return               返回QCLOUD_ERR_SUCCESS, 表示成功
 */
bool update_value_if_key_match(const char *pJsonDoc, int32_t tokenCount,
                               DeviceProperty *pProperty, uint32_t *pDataLength, int32_t *pDataPosition);

#ifdef __cplusplus
}
#endif

#endif //MQTT_CLIENT_C_QCLOUD_IOT_JSON_DOC_HELPER_H
