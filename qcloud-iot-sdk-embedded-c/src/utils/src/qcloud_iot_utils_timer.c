#ifdef __cplusplus
extern "C" {
#endif
    
#include "qcloud_iot_utils_timer.h"
    
char expired(Timer *timer) {
    return HAL_Timer_expired(timer);
}

void countdown_ms(Timer *timer, unsigned int timeout_ms) {
    HAL_Timer_countdown_ms(timer, timeout_ms);
}

void countdown(Timer *timer, unsigned int timeout) {
    HAL_Timer_countdown(timer, timeout);
}

int left_ms(Timer *timer) {
    return HAL_Timer_remain(timer);
}

void InitTimer(Timer *timer) {
    HAL_Timer_init(timer);
}
    
#ifdef __cplusplus
}
#endif
