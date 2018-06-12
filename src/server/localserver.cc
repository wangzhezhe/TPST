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
#include <thread>
#define BILLION 1000000000L

mutex NotifiedNumMtx;
int localNotifiedNum = 0;

int localwaitTime = 500000;

void startAction(string clientID)
{

    //get map<string, EventTriggure *> clientIdtoConfig;
    struct timespec start, end1, end2;
    double diff;

    clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

    if (clientIdtoConfig.find(clientID) == clientIdtoConfig.end())
    {
        printf("failed to get eventtriggure from clientIdtoConfig\n");
        return;
    }
    EventTriggure *etrigger = clientIdtoConfig[clientID];

    int actionSize = etrigger->actionList.size();
    int i;

    clock_gettime(CLOCK_REALTIME, &end1); /* mark the end time */
    diff = (end1.tv_sec - start.tv_sec) * 1.0 + (end1.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    printf("debug for clientIdtoConfig time = (%lf) second\n", diff);
    //string command = string ("/bin/bash /home1/zw241/observerchain/tests/performance/app/analysis.sh sim.out > ana.out");
    string command = string ("echo abc");

    if (etrigger->driver.compare("local") == 0)
    {

        for (i = 0; i < actionSize; i++)
        {
            localTaskStart(etrigger->actionList[i].data());
            //printf("dummy execution\n");
            //thread{localTaskStart, command}.detach();
            //localTaskStart(command);
        }
    }

    clock_gettime(CLOCK_REALTIME, &end2); /* mark the end time */
    diff = (end2.tv_sec - end1.tv_sec) * 1.0 + (end2.tv_nsec - end1.tv_nsec) * 1.0 / BILLION;
    printf("debug for command execution time = (%lf) second\n", diff);

    return;
}

void localNotifyBack(string clientID)
{

    //printf("notify back for clientid %s\n", clientID.data());
    struct timespec start, end;
    double diff;
    //clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

    //thread{startAction, clientID}.detach();
    startAction(clientID);
    //clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
    //diff = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    //printf("debug for startAction response time = (%lf) second\n", diff);

    NotifiedNumMtx.lock();
    localNotifiedNum++;
    NotifiedNumMtx.unlock();
    //printf("local notify num %d\n",localNotifiedNum);

    return;
}

void *checkNotify(void *arguments)
{

    struct timespec start1, end1, end2;
    double diff;

    clock_gettime(CLOCK_REALTIME, &start1); /* mark start time */

    pubsubWrapper *psw = (pubsubWrapper *)arguments;
    string clientidstr = psw->clientID;
    int clientsize = 0;
    printf("start checkNotify for clientid (%s)\n", clientidstr.data());
    int times = 0;
    while (1)
    {
        //clock_gettime(CLOCK_REALTIME, &start2); /* mark start time */

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

            usleep(1 * (localwaitTime));
            times++;
            printf("checknotify %s sleep time %d\n", clientidstr.data(), times);
        }
    }

    clock_gettime(CLOCK_REALTIME, &end1); /* mark the end time */
    diff = (end1.tv_sec - start1.tv_sec) * 1.0 + (end1.tv_nsec - start1.tv_nsec) * 1.0 / BILLION;
    printf("debug for checknotify %s part1 response time = (%lf) second\n", clientidstr.data(), diff);

    localNotifyBack(clientidstr);
    //free memory
    //free(psw);

    clock_gettime(CLOCK_REALTIME, &end2); /* mark the end time */
    diff = (end2.tv_sec - end1.tv_sec) * 1.0 + (end2.tv_nsec - end1.tv_nsec) * 1.0 / BILLION;
    printf("debug for checknotify %s part2 response time = (%lf) second\n", clientidstr.data(), diff);
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