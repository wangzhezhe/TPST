#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>

#include "getip.h"

#define INTERFACE "eno1"

//#define INTERFACE "lo"

using namespace std;

string parseIP(string peerURL)
{

    int startPosition = peerURL.find("ipv4:");

    //delete ipv4:
    peerURL.erase(startPosition, 5);

    //delete the port suffix
    int len = peerURL.length();

    startPosition = peerURL.find(":");

    printf("start position of : (%d) original str %s\n", startPosition, peerURL.data());

    peerURL.erase(startPosition, len - startPosition);

    return peerURL;
}

void recordIPPortWithoutFile(string &ipstr, string port)
{
    int n;
    struct ifreq ifr;
    //assume the network interface exist
    //char array[] = "eth5";
    char array[] = INTERFACE;

    n = socket(AF_INET, SOCK_DGRAM, 0);
    //Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;
    //Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name, array, IFNAMSIZ - 1);
    ioctl(n, SIOCGIFADDR, &ifr);
    close(n);
    //display result
    char *ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

    ipstr = string(ip);
}

void recordIPPort(string &ipstr, string port)
{
    int n;
    struct ifreq ifr;
    //assume the network interface exist
    //char array[] = "eth5";
    char array[] =INTERFACE;
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
    strncpy(ifr.ifr_name, array, IFNAMSIZ - 1);
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

    return 0;
}
*/
