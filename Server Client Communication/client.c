#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>

#include "command.h"

void doLSCommand(int sid);
void doExitCommand(int sid);
void doGETCommand(int sid);
void doPUTCommand(int sid);
void doLLScmd();
void doFindlsize(char* filename);
void doFindServerFileSize(int sid, char* filename);
void domGet(int sid, char* filename, int cur);


void checkError(int status, int line) {
   if (status < 0) {
      printf("socket error(%d)-%d: [%s]\n",getpid(),line,strerror(errno));
      exit(-1);
   }
}


int main(int argc,char* argv[]) {
   char* DEFAULT_HOST = "localhost";
   char* DEFAULT_PORT = "3131";
   if (argc == 2) {
      DEFAULT_HOST = argv[1];
   }
   if (argc == 3) {
      DEFAULT_HOST = argv[1];
      DEFAULT_PORT = argv[2];
   }

   // prepare hints for getaddrinfo
   struct addrinfo hints;
   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;

   // get server address info
   struct addrinfo *servinfo;
   int res = getaddrinfo(DEFAULT_HOST, DEFAULT_PORT, &hints, &servinfo);
   checkError(res,__LINE__);

   // create socket
   int sid = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
   checkError(sid,__LINE__);

   // connect to the server
   res = connect(sid, servinfo->ai_addr, servinfo->ai_addrlen); // loop through servinfo list$
   checkError(res,__LINE__);

   // cleanup
   freeaddrinfo(servinfo); // done with this struct

   int done = 0;
   do {
      char opcode[32];
      scanf("%s",opcode);
      if (strncmp(opcode,"lsize",4) == 0) {
         char filename[256];
         scanf("%s",filename);
         doFindlsize(filename);
      } else if (strncmp(opcode,"ls",2) == 0) {
         doLSCommand(sid);
      } else if (strncmp(opcode,"get",3)==0) {
         doGETCommand(sid);
      } else if (strncmp(opcode,"put",3)==0) {
         doPUTCommand(sid);
      } else if (strncmp(opcode,"exit",4) == 0) {
         doExitCommand(sid);
         done = 1;
      } else if (strncmp(opcode,"lls",3) == 0) {
         doLLScmd();
      } else if (strncmp(opcode,"size",4) == 0) {
         char filename[32];
         scanf("%s",filename);
         doFindServerFileSize(sid, filename);
      } else if (strncmp(opcode,"mget",4) == 0) {
         //get list of files to get 
         char files[256];
         scanf("%[^\n]",files);
         char *file;
   
         /* get the first token */
         file = strtok(files, " ");
         int cur = 0;
         /* walk through other tokens */
         while( file != NULL ) {
            domGet(sid, file, cur);
            cur=1;
            file = strtok(NULL, " ");
         }
      }
   } while(!done);

   return 0;
} // END MAIN

void domGet(int sid, char* fName, int cur) {
   Command c;
   Payload p;
   int status;
   c.code = htonl(CC_GET);

   strncpy(c.arg,fName,255);
   c.arg[255] = 0;
   status = send(sid,&c,sizeof(c),0);checkError(status,__LINE__);
   status = recv(sid,&p,sizeof(p),0);checkError(status,__LINE__);
   p.code = ntohl(p.code);
   p.length = ntohl(p.length);
   receiveFileOverSocket(sid,c.arg,".download",p.length);
   if (cur == 0) {
      printf("transfer done\n");
   }
}

void doFindServerFileSize(int sid, char* filename) {
   Command c;
   Payload p;
   int status;
   c.code = htonl(F_SIZE);
   strncpy(c.arg,filename,255);
   c.arg[255] = 0;
   status = send(sid,&c,sizeof(c),0);checkError(status,__LINE__);
   status = recv(sid,&p,sizeof(p),0);checkError(status,__LINE__);
   p.code = ntohl(p.code);
   p.length = ntohl(p.length);
   printf("File size of %s is %d bytes\n",filename, p.length);
}

void doFindlsize(char* filename) {
   // printf("%s\n",filename);
   FILE* fp;
   fp = fopen(filename,"r");
   fseek(fp, 0L, SEEK_END);
   long endPointer = ftell(fp);
   fseek(fp, 0L, SEEK_SET);
   long startPointer = ftell(fp);
   printf("File size of %s is %ld bytes\n",filename, endPointer - startPointer);
   fclose(fp);
}

void doLLScmd() {
   char* msg = makeFileList(".");
   printf("[\n%s]\n",msg);
}

void doLSCommand(int sid) {
   Command c;
   Payload p;
   int status;
   c.code = htonl(CC_LS); // CC_LS = 0
   memset(c.arg, 0, sizeof(c.arg));
   status = send(sid,&c,sizeof(c),0); checkError(status,__LINE__);
   status = recv(sid,&p,sizeof(p),0); checkError(status,__LINE__);
   p.code = ntohl(p.code);
   p.length = ntohl(p.length);
   int rec  = 0,rem = p.length;
   char* buf = malloc(sizeof(char) * p.length);
   while (rem != 0) {
      int nbrecv = recv(sid,buf+rec,rem,0);checkError(status,__LINE__);
      rec += nbrecv;
      rem -= nbrecv;
   }
   if(p.code == PL_TXT)
      printf("Got: [\n%s]\n",buf);
   else {
      printf("Unexpected payload: %d\n",p.code);
   }
   free(buf);
}

void doGETCommand(int sid) {
   Command c;
   Payload p;
   int status;
   c.code = htonl(CC_GET);
   // printf("Give a filename:");
   char fName[256];
   scanf("%s",fName);
   strncpy(c.arg,fName,255);
   c.arg[255] = 0;
   status = send(sid,&c,sizeof(c),0);checkError(status,__LINE__);
   status = recv(sid,&p,sizeof(p),0);checkError(status,__LINE__);
   p.code = ntohl(p.code);
   p.length = ntohl(p.length);
   receiveFileOverSocket(sid,c.arg,".download",p.length);
   printf("transfer done\n");
}

void doPUTCommand(int sid) {
   Command c;
   Payload p;
   int status;
   c.code = htonl(CC_PUT);
   // printf("Give a local filename:");
   char fName[256];
   scanf("%s",fName);
   strncpy(c.arg,fName,255);
   c.arg[255] = 0;
   status = send(sid,&c,sizeof(c),0);checkError(status,__LINE__);
   int fs = getFileSize(c.arg);
   p.code   = ntohl(PL_FILE);
   p.length = ntohl(fs);
   status = send(sid,&p,sizeof(p),0);checkError(status,__LINE__);
   sendFileOverSocket(c.arg,sid);
   printf("transfer done\n");
}

void doExitCommand(int sid) {
   Command c;
   Payload p;
   int status;
   c.code = htonl(CC_EXIT);
   memset(c.arg, 0, sizeof(c.arg));
   status = send(sid,&c,sizeof(c),0);checkError(status,__LINE__);
}
