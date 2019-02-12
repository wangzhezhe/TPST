#include "pubsub.h"
#include "stdio.h"
#include "stdlib.h"
#include "string"
#include <omp.h>
#include "../utils/split/split.h"
#include "../utils/strparse/strparse.h"
#include "../../deps/spdlog/spdlog.h"

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

// TODO
// map the event into the indexed event in subtoClient map
// update this map after the subscription operation
// make sure insert position when publish by getIndexMap
mutex getIndexMtx;
map<string, set<string>> getIndexMap;

mutex publishedEventMtx;

using namespace std;

SimplepubsubWrapper *getSimplepubsubWrapper(pubsubWrapper *psw)
{
    SimplepubsubWrapper *temppsw = new (SimplepubsubWrapper);
    temppsw->clientID = psw->clientID;
    temppsw->subMetadata = psw->subMetadata;
    temppsw->peerURL = psw->peerURL;

    map<string, set<int>> requiredeventMap = psw->requiredeventMap;

    for (map<string, set<int>>::iterator it = requiredeventMap.begin(); it != requiredeventMap.end(); ++it)
    {
        string event = it->first;
        temppsw->eventList.push_back(event);
    }

    return temppsw;
}

void eventUnSubscribe(string event, string clientId)
{

    if (subtoClient.find(event) != subtoClient.end())
    {
        if (subtoClient[event].find(clientId) != subtoClient[event].end())
        {

            subtoClientMtx.lock();
            subtoClient[event].erase(clientId);
            subtoClientMtx.unlock();
        }
    }

    return;
}

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

void addNewClient(string clientid, string notifyAddr, vector<string> eventList, string matchType, string subMetadata)
{
    pubsubWrapper *psw = new (pubsubWrapper);
    psw->peerURL = notifyAddr;
    //range eventList
    map<string, int> publishedEvent;
    psw->publishedEvent = publishedEvent;
    psw->clientID = clientid;
    psw->matchType = matchType;
    psw->iftrigure = false;
    psw->subMetadata = subMetadata;
    int size = eventList.size();

    int requireNum;

    //use first event as the index
    string indexEvent;
    if (size > 0)
    {
        indexEvent = eventList[0];
    }
    else
    {
        printf("error: event list for addNewClient should larger than 0\n");
        return;
    }

    //printf("debug sub 3 new event key %s\n",indexEvent.data());

    string indexEventWithoutNum;
    ParseEvent(indexEvent, indexEventWithoutNum, requireNum);

    for (int i = 0; i < size; i++)
    {
        string eventWithoutNum;
        ParseEvent(eventList[i], eventWithoutNum, requireNum);
        psw->requiredeventMap[eventWithoutNum].insert(requireNum);

        getIndexMtx.lock();
        if (getIndexMap.find(eventWithoutNum) != getIndexMap.end())
        {
            getIndexMap[eventWithoutNum].insert(indexEventWithoutNum);
        }
        else
        {
            set<string> indexSet;
            indexSet.insert(indexEventWithoutNum);
            getIndexMap[eventWithoutNum] = indexSet;
        }

        getIndexMtx.unlock();
    }

    subtoClientMtx.lock();
    subtoClient[indexEventWithoutNum][clientid] = psw;
    subtoClientMtx.unlock();

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

    psw->publishedEvent.clear();
    return;
}

bool checkIfTriggureMetaGrid(pubsubWrapper *psw, string publishedMeta)
{
    printf("start checkIfTriggureMetaGrid\n");
    //extract the binding box from the publishedMeta
    vector<int> publb;
    vector<int> pubub;

    string pubGridMeta = getGridFromRawMeta(publishedMeta);

    getbbxfromGridMeta(pubGridMeta, publb, pubub);

    printf("check pub lb %d %d ub %d %d\n", publb[0], publb[1], pubub[0], pubub[1]);

    //extract the binding box from the subscribed data
    vector<int> sublb;

    vector<int> subub;

    printf("check psw sub metadata %s\n", psw->subMetadata.data());
    string submetastr = psw->subMetadata;
    int position = submetastr.find(":");
    //the format in psw is not correct
    if (position == submetastr.npos)
    {
        return false;
    }

    getbbxfromGridMeta(submetastr, sublb, subub);

    //compare two bingding box, to check if triggure

    //plbx publb[0]
    //plby publb[1]

    //pubx pubub[0]
    //puby pubub[1]

    //slbx sublb[0]
    //slby sublb[1]

    //subx subub[0]
    //suby subub[1]

    printf("check sub lb %d %d ub %d %d\n", sublb[0], sublb[1], subub[0], subub[1]);

    //refer to https://www.geeksforgeeks.org/find-two-rectangles-overlap/

    if (sublb[0] > pubub[0] || subub[0] < publb[0])
    {
        return false;
    }

    if (subub[1] < publb[1] || sublb[1] > pubub[1])
    {
        return false;
    }

    return true;
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

    //printf("debug range dynamicPublishedEvent\n");
    map<string, int>::iterator debugit;
    for (debugit = dynamicPublishedEvent.begin(); debugit != dynamicPublishedEvent.end(); ++debugit)
    {
        string publishedEvent = debugit->first;
        int times = debugit->second;

        //printf("published event %s times %d\n", publishedEvent.data(), times);
    }

    //if every required event is in publishedEvent, true, else false
    for (itmap = requiredeventMap.begin(); itmap != requiredeventMap.end(); ++itmap)
    {

        eventkeywithoutNum = itmap->first;
        set<int> eventNumSet = itmap->second;

        //printf("debug required event %s\n", eventkeywithoutNum.data());
        for (itset = eventNumSet.begin(); itset != eventNumSet.end(); ++itset)
        {
            requirePushNum = (*itset);
            if (dynamicPublishedEvent.find(eventkeywithoutNum) == dynamicPublishedEvent.end())
            {
                //event has not been published
                //printf("eventkeywithoutNum (%s) is not been published\n", eventkeywithoutNum.data());
                return false;
            }
            if (dynamicPublishedEvent[eventkeywithoutNum] != requirePushNum)
            {
                //event has not been published required times
                //printf("eventkeywithoutNum times %d is not required %d\n", dynamicPublishedEvent[eventkeywithoutNum], requirePushNum);

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
void pubsubSubscribe(vector<string> eventList, string clientId, string notifyAddr, string matchType, string subMetadata)
{
    addNewClient(clientId, notifyAddr, eventList, matchType, subMetadata);
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

void outputsubtoClient()
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

void pubsubPublish(vector<string> eventList, string matchType, string metadata)
{

    int size = eventList.size();
    int i;
    //printf("eventList len %d\n", size);
    string eventwithoutNum;
    for (i = 0; i < size; i++)
    {
        //this event should not in full format, for required number larger than 1, one event should only binding with one number
        eventwithoutNum = eventList[i];

        //if there is no eventwithoutNum in map, return

        if (getIndexMap.find(eventwithoutNum) == getIndexMap.end())
        {
            //event not been registered
            continue;
        }

        //if (ifdebug)
        //{
        //    printf("%s debug0\n", eventwithoutNum.data());
        //}

        //range the index set firstly
        getIndexMtx.lock();
        set<string> indexEventSet = getIndexMap[eventwithoutNum];
        getIndexMtx.unlock();

        //if (ifdebug)
        //{
        //    printf("%s debug1\n", eventwithoutNum.data());
        //}

        for (set<string>::iterator it = indexEventSet.begin(); it != indexEventSet.end(); ++it)
        {

            string indexEvent = *it;

            //printf("debug getIndexMap eventwithoutNum %s indexEvent %s\n", eventwithoutNum.data(), indexEvent.data());

            //if (ifdebug)
            //{
            //    printf("%s debug2\n", eventwithoutNum.data());
            //}

            subtoClientMtx.lock();
            if (subtoClient.find(indexEvent) == subtoClient.end())
            {
                printf("error: indexEvent %s is not in subtoClient\n", indexEvent.data());
                subtoClientMtx.unlock();
                return;
            }
            subtoClientMtx.unlock();

            //if (ifdebug)
            //{
            //    printf("%s debug3\n", eventwithoutNum.data());
            //}

            //traverse map
            subtoClientMtx.lock();
            map<string, pubsubWrapper *> innermap = subtoClient[indexEvent];
            subtoClientMtx.unlock();

            //if (ifdebug)
            //{
            //    printf("%s debug4\n", eventwithoutNum.data());
            //}

            map<string, pubsubWrapper *>::iterator itmap;

            //printf("number for clientset %d when publish event %s\n", setnum, eventwithoutNum.data());
            //printf("debug publish event %s map size %d\n", eventwithoutNum.data(), clientMap.size());
            //clock_gettime(CLOCK_REALTIME, &end1);
            //diff = (end1.tv_sec - start.tv_sec) * 1.0 + (end1.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
            //printf("debug for publish end1 response time = (%lf) second\n", diff);
            //traverse the map and put publish into it
            for (itmap = innermap.begin(); itmap != innermap.end(); ++itmap)
            {

                //using openmp here, there is do data depedency between every clients

                int tid;

                string clientId = itmap->first;
                pubsubWrapper *clientWrapper = itmap->second;
                bool tempiftrigure = false;

                //if (ifdebug)
                //{
                //    printf("%s debug5\n", eventwithoutNum.data());
                //}

                if (matchType.compare("NAME") == 0)
                {
                    if (clientWrapper->publishedEvent.find(eventwithoutNum) == clientWrapper->publishedEvent.end())
                    {

                        clientWrapper->publishedEvent[eventwithoutNum] = 0;
                    }

                    clientWrapper->publishedEvent[eventwithoutNum]++;

                    //printf("insert eventwithoutNum %s into publishedEvent with number %d\n", eventwithoutNum.data(), clientWrapper->publishedEvent[eventwithoutNum]);

                    tempiftrigure = checkIfTriggure(clientWrapper);
                }
                else if (matchType.compare("META_GRID") == 0)
                {
                    tempiftrigure = checkIfTriggureMetaGrid(clientWrapper, metadata);
                }
                else
                {
                    spdlog::debug("unsuported match type {}", matchType.data());

                }

                //if (ifdebug)
                //{
                //    printf("%s debug6\n", eventwithoutNum.data());
                //}

                if (tempiftrigure == true)
                {
                    //modify metadata only when need to notify
                    //TODO update the metadata storing published events
                    //if there are multiple pub for same key in small range of time
                    //some of metadata will missed
                    spdlog::debug("tempiftrigure is true for indexEvent {}", indexEvent.data());
                    subtoClientMtx.lock();
                    subtoClient[indexEvent][clientId]->pubMetadata = metadata;
                    subtoClient[indexEvent][clientId]->iftrigure = tempiftrigure;
                    subtoClientMtx.unlock();
                    //printf("check iftrigure event %s indexEvent %s bool %d\n", eventwithoutNum.data(), indexEvent.data(), subtoClient[indexEvent][clientId]->iftrigure);
                }

                //if (ifdebug)
                //{
                //    printf("%s debug7\n", eventwithoutNum.data());
                //}
            }
        }
    }
}