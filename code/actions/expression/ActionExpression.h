#pragma once

#include "AbstractExpression.h"
#include "TypeDefinition.h"

#include "parse/parselo.h"

namespace actions {
namespace expression {

class AbstractExpression;

class BaseActionExpression {
  protected:
	static std::shared_ptr<AbstractExpression> parseExpression(const SCP_string& expressionText);

	static SCP_string getTypeName(ValueType type);
};

template <typename T>
class ActionExpression : public BaseActionExpression {
  public:
	ActionExpression() = default;

	ActionExpression(const ActionExpression&) = default;
	ActionExpression& operator=(const ActionExpression&) = default;

	ActionExpression(ActionExpression&&) noexcept = default;
	ActionExpression& operator=(ActionExpression&&) noexcept = default;

	T execute() const
	{
		// Do not crash on invalid expressions
		if (m_expression == nullptr) {
			return T();
		}

		const auto returnVal = m_expression->execute();

		return returnVal.template get<T>();
	}

	static ActionExpression<T> parseFromTable()
	{
		SCP_string expressionText;

		stuff_string(expressionText, F_NAME);

		auto expression = parseExpression(expressionText);

		if (!expression) {
			return ActionExpression<T>(nullptr);
		}

		auto type = expression->getExpressionType();

		// Check if the type this evaluates to matches what we expect
		if (!checkTypeWithImplicitConversion(type, ValueTypeTraits<T>::type)) {
			error_display(0,
				"Expression evaluates to type <%s> but <%s> was expected!",
				getTypeName(type).c_str(),
				getTypeName(ValueTypeTraits<T>::type).c_str());
			return ActionExpression<T>();
		}

		// Everything is valid
		return ActionExpression<T>(expression);
	}

  private:
	explicit ActionExpression(std::shared_ptr<AbstractExpression> expression) : m_expression(std::move(expression)) {}

	std::shared_ptr<AbstractExpression> m_expression;
};

} // namespace expression
} // namespace actions
