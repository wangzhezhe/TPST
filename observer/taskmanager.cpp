#include "stdlib.h"
#include "stdio.h"
#include "errno.h"
#include "string.h"
#include "taskmanager.h"
#include "observer.h"
#include "sys/types.h"
#include "sys/inotify.h"
#include "limits.h"
#include "unistd.h"
#include <string>
#include <iostream>
#include <map>
#include <dlfcn.h>
#include <vector>
#include <pthread.h>
#include "../lib/file/loaddata.h"
#include "../observer/taskmanager.h"
#include "../eventstore/eventStore.h"
#include "../lib/rapidjson/include/rapidjson/document.h"
#include "../lib/rapidjson/include/rapidjson/writer.h"
#include "../lib/rapidjson/include/rapidjson/stringbuffer.h"
#include "../storage/memcache.h"

using namespace std;
using namespace rapidjson;

//global eveent map
//data in es is only meta data
map<string, TaskManager *> taskManagerGlobal;

//use es (event store) as a global variable
vector<HeadNode *> es;

//this should be defined in the taskmanager.cpp
map<string, vector<float> > storage;


void *startFirstAction(void *args)
{
    TaskManager *tm = static_cast<TaskManager *>(args);
    printf("execute action function of tm (%s)\n", tm->taskmeta->name);
    //(*actionFunc)(vector<TaskManagerMeta *>, map<int, vector<float> > );
    extern map<string, TaskManager *> taskManagerGlobal;
    tm->observer->actionfunc(es, storage);

    //publish event to event store and trigure the associated one in new thread
    if (tm->taskmeta->publishEvent != NULL)
    {
        notifyTmList(tm, es, tm->taskmeta->publishEvent);
    }
}

TaskManager *Task_create(const char *tmname, const char *aggRule, const char *subevent, const char *pubevent, vector<HeadNode *> &es, TaskType taskType)
{


    //if exist<same name>, return the old one
    printf("test taskType in create %d\n", taskType);

    //if not exist, creat new one
    TaskManager *tm = getTmfromES(tmname, es);

    if (tm != NULL)
    {
        return tm;
    }
    TaskManagerMeta *tmmeta = new (TaskManagerMeta);
    tm = new (TaskManager);

    memcpy(tmmeta->name, tmname, strlen(tmname) + 1);

    if (aggRule != NULL)
    {
        memcpy(tmmeta->aggRule, aggRule, strlen(aggRule) + 1);
    }

    tmmeta->tasktype = taskType;

    tm->taskmeta = tmmeta;
    tm->observer = NULL;
    tm->subject = NULL;

    Task_pushevent(tm, pubevent);

    Task_subscribe(es, tm, subevent);

    //insert tm into global cache
    string str = tmname;

    taskManagerGlobal[str] = tm;

    printf("task create name (%s) blocktype (%d)\n", tmmeta->name, tmmeta->tasktype);

    printf("task manager lobal insert ok\n");

    return tm;
}

void TaskFileParsing(Document &d, const char *taskName, char *jsonbuffer,char*taskPath,char*projectPath,char*tmDir)
{
    const char *subscribeevent;
    const char *pushevent;
    const char *actionfuncPath;
    const char *filterfuncPath;
    const char *aggfuncPath;
    void *funcHandleAction;
    void *funcHandleFilter;
    void *funcHandleAggreFilter;
    d.Parse(jsonbuffer);
    subscribeevent = d["subscribeEvent"].GetString();
    pushevent = d["publishEvent"].GetString();
    //if task have filter func, there is no action func
    //actionfuncPath = d["actionFunc"].GetString();

    //if the tm is new

    printf("get subscribeevent name %s\n", subscribeevent);
    printf("get pushevent name %s\n", pushevent);
    printf("create tm by %s\n", taskPath);

    bool ifContainAggre = d.HasMember("aggregationrule");
    bool ifContainAggreFunc = d.HasMember("aggregationFilterFunc");
    bool ifUnblocked = d.HasMember("taskType");

    TaskType taskType = BLOCKED;

    if (ifUnblocked)
    {
        taskType = UNBLOCKED;
    }

    TaskManager *t = NULL;
    if (ifContainAggre == true && ifContainAggreFunc == true)
    {
        printf("task (%s) with the filter operation\n", taskName);

        const char *aggreRule = d["aggregationrule"].GetString();

        //TaskManager *Task_create(const char *tmname, const char *aggRule, char *subevent, char *pubevent, vector<TaskManagerMeta *> &es)
        t = Task_create(taskName, aggreRule, subscribeevent, pushevent, es, taskType);

        //load aggregationFunc and register
        const char *aggfuncPath = d["aggregationFilterFunc"].GetString();
        filterfuncPath = d["filterFunc"].GetString();
        printf("get aggrule (%s) get aggfuncpath (%s)\n", aggreRule, aggfuncPath);
    }
    else
    {

        printf("task manager %s do not include filter operation\n", taskName);
        t = Task_create(taskName, NULL, subscribeevent, pushevent, es, taskType);
    }

    //register relevent event and function
    printEventStore(es);

    //get the function name and compile it dynamically
    char filterPath[100];
    char actionPath[100];

    printf("get filter path %s\n", filterPath);

    //open dynamic link library

    if (ifContainAggre == true && ifContainAggreFunc == true)
    {
        aggFilterFunc agg_filter_func = NULL;
        //TODO compile into .so file dynamically
        aggfuncPath = d["aggregationFilterFunc"].GetString();
        char aggfilterPath[100];
        char filterPath[100];

        snprintf(aggfilterPath, sizeof(aggfilterPath), "%s/%s/%s", projectPath, tmDir, aggfuncPath);
        snprintf(filterPath, sizeof(filterPath), "%s/%s/%s", projectPath, tmDir, filterfuncPath);

        printf("get aggfilter path %s\n", aggfilterPath);
        funcHandleAggreFilter = dlopen(aggfilterPath, RTLD_LAZY);
        funcHandleFilter = dlopen(filterPath, RTLD_LAZY);
        filterFunc fit_func = NULL;
        //TODO compile into .so file dynamically
        *(void **)(&fit_func) = dlsym(funcHandleFilter, "filter");
        Task_registerFilter(t, fit_func);
        *(void **)(&agg_filter_func) = dlsym(funcHandleAggreFilter, "aggregatefilter");
        Task_registerAggreFilter(t, agg_filter_func);
    }
    else
    {
        actionfuncPath = d["actionFunc"].GetString();
        snprintf(actionPath, sizeof(actionPath), "%s/%s/%s", projectPath, tmDir, actionfuncPath);

        printf("get action path %s\n", actionPath);
        funcHandleAction = dlopen(actionPath, RTLD_LAZY);

        if (funcHandleAction == NULL && funcHandleFilter == NULL)
        {
            fprintf(stderr, "%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        dlerror();
        actionFunc act_func = NULL;
        //TODO compile into .so file dynamically
        act_func = (actionFunc)dlsym(funcHandleAction, "action");

        //put the func into the tm
        Task_registerAction(t, act_func);
    }

    char *error;
    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }

    //TODO use filter function to controle the excution of first
    //if subscribe event is NOSUBSCRIBE
    //start a new thread to execute the action function of this task

    if (strcmp(t->taskmeta->subEvent->str, "NULL") == 0)
    {
        pthread_t id;
        pthread_create(&id, NULL, startFirstAction, t);
    }
}

int Task_pushevent(TaskManager *tm, const char *event_str)
{
    TaskEvent *te = (TaskEvent *)malloc(sizeof(TaskEvent));
    memcpy(te->str, event_str, strlen(event_str) + 1);
    tm->taskmeta->publishEvent = te;
    printf("task manager %s will pub event:(%s)\n", tm->taskmeta->name, te->str);
    return 0;
}

int Task_subscribe(vector<HeadNode *> &es, TaskManager *tm, const char *event_str)
{
    TaskEvent *te = (TaskEvent *)malloc(sizeof(TaskEvent));
    memcpy(te->str, event_str, strlen(event_str) + 1);
    tm->taskmeta->subEvent = te;
    printf("task manager %s will subscribe event:(%s)\n", tm->taskmeta->name, te->str);

    //put the event into the event store
    registerEvent(tm->taskmeta, te, es);
    return 0;
}

int Task_registerAction(TaskManager *tm, actionFunc actionfun)
{

    // ok to excute here notifyfun();
    if (tm->observer == NULL)
    {
        tm->observer = observerNew(tm, actionfun);
    }
    else
    {
        //TODO use list to notify a new function?
        printf("observer exist\n");
    }
    printf("%s register the action function\n", tm->taskmeta->name);
    return 0;
}

//TODO register action and filter

int Task_registerFilter(TaskManager *tm, filterFunc filterfunc)
{

    // ok to excute here notifyfun();
    if (tm->observer != NULL)
    {
        tm->observer->filterfunc = filterfunc;
    }
    else
    {
        //TODO use list to notify a new function?
        printf("observer not exist\n");
    }
    printf("%s register the filter function\n", tm->taskmeta->name);
    return 0;
}

//TODO register regregation

int Task_registerAggreFilter(TaskManager *tm, aggFilterFunc aggfilterfunc)
{

    // ok to excute here notifyfun();
    if (tm->observer != NULL)
    {
        tm->observer->aggfilterfunc = aggfilterfunc;
    }
    else
    {
        //TODO use list to notify a new function?
        printf("observer not exist\n");
    }
    printf("%s register the AggreFilter function\n", tm->taskmeta->name);
    return 0;
}

TaskEvent *Task_getPublishEvent(TaskManagerMeta *taskmeta)
{
    if (taskmeta->publishEvent != NULL)
    {
        return taskmeta->publishEvent;
    }
    else
    {
        printf("publish event is NULL for %s\n", taskmeta->name);
        //exit(EXIT_FAILURE);
        return NULL;
    }
}

//TODO this func shoule be independent to specific Task
void Task_handleEvent()
{
    //printf("%s get the event and do some actions\n", tm->name);
    printf("notice the event, start new thread to do some actions here\n");
    //notifyObserver();
}

//register event and associated tm to the event store
//tm interested in the event
int registerEvent(TaskManagerMeta *tmmeta, TaskEvent *te, vector<HeadNode *> &es)
{

    //traverse event cache, insert tm into list start with associated event node
    int i = 0;
    int j = 0;
    int count = es.size();
    int tmcount = 0;

    //comare the event node, if match insert
    //printf("curr count %d\n", count);
    for (i = 0; i < count; i++)
    {
        if (strcmp(te->str, es[i]->te->str) == 0)
        {
            //insert
            //TODO if the tm have alreay been inserted, skip the operation
            tmcount = es[i]->tmList.size();
            for (j = 0; j < tmcount; j++)
            {
                if (strcmp(tmmeta->name, es[i]->tmList[j]->name) == 0)
                {
                    //event exit, skip inserting operation
                    return 0;
                }
            }
            es[i]->tmList.push_back(tmmeta);
            return 0;
        }
    }

    //if not match, create new node
    HeadNode *newhead = new (HeadNode);
    newhead->te = te;
    vector<TaskManagerMeta *> newtmList;
    newtmList.push_back(tmmeta);
    newhead->tmList = newtmList;
    es.push_back(newhead);

    return 0;
}

TaskManager *getTmfromES(const char *tmname, vector<HeadNode *> &es)
{
    //put the intact datastructure into the map
    string key = tmname;

    cout << key << endl;

    map<string, TaskManager *>::iterator iter;

    iter = taskManagerGlobal.find(key);
    if (iter != taskManagerGlobal.end())
    {
        // the first variable of iterator is key, the second variable is value
        return iter->second;
    }
    else
    {
        return NULL;
    }

    //use key to index the element

    //TODO update here
    //use other search algorithm to improve the index speed
    /*
    int count = es.size();
    int tmlistCount;
    int i, j;
    for (i = 0; i < count; i++)
    {
        //range list and find task manger
        tmlistCount = es[i]->tmList.size();
        for (j = 0; j < tmlistCount; j++)
        {
            if (strcmp(tmname, es[i]->tmList[j]->name) == 0)
            {
                //delete tmlist[j]
                return es[i]->tmList[j];
            }
        }
    }
    //TODO delete the associated event that the tm attached to
    //add another element in event node to label how many tm will publish this event
    return NULL;
    */
}

//delete the task manager from the event srore
//assume that element in the tm is initialised
int deleteTmfromES(TaskManager *tm, vector<HeadNode *> &es)
{
    //get the listen event from the list
    TaskEvent *subEvent = tm->taskmeta->subEvent;
    //traverse the es and search the event
    int count = es.size();
    int tmlistCount;
    int i, j;
    for (i = 0; i < count; i++)
    {
        if (strcmp(subEvent->str, es[i]->te->str) == 0)
        {
            //range list and find task manger
            tmlistCount = es[i]->tmList.size();
            for (j = 0; j < tmlistCount; j++)
            {
                if (strcmp(tm->taskmeta->name, es[i]->tmList[j]->name) == 0)
                {
                    //delete tmlist[j]
                    es[i]->tmList.erase(es[i]->tmList.begin() + j);
                }
            }
        }
    }
    return 0;
    //TODO delete the associated event that the tm attached to
    //add another element in event node to label how many tm will publish this event
}

//input: specific event message
//action: trigger the tm registered to this event message
//call the notify function registered into the task manager earlier

int notifyTmList(TaskManager *tm, vector<HeadNode *> &es, TaskEvent *te)
{
    int i, j;
    int count = es.size();
    int listCount = 0;
    // traverse the es and find the match event
    // need to be optimised here
    printf("start process of publishing event (%s)\n", tm->taskmeta->publishEvent);
    for (i = 0; i < count; i++)
    {
        if (strcmp(te->str, es[i]->te->str) == 0)
        {
            //traverse the taskmanager if event match
            //call the function registered into the taskManager
            listCount = es[i]->tmList.size();
            for (j = 0; j < listCount; j++)
            {
                //TODO start new thread to do this
                //get tm from the tm meta

                TaskManager *tm = getTmfromES(es[i]->tmList[j]->name, es);

                if (strcmp(es[i]->tmList[j]->publishEvent->str, "NULL") != 0)
                {

                    //get the event need to be published into the eventstore first
                    //start this part in parallel
                    TaskEvent *publishevent = Task_getPublishEvent(es[i]->tmList[j]);
                    printf("publish event(%s)\n", publishevent->str);
                    if (publishevent != NULL)
                    {
                        // call the function and publish relevant event
                        callNotify(tm, es, publishevent, tm->observer);
                    }
                }
                else
                {
                    printf("the tm name is %s, the publish of this tm is NULL\n", es[i]->tmList[j]->name);
                }
            }
        }
    }
    return 0;
}

int Task_destroy(TaskManager *tm)
{

    //free observer subject
    if (tm == NULL)
    {
        return 0;
    }
    if (tm->taskmeta->publishEvent != NULL)
    {
        free(tm->taskmeta->publishEvent);
    }

    if (tm->taskmeta->subEvent != NULL)
    {
        free(tm->taskmeta->subEvent);
    }

    free(tm->taskmeta);
    free(tm);

    return 0;
}

void *executeActionFunc(void *args)
{

    //get tm from the input parameters
    TaskManager *tm = static_cast<TaskManager *>(args);
    printf("start a new thread to execute action function of tm (%s)\n", tm->taskmeta->name);
    //typedef int (*actionFunc)(vector<HeadNode *>, map<string, vector <float> >);
    tm->observer->actionfunc(es, storage);
}

// call the function in current tm in a new thread
//
int callNotify(TaskManager *tm, vector<HeadNode *> &es, TaskEvent *te, Observer *obs)
{

    if (obs->actionfunc != NULL)
    {
        //1 start new thread to execute the action function
        pthread_t id;
        pthread_create(&id, NULL, executeActionFunc, tm);

        //2 publish relevant event after new thread finish running
        //  support different level of logic entities here (thread level, application level<need some runtime tool to help this>)

        // according to event type
        // if it is blocked one, publish the event after new thread finish exectution
        // if it is unblocked one, publish the event when new thread start
        printf("block type for (%s): (%d)\n", tm->taskmeta->name, tm->taskmeta->tasktype);
        if (tm->taskmeta->tasktype == BLOCKED)
        {
            pthread_join(id, NULL);
        }
        else
        {
            //publish the following events even if the action function not finish for current taskmanager
            printf("(%s) is unblocked type\n", tm->taskmeta->name);
        }
    }
    //call the function of the taskManager and put finish event into the store

    notifyTmList(tm, es, te);
    return 0;
}

//aggregation data this should provided by an api
//range the input data and call the filter func, if data return 1, than aggregate, else break, using openmp
//return the data after pushing operation

int TaskFilterAndAggregation(vector<double> dataArray, vector<HeadNode *> &es)
{

    //range the registered TM
    int count = es.size();
    int tmlistCount;
    int i, j, k;
    int dataLen;
    double tempv;
    double redvalue;
    int ifAggre, ifPush;
    //TODO load the variable from the configure file
    int constraints1 = 9;
    int constraints2 = 200;
    for (i = 0; i < count; i++)
    {
        //range list and find task manger
        tmlistCount = es[i]->tmList.size();
        for (j = 0; j < tmlistCount; j++)
        {
            //TODO get tm from tm list
            /*
            TaskManager *tm = es[i]->tmList[j];

            // for every TM in the es, get the filter function, get aggregation rules
            // range the data
            dataLen = dataArray.size();

            // omp_set_num_threads(16);
            redvalue = 0;
            for (k = 0; k < dataLen; k++)
            {

                //filter and aggregation
                //if contain the filter label, do filter and aggregation
                //TODO pass this constraints from tne outside of the function
                //this should be done by a new thread

                if (IfFilterAggregation(tm) == 1)
                {
                    ifAggre = TaskFilter(tm, dataArray[k], constraints1);

                    if (ifAggre == 1)
                    {
                        redvalue = redvalue + dataArray[k];
                    }
                }
            }
            if (IfFilterAggregation(tm) == 1)
            {
                //use redvalue to tets the reduction filter
                printf("aggregate value %f\n", redvalue);
                ifPush = TaskReductionFilter(tm, redvalue, constraints2);
                //printf("if push event %d\n", ifPush);
                if (ifPush == 1)
                {
                    printf("filter func return ture, tm (%s) push events (%s)\n", tm->name, tm->subEvent->str);
                    callNotify(tm, es, tm->publishEvent, tm->observer);
                }
                //according to the result of the aggregation to determine if publish the event
            }
            */
        }
    }
}

int IfFilterAggregation(TaskManager *tm)
{

    if (tm->observer->aggfilterfunc != NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//filter data
//input data value, call the filter function return if it is filtered out
int TaskFilter(TaskManager *tm, double value, double constraint)
{
    if (tm->observer->filterfunc != NULL)
    {
        //TODO matain a map from the pointer to the function name
        int retv = tm->observer->filterfunc(value, constraint);
        //printf("ret value for filter func %d\n",retv);
        return retv;
    }
    else
    {
        printf("failed to get the filter function\n");
    }
    return 0;
}

int TaskReductionFilter(TaskManager *tm, double value, double constraint)
{
    if (tm->observer->aggfilterfunc != NULL)
    {
        //TODO matain a map from the pointer to the function name
        int retv = tm->observer->aggfilterfunc(value, constraint);
        return retv;
    }
    else
    {
        printf("failed to get the action filter function\n");
    }
    return 0;
}