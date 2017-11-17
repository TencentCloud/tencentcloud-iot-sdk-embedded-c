#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "mqtt_client.h"

#include "qcloud_iot_utils_base64.h"
#include "qcloud_iot_import.h"
#include "device.h"

#define DECODE_STR_LENGTH 32
static unsigned char dst_str[DECODE_STR_LENGTH];
    
static char *s_qcloud_iot_host = "connect.iot.qcloud.com";
static int s_qcloud_iot_port = 8883;

void* IOT_MQTT_Construct(MQTTInitParams *pParams)
{
	POINTER_SANITY_CHECK(pParams, NULL);
	STRING_PTR_SANITY_CHECK(pParams->product_name, NULL);
	STRING_PTR_SANITY_CHECK(pParams->device_name, NULL);
	STRING_PTR_SANITY_CHECK(pParams->client_id, NULL);

	iot_device_info_init();
	if (iot_device_info_set(pParams->product_name, pParams->device_name, pParams->client_id) != QCLOUD_ERR_SUCCESS)
    {
        Log_e("faile to set device info!");
        return NULL;
    }

	Qcloud_IoT_Client* mqtt_client = NULL;

	// 初始化MQTTClient
	if ((mqtt_client = (Qcloud_IoT_Client*) HAL_Malloc (sizeof(Qcloud_IoT_Client))) == NULL) {
		Log_e("memory not enough to malloc MQTTClient");
		return NULL;
	}

	int rc = qcloud_iot_mqtt_init(mqtt_client, pParams);
	if (rc != QCLOUD_ERR_SUCCESS) {
		Log_e("mqtt init failed: %d", rc);
		HAL_Free(mqtt_client);
		return NULL;
	}

	MQTTConnectParams connect_params = DEFAULT_MQTTCONNECT_PARAMS;
	connect_params.client_id = pParams->client_id;
	connect_params.username = pParams->username;
	connect_params.password = pParams->password;
	connect_params.keep_alive_interval = pParams->keep_alive_interval;
	connect_params.clean_session = pParams->clean_session;
	connect_params.auto_connect_enable = pParams->auto_connect_enable;
	connect_params.on_disconnect_handler = pParams->on_disconnect_handler;

	rc = qcloud_iot_mqtt_connect(mqtt_client, &connect_params);
	if (rc != QCLOUD_ERR_SUCCESS) {
		Log_e("mqtt connect failed: %d", rc);
		HAL_Free(mqtt_client);
		return NULL;
	}

	return mqtt_client;
}

int IOT_MQTT_Destroy(void **pClient) {
	POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

	Qcloud_IoT_Client   *mqtt_client = (Qcloud_IoT_Client *)*pClient;

	int rc = qcloud_iot_mqtt_disconnect(mqtt_client);
	mqtt_client = NULL;

    HAL_Free(*pClient);
    *pClient = NULL;

	return rc;
}

int IOT_MQTT_Yield(void *pClient, uint32_t timeout_ms) {

	Qcloud_IoT_Client   *mqtt_client = (Qcloud_IoT_Client *)pClient;

	return qcloud_iot_mqtt_yield(mqtt_client, timeout_ms);
}

int IOT_MQTT_Publish(void *pClient, char *topicName, PublishParams *pParams) {

	Qcloud_IoT_Client   *mqtt_client = (Qcloud_IoT_Client *)pClient;

	return qcloud_iot_mqtt_publish(mqtt_client, topicName, pParams);
}

int IOT_MQTT_Subscribe(void *pClient, char *topicFilter, SubscribeParams *pParams) {

	Qcloud_IoT_Client   *mqtt_client = (Qcloud_IoT_Client *)pClient;

	return qcloud_iot_mqtt_subscribe(mqtt_client, topicFilter, pParams);
}

int IOT_MQTT_Unsubscribe(void *pClient, char *topicFilter) {

	Qcloud_IoT_Client   *mqtt_client = (Qcloud_IoT_Client *)pClient;

	return qcloud_iot_mqtt_unsubscribe(mqtt_client, topicFilter);
}

bool IOT_MQTT_IsConnected(void *pClient) {
    bool is_connected = false;

    IOT_FUNC_ENTRY;

    if (pClient == NULL) {
        IOT_FUNC_EXIT_RC(is_connected);
    }

    Qcloud_IoT_Client   *mqtt_client = (Qcloud_IoT_Client *)pClient;

    if (mqtt_client->is_connected == 1) {
        is_connected = true;
    }

    IOT_FUNC_EXIT_RC(is_connected);
}

int qcloud_iot_mqtt_init(Qcloud_IoT_Client *pClient, MQTTInitParams *pParams) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(pParams, QCLOUD_ERR_INVAL);

    Log_d("client_id: %s", pParams->client_id);
    Log_d("product_name: %s", pParams->product_name);
    Log_d("device_name: %s", pParams->device_name);
	memset(pClient, 0x0, sizeof(Qcloud_IoT_Client));
	if (pParams->is_asymc_encryption) {
	    bool certEmpty = (pParams->ca_file == NULL || pParams->cert_file == NULL || pParams->key_file == NULL);
	    if (certEmpty) {
	    	IOT_FUNC_EXIT_RC(QCLOUD_ERR_INVAL);
	    }
	    Log_d("ca file: %s", pParams->ca_file);
	    Log_d("cert file: %s", pParams->cert_file);
	    Log_d("key file: %s", pParams->key_file);
	}
	else {
	    bool pskEmpty = (pParams->psk == NULL);
	    if (pskEmpty) {
	    	IOT_FUNC_EXIT_RC(QCLOUD_ERR_INVAL);
	    }
	    Log_d("psk: %s", pParams->psk);
	}

    int i = 0;
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
        pClient->message_handlers[i].topicFilter = NULL;
        pClient->message_handlers[i].messageHandler = NULL;
        pClient->message_handlers[i].qos = QOS0;
        pClient->message_handlers[i].pMessageHandlerData = NULL;
    }

    pClient->command_timeout_ms = pParams->command_timeout;

    pClient->next_packetId = 1;
    pClient->buf_size = QCLOUD_IOT_MQTT_TX_BUF_LEN;
    pClient->read_buf_size = QCLOUD_IOT_MQTT_RX_BUF_LEN;
    pClient->is_connected = 0;
    pClient->is_ping_outstanding = 0;
    pClient->was_manually_disconnected = 0;
    pClient->counter_network_disconnected = 0;
    
    // TLS连接参数初始化
    pClient->network_stack.tlsConnectParams.is_asymc_encryption = pParams->is_asymc_encryption;
    pClient->network_stack.tlsConnectParams.ca_file = pParams->ca_file;
    pClient->network_stack.tlsConnectParams.cert_file = pParams->cert_file;
    pClient->network_stack.tlsConnectParams.key_file = pParams->key_file;

    pClient->network_stack.tlsConnectParams.psk_id = pParams->client_id;
    // psk
    if (pParams->psk != NULL) {
        size_t src_len = strlen(pParams->psk);
        size_t len;
        memset(dst_str, 0x00, DECODE_STR_LENGTH);
        qcloud_iot_utils_base64decode(dst_str, sizeof( dst_str ), &len, (unsigned char *)pParams->psk, src_len );
        pClient->network_stack.tlsConnectParams.psk = (char *)dst_str;
        pClient->network_stack.tlsConnectParams.psk_length = len;
    }

    pClient->network_stack.tlsConnectParams.host = s_qcloud_iot_host;
    pClient->network_stack.tlsConnectParams.port = s_qcloud_iot_port;
    pClient->network_stack.tlsConnectParams.timeout_ms = QCLOUD_IOT_TLS_HANDSHAKE_TIMEOUT;

    // 底层网络操作相关的数据结构初始化
    qcloud_iot_mqtt_network_init(&(pClient->network_stack));

    // ping定时器以及重连延迟定时器相关初始化
    InitTimer(&(pClient->ping_timer));
    InitTimer(&(pClient->reconnect_delay_timer));

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

int qcloud_iot_mqtt_set_disconnect_handler(Qcloud_IoT_Client *pClient, OnDisconnectHandler on_disconnect_handler) {
    IOT_FUNC_ENTRY;

    if (pClient == NULL) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_INVAL);
    }

    pClient->options.on_disconnect_handler = on_disconnect_handler;
    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

int qcloud_iot_mqtt_set_autoreconnect(Qcloud_IoT_Client *pClient, bool value) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

    pClient->options.auto_connect_enable = (uint8_t) value;

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

bool qcloud_iot_mqtt_is_autoreconnect_enabled(Qcloud_IoT_Client *pClient) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

    bool is_enabled = false;
    if (pClient->options.auto_connect_enable == 1) {
        is_enabled = true;
    }

    IOT_FUNC_EXIT_RC(is_enabled);
}

int qcloud_iot_mqtt_get_network_disconnected_count(Qcloud_IoT_Client *pClient) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

    IOT_FUNC_EXIT_RC(pClient->counter_network_disconnected);
}

int qcloud_iot_mqtt_reset_network_disconnected_count(Qcloud_IoT_Client *pClient) {
    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

    pClient->counter_network_disconnected = 0;

    IOT_FUNC_EXIT_RC(QCLOUD_ERR_SUCCESS);
}

#ifdef __cplusplus
}
#endif
