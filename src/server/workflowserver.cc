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
#include "../publishclient/pubsubclient.h"
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
using workflowserver::PubSubReply;
using workflowserver::PubSubRequest;
using workflowserver::SubNumReply;
using workflowserver::SubNumRequest;

#define NOTIFYPORT 50052

using namespace std;

int waitTime;
int nodeNumber;



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

  Status GetSubscribedNumber(ServerContext *context, const SubNumRequest *request, SubNumReply *reply)
  {

    //get the request event
    string requestEvent = request->subevent();
    //printf("search the clients number associated with %s\n", requestEvent.data());
    //search how many clients associated with this event
    int clientsNumber = getSubscribedClientsNumber(requestEvent);

    //return this number
    reply->set_clientnumber(clientsNumber);

    return Status::OK;
  }

  Status Subscribe(ServerContext *context, const PubSubRequest *request, PubSubReply *reply)
  {

    struct timespec start, end;
    double diff;

    clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

    string peerURL = context->peer();

    //replace the port here the server port for notify is 50052
    string clientIP = parseIP(peerURL);

    string notifyAddr = clientIP + ":" + to_string(NOTIFYPORT);
    string clientId = request->clientid();
    if (clientId == "")
    {
      printf("client id is not supposed to be \"\"\n");
      return Status::OK;
    }

    //clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
    //diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    //printf("debug for subevent stage1 response time = (%llu) second\n", (long long unsigned int)diff);

    //every elemnt could be accessed by specific function
    //reply->set_returnmessage(prefix + request->pubsubmessage());

    //parse the request events
    int size = request->pubsubmessage_size();
    printf("server get (%d) subscribed events\n", size);
    int i = 0;
    vector<string> eventList;
    string eventStr;

    //test respond time
    string debugevents;

    for (i = 0; i < size; i++)
    {
      eventStr = request->pubsubmessage(i);
      //for debug
      debugevents = eventStr;
      //printf("get events (%s)\n", eventStr.data());
      eventList.push_back(eventStr);
      //default number is 1
      int trinum = 1;
      string eventMessage;
      ParseEvent(eventStr, eventMessage, trinum);
      //printf("after parsing %s %d\n", eventMessage.data(), trinum);
      addNewEvent(eventMessage, trinum);
    }

    //clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
    //diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    //printf("debug for subevent stage2 response time = (%llu) second\n", (long long unsigned int)diff);

    addNewClient(clientId, notifyAddr, eventList);

    pubsubSubscribe(eventList, clientId);

    //TODO control thread number here
    //start new thread to execute the checking logic

    pthread_t tid;

    printf("create thread for %s\n", clientId.data());
    pthread_create(&tid, NULL, &checkNotify, (void *)clientId.data());

    reply->set_returnmessage("SUBSCRIBED");

    clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
    diff = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    printf("debug for subevent stage4 (%s) response time = (%lf) second\n", debugevents.data(), diff);

    return Status::OK;
  }

  Status Publish(ServerContext *context, const PubSubRequest *request, PubSubReply *reply)
  {

    //test respond time
    string debugeventspub;
    double diff;
    struct timespec start, end;

    clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

    //parse the request events
    int size = request->pubsubmessage_size();
    //printf("server get (%d) published events\n", size);
    int i = 0;
    vector<string> eventList;
    string eventStr;

    for (i = 0; i < size; i++)
    {
      eventStr = request->pubsubmessage(i);
      debugeventspub = eventStr;
      printf("server publish event (%s)\n", eventStr.data());
      eventList.push_back(eventStr);
    }
    //publish
    pubsubPublish(eventList);
    reply->set_returnmessage("OK");

    clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
    diff = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    printf("debug for publish (%s) response time = (%lf) second\n", debugeventspub.data(), diff);
    return Status::OK;
  }
};

void MultiClient()
{
  vector<string> multiAddr;
  int size;
  while (1)
  {
    multiAddr = loadMultiNodeIPPort();
    size = multiAddr.size();
    if (size == nodeNumber)
    {
      break;
    }

    usleep(50000);
    printf("there are %d clients record their ip in the multinode dir\n",size);
  }

  initMultiClients();
}

void RunServer()
{

  string serverPort = string("50051");
  string ip;
  printf("record ip\n");
  recordIPPort(ip, serverPort);
  //get the server ip from the config file
  /*
  string ipconfigfilepath = string("./ipconfig");

  //load the file
  string ip;
  string port;
  int r = loadIPPort(ipconfigfilepath, ip, port);
  if (r == 1)
  {
    printf("failed to open the ip port config file (%s)\n", ipconfigfilepath.data());
  }
  */

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

int main(int argc, char **argv)
{

  //get wait time ./workflowserver 1000
  printf("parameter length %d\n", argc);
  if (argc == 3)
  {
    waitTime = atoi(argv[1]);
    printf("chechNotify wait period %d\n", waitTime);
    nodeNumber = atoi(argv[2]);
    printf("instance number of the backend is %d\n", nodeNumber);
  }
  else
  {
    printf("./workflowserver <subscribe period time> <number of the nodes>\n");
    return 0;
  }

  RunServer();
  return 0;
}
