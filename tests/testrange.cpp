#include <map>
#include <set>
#include <string>
#include "stdio.h"
#include "stdlib.h"
#define BILLION 1000000000L
using namespace std;

int main()
{
    map<string, set<string> > subtoClient;
    int i = 0;
    set<string> tempset;
    set<string>::iterator itset;
    for (i = 0; i < 1024; i++)
    {
        string stri = to_string(i);
        tempset.insert(string("id") + stri);
    }
    string initstr("INIT");
    subtoClient[initstr] = tempset;

    struct timespec start, end1;
    double diff;

    clock_gettime(CLOCK_REALTIME, &start);

    set<string> clientSet = subtoClient["INIT"];

    int setnum = clientSet.size();
    //printf("number for clientset %d when publish event %s\n", setnum, eventwithoutNum.data());
    for (itset = clientSet.begin(); itset != clientSet.end(); ++itset)
    {
        string clientid = (*itset);
        printf("get clientid %s\n", clientid.data());
    }
    clock_gettime(CLOCK_REALTIME, &end1); /* mark the end time */
    diff = (end1.tv_sec - start.tv_sec) * 1.0 + (end1.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    printf("debug for publish end1 response time = (%lf) second\n", diff);
}