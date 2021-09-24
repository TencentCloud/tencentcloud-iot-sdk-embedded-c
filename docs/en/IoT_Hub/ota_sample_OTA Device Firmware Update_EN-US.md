# OTA Device Firmware Update

For more information on the OTA firmware update feature and process on IoT Hub, please see [Device Firmware Update](https://cloud.tencent.com/document/product/634/14674).

## 1. Uploading Firmware

To update the firmware, you first need to upload it to the IoT Hub backend in the console as shown below:
![](https://main.qcloudimg.com/raw/71f04d532d5143890c4a464dfaea4e31.png)

## 2. Compiling and Running Demo (with **Key-Authenticated MQTT Device** as Example)
#### 1. Compile the SDK
Modify `CMakeLists.txt` to ensure that the following options exist:
```
set(BUILD_TYPE                   "release")
set(COMPILE_TOOLS                "gcc") 
set(PLATFORM 	                "linux")
set(FEATURE_MQTT_COMM_ENABLED ON)
set(FEATURE_OTA_COMM_ENABLED ON)
set(FEATURE_OTA_SIGNAL_CHANNEL "MQTT")
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
Enter the device information in the configuration file `device_info.json`.
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

#### 3. Run the demo
Run the OTA MQTT demo `ota_mqtt_sample`:
```
./output/release/bin/ota_mqtt_sample 
INF|2020-03-04 16:50:35|qcloud_iot_device.c|iot_device_info_set(50): SDK_Ver: 3.1.2, Product_ID: S3EUVBRJLB, Device_Name: test_device
DBG|2020-03-04 16:50:35|HAL_TLS_mbedtls.c|HAL_TLS_Connect(206): Setting up the SSL/TLS structure...
DBG|2020-03-04 16:50:35|HAL_TLS_mbedtls.c|HAL_TLS_Connect(248): Performing the SSL/TLS handshake...
DBG|2020-03-04 16:50:35|HAL_TLS_mbedtls.c|HAL_TLS_Connect(249): Connecting to /S3EUVBRJLB.iotcloud.tencentdevices.com/8883...
INF|2020-03-04 16:50:35|HAL_TLS_mbedtls.c|HAL_TLS_Connect(271): connected with /S3EUVBRJLB.iotcloud.tencentdevices.com/8883...
INF|2020-03-04 16:50:35|mqtt_client.c|IOT_MQTT_Construct(111): mqtt connect with id: BzaMF success
INF|2020-03-04 16:50:35|ota_mqtt_sample.c|main(516): Cloud Device Construct Success
DBG|2020-03-04 16:50:35|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(141): topicName=$ota/update/S3EUVBRJLB/test_device|packet_id=63333
INF|2020-03-04 16:50:35|ota_mqtt_sample.c|_event_handler(77): subscribe success, packet-id=63333
DBG|2020-03-04 16:50:35|ota_mqtt.c|_otamqtt_event_callback(117): OTA topic subscribe success
INF|2020-03-04 16:50:36|ota_mqtt_sample.c|_get_local_fw_running_version(363): FW running version: 1.0.0
DBG|2020-03-04 16:50:36|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=63334|topicName=$ota/report/S3EUVBRJLB/test_device|payload={"type": "report_version", "report":{"version":"1.0.0"}}
INF|2020-03-04 16:50:36|ota_mqtt_sample.c|_event_handler(89): publish success, packet-id=63334
DBG|2020-03-04 16:50:36|ota_mqtt.c|_otamqtt_upgrage_cb(103): topic=$ota/update/S3EUVBRJLB/test_device
INF|2020-03-04 16:50:36|ota_mqtt.c|_otamqtt_upgrage_cb(104): len=86, topic_msg={"result_code":0,"result_msg":"success","type":"report_version_rsp","version":"1.0.0"}
INF|2020-03-04 16:50:36|ota_client.c|_ota_callback(102): Report version success!
INF|2020-03-04 16:50:36|ota_mqtt_sample.c|process_ota(389): wait for ota upgrade command...
INF|2020-03-04 16:50:37|ota_mqtt_sample.c|process_ota(389): wait for ota upgrade command...
```
You can see that the device has reported the current firmware version and is waiting for the delivery of the firmware update command.

#### 4. Run the firmware update command

After receiving the firmware version reported by the device, select the target new firmware and run the update command. You can update the firmware in the console in the following three methods:

1. Update all devices by firmware version number. The selected version number is the version number for update, and you can select multiple version numbers for update.
![](https://main.qcloudimg.com/raw/cc47178dc3924ad2c96fb98365159cfd.png)
2. Batch update specified devices by firmware version number. You can select multiple version numbers and devices for update.
![](https://main.qcloudimg.com/raw/963239bf3facc8b9a54660b1498c6dc2.png)
3. Batch update the devices in the template file by device name. You can ignore the version number for update and directly update the devices in the template file to the selected firmware.
![](https://main.qcloudimg.com/raw/141c4b0c32aed3e70edcae58419478dc.png)

***For methods 1 and 2, if the currently running firmware of the devices to be updated is not uploaded to the console, you cannot select their version number for update. In this case, you can upload this firmware to the console or select method 3 for update.***



At this point, observe the printout of the demo, and you can see that the device received the update command over MQTT, downloaded the firmware over HTTPS, and saved it locally.
```
INF|2020-03-04 16:50:38|ota_mqtt_sample.c|process_ota(389): wait for ota upgrade command...
DBG|2020-03-04 16:50:39|ota_mqtt.c|_otamqtt_upgrage_cb(103): topic=$ota/update/S3EUVBRJLB/test_device
INF|2020-03-04 16:50:39|ota_mqtt.c|_otamqtt_upgrage_cb(104): len=454, topic_msg={"file_size":175436,"md5sum":"ad4615b866c13afb8b293a679bfa5dc4","type":"update_firmware","url":"https://ota-1255858890.cos.ap-guangzhou.myqcloud.com/","version":"1.3.0"}

INF|2020-03-04 16:50:39|ota_mqtt_sample.c|process_ota(389): wait for ota upgrade command...
ERR|2020-03-04 16:50:39|ota_mqtt_sample.c|_get_local_fw_info(251): open file ./FW_S3EUVBRJLBtest_device.json failed
DBG|2020-03-04 16:50:39|ota_client.c|IOT_OTA_StartDownload(347): to download FW from offset: 0, size: 175436
DBG|2020-03-04 16:50:39|ota_fetch.c|ofc_Init(83): head_content:Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Accept-Encoding: gzip, deflate
Range: bytes=0-175436

DBG|2020-03-04 16:50:39|HAL_TLS_mbedtls.c|_mbedtls_client_init(134): psk/pskid is empty!|psk=(null)|psd_id=(null)
DBG|2020-03-04 16:50:39|HAL_TLS_mbedtls.c|HAL_TLS_Connect(206): Setting up the SSL/TLS structure...
DBG|2020-03-04 16:50:39|HAL_TLS_mbedtls.c|HAL_TLS_Connect(248): Performing the SSL/TLS handshake...
DBG|2020-03-04 16:50:39|HAL_TLS_mbedtls.c|HAL_TLS_Connect(249): Connecting to /ota-1255858890.cos.ap-guangzhou.myqcloud.com/443...
INF|2020-03-04 16:50:40|HAL_TLS_mbedtls.c|HAL_TLS_Connect(271): connected with /ota-1255858890.cos.ap-guangzhou.myqcloud.com/443...
DBG|2020-03-04 16:50:40|utils_httpc.c|qcloud_http_client_connect(747): http client connect success
DBG|2020-03-04 16:50:40|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$ota/report/S3EUVBRJLB/test_device|payload={"type": "report_progress", "report": {"progress": {"state":"downloading", "percent":"0", "result_code":"0", "result_msg":""}, "version": "1.3.0"}}
DBG|2020-03-04 16:50:41|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$ota/report/S3EUVBRJLB/test_device|payload={"type": "report_progress", "report": {"progress": {"state":"downloading", "percent":"31", "result_code":"0", "result_msg":""}, "version": "1.3.0"}}
DBG|2020-03-04 16:50:42|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$ota/report/S3EUVBRJLB/test_device|payload={"type": "report_progress", "report": {"progress": {"state":"downloading", "percent":"59", "result_code":"0", "result_msg":""}, "version": "1.3.0"}}
DBG|2020-03-04 16:50:43|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$ota/report/S3EUVBRJLB/test_device|payload={"type": "report_progress", "report": {"progress": {"state":"downloading", "percent":"88", "result_code":"0", "result_msg":""}, "version": "1.3.0"}}
DBG|2020-03-04 16:50:43|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$ota/report/S3EUVBRJLB/test_device|payload={"type": "report_progress", "report": {"progress": {"state":"downloading", "percent":"100", "result_code":"0", "result_msg":""}, "version": "1.3.0"}}
INF|2020-03-04 16:50:43|ota_client.c|IOT_OTA_Ioctl(638): FW MD5 check: origin=ad4615b866c13afb8b293a679bfa5dc4, now=ad4615b866c13afb8b293a679bfa5dc4
INF|2020-03-04 16:50:43|ota_mqtt_sample.c|process_ota(456): The firmware is valid
DBG|2020-03-04 16:50:43|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=63335|topicName=$ota/report/S3EUVBRJLB/test_device|payload={"type": "report_progress", "report":{"progress":{"state":"done", "result_code":"0", "result_msg":""}, "version":"1.3.0"}}
INF|2020-03-04 16:50:44|ota_mqtt_sample.c|_event_handler(89): publish success, packet-id=63335
INF|2020-03-04 16:50:44|mqtt_client_connect.c|qcloud_iot_mqtt_disconnect(450): mqtt disconnect!
INF|2020-03-04 16:50:44|ota_mqtt.c|_otamqtt_event_callback(135): mqtt client has been destroyed
INF|2020-03-04 16:50:44|mqtt_client.c|IOT_MQTT_Destroy(171): mqtt release!
```

## 3. Code Use Instructions

1. To update the firmware, you can use the compilation option `FEATURE_OTA_USE_HTTPS` to select HTTP or HTTPS for firmware download.
2. The firmware update feature of the SDK supports checkpoint restart; that is, if the firmware download process is interrupted due to network or other issues, it will resume from where interrupted.
3. In `ota_mqtt_sample.c`, the `process_ota` function is the core process of OTA, which includes reporting the version, waiting for the update command, initiating a download and saving the downloaded file, verifying the MD5 value of the file, reporting the update result, and saving the download status in real time for the checkpoint restart feature. It can remain the same during adaption to different platforms. However, functions related to firmware saving and checkpoint restart status saving are subject to the platform. File operations are used in the demo, which can be reused directly on platforms that support standard file operations such as Linux and Windows but require adaptation for other platforms.


