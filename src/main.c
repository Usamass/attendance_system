#include <stdio.h>
#include <string.h>
#include <esp_eth_enc28j60.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_chip_info.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "SWH_RGB.h"
#include "SWH_ETH.h"
#include "../event_bits.h"
#include "../SWH_eventGroups.h"
#include "device_configs.h"
#include "swh_file_system.h"

// static const char *ETH_TAG = "SWH_eth_test";
// static const char *HTTP_SERVER_TAG = "SWH_HTTP_TEST";
extern esp_eth_handle_t eth_handle;

#define ETHERNET_CONNECT_FLAG 12
#define ETHERNET_DISCONNECT_FLAG 13
#define GOT_IP_FLAG 14

// ------------- include all the notifications msgs in this struct--------
typedef struct
{
    int val;
    char* from;
    char* msg;

} NOTIFIER;
//------------------------------------------------------------------------

// esp_err_t authenticateMe();
esp_err_t getStudentsData(device_config_t);

QueueHandle_t mailBox;

#define EXAMPLE_ESP_MAXIMUM_RETRY (5)
#define EXAMPLE_HTTP_QUERY_KEY_MAX_LEN (64)
#define MAX_HTTP_OUTPUT_BUFFER (1000)
#define MIN(a,b) (((a)<(b))?(a):(b))

static const char *HTTP_CLIENT_TAG = "http client";
static const char *MAILBOX_TAG = "mailbox";
static const char *TAG_exe = "spiffs";
// static const char *JSON_TAG = "JSON";
// ------------------------------------ ESP_HTTP_CLIENT_EVENT_HANDLER---------------------------

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer; // Buffer to store response of http request from event handler
    static int output_len;      // Stores number of bytes read
    switch (evt->event_id)
    {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            ESP_LOGI(HTTP_CLIENT_TAG , "HTTP_RESPONSE_DATA: %s \n" , (char*)evt->data);
            /*
            *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
            *  However, event handler can also be used in case chunked encoding is used.
            */
            if (!esp_http_client_is_chunked_response(evt->client))
            {
                // If user_data buffer is configured, copy the response into the buffer
                int copy_len = 0;
                if (evt->user_data)
                {
                    copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                    if (copy_len)
                    {
                        memcpy(evt->user_data + output_len, evt->data, copy_len);
                    }
                }
                else
                {
                    const int buffer_len = esp_http_client_get_content_length(evt->client);
                    if (output_buffer == NULL)
                    {
                        output_buffer = (char *)malloc(buffer_len);
                        output_len = 0;
                        if (output_buffer == NULL)
                        {
                            ESP_LOGE(HTTP_CLIENT_TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    copy_len = MIN(evt->data_len, (buffer_len - output_len));
                    if (copy_len)
                    {
                        memcpy(output_buffer + output_len, evt->data, copy_len);
                    }
                }
                output_len += copy_len;
            }

        break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL)
            {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
        break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_DISCONNECTED");

            if (output_buffer != NULL)
            {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
        break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_EVENT_REDIRECT");
            esp_http_client_set_header(evt->client, "From", "user@example.com");
            esp_http_client_set_header(evt->client, "Accept", "text/html");
            esp_http_client_set_redirection(evt->client);
        break;
    }
    return ESP_OK;
}
//------------------------------This task will notify network status------------------------------------------------------

static void networkStatusTask(void *pvParameter)
{
    BaseType_t mailBox_status;
    NOTIFIER noti;
     device_config_t dConfig = {
        .device_id = "LRO_1",
        .authToken = "asldkhoiawe34",
        .location_id = 2,
        .location_name = "lahore"
    };

    while (true){
        EventBits_t connectBits = xEventGroupWaitBits(
            swh_ethernet_event_group,
            ETHERNET_CONNECTED_BIT | ETHERNET_DISCONNECT_BIT | GOT_IP_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);
        if ((connectBits & GOT_IP_BIT) != 0){
            ESP_LOGI(HTTP_CLIENT_TAG, "initializting client\n");
            noti.val = GOT_IP_FLAG;
            noti.msg = "got ip address";
            // server_initiation();
            //authenticateMe();
            getStudentsData(dConfig);

            mailBox_status = xQueueSend(mailBox, &noti, portMAX_DELAY);
            if (mailBox_status != pdPASS){
                ESP_LOGI(MAILBOX_TAG, "Could not send to the queue \n");
            }
        }
        else if((connectBits & ETHERNET_CONNECTED_BIT) != 0){
            noti.val = ETHERNET_CONNECT_FLAG;
            noti.msg = "Ethernet connected!";
            ESP_LOGI("SWH_ethernet", "Ethernet connected\n");
       
            mailBox_status = xQueueSend(mailBox, &noti, portMAX_DELAY);
            if (mailBox_status != pdPASS){
                ESP_LOGI(MAILBOX_TAG, "Could not send to the queue \n");
            }

        }
        else {
            noti.val = ETHERNET_DISCONNECT_FLAG;
            noti.msg = "Ethernet disconnected!";
            ESP_LOGI("SWH_ethernet", "Ethernet disconnected\n");
       
            mailBox_status = xQueueSend(mailBox, &noti, portMAX_DELAY);
            if (mailBox_status != pdPASS){
                ESP_LOGI(MAILBOX_TAG, "Could not send to the queue \n");
            }

        }
    }
}

//-------------------------------------This task will display the notifications to LCD ----------------------------

static void displayNotification() // This task will wait for the data appeare in mainbox queue
{
    BaseType_t mailBox_status;
    NOTIFIER noti;

    while (true)
    {
        mailBox_status = xQueueReceive(mailBox, &noti, portMAX_DELAY);
        if (mailBox_status == pdPASS)
        {
            ESP_LOGI(MAILBOX_TAG, "data reveived from mailbox : value = %d \t msg = %s \n", noti.val, noti.msg);
        }
        else
        {
            ESP_LOGI(MAILBOX_TAG, "could not receive from the mailbox \n");
        }
    }
}
// This notifier Task will notify all the activity on the device to lcd display as notifications.
static void notifierTask()
{
    NOTIFIER noti = {
        .val = 10,
        .msg = "from notifier task"};
    BaseType_t mailbox_status;

    while (true)
    {

        vTaskDelay(pdMS_TO_TICKS(500));
        mailbox_status = xQueueSend(mailBox, &noti, portMAX_DELAY);
        if (mailbox_status != pdPASS)
        {
            ESP_LOGI(MAILBOX_TAG, "Could not send to the queue \n");
        }
    }
}

void app_main(void)
{
    // init(); // this function will initialize all the configs on device.
    rgbConfig();
    swh_eth_init(); // initializing ethernet hardware.
    swh_file_system_init();  // initializing file system.
   

    printf("Event group handle in main: %p\n", swh_ethernet_event_group);
    xTaskCreate(networkStatusTask, "network status task", 4000, NULL, 1, NULL);
    mailBox = xQueueCreate(1, sizeof(NOTIFIER)); // creating mailbox with 1 NOTIFIER space.

    if (mailBox != NULL)
    {
        ESP_LOGI(MAILBOX_TAG, "mailbox has been created!\n");
        xTaskCreate(notifierTask, "notifier-task", 2048, NULL, 1, NULL);
        xTaskCreate(displayNotification, "display-notification", 2048, NULL, 4, NULL);
    }
    else
    {

        ESP_LOGI(MAILBOX_TAG, "mailbox could not created \n");
    }

}

esp_err_t getStudentsData(device_config_t dConfig)
{
    const int locationID = getLocationId(&dConfig);
    const char* auth = getAuthToken(&dConfig);

    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER];
    esp_err_t err;
    // making query string.
    char queryString[13] = "location_id=";
    queryString[12] = '0' + locationID;

       esp_http_client_config_t config = {
        .host = "192.168.50.209:8000",
        .path = "/api/students",
        .query = queryString,
        .method = HTTP_METHOD_GET,
        .event_handler = _http_event_handler,
        .user_data = local_response_buffer,        // Pass address of local buffer to get response
        .disable_auto_redirect = true,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client , "token" , auth);
   
    // GET
    err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(HTTP_CLIENT_TAG, "HTTP GET Status = %d, content_length = %"PRIu64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(HTTP_CLIENT_TAG, "HTTP GET request failed: %s", esp_err_to_name(err));

        return ESP_FAIL;
        
    }
    ESP_LOG_BUFFER_HEX(HTTP_CLIENT_TAG, local_response_buffer, strlen(local_response_buffer));

    return ESP_OK;
}
