## Overview
IoT Hub provides a dedicated [Tencent Cloud IoT AT instruction set](https://github.com/tencentyun/qcloud-iot-sdk-tencent-at-based/blob/master/docs/%E8%85%BE%E8%AE%AF%E4%BA%91IoT%20AT%E6%8C%87%E4%BB%A4%E9%9B%86-V3.1.3.pdf). If the communication module implements this instruction set, it will be easier for devices to connect and communicate, and less code will be required. For this scenario, please refer to the [SDK for MCU AT](https://github.com/tencentyun/qcloud-iot-sdk-tencent-at-based.git) dedicated to the Tencent Cloud customized AT module.

Currently, Tencent Cloud works closely with major module vendors to port the core protocols of the SDK to modules, which encapsulate unified Tencent Cloud AT instructions. The modules that support Tencent Cloud customized AT instructions are as listed below:

| No.  | Module Vendor     |   Module Model       |  Communication Standard      |   Firmware Version      |
| -------| ------------| -------------------|------------------|----------------|
| 1      | China Mobile        |   M5311          |      NB-IoT    |  M5311_LV_MOD_BOM_R002_1901232019_0906   |
| 2      | China Mobile        |   M6315         |          2G    |   CMIOT_M6315_20180901_V10_EXT_20190827_152209     |
| 3     | China Mobile         |   M8321          |         4G     |   QCloud_AT_v3.0.1_4G_Cellular 20190909_171245     |
| 4      | Neoway               | N10              |     2G         |     N10_I_1187_PQS63010_TC_V002C   |
| 5      | Neoway               | N21                |    NB-IoT      |     N21_RDD0CM_TC_V006A   |
| 6      | Neoway               | N720            |    4G         |    N720_EAB0CMF_BZ_V003A_T1    |
| 7      | MobileTek               | L206            |    2G         | L206Dv01.04b04.04 |
| 8      | Espressif               | ESP8266            |    Wi-Fi         |   QCloud_AT_ESP_WiFi_v1.1.0     |

