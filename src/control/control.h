#ifndef __QMSD_CONTROL_H
#define __QMSD_CONTROL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <inttypes.h>
#include "../ui/ui.h"
#include "../SWH_data_buffers.h"
#include "../SWH_event_flags.h"
#include <sys_beeps.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lvgl.h>
#include <lvgl_helpers.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_timer.h>
#include <esp_log.h>
#include "device_configs.h"




// typedef enum {
//     QMSD_CTRL_WIFI_STA_SET_CONFIG,               /* set sta wifi sta config(ssid/password) in nvs */
//     QMSD_CTRL_WIFI_CLEAR_CONFIG,              /* clear wifi config(nvs) */
//     QMSD_CTRL_WIFI_STA_START,              /* start wifi connect */
//     QMSD_CTRL_WIFI_STA_STOP,         /* stop wifi */
//     QMSD_CTRL_WIFI_STA_SCAN,                  /* start wifi scan */
//     QMSD_CTRL_GPIO_ISR,           /* gpio isr */
// } qmsd_ctrl_event_t;

// typedef struct {
//     const char *ssid;
//     const char *password;
//     uint8_t enable;
// } qmsd_ctrl_event_wifi_config;

void control_init(void);

// int qmsd_ctrl_event_send(int32_t event_id, void *event_data, size_t event_data_size, int32_t ticks_to_wait);
// int qmsd_ctrl_event_isr_send(int32_t event_id, void *event_data, size_t event_data_size, void *task_unblocked);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //__CONTROL_H
