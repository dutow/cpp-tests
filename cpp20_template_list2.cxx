
#include <cstddef>
#include <iostream>
#include <type_traits>

template <size_t I>
constexpr size_t pow2 = pow2<I - 1> * 2;

template <>
constexpr size_t pow2<0> = 1;

template <>
constexpr size_t pow2<64> = (pow2<63> - 1) * 2 + 1;

// instantiate it so all's done
template size_t pow2<63>;

static_assert(pow2<3> == 8);

template <typename T>
struct list_head {
  template <typename Id>
  friend constexpr auto next_link(list_head*, Id) noexcept;

  template <long long int idx>
  struct index {
    template <typename Id>
    friend constexpr auto get_nth(index<idx>, Id) noexcept;
  };

  template <typename Id>
  friend constexpr auto get_nth(index<-1>, Id) noexcept {
    return static_cast<list_head<T>*>(nullptr);
  }
};

template <typename T, typename Id,
          typename R = decltype(next_link(static_cast<T>(nullptr), Id{}))>
constexpr R list_next_or_this(int, T, Id) {
  return nullptr;
};

template <typename T, typename Id>
constexpr T list_next_or_this(float, T, Id) {
  return nullptr;
}

template <typename T, typename Id>
constexpr T list_last_or_this(float, T, Id);

template <typename T, typename Id,
          typename R = decltype(next_link(static_cast<T>(nullptr), Id{}))>
constexpr auto list_last_or_this(int, T, Id)
    -> decltype(list_last_or_this(0, R{}, Id{})) {
  return nullptr;
}

template <typename L, typename Id>
constexpr decltype(list_last_or_this(0, static_cast<list_head<L>*>(nullptr),
                                     Id{}))
list_last(L, Id) {
  return nullptr;
}

template <typename T, typename Id>
constexpr size_t list_size_or_same(float, T, Id) {
  return 0;
}

template <typename T, typename Id,
          typename R = decltype(next_link(static_cast<T>(nullptr), Id{}))>
constexpr auto list_size_or_same(int, T, Id)
    -> decltype(list_size_or_same(0, R{}, Id{})) {
  return list_size_or_same(0, R{}, Id{}) + 1;
}

template <typename L, typename Id>
constexpr decltype(list_size_or_same(0, static_cast<list_head<L>*>(nullptr),
                                     Id{}))
list_size(L, Id) {
  return list_size_or_same(0, static_cast<list_head<L>*>(nullptr), Id{});
}

/////////////////////////////////////////////

template <typename L, size_t N, typename Id,
          typename R = decltype(
              get_nth(typename list_head<L>::template index<N>{}, Id{}))>
constexpr bool nth_exists(int) {
  return true;
};

template <typename L, size_t N, typename Id>
constexpr bool nth_exists(float) {
  return false;
};

template <typename L, size_t curr_idx, size_t pow_level, typename Id>
constexpr size_t log_up_check(float) {
  return pow_level;
}

template <
    typename L, size_t curr_idx, size_t pow_level, typename Id,
    typename = std::enable_if_t<nth_exists<L, pow2<pow_level + 1>, Id>(0)>>
constexpr size_t log_up_check(int) {
  return log_up_check<L, pow2<pow_level + 1>, pow_level + 1, Id>(0);
}

template <size_t start, size_t end, size_t next_pow>
constexpr size_t log_in_middle() {
  // this links to the first element of the next, indexing starts with 0!
  return start + pow2<next_pow>;
}

template <typename L, size_t start, size_t end, size_t next_pow, typename Id>
constexpr size_t log_in_check(float);

template <typename L, size_t start, size_t end, size_t next_pow, typename Id,
          typename = std::enable_if_t<
              nth_exists<L, log_in_middle<start, end, next_pow>(), Id>(0)>>
constexpr size_t log_in_check(int) {
  if constexpr (next_pow == 0) {
    // it's this or the next
    if constexpr (nth_exists<L, start + 1, Id>(0)) {
      return start + 1;
    }
    return start;
  } else {
    // the middle exists -> it should be bigger
    return log_in_check<L, log_in_middle<start, end, next_pow>(), end,
                        next_pow - 1, Id>(0);
  }
}

template <typename L, size_t start, size_t end, size_t next_pow, typename Id>
constexpr size_t log_in_check(float) {
  // start should exist, otherwise we wouldn't be here
  if constexpr (next_pow == 0) {
    // it's this or the next
    if constexpr (nth_exists<L, start + 1, Id>(0)) {
      return start + 1;
    }
    return start;
  } else {
    // the middle doesn't exists -> it should be smaller
    return log_in_check<L, start, log_in_middle<start, end, next_pow>(),
                        next_pow - 1, Id>(0);
  }
}

template <typename L, typename Id>
constexpr size_t log_size(L, Id) {
  constexpr size_t pow_level_before = log_up_check<L, 0, 0, Id>(0);
  if constexpr (pow_level_before == 0) {
    // it's either 0 or 1
    if constexpr (nth_exists<L, 1, Id>(0)) {
      return 2;
    }
    if constexpr (nth_exists<L, 0, Id>(0)) {
      return 1;
    }
    return 0;
  } else if constexpr (pow_level_before == 64) {
    return pow2<pow_level_before>;
  } else {
    const size_t pow_level_after = pow_level_before + 1;
    // +1: this returns the last accessibe index ; indexing starts at 0
    return log_in_check<L, pow2<pow_level_before>, pow2<pow_level_after>,
                        pow_level_before - 1, Id>(0) +
           1;
  }
}

/////////////////////////////////////////////

template <typename L, typename T>
struct link {
 public:
  static constexpr const long long int idx = log_size(L{}, []() {});

  template <typename Id>
  friend constexpr auto get_nth(list_head<L>::template index<idx>,
                                Id) noexcept {
    return static_cast<T*>(nullptr);
  }

  using prev_type = decltype(
      get_nth(typename list_head<L>::template index<idx - 1>{}, []() {}));

  friend constexpr prev_type prev_link(T*) noexcept {
    return static_cast<prev_type>(nullptr);
  }

  template <typename Id>
  friend constexpr auto next_link(T*, Id) noexcept;

  template <typename Id>
  friend constexpr auto next_link(prev_type, Id) noexcept {
    return static_cast<T*>(nullptr);
  }
};

struct A_LIST {};

using head = list_head<A_LIST>;

static_assert(log_size(A_LIST{}, []() {}) == 0);
void f_start() {
  constexpr size_t s = log_size(A_LIST{}, []() {});
  // uh, it's not the same!
  std::cout << s << std::endl;
  std::cout << log_size(A_LIST{}, []() {}) << std::endl;
}

static_assert(list_size(A_LIST{}, []() {}) == 0);

struct S1 : public link<A_LIST, S1> {};

void f_2() {
  struct S {};
  constexpr size_t pow_level_before = log_up_check<A_LIST, 0, 0, S>(0);
  std::cout << pow_level_before << std::endl;
  constexpr size_t s = log_size(A_LIST{}, []() {});
  // uh, it's not the same!
  std::cout << s << std::endl;
  std::cout << log_size(A_LIST{}, []() {}) << std::endl;
}

static_assert(S1::idx == 0);

auto v1_1 = prev_link(static_cast<S1*>(nullptr));
static_assert(std::is_same_v<decltype(v1_1), head*>);

auto v1_2 = next_link(static_cast<head*>(nullptr), []() {});
static_assert(std::is_same_v<decltype(v1_2), S1*>);

auto v1_3 = list_next_or_this(0, static_cast<head*>(nullptr), []() {});
static_assert(std::is_same_v<decltype(v1_3), S1*>);

auto v1_4 = list_last(A_LIST{}, []() {});
static_assert(std::is_same_v<decltype(v1_4), S1*>);

struct S2 : public link<A_LIST, S2> {};
static_assert(S2::idx == 1);

void f_3() {
  struct S {};
  constexpr size_t pow_level_before = log_up_check<A_LIST, 0, 0, S>(0);
  std::cout << pow_level_before << std::endl;
  constexpr size_t s = log_size(A_LIST{}, []() {});
  // uh, it's not the same!
  std::cout << s << std::endl;
  std::cout << log_size(A_LIST{}, []() {}) << std::endl;
}

auto v2_1 = prev_link(static_cast<S2*>(nullptr));
static_assert(std::is_same_v<decltype(v2_1), S1*>);

auto v2_2 = next_link(static_cast<S1*>(nullptr), []() {});
static_assert(std::is_same_v<decltype(v2_2), S2*>);

auto v2_4 = list_last(A_LIST{}, []() {});
static_assert(std::is_same_v<decltype(v2_4), S2*>);

template <auto X = []() {}>
auto linkup() {
  struct S : public link<A_LIST, S> {};
  static_assert(S::idx == 2);
  static_assert(
      std::is_same_v<decltype(get_nth(head::index<2>{}, []() {})), S*>);
  return S{};
}

// static_assert(log_size(A_LIST{}, []() {}) == 3);

void wrap1() { linkup(); }

void f_4() {
  struct S {};
  constexpr size_t pow_level_before = log_up_check<A_LIST, 0, 0, S>(0);
  std::cout << pow_level_before << std::endl;
  constexpr size_t s = log_size(A_LIST{}, []() {});
  // uh, it's not the same!
  std::cout << s << std::endl;
  std::cout << log_size(A_LIST{}, []() {}) << std::endl;
}

struct S4 : public link<A_LIST, S4> {};

void f_5() {
  struct S {};
  constexpr size_t pow_level_before = log_up_check<A_LIST, 0, 0, S>(0);
  std::cout << pow_level_before << std::endl;
  constexpr size_t s = log_size(A_LIST{}, []() {});
  // uh, it's not the same!
  std::cout << s << std::endl;
  std::cout << log_size(A_LIST{}, []() {}) << std::endl;
}

auto v3_4 = list_last(A_LIST{}, []() {});
static_assert(!std::is_same_v<decltype(v3_4), S2*>);

static_assert(list_size(A_LIST{}, []() {}) == 4);

template <auto X = []() {}>
struct MASS_S : public link<A_LIST, MASS_S<X>> {};

#define LINK10  \
  { MASS_S x; } \
  { MASS_S x; } \
  { MASS_S x; } \
  { MASS_S x; } \
  { MASS_S x; } \
  { MASS_S x; } \
  { MASS_S x; } \
  { MASS_S x; } \
  { MASS_S x; } \
  { MASS_S x; }

auto link10() {
  { MASS_S x; }
  static_assert(log_size(A_LIST{}, []() {}) == 5);
  { MASS_S x; }
  static_assert(log_size(A_LIST{}, []() {}) == 6);
  { MASS_S x; }
  static_assert(log_size(A_LIST{}, []() {}) == 7);
  { MASS_S x; }
  static_assert(log_size(A_LIST{}, []() {}) == 8);
  { MASS_S x; }
  static_assert(log_size(A_LIST{}, []() {}) == 9);
  { MASS_S x; }
  static_assert(log_size(A_LIST{}, []() {}) == 10);
  { MASS_S x; }
  static_assert(log_size(A_LIST{}, []() {}) == 11);
  { MASS_S x; }
  static_assert(log_size(A_LIST{}, []() {}) == 12);
  { MASS_S x; }
  static_assert(log_size(A_LIST{}, []() {}) == 13);
  { MASS_S x; }
  static_assert(log_size(A_LIST{}, []() {}) == 14);
}

static_assert(nth_exists<A_LIST, 8, decltype([]() {})>(0));
static_assert(nth_exists<A_LIST, 13, decltype([]() {})>(0));
static_assert(!nth_exists<A_LIST, 14, decltype([]() {})>(0));
static_assert(!nth_exists<A_LIST, 16, decltype([]() {})>(0));
// static_assert(log_size(A_LIST{}, []() {}) == 4);

void f() {
  struct S {};
  constexpr size_t pow_level_before = log_up_check<A_LIST, 0, 0, S>(0);
  std::cout << pow_level_before << std::endl;
  constexpr size_t s = log_size(A_LIST{}, []() {});
  // uh, it's not the same!
  std::cout << s << std::endl;
  std::cout << log_size(A_LIST{}, []() {}) << std::endl;
}

auto linkmany() {
  LINK10;
  LINK10;
  LINK10;
  LINK10;
  LINK10;
  LINK10;
  LINK10;
  LINK10;
  LINK10;
  LINK10;
}

void g() {
  std::cout << log_size(A_LIST{}, []() {}) << std::endl;
}

int main() {
  f_start();
  std::cout << "====" << std::endl;
  f_2();
  std::cout << "====" << std::endl;
  f_3();
  std::cout << "====" << std::endl;
  f_4();
  std::cout << "====" << std::endl;
  f_5();
  std::cout << "====" << std::endl;
  f();
  std::cout << "====" << std::endl;
  g();
  return 0;
}
