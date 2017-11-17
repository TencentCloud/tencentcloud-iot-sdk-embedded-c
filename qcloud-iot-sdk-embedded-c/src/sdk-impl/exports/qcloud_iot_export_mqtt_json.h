#ifndef QCLOUD_IOT_EXPORT_MQTT_JSON_H
#define QCLOUD_IOT_EXPORT_MQTT_JSON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 从JSON文档中解析出action字段
 *
 * @param pJsonDoc        待解析的JSON文档
 * @param tokenCount      JSONToken的个数
 * @param pAction         action字段
 * @return                返回true, 表示解析成功
 */
bool IOT_MQTT_JSON_GetAction(const char *pJsonDoc, int32_t tokenCount, char *pAction);

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_MQTT_JSON_H */
