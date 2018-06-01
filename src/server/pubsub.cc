#include "pubsub.h"
#include "stdio.h"
#include "stdlib.h"
#include "string"
#include <omp.h>
#include "../utils/split/split.h"

//event string to pubsubevent (value is real element with memory) from eventstring to eventStructure
//this map store the static event
//the dynamic event store in the innermap of clienttoSub, the value represent the number of event published
//map<int, bool> map the required trigguring time of specific event into if it is satisfied
//for example, event1:1 represent the event1 needed to be published 1 time, the init value is false for this inner map
mutex strtoEventMtx;
map<string, map<int, bool>> strtoEvent;

//client id to pubsubWrapper(value is real element with memory) from clientid to clientStructure
mutex clientidtoWrapperMtx;
map<string, pubsubWrapper *> clientidtoWrapper;

// to pubsubWrapperid (value is pointer) from subeventstring to set of clientid
// the key here is full form insluding the publish number for example event1:1
mutex subtoClientMtx;
map<string, set<string>> subtoClient;

// clientid to pubsubEvent (value is pointer) from clientid to map of subscribedEvent
// in the inner map, the integer represent the time that the event have been pushed
map<string, map<string, int>> clienttoSub;

//strtoEvent and clientidtoWrapper have been established before this function
using namespace std;

void addNewClient(string clientid)
{
    pubsubWrapper *psw = new (pubsubWrapper);
    psw->iftrigure = false;
    clientidtoWrapperMtx.lock();
    clientidtoWrapper[clientid] = psw;
    clientidtoWrapperMtx.unlock();
}

void deleteClient(string clientid)
{
    clientidtoWrapperMtx.lock();
    clientidtoWrapper.erase(clientid);
    clientidtoWrapperMtx.unlock();
}
void deleteClientFromSTC(string clientid, string substr)
{

    subtoClientMtx.lock();
    subtoClient[substr].erase(clientid);
    subtoClientMtx.unlock();
}

void addNewEvent(string str, int num)
{
    if (strtoEvent.find(str) == strtoEvent.end())
    {
        // not found
        // add new innermap
        map<int, bool> innermap;
        innermap[num] = false;
        strtoEventMtx.lock();
        printf("add event %s %d\n", str.data(), num);
        strtoEvent[str] = innermap;
        strtoEventMtx.unlock();
    }
    else
    {
        if (strtoEvent[str].find(num) == strtoEvent[str].end())
        {
            //not found
            strtoEvent[str][num] = false;
        }
        else
        {
            //found, this event have already been added
            printf("event (%s:%d) have already been registered into the global event map\n", str.data(), num);
        }
    }
}
void deleteEvent(string str, int num)
{
    if (strtoEvent.find(str) == strtoEvent.end())
    {
        // not found, event is already deleted
        printf("event with prefix (%s) have already been deleted\n", str.data());
    }
    else
    {
        if (strtoEvent[str].find(num) == strtoEvent[str].end())
        {
            //not found
            printf("event (%s:%d) have already been deleted\n", str.data(), num);
        }
        else
        {
            //found
            strtoEventMtx.lock();
            strtoEvent[str].erase(num);
            strtoEventMtx.unlock();
        }
    }
}

//the event here is in full format such as: event1:1, event2:2
//when subscribe, only one of same event type could exist, for example event1:2 and event1:3 coube not be subscribed at the same time
//TODO return error when same event has been subscribed with multiple number
void pubsubSubscribe(vector<string> eventList, string clientId)
{

    int size = eventList.size();
    int i = 0;

    //put value into clienttoSub and subtoCliet

    for (i = 0; i < size; i++)
    {
        string eventWithoutNum;
        int requireNum;

        ParseEvent(eventList[i], eventWithoutNum, requireNum);

        if (clienttoSub.find(clientId) == clienttoSub.end())
        {
            // not found
            map<string, int> innermap;
            innermap[eventWithoutNum] = 0;
            //used to store dynamic pushed number
            clienttoSub[clientId] = innermap;
        }
        else
        {
            if (clienttoSub[clientId].find(eventWithoutNum) == clienttoSub[clientId].end())
            {
                // not found
                clienttoSub[clientId][eventWithoutNum] = 0;
            }
            else
            {
                // found(already exist)
                // do nothing
            }
        }

        //put value into subtoCliet
        if (subtoClient.find(eventWithoutNum) == subtoClient.end())
        {
            // not found
            set<string> s;
            s.insert(clientId);
            subtoClient[eventWithoutNum] = s;
        }
        else
        {
            //if client id exist
            subtoClient[eventWithoutNum].insert(clientId);
        }
    }
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
    map<string, set<string>>::iterator itera;
    map<int, bool>::iterator inneritea;
    for (itera = subtoClient.begin(); itera != subtoClient.end(); ++itera)
    {
        //parse the event key
        string fullEvent = itera->first;
        string eventMessage;
        int num;
        ParseEvent(fullEvent, eventMessage, num);

        //traverse the innermap of strtoEvent
        map<int, bool> staticNumber = strtoEvent[eventMessage];

        for (inneritea = staticNumber.begin(); inneritea != staticNumber.end(); ++inneritea)
        {
            int requiredNum = inneritea->first;
            printf("sub %s required num %d\n", eventMessage.data(), requiredNum);
        }

        set<string> s = itera->second;
        set<string>::iterator itset;
        for (itset = s.begin(); itset != s.end(); ++itset)
        {
            string str = *itset;
            printf("set value (%s)\n", str.data());
        }
    }

    map<string, map<string, int>>::iterator iterb;

    for (iterb = clienttoSub.begin(); iterb != clienttoSub.end(); ++iterb)
    {
        cout << "clienttoSub key: " << iterb->first << endl;
        map<string, int> psemap = iterb->second;
        map<string, int>::iterator itset;
        for (itset = psemap.begin(); itset != psemap.end(); ++itset)
        {
            string key = itset->first;
            int pushnum = itset->second;
            printf("set value (%s) trigurenum (%d)\n", key.data(), pushnum);
        }
    }
}

void pubsubPublish(vector<string> eventList)
{

    int size = eventList.size();
    int i;
    for (i = 0; i < size; i++)
    {
        //this event should not in full format, for required number larger than 1, one event should only binding with one number
        string eventwithoutNum = eventList[i];

        int currentEventReqNum;

        //get require number

        //get RequireTriggure Map from strtoEvent
        map<int, bool> RequireTriggureMap;

        if (strtoEvent.find(eventwithoutNum) == strtoEvent.end())
        {
            // not found
            // do nothing
            printf("failed to get event %s from strtoEvent\n", eventwithoutNum.data());
            continue;
        }
        else
        {
            RequireTriggureMap = strtoEvent[eventwithoutNum];
            //range RequireTriggureMap to get currentEventReqNum
            map<int, bool>::iterator itinnermap;
            for (itinnermap = RequireTriggureMap.begin(); itinnermap != RequireTriggureMap.end(); ++itinnermap)
            {
                currentEventReqNum = itinnermap->first;
            }
        }

        printf("parse event (%s) required num (%d)\n", eventwithoutNum.data(), currentEventReqNum);

        string eventWithNum = eventwithoutNum + string(":") + to_string(currentEventReqNum);

        printf("event with num (%s)\n", eventWithNum.data());

        //get clientSet

        //use full format

        set<string> clientSet = subtoClient[eventwithoutNum];
        //traverse set
        set<string>::iterator itset;

        int setnum = clientSet.size();
        printf("number for clientset %d when publish event %s\n", setnum, eventwithoutNum.data());
        for (itset = clientSet.begin(); itset != clientSet.end(); ++itset)
        {

            //test if specific element in set exist in clientidtoWrapper
            string clientid = (*itset);
            int tid;
#pragma omp parallel
            {

#pragma omp single nowait private(tid)
                {
                    tid = omp_get_thread_num();
                    printf("threadid %d for client %s\n", tid, clientid.data());

                    //if not exist, delete this string
                    if (clientidtoWrapper.find(clientid) == clientidtoWrapper.end())
                    {
                        //delete this element in set
                        //the value have been already deleted in idtowrapper
                        //#pragma omp critical
                        //{
                            clientSet.erase(clientid);
                        //}
                       
                        //continue;
                    }else{
                        //clientId is valid
                        //get  map<string, int> dynamicEventPushMap;
                        int newPublishTime;
                        if (clienttoSub.find(clientid) == clienttoSub.end())
                        {
                            // not found
                            // do nothing
                            printf("failed to get dynamicEventPushMap from clienttoSub by clientId %s\n", clientid.data());
                            //continue;
                        }
                        else
                        {

                            if (clienttoSub[clientid].find(eventwithoutNum) == clienttoSub[clientid].end())
                            {
                                //new comming event
                                //#pragma omp critical
                                //{
                                    clienttoSub[clientid][eventwithoutNum] = 0;
                                //}
                              
                            }
                            else
                            {
                                //#pragma omp critical
                                //{
                                clienttoSub[clientid][eventwithoutNum]++;
                                //}
                                newPublishTime = clienttoSub[clientid][eventwithoutNum];
                            }
                        }

                        if (RequireTriggureMap.find(newPublishTime) == RequireTriggureMap.end())
                        {
                            //not exist
                            //continue;
                        }
                        else
                        {
                            strtoEventMtx.lock();
                            RequireTriggureMap[newPublishTime] = true;
                            strtoEventMtx.unlock();
                        }
                        map<string, int> dynamicEventPushMap = clienttoSub[clientid];
                        bool notifyFlag = true;

                        map<string, int>::iterator itsetsub;

                        for (itsetsub = dynamicEventPushMap.begin(); itsetsub != dynamicEventPushMap.end(); ++itsetsub)
                        {

                            string eventkey = itsetsub->first;
                            int pushNum = itsetsub->second;

                            if (RequireTriggureMap.find(pushNum) == RequireTriggureMap.end())
                            {
                                //event is not subscribed
                                printf("event (%s) (%d) is not subscribed\n", eventkey.data(), pushNum);
                                notifyFlag = false;
                                break;
                            }
                            else
                            {
                                if (RequireTriggureMap[pushNum] == false)
                                {
                                    notifyFlag = false;
                                    break;
                                }
                            }
                        }

                        if (notifyFlag == true)
                        {
                            printf("trigure/notify curr id (%s)\n", clientid.data());

                            //modify the global satisfied label to true
                            //clientidtoWrapperMtx.lock();
                            #pragma omp critical
                            {
                                clientidtoWrapper[clientid]->iftrigure = true;
                            }               
                            //clientidtoWrapperMtx.unlock();
                            //the value should be zero after trigguring operation
                            for (itsetsub = dynamicEventPushMap.begin(); itsetsub != dynamicEventPushMap.end(); ++itsetsub)
                            {

                                string eventkey = itsetsub->first;
                                clienttoSub[clientid][eventkey] = 0;
                            }
                        }
                    }
                }
            }
        }
    }
}

/*
            pubsubEvent *staticpse = strtoEvent[currpushEvent];
            if (staticpse == NULL)
            {
                printf("failed to get event (%s) from strtoEvent\n", currpushEvent.data());
            }
            //if clientid exist
            set<string> s = subtoClient[currpushEvent];
            //traverse set
            set<string>::iterator itset;
            for (itset = s.begin(); itset != s.end(); ++itset)
            {
                //test if specific element in set exist in clientidtoWrapper
                string clientid = (*itset);

                //if not exist, delete this string
                if (clientidtoWrapper.find(clientid) == clientidtoWrapper.end())
                {
                    //delete this element in set
                    //the value have been already deleted in idtowrapper
                    s.erase(clientid);
                    continue;
                }
                else
                {

                    if (clienttoSub.find(clientid) == clienttoSub.end())
                    {
                        printf("error, failed to find id %s\n in client to sub\n", clientid.data());
                    }
                    else
                    {
                        //else
                        //find the value in clienttoSub event and triggure num ++
                        if (clienttoSub[clientid].find(currpushEvent) == clienttoSub[clientid].end())
                        {
                            //new comming event
                            clienttoSub[clientid][currpushEvent] = 0;
                        }
                        else
                        {
                            clienttoSub[clientid][currpushEvent]++;
                        }

                        if (clienttoSub[clientid][currpushEvent] >= staticpse->trigureNum)
                        {

                            //traverse map, use flag to control if notify

                            int notifyFlag = 1;
                            map<string, int> psemap = clienttoSub[clientid];
                            psemap[staticpse->event]++;
                            //size of this set is usually small, it's ok to use linear traverse
                            map<string, int>::iterator itsetsub;
                            for (itsetsub = psemap.begin(); itsetsub != psemap.end(); ++itsetsub)
                            {

                                string eventkey = itsetsub->first;
                                int pushNum = itsetsub->second;

                                //decide if satisfied
                                if (pushNum < staticpse->trigureNum)
                                {
                                    notifyFlag = 0;
                                }
                            }
                            if (notifyFlag == 1)
                            {
                                printf("trigure/notify curr id (%s)\n", clientid.data());

                                //modify the global satisfied label to true
                                clientidtoWrapperMtx.lock();
                                clientidtoWrapper[clientid]->iftrigure = true;
                                clientidtoWrapperMtx.unlock();
                                //the value should be zero after trigguring operation
                                for (itsetsub = psemap.begin(); itsetsub != psemap.end(); ++itsetsub)
                                {

                                    string eventkey = itsetsub->first;
                                    clienttoSub[clientid][eventkey] = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

*/