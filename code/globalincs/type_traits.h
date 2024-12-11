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

template<typename T, typename Enable = void>
struct is_dereferencable_pointer : std::false_type {};

template<typename T>
struct is_dereferencable_pointer<T, typename std::enable_if_t<std::is_same_v<typename std::decay_t<T>, std::unique_ptr<typename std::decay_t<T>::element_type>>>> : std::true_type {};

template<typename T>
struct is_dereferencable_pointer<T, typename std::enable_if_t<std::is_same_v<typename std::decay_t<T>, std::shared_ptr<typename std::decay_t<T>::element_type>>>> : std::true_type {};

template<typename T>
struct is_dereferencable_pointer<T, typename std::enable_if_t<std::is_same_v<typename std::decay_t<T>, std::weak_ptr<typename std::decay_t<T>::element_type>>>> : std::true_type {};

template<typename T>
struct is_dereferencable_pointer<T, typename std::enable_if_t<std::is_pointer_v<typename std::decay_t<T>>>> : std::true_type {};

template<typename T>
inline constexpr bool is_dereferencable_pointer_v = is_dereferencable_pointer<T>::value;

template<class T, template<class...> class U>
inline constexpr bool is_instance_of_v = std::false_type{};

template<template<class...> class U, class... Vs>
inline constexpr bool is_instance_of_v<U<Vs...>,U> = std::true_type{};


