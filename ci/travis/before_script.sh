#!/usr/bin/env bash

set -ex

mkdir -p build
cd build

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    CXXFLAGS="-m64 -mtune=generic -mfpmath=sse -msse -msse2 -pipe -Wno-unknown-pragmas"
    CFLAGS="-m64 -mtune=generic -mfpmath=sse -msse -msse2 -pipe -Wno-unknown-pragmas"

    if [[ "$CC" =~ ^clang.*$ ]]; then
        CXXFLAGS="$CXXFLAGS -stdlib=libc++"
    fi

    export CXXFLAGS
    export CFLAGS
    CMAKE="cmake -G Ninja -DFSO_FATAL_WARNINGS=ON"
    if [ "$BUILD_DEPLOYMENT" = true ]; then
        for config in $BUILD_CONFIGS
        do
            mkdir -p $config
            cd $config
            eval $CMAKE -DFSO_INSTALL_DEBUG_FILES=ON -DCMAKE_BUILD_TYPE=$config -DCMAKE_INSTALL_PREFIX="/tmp/release/$config" \
                -DFSO_BUILD_APPIMAGE=ON -DAPPIMAGE_ASSISTANT="$HOME/AppImageAssistant" \
                -DFSO_BUILD_INCLUDED_LIBS=ON -DFFMPEG_USE_PRECOMPILED=ON -DFSO_BUILD_QTFRED=ON ../..
            cd ..
        done
    else
        eval $CMAKE -DCMAKE_BUILD_TYPE=$CONFIGURATION -DFSO_BUILD_TESTS=ON -DFSO_BUILD_INCLUDED_LIBS=ON -DFSO_BUILD_QTFRED=ON ..
    fi
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    if [ "$BUILD_DEPLOYMENT" = true ]; then
        cmake -G "Xcode" -DFSO_FATAL_WARNINGS=ON -DFSO_INSTALL_DEBUG_FILES=ON -DCMAKE_INSTALL_PREFIX="/tmp/release" ..
    else
        cmake -G "Xcode" ..
    fi
fi
