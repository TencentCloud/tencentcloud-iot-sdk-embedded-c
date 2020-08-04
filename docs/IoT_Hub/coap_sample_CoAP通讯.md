# CoAP通讯
IoT Hub支持设备端通过CoAP协议接入，请参考[CoAP 协议说明](https://cloud.tencent.com/document/product/634/14063)

请先参考**mqtt_sample_快速入门.md**文档，介绍如何在腾讯云物联网通信IoT Hub控制台创建设备

## 编译 SDK
编译运行示例程序coap_sample，首先修改CMakeLists.txt确保以下选项存在(以密钥认证设备为例)
```
set(BUILD_TYPE                   "release")
set(COMPILE_TOOLS                "gcc") 
set(PLATFORM 	                "linux")
set(FEATURE_COAP_COMM_ENABLED ON)
set(FEATURE_AUTH_MODE "KEY")
set(FEATURE_AUTH_WITH_NOTLS OFF)
set(FEATURE_DEBUG_DEV_INFO_USED  OFF)
```
编译SDK

## 填写设备信息
将上面在腾讯云物联网IoT Hub创建的设备的设备信息(以**密钥认证设备**为例)填写到device_info.json中
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

## 执行coap_sample
```
./output/release/bin/coap_sample 
INF|2019-09-16 23:49:36|device.c|iot_device_info_set(67): SDK_Ver: 3.1.0, Product_ID: S3EUVBRJLB, Device_Name: test_device
INF|2019-09-16 23:49:38|coap_client.c|IOT_COAP_Construct(82): coap connect success
INF|2019-09-16 23:49:38|coap_client_message.c|coap_message_send(402): add coap message id: 9734 into wait list ret: 0
DBG|2019-09-16 23:49:38|coap_client_message.c|_coap_message_handle(295): receive coap piggy ACK message, id 9734
INF|2019-09-16 23:49:38|coap_client_auth.c|_coap_client_auth_callback(43): auth token message success, code_class: 2 code_detail: 5
DBG|2019-09-16 23:49:38|coap_client_auth.c|_coap_client_auth_callback(53): auth_token_len = 10, auth_token = YWAIGGUGUC
DBG|2019-09-16 23:49:38|coap_client_message.c|_coap_message_list_proc(146): remove the message id 9734 from list
INF|2019-09-16 23:49:38|coap_client_message.c|_coap_message_list_proc(85): remove node
INF|2019-09-16 23:49:38|coap_client.c|IOT_COAP_Construct(91): device auth successfully, connid: Xy9W9
INF|2019-09-16 23:49:38|coap_sample.c|main(170): topic name is S3EUVBRJLB/test_device/data
INF|2019-09-16 23:49:38|coap_client_message.c|coap_message_send(402): add coap message id: 9735 into wait list ret: 0
DBG|2019-09-16 23:49:38|coap_sample.c|main(177): client topic has been sent, msg_id: 9735
DBG|2019-09-16 23:49:39|coap_client_message.c|_coap_message_handle(295): receive coap piggy ACK message, id 9735
INF|2019-09-16 23:49:39|coap_sample.c|event_handler(78): message received ACK, msgid: 9735
DBG|2019-09-16 23:49:39|coap_client_message.c|_coap_message_list_proc(146): remove the message id 9735 from list
INF|2019-09-16 23:49:39|coap_client_message.c|_coap_message_list_proc(85): remove node
INF|2019-09-16 23:49:39|coap_client.c|IOT_COAP_Destroy(125): coap release!
```