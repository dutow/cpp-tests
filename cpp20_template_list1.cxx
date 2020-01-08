
#include <cstddef>
#include <type_traits>

template<typename T>
struct list_head {
    template<typename Id>
    friend constexpr auto next_link(list_head*, Id) noexcept;
};

template<typename T, typename Id, typename R = decltype(next_link(static_cast<T>(nullptr), Id{}))>
constexpr R list_next_or_this(int, T, Id) { return nullptr; };

template<typename T, typename Id>
constexpr T list_next_or_this(float, T, Id) { return nullptr; }

template<typename T, typename Id>
constexpr T list_last_or_this(float, T, Id);

template<typename T, typename Id, typename R = decltype(next_link(static_cast<T>(nullptr), Id{}))>
constexpr auto list_last_or_this(int, T, Id) -> decltype(list_last_or_this(0, R{}, Id{})) {
  return nullptr; }

template<typename L, typename Id>
constexpr decltype(list_last_or_this(0, static_cast<list_head<L>*>(nullptr), Id{})) list_last(L, Id) { return nullptr; }


template<typename L, typename T>
struct link {
  public:
    using prev_type = decltype(list_last(L{}, [](){}));

    friend constexpr prev_type prev_link(T*) noexcept {
      return static_cast<prev_type>(nullptr);
    }

    template<typename Id>
    friend constexpr auto next_link(T*, Id) noexcept;

    template<typename Id>
    friend constexpr auto next_link(prev_type, Id) noexcept {
      return static_cast<T*>(nullptr);
    }
};

struct A_LIST{};

using head = list_head<A_LIST>;

struct S1: public link<A_LIST, S1>{};

auto v1_1 = prev_link(static_cast<S1*>(nullptr));
static_assert(std::is_same_v<decltype(v1_1), head*>);

auto v1_2 = next_link(static_cast<head*>(nullptr), [](){});
static_assert(std::is_same_v<decltype(v1_2), S1*>);

auto v1_3 = list_next_or_this(0, static_cast<head*>(nullptr), [](){});
static_assert(std::is_same_v<decltype(v1_3), S1*>);


auto v1_4 = list_last(A_LIST{}, [](){});
static_assert(std::is_same_v<decltype(v1_4), S1*>);

struct S2: public link<A_LIST, S2>{};
auto v2_1 = prev_link(static_cast<S2*>(nullptr));
static_assert(std::is_same_v<decltype(v2_1), S1*>);

auto v2_2 = next_link(static_cast<S1*>(nullptr), [](){});
static_assert(std::is_same_v<decltype(v2_2), S2*>);

auto v2_4 = list_last(A_LIST{}, [](){});
static_assert(std::is_same_v<decltype(v2_4), S2*>);

template<auto X=[](){}>
auto linkup() {
struct S: public link<A_LIST, S>{};
return S{};
}

void wrap1(){
linkup();
}

auto v3_4 = list_last(A_LIST{}, [](){});
static_assert(!std::is_same_v<decltype(v3_4), S2*>);

