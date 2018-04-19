#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "sys/types.h"
#include "sys/inotify.h"
#include "limits.h"
#include "unistd.h"
#include <dlfcn.h>
#include <vector>
#include <pthread.h>

//#include "../observer/taskmanager.h"
//#include "../eventstore/eventStore.h"
//#include "../storage/memcache.h"
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

enum EVENTTYPE
{
    created,
    modified,
    deleted
};



//g++ -o notify watchnotifytm.cpp ../lib/file/loaddata.c
//this source code is only avliable if use create/delete the file on the same machine

int main(int argc, char **argv)
{
    int length, i = 0, wd;
    int fd;
    char buffer[BUF_LEN];
    char projectPath[50] = "/home1/zw241/observerchain/tests";
    //char tmDir[50] = "TaskManagerFiles";
    char tmDir[50] = "TrigureFiles";
    Document d;
    EVENTTYPE eventType;
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

     
                         jsonParsing(d,jsonbuffer);
                        //jsonParsing(jsonbuffer);
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