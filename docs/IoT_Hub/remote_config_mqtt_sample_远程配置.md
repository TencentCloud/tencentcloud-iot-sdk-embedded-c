# 远程配置

本文档介绍通过 MQTT 对产品设备进行远程配置的功能，并结合 SDK 的**remote_config_mqtt_sample**展示远程配置功能。

## 一. 控制台创建设备

#### 1. 背景介绍

设备运维阶段，需要对设备配置参数进行更新以满足现场使用环境的需求。
如温湿度设备需要更改温湿度上下限的设置；modbus 网关设备需要更改串口参数及 modbus 参数信息；zigbee 网关需要更改 zigbee 配置参数信息等等。
* 配置推送 topic 和配置请求响应 topic： `$config/update/${productID}/${deviceName}`
* 配置请求 topic: `$config/report/${productID}/${deviceName}`

#### 2. 远程配置流程图

##### 2.1 云平台主动下发配置流程

![image.png](https://main.qcloudimg.com/raw/5000bef6371f1080bc44deba57e34565.png)

##### 2.2 设备主动请求配置更新

![image.png](https://main.qcloudimg.com/raw/c0236c38e9e46b8e99424fa96f89c605.png)

#### 3. 创建产品和设备

请参考[设备互通](https://cloud.tencent.com/document/product/634/11913) 创建 AirConditioner 空调产品和设备标题内容。
创建 airConditioner1 空调设备。

## 二. 编译运行示例程序(以**密钥认证设备**为例)

#### 1. 编译 SDK

修改CMakeLists.txt确保以下选项存在
```
set(BUILD_TYPE                   "release")
set(COMPILE_TOOLS                "gcc")
set(PLATFORM 	                 "linux")
set(FEATURE_REMOTE_CONFIG_MQTT_ENABLED ON)
set(FEATURE_AUTH_MODE "KEY")
set(FEATURE_AUTH_WITH_NOTLS OFF)
set(FEATURE_DEBUG_DEV_INFO_USED  OFF)
```
执行脚本编译
```
./cmake_build.sh 
```
示例输出 `remote_config_mqtt_sample` 位于 `output/release/bin` 文件夹中

#### 2. 填写设备信息

将上面创建的 airConditioner1 设备的设备信息填写到JSON文件 aircond_device_info1.json 中:
```
{
    "auth_mode":"KEY",	
    "productId":"YI7XCD5DRH",
    "deviceName":"airConditioner1",	
    "key_deviceinfo":{    
        "deviceSecret":"+PBSkYlvNffjLDTGLpVOQA=="
    }
}
```

#### 3. 执行`remote_config_mqtt_sample`示例程序

可以看到设备 airConditioner1 订阅了远程配置消息。

```
./output/release/bin/remote_config_mqtt_sample -l
INF|2020-08-26 19:31:03|qcloud_iot_device.c|iot_device_info_set(50): SDK_Ver: 3.2.1, Product_ID: YI7XCD5DRH, Device_Name: airConditioner1
DBG|2020-08-26 19:31:03|HAL_TLS_mbedtls.c|HAL_TLS_Connect(200): Setting up the SSL/TLS structure...
DBG|2020-08-26 19:31:03|HAL_TLS_mbedtls.c|HAL_TLS_Connect(242): Performing the SSL/TLS handshake...
DBG|2020-08-26 19:31:03|HAL_TLS_mbedtls.c|HAL_TLS_Connect(243): Connecting to /YI7XCD5DRH.iotcloud.tencentdevices.com/8883...
INF|2020-08-26 19:31:03|HAL_TLS_mbedtls.c|HAL_TLS_Connect(265): connected with /YI7XCD5DRH.iotcloud.tencentdevices.com/8883...
INF|2020-08-26 19:31:03|mqtt_client.c|IOT_MQTT_Construct(118): mqtt connect with id: 9AEwi success
INF|2020-08-26 19:31:03|remote_config_mqtt_sample.c|main(300): Cloud Device Construct Success
DBG|2020-08-26 19:31:03|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(142): topicName=$config/update/YI7XCD5DRH/airConditioner1|packet_id=13502
INF|2020-08-26 19:31:03|remote_config_mqtt_sample.c|_mqtt_event_handler(128): subscribe success, packet-id=13502
DBG|2020-08-26 19:31:03|remote_config_mqtt.c|_config_mqtt_sub_event_handler(156): mqtt config topic subscribe success
INF|2020-08-26 19:31:03|remote_config_mqtt_sample.c|main(315): config topic subscribe success, packetid:13502
```

##### 3.1 设备订阅远程配置函数

```
int IOT_Subscribe_Config(void *client, ConfigSubscirbeUserData *config_sub_userdata, int subscribe_timeout)
```

#### 4. 云平台上给产品添加远程配置内容

点击 AirConditioner 产品的管理按钮，然后点击远程配置选项卡添加远程配置内容保存和启用远程配置
![image.png](https://main.qcloudimg.com/raw/16001d465cc7f5765d44842b6a47ef0d.png)

#### 5. 观察设备主动请求配置更新

观察设备 airConditioner1 的打印输出，可以看到设备发布获取配置消息并收到了云平台的下行配置回复。 `Recv Msg Topic:$config/update/YI7XCD5DRH/airConditioner1` `"type":"reply"`

配置信息是 `{"baud rate":9600,"data bits":8,"stop bit":1,"parity":"NONE","thread sleep":1000}`
```
DBG|2020-08-26 19:31:03|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$config/report/YI7XCD5DRH/airConditioner1|payload={"type":"get"}
DBG|2020-08-26 19:31:08|remote_config_mqtt.c|_config_mqtt_message_callback(90): Recv Msg Topic:$config/update/YI7XCD5DRH/airConditioner1, buff data:{
   "type":"reply",
   "result":0,
   "payload": {"baud rate":9600,"data bits":8,"stop bit":1,"parity":"NONE","thread sleep":1000}
}
INF|2020-08-26 19:31:08|remote_config_mqtt_sample.c|_on_config_proc_handler(94): config message arrived , proc result: 9600, 8, 0, 1, 1000
INF|2020-08-26 19:31:08|remote_config_mqtt_sample.c|_on_config_proc_handler(96): config message arrived , configData: {"baud rate":9600,"data bits":8,"stop bit":1,"parity":"NONE","thread sleep":1000}

```

##### 5.1 设备主动请求配置更新函数

```
int IOT_Get_Config(void *client, char *json_buffer, int buffer_size, int reply_timeout)
```

#### 6. 观察云平台主动下发配置

观察设备 airConditioner1 的打印输出，可以看到已经处理了远程配置下行消息。 `Recv Msg Topic:$config/update/YI7XCD5DRH/airConditioner1` `"type":"push"`

配置信息是 `{"baud rate":115200,"data bits":8,"stop bit":1,"parity":"NONE","thread sleep":1000}`

```
DBG|2020-08-26 19:31:30|remote_config_mqtt.c|_config_mqtt_message_callback(90): Recv Msg Topic:$config/update/YI7XCD5DRH/airConditioner1, payload:{
   "type":"push",
   "payload": {"baud rate":115200,"data bits":8,"stop bit":1,"parity":"NONE","thread sleep":1000}
}
INF|2020-08-26 19:31:30|remote_config_mqtt_sample.c|_on_config_proc_handler(44): config message arrived , configData: {"baud rate":115200,"data bits":8,"stop bit":1,"parity":"NONE","thread sleep":1000}
INF|2020-08-26 19:31:30|remote_config_mqtt_sample.c|_on_config_proc_handler(96): config message arrived , proc end: 115200, 8, 0, 1, 1000

```

#### 7. 云平台远程配置下行结果码 result 字段说明

| 结果码 | 说明                             |
|--------|----------------------------------|
| 0	     | 远程配置获取成功                 |
| 1001	 | 产品在云平台上的远程配置被禁用   |
| 1002	 | 产品在云平台上不存在远程配置内容 |
| 1003	 | 产品在云平台上的远程配置不合法   |

***至此，完成了远程配置的示例运行***

使用远程配置功能时，参考例子程序，修改 `_on_config_proc_handler` 函数适配实际设备的配置解析与更新。
