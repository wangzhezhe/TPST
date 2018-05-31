#include <string>
#include <vector>
#include "pubsub.h"

void testclear()
{
    strtoEvent.clear();
    clientidtoWrapper.clear();
    subtoClient.clear();
    clienttoSub.clear();
}

void testOnesubOnePush()
{
    testclear();

    //add new client new event
    vector<string> eventList;
    eventList.push_back("ea");
    string clientid = "id1";
    addNewClient(clientid);
    addNewEvent("ea", 1);

    pubsubSubscribe(eventList, clientid);
    output();
    printf("\n");
    pubsubPublish(eventList);
    printf("\n");
    output();
}

void testOnesubMultiPush()
{
    testclear();

    //add new client new event
    vector<string> eventSubList;
    vector<string> eventPushList;
    eventSubList.push_back("T1_FINISH:2");
    eventPushList.push_back("T1_FINISH");
    string clientid = "id1";
    addNewClient(clientid);
    //in real using, parse this before subscribe
    addNewEvent("T1_FINISH", 3);

    pubsubSubscribe(eventSubList, clientid);
    output();
    printf("send push\n\n");
    pubsubPublish(eventPushList);

    output();
    printf("send push\n\n");
    pubsubPublish(eventPushList);
    printf("send push\n\n");
    pubsubPublish(eventPushList);

    output();

    
}

void testMultisubOnePush()
{

    vector<string> eventList;
    eventList.push_back("ea");
    addNewEvent("ea", 1);
    string clientid = "id1";
    addNewClient(clientid);
    pubsubSubscribe(eventList, clientid);

    clientid = "id2";
    addNewClient(clientid);
    pubsubSubscribe(eventList, clientid);

    clientid = "id3";
    addNewClient(clientid);
    pubsubSubscribe(eventList, clientid);

    output();
    printf("send push\n\n");
    pubsubPublish(eventList);
    output();
}

void testSplit(){
    string fullevent=string("event1:1");
    string eventMessage;
    int pushNum;
    ParseEvent(fullevent,eventMessage,pushNum);
    printf("event message is (%s) push num is (%d)\n",eventMessage.data(),pushNum);
}


int main()
{

    //multipush could trigure one subscribed client
    printf("------testOnesubMultiPush------\n");
    testOnesubMultiPush();
    /*
    printf("---------------test for sub-------------------\n");
    vector<string> eventList;
    eventList.push_back("ea");
    string clientid = "id1";
    addNewClient(clientid);
    addNewEvent("ea", 1);

    pubsubSubscribe(eventList, clientid);
    output();
    printf("test1 ok\n");

    eventList.push_back("eb");
    eventList.push_back("ec");
    addNewEvent("eb", 1);
    addNewEvent("ec", 1);
    clientid = "id1";

    pubsubSubscribe(eventList, clientid);
    output();
    printf("test2 ok\n");

    clientid = "id2";
    addNewClient(clientid);
    pubsubSubscribe(eventList, clientid);
    output();
    printf("test3 ok\n");

    printf("---------------test for pub-------------------\n");

    //one push can trigure one subscribed client
    printf("------testOnesubOnePush------\n");
    testOnesubOnePush();

    //multipush could trigure one subscribed client
    printf("------testOnesubMultiPush------\n");
    testOnesubMultiPush();

    //one push trigure multi subscribed client
    printf("------testMultisubOnePush------\n");
    testMultisubOnePush();

    printf("------testMultiThreadOperation?------\n");
    */

    //testSplit();
}