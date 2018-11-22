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
#include <queue>

#include <grpc++/grpc++.h>
#include <uuid/uuid.h>

#include <mpi.h>
#include "pubsub.h"
#include "unistd.h"
#include <mutex>
#include <stdint.h> /* for uint64 definition */
#include <stdlib.h> /* for exit() definition */
#include <time.h>   /* for clock_gettime */

#include "../utils/threadpool/ThreadPool.h"

#include "../utils/groupManager/groupManager.h"

#include "../utils/dht/dht.h"

#include "../publishclient/pubsubclient.h"

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
string ServerAddr;

//for debugging
double subavg = 0;

double pubavg = 0;

mutex subtimesMtx;
int subtimes = 0;

mutex pubtimesMtx;
int pubtimes = 0;

ThreadPool *globalThreadPool = NULL;

mutex resultmutex;
deque<std::shared_future<void *>> resultQueue;

bool propagateSub = false;
bool propagatePub = false;

typedef struct NotifyInfo
{
  string addr;
  string metaInfo;
  string clientid;
} NotifyInfo;

mutex nqmutex;
deque<NotifyInfo> notifyQueue;

int notifySleep = 1000000;

//broadcaster this event to nodes in
/* todo update the propagation process
void publishMultiServer(vector<string> eventList, string metadata)
{

  //printf("debug multi publish metadata %s\n", metadata.data());
  //get self ip:port
  string serverSocket = ServerIP + ":" + ServerPort;

  //get the multiserver ip
  //TODO this group info should be updated dynamically
  string clusterDir = GM_CLUSTERDIR;
  vector<string> addrList = workerAddrMap[clusterDir];
  int size = addrList.size();
  //printf("addr size %d\n", size);
  int i, subnum, replynum;
  string reply, tempaddr;
  for (i = 0; i < size; i++)
  {
    tempaddr = addrList[i];
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
*/

void *checkNotify(void *arguments)
{

  struct timespec start, end, finish;
  double diff;

  clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

  pubsubWrapper *psw = (pubsubWrapper *)arguments;
  string clientidstr = psw->clientID;

  int clientsize = 0;

  //#ifdef DEBUG
  //printf("start checkNotify for clientid (%s)\n", clientidstr.data());
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

  //TODO put the metadata and the url at the notifyqueue
  //TODO use another thread to get the element from the queue periodically

  NotifyInfo ninfo = NotifyInfo();
  ninfo.addr = peerURL.data();
  ninfo.metaInfo = psw->metadata;
  ninfo.clientid = clientidstr;

  nqmutex.lock();
  notifyQueue.push_back(ninfo);
  nqmutex.unlock();

  deletePubEvent(psw);

  //printf("server %s notify event %s to %s\n", ServerIP.data(), eventkeywithoutNum.data(), peerURL.data());

  /*

 

  //printf("notification get reply (%s)\n", reply.data());
  //TODO delete the published event in this pubsub wrapper
  //Modify the published number
  //if the number is zero, then delete
  //if the number is not zero, poblish multiple times
 

  return NULL;
  */

  //for testing

  //clock_gettime(CLOCK_REALTIME, &finish); /* mark the end time */
  //diff = (finish.tv_sec - start1.tv_sec) * 1.0 + (finish.tv_nsec - start1.tv_nsec) * 1.0 / BILLION;
  //printf("checknotify finish time = (%lld.%.9ld) second serverip %s\n", (long long)finish.tv_sec, finish.tv_nsec, ServerIP.data());

  //clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
  //diff = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
  //printf("checknotify (%s) checking finish time = (%lf) second serverip %s\n", satisfiedStr.data(), diff, ServerIP.data());
}

void notifyback(string peerURL, string metadata, string clientidstr)
{
  //example: ipv4:192.168.11.4:59488
  GreeterClient *greeter = new GreeterClient(grpc::CreateChannel(
      peerURL.data(), grpc::InsecureChannelCredentials()));

  //printf("CheckNotify back:id %s meta %s addr %s\n", clientidstr.data(), metadata.data(), peerURL.data());
  string reply = greeter->NotifyBack(clientidstr, metadata);
  return;
}

void getElementFromNotifyQ()
{
  int len = 0;

  while (1)
  {
    int size = notifyQueue.size();

    if (size > 0)
    {
      printf("current notifyQueue size %d\n", size);
      for (int i = 0; i < 512; i++)
      {
        //get front
        if (notifyQueue.size() > 0)
        {

          nqmutex.lock();
          NotifyInfo ninfo = notifyQueue.front();
          //send request and notify back
          notifyback(ninfo.addr, ninfo.metaInfo, ninfo.clientid);
          notifyQueue.pop_front();
          nqmutex.unlock();
        }
        else
        {
          break;
        }
      }
    }
    else
    {
      //printf("no task\n");
      usleep(notifySleep);
    }
  }
}

void check()
{
  int len = 0;

  while (1)
  {
    int size = resultQueue.size();

    if (size > 0)
    {
      int i = 0;
      for (i = 0; i < size; i++)
      {
        //mtx.lock();
        //std::shared_future<int> result = resultList[i];
        resultmutex.lock();
        shared_future<void *> result = resultQueue.front();
        result.get();
        resultQueue.pop_front();
        resultmutex.unlock();
        //mtx.unlock();
      }
    }
    else
    {
      //printf("no task\n");
      sleep(1);
    }
  }
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
    //pthread_create(&id, NULL, checkNotify, (void *)psw);
    //join the thread into the threadpool
    shared_future<void *> result = globalThreadPool->enqueue(checkNotify, (void *)psw);

    resultmutex.lock();
    resultQueue.push_back(result);
    resultmutex.unlock();
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

  // TODO add group checking function
  // update the group status in this server

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

    printf("debug subscribe server id %d addr %s recieve subscription\n",  gm_rank, ServerAddr.data());

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

    subtimesMtx.lock();
    diff = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    subavg = (subavg * subtimes + diff) / (subtimes + 1);
    subtimes++;
    subtimesMtx.unlock();

    //if (subtimes % 128 == 0)
    //{
    printf("debug for subevent (%s) response time = (%lf) avg time = (%lf) subtimes = (%d)\n", eventList[0].data(), diff, subavg, subtimes);
    //}

    //TODO propagate subscription
    string source = request->source();

    if (propagateSub == true && source.compare("CLIENT") == 0)
    {
      printf("propagate subscription to other nodes in group\n");

      //get the client in current group

      //assume every client subscribe one events
      //assume there is one event msg subscription
      string eventMsg = eventStr;
      string clusterDir = getClusterDirFromEventMsg(eventMsg);

      if (workerClients.find(clusterDir) == workerClients.end())
      {
        initClients(clusterDir);
      }

      map<string, GreeterClient *> greeterMap = workerClients[clusterDir];

      for (map<string, GreeterClient *>::iterator it=greeterMap.begin(); it!=greeterMap.end(); ++it)
      {
        string key = it->first;

        if (key.compare(ServerAddr) != 0)
        {
          GreeterClient *greeter = greeterMap[key];
          string reply = greeter->Subscribe(eventList, clientId, notifyAddr, "SERVER");

          if (reply.compare("SUBSCRIBED") != 0)
          {
            printf("propagation to server %s fail\n",ServerAddr.data());
          }

          printf("propagate event %s to server %s\n",eventList[0].data(),key.data());
        }
      }
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
      //printf("debug published event %s\n", eventStr.data());
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

      //std::thread pubthread(publishMultiServer, eventList, metadata);
      //pubthread.detach();
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

/*
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
*/

void RunServer(string serverIP, string serverPort, int threadPool)
{
  //init thread pool
  ThreadPool threadPoolInstance(threadPool);
  globalThreadPool = &threadPoolInstance;
  //start check()
  thread tcheck(check);
  thread notifycheck(getElementFromNotifyQ);

  string socketAddr = serverIP + ":" + serverPort;
  //ServerAddr is global value
  ServerAddr = socketAddr;
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

  tcheck.join();
  notifycheck.join();
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

  int threadPoolSize = 32;

  if (argc == 7)
  {
    waitTime = atoi(argv[1]);
    printf("chechNotify wait period %d\n", waitTime);

    string interfaces = string(argv[2]);

    GM_INTERFACE = interfaces;
    printf("network interfaces is %s\n", interfaces.data());

    gm_rank = world_rank;

    int groupSize = atoi(argv[3]);
    printf("group size is %d\n", groupSize);
    gm_requiredGroupSize = groupSize;

    gm_groupNumber = atoi(argv[4]);
    printf("group number is %d\n", gm_groupNumber);

    threadPoolSize = atoi(argv[5]);

    notifySleep = atoi(argv[6]);
  }
  else
  {
    printf("./workflowserver <subscribe period time><network interfaces><group size><group number><size of thread pool><notify sleep>\n");
    return 0;
  }
  //ServerPort = string("50051");
  int freePort = getFreePortNum();
  //this option should be automic in multithread case
  ServerPort = to_string(freePort);

  //recordIPortForMultiNode(ServerIP, ServerPort);
  //MultiClient();

  string ServerIP;

  string clusterDir = getClusterDir(gm_rank);

  recordIPortIntoClusterDir(ServerIP, ServerPort, clusterDir, gm_requiredGroupSize);

  //printf("file Num for clusterDir %s is %d\n",clusterDir.data(), fileNum);

  propagateSub = true;

  propagatePub = false;

  RunServer(ServerIP, ServerPort, threadPoolSize);
  return 0;
}
