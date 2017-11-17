#include "qcloud_iot_export_log.h"
#include "iot_test_integration_runner.h"
#include "iot_test_config.h"

int main() {
    int rc = 0;
    qcloud_iot_set_log_level(INFO);
    Log_i("Starting Test 1 MQTT Version 3.1.1 Subscribe QoS1 Publish QoS1 with Single Client.");
    rc = iot_mqtt_tests_basic_connectivity();

    if (rc != 0) {
        Log_i("Test 1 MQTT Version 3.1.1 Subscribe QoS1 Publish QoS1 with Single Client Failed.")
        return rc;
    }
    Log_i("Test 1 MQTT Version 3.1.1 Subscribe QoS1 Publish QoS1 with Single Client Success.")

    Log_i("Starting Test 2 MQTT Version 3.1.1 Auto Reconnect with Single Client.");

    rc = iot_mqtt_tests_auto_reconnect();

    if (rc != 0) {
        Log_i("Starting Test 2 MQTT Version 3.1.1 Auto Reconnect with Single Client Failed: %d", rc);
        return rc;
    }

    Log_i("Starting Test 2 MQTT Version 3.1.1 Auto Reconnect with Single Client Success.");


    Log_i("Starting Test 3 MQTT Version 3.1.1 Publish & Subscribe with Multiple Client.");

    rc = iot_mqtt_tests_multiple_clients();

    if (rc != 0) {
        Log_i("Starting Test 3 MQTT Version 3.1.1 Publish & Subscribe with Multiple Client Failed: %d", rc);
        return rc;
    }

    Log_i("Starting Test 3 MQTT Version 3.1.1 Publish & Subscribe with Multiple Client Success.");

    return 0;
}

