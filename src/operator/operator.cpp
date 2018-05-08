/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <unistd.h>
#include <pthread.h>

#include <grpc++/grpc++.h>

#include "workflowserver.grpc.pb.h"
#include "../publishclient/pubsubclient.h"

/*
//deprecated by using redis
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
*/

//using self defined pub sub server
int main(int argc, char **argv)
{
    sleep(5);
    printf("new thread push event\n");
    vector<string> publisheventList;
    publisheventList.push_back("T1_FINISH");
    GreeterClient *greeter = GreeterClient::getClient();
    if (greeter == NULL)
    {
        printf("failed to get initialised greeter\n");
        return 0;
    }
    string reply = greeter->Publish(publisheventList);
    cout << "Publish return value: " << reply << endl;
}