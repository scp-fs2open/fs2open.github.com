#pragma once

#include "globalincs/pstypes.h"

#include "ParseContext.h"

#include "actions/expression/nodes/AbstractExpression.h"

namespace actions {
namespace expression {

class ExpressionParser {
  public:
	ExpressionParser(SCP_string expressionText);

	std::shared_ptr<nodes::AbstractExpression> parse(const ParseContext& context);

	const SCP_string& getErrorText() const;

  private:
	SCP_string m_expressionText;

	SCP_string m_errorText;
};

} // namespace expression
} // namespace actions
