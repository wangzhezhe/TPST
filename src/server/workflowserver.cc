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

int waitTime = 1000;
int nodeNumber;
string ServerIP;
string ServerPort;

//broadcaster this event to nodes in
void publishMultiServer(vector<string> eventList)
{

  //get self ip:port
  string serverSocket = ServerIP + ":" + ServerPort;

  //get the multiserver ip
  int size = multiaddr.size();
  printf("addr size %d\n", size);
  int i;
  string reply;
  for (i = 0; i < size; i++)
  {
    if (serverSocket.compare(multiaddr[i]) != 0)
    {
      //set publish request
      //TODO the fault tolerant for propagation
      reply = multiClients[multiaddr[i]]->Publish(eventList, "SERVER");
      if (reply.compare("OK") != 0)
      {
        printf("failed to propagate event to server %s\n", multiaddr[i].data());
        return;
      }

      printf("propagate to server %s ok\n", multiaddr[i].data());
    }
  }
}

void *checkNotify(void *arguments)
{

  struct timespec start1, end1;
  double diff;

  clock_gettime(CLOCK_REALTIME, &start1); /* mark start time */

  pubsubWrapper *psw = (pubsubWrapper *)arguments;
  string clientidstr = psw->clientID;
  int clientsize = 0;
  printf("start checkNotify for clientid (%s)\n", clientidstr.data());
  int times = 0;
  while (1)
  {
    //clock_gettime(CLOCK_REALTIME, &start2); /* mark start time */

    bool notifyFlag = checkIfTriggure(psw);

    //printf("notifyflag for client id %s (%d)\n",clientId.data(),notifyFlag);
    if (notifyFlag == true)
    {
      break;
    }
    else
    {

      srand((unsigned)time(0));

      usleep(1 * (waitTime));
      times++;
      //printf("checknotify %s sleep time %d\n", clientidstr.data(), times);
    }
  }

  string peerURL = psw->peerURL;

  //example: ipv4:192.168.11.4:59488
  //printf("use peerurl %s\n", peerURL.data());
  //GreeterClient *greeter = getClientFromAddr(peerURL);
  printf("----notify back peer url %s\n", peerURL.data());
  GreeterClient greeter(grpc::CreateChannel(
      peerURL.data(), grpc::InsecureChannelCredentials()));

  string reply = greeter.NotifyBack(clientidstr);

  //TODO delete the published event in this pubsub wrapper
  deletePubEvent(psw);

  clock_gettime(CLOCK_REALTIME, &end1); /* mark the end time */
  diff = (end1.tv_sec - start1.tv_sec) * 1.0 + (end1.tv_nsec - start1.tv_nsec) * 1.0 / BILLION;
  printf("debug for checknotify %s response time = (%lf) second\n", clientidstr.data(), diff);
}

void startNotify(string eventwithoutNum, string clientID)
{
  pthread_t id;
  pubsubWrapper *psw = subtoClient[eventwithoutNum][clientID];
  pthread_create(&id, NULL, checkNotify, (void *)psw);
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

    //extract event and create event List
    for (i = 0; i < size; i++)
    {
      eventStr = request->pubsubmessage(i);
      //printf("get events (%s)\n", eventStr.data());
      eventList.push_back(eventStr);
      //default number is 1
    }

    pubsubSubscribe(eventList, clientId, notifyAddr);

    //start notification
    size = eventList.size();
    int trinum = 1;
    string eventMessage;
    for (i = 0; i < size; i++)
    {
      eventMessage = eventList[i];
      ParseEvent(eventStr, eventMessage, trinum);
      startNotify(eventMessage, clientId);
    }

    reply->set_returnmessage("SUBSCRIBED");

    clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
    diff = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    printf("debug for subevent (%s) response time = (%lf) second\n", eventMessage.data(), diff);

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

    //publish on one server
    pubsubPublish(eventList);
    reply->set_returnmessage("OK");

    clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
    diff = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    printf("debug for publish (%s) response time = (%lf) second\n", debugeventspub.data(), diff);

    //TODO add a string in request to label if the publish request is from client or server
    //if the request is from server return directly
    //if the request is from client, broadcaster eventList to multi server nodes

    //CLIENT or SERVER

    string source = request->source();

    if (source.compare("CLIENT") == 0)
    {
      publishMultiServer(eventList);
    }

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
    printf("there are %d clients record their ip in the multinode dir\n", size);
  }

  initMultiClients();
}

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

int main(int argc, char **argv)
{

  //get wait time ./workflowserver 1000
  printf("parameter length %d\n", argc);
  if (argc == 4)
  {
    waitTime = atoi(argv[1]);
    printf("chechNotify wait period %d\n", waitTime);
    nodeNumber = atoi(argv[2]);
    printf("instance number of the backend is %d\n", nodeNumber);
    string interfaces = string(argv[3]);

    INTERFACE = interfaces;

    printf("network interfaces is %s\n", interfaces.data());
  }
  else
  {
    printf("./workflowserver <subscribe period time> <number of the nodes> <network interfaces>\n");
    return 0;
  }
  ServerPort = string("50051");
  recordIPortForMultiNode(ServerIP, ServerPort);
  MultiClient();
  RunServer(ServerIP, ServerPort);
  return 0;
}
