#pragma once

#include "globalincs/pstypes.h"

#include <functional>

namespace util {

template <typename Ret, typename... Args>
class event final {
  public:
	using callback_type = std::function<Ret(Args...)>;

	event()  = default;
	~event() = default;

	// no copies
	event(const event<Ret, Args...>& other) = delete;
	event<Ret, Args...>& operator=(const event<Ret, Args...>& other) = delete;

	// moves allowed
	event(event<Ret, Args...>&& other) noexcept = default;
	event<Ret, Args...>& operator=(event<Ret, Args...>&& other) noexcept = default;

	void add(callback_type func) { _listeners.push_back(func); }

	void clear() { _listeners.clear(); }

	// This variant is used if the listeners return no values
	template <typename Dummy = void>
	inline typename std::enable_if<std::is_same<Ret, void>::value, Dummy>::type operator()(Args... args) const
	{
		for (const auto& l : _listeners) {
			l(args...);
		}
	}

	// In this case we collect the return values and return them to the caller
	template <typename Dummy = SCP_vector<Ret>>
	inline typename std::enable_if<!std::is_same<Ret, void>::value, Dummy>::type operator()(Args... args) const
	{
		SCP_vector<Ret> vals;

		for (const auto& l : _listeners) {
			vals.push_back(l(args...));
		}

		return vals;
	}

  private:
	SCP_vector<callback_type> _listeners;
};

} // namespace util
