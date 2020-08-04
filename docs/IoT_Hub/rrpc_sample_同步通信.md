# 广播通信
本文档介绍同步通信功能，并结合 SDK 的**rrpc_sample**展示同步通信功能。

## 一. 控制台创建设备

#### 1. 背景介绍
MQTT协议是基于发布/订阅的异步通信模式，服务器无法控制设备同步返回结果。为解决此问题，物联网通信平台实现了一套同步通信机制，称为RRPC(Revert RPC)。
即由服务器向客户端发起请求，客户端即时响应并同步给出答复。
* 订阅消息Topic: `$rrpc/rxd/{productID}/{deviceName}/+`
* 请求消息Topic: `$rrpc/rxd/{productID}/{deviceName}/{processID}`
* 应答消息Topic: `$rrpc/txd/{productID}/{deviceName}/{processID}`
* processID   : 服务器生成的唯一的消息ID，用来标识不同RRPC消息。可以通过RRPC应答消息中携带的`processID`找到对应的RRPC请求消息。

#### 2. 原理图
![image.png](https://main.qcloudimg.com/raw/1e83a60cb7b6438ebb5927b7237b77ba.png)
* **RRPC请求4s超时**，即4s内设备端没有应答就认为请求超时。

#### 3. 创建产品和设备
请参考[设备互通](https://cloud.tencent.com/document/product/634/11913) 创建空调产品，并创建airConditioner1空调设备。

## 二. 编译运行示例程序(以**密钥认证设备**为例)

#### 1. 编译 SDK
修改CMakeLists.txt确保以下选项存在
```
set(BUILD_TYPE                   "release")
set(COMPILE_TOOLS                "gcc") 
set(PLATFORM 	                 "linux")
set(FEATURE_MQTT_COMM_ENABLED ON)
set(FEATURE_RRPC_ENABLED ON)
set(FEATURE_AUTH_MODE "KEY")
set(FEATURE_AUTH_WITH_NOTLS OFF)
set(FEATURE_DEBUG_DEV_INFO_USED  OFF)
```
执行脚本编译
```
./cmake_build.sh 
```
示例输出`rrpc_sample`位于`output/release/bin`文件夹中

#### 2. 填写设备信息
将上面创建的airConditioner1设备的设备信息填写到JSON文件aircond_device_info1.json中:
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

#### 3. 执行`rrpc_sample`示例程序
可以看到设备airConditioner1订阅了RRPC消息，然后处于等待状态。
```
./rrpc_sample -c ./aircond_device_info1.json -l 1000
INF|2020-08-03 23:57:55|qcloud_iot_device.c|iot_device_info_set(50): SDK_Ver: 3.2.0, Product_ID: KL4J2K3JZ8, Device_Name: airConditioner1
DBG|2020-08-03 23:57:55|HAL_TLS_mbedtls.c|HAL_TLS_Connect(200): Setting up the SSL/TLS structure...
DBG|2020-08-03 23:57:55|HAL_TLS_mbedtls.c|HAL_TLS_Connect(242): Performing the SSL/TLS handshake...
DBG|2020-08-03 23:57:55|HAL_TLS_mbedtls.c|HAL_TLS_Connect(243): Connecting to /KL4J2K3JZ8.iotcloud.tencentdevices.com/8883...
INF|2020-08-03 23:57:55|HAL_TLS_mbedtls.c|HAL_TLS_Connect(265): connected with /KL4J2K3JZ8.iotcloud.tencentdevices.com/8883...
INF|2020-08-03 23:57:56|mqtt_client.c|IOT_MQTT_Construct(113): mqtt connect with id: 2LxS1 success
INF|2020-08-03 23:57:56|rrpc_sample.c|main(206): Cloud Device Construct Success
DBG|2020-08-03 23:57:56|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(142): topicName=$rrpc/rxd/KL4J2K3JZ8/airConditioner1/+|packet_id=5920
INF|2020-08-03 23:57:56|rrpc_sample.c|_mqtt_event_handler(49): subscribe success, packet-id=5920
DBG|2020-08-03 23:57:56|rrpc_client.c|_rrpc_event_callback(104): rrpc topic subscribe success
```

#### 4. 调用云API `PublishRRPCMessage` 发送RRPC请求消息
打开腾讯云[API控制台](https://console.cloud.tencent.com/api/explorer?Product=iotcloud&Version=2018-06-14&Action=PublishRRPCMessage&SignVersion=)，填写个人密钥和设备参数信息，选择在线调用并发送请求
![image.png](https://main.qcloudimg.com/raw/fe7a7749159b25fe12a07076f9093b79.png)

#### 5. 观察RRPC请求消息
观察设备airConditioner1的打印输出，可以看到已经收到RRPC请求消息，`process id`为469。
```
DBG|2020-08-04 00:07:36|rrpc_client.c|_rrpc_message_cb(85): topic=$rrpc/rxd/KL4J2K3JZ8/airConditioner1/469
INF|2020-08-04 00:07:36|rrpc_client.c|_rrpc_message_cb(86): len=6, topic_msg=closed
INF|2020-08-04 00:07:36|rrpc_client.c|_rrpc_get_process_id(76): len=3, process id=469
INF|2020-08-04 00:07:36|rrpc_sample.c|_rrpc_message_handler(137): rrpc message=closed
```

#### 6. 观察RRPC应答消息
观察设备airConditioner1的打印输出，可以看到已经处理了RRPC请求消息，并回复了RRPC应答消息，`process id`为469。
```
DBG|2020-08-04 00:07:36|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$rrpc/txd/KL4J2K3JZ8/airConditioner1/469|payload=ok
```

#### 7. 观察服务器响应结果
观察服务器的响应结果，可以看到已经收到了RRPC应答消息。`MessageId`为469，`Payload`经过`base64`编码后为`b2s=`，其与客户端实际应答消息经过`base64`编码后一致。可以确认收到了应答消息。
![image.png](https://main.qcloudimg.com/raw/7bbd67cf4529752fc9e125754cc0fa94/image-1.png)

***至此，完成了RRPC同步通信的示例运行***
