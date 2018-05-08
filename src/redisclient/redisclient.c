#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hiredis.h>
#include "redisclient.h"

#define COMMANDLEN 100

redisContext *redisInit()
{

    unsigned int j;
    redisContext *c;

    const char *hostname = "127.0.0.1";
    int port = 6379;

    struct timeval timeout = {1, 500000}; // 1.5 seconds
    c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || c->err)
    {
        if (c)
        {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        }
        else
        {
            printf("Connection error: can't allocate redis context\n");
        }
        return NULL;
    }

    return c;
}

void redisSubscribe(runtimeAction *ra, redisContext *c, char *subscribeEventStrList, runtimeFunc runtimefunc)
{
    //If delete this , their is no ouput when redirect, don't know why
    //printf("redis sub0\n");
    // subcribe command
    if (ra == NULL)
    {
        printf("runtime action should not be NULL\n");
        return;
    }
    char command[COMMANDLEN];
    sprintf(command, "SUBSCRIBE %s", subscribeEventStrList);
    redisReply *reply = (redisReply *)redisCommand(c, command);
    int i, j;
    //printf("redis sub1\n");
    while (redisGetReply(c, (void **)&reply) == REDIS_OK)
    {
        if (reply->type == REDIS_REPLY_ARRAY)
        {
            if (strcmp(reply->element[0]->str, "subscribe") == 0)
            {
                freeReplyObject(reply);
                continue;
            }

            //Execute runtime function
            for (i = 0; i < ra->actionLen; i++)
            {
                //printf("test debug0\n");
                runtimefunc(ra->actionList[i]);
            }
        }

        freeReplyObject(reply);
    }
    /* Disconnects and frees the context */
    redisFree(c);

    return;
}

void redisPublish(redisContext *c, char *publishEventStrList, char *publishMessages)
{

    //publish command
    char command[COMMANDLEN];
    if (publishMessages == NULL)
    {
        strcpy(publishMessages, "null");
    }
    sprintf(command, "PUBLISH %s %s", publishEventStrList, publishMessages);

#ifdef DEBUG
    printf("command:%s\n", command);
#endif
    
    redisReply *reply = (redisReply *)redisCommand(c, command);
    int j;
#ifdef DEBUG
    printf("get the publish return value (%ld)\n", reply->integer);
#endif
    freeReplyObject(reply);    
    /* Disconnects and frees the context */
    redisFree(c);

    return;
}
