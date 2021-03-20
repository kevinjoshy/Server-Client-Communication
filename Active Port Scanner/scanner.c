#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// Declare any necessary macros and additional functions here
// TODO
#define handle_error(msg) \
            do { perror(msg); exit(-1); } while (0)

#define DEFAULT_HOST "localhost"
#define BUF_LEN       100

int main(int argc, char* argv[])
{
    char* servername = DEFAULT_HOST;
    //  TODO
    char *portStart;
    char *portEnd;
    if (argc==3) {
        portStart = argv[1];
        portEnd   = argv[2];
    }
    else if (argc==4) {
        servername = argv[1];
        portStart = argv[2];
        portEnd   = argv[3];
    }
    else {
        portStart = "20";
        portEnd   = "100";
    }

    // prepare hints for getaddrinfo
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;          // Basic for UNIX
    hints.ai_socktype = SOCK_STREAM;    // connection-based protocol.

    // get server address info
    struct addrinfo *servinfo;          // Gets the server info 
    
    for (int PORT = atoi(portStart); PORT <= atoi(portEnd); PORT++) {
        char strPORT[100];
        sprintf(strPORT, "%d", PORT);
        int status = getaddrinfo(servername, strPORT, &hints, &servinfo);
        if( status != 0 )
            handle_error("client: getaddrinfo error");

        // create socket
        int sock_fd = socket(servinfo->ai_family,
                            servinfo->ai_socktype,
                            servinfo->ai_protocol);
        if( sock_fd == -1 )
            handle_error("client: socket error");

        // Attempt to connect
        status = connect(sock_fd, 
                        servinfo->ai_addr, 
                        servinfo->ai_addrlen); 
        if (status < 0) {                // connection status 
            // printf("Port %d is NOT open.\n", PORT);
            close(sock_fd);
            freeaddrinfo(servinfo);
            continue;                    // Not open socket 
        }

        char *getPortType = "GET /index.html HTTP/1.1\n";
        char serverReply[5];
        serverReply[0] = 'I';
        // write message to the socket // include NUL terminator
        write(sock_fd, getPortType, strlen(getPortType) + 1);
        shutdown(sock_fd, 1); // Done writing to Server
        // read and print server's response
        read(sock_fd, serverReply, 4);
        shutdown(sock_fd, 0); // Done writing to Server

        serverReply[4] = '\0';
        if (strcmp(serverReply, "HTTP")==0) {
            printf("found active %s port: %d\n", serverReply, PORT);
        }
        else printf("found active non-HTTP port: %d\n", PORT);

        close(sock_fd);
        freeaddrinfo(servinfo);
    }

    exit(0);
}

