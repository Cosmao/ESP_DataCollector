if [ ! -d backend/certs ]; then
  mkdir -p backend/certs
fi

cd backend/certs

# Generate server key
openssl genrsa -out server.key 2048

# Generate CA certificate with country and organization
openssl req -new -x509 -days 3650 -key server.key -out ca.crt -subj "/C=SE/O=iot23/CN=localhost"

# Generate client key
openssl genrsa -out client.key 2048
openssl genrsa -out mqttConsumer.key 2048

# Create client certificate signing request with country and organization
openssl req -new -key client.key -out client.csr -subj "/C=SE/O=iot23/CN=localhost"
openssl req -new -key mqttConsumer.key -out mqttConsumer.csr -subj "/C=SE/O=iot23/CN=localhost"

# Sign the client certificate with the CA certificate
openssl x509 -req -in client.csr -CA ca.crt -CAkey server.key -CAcreateserial -out client.crt -days 3650
openssl x509 -req -in mqttConsumer.csr -CA ca.crt -CAkey server.key -CAcreateserial -out mqttConsumer.crt -days 3650

# copy to the ESP dir
if [ -d ../../esp32/main ]; then
  cp ca.crt ../../esp32/main/
  cp client.crt ../../esp32/main/
  cp client.key ../../esp32/main/
else
  echo "ERROR: Could not find esp32 directory for keys!"
fi

# Copy to mqttConsumer 
if [ -d ../mqtt_consumer/ ]; then
  cp ca.crt ../mqtt_consumer/
  cp mqttConsumer.crt ../mqtt_consumer/
  cp mqttConsumer.key ../mqtt_consumer/
else
  echo "ERROR: Could not find mqttConsumer directory for keys!"
fi

# Copy to mosquitto
if [ -d ../mosquitto/ ]; then
  cp ca.crt ../mosquitto/
  cp server.key ../mosquitto/
else
  echo "ERROR: Could not find mqttConsumer directory for keys!"
fi
