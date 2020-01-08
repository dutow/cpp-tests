
#include <cstdint>
#include <iostream>
#include <memory>

// version of the gist at:
// https://gist.github.com/graphitemaster/494f21190bb2c63c5516 original version
// by graphitemaster
//

template <size_t maxalign>
struct unioner {
  template <typename T0, typename T1, size_t O>
  union U_inner {
    struct {
      char pad[O];  // offset

      T1 m[sizeof(T0) / (sizeof(T1) + 1)];  // instance of type of member
    } data;
    U_inner<T0, T1, O + 1> other;
  };

  template <typename T0, typename T1>
  union U_inner<T0, T1, 0> {
    struct {
      T1 m[sizeof(T0) / (sizeof(T1) + 1)];  // instance of type of member
    } data;
    U_inner<T0, T1, 1> other;
  };

  template <typename T0, typename T1>
  union U_inner<T0, T1, maxalign> {};
};

template <typename T0, typename T1, typename T2>
struct offset_of_impl {
  using inner_t = typename unioner<alignof(T1)>::template U_inner<T0, T1, 0>;
  union U {
    char c;
    inner_t m;
    T0 object;
    constexpr U() : c(0) {}  // make c the active member
  };
  static constexpr U u = {};

  static constexpr const T1* addr_helper(const T1* base, const T1* target) {
    auto addr = base;
    while (addr < target) {
      addr++;
    }
    return addr;
  }
  static constexpr ptrdiff_t addr_diff(const T1* base, const T1* target) {
    return (target - base) * sizeof(T1);
  }

  template <size_t off, typename TT>
  static constexpr std::ptrdiff_t offset2(T1 T2::*member, TT& union_part) {
    const auto addr_target =
        std::addressof(offset_of_impl<T0, T1, T2>::u.object.*member);
    const auto addr_base = std::addressof(union_part.data.m[0]);
    const auto addr = addr_helper(addr_base, addr_target);

    // != will never return true... but < seems to work?
    if (addr < addr_target) {
      if constexpr (off + 1 < alignof(T1)) {
        return offset2<off + 1>(member, union_part.other);
      } else {
        throw 1;  // shouldn't happen
      }
    }
    return (addr - addr_base) * sizeof(T1) + off;
  }

  static constexpr std::ptrdiff_t offset(T1 T2::*member) {
    // The following avoids use of reinterpret_cast, so is constexpr.
    // The subtraction gives the correct offset because the union layout rules
    // guarantee that all union members have the same starting address. On the
    // other hand, it will break if object.*member is not aligned.
    const auto addr_target =
        std::addressof(offset_of_impl<T0, T1, T2>::u.object.*member);
    const auto addr_base =
        (std::addressof(offset_of_impl<T0, T1, T2>::u.m.data.m[0]));
    const auto addr = addr_helper(addr_base, addr_target);

    return offset2<0>(member, offset_of_impl<T0, T1, T2>::u.m);

    if (addr != addr_target) {
      return 0;
    }

    return (addr - addr_base) * sizeof(T1);
  }
};

template <typename T0, typename T1, typename T2>
constexpr typename offset_of_impl<T0, T1, T2>::U offset_of_impl<T0, T1, T2>::u;

template <typename T0, typename T1, typename T2>
inline constexpr std::ptrdiff_t offset_of(T1 T2::*member, T0* = nullptr) {
  return offset_of_impl<T0, T1, T2>::offset(member);
}

template <typename T0, T0::*member T1>
void f() {}

struct s {
  float a;
  char b;
  int c;
};

#pragma pack(push, 1)
struct s2 {
  float a;
  char b;
  int c;
  double d;
  char e;
};
#pragma pack(pop)

struct a {
  int i;
  int j;
};
struct b {
  int i;
  int k;
};
struct ab : public a, public b {};

int main() {
  constexpr size_t s_b = offset_of<s>(&s::b);
  constexpr size_t s_c = offset_of<s>(&s::c);
  // compilation error with both gcc & clang
  constexpr size_t s2_c = offset_of<s2>(&s2::c);
  std::cout << s_b << std::endl;
  std::cout << s_c << std::endl;
  std::cout << s2_c << std::endl;
  std::cout << offset_of<s2>(&s2::e) << std::endl;
  std::cout << alignof(&s2::e) << std::endl;

  // these only work with gcc, not clang
  // also generates a warning
  // std::cout << offsetof(ab, a::i) << std::endl;
  // std::cout << offsetof(ab, b::i) << std::endl;
  auto ai = &ab::a::i;
  auto bi = &ab::b::i;

  ab v;
  v.*ai = 11;
  v.*bi = 22;

#define DBG_PRINT(s) std::cout << #s << " = " << (s) << std::endl;

  std::cout << ((a&)v).i << " " << ((b&)v).i << std::endl;
  DBG_PRINT(offset_of<ab>(&ab::b::i));
  DBG_PRINT(offset_of<ab>(&ab::a::i));

  DBG_PRINT(offset_of<ab>(&ab::k));
  // incorrect result
  DBG_PRINT(offset_of<ab>(&ab::b::k));
  // doesn't work with clang, correct result with gcc
  // DBG_PRINT((offsetof(ab, b::k)));
  DBG_PRINT(offset_of<ab>(&ab::k));
  DBG_PRINT((offsetof(ab, k)));

  return 0;
}
