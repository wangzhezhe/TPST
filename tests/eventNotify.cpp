#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "sys/types.h"
#include "sys/inotify.h"
#include "limits.h"
#include "unistd.h"
#include "time.h"

#include <dlfcn.h>
#include <vector>
#include <pthread.h>
#include <vector>
#include <string>
#include <algorithm>
#include <uuid/uuid.h>

//#include "../observer/taskmanager.h"
//#include "../eventstore/eventStore.h"
//#include "../storage/memcache.h"
#include "../src/server/notifyserver.h"

#include "../src/utils/getip/getip.h"

#include "../src/utils/file/loaddata.h"

#include "../src/observer/eventmanager.h"
#include "../src/publishclient/pubsubclient.h"

#include "../deps/rapidjson/include/rapidjson/document.h"
#include "../deps/rapidjson/include/rapidjson/writer.h"
#include "../deps/rapidjson/include/rapidjson/stringbuffer.h"

#define MAX_EVENTS 1024                                /*Max. number of events to process at one go*/
#define LEN_NAME 16                                    /*Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE (sizeof(struct inotify_event))      /*size of one event*/
#define BUF_LEN (MAX_EVENTS * (EVENT_SIZE + LEN_NAME)) /*buffer to store the data of events*/

using namespace rapidjson;
using namespace std;

enum EVENTTYPE
{
    created,
    modified,
    deleted
};

char projectPath[100] = "/home1/zw241/observerchain/tests";
//char tmDir[50] = "TaskManagerFiles";
//char projectPath[100] = "/home/parallels/Documents/cworkspace/observerchain/tests";
//char tmDir[50] = "TrigureFiles";
char tmDir[50] = "TrigureFiles";

void *tempStartOperator(void *arg)
{
    printf("execute the init operator\n");
    int requiredNum = *((int *)arg);
    printf("required INIT subscription is %d\n", requiredNum);
    string INITEvent = string("INIT");

    char idstr[50];
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, idstr);

    string clientID(idstr);
    GreeterClient *greeter = roundrobinGetClient(clientID);

    if (greeter == NULL)
    {
        printf("failed to get greeter for event subscribe\n");
        return NULL;
    }

    //get number of init event
    int replyNum;
    while (1)
    {
        replyNum = greeter->GetSubscribedNumber(INITEvent);
        //printf("there are %d clients subscribe %s event\n", replyNum, queryEvent.data());
        if (replyNum < requiredNum)
        {
            usleep(500);
        }
        else
        {
            break;
        }
    }

    //publish INIT event
    vector<string> eventList;
    eventList.push_back(INITEvent);
    string reply = greeter->Publish(eventList,"CLIENT");
    printf("publish INIT event return (%s)\n", reply.data());

    return NULL;
}

//go through the Trigurefile folder and register the .json file with type=trigure into the system
void gothroughFolderRegister(const char *watchdir)
{

    vector<string> fileList;
    fileList = scanFolder(watchdir);

    int count = fileList.size();
    char taskPath[100];
    for (int i = 0; i < count; i++)
    {

        // if it is not json file
        if (strstr(fileList[i].data(), ".json") == NULL)
        {
            continue;
        }
        memset(taskPath, sizeof(taskPath), 0);

        char cwd[FILENAME_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL)
        {
            //Current working dir: /home1/zw241/observerchain/tests/performance
            //printf("Current working dir: %s\n", cwd);
            printf("failed to get current dir\n");
            return;
        }

        //get current dir

        //snprintf(taskPath, sizeof taskPath, "%s/%s/%s", projectPath, tmDir, fileList[i].data());
        //snprintf(taskPath, sizeof taskPath, "%s/%s/%s", projectPath, dir, fileList[i].data());

        //delete . and / in watchdir path
        string watchdirstr = string(watchdir);
        watchdirstr.erase(std::remove(watchdirstr.begin(), watchdirstr.end(), '.'), watchdirstr.end());
        watchdirstr.erase(std::remove(watchdirstr.begin(), watchdirstr.end(), '/'), watchdirstr.end());

        snprintf(taskPath, sizeof taskPath, "%s/%s/%s", cwd, watchdirstr.data(), fileList[i].data());

        string jsonbuffer = loadFile(taskPath);
        //printf("dir path original %s after deletion %s\n", watchdir, watchdirstr.data());
        printf("taskPath %s\n", taskPath);

        //create new client id and new event triggure

        string clientID;

        EventTriggure *etrigger = addNewConfig(jsonbuffer, clientID);

        //send subscribe request (unblocked with id) send eventlist and the client id

        if (clientID != "")
        {
            eventSubscribe(etrigger, clientID);
        }
    }

    return;
}

//g++ -o notify watchnotifytm.cpp ../lib/file/loaddata.c
//this source code is only avliable if use create/delete the file on the same machine

int main(int argc, char **argv)
{

    int length, i = 0, wd;
    int fd;
    char buffer[BUF_LEN];

    EVENTTYPE eventType;

    if (argc != 5)
    {
        printf("<binary> <watchpath> <required number of notification> <notifyserver interfaces> <required INIT Number>\n");
        return 0;
    }

    //start a new thread to run notify server

    //parse the json file and create the clientid and put them in a map

    pthread_t notifyserverid;
    int status;
    pthread_create(&notifyserverid, NULL, &RunNotifyServer, NULL);
    printf("waiting the termination of threads id %ld\n", notifyserverid);

    //traverse the map and send the subscribe request

    // write ip port of current nodes into config files
    // the load operation is defined at pubsubclient
    initMultiClients();

    gothroughFolderRegister(argv[1]);

    int requiredNotifiedNum = atoi(argv[2]);

    INTERFACE = string(argv[3]);

    printf("notify server listen to interface %s\n", INTERFACE.data());

    int requiredInitNum = atoi(argv[4]);

    pthread_t operatorid;

    pthread_create(&operatorid, NULL, &tempStartOperator, (void *)&requiredInitNum);

    while (1)
    {
        if (NotifiedNum == requiredNotifiedNum)
        {
            break;
        }
        else
        {
            usleep(1000);
        }
    }
}