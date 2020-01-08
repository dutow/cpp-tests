
#include <cstdint>
#include <iostream>
#include <memory>

// version of the gist at:
// https://gist.github.com/graphitemaster/494f21190bb2c63c5516 original version
// by graphitemaster
//

template <typename TSTRUCT, typename TMEMBER, typename TDUMMY, size_t N = 0>
struct offset_of_impl {
  union U {
    char c;
#pragma pack(push, 1)
    struct {
      char pad[N];
      TMEMBER m[sizeof(TSTRUCT) / sizeof(TMEMBER) + 1];
    };
#pragma pack(pop)
    TSTRUCT object;
    constexpr U() : c(0) {}  // make c the active member
  };

  // Has to be outide of offset: can't take address of a local
  // Has to be static: this is not a constant expression
  static constexpr U u = {};

  static constexpr const TMEMBER* addr_helper(const TMEMBER* base,
                                              const TMEMBER* target) {
    auto addr = base;
    while (addr < target) {
      addr++;
    }
    return addr;
  }

  static constexpr std::ptrdiff_t offset(TMEMBER TDUMMY::*member) {
    // using addressof which ignores operator& overloading
    constexpr const auto addr_base =
        std::addressof(offset_of_impl<TSTRUCT, TMEMBER, TDUMMY, N>::u.m[0]);
    // .*member is not a constant expressino
    const auto addr_target = std::addressof(
        offset_of_impl<TSTRUCT, TMEMBER, TDUMMY, N>::u.object.*member);
    const auto addr_found = addr_helper(addr_base, addr_target);

    if (addr_found > addr_target) {
      if constexpr (N < sizeof(TMEMBER)) {
        return offset_of_impl<TSTRUCT, TMEMBER, TDUMMY, N + 1>::offset(member);
      } else {
        throw 1;
      }
    }

    return (addr_found - addr_base) * sizeof(TMEMBER) + N;
  }
};

template <typename TSTRUCT, typename TMEMBER, typename TDUMMY, size_t N>
constexpr typename offset_of_impl<TSTRUCT, TMEMBER, TDUMMY, N>::U
    offset_of_impl<TSTRUCT, TMEMBER, TDUMMY, N>::u;

template <typename TSTRUCT, typename TMEMBER, typename TDUMMY>
inline constexpr std::ptrdiff_t offset_of(TMEMBER TDUMMY::*member,
                                          TSTRUCT* = nullptr) {
  return offset_of_impl<TSTRUCT, TMEMBER, TDUMMY>::offset(member);
}

struct s {
  float a;
  char b;
  int c;
  char d;
};

#pragma pack(push, 1)
struct s_packed {
  float a;
  char b;
  int c;
  double d;
  char e;
};
#pragma pack(pop)

struct s_oversized {
  int f1;
  s f2;
  int f3;
  s f4;
  char f5;
};

struct a {
  int i;
  int j;
};
struct b {
  int i;
  int k;
};
struct ab : public a, public b {};

#define DBG_PRINT(s) std::cout << #s << " = " << (s) << std::endl;

int main() {
  constexpr size_t s_b = offset_of<s>(&s::b);
  DBG_PRINT(s_b);
  static_assert(s_b == 4);
  constexpr size_t s_c = offset_of<s>(&s::c);
  DBG_PRINT(s_c);
  static_assert(s_c == 8);
  // pragma pack, unaligned
  constexpr size_t packed_c = offset_of<s_packed>(&s_packed::c);
  DBG_PRINT(packed_c);
  static_assert(packed_c == 5);

  // sizeof > alignof
  static_assert(sizeof(s) > 8);
  static_assert(alignof(s) < 8);
  constexpr size_t large_f2 = offset_of<s_oversized>(&s_oversized::f2);
  DBG_PRINT(large_f2);
  static_assert(large_f2 == 4);
  constexpr size_t large_f4 = offset_of<s_oversized>(&s_oversized::f4);
  DBG_PRINT(large_f4);
  static_assert(large_f4 == 8 + sizeof(s));

  constexpr size_t multi_b_i = offset_of<ab>(&ab::b::i);
  static_assert(multi_b_i == 8);
  DBG_PRINT(multi_b_i);
  constexpr size_t multi_a_i = offset_of<ab>(&ab::a::i);
  static_assert(multi_a_i == 0);
  DBG_PRINT(multi_a_i);

  constexpr size_t multi_k = offset_of<ab>(&ab::k);
  DBG_PRINT(multi_k);
  constexpr size_t multi_b_k = offset_of<ab>(&ab::b::k);
  DBG_PRINT(multi_b_k);

  return 0;
}
