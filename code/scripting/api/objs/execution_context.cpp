#include "execution_context.h"

#include "enums.h"

namespace scripting {
namespace api {

execution_context_h::execution_context_h(std::shared_ptr<executor::IExecutionContext> executionContext)
	: m_executionContext(std::move(executionContext))
{
}
const std::shared_ptr<executor::IExecutionContext>& execution_context_h::getExecutionContext() const
{
	return m_executionContext;
}
bool execution_context_h::isValid() const { return m_executionContext != nullptr; }

ADE_OBJ(l_ExecutionContext,
	execution_context_h,
	"execution_context",
	"An execution context for asynchonous operations");

ADE_FUNC(determineState,
	l_ExecutionContext,
	nullptr,
	"Determines the current state of the context.",
	"enumeration",
	"One of the CONTEXT_ enumerations")
{
	execution_context_h* context = nullptr;
	if (!ade_get_args(L, "o", l_ExecutionContext.GetPtr(&context))) {
		return ADE_RETURN_FALSE;
	}

	if (!context->isValid()) {
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_CONTEXT_INVALID)));
	}

	const auto state = context->getExecutionContext()->determineContextState();
	switch (state) {
	case executor::IExecutionContext::State::Valid:
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_CONTEXT_VALID)));
	case executor::IExecutionContext::State::Suspended:
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_CONTEXT_SUSPENDED)));
	case executor::IExecutionContext::State::Invalid:
	default:
		return ade_set_args(L, "o", l_Enum.Set(enum_h(LE_CONTEXT_INVALID)));
	}
}

ADE_FUNC(isValid,
	l_ExecutionContext,
	nullptr,
	"Determines if the handle is valid",
	"boolean",
	"true if valid, false otherwise")
{
	execution_context_h* context = nullptr;
	if (!ade_get_args(L, "o", l_ExecutionContext.GetPtr(&context))) {
		return ADE_RETURN_FALSE;
	}

	return ade_set_args(L, "b", context != nullptr && context->isValid());
}

} // namespace api
} // namespace scripting
