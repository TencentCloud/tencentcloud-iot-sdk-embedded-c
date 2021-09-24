# Getting Started with Gateway Device
This document describes how to apply for a gateway device and bind a subdevice in the IoT Hub console as well as quickly try out the connection of the gateway device to IoT Hub over the MQTT protocol for proxied subdevice connection/disconnection and message receiving/sending based on the **gateway_sample** of the SDK.


## 1. Creating Gateway Device in Console

#### 1. Create a gateway product and device
Click **Create Product** to create a product.

![](https://main.qcloudimg.com/raw/bc3df4129fe2acd3aefa6f9649670ff8.png)

In the pop-up window, select the node type and product type, enter the product name, select the authentication method and data format, enter the product description, and click **Confirm**. To create a general gateway product, you can select the options as shown below.

![](https://main.qcloudimg.com/raw/c42bde1d1e47986241019281d9e318ab.png)

On the generated product page, click **Add Device** on the **Devices** tab.

![](https://main.qcloudimg.com/raw/4a56087f02bfd7a99999cfbb0d3e6d5c.png)

If the authentication method is certificate authentication, after the device name is entered, be sure to click **Download** in the pop-up window. The device key file and device certificate in the downloaded package are used for authenticating the device during connection to IoT Hub.

![](https://main.qcloudimg.com/raw/1375d751f121a1fe59fe70b0a725402e.png)

If the authentication method is key authentication, after the device name is entered, the key of the added device will be displayed in the pop-up window.

![](https://main.qcloudimg.com/raw/4d442b6c1a05a0029359f8bdf7e173b4.png)

You can create gateway products and devices, but if you want a gateway device to communicate on behalf of a subdevice, you need to create a subproduct and a subdevice first just like with a general product and then add them on the gateway product page. Please note that a subdevice cannot directly connect to products on IoT Hub; instead, it relies on a gateway device for connection. Therefore, the authentication method of the subdevice does not affect the connection, and the gateway device is responsible for authenticating the connection.
After the subproduct is created, add it on the subproduct tab of the gateway product page first.

![](https://main.qcloudimg.com/raw/62968c854400ad231abb6b4224726d38.png)

Then, add the subdevice on the subdevice tab of the gateway device page.

![](https://main.qcloudimg.com/raw/47cc055a4f8bfa3f78137e82e654678e.png)

#### 2. Create a topic that supports subscribing and publishing

On the product settings page, click **Permissions** > **Add Topic Permission**.

![](https://main.qcloudimg.com/raw/f1e483d9642960b78058fbb85ac0a706.png)

In the pop-up window, enter `data`, set the operation permission to **Publish and Subscribe**, and click **Create**.

Then, the `productID/\${deviceName}/data` topic will be created, and you can view all permissions of the product in the permission list on the product page.


## 2. Compiling and Running Demo

#### 1. Compile the SDK
Modify `CMakeLists.txt` to ensure that the following options exist (with a **key-authenticated device** as example).
```
set(BUILD_TYPE                   "release")
set(COMPILE_TOOLS                "gcc") 
set(PLATFORM 	                "linux")
set(FEATURE_MQTT_COMM_ENABLED ON)
set(FEATURE_GATEWAY_ENABLED ON)
set(FEATURE_AUTH_MODE "KEY")
set(FEATURE_AUTH_WITH_NOTLS OFF)
set(FEATURE_DEBUG_DEV_INFO_USED  OFF)
```
Run the following script for compilation.
```
./cmake_build.sh 
```
The demo output is in the `output/release/bin` folder.

#### 2. Enter the device information
Enter the information of the gateway device and subdevice created above on the IoT Hub platform in `device_info.json` (with a **key-authenticated device** as example).
```
{
    "auth_mode":"KEY",	
	"productId":"NINEPLMEB6",
  	"deviceName":"Gateway-demo",	
    "key_deviceinfo":{    
        "deviceSecret":"vX23qEazsGsMyf5SMfs6OA6y"
    }
	
	"subDev":{
        "sub_productId":"S3EUVBRJLB",
        "sub_devName":"test_device"
    }
}
```

#### 3. Run the gateway demo
The following log information shows that the demo connected, disconnected, and published and subscribed to messages successfully through the MQTT gateway on behalf of the subdevice.
```
./gateway_sample
INF|2019-09-16 14:35:34|device.c|iot_device_info_set(67): SDK_Ver: 3.1.0, Product_ID: NINEPLMEB6, Device_Name: Gateway-demo
DBG|2019-09-16 14:35:34|HAL_TLS_mbedtls.c|HAL_TLS_Connect(204): Setting up the SSL/TLS structure...
DBG|2019-09-16 14:35:34|HAL_TLS_mbedtls.c|HAL_TLS_Connect(246): Performing the SSL/TLS handshake...
DBG|2019-09-16 14:35:34|HAL_TLS_mbedtls.c|HAL_TLS_Connect(247): Connecting to /NINEPLMEB6.iotcloud.tencentdevices.com/8883...
INF|2019-09-16 14:35:34|HAL_TLS_mbedtls.c|HAL_TLS_Connect(269): connected with /NINEPLMEB6.iotcloud.tencentdevices.com/8883...
INF|2019-09-16 14:35:34|mqtt_client.c|IOT_MQTT_Construct(125): mqtt connect with id: Senw3 success
DBG|2019-09-16 14:35:34|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(138): topicName=$gateway/operation/result/NINEPLMEB6/Gateway-demo|packet_id=27802
DBG|2019-09-16 14:35:34|gateway_api.c|_gateway_event_handler(23): gateway sub|unsub(3) success, packet-id=27802
DBG|2019-09-16 14:35:34|gateway_api.c|IOT_Gateway_Subdev_Online(125): there is no session, create a new session
DBG|2019-09-16 14:35:34|mqtt_client_publish.c|qcloud_iot_mqtt_publish(337): publish packetID=0|topicName=$gateway/operation/NINEPLMEB6/Gateway-demo|payload={"type":"online","payload":{"devices":[{"product_id":"S3EUVBRJLB","device_name":"test_device"}]}}
INF|2019-09-16 14:35:35|gateway_common.c|_gateway_message_handler(134): client_id(S3EUVBRJLB/test_device), online success. result 0
DBG|2019-09-16 14:35:35|mqtt_client_subscribe.c|qcloud_iot_mqtt_subscribe(138): topicName=S3EUVBRJLB/test_device/data|packet_id=27803
DBG|2019-09-16 14:35:35|gateway_api.c|_gateway_event_handler(23): gateway sub|unsub(3) success, packet-id=27803
INF|2019-09-16 14:35:35|gateway_sample.c|_event_handler(61): subscribe success, packet-id=27803
DBG|2019-09-16 14:35:35|mqtt_client_publish.c|qcloud_iot_mqtt_publish(329): publish topic seq=27804|topicName=S3EUVBRJLB/test_device/data|payload={"data":"test gateway"}
INF|2019-09-16 14:35:35|gateway_sample.c|_event_handler(88): publish success, packet-id=27804
INF|2019-09-16 14:35:35|gateway_sample.c|_message_handler(111): Receive Message With topicName:S3EUVBRJLB/test_device/data, payload:{"data":"test gateway"}
DBG|2019-09-16 14:35:36|mqtt_client_publish.c|qcloud_iot_mqtt_publish(337): publish packetID=0|topicName=$gateway/operation/NINEPLMEB6/Gateway-demo|payload={"type":"offline","payload":{"devices":[{"product_id":"S3EUVBRJLB","device_name":"test_device"}]}}
INF|2019-09-16 14:35:36|gateway_common.c|_gateway_message_handler(139): client_id(S3EUVBRJLB/test_device), offline success. result 0
INF|2019-09-16 14:35:37|mqtt_client_connect.c|qcloud_iot_mqtt_disconnect(437): mqtt disconnect!
INF|2019-09-16 14:35:37|mqtt_client.c|IOT_MQTT_Destroy(186): mqtt release!
```

#### 4. Observe the subdevice connection
The following log information shows that the gateway device `Gateway-demo` in the demo connected the `test_device` subdevice to IoT Hub successfully over MQTT:
```
DBG|2019-09-16 14:35:34|mqtt_client_publish.c|qcloud_iot_mqtt_publish(337): publish packetID=0|topicName=$gateway/operation/NINEPLMEB6/Gateway-demo|payload={"type":"online","payload":{"devices":[{"product_id":"S3EUVBRJLB","device_name":"test_device"}]}}
INF|2019-09-16 14:35:35|gateway_common.c|_gateway_message_handler(134): client_id(S3EUVBRJLB/test_device), online success. result 0
```

#### 5. Observe the message receiving/sending by the gateway on behalf of the subdevice
The following log information shows that the gateway device `Gateway-demo` in the demo received and sent messages over MQTT on behalf of the `test_device` subdevice.
```
DBG|2019-09-16 14:35:35|mqtt_client_publish.c|qcloud_iot_mqtt_publish(329): publish topic seq=27804|topicName=S3EUVBRJLB/test_device/data|payload={"data":"test gateway"}
INF|2019-09-16 14:35:35|gateway_sample.c|_event_handler(88): publish success, packet-id=27804
INF|2019-09-16 14:35:35|gateway_sample.c|_message_handler(111): Receive Message With topicName:S3EUVBRJLB/test_device/data, payload:{"data":"test gateway"}
```
#### 6. Observe the logs of all sub-devices unbound by the gateway
```
DBG|2021-05-18 11:09:07|gateway_common.c|_gateway_message_handler(369): msg payload: {"type":"unbind_all"}
DBG|2021-05-18 11:09:07|gateway_common.c|_gateway_message_handler(377): recv request for unbind_all
DBG|2021-05-18 11:09:07|gateway_common.c|_gateway_subdev_unbind_all(300): remove all session product id: D95V6NB56B device_name: deng1
DBG|2021-05-18 11:09:07|gateway_common.c|_gateway_subdev_unbind_all_reply(325): reply {"type":"unbind_all", "payload":{"result":0}}
DBG|2021-05-18 11:09:07|mqtt_client_publish.c|qcloud_iot_mqtt_publish(341): publish packetID=0|topicName=$gateway/operation/GU25ZFSZLA/gw1|payload={"type":"unbind_all", "payload":{"result":0}}
DBG|2021-05-18 11:09:07|gateway_api.c|_gateway_event_handler(62): gateway all subdev have been unbind
DBG|2021-05-18 11:09:07|gateway_sample.c|_event_handler(90): gateway all subdev have been unbind
INF|2021-05-18 11:09:07|gateway_sample.c|_event_handler(75): publish success, packet-id=51834
DBG|2021-05-18 11:09:07|gateway_sample.c|main(408): gateway all subdev have been unbind by cloud platform stop publish subdev msg
DBG|2021-05-18 11:09:07|gateway_api.c|IOT_Gateway_Subdev_Offline(209): no session, can not offline
ERR|2021-05-18 11:09:07|gateway_sample.c|main(420): IOT_Gateway_Subdev_Offline fail.
```
## 3. Code Description

#### How do I bind a subdevice through the SDK?
You can use a gateway to bind or unbind a subdevice in the console or through MQTT messages. The SDK provides the corresponding APIs.
```
int IOT_Gateway_Subdev_Bind(void *client, GatewayParam *param, DeviceInfo *pBindSubDevInfo);
int IOT_Gateway_Subdev_Unbind(void *client, GatewayParam *param, DeviceInfo *pSubDevInfo);
```
For more information on the usage, please see the sample code of `gateway_sample`.
When you run `gateway_sample -b psk_device_info.json`, the subdevice specified in `psk_device_info.json` will be bound first, and then the connection, disconnection, and message receiving/sending operations will be performed.

#### How do I proxy multiple subdevices?
The `gateway_sample` demo shows how a gateway proxies one subdevice. If you need to add more subdevices, you can create them in the console, create more instances of the `GatewayParam` parameter in the code, and pass the parameter to the APIs of the gateway. In this way, a gateway can communicate on behalf of multiple subdevices.
```
    // one GatewayParam only support one sub-device
    // use more GatewayParam to add more sub-device
    GatewayParam gw_param1 = DEFAULT_GATEWAY_PARAMS;;
    gw_param1.product_id = gw_dev_info.gw_info.product_id;
    gw_param1.device_name = gw_dev_info.gw_info.device_name;

    gw_param1.subdev_product_id = "SUB-PRODUCT1";
    gw_param1.subdev_device_name = "SUB-DEVICE1";

    IOT_Gateway_Subdev_Online(client, &gw_param1);
```

#### How do I get the list of subdevices on IoT Hub bound to the gateway?
Before you bind a subdevice to the gateway, you can get the list of bound subdevices on IoT Hub first. The SDK provides the corresponding APIs.
```
int IOT_Gateway_Subdev_GetBindList(void *client, GatewayParam *param, SubdevBindList *subdev_bindlist);
void IOT_Gateway_Subdev_DestoryBindList(SubdevBindList *subdev_bindlist);

```

