

#ifndef local_h
#define local_h

#include <string>

using namespace std;

//TODO make this a common defination 
//typedef int (*runtimeFunc)(char *);

int localTaskStart( string batchPath);

extern int localFinishNum;
#endif