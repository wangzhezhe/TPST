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
    //string clientid;
    //int trigureNum; this value should be maintained in innermap of clienttoSub
    //PubSubReply *reply;
    bool iftrigure;
}pubsubWrapper;



extern map<string, map<int, bool>> strtoEvent;

//client id to pubsubWrapper(value is real element with memory) from clientid to clientStructure

extern mutex clientidtoWrapperMtx;

extern map<string, pubsubWrapper *> clientidtoWrapper;

// to pubsubWrapperid (value is pointer) from subeventstring to set of clientid
extern map<string, set<string>> subtoClient;

// clientid to pubsubEvent (value is pointer) from clientid to map of subscribedEvent
// in the inner map, the integer represent the time that the event have been pushed
extern map<string, map<string, int>> clienttoSub;

void pubsubSubscribe(vector<string> eventList, string clientId);

void pubsubPublish(vector<string> eventList);

void addNewClient(string clientid);

void addNewEvent(string str, int num);

void output();

void ParseEvent(string fullEvent, string & eventMessage, int & num);

#endif