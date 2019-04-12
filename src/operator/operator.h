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



void ActionByOperator(string clientID, string metadata);

void testBundleRegister(GreeterClient *greeter,
                        string notifyaddr,
                        string taskName,
                        string taskTemplates,
                        string preTopic,
                        string preMeta,
                        string postTopic,
                        string postMeta);


#endif