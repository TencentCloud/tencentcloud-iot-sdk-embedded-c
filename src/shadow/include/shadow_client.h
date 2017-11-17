/*
 * shadow_client.h
 *
 *  Created on: 2017年11月2日
 *      Author: shockcao
 */

/**
 * @brief 设备影子文档操作相关的一些接口
 *
 * 这里提供一些接口用于管理设备影子文档或与设备影子文档进行交互; 通过DeviceName,
 * 可以与设备影子进行交互, 包括当前设备的设备影子和其他设备的设备影子; 一
 * 个设备一共有三种不同操作与设备影子交互:
 *     1. Get
 *     2. Update
 *     3. Delete
 *
 * 以上三种操作, 底层还是基于MQTT协议, 工作原理也是基于发布/订阅模型, 当执行
 * 上述操作是, 会收到相应的响应: 1. accepted; 2. rejected。例如, 我们执行
 * Get与设备影子进行交互, 设备端将发送和接收到一下信息:
 *     1. 发布MQTT主题: $shadow/get/{productName}/{deviceName};
 *     2. 订阅MQTT主题: $shadow/get/accepted/{productName}/{deviceName} 和 $shadow/get/rejected/{productName}/{deviceName}
 *     3. 如果整个请求成功的话, 设备端会收到accepted主题, 以及相应设备的json文档。
 */
#ifndef SHADOW_CLIENT_H_
#define SHADOW_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "qcloud_iot_sdk_impl_internal.h"
#include "device.h"
#include "mqtt_client.h"
#include "shadow_client_json.h"

/**
 * @brief 文档操作请求的参数结构体定义
 */
typedef struct _RequestParam {

    Method               	method;              // 文档请求方式: GET, UPDATE, DELETE

    uint32_t             	timeout_sec;         // 请求超时时间, 单位:s

    OnRequestCallback    	request_callback;          // 请求回调方法

    void                 	*user_data;          // 用户数据, 会通过回调方法OnRequestCallback返回

} RequestParams;

#define DefaultRequestParams {GET, 4, NULL, NULL};

/**
 * @brief 该结构体用于保存已登记的设备属性及设备属性处理的回调方法
 */
typedef struct {

    void *pProperty;						// 设备属性

    bool is_free;                           // 该结构体是否处于空闲状态, 用遍历数组的时候判断元素是否被赋值

    OnDeviceDropertyCallback callback;      // 回调处理函数

} PropertyHandler;

extern uint32_t json_document_version;
extern bool discard_old_delta_flag;
extern uint32_t client_token_num;

/**
 * @brief 重置本地设备文档版本号
 */
void iot_shadow_reset_document_version(void);

/**
 * @brief 初始化文档请求管理器
 *
 * @param pClient MQTTClient结构体
 */
void init_request_manager();

/**
 * @brief 重置文档请求管理器, MQTT连接断开的时候, 文档请求管理器需要做一些清理工作
 */
void reset_requset_manager(void *pClient);

/**
 * @brief 初始化delta管理器
 */
void init_shadow_delta(void);

/**
 * @brief 重置文档注册字段变更请求，Shadow销毁时
 */
void reset_shadow_delta(void *pClient);

/**
 * @brief 处理请求队列中已经超时的请求
 *
 * 该函数在`qcloud_iot_shadow_yield()`中被调用
 */
void handle_expired_request(void);

/**
 * @brief 如果没有订阅delta主题, 则进行订阅, 并记录相应设备属性
 *
 * @param pProperty 设备属性
 * @param callback  相应设备属性处理回调函数
 * @return          返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int register_property_on_delta(void* pClient, DeviceProperty *pProperty, OnDeviceDropertyCallback callback);

/**
 * @brief 所有的云端设备文档操作请求, 通过该方法进行中转分发
 *
 * @param pParams  请求参数
 * @param pJsonDoc 请求文档
 * @return         返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int do_shadow_request(void *pClient, RequestParams *pParams, char *pJsonDoc);

/**
 * @brief 根据传入的用户自定义 action 和 device_name 拼接完整的Shadow topic
 *        形式： $shadow/update/product_name/device_name，topicFilter"/update"
 *
 * @param topic                 topic字符串容器
 * @param buf_size              字符串容器大小
 * @param action                用户自定义部分
 */
int stiching_shadow_topic(char *topic, int buf_size, const char *action);

#ifdef __cplusplus
}
#endif

#endif /* SHADOW_CLIENT_H_ */
