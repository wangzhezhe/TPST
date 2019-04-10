#ifndef operator_h
#define operator_h

#include "unistd.h"

#include <map>

using namespace std;

//trigure type
const string preTrg("PRETRIGGURE");
const string postTrg("POSTTRIGGURE");




typedef struct Triggure{
    string type;
    //if submeta is empty
    //the task can be started direactly
    string subtopic;
    string subMeta;
    //name of the task it belongs to
    string taskName;
}Triggure;


typedef struct Bundle{
    string taskName;
    string taskTemplates;
    string taskRunningID;
    Triggure pretg;
    Triggure posttg;
}Bundle;

mutex bmapMutex;
map<string,Bundle> bmap;

//from the sub id to the triggure
mutex submapMutex;
map<string,Triggure> submap;


void testPublishTgRegister(GreeterClient *greeter);


void testBundleRegister(GreeterClient *greeter);


#endif