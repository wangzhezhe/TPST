#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <vector>
#include "getip.h"
#include <dirent.h>
#include <system_error>

//#define INTERFACE "lo"

//#define INTERFACE "en0"

string INTERFACE("eno1");

int GETIPCOMPONENTNUM;

//number of instance in every cluster
//default value is 8
int GETIPNUMPERCLUSTER = 8;
int GETIPCOMPONENTID;
int SERVERCLUSTERNUM;

string multinodeip("./multinodeip");

using namespace std;

//get how many second level small cluster
int getsubClientNum()
{
    //assume that all server component have been started properly
    string dir = multinodeip;
    int clusterNum = 0;
    DIR *dp;
    struct dirent *entry;
    if ((dp = opendir(dir.data())) == NULL)
    {
        printf("Can`t open directory %s\n", dir.data());
        exit(0);
    }

    while ((entry = readdir(dp)) != NULL)
    {

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        clusterNum++;
    }
    //change to the upper dir
    printf("there are %d second level cluster\n", clusterNum);
    closedir(dp);

    return clusterNum;
}

// input
string getClusterDirByTopicId(int topicid)
{

    //get number of subcluster
    int subclusterNum = getsubClientNum();
    //get cluster id from the topicid
    if (subclusterNum == 0)
    {
        printf("error, the subclusternum is 0\n");
        exit(0);
    }
    int clusterId = topicid % subclusterNum;

    //return clusterId

    string clusterDir = multinodeip + "/cluster" + to_string(clusterId);

    printf("get dir %s\n", GETIPCOMPONENTID, clusterDir.data());

    return clusterDir;
}

string clientClusterDir()
{

    //printf("component num is %d\n", GETIPCOMPONENTNUM);
    //printf("component num per cluster is %d\n", SERVERCLUSTERNUM);

    int clusternum = SERVERCLUSTERNUM;

    if (clusternum == 0)
    {
        clusternum++;
    }

    int clusterIndex = GETIPCOMPONENTID % clusternum;

    string clusterDir = multinodeip + "/cluster" + to_string(clusterIndex);

    printf("component id %d get dir %s\n", GETIPCOMPONENTID, clusterDir.data());

    return clusterDir;
}

string getClusterDir()
{

    printf("component num is %d\n", GETIPCOMPONENTNUM);
    printf("component num per cluster is %d\n", GETIPNUMPERCLUSTER);

    int clusternum = GETIPCOMPONENTNUM / GETIPNUMPERCLUSTER;

    if (clusternum == 0)
    {
        clusternum++;
    }

    int clusterIndex = GETIPCOMPONENTID % clusternum;

    string clusterDir = multinodeip + "/cluster" + to_string(clusterIndex);

    printf("component id %d get dir %s\n", GETIPCOMPONENTID, clusterDir.data());

    return clusterDir;
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

string parsePort(string peerURL)
{

    int startPosition = peerURL.find("ipv4:");

    //delete ipv4:
    peerURL.erase(startPosition, 5);

    //delete the port suffix
    int len = peerURL.length();

    int endPosition = peerURL.find(":");

    //printf("start position of : (%d) original str %s\n", startPosition, peerURL.data());

    peerURL.erase(0, endPosition+1);

    return peerURL;
}


vector<string> loadMultiNodeIPPortByClusterDir(string clusterDir)
{
    // only init the client in clusterDir
    string dir = clusterDir;
    DIR *dp;
    vector<string> multiNodeAddr;
    struct dirent *entry;
    struct stat statbuf;
    try
    {
        if ((dp = opendir(dir.data())) == NULL)
        {
            printf("Can`t open directory %s\n", dir.data());
            exit(0);
        }

        while ((entry = readdir(dp)) != NULL)
        {

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }

            //printf("load addr name %s\n", entry->d_name);
            multiNodeAddr.push_back(string(entry->d_name));
        }
        //change to the upper dir
        //printf("debug id %d load finish\n", GETIPCOMPONENTID);
        closedir(dp);
        return multiNodeAddr;
    }
    catch (const std::system_error &e)
    {
        std::cout << "getip Caught system_error with code " << e.code()
                  << " meaning " << e.what() << '\n';
        exit(1);
    }
}

vector<string> loadMultiNodeIPPort(string identity)
{
    //the string in vector is ip:port
    //string dir = string("./multinodeip");
    string dir;
    if (identity.compare("client") == 0)
    {
        dir =clientClusterDir();
    }
    else
    {
        dir = getClusterDir();
    }

    DIR *dp;
    vector<string> multiNodeAddr;
    struct dirent *entry;
    struct stat statbuf;
    try
    {
        if ((dp = opendir(dir.data())) == NULL)
        {
            printf("Can`t open directory %s\n", dir.data());
            exit(0);
        }

        while ((entry = readdir(dp)) != NULL)
        {

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }

            printf("load addr name %s\n", entry->d_name);
            multiNodeAddr.push_back(string(entry->d_name));
        }
        //change to the upper dir
        printf("debug id %d load finish\n", GETIPCOMPONENTID);
        closedir(dp);
        return multiNodeAddr;
    }
    catch (const std::system_error &e)
    {
        std::cout << "getip Caught system_error with code " << e.code()
                  << " meaning " << e.what() << '\n';
        exit(1);
    }
}

// put ip:port into multinodeip/clusterid dir
// send the ip to ipstr
void recordIPortForMultiNode(string &ipstr, string port)
{
    int n;
    struct ifreq ifr;
    //assume the network interface exist

    n = socket(AF_INET, SOCK_DGRAM, 0);
    //Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;
    //Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name, INTERFACE.data(), IFNAMSIZ - 1);
    ioctl(n, SIOCGIFADDR, &ifr);
    close(n);
    //display result
    char *ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

    string dirFirst = multinodeip;

    string dirSecond = getClusterDir();

    //if dir is created

    fstream dirfile;
    dirfile.open(dirFirst.data(), ios::in);
    if (!dirfile)
    {
        while (1)
        {
            const int dir_err = mkdir(dirFirst.data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (dir_err < 0)
            {
                printf("Error creating directory %s, retry...", dirFirst.data());
                usleep(10000);
                dirfile.open(dirFirst.data(), ios::in);
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

    fstream dirfilesecond;
    dirfilesecond.open(dirSecond.data(), ios::in);
    if (!dirfilesecond)
    {
        while (1)
        {
            const int dir_err = mkdir(dirSecond.data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (dir_err < 0)
            {
                printf("Error creating directory %s, retry...", dirSecond.data());
                usleep(10000);
                dirfilesecond.open(dirSecond.data(), ios::in);
                if (dirfilesecond)
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
    dirfilesecond.close();

    char addrFile[100];
    sprintf(addrFile, "%s/%s:%s", dirSecond.data(), ip, port.data());

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

void recordIPPortWithoutFile(string &ipstr, string port)
{
    int n;
    struct ifreq ifr;
    //assume the network interface exist

    n = socket(AF_INET, SOCK_DGRAM, 0);
    //Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;
    //Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name, INTERFACE.data(), IFNAMSIZ - 1);
    ioctl(n, SIOCGIFADDR, &ifr);
    close(n);
    //display result
    
    char *ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

    ipstr = string(ip);

    printf("debug ipstr %s",ipstr.data());
    return;
}

void recordIPPort(string &ipstr, string port)
{
    int n;
    struct ifreq ifr;
    //assume the network interface exist
    FILE *fpt = fopen("./ipconfig", "w");

    if (fpt == NULL)
    {
        printf("failed to create ./ipconfig\n");
        return;
    }

    n = socket(AF_INET, SOCK_DGRAM, 0);
    //Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;
    //Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name, INTERFACE.data(), IFNAMSIZ - 1);
    ioctl(n, SIOCGIFADDR, &ifr);
    close(n);
    //display result
    char *ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    fprintf(fpt, "%s:%s\n", ip, port.data());

    fclose(fpt);
    ipstr = string(ip);
}

int loadIPPort(string configpath, string &ipstr, string &port)
{

    FILE *fp;
    char buf[256];
    if ((fp = fopen(configpath.data(), "r")) == NULL)
    { /* Open source file. */
        perror("fopen source-file");
        return 1;
    }

    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
        //printf("load ip:port (%s)\n", buf);
    }
    fclose(fp);

    //split ip:port
    const char *sep = ":";
    char *p;
    p = strtok(buf, sep);
    ipstr = string(p);

    p = strtok(NULL, sep);
    port = string(p);

    return 0;
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

        printf("port number %d\n", ntohs(server.sin_port));
        close(socket_desc);
        return int(ntohs(server.sin_port));
    }
    else
    {

        printf("failed to get port");
        return -1;
    }
}

/*
int main()
{
    
    string ipstr;
    string port=string("12345");
    recordIPPort(ipstr,port);
    string ip;

    string path=string("./ipconfig");
    loadIPPort(path,ip,port);

    printf("get ip (%s)\n",ip.data());
    printf("get port (%s)\n",port.data());
    
    string socketAddr = ip+":"+port;

    printf("socket addr (%s)\n",socketAddr.data());

    printf("-----test peerurl-----\n");

    
    
    string peerurl=string("ipv4:123.123.123.123:2345543");

    string clientip=parseIP(peerurl);

    printf("clientip (%s)\n",clientip.data());

    string clientPort=parsePort(peerurl);

    printf("clientPort (%s)\n",clientPort.data());

    
    string ipstr;
    string port = string("12345");
    recordIPortForMultiNode(ipstr, port);


    vector<string> multiaddr=loadMultiNodeIPPort();
    int size=0,i=0;
    size=multiaddr.size();

    for(i=0;i<size;i++){
        printf("node (%d) addr (%s)\n",i,multiaddr[i].data());
    }
    

    return 0;
}
*/