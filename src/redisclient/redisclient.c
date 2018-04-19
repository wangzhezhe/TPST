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

void redisSubscribe(runtimeAction *ra,redisContext *c, char *subscribeEventStrList, runtimeFunc runtimefunc)
{

    // subcribe command
    if(ra==NULL){
        printf("runtime action should not be NULL\n");
        return;
    }
    char command[COMMANDLEN];
    sprintf(command, "SUBSCRIBE %s", subscribeEventStrList);
    printf("command:(%s)\n",command);
    redisReply *reply = (redisReply *)redisCommand(c, command);
    int i,j;
    while (redisGetReply(c, (void**)&reply) == REDIS_OK)
    {
        if (reply->type == REDIS_REPLY_ARRAY)
        {
            if(strcmp(reply->element[0]->str,"subscribe")==0){
                freeReplyObject(reply);
                continue;
            }
            for (j = 0; j < reply->elements; j++)
            {
                printf("%u) %s\n", j, reply->element[j]->str);
                //call runtime func to start the command
                //TODO start by openMP

            }
            //Execute runtime function
            for(i=0;i<ra->actionLen;i++){
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
    sprintf(command, "PUBLISH %s %s", publishEventStrList, publishMessages);
    printf("command:%s\n",command);
    
    redisReply *reply = (redisReply *)redisCommand(c, command);
    int j;
    while (redisGetReply(c, (void**)&reply) == REDIS_OK)
    {
        printf("get the publish return value (%s) element %d \n", reply->str, (int)reply->elements);
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

    return;
}
