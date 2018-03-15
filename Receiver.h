//
//  Receiver.hpp
//  118_project_2
//
//  Created by Jaron Mink on 3/14/18.
//  Copyright © 2018 JaronMink. All rights reserved.
//

#ifndef Receiver_hpp
#define Receiver_hpp

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <list>
#include <sstream>
#include <queue>
#include <time.h>

class Receiver {
public:
    Receiver(); //init packet num
    
    int read_packet();
    // returns -1 if invalid data, 0 if valid data, 1 if ACK
    int receive_packet(char* packet, size_t packet_len, uint16_t &acknowledgement_num, uint16_t &receiver_window);
    //if data, send ACK (telegraph to JJP that we received data, ie return true)
    //if ACK, notify sender that packet has been successfully acked
    //if data
    //put into temporary buffer (update avaliable space)
    
    size_t read(void *buf, size_t nbytes); //read from stored data
    //read from sstring, either up to nbytes or x bytes. return x bytes.
    size_t get_avaliable_space();
    //(5120) total space in bufer - sum of packet size in temp storage
    void set_sockfd(int sockfd) {mSockfd = sockfd;}
    
private:
    //update_temporary_storage //transfer valid data from temp storage to sstream
    struct packetPair
    {
        uint16_t seq_num;
        char* packet;
        size_t packet_len;
        
        packetPair(uint16_t givenSeqNum, char* givenPacket, size_t givenPacketLen)  {
            seq_num = givenSeqNum;
            packet = givenPacket;
            packet_len = givenPacketLen;
        }
        
        bool operator<(const struct packetPair& other) const
        {  return seq_num < other.seq_num; }
    };
    
    //next expected packet, init to 0
    int mSockfd;
    uint16_t expected_packet_num;
    std::stringstream bufSS;
    std::priority_queue<packetPair> storage_queue;
    size_t used_space;
};

#endif /* Receiver_hpp */
