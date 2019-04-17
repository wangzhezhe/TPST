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

#include <cstdlib>
#include <fstream>
#include <sstream>

#include "../publishclient/pubsubclient.h"
#include "../utils/split/split.h"
#include "../utils/groupManager/groupManager.h"
#include "../observer/eventmanager.h"
#include "../utils/file/loaddata.h"

#include <grpc++/grpc++.h>
#include "workflowserver.grpc.pb.h"

#include "operator.h"
#include "../../src/metadatamanagement/metaclient.h"

mutex bmapMutex;
map<string, Bundle> bmap;

//from the sub id to the triggure
mutex submapMutex;
map<string, Triggure> submap;

// call slurm templates and get the id
string runTaskByTemplateSlurm(string templateName)
{

    //default driver is the slurm

    string command = "sbatch " + templateName + " >syscommand.txt";

    std::system(command.data()); // execute the UNIX command "ls -l >test.txt"
    std::stringstream buffer;
    buffer << std::ifstream("syscommand.txt").rdbuf();
    std::string str = buffer.str();
    printf("execute results : %s", str.data());

    //Submitted batch job 43942
    //filter out the id from the string

    int len = str.length();
    string taskID = str.substr(len - 5, len);

    //scancel 43942

    return taskID;
}

void stopTaskBySlurm(string taskName)
{
    printf("execute slurm stop for %s\n",taskName.data());
    
    //record time for task stop
    recordKey(taskName);

    bmapMutex.lock();
    string taskID = bmap[taskName].taskRunningID;
    bmapMutex.unlock();
    string command = "scancel " + taskID;
    std::system(command.data());
    printf("%s\n",command.data());

    return;
}

void ActionByOperator(string clientID, string metadata)
{

    printf("debug start operation id (%s) and meta (%s)\n", clientID.data(), metadata.data());

    //map the clientID into the triggure map
    submapMutex.lock();
    Triggure triggure = submap[clientID];
    submapMutex.unlock();

    string triggureType = triggure.type;

    string taskName = triggure.taskName;

    bmapMutex.lock();
    string taskTemplates = bmap[taskName].taskTemplates;
    bmapMutex.unlock();
    //if the triggure is pre or post (according to metadata)

    if (triggureType == preTrg)
    {
        //pre triggure
        runTaskByTemplateSlurm(taskTemplates);
    }
    else if (triggureType == postTrg)
    {
        //post triggure
        //if the post type is the stop, than stop the task
        stopTaskBySlurm(taskName);
    }
    else
    {
        //unsopported triggure
        printf("unsupported triggure type (%s)\n", triggureType.data());
    }

    //if pre get the bundle and execute the task template

    //update task runningid

    //if post do operation on task
    return;
}

void testPublishTgRegister(GreeterClient *greeter)
{

    printf("------test testPublishTgRegister------\n");
    vector<string> postPublishList;
    postPublishList.push_back("testposttopic");
    string publishMeta = "publish post triggure meta";
    string reply = greeter->Publish(postPublishList, sourceClient, publishMeta, "NAME");

    if (reply.compare("OK") != 0)
    {
        printf("rpc failed, publish %s failed\n", postPublishList[0].data());
    }

    return;
}

void testBundleRegister(GreeterClient *greeter,
                        string notifyaddr,
                        string taskName,
                        string taskTemplates,
                        string preTopic,
                        string preMeta,
                        string postTopic,
                        string postMeta)
{
    printf("------test bundle register ok------\n");
    //generate the task and the triggure
    Bundle *b = new (Bundle);
    b->taskName = taskName;
    b->taskTemplates = taskTemplates;

    b->pretg.type = preTrg;
    //b->pretg.subMeta = "testpresubmeta";

    b->pretg.subtopic = preTopic;
    b->pretg.subMeta = preMeta;
    b->pretg.taskName = b->taskName;

    b->posttg.type = postTrg;

    b->posttg.subtopic = postTopic;
    b->posttg.subMeta = postMeta;

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

    if (b->posttg.subtopic.compare("NONE") != 0)
    {
        vector<string> postTgList;
        postTgList.push_back(b->posttg.subtopic);

        string replypost = greeter->Subscribe(postTgList, postSubID, notifyaddr, sourceClient, matchType, b->posttg.subMeta);

        if (replypost.compare("SUBSCRIBED") != 0)
        {
            printf("rpc replypost failed\n");
        }
    }

    //register the pre triggure
    //if it is init, execute the task direactly
    if (b->pretg.subtopic.compare("INIT") != 0)
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
        string taskRunningID = runTaskByTemplateSlurm(taskTemplates);
        //get the task id and update the map
        bmapMutex.lock();
        bmap[b->taskName].taskRunningID = taskRunningID;
        bmapMutex.unlock();
    }

    printf("ok to sub the pre triggure and post triggure for (%s)\n", b->taskName.data());

    return;
}
