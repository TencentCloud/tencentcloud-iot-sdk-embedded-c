#include "time.h"
#include "util.h"

std::string log_time(){
    char str_time[255];
	time_t Today;
	tm* Time;
	time(&Today);
	Time = localtime(&Today);
	strftime(str_time, 255, "[%Y-%m-%d %H:%M:%S]", Time);
	return str_time;
}
