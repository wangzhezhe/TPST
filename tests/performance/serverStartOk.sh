#!/bin/bash
# wait server to start
sleep 1
count=$1
num_server=0
while [ "$num_server" -ne "$count" ]; do
    num_server=$(ls ./multinodeip -lR| grep "^-" | wc -l)
    echo $num_server
    sleep 1
done
echo "All Started"