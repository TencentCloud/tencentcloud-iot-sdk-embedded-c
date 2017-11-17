#ifndef QCLOUD_IOT_EXPORT_SHADOW_H
#define QCLOUD_IOT_EXPORT_SHADOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qcloud_iot_export_mqtt.h"
#include "qcloud_iot_export_shadow_json.h"

typedef struct {
	/**
	 * Shadow依赖MQTT，实际使用MQTT参数进行连接和消息发布订阅
	 */
	MQTTInitParams				mqtt;

    OnMessageHandler 			onDocumentDelete;

} ShadowInitParams;

#define DEFAULT_SHAWDOW_INIT_PARAMS {DEFAULT_MQTTINIT_PARAMS, NULL}

/**
 * @brief 请求响应返回的类型
 */
typedef enum {
    ACK_TIMEOUT,  // 请求超时
    ACK_REJECTED, // 请求拒绝
    ACK_ACCEPTED  // 请求接受
} RequestAck;

/**
 * @brief 操作云端设备文档可以使用的三种方式
 */
typedef enum {
    GET,     // 获取云端设备文档
    UPDATE,  // 更新或创建云端设备文档
    DELETE   // 删除云端设备文档
} Method;

/**
 * @brief 每次文档请求响应的回调函数
 *
 * @param method         文档操作方式
 * @param requestAck     请求响应类型
 * @param pJsonDocument  云端响应返回的文档
 * @param pUserdata      用户数据
 *
 */
typedef void (*OnRequestCallback)(Method method, RequestAck requestAck, const char *pJsonDocument, void *pUserdata);

/**
 * @brief 设备属性处理回调函数
 *
 * @param pJsonValueBuffer 设备属性值
 * @param valueLength      设备属性值长度
 * @param DeviceProperty   设备属性结构体
 */
typedef void (*OnDeviceDropertyCallback)(const char *pJsonValueBuffer, uint32_t valueLength, DeviceProperty *pProperty);

/**
 * @brief 构造ShadowClient
 *
 * @param pInitParams MQTT协议连接接入与连接维持阶段所需要的参数
 *
 * @return 返回NULL: 构造失败
 */
void* IOT_Shadow_Construct(ShadowInitParams *pParams);

/**
 * @brief 客户端目前是否已连接
 *
 * @param pClient Shadow Client结构体
 * @return 返回true, 表示客户端已连接
 */
bool IOT_Shadow_IsConnected(void *pClient);

/**
 * @brief 销毁ShadowClient 关闭MQTT连接
 *
 * @param pClient ShadowClient对象
 *
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int IOT_Shadow_Destroy(void *pClient);

/**
 * @brief 消息接收, 心跳包管理, 超时请求处理
 *
 * @param timeout_ms 超时时间, 单位:ms
 * @return           返回QCLOUD_ERR_SUCCESS, 表示调用成功
 */
int IOT_Shadow_Yield(void *pClient, uint32_t timeout_ms);

/**
 * @brief 更新设备影子文档
 *
 * @param pClient       Client结构体
 * @param pJsonDoc      更新到云端的设备文档
 * @param callback      请求响应处理回调函数
 * @param pUserdata     用户数据, 请求响应返回时通过回调函数返回
 * @param timeout_sec   请求超时时间, 单位:s
 * @return              返回QCLOUD_ERR_SUCCESS, 表示请求成功
 */
int IOT_Shadow_Update(void *pClient, char *pJsonDoc, OnRequestCallback callback, void *pUserdata,
		uint8_t timeout_sec);

/**
 * @brief 获取设备影子文档
 *
 * @param pClient       Client结构体
 * @param callback      请求响应处理回调函数
 * @param pUserdata     用户数据, 请求响应返回时通过回调函数返回
 * @param timeout_sec   请求超时时间, 单位:s
 * @return              返回QCLOUD_ERR_SUCCESS, 表示请求成功
 */
int IOT_Shadow_Get(void *pClient, OnRequestCallback callback, void *pUserdata, uint8_t timeout_sec);

/**
 * @brief 删除设备影子文档
 *
 * @param pClient       Client结构体
 * @param callback      请求响应处理回调函数
 * @param pUserdata     用户数据, 请求响应返回时通过回调函数返回
 * @param timeout_sec   请求超时时间, 单位:s
 * @return              返回QCLOUD_ERR_SUCCESS, 表示请求成功
 */
int IOT_Shadow_Delete(void *pClient, OnRequestCallback callback, void *pUserdata, uint8_t timeout_sec);

/**
 * @brief 订阅设备影子文档更新成功的消息
 *
 * 当设备影子文件更新成功后, 服务器会发布`$shadow/update/documents`消息
 *
 * @param pClient       Client结构体
 * @param callback      消息回调处理函数
 * @return              返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int IOT_Shadow_Register_Update_Documents(void *pClient, OnMessageHandler callback);

/**
 * @brief 注册当前设备的设备属性
 *
 * 如果客户端还未向云端订阅delta消息, 那么首先会向云端订阅该消息; 同时, SDK会保存设备属性,
 * 当云端发送delta消息给客户端时, SDK会检测delta消息中是否存在已登记属性的更新操作
 *
 * @param pClient    Client结构体
 * @param pProperty  设备属性, 例如灯的开关
 * @param callback   设备属性更新回调处理函数
 * @return           返回QCLOUD_ERR_SUCCESS, 表示请求成功
 */
int IOT_Shadow_Register_Property(void *pClient, DeviceProperty *pProperty, OnDeviceDropertyCallback callback);

/**
 * @brief 是否开启废弃旧的delta消息功能
 * @param enable 是否开启
 */
void IOT_Shadow_Discard_Old_Delta(bool enable);

/**
 * @brief 获取本地设备文档版本号
 *
 * @return 文档版本号
 */
uint32_t IOT_Shadow_Get_Document_Version(void);

#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_SHADOW_H */
