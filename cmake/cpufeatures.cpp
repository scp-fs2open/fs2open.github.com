// Code snippet from http://stackoverflow.com/a/7495023
#ifdef _WIN32
#include <intrin.h>
//  Windows
#define cpuid    __cpuid

#else

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

#endif

#include <iostream>

int main( int argc, char* argv[] )
{
	int SSE     = false;
	int SSE2    = false;
	int AVX     = false;
	int AVX2    = false;

	int info[4];
	cpuid(info, 0, 0);
	int nIds = info[0];

	//  Detect Instruction Set
	if (nIds >= 1){
		cpuid(info, 0x00000001, 0);
		SSE   = (info[3] & ((int)1 << 25)) != 0;
		SSE2  = (info[3] & ((int)1 << 26)) != 0;

		AVX   = (info[2] & ((int)1 << 28)) != 0;
	}
	if (nIds >= 7){
		cpuid(info,0x00000007, 0);
		AVX2  = (info[1] & ((int)1 <<  5)) != 0;
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
