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

#define ENC_MOSI_PIN GPIO_NUM_13
#define ENC_MISO_PIN GPIO_NUM_12
#define ENC_CLK_PIN  GPIO_NUM_14
#define ENC_CS_PIN   GPIO_NUM_15
#define ENC_INT_PIN  GPIO_NUM_4
#define ENC_SPI_CLOCK_MHZ (8)

#define ENC_SPI_HOST SPI2_HOST

#define RED_LED_PIN GPIO_NUM_5
#define GREEN_LED_PIN GPIO_NUM_17
#define BLUE_LED_PIN GPIO_NUM_16
#define BUZZER_PIN GPIO_NUM_22

static const char *TAG = "SWH_eth_test";
static const char *HTTP_TAG = "SWH_HTTP_TEST";

void rgbConfig(void);  // rgb led configuration
void setRGBLED(uint8_t , uint8_t , uint8_t);  // setting individual leds
esp_err_t start_rest_server(void);
static esp_err_t hello_world_handler(httpd_req_t *req);

/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        setRGBLED(1 , 1 , 0);
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
        setRGBLED(0 , 1 , 1);
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        //setRGBLED(1 , 1 , 0);
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    setRGBLED(1 , 0 , 1);

    start_rest_server();
}

void app_main(void)
{
    
    rgbConfig(); // configuring rgb leds
    setRGBLED(0 , 1 , 1); // set Red led by default in beginning


    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    // Initialize TCP/IP network interface (should be called only once in application)
    ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&netif_cfg);

    spi_bus_config_t buscfg = {
        .miso_io_num = ENC_MISO_PIN,
        .mosi_io_num = ENC_MOSI_PIN,
        .sclk_io_num = ENC_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(ENC_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
    /* ENC28J60 ethernet driver is based on spi driver */
    spi_device_interface_config_t spi_devcfg = {
        .mode = 0,
        .clock_speed_hz = ENC_SPI_CLOCK_MHZ * 1000 * 1000,
        .spics_io_num = ENC_CS_PIN,
        .queue_size = 20,
        .cs_ena_posttrans = enc28j60_cal_spi_cs_hold_time(ENC_SPI_CLOCK_MHZ),
    };

    eth_enc28j60_config_t enc28j60_config = ETH_ENC28J60_DEFAULT_CONFIG(ENC_SPI_HOST, &spi_devcfg);
    enc28j60_config.int_gpio_num = ENC_INT_PIN;

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    esp_eth_mac_t *mac = esp_eth_mac_new_enc28j60(&enc28j60_config, &mac_config);

    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.autonego_timeout_ms = 0; // ENC28J60 doesn't support auto-negotiation
    phy_config.reset_gpio_num = -1; // ENC28J60 doesn't have a pin to reset internal PHY
    esp_eth_phy_t *phy = esp_eth_phy_new_enc28j60(&phy_config);

    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &eth_handle));

    /* ENC28J60 doesn't burn any factory MAC address, we need to set it manually.
       02:00:00 is a Locally Administered OUI range so should not be used except when testing on a LAN under your control.
    */
    mac->set_addr(mac, (uint8_t[]) {
        0x02, 0x00, 0x00, 0x12, 0x34, 0x56
    });

    // ENC28J60 Errata #1 check
    if (emac_enc28j60_get_chip_info(mac) < ENC28J60_REV_B5 && ENC_SPI_CLOCK_MHZ < 8) {
        ESP_LOGE(TAG, "SPI frequency must be at least 8 MHz for chip revision less than 5");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    /* attach Ethernet driver to TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    /* It is recommended to use ENC28J60 in Full Duplex mode since multiple errata exist to the Half Duplex mode */
#if CONFIG_EXAMPLE_ENC28J60_DUPLEX_FULL
    eth_duplex_t duplex = ETH_DUPLEX_FULL;
    ESP_ERROR_CHECK(esp_eth_ioctl(eth_handle, ETH_CMD_S_DUPLEX_MODE, &duplex));
#endif

    /* start Ethernet driver state machine */
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
    //vTaskDelay(pdMS_TO_TICKS(100));
}
void rgbConfig(void)
{
    // setting gpio mode of the rgb pins
    gpio_set_direction(RED_LED_PIN , GPIO_MODE_OUTPUT);
    gpio_set_direction(GREEN_LED_PIN , GPIO_MODE_OUTPUT);
    gpio_set_direction(BLUE_LED_PIN , GPIO_MODE_OUTPUT);

    gpio_set_level(RED_LED_PIN , 1);   
    gpio_set_level(GREEN_LED_PIN , 1); 
    gpio_set_level(BLUE_LED_PIN , 1); 


}

void setRGBLED(uint8_t red , uint8_t green , uint8_t blue)
{ 
    gpio_set_level(RED_LED_PIN , red);
    gpio_set_level(GREEN_LED_PIN , green);
    gpio_set_level(BLUE_LED_PIN , blue);


}
esp_err_t start_rest_server(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(HTTP_TAG, "Starting HTTP Server");
    ESP_ERROR_CHECK(httpd_start(&server, &config));

    /* URI handler for fetching system info */
    httpd_uri_t hello_world_get_uri = {
        .uri = "/hello",
        .method = HTTP_GET,
        .handler = hello_world_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &hello_world_get_uri);
    return ESP_OK;
}


    /* URI handler for fetching temperature data */
//     httpd_uri_t temperature_data_get_uri = {
//         .uri = "/api/v1/temp/raw",
//         .method = HTTP_GET,
//         .handler = temperature_data_get_handler,
//         .user_ctx = rest_context
//     };
//     httpd_register_uri_handler(server, &temperature_data_get_uri);

//     /* URI handler for light brightness control */
//     httpd_uri_t light_brightness_post_uri = {
//         .uri = "/api/v1/light/brightness",
//         .method = HTTP_POST,
//         .handler = light_brightness_post_handler,
//         .user_ctx = rest_context
//     };
//     httpd_register_uri_handler(server, &light_brightness_post_uri);

//     /* URI handler for getting web server files */
//     httpd_uri_t common_get_uri = {
//         .uri = "/*",
//         .method = HTTP_GET,
//         .handler = rest_common_get_handler,
//         .user_ctx = rest_context
//     };
//     httpd_register_uri_handler(server, &common_get_uri);
// err_start:
//     free(rest_context);
// err:
//     return ESP_FAIL;

static esp_err_t hello_world_handler(httpd_req_t *req)
{
    httpd_resp_set_status(req , HTTPD_200);
    httpd_resp_set_type(req , "application/json");

    httpd_resp_sendstr(req , "{\"data\":hello_world\"}");

    return ESP_OK;
}