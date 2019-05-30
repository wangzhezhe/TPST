



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
import random
import threading
from threading import Lock  

sys.path.append('../../../src/publishclient/pythonclient')
import pubsub as pubsubclient

sys.path.append('../../../src/metadatamanagement/pythonclient')
import metaclient

gridnum=50

comm = MPI.COMM_WORLD
rank = comm.Get_rank()





appid = random.randint(1,100)
ds = dataspaces.dataspaceClient(appid,comm)

# copy all conf.* file to current dir
serverdir = "/home1/zw241/dataspaces/tests/C"

confpath = serverdir+"/conf*"

copyCommand = "cp "+confpath+" ."

os.system(copyCommand)  

# check the meta periodically
addrList =metaclient.getServerAddr()
addr = addrList[0]



ts = 11
lb=[0]

ub=[gridnum*gridnum*gridnum*(1)-1]
var_name = "ex1_sample_data"

mutex = threading.Lock()
runningTask=0

def threadCheckFinish():

    # check if there is finish meta data
    addrList=metaclient.getServerAddr()
    addr = addrList[0]

    while(1):
        value=metaclient.getMeta(addr, "FINISH")
        if(value=="NULL"):
            print("not finish")
            time.sleep(0.5)
            continue
        else:
            print("sim finish")
            while(1):
                if(runningTask==0):
                    metaclient.Recordtimetick(addr, "TOTAL")
                    os._exit(0)
                else:
                    time.sleep(0.1)
                    print("running task ", runningTask)
                    continue
    


# if the value is not NULL
def threadFunction():

    global runningTask

    getdata_p1,rcode = ds.get(var_name, ts, lb, ub)

    time.sleep(1)

    mutex.acquire()
    runningTask=runningTask-1
    mutex.release()
    print("task finish, task num ", runningTask)


thread = threading.Thread(target = threadCheckFinish) 
thread.start()

while(1):
    value=metaclient.getMeta(addr, "DATAOK")
    if(value=="NULL"):
        time.sleep(0.1)
        continue
    else:
        # start a new task to execute
        mutex.acquire()
        runningTask=runningTask+1
        mutex.release()
        thread = threading.Thread(target = threadFunction) 
        thread.start()
        print("start new task, task num ", runningTask)


