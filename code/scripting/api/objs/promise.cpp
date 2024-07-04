//
// Created by marius on 14.05.20.
//

#include "promise.h"

namespace scripting {
namespace api {

ADE_OBJ(l_Promise,
	LuaPromise,
	"promise",
	"A promise that represents an operation that will return a value at some point in the future");

// Can't call this "then" since that is a Lua keyword
ADE_FUNC(continueWith,
	l_Promise,
	"function(any... args) => any...",
	"When the called on promise resolves, this function will be called with the resolved value of the promise.",
	"promise",
	"A promise that will resolve with the return value of the passed function.")
{
	LuaPromise* promise = nullptr;
	luacpp::LuaFunction thenFunc;
	if (!ade_get_args(L, "ou", l_Promise.GetPtr(&promise), &thenFunc)) {
		return ADE_RETURN_NIL;
	}

	if (!thenFunc.isValid()) {
		LuaError(L, "'continueWith' function is invalid!");
		return ADE_RETURN_NIL;
	}

	if (promise == nullptr || !promise->isValid()) {
		LuaError(L, "Invalid promise detected. This should not happen. Please contact a developer.");
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L,
		"o",
		l_Promise.Set(promise->then([L, thenFunc](const luacpp::LuaValueList& val) { return thenFunc(L, val); })));
}

ADE_FUNC(catch,
	l_Promise,
	"function(any... args) => any...",
	"When the called on promise produces an error, this function will be called with the error value of the promise.",
	"promise",
	"A promise that will resolve with the return value of the passed function.")
{
	LuaPromise* promise = nullptr;
	luacpp::LuaFunction thenFunc;
	if (!ade_get_args(L, "ou", l_Promise.GetPtr(&promise), &thenFunc)) {
		return ADE_RETURN_NIL;
	}

	if (!thenFunc.isValid()) {
		LuaError(L, "'continueWith' function is invalid!");
		return ADE_RETURN_NIL;
	}

	if (promise == nullptr || !promise->isValid()) {
		LuaError(L, "Invalid promise detected. This should not happen. Please contact a developer.");
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "o", l_Promise.Set(promise->catchError([L, thenFunc](const luacpp::LuaValueList& val) {
		return thenFunc(L, val);
	})));
}

ADE_FUNC(isResolved,
	l_Promise,
	nullptr,
	"Checks if the promise is already resolved.",
	"boolean",
	"true if resolved, false if result is still pending.")
{
	LuaPromise* promise = nullptr;
	if (!ade_get_args(L, "o", l_Promise.GetPtr(&promise))) {
		return ADE_RETURN_NIL;
	}

	if (promise == nullptr || !promise->isValid()) {
		LuaError(L, "Invalid promise detected. This should not happen. Please contact a developer.");
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "b", promise->isResolved());
}

ADE_FUNC(isErrored,
	l_Promise,
	nullptr,
	"Checks if the promise is already in an error state.",
	"boolean",
	"true if errored, false if result is still pending.")
{
	LuaPromise* promise = nullptr;
	if (!ade_get_args(L, "o", l_Promise.GetPtr(&promise))) {
		return ADE_RETURN_NIL;
	}

	if (promise == nullptr || !promise->isValid()) {
		LuaError(L, "Invalid promise detected. This should not happen. Please contact a developer.");
		return ADE_RETURN_NIL;
	}

	return ade_set_args(L, "b", promise->isErrored());
}

ADE_FUNC(getValue,
	l_Promise,
	nullptr,
	"Gets the resolved value of this promise. Causes an error when used on an unresolved or errored promise!",
	"any",
	"The resolved values.")
{
	LuaPromise* promise = nullptr;
	if (!ade_get_args(L, "o", l_Promise.GetPtr(&promise))) {
		return ADE_RETURN_NIL;
	}

	if (promise == nullptr || !promise->isValid()) {
		LuaError(L, "Invalid promise detected. This should not happen. Please contact a developer.");
		return ADE_RETURN_NIL;
	}

	if (!promise->isResolved()) {
		LuaError(L, "Tried to get the value of a promise that was not resolved!");
		return ADE_RETURN_NIL;
	}

	// We can't use our usual functions for returning here since we return a variable amount of values
	const auto& retVals = promise->resolveValue();
	for (const auto& retVal : retVals) {
		retVal.pushValue(L);
	}
	return static_cast<int>(retVals.size());
}

ADE_FUNC(getErrorValue,
	l_Promise,
	nullptr,
	"Gets the error value of this promise. Causes an error when used on an unresolved or resolved promise!",
	"any",
	"The error values.")
{
	LuaPromise* promise = nullptr;
	if (!ade_get_args(L, "o", l_Promise.GetPtr(&promise))) {
		return ADE_RETURN_NIL;
	}

	if (promise == nullptr || !promise->isValid()) {
		LuaError(L, "Invalid promise detected. This should not happen. Please contact a developer.");
		return ADE_RETURN_NIL;
	}

	if (!promise->isErrored()) {
		LuaError(L, "Tried to get the value of a promise that was not resolved!");
		return ADE_RETURN_NIL;
	}

	// We can't use our usual functions for returning here since we return a variable amount of values
	const auto& retVals = promise->errorValue();
	for (const auto& retVal : retVals) {
		retVal.pushValue(L);
	}
	return static_cast<int>(retVals.size());
}

} // namespace api
} // namespace scripting
