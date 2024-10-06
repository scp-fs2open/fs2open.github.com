// Code snippet from http://stackoverflow.com/a/7495023
#ifdef _WIN32
#include <intrin.h>
//  Windows
#define cpuid    __cpuidex
#define xgetbv   _xgetbv

#else

#include <cstdint>

//  GCC Inline Assembly
void cpuid(int CPUInfo[4], int InfoType, int SubType){
    __asm__ __volatile__ (
        "cpuid":
        "=a" (CPUInfo[0]),
        "=b" (CPUInfo[1]),
        "=c" (CPUInfo[2]),
        "=d" (CPUInfo[3]) :
        "a" (InfoType),
		"c" (SubType)
    );
}

uint64_t xgetbv(uint32_t xcr) {
	uint32_t lower, higher;
	__asm__ __volatile__ (
		"xgetbv":
		"=a"(lower),
		"=d"(higher) :
		"c"(xcr)
	);
	return (static_cast<uint64_t>(higher) << static_cast<uint64_t>(32)) | static_cast<uint64_t>(lower);
}

#endif

#include <iostream>

int main( int argc, char* argv[] )
{
	bool SSE     = false;
	bool SSE2    = false;
	bool OSXSAVE = false;
	bool AVX     = false;
	bool AVX2    = false;

	int info[4];
	cpuid(info, 0, 0);
	int nIds = info[0];

	//  Detect Instruction Set
	if (nIds >= 1){
		cpuid(info, 0x00000001, 0);
		SSE     = (info[3] & ((int)1 << 25)) != 0;
		SSE2    = (info[3] & ((int)1 << 26)) != 0;
		OSXSAVE = (info[2] & ((int)1 << 27)) != 0;
		AVX     = (info[2] & ((int)1 << 28)) != 0;
	}
	if (nIds >= 7){
		cpuid(info,0x00000007, 0);
		AVX2  = (info[1] & ((int)1 <<  5)) != 0;
	}

	//  Detect OS Support
	if (!OSXSAVE || (xgetbv(0) & 0x6) == 0) {
		AVX = false;
		AVX2 = false;
	}

	if (AVX2)
		std::cout << "AVX2";
	else if(AVX)
		std::cout << "AVX";
	else if(SSE2)
		std::cout << "SSE2";
	else if(SSE)
		std::cout << "SSE";

	return 0;
}
