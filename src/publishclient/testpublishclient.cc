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
#include <vector>

#include <unistd.h>
#include <pthread.h>

#include <grpc++/grpc++.h>

#include "workflowserver.grpc.pb.h"
#include "pubsubclient.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using workflowserver::Greeter;
using workflowserver::HelloReply;
using workflowserver::HelloRequest;
using workflowserver::PubSubReply;
using workflowserver::PubSubRequest;
using namespace std;

void *PublishOperation(void *ptr)
{
    sleep(5);
    printf("new thread push event\n");
    vector<string> publisheventList;
    publisheventList.push_back("event1");
    GreeterClient *greeter = GreeterClient::getClient();
    if (greeter == NULL)
    {
        printf("failed to get initialised greeter\n");
        return NULL;
    }
    string reply = greeter->Publish(publisheventList);
    cout << "Publish return value: " << reply << endl;
}

int main(int argc, char **argv)
{
    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint (in this case,
    // localhost at port 50051). We indicate that the channel isn't authenticated
    // (use of InsecureChannelCredentials()).
    /*
    string user("world");

    //get greeter
    GreeterClient *greeter = GreeterClient::getClient();
    if (greeter == NULL)
    {
        printf("failed to get initialised greeter\n");
        return 0;
    }
    string reply = greeter->SayHello(user);
    cout << "Greeter received: " << reply << endl;

    pthread_t id;
    pthread_create(&id, NULL, PublishOperation, NULL);

    vector<string> subeventList;
    subeventList.push_back("event1");
    //eventList.push_back("event2");

    
    //RepeatedPtrField<string> tpf;
    //eventList.insert("event1");
    //int size=eventList.size();
    //int i=0;
    //for(i=0;i<size;i++){
    //    tpf->set_pubsubmessage(i,eventList[i]);
    //}
    
    //intiSocketAddr();
    string clientid("testid");
    reply = greeter->Subscribe(subeventList,clientid);
    cout << "Subscribe return value: " << reply << endl;
    */
    //start a new thread to do the push operation

    initMultiClients();

    return 0;
}
