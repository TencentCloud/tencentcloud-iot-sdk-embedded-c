# IoT Hub Device C-SDK 

IoT Hub device C-SDK relies on a secure and powerful data channel to enable IoT developers to quickly connect devices to the cloud for two-way communication.

After v3.1.0, the SDK refactored and optimized the compilation environment, code, and directory structure, increasing the availability and portability.

This SDK is applicable only to IoT Hub. For the SDK of IoT Explorer, please see [IoT Explorer C-SDK](https://github.com/tencentyun/qcloud-iot-explorer-sdk-embedded-c).

## 1. Scope of Application of C-SDK
Featuring a modular design, the C-SDK separates the core protocol service from the hardware abstraction layer and provides flexible configuration options and multiple compilation methods, making it suitable for development platforms and use environments of different devices.

#### 1. Network communication-capable devices on Linux/Windows

For devices that have network communication capabilities and run on standard Linux/Windows, such as PCs, servers, and gateway devices, as well as advanced embedded devices such as Raspberry Pi, you can directly compile and run the SDK on them.
For embedded Linux devices that require cross compilation, if the toolchain of the development environment has `glibc` or similar libraries which can provide system calls, including socket communication, SELECT sync IO, dynamic memory allocation, functions for getting time/sleeping/generating random number/printing, as well as critical data protection such as the mutex mechanism (only when multiple threads are required), only simple adaptation (e.g., changing the cross compiler settings in `CMakeLists.txt` or `make.settings`) is required before the SDK can be compiled and run.

#### 2. Network communication-capable devices on RTOS

For IoT devices that have network communication capabilities and run on RTOS, the C-SDK needs to be adapted to different RTOS systems for porting. Currently, it has been adapted to multiple IoT-oriented RTOS platforms, including FreeRTOS, RT-Thread, and TencentOS tiny.
When porting the SDK to an RTOS device, if the platform provides C runtime libraries like `Newlib` and embedded TCP/IP protocol stacks like lwIP, adaptation for porting can be done easily.

#### 3. Devices with MCU + communication module

For MCUs that have no network communication capabilities, the "MCU + communication module" combination is often used. Communication modules (including Wi-Fi/2G/4G/NB-IoT) generally provide serial port-based AT instruction protocols for MCUs to communicate over the network. For this scenario, the C-SDK encapsulates the AT-socket network layer, where the core protocol and service layer don't need to be ported. In addition, it provides FreeRTOS-based and nonOS HAL implementation methods. For more information, please see the corresponding document in the `docs` directory.

In addition, IoT Hub provides a dedicated AT instruction set. If the communication module implements this instruction set, it will be easier for devices to connect and communicate, and less code will be required. For this scenario, please refer to the [SDK for MCU AT](https://github.com/tencentyun/qcloud-iot-sdk-tencent-at-based.git) dedicated to the Tencent Cloud customized AT module.

## 2. SDK Directory Structure Overview

#### 1. Directory structure and top-level documents

| Name               | Description                                                         |
| ------------------ | ------------------------------------------------------------ |
| CMakeLists.txt     | CMake compilation and description file                                            |
| CMakeSettings.json | CMake configuration file on Visual Studio                               |
| cmake_build.sh     | Compilation script with CMake on Linux                                   |
| make.settings      | Configuration file compiled directly by Makefile on Linux                        |
| Makefile           | Direct compilation with Makefile on Linux                                  |
| device_info.json   | Device information file. If `DEBUG_DEV_INFO_USED` = `OFF`, the device information will be parsed from this file |
| docs               | Documentation directory, i.e., the use instructions of the SDKs for different platforms                      |
| external_libs      | Third-party package components, such as Mbed TLS                                  |
| samples            | Application demos                                                     |
| include            | External header files provided to users                                   |
| platform           | Platform source code files. Currently, implementations are provided for different OS (Linux/Windows/FreeRTOS/nonOS), TLS (Mbed TLS), and AT module |
| sdk_src            | Core communication protocols and service code of the SDK                                    |
| tools              | Compilation and code generation script tools supporting the SDK                              |

## 3. SDK Compilation Method Description

The C-SDK supports three compilation methods:

- CMake
- Makefile
- Code extraction

For more information on the compilation methods and compilation configuration options, please see **C-SDK_Build Compilation Environment and Configuration Description** in the `docs` directory.

## 4. SDK Demos

The `samples` directory of the C-SDK contains demos showing how to use the features. For more information on how to run the demos, please see the documents in the `docs` directory.

## 5. Notes

#### Multi-device connection
Starting from v3.2.0, the SDK supports multiple devices as clients to connect to the IoT Hub backend at the same time and optimizes and enhances the use of APIs in a multithreaded environment. For more information, please see **C-SDK_MTMC Multithreaded Multi-Device Support** in the `docs` directory.
This version encapsulates the status variables and parameters in an object-oriented way. Its impact on external APIs is that the initialization process of certificate-authenticated devices is changed, but there are no changes in other APIs. You only need to refer to the relevant demos to modify the `_setup_connect_init_params` function.

#### API changes for OTA update
Starting from SDK v3.0.3, OTA update supports checkpoint restart. When the firmware download process is interrupted due to network exceptions or other issues, the downloaded part of the firmware can be saved, so that the download can start from where interrupted instead of from the beginning when it is resumed.
After this new feature was supported, the methods of using relevant OTA APIs changed. If you have upgraded from v3.0.2 or below, you should modify your logic code; otherwise, firmware download will fail. For more information on how to modify it, please see **`samples/ota/ota_mqtt_sample.c`**.

#### Code name changes
To improve the code readability and comply with the naming conventions, SDK v3.1.0 incorporated changes to certain variables, functions, and macro names. If you have upgraded from v3.0.3 or below, you can run the `tools/update_from_old_SDK.sh` script on Linux to replace the names in your own code, and then you can use the new version of the SDK directly.

| Old Name              | New Name                                                         |
| ------------------ | ------------------------------------------------------------ |
| QCLOUD_ERR_SUCCESS     | QCLOUD_RET_SUCCESS                                            |
| QCLOUD_ERR_MQTT_RECONNECTED | QCLOUD_RET_MQTT_RECONNECTED                               |
| QCLOUD_ERR_MQTT_MANUALLY_DISCONNECTED     | QCLOUD_RET_MQTT_MANUALLY_DISCONNECTED                                   |
| QCLOUD_ERR_MQTT_CONNACK_CONNECTION_ACCEPTED      | QCLOUD_RET_MQTT_CONNACK_CONNECTION_ACCEPTED                        |
| QCLOUD_ERR_MQTT_ALREADY_CONNECTED           | QCLOUD_RET_MQTT_ALREADY_CONNECTED                                  |
| MAX_SIZE_OF_DEVICE_SERC   | MAX_SIZE_OF_DEVICE_SECRET |
| devCertFileName               | dev_cert_file_name                      |
| devPrivateKeyFileName      | dev_key_file_name                                  |
| devSerc            | device_secret                                                     |
| MAX_SIZE_OF_PRODUCT_KEY            | MAX_SIZE_OF_PRODUCT_SECRET                                   |
| product_key           | product_secret |
| DEBUG            | eLOG_DEBUG                                    |
| INFO              | eLOG_INFO                              |
| WARN            | eLOG_WARN                                                     |
| ERROR            | eLOG_ERROR                                   |
| DISABLE           | eLOG_DISABLE |
| Log_writter            | IOT_Log_Gen                                    |
| qcloud_iot_dyn_reg_dev              | IOT_DynReg_Device                              |
| IOT_SYSTEM_GET_TIME              | IOT_Get_SysTime                              |

```
#! /bin/bash

sed -i "s/QCLOUD_ERR_SUCCESS/QCLOUD_RET_SUCCESS/g" `grep -rwl QCLOUD_ERR_SUCCESS ./*`
sed -i "s/QCLOUD_ERR_MQTT_RECONNECTED/QCLOUD_RET_MQTT_RECONNECTED/g" `grep -rwl QCLOUD_ERR_MQTT_RECONNECTED ./*`
sed -i "s/QCLOUD_ERR_MQTT_MANUALLY_DISCONNECTED/QCLOUD_RET_MQTT_MANUALLY_DISCONNECTED/g" `grep -rwl QCLOUD_ERR_MQTT_MANUALLY_DISCONNECTED ./*`
sed -i "s/QCLOUD_ERR_MQTT_CONNACK_CONNECTION_ACCEPTED/QCLOUD_RET_MQTT_CONNACK_CONNECTION_ACCEPTED/g" `grep -rwl QCLOUD_ERR_MQTT_CONNACK_CONNECTION_ACCEPTED ./*`
sed -i "s/QCLOUD_ERR_MQTT_ALREADY_CONNECTED/QCLOUD_RET_MQTT_ALREADY_CONNECTED/g" `grep -rwl QCLOUD_ERR_MQTT_ALREADY_CONNECTED ./*` 
sed -i "s/MAX_SIZE_OF_DEVICE_SERC/MAX_SIZE_OF_DEVICE_SECRET/g" `grep -rwl MAX_SIZE_OF_DEVICE_SERC ./*`
sed -i "s/devCertFileName/dev_cert_file_name/g" `grep -rwl devCertFileName ./*`
sed -i "s/devPrivateKeyFileName/dev_key_file_name/g" `grep -rwl devPrivateKeyFileName ./*`
sed -i "s/devSerc/device_secret/g" `grep -rwl devSerc ./*`
sed -i "s/MAX_SIZE_OF_PRODUCT_KEY/MAX_SIZE_OF_PRODUCT_SECRET/g" `grep -rwl MAX_SIZE_OF_PRODUCT_KEY ./*`
sed -i "s/product_key/product_secret/g" `grep -rwl product_key ./*` 
sed -i "s/DEBUG/eLOG_DEBUG/g" `grep -rwl DEBUG ./*`
sed -i "s/INFO/eLOG_INFO/g" `grep -rwl INFO ./*`
sed -i "s/WARN/eLOG_WARN/g" `grep -rwl WARN ./*`
sed -i "s/ERROR/eLOG_ERROR/g" `grep -rwl ERROR ./*`
sed -i "s/DISABLE/eLOG_DISABLE/g" `grep -rwl DISABLE ./*`
sed -i "s/Log_writter/IOT_Log_Gen/g" `grep -rwl Log_writter ./*` 
sed -i "s/qcloud_iot_dyn_reg_dev/IOT_DynReg_Device/g" `grep -rwl qcloud_iot_dyn_reg_dev ./*`
sed -i "s/IOT_SYSTEM_GET_TIME/IOT_Get_SysTime/g" `grep -rwl IOT_SYSTEM_GET_TIME ./*`
```
