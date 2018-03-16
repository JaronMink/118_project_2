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
#include <mutex>
#include <fcntl.h>
#include "JJP.h"
std::mutex buf_mutex, buf_mutex2;


JJP::JJP(int domain, int type, int protocol){
    mSockfd = ::socket(domain, type, protocol);
    int flags;
    if (-1 == (flags = fcntl(mSockfd, F_GETFL, 0)))
      flags = 0;
    fcntl(mSockfd, F_SETFL, flags | O_NONBLOCK);
    if (mSockfd < 0)
      perror("ERROR opening socket");
    mSender.set_sockfd(mSockfd);
    mReceiver.set_sockfd(mSockfd);
    clilen = sizeof(client_addr);
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

void JJP::SYN_client() {
    bool receivedFIN = false, receivedACK = false, receivedSYN = false;
    char *rcvd_packet, *syn_packet;
    uint16_t ackNum, receivedSequenceNumber = 0;
    
    //send SYN
    sequence_number = rand() % 30720;
    size_t syn_packet_len = mPacker.create_SYN(&syn_packet, sequence_number);
    mSender.send(syn_packet, syn_packet_len, sequence_number, false);
    std::cout << "sending SYN with sequence number " << sequence_number << std::endl;
    sequence_number = (sequence_number + syn_packet_len) % 30720;
    
    //wait for SYNACK and send ack once received

    while(true) {
        receivedACK = receivedFIN = receivedSYN = false;
        
        long rcvd_len = read_single_packet(&rcvd_packet);
        if (rcvd_len > 0) {
            if((mReceiver.receive_packet(rcvd_packet, rcvd_len, receivedSequenceNumber, ackNum, other_receive_window, receivedACK, receivedFIN, receivedSYN)) < 0)
                perror("error receiving packet");
        }
        if(!(receivedSYN && receivedACK)) //wait for SYNACK!!!
            continue;
        
        //ok we got syn ack, lets ack and begin processing
        std::cout << "received SYNack with sequence number " << receivedSequenceNumber << std::endl;
        mReceiver.set_seq_num(receivedSequenceNumber + rcvd_len);
        
        mSender.update_other_rwnd(other_receive_window); //update sender with rwnd
        mPacker.update_own_rwnd(mReceiver.get_avaliable_space());
        
        char* ACKPacket;
        size_t ACKPacket_len = mPacker.create_ACK(&ACKPacket, sequence_number, receivedSequenceNumber);
        sequence_number = (sequence_number + ACKPacket_len) % 30720;
        mSender.send(ACKPacket, ACKPacket_len, sequence_number, true);
        std::cout << "Sending ACK " <<  receivedSequenceNumber << " len" << ACKPacket_len << std::endl;
    }
}

void JJP::SYN_server() {
    bool receivedFIN = false, receivedACK = false, receivedSYN = false;
    char *rcvd_packet;
    uint16_t ackNum, receivedSequenceNumber = 0;
     
    while(true) {
        receivedACK = receivedFIN = receivedSYN = false;
        
        long rcvd_len = read_single_packet(&rcvd_packet);
        if (rcvd_len > 0) {
            if((mReceiver.receive_packet(rcvd_packet, rcvd_len, receivedSequenceNumber, ackNum, other_receive_window, receivedACK, receivedFIN, receivedSYN)) < 0)
                perror("error receiving packet");
        }
        if(!receivedSYN) //wait for SYN!!!
            continue;
        //ok we got syn, now lets send a SYNACK
        std::cout << "received SYN with sequence number " << receivedSequenceNumber << std::endl;
        mReceiver.set_seq_num(receivedSequenceNumber + rcvd_len);
        
        mSender.update_other_rwnd(other_receive_window); //update sender with rwnd
        mPacker.update_own_rwnd(mReceiver.get_avaliable_space());
        
        char* syn_ack_packet = NULL;
        sequence_number = rand() % 30720;
        size_t syn_ack_packet_len = mPacker.create_SYNACK(&syn_ack_packet, sequence_number, receivedSequenceNumber);
        sequence_number = (sequence_number + syn_ack_packet_len) % 30720;
        mSender.send(syn_ack_packet, syn_ack_packet_len, sequence_number, false);
        std::cout << "sending SYNACK with sequence number " << sequence_number << std::endl;
    }
}




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
    bool receivedFIN = false, receivedACK = false, receivedSYN = false;
    char *rcvd_packet, *sending_packet, *update_packet;
    uint16_t ackNum, receivedSequenceNumber;
    //int updateTimer = 10; //every 10 loops when rwnd is 0 send update
    while(true) {
        receivedACK = receivedFIN = receivedSYN = false;

        std::lock(buf_mutex, buf_mutex2);
        long rcvd_len = read_single_packet(&rcvd_packet);
        if (rcvd_len > 0) {
          //printf("Receiving packet of byte length %d\n", rcvd_len);
          if((mReceiver.receive_packet(rcvd_packet, rcvd_len, receivedSequenceNumber, ackNum, other_receive_window, receivedACK, receivedFIN, receivedSYN)) < 0)
              perror("error receiving packet");
        }

            //todo,
                //if Data, send ack with dontStore as true -done
                //if FIN disconnect and begin ending
                //if ACK notify sender -done
                //update rwnd -done

        mSender.update_other_rwnd(other_receive_window); //update sender with rwnd
        mPacker.update_own_rwnd(mReceiver.get_avaliable_space());

        if ((rcvd_len > 12) || receivedFIN || receivedSYN) {
          char* ACKPacket;
          size_t ACKPacket_len = mPacker.create_ACK(&ACKPacket, sequence_number, receivedSequenceNumber);
          sequence_number = (sequence_number + ACKPacket_len) % 30720;
          mSender.send(ACKPacket, ACKPacket_len, sequence_number, true);
          std::cout << "Sending ACK " <<  receivedSequenceNumber << " len" << ACKPacket_len << std::endl;
        }

        if (receivedACK) {
          std::cout << "Received ACK " << ackNum << std::endl;
          mSender.notify_ACK(ackNum);
       }

        uint32_t available_space = mSender.get_avaliable_space();
        mSender.resend_expired_packets();

        if (available_space > 1024)
          available_space = 1024;
        size_t sending_packet_len = mPacker.create_data_packet(&sending_packet, available_space, sequence_number);
        if (sending_packet_len > 0) {
          //printf("Packet length: %d\n", sending_packet_len);
          mSender.send(sending_packet, available_space, sequence_number, false);
          sequence_number = (sequence_number + sending_packet_len) % 30720;
        }


        /*

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
        */
        buf_mutex.unlock();
        buf_mutex2.unlock();


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
      std::lock(buf_mutex, buf_mutex2);
      size_t packet_len = mPacker.create_data_packet(packet, 1024, sequence_num);
      buf_mutex.unlock();
      buf_mutex2.unlock();
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
      std::lock(buf_mutex, buf_mutex2);
      //int isACKorFIN = mReceiver.receive_packet(buffer, bytesRead, ackNum, receiveWindow);
      buf_mutex.unlock();
      buf_mutex2.unlock();

      printf("Receiving packet of byte length %d\n", bytesRead);

      //if (isACKorFIN)
        //{}//mSender.notify_ACK(
    }
  }
}

size_t JJP::read_single_packet(char** packet) {
    char header[12];


    long totalBytesRead = ::recvfrom(mSockfd, header, 12, MSG_PEEK, (struct sockaddr*) &client_addr, &clilen);

    //if we didn't read anything, assume we don't have any data to read.
    if (totalBytesRead <= 0) {
        *packet = NULL;
        return 0;
    }
    //std::cout << totalBytesRead << std::endl;

    while(totalBytesRead != 12) {
        int attemptedBytesRead = ::recvfrom(mSockfd, header+totalBytesRead, 12-totalBytesRead, MSG_PEEK, (struct sockaddr*) &client_addr, &clilen);
        if (attemptedBytesRead == -1)
          continue;
        totalBytesRead += attemptedBytesRead;
        //std::cout << totalBytesRead << std::endl;
    }
    uint32_t packet_len = 0;
    memmove((char*) &packet_len, header, 4);
    //std::cerr << "received packet len:" << packet_len << std::endl;
    char* received_packet = (char *) malloc(sizeof(char) * packet_len);
    //move header into packet
    //memmove(received_packet, header, 12);

    totalBytesRead = 0;//::recvfrom(mSockfd, received_packet+12, data_len, 0, (struct sockaddr*) &client_addr, &clilen);
    while(totalBytesRead < packet_len) {
        int attemptedBytesRead = ::recvfrom(mSockfd, received_packet + totalBytesRead, packet_len-totalBytesRead, 0, (struct sockaddr*) &client_addr, &clilen);
        if (attemptedBytesRead == -1)
          continue;
        totalBytesRead += attemptedBytesRead;
        //std::cout << totalBytesRead << std::endl;
    }

    //memmove(received_packet + 12, body, data_len);

    *packet = received_packet;

    mSender.set_recipient((struct sockaddr*) &client_addr, (socklen_t) clilen);

    return packet_len;
}

int JJP::accept(struct sockaddr *addr, socklen_t addrlen){
  mSender.set_recipient(addr,addrlen);

  std::thread process(&JJP::processing_thread2, this);
  process.detach();
  // don't block, we must return the sockfd

  return 0;
}

int JJP::connect(struct sockaddr *addr, socklen_t addrlen){
  mSender.set_recipient(addr,addrlen);

  std::thread process(&JJP::processing_thread2, this);
  process.detach();

  return 0;
}

ssize_t JJP::write(const void *buf, size_t nbytes) {
    //std::lock(buf_mutex, buf_mutex2);
    int i = mPacker.store((char*)buf, nbytes);
    //buf_mutex.unlock();
    //buf_mutex2.unlock();
    return i;
}

ssize_t JJP::read(void *buf, size_t nbytes) {
    //std::lock(buf_mutex, buf_mutex2);
    int i = mReceiver.read(buf, nbytes);
    //buf_mutex.unlock();
    //buf_mutex2.unlock();
    return i;
}
