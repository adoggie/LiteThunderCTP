#!/usr/bin/env bash

pwd=$(cd `dirname $0`;pwd)
cd $pwd

pkill -f twap_step
sleep 2

/Users/admin/anaconda3/bin/python twap_step1.py run

