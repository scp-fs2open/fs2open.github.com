#pragma once

#include "globalincs/scp_defines.h"

#include <optional>

template <typename T> struct is_optional : std::false_type {};
template<typename T> struct is_optional<std::optional<T>> : std::true_type {};
template<> 			 struct is_optional<std::nullopt_t> : std::true_type {};

template<typename T>
inline constexpr bool is_optional_v = is_optional<T>::value;

template <typename T> struct is_tuple : std::false_type {};
template <typename... U> struct is_tuple<std::tuple <U...>> : std::true_type {};

template<typename T>
inline constexpr bool is_tuple_v = is_tuple<T>::value;

template<typename T, typename F>
constexpr auto has_member_impl(F&& f) -> decltype(f(std::declval<T>()), true) {
	SCP_UNUSED(f);
	return true;
}

template<typename>
constexpr bool has_member_impl(...) { return false; }

#define has_member(T, member) has_member_impl<T>( [](auto&& obj)->decltype(obj.member){ return obj.member; } )

template<typename T, typename Enable = void>
struct is_dereferenceable_pointer : std::false_type {};

template<typename T>
struct is_dereferenceable_pointer<T, typename std::enable_if_t<std::is_same_v<typename std::decay_t<T>, std::unique_ptr<typename std::decay_t<T>::element_type>>>> : std::true_type {};

template<typename T>
struct is_dereferenceable_pointer<T, typename std::enable_if_t<std::is_same_v<typename std::decay_t<T>, std::shared_ptr<typename std::decay_t<T>::element_type>>>> : std::true_type {};

template<typename T>
struct is_dereferenceable_pointer<T, typename std::enable_if_t<std::is_same_v<typename std::decay_t<T>, std::weak_ptr<typename std::decay_t<T>::element_type>>>> : std::true_type {};

template<typename T>
struct is_dereferenceable_pointer<T, typename std::enable_if_t<std::is_pointer_v<typename std::decay_t<T>>>> : std::true_type {};

template<typename T>
inline constexpr bool is_dereferenceable_pointer_v = is_dereferenceable_pointer<T>::value;

template<class T, template<class...> class U>
inline constexpr bool is_instance_of_v = std::false_type{};

template<template<class...> class U, class... Vs>
inline constexpr bool is_instance_of_v<U<Vs...>,U> = std::true_type{};


