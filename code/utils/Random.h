#pragma once

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
} // namespace util
