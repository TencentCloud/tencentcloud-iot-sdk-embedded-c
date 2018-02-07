# 腾讯物联云 SDK
腾讯物联云 SDK 依靠安全且性能强大的数据通道，为物联网领域开发人员提供终端(如传感器, 执行器, 嵌入式设备或智能家电等等)和云端的双向通信能力。

# 快速开始
本节将讲述如何在腾讯物联云控制台申请设备, 并结合本 SDK 快速体验设备通过 MQTT+TLS/SSL 协议连接到腾讯云, 发送和接收消息；通过 CoAP+DTLS 协议连接到腾讯云，上报数据。

## 一. 控制台创建设备

#### 1. 注册/登录腾讯云账号
访问[腾讯云登录页面](https://cloud.tencent.com/login?s_url=https%3A%2F%2Fcloud.tencent.com%2F), 点击[立即注册](https://cloud.tencent.com/register?s_url=https%3A%2F%2Fcloud.tencent.com%2F), 免费获取腾讯云账号，若您已有账号，可直接登录。

#### 2. 访问物联云控制台
登录后点击右上角控制台，进入控制台后, 鼠标悬停在云产品上, 弹出层叠菜单。
![](http://qzonestyle.gtimg.cn/qzone/vas/opensns/res/doc/{0603FE05-A96A-41E4-A0B8-AA2D9200928A}.png
)
点击物联云，或直接访问[物联云控制台](https://console.qcloud.com/iotcloud)

#### 3. 创建产品和设备
点击页面**创建新产品**按钮, 创建一个产品, 然后在下方**产品名称**一栏中点击刚刚创建好的产品进入产品设置页面，可在产品设置页面编辑产品描述，之后再**设备列表**页面新建设备。

![](http://qzonestyle.gtimg.cn/qzone/vas/opensns/res/doc/iot_15157295174920.png)

创建设备成功后，切记点击弹窗中的**下载**按钮，下载所得的设备密钥文件用于设备连接物联云的鉴权，点击**管理**，得到设备标识 **设备名**，下载**设备证书**（用于非对称加密连接）。

![](http://qzonestyle.gtimg.cn/qzone/vas/opensns/res/doc/iot_15157296439268.png)

#### 4. 创建可订阅可发布的Topic

按照**第三步**中进入产品设置页面的方法进入页面后, 点击权限列表，再点击**定义Topic权限**, 输入 data, 并设置为可订阅可发布权限，点击创建。

![](http://qzonestyle.gtimg.cn/qzone/vas/opensns/res/doc/20171123172612qq.png
)
随后将会创建出 productName/\${deviceName}/data 的 Topic

## 二. 编译示例程序

#### 1. 下载SDK
登录 Linux, 运行如下命令从 github 克隆代码, 或者访问最新[下载](https://github.com/tencentyun/qcloud-iot-sdk-embedded-c/releases)地址, 将下载到的压缩包在 Linux 上解压缩

`git clone https://github.com/tencentyun/qcloud-iot-sdk-embedded-c.git`

#### 2. 填入设备信息
编辑 samples/mqtt/mqtt_example.c 文件和 samples/coap/coap_sample.c 文件中如下代码段, 填入之前创建产品和设备步骤中得到的 **产品ID**，**设备名称**，

1. 若使用**非对称**加密方式，填写 **QCLOUD_IOT_CERT_FILENAME** 和 **QCLOUD_IOT_CERT_FILENAME** 并将文件放置在根目录下 certs 目录中，
2. 若使用**对称**加密方式，填写 **QCLOUD_IOT_PSK**

![](https://mc.qcloudimg.com/static/img/8bacb4e2ac9e0b149b7fade396c682c8/mqttsampleconfig.png
)

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
├── door_coap_sample
├── coap_sample
├── certs
│   ├── README.md
│   ├── TEST_CLIENT_cert.crt
│   └── TEST_CLIENT_private.key
├── door_mqtt_sample
└── mqtt_sample
```

## 三. 运行示例程序

#### 1. 执行 MQTT 示例程序
```
./mqtt_sample
INF|2018-01-05 20:37:23|device.c|iot_device_info_init(37): device info init success!
INF|2018-01-05 20:37:23|device.c|iot_device_info_set(42): start to set device info!
INF|2018-01-05 20:37:23|device.c|iot_device_info_set(66): device info set successfully!
DBG|2018-01-05 20:37:23|mqtt_client.c|qcloud_iot_mqtt_init(171): product_id: A58FGENPD7
DBG|2018-01-05 20:37:23|mqtt_client.c|qcloud_iot_mqtt_init(172): device_name: Air_0
DBG|2018-01-05 20:37:23|mqtt_client.c|qcloud_iot_mqtt_init(188): cert file: /home/ubuntu/chipengliu/mac_local_code/qcloud-iot-sdk-embedded-c-master/output/release/bin/certs/Air_0_cert.crt
DBG|2018-01-05 20:37:23|mqtt_client.c|qcloud_iot_mqtt_init(189): key file: /home/ubuntu/chipengliu/mac_local_code/qcloud-iot-sdk-embedded-c-master/output/release/bin/certs/Air_0_private.key
INF|2018-01-05 20:37:23|mqtt_client.c|IOT_MQTT_Construct(93): mqtt connect with id: Ip51X success
INF|2018-01-05 20:37:23|mqtt_sample.c|main(237): Cloud Device Construct Success
DBG|2018-01-05 20:37:23|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(124): topicName=A58FGENPD7/Air_0/data|packet_id=43174|pUserdata=(null)
INF|2018-01-05 20:37:24|mqtt_sample.c|event_handler(77): subscribe success, packet-id=43174
DBG|2018-01-05 20:37:25|mqtt_client_publish.c|qcloud_iot_mqtt_publish(317): publish topic seq=43175|topicName=A58FGENPD7/Air_0/data|payload={"action": "publish_test", "count": "0"}
INF|2018-01-05 20:37:25|mqtt_sample.c|on_message_callback(134): Receive Message With topicName:A58FGENPD7/Air_0/data, payload:{"action": "publish_test", "count": "0"}
INF|2018-01-05 20:37:25|mqtt_sample.c|event_handler(104): publish success, packet-id=43175

```

#### 2. 观察消息发送
如下日志信息显示示例程序通过 MQTT 的 Publish 类型消息, 上报数据到 /{productName}/{deviceName}/data 成功。
```
INF|2018-01-05 20:37:25|mqtt_sample.c|event_handler(104): publish success, packet-id=43175
```

#### 3. 观察消息下发
如下日志信息显示该消息因为是到达已被订阅的 Topic, 所以又被服务器原样推送到示例程序, 并进入相应的回调函数
```
INF|2017-11-23 19:38:46|mqtt_sample.c|on_message_callback(76): Receive Message With topicName:shock_test/shock3/data, payload:{"action": "publish_test", "count": "0"}
```

#### 4. 观察控制台日志
可以登录物联云控制台, 点击左边导航栏中的**云日志**, 查看刚才上报的消息
![](http://qzonestyle.gtimg.cn/qzone/vas/opensns/res/doc/iot_1515734324922.png)

#### 5. 执行 CoAP 示例程序
```
./coap_sample
INF|2018-01-11 19:44:52|device.c|iot_device_info_init(37): device info init success!
INF|2018-01-11 19:44:52|device.c|iot_device_info_set(42): start to set device info!
INF|2018-01-11 19:44:52|device.c|iot_device_info_set(66): device info set successfully!
DBG|2018-01-11 19:44:52|coap_client.c|qcloud_iot_coap_init(269): cert file: /home/ubuntu/shockcao/iot_c_test/qcloud-iot-sdk-embedded-c/output/release/bin/certs/shock23_cert.crt
DBG|2018-01-11 19:44:52|coap_client.c|qcloud_iot_coap_init(270): key file: /home/ubuntu/shockcao/iot_c_test/qcloud-iot-sdk-embedded-c/output/release/bin/certs/shock23_private.key
INF|2018-01-11 19:44:52|coap_client.c|IOT_COAP_Construct(75): coap connect success
INF|2018-01-11 19:44:52|coap_client_message.c|coap_message_send(390): add coap message id: 64979 into wait list ret: 0
DBG|2018-01-11 19:44:52|HAL_DTLS_mbedtls.c|HAL_DTLS_Read(356): mbedtls_ssl_read len 59 bytes
DBG|2018-01-11 19:44:52|coap_client_message.c|_coap_message_handle(292): receive coap piggy ACK message, id 64979
INF|2018-01-11 19:44:52|coap_client_auth.c|_coap_client_auth_callback(41): auth token message success, code_class: 2 code_detail: 5
DBG|2018-01-11 19:44:52|coap_client_auth.c|_coap_client_auth_callback(51): auth_token_len = 11, auth_token = DGDEMCLPQG¥ 
DBG|2018-01-11 19:44:52|coap_client_message.c|_coap_message_list_proc(144): remove the message id 64979 from list
INF|2018-01-11 19:44:52|coap_client_message.c|_coap_message_list_proc(85): remove node
INF|2018-01-11 19:44:52|coap_client.c|IOT_COAP_Construct(84): device auth successfully, connid: 0a23A
ERR|2018-01-11 19:44:52|coap_sample.c|main(104): 0x7f1ea6209010
INF|2018-01-11 19:44:52|coap_sample.c|main(116): topic name is /19C09FD2UU/shock23/control
INF|2018-01-11 19:44:52|coap_client_message.c|coap_message_send(390): add coap message id: 64980 into wait list ret: 0
DBG|2018-01-11 19:44:52|coap_sample.c|main(123): client topic has been sent, msg_id: 64980
DBG|2018-01-11 19:44:53|HAL_DTLS_mbedtls.c|HAL_DTLS_Read(356): mbedtls_ssl_read len 51 bytes
DBG|2018-01-11 19:44:53|coap_client_message.c|_coap_message_handle(292): receive coap piggy ACK message, id 64980
INF|2018-01-11 19:44:53|coap_sample.c|event_handler(45): message received ACK, msgid: 64980
DBG|2018-01-11 19:44:53|coap_client_message.c|_coap_message_list_proc(144): remove the message id 64980 from list
INF|2018-01-11 19:44:53|coap_client_message.c|_coap_message_list_proc(85): remove node
INF|2018-01-11 19:44:53|coap_client.c|IOT_COAP_Destroy(118): coap release!

```

#### 6. 观察消息发送
如下日志信息显示示例程序通过 CoAP 上报数据到 /{productId}/{deviceName}/data 成功。
```
DBG|2018-01-11 19:44:53|/home/ubuntu/shockcao/iot_c_test/qcloud-iot-sdk-embedded-c/src/coap/src/coap_client_message.c|_coap_message_handle(292): receive coap piggy ACK message, id 64980
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