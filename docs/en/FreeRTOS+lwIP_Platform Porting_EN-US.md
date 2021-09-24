# Document Description
This document describes how to port IoT Hub C-SDK to the **FreeRTOS + lwIP** platform.

## FreeRTOS Porting Overview
As a micro-kernel system, FreeRTOS mainly provides core OS mechanisms such as task creation and scheduling and inter-task communication. Different device platforms also should be equipped with different software components before they can form a complete embedded operating platform, including C runtime libraries (such as Newlib or ARM CMSIS library) and TCP/IP network protocol stacks (such as lwIP). In addition, the compilation and development environments vary by device platform, so when porting C-SDK, you need to adapt it according to the specific conditions of different devices.
The SDK provides a reference implementation based on **FreeRTOS + lwIP + Newlib** in **`platform/os/freertos`**, which has been verified and tested on Espressif's ESP8266 platform.

## Extracting Relevant Code from IoT Hub C-SDK

Because different RTOS-based platforms have different compilation methods, it is generally impossible to directly use the SDK's CMake or Make to compile. Therefore, the SDK provides the code extraction feature. It allows you to extract the relevant code into a separate folder based on your needs. The code hierarchy in the folder is concise, making it easy for you to copy and integrate it into your own development environment.

Change the platform in `CMakeLists.txt` to FreeRTOS and enable the code extraction feature:
```
set(BUILD_TYPE                  "release")
set(PLATFORM 	                "freertos")
set(EXTRACT_SRC ON)
set(FEATURE_AT_TCP_ENABLED OFF)
```
Run the following command on Linux:
```
mkdir build
cd build
cmake ..
```
You can find the relevant code files in `output/qcloud_iot_c_sdk` with the following directory hierarchy:
```
 qcloud_iot_c_sdk
 ├── include
 │   ├── config.h
 │   ├── exports
 ├── platform
 └── sdk_src
     └── internal_inc
```
The `include` directory contains the SDK APIs and variable parameters, where `config.h` is the compilation macros generated according to the compilation options. For more information, please see **C-SDK_API and Variable Parameter Description**.
The `platform` directory contains platform-related code, which can be modified and adapted according to the specific conditions of the device. For more information on functions, please see **C-SDK_Cross-Platform Porting Overview**.
The `sdk_src` directory contains the SDK core logic and protocol-related code, which generally don't need to be modified, where `internal_inc` is the header file used internally by the SDK.

You can copy `qcloud_iot_c_sdk` to the compilation and development environment of your target platform and then modify the compilation options as needed.

## Sample Porting to Espressif's ESP8266 RTOS
Build a demo project based on Espressif's ESP8266 RTOS platform in the Linux development environment. For the complete practical demo, please see [ESP8266 Launcher-Based Project](https://github.com/tencentyun/qcloud-iot-c-sdk-porting-examples/tree/master/QCloud_IoT_ESP8266_RTOS).

### 1. Get `ESP8266_RTOS_SDK` and create a project

Please refer to [ESP8266_RTOS_SDK](https://github.com/espressif/ESP8266_RTOS_SDK) to get the `RTOS_SDK` and cross compiler and create a project.

### 2. Copy the SDK code

Copy the `qcloud_iot_c_sdk` directory extracted above to `components/qcloud_iot`.

In `components/qcloud_iot`, create a compilation configuration file `component.mk` with the following content:
```
#
# Component Makefile
#

COMPONENT_ADD_INCLUDEDIRS := \
	qcloud_iot_c_sdk/include \
    qcloud_iot_c_sdk/include/exports \
	qcloud_iot_c_sdk/sdk_src/internal_inc

COMPONENT_SRCDIRS := \
	qcloud_iot_c_sdk/sdk_src \
	qcloud_iot_c_sdk/platform
    
```

At this point, you can compile `qcloud_iot_c_sdk` as a component and then call the IoT Hub C-SDK APIs in your code to connect devices and send/receive messages.
