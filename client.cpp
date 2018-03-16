/*
 A simple client in the internet domain using TCP
 Usage: ./client hostname port filename (./client 192.168.0.151 10000 test.txt)
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>    // structures for stat
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>      // define structures like hostent
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include "JJP.h"

void error(const char *msg)
{
  perror(msg);
  exit(1);
}

void readFileContent(int fileFD, char** content, int* contentLen) {
  struct stat st;
  if(fstat(fileFD, &st) < 0) {
    error("ERROT: cannot read requested files stats");
  }

  int fileLen = st.st_size; //byte size of file
  char* fileStr = (char*) malloc(sizeof(char) * fileLen);

  int bytesTotal = 0;
  int bytesRead = 0;
  while((bytesRead = read(fileFD, (fileStr + bytesTotal), fileLen - bytesRead)) > 0) {
    std::cout << bytesTotal << std::endl;
    bytesTotal += bytesRead;
  }
  if(bytesRead < 0) {
    error("ERROR: cannot read from specified file");
    }

  //return contents and length
  *content = fileStr;
  *contentLen = fileLen;
  return;
}


int main(int argc, char *argv[])
{
    int portno, n;
    struct sockaddr_in myaddr, remaddr;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    char* server = argv[1];
    portno = atoi(argv[2]);
    JJP mJJP(AF_INET, SOCK_DGRAM, 0);

    memset((char *)&myaddr, 0, sizeof(myaddr));
  	myaddr.sin_family = AF_INET;
  	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  	myaddr.sin_port = htons(0);

  	if (mJJP.bind((struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
  		perror("bind failed");
  		return 0;
  	}

  	/* now define remaddr, the address to whom we want to send messages */
  	/* For convenience, the host address is expressed as a numeric IP address */
  	/* that we will convert to a binary format via inet_aton */

  	memset((char *) &remaddr, 0, sizeof(remaddr));
  	remaddr.sin_family = AF_INET;
  	remaddr.sin_port = htons(portno);
  	if (inet_aton(server, &remaddr.sin_addr)==0) {
  		fprintf(stderr, "inet_aton() failed\n");
  		exit(1);
  	}


    /*int requestedFD = open("test.txt", O_RDONLY);
    if( requestedFD >= 0) {
        char* fileContent = NULL;
        int fileLen = -1;
        std::cout << requestedFD << std::endl;
        readFileContent(requestedFD, &fileContent, &fileLen);

        mJJP.write(fileContent, fileLen);
      }*/


    n = mJJP.write("HelloHelloHelloHelloHello",strlen("HelloHelloHelloHelloHello"));  // write to the socket
    mJJP.write("HelloHelloHelloHelloHello",strlen("HelloHelloHelloHelloHello"));  // write to the socket
    mJJP.write("HelloHelloHelloHelloHello",strlen("HelloHelloHelloHelloHello"));  // write to the socket
    mJJP.write("HelloHelloHelloHelloHello",strlen("HelloHelloHelloHelloHello"));  // write to the socket
    mJJP.write("HelloHelloHelloHelloHello",strlen("HelloHelloHelloHelloHello"));  // write to the socket
    mJJP.connect((struct sockaddr*) &remaddr, sizeof(remaddr));

    //printf("Please enter the message: ");
    //memset(buffer,0, 256);
    //fgets(buffer,255,stdin);  // read message

    //n = mJJP.write("Hello",strlen("Hello"));  // write to the socket
    if (n < 0)
         error("ERROR writing to socket");

    memset(buffer,0,256);
    while ((n = mJJP.read(buffer,255)) == 0);  // read from the socket
    if (n < 0)
         error("ERROR reading from socket");
    printf("%s\n",buffer);  // print server's response

    //close(sockfd);  // taken care of by JJP destructor

    return 0;
}
