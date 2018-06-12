#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>
#include <uuid/uuid.h>

#include "pubsub.h"
#include "unistd.h"
#include <mutex>
#include "../utils/getip/getip.h"
#include "../observer/eventmanager.h"
#include "../publishclient/pubsubclient.h"
#include "../runtime/local.h"

#include <stdint.h> /* for uint64 definition */
#include <stdlib.h> /* for exit() definition */
#include <time.h>   /* for clock_gettime */
#define BILLION 1000000000L

mutex NotifiedNumMtx;
int localNotifiedNum = 0;

int localwaitTime = 1000;

void startAction(string clientID)
{

    //get map<string, EventTriggure *> clientIdtoConfig;

    if (clientIdtoConfig.find(clientID) == clientIdtoConfig.end())
    {
        printf("failed to get eventtriggure from clientIdtoConfig\n");
        return;
    }
    EventTriggure *etrigger = clientIdtoConfig[clientID];

    int actionSize = etrigger->actionList.size();
    int i;
    for (i = 0; i < actionSize; i++)
    {
        if (etrigger->driver.compare("local") == 0)
        {
            localTaskStart(etrigger->actionList[i].data());
        }
    }
    return;
}

void localNotifyBack(string clientID)
{

    //printf("notify back for clientid %s\n", clientID.data());
    struct timespec start, end;
    double diff;
    clock_gettime(CLOCK_REALTIME, &start); /* mark start time */
    startAction(clientID);
    clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
    diff = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    printf("debug for startAction response time = (%lf) second\n", diff);

    //NotifiedNumMtx.lock();
    localNotifiedNum++;
    //NotifiedNumMtx.unlock();

    return;
}

void *checkNotify(void *arguments)
{

    struct timespec start, end, end0;
    double diff;

    clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

    pubsubWrapper *psw = (pubsubWrapper *)arguments;
    string clientidstr = psw->clientID;
    int clientsize = 0;
    printf("start checkNotify for clientid (%s)\n", clientidstr.data());
    while (1)
    {

        bool notifyFlag = checkIfTriggure(psw);

        //printf("notifyflag for client id %s (%d)\n",clientId.data(),notifyFlag);
        if (notifyFlag == true)
        {

            break;
        }
        else
        {

            srand((unsigned)time(0));
            int random_integer = (rand() % localwaitTime) + 1;

            usleep(1 * (localwaitTime + random_integer));
        }
    }
    clock_gettime(CLOCK_REALTIME, &end0); /* mark the end time */
    diff = (end0.tv_sec - start.tv_sec) * 1.0 + (end0.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    printf("debug for checknotify %s part1 response time = (%lf) second\n", clientidstr.data(),diff);

    //printf("notify back (%s)\n", clientidstr.data());

    localNotifyBack(clientidstr);
    //free memory
    //free(psw);

    clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
    diff = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    printf("debug for checknotify %s part2 response time = (%lf) second\n", clientidstr.data(),diff);
}

void startNotifySequence()
{
    string eventwithoutNum = string("INIT");
    map<string, pubsubWrapper *>::iterator itmap;
    map<string, pubsubWrapper *> innermap = subtoClient[eventwithoutNum];
    for (itmap = innermap.begin(); itmap != innermap.end(); ++itmap)
    {
        string clientid = itmap->first;
        pubsubWrapper *psw = itmap->second;
        checkNotify((void *)psw);
    }

    return;
}

void startNotify(string eventwithoutNum, string clientID)
{
    pthread_t id;
    pubsubWrapper *psw = subtoClient[eventwithoutNum][clientID];
    pthread_create(&id, NULL, checkNotify, (void *)psw);
}

void localSubscribe(string clientId, vector<string> eventList)
{
    struct timespec start, end;
    double diff;

    clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

    //replace the port here the server port for notify is 50052

    pubsubSubscribe(eventList, clientId, "dummynotifyback");
    int size = eventList.size();
    int i;
    string eventStr;
    for (i = 0; i < size; i++)
    {
        int trinum = 1;
        string eventMessage;
        eventStr = eventList[i];
        ParseEvent(eventStr, eventMessage, trinum);
        printf("after parsing %s %d\n", eventMessage.data(), trinum);
        startNotify(eventMessage, clientId);
    }

    //clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
    //diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    //printf("debug for subevent stage2 response time = (%llu) second\n", (long long unsigned int)diff);

    clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
    diff = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    printf("debug for subevent response time = (%lf) second\n", diff);

    printf("local subscribe for id %s finish\n", clientId.data());

    return;
}

void localPublish(vector<string> eventList)
{
    pubsubPublish(eventList);
    return;
}