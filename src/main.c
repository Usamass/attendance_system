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
// #include "../SWH_web_pages.h"
#include "device_configs.h"
#include "swh_file_system.h"
#include "swh_utility.h"
#include "swh_client.h"
#include "mapping_table.h"

#include <lvgl.h>
#include <lvgl_esp32_drivers/lvgl_helpers.h>
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "r307.h"
#include "sys_beeps.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "ui/ui.h"


/*********************
 *      DEFINES
 *********************/
#define TAG "LVGL_TAG"
#define LV_TICK_PERIOD_MS 1
#define FINGERPRINT_INPUT_PIN 34
#define FINGERPRINT_NOTI_DELAY 20
#define GOT_F_ID 5
#define TOUCH_TIME_MS 500


// static const char *ETH_TAG = "SWH_eth_test";
// static const char *HTTP_SERVER_TAG = "SWH_HTTP_TEST";
// esp_err_t getStudentsData(device_config_t);

QueueHandle_t spiffs_mailBox;
extern uint16_t template_number;   // getting templete number from fingerprint library
extern uint16_t page_id;
extern uint8_t is_ethernet_connected;
extern char loginPage[];
extern char dashboard[];
extern char enrollmentPage[];

EventGroupHandle_t spiffs_event_group;
EventGroupHandle_t fingerprint_event;
mapping_t id_mapping;
mapping_strct mp_struct;
uint8_t disp_msg = 0x00;
uint8_t opt_flag;
uint16_t prev_touch_time = 0;
device_configs dConfig_flag;

i2c_dev_t dev;
struct tm mytime;
bool resource_mutex = false;
  
// #define EXAMPLE_ESP_MAXIMUM_RETRY (5)
// #define EXAMPLE_HTTP_QUERY_KEY_MAX_LEN (64)
// #define MIN(a, b) (((a) < (b)) ? (a) : (b))
/**********************
 *  STATIC PROTOTYPES
 **********************/


static const char *HTTP_CLIENT_TAG = "http client";
static const char *MAILBOX_TAG = "mailbox";
static const char *TAG_exe = "spiffs";
static const char* F_TAG = "finger tag";
char* my_id = NULL;
char* html_files[] = {
                        "/spiffs/html_pages/login_page.txt" , 
                        "/spiffs/html_pages/dashboard_page.txt" , 
                        "/spiffs/html_pages/enrollment_page.txt"
                     };
char* html_ptr[] = {&loginPage , &dashboard , &enrollmentPage};
                /*   login_page , dashboard_page , enrollment_page*/
                /*           |            |               | */
const char* new_html_files[3] = {NULL ,        NULL ,          NULL};
// static const char *JSON_TAG = "JSON";

char default_address[4] = {0xFF, 0xFF, 0xFF, 0xFF};         //++ Default Module Address is FF:FF:FF:FF
char default_password[4] = {0x00, 0x00, 0x00, 0x00};        //++ Default Module Password is 00:00:00:00

// fingerprint touch sensor event handlar (external event)
static void IRAM_ATTR fingerprint_handler(void* args){

    uint16_t new_touch_time = esp_timer_get_time()/1000;
    if (((new_touch_time - prev_touch_time) > 1000) && (resource_mutex == false)) {
        xEventGroupSetBits(fingerprint_event , FINGERPRINT_IMG_EVENT_BIT);

    }
    prev_touch_time = new_touch_time;

    
    

}
/*This task will collect the fingerprints from the sensor*/
static void takeImg(void* args){
    EventBits_t f_bit;
    uint8_t confirmation_code;
    uint8_t max_image = 0;


    while (true){
       f_bit = xEventGroupWaitBits(fingerprint_event , FINGERPRINT_IMG_EVENT_BIT , pdTRUE , pdFALSE , portMAX_DELAY);

       if ((f_bit & FINGERPRINT_IMG_EVENT_BIT) != 0){
            confirmation_code = -1;
            while (confirmation_code != 0x00 && max_image < 5){
                confirmation_code = GenImg(default_address);
                max_image++;
            }
            max_image = 0;

            if (confirmation_code == 0x00){
                shortBeep();
                resource_mutex = true;
                xEventGroupSetBits(fingerprint_event , FINGERPRINT_DONE_EVENT_BIT);
            }
            
           

        }
    }

}
/* imp!!  This task will enroll or make attendance to collected fingerprints */
/* opt_flag = 0 -> attendance
   opt_flag = 1 -> enrollment
*/
static void fingerprintTask(void* args){
    EventBits_t f_bit;
    uint8_t confirmation_code = 0;
    uint8_t count = 0;
    char int_str[10] = {0};
    char temp_str[10] = {0};
   
    while (true){
    
        f_bit = xEventGroupWaitBits(fingerprint_event , FINGERPRINT_DONE_EVENT_BIT , pdTRUE , pdFALSE , portMAX_DELAY);
        if ((f_bit & FINGERPRINT_DONE_EVENT_BIT) != 0){
            ESP_LOGI(F_TAG , "value of opt_flag=%d" , opt_flag);
            if (opt_flag){
                /* Enrollment logic */ 
                count++;
                TempleteNum(default_address);
                sprintf(temp_str , "%d" , (template_number +1));
                if (count <= 2){
                    sprintf(int_str , "%d" , count);
                    disp_msg = FINGERPRINT_RELEASE_MSG;
                    ESP_LOGI(F_TAG , "Please Release the finger! count: %d\n" , count);
                    while (gpio_get_level(FINGERPRINT_INPUT_PIN) != 1);
                    disp_msg = FINGERPRINT_RELEASE_CODE;
                    ESP_LOGI(F_TAG , "finger released!\n");
                    confirmation_code = Img2Tz(default_address , int_str);
                    if (confirmation_code == 0x00){
                        if (count == 2){
                            confirmation_code = RegModel(default_address);
                            if (confirmation_code == 0x00){
                                confirmation_code = Store(default_address , "1" , temp_str); // page id field is variable
                                if (confirmation_code != 0x00){
                                    count = 0;
                                    opt_flag = 0;
                                    disp_msg = FINGERPRINT_ENROLL_ERROR;
                                    free(mp_struct.vu_id_st);
                                }
                                else {
                                    disp_msg = FINGERPRINT_STORE_SUCCESS;
                                    twoShortBeeps();
                                    mp_struct.f_id_st = atoi(temp_str);   // assign fingerid to mp_struct variable.
                                    opt_flag = 0; // reset to attendance.
                                    ESP_LOGI(F_TAG , "vu_id: %s - f_id_st :%d - tamp_count: %d" , mp_struct.vu_id_st , mp_struct.f_id_st , mp_struct.tamp_count);
                                    
                                    char* enrollment = enrollmentToJson(mp_struct.vu_id_st , mp_struct.tamp_count);
                                    ESP_LOGI(F_TAG , "enrollment: %s" , enrollment);
                                    sendEnrollment(dConfig , enrollment);
                                    free(enrollment);
                                    xEventGroupSetBits(spiffs_event_group , CLIENT_RECIEVED_BIT);  // store the mapping.
                                }
                                count = 0;
                            }
                            else {
                                count = 0;                                
                                disp_msg = FINGERPRINT_ENROLL_ERROR;
                                threeShortBeeps();
                                free(mp_struct.vu_id_st);
                            }
                        }
                        else {
                            confirmation_code = Search(default_address , "1" , "1" , temp_str);
                            if (confirmation_code == 0x00){                                
                                disp_msg = FINGERPRINT_ALREADY_THERE;
                                ESP_LOGI(F_TAG , "Fingerprint is already registered!\n");
                                count = 0;
                                opt_flag = 0; // reset to attendance
                            }
                            else {
                                disp_msg = FINGERPRINT_SENCOND_PRINT;
                                ESP_LOGI(F_TAG , "Please put same finger on the sensor!\n");
                            }
                          

                        }
                    }
                    else {
                        count = 0;                        
                        disp_msg = FINGERPRINT_ENROLL_ERROR;
                        threeShortBeeps();
                        free(mp_struct.vu_id_st);
                    }
                
                
                }    
            }
            else {
                /*attendance logic*/
                TempleteNum(default_address);
                sprintf(int_str , "%d" , (template_number +2));
                // ESP_LOGI(F_TAG , "Please Release the finger! templete: %s ", int_str);
                // while (gpio_get_level(FINGERPRINT_INPUT_PIN) != 1);
                // ESP_LOGI(F_TAG , "finger released!\n");
                
                confirmation_code = Img2Tz(default_address , "1");
                if (confirmation_code == 0x00){
                    confirmation_code = Search(default_address , "1" , "1" , int_str);
                    if (confirmation_code == 0x00){
                        // ds1307_get_time(&dev, &mytime);
                        // printf("%04d-%02d-%02d %02d:%02d:%02d\n", mytime.tm_year + 1900 /*Add 1900 for better readability*/, mytime.tm_mon + 1,
                        // mytime.tm_mday, mytime.tm_hour, mytime.tm_min, mytime.tm_sec);
                        if (is_ethernet_connected /*, && got_ip_address?*/) {
                            my_id = get_vu_id(&id_mapping , page_id);
                            char* attendance = attendanceToJson(my_id);
                            ESP_LOGI(F_TAG , "%s page_id: %d" , attendance , page_id);
                            sendAttendance(dConfig , attendance);
                            twoShortBeeps(); 
                            free(attendance);
                                disp_msg = FINGERPRINT_SUCCESS;

                        }
                        else disp_msg = ETHERNET_DISCONNECT_FLAG; 
                                              

                    }
                    else {
                        threeShortBeeps();    
                        disp_msg = FINGERPRINT_NOT_MACHING;  // no fingerprint matches
                    }
                    
                }
                else {
                    threeShortBeeps();
                    disp_msg = 0x03; // image error try again

                }


            }
        }
    }
}


//------------------------------This task will notify network status------------------------------------------------------

static void networkStatusTask(void *pvParameter)
{
    
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
            swh_server_init();
            // send got ip address msg to display
            disp_msg = GOT_IP_FLAG;
            // initializing html webpages as ip address received.
            new_html_files[0] = str_replace(loginPage , "_IP" , getIpAddr(&dConfig));
            // ESP_LOGI("str_html" , "%s" , new_html_files[0]);
            new_html_files[1] = str_replace(dashboard , "_IP" , getIpAddr(&dConfig));
            // ESP_LOGI("str_html" , "%s" , new_html_files[1]);
            new_html_files[2] = str_replace(enrollmentPage , "_IP" , getIpAddr(&dConfig));
            ESP_LOGI(HTTP_CLIENT_TAG, "initializting Server\n");
            shortBeep();

        }
        else if ((connectBits & ETHERNET_CONNECTED_BIT) != 0)
        {
            // send ethernet connected msg to display.
            disp_msg = ETHERNET_CONNECT_FLAG;
            shortBeep();
            
        }
        else
        {
            // send msg to display for ethernet disconnect
            disp_msg = ETHERNET_DISCONNECT_FLAG;
            longBeep();
        
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
            CLIENT_RECIEVED_BIT | DEVICE_CONFIG_BIT | LOAD_MAPPING_BIT | FLASH_FLUSHING_BIT | HTML_FILE_BIT, // add other event bit as the application grows.
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);
        if ((requestBits & CLIENT_RECIEVED_BIT) != 0){
            DataSource_t dataSrc = CLIENT;
            spiffs_noti.data_scr = dataSrc;
            spiffs_noti.flag_type = CLIENT_WRITE_FLAG;
            spiffs_noti.data = deserialize_it(&id_mapping , &mp_struct);

            mailBox_status = xQueueSend(spiffs_mailBox, &spiffs_noti, portMAX_DELAY);
            if (mailBox_status != pdPASS){
                ESP_LOGI(MAILBOX_TAG, "Could not send to the queue \n");
            }
        }
        else if ((requestBits & DEVICE_CONFIG_BIT) != 0){
            DataSource_t dataSrc = DEVICE_CONFIGS;
            spiffs_noti.data_scr = dataSrc;
            if (dConfig_flag == device_configs_read) {
                spiffs_noti.flag_type = DEVICE_CONFIG_READ_FLAG;
                spiffs_noti.data = NULL;

            } else {
                spiffs_noti.flag_type = DEVICE_CONFIG_WRITE_FLAG;
                spiffs_noti.data = serialize_it(&dConfig);

            }
            // before sending it must convert it into json format.
            

            mailBox_status = xQueueSend(spiffs_mailBox, &spiffs_noti, portMAX_DELAY);
            if (mailBox_status != pdPASS){
                ESP_LOGI(MAILBOX_TAG, "Could not send to the queue \n");
            }
        }
        else if ((requestBits & LOAD_MAPPING_BIT) != 0){

            DataSource_t dataSrc = LOAD_MAPPING;
            spiffs_noti.data_scr = dataSrc;
            spiffs_noti.flag_type = LOAD_MAPPING_FLAG;
            spiffs_noti.data = NULL;

            mailBox_status = xQueueSend(spiffs_mailBox, &spiffs_noti, portMAX_DELAY);
            if (mailBox_status != pdPASS){
                ESP_LOGI(MAILBOX_TAG, "Could not send to the queue \n");
            }

        }
        else if ((requestBits & FLASH_FLUSHING_BIT) != 0){
            DataSource_t dataSrc = FLASH_FLUSH;
            spiffs_noti.data_scr = dataSrc;
            spiffs_noti.data = NULL;

            mailBox_status = xQueueSend(spiffs_mailBox, &spiffs_noti, portMAX_DELAY);
            if (mailBox_status != pdPASS){
                ESP_LOGI(MAILBOX_TAG, "Could not send to the queue \n");
            }

        }
        else if ((requestBits & HTML_FILE_BIT) != 0){
            DataSource_t dataSrc = HTML_FILES;
            spiffs_noti.data_scr = dataSrc;
            spiffs_noti.flag_type = HTML_FILE_READ;
            spiffs_noti.data = NULL;

            mailBox_status = xQueueSend(spiffs_mailBox, &spiffs_noti, portMAX_DELAY);
            if (mailBox_status != pdPASS){
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
            //ESP_LOGI(MAILBOX_TAG, "data received from : %d \t data = %s \n", spiffs_noti.data_scr, spiffs_noti.data);
            if (spiffs_noti.data_scr == CLIENT)
            {
                if (spiffs_noti.flag_type == CLIENT_WRITE_FLAG)
                {
                    if (spiffs_noti.data != NULL)
                    {
                        ESP_LOGI(TAG_exe, "Opening stdData.txt file" );
                        FILE *f = fopen("/spiffs/stdData.txt", "w");
                        if (f == NULL)
                        {
                            ESP_LOGE(TAG_exe, "Failed to open file for writing");
                        }
                        fprintf(f, spiffs_noti.data);
                        fclose(f);
                        free(mp_struct.vu_id_st);
                        ESP_LOGI(TAG_exe, "File written");
                        xEventGroupSetBits(spiffs_event_group , LOAD_MAPPING_BIT); 

                    }
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
                        dConfig_flag = device_configs_read;
                        xEventGroupSetBits(spiffs_event_group , DEVICE_CONFIG_BIT); 

                        
                        
                    }
                }
                else {
                   ESP_LOGI(TAG_exe, "Reading from deviceConfigs.txt file");
                    FILE *f = fopen("/spiffs/deviceConfigs.txt", "r");
                    if (f == NULL)
                    {
                        ESP_LOGE(TAG_exe, "Failed to open file for reading");
                        return;
                    }
                    fseek(f, 0L, SEEK_END); // move file pointer to end of file
                    long size = ftell(f);   // get file size
                    fseek(f, 0L, SEEK_SET); // move file pointer back to beginning of file
                    
                    if (size <= 0)
                    {
                        ESP_LOGI(TAG_exe, "No data inside /spiffs/deviceConfigs.txt size = %lu\n" , size);
                        fclose(f);
                        disp_msg = DEVICE_CONFIGS_NOT_FOUND;
                        //return;
                    }
                    else {
                        char* config_data = (char *)malloc(size); // allocate memory for buffer
                        size_t result = fread(config_data, 1, size, f); // read file into buffer
                        if (result != size)
                        {
                            ESP_LOGI(TAG_exe, "Error reading file.\n");
                            free(config_data);
                            fclose(f);
                            return;
                        }

                        fclose(f); // close the file

                        ESP_LOGI(TAG_exe, "Read from file: '%s'", config_data);
                        deserialize_configs(config_data , &dConfig);
                        
                    }    
                }
            }
            else if (spiffs_noti.data_scr == LOAD_MAPPING) {

                if (spiffs_noti.flag_type == LOAD_MAPPING_FLAG){

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
                    
                    if (size <= 0)
                    {
                        id_mapping.mapping_arr = NULL;
                        ESP_LOGI(TAG_exe, "No data inside /spiffs/stdData.txt size = %lu\n" , size);
                        fclose(f);
                        //return;
                    }
                    else {
                        id_mapping.mapping_arr = (char *)malloc(size); // allocate memory for buffer
                        size_t result = fread(id_mapping.mapping_arr, 1, size, f); // read file into buffer
                        if (result != size)
                        {
                            ESP_LOGI(TAG_exe, "Error reading file.\n");
                            free(id_mapping.mapping_arr);
                            fclose(f);
                            return;
                        }

                        fclose(f); // close the file

                        ESP_LOGI(TAG_exe, "Read from file: '%s'", id_mapping.mapping_arr);
                        // parse this to get root* (id_mapping.root = cJSON_parse(id_mapping.mapping_arr))
                        parse_mapping(&id_mapping);
                        //get_vu_id(&id_mapping , 6);

                        //free(id_mapping.mapping_arr);
                    }
                }

            }
            else if (spiffs_noti.data_scr == FLASH_FLUSH){
                ESP_LOGI("SPIFFS testing" , "inside flushing flash!\n");
                FILE* file = fopen("/spiffs/stdData.txt", "w");
                if (file == NULL) {
                    ESP_LOGI("SPIFFS testing" , "writing on file error!\n");
                    // Error handling: Failed to open the file
                    // Handle the error appropriately
                }
                fclose(file);

            }
            else if (spiffs_noti.data_scr == HTML_FILES)
            {
                if (spiffs_noti.flag_type == HTML_FILE_WRITE)
                {
                    
                    ESP_LOGI(TAG_exe, "Opening HTML file");
                    for (int i = 0 ; i < 3 ; i++) {
                        FILE *f = fopen(html_files[i], "w");
                        if (f == NULL)
                        {
                            ESP_LOGE(TAG_exe, "Failed to open file for writing");
                        }
                        fprintf(f, html_ptr[i]);
                        fclose(f);
                        ESP_LOGI(TAG_exe, "File written");
                    }
                        
                        
                        
                    
                }
                else {
                    ESP_LOGI(TAG_exe, "Reading html files");
                    for (int i = 0 ; i < 3 ; i++) {
                        FILE *f = fopen(html_files[i], "r");
                        if (f == NULL)
                        {
                            ESP_LOGE(TAG_exe, "Failed to open file for reading");
                            return;
                        }
                        fseek(f, 0L, SEEK_END); // move file pointer to end of file
                        long size = ftell(f);   // get file size
                        fseek(f, 0L, SEEK_SET); // move file pointer back to beginning of file
                        
                        if (size <= 0)
                        {
                            ESP_LOGI(TAG_exe, "No data inside %s size = %lu\n" , html_files[i] , size);
                            fclose(f);
                            //return;
                        }
                        else {
                            char* file = (char *)malloc(size); // allocate memory for buffer
                            size_t result = fread(file , 1, size, f); // read file into buffer
                            if (result != size)
                            {
                                ESP_LOGI(TAG_exe, "Error reading %s.\n" , html_files[i]);
                                free(file);
                                fclose(f);
                                return;
                            }

                            fclose(f); // close the file

                            new_html_files[i] = str_replace(file , "_IP" , getIpAddr(&dConfig));
                            ESP_LOGI(TAG_exe, "Read from file: '%s' \n %s", html_files[i] , new_html_files[i]);


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
}
void app_main(void)
{
    // init(); // this function will initialize all the configs on device.
    rgbConfig();
    init_buzzer();
    r307_init();            // initializing uart for r307 fingerprint sensor.
    swh_eth_init();         // initializing ethernet hardware.
    swh_file_system_init(); // initializing file system.
    ESP_ERROR_CHECK(i2cdev_init());  // initializing i2c driver for rtc module.
    memset(&dev, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(ds1307_init_desc(&dev, 0, 21, 22));
    dConfig_flag = device_configs_read;
    
    spiffs_event_group = xEventGroupCreate();
    xEventGroupClearBits(
        spiffs_event_group, 
        CLIENT_RECIEVED_BIT | 
        SPIFFS_OPERATION_DONE | 
        DEVICE_CONFIG_BIT |
        LOAD_MAPPING_BIT | 
        FLASH_FLUSHING_BIT
    );

    fingerprint_event = xEventGroupCreate();
    xEventGroupClearBits(fingerprint_event , FINGERPRINT_IMG_EVENT_BIT | FINGERPRINT_DONE_EVENT_BIT | FINGERPRINT_ERROR_EVENT_BIT);

    gpio_reset_pin(FINGERPRINT_INPUT_PIN);                                         
    gpio_set_direction(FINGERPRINT_INPUT_PIN , GPIO_MODE_INPUT);
    gpio_set_intr_type(FINGERPRINT_INPUT_PIN , GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(FINGERPRINT_INPUT_PIN , fingerprint_handler , (void*)1);


    id_mapping.mapping_arr = NULL; // this is done in init task, where it will point to mapping data from flash
    
    main_ui_entry();
    xTaskCreatePinnedToCore(takeImg, "img task", 4096, NULL, 2, NULL , 0);
    xTaskCreatePinnedToCore(fingerprintTask, "fingerprint task", 4096, NULL, 1, NULL , 0);
    xTaskCreatePinnedToCore(networkStatusTask, "network status task", 4000, NULL, 1, NULL , 0);    
    // mailBox = xQueueCreate(1, sizeof(NOTIFIER)); // creating mailbox with 1 NOTIFIER space.

    // if (mailBox != NULL)
    // {
    //     ESP_LOGI(MAILBOX_TAG, "mailbox has been created!\n");
    //     xTaskCreate(notifierTask, "notifier-task", 2048, NULL, 1, NULL);
    //     xTaskCreate(displayNotification, "display-notification", 2048, NULL, 4, NULL);
    // }
    // else
    // {

    //     ESP_LOGI(MAILBOX_TAG, "mailbox could not created \n");
    // }
    spiffs_mailBox = xQueueCreate(1, sizeof(SPIFFS_NOTIFIER)); // creating mailbox with 1 NOTIFIER space.

    if (spiffs_mailBox != NULL)
    {
        ESP_LOGI(MAILBOX_TAG, "spiffs mailbox has been created!\n");
        xTaskCreatePinnedToCore(db_interface_task, "db-interface-task", 4048, NULL, 1, NULL , 0);
        xTaskCreatePinnedToCore(spiffs_task, "spiffs task", 4048, NULL, 3, NULL , 0);
    }
    else
    {

        ESP_LOGI(MAILBOX_TAG, "spiffs mailbox could not created \n");
    }

    uint8_t confirmation_code = 0;
    confirmation_code = VfyPwd(default_address, default_password);      //++ Performs Password Verification with Fingerprint Module

    if (confirmation_code == 0x00)
    {
        ESP_LOGI(F_TAG ,"R307 FINGERPRINT MODULE DETECTED\n");
    }

    confirmation_code = ReadSysPara(default_address);

    if (confirmation_code == 0x00)
    {
        ESP_LOGI(F_TAG , "R307 System Parameter Read!\n");
    }
   
    vTaskDelay(pdMS_TO_TICKS(100));
    // xEventGroupSetBits(spiffs_event_group , FLASH_FLUSHING_BIT);
    /*Load mapping data from the flash*/
    xEventGroupSetBits(spiffs_event_group , LOAD_MAPPING_BIT); 
    /*Load device configurations from the flash*/
    xEventGroupSetBits(spiffs_event_group , DEVICE_CONFIG_BIT); 
    /*Reading HTML files from the flash*/
    // xEventGroupSetBits(spiffs_event_group , HTML_FILE_BIT);
    //Empty(default_address);   
    TempleteNum(default_address);
       
    // ESP_LOGI("str_html" , "%s" , new_html_files[2]);



}



