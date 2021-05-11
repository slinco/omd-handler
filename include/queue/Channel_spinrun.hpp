#ifndef CHANNEL_HPP_
#define CHANNEL_HPP_

#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <vector>
#include <atomic>

template<typename session_type, typename queue_type>
class Channel : private boost::noncopyable
// queue observer and manager; dispatch messages to appropriate sessions
// Channel can NOT depend on node_type, since different thread my use different node type
// (though the definition of nodes are all the same, but staticed shared memory block
//  are differentiated based on node type)
{
  typedef boost::shared_ptr<session_type> session_ptr;

public:
  Channel(boost::asio::io_service& io_service, size_t queue_size)
    : queue_(queue_size), stop_ (false)
  {
  }

  template <typename node_type, void (session_type::*handler)(node_type&)>
  void run()
  {
    stop_ = false;
    while (!stop_) 
      deliver<node_type, handler>();
  }

  void stop()
  {
    stop_ = true;
  }

  void join(session_ptr session)
  {
    sessions_.push_back(session);
  }

  void leave(session_ptr session)
  {
    sessions_.erase(std::find(sessions_.begin(), sessions_.end(), session));
  }

  template <typename node_type>
  void push_in(node_type const& node)
  {
    queue_.push(node);
  }

  template<typename node_type, void (session_type::*handler)(node_type&)>
  void deliver()
  {
    static size_t i = 0;      // i for selection of an appropriate session

    node_type node;
    while (queue_.pop(node)) { // every time we use the same memory shared by the node_type
      (sessions_[i].get()->*handler)(node);
      i = (i + 1) % sessions_.size(); // for next time picking up a session
    }
  }

private:
  std::vector<session_ptr> sessions_;
  queue_type queue_;
  bool stop_; 
};


#endif
