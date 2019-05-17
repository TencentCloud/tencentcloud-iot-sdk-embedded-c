# 腾讯物联网设备端 C-SDK
腾讯物联网设备端 C-SDK 依靠安全且性能强大的数据通道，为物联网领域开发人员提供终端(如传感器, 执行器, 嵌入式设备或智能家电等等)和云端的双向通信能力。C-SDK V3.0.0版本以后同时支持腾讯的现有的两个物联网平台，[物联网通信](https://console.cloud.tencent.com/iotcloud/products)和[物联网开发平台](https://console.cloud.tencent.com/iotexplorer)。

## SDK 架构图
![](https://main.qcloudimg.com/raw/fb3ce5f898ba604e47a396b0dd5dbf8e.jpg)

## SDK 源码结构及说明
| 名称                            | 说明                                                         |
| ------------------------------- | ------------------------------------------------------------ |
| docs                            | 文档目录，SDK在不同平台下使用说明文档。                           |
| packages                     | 第三方软件包组件，目前有mbedTLS库、gtest库 |
| src                             | SDK核心通信协议及服务                                      |
| samples                     | 应用示例 |
| tools                           | 物联网开发平台数据模板配置文件代码生成脚本。                                               |
| README.md                       | SDK 使用说明。                                               |

## 快速开始
物联网开发平台请参考[物联网开发平台SDK说明文档](https://github.com/tencentyun/qcloud-iot-sdk-embedded-c/blob/master/docs/物联网开发平台.md)，物联网通信平台请参考[物联网通信SDK说明文档](https://github.com/tencentyun/qcloud-iot-sdk-embedded-c/blob/master/docs/物联网通信.md)。

## 移植说明
C-SDK和平台交互的协议逻辑和OS/硬件平台无关，涉及OS/硬件平台相关代码全部剥离在 src/platform 目录，C-SDK已经实现了linux环境面HAL层相关实现，其他嵌入式RTOS/Windows及特定硬件平台请参考适配。

请参考[跨平台移植指导](https://cloud.tencent.com/document/product/634/12515)


#### 关于 SDK 的更多使用方式及接口了解, 请访问[官方 WiKi](https://github.com/tencentyun/qcloud-iot-sdk-embedded-c/wiki)