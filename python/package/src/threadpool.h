#pragma once
#ifndef TWS11_SRC_THREADPOOL_H
#define TWS11_SRC_THREADPOOL_H

#include <mutex>
#include <queue>
#include <future>
#include <thread>
#include <functional>

class threadpool {
public:
  class job_base {
  public:
    virtual ~job_base() {}
    virtual void execute() = 0;
  };

  template <typename RetType>
  class job : public job_base {
  public:
    job(std::packaged_task<RetType()>&& f);
    void execute() override final;

  private:
    std::packaged_task<RetType()> m_f;
  };

private:
  using queue_type = std::queue<std::unique_ptr<job_base>>;

public:
  threadpool() = delete;
  threadpool(std::size_t sz);
  ~threadpool();

public:
  template <typename Func, typename ... Args>
  std::future<typename std::result_of<Func(Args...)>::type>
  enqueue(Func&& f, Args&&... args);

private:
  std::vector<std::thread> m_workers  ;
  queue_type               m_queue    ;
  std::mutex               m_mutex    ;
  std::condition_variable  m_cv       ;
  bool                     m_terminate;
};

template <typename RetType>
inline threadpool::job<RetType>::job(std::packaged_task<RetType()>&& f) 
  : m_f(std::move(f)) {}

template <typename RetType>
inline void threadpool::job<RetType>::execute() { m_f(); }

inline threadpool::~threadpool() {
  
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_terminate = true;
  }

  m_cv.notify_all();

  for(std::thread& t : m_workers) {          // join all threads.
    t.join();
  }

  m_workers.clear();
}

inline threadpool::threadpool(std::size_t sz) 
  : m_terminate(false) {
  
  for (std::size_t i = 0; i < sz; i++) {

    m_workers.push_back(std::thread([this]() -> void {
      
      while(true) {

        std::unique_ptr<job_base> j;

        {
          std::unique_lock<std::mutex> lock(m_mutex);
          m_cv.wait(lock, [this] {
            return m_terminate || !m_queue.empty();
          });                                           // wait until queue not empty or m_terminate == true */

          if (m_terminate && m_queue.empty()) {
            return;
          }

          j = std::move(m_queue.front());
          m_queue.pop();
        }

        j->execute();
      }
    }));
  }
}

template <typename Func, typename ... Args>
inline std::future<typename std::result_of<Func(Args...)>::type> 
threadpool::enqueue(Func&& f, Args&&... args) {
  
  using return_type = typename std::result_of<Func(Args...)>::type;

  std::packaged_task<return_type()> task(std::bind(f, args...));
  std::future<int> fut = task.get_future();
  std::unique_ptr<job_base> j(new job<return_type>(std::move(task)));

  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_queue.push(std::move(j));
  }

  m_cv.notify_one();
  return fut;
}

#endif // TWS11_SRC_THREADPOOL_H