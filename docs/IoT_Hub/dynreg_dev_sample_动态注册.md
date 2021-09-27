# 设备接入及动态注册

关于IoT Hub设备接入认证的详细介绍，请参考 [设备身份认证 ](https://cloud.tencent.com/document/product/634/35272)
简单来说，物联网通信平台IoT Hub提供三种设备端接入认证的方案：

- 证书认证（设备级）：为每台设备分配证书 + 私钥，使用非对称加密认证接入，用户需要为每台设备烧录不同的配置信息。
- 密钥认证（设备级）：为每台设备分配设备密钥，使用对称加密认证接入，用户需要为每台设备烧录不同的配置信息。
- 动态注册认证（产品级）：为同一产品下的所有设备分配统一密钥，设备通过注册请求获取设备证书/密钥后认证接入，用户可以为同一批设备烧录相同的配置信息。

使用设备动态注册功能，可以使得同一个产品型号的设备出厂时烧录的是统一的固件，该固件只包含统一的产品ID和产品密钥。设备出厂后判断设备信息为空（判断设备信息为空的逻辑由业务逻辑实现，可参考示例），则触发设备动态注册，从平台申请设备的证书（创建的产品为证书认证产品）或者设备的密钥（创建的产品为密钥认证方式），将获取到的设备信息保存下来之后，后续的设备接入就可以使用。

## 一. 控制台使能动态注册

动态注册时，设备名称的生成有两种方式，一种是在控制台使能了动态注册产品的自动创建设备功能，则设备可以自行生成设备名称，但需保证同一产品下没有重复，一般取设备的唯一信息比如CPUID或mac地址。另一种是没有使能动态注册产品的自动创建设备，则需要在控制台预先录入各设备的名称，且设备动态注册时设备要与录入的设备名称一致，此种方式更加安全，便利性有所下降。

控制台使能动态注册设置如下图示：
![](https://main.qcloudimg.com/raw/a02f57cbe40f26ead94170396d78253c.jpg)

将产品密钥ProductSecret保存下来。

## 二. 编译运行示例程序

### 1. 密钥认证设备注册示例

#### 1. 编译 SDK
修改CMakeLists.txt确保以下选项存在
```
set(BUILD_TYPE                   "release")
set(COMPILE_TOOLS                "gcc") 
set(PLATFORM 	                "linux")
set(FEATURE_MQTT_COMM_ENABLED ON)
set(FEATURE_DEV_DYN_REG_ENABLED ON)
set(FEATURE_AUTH_MODE "KEY")
set(FEATURE_AUTH_WITH_NOTLS OFF)
set(FEATURE_DEBUG_DEV_INFO_USED  OFF)
```
执行脚本编译
```
./cmake_build.sh 
```
示例输出dynreg_dev_sample位于`output/release/bin`文件夹中

#### 2. 填写设备信息
将控制台获取到的产品信息填写到一个JSON文件dynreg_device_info.json中，其中deviceName字段填写要生成的设备名字，deviceSecret字段保持为"YOUR_IOT_PSK"，这样dynreg_dev_sample就会判断设备信息为空，知道这个设备是需要进行动态注册。这部分逻辑可以由用户自行实现，sample仅作示例。
```
{
    "auth_mode":"KEY",
    "productId":"S3EUVBRJLB",
    "productSecret":"8Xz56tyfgQAZEDCTUGau4snA",
    "deviceName":"device_1234",
    "key_deviceinfo":{
        "deviceSecret":"YOUR_IOT_PSK"
    }
}
```
#### 3. 运行示例
执行设备动态注册例程dynreg_dev_sample：
```
./output/release/bin/dynreg_dev_sample -c ./dynreg_device_info.json 
DBG|2019-09-17 11:50:35|dynreg_dev_sample.c|main(80): dev psk not exist!
DBG|2019-09-17 11:50:35|dynreg.c|IOT_DynReg_Device(467): sign:ZWM4YTEyMWE2ODUxYzk1M2Q0MDc2OWNmN2FhMTg1ZWM1ODgxMWNkNQ==
DBG|2019-09-17 11:50:35|dynreg.c|IOT_DynReg_Device(483): request:{"deviceName":"device_1234","nonce":1439928322,"productId":"S3EUVBRJLB","timestamp":1568692235,"signature":"ZWM4YTEyMWE2ODUxYzk1M2Q0MDc2OWNmN2FhMTg1ZWM1ODgxMWNkNQ=="}
DBG|2019-09-17 11:50:35|dynreg.c|IOT_DynReg_Device(485): resbuff len:256
DBG|2019-09-17 11:50:35|HAL_TLS_mbedtls.c|_mbedtls_client_init(134): psk/pskid is empty!|psk=(null)|psd_id=(null)
DBG|2019-09-17 11:50:35|HAL_TLS_mbedtls.c|HAL_TLS_Connect(204): Setting up the SSL/TLS structure...
DBG|2019-09-17 11:50:35|HAL_TLS_mbedtls.c|HAL_TLS_Connect(246): Performing the SSL/TLS handshake...
DBG|2019-09-17 11:50:35|HAL_TLS_mbedtls.c|HAL_TLS_Connect(247): Connecting to /gateway.tencentdevices.com/443...
INF|2019-09-17 11:50:35|HAL_TLS_mbedtls.c|HAL_TLS_Connect(269): connected with /gateway.tencentdevices.com/443...
DBG|2019-09-17 11:50:35|utils_httpc.c|qcloud_http_client_connect(749): http client connect success
DBG|2019-09-17 11:50:37|dynreg.c|_parse_devinfo(244): recv: {"code":0,"message":"","len":53,"payload":"eHGRUBar9LBPI+mcaHnVEZsj05mSdoZLPhd54hDJXv/2va2rXSpfddgRy5XE/FIS835NjUr5Mhw1AJSg4yGC/w=="}
DBG|2019-09-17 11:50:37|dynreg.c|_parse_devinfo(258): payload:eHGRUBar9LBPI+mcaHnVEZsj05mSdoZLPhd54hDJXv/2va2rXSpfddgRy5XE/FIS835NjUr5Mhw1AJSg4yGC/w==
DBG|2019-09-17 11:50:37|dynreg.c|IOT_DynReg_Device(489): request dev info success
DBG|2019-09-17 11:50:37|HAL_Device_linux.c|iot_save_devinfo_to_json_file(340): JsonDoc(183):{
"auth_mode":"KEY",
"productId":"S3EUVBRJLB",
"productSecret":"8XzjN8rfgFVGDTNTUGau4snA",
"deviceName":"device_1234",
"key_deviceinfo":{
"deviceSecret":"7WmFArtyFGH5632QwJWtYwio"
}
}
DBG|2019-09-17 11:50:37|dynreg_dev_sample.c|main(99): dynamic register success,productID: S3EUVBRJLB, devName: device_1234, device_secret: 7WmFArtyFGH5632QwJWtYwio
```
可以看到设备动态注册已经成功，获取到的设备密钥也写入到dynreg_device_info.json文件中了。

### 2. 平台证书动态注册示例

 #### 1. 编译SDK

   修改CMakeLists.txt确保以下选项存在

   ```
   set(BUILD_TYPE                   "release")
   set(COMPILE_TOOLS                "gcc") 
   set(PLATFORM 	                "linux")
   set(FEATURE_MQTT_COMM_ENABLED ON)
   set(FEATURE_DEV_DYN_REG_ENABLED ON)
   set(FEATURE_AUTH_MODE "CERT")
   set(FEATURE_AUTH_WITH_NOTLS OFF)
   set(FEATURE_DEBUG_DEV_INFO_USED  OFF)
   ```

#### 2. 填写设备信息

   将控制台获取到的产品信息填写到一个`JSON`文件`dynreg_device_info.json`中，其中`deviceName`字段填写你想要生成的设备名字，`devCertFile`为`YOUR_DEVICE_CERT_FILE_NAME`, `devPrivateKeyFile`为`YOUR_DEVICE_PRIVATE_KEY_FILE_NAME`，dynreg_dev_sample会根据这两个字段的名字来判定需要使用平台证书来完成动态注册。

   ```
{
    "auth_mode": "CERT",
    "productId": "V5YXNNUIF4",
    "deviceName": "dev02221",
    "productSecret": "Rqwu1ZvwM8WpDrKYpqjcG9fF",
    "cert_deviceinfo": {
        "devCertFile": "YOUR_DEVICE_CERT_FILE_NAME",
        "devPrivateKeyFile": "YOUR_DEVICE_PRIVATE_KEY_FILE_NAME"
    }
}
   ```

#### 3. 运行示例

执行设备动态注册例程dynreg_dev_sample，sample，会从平台获取证书文件并保存到certs文件夹里。
```
$ ./output/release/bin/dynreg_dev_sample 
DBG|2021-09-09 16:08:32|dynreg_dev_sample.c|main(69): dev Cert not exist!
ERR|2021-09-09 16:08:32|dynreg.c|_get_dev_cert_file(269): fail to open file dev02221_cert.crt
request:{"ProductId":"V5YXNNUIF4","DeviceName":"dev02221"}
DBG|2021-09-09 16:08:32|HAL_TLS_mbedtls.c|_mbedtls_client_init(196): cert_file/key_file is empty!|cert_file=null|key_file=null
DBG|2021-09-09 16:08:32|HAL_TLS_mbedtls.c|HAL_TLS_Connect(273): Setting up the SSL/TLS structure...
DBG|2021-09-09 16:08:32|HAL_TLS_mbedtls.c|HAL_TLS_Connect(315): Performing the SSL/TLS handshake...
DBG|2021-09-09 16:08:32|HAL_TLS_mbedtls.c|HAL_TLS_Connect(316): Connecting to /ap-guangzhou.gateway.tencentdevices.com/443...
INF|2021-09-09 16:08:32|HAL_TLS_mbedtls.c|HAL_TLS_Connect(338): connected with /ap-guangzhou.gateway.tencentdevices.com/443...
DBG|2021-09-09 16:08:32|utils_httpc.c|qcloud_http_client_connect(745): http client connect success
DBG|2021-09-09 16:08:32|dynreg.c|_post_reg_request_by_http(505): dynamic register recvl, Ret = 0
DBG|2021-09-09 16:08:32|utils_httpc.c|_http_client_retrieve_content(455): no more (last chunk)
recv: {"Response":{"Len":3011,"Payload":"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","RequestId":"5c722b0b-a503-4bd0-b5a3-d9d769ecd02d"}}
ERR|2021-09-09 16:08:32|dynreg.c|_get_json_resault_code(106): Invalid json content: {"Response":{"Len":3011,"Payload":"PxuttttttttttttttttttttttteeeeeeeeeeeeeeeeeeeeesssssssssssssssssssttttttttttttttttttttttttttttttttttttClcW8dKhXDR2JbZWF9cK8SIMpNB1lIPxEKrlXvdudqey92d3I6RHgpyY6R+w3x0Etxz2LKDxMrv8+8zpJ5kKZDq3SArL6kTCc2/Ry69q9DUwq35avexkDH9oxEu5QDp9tq3elI+hC7QQf020vpND4MzdakAKI4dhhb2eDDua5u2fu6Sbsntk/dRprvvvGm0YsRlW5cIRdwI1z8o9gYlZhadSPZJSZpsAjCH4A/KSPLvIzsO916C79oDSGwQmb/dmOj4P85P/pDNqD7LKFSv8qRdQYZc3mcsVZRxntwf/T5EX1o3zWUZ/P/vpRTf8iOa9fYEx4ob/NKxywWTqAy2eylQTEM55qdAQSCeXO5PxMlRKJFm0a7maEXak8ABrMJJMI34+AhT+8C0OQGPLpYMFxziOdBdT6C8y3Ixmr23f4P
DBG|2021-09-09 16:08:32|dynreg.c|_parse_devinfo(353): payload:PxunWaI74fg4U6ttttttttttttttttttttttttttttttttttteeeeeeeeeeeeeeeeeeeeeeeeeeeesssssssssssssssssssssssssstttttttttttttttttttttttttQmb/dmOj4P85P/pDNqD7LKFSv8qRdQYZc3mcsVZRxntwf/T5EX1o3zWUZ/P/vpRTf8iOa9fYEx4ob/NKxywWTqAy2eylQTEM55qdAQSCeXO5PxMlRKJFm0a7maEXak8ABrMJJMI34+AhT+8C0OQGPLpYMFxziOdBdT6C8y3Ixmr23f4PMSY73I18STketFo8uuZB4DvVNc/V8PzOcxFsLSP2kfkeQ2JeHDNtKSL/P
DBG|2021-09-09 16:08:32|dynreg.c|_parse_devinfo(372): The decrypted data is:{"encryptionType":1,"clientCert":"-----BEGIN CERTIFICATE-----\nMIIDUDCCAjigAwIBttttttttttttttttttttttttttttttttttteeeeeeeeeeeeeeeeeeeeeeeeessssssssssssssssssssssstttttttttttttttttttttttttttttRWC7vaEXZVNAnp7/S2c\nb2QqbhakfgJ9lxcdyq01XpLnRUh9ycoaQM6RivFejTmmFelc91ykhVIj5XsVIyQ3\niAfBLE/Gv4C95NjUbrBTRY2UZu+ltA9AWMK1B/PWN7vpFooVSw0X/UhvvzN87W3S\nqRRIjg8+virGz5Z03vzUFpYX2mc2e5VqEx8CAwEAAaM/MD0wDAYDVR0TAQH/BAIw\nADAOBgNVHQ8BAf8EBAMCB4AwHQYDVR0OBBYEFLtZjDKOGeM/iyks43xGdB2n/Wyn\nMA0GCSqGSIb3DQEBCwUAA4IBAQCzyFNCAAfkj9y71H0UYGHaH6rTSknEZNFxGJav\nBPtyRLH/R1s7JqOIKHvKOAR
DBG|2021-09-09 16:08:32|dynreg.c|_cert_file_save(232): save dev02221_cert.crt file succes
DBG|2021-09-09 16:08:32|dynreg.c|_cert_file_save(232): save dev02221_private.key file succes
DBG|2021-09-09 16:08:32|dynreg.c|IOT_DynReg_Device(565): request dev info success
DBG|2021-09-09 16:08:32|HAL_Device_linux.c|iot_save_devinfo_to_json_file(362): JsonDoc(218):{
"auth_mode":"CERT",
"productId":"V5YXNNUIF4",
"deviceName":"dev02221",
"productSecret":"Rqwu1ZvwM8WpDrKYpqjcG9fF",
"cert_deviceinfo":{
"devCertFile":"dev02221_cert.crt",
"devPrivateKeyFile":"dev02221_private.key"
}
}
DBG|2021-09-09 16:08:32|dynreg_dev_sample.c|main(100): dynamic register success, productID: V5YXNNUIF4, devName: dev02221, CertFile: dev02221_cert.crt, KeyFile: dev02221_private.key  
```

### 3. 私有证书动态注册示例

关于私有证书的生成，请参考文档中心，[证书管理](https://cloud.tencent.com/document/product/634/59363)

 #### 1. 编译SDK

   修改CMakeLists.txt确保以下选项存在

   ```
   set(BUILD_TYPE                   "release")
   set(COMPILE_TOOLS                "gcc") 
   set(PLATFORM 	                "linux")
   set(FEATURE_MQTT_COMM_ENABLED ON)
   set(FEATURE_DEV_DYN_REG_ENABLED ON)
   set(FEATURE_AUTH_MODE "CERT")
   set(FEATURE_AUTH_WITH_NOTLS OFF)
   set(FEATURE_DEBUG_DEV_INFO_USED  OFF)
   ```

#### 2. 填写设备信息

   将控制台获取到的产品信息填写到一个`JSON`文件`dynreg_device_info.json`中，其中`deviceName`字段填写你生成私有证书时填写的设备名字，`devCertFile`为`设备名_cert.crt`, `devPrivateKeyFile`为`设备名_private.key`

   ```
   {
       "auth_mode": "CERT",
       "productId": "33E7HIWES4",
       "deviceName": "dev011",
       "productSecret": "biraxh1bZv6o2cKSdceR0Vhi",
       "cert_deviceinfo": {
           "devCertFile": "dev011_cert.crt",
           "devPrivateKeyFile": "dev011_private.key"
       }
   }
   ```

   同时，在sdk的certs目录下，存放你的私有证书文件。

   ```
   ├── certs
   │   ├── README.md
   │   ├── dev011_cert.crt  	# 私有证书文件要放到certs文件夹下
   │   └── dev011_private.key
   ```

   

#### 3. 运行示例

执行设备动态注册例程dynreg_dev_sample，sample会首先从certs文件夹下读取私有证书文件，然后上传到平台，平台做鉴权后返回动态注册结果。

```
$ ./output/release/bin/dynreg_dev_sample
DBG|2021-09-09 15:37:43|dynreg_dev_sample.c|main(73): dev Cert is exist!
DBG|2021-09-09 15:37:43|dynreg.c|_get_fileSize(249): file length: 1582
request:{"ProductId":"33E7HIWES4","DeviceName":"dev011","clientCert":"-----BEGIN CERTIFICATE-----\nMIIEZDCCAkygAwIBAgIUIPxOT6M+nMZ/XSperldmLfBr87EwDQYJKoZIhvcNAQEN\nBQAwXDELMAkGA1UEBhMCQ04xDjAMBgNVBAgMBUhlbGxvMREwDwYDVQQHDAhTaGVu\nemhlbjESMBAGA1UECgwJSGVsbG8gSW9UMRYwFAYDVQQDDA1IZWxsbyBDQSBUZXN0\nMB4XDTIxMDkwOTA3MzY1NVoXDTMxMDkwNzA3MzY1NtttttttttttttttttttttttttteeeeeeeeeeeeeeeeeeeeeeeeeesssssssssssssssssssssssssssstttttttttttttttttttttttttrxPo+TMnei7l8MCrXHc6KioFm/HLwCHlkTSqBxQMN5GaqC8ZW9BWz1\nLj6RkDk4hElT4Z1P4AVYiaGWqTtO67NCW3ez2XV3vtM/ieiPQRyFKQ1MFKaU0vdz\nh0Jfz4GIAYHrYZSIt/t2VuRl/3l8Gy2VJilx7OF2U6pvw8O2agwRuIfsQkWenZSO\npkAlgt3Hgjr2nZghTMtAJgjLWZykT9pD\n-----END CERTIFICATE-----\n"}
DBG|2021-09-09 15:37:43|HAL_TLS_mbedtls.c|_mbedtls_client_init(196): cert_file/key_file is empty!|cert_file=null|key_file=null
DBG|2021-09-09 15:37:43|HAL_TLS_mbedtls.c|HAL_TLS_Connect(273): Setting up the SSL/TLS structure...
DBG|2021-09-09 15:37:43|HAL_TLS_mbedtls.c|HAL_TLS_Connect(315): Performing the SSL/TLS handshake...
DBG|2021-09-09 15:37:43|HAL_TLS_mbedtls.c|HAL_TLS_Connect(316): Connecting to /ap-guangzhou.gateway.tencentdevices.com/443...
INF|2021-09-09 15:37:43|HAL_TLS_mbedtls.c|HAL_TLS_Connect(338): connected with /ap-guangzhou.gateway.tencentdevices.com/443...
DBG|2021-09-09 15:37:43|utils_httpc.c|qcloud_http_client_connect(745): http client connect success
DBG|2021-09-09 15:37:43|dynreg.c|_post_reg_request_by_http(505): dynamic register recvl, Ret = 0
recv: {"Response":{"Len":20,"Payload":"T1WVr7/6uI5g3M58BSYcAf7FdlwJDNa9YPseIZPw5xE=","RequestId":"a2793e5b-0e5a-4b71-bd52-90b81392114c"}}
ERR|2021-09-09 15:37:45|dynreg.c|_get_json_resault_code(106): Invalid json content: {"Response":{"Len":20,"Payload":"T1WVr7/6uI5g3M58BSYcAf7FdlwJDNa9YPseIZPw5xE=","RequestId":"a2793e5b-0e5a-4b71-bd52-90b81392114c"}}
DBG|2021-09-09 15:37:45|dynreg.c|_parse_devinfo(353): payload:T1WVr7/6uI5g3M58BSYcAf7FdlwJDNa9YPseIZPw5xE=
DBG|2021-09-09 15:37:45|dynreg.c|_parse_devinfo(372): The decrypted data is:{"encryptionType":1}
DBG|2021-09-09 15:37:45|dynreg.c|IOT_DynReg_Device(565): request dev info success
DBG|2021-09-09 15:37:45|HAL_Device_linux.c|iot_save_devinfo_to_json_file(362): JsonDoc(212):{
"auth_mode":"CERT",
"productId":"33E7HIWES4",
"deviceName":"dev011",
"productSecret":"biraxh1bZv6o2cKSdceR0Vhi",
"cert_deviceinfo":{
"devCertFile":"dev011_cert.crt",
"devPrivateKeyFile":"dev011_private.key"
}
}
DBG|2021-09-09 15:37:45|dynreg_dev_sample.c|main(100): dynamic register success, productID: 33E7HIWES4, devName: dev011, CertFile: dev011_cert.crt, KeyFile: dev011_private.key
```



