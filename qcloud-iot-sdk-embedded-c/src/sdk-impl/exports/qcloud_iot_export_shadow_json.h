#ifndef QCLOUD_IOT_EXPORT_SHADOW_JSON_H
#define QCLOUD_IOT_EXPORT_SHADOW_JSON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief JSON文档中支持的数据类型
 */
typedef enum {
    JINT32,     // 32位有符号整型
    JINT16,     // 16位有符号整型
    JINT8,      // 8位有符号整型
    JUINT32,    // 32位无符号整型
    JUINT16,    // 16位无符号整型
    JUINT8,     // 8位无符号整型
    JFLOAT,     // 单精度浮点型
    JDOUBLE,    // 双精度浮点型
    JBOOL,      // 布尔型
    JSTRING,    // 字符串
    JOBJECT     // JSON对象
} JsonDataType;

/**
 * @brief 定义设备的某个属性, 实际就是一个JSON文档节点
 */
typedef struct _JSONNode {
    const char   *key;    // 该JSON节点的Key
    void         *data;   // 该JSON节点的Value
    JsonDataType type;    // 该JSON节点的数据类型
} DeviceProperty;

/**
 * @brief 在JSON文档中添加reported字段
 *
 * 本函数需要在函数json_document_init调用之后调用
 *
 * @param pJsonBuffer   为存储JSON文档准备的字符串缓冲区
 * @param sizeOfBuffer  缓冲区大小
 * @param count         可变参数的个数, 即需上报的设备属性的个数
 * @return              返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int IOT_Shadow_JSON_ConstructReport(char *pJsonBuffer, size_t sizeOfBuffer, uint8_t count, ...);

/**
 * @brief 在JSON文档中添加desired字段
 *
 * @param pJsonBuffer   为存储JSON文档准备的字符串缓冲区
 * @param sizeOfBuffer  缓冲区大小
 * @param count         可变参数的个数, 即需上报的设备属性的个数
 * @return              返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int IOT_Shadow_JSON_ConstructDesire(char *pJsonBuffer, size_t sizeOfBuffer, uint8_t count, ...);

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_SHADOW_JSON_H */
