#ifndef MPMC_FLAT_QUEUE_HPP_
#define MPMC_FLAT_QUEUE_HPP_

// Not used in this project currently

#include <boost/utility.hpp>     // for noncopyable

// Doc: requirement for event type (default constroctor, exception safe assignment, less comparasion)

template <typename INT, size_t Size = 1024*100>  // Clock should be a chrono clock
class mpmc_flat_queue : boost::noncopyable 
{
  enum {element_width = sizeof(INT) / sizeof(char) };

public:
  explicit mpmc_flat_queue() :
    first_(0), last_(0)
  {}

  void push(char const* data, size_t length)
  // Modify last_, and in rare case, fisrt_ modified by clean()
  {
    INT len = length;
    length = (length + element_width - 1) / element_width;
    INT const* src = reinterpret_cast<INT const*>(data);

    boost::lock_guard<boost::mutex> push_lock(push_mutex_); // cannot have two threads do the job simutaneously
    
    if (size() > 0) {
      data_[last_] = len;
      last_ = next(last_);
    }

    do { // actually waiting for some space spared
      auto sz = size();  // maybe changed every loop
      for (; length > 0 && sz > 0; --length, --sz, ++src) {
        data_[last_] = *src;
        last_ = next(last_);
      }
    } while (length > 0)
  }

  bool pop_nonblock(char* data, size_t& length)
  {
    INT len;
    INT const* dest = reinterpret_cast<INT const*>(data);

    boost::lock_guard<boost::mutex> pop_lock(pop_mutex_); // cannot have two threads do the job simutaneously
    auto sz = size();
    if (sz == 0) return false;
    
    auto len_in_byte = data_[first_];
    first_ = next(first_);
    len = (len_in_byte + element_width - 1) % element_width;
    assert(length >= len_in_byte + element_width - 1);  // check if room provided big enough
    length = len_in_byte;  // return to caller
    while (len > 0) {
      *dest++ = data_[first_];
      first_ = next(first_);
    }
  }

  void pop(char* data, size_t& length)  // block pop
  {
    INT len;
    INT const* dest = reinterpret_cast<INT const*>(data);

    boost::lock_guard<boost::mutex> pop_lock(pop_mutex_); // cannot have two threads do the job simutaneously
    auto sz = size();
    if (sz == 0)
      cv.wait(pop_lock, [sz](){return sz != 0});
      
    
    auto len_in_byte = data_[first_];
    first_ = next(first_);
    len = (len_in_byte + element_width - 1) % element_width;
    assert(length >= len_in_byte + element_width - 1);  // check if room provided big enough
    length = len_in_byte;  // return to caller
    while (len > 0) {
      *dest++ = data_[first_];
      first_ = next(first_);
    }
  }

private:
  int size()
  {
    int n = int(last_ - first_);
    n = n < 0 ? n + Size + 1 : n;
    return n;
  }

  size_t next(size_t current)
  {
    return (current + 1) % (Size + 1);
  }

  INT data_[Size+1]; // one extar element to avoid simultaneously access head_ and tail_
  size_t first_, last_;
  mutable boost::mutex push_mutex_, pop_mutex_;
  boost::condition_variable cv_;
};

#endif
