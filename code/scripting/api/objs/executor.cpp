
#include "executor.h"

#include <utility>

namespace scripting {
namespace api {

executor_h::executor_h() = default;
executor_h::executor_h(std::shared_ptr<executor::Executor> executor) : m_executor(std::move(executor)) {}
bool executor_h::isValid() const { return m_executor != nullptr; }
const std::shared_ptr<executor::Executor>& executor_h::getExecutor() const { return m_executor; }

//**********HANDLE: executor
ADE_OBJ(l_Executor, executor_h, "executor", "An executor that can be used for scheduling the execution of a function.");

ADE_FUNC(schedule,
	l_Executor,
	"function() => boolean",
	"Takes a function that returns a boolean and schedules that for execution on this executor. If the function "
	"returns true it will be run again the. If it returns false it will be removed from the executor.<br>Note: Use "
	"this with care since using this without proper care can lead to the function being run in states that are not "
	"desired. Consider using async.run.",
	"boolean",
	"true if function was scheduled, false otherwise.")
{
	executor_h* executor = nullptr;
	luacpp::LuaFunction func;
	if (!ade_get_args(L, "ou", l_Executor.GetPtr(&executor), &func)) {
		return ADE_RETURN_FALSE;
	}

	if (executor == nullptr || !executor->isValid()) {
		return ADE_RETURN_FALSE;
	}

	if (!func.isValid()) {
		LuaError(L, "Invalid function handle specified!");
		return ADE_RETURN_FALSE;
	}

	// Post the function onto the executor with a wrapper to convert the Lua value to the proper enum
	executor->getExecutor()->post([L, func]() {
		const auto ret = func(L);

		if (ret.empty()) {
			return executor::Executor::CallbackResult::Done;
		}

		if (!ret[0].is(luacpp::ValueType::BOOLEAN)) {
			return executor::Executor::CallbackResult::Done;
		}

		if (ret[0].getValue<bool>()) {
			return executor::Executor::CallbackResult::Reschedule;
		}
		return executor::Executor::CallbackResult::Done;
	});

	return ADE_RETURN_TRUE;
}

ADE_FUNC(isValid,
	l_Executor,
	nullptr,
	"Determined if this handle is valid",
	"boolean",
	"true if valid, false otherwise.")
{
	executor_h* executor = nullptr;
	if (!ade_get_args(L, "o", l_Executor.GetPtr(&executor))) {
		return ADE_RETURN_FALSE;
	}

	if (executor == nullptr || !executor->isValid()) {
		return ADE_RETURN_FALSE;
	}
	return ADE_RETURN_TRUE;
}

} // namespace api
} // namespace scripting
