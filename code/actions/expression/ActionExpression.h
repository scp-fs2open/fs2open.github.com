
#include "AbstractExpression.h"

#include "parse/parselo.h"

#pragma once

namespace actions {
namespace expression {

namespace detail {
template <typename T>
struct ValueTypeTraits;

template <>
struct ValueTypeTraits<int> {
	static constexpr ValueType type = ValueType::Integer;
};
template <>
struct ValueTypeTraits<float> {
	static constexpr ValueType type = ValueType::Float;
};
template <>
struct ValueTypeTraits<vec3d> {
	static constexpr ValueType type = ValueType::Vector;
};
template <>
struct ValueTypeTraits<SCP_string> {
	static constexpr ValueType type = ValueType::Identifier;
};

} // namespace detail

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

		return returnVal.get<T>();
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
		if (type != detail::ValueTypeTraits<T>::type) {
			error_display(0,
				"Expression evaluates to type <%s> but <%s> was expected!",
				getTypeName(type).c_str(),
				getTypeName(detail::ValueTypeTraits<T>::type).c_str());
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
