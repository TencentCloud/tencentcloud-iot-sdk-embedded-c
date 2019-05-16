final-out :
	$(TOP_Q) \
	mkdir -p ${FINAL_DIR}  ${DIST_DIR}  ${FINAL_DIR}/lib \
			 ${FINAL_DIR}/include  ${FINAL_DIR}/bin ${FINAL_DIR}/src \
			 ${FINAL_DIR}/unittest
	
	$(TOP_Q) \
	mv ${COMP_LIB} ${FINAL_DIR}/lib/ && \
	mv ${PLATFORM_LIB} ${FINAL_DIR}/lib
	
ifeq (,$(filter -DAUTH_WITH_NOTLS,$(CFLAGS)))
	$(TOP_Q) \
	mv ${TEMP_DIR}/*.a ${FINAL_DIR}/lib/
endif
	
	$(TOP_Q) \
	rm -rf ${TEMP_DIR}
	
.PHONY: mqtt_sample ota_mqtt_sample ota_coap_sample shadow_sample coap_sample nbiot_sample samples_final gateway_sample multi_thread_mqtt_sample dynamic_reg_dev_sample event_sample
	
final : mqtt_sample ota_mqtt_sample ota_coap_sample shadow_sample coap_sample nbiot_sample samples_final gateway_sample multi_thread_mqtt_sample dynamic_reg_dev_sample event_sample
