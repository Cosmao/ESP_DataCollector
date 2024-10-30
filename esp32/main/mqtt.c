#include "include/mqtt.h"
#include "esp_event_base.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "include/dht11.h"
#include "include/settings.h"
#include "mqtt_client.h"
#include "portmacro.h"

static const char *MQTTTAG = "Mqtt status";
const char *topic = "sensors/esp/";
static bool mqttConnected = false;

extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_crt_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_crt_end");

static void mqtt_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  switch (event_id) {
  case MQTT_EVENT_CONNECTED: {
    esp_mqtt_client_subscribe(client, topic, 1);
    mqttConnected = true;
    break;
  }
  case MQTT_EVENT_DISCONNECTED: {
    mqttConnected = false;
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
      .broker.address.uri = "mqtts://emil.kool.se:8883",
      .broker.verification.certificate = (const char *)server_cert_pem_start,
      .broker.verification.skip_cert_common_name_check = true,
      .credentials = {.authentication = {
                          .certificate = (const char *)client_cert_pem_start,
                          .key = (const char *)client_key_pem_start,
                      }}};
  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_conf);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler,
                                 client);
  ESP_ERROR_CHECK(esp_mqtt_client_start(client));
  return client;
}

void mqttTask(void *pvParameter) {
#define buffSize 100
  settings_t *settingsPtr = (settings_t *)pvParameter;
  dht_t *dht = settingsPtr->dht;

  if (settingsPtr->isConnectedToWifi) {
    esp_mqtt_client_handle_t mqttClient = mqttInit();
    char buff[buffSize];
    while (true) {
      if (xSemaphoreTake(dht->dhtMutex, (TickType_t)10) == pdTRUE) {
        if ((!dht->sent) && (mqttConnected)) {
          snprintf(buff, buffSize, "{\"temperature\":%.1f,\"humidity\":%.1f}",
                   getDHTValue(&dht->temperature), getDHTValue(&dht->humidity));
          dht->sent = true;
          esp_mqtt_client_enqueue(mqttClient, topic, buff, 0, 1, 0, false);
        }
        xSemaphoreGive(dht->dhtMutex);
      }
      vTaskDelay(30000 / portTICK_PERIOD_MS);
    }
  }
  ESP_LOGE("MQTT", "mqtt exited");
  vTaskDelete(NULL);
}
