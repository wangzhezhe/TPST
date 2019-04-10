


#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <set>



using namespace std;

string getSingleClientFromDir();

void recordIPortIntoClusterDir(string &ipstr, string port, string clusterDir, int RequiredDirSize);

void updateWorkerAddrMap(string clusterDir);

string getClusterDir(int currentID);

int getFreePortNum();

string parseIP(string peerURL);

string parsePort(string peerURL);

vector<string> loadAddrInDir(string Dir);

void recordIPPortWithoutFile(string &ipstr);

vector<string> getFreeNodeList(int needNum);

bool nodeAttach(string dir, string nodeAddr);

void updateCoordinatorAddr();

void createDir(string DirPath);

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

extern const string sourceIngroup; 
extern const string sourceBetweengroup;
extern const string sourceClient;

extern set<string> coordinatorAddrSet;

#endif