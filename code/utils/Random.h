#pragma once

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <type_traits>

namespace util {

class Random {
public:
	static constexpr int MAX_VALUE = 0x7fffffff;
	static constexpr int HALF_MAX_VALUE = MAX_VALUE / 2;
	static constexpr float INV_F_MAX_VALUE = 1.0f / static_cast<float>(MAX_VALUE);

	// seed(1) will result in RNG using its default seed
	static void seed(unsigned int val);

	// return a value in [0, MAX_VALUE]
	static int next(); // uniformly between 0 and MAX_VALUE

	// return a value in [0, modulus-1]
	static int next(int modulus);

	// return a value in [low, high]
	static int next(int low, int high);

	// return true/false with equal probability
	static bool flip_coin();

	// jump ahead in the RNG sequence
	static void advance(unsigned long long distance);
private:
	Random();
};

// A UniformRandomBitGenerator over static_rand(), so that it can be passed to std::shuffle, the standard
// distributions, or deterministic_shuffle().  Successive calls return static_rand(seed), static_rand(seed + 1),
// and so on.  Because static_rand() is initialized from the netgame seed in multiplayer (see game_level_init),
// a generator constructed with the same seed produces the same sequence on every machine.  Note that
// std::shuffle and the standard distributions are implementation-defined, so for results that must match
// across platforms as well, use deterministic_shuffle().
//
// Be aware that static_rand() only looks at the low 13 bits of its input (see SEMIRAND_MAX_LOG), so there
// are only 8192 distinct sequences no matter how the seed is derived.  That is fine for shuffling a handful
// of items, where two seeds occasionally sharing a sequence is harmless, but it is not a source of real
// seed entropy.
class StaticRandGenerator {
	unsigned int m_seed;
	unsigned int m_count = 0;
public:
	using result_type = unsigned int;

	explicit StaticRandGenerator(unsigned int seed) : m_seed(seed) {}

	static constexpr result_type min() { return 0; }
	static constexpr result_type max() { return 0x3fffffff; }	// STATIC_RAND_MAX; checked in Random.cpp

	result_type operator()();
};

// Fisher-Yates shuffle with a fully specified algorithm.  Unlike std::shuffle, whose algorithm (and its use of
// uniform_int_distribution) is left to the implementation, this produces the same permutation on every platform
// given the same generator sequence, which is what multiplayer needs.  Accepts any UniformRandomBitGenerator.
// The modulo introduces a bias that is negligible for ranges far smaller than the generator's range.
template <typename RandomIt, typename URBG>
void deterministic_shuffle(RandomIt first, RandomIt last, URBG &&g)
{
	using diff_t = typename std::iterator_traits<RandomIt>::difference_type;
	using gen_t = std::remove_reference_t<URBG>;

	for (diff_t i = (last - first) - 1; i > 0; --i)
	{
		auto r = static_cast<uint64_t>(g() - gen_t::min());
		auto j = static_cast<diff_t>(r % static_cast<uint64_t>(i + 1));
		std::iter_swap(first + i, first + j);
	}
}
} // namespace util
