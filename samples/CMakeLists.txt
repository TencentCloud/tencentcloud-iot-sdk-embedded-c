
set(INC_DIR  ${PROJECT_SOURCE_DIR}/output/${BUILD_TYPE}/include)
set(LINK_DIR ${PROJECT_SOURCE_DIR}/output/${BUILD_TYPE}/lib/)
include_directories(${INC_DIR})
link_directories(${LINK_DIR})

if(FEATURE_AUTH_WITH_NOTLS)
set(link_lib
	iot_sdk
	iot_platform
)
else()
set(link_lib
	iot_sdk
	iot_platform
	mbedtls
	mbedx509
	mbedcrypto
)
endif()

if(WEBSOCKET_CLIENT STREQUAL "ON")
	set(link_lib ${link_lib} wslay)
endif()

if(CMAKE_COMPILER_IS_GNUCC)
	set(lib -Wl,--start-group ${link_lib} -Wl,--end-group)
else ()
	set(lib ${link_lib})
endif()


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/${BUILD_TYPE}/bin)

# MQTT
if(${FEATURE_MQTT_COMM_ENABLED} STREQUAL "ON")
file(GLOB src_mqtt_sample 					${PROJECT_SOURCE_DIR}/samples/mqtt/mqtt_sample.c)
add_executable(mqtt_sample 					${src_mqtt_sample})
target_link_libraries(mqtt_sample 				 ${lib})

file(GLOB src_door_mqtt_sample 				${PROJECT_SOURCE_DIR}/samples/scenarized/door_mqtt_sample.c)
add_executable(door_mqtt_sample 			${src_door_mqtt_sample})
target_link_libraries(door_mqtt_sample 		 	 ${lib})
endif()

# Multi-Thread Multi-Client sample
if (${FEATURE_MULTITHREAD_ENABLED} STREQUAL "ON")
	file(GLOB src_multi_thread_mqtt_sample 		${PROJECT_SOURCE_DIR}/samples/mqtt/multi_thread_mqtt_sample.c)
	add_executable(multi_thread_mqtt_sample 	${src_multi_thread_mqtt_sample})
	target_link_libraries(multi_thread_mqtt_sample 			 ${lib})

if (${FEATURE_DEBUG_DEV_INFO_USED} STREQUAL "OFF")
	file(GLOB src_multi_client_mqtt_sample 		${PROJECT_SOURCE_DIR}/samples/multi_client/multi_client_mqtt_sample.c)
	add_executable(multi_client_mqtt_sample 	${src_multi_client_mqtt_sample})
	target_link_libraries(multi_client_mqtt_sample 			 ${lib})

	file(GLOB src_multi_client_shadow_sample 		${PROJECT_SOURCE_DIR}/samples/multi_client/multi_client_shadow_sample.c)
	add_executable(multi_client_shadow_sample 	${src_multi_client_shadow_sample})
	target_link_libraries(multi_client_shadow_sample 			 ${lib})
endif()
endif()

# DYN_REG
if(${FEATURE_DEV_DYN_REG_ENABLED} STREQUAL "ON")
	file(GLOB src_dynreg_dev_sample		${PROJECT_SOURCE_DIR}/samples/dynreg_dev/dynreg_dev_sample.c)
	add_executable(dynreg_dev_sample 		${src_dynreg_dev_sample})
	target_link_libraries(dynreg_dev_sample 	 ${lib})
endif()

# OTA MQTT
if(${FEATURE_OTA_COMM_ENABLED} STREQUAL "ON" AND ${FEATURE_OTA_SIGNAL_CHANNEL} STREQUAL "MQTT")
	file(GLOB src_ota_mqtt_sample 				${PROJECT_SOURCE_DIR}/samples/ota/ota_mqtt_sample.c)
	add_executable(ota_mqtt_sample				${src_ota_mqtt_sample})
	target_link_libraries(ota_mqtt_sample			 ${lib})
endif()

# Resource MQTT
if(${FEATURE_RESOURCE_COMM_ENABLED} STREQUAL "ON" AND ${FEATURE_RESOURCE_SIGNAL_CHANNEL} STREQUAL "MQTT")
	file(GLOB src_resource_mqtt_sample 				${PROJECT_SOURCE_DIR}/samples/resource/resource_mqtt_sample.c)
	add_executable(resource_mqtt_sample				${src_resource_mqtt_sample})
	target_link_libraries(resource_mqtt_sample			 ${lib})
endif()

# SHADOW
if(${FEATURE_MQTT_DEVICE_SHADOW} STREQUAL "ON")
	file(GLOB src_shadow_sample 				${PROJECT_SOURCE_DIR}/samples/shadow/shadow_sample.c)
	add_executable(shadow_sample 				${src_shadow_sample})
	target_link_libraries(shadow_sample 			 ${lib})
	
	file(GLOB src_aircond_shadow_sample 		${PROJECT_SOURCE_DIR}/samples/scenarized/aircond_shadow_sample.c)
	file(GLOB src_aircond_shadow_sample_v2 		${PROJECT_SOURCE_DIR}/samples/scenarized/aircond_shadow_sample_v2.c)
	add_executable(aircond_shadow_sample 		${src_aircond_shadow_sample})
	add_executable(aircond_shadow_sample_v2 	${src_aircond_shadow_sample_v2})
	target_link_libraries(aircond_shadow_sample 	 ${lib})
	target_link_libraries(aircond_shadow_sample_v2   ${lib})
endif()


# GATEWAY
if(${FEATURE_GATEWAY_ENABLED} STREQUAL "ON")
	file(GLOB src_gateway_sample  				${PROJECT_SOURCE_DIR}/samples/gateway/gateway_sample.c)
	add_executable(gateway_sample 				${src_gateway_sample})
	target_link_libraries(gateway_sample 			 ${lib})
endif()

# GATEWAY SubDev ota
if(${FEATURE_OTA_COMM_ENABLED} STREQUAL "ON" AND ${FEATURE_OTA_SIGNAL_CHANNEL} STREQUAL "MQTT" AND ${FEATURE_GATEWAY_ENABLED} STREQUAL "ON")
	file(GLOB src_ota_mqtt_subdev_sample  				${PROJECT_SOURCE_DIR}/samples/ota/ota_mqtt_subdev_sample.c)
	add_executable(ota_mqtt_subdev_sample 				${src_ota_mqtt_subdev_sample})
	target_link_libraries(ota_mqtt_subdev_sample 			 ${lib})
endif()

# COAP
if(${FEATURE_COAP_COMM_ENABLED} STREQUAL "ON")
	file(GLOB src_coap_sample 					${PROJECT_SOURCE_DIR}/samples/coap/coap_sample.c)
	add_executable(coap_sample        	   	 	${src_coap_sample})
	target_link_libraries(coap_sample        	   	 ${lib})

	file(GLOB src_door_coap_sample 				${PROJECT_SOURCE_DIR}/samples/scenarized/door_coap_sample.c)
	add_executable(door_coap_sample 			${src_door_coap_sample})
	target_link_libraries(door_coap_sample 		 	 ${lib})
endif()


if(${FEATURE_OTA_COMM_ENABLED} STREQUAL "ON" AND ${FEATURE_OTA_SIGNAL_CHANNEL} STREQUAL "COAP")
	file(GLOB src_ota_coap_sample 				${PROJECT_SOURCE_DIR}/samples/ota/ota_coap_sample.c)
	add_executable(ota_coap_sample				${src_ota_coap_sample})
	target_link_libraries(ota_coap_sample			 ${lib})
endif()

if(${FEATURE_BROADCAST_ENABLED} STREQUAL "ON")
	file(GLOB src_broadcast_sample 				${PROJECT_SOURCE_DIR}/samples/broadcast/broadcast_sample.c)
	add_executable(broadcast_sample				${src_broadcast_sample})
	target_link_libraries(broadcast_sample			 ${lib})
endif()

if(${FEATURE_RRPC_ENABLED} STREQUAL "ON")
	file(GLOB src_rrpc_sample 					${PROJECT_SOURCE_DIR}/samples/rrpc/rrpc_sample.c)
	add_executable(rrpc_sample					${src_rrpc_sample})
	target_link_libraries(rrpc_sample			 	${lib})
endif()

if(${FEATURE_REMOTE_CONFIG_MQTT_ENABLED} STREQUAL "ON")
	file(GLOB src_remote_config_mqtt_sample 				${PROJECT_SOURCE_DIR}/samples/config/remote_config_mqtt_sample.c)
	add_executable(remote_config_mqtt_sample					${src_remote_config_mqtt_sample})
	target_link_libraries(remote_config_mqtt_sample			 	${lib})
endif()

