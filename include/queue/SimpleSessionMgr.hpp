#ifndef SIMPLE_SESSION_MGR_HPP_
#define SIMPLE_SESSION_MGR_HPP_

template <typename Session> 
class SimpleSessionMgr
{
public:
  typedef boost::shared_ptr<Session> session_ptr;

private:
  std::vector<session_ptr> sessions_;

public:
  void join(session_ptr session)
  {
    sessions_.push_back(session);
  }

  void leave(session_ptr session)
  {
    sessions_.erase(std::find(sessions_.begin(), sessions_.end(), session));
  }

  template<typename node_type>
  session_ptr pickup(node_type const& node) // may select session based on node 
  {
    static size_t i = 0;
    session_ptr s_ptr = sessions_[i];
    i = (i + 1) % sessions_.size();
    return s_ptr;
  }
};

#endif


