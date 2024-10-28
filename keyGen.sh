cd backend/mqttDocker/certs

# Generate server key
openssl genrsa -out server.key 2048

# Generate CA certificate with country and organization
openssl req -new -x509 -days 3650 -key server.key -out ca.crt -subj "/C=SE/O=iot23/CN=localhost"

# Generate client key
openssl genrsa -out client.key 2048

# Create client certificate signing request with country and organization
openssl req -new -key client.key -out client.csr -subj "/C=SE/O=iot23/CN=localhost"

# Sign the client certificate with the CA certificate
openssl x509 -req -in client.csr -CA ca.crt -CAkey server.key -CAcreateserial -out client.crt -days 3650

# copy to the ESP dir
cp ca.crt ../../../esp32/main/
cp client.crt ../../../esp32/main/
cp client.key ../../../esp32/main/
