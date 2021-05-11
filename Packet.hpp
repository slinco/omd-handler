#pragma once
#ifndef OMD_PACKET_HPP_
#define OMD_PACKET_HPP_

#include <cstdint>

namespace omd {

    const int32_t NULL_INT32 = 0x80000000;
    const int64_t NULL_INT64 = 0x800000000000000;

    struct header_t {
        uint16_t pktSize;
        uint8_t  msgCount;
        char     filler[1];
        uint32_t seqNum;
        uint64_t sendTime;
    };

    const size_t MAX_PACKET_SIZE = 1500;
    const size_t MAX_BODY_SIZE = MAX_PACKET_SIZE - sizeof(header_t);

    struct body_t {
        body_t() = default;
        body_t(body_t const&) = default;
        body_t& operator=(body_t const&) = default;

        template <typename MsgType>
        body_t(MsgType const& msg) {
            *this = msg;
        }

        template <typename MsgType>
        body_t& operator=(MsgType const& msg) {
            memcpy(data, &msg, sizeof(MsgType));
            msgSize = sizeof(MsgType) + sizeof(msgSize) + sizeof(msgType);
            msgType = MsgType::type;
            return *this;
        }

        template <typename MsgType>
        operator MsgType const& () const {
            assert(sizeof(MsgType) == msgSize - sizeof(msgSize) - sizeof(msgType));
            return *reinterpret_cast<MsgType const*>(data);
        }

        uint16_t msgSize;
        uint16_t msgType;

    private:
        char data[MAX_BODY_SIZE - sizeof(uint16_t)*2];
        friend class DataFeed;
        friend class Packet; // in Packet::setRequest()
    };


    class MsgIterator
    {
    private:
        uint8_t const* curr_;

    public:
        template <typename T>
        MsgIterator(T const* p) : curr_(reinterpret_cast<uint8_t const*>(p)) {}

        MsgIterator(MsgIterator const&) = default;
        MsgIterator& operator=(MsgIterator const&) = default;

        body_t const* operator->() const
        { return reinterpret_cast<body_t const*>(curr_); }

        body_t const& operator*() const
        { return *reinterpret_cast<body_t const*>(curr_); }

        MsgIterator& operator++()
        { curr_ += *reinterpret_cast<uint16_t const*>(curr_); return *this; }

        bool operator!=(MsgIterator const& it)
        { return curr_ != it.curr_; }
    };

    class Packet
// Consists of a header_t and a body_t, representing a full packet 
// received from OMD servers.
    {
    public:
        friend class UdpClient;
        //template <typename Ch> friend class TcpClient;
        //friend class PacketQueue;

        typedef MsgIterator iterator;

        Packet() = default;
        Packet(Packet const& o) { memcpy(this, &o, o.size()); }

        uint32_t size()     const { return header_.pktSize; }
        uint32_t seqNum()   const { return header_.seqNum; }
        uint32_t msgCount() const { return header_.msgCount; }
        uint64_t sendTime() const { return header_.sendTime; }
        size_t body_size()  const { return size() - sizeof(header_t); }

        bool isHeartbeat() const { return msgCount() == 0; }

        iterator begin() const { return iterator(&body_); }
        iterator end() const { return iterator(reinterpret_cast<uint8_t const*>(this)+size()); }

        // Used for Tcp Retransmission Request
        template<typename MsgType>
        void setRequest(MsgType const& b) {
            body_ = b;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
            header_.pktSize = sizeof(header_t) + sizeof(MsgType) + offsetof(body_t, data);
#pragma GCC diagnostic pop
            header_.msgCount = 1;
            header_.sendTime = 0; // not used; std::chrono::system_clock::now().time_since_epoch().count();
            header_.seqNum = 0;
        }

    private:
        header_t header_;
        body_t   body_;
    };

} // namespace omd

#endif

