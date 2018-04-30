#!/bin/bash

rm ./stdout.log

./eventNotify ./TrigureFiles/ > stdout.log 2>&1 &

sleep 1

python calculatetime.py