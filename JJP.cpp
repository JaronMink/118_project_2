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
    if (mSockfd < 0)
      perror("ERROR opening socket");
    //mSender.set_sockfd(mSockfd);
    //mReceiver.set_sockfd(mSockfd);
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
void JJP::processing_thread() {
  /*
  while (1) {
    int n;
    size_t bytesRead = 0;
    uint16_t ackNum, receiveWindow;
    char buffer[1024];

    memset(buffer, 0, 1024);  // reset memory

    //size_t available_space = mSender.get_avaliable_space();
    //if (available_space > 0) {
      char* packet[1024];
      size_t sequence_num = 0;
      size_t packet_len = mPacker.create_data_packet(packet, 1024, sequence_num);
      if (packet_len > 0) {
        printf("Packet length: %d\n", packet_len);
        printf("%d\n", mSender.send(*packet, packet_len, sequence_num));
      }
    //}

    //printf("About to receive message in thread.\n");
    //read client's message
    struct sockaddr_in* client_addr;
    socklen_t clilen = sizeof(client_addr);
    if ((n = ::recvfrom(mSockfd, buffer, 1023, 0, (struct sockaddr*) client_addr, &clilen)) > 0)
      bytesRead += n;
    //printf("Received message in thread.\n");
    if (bytesRead > 0) {
      int isACKorFIN = mReceiver.receive_packet(buffer, bytesRead, ackNum, receiveWindow);
      printf("Receiving packet of byte length %d\n", bytesRead);

      if (isACKorFIN)
        {}//mSender.notify_ACK(
    }
  }*/
}

int JJP::accept(struct sockaddr *addr, socklen_t addrlen){
  //mSender.set_recipient(addr,sizeof(addr));

  std::thread process(&JJP::processing_thread, this);
  process.detach();
  // don't block, we must return the sockfd

  return 0;
}

int JJP::connect(struct sockaddr *addr, socklen_t addrlen){
  //mSender.set_recipient(addr,addrlen);

  std::thread process(&JJP::processing_thread, this);
  process.detach();

  return 0;
}

ssize_t JJP::write(const void *buf, size_t nbytes) {
    return mPacker.store((char*)buf, nbytes);
}

ssize_t JJP::read(void *buf, size_t nbytes) {
    return mReceiver.read(buf, nbytes);
}
