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
    void processing_thread(int newsockfd);
    int accept(struct sockaddr *addr, socklen_t * addrlen);
    int connect(const struct sockaddr *addr, socklen_t addrlen);
    
    ssize_t write(const void *buf, size_t nbytes);
    ssize_t read(void *buf, size_t nbytes);
    Packer mPacker;
    Sender mSender;
    Receiver mReceiver;
    int mSockfd;
};


#endif /* JJP_hpp */
