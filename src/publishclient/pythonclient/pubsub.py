# Copyright 2015 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""The Python implementation of the GRPC helloworld.Greeter client."""

from __future__ import print_function

import grpc
import netifaces as ni

import workflowserver_pb2
import workflowserver_pb2_grpc

_ONE_DAY_IN_SECONDS = 60 * 60 * 24

class Greeter(helloworld_pb2_grpc.GreeterServicer):

    def SayHello(self, request, context):
        return helloworld_pb2.HelloReply(message='Hello, %s!' % request.name)

    def Notify(self, request, context):
        return helloworld_pb2.Notify(returnmessage='notifyOk')


# start one notify server to recieve notification
def runNotifyServer(notifyAddr):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    workflowserver_pb2_grpc.add_GreeterServicer_to_server(Greeter(), server)
    server.add_insecure_port(notifyAddr)
    server.start()
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        server.stop(0)



def sendNotify(addr, clientId):
    # NOTE(gRPC Python Team): .close() is possible on a channel and should be
    # used in circumstances in which the with statement does not fit the needs
    # of the code.
    
    with grpc.insecure_channel(addr) as channel:
        stub = workflowserver_pb2_grpc.GreeterStub(channel)
        #response = stub.SayHello(workflowserver_pb2.HelloRequest(name='you'))

        request = workflowserver_pb2.NotifyRequest()

        # refer to https://developers.google.com/protocol-buffers/docs/reference/python-generated#embedded_message
        request.clientid=clientId
        request.metadata="testMeta"

        response = stub.Notify(request)
        #send the publish event
   
    print("Publish client received: " + response.returnmessage)


def subscribeEventList(addr,eventList,clientId):
    with grpc.insecure_channel(addr) as channel:
        stub = workflowserver_pb2_grpc.GreeterStub(channel)
        #response = stub.SayHello(workflowserver_pb2.HelloRequest(name='you'))

        request = workflowserver_pb2.PubSubRequest()

        # refer to https://developers.google.com/protocol-buffers/docs/reference/python-generated#embedded_message
        request.pubsubmessage.extend(eventList)
        request.clientid=clientId
        request.source="CLIENT"
        request.metadata="testMeta"

        response = stub.Subscribe(request)
        #send the publish event
   
    print("Publish client received: " + response.returnmessage)    

def publishEventList(addr,eventList,clientId):
    # NOTE(gRPC Python Team): .close() is possible on a channel and should be
    # used in circumstances in which the with statement does not fit the needs
    # of the code.
    
    with grpc.insecure_channel(addr) as channel:
        stub = workflowserver_pb2_grpc.GreeterStub(channel)
        #response = stub.SayHello(workflowserver_pb2.HelloRequest(name='you'))

        request = workflowserver_pb2.PubSubRequest()

        # refer to https://developers.google.com/protocol-buffers/docs/reference/python-generated#embedded_message
        request.pubsubmessage.extend(eventList)
        request.clientid=clientId
        request.source="CLIENT"
        request.metadata="testMeta"

        response = stub.Publish(request)
        #send the publish event
   
    print("Publish client received: " + response.returnmessage)




if __name__ == '__main__':
    
    port = 50051

    ni.ifaddresses('eno1')
    ip = ni.ifaddresses('eno1')[ni.AF_INET][0]['addr']
    addr = ip + ":" + str(port)
    print (addr)  

    # should print "192.168.100.37"
    # get the ip
    eventList = ["eventA","eventB","eventC"]
    publishEventList(addr,eventList)