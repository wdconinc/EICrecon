#pragma once
#include <vector>
namespace ROOT {
namespace VecOps {
  template <typename T> class RVec : public std::vector<T> {
  public:
    using std::vector<T>::vector;
    RVec() = default;
    template <typename U> RVec(const RVec<U>& other) : std::vector<T>(other.begin(), other.end()) {}
  };
} // namespace VecOps
} // namespace ROOT
