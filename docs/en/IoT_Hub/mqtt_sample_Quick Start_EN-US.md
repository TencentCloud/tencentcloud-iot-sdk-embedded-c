
# Quick Start
This document describes how to apply for devices in the IoT Hub console and quickly try out device connection to IoT Hub over the MQTT protocol for message receiving/sending based on the **mqtt_sample** of the SDK.

## 1. Creating Device in Console

#### 1. Sign up for or log in to your Tencent Cloud account
Go to the [Tencent Cloud login page](https://cloud.tencent.com/login?s_url=https%3A%2F%2Fcloud.tencent.com%2F) and click [Sign up now](https://cloud.tencent.com/register?s_url=https%3A%2F%2Fcloud.tencent.com%2F) to get a Tencent Cloud account for free. If you already have one, you can log in directly.

#### 2. Go to the IoT Hub console
After login, click **Console** in the top-right corner to enter the console, mouse over **Products**, and click **IoT Hub** in the pop-up menu.

![](https://main.qcloudimg.com/raw/5161335905cf740555a96731d22cd610.png)

Alternatively, directly go to the [IoT Hub console](https://console.intl.cloud.tencent.com/iotcloud).

#### 3. Create a product and device
Click **Create Product** to create a product.

![](https://main.qcloudimg.com/raw/bc3df4129fe2acd3aefa6f9649670ff8.png)

In the pop-up window, select the node type and product type, enter the product name, select the authentication method and data format, enter the product description, and click **Confirm**. To create a general directly connected product, you can select the options as shown below.

![](https://main.qcloudimg.com/raw/9b4c1019f5e42e36d439703217a19fe5.png)

On the generated product page, click **Add Device** on the **Devices** tab.

![](https://main.qcloudimg.com/raw/4a56087f02bfd7a99999cfbb0d3e6d5c.png)

If the authentication method is certificate authentication, after the device name is entered, be sure to click **Download** in the pop-up window. The device key file and device certificate in the downloaded package are used for authenticating the device during connection to IoT Hub.

![](https://main.qcloudimg.com/raw/1375d751f121a1fe59fe70b0a725402e.png)

If the authentication method is key authentication, after the device name is entered, the key of the added device will be displayed in the pop-up window.

![](https://main.qcloudimg.com/raw/4d442b6c1a05a0029359f8bdf7e173b4.png)

#### 4. Create a topic that supports subscribing and publishing

After entering the product settings page as instructed in **Step 3**, click **Permissions** > **Add Topic Permission**.

![](https://main.qcloudimg.com/raw/f1e483d9642960b78058fbb85ac0a706.png)

In the pop-up window, enter `data`, set the operation permission to **Publish and Subscribe**, and click **Create**.

Then, the `productID/\${deviceName}/data` topic will be created, and you can view all permissions of the product in the permission list on the product page.

#### 5. Create a message forwarding rule
Message forwarding from one device to another requires the rule engine. For more information, please see [Overview](https://cloud.tencent.com/document/product/634/14446).

Please note that if you want a device to receive messages sent by itself, you also need to create a forwarding rule. For this example, you can set the forwarding rule as shown below (and replace the `productID` and `deviceName` with the information of the device created above).
![](https://main.qcloudimg.com/raw/0b13b3fd8f30dd3299b9efe9da7f5134.png)

## 2. Compiling and Running Demo

#### 1. Compile the SDK
Modify `CMakeLists.txt` to ensure that the following options exist (with a key-authenticated device as example).
```
set(BUILD_TYPE                   "release")
set(COMPILE_TOOLS                "gcc") 
set(PLATFORM 	                "linux")
set(FEATURE_MQTT_COMM_ENABLED ON)
set(FEATURE_AUTH_MODE "KEY")
set(FEATURE_AUTH_WITH_NOTLS OFF)
set(FEATURE_DEBUG_DEV_INFO_USED  OFF)
```
Run the following script for compilation.
```
./cmake_build.sh 
```
The demo output is in the `output/release/bin` folder.

#### 2. Enter the device information
Enter the information of the device created above on the IoT Hub platform in `device_info.json` (with a **key-authenticated device** as example).
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

#### 3. Run the `mqtt_sample` demo
```
./output/release/bin/mqtt_sample 
INF|2019-09-12 21:28:20|device.c|iot_device_info_set(67): SDK_Ver: 3.1.0, Product_ID: S3EUVBRJLB, Device_Name: test_device
DBG|2019-09-12 21:28:20|HAL_TLS_mbedtls.c|HAL_TLS_Connect(204): Setting up the SSL/TLS structure...
DBG|2019-09-12 21:28:20|HAL_TLS_mbedtls.c|HAL_TLS_Connect(246): Performing the SSL/TLS handshake...
DBG|2019-09-12 21:28:20|HAL_TLS_mbedtls.c|HAL_TLS_Connect(247): Connecting to /S3EUVBRJLB.iotcloud.tencentdevices.com/8883...
INF|2019-09-12 21:28:20|HAL_TLS_mbedtls.c|HAL_TLS_Connect(269): connected with /S3EUVBRJLB.iotcloud.tencentdevices.com/8883...
INF|2019-09-12 21:28:20|mqtt_client.c|IOT_MQTT_Construct(125): mqtt connect with id: p8t0W success
INF|2019-09-12 21:28:20|mqtt_sample.c|main(303): Cloud Device Construct Success
DBG|2019-09-12 21:28:20|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(138): topicName=$sys/operation/result/S3EUVBRJLB/test_device|packet_id=1932
INF|2019-09-12 21:28:20|mqtt_sample.c|_mqtt_event_handler(71): subscribe success, packet-id=1932
DBG|2019-09-12 21:28:20|system_mqtt.c|_system_mqtt_sub_event_handler(80): mqtt sys topic subscribe success
DBG|2019-09-12 21:28:20|mqtt_client_publish.c|qcloud_iot_mqtt_publish(337): publish packetID=0|topicName=$sys/operation/S3EUVBRJLB/test_device|payload={"type": "get", "resource": ["time"]}
DBG|2019-09-12 21:28:20|system_mqtt.c|_system_mqtt_message_callback(63): Recv Msg Topic:$sys/operation/result/S3EUVBRJLB/test_device, payload:{"type":"get","time":1568294900}
INF|2019-09-12 21:28:21|mqtt_sample.c|main(316): system time is 1568294900
DBG|2019-09-12 21:28:21|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(138): topicName=S3EUVBRJLB/test_device/data|packet_id=1933
INF|2019-09-12 21:28:21|mqtt_sample.c|_mqtt_event_handler(71): subscribe success, packet-id=1933
DBG|2019-09-12 21:28:21|mqtt_client_publish.c|qcloud_iot_mqtt_publish(329): publish topic seq=1934|topicName=S3EUVBRJLB/test_device/data|payload={"action": "publish_test", "count": "0"}
INF|2019-09-12 21:28:21|mqtt_sample.c|_mqtt_event_handler(98): publish success, packet-id=1934
INF|2019-09-12 21:28:21|mqtt_sample.c|on_message_callback(195): Receive Message With topicName:S3EUVBRJLB/test_device/data, payload:{"action": "publish_test", "count": "0"}
INF|2019-09-12 21:28:22|mqtt_client_connect.c|qcloud_iot_mqtt_disconnect(437): mqtt disconnect!
INF|2019-09-12 21:28:22|system_mqtt.c|_system_mqtt_sub_event_handler(98): mqtt client has been destroyed
INF|2019-09-12 21:28:22|mqtt_client.c|IOT_MQTT_Destroy(186): mqtt release!
```

#### 4. Observe message sending
The following log information shows that the demo reported data to `/{productID}/{deviceName}/data` through the `Publish` message type of MQTT, and that the server received and successfully processed the message. 
```
INF|2019-09-12 21:28:21|mqtt_sample.c|_mqtt_event_handler(98): publish success, packet-id=1934
```

#### 5. Observe message receiving
The following log information shows that as the message reached the subscribed topic, it was pushed to the demo as-is by the server and entered the corresponding callback function.
```
INF|2019-09-12 21:28:21|mqtt_sample.c|on_message_callback(195): Receive Message With topicName:S3EUVBRJLB/test_device/data, payload:{"action": "publish_test", "count": "0"}
```

#### 6. Observe logs in the console
Log in to the IoT Hub console and click **Cloud Log** on the left sidebar to view the message just reported.
![](https://main.qcloudimg.com/raw/09002dd8634e37ef408aa151357bc350.png)

#### 7. Observe NTP time logs
The following log information shows the NTP time sync: the device published a message to get the NTP time; IoT Hub logged the timestamp `ntptime1` of receiving the message to get the NTP time from the device and the timestamp `ntptime2` of replying to the device and sent `ntptime1` and `ntptime2` to the device; the device received the reply message from IoT Hub, calculated the actual time on the device with an algorithm, and called the system time setting function to set the time. As can be seen in the demo log, the time was correctly set.
The system time settings have been adapted to Windows and Linux but not to other systems. When running the demo, you should run it as the admin on Windows or superuser on Linux.
```
sudo ./output/release/bin/mqtt_sample
DBG|2020-10-22 10:48:26|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$sys/
operation/S3EUVBRJLB/test_device|payload={"type": "get", "resource": ["time"]}
DBG|2020-10-22 10:48:27|system_mqtt.c|_system_mqtt_message_callback(47): Recv Msg Topic:$sys/operation/result/
S3EUVBRJLB/test_device, payload:{"type":"get","time":1603334906,"ntptime1":1603334906594,"ntptime2":1603334906594}
INF|2020-10-22 10:48:26|system_mqtt.c|IOT_Sync_NTPTime(294): set systime ms success, timestamp 1603334906644 ms
INF|2020-10-22 10:48:26|mqtt_sample.c|main(325): sync ntp time success!

```
The `HAL_Timer_freertos.c HAL_Timer_rtthread.c HAL_Timer_nonos.c` file provides the `timestamp_to_date` function to convert a timestamp to the year, month, day, hour, minute, second, and millisecond, making it easy to set the RTC clock on embedded devices.
The `HAL_Timer_set_systime_sec` system time setting function is for 32-bit platforms, while `HAL_Timer_set_systime_ms` for 64-bit platforms.

