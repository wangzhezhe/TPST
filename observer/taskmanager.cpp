#include "stdlib.h"
#include "string.h"
#include "taskmanager.h"
#include "observer.h"

TaskManager *Task_create(const char *tmname, vector<HeadNode *> &es)
{
    //if exist<same name>, return the old one

    //if not exist, creat new one
    TaskManager *tm = getTmfromES(tmname, es);

    if (tm != NULL)
    {
        return tm;
    }
    tm = (TaskManager *)malloc(sizeof(TaskManager));

    memcpy(tm->name, tmname, strlen(tmname) + 1);
    tm->observer = NULL;
    tm->subject = NULL;
    tm->publishEvent = NULL;
    tm->watchEvent = NULL;

    return tm;
}

int Task_pushevent(TaskManager *tm, const char *event_str)
{
    TaskEvent *te = (TaskEvent *)malloc(sizeof(TaskEvent));
    memcpy(te->str, event_str, strlen(event_str) + 1);
    tm->publishEvent = te;
    printf("taske manager %s will pub event:(%s)\n", tm->name, te->str);
    return 0;
}

int Task_listen(vector<HeadNode *> &es, TaskManager *tm, const char *event_str)
{
    TaskEvent *te = (TaskEvent *)malloc(sizeof(TaskEvent));
    memcpy(te->str, event_str, strlen(event_str) + 1);
    tm->watchEvent = te;
    printf("taske manager %s will listen event:(%s)\n", tm->name, te->str);

    //put the event into the event store
    registerEvent(tm, te, es);
    return 0;
}

int Task_registerAction(TaskManager *tm, notifyFunc notifyfun)
{

    // ok to excute here notifyfun();
    if (tm->observer == NULL)
    {
        tm->observer = observerNew(tm, notifyfun);
    }
    else
    {
        //TODO use list to notify a new function?
        printf("observer exist\n");
    }
    printf("%s register the function\n", tm->name);
    return 0;
}

void notifyObserver()
{
    return;
}

TaskEvent *Task_getPublishEvent(TaskManager *tm)
{
    if (tm->publishEvent != NULL)
    {
        return tm->publishEvent;
    }
    else
    {
        printf("publish event is NULL for %s\n", tm->name);
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
int registerEvent(TaskManager *tm, TaskEvent *te, vector<HeadNode *> &es)
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
                if (strcmp(tm->name, es[i]->tmList[j]->name) == 0)
                {
                    //event exit, skip inserting operation
                    return 0;
                }
            }
            es[i]->tmList.push_back(tm);
            return 0;
        }
    }

    //if no match, create new node
    HeadNode *newhead = new (HeadNode);
    newhead->te = te;
    vector<TaskManager *> newtmList;
    newtmList.push_back(tm);
    newhead->tmList = newtmList;
    es.push_back(newhead);

    
    return 0;
}

TaskManager *getTmfromES(const char *tmname, vector<HeadNode *> &es)
{
    //TODO update here
    //use other search algorithm to improve the index speed
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
}

//delete the task manager from the event srore
//assume that element in the tm is initialised
int deleteTmfromES(TaskManager *tm, vector<HeadNode *> &es)
{
    //get the listen event from the list
    TaskEvent *watchevent = tm->watchEvent;
    //traverse the es and search the event
    int count = es.size();
    int tmlistCount;
    int i, j;
    for (i = 0; i < count; i++)
    {
        if (strcmp(watchevent->str, es[i]->te->str) == 0)
        {
            //range list and find task manger
            tmlistCount = es[i]->tmList.size();
            for (j = 0; j < tmlistCount; j++)
            {
                if (strcmp(tm->name, es[i]->tmList[j]->name) == 0)
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
    //traverse the es and find the match event
    for (i = 0; i < count; i++)
    {
        if (strcmp(te->str, es[i]->te->str) == 0)
        {
            //traverse the taskmanager if event match
            //call the function registered into the taskManager
            listCount = es[i]->tmList.size();
            for (j = 0; j < listCount; j++)
            {
                if (es[i]->tmList[j]->observer != NULL)
                {

                    //get the event need to be published into the eventstore first
                    //start this part in parallel
                    TaskEvent *publishevent = Task_getPublishEvent(es[i]->tmList[j]);
                    printf("publish event(%s)\n", publishevent->str);
                    if (publishevent != NULL)
                    {
                        callNotify(tm, es, publishevent, es[i]->tmList[j]->observer);
                    }
                }
                else
                {
                    printf("the tm name is %s, the observer is NULL\n", es[i]->tmList[j]->name);
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
    if (tm->publishEvent != NULL)
    {
        free(tm->publishEvent);
    }

    if (tm->watchEvent != NULL)
    {
        free(tm->watchEvent);
    }

    free(tm);

    return 0;
}

//the first parameter is the event needed to triguer the event store after function execution
int callNotify(TaskManager *tm, vector<HeadNode *> &es, TaskEvent *te, Observer *obs)
{
    if (obs->notifyfunc != NULL)
    {
        //TODO matain a map from the pointer to the function name
        obs->notifyfunc();
    }
    //call the function of the taskManager and put finish event into the store

    notifyTmList(tm, es, te);
    return 0;
}
