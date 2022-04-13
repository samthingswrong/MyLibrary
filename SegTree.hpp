#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>

using std::vector;

const int inf = 1e9 + 7;

template <typename T, template <typename> typename Op>
constexpr T neutral(0);

template <typename T>
class Addition {
 public:
  T operator()(const T& a, const T& b) {
    return a + b;
  }
};

template <typename T>
class Minimum {
 public:
  T operator()(const T& a, const T& b) {
    return (a < b ? a : b);
  }
};

template <typename T>
class Maximum {
 public:
  T operator()(const T& a, const T& b) {
    return (a > b ? a : b);
  }
};

template <>
constexpr int neutral<int, Minimum> = inf;

template <>
constexpr int neutral<int, Maximum> = -inf;

template <>
constexpr int neutral<int, Addition> = 0;

template <typename T = int, template <typename> typename Sum = Addition,
          template <typename> typename Min = Minimum>
class SegTree {
 public:
  SegTree(const vector<T>& a) {
    size_t sz = static_cast<size_t>(std::pow(2, ceil(std::log2(a.size()))));
    v_.assign(2 * sz, {e_sum_, e_min_, 0, 0});
    for (size_t i = sz; i < sz + a.size(); ++i) {
      v_[i].sum = a[i % sz];
      v_[i].min = a[i % sz];
    }

    for (size_t i = sz; i < 2 * sz; ++i) {
      v_[i].left = i % sz;
      v_[i].right = i % sz;
    }

    for (size_t i = sz - 1; i >= 1; --i) {
      v_[i].sum = sum_(v_[2 * i].sum, v_[2 * i + 1].sum);
      v_[i].min = min_(v_[2 * i].min, v_[2 * i + 1].min);
      v_[i].left = v_[2 * i].left;
      v_[i].right = v_[2 * i + 1].right;
    }
  }

  T GetMin(size_t l, size_t r) {
    if (l < 0 || r > v_.size() / 2 || l > r) {
      throw std::runtime_error("Nice joke with indexes!");
    }

    return GetMin(1, l, r);
  }

  T GetSum(size_t l, size_t r) {
    if (l < 0 || r > v_.size() / 2 || l > r) {
      throw std::runtime_error("Nice joke with indexes!");
    }

    return GetSum(1, l, r);
  }

  void Change(size_t i, const T& val) {
    if (i < 0 || i >= v_.size() / 2) {
      throw std::runtime_error("Nice joke with indexes!");
    }

    size_t node = v_.size() / 2 + i;
    v_[node].sum = val;
    v_[node].min = val;

    while (node > 1) {
      node /= 2;
      v_[node].sum = sum_(v_[2 * node].sum, v_[2 * node + 1].sum);
      v_[node].min = min_(v_[2 * node].min, v_[2 * node + 1].min);
    }
  }

 private:
  T GetMin(size_t node, size_t l, size_t r);
  T GetSum(size_t node, size_t l, size_t r);

  struct Node {
    T sum;
    T min;
    size_t left;
    size_t right;
  };

  T e_sum_ = neutral<T, Sum>;
  T e_min_ = neutral<T, Min>;

  Sum<T> sum_;
  Min<T> min_;

  vector<Node> v_;
};

template <typename T, template <typename> typename Sum,
          template <typename> typename Min>
T SegTree<T, Sum, Min>::GetMin(size_t node, size_t l, size_t r) {
  if (l > v_[node].right || r < v_[node].left) {
    return e_min_;
  }

  if (l <= v_[node].left && r >= v_[node].right) {
    return v_[node].min;
  }

  return min_(GetMin(2 * node, l, r), GetMin(2 * node + 1, l, r));
}

template <typename T, template <typename> typename Sum,
          template <typename> typename Min>
T SegTree<T, Sum, Min>::GetSum(size_t node, size_t l, size_t r) {
  if (l > v_[node].right || r < v_[node].left) {
    return e_sum_;
  }

  if (l <= v_[node].left && r >= v_[node].right) {
    return v_[node].sum;
  }

  return sum_(GetSum(2 * node, l, r), GetSum(2 * node + 1, l, r));
}
