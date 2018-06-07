#!/bin/bash



set -e
# read the node list
nodeList="e3c-258 e3c-259 e3c-298"

index=1

echo "required notify number : $1"

for node in ${nodeList}
do

    echo "cd /home1/zw241/observerchain/tests/performance/ && source ~/.grpc && python broadcaster.py $index $1 > $index.log 2>&1 &"
    ssh $node "cd /home1/zw241/observerchain/tests/performance/ && source ~/.grpc && python broadcaster.py $index $1 > $index.log 2>&1 &" &
    
index=$(($index + 1))

echo "current index" $index

done


