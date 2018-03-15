/*
 A simple client in the internet domain using TCP
 Usage: ./client hostname port filename (./client 192.168.0.151 10000 test.txt)
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>      // define structures like hostent
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "JJP.h"

void error(char *msg)
{
    perror(msg);
    exit(0);
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

    //n = mJJP.write("Hello",strlen("Hello"));  // write to the socket
    mJJP.connect((struct sockaddr*) &remaddr, sizeof(remaddr));

    //printf("Please enter the message: ");
    //memset(buffer,0, 256);
    //fgets(buffer,255,stdin);  // read message

    n = mJJP.write("Hello",strlen("Hello"));  // write to the socket
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
