

#ifndef redisclient_h
#define redisclient_h


#include <hiredis.h>
#include "../runtime/slurm.h"



typedef struct runtimeAction
{
    unsigned int actionLen;
    char actionList[100][100];
}runtimeAction;


redisContext *redisInit();

void redisSubscribe(runtimeAction*ra,redisContext *c, char *subscribeEventStrList, runtimeFunc runtimefunc);

void redisPublish(redisContext *c, char *publishEventStrList, char *publishMessages);

#endif