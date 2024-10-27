#ifndef fota__h
#define fota__h

void check_update_task(void *pvParameter);
void checkForFOTA(void);

typedef enum fota_err_t {
  FOTA_OK,
  FOTA_JSON_NO_JSON,
  FOTA_JSON_NO_VERSION,
  FOTA_JSON_SAME_VERSION,
  FOTA_JSON_URL_ERROR,
} fota_err_t;

#endif // !fota__h
