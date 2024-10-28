FROM ubuntu:latest

RUN apt-get update && \
    apt-get install -y openssh-server curl ca-certificates mosquitto && \
    apt-get install -y python3 && \
    apt-get install -y python3-paho-mqtt && \
    mkdir /var/run/sshd

RUN echo 'root:password' | chpasswd && \
    sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config && \
    sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/' /etc/ssh/sshd_config

RUN mkdir /mosquitto/

ADD certs /mosquitto/

# Set appropriate permissions for Mosquitto to access certificates and log files
RUN chown mosquitto:mosquitto /mosquitto/* && chmod 640 /mosquitto/server.key /mosquitto/client.key
RUN touch /mosquitto/mosquitto.log && \
    chown mosquitto:mosquitto /mosquitto/mosquitto.log && \
    chmod 640 /mosquitto/mosquitto.log && \
    touch /mosquitto/sslkeys.log && \
    chown mosquitto:mosquitto /mosquitto/sslkeys.log && \
    chmod 666 /mosquitto/sslkeys.log

ENV SSLKEYLOGFILE=/mosquitto/sslkeys.log

COPY mosquitto.conf /mosquitto/mosquitto.conf
COPY runStuff.sh /mosquitto/runStuff.sh

RUN chmod +x /mosquitto/runStuff.sh

RUN echo "SSLKEYLOGFILE IS $SSLKEYLOGFILE"

EXPOSE 22
EXPOSE 8883
EXPOSE 33073

CMD /mosquitto/runStuff.sh
