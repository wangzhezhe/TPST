#ifndef DLM_H
#define DLM_H

#include <string>

using namespace std;

void getLock(string Dir, string identity);

void releaseLock(string Dir);

#endif