### How to use
1. Clone the repo
2. Run the `keyGen.sh` bash script to generate all the keys and put them in the correct directory
3. Compile and flash the ESP (Change your endpoints in the code as well!)
4. Change any usernames, passwords and tokens inside the docker-compose.yml 
5. Start your docker project either with `docker-compose up -d` inside the backend directory or manually in this order 
    - influxdb
    - mosquitto
    - mqtt_consumer
    - grafana
6. Configure the wifi settings on the ESP32 with the provided settings script `settings.sh` inside the esp32 directory using the following syntax. ```./settings.sh {USB_PORT} {SSID} {PASSWORD} {DEVICE_NAME}```
7. Make sure it says NVS successfuly.
8. Either manually restart or send a `r` to the esp over USB.

## Notes
You might need to run `idf.py menuconfig` to set it up for tinyUSB, 2 FOTA partitions.
