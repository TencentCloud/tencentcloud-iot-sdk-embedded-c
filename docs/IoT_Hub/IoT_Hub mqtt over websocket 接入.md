# mqtt over websocket 接入

本文档介绍 CSDK 通过 MQTT over Websocket 接入 IoT Hub 物联网通信平台的功能

## 一. 描述

#### 1. 背景介绍

因部分环境限制，设备只能访问 80、443 等 http 协议端口，因此 CSDK 增加 mqtt over websocket 功能。

#### 2. mqtt over websocket 接入介绍

##### 2.1 平台 mqtt over websocket 接入文档链接

参考文档：[设备基于 WebSocket 的 MQTT 接入](https://cloud.tencent.com/document/product/634/46347)

##### 2.2 CSDK mqtt over websocket 接入实现

1. 443 端口，不管设备是密钥认证或者证书认证，https 接入都使用密钥协商机制，不校验 CA 证书与 client 证书


2. 密钥认证设备，mqtt 连接报文中的 password 需要通过平台生成的设备密钥 base64 decode 后的数据对 username 进行 hmacsha1 的计算得到，password 需要 hex 十六进制形式。


3. 证书认证设备，mqtt 连接报文中的 password 需要通过平台生成的设备私钥文件对 username 进行 rsa-sha256 的计算得到，password 需要 hex 十六进制形式。

## 二. 编译运行示例程序(以**密钥认证设备**为例)

#### 1. 编译 SDK

修改CMakeLists.txt确保以下选项存在
```
set(BUILD_TYPE                   "release")
set(COMPILE_TOOLS                "gcc")
set(PLATFORM 	                 "linux")
set(FEATURE_WEBSOCKET_MQTT ON)
set(FEATURE_AUTH_MODE "KEY")
set(FEATURE_AUTH_WITH_NOTLS OFF)
set(FEATURE_DEBUG_DEV_INFO_USED  OFF)
```
执行脚本编译
```
./cmake_build.sh 
```
示例输出 `mqtt_sample` 位于 `output/release/bin` 文件夹中

#### 2. 填写设备信息

device_info.json 中:
```
{
    "auth_mode":"KEY",
    "region": "china",
    "productId":"7CN3UQ4VGT",
    "deviceName":"test1",
    "key_deviceinfo":{    
        "deviceSecret":"xxxxxxxxxxxx"
    }
}
```

#### 3. 执行`mqtt_sample`示例程序

可以看到设备先进行了 443 端口连接与 http upgrade websocket，然后成功接入平台并订阅 topic 和发布数据。

```
INF|2022-08-11 11:42:50|qcloud_iot_device.c|iot_device_info_set(49): SDK_Ver: 3.2.3, Product_ID: 7CN3UQ4VGT, Device_Name: test1
DBG|2022-08-11 11:42:50|utils_websocket_client.c|_network_transport_init(47): init 7CN3UQ4VGT.ap-guangzhou.iothub.tencentdevices.com:443, TLS
DBG|2022-08-11 11:42:50|utils_websocket_client.c|_network_transport_connect(53): start connect 7CN3UQ4VGT.ap-guangzhou.iothub.tencentdevices.com:443
DBG|2022-08-11 11:42:50|HAL_TLS_mbedtls.c|HAL_TLS_Connect(297): Setting up the SSL/TLS structure...
DBG|2022-08-11 11:42:50|HAL_TLS_mbedtls.c|HAL_TLS_Connect(346): Performing the SSL/TLS handshake...
DBG|2022-08-11 11:42:50|HAL_TLS_mbedtls.c|HAL_TLS_Connect(347): Connecting to /7CN3UQ4VGT.ap-guangzhou.iothub.tencentdevices.com/443...
INF|2022-08-11 11:42:50|HAL_TLS_mbedtls.c|HAL_TLS_Connect(371): connected with /7CN3UQ4VGT.ap-guangzhou.iothub.tencentdevices.com/443...
DBG|2022-08-11 11:42:50|utils_websocket_client.c|_network_transport_connect(59): connected 7CN3UQ4VGT.ap-guangzhou.iothub.tencentdevices.com:443, handle:0x556d5a67bda0
DBG|2022-08-11 11:42:50|utils_websocket_client.c|Utils_WSClient_connect(218): header: GET / HTTP/1.1
Host: 7CN3UQ4VGT.ap-guangzhou.iothub.tencentdevices.com:443
Upgrade: websocket
Connection: Upgrade
Sec-Websocket-Protocol: mqtt
Sec-WebSocket-Key: MTIzNDU2Nzg5MGFiY2RlZg==
Sec-WebSocket-Version: 13


ERR|2022-08-11 11:42:51|utils_websocket_client.c|Utils_WSClient_connect(246): websocket resp code :101
DBG|2022-08-11 11:42:51|network_socket.c|network_websocket_mqtt_connect(211): websocket conn create
DBG|2022-08-11 11:42:51|network_socket.c|_websocket_mqtt_recv_msg_callback(166): actual recv len:4
INF|2022-08-11 11:42:51|mqtt_client.c|IOT_MQTT_Construct(120): mqtt connect with id: jig9c success
INF|2022-08-11 11:42:51|mqtt_sample.c|main(312): Cloud Device Construct Success
DBG|2022-08-11 11:42:51|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(142): topicName=$sys/operation/result/7CN3UQ4VGT/test1|packet_id=13922
DBG|2022-08-11 11:42:51|network_socket.c|_websocket_mqtt_recv_msg_callback(166): actual recv len:5
DBG|2022-08-11 11:42:51|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(142): topicName=$sys/operation/result/7CN3UQ4VGT/test1|packet_id=13923
INF|2022-08-11 11:42:51|mqtt_sample.c|_mqtt_event_handler(50): subscribe success, packet-id=13922
DBG|2022-08-11 11:42:51|system_mqtt.c|_system_mqtt_sub_event_handler(118): mqtt sys topic subscribe success
DBG|2022-08-11 11:42:51|network_socket.c|_websocket_mqtt_recv_msg_callback(166): actual recv len:5
DBG|2022-08-11 11:42:51|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$sys/operation/7CN3UQ4VGT/test1|payload={"type": "get", "resource": ["time"]}
WRN|2022-08-11 11:42:51|mqtt_client_common.c|_handle_suback_packet(1008): Identical topic found: $sys/operation/result/7CN3UQ4VGT/test1
INF|2022-08-11 11:42:51|mqtt_sample.c|_mqtt_event_handler(50): subscribe success, packet-id=13923
DBG|2022-08-11 11:42:51|system_mqtt.c|_system_mqtt_sub_event_handler(118): mqtt sys topic subscribe success
DBG|2022-08-11 11:42:51|network_socket.c|_websocket_mqtt_recv_msg_callback(166): actual recv len:124
DBG|2022-08-11 11:42:51|system_mqtt.c|_system_mqtt_message_callback(65): Recv Msg Topic:$sys/operation/result/7CN3UQ4VGT/test1, payload:{"type":"get","time":1660189367,"ntptime1":1660189367410,"ntptime2":1660189367410}
INF|2022-08-11 11:42:52|mqtt_sample.c|main(324): system time is 1660189367
DBG|2022-08-11 11:42:52|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$sys/operation/7CN3UQ4VGT/test1|payload={"type": "get", "resource": ["time"]}
DBG|2022-08-11 11:42:52|network_socket.c|_websocket_mqtt_recv_msg_callback(166): actual recv len:124
DBG|2022-08-11 11:42:52|system_mqtt.c|_system_mqtt_message_callback(65): Recv Msg Topic:$sys/operation/result/7CN3UQ4VGT/test1, payload:{"type":"get","time":1660189367,"ntptime1":1660189367685,"ntptime2":1660189367685}
ERR|2022-08-11 11:42:52|system_mqtt.c|IOT_Sync_NTPTime(322): set systime ms failed, timestamp 1660189367834, please check permission or other ret :-1
INF|2022-08-11 11:42:52|mqtt_sample.c|main(332): sync ntp time success!
DBG|2022-08-11 11:42:52|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(142): topicName=7CN3UQ4VGT/test1/data|packet_id=13924
DBG|2022-08-11 11:42:52|network_socket.c|_websocket_mqtt_recv_msg_callback(166): actual recv len:5
INF|2022-08-11 11:42:52|mqtt_sample.c|_mqtt_event_handler(50): subscribe success, packet-id=13924
DBG|2022-08-11 11:42:53|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=13925|topicName=7CN3UQ4VGT/test1/data|payload={"action": "publish_test", "count": "0"}
DBG|2022-08-11 11:42:53|network_socket.c|_websocket_mqtt_recv_msg_callback(166): actual recv len:4
INF|2022-08-11 11:42:53|mqtt_sample.c|_mqtt_event_handler(74): publish success, packet-id=13925
DBG|2022-08-11 11:42:53|network_socket.c|network_websocket_mqtt_disconnect(283): websocket disconn
INF|2022-08-11 11:42:53|mqtt_client_connect.c|qcloud_iot_mqtt_disconnect(503): mqtt disconnect!
INF|2022-08-11 11:42:53|system_mqtt.c|_system_mqtt_sub_event_handler(137): mqtt client has been destroyed
INF|2022-08-11 11:42:53|mqtt_client.c|IOT_MQTT_Destroy(184): mqtt release!
```

#### 4. 控制台在线调试下发数据

控制台在线调试下发数据![image.png](https://qcloudimg.tencent-cloud.cn/raw/1eda2717dde0e59cb8452a7887e3e22e.png)

`Receive Message With topicName:7CN3UQ4VGT/test1/data, payload:mqtt over websocket cloud publish data`

```
DBG|2022-08-11 11:47:57|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=58221|topicName=7CN3UQ4VGT/test1/data|payload={"action": "publish_test", "count": "47"}
DBG|2022-08-11 11:47:57|network_socket.c|_websocket_mqtt_recv_msg_callback(166): actual recv len:4
INF|2022-08-11 11:47:57|mqtt_sample.c|_mqtt_event_handler(74): publish success, packet-id=58221
DBG|2022-08-11 11:47:59|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=58222|topicName=7CN3UQ4VGT/test1/data|payload={"action": "publish_test", "count": "48"}
DBG|2022-08-11 11:47:59|network_socket.c|_websocket_mqtt_recv_msg_callback(166): actual recv len:4
INF|2022-08-11 11:47:59|mqtt_sample.c|_mqtt_event_handler(74): publish success, packet-id=58222
DBG|2022-08-11 11:48:00|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=58223|topicName=7CN3UQ4VGT/test1/data|payload={"action": "publish_test", "count": "49"}
DBG|2022-08-11 11:48:00|network_socket.c|_websocket_mqtt_recv_msg_callback(166): actual recv len:4
INF|2022-08-11 11:48:01|mqtt_sample.c|_mqtt_event_handler(74): publish success, packet-id=58223
DBG|2022-08-11 11:48:02|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=58224|topicName=7CN3UQ4VGT/test1/data|payload={"action": "publish_test", "count": "50"}
DBG|2022-08-11 11:48:02|network_socket.c|_websocket_mqtt_recv_msg_callback(166): actual recv len:4
INF|2022-08-11 11:48:02|mqtt_sample.c|_mqtt_event_handler(74): publish success, packet-id=58224
DBG|2022-08-11 11:48:04|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=58225|topicName=7CN3UQ4VGT/test1/data|payload={"action": "publish_test", "count": "51"}
DBG|2022-08-11 11:48:04|network_socket.c|_websocket_mqtt_recv_msg_callback(166): actual recv len:63
DBG|2022-08-11 11:48:04|network_socket.c|_websocket_mqtt_recv_msg_callback(166): actual recv len:4
INF|2022-08-11 11:48:04|mqtt_sample.c|_on_message_callback(172): Receive Message With topicName:7CN3UQ4VGT/test1/data, payload:mqtt over websocket cloud publish data
INF|2022-08-11 11:48:04|mqtt_sample.c|_mqtt_event_handler(74): publish success, packet-id=58225
DBG|2022-08-11 11:48:05|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=58226|topicName=7CN3UQ4VGT/test1/data|payload={"action": "publish_test", "count": "52"}
DBG|2022-08-11 11:48:05|network_socket.c|_websocket_mqtt_recv_msg_callback(166): actual recv len:4
INF|2022-08-11 11:48:05|mqtt_sample.c|_mqtt_event_handler(74): publish success, packet-id=58226
DBG|2022-08-11 11:48:07|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=58227|topicName=7CN3UQ4VGT/test1/data|payload={"action": "publish_test", "count": "53"}
DBG|2022-08-11 11:48:07|network_socket.c|_websocket_mqtt_recv_msg_callback(166): actual recv len:4
INF|2022-08-11 11:48:07|mqtt_sample.c|_mqtt_event_handler(74): publish success, packet-id=58227
```
