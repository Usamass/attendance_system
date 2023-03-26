#include <stdio.h>
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
    // data pointer depending upon flay_type(read, write or query)
    char* data;

}SPIFFS_NOTIFIER;