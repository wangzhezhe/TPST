#ifndef eventStore_h
#define eventStore_h

#include <vector>
#include "../observer/taskcommon.h"

//event store
vector<HeadNode *> initEventStore();
void printEventStore(vector<HeadNode *> &es);

#endif

