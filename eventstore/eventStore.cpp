
#include <vector>
#include <string.h>
#include "eventStore.h"
using namespace std;

//event Register
//input tm and associated message
vector<HeadNode *> initEventStore()
{
  vector<HeadNode *> es;
  return es;
}

void printEventStore(vector<HeadNode *> &es)
{
  int newcount = es.size();
  int i, j;
  //printf("new count %d\n", newcount);
  for (i = 0; i < newcount; i++)
  {
    printf("event name %s\n", es[i]->te->str);
    int tempc = es[i]->tmList.size();
    for (j = 0; j < tempc; j++)
    {
      printf("tm name %s\n", es[i]->tmList[j]->name);
    }
  }
}