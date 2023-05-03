#include <stdio.h>
#include <string.h>
#include <esp_eth_enc28j60.h>
#include <ds1307.h>
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
#include "swh_utility.h"
#include "swh_client.h"


// static const char *ETH_TAG = "SWH_eth_test";
// static const char *HTTP_SERVER_TAG = "SWH_HTTP_TEST";
// esp_err_t getStudentsData(device_config_t);

QueueHandle_t mailBox;
QueueHandle_t spiffs_mailBox;
extern device_config_t dConfig; // device ip and mac will be set on connect to network
EventGroupHandle_t spiffs_event_group;

i2c_dev_t dev;
struct tm mytime = {
        .tm_year = 123, //(2022 - 1900)
        .tm_mon  = 04,  
        .tm_mday = 10,
        .tm_hour = 8,
        .tm_min  = 33,
        .tm_sec  = 30
    };
  
// #define EXAMPLE_ESP_MAXIMUM_RETRY (5)
// #define EXAMPLE_HTTP_QUERY_KEY_MAX_LEN (64)
// #define MIN(a, b) (((a) < (b)) ? (a) : (b))

static const char *HTTP_CLIENT_TAG = "http client";
static const char *MAILBOX_TAG = "mailbox";
static const char *TAG_exe = "spiffs";
// static const char *JSON_TAG = "JSON";

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
            ESP_LOGI(HTTP_CLIENT_TAG, "initializting Server\n");
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
            spiffs_noti.data = serialize_it(&dConfig);

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
            ESP_LOGI(MAILBOX_TAG, "data received from : %d \t data = %s \n", spiffs_noti.data_scr, spiffs_noti.data);
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
    ESP_ERROR_CHECK(i2cdev_init());
    memset(&dev, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(ds1307_init_desc(&dev, 0, 21, 22));
    ESP_ERROR_CHECK(ds1307_set_time(&dev, &mytime)); 

    spiffs_event_group = xEventGroupCreate();
    xEventGroupClearBits(
        spiffs_event_group, 
        CLIENT_RECIEVED_BIT | 
        SPIFFS_OPERATION_DONE | 
        DEVICE_CONFIG_BIT
    );

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


