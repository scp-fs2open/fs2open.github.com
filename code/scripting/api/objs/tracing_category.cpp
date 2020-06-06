#include "tracing_category.h"

#include "tracing/tracing.h"

namespace scripting {
namespace api {

ADE_OBJ(l_TracingCategory, tracing::Category, "tracing_category", "A category for tracing engine performance");

ADE_FUNC(trace,
	l_TracingCategory,
	"function body()",
	"Traces the run time of the specified function that will be invoked in this call.",
	nullptr,
	nullptr)
{
	tracing::Category* category = nullptr;
	luacpp::LuaFunction func;
	if (!ade_get_args(L, "ou", l_TracingCategory.GetPtr(&category), &func)) {
		return ADE_RETURN_NIL;
	}

	if (!func.isValid()) {
		LuaError(L, "Invalid function reference passed!");
		return ADE_RETURN_NIL;
	}

	TRACE_SCOPE(*category);
	func();
	return ADE_RETURN_NIL;
}

} // namespace api
} // namespace scripting
