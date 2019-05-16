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

#ifndef QCLOUD_IOT_EXPORT_SHADOW_H_
#define QCLOUD_IOT_EXPORT_SHADOW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "qcloud_iot_export_mqtt.h"

typedef enum _eShadowType_{
	eSHADOW = 0,
	eTEMPLATE = 1,
}eShadowType;


typedef struct {
    /**
     * 设备基础信息
     */
    char                        *product_id;            // 产品名称
    char                        *device_name;           // 设备名称

#ifdef AUTH_MODE_CERT
    /**
     * 非对称加密使用
     */
    char                        *cert_file;              // 客户端证书文件路径
    char                        *key_file;               // 客户端私钥文件路径
#else
    /**
     * 对称加密
     */
    char                        *device_secret;                    // 对称加密密钥
#endif

    uint32_t                    command_timeout;         // 发布订阅信令读写超时时间 ms
    uint32_t                    keep_alive_interval_ms;  // 心跳周期, 单位: ms

    uint8_t                     clean_session;           // 清理会话标志位

    uint8_t                     auto_connect_enable;     // 是否开启自动重连 1:启用自动重连 0：不启用自动重连  建议为1

    MQTTEventHandler            event_handle;            // 事件回调
	eShadowType					shadow_type;			//影子类型，eSHADOW：通用影子操作结果TOPIC eTEMPLATE：数据模板操作结果TOPIC
} ShadowInitParams;

#ifdef AUTH_MODE_CERT
    #define DEFAULT_SHAWDOW_INIT_PARAMS { NULL, NULL, NULL, NULL, 2000, 240 * 1000, 1, 1, {0}}
#else
    #define DEFAULT_SHAWDOW_INIT_PARAMS { NULL, NULL, NULL, 2000, 240 * 1000, 1, 1, {0}}
#endif

/**
 * @brief 请求响应返回的类型
 */
typedef enum {
    ACK_NONE = -3,      // 请求超时
    ACK_TIMEOUT = -2,   // 请求超时
    ACK_REJECTED = -1,  // 请求拒绝
    ACK_ACCEPTED = 0    // 请求接受
} RequestAck;

/**
 * @brief 操作云端设备文档可以使用的三种方式
 */
typedef enum {
    GET,     // 获取云端设备文档
    UPDATE,  // 更新或创建云端设备文档
} Method;

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
    char   		 *key;    // 该JSON节点的Key
    void         *data;   // 该JSON节点的Value
    JsonDataType type;    // 该JSON节点的数据类型
} DeviceProperty;


/**
 * @brief 定义数据模板的数据点类型
 */
 
#define TYPE_TEMPLATE_INT    	JINT32
#define TYPE_TEMPLATEENUM    	JINT32
#define TYPE_TEMPLATE_FLOAT  	JFLOAT
#define TYPE_TEMPLATE_BOOL   	JINT8
#define TYPE_TEMPLATE_STRING 	JSTRING
#define TYPE_TEMPLATE_TIME 		JUINT32
#define TYPE_TEMPLATE_JOBJECT 	JOBJECT



typedef int32_t   TYPE_DEF_TEMPLATE_INT;
typedef int32_t   TYPE_DEF_TEMPLATE_ENUM;
typedef float     TYPE_DEF_TEMPLATE_FLOAT;
typedef char      TYPE_DEF_TEMPLATE_BOOL;
typedef char      TYPE_DEF_TEMPLATE_STRING;
typedef uint32_t  TYPE_DEF_TEMPLATE_TIME;
typedef void *    TYPE_DEF_TEMPLATE_OBJECT;


/**
 * @brief 定义数据模板的属性状态
 */
typedef enum _eDataState_{
    eNOCHANGE = 0,
	eCHANGED = 1,	
} eDataState;

/**
 * @brief 定义数据模板的属性结构
 */
typedef struct {
    DeviceProperty data_property;
    eDataState state;
} sDataPoint;


/**
 * @brief 每次文档请求响应的回调函数
 *
 * @param method         文档操作方式
 * @param requestAck     请求响应类型
 * @param pJsonDocument  云端响应返回的文档
 * @param userContext      用户数据
 *
 */
typedef void (*OnRequestCallback)(void *pClient, Method method, RequestAck requestAck, const char *pJsonDocument, void *userContext);

/**
 * @brief 设备属性处理回调函数
 *
 * @param pJsonValueBuffer 设备属性值
 * @param valueLength      设备属性值长度
 * @param DeviceProperty   设备属性结构体
 */
typedef void (*OnPropRegCallback)(void *pClient, const char *pJsonValueBuffer, uint32_t valueLength, DeviceProperty *pProperty);

/**
 * @brief 构造ShadowClient
 *
 * @param pInitParams MQTT协议连接接入与连接维持阶段所需要的参数
 *
 * @return 返回NULL: 构造失败
 */
void* IOT_Shadow_Construct(ShadowInitParams *pParams);

/**
 * @brief 发布MQTT消息
 *
 * @param handle        Shadow客户端结构体
 * @param topicName     主题名
 * @param pParams       发布参数
 * @return < 0  :   表示失败
 *         >= 0 :   返回唯一的packet id 
 */
int IOT_Shadow_Publish(void *handle, char *topicName, PublishParams *pParams);

/**
 * @brief 订阅MQTT消息
 *
 * @param handle        Shadow客户端结构体
 * @param topicName     主题名
 * @param pParams       发布参数
 * @return <  0  :   表示失败
 *         >= 0 :   返回唯一的packet id
 */
int IOT_Shadow_Subscribe(void *handle, char *topicFilter, SubscribeParams *pParams);

/**
 * @brief 取消订阅MQTT消息
 *
 * @param handle        Shadow客户端结构体
 * @param topicName     主题名
 * @return <  0  :   表示失败
 *         >= 0 :   返回唯一的packet id
 */
int IOT_Shadow_Unsubscribe(void *handle, char *topicFilter);

/**
 * @brief 客户端目前是否已连接
 *
 * @param pClient Shadow Client结构体
 * @return 返回true, 表示客户端已连接
 */
bool IOT_Shadow_IsConnected(void *handle);

/**
 * @brief 销毁ShadowClient 关闭MQTT连接
 *
 * @param pClient ShadowClient对象
 *
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int IOT_Shadow_Destroy(void *handle);

/**
 * @brief 消息接收, 心跳包管理, 超时请求处理
 *
 * @param timeout_ms 超时时间, 单位:ms
 * @return           返回QCLOUD_ERR_SUCCESS, 表示调用成功
 */
int IOT_Shadow_Yield(void *handle, uint32_t timeout_ms);

/**
 * @brief 异步方式更新设备影子文档
 *
 * @param pClient           Client结构体
 * @param pJsonDoc          更新到云端的设备文档
 * @param sizeOfBuffer      文档长度
 * @param callback          请求响应处理回调函数
 * @param userContext       用户数据, 请求响应返回时通过回调函数返回
 * @param timeout_ms        请求超时时间, 单位:ms
 * @return                  返回QCLOUD_ERR_SUCCESS, 表示请求成功
 */
int IOT_Shadow_Update(void *handle, char *pJsonDoc, size_t sizeOfBuffer, OnRequestCallback callback, void *userContext, uint32_t timeout_ms);

/**
 * @brief 同步方式更新设备影子文档
 *
 * @param pClient           Client结构体
 * @param pJsonDoc          更新到云端的设备文档
 * @param sizeOfBuffer      文档长度
 * @param timeout_ms        请求超时时间, 单位:ms
 * @return                  QCLOUD_ERR_SUCCESS 请求成功
 *                          QCLOUD_ERR_SHADOW_UPDATE_TIMEOUT 请求超时
 *                          QCLOUD_ERR_SHADOW_UPDATE_REJECTED 请求被拒绝
 */
int IOT_Shadow_Update_Sync(void *handle, char *pJsonDoc, size_t sizeOfBuffer, uint32_t timeout_ms);

/**
 * @brief 获取设备影子文档
 *
 * @param pClient           Client结构体
 * @param callback          请求响应处理回调函数
 * @param userContext       用户数据, 请求响应返回时通过回调函数返回
 * @param timeout_ms        请求超时时间, 单位:s
 * @return                  返回QCLOUD_ERR_SUCCESS, 表示请求成功
 */
int IOT_Shadow_Get(void *handle, OnRequestCallback callback, void *userContext, uint32_t timeout_ms);

/**
 * @brief 获取设备影子文档
 *
 * @param pClient           Client结构体
 * @param timeout_ms        请求超时时间, 单位:s
 * @return                  QCLOUD_ERR_SUCCESS 请求成功
 *                          QCLOUD_ERR_SHADOW_GET_TIMEOUT 请求超时
 *                          QCLOUD_ERR_SHADOW_GET_REJECTED 请求被拒绝
 */
int IOT_Shadow_Get_Sync(void *handle, uint32_t timeout_ms);

/**
 * @brief 注册当前设备的设备属性
 *
 * @param pClient    Client结构体
 * @param pProperty  设备属性
 * @param callback   设备属性更新回调处理函数
 * @return           返回QCLOUD_ERR_SUCCESS, 表示请求成功
 */
int IOT_Shadow_Register_Property(void *handle, DeviceProperty *pProperty, OnPropRegCallback callback);

/**
 * @brief 删除已经注册过的设备属性
 *
 * @param pClient    Client结构体
 * @param pProperty  设备属性
 * @return           返回QCLOUD_ERR_SUCCESS, 表示请求成功
 */
int IOT_Shadow_UnRegister_Property(void *handle, DeviceProperty *pProperty);

/**
 * @brief 在JSON文档中添加reported字段，不覆盖更新
 *
 *
 * @param jsonBuffer    为存储JSON文档准备的字符串缓冲区
 * @param sizeOfBuffer  缓冲区大小
 * @param count         可变参数的个数, 即需上报的设备属性的个数
 * @return              返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int IOT_Shadow_JSON_ConstructReport(void *handle, char *jsonBuffer, size_t sizeOfBuffer, uint8_t count, ...);



/**
 * @brief 在JSON文档中添加reported字段，不覆盖更新
 *
 *
 * @param jsonBuffer    为存储JSON文档准备的字符串缓冲区
 * @param sizeOfBuffer  缓冲区大小
 * @param count         需上报的设备属性的个数
 * @param pDeviceProperties         需上报的设备属性的个数
 * @return              返回QCLOUD_ERR_SUCCESS, 表示成功
 */

int IOT_Shadow_JSON_ConstructReportArray(void *handle, char *jsonBuffer, size_t sizeOfBuffer, uint8_t count, DeviceProperty *pDeviceProperties[]);


/**
 * @brief 在JSON文档中添加reported字段，覆盖更新
 *
 *
 * @param jsonBuffer    为存储JSON文档准备的字符串缓冲区
 * @param sizeOfBuffer  缓冲区大小
 * @param overwrite		重写字段
 * @param count         可变参数的个数, 即需上报的设备属性的个数
 * @return              返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int IOT_Shadow_JSON_Construct_OverwriteReport(void *handle, char *jsonBuffer, size_t sizeOfBuffer, uint8_t count, ...);

/**
 * @brief 在JSON文档中添加reported字段，同时清空desired字段
 *
 *
 * @param jsonBuffer    为存储JSON文档准备的字符串缓冲区
 * @param sizeOfBuffer  缓冲区大小
 * @param count         可变参数的个数, 即需上报的设备属性的个数
 * @return              返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int IOT_Shadow_JSON_ConstructReportAndDesireAllNull(void *handle, char *jsonBuffer, size_t sizeOfBuffer, uint8_t count, ...);

/**
 * @brief 在JSON文档中添加 "desired": null 字段
 *
 * @param jsonBuffer   为存储JSON文档准备的字符串缓冲区
 * @param sizeOfBuffer  缓冲区大小
 */
int IOT_Shadow_JSON_ConstructDesireAllNull(void *handle, char *jsonBuffer, size_t sizeOfBuffer);

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_SHADOW_H_ */
