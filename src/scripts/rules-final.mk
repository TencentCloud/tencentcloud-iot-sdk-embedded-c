final-out :
	$(TOP_Q) \
	mkdir -p ${FINAL_DIR}  ${DIST_DIR}  ${FINAL_DIR}/lib \
			 ${FINAL_DIR}/include  ${FINAL_DIR}/bin ${FINAL_DIR}/src \
			 ${FINAL_DIR}/unittest
	
	$(TOP_Q) \
	mv ${COMP_LIB} ${FINAL_DIR}/lib/ && \
	mv ${PLATFORM_LIB} ${FINAL_DIR}/lib && \
	mv ${TEMP_DIR}/*.a ${FINAL_DIR}/lib/
	
	$(TOP_Q) \
	rm -rf ${TEMP_DIR}
	
.PHONY: mqtt_sample shadow_sample coap_sample samples_final
	
final : mqtt_sample shadow_sample coap_sample samples_final