#include <stdio.h>
#include <string.h>
#include <esp_eth_enc28j60.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_chip_info.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "SWH_RGB.h"
#include "SWH_ETH.h"
#include "../event_bits.h"
#include "../SWH_eventGroups.h"

static const char *ETH_TAG = "SWH_eth_test";
static const char *HTTP_SERVER_TAG = "SWH_HTTP_TEST";

// void server_initiation();

// #define ENC_MOSI_PIN GPIO_NUM_13
// #define ENC_MISO_PIN GPIO_NUM_12
// #define ENC_CLK_PIN  GPIO_NUM_14
// #define ENC_CS_PIN   GPIO_NUM_15
// #define ENC_INT_PIN  GPIO_NUM_4
// #define BUZZER_PIN GPIO_NUM_22
// #define ENC_SPI_CLOCK_MHZ (8)
// #define CONFIG_ENC28J60_DUPLEX_FULL
// #define HTTP_QUERY_KEY_MAX_LEN  (64)


// #define ENC_SPI_HOST SPI2_HOST

// /** Event handler for Ethernet events */
// static void eth_event_handler(void *arg, esp_event_base_t event_base,
//                               int32_t event_id, void *event_data)
// {
//     uint8_t mac_addr[6] = {0};
//     /* we can get the ethernet driver handle from event data */
//     esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

//     switch (event_id) {
//     case ETHERNET_EVENT_CONNECTED:
//         setRGBLED(1 , 1 , 0);
//         esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
//         ESP_LOGI(ETH_TAG, "Ethernet Link Up");
//         ESP_LOGI(ETH_TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
//                  mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
//         break;
//     case ETHERNET_EVENT_DISCONNECTED:
//         ESP_LOGI(ETH_TAG, "Ethernet Link Down");
//         setRGBLED(0 , 1 , 1);
//         break;
//     case ETHERNET_EVENT_START:
//         ESP_LOGI(ETH_TAG, "Ethernet Started");
//         //setRGBLED(1 , 1 , 0);
//         break;
//     case ETHERNET_EVENT_STOP:
//         ESP_LOGI(ETH_TAG, "Ethernet Stopped");
//         break;
//     default:
//         break;
//     }
// }

// /** Event handler for IP_EVENT_ETH_GOT_IP */
// static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
//                                  int32_t event_id, void *event_data)
// {
//     ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
//     const esp_netif_ip_info_t *ip_info = &event->ip_info;

//     ESP_LOGI(ETH_TAG, "Ethernet Got IP Address");
//     ESP_LOGI(ETH_TAG, "~~~~~~~~~~~");
//     ESP_LOGI(ETH_TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
//     ESP_LOGI(ETH_TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
//     ESP_LOGI(ETH_TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
//     ESP_LOGI(ETH_TAG, "~~~~~~~~~~~");
//     setRGBLED(1 , 0 , 1);
//     server_initiation();
// }

void app_main(void)
{
    
    rgbConfig(); // configuring rgb leds
    setRGBLED(0 , 1 , 1); // set Red led by default in beginning
    gpio_reset_pin(BUZZER_PIN); // resetting buzzer pin
    gpio_set_direction(BUZZER_PIN , GPIO_MODE_OUTPUT);  // setting buzzer pin as an output

    swh_eth_init();
// <--------------------------------------------------------- Ethernet hardware Configs-------------------------------------------------->
//     ESP_ERROR_CHECK(gpio_install_isr_service(0));
//     // Initialize TCP/IP network interface (should be called only once in application)
//     ESP_ERROR_CHECK(esp_netif_init());
//     // Create default event loop that running in background
//     ESP_ERROR_CHECK(esp_event_loop_create_default());
//     esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
//     esp_netif_t *eth_netif = esp_netif_new(&netif_cfg);

//     spi_bus_config_t buscfg = {
//         .miso_io_num = ENC_MISO_PIN,
//         .mosi_io_num = ENC_MOSI_PIN,
//         .sclk_io_num = ENC_CLK_PIN,
//         .quadwp_io_num = -1,
//         .quadhd_io_num = -1,
//     };
//     ESP_ERROR_CHECK(spi_bus_initialize(ENC_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
//     /* ENC28J60 ethernet driver is based on spi driver */
//     spi_device_interface_config_t spi_devcfg = {
//         .mode = 0,
//         .clock_speed_hz = ENC_SPI_CLOCK_MHZ * 1000 * 1000,
//         .spics_io_num = ENC_CS_PIN,
//         .queue_size = 20,
//         .cs_ena_posttrans = enc28j60_cal_spi_cs_hold_time(ENC_SPI_CLOCK_MHZ),
//     };

//     eth_enc28j60_config_t enc28j60_config = ETH_ENC28J60_DEFAULT_CONFIG(ENC_SPI_HOST, &spi_devcfg);
//     enc28j60_config.int_gpio_num = ENC_INT_PIN;

//     eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
//     esp_eth_mac_t *mac = esp_eth_mac_new_enc28j60(&enc28j60_config, &mac_config);

//     eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
//     phy_config.autonego_timeout_ms = 0; // ENC28J60 doesn't support auto-negotiation
//     phy_config.reset_gpio_num = -1; // ENC28J60 doesn't have a pin to reset internal PHY
//     esp_eth_phy_t *phy = esp_eth_phy_new_enc28j60(&phy_config);

//     esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
//     esp_eth_handle_t eth_handle = NULL;
//     ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &eth_handle));

//     /* ENC28J60 doesn't burn any factory MAC address, we need to set it manually.
//        02:00:00 is a Locally Administered OUI range so should not be used except when testing on a LAN under your control.
//     */
//     mac->set_addr(mac, (uint8_t[]) {
//         0x02, 0x00, 0x00, 0x12, 0x34, 0x56
//     });

//     if (emac_enc28j60_get_chip_info(mac) < ENC28J60_REV_B5 && ENC_SPI_CLOCK_MHZ < 8) {
//         ESP_LOGE(ETH_TAG, "SPI frequency must be at least 8 MHz for chip revision less than 5");
//         ESP_ERROR_CHECK(ESP_FAIL);
//     }

//     /* attach Ethernet driver to TCP/IP stack */
//     ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
//     // Register user defined event handers
//     ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
//     ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

//     /* It is recommended to use ENC28J60 in Full Duplex mode since multiple errata exist to the Half Duplex mode */
// #ifdef CONFIG_ENC28J60_DUPLEX_FULL
//     eth_duplex_t duplex = ETH_DUPLEX_FULL;
//     ESP_ERROR_CHECK(esp_eth_ioctl(eth_handle, ETH_CMD_S_DUPLEX_MODE, &duplex));
// #endif

//     /* start Ethernet driver state machine */
//     ESP_ERROR_CHECK(esp_eth_start(eth_handle));
}




































// // <----------------------------------------------- Handler function for http uri's------------------------------------------------------------------------->

// static esp_err_t led_color_handler(httpd_req_t *req)
// {
//     char* buff;
//     size_t buff_len;
//     uint8_t red = 1;
//     uint8_t green = 1;
//     uint8_t blue = 1;



//     buff_len = httpd_req_get_url_query_len(req) +1;
//     if (buff_len > 1){
//         buff = malloc(buff_len);
//         if (httpd_req_get_url_query_str(req , buff , buff_len) == ESP_OK){
//             ESP_LOGI(HTTP_SERVER_TAG , "Found URL query => %s" , buff);
//             char param[HTTP_QUERY_KEY_MAX_LEN]; /*dec_param[EXAMPLE_HTTP_QUERY_KEY_MAX_LEN] = {0};*/
//             // Get value of expected key from query string 

//             if (httpd_query_key_value(buff , "red" , param , sizeof(param)) == ESP_OK){
//                 red = param[0] - '0';

//                 ESP_LOGI(HTTP_SERVER_TAG , "Found URL first query parameter => red parameter=%s \t red val=%d" , param , red);
                
//             }
//             if (httpd_query_key_value(buff , "green" , param , sizeof(param)) == ESP_OK){
//                 green = param[0]- '0';
//                 ESP_LOGI(HTTP_SERVER_TAG , "Found URL first query parameter => green parameter=%s \t green val=%d" , param , green);

                
//             }
//             if (httpd_query_key_value(buff , "blue" , param , sizeof(param)) == ESP_OK){
//                 blue = param[0]- '0';
//                 ESP_LOGI(HTTP_SERVER_TAG , "Found URL first query parameter => blue parameter=%s \t blue val=%d" , param , blue);

                
//             }


//         }
//         if (red == 0){
//             const char* resp = "red led turned on!";
//             httpd_resp_send(req , resp , HTTPD_RESP_USE_STRLEN);
//         }
//         else if(green == 0){
//             const char* resp = "green led turned on!";
//             httpd_resp_send(req , resp , HTTPD_RESP_USE_STRLEN);

//         }
//         else if(blue == 0){
//             const char* resp = "blue led turned on!";
//             httpd_resp_send(req , resp , HTTPD_RESP_USE_STRLEN);

//         }
//         else{
//             const char* resp = "leds turned off!";
//             httpd_resp_send(req , resp , HTTPD_RESP_USE_STRLEN);


//         }
//         setRGBLED(red , green , blue);

//     }


  
//     return ESP_OK;
// }

// static esp_err_t off_handler(httpd_req_t *req)
// {
//     setRGBLED(1 , 1 , 1);
//     httpd_resp_set_type(req , "application/json");
//     const char *post_data = "{\"name\":\"osama_shafique\"}";    
//     httpd_resp_send(req, post_data, HTTPD_RESP_USE_STRLEN);
    
//     return ESP_OK;
// }
// static esp_err_t buzzer_on(httpd_req_t *req)
// {
//     gpio_set_level(BUZZER_PIN , 1);
//     const char* resp = "Buzzer turned on!";
//     httpd_resp_send(req , resp , HTTPD_RESP_USE_STRLEN);

//     return ESP_OK;    

// }
// static esp_err_t buzzer_off(httpd_req_t *req)
// {
//     gpio_set_level(BUZZER_PIN , 0);
//     const char* resp = "Buzzer turned off!";
//     httpd_resp_send(req , resp , HTTPD_RESP_USE_STRLEN);
    
//     return ESP_OK;    


// }
// static esp_err_t get_system_info(httpd_req_t* req)
// {
//     httpd_resp_set_type(req , "application/json");  // setting content type of the response

//     cJSON* root = cJSON_CreateObject(); // creating a json object
//     // getting esp chip info
//     esp_chip_info_t myChipInfo;
//     esp_chip_info(&myChipInfo);
//     // adding element to json object
//     cJSON_AddStringToObject(root , "version" , IDF_VER);
//     cJSON_AddNumberToObject(root , "cores" , myChipInfo.cores);
//     // parsing it to string
//     const char* jsonString = cJSON_Print(root);
//     // sending the jsonString as a response
//     httpd_resp_sendstr(req , jsonString);

//     free((void*)jsonString);

//     return ESP_OK;    

// }
// static esp_err_t get_post_data(httpd_req_t* req)
// {
//     const int buff_len = 100;
//     char buff[100];
//     int ret;
//     // getting content length of the request.
//     int remaining = req->content_len;

//     while (remaining > 0){
//         if ((ret = httpd_req_recv(req , buff , MIN(remaining , buff_len))) <= 0){
//             if (ret == HTTPD_SOCK_ERR_TIMEOUT){
//                 continue;
//             }
//             return ESP_FAIL;
//         }
//     }
//     buff[buff_len] = '\0';

//     ESP_LOGI(HTTP_SERVER_TAG , "-----RECEIVED DATA------\n");
//     ESP_LOGI(HTTP_SERVER_TAG , "http data :%s \n ", buff);
//     ESP_LOGI(HTTP_SERVER_TAG , "--------------------------\n");

//     httpd_resp_send(req , buff , buff_len);
//     return ESP_OK;

// }

// //<---------------------------------------------- Server initialization with default configuration-------------------------------------------------------------->

// void server_initiation()
// {
//     httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
//     httpd_handle_t server_handle = NULL;
//     ESP_LOGI(HTTP_SERVER_TAG, "Starting server on port: '%d'", server_config.server_port);

//     if (httpd_start(&server_handle, &server_config) == ESP_OK){
//         httpd_uri_t uri_led_color = {
//             .uri = "/led",
//             .method = HTTP_GET,
//             .handler = led_color_handler,
//             .user_ctx = NULL
//         };

//         httpd_register_uri_handler(server_handle, &uri_led_color);

//         httpd_uri_t uri_off = {
//         .uri = "/off",
//         .method = HTTP_GET,
//         .handler = off_handler,
//         .user_ctx = NULL
//         };

//         httpd_register_uri_handler(server_handle, &uri_off);


//         httpd_uri_t uri_buzzer_on = {
//             .uri = "/buzzer/on",
//             .method = HTTP_GET,
//             .handler = buzzer_on,
//             .user_ctx = NULL
//         };

//         httpd_register_uri_handler(server_handle , &uri_buzzer_on);


//         httpd_uri_t uri_buzzer_off = {
//             .uri = "/buzzer/off",
//             .method = HTTP_GET,
//             .handler = buzzer_off,
//             .user_ctx = NULL
//         };

//         httpd_register_uri_handler(server_handle , &uri_buzzer_off);

//         httpd_uri_t uri_system_info = {
//             .uri = "/GetSystemInfo",
//             .method = HTTP_GET,
//             .handler = get_system_info,
//             .user_ctx = NULL
//         };

//         httpd_register_uri_handler(server_handle , &uri_system_info);

//         httpd_uri_t uri_get_data = {
//             .uri = "/GetData",
//             .method = HTTP_POST,
//             .handler = get_post_data,
//             .user_ctx = NULL

//         };
        
//         httpd_register_uri_handler(server_handle , &uri_get_data);

//         // httpd_uri_t uri_put_hander = {
//         //     .uri = "/data",
//         //     .method = HTTP_PUT,
//         //     .handler = http_put_handler,
//         //     .user_ctx = NULL
//         // };
//         // httpd_register_uri_handler(server_handle , &uri_put_hander);


//     }
//     else{
//         ESP_LOGI(HTTP_SERVER_TAG, "Error starting server!");
//     }

// }
// int MIN(int buff_1 , int buff_2)
// {
//     if ((buff_1 - buff_2) <= 0)
//     return buff_1;
//     else 
//     return buff_2;

// }

