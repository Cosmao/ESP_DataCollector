idf_component_register(SRCS "main.c" "wifi.c" "dht11.c" "https.c"
"usb.c" "nvStorage.c" "settings.c" "mqtt.c" "fota.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES 
                    esp_wifi 
                    esp_timer
                    nvs_flash 
                    esp_driver_gpio
                    esp_http_client
                    esp_https_ota
                    esp-tls
                    mqtt
                    json)
