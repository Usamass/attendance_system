#include "device_configs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "esp_eth.h"
#include "esp_netif.h"
#include "esp_log.h"

static const char* DEVICE_CONF_TAG = "device configs";


extern esp_eth_handle_t eth_handle; // ethernet handler for getting mac addr in this file.

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

int getLocationId(device_config_t* dConfig)
{


    return dConfig->location_id;
}

void setDeviceLocation(device_config_t* dConfig , const char* l_name)
{
    int server_address_len = strlen(l_name);

    dConfig->server_address = (char*)malloc(server_address_len * sizeof(char));
    memcpy(dConfig->server_address , l_name , server_address_len +1);}

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

const char* getMacAddr(device_config_t* dConfig)
{
    return dConfig->MAC_addr;

}
void setMacAddr(device_config_t* dConfig , uint8_t* mac_addr)
{
    esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);

    // const uint8_t MAC_SIZE = 18;
    char* macStr = (char*)malloc(MAC_MAC_LEN * sizeof(char));

    snprintf(macStr , MAC_MAC_LEN , "%02X:%02X:%02X:%02X:%02X:%02X", 
    mac_addr[0], mac_addr[1], 
    mac_addr[2], mac_addr[3], 
    mac_addr[4], mac_addr[5]);
    
    dConfig->MAC_addr = macStr; 
    ESP_LOGI(DEVICE_CONF_TAG , "device config mac %s" , dConfig->MAC_addr);
}

void setIpAddr(device_config_t* dConfig , const esp_netif_ip_info_t *ip_info)
{
    dConfig->IP_addr = (char*)malloc(MAX_IP_LEN * sizeof(char));

    sprintf(dConfig->IP_addr , IPSTR , IP2STR(&ip_info->ip));
    ESP_LOGI(DEVICE_CONF_TAG , "device config ip %s" , dConfig->IP_addr);
}

const char* getIpAddr(device_config_t* dConfig)
{
    return dConfig->IP_addr;

}

void deviceConfigDestroy(device_config_t* dConfig)
{
    free(dConfig->authToken);
    free(dConfig->MAC_addr);
    free(dConfig->IP_addr);
    free(dConfig->server_address);
    free(dConfig->device_id);
}

