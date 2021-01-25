
#pragma once
#ifndef TWS11_SRC_CLIENT_H
#define TWS11_SRC_CLIENT_H

#include <memory>
#include <chrono>
#include <string>
#include <functional>

#include <tws/DefaultEWrapper.h>
#include <tws/EReaderOSSignal.h>
#include <tws/EClientSocket.h>

#include "option_desc.h"

class EReader;

class client; // fwd decl

namespace details {

  class reader {
  public:
    reader(client&, int);
    ~reader();

  public:
    bool start();
    void stop();
  
  private:
    friend client;
    client&         m_cl           ;
    EReaderOSSignal m_oss          ;
    EClientSocket   m_sock         ;
    std::unique_ptr<EReader> m_impl;
  };
};

class client : public DefaultEWrapper {
public:
  enum class state {
      idle
    , connect
    , hist_ack
    , chain_ack
    , details_ack
  };

public:
  using logger_type         = std::function<void(const std::string&)>;
  using data_handle_type    = std::function<void(TickerId, const Bar&)>;
  using chain_handle_type   = std::function<void(option_desc&&)>;
  using details_handle_type = std::function<void(const ContractDetails&)>;

public:
  client(int, std::string, int, bool, int, logger_type = logger_type(), const std::string& = "America/New_York");

public:
  bool connect();
  void disconnect();
  bool run();
  void request(const Contract&
    , const std::string&
    , const std::string&
    , const std::string&
    , const std::chrono::system_clock::time_point&);

  void get_chain(const Contract&, const std::string&);
  void get_details(const Contract&);

public:
  int                 id() const { return m_id   ; }
  std::string       host() const { return m_host ; }
  int               port() const { return m_port ; }
  bool        extra_auth() const { return m_extra; }
  std::string   timezone() const { return m_tz   ; }
  bool         connected() const { return m_state != state::idle; }

public:
  const data_handle_type&    data_handle   () const { return m_bar_hdl    ; }
  const chain_handle_type&   chain_handle  () const { return m_chain_hdl  ; }
  const details_handle_type& details_handle() const { return m_details_hdl; }
  void data_handle   (const data_handle_type&    h) { m_bar_hdl     = h; }
  void chain_handle  (const chain_handle_type&   h) { m_chain_hdl   = h; }
  void details_handle(const details_handle_type& h) { m_details_hdl = h; }

// Ewrapper interface
protected:
  void error(int, int, const std::string&) override final;
  void historicalData(TickerId, const Bar&) override final;
  void historicalDataEnd(int, const std::string&, const std::string&) override final;
  void securityDefinitionOptionalParameterEnd(int reqId) override final;
  void contractDetails(int, const ContractDetails&) override final;
  void contractDetailsEnd(int reqId) override final;

  void securityDefinitionOptionalParameter(
      int reqId
    , const std::string& exchange
    , int underlyingConId
    , const std::string& tradingClass
    , const std::string& multiplier
    , const std::set<std::string>& expirations
    , const std::set<double>& strikes) override final;

private:
  data_handle_type    m_bar_hdl    ;
  chain_handle_type   m_chain_hdl  ;
  details_handle_type m_details_hdl;

private:
  logger_type m_log   ;
  state       m_state ;
  int         m_id    ;
  std::string m_host  ;
  int         m_port  ;
  bool        m_extra ;
  std::string m_tz    ; 

private:

private:
  details::reader m_rd;
};

#endif // TWS11_SRC_CLIENT_H