#!/usr/bin/env bash

pwd=$(cd `dirname $0`;pwd)
cd $pwd/
PYTHON=/home/scott/anaconda3/bin/python

$PYTHON tick-rewriter.py run
#sleep 5

#*/1 * * * * bash /home/scott/projects/RapidMarket/scripts/start-host-agent.sh
