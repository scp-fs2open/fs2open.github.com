
#include "SEXPParameterExtractor.h"

#include "network/multi_sexp.h"
#include "parse/sexp.h"

namespace sexp {
integer_return::integer_return() = default;
integer_return::integer_return(int _value) : value(_value) {}

namespace detail {
void nodeToValue(int node, bool& valOut) { valOut = is_sexp_true(node); }
void sendValue(bool value) { Current_sexp_network_packet.send_bool(value); }
void getValueFromPacket(bool& valOut) { Current_sexp_network_packet.get_bool(valOut); }

void nodeToValue(int node, integer_return& valOut)
{
	valOut.value = eval_num(node, valOut.is_nan, valOut.is_nan_forever);
}
void sendValue(const integer_return& value)
{
	Current_sexp_network_packet.send_int(value.value);
	Current_sexp_network_packet.send_bool(value.is_nan);
	Current_sexp_network_packet.send_bool(value.is_nan_forever);
}
void getValueFromPacket(integer_return& valOut)
{
	Current_sexp_network_packet.get_int(valOut.value);
	Current_sexp_network_packet.get_bool(valOut.is_nan);
	Current_sexp_network_packet.get_bool(valOut.is_nan_forever);
}

} // namespace detail

SEXPParameterExtractor::SEXPParameterExtractor() : _multiClient(true) {}
SEXPParameterExtractor::SEXPParameterExtractor(int node, bool sendMultiData)
	: _sendMultiData(sendMultiData), _node(node)
{
}

SEXPParameterExtractor SEXPParameterExtractor::fromMultiPacket() { return {}; }
SEXPParameterExtractor SEXPParameterExtractor::fromSexpNode(int node, bool sendMultiData)
{
	return SEXPParameterExtractor(node, sendMultiData);
}

bool SEXPParameterExtractor::hasMore() const
{
	if (_multiClient) {
		return Current_sexp_network_packet.hasMore();
	}
	return _node != -1;
}
void SEXPParameterExtractor::sendMultiValues() {
	for (const auto& cb : _sendCallbacks) {
		cb();
	}
	_sendCallbacks.clear();
}

} // namespace sexp
