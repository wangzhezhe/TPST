// start redis before starting the operator

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <hiredis.h>
#include "../redisclient/redisclient.h"

int main(int argc, char **argv)
{
    redisContext *c = redisInit();

    printf("do some monitoring operation\n");
    printf("when interesting things happen, for example, some conditions are satisfied, publish events\n");

    char *pubEvents = "T1_FINISH";
    char *pubMessages ="empty";

    redisPublish(c, pubEvents, pubMessages);

    return 0;
}