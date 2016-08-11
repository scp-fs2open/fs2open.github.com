
include(EnableExtraCompilerWarnings)
include(CheckCXXCompilerFlag)
include(util)

MESSAGE(STATUS "Doing configuration specific to visual studio...")

set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS Debug)

option(MSVC_USE_RUNTIME_DLL "Use the dynamically linked version of the runtime" OFF)
MARK_AS_ADVANCED(FORCE MSVC_USE_RUNTIME_DLL)

# These are the warnings we disable
set(WARNING_FLAGS
	/wd"4100" # unreferenced formal parameters
	/wd"4127" # constant conditional (assert)
	/wd"4201" # nonstandard extension used: nameless struct/union (happens a lot in Windows include headers)
	/wd"4290" # C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
	/wd"4390" # empty control statement (triggered by nprintf and mprintf's inside of one-line if's, etc)
	/wd"4410" # illegal size for operand... ie... 	fxch st(1)
	/wd"4511" # copy constructor could not be generated (happens a lot in Windows include headers)
	/wd"4512" # assignment operator could not be generated (happens a lot in Windows include headers)
	/wd"4514" # unreferenced inline function removed
	/wd"4611" # _setjmp warning.  Since we use setjmp alot, and we don't really use constructors or destructors, this warning doesn't really apply to us.
	/wd"4663" # C++ language change (template specification)
	/wd"4710" # is inline function not expanded (who cares?)
	/wd"4711" # tells us an inline function was expanded (who cares?)
	/wd"4786" # is identifier truncated to 255 characters (happens all the time in Microsoft #includes) -- Goober5000"
	/wd"4996" # deprecated strcpy, strcat, sprintf, etc. (from MSVC 2005) - taylor
	/wd"4311" # Disables warnings about casting pointer types to ints. The funny thing is these warnings can't be resolved, just disabled... - m!m
	/wd"4302" # Same as above - m!m
	/wd"4366" # The result of the unary '&' operator may be unaligned - m!m
	$<$<CONFIG:Release>:/wd"4101"> # In release mode there are unreferenced variables because debug needs them
)

target_compile_options(compiler INTERFACE ${WARNING_FLAGS})

# Base
set(CMAKE_C_FLAGS "/MP /GS- /analyze- /Zc:wchar_t /errorReport:prompt /WX- /Zc:forScope /Gd /EHsc /nologo")
set(CMAKE_CXX_FLAGS "/MP /GS- /analyze- /Zc:wchar_t /errorReport:prompt /WX- /Zc:forScope /Gd /EHsc /nologo")

set(CMAKE_EXE_LINKER_FLAGS "/MANIFEST /DYNAMICBASE:NO /SAFESEH:NO /ERRORREPORT:PROMPT /NOLOGO")
set(CMAKE_STATIC_LINKER_FLAGS "")

# Release
set(CMAKE_C_FLAGS_RELEASE "/GL /W2 /Gy- /Ox /Ot /Ob2 /fp:precise /GF /Oy /Oi /Zi /W3")
set(CMAKE_CXX_FLAGS_RELEASE "/GL /W2 /Gy- /Ox /Ot /Ob2 /fp:precise /GF /Oy /Oi /Zi /W3")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/OPT:REF /LTCG /INCREMENTAL:NO")
set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "/LTCG")

globally_enable_extra_compiler_warnings()

CHECK_CXX_COMPILER_FLAG("/Zo" MSVC_COMPILER_SUPPORTS_ARCH_ZO)

if (MSVC_COMPILER_SUPPORTS_ARCH_ZO)
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zo")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zo")
endif()

IF(MSVC_USE_RUNTIME_DLL)
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
ELSE(MSVC_USE_RUNTIME_DLL)
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
ENDIF(MSVC_USE_RUNTIME_DLL)

# Debug
set(CMAKE_C_FLAGS_DEBUG "/W4 /Gy /Zi /Od /RTC1 /Gd /Oy-")
set(CMAKE_CXX_FLAGS_DEBUG "/W4 /Gy /Zi /Od /RTC1 /Gd /Oy-")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG /INCREMENTAL:NO /MAPINFO:EXPORTS /NODEFAULTLIB:libcmt.lib")

IF(MSVC_USE_RUNTIME_DLL)
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MDd")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
ELSE(MSVC_USE_RUNTIME_DLL)
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
ENDIF(MSVC_USE_RUNTIME_DLL)

INCLUDE(MSVCMultipleProcessCompile)

# Visual Studio supports compiling for multiple vector instruction sets
SET(POSSIBLE_INSTUCTION_SETS "" SSE SSE2 AVX)

if (NOT DEFINED MSVC_SIMD_INSTRUCTIONS)
	detect_simd_instructions(MSVC_DETECTED_SIMD_INSTRUCTIONS)

	SET(MSVC_SIMD_INSTRUCTIONS "${MSVC_DETECTED_SIMD_INSTRUCTIONS}" CACHE FILEPATH "The SIMD instructions which will be used, possible values are ${POSSIBLE_INSTUCTION_SETS}")
	MARK_AS_ADVANCED(FORCE MSVC_SIMD_INSTRUCTIONS)
endif()
set(FSO_INSTRUCTION_SET ${MSVC_SIMD_INSTRUCTIONS})

LIST(FIND POSSIBLE_INSTUCTION_SETS "${MSVC_SIMD_INSTRUCTIONS}" SET_INDEX)

if (SET_INDEX LESS 0)
	MESSAGE(STATUS "An invalid instruction set was specified, defaulting to no special compiler options.")
else()
	IF (NOT SET_INDEX EQUAL 0)
		SET(FOUND)

		FOREACH(list_index RANGE ${SET_INDEX} 1)
			list(GET POSSIBLE_INSTUCTION_SETS ${list_index} _simd_set)
			CHECK_CXX_COMPILER_FLAG("/arch:${_simd_set}" COMPILER_SUPPORTS_ARCH_${_simd_set})

			IF(COMPILER_SUPPORTS_ARCH_${_simd_set})
				set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:${_simd_set}")
				set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:${_simd_set}")

				SET(FOUND TRUE)
				BREAK()
			ENDIF()
		ENDFOREACH(list_index)

		IF(NOT FOUND)
			# Don't set anything, it will likely not work
			MESSAGE(STATUS "Your compiler does not support any optimization flags, defaulting to none")
		ENDIF(NOT FOUND)
	ELSE()
		CHECK_CXX_COMPILER_FLAG("/arch:IA32" COMPILER_SUPPORTS_ARCH_IA32)

		IF(COMPILER_SUPPORTS_ARCH_IA32)
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:IA32")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:IA32")
		ENDIF(COMPILER_SUPPORTS_ARCH_IA32)
	ENDIF()
endif()

target_compile_definitions(compiler INTERFACE _CRT_SECURE_NO_DEPRECATE
	_CRT_SECURE_NO_WARNINGS _SECURE_SCL=0 NOMINMAX)
