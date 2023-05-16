#ifndef _SWH_UTILS_H
#define _SWH_UTILS_H
#include "device_configs.h"
#include <time.h>
#define MAX_DEVICE_CONFIG_BUFFER (50)

char* serialize_it(device_config_t* dConfig);
const char* str_replace();
struct tm date_time_parser(char* date_str , char* time_str);


#endif /*_SWH_UTILS_H*/

