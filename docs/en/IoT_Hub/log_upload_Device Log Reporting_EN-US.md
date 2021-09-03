# Device Log Reporting
## Overview
The device log reporting feature can report device logs to the cloud over HTTP and display them in the console for you to remotely debug, diagnose, and monitor the device status. Currently, this feature is only supported in the MQTT mode.
After you simply set the compilation macro `FEATURE_LOG_UPLOAD_ENABLED` of the SDK to `y` (which is the default value) and set the reporting level in the console, when you call the `Log_e/w/i/d` API in the code, the logs will be printed out in the terminal and also reported to the cloud for display in the console as shown below.
![](https://main.qcloudimg.com/raw/09002dd8634e37ef408aa151357bc350.png)

## Log Level
For more information on how to set the reporting level, please see the figure below. The greater the level, the more the logs that will be reported. For example, Level 3 (INFO) indicates that logs at the ERROR, WARN, and INFO levels will all be reported, while logs at the DEBUG level will not. This feature is disabled in the console by default, which indicates that devices will report only logs at the ERROR level when an MQTT connection fails.
![](https://main.qcloudimg.com/raw/78164d487b1d927424fbf7d3ed2f6a43.png)

## How to Use
For more code samples, please see the comment descriptions of `mqtt_sample` and `qcloud_iot_export_log.h`. In addition to enabling the compilation macro, you also need to call the `IOT_Log_Init_Uploader` function for initialization. The SDK will periodically report logs in the `IOT_MQTT_Yield` function. Plus, you can also call `IOT_Log_Upload(true)` to forcibly report logs as needed when the program quits unexpectedly. The SDK provides a caching mechanism when it cannot report logs due to HTTP communication errors. It will report logs again after the communication returns to normal. However, you need to provide the relevant callback functions according to the specific device conditions. If the callback functions are not provided, or the provided callback functions are incomplete, the caching mechanism will not work, and the logs will be discarded in case the HTTP communication fails.

Enable the log reporting feature and make sure that the following option is enabled in the compilation configuration file `CMakeLists.txt`.
```
set(FEATURE_LOG_UPLOAD_ENABLED ON)
```
