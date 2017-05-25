#pragma once
/*! \file Iterators on FS2 classes. */
#include <iterator>
#include <type_traits>
#include "globalincs/linklist.h"

namespace fso {
namespace fred {

template<typename T>
struct iterator : public std::iterator<std::bidirectional_iterator_tag, decltype(std::declval<T>().prev)>
{
	using base = std::iterator<std::bidirectional_iterator_tag, decltype(std::declval<T>().prev)>;
	using typename base::reference;
	using typename base::value_type;

	iterator(const T &t) : current{ GET_FIRST(&t) } {}
	iterator(const iterator &rhs) : current{ rhs.current } {}
	iterator &operator++() { current = GET_NEXT(current); return *this; }
	iterator &operator--() { current = GET_PREV(current); return *this; }
	iterator &operator++(int) { iterator lhs(*this); operator++(); return lhs; }
	iterator &operator--(int) { iterator lhs(*this); operator--(); return lhs; }

	bool operator ==(const iterator &rhs) { return current == rhs.current; }
	bool operator !=(const iterator &rhs) { return !operator==(rhs); }
	reference operator*() { return current; }

private:
	value_type current;
};

namespace detail {
template <typename T>
struct iterator_helper
{
	using Tptr = typename std::add_const<typename std::add_pointer<T>::type>::type;
	using prev_type = decltype(std::declval<T>().prev);
	using next_type = decltype(std::declval<T>().next);

	using result_type = iterator<T>;

	using is_applicable_helper = std::integral_constant<bool,
		std::is_convertible<Tptr, prev_type>::value
		&& std::is_convertible<Tptr, next_type>::value
		&& std::is_same<prev_type, next_type>::value>;

	static result_type apply(const T &t)
	{
		return iterator<T>(t);
	}
};
} // namespace detail

template<typename T>
typename std::enable_if<
	detail::iterator_helper<T>::is_applicable_helper::value, typename
	detail::iterator_helper<T>::result_type
>::type
begin(const T&t) {
	return detail::iterator_helper<T>::apply(t);
}

template<typename T>
typename std::enable_if<
	detail::iterator_helper<T>::is_applicable_helper::value, typename
	detail::iterator_helper<T>::result_type
>::type
end(const T&t) {

	iterator<T> last(detail::iterator_helper<T>::apply(t));
	--last; // make the iterator points to the head, which is considered out of list.

	return last;
}

} // namespace fred
} // namespace fso