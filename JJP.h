//
//  JJP.hpp
//  118_project_2
//
//  Created by Jaron Mink on 3/14/18.
//  Copyright © 2018 JaronMink. All rights reserved.
//

#ifndef JJP_hpp
#define JJP_hpp
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <list>
#include <sstream>
#include <queue>
#include <time.h>
#include <mutex>
#include "Packer.h"
#include "Sender.h"
#include "Receiver.h"

class JJP {
public:
    JJP(int domain, int type, int protocol); //like socket defintion
    ~JJP();
    int setsockopt(int level, int optname, const void *optval, socklen_t optlen);
    int bind(const struct sockaddr *addr, socklen_t addrlen);
    int listen(int backlog);
    int accept(struct sockaddr *addr, socklen_t addrlen);
    int connect(struct sockaddr *addr, socklen_t addrlen);
    int get_buf_size();

    ssize_t write(const void *buf, size_t nbytes);
    ssize_t read(void *buf, size_t nbytes);

private:
    void processing_thread(bool isClient);
    void SYN_server();
    void SYN_client();
    void FIN_server(int receievedSequenceNumber);
    void FIN_client();


    size_t read_single_packet(char** packet);
    Packer mPacker;
    Sender mSender;
    Receiver mReceiver;
    uint16_t other_receive_window;
    uint16_t sequence_number;
    int mSockfd;
    struct sockaddr_in client_addr;
    socklen_t clilen;
    std::mutex* buf_mutex;
};
/**
 Max packet size = 1024 bytes
 window size = 5120 bytes
 max sequence num = 30720
 retransmission time = 500ms

 HEADER SPECS -96 bits total (12 bytes)

 DataLen(32bits),
 seqNum(16bits), ackNum(16bits)
 rcwn(16bits),ACK(1bit),fin(1bit),Syn(1bit),unused(13bits)
 **/




#endif /* JJP_hpp */
