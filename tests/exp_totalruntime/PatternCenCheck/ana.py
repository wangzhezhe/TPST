# check meta periodically
# use tuple space pattern
# if there is finish key,just finish
# watch key info in metaserver
# get metadata
# fetch the real data
# record the time

from mpi4py import MPI
import numpy as np
import dataspaces.dataspaceClient as dataspaces
import ctypes
import os
import time
import math
import timeit
import sys
from threading import Thread

sys.path.append('../../../src/publishclient/pythonclient')
import pubsub as pubsubclient

sys.path.append('../../../src/metadatamanagement/pythonclient')
import metaclient


prepareTime=0
actualworkTime=0

startTime = timeit.default_timer()

# init dataspace client

# copy all conf.* file to current dir
#serverdir = "/home1/zw241/dataspaces/tests/C"

#confpath = serverdir+"/conf*"

#copyCommand = "cp "+confpath+" ."

#os.system(copyCommand)

var_name = "ex1_sample_data" 
#lock_name = "my_test_lock_"+str(rank)
lock_name = "my_test_lock"


comm = MPI.COMM_WORLD
rank = comm.Get_rank()

appid = 3
ds = dataspaces.dataspaceClient(appid,comm)

# check the meta periodically
addrList =metaclient.getServerAddr()
addr = addrList[0]


ts = 10
lb=[0]
ub=[150*150*150*(1)-1]
var_name = "ex1_sample_data"


if (len(sys.argv)!=2):
    print("simulation <iteration>")
    exit(0)

iteration = int(sys.argv[1])

version = 0

def threadFunction():

    # check the meta periodically
    addrList =metaclient.getServerAddr()
    addr = addrList[0]

    # if the value is not NULL

    while(1):
        value=metaclient.getMeta(addr, "FINISH")
        if(value=="NULL"):
            time.sleep(0.1)
            continue
        else:
            break
        
    endTime = timeit.default_timer()
    print("total running time: ", endTime-startTime)

    # record time
    metaclient.Recordtimetick(addr, "centick")
    # exit only when the vertion is larger than iteration
    if(version>=iteration):
        os._exit(0)


thread = Thread(target = threadFunction)
thread.start()
print("start the thread watching the metaserver for task finish")



while (version<iteration):
    startcheck = timeit.default_timer()
    # use consume pattern
    value=metaclient.getMeta(addr, "DATAOK")
    print("get value", value)
    if(value=="NULL"):
        time.sleep(0.1)
        endcheck = timeit.default_timer()
        prepareTime=prepareTime+endcheck-startcheck
        continue
    else:
        # pull real data once
        getdata_p1,rcode = ds.get(var_name, ts, lb, ub)
        # execute analysing based on data
        time.sleep(5.0)
        version=version+1




ds.finalize()

endTime = timeit.default_timer()

print("total running time: ", endTime-startTime)

addrList =metaclient.getServerAddr()
addr = addrList[0]
metaclient.Recordtimetick(addr, "centick")


