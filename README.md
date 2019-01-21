# 腾讯物联网通信 SDK
腾讯物联网通信 SDK 依靠安全且性能强大的数据通道，为物联网领域开发人员提供终端(如传感器, 执行器, 嵌入式设备或智能家电等等)和云端的双向通信能力。

# 快速开始
本节将讲述如何在腾讯物联网通信控制台申请设备, 并结合本 SDK 快速体验设备通过 MQTT+TLS/SSL 协议连接到腾讯云, 发送和接收消息；通过 CoAP+DTLS 协议连接到腾讯云，上报数据。

## 一. 控制台创建设备

#### 1. 注册/登录腾讯云账号
访问[腾讯云登录页面](https://cloud.tencent.com/login?s_url=https%3A%2F%2Fcloud.tencent.com%2F), 点击[立即注册](https://cloud.tencent.com/register?s_url=https%3A%2F%2Fcloud.tencent.com%2F), 免费获取腾讯云账号，若您已有账号，可直接登录。

#### 2. 访问物联网通信控制台
登录后点击右上角控制台，进入控制台后，鼠标悬停在云产品上，弹出层叠菜单，点击物联网通信。

![](https://main.qcloudimg.com/raw/1a0c668f382a13a6ec33a6b4b1dbd8b5.png)

或者直接访问[物联网通信控制台](https://console.qcloud.com/iotcloud)

#### 3. 创建产品和设备
点击页面**创建新产品**按钮, 创建一个产品。

![](https://main.qcloudimg.com/raw/a0da21dc6ac9a9e1dede0077d40cfb22.png)

在弹出的产品窗口中，输入产品名称，选择认证方式，输入产品描述，然后点击创建。

![](https://main.qcloudimg.com/raw/a4cda31d9b3a8309c900c5eac042f350.png)

在生成的产品页面下，点击**设备列表**页面添加新设备。

![](https://main.qcloudimg.com/raw/0530e0da724cd36baefc7011ebce4775.png)

如果产品认证方式为证书认证，输入设备名称成功后，切记点击弹窗中的**下载**按钮，下载所得包中的设备密钥文件和设备证书用于设备连接物联网通信的鉴权。

![](https://main.qcloudimg.com/raw/6592056f1b55fa9262e4b2ab31d0b218.png)

如果产品认证方式为密钥认证，输入设备名称成功后，会在弹窗中显示新添加设备的密钥

![](https://main.qcloudimg.com/raw/fe7a013b1d8c29c477d0ed6d00643751.png)

#### 4. 创建可订阅可发布的Topic

按照**第三步**中进入产品设置页面的方法进入页面后, 点击权限列表，再点击**添加Topic权限**。

![](https://main.qcloudimg.com/raw/65a2d1b7251de37ce1ca2ba334733c57.png)

在弹窗中输入 data, 并设置操作权限为**发布和订阅**，点击创建。

![](https://main.qcloudimg.com/raw/f429b32b12e3cb0cf319b1efe11ccceb.png)

随后将会创建出 productID/\${deviceName}/data 的 Topic，在产品页面的权限列表中可以查看该产品的所有权限。

## 二. 编译示例程序

#### 1. 下载SDK
登录 Linux, 运行如下命令从 github 克隆代码, 或者访问最新[下载](https://github.com/tencentyun/qcloud-iot-sdk-embedded-c/releases)地址, 将下载到的压缩包在 Linux 上解压缩

`git clone https://github.com/tencentyun/qcloud-iot-sdk-embedded-c.git`

#### 2. 填入设备信息
编辑 samples/mqtt/mqtt_sample.c 文件和 samples/coap/coap_sample.c 文件中如下代码段, 填入之前创建产品和设备步骤中得到的 **产品ID**，**设备名称**：

1. 若使用**证书认证**加密方式，填写 **QCLOUD_IOT_CERT_FILENAME** 和 **QCLOUD_IOT_CERT_FILENAME** 并将文件放置在根目录下 certs 目录中。将根目录下 make.settings 文件中的配置项 FEATURE_AUTH_MODE 设置为 CERT，FEATURE_AUTH_WITH_NOTLS 设置为 n。

```
FEATURE_AUTH_MODE        = CERT   # MQTT/CoAP接入认证方式，使用证书认证：CERT；使用密钥认证：KEY
FEATURE_AUTH_WITH_NOTLS  = n      # 接入认证是否不使用TLS，证书方式必须选择使用TLS，密钥认证可选择不使用TLS
```

2. 若使用**秘钥认证**加密方式，填写 **QCLOUD_IOT_DEVICE_SECRET**。将根目录下 make.settings 文件中的配置项 FEATURE_AUTH_MODE 设置为 KEY，FEATURE_AUTH_WITH_NOTLS 设置为 n 时通过 TLS 密钥认证方式连接，设置为 y 时，则通过 HMAC-SHA1 加密算法连接。

![](https://main.qcloudimg.com/raw/17fbbc7c0e4314903b94ffe3da5ee5d0.png)

#### 3. 编译 SDK 产生示例程序
在根目录执行

```
$make clean
$make
```

编译成功完成后, 生成的样例程序在当前目录的 output/release/bin 目录下:

```
output/release/bin/
├── aircond_shadow_sample
├── aircond_shadow_sample_v2
├── certs
│   ├── README.md
│   ├── TEST_CLIENT_cert.crt
│   └── TEST_CLIENT_private.key
├── coap_sample
├── door_coap_sample
├── door_mqtt_sample
├── mqtt_sample
├── ota_mqtt_sample
└── shadow_sample
```

## 三. 运行示例程序

#### 1. 执行 MQTT 示例程序
```
./mqtt_sample
INF|2018-04-27 17:39:35|device.c|iot_device_info_init(37): device info init success!
INF|2018-04-27 17:39:35|device.c|iot_device_info_set(42): start to set device info!
INF|2018-04-27 17:39:35|device.c|iot_device_info_set(66): device info set successfully!
DBG|2018-04-27 17:39:35|mqtt_client.c|qcloud_iot_mqtt_init(185): product_id: QICJYEM1T4
DBG|2018-04-27 17:39:35|mqtt_client.c|qcloud_iot_mqtt_init(186): device_name: Demo1
DBG|2018-04-27 17:39:35|mqtt_client.c|qcloud_iot_mqtt_init(259): cert file: /home/ubuntu/skyztmeng/v2_10_coap_ota/output/release/bin/certs/Demo1_cert.crt
DBG|2018-04-27 17:39:35|mqtt_client.c|qcloud_iot_mqtt_init(260): key file: /home/ubuntu/skyztmeng/v2_10_coap_ota/output/release/bin/certs/Demo1_private.key
DBG|2018-04-27 17:39:35|HAL_TLS_mbedtls.c|HAL_TLS_Connect(204):  Connecting to /iotcloud-mqtt.gz.tencentdevices.com/8883...
DBG|2018-04-27 17:39:35|HAL_TLS_mbedtls.c|HAL_TLS_Connect(209):  Setting up the SSL/TLS structure...
DBG|2018-04-27 17:39:35|HAL_TLS_mbedtls.c|HAL_TLS_Connect(251): Performing the SSL/TLS handshake...
INF|2018-04-27 17:39:35|mqtt_client.c|IOT_MQTT_Construct(107): mqtt connect with id: mgiO5 success
INF|2018-04-27 17:39:35|mqtt_sample.c|main(243): Cloud Device Construct Success
DBG|2018-04-27 17:39:35|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(123): topicName=$sys/operation/result/QICJYEM1T4/Demo1|packet_id=30134|pUserdata=(null)
INF|2018-04-27 17:39:35|system_mqtt.c|system_mqtt_event_handler(56): subscribe success, packet-id=30134
DBG|2018-04-27 17:39:35|system_mqtt.c|IOT_SYSTEM_GET_TIME(155): the count of subscribe is 0, 
DBG|2018-04-27 17:39:35|mqtt_client_publish.c|qcloud_iot_mqtt_publish(325): publish packetID=0|topicName=$sys/operation/QICJYEM1T4/Demo1|payload={"type": "get", "resource": ["time"]}
INF|2018-04-27 17:39:35|system_mqtt.c|on_system_mqtt_message_callback(35): Receive Message With topicName:$sys/operation/result/QICJYEM1T4/Demo1, payload:{"type":"get","time":1524821975}
INF|2018-04-27 17:39:35|system_mqtt.c|on_system_mqtt_message_callback(44): the value of time is 1524821975
INF|2018-04-27 17:39:35|system_mqtt.c|IOT_SYSTEM_GET_TIME(181): receive time info success: 1, yield count is 1
INF|2018-04-27 17:39:35|system_mqtt.c|IOT_SYSTEM_GET_TIME(183): the time is 1524821975
INF|2018-04-27 17:39:35|mqtt_sample.c|main(254): the time is 1524821975
DBG|2018-04-27 17:39:35|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(123): topicName=QICJYEM1T4/Demo1/data|packet_id=30135|pUserdata=(null)
INF|2018-04-27 17:39:35|mqtt_sample.c|event_handler(80): subscribe success, packet-id=30135
DBG|2018-04-27 17:39:35|mqtt_client_publish.c|qcloud_iot_mqtt_publish(317): publish topic seq=30136|topicName=QICJYEM1T4/Demo1/data|payload={"action": "publish_test", "count": "0"}
INF|2018-04-27 17:39:35|mqtt_sample.c|event_handler(107): publish success, packet-id=30136
INF|2018-04-27 17:39:35|mqtt_sample.c|on_message_callback(137): Receive Message With topicName:QICJYEM1T4/Demo1/data, payload:{"action": "publish_test", "count": "0"}
INF|2018-04-27 17:39:36|mqtt_client_connect.c|qcloud_iot_mqtt_disconnect(472): mqtt disconnect!
INF|2018-04-27 17:39:36|mqtt_client.c|IOT_MQTT_Destroy(136): mqtt release!

```

#### 2. 观察消息发送
如下日志信息显示示例程序通过 MQTT 的 Publish 类型消息, 上报数据到 /{productID}/{deviceName}/data 成功。
```
INF|2018-04-27 17:39:35|mqtt_sample.c|event_handler(107): publish success, packet-id=30136
```

#### 3. 观察消息接收
如下日志信息显示该消息因为是到达已被订阅的 Topic, 所以又被服务器原样推送到示例程序, 并进入相应的回调函数
```
INF|2018-04-27 17:39:35|mqtt_sample.c|on_message_callback(137): Receive Message With topicName:QICJYEM1T4/Demo1/data, payload:{"action": "publish_test", "count": "0"}
```

#### 4. 观察控制台日志
可以登录物联网通信控制台, 点击左边导航栏中的**云日志**, 查看刚才上报的消息
![](https://main.qcloudimg.com/raw/5f8e96c89b1828e8e5ae4b35fdc23d5e.png)

#### 5. 执行 CoAP 示例程序
```
./coap_sample
INF|2018-04-27 17:54:17|device.c|iot_device_info_init(37): device info init success!
INF|2018-04-27 17:54:17|device.c|iot_device_info_set(42): start to set device info!
INF|2018-04-27 17:54:17|device.c|iot_device_info_set(66): device info set successfully!
DBG|2018-04-27 17:54:17|coap_client.c|qcloud_iot_coap_init(286): cert file: /home/ubuntu/skyztmeng/v2_10_coap_ota/output/release/bin/certs/Demo1_cert.crt
DBG|2018-04-27 17:54:17|coap_client.c|qcloud_iot_coap_init(287): key file: /home/ubuntu/skyztmeng/v2_10_coap_ota/output/release/bin/certs/Demo1_private.key
INF|2018-04-27 17:54:17|coap_client.c|IOT_COAP_Construct(83): coap connect success
INF|2018-04-27 17:54:17|coap_client_message.c|coap_message_send(404): add coap message id: 56647 into wait list ret: 0
DBG|2018-04-27 17:54:17|coap_client_message.c|_coap_message_handle(297): receive coap piggy ACK message, id 56647
INF|2018-04-27 17:54:17|coap_client_auth.c|_coap_client_auth_callback(42): auth token message success, code_class: 2 code_detail: 5
DBG|2018-04-27 17:54:17|coap_client_auth.c|_coap_client_auth_callback(52): auth_token_len = 11, auth_token = QEWMYSSYUYC
DBG|2018-04-27 17:54:17|coap_client_message.c|_coap_message_list_proc(148): remove the message id 56647 from list
INF|2018-04-27 17:54:17|coap_client_message.c|_coap_message_list_proc(87): remove node
INF|2018-04-27 17:54:17|coap_client.c|IOT_COAP_Construct(92): device auth successfully, connid: I75BN
ERR|2018-04-27 17:54:17|coap_sample.c|main(153): 0x25f0010
INF|2018-04-27 17:54:17|coap_sample.c|main(166): topic name is QICJYEM1T4/Demo1/data
INF|2018-04-27 17:54:17|coap_client_message.c|coap_message_send(404): add coap message id: 56648 into wait list ret: 0
DBG|2018-04-27 17:54:17|coap_sample.c|main(173): client topic has been sent, msg_id: 56648
DBG|2018-04-27 17:54:18|coap_client_message.c|_coap_message_handle(297): receive coap piggy ACK message, id 56648
INF|2018-04-27 17:54:18|coap_sample.c|event_handler(90): message received ACK, msgid: 56648
DBG|2018-04-27 17:54:18|coap_client_message.c|_coap_message_list_proc(148): remove the message id 56648 from list
INF|2018-04-27 17:54:18|coap_client_message.c|_coap_message_list_proc(87): remove node
INF|2018-04-27 17:54:18|coap_client.c|IOT_COAP_Destroy(126): coap release!

```

#### 6. 观察消息发送
如下日志信息显示示例程序通过 CoAP 上报数据到 /{productID}/{deviceName}/data 成功。
```
DBG|2018-04-27 17:54:18|coap_client_message.c|_coap_message_handle(297): receive coap piggy ACK message, id 56648
INF|2018-04-27 17:54:18|coap_sample.c|event_handler(90): message received ACK, msgid: 56648
```

## 四. 可变接入参数配置

可变接入参数配置：SDK 的使用可以根据具体场景需求，配置相应的参数，满足实际业务的运行。可变接入参数包括：
1. MQTT 心跳消息发送周期, 单位: ms 
2. MQTT 阻塞调用(包括连接, 订阅, 发布等)的超时时间, 单位:ms。 建议 5000 ms
3. TLS 连接握手超时时间, 单位: ms
4. MQTT 协议发送消息和接受消息的 buffer 大小默认是 512 字节，最大支持 256 KB
5. CoAP 协议发送消息和接受消息的 buffer 大小默认是 512 字节，最大支持 64 KB
6. 重连最大等待时间
修改 qcloud_iot_export.h 文件如下宏定义可以改变对应接入参数的配置。
```
/* MQTT心跳消息发送周期, 单位:ms */
#define QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL                         (240 * 1000)

/* MQTT 阻塞调用(包括连接, 订阅, 发布等)的超时时间, 单位:ms 建议5000ms */
#define QCLOUD_IOT_MQTT_COMMAND_TIMEOUT                             (5000)

/* TLS连接握手超时时间, 单位:ms */
#define QCLOUD_IOT_TLS_HANDSHAKE_TIMEOUT                            (5000)

/* MQTT消息发送buffer大小, 支持最大256*1024 */
#define QCLOUD_IOT_MQTT_TX_BUF_LEN                                  (512)

/* MQTT消息接收buffer大小, 支持最大256*1024 */
#define QCLOUD_IOT_MQTT_RX_BUF_LEN                                  (512)

/* COAP 发送消息buffer大小，最大支持64*1024字节 */
#define COAP_SENDMSG_MAX_BUFLEN                                     (512)

/* COAP 接收消息buffer大小，最大支持64*1024字节 */
#define COAP_RECVMSG_MAX_BUFLEN                                     (512)

/* 重连最大等待时间 */
#define MAX_RECONNECT_WAIT_INTERVAL                                 (60000)
```

#关于 SDK 的更多使用方式及接口了解, 请访问[官方 WiKi](https://github.com/tencentyun/qcloud-iot-sdk-embedded-c/wiki)