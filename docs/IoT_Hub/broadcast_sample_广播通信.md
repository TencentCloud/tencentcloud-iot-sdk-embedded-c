# 广播通信
本文档介绍广播通信功能，并结合 SDK 的 **broadcast_sample** 展示广播功能。

## 一. 控制台创建设备

#### 1. 场景说明
用户有多个空调设备接入物联网通信平台，现在需要服务器向所有的空调设备发送一条相同的指令来关闭空调。
服务器调用`PublishBroadcastMessage`接口，指定产品`ProductId`和广播消息`Payload`，该产品所有在线的设备就会收到消息内容为`Payload`的广播消息。
广播消息的Topic为`$broadcast/rxd/{productID}/{deviceName}`。

#### 2. 创建产品和设备
请参考 [设备互通](https://cloud.tencent.com/document/product/634/11913) 创建空调产品，并依次创建airConditioner1，airConditioner2等多个空调设备。


## 二. 编译运行示例程序(以**密钥认证设备**为例)

#### 1. 编译 SDK
修改CMakeLists.txt确保以下选项存在
```
set(BUILD_TYPE                   "release")
set(COMPILE_TOOLS                "gcc") 
set(PLATFORM 	                 "linux")
set(FEATURE_MQTT_COMM_ENABLED ON)
set(FEATURE_BROADCAST_ENABLED ON)
set(FEATURE_AUTH_MODE "KEY")
set(FEATURE_AUTH_WITH_NOTLS OFF)
set(FEATURE_DEBUG_DEV_INFO_USED  OFF)
```
执行脚本编译
```
./cmake_build.sh 
```
示例输出`broadcast_sample`位于`output/release/bin`文件夹中

#### 2. 填写设备信息
将上面创建的airConditioner1设备的设备信息填写到一个JSON文件aircond_device_info1.json中:
```
{
    "auth_mode":"KEY",	
    "productId":"KL4J2K3JZ8",
    "deviceName":"airConditioner1",	
    "key_deviceinfo":{    
        "deviceSecret":"zOZXUaycuwlePtTcD78dBA=="
    }
}
```
将airConditioner2的设备信息填到aircond_device_info2.json中:
```
{
    "auth_mode":"KEY",	
    "productId":"KL4J2K3JZ8",
    "deviceName":"airConditioner2",	
    "key_deviceinfo":{    
        "deviceSecret":"+IiVNsyKRh0AW1lcOIE07A=="
    }
}
```
依次将其他的设备信息填到对应的json文件。

#### 3. 执行`broadcast_sample`示例程序
因为这个场景涉及到多个sample同时运行，可以打开多个终端运行`broadcast_sample`示例，可以看到所有的示例都订阅了`$broadcast/rxd/{productID}/{deviceName}`主题，然后就处于等待状态。
设备airConditioner1输出如下:
```
./broadcast_sample -c ./aircond_device_info1.json -l 100
INF|2020-08-03 22:50:28|qcloud_iot_device.c|iot_device_info_set(50): SDK_Ver: 3.2.0, Product_ID: KL4J2K3JZ8, Device_Name: airConditioner1
DBG|2020-08-03 22:50:28|HAL_TLS_mbedtls.c|HAL_TLS_Connect(200): Setting up the SSL/TLS structure...
DBG|2020-08-03 22:50:28|HAL_TLS_mbedtls.c|HAL_TLS_Connect(242): Performing the SSL/TLS handshake...
DBG|2020-08-03 22:50:28|HAL_TLS_mbedtls.c|HAL_TLS_Connect(243): Connecting to /KL4J2K3JZ8.iotcloud.tencentdevices.com/8883...
INF|2020-08-03 22:50:28|HAL_TLS_mbedtls.c|HAL_TLS_Connect(265): connected with /KL4J2K3JZ8.iotcloud.tencentdevices.com/8883...
INF|2020-08-03 22:50:28|mqtt_client.c|IOT_MQTT_Construct(113): mqtt connect with id: 932If success
INF|2020-08-03 22:50:28|broadcast_sample.c|main(197): Cloud Device Construct Success
DBG|2020-08-03 22:50:28|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(142): topicName=$broadcast/rxd/KL4J2K3JZ8/airConditioner1|packet_id=22429
INF|2020-08-03 22:50:28|broadcast_sample.c|_mqtt_event_handler(49): subscribe success, packet-id=22429
DBG|2020-08-03 22:50:28|broadcast.c|_broadcast_event_callback(37): broadcast topic subscribe success
```
设备airConditioner2输出如下:
```
./broadcast_sample -c ./aircond_device_info2.json -l 100
INF|2020-08-03 22:51:24|qcloud_iot_device.c|iot_device_info_set(50): SDK_Ver: 3.2.0, Product_ID: KL4J2K3JZ8, Device_Name: airConditioner2
DBG|2020-08-03 22:51:24|HAL_TLS_mbedtls.c|HAL_TLS_Connect(200): Setting up the SSL/TLS structure...
DBG|2020-08-03 22:51:24|HAL_TLS_mbedtls.c|HAL_TLS_Connect(242): Performing the SSL/TLS handshake...
DBG|2020-08-03 22:51:24|HAL_TLS_mbedtls.c|HAL_TLS_Connect(243): Connecting to /KL4J2K3JZ8.iotcloud.tencentdevices.com/8883...
INF|2020-08-03 22:51:24|HAL_TLS_mbedtls.c|HAL_TLS_Connect(265): connected with /KL4J2K3JZ8.iotcloud.tencentdevices.com/8883...
INF|2020-08-03 22:51:25|mqtt_client.c|IOT_MQTT_Construct(113): mqtt connect with id: f9px6 success
INF|2020-08-03 22:51:25|broadcast_sample.c|main(197): Cloud Device Construct Success
DBG|2020-08-03 22:51:25|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(142): topicName=$broadcast/rxd/KL4J2K3JZ8/airConditioner2|packet_id=29904
INF|2020-08-03 22:51:25|broadcast_sample.c|_mqtt_event_handler(49): subscribe success, packet-id=29904
DBG|2020-08-03 22:51:25|broadcast.c|_broadcast_event_callback(37): broadcast topic subscribe success
```

#### 4. 调用云API `PublishBroadcastMessage` 发送广播消息
打开腾讯云[API控制台](https://console.cloud.tencent.com/api/explorer?Product=iotcloud&Version=2018-06-14&Action=PublishBroadcastMessage&SignVersion=)，填写个人密钥和设备参数信息，选择在线调用并发送请求
![image.png](https://main.qcloudimg.com/raw/94c4a7cbd9c8e76d728494ebaf58bf93.png)

#### 5. 观察空调设备的消息接收
观察设备airConditioner1的打印输出，可以看到已经收到服务器发送的消息。
```
DBG|2020-08-03 22:55:32|broadcast.c|_broadcast_message_cb(25): topic=$broadcast/rxd/KL4J2K3JZ8/airConditioner1
INF|2020-08-03 22:55:32|broadcast.c|_broadcast_message_cb(26): len=6, topic_msg=closed
INF|2020-08-03 22:55:32|broadcast_sample.c|_broadcast_message_handler(134): broadcast message=closed
```
观察设备airConditioner2的打印输出，可以看到同时收到服务器发送的消息。
```
DBG|2020-08-03 22:55:32|broadcast.c|_broadcast_message_cb(25): topic=$broadcast/rxd/KL4J2K3JZ8/airConditioner2
INF|2020-08-03 22:55:32|broadcast.c|_broadcast_message_cb(26): len=6, topic_msg=closed
INF|2020-08-03 22:55:32|broadcast_sample.c|_broadcast_message_handler(134): broadcast message=closed
```

#### 6. 空调设备关闭
接收到指令的设备解析指令进行处理。

***至此，完成了广播通信的示例运行。***
