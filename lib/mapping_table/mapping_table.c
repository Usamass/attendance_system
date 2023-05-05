#include "mapping_table.h"


char* deserialize_it(mapping_t* id_mapping , mapping_strct* mp_strct)
{  
    if (id_mapping->mapping_arr == NULL){
        id_mapping->root = cJSON_CreateArray();
        id_mapping->size_of_arr = 0;

        cJSON* first_object;
        first_object = cJSON_CreateObject();

        cJSON_AddStringToObject(first_object , "vu_id" , mp_strct->vu_id_st);
        cJSON_AddNumberToObject(first_object , "f_id" , mp_strct->f_id_st);
        cJSON_AddNumberToObject(first_object , "templates" , 1);
        cJSON_AddItemToArray(id_mapping->root , first_object);

        id_mapping->mapping_arr = cJSON_Print(id_mapping->root);

        return id_mapping->mapping_arr;


    }
    else {

        cJSON* first_object;
        int tmpl = 0;
        first_object = cJSON_CreateObject();
        tmpl = get_tamp_count(id_mapping , mp_strct->vu_id_st);
        tmpl = (tmpl == -1) ? 1 : tmpl +1;

        cJSON_AddStringToObject(first_object , "vu_id" , mp_strct->vu_id_st);
        cJSON_AddNumberToObject(first_object , "f_id" , mp_strct->f_id_st);
        cJSON_AddNumberToObject(first_object , "templates" , tmpl);
        cJSON_AddItemToArray(id_mapping->root , first_object);

        id_mapping->mapping_arr = cJSON_Print(id_mapping->root);

        return id_mapping->mapping_arr;    
    }
}

int get_mapping_size(mapping_t* id_mapping)
{
    return cJSON_GetArraySize(id_mapping->root);

}

int get_tamp_count(mapping_t* id_mapping , const char* id)
{
    int tmpl = -1;
    for (int i = 0 ; i < cJSON_GetArraySize(id_mapping->root) ; i++) {
        cJSON* obj = cJSON_GetArrayItem(id_mapping->root , i);

        if (strcmp(id , cJSON_GetObjectItem(obj , "vu_id")->valuestring) == 0){
            tmpl = cJSON_GetObjectItem(obj , "templates")->valueint;
            
        }

    }
    return tmpl;

}

int get_finger_id(mapping_t* id_mapping , const char* id)
{
    int f_id = -1;
    for (int i = 0 ; i < cJSON_GetArraySize(id_mapping->root) ; i++) {
        cJSON* obj = cJSON_GetArrayItem(id_mapping->root , i);

        if (strcmp(id , cJSON_GetObjectItem(obj , "vu_id")->valuestring) == 0){
            f_id = cJSON_GetObjectItem(obj , "f_id")->valueint;
            
        }

    }
    return f_id;

}





