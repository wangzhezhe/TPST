# subscribe to server according to config
# store the json info at current map
# wait the notify and when there is notify, trigure the registered program

import json 
import pubsub as pubsubclient
import os
import netifaces as ni
from threading import Thread


def threaded_function():
    
    port = 50052
    ni.ifaddresses('eno1')
    ip = ni.ifaddresses('eno1')[ni.AF_INET][0]['addr']
    serverAddr = ip + ":" + str(port)
    #print ("notify server addr %s"%(serverAddr))  
    # start a new server to do this
    pubsubclient.runNotifyServer(serverAddr)



if __name__ == '__main__':

    # start notify server
    thread = Thread(target = threaded_function)
    thread.start()
    


    with open('sublist.json', 'r') as f:
        dicData = json.load(f)

    # print ("json data")
    print (dicData['actionList'])
    
    pubsubclient.registerDic=dicData

    # get addr of the wf server and info of the server
    subEventList = dicData['subeventList']
    clientId = dicData['clientid']

    # print (data.clientid)

    # load the configuration of the registration



    # how to make the data structure in distributed way
    # store the info in local map



    # start the notify server
    # when the server is notified, run the action list
    # publish to which service???
    # get the service ip from the dir of the ip dir

    projectDir = "/project1/parashar-001/zw241/software/eventDrivenWorkflow"
    serverAddrDir = projectDir + "/src/server/multinodeip"
    serverAddrFile = projectDir + "/src/server/multinodeip/cluster0"

    for root, dirs, files in os.walk(serverAddrFile):
        print (files)

    # test case for one server

    clientid = '0'
    pubsubServerAddr = files[0]
    addr = pubsubServerAddr
    eventList = subEventList

    # subscribe all the data into the server
    pubsubclient.subscribeEventList(addr,eventList,clientid)

    # waiting server
    thread.join()   