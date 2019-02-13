// this file is used for scalable test
// this is the prototype of the function of operator
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
#include <mpi.h>

//#include "../observer/taskmanager.h"
//#include "../eventstore/eventStore.h"
//#include "../storage/memcache.h"
#include "../src/server/notifyserver.h"

//#include "../src/utils/getip/getip.h"

#include "../src/utils/groupManager/groupManager.h"

#include "../src/utils/file/loaddata.h"

#include "../src/observer/eventmanager.h"
#include "../src/publishclient/pubsubclient.h"

#include "../deps/rapidjson/include/rapidjson/document.h"
#include "../deps/rapidjson/include/rapidjson/writer.h"
#include "../deps/rapidjson/include/rapidjson/stringbuffer.h"

#include "../deps/spdlog/spdlog.h"

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

/*
void *tempStartOperator(void *arg)
{
    printf("execute the init operator\n");
    int requiredNum = *((int *)arg);
    printf("required INIT subscription is %d\n", requiredNum);
    string INITEvent = string("INIT");
    vector<string> InitList;
    InitList.push_back(INITEvent);
    char idstr[50];
    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, idstr);

    string clientID(idstr);
    GreeterClient *greeter = roundrobinGetClient();

    if (greeter == NULL)
    {
        printf("failed to get greeter for event subscribe\n");
        return NULL;
    }

    //get number of init event
    int replyNum;
    while (1)
    {
        replyNum = greeter->GetSubscribedNumber(InitList);
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
    string metadata = "test init meta";
    string reply = greeter->Publish(eventList, sourceClient, metadata);
    printf("publish INIT event return (%s)\n", reply.data());

    return NULL;
}
*/
//for test using, send multipublish request after subscribe
void fakePublishTest(int pubSize)
{
    int i = 0;
    //srand(time(0));
    //string metadata = to_string(COMPONENTID) + "metadataTest" + to_string(i);
    //string metadata = to_string(COMPONENTID) + "metadataTest";
    vector<string> pubeventList;
    spdlog::debug("id {} the pubsize is {}", gm_rank, pubSize);
    for (i = 0; i < pubSize; i++)
    {
        //int index = (rand() % (pubSize - 0 + 1));

        string metadata = to_string(gm_rank) + "metadataTest" + to_string(i);

        string fakePub = to_string(gm_rank) + "fakeSub_" + to_string(i);

        pubeventList.clear();

        pubeventList.push_back(fakePub);

        spdlog::debug("id {} publish times before eventPublish {}", gm_rank, publishClient);
        eventPublish(pubeventList, metadata);
        spdlog::debug("id {} publish times after eventPublish {}", gm_rank, publishClient);

        publishMutex.lock();
        publishClient = publishClient + 1;
        publishMutex.unlock();
        //usleep(3000);
    }
    //publish in collective pattern
    //eventPublish(pubeventList, metadata);

    return;
}

void oneSubMultiplePub(int subSize, string notifyAddr)
{
    return;
}

void onePubMultipleSameSub(int subSize, string notifyAddr)
{

    //sub same event multiple times
    string testEvent = "fakeOnePubMultipleSub_0";
    //string metadata = "metadataTestPublish";

    vector<string> pubeventList;
    pubeventList.push_back(testEvent);

    printf("debug onePubMultipleSub subsize is %d\n", subSize);

    int i = 0;
    for (i = 0; i < subSize; i++)
    {
        vector<string> subeventList;
        vector<string> actionList;

        string fakeaction = "fakeaction" + to_string(i);

        subeventList.push_back(testEvent);
        //pubeventList.push_back(redundantPushEvent);
        actionList.push_back(fakeaction);

        string clientID;

        string driver = "local";

        EventTriggure *etrigger = fakeaddNewConfig(driver, subeventList, pubeventList, actionList, clientID);

        if (clientID != "")
        {
            eventSubscribe(etrigger, clientID, notifyAddr, testEvent);
        }
    }

    printf("wait for group redistribution sleep 5s\n");

    sleep(15);

    //only the first client send the publish request

    //event publish
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start); /* mark the end time */
    printf("start id %d start pub time = (%lld.%.9ld)\n", gm_rank, (long long)start.tv_sec, start.tv_nsec);

    if (gm_rank == 0)
    {

        string metadata = "metadataTest";
        eventPublish(pubeventList, metadata);
    }

    return;
}

void multipleSub(int subSize, string notifyAddr)
{

    //subscription with multiple event keys
    //the subSize here is the number for the events key that are subscribed

    vector<string> subeventList;
    vector<string> actionList;
    vector<string> pubeventList;
    string driver = "local";

    for (int i = 0; i < subSize; i++)
    {

        //every component pub sub different event
        string fakeSub = to_string(gm_rank) + "fakeSub_" + to_string(i);
        string fakeaction = "fakeaction" + to_string(i);

        subeventList.push_back(fakeSub);

        actionList.push_back(fakeaction);
    }

    string clientID;

    EventTriggure *etrigger = fakeaddNewConfig(driver, subeventList, pubeventList, actionList, clientID);

    printf("sub size is %d debugtest\n", subSize);

    if (clientID != "")
    {
        //use first event as the hash value
        eventSubscribe(etrigger, clientID, notifyAddr, subeventList[0]);
    }

    //sleep some time

    sleep(5);

    //event publish
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start); /* mark the end time */
    printf("start id %d start pub time = (%lld.%.9ld)\n", gm_rank, (long long)start.tv_sec, start.tv_nsec);
    //publish events one by one
    int j = 0;
    for (j = 0; j < subSize; j++)
    {
        //printf("debug1 %d\n", j);
        string metadata = "metadataTest";
        vector<string> pubeventList;
        pubeventList.push_back(subeventList[j]);
        eventPublish(pubeventList, metadata);
        //printf("debug2 subsize %d j %d publishedEvent %s\n", subSize, j, pubeventList[0].data());
    }

    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
    printf("id %d finish pub time = (%lld.%.9ld)\n", gm_rank, (long long)end.tv_sec, end.tv_nsec);
}

void fakegothroughFolderRegister(int subSize, string notifyAddr)
{
    //the json buffer shouled be load from memory
    /*
{
    "type": "TOPICTRIGGER",
    "eventSubList": ["INIT"],
    "eventPubListBeforAction": ["SIMSTART"],
    "eventPubListAfterAction": ["SIMFINISH"],
    "driver": "local",
    "actionList": [
       "/bin/bash ./app/simulate.sh "
     ]
}

{
    "type": "DATATRIGGER",
    "eventDescription": "/path/to/the/customized/script",
    "eventPubListBeforAction": ["SIMSTART"],
    "eventPubListAfterAction": ["SIMFINISH"],
    "driver": "local",
    "actionList": [
       "/bin/bash ./app/simulate.sh"
     ]
}


    */

    //TODO using distributed lock to get the id from the shared file system
    int i = 0;

    string driver = "local";
    //for test using

    printf("call fakegothroughFolderRegister, subsize is %d\n", subSize);

    struct timespec startsub;
    struct timespec aftersub;

    clock_gettime(CLOCK_REALTIME, &startsub); /* mark the end time */

    for (i = 0; i < subSize; i++)
    {
        vector<string> pubeventList;
        vector<string> subeventList;
        vector<string> actionList;

        //every component pub sub different event
        string fakeSub = to_string(gm_rank) + "fakeSub_" + to_string(i);
        string fakeaction = "fakeaction" + to_string(i);

        subeventList.push_back(fakeSub);
        //pubeventList.push_back(redundantPushEvent);
        actionList.push_back(fakeaction);

        string clientID;

        EventTriggure *etrigger = fakeaddNewConfig(driver, subeventList, pubeventList, actionList, clientID);

        //printf("debugtest sub id %d\n",i);

        if (clientID != "")
        {
            eventSubscribe(etrigger, clientID, notifyAddr, fakeSub);
        }

        // printf("sub id %d ok\n",i);
    }
    clock_gettime(CLOCK_REALTIME, &aftersub); /* mark the end time */
    double subdiff = (aftersub.tv_sec - startsub.tv_sec) * 1.0 + (aftersub.tv_nsec - startsub.tv_nsec) * 1.0 / BILLION;
    spdlog::info("id {} total subtime {}", gm_rank, subdiff);

    //sleep some time then publish
    sleep(5);

    struct timespec startpub, afterpub;

    clock_gettime(CLOCK_REALTIME, &startpub); /* mark the end time */

    printf("start id %d start pub time = (%lld.%.9ld)\n", gm_rank, (long long)startpub.tv_sec, startpub.tv_nsec);

    fakePublishTest(subSize);

    clock_gettime(CLOCK_REALTIME, &afterpub); /* mark the end time */
    double pubdiff = (afterpub.tv_sec - startpub.tv_sec) * 1.0 + (afterpub.tv_nsec - startpub.tv_nsec) * 1.0 / BILLION;
    spdlog::info("id {} total pubtime {}", gm_rank, pubdiff);

    return;
}

/*
//go through the Trigurefile folder and register the .json file with type=trigure into the system
void gothroughFolderRegister(const char *watchdir, string notifyAddr)
{

    vector<string> fileList;
    fileList = scanFolder(watchdir);
    int count = fileList.size();
    char taskPath[1000];

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

        snprintf(taskPath, sizeof(taskPath), "%s/%s/%s", cwd, watchdirstr.data(), fileList[i].data());

        string jsonbuffer = loadFile(taskPath);
        //printf("dir path original %s after deletion %s\n", watchdir, watchdirstr.data());

        //create new client id and new event triggure

        string clientID;

        EventTriggure *etrigger = addNewConfig(jsonbuffer, clientID);

        //send subscribe request (unblocked with id) send eventlist and the client id

        if (clientID != "")
        {
            eventSubscribe(etrigger, clientID, notifyAddr);
        }
    }

    return;
}

*/

//g++ -o notify watchnotifytm.cpp ../lib/file/loaddata.c
//this source code is only avliable if use create/delete the file on the same machine

int main(int argc, char **argv)
{

    //MPI init
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int length, i = 0, wd;
    int fd;
    char buffer[BUF_LEN];

    EVENTTYPE eventType;

    if (argc != 7)
    {
        //printf("<binary> <watchpath> <required number of notification> <notifyserver interfaces><notify server port> <required INIT Number>\n");
        printf("<binary> <sub number> <required number of notification> <notifyserver interfaces><group number><group size><log level> \n");
        return 0;
    }

    //use this if the watchpath is used(this will load the config file from the disk)
    //gothroughFolderRegister(argv[1]);

    int subSize = atoi(argv[1]);

    printf("subsize is %d\n", subSize);

    //deprecated currently
    int requiredNotifiedNum = atoi(argv[2]);

    GM_INTERFACE = string(argv[3]);
    printf("notify server listen to interface %s\n", GM_INTERFACE.data());

    // number of the server cluster
    gm_groupNumber = atoi(argv[4]);
    //use group manager

    gm_requiredGroupSize = atoi(argv[5]);

    gm_rank = world_rank;

    int logLevel = atoi(argv[6]);

    if (logLevel == 0)
    {
        spdlog::set_level(spdlog::level::info);
    }
    else
    {
        spdlog::set_level(spdlog::level::debug);
    }

    spdlog::info("curr component id {}", gm_rank);

    //init the worker and coordinator
    //init clients when sending the events
    //initMultiClients();

    //start a new thread to run notify server
    //parse the json file and create the clientid and put them in a map
    pthread_t notifyserverid;
    int status;

    //get notify server addr
    string notifyAddr = getNotifyServerAddr();
    NOTIFYADDR = notifyAddr;
    //send value to server addr

    //start the server
    pthread_create(&notifyserverid, NULL, &RunNotifyServer, NULL);

    //wait the notify server start
    sleep(1);

    //onePubMultipleSameSub(subSize, notifyAddr);

    fakegothroughFolderRegister(subSize, notifyAddr);

    //multipleSub(subSize, notifyAddr);

    while (1)
    {

        usleep(1000);
    }
}