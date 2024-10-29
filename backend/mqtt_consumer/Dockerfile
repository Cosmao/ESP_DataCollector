# Use a Python base image
FROM python:3.9

COPY ca.crt /certs/ca.crt
COPY mqttConsumer.crt /certs/mqttConsumer.crt
COPY mqttConsumer.key /certs/mqttConsumer.key

# Set the working directory
WORKDIR /app

RUN pip install --upgrade pip

# Copy the requirements file and install dependencies
COPY requirements.txt .
RUN pip install -r requirements.txt

# Copy the Python script
COPY mqtt_to_influx.py .
#Nice comment

# Run the script
CMD ["python", "mqtt_to_influx.py"]
