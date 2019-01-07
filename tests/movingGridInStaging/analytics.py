from mpi4py import MPI
import numpy as np
import dspaceswrapper.dataspaces as dataspaces
import ctypes
import os
import time
import math
import timeit

comm = MPI.COMM_WORLD
rank = comm.Get_rank()

# input the coordinate of the points and return the index of grid in array


def getIndex(px, py, pz):
    # TODO should add all boundry case
    # only for lower case
    r = 15
    gridnum = 15
    deltar = 1.0*r/gridnum

    if (px < 0 or py < 0 or pz < 0 or px > gridnum*deltar or py > gridnum*deltar or pz > gridnum*deltar):
        #print "out of the box "
        #print [px,py,pz]
        return -1

    gnumx = math.floor((px-0)/deltar)
    gnumy = math.floor((py-0)/deltar)
    gnumz = math.floor((pz-0)/deltar)

    index = int(gnumz*gridnum*gridnum + gnumy*gridnum+gnumx)

    return index


def checkAndPublishEvent(gridDataArray, iteration):
    ifTargetEventHappen = True
    massOriginInterest = [6, 0, 6]
    targetValue = 7.5
    massR = 4
    # put the analysis into the simulation part
    for i in range(massOriginInterest[0], massOriginInterest[0]+massR):
        for j in range(massOriginInterest[1], massOriginInterest[1]+massR):
            for k in range(massOriginInterest[2], massOriginInterest[2]+massR):
                #print "index i j k (%d %d %d)" % (i,j,k)
                #print  nparray[i][j][k]
                #print "index i j k (%d %d %d)" % (i,j,k)
                #print nparray[i][j][k]
                index = getIndex(i, j, k)
                if (gridDataArray[index] != targetValue):
                    ifTargetEventHappen = False
                    break

    if (ifTargetEventHappen == True):
        print (iteration)
        # send publish event
        event = "CUBIC_DETECTED"
        meta = str(iteration)
        detecttime = timeit.default_timer()
        print (detecttime)
        print ("publish to pub/sub broker")
        return
        # pubsubclient.initAddrAndPublish(event,meta)
        ifFirstHappen = True
    return


# copy all conf.* file to current dir
serverdir = "/home1/zw241/dataspaces/tests/C"

confpath = serverdir+"/conf*"

copyCommand = "cp "+confpath+" ."

os.system(copyCommand)

# number of clients at clients end to join server
num_peers = 1
appid = 2

var_name = "ex1_sample_data"
lock_name = "my_test_lock"

# {lb = {0}, ub = {3374}}

iterationNum = 50

ds = dataspaces.dataspaceClient()

ds.dspaces_init(comm, num_peers, appid)

for ver in range(iterationNum):

    ds.dspaces_lock_on_read(lock_name)

    lb = [0]
    ub = [3374]

    getdata = ds.dspaces_get_data(var_name, ver, lb, ub)

    print ("get data")
    print (getdata)

    ds.dspaces_unlock_on_read(lock_name)

    checkAndPublishEvent(getdata, ver)

    # time.sleep(1)


ds.dspaces_wrapper_finalize()
MPI.Finalize()
