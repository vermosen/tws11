#include "mtclient.h"

mtclient::mtclient(
    int id
  , std::string host
  , int port
  , bool extra
  , int timeout
  , std::size_t pool_size
  , logger_type log
  , const std::string& timezone) 
  : m_tp(pool_size)
  , m_client(id, host, port, extra, timeout, log, timezone) {}

  bool mtclient::connect() { return m_client.connect(); }
  void mtclient::disconnect() { m_client.disconnect(); }