#ifndef settings_h
#define settings_h
#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "include/dht11.h"

#define settingsBufferSize 64

typedef struct settings_t {
  char name[settingsBufferSize];
  char SSID[settingsBufferSize];
  char password[settingsBufferSize];
  dht_t *dht;
  SemaphoreHandle_t settingsMutex;
} settings_t;

esp_err_t settingsInit(settings_t *settings);

#endif // !settings_h
