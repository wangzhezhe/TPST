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

#include "../src/server/localserver.h"

#include "../src/utils/getip/getip.h"

#include "../src/utils/file/loaddata.h"

#include "../src/observer/eventmanager.h"

#include "../src/server/pubsub.h"

#include "../deps/rapidjson/include/rapidjson/document.h"
#include "../deps/rapidjson/include/rapidjson/writer.h"
#include "../deps/rapidjson/include/rapidjson/stringbuffer.h"

#define MAX_EVENTS 1024                                /*Max. number of events to process at one go*/
#define LEN_NAME 16                                    /*Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE (sizeof(struct inotify_event))      /*size of one event*/
#define BUF_LEN (MAX_EVENTS * (EVENT_SIZE + LEN_NAME)) /*buffer to store the data of events*/
#define BILLION 1000000000L

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

        string clientID = addNewConfig(jsonbuffer);

        //send subscribe request (unblocked with id) send eventlist and the client id
        if (clientID != "")
        {
            //eventSubscribe(clientIdtoConfig[clientID], clientID);
            localSubscribe(clientID, clientIdtoConfig[clientID]->eventList);
        }

        // don't need to start a new thread because the subscribed api will return immediately
    }

    return;
}

void *tempStartOperator(void *arg)
{
    printf("execute the init operator\n");
    int requiredNum = *((int *)arg);
    printf("required INIT subscription is %d\n", requiredNum);
    string requestEvent = string("INIT");
    int clientsNumber = getSubscribedClientsNumber(requestEvent);

    while (1)
    {
        clientsNumber = getSubscribedClientsNumber(requestEvent);
        if (clientsNumber < requiredNum)
        {
            usleep(1000);
        }
        else
        {
            break;
        }
        printf("current INIT subscribed number is %d\n", clientsNumber);
    }

    printf("Publish INIT event\n");

    vector<string> eventList;
    eventList.push_back(requestEvent);
    struct timespec start, end;
    double diff;

    clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

    localPublish(eventList);

    clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
    diff = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    printf("debug for publish (%s) response time = (%lf) second\n", requestEvent.data(), diff);
    //only for testing
    //startNotifySequence();
    return NULL;
}

//g++ -o notify watchnotifytm.cpp ../lib/file/loaddata.c
//this source code is only avliable if use create/delete the file on the same machine

int main(int argc, char **argv)
{

    int length, i = 0, wd;
    int fd;
    char buffer[BUF_LEN];

    EVENTTYPE eventType;

    if (argc != 4)
    {
        printf("<binary> <watchpath> <number of notify to finish the excution> <INIT subscribed number>\n");
        return 0;
    }

    //start a new thread to run notify server

    //parse the json file and create the clientid and put them in a map

    int requiredInitNum = atoi(argv[3]);

    pthread_t operatorid;

    pthread_create(&operatorid, NULL, &tempStartOperator, (void *)&requiredInitNum);

    // the load operation is defined at pubsubclient

    gothroughFolderRegister(argv[1]);

    int requiredNotifiedNum = atoi(argv[2]);

    //do this in operator
    //initOperator(jsonFileinFolder);

    //waitthreadFinish();

    //if notified number equals to specific number, kill notifyserverid

    while (1)
    {
        if (localNotifiedNum == requiredNotifiedNum)
        {
            break;
        }
        else
        {
            usleep(1000);
        }

        //printf("localNotifiedNum %d requiredNotifiedNum %d\n",localNotifiedNum,requiredNotifiedNum);
    }
    //pthread_join(notifyserverid, (void **)&status);
    //printf("notify server return %d\n", status)
}
