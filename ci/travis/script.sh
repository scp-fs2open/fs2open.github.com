#!/usr/bin/env sh

set -e

cd build

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    ninja
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cmake --build . --config "$CONFIGURATION" | tee build.log | xcpretty -f `xcpretty-travis-formatter`
    XCODE_RET=${PIPESTATUS[0]}
    if [ "$XCODE_RET" -ne "0" ]; then
        pastebin -e 1d -f build.log
        exit $XCODE_RET
   fi
fi
