## C-SDK Cross-Platform Porting

This document describes how to port the device C-SDK to the target hardware platform. C-SDK adopts modular design to separate the core protocol service and hardware abstraction layer (HAL). When porting across platforms, you generally only need to modify and adapt the HAL.

## C-SDK Architecture

#### Architecture diagram

![framework](https://main.qcloudimg.com/raw/d273e85d282bed8dfbd6302bbec31dad.png)

#### Architecture description

The SDK is designed into four layers from top to bottom: platform service layer, core protocol layer, network layer, and hardware abstraction layer.

- Service layer
This layer is above the network protocol layer. It implements features such as device connection authentication, device shadow, data template, gateway, dynamic registration, log reporting, and OTA and provides relevant APIs. For more information on the service layer APIs, please see [C-SDK_API and Variable Parameter Description]().

- Protocol layer
The network protocols over which devices can interact with the IoT Hub platform include MQTT, CoAP, and HTTP.

- Network layer
This layer implements network protocol stacks based on TLS/SSL (TLS/DTLS), POSIX_socket (TCP/UDP), and AT_socket. Different services can use different protocol stack API functions as needed.

- Hardware abstraction layer
To implement the abstract encapsulation of underlying operations of different hardware platforms, it is necessary to conduct porting for the specific software and hardware platforms, which is divided into two parts of required and optional HAL APIs.

## HAL Porting

HAL mainly has several major parts for porting, including those related to the OS, network and TLS, time and print, and device information.
In the **platform/os** directory, the SDK demonstrates the implementation of HAL in four scenarios: Linux, Windows, FreeRTOS, and nonOS. You can refer to the corresponding directory to port for the target platform.

#### OS APIs
The following functions related to thread/mutex lock/semaphore need to be ported only when multithreading/multitasking (MULTITHREAD_ENABLED) is enabled.

| No. | Function | Description |
| ---- | ---------------------- | ------------------------------------------ |
| 1 | HAL_Malloc | Dynamically applies for memory block |
| 2 | HAL_Free | Releases memory block |
| 3 | HAL_ThreadCreate | Creates thread |
| 4 | HAL_MutexCreate | Creates mutex lock |
| 5 | HAL_MutexDestroy | Terminates mutex lock |
| 6 | HAL_MutexLock | Locks mutex |
| 7 | HAL_MutexUnlock | Unlocks mutex |
| 8 | HAL_SemaphoreCreate | Creates semaphore |
| 9 | HAL_SemaphoreDestroy | Terminates semaphore |
| 10 | HAL_SemaphoreWait | Waits for semaphore |
| 11 | HAL_SemaphorePost | Releases semaphore |
| 12 | HAL_SleepMs | Sleeps |

#### Network and TLS HAL APIs

Network APIs provide either-or adaptation and porting. For devices that have network communication capabilities and integrate TCP/IP network protocol stacks, you need to implement the `POSIX_socket` network HAL APIs. For devices using TLS/SSL for encrypted communication, you also need to implement the TLS HAL APIs. For devices with [MCU + universal TCP_AT module](), you can choose the `AT_Socket` framework provided by the SDK and implement relevant AT module APIs.

##### HAL APIs based on POSIX_socket

Among them, TCP/UDP APIs are implemented based on POSIX socket functions. TLS APIs are dependent on the **mbedtls** library. Before porting, you must ensure that the **mbedtls** library is available on the system. If you use other TLS/SSL libraries, please refer to the relevant implementation of **`platform/tls/mbedtls`** for porting and adapting.
UDP/DTLS functions need to be ported only when **CoAP** communication is enabled.

| No. | Function | Description |
| ---- | ---------------------- | ------------------------------------------ |
| 1 | HAL_TCP_Connect | Establishes TCP connection |
| 2 | HAL_TCP_Disconnect | Closes TCP connection |
| 3 | HAL_TCP_Write | Writes data to TCP connection |
| 4 | HAL_TCP_Read | Reads data from TCP connection |
| 5 | HAL_TLS_Connect | Establishes TLS connection |
| 6 | HAL_TLS_Disconnect | Closes TLS connection |
| 7 | HAL_TLS_Write | Writes data to TLS connection |
| 8 | HAL_TLS_Read | Reads data from TLS connection |
| 9 | HAL_UDP_Connect | Establishes UDP connection |
| 10 | HAL_UDP_Disconnect | Closes UDP connection |
| 11 | HAL_UDP_Write | Writes data to UDP connection |
| 12 | HAL_UDP_Read | Reads data from UDP connection |
| 13 | HAL_DTLS_Connect | Establishes DTLS connection |
| 14 | HAL_DTLS_Disconnect | Closes DTLS connection |
| 15 | HAL_DTLS_Write | Writes data to DTLS connection |
| 16 | HAL_DTLS_Read | Reads data from DTLS connection |

##### HAL APIs based on AT_socket
After `AT_socket` is selected by enabling the compilation macro **`AT_TCP_ENABLED`**, the SDK will call the `at_socket` API of `network_at_tcp.c`. You don't need to port the `at_socket` layer, but you need to implement the AT serial port driver and AT module driver. For the AT module driver, you only need to implement the driver API of the driver structure *`at_device_op_t`* in `at_device` of the AT framework. You can refer to the supported modules in the `at_device` directory. For the AT serial port driver, you need to implement serial port receipt interruption and then call the callback function *`at_client_uart_rx_isr_cb`* in the interruption service program. You can refer to `HAL_AT_UART_freertos.c` to port for the target platform.

| No. | Function | Description |
| ---- | ---------------------- | ------------------------------------------ |
| 1 | HAL_AT_Uart_Init | Initializes AT serial port |
| 2 | HAL_AT_Uart_Deinit | Deinitializes AT serial port |
| 3 | HAL_AT_Uart_Send | Sends data over AT serial port |
| 4 | HAL_AT_UART_IRQHandler | Handles AT serial port receipt interruption |


#### Time and print HAL APIs

| No. | Function | Description |
| ---- | ---------------------- | ------------------------------------------ |
| 1 | HAL_Printf | Writes formatted data to standard output stream |
| 2 | HAL_Snprintf | Writes formatted data to string |
| 3 | HAL_UptimeMs | Retrieves the number of milliseconds that has elapsed since the system started |
| 4 | HAL_DelayMs | Blocking delay in milliseconds |

#### Device information HAL APIs

To connect a device to the IoT Hub platform, you need to create product and device information on the platform and save such information in a non-volatile storage medium on the device. You can refer to `platform/os/linux/HAL_Device_linux.c` for implementation.

| No. | Function | Description |
| ---- | ---------------------- | ------------------------------------------ |
| 1 | HAL_GetDevInfo | Reads device information |
| 2 | HAL_SetDevInfo | Saves device Information |

