#if defined(__GNUC__) && !defined(__x86_64)
int main(void)
{
	/*
	 * For cases where we are trying to build with GNU
	 * on a non-x86_64 platform.
	 */
	return 0;
}
#else
// Code snippet from http://stackoverflow.com/a/7495023
#ifdef _WIN32
#include <intrin.h>
//  Windows
#define cpuid    __cpuid

#else

//  GCC Inline Assembly
void cpuid(int CPUInfo[4], int InfoType){
    __asm__ __volatile__ (
        "cpuid":
        "=a" (CPUInfo[0]),
        "=b" (CPUInfo[1]),
        "=c" (CPUInfo[2]),
        "=d" (CPUInfo[3]) :
        "a" (InfoType)
    );
}

#endif

#include <iostream>

#if defined(_WIN32)
	#define SSE_FLAG_STR    "SSE"
	#define SSE2_FLAG_STR   "SSE2"
	#define SSE3_FLAG_STR   "SSE2"   /* MSVC does not have a flag for   */
	#define SSSE3_FLAG_STR  "SSE2"   /* SSE3, SSSE3, SSE4.1 or SSE4.2.  */
	#define SSE4_1_FLAG_STR "SSE2"   /* Automatically downgrade to SSE2 */
	#define SSE4_2_FLAG_STR "SSE2"   /* for those four (4) cases.       */
	#define AVX_FLAG_STR    "AVX"
	#define AVX2_FLAG_STR   "AVX2"
#elif defined(__GNUC__)
	#define SSE_FLAG_STR    "sse"
	#define SSE2_FLAG_STR   "sse2"
	#define SSE3_FLAG_STR   "sse3"
	#define SSSE3_FLAG_STR  "ssse2"
	#define SSE4_1_FLAG_STR "sse4.1"
	#define SSE4_2_FLAG_STR "sse4.2"
	#define AVX_FLAG_STR    "avx"
	#define AVX2_FLAG_STR   "avx2"
#endif

#define CPUID_SSE      0x02000000
#define CPUID_SSE2     0x04000000
#define CPUID_SSE3     0x00000001
#define CPUID_SSSE3    0x00000200
#define CPUID_SSE4_1   0x00080000
#define CPUID_SSE4_2   0x00100000
#define CPUID_AVX      0x10000000
#define CPUID_AVX2     0x00000020

struct simd_flag {
	unsigned int  supported;  /* Non-zero if SIMD set supported */
    const char   *simd_flag;  /* Flag passed to the compiler */
};

int main(int argc, char* argv[])
{
	int info[4];
	cpuid(info, 0);

	int nIds = info[0];

	struct simd_flag simd_table[] = {
		{0, SSE_FLAG_STR},        /* Index 0 */
		{0, SSE2_FLAG_STR},       /* Index 1 */
		{0, SSE3_FLAG_STR},       /* Index 2 */
		{0, SSSE3_FLAG_STR},      /* Index 3 */
		{0, SSE4_1_FLAG_STR},     /* Index 4 */
		{0, SSE4_2_FLAG_STR},     /* Index 5 */
		{0, AVX_FLAG_STR},        /* Index 6 */
		{0, AVX2_FLAG_STR}        /* Index 7 */
	};

	/*
	 * Query the CPU to determine which SIMD instruction sets are supported.
	 * This information will eventually be passed to the build system.
	 */
	if (nIds >= 1) {
		cpuid(info, 0x00000001);

		simd_table[0].supported = (info[3] & CPUID_SSE);
		simd_table[1].supported = (info[3] & CPUID_SSE2);
		simd_table[2].supported = (info[2] & CPUID_SSE3);
		simd_table[3].supported = (info[2] & CPUID_SSSE3);
		simd_table[4].supported = (info[2] & CPUID_SSE4_1);
		simd_table[5].supported = (info[2] & CPUID_SSE4_2);
		simd_table[6].supported = (info[2] & CPUID_AVX);
	}

	if (nIds >= 7){
		cpuid(info, 0x00000007);

		simd_table[7].supported = (info[1] & CPUID_AVX2);
	}

	/*
	 * Find the most advanced set of supported SIMD instruction sets and
	 * print its associated compiler flag.
	 */
	for (int i = sizeof(simd_table) / sizeof(simd_table[0]) - 1;
	     i >= 0;
	     i--) {
		if (simd_table[i].supported != 0) {
			std::cout << simd_table[i].simd_flag;
			break;
		}
	}

	return 0;
}
#endif
