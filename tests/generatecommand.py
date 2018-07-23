# generate wraprun command to start the server and the client

import sys
import os

testnum=1


serverstr = "wraprun -n 1 ./workflowserver 1500 "+str(testnum)+" ipogif0 50052 0"
clientstr = "wraprun -n 1 ./eventNotify 128 10 ipogif0 50052 1 0 "+str(testnum)

for x in range(1,testnum):
    #print x
    serverstr = serverstr + " : -n 1 ./workflowserver   1500 "+str(testnum)+" ipogif0 50052 "+str(x)
    clientstr = clientstr+ " : -n 1 ./eventNotify 128 10 ipogif0 50052 1 "+str(x)+ " " + str(testnum)

print serverstr
print "\n"
print clientstr

