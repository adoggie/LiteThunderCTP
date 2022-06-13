#!/usr/bin/env bash
# 平台下发设备控制

for i in {1..1000}
do
redis-cli publish iot.down.pub.DTU-XXX-0001 "{    \"id\":\"\",    \"name\":\"data_set\",   \"values\":{ \"device_id\":\"DTU-XXX-0001\",    \"params\":{        \"sensor_id\":\"door_1\",        \"switch\": \"close\"    }}}"
sleep 3
done
#redis-cli publish iot.down.pub.DTU-XXX-0001 "{    \"id\":\"\",    \"name\":\"data_query\", \"values\":{   \"device_id\":\"DTU-XXX-0001\",    \"params\":[ \"door_1\" ] }} }"
