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

using namespace std;

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

vector<string> loadMultiNodeIPPort()
{
    //the string in vector is ip:port
    string dir = string("./multinodeip");
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
        closedir(dp);
    }
    catch (const std::system_error &e)
    {
        std::cout << "getip Caught system_error with code " << e.code()
                  << " meaning " << e.what() << '\n';
        exit(1);
    }

    return multiNodeAddr;
}

// put ip:port into multinodeip dir
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

    string dir = string("./multinodeip");

    //if dir is created

    fstream dirfile;
    dirfile.open(dir.data(), ios::in);
    if (!dirfile)
    {
        while (1)
        {
            const int dir_err = mkdir(dir.data(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (dir_err < 0)
            {
                printf("Error creating directory %s, retry...", dir.data());
                usleep(10000);
                dirfile.open(dir.data(), ios::in);
                if(dirfile){
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }

    char addrFile[100];
    sprintf(addrFile, "%s/%s:%s", dir.data(), ip, port.data());

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

/*
int main()
{
    /*
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
    
    string peerurl=string("ipv4:123.123.123.123:2");

    string clientip=parseIP(peerurl);

    printf("clientip (%s)\n",clientip.data());

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
