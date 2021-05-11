#pragma once
#ifndef OMD_PKT_QUEUE_HPP_
#define OMD_PKT_QUEUE_HPP_

#include "Packet.hpp"
#include "queue/spsc_flat_lf_queue.hpp"

namespace omd {

class PktQueue
{
 public:
  PktQueue():queue_(1024*1024*3){}
  PktQueue(size_t size) : queue_(size) {}

  void push(Packet const& pkt) {
    queue_.push(reinterpret_cast<char const*>(&pkt), pkt.size());
  }

  bool pop(Packet& pkt) {
    size_t len = sizeof(Packet);
    if (queue_.pop(reinterpret_cast<char*>(&pkt), len)) {
      assert(pkt.body_size() == len - sizeof(header_t));
      return true;
    }
    return false;
  }

 private:
  spsc_flat_lockfree_queue<unsigned long> queue_;
};

} // namespace omd

#endif

