#ifndef QS_CHANNEL_HPP_
#define QS_CHANNEL_HPP_

#include <boost/noncopyable.hpp>
#include <boost/ref.hpp>  // for boost:ref()
#include <boost/asio.hpp>
#include <boost/function.hpp>

// By design, Channel is ONE Consumer no matter how user to get data from it.
// User provide a delivery handler for getting data and Channel guarantee
// it will be called by a single thread.
//
// The producer side, however, can be multiple if macro CHANNEL_MULTIPLE_PRODUCERS
// defined. If not, user should make sure only one thread pushes data into
// the channel.

#if defined(CHANNEL_MULTIPLE_PRODUCERS)
#include <atomic>
#endif

//-----------------------------------------------------------------------------
// Channel Requirements for SessionMgr:
//
//     template <typename Session>
//     struct SessionMgr {
//       typedef Session session_type;
//       typedef boost::shared_ptr<session_type> session_ptr;
//
//       void join(session_ptr);
//       void leave(session_ptr);
//       session_ptr pickup(node_type const& node); // node_type can be templated
//     };
//
// Channel Requirements for QueueType:
//
//     struct QueueType {
//       QueueType(size_t size);
//       void push(node_type const& node);
//       bool pop(node_type& node); // this node_type can be diff from that in push
//    };
//
// We don't require Session Policy classes and QueueType classes be derived from
// an (abstract) base class since this is nothing about runtime thing.
//
//-----------------------------------------------------------------------------

template<typename SessionMgr, typename QueueType>
class Channel : private boost::noncopyable
// queue observer and manager; dispatch messages to appropriate sessions
// Channel can NOT depend on node_type, since different thread my use different node type
// (though the definition of nodes are all the same, but staticed shared memory block
//  are differentiated based on node type)
{
  typedef typename SessionMgr::session_ptr session_ptr;

public:
  Channel(boost::asio::io_service& io_service, size_t queue_size, char const* id = nullptr)
  // Init but not specify delivery task. 
  // Later user can call setDeliveryHandle() to do it.
    : sessions_()
    , queue_(queue_size)
    , io_service_(io_service)
    , work_(io_service)
    , id_(id, id?strlen(id):0)
#if defined(CHANNEL_MULTIPLE_PRODUCERS)
    , push_lock_(false)
#endif
  {}

  template <typename node_type, typename handle_type>
  void setDeliveryHandle(handle_type handler)
  {
    handler_ = [this, handler]{this->deliver<node_type, handle_type>(handler);};
  }

  void run()
  { io_service_.run(); }

  void stop()
  { io_service_.stop(); }

  void join(session_ptr session)
  { sessions_.join(session); }

  void leave(session_ptr session)
  { sessions_.leave(session); }

  template <typename node_type>
  void push_in(node_type const& node)
  // Allow multiple threads to push
  {
#if defined(CHANNEL_MULTIPLE_PRODUCERS)
    for(bool expected = false; !push_lock_.compare_exchange_weak(expected, true); expected = false);
#endif
    queue_.push(node);
#if defined(CHANNEL_MULTIPLE_PRODUCERS)
    push_lock_ = false;
#endif
    io_service_.post(handler_);  // thread safe
  }

  template<typename node_type, typename handle_type>
  void deliver(handle_type handler)
  // handle_type must be of form: void handler(session_ptr, node_type_revariant);
  // where node_type_variant can be a value of node, or (const) reference to a node.
  // WE let handle_type as a template type instead of function pointer parameter
  // so that node_type can be in different forms, for example, 
  //   node_type&, node_type const& or node_type (by value)
  {
    //using boost::decay;
    //using boost::function_traits;
    //typedef typename decay<typename function_traits<handle_type>::arg2_type>::type node_type;
    node_type node;
    while (queue_.pop(node)) { // every time we use the same memory shared by the node_type
      auto s_ptr = sessions_.pickup(node);
      if (s_ptr) // otherwise discard the message
        handler(s_ptr, boost::ref(node));
    }
  }

private:
  SessionMgr sessions_;  // hold an array of session_ptrs and admin them
  QueueType  queue_;     // msgs pass through Channel via the queue

  boost::asio::io_service& io_service_;
  boost::asio::io_service::work work_;
  boost::function<void ()> handler_;
  std::string id_;  // for debug purpose

#if defined(CHANNEL_MULTIPLE_PRODUCERS)
  std::atomic_bool push_lock_;
#endif
};

#endif
