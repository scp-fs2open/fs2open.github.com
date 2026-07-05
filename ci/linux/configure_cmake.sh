#!/usr/bin/env bash

if [ "$COMPILER" = "gcc-9" ]; then
    export CC=gcc-9
    export CXX=g++-9
fi
if [ "$COMPILER" = "gcc-13" ]; then
    export CC=gcc-13
    export CXX=g++-13
fi
if [ "$COMPILER" = "clang-16" ]; then
    export CC=clang-16
    export CXX=clang++-16
fi

LD_LIBRARY_PATH=$Qt5_DIR/lib:$LD_LIBRARY_PATH
if [ "$RUNNER_OS" = "macOS" ]; then
    CXXFLAGS="-mtune=generic -pipe -Wno-unknown-pragmas"
    CFLAGS="-mtune=generic -pipe -Wno-unknown-pragmas"
    export CMAKE_OSX_ARCHITECTURES="$ARCHITECTURE"
else
    PLATFORM_CMAKE_OPTIONS="-DFSO_BUILD_APPIMAGE=ON -DFORCED_SIMD_INSTRUCTIONS=SSE2"

    # Optionally use static libstdc++/libc++ on Linux, but default to off
    # (Issue #5571 enabled it as a fix for 20.04 compatibility, but we've moved to 22.04)
    PLATFORM_CMAKE_OPTIONS="$PLATFORM_CMAKE_OPTIONS -DUSE_STATIC_LIBCXX=${USE_STATIC_LIBCXX:-OFF}"
fi

CMAKE_OPTIONS="$JOB_CMAKE_OPTIONS"
if [[ "$COMPILER" =~ ^clang.*$ ]]; then
    # Default to allowing system standard c++ library be used. On Linux, which
    # typically uses libstdc++ by default, using libc++ causes a conflict with
    # Qt6 libs. So an error/warning is in place to catch that case when QtFRED
    # is enabled.
    CMAKE_OPTIONS="$CMAKE_OPTIONS -DCLANG_USE_LIBCXX=${CLANG_USE_LIBCXX:-OFF}"
fi

if [ ! "$CCACHE_PATH" = "" ]; then
    echo "Using ccache at $CCACHE_PATH"
    CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_C_COMPILER_LAUNCHER=$CCACHE_PATH -DCMAKE_CXX_COMPILER_LAUNCHER=$CCACHE_PATH"
fi

if [ -n "${ENABLE_QTFRED:-}" ]; then
    CMAKE_OPTIONS="$CMAKE_OPTIONS -DFSO_BUILD_QTFRED=${ENABLE_QTFRED}"
fi

mkdir build
cd build

export CXXFLAGS
export CFLAGS
export LD_LIBRARY_PATH

if [ "$RUNNER_OS" = "macOS" ] && [ ! -x "$(which ninja)" ]; then
    brew install ninja
fi

# CMAKE_JOB_POOLS and CMAKE_JOB_POOL_LINK ensure that when using ninja, link operations are done sequentially
# we have some build rules that do not play nice with parallel invocation and that fixes these issues

cmake -G Ninja -DFSO_FATAL_WARNINGS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $CMAKE_OPTIONS $PLATFORM_CMAKE_OPTIONS \
    -DCMAKE_INSTALL_PREFIX="$(pwd)/install" -DCMAKE_BUILD_TYPE=$CONFIGURATION -DQT_USE_PRECOMPILED=ON \
    -DFFMPEG_USE_PRECOMPILED=ON -DFSO_BUILD_TESTS=ON -DFSO_BUILD_INCLUDED_LIBS=ON \
    -DCMAKE_JOB_POOLS=link=1 -DCMAKE_JOB_POOL_LINK=link ..
