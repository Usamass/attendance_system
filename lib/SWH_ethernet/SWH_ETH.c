#include <stdio.h>
#include <string.h>
#include <esp_eth_enc28j60.h>
#include "SWH_ETH.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_chip_info.h"
#include "../event_bits.h"
#include "SWH_RGB.h"
#include "device_configs.h"

EventGroupHandle_t swh_ethernet_event_group;
esp_eth_handle_t eth_handle;
device_config_t dConfig;    // dConfig defined here first.
uint8_t is_ethernet_connected = false;

static char* ETH_TAG = "SWH_ethernet";
/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        is_ethernet_connected = true;
        xEventGroupSetBits(swh_ethernet_event_group , ETHERNET_CONNECTED_BIT);
        setRGBLED(1 , 1 , 0);
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(ETH_TAG, "Ethernet Link Up: %d" , is_ethernet_connected);
        ESP_LOGI(ETH_TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        setMacAddr(&dConfig , mac_addr);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        is_ethernet_connected = false;
        ESP_LOGI(ETH_TAG, "Ethernet Link Down: %d" , is_ethernet_connected);
        xEventGroupSetBits(swh_ethernet_event_group , ETHERNET_DISCONNECT_BIT);
        setRGBLED(0 , 1 , 1);
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(ETH_TAG, "Ethernet Started");
        //setRGBLED(1 , 1 , 0);
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(ETH_TAG, "Ethernet Stopped");
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

    ESP_LOGI(ETH_TAG, "Ethernet Got IP Address");
    ESP_LOGI(ETH_TAG, "~~~~~~~~~~~");
    ESP_LOGI(ETH_TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(ETH_TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(ETH_TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(ETH_TAG, "~~~~~~~~~~~");
    setIpAddr(&dConfig , &event->ip_info);
    setRGBLED(1 , 0 , 1);
    xEventGroupSetBits(swh_ethernet_event_group , GOT_IP_BIT);
}

void swh_eth_init()
{
    uint8_t dma_channel = 1;
    swh_ethernet_event_group = xEventGroupCreate();
    xEventGroupClearBits(swh_ethernet_event_group , ETHERNET_CONNECTED_BIT | 
    ETHERNET_DISCONNECT_BIT | 
    GOT_IP_BIT);

    // <--------------------------------------------------------- Ethernet hardware Configs-------------------------------------------------->
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
    ESP_ERROR_CHECK(spi_bus_initialize(ENC_SPI_HOST, &buscfg, (spi_dma_chan_t)dma_channel));
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

    if (emac_enc28j60_get_chip_info(mac) < ENC28J60_REV_B5 && ENC_SPI_CLOCK_MHZ < 8) {
        ESP_LOGE(ETH_TAG, "SPI frequency must be at least 8 MHz for chip revision less than 5");
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    /* attach Ethernet driver to TCP/IP stack */
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    /* It is recommended to use ENC28J60 in Full Duplex mode since multiple errata exist to the Half Duplex mode */
#ifdef CONFIG_ENC28J60_DUPLEX_FULL
    eth_duplex_t duplex = ETH_DUPLEX_FULL;
    ESP_ERROR_CHECK(esp_eth_ioctl(eth_handle, ETH_CMD_S_DUPLEX_MODE, &duplex));
#endif

    /* start Ethernet driver state machine */
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));







}
