#include "include/mqtt.h"
#include "esp_event_base.h"
#include "esp_log.h"
#include "include/dht11.h"
#include "include/fota.h"
#include "include/settings.h"
#include "mqtt_client.h"

// TODO: Add mTLS or something similar, subscribe to topic for new firmware
// updates and start a https update then

static const char *MQTTTAG = "Mqtt status";

static void mqtt_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  switch (event_id) {
  case MQTT_EVENT_CONNECTED: {
    esp_mqtt_client_subscribe(client, "/idfpye/qos1", 1);
    break;
  }
  case MQTT_EVENT_DISCONNECTED: {
    break;
  }
  case MQTT_EVENT_SUBSCRIBED: {
    break;
  }
  case MQTT_EVENT_PUBLISHED: {
    break;
  }
  case MQTT_EVENT_DATA: {
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    break;
  }
  case MQTT_EVENT_ERROR: {
    ESP_LOGE(MQTTTAG, "Error happened not good");
    break;
  }
  case MQTT_EVENT_BEFORE_CONNECT: {
    ESP_LOGE(MQTTTAG, "MQTT event before connecting");
    break;
  }
  default: {
    ESP_LOGE(MQTTTAG, "Unknown MQTT eventID: %d", event->event_id);
    break;
  }
  }
}

static esp_mqtt_client_handle_t mqttInit(void) {
  const esp_mqtt_client_config_t mqtt_conf = {
      .broker.address.uri = "mqtt://pajjen.local:1883",
  };
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_conf);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 client);
  ESP_ERROR_CHECK(esp_mqtt_client_start(client));
  return client;
}

void mqttTask(void *pvParameter) {
#define buffSize 100
  settings_t *settingsPtr = (settings_t *)pvParameter;
  dht_t *dhtStructPtr = settingsPtr->dht;
  if (wifiInitStation(settingsPtr)) {
    esp_mqtt_client_handle_t mqttClient = mqttInit();
    char buff[buffSize];
    while (true) {
      checkForFOTA();
      snprintf(buff, buffSize, "{ \"Temperature\":%d.%d, \"Humidity\":%d.%d }",
               dhtStructPtr->temperature.integer,
               dhtStructPtr->temperature.decimal,
               dhtStructPtr->humidity.integer, dhtStructPtr->humidity.decimal);
      esp_mqtt_client_enqueue(mqttClient, "/idfpye/qos1", buff, 0, 1, 0, false);

      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
  }
}
