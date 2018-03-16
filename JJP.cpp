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
#include <time.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <list>
#include <thread>
#include <mutex>
#include <fcntl.h>
#include "JJP.h"


//std::mutex test_mutex;


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
    buf_mutex = new std::mutex();
}

JJP::~JJP() {
    delete(buf_mutex);
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

void JJP::FIN_client() {

    bool receivedFIN = false, receivedACK = false, receivedSYN = false;
    char *rcvd_packet, *fin_packet;
    uint16_t ackNum, receivedSequenceNumber = 0;

    //send FIN
    size_t fin_packet_len = mPacker.create_FIN(&fin_packet, sequence_number);
    mSender.send(fin_packet, fin_packet_len, sequence_number, false);
    std::cout << "sending FIN with sequence number " << sequence_number << std::endl;
    sequence_number = (sequence_number + fin_packet_len) % 30720;

    //wait for FINACK and send ack once received

    while(true) {
        mSender.resend_expired_packets();
        receivedACK = receivedFIN = receivedSYN = false;

        long rcvd_len = read_single_packet(&rcvd_packet);
        if (rcvd_len > 0) {
            if((mReceiver.receive_packet(rcvd_packet, rcvd_len, receivedSequenceNumber, ackNum, other_receive_window, receivedACK, receivedFIN, receivedSYN)) < 0)
                perror("error receiving packet");
        }
        if(!(receivedFIN && receivedACK)) //wait for FINACK!!!
            continue;

        //ok we got fin ack, lets ack and wait for 2 sec
        mSender.notify_ACK(ackNum);
        std::cout << "received FINack with sequence number " << receivedSequenceNumber << std::endl;
        mReceiver.set_seq_num(receivedSequenceNumber + rcvd_len);

        mSender.update_other_rwnd(other_receive_window); //update sender with rwnd
        mPacker.update_own_rwnd(mReceiver.get_avaliable_space());

        char* ACKPacket;
        size_t ACKPacket_len = mPacker.create_ACK(&ACKPacket, sequence_number, receivedSequenceNumber);
        mSender.send(ACKPacket, ACKPacket_len, sequence_number, true);
        std::cout << "Sending ACK " <<  receivedSequenceNumber << " len" << ACKPacket_len << std::endl;
        sequence_number = (sequence_number + ACKPacket_len) % 30720;

        time_t begin = time(NULL);
        time_t now = time(NULL);
        //wait and ack anything that comes in
        while(difftime(now, begin) < 2) {
            now = time(NULL);

            mSender.resend_expired_packets();
            long rcvd_len = read_single_packet(&rcvd_packet);
            if (rcvd_len > 0) {
                if((mReceiver.receive_packet(rcvd_packet, rcvd_len, receivedSequenceNumber, ackNum, other_receive_window, receivedACK, receivedFIN, receivedSYN)) < 0)
                    perror("error receiving packet");
            }
            if(!(receivedFIN && receivedACK)) //wait for FINACK!!!
                continue;

            //ok we got fin ack, lets ack
            mSender.notify_ACK(ackNum);
            std::cout << "received FINack with sequence number " << receivedSequenceNumber << std::endl;
            mReceiver.set_seq_num(receivedSequenceNumber + rcvd_len);

            mSender.update_other_rwnd(other_receive_window); //update sender with rwnd
            mPacker.update_own_rwnd(mReceiver.get_avaliable_space());

            char* ACKPacket;
            size_t ACKPacket_len = mPacker.create_ACK(&ACKPacket, sequence_number, receivedSequenceNumber);
            mSender.send(ACKPacket, ACKPacket_len, sequence_number, true);
            std::cout << "Sending ACK " <<  receivedSequenceNumber << " len" << ACKPacket_len << std::endl;
            sequence_number = (sequence_number + ACKPacket_len) % 30720;
        }
        std::cerr << "two second have passed, lets now return" << std::endl;
        return;
    }
}

//we received a fin already, lets just fin ack, wait for ack then leave
void JJP::FIN_server(int receievedSequenceNumber) {
    bool receivedFIN = false, receivedACK = false, receivedSYN = false;
    char *rcvd_packet;
    uint16_t ackNum, receivedSequenceNumber;
    //ok we got fin, now lets send a FINACK
    std::cout << "received FIN with sequence number " << receivedSequenceNumber << std::endl;

    uint16_t fin_ack_seq_num = sequence_number;
    char* fin_ack_packet = NULL;
    size_t fin_ack_packet_len = mPacker.create_FINACK(&fin_ack_packet, sequence_number, receivedSequenceNumber);
    mSender.send(fin_ack_packet, fin_ack_packet_len, sequence_number, false);
    std::cout << "sending SYNACK with sequence number " << sequence_number << std::endl;
    sequence_number = (sequence_number + fin_ack_packet_len) % 30720;

    //wait for ack, then leave
    while(true){
        mSender.resend_expired_packets();

        long rcvd_len = read_single_packet(&rcvd_packet);
        if (rcvd_len > 0) {
            if((mReceiver.receive_packet(rcvd_packet, rcvd_len, receivedSequenceNumber, ackNum, other_receive_window, receivedACK, receivedFIN, receivedSYN)) < 0)
                perror("error receiving packet");
        }
        if(!(receivedACK && (ackNum == fin_ack_seq_num)))   //wait for FINACK!!!
            continue;
        std::cerr << "received ack for finACK, leaving program, ack num: " << ackNum << std::endl;
           return;
    }
}

void JJP::SYN_client() {
    bool receivedFIN = false, receivedACK = false, receivedSYN = false;
    char *rcvd_packet, *syn_packet;
    uint16_t ackNum, receivedSequenceNumber = 0;

    //send SYN
    srand(time(NULL));
    sequence_number = rand() % 30720;
    size_t syn_packet_len = mPacker.create_SYN(&syn_packet, sequence_number);
    mSender.send(syn_packet, syn_packet_len, sequence_number, false);
    std::cout << "Sending packet " << sequence_number << " " << mSender.get_cwnd() << " SYN" << std::endl;
    sequence_number = (sequence_number + syn_packet_len) % 30720;

    //wait for SYNACK and send ack once received

    while(true) {
        mSender.resend_expired_packets();
        receivedACK = receivedFIN = receivedSYN = false;

        long rcvd_len = read_single_packet(&rcvd_packet);
        if (rcvd_len > 0) {
            if((mReceiver.receive_packet(rcvd_packet, rcvd_len, receivedSequenceNumber, ackNum, other_receive_window, receivedACK, receivedFIN, receivedSYN)) < 0)
                perror("error receiving packet");
        }
        if(!(receivedSYN && receivedACK)) //wait for SYNACK!!!
            continue;

        //ok we got syn ack, lets ack and begin processing
        mSender.notify_ACK(ackNum);
        //std::cout << "received SYNack with sequence number " << receivedSequenceNumber << std::endl;
        mReceiver.set_seq_num(receivedSequenceNumber + rcvd_len);

        mSender.update_other_rwnd(other_receive_window); //update sender with rwnd
        mPacker.update_own_rwnd(mReceiver.get_avaliable_space());

        char* ACKPacket;
        size_t ACKPacket_len = mPacker.create_ACK(&ACKPacket, sequence_number, receivedSequenceNumber);
        mSender.send(ACKPacket, ACKPacket_len, sequence_number, true);
        //std::cout << "Sending ACK " <<  receivedSequenceNumber << " len" << ACKPacket_len << std::endl;
        sequence_number = (sequence_number + ACKPacket_len) % 30720;

        return;
    }
}

void JJP::SYN_server() {
    bool receivedFIN = false, receivedACK = false, receivedSYN = false;
    char *rcvd_packet;
    uint16_t ackNum, receivedSequenceNumber = 0;

    while(true) {
        mSender.resend_expired_packets();
        receivedACK = receivedFIN = receivedSYN = false;

        long rcvd_len = read_single_packet(&rcvd_packet);
        if (rcvd_len > 0) {
            if((mReceiver.receive_packet(rcvd_packet, rcvd_len, receivedSequenceNumber, ackNum, other_receive_window, receivedACK, receivedFIN, receivedSYN)) < 0)
                perror("error receiving packet");
        }
        if(!receivedSYN) //wait for SYN!!!
            continue;
        //ok we got syn, now lets send a SYNACK
        //std::cout << "received SYN with sequence number " << receivedSequenceNumber << std::endl;
        mReceiver.set_seq_num(receivedSequenceNumber + rcvd_len);

        mSender.update_other_rwnd(other_receive_window); //update sender with rwnd
        mPacker.update_own_rwnd(mReceiver.get_avaliable_space());

        char* syn_ack_packet = NULL;
        srand(time(NULL)-100);
        sequence_number = rand() % 30720;
        size_t syn_ack_packet_len = mPacker.create_SYNACK(&syn_ack_packet, sequence_number, receivedSequenceNumber);
        mSender.send(syn_ack_packet, syn_ack_packet_len, sequence_number, false);
        std::cout << "Sending packet " << sequence_number << " " << mSender.get_cwnd() << " SYN" << std::endl;
        //std::cout << "sending SYNACK with sequence number " << sequence_number << std::endl;
        sequence_number = (sequence_number + syn_ack_packet_len) % 30720;

        return;
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
void JJP::processing_thread(bool isClient) {
    bool receivedFIN = false, receivedACK = false, receivedSYN = false;
    char *rcvd_packet, *sending_packet, *update_packet;

    uint16_t ackNum, receivedSequenceNumber;
    //int updateTimer = 10; //every 10 loops when rwnd is 0 send update
    while(true) {
        receivedACK = receivedFIN = receivedSYN = false;

        buf_mutex->lock();
        long rcvd_len = read_single_packet(&rcvd_packet);
        if (rcvd_len > 0) {
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
          mSender.send(ACKPacket, ACKPacket_len, sequence_number, true);
          //std::cout << "Sending ACK " <<  receivedSequenceNumber << " len" << ACKPacket_len << std::endl;
          sequence_number = (sequence_number + ACKPacket_len) % 30720;
        }

        if (receivedACK) {
          std::cout << "Receiving packet " << receivedSequenceNumber << std::endl;
          mSender.notify_ACK(ackNum);
       }
       /*else if (mPacker.size() == 0 && isClient) {
         FIN_client();
         return;
       }
       if (receivedFIN && !isClient) {
         FIN_server(ackNum);
           return;
       }*/

        uint32_t available_space = mSender.get_avaliable_space();
        //std::cout << available_space << " " << mPacker.size() << std::endl;
        mSender.resend_expired_packets();

        if (available_space > 1024)
          available_space = 1024;
        size_t sending_packet_len = mPacker.create_data_packet(&sending_packet, available_space, sequence_number);
        if (sending_packet_len > 0) {
          mSender.send(sending_packet, available_space, sequence_number, false);
          std::cout << "Sending packet " << sequence_number << " " << mSender.get_cwnd() << std::endl;
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
        buf_mutex->unlock();
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

    while(totalBytesRead != 12) {
        int attemptedBytesRead = ::recvfrom(mSockfd, header+totalBytesRead, 12-totalBytesRead, MSG_PEEK, (struct sockaddr*) &client_addr, &clilen);
        if (attemptedBytesRead == -1)
          continue;
        totalBytesRead += attemptedBytesRead;
    }
    uint32_t packet_len = 0;
    memmove((char*) &packet_len, header, 4);
    char* received_packet = (char *) malloc(sizeof(char) * packet_len);

    totalBytesRead = 0;
    while(totalBytesRead < packet_len) {
        int attemptedBytesRead = ::recvfrom(mSockfd, received_packet + totalBytesRead, packet_len-totalBytesRead, 0, (struct sockaddr*) &client_addr, &clilen);
        if (attemptedBytesRead == -1)
          continue;
        totalBytesRead += attemptedBytesRead;
    }

    *packet = received_packet;

    mSender.set_recipient((struct sockaddr*) &client_addr, (socklen_t) clilen);

    return packet_len;
}

int JJP::accept(struct sockaddr *addr, socklen_t addrlen){
  mSender.set_recipient(addr,addrlen);

  SYN_server();

  std::thread process(&JJP::processing_thread, this, false);
  process.detach();

  return 0;
}

int JJP::connect(struct sockaddr *addr, socklen_t addrlen){
  mSender.set_recipient(addr,addrlen);

  SYN_client();

  std::thread process(&JJP::processing_thread, this, true);
  process.detach();

  return 0;
}

ssize_t JJP::write(const void *buf, size_t nbytes) {
    buf_mutex->lock();
    int i = mPacker.store((char*)buf, nbytes);
    buf_mutex->unlock();
    return i;
}

ssize_t JJP::read(void *buf, size_t nbytes) {
    buf_mutex->lock();
    int i = mReceiver.read(buf, nbytes);
    buf_mutex->unlock();
    return i;
}

int JJP::get_buf_size() {
  return mReceiver.get_buf_size();
}
