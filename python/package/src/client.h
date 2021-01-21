
#include <memory>
#include <chrono>
#include <string>
#include <functional>

#include <tws/DefaultEWrapper.h>
#include <tws/EReaderOSSignal.h>
#include <tws/EClientSocket.h>

class client;

class EReader;

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
  };

public:
  using data_type = Bar;
  using logger_type = std::function<void(const std::string&)>;
  using handle_type = std::function<void(TickerId, const data_type&)>;
  using chain_hdl_type = std::function<void()>;

public:
  client(int, std::string, int, bool, int, logger_type = logger_type());

public:
  bool connect();
  void disconnect();
  bool run();
  void request(const Contract&
    , const std::string&
    , const std::string&
    , const std::string&
    , const std::chrono::system_clock::time_point&);

public:
  int           id() const { return m_id   ; }
  std::string host() const { return m_host ; }
  int         port() const { return m_port ; }
  bool  extra_auth() const { return m_extra; }
  bool   connected() const { return m_state != state::idle; }

public:
  handle_type& data_handle() { return m_sink; }
  chain_hdl_type& chain_handle() { return m_chain; }
  
// Ewrapper interface
protected:
  void error(int, int, const std::string&) override final;
  void historicalData(TickerId, const Bar&) override final;
  void historicalDataEnd(int, const std::string&, const std::string&) override final;

private:
  handle_type    m_sink ;
  chain_hdl_type m_chain;

private:
  logger_type m_log   ;
  state       m_state ;
  int         m_id    ;
  std::string m_host  ;
  int         m_port  ;
  bool        m_extra ;

private:
  details::reader m_rd;
};