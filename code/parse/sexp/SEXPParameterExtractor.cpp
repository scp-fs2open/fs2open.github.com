
#include "SEXPParameterExtractor.h"

#include "parse/sexp.h"

namespace sexp {
integer_return::integer_return() = default;
integer_return::integer_return(int _value) : value(_value) {}

namespace detail {
void nodeToValue(int node, bool& valOut) { valOut = is_sexp_true(node); }

void nodeToValue(int node, integer_return& valOut)
{
	valOut.value = eval_num(node, valOut.is_nan, valOut.is_nan_forever);
}

} // namespace detail

SEXPParameterExtractor::SEXPParameterExtractor(int node) : _node(node) {}
bool SEXPParameterExtractor::hasMore() const { return _node != -1; }

} // namespace sexp
