#include "include/usb.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/idf_additions.h"
#include "include/nvStorage.h"
#include "include/settings.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include <stdio.h>

static interpret_ret interpretInput(char *str, settings_t *settings) {
  if (xSemaphoreTake(settings->settingsMutex, (TickType_t)10) == pdTRUE) {
    interpret_ret ret = INTERP_BAD_DATA;
    switch (str[0]) {
    case 's':
      snprintf(settings->SSID, settingsBufferSize, "%s", (str + 1));
      ret = INTERP_OK_SSID;
      break;
    case 'p':
      snprintf(settings->password, settingsBufferSize, "%s", (str + 1));
      ret = INTERP_OK_PW;
      break;
    case 'n':
      snprintf(settings->name, settingsBufferSize, "%s", (str + 1));
      ret = INTERP_OK_NAME;
      break;
    case 'r':
      ret = INTERP_RESTART;
      break;
    case 'c':
      ret = INTERP_COMMIT;
      break;
    case 'g':
      ret = INTERP_REQ_PRINT;
      break;
    }
    xSemaphoreGive(settings->settingsMutex);
    return ret != INTERP_BAD_DATA ? ret : INTERP_BAD_DATA;
  }
  return INTERP_NO_MUTEX;
}

void usbTask(void *pvParameter) {
  settings_t *settingsPtr = (settings_t *)pvParameter;

  const tinyusb_config_t tusb_cfg = {
      .device_descriptor = NULL,
      .string_descriptor = NULL,
      .external_phy =
          false, // In the most cases you need to use a `false` value
      .configuration_descriptor = NULL,
  };

  ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

  tinyusb_config_cdcacm_t acm_cfg = {
      0}; // the configuration uses default values

  ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));

  char buffer[settingsBufferSize];
  // TODO: Read character by character, encounting a NULL or newline? switch on
  // it and treat the input accordingly
  while (1) {
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
      interpretInput(buffer, settingsPtr);
      switch (interpretInput(buffer, settingsPtr)) {
      case INTERP_OK_SSID:
        ESP_LOGI("USB", "SSID OK: %s", settingsPtr->SSID);
        break;
      case INTERP_OK_NAME:
        ESP_LOGI("USB", "NAME OK: %s", settingsPtr->name);
        break;
      case INTERP_OK_PW:
        ESP_LOGI("USB", "PW OK: %s", settingsPtr->password);
        break;
      case INTERP_COMMIT:
        nvsCommitAll(settingsPtr);
        break;
      case INTERP_RESTART:
        esp_restart();
        break;
      case INTERP_BAD_DATA:
        ESP_LOGI("USB", "Bad data");
        break;
      case INTERP_REQ_PRINT:
        ESP_LOGI("USB", "Current settings\nSSID: %s\nPW: %s\nName: %s",
                 settingsPtr->SSID, settingsPtr->password, settingsPtr->name);
        break;
      case INTERP_NO_MUTEX:
        ESP_LOGE("USB", "Could not get mutex");
        break;
      }
    }
    vTaskDelay((2 * 1000) / portTICK_PERIOD_MS);
  }
}
