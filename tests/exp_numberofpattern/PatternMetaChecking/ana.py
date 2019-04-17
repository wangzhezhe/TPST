



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

startanay = timeit.default_timer()

# init dataspace client

# copy all conf.* file to current dir
serverdir = "/home1/zw241/dataspaces/tests/C"

confpath = serverdir+"/conf*"

copyCommand = "cp "+confpath+" ."

os.system(copyCommand)

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
ub=[15*15*15*(1)-1]
var_name = "ex1_sample_data"



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
        
    print("sim is finish, stop")
    print("prepare time: ",prepareTime)
    print("actual work time: ",actualworkTime)
    os._exit(0)


thread = Thread(target = threadFunction)
thread.start()
print("start the thread watching the metaserver for task finish")

initanayok = timeit.default_timer()

prepareTime=prepareTime+initanayok-startanay

while(1):
    startcheck = timeit.default_timer()
    # use consume pattern
    value=metaclient.getMeta(addr, "DATAOK")
    print("get value", value)
    if(value=="NULL"):
        time.sleep(0.5)
        endcheck = timeit.default_timer()
        prepareTime=prepareTime+endcheck-startcheck
        continue
    else:
        # pull real data once
        getdata_p1,rcode = ds.get(var_name, ts, lb, ub)
        endcheck = timeit.default_timer()
        prepareTime=prepareTime+endcheck-startcheck
        actualworkTime=actualworkTime+0.5



ds.finalize()


print("prepare time: ",prepareTime)

print("actual work time: ",actualworkTime)


