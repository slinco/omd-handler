#pragma once
#ifndef OMD_UDPCLIENT_HPP_
#define OMD_UDPCLIENT_HPP_

#include <chrono>
#include "spdlog/spdlog.h"
#include "OMDCPrinter.hpp"
#include <thread>
#include <iostream>
#include "Packet.hpp"
#include "PktQueue.cpp"

#include <boost/atomic.hpp>
using namespace std::chrono;

extern uint64_t micros();

#pragma pack(push, 1)
struct header_t {
  uint16_t pktSize;
  uint8_t  msgCount;
  char     filler[1];
  uint32_t seqNum;
  uint64_t sendTime;
};
#pragma pack(pop)

namespace omd {

    class UdpClient
    {
    public:
        UdpClient(const std::string& logger_name, const std::string& nic, int listen_port, const std::string& group_ip)
        {
          //init logger
          _logger_name = logger_name;
          log = spdlog::get(logger_name);
          processor.initlog(logger_name);

          struct sockaddr_in localSock;
          struct ip_mreq     group;
          /* Create a datagram socket on which to receive. */
          sd = socket(AF_INET, SOCK_DGRAM, 0);
          if(sd < 0){
            log->error("Opening datagram socket error");
            exit(1);
          }else{
            log->info("Opening datagram socket....OK");
          }

          /* Enable SO_REUSEADDR to allow multiple instances of this */
          /* application to receive copies of the multicast datagrams. */
          {
            int reuse = 1;
            if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0){
              log->error("Setting SO_REUSEADDR error");
              close(sd);
              exit(1);
            }
            else {
              log->info("Setting SO_REUSEADDR...OK");
            }
          }

          /* Bind to the proper port number with the IP address */
          /* specified as INADDR_ANY. */
          memset((char *) &localSock, 0, sizeof(localSock));
          localSock.sin_family = AF_INET;
          localSock.sin_port = htons(listen_port);
          localSock.sin_addr.s_addr = INADDR_ANY;

          if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock))){
            log->error("Binding datagram socket error");
            close(sd);
            exit(1);
          }else {
            log->info("Binding datagram socket...OK.");
          }

          /* Join the multicast group 226.1.1.1 on the local 203.106.93.94 */
          /* interface. Note that this IP_ADD_MEMBERSHIP option must be */
          /* called for each local interface over which the multicast */
          /* datagrams are to be received. */
          group.imr_multiaddr.s_addr = inet_addr(group_ip.c_str());
          group.imr_interface.s_addr = inet_addr(nic.c_str());
          if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0){
            log->error("Adding multicast group error");
            close(sd);
            exit(1);
          }else{
            log->info("Adding multicast group...OK.");
          }

          //set buffer size
          uint64_t recvbuf_size = 20*1024*1024;  //20 MB
          if (setsockopt(sd, SOL_SOCKET, SO_RCVBUF, &recvbuf_size, sizeof(recvbuf_size)) < 0) {
            log->error("Set SO_RCVBUF error");
            close(sd);
            exit(1);
          }else{
            log->info("Set recv buffer {}...OK.", recvbuf_size);
          }

          //set Time-To-Live
          int ttl = 255; /* max = 255 */
          if(setsockopt(sd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0){
            log->error("Set ttl error");
            close(sd);
            exit(1);
          }else{
            log->info("Set ttl {}...OK.", ttl);
          }
        }

        void start(bool isRecvOnly){
          if(isRecvOnly) {
            log->info("Start UdpClient recv thread");
            producer_thread = std::thread(&UdpClient::recv, this);
          }else{
            log->info("Start UdpClient recvAndPush thread");
            producer_thread = std::thread(&UdpClient::recvAndPush, this);
          }
          consumer_thread = std::thread(&UdpClient::consume, this);
        }

        void join(){
          producer_thread.join();
          consumer_thread.join();
        }

        void consume(){
          boost::atomic_int64_t expected_seq( 0 );
          int lost_count = 0;
          bool isFirstPacket = true;
          Packet popPkt;
          while (true) {
            while (queue_.pop(popPkt) ) {
              header_t hd_ = popPkt.header_;

              if (isFirstPacket) {
                expected_seq = popPkt.header_.seqNum;
                isFirstPacket = false;
              }

              if(hd_.msgCount != 0) {
                //check packet loss
                if (hd_.seqNum != expected_seq) {
                  lost_count += hd_.seqNum - expected_seq > 0 ? hd_.seqNum - expected_seq : 0;
                  mainlog->error("[{}][LOSS]seq:{}~{},total:{}|[RECV]seq:{},msg count:{}|type:{}", _logger_name, expected_seq, hd_.seqNum - 1, lost_count, hd_.seqNum, hd_.msgCount, popPkt.body_.msgType);
                  expected_seq = hd_.seqNum;
                }
                expected_seq += hd_.msgCount;
              }

              uint64_t sendtime = hd_.sendTime / 1000;
              int size = hd_.pktSize;
              processMsg<omd::PrintProcessor>(reinterpret_cast<char *>(&popPkt), size, sendtime);
            }
          }
        }

        void recvAndPush() {
          char mReadBuffer[MAX_PACKET_SIZE];
          int stats_count = 0;
          uint64_t us_execution = 0;
          while(true) {
            ssize_t nbytes = recvfrom(sd, mReadBuffer, MAX_PACKET_SIZE, 0, (struct sockaddr *) &other, &other_len);
            if (nbytes != -1) {
              uint64_t us_start = micros();
              Packet* pPacket = (Packet*)mReadBuffer;
              queue_.push(*pPacket);
              uint64_t us_end = micros();
              us_execution += (us_end - us_start);
              if (++stats_count == stats_iterations) {
                log->info("{} iterations - execution time:{} us, avg:{} us", stats_count, us_execution, static_cast<double> (us_execution) / stats_count);
                stats_count = 0;
                us_execution = 0;
              }
            }
          }
        }

        void recv() {
          char mReadBuffer[MAX_PACKET_SIZE];
          int stats_count = 0;
          int lost_count = 0;
          uint64_t us_execution = 0;
          boost::atomic_int64_t expected_seq( 0 );
          bool isFirstPacket = true;
          while(true) {
            ssize_t nbytes = recvfrom(sd, mReadBuffer, MAX_PACKET_SIZE, 0, (struct sockaddr *) &other, &other_len);
            if (nbytes != -1) {
              uint64_t us_start = micros();
              Packet* pPacket = (Packet*)mReadBuffer;
              header_t hd_ = pPacket->header_;

//              log->info("[HEADER]seq:{},msg count:{}", hd_.seqNum);
              if (isFirstPacket) {
                expected_seq = hd_.seqNum;
                isFirstPacket = false;
              }

              if(hd_.msgCount != 0){
                //check packet loss
                if (hd_.seqNum != expected_seq) {
                  lost_count += hd_.seqNum - expected_seq > 0 ? hd_.seqNum - expected_seq : 0;
                  mainlog->error("[{}][LOSS]seq:{}~{},total:{}|[RECV]seq:{},msg count:{}|type:{}", _logger_name, expected_seq, hd_.seqNum - 1, lost_count, hd_.seqNum, hd_.msgCount, pPacket->body_.msgType);
                  expected_seq = hd_.seqNum;
                }
                expected_seq += hd_.msgCount;
              }

              uint64_t us_end = micros();
              us_execution += (us_end - us_start);
              if (++stats_count == stats_iterations) {
                log->info("{} iterations - execution time:{} us, avg:{} us", stats_count, us_execution, static_cast<double> (us_execution) / stats_count);
                stats_count = 0;
                us_execution = 0;
              }
            }
          }
        }

        template <typename _Processor>
        void processMsg(char* msg, size_t size, uint64_t sendtime)
        {
            processor.update(sendtime);
            parser.parse(msg, size, processor);
        }
     private:
        const int stats_iterations = 10000;
        uint64_t now = micros();
        PrintProcessor processor{now};
        Packet packet_;
        omd::PktQueue queue_;
        std::thread producer_thread;
        std::thread consumer_thread;
        openomd::OmdcParser parser;
        int sd = -1; //socket descriptor
        struct sockaddr_in  other;
        socklen_t other_len = sizeof(other);
        std::string _logger_name;
        std::shared_ptr<spdlog::logger> log = spdlog::get("main");
        std::shared_ptr<spdlog::logger> mainlog = spdlog::get("main");
    };

}// namespace omd

#endif