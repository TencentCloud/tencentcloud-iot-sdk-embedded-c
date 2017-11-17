#include <qcloud_iot_mqtt_client.h>
#include "qcloud_iot_mqtt_client_interface.h"
#include "iot_test_integration_runner.h"

char rootCA[PATH_MAX + 1];
char clientCert[PATH_MAX + 1];
char clientKey[PATH_MAX + 1];
int countArray[100];
char payload[100];

Qcloud_IoT_Client clientPub;
Qcloud_IoT_Client clientSub;

/**
 * 订阅消息接收处理回调函数
 *
 * @param iotMessage
 */
static void on_message_callback(char *topicName, size_t topicNameLen, MQTTMessage *message, void *pUserdata) {
    char tempBuf[100];
    char *temp = NULL;
    int id = 0;

    snprintf(tempBuf, message->payload_len, message->payload);
    Log_i("Client Received Message: %s", tempBuf);
    temp = strtok(tempBuf, ":");
    temp = strtok(NULL, " ");
    if (NULL == temp) {
        return;
    }

    id = atoi(temp);

    if (id >= 0 && id < PUBLISH_COUNT) {
        countArray[id]++;
    }
}

/**
 * 订阅预定的测试Topic
 *
 * @param pClient
 * @param qos             服务质量级别
 * @param pSubscribeTime  定时器用于计算订阅Topic所耗费时间
 * @return
 */
static int _subscribe_to_test_topic(Qcloud_IoT_Client *pClient, QoS qos, struct timeval *pSubscribeTime) {
    int rc;
    struct timeval start, end;

    SubscribeParams iotTopic = DEFAULT_SUB_PARAMS;
    iotTopic.qos = qos;
    iotTopic.on_message_handler = on_message_callback;

    gettimeofday(&start, NULL);
    rc = qcloud_iot_mqtt_subscribe(pClient, INTEGRATION_TEST_TOPIC, &iotTopic);
    gettimeofday(&end, NULL);

    timersub(&end, &start, pSubscribeTime);

    return rc;
}

void on_disconnect_handler_multiple_clients() {
}

/**
 * 测试MQTT客户端基本的连接状态
 *
 * @return
 */
int iot_mqtt_tests_multiple_clients() {
    int rc;
    int connect_count = 0;
    int publish_count = 0;
    int i = 0, rxMsgCount = 0;
    struct timeval connectTime, subscribeTime;
    struct timeval start, end;

    for (i = 0; i < PUBLISH_COUNT; i++) {
        countArray[i] = 0;
    }

    char cert_dir[PATH_MAX + 1] = "../../certs";
    char CurrentWD[PATH_MAX + 1];

    getcwd(CurrentWD, sizeof(CurrentWD));
    sprintf(rootCA, "%s/%s/%s", CurrentWD, cert_dir, QCLOUD_IOT_CA_FILENAME);
    sprintf(clientCert, "%s/%s/%s", CurrentWD, cert_dir, QCLOUD_IOT_CERT_FILENAME);
    sprintf(clientKey, "%s/%s/%s", CurrentWD, cert_dir, QCLOUD_IOT_KEY_FILENAME);

    MQTTConnectParams connectParams = DEFAULT_CONNECT_PARAMS;
    connectParams.client_id = "ClientPub";
    connectParams.keep_alive_interval = QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;
    connectParams.ca_file = rootCA;
    connectParams.cert_file = clientCert;
    connectParams.key_file = clientKey;
    connectParams.on_disconnect_handler = on_disconnect_handler_multiple_clients;
    connectParams.auto_connect_enable = 0;  // 关闭自动重连功能

    rc = qcloud_iot_mqtt_init(&clientPub, &connectParams);
    if (rc != QCLOUD_ERR_SUCCESS) {
        Log_e("MQTT ClientPub Init Failed. Error Code: %d", rc);
        return rc;
    }

    do {
        gettimeofday(&start, NULL);
        rc = qcloud_iot_mqtt_connect(&clientPub, &connectParams);
        gettimeofday(&end, NULL);
        timersub(&end, &start, &connectTime);

        connect_count++;
    } while (rc != QCLOUD_ERR_SUCCESS && connect_count < CONNECT_MAX_ATTEMPT_COUNT);

    if (rc == QCLOUD_ERR_SUCCESS) {
        Log_i("ClientPub Connect Success. Time sec: %ld, usec: %d", connectTime.tv_sec, connectTime.tv_usec);
    } else {
        Log_e("ClientPub Connect Failed. Error Code: %d", rc);

        return rc;
    }

    rc = qcloud_iot_mqtt_init(&clientSub, &connectParams);
    if (rc != QCLOUD_ERR_SUCCESS) {
        Log_e("MQTT ClientSub Init Failed. Error Code: %d", rc);
        return rc;
    }

    do {
        MQTTConnectParams connectParams = DEFAULT_CONNECT_PARAMS;
        connectParams.client_id = "ClientSub";
        connectParams.keep_alive_interval = QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;

        gettimeofday(&start, NULL);
        rc = qcloud_iot_mqtt_connect(&clientSub, &connectParams);
        gettimeofday(&end, NULL);
        timersub(&end, &start, &connectTime);

        connect_count++;
    } while (rc != QCLOUD_ERR_SUCCESS && connect_count < CONNECT_MAX_ATTEMPT_COUNT);

    if (rc == QCLOUD_ERR_SUCCESS) {
        Log_i("ClientSub Connect Success. Time sec: %ld, usec: %d", connectTime.tv_sec, connectTime.tv_usec);
    } else {
        Log_e("ClientSub Connect Failed. Error Code: %d", rc);

        return rc;
    }

    rc = _subscribe_to_test_topic(&clientSub, QOS1, &subscribeTime);

    if (rc == QCLOUD_ERR_SUCCESS) {
        Log_i("ClientSub subscribe Success. Time sec: %ld, usec: %d", subscribeTime.tv_sec, subscribeTime.tv_usec);
    } else {
        Log_e("ClientSub Connect Failed. Error Code: %d", rc);

        return rc;
    }

    PublishParams pubParams = DefaultPubParams;
    pubParams.qos = QOS1;
    while (publish_count < PUBLISH_COUNT) {
        rc = qcloud_iot_mqtt_yield(&clientSub, 100);
        if (rc != QCLOUD_ERR_SUCCESS) {
            Log_e("ClientSub Yield Failed. Error Code: %d", rc);
            qcloud_iot_mqtt_disconnect(&clientSub);
            qcloud_iot_mqtt_disconnect(&clientPub);
            return rc;
        }
        snprintf(payload, 100, "Hello From C SDK : %d", publish_count);
        pubParams.payload = (void *) payload;
        pubParams.payload_len = strlen(payload) + 1;

        rc = qcloud_iot_mqtt_publish(&clientPub, INTEGRATION_TEST_TOPIC, &pubParams);

        if (rc != QCLOUD_ERR_SUCCESS) {
            Log_e("Client Publish Failed:%d", rc);
        }

        publish_count++;
    }

    rc = qcloud_iot_mqtt_yield(&clientSub, 100);

    if (rc != QCLOUD_ERR_SUCCESS) {
        Log_e("ClientSub Yield Failed. Error Code: %d", rc);
        qcloud_iot_mqtt_disconnect(&clientSub);
        qcloud_iot_mqtt_disconnect(&clientPub);
        return rc;
    }

    for (i = 0; i < PUBLISH_COUNT; i++) {
        if (countArray[i] > 0) {
            rxMsgCount++;
        }
    }

    if (publish_count == rxMsgCount) {
        Log_i("Success");
        Log_i("ClientPub Published Message Count: %d, ClientSub Received Message Count: %d", publish_count, rxMsgCount);
    } else {
        Log_e("Failure");
        Log_e("Publish Message Count Not Equal Receive Message Count");
        Log_e("ClientPub Published Message Count: %d, ClientSub Received Message Count: %d", publish_count, rxMsgCount);
        return -1;
    }

    qcloud_iot_mqtt_disconnect(&clientSub);
    qcloud_iot_mqtt_disconnect(&clientPub);

    return rc;
}
