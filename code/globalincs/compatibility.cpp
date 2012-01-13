#include "globalincs/compatibility.h"
#include "stdlib.h"

int rand_function(int limit)
{
	return (rand() % limit);
}

// adapted from http://www.cplusplus.com/reference/algorithm/random_shuffle/
void random_shuffle(int *first, int *last)
{
	random_shuffle(first, last, rand_function);
}

// adapted from http://www.cplusplus.com/reference/algorithm/random_shuffle/
template <class RandomNumberGenerator>
void random_shuffle(int *first, int *last, RandomNumberGenerator& custom_rand)
{
	int i, n, temp;
	n = (last-first);

	for (i=n-1; i>0; --i)
	{
		int pos = custom_rand(i+1);
		temp = first[i];
		first[i] = first[pos];
		first[pos] = temp;
	}
}