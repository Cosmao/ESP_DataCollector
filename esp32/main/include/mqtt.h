#ifndef __mqtt_h
#define __mqtt_h

#include "mqtt_client.h"
#include "wifi.h"

esp_mqtt_client_handle_t mqtt_init(void);
void mqttTask(void *pvParameter);

#endif // !__mqtt_h
