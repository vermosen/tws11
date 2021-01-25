

#include "threadpool.h"
#include "client.h"

class mtclient {
public:
  using logger_type         = typename client::logger_type;
  using data_handle_type    = typename client::data_handle_type;
  using chain_handle_type   = typename client::chain_handle_type;
  using details_handle_type = typename client::details_handle_type;

public:
  mtclient(int
    , std::string
    , int
    , bool
    , int
    , std::size_t
    , logger_type = logger_type()
    , const std::string& = "America/New_York");

public:
  bool    connect();
  void disconnect();

public:
  bool connected() const { return m_client.connected(); }

private:
  threadpool  m_tp    ;
  client      m_client;
};