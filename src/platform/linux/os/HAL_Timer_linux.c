#ifdef __cplusplus
extern "C" {
#endif
    
#include <time.h>
#include <sys/time.h>
    
#include "qcloud_iot_import.h"

static char now_time_str[20] = {0};
    
char HAL_Timer_expired(Timer *timer) {
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(&timer->end_time, &now, &res);
    return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}

void HAL_Timer_countdown_ms(Timer *timer, unsigned int timeout_ms) {
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
    timeradd(&now, &interval, &timer->end_time);
}

void HAL_Timer_countdown(Timer *timer, unsigned int timeout) {
    struct timeval now;
    gettimeofday(&now, NULL);
    struct timeval interval = {timeout, 0};
    timeradd(&now, &interval, &timer->end_time);
}

int HAL_Timer_remain(Timer *timer) {
    struct timeval now, res;
    gettimeofday(&now, NULL);
    timersub(&timer->end_time, &now, &res);
    return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}

void HAL_Timer_init(Timer *timer) {
    timer->end_time = (struct timeval) {0, 0};
}

char* HAL_Timer_current(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	time_t now_time = tv.tv_sec;

	struct tm tm_tmp = *localtime(&now_time);
	strftime(now_time_str, 20, "%F %T", &tm_tmp);

	return now_time_str;
}
    
#ifdef __cplusplus
}
#endif
