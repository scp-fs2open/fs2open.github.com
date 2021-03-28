#include "Random.h"

#include <limits>
#include <random>

#include <cstdint>

#include "globalincs/pstypes.h"

namespace util {
namespace {
template <typename RngType>
class RandomImpl {
public:
	void seed(const unsigned int val)
	{
		static_assert(RngType::min() == 0, "RNG min must be 0");
		static_assert(RngType::max() == std::numeric_limits<uint32_t>::max(), "RNG max must be INT32_MAX");

		// follow srand semantics
		if (val == 1) {
			m_rng.seed();
		} else {
			m_rng.seed(val);
		}
	}

	int next()
	{
		return m_rng() & Random::MAX_VALUE;
	}

	void advance(unsigned long long distance)
	{
		m_rng.discard(distance);
	}

private:
	RngType m_rng;
};

RandomImpl<std::mt19937> SCP_rng;
} // namespace

Random::Random() = default;

void Random::seed(unsigned int val)
{
	Assert(val > 0);
	SCP_rng.seed(val);
}

int Random::next()
{
	return SCP_rng.next();
}

int Random::next(int modulus)
{
	Assert(modulus > 0);

	return SCP_rng.next() % modulus;
}

int Random::next(int low, int high)
{
	const int range = high - low + 1;
	Assert(range > 0);

	return low + (SCP_rng.next() % range);
}

bool Random::flip_coin()
{
	// [0, HALF_MAX_VALUE] and [HALF_MAX_VALUE+1,MAX_VALUE] are the same size
	return SCP_rng.next() <= Random::HALF_MAX_VALUE;
}

void Random::advance(unsigned long long distance)
{
	SCP_rng.advance(distance);
}
} // namespace util
