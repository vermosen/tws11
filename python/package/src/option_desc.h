#pragma once
#ifndef TWS11_SRC_OPTION_DESC_H
#define TWS11_SRC_OPTION_DESC_H

#include <string>

struct option_desc {
  int         m_underlying;
  std::string m_category  ;
  std::string m_multiplier;
  double      m_strike    ;
  std::string m_exp       ;
  std::string m_exchange  ;
};

#endif // TWS11_SRC_OPTION_DESC_H