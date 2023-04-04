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
#include "swh_server.h"
#include "../event_bits.h"
#include "../SWH_eventGroups.h"
#include "../SWH_custom_data_structs.h"
#include "../SWH_event_flags.h"
#include "../SWH_data_buffers.h"
#include "device_configs.h"
#include "swh_file_system.h"


// static const char *ETH_TAG = "SWH_eth_test";
// static const char *HTTP_SERVER_TAG = "SWH_HTTP_TEST";
// esp_err_t getStudentsData(device_config_t);

QueueHandle_t mailBox;
QueueHandle_t spiffs_mailBox;
extern device_config_t dConfig; // device ip and mac will be set on connect to network
EventGroupHandle_t spiffs_event_group;
  
#define EXAMPLE_ESP_MAXIMUM_RETRY (5)
#define EXAMPLE_HTTP_QUERY_KEY_MAX_LEN (64)
#define MAX_HTTP_OUTPUT_BUFFER (1000)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

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
        ESP_LOGI(HTTP_CLIENT_TAG, "HTTP_RESPONSE_DATA: %s \n", (char *)evt->data);
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
    while (true)
    {
        EventBits_t connectBits = xEventGroupWaitBits(
            swh_ethernet_event_group,
            ETHERNET_CONNECTED_BIT | ETHERNET_DISCONNECT_BIT | GOT_IP_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);
        if ((connectBits & GOT_IP_BIT) != 0)
        {
            ESP_LOGI(HTTP_CLIENT_TAG, "initializting client\n");
            noti.val = GOT_IP_FLAG;
            noti.msg = "got ip address";
            swh_server_init();

            // getStudentsData(dConfig);

            mailBox_status = xQueueSend(mailBox, &noti, portMAX_DELAY);
            if (mailBox_status != pdPASS)
            {
                ESP_LOGI(MAILBOX_TAG, "Could not send to the queue \n");
            }
        }
        else if ((connectBits & ETHERNET_CONNECTED_BIT) != 0)
        {
            noti.val = ETHERNET_CONNECT_FLAG;
            noti.msg = "Ethernet connected!";
            ESP_LOGI("SWH_ethernet", "Ethernet connected\n");

            mailBox_status = xQueueSend(mailBox, &noti, portMAX_DELAY);
            if (mailBox_status != pdPASS)
            {
                ESP_LOGI(MAILBOX_TAG, "Could not send to the queue \n");
            }
        }
        else
        {
            noti.val = ETHERNET_DISCONNECT_FLAG;
            noti.msg = "Ethernet disconnected!";
            ESP_LOGI("SWH_ethernet", "Ethernet disconnected\n");

            mailBox_status = xQueueSend(mailBox, &noti, portMAX_DELAY);
            if (mailBox_status != pdPASS)
            {
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

static void db_interface_task()
{
    BaseType_t mailBox_status;

    SPIFFS_NOTIFIER spiffs_noti;
    while (true)
    {
        EventBits_t requestBits = xEventGroupWaitBits(
            spiffs_event_group,
            CLIENT_RECIEVED_BIT | DEVICE_CONFIG_BIT, // add other event bit as the application grows.
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);
        if ((requestBits & CLIENT_RECIEVED_BIT) != 0)
        {
            DataSource_t dataSrc = CLIENT;
            spiffs_noti.data_scr = dataSrc;
            spiffs_noti.flag_type = CLIENT_WRITE_FLAG;
            spiffs_noti.data = client_receive_buffer;

            mailBox_status = xQueueSend(spiffs_mailBox, &spiffs_noti, portMAX_DELAY);
            if (mailBox_status != pdPASS)
            {
                ESP_LOGI(MAILBOX_TAG, "Could not send to the queue \n");
            }
        }
        else if ((requestBits & DEVICE_CONFIG_BIT) != 0)
        {
            // before sending it must convert it into json format.
            DataSource_t dataSrc = DEVICE_CONFIGS;
            spiffs_noti.data_scr = dataSrc;
            spiffs_noti.flag_type = DEVICE_CONFIG_WRITE_FLAG;
            spiffs_noti.data = device_config_buffer;

            mailBox_status = xQueueSend(spiffs_mailBox, &spiffs_noti, portMAX_DELAY);
            if (mailBox_status != pdPASS)
            {
                ESP_LOGI(MAILBOX_TAG, "Could not send to the queue \n");
            }
        }
    }
}

static void spiffs_task()
{
    BaseType_t mailBox_status;
    SPIFFS_NOTIFIER spiffs_noti;

    while (true)
    {
        mailBox_status = xQueueReceive(spiffs_mailBox, &spiffs_noti, portMAX_DELAY);
        if (mailBox_status == pdPASS)
        {
            ESP_LOGI(MAILBOX_TAG, "data received form : %d \t data = %s \n", spiffs_noti.data_scr, spiffs_noti.data);
            if (spiffs_noti.data_scr == CLIENT)
            {
                if (spiffs_noti.flag_type == CLIENT_WRITE_FLAG)
                {
                    if (spiffs_noti.data != NULL)
                    {
                        ESP_LOGI(TAG_exe, "Opening stdData.txt file");
                        FILE *f = fopen("/spiffs/stdData.txt", "w");
                        if (f == NULL)
                        {
                            ESP_LOGE(TAG_exe, "Failed to open file for writing");
                        }
                        fprintf(f, spiffs_noti.data);
                        fclose(f);
                        free(spiffs_noti.data);
                        ESP_LOGI(TAG_exe, "File written");
                    }
                }
                else
                {
                    ESP_LOGI(TAG_exe, "Reading from stdData.txt file");
                    FILE *f = fopen("/spiffs/stdData.txt", "r");
                    if (f == NULL)
                    {
                        ESP_LOGE(TAG_exe, "Failed to open file for reading");
                        return;
                    }
                    fseek(f, 0L, SEEK_END); // move file pointer to end of file
                    long size = ftell(f);   // get file size
                    fseek(f, 0L, SEEK_SET); // move file pointer back to beginning of file

                    char *data = (char *)malloc(size); // allocate memory for buffer
                    if (data == NULL)
                    {
                        ESP_LOGI(TAG_exe, "Error allocating memory.\n");
                        fclose(f);
                        return;
                    }

                    size_t result = fread(data, 1, size, f); // read file into buffer
                    if (result != size)
                    {
                        ESP_LOGI(TAG_exe, "Error reading file.\n");
                        free(data);
                        fclose(f);
                        return;
                    }

                    fclose(f); // close the file

                    ESP_LOGI(TAG_exe, "Read from file: '%s'", data);
                    // xEventGroupSetBits(spiffs_event_group, SPIFFS_OPERATION_DONE);
                }
            }
            else if (spiffs_noti.data_scr == DEVICE_CONFIGS)
            {
                if (spiffs_noti.flag_type == DEVICE_CONFIG_WRITE_FLAG)
                {
                    if (spiffs_noti.data != NULL)
                    {
                        ESP_LOGI(TAG_exe, "Opening deviceConfigs.txt file");
                        FILE *f = fopen("/spiffs/deviceConfigs.txt", "w");
                        if (f == NULL)
                        {
                            ESP_LOGE(TAG_exe, "Failed to open file for writing");
                        }
                        fprintf(f, spiffs_noti.data);
                        fclose(f);
                        free(spiffs_noti.data);
                        ESP_LOGI(TAG_exe, "File written");
                        // task that is waiting for the spiffs operation to be done will be signaled
                        // xEventGroupSetBits(spiffs_event_group , SPIFFS_OPERATION_DONE);
                    }
                }
            }
        }
        else
        {
            ESP_LOGI(MAILBOX_TAG, "could not receive from the mailbox \n");
        }
    }
}

void app_main(void)
{
    // init(); // this function will initialize all the configs on device.
    rgbConfig();
    swh_eth_init();         // initializing ethernet hardware.
    swh_file_system_init(); // initializing file system.
    spiffs_event_group = xEventGroupCreate();
    xEventGroupClearBits(spiffs_event_group, CLIENT_RECIEVED_BIT | SPIFFS_OPERATION_DONE);

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
    spiffs_mailBox = xQueueCreate(1, sizeof(SPIFFS_NOTIFIER)); // creating mailbox with 1 NOTIFIER space.

    if (spiffs_mailBox != NULL)
    {
        ESP_LOGI(MAILBOX_TAG, "spiffs mailbox has been created!\n");
        xTaskCreate(db_interface_task, "db-interface-task", 2048, NULL, 1, NULL);
        xTaskCreate(spiffs_task, "spiffs task", 4048, NULL, 3, NULL);
    }
    else
    {

        ESP_LOGI(MAILBOX_TAG, "spiffs mailbox could not created \n");
    }
}

// esp_err_t getStudentsData(device_config_t dConfig)
// {
//     // const int locationID = getLocationId(&dConfig);
//     // const char *auth = getAuthToken(&dConfig);

//     // char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER];
//     client_receive_buffer = (char *)malloc(MAX_HTTP_OUTPUT_BUFFER * sizeof(char));

//     esp_err_t err;
//     // making query string.
//     // char queryString[15];
//     // char url[100];
//     // snprintf(url, sizeof(url), "http://192.168.50.209:8000/api/students?location=%d", locationID);
//     // snprintf(url, sizeof(queryString), "location=%d", locationID);

//     esp_http_client_config_t config = {
//         .host = "2a85849f-67d6-40e7-a2cc-87c61ef2ac71.mock.pstmn.io",
//         .path = "/getStudentData",
//         // .query = queryString,
//         // .url = "https://2a85849f-67d6-40e7-a2cc-87c61ef2ac71.mock.pstmn.io/getStudentData",
//         .method = HTTP_METHOD_GET,
//         .event_handler = _http_event_handler,
//         .user_data = client_receive_buffer, // Pass address of local buffer to get response
//         .disable_auto_redirect = true,
//     };
//     esp_http_client_handle_t client = esp_http_client_init(&config);
//     esp_http_client_set_header(client, "token", auth);

//     // GET
//     err = esp_http_client_perform(client);

//     if (err == ESP_OK)
//     {
//         int status_code = esp_http_client_get_status_code(client);
//         ESP_LOGI(HTTP_CLIENT_TAG, "HTTP GET Status = %d, content_length = %" PRIu64,
//                  status_code,
//                  esp_http_client_get_content_length(client));
//         if (status_code == 200)
//         {
//             xEventGroupSetBits(spiffs_event_group, CLIENT_RECIEVED_BIT);
//         }
//     }
//     else
//     {
//         ESP_LOGE(HTTP_CLIENT_TAG, "HTTP GET request failed: %s", esp_err_to_name(err));

//         return ESP_FAIL;
//     }
//     ESP_LOG_BUFFER_HEX(HTTP_CLIENT_TAG, client_receive_buffer, strlen(client_receive_buffer));

//     return ESP_OK;
// }
