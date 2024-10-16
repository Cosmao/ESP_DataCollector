#include "include/settings.h"
#include "esp_log.h"
#include "include/nvStorage.h"
#include "projdefs.h"

esp_err_t settingsInit(settings_t *settings) {
  if (settings == NULL) {
    return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
  }
  settings->settingsMutex = xSemaphoreCreateMutex();
  if (settings->settingsMutex == NULL) {
    ESP_LOGE("MUTEX", "Mutex creation failed");
    return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
  }
  nvsReadErrCheck(nvsRead(ESP_ssid, settings->SSID, sizeof(settings->SSID)));
  nvsReadErrCheck(
      nvsRead(ESP_pw, settings->password, sizeof(settings->password)));
  nvsReadErrCheck(nvsRead(ESP_Name, settings->name, sizeof(settings->name)));
  return ESP_OK;
}
