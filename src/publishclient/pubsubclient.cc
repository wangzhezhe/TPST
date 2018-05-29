/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <unistd.h>
#include <pthread.h>

#include <grpc++/grpc++.h>

#include "workflowserver.grpc.pb.h"
#include "pubsubclient.h"
#include "../utils/getip/getip.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using workflowserver::Greeter;
using workflowserver::HelloReply;
using workflowserver::HelloRequest;
using workflowserver::PubSubReply;
using workflowserver::PubSubRequest;

using namespace std;

// this should be loaded from the ./ipconfig

//string socketaddr=string("10.211.55.5:50051");
//GreeterClient greeter = (grpc::CreateChannel(
//        socketaddr.data(), grpc::InsecureChannelCredentials()));

GreeterClient *GreeterClient::getClient()
{

    string ip;
    string port;
    //string ipconfigfilepath = string("/home/parallels/Documents/cworkspace/observerchain/src/server/ipconfig");
    //workflow server and eventNotify should run at same directory, ipconfig will be outputed into this dir
    string ipconfigfilepath = string("./ipconfig");
    int r = loadIPPort(ipconfigfilepath, ip, port);
    if (r == 1)
    {
        printf("failed to open the ip port config file (%s)\n", ipconfigfilepath.data());
        return NULL;
    }
    string socketaddr = ip + ":" + port;
    printf("server socket addr %s\n", socketaddr.data());
    //singleton mode
    static GreeterClient *singleClient = new GreeterClient(grpc::CreateChannel(
        socketaddr.data(), grpc::InsecureChannelCredentials()));
    return singleClient;
}

// Assembles the client's payload, sends it and presents the response back
// from the server.
string GreeterClient::SayHello(const string &user)
{
    // Data we are sending to the server.
    HelloRequest request;
    request.set_name(user);

    // Container for the data we expect from the server.
    HelloReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->SayHello(&context, request, &reply);

    // Act upon its status.
    if (status.ok())
    {
        return reply.message();
    }
    else
    {
        cout << status.error_code() << ": " << status.error_message()
             << endl;
        return "RPC failed";
    }
}

string GreeterClient::Subscribe(vector<string> eventList)
{

    // Container for the data we expect from the server.
    PubSubRequest request;
    PubSubReply reply;
    int size = eventList.size();
    int i = 0;
    for (i = 0; i < size; i++)
    {
        //attention the use here, the request could be transfered into a specific type with specific function
        request.add_pubsubmessage(eventList[i]);
        //printf("add %s into request \n",eventList[i].data());
    }
    
    
    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->Subscribe(&context, request, &reply);

    // Act upon its status.
    if (status.ok())
    {
        return reply.returnmessage();
    }
    else
    {
        cout << status.error_code() << ": " << status.error_message()
             << endl;
        return "RPC failed";
    }
}

string GreeterClient::Publish(vector<string> eventList)
{
    // Container for the data we expect from the server.
    PubSubRequest request;
    PubSubReply reply;
    int size = eventList.size();
    int i = 0;
    for (i = 0; i < size; i++)
    {
        //attention the use here, the request could be transfered into a specific type with specific function
        request.add_pubsubmessage(eventList[i]);
    }

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->Publish(&context, request, &reply);

    // Act upon its status.
    if (status.ok())
    {
        return reply.returnmessage();
    }
    else
    {
        cout << status.error_code() << ": " << status.error_message()
             << endl;
        return "RPC failed";
    }
}
