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

//#include "../observer/taskmanager.h"
//#include "../eventstore/eventStore.h"
//#include "../storage/memcache.h"

#include "../src/utils/getip/getip.h"

#include "../src/utils/file/loaddata.h"

#include "../src/observer/eventmanager.h"

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

//char projectPath[50] = "/home1/zw241/observerchain/tests";
//char tmDir[50] = "TaskManagerFiles";
char projectPath[100] = "/home/parallels/Documents/cworkspace/observerchain/tests";
char tmDir[50] = "TrigureFiles";

//go through the Trigurefile folder and register the .json file with type=trigure into the system
void gothroughFolderRegister(Document &d, const char *dir)
{
    vector<string> fileList;
    fileList = scanFolder(dir);

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
        snprintf(taskPath, sizeof taskPath, "%s/%s/%s", projectPath, tmDir, fileList[i].data());
        char *jsonbuffer = NULL;
        jsonbuffer = loadFile(taskPath);
        //printf("taskPath %s\n",taskPath);
        //printf("json buffer %s\n",jsonbuffer);
        int iftrigger = jsonIfTrigger(d, jsonbuffer);
        if (iftrigger == 1)
        {
#ifdef DEBUG
            //do the register operation
            printf("register the file:(%s)\n", fileList[i].data());
#endif
            //subscribe the specific file
            jsonParsingTrigger(d, jsonbuffer);
        }
    }
    return;
}

void *tempStartOperator(void *arg)
{
    //system("/home1/zw241/observerchain/src/operator/operator");
    system("/home/parallels/Documents/cworkspace/observerchain/src/operator");
    return NULL;
}

//g++ -o notify watchnotifytm.cpp ../lib/file/loaddata.c
//this source code is only avliable if use create/delete the file on the same machine

int main(int argc, char **argv)
{

    int length, i = 0, wd;
    int fd;
    char buffer[BUF_LEN];

    Document d;
    EVENTTYPE eventType;

    if (argc != 2)
    {
        printf("<binary> <watchpath>\n");
        return 0;
    }

    // write ip port of current nodes into config files
    // the load operation is defined at pubsubclient
     
    gothroughFolderRegister(d, argv[1]);

#ifdef TIME
    //send publish api and record time
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    printf("start sec:(%ld), start nsec:(%ld)\n", start.tv_sec, start.tv_nsec);
    pthread_t id;
    pthread_create(&id, NULL, tempStartOperator, NULL);
#endif

    // TODO If put the Document d in the while loop/
    // it will crash when load the data for second time???

    /* Initialize Inotify*/
    fd = inotify_init();
    //void *funcHandleAction;
    //void *funcHandleFilter;
    //void *funcHandleAggreFilter;
    if (fd < 0)
    {
        perror("Couldn't initialize inotify");
    }

    //init operation create event store
    //TODO add lock operation for underlying part of es
    //es = initEventStore();

    //storage simulator
    //TODO change it into interface to support multiple storage end
    //map<int, vector<float> > memcache = initMemCache();

    //TODO scan the dir before watch to register all the .json files automatically
    /* add watch to starting directory */
    wd = inotify_add_watch(fd, argv[1], IN_CREATE | IN_MODIFY | IN_DELETE);
    //use the dynamic link load the function back
    void *funcHandle;
    if (wd == -1)
    {
        printf("Couldn't add watch to %s\n", argv[1]);
    }
    else
    {
        printf("Watching:: %s\n", argv[1]);
    }

    //create another thread to create the test data for every time step
    //add lock for the underlaying datastructure
    //pthread_t id;
    //pthread_create(&id, NULL, createData, &es);

    /* do it forever*/
    while (1)
    {
        i = 0;
        length = read(fd, buffer, BUF_LEN);

        if (length < 0)
        {
            perror("read");
        }

        while (i < length)
        {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];

            if (event->len)
            {
                if (event->mask & IN_CREATE)
                {
                    eventType = created;
                }

                if (event->mask & IN_MODIFY)
                {
                    eventType = modified;
                }

                if (event->mask & IN_DELETE)
                {
                    eventType = deleted;
                }

                i += EVENT_SIZE + event->len;

                //if the file contains the  .json
                //printf("ename %s\n", event->name);
                int rcode = ifjson(event->name);
                if (rcode == 1)
                {

                    //detect if the file exist
                    //if exist, update relevent info and update the event store

                    //if not exist, create and push to event store
                    //json file path
                    char taskPath[100];
                    snprintf(taskPath, sizeof taskPath, "%s/%s/%s", projectPath, tmDir, event->name);
                    printf("trigure file path %s\n", taskPath);
                    if (eventType == modified)
                    {
                        //create the task manager by file name
                        //load the tm json file
                        printf("test modified\n");
                        char *jsonbuffer = NULL;
                        jsonbuffer = loadFile(taskPath);
                        //there will be some weird characters at the end of file leading to the parsing fail some times
                        printf("json buffer\n%s\n", jsonbuffer);
                        jsonParsingTrigger(d, jsonbuffer);
                        //jsonParsingTrigger(jsonbuffer);
                    }
                    else if (eventType == deleted)
                    {
                        printf("TODO: process delete cases\n");
                    }
                    else
                    {
                        printf("TODO: process eventtype %s case\n", event->name);
                    }
                }
                //than load the relavent  json  file, create the task manager
                //extract the dynamic func and build into .c file
                //get the path of the .so file
                //load it into the object
            }
        }
    }

    /* Clean up*/
    //if handler is closed, the function could not be excuted!
    //TODO use a new handler for the new function
    dlclose(funcHandle);
    //TODO else ,modify the old one
    inotify_rm_watch(fd, wd);
    close(fd);

    return 0;
}
