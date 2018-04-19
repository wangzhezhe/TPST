#ifndef eventmanager_h
#define eventmanager_h

#include "../../deps/rapidjson/include/rapidjson/document.h"
#include "../../deps/rapidjson/include/rapidjson/writer.h"
#include "../../deps/rapidjson/include/rapidjson/stringbuffer.h"

using namespace rapidjson;

typedef struct EventTriggure
{
    unsigned int eventLen;
    char eventList[100][100];
    char driver[50];
    unsigned int actionLen;
    char actionList[100][100];

} EventTriggure;

void jsonParsing(Document &d, char *jsonbuffer);

#endif