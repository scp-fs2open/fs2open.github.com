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
    # TODO: Vulkan support is disabled on MacOS due to issues with the test suite not linking correctly
    PLATFORM_CMAKE_OPTIONS="-DFSO_BUILD_WITH_VULKAN=OFF"
    export CMAKE_OSX_ARCHITECTURES="$ARCHITECTURE"
else
    PLATFORM_CMAKE_OPTIONS="-DFSO_BUILD_APPIMAGE=ON -DFORCED_SIMD_INSTRUCTIONS=SSE2 -DUSE_STATIC_LIBCXX=ON"
fi

CMAKE_OPTIONS="$JOB_CMAKE_OPTIONS"
if [[ "$COMPILER" =~ ^clang.*$ ]]; then
    CMAKE_OPTIONS="$CMAKE_OPTIONS -DCLANG_USE_LIBCXX=ON"
    # force clang to silently allow -static-libstdc++ flag
fi

if [ ! "$CCACHE_PATH" = "" ]; then
    echo "Using ccache at $CCACHE_PATH"
    CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_C_COMPILER_LAUNCHER=$CCACHE_PATH -DCMAKE_CXX_COMPILER_LAUNCHER=$CCACHE_PATH"
fi

mkdir build
cd build

export CXXFLAGS
export CFLAGS
export LD_LIBRARY_PATH

if [ "$RUNNER_OS" = "macOS" ]; then
    brew install ninja
fi

# CMAKE_JOB_POOLS and CMAKE_JOB_POOL_LINK ensure that when using ninja, link operations are done sequentially
# we have some build rules that do not play nice with parallel invocation and that fixes these issues

cmake -G Ninja -DFSO_FATAL_WARNINGS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $CMAKE_OPTIONS $PLATFORM_CMAKE_OPTIONS \
    -DCMAKE_INSTALL_PREFIX="$(pwd)/install" -DCMAKE_BUILD_TYPE=$CONFIGURATION \
    -DFFMPEG_USE_PRECOMPILED=ON -DFSO_BUILD_TESTS=ON -DFSO_BUILD_INCLUDED_LIBS=ON -DFSO_BUILD_QTFRED=${ENABLE_QTFRED:-OFF} \
    -DSHADERS_ENABLE_COMPILATION=ON -DCMAKE_JOB_POOLS=link=1 -DCMAKE_JOB_POOL_LINK=link ..
