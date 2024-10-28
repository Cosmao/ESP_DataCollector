import ssl
import paho.mqtt.client as mqtt
import time

# Define callback functions
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to broker")
    else:
        print(f"Connection failed with code {rc}")

def on_message(client, userdata, message):
    print(f"Received message: {message.payload.decode()}")

# Create an MQTT client instance
client = mqtt.Client()

ssl_keylogfile = "sslkeylogfile.txt"

context = ssl.create_default_context(ssl.Purpose.SERVER_AUTH, cafile="./mqttDocker/certs/ca.crt")
context.load_cert_chain(certfile="./mqttDocker/certs/client.crt", keyfile="./mqttDocker/certs/client.key")
context.check_hostname = False

context.keylog_filename = ssl_keylogfile
client.tls_set_context(context)

# Assign callback functions
client.on_connect = on_connect
client.on_message = on_message

# Connect to the broker
client.connect("localhost", 8883)

# Start the loop to process network traffic
client.loop_start()

# Subscribe to a topic
client.subscribe("test/topic")

try:
    while True:
        # Publish a message every 10 seconds
        client.publish("test/topic", "Hello, MQTT with SSL!")
        time.sleep(5)  # Sleep for 10 seconds
except KeyboardInterrupt:
    print("Exiting...")

# Stop the loop after some time (optional)
client.loop_stop()
