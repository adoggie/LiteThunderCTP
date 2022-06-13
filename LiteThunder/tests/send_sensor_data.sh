#!/usr/bin/env bash

nanocat --pub --connect tcp://127.0.0.1:8902  --data "{\"message\":\"data_report\",\"sensor_id\":\"door_1\", \"params\":{\"switch\":\"open\"} }" -i 2
