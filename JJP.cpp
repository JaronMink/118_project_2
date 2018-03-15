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
    mSender.set_sockfd(mSockfd);
    mReceiver.set_sockfd(mSockfd);
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
/**
 thread psuedo code:
 read_from_UDP_and_translate_to_packet
 process_packet
 if_ack
 notify_sender
 
 get_avaliable_space
 create_packet_with_len
 send_packet
 **/

//read a single packet
//put packet in buffer
//process and extract flags
//ifack_
//notify sender
//ifFIN
//prepare to disconnect

//get_avaliable_space
//create_packet_with_len
//send_packet
void JJP::processing_thread2() {
    bool receivedFIN = false;
    uint16_t sequence_number = 0;
    char *rcvd_packet, *sending_packet, *update_packet;
    uint16_t ackNum, receiveWindow;
    int updateTimer = 10; //every 10 loops when rwnd is 0 send update
    while(!receivedFIN) {
        
        size_t rcvd_len = read_single_packet(&rcvd_packet);
        int isACKorFIN = mReceiver.receive_packet(rcvd_packet, rcvd_len, ackNum, receiveWindow);
        if (isACKorFIN)
        {}//mSender.notify_ACK(
            //todo,
                //ACK packet received
                //if FIN disconnect and begin ending
                //if ACK notify sender
                //update rwnd
        
        uint32_t avaliable_space = mSender.get_avaliable_space();
        size_t sending_packet_len = mPacker.create_data_packet(&sending_packet, avaliable_space, sequence_number);
        mSender.send(sending_packet, avaliable_space, sequence_number, false);
        mSender.resend_expired_packets();
        
        //periodically update if receiver is 0
        if(receiveWindow == 0) {
            if(updateTimer != 0) {
                updateTimer--;
            }
            else {
                updateTimer=10;
                sequence_number++;
                size_t update_size = mPacker.create_update(&update_packet, sequence_number);
                mSender.send(update_packet, update_size, sequence_number, false);
            }
        }
        
    }
}

void JJP::processing_thread() {
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
        printf("%d\n", mSender.send(*packet, packet_len, sequence_num, false));
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
  }
}

size_t JJP::read_single_packet(char** packet) {
    char header[12];
    struct sockaddr_in* client_addr;
    socklen_t clilen = sizeof(client_addr);
    
    size_t totalBytesRead = ::recvfrom(mSockfd, header, 12, 0, (struct sockaddr*) client_addr, &clilen);
    while(totalBytesRead != 12) {
        totalBytesRead += ::recvfrom(mSockfd, header+totalBytesRead, 12-totalBytesRead, 0, (struct sockaddr*) client_addr, &clilen);
    }
    uint32_t packet_len = 0;
    memmove((char*) &packet_len, header, 4);
    std::cerr << "received packet len:" << packet_len << std::endl;
    char* received_packet = (char *) malloc(sizeof(char) * packet_len);
    //move header into packet
    memmove(received_packet, header, 12);
    
    size_t data_len = packet_len - 12;
    
    totalBytesRead = ::recvfrom(mSockfd, packet+12, data_len, 0, (struct sockaddr*) client_addr, &clilen);
    while(totalBytesRead < data_len) {
        totalBytesRead += ::recvfrom(mSockfd, packet+12+totalBytesRead, data_len-totalBytesRead, 0, (struct sockaddr*) client_addr, &clilen);
    }
    
    *packet = received_packet;
    return packet_len;
}

int JJP::accept(struct sockaddr *addr, socklen_t addrlen){
  mSender.set_recipient(addr,sizeof(addr));

  std::thread process(&JJP::processing_thread, this);
  process.detach();
  // don't block, we must return the sockfd

  return 0;
}

int JJP::connect(struct sockaddr *addr, socklen_t addrlen){
  mSender.set_recipient(addr,addrlen);

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
