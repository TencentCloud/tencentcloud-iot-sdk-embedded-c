# C-SDK Multi-Thread and Multi-Device Support

To support multi-thread and multi-device instances, you need to enable the compilation configuration item `FEATURE_MULTITHREAD_ENABLED` first.

## Notes on Use in Multithreaded Environment

The device C-SDK can be used in a multithreaded and multitasking environment, but there are certain specifications for the use of APIs.
For example, to use MQTT APIs in a multithreaded environment, you need to pay attention to the following:
```
1. You cannot use multiple threads to call `IOT_MQTT_Yield`, `IOT_MQTT_Construct`, or `IOT_MQTT_Destroy`.
2. You can use multiple threads to call `IOT_MQTT_Publish`, `IOT_MQTT_Subscribe`, and `IOT_MQTT_Unsubscribe`.
3. As the function to read MQTT messages from `socket` and process them, `IOT_MQTT_Yield` should have a certain execution time to prevent it from being suspended or preempted for a long time.
```
In a multithreaded environment, we recommend you use `IOT_MQTT_StartLoop` and `IOT_MQTT_StopLoop` to start and stop the `IOT_MQTT_Yield` thread on the backend, so that when you need to read MQTT messages, you don't need to proactively call the `IOT_MQTT_Yield` function; instead, the backend thread will read and process them.
For the use cases, please see `samples/mqtt/multi_thread_mqtt_sample`.

For more information on how to use the APIs of other modules, please see the usage of MQTT APIs.

## Multi-Client and Multi-Device Connection

Starting from v3.2.0, the SDK supports multi-device connection. It encapsulates the status variables and parameters in an object-oriented way, so that, for example, one `MQTT/Shadow Client` instance can correspond to one device. You can provide multiple device identity triplets and create multiple `Client` instances to connect multiple devices at the same time for communication.
For the specific sample code, please see the samples in `samples/multi_client`. You can create multiple threads and create their respective `MQTT/Shadow Client` in each thread, and multiple devices can access the backend service at the same time.


