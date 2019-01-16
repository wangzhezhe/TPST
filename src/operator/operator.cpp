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

#include "../publishclient/pubsubclient.h"
#include "../utils/split/split.h"
#include "../server/notifyserver.h"
#include "../utils/groupManager/groupManager.h"
#include "../observer/eventmanager.h"
#include "../utils/file/loaddata.h"

#include <grpc++/grpc++.h>
#include "workflowserver.grpc.pb.h"

void parseScript()
{
    string triggerPath("/project1/parashar-001/zw241/software/eventDrivenWorkflow/tests/TrigureFiles");
    vector<string> fileList;
    fileList = scanFolder(triggerPath.data());

    int count = fileList.size();

    for (int i = 0; i < count; i++)
    {

        // if it is json file
        if (strstr(fileList[i].data(), ".json") != NULL)
        {

            string filePath = triggerPath + "/" + fileList[i];
            printf("get file path (%s)\n", filePath.data());

            string jsonbuffer = loadFile(filePath.data());

            //create new client id and new event triggure

            string clientID;

            EventTriggure *etrigger = addNewConfig(jsonbuffer, clientID);

            printf("check etrigger, type (%s) driver (%s) meta (%s) msg (%s)\n", etrigger->matchType.data(), etrigger->driver.data(), etrigger->metaData.data(),etrigger->eventSubList[0].data());

            //send subscribe request (unblocked with id) send eventlist and the client id

            if (clientID != "")
            {
                eventSubscribe(etrigger, clientID, NOTIFYADDR ,etrigger->eventSubList[0]);
            }
        }
    }

    return;
}

//using self defined pub sub server
int main(int argc, char **argv)
{

    //start the notify server

    pthread_t notifyserverid;
    int status;

    //get notify server addr
    string notifyAddr = getNotifyServerAddr();
    NOTIFYADDR = notifyAddr;
    //send value to server addr
    
    //TODO send this parameter from outside
    gm_groupNumber = 1;

    //start the server
    pthread_create(&notifyserverid, NULL, &RunNotifyServer, NULL);

    //wait the notify server start
    sleep(1);

    printf("the address of notify server is %s\n", NOTIFYADDR.data());

    //parse the files in runtimeScripts
    parseScript();

    //register those files into the pub/sub broker

    while (1)
    {
        usleep(1000);
    }

    return 0;
}