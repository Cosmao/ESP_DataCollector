#include "esp_http_client.h"

#define FIRMWARE_VERSION 0.1
#define UPDATE_JSON_URL                                                        \
  "https://raw.githubusercontent.com/Cosmao/FOTATest/master/bin/"              \
  "firmware.json"

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
