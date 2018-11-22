
#include <string>
#include "../groupManager/groupManager.h"
#include <stdio.h>  /* printf, scanf, puts, NULL */
#include <stdlib.h> /* srand, rand */
#include <time.h>   /* time */

using namespace std;

//only for test using for the event in following format
//eventmessage_i
string getClusterDirFromEventMsg(string eventMsg)
{

    int startPosition = eventMsg.find("_");

    eventMsg.erase(0, startPosition+1);

    string clusterDir = "cluster"+eventMsg;

    clusterDir = gm_multinodeip + "/" + clusterDir;

    return clusterDir;
}

//use int i to simulate event with suffix number i
string getClusterDirFromEvent(int i)
{

    int clusterId = 0;

    int clusterNum = gm_groupNumber;

    clusterId = i % clusterNum;

    string clusterDir = gm_multinodeip + "/" + "cluster" + to_string(clusterId);

    return clusterDir;
}

//use random function is also ok
//use srand (time(0)) before calling rand()
int getServerIdFromAddr(int serverNum)
{

    int serverId = rand() % serverNum;
    return serverId;
}

int testGetId(int num){
    srand(time(0));
    int id = getServerIdFromAddr(num);
    printf("server id is %d\n",id);
}


/*
int main()
{
  
    int i = 0;
    int size = 10;
    

    for (i = 0; i < 10; i++)
    {
        testGetId(i);
        int id = getServerIdFromAddr(size);
        
    }
    

   //string message= "fakeabc_0";
   //getClusterDirFromEventMsg(message);
}
*/
