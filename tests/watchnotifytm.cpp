#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "sys/types.h"
#include "sys/inotify.h"
#include "limits.h"
#include "unistd.h"
#include <dlfcn.h>
#include "../lib/file/loaddata.h"
#include "../observer/taskmanager.h"
#include "../eventstore/eventStore.h"
#include "../lib/rapidjson/include/rapidjson/document.h"
#include "../lib/rapidjson/include/rapidjson/writer.h"
#include "../lib/rapidjson/include/rapidjson/stringbuffer.h"

#define MAX_EVENTS 1024                                /*Max. number of events to process at one go*/
#define LEN_NAME 16                                    /*Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE (sizeof(struct inotify_event))      /*size of one event*/
#define BUF_LEN (MAX_EVENTS * (EVENT_SIZE + LEN_NAME)) /*buffer to store the data of events*/

typedef void (*ACT_FUNC)();

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
    char projectPath[50] = "/home/zw241/observerchain/tests";
    char tmDir[50] = "TaskManagerFiles";
    EVENTTYPE eventType;
    /* Initialize Inotify*/
    fd = inotify_init();
    if (fd < 0)
    {
        perror("Couldn't initialize inotify");
    }

    //init operation create event store
    vector<HeadNode *> es = initEventStore();

    /* add watch to starting directory */
    wd = inotify_add_watch(fd, argv[1], IN_CREATE | IN_MODIFY | IN_DELETE);

    if (wd == -1)
    {
        printf("Couldn't add watch to %s\n", argv[1]);
    }
    else
    {
        printf("Watching:: %s\n", argv[1]);
    }

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
                    if (event->mask & IN_ISDIR)
                        printf("The directory %s was Created.\n", event->name);

                    else
                        printf("The file %s was Created with WD %d\n", event->name, event->wd);
                }

                if (event->mask & IN_MODIFY)
                {
                    eventType = modified;
                    if (event->mask & IN_ISDIR)
                        printf("The directory %s was modified.\n", event->name);
                    else
                        printf("The file %s was modified with WD %d\n", event->name, event->wd);
                }

                if (event->mask & IN_DELETE)
                {
                    eventType = deleted;
                    if (event->mask & IN_ISDIR)
                        printf("The directory %s was deleted.\n", event->name);
                    else
                        printf("The file %s was deleted with WD %d\n", event->name, event->wd);
                }

                i += EVENT_SIZE + event->len;
            }

            //if the file contains the  .json
            int rcode = ifjson(event->name);
            if (rcode == 1)
            {
                //json file path
                char taskPath[100];
                snprintf(taskPath, sizeof taskPath, "%s/%s/%s", projectPath, tmDir, event->name);
                printf("newtask file path %s\n", taskPath);
                //create the task manager by file name
                if (eventType == modified)
                {
                    printf("create tm by %s\n", taskPath);
                    //load the tm json file
                    char *jsonbuffer = NULL;
                    jsonbuffer = loadFile(taskPath);
                    Document d;
                    d.Parse(jsonbuffer);
                    const char *taskName;
                    const char *listenevent;
                    const char *pushevent;
                    const char *actionfuncPath;
                    const char *filterfuncPath;
                    taskName = d["name"].GetString();

                    listenevent = d["listenEvent"].GetString();
                    pushevent = d["publishEvent"].GetString();
                    actionfuncPath = d["actionFunc"].GetString();
                    filterfuncPath = d["filterFunc"].GetString();
                    //if the tm is new

                    printf("get listenevent name %s\n", listenevent);
                    printf("get pushevent name %s\n", pushevent);

                    TaskManager *t = Task_create(taskName);
                    //register relevent event and function

                    Task_pushevent(t, pushevent);

                    Task_listen(es, t, listenevent);

                    printEventStore(es);

                    //get the function name and compile it dynamically
                    char filterPath[100];
                    char actionPath[100];
                    snprintf(filterPath, sizeof(filterPath), "%s/%s/%s", projectPath, tmDir, filterfuncPath);
                    snprintf(actionPath, sizeof(actionPath), "%s/%s/%s", projectPath, tmDir, actionfuncPath);

                    printf("get action path %s\n", actionPath);
                    printf("get filter path %s\n", filterPath);
                    //use the dynamic link load the function back
                    void *funcHandle;

                    //open dynamic link library
                    funcHandle = dlopen(actionPath, RTLD_LAZY);
                    if (!funcHandle)
                    {
                        fprintf(stderr, "%s\n", dlerror());
                        exit(EXIT_FAILURE);
                    }
                    dlerror();
                    ACT_FUNC act_func = NULL;
                    //TODO compile into .so file dynamically
                    *(void **)(&act_func) = dlsym(funcHandle, "action");
                    char *error;
                    if ((error = dlerror()) != NULL)
                    {
                        fprintf(stderr, "%s\n", error);
                        exit(EXIT_FAILURE);
                    }
                    //call the func
                    (*act_func)();

                    dlclose(funcHandle);
                    //TODO else  modify the old one

                    //register the function
                }
            }
            //than load the relavent  json  file, create the task manager
            //extract the dynamic func and build into .c file
            //get the path of the .so file
            //load it into the object
        }
    }

    /* Clean up*/
    inotify_rm_watch(fd, wd);
    close(fd);

    return 0;
}