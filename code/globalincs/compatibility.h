#ifndef __COMPATIBILITY_H__
#define __COMPATIBILITY_H__

void random_shuffle(int *first, int *last);

template <class RandomNumberGenerator>
void random_shuffle(int *first, int *last, RandomNumberGenerator& custom_rand);

#endif
