#ifndef PUBSUBCLIENT_H
#define PUBSUBCLIENT_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <unistd.h>
#include <pthread.h>

#include <grpc++/grpc++.h>

#include "workflowserver.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using workflowserver::Greeter;
using workflowserver::HelloReply;
using workflowserver::HelloRequest;
using workflowserver::PubSubReply;
using workflowserver::PubSubRequest;
//using workflowserver::SubscribedNumberRequest;
//using workflowserver::SubscribedNumberReply;
using workflowserver::SubNumRequest;
using workflowserver::SubNumReply;

using workflowserver::NotifyReply;
using workflowserver::NotifyRequest;


using namespace std;

class GreeterClient
{


private:
  unique_ptr<Greeter::Stub> stub_;



public:
  static GreeterClient *getClient();
 
 GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  //GreeterClient(std::shared_ptr<Channel> channel)
  //    : stub_(Greeter::NewStub(channel)) {}

  string SayHello(const string &user);

  string Subscribe(vector<string> eventSubList, string clientID, string notifyAddr, string source, string matchType, string metadata);

  string Publish(vector<string> eventList,string source, string metadata);
  
  string NotifyBack(string clientId,string metadata);
 
  int GetSubscribedNumber(vector<string> eventList);

  string RecordSub(string subevent, string serverAddr, int subNum);

  string RedistributeSub(string subevent, string srcAddr, string dstAddr, int diffNum);

  void initMultiClients(string identity);

  string UpdateCluster(string newClusterDir);
};

//define as singleton global variable
//extern GreeterClient greeter;

//GreeterClient& intiSocketAddr();

//GreeterClient *getClientFromAddr(string peerURL);

//GreeterClient *roundrobinGetClient();

void initMultiClients(string identity);

void initMultiClientsByClusterDir(string clusterDir);

void updateWorkerClients(string groupDir);

void updateCoordinatorClients(string groupDir);

GreeterClient *getClientFromEvent(string eventString);

vector<GreeterClient *> getClientsExcept(string groupDir, string ipaddr);

void initClients(string clusterDir);

//TODO don't extern those two in future

extern map<string, GreeterClient *> multiClients;

extern map<string, map<string, GreeterClient *>> workerClients;
extern vector<GreeterClient *> coordinatorClients;;

extern mutex coordinatorClientsLock;

#endif