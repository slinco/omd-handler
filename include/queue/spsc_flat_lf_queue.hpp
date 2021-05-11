#ifndef SPSC_FLAT_LOCKFREE_QUEUE_HPP_
#define SPSC_FLAT_LOCKFREE_QUEUE_HPP_

#include <boost/lockfree/spsc_queue.hpp>

template <typename INT>
class spsc_flat_lockfree_queue
{
public:
  enum { element_width = sizeof(INT) / sizeof(char) };
  typedef boost::lockfree::spsc_queue<INT> queue_type;

  explicit spsc_flat_lockfree_queue(size_t size)
    : queue_(size)
  {
  }

  void push(char const* body, size_t length)
  // length is in bytes and should be smaller than the size of queue ((queue size-1) * element_width + 1)
  {
     INT len = length; // in terms of bytes (chars)
     while (!queue_.push(len));

     length = (length + element_width - 1) / element_width; // in terms of INT (CPU word)
     size_t n;
     for (auto p = reinterpret_cast<INT const*>(body); length > 0; length -= n, p += n) {
       n = queue_.push(p, length);
     }
  }

  bool pop(char* body, size_t& body_size)
  // Assume body is big enough
  {
    INT len;
    if (!queue_.pop(len)) { // it's empty
      return false;
    }

    assert(body_size >= len + element_width - 1);
    body_size = len;  // need to return the real len of message to caller

    size_t length = (len + element_width - 1) / element_width; // in terms of INT (CPU word)
    size_t n;
    for (INT* p = reinterpret_cast<INT*>(body); length > 0; length -= n, p += n) {
      n = queue_.pop(p, length);
    }
    assert(length == 0);

    return true;
  }

  void clear()
  {
    queue_.consume_all([](INT&){});
  }

private:
  queue_type queue_;
};

#endif
