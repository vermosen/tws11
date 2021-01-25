
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <condition_variable>

class client;

namespace details {

  class job {};

  // for example:
  // https://codereview.stackexchange.com/questions/221617/thread-pool-c-implementation
  class threadpool {
  public:
    threadpool(std::size_t sz);
    threadpool(const threadpool&) = delete;
    threadpool &operator=(const threadpool&) = delete;

  private:
    std::size_t m_nthreads; // number of threads in the pool
    std::vector<std::thread> m_pool; //the actual thread pool
    std::queue<std::shared_ptr<job>> m_queue;
    std::condition_variable m_cv;// used to notify threads about available jobs
    std::mutex m_mutex; // used to push/pop jobs to/from the queue
  };
};

class batch {
  batch(client& cl);

private:
  client& m_cl;
};

class mtclient {};
