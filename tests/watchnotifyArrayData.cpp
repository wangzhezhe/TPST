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

using namespace rapidjson;

enum EVENTTYPE
{
    created,
    modified,
    deleted
};
vector<double> varray;

//be careful that return value should be the void* here
void *createData(void *args)
{
    int vecSize = 200;
    int count = 0;

    while (1)
    {

        //creat the random number
        //put the number into vector
        int i = 0;
        double temp;
        varray.clear();
        for (i = 0; i < vecSize; i++)
        {
            temp = (double)rand() * 10 / RAND_MAX;
            varray.push_back(temp);
        }
        printf("timestep %d\n", count);
        count++;
        //new time step

        //if filter and aggregation operation finish
        //than call the sleep operation
        //vector<int> *v = static_cast<vector<int> *>(args);
        vector<HeadNode *> *tmpes = static_cast<vector<HeadNode *> *>(args);
        //if contain the sum label 
        TaskFilterAndAggregation(varray, *tmpes);
        //simulate every steps
        usleep(2000000);
    }
}

//g++ -o notify watchnotifytm.cpp ../lib/file/loaddata.c
//this source code is only avliable if use create/delete the file on the same machine

int main(int argc, char **argv)
{
    int length, i = 0, wd;
    int fd;
    char buffer[BUF_LEN];
    char projectPath[50] = "/home1/zw241/observerchain/tests";
    char tmDir[50] = "TaskManagerFiles";
    EVENTTYPE eventType;
    // TODO If put the Document d in the while loop/
    // it will crash when load the data for second time???
    Document d;
    /* Initialize Inotify*/
    fd = inotify_init();
    void *funcHandleAction;
    void *funcHandleFilter;
    void *funcHandleAggreFilter;
    if (fd < 0)
    {
        perror("Couldn't initialize inotify");
    }

    //init operation create event store
    //TODO add lock operation for underlying part of es
    vector<HeadNode *> es = initEventStore();

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
    pthread_t id;
    pthread_create(&id, NULL, createData, &es);

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

                //detect if the file exist
                //if exist, update relevent info and update the event store

                //if not exist, create and push to event store
                //json file path
                char taskPath[100];
                snprintf(taskPath, sizeof taskPath, "%s/%s/%s", projectPath, tmDir, event->name);
                printf("newtask file path %s\n", taskPath);
                //printf("current eventmask %d\n", event->mask);
                if (eventType == modified)
                {

                    //create the task manager by file name
                    //load the tm json file
                    printf("test modified\n");
                    char *jsonbuffer = NULL;
                    jsonbuffer = loadFile(taskPath);
                    //there will be some weird characters at the end of file leading to the parsing fail some times
                    //printf("json buffer %s\n", jsonbuffer);

                    d.Parse(jsonbuffer);
                    const char *taskName;
                    const char *subscribeevent;
                    const char *pushevent;
                    const char *actionfuncPath;
                    const char *filterfuncPath;
                    const char *aggfuncPath;
                    //get task name from json file direactly
                    //taskName = d["name"].GetString();
                    taskName = getTaskNameFromEventName(event->name);
                    subscribeevent = d["subscribeEvent"].GetString();
                    pushevent = d["publishEvent"].GetString();
                    actionfuncPath = d["actionFunc"].GetString();
                    filterfuncPath = d["filterFunc"].GetString();
                    //if the tm is new

                    printf("get subscribeevent name %s\n", subscribeevent);
                    printf("get pushevent name %s\n", pushevent);
                    printf("create tm by %s\n", taskPath);

                    //create task manager
                    //TODO put this in an extra func
                    bool ifContainAggre = d.HasMember("aggregationrule");
                    bool ifContainAggreFunc = d.HasMember("aggregationFilterFunc");

                    TaskManager *t = NULL;
                    if (ifContainAggre == true && ifContainAggreFunc == true)
                    {

                        const char *aggreRule = d["aggregationrule"].GetString();

                        t = Task_create(taskName, aggreRule, es);

                        //load aggregationFunc and register
                        const char *aggfuncPath = d["aggregationFilterFunc"].GetString();

                        printf("get aggrule (%s) get aggfuncpath (%s)\n", aggreRule, aggfuncPath);
                    }
                    else
                    {
                        printf("task manager %s do not contain filter operation\n", taskName);
                        t = Task_create(taskName, NULL, es);
                    }

                    //register relevent event and function

                    Task_pushevent(t, pushevent);

                    Task_subscribe(es, t, subscribeevent);

                    printEventStore(es);

                    //get the function name and compile it dynamically
                    char filterPath[100];
                    char actionPath[100];
                    snprintf(filterPath, sizeof(filterPath), "%s/%s/%s", projectPath, tmDir, filterfuncPath);
                    snprintf(actionPath, sizeof(actionPath), "%s/%s/%s", projectPath, tmDir, actionfuncPath);

                    printf("get action path %s\n", actionPath);
                    printf("get filter path %s\n", filterPath);

                    //open dynamic link library

                    funcHandleAction = dlopen(actionPath, RTLD_LAZY);
                    funcHandleFilter = dlopen(filterPath, RTLD_LAZY);

                    if (funcHandleAction == NULL && funcHandleFilter == NULL)
                    {
                        fprintf(stderr, "%s\n", dlerror());
                        exit(EXIT_FAILURE);
                    }
                    dlerror();
                    notifyFunc act_func = NULL;
                    //TODO compile into .so file dynamically
                    *(void **)(&act_func) = dlsym(funcHandleAction, "action");
                    filterFunc fit_func = NULL;
                    //TODO compile into .so file dynamically
                    *(void **)(&fit_func) = dlsym(funcHandleFilter, "filter");
                    //put the func into the tm
                    Task_registerAction(t, act_func);
                    Task_registerFilter(t, fit_func);

                    if (ifContainAggre == true && ifContainAggreFunc == true)
                    {
                        aggFilterFunc agg_filter_func = NULL;
                        //TODO compile into .so file dynamically
                        aggfuncPath = d["aggregationFilterFunc"].GetString();
                        char aggfilterPath[100];
                        snprintf(aggfilterPath, sizeof(aggfilterPath), "%s/%s/%s", projectPath, tmDir, aggfuncPath);

                        printf("get aggfilter path %s\n", aggfilterPath);
                        funcHandleAggreFilter = dlopen(aggfilterPath, RTLD_LAZY);

                        *(void **)(&agg_filter_func) = dlsym(funcHandleAggreFilter, "aggregatefilter");
                        Task_registerAggreFilter(t, agg_filter_func);
                    }

                    char *error;
                    if ((error = dlerror()) != NULL)
                    {
                        fprintf(stderr, "%s\n", error);
                        exit(EXIT_FAILURE);
                    }

                    //TODO use filter function to controle the excution of first
                    //task in the flow
                    //if (strcmp(taskName, "tm0") == 0)
                    //{
                    //    callNotify(t, es, t->publishEvent, t->observer);
                    //}

                    //register the function
                }
                else if (eventType == deleted)
                {

                    printf("test deleted\n");

                    //file name should be same with the task manager name
                    //when deleting, only the name of file will be captured

                    //get task name from file
                    char *taskName = getTaskNameFromEventName(event->name);

                    printf("delete task with name %s\n", taskName);

                    char *jsonbuffer = NULL;

                    TaskManager *tm = getTmfromES(taskName, es);
                    if (tm != NULL)
                    {
                        deleteTmfromES(tm, es);
                    }
                    else
                    {
                        printf("getTmfromEs return NULL, tm is already been deleted\n");
                    }
                    //event store layout after deletion
                    printEventStore(es);
                }
                else
                {
                    printf("TODO: process eventtype %s case\n");
                }
            }
            //than load the relavent  json  file, create the task manager
            //extract the dynamic func and build into .c file
            //get the path of the .so file
            //load it into the object
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