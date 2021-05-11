#ifndef QUEUE_NODE_HPP_
#define QUEUE_NODE_HPP_

#include <boost/array.hpp>
#include <boost/fusion/tuple.hpp>
#include <boost/type_traits/remove_reference.hpp> 
#include <boost/move/utility.hpp> // for boost::forward
#include <boost/noncopyable.hpp>

template<size_t fixed_buf_size, int type_id>  
class SharedAlloc : boost::noncopyable
{
private:
  typedef boost::array<char, fixed_buf_size> buffer_type;
  buffer_type buffer_;   // first element is always tag

  SharedAlloc() {};

public:
  static SharedAlloc& get() {
    static SharedAlloc instance;
    return instance; 
  }

  char* allocate(size_t n) {
    assert(n * sizeof(char) < fixed_buf_size);
    return reinterpret_cast<char*>(buffer_.data()); 
  }  

  void deallocate(char*, size_t) {}

  constexpr size_t max_size() const { return fixed_buf_size; }
};


template<size_t fixed_buf_size, int type_id>
// type_id purely for differentiate node types for different threads
class Node
// All the node share the same memory, so before create a new one, 
// copy the content of old one to your own place
// ASSUME all the types of T1, T2, ..., Tn are C++ basic types (not poniters)
// We let every thread use an unique type of Node to avoid node memory sharing 
// between threads.
{
private:
  template <typename queue_type> friend class node_queue;

  typedef SharedAlloc<fixed_buf_size, type_id> Alloc; // if change here, change node_queue too
  typedef int tag_type; 

  char*  storage_;
  size_t length_;  // not include size of tag. 

  // tag should be treated as a seperate field though stored in buffer_

  void setTag(tag_type tag) { // first element of buffer is for tag
    *(reinterpret_cast<tag_type*>(storage_)) = tag;
  }

  // address of buffer for data started after tag
  char* data_addr() { return storage_ + sizeof(tag_type); }

  // These 4 used by friend Queue class
  char* node_addr() { return storage_; }
  char const* node_addr() const { return storage_; }              // addr start from tag
  size_t node_size() const { return length_ + sizeof(tag_type); } // size of tag+data 
  size_t node_max_size() const { return Alloc::get().max_size(); } 

  template<typename T> using decay_t = typename boost::remove_reference<T>::type;

public:
  template<typename... Types>
  using tuple = boost::fusion::tuple<Types...>;

  size_t length() const { return length_; }   // data size
  char const* data() const { return length() ? storage_+sizeof(tag_type) : nullptr; }
  tag_type tag() const { return *reinterpret_cast<tag_type const*>(storage_); }

  Node(tag_type tag = 0)
    : length_(0)
  {
    storage_ = Alloc::get().allocate(sizeof(tag_type));
    setTag(tag);
  }

  template <typename... Types> 
  Node(tag_type tag, Types&&... args)
  // If some types if T[n], please use boost::array<T,N> instead
  // DO NOT use std::reference_wrapper type.
  {
    typedef tuple<decay_t<Types>...> tuple_type;
    storage_ = Alloc::get().allocate(sizeof(tag_type) + sizeof(tuple_type));
    setTag(tag);
    new (data_addr()) tuple_type(boost::forward<Types>(args)...);
    length_ = sizeof(tuple_type);
  }

  Node(void const* d, size_t length, tag_type tag = 0) // for msgQueue
  {
    storage_ = Alloc::get().allocate(sizeof(tag_type) + length);
    setTag(tag);
    length_ = length;
    memcpy(data_addr(), d, length_);
  }

  Node(Node&& n) {  // only movable, not copyable
    storage_ = n.storage_;
    length_ = n.length_;
    n.storage_ = nullptr;
    n.length_ = 0;
  }

  Node& operator=(Node&& n) {  // only movable, not copyable
    storage_ = n.storage_;
    length_ = n.length_;
    n.storage_ = nullptr;
    n.length_ = 0;
    return *this;
  }
 
  ~Node() {
    Alloc::get().deallocate(storage_, node_size());
  }

  template <typename... Types>
  operator tuple<Types...> const&() const {
    return *reinterpret_cast<tuple<Types...> const*>(data());
  }

  template <typename... Types>
  operator tuple<Types...> &() {
    return *reinterpret_cast<tuple<Types...>*>(data_addr());
  }
};

template <typename queue_type>
class node_queue
{
private:
  queue_type queue_;

public:
  node_queue(size_t size)
    : queue_(size)
  {}

  template <typename node_type>   // different thread use different node_type
  void push(node_type const& node)
  {
    queue_.push(node.node_addr(), node.node_size());
  }
  
  template <typename node_type>
  bool pop(node_type& node) 
  // Tightly coupled with node_type!!!! for speed
  // If Alloc policy changed, here should be changed too.
  {
    size_t len = node.node_max_size();
    if (queue_.pop(node.node_addr(), len)) {
       node.length_ = len - sizeof(typename node_type::tag_type);
       return true;
    }
    return false;
  }

};

// NodeQueue don't specify node type. 
// Node Type is specified in its methods of push and pop.
// This way we allow different types of nodes with the same structure
// but holding different memory area (see below) be operated on the queue.
#include "spsc_flat_lf_queue.hpp"
typedef node_queue<spsc_flat_lockfree_queue<unsigned long>> NodeQueue; 

#endif

