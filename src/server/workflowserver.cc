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
using workflowserver::SubNumRequest;
using workflowserver::SubNumReply;


using namespace std;

int waitTime;

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
    uint64_t diff;
    struct timespec start, end;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start); /* mark start time */
    //create the uuid

    uuid_t uuid;
    char str[50];

    uuid_generate(uuid);
    uuid_unparse(uuid, str);

    string clientId(str);

    pubsubWrapper *psw = new (pubsubWrapper);
    psw->iftrigure = false;
    //put clientId into global map TODO add lock
    clientidtoWrapperMtx.lock();
    clientidtoWrapper[clientId] = psw;
    clientidtoWrapperMtx.unlock();

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end); /* mark the end time */
    diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    printf("debug for subevent stage1 response time = (%llu) second\n", (long long unsigned int)diff);

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
      printf("get events (%s)\n", eventStr.data());
      eventList.push_back(eventStr);
      //default number is 1
      int trinum = 1;
      string eventMessage;
      ParseEvent(eventStr, eventMessage, trinum);
      printf("after parsing %s %d\n", eventMessage.data(), trinum);
      addNewEvent(eventMessage, trinum);
    }

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end); /* mark the end time */
    diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    printf("debug for subevent stage2 response time = (%llu) second\n", (long long unsigned int)diff);

    pubsubSubscribe(eventList, clientId);

    printf("clientid (%s) call subscribe func, waiting to be notified\n", clientId.data());

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end); /* mark the end time */
    diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    printf("debug for subevent stage3 response time = (%llu) second\n", (long long unsigned int)diff);
    //request should be a event list

    //put event list into vector and call subscribe function

    //naive implementation
    //use while loop to check a variable if satisfied value is true
    //satisfied value is controled by publish function

    //get reply
    //TODO use openmp to do parallel checking
    //int timestep = 1;
    int clientsize = 0;
    while (1)
    {

      bool notifyFlag = checkIfTriggure(clientId);

      //printf("notifyflag for client id %s (%d)\n",clientId.data(),notifyFlag);
      if (notifyFlag == true)
      {
        printf("trigure/notify curr id (%s)\n", clientId.data());

        //modify the global satisfied label to true
        //clientidtoWrapperMtx.lock();

        //clientidtoWrapper[clientid]->iftrigure = true;

        //clientidtoWrapperMtx.unlock();
        //the value should be zero after trigguring operation

        //put dynamic area of associated events into zero
        map<string, int>::iterator itsetsub;
        map<string, int> dynamicEventPushMap = clienttoSub[clientId];
        for (itsetsub = dynamicEventPushMap.begin(); itsetsub != dynamicEventPushMap.end(); ++itsetsub)
        {

          string eventkey = itsetsub->first;
          //add lock here??
          clienttoSubMtx.lock();
          clienttoSub[clientId][eventkey] = 0;
          clienttoSubMtx.unlock();
        }
        break;
      }
      else
      {
        //sleep(timestep);
        clientsize = subtoClient[debugevents].size();
        //printf("stage wait for event %s number %d clientSize %d\n",debugevents.data(),timestep, clientsize);

        usleep(1 * waitTime);
        //timestep++;
      }
    }

    //generate uid on server end

    //send message subscribe function

    //delete the clientid in the global map
    reply->set_returnmessage("TRIGGERED");

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end); /* mark the end time */
    diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    printf("debug for subevent stage4 (%s) response time = (%llu) second\n", debugevents.data(), (long long unsigned int)diff);
    //delete clientID
    deleteClient(clientId);

    //traverse clientList again, delete subtoClient

    size = eventList.size();
    for (i = 0; i < size; i++)
    {
      deleteClientFromSTC(clientId, eventList[i]);
    }

    return Status::OK;
  }

  Status Publish(ServerContext *context, const PubSubRequest *request, PubSubReply *reply)
  {

    //test respond time
    string debugeventspub;
    uint64_t diff;
    struct timespec start, end;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start); /* mark start time */

    //parse the request events
    int size = request->pubsubmessage_size();
    printf("server get (%d) published events\n", size);
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

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end); /* mark the end time */
    diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    printf("debug for publish (%s) response time = (%llu) second\n", debugeventspub.data(), (long long unsigned int)diff);
    return Status::OK;
  }
};

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
  if (argc == 2)
  {
    waitTime = atoi(argv[1]);
    printf("subscribe wait period %d\n", waitTime);
  }
  else
  {
    printf("./workflowserver <subscribe period time>\n");
    return 0;
  }

  RunServer();
  return 0;
}
