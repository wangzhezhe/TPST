# write to staging service

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import math
import numpy
from pyevtk.hl import imageToVTK
import os
import shutil
import copy
import random
import time
import ctypes
from mpi4py import MPI
import dataspaces.dataspaceClient as dataspaces


import sys
# insert pubsub and detect the things after every iteration
sys.path.append('../../../src/publishclient/pythonclient')
import pubsub as pubsubclient
import timeit

sys.path.append('../../../src/metadatamanagement/pythonclient')
import metaclient



startsim = timeit.default_timer()

comm = MPI.COMM_WORLD
rank = comm.Get_rank()




# init dataspace client

# copy all conf.* file to current dir
serverdir = "/home1/zw241/dataspaces/tests/C"

confpath = serverdir+"/conf*"

copyCommand = "cp "+confpath+" ."

os.system(copyCommand)

# number of clients at clients end to join server
num_peers= 2
appid = 1

var_name = "ex1_sample_data" 
lock_name = "my_test_lock"


ds = dataspaces.dataspaceClient(appid,comm)
pubsubaddrList = pubsubclient.getServerAddr()
print (pubsubaddrList)
pubsubAddr = pubsubaddrList[0]

#pubsubaddrList = pubsubclient.getServerAddr()
#print (pubsubaddrList)

#pubsubAddr = pubsubaddrList[0]

def putDataToDataSpaces(gridList,timestep):

    cellDataArray=[]
    for i in range (len(gridList)):
        #print gridList[i].p
        cellDataArray.append(gridList[i].p*1.0)

    #ds.dspaces_lock_on_write(lock_name)

    # elemsize = ctypes.sizeof(ctypes.c_double)
    # data = ([[1.1,2.2,3.3],[4.4,5.5,6.6]])

    # dataarray = (ver+1)*numpy.asarray(data)
    ver = timestep
    
    # data is 1 d array
    if(rank==0):
        lb = [0]
    if (rank ==1):
        lb = [3380]
    
    #ds.lock_on_write(lock_name)
    ds.put(var_name,ver,lb,cellDataArray)
    #ds.unlock_on_write(lock_name)
    #print("write to dataspaces for ts %d" % (timestep))


def sendEventToPubSub(pubsubAddr, ts):

    eventList = ["variable_1"]
    # this shoule be deleted
    clientId = "test" + "_" + str(ts)
    metainfo = "GRID[<0,0>:<1,1>]%TS["+str(ts)+"]"
    matchtype= "META_GRID"
    print("debug clientid %s metainfo %s matchtype %s"%(clientId,metainfo,matchtype))
    pubsubclient.publishEventList(pubsubAddr,eventList,clientId,metainfo,matchtype)
    print("pubsubclient %s ok"%(clientId))


#prifix = "./image"
#if os.path.isdir(prifix):
#    shutil.rmtree(prifix)


#os.mkdir(prifix)

#define grid
class Grid:
    def __init__(self, p, ux, uy, uz, lb, ub):
        self.p = p
        self.ux = ux
        self.uy = uy
        self.uz = uz
        self.lb = lb
        self.ub = ub

    def __repr__(self):
        return repr((self.p, self.ux, self.uy, self.uz, self.lb,self.ub))


r = 15

xlimit = r
ylimit = r 
zlimit = r 

gridnum=50

deltar=1.0*r/gridnum

massR = 4

lbx = -r*1.0
lby = -r*1.0
lbz = -r*1.0

ubx = r*1.0
uby = r*1.0
ubz = r*1.0

deltat=1


initux=0
inituy=0
inituz=1

constantVFiled = [initux,inituy,inituz]

initp = 1.5

#boundry p
boundp = initp*(-5)


gridList=[]



# generate array of grid and init lb and ub
for zi in range (gridnum):
    gridlbz = zi*deltar
    for yi in range (gridnum):
        gridlby = yi*deltar
        for xi in range (gridnum):
            gridlbx=xi*deltar
            gridobj = Grid(initp,constantVFiled[0],constantVFiled[1],constantVFiled[2],[gridlbx,gridlby,gridlbz],[gridlbx+deltar,gridlby+deltar,gridlbz+deltar])
            gridList.append(gridobj)

print("data init ok")

def updateGridValueFake(gridListInput,ifcenter):
    if(ifcenter==True):
        # update center other parts is init value
        massOriginInterest=[7,7,7]
    else:
        # update left, center part is red
        # update center other parts is init value
        massOriginInterest=[2,2,2]

    rmassLb = [massOriginInterest[0]-massR/2.0,massOriginInterest[1]-massR/2.0,massOriginInterest[2]-massR/2.0]
        
    for i in range (len(gridList)):
        zindex = gridList[i].lb[2]
        yindex = gridList[i].lb[1]
        xindex = gridList[i].lb[0]

        gridList[i].p = initp*(-5)
        if (xindex >= rmassLb[0] and xindex <= rmassLb[0]+massR and yindex>=rmassLb[1] and yindex<=rmassLb[1]+massR  and zindex>=rmassLb[2] and zindex<=rmassLb[2]+massR) :
            # update p value
            gridList[i].p=initp*(5)

    # simulate the time to caculate the data
    time.sleep(0.5)



if (len(sys.argv)!=3):
    print("simulation <iteration> <when interesting thing happen>")
    exit(0)

iteration = int(sys.argv[1])

changeVPeriod = int(sys.argv[2])

vsign = 1

for t in range (iteration):
    moveToCenter = False
    #if (t>=changeVPeriod and t%changeVPeriod==0):
    if (t==changeVPeriod):
        moveToCenter = True
        
    updateGridValueFake(gridList,moveToCenter)


    putDataToDataSpaces(gridList,t)
    
    if (moveToCenter):
        addrList=metaclient.getServerAddr()
        addr = addrList[0]
        metaclient.Recordtimestart(addr, "TIMET")
        
ds.finalize()
MPI.Finalize()

endsim = timeit.default_timer()

print("time span")
print (endsim-startsim)