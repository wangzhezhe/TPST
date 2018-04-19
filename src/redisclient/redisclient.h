

#ifndef redisclient_h
#define redisclient_h


#include <hiredis.h>

redisContext *redisInit();
void redisSubscribe(redisContext *c, char *subscribeEventStrList);
void redisPublish(redisContext *c, char *publishEventStrList, char *publishMessages);

#endif