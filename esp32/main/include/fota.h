#ifndef fota__h
#define fota__h

void checkForFOTA(void);
void fotaTask(void *pvParameter);

typedef enum fota_err_t {
  FOTA_OK,
  FOTA_JSON_NO_JSON,
  FOTA_JSON_NO_VERSION,
  FOTA_JSON_SAME_VERSION,
  FOTA_JSON_URL_ERROR,
} fota_err_t;

#endif // !fota__h
