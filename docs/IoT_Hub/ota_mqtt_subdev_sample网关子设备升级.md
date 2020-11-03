# 网关 OTA 子设备固件升级

关于物联平台 OTA 固件升级功能及流程介绍，请参考官网文档[设备固件升级](https://cloud.tencent.com/document/product/634/14674)

## 一. 子设备升级流程介绍

由于子设备无法直接和云端建立连接，为满足子设备固件升级的需求，网关上使用设备 OTA 升级方式对子设备进行固件升级，支持网关子设备单台升级和批量升级，流程如下图所示：
![](https://main.qcloudimg.com/raw/531945fe455b518f1f9788ae9e0006b0.png)

## 二. 上传固件

进行固件升级，首先要上传固件到物联网后台，可以通过控制台进行上传，如下图示：
![](https://main.qcloudimg.com/raw/2ccbc69f812c91884941060b17db86e8.png)

## 三. 编译运行示例程序(以**MQTT密钥认证设备**为例)
#### 1. 编译 SDK
修改CMakeLists.txt确保以下选项存在
```
set(BUILD_TYPE                   "release")
set(COMPILE_TOOLS                "gcc")
set(PLATFORM 	                "linux")
set(FEATURE_MQTT_COMM_ENABLED ON)
set(FEATURE_OTA_COMM_ENABLED ON)
set(FEATURE_OTA_SIGNAL_CHANNEL "MQTT")
set(FEATURE_GATEWAY_ENABLED ON)
set(FEATURE_AUTH_MODE "KEY")
set(FEATURE_AUTH_WITH_NOTLS OFF)
set(FEATURE_DEBUG_DEV_INFO_USED  OFF)
```
执行脚本编译
```
./cmake_build.sh 
```
示例输出ota_mqtt_sample位于`output/release/bin`文件夹中

#### 2. 填写设备信息
##### 1. 填写网关设备信息
将网关设备信息填写到配置文件device_info.json中
```
{
  "auth_mode":"KEY",

  "productId":"JR1WOKB7Q7",
  "productSecret":"YOUR_PRODUCT_SECRET",
  "deviceName":"gw_deng1",

  "key_deviceinfo":{    
     "deviceSecret":"1RRdhkwmZ45wYXjuPfygeA=="
  }
}
```
##### 2. 填写子设备信息
例子程序中的子设备信息被配置 `ota_mqtt_subdev_sample.c` 在如下代码中，实际环境中子设备的信息一般由子设备上报给网关。
```
static DeviceInfo sg_subdev_info[] = {
    {.product_id = "E69UADXUYY", .device_name = "light1"},
    {.product_id = "E69UADXUYY", .device_name = "light2"},
    {.product_id = "E69UADXUYY", .device_name = "light3"},
    {.product_id = "YI7XCD5DRH", .device_name = "airConditioner1"},
};
gw->sub_dev_info = sg_subdev_info;
gw->sub_dev_num = sizeof(sg_subdev_info) / sizeof(DeviceInfo);
```

#### 3. 运行示例
执行子设备 OTA MQTT 例程 ota_mqtt_subdev_sample：
```
./output/release/bin/ota_mqtt_subdev_sample 
INF|2020-10-18 19:23:24|qcloud_iot_device.c|iot_device_info_set(50): SDK_Ver: 3.2.2, Product_ID: JR1WOKB7Q7, Device_Name: gw_deng1
DBG|2020-10-18 19:23:24|HAL_TLS_mbedtls.c|HAL_TLS_Connect(200): Setting up the SSL/TLS structure...
DBG|2020-10-18 19:23:24|HAL_TLS_mbedtls.c|HAL_TLS_Connect(242): Performing the SSL/TLS handshake...
DBG|2020-10-18 19:23:24|HAL_TLS_mbedtls.c|HAL_TLS_Connect(243): Connecting to /JR1WOKB7Q7.iotcloud.tencentdevices.com/8883...
INF|2020-10-18 19:23:25|HAL_TLS_mbedtls.c|HAL_TLS_Connect(265): connected with /JR1WOKB7Q7.iotcloud.tencentdevices.com/8883...
INF|2020-10-18 19:23:25|mqtt_client.c|IOT_MQTT_Construct(118): mqtt connect with id: 3awT3 success
DBG|2020-10-18 19:23:25|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(142): topicName=$gateway/operation/result/JR1WOKB7Q7/gw_deng1|packet_id=61021
DBG|2020-10-18 19:23:25|gateway_api.c|_gateway_event_handler(34): gateway sub|unsub(3) success, packet-id=61021
INF|2020-10-18 19:23:25|mqtt_client.c|_mqtt_yield_thread(258): MQTT client JR1WOKB7Q7gw_deng1 start loop
DBG|2020-10-18 19:23:26|gateway_api.c|IOT_Gateway_Subdev_Online(131): there is no session, create a new session
DBG|2020-10-18 19:23:26|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$gateway/operation/JR1WOKB7Q7/gw_deng1|payload={"type":"online","payload":{"devices":[{"product_id":"E69UADXUYY","device_name":"light1"}]}}
DBG|2020-10-18 19:23:27|gateway_common.c|_gateway_message_handler(163): msg payload: {"type":"online","payload":{"devices":[{"product_id":"E69UADXUYY","device_name":"light1","result":0}]}}
INF|2020-10-18 19:23:27|gateway_common.c|_gateway_message_handler(222): client_id(E69UADXUYY/light1), online result 0
DBG|2020-10-18 19:23:27|ota_mqtt_subdev_sample.c|main(796): subDev Pid:E69UADXUYY devName:light1 online success.
DBG|2020-10-18 19:23:27|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(142): topicName=$ota/update/E69UADXUYY/light1|packet_id=61022
DBG|2020-10-18 19:23:27|gateway_api.c|_gateway_event_handler(34): gateway sub|unsub(3) success, packet-id=61022
INF|2020-10-18 19:23:27|ota_mqtt_subdev_sample.c|_event_handler(555): subscribe success, packet-id=61022
DBG|2020-10-18 19:23:27|ota_mqtt.c|_otamqtt_event_callback(119): OTA topic subscribe success
DBG|2020-10-18 19:23:27|ota_mqtt_subdev_sample.c|main(805): subDev Pid:E69UADXUYY devName:light1 initialize OTA Handle success.
INF|2020-10-18 19:23:27|ota_mqtt_subdev_sample.c|_get_subdev_fw_running_version(406): E69UADXUYY-light1 FW running version: 0.1
INF|2020-10-18 19:23:27|ota_client.c|IOT_OTA_ResetStatus(131): reset OTA state!
DBG|2020-10-18 19:23:27|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=61023|topicName=$ota/report/E69UADXUYY/light1|payload={"type": "report_version", "report":{"version":"0.1"}}
DBG|2020-10-18 19:23:27|gateway_api.c|IOT_Gateway_Subdev_Online(131): there is no session, create a new session
DBG|2020-10-18 19:23:27|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$gateway/operation/JR1WOKB7Q7/gw_deng1|payload={"type":"online","payload":{"devices":[{"product_id":"E69UADXUYY","device_name":"light2"}]}}
INF|2020-10-18 19:23:27|ota_mqtt_subdev_sample.c|_event_handler(582): publish success, packet-id=61023
DBG|2020-10-18 19:23:27|ota_mqtt.c|_otamqtt_upgrage_cb(105): topic=$ota/update/E69UADXUYY/light1
INF|2020-10-18 19:23:27|ota_mqtt.c|_otamqtt_upgrage_cb(106): len=84, topic_msg={"result_code":0,"result_msg":"success","type":"report_version_rsp","version":"0.1"}
INF|2020-10-18 19:23:27|ota_client.c|_ota_callback(98): Report version success!
DBG|2020-10-18 19:23:28|gateway_common.c|_gateway_message_handler(163): msg payload: {"type":"online","payload":{"devices":[{"product_id":"E69UADXUYY","device_name":"light2","result":0}]}}
INF|2020-10-18 19:23:28|gateway_common.c|_gateway_message_handler(222): client_id(E69UADXUYY/light2), online result 0
DBG|2020-10-18 19:23:28|ota_mqtt_subdev_sample.c|main(796): subDev Pid:E69UADXUYY devName:light2 online success.
DBG|2020-10-18 19:23:28|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(142): topicName=$ota/update/E69UADXUYY/light2|packet_id=61024
DBG|2020-10-18 19:23:28|gateway_api.c|_gateway_event_handler(34): gateway sub|unsub(3) success, packet-id=61024
INF|2020-10-18 19:23:28|ota_mqtt_subdev_sample.c|_event_handler(555): subscribe success, packet-id=61024
DBG|2020-10-18 19:23:28|ota_mqtt.c|_otamqtt_event_callback(119): OTA topic subscribe success
DBG|2020-10-18 19:23:28|ota_mqtt_subdev_sample.c|main(805): subDev Pid:E69UADXUYY devName:light2 initialize OTA Handle success.
INF|2020-10-18 19:23:28|ota_mqtt_subdev_sample.c|_get_subdev_fw_running_version(406): E69UADXUYY-light2 FW running version: 0.1
INF|2020-10-18 19:23:28|ota_client.c|IOT_OTA_ResetStatus(131): reset OTA state!
DBG|2020-10-18 19:23:28|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=61025|topicName=$ota/report/E69UADXUYY/light2|payload={"type": "report_version", "report":{"version":"0.1"}}
DBG|2020-10-18 19:23:28|gateway_api.c|IOT_Gateway_Subdev_Online(131): there is no session, create a new session
DBG|2020-10-18 19:23:28|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$gateway/operation/JR1WOKB7Q7/gw_deng1|payload={"type":"online","payload":{"devices":[{"product_id":"E69UADXUYY","device_name":"light3"}]}}
INF|2020-10-18 19:23:28|ota_mqtt_subdev_sample.c|_event_handler(582): publish success, packet-id=61025
DBG|2020-10-18 19:23:28|ota_mqtt.c|_otamqtt_upgrage_cb(105): topic=$ota/update/E69UADXUYY/light2
INF|2020-10-18 19:23:28|ota_mqtt.c|_otamqtt_upgrage_cb(106): len=84, topic_msg={"result_code":0,"result_msg":"success","type":"report_version_rsp","version":"0.1"}
INF|2020-10-18 19:23:28|ota_client.c|_ota_callback(98): Report version success!
DBG|2020-10-18 19:23:29|gateway_common.c|_gateway_message_handler(163): msg payload: {"type":"online","payload":{"devices":[{"product_id":"E69UADXUYY","device_name":"light3","result":0}]}}
INF|2020-10-18 19:23:29|gateway_common.c|_gateway_message_handler(222): client_id(E69UADXUYY/light3), online result 0
DBG|2020-10-18 19:23:29|ota_mqtt_subdev_sample.c|main(796): subDev Pid:E69UADXUYY devName:light3 online success.
DBG|2020-10-18 19:23:29|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(142): topicName=$ota/update/E69UADXUYY/light3|packet_id=61026
DBG|2020-10-18 19:23:29|gateway_api.c|_gateway_event_handler(34): gateway sub|unsub(3) success, packet-id=61026
INF|2020-10-18 19:23:29|ota_mqtt_subdev_sample.c|_event_handler(555): subscribe success, packet-id=61026
DBG|2020-10-18 19:23:29|ota_mqtt.c|_otamqtt_event_callback(119): OTA topic subscribe success
DBG|2020-10-18 19:23:29|ota_mqtt_subdev_sample.c|main(805): subDev Pid:E69UADXUYY devName:light3 initialize OTA Handle success.
INF|2020-10-18 19:23:29|ota_mqtt_subdev_sample.c|_get_subdev_fw_running_version(406): E69UADXUYY-light3 FW running version: 0.1
INF|2020-10-18 19:23:29|ota_client.c|IOT_OTA_ResetStatus(131): reset OTA state!
DBG|2020-10-18 19:23:29|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=61027|topicName=$ota/report/E69UADXUYY/light3|payload={"type": "report_version", "report":{"version":"0.1"}}
DBG|2020-10-18 19:23:29|gateway_api.c|IOT_Gateway_Subdev_Online(131): there is no session, create a new session
DBG|2020-10-18 19:23:29|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$gateway/operation/JR1WOKB7Q7/gw_deng1|payload={"type":"online","payload":{"devices":[{"product_id":"YI7XCD5DRH","device_name":"airConditioner1"}]}}
INF|2020-10-18 19:23:29|ota_mqtt_subdev_sample.c|_event_handler(582): publish success, packet-id=61027
DBG|2020-10-18 19:23:29|ota_mqtt.c|_otamqtt_upgrage_cb(105): topic=$ota/update/E69UADXUYY/light3
INF|2020-10-18 19:23:29|ota_mqtt.c|_otamqtt_upgrage_cb(106): len=84, topic_msg={"result_code":0,"result_msg":"success","type":"report_version_rsp","version":"0.1"}
INF|2020-10-18 19:23:29|ota_client.c|_ota_callback(98): Report version success!
DBG|2020-10-18 19:23:29|gateway_common.c|_gateway_message_handler(163): msg payload: {"type":"online","payload":{"devices":[{"product_id":"YI7XCD5DRH","device_name":"airConditioner1","result":0}]}}
INF|2020-10-18 19:23:29|gateway_common.c|_gateway_message_handler(222): client_id(YI7XCD5DRH/airConditioner1), online result 0
DBG|2020-10-18 19:23:29|ota_mqtt_subdev_sample.c|main(796): subDev Pid:YI7XCD5DRH devName:airConditioner1 online success.
DBG|2020-10-18 19:23:29|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(142): topicName=$ota/update/YI7XCD5DRH/airConditioner1|packet_id=61028
DBG|2020-10-18 19:23:29|gateway_api.c|_gateway_event_handler(34): gateway sub|unsub(3) success, packet-id=61028
INF|2020-10-18 19:23:29|ota_mqtt_subdev_sample.c|_event_handler(555): subscribe success, packet-id=61028
DBG|2020-10-18 19:23:29|ota_mqtt.c|_otamqtt_event_callback(119): OTA topic subscribe success
DBG|2020-10-18 19:23:30|ota_mqtt_subdev_sample.c|main(805): subDev Pid:YI7XCD5DRH devName:airConditioner1 initialize OTA Handle success.
INF|2020-10-18 19:23:30|ota_mqtt_subdev_sample.c|_get_subdev_fw_running_version(406): YI7XCD5DRH-airConditioner1 FW running version: 0.1
INF|2020-10-18 19:23:30|ota_client.c|IOT_OTA_ResetStatus(131): reset OTA state!
DBG|2020-10-18 19:23:30|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=61029|topicName=$ota/report/YI7XCD5DRH/airConditioner1|payload={"type": "report_version", "report":{"version":"0.1"}}
INF|2020-10-18 19:23:30|ota_mqtt_subdev_sample.c|_event_handler(582): publish success, packet-id=61029
INF|2020-10-18 19:23:30|ota_mqtt_subdev_sample.c|main(844): wait for ota upgrade command...
DBG|2020-10-18 19:23:30|ota_mqtt.c|_otamqtt_upgrage_cb(105): topic=$ota/update/YI7XCD5DRH/airConditioner1
INF|2020-10-18 19:23:30|ota_mqtt.c|_otamqtt_upgrage_cb(106): len=84, topic_msg={"result_code":0,"result_msg":"success","type":"report_version_rsp","version":"0.1"}
INF|2020-10-18 19:23:30|ota_client.c|_ota_callback(98): Report version success!
INF|2020-10-18 19:23:32|ota_mqtt_subdev_sample.c|main(844): wait for ota upgrade command...
INF|2020-10-18 19:23:34|ota_mqtt_subdev_sample.c|main(844): wait for ota upgrade command...
INF|2020-10-18 19:23:36|ota_mqtt_subdev_sample.c|main(844): wait for ota upgrade command...
```
从例子程序的日志可以观察到，网关代理子设备上线，订阅 OTA Topic，并将子设备当前的固件版本号上报给云平台，然后网关进入等待云平台下发固件升级命令的状态。

#### 4. 执行固件升级命令

云平台在接收到设备上报的固件版本号之后，选择要升级的新固件版本并下发升级命令；可以通过控制台操作固件升级，控制台有三种升级方式供选择，分别是：

1. 按固件版本号升级所有设备。选择的版本号是待升级的版本号，可选多个待升级版本号。
![](https://main.qcloudimg.com/raw/dcc27c9875ee4f94b9016e6ee094895e.png)
2. 按固件版本号批量升级指定设备。选择的版本号是待升级的版本号，可选多个待升级版本号；选择的设备是本次待升级的设备，可选多个待升级设备。
![](https://main.qcloudimg.com/raw/aea220c450a1d32fadf3f3b693840f4c.png)
3. 按设备名称批量升级模板文件中的设备。不关心待升级的版本号，直接将模板文件中的设备升级为选择的固件。
![](https://main.qcloudimg.com/raw/86c1c899ce0723162cd851b18ffdc973.png)

***选择 1、2 方式如果待升级设备当前运行的固件没有上传到控制台，那么在选择待升级版本号时就不能选择待升级设备的版本号，此时可以通过在控制台上传待升级设备当前运行的固件版本或者选择方式 3 升级。***


控制台执行固件升级命令后，再观察例子程序的日志，可以看到网关通过 MQTT OTA Topic 接收到子设备 light1 light2 light3 的升级命令，通过 HTTPS 下载子设备固件并保存，下载成功后开始升级子设备固件，最后上报子设备升级成功的结果给云平台。
```
INF|2020-10-18 19:23:45|ota_mqtt_subdev_sample.c|main(844): wait for ota upgrade command...
INF|2020-10-18 19:23:47|ota_mqtt_subdev_sample.c|main(844): wait for ota upgrade command...
DBG|2020-10-18 19:23:47|ota_mqtt.c|_otamqtt_upgrage_cb(105): topic=$ota/update/E69UADXUYY/light1
INF|2020-10-18 19:23:47|ota_mqtt.c|_otamqtt_upgrage_cb(106): len=457, topic_msg={"file_size":29972,"md5sum":"20a4d2b1f8492a22fa3e472a758bf9ec","type":"update_firmware","url":"https://ota-1255858890.cos.ap-guangzhou.myqcloud.com/100003908905_E69UADXUYY_V100R0001?","version":"V100R0001"}
DBG|2020-10-18 19:23:48|ota_mqtt.c|_otamqtt_upgrage_cb(105): topic=$ota/update/E69UADXUYY/light2
INF|2020-10-18 19:23:48|ota_mqtt.c|_otamqtt_upgrage_cb(106): len=457, topic_msg={"file_size":29972,"md5sum":"20a4d2b1f8492a22fa3e472a758bf9ec","type":"update_firmware","url":"https://ota-1255858890.cos.ap-guangzhou.myqcloud.com/100003908905_E69UADXUYY_V100R0001?","version":"V100R0001"}
DBG|2020-10-18 19:23:48|ota_mqtt.c|_otamqtt_upgrage_cb(105): topic=$ota/update/E69UADXUYY/light3
INF|2020-10-18 19:23:48|ota_mqtt.c|_otamqtt_upgrage_cb(106): len=457, topic_msg={"file_size":29972,"md5sum":"20a4d2b1f8492a22fa3e472a758bf9ec","type":"update_firmware","url":"https://ota-1255858890.cos.ap-guangzhou.myqcloud.com/100003908905_E69UADXUYY_V100R0001?","version":"V100R0001"}
ERR|2020-10-18 19:23:50|ota_mqtt_subdev_sample.c|_get_local_fw_info(176): open file ./FW_E69UADXUYYlight3.json failed
DBG|2020-10-18 19:23:50|ota_client.c|IOT_OTA_StartDownload(360): to download FW from offset: 0, size: 29972
DBG|2020-10-18 19:23:50|ota_fetch.c|ofc_Init(79): head_content:Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Accept-Encoding: gzip, deflate
Range: bytes=0-29972

DBG|2020-10-18 19:23:50|HAL_TLS_mbedtls.c|_mbedtls_client_init(132): psk/pskid is empty!|psk=(null)|psd_id=(null)
DBG|2020-10-18 19:23:50|HAL_TLS_mbedtls.c|HAL_TLS_Connect(200): Setting up the SSL/TLS structure...
DBG|2020-10-18 19:23:50|HAL_TLS_mbedtls.c|HAL_TLS_Connect(242): Performing the SSL/TLS handshake...
DBG|2020-10-18 19:23:50|HAL_TLS_mbedtls.c|HAL_TLS_Connect(243): Connecting to /ota-1255858890.cos.ap-guangzhou.myqcloud.com/443...
INF|2020-10-18 19:23:50|HAL_TLS_mbedtls.c|HAL_TLS_Connect(265): connected with /ota-1255858890.cos.ap-guangzhou.myqcloud.com/443...
DBG|2020-10-18 19:23:50|utils_httpc.c|qcloud_http_client_connect(742): http client connect success
DBG|2020-10-18 19:23:50|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$ota/report/E69UADXUYY/light3|payload={"type": "report_progress", "report": {"progress": {"state":"downloading", "percent":"0", "result_code":"0", "result_msg":""}, "version": "V100R0001"}}
DBG|2020-10-18 19:23:51|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$ota/report/E69UADXUYY/light3|payload={"type": "report_progress", "report": {"progress": {"state":"downloading", "percent":"100", "result_code":"0", "result_msg":""}, "version": "V100R0001"}}
INF|2020-10-18 19:23:51|ota_client.c|IOT_OTA_Ioctl(656): FW MD5 check: origin=20a4d2b1f8492a22fa3e472a758bf9ec, now=20a4d2b1f8492a22fa3e472a758bf9ec
INF|2020-10-18 19:23:51|ota_mqtt_subdev_sample.c|process_ota(494): The firmware is valid
DBG|2020-10-18 19:23:51|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=61030|topicName=$ota/report/E69UADXUYY/light3|payload={"type": "report_progress", "report":{"progress":{"state":"burning", "result_code":"0", "result_msg":""}, "version":"V100R0001"}}
INF|2020-10-18 19:23:51|ota_mqtt_subdev_sample.c|_subdev_process_upgrade(414): E69UADXUYY-light3 download success, fw name:./FW_E69UADXUYYlight3_V100R0001.bin, fw size:29972, fw version:V100R0001, now to upgrade subdevice ...
INF|2020-10-18 19:23:51|ota_mqtt_subdev_sample.c|process_ota(517): E69UADXUYY-light3 upgrade success
DBG|2020-10-18 19:23:51|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=61031|topicName=$ota/report/E69UADXUYY/light3|payload={"type": "report_progress", "report":{"progress":{"state":"done", "result_code":"0", "result_msg":""}, "version":"V100R0001"}}
INF|2020-10-18 19:23:51|ota_client.c|IOT_OTA_ResetStatus(131): reset OTA state!
INF|2020-10-18 19:23:51|ota_mqtt_subdev_sample.c|_event_handler(582): publish success, packet-id=61030
INF|2020-10-18 19:23:51|ota_mqtt_subdev_sample.c|_event_handler(582): publish success, packet-id=61031
ERR|2020-10-18 19:23:52|ota_mqtt_subdev_sample.c|main(841): process ota success subdevice E69UADXUYY-light3
ERR|2020-10-18 19:23:52|ota_mqtt_subdev_sample.c|_get_local_fw_info(176): open file ./FW_E69UADXUYYlight2.json failed
DBG|2020-10-18 19:23:52|ota_client.c|IOT_OTA_StartDownload(360): to download FW from offset: 0, size: 29972
DBG|2020-10-18 19:23:52|ota_fetch.c|ofc_Init(79): head_content:Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Accept-Encoding: gzip, deflate
Range: bytes=0-29972

DBG|2020-10-18 19:23:52|HAL_TLS_mbedtls.c|_mbedtls_client_init(132): psk/pskid is empty!|psk=(null)|psd_id=(null)
DBG|2020-10-18 19:23:52|HAL_TLS_mbedtls.c|HAL_TLS_Connect(200): Setting up the SSL/TLS structure...
DBG|2020-10-18 19:23:52|HAL_TLS_mbedtls.c|HAL_TLS_Connect(242): Performing the SSL/TLS handshake...
DBG|2020-10-18 19:23:52|HAL_TLS_mbedtls.c|HAL_TLS_Connect(243): Connecting to /ota-1255858890.cos.ap-guangzhou.myqcloud.com/443...
INF|2020-10-18 19:23:52|HAL_TLS_mbedtls.c|HAL_TLS_Connect(265): connected with /ota-1255858890.cos.ap-guangzhou.myqcloud.com/443...
DBG|2020-10-18 19:23:52|utils_httpc.c|qcloud_http_client_connect(742): http client connect success
DBG|2020-10-18 19:23:52|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$ota/report/E69UADXUYY/light2|payload={"type": "report_progress", "report": {"progress": {"state":"downloading", "percent":"0", "result_code":"0", "result_msg":""}, "version": "V100R0001"}}
DBG|2020-10-18 19:23:53|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$ota/report/E69UADXUYY/light2|payload={"type": "report_progress", "report": {"progress": {"state":"downloading", "percent":"100", "result_code":"0", "result_msg":""}, "version": "V100R0001"}}
INF|2020-10-18 19:23:53|ota_client.c|IOT_OTA_Ioctl(656): FW MD5 check: origin=20a4d2b1f8492a22fa3e472a758bf9ec, now=20a4d2b1f8492a22fa3e472a758bf9ec
INF|2020-10-18 19:23:53|ota_mqtt_subdev_sample.c|process_ota(494): The firmware is valid
DBG|2020-10-18 19:23:53|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=61032|topicName=$ota/report/E69UADXUYY/light2|payload={"type": "report_progress", "report":{"progress":{"state":"burning", "result_code":"0", "result_msg":""}, "version":"V100R0001"}}
INF|2020-10-18 19:23:53|ota_mqtt_subdev_sample.c|_subdev_process_upgrade(414): E69UADXUYY-light2 download success, fw name:./FW_E69UADXUYYlight2_V100R0001.bin, fw size:29972, fw version:V100R0001, now to upgrade subdevice ...
INF|2020-10-18 19:23:53|ota_mqtt_subdev_sample.c|process_ota(517): E69UADXUYY-light2 upgrade success
DBG|2020-10-18 19:23:53|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=61033|topicName=$ota/report/E69UADXUYY/light2|payload={"type": "report_progress", "report":{"progress":{"state":"done", "result_code":"0", "result_msg":""}, "version":"V100R0001"}}
INF|2020-10-18 19:23:53|ota_client.c|IOT_OTA_ResetStatus(131): reset OTA state!
INF|2020-10-18 19:23:53|ota_mqtt_subdev_sample.c|_event_handler(582): publish success, packet-id=61032
INF|2020-10-18 19:23:53|ota_mqtt_subdev_sample.c|_event_handler(582): publish success, packet-id=61033
ERR|2020-10-18 19:23:54|ota_mqtt_subdev_sample.c|main(841): process ota success subdevice E69UADXUYY-light2
ERR|2020-10-18 19:23:54|ota_mqtt_subdev_sample.c|_get_local_fw_info(176): open file ./FW_E69UADXUYYlight1.json failed
DBG|2020-10-18 19:23:54|ota_client.c|IOT_OTA_StartDownload(360): to download FW from offset: 0, size: 29972
DBG|2020-10-18 19:23:54|ota_fetch.c|ofc_Init(79): head_content:Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Accept-Encoding: gzip, deflate
Range: bytes=0-29972

DBG|2020-10-18 19:23:54|HAL_TLS_mbedtls.c|_mbedtls_client_init(132): psk/pskid is empty!|psk=(null)|psd_id=(null)
DBG|2020-10-18 19:23:54|HAL_TLS_mbedtls.c|HAL_TLS_Connect(200): Setting up the SSL/TLS structure...
DBG|2020-10-18 19:23:54|HAL_TLS_mbedtls.c|HAL_TLS_Connect(242): Performing the SSL/TLS handshake...
DBG|2020-10-18 19:23:54|HAL_TLS_mbedtls.c|HAL_TLS_Connect(243): Connecting to /ota-1255858890.cos.ap-guangzhou.myqcloud.com/443...
INF|2020-10-18 19:23:54|HAL_TLS_mbedtls.c|HAL_TLS_Connect(265): connected with /ota-1255858890.cos.ap-guangzhou.myqcloud.com/443...
DBG|2020-10-18 19:23:54|utils_httpc.c|qcloud_http_client_connect(742): http client connect success
DBG|2020-10-18 19:23:54|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$ota/report/E69UADXUYY/light1|payload={"type": "report_progress", "report": {"progress": {"state":"downloading", "percent":"0", "result_code":"0", "result_msg":""}, "version": "V100R0001"}}
DBG|2020-10-18 19:23:55|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$ota/report/E69UADXUYY/light1|payload={"type": "report_progress", "report": {"progress": {"state":"downloading", "percent":"100", "result_code":"0", "result_msg":""}, "version": "V100R0001"}}
INF|2020-10-18 19:23:55|ota_client.c|IOT_OTA_Ioctl(656): FW MD5 check: origin=20a4d2b1f8492a22fa3e472a758bf9ec, now=20a4d2b1f8492a22fa3e472a758bf9ec
INF|2020-10-18 19:23:55|ota_mqtt_subdev_sample.c|process_ota(494): The firmware is valid
DBG|2020-10-18 19:23:55|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=61034|topicName=$ota/report/E69UADXUYY/light1|payload={"type": "report_progress", "report":{"progress":{"state":"burning", "result_code":"0", "result_msg":""}, "version":"V100R0001"}}
INF|2020-10-18 19:23:55|ota_mqtt_subdev_sample.c|_subdev_process_upgrade(414): E69UADXUYY-light1 download success, fw name:./FW_E69UADXUYYlight1_V100R0001.bin, fw size:29972, fw version:V100R0001, now to upgrade subdevice ...
INF|2020-10-18 19:23:55|ota_mqtt_subdev_sample.c|process_ota(517): E69UADXUYY-light1 upgrade success
DBG|2020-10-18 19:23:55|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=61035|topicName=$ota/report/E69UADXUYY/light1|payload={"type": "report_progress", "report":{"progress":{"state":"done", "result_code":"0", "result_msg":""}, "version":"V100R0001"}}
INF|2020-10-18 19:23:55|ota_client.c|IOT_OTA_ResetStatus(131): reset OTA state!
INF|2020-10-18 19:23:55|ota_mqtt_subdev_sample.c|_event_handler(582): publish success, packet-id=61034
INF|2020-10-18 19:23:55|ota_mqtt_subdev_sample.c|_event_handler(582): publish success, packet-id=61035
ERR|2020-10-18 19:23:56|ota_mqtt_subdev_sample.c|main(841): process ota success subdevice E69UADXUYY-light1
INF|2020-10-18 19:23:56|ota_mqtt_subdev_sample.c|main(844): wait for ota upgrade command...
INF|2020-10-18 19:23:58|ota_mqtt_subdev_sample.c|main(844): wait for ota upgrade command...
INF|2020-10-18 19:24:01|ota_mqtt_subdev_sample.c|main(844): wait for ota upgrade command...
INF|2020-10-18 19:24:03|ota_mqtt_subdev_sample.c|main(844): wait for ota upgrade command...
```

#### 5. 云平台查看固件升级结果
 
在云平台的控制台可以看到固件升级结果，如下图示：
![](https://main.qcloudimg.com/raw/fbdc347ed604583424c46a3e03054a7d.png)

## 四. 例子程序代码使用说明

1. 固件升级，支持通过 HTTP 或者 HTTPS 来下载固件，可以通过编译选项 FEATURE_OTA_USE_HTTPS 来选择
2. SDK 的 HTTP 或者 HTTPS 下载固件，支持断点续传功能，即当固件下载过程因为网络或者其他原因中断了，则再次下载的时候，可以从上一次中断的位置继续下载。
3. 在 ota_mqtt_subdev_sample.c 中， process_ota 函数为 OTA 的核心流程包括等待升级命令，发起 HTTP 或者 HTTPS 下载固件并保存到网关上，校验固件文件 MD5，与子设备交互升级，上报子设备升级结果。网关上的固件保存，断点续传状态保存，与子设备交互升级的函数，跟具体平台与环境密切相关。示例中固件和断点续传状态的存储使用了文件操作，在支持标准文件操作的系统比如 Linux/Windows 上面可以直接使用，对于其他平台则需要进行适配；与子设备交互升级的函数需要根据具体环境进行适配。
 


