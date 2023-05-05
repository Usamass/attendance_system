#include <string.h>
#include "swh_utility.h"
#include "device_configs.h"
#include "cJSON.h"

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

const char* str_replace(char* str, const char* old, const char* new) {
    // allocate memory for the result string
    int old_len = strlen(old);
    int new_len = strlen(new);
    int size = strlen(str) + 1;
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
        } else {
            // copy the current character into the result string
            *q++ = *p++;
        }
    }
    *q = '\0';  // terminate the result string

    return result;
}
