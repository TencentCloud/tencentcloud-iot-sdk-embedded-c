# 远程登录 SSH

本文档介绍通过 MQTT 开启远程登录设备 SSH 的功能，并结合 CSDK 的 **mqtt_sample** 展示远程登录 SSH 功能。

## 一. 功能介绍

#### 1. 背景介绍

设备运维阶段，需要远程登录到设备上进行运维操作；但是设备做为 SSH Server 时因大多数处于内网中而造成远程登录困难。
腾讯云物联网通信平台提供的远程登录 SSH 功能是在具有 SSH Server 能力的设备上，CSDK 通过 websocket 与平台建立连接后使用 websocket 通道透传 SSH 指令， CSDK 与 SSH Server 建立本地 TCP 连接转发 SSH 数据。
以这种间接的方式满足远程登录设备的需求。
* 远程登录订阅 topic ：`$sys/operation/get/${productid}/${devicename}`
* 远程登录发布 topic : `$sys/operation/${productID}/${deviceName}`

#### 2. 远程登录 SSH 流程图

![image.png](https://qcloudimg.tencent-cloud.cn/raw/1c7006f1d841207b45671172c5578fcd.png)

## 二. 编译运行示例程序(以**密钥认证设备**为例)

#### 1. 编译 SDK

修改CMakeLists.txt确保以下选项存在
```
set(BUILD_TYPE                   "release")
set(COMPILE_TOOLS                "gcc")
set(PLATFORM 	                 "linux")
set(FEATURE_MULTITHREAD_ENABLED ON)
set(FEATURE_SYSTEM_COMM_ENABLED ON)
set(FEATURE_REMOTE_LOGIN_WEBSOCKET_SSH ON)
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
    "productId":"6TK8TYKEIA",
    "deviceName":"deng1",
    "key_deviceinfo":{    
        "deviceSecret":"xxxxxxxxxxxx"
    }
}
```

#### 3. 执行`mqtt_sample`示例程序

可以看到设备 deng1 订阅了远程登录 topic。

```
./output/release/bin/remote_config_mqtt_sample -l 9999999
INF|2021-11-03 20:57:30|qcloud_iot_device.c|iot_device_info_set(49): SDK_Ver: 3.2.2, Product_ID: 6TK8TYKEIA, Device_Name: deng1
DBG|2021-11-03 20:57:30|HAL_TLS_mbedtls.c|HAL_TLS_Connect(273): Setting up the SSL/TLS structure...
DBG|2021-11-03 20:57:30|HAL_TLS_mbedtls.c|HAL_TLS_Connect(315): Performing the SSL/TLS handshake...
DBG|2021-11-03 20:57:30|HAL_TLS_mbedtls.c|HAL_TLS_Connect(316): Connecting to /6TK8TYKEIA.iotcloud.tencentdevices.com/8883...
INF|2021-11-03 20:57:31|HAL_TLS_mbedtls.c|HAL_TLS_Connect(338): connected with /6TK8TYKEIA.iotcloud.tencentdevices.com/8883...
INF|2021-11-03 20:57:31|mqtt_client.c|IOT_MQTT_Construct(118): mqtt connect with id: GmxF9 success
INF|2021-11-03 20:57:31|mqtt_sample.c|main(311): Cloud Device Construct Success
DBG|2021-11-03 20:57:31|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(142): topicName=$sys/operation/result/6TK8TYKEIA/deng1|packet_id=7454
INF|2021-11-03 20:57:31|mqtt_sample.c|_mqtt_event_handler(50): subscribe success, packet-id=7454
DBG|2021-11-03 20:57:31|system_mqtt.c|_system_mqtt_sub_event_handler(118): mqtt sys topic subscribe success
```

#### 4. 控制台上进行远程登录
![image.png](https://qcloudimg.tencent-cloud.cn/raw/79b2ee63f1dde9e21fae6404e003106c.png)

#### 5. 观察设备开启 websocket 连接

```
DBG|2021-11-03 21:00:07|system_mqtt.c|_system_mqtt_message_callback(65): Recv Msg Topic:$sys/operation/result/6TK8TYKEIA/deng1, payload:{"type":"ssh","switch":1}
DBG|2021-11-03 21:00:07|system_mqtt_ssh_proxy.c|IOT_QCLOUD_SSH_Start(947): create qcloud_ssh thread success!
DBG|2021-11-03 21:00:07|system_mqtt_ssh_proxy.c|_network_tcp_init(117): init gateway.tencentdevices.com:443, TLS
DBG|2021-11-03 21:00:07|system_mqtt_ssh_proxy.c|_network_tcp_connect(123): start connect gateway.tencentdevices.com:443
DBG|2021-11-03 21:00:07|HAL_TLS_mbedtls.c|_mbedtls_client_init(204): psk/pskid is empty!|psk=null|psd_id=null
DBG|2021-11-03 21:00:07|HAL_TLS_mbedtls.c|HAL_TLS_Connect(273): Setting up the SSL/TLS structure...
DBG|2021-11-03 21:00:07|HAL_TLS_mbedtls.c|HAL_TLS_Connect(315): Performing the SSL/TLS handshake...
DBG|2021-11-03 21:00:07|HAL_TLS_mbedtls.c|HAL_TLS_Connect(316): Connecting to /gateway.tencentdevices.com/443...
INF|2021-11-03 21:00:08|HAL_TLS_mbedtls.c|HAL_TLS_Connect(338): connected with /gateway.tencentdevices.com/443...
DBG|2021-11-03 21:00:08|system_mqtt_ssh_proxy.c|_network_tcp_connect(129): connected gateway.tencentdevices.com:443, handle:0x7efcdc000b60
ERR|2021-11-03 21:00:08|system_mqtt_ssh_proxy.c|IOT_WebSocket_connect(284): websocket resp code :101
DBG|2021-11-03 21:00:08|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$sys/operation/6TK8TYKEIA/deng1|payload={"type": "ssh", "switch": 1}
DBG|2021-11-03 21:00:08|system_mqtt_ssh_proxy.c|_websocket_ssh_proc_verifydevcie(763): send packet:{"requestId":"ws-00000000","msgType":8,"payloadLen":167,"serviceType":0,"timestmap":1635944408,"token":"ws-token-103545508"}
{"productId":"6TK8TYKEIA","deviceName":"deng1","timestamp":1635944408,"rand":"103545508","version":"1.0","signMethod":"hmacsha1","sign":"2uRTw5WIgIGuQKuA1pM2Z2sq9m4="}
DBG|2021-11-03 21:00:08|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :293
DBG|2021-11-03 21:00:08|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 113, {"requestId":"ws-00000000","msgType":9,"payloadLen":0,"serviceType":0,"timestamp":0,"token":"ws-token-103545508"}
ERR|2021-11-03 21:00:08|system_mqtt_ssh_proxy.c|_websocket_ssh_proc_verifyresult(788): verify success
DBG|2021-11-03 21:00:08|system_mqtt_ssh_proxy.c|_websocket_ssh_proc_ping(659): send packet:{"requestId":"ws-00000001","msgType":4,"payloadLen":0,"serviceType":0,"timestmap":1635944408,"token":"wsping-e5e2a792"}

DBG|2021-11-03 21:00:08|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :121
DBG|2021-11-03 21:00:09|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 110, {"requestId":"ws-00000001","msgType":5,"payloadLen":0,"serviceType":0,"timestamp":0,"token":"wsping-e5e2a792"}8"}
```
控制台界面
![image.png](https://qcloudimg.tencent-cloud.cn/raw/a350ae54c0ce2f53bdf1202bf0491aeb.png)

#### 6. 观察 SSH 透传日志

```
DBG|2021-11-03 21:12:24|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 131, {"requestId":"test","msgType":0,"payloadLen":0,"serviceType":0,"timestamp":1635945143,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}

DBG|2021-11-03 21:12:24|system_mqtt_ssh_proxy.c|_network_tcp_init(117): init 127.0.0.1:22, TCP
DBG|2021-11-03 21:12:24|system_mqtt_ssh_proxy.c|_local_ssh_create(417): new ssh session wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8
DBG|2021-11-03 21:12:24|system_mqtt_ssh_proxy.c|_websocket_ssh_proc_newsession_resp(592): send packet:{"requestId":"ws-0000001a","msgType":1,"payloadLen":30,"serviceType":0,"timestmap":1635945144,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
{"code":0,"message":"success"}
DBG|2021-11-03 21:12:24|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :169
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 172, {"requestId":"K0RcyxzcFTBhBhGFu0GPlVoHPK4IS6f9","msgType":6,"payloadLen":12,"serviceType":0,"timestamp":1635945149,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
SSH-2.0-Go

DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_network_tcp_connect(123): start connect 127.0.0.1:22
INF|2021-11-03 21:12:30|HAL_TCP_linux.c|HAL_TCP_Connect(105): connected with TCP server: 127.0.0.1:22
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_network_tcp_connect(129): connected 127.0.0.1:22, handle:0x5
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_local_ssh_send(495): send data to ssh 12
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_local_ssh_recv(549): recv from local ssh 512
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_websocket_ssh_send_rawdata(510): send packet:{"requestId":"ws-0000001b","msgType":6,"payloadLen":512,"serviceType":0,"timestmap":1635945150,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
, 140, 512
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :652
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_local_ssh_recv(549): recv from local ssh 512
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_websocket_ssh_send_rawdata(510): send packet:{"requestId":"ws-0000001c","msgType":6,"payloadLen":512,"serviceType":0,"timestmap":1635945150,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
, 140, 512
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :652
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_local_ssh_recv(549): recv from local ssh 73
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_websocket_ssh_send_rawdata(510): send packet:{"requestId":"ws-0000001d","msgType":6,"payloadLen":73,"serviceType":0,"timestmap":1635945150,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
, 139, 73
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :212
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 961, {"requestId":"3mSF99CEJseFjWvIlrqJDAdxHvZFpweY","msgType":6,"payloadLen":800,"serviceType":0,"timestamp":1635945149,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}

DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_local_ssh_send(495): send data to ssh 800
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 208, {"requestId":"pM97jo04y4SCkR61f3OucNB3kGDvFfVA","msgType":6,"payloadLen":48,"serviceType":0,"timestamp":1635945149,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}

DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_local_ssh_send(495): send data to ssh 48
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_local_ssh_recv(549): recv from local ssh 280
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_websocket_ssh_send_rawdata(510): send packet:{"requestId":"ws-0000001e","msgType":6,"payloadLen":280,"serviceType":0,"timestmap":1635945150,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
, 140, 280
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :420
DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 176, {"requestId":"0lKxfLGEFTQvQFnsjhBRoJoXRWFprcM5","msgType":6,"payloadLen":16,"serviceType":0,"timestamp":1635945149,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}

DBG|2021-11-03 21:12:30|system_mqtt_ssh_proxy.c|_local_ssh_send(495): send data to ssh 16
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 212, {"requestId":"ztB012AOHM049RcvT4KnRbSKX1RccaXd","msgType":6,"payloadLen":52,"serviceType":0,"timestamp":1635945149,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}

DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_local_ssh_send(495): send data to ssh 52
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_local_ssh_recv(549): recv from local ssh 52
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_websocket_ssh_send_rawdata(510): send packet:{"requestId":"ws-0000001f","msgType":6,"payloadLen":52,"serviceType":0,"timestmap":1635945151,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
, 139, 52
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :191
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 228, {"requestId":"Q2W5F3m5TzOXGv2beoeMzxxt7FMuojIt","msgType":6,"payloadLen":68,"serviceType":0,"timestamp":1635945150,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}

DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_local_ssh_send(495): send data to ssh 68
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_local_ssh_recv(549): recv from local ssh 52
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_websocket_ssh_send_rawdata(510): send packet:{"requestId":"ws-00000020","msgType":6,"payloadLen":52,"serviceType":0,"timestmap":1635945151,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
, 139, 52
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :191
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 244, {"requestId":"E8VGeZfn33v5tgRymwDQaFXKes0TrQTh","msgType":6,"payloadLen":84,"serviceType":0,"timestamp":1635945150,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}

DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_local_ssh_send(495): send data to ssh 84
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_local_ssh_recv(549): recv from local ssh 36
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_websocket_ssh_send_rawdata(510): send packet:{"requestId":"ws-00000021","msgType":6,"payloadLen":36,"serviceType":0,"timestmap":1635945151,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
, 139, 36
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :175
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 212, {"requestId":"WGPUQpSE3aKZrdvJ7MQ0BxQ6wd030kca","msgType":6,"payloadLen":52,"serviceType":0,"timestamp":1635945150,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}

DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_local_ssh_send(495): send data to ssh 52
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_local_ssh_recv(549): recv from local ssh 512
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_websocket_ssh_send_rawdata(510): send packet:{"requestId":"ws-00000022","msgType":6,"payloadLen":512,"serviceType":0,"timestmap":1635945151,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
, 140, 512
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :652
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_local_ssh_recv(549): recv from local ssh 40
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_websocket_ssh_send_rawdata(510): send packet:{"requestId":"ws-00000023","msgType":6,"payloadLen":40,"serviceType":0,"timestmap":1635945151,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
, 139, 40
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :179
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 261, {"requestId":"4aXKedUIuzG1lgqHaadgbIyOPue0h7dd","msgType":6,"payloadLen":100,"serviceType":0,"timestamp":1635945150,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}

DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_local_ssh_send(495): send data to ssh 100
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_local_ssh_recv(549): recv from local ssh 36
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_websocket_ssh_send_rawdata(510): send packet:{"requestId":"ws-00000024","msgType":6,"payloadLen":36,"serviceType":0,"timestmap":1635945151,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
, 139, 36
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :175
DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 212, {"requestId":"sS52JaNaWCMFNqbUI0TBoTutkWhubgcq","msgType":6,"payloadLen":52,"serviceType":0,"timestamp":1635945150,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}

DBG|2021-11-03 21:12:31|system_mqtt_ssh_proxy.c|_local_ssh_send(495): send data to ssh 52
DBG|2021-11-03 21:12:32|system_mqtt_ssh_proxy.c|_local_ssh_recv(549): recv from local ssh 512
DBG|2021-11-03 21:12:32|system_mqtt_ssh_proxy.c|_websocket_ssh_send_rawdata(510): send packet:{"requestId":"ws-00000025","msgType":6,"payloadLen":512,"serviceType":0,"timestmap":1635945152,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
, 140, 512
DBG|2021-11-03 21:12:32|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :652
DBG|2021-11-03 21:12:32|system_mqtt_ssh_proxy.c|_local_ssh_recv(549): recv from local ssh 28
DBG|2021-11-03 21:12:32|system_mqtt_ssh_proxy.c|_websocket_ssh_send_rawdata(510): send packet:{"requestId":"ws-00000026","msgType":6,"payloadLen":28,"serviceType":0,"timestmap":1635945152,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
, 139, 28
DBG|2021-11-03 21:12:32|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :167
DBG|2021-11-03 21:12:32|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 196, {"requestId":"Q6x7v6NTSFljaTSKCA7ljkbUUsr21aPU","msgType":6,"payloadLen":36,"serviceType":0,"timestamp":1635945150,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}

DBG|2021-11-03 21:12:32|system_mqtt_ssh_proxy.c|_local_ssh_send(495): send data to ssh 36
DBG|2021-11-03 21:12:32|system_mqtt_ssh_proxy.c|_local_ssh_recv(549): recv from local ssh 116
DBG|2021-11-03 21:12:32|system_mqtt_ssh_proxy.c|_websocket_ssh_send_rawdata(510): send packet:{"requestId":"ws-00000027","msgType":6,"payloadLen":116,"serviceType":0,"timestmap":1635945152,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
, 140, 116
DBG|2021-11-03 21:12:32|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :256
DBG|2021-11-03 21:12:32|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 196, {"requestId":"Ruc8ySMvC07s6yyvln8pRFxIQp0glSA7","msgType":6,"payloadLen":36,"serviceType":0,"timestamp":1635945151,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}

DBG|2021-11-03 21:12:32|system_mqtt_ssh_proxy.c|_local_ssh_send(495): send data to ssh 36
DBG|2021-11-03 21:12:41|system_mqtt_ssh_proxy.c|_websocket_ssh_proc_ping(659): send packet:{"requestId":"ws-00000028","msgType":4,"payloadLen":0,"serviceType":0,"timestmap":1635945161,"token":"wsping-e5ee237b"}
```
控制台界面
![image.png](https://qcloudimg.tencent-cloud.cn/raw/a9ac64d19613283af0515c0bd6111013.png)

#### 7. 观察远程登录关闭日志

```
DBG|2021-11-03 21:14:48|system_mqtt_ssh_proxy.c|_websocket_on_msg_recv_callback(232): 180, {"requestId":"release-session","msgType":2,"payloadLen":37,"serviceType":0,"timestamp":1635945287,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
{"code":5,"msg":"前端关闭连接"}
DBG|2021-11-03 21:14:48|system_mqtt_ssh_proxy.c|_local_ssh_destory(448): destory ssh session wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8
DBG|2021-11-03 21:14:48|system_mqtt_ssh_proxy.c|_websocket_ssh_proc_releasesession_resp(623): send packet:{"requestId":"ws-0000002d","msgType":3,"payloadLen":30,"serviceType":0,"timestmap":1635945288,"token":"wo3oxP2Qyk6mvvKw14UbftlLbMvf1Mh8"}
{"code":0,"message":"success"}
DBG|2021-11-03 21:14:48|system_mqtt_ssh_proxy.c|IOT_WebSocket_send(325): websocket send :169
DBG|2021-11-03 21:14:48|system_mqtt.c|_system_mqtt_message_callback(65): Recv Msg Topic:$sys/operation/result/6TK8TYKEIA/deng1, payload:{"type":"ssh","switch":0}
DBG|2021-11-03 21:14:48|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$sys/operation/6TK8TYKEIA/deng1|payload={"type": "ssh", "switch": 0}
```

## 三. 远程登录 CSDK 相关文件
```
sdk_src/services/ssh/system_mqtt_ssh_proxy.c
sdk_src/services/system/system_mqtt.c
internal_inc/qcloud_iot_common.h

#define LOCAL_SSH_PORT     22          若 sshd 服务监听端口非 22 请修改此
#define LOCAL_SSH_IP       "127.0.0.1"  若想做为跳板连接其他设备修改此 IP 为目标设备 IP

使用该功能时以下两个开关都要设置为 ON
set(FEATURE_SYSTEM_COMM_ENABLED ON)
set(FEATURE_REMOTE_LOGIN_SSH ON)
```
