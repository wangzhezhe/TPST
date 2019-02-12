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
#include <thread>
#include "../utils/groupManager/groupManager.h"
#include "../observer/eventmanager.h"
#include "../runtime/local.h"
#include "../../src/publishclient/pubsubclient.h"

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

//default value
//string NOTIFYPORT("50052");

string NOTIFYADDR;

void startAction(string clientID, string metadata)
{

    //get map<string, EventTriggure *> clientIdtoConfig;

    printf("clientid %s\n", clientID.data());

    if (clientIdtoConfig.find(clientID) == clientIdtoConfig.end())
    {
        printf("failed to get eventtriggure from clientIdtoConfig\n");
        return;
    }
    EventTriggure *etrigger = clientIdtoConfig[clientID];

    int actionSize = etrigger->actionList.size();
    int i;

    printf("debug current driver %s\n",etrigger->driver.data());

    if (etrigger->driver.compare("local") == 0)
    {
        for (i = 0; i < actionSize; i++)
        {

            localTaskStart(etrigger->actionList[i].data(), metadata);

            //get the publishEvent from configure and call the publish operation
        }
    }

    if (etrigger->driver.compare("python") == 0)
    {
        for (i = 0; i < actionSize; i++)
        {

            //thread py_thread(pythonTaskStart, etrigger->actionList[i].data(), metadata);
            //py_thread.detach();

            //get the publishEvent from configure and call the publish operation
        }
    }

    /*
    //when all action finish, push the events into files (if following events needed to be published after the application running)

    int pubSize = etrigger->eventPubList.size();

    //get a client
    GreeterClient *greeter = roundrobinGetClient();

    if (greeter == NULL)
    {
        printf("failed to get greeter for event publish\n");
        return;
    }

    string metadata("testmetadata");
    //use updated source
    greeter->Publish(etrigger->eventPubList, sourceClient, metadata);
    */

    return;
}

// Logic and data behind the server's behavior.
class GreeterServiceImplNotify final : public Greeter::Service
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
        string metadata = request->metadata();
        string peerURL = context->peer();

        //meta data is the version here
        //printf("get client id %s get metadata %s\n", clientID.data(), metadata.data());

        //TODO get the json from the configID and use runtime to star this
        //it's better to put the mapping relation here
        //printf("call runtime to start tht actions\n");

        std::string message("OK");
        reply->set_returnmessage(message);

        //don't do this for pub/sub performance testing

        startAction(clientID, metadata);

        NotifiedNumMtx.lock();
        NotifiedNum++;
        NotifiedNumMtx.unlock();

        struct timespec finish;

        //if (NotifiedNum % 128 == 0)
        //{
        clock_gettime(CLOCK_REALTIME, &finish); /* mark the end time */
        printf("id %d notifynum %d finish time = (%lld.%.9ld)\n",
               gm_rank, NotifiedNum, (long long)finish.tv_sec, finish.tv_nsec);
        //}
        return Status::OK;
    }
};

/*
void RunServer(string serverIP, string serverPort)
{

  string socketAddr = serverIP + ":" + serverPort;
  printf("server socket addr %s\n", socketAddr.data());
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
*/

string getNotifyServerAddr()
{
    int freePort = getFreePortNum();
    string serverPort = to_string(freePort);
    string ip;
    //printf("record ip\n");

    recordIPPortWithoutFile(ip, serverPort);
    //TODO send ip:port to workflowserver

    string socketAddr = ip + ":" + serverPort;
    return socketAddr;
}

void runNotifyServer()
{
    //TODO +1 is the port is occupied
    //int port=50052;
    //os will assign a free port

    //string serverPort = NOTIFYPORT;

    printf("notify server addr %s\n", NOTIFYADDR.data());
    std::string server_address(NOTIFYADDR);
    GreeterServiceImplNotify service;

    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    //std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

//this server aims to recieve the notify request from the workflow server
void *RunNotifyServer(void *arg)
{

    //pthread_t id;
    //pthread_create(&id, NULL, RunNotifyServer, NULL);
    //printf("start new thread %ld for notifyserver\n",id);
    runNotifyServer();
    return NULL;
}
