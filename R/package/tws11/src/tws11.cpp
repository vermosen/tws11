#include <cpp11.hpp>

#include <Eigen/Dense>

using namespace cpp11;
namespace writable = cpp11::writable;

[[cpp11::register]]
double mean_cpp(doubles x) {
  int n = x.size();
  double total = 0;
  for(double value : x) {
    total += value;
  }
  return total / n;
}

/* [[cpp11::register]]
double test_eigen(Eigen::VectorXd vec) {
  return vec.mean();
} */