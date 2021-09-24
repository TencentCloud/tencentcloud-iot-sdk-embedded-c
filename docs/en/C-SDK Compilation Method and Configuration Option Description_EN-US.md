# C-SDK Compilation Method and Configuration Option Description

This document describes the compilation methods and compilation configuration options of the C-SDK, as well as the compilation environment setup and compilation samples in the Linux and Windows development environments.

## C-SDK Compilation Method Description

The C-SDK supports three compilation methods:

#### 1. CMake

We recommend you use CMake, a cross-platform compilation tool, for compilation in the Linux and Windows development environments.
Compilation with CMake uses `CMakeLists.txt` as the input file for compilation configuration options.

#### 2. Makefile

In environments where CMake is not supported, you can use Makefile for compilation. As for SDK v3.0.3 or below, this method uses `make.settings` as the input file for compilation configuration options, and you only need to run `make` after the modification.

#### 3. Code extraction

This method allows you to select features based on your needs and extract the relevant code into a separate folder. The code hierarchy in the folder is concise, making it easy for you to copy and integrate it into your own development environment.
This method relies on CMake. Configure relevant feature modules in `CMakeLists.txt`, set `EXTRACT_SRC` to `ON`, and run the following command on Linux:
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

## C-SDK Compilation Option Description

#### 1. Compilation configuration options
Most of the following configuration options apply to CMake and `make.setting`. The `ON` value in CMake corresponds to `y` in `make.setting`, and `OFF` to `n`.

| Name                             | CMake Value            | Description                                                         |
| :------------------------------- | ------------- | ------------------------------------------------------------ |
| BUILD_TYPE                       | release/debug | release: disable the `IOT_DEBUG` information (the compilation is output to the `release` directory) <br />debug: enable the `IOT_DEBUG` information (the compilation is output to the `debug` directory) |
| EXTRACT_SRC                      | ON/OFF        | Whether to enable code extraction, which takes effect only for CMake                                             |
| COMPILE_TOOLS                    | gcc           | GCC and MSVC are supported. It can also be a cross compiler, such as `arm-none-linux-gnueabi-gcc` |
| PLATFORM                         | linux         | Includes linux/windows/freertos/nonos                             |
| FEATURE_MQTT_COMM_ENABLED        | ON/OFF        | Whether to enable the MQTT channel                                               |
| FEATURE_MQTT_DEVICE_SHADOW       | ON/OFF        | Whether to enable device shadow                                               |
| FEATURE_COAP_COMM_ENABLED        | ON/OFF        | Whether to enable the CoAP channel                                               |
| FEATURE_GATEWAY_ENABLED          | ON/OFF        | Whether to enable the gateway feature                                               |
| FEATURE_OTA_COMM_ENABLED         | ON/OFF        | Whether to enable OTA firmware update                                            |
| FEATURE_OTA_SIGNAL_CHANNEL       | MQTT/CoAP     | OTA signaling channel type                                              |
| FEATURE_AUTH_MODE                | KEY/CERT      | Connection authentication method                                                 |
| FEATURE_AUTH_WITH_NOTLS          | ON/OFF        | OFF: TLS enabled; ON: TLS disabled    |
| FEATURE_MULTITHREAD_ENABLED      | ON/OFF        | Whether to enable the SDK support for multithreaded environment                                |
| FEATURE_DEV_DYN_REG_ENABLED      | ON/OFF        | Whether to enable dynamic device registration                                             |
| FEATURE_LOG_UPLOAD_ENABLED       | ON/OFF        | Whether to enable log reporting                                                 |
| FEATURE_DEBUG_DEV_INFO_USED      | ON/OFF        | Whether to enable device information acquisition source                                         |
| FEATURE_SYSTEM_COMM_ENABLED      | ON/OFF        | Whether to enable backend time acquisition                                             |
| FEATURE_OTA_USE_HTTPS            | ON/OFF        | Whether to use HTTPS for firmware download                                       |
| FEATURE_AT_TCP_ENABLED           | ON/OFF        | Whether to enable the TCP feature in the AT module                                            |
| FEATURE_AT_UART_RECV_IRQ         | ON/OFF        | Whether to enable the receipt interruption feature in the AT module                                       |
| FEATURE_AT_OS_USED               | ON/OFF        | Whether to enable the multithreaded feature in the AT module                                         |
| FEATURE_AT_DEBUG                 | ON/OFF        | Whether to enable the debugging feature in the AT module                                           |


There is a dependency relationship between the configuration options. A configuration option is valid only when the value of its dependent option is valid as shown below:

| Name                             | Dependent Option                                                | Valid Value       |
| :------------------------------- | ------------------------------------------------------- | ------------ |
| FEATURE_MQTT_DEVICE_SHADOW       | FEATURE_MQTT_COMM_ENABLED                               | ON           |
| FEATURE_GATEWAY_ENABLED          | FEATURE_MQTT_COMM_ENABLED                               | ON             |
| FEATURE_OTA_SIGNAL_CHANNEL(MQTT) | FEATURE_OTA_COMM_ENABLED<br />FEATURE_MQTT_COMM_ENABLED | ON<br />ON   |
| FEATURE_OTA_SIGNAL_CHANNEL(COAP) | FEATURE_OTA_COMM_ENABLED<br />FEATURE_COAP_COMM_ENABLED | ON<br />ON   |
| FEATURE_AUTH_WITH_NOTLS          | FEATURE_AUTH_MODE                                       | KEY          |
| FEATURE_AT_UART_RECV_IRQ         | FEATURE_AT_TCP_ENABLED                                  | ON           |
| FEATURE_AT_OS_USED               | FEATURE_AT_TCP_ENABLED                                  | ON           |
| FEATURE_AT_DEBUG                 | FEATURE_AT_TCP_ENABLED                                  | ON           |

#### 2. Device information options

After a device is created in the IoT Hub console, you need to configure its information (`ProductID/DeviceName/DeviceSecret/Cert/Key` file) in the SDK first before it can run properly. In the development phase, the SDK provides two methods of storing the device information:
1. If the device information is stored in the code (compilation option `DEBUG_DEV_INFO_USED` = `ON`), you should modify the device information in `platform/os/xxx/HAL_Device_xxx.c`. This method can be used on platforms without a file system.
2. If the device information is stored in the configuration file (compilation option `DEBUG_DEV_INFO_USED` = `OFF`), you should modify the device information in the `device_info.json` file with no need to recompile the SDK. This method is recommended for development on Linux and Windows.


## C-SDK Compilation Environments

### Linux (Ubuntu)

The Ubuntu version used in this document is v14.04 or v16.04.

1. Install the necessary software.
```
$ sudo apt-get install -y build-essential make git gcc cmake
```
  The SDK requires CMake v3.5 or above. The CMake version installed by default is low. For more information on how to download and install the specific version of CMake, please see:
  Download: https://cmake.org/download/
  Installation: https://gitlab.kitware.com/cmake/cmake


2. Modify the configuration.
Modify `CMakeLists.txt` to ensure that the following options exist (with a key-authenticated device as example).
```
set(BUILD_TYPE                   "release")
set(COMPILE_TOOLS                "gcc") 
set(PLATFORM 	                "linux")
set(FEATURE_MQTT_COMM_ENABLED ON)
set(FEATURE_AUTH_MODE "KEY")
set(FEATURE_AUTH_WITH_NOTLS OFF)
set(FEATURE_DEBUG_DEV_INFO_USED  OFF)  
```

3. Run the following script for compilation.
Below is a complete compilation library and demo:
```
./cmake_build.sh 
```
The output library files, header files, and samples are in the `output/release` folder.
After the complete compilation, if you only need to compile the demo, then run:
```
./cmake_build.sh samples
```

4. Enter the device information.
Enter the information of the device created above on IoT Hub in `device_info.json` (with a **key-authenticated device** as example).
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

5. Run the demo.
The demo output is in the `output/release/bin` folder. For example, to run the `mqtt_sample` demo, enter `./output/release/bin/mqtt_sample`.


### Windows

#### Getting and installing Visual Studio 2019

Download [Visual Studio 2019](https://visualstudio.microsoft.com/zh-hans/downloads/) and install it. In this document, the downloaded and installed version is v16.2 Community.

![](https://main.qcloudimg.com/raw/394078661873f4464e7c2bf2836aa6a2.png)

Then, select **Desktop development with C++** and **C++ CMake tools for Windows**.

![](https://main.qcloudimg.com/raw/2c5ea21e32ed3a6375d41370c6b521d9.png)

#### Compilation and running

1. Run Visual Studio, select **Open a local folder**, and select the downloaded C-SDK directory.

   ![](https://main.qcloudimg.com/raw/0d954b9cd18c54249f065e0f67aa0b69.png)

2. [Modify the user information]().

3. Double-Click `CMakeLists.txt` in the root directory and make sure that the platform is set to **Windows** and the compilation tool is set to **MSVC** in the compilation toolchain (for more information on other configuration options, please see [CMake Compilation Configuration and Code Extraction]()).

   ![](https://main.qcloudimg.com/raw/d4ef02feb3304c99dbfb02fe37996870.png)

   ```cmake
   # Compilation toolchain
   #set(COMPILE_TOOLS "gcc") 
   #set(PLATFORM 	  "linux")
    
   set(COMPILE_TOOLS "MSVC") 
   set(PLATFORM 	  "windows") 
   ```

4. Visual Studio will automatically build the CMake cache. Just wait for the build to complete.

   ![](https://main.qcloudimg.com/raw/a2c776d7843654b22d41bd6da742d4f7.png)

5. After the cache is built, select **Build** > **Build All**.

   ![](https://main.qcloudimg.com/raw/df8e3e77c0abb83628ef4fa03a0a52c0.png)

6. Select the corresponding demo for running, which should correspond to the user information.

   ![](https://main.qcloudimg.com/raw/5a3958703777c2de30763411625dab06.png)
