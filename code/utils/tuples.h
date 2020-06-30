#pragma once

#include <tuple>
#include <utility>

namespace util {
namespace tuples {

template <std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type for_each(const std::tuple<Tp...>&, FuncT)
{
}
template <std::size_t I = 0, typename FuncT, typename... Tp>
	inline typename std::enable_if < I<sizeof...(Tp), void>::type for_each(const std::tuple<Tp...>& t, FuncT f)
{
	f(std::get<I>(t));
	for_each<I + 1, FuncT, Tp...>(t, f);
}

template <std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type for_each(std::tuple<Tp...>&&, FuncT)
{
}
template <std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if < I<sizeof...(Tp), void>::type for_each(std::tuple<Tp...>&& t, FuncT f)
{
	f(std::move(std::get<I>(t)));
	for_each<I + 1, FuncT, Tp...>(std::move(t), f);
}

} // namespace tuples
} // namespace util
