//
//  Sender.hpp
//  118_project_2
//
//  Created by Jaron Mink on 3/14/18.
//  Copyright © 2018 JaronMink. All rights reserved.
//

#ifndef Sender_hpp
#define Sender_hpp
#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <list>
#include <sstream>
#include <queue>
#include <time.h>
class Sender {
public:
    Sender();
    uint32_t get_avaliable_space();
    size_t send(char* packet, size_t packet_len, uint16_t seq_num, bool isResend);
    size_t send_update(char * packet, size_t packet_len); //just to send updates when rwnd is 0
    void resend_expired_packets();

    void set_sockfd(int sockfd) {mSockfd = sockfd;}
    void set_recipient(struct sockaddr *addr, socklen_t addrlen);

    void update_own_cwnd(size_t new_wnd) {cwnd = new_wnd;}
    void update_other_rwnd(size_t new_wnd) {rwnd = new_wnd;}
    void notify_ACK(uint16_t seq_num);
//private:
    // size_t send_packet(char* packet, size_t packet_len);
    size_t max_buf_size();
    class PacketObj {
    public:
        PacketObj(char* pack, size_t pack_len, uint16_t seq_num) {
            sequence_num = seq_num;
            packet = pack;
            packet_len = pack_len;
            isAcked = false;
            time(&sent_time);
        }

        time_t sent_time;
        uint16_t sequence_num;
        char* packet;
        size_t packet_len;
        bool isAcked;
    };

    bool packet_has_timed_out(PacketObj packet_obj) {
        time_t now = time(0);
        if(difftime(now, packet_obj.sent_time)*1000.0 > 500.0) {
            std::cout<< now << " " << packet_obj.sent_time  << " " <<packet_obj.sequence_num << " has timed ou t" << std::endl;
            return true;
        }
        return false;
    }

    std::list<PacketObj> packet_buffer;
    //size_t max_size;
    int mSockfd;
    const double timeout_ms = 500;

    size_t next_byte;
    //char m_buf[5120];
    //char* BUF; // ACK + min(rwnd, cwnd)
    //char* ACK; //last Acked byte
    //char* NEXT; //last sent byte
    size_t cwnd; //5120 bytes usually
    size_t rwnd; //0-5120 bytes
    struct sockaddr * m_servaddr;
    socklen_t m_servlen;
};

#endif /* Sender_hpp */
