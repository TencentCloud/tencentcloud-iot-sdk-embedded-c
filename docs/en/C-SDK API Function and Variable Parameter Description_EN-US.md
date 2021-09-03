# C-SDK API Function and Variable Parameter Description

The header files of the device C-SDK provided for you to call such as API function declarations, constants, and variable parameter definitions are in the `include` directory. This document describes the variable parameters and API functions in the directory.

## Variable Parameter Configuration

You can configure corresponding parameters in the C-SDK based on the needs in specific scenarios to ensure the smooth operations of your businesses. Variable connection parameters include:
1. Timeout period of blocking MQTT calls (including connection, subscribing, and publishing) in milliseconds. 5,000 milliseconds is recommended.
2. Size of the buffer for message receiving/sending over the MQTT protocol, which is 2,048 bytes by default and currently can be up to 16 KB per MQTT message in the cloud.
3. Size of the buffer for message sending and receiving over the CoAP protocol, which is 512 bytes by default and currently can be up to 1 KB per CoAP message in the cloud.
4. MQTT heartbeat message sending interval in milliseconds, which can be up to 690s.
5. Maximum waiting time for reconnection in milliseconds. When a device is reconnected after disconnection, the waiting time will double if reconnection fails, and reconnection will stop when the maximum waiting time is exceeded.

You can modify the configuration of the corresponding connection parameters by modifying the following macro definitions in the `include/qcloud_iot_export_variables.h` file.
You need to recompile the SDK after modification. 
```
/* default MQTT/CoAP timeout value when connect/pub/sub (unit: ms) */
#define QCLOUD_IOT_MQTT_COMMAND_TIMEOUT                             (5 * 1000)

/* default MQTT keep alive interval (unit: ms) */
#define QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL                         (240 * 1000)

/* default MQTT Tx buffer size, MAX: 16*1024 */
#define QCLOUD_IOT_MQTT_TX_BUF_LEN                                  (2048)

/* default MQTT Rx buffer size, MAX: 16*1024 */
#define QCLOUD_IOT_MQTT_RX_BUF_LEN                                  (2048)

/* default COAP Tx buffer size, MAX: 1*1024 */
#define COAP_SENDMSG_MAX_BUFLEN                                     (512)

/* default COAP Rx buffer size, MAX: 1*1024 */
#define COAP_RECVMSG_MAX_BUFLEN                                     (512)

/* MAX MQTT reconnect interval (unit: ms) */
#define MAX_RECONNECT_WAIT_INTERVAL                                 (60 * 1000)
```

## API Function Description
The following describes the main features and corresponding APIs provided by the C-SDK v3.1.0 for you to write business logic. For more information on API parameters and returned values, please see the comments in the header files of the SDK code such as `include/exports/qcloud_iot_export_*.h`.
For the sample code for API usage, please see the corresponding code in the `samples` directory and the documentation in the `docs/IoT_Hub` directory.

### 1. MQTT APIs
For more information on how to use MQTT APIs, please see the `mqtt_sample_Getting Started` document in the `docs/IoT_Hub` directory of the SDK.

| No.  | Function                        | Description                                              |
| ---- | -------------------- | ----------------------------------------------- |
| 1    | IOT_MQTT_Construct   | Constructs `MQTTClient` and connects to MQTT cloud service                |
| 2    | IOT_MQTT_Destroy     | Closes MQTT connection and terminates `MQTTClient`                 |
| 3    | IOT_MQTT_Yield       | Performs tasks such as reading MQTT messages, processing messages, timing out requests, and managing heartbeat packets and reconnection status in the current thread context |
| 4    | IOT_MQTT_Publish     | Publishes MQTT message                                  |
| 5    | IOT_MQTT_Subscribe   | Subscribes to MQTT topic                                  |
| 6    | IOT_MQTT_Unsubscribe | Unsubscribes from subscribed MQTT topic                      |
| 7    | IOT_MQTT_IsConnected | Views whether MQTT is currently connected to                        |
| 8    | IOT_MQTT_IsSubReady  | Queries whether the corresponding topic has been subscribed to successfully              |
| 9    | IOT_MQTT_GetDeviceInfo  | Gets the corresponding device information of `MQTTclient`             |
| 10    | IOT_MQTT_StartLoop  | Starts the backend `Yield` thread of `MQTTclient` in multithreaded environment       |
| 11    | IOT_MQTT_StopLoop  | Stops the backend `Yield` thread of `MQTTclient` in multithreaded environment             |

- API use instructions
```
When you construct MQTT, in addition to the device information, you also need to provide a callback function to receive event notifications such as connection status notification, whether a topic has been subscribed to successfully, and whether a QoS 1 message has been published successfully. When you subscribe to a topic, you need to provide another callback function to receive the messages delivered by it. For more information on how to use the APIs, please see the `mqtt_sample_Getting Started` document in the `docs/IoT_Hub` directory.
```

### 2. Device shadow APIs
For more information on the device shadow feature, please see [Details](https://cloud.tencent.com/document/product/634/11918).
You can also refer to the device shadow document in the `docs/IoT_Hub` directory of the SDK.

| No.  | Function                        | Description                                              |
| ---- | -------------------------------------------------- | ---------------------------------------------- |
| 1    | IOT_Shadow_Construct                               | Constructs device shadow client `ShadowClient` and connects to MQTT cloud service         |
| 2    | IOT_Shadow_Publish                                 | The shadow client publishes an MQTT message                                  |
| 3    | IOT_Shadow_Subscribe                               | The shadow client subscribes to an MQTT topic                                  |
| 4    | IOT_Shadow_Unsubscribe                             | The shadow client unsubscribes from a subscribed MQTT topic                         |
| 5    | IOT_Shadow_IsConnected                             | Views whether MQTT of the shadow client is currently connected to                       |
| 6    | IOT_Shadow_Destroy                                 | Closes shadow MQTT connection and terminates `ShadowClient`              |
| 7    | IOT_Shadow_Yield                                   | Performs tasks such as reading MQTT messages, processing messages, timing out requests, and managing heartbeat packets and reconnection status in the current thread context |
| 8    | IOT_Shadow_Update                                  | Updates device shadow document asynchronously                              |
| 9    | IOT_Shadow_Update_Sync                             | Updates device shadow document synchronously                           |
| 10   | IOT_Shadow_Get                                     | Gets device shadow document asynchronously                           |
| 11   | IOT_Shadow_Get_Sync                                | Gets device shadow document synchronously                           |
| 12   | IOT_Shadow_Register_Property                       | Registers the attribute of the current device                             |
| 13   | IOT_Shadow_UnRegister_Property                     | Deletes registered device attribute                           |
| 14   | IOT_Shadow_JSON_ConstructReport                    | Adds `reported` field to JSON document for update in a non-overwriting manner            |
| 15   | IOT_Shadow_JSON_Construct_OverwriteReport          | Adds `reported` field to JSON document for update in an overwriting manner              |
| 16   | IOT_Shadow_JSON_ConstructReportAndDesireAllNull    | Adds `reported` field to JSON document and empties `desired` field   |
| 17   | IOT_Shadow_JSON_ConstructDesireAllNull             | Adds `"desired": null` field to JSON document             |
| 18   | IOT_Shadow_Get_Mqtt_Client                         | Gets the corresponding `MQTTclient` of `ShadowClient`             |

### CoAP APIs
For more information on the CoAP feature, please see the CoAP communication document in the `docs/IoT_Hub` directory of the SDK.

| No.  | Function                        | Description                                              |
| ---- | ---------------------------- | ------------------------------------------------- |
| 1    | IOT_COAP_Construct           | Constructs `CoAPClient` and completes CoAP connection                   |
| 2    | IOT_COAP_Destroy             | Closes CoAP connection and terminates `CoAPClient`                    |
| 3    | IOT_COAP_Yield               | Performs tasks such as reading CoAP messages and processing messages in the current thread context      |
| 4    | IOT_COAP_SendMessage         | Publishes CoAP message                                    |
| 5    | IOT_COAP_GetMessageId        | Gets `msgId` in `CoAP Response` message                     |
| 6    | IOT_COAP_GetMessagePayload   | Gets the content of `CoAP Response` message                        |
| 7    | IOT_COAP_GetMessageCode      | Gets the error code of `CoAP Response` message                      |
| 9    | IOT_COAP_GetDeviceInfo       | Gets the corresponding device information of `COAPclient`             |

### OTA APIs
For more information on the OTA firmware download feature, please see [Device Firmware Update](https://cloud.tencent.com/document/product/634/14674).
You can also refer to the firmware update document in the `docs/IoT_Hub` directory of the SDK.

| No.  | Function                        | Description                                              |
| ---- | ---------------------------- | ------------------------------------------------- |
| 1    | IOT_OTA_Init                 | Initializes OTA module. The client needs to initialize MQTT/CoAP before calling this API     |
| 2    | IOT_OTA_Destroy              | Releases resources related to OTA module                              |
| 3    | IOT_OTA_ReportVersion        | Reports local firmware version information to OTA server                         |
| 4    | IOT_OTA_IsFetching           | Checks whether the firmware is being downloaded                            |
| 5    | IOT_OTA_IsFetchFinish        | Checks whether the firmware has been downloaded                              |
| 6    | IOT_OTA_FetchYield           | Gets firmware from remote server with specific timeout value                    |
| 7    | IOT_OTA_Ioctl                | Gets specified OTA information                                 |
| 8    | IOT_OTA_GetLastError         | Gets the last error code                                 |
| 9    | IOT_OTA_StartDownload        | Establishes HTTP connection with firmware server according to obtained firmware update address and local firmware information offset (whether to support checkpoint restart)     |
| 10   | IOT_OTA_UpdateClientMd5      | Calculates the MD5 of local firmware before checkpoint restart                              |
| 11   | IOT_OTA_ReportUpgradeBegin   | Reports the status of impending update to server before firmware update   |
| 12   | IOT_OTA_ReportUpgradeSuccess | Reports the status of update success to server after successful firmware update                         |
| 13   | IOT_OTA_ReportUpgradeFail    | Reports the status of update failure to server after failed firmware update        |

### Log APIs
For more information on the device log reporting feature, please see the device log reporting document in the `docs/IoT_Hub` directory of the SDK.

| No.  | Function                        | Description                                              |
| ---- | -------------------------- | --------------------------------------------- |
| 1    | IOT_Log_Set_Level          | Sets the printout level of SDK logs                            |
| 2    | IOT_Log_Get_Level          | Returns the printout level of SDK logs                            |
| 3    | IOT_Log_Set_MessageHandler | Sets log callback function to redirect SDK logs to another output method  |
| 4    | IOT_Log_Init_Uploader      | Enables SDK log reporting to the cloud and initializes resources                |
| 5    | IOT_Log_Fini_Uploader      | Disables SDK log reporting to the cloud and releases resources                      |
| 6    | IOT_Log_Upload             | Reports SDK execution logs to the cloud                         |
| 7    | IOT_Log_Set_Upload_Level   | Sets the reporting level of SDK logs                            |
| 8    | IOT_Log_Get_Upload_Level   | Returns the reporting level of SDK logs                            |
| 9    | Log_d/i/w/e                | Prints SDK logs by level                         |

### System time APIs

| No.  | Function                        | Description                                              |
| ---- | ---------------------------- | ------------------------------------------------- |
| 1    | IOT_Get_SysTime              | Gets IoT Hub's backend time. Currently, the time sync feature is supported only for the MQTT channel |
| 2    | IOT_Sync_NTPTime             | Syncs IoT Hub's backend timestamp and sets device system time through NTP algorithm. Currently, the time sync feature is supported only for the MQTT channel |

### Gateway feature APIs
For more information on the gateway feature, please see the gateway feature document in the `docs/IoT_Hub` directory of the SDK.

| No.  | Function                        | Description                                              |
| ---- | ---------------------------------- | ----------------------------------------------------|
| 1    | IOT_Gateway_Construct        | Constructs gateway client and completes MQTT connection                   |
| 2    | IOT_Gateway_Destroy          | Closes MQTT connection and terminates gateway client                    |
| 3    | IOT_Gateway_Subdev_Bind            | Binds subdevice to gateway                                      |
| 4    | IOT_Gateway_Subdev_Unbind          | Unbinds subdevice from gateway                                      |
| 5    | IOT_Gateway_Subdev_Online          | Proxies subdevice connection                                      |
| 6    | IOT_Gateway_Subdev_Offline         | Proxies subdevice disconnection                                      |
| 7    | IOT_Gateway_Yield            | Performs tasks such as reading MQTT messages, processing messages, timing out requests, and managing heartbeat packets and reconnection status in the current thread context |
| 8   | IOT_Gateway_Publish          | Publishes MQTT message                                  |
| 9    | IOT_Gateway_Subscribe        | Subscribes to MQTT topic                                  |
| 10   | IOT_Gateway_Unsubscribe      | Unsubscribes from subscribed MQTT topic                      |
| 11   | IOT_Gateway_Get_Mqtt_Client        | Gets the corresponding `MQTTclient` of `GatewayClient`             |
| 12   | IOT_Gateway_Subdev_GetBindList     | Gets the list of subdevices on IoT Hub bound to gateway              |
| 13   | IOT_Gateway_Subdev_DestoryBindList | Terminates the obtained list data of bound subdevices                |

### Dynamic registration APIs
For more information on the dynamic registration feature, please see the dynamic registration document in the `docs/IoT_Hub` directory of the SDK.

| No.  | Function                        | Description                                              |
| ---- | ---------------------------- | ------------------------------------------------- |
| 1    | IOT_DynReg_Device            | Dynamically registers device with backend and gets device private key or certificate |

### Broadcast communication APIs
For more information on the broadcast communication feature, please see the broadcast communication document in the `docs/IoT_Hub` directory of the SDK.

| No.  | Function                        | Description                                              |
| ---- | ---------------------------- | ------------------------------------------------- |
| 1    | IOT_Broadcast_Subscribe      | Subscribes to the topic that receives broadcast messages and provides the message callback processing function |

### Sync communication APIs
For more information on the RRPC sync communication feature, please see the sync communication document in the `docs/IoT_Hub` directory of the SDK.

| No.  | Function                        | Description                                              |
| ---- | ---------------------------- | ------------------------------------------------- |
| 1    | IOT_RRPC_Init            | Subscribes to the topic that receives RRPC messages and provides the message callback processing function |
| 2    | IOT_RRPC_Reply            | Replies to RRPC message |

### Remote configuration APIs
For more information on the remote configuration feature, please see the `remote_config_mqtt_sample_Remote Configuration` document in the `docs/IoT_Hub` directory of the SDK.

| No.  | Function                        | Description                                              |
| ---- | ---------------------------- | -------------------------- |
| 1    | IOT_Subscribe_Config         | Subscribes to remote configuration topic from IoT Hub |
| 2    | IOT_Get_Config               | Gets device configuration from IoT Hub       |
