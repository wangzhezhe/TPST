
#ifndef localserver_h
#define localserver_h

#include <string>
#include <vector>

using namespace std;

void localPublish(vector<string> eventList);
void localSubscribe(string clientId, vector<string> eventList);
extern int localNotifiedNum;
void startNotifySequence();



#endif