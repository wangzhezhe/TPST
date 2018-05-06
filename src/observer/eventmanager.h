#ifndef eventmanager_h
#define eventmanager_h

#include "../../deps/rapidjson/include/rapidjson/document.h"
#include "../../deps/rapidjson/include/rapidjson/writer.h"
#include "../../deps/rapidjson/include/rapidjson/stringbuffer.h"

#include <vector>
#include <string>

using namespace rapidjson;

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

void jsonParsingTrigger(Document &d, char *jsonbuffer);
int jsonIfTrigger(Document &d, char *jsonbuffer);

#endif