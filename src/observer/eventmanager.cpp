
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

    while (1)
    {
        int reply = greeter->GetSubscribedNumber("INIT");
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

void eventSubscribe(EventTriggure *etrigger, string clientID)
{
    //only could be transfered by this way if original pointed is initiallises by malloc instead on new
    //EventTriggure *etrigger = (EventTriggure *)arguments;
#ifdef DEBUG
    printf("start new thread\n");
    printf("event len %d\n", etrigger->eventList.size());
#endif

    GreeterClient *greeter = GreeterClient::getClient();
    if (greeter == NULL)
    {
        printf("failed to get initialised greeter\n");
        return;
    }

    //debug how long should be used to go get the subscribed event

    uint64_t diff;
    struct timespec start, end;

    /* measure monotonic time */
    clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

    subscribedMutex.lock();
    SubscribedClient++;
    subscribedMutex.unlock();

    string reply = greeter->Subscribe(etrigger->eventList, clientID);
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
/*
void jsonParsingTrigger(Document &d)
{
    //#ifdef DEBUG
    //    printf("debug json buffer%s\n",jsonbuffer);
    //#endif
    //d.Parse(jsonbuffer);
    int subscribedNum = 0;
    const char *type = d["type"].GetString();
    EventTriggure *triggure = new (EventTriggure);
    if (strcmp(type, "TRIGGER") == 0)
    {
#ifdef DEBUG
        printf("process event trigure\n");
#endif
        //register trigure and send subscribe call to pub-sub backend
        const Value &eventList = d["eventList"];
        SizeType i;

        for (i = 0; i < eventList.Size(); i++)
        {
            const char *tempstr = eventList[i].GetString();
            string str = string(tempstr);
            triggure->eventList.push_back(str);
#ifdef DEBUG
            printf("eventList[%d] = %s\n", i, str.data());
#endif
            //strcpy(etrigger->eventList[i], tempstr);
        }
        //etrigger->eventLen = (unsigned int)i;

        const Value &actionList = d["actionList"];

        for (i = 0; i < actionList.Size(); i++)
        {
            const char *tempstr = actionList[i].GetString();
            triggure->actionList.push_back(string(tempstr));

#ifdef DEBUG
            printf("actionList[%d] = %s\n", i, tempstr);
#endif
            //strcpy(etrigger->actionList[i], tempstr);
        }
        //etrigger->actionLen = (unsigned int)i;

        const char *driver = d["driver"].GetString();
#ifdef DEBUG
        printf("driver:%s\n", driver);
#endif

        triggure->driver = string(driver);

        //start a new thread to send the request to pub-sub backend

        //printf("eventlen %d actionlen %d\n",etrigger->eventLen,etrigger->actionLen);

        pthread_t tid;
        if (pthread_create(&tid, NULL, &eventSubscribe, (void *)triggure) != 0)
        {
#ifdef DEBUG
            printf("fail to create pthread wirh id %d\n", (int)tid);
#endif
            return;
        }
        //threadIdList.push_back(tid);
        threadIdQueue.push(tid);
    }

    return;
}
*/

int jsonIfTriggerorOperator(Document &d, char *jsonbuffer)
{
    d.Parse(jsonbuffer);
    //printf("jsonIfTriggerorOperator (%s)\n",jsonbuffer);
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
    int size = et->eventList.size();
    int i;
    for (i = 0; i < size; i++)
    {
        printf("event %s\n", et->eventList[i].data());
    }

    size = et->actionList.size();
    for (i = 0; i < size; i++)
    {
        printf("action %s\n", et->actionList[i].data());
    }

    return;
}

string addNewConfig(string jsonbuffer)
{
    //parse json buffer
    Document d;
    //printf("current file data %s\n",jsonbuffer.data());
    d.Parse(const_cast<char *>(jsonbuffer.data()));
    //printf("jsonIfTriggerorOperator (%s)\n",jsonbuffer);
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
        const Value &eventList = d["eventList"];
        SizeType i;

        for (i = 0; i < eventList.Size(); i++)
        {
            const char *tempstr = eventList[i].GetString();
            string str = string(tempstr);
            triggure->eventList.push_back(str);
#ifdef DEBUG
            printf("eventList[%d] = %s\n", i, str.data());
#endif
        }

        const Value &actionList = d["actionList"];

        for (i = 0; i < actionList.Size(); i++)
        {
            const char *tempstr = actionList[i].GetString();
            triggure->actionList.push_back(string(tempstr));

#ifdef DEBUG
            printf("actionList[%d] = %s\n", i, tempstr);
#endif
        }

        string clientId(idstr);
        clientIdtoConfigMtx.lock();
        clientIdtoConfig[clientId] = triggure;
        printf("add clientid %s\n", clientId.data());
        clientIdtoConfigMtx.unlock();

        //outputTriggure(clientId);
        return clientId;
    }
    else if (strcmp(type, "OPERATOR") == 0)
    {
        printf("unsupported operator now %s\n", type);
        return "";
    }
    else
    {
        printf("unsupported type %s\n", type);
        return "";
    }


}