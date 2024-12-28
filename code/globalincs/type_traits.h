#pragma once

#include <optional>
#include <tl/optional.hpp>

template <typename T> struct is_optional : std::false_type {};
template<typename T> struct is_optional<std::optional<T>> : std::true_type {};
template<> 			 struct is_optional<std::nullopt_t> : std::true_type {};
template<typename T> struct is_optional<tl::optional<T>> : std::true_type {};
template<> 			 struct is_optional<tl::nullopt_t> : std::true_type {};

template<typename T>
inline constexpr bool is_optional_v = is_optional<T>::value;

template <typename T> struct is_tuple : std::false_type {};
template <typename... U> struct is_tuple<std::tuple <U...>> : std::true_type {};

template<typename T>
inline constexpr bool is_tuple_v = is_tuple<T>::value;