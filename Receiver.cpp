//
//  Receiver.cpp
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
#include "Receiver.h"



Receiver::Receiver() {
    expected_packet_num = 0;
    used_space = 0;
    mSockfd = -1;
}

int read_packet();


int Receiver::receive_packet(char* packet, size_t packet_len, uint16_t& sequence_number, uint16_t &acknowledgement_num, uint16_t &receiver_window, bool &isACK, bool &isFIN, bool &isSYN) {
    //if data, send ACK (telegraph to JJP that we received data, ie return true)
    //if ACK, notify sender that packet has been successfully acked
    //if data
    //put into temporary buffer (update avaliable space)
    uint32_t packet_length;
    //bool isACK, isFIN, isSYN;
    int16_t flags = 0;

    char* header = packet;

    memmove((char*)&packet_length, header, 4);
    memmove((char*)&sequence_number, header+4, 2);
    memmove((char*)&acknowledgement_num, header+6, 2);
    memmove((char*)&receiver_window, header+8, 2);
    memmove((char*)&flags, header+10, 2);

    isACK = ((flags & (0x1<<15)) != 0);
    isFIN = ((flags & (0x1<<14)) != 0);
    isSYN = ((flags & (0x1<<13)) != 0);

    //int ret_flag = 0;

    //if (isACK || isFIN)
     //   ret_flag = 1;

    packetPair pPair(sequence_number, packet, packet_len);

    if (get_avaliable_space() > 0)
    {
        storage_queue.push(pPair);
        used_space += packet_len;
    }

    while (!storage_queue.empty() && storage_queue.top().seq_num == expected_packet_num) {
        packetPair currPair = storage_queue.top();
        storage_queue.pop();
        used_space -= currPair.packet_len;
        bufSS.write(currPair.packet, currPair.packet_len);
        expected_packet_num = (expected_packet_num + currPair.packet_len) % 30720;
    }

    return 0; //return -1 before if anything goes wrong
}

size_t Receiver::read(void *buf, size_t nbytes){
    bufSS.read((char*)buf, nbytes);
    return bufSS.gcount();
}

size_t Receiver::get_avaliable_space(){
    return 5120 - used_space;
}
