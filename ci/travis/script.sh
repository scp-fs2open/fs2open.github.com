#!/usr/bin/env sh

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    make -j 4
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cd projects/Xcode
    xcodebuild ARCHS=$MACOSX_ARCH ONLY_ACTIVE_ARCH=NO -configuration "$CONFIGURATION" | xcpretty -c && exit ${PIPESTATUS[0]}
fi
