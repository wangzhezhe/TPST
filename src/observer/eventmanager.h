#ifndef eventmanager_h
#define eventmanager_h

#include "../../deps/rapidjson/include/rapidjson/document.h"
#include "../../deps/rapidjson/include/rapidjson/writer.h"
#include "../../deps/rapidjson/include/rapidjson/stringbuffer.h"

#include <vector>
#include <string>
#include <pthread.h>
#include <mutex>
#include <map>

using namespace rapidjson;
using namespace std;

/*
typedef struct EventTriggure
{
    unsigned int eventLen;
    char eventList[100][100];
    char driver[50];
    unsigned int actionLen;
    char actionList[100][100];

} EventTriggure;
*/


typedef struct EventTriggure
{
    string driver;
    string metaData;
    string matchType;
    vector<string> eventSubList;
    vector<string> eventPubList;
    vector<string> actionList;

} EventTriggure;



int jsonIfTriggerorOperator(Document &d, char *jsonbuffer);
void waitthreadFinish();
void jsonParsingTrigger(Document &d);

void initOperator(int jsonNum);
EventTriggure* addNewConfig(string jsonbuffer,string &clientID);

void eventSubscribe(EventTriggure *etrigger, string clientID, string notifyAddr,string eventMsg);

void eventPublish(vector<string> pubList,string metadata);


EventTriggure *fakeaddNewConfig(string driver,
                                vector<string> eventSubList,
                                vector<string> eventPubList,
                                vector<string> actionList,
                                string &clientID);

extern mutex subscribedMutex;
extern int SubscribedClient;
extern vector<string> operatorList;
extern map<string, EventTriggure *> clientIdtoConfig;

extern mutex publishMutex;
extern int publishClient;

#endif