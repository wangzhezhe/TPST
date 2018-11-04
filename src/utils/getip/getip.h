

#ifndef getip_h
#define getip_h

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <dirent.h>

using namespace std;

void recordIPPort(string &ipstr, string port);

int loadIPPort(string configpath,string &ipstr,string &port);

void recordIPPortWithoutFile(string &ipstr, string port);

void recordIPortForMultiNode(string &ipstr, string port);

string parseIP(string peerURL);

string parsePort(string peerURL);

string getClusterDirByTopicId(int topicid);

vector<string> loadMultiNodeIPPort(string identity);

vector<string> loadMultiNodeIPPortByClusterDir(string clusterDir);

int getFreePortNum();

string clientClusterDir();

extern string INTERFACE;
extern int GETIPCOMPONENTNUM;
extern int GETIPCOMPONENTID;
extern int GETIPNUMPERCLUSTER;
extern int SERVERCLUSTERNUM;

#endif