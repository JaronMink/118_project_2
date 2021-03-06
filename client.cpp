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
#include <mutex>
#include <thread>
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
    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port filename\n", argv[0]);
       exit(0);
    }

    char* server = argv[1];
    portno = atoi(argv[2]);
    char* filename = argv[3];

    JJP mJJP(AF_INET, SOCK_DGRAM, 0);

    memset((char *)&myaddr, 0, sizeof(myaddr));
  	myaddr.sin_family = AF_INET;
  	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  	myaddr.sin_port = htons(0);

  	if (mJJP.bind((struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
  		perror("bind failed");
  		return 0;
  	}

  	memset((char *) &remaddr, 0, sizeof(remaddr));
  	remaddr.sin_family = AF_INET;
  	remaddr.sin_port = htons(portno);
  	if (inet_aton(server, &remaddr.sin_addr)==0) {
  		fprintf(stderr, "inet_aton() failed\n");
  		exit(1);
  	}

    mJJP.write(filename,strlen(filename));

    mJJP.connect((struct sockaddr*) &remaddr, sizeof(remaddr));

    //printf("Please enter the message: ");
    //memset(buffer,0, 256);
    //fgets(buffer,255,stdin);  // read message

    int file_fd = open("received.data",O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (file_fd < 0) {
      perror("fopen");
    }
    while(!mJJP.isDisconnected()) {
      while (!mJJP.isDisconnected() && mJJP.get_buf_size() == 0) usleep(10);

      while(!mJJP.isDisconnected() && ((n = mJJP.read(buffer, 1023)) == 0)) ;
      if (mJJP.isDisconnected())
        break;
      if (n < 0) error("ERROR reading from socket");
      //printf("Received Message:\n%s\n", buffer);
      write(file_fd, buffer, n);
    }

    mJJP.close();  // taken care of by JJP destructor

    return 0;
}
