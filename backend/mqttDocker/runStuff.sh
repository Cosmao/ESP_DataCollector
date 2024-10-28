#!/bin/bash
export SSLKEYLOGFILE="/mosquitto/sslkeyfile"
/usr/sbin/sshd -D &
mosquitto -c /mosquitto/mosquitto.conf
