#pragma once
#include <type_traits>

template <typename E>
requires std::is_enum_v<E>
constexpr auto from_enum(E val) {
    return static_cast<std::underlying_type_t<E>>(val);
}

template <typename E>
requires std::is_enum_v<E>
constexpr auto to_enum(std::underlying_type_t<E> val) {
    return static_cast<E>(val);
}

template <typename E>
requires std::is_enum_v<E>
constexpr auto operator|(E lhs, E rhs) {
    return to_enum<E>(from_enum(lhs) | from_enum(rhs));
}

template <typename E>
requires std::is_enum_v<E>
constexpr auto& operator|=(E& lhs, E rhs) {
    lhs = to_enum<E>(from_enum(lhs) | from_enum(rhs));
    return lhs;
}

template <typename E>
requires std::is_enum_v<E>
constexpr auto operator&(E lhs, E rhs) {
    return to_enum<E>(from_enum(lhs) & from_enum(rhs));
}

template <typename E>
requires std::is_enum_v<E>
constexpr auto& operator&=(E& lhs, E rhs) {
    lhs = to_enum<E>(from_enum(lhs) & from_enum(rhs));
    return lhs;
}

template <typename T, size_t SZ>
constexpr size_t array_size(const T(&)[SZ]) {
    return SZ;
}

template <typename T>
requires std::is_array_v<T>
constexpr size_t type_array_size() {
    return std::extent_v<T>;
}

#if DEBUG_PRINT
#define dbg(expr) std::cout << expr
#else
#define dbg(expr)
#endif