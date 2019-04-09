
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

#include <grpcpp/grpcpp.h>

#include "unistd.h"

#include "metaserver.grpc.pb.h"
#include "../utils/groupManager/groupManager.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using metaserver::Meta;

using metaserver::HelloReply;
using metaserver::HelloRequest;

using metaserver::PutReply;
using metaserver::PutRequest;

using metaserver::GetReply;
using metaserver::GetRequest;

using metaserver::TimeReply;
using metaserver::TimeRequest;

const string metaserverDir = "Metaserver";

class MetaClient
{
  public:
    MetaClient(std::shared_ptr<Channel> channel)
        : stub_(Meta::NewStub(channel)) {}

    // Assembles the client's payload, sends it and presents the response back
    // from the server.
    std::string SayHello(const std::string &user)
    {
        // Data we are sending to the server.
        HelloRequest request;
        request.set_name(user);

        // Container for the data we expect from the server.
        HelloReply reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        Status status = stub_->SayHello(&context, request, &reply);

        // Act upon its status.
        if (status.ok())
        {
            return reply.message();
        }
        else
        {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return "RPC failed";
        }
    }

    std::string Recordtime(const std::string &key)
    {
        // Data we are sending to the server.
        TimeRequest request;
        request.set_key(key);

        // Container for the data we expect from the server.
        TimeReply reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        Status status = stub_->Recordtime(&context, request, &reply);

        // Act upon its status.
        if (status.ok())
        {
            return reply.message();
        }
        else
        {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return "RPC failed";
        }
    }

    std::string Getmeta(const std::string &key)
    {
        // Data we are sending to the server.
        GetRequest request;
        request.set_key(key);

        // Container for the data we expect from the server.
        GetReply reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        Status status = stub_->Getmeta(&context, request, &reply);

        // Act upon its status.
        if (status.ok())
        {
            return reply.message();
        }
        else
        {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return "RPC failed";
        }
    }
    
    std::string Putmeta(const std::string &key,const std::string &value)
    {
        // Data we are sending to the server.
        PutRequest request;
        request.set_key(key);
        request.set_value(value);

        // Container for the data we expect from the server.
        PutReply reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        Status status = stub_->Putmeta(&context, request, &reply);

        // Act upon its status.
        if (status.ok())
        {
            return reply.message();
        }
        else
        {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return "RPC failed";
        }
    }

  private:
    std::unique_ptr<Meta::Stub> stub_;
};

string getAddr()
{

    vector<string> addrList = loadAddrInDir(metaserverDir);
    if (addrList.size() > 0)
    {
        return addrList[0];
    }

    return "";
}

int main(int argc, char **argv)
{
    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint (in this case,
    // localhost at port 50051). We indicate that the channel isn't authenticated
    // (use of InsecureChannelCredentials()).

    string serverAddr = getAddr();

    if (serverAddr == "")
    {

        printf("server addr is 0\n");
        return 0;
    }

    MetaClient metaclient(grpc::CreateChannel(
        serverAddr, grpc::InsecureChannelCredentials()));
    std::string user("world");
    std::string reply = metaclient.SayHello(user);
    std::cout << "Greeter received: " << reply << std::endl;

    std::string key("world");
    reply = metaclient.Recordtime(key);
    std::cout << "Timer received: " << reply << std::endl;

    sleep(1);

    reply = metaclient.Recordtime(key);
    std::cout << "Timer received: " << reply << std::endl;

    key = "pattern1";
    string meta1("meta1");
    string meta2("meta2");
    string meta3("meta3");

    reply = metaclient.Getmeta(key);
    std::cout << "Get pattern1 recieve: " << reply << std::endl;

    reply = metaclient.Putmeta(key, meta1);
    std::cout << "Put meta1 recieve: " << reply << std::endl;
    reply = metaclient.Putmeta(key, meta2);
    std::cout << "Put meta2 recieve: " << reply << std::endl;
    reply = metaclient.Putmeta(key, meta3);
    std::cout << "Put meta3 recieve: " << reply << std::endl;

    reply = metaclient.Getmeta(key);
    std::cout << "Get pattern1 recieve: " << reply << std::endl;
    reply = metaclient.Getmeta(key);
    std::cout << "Get pattern1 recieve: " << reply << std::endl;
    reply = metaclient.Getmeta(key);
    std::cout << "Get pattern1 recieve: " << reply << std::endl;

    reply = metaclient.Getmeta(key);
    std::cout << "Get pattern1 recieve: " << reply << std::endl;

    return 0;
}
