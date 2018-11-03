
// get one free port of the system
// the method should be thread safety, it needed to be used by multiple threads


#include <stdio.h>
#include <string.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <string>

using namespace std;

int getFreePortNum(){

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

    if (getsockname(socket_desc, (struct sockaddr *)&server, &len) != -1){
        
        printf("port number %d\n", ntohs(server.sin_port));
        close(socket_desc);
        return int(ntohs(server.sin_port));

    }else{

        printf("failed to get port");
        return -1;
    }

}
