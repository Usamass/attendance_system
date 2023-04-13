#ifndef _SWH_CLIENT_H
#define _SWH_CLIENT_H
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include "../event_bits.h"
#include "../SWH_eventGroups.h"
#include "device_configs.h"
#include "../SWH_data_buffers.h"

#define EXAMPLE_ESP_MAXIMUM_RETRY (5)
#define EXAMPLE_HTTP_QUERY_KEY_MAX_LEN (64)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

esp_err_t getStudentsData(/*device_config_t*/);

