#pragma once

#include "Executor.h"

namespace executor {

/**
 * @brief Abstract class representing the context of an execution.
 *
 * This is intended to be used with executors to ensure that work items are only executed in the same context as they
 * were started. See runInContext() for how to use this with executors.
 */
class IExecutionContext {
  public:
	enum class State {
		Valid,     //! We are currently in the state captured in this context
		Suspended, //! We are in a state that is not valid but may return to the captured state
		Invalid,   //! We are in an unrelated state
	};

	using Callback = std::function<Executor::CallbackResult(State contextState)>;

	virtual ~IExecutionContext() = default;

	/**
	 * @brief Determines the state of the context
	 * @return The context state
	 */
	virtual State determineContextState() const = 0;
};

/**
 * @brief Wraps the specified callback in an execution context
 *
 * The return value is a work item for an executor that will only call the passed function when the context is valid.
 * The function will not be called when the context is suspended. Should the context become invalid, the function will
 * be called one last time with State::Invalid to indicate to the function that the context became invalid. This is
 * useful to do cleanup work (e.g. setting an error state in created Lua Promises).
 *
 * @param context The execution context
 * @param func The function to wrap
 * @return An executor work item
 */
Executor::Callback runInContext(std::shared_ptr<IExecutionContext> context, IExecutionContext::Callback func);

} // namespace executor
