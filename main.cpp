//
//  main.cpp
//  118_project_2
//
//  Created by Jaron Mink on 3/14/18.
//  Copyright © 2018 JaronMink. All rights reserved.
//

#include <iostream>
#include "JJP.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <netdb.h> 
#include "Sender.h"
#include "Packer.h"
using namespace std;
int main(int argc, const char * argv[]) {
    Packer myPacker;
    Sender mySender;
    char *packet1=NULL, *packet2 = NULL;
    myPacker.store("hello!", strlen("hello!"));
    myPacker.store("hello!hello!hello!hello!hello!hello!hello!hello!hello!hello!hello!hello!hello!", strlen("hello!hello!hello!hello!hello!hello!hello!hello!hello!hello!hello!hello!hello!"));

    size_t packetLen1 = myPacker.create_data_packet(&packet1, 25, 20);
    cout << "packetLen: " << packetLen1 << endl;
    if(packet1 == NULL)
        cout << "null\n";

    size_t packetLen2 = myPacker.create_data_packet(&packet2, 25, 21);
    cout << "packetLen: " << packetLen2 << endl;
    if(packet2 == NULL)
        cout << "null\n";

    //mySender
    int portno, n;
    struct sockaddr_in myaddr, remaddr;
    
    char buffer[256];
    
    char* server = "127.0.0.1";
    portno = 8003;
    //JJP mJJP(AF_INET, SOCK_STREAM, 0);
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);
    
    if(::bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
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
    mySender.set_sockfd(sockfd);
    mySender.set_recipient((struct sockaddr*) &remaddr, sizeof(remaddr));
    cout << "nextByte:"<< mySender.next_byte << endl;
    cout << "avaliable space:"<< mySender.get_avaliable_space() << endl;

    mySender.send(packet1, packetLen1, 20, false);
    cout << "nextByte:"<< mySender.next_byte << endl;
    cout << "avaliable space:"<< mySender.get_avaliable_space() << endl;
    mySender.send(packet2, packetLen2, 21, false);
    cout << "nextByte:"<< mySender.next_byte << endl;
    cout << "avaliable space:"<< mySender.get_avaliable_space() << endl;
    cout << "sleep" << endl;
    sleep(5);
    mySender.resend_expired_packets();
    cout << "nextByte:"<< mySender.next_byte << endl;
    cout << "avaliable space:"<< mySender.get_avaliable_space() << endl;

    mySender.notify_ACK(21);
    cout << "nextByte:"<< mySender.next_byte << endl;
    cout << "avaliable space:"<< mySender.get_avaliable_space() << endl;

    mySender.resend_expired_packets();
    mySender.notify_ACK(20);
    cout << "nextByte:"<< mySender.next_byte << endl;
    cout << "avaliable space:"<< mySender.get_avaliable_space() << endl;
    mySender.resend_expired_packets();

   /* if(sendto(sockfd, "hello", strlen("hello"), 0, (struct sockaddr*) &remaddr, sizeof(remaddr)) == -1)
        perror("error:");
    */
    // insert code here...
    std::cout << "Hefllo, World!\n";
    return 0;
}
