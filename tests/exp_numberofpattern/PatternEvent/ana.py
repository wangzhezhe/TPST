



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

sys.path.append('../../../src/publishclient/pythonclient')
import pubsub as pubsubclient

sys.path.append('../../../src/metadatamanagement/pythonclient')
import metaclient

initanastart = timeit.default_timer()

comm = MPI.COMM_WORLD
rank = comm.Get_rank()

appid = 3

serverdir = "/home1/zw241/dataspaces/tests/C"

confpath = serverdir+"/conf*"

copyCommand = "cp "+confpath+" ."

os.system(copyCommand)

# number of clients at clients end to join server
num_peers= 2
appid = 1

var_name = "ex1_sample_data" 
#lock_name = "my_test_lock_"+str(rank)
lock_name = "my_test_lock"

ds = dataspaces.dataspaceClient(appid,comm)

# check the meta periodically
addrList =metaclient.getServerAddr()
addr = addrList[0]
metaclient.Recordtimetick(addr, "SCHEDULE")

anakey = "ANA_"+str(rank)

metaclient.Recordtime(addr, anakey)

#assume those info is required from the metadata

ts = 10
lb=[0]
ub=[15*15*15*(1)-1]
var_name = "ex1_sample_data"

# pull real data once
getdata_p1,rcode = ds.get(var_name, ts, lb, ub)

initanaok = timeit.default_timer()


prepareTime = initanaok-initanastart

actualworkTime = 0.5


# do real analytics

print("prepare time: ",prepareTime)

print("actual work time: ",actualworkTime)