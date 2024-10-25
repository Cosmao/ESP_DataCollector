#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "include/settings.h"
#include "include/wifi.h"

#define FIRMWARE_VERSION 0.1
#define UPDATE_JSON_URL                                                        \
  "https://raw.githubusercontent.com/Cosmao/ESP_DataCollector/refs/heads/"     \
  "FOTA/esp32/build/firmware.json"

// receive buffer
char rcv_buffer[200];

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

void check_update_task(void *pvParameter) {
  settings_t *settingsPtr = (settings_t *)pvParameter;
  if (wifiInitStation(settingsPtr)) {

    int cnt = 0;
    while (1) {
      char buf[255];
      sprintf(buf, "%s?token=%d", UPDATE_JSON_URL, cnt);
      cnt++;
      ESP_LOGI("FOTA", "Looking for a new firmware at %s", buf);

      // configure the esp_http_client
      esp_http_client_config_t config = {
          .url = buf,
          .event_handler = _http_event_handler,
          .keep_alive_enable = true,
          .timeout_ms = 30000,
          .crt_bundle_attach = esp_crt_bundle_attach,
      };
      esp_http_client_handle_t client = esp_http_client_init(&config);

      // downloading the json file
      esp_err_t err = esp_http_client_perform(client);
      if (err == ESP_OK) {

        // parse the json file
        cJSON *json = cJSON_Parse(rcv_buffer);
        if (json == NULL)
          ESP_LOGE("FOTA",
                   "downloaded file is not a valid json, aborting...\n");
        else {
          cJSON *version = cJSON_GetObjectItemCaseSensitive(json, "version");
          cJSON *file = cJSON_GetObjectItemCaseSensitive(json, "file");

          // check the version
          if (!cJSON_IsNumber(version))
            ESP_LOGE("FOTA", "unable to read new version, aborting...\n");
          else {

            double new_version = version->valuedouble;
            if (new_version > FIRMWARE_VERSION) {

              ESP_LOGI("FOTA",
                       "current firmware version (%.1f) is lower than the "
                       "available one (%.1f), upgrading...",
                       FIRMWARE_VERSION, new_version);
              if (cJSON_IsString(file) && (file->valuestring != NULL)) {
                ESP_LOGI("FOTA",
                         "downloading and installing new firmware (%s)...",
                         file->valuestring);

                esp_http_client_config_t ota_client_config = {
                    .url = file->valuestring,
                    .keep_alive_enable = true,
                    .crt_bundle_attach = esp_crt_bundle_attach,
                };

                esp_https_ota_config_t ota_config = {
                    .http_config = &ota_client_config,
                };

                esp_err_t ret = esp_https_ota(&ota_config);
                if (ret == ESP_OK) {
                  printf("OTA OK, restarting...\n");
                  esp_restart();
                } else {
                  printf("OTA failed...\n");
                }
              } else
                ESP_LOGE("FOTA",
                         "unable to read the new file name, aborting...");
            } else
              ESP_LOGI("FOTA",
                       "current firmware version (%.1f) is greater than or "
                       "equal to the available one (%.1f) nothing to do",
                       FIRMWARE_VERSION, new_version);
          }
        }
      } else
        ESP_LOGE("FOTA", "unable to download the json file, aborting...");

      esp_http_client_cleanup(client);

      vTaskDelay(30000 / portTICK_PERIOD_MS);
    }
  }
}
