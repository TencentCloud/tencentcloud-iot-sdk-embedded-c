#include <iostream>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include "dev_iot_config.h"
#include "util.h"

/**
 * 设置MQTT connet初始化参数
 *
 * @param pInitParams MQTT connet初始化参数
 */
static void _setup_connect_init_params(MQTTInitParams* pInitParams, char* tx_client_id, char* tx_device_password, char* tx_product_name, char* tx_device_name, char* tx_ca, char* tx_cert, char* tx_private, char* tx_psk, int tx_sym);

/**
 * 1. 设置客户端初始化参数
 * 2. 设置MQTT connet初始化参数
 * 3. 连接腾讯云
 */
static int tx_init_conn_iot(string connType, int testPort);
