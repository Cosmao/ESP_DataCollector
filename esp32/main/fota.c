#include "include/fota.h"
#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "include/wifi.h"
#include "portmacro.h"
#include <stdio.h>
#include <string.h>

#define FIRMWARE_VERSION 10
#define UPDATE_JSON_URL                                                        \
  "https://raw.githubusercontent.com/Cosmao/ESP_DataCollector/refs/heads/"     \
  "main/esp32/build/firmware.json"

// receive buffer
#define rcvBufferSize 200
char rcv_buffer[rcvBufferSize];

// esp_http_client event handler
esp_err_t _http_event_handler(esp_http_client_event_t *evt) {

  switch (evt->event_id) {
  case HTTP_EVENT_REDIRECT:
  case HTTP_EVENT_ERROR:
    break;
  case HTTP_EVENT_ON_CONNECTED:
    break;
  case HTTP_EVENT_HEADER_SENT:
    break;
  case HTTP_EVENT_ON_HEADER:
    break;
  case HTTP_EVENT_ON_DATA:
    if (!esp_http_client_is_chunked_response(evt->client)) {
      strncpy(rcv_buffer, (char *)evt->data,
              rcvBufferSize > evt->data_len ? evt->data_len : rcvBufferSize);
    }
    break;
  case HTTP_EVENT_ON_FINISH:
    break;
  case HTTP_EVENT_DISCONNECTED:
    break;
  }
  return ESP_OK;
}

static esp_err_t buildHttpClient(esp_http_client_handle_t *client) {
#define buffLen 255
  char buff[buffLen];
  snprintf(buff, buffLen, "%s", UPDATE_JSON_URL);
  esp_http_client_config_t config = {
      .url = buff,
      .event_handler = _http_event_handler,
      .crt_bundle_attach = esp_crt_bundle_attach,
  };
  *client = esp_http_client_init(&config);
  return ESP_OK;
}

static fota_err_t parseJSON(char *firmwareURI, int buffSize) {
  cJSON *json = cJSON_Parse(rcv_buffer);
  if (json == NULL) {
    ESP_LOGE("FOTA", "downloaded file is not a valid json, aborting...\n");
    cJSON_Delete(json);
    return FOTA_JSON_NO_JSON;
  }

  const cJSON *version = cJSON_GetObjectItemCaseSensitive(json, "version");
  if (!cJSON_IsNumber(version)) {
    ESP_LOGE("FOTA", "unable to read new version, aborting...\n");
    cJSON_Delete(json);
    return FOTA_JSON_NO_VERSION;
  }

  const int newVersion = version->valueint;
  if (!(newVersion > FIRMWARE_VERSION)) {
    ESP_LOGI("FOTA",
             "current firmware version (%d) is greater than or "
             "equal to the available one (%d) nothing to do",
             FIRMWARE_VERSION, newVersion);
    cJSON_Delete(json);
    return FOTA_JSON_SAME_VERSION;
  }

  const cJSON *file = cJSON_GetObjectItemCaseSensitive(json, "file");
  if (!cJSON_IsString(file) || !(file->valuestring != NULL)) {
    ESP_LOGE("FOTA", "Error reading fota URI");
    cJSON_Delete(json);
    return FOTA_JSON_URL_ERROR;
  }
  snprintf(firmwareURI, buffSize, "%s", file->valuestring);
  cJSON_Delete(json);
  return 0;
}

static esp_err_t preformOTA(const char *url) {
  esp_http_client_config_t ota_client_config = {
      .url = url,
      .keep_alive_enable = false,
      .crt_bundle_attach = esp_crt_bundle_attach,
  };

  esp_https_ota_config_t ota_config = {
      .http_config = &ota_client_config,
  };

  esp_err_t ret = esp_https_ota(&ota_config);
  if (ret == ESP_OK) {
    ESP_LOGW("FOTA", "OTA successful, restarting");
    esp_restart();
  } else {
    ESP_LOGE("FOTA", "FOTA failed!");
    return ret;
  }
}

void checkForFOTA(void) {
#define buffSize 255
  memset(rcv_buffer, '\0', sizeof(rcv_buffer));
  esp_http_client_handle_t client;
  buildHttpClient(&client);

  esp_err_t err = esp_http_client_perform(client);
  if (err != ESP_OK) {
    ESP_LOGE("FOTA", "Error downloading the json");
    esp_http_client_cleanup(client);
    return;
  }

  char buff[buffSize];
  if (parseJSON(buff, buffSize) != FOTA_OK) {
    esp_http_client_cleanup(client);
    return;
  }

  preformOTA(buff);
  esp_http_client_cleanup(client);
}

void fotaTask(void *pvParameter) {
#define updateDelayTime 1000 * 60 * 10
  settings_t *settingsPtr = (settings_t *)pvParameter;
  while (1) {
    if (settingsPtr->isConnectedToWifi) {
      ESP_LOGI("FOTA", "Checking for FOTA");
      checkForFOTA();
    }
    vTaskDelay(updateDelayTime / portTICK_PERIOD_MS);
  }

  ESP_LOGE("FOTA", "Fota thread exiting");
  vTaskDelete(NULL);
}
