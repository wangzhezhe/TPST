#include "local.h"
#include <time.h> /* for clock_gettime */
#include <thread>
#include <string>
#include "unistd.h"

using namespace std;

#define BILLION 1000000000L

int main()
{

    struct timespec start, end;
    double diff;

    clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

    int i = 0;
    int reqNum=1024;
    string batchPath = "/bin/bash /home1/zw241/observerchain/tests/performance/app/simulate.sh --timesteps 1 --range 100 --nvalues 5 --log off> sim.out";
    localTaskStart(batchPath);
    for (i = 0; i < reqNum; i++)
    {
        string batchPath = "/bin/bash /home1/zw241/observerchain/tests/performance/app/analysis.sh sim.out > ana" + to_string(i) + ".out";
        thread{localTaskStart, batchPath}.detach();
    }

    while (1)
    {
        if ((localFinishNum-1) == reqNum)
        {
            break;
        }
        else
        {
            usleep(100000);
        }
        printf("curr num %d\n",localFinishNum);
    }

    clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */
    diff = (end.tv_sec - start.tv_sec) * 1.0 + (end.tv_nsec - start.tv_nsec) * 1.0 / BILLION;
    printf("debug for total simulation time = (%lf) second\n", diff);

    return 0;
}