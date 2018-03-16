#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <sys/stat.h>    // structures for stat
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
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
    int portno;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    JJP mJJP(AF_INET, SOCK_DGRAM, 0);  // create socket
    memset((char *) &serv_addr, 0, sizeof(serv_addr));   // reset memory

    // fill in address info
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (mJJP.bind((struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    //mJJP.listen(5);  // 5 simultaneous connection at most


    int n;
    /*n = mJJP.write("HelloHelloHelloHelloHello",strlen("HelloHelloHelloHelloHello"));  // write to the socket
    mJJP.write("HelloHelloHelloHelloHello",strlen("HelloHelloHelloHelloHello"));  // write to the socket
    mJJP.write("HelloHelloHelloHelloHello",strlen("HelloHelloHelloHelloHello"));  // write to the socket
    mJJP.write("HelloHelloHelloHelloHello",strlen("HelloHelloHelloHelloHello"));  // write to the socket
    mJJP.write("HelloHelloHelloHelloHello",strlen("HelloHelloHelloHelloHello"));  // write to the socket


    int requestedFD = open("test.txt", O_RDONLY);
    if( requestedFD >= 0) {
        char* fileContent = NULL;
        int fileLen = -1;
        readFileContent(requestedFD, &fileContent, &fileLen);

        mJJP.write(fileContent, fileLen);
      }
      */

    //accept connections and process them
    if (mJJP.accept((struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0)
      error("ERROR on accept");

    //int n;
    char buffer[1024];

    memset(buffer, 0, 1024);  // reset memory

    //read client's message

    while(true) {
      while (mJJP.get_buf_size() == 0) usleep(10);

      while((n = mJJP.read(buffer, 1023)) == 0) ;
      if (n < 0) error("ERROR reading from socket");
      printf("Received Message:\n%s\n", buffer);

      int requestedFD = open(buffer, O_RDONLY);
      if( requestedFD >= 0) {
          char* fileContent = NULL;
          int fileLen = -1;
          readFileContent(requestedFD, &fileContent, &fileLen);

          mJJP.write(fileContent, fileLen);
        }
      break;
    }
    while(true);

    //reply to client
    if (n < 0) error("ERROR writing to socket");

    //close(sockfd); // taken care of by JJP destructor
    return 0;
}
