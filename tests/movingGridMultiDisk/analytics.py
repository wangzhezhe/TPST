from mpi4py import MPI
import numpy as np

import ctypes
import os
import time
import math
import timeit
import sys

import vtk
from vtk.util import numpy_support as VN

sys.path.append('../../src/publishclient/pythonclient')
import pubsub as pubsubclient

#comm = MPI.COMM_WORLD
#rank = comm.Get_rank()

# input the coordinate of the points and return the index of grid in array
initp =  1.5
targetValue = 7.5

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

    return pz*15*15+py*15+px


def checkAndPublishEvent(gridDataArray_p1, gridDataArray_p2):

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



def getDataFromDisk(ts):

    # range the iteration
    # get data1
    reader = vtk.vtkXMLImageDataReader()
    file_0 = "image_rank0"+"/"+"image"+str(ts)+ ".vti"
    reader.SetFileName(file_0)
    reader.Update()

    pressuredata0 = reader.GetOutput().GetCellData().GetArray('pressure')
    data0 = VN.vtk_to_numpy(pressuredata0)

    file_1 = "image_rank1"+"/"+"image"+str(ts) + ".vti"
    #print(file_1)
    reader.SetFileName(file_1)
    reader.Update()
    pressuredata1 = reader.GetOutput().GetCellData().GetArray('pressure')

    data1 = VN.vtk_to_numpy(pressuredata1)

    return data0, data1



if (len(sys.argv)!=2):
    print("analytics <iteration>")
    exit(0)

iteration = int(sys.argv[1])

startanay = timeit.default_timer()

for ts in range (iteration):

    print("current ts %d"%(ts))

    data1, data2 = getDataFromDisk(ts)

    # check data1 data2

    #print("get data1")
    #print (data1)

    #print("get data2")
    #print (data2)

    patternHeppen = checkAndPublishEvent(data1,data2)

    if(patternHeppen==True):
        print("patternHeppen at ts %d"%(ts))
        break

endanay = timeit.default_timer()

print("time span")
print(endanay-startanay)