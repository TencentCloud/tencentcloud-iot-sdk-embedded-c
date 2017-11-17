/*
 * shadow_client_delta.c
 *
 *  Created on: 2017年11月2日
 *      Author: shockcao
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "shadow_client.h"

#include <string.h>

#include "qcloud_iot_json_utils.h"

/**
 * @brief 保存设备注册属性的回调函数，当收到服务端发来的delta消息的时候
 *        如果消息中包含对应的属性，则进行回调
 */
static PropertyHandler sg_property_handlers[MAX_JSON_TOKEN_EXPECTED];

/**
 * @brief 已经订阅过delta主题
 */
static bool sg_has_subscribed_delta = false;

/**
 * @brief 记录已经注册了多少个(次)属性
 */
static uint32_t sg_registed_attribute_count = 0;

/**
 * @brief 收到的数据流
 */
static char sg_content_received[CLOUD_RX_BUF_LEN];

/**
 * @brief delta 主题名称
 */
static char deltaTopic[MAX_SIZE_OF_CLOUD_TOPIC];

/**
 * 订阅delta相关主题消息回调函数
 * 当服务端发送delta消息到终端, 将会调用此方法
 *
 * @param topicName         终端订阅的topic, 一般携带delta字段
 * @param topicNameLen      topicName长度
 * @param params            消息负载
 * @param pUserdata         用户数据
 */
static void _on_delta_message_handler(char *topicName, uint16_t topicNameLen, MQTTMessage *params, void *pUserdata) {

    int32_t tokenCount;
    uint32_t i = 0;
    int32_t DataPosition;
    uint32_t dataLength;

    if (params == NULL || params->payload_len > CLOUD_RX_BUF_LEN) {
        return;
    }

    memcpy(sg_content_received, params->payload, params->payload_len);
    sg_content_received[params->payload_len] = '\0';

    if (!check_and_parse_json(sg_content_received, &tokenCount, NULL)) {
        Log_w("Received JSON is not valid");
        return;
    }

    if (discard_old_delta_flag) {
        uint32_t version_num = 0;
        if (parse_version_num(sg_content_received, tokenCount, &version_num)) {
            Log_d("topic=%s|version=%d", topicName, version_num);
            if (version_num > json_document_version) {
                json_document_version = version_num;
                Log_d("New Version number: %d", json_document_version);
            } else {
                Log_w("Old Delta Message received - Ignoring rx: %d local: %d", version_num, json_document_version);
                return;
            }
        }
    }

    for (i = 0; i < sg_registed_attribute_count; i++) {
        if (!sg_property_handlers[i].is_free && sg_property_handlers[i].pProperty != NULL) {
            if (update_value_if_key_match(sg_content_received, tokenCount, sg_property_handlers[i].pProperty, &dataLength, &DataPosition))
            {
                if (sg_property_handlers[i].callback != NULL)
                {
                    sg_property_handlers[i].callback(sg_content_received + DataPosition, dataLength, sg_property_handlers[i].pProperty);
                }
            }
        }
    }
}

/**
 * @brief 订阅delta主题，获取设备影子文档注册属性变更
 */
static int _shadow_subcribe_delta(void* iot_client)
{
	stiching_shadow_topic(deltaTopic, sizeof(deltaTopic), "update/delta");

	SubscribeParams subParams = DEFAULT_SUB_PARAMS;
	subParams.qos = QOS0;
	subParams.on_message_handler = _on_delta_message_handler;

    Log_d("subscribe delta topic %s", deltaTopic);
	return IOT_MQTT_Subscribe(iot_client, deltaTopic, &subParams);
}

/**
 * @brief 取消订阅delta主题
 */
static int _shadow_unsubscribe_delta(void* iot_client)
{
	char deltaTopic[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
	stiching_shadow_topic(deltaTopic, sizeof(deltaTopic), "update/delta");
	return IOT_MQTT_Unsubscribe(iot_client, deltaTopic);
}

void init_shadow_delta() {
	uint32_t i;

    for (i = 0; i < MAX_JSON_TOKEN_EXPECTED; i++) {
    	sg_property_handlers[i].callback = NULL;
        sg_property_handlers[i].pProperty = NULL;
        sg_property_handlers[i].is_free = true;
    }

    sg_registed_attribute_count = 0;
    sg_has_subscribed_delta = false;
}

void reset_shadow_delta(void *pClient) {
    uint32_t i;

    _shadow_unsubscribe_delta(pClient);

    // 清空注册过的属性
    for (i = 0; i < sg_registed_attribute_count; i++) {
    	sg_property_handlers[i].callback = NULL;
        sg_property_handlers[i].pProperty = NULL;
        sg_property_handlers[i].is_free = true;
    }
}

int register_property_on_delta(void *pClient, DeviceProperty *pProperty, OnDeviceDropertyCallback callback) {

    IOT_FUNC_ENTRY;
    int rc = QCLOUD_ERR_SUCCESS;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(callback, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(pProperty, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(pProperty->key, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(pProperty->data, QCLOUD_ERR_INVAL);

    // if delta topic not subscribed
    if (!sg_has_subscribed_delta) {
        rc = _shadow_subcribe_delta(pClient);
        if (rc != QCLOUD_ERR_SUCCESS) {
        	IOT_FUNC_EXIT_RC(rc);
        }
        sg_has_subscribed_delta = true;
    }

    if (sg_registed_attribute_count >= MAX_JSON_TOKEN_EXPECTED) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MAX_JSON_TOKEN);
    }

    sg_property_handlers[sg_registed_attribute_count].callback = callback;
    sg_property_handlers[sg_registed_attribute_count].pProperty = pProperty;
    sg_property_handlers[sg_registed_attribute_count].is_free = false;
    sg_registed_attribute_count++;

    IOT_FUNC_EXIT_RC(rc);
}

#ifdef __cplusplus
}
#endif
