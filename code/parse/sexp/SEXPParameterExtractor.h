#pragma once

#include "globalincs/pstypes.h"

#include "parse/sexp.h"

namespace sexp {

struct integer_return {
	int value = -1;
	bool is_nan = false;
	bool is_nan_forever = false;

	integer_return();
	integer_return(int _value);
};

namespace detail {
void nodeToValue(int node, bool& valOut);

template <typename T>
struct conversion_traits {
	using output_type = T;
};

template <>
struct conversion_traits<int> {
	using output_type = integer_return;
};
void nodeToValue(int node, integer_return& valOut);

} // namespace detail

class SEXPParameterExtractor {
	int _node = -1;

  public:
	SEXPParameterExtractor(int node);

	bool hasMore() const;

	template <typename T>
	typename detail::conversion_traits<T>::output_type getArg()
	{
		Assertion(hasMore(), "Required value was missing!");
		typename detail::conversion_traits<T>::output_type value;
		detail::nodeToValue(_node, value);
		_node = CDR(_node);
		return value;
	}

	template <typename T>
	typename detail::conversion_traits<T>::output_type getOptionalArg(const T& defaultVal = T())
	{
		if (!hasMore()) {
			return typename detail::conversion_traits<T>::output_type(defaultVal);
		}

		return getArg<T>();
	}
};

} // namespace sexp
