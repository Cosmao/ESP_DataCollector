#ifndef nvStorage_h
#define nvStorage_h

#include "esp_err.h"
#include "include/settings.h"

esp_err_t nvsRead(const char *key, char *buffer, size_t buffSize);
esp_err_t nvsCommit(const char *key, char *buffer);
void nvsCommitAll(settings_t *settings);
void nvsReadErrCheck(esp_err_t ret);

static const char *ESP_ssid = "SSID";
static const char *ESP_pw = "PASSWORD";
static const char *ESP_Name = "NAME";

#endif // !nvStorage_h
