#!/usr/bin/env bash

if [ "$COMPILER" = "gcc-5" ]; then
    export CC=gcc-5
    export CXX=g++-5
fi
if [ "$COMPILER" = "gcc-10" ]; then
    export CC=gcc-10
    export CXX=g++-10
fi
if [ "$COMPILER" = "clang-9" ]; then
    # Work around bug in clang that uses the wrong installation directory: https://bugs.llvm.org/show_bug.cgi?id=47460
    export CC=$(readlink -f $(which clang-9))
    export CXX=$(readlink -f $(which clang++-9))
fi

LD_LIBRARY_PATH=$Qt5_DIR/lib:$LD_LIBRARY_PATH
CXXFLAGS="-m64 -mtune=generic -mfpmath=sse -msse -msse2 -pipe -Wno-unknown-pragmas"
CFLAGS="-m64 -mtune=generic -mfpmath=sse -msse -msse2 -pipe -Wno-unknown-pragmas"

CMAKE_OPTIONS="$JOB_CMAKE_OPTIONS"
if [[ "$COMPILER" =~ ^clang.*$ ]]; then
    CMAKE_OPTIONS="$CMAKE_OPTIONS -DCLANG_USE_LIBCXX=ON"
fi

mkdir build
cd build

export CXXFLAGS
export CFLAGS
export LD_LIBRARY_PATH

cmake -G Ninja -DFSO_FATAL_WARNINGS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $CMAKE_OPTIONS \
    -DCMAKE_INSTALL_PREFIX="$(pwd)/install" -DFSO_BUILD_APPIMAGE=ON -DCMAKE_BUILD_TYPE=$CONFIGURATION \
    -DFFMPEG_USE_PRECOMPILED=ON -DFSO_BUILD_TESTS=ON -DFSO_BUILD_INCLUDED_LIBS=ON -DFSO_BUILD_QTFRED=ON \
    -DSHADERS_ENABLE_COMPILATION=ON ..
