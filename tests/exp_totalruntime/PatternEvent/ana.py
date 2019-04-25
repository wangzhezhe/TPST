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


# pull real data once
getdata_p1,rcode = ds.get(var_name, ts, lb, ub)

time.sleep(5.0)

ds.finalize()

endTime = timeit.default_timer()

print("total running time: ", endTime-startTime)

addrList =metaclient.getServerAddr()
addr = addrList[0]
metaclient.Recordtimetick(addr, "centick")


