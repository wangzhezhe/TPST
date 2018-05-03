
#include "stdlib.h"
#include "stdio.h"
#include "errno.h"
#include "string.h"
#include "pthread.h"

#include "eventmanager.h"
#include "../redisclient/redisclient.h"
#include "../runtime/slurm.h"

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


void *eventSubscribe(void *arguments)
{
    //only could be transfered by this way if original pointed is initiallises by malloc instead on new
    EventTriggure *etrigure = (EventTriggure *)arguments;
#ifdef DEBUG
    printf("start new thread\n");
    printf("event len %d\n", etrigure->eventLen);
#endif
    
    char subscribeList[500];

    for (int i = 0; i < etrigure->eventLen; i++)
    {
        sprintf(subscribeList, "%s %s", subscribeList, etrigure->eventList[i]);
    }

    //send subscribe to pubsub backend
    redisContext *redisc = redisInit();
    if (redisc == NULL)
    {
        printf("failed to start connection to redis backend\n");
        return NULL;
    }
    
    //register the trigure function when recieve the respond from subscribe api
    runtimeAction *ra=(runtimeAction *)malloc(sizeof(runtimeAction));
    ra->actionLen=etrigure->actionLen;
    for(int i=0;i<ra->actionLen;i++){
        strcpy(ra->actionList[i],etrigure->actionList[i]);
    }
    redisSubscribe(ra, redisc, subscribeList,(runtimeFunc)slurmTaskStart);

    //TODO finish this function when all the events have been unsubscribed

    return NULL;
}

int jsonIfTrigger(Document &d, char *jsonbuffer){
    d.Parse(jsonbuffer);
    const char *type = d["type"].GetString();
    if (strcmp(type, "TRIGGER") == 0)
    {
        return 1;
    }else{
        return 0;
    }
    
}

void jsonParsingTrigger(Document &d, char *jsonbuffer)
{
//#ifdef DEBUG
//    printf("debug json buffer%s\n",jsonbuffer);
//#endif
    d.Parse(jsonbuffer);
    const char *type = d["type"].GetString();
    if (strcmp(type, "TRIGGER") == 0)
    {
#ifdef DEBUG
        printf("process event trigure\n");
#endif
        //register trigure and send subscribe call to pub-sub backend

        EventTriggure *trigure = (EventTriggure *)malloc(sizeof(EventTriggure));
        const Value &eventList = d["eventList"];
        SizeType i;
        for (i = 0; i < eventList.Size(); i++)
        {
            const char *tempstr = eventList[i].GetString();
#ifdef DEBUG
            printf("eventList[%d] = %s\n", i, tempstr);
#endif
            strcpy(trigure->eventList[i], tempstr);
        }
        trigure->eventLen = (unsigned int)i;

        const Value &actionList = d["actionList"];

        for (i = 0; i < actionList.Size(); i++)
        {
            const char *tempstr = actionList[i].GetString();
#ifdef DEBUG
            printf("actionList[%d] = %s\n", i, tempstr);
#endif
            strcpy(trigure->actionList[i], tempstr);
        }
        trigure->actionLen = (unsigned int)i;

        const char *driver = d["driver"].GetString();
#ifdef DEBUG
        printf("driver:%s\n", driver);
#endif
        strcpy(trigure->driver, driver);

        //start a new thread to send the request to pub-sub backend

        //printf("eventlen %d actionlen %d\n",trigure->eventLen,trigure->actionLen);

        pthread_t tid;
        if (pthread_create(&tid, NULL, &eventSubscribe, (void *)trigure) != 0)
        {
#ifdef DEBUG
            printf("fail to create pthread wirh id %d\n", (int)tid);
#endif
            return;
        }
    }
    else if (strcmp(type, "OPERATOR") == 0)
    {
        printf("process operator\n");
    }
    else
    {
        printf("unsupported type\n");
    }
    return;
}