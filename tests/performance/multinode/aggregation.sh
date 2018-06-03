#!/bin/bash



set -e
# read the node list
nodeList="e3c-573 e3c-574 e3c-575"

index=1

for node in ${nodeList}
do

    echo "cd /home1/zw241/observerchain/tests/performance && source ~/.grpc && python parallelmap.py $index > $index.log 2>&1 &"
    ssh $node "cd /home1/zw241/observerchain/tests/performance && source ~/.grpc && python parallelmap.py $index > $index.log 2>&1 &" &
    
index=$(($index + 1))

echo "current index" $index

done



# node2 start the operator INIT

# node 3 4 5  ssh to every node and run eventNotify
