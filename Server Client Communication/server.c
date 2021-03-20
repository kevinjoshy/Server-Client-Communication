#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <dirent.h>


#include "command.h"

#define BACKLOG 10

void checkError(int status)
{
   if (status < 0) {
      printf("socket error(%d): [%s]\n",getpid(),strerror(errno));
      exit(-1);
   }
}

void handleNewConnection(int chatSocket);

int main(int argc,char* argv[]) {
   char* DEFAULT_PORT = "3131";
   if (argc == 2) {
      DEFAULT_PORT = argv[1];
   }

   // prepare hints for getaddrinfo
   struct addrinfo hints;
   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE;     // fill in my IP

   // get server address info
   struct addrinfo *servinfo;
   int res = getaddrinfo(NULL, DEFAULT_PORT, &hints, &servinfo);
   checkError(res);

   // create socket
   int sid = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
   checkError(sid);

   // set socket options to allow port reuse
   int yes = 1;
   res = setsockopt(sid, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
   checkError(res);

   // bind socket to server address
   res = bind(sid, servinfo->ai_addr, servinfo->ai_addrlen);
   checkError(res);

   // cleanup
   freeaddrinfo(servinfo); // done with this structure

   // listen on that socket for "Let's talk" message. No more than 10 pending at once
   res = listen(sid,BACKLOG);
   checkError(res);

   while( 1 ) {
      // accept incoming connection
      int chatSocket = accept(sid, (struct sockaddr*)NULL, NULL);
      checkError(chatSocket);
      printf("We accepted a socket: %d\n",chatSocket);

      pid_t child = fork();
      if (child == 0) {
         //int st = close(sid);
         handleNewConnection(chatSocket);
         close(chatSocket);
         return -1; /* Report that I died voluntarily */
      } else if (child > 0) {
         printf("Created a child: %d\n",child);
         close(chatSocket);
         int status = 0;
         pid_t deadChild; // reap the dead children (as long as we find some)
         do {
            deadChild = waitpid(0,&status,WNOHANG);checkError(deadChild);
            if (deadChild > 0)
               printf("Reaped %d\n",deadChild);
         } while (deadChild > 0);
      }
   }
   return 0;
}

void handleNewConnection(int chatSocket)
{
   // This is the child process.
   int done = 0;
   do {
      Command c;
      int status = recv(chatSocket,&c,sizeof(Command),0);checkError(status);
      c.code = ntohl(c.code);
      switch(c.code){
         case CC_LS: {
            Payload p;
            char* msg = makeFileList(".");
            p.code = htonl(PL_TXT);
            p.length = htonl(strlen(msg)+1);
            status = send(chatSocket,&p,sizeof(Payload),0);checkError(status);
            int rem = strlen(msg)+1,sent = 0;
            while (rem != 0) {
               status = send(chatSocket,msg+sent,rem,0);
               rem -= status;
               sent += status;
            }
            free(msg);
         }break;
         case CC_GET: { // send the named file back to client
            Payload p;
            int fileSize = getFileSize(c.arg);
            p.code = htonl(PL_FILE);
            p.length = htonl(fileSize);
            status = send(chatSocket,&p,sizeof(Payload),0);checkError(status);
            sendFileOverSocket(c.arg,chatSocket);
            printf("File [%s] sent\n",c.arg);
         }break;
         case CC_PUT: {// save locally a named file sent by the client
            Payload p;
            status = recv(chatSocket,&p,sizeof(p),0);checkError(status);
            p.code = ntohl(p.code);
            p.length = ntohl(p.length);
            receiveFileOverSocket(chatSocket,c.arg,".upload",p.length);
            printf("File [%s] received\n",c.arg);
         }break;
         case F_SIZE: {
            Payload p;
            int fileSize = getFileSize(c.arg);
            p.code = htonl(PL_TXT);
            p.length = htonl(fileSize);
            status = send(chatSocket,&p,sizeof(Payload),0);checkError(status);
            printf("Size of file [%s] sent\n",c.arg);
         }break;
         case CC_EXIT:
            done = 1;
            break;
      }
   } while (!done);
}
