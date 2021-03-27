#ifndef SCP_RANDOM_C_COMPAT_H
#define SCP_RANDOM_C_COMPAT_H

// provides a C compatibility layer to the Random class

#ifdef __cplusplus
extern "C" {
#endif

void SCP_srand(unsigned int seed);

int SCP_rand(void);

int SCP_RAND_MAX(void);

#ifdef __cplusplus
}
#endif

#endif // SCP_RANDOM_C_COMPAT_H

