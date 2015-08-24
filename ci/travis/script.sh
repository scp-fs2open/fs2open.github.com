#!/usr/bin/env sh

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    make -j 2
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cd projects/Xcode4
    xcodebuild ARCHS=$MACOSX_ARCH ONLY_ACTIVE_ARCH=NO -configuration "$CONFIGURATION" | xcpretty -c
fi
