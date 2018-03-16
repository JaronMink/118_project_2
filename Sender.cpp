//
//  Sender.cpp
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
#include "Sender.h"

//: - TODO add packet to queue if its not an ack, if an ack, don't add (ie add true for parameter)


/***
 Public
 ***/
Sender::Sender() {
    next_byte = 0;
    cwnd = 5120;
    rwnd = 5120;
    mSockfd = -1;
}

//set isACK for PacketObj that matches and move up window as much as we can
void Sender::notify_ACK(uint16_t seq_num) {
    std::cout << "In notify_ACK" << std::endl;
    for(std::list<PacketObj>::iterator it = packet_buffer.begin(); it != packet_buffer.end(); it++) {
      std::cout << it->sequence_num << std::endl;
        if(it->sequence_num == seq_num) {
            it->isAcked = true;
            break;
        }
    }
    std::cerr << "packets before:" << packet_buffer.size() << std::endl;
    //move window up
    bool hasMovedWindow = true;
    for(std::list<PacketObj>::iterator it = packet_buffer.begin(); hasMovedWindow;) {
        if(it->isAcked) {
            next_byte-=it->packet_len;
            it = packet_buffer.erase(it);
            std::cerr << "moved window once\n";
            hasMovedWindow = true;
        } else {
            hasMovedWindow = false;
        }
    }
    std::cerr << "packets after:" << packet_buffer.size() << std::endl << std::endl;
}

void Sender::set_recipient(struct sockaddr *addr, socklen_t addrlen){
  m_servaddr = addr;
  m_servlen = addrlen;
}

uint32_t Sender::get_avaliable_space(){
    if(max_buf_size() < next_byte) {
        std::cerr << "error, next_byte greater than max_buf_size\n";
        exit(1);
    }
    return ((uint32_t) max_buf_size() - (uint32_t) next_byte);
}

//go through all packets and resend expired ones
void Sender::resend_expired_packets() {
    for(std::list<PacketObj>::iterator it = packet_buffer.begin(); it != packet_buffer.end(); it++) {
        if((!it->isAcked) && packet_has_timed_out(*it)) {
            std::cout <<"resending!\n"<< std::endl;
            send(it->packet, it->packet_len, it->sequence_num, true);
            time_t now = time(0); //reset timer
            it->sent_time = now;
        }
    }
}

size_t Sender::send(char* packet, size_t packet_len, uint16_t seq_num, bool dontStore){
    if(((long)get_avaliable_space() - (long) packet_len) < 0) { //if we don't have enough space to hold packet, do nothing
        std::cerr << "Not enough space!"<< std::endl;
        return 0;
    }
    //write to socket
    //if(sendto(mSockfd, "hello", strlen("hello"), 0, m_servaddr, m_servlen) == -1)
      //  perror("error:");
    if (sendto(mSockfd, packet, packet_len, 0, m_servaddr, m_servlen) == -1)
      perror("sendto: ");
    std::cout << "Sending packet " << seq_num << " " << cwnd << std::endl;

    if(!dontStore) {
        //store in object
        PacketObj new_packet_object(packet, packet_len, seq_num);
        packet_buffer.push_back(new_packet_object);
        next_byte += packet_len;
    }

    return packet_len;
}

/***
 Sender Private
 ***/
size_t Sender::max_buf_size(){
    size_t minThreshold = 5120;
    if(cwnd < rwnd) {
        minThreshold = cwnd;
    } else {
        minThreshold = rwnd;
    }
    return minThreshold;
}
