#include "mtclient.h"

#include "client.h"

namespace details {
  threadpool::threadpool(std::size_t sz) 
  : m_pool(sz) {
    for (auto& t : m_pool) {
      t.detach();
    }
  }
}

batch::batch(client& cl) : m_cl(cl) {}