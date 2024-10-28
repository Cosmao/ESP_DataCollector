## Todo
- AWS
- Om tid finns, FIXA USB SKITEN

### How to use
Compile or flash the ESP code \
Docker stuff in this order \
```docker-compose up -d influxdb``` \
Setup your influxdb login and either name your bucket to buckit or you'll have to change it in a lot of places \
Get the full access token and add it to your docker-compose for mqtt_consumer \
After that you can run \
```docker-compose up -d``` \
If all endpoints are setup and access tokens influxdb should start seeing data
