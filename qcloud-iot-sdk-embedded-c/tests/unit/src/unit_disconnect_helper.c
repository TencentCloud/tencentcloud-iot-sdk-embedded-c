#include <stdio.h>
#include <unistd.h>
#include <CppUTest/TestHarness_c.h>
#include <stdbool.h>
#include <qcloud_iot_mqtt_client_interface.h>
#include <unit_helper_functions.h>

static MQTTConnectParams connectParams;
static Qcloud_IoT_Client iotClient;

static bool handlerInvoked = false;

void disconnectTestHandler(void) {
	handlerInvoked = true;
}

TEST_GROUP_C_SETUP(DisconnectTests) {
	int rc;
	resetTlsBuffer();
	MQTTInitParamsSetup(&connectParams, QCLOUD_IOT_MQTT_HOST, QCLOUD_IOT_MQTT_PORT, true, disconnectTestHandler);
	rc = qcloud_iot_mqtt_init(&iotClient, &connectParams);
	CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

	ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
	connectParams.keep_alive_interval = 5;
	setTLSRxBufferForConnack(&connectParams, 0, 0);
	rc = qcloud_iot_mqtt_connect(&iotClient, &connectParams);
	CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

	resetTlsBuffer();
}

TEST_GROUP_C_TEARDOWN(DisconnectTests) { }

/* F:1 - Disconnect with Null/empty client instance */
TEST_C(DisconnectTests, NullClientDisconnect) {
	int rc = qcloud_iot_mqtt_disconnect(NULL);
	CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/* F:2 - Set Disconnect Handler with Null/empty Client */
TEST_C(DisconnectTests, NullClientSetDisconnectHandler) {
	int rc = qcloud_iot_mqtt_set_disconnect_handler(NULL, disconnectTestHandler);
	CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/* F:3 - Call Set Disconnect handler with Null handler */
TEST_C(DisconnectTests, SetDisconnectHandlerNullHandler) {
	int rc = qcloud_iot_mqtt_set_disconnect_handler(&iotClient, NULL);
	CHECK_EQUAL_C_INT(QCLOUD_ERR_INVAL, rc);
}

/* F:4 - Disconnect attempt, not connected */
TEST_C(DisconnectTests, disconnectNotConnected) {
	int rc = QCLOUD_ERR_SUCCESS;

	/* First make sure client is disconnected */
	rc = qcloud_iot_mqtt_disconnect(&iotClient);

	/* Check client is disconnected */
	CHECK_EQUAL_C_INT(false, qcloud_iot_mqtt_is_connected(&iotClient));

	/* Now call disconnect again */
	rc = qcloud_iot_mqtt_disconnect(&iotClient);
	CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_NO_CONN, rc);
}

/* F:5 - Disconnect success */
TEST_C(DisconnectTests, disconnectNoAckSuccess) {
	int rc = QCLOUD_ERR_SUCCESS;
	rc = qcloud_iot_mqtt_disconnect(&iotClient);
	CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
}

/* F:6 - Disconnect, Handler invoked on disconnect */
TEST_C(DisconnectTests, HandlerInvokedOnDisconnect) {
	bool connected = false;
	bool currentAutoReconnectStatus = false;
	int i;
	int j;
	int attempt = 3;
    int dcCount = 0;
	int rc = QCLOUD_ERR_SUCCESS;

	handlerInvoked = false;

	currentAutoReconnectStatus = qcloud_iot_mqtt_is_autoreconnect_enabled(&iotClient);

	connected = qcloud_iot_mqtt_is_connected(&iotClient);
	CHECK_EQUAL_C_INT(1, connected);

	qcloud_iot_mqtt_set_autoreconnect(&iotClient, false);

	// 3 cycles of half keep alive time expiring
	// verify a ping request is sent and give a ping response
	for(i = 0; i < attempt; i++) {
		/* Set TLS buffer for ping response */
		resetTlsBuffer();
		setTLSRxBufferForPingresp();
		for(j = 0; j <= connectParams.keep_alive_interval; j++) {
			sleep(1);
			rc = qcloud_iot_mqtt_yield(&iotClient, 100);
			CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
		}
		CHECK_EQUAL_C_INT(1, isLastTLSTxMessagePingreq());
	}

	// keepalive() waits for 1/2 of _mqtt_keep_alive time after sending ping request
	// to receive a pingresponse before determining the connection is not alive
	// wait for _mqtt_keep_alive time and then yield()
	sleep(connectParams.keep_alive_interval);
	rc = qcloud_iot_mqtt_yield(&iotClient, 100);
	CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_NO_CONN, rc);
	CHECK_EQUAL_C_INT(1, isLastTLSTxMessageDisconnect());

	connected = qcloud_iot_mqtt_is_connected(&iotClient);
	CHECK_EQUAL_C_INT(0, connected);

	CHECK_EQUAL_C_INT(true, handlerInvoked);

	dcCount = qcloud_iot_mqtt_get_network_disconnected_count(&iotClient);
	CHECK_C(1 == dcCount);

    qcloud_iot_mqtt_reset_network_disconnected_count(&iotClient);

	dcCount = qcloud_iot_mqtt_get_network_disconnected_count(&iotClient);
	CHECK_C(0 == dcCount);

	resetTlsBuffer();
	qcloud_iot_mqtt_set_autoreconnect(&iotClient, currentAutoReconnectStatus);
}


/* F:7 - Disconnect, with set handler and invoked on disconnect */
TEST_C(DisconnectTests, SetHandlerAndInvokedOnDisconnect) {
	bool connected = false;
	bool currentAutoReconnectStatus = false;
	int i;
	int j;
	int attempt = 3;
	int dcCount = 0;
	int rc = QCLOUD_ERR_SUCCESS;

	handlerInvoked = false;
	MQTTInitParamsSetup(&connectParams, "localhost", 8883, false, NULL);
	rc = qcloud_iot_mqtt_init(&iotClient, &connectParams);
	CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

	ConnectParamsSetup(&connectParams, QCLOUD_IOT_MQTT_CLIENT_ID);
	connectParams.keep_alive_interval = 5;
	setTLSRxBufferForConnack(&connectParams, 0, 0);
	rc = qcloud_iot_mqtt_connect(&iotClient, &connectParams);
	CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);

	qcloud_iot_mqtt_set_disconnect_handler(&iotClient, disconnectTestHandler);
    qcloud_iot_mqtt_set_autoreconnect(&iotClient, true);

	currentAutoReconnectStatus = qcloud_iot_mqtt_is_autoreconnect_enabled(&iotClient);

	connected = qcloud_iot_mqtt_is_connected(&iotClient);
	CHECK_EQUAL_C_INT(1, connected);

	qcloud_iot_mqtt_set_autoreconnect(&iotClient, false);

	// 3 cycles of keep alive time expiring
	// verify a ping request is sent and give a ping response
	for(i = 0; i < attempt; i++) {
		/* Set TLS buffer for ping response */
		resetTlsBuffer();
		setTLSRxBufferForPingresp();
		for(j = 0; j <= connectParams.keep_alive_interval; j++) {
			sleep(1);
			rc = qcloud_iot_mqtt_yield(&iotClient, 100);
			CHECK_EQUAL_C_INT(QCLOUD_ERR_SUCCESS, rc);
		}
		CHECK_EQUAL_C_INT(1, isLastTLSTxMessagePingreq());
	}
	resetTlsBuffer();

	// keepalive() waits for 1/2 of _mqtt_keep_alive time after sending ping request
	// to receive a pingresponse before determining the connection is not alive
	// wait for _mqtt_keep_alive time and then yield()
	sleep(connectParams.keep_alive_interval);
	rc = qcloud_iot_mqtt_yield(&iotClient, 100);
	CHECK_EQUAL_C_INT(QCLOUD_ERR_MQTT_NO_CONN, rc);
	CHECK_EQUAL_C_INT(1, isLastTLSTxMessageDisconnect());

	connected = qcloud_iot_mqtt_is_connected(&iotClient);
	CHECK_EQUAL_C_INT(0, connected);

	CHECK_EQUAL_C_INT(true, handlerInvoked);

	dcCount = qcloud_iot_mqtt_get_network_disconnected_count(&iotClient);
	CHECK_C(1 == dcCount);

    qcloud_iot_mqtt_reset_network_disconnected_count(&iotClient);

	dcCount = qcloud_iot_mqtt_get_network_disconnected_count(&iotClient);
	CHECK_C(0 == dcCount);

	resetTlsBuffer();
	qcloud_iot_mqtt_set_autoreconnect(&iotClient, currentAutoReconnectStatus);
}
