# 资源管理上传与下载

关于物联平台资源管理上传和下载功能及流程介绍。
```
subscribe topic: $resource/down/service/product_id/device_name; 订阅接收云平台资源管理下行消息
publish   topic: $resource/up/service/product_id/device_name；向云平台发布资源管理消息
```

## 一. 资源上传

资源上传功能是设备将本地资源上传到云平台，时序如下图示：
![](https://main.qcloudimg.com/raw/fc22b31a6d51cbcc132ede99b11441c7.png)


注意: md5sum 是16字节的二进制转为了32字节的小写十六进制字符串；content-md5 值是16字节的 md5 二进制值通过 base64 编码得到的。
```
设备发布：{"type": "create_upload_task","size": resource_size,"name": "resource_name","md5sum": "md5sum resource_name"}
平台发布：{"type":"create_upload_task_rsp","size":resource_size,"name":"resource_name","md5sum":"md5sum resource_name","url":"upload http/https url"}

设备请求：(http put request upload url head: Content-Length: resource_size\r\nContent-MD5: md5sumToBinaryAfterBase64Encode)

设备发布：{"type":"report_upload_progress","name":"resource_name","progress":{"percent":98,"state":"uploading","result_code":0,"result_msg":""}}
平台发布：{"type":"report_upload_progress_rsp","result_code":0/err_code,"result_msg":"ok/err_msg"}

设备发布：{"type":"report_upload_progress","name":"resource_name","progress":{"state":"done/fail","result_code":0/err_code,"result_msg":"ok/err_msg"}}
平台请求：{"type":"report_upload_progress_rsp","result_code":0/err_code,"result_msg":"ok/err_msg"}

```

## 二. 资源下载

资源下载功能是云平台将资源下发到设备上，时序如下图示：
![](https://main.qcloudimg.com/raw/1e9e46815e397c26179b46028fc6e759.png)


注意: md5sum 是16字节的二进制转为了32字节的小写十六进制字符串。
```
平台发布：{"type":"download","size":resource_size,"name":"resource_name","md5sum":"resource_md5sum","url":"download http/https url"}

设备请求：(http get request upload url head: Range: bytes=%d-%d\r\n)

设备发布：{"type":"report_download_progress","name":"resource_name","progress":{"percent":0,"state":"downloading","result_code":0,"result_msg":""}}
平台发布：{"type":"report_download_progress_rsp","result_code":0/err_code,"result_msg":"ok/err_msg"}

设备发布：{"type":"report_download_progress","name":"test","progress":{"state":"done/fail","result_code":0/err_code"result_msg":"ok/err_msg"}}
平台发布：{"type":"report_download_progress_rsp","result_code":0/err_code,"result_msg":"ok/err_msg"}
```

## 三、控制台资源管理

### 资源下载
主页面：
![](https://main.qcloudimg.com/raw/274f81aa0a74821047dd68b64cd63097.png)
资源添加：
![](https://main.qcloudimg.com/raw/5bc3e2a855fd06f86553c1557733e3ae.png)
资源下发，单台模式：
![](https://main.qcloudimg.com/raw/6598e69bf89577a0d2944f9b6ff93e28.png)
资源下发，批量模式：
![](https://main.qcloudimg.com/raw/069a240ff06c5cf1f01e0d5ce56e7576.png)
下载进度：
![](https://main.qcloudimg.com/raw/d7d49d58cabf76029db5dae54a6ec565.png)
![](https://main.qcloudimg.com/raw/c412a0297faf5ed215e788495b0b56f5.png)

### 资源上传
主页面：
![](https://main.qcloudimg.com/raw/d2bc652e2ef749ccd294018708deacf2.png)
上传进度：
![](https://main.qcloudimg.com/raw/6163ff5decff75c3dde43422457d2dfb.png)

## 四. 编译运行示例程序(以**MQTT密钥认证设备**为例)
#### 1. 编译 SDK
修改CMakeLists.txt确保以下选项存在
```
set(BUILD_TYPE                   "release")
set(COMPILE_TOOLS                "gcc") 
set(PLATFORM 	                "linux")
set(FEATURE_MQTT_COMM_ENABLED ON)
set(FEATURE_RESOURCE_COMM_ENABLED ON)
set(FEATURE_RESOURCE_USE_HTTPS OFF)
set(FEATURE_RESOURCE_SIGNAL_CHANNEL "MQTT")
set(FEATURE_AUTH_MODE "KEY")
set(FEATURE_AUTH_WITH_NOTLS OFF)
set(FEATURE_DEBUG_DEV_INFO_USED  OFF)
```
执行脚本编译
```
./cmake_build.sh
```
示例输出 resource_mqtt_sample 位于`output/release/bin`文件夹中

#### 2. 填写设备信息
将设备信息填写到配置文件 device_info.json中
```
{
"auth_mode":"KEY",
"productId":"D95V6NB56B",
"deviceName":"deng4",
"productSecret":"HvDlqTo50VuumrAe2JAtn8EF",
"key_deviceinfo":{
"deviceSecret":"yiTRtLVwFsaarPC+q2ZnFw=="
}
}
```

#### 3. 运行示例
执行 resource_mqtt_sample：
```
在程序运行目录下创建名为 uploadtest 的资源，例子程序默认上传该名称的资源。
例子程序会先进行资源上传，然后进入资源下载等待平台下发资源下载的消息。
./output/release/bin/resource_mqtt_sample 
```

```
$ ./output/release/bin/resource_mqtt_sample 
INF|2021-03-15 10:35:24|qcloud_iot_device.c|iot_device_info_set(49): SDK_Ver: 3.2.2, Product_ID: D95V6NB56B, Device_Name: deng4
INF|2021-03-15 10:35:24|HAL_TCP_linux.c|HAL_TCP_Connect(105): connected with TCP server: 111.230.160.203:1883
INF|2021-03-15 10:35:25|mqtt_client.c|IOT_MQTT_Construct(118): mqtt connect with id: Bg308 success
INF|2021-03-15 10:35:25|resource_mqtt_sample.c|main(714): Cloud Device Construct Success
DBG|2021-03-15 10:35:25|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(142): topicName=$resource/down/service/D95V6NB56B/deng4|packet_id=31595
INF|2021-03-15 10:35:25|resource_mqtt_sample.c|_event_handler(72): subscribe success, packet-id=31595
DBG|2021-03-15 10:35:25|resource_mqtt.c|_qcloud_resource_mqtt_event_callback(68): resource topic subscribe success
```
```
资源上传日志
ERR|2021-03-15 10:35:26|resource_mqtt_sample.c|main(775): resource upload enter
DBG|2021-03-15 10:35:26|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=31596|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type": "create_upload_task","size": 1174267,"name": "uploadtest","md5sum": "2af9455c6051c337cbfba7fbc0e1abc1"}
INF|2021-03-15 10:35:26|resource_mqtt_sample.c|_event_handler(84): publish success, packet-id=31596
DBG|2021-03-15 10:35:26|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:26|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=519, topic_msg={"type":"create_upload_task_rsp","size":1174267,"name":"uploadtest","md5sum":"2af9455c6051c337cbfba7fbc0e1abc1","url":"https://iothub-tasktest-1258344699.cos.ap-guangzhou.myqcloud.com/100000548204%2FD95V6NB56B_device%2Fdeng4%2Fuploadtest?sign=q-sign-algorithm%3Dsha1%26q-ak%3DAKIDCMK8Tk7KMDyZLys1KQivxnYBgz74lZ2A%26q-sign-time%3D1615775725%3B1615779325%26q-key-time%3D1615775725%3B1615779325%26q-header-list%3Dcontent-length%3Bcontent-md5%26q-url-param-list%3D%26q-signature%3D70c8e04d360528d73a49215485aab31fc1bba283"}
INF|2021-03-15 10:35:27|resource_mqtt_sample.c|process_resource_upload(568): wait for resource upload command...
DBG|2021-03-15 10:35:27|resource_client.c|QCLOUD_IOT_RESOURCE_StartUpload(583): to upload resource total size: 1174267
ERR|2021-03-15 10:35:27|resource_upload.c|qcloud_resource_upload_http_init(81): http head:Content-MD5:KvlFXGBRwzfL+6f7wOGrwQ==

INF|2021-03-15 10:35:27|HAL_TCP_linux.c|HAL_TCP_Connect(105): connected with TCP server: iothub-tasktest-1258344699.cos.ap-guangzhou.myqcloud.com:80
DBG|2021-03-15 10:35:27|utils_httpc.c|qcloud_http_client_connect(845): http client connect success
DBG|2021-03-15 10:35:27|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":0,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:27|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:27|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:28|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":4,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:28|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:28|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:29|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":8,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:29|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:29|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:30|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":13,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:30|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:30|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:31|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":17,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:31|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:31|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:32|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":21,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:32|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:32|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:33|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":25,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:33|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:33|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:34|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":30,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:34|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:34|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:35|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":34,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:35|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:35|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:36|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":38,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:36|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:36|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:37|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":43,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:37|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:37|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:38|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":47,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:38|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:38|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:39|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":51,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:39|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:39|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:40|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":55,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:40|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:40|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:41|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":60,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:41|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:41|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:42|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":64,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:42|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:42|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:43|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":68,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:43|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:43|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:44|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":72,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:44|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:44|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:45|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":77,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:45|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:45|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:46|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":81,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:46|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:46|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:47|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":85,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:47|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:47|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:48|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
DBG|2021-03-15 10:35:48|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":89,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:48|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:48|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:49|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":94,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:49|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:49|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:50|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":98,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:50|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:50|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:35:51|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"percent":100,"state":"uploading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:35:53|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:53|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
ERR|2021-03-15 10:35:53|resource_client.c|QCLOUD_IOT_RESOURCE_UploadIoctl(1109): upload MD5 check: origin=2af9455c6051c337cbfba7fbc0e1abc1, now=2af9455c6051c337cbfba7fbc0e1abc1
INF|2021-03-15 10:35:53|resource_mqtt_sample.c|process_resource_upload(626): The upload resource is valid
DBG|2021-03-15 10:35:54|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=31597|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_upload_progress","name":"uploadtest","progress":{"state":"done","result_code":0,"result_msg":""}}
INF|2021-03-15 10:35:54|resource_mqtt_sample.c|_event_handler(84): publish success, packet-id=31597
DBG|2021-03-15 10:35:54|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:35:54|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=71, topic_msg={"type":"report_upload_progress_rsp","result_code":0,"result_msg":"ok"}
INF|2021-03-15 10:35:54|resource_client.c|_qcloud_iot_resource_upload_reset(230): reset resource upload state!
```
```
资源下载日志
INF|2021-03-15 10:35:48|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=458, topic_msg={"type":"download","size":401463,
"name":"test","md5sum":"935c4b721df0ac41d1b09a7cf6a4cc22","url":"https://iothub-tasktest-1258344699.cos.ap-guangzhou.myqcloud.com/100000548204%2FD95V6NB56B_product%2Ftest?sign=q-sign-algorithm%3Dsha1%26q-ak%3DAKIDCMK8Tk7KMDyZLys1KQivxnYBgz74lZ2A%26q-sign-time%3D1615775747%3B1615776347%26q-key-time%3D1615775747%3B1615776347%26q-header-list%3D%26q-url-param-list%3D%26q-signature%3D1691fa6154a160acfc997a29d31afb7e4435d9bc"}
ERR|2021-03-15 10:35:59|resource_mqtt_sample.c|main(782): resource download enter
INF|2021-03-15 10:35:59|resource_mqtt_sample.c|process_resource_download(419): wait for resource download command...
ERR|2021-03-15 10:35:59|resource_mqtt_sample.c|_get_local_resource_info(267): open file ./download_D95V6NB56Bdeng4.json failed
DBG|2021-03-15 10:35:59|resource_client.c|QCLOUD_IOT_RESOURCE_StartDownload(872): to download FW from offset: 0, size: 401463
DBG|2021-03-15 10:35:59|ota_fetch.c|ofc_Init(79): head_content:Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Accept-Encoding: gzip, deflate
Range: bytes=0-401463

INF|2021-03-15 10:35:59|HAL_TCP_linux.c|HAL_TCP_Connect(105): connected with TCP server: iothub-tasktest-1258344699.cos.ap-guangzhou.myqcloud.com:80
DBG|2021-03-15 10:35:59|utils_httpc.c|qcloud_http_client_connect(845): http client connect success
DBG|2021-03-15 10:36:00|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_download_progress","name":"test","progress":{"percent":0,"state":"downloading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:36:00|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:36:00|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=73, topic_msg={"type":"report_download_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:36:01|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_download_progress","name":"test","progress":{"percent":12,"state":"downloading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:36:01|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:36:01|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=73, topic_msg={"type":"report_download_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:36:02|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_download_progress","name":"test","progress":{"percent":24,"state":"downloading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:36:02|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:36:02|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=73, topic_msg={"type":"report_download_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:36:03|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_download_progress","name":"test","progress":{"percent":37,"state":"downloading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:36:03|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:36:03|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=73, topic_msg={"type":"report_download_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:36:04|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_download_progress","name":"test","progress":{"percent":49,"state":"downloading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:36:04|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:36:04|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=73, topic_msg={"type":"report_download_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:36:05|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_download_progress","name":"test","progress":{"percent":62,"state":"downloading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:36:05|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:36:05|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=73, topic_msg={"type":"report_download_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:36:06|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_download_progress","name":"test","progress":{"percent":74,"state":"downloading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:36:06|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:36:06|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=73, topic_msg={"type":"report_download_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:36:07|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_download_progress","name":"test","progress":{"percent":87,"state":"downloading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:36:07|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:36:07|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=73, topic_msg={"type":"report_download_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:36:08|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_download_progress","name":"test","progress":{"percent":99,"state":"downloading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:36:08|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:36:08|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=73, topic_msg={"type":"report_download_progress_rsp","result_code":0,"result_msg":"ok"}
DBG|2021-03-15 10:36:08|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_download_progress","name":"test","progress":{"percent":100,"state":"downloading","result_code":0,"result_msg":""}}
DBG|2021-03-15 10:36:08|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:36:08|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=73, topic_msg={"type":"report_download_progress_rsp","result_code":0,"result_msg":"ok"}
INF|2021-03-15 10:36:08|resource_client.c|QCLOUD_IOT_RESOURCE_DownloadIoctl(1021): download MD5 check: origin=935c4b721df0ac41d1b09a7cf6a4cc22, now=935c4b721df0ac41d1b09a7cf6a4cc22
INF|2021-03-15 10:36:08|resource_mqtt_sample.c|process_resource_download(495): The resource is valid
DBG|2021-03-15 10:36:09|mqtt_client_publish.c|qcloud_iot_mqtt_publish(334): publish topic seq=31598|topicName=$resource/up/service/D95V6NB56B/deng4|payload={"type":"report_download_progress","name":"test","progress":{"state":"done","result_code":0,"result_msg":""}}
INF|2021-03-15 10:36:09|resource_client.c|_qcloud_iot_resource_download_reset(214): reset resource download state!
INF|2021-03-15 10:36:10|resource_mqtt_sample.c|_event_handler(84): publish success, packet-id=31598
DBG|2021-03-15 10:36:10|resource_mqtt.c|_qcloud_resource_mqtt_cb(53): topic=$resource/down/service/D95V6NB56B/deng4
INF|2021-03-15 10:36:10|resource_mqtt.c|_qcloud_resource_mqtt_cb(54): len=73, topic_msg={"type":"report_download_progress_rsp","result_code":0,"result_msg":"ok"}
INF|2021-03-15 10:36:15|resource_mqtt.c|_qcloud_resource_mqtt_event_callback(82): resource topic has been unsubscribed
INF|2021-03-15 10:36:15|mqtt_client_connect.c|qcloud_iot_mqtt_disconnect(464): mqtt disconnect!
INF|2021-03-15 10:36:15|mqtt_client.c|IOT_MQTT_Destroy(182): mqtt release!
```

## 五. 代码使用说明

1. 资源上传与下载，支持通过 HTTP 或者 HTTPS 的方式，可以通过编译选项 FEATURE_RESOURCE_USE_HTTPS 来选择
2. 资源的下载，支持断点续传功能，即当资源下载过程因为网络或者其他原因中断了，则再次下载的时候，可以从上一次中断的位置继续下载。
3. 在 resource_mqtt_sample.c 中，process_resource_download 函数为资源下载的核心流程，与时序图对照。
3. 在 resource_mqtt_sample.c 中，process_resource_upload 函数为资源上传的核心流程，与时序图对照。

