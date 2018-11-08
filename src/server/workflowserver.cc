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
#include <thread>

#include <grpc++/grpc++.h>
#include <uuid/uuid.h>

#include <mpi.h>
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

//parse the port automatically
//string NOTIFYPORT("50055");

using namespace std;

int waitTime = 1000;
int nodeNumber;
string ServerIP;
string ServerPort;

//for debugging
double subavg = 0;

double pubavg = 0;

mutex subtimesMtx;
int subtimes = 0;

mutex pubtimesMtx;
int pubtimes = 0;

//broadcaster this event to nodes in
void publishMultiServer(vector<string> eventList, string metadata)
{

  //printf("debug multi publish metadata %s\n", metadata.data());
  //get self ip:port
  string serverSocket = ServerIP + ":" + ServerPort;

  //get the multiserver ip
  int size = multiaddr.size();
  //printf("addr size %d\n", size);
  int i, subnum, replynum;
  string reply, tempaddr;
  for (i = 0; i < size; i++)
  {
    tempaddr = multiaddr[i];
    if (serverSocket.compare(tempaddr) != 0)
    {

      //ask if notified first, if number !=0 then propagate
      replynum = multiClients[tempaddr]->GetSubscribedNumber(eventList);

      //check the subtoclient in other nodes
      if (replynum > 0)
      {
        reply = multiClients[tempaddr]->Publish(eventList, "SERVER", metadata);
        if (reply.compare("OK") != 0)
        {
          printf("failed to propagate event to server %s\n", tempaddr.data());
          return;
        }
      }
      //TODO the fault tolerant for propagation

      //printf("propagate to server %s ok\n", multiaddr[i].data());
    }
  }
}

void *checkNotify(void *arguments)
{

  struct timespec start, end, finish;
  double diff;

  clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

  pubsubWrapper *psw = (pubsubWrapper *)arguments;
  string clientidstr = psw->clientID;

  int clientsize = 0;

//#ifdef DEBUG  
  printf("start checkNotify for clientid (%s)\n", clientidstr.data());
//#endif

  int times = 0;
  string satisfiedStr;

  //testusing, get the pub event
  map<string, set<int>> reqEventMap = psw->requiredeventMap;

  string eventkeywithoutNum;

  map<string, set<int>>::iterator itmap;

  string peerURL = psw->peerURL;

  //for (itmap = reqEventMap.begin(); itmap != reqEventMap.end(); ++itmap)
  //{
  //  eventkeywithoutNum = itmap->first;
  //  printf("server %s start checking notify event %s to %s\n", ServerIP.data(), eventkeywithoutNum.data(), peerURL.data());
  //}

  //on publish end, don't need while loop

  //clock_gettime(CLOCK_REALTIME, &start2);

  //bool notifyFlag = checkIfTriggure(psw, satisfiedStr);
  bool notifyFlag = psw->iftrigure;
  //printf("notifyflag for client id %s (%d)\n",clientId.data(),notifyFlag);
  if (notifyFlag == false)
  {
    return NULL;
  }

  //printf("server %s notify event %s to %s\n", ServerIP.data(), eventkeywithoutNum.data(), peerURL.data());

  //example: ipv4:192.168.11.4:59488
  GreeterClient *greeter = new GreeterClient(grpc::CreateChannel(
      peerURL.data(), grpc::InsecureChannelCredentials()));

  //get metadata when iftrigure is true
  string metadata = psw->metadata;
  printf("CheckNotify back: id %s meta %s\n", clientidstr.data(), metadata.data());
  string reply = greeter->NotifyBack(clientidstr, metadata);

  //printf("notification get reply (%s)\n", reply.data());
  //TODO delete the published event in this pubsub wrapper
  deletePubEvent(psw);

  //for testing

  //clock_gettime(CLOCK_REALTIME, &finish); /* mark the end time */
  //diff = (finish.tv_sec - start1.tv_sec) * 1.0 + (finish.tv_nsec - start1.tv_nsec) * 1.0 / BILLION;
  //printf("checknotify finish time = (%lld.%.9ld) second serverip %s\n", (long long)finish.tv_sec, finish.tv_nsec, ServerIP.data());

  //clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
  //diff = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
  //printf("checknotify (%s) checking finish time = (%lf) second serverip %s\n", satisfiedStr.data(), diff, ServerIP.data());
}

void startNotify(string eventwithoutNum)
{
  pthread_t id;

  //range all the client id
  map<string, pubsubWrapper *> subscribedMap = subtoClient[eventwithoutNum];
  //range subscribedMap
  map<string, pubsubWrapper *>::iterator itmap;

  for (itmap = subscribedMap.begin(); itmap != subscribedMap.end(); ++itmap)
  {
    string clientID = itmap->first;
    //pubsubWrapper *psw = subtoClient[eventwithoutNum][clientID];
    pubsubWrapper *psw = itmap->second;
    //printf("server %s checking notify for %s\n", ServerIP.data(), eventwithoutNum.data());
    //TODO use thread pool here
    pthread_create(&id, NULL, checkNotify, (void *)psw);
  }
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
    vector<string> eventList;
    //parse the request events
    int size = request->subevent_size();
    //printf("server get (%d) subscribed events\n", size);
    int i = 0;
    string eventStr;
    //extract event and create event List
    for (i = 0; i < size; i++)
    {
      eventStr = request->subevent(i);
      //printf("get events (%s)\n", eventStr.data());
      eventList.push_back(eventStr);
      //default number is 1
    }

    //get the request event
    //string requestEvent = request->subevent();
    //printf("search the clients number associated with %s\n", requestEvent.data());
    //search how many clients associated with this event

    int clientsNumber = getSubscribedClientsNumber(eventList);

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
    string clientPort = parsePort(peerURL);

    //this value should require from the meta data
    //string notifyAddr = clientIP + ":" + clientPort;

    string notifyAddr = request->metadata();

    //printf("debug notify addr is %s\n", notifyAddr.data());

    //this is the id used to label the identity of the client
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
    //printf("server get (%d) subscribed events\n", size);
    int i = 0;
    vector<string> eventList;
    string eventStr;

    //test respond time
    string debugevents;

    //extract event and create event List
    for (i = 0; i < size; i++)
    {
      eventStr = request->pubsubmessage(i);

//#ifdef DEBUG
      printf("get subscribed event (%s)\n", eventStr.data());
//#endif 
      eventList.push_back(eventStr);
      //default number is 1
    }

    pubsubSubscribe(eventList, clientId, notifyAddr);

    /* put this part in publish end
    //start notification
    size = eventList.size();
    int trinum = 1;
    string eventMessage;
    for (i = 0; i < size; i++)
    {
      eventMessage = eventList[i];
      ParseEvent(eventStr, eventMessage, trinum);
      //TODO the thread for checking notify shouled be recorded and stored
      //the checking notify should be killed when doing unsubscribe
      //do this after publishing
      //TODO what if submultiple events???
      startNotify(eventMessage, clientId);
    }
    */

    reply->set_returnmessage("SUBSCRIBED");

    clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
    diff = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    //TODO add lock here

    subtimesMtx.lock();
    subavg = (subavg * subtimes + diff) / (subtimes + 1);
    subtimes++;
    subtimesMtx.unlock();

    if (subtimes % 128 == 0)
    {
      printf("debug for subevent (%s) response time = (%lf) avg time = (%lf) subtimes = (%d)\n", eventList[0].data(), diff, subavg, subtimes);
    }

    return Status::OK;
  }

  Status Publish(ServerContext *context, const PubSubRequest *request, PubSubReply *reply)
  {

    //test respond time
    string debugeventspub;
    double diff, diff1, diff2, diff3;
    struct timespec start, end1, end2, end3, finish;

    string source = request->source();
    string metadata = request->metadata();

    //printf("debug source %s", source.data());
    //printf("debug get publish event source (%s) publish meta (%s)\n", source.data(), metadata.data());
    //broadcaster to other servers

    clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

    //parse the request events
    int size = request->pubsubmessage_size();
    //printf("debug msg size %d\n", size);

    int i = 0;
    vector<string> eventList;
    string eventStr;

    for (i = 0; i < size; i++)
    {
      eventStr = request->pubsubmessage(i);
//#ifdef DEBUG
      printf("debug published event %s\n", eventStr.data());
//#endif
      eventList.push_back(eventStr);
      //printf("server (%s) get (%s) published events\n", ServerIP.data(), );
    }

    //publish on one server
    pubsubPublish(eventList, metadata);

    //TODO add a string in request to label if the publish request is from client or server
    //if the request is from server return directly
    //if the request is from client, broadcaster eventList to multi server nodes

    //CLIENT or SERVER

    //broadcaster to other servers
    if (source.compare("CLIENT") == 0)
    {
      //clock_gettime(CLOCK_REALTIME, &end1); /* mark the end time */
      //diff1 = (en+d1.tv_sec - start.tv_sec) * 1.0 + (end1.tv_nsec - start.tv_nsec) * 1.0 / BILLION;

      //TODO create new thread here (use asnchronous way to do the communication)
      //publishMultiServer(eventList);

      std::thread pubthread(publishMultiServer, eventList, metadata);
      pubthread.detach();
      //only caculate time when propagation is needed
      //don't calculate time for the propagation between servers
      clock_gettime(CLOCK_REALTIME, &end2); /* mark the end time */
      diff = (end2.tv_sec - start.tv_sec) * 1.0 + (end2.tv_nsec - start.tv_nsec) * 1.0 / BILLION;

      pubtimesMtx.lock();
      pubavg = (pubavg * pubtimes + diff) / (pubtimes + 1);
      pubtimes++;
      pubtimesMtx.unlock();

      if (pubtimes % 128 == 0)
      {
        printf("debug for publish response time = (%lf) avg time = (%lf) pubtimes (%d)\n", pubavg, diff, pubtimes);
      }

      //for test using, only test one event case triggureing
      //clock_gettime(CLOCK_REALTIME, &finish); /* mark the end time */
      //diff = (finish.tv_sec - start1.tv_sec) * 1.0 + (finish.tv_nsec - start1.tv_nsec) * 1.0 / BILLION;
      //printf("publish (%s) finish time = (%lld.%.9ld) second serverip %s\n", eventList[0].data(), (long long)finish.tv_sec, finish.tv_nsec, ServerIP.data());
    }

    //if(source.compare("SERVER") == 0){
    //clock_gettime(CLOCK_REALTIME, &end3); /* mark the end time */
    //diff3 = (end3.tv_sec - start.tv_sec) * 1.0 + (end3.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    //printf("debug for publish propagate (%s) response time = (%lf)\n", debugeventspub.data(), diff3);

    //}

    //check if triggure
    //start notification
    size = eventList.size();
    int trinum = 1;
    string eventMessage;
    for (i = 0; i < size; i++)
    {
      eventMessage = eventList[i];
      //ParseEvent(eventStr, eventMessage, trinum);
      //TODO the thread for checking notify shouled be recorded and stored
      //the checking notify should be killed when doing unsubscribe
      //do this after publishing
      //TODO what if submultiple events???

      startNotify(eventMessage);
    }

    reply->set_returnmessage("OK");

    return Status::OK;
  }
};

void MultiClient()
{
  vector<string> multiAddr;
  int size;
  while (1)
  {
    multiAddr = loadMultiNodeIPPort("server");
    size = multiAddr.size();
    //if (size == nodeNumber)
    //TODO change this into a parameter
    if (size == GETIPNUMPERCLUSTER)
    {
      break;
    }

    usleep(50000);
    //printf("there are %d clients record their ip in the multinode dir\n", size);
  }

  initMultiClients("server");
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
  //std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char **argv)
{

  //MPI init
  // Initialize the MPI environment
  MPI_Init(NULL, NULL);

  // Get the number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  //get wait time ./workflowserver 1000
  printf("parameter length %d\n", argc);
  if (argc == 4)
  {
    waitTime = atoi(argv[1]);
    printf("chechNotify wait period %d\n", waitTime);
    //nodeNumber = atoi(argv[2]);

    //GETIPCOMPONENTNUM = nodeNumber;
    GETIPCOMPONENTNUM = world_size;
    printf("total instance number of the backend is %d\n", GETIPCOMPONENTNUM);

    string interfaces = string(argv[2]);

    INTERFACE = interfaces;
    printf("network interfaces is %s\n", interfaces.data());

    //NOTIFYPORT = string(argv[3]);
    //if running by mpi
    //this parameter can assigned by mpi rank
    //GETIPCOMPONENTID = atoi(argv[5]);
    GETIPCOMPONENTID = world_rank;

    int groupSize = atoi(argv[3]);
    printf("group size is %d\n", groupSize);
    GETIPNUMPERCLUSTER = groupSize;
  }
  else
  {
    printf("./workflowserver <subscribe period time><network interfaces><group size>\n");
    return 0;
  }
  //ServerPort = string("50051");
  int freePort = getFreePortNum();
  //this option should be automic in multithread case
  ServerPort = to_string(freePort);
  recordIPortForMultiNode(ServerIP, ServerPort);
  MultiClient();
  RunServer(ServerIP, ServerPort);
  return 0;
}
