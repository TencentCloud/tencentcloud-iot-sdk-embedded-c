/*
 * iot_client.c
 *
 *  Created on: 2017年11月2日
 *      Author: shockcao
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "shadow_client.h"

#include <stdlib.h>
#include <stdio.h>

#include "qcloud_iot_sdk_impl_internal.h"

static void _init_request_params(RequestParams *pParams, Method method, OnRequestCallback callback, void *pUserdata, uint8_t timeout_sec) {
	pParams->method 		=		method;
	pParams->user_data 	= 		pUserdata;
	pParams->timeout_sec 	= 		timeout_sec;
	pParams->request_callback 	= 		callback;
}

static int _shadow_subcribe_delete(Qcloud_IoT_Client* iot_client, ShadowInitParams *pParams)
{
	char deleteAcceptTopic[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
	SubscribeParams subParams = DEFAULT_SUB_PARAMS;

	stiching_shadow_topic(deleteAcceptTopic, sizeof(deleteAcceptTopic), "delete/accepted");

	subParams.qos = QOS0;
	subParams.on_message_handler = pParams->onDocumentDelete;
	subParams.pUserdata = pParams->mqtt.device_name;
	return IOT_MQTT_Subscribe(iot_client, deleteAcceptTopic, &subParams);
}

static int _shadow_subcribe_update_document(Qcloud_IoT_Client* iot_client, OnMessageHandler callback)
{
	char updateDocTopic[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
	SubscribeParams subParams = DEFAULT_SUB_PARAMS;

	stiching_shadow_topic(updateDocTopic, sizeof(updateDocTopic), "update/documents");
	subParams.qos = QOS0;
	subParams.on_message_handler = callback;

	int rc = qcloud_iot_mqtt_subscribe(iot_client, updateDocTopic, &subParams);

	IOT_FUNC_EXIT_RC(rc);
}

void* IOT_Shadow_Construct(ShadowInitParams *pParams)
{
	POINTER_SANITY_CHECK(pParams, NULL);

	Qcloud_IoT_Client *iot_client = NULL;

	// 初始化Qcloud_IoT_Client
	if ((iot_client = IOT_MQTT_Construct(&pParams->mqtt)) == NULL) {
		goto End;
	}

	iot_shadow_reset_document_version();
	init_shadow_delta();	//初始化delta
	init_request_manager((Qcloud_IoT_Client*)iot_client);

	// 订阅本设备云端文档被删除的消息
	if (pParams->onDocumentDelete != NULL) {
		int rc = _shadow_subcribe_delete(iot_client, pParams);
		if (rc != QCLOUD_ERR_SUCCESS) {
			goto End;
		}
	}

	return iot_client;

End:
	return NULL;
}

bool IOT_Shadow_IsConnected(void *pClient)
{
	POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

	Qcloud_IoT_Client* iot_client = (Qcloud_IoT_Client*)pClient;

	return IOT_MQTT_IsConnected(iot_client);
}

int IOT_Shadow_Destroy(void *pClient)
{
	POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

	reset_shadow_delta(pClient);
	reset_requset_manager(pClient);

	IOT_MQTT_Destroy(&pClient);

	return QCLOUD_ERR_SUCCESS;
}

int IOT_Shadow_Yield(void *pClient, uint32_t timeout_ms) {

    IOT_FUNC_ENTRY;
    int rc;

	POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
	NUMBERIC_SANITY_CHECK(timeout_ms, QCLOUD_ERR_INVAL);

    handle_expired_request();
    rc = IOT_MQTT_Yield(pClient, timeout_ms);
    IOT_FUNC_EXIT_RC(rc);
}

int IOT_Shadow_Register_Update_Documents(void *pClient, OnMessageHandler callback) {

    IOT_FUNC_ENTRY;

    POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
    POINTER_SANITY_CHECK(callback, QCLOUD_ERR_INVAL);

    Qcloud_IoT_Client* iot_client = (Qcloud_IoT_Client*)pClient;

    return _shadow_subcribe_update_document(iot_client, callback);
}

int IOT_Shadow_Register_Property(void *pClient, DeviceProperty *pProperty, OnDeviceDropertyCallback callback) {

    IOT_FUNC_ENTRY;
    int rc;

	POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);

    if (IOT_MQTT_IsConnected(pClient) == false) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_NO_CONN);
    }

    rc = register_property_on_delta(pClient, pProperty, callback);

    IOT_FUNC_EXIT_RC(rc);
}

int IOT_Shadow_Update(void *pClient, char *pJsonDoc, OnRequestCallback callback, void *pUserdata, uint8_t timeout_sec) {

    IOT_FUNC_ENTRY;
	int rc;

	POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(callback, QCLOUD_ERR_INVAL);
	NUMBERIC_SANITY_CHECK(timeout_sec, QCLOUD_ERR_INVAL);

	if (IOT_MQTT_IsConnected(pClient) == false) {
		IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_NO_CONN);
	}

	Log_d("UPDATE Request Document: %s", pJsonDoc);

	RequestParams requestParams = DefaultRequestParams;
	_init_request_params(&requestParams, UPDATE, callback, pUserdata, timeout_sec);

	rc = do_shadow_request(pClient, &requestParams, pJsonDoc);
	IOT_FUNC_EXIT_RC(rc);
}

int IOT_Shadow_Get(void *pClient, OnRequestCallback callback, void *pUserdata, uint8_t timeout_sec) {

    IOT_FUNC_ENTRY;
	int rc;

	POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(callback, QCLOUD_ERR_INVAL);
	NUMBERIC_SANITY_CHECK(timeout_sec, QCLOUD_ERR_INVAL);

	if (IOT_MQTT_IsConnected(pClient) == false) {
		IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_NO_CONN);
	}

	char getRequestJsonDoc[MAX_SIZE_OF_JSON_WITH_CLIENT_TOKEN];
	build_empty_json(getRequestJsonDoc);
	Log_i("GET Request Document: %s", getRequestJsonDoc);

	RequestParams requestParams = DefaultRequestParams;
	_init_request_params(&requestParams, GET, callback, pUserdata, timeout_sec);

	rc = do_shadow_request(pClient, &requestParams, getRequestJsonDoc);
	IOT_FUNC_EXIT_RC(rc);
}

int IOT_Shadow_Delete(void *pClient, OnRequestCallback callback, void *pUserdata, uint8_t timeout_sec) {

    IOT_FUNC_ENTRY;
    int rc;

	POINTER_SANITY_CHECK(pClient, QCLOUD_ERR_INVAL);
	POINTER_SANITY_CHECK(callback, QCLOUD_ERR_INVAL);
	NUMBERIC_SANITY_CHECK(timeout_sec, QCLOUD_ERR_INVAL);

    if (IOT_MQTT_IsConnected(pClient) == false) {
        IOT_FUNC_EXIT_RC(QCLOUD_ERR_MQTT_NO_CONN);
    }

    char deleteRequestJsonDoc[MAX_SIZE_OF_JSON_WITH_CLIENT_TOKEN];
    build_empty_json(deleteRequestJsonDoc);
    Log_i("DELETE Request Document: %s", deleteRequestJsonDoc);

    RequestParams requestParams = DefaultRequestParams;
    _init_request_params(&requestParams, DELETE, callback, pUserdata, timeout_sec);

    rc = do_shadow_request(pClient, &requestParams, deleteRequestJsonDoc);
    IOT_FUNC_EXIT_RC(rc);
}

void IOT_Shadow_Discard_Old_Delta(bool enable) {
	discard_old_delta_flag = enable;
}

void iot_shadow_reset_document_version(void) {
    json_document_version = 0;
}

uint32_t IOT_Shadow_Get_Document_Version(void) {
	return json_document_version;
}

#ifdef __cplusplus
}
#endif

