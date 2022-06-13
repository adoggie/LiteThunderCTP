#!/usr/bin/env bash

# 从dtu设备端接受来自平台 Mx -> GwServer-> Monkey -> Nanomsg 的消息
nanocat --sub --connect tcp://127.0.0.1:8901 -v --ascii --subscribe door_1

