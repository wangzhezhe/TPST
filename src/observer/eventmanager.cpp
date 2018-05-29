
#include "stdlib.h"
#include "stdio.h"
#include "errno.h"
#include "string.h"
#include "pthread.h"

#include "eventmanager.h"
//#include "../redisclient/redisclient.h"
#include "../runtime/slurm.h"
#include "../runtime/local.h"
#include "../publishclient/pubsubclient.h"
#include <string>
#include <iostream>

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

vector<pthread_t> threadIdList;

void *eventSubscribe(void *arguments)
{
    //only could be transfered by this way if original pointed is initiallises by malloc instead on new
    EventTriggure *etrigger = (EventTriggure *)arguments;
#ifdef DEBUG
    printf("start new thread\n");
    printf("event len %d\n", etrigger->eventList.size());
#endif

    GreeterClient *greeter = GreeterClient::getClient();
    if (greeter == NULL)
    {
        printf("failed to get initialised greeter\n");
        return NULL;
    }

    string reply = greeter->Subscribe(etrigger->eventList);
    cout << "Subscribe return value: " << reply << endl;
    int i = 0;
    if (reply.compare("TRIGGERED") != 0)
    {
        printf("rpc failed, don't execute command:\n");
        int actionSize = etrigger->actionList.size();
        for (i = 0; i < actionSize; i++)
        {
            printf("%s\n", etrigger->actionList[i].data());
        }

        return 0;
    }

    //TODO
    //when trigureed, call the runtime function
    //(runtimeFunc)slurmTaskStart(path)


    int actionSize = etrigger->actionList.size();
    for (i = 0; i < actionSize; i++)
    {
        //TODO use different runtime according to the driver type
        //slurmTaskStart(etrigger->actionList[i].data());
        if (etrigger->driver.compare("local") == 0)
        {
            localTaskStart(etrigger->actionList[i].data());
        }
    }

    //send rpc request to back end

    /*
    //char subscribeList[500];
    for (int i = 0; i < etrigger->eventLen; i++)
    {
        sprintf(subscribeList, "%s %s", subscribeList, etrigger->eventList[i]);
    }

    //send subscribe to pubsub backend
    redisContext *redisc = redisInit();
    if (redisc == NULL)
    {
        printf("failed to start connection to redis backend\n");
        return NULL;
    }

    //register the triggered function when recieve the respond from subscribe api
    runtimeAction *ra = (runtimeAction *)malloc(sizeof(runtimeAction));
    ra->actionLen = etrigger->actionLen;
    for (int i = 0; i < ra->actionLen; i++)
    {
        strcpy(ra->actionList[i], etrigger->actionList[i]);
    }
    redisSubscribe(ra, redisc, subscribeList, (runtimeFunc)slurmTaskStart);

    //TODO finish this function when all the events have been unsubscribed
*/
    return NULL;
}

int jsonIfTriggerorOperator(Document &d, char *jsonbuffer)
{
    d.Parse(jsonbuffer);
    //printf("jsonIfTriggerorOperator (%s)\n",jsonbuffer);
    const char *type = d["type"].GetString();
    //printf("get type after parsing (%s)\n",type);
    if (strcmp(type, "TRIGGER") == 0)
    {
        //printf("TRIGGER type\n");
        return 1;
    }
    else if (strcmp(type, "OPERATOR") == 0)
    {
        //printf("OPERATOR type\n");
        return 2;
    }
    else
    {
        return -1;
    }
}

void jsonParsingTrigger(Document &d, char *jsonbuffer)
{
    //#ifdef DEBUG
    //    printf("debug json buffer%s\n",jsonbuffer);
    //#endif
    d.Parse(jsonbuffer);
    const char *type = d["type"].GetString();
    EventTriggure *triggure = new (EventTriggure);
    printf("----get type %s----\n", type);
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
        threadIdList.push_back(tid);
    }

    return;
}

void waitthreadFinish()
{
    int len = threadIdList.size();
    int i, joinReturn;
    for (i = 0; i < len; i++)
    {
        joinReturn = pthread_join(threadIdList[i], NULL);
        printf("thread id %d return %d\n", threadIdList[i], joinReturn);
    }
}