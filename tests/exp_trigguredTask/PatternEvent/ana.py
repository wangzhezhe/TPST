



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

comm = MPI.COMM_WORLD
rank = comm.Get_rank()

appid = 3
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

# time it
addrList=metaclient.getServerAddr()
addr = addrList[0]
metaclient.Recordtime(addr, anakey)

# do real analytics