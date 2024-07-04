#pragma once

#include <utility>

namespace util {
namespace detail {
template <typename F>
struct FinalAction {
	FinalAction(F f) : m_finally{std::move(f)} {}
	~FinalAction()
	{
		if (m_enabled) {
			m_finally();
		}
	}

	void disable() { m_enabled = false; };

  private:
	F m_finally;
	bool m_enabled{true};
};
} // namespace detail

template <typename F>
detail::FinalAction<F> finally(F f)
{
	return detail::FinalAction<F>(f);
}

} // namespace util
