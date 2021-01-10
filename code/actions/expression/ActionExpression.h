#pragma once

#include "ParseContext.h"
#include "ProgramVariables.h"
#include "TypeDefinition.h"

#include "parse/parselo.h"

namespace actions {
namespace expression {
namespace nodes {
class AbstractExpression;
}
template <typename T>
class TypedActionExpression;

class ActionExpression {
  public:
	ActionExpression() = default;

	Value execute(const ProgramVariables& variables) const;

	bool isValid() const;

	template <typename T>
	TypedActionExpression<T> asTyped() const;

	static ActionExpression parseFromTable(ValueType expectedReturnType, const ParseContext& context);

  protected:
	explicit ActionExpression(std::shared_ptr<nodes::AbstractExpression> expression)
		: m_expression(std::move(expression))
	{
	}

	std::shared_ptr<nodes::AbstractExpression> m_expression;
};

template <typename T>
class TypedActionExpression {
  public:
	explicit TypedActionExpression(ActionExpression expression) : m_expression(std::move(expression)) {}

	T execute(const ProgramVariables& variables) const
	{
		// Do not crash on invalid expressions
		if (!m_expression.isValid()) {
			return T();
		}

		return m_expression.execute(variables).template get<T>();
	}

	static TypedActionExpression<T> parseFromTable(const ParseContext& context)
	{
		return ActionExpression::parseFromTable(ValueTypeTraits<T>::type, context).template asTyped<T>();
	}

  private:
	ActionExpression m_expression;
};

template <typename T>
TypedActionExpression<T> ActionExpression::asTyped() const
{
	return TypedActionExpression<T>(*this);
}

} // namespace expression
} // namespace actions
