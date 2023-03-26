// All device configurations are managed here
#include <inttypes.h>
typedef struct
{
    char* device_id;
    char* MAC_addr;
    int   location_id;
    char* location_name;
    // if authentication token accquired.
    char *authToken;

}device_config_t;

void setDeviceId(device_config_t* , const char*);
void setLocationId(device_config_t* , const int);
void setAuthToken(device_config_t* , const char*);
void setMacAddr(device_config_t* , const uint8_t*);
const char* getDeviceId(device_config_t*);
const int getLocationId(device_config_t*);
const char* getAuthToken(device_config_t*);
const char* getMacAddr(device_config_t*);

void deviceConfigDestroy(device_config_t*);