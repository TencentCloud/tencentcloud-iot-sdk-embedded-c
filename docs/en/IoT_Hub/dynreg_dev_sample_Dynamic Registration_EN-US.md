# Device Connection and Dynamic Registration

For more information on how to authenticate a device for connection to IoT Hub, please see [Overview](https://cloud.tencent.com/document/product/634/35272).
Simply put, IoT Hub provides three authentication methods for device connection:

- Certificate authentication (device-level): it assigns a certificate + private key to each device and uses asymmetric encryption to authenticate the connection. You need to burn different configuration information for each device.
- Key authentication (device-level): it assigns a device key to each device and uses symmetric encryption to authenticate the connection. You need to burn different configuration information for each device.
- Dynamic registration authentication (product-level): it assigns a unified key to all devices under the same product, and a device gets a device certificate/key through a registration request for authentication. You can burn the same configuration information for the same batch of devices.

The dynamic device registration feature allows you to burn unified firmware for devices of the same product model before they are shipped, which contains only the unified product ID and product key. Then, when the device information is determined to be empty (for more information on how to implement the determining logic, please see the demo), dynamic device registration will be triggered, and the device certificate (for certificate-authenticated products) or device key (for key-authenticated products) will be requested from the platform and saved in the device information for subsequent device connection.

## 1. Enabling Dynamic Registration in Console

During dynamic registration, there are two ways to generate device names. One is to enable automatic device creation in the console. In this case, device names can be generated automatically, which are generally unique device identifiers (such as CPUID or MAC address) but must be unique under the same product ID. The other is to enter the device names in the console in advance and enter the same names during dynamic registration, which is more secure but less convenient.

You can enable dynamic registration in the console as shown below:
![](https://main.qcloudimg.com/raw/cbeaa7b27399d363c6cafa1b7ee5eb1c.png)

Save the product key `ProductSecret`.

## 2. Compiling and Running Demo (with **Key-Authenticated Device** as Example)
#### 1. Compile the SDK
Modify `CMakeLists.txt` to ensure that the following options exist:
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
Run the following script for compilation.
```
./cmake_build.sh 
```
The demo output `dynreg_dev_sample` is in the `output/release/bin` folder.

#### 2. Enter the device information
Enter the product information obtained in the console in the JSON file `dynreg_device_info.json`, with the device name to be generated in the `deviceName` field and "YOUR_IOT_PSK" in the `deviceSecret` field, and `dynreg_dev_sample` will determine that the device information is empty and know that the device needs to be dynamically registered. You can implement this logic on your own, and the sample is used as an example only.
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
#### 3. Run the demo
Run the dynamic registration demo `dynreg_dev_sample`:
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
You can see that the device has been dynamically registered successfully, and the obtained device key has been written to the `dynreg_device_info.json` file.
