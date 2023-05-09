// all data buffers are defined here.
#ifndef _DATA_BUFFER_H
#define _DATA_BUFFER_H
#include <stdio.h>
#include "SWH_custom_data_structs.h"
#include "../mapping_table/mapping_table.h"
#define MAX_HTTP_OUTPUT_BUFFER (1000)

extern char* client_receive_buffer;
extern mapping_strct mp_struct;
extern mapping_t id_mapping;
#endif /*_DATA_BUFFER_H*/