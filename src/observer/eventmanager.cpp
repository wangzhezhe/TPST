
#include "stdlib.h"
#include "stdio.h"
#include "errno.h"
#include "string.h"
#include "pthread.h"

#include "eventmanager.h"
#include "../runtime/slurm.h"
#include "../runtime/local.h"
#include "../publishclient/pubsubclient.h"
#include <string>
#include <iostream>
#include <queue>
#include <stdint.h> /* for uint64 definition */
#include <stdlib.h> /* for exit() definition */
#include <time.h>   /* for clock_gettime */
#include <pthread.h>
#include <uuid/uuid.h>
#define BILLION 1000000000L

using namespace std;

/*
{
    "type": "EVENT",
    "eventList": ["T1_FINISH"],
    "driver": "slurm",
    "actionList": [
       "./triguresbatchjoba",
       "./triguresbatchjobb"
    ]
}
*/

enum FILETYPE
{
    TRIGGER,
    OPERATOR
};

queue<pthread_t> threadIdQueue;
mutex subscribedMutex;
int SubscribedClient = 0;

mutex publishMutex;
int publishClient = 0;

vector<string> operatorList;

//from client id to the config file that is needed to be excuted when there is notify request
mutex clientIdtoConfigMtx;
map<string, EventTriggure *> clientIdtoConfig;

void initOperator(int jsonNum)
{

    GreeterClient *greeter = GreeterClient::getClient();
    if (greeter == NULL)
    {
        printf("failed to get initialised greeter\n");
        return;
    }

    vector<string> InitList;
    InitList.push_back(string("INIT"));

    while (1)
    {
        int reply = greeter->GetSubscribedNumber(InitList);
        //printf("there are %d clients subscribe INIT event\n", reply);
        if (reply < jsonNum)
        {
            usleep(100);
        }
        else
        {
            break;
        }
    }

    //it's better to declare a new document instance for new file every time
    Document d;
    //go throught the operatorVector
    int len = operatorList.size();
    int i = 0;
    char command[500];
    for (i = 0; i < len; i++)
    {
        //get the operator command
        //printf("parsing operator (%s)\n", operatorList[i].data());
        d.Parse(operatorList[i].data());

        const char *type = d["type"].GetString();
        printf("execute type:(%s)\n", type);
        const char *action = d["action"].GetString();
        printf("execute action:(%s)\n", action);
        system(action);
    }
}
void eventPublish(vector<string> pubList,string metadata)
{
    GreeterClient *greeter = roundrobinGetClient();

    if (greeter == NULL)
    {
        printf("failed to get greeter for event subscribe\n");
        return;
    }

    publishMutex.lock();
    publishClient++;
    publishMutex.unlock();

    //if (publishClient % 100 == 0)
    //{
    //    printf("publish times %d\n", publishClient);
    //}


    string reply = greeter->Publish(pubList, "CLIENT", metadata);

    if (reply.compare("OK") != 0)
    {
        printf("rpc failed, publish %s failed\n", pubList[0].data());
    }

    return;
}
void eventSubscribe(EventTriggure *etrigger, string clientID, string notifyAddr)
{
    //only could be transfered by this way if original pointed is initiallises by malloc instead on new
    //EventTriggure *etrigger = (EventTriggure *)arguments;

    GreeterClient *greeter = roundrobinGetClient();

    if (greeter == NULL)
    {
        printf("failed to get greeter for event subscribe\n");
        return;
    }

    //printf("success to get the greeter\n");

    //debug how long should be used to go get the subscribed event

    uint64_t diff;
    struct timespec start, end;

    /* measure monotonic time */
    clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

    subscribedMutex.lock();
    SubscribedClient++;
    subscribedMutex.unlock();

    //if (SubscribedClient % 100 == 0)
    //{
    //    printf("sub times %d\n", SubscribedClient);
    //}

    //printf("debug sub event %s\n",etrigger->eventSubList[0].data());
    string reply = greeter->Subscribe(etrigger->eventSubList, clientID, notifyAddr);
    //cout << "Subscribe return value: " << reply << endl;

    clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */

    diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    //printf("debug time get (%s) response time = (%lf) second\n", etrigger->eventList[0].data(), (float)diff / BILLION);

    int i = 0;
    if (reply.compare("SUBSCRIBED") != 0)
    {
        printf("rpc failed, don't execute command:\n");
        int actionSize = etrigger->actionList.size();
        for (i = 0; i < actionSize; i++)
        {
            printf("%s\n", etrigger->actionList[i].data());
        }
    }

    return;
}

int jsonIfTriggerorOperator(Document &d, char *jsonbuffer)
{
    d.Parse(jsonbuffer);
    const char *type = d["type"].GetString();
    //printf("get type after parsing (%s)\n",type);
    if (strcmp(type, "TRIGGER") == 0)
    {
        return 1;
    }
    else if (strcmp(type, "OPERATOR") == 0)
    {
        return 2;
    }
    else
    {
        return -1;
    }
}

void waitthreadFinish()
{

    int joinReturn;
    pthread_t currpid;

    while (threadIdQueue.empty() == false)
    {
        printf("thread num waiting to be finished %d\n", threadIdQueue.size());
        currpid = threadIdQueue.front();
        joinReturn = pthread_join(currpid, NULL);
        //printf("thread id %ld return %d\n", currpid, joinReturn);
        threadIdQueue.pop();
    }
}

//controle when to start operator

/*
{
    "type": "TRIGGER",
    "eventList": ["INIT"],
    "driver": "local",
    "actionList": [
       "/bin/bash ./app/simulate.sh --timesteps 1 --range 100 --nvalues 5 --log off > sim1.out",
       "./operator T1SIM_FINISH 3 publish T1SIM_FINISH"
     ]
}
*/

/*
typedef struct EventTriggure
{
    vector<string> eventList;
    string driver;
    vector<string> actionList;

} EventTriggure;
*/

void outputTriggure(string clientId)
{
    EventTriggure *et = clientIdtoConfig[clientId];
    printf("driver %s\n", et->driver.data());
    int size = et->eventSubList.size();
    int i;
    for (i = 0; i < size; i++)
    {
        printf("sub event %s\n", et->eventSubList[i].data());
    }

    size = et->actionList.size();
    for (i = 0; i < size; i++)
    {
        printf("action %s\n", et->actionList[i].data());
    }

    return;
}

/*

typedef struct EventTriggure
{
    string driver;
    vector<string> eventSubList;
    vector<string> eventPubList;
    vector<string> actionList;

} EventTriggure;

*/
//put key info into the configuration
EventTriggure *fakeaddNewConfig(string driver,
                                vector<string> eventSubList,
                                vector<string> eventPubList,
                                vector<string> actionList,
                                string &clientID)
{
    EventTriggure *triggure = new (EventTriggure);

    triggure->driver = driver;
    triggure->eventSubList = eventSubList;
    triggure->eventPubList = eventPubList;
    triggure->actionList = actionList;

    uuid_t uuid;
    char idstr[50];

    uuid_generate(uuid);
    uuid_unparse(uuid, idstr);

    string clientId(idstr);
    clientIdtoConfigMtx.lock();
    clientIdtoConfig[clientId] = triggure;
    //printf("add clientid %s\n", clientId.data());
    clientIdtoConfigMtx.unlock();

    //outputTriggure(clientId);
    clientID = clientId;
    return triggure;
}

EventTriggure *addNewConfig(string jsonbuffer, string &clientID)
{
    //parse json buffer
    Document d;
    //printf("current file data %s\n",jsonbuffer.data());
    d.Parse(const_cast<char *>(jsonbuffer.data()));
    //printf("jsonIfTriggerorOperator (%s)\n", jsonbuffer.data());
    const char *type = d["type"].GetString();
    EventTriggure *triggure = new (EventTriggure);

    //get client id
    uuid_t uuid;
    char idstr[50];

    uuid_generate(uuid);
    uuid_unparse(uuid, idstr);
    if (strcmp(type, "TRIGGER") == 0)
    {
        const char *driver = d["driver"].GetString();
        triggure->driver = string(driver);

#ifdef DEBUG
        printf("process event trigure\n");
#endif
        //register trigure and send subscribe call to pub-sub backend
        const Value &eventSubList = d["eventSubList"];
        const Value &eventPubList = d["eventPubList"];
        const Value &actionList = d["actionList"];

        SizeType i;

        for (i = 0; i < eventSubList.Size(); i++)
        {
            const char *tempsubstr = eventSubList[i].GetString();
            string substr = string(tempsubstr);
            triggure->eventSubList.push_back(substr);
        }

        for (i = 0; i < eventPubList.Size(); i++)
        {
            const char *temppubstr = eventPubList[i].GetString();
            string pubstr = string(temppubstr);
            triggure->eventPubList.push_back(pubstr);
        }

        for (i = 0; i < actionList.Size(); i++)
        {
            const char *tempstr = actionList[i].GetString();
            triggure->actionList.push_back(string(tempstr));
        }

        string clientId(idstr);
        clientIdtoConfigMtx.lock();
        clientIdtoConfig[clientId] = triggure;
        //printf("add clientid %s\n", clientId.data());
        clientIdtoConfigMtx.unlock();

        //outputTriggure(clientId);
        clientID = clientId;
        return triggure;
    }
    else if (strcmp(type, "OPERATOR") == 0)
    {
        printf("unsupported operator now %s\n", type);
        return NULL;
    }
    else
    {
        printf("unsupported type %s\n", type);
        return NULL;
    }
}