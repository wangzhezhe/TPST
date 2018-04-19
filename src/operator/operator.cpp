// start redis before starting the operator

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <hiredis.h>

int main(int argc, char **argv)
{
    unsigned int j;
    redisContext *c;
    redisReply *reply;
    const char *hostname = (argc > 1) ? argv[1] : "127.0.0.1";
    int port = (argc > 2) ? atoi(argv[2]) : 6379;

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
        exit(1);
    }

    //TODO start the logic for data checking and wait for interesting things happen
    sleep(2);

    char publishEvents[20] = "second messagetest";

    char command[50];

    sprintf(command, "PUBLISH %s", publishEvents);
    reply =  (redisReply *)redisCommand(c, command);
    
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        printf("elem number %d\n",reply->elements);
        for (j = 0; j < reply->elements; j++)
        {
            printf("%u) %s\n", j, reply->element[j]->str);
        }
    }else{
        printf("publish return: %s\n", reply->str);
    }
    freeReplyObject(reply);
}