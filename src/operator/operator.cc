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
#include <uuid/uuid.h>

#include "../publishclient/pubsubclient.h"
#include "../utils/split/split.h"
#include "../utils/groupManager/groupManager.h"
#include "../observer/eventmanager.h"
#include "../utils/file/loaddata.h"

#include <grpc++/grpc++.h>
#include "workflowserver.grpc.pb.h"

#include "operator.h"


void testPublishTgRegister(GreeterClient *greeter)
{

    printf("------test testPublishTgRegister------\n");
    vector<string> postPublishList;
    postPublishList.push_back("testposttopic");
    string publishMeta = "publish post triggure meta\n";
    string reply = greeter->Publish(postPublishList, sourceClient, publishMeta, "NAME");

    if (reply.compare("OK") != 0)
    {
        printf("rpc failed, publish %s failed\n", postPublishList[0].data());
    }

    return;
}

void testBundleRegister(GreeterClient *greeter,string notifyaddr)
{
    printf("------test bundle register ok------\n");
    //generate the task and the triggure
    Bundle *b = new (Bundle);
    b->taskName = "testTaskName";
    b->taskTemplates = "testTemplates";

    b->pretg.type = preTrg;
    b->pretg.subMeta = "testpresubmeta";
    b->pretg.subtopic = "testpretopic";
    b->pretg.taskName = b->taskName;

    b->posttg.type = postTrg;
    b->posttg.subMeta = "testpostsubmeta";
    b->posttg.subtopic = "testposttopic";
    b->posttg.taskName = b->taskName;

    //put them into the map
    bmap[b->taskName] = *b;

    //subscribe the triggure
    //generate the id from triggure

    uuid_t uuid;
    char idstr[50];

    uuid_generate(uuid);
    uuid_unparse(uuid, idstr);

    string preSubID(idstr);

    uuid_generate(uuid);
    uuid_unparse(uuid, idstr);

    string postSubID(idstr);

    //update the triggure map
    //binding id with the trigure
    submapMutex.lock();
    submap[preSubID] = b->pretg;
    submap[postSubID] = b->posttg;
    submapMutex.unlock();

    //subscribe to the server (sub post tg firstly then sub pre tg)

    string matchType = "NAME";

    vector<string> postTgList;
    postTgList.push_back(b->posttg.subtopic);

    string replypost = greeter->Subscribe(postTgList, postSubID, notifyaddr, sourceClient, matchType, b->posttg.subMeta);

    if (replypost.compare("SUBSCRIBED") != 0)
    {
        printf("rpc replypost failed\n");
    }

    //register the pre triggure
    //if it is init, execute the task direactly
    if (b->pretg.subMeta.compare("INIT") != 0)
    {
        vector<string> preTgList;
        preTgList.push_back(b->pretg.subtopic);
        string replypre = greeter->Subscribe(preTgList, preSubID, notifyaddr, sourceClient, matchType, b->pretg.subMeta);
        if (replypre.compare("SUBSCRIBED") != 0)
        {
            printf("rpc replypre failed\n");
        }
    }
    else
    {
        //start the task
        printf("pretag for task (%s) is INIT, start task directly\n", b->taskName.data());
    }

    printf("ok to sub the pre triggure and post triggure for (%s)\n", b->taskName.data());

    return;
}

