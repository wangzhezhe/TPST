#ifndef eventmanager_h
#define eventmanager_h

#include "../../deps/rapidjson/include/rapidjson/document.h"
#include "../../deps/rapidjson/include/rapidjson/writer.h"
#include "../../deps/rapidjson/include/rapidjson/stringbuffer.h"

#include <vector>
#include <string>
#include <pthread.h>
#include <mutex>

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
    unsigned int eventLen;
    vector<string> eventList;
    string driver;
    unsigned int actionLen;
    vector<string> actionList;

} EventTriggure;



int jsonIfTriggerorOperator(Document &d, char *jsonbuffer);
void waitthreadFinish();
void jsonParsingTrigger(Document &d);



extern mutex subscribedMutex;
extern int SubscribedClient;

#endif