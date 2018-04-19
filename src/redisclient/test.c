#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "redisclient.h"

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
    
    pthread_t id;
    pthread_create(&id, NULL, publish, NULL);
    //blocked here
    redisSubscribe(c, subEvent);

    return 0;
}
