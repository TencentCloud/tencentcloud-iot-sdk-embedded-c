# Device Shadow
This document describes the device shadow feature and demonstrates the data flow and feature of the shadow based on the **shadow_sample** of the SDK.

## Overview

Device shadow is essentially a copy of device data in JSON format cached on the server and is mainly used to save:

- Current device configurations
- Current device status

As an intermediary, device shadow can effectively implement two-way data sync between device and user application:

- For device configuration, the user application does not need to directly modify the device; instead, it can modify the device shadow on the server, which will sync modifications to the device. In this way, if the device is offline at the time of modification, it will receive the latest configuration from the shadow once coming back online.
- For device status, the device reports the status to the device shadow, and when users initiate queries, they can simply query the shadow. This can effectively reduce the network interactions between the device and the server, especially for low-power devices.

For more information on the device shadow, please see [Device Shadow Details](https://cloud.tencent.com/document/product/634/11918).

## Compiling and Running Demo (with **Key-Authenticated Device** as Example)

#### 1. Compile the SDK and demo
Modify `CMakeLists.txt` to ensure that the following options exist:
```
set(BUILD_TYPE                   "release")
set(COMPILE_TOOLS                "gcc") 
set(PLATFORM 	                "linux")
set(FEATURE_MQTT_COMM_ENABLED ON)
set(FEATURE_MQTT_DEVICE_SHADOW ON)
set(FEATURE_AUTH_MODE "KEY")
set(FEATURE_AUTH_WITH_NOTLS OFF)
set(FEATURE_DEBUG_DEV_INFO_USED  OFF)
```
Run the following script for compilation.
```
./cmake_build.sh 
```
The demo output `shadow_sample` is in the `output/release/bin` folder.

#### 2. Enter the device information
Enter the device information in the configuration file `device_info.json`.
```
{
  "auth_mode":"KEY",	
  "productId":"S3EUVBRJLB",
  "deviceName":"test_device",	
  "key_deviceinfo":{    
      "deviceSecret":"vX6PQqazsGsMyf5SMfs6OA6y"
  }
}
```
#### 3. Run the `shadow_sample` demo for the first time
The demo subscribes to the `$shadow/operation/result/{productID}/{deviceName}` topic, sends the `shadow GET` command to the `$shadow/operation/{productID}/{deviceName}` topic to get the device status cached in the cloud, updates the `updateCount` variable in a loop, and runs `shadow UPDATE`.
Assume that the demo is stopped when `updateCount` is `3`.

```
./output/release/bin/shadow_sample 
INF|2019-09-17 12:49:59|device.c|iot_device_info_set(67): SDK_Ver: 3.1.0, Product_ID: S3EUVBRJLB, Device_Name: test_device
DBG|2019-09-17 12:49:59|HAL_TLS_mbedtls.c|HAL_TLS_Connect(204): Setting up the SSL/TLS structure...
DBG|2019-09-17 12:49:59|HAL_TLS_mbedtls.c|HAL_TLS_Connect(246): Performing the SSL/TLS handshake...
DBG|2019-09-17 12:49:59|HAL_TLS_mbedtls.c|HAL_TLS_Connect(247): Connecting to /S3EUVBRJLB.iotcloud.tencentdevices.com/8883...
INF|2019-09-17 12:49:59|HAL_TLS_mbedtls.c|HAL_TLS_Connect(269): connected with /S3EUVBRJLB.iotcloud.tencentdevices.com/8883...
INF|2019-09-17 12:50:00|mqtt_client.c|IOT_MQTT_Construct(125): mqtt connect with id: bph30 success
DBG|2019-09-17 12:50:00|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(138): topicName=$shadow/operation/result/S3EUVBRJLB/test_device|packet_id=46510
DBG|2019-09-17 12:50:00|shadow_client.c|_shadow_event_handler(63): shadow subscribe success, packet-id=46510
INF|2019-09-17 12:50:00|shadow_client.c|IOT_Shadow_Construct(172): Sync device data successfully
DBG|2019-09-17 12:50:00|shadow_client.c|IOT_Shadow_Get(383): GET Request Document: {"clientToken":"S3EUVBRJLB-0"}
DBG|2019-09-17 12:50:00|mqtt_client_publish.c|qcloud_iot_mqtt_publish(337): publish packetID=0|topicName=$shadow/operation/S3EUVBRJLB/test_device|payload={"type":"get", "clientToken":"S3EUVBRJLB-0"}
DBG|2019-09-17 12:50:00|shadow_client_manager.c|_on_operation_result_handler(278): type:get
DBG|2019-09-17 12:50:00|shadow_client.c|_update_ack_cb(114): requestAck=0
DBG|2019-09-17 12:50:00|shadow_client.c|_update_ack_cb(117): Received Json Document={"clientToken":"S3EUVBRJLB-0","payload":{"state":{"reported":{}},"timestamp":1568695800,"version":0},"result":0,"timestamp":1568695800,"type":"get"}
DBG|2019-09-17 12:50:00|shadow_client.c|IOT_Shadow_Update(317): UPDATE Request Document: {"state":{"reported":{"updateCount":0}}, "clientToken":"S3EUVBRJLB-1"}
DBG|2019-09-17 12:50:00|mqtt_client_publish.c|qcloud_iot_mqtt_publish(337): publish packetID=0|topicName=$shadow/operation/S3EUVBRJLB/test_device|payload={"type":"update", "state":{"reported":{"updateCount":0}}, "clientToken":"S3EUVBRJLB-1"}
DBG|2019-09-17 12:50:01|shadow_client_manager.c|_on_operation_result_handler(278): type:update
INF|2019-09-17 12:50:01|shadow_sample.c|OnShadowUpdateCallback(49): recv shadow update response, response ack: 0
DBG|2019-09-17 12:50:01|shadow_client.c|IOT_Shadow_Update(317): UPDATE Request Document: {"state":{"reported":{"updateCount":1}}, "clientToken":"S3EUVBRJLB-2"}
DBG|2019-09-17 12:50:01|mqtt_client_publish.c|qcloud_iot_mqtt_publish(337): publish packetID=0|topicName=$shadow/operation/S3EUVBRJLB/test_device|payload={"type":"update", "state":{"reported":{"updateCount":1}}, "clientToken":"S3EUVBRJLB-2"}
DBG|2019-09-17 12:50:02|shadow_client_manager.c|_on_operation_result_handler(278): type:update
INF|2019-09-17 12:50:02|shadow_sample.c|OnShadowUpdateCallback(49): recv shadow update response, response ack: 0
DBG|2019-09-17 12:50:02|shadow_client.c|IOT_Shadow_Update(317): UPDATE Request Document: {"state":{"reported":{"updateCount":2}}, "clientToken":"S3EUVBRJLB-3"}
DBG|2019-09-17 12:50:02|mqtt_client_publish.c|qcloud_iot_mqtt_publish(337): publish packetID=0|topicName=$shadow/operation/S3EUVBRJLB/test_device|payload={"type":"update", "state":{"reported":{"updateCount":2}}, "clientToken":"S3EUVBRJLB-3"}
DBG|2019-09-17 12:50:03|shadow_client_manager.c|_on_operation_result_handler(278): type:update
INF|2019-09-17 12:50:03|shadow_sample.c|OnShadowUpdateCallback(49): recv shadow update response, response ack: 0
DBG|2019-09-17 12:50:04|shadow_client.c|IOT_Shadow_Update(317): UPDATE Request Document: {"state":{"reported":{"updateCount":3}}, "clientToken":"S3EUVBRJLB-4"}
DBG|2019-09-17 12:50:04|mqtt_client_publish.c|qcloud_iot_mqtt_publish(337): publish packetID=0|topicName=$shadow/operation/S3EUVBRJLB/test_device|payload={"type":"update", "state":{"reported":{"updateCount":3}}, "clientToken":"S3EUVBRJLB-4"}
```
#### 4. View the device shadow status change in the cloud and modify the status
You can see from the virtual device in the console that the `updateCount` of the device has been updated to `3`, and even if it is disconnected, you can still get its latest status before disconnection from the cache.
![](https://main.qcloudimg.com/raw/9b27d49916ceacfa939de389f187819f.png)

At this point, you can click **Modify** to modify the shadow status, which simulates modifying the target status of the device through TencentCloud API while it is offline; for example, you can change the value of `updateCount` to 5.

![](https://main.qcloudimg.com/raw/00a268be1e9750b158696ca8a4589eb3.png)

After clicking **Confirm**, run `shadow_sample` again.

#### 5. Run the `shadow_sample` demo for the second time
At this point, when you run the `shadow GET` command to get the device status cached in the cloud, you can see that `updateCount` has become the latest status in the cloud.
``` 
./output/release/bin/shadow_sample 
INF|2019-09-17 12:59:01|device.c|iot_device_info_set(67): SDK_Ver: 3.1.0, Product_ID: S3EUVBRJLB, Device_Name: test_device
DBG|2019-09-17 12:59:01|HAL_TLS_mbedtls.c|HAL_TLS_Connect(204): Setting up the SSL/TLS structure...
DBG|2019-09-17 12:59:01|HAL_TLS_mbedtls.c|HAL_TLS_Connect(246): Performing the SSL/TLS handshake...
DBG|2019-09-17 12:59:01|HAL_TLS_mbedtls.c|HAL_TLS_Connect(247): Connecting to /S3EUVBRJLB.iotcloud.tencentdevices.com/8883...
INF|2019-09-17 12:59:01|HAL_TLS_mbedtls.c|HAL_TLS_Connect(269): connected with /S3EUVBRJLB.iotcloud.tencentdevices.com/8883...
INF|2019-09-17 12:59:01|mqtt_client.c|IOT_MQTT_Construct(125): mqtt connect with id: hY8DA success
DBG|2019-09-17 12:59:01|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(138): topicName=$shadow/operation/result/S3EUVBRJLB/test_device|packet_id=63420
DBG|2019-09-17 12:59:01|shadow_client.c|_shadow_event_handler(63): shadow subscribe success, packet-id=63420
INF|2019-09-17 12:59:01|shadow_client.c|IOT_Shadow_Construct(172): Sync device data successfully
DBG|2019-09-17 12:59:01|shadow_client.c|IOT_Shadow_Get(383): GET Request Document: {"clientToken":"S3EUVBRJLB-0"}
DBG|2019-09-17 12:59:01|mqtt_client_publish.c|qcloud_iot_mqtt_publish(337): publish packetID=0|topicName=$shadow/operation/S3EUVBRJLB/test_device|payload={"type":"get", "clientToken":"S3EUVBRJLB-0"}
DBG|2019-09-17 12:59:01|shadow_client_manager.c|_on_operation_result_handler(278): type:get
DBG|2019-09-17 12:59:01|shadow_client.c|_update_ack_cb(114): requestAck=0
DBG|2019-09-17 12:59:01|shadow_client.c|_update_ack_cb(117): Received Json Document={"clientToken":"S3EUVBRJLB-0","payload":{"state":{"reported":{"updateCount":5}},"timestamp":1568696289949,"version":5},"result":0,"timestamp":1568696341,"type":"get"}
```
