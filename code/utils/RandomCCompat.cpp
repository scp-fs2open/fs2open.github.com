#include "Random.h"
#include "RandomCCompat.h"

using Random = util::Random;

extern "C" {

void SCP_srand(unsigned int seed)
{
	Random::seed(seed);
}

int SCP_rand(void)
{
	return Random::next();
}

int SCP_RAND_MAX(void)
{
	return Random::MAX_VALUE;
}
} // extern "C"
