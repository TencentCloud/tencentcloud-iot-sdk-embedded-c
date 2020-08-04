# 设备日志上报功能
## 功能简介
设备端日志上报功能，可将设备端的Log通过HTTP上报到云端，并可在控制台展示，方便用户远程调试、诊断及监控设备运行状况。目前该功能仅支持MQTT模式。
只要将SDK的编译宏FEATURE_LOG_UPLOAD_ENABLED置为y（默认为y），并在控制台设置上报级别，则在代码中调用Log_e/w/i/d接口的日志除了会在终端打印出来，还会上报云端并在控制台展示，如下图。
![](https://main.qcloudimg.com/raw/cae7f9e7cf1e354cfc1e3578eb6746bc.png)

## 日志级别
上报级别设置可参见下图，Level级别越大则上报的日志越多，比如Level3(信息)会将ERROR/WARN/INFO级别的日志都上报而DEBUG级别则不上报。控制台默认为关闭状态，则表示设备端仅在MQTT连接失败的时候才会上报ERROR级别日志。
![](https://main.qcloudimg.com/raw/826b648993a267b1cc2f082148d8d073.png)

## 功能使用
代码具体用例可以参考mqtt_sample以及qcloud_iot_export_log.h注释说明，用户除了打开编译宏开关，还需要调用IOT_Log_Init_Uploader函数进行初始化。SDK在IOT_MQTT_Yield函数中会定时进行上报，此外，用户可根据自身需要，在程序出错退出的时候调用IOT_Log_Upload(true)强制上报。同时SDK提供在HTTP通讯出错无法上报日志时的缓存和恢复正常后重新上报机制，但需要用户根据设备具体情况提供相关回调函数，如不提供回调或回调函数提供不全则该缓存机制不生效，HTTP通讯失败时日志会被丢掉。

打开日志上报功能，请确保编译配置文件CMakeLists.txt中使能下面选项
```
set(FEATURE_LOG_UPLOAD_ENABLED ON)
```
