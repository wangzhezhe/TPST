#ifndef local_h
#define local_h

#include <string>

using namespace std;

//TODO make this a common defination 
//typedef int (*runtimeFunc)(char *);

int pythonTaskStart(string batchPath, string metadata);

int localTaskStart( string batchPath, string metadata);

extern int localFinishNum;
#endif