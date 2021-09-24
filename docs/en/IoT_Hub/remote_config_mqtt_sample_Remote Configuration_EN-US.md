# Remote Configuration

This document describes how to configure a product and device remotely over MQTT based on the **remote_config_mqtt_sample** of the SDK.

## 1. Creating Device in Console

#### 1. Background

During device OPS, it is necessary to update the device configuration parameters to meet the needs in the use environment.
For example, you may need to modify the temperature and humidity upper and low limits of a temperature and humidity monitor device, the serial port parameter and Modbus parameter information of a Modbus gateway device, or the Zigbee configuration parameter information of a Zigbee gateway.
* Configuration push and request response topic: `$config/update/${productID}/${deviceName}`
* Configuration request topic: `$config/report/${productID}/${deviceName}`

#### 2. Remote configuration flowchart

##### 2.1 IoT Hub actively delivers the configuration

![image.png](https://main.qcloudimg.com/raw/94fc3451122978994f15ea3c1d00d61b.png)

##### 2.2 A device actively requests a configuration update

![image.png](https://main.qcloudimg.com/raw/f08e00711976287d013c4fb05d40e358.png)

#### 3. Product and device creation

Create the `AirConditioner` product and device as instructed in [Device Interconnection](https://cloud.tencent.com/document/product/634/11913).
Create the `airConditioner1` device.

## 2. Compiling and Running Demo (with **Key-Authenticated Device** as Example)

#### 1. Compile the SDK

Modify `CMakeLists.txt` to ensure that the following options exist:
```
set(BUILD_TYPE                   "release")
set(COMPILE_TOOLS                "gcc")
set(PLATFORM 	                 "linux")
set(FEATURE_REMOTE_CONFIG_MQTT_ENABLED ON)
set(FEATURE_AUTH_MODE "KEY")
set(FEATURE_AUTH_WITH_NOTLS OFF)
set(FEATURE_DEBUG_DEV_INFO_USED  OFF)
```
Run the following script for compilation.
```
./cmake_build.sh 
```
The demo output `remote_config_mqtt_sample` is in the `output/release/bin` folder.

#### 2. Enter the device information

Enter the information of the `airConditioner1` device created above in the JSON file `aircond_device_info1.json`:
```
{
    "auth_mode":"KEY",	
    "productId":"YI7XCD5DRH",
    "deviceName":"airConditioner1",	
    "key_deviceinfo":{    
        "deviceSecret":"+PBSkYlvNffjLDTGLpVOQA=="
    }
}
```

#### 3. Run the `remote_config_mqtt_sample` demo

You can see that the `airConditioner1` device has subscribed to the remote configuration message.

```
./output/release/bin/remote_config_mqtt_sample -l
INF|2020-08-26 19:31:03|qcloud_iot_device.c|iot_device_info_set(50): SDK_Ver: 3.2.1, Product_ID: YI7XCD5DRH, Device_Name: airConditioner1
DBG|2020-08-26 19:31:03|HAL_TLS_mbedtls.c|HAL_TLS_Connect(200): Setting up the SSL/TLS structure...
DBG|2020-08-26 19:31:03|HAL_TLS_mbedtls.c|HAL_TLS_Connect(242): Performing the SSL/TLS handshake...
DBG|2020-08-26 19:31:03|HAL_TLS_mbedtls.c|HAL_TLS_Connect(243): Connecting to /YI7XCD5DRH.iotcloud.tencentdevices.com/8883...
INF|2020-08-26 19:31:03|HAL_TLS_mbedtls.c|HAL_TLS_Connect(265): connected with /YI7XCD5DRH.iotcloud.tencentdevices.com/8883...
INF|2020-08-26 19:31:03|mqtt_client.c|IOT_MQTT_Construct(118): mqtt connect with id: 9AEwi success
INF|2020-08-26 19:31:03|remote_config_mqtt_sample.c|main(300): Cloud Device Construct Success
DBG|2020-08-26 19:31:03|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(142): topicName=$config/update/YI7XCD5DRH/airConditioner1|packet_id=13502
INF|2020-08-26 19:31:03|remote_config_mqtt_sample.c|_mqtt_event_handler(128): subscribe success, packet-id=13502
DBG|2020-08-26 19:31:03|remote_config_mqtt.c|_config_mqtt_sub_event_handler(156): mqtt config topic subscribe success
INF|2020-08-26 19:31:03|remote_config_mqtt_sample.c|main(315): config topic subscribe success, packetid:13502
```

##### 3.1 The device subscribes to the remote configuration function

```
int IOT_Subscribe_Config(void *client, ConfigSubscirbeUserData *config_sub_userdata, int subscribe_timeout)
```

#### 4. Add the remote configuration content to the product on IoT Hub

Click **Manage** for the `AirConditioner` product, click the **Remote Configuration** tab, add the remote configuration content, save, and enable **Remote Configuration**.
![image.png](https://main.qcloudimg.com/raw/3d51b3214be20ea52f64db1efa687335.png)

#### 5. Observe the device actively requesting a configuration update

Observe the printout of the `airConditioner1` device, and you can see that the device published a message to get the configuration and received the configuration response from IoT Hub: `Recv Msg Topic:$config/update/YI7XCD5DRH/airConditioner1` `"type":"reply"`

The configuration information is `{"baud rate":9600,"data bits":8,"stop bit":1,"parity":"NONE","thread sleep":1000}`.
```
DBG|2020-08-26 19:31:03|mqtt_client_publish.c|qcloud_iot_mqtt_publish(340): publish packetID=0|topicName=$config/report/YI7XCD5DRH/airConditioner1|payload={"type":"get"}
DBG|2020-08-26 19:31:08|remote_config_mqtt.c|_config_mqtt_message_callback(90): Recv Msg Topic:$config/update/YI7XCD5DRH/airConditioner1, buff data:{
   "type":"reply",
   "result":0,
   "payload": {"baud rate":9600,"data bits":8,"stop bit":1,"parity":"NONE","thread sleep":1000}
}
INF|2020-08-26 19:31:08|remote_config_mqtt_sample.c|_on_config_proc_handler(94): config message arrived , proc result: 9600, 8, 0, 1, 1000
INF|2020-08-26 19:31:08|remote_config_mqtt_sample.c|_on_config_proc_handler(96): config message arrived , configData: {"baud rate":9600,"data bits":8,"stop bit":1,"parity":"NONE","thread sleep":1000}

```

##### 5.1 The device actively requests the configuration update function

```
int IOT_Get_Config(void *client, char *json_buffer, int buffer_size, int reply_timeout)
```

#### 6. Observe IoT Hub actively delivering the configuration

Observe the printout of the `airConditioner1` device, and you can see that the received remote configuration message has been processed: `Recv Msg Topic:$config/update/YI7XCD5DRH/airConditioner1` `"type":"push"`

The configuration information is `{"baud rate":115200,"data bits":8,"stop bit":1,"parity":"NONE","thread sleep":1000}`.

```
DBG|2020-08-26 19:31:30|remote_config_mqtt.c|_config_mqtt_message_callback(90): Recv Msg Topic:$config/update/YI7XCD5DRH/airConditioner1, payload:{
   "type":"push",
   "payload": {"baud rate":115200,"data bits":8,"stop bit":1,"parity":"NONE","thread sleep":1000}
}
INF|2020-08-26 19:31:30|remote_config_mqtt_sample.c|_on_config_proc_handler(44): config message arrived , configData: {"baud rate":115200,"data bits":8,"stop bit":1,"parity":"NONE","thread sleep":1000}
INF|2020-08-26 19:31:30|remote_config_mqtt_sample.c|_on_config_proc_handler(96): config message arrived , proc end: 115200, 8, 0, 1, 1000

```

#### 7. Description of remote configuration result codes sent by IoT Hub

| Result Code | Description                             |
|--------|----------------------------------|
| 0	     | Remote configuration has been obtained successfully                 |
| 1001	 | The remote configuration feature of the product is disabled on IoT Hub  |
| 1002	 | There is no remote configuration content of the product on IoT Hub |
| 1003	 | Remote configuration of the product on IoT Hub is invalid   |

***At this point, you have completed the execution of the remote configuration demo.***

When you use the remote configuration feature, you can refer to the demo and modify the `_on_config_proc_handler` function to adapt to the configuration parsing and update of the actual device.
