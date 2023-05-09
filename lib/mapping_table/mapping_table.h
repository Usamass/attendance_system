#ifndef _MAPPING_TABLE_H
#define _MAPPING_TABLE_H
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "../SWH_custom_data_structs.h"

typedef struct {
    cJSON* root;
    char* mapping_arr;
    int size_of_arr;
        
}mapping_t;

char* deserialize_it(mapping_t* , mapping_strct*);
void get_vu_id(mapping_t* id_mapping , int f_id);
int get_mapping_size(mapping_t* id_mapping);
int get_tamp_count(mapping_t* id_mapping , const char* id);
int get_finger_id(mapping_t* id_mapping , const char* id);
void parse_mapping(mapping_t* id_mapping);



#endif
