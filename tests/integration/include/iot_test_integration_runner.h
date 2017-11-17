#ifndef MQTT_CLIENT_C_IOT_TEST_INTEGRATION_RUNNER_H
#define MQTT_CLIENT_C_IOT_TEST_INTEGRATION_RUNNER_H

#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include "qcloud_iot_export_log.h"
#include "integration_tests_config.h"
#include "qcloud_iot_mqtt_client.h"
#include "iot_test_config.h"

int iot_mqtt_tests_basic_connectivity();

int iot_mqtt_tests_auto_reconnect();

int iot_mqtt_tests_multiple_clients();



#endif //MQTT_CLIENT_C_IOT_TEST_INTEGRATION_RUNNER_H
