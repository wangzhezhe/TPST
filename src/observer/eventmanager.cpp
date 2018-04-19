
#include "stdlib.h"
#include "stdio.h"
#include "errno.h"
#include "string.h"
#include "pthread.h"

#include "eventmanager.h"

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

void* eventSubscribe(void *arguments){
    //only could be transfered by this way if original pointed is initiallises by malloc instead on new
    EventTriggure *etrigure = (EventTriggure *)arguments;
    printf("start new thread\n");
    printf("event len %d\n",etrigure->eventLen);

    //send subscribe to pubsub backend




    //if there are return value, start to triguring relevant actions


    return NULL;
}

void jsonParsing(Document &d, char *jsonbuffer)
{
    d.Parse(jsonbuffer);
    const char *type = d["type"].GetString();
    if (strcmp(type, "EVENT") == 0)
    {
        printf("process event trigure\n");

        //register trigure and send subscribe call to pub-sub backend

        EventTriggure *trigure = (EventTriggure *)malloc (sizeof(EventTriggure));
        const Value &eventList = d["eventList"];
        SizeType i;
        for (i = 0; i < eventList.Size(); i++)
        {
            const char *tempstr = eventList[i].GetString();
            printf("eventList[%d] = %s\n", i, tempstr);
            strcpy(trigure->eventList[i], tempstr);
        }
        trigure->eventLen = (unsigned int)i;

        const Value &actionList = d["actionList"];

        for (i = 0; i < actionList.Size(); i++)
        {
            const char *tempstr = actionList[i].GetString();
            printf("actionList[%d] = %s\n", i, tempstr);
            strcpy(trigure->actionList[i], tempstr);
        }
        trigure->actionLen = ( unsigned int)i;

        const char *driver = d["driver"].GetString();
        printf("driver:%s\n", driver);
        strcpy(trigure->driver, driver);

        //start a new thread to send the request to pub-sub backend

        //printf("eventlen %d actionlen %d\n",trigure->eventLen,trigure->actionLen);

        pthread_t tid;
        if (pthread_create(&tid, NULL, &eventSubscribe, (void *)trigure) != 0)
        {
            printf("fail to create pthread wirh id %d\n", (int)tid);
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