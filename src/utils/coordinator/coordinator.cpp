#include <map>
#include <math.h>
#include <string>
#include <mutex>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unistd.h>

#include "./coordinator.h"
#include "../groupManager/groupManager.h"

using namespace std;

//from eventName to map<nodeaddr, subNumber>
mutex eventRecordMapLock;

//map<string, eventStatusInGroup *> eventRecordMap;

map<string, map<string, int>> eventRecordMap;

// 0.5 s
int checkPeriod = 5000000;

//default value is 1024
int overLoadThreshold = 1024;

//debug map

double getAverage(string event)
{
    if (eventRecordMap.find(event) == eventRecordMap.end())
    {
        return 0;
    }
    map<string, int> subNumForNode = eventRecordMap[event];
    int serverNodesize = subNumForNode.size();

    if (serverNodesize == 0)
    {
        printf("no server process for event %s\n", event.data());
        return 0;
    }

    double sum = 0;
    for (map<string, int>::iterator inerit = subNumForNode.begin(); inerit != subNumForNode.end(); ++inerit)
    {
        sum += inerit->second;
    }

    return sum / serverNodesize * 1.0;
}

void outputMap()
{

    printf("mapsize (%lu)\n", eventRecordMap.size());

    for (map<string, map<string, int>>::iterator it = eventRecordMap.begin(); it != eventRecordMap.end(); ++it)
    {
        string event = it->first;
        map<string, int> subNumForNode = it->second;

        for (map<string, int>::iterator inerit = subNumForNode.begin(); inerit != subNumForNode.end(); ++inerit)
        {
            string innerevent = inerit->first;
            int innervalue = inerit->second;
            printf("event %s inner server addr (%s) inner value (%d)\n", event.data(), innerevent.data(), innervalue);
        }
    }
}

void redistributeStatusMap(string event, string serverAddr, int subNumber)
{

    eventRecordMapLock.lock();
    eventRecordMap[event][serverAddr] += subNumber;
    eventRecordMapLock.unlock();
}

//this function will be called for every subscription in group
//subNum is current total subscription value
void updateStatusMap(string event, string serverAddr, int subNumber)
{

    
    //empty
    if (eventRecordMap.find(event) == eventRecordMap.end())
    {

        eventRecordMap[event][serverAddr] = subNumber;

        //printf("new event %s new addr %s\n", event.data(), serverAddr.data());
    }
    else
    {
        if (subNumber > eventRecordMap[event][serverAddr])
        {
            //exist
            //update node subNum

            //printf("update event %s update addr %s\n", event.data(), serverAddr.data());

            eventRecordMap[event][serverAddr] = subNumber;

            //int currSubNum = eventRecordMap[event]->currSubNumber;

            //int serverNum = eventRecordMap[event]->subNumForNode.size();

            //eventRecordMap[event]->avg = (subNumber) / serverNum;

            //eventRecordMap[event]->currSubNumber = subNumber;
        }
    }
}

int calculateNewServerNum(double avg, int currSubNum)
{

    int addedProcessNum = floor((avg * currSubNum) / overLoadThreshold - currSubNum);

    return addedProcessNum;
}

vector<string> fakeGetServerList(int num)
{
    vector<string> serverList;
    for (int i = 0; i < num; i++)
    {
        string estr = "fakeServer_" + to_string(i);
        serverList.push_back(estr);
    }

    return serverList;
}

void AddNewServer(string event, vector<string> serverAddr)
{

    int size = serverAddr.size();
    for (int i = 0; i < size; i++)
    {
        string addr = serverAddr[i];
        eventRecordMapLock.lock();
        eventRecordMap[event][addr] = 0;
        eventRecordMapLock.unlock();
    }
}

bool SortByMeta(const SubMeta &sbm1, const SubMeta &sbm2)
{
    return sbm1.subNumber > sbm2.subNumber;
}

Plan *generatePlan(string event, string src, string dest, int num)
{
    Plan *plan = new (Plan);
    plan->srcAddr = src;
    plan->destAddr = dest;
    plan->moveSubscription = event;
    plan->moveNumber = num;

    return plan;
}

void fakeRedistributeByPlan(vector<Plan> planList)
{
    int size = planList.size();

    for (int i = 0; i < size; i++)
    {
        Plan plan = planList[i];
        string moveEvent = plan.moveSubscription;
        string srcaddr = plan.srcAddr;
        string destaddr = plan.destAddr;

        int diff = plan.moveNumber;

        redistributeStatusMap(moveEvent, srcaddr, -diff);
        redistributeStatusMap(moveEvent, destaddr, diff);
    }
}

vector<Plan> generatePlanForEvent(string imbEvent)
{

    map<string, int> innerMap = eventRecordMap[imbEvent];

    //caculate avg

    double avg = getAverage(imbEvent);

    int serverNodesize = eventRecordMap[imbEvent].size();

    vector<SubMeta> subMetaList;
    vector<Plan> planList;

    for (map<string, int>::iterator it = innerMap.begin(); it != innerMap.end(); ++it)
    {

        string saddr = it->first;
        int subNumForOneServer = it->second;

        SubMeta *subMeta = new (SubMeta);
        subMeta->serverAddr = saddr;
        subMeta->subNumber = subNumForOneServer;

        subMetaList.push_back(*subMeta);
    }

    int size = subMetaList.size();

    if (size <= 1)
    {
        printf("only one submeta in List\n");
        return planList;
    }

    sort(subMetaList.begin(), subMetaList.end(), SortByMeta);

    int p1 = 0, p2 = 1;

    int newAvg = ceil(avg);

    while (p2 < size)
    {

        if (subMetaList[p2].subNumber >= newAvg)
        {
            p2++;
        }
        else
        {
            int supply = subMetaList[p1].subNumber - newAvg;
            int needNum = newAvg - subMetaList[p2].subNumber;

            if (supply >= needNum)
            {
                //generate plan
                Plan *plan = generatePlan(imbEvent, subMetaList[p1].serverAddr, subMetaList[p2].serverAddr, needNum);
                planList.push_back(*plan);
                p2++;
            }
            else
            {
                //generate plan

                Plan *plan = generatePlan(imbEvent, subMetaList[p1].serverAddr, subMetaList[p2].serverAddr, supply);
                planList.push_back(*plan);
                p1++;
            }
        }
    }

    return planList;
}

/*

int main()
{

    int i = 0, j = 0;

    for (i = 0; i < 2; i++)
    {
        string event = "event_" + to_string(i);
        for (j = 0; j < 5; j++)
        {
            string addr = "addr_" + to_string(j);
            updateStatusMap(event, addr, 256);
            if (i == 1 && j > 3)
            {
                updateStatusMap(event, addr, 1024 * 10);
            }
        }
    }

    outputMap();

    checkOverload();

    printf("after checking\n");

    outputMap();

    periodChecking();

    //outputMap();
}
*/
