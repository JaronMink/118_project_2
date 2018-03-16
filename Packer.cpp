//
//  Packer.cpp
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
#include "Packer.h"

/***
 Public
 ***/

Packer::Packer() {
    bufLen = 0;
}
size_t Packer::store(const char* str, size_t len) {
    bufLen+=len;
    bufSS.write(str, len);
    return len; //assume we wrote the whole thing
}

//read into buf and return size of packet
size_t Packer::create_data_packet(char** buf, uint32_t len, uint16_t sequence_number){
    long dataLen = len - headerLen;
    if(dataLen <= 0 || dataLen > (1024 - headerLen)) { //if we don't have enough space to put any data, or packet is too big return a nullptr
        return 0;
    }

    char* body = (char*)malloc(sizeof(char)*dataLen);
    bufSS.read(body, dataLen);
    size_t bytesRead = bufSS.gcount();
    bufLen -= bytesRead;
    uint32_t totalPacketSize = bytesRead + headerLen;
    if(bytesRead <= 0) {
        //std::cout << "Packer: No bytes to read from packer\n";
        return 0;
    }

    char* header = create_header(totalPacketSize, sequence_number, (uint16_t) 0, rwnd, false, false, false);

    char* packet = (char*) malloc(sizeof(char)*totalPacketSize);
    memmove(packet, header, sizeof(char)*12);
    memmove(packet + 12, body, sizeof(char)*bytesRead);
    *buf = packet;
    free(body);
    free(header);
    return totalPacketSize;
}

/***
 Private
 ***/
char* Packer::create_header(uint32_t packet_length, uint16_t sequence_number, uint16_t acknowledgement_num, uint16_t receiver_window, bool isACK, bool isFIN, bool isSYN){ //size_t sequenceNum, bool isACK, size_t ackNum, bool isFIN, size_t rcwn, size_t receiverWindow, size_t totalSize) {
    char* header = (char *) malloc(sizeof(char)*12); //96 bit header

    int16_t flags = 0;
    if(isACK){
        flags = flags | (0x1<<15); //flag is 15th bit
    }
    if(isFIN) {
        flags = flags | (0x1<<14); //flag is 14th bit
    }
    if (isSYN) {
        flags = flags | (0x1<<13); //flag is 13th bit
    }
    //add contents to packet

    memmove(header, (char*)&packet_length, 4);
    memmove(header+4,(char*)&sequence_number, 2);
    memmove(header+6,(char*)&acknowledgement_num, 2);
    memmove(header+8,(char*)&receiver_window, 2);
    memmove(header+10,(char*)&flags, 2);

    return header;
}

size_t Packer::size() {
    return bufLen;
}

size_t Packer::create_FIN(char** packet, uint16_t seq_num) {
    *packet = create_header(sizeof(char)*12, seq_num, (uint16_t) 0, rwnd, false, true, false);
    return (sizeof(char)*12);
}
size_t Packer::create_ACK(char** packet, uint16_t seq_num, uint16_t ack_num) {
    *packet = create_header(sizeof(char)*12, seq_num, ack_num, rwnd, true, false, false);
    return (sizeof(char)*12);
}
size_t Packer::create_SYN(char** packet, uint16_t seq_num) {
    *packet = create_header(sizeof(char)*12, seq_num, (uint16_t) 0, rwnd, false, false, true);
    return (sizeof(char)*12);
}

size_t Packer::create_update(char** packet, uint16_t seq_num) {
    *packet = create_header(sizeof(char)*12, seq_num, (uint16_t) 0, rwnd, false, false, false);
    return (sizeof(char)*12);
}
