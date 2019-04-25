



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

# check the meta periodically
addrList =metaclient.getServerAddr()
addr = addrList[0]
metaclient.Recordtimetick(addr, "ANASTART")


