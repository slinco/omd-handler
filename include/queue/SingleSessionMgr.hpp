#ifndef SINGLE_SESSION_MGR_HPP_
#define SINGLE_SESSION_MGR_HPP_

template <typename Session> 
class SingleSessionMgr // Only one session is allowed for a channel
{
public:
  typedef boost::shared_ptr<Session> session_ptr;

private:
  session_ptr session_;

public:
  void join(session_ptr session) { session_ = session; }
  void leave(session_ptr session) { session_.reset(); }
  template <typename NodeType>
  session_ptr pickup(NodeType const& node) { return session_; }
};

#endif

