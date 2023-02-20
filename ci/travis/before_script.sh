#!/usr/bin/env bash

set -ex

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    mkdir -p /tmp/builds
    if [ "$RELEASE_BUILD" = true ]; then
        # Package the source code in release builds
        PARENT_DIR="$(basename "$PWD")"
        (cd .. && tar -czvf /tmp/builds/$PACKAGE_NAME-source-Unix.tar.gz --exclude-vcs "$PARENT_DIR")
    fi
fi

mkdir -p build
cd build

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    CXXFLAGS="-m64 -mtune=generic -mfpmath=sse -msse -msse2 -pipe -Wno-unknown-pragmas"
    CFLAGS="-m64 -mtune=generic -mfpmath=sse -msse -msse2 -pipe -Wno-unknown-pragmas"

    export CXXFLAGS
    export CFLAGS
    CMAKE="cmake -G Ninja -DFSO_FATAL_WARNINGS=ON $CMAKE_OPTIONS"
    if [ "$BUILD_DEPLOYMENT" = true ]; then
        for config in $BUILD_CONFIGS
        do
            mkdir -p $config
            cd $config
            eval $CMAKE -DFSO_INSTALL_DEBUG_FILES=ON -DCMAKE_BUILD_TYPE=$config -DCMAKE_INSTALL_PREFIX="/tmp/release/$config" \
                -DFSO_BUILD_APPIMAGE=ON -DFSO_BUILD_INCLUDED_LIBS=ON -DFFMPEG_USE_PRECOMPILED=ON -DFSO_BUILD_QTFRED=OFF ../..
            cd ..
        done
    else
        eval $CMAKE -DCMAKE_BUILD_TYPE=$CONFIGURATION -DFSO_BUILD_TESTS=ON -DFSO_BUILD_INCLUDED_LIBS=ON -DFSO_BUILD_QTFRED=OFF ..
    fi
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    if [ "$BUILD_DEPLOYMENT" = true ]; then
        cmake -G "Xcode" -DFSO_FATAL_WARNINGS=ON -DFSO_INSTALL_DEBUG_FILES=ON -DCMAKE_INSTALL_PREFIX="/tmp/release" ..
    else
        cmake -G "Xcode" $CMAKE_OPTIONS ..
    fi
elif [ "$TRAVIS_OS_NAME" = "windows" ]; then
    if [ "$CMAKE_GENERATOR" = "Ninja" ]; then
        cmake -DFSO_USE_SPEECH="OFF" -DFSO_FATAL_WARNINGS="ON" -DFSO_USE_VOICEREC="OFF" -DFSO_BUILD_TESTS="ON" \
            -DFSO_BUILD_FRED2="OFF" -G "$CMAKE_GENERATOR" ..
    else
        cmake -DFSO_USE_SPEECH="ON" -DFSO_FATAL_WARNINGS="ON" -DFSO_USE_VOICEREC="OFF" -DFSO_BUILD_TESTS="ON" \
            -DMSVC_SIMD_INSTRUCTIONS=SSE2 -DFSO_BUILD_FRED2="ON" -G "$CMAKE_GENERATOR" -T "$CMAKE_TOOLSET" ..
    fi
fi
