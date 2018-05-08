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
using namespace std;

class GreeterClient
{

private: GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

private:
  unique_ptr<Greeter::Stub> stub_;



public:
  static GreeterClient *getClient();

  //GreeterClient(std::shared_ptr<Channel> channel)
  //    : stub_(Greeter::NewStub(channel)) {}

  string SayHello(const string &user);

  string Subscribe(vector<string> eventList);

  string Publish(vector<string> eventList);
};

//define as singleton global variable
//extern GreeterClient greeter;

//GreeterClient& intiSocketAddr();

#endif