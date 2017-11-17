#include <qcloud_iot_mqtt_client.h>
#include "qcloud_iot_mqtt_client_interface.h"
#include "iot_test_integration_runner.h"

int disconnectedCount = 0;
char rootCA[PATH_MAX + 1];
char clientCert[PATH_MAX + 1];
char clientKey[PATH_MAX + 1];
Qcloud_IoT_Client client;

void test_auto_reconnect_on_disconnect_handler(void) {
    Log_i("on_disconnect_handler")
    disconnectedCount++;
}

int iot_mqtt_tests_auto_reconnect() {
    int rc = QCLOUD_ERR_SUCCESS;
    char cert_dir[PATH_MAX + 1] = "../../certs";
    char CurrentWD[PATH_MAX + 1];

    getcwd(CurrentWD, sizeof(CurrentWD));
    sprintf(rootCA, "%s/%s/%s", CurrentWD, cert_dir, QCLOUD_IOT_CA_FILENAME);
    sprintf(clientCert, "%s/%s/%s", CurrentWD, cert_dir, QCLOUD_IOT_CERT_FILENAME);
    sprintf(clientKey, "%s/%s/%s", CurrentWD, cert_dir, QCLOUD_IOT_KEY_FILENAME);

    MQTTConnectParams connectParams = DEFAULT_CONNECT_PARAMS;
	connectParams.client_id = QCLOUD_IOT_MQTT_CLIENT_ID;
	connectParams.keep_alive_interval = 5;
    connectParams.ca_file = rootCA;
    connectParams.cert_file = clientCert;
    connectParams.key_file = clientKey;
    connectParams.on_disconnect_handler = test_auto_reconnect_on_disconnect_handler;
    connectParams.auto_connect_enable = 0;  // 关闭自动重连功能

    rc = qcloud_iot_mqtt_init(&client, &connectParams);
    if (rc != QCLOUD_ERR_SUCCESS) {
        Log_e("MQTT Client Init Failed. Error Code: %d", rc);
        return rc;
    }

    rc = qcloud_iot_mqtt_connect(&client, &connectParams);

    if (rc != QCLOUD_ERR_SUCCESS) {
        Log_e("MQTT Client Connect Failed. Error Code: %d", rc);
    }

    // 1. Test Disconnect Handler
    // 这边要依赖后台断开TCP连接, 可靠吗?? sleep time with keep_alive_internal * 1.5
    sleep(5 * 2);

    qcloud_iot_mqtt_yield(&client, 100);
    Log_i("MQTT Client Connect State: %d", qcloud_iot_mqtt_is_connected(&client));
    if (disconnectedCount == 1) {
        Log_i("Success invoking Disconnect Handler.");
    } else {
        Log_e("Failure to invoke Disconnect Handler.");
        return -1;
    }

    // 2. Test Manual Reconnect
    rc = qcloud_iot_mqtt_attempt_reconnect(&client);

    if (rc == QCLOUD_ERR_MQTT_RECONNECTED) {
        Log_i("MQTT Client Manual Reconnect Success.");
    } else {
        Log_e("MQTT Client Manual Reconnect Failed: %d", rc);
        return rc;
    }

    // 3. Test Auto Reconnect
    rc = qcloud_iot_mqtt_set_autoreconnect(&client, true);
    if (rc == QCLOUD_ERR_SUCCESS) {
        Log_i("MQTT Client Auto Reconnect Enable Success.");
    } else {
        Log_e("MQTT Client Auto Reconnect Enable Failed: %d", rc);
        return rc;
    }

    // 这边要依赖后台断开TCP连接, 可靠吗??
    sleep(5 * 2);

    qcloud_iot_mqtt_yield(&client, 100);

    if (disconnectedCount == 2) {
        Log_i("Success invoking Disconnect Handler.");
    } else {
        Log_e("Failure to invoke Disconnect Handler.");
        return -1;
    }

    return rc;
}

