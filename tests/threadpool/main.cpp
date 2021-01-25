
#include <gtest/gtest.h>

#include "threadpool.h"

TEST(concurrency_threadpool, create_tp) {
  auto nthreads = std::thread::hardware_concurrency() - 1;
  
  {
    threadpool tp(nthreads);
  }
  
  ASSERT_TRUE(true);
}

TEST(concurrency_threadpool, add_job) {

  auto nthreads = std::thread::hardware_concurrency() - 1;
  threadpool tp(nthreads);

  auto func = [](int a, int b) { 
    return a + b; 
  };

  auto fut = tp.enqueue(func, 2, 3);
  auto value = fut.get(); 
  ASSERT_EQ(value, 5);
}

TEST(concurrency_threadpool, add_many_job) {

  auto nthreads = std::thread::hardware_concurrency() - 1;
  threadpool tp(nthreads);

  std::vector<std::future<int>> results;

  for (std::size_t i = 0; i < 1000; i++) {
    results.push_back(tp.enqueue([](int a, int b) { return a + b; }, 2, i));
  }

  int i = 2;
  for(auto& f : results) {
    auto value = f.get();
    ASSERT_EQ(value, i);
    ++i;       
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
