## Overview

For MCUs that have no network communication capabilities, the "MCU + communication module" combination is often used. Communication modules (including Wi-Fi/2G/4G/NB-IoT) generally provide serial port-based AT instruction protocols for MCUs to communicate over the network. For this scenario, the C-SDK encapsulates the AT-socket network layer, where the core protocol and service layer don't need to be ported. This document describes how to port C-SDK for connection to IoT Hub in the target environment of MCU (nonOS) + universal TCP AT module.
Compared with the RTOS scenario, the network data received by `at_socket` is processed differently. The application layer needs to periodically call **`IOT_MQTT_Yield`** to receive the server's downstream data. If the receipt window is missed, there will be data loss. Therefore, in scenarios with complex business logic, we recommended you use RTOS and select the nonOS mode by configuring `FEATURE_AT_OS_USED = OFF`.

## Porting Steps

### 1. Download the latest version of the device [C-SDK](https://github.com/tencentyun/qcloud-iot-sdk-embedded-c)

### 2. Configure the SDK features and extract the code
- Use the general TCP module to compile and configure the options for nonOS as follows:

| Name | Configuration | Description |
| :------------------------------- | ------------- | ------------------------------------------------------------ |
| BUILD_TYPE | debug/release | Set as needed |
| EXTRACT_SRC | ON | Enable code extraction |
| COMPILE_TOOLS | gcc/MSVC | Set as needed and ignore in case of IDE |
| PLATFORM | linux/windows | Set as needed and ignore in case of IDE |
| FEATURE_OTA_COMM_ENABLED | ON/OFF | Set as needed |
| FEATURE_AUTH_MODE | KEY | Key authentication is recommended for resource-constrained devices |
| FEATURE_AUTH_WITH_NOTLS | ON/OFF | Enable TLS as needed |
| FEATURE_AT_TCP_ENABLED | ON | Enable `at_socket` component |
| FEATURE_AT_UART_RECV_IRQ | ON | Enable AT serial port receipt interruption |
| FEATURE_AT_OS_USED               | OFF        | Use `at_socket` component in environment without RTOS                         |
| FEATURE_AT_DEBUG | OFF | The AT module debugging feature is disabled by default, and it needs to be enabled during debugging |

- For more information on how to extract the code, please see [C-SDK_Build Compilation Environment and Configuration Option Description]().

### 3. Port the HAL

For more information, please see [C-SDK_Cross-Platform Porting Overview]().

For network HAL APIs, the `AT_Socket` framework provided by the SDK has been selected through the above compilation options. The SDK will call the `at_socket` API of `network_at_tcp.c`. You don't need to port the `at_socket` layer, but you need to implement the AT serial port driver and AT module driver. For the AT module driver, you only need to implement the driver API of the driver structure *`at_device_op_t`* in `at_device` of the AT framework. You can refer to the supported modules in the `at_device` directory. For the AT serial port driver, you need to implement serial port receipt interruption and then call the callback function *`at_client_uart_rx_isr_cb`* in the interruption service program. You can refer to `HAL_OS_nonos.c` to port for the target platform.

### 4. Develop the business logic

You can refer to the demos in the `Docs/IoT_Hub` directory.

