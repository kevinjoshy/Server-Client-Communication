#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <assert.h>


#define PORT         "3100"
#define DEFAULT_HOST "localhost"
#define BUF_LEN       30

#define handle_error(msg) \
           do { perror(msg); exit(-1); } while (0)

int main(int argc,char **argv)
{
    char* servername = DEFAULT_HOST;

    if( argc > 1 ) servername = argv[1];

    // prepare hints for getaddrinfo
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;


    // get server address info
    struct addrinfo *servinfo;
    int res = getaddrinfo(servername, PORT, &hints, &servinfo);
    if( res != 0 )
        handle_error("client: getaddrinfo error");

    // create socket
    int sock_fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if( sock_fd == -1 )
        handle_error("client: socket error");

    // connect to the server
    res = connect(sock_fd, servinfo->ai_addr, servinfo->ai_addrlen); // loop through servinfo list if more than one address...
    if( res == -1 )
        handle_error("client: connect error");

    // cleanup
    freeaddrinfo(servinfo); // done with this struct

    FILE* rx = fdopen(sock_fd, "r");
    FILE* tx = fdopen(dup(sock_fd), "w");
    char sendline[BUF_LEN + 1];
    char recvline[BUF_LEN + 1];
    // exchange messages
    while (1) {
        // read and print server's response
        fgets(recvline, BUF_LEN, rx );
        if( feof(rx) ) {
            break;
        }

        printf("%s", recvline);

        // fgets reads in the newline and adds NUL terminator
        fgets(sendline, sizeof(sendline), stdin);
        if( feof(stdin) ) {
            break;
        }

        // write message to the socket
        fprintf(tx,"%s", sendline);
        fflush(tx);
        if( feof(tx) ) {
            break;
        }
    }

    fclose(rx);
    fclose(tx);

    exit(0);
}

