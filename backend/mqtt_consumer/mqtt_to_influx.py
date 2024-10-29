import os
import json
import ssl
import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient

# Environment variables
MQTT_BROKER = os.getenv('MQTT_BROKER', 'localhost')
MQTT_PORT = int(os.getenv('MQTT_PORT', 8883))
INFLUXDB_HOST = os.getenv('INFLUXDB_HOST', 'localhost')
INFLUXDB_PORT = int(os.getenv('INFLUXDB_PORT', 8086))
INFLUXDB_DB = os.getenv('INFLUXDB_DB', 'buckit')
INFLUXDB_TOKEN = os.getenv('INFLUXDB_TOKEN')  
#file = open(INFLUXDB_TOKEN_LOCATION, "r")
#line = file.read()
#line = line[20:]
#INFLUXDB_TOKEN = line.strip()
#file.close()
MQTT_TOPIC = os.getenv('MQTT_TOPIC', 'sensors/#')  # Subscribe to all topics under "sensors/"

# Certificate paths
CA_CERT = '/certs/ca.crt'
CLIENT_CERT = '/certs/mqttConsumer.crt'
CLIENT_KEY = '/certs/mqttConsumer.key'

# Connect to InfluxDB
influx_client = InfluxDBClient(
    host=INFLUXDB_HOST,
    port=INFLUXDB_PORT,
    password=INFLUXDB_TOKEN,      # use the token as the password
    database=INFLUXDB_DB
)
influx_client.switch_database(INFLUXDB_DB)

print(f"Connected to DB with token: `{INFLUXDB_TOKEN}`")

def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT broker with code {rc}")
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    print(f"Received `{payload}` from `{msg.topic}` topic")

    # Parse the JSON payload
    try:
        data = json.loads(payload)  # Convert JSON string to Python dictionary
    except json.JSONDecodeError:
        print("Failed to decode JSON")
        return

    # Prepare data for InfluxDB
    influx_data = []
    measurement = "sensor_data"  # Measurement name

    # Create a point for each key-value pair in the JSON data
    for key, value in data.items():
        influx_data.append({
            "measurement": measurement,
            "tags": {
                "topic": msg.topic,
                "key": key  # Optional: add key as a tag if desired
            },
            "fields": {
                "value": float(value) if isinstance(value, (int, float)) else str(value)  # Convert to float if it's a number
            }
        })

    # Write the points to InfluxDB
    influx_client.write_points(influx_data)

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
