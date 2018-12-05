


#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include <string>
#include <map>
#include <vector>
#include <mutex>


using namespace std;

void recordIPortIntoClusterDir(string &ipstr, string port, string clusterDir, int RequiredDirSize);

void updateWorkerAddrMap(string clusterDir);

string getClusterDir(int currentID);

int getFreePortNum();

string parseIP(string peerURL);

string parsePort(string peerURL);

vector<string> loadAddrInDir(string Dir);

void recordIPPortWithoutFile(string &ipstr, string port);

vector<string> getFreeNodeList(int needNum);

extern const string gm_multinodeip;

extern const string gm_FreePool;

extern const string gm_coordinatorDir;

extern const string gm_workerDir;

extern string GM_INTERFACE;

extern int gm_requiredGroupSize;

extern int gm_groupNumber;

extern int gm_rank;

extern string SERVERSTATUS;

extern string status_coor;

extern mutex workerAddrMapLock;

extern map<string, vector<string>> workerAddrMap;

extern string GM_CLUSTERDIR;

#endif