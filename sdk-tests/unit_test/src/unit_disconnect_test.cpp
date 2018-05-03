/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include <gtest/gtest.h>
#include <unit_helper_functions.h>

static MQTTConnectParams 	connectParams;
static MQTTInitParams 		initParams;
static Qcloud_IoT_Client 	iotClient;

static bool handlerInvoked = false;

void disconnectTestHandler(void) {
	handlerInvoked = true;
}

void event_handle(void *pcontext, void *pclient, MQTTEventMsg *msg)
{
    switch (msg->event_type) {
        case MQTT_EVENT_UNDEF:
            Log_d("MQTT_EVENT_UNDEF|undefined event occur.");
            break;

        case MQTT_EVENT_DISCONNECT:
            Log_d("MQTT_EVENT_DISCONNECT|MQTT disconnect.");
			disconnectTestHandler();
            break;

        default:
            Log_d("Should NOT arrive here.");
            break;
    }
}


class DisconnectTests : public testing::Test
{
protected:
    virtual void SetUp()
    {
        std::cout << "DisconnectTests Test Begin \n";

        int rc;
		MQTTInitParamsSetup(&initParams, true);
		rc = qcloud_iot_mqtt_init(&iotClient, &initParams);
		initParams.event_handle.h_fp = event_handle;
		ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

		ConnectParamsSetup(&connectParams, &initParams);
		connectParams.keep_alive_interval = 5;
		rc = qcloud_iot_mqtt_connect(&iotClient, &connectParams);
		ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    }
    virtual void TearDown()
    {
        std::cout << "DisconnectTests Test End \n";
    } 
	
};


/* F:1 - Disconnect with Null/empty client instance */
TEST_F(DisconnectTests, NullClientDisconnect) {
	int rc = qcloud_iot_mqtt_disconnect(NULL);
	ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

// /* F:2 - Set Disconnect Handler with Null/empty Client */
// TEST_F(DisconnectTests, NullClientSetDisconnectHandler) {
// 	int rc = qcloud_iot_mqtt_set_disconnect_handler(NULL, disconnectTestHandler);
// 	ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
// }

// /* F:3 - Call Set Disconnect handler with Null handler */
// TEST_F(DisconnectTests, SetDisconnectHandlerNullHandler) {
// 	int rc = qcloud_iot_mqtt_set_disconnect_handler(&iotClient, NULL);
// 	ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
// }

/* F:4 - Disconnect attempt, not connected */
TEST_F(DisconnectTests, disconnectNotConnected) {
	int rc;

	/* First make sure client is disconnected */
	rc = qcloud_iot_mqtt_disconnect(&iotClient);
	ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

	/* Check client is disconnected */
	ASSERT_TRUE(false == IOT_MQTT_IsConnected(&iotClient));

	/* Now call disconnect again */
	rc = qcloud_iot_mqtt_disconnect(&iotClient);
	ASSERT_EQ(QCLOUD_ERR_MQTT_NO_CONN, rc);
}

/* F:5 - Disconnect success */
TEST_F(DisconnectTests, disconnectNoAckSuccess) {
	int rc;
	rc = qcloud_iot_mqtt_disconnect(&iotClient);
	ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
