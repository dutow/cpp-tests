
// Blogpost: inheriting from forward declared types

#include <iostream>

template <typename T>
struct s_t : public T {
  int get_a() const { return this->a; }
};

struct TT;

// struct s : public TT;
using ss = s_t<TT>;

// ss xx;

struct TT {
  int a = 0;
};

int main() {
  ss x;
  x.a = 0;
  std::cout << x.get_a() << std::endl;
  return x.a;
}
