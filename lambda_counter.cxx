
#include <iostream>

template <size_t I>
struct flag {
  // Note: Id isn't really neccessary, and this works even when we always supply
  // the same type
  template <typename Id>
  friend constexpr auto adl(flag<I>, Id);
};

template <size_t I>
struct set {
  static const constexpr size_t val = I;
  template <typename Id>
  friend constexpr auto adl(flag<I>, Id) {
    return size_t(I);
  }
};

template <typename X, size_t I, typename T = flag<I>>
constexpr auto get_current(float) {
  return I;
}

template <typename X, size_t I, typename T = flag<I>,
          typename = decltype(adl(T{}, X{}))>
constexpr auto get_current(int) {
  return get_current<X, I + 1>(0);
}

template <auto X = []() {}>
constexpr auto lambda_counter() {
  return set<get_current<decltype(X), 0>(0)>::val;
}

// template struct set<1>;
// template struct set<2>;

struct S;
int main() {
  // decltype(adl(flag<1>{})) v2;
  // std::cout << wrap<0>::get_current() << std::endl;
  std::cout << lambda_counter() << std::endl;
  std::cout << lambda_counter() << std::endl;
  std::cout << lambda_counter() << std::endl;
  std::cout << lambda_counter() << std::endl;
  std::cout << lambda_counter() << std::endl;

  return 0;
}
