#ifndef loaddata_h
#define loaddata_h

#include <vector>
#include <string>

using namespace std;

string loadFile(const char *filename);
int ifjson(char *filename);
char *getTaskNameFromEventName(char *filename);
vector <string> scanFolder(const char *dirpath);

#endif