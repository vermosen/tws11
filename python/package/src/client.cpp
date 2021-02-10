
#include "client.h"

#include <sstream> // stringstream
#include <iomanip> // put_time

#include <tws/EReader.h>

client::client(
    int id
  , std::string host
  , int port
  , bool extra
  , int timeout
  , logger_type log
  , const std::string& tz)

  : m_log(log) 
  , m_state(state::idle)
  , m_id(id)
  , m_host(host)
  , m_port(port)
  , m_extra(extra)
  , m_tz(tz)
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
  return (m_state != state::idle);
}

void client::reset() {
             m_log = {}; 
         m_bar_hdl = {};
       m_chain_hdl = {};
     m_details_hdl = {};
  m_completion_hdl = {};
}

void client::request(
      TickerId id 
    , const Contract& c
    , const std::string& field
    , const std::string& bar
    , const std::string& dur
    , const std::chrono::system_clock::time_point& end) {

  std::stringstream ss;
  std::time_t t = std::chrono::system_clock::to_time_t(end);
  std::tm m = *std::gmtime(&t);
  ss << std::put_time(&m, "%Y%m%d %H:%M:%S") << " " << m_tz;

  m_rd.m_sock.reqHistoricalData(id, c, ss.str(), dur, bar, field, false, 2, false, TagValueListSPtr());
  m_state = state::hist_ack;
}

void client::get_chain(const Contract& c, const std::string& exchange) {

  //see https://interactivebrokers.github.io/tws-api/classIBApi_1_1EClient.html#adb17b291044d2f8dcca5169b2c6fd690
  m_rd.m_sock.reqSecDefOptParams(1, c.symbol, exchange, c.secType, c.conId);
  m_state = state::chain_ack;
}

void client::get_details(int id, const Contract& c) {
  m_rd.m_sock.reqContractDetails(id, c);
  m_state = state::details_ack;
}

void client::securityDefinitionOptionalParameter(
    int reqId
  , const std::string& exchange
  , int underlying
  , const std::string& tr_class
  , const std::string& multiplier
  , const std::set<std::string>& expirations
  , const std::set<double>& strikes) {

  for (auto& it : strikes) {
    for (auto& jt : expirations) {
      option_desc d = { underlying, tr_class, multiplier, it, jt, exchange };
      m_chain_hdl(std::move(d));
    }
  }
}

void client::contractDetails(int id, const ContractDetails& c) {
  m_details_hdl(id, c);
}

void client::error(int id, int ec, const std::string& msg) {
  
  if (ec == 2107 || ec == 2106) { // garbage warnings about datafarm
    return;
  }

  if (ec == 162) {                // msg: Trading TWS session is connected from a different IP address 
    m_state = state::idle;
  }

  std::stringstream ss; ss
      << "[error] id - "
      << id
      << " [" << ec << "] "
      << msg
      ;

  m_log(ss.str());
}
void client::historicalData(TickerId id, const Bar& b) {
  m_bar_hdl(id, b);
}

void client::historicalDataEnd(int id, const std::string& start, const std::string& end) {

  /* std::stringstream ss; ss
    << "[info] " 
    << "historical data returned for req - "
    << id << ", start - "
    << start << ", end - "
    << end
    ;

  m_log(ss.str()); */

  m_state = state::connect;
  m_completion_hdl(id);
}

void client::securityDefinitionOptionalParameterEnd(int id) {
  m_state = state::connect;
  m_completion_hdl(id);
}

void client::contractDetailsEnd(int id) {
  m_state = state::connect;
  m_completion_hdl(id);
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