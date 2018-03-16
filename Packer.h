//
//  Packer.hpp
//  118_project_2
//
//  Created by Jaron Mink on 3/14/18.
//  Copyright © 2018 JaronMink. All rights reserved.
//

#ifndef Packer_hpp
#define Packer_hpp
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <list>
#include <sstream>
#include <queue>
#include <time.h>

class Packer {
public:
    Packer();
    //store data to be send
    size_t store(const char* str, size_t len);
    //create a packet of total size len, sets a pointer buf to pa cket and returns size of it
    size_t create_data_packet(char** buf, uint32_t len, uint16_t sequence_number);
    size_t create_FIN(char** packet, uint16_t seq_num);
    size_t create_ACK(char** packet, uint16_t seq_num, uint16_t ack_num);
    size_t create_SYN(char** packet, uint16_t seq_num);
    size_t create_FINACK(char** packet, uint16_t seq_num, uint16_t ack_num);
    size_t create_SYNACK(char** packet, uint16_t seq_num, uint16_t ack_num);

    
    size_t create_update(char** packet, uint16_t seq_num);
    void update_own_rwnd(size_t new_rwnd) {rwnd=new_rwnd;}
private:
    //given x bytes of data, add a header to it
    char* create_header(uint32_t packet_length, uint16_t sequence_number, uint16_t acknowledgement_num, uint16_t receiver_window, bool isACK, bool isFIN, bool isSYN);
    size_t size();
    size_t rwnd;
    std::stringstream bufSS;
    size_t bufLen;
    static const size_t headerLen = sizeof(char)*12; //96 bit header
};

#endif /* Packer_hpp */
