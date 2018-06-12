#include <string>
#include <vector>
#include "pubsub.h"
#include <iostream>
#include <unistd.h>

void testclear()
{
    subtoClient.clear();
}

void testOnesubOnePush()
{
    testclear();

    //add new client new event
    vector<string> eventList;
    eventList.push_back("ea");
    string clientid = "id1";
    //do this before subscribe

    pubsubSubscribe(eventList, clientid, "notifyback");
    output();
    printf("\n");
    pubsubPublish(eventList);
    printf("\n");
    output();
}

void testMultiPushMultiSub()
{
    testclear();

    //add new client new event
    vector<string> eventSubList;
    vector<string> eventPushList;
    eventSubList.push_back("T1_FINISH:1");
    eventSubList.push_back("T2_FINISH:1");
    eventSubList.push_back("T3_FINISH:1");
    eventPushList.push_back("T1_FINISH");
    eventPushList.push_back("T2_FINISH");

    eventPushList.push_back("T3_FINISH");

    string clientid = "id1";

    pubsubSubscribe(eventSubList, clientid, "notifyback");
    output();
    printf("send push\n\n");
    pubsubPublish(eventPushList);

    output();
}

void testMultipushOneSub()
{
    testclear();

    //add new client new event
    vector<string> eventSubList;
    vector<string> eventPushList;
    eventSubList.push_back("T1_FINISH:3");
    eventPushList.push_back("T1_FINISH");
    string clientid = "id1";
    //in real using, parse this before subscribe

    pubsubSubscribe(eventSubList, clientid, "notifyback");
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
    testclear();
    vector<string> eventListsub;
    vector<string> eventListpush;
    eventListsub.push_back("ea:1");
    //when push, using format without number
    eventListpush.push_back("ea");

    string clientid = "id1";
    pubsubSubscribe(eventListsub, clientid, "notifyback");

    clientid = "id2";
    pubsubSubscribe(eventListsub, clientid, "notifyback");

    clientid = "id3";
    pubsubSubscribe(eventListsub, clientid, "notifyback");

    output();
    printf("send push\n\n");
    pubsubPublish(eventListpush);

    output();
}

void startNotify(string eventwithoutNum, string clientID)
{
    pthread_t id;
    pubsubWrapper *psw = subtoClient[eventwithoutNum][clientID];
    pthread_create(&id, NULL, checkNotify, (void *)psw);
}

void testLargeSizeBroadcasterTrigguer()
{
    testclear();
    vector<string> eventListsub;
    vector<string> eventListpush;
    eventListsub.push_back("INIT:1");
    //when push, using format without number
    eventListpush.push_back("INIT");
    int i = 0;
    for (i = 0; i < 4096; i++)
    {
        string clientid = "id" + to_string(i);
        pubsubSubscribe(eventListsub, clientid, "notifyback");
        startNotify("INIT", clientid);
    }
    pubsubPublish(eventListpush);
    usleep(5000000);
}

void testBroadcasterTrigguer()
{
    testclear();
    vector<string> eventListsub;
    vector<string> eventListpush;
    eventListsub.push_back("ea:1");
    //when push, using format without number
    eventListpush.push_back("ea");

    string clientid = "id1";
    pubsubSubscribe(eventListsub, clientid, "notifyback");
    startNotify("ea", "id1");

    clientid = "id2";
    pubsubSubscribe(eventListsub, clientid, "notifyback");
    //check notify
    startNotify("ea", "id2");

    clientid = "id3";
    pubsubSubscribe(eventListsub, clientid, "notifyback");
    //check notify
    startNotify("ea", "id3");

    output();

    printf("send push\n\n");

    pubsubPublish(eventListpush);
    usleep(1000000);
}

void testMultiSub()
{
    testclear();

    vector<string> eventList;
    eventList.push_back("ea:1");
    string clientid1 = "id1";
    string clientid2 = "id2";
    string clientid3 = "id3";

    pubsubSubscribe(eventList, clientid1, "notifyaddr");
    pubsubSubscribe(eventList, clientid2, "notifyaddr");

    pubsubSubscribe(eventList, clientid3, "notifyaddr");
    output();
}

void testSplit()
{
    string fullevent = string("event1:1");
    string eventMessage;
    int pushNum;
    ParseEvent(fullevent, eventMessage, pushNum);
    printf("event message is (%s) push num is (%d)\n", eventMessage.data(), pushNum);
}

int main()
{

    //multipush could trigure one subscribed client
    //printf("------testOnesubMultiPush------\n");
    // testOnesubMultiPush();

    printf("---------------test for sub-------------------\n");
    vector<string> eventList;
    eventList.push_back("ea:1");
    eventList.push_back("eb:1");
    eventList.push_back("ec:1");

    string clientid = "id1";
    ;

    pubsubSubscribe(eventList, clientid, "notifyaddr");
    output();
    printf("test1 ok\n");

    printf("---------------test for multi client sub-------------------\n");
    testMultiSub();

    printf("---------------test for multi client sub one push(broadcaster)-------------------\n");
    testMultisubOnePush();

    printf("---------------test for multi pub same one sub(aggregation)-------------------\n");
    testMultipushOneSub();

    printf("---------------test for multi pub diff one sub(aggregation)-------------------\n");
    testMultiPushMultiSub();

    printf("---------------test for broadcaster trigguer-------------------\n");
    testBroadcasterTrigguer();

    testLargeSizeBroadcasterTrigguer();
    /*
    eventList.push_back("eb");
    eventList.push_back("ec");
   
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