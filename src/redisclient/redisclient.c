#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hiredis.h>

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

void redisSubscribe(redisContext *c, char *subscribeEventStrList)
{

    // subcribe command
    char command[COMMANDLEN];
    sprintf(command, "SUBSCRIBE %s", subscribeEventStrList);
    printf("command:%s\n",command);
    redisReply *reply = redisCommand(c, command);
    int j;
    while (redisGetReply(c, &reply) == REDIS_OK)
    {
        printf("get the subscribe return value (%s) element %d \n", reply->str, reply->elements);
        if (reply->type == REDIS_REPLY_ARRAY)
        {
            for (j = 0; j < reply->elements; j++)
            {
                printf("%u) %s\n", j, reply->element[j]->str);
            }
        }
        freeReplyObject(reply);
    }
    /* Disconnects and frees the context */
    redisFree(c);

    return 0;
}

void redisPublish(redisContext *c, char *publishEventStrList, char *publishMessages)
{

    //publish command
    char command[COMMANDLEN];
    sprintf(command, "PUBLISH %s %s", publishEventStrList, publishMessages);
    printf("command:%s\n",command);
    redisReply *reply = redisCommand(c, command);
    int j;
    while (redisGetReply(c, &reply) == REDIS_OK)
    {
        printf("get the publish return value (%s) element %d \n", reply->str, reply->elements);
        if (reply->type == REDIS_REPLY_ARRAY)
        {
            for (j = 0; j < reply->elements; j++)
            {
                printf("%u) %s\n", j, reply->element[j]->str);
            }
        }
        freeReplyObject(reply);
    }
    /* Disconnects and frees the context */
    redisFree(c);

    return 0;
}
