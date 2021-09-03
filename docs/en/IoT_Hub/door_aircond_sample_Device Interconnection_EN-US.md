# Device Interconnection
This document describes how to quickly try out the interconnection of devices based on the **aircond_shadow_sample** and **door_mqtt_sample** of the SDK by using the message forwarding and rule engine features of IoT Hub in a smart home scenario.

## 1. Creating Device in Console

#### 1. Scenario description
For more information on the application scenario described in this document, please see [Device Interconnection](https://cloud.tencent.com/document/product/634/11913).
In this smart home scenario, the temperature control of the air conditioner is triggered when the door opens. The messages between the two devices are forwarded through the rule engine in the cloud.

![](https://main.qcloudimg.com/raw/f4c5186f03f8636eb8f8d80622b49b02.png)

#### 2. Product and device creation and rule engine configuration
Create an air conditioner product, a door product, `airConditioner1` device, and `door1` device as instructed in [Device Interconnection](https://cloud.tencent.com/document/product/634/11913) and then configure the rule engine.


## 2. Compiling and Running Demo (with **Key-Authenticated Device** as Example)

#### 1. Compile the SDK
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
The demo outputs `aircond_shadow_sample` and `door_mqtt_sample` are in the `output/release/bin` folder.

#### 2. Enter the device information
Enter the information of the `airConditioner1` device created above in the JSON file `aircond_device_info.json`.
```
{
    "auth_mode":"KEY",	
	"productId":"GYT9V6D4AF",
  	"deviceName":"airConditioner1",	
    "key_deviceinfo":{    
        "deviceSecret":"vXeds12qazsGsMyf5SMfs6OA6y"
    }
}
```
Enter the information of the `door1` device in another JSON file `door_device_info.json`.
```
{
    "auth_mode":"KEY",	
	"productId":"S3EUVBRJLB",
  	"deviceName":"door1",	
    "key_deviceinfo":{    
        "deviceSecret":"i92E3QMNmxi5hvIxUHjO8gTdg"
    }
}
```

#### 3. Run the `aircond_shadow_sample` demo
Because this scenario involves two demos running simultaneously, you can run the air conditioner demo in the current terminal console first, and you can see that the demo subscribes to the `/{productID}/{deviceName}/control` topic and then enters the loop waiting status.
```
 ./output/release/bin/aircond_shadow_sample -c ./aircond_device_info.json
INF|2019-09-16 23:25:17|device.c|iot_device_info_set(67): SDK_Ver: 3.1.0, Product_ID: GYT9V6D4AF, Device_Name: airConditioner1
DBG|2019-09-16 23:25:17|HAL_TLS_mbedtls.c|HAL_TLS_Connect(204): Setting up the SSL/TLS structure...
DBG|2019-09-16 23:25:17|HAL_TLS_mbedtls.c|HAL_TLS_Connect(246): Performing the SSL/TLS handshake...
DBG|2019-09-16 23:25:17|HAL_TLS_mbedtls.c|HAL_TLS_Connect(247): Connecting to /GYT9V6D4AF.iotcloud.tencentdevices.com/8883...
INF|2019-09-16 23:25:18|HAL_TLS_mbedtls.c|HAL_TLS_Connect(269): connected with /GYT9V6D4AF.iotcloud.tencentdevices.com/8883...
INF|2019-09-16 23:25:19|mqtt_client.c|IOT_MQTT_Construct(125): mqtt connect with id: Nh9Vc success
DBG|2019-09-16 23:25:19|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(138): topicName=$shadow/operation/result/GYT9V6D4AF/airConditioner1|packet_id=56171
DBG|2019-09-16 23:25:19|shadow_client.c|_shadow_event_handler(63): shadow subscribe success, packet-id=56171
INF|2019-09-16 23:25:19|aircond_shadow_sample.c|event_handler(96): subscribe success, packet-id=56171
INF|2019-09-16 23:25:19|shadow_client.c|IOT_Shadow_Construct(172): Sync device data successfully
INF|2019-09-16 23:25:19|aircond_shadow_sample.c|main(256): Cloud Device Construct Success
DBG|2019-09-16 23:25:19|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(138): topicName=GYT9V6D4AF/airConditioner1/control|packet_id=56172
DBG|2019-09-16 23:25:19|shadow_client.c|_shadow_event_handler(63): shadow subscribe success, packet-id=56172
INF|2019-09-16 23:25:19|aircond_shadow_sample.c|event_handler(96): subscribe success, packet-id=56172
INF|2019-09-16 23:25:19|aircond_shadow_sample.c|main(291): airConditioner state: close
INF|2019-09-16 23:25:19|aircond_shadow_sample.c|main(292): currentTemperature: 32.000000, energyConsumption: 0.000000
```

#### 4. Run the door_mqtt_sample demo to simulate a homecoming event
Open another terminal console, run the door demo, and you can see that the demo sends a JSON message {"action": "come_home", "targetDevice": "airConditioner1"} to the `/{productID}/{deviceName}/event` topic, which notifies the target device `airConditioner1` of the homecoming event and then quits.

```
./output/release/bin/door_mqtt_sample -c ../devinfo/door_device_info.json -t airConditioner1 -a come_home
INF|2019-09-16 23:29:11|device.c|iot_device_info_set(67): SDK_Ver: 3.1.0, Product_ID: S3EUVBRJLB, Device_Name: test_dev_key
DBG|2019-09-16 23:29:11|HAL_TLS_mbedtls.c|HAL_TLS_Connect(204): Setting up the SSL/TLS structure...
DBG|2019-09-16 23:29:11|HAL_TLS_mbedtls.c|HAL_TLS_Connect(246): Performing the SSL/TLS handshake...
DBG|2019-09-16 23:29:11|HAL_TLS_mbedtls.c|HAL_TLS_Connect(247): Connecting to /S3EUVBRJLB.iotcloud.tencentdevices.com/8883...
INF|2019-09-16 23:29:11|HAL_TLS_mbedtls.c|HAL_TLS_Connect(269): connected with /S3EUVBRJLB.iotcloud.tencentdevices.com/8883...
INF|2019-09-16 23:29:11|mqtt_client.c|IOT_MQTT_Construct(125): mqtt connect with id: d89Wh success
INF|2019-09-16 23:29:11|door_mqtt_sample.c|main(229): Cloud Device Construct Success
DBG|2019-09-16 23:29:11|mqtt_client_publish.c|qcloud_iot_mqtt_publish(329): publish topic seq=46683|topicName=S3EUVBRJLB/test_dev_key/event|payload={"action": "come_home", "targetDevice": "airConditioner1"}
INF|2019-09-16 23:29:11|door_mqtt_sample.c|main(246): Wait for publish ack
INF|2019-09-16 23:29:11|door_mqtt_sample.c|event_handler(81): publish success, packet-id=46683
INF|2019-09-16 23:29:13|mqtt_client_connect.c|qcloud_iot_mqtt_disconnect(437): mqtt disconnect!
INF|2019-09-16 23:29:13|mqtt_client.c|IOT_MQTT_Destroy(186): mqtt release!
```

#### 5. Observe the message reception of the air conditioner and simulate a message response
At this point, observe the printout of `aircond_shadow_sample`, and you can see that it has received the homecoming message sent by `door1`, turned on the air conditioner, and started to regulate the temperature.
```
INF|2019-09-16 23:29:11|aircond_shadow_sample.c|main(291): airConditioner state: close
INF|2019-09-16 23:29:11|aircond_shadow_sample.c|main(292): currentTemperature: 32.000000, energyConsumption: 0.000000
INF|2019-09-16 23:29:12|aircond_shadow_sample.c|on_message_callback(140): Receive Message With topicName:GYT9V6D4AF/airConditioner1/control, payload:{"action":"come_home","targetDevice":"airConditioner1"}
INF|2019-09-16 23:29:12|aircond_shadow_sample.c|main(291): airConditioner state: open
INF|2019-09-16 23:29:12|aircond_shadow_sample.c|main(292): currentTemperature: 31.000000, energyConsumption: 1.000000
INF|2019-09-16 23:29:13|aircond_shadow_sample.c|main(291): airConditioner state: open
INF|2019-09-16 23:29:13|aircond_shadow_sample.c|main(292): currentTemperature: 30.000000, energyConsumption: 2.000000
```

#### 6. Run the door_mqtt_sample demo to simulate a homeleaving event
At this point, if you run the door demo again and change the program launch parameter, you can see that the demo sends a JSON message {"action": "leave_home", "targetDevice": "airConditioner1"} to the `/{productID}/{deviceName}/event` topic, which notifies the target device `airConditioner1` of the homeleaving event.

```
./output/release/bin/door_mqtt_sample -c ../devinfo/door_device_info.json -t airConditioner1 -a leave_home
INF|2019-09-16 23:40:35|device.c|iot_device_info_set(67): SDK_Ver: 3.1.0, Product_ID: S3EUVBRJLB, Device_Name: test_dev_key
DBG|2019-09-16 23:40:35|HAL_TLS_mbedtls.c|HAL_TLS_Connect(204): Setting up the SSL/TLS structure...
DBG|2019-09-16 23:40:35|HAL_TLS_mbedtls.c|HAL_TLS_Connect(246): Performing the SSL/TLS handshake...
DBG|2019-09-16 23:40:35|HAL_TLS_mbedtls.c|HAL_TLS_Connect(247): Connecting to /S3EUVBRJLB.iotcloud.tencentdevices.com/8883...
INF|2019-09-16 23:40:35|HAL_TLS_mbedtls.c|HAL_TLS_Connect(269): connected with /S3EUVBRJLB.iotcloud.tencentdevices.com/8883...
INF|2019-09-16 23:40:35|mqtt_client.c|IOT_MQTT_Construct(125): mqtt connect with id: 3I59W success
INF|2019-09-16 23:40:35|door_mqtt_sample.c|main(229): Cloud Device Construct Success
DBG|2019-09-16 23:40:35|mqtt_client_publish.c|qcloud_iot_mqtt_publish(329): publish topic seq=39867|topicName=S3EUVBRJLB/test_dev_key/event|payload={"action": "leave_home", "targetDevice": "airConditioner1"}
INF|2019-09-16 23:40:35|door_mqtt_sample.c|main(246): Wait for publish ack
INF|2019-09-16 23:40:35|door_mqtt_sample.c|event_handler(81): publish success, packet-id=39867
INF|2019-09-16 23:40:37|mqtt_client_connect.c|qcloud_iot_mqtt_disconnect(437): mqtt disconnect!
INF|2019-09-16 23:40:37|mqtt_client.c|IOT_MQTT_Destroy(186): mqtt release!
```

#### 7. Observe the message reception of the air conditioner and simulate a message response
At this point, observe the printout of `aircond_shadow_sample`, and you can see that it has received the homeleaving message sent by `door1` and turned off the air conditioner, and the temperature has started to go up.
```
INF|2019-09-16 23:40:35|aircond_shadow_sample.c|main(291): airConditioner state: open
INF|2019-09-16 23:40:35|aircond_shadow_sample.c|main(292): currentTemperature: 26.000000, energyConsumption: 6.000000
INF|2019-09-16 23:40:36|aircond_shadow_sample.c|on_message_callback(140): Receive Message With topicName:GYT9V6D4AF/airConditioner1/control, payload:{"action":"leave_home","targetDevice":"airConditioner1"}
INF|2019-09-16 23:40:36|aircond_shadow_sample.c|main(291): airConditioner state: close
INF|2019-09-16 23:40:36|aircond_shadow_sample.c|main(292): currentTemperature: 26.500000, energyConsumption: 6.000000
INF|2019-09-16 23:40:37|aircond_shadow_sample.c|main(291): airConditioner state: close
INF|2019-09-16 23:40:37|aircond_shadow_sample.c|main(292): currentTemperature: 27.000000, energyConsumption: 6.000000
INF|2019-09-16 23:40:39|aircond_shadow_sample.c|main(291): airConditioner state: close
```

At this point, you have completed the execution of the demos that simulate device interconnection.
