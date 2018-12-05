

#include <iostream>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <fstream>
#include <iostream>

#include <dirent.h>
#include <system_error>

#include <sys/stat.h>
#include <vector>
#include <map>
#include <dirent.h>
#include <system_error>
#include <cstring>
#include <mutex>

#include "./groupManager.h"
#include "../dlm/dlm.h"



using namespace std;

const string gm_multinodeip("./multinodeip");

const string gm_FreePool("./multinodeip/FreePool");

//it seems the coordinator is useless
//coordinator need to select the load info in current group !!!
//and decide if it is needed to expand the group
const string gm_coordinatorDir("coordinator");

const string gm_workerDir("worker");

string GM_INTERFACE("eno1");

int gm_requiredGroupSize;
int gm_groupNumber;
int gm_rank;

string SERVERSTATUS;

string status_coor("coordinator");
string status_worker("worker");
string status_free("freepool");

//current cluster dir
string GM_CLUSTERDIR;

mutex workerAddrMapLock;
map<string, vector<string>> workerAddrMap;

//all the coordinator Addr
map<string, string> coordinatorAddrMap;

bool loadCoordinator = false;

//the Dir could be the coordinator or the worker ./multinodeip/cluster0/worker
vector<string> loadAddrInDir(string Dir)
{
    // only init the client in clusterDir
    string dir = Dir;
    DIR *dp;
    vector<string> AddrList;
    struct dirent *entry;
    struct stat statbuf;
    try
    {
        if ((dp = opendir(dir.data())) == NULL)
        {
            printf("Can`t open directory %s\n", dir.data());
            return AddrList;
        }

        while ((entry = readdir(dp)) != NULL)
        {

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }

            //printf("load addr name %s\n", entry->d_name);
            AddrList.push_back(string(entry->d_name));
        }
        //change to the upper dir
        //printf("debug id %d load finish\n", GETIPCOMPONENTID);
        closedir(dp);
        return AddrList;
    }
    catch (const std::system_error &e)
    {
        std::cout << "getip Caught system_error with code " << e.code()
                  << " meaning " << e.what() << '\n';
        exit(1);
    }
}

//put addr into the map from the disk
//identity is event message here
//TODO use notification to update the clients
//only call this when there is new node add into group
void updateWorkerAddrMap(string clusterDir)
{

    //add lock
    //load the file in worker dir
    //range worker dir
    //if there is new file, add into map

    //number of the cluster in current dir
    //only update specific clusterDir
    int size;
    getLock(clusterDir, clusterDir);

    string workerDir = clusterDir + "/" + gm_workerDir;
    string coordinatorDir = clusterDir + "/" + gm_coordinatorDir;
    vector<string> workerAddr = loadAddrInDir(workerDir);
    vector<string> coordinatorAddr = loadAddrInDir(coordinatorDir);

    releaseLock(clusterDir);

    //first time to load
    if (loadCoordinator == false)
    {
        int corSize = coordinatorAddr.size();
        for (int j = 0; j < corSize; j++)
        {
            workerAddr.push_back(coordinatorAddr[j]);
        }
        workerAddrMapLock.lock();
        workerAddrMap[clusterDir] = workerAddr;
        workerAddrMapLock.unlock();
        loadCoordinator = true;
    }

    return;
}

string getClusterDir(int currentID)
{

    //printf("init size of cluster is %d\n", INITGROUPSIZE);
    int clusterIndex = currentID % gm_groupNumber;

    //when write ip into the dir
    //use the path from the ./multinodeip

    string clusterDir = gm_multinodeip + "/cluster" + to_string(clusterIndex);

    //printf("component id %d get dir %s\n", currentID, clusterDir.data());

    GM_CLUSTERDIR = clusterDir;

    return clusterDir;
}

int getFileNumInDir(string DirPath)
{

    //assume that all server component have been started properly
    string dir = DirPath;
    int FileNum = 0;
    DIR *dp;
    struct dirent *entry;
    if ((dp = opendir(dir.data())) == NULL)
    {
        //cluster dir not exist
        return 0;
    }

    while ((entry = readdir(dp)) != NULL)
    {

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        FileNum++;
    }
    //change to the upper dir
    //printf("there are %d second level cluster\n", FileNum);
    closedir(dp);

    return FileNum;
}

void recordIPPortWithoutFile(string &ipstr, string port)
{
    int n;
    struct ifreq ifr;
    //assume the network interface exist

    n = socket(AF_INET, SOCK_DGRAM, 0);
    //Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;
    //Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name, GM_INTERFACE.data(), IFNAMSIZ - 1);
    ioctl(n, SIOCGIFADDR, &ifr);
    close(n);
    //display result

    char *ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

    ipstr = string(ip);

    printf("debug ipstr %s", ipstr.data());
    return;
}

vector<string> getWorkAddr()
{
    //get the addr of all the worker in current group by groupid
    vector<string> test;
    return test;
}

void createDir(string DirPath)
{

    //if there is no Dir in current Dir Path, create
    fstream dirfile;
    dirfile.open(DirPath.data(), ios::in);
    if (!dirfile)
    {
        while (1)
        {
            const int dir_err = mkdir(DirPath.data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (dir_err < 0)
            {
                printf("Error creating directory %s, retry...", DirPath.data());
                usleep(10000);
                dirfile.open(DirPath.data(), ios::in);
                if (dirfile)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }
    dirfile.close();
    return;
}

//RequiredDirSize is the group size sending by input parameters
void recordIPortIntoClusterDir(string &ipstr, string port, string clusterDir, int RequiredDirSize)
{
    int n;
    struct ifreq ifr;
    //assume the network interface exist

    n = socket(AF_INET, SOCK_DGRAM, 0);
    //Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;
    //Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name, GM_INTERFACE.data(), IFNAMSIZ - 1);
    ioctl(n, SIOCGIFADDR, &ifr);
    close(n);
    //display result
    char *ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

    string dirFirst = gm_multinodeip;

    string dirSecond;

    if (gm_rank >= gm_groupNumber * gm_requiredGroupSize)
    {
        dirSecond = gm_FreePool;
    }
    else
    {
        dirSecond = clusterDir;
    }

    createDir(dirFirst);

    createDir(dirSecond);

    string dirThird;

    string finalDir;

    //if rank==group_num
    //put them in coordinator clusterrank
    //else put them in worker rank%group_num
    //if rank > groupNum*groupSize
    //put them in freePool

    if (gm_rank < gm_groupNumber)
    {
        //coordinator dir
        dirThird = dirSecond + "/" + gm_coordinatorDir;
        createDir(dirThird);
        finalDir = dirThird;
        SERVERSTATUS = status_coor;
    }
    else if (gm_rank >= gm_groupNumber && gm_rank < (gm_groupNumber * gm_requiredGroupSize))
    {
        //worker dir
        dirThird = dirSecond + "/" + gm_workerDir;
        createDir(dirThird);
        finalDir = dirThird;
        SERVERSTATUS = status_worker;
    }
    else
    {
        finalDir = dirSecond;
        SERVERSTATUS = status_free;
    }

    char addrFile[100];
    sprintf(addrFile, "%s/%s:%s", finalDir.data(), ip, port.data());

    FILE *fpt = fopen(addrFile, "w");

    if (fpt == NULL)
    {
        printf("failed to create %s\n", addrFile);
        return;
    }

    fclose(fpt);
    ipstr = string(ip);
    return;
}

/////////////function of coordinator//////////////

//node attach into specific group
bool nodeAttach(string dir, string nodeAddr)
{
    //add nodeAddr into the string dir (cluster dir)
    string workerDirPath = dir + "/" + gm_workerDir;
    char addrFile[100];
    sprintf(addrFile, "%s/%s", workerDirPath.data(), nodeAddr.data());

    FILE *fpt = fopen(addrFile, "w");

    if (fpt == NULL)
    {
        printf("failed to create %s for node attach\n", addrFile);
        return false;
    }

    fclose(fpt);

    //TODO update cluster Dir

    return true;
}

//get freenode from the FreePool
vector<string> getFreeNodeList(int needNum)
{

    vector<string> serverAddrList;

    //get needNum freeNode from the freePool
    string dir = gm_FreePool;
    DIR *dp;
    struct dirent *entry;
    string nodeAddr;
    if ((dp = opendir(dir.data())) == NULL)
    {
        //cluster dir not exist
        nodeAddr = "";
        printf("failed to open dir\n");
        closedir(dp);
        return serverAddrList;
    }

    int fileNum = 0;

    while ((entry = readdir(dp)) != NULL)
    {

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        else
        {
            nodeAddr = string(entry->d_name);
            serverAddrList.push_back(nodeAddr);



            fileNum++;

            if (fileNum == needNum)
            {
                break;
            }
        }
    }
    //change to the upper dir
    //printf("there are %d second level cluster\n", FileNum);
    closedir(dp);

    //delete addr in freepool

    // don't delete for debugging
    for (int i = 0; i < serverAddrList.size(); i++)
    {
        nodeAddr = serverAddrList[i];
        string fileDir = gm_FreePool + "/" + nodeAddr;
        if (remove(fileDir.data()) != 0)
        {
            printf("Error deleting file %s in freePool\n", fileDir.data());
        }
        printf("move addr %s out of freepool\n", nodeAddr.data());
    }

    //TODO after moving file, the specific server should be notified to modify the groupInfo
    //server should know which group it located in
    //or maintain specific data structure from ip to group info on disk
    //or those info can be updated by coordinator

    return serverAddrList;
}

//node detach from current group
//the method shoule be called by higher layer
//there shoule be no ongoing subscription(or ongoing checking notification)
//when this method is called
string nodeDetach(string cluster, string nodeAddr)
{
    string detachAddr;

    //delete the worker from the worker dir in this cluster

    //there should be no subscription on this node

    return detachAddr;
}

//use lock when calling this method
void updateworkerClients()
{

    //update all the clients info for worker
    //delete the old one and add the new one
}

string clientClusterDir()
{

    //printf("component num is %d\n", GETIPCOMPONENTNUM);
    //printf("component num per cluster is %d\n", SERVERCLUSTERNUM);

    int clusternum = gm_requiredGroupSize;

    if (clusternum == 0)
    {
        clusternum++;
    }

    int clusterIndex = gm_rank % clusternum;

    string clusterDir = gm_multinodeip + "/cluster" + to_string(clusterIndex);

    return clusterDir;
}

int getFreePortNum()
{

    int socket_desc, new_socket, c;
    struct sockaddr_in server;
    char message[100];

    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(0);

    //server function: bind, listen , accept

    //Bind

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("bind failed");
        return 1;
    }

    socklen_t len = sizeof(server);

    if (getsockname(socket_desc, (struct sockaddr *)&server, &len) != -1)
    {

        //printf("port number %d\n", ntohs(server.sin_port));
        close(socket_desc);
        return int(ntohs(server.sin_port));
    }
    else
    {

        printf("failed to get port");
        return -1;
    }
}

string parsePort(string peerURL)
{

    int startPosition = peerURL.find("ipv4:");

    //delete ipv4:
    peerURL.erase(startPosition, 5);

    //delete the port suffix
    int len = peerURL.length();

    int endPosition = peerURL.find(":");

    //printf("start position of : (%d) original str %s\n", startPosition, peerURL.data());

    peerURL.erase(0, endPosition + 1);

    return peerURL;
}

string parseIP(string peerURL)
{

    int startPosition = peerURL.find("ipv4:");

    //delete ipv4:
    peerURL.erase(startPosition, 5);

    //delete the port suffix
    int len = peerURL.length();

    startPosition = peerURL.find(":");

    //printf("start position of : (%d) original str %s\n", startPosition, peerURL.data());

    peerURL.erase(startPosition, len - startPosition);

    return peerURL;
}

/*

int main()
{

    gm_groupNumber = 4;

    gm_requiredGroupSize = 2;

    string command = "rm -rf " + gm_multinodeip;

    system(command.data());

    //simulate every client

    for (int i = 0; i < 10; i++)
    {

        string clusterDir = getClusterDir(i);

        int freePort = getFreePortNum();

        string ServerPort = to_string(freePort);

        string ServerIP;

        recordIPortIntoClusterDir(ServerIP, ServerPort, clusterDir, gm_requiredGroupSize);

        //printf("file Num for clusterDir %s is %d\n",clusterDir.data(), fileNum);
    }

    //get freenode
    string freeAddr;
    bool ifgetOk = getFreeNode(freeAddr);
    printf("get free Addr %s ifok %d\n", freeAddr.data(), ifgetOk);

    if (ifgetOk)
    {
        string clusterDir = gm_multinodeip + "/cluster0";
        nodeAttach(clusterDir, freeAddr);
    }

    freeAddr = "";
    ifgetOk = getFreeNode(freeAddr);
    printf("get free Addr %s ifok %d\n", freeAddr.data(), ifgetOk);
    freeAddr = "";
    ifgetOk = getFreeNode(freeAddr);
    printf("get free Addr %s ifok %d\n", freeAddr.data(), ifgetOk);
    vector<string> testStr = loadAddrInDir(gm_multinodeip);
    int size = testStr.size();
    for (int i = 0; i < size; i++)
    {
        printf("dir name %s\n", testStr[i].data());
    }
}

*/