### Stuff to do better
- better AWS security
- s3 bucket
- ringbuffer for temp values
- get time and timestamp in the buffer to send as batch 
- deep sleep
- Proper CN keys for mqtt, currently ignoring the CN
- Signing binaries (Not hard, just seemed tedious during development)
- Secure boot (Requires burning eFUSE)
- Marking bad firmware as unusuable so it rolls back to a working FOTA update

### Currently using
- mTLS on mqtt
- Only exposed ports are:
    - 3000 (grafana)
    - 8086 (influxdb)
    - 8883 (mqtt TLS)
- Least privilgied access token for grafana to influxdb
- Container dependency with logging
- FOTA support (Currently using github for storage and proper HTTPS certs)
