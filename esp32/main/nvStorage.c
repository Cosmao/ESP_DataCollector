#include "include/nvStorage.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs.h"

esp_err_t nvsRead(const char *key, char *buffer, size_t buffSize) {
  nvs_handle_t nvsHandle;
  esp_err_t ret = nvs_open("storage", NVS_READWRITE, &nvsHandle);
  if (ret == ESP_OK) {
    ret = nvs_get_str(nvsHandle, key, buffer, &buffSize);
    switch (ret) {
    case ESP_OK:
      break;
    case ESP_ERR_NVS_NOT_FOUND:
      buffer[0] = '\0';
      break;
    }
  }
  nvs_close(nvsHandle);
  return ret;
}

esp_err_t nvsCommit(const char *key, char *buffer) {
  nvs_handle_t nvsHandle;
  esp_err_t ret = nvs_open("storage", NVS_READWRITE, &nvsHandle);
  if (ret == ESP_OK) {
    ret = nvs_set_str(nvsHandle, key, buffer);
    if (ret != ESP_OK) {
      ESP_LOGE("NVS", "%s set failed", key);
    }
    ret = nvs_commit(nvsHandle);
    if (ret != ESP_OK) {
      ESP_LOGE("NVS", "commit failed");
      nvs_close(nvsHandle);
      return ret;
    }
  }
  nvs_close(nvsHandle);
  return ret;
}

void nvsCommitAll(settings_t *settings) {
  esp_err_t ret;
  ret = nvsCommit(ESP_ssid, settings->SSID);
  if (ret != ESP_OK) {
    return;
  }
  ret = nvsCommit(ESP_pw, settings->password);
  if (ret != ESP_OK) {
    return;
  }
  ret = nvsCommit(ESP_Name, settings->name);
  if (ret != ESP_OK) {
    return;
  }
  ESP_LOGI("NVS", "All commit good");
}

void nvsReadErrCheck(esp_err_t ret) {
  switch (ret) {
  case ESP_OK:
    return;
  case ESP_ERR_NVS_NOT_FOUND:
    ESP_LOGE("NVS", "Key not found");
    break;
  case ESP_ERR_NVS_INVALID_HANDLE:
    ESP_LOGE("NVS", "Invalid handle");
    break;
  }
}
