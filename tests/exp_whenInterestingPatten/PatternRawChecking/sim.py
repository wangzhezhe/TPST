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
from threading import Thread

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
#lock_name = "my_test_lock_"+str(rank)
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

gridnum=15

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


class ColorMass:
    
    massOrigin=[]

    massR = massR

    p = initp



ifFirstHappen = False

def checkAndPublishEvent(gridListNew,iteration):
    ifTargetEventHappen = True
    massOriginInterest = [6,0,6]
    targetValue = 7.5

    # put the analysis into the simulation part
    for i in range (massOriginInterest[0],massOriginInterest[0]+massR):
        for j in range (massOriginInterest[1],massOriginInterest[1]+massR):
            for k in range (massOriginInterest[2],massOriginInterest[2]+massR):
                #print "index i j k (%d %d %d)" % (i,j,k)
                #print  nparray[i][j][k]
                #print "index i j k (%d %d %d)" % (i,j,k)
                #print nparray[i][j][k]
                index = getIndex(i,j,k)
                if (gridListNew[index].p!=targetValue):
                    ifTargetEventHappen = False
                    break

    if (ifTargetEventHappen == True):
        print (iteration)
        # send publish event
        event = "CUBIC_DETECTED"
        meta = str(iteration)
        detecttime = timeit.default_timer()
        print (detecttime)
        pubsubclient.initAddrAndPublish(event,meta)
        ifFirstHappen = True
    return

# detect if the point is in mass
def inMassBody(px,py,pz,massOrigin,t,currIndex):
    
    upbdx = massOrigin[0]+deltar*massR
    upbdy = massOrigin[1]+deltar*massR
    upbdz = massOrigin[2]+deltar*massR

    #if (t>=5 and t <= 7 and currIndex%100 ==11):
    #    print "inMassBody"
    #    print [px,py,pz]
    #    print massOrigin
    #    print [upbdx,upbdy,upbdz]

    if (px >= massOrigin[0] and px <= upbdx and py >=massOrigin[1] and py <= upbdy and pz >=massOrigin[2] and pz <= upbdz):
        return True
    else:
        return False

def ifOutBoundForNextStep(massOrigin,t,currIndex):
    # caculate the position for next step
    nextx = massOrigin[0]+deltat*constantVFiled[0]
    nexty = massOrigin[1]+deltat*constantVFiled[1]
    nextz = massOrigin[2]+deltat*constantVFiled[2]

    bxlpx=0
    bxlby=0
    bxlbz=0

    # check eight point

    bxupx=r
    bxupy=r
    bxupz=r


    # adjust if eight point is at outside of the boundry
    if nextx<bxlpx or nextx>bxupx or nexty<bxlby or nexty>bxupy or nextz<bxlbz or nextz>bxupz:
        return True

    
    
    checkx=nextx+massR*deltar
    checky=nexty
    checkz=nextz

    if checkx<bxlpx or checkx>bxupx or checky<bxlby or checky>bxupy or checkz<bxlbz or checkz>bxupz:
        return True


    checkx=nextx
    checky=nexty+massR*deltar
    checkz=nextz

    if checkx<bxlpx or checkx>bxupx or checky<bxlby or checky>bxupy or checkz<bxlbz or checkz>bxupz:
        return True


    checkx=nextx
    checky=nexty
    checkz=nextz+massR*deltar

    if checkx<bxlpx or checkx>bxupx or checky<bxlby or checky>bxupy or checkz<bxlbz or checkz>bxupz:
        return True


    checkx=nextx
    checky=nexty+massR*deltar
    checkz=nextz+massR*deltar

    if checkx<bxlpx or checkx>bxupx or checky<bxlby or checky>bxupy or checkz<bxlbz or checkz>bxupz:
        return True


    checkx=nextx+massR*deltar
    checky=nexty
    checkz=nextz+massR*deltar

    if checkx<bxlpx or checkx>bxupx or checky<bxlby or checky>bxupy or checkz<bxlbz or checkz>bxupz:
        return True


    checkx=nextx+massR*deltar
    checky=nexty+massR*deltar
    checkz=nextz

    if checkx<bxlpx or checkx>bxupx or checky<bxlby or checky>bxupy or checkz<bxlbz or checkz>bxupz:
        return True


    checkx=nextx+massR*deltar
    checky=nexty+massR*deltar
    checkz=nextz+massR*deltar

    if checkx<bxlpx or checkx>bxupx or checky<bxlby or checky>bxupy or checkz<bxlbz or checkz>bxupz:
        return True

        
    return False

def boundryDetect(gridListObj):
    # detect collision 
    # update the velocity
    
    # collision x direaction 1
    # collision y direaction 2
    # collision z direaction 3
    collisionFace = []


    pcpx = gridListObj.lb[0] + deltar/2
    pcpy = gridListObj.lb[1] + deltar/2
    pcpz = gridListObj.lb[2] + deltar/2

    cux =gridListObj.ux
    cuy =gridListObj.uy
    cuz =gridListObj.uz

    newx = pcpx+cux*deltat
    newy = pcpy+cuy*deltat
    newz = pcpz+cuz*deltat
    

    if (newx > gridnum*deltar) or (newx < 0):
        collisionFace.append(1)
    if (newy > gridnum*deltar) or (newy < 0):
        collisionFace.append(2)
    if (newz > gridnum*deltar) or (newz < 0):
        collisionFace.append(3)

    return collisionFace

# the effect of the collision have been added
def updateGridValue(gridListOld,gridListNew,currIndex,t,redmassOrigin,bluemassOrigin):
    
    
    rmassLb = [redmassOrigin[0]-massR/2.0,redmassOrigin[1]-massR/2.0,redmassOrigin[2]-massR/2.0]
    bmassLb = [bluemassOrigin[0]-massR/2.0,bluemassOrigin[1]-massR/2.0,bluemassOrigin[2]-massR/2.0]

    zgridindex = int(gridListNew[currIndex].lb[2])
    ygridindex = int(gridListNew[currIndex].lb[1])
    xgridindex = int(gridListNew[currIndex].lb[0])


    # get curr u (velocity)
    #cux = gridListNew[currIndex].ux
    #cuy = gridListNew[currIndex].uy
    #cuz = gridListNew[currIndex].uz

    cux = constantVFiled[0]
    cuy = constantVFiled[1]
    cuz = constantVFiled[2]

    # get curr p

    pcpx = gridListNew[currIndex].lb[0] + deltar/2
    pcpy = gridListNew[currIndex].lb[1] + deltar/2
    pcpz = gridListNew[currIndex].lb[2] + deltar/2
    
    # get old value position

    oldx = pcpx - deltat*cux
    oldy = pcpy - deltat*cuy
    oldz = pcpz - deltat*cuz



    oldpIndex = getIndex(oldx,oldy,oldz)

    newpIndex = getIndex(pcpx,pcpy,pcpz)
    

    # in flow for one time step 
    # oldx oldy oldz at the outside p = 0
    # clamping case, the p value shoule be 0
    if oldpIndex == -1:
        gridListNew[newpIndex].p = initp
        return


    #if body is outof box
    #when the point is in body
    #do not update



    redifout = ifOutBoundForNextStep(rmassLb,t,currIndex)

    blueifout = ifOutBoundForNextStep(bmassLb,t,currIndex)

    #if (t>=4 and t <= 6 and currIndex%100 ==11):
    #    print t 
    #    print massLb
    #    print ifout
     
    if (redifout == True or blueifout == True) and gridListNew[currIndex].p != initp:
        rifinBody = inMassBody(pcpx,pcpy,pcpz,rmassLb,t,currIndex)
        bifinBody = inMassBody(pcpx,pcpy,pcpz,bmassLb,t,currIndex)
        #if (t>=4 and t <= 7 and currIndex%100 ==11):
        #    print "ifinBody currIndex"
        #    print [ifinBody,currIndex]
        if rifinBody==True or bifinBody==True:
            #do not move if the condition is true
            return
        else:
            gridListNew[newpIndex].p = gridListOld[oldpIndex].p

    else:
        gridListNew[newpIndex].p = gridListOld[oldpIndex].p


        
    """
    # if on boundry
    for dis in range (massR):
        if (zgridindex==(gridnum-1-dis)):
            # not move
            # if current if updated value 
            # do not update
            # if next 2 level is boundry do not update
            disIndex = currIndex+dis*gridnum*gridnum
            # TODO adjust if collision
            if gridListNew[disIndex].p != initp:
                print "t %d curr index%d" % (t,currIndex)
                print [xgridindex,ygridindex,zgridindex]
                return
    """
 

    #if (t>=5 and t <= 9 and currIndex==912):
    #    print "after updating"
    #    print [gridListNew[newpIndex].p,gridListOld[oldpIndex].p]

def generateImage(gridList,filename):

    cellData=[]
    for i in range (len(gridList)):
        #print gridList[i].p
        cellData.append(gridList[i].p)

    pressure=numpy.asarray(cellData).reshape(gridnum, gridnum, gridnum,order='F')
    
    #print pressure

    imageToVTK(filename, cellData = {"pressure" : pressure})
    
# input the coordinate of the points and return the index of grid in array
def getIndex(px,py,pz):
    # TODO should add all boundry case
    # only for lower case
    if (px<0 or py <0 or pz<0 or px > gridnum*deltar or py > gridnum*deltar or pz > gridnum*deltar):
        #print "out of the box "
        #print [px,py,pz]
        return -1
    
    gnumx=math.floor((px-0)/deltar)
    gnumy=math.floor((py-0)/deltar)
    gnumz=math.floor((pz-0)/deltar)

    index = int(gnumz*gridnum*gridnum + gnumy*gridnum+gnumx)

    return index

gridList=[]

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')


# init color mass
redmass = ColorMass()
redmass.massOrigin=[1+redmass.massR/2,1+redmass.massR/2,1+redmass.massR/2]
redmass.p = initp*(5)


bluemass = ColorMass()
bluemass.massOrigin = [8+bluemass.massR/2,1+bluemass.massR/2,1+bluemass.massR/2]
bluemass.p = initp*(-5)


# generate array of grid and init lb and ub
for zi in range (gridnum):
    gridlbz = zi*deltar
    for yi in range (gridnum):
        gridlby = yi*deltar
        for xi in range (gridnum):
            gridlbx=xi*deltar
            gridobj = Grid(initp,constantVFiled[0],constantVFiled[1],constantVFiled[2],[gridlbx,gridlby,gridlbz],[gridlbx+deltar,gridlby+deltar,gridlbz+deltar])
            gridList.append(gridobj)
            #plt.plot(gridlbx,gridlby,gridlbz,'k.')
            ax.scatter(gridlbx, gridlby, gridlbz, color='k',marker='.')


# show grid data
plt.axis('scaled')
 
#plt.show()
 
# init the value at the grid center
for i in range (len(gridList)):

    # add boundry condition
    #zindex = i / (gridnum*gridnum)
    #yindex = (i % (gridnum*gridnum))/gridnum
    #xindex = (i % (gridnum*gridnum) % gridnum)

    zindex = gridList[i].lb[2]
    yindex = gridList[i].lb[1]
    xindex = gridList[i].lb[0]


    if xindex==0 or xindex ==(gridnum-1) or yindex==0 or yindex==(gridnum-1) or zindex==0 or zindex==(gridnum-1):
        # boundry condition for p value and v value
        #gridList[i].p=boundp
        gridList[i].p=initp
        #if (xindex==0 or xindex ==(gridnum-1)):
            #gridList[i].ux = 0
        #    print "x boundry"
        #if (yindex==0 or yindex==(gridnum-1)):
            #gridList[i].uy = 0
        #    print "y boundry"
        #if (zindex==0 or zindex==(gridnum-1)):
            #gridList[i].uz = inituz / 2.0
            #gridList[i].ux = 1
        #    print "z boundry"
            

    # init condition
    # generate init color mass
    rmassLb = [redmass.massOrigin[0]-massR/2.0,redmass.massOrigin[1]-massR/2.0,redmass.massOrigin[2]-massR/2.0]
    rmassUb = [redmass.massOrigin[0]+massR/2.0,redmass.massOrigin[1]+massR/2.0,redmass.massOrigin[2]+massR/2.0]

    if (xindex >= rmassLb[0] and xindex <= rmassUb[0]-deltar and yindex>=rmassLb[1] and yindex<=rmassUb[1]-deltar  and zindex>=rmassLb[2] and zindex<=rmassUb[2]-deltar) :
        # update p value
        gridList[i].p=redmass.p
        # update velocity
        gridList[i].ux = constantVFiled[0]
        gridList[i].uy = constantVFiled[1]
        gridList[i].uz = constantVFiled[2]


    bmassLb = [bluemass.massOrigin[0]-massR/2.0,bluemass.massOrigin[1]-massR/2.0,bluemass.massOrigin[2]-massR/2.0]
    bmassUb = [bluemass.massOrigin[0]+massR/2.0,bluemass.massOrigin[1]+massR/2.0,bluemass.massOrigin[2]+massR/2.0]
    
    '''
    if (xindex >= bmassLb[0] and xindex <= bmassUb[0]-deltar and yindex>=bmassLb[1] and yindex<=bmassUb[1]-deltar  and zindex>=bmassLb[2] and zindex<=bmassUb[2]-deltar) :
        # update p value
        gridList[i].p=bluemass.p
        # update velocity
        gridList[i].ux = constantVFiled[0]
        gridList[i].uy = constantVFiled[1]
        gridList[i].uz = constantVFiled[2]
    '''

    #print "debug"
    #print [xindex,yindex,zindex]

    #if (i >=6 and i < 8) :
    #    gridList[i].p=initp*(-2)

# update the p in grid array

# ten timesteps

gridListNew = gridList

# trace the position of the mass origin
massOriginNew = redmass.massOrigin

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
            gridList[i].p=redmass.p

    # simulate the time to caculate the data
    time.sleep(0.1)



if (len(sys.argv)!=3):
    print("simulation <iteration> <when interesting thing happen>")
    exit(0)

iteration = int(sys.argv[1])

changeVPeriod = int(sys.argv[2])

vsign = 1

startsim = timeit.default_timer()


# send record to clock service

addrList=metaclient.getServerAddr()
addr = addrList[0]
metaclient.Recordtime(addr, "SIM")

for t in range (iteration):
    moveToCenter = False
    if (t>=changeVPeriod and t%changeVPeriod==0):
        moveToCenter = True
        
    updateGridValueFake(gridList,moveToCenter)
    
    
    putDataToDataSpaces(gridListNew,t)
    
        
ds.finalize()
MPI.Finalize()

endsim = timeit.default_timer()

print("time span")
print (endsim-startsim)
