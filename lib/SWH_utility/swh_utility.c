#include <string.h>
#include <esp_log.h>
#include "swh_utility.h"
#include "device_configs.h"
#include "cJSON.h"
#include "mapping_table.h"

/* for serializing the data received */
char* serialize_it(device_config_t* dConfig)
{
    char* device_config_buffer = (char*)malloc(MAX_DEVICE_CONFIG_BUFFER * sizeof(char));
    cJSON* root;
    root = cJSON_CreateObject();

    cJSON_AddStringToObject(root , "device_id" , dConfig->device_id);
    cJSON_AddStringToObject(root , "auth_token" , dConfig->authToken);
    cJSON_AddStringToObject(root , "device_location" , dConfig->location_name);
    cJSON_AddNumberToObject(root , "location_id" , dConfig->location_id);

    device_config_buffer = cJSON_Print(root);
    cJSON_Delete(root);


    return device_config_buffer;
}

// char* deserialize_it()
// {

// }

struct tm date_time_parser(char* date_str , char* time_str)
{
    struct tm mytime = {0};

    if (strptime(date_str, "%Y-%m-%d", &mytime) == NULL) {
        fprintf(stderr, "Failed to parse date string: %s\n", date_str);
        exit(EXIT_FAILURE);
    }
    strptime(time_str, "%H:%M", &mytime);

    printf("Year: %d\n", mytime.tm_year + 1900);
    printf("Month: %d\n", mytime.tm_mon + 1);
    printf("Day: %d\n", mytime.tm_mday);
    printf("Hour: %d\n", mytime.tm_hour);
    printf("Min: %d\n", mytime.tm_min);
    printf("Sec: %d\n", mytime.tm_sec);



    return mytime;



}

char* str_replace(char* str, const char* old, const char* new) {
    // allocate memory for the result string
    int old_len = strlen(old);
    int new_len = strlen(new);
    int size = strlen(str) + 100;
    char* result = (char*) malloc(size);

    // iterate over the input string, copying characters to the result string
    char* p = str;
    char* q = result;
    while (*p != '\0') {
        // check if the current substring matches the old string
        if (strncmp(p, old, old_len) == 0) {
            // copy the new string into the result string
            strncpy(q, new, new_len);
            q += new_len;
            p += old_len;
        } 
        else{
            // copy the current character into the result string
            *q++ = *p++;
        }
    }
    *q = '\0';  // terminate the result string
    return result;
}

char* attendanceToJson(char* vu_id) 
{
    char time_stamp[30] = {0};
    // char* attendance = malloc(sizeof(char) * 50);
    ds1307_get_time(&dev, &mytime);
    //2023-04-23T18:35:43Z
    //"timestamp": "2023-04-23T18:35:43.511Z"

    sprintf(time_stamp , "%04d-%02d-%02dT%02d:%02d:%02dZ", mytime.tm_year + 1900 , mytime.tm_mon
    , mytime.tm_mday , mytime.tm_hour , mytime.tm_min , mytime.tm_sec);

    printf("%s\n" , time_stamp);
        

    cJSON* first_object;
    first_object = cJSON_CreateObject();

    cJSON_AddStringToObject(first_object , "student_id" , vu_id);
    cJSON_AddStringToObject(first_object , "timestamp" , time_stamp);

    char* attendance = cJSON_Print(first_object);
    cJSON_Delete(first_object);
    //free(vu_id);

    return attendance;

    
}
char* enrollmentToJson(char* vu_id , int f_count) 
{  
    cJSON* first_object;
    first_object = cJSON_CreateObject();

    cJSON_AddStringToObject(first_object , "student_vuid" , vu_id);
    cJSON_AddNumberToObject(first_object , "finger_count" , f_count);

    char* enrollment = cJSON_Print(first_object);
    cJSON_Delete(first_object);
    //free(vu_id);

    return enrollment;

    
}