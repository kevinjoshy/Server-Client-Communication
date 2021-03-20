#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PORT      "3100"
#define DEFAULT_HOST "localhost"
#define BUF_LEN       30

#define BACKLOG 10

#define handle_error(msg) \
            do { perror(msg); exit(-1); } while (0)

int main(int argc,char* argv[]) {
    // prepare hints for getaddrinfo
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;  // Use wildcard IP address

    // get server address info
    struct addrinfo *servinfo;
    int status = getaddrinfo(DEFAULT_HOST, PORT, &hints, &servinfo);
    if( status != 0 )
        handle_error("server: getaddrinfo error");

    // create socket
    int listen_fd = socket(servinfo->ai_family, 
                        servinfo->ai_socktype, 
                        servinfo->ai_protocol);
    if (listen_fd == -1)
        handle_error("server: socket error");

    int yes = 1;
    status = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (status == -1)
        handle_error("server: setsockopt error");

    // bind socket to server address
    status = bind(listen_fd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (status == -1)
        handle_error("server: bind error");

    // cleanup
    freeaddrinfo(servinfo); // done with this struct

    // prepare socket for accepting incoming connections
    status = listen(listen_fd, BACKLOG); // BACKLOG is max num connections 
    if( status == -1 )
        handle_error("server: listen error");

    while (1) {
        // accept incoming connection
	    int comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);
	    if(comm_fd == -1)
	       handle_error("server: accept error");

	    fprintf(stderr, "Accepted new connection\n");

	    pid_t pid = fork();
        if ( pid == 0 ) { // child
            close(listen_fd);
            FILE* rx = fdopen(comm_fd, "r");
		    FILE* tx = fdopen(dup(comm_fd), "w");

            int sum = 0;
            int add = 0;
            char sendline[BUF_LEN];
            char recvline[BUF_LEN];
            while (1) {
                sprintf(sendline, "%d", sum);
                strcat(sendline, "\n");

                fprintf(tx,"%s", sendline);
                fflush(tx);
		        if(feof(tx)) {
		            break;
		        }

                fgets(recvline, BUF_LEN, rx);
		        if( feof(rx) ) {
		            break;
		        }

                if (strcmp(recvline,"exit\n")==0) { // check exit 
                    fprintf(tx, "Bye %d\n", sum);
                    fflush(tx);
                    break; 
                }

                if (strlen(recvline) > 30) {
                    fprintf(tx, "Error LongLine %d\n", sum);
                    fflush(tx);
                    continue;
                }

                int err = sscanf(recvline, "%d", &add);
                if (err != 1) {
                    fprintf(tx, "Error NaN %d\n", sum);
                    fflush(tx);
                    add = 0;
                    continue;
                }

                sum += add;
            }
            fclose(rx);
            fclose(tx);
            exit(0);
        }
        else { // Parent
            fprintf(stderr, "Created child with PID %d\n", pid);
            close(comm_fd);
            // reap dead children, if any
			int status;
            pid_t deadChild;
            do {
                deadChild = waitpid(-1, &status, WNOHANG);
                if (deadChild == -1)
                    handle_error("server: waitpid");
                if (deadChild > 0)
                    fprintf(stderr, "Reaped child with PID %d\n", deadChild);
            } while (deadChild > 0);
        }
    }

    exit(0);
}



