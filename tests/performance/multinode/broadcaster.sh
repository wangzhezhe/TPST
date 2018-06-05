#!/bin/bash



set -e
# read the node list
nodeList="e3c-465 e3c-466 e3c-467 e3c-468 e3c-469"

index=1

for node in ${nodeList}
do

    echo "cd /scratch/zw241/ && source ~/.grpc && python broadcaster.py $index > $index.log 2>&1 &"
    ssh $node "cd /scratch/zw241/ && source ~/.grpc && python broadcaster.py $index > $index.log 2>&1 &" &
    
index=$(($index + 1))

echo "current index" $index

done


