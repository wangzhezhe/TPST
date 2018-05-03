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
#include "pubsub.h"
#include "unistd.h"

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

using namespace std;

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

  Status Subscribe(ServerContext *context, const PubSubRequest *request, PubSubReply *reply)
  {
    //call redis backend to subscribe the message
    std::string prefix("subscribe:");
    //every elemnt could be accessed by specific function
    //reply->set_returnmessage(prefix + request->pubsubmessage());

    printf("call subscribe func, waiting to be notified\n");


    //naive implementation
    //use while loop to check a variable if satisfied value is true 
    //satisfied value is controled by publish function

    //get reply
    
    sleep(5);
    //generate uid on server end
    
    //send message subscribe function

    //delete the clientid in the global map

    return Status::OK;
  }

  Status Publish(ServerContext *context, const PubSubRequest *request, PubSubReply *reply)
  {
    //call redis backend to publish the message
    std::string prefix("publish:");
    reply->set_returnmessage(prefix + request->pubsubmessage());
    return Status::OK;
  }
};

void RunServer()
{
  std::string server_address("0.0.0.0:50051");
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

  RunServer();

  return 0;
}
