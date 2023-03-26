#include "device_configs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void setDeviceId(device_config_t* dConfig, const char* d_id)
{
    int id_len = strlen(d_id);

    dConfig->device_id = (char*)malloc(id_len * sizeof(char));
    memcpy(dConfig->device_id , d_id , id_len +1);


}

const char* getDeviceId(device_config_t* dConfig)
{
    return dConfig->device_id; 

}

void setLocationId(device_config_t* dConfig , const int l_id)
{
    dConfig->location_id = l_id;
    
}

const int getLocationId(device_config_t* dConfig)
{


    return dConfig->location_id;
}

void setAuthToken(device_config_t* dConfig , const char* token)
{
    int token_len = strlen(token);

    dConfig->authToken = (char*)malloc(token_len * sizeof(char));
    memcpy(dConfig->authToken , token , token_len +1);
    
}

const char* getAuthToken(device_config_t* dConfig)
{
    return dConfig->authToken;

}
void setMacAddr(device_config_t* dConfig , const uint8_t* mac_8t)
{
    const uint8_t MAC_SIZE = 18;
    char* macStr = (char*)malloc(MAC_SIZE * sizeof(char));

    snprintf(macStr , MAC_SIZE , "%02X:%02X:%02X:%02X:%02X:%02X", 
    mac_8t[0], mac_8t[1], 
    mac_8t[2], mac_8t[3], 
    mac_8t[4], mac_8t[5]);
    
    dConfig->MAC_addr = macStr;
}

const char* getMacAddr(device_config_t* dConfig)
{
    return dConfig->MAC_addr;

}

void deviceConfigDestroy(device_config_t* dConfig)
{
    free(dConfig->authToken);
    free(dConfig->MAC_addr);
    free(dConfig->location_name);
    free(dConfig->device_id);
}

