from mpi4py import MPI
import numpy as np
import dspaceswrapper.dataspaces as dataspaces
import ctypes
import os
import time
import math
import timeit
import sys

sys.path.append('/project1/parashar-001/zw241/software/eventDrivenWorkflow/src/publishclient/pythonclient')
import pubsub as pubsubclient

# input the coordinate of the points and return the index of grid in array
comm = MPI.COMM_WORLD
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




def checkIteration(vs):

    # copy all conf.* file to current dir
    serverdir = "/home1/zw241/dataspaces/tests/C"

    confpath = serverdir+"/conf*"

    copyCommand = "cp "+confpath+" ."

    os.system(copyCommand)

    # number of clients at clients end to join server

    var_name = "ex1_sample_data"
    lock_name = "my_test_lock"

    #if(len(sys.argv)!=2):
    #    print("./analytics <version>")
    #    exit(0)
    
    #vs = int(sys.argv[1])

    appid = vs
    ds = dataspaces.dataspaceClient(appid,comm)


    vsLb = vs-9

    startanay = timeit.default_timer()

    #ds.dspaces_init(comm, num_peers, appid)

    for version in range (vsLb, vs+1):
        lb = [0]
        ub = [3374]

        #print("get version")
        #print(version)
        #ds.lock_on_read(lock_name)
        getdata_p1 = ds.get(var_name, version, lb, ub)
        #ds.unlock_on_read(lock_name)

        patternHeppen = checkDataPatternCenter(getdata_p1)

        if(patternHeppen==True):
            print("------------------")
            print("patternHeppen at ts %d"%(version))
            print("------------------")
            print("send data patten and ts to operator")

            sendEventToPubSub(vs)
            break

    ds.dspaces_wrapper_finalize()

    endanay = timeit.default_timer()

    print("time span")
    print(endanay-startanay)


if(len(sys.argv)!=2):
    print("./analytics <version>")
    exit(0)
vs = int(sys.argv[1])

checkIteration(vs)