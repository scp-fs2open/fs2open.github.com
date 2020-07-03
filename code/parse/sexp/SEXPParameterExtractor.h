#pragma once

#include "globalincs/pstypes.h"

#include "network/multi.h"
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
template <typename T>
struct conversion_traits {
	using output_type = T;
};

void nodeToValue(int node, bool& valOut);
void sendValue(bool value);
void getValueFromPacket(bool& valOut);

template <>
struct conversion_traits<int> {
	using output_type = integer_return;
};
void nodeToValue(int node, integer_return& valOut);
void sendValue(const integer_return& value);
void getValueFromPacket(integer_return& valOut);

} // namespace detail

class SEXPParameterExtractor {
	bool _multiClient = false;
	bool _sendMultiData = false;
	int _node = -1;

	using SendValueCallback = std::function<void()>;

	SCP_vector<SendValueCallback> _sendCallbacks;

	SEXPParameterExtractor();
	explicit SEXPParameterExtractor(int node, bool sendMultiData);

  public:
	static SEXPParameterExtractor fromMultiPacket();
	static SEXPParameterExtractor fromSexpNode(int node, bool sendMultiData);

	bool hasMore() const;

	template <typename T>
	typename detail::conversion_traits<T>::output_type getArg()
	{
		Assertion(hasMore(), "Required value was missing!");
		typename detail::conversion_traits<T>::output_type value;

		if (MULTIPLAYER_MASTER && _sendMultiData) {
			Assertion(!_multiClient, "A multiplayer client parameter extractor was created on the master!");

			detail::nodeToValue(_node, value);
			// This might look weird but we cannot actually send this value yet since we are still evaluating our
			// parameters and sending now might mix the parameters of different SEXP operators
			_sendCallbacks.push_back([value]() { detail::sendValue(value); });
			_node = CDR(_node);
		} else if (MULTIPLAYER_CLIENT) {
			detail::getValueFromPacket(value);
		} else {
			detail::nodeToValue(_node, value);
			_node = CDR(_node);
		}

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

	/**
	 * @brief Send recorded values via the SEXP packet system
	 */
	void sendMultiValues();
};

} // namespace sexp
