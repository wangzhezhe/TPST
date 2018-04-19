#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "redisclient.h"
#include "../runtime/slurm.h"

void publish()
{
    redisContext *c = redisInit();
    printf("new thread\n");
    sleep(5);
    redisPublish(c, "subEventa", "testabc");
    return;
}

int main(int argc, char **argv)
{
    redisContext *c = redisInit();

    char *subEvent = "subEventa subEventb";
    
    //pthread_t id;
    //pthread_create(&id, NULL, publish, NULL);
    //blocked here
    runtimeAction *ra=malloc(sizeof(runtimeAction));
    ra->actionLen=1;
    strcpy(ra->actionList[0],"abcd");
    redisSubscribe(ra, c, subEvent,(runtimeFunc)slurmTaskStart);

    return 0;
}
