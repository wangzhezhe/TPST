#include "pubsub.h"
#include "stdio.h"
#include "stdlib.h"
#include "string"
#include <omp.h>
#include "../utils/split/split.h"
#include <time.h>
#include <unistd.h>

#define BILLION 1000000000L

//event string to pubsubevent (value is real element with memory) from eventstring to eventStructure
//this map store the static event
//the dynamic event store in the innermap of clienttoSub, the value represent the number of event published
//map<int, bool> map the required trigguring time of specific event into if it is satisfied
//for example, event1:1 represent the event1 needed to be published 1 time, the init value is false for this inner map

//mutex clientidtoWrapperMtx;
//map<string, pubsubWrapper*> clientidtoWrapper;

// to pubsubWrapperid (value is pointer) from subeventstring to set of clientid
// the key here is not in full format
mutex subtoClientMtx;
//map<event, map<clientid,pubsubwrapper*>>
map<string, map<string, pubsubWrapper *>> subtoClient;

mutex publishedEventMtx;

using namespace std;

int getSubscribedClientsNumber(vector<string> subEventList)
{
    //printf("subevent %s\n", subEvent.data());
    //printf("debug1 size of subtoClient[INIT] %d\n", subtoClient[subEvent].size());
    //printf("debug2 size of subtoClient[INIT] %d\n", subtoClient["INIT"].size());

    int size = subEventList.size();

    int i;
    int totalSubNum = 0;

    for (i = 0; i < size; i++)
    {
        string subEvent = subEventList[i];
        if (subtoClient.find(subEvent) == subtoClient.end())
        {
            // not found
            continue;
        }

        //if client id exist
        //there is a map associated with every event
        totalSubNum = totalSubNum + subtoClient[subEvent].size();
    }

    return totalSubNum;
}

void addNewClientLocal(string clientid, vector<string> eventList)
{

    pubsubWrapper *psw = new (pubsubWrapper);
    //this value should acquire from meta data
    psw->peerURL = "";
    psw->clientID = clientid;
    psw->iftrigure = false;
    //publishedEvent is defaut value
    //range eventList
    psw->clientID = clientid;
    int size = eventList.size();

    int i = 0;
    string eventWithoutNum;
    int requireNum;
    for (i = 0; i < size; i++)
    {
        ParseEvent(eventList[i], eventWithoutNum, requireNum);
        subtoClientMtx.lock();
        subtoClient[eventWithoutNum][clientid] = psw;
        subtoClientMtx.unlock();
        psw->requiredeventMap[eventWithoutNum].insert(requireNum);
        
    }
    return;
}

void addNewClient(string clientid, string notifyAddr, vector<string> eventList)
{
    pubsubWrapper *psw = new (pubsubWrapper);
    psw->peerURL = notifyAddr;
    //range eventList
    map<string, int> publishedEvent;
    psw->publishedEvent = publishedEvent;
    psw->clientID = clientid;
    psw->iftrigure = false;
    int size = eventList.size();

    int i = 0;
    string eventWithoutNum;
    int requireNum;
    for (i = 0; i < size; i++)
    {
        ParseEvent(eventList[i], eventWithoutNum, requireNum);
        subtoClientMtx.lock();
        subtoClient[eventWithoutNum][clientid] = psw;
        subtoClientMtx.unlock();
        psw->requiredeventMap[eventWithoutNum].insert(requireNum);
        
    }

    return;
}

void deleteClient(string subevent, string clientid)
{
    printf("delete subevent %s\n", subevent.data());
    subtoClientMtx.lock();
    subtoClient[subevent].erase(clientid);
    subtoClientMtx.unlock();
    return;
}

void deletePubEvent(pubsubWrapper *psw)
{

    publishedEventMtx.lock();
    psw->publishedEvent.clear();
    publishedEventMtx.unlock();
    return;
}

//the psw should associated with the specific client id
bool checkIfTriggure(pubsubWrapper *psw)
{

    // check all associated event to make sure if the client is triggured
    bool notifyFlag = true;

    //traverse the requiredMap (get this from psw->requiredeventMap)

    map<string, set<int>> requiredeventMap = psw->requiredeventMap;
    map<string, set<int>>::iterator itmap;
    set<int>::iterator itset;
    int requirePushNum;
    string eventkeywithoutNum;
    //get the dynamicPublishedMap
    map<string, int> dynamicPublishedEvent = psw->publishedEvent;

    //if every required event is in publishedEvent, true, else false
    for (itmap = requiredeventMap.begin(); itmap != requiredeventMap.end(); ++itmap)
    {

        eventkeywithoutNum = itmap->first;
        set<int> eventNumSet = itmap->second;
        for (itset = eventNumSet.begin(); itset != eventNumSet.end(); ++itset)
        {
            requirePushNum = (*itset);
            if (dynamicPublishedEvent.find(eventkeywithoutNum) == dynamicPublishedEvent.end())
            {
                //event has not been published
                return false;
            }
            if (dynamicPublishedEvent[eventkeywithoutNum] != requirePushNum)
            {
                //event has not been published required times
                return false;
            }

            //for testing using
            //satisfiedStr = eventkeywithoutNum;
        }
    }

    return notifyFlag;
}

//the event here is in full format such as: event1:1, event2:2
//when subscribe, only one of same event type could exist, for example event1:2 and event1:3 coube not be subscribed at the same time
//TODO return error when same event has been subscribed with multiple number
void pubsubSubscribe(vector<string> eventList, string clientId, string notifyAddr)
{
    addNewClient(clientId, notifyAddr, eventList);
}

void ParseEvent(string fullEvent, string &eventMessage, int &num)
{

    //split eventmessage:requiredNum
    vector<string> v = split(fullEvent, ":");
    int s = v.size();
    if (s == 1)
    {
        eventMessage = v[0];
        //default value is subscribing one time
        num = 1;
    }
    else if (s == 2)
    {
        eventMessage = v[0];
        num = stoi(v[1]);
    }
    else
    {
        eventMessage = v[0];
        num = stoi(v[1]);
        printf("event %s should be the format of message:num \n", fullEvent.data());
    }

    return;
}

void output()
{
    //output subtoCliet
    printf("---------current inner data value----------\n");
    map<string, map<string, pubsubWrapper *>>::iterator itera;
    map<string, set<int>>::iterator iterrq;
    set<int>::iterator itset;
    for (itera = subtoClient.begin(); itera != subtoClient.end(); ++itera)
    {
        //parse the event key
        string eventwithoutNum = itera->first;

        printf("debug output eventwithoutNum %s\n", eventwithoutNum.data());

        map<string, pubsubWrapper *> innermap = itera->second;
        map<string, pubsubWrapper *>::iterator itmap;
        for (itmap = innermap.begin(); itmap != innermap.end(); ++itmap)
        {
            string clientId = itmap->first;
            pubsubWrapper *pwd = itmap->second;
            printf("client id of pubsubWrapper value (%s)\n", clientId.data());

            map<string, set<int>> requiredeventMap = pwd->requiredeventMap;

            for (iterrq = requiredeventMap.begin(); iterrq != requiredeventMap.end(); ++iterrq)
            {
                string reqstr = iterrq->first;
                set<int> num = iterrq->second;
                for (itset = requiredeventMap[reqstr].begin(); itset != requiredeventMap[reqstr].end(); ++itset)
                {
                    int num = (*itset);
                    printf("subEvent (%s) required num (%d)\n", reqstr.data(), num);
                }
            }

            map<string, int> publishedEvent = pwd->publishedEvent;
            //printf("size of publishend evetList %d\n", publishedEvent.size());
            map<string, int>::iterator itmap;
            for (itmap = publishedEvent.begin(); itmap != publishedEvent.end(); ++itmap)
            {
                string publishedEventStr = itmap->first;
                int publishNum = itmap->second;
                printf("client id %s publish %s for %d times\n", clientId.data(), publishedEventStr.data(), publishNum);
            }
        }
    }
}

void pubsubPublish(vector<string> eventList, string metadata)
{

    struct timespec start, end1, end2;
    double diff;

    //clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

    int size = eventList.size();
    int i;
    //printf("eventList len %d\n", size);
    string eventwithoutNum;
    for (i = 0; i < size; i++)
    {
        //this event should not in full format, for required number larger than 1, one event should only binding with one number
        eventwithoutNum = eventList[i];

        //if there is no eventwithoutNum in map, return

        if (subtoClient.find(eventwithoutNum) == subtoClient.end())
        {
            return;
        }
        subtoClientMtx.lock();
        map<string, pubsubWrapper *> clientMap = subtoClient[eventwithoutNum];
        subtoClientMtx.unlock();
        //traverse map
        map<string, pubsubWrapper *>::iterator itmap;

        //printf("number for clientset %d when publish event %s\n", setnum, eventwithoutNum.data());
        //printf("debug publish event %s map size %d\n", eventwithoutNum.data(), clientMap.size());
        //clock_gettime(CLOCK_REALTIME, &end1);
        //diff = (end1.tv_sec - start.tv_sec) * 1.0 + (end1.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
        //printf("debug for publish end1 response time = (%lf) second\n", diff);
        //traverse the map and put publish into it
        for (itmap = clientMap.begin(); itmap != clientMap.end(); ++itmap)
        {

            //using openmp here, there is do data depedency between every clients

            int tid;

            //#pragma omp parallel
            //            {

            //#pragma omp single nowait private(tid)
            //{
            //test if specific element in set exist in clientidtoWrapper
            string clientId = itmap->first;
            pubsubWrapper *clientWrapper = itmap->second;
            //printf("debug push clientid %s\n", clientId.data());
            //tid = omp_get_thread_num();
            //this eventmap is a small one
            map<string, int> publishedEvent = clientWrapper->publishedEvent;

            if (publishedEvent.find(eventwithoutNum) == publishedEvent.end())
            {
                publishedEvent[eventwithoutNum] = 0;
            }

            publishedEvent[eventwithoutNum]++;

            clientWrapper->publishedEvent = publishedEvent;

            //check if notify here
            //TODO get a new thread from the thread pool to check this asynchronously
            //TODO attention the case, use number to label the published times, decrease the number after every notification
            //Becasue notify will only happens when there is publishing events
            bool tempiftrigure = checkIfTriggure(clientWrapper);
            if (tempiftrigure == true)
            {
                //modify metadata only when need to notify
                clientMap[clientId]->metadata = metadata;

                clientMap[clientId]->iftrigure = tempiftrigure;
                //printf("check iftrigure event %s bool %d\n", eventwithoutNum.data(), clientMap[clientId]->iftrigure);
            }
        }
    }

    //clock_gettime(CLOCK_REALTIME, &end2);
    //diff = (end2.tv_sec - start.tv_sec) * 1.0 + (end2.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    //printf("debug for publish (%s) response time = (%lf) second\n", eventwithoutNum.data(), diff);
}