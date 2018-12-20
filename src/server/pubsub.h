#ifndef pubsub_h
#define pubsub_h

#include <grpc++/grpc++.h>
#include <string>
#include <map>

#ifdef BAZEL_BUILD
#else
#include "workflowserver.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using workflowserver::PubSubReply;
using workflowserver::PubSubRequest;
using namespace std;



typedef struct pubsubEvent{
    //static triggure number, when publish number equal or larger than this value, event is supposed to be triggured
    int trigureNum;
    string event;
}pubsubEvent;

typedef struct pubsubWrapper{
    mutex pswmtx;
    string peerURL;
    string clientID;
    //one clientid, one metadata
    string metadata;
    //int trigureNum; this value should be maintained in innermap of clienttoSub
    //PubSubReply *reply;
    bool iftrigure;
    map<string, int> publishedEvent;
    map<string, set<int>> requiredeventMap; //transfer eventList into this format
}pubsubWrapper;


typedef struct SimplepubsubWrapper
{
    vector <string> eventList;
    string peerURL;
    string clientID;
    string metadata;
} SimplepubsubWrapper;


extern mutex subtoClientMtx;


extern map<string, set<int>> strtoEvent;

extern map<string, map<string, pubsubWrapper *> > subtoClient;

extern mutex getIndexMtx;
extern map<string, set<string>> getIndexMap;

void pubsubSubscribe(vector<string> eventList, string clientId, string notifyAddr);

void pubsubPublish(vector<string> eventList, string metadata);

//void addNewClientLocal(string clientid, vector<string> eventList);

//void addNewEvent(string str, int num);

void output();

void ParseEvent(string fullEvent, string & eventMessage, int & num);

//void deleteClient(string subevent, string clientid);

//void deleteClientFromSTC(string clientid, string substr);

//bool checkIfTriggure(pubsubWrapper *psw);

bool checkIfTriggure(pubsubWrapper *psw, string &satisfiedStr);

int getSubscribedClientsNumber(vector<string> subEventList);

void deletePubEvent(pubsubWrapper *psw);

void eventUnSubscribe(string event,string clientId);

//void *checkNotify(void *arguments);

SimplepubsubWrapper *getSimplepubsubWrapper(pubsubWrapper *psw);

void outputsubtoClient();

#endif