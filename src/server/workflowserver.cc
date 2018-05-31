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

    //every elemnt could be accessed by specific function
    //reply->set_returnmessage(prefix + request->pubsubmessage());

    //parse the request events
    int size = request->pubsubmessage_size();
    printf("server get (%d) subscribed events\n", size);
    int i = 0;
    vector<string> eventList;
    string eventStr;
    for (i = 0; i < size; i++)
    {
      eventStr = request->pubsubmessage(i);
      printf("get events (%s)\n", eventStr.data());
      eventList.push_back(eventStr);
      //default number is 1
      int trinum = 1;
      string eventMessage;
      ParseEvent(eventStr, eventMessage, trinum);
      printf("after parsing %s %d\n",eventMessage.data(),trinum);
      addNewEvent(eventMessage, trinum);
    }

    pubsubSubscribe(eventList, clientId);

    printf("clientid (%s) call subscribe func, waiting to be notified\n", clientId.data());

    //request should be a event list

    //put event list into vector and call subscribe function

    //naive implementation
    //use while loop to check a variable if satisfied value is true
    //satisfied value is controled by publish function

    //get reply

    while (1)
    {
      if (clientidtoWrapper[clientId]->iftrigure == true)
      {
        break;
      }

      //set timestep?
      int timestep =1;
      //sleep(timestep);
      usleep(1 * 50);
    }
    //generate uid on server end

    //send message subscribe function

    //delete the clientid in the global map
    reply->set_returnmessage("TRIGGERED");
    return Status::OK;
  }

  Status Publish(ServerContext *context, const PubSubRequest *request, PubSubReply *reply)
  {

    //parse the request events
    int size = request->pubsubmessage_size();
    printf("server get (%d) published events\n", size);
    int i = 0;
    vector<string> eventList;
    string eventStr;
    for (i = 0; i < size; i++)
    {
      eventStr = request->pubsubmessage(i);
      printf("server publish event (%s)\n", eventStr.data());
      eventList.push_back(eventStr);
    }
    //publish
    pubsubPublish(eventList);
    reply->set_returnmessage("OK");
    return Status::OK;
  }
};

void RunServer()
{

  string serverPort = string("50051");
  string ip;
  printf("record ip\n");
  recordIPPort(ip,serverPort);
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

  RunServer();
  return 0;
}
