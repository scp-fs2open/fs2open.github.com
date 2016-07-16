#!/usr/bin/env sh

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    make -j 4
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cd projects/Xcode

    xcodebuild -project FS2_Open.xcodeproj -configuration "$CONFIGURATION" clean build | tee release.log | xcpretty
    XCODE_RET=${PIPESTATUS[0]}
    if [ "$XCODE_RET" -ne "0" ]; then
        pastebin -e 1d -f release.log
        exit $XCODE_RET
    fi
fi
