# Subdevice OTA Firmware Update Through Gateway

For more information on the OTA firmware update feature and process on IoT Hub, please see [Device Firmware Update](https://cloud.tencent.com/document/product/634/14674).

## 1. Subdevice Update Process Overview

As subdevices cannot connect directly to the cloud, you can use the device OTA update method to update their firmware through a gateway. You can update one or multiple subdevices at a time as shown below:
![](https://main.qcloudimg.com/raw/8b1617dc2e09727de53c03a199a74733.png)

## 2. Uploading Firmware

To update the firmware, you first need to upload it to the IoT Hub backend in the console as shown below:
![](https://main.qcloudimg.com/raw/71f04d532d5143890c4a464dfaea4e31.png)

## 3. Compiling and Running Demo (with **Key-Authenticated MQTT Device** as Example)
#### 1. Compile the SDK
Modify `CMakeLists.txt` to ensure that the following options exist:
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
Run the following script for compilation.
```
./cmake_build.sh 
```
The demo output `ota_mqtt_sample` is in the `output/release/bin` folder.

#### 2. Enter the device information
##### 1. Enter the gateway device information
Enter the gateway device information in the configuration file `device_info.json`.
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
##### 2. Enter the subdevice information
The subdevice information in the demo is configured as `ota_mqtt_subdev_sample.c` in the following code, while the subdevice information in a real-world environment is generally reported to the gateway by the subdevices themselves.
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

#### 3. Run the demo
Run the subdevice OTA MQTT demo `ota_mqtt_subdev_sample`:
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
As you can see from the log of the demo, the gateway connected and subscribed to the OTA topic on behalf of the subdevices, reported their current firmware version numbers to IoT Hub, and then waited for the platform to deliver the firmware update command.

#### 4. Run the firmware update command

After IoT Hub receives the firmware version numbers reported by the devices, select the target new firmware and deliver the update command. You can update the firmware in the console in the following three methods:

1. Update all devices by firmware version number. The selected version number is the version number for update, and you can select multiple version numbers for update.
![](https://main.qcloudimg.com/raw/cc47178dc3924ad2c96fb98365159cfd.png)
2. Batch update specified devices by firmware version number. You can select multiple version numbers and devices for update.
![](https://main.qcloudimg.com/raw/963239bf3facc8b9a54660b1498c6dc2.png)
3. Batch update the devices in the template file by device name. You can ignore the version number for update and directly update the devices in the template file to the selected firmware.
![](https://main.qcloudimg.com/raw/141c4b0c32aed3e70edcae58419478dc.png)

***For methods 1 and 2, if the currently running firmware of the devices to be updated is not uploaded to the console, you cannot select their version number for update. In this case, you can upload this firmware to the console or select method 3 for update.***


After running the firmware update command in the console, observe the log of the demo, and you can see that the gateway received the update commands for the `light1`, `light2`, and `light3` subdevices through the MQTT OTA topic, downloaded the subdevice firmware through HTTPS and saved it, started to update the subdevice firmware, and finally reported the result of the subdevice update success to IoT Hub.
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

#### 5. View the firmware update result on IoT Hub
 
You can see the firmware update result in the IoT Hub console as shown below:
![](https://main.qcloudimg.com/raw/61babc585547bf431600460112581a18.png)

## 4. Demo Code Use Instructions

1. To update the firmware, you can use the compilation option `FEATURE_OTA_USE_HTTPS` to select HTTP or HTTPS for firmware download.
2. The HTTP or HTTPS firmware download feature of the SDK supports checkpoint restart; that is, if the firmware download process is interrupted due to network or other issues, it will resume from where interrupted.
3. In `ota_mqtt_subdev_sample.c`, the `process_ota` function is the core process of OTA, which includes waiting for the update command, initiating a firmware download over HTTP or HTTPS and saving the firmware on the gateway, verifying the MD5 value of the firmware file, interacting with the subdevices for update, and reporting the subdevice update result. The functions for saving the firmware, saving the checkpoint restart status, and interacting with the subdevices for update are subject to the platform and environment. The storage of the firmware and checkpoint start status in the demo uses file operations, which can be reused directly on platforms that support standard file operations such as Linux and Windows but require adaptation for other platforms. The function for interacting with the subdevices for update needs to be adapted to the specific environment.
 


