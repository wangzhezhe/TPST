#include "pubsub.h"
#include "stdio.h"

//event string to pubsubevent (value is real element with memory) from eventstring to eventStructure
//this map store the static event
//the dynamic event store in the innermap of clienttoSub, the value represent the number of event published
map<string, pubsubEvent *> strtoEvent;

//client id to pubsubWrapper(value is real element with memory) from clientid to clientStructure
mutex clientidtoWrapperMtx;
map<string, pubsubWrapper *> clientidtoWrapper;

// to pubsubWrapperid (value is pointer) from subeventstring to set of clientid
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
    clientidtoWrapper[clientid] = psw;
}

void addNewEvent(string str, int num)
{
    pubsubEvent *pse = new (pubsubEvent);
    pse->event = str;
    pse->trigureNum = num;
    strtoEvent[str] = pse;
}

void pubsubSubscribe(vector<string> eventList, string clientId)
{

    int size = eventList.size();
    int i = 0;

    //put value into clienttoSub and subtoCliet

    for (i = 0; i < size; i++)
    {
        if (clienttoSub.find(clientId) == clienttoSub.end())
        {
            // not found
            map<string, int> innermap;
            innermap[eventList[i]] = 0;
            clienttoSub[clientId] = innermap;
        }
        else
        {

            if (clienttoSub[clientId].find(eventList[i]) == clienttoSub[clientId].end())
            {
                // not found
                clienttoSub[clientId][eventList[i]] = 0;
            }
            else
            {
                //found(already exist)
                //do nothing
            }
        }

        //put value into subtoCliet
        if (subtoClient.find(eventList[i]) == subtoClient.end())
        {
            // not found
            set<string> s;
            s.insert(clientId);
            subtoClient[eventList[i]] = s;
        }
        else
        {
            //if client id exist
            subtoClient[eventList[i]].insert(clientId);
        }
    }
}

void output()
{
    //output subtoCliet
    map<string, set<string>>::iterator itera;
    for (itera = subtoClient.begin(); itera != subtoClient.end(); ++itera)
    {
        printf("subtoCliet key (%s) static trigNum (%d)\n", strtoEvent[itera->first]->event.data(), strtoEvent[itera->first]->trigureNum);
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
    //output clienttoSub
}

void pubsubPublish(vector<string> eventList)
{

    int size = eventList.size();
    int i;
    for (i = 0; i < size; i++)
    {
        string currpushEvent = eventList[i];
        //search for pub to client

        if (subtoClient.find(currpushEvent) == subtoClient.end())
        {
            // not fount
            // do nothing
            printf("failed to get event %s from strtoClient\n", currpushEvent.data());
        }
        else
        {
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