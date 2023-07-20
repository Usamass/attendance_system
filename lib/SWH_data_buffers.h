// all data buffers are defined here.
#ifndef _DATA_BUFFER_H
#define _DATA_BUFFER_H
#include <stdio.h>
#include <ds1307.h>
#include "SWH_custom_data_structs.h"
#include "../mapping_table/mapping_table.h"
#define MAX_HTTP_OUTPUT_BUFFER (1000)

extern char* client_receive_buffer;
extern mapping_strct mp_struct;
extern mapping_t id_mapping;
extern i2c_dev_t dev;
extern struct tm mytime;
extern uint8_t disp_msg;
extern uint8_t opt_flag;
extern char* vu_id;
extern const char* new_html_files[3];
#endif /*_DATA_BUFFER_H*/