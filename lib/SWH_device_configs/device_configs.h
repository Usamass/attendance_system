// All device configurations are managed here

#include <inttypes.h>
#include "esp_netif.h"

#define MAX_IP_LEN 18
#define MAC_MAC_LEN 18

#ifndef _DEVICE_CONF_H_

#define _DEVICE_CONF_H_

typedef struct
{
    char* device_id;
    char* MAC_addr;
    char* IP_addr;
    int   location_id;
    char* location_name;
    // if authentication token accquired.
    char *authToken;

}device_config_t;

void setDeviceId(device_config_t* , const char*);
void setLocationId(device_config_t* , const int);
void setDeviceLocation(device_config_t* , const char*);
void setAuthToken(device_config_t* , const char*);
void setMacAddr(device_config_t* , uint8_t*);
void setIpAddr(device_config_t* , const esp_netif_ip_info_t*);
const char* getDeviceId(device_config_t*);
int getLocationId(device_config_t*);
const char* getDeviceLocation(device_config_t*);
const char* getAuthToken(device_config_t*);
const char* getMacAddr(device_config_t*);
const char* getIpAddr(device_config_t*);

void deviceConfigDestroy(device_config_t*);

#endif /* _DEVICE_CONF_H_ */
