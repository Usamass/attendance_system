#include <stdio.h>
#ifndef _CUSTOM_DATA_STRUCT_H
#define _CUSTOM_DATA_STRUCT_H
// ------------- include all the notifications msgs in this struct--------
typedef struct
{
    int val;
    char* from;
    char* msg;

} NOTIFIER;
// data source recognition
typedef enum
{
    CLIENT,
    FINGER_PRINT,
    DEVICE_CONFIGS
} DataSource_t;

typedef struct {

    DataSource_t data_scr;
    int flag_type;
    // data pointer depending upon flag_type(read, write or query)
    char* data;

}SPIFFS_NOTIFIER;

typedef struct {
    char* vu_id_st;
    int f_id_st;

}mapping_strct;



#endif /*_CUSTOM_DATA_STRUCT_H*/