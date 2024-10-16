#include "include/https.h"
#include "esp_crt_bundle.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "esp_wifi.h"
#include "freertos/idf_additions.h"
#include "include/dht11.h"
#include "include/wifi.h"
#include "portmacro.h"
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

static const char *HTTPTAG = "HTTP";

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048
#define MAX_STR_BUFFER 128

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
  static char *output_buffer; // Buffer to store response of http request from
                              // event handler
  static int output_len;      // Stores number of bytes read
  switch (evt->event_id) {
  case HTTP_EVENT_ERROR:
    ESP_LOGD(HTTPTAG, "HTTP_EVENT_ERROR");
    break;
  case HTTP_EVENT_ON_CONNECTED:
    ESP_LOGD(HTTPTAG, "HTTP_EVENT_ON_CONNECTED");
    break;
  case HTTP_EVENT_HEADER_SENT:
    ESP_LOGD(HTTPTAG, "HTTP_EVENT_HEADER_SENT");
    break;
  case HTTP_EVENT_ON_HEADER:
    ESP_LOGD(HTTPTAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key,
             evt->header_value);
    break;
  case HTTP_EVENT_ON_DATA:
    ESP_LOGD(HTTPTAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
    // Clean the buffer in case of a new request
    if (output_len == 0 && evt->user_data) {
      // we are just starting to copy the output data into the use
      memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
    }
    /*
     *  Check for chunked encoding is added as the URL for chunked encoding used
     * in this example returns binary data. However, event handler can also be
     * used in case chunked encoding is used.
     */
    if (!esp_http_client_is_chunked_response(evt->client)) {
      // If user_data buffer is configured, copy the response into the buffer
      int copy_len = 0;
      if (evt->user_data) {
        // The last byte in evt->user_data is kept for the NULL character in
        // case of out-of-bound access.
        copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
        if (copy_len) {
          memcpy(evt->user_data + output_len, evt->data, copy_len);
        }
      } else {
        int content_len = esp_http_client_get_content_length(evt->client);
        if (output_buffer == NULL) {
          // We initialize output_buffer with 0 because it is used by strlen()
          // and similar functions therefore should be null terminated.
          output_buffer = (char *)calloc(content_len + 1, sizeof(char));
          output_len = 0;
          if (output_buffer == NULL) {
            ESP_LOGE(HTTPTAG,
                     "Failed to allocate memory for output buffer with size %d",
                     content_len);
            return ESP_FAIL;
          }
        }
        copy_len = MIN(evt->data_len, (content_len - output_len));
        if (copy_len) {
          memcpy(output_buffer + output_len, evt->data, copy_len);
        }
      }
      output_len += copy_len;
    }

    break;
  case HTTP_EVENT_ON_FINISH:
    ESP_LOGD(HTTPTAG, "HTTP_EVENT_ON_FINISH");
    if (output_buffer != NULL) {
      // Response is accumulated in output_buffer. Uncomment the below line to
      // print the accumulated response ESP_LOG_BUFFER_HEX(TAG, output_buffer,
      // output_len);
      ESP_LOGI(HTTPTAG, "%s", output_buffer);
      free(output_buffer);
      output_buffer = NULL;
    }
    output_len = 0;
    break;
  case HTTP_EVENT_DISCONNECTED:
    ESP_LOGI(HTTPTAG, "HTTP_EVENT_DISCONNECTED");
    int mbedtls_err = 0;
    esp_err_t err = esp_tls_get_and_clear_last_error(
        (esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
    if (err != 0) {
      ESP_LOGI(HTTPTAG, "Last esp error code: 0x%x", err);
      ESP_LOGI(HTTPTAG, "Last mbedtls failure: 0x%x", mbedtls_err);
    }
    if (output_buffer != NULL) {
      free(output_buffer);
      output_buffer = NULL;
    }
    output_len = 0;
    break;
  case HTTP_EVENT_REDIRECT:
    ESP_LOGD(HTTPTAG, "HTTP_EVENT_REDIRECT");
    esp_http_client_set_header(evt->client, "From", "user@example.com");
    esp_http_client_set_header(evt->client, "Accept", "text/html");
    esp_http_client_set_redirection(evt->client);
    break;
  }
  return ESP_OK;
}

static esp_err_t getEndpointURL(char *array, uint8_t maxSize) {
  uint8_t mac[6];
  esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);

  /*  snprintf(array, maxSize,
             "https://myapp-latest-lpiy.onrender.com/sensor/add/"
             "%0x2:%0x2:%0x2:%0x2:%0x2:%0x2",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);*/
  snprintf(array, maxSize, "https://httpbin.org/post");
  return ESP_OK;
}

static esp_err_t setupPost(esp_http_client_handle_t *client,
                           settings_t *settingsPtr) {
  dht_t *dht = settingsPtr->dht;
  esp_http_client_set_method(*client, HTTP_METHOD_POST);
  esp_http_client_set_header(*client, "Content-Type", "application/json");
  char post[MAX_STR_BUFFER];
  if (xSemaphoreTake(settingsPtr->mutex, (TickType_t)10) == pdTRUE) {
    snprintf(post, MAX_STR_BUFFER,
             "{\"temperature\":%.1f,\"humidity\":%.1f,\"name\":\"EmilESP\"}",
             getDHTValue(&dht->temperature), getDHTValue(&dht->humidity));
    dht->sent = true;
    xSemaphoreGive(settingsPtr->mutex);
    ESP_LOGI(HTTPTAG, "%s", post);
    esp_http_client_set_post_field(*client, post, strlen(post));

    return ESP_OK;
  } else {
    ESP_LOGE(HTTPTAG, "Could not aquire mutex");
    return ESP_FAIL;
  }
}

static void httpsSendMessage(esp_http_client_config_t *config,
                             settings_t *settingsPtr) {
  esp_http_client_handle_t client = esp_http_client_init(config);
  esp_err_t err = setupPost(&client, settingsPtr);
  if (err != ESP_OK) {
    esp_http_client_cleanup(client);
    return;
  }
  err = esp_http_client_perform(client);
  if (err == ESP_OK) {
    ESP_LOGI(HTTPTAG, "HTTPS Status = %d, content_length = %" PRId64,
             esp_http_client_get_status_code(client),
             esp_http_client_get_content_length(client));
  } else {
    ESP_LOGE(HTTPTAG, "Error perform http request %s", esp_err_to_name(err));
  }
  esp_http_client_cleanup(client);
}

void httpsTask(void *pvParameter) {
  settings_t *settingsPtr = (settings_t *)pvParameter;
  if (!wifiInitStation(settingsPtr)) {
    ESP_LOGE("WIFI", "Could not establish wifi connection!");
    vTaskDelete(NULL);
  }

  dht_t *dht = settingsPtr->dht;
  char url[MAX_STR_BUFFER];
  getEndpointURL(url, MAX_STR_BUFFER);

  esp_http_client_config_t config = {
      .url = url,
      .event_handler = _http_event_handler,
      .crt_bundle_attach = esp_crt_bundle_attach,
      .transport_type = HTTP_TRANSPORT_OVER_SSL,
  };

  while (true) {
    if (!dht->sent) {
      httpsSendMessage(&config, settingsPtr);
    }
    vTaskDelay((updateTime * 1000) / portTICK_PERIOD_MS);
  }
}
