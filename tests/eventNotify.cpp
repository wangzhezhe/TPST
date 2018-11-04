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
    string reply = greeter->Publish(eventList, "CLIENT", metadata);
    printf("publish INIT event return (%s)\n", reply.data());

    return NULL;
}

//for test using, send multipublish request after subscribe
void fakePublishTest(int pubSize)
{
    int i = 0;
    //srand(time(0));
    for (i = 0; i < pubSize; i++)
    {
        vector<string> pubeventList;

        string metadata = to_string(COMPONENTID) + "metadataTest" + to_string(i);

        //int index = (rand() % (pubSize - 0 + 1));

        string fakePub = to_string(COMPONENTID) + "fakeSub" + to_string(i);

        pubeventList.push_back(fakePub);

        eventPublish(pubeventList, metadata);
    }

    return;
}

void fakegothroughFolderRegister(int subSize)
{
    //the json buffer shouled be load from memory
    /*
{
    "type": "TRIGGER",
    "eventSubList": ["INIT"],
    "eventPubList": ["SIMFINISH"],
    "driver": "local",
    "actionList": [
       "/bin/bash ./app/simulate.sh --timesteps 1 --range 100 --nvalues 5 --log off > sim1.out"
     ]
}

    */

    //TODO using distributed lock to get the id from the shared file system
    int i = 0;
    string TRIGGURETYPE = "TRIGGER";
    string driver = "local";
    //for test using
    string redundantPushEvent = "redundant";

    for (i = 0; i < subSize; i++)
    {
        vector<string> pubeventList;
        vector<string> subeventList;
        vector<string> actionList;

        //every component pub sub different event
        string fakeSub = to_string(COMPONENTID) + "fakeSub" + to_string(i);
        string fakePub = to_string(COMPONENTID) + "fakePub" + to_string(i);
        string fakeaction = "fakeaction" + to_string(i);

        subeventList.push_back(fakeSub);
        pubeventList.push_back(fakePub);
        //pubeventList.push_back(redundantPushEvent);
        actionList.push_back(fakeaction);

        string clientID;

        EventTriggure *etrigger = fakeaddNewConfig(driver, subeventList, pubeventList, actionList, clientID);

        if (clientID != "")
        {
            eventSubscribe(etrigger, clientID);
        }
    }

    fakePublishTest(subSize);

    return;
}

//go through the Trigurefile folder and register the .json file with type=trigure into the system
void gothroughFolderRegister(const char *watchdir)
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
            eventSubscribe(etrigger, clientID);
        }
    }

    return;
}

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

    if (argc != 5)
    {
        //printf("<binary> <watchpath> <required number of notification> <notifyserver interfaces><notify server port> <required INIT Number>\n");
        printf("<binary> <sub number> <required number of notification> <notifyserver interfaces><group number> \n");
        return 0;
    }

    //use this if the watchpath is used(this will load the config file from the disk)
    //gothroughFolderRegister(argv[1]);

    int subSize = atoi(argv[1]);

    printf("subsize is %d\n", subSize);

    int requiredNotifiedNum = atoi(argv[2]);

    INTERFACE = string(argv[3]);
    printf("notify server listen to interface %s\n", INTERFACE.data());

    //assign free one automatically
    //NOTIFYPORT = string(argv[4]);
    //printf("notify server listen to port %s\n", NOTIFYPORT.data());

    // number of the server cluster
    SERVERCLUSTERNUM = atoi(argv[4]);

    //printf("waiting the termination of threads id %ld\n", notifyserverid);

    // send the init request when there are specific number of clients subscribe the init event
    //int requiredInitNum = atoi(argv[5]);

    //int componentid = atoi(argv[6]);

    int componentid = world_rank;

    printf("curr component id %d\n", componentid);

    COMPONENTID = componentid;
    GETIPCOMPONENTID = componentid;

    //int totalNum = atoi(argv[7]);

    int totalNum = world_size;
    GETIPCOMPONENTNUM = totalNum;

    string identity = "client";
    initMultiClients(identity);

    //start a new thread to run notify server
    //parse the json file and create the clientid and put them in a map
    pthread_t notifyserverid;
    int status;
    pthread_create(&notifyserverid, NULL, &RunNotifyServer, NULL);

    //wait the notify server start
    sleep(1);

    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start); /* mark the end time */
    printf("start id %d start time = (%lld.%.9ld)\n", COMPONENTID, (long long)start.tv_sec, start.tv_nsec);

    fakegothroughFolderRegister(subSize);

    //pthread_t operatorid;

    //pthread_create(&operatorid, NULL, &tempStartOperator, (void *)&requiredInitNum);

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