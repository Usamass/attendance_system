#include "control.h"
#define LV_TICK_PERIOD_MS 1
#define FINGERPRINT_NOTI_DELAY 20



extern lv_font_t lv_font_montserrat_32;
extern lv_img_dsc_t art;
extern lv_img_dsc_t remove_icon;
extern lv_img_dsc_t check;

static void lv_tick_task(void *arg);
static void guiTask(void *pvParameter);
static lv_obj_t* create_msgbox(lv_obj_t* bck_img , const char* title , const char* msg);
static lv_obj_t* create_img(lv_obj_t* bck_img , const lv_img_dsc_t* icon_scr);



/* all the graphics related work is here*/

SemaphoreHandle_t xGuiSemaphore;

static void guiTask(void *pvParameter) {
    
    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();
    
    /* Initialize SPI or I2C bus used by the drivers */
    lvgl_driver_init();

    static lv_color_t buf1[DISP_BUF_SIZE];

    /* Use double buffered when not working with monochrome displays */
    static lv_color_t buf2[DISP_BUF_SIZE];


    static lv_disp_draw_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;


    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    /* Create the demo application */
    static const lv_font_t * font_large; 

    font_large = &lv_font_montserrat_32;
    lv_obj_t * img1 = lv_img_create(lv_scr_act());
    lv_img_set_src(img1, &art);
    lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(img1, 370, 250);

    lv_obj_t * title = lv_label_create(img1);
    lv_obj_set_align(title , LV_ALIGN_CENTER);
    lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(title, font_large, 0);
    lv_obj_set_style_text_line_space(title, 8, 0);
    
    uint32_t count = 0;
    uint32_t time_lapse = 0;

    lv_obj_t* mbox1 = NULL;
    lv_obj_t* check_obj = NULL;
    bool msgbox_created = false;
    bool img_created = false;

 
    while (1) {

        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));
        count++;
        ds1307_get_time(&dev, &mytime);
        lv_label_set_text_fmt(title , "%02d: %02d: %02d\n%02d-%02d-%04d",
                            mytime.tm_hour, mytime.tm_min, mytime.tm_sec , 
                            mytime.tm_mday , mytime.tm_mon , mytime.tm_year + 1900);
        if ((msgbox_created == false) &&  (disp_msg == FINGERPRINT_SUCCESS)) {
            char msg[50] = {0};
            sprintf(msg , "Success!\n%s" , my_id);
            time_lapse = count;

            mbox1 = create_msgbox(img1 ,"<-Fingerprint->" , msg);
            check_obj = create_img(img1 , &check);
            twoShortBeeps();
            msgbox_created = true;
            img_created = true;
            disp_msg = 0x00;
            // printf("msg box created!\n");
        }
        else if ((msgbox_created == false) &&  (disp_msg == FINGERPRINT_NOT_MACHING)) {
            time_lapse = count;
            mbox1 = create_msgbox(img1 ,"<-Fingerprint->" , "No Matching Fingerprint!");
            check_obj = create_img(img1 , &remove_icon);
            threeShortBeeps();
            msgbox_created = true;
            img_created = true;
            disp_msg = 0x00;
            // printf("msg box created!\n");

        }
        else if ((msgbox_created == false) && (disp_msg == FINGERPRINT_RELEASE_MSG)) {
            time_lapse = count;
            mbox1 = create_msgbox(img1 ,"<-Fingerprint->" , "Please Release the finger!");
            msgbox_created = true;
            disp_msg = 0x00;
            // printf("msg box created!\n");


        }
        else if ((msgbox_created == false) && (disp_msg == FINGERPRINT_ENROLL_ERROR)) {
            time_lapse = count;
            mbox1 = create_msgbox(img1 ,"<-Fingerprint->" , "ENROLL ERROR!\nPlease try again.");
            check_obj = create_img(img1 , &remove_icon);
            threeShortBeeps();
            msgbox_created = true;
            img_created = true;
            disp_msg = 0x00;
            // printf("msg box created!\n");

        }
        else if ((msgbox_created == false) && (disp_msg == FINGERPRINT_SENCOND_PRINT)) {
            time_lapse = count;
            mbox1 = create_msgbox(img1 ,"<-Fingerprint->" , "Please put the same finger on the sensor.");
            msgbox_created = true;
            disp_msg = 0x00;
            // printf("msg box created!\n");

        }
        else if ((msgbox_created == false) && (disp_msg == FINGERPRINT_STORE_SUCCESS)) {
            time_lapse = count;
            mbox1 = create_msgbox(img1 ,"<-Fingerprint->" , "Fingerprints Stored Successfully!.");
            check_obj = create_img(img1 , &check);
            img_created = true;
            msgbox_created = true;
            disp_msg = 0x00;
            // printf("msg box created!\n");

        }
        else if ((msgbox_created == false) && (disp_msg == FINGERPRINT_ENROLL_CODE)) {
            time_lapse = count;
            mbox1 = create_msgbox(img1 ,"<-Fingerprint->" , "Please put your finger on the sensor.");
            msgbox_created = true;
            disp_msg = 0x00;
            //opt_flag = 0; // reset the flage to attendance
            // printf("msg box created!\n");

        }
        else if ((msgbox_created == false) && (disp_msg == FINGERPRINT_MAX_TAMP)) {
            time_lapse = count;
            mbox1 = create_msgbox(img1 ,"<-Fingerprint->" , "Max Tamplate count is already achieved!");
            msgbox_created = true;
            disp_msg = 0x00;
            // printf("msg box created!\n");

        }
        else if ((msgbox_created == false) && (disp_msg == ETHERNET_CONNECT_FLAG)) {
            time_lapse = count;
            mbox1 = create_msgbox(img1 ,"<-Ethernet->" , "Ethernet connected!");
            msgbox_created = true;
            disp_msg = 0x00;
            // printf("msg box created!\n");

        }
        else if ((msgbox_created == false) && (disp_msg == ETHERNET_DISCONNECT_FLAG)) {
            time_lapse = count;
            mbox1 = create_msgbox(img1 ,"<-Ethernet->" , "Ethernet disconnected!");
            msgbox_created = true;
            disp_msg = 0x00;
            // printf("msg box created!\n");

        }
        else if ((msgbox_created == false) && (disp_msg == GOT_IP_FLAG)) {
            time_lapse = count;
            mbox1 = create_msgbox(img1 ,"<-IP Received->" , getIpAddr(&dConfig));
            msgbox_created = true;
            disp_msg = 0x00;
            // printf("msg box created!\n");

        }
        else if ((msgbox_created == false) && (disp_msg == FINGERPRINT_ALREADY_THERE)) {
            time_lapse = count;
            mbox1 = create_msgbox(img1 ,"<-Fingerprint->" , "Fingerprint is already registered!");
            msgbox_created = true;
            disp_msg = 0x00;
            // printf("msg box created!\n");

        }
        else if ((msgbox_created == false) && (disp_msg == DEVICE_CONFIGS_NOT_FOUND)) {
            time_lapse = count;
            mbox1 = create_msgbox(img1 ,"<-Configuration->" , "This device is not configured yet!");
            msgbox_created = true;
            disp_msg = 0x00;
            // printf("msg box created!\n");

        }
        else if ((msgbox_created == false) && (disp_msg == SERVER_ERROR)) {
            time_lapse = count;
            mbox1 = create_msgbox(img1 ,"<-Server Error->" , "HTTP request failed");
            msgbox_created = true;
            disp_msg = 0x00;
            // printf("msg box created!\n");

        }
        
        

        if ((count - time_lapse) == FINGERPRINT_NOTI_DELAY && msgbox_created == true) {
            lv_obj_del(mbox1);
            if (img_created == true) {
                lv_obj_del(check_obj);
                img_created = false;
            }
            
            msgbox_created = false;
            resource_mutex = false;
            
            // printf("msg box deleted!\n");
        }
        if (disp_msg == FINGERPRINT_RELEASE_CODE && msgbox_created == true) {
            lv_obj_del(mbox1);
            if (img_created == true) {
                lv_obj_del(check_obj);
                img_created = false;
            }
            msgbox_created = false;
            resource_mutex = false;

            
            // printf("msg box deleted!\n");

        }
        
        

        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }

    /* this task should NEVER return */
    vTaskDelete(NULL);
}

// lvgl heart beat
static void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}

lv_obj_t* create_msgbox(lv_obj_t* bck_img , const char* title , const char* msg)
{
    lv_obj_t* msgbox = lv_msgbox_create(bck_img, title, msg, NULL, false);
    lv_obj_set_height(msgbox , 150);
    lv_obj_set_width(msgbox , 200);
    lv_obj_center(msgbox);

    return msgbox;


}
lv_obj_t* create_img(lv_obj_t* bck_img , const lv_img_dsc_t* icon_scr)
{
    lv_obj_t* icon = lv_img_create(bck_img);
    lv_img_set_src(icon , icon_scr);
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_size(icon , 50 , 50);

    return icon;
}



void control_init(void)
{
    ESP_LOGI("GUI_TASK" , "inside control init\n");
    xTaskCreatePinnedToCore(guiTask, "gui", 4096, NULL, 0, NULL, 1); // this is graphics handling task pinned to core 1
    // xTaskCreate(example_task , "example" , 2048 , NULL , 1 , NULL);



}
