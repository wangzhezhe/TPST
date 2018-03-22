#include "stdio.h"
#include <unistd.h>
#include "../../observer/taskcommon.h"
#include <vector>
#include <map>
#include <string>

using namespace std;
// reference to db
// reference to es
extern "C" int action(vector<HeadNode *> &es, map<string, vector<float> > storage)
{
    printf("start first task t0\n");
}
