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
#include <string.h>

#include <gtest/gtest.h>
#include <unit_helper_functions.h>
#include "qcloud_iot_export.h"


// static MQTTConnectParams connectParams;
static MQTTInitParams   initParams;
static PublishParams    testPubMsgParams;

static char subTopic[100] = {0};

static Qcloud_IoT_Client *iotClient;
char cPayload[100];


class PublishTests : public testing::Test
{
protected:
    virtual void SetUp()
    {
        std::cout << "PublishTests Test Begin \n";

        sprintf(subTopic, "%s/%s/abc", QCLOUD_IOT_MY_PRODUCT_ID, QCLOUD_IOT_MY_DEVICE_NAME);
        SetupMQTTConnectInitParams(&initParams);
        iotClient = (Qcloud_IoT_Client *)IOT_MQTT_Construct(&initParams);

	    testPubMsgParams.qos = QOS1;
	    testPubMsgParams.retained = 0;
	    sprintf(cPayload, "%s : %d ", "hello from SDK", 0);
	    testPubMsgParams.payload = (void *) cPayload;
	    testPubMsgParams.payload_len = strlen(cPayload);
    }
    virtual void TearDown()
    {
        std::cout << "PublishTests Test End \n";
        void *c = (void*)iotClient;
        IOT_MQTT_Destroy(&c);
    } 
	
};


/* E:1 - Publish with Null/empty client instance */
TEST_F(PublishTests, PublishNullClient) {
    int rc;

    testPubMsgParams.qos = QOS1;
    testPubMsgParams.retained = 0;
    testPubMsgParams.payload = (void *)"Message";
    testPubMsgParams.payload_len = 7;

    rc = qcloud_iot_mqtt_publish(NULL, subTopic, &testPubMsgParams);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

/* E:2 - Publish with Null/empty Topic Name */
TEST_F(PublishTests, PublishNullTopic) {
    int rc;

    testPubMsgParams.qos = QOS1;
    testPubMsgParams.retained = 0;
    testPubMsgParams.payload = (void *)"Message";
    testPubMsgParams.payload_len = 7;

    rc = qcloud_iot_mqtt_publish(iotClient, NULL, &testPubMsgParams);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);

    rc = qcloud_iot_mqtt_publish(iotClient, (char *)"", &testPubMsgParams);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

/* E:3 - Publish with Null/empty payload */
TEST_F(PublishTests, PublishNullParams) {
    int rc;

    rc = qcloud_iot_mqtt_publish(iotClient, subTopic, NULL);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);

    testPubMsgParams.qos = QOS1;
    testPubMsgParams.retained = 0;
    testPubMsgParams.payload = NULL;
    testPubMsgParams.payload_len = 0;

    rc = qcloud_iot_mqtt_publish(iotClient, subTopic, &testPubMsgParams);
    ASSERT_EQ(QCLOUD_ERR_INVAL, rc);
}

/* E:4 - Publish with network disconnected */
TEST_F(PublishTests, PublishNetworkDisconnected) {
    int rc;

    /* Ensure network is disconnected */
    rc = qcloud_iot_mqtt_disconnect(iotClient);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);

    testPubMsgParams.qos = QOS1;
    testPubMsgParams.retained = 0;
    testPubMsgParams.payload = (void *)"Message";
    testPubMsgParams.payload_len = 7;

    rc = qcloud_iot_mqtt_publish(iotClient, subTopic, &testPubMsgParams);
    ASSERT_EQ(QCLOUD_ERR_MQTT_NO_CONN, rc);
}

/* E:6 - Publish QoS0 success */
TEST_F(PublishTests, publishQoS0NoPubackSuccess) {
    int rc;

    testPubMsgParams.qos = QOS0; // switch to a Qos0 PUB
    rc = IOT_MQTT_Publish(iotClient, subTopic, &testPubMsgParams);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
}

/* E:7 - Publish with QoS1 send success, Puback received */
TEST_F(PublishTests, publishQoS1Success) {
    int rc;

    testPubMsgParams.qos = QOS1;
    rc = IOT_MQTT_Publish(iotClient, subTopic, &testPubMsgParams);
    ASSERT_EQ(QCLOUD_ERR_SUCCESS, rc);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
