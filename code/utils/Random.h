#pragma once

namespace util {

class Random {
  public:
	static constexpr int MAX_VALUE = 0x7fffffff;

	static void seed(unsigned int val);

	static int next(); // uniformly between 0 and MAX_VALUE

	static int next(int modulus);

	static int next(int low, int high);

	// jump ahead in the RNG sequence
	static void advance(unsigned long long distance);
};
} // namespace util
