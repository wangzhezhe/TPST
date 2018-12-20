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
//#include "../utils/getip/getip.h"
#include "../utils/groupManager/groupManager.h"
#include "../utils/dht/dht.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using workflowserver::Greeter;
using workflowserver::HelloReply;
using workflowserver::HelloRequest;
using workflowserver::PubSubReply;
using workflowserver::PubSubRequest;

using workflowserver::SubNumReply;
using workflowserver::SubNumRequest;

using workflowserver::NotifyReply;
using workflowserver::NotifyRequest;

using workflowserver::RecordSubReply;
using workflowserver::RecordSubRequest;

using workflowserver::RedistributeReply;
using workflowserver::RedistributeRequest;

using workflowserver::UpdateClusterReply;
using workflowserver::UpdateClusterRequest;

using namespace std;

// this should be loaded from the ./ipconfig

//string socketaddr=string("10.211.55.5:50051");
//GreeterClient greeter = (grpc::CreateChannel(
//        socketaddr.data(), grpc::InsecureChannelCredentials()));

/*
GreeterClient *getClientFromAddr(string peerURL)
{
    //if string contain ipv4: delete it

    printf("notify address %s\n", peerURL.data());

    string socketaddr = peerURL;
    //printf("server socket addr %s\n", socketaddr.data());
    //singleton mode
    //GreeterClient *newlient = new GreeterClient(grpc::CreateChannel(
    //    socketaddr.data(), grpc::InsecureChannelCredentials()));

    GreeterClient greeter(grpc::CreateChannel(
        socketaddr.data(), grpc::InsecureChannelCredentials()));
    return &greeter;
}
*/

////////for getip ///////////[deprecated]
//map<string, GreeterClient *> multiClients;
////////for getip ///////////

//vector<string> multiaddr;

///////for group management////////

mutex workerClientsLock;
map<string, map<string, GreeterClient *>> workerClients;
//map<string, map<string, GreeterClient *>> coordinatorClients;

mutex coordinatorClientsLock;
vector<GreeterClient *> coordinatorClients;

int robincount = 0;

void updateCoordinatorClients(string groupDir)
{
    //load the file in coordinator dir
    string coordinatorAddr = groupDir + "/" + gm_coordinatorDir;

    //clean the old clients and update

    vector<string> coordinatorAddrList = loadAddrInDir(coordinatorAddr);

    int size = 0, i = 0;
    size = coordinatorAddrList.size();

    //traverse the addr vector
    //create the client and put it into the map
    for (i = 0; i < size; i++)
    {
        //printf("server process (%d) in cluster (%s) listen addr (%s)\n", i, clusterDir.data(), multiaddr[i].data());
        GreeterClient *greeter = new GreeterClient(grpc::CreateChannel(
            coordinatorAddrList[i].data(), grpc::InsecureChannelCredentials()));
        //coordinatorClients[groupDir][coordinatorAddrList[i]] = greeter;
        coordinatorClientsLock.lock();
        coordinatorClients.push_back(greeter);
        coordinatorClientsLock.unlock();
    }

    return;
}

/*
vector<GreeterClient *> getClientsExcept(string groupDir, string ipaddr)
{
    workerAddrMapLock.lock();
    vector<string> workList = workerAddrMap[groupDir];
    workerAddrMapLock.unlock();

    vector<GreeterClient *> clientsInGroup;

    int size = workList.size();

    for (int i = 0; i < size; i++)
    {
        string addr = workList[i];
        if (ipaddr.compare(addr) != 0)
        {
            GreeterClient *greeter = new GreeterClient(grpc::CreateChannel(
                addr.data(), grpc::InsecureChannelCredentials()));
            clientsInGroup.push_back(greeter);
        }
    }

    return clientsInGroup;
}
*/

//the format of the key is sth like ./multinodeip/cluster0
void updateWorkerClients(string groupDir)
{

    //traverse the workerAddrMap
    //if the key exist, go through
    //if the key not exist, add new
    workerAddrMapLock.lock();
    vector<string> workList = workerAddrMap[groupDir];
    workerAddrMapLock.unlock();

    int size = workList.size();

    for (int i = 0; i < size; i++)
    {
        string addr = workList[i];
        if (workerClients.find(addr) != workerClients.end())
        {
            continue;
        }
        else
        {
            GreeterClient *greeter = new GreeterClient(grpc::CreateChannel(
                addr.data(), grpc::InsecureChannelCredentials()));

            workerClientsLock.lock();
            workerClients[groupDir][addr.data()] = greeter;
            workerClientsLock.unlock();
        }
    }

    //TODO shrink (delete the old one from the map)
}

/*
GreeterClient *randomGetClient(string clientId)
{
    //get last 3 digit of client id idNum

    int clientSize = multiClients.size();
    if (clientSize == 0)
    {
        printf("failed to get init client\n");
        return NULL;
    }
    //get the index of client idNum%clientSize
    string tempstr = clientId.substr(clientId.size() - 3);
    int tempData = atoi(tempstr.data());
    //printf("init index is %d size of clientSize %d\n", tempData, clientSize);
    int tmpindex = tempData % clientSize;
    string addr = multiaddr[tmpindex];
    //printf("get server socket addr %s\n", addr.data());
    return multiClients[addr];
}
*/

/*
GreeterClient *roundrobinGetClient()
{
    //get last 3 digit of client id idNum

    int clientSize = multiClients.size();
    if (clientSize == 0)
    {
        printf("failed to get init client\n");
        return NULL;
    }

    //get the tmpindex by round robin way
    int tmpindex;
    robincount++;

    //avoid the overflow of large number
    robincount = robincount % clientSize;
    tmpindex = robincount;
    string addr = multiaddr[tmpindex];
    //printf("index %d get server socket addr %s\n", tmpindex, addr.data());
    return multiClients[addr];
}
*/

/*deprecated
GreeterClient *GreeterClient::getClient()
{

    string ip;
    string port;
    //string ipconfigfilepath = string("/home/parallels/Documents/cworkspace/observerchain/src/server/ipconfig");
    //workflow server and eventNotify should run at same directory, ipconfig will be outputed into this dir
    string ipconfigfilepath = string("./ipconfig");
    int r = loadIPPort(ipconfigfilepath, ip, port);
    if (r == 1)
    {
        printf("failed to open the ip port config file (%s)\n", ipconfigfilepath.data());
        return NULL;
    }
    string socketaddr = ip + ":" + port;
    printf("server socket addr %s\n", socketaddr.data());
    //singleton mode
    static GreeterClient *singleClient = new GreeterClient(grpc::CreateChannel(
        socketaddr.data(), grpc::InsecureChannelCredentials()));
    return singleClient;
}
*/

string GreeterClient::NotifyBack(string clientId, string metadata)
{

    NotifyRequest request;
    request.set_clientid(clientId);
    request.set_metadata(metadata);

    // Container for the data we expect from the server.
    NotifyReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    //printf("send rpc request for %s\n",clientId.data());
    Status status = stub_->Notify(&context, request, &reply);

    // Act upon its status.
    if (status.ok())
    {
        return reply.returnmessage();
    }
    else
    {
        cout << status.error_code() << ": " << status.error_message()
             << endl;
        return "RPC failed";
    }
}

// Assembles the client's payload, sends it and presents the response back
// from the server.
string GreeterClient::SayHello(const string &user)
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
        cout << status.error_code() << ": " << status.error_message()
             << endl;
        return "RPC failed";
    }
}

string GreeterClient::Subscribe(vector<string> eventSubList, string clientID, string notifyAddr, string source)
{

    // Container for the data we expect from the server.
    //printf("debug sub part1\n");
    PubSubRequest request;
    PubSubReply reply;
    int size = eventSubList.size();
    int i = 0;
    for (i = 0; i < size; i++)
    {
        //attention the use here, the request could be transfered into a specific type with specific function
        request.add_pubsubmessage(eventSubList[i]);
        //printf("add %s into request \n",eventList[i].data());
    }
    //printf("debug sub part2\n");
    request.set_clientid(clientID);
    request.set_metadata(notifyAddr);
    request.set_source(source);

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->Subscribe(&context, request, &reply);
    //printf("debug sub part3\n");
    // Act upon its status.
    if (status.ok())
    {
        return reply.returnmessage();
    }
    else
    {
        cout << status.error_code() << ": " << status.error_message()
             << endl;
        return "RPC failed";
    }
}

string GreeterClient::Publish(vector<string> eventList, string source, string metadata)
{
    // Container for the data we expect from the server.
    PubSubRequest request;
    PubSubReply reply;
    int size = eventList.size();
    int i = 0;
    for (i = 0; i < size; i++)
    {
        //attention the use here, the request could be transfered into a specific type with specific function
        request.add_pubsubmessage(eventList[i]);
    }

    request.set_source(source);

    request.set_metadata(metadata);

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->Publish(&context, request, &reply);

    // Act upon its status.
    if (status.ok())
    {
        return reply.returnmessage();
    }
    else
    {
        cout << status.error_code() << ": " << status.error_message()
             << endl;
        return "RPC failed";
    }
    return reply.returnmessage();
}

int GreeterClient::GetSubscribedNumber(vector<string> eventList)
{
    SubNumRequest request;

    // Data we are sending to the server.
    SubNumReply reply;

    int i = 0;
    int size = eventList.size();

    for (i = 0; i < size; i++)
    {
        //attention the use here, the request could be transfered into a specific type with specific function
        request.add_subevent(eventList[i]);
    }

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->GetSubscribedNumber(&context, request, &reply);
    // Act upon its status.
    if (status.ok())
    {

        int replyNum = reply.clientnumber();
        return replyNum;
    }
    else
    {
        cout << status.error_code() << ": " << status.error_message()
             << endl;
        return -1;
    }
}

string GreeterClient::RecordSub(string subevent, string serverAddr, int subNum)
{
    //send info(subNum) to the coordinator in current group
    RecordSubRequest request;

    request.set_serveraddr(serverAddr);
    request.set_subevent(subevent);
    request.set_subnum(subNum);

    // Data we are sending to the server.
    RecordSubReply reply;

    ClientContext context;

    // The actual RPC.
    Status status = stub_->RecordSub(&context, request, &reply);

    if (status.ok())
    {
        return reply.returnmessage();
    }
    else
    {
        cout << status.error_code() << ": " << status.error_message()
             << endl;
        return "RPC failed";
    }
}

string GreeterClient::RedistributeSub(string subevent, string srcAddr, string dstAddr, int diffNum)
{
    RedistributeRequest request;

    // The actual RPC.
    printf("send RedistributeSub to %s\n", dstAddr.data());

    request.set_subevent(subevent);
    request.set_srcaddr(srcAddr);
    request.set_destaddr(dstAddr);
    request.set_diff(diffNum);

    // Data we are sending to the server.
    RedistributeReply reply;

    ClientContext context;

    Status status = stub_->RedistributeSub(&context, request, &reply);

    printf("get status from RedistributeSub\n");

    if (status.ok())
    {
        return reply.returnmessage();
    }
    else
    {
        cout << status.error_code() << ": " << status.error_message()
             << endl;
        return "RPC failed";
    }
}

string GreeterClient::UpdateCluster(string newClusterDir)
{
    UpdateClusterRequest request;

    request.set_cluster(newClusterDir);

    // Data we are sending to the server.
    UpdateClusterReply reply;

    ClientContext context;

    Status status = stub_->UpdateCluster(&context, request, &reply);

    if (status.ok())
    {
        return reply.returnmessage();
    }
    else
    {
        cout << status.error_code() << ": " << status.error_message()
             << endl;
        return "RPC failed";
    }
}

/*
void initMultiClientsByClusterDir(string clusterDir)
{
    //get addr vector from getip
    multiaddr = loadMultiNodeIPPortByClusterDir(clusterDir);
    int size = 0, i = 0;
    size = multiaddr.size();

    //traverse the addr vector
    //create the client and put it into the map
    for (i = 0; i < size; i++)
    {
        //printf("server process (%d) in cluster (%s) listen addr (%s)\n", i, clusterDir.data(), multiaddr[i].data());
        GreeterClient *greeter = new GreeterClient(grpc::CreateChannel(
            multiaddr[i].data(), grpc::InsecureChannelCredentials()));
        multiClients[multiaddr[i]] = greeter;
    }

    return;
}
*/

//call this at init step
void initClients(string clusterDir)
{

    updateWorkerAddrMap(clusterDir);

    //printf("update worker map ok\n");

    updateWorkerClients(clusterDir);
    updateCoordinatorClients(clusterDir);

    //printf("init clients for clusterDir %s\n", clusterDir.data());
}

//fake use eventId for testing
GreeterClient *getClientFromEvent(string eventString)
{

    //get cluster dir from the hash function
    string clusterDir = getClusterDirFromEventMsg(eventString);

    //printf("current clusterDir %s\n", clusterDir.data());

    //get client from the addr map

    //get number i element from workerClients[clusterDir]

    //debug print workerClients

    if (workerClients.find(clusterDir) == workerClients.end())
    {
        initClients(clusterDir);
    }

    int serverNum = workerClients[clusterDir].size();

    if (serverNum == 0)
    {
        printf("failed to get server number (value is 0) for %s\n", clusterDir.data());
        return NULL;
    }

    srand(time(0));
    int serverId = getServerIdFromAddr(serverNum);
    //printf("[publishclient] server id is %d\n", serverId);

    workerClientsLock.lock();
    map<string, GreeterClient *> clients = workerClients[clusterDir];
    workerClientsLock.unlock();

    int counter = 0;
    for (map<string, GreeterClient *>::iterator it = clients.begin(); it != clients.end(); ++it)
    {

        if (counter == serverId)
        {
            return clients[it->first];
        }
        counter++;
    }

    return NULL;
}