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
  public:
    GreeterClient(std::shared_ptr<Channel> channel)
        : stub_(Greeter::NewStub(channel)) {}

    // Assembles the client's payload, sends it and presents the response back
    // from the server.
    string SayHello(const string &user);

    string Subscribe(vector<string> eventList);

    string Publish(vector<string> eventList);

  private:
    unique_ptr<Greeter::Stub> stub_;
};

//define as singleton global variable
extern GreeterClient greeter;

#endif