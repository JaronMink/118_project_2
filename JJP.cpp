//
//  JJP.cpp
//  118_project_2
//
//  Created by Jaron Mink on 3/14/18.
//  Copyright © 2018 JaronMink. All rights reserved.
//
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <list>
#include <thread>
#include "JJP.h"
JJP::JJP(int domain, int type, int protocol){
    mSockfd = ::socket(domain, type, protocol);
}

JJP::~JJP() {
    close(mSockfd);
}

int JJP::setsockopt(int level, int optname, const void *optval, socklen_t optlen){
    return ::setsockopt(mSockfd, level, optname, optval, optlen);
}

int JJP::bind(const struct sockaddr *addr, socklen_t addrlen){
    return ::bind(mSockfd, addr, addrlen);
}

int JJP::listen(int backlog){
    return ::listen(mSockfd, backlog);
}
void JJP::processing_thread(int newsockfd) {
    while (1) {
        int n;
        size_t bytesRead = 0;
        uint16_t ackNum, receiveWindow;
        char buffer[1024];
        
        memset(buffer, 0, 1024);  // reset memory
        
        //read client's message
        while((n = ::read(newsockfd, buffer, 1023)) == 0)
            bytesRead += n;
        if (bytesRead > 0) {
            int isACKorFIN = mReceiver.receive_packet(buffer, bytesRead, ackNum, receiveWindow);
            printf("Receiving packet %d\n", ackNum);
            
            if (isACKorFIN)
            {}//mSender.notify_ACK(
        }
        
        size_t available_space = mSender.get_avaliable_space();
        if (available_space > 0) {
            char* packet[1024];
            size_t sequence_num = 0;
            size_t packet_len = mPacker.create_data_packet(packet, 1024, sequence_num);
            mSender.send(*packet, packet_len, sequence_num);
        }
    }
}

int JJP::accept(struct sockaddr *addr, socklen_t * addrlen){
    int newsockfd = ::accept(mSockfd, addr, addrlen);
    
    std::thread process(&JJP::processing_thread, this, newsockfd);
    // don't block, we must return the sockfd
    
    return newsockfd;
}

int JJP::connect(const struct sockaddr *addr, socklen_t addrlen){
    int retVal = ::connect(mSockfd, addr, addrlen);
    ///
    //todo create new thread that constantly reads and writes from socket and does stuff
    ///
    return retVal;
}

ssize_t JJP::write(const void *buf, size_t nbytes) {
    return mPacker.store((char*)buf, nbytes);
}

ssize_t JJP::read(void *buf, size_t nbytes) {
    return mReceiver.read(buf, nbytes);
}
