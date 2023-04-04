#include "freertos/event_groups.h"
// All the event group signaturs are defined here.

#ifndef _EVENT_GROUPS_H
#define _EVNET_GROUPS_H
extern EventGroupHandle_t swh_ethernet_event_group;
extern EventGroupHandle_t spiffs_event_group;
#endif
