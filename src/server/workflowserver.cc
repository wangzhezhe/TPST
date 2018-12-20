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
#include <omp.h>

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

#include "../utils/coordinator/coordinator.h"

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

using workflowserver::RecordSubReply;
using workflowserver::RecordSubRequest;

using workflowserver::RedistributeReply;
using workflowserver::RedistributeRequest;

using workflowserver::UpdateClusterReply;
using workflowserver::UpdateClusterRequest;

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

// 0.5 s
int coorCheckPeriod = 10000000;

const int checkingPeriod = 100000;

typedef struct NotifyInfo
{
  string addr;
  string metaInfo;
  string clientid;
} NotifyInfo;

mutex nqmutex;
deque<NotifyInfo> notifyQueue;

int notifySleep = 1000000;

void publishMultiGroup(vector<string> eventList, string metadata)
{

  //range coordinatorAddrSet
  set<string>::iterator it = coordinatorAddrSet.begin();

  // Iterate till the end of set
  while (it != coordinatorAddrSet.end())
  {
    // Print the element
    string coordNator = (*it);
    //Increment the iterator
    it++;

    if (coordNator.compare(ServerAddr) != 0)
    {
      GreeterClient *greeter = new GreeterClient(grpc::CreateChannel(
          coordNator.data(), grpc::InsecureChannelCredentials()));
      //broadcasting between groups
      string reply = greeter->Publish(eventList, sourceBetweengroup, metadata);

      if (reply.compare("OK") != 0)
      {
        printf("failed to propagate event to coordinator %s\n", coordNator.data());
      }
    }
  }
}

//broadcaster this event to nodes in group

void publishMultiServer(vector<string> eventList, string metadata)
{

  printf("debug call publishMultiServer\n");
  //get the cluster dir
  string clusterWorkerDir = GM_CLUSTERDIR + "/" + gm_workerDir;
  vector<string> workerList = loadAddrInDir(clusterWorkerDir);

  //get the address for all the nodes in group (except its self in working group)
  int size = workerList.size();

  //broadcast the event to all the nodes

  for (int i = 0; i < size; i++)
  {

    string tempaddr = workerList[i];
    if (ServerAddr.compare(tempaddr) != 0)
    {
      //send request
      GreeterClient *greeter = new GreeterClient(grpc::CreateChannel(
          tempaddr.data(), grpc::InsecureChannelCredentials()));
      string reply = greeter->Publish(eventList, sourceIngroup, metadata);

      printf("broad cast to server %s\n", tempaddr.data());

      if (reply.compare("OK") != 0)
      {
        printf("failed to propagate event to server %s\n", tempaddr.data());
        return;
      }
    }
  }

  return;
}

void *checkNotify(void *arguments)
{

  struct timespec start, end, finish;
  double diff;

  //clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

  pubsubWrapper *psw = (pubsubWrapper *)arguments;
  string clientidstr = psw->clientID;

  int clientsize = 0;

  //#ifdef DEBUG
  //printf("start checkNotify for clientid (%s)\n", clientidstr.data());
  //#endif

  int times = 0;
  string satisfiedStr;

  string eventkeywithoutNum;

  string peerURL = psw->peerURL;

  //while (1)
  //{
  //printf("notifyflag for client id (%s) status (%d)\n", clientidstr.data(), notifyFlag);

  psw->pswmtx.lock();

  if (psw->iftrigure == false)
  {
    //usleep(checkingPeriod);
    psw->pswmtx.unlock();
    return NULL;
  }
  else
  {
    //avoid notify multiple times
    psw->iftrigure = false;
  }

  psw->pswmtx.unlock();

  printf("debug prepare to notify psw\n");
  //}

  //TODO put the metadata and the url at the notifyqueue
  //TODO use another thread to get the element from the queue periodically

  NotifyInfo ninfo = NotifyInfo();
  ninfo.addr = peerURL.data();
  ninfo.metaInfo = psw->metadata;
  ninfo.clientid = clientidstr;

  nqmutex.lock();
  notifyQueue.push_back(ninfo);
  nqmutex.unlock();

  subtoClientMtx.lock();
  deletePubEvent(psw);

  subtoClientMtx.unlock();

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
  subtoClientMtx.lock();
  map<string, pubsubWrapper *> subscribedMap = subtoClient[eventwithoutNum];

  //printf("debug startnotify server id %d mapsize %d indexEvent %s\n", gm_rank, subscribedMap.size(), eventwithoutNum.data());

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

    //delete psw here
  }
  subtoClientMtx.unlock();
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

    //printf("debug subscribe server id %d addr %s recieve subscription\n",  gm_rank, ServerAddr.data());

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

    // propagate subscription
    string source = request->source();

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
      printf("server %d get subscribed event (%s)\n", gm_rank, eventStr.data());
      //#endif
      eventList.push_back(eventStr);
      //default number is 1
    }

    pubsubSubscribe(eventList, clientId, notifyAddr);

    reply->set_returnmessage("SUBSCRIBED");

    clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */

    subtimesMtx.lock();
    diff = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    subavg = (subavg * subtimes + diff) / (subtimes + 1);
    subtimes++;
    subtimesMtx.unlock();

    //if (subtimes % 128 == 0)
    //{
    printf("debug server id %d for subevent (%s) response time = (%lf) avg time = (%lf) subtimes = (%d)\n", gm_rank, eventList[0].data(), diff, subavg, subtimes);
    //}

    //get coordinator clients
    //this should be updated after checking cluster !!!!
    string clusterDir = GM_CLUSTERDIR;
    if (coordinatorClients.size() == 0)
    {
      //update coordinator
      updateWorkerClients(clusterDir);
      updateCoordinatorClients(clusterDir);
    }

    //if still zero, return
    if (coordinatorClients.size() == 0)
    {
      printf("failed to get coordinate clients for server id %d\n", gm_rank);
      return Status::OK;
    }

    //TODO assume there are only one coordinator
    coordinatorClientsLock.lock();
    GreeterClient *coordinatorClient = coordinatorClients[0];
    coordinatorClientsLock.unlock();

    string serverAddr = ServerAddr;

    for (i = 0; i < eventList.size(); i++)
    {
      eventStr = eventList[i];
      //for multiple sub, only record the first one
      if (subtoClient.find(eventStr) != subtoClient.end())
      {
        int subNum = subtoClient[eventStr].size();

        string reply = coordinatorClient->RecordSub(eventStr, serverAddr, subNum);
        if (reply.compare("OK") != 0)
        {
          printf("server %s recorde for coordinator fail\n", eventStr.data());
        }
      }
    }

    if (propagateSub == true && source.compare(sourceClient) == 0)
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

      for (map<string, GreeterClient *>::iterator it = greeterMap.begin(); it != greeterMap.end(); ++it)
      {
        string key = it->first;

        if (key.compare(ServerAddr) != 0)
        {
          GreeterClient *greeter = greeterMap[key];
          string reply = greeter->Subscribe(eventList, clientId, notifyAddr, sourceIngroup);

          if (reply.compare("SUBSCRIBED") != 0)
          {
            printf("propagation to server %s fail\n", ServerAddr.data());
          }

          printf("propagate event %s to server %s\n", eventList[0].data(), key.data());
        }
      }
    }

    return Status::OK;
  }

  Status RedistributeSub(ServerContext *context, const RedistributeRequest *request, RedistributeReply *reply)
  {

    double difftime;
    struct timespec start, end;

    printf("RedistributeSub is called\n");

    string eventStr = request->subevent();
    printf("debug get subevent \n");

    string srcAddr = request->srcaddr();
    printf("debug get srcAddr \n");

    string dstAddr = request->destaddr();
    printf("debug get dstAddr \n");

    int diff = request->diff();
    printf("debug get diff \n");

    mutex countMutex;
    int count = 0;
    //go through map

    printf("server %s redistribution src %s dst %s event %s diff %d\n",
           ServerAddr.data(), srcAddr.data(), dstAddr.data(), eventStr.data(), diff);

    map<string, pubsubWrapper *>::iterator itera;

    if (subtoClient.find(eventStr) == subtoClient.end())
    {
      printf("no sub eventStr %s in server %s\n", eventStr.data(), ServerAddr.data());
      return Status::OK;
    }

    printf("debug size of client subscribe eventStr %d\n", subtoClient[eventStr].size());

    clock_gettime(CLOCK_REALTIME, &start);

    vector<SimplepubsubWrapper *> spsw;

    subtoClientMtx.lock();

    for (itera = subtoClient[eventStr].begin(); itera != subtoClient[eventStr].end(); ++itera)
    {
      string clientid = itera->first;
      pubsubWrapper *psw = itera->second;

      SimplepubsubWrapper *temppsw = getSimplepubsubWrapper(psw);
      spsw.push_back(temppsw);

      //get greeter from the dstAddr

      //delete during traverse
      //refer to https://www.cnblogs.com/zhoulipeng/p/3432009.html
      delete itera->second;
      subtoClient[eventStr].erase(itera++);

      //unsubscribe current info
      //eventUnSubscribe(eventStr, clientid);
      countMutex.lock();
      count++;
      countMutex.unlock();

      if (count == diff)
      {
        break;
      }
      //printf("count is %d\n", count);
    }

    printf("debug size for self %d\n", subtoClient[eventStr].size());

    subtoClientMtx.unlock();

    clock_gettime(CLOCK_REALTIME, &end);
    difftime = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;

    GreeterClient *greeter = new GreeterClient(grpc::CreateChannel(
        dstAddr.data(), grpc::InsecureChannelCredentials()));

    for (int i = 0; i < spsw.size(); i++)
    {

      //printf("send clientid %s to dest %s\n", clientid.data(), dstAddr.data());
      //subscribe to dest
      SimplepubsubWrapper *temppsw = spsw[i];

      string clientid = temppsw->clientID;
      string notifyAddr = temppsw->peerURL;
      vector<string> eventList = temppsw->eventList;

      string reply = greeter->Subscribe(eventList, clientid, notifyAddr, sourceIngroup);

      if (reply.compare("SUBSCRIBED") != 0)
      {
        printf("redistribution, subscribe to server %s fail\n", dstAddr.data());
      }
    }

    printf("RedistributeSub for eventStr %s is %lf\n", eventStr.data(), difftime);

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

      //#endif
      eventList.push_back(eventStr);
      //printf("server (%s) get (%s) published events\n", ServerIP.data(), );

      //printf("debug size %d eventList size %d server %d get publish event source (%s) publish meta (%s) publish event (%s)\n",
      //       size, eventList.size(), gm_rank, source.data(), metadata.data(), eventStr.data());
    }

    //publish on one server
    pubsubPublish(eventList, metadata);

    //printf("debug publish id %d pubevent %s pubsubPublish ok\n", gm_rank, eventList[0].data());

    //broadcaster to other servers in same group
    if (propagatePub == true && source.compare(sourceClient) == 0)
    {

      //TODO create new thread here (use asnchronous way to do the communication)
      //publishMultiServer(eventList);

      //printf("id %d pub to multiserver\n", gm_rank);

      std::thread pubthread(publishMultiServer, eventList, metadata);
      pubthread.detach();
    }

    if (SERVERSTATUS.compare(status_coor) == 0)
    {
      //printf("id %d status is coordinator\n", gm_rank);

      //recieve info from in group server
      if (source.compare(sourceIngroup) == 0 || source.compare(sourceClient) == 0)
      {
        //printf("id %d sourceIngroup or from sourceClient %s\n", gm_rank, source.data());

        //broadcast to all the coordinator in all groups
        //get all the coordinator in all the group
        std::thread multiGroupthread(publishMultiGroup, eventList, metadata);
        multiGroupthread.detach();
      }
      //recieve events from client direactly or from group members
      if (source.compare(sourceBetweengroup) == 0)
      {

        //if there is sub info in map, broadcaster to all server in group
        //else do nothing
        //printf("debug id %d recieve event from other coordinator\n", gm_rank);

        //assume the event will be published one by one
        if (eventRecordMap.find(eventStr) != eventRecordMap.end())
        {
          thread pubthread(publishMultiServer, eventList, metadata);
          pubthread.detach();
        }
      }
    }

    //printf("debug publish id %d pubevent %s broadcasting\n", gm_rank, eventList[0].data());

    clock_gettime(CLOCK_REALTIME, &end1);
    diff1 = (end1.tv_sec - start.tv_sec) * 1.0 + (end1.tv_nsec - start.tv_nsec) * 1.0 / BILLION;

    pubtimesMtx.lock();
    pubavg = (pubavg * pubtimes + diff1) / (pubtimes + 1);
    pubtimes++;
    pubtimesMtx.unlock();

    //if (pubtimes % 128 == 0)
    //{
    //printf("debug for publish (%s) response time = (%lf) avg time = (%lf) pubtimes (%d) current id %d\n", eventStr.data(), pubavg, diff1, pubtimes, gm_rank);
    //}

    size = eventList.size();
    string peerURL = context->peer();

    if (size > 0)
    {
      //use first as the index, only start notify when there is sub info on current server process
      getIndexMtx.lock();
      if (getIndexMap.find(eventStr) != getIndexMap.end())
      {
        //printf("debug start notify event %s source %s peer %s\n", eventStr.data(), source.data(), peerURL.data());
        //outputsubtoClient();
        //get indexEvent
        set<string> indexEventSet = getIndexMap[eventStr];
        for (set<string>::iterator it = indexEventSet.begin(); it != indexEventSet.end(); ++it)
        {
          string indexEvent = *it;
          startNotify(indexEvent);
        }
      }
      getIndexMtx.unlock();
    }

    reply->set_returnmessage("OK");

    return Status::OK;
  }

  Status RecordSub(ServerContext *context, const RecordSubRequest *request, RecordSubReply *reply)
  {

    string serveraddr = request->serveraddr();
    string subevent = request->subevent();

    //TODO use aggregation message to send the info
    int subnum = request->subnum();

    if (subnum % 128 == 0)
    {
      printf("server addr is %s and sub number is %d\n", serveraddr.data(), subnum);
    }

    //update the recordmap
    //void updateStatusMap(string event, string serverAddr, int subNumber)

    eventRecordMapLock.lock();
    updateStatusMap(subevent, serveraddr, subnum);
    eventRecordMapLock.unlock();
    reply->set_returnmessage("OK");

    return Status::OK;
  }

  Status UpdateCluster(ServerContext *context, const UpdateClusterRequest *request, UpdateClusterReply *reply)
  {
    string clusterDir = request->cluster();
    GM_CLUSTERDIR = clusterDir;
    //update the coordinator clients again
    updateCoordinatorClients(GM_CLUSTERDIR);
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

void RedistributeByPlan(vector<Plan> planList)
{

  int size = planList.size();

  //get greeter for src (src are same)

  printf("plan size is %d\n", size);
  omp_lock_t writelock;

  omp_init_lock(&writelock);
#pragma omp parallel
  {
#pragma omp for
    for (int i = 0; i < size; i++)
    {
      int num_threads = omp_get_num_threads();
      printf("Thread rank: %d\n", num_threads);
      Plan plan = planList[i];
      string moveEvent = plan.moveSubscription;
      string srcaddr = plan.srcAddr;
      string destaddr = plan.destAddr;

      int diff = plan.moveNumber;

      //unsubscribe event string at src
      //subscribe event string at dest

      //send redistribute request
      printf("send redistribution moveEvent %s src %s dst %s diff %d\n",
             moveEvent.data(), srcaddr.data(), destaddr.data(), diff);

      GreeterClient *greeter = new GreeterClient(grpc::CreateChannel(
          srcaddr.data(), grpc::InsecureChannelCredentials()));

      greeter->RedistributeSub(moveEvent, srcaddr, destaddr, diff);

      //update the sub num for self

      //this function is called on coordinator
      int newSubTime = subtoClient[moveEvent].size();

      printf("newsize after redistribution %d for str %s for server %s\n", newSubTime, moveEvent.data(), ServerAddr.data());
      omp_set_lock(&writelock);

      eventRecordMapLock.lock();
      eventRecordMap[moveEvent][srcaddr] = newSubTime;
      eventRecordMapLock.unlock();
      // one thread at a time stuff
      omp_unset_lock(&writelock);
      printf("finish plan\n");
    }
  }
  omp_destroy_lock(&writelock);
}

//check
void checkOverload()
{

  //range map
  //for every event
  //calculate the number of server process
  //calculate the total subscription number of all events
  //if one average > threshold
  //add into map
  vector<string> imBalancedEventVector;

  eventRecordMapLock.lock();
  for (map<string, map<string, int>>::iterator it = eventRecordMap.begin(); it != eventRecordMap.end(); ++it)
  {
    string event = it->first;

    double avg = getAverage(event);

    printf("avg for event %s is %lf\n", event.data(), avg);
    if (avg > overLoadThreshold * 1.0)
    {
      //cacluate adding new nodes number
      imBalancedEventVector.push_back(event);
    }
  }
  eventRecordMapLock.unlock();

  int imbsize = imBalancedEventVector.size();

  printf("new imbalance event size %d\n", imbsize);

  if (imbsize == 0)
  {
    return;
  }

  vector<string> serverAddrList;

  //range the imbalancedVector
  //calculate freeNumber
  //get max value among them
  int serverNumber = 1;
  for (int i = 0; i < imbsize; i++)
  {
    printf("imbalance event %s\n", imBalancedEventVector[i].data());
    string imbEvent = imBalancedEventVector[i];
    int subSize = eventRecordMap[imbEvent].size();
    double avg = getAverage(imbEvent);
    int addNumber = calculateNewServerNum(avg, subSize);
    serverNumber = max(serverNumber, addNumber);

    printf("get serverNumber %d server process from pool\n", serverNumber);

    // get free node, the value is serverNumber
    //TODO execute the dir addr replacement at this step
    //TODO consider if there is not enough resources in freepool
    //vector<string> serverAddrList = fakeGetServerList(serverNumber);

    serverAddrList = getFreeNodeList(serverNumber);

    int listSize = serverAddrList.size();
    printf("avaliable new listsize is %d\n", listSize);
    for (int j = 0; j < listSize; j++)
    {
      string serAddr = serverAddrList[j];

      //update the cluster dir for new added nodes
      //(after subscribing, the event record map at the coordinate node need to be updated)

      GreeterClient *greeter = new GreeterClient(grpc::CreateChannel(
          serAddr.data(), grpc::InsecureChannelCredentials()));

      string newCluster = GM_CLUSTERDIR;
      string reply = greeter->UpdateCluster(newCluster);

      if (reply.compare("OK") != 0)
      {
        printf("update cluetsr %s fail\n", newCluster.data());
      }

      //void updateStatusMap(string event, string serverAddr, int subNumber)
      updateStatusMap(imbEvent, serAddr, 0);
    }

    //debug
    //outputMap();
    //all the data related to this event is updated

    vector<Plan> planList = generatePlanForEvent(imbEvent);

    //debug plan
    for (int k = 0; k < planList.size(); k++)
    {
      printf("plan: event %s src %s dest %s num %d\n",
             planList[k].moveSubscription.data(), planList[k].srcAddr.data(), planList[k].destAddr.data(), planList[k].moveNumber);
    }

    RedistributeByPlan(planList);

    //TODO put server list in new dir

    int addrSize = serverAddrList.size();

    for (int i = 0; i < addrSize; i++)
    {
      string newAddr = serverAddrList[i];
      bool updateOk = nodeAttach(GM_CLUSTERDIR, newAddr);
      if (updateOk == false)
      {
        printf("failed to add server (%s) into cluster (%s)\n", newAddr.data(), GM_CLUSTERDIR.data());
      }
    }

    // move a to b for specific number
    // redistribution
    // update Dir
  }
  return;
}

void periodChecking()
{

  //while (1)
  //  {
  usleep(coorCheckPeriod);
  printf("initial status:\n");
  outputMap();

  struct timespec start, end1;
  double diff1;

  clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

  checkOverload();

  clock_gettime(CLOCK_REALTIME, &end1); /* mark start time */
  diff1 = (end1.tv_sec - start.tv_sec) * 1.0 + (end1.tv_nsec - start.tv_nsec) * 1.0 / BILLION;

  //   }

  printf("total time for checkOverload and redistribution is %lf\n", diff1);

  while (1)
  {
    usleep(5000000);
    outputMap();
  }
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

  if (argc == 8)
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

    overLoadThreshold = atoi(argv[7]);
  }
  else
  {
    printf("./workflowserver <subscribe period time> <network interfaces> <group size> <group number> <size of thread pool> <notify sleep> <threshold for coordinator checking>\n");
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

  propagateSub = false;

  propagatePub = true;

  const bool startPeridChecking = false;

  printf("server id %d server status %s\n", gm_rank, SERVERSTATUS.data());

  thread runServer(RunServer, ServerIP, ServerPort, threadPoolSize);

  if (startPeridChecking == true && SERVERSTATUS.compare(status_coor) == 0)
  {
    printf("server id %d is coordinator, run group checking\n", gm_rank);
    thread tCheck(periodChecking);
    tCheck.join();
  }

  //update the coordinator addr
  //wait all server to write data into dir
  sleep(1);
  updateCoordinatorAddr();
  runServer.join();

  return 0;
}
