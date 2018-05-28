# start the workflow engine (register the subscribed task)
# watch folder ./chain/
# bulid eventNotify and workflowServer before running this script

# start workflow server, create ipconfig, ipconfig will be generated at current dir, run workflowserver and eventFlow at same dir

cp ../../src/server/workflowserver .
cp ../../tests/eventNotify .
cp ../../src/operator/operator .

./workflowserver > wfserver.log 2>&1 

#start the eventNotify, this will call the pub-sub api provided by workflow server


./eventNotify .

# start the operator to send the specific event like this 
# operator push <eventlist>


# caculate the time 