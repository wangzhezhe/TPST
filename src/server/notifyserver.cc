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

#include <grpc++/grpc++.h>
#include <uuid/uuid.h>

#include "pubsub.h"
#include "unistd.h"
#include <mutex>
#include "../utils/getip/getip.h"
#include "../observer/eventmanager.h"
#include "../runtime/local.h"

#include <stdint.h> /* for uint64 definition */
#include <stdlib.h> /* for exit() definition */
#include <time.h>   /* for clock_gettime */
#define BILLION 1000000000L

#ifdef BAZEL_BUILD
#else
#include "workflowserver.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using workflowserver::Greeter;
using workflowserver::HelloReply;
using workflowserver::HelloRequest;
using workflowserver::NotifyReply;
using workflowserver::NotifyRequest;

using namespace std;

mutex NotifiedNumMtx;
int NotifiedNum = 0;

void startAction(string clientID)
{

    //get map<string, EventTriggure *> clientIdtoConfig;

    if (clientIdtoConfig.find(clientID) == clientIdtoConfig.end())
    {
        printf("failed to get eventtriggure from clientIdtoConfig\n");
        return;
    }
    EventTriggure *etrigger = clientIdtoConfig[clientID];

    int actionSize = etrigger->actionList.size();
    int i;
    for (i = 0; i < actionSize; i++)
    {
        if (etrigger->driver.compare("local") == 0)
        {
            localTaskStart(etrigger->actionList[i].data());
        }
    }
    return;
}

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service
{
    //for test using
    Status SayHello(ServerContext *context, const HelloRequest *request, HelloReply *reply) override
    {
        std::string prefix("Hello:");
        reply->set_message(prefix + request->name());
        return Status::OK;
    }

    Status Notify(ServerContext *context, const NotifyRequest *request, NotifyReply *reply)
    {

        //get clientID

        string clientID = request->clientid();

        //printf("get client id %s\n", clientID.data());

        //TODO get the json from the configID and use runtime to star this
        //it's better to put the mapping relation here
        //printf("call runtime to start tht actions\n");

        std::string message("OK");
        reply->set_returnmessage(message);
        startAction(clientID);

        NotifiedNumMtx.lock();
        NotifiedNum++;
        NotifiedNumMtx.unlock();

        //printf("number which is notified %d\n", NotifiedNum);
        return Status::OK;
    }
};

void runNotifyServer()
{

    string serverPort = string("50052");
    string ip;
    printf("record ip\n");

    recordIPPortWithoutFile(ip, serverPort);
    //TODO send ip:port to workflowserver

    string socketAddr = ip + ":" + serverPort;

    std::string server_address(socketAddr);
    GreeterServiceImpl service;

    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

void *RunNotifyServer(void *arg)
{

    //pthread_t id;
    //pthread_create(&id, NULL, RunNotifyServer, NULL);
    //printf("start new thread %ld for notifyserver\n",id);
    runNotifyServer();
    return NULL;
}
