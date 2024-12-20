#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "include/dht11.h"
#include "include/fota.h"
#include "include/mqtt.h"
#include "include/settings.h"
#include "include/usb.h"
#include "include/wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "portmacro.h"
#include <stdlib.h>

void app_main(void) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  dht_t *dhtStructPtr = (dht_t *)malloc(sizeof(dht_t));
  ESP_ERROR_CHECK(dhtInit(dhtStructPtr));

  settings_t *settingsPtr = (settings_t *)malloc(sizeof(settings_t));
  ESP_ERROR_CHECK(settingsInit(settingsPtr));

  settingsPtr->dht = dhtStructPtr;

  if (!wifiInitStation(settingsPtr)) {
    ESP_LOGE("WIFI", "Wifi failed to connect");
  }

  TaskHandle_t mqttHandle = NULL;
  BaseType_t taskRet =
      xTaskCreate(&mqttTask, "MQTT task", 8192, settingsPtr, 4, &mqttHandle);
  if (taskRet == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) {
    vTaskDelete(mqttHandle);
    ESP_LOGE("HTTP", "Could not allocate memory for task");
    esp_restart();
  }

  TaskHandle_t dhtHandle = NULL;
  taskRet =
      xTaskCreate(&dhtTask, "DHT task", 4096, dhtStructPtr, 5, &dhtHandle);
  if (taskRet == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) {
    vTaskDelete(dhtHandle);
    ESP_LOGE("DHT", "Could not allocate memory for task");
    esp_restart();
  }

  TaskHandle_t usbHandle = NULL;
  taskRet = xTaskCreate(&usbTask, "USB Task", 4096, settingsPtr, 3, &usbHandle);
  if (taskRet == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) {
    vTaskDelete(usbHandle);
    ESP_LOGE("USB", "Could not allocate memory for task");
    esp_restart();
  }

  TaskHandle_t fotaHandle = NULL;
  taskRet =
      xTaskCreate(&fotaTask, "Fota Task", 4096, dhtStructPtr, 4, &dhtHandle);
  if (taskRet == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) {
    vTaskDelete(fotaHandle);
    ESP_LOGE("FOTA", "Could not allocate memory for task");
    esp_restart();
  }
}
