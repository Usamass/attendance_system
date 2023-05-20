#include "mapping_table.h"


char* deserialize_it(mapping_t* id_mapping , mapping_strct* mp_strct)
{  
    printf("vu_id in mapping table %s" , mp_strct->vu_id_st);
    if (id_mapping->mapping_arr == NULL){
        //id_mapping->mapping_arr = (char*)malloc(sizeof(char) * ALLOC_SIZE);
        printf("inside null mapping\n");
        id_mapping->root = cJSON_CreateArray();
        id_mapping->size_of_arr = 0;

        cJSON* first_object;
        first_object = cJSON_CreateObject();

        cJSON_AddStringToObject(first_object , "vu_id" , mp_strct->vu_id_st);
        cJSON_AddNumberToObject(first_object , "f_id_1" , mp_strct->f_id_st);
        cJSON_AddNumberToObject(first_object , "templates" , 1);
        cJSON_AddItemToArray(id_mapping->root , first_object);

        id_mapping->mapping_arr = cJSON_Print(id_mapping->root);

        return id_mapping->mapping_arr;


    }
    else {

        //id_mapping->mapping_arr = (char*)realloc(id_mapping->mapping_arr , sizeof(char) * ALLOC_SIZE); 
        printf("inside mapping\n");
        cJSON* first_object;
        int tmpl = 0;
        first_object = cJSON_CreateObject();
        tmpl = get_tamp_count(id_mapping , mp_strct->vu_id_st);

        if (tmpl == -1){   // new vu_id
            tmpl = 1;
            cJSON_AddStringToObject(first_object , "vu_id" , mp_strct->vu_id_st);
            cJSON_AddNumberToObject(first_object , "f_id_1" , mp_strct->f_id_st);
            cJSON_AddNumberToObject(first_object , "templates" , tmpl);
            cJSON_AddItemToArray(id_mapping->root , first_object);
            

        }
        else {
            printf("inside f_id_2\n");
            tmpl++;
            for (int i = 0 ; i < cJSON_GetArraySize(id_mapping->root) ; i++) {
                cJSON* obj = cJSON_GetArrayItem(id_mapping->root , i);

                if (strcmp(mp_strct->vu_id_st , cJSON_GetObjectItem(obj , "vu_id")->valuestring) == 0){
                    cJSON_AddNumberToObject(obj , "f_id_2" , mp_strct->f_id_st);
                    cJSON* item = cJSON_GetObjectItem(obj , "templates");
                    cJSON_SetNumberValue(item, (double)tmpl);

                }
            }            


        }
    
        //free(mp_strct->vu_id_st);
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

char* get_vu_id(mapping_t* id_mapping , int f_id)
{
    cJSON* obj = NULL;
    cJSON* arr_item = NULL;
    char* vu_id = (char*)malloc(sizeof(char) * 20);
    printf("getting vu_id : %d\n" , f_id);
    obj = cJSON_Parse(id_mapping->mapping_arr);
    printf("size of cJSON array: %d\n" , cJSON_GetArraySize(obj));
    
    for(int i = 0 ; i < cJSON_GetArraySize(obj) ; i++){
        printf("inside for loop of mapping_table\n");

        arr_item = cJSON_GetArrayItem(id_mapping->root , i);
        if (f_id == cJSON_GetObjectItem(arr_item , "f_id_1")->valueint){
            //printf("vu_id_1 : %s\n" , vu_id = cJSON_GetObjectItem(arr_item , "vu_id")->valuestring);
            vu_id = cJSON_GetObjectItem(arr_item , "vu_id")->valuestring;
            printf("from mapping_table %s" , vu_id);
            return vu_id;
        }
        else if (cJSON_GetObjectItem(arr_item , "f_id_2")) {
            if (f_id == cJSON_GetObjectItem(arr_item , "f_id_2")->valueint){
                //printf("vu_id_2 : %s" ,
                vu_id = cJSON_GetObjectItem(arr_item , "vu_id")->valuestring;
                printf("from mapping_table %s" , vu_id);
                return vu_id;


            }
        }
        
    }
    return NULL;

}

void parse_mapping(mapping_t* id_mapping)
{
    id_mapping->root = cJSON_Parse(id_mapping->mapping_arr);
}







