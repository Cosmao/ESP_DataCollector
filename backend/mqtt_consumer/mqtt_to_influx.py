import os
import ssl
import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient

# Environment variables
MQTT_BROKER = os.getenv('MQTT_BROKER', 'localhost')
MQTT_PORT = int(os.getenv('MQTT_PORT', 8883))
INFLUXDB_HOST = os.getenv('INFLUXDB_HOST', 'localhost')
INFLUXDB_PORT = int(os.getenv('INFLUXDB_PORT', 8086))
INFLUXDB_DB = os.getenv('INFLUXDB_DB', 'your_database_name')
MQTT_TOPIC = os.getenv('MQTT_TOPIC', 'sensors/#')  # Subscribe to all topics under "sensors/"

# Certificate paths
CA_CERT = '/certs/ca.crt'
CLIENT_CERT = '/certs/mqttConsumer.crt'
CLIENT_KEY = '/certs/mqttConsumer.key'

# Connect to InfluxDB
influx_client = InfluxDBClient(host=INFLUXDB_HOST, port=INFLUXDB_PORT)
influx_client.switch_database(INFLUXDB_DB)

def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT broker with code {rc}")
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    print(f"Received `{payload}` from `{msg.topic}` topic")

    # Prepare data for InfluxDB
    data = [
        {
            "measurement": "mqtt_data",
            "tags": {
                "topic": msg.topic,
            },
            "fields": {
                "value": float(payload) if payload.replace('.', '', 1).isdigit() else payload
            }
        }
    ]
    influx_client.write_points(data)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

context = ssl.create_default_context(ssl.Purpose.SERVER_AUTH, cafile=CA_CERT)
context.load_cert_chain(certfile=CLIENT_CERT, keyfile=CLIENT_KEY)
context.check_hostname = False
client.tls_set_context(context)

# Connect to the MQTT broker
client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_forever()
