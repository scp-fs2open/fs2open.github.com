#ifndef HASHING_H
#define HASHING_H

#define FNV_32_PRIME ((unsigned int)0x01000193)
#define FNV_32_INIT ((unsigned int )2166136261)
#define FNV_MASK_8 (((unsigned int)1<<8)-1)

unsigned int hash_coords_2(unsigned int x, unsigned int y, unsigned int seed);
unsigned int hash_coords_3(unsigned int x, unsigned int y, unsigned int z, unsigned int seed);
unsigned int hash_coords_4(unsigned int x, unsigned int y, unsigned int z, unsigned int w, unsigned int seed);
unsigned int hash_coords_6(unsigned int x, unsigned int y, unsigned int z, unsigned int w, unsigned int u, unsigned int v, unsigned int seed);

unsigned int fnv_32_a_combine(unsigned int hash, unsigned int val);
unsigned char xor_fold_hash(unsigned int hash);

#endif

