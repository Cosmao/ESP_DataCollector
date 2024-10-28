#include "include/fota.h"
#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "include/wifi.h"
#include <stdio.h>
#include <string.h>

#define FIRMWARE_VERSION 5
#define UPDATE_JSON_URL                                                        \
  "https://raw.githubusercontent.com/Cosmao/ESP_DataCollector/refs/heads/"     \
  "main/esp32/build/firmware.json"

// receive buffer
char rcv_buffer[200];

// TODO: Change to the https more robust event handler
// clean up code into functions
// Maybe make it trigger on something? Or just a thread running rarely?

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
      strncpy(rcv_buffer, (char *)evt->data, evt->data_len);
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

// TODO: Fix a new return enum
static fota_err_t parseJSON(char *firmwareURI, int buffSize) {
  cJSON *json = cJSON_Parse(rcv_buffer);
  if (json == NULL) {
    ESP_LOGE("FOTA", "downloaded file is not a valid json, aborting...\n");
    cJSON_free(json);
    return FOTA_JSON_NO_JSON;
  }
  const cJSON *version = cJSON_GetObjectItemCaseSensitive(json, "version");
  if (!cJSON_IsNumber(version)) {
    ESP_LOGE("FOTA", "unable to read new version, aborting...\n");
    cJSON_free(json);
    return FOTA_JSON_NO_VERSION;
  }

  int newVersion = version->valueint;
  if (!(newVersion > FIRMWARE_VERSION)) {
    cJSON_free(json);
    return FOTA_JSON_SAME_VERSION;
  }

  const cJSON *file = cJSON_GetObjectItemCaseSensitive(json, "file");
  if (!cJSON_IsString(file) || !(file->valuestring != NULL)) {
    ESP_LOGE("FOTA", "Error reading fota URI");
    cJSON_free(json);
    return FOTA_JSON_URL_ERROR;
  }
  snprintf(firmwareURI, buffSize, "%s", file->valuestring);
  cJSON_free(json);
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
