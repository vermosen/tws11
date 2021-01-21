
#include "client.h"

#include <sstream>

#include <tws/EReader.h>

client::client(
    int id
  , std::string host
  , int port
  , bool extra
  , int timeout
  , logger_type log)

  : m_sink(handle_type())
  , m_log(log) 
  , m_state(state::idle)
  , m_id(id)
  , m_host(host)
  , m_port(port)
  , m_extra(extra)
  , m_rd(*this, timeout) {}

bool client::connect() {
  auto result = m_rd.start();
  if (result) {
    m_state = state::connect;
  }

  return result;
}

void client::disconnect() {
  m_rd.stop();
  m_state = state::idle;
}

bool client::run() {
  m_rd.m_oss.waitForSignal();
  m_rd.m_impl->processMsgs();
  return (m_state != state::idle && m_state != state::connect);
}

void client::request(
      const Contract& c
    , const std::string& field
    , const std::string& bar
    , const std::string& dur
    , const std::chrono::system_clock::time_point& end) {

      std::stringstream send; send
        << ""
        ;

      m_rd.m_sock.reqHistoricalData(1, c, send.str(), dur, bar, field, false, 2, false, TagValueListSPtr());
      m_state = state::hist_ack;
    }

void client::error(int id, int ec, const std::string& msg) {
  
  if (ec != 2107 && ec != 2106) { // garbage warnings about datafarm
    std::stringstream ss; ss
        << "[error] id - "
        << id
        << " [" << ec << "] "
        << msg
        ;
  
    m_log(ss.str());
  }
  else if (ec == 162) { // Trading TWS session is connected from a different IP address 

  }

}
void client::historicalData(TickerId id, const Bar& b) {
  m_sink(id, b);
}

void client::historicalDataEnd(int req, const std::string& start, const std::string& end) {

  std::stringstream ss; ss
    << "[info] " 
    << "historical data returned for req - "
    << req << ", start - "
    << start << ", end - "
    << end
    ;

  m_log(ss.str());

  m_state = state::connect; // reset the client state
}

namespace details {
  reader::reader(client& cl, int timeout) 
    : m_cl(cl)
    , m_oss(/*waitTimeout=*/timeout)
    , m_sock(&m_cl, &m_oss)
    , m_impl(nullptr) {}

  reader::~reader() {}

  void reader::stop() {
    m_sock.eDisconnect();
  }

  bool reader::start() {

    auto id    = m_cl.id()        ;
    auto host  = m_cl.host()      ;
    auto port  = m_cl.port()      ;
    auto extra = m_cl.extra_auth();

    auto res = m_sock.eConnect(host.c_str(), port, id, extra);

    if (res) {
      m_impl = std::unique_ptr<EReader>(new EReader(&m_sock, &m_oss));
      m_impl->start();
    }

    return res;
  }
}