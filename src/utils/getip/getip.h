

#ifndef getip_h
#define getip_h

#include <string>

using namespace std;

void recordIPPort(string &ipstr, string port);

int loadIPPort(string configpath,string &ipstr,string &port);

#endif