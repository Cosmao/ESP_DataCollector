#ifndef __mqtt_h
#define __mqtt_h

#include "mqtt_client.h"
#include "wifi.h"

esp_mqtt_client_handle_t mqttInit(void);
void mqttTask(void *pvParameter);

#endif // !__mqtt_h
