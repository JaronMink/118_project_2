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
    for(std::list<PacketObj>::iterator it = packet_buffer.begin(); it != packet_buffer.end(); it++) {
        if(it->sequence_num == seq_num) {
            it->isAcked = true;
            return;
        }
    }

}

void Sender::set_recipient(struct sockaddr *addr, socklen_t addrlen){
  m_servaddr = addr;
  m_servlen = addrlen;
}

size_t Sender::get_avaliable_space(){
    if(max_buf_size() < next_byte) {
        std::cerr << "error, next_byte greater than max_buf_size\n";
        exit(1);
    }
    return (max_buf_size() - next_byte);
}

//go through all packets and resend expired ones
void Sender::resend_expired_packets() {
    for(std::list<PacketObj>::iterator it = packet_buffer.begin(); it != packet_buffer.end(); it++) {
        if(packet_has_timed_out(*it)) {
            send(it->packet, it->packet_len, it->sequence_num );
        }
    }
}

size_t Sender::send(char* packet, size_t packet_len, uint16_t seq_num){
    if(((long)get_avaliable_space() - (long) packet_len) < 0) { //if we don't have enough space to hold packet, do nothing
        return 0;
    }
    //write to socket
    ::write(mSockfd, packet, packet_len);
    //store in object
    PacketObj new_packet_object(packet, packet_len, seq_num);
    packet_buffer.push_back(new_packet_object);
    next_byte += packet_len;


    //set up alarm for res
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
