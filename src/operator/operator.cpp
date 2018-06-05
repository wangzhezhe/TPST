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
#include <stdlib.h>
#include <string>

#include <grpc++/grpc++.h>

#include "workflowserver.grpc.pb.h"
#include "../publishclient/pubsubclient.h"
#include "../utils/split/split.h"


void initOperator(string queryEvent, int jsonNum)
{

    GreeterClient *greeter = GreeterClient::getClient();
    if (greeter == NULL)
    {
        printf("failed to get initialised greeter\n");
        return;
    }

    while (1)
    {
        int reply = greeter->GetSubscribedNumber(queryEvent.data());
        printf("there are %d clients subscribe %s event\n", queryEvent.data(), reply);
        if (reply < jsonNum)
        {
            usleep(500);
        }
        else
        {
            break;
        }
    }
}

//using self defined pub sub server
int main(int argc, char **argv)
{

    // check the input argument first is pub/sub second is the event list , shuld be the following format [event1,event2,event3]

    if (argc != 5)
    {
        printf("using following format: ./operator <checkEvent> <requiredNum> publish [event1,event2] or ./operator subscribe [event1.event2]\n");
        return 0;
    }

    string queryEvent = string(argv[1]);
    int requiredNum = atoi(argv[2]);

    //check second parameter

    string operation = string(argv[3]);
    if (operation.compare("publish") != 0 && operation.compare("subscribe") != 0)
    {
        printf("operation should be publish or subscribe\n");
        return 0;
    }

    string eventStr = string(argv[4]);
    string seprater = string(",");

    printf("operation %s event list %s\n", operation.data(), eventStr.data());

    //check third parameter

    vector<string> eventList;
    eventList = split(eventStr, seprater);

    initOperator(queryEvent,requiredNum);

    GreeterClient *greeter = GreeterClient::getClient();
    if (greeter == NULL)
    {
        printf("failed to get initialised greeter\n");
        return 0;
    }
    if (operation.compare("publish") == 0)
    {
        string reply = greeter->Publish(eventList);
        cout << "Publish return value: " << reply << endl;
    }
    else
    {
        //subscribe
        string reply = greeter->Subscribe(eventList);
        cout << "Subscribe return value: " << reply << endl;
    }
}