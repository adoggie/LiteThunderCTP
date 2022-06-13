#!/usr/bin/env bash

pwd=$(cd `dirname $0`;pwd)
cd $pwd

pkill -f elx-ctp-trader
sleep 2

./cmake-build-debug/elx-ctp-trader ./settings.txt
