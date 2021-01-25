#include <string>

struct option_desc {
  int         m_underlying;
  std::string m_category  ;
  std::string m_multiplier;
  double      m_strike    ;
  std::string m_exp       ;
  std::string m_exchange  ;
};