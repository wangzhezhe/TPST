# check data from ts 0
# when data is ok 
# send tick to the metaserver

# check the all timestep data after sim finish

from mpi4py import MPI
import numpy as np
import dataspaces.dataspaceClient as dataspaces
import ctypes
import os
import time
import math
import timeit
import sys
import threading

sys.path.append('../../../src/publishclient/pythonclient')
import pubsub as pubsubclient

sys.path.append('../../../src/metadatamanagement/pythonclient')
import metaclient

startanay = timeit.default_timer()

r = 15
gridnum = 50
deltar = 1.0*r/gridnum

lb=[0]
ub=[gridnum*gridnum*gridnum*(1)-1]

# input the coordinate of the points and return the index of grid in array
comm = MPI.COMM_WORLD
size = comm.Get_size()
rank = comm.Get_rank()

def sendEventToPubSub(ts):

    addrList = pubsubclient.getServerAddr()
    print (addrList)

    addr = addrList[0]

    eventList = ["dataPattern_1"]
    # this shoule be deleted
    clientId = "test" + "_" + str(ts)
    metainfo = "GRID[<-1,-1>:<-1,-1>]%TS["+str(ts)+"]"
    matchtype = "NAME"
    pubsubclient.publishEventList(addr,eventList,clientId,metainfo,matchtype)

def getIndex(px, py, pz):
    # TODO should add all boundry case
    # only for lower case


    if (px < 0 or py < 0 or pz < 0 or px > gridnum*deltar or py > gridnum*deltar or pz > gridnum*deltar):
        #print "out of the box "
        #print [px,py,pz]
        return -1

    gnumx = math.floor((px-0)/deltar)
    gnumy = math.floor((py-0)/deltar)
    gnumz = math.floor((pz-0)/deltar)

    index = int(gnumz*gridnum*gridnum + gnumy*gridnum+gnumx)

    return index

def checkAndPublishEvent(gridDataArray_p1, gridDataArray_p2):
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
                if (gridDataArray_p1[index] != targetValue):
                    ifTargetEventHappen = False
                    break

    if (ifTargetEventHappen == True):
        print (iteration)
        # send publish event
        detecttime = timeit.default_timer()
        print (detecttime)
        print ("publish to pub/sub broker")
        #sendEventToPubSub(iteration)
        ifFirstHappen = True
    return

initp =  1.5
targetValue = 7.5

def checkDataPattern(gridDataArray_p1, gridDataArray_p2):

    coord1 = []
    coord2 = []
    # get the index of red block in data 1
    # print("caculate coord1")
    break_flag=False
    for x in range(15):
        if(break_flag==True):
            break
        for y in range (15):
            if(break_flag==True):
                break
            for z in range (15):
                index = getIndex(x,y,z)
                if (gridDataArray_p1[index]==targetValue):
                    coord1 = [x,y,z]
                    break_flag=True
                    #print(coord1)
                    break


    # get the index of the red block in data 2
    #print("caculate coord2")
    break_flag=False
    for x in range(15):
        if(break_flag==True):
            break
        for y in range (15):
            if(break_flag==True):
                break
            for z in range (15):
                index = getIndex(x,y,z)
                if (gridDataArray_p2[index]==targetValue):
                    coord2 = [x,y,z]
                    break_flag=True
                    #print(coord2)
                    break
    
    distance = pow((coord2[0]-coord1[0]),2)+pow((coord2[1]-coord1[1]),2)+pow((coord2[2]-coord1[2]),2)
    #print(distance)
    if(distance>140 and distance<150):
        return True
    else:
        return False

def checkDataPatternCenter(gridDataArray_p1):
    massOriginInterest = [7, 7, 7]
    targetValue = 7.5

    index = getIndex(massOriginInterest[0], massOriginInterest[1], massOriginInterest[2])
    if (gridDataArray_p1[index] == targetValue):
        return True
    else:
        return False



# copy all conf.* file to current dir
serverdir = "/home1/zw241/dataspaces/tests/C"

confpath = serverdir+"/conf*"

copyCommand = "cp "+confpath+" ."

os.system(copyCommand)

# number of clients at clients end to join server
num_peers = rank
appid = 2

var_name = "ex1_sample_data"
lock_name = "my_test_lock"

if(len(sys.argv)!=2):
    print("./analytics <iteration>")
    exit(0)
    
iteration = int(sys.argv[1])



ds = dataspaces.dataspaceClient(appid,comm)

currIter = 0

#while (True):
version = 0
#for version in range(iteration):

runTask=0

def actualWork():
    global runTask
    time.sleep(1)
    runTask=runTask-1

while (version<iteration):

    #use read write lock here
    #ds.lock_on_read(lock_name)
    # use lock type  = 1
    getdata_p1,rcode = ds.get(var_name, version, lb, ub)
    #ds.unlock_on_read(lock_name)
    # check if data ok

    if(rcode == -11):
        print("data not avaliable for ts %d"%(version))
        time.sleep(0.25)
        continue

    patternHappen = checkDataPatternCenter(getdata_p1)
    #the time used for predicates every time
    time.sleep(0.01)

    version=version+1
   
    if(patternHappen==True):
        print("---------patternHappen at ts %d for rank %d----------"%(version,rank))
        # simulate the vis time
        # execute the following part for the task
        # the time used for predicates checking
        #addrList=metaclient.getServerAddr()
        #addr = addrList[0]
        #metaclient.Recordtimetick(addr, "TIMET")
        #break
        #execute actual work
        thread = threading.Thread(target = actualWork) 
        thread.start()
        runTask=runTask+1


while(1):
    if(runTask==0):
        break
    else:
        continue

ds.finalize()
endanay = timeit.default_timer()

addrList=metaclient.getServerAddr()
addr = addrList[0]
metaclient.Recordtimetick(addr, "TIMET")
 

print("time span")
print(endanay-startanay)