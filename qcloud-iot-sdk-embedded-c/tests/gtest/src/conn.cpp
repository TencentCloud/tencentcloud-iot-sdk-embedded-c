#include <iostream>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include "dev_iot_config.h"
#include "conn.h"

extern "C"{
#include "qcloud_iot_export.h"
};
using namespace std;

/**
 * 设置MQTT connet初始化参数
 *
 * @param pInitParams MQTT connet初始化参数
 */
static void _setup_connect_init_params(MQTTInitParams* pInitParams, char* tx_client_id, char* tx_device_password, char* tx_product_name, char* tx_device_name, char* tx_ca, char* tx_cert, char* tx_private, char* tx_psk, int tx_sym)
{
	pInitParams->client_id = tx_client_id;
	pInitParams->device_name = tx_device_name;
	pInitParams->product_name = tx_product_name;
	pInitParams->password = tx_device_password;

	pInitParams->is_asymc_encryption = tx_sym;

	if (pInitParams->is_asymc_encryption) {
		char cert_dir[PATH_MAX + 1] = "../certs";
		char cur_dir[PATH_MAX + 1];

		char ca_file[PATH_MAX + 1];
		char cert_file[PATH_MAX + 1];
		char key_file[PATH_MAX + 1];

		getcwd(cur_dir, sizeof(cur_dir));
		sprintf(ca_file, "%s/%s/%s", cur_dir, cert_dir, tx_ca);
		sprintf(cert_file, "%s/%s/%s", cur_dir, cert_dir, tx_cert);
		sprintf(key_file, "%s/%s/%s", cur_dir, cert_dir, tx_private);

		pInitParams->ca_file = ca_file;
		pInitParams->cert_file = cert_file;
		pInitParams->key_file = key_file;
	}
	else {
		pInitParams->psk = tx_psk;
	}

	pInitParams->command_timeout = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
	pInitParams->connect_timeout = QCLOUD_IOT_TLS_HANDSHAKE_TIMEOUT;
	pInitParams->keep_alive_interval = QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;

	pInitParams->is_asymc_encryption = 1;
    pInitParams->on_disconnect_handler = NULL;
}

/**
 * 1. 设置客户端初始化参数
 * 2. 设置MQTT connet初始化参数
 * 3. 连接腾讯云
 */
static int tx_init_conn_iot(string connType, int testPort){
    MQTTInitParams initParams = DEFAULT_MQTTINIT_PARAMS;

    if(connType == "door"){
        _setup_connect_init_params(&initParams, DOOR_QCLOUD_IOT_MQTT_CLIENT_ID, DOOR_QCLOUD_IOT_MQTT_PASSWORD, DOOR_QCLOUD_IOT_MY_PRODUCT_NAME, DOOR_QCLOUD_IOT_MY_DEVICE_NAME, DOOR_QCLOUD_IOT_CA_FILENAME, DOOR_QCLOUD_IOT_CERT_FILENAME, DOOR_QCLOUD_IOT_KEY_FILENAME, DOOR_QCLOUD_IOT_PSK, testPort);
    }else{
        _setup_connect_init_params(&initParams, AIR_CONDITION_QCLOUD_IOT_MQTT_CLIENT_ID, AIR_CONDITION_QCLOUD_IOT_MQTT_PASSWORD, AIR_CONDITION_QCLOUD_IOT_MY_PRODUCT_NAME, AIR_CONDITION_QCLOUD_IOT_MY_DEVICE_NAME,AIR_CONDITION_QCLOUD_IOT_CA_FILENAME, AIR_CONDITION_QCLOUD_IOT_CERT_FILENAME, AIR_CONDITION_QCLOUD_IOT_KEY_FILENAME, AIR_CONDITION_QCLOUD_IOT_PSK, testPort);
    }

    void *client = IOT_MQTT_Construct(&initParams);
    if (client != NULL) {
        printf("%s Cloud Device Construct[Conn] Success\n", DEBUG);
        return QCLOUD_ERR_SUCCESS;
    } else {
        printf("%s Cloud Device Construct[Conn] Failed\n", DEBUG);
        return QCLOUD_ERR_FAILURE;
    }
}
