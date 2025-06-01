
# Sample toolchain file for building for Windows from a Linux system.
#
# Typical usage:
#    *) install cross compiler: `sudo apt-get install mingw-w64 g++-mingw-w64`
#    *) cd build
#    *) cmake -DCMAKE_TOOLCHAIN_FILE=~/Toolchain-Ubuntu-mingw64.cmake ..

set(CMAKE_SYSTEM_NAME Windows)
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

# cross compilers to use for C and C++
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

# target environment on the build host system
#   set 1st to dir with the cross compiler's C/C++ headers/libs
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

# modify default behavior of FIND_XXX() commands to
# search for headers/libs in the target environment and
# don't search for programs in the build host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

if (FSO_BUILD_WITH_DISCORD)
    message(WARNING "Discord support is not tested for cross-compilation. Here be dragons! To disable, set -DFSO_BUILD_WITH_DISCORD:BOOL=OFF")
endif()

if (FSO_BUILD_FRED2)
    message(WARNING "FRED2 support is not tested for cross-compilation. Here be dragons! To disable, set -DFSO_BUILD_FRED2:BOOL=OFF")
endif()

if (FSO_USE_SPEECH)
    message(WARNING "Speech support is not tested for cross-compilation. Here be dragons! To disable, set -DFSO_USE_SPEECH:BOOL=OFF")
endif()

if (FSO_USE_VOICEREC)
    message(WARNING "Voice recognition support is not tested for cross-compilation. Here be dragons! To disable, set -DFSO_USE_VOICEREC:BOOL=OFF")
endif()