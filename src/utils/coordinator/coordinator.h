
#ifndef coordinator_h
#define coordinator_h

#include <string>

using namespace std;

typedef struct eventStatusInGroup
{
    double avg = 0;
    int currSubNumber;
    map<string, int> subNumForNode;

} eventStatusInGroup;


typedef struct Plan{

    string srcAddr;
    string destAddr;
    //TODO only for one sub
    // what if multiple sub?
    string moveSubscription;
    int moveNumber;
}Plan;


typedef struct SubMeta{
    string serverAddr;
    int subNumber;
}SubMeta;


extern mutex eventRecordMapLock;

extern map<string, map<string, int>> eventRecordMap;

double getAverage(string event);

extern int overLoadThreshold;

int calculateNewServerNum(double avg, int currSubNum);

vector<Plan> generatePlanForEvent(string imbEvent);

void updateStatusMap(string event, string serverAddr, int subNumber);

void outputMap();

#endif