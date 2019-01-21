
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

#ifndef QCLOUD_IOT_EXPORT_GATEWAY_H_
#define QCLOUD_IOT_EXPORT_GATEWAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "qcloud_iot_export_mqtt.h"

/* 网关 子设备输入参数 */
typedef struct {
	/**
	 * 网关设备基础信息
	*/
	char 						*product_id;			// 产品名称
	char 						*device_name;			// 设备名称
	/**
	 * 子设备基础信息
	*/
	char 						*subdev_product_id;			// 子产品名称
	char 						*subdev_device_name;			// 子设备名称

} GatewayParam;

#define DEFAULT_GATEWAY_PARAMS {NULL, NULL, NULL, NULL}

/**
 * @brief 定义了函数指针的数据类型. 当网关事件发生时，将调用这种类型的函数.
 *
 * @param context, the program context
 * @param client, the gateway client
 * @param msg, the event message.
 *
 * @return none
 */
typedef void (*GatewayEventHandleFun)(void *client, void *context, void *msg);


/* The structure of gateway init param */
typedef struct {
    MQTTInitParams 		init_param; 	/* MQTT params */
    void 			*event_context; /* the user context */
    GatewayEventHandleFun 	event_handler; 	/* event handler for gateway user*/
} GatewayInitParam;

#define DEFAULT_GATEWAY_INIT_PARAMS { DEFAULT_MQTTINIT_PARAMS, NULL, NULL}


/**
 * @brief 构造Gateway client 
 *
 * @param init_param Gateway MQTT协议连接接入与连接维持阶段所需要的参数
 *
 * @return 返回NULL: 构造失败
 */
void *IOT_Gateway_Construct(GatewayInitParam* init_param);

/**
 * @brief 销毁Gateway client 关闭MQTT连接 
 *
 * @param Gateway client对象 
 *
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功 
 */
int IOT_Gateway_Destroy(void *client);

/**
 * @brief 子设备上线 
 *
 * @param Gateway client对象 
 * @param param 网关子设备参数
 *
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int IOT_Gateway_Subdev_Online(void *client, GatewayParam* param);

/**
 * @brief 子设备下线 
 *
 * @param Gateway client对象 
 * @param param 网关子设备参数
 *
 * @return 返回QCLOUD_ERR_SUCCESS, 表示成功
 */
int IOT_Gateway_Subdev_Offline(void *client, GatewayParam* param);


/**
 * @brief 发布Gateway MQTT消息
 *
 * @param client Gateway client对象 
 * @param topic_name 主题名
 * @param params 发布参数
 * @return < 0  :   表示失败
 *         >= 0 :   返回唯一的packet id 
 */
int IOT_Gateway_Publish(void *client, char *topic_name, PublishParams *params);


/**
 * @brief 订阅Gateway MQTT消息
 *
 * @param client Gateway client对象 
 * @param topic_filter 主题过滤器
 * @param params 订阅参数
 * @return < 0  :   表示失败
 *         >= 0 :   返回唯一的packet id
 */
int IOT_Gateway_Subscribe(void *client, char *topic_filter, SubscribeParams *params);


/**
 * @brief 取消订阅Gateway MQTT消息
 *
 * @param client Gateway client对象 
 * @param topic_filter 主题过滤器 
 * @return < 0  :   表示失败
 *         >= 0 :   返回唯一的packet id
 */
int IOT_Gateway_Unsubscribe(void *client, char *topic_filter);



/**
 * @brief 消息接收, 心跳包管理, 超时请求处理
 *
 * @param client Gateway client对象 
 * @param timeout_ms 超时时间, 单位:ms
 * @return 返回QCLOUD_ERR_SUCCESS, 表示调用成功
 */
int IOT_Gateway_Yield(void *client, uint32_t timeout_ms);


#ifdef __cplusplus
}
#endif

#endif /* QCLOUD_IOT_EXPORT_GATEWAY_H_ */
