
#include <iostream>

template<typename T2, size_t = sizeof(T2)>
constexpr int f(int) { return 1; }

template<typename T2>
constexpr int f(float) { return 2; }

struct S;

constexpr auto a = f<S>(0); // 2
static_assert(a == 2, "SFINAE failure");

struct S{};

constexpr auto b = f<S>(0);
static_assert(b == 1, "SFINAE success");

int main() {
  std::cout << a << std::endl;
  std::cout << b << std::endl;
  return 0;
}
