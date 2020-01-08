
#include <cstddef>

template <typename T, typename L>
struct field {};

struct s {
  int i;
  field<s, decltype([](auto s) { return offsetof(decltype(s), a); })> a;
};
