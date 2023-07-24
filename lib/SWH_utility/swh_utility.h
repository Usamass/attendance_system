#ifndef _SWH_UTILS_H
#define _SWH_UTILS_H
#include "device_configs.h"
#include "../SWH_data_buffers.h"
#include <time.h>
#define MAX_DEVICE_CONFIG_BUFFER (50)

char* serialize_it(device_config_t* dConfig);
char* str_replace(char* str, const char* old, const char* new);
struct tm date_time_parser(char* date_str , char* time_str);
char* attendanceToJson(char* vu_id);
char* enrollmentToJson(char* vu_id , int f_count);

#endif /*_SWH_UTILS_H*/

