
#include "ErrorListener.h"

namespace libs {
namespace antlr {

void ErrorListener::syntaxError(antlr4::Recognizer*,
	antlr4::Token* offendingSymbol,
	size_t line,
	size_t charPositionInLine,
	const std::string& msg,
	std::exception_ptr)
{
	// Apparently stop < start is a thing?
	const auto left = std::min(offendingSymbol->getStartIndex(), offendingSymbol->getStopIndex());
	const auto right = std::max(offendingSymbol->getStartIndex(), offendingSymbol->getStopIndex());
	const auto length = right - left + 1;

	diagnostics.push_back(Diagnostic{line, charPositionInLine, length, msg});
}

} // namespace antlr
} // namespace libs
