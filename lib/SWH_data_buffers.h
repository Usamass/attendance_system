// all data buffers are defined here.
#ifndef _DATA_BUFFER_H
#define _DATA_BUFFER_H
#include <stdio.h>
#include <ds1307.h>
#include "SWH_custom_data_structs.h"
#include "../mapping_table/mapping_table.h"
#include "device_configs.h"
#define MAX_HTTP_OUTPUT_BUFFER (2000)

typedef enum {device_configs_read , device_configs_write} device_configs;
extern char* client_receive_buffer;
extern mapping_strct mp_struct;
extern mapping_t id_mapping;
extern i2c_dev_t dev;
extern struct tm mytime;
extern uint8_t disp_msg;
extern uint8_t opt_flag;
extern char* vu_id;
extern const char* new_html_files[3];
extern device_configs dConfig_flag;
extern device_config_t dConfig; // device ip and mac will be set on connect to network.
extern bool resource_mutex;
extern char* my_id;

#endif /*_DATA_BUFFER_H*/