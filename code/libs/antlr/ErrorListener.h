
#include "globalincs/pstypes.h"

#include <BaseErrorListener.h>
#include <Token.h>

namespace libs {
namespace antlr {

struct Diagnostic {
	size_t line;
	size_t columnInLine;
	size_t tokenLength;
	SCP_string errorMessage;
};

struct ErrorListener : public antlr4::BaseErrorListener {
	SCP_vector<Diagnostic> diagnostics;

	void syntaxError(antlr4::Recognizer* /*recognizer*/,
		antlr4::Token* offendingSymbol,
		size_t line,
		size_t charPositionInLine,
		const std::string& msg,
		std::exception_ptr /*e*/) override;
};

} // namespace antlr
} // namespace libs
